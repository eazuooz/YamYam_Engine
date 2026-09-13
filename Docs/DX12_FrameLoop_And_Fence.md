# DX12 Frame Loop & Fence 동기화 완전 해설

> YamYam Engine 기준으로 작성된 DX12 프레임 루프와 GPU 동기화 아키텍처 문서.
> Editor 빌드(`#define WITH_EDITOR`)와 Game 빌드의 구조적 차이, 그리고 Fence가 왜 필요한지를
> 처음부터 순서대로 설명한다.

---

## 1. 들어가며 — GPU는 CPU의 명령을 즉시 실행하지 않는다

DirectX 12를 처음 접할 때 가장 큰 충격은 이것이다.

```cpp
mCommandList->DrawIndexedInstanced(...); // 이 줄은 GPU가 즉시 실행하지 않는다
```

이 코드는 GPU에게 "지금 그려라"를 명령하는 게 아니다. CommandList라는 **녹화 테이프**에
그 명령을 기록하는 것이다. GPU가 실제로 그리기 시작하는 시점은 나중에 `ExecuteCommandLists()`를
호출할 때다.

```
CPU 시점:                          GPU 시점:

DrawIndexedInstanced() 호출        ← 아직 아무 것도 안 함
ExecuteCommandLists() 호출         ← 이제 GPU가 실행 시작
CPU는 계속 다음 작업 진행          ← GPU는 비동기로 처리 중
```

여기서 문제가 생긴다. CPU는 빠르게 다음 프레임을 준비하려 하는데, GPU는 아직 이전 프레임을
처리하고 있다. 이 타이밍을 맞추는 장치가 바로 **Fence**다.

---

## 2. Fence란 무엇인가

Fence는 CPU와 GPU 사이에 놓인 **단방향 신호 깃발**이다.

```
   CPU                               GPU
    |                                 |
    |  mCommandQueue->Signal(fence, N) |
    |-------------------------------->|  "N번 깃발 꽂아줘"
    |                                 |
    |  (CPU는 계속 다른 일 함)         |  ... GPU 작업 중 ...
    |                                 |
    |                                 |  [GPU가 N까지 처리 완료]
    |                                 |  fence->GetCompletedValue() == N
    |                                 |
    |  (CPU가 N 이상인지 확인 가능)    |
```

`Signal(fence, N)`은 GPU에게 "네가 여기까지 실행을 완료하면, fence의 값을 N으로 만들어줘"
라고 예약하는 것이다. CPU는 나중에 `fence->GetCompletedValue()`로 GPU가 어디까지 왔는지
언제든지 폴링할 수 있다.

### 2.1 fence 값은 단조 증가(Monotonic)해야 한다

YamYam Engine에서는 `mFenceLastSignalValue`라는 전역 카운터를 관리한다.

```cpp
// GraphicDevice_DX12.h
UINT64 mFenceLastSignalValue;  // 초깃값 0, 이후 단조 증가

// 신호를 보낼 때마다 반드시 이전 값보다 크게 설정
UINT64 fenceValue = mFenceLastSignalValue + 1;
mCommandQueue->Signal(mFence.Get(), fenceValue);
mFenceLastSignalValue = fenceValue;
```

왜 단조 증가여야 하는가? GPU는 Signal을 순서대로 처리한다. 만약 프레임 1에 fence=3을
Signal하고, 프레임 2에 fence=2를 Signal하면, GPU는 내부적으로 혼란에 빠진다.
fence 값은 항상 이전보다 커야 "이 fence 값이 완료됐다"는 게 명확해진다.

### 2.2 이벤트 기반 대기

CPU가 GPU를 기다릴 때 루프로 폴링하면 CPU 자원을 낭비한다.

```cpp
// 나쁜 방법 (Busy-wait)
while (mFence->GetCompletedValue() < targetValue) { /* CPU 100% 소모 */ }

// 좋은 방법 (Event-based wait)
mFence->SetEventOnCompletion(targetValue, mFenceEvent);
WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
// GPU가 targetValue에 도달하는 순간 OS가 이 스레드를 깨워준다
```

