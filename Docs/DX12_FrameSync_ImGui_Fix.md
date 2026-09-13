# DX12 프레임 동기화 및 ImGui 렌더링 수정

> 커밋: `7ac7ef2` — Fix DX12 frame sync and ImGui rendering

---

## 시작하기 전에

이 커밋은 단순한 기능 추가가 아니다. **GPU와 CPU의 동기화가 무너지면서 발생하던 크래시와 렌더링 오류**를 근본 원인부터 수정한 작업이다. 증상 자체는 간헐적인 크래시, 화면 깜박임, ImGui가 그려지지 않는 문제 등 다양한 형태로 나타났지만, 원인은 크게 세 가지 영역에서 얽혀 있었다.

1. Fence 값 관리 설계가 잘못되어 있었다.
2. ImGui 렌더링이 커맨드 리스트 흐름을 무시하고 동작하고 있었다.
3. 프레임 루프에서 대기(Wait)의 위치가 잘못되어 있었다.

각 문제를 순서대로 짚어보겠다.

---

## 1. Fence란 무엇이고 왜 중요한가

DX12는 CPU와 GPU가 **비동기적으로** 동작한다. CPU가 커맨드 리스트를 제출하고 나면 GPU는 독립적으로 그것을 처리한다. CPU는 GPU가 언제 끝났는지 알 수 없기 때문에, 다음 프레임에서 같은 커맨드 할당자(CommandAllocator)를 재사용하려면 GPU가 이전 프레임의 명령을 **완전히 끝냈는지 확인**해야 한다. 이 확인 수단이 **Fence**다.

```
  CPU 타임라인
  ──────────────────────────────────────────────────────────▶
  │ Frame 0 기록  │ Submit │         │ Frame 1 기록  │ Submit
  │ (Cmd 기록)    │        │         │               │
                  │        │         │               │
                  ▼        │         │               ▼
  GPU 타임라인    Signal(1) │         │             Signal(2)
  ──────────────────────────────────────────────────────────▶
                  │ Frame 0 실행 중... │ 완료 → Fence=1
                  │                   │
                  └── CPU: Wait(1) ───┘
                      "Fence가 1이 될 때까지 대기"
                      → 1이 됐다 → Frame 0 Allocator 안전하게 Reset 가능
```

더블 버퍼링 구조에서는 프레임 0과 1이 번갈아가며 사용된다. 각 프레임은 자신만의 커맨드 할당자를 가지며, 그 할당자를 재사용하기 전에 반드시 GPU가 이전 작업을 끝냈는지 확인해야 한다.

```
  FrameContext[0]  ──▶  CommandAllocator[0]  ──▶  Frame 0, 2, 4 ...
  FrameContext[1]  ──▶  CommandAllocator[1]  ──▶  Frame 1, 3, 5 ...

  Frame 2 시작 전:
    FrameContext[0].FenceValue 가 완료됐는지 확인
    (Frame 0에서 Signal한 값을 GPU가 처리했는지)
    → 완료됐으면 Allocator[0] Reset 후 재사용
    → 아직이면 Wait
```

---

## 2. 문제 1 — 초기화 시 FenceValue++ 가 데드락을 만든다

수정 전 `Initialize()` 마지막에 다음 한 줄이 있었다.

```cpp
mFrameContext[mFrameIndex].FenceValue++;
// Fence는 0으로 생성됐는데, FenceValue만 1이 됨
```

이 한 줄이 첫 프레임에서 데드락을 일으킬 수 있었다.

```
  Initialize() 직후 상태

  Fence 실제 값:         0
  FrameContext[0].FenceValue:  1  ← 잘못된 초기화

  첫 프레임 WaitForNextFrameResources() 진입:

  fenceValue = FrameContext[0].FenceValue = 1
  if (fenceValue != 0)  →  true!
      mFence->SetEventOnCompletion(1, event)  ← "Fence가 1이 될 때까지 대기"
      WaitForSingleObject(event, INFINITE)

  하지만 아직 아무 Signal도 보내지 않았음
  GPU의 Fence는 영원히 0
  → 무한 대기 (데드락)
```

수정 후에는 FenceValue의 초기값 0이 "이 컨텍스트에 대기할 GPU 작업이 없다"는 뜻으로 명확히 약속된다. `FenceValue != 0` 체크가 의미있게 동작하려면 0은 반드시 "비어있음"을 의미해야 한다.

