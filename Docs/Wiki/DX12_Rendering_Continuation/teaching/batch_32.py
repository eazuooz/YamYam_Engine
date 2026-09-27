from edit import Edit
e=Edit(32)
e.intro('''## 화면의 한 픽셀에서 광선을 보내 봅시다

Raster에서는 삼각형이 덮는 픽셀을 찾았습니다. 이번에는 반대로 카메라에서 특정 픽셀 방향으로 광선을 보내고, 그 광선이 처음 만나는 표면의 색을 구한다고 생각해 봅시다. 아무것도 만나지 않으면 회색 배경을 돌려줍니다. 이 작은 예제로 DXR의 가속 구조, 셰이더 연결, Shader Table이 각각 왜 필요한지 살펴보겠습니다.

이 글은 DXR API의 연결을 배우는 선택 강의이며 현재 YamYam에 레이트레이싱 렌더러를 구현한 기록은 아닙니다. Device5·CommandList4, DXC lib_6_3 이상, OPTIONS5의 RaytracingTier 지원을 확인한 환경을 전제로 합니다. 코드 블록은 각 연결부의 발췌이며 버퍼 할당·카메라 업로드·최종 표시를 포함한 완전한 실행 프로그램은 아닙니다.''')
e.replace('- 🗿 **Intersection** - 충돌 처리를 위한 사용자 정의 솔루션으로, 벤더의 교차점 계산 기능에 의존하지 않고 주어진 광선에 대한 충돌 교차점을 프로그래밍 방식으로 보고할 수 있습니다.', '- **Intersection**은 절차적 도형의 후보 AABB 안에서 실제 교차를 계산하고 보고하는 셰이더입니다. 삼각형 예제에서는 내장 삼각형 교차를 사용하므로 별도 Intersection Shader를 넣지 않습니다.')
e.code(0,before='''### 광선이 맞았을 때와 빗나갔을 때 색을 돌려줍니다

RayGeneration은 출력 픽셀의 가운데를 지나는 광선을 만듭니다. TraceRay는 TLAS를 탐색하고, 허용된 최근접 교차가 있으면 ClosestHit, 없으면 Miss 경로로 결과를 채웁니다. RayPayload는 그 결과를 호출자에게 돌려주는 데이터입니다. 이번에는 float4 색 하나만 담습니다.''',after='''DispatchRaysIndex는 현재 출력 위치이고 +0.5는 픽셀 가운데를 선택합니다. 0~1 UV를 클립 공간의 x,y로 바꾼 뒤 역 ViewProjection으로 월드의 먼 점을 구합니다. 카메라 위치에서 그 점으로 향하는 방향을 정규화해 RayDesc에 넣습니다. 이 코드는 일반 깊이·유한 far plane의 원근 카메라를 가정합니다. 직교 카메라라면 픽셀별 원점과 공통 방향을 만드는 다른 생성식이 필요합니다.

TMin과 TMax는 광선에서 탐색할 거리 구간입니다. payload는 처음 검정으로 채우고 TraceRay가 돌아오면 결과 색을 UAV의 현재 픽셀에 씁니다. ClosestHit의 barycentrics를 임시 UV로 쓰므로 이 코드는 모델의 실제 UV를 복원하는 구현이 아닙니다. 실제 재질을 붙이려면 정점 UV를 읽고 교차점의 무게중심 값으로 보간해야 합니다. time을 바꾸면 이 임시 UV의 x가 조금 이동합니다.

AnyHit는 후보 교차를 허용하거나 무시하는 데 쓸 수 있지만 모든 교차가 거리 순으로 한 번씩 호출된다고 가정하면 안 됩니다. 이번 BLAS는 OPAQUE 삼각형이며 AnyHit·사용자 Intersection을 사용하지 않습니다. 색 하나를 돌려주는 이 예제만으로 여러 반사·간접광·누적·노이즈 제거가 생기는 것도 아닙니다.''')
e.code(1,before='''### 한 번의 Dispatch에서 함께 쓰는 데이터는 Global로 연결합니다

모든 픽셀이 같은 카메라와 장면을 보므로 카메라 b0, 장면 TLAS t0, 출력 u0를 Global Root Signature로 묶습니다. 이번 descriptor heap에서는 [0]에 UAV, [1]에 카메라 CBV, [2]에 재질 텍스처 SRV를 놓는다고 정합니다.''',after='''첫 root parameter는 heap 시작을 받는 table입니다. UAV 범위가 offset 0, CBV 범위는 APPEND이므로 이어지는 offset 1을 읽습니다. 두 번째 root parameter는 t0 TLAS의 GPU 주소를 직접 받는 root SRV입니다. TLAS 주소는 이 table의 세 번째 descriptor로 넣는 것이 아닙니다.

카메라 상수에는 inverseViewProjection, cameraOrigin, maxRayDistance를 HLSL과 같은 순서·패킹으로 쓰고 CBV의 256바이트 정렬도 맞춥니다. Root Signature가 선언되었다고 이 값이 자동 계산·업로드되지는 않습니다. Root Signature 1.1 지원을 전제한 코드이므로 미지원 환경을 다루려면 버전 검사와 1.0 직렬화 경로도 준비합니다.''')
e.code(2,after='''Local root의 첫 인자는 b1에 보낼 32비트 값 네 개입니다. time과 패딩을 Shader Record 안에 값으로 넣습니다. 두 번째 인자는 descriptor table 시작 GPU handle이며 이 예제는 heap의 시작 handle을 전달하고 range offset 2로 t1을 찾습니다. 이미 세 번째 슬롯으로 이동한 handle에 다시 offset 2를 적용하면 잘못된 슬롯을 읽게 됩니다.

이 local 구성은 State Object에서 MyHitGroup에만 연결합니다. RayGen과 Miss는 이 인자를 사용하지 않아 identifier만 있는 record로 충분합니다. local은 “각 셰이더에 반드시 필요하다”는 의미가 아니라, record별로 달라질 데이터를 붙일 수 있다는 의미입니다.''')
e.code(3,before='''### Record는 어떤 셰이더를 실행할지와 그 인자를 나란히 담습니다

State Object가 제공하는 shader identifier 32바이트 뒤에 local root 인자를 선언 순서대로 붙입니다. 이번 HitRecord는 identifier 32 + 상수 16 + table handle 8 = 56바이트의 의미 있는 데이터를 갖고, record 정렬 단위 32의 다음 배수인 64바이트를 사용합니다.''',after='''sizeof가 64가 되었다고 GPU table의 시작 주소까지 자동으로 맞는 것은 아닙니다. 각 table의 GPU 시작 주소는 별도로 64바이트 정렬해야 합니다. record stride의 32바이트 배수 조건과 table 시작의 64바이트 조건을 구분합니다. identifier는 C++ 함수 포인터나 HLSL 소스 주소가 아니라 해당 State Object에서 GetShaderIdentifier로 얻은 바이트입니다.

RayGen과 Miss는 각각 32바이트 identifier만 기록하고, HitGroup은 64바이트 record에 time과 heap 시작 handle을 채웁니다. padding까지 0으로 초기화한 메모리를 사용하고, GPU가 읽는 동안 table·descriptor·참조 리소스가 유지되도록 합니다.''')
e.code(4,before='''### 셰이더 이름과 HitGroup의 관계를 State Object에 등록합니다

라이브러리에 raygen·closesthit·miss 함수가 들어 있어도 DXR이 어떤 조합을 사용할지 연결해야 합니다. MyHitGroup은 삼각형 교차 이후 closesthit를 실행하는 그룹의 이름입니다. Shader Table은 이 그룹 identifier를 사용합니다.''',after='''shaderConfig의 payload 크기 16바이트는 앞 RayPayload의 float4와 일치하고, 삼각형 attribute 8바이트는 barycentrics의 float2입니다. MaxTraceRecursionDepth=1은 이 예제의 RayGen→TraceRay 한 단계에 맞춥니다. ClosestHit에서 추가 광선을 재귀적으로 보내는 경로를 넣는다면 그 제한과 스택·비용도 다시 설계해야 합니다.

subObjects[4]의 Local Root Signature를 MyHitGroup에 association하는 부분이 중요합니다. 이 연결이 맞아야 HitRecord의 뒤쪽 바이트가 b1과 t1 table 인자로 해석됩니다. State Object와 shader identifier를 만든 뒤 Shader Table 메모리를 채우는 실제 생성 순서를 지킵니다. 글은 개념 설명을 위해 Record 구조를 먼저 소개한 것입니다.''')
e.code(5,before='''### BLAS에는 모양을, TLAS에는 배치된 인스턴스를 넣습니다

같은 의자 메시를 방에 열 개 놓는다고 생각해 봅시다. BLAS는 의자 메시의 삼각형들을 찾기 위한 구조이고 TLAS는 그 BLAS를 참조하는 열 개 인스턴스의 배치를 표현할 수 있습니다. 아래는 우선 삼각형 geometry 하나와 단위 변환 인스턴스 하나만 사용하는 예제입니다.''',after='''PrebuildInfo는 결과 AS와 scratch가 각각 얼마나 큰지 계산합니다. result는 이후 TraceRay가 탐색할 결과이고 scratch는 빌드 동안만 쓰는 작업 공간입니다. 같은 버퍼라고 생각해 서로 덮어쓰면 안 됩니다. 아래 연결 조건에 맞게 각각 할당한 뒤 Build 명령을 기록합니다.

BLAS Build 다음의 UAV Barrier는 TLAS Build가 BLAS 결과를 사용할 수 있도록 순서를 연결합니다. TLAS Build 뒤에도 TraceRay 사용 전에 순서를 보장합니다. 이는 CPU가 빌드 완료를 기다리는 Fence와 다릅니다. 같은 Queue에 올바른 순서로 기록하고 CPU 메모리 재사용 시점에는 별도 완료 조건을 확인합니다.

여기서는 R16_UINT 인덱스를 전제하므로 실제 인덱스 배열도 uint16_t여야 합니다. 앞 Raster 강의의 uint32_t 버퍼를 그대로 가져오면 geometry의 IndexFormat도 R32_UINT로 바꿔야 합니다. InstanceMask=0xff는 앞 TraceRay의 마스크와 겹쳐 이 인스턴스가 탐색 대상이 되게 합니다.''')
e.code(6,before='''### DispatchRays는 최종 출력 크기로 실행합니다

앞 Compute의 Dispatch(그룹 수)와 달리 이 Width·Height는 실행할 RayGeneration의 크기입니다. 800×600을 넣으면 이 예제는 출력 위치마다 광선 하나를 만들므로 480,000개의 초기 광선을 보냅니다. 반사 같은 추가 광선은 셰이더에서 TraceRay를 더 호출해야 생깁니다.''',after='''SetComputeRootSignature를 사용하는 것은 DXR이 Compute의 root 바인딩 방식을 사용하기 때문입니다. PSO 선택은 일반 Compute의 SetPipelineState 대신 State Object용 SetPipelineState1을 사용합니다. descriptor table 시작과 TLAS 주소, RayGen/Miss/HitGroup table 주소를 각각 연결한 뒤 DispatchRays를 기록합니다.

실제 화면 표시에는 출력 UAV를 다음 읽기·복사 상태로 전환하고 백버퍼나 ImGui에 연결하는 패스가 더 필요합니다. DispatchRays를 호출한 순간 CPU 이미지가 생기거나 Present가 자동으로 실행되지는 않습니다.''')
e.replace('### 이 예제를 완성할 때 필요한 연결','''## 회색 배경에서 시작해 한 연결씩 확인합니다

먼저 광선이 아무 도형에도 닿지 않을 때 Miss의 회색이 나오도록 출력·표시 경로를 확인합니다. 이후 삼각형 BLAS와 TLAS 인스턴스를 연결하고 교차한 영역만 ClosestHit 색이 나오는지 봅니다. 이때 화면 전체가 회색이면 카메라 방향뿐 아니라 instance mask, 입력 버퍼 형식, TLAS 주소와 Build 순서를 확인합니다. 교차는 되는데 재질이 잘못되면 local record의 인자 배치와 descriptor offset을 따로 봅니다.

이렇게 광선 생성, 교차 탐색, 재질 선택, 결과 표시를 나누면 긴 API 목록보다 실패 지점을 이해하기 쉽습니다. 다음은 발췌 코드를 실행 프로그램에 연결할 때 빠짐없이 준비해야 할 메모리 조건입니다.

### 메모리와 실행 연결 조건''')
e.save()
