from edit import Edit

e=Edit(30)
e.intro('''## 정점 버퍼에서 읽지 않고 셰이더가 삼각형을 만들 수 있을까요?

앞의 Raster 예제는 CPU가 만든 Vertex Buffer와 Index Buffer를 IA에 연결했습니다. Mesh Shader에서는 GPU의 작업 그룹이 정점과 삼각형 연결 정보를 직접 출력할 수 있습니다. 출력한 삼각형은 여전히 래스터라이저와 픽셀 셰이더로 이어집니다. 따라서 Mesh Shader는 래스터라이제이션을 없애는 기능이 아니라 **기하를 준비하는 앞부분을 바꾸는 방식**입니다.

이번 선택 강의에서는 정점 세 개를 출력하는 작은 예제로 이 차이를 배웁니다. 현재 YamYam의 Scene/Game 렌더링은 VS/PS 경로이며 이 글은 구현 완료 기록이 아닙니다. 실험 전에는 아래 OPTIONS7 검사로 MeshShaderTier 지원을 확인하고, DXC와 해당 인터페이스가 있는 SDK를 준비합니다.''')
e.replace('Mesh 파이프라인은 전통적인 그래픽스 파이프라인보다 더 유연하고 강력한 구조를 제공합니다. 특히 병렬 처리에 특화되어 있어, 대규모 지오메트리 데이터를 효율적으로 처리할 수 있습니다.\n이제 Mesh 파이프라인을 살펴봅시다.', '예를 들어 큰 모델을 작은 삼각형 묶음인 meshlet으로 나누고, 보이는 묶음만 GPU에서 처리하는 구조를 만들 수 있습니다. 다만 데이터 분할·가시성 검사·작업 배분을 실제로 구현해야 이점을 얻습니다. 기존 Vertex Shader를 Mesh Shader라는 이름으로 바꾸는 것만으로 자동으로 빨라지지는 않습니다.')
e.code(0,replacement=e.codes[0].replace('```plain text','```c++').replace('// AS를 사용할 때','// HLSL: AS를 사용할 때',1),before='''### Amplification Shader는 다음 작업을 정할 수 있습니다

AS는 선택 단계입니다. 예를 들어 그룹 하나가 네 개의 meshlet을 처리할 MS 작업을 요청할 수 있습니다. 아래 코드는 가시성 검사까지 구현한 예제가 아니라, baseMeshlet 정보와 네 작업을 전달하는 별도의 작은 예제입니다.''',after='''AS 그룹 번호가 2라면 baseMeshlet은 8이 되고 MS 그룹 네 개를 요청합니다. 실제 MS에서는 이 base 값과 자기 그룹 번호를 이용해 담당 meshlet을 고르게 설계할 수 있습니다. AS의 DispatchMesh는 HLSL에서 자식 작업과 payload를 전달하는 호출이며, CPU의 ID3D12GraphicsCommandList6::DispatchMesh와 실행 위치가 다릅니다.

아래 최소 MS는 AS를 생략하므로 이 payload를 받지 않습니다. 두 코드를 한 쌍으로 연결하려면 동일한 MeshPayload 정의와 MS의 in payload 매개변수를 추가해야 합니다.''')
e.code(1,replacement=e.codes[1].replace('```plain text','```c++').replace('// AS를 생략한','// HLSL: AS를 생략한',1),before='''### 스레드 하나가 삼각형 하나를 만드는 최소 예제

처음에는 병렬성을 줄여 데이터 연결부터 봅니다. CPU에서 DispatchMesh(1,1,1)을 호출하고 numthreads(1,1,1)로 그룹 안에 스레드 하나를 둡니다. 한 스레드가 정점 세 개와 인덱스 하나를 모두 쓰므로 같은 출력에 여러 스레드가 덮어쓰는 문제가 없습니다.''',after='''SetMeshOutputCounts(3,1)은 정점 3개와 삼각형 1개를 출력하겠다는 선언입니다. verts 배열은 각 정점의 SV_Position을 담고, tris[0]의 uint3(0,1,2)는 그 정점들을 연결합니다. 전통적인 예제의 VB/IB 역할이 이 출력 배열로 옮겨온 것을 볼 수 있습니다.

numthreads를 32로만 바꾸면 32개 스레드가 같은 verts[0..2]와 tris[0]에 쓰게 됩니다. 병렬 예제로 확장하려면 각 스레드가 서로 다른 요소를 담당하도록 나눠야 합니다. SetMeshOutputCounts는 그룹 전체에서 일관된 제어 흐름으로 호출하고 선언한 배열 용량과 디바이스 지원 한도 안의 개수를 지정합니다. 출력 배열은 groupshared 임시 메모리 선언과도 다릅니다.''')
e.code(2,after='''이 stream은 전통적인 Graphics PSO의 VS 자리에 MS를 넣어 보는 단순 치환이 아닙니다. MS를 사용하는 PSO에는 IA InputLayout과 VS/HS/DS/GS를 함께 구성하지 않습니다. 선택 AS, MS, PS와 출력·Rasterizer·Blend·Depth 상태를 subobject 형태로 모읍니다.

위 코드는 stream의 구조와 CreatePipelineState 호출 형식을 보여 주는 발췌입니다. 실제 stream 객체를 만들고 모든 필드를 채워야 합니다. 최소 예제라면 빈 Root Signature, 앞 MS의 바이트코드, 일정한 색을 반환하는 PS, 실제 RTV 포맷, SampleCount 1, 필요에 맞는 깊이·컬링 상태를 넣습니다. 정점 데이터가 셰이더에 상수로 들어 있으므로 이 최소 사례에는 VB·IB 바인딩이 필요 없습니다.''')
e.code(3,replacement=e.codes[3].replace('```javascript','```c++'),after='''normal을 추가하면 각 정점에서 값을 써서 PS로 전달해야 의미가 있습니다. PrimitiveAttributes는 삼각형 단위 속성의 예입니다. 구조체 선언만 추가한다고 MS의 출력 매개변수나 렌더 타깃 배열 설정까지 연결되지는 않습니다. 첫 삼각형을 확인한 뒤 출력 항목을 하나씩 늘립니다.''')
e.code(5,after='''DX12 Device 생성 성공과 Mesh Shader 지원은 별개의 검사입니다. 지원하지 않는 장치에서는 기존 IA/VS 경로를 사용합니다. AS/MS는 DXC의 as_6_5 / ms_6_5 이상으로 컴파일하고 CPU DispatchMesh에는 ID3D12GraphicsCommandList6가 필요합니다. 지원 Tier 확인은 실제 PSO 생성·Dispatch보다 먼저 수행합니다.''')
e.tail('### 컴파일과 실행 조건','''## 같은 삼각형을 다른 앞단으로 그려 봅니다

앞 Raster 강의의 VB/IB 삼각형과 이번 MS의 정점 좌표를 같게 놓고 비교하면, 화면의 도형은 같아도 기하를 공급하는 경로가 다름을 확인할 수 있습니다. MS의 꼭짓점 하나만 바꾸면 그 꼭짓점만 이동해야 합니다. CPU의 DispatchMesh 그룹 수를 늘리는 것만으로 서로 다른 위치의 삼각형이 나오지는 않습니다. 각 그룹이 위치나 meshlet 데이터를 다르게 선택해야 합니다.

기본 연결이 이해되면 AS를 붙여 작업 분배, 여러 스레드로 출력 요소 분담, meshlet별 가시성 검사 순으로 확장합니다. 통계 구조체를 선언하는 것만으로 측정이 생기지는 않으며 QueryHeap의 기록·Resolve·Fence 완료·readback까지 연결해야 실제 수치를 얻습니다. [Microsoft Mesh Shader 명세](https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html)에서 출력 규칙과 지원 조건을 확인할 수 있습니다.''')
e.save()