```
  수정 후 Initialize() 직후 상태

  Fence 실제 값:              0
  FrameContext[0].FenceValue: 0  ← "아직 GPU 작업 없음"
  FrameContext[1].FenceValue: 0  ← "아직 GPU 작업 없음"
  mFenceLastSignalValue:      0

  첫 프레임 WaitForNextFrameResources() 진입:

  fenceValue = FrameContext[0].FenceValue = 0
  if (fenceValue != 0)  →  false → 대기 없이 바로 통과  ✅
```

---

## 3. 문제 2 — Fence 값이 단조 증가를 보장받지 못했다

DX12 Fence는 반드시 **단조 증가(monotonically increasing)** 해야 한다. 이미 완료된 값보다 작거나 같은 값으로 Signal하면 GPU validation 오류가 발생한다.

수정 전에는 FenceValue가 여러 곳에서 독립적으로 관리됐다.

```
  수정 전 FenceValue 관리 (분산된 상태)

  Initialize()
    └─ FrameContext[0].FenceValue = 1   (++)

  WaitForGpu()
    └─ Signal(FrameContext[mFrameIndex].FenceValue)  ← 현재 값으로 Signal
    └─ FrameContext[mFrameIndex].FenceValue++         ← 이후 증가

  MoveToNextFrame()
    └─ currentFenceValue = FrameContext[mFrameIndex].FenceValue
    └─ Signal(currentFenceValue)
    └─ FrameContext[mFrameIndex].FenceValue = currentFenceValue + 1  ← 나중에 설정

  문제: Signal에 사용된 값이 어디선가 이미 사용됐거나,
        혹은 0이 된 경우 (초기화 이후 리셋됐을 때) Signal(0) 호출 가능
        → D3D12 오류
```

수정 후에는 `mFenceLastSignalValue`라는 단일 카운터 하나만 사용한다. Signal은 항상 이 값에 +1을 더한 값으로만 이루어진다.

```
  수정 후 FenceValue 관리 (단일 카운터)

  mFenceLastSignalValue: 0 (초기)
                         │
                         ▼
  Signal 시:   value = mFenceLastSignalValue + 1
               Signal(queue, value)
               mFenceLastSignalValue = value        ← 항상 증가
               FrameContext[mFrameIndex].FenceValue = value  ← 이 프레임의 증거 저장

  시간 흐름 →

  mFenceLastSignalValue:    0 ──▶ 1 ──▶ 2 ──▶ 3 ──▶ 4 ──▶ ...
                                  │      │      │      │
  FrameContext[0].FenceValue:     1      │      3      │
  FrameContext[1].FenceValue:     │      2      │      4
                                  │      │      │      │
  GPU Fence 완료:                 ①     ②      ③      ④    (항상 오름차순)
```

---

## 4. 문제 3 — WaitForNextFrameResources가 mFrameIndex를 변경했다

이것이 가장 교묘한 버그였다. `WaitForNextFrameResources()`는 프레임 루프 맨 앞에서 호출된다. 그런데 수정 전에는 이 함수 안에서 `mFrameIndex`를 증가시키고 있었다.

```cpp
// Before
UINT nextFrameIndex = mFrameIndex + 1;
mFrameIndex = nextFrameIndex;  // ← 프레임 시작 직후에 인덱스 변경!
FrameContext* frameCtx = &mFrameContext[nextFrameIndex % 2];
```

`mFrameIndex`는 엔진 전체에서 "현재 렌더링 중인 백버퍼"를 가리키는 인덱스다. 이 값이 프레임 중간에 바뀌면, 이후 호출되는 모든 함수들이 잘못된 대상에 작업을 하게 된다.

