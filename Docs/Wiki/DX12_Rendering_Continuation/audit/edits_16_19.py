e=Edit(16,'같은 메시에서 Rasterizer의 FillMode를 바꾸어 삼각형 연결을 확인합니다. 단색 셰이더는 시인성을 위한 별도 선택입니다.','와이어프레임 전환 후 원래 재질·깊이·래스터라이저 상태로 돌아옵니다.')
e.para('- Z-버퍼 사용 방식','- 깊이 클리핑과 bias를 제어합니다. 깊이 비교·쓰기 정책은 DepthStencil State의 역할입니다.')
e.para('- 뒷면을 렌더링하지 않으면 성능','- 보이지 않는 면의 작업을 줄일 수 있습니다. 전체 성능이 항상 50% 향상되는 것은 아닙니다.')
e.para('- OpenGL은 반시계 방향을 앞면','- 앞면 판정은 투영 후 winding과 설정의 조합입니다. API 이름만으로 모델 데이터의 방향을 단정하지 않습니다.')
e.para('- **FALSE**: 클리핑 비활성화','- **FALSE**: z 깊이 클리핑을 끕니다. x/y와 w 관련 클리핑까지 모두 없어지는 것은 아닙니다.')
e.para('- 일반적으로 Solid 렌더링보다 빠름','- 선이 덮는 픽셀 수와 래스터라이저 처리량이 달라지므로 실제 GPU 시간을 측정합니다. 항상 Solid보다 빠르지는 않습니다.')
e.para('- **Rasterization**: 감소','- **Rasterization**: 면 대신 선의 커버리지를 생성하며 GPU·도형 밀도에 따라 비용이 달라집니다.')
e.para('- **Pixel Shader**: 대폭 감소','- **Pixel Shader**: 넓은 면에서는 호출이 줄 수 있으나, 촘촘한 선의 겹침에서는 감소를 보장하지 않습니다.')
e.block(8,'// MSAA는 RT/DS의 SampleDesc와 지원 sample count도 함께 설정합니다.\nrsDesc.MultisampleEnable = true; // 이 플래그 하나로 멀티샘플 RT가 생성되지 않음')
e.block_replace(12,'nullptr, 0xffffff','nullptr, 0xffffffff')
e.block_replace(12,'Shader* wireframeShader = Resources::Find<Shader>(L"WireframeShader");','Shader* wireframeShader = Resources::Find<Shader>(L"WireframeShader");\n        if (!wireframeShader) return; // 초기화 단계에서 누락을 오류로 보고합니다.')
e.block_replace(15,'    float WireframeThickness;','    // 색상만 사용합니다. 상수 하나로 하드웨어 wireframe 두께는 바뀌지 않습니다.')
e.block_replace(17,'static bool wireframeEnabled = false;','bool wireframeEnabled = Renderer::GetInstance()->IsWireframeMode();')
e.block(19,'Solid + Wireframe 오버레이 패스\n1. 기존 상태와 Material을 보관하고 Solid Draw\n2. 별도 override 패스에서 Wireframe + 깊이 LessEqual + 깊이 쓰기 끔\n3. 필요하면 겹침을 줄이는 depth bias 적용 후 단색 Draw\n4. 기존 Wireframe 플래그·Material·깊이 상태 복원\nDraw 내부에서 Shader::Bind가 상태를 다시 덮지 않도록 연결합니다.','plain text')
e.block(21,'''// PS 입력의 SV_POSITION.z는 이미 래스터화된 깊이입니다.
struct PS_INPUT { float4 Position : SV_POSITION; };
float4 PSMain(PS_INPUT input) : SV_TARGET
{
    float depthColor = 1.0f - saturate(input.Position.z);
    return float4(depthColor, depthColor, depthColor, 1.0f);
}
// 일반 원근 깊이는 비선형입니다. 선형 거리 시각화는 별도 변환이 필요합니다.''',lang='hlsl')
e.block(26,'선 두께를 직접 만드는 확장 설계\n1. 선의 두 끝을 화면/클립 공간으로 변환\n2. 화면 해상도를 고려해 수직 방향의 폭 계산\n3. 두 삼각형(쿼드)을 생성하고 적절한 clipping 처리\nWireframe Rasterizer를 켠다고 Geometry Shader 입력이 line으로 바뀌지는 않습니다.\n별도 edge 목록 또는 삼각형의 barycentric 기법 등을 선택해야 합니다.','plain text')
e.block_replace(27,'D3D11_RASTERIZER_DESC shadowRsDesc = {};','D3D11_RASTERIZER_DESC shadowRsDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);')
e.block_replace(28,'D3D11_RASTERIZER_DESC twoSidedRsDesc = {};','D3D11_RASTERIZER_DESC twoSidedRsDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);')
e.finish('### 코드·이미지 적용 범위\n\n위 실행 그림은 메시의 삼각형 연결을 확인하는 용도입니다. 뒤의 색상 애니메이션·오버레이·선택 강조·두께 변경은 추가 버퍼와 패스가 필요한 확장 예제입니다. Renderer 토글 예제에는 실제 싱글톤 선언과 Shader 플래그 연결이 생략되어 있습니다. 공유 Shader의 상태를 바꿀 때는 원래 값을 저장·복원하고 다른 Material에 남지 않도록 합니다. 상태 생성 함수의 성공 여부는 실제 래퍼의 bool/HRESULT에 맞춰 검사합니다.\n\nDX12에서는 독립된 D3D11 RasterizerState를 바인딩하지 않고 Rasterizer 설정을 포함한 PSO를 선택합니다. 현재 PSO 조합 관리 구현은 다음 문서에서 이어집니다.\n<mention-page url="'+NEW2+'"/>')