`mFenceEvent`는 Windows의 Event 커널 오브젝트다. GPU가 해당 fence 값에 도달하면
하드웨어 인터럽트를 통해 이 이벤트를 신호(signal) 상태로 만들고,
`WaitForSingleObjectEx`가 반환된다.

---

## 3. FrameContext와 더블 버퍼링

YamYam Engine은 스왑 체인을 2개의 백 버퍼로 구성한다(Double Buffering).

```
SwapChain
  ├── BackBuffer[0]  ← 현재 화면에 표시 중
  └── BackBuffer[1]  ← GPU가 지금 그리는 중
```

그에 대응하여 `FrameContext`도 2개다.

```cpp
struct FrameContext {
    ComPtr<ID3D12CommandAllocator> CommandAllocator;
    UINT64 FenceValue;  // 이 컨텍스트가 마지막으로 제출된 fence 값
                        // 0이면 GPU 미사용 상태(초깃값)
};

FrameContext mFrameContext[2];
```

**CommandAllocator**는 GPU 커맨드의 메모리 풀이다. CommandList에 기록한 모든 명령의
실제 바이트들이 여기에 저장된다. `Reset()`을 통해 재사용하는데, 이때 반드시 GPU가
해당 알로케이터의 이전 커맨드를 모두 처리한 상태여야 한다.

이것이 핵심이다. **알로케이터 Reset 전에 GPU가 완료됐는지 확인해야 한다.**
`FenceValue`는 이 알로케이터가 마지막으로 사용된 프레임의 fence 값을 기억해두는 역할이다.

```
Frame 0 제출 시:
  mFrameContext[0].FenceValue = 1  ← "fence 1이 완료되면 이 알로케이터 재사용 가능"

Frame 1 제출 시:
  mFrameContext[1].FenceValue = 2  ← "fence 2가 완료되면 이 알로케이터 재사용 가능"

Frame 0을 다시 쓰려 할 때:
  GPU completed value >= 1?  → 맞으면 Reset() 해도 안전
```

---

## 4. 4가지 동기화 함수 해부

YamYam Engine에는 동기화 목적의 함수가 4개 있다.
각각의 목적과 시점이 다르다.

### 4.1 `WaitForGpu()` — 완전 정지

```cpp
void GraphicDevice_DX12::WaitForGpu()
{
    const UINT64 fence = mFenceLastSignalValue + 1;
    mCommandQueue->Signal(mFence.Get(), fence);
    mFence->SetEventOnCompletion(fence, mFenceEvent);
    WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
    mFenceLastSignalValue = fence;
}
```

```
CPU ──Signal(N)──┐
                 │
                 ▼
GPU ─────────────[모든 작업 완료]──Signal 응답
                                       │
CPU ◄──────────────────────────────────┘ (깨어남)
```

**언제 쓰는가?** 초기화 시점, 또는 윈도우 크기 변경처럼 GPU 리소스를 완전히 재구성해야 할 때.
Signal과 Wait를 하나의 함수에서 한 번에 처리하기 때문에 호출하는 순간 CPU가 완전히 멈춘다.
매 프레임 쓰면 GPU-CPU 파이프라이닝이 전혀 되지 않아 성능이 절반으로 떨어진다.

---

### 4.2 `SignalFrameCompletion()` — Signal만, Wait 없음

```cpp
void GraphicDevice_DX12::SignalFrameCompletion()
{
    UINT64 fenceValue = mFenceLastSignalValue + 1;
    mCommandQueue->Signal(mFence.Get(), fenceValue);
    mFenceLastSignalValue = fenceValue;

    // 이 프레임에 사용한 알로케이터에 fence 값 기록
    mFrameContext[mFrameIndex].FenceValue = fenceValue;

    // Present() 이후의 새 백버퍼 인덱스로 전진
    mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
}
```

**Editor 전용 함수**다. Signal만 보내고 즉시 반환한다. CPU는 기다리지 않는다.
다음 프레임 시작 시점에 `WaitForNextFrameResources()`가 그 fence 값을 기다린다.

