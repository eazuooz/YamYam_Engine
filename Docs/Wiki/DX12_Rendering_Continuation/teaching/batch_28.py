from edit import Edit
e=Edit(28)
e.intro('''## 화면을 지우는 한 프레임에 필요한 것부터 준비합니다

DX12 초기화에는 낯선 객체가 한꺼번에 등장합니다. 이번에는 “창을 파란색으로 지우고 화면에 보여 준다”는 작은 목표를 기준으로 역할을 나누겠습니다. 사용할 GPU를 고르고, 작업을 적을 공간을 만들고, 결과를 담을 백버퍼를 얻은 뒤, GPU가 작업을 마친 시점을 확인해야 합니다.

아래 코드는 각 객체의 생성 지점을 순서대로 설명하는 Win32 학습 예제입니다. 유효한 HWND와 양수인 클라이언트 크기는 미리 준비합니다. 도형을 그리는 PSO와 셰이더는 다음 강의에서 연결합니다.''')
e.replace('Directx12 를 사용하려면 GPU에 접근하는 데이터들을 정확한 구조로 만들어줘야 한다.', 'Factory → Adapter → Device는 “어느 GPU를 사용할지” 준비하는 흐름입니다. Device 이후에는 명령을 위한 Queue·Allocator·List, 표시를 위한 SwapChain·RTV, 재사용을 위한 Fence로 갈라집니다. 아래 그림에서 생성 관계를 먼저 보고 세부 코드로 내려가 봅시다.')
e.code(0,after='''Factory는 GPU와 화면 관련 DXGI 객체를 찾고 만들 때 사용하는 입구입니다. 예제는 IDXGIFactory4만으로 필요한 기능을 사용하며 더 높은 번호의 인터페이스가 항상 필요한 것은 아닙니다. ComPtr은 COM 참조를 소유합니다. Check는 HRESULT 실패를 놓치지 않도록 한 곳에서 처리합니다.

디버그 레이어는 Device를 만들기 **전에** 켭니다. 생성 뒤 켜는 옵션처럼 생각하면 안 됩니다. Debug 빌드에서 D3D12GetDebugInterface가 실패하면 Windows Graphics Tools 설치 상태를 확인합니다. 이 예제는 실패를 무시하고 불완전한 초기화를 계속하지 않습니다.''')
e.code(1,after='''후보를 하나씩 꺼내 소프트웨어 어댑터를 건너뛰고, D3D12CreateDevice의 출력 포인터를 nullptr로 호출해 지원 여부를 검사합니다. 성공한 후보를 adapter에 보관한 뒤 다음 절에서 실제 Device를 만듭니다. 출력 포인터가 없는 성공은 생성된 Device 객체를 받은 것이 아니라 지원 조건을 만족한다는 뜻입니다.

여기서 Feature Level 11_0은 GPU에 요구하는 기능 수준입니다. API가 Direct3D 12라는 사실과 기능 수준에 12_0을 요구하는 것은 다릅니다. DedicatedVideoMemory가 0인 통합 GPU도 유효할 수 있으므로 메모리 수치만으로 탈락시키지 않습니다. 하드웨어 대신 WARP를 쓰려면 EnumWarpAdapter를 선택하는 별도 경로를 둡니다.''')
e.replace('디버그 디바이스를 만들엇 사용하면 그래픽 디버그 모드를 사용 할수도 있습니다. 이를 통해서 그래픽 작업 관련해서 잘못 된 점들을 찾아서 고칠수 있습니다.', '위 호출로 선택한 어댑터에 대한 Device를 얻었습니다. 이 Device로 Queue·버퍼·텍스처·PSO를 만듭니다. 아래 As는 새 GPU를 만드는 호출이 아니라 같은 객체의 디버그 인터페이스를 요청하는 QueryInterface입니다. 디버그 레이어 활성화와 구별합니다.')
e.code(4,after='''처음에는 DIRECT Queue 하나를 사용합니다. 삼각형 Draw뿐 아니라 필요한 복사 명령도 같은 큐에서 실행할 수 있기 때문입니다. Queue를 만들었다고 명령이 자동 생성되지는 않습니다. CPU가 Command List에 기록하고 Close한 뒤 ExecuteCommandLists로 전달해야 합니다. 다른 큐를 추가할 때는 결과를 주고받는 작업 사이의 순서도 연결해야 합니다.''')
e.code(5,after='''Allocator는 명령 기록용 저장 공간이고 List는 그 공간에 기록하는 인터페이스입니다. 종이와 필기 도구에 비유하면 두 객체를 구별하기 쉽습니다. List에 Clear나 Draw를 기록하고 Close해서 제출할 준비를 합니다. 생성 직후에는 열린 상태이므로 프레임마다 Reset하는 구조라면 초기화를 끝낼 때 한 번 Close해 둡니다.

다음 프레임에 Allocator를 Reset한다는 것은 이전 명령을 담던 공간을 다시 쓸 수 있게 하는 일입니다. GPU가 아직 그 명령을 읽는 중이면 지워서는 안 됩니다. 이 조건을 확인하기 위해 다음 Fence가 필요합니다.''')
e.code(6,after='''Fence를 0으로 만들고 다음 제출에는 1부터 값을 붙입니다. 예를 들어 Queue에 작업을 제출한 뒤 Signal(fence, 7)을 넣었다면, GetCompletedValue가 7 이상일 때 그 Signal 앞의 작업이 끝났다고 판단할 수 있습니다. CPU가 가진 nextFenceValue를 7로 바꾸는 것만으로 GPU 완료가 생기지는 않습니다.

실제 CPU 대기는 SetEventOnCompletion으로 완료할 값을 지정하고 이벤트를 기다리는 경로로 연결합니다. 대기 후 Allocator·업로드 메모리 등을 재사용합니다. 여러 프레임을 동시에 진행할 때는 각 프레임 슬롯에 그 슬롯의 마지막 제출 Fence 값을 따로 저장합니다.''')
e.code(7,after='''백버퍼는 표시용 PRESENT 상태에서 색상을 쓰는 RENDER_TARGET 상태로 바꾼 뒤 Clear/Draw합니다. 기록이 끝나면 반대 방향으로 바꿔 화면 표시를 준비합니다. 이 코드는 실제 backBuffer를 뒤 절에서 얻은 **후**, 프레임 명령 기록 안에서 실행합니다. 초기화 코드를 읽는 순서 그대로 아직 없는 backBuffer를 참조하는 것은 아닙니다.

Barrier는 GPU 명령 사이의 상태 전환을 적습니다. 그 명령을 CPU가 적었다고 GPU가 실행을 끝낸 것은 아니므로, Allocator 재사용을 위한 Fence 확인을 대신하지 못합니다.''')
e.code(8,after='''SwapChain 생성에 Device가 아니라 commandQueue를 넘기는 점이 DX11과 다릅니다. 백버퍼 두 개를 만들고 현재 그릴 index를 얻습니다. GetBuffer로 받은 것은 실제 이미지 리소스이며, CreateRenderTargetView는 그 이미지에 출력할 설명을 RTV heap 슬롯에 씁니다.

handle.ptr에는 픽셀 데이터 주소가 아니라 descriptor 위치가 들어 있습니다. 다음 RTV로 이동할 때 sizeof 같은 임의 크기를 더하지 않고 Device가 알려 준 descriptor stride를 더합니다. RTV heap은 shader-visible로 만들지 않습니다. OMSetRenderTargets는 이 CPU handle을 받습니다.

viewport의 MinDepth=0, MaxDepth=1은 렌더링 깊이 범위입니다. 카메라 near/far 거리 0.1과 1000을 여기에 넣는 것이 아닙니다. scissor는 그리기를 허용할 사각형 범위입니다. flip-model 백버퍼는 SampleDesc.Count=1을 사용하고 MSAA가 필요하면 별도 타깃에서 resolve합니다.''')
e.tail('### 초기화 다음 단계','''## 준비한 객체로 한 프레임을 조립합니다

이제 다음 순서로 Clear만 실행할 수 있습니다. 이전 GPU 사용이 끝난 Allocator와 List를 Reset하고, 현재 백버퍼를 RENDER_TARGET으로 전환합니다. RTV를 지정해 색을 지운 뒤 PRESENT로 되돌리고 List를 Close합니다. Queue에 제출하고 Present한 뒤 Fence 값을 Signal합니다. 처음에는 그 완료를 기다린 다음 다음 프레임을 시작하면 동작을 이해하기 쉽습니다.

```mermaid
flowchart LR
    A[이전 Fence 완료 확인] --> B[Allocator와 List Reset]
    B --> C[백버퍼 상태 전환·Clear]
    C --> D[PRESENT 전환·Close]
    D --> E[Queue 제출·Present]
    E --> F[Fence Signal]
    F --> A
```

화면에 Clear 색이 보이면 GPU 선택, 명령 제출, 출력 타깃, 표시 경로가 연결된 것입니다. 삼각형이 아직 없어도 초기화 전체를 작은 결과로 검증할 수 있습니다. 종료할 때도 마지막 GPU 사용을 기다린 뒤 리소스를 놓고 CPU 이벤트 핸들을 닫습니다. 상세한 Draw와 두 프레임 겹치기는 이후 강의에서 이 경로 위에 추가합니다.

[Microsoft: 명령 목록과 Allocator의 기록·재사용](https://learn.microsoft.com/en-us/windows/win32/direct3d12/recording-command-lists-and-bundles)
<mention-page url="https://www.notion.so/1bd0b1ffa61e8097b9d6dda7ad3704a0"/>''')
e.save()