e=Edit(17,'먼저 만들고 싶은 에디터의 작업 흐름을 정한 뒤 Win32 + DX11 백엔드로 ImGui를 연결합니다.','NewFrame → UI 구성 → Render → 백엔드 Draw → Present의 한 프레임 흐름을 구분합니다.','flowchart LR\n    W[Win32 입력] --> N[ImGui NewFrame]\n    N --> U[창·버튼·Image 구성]\n    U --> R[ImGui Render: DrawData 생성]\n    R --> B[DX11 backend: GPU 명령]\n    B --> P[Present]')
e.guide+='\n### 우리가 만들려는 에디터와 현재 위치\n\n목표는 씬을 보면서 오브젝트를 고르고 속성을 수정한 뒤 Game 화면으로 결과를 확인하는 흐름입니다. 상용 엔진의 Terrain·Blueprint·Hot Reload·Profiler는 참고할 수 있는 확장 방향이며 YamYam의 완료 목록이 아닙니다. 현재 DX12에서 확인한 핵심은 **도킹 UI와 서로 다른 카메라의 Scene/Game 렌더 결과 표시**입니다. Hierarchy·Inspector의 실제 편집 연결은 후속 작업입니다.\n\n**참고 화면 — Godot 공식 문서**\n<image src="https://docs.godotengine.org/en/stable/_images/editor_intro_editor_empty.webp">Godot 에디터: 중앙 작업 뷰, Scene 트리, Inspector, FileSystem</image>\n\n[Godot 공식 에디터 안내](https://docs.godotengine.org/en/stable/getting_started/introduction/first_look_at_the_editor.html)의 화면입니다. Godot의 Scene dock은 노드 트리로, YamYam의 Scene View와는 역할이 다릅니다. YamYam 실제 실행 화면과 목표별 비교 표는 다음 글에서 확인합니다.\n<mention-page url="'+NEW1+'"/>\n'
e.para('에디터에서 보는 것이 실제 게임과 동일합니다.','에디터가 수정 결과를 빠르게 보여 주는 것이 목표입니다. Scene View의 편집 카메라·보조선과 실제 Game 카메라 출력은 다를 수 있으므로 두 뷰를 따로 확인합니다.')
e.para('- 상태 관리 불필요','- 매 프레임 UI를 구성하지만 데이터 값은 애플리케이션이, 포커스·드래그 등의 내부 상태는 ImGui가 관리합니다.')
e.para('<td>생산성 향상 (10배 이상)','<td>시각적 편집과 빠른 피드백, 협업·디버깅 도구와 확장성. 향상 폭은 작업과 도구에 따라 달라집니다.</td>')
e.block_replace(5,'시간: 코드 방식 1시간 vs 에디터 방식 5분','위 내용은 작업 방식 비교 예시이며 실제 시간을 측정한 결과가 아닙니다.')
e.block_replace(8,'→ 병렬 작업으로 개발 속도 4배 향상','→ 역할별 병행 작업이 가능하지만 속도 향상 비율은 측정이 필요합니다.')
e.block_replace(10,'성능 병목 지점 찾기:','성능 병목 지점을 설명하기 위한 가상 수치 예시 (YamYam 측정값 아님):')
e.block_replace(2,'// 에디터에서 수정 가능','// Inspector UI 또는 리플렉션 등록을 구현해야 편집 가능')
e.block_replace(11,'        for (int x = 0; x < w; x++)','        if (w < 1 || h < 1 || w > 512 || h > 512 || d < 0.f || d > 1.f) return;\n        for (int x = 0; x < w; x++)')
for n in (16,28):e.block_replace(n,'CreateWindow(','CreateWindowA(')
e.block_replace(20,'    ImGui_ImplWin32_Init(mHwnd);','    if (!ImGui_ImplWin32_Init(mHwnd))\n    { ImGui::DestroyContext(); return false; }')
e.block_replace(20,'    ImGui_ImplDX11_Init(device, deviceContext);','    if (!ImGui_ImplDX11_Init(device, deviceContext))\n    { ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext(); return false; }')
e.block(31,'개념 흐름 (실제 ImGui::Button 소스가 아님)\n1. 안정적인 ID와 위젯 배치 영역 계산\n2. hover·active·포커스·마우스/키보드 입력 상태 처리\n3. 배경과 글자를 DrawList에 추가\n4. 활성화된 프레임에 true 반환\n마우스 버튼을 누른 순간만 검사하는 함수로 실제 Button 동작을 대체하지 않습니다.','plain text')
e.block(33,'DX11 백엔드가 맡는 작업\n1. DrawData 크기에 맞는 정점·인덱스 버퍼 준비와 업로드\n2. UI용 Shader, Input Layout, Blend/Depth/Rasterizer 설정\n3. DrawCmd별 clip rect, Texture SRV, offset, callback 처리\n4. DrawIndexed 실행 후 백업한 엔진 상태 복원\n이 목록은 역할 설명입니다. 실제 imgui_impl_dx11.cpp를 사용합니다.','plain text')
e.block(34,'// 일반 경로에서 Render는 필요하면 EndFrame을 내부 호출합니다.\nImGui::Render();\n// 생성된 DrawData를 현재 프레임의 renderer backend에 전달\nImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());\n// 다음 프레임은 backend NewFrame 후 ImGui::NewFrame으로 시작합니다.\n// 창·포커스·위젯 ID 상태까지 전부 삭제되는 것은 아닙니다.')
e.block_replace(37,'// 5분 만에 만드는 레벨 에디터','// 레벨 에디터의 UI 틀 — 배치·삭제·이동·저장은 별도 구현')
e.finish('### 통합할 때 확인할 경계\n\n도킹·멀티뷰포트는 해당 기능이 포함된 ImGui 브랜치와 같은 버전의 core/backend 파일을 함께 사용합니다. 매번 최신 파일 일부만 교체하지 않습니다. ImGuiIO의 WantCaptureMouse/WantCaptureKeyboard는 게임 입력 전달 정책에 참고하되, 플랫폼 백엔드에는 입력을 계속 전달합니다. 멀티뷰포트와 Scene/Game 카메라는 서로 다른 개념입니다.\n\nRenderImGui 직전에는 UI가 그려질 백버퍼 RTV를 바인딩해야 합니다. RT를 읽는 Image가 있다면 같은 리소스가 동시에 출력으로 바인딩되지 않게 합니다. ImGui::Begin의 반환값이 false여도 End는 대응해서 호출합니다. 커스텀 Inspector·레벨 생성·DrawList 구조 예시는 엔진 기능의 설계 설명이며 그대로 완성된 내부 구현은 아닙니다.\n\nDX12에서는 GPU가 읽는 descriptor·프레임 allocator·RT의 수명까지 엔진이 관리합니다. 그 차이는 두 후속 구현 글에서 다룹니다.\n<mention-page url="'+NEW2+'"/>')

