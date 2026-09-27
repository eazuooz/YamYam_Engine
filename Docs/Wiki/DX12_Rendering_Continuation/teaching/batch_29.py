from edit import Edit
e=Edit(29)
e.intro('''## Clear 화면에 색이 섞인 삼각형을 추가합니다

앞 글에서 GPU를 고르고 명령 목록과 백버퍼를 준비했습니다. 이번에는 위치와 색을 가진 정점 세 개를 GPU에 보내고, 상수 버퍼의 행렬로 삼각형을 움직여 보겠습니다. 화면의 색이 어떻게 생기는지뿐 아니라 **데이터를 저장하는 버퍼 → 그 데이터의 접근 정보 → 셰이더 연결 → Draw 명령**을 하나의 흐름으로 읽는 것이 목표입니다.

코드는 개념별 생성·기록 부분을 발췌한 학습 예제입니다. device, Queue, Allocator, SwapChain, 백버퍼 RTV, viewport, scissor는 앞의 초기화에서 준비한 객체를 사용합니다. 각 생성 블록은 초기화 함수로 나누고, 생성한 리소스·뷰·PSO는 프레임과 종료까지 살아 있는 멤버에 보관합니다. 같은 지역 변수명을 한 함수에 반복 선언하는 형태로 붙이지 않습니다.''')
e.replace('디스크립터 힙은 그래픽 오브젝트에 할당된 오브젝트를 설명을 저장해둔 공간입니다.\nDescriptor Heap은 리소스 내용을 저장하는 Resource Heap과 다릅니다. Descriptor라는 접근 정보를 모아 둡니다. 모든 Descriptor Heap이 shader-visible인 것은 아닙니다.\n"셰이더(Shader)가 사용하는 GPU 리소스들(텍스처, 버퍼 등)의 \\*\\*설명서(Descriptor)\\*\\*를\nCBV/SRV/UAV와 Sampler Heap은 shader-visible로 만들 수 있고, RTV/DSV Heap은 CPU handle로 명령에 지정합니다.', '''위 그림의 heap을 모두 같은 메모리로 생각하면 혼동하기 쉽습니다. Resource에는 실제 정점·색·행렬 데이터가 있고, Descriptor Heap에는 그 Resource를 어떤 범위와 형식으로 읽거나 쓸지 적은 설명이 있습니다. 주소록과 그 주소에 있는 물건이 다른 것처럼, descriptor 자체에 삼각형 정점이 들어 있는 것은 아닙니다.

CBV/SRV/UAV와 Sampler Heap은 shader-visible로 만들어 GPU가 읽게 할 수 있습니다. RTV/DSV는 CPU handle로 출력 명령에 지정합니다. 이번에는 백버퍼의 RTV heap과 셰이더 상수용 CBV heap을 구별해서 사용합니다.''')
e.code(0,after='''이 heap에는 백버퍼 수만큼 RTV 자리를 확보합니다. 생성만으로 각 슬롯이 백버퍼를 가리키지는 않습니다. 앞 글의 GetBuffer와 CreateRenderTargetView로 각 자리를 채운 뒤 OMSetRenderTargets에 현재 프레임의 CPU handle을 전달합니다. 앞에서 이미 RTV heap을 만들었다면 그 객체를 재사용합니다.''')
e.code(1,before='''### 셰이더의 b0와 root parameter 0을 연결합니다

이 예제는 Vertex Shader의 b0 상수 버퍼 하나를 descriptor table로 연결합니다. b0는 HLSL 레지스터 번호이고 root parameter 0은 CPU가 Command List에서 설정할 루트 슬롯 번호입니다. 숫자가 우연히 둘 다 0이지만 같은 번호 체계는 아닙니다.''',after='''range는 b0부터 CBV 한 개를 읽겠다는 범위입니다. parameter는 그 범위를 담은 table 하나이고, ShaderVisibility=VERTEX로 VS에서 사용합니다. 루트 시그니처를 직렬화하고 Device 객체로 만든 뒤 PSO와 Draw 양쪽에서 연결합니다.

여기에는 아직 어떤 상수 버퍼의 주소도 들어 있지 않습니다. Root Signature는 연결 모양을 정하고, 뒤에서 만든 CBV descriptor와 SetGraphicsRootDescriptorTable 호출이 실제 데이터를 선택합니다. RTV는 셰이더 입력 table이 아니라 출력 타깃이므로 이 루트 파라미터에 넣지 않습니다.''')
e.code(2,after='''UPLOAD는 CPU가 데이터를 써서 GPU에 전달할 때, READBACK은 GPU가 복사한 결과를 CPU가 읽을 때 사용하는 메모리 방향입니다. DEFAULT는 GPU 작업용 리소스에 주로 사용합니다. 작은 입문 삼각형은 UPLOAD 버퍼에서 직접 읽게 해 복사 단계를 줄여 이해할 수 있지만, 큰 정적 메시의 최종 배치까지 항상 UPLOAD로 정하라는 뜻은 아닙니다.

Map의 {0,0}은 CPU가 읽지 않겠다는 범위 정보입니다. 반환된 주소에 memcpy로 쓰고 Unmap의 written 범위로 기록 구간을 알립니다. Map/Unmap은 GPU 완료를 기다리는 호출이 아닙니다. CPU가 데이터를 덮어쓰기 전에 이전 GPU 읽기가 끝났는지는 Fence로 별도 확인합니다.''')
e.code(3,before='''### 정점 한 개는 24바이트입니다

위치 float3은 12바이트, 색 float3도 12바이트이므로 Vertex의 stride는 24입니다. 정점 세 개의 데이터는 72바이트입니다. 아래의 Resource는 그 72바이트를 보관하고 VertexBufferView는 시작 GPU 주소·전체 바이트 수·한 정점 간격을 알려 줍니다.''',after='''정점 배열의 첫 물체는 화면 오른쪽 아래에 빨강, 왼쪽 아래에 초록, 위쪽에 파랑을 배치합니다. 지금은 World·View·Projection을 단위 행렬로 시작하므로 입력 좌표가 바로 클립 공간에 도달합니다. VertexBufferView는 descriptor heap의 슬롯이 아니라 IASetVertexBuffers에 값으로 넘기는 뷰 구조체입니다.''')
e.code(4,before='''### 인덱스는 정점 데이터가 아니라 참조 순서입니다

uint32_t 세 개 `[0,1,2]`는 앞 버퍼의 첫째·둘째·셋째 정점으로 삼각형 하나를 만들라는 뜻입니다. 각 값은 좌표가 아니라 정점 번호입니다. 이번 삼각형 하나에서는 메모리 절약 효과가 없지만, 이후 두 삼각형이 정점을 공유하는 메시에서도 같은 Indexed Draw 경로를 사용하려고 연결합니다.''',after='''Format=R32_UINT는 GPU가 인덱스 하나를 4바이트 unsigned 정수로 읽게 합니다. uint16_t 배열로 바꾼다면 Format도 R16_UINT로 맞춰야 합니다. VertexBuffer의 stride와 달리 IndexBufferView는 Format으로 요소 크기를 결정합니다. DrawIndexedInstanced의 첫 인자는 바이트 수 12가 아니라 **인덱스 개수 3**입니다.''')
e.code(5,before='''### 192바이트의 행렬을 256바이트 자리로 배정합니다

4×4 float 행렬은 64바이트입니다. Projection, Model, View 세 개는 192바이트이지만 DX12 CBV의 크기·주소는 256바이트 정렬 조건에 맞춰야 합니다. `(192+255)&~255`는 256이 됩니다. 나중에 다음 Draw의 상수를 놓는다면 같은 주소에 덮는 대신 다음 256바이트 자리로 이동할 수 있습니다.''',after='''C++ 구조체의 순서는 Projection → Model → View이고 바로 아래 HLSL도 같은 순서입니다. CPU 메모리를 복사하는 코드와 셰이더가 해석하는 구조가 다르면 값이 올바르게 연결되지 않습니다. 처음에는 세 행렬을 Identity로 채워 정점 데이터 자체가 보이는지 확인합니다.

CBV는 constantBuffer의 GPU 주소와 256바이트 크기를 가리키고, shader-visible constantBufferHeap의 첫 슬롯에 기록됩니다. CPU handle은 CreateConstantBufferView의 기록 위치이고 GPU handle은 Draw에서 table 시작점을 지정할 때 사용합니다. HLSL의 16바이트 패킹과 DX12 CBV의 256바이트 정렬은 서로 다른 규칙입니다.''')
e.code(6,replacement=e.codes[6].replace('```glsl','```c++').replace('cbuffer cb','// HLSL: Vertex Shader\ncbuffer cb',1),after='''POSITION과 COLOR는 C++ 멤버 이름을 검색하는 문자열이 아닙니다. PSO의 InputLayout이 byte offset 0과 12를 해당 시맨틱으로 연결합니다. VS는 위치를 Model → View → Projection 순서로 변환하고 SV_Position으로 출력합니다. row_major와 행벡터 mul을 맞췄으므로 위 C++ 행렬을 전치하지 않습니다.

색은 VS에서 그대로 넘깁니다. 삼각형 내부의 색이 자연스럽게 섞이는 과정은 이후 래스터라이저의 보간에서 일어납니다. VS가 모든 화면 픽셀을 하나씩 칠하는 구조가 아닙니다.''')
e.code(7,before='''### HLSL 파일을 바이트코드로 준비합니다

아래는 Shader 컴파일 함수 내부입니다. filename은 HLSL 경로, entryPoint는 이 글의 셰이더 함수 이름 main, target은 VS에서 vs_5_0, PS에서 ps_5_0을 전달합니다. 반환된 blob 두 개를 vertexShaderBlob과 pixelShaderBlob으로 보관해 PSO에 넣습니다.''',after='''컴파일러는 Device와 별개입니다. 실패하면 errorBlob의 메시지를 먼저 확인하고 PSO 생성으로 진행하지 않습니다. shader model 6 기능을 사용하는 Mesh Shader 예제는 뒤에서 DXC를 사용하지만, 이 VS/PS 입문 예제는 D3DCompiler 경로로 설명합니다.''')
e.code(8,replacement=e.codes[8].replace('struct PixelInput','// HLSL: Pixel Shader\nstruct PixelInput',1),after='''PS는 보간된 RGB를 받아 알파 1을 붙여 반환합니다. SV_Target0은 출력 슬롯 0을 뜻합니다. Root Signature의 parameter 0이나 HLSL b0와 무관합니다. 이 예제의 PS는 별도 리소스를 읽지 않고 VS에서 전달된 색만 사용합니다.''')
e.code(9,before='''### 이 정점·셰이더·출력 형식을 한 조합으로 만듭니다

PSO에는 InputLayout, Root Signature, VS/PS, Rasterizer, Blend, Depth, 출력 포맷과 샘플 수를 함께 넣습니다. 이 예제는 컬링·깊이를 꺼서 정점 순서나 깊이 설정 때문에 삼각형이 사라지는 변수를 줄입니다. RTV 포맷과 SampleDesc는 실제 백버퍼와 맞아야 합니다.''',after='''POSITION은 offset 0, COLOR는 offset 12이고 둘 다 float3입니다. 이는 앞 Vertex의 24바이트 구조와 일치합니다. PrimitiveTopologyType=TRIANGLE은 PSO의 큰 종류이고, 실제 TRIANGLELIST 선택은 Command List에서 합니다. viewport·scissor·리소스 table 시작점도 Draw마다 설정할 수 있는 상태이며 PSO를 만들었다고 자동으로 정해지지 않습니다.''')
e.code(10,after='''CreateCommandList는 열린 기록 상태로 생성하므로 프레임 함수에서 Reset할 계획이라면 먼저 Close합니다. 앞 초기화에서 만든 List를 사용할 때는 다시 생성하지 않고 그 객체에 같은 초기 Close 조건을 적용합니다. Allocator와 List를 매 프레임 새로 생성하는 예제가 아닙니다.''')
e.code(11,before='''### 주소를 연결한 뒤 실제 Draw를 기록합니다

이 블록은 setupCommands 함수의 본문입니다. surfaceSize는 앞 초기화의 scissor rect, rtvDescriptorSize는 RTV heap의 increment size입니다. 프레임 시작 전에 이전 GPU 사용이 끝났다는 조건을 만족한 후 실행합니다.''',after='''SetDescriptorHeaps는 GPU가 접근할 heap을 고르고, SetGraphicsRootDescriptorTable(0, cbvHandle)은 root parameter 0의 시작 위치를 정합니다. Root Signature에 table을 선언하는 것, heap에 CBV를 만드는 것, Draw에서 시작 handle을 설정하는 것이 이 지점에서 만납니다.

그다음 백버퍼를 쓰기 상태로 전환하고 RTV·viewport·scissor를 지정합니다. VB/IB와 TRIANGLELIST를 설정한 후 인덱스 세 개로 인스턴스 하나를 그립니다. 마지막에 백버퍼를 PRESENT로 되돌리고 Close해야 Queue에 제출할 수 있습니다. Clear만 보인다면 먼저 PSO 생성 성공과 이 Draw 직전의 바인딩을 확인합니다.''')
e.code(12,replacement=e.codes[12].replace('XMMatrixRotationY(angle)','XMMatrixRotationZ(angle)'),before='''### Identity로 확인한 다음 Z축 회전을 넣습니다

render의 첫 실행은 세 행렬을 Identity로 둔 결과부터 확인해도 됩니다. 이어 아래처럼 Model을 Z축 회전으로 바꾸면 삼각형이 화면 평면에서 회전합니다. View·Projection을 단위 행렬로 둔 입문 예제라 Y축으로 회전시키면 일부 정점이 z<0 클립 영역으로 들어갈 수 있어, 처음 실험은 Z축을 사용합니다.''',after='''angle은 프레임마다 고정량을 더하는 대신 실제 경과 초를 더하므로 약 1 radian/초로 증가합니다. 이 프레임 끝에서 GPU 완료를 기다리기 때문에 다음 호출의 상수 버퍼 덮어쓰기와 Allocator Reset이 보호됩니다. 지금은 원리를 보기 쉬운 직렬 실행이며, 이 대기를 없애려면 프레임 슬롯별 업로드 영역과 Fence부터 분리해야 합니다.''')
e.tail('### 예제와 엔진 구현의 차이','''## 화면에서 데이터의 연결을 확인합니다

처음에는 세 정점의 빨강·초록·파랑이 삼각형 내부에서 보간되는지 봅니다. Color만 바꾸면 모양은 유지되고 색만 달라져야 합니다. Model을 Z축 회전으로 바꾸면 모양 전체가 함께 돌아갑니다. CBV의 크기·table 연결을 잘못하면 행렬 변경이 보이지 않거나 잘못된 위치가 나오므로 정점 데이터와 상수 데이터의 경로를 나눠 확인합니다.

이 글은 CBV descriptor table 하나로 관계를 배우는 예제입니다. 현재 YamYam Transform 구현은 root CBV와 Draw별 256바이트 구간을 사용합니다. 개념은 같지만 바인딩 API가 다릅니다. 다음 구현에서는 왜 여러 Draw·여러 프레임·Scene/Game 카메라에 각기 다른 상수 데이터가 필요한지 실제 엔진 코드로 연결합니다.
<mention-page url="https://www.notion.so/3dc0b1ffa61e814e9e8bd403ca145ebe"/>''')
e.save()
