e=Edit(37,'이 글은 Texture·RT 구현 전의 DX12 상수 버퍼·인덱스 버퍼 복구 단계입니다. 당시 문제와 해결 원리를 보존하고, 완료된 후속 작업은 마지막 표에서 연결합니다.','4개 정점과 6개 인덱스가 DrawIndexedInstanced로 연결되고 TransformCB가 root b0로 전달되는 구조를 이해합니다.')
e.para('그 결과 셰이더는 초기화되지 않은 메모리를','당시에는 계산한 Transform 데이터가 의도한 GPU 바인딩으로 전달되지 않았습니다. 화면에 삼각형이 보였다는 사실만으로 TransformCB가 정상이라고 판단할 수 없습니다. 초기화되지 않은 메모리를 실제로 읽었는지와 구체적인 픽셀 결과는 셰이더·바인딩·검증 로그를 함께 확인해야 합니다.')
e.para('힙은 크게 세 가지가 있다.','이 단계에서 사용하는 기본 Heap 타입은 DEFAULT·UPLOAD·READBACK입니다. DEFAULT는 GPU 작업 중심, UPLOAD는 CPU 쓰기·GPU 읽기, READBACK은 GPU 복사 결과의 CPU 읽기에 사용합니다. 실제 메모리 배치와 성능은 GPU 아키텍처에 따라 달라집니다.')
e.para('또 하나 중요한 제약이 있다.','CBV 주소와 크기는 DX12의 256바이트 정렬 규칙을 맞춥니다. TransformCB의 행렬 3개는 192바이트이고 한 Draw용 구간은 256바이트로 올림합니다. HLSL 패킹의 16바이트 규칙과 구분하며 특정 하드웨어 캐시 라인 크기라고 단정하지 않습니다.')
e.para('지금이야 꼭짓점이 4개 vs 6개의 차이지만','정점 공유가 많은 메시에서는 인덱스 방식이 메모리와 정점 처리량을 줄일 수 있습니다. 절약량을 계산할 때는 인덱스 버퍼 크기도 포함하고, UV·노멀 경계 때문에 정점을 분리해야 하는 경우도 고려합니다.')
e.block(1,'''// 당시 Persistent Map의 개념 예시입니다.
const D3D12_RANGE noCpuRead{0, 0};
ThrowIfFailed(buffer->Map(0, &noCpuRead, &mMappedData));
// GPU가 아직 읽고 있는 주소를 덮어쓰면 안 됩니다.
std::memcpy(mMappedData, data, mSize);
// 여러 Draw/여러 프레임에서 사용하려면 서로 다른 구간을 할당해야 합니다.
// 현재 구현은 프레임별 64KiB 페이지와 Draw별 256바이트 구간으로 확장됐습니다.''')
for n in (4,5):e.block_replace(n,'...', 'D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT')
e.block_replace(12,'[프레임 시작]','[당시 프레임 흐름 — 아래 후속 구현 표와 함께 읽기]\n[재사용할 프레임 리소스의 GPU 완료 확인 후 시작]')
e.block_replace(12,'← PSO가 커맨드 리스트에 자동 바인딩','← Reset에 전달한 초기 PSO 사용')
a=e.body.index('## 8. 남은 과제')
rewrite_tail(e,'## 8. 남은 과제','''## 8. 당시 남은 과제와 현재 완료한 작업

위 그림과 임시 vertex color 셰이더 설명은 **Texture 구현 전 시점**의 기록입니다. 다음 작업들은 후속 커밋에서 진행했습니다.

| 당시 과제 | 현재 구현과 이어 읽을 글 |
| --- | --- |
| Texture DX12 생성·업로드·SRV | DEFAULT 컬러 리소스와 업로드·descriptor 할당 구현. 후속 1편 |
| RenderTarget | Scene/Game 컬러·깊이 RT와 resize 구현. 후속 1편 |
| 카메라 출력 분리 | Game RT와 EditorCamera RT 분리. 등록 게임 카메라별 RT 자동 생성은 아님 |
| ImGui Image 연결 | DX12 GPU SRV handle로 표시. 후속 1편 |
| 상수 버퍼 구간과 수명 | 같은 프레임의 Draw와 동시에 실행 중인 프레임 사이의 덮어쓰기 방지. 후속 1편 |
| Shader별 PSO | Shader가 상태 조합별 PSO 캐시 소유, Material 모드에 맞게 선택. 후속 2편 |
| 렌더 순서·표시 알파·지연 해제 | Opaque/CutOut/Transparent 복구와 display SRV, 최종 Fence 기준 반환. 후속 2편 |

후속 1편: <mention-page url="'''+NEW1+'''"/>
후속 2편: <mention-page url="'''+NEW2+'''"/>
''')
e.finish()