e=Edit(18,'Scene 이미지를 그린 정확한 영역에 기즈모를 얹고, 조작된 월드 행렬을 엔진 Transform 규약에 맞춰 반영합니다.','이동·회전·스케일 조작이 Scene 카메라와 같은 영역·행렬을 사용합니다.','flowchart LR\n    I[Scene Image의 실제 화면 사각형] --> G[ImGuizmo SetRect·Manipulate]\n    C[Scene 카메라 V·P] --> G\n    W[오브젝트 World] --> G\n    G --> A[엔진 Transform에 적용]')
e.para('- 짐벌 락 방지 (쿼터니언 기반)','- 행렬 조작을 제공하지만 Euler 분해 결과의 불연속·짐벌 락·음수 스케일 문제를 자동으로 해결하지는 않습니다.')
e.para('ImGuizmo는 4x4 변환 행렬을 사용하며','ImGuizmo에는 연속된 float 16개를 전달합니다. 저장 순서와 벡터 곱셈 방향을 함께 맞춰야 합니다. YamYam의 현재 SceneWindow는 CPU의 SimpleMath 행렬을 그대로 넘기고 있으며, HLSL 업로드 규약과 별도로 확인해야 합니다.')
e.para('- DirectX는 행 우선(Row-major)','- DirectXMath의 행벡터 규약과 HLSL의 기본 column-major 저장은 다른 주제입니다. API 이름만으로 무조건 전치하지 않습니다.')
e.para('- **행렬 전치 문제**: DirectX와','- **행렬 규약 불일치**: 저장 순서·곱셈 순서·좌표계·투영을 확인한 뒤 필요한 변환만 적용합니다.')
e.para('- 더블클릭으로 원래 시점 복원','- 원래 시점 복원은 별도 저장·복원 UI로 구현할 수 있습니다. 현재 포함된 ViewManipulate에 더블클릭 복원을 기본 기능으로 가정하지 않습니다.')
e.para('### 완전한 에디터 윈도우 구현','### 에디터 윈도우 통합 설계 예제')
e.para('- 단 몇 줄의 코드로 Unity/Unreal 수준의','- 기본 기즈모 호출은 짧지만 선택·부모 변환·Undo·입력 경합은 엔진에서 연결해야 합니다.')
e.block(1,e.blocks[1][3],lang='cmake')
e.block_replace(2,'    ImGui::NewFrame();','    ImGui_ImplDX11_NewFrame();\n    ImGui_ImplWin32_NewFrame();\n    ImGui::NewFrame(); // 상위 루프가 호출했다면 여기서 중복 호출하지 않음')
e.block(3,'''// Scene Image를 제출한 직후, 같은 ImGui 창 안에서 호출합니다.
const ImVec2 imageMin = ImGui::GetItemRectMin();
const ImVec2 imageMax = ImGui::GetItemRectMax();
ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
ImGuizmo::SetRect(imageMin.x, imageMin.y,
    imageMax.x - imageMin.x, imageMax.y - imageMin.y);''')