```
  수정 전 프레임 흐름 (mFrameIndex=0 에서 시작한다고 가정)

  [프레임 시작]
  WaitForNextFrameResources()
    └─ mFrameIndex = 0 + 1 = 1  ← 여기서 변경됨!

  Application::Render()
    └─ ResetCommandAllocator()
    │    └─ mFrameContext[1].Allocator->Reset()  ← 인덱스 1 사용
    └─ TransitionBarrier(PRESENT → RT)
    │    └─ mRenderTargets[1] 에 배리어  ← 인덱스 1 사용
    └─ BindFrameBuffer()
         └─ mRtvHeap[1] 에 렌더 타겟 설정  ← 인덱스 1 사용

  실제 SwapChain 현재 백버퍼: 0  ← 인덱스 불일치!
                                     │
                                     ▼
  RT[0]은 여전히 PRESENT 상태인데
  RT[1]을 RT 상태로 전환 후 렌더링
  → Present 시 RT[0]을 제시하려 하지만
    RT[0]의 상태가 PRESENT가 아닌 경우 발생 가능
  → GPU validation 오류 또는 화면 깜박임
```

수정 후에는 `WaitForNextFrameResources()`가 `mFrameIndex`를 전혀 건드리지 않는다. `mFrameIndex`는 오직 두 곳에서만 갱신된다.

```
  수정 후 mFrameIndex 갱신 시점

  SignalFrameCompletion() 또는 MoveToNextFrame() 내부
    └─ mFrameIndex = mSwapChain->GetCurrentBackBufferIndex()
       ↑ Present() 이후, SwapChain이 실제로 버퍼를 교체한 다음에만 갱신

  WaitForNextFrameResources()
    └─ mFrameIndex 건드리지 않음
    └─ frameCtx = &mFrameContext[mFrameIndex]  ← 현재 그대로 사용

  프레임 전체에서 mFrameIndex는 일관되게 유지됨  ✅
```

---

## 5. 문제 4 — SignalFrameCompletion이 잘못된 컨텍스트에 Fence를 저장했다

위의 문제 3(WaitForNextFrameResources에서 mFrameIndex 변경)의 연쇄 영향이었다.

```
  수정 전 SignalFrameCompletion()

  UINT64 fenceValue = mFenceLastSignalValue + 1;
  Signal(fenceValue);
  mFenceLastSignalValue = fenceValue;

  FrameContext* frameCtx = &mFrameContext[mFrameIndex % 2];
  //                                      ^^^^^^^^^^^
  //  WaitForNextFrameResources에서 이미 mFrameIndex가 증가했으므로
  //  이번 프레임에 실제 사용한 Allocator가 있는 컨텍스트가 아닌
  //  다음 프레임의 컨텍스트에 Fence 값이 저장됨!

  frameCtx->FenceValue = fenceValue;
```

이것은 무엇을 의미하는가? FenceValue는 "이 컨텍스트의 Allocator가 마지막으로 사용된 프레임의 Signal 값"을 저장해야 한다. 다음 프레임에서 이 Allocator를 재사용할 때 그 Signal이 완료됐는지 확인하기 위해서다. 잘못된 컨텍스트에 저장되면 이 확인이 무의미해진다.

```
  잘못된 저장 예시

  실제 이번 프레임에 사용한 Allocator: FrameContext[0]
  Fence 값: 3 을 FrameContext[1]에 저장 ← 엉뚱한 곳!

  다음에 FrameContext[0] 재사용할 때:
    FenceValue = 0  (저장 안 됐으므로)
    0이니까 바로 통과 → GPU가 아직 작업 중인데 Allocator Reset!
    → 크래시 또는 렌더링 오류

  다음에 FrameContext[1] 재사용할 때:
    FenceValue = 3  (엉뚱하게 저장됨)
    3을 기다림 → 3은 Frame 0의 작업 완료 값
    실제 Frame 1의 완료 여부와는 무관한 값을 기다리게 됨
```

수정 후에는 mFrameIndex가 프레임 내내 일관되므로 단순하게 `mFrameContext[mFrameIndex]`에 저장하면 항상 올바르다.

---

## 6. ImGui 렌더링 구조의 문제

### 커맨드 리스트는 하나다

DX12에서 커맨드 리스트는 열려 있는(Open) 동안만 명령을 기록할 수 있다. `Close()`를 호출하면 이후에는 `Execute` 및 `Reset`만 가능하다.

YamYam Engine의 렌더링은 하나의 커맨드 리스트를 사용한다. 따라서 게임 렌더링과 ImGui 렌더링이 **같은 커맨드 리스트에 순서대로 기록**되어야 한다.