e=Edit(38,'CPU 기록·GPU 실행·표시의 시점을 나눠 Fence를 이해합니다. 현재 ce56f16 기준의 함수와 이전 버그 기록을 함께 두고, Scene/Game 및 descriptor 수명 관리로 연결합니다.','대기 대상이 방금 제출한 프레임이 아니라 재사용할 슬롯의 마지막 Fence임을 이해하고 Allocator Reset 전에 확인합니다.')
e.para('Fence는 CPU와 GPU 사이에 놓인','이 엔진에서 Fence는 Queue가 어디까지 완료했는지 나타내는 값입니다. CPU 대기와 Queue 간 동기화에도 사용할 수 있으므로 본질적으로 단방향 깃발만 가능한 객체는 아닙니다.')
e.para('왜 단조 증가(**이전 값보다 반드시 큰 값으로만 증가)','이 엔진은 완료 값을 비교하는 타임라인을 사용하므로 Signal 값을 단조 증가시킵니다. API가 작은 값의 Signal을 받으면 GPU가 혼란에 빠진다는 뜻이 아닙니다. 값이 되감기거나 여러 주체가 일관성 없이 Signal하면 어떤 작업의 완료를 뜻하는지 추론하기 어려워집니다.')
e.para('\t- CPU는 명령을 던지는 속도가','\t- CPU와 GPU는 독립적으로 진행합니다. Fence는 완료 지점을 연결하고, Resource Barrier는 GPU 리소스 접근의 상태·순서를 연결합니다. 서로 다른 역할입니다.')
e.para('`mFenceEvent`는 Windows의 Event','mFenceEvent는 Win32 Event 객체입니다. SetEventOnCompletion으로 지정한 값에 도달하면 OS가 이벤트를 신호 상태로 만들고 대기가 풀립니다. 호출 결과와 WAIT_FAILED도 확인해야 하며 구체적인 하드웨어 인터럽트 경로까지 가정할 필요는 없습니다.')
e.para('**언제 쓰는가?** 초기화 시점','**언제 쓰는가?** 초기화 업로드, 종료, 리사이즈 등 해당 Queue의 앞선 사용 완료가 필요할 때입니다. 이 함수가 Signal하기 전에 Queue에 제출된 작업까지 기다립니다. 매 프레임 전체 완료 대기를 넣으면 병렬성이 줄 수 있지만 고정 50% 성능 저하가 보장되는 것은 아닙니다.')
e.para('**Editor 전용 함수**다.','현재 Editor 경로에서 사용하는 함수입니다. Signal을 제출하고 CPU는 다음 작업으로 진행합니다. 다음 프레임 시작에는 **그때 재사용할 FrameContext의 이전 Fence 값**을 기다립니다. 방금 제출한 Fence를 매번 곧바로 기다리는 구조가 아닙니다.')
e.para('이것을 기다리면 스왑 체인 내부 큐가','Frame Latency Waitable Object는 표시 지연을 제어하는 조건입니다. 허용할 선행 프레임 수는 SetMaximumFrameLatency 등 실제 설정과 함께 봅니다. Fence 완료와 별개이며 무조건 두 프레임이라는 고정 규칙은 아닙니다.')
e.para('Editor 경로는 `SignalFrameCompletion()`으로 Signal만 먼저 보내고,','Editor는 최종 제출을 Signal한 뒤 다음 슬롯의 사용 조건을 다음 루프 시작에 확인합니다. CPU와 GPU의 겹침은 여러 슬롯을 돌려 쓰는 데서 생기며, 단순히 대기 호출을 함수 앞뒤로 옮겼기 때문만은 아닙니다.')
e.para('Game-only 경로는 `MoveToNextFrame()`에서 Signal과 Wait를','Game-only의 MoveToNextFrame도 **다음 슬롯에 남아 있는 이전 사용**만 기다립니다. 따라서 이 경로도 CPU/GPU 파이프라이닝이 가능합니다. 차이는 ImGui 추가 창 제출과 Command List 종료의 책임, 대기를 배치한 위치입니다.')
e.para('ImGui는 "다중 뷰포트(Docking)"','ImGui의 Docking은 탭과 도킹 영역 배치, Multi-Viewport는 추가 OS 창 지원입니다. 현재 UpdatePlatformWindows 래퍼는 UpdatePlatformWindows와 RenderPlatformWindowsDefault를 호출해 추가 창의 생성·렌더링·Present를 처리합니다.')
e.para('이 처리는 반드시 메인 `Present()` 직전에','이 엔진은 추가 창 렌더링을 메인 Present 전에 수행합니다. 모든 엔진에서 이 위치만 가능한 API 규칙은 아닙니다. 현재 중요한 조건은 같은 Queue의 추가 창 제출을 포함한 뒤 최종 Fence를 Signal하는 것입니다.')
e.para('`mFrameContext[i].FenceValue`는 0이면','FrameContext.FenceValue=0은 이 코드에서 아직 대기할 사용이 없다는 약속입니다. mFenceLastSignalValue가 1로 시작한다고 0인 FrameContext가 자동으로 1이 되는 것은 아닙니다. 초기 완료 값, 다음 Signal 값, 슬롯에 저장한 값을 일관되게 관리해야 하며, 아직 Signal하지 않은 값을 기다리면 멈출 수 있습니다. 당시 수정은 불필요한 카운터 증가를 제거한 것이고 그 증가만으로 위 문제가 필연적으로 발생한다고 설명하지 않습니다.')
e.para('<td>낮음</td>','<td>가능 — 다음에 재사용할 슬롯만 대기</td>')
e.para('<td>높음</td>','<td>가능 — 다음에 재사용할 슬롯과 표시 지연 조건 대기</td>')
e.para('두 경로를 하나로 합칠 수 없는 이유는','현재 두 경로는 마지막 Draw를 기록하는 주체가 달라 Close 책임을 나눕니다. 게임·UI 패스가 모두 끝난 뒤 공통 코드에서 최종 barrier와 Close를 수행하도록 리팩토링하는 것도 가능합니다. WITH_EDITOR 분기는 현재 선택한 구현이며 두 경로를 통합하는 것이 원천적으로 불가능한 것은 아닙니다.')
e.block(0,'mCommandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0); // 명령 기록')
e.block(1,'Draw 호출 → Command List에 기록\nExecuteCommandLists → Queue에 제출\nGPU → 앞선 의존 작업과 실행 순서에 따라 비동기로 실행\nCPU → 제출 호출이 반환돼도 GPU 완료를 뜻하지 않음','plain text')
e.block(5,'SwapChain의 백버퍼 2개\nBackBuffer[0], BackBuffer[1]\nGetCurrentBackBufferIndex()가 지금 그릴 대상을 알려줍니다.\n0번이 항상 화면 표시용, 1번이 항상 렌더링용으로 고정된 것은 아닙니다.','plain text')
e.block_replace(6,'UINT64 FenceValue;','UINT64 FenceValue = 0;')
e.block(11,'2개 슬롯을 사용하는 예\nN프레임: 슬롯0 사용 → Fence 10 저장\nN+1프레임: 슬롯1의 이전 Fence 대기 → Fence 11 저장\nN+2프레임: 슬롯0 재사용 전 Fence 10 완료 확인\n프레임 번호와 Fence 번호가 항상 같은 값일 필요는 없습니다.','plain text')
e.block(21,'Game-only: 제출 → Present → Signal → 다음 슬롯의 이전 Fence 대기\nEditor: 제출 → 추가 창 → Present → Signal → 다음 루프에서 다음 슬롯의 이전 Fence 대기\n두 경로 모두 여러 슬롯이 있으면 CPU/GPU 작업을 겹칠 수 있습니다.\n차이는 UI·플랫폼 창 제출과 표시 지연 조건을 어디서 관리하는지입니다.','plain text')
e.block(22,'메인 Command List 제출\n→ 추가 플랫폼 창의 렌더링·Present\n→ 메인 SwapChain Present\n→ 같은 Queue의 최종 Fence Signal','plain text')
e.block(28,'프레임 시작: 재사용할 슬롯 Fence + 표시 지연 조건 확인\nCPU: 게임 로직 → Game RT → Scene RT → ImGui 합성 기록\n제출: 메인 목록 → 추가 플랫폼 창 → Present → 최종 Signal\n프레임 끝: Scene 이벤트 처리\nGPU: 제출된 명령을 순서·의존성에 맞게 실행\n리소스 재사용: 해당 슬롯/descriptor의 이전 GPU 사용 완료 후','plain text')
for n,signature in [(8,'void GraphicDevice_DX12::WaitForGpu('),(10,'void GraphicDevice_DX12::SignalFrameCompletion('),(12,'FrameContext* GraphicDevice_DX12::WaitForNextFrameResources('),(17,'void GraphicDevice_DX12::MoveToNextFrame(')]:
    code,line=source_function('YamYamEngine_CORE/yaGraphicDevice_DX12.cpp',signature)
    e.block(n,code,caption=f'ce56f16 실제 코드 · yaGraphicDevice_DX12.cpp:{line}')
# Nested callout diagrams use direct replacements to preserve their block layout.
e.replace('게임 전용 경로(MoveToNextFrame)는 fence만 쓰므로 프레임 큐가 쌓일 수 있으나\n게임에서는 1~2프레임 지연이 허용 범위 안에 있어 실용상 문제없다.','Game-only도 입력 지연 요구를 측정해 별도 프레임 pacing을 선택할 수 있습니다.\n게임이면 1~2프레임 지연이 항상 문제없다고 가정하지 않습니다.')
e.finish('### 이 글 다음에 추가한 구현\n\n현재 함수에는 CollectRetiredResources와 SealRetiredResources가 연결돼 있습니다. 이는 Texture·RT·descriptor의 GPU 수명을 관리하는 후속 작업입니다. 위 실제 코드의 일부 Signal/Wait 반환값 검사는 여전히 개선할 부분이며, 문서 검수에서 엔진 소스를 수정한 것은 아닙니다. <mention-page url="'+NEW1+'"/> <mention-page url="'+NEW2+'"/>')