e.block_replace(8,'ImGuizmo::TRANSLATE | ImGuizmo::ROTATE | ImGuizmo::SCALE;','static_cast<ImGuizmo::OPERATION>(\n        ImGuizmo::TRANSLATE | ImGuizmo::ROTATE | ImGuizmo::SCALE);')
e.block(16,'변환 결과 적용 방식 중 하나를 선택합니다.\nA. SetWorldMatrix 어댑터에서 엔진 규약에 맞춰 저장·분해\nB. 부모가 없는 TRS 모델이면 행렬을 분해해 Position/Rotation/Scale 설정\nImGuizmo의 Decompose 회전 결과는 degree입니다.\n부모 계층을 추가하면 row-vector 규약에서 local = newWorld × inverse(parentWorld).\n월드 행렬을 분해한 값을 그대로 자식의 로컬 TRS에 넣으면 잘못됩니다.','plain text')
e.block_replace(21,'static float snapValue = 1.0f;\n    snapValues = &snapValue;','static float snap[3] = { 1.0f, 1.0f, 1.0f };\n    snapValues = snap; // 이동은 xyz 세 요소를 읽습니다.')
e.block(23,'''float deltaMatrix[16] = {};
const bool changed = ImGuizmo::Manipulate(
    view, proj, operation, mode, matrix, deltaMatrix);
// changed일 때만 결과를 반영합니다.
// 다중 선택의 공통 pivot 이동·회전·크기 처리는 아래의 전후 행렬 설계를 사용합니다.''')
e.block_replace(24,'// 특정 축이 사용 중인지','// 특정 조작 핸들 위에 마우스가 있는지')
e.block_replace(24,'// X축 조작 중','// 이동 X축 핸들 위에 있음 (드래그 여부는 IsUsing과 별개)')
e.block_replace(26,'        ImGui::Begin("Scene View");','        if (!ImGui::Begin("Scene View")) { ImGui::End(); return; }')
e.block_replace(26,'        ImGui::Image(sceneRenderTexture, viewportSize);','        if (viewportSize.x <= 0.f || viewportSize.y <= 0.f)\n        { ImGui::End(); return; }\n        ImGui::Image(sceneRenderTexture, viewportSize);')
e.block_replace(26,'        ImGuizmo::SetRect(viewportPos.x','        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());\n        ImGuizmo::SetRect(viewportPos.x')
e.block_replace(26,'        Camera* camera = GetActiveCamera();','        Camera* camera = GetActiveCamera();\n        if (!camera) return;')
e.block_replace(26,'            transform->SetPosition(position);','            // 이 예제는 부모 없는 TRS, degree 회전 규약을 가정합니다.\n            transform->SetPosition(position);')
e.block_replace(26,'            UndoSystem::RecordTransform(transform);','            // 매 프레임 명령을 쌓지 않습니다.\n            // 드래그 시작 전 값과 종료 후 값을 한 Undo 명령에 저장합니다.')
e.block(27,'기즈모 단축키 전달 정책\n1. Scene 창 포커스가 있고 텍스트 편집 중이 아닐 때만 처리\n2. Q: 엔진의 gizmoEnabled=false (임의의 ImGuizmo::NONE 상수에 의존하지 않음)\n3. W/E/R: TRANSLATE/ROTATE/SCALE 선택 후 gizmoEnabled=true\n4. Ctrl: 현재 조작의 스냅 값 사용\n5. IsUsing/IsOver인 동안 카메라 이동과 오브젝트 선택 입력을 중재','plain text')
e.block(28,'다중 선택 공통 pivot 설계 (row-vector 규약)\n드래그 시작: pivotStart, 각 worldStart, 선택 객체 ID를 저장\n드래그 중: pivotAfter를 Manipulate 결과로 받음\ndelta = inverse(pivotStart) × pivotAfter\n각 newWorld = worldStart × delta\n부모가 있는 객체는 newWorld를 로컬로 변환한 뒤 적용\n드래그 종료: 전체 변환을 하나의 Undo 명령으로 등록\n매 프레임 원본 pivot을 다시 만들고 누적 결과에 delta를 중복 적용하지 않습니다.','plain text')
e.block(31,'''// WantCaptureMouse/Keyboard는 ImGui가 계산하는 출력값입니다.
// 엔진의 입력 전달 조건을 별도로 계산합니다.
const bool blockSceneCamera = ImGuizmo::IsUsing() || ImGuizmo::IsOver();
// Scene 카메라 입력을 처리할 때 blockSceneCamera를 검사합니다.''')
e.block(35,'''// <cmath> 필요. NaN뿐 아니라 모든 요소의 무한대도 검사합니다.
bool IsFiniteMatrix(const float* matrix)
{
    if (!matrix) return false;
    for (int i = 0; i < 16; ++i)
        if (!std::isfinite(matrix[i])) return false;
    return true;
}
// view/projection/model을 검사하고 실패하면 편집을 중단합니다.
// 사용자 Transform을 자동으로 Identity로 덮어쓰지 않습니다.''')
e.block(36,'// 현재 YamYam SceneWindow는 CPU 행렬을 전치 없이 넘깁니다.\n// HLSL 업로드용 전치 데이터를 여기 재사용하지 않습니다.\n// 평행이동 테스트: float 배열의 12, 13, 14 위치와 화면 이동을 확인합니다.')
e.block(37,'// 좌표계 변환이 정말 필요한 경우에만 적용합니다.\n// 각도 단위·행벡터/열벡터 규약·변환 방향을 확인합니다.\n// SimpleMath CreateRotationX 인자는 라디안입니다.\nauto conversion = Matrix::CreateRotationX(DirectX::XM_PIDIV2);\n// row-vector에서 +Y를 +Z로 옮기는 +90도 예시')
e.block(39,'감도 조절은 행렬 전체에 스칼라를 곱하는 방식으로 하지 않습니다.\n행렬 전체를 곱하면 회전·스케일뿐 아니라 동차 성분도 달라집니다.\n필요하면 이동량을 조절하거나 회전 delta를 분해해 보간하는 별도 정책을 만듭니다.','plain text')
e.finish('### 현재 엔진과의 연결·남은 작업\n\n설명용 Matrix4x4·SelectionManager·SetWorldMatrix·UndoSystem은 연결 지점을 보여 주는 인터페이스입니다. 현재 저장소에 같은 이름과 완성도를 보장하지 않습니다. 실제 SceneWindow는 EditorCamera 행렬과 Scene RT의 Image 영역을 쓰고, 선택 객체가 있을 때 Manipulate를 호출합니다.\n\nDecompose/Recompose는 degree Euler를 사용하며 포함된 ImGuizmo 헤더도 수치 안정성 한계를 언급합니다. 음수·비균일 스케일, shear, 부모 변환은 별도 검증이 필요합니다. Undo 명령은 조작 결과를 반영하기 **전**의 값을 잡고, 삭제된 객체를 raw pointer로 계속 참조하지 않도록 ID/수명 정책을 마련합니다. [확인한 소스](https://github.com/eazuooz/YamYam_Engine/blob/ce56f16dd7a40fd2b4bf03c680790e5b637c7530/Editor_Window/ImGuizmo.cpp)\n\n화면이 찌그러지거나 기즈모가 어긋나면 Scene Image 사각형, 해당 카메라의 aspect ratio, projection 종류, CPU 행렬 규약 순서로 확인합니다. 공식 API는 [ImGuizmo 저장소](https://github.com/CedricGuillemet/ImGuizmo)를 참고합니다.')