```
  올바른 커맨드 리스트 사용 흐름

  Reset(Allocator, PSO)
       │
       ▼
  [게임 렌더링 명령 기록]
  SetGraphicsRootSignature
  BindViewportAndScissor
  TransitionBarrier(PRESENT → RT)    ← 백버퍼를 RT 상태로
  BindFrameBuffer + ClearRTV
  Draw calls ...
       │
       ▼
  [ImGui 렌더링 명령 기록]         ← 같은 커맨드 리스트를 이어받음
  SetDescriptorHeaps(srvHeap)
  ImGui_ImplDX12_RenderDrawData
  TransitionBarrier(RT → PRESENT)   ← 백버퍼를 PRESENT 상태로
       │
       ▼
  Close()
       │
       ▼
  ExecuteCommandList()
  Present()
```

### 수정 전 — 흐름을 깬 구조

수정 전 `guiImguiEditor::End()`는 이 흐름을 여러 곳에서 깨고 있었다.

```
  수정 전 guiImguiEditor::End() 내부

  ┌─────────────────────────────────────────────────────────┐
  │  WaitForNextFrameResources()   ← ① 이미 Reset된 후에 Wait!
  │                                                          │
  │  backBufferIdx =                                         │
  │    swapChain->GetCurrentBackBufferIndex()  ← ② mFrameIndex와 다를 수 있음
  │                                                          │
  │  barrier: PRESENT → RT  ← ③ Application::Render()가 이미 쳤음 (중복!)
  │                                                          │
  │  SetDescriptorHeaps(srvHeap)                             │
  │  ImGui_ImplDX12_RenderDrawData                           │
  │                                                          │
  │  barrier: RT → PRESENT  ← ④ backBufferIdx 기준 (② 와 같은 인덱스)
  │  commandList->Close()                                    │
  └─────────────────────────────────────────────────────────┘

  ① Application::Render() 에서 ResetCommandAllocator/List 가 이미 끝난 후
    Wait 호출 → 아무 의미 없거나, 이미 진행 중인 프레임에 간섭

  ② GetCurrentBackBufferIndex() 는 Present 전/후에 따라 값이 달라짐
    Application::Render() 의 mFrameIndex 와 다를 경우:

    Application::Render() 가 mFrameIndex=0 기준으로
      TransitionBarrier(RT[0]: PRESENT→RT) 를 기록
    End() 가 backBufferIdx=1 기준으로
      TransitionBarrier(RT[1]: RT→PRESENT) 를 기록

    결과:
      RT[0] → PRESENT 상태로 복원 안 됨  ← Present 시 오류
      RT[1] → RT 상태인데 PRESENT로 전환하려 함  ← 상태 불일치
```

```
  인덱스 불일치 시나리오

  Application::Render()
    mFrameIndex = 0
    │
    ├─ PRESENT → RT (RT[0])
    ├─ Draw ...
    │

  guiImguiEditor::End()
    backBufferIdx = GetCurrentBackBufferIndex() = 1  ← 다름!
    │
    ├─ PRESENT → RT (RT[1])  ← RT[1]은 이미 PRESENT 상태, OK
    ├─ ImGui draw
    ├─ RT → PRESENT (RT[1])  ← RT[1] 복원
    └─ Close()

  ExecuteCommandList()
  Present()
    └─ RT[0] 는 여전히 RT 상태 ← PRESENT 불가 → 오류
```

### 수정 후 — 하나의 흐름으로 통일

```
  수정 후 guiImguiEditor::End() 내부

  ┌─────────────────────────────────────────────────────────┐
  │  ImGui::Render()                                         │
  │    └─ CPU 측 드로우 데이터 정리 (GPU 작업 없음)           │
  │                                                          │
  │  commandList = graphicDevice->GetCommandList()           │
  │    └─ Application::Render()가 사용 중인 그 커맨드 리스트  │
  │                                                          │
  │  commandList->SetDescriptorHeaps(1, &srvHeap)            │
  │    └─ ImGui 폰트/이미지 SRV 접근을 위해 필수             │
  │                                                          │
  │  ImGui_ImplDX12_RenderDrawData(drawData, commandList)    │
  │    └─ 이미 열려 있는 커맨드 리스트에 ImGui 명령 추가     │
  │                                                          │
  │  backBufferIdx = graphicDevice->GetFrameIndex()          │
  │    └─ mFrameIndex 그대로 사용 → Application과 동일 인덱스│
  │                                                          │
  │  barrier: RT → PRESENT (RT[backBufferIdx])               │
  │    └─ Application::Render()가 PRESENT→RT 했던 그 버퍼   │
  │                                                          │
  │  commandList->Close()                                    │
  └─────────────────────────────────────────────────────────┘

  모든 배리어가 동일한 인덱스를 사용
  RT[i]: PRESENT → RT (Application) → ImGui draw → RT → PRESENT (End)
  일관된 상태 전환  ✅
```

