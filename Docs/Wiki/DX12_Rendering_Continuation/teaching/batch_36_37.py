from edit import Edit

e=Edit(36)
e.intro('''## 삼각형 위에 ImGui 창을 함께 그려 봅시다

삼각형이 나오는 DX12 프로그램에 ImGui 초기화 코드를 붙였는데 UI가 안 보이거나, Scene 화면 안에 에디터 전체가 다시 그려질 수 있습니다. UI도 GPU의 Draw이므로 **어느 Command List에, 어느 RenderTarget을 대상으로, 언제 기록하는지**를 맞춰야 합니다.

이번에는 텍스트 창 하나에서 시작해 현재 엔진의 ImguiEditor::Begin/End 경로를 따라가겠습니다. Game과 Scene은 먼저 이미지를 만들고, ImGui는 그 이미지와 버튼·문자를 마지막에 백버퍼로 합성합니다. GPU 완료 대기는 엔진 프레임 시작에서 관리하고 UI 중간에 명령 메모리를 다시 Reset하지 않습니다.''')
e.code(0,replacement='\n'.join(line for line in e.codes[0].split('\n') if 'current version of the backend' not in line),before='''### 폰트도 GPU가 읽는 텍스처입니다

ImGui의 글자는 폰트 atlas 텍스처를 샘플해 그립니다. 따라서 Device만 전달해서 끝나는 것이 아니라, 그 텍스처를 가리킬 SRV 슬롯을 어떻게 얻고 돌려줄지도 알려 줘야 합니다. Scene/Game Image도 같은 shader-visible heap에 유효한 descriptor를 가지고 있어야 합니다.''',after='''Alloc callback은 한 슬롯의 CPU/GPU handle을 함께 반환합니다. CPU handle은 backend가 SRV 내용을 써 넣는 자리이고 GPU handle은 UI Draw가 그 텍스처를 읽을 때 사용할 주소입니다. Free callback은 즉시 free list에 넣는 대신 엔진의 RetireResource로 넘깁니다. 이미 기록한 UI 명령이 그 슬롯을 GPU에서 읽을 수 있기 때문입니다.

예를 들어 폰트가 슬롯 0을 사용 중인데 Scene RT를 같은 슬롯 0에 덮어쓰면 글자가 Scene 이미지의 일부를 샘플할 수 있습니다. 각 이미지의 슬롯을 분리하고, 해제 후에도 마지막 GPU 사용이 끝난 다음 재사용하는 것이 초기화 callback의 실제 목적입니다.''')
e.code(1,after='''Begin은 UI 프레임의 입력·레이아웃 계산을 시작합니다. 이 시점에는 버튼과 창을 어떤 모습으로 만들지 함수 호출로 선언할 준비가 된 것이며, GPU에 제출한 상태는 아닙니다. ImGuizmo도 같은 UI 프레임 안에 선과 도구를 추가하므로 여기서 프레임을 시작합니다.''')
e.code(2,after='''NewFrame 이후에 Begin("Debug Info")와 Text를 매 프레임 호출해야 창이 그려집니다. 이 Begin의 반환값은 내용을 그릴 수 있는 상태인지 나타내며 false여도 End는 호출합니다. 예를 들어 창을 접었을 때 내용은 생략해도 Begin/End의 구조는 유지합니다.

비싼 통계 계산은 별도로 캐시할 수 있지만 Text나 Image 선언까지 몇 프레임마다 생략하면 UI 자체가 사라질 수 있습니다. 클릭 횟수처럼 프레임 사이에 유지할 값은 멤버나 static 변수로 보관하고 매 프레임 초기화하지 않습니다.''')
e.code(3,before='''### UI 명령을 백버퍼에 기록하고 목록을 닫습니다

SceneWindow가 마지막으로 카메라 화면을 자기 RT에 그렸다면 현재 출력도 Scene RT입니다. End는 먼저 ImGui의 DrawData를 마무리하고, 출력을 백버퍼로 복원한 뒤 그 DrawData를 같은 열린 Command List에 추가합니다. 아래 코드는 현재 구현의 순서를 보여 줍니다.''',after='''BindFrameBuffer(false,false)의 첫 false는 다시 Clear하지 않겠다는 뜻이고 두 번째는 깊이 타깃을 붙이지 않겠다는 뜻입니다. ImGui PSO의 DSVFormat=UNKNOWN과 맞춥니다. SetDescriptorHeaps에는 InitInfo에 전달한 것과 같은 heap을 사용합니다. 다른 heap을 바인딩하고 이전 heap의 GPU handle을 Image에 주면 올바른 이미지를 읽지 못합니다.

마지막 RT→PRESENT Barrier와 Close는 UI Draw까지 끝난 뒤입니다. Game 렌더링이 먼저 List를 닫아 버리면 UI를 뒤에 추가할 수 없습니다. 반대로 End 안에서 Allocator를 Reset하면 앞서 기록한 Game/Scene 명령의 저장 공간까지 재사용하게 됩니다. 그래서 프레임 시작의 Reset과 UI 끝의 Close를 한 번씩만 수행합니다.''')
e.code(4,after='''추가 플랫폼 창은 Scene/Game 탭을 OS 창 밖으로 꺼낼 때 생깁니다. 메인 창의 Draw만 끝났다고 전체 프레임의 GPU 사용이 끝나는 것은 아닙니다. 추가 창도 같은 RT의 SRV를 읽을 수 있으므로 플랫폼 창 제출 뒤에 찍은 최종 Fence까지 해당 리소스가 살아 있어야 합니다.''')
e.replace('## 6. 화면이 나오지 않을 때','''## 6. 텍스트 창에서 Scene 이미지까지 확인합니다

먼저 Debug Info 창의 글자가 보이는지 확인합니다. 이것으로 폰트 SRV·UI PSO·DrawData 기록·백버퍼 출력 경로를 확인할 수 있습니다. 다음에 Scene RT를 Image로 넣고, 마지막으로 탭을 별도 OS 창으로 꺼내 봅니다. 처음부터 모든 기능을 동시에 붙이는 것보다 어느 연결에서 문제가 생겼는지 구별하기 쉽습니다.

### 결과에 따라 확인할 연결''')
e.save()