e=Edit(19,'EditorApplication은 전체 흐름, EditorWindow는 창, Editor는 객체별 편집 UI를 맡도록 나누는 설계입니다. 선언과 실제 구현을 함께 확인합니다.','새 창을 등록해 OnGUI를 호출하는 경로와, 아직 비어 있는 편집·저장 기능을 구분합니다.','flowchart TD\n    A[메인 루프] --> E[EditorApplication]\n    E --> I[ImguiEditor: Begin·End]\n    E --> W[EditorWindow별 OnGUI]\n    W --> S[Scene·Game 화면]\n    W -. 후속 구현 .-> P[Hierarchy·Inspector 편집]\n    E -. 후속 구현 .-> F[씬 저장·프로젝트 열기]')
e.guide+='\n### 소스 기준 구현 상태\n\n`ce56f16`에서 **GUILayout은 빈 클래스**이고 OpenProject/NewScene/SaveScene/SaveSceneAs/OpenScene도 빈 함수입니다. 아래 자동 레이아웃 API와 저장 흐름은 앞으로 구현할 설계안입니다. 창 목록과 도킹, Scene/Game RT 표시를 먼저 복구한 상태입니다.\n\n```c++\nnamespace gui\n{\n    class GUILayout\n    {\n    };\n}\n```\n\n출처: [guiGUILayout.h](https://github.com/eazuooz/YamYam_Engine/blob/ce56f16dd7a40fd2b4bf03c680790e5b637c7530/Editor_Window/guiGUILayout.h), [guiEditorApplication.cpp](https://github.com/eazuooz/YamYam_Engine/blob/ce56f16dd7a40fd2b4bf03c680790e5b637c7530/Editor_Window/guiEditorApplication.cpp).\n'
e.para('- **싱글톤 패턴**: 모든 메서드가','- **정적 관리 함수 중심 설계**: static 메서드와 상태로 접근합니다. 생성자를 제한하는 전형적인 Singleton 구현과는 구분하며 GetWindow 템플릿은 현재 인스턴스 메서드입니다.')
e.para('`GUILayout`은 **자동 레이아웃 시스템**','`GUILayout`은 자동 레이아웃 유틸리티로 계획한 클래스입니다. 현재 구현은 비어 있으므로 아래 기능·API는 설계 제안으로 읽습니다. 실제 UI는 먼저 ImGui의 SameLine·Table·Group·StyleVar로 만들 수 있습니다.')
e.para('- 새로운 `EditorWindow`나 `Editor`를 추가해도','- 새 EditorWindow나 Editor를 파생할 수 있습니다. 생성·등록 경로와 선택 객체 전달은 함께 연결해야 합니다.')
e.para('- 불필요한 재렌더링 방지','- 비싼 데이터 준비·레이아웃 계산 결과를 캐시할 수 있습니다. ImGui의 표시할 위젯 제출 자체는 매 프레임 필요합니다.')
for n in (0,3,6):e.block(n,e.blocks[n][3],lang='plain text')
e.block_replace(9,'GUILayout::TextField(nameBuffer);','GUILayout::TextField(nameBuffer, sizeof(nameBuffer));')
e.block_replace(11,'GUILayout::Button($"Button {i}");','GUILayout::Button("Button " + std::to_string(i));')
e.block_replace(17,'GUILayout::Label($"FPS: {GetFPS()}");','GUILayout::Label("FPS: " + std::to_string(GetFPS()));')
e.block(15,'// 아래는 제안 API를 순차적으로 시험하는 독립된 그룹입니다.\nGUILayout::SetAlignment(GUILayout::Alignment::Left);\nGUILayout::BeginHorizontal();\nGUILayout::Button("Left aligned");\nGUILayout::EndHorizontal();\n// Center/Right/Justify도 그룹마다 SetAlignment 후 Begin/End를 대응시킵니다.')
e.block_replace(16,'    GUILayout::BeginVertical();','    if (!selectedObject) return;\n    GUILayout::BeginVertical();')
e.block_replace(18,'    private:\n        // 내부 레이아웃 상태 관리','    private:\n        // LayoutContext 구조와 각 메서드 본문은 별도 설계가 필요합니다.\n        // 내부 레이아웃 상태 관리')
e.block(21,'엔진 메인 루프 (OS 메시지·Update·Render·Present 담당)\n  → EditorApplication: 프레임 UI 흐름\n  → ImguiEditor Begin / 각 EditorWindow OnGUI / ImguiEditor End\n  → 필요한 창에서 객체별 Editor UI 호출 (후속 연결)\nEditorWindow마다 별도의 while 메인 루프를 만드는 구조가 아닙니다.','plain text')
e.block_replace(23,'사용자가 File → Save 메뉴 클릭','설계 목표 — 저장 구현 전에는 성공으로 표시하지 않습니다.\n사용자가 File → Save 메뉴 클릭')
e.block_replace(24,'// 커스텀 에디터 윈도우 플러그인','// 창 파생·등록 인터페이스 제안 (실제 plugin loader는 별도 구현)')
e.finish('### 현재 ImGui로 바로 표현할 수 있는 레이아웃\n\n다음 코드는 열린 ImGui 창의 OnGUI 안에서 사용하는 2열 배치입니다. 아직 없는 GUILayout 구현에 의존하지 않습니다.\n\n```c++\nstatic float value = 1.0f;\nif (ImGui::BeginTable("Properties", 2))\n{\n    ImGui::TableNextRow();\n    ImGui::TableNextColumn();\n    ImGui::TextUnformatted("Value");\n    ImGui::TableNextColumn();\n    ImGui::SetNextItemWidth(-1.0f);\n    ImGui::DragFloat("##Value", &value, 0.1f);\n    ImGui::EndTable();\n}\n```\n\n창의 State 값을 변경하는 것만으로 OnEnable/OnDisable/OnDestroy가 자동 호출되지는 않습니다. 상태 전이와 수명 처리를 관리자가 실제로 호출해야 합니다. 같은 제목·위젯을 반복할 때는 안정적인 ID를 주고, 창/선택 객체를 파괴할 때는 참조를 먼저 정리합니다.\n\n현재 화면과 Godot의 목표 비교는 다음 문서의 실제 캡처에서 확인합니다.\n<mention-page url="'+NEW1+'"/>')