---

## 7. SetDescriptorHeaps — 왜 ImGui 전에 반드시 필요한가

DX12에서 셰이더가 텍스처(SRV)를 읽으려면, 해당 텍스처의 descriptor가 있는 Heap이 커맨드 리스트에 **바인딩**되어 있어야 한다. ImGui는 폰트 텍스처를 SRV로 관리하기 때문에 렌더링 직전에 반드시 SRV Heap을 설정해야 한다.

```
  SetDescriptorHeaps 없을 때

  ImGui_ImplDX12_RenderDrawData()
    └─ Draw 명령 기록
    └─ 폰트 SRV 슬롯 참조
         └─ GPU: "어느 Heap에서 찾아야 하지?"  ← 바인딩된 Heap 없음
              └─ D3D12 ERROR: No descriptor heaps bound
                              → 렌더링 실패 또는 크래시

  SetDescriptorHeaps(srvHeap) 호출 후

  커맨드 리스트 상태:
    Bound Heap: srvHeap (SHADER_VISIBLE CBV_SRV_UAV)
         │
         ▼
  ImGui_ImplDX12_RenderDrawData()
    └─ 폰트 SRV → srvHeap[slot] 에서 찾음  ✅
```

---

## 8. 프레임 루프에서 Wait의 위치

`WaitForNextFrameResources()`는 커맨드 할당자를 **Reset하기 전**에 반드시 완료되어야 한다.

```
  올바른 순서 (수정 후)

  ┌─ 프레임 루프 ────────────────────────────────────────────┐
  │                                                           │
  │  1. WaitForNextFrameResources()                           │
  │       └─ FrameContext[mFrameIndex].FenceValue 확인        │
  │       └─ GPU가 이전 프레임 명령 완료할 때까지 대기        │
  │       └─ 완료 → FenceValue = 0 (재사용 가능 표시)        │
  │                     ↓ 이제 안전                           │
  │  2. Application::Run()                                    │
  │       └─ ResetCommandAllocator()   ← GPU가 다 쓴 후 리셋 │
  │       └─ ResetCommandList()                               │
  │       └─ [게임 렌더링 명령 기록]                          │
  │                     ↓                                     │
  │  3. EditorApplication::Run()                              │
  │       └─ ImGui::Begin (새 프레임)                         │
  │       └─ OnImGuiRender (UI 구성)                          │
  │       └─ ImguiEditor::End()                               │
  │            └─ [ImGui 명령 기록]                           │
  │            └─ RT → PRESENT 배리어                        │
  │            └─ commandList->Close()                        │
  │                     ↓                                     │
  │  4. ExcuteCommandList()    ← GPU에 제출                   │
  │  5. UpdatePlatformWindows()                               │
  │  6. Present()                                             │
  │  7. SignalFrameCompletion()                               │
  │       └─ Signal(++mFenceLastSignalValue)                  │
  │       └─ FrameContext[mFrameIndex].FenceValue = value     │
  │       └─ mFrameIndex = swapChain->GetCurrentBackBufferIndex()
  │                                                           │
  └─ 다음 프레임으로 ────────────────────────────────────────┘
```

수정 전에는 Wait가 `EditorApplication::Run()` 내부의 `guiImguiEditor::End()`에 있었다. 이미 2번(Reset)이 끝난 후에 Wait가 실행되는 꼴이었다. 할당자를 리셋했는데 나중에 "GPU가 다 썼나요?"를 확인하는 것은 순서가 완전히 뒤집혀 있다.

```
  수정 전 잘못된 순서

  Application::Run()
    └─ ResetCommandAllocator()  ← ① 먼저 리셋 (GPU가 아직 쓰고 있을 수 있음!)
    └─ ResetCommandList()
    └─ 게임 렌더링

  EditorApplication::Run()
    └─ guiImguiEditor::End()
         └─ WaitForNextFrameResources()  ← ② 뒤늦게 Wait
              "GPU가 끝났나요?"
              이미 Reset해버린 후라 의미 없음
```