```
프레임 N 끝:   Signal(fence=N) ─────┐  CPU 즉시 반환
                                     │
프레임 N+1 시작: Wait(fence=N) ◄────┘  그때 가서 대기
```

중요한 것은 **mFrameIndex를 갱신하는 시점**이다. Present() 이후에 `GetCurrentBackBufferIndex()`를
호출해야 스왑 체인이 뒤집힌 후의 새 백버퍼 인덱스를 얻을 수 있다.
그래서 `SignalFrameCompletion()`은 Present() 이후에 호출된다.

---

### 4.3 `WaitForNextFrameResources()` — 다음 프레임 시작 직전 Wait

```cpp
FrameContext* GraphicDevice_DX12::WaitForNextFrameResources()
{
    HANDLE waitableObjects[] = { mSwapChain->GetFrameLatencyWaitableObject(), nullptr };
    DWORD numWaitableObjects = 1;

    FrameContext* frameCtx = &mFrameContext[mFrameIndex];
    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue != 0)  // GPU 작업이 실제로 있었던 프레임만 대기
    {
        frameCtx->FenceValue = 0;
        mFence->SetEventOnCompletion(fenceValue, mFenceEvent);
        waitableObjects[1] = mFenceEvent;
        numWaitableObjects = 2;
    }

    WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

    return frameCtx;
}
```

두 가지를 동시에 기다린다.

```
WaitForMultipleObjects([SwapChain Latency Object, Fence Event])
         │                          │
         ▼                          ▼
스왑체인이 새 프레임          알로케이터가 GPU에서
준비됐는지 확인               안전하게 사용 가능한지 확인
```

`DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT`로 생성된 스왑 체인은
"나 이제 새 프레임 받을 준비 됐어"라는 신호를 주는 핸들을 가진다.
이것을 기다리면 스왑 체인 내부 큐가 넘치지 않게 된다(최대 2프레임 선행).

**mFrameIndex는 이 함수에서 절대로 변경하지 않는다.**
이미 `SignalFrameCompletion()`에서 올바른 다음 인덱스로 설정됐기 때문이다.
만약 여기서도 인덱스를 바꾸면, 알로케이터와 렌더 타깃이 잘못된 인덱스를 보게 된다.

---

### 4.4 `MoveToNextFrame()` — Signal + Wait + 인덱스 전진을 한 번에

```cpp
void GraphicDevice_DX12::MoveToNextFrame()
{
    const UINT64 currentFenceValue = mFenceLastSignalValue + 1;
    mCommandQueue->Signal(mFence.Get(), currentFenceValue);
    mFenceLastSignalValue = currentFenceValue;

    mFrameContext[mFrameIndex].FenceValue = currentFenceValue;

    // 새 백버퍼 인덱스로 전진
    mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

    // 새 프레임의 알로케이터가 아직 GPU 중이면 여기서 바로 대기
    if (mFence->GetCompletedValue() < mFrameContext[mFrameIndex].FenceValue)
    {
        mFence->SetEventOnCompletion(mFrameContext[mFrameIndex].FenceValue, mFenceEvent);
        WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
    }
}
```

**Game-only 전용 함수**다. Signal 후 즉시 인덱스를 전진하고, 필요하면 그 자리에서 Wait한다.
GPU가 빠르면(완료 값이 충분히 높으면) Wait 없이 통과한다.

```
GPU 빠를 때:  Signal ─→ 인덱스 전진 ─→ GetCompletedValue 확인 ─→ 조건 거짓 ─→ 통과
GPU 느릴 때:  Signal ─→ 인덱스 전진 ─→ GetCompletedValue 확인 ─→ 조건 참  ─→ 대기
```

---

## 5. #ifdef WITH_EDITOR 구분의 핵심

이제 모든 조각이 갖춰졌다. 왜 두 경로가 다른지 이해할 수 있다.

### 5.1 전체 프레임 흐름 비교