e=Edit(37)
e.intro('''## Transform을 바꾸었는데 화면의 사각형이 움직이지 않습니다

삼각형을 직접 그리는 데 성공한 뒤 엔진의 SpriteRenderer를 연결하면 새로운 문제가 나타납니다. 객체의 Position은 바뀌는데 화면이 움직이지 않거나, 사각형의 한쪽만 그려질 수 있습니다. CPU의 Transform과 GPU 셰이더 사이, 정점 네 개와 삼각형 두 개 사이에 빠진 연결을 찾아야 합니다.

이 글은 Texture·RT를 완성하기 전, DX12 ConstantBuffer·IndexBuffer와 SpriteRenderer의 기초 경로를 복구한 단계입니다. 당시에는 텍스처 대신 정점 색으로 연결을 확인했습니다. 현재 엔진에서는 이 위에 Texture·Draw별 상수 구간·Scene/Game RT·렌더 모드별 PSO를 추가했으며 마지막에 해당 강의를 연결합니다.''')
e.replace('정리하면, 화면에 무언가 그려지고 있었지만 **올바른 이유로 그려지는 것이 아무것도 없는 상태**였다.', '따라서 이 단계에서는 화면에 도형이 보이는지만 확인하지 않고, Transform 변경이 실제 GPU 상수 데이터에 도달하는지와 Draw가 유효한 정점만 참조하는지를 따로 확인해야 했습니다.')
e.replace('그런데 직전에 RectMesh를 4개 버텍스로 변경했기 때문에 존재하지 않는 버텍스 4번, 5번을 읽게 됐고, 그 결과 절반만 제대로 된 삼각형이 나오고 나머지는 깨진 쓰레기 픽셀이 출력됐다.', '그런데 RectMesh의 유효 정점이 4개라면 이 Draw는 4번과 5번까지 요청해 데이터 범위를 벗어납니다. 화면의 구체적인 깨짐 모양을 항상 같은 결과로 단정하기보다, 요청한 정점 수와 실제 버퍼 범위가 맞지 않는 호출 자체를 고쳐야 합니다.')
e.code(0,replacement='''```mermaid
flowchart LR
    T[CPU TransformCB: World·View·Projection] --> C[memcpy로 지정 구간에 쓰기]
    C --> U[UPLOAD Resource]
    U --> B[GPU 가상 주소를 root CBV로 바인딩]
    B --> V[VS의 b0에서 행렬 읽기]
```''',after='''이 그림은 접근 흐름을 나타냅니다. UPLOAD·DEFAULT가 실제로 어느 물리 메모리에 놓이는지는 GPU 구조에 따라 달라질 수 있습니다. 아래의 이전 도식도 이 용도 구분으로 읽고, 당시 IndexBuffer가 UPLOAD로 구현된 점과 구별합니다.''')
e.code(1,after='''Persistent Map은 주소를 계속 보관하는 방식입니다. GPU가 값을 읽는 순간까지 해당 주소의 데이터가 보존되어야 한다는 조건은 그대로입니다. A의 World를 쓰고 Draw를 기록한 뒤 같은 자리에 B의 World를 쓰면 두 Draw가 B의 데이터를 읽을 수 있습니다. Map을 한 번만 했느냐보다 **Draw마다 어느 구간을 사용했느냐**가 중요합니다.''')
e.code(3,after='''192는 256보다 작으므로 첫 구간은 256바이트로 올립니다. 다음 물체의 상수를 같은 버퍼에 배치한다면 base+256, 그다음은 base+512처럼 경계를 맞춥니다. 192바이트 memcpy의 복사량과 한 Draw에 배정하는 구간 256바이트를 혼동하지 않습니다. 일반 크기를 받는 allocator에서는 덧셈 overflow와 버퍼 용량도 검사해야 합니다.''')
e.code(5,after='''InitAsConstantBufferView(0)의 0은 HLSL register(b0)이고 rootParams[0]의 0은 루트 파라미터 순서입니다. Bind에서는 SetGraphicsRootConstantBufferView(0, gpuAddress)로 이 순서의 슬롯에 실제 버퍼 주소를 줍니다. Descriptor Heap에 CBV를 만드는 table 방식과 달리 이 단계는 root CBV를 사용합니다.

빈 Root Signature에서 b0 하나를 받는 형태로 바꾸는 이유는 Transform을 소비하는 셰이더의 입력 계약이 달라졌기 때문입니다. 현재 후속 구현은 여기에 t0 텍스처 table과 s0 정적 sampler도 추가했으므로 아래 한 파라미터 예제는 이 단계의 변경으로 읽습니다.''')
e.code(6,after='''오른쪽 방식에서 첫 삼각형은 0·1·2, 두 번째는 0·2·3입니다. 두 삼각형이 공유하는 대각선의 정점 0과 2를 중복 저장하지 않습니다. DrawIndexedInstanced(6)의 6은 정점 배열의 길이 4가 아니라 읽을 인덱스 개수입니다. 인덱스가 가리키는 최대 번호는 3이어야 합니다.''')
e.code(8,after='''이 구조에서는 정점 네 개가 144바이트이고 uint32_t 인덱스 여섯 개는 24바이트입니다. 합계 168바이트로, 정점 여섯 개를 중복 저장하는 216바이트보다 48바이트 작습니다. UV를 추가했다면 VB stride뿐 아니라 VS 입력과 InputLayout도 36바이트 구조에 맞게 함께 바꿔야 합니다.''')
e.code(11,after='''TransformCB가 정상이어도 VS가 그 상수를 읽지 않으면 물체는 움직이지 않습니다. 그래서 버퍼 업로드만 고치는 것으로 끝내지 않고, Material이 선택하는 셰이더까지 SpriteDefault 경로로 연결했습니다. 처음에는 PS가 vertex color만 출력하게 해 행렬·기하 연결을 확인하고, 이후 텍스처 리소스와 sampler가 준비된 뒤 Sample을 복구합니다.''')
e.replace('## 8. 당시 남은 과제와 현재 완료한 작업','''## 8. Position을 바꾸어 원인과 결과를 확인합니다

같은 사각형의 World 위치를 바꾸었을 때 화면에서도 이동하는지 확인합니다. CPU Transform 값만 바뀌고 화면은 그대로라면 SetData의 memcpy, Bind의 GPU 주소, VS의 b0 사용 순서로 따라갑니다. 한쪽 삼각형만 보인다면 인덱스 여섯 개·IBV Format·DrawIndexedInstanced 호출을 확인합니다. UV는 텍스처를 아직 읽지 않는 이 단계에서도 메모리 레이아웃을 맞춰 두어야 합니다.

도형과 Transform이 연결되면 Texture를 붙일 때 문제 범위를 샘플링·SRV·sampler로 좁힐 수 있습니다. 다음은 이 기초 위에 추가한 현재 구현과 이어 읽을 위치입니다.

### 이 단계에서 다음 단계로''')
e.save()