e=Edit(31)
e.intro('''## 삼각형 없이 텍스처의 모든 픽셀을 계산해 봅시다

이미지를 그리는 방법이 꼭 정점과 삼각형을 거칠 필요는 없습니다. 출력 텍스처의 (x,y)에 어떤 색을 쓸지 계산할 수 있다면 Compute Shader의 스레드 하나에 픽셀 하나를 맡길 수 있습니다. 이번에는 16×16 스레드 그룹으로 반복 무늬를 만들고 그 결과를 백버퍼로 복사해 보겠습니다.

이 글은 Compute의 선택 학습 예제입니다. 현재 YamYam Scene/Game에 이 효과가 구현되어 있다는 뜻은 아닙니다. 앞 초기화의 Device, Direct Queue/List, Fence, 백버퍼가 준비되었다고 가정하고 CS·UAV·Dispatch 연결을 설명합니다.''')
e.replace('### 상태와 동기화','### 이번 예제의 데이터가 흐르는 방향')
e.replace('이제 데이터를 기록 할 수 있는 언오더드 엑세스 뷰를 만들어 보겠습니다.', '셰이더가 쓰는 RWTexture2D를 실제 GPU 텍스처와 연결할 차례입니다. Resource에는 픽셀 저장 공간이 있고 UAV descriptor에는 쓰기 대상으로 접근할 형식과 범위가 들어 있습니다. Root Signature에 u0를 선언한 것만으로 이 저장 공간이 생기지는 않습니다.')
e.code(1,after='''u0는 HLSL의 RWTexture2D 레지스터입니다. root parameter 0은 이 UAV 하나를 담은 descriptor table로 선언합니다. 나중에 SetComputeRootDescriptorTable(0, uavGPU)을 호출하면 그 시작 handle이 u0에 연결됩니다. Compute에는 Vertex/Pixel 가시성 구분을 쓰지 않아 ShaderVisibility=ALL로 둡니다.''')
e.code(2,replacement=e.codes[2].replace('RWTexture2D','// HLSL: main을 cs_5_0 등 이 예제에 맞는 프로파일로 컴파일합니다.\nRWTexture2D',1),before='''### 그룹 안의 좌표와 전체 좌표를 나눕니다

그룹 크기가 16×16이면 그룹 안 x,y는 각각 0~15입니다. 그룹 (1,0)의 로컬 좌표 (3,5)는 전체 출력에서 (19,5)를 담당합니다. 전체 좌표는 `그룹 번호 × 그룹 크기 + 그룹 안 좌표`로 연결됩니다.''',after='''출력의 R과 G는 localID를 사용하므로 16픽셀마다 다시 0으로 돌아갑니다. B는 globalID.x를 width로 나누므로 전체 화면 왼쪽에서 오른쪽으로 증가합니다. 그 결과 작은 타일 무늬 위에 가로 방향의 파랑 변화가 함께 보입니다. 모든 채널을 globalID 기준으로 바꾸면 반복되는 타일 경계가 사라지는 실험을 할 수 있습니다.

해상도가 17×17이면 한 그룹으로 부족해 2×2그룹을 실행합니다. 실제 실행 영역은 32×32이므로 1024개 스레드 중 289개만 유효한 픽셀입니다. 앞의 경계 검사로 나머지는 쓰기 전에 반환합니다. “그룹 수를 올림”과 “셰이더에서 경계 검사”가 한 쌍인 이유입니다.''')
e.code(3,after='''CS만 사용하므로 InputLayout·VS·PS·Blend·RTV 포맷을 Compute PSO에 넣지 않습니다. compShader는 앞 HLSL의 main을 컴파일해 얻은 바이트코드입니다. 단, PSO 단계가 적다는 것이 전체 알고리즘이나 동기화가 자동으로 쉬워진다는 뜻은 아닙니다. 서로 다른 스레드가 같은 주소를 수정한다면 별도의 경쟁·동기화 문제를 풀어야 합니다.''')
e.code(4,after='''ALLOW_UNORDERED_ACCESS는 이 텍스처를 UAV 쓰기 대상으로 사용할 수 있게 합니다. 초기 상태도 UNORDERED_ACCESS이므로 첫 Dispatch가 여기에 기록합니다. CreateUnorderedAccessView에는 CPU handle을, 명령 목록의 table 바인딩에는 같은 슬롯의 GPU handle을 사용합니다.

이 예제는 백버퍼에 CopyResource로 옮기므로 크기·포맷·샘플 수를 서로 맞춥니다. 원하는 모든 백버퍼를 곧바로 UAV로 쓸 수 있다고 가정하지 않고 별도 출력 텍스처를 둡니다. 이 장에서는 R8G8B8A8_UNORM single-sample 텍스처 한 장부터 확인합니다.''')
e.code(5,before='''### Dispatch의 인자는 스레드 수가 아니라 그룹 수입니다

가로 1920, 세로 1080이라면 x는 120그룹, y는 68그룹입니다. y의 마지막 그룹에서 일부 스레드는 1080 바깥이라 CS 경계 검사로 빠집니다. DIRECT List에서 Compute와 백버퍼 복사를 함께 기록하므로 별도 Compute Queue 사이의 동기화는 이번 예제에 필요하지 않습니다.''',after='''Dispatch 이후 출력 텍스처는 UAV 쓰기에서 COPY_SOURCE 읽기로, 백버퍼는 PRESENT에서 COPY_DEST 쓰기로 전환합니다. CopyResource가 끝난 뒤 각각 다음 Dispatch용 UNORDERED_ACCESS와 화면 표시용 PRESENT로 되돌립니다. 상태 전환은 작업의 접근 순서를 연결하고, CPU에서 다음 프레임 메모리·Allocator를 재사용할 조건은 Fence가 보장합니다.

CopyResource는 이미지 크기를 바꾸거나 색 공간을 변환하는 그리기 함수가 아닙니다. 백버퍼를 리사이즈했다면 출력 텍스처도 맞춰 다시 만들고, 이전 리소스가 GPU에서 사용 중인지 확인해야 합니다. 다음 패스도 UAV 상태로 같은 데이터에 의존한다면 적절한 UAV Barrier가 필요한지 판단합니다.''')
e.replace('### 참고자료','''## 무늬를 바꾸며 실행 범위를 확인합니다

17×17처럼 그룹 크기로 나누어떨어지지 않는 작은 해상도로 먼저 생각해 보고, 실제 화면에서는 16픽셀마다 반복되는 R/G와 전체 화면에 걸친 B의 변화를 비교합니다. globalID 대신 localID를 사용한 채 전체 이미지의 그라데이션을 기대하면 반복 무늬가 나오는 이유를 이제 설명할 수 있습니다.

UAV 쓰기는 곧 화면 표시가 아닙니다. PSO와 root table이 맞고 Dispatch가 실행되어도 복사·Present가 연결되지 않으면 화면에는 결과가 나타나지 않습니다. 이후에는 같은 출력 텍스처를 SRV로 전환해 후처리나 ImGui Image에서 읽는 방식으로 확장할 수 있습니다.

### 참고자료''')
e.save()