```
=== Game-only 경로 ===

Loop {
    application.Run()            // Allocator Reset, CommandList Reset
                                 // PRESENT→RT 배리어
                                 // 게임 오브젝트 Draw 기록
    application.CloseCommandList() // RT→PRESENT 배리어, CommandList Close
    application.ExcuteCommandList() // GPU에 제출
    application.Present()        // 화면 출력
    application.MoveToNextFrame() // Signal + 인덱스 전진 + 필요시 Wait
}


=== Editor 경로 ===

Loop {
    application.WaitForNextFrameResources() // ① Allocator 안전 확인 후 Wait
    application.Run()                       // ② Allocator Reset, CommandList Reset
                                            //    게임 오브젝트 Draw 기록
    EditorApplication::Run()               // ③ ImGui Draw 기록
      → ImguiEditor::End()                 //    RT→PRESENT 배리어, CommandList Close
    application.ExcuteCommandList()        // ④ GPU에 제출
    EditorApplication::UpdatePlatformWindows() // ⑤ 멀티뷰포트 처리
    application.Present()                  // ⑥ 화면 출력
    application.SignalFrameCompletion()    // ⑦ Signal + 인덱스 전진 (Wait는 다음 프레임 ①에서)
}
```

### 5.2 차이 1: CommandList를 누가 닫는가

```
Game-only:

  application.Run()
    ├── ResetAllocator()
    ├── ResetCommandList()
    ├── PRESENT → RT 배리어
    └── Draw 명령 기록

  application.CloseCommandList()   ← 게임 코드가 명시적으로 닫음
    ├── RT → PRESENT 배리어
    └── commandList->Close()


Editor:

  application.Run()
    ├── ResetAllocator()
    ├── ResetCommandList()
    ├── PRESENT → RT 배리어
    └── Draw 명령 기록
    [CommandList는 열린 채로 남아 있음]

  EditorApplication::Run()
    └── ImguiEditor::End()
          ├── ImGui_ImplDX12_RenderDrawData()  ← ImGui 커맨드 추가
          ├── RT → PRESENT 배리어             ← ImGui가 직접 배리어 삽입
          └── commandList->Close()            ← ImGui가 닫음
```

ImGui는 자신의 렌더 데이터를 CommandList에 추가한 **직후**에 RT→PRESENT 배리어를 삽입해야 한다.
게임 코드가 먼저 `CloseCommandList()`로 닫아버리면, ImGui가 그 이후에 배리어를 추가할 방법이 없다.
그래서 Editor 경로에서는 CommandList를 열어둔 채로 ImGui에게 넘기고,
ImGui가 마지막 배리어와 Close를 처리한다.

### 5.3 차이 2: Wait 타이밍 — 파이프라이닝

```
Game-only (프레임 끝에 대기):

  시간 →  [N프레임 CPU 작업] [GPU 제출] [GPU 처리] [Wait]  [N+1프레임 CPU 작업]
                                                      ↑
                                                 CPU가 여기서 GPU 끝날 때까지 멈춤
                                                 GPU가 쉬는 시간 발생

Editor (프레임 시작에 대기):

  시간 →  [Wait] [N프레임 CPU 작업] [GPU 제출]
                                             ↓
                        [N+1프레임 Wait]    [GPU 처리 중...]   [GPU 완료]
                              ↑
                         GPU가 처리하는 동안 CPU도 게임 로직 준비 시작
                         겹치는 구간 발생 → 처리량 향상
```

Editor 경로는 `SignalFrameCompletion()`으로 Signal만 먼저 보내고,
실제 Wait는 다음 프레임의 `WaitForNextFrameResources()`에서 한다.
이 덕분에 CPU가 ImGui 레이아웃 계산, 게임 로직 업데이트를 GPU의 렌더링과 **겹쳐서** 처리할 수 있다.

Game-only 경로는 `MoveToNextFrame()`에서 Signal과 Wait를 한 번에 처리한다.
게임 단독 실행에서는 ImGui 멀티뷰포트 같은 복잡한 후처리가 없어 구조가 단순한 게 더 낫다.
코드도 짧고 흐름이 명확하다.