---

## 9. Input 포커스 버그

동기화와 직접 관련은 없지만 같은 커밋에서 수정됐다.

```
  수정 전 updateKey() 구조

  if (GetFocus()) {
      updateKeyDown / updateKeyUp
      getMousePositionByWindow()
  } else {
      clearKeys()
      ← 함수 종료 → 여기까지는 괜찮지만...
  }

  문제: clearKeys() 는 모든 키를 NONE 상태로 리셋하는데
        다음 updateKeys() 호출(다른 Key 처리)에서 포커스가 없어도
        같은 분기를 타며 clearKeys() 를 반복 호출
        → 불필요한 반복 실행 + 의도가 불명확

  수정 후 updateKey() 구조

  if (!GetFocus()) {
      clearKeys();
      return;   ← 명시적 early return, 이후 코드 절대 실행 안 됨
  }

  updateKeyDown / updateKeyUp     ← 포커스 있을 때만 도달
  getMousePositionByWindow()
```

---

## 10. 전체 수정 흐름 한눈에 보기

```
  수정 전 문제 지도

  Initialize()
    └─ FenceValue++ ─────────────────────────────▶ 첫 프레임 데드락

  WaitForNextFrameResources()
    └─ mFrameIndex++ ────────────────────────────▶ 프레임 전체 인덱스 불일치
                                                    │
                                           SignalFrameCompletion()
                                             └─ 잘못된 FrameContext에 저장
                                                → Allocator 재사용 시 오류

  MoveToNextFrame()
    └─ FrameContext.FenceValue 를 Signal에 사용 ▶ 단조 증가 미보장
                                                 → D3D12 validation 오류

  guiImguiEditor::End()
    └─ 내부에서 Wait 호출 ───────────────────────▶ Reset 이후 Wait (순서 역전)
    └─ 독자적 backBufferIdx ─────────────────────▶ mFrameIndex 와 불일치
    └─ 중복 PRESENT→RT 배리어 ───────────────────▶ 리소스 상태 오류
    └─ SetDescriptorHeaps 위치 문제 ─────────────▶ ImGui 텍스처 렌더링 실패

  ──────────────────────────────────────────────────────────────

  수정 후 개선된 구조

  mFenceLastSignalValue (단일 카운터)
    └─ 모든 Signal은 여기서 +1 ──────────────────▶ 단조 증가 보장

  WaitForNextFrameResources()
    └─ mFrameIndex 건드리지 않음 ────────────────▶ 인덱스 일관성 보장

  SignalFrameCompletion()
    └─ mFrameContext[mFrameIndex] 에 정확히 저장 ▶ Allocator 추적 정확

  프레임 루프 맨 앞에 Wait
    └─ Reset 전 GPU 완료 확인 ───────────────────▶ 올바른 순서

  guiImguiEditor::End()
    └─ GetFrameIndex() 로 mFrameIndex 공유 ──────▶ 배리어 대상 일치
    └─ 기존 커맨드 리스트 이어받아 사용 ─────────▶ 단일 흐름 유지
    └─ SetDescriptorHeaps → ImGui render ────────▶ 텍스처 렌더링 정상
```

---

## 11. 이 수정이 왜 어려운가

Fence 동기화 버그는 **재현이 불규칙적**이라는 특성 때문에 원인을 찾기가 어렵다. GPU와 CPU의 타이밍이 매 실행마다 다르기 때문에, 같은 코드가 어떤 날은 정상 동작하고 어떤 날은 크래시가 난다. GPU validation layer를 켜면 오류 메시지가 나오지만, 메시지만으로는 어떤 순서로 잘못됐는지를 파악하기 어렵다.

이번 수정의 핵심은 상태를 여러 곳에서 분산 관리하던 방식을 **명확한 단일 흐름으로 단순화**한 것이다. mFrameIndex는 SwapChain에서만 갱신되고, FenceValue는 단일 카운터에서만 증가하며, ImGui는 열려 있는 커맨드 리스트를 이어받는다. 각자의 책임이 명확해지면 버그가 끼어들 틈이 없어진다.