### 5.4 차이 3: UpdatePlatformWindows

ImGui는 "다중 뷰포트(Docking)" 기능을 지원한다. 씬 뷰, 게임 뷰, 인스펙터 창이 메인 윈도우 밖으로
떨어져 나와 독립적인 OS 창이 될 수 있다. `UpdatePlatformWindows()`는 그 창들을 각각 Present한다.

```
Without UpdatePlatformWindows:
  메인 윈도우만 Present

With UpdatePlatformWindows:
  메인 윈도우 Present
  뷰포트 창 A Present
  뷰포트 창 B Present
  ...
```

이 처리는 반드시 메인 `Present()` 직전에 이루어져야 한다. 즉 Game-only 경로에는
존재하지 않는 Editor 전용 개념이다.

---

## 6. mFrameIndex의 생명주기

`mFrameIndex`는 엔진 전체에서 "지금 어느 백버퍼를 쓰고 있는가"를 알려주는 유일한 기준이다.

```
mRenderTargets[mFrameIndex]          ← 지금 그릴 렌더 타깃
mFrameContext[mFrameIndex].Allocator ← 지금 쓸 명령 알로케이터
mRenderTragetDesciptorHandle[mFrameIndex] ← 지금 바인딩할 RTV 핸들
```

이 인덱스가 잘못되면 알로케이터와 렌더 타깃이 어긋나고,
GPU가 아직 처리 중인 알로케이터를 Reset하려다 크래시가 난다.

```
Editor 경로에서 mFrameIndex 변경 시점:

  [WaitForNextFrameResources]  ← mFrameIndex 읽기만 함 (변경 금지)
  [Run → ImGui → Execute → Present]
  [SignalFrameCompletion]      ← 여기서만 mFrameIndex 갱신
                                  (Present 이후에 GetCurrentBackBufferIndex())


Game-only 경로에서 mFrameIndex 변경 시점:

  [Run → CloseCommandList → Execute → Present]
  [MoveToNextFrame]            ← 여기서만 mFrameIndex 갱신
```

두 경로 모두 **Present 이후**에만 mFrameIndex를 바꾼다는 공통 원칙이 있다.
Present()는 스왑 체인에게 "이번 프레임 끝났다, 버퍼 뒤집어"를 알리는 신호다.
그 이후에야 `GetCurrentBackBufferIndex()`가 다음 프레임의 올바른 인덱스를 반환한다.

---

## 7. 버그 히스토리: 무엇이 잘못됐었는가

이 엔진은 Editor 경로를 구현하면서 여러 동기화 버그를 겪었다.
그 과정을 정리하면 왜 이 구조가 필요한지 더 잘 이해된다.

### 버그 1: WaitForNextFrameResources에서 mFrameIndex를 직접 증가시킴

```cpp
// 잘못된 코드 (수정 전)
FrameContext* GraphicDevice_DX12::WaitForNextFrameResources()
{
    mFrameIndex = (mFrameIndex + 1) % 2;  // ← 이게 문제였다
    FrameContext* frameCtx = &mFrameContext[mFrameIndex];
    // ...
}
```

이 함수는 프레임 **시작**에 호출된다. 그런데 여기서 mFrameIndex를 바꿔버리면,
그 직후 `ResetCommandAllocator()`가 잘못된 알로케이터를 Reset한다.
스왑 체인은 아직 Present 전이라 인덱스가 맞지 않는다.

**수정:** mFrameIndex 갱신을 `SignalFrameCompletion()`(Present 이후)으로 이동.
`WaitForNextFrameResources`는 mFrameIndex를 읽기만 한다.

### 버그 2: Initialize에서 FenceValue를 0이 아닌 1로 시작함

```cpp
// 잘못된 코드 (수정 전)
void GraphicDevice_DX12::Initialize()
{
    // ...
    mFenceLastSignalValue++;  // ← 0 → 1로 만들어버림
}
```

`mFrameContext[i].FenceValue`는 0이면 "이 알로케이터는 GPU 작업이 없다(Wait 불필요)"를
의미하기로 약속됐다. 그런데 카운터를 1로 만들면, WaitForNextFrameResources에서
FenceValue=0인 컨텍스트도 "혹시 1 이상인가?"를 체크하게 되고 타이밍이 꼬인다.

**수정:** Initialize에서 `mFenceLastSignalValue++` 제거. 모두 0에서 시작.

### 버그 3: WaitForNextFrameResources가 ImguiEditor::End() 안에 있었음

```
잘못된 흐름:
  application.Run()          ← Allocator Reset
  EditorApplication::Run()
    → ImguiEditor::End()
        → WaitForNextFrameResources()  ← Reset 이후에 Wait!
```

Allocator를 Reset한 **이후**에 "이 Allocator가 GPU에서 안전한가"를 확인하는 것은
순서가 완전히 뒤바뀐 것이다. GPU가 아직 이전 프레임 커맨드를 처리하는 중에
Allocator를 이미 덮어쓴 상황이 된다.

**수정:** `WaitForNextFrameResources()`를 `main.cpp`의 프레임 루프 **맨 앞**으로 이동.
Allocator Reset보다 반드시 먼저 실행.

---

## 8. 전체 그림으로 보기

```
┌─────────────────────────────────────────────────────────────────────┐
│                      Frame N 타임라인                                │
│                                                                      │
│  CPU │                                                               │
│      │ WaitForNextFrame  Run()    ImGui   Execute  Present  Signal   │
│      │ ─────────────────[░░░░░░░░][░░░░░░][░░░░░░][░░░░░░][░░░░░░]  │
│      │         ↑                                                ↑    │
│      │   Wait 완료                                          mFrameIndex 갱신
│      │                                                               │
│  GPU │                                                               │
│      │          [░░░░░░░░░░░░░░░░░░░░░░░░░░]                        │
│      │                이전 프레임 처리 중         ↑                  │
│      │                                      새 프레임 처리 시작      │
│      │                                                               │
│      ↓ 시간                                                          │
│                                                                      │
│ Frame N+1:                                                           │
│      │ WaitForNextFrame  Run()    ImGui   Execute  Present  Signal   │
│      │ ─────────────────[░░░░░░░░][░░░░░░][░░░░░░][░░░░░░][░░░░░░]  │
│      │         ↑                                                     │
│      │   N프레임 fence 대기                                          │
│                                                                      │
│      ← CPU와 GPU가 겹쳐서 동작하는 구간 →                            │
└─────────────────────────────────────────────────────────────────────┘
```

`SignalFrameCompletion()`이 다음 프레임의 `WaitForNextFrameResources()`까지
Wait를 미루기 때문에, CPU가 ImGui UI를 구성하고 게임 로직을 업데이트하는 시간 동안
GPU는 이전 프레임의 렌더링을 처리할 수 있다. 이 겹치는 구간이 더블 버퍼링의 핵심 이득이다.

---

## 9. 정리

| 항목 | Game-only | Editor |
|------|-----------|--------|
| CommandList Close | `application.CloseCommandList()` | `ImguiEditor::End()` |
| Wait 시점 | 프레임 끝 (`MoveToNextFrame`) | 프레임 시작 (`WaitForNextFrameResources`) |
| Signal 시점 | `MoveToNextFrame` (Wait와 함께) | `SignalFrameCompletion` (Wait 분리) |
| mFrameIndex 갱신 | `MoveToNextFrame` | `SignalFrameCompletion` |
| UpdatePlatformWindows | 없음 | Present 직전 |
| CPU-GPU 겹침 | 낮음 | 높음 |

두 경로를 하나로 합칠 수 없는 이유는, ImGui가 CommandList Close의 책임을 가져가기 때문이다.
게임 코드가 먼저 닫으면 ImGui가 배리어를 추가할 수 없고,
ImGui가 없는 Game-only 경로에서는 게임 코드가 직접 닫아야 한다.
이 구조적 차이 때문에 `#ifdef WITH_EDITOR`로 두 경로를 명확히 분리하는 것이 가장 깔끔한 해법이다.
