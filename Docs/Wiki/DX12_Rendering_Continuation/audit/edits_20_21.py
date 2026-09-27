e=Edit(20,'DockSpace는 창을 배치할 공간이고 Scene/Game의 렌더링 내용은 각 창이 별도로 만듭니다. 초기 배치와 사용자 배치의 수명을 구분합니다.','도킹·탭 전환·창 닫기가 유지되고 초기 레이아웃이 매 프레임 사용자 배치를 덮지 않습니다.')
e.guide+='\n**범위:** File/Edit/Script 메뉴와 프리셋 관리 예시는 UI 설계입니다. 현재 YamYam의 저장·프로젝트 열기 함수는 비어 있으며, 메뉴가 있다는 것만으로 직렬화·Undo·C# hot reload가 완성된 것은 아닙니다. 실제 Scene/Game 화면과 배치 목표 비교는 다음 글에 있습니다.\n<mention-page url="'+NEW1+'"/>\n'
e.block_replace(0,'// ...','// ...\nImGui::End();')
e.block_replace(3,'Hierchy','Hierarchy')
e.block(9,e.blocks[9][3],lang='plain text')
e.block(12,'''// 아래 설정은 용도별 대안입니다. 모두 한꺼번에 OR하지 않습니다.
ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_None;
// 빈 중앙 영역의 배경·입력을 뒤로 통과시킴
flags = ImGuiDockNodeFlags_PassthruCentralNode;
// 중앙 노드 위로 도킹하지 못하게 함 (중앙을 비워 둠)
flags = ImGuiDockNodeFlags_NoDockingOverCentralNode;
// 사용자의 추가 분할 금지
flags = ImGuiDockNodeFlags_NoDockingSplit;
// 노드 크기 변경 금지
flags = ImGuiDockNodeFlags_NoResize;
// NoTabBar는 탭을 항상 보이는 기능이 아니라 탭 바를 숨기는 내부 플래그입니다.''')
e.block(13,'''// 일반적인 도킹
mDockspaceFlags = ImGuiDockNodeFlags_None;
// 빈 중앙을 투명하게 쓸 때, host window에도 배경 없음 설정 적용
mDockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
// 분할·크기 변경을 제한하는 경우
mDockspaceFlags = ImGuiDockNodeFlags_NoDockingSplit | ImGuiDockNodeFlags_NoResize;''')
e.block_replace(11,'ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");\n        ImGui::DockSpace(dockspace_id,','mDockspaceID = ImGui::GetID("MyDockSpace");\n        BuildDefaultLayout(); // DockSpace가 노드를 만들기 전에 최초 1회 처리\n        ImGui::DockSpace(mDockspaceID,')
e.block_replace(19,'ImGui::GetMainViewport()->Size','ImGui::GetMainViewport()->WorkSize')
e.block_replace(21,'ImGui::DockBuilderAddNode(mDockspaceID);','ImGui::DockBuilderAddNode(mDockspaceID, ImGuiDockNodeFlags_DockSpace);')
e.block_replace(21,'ImGui::GetMainViewport()->Size','ImGui::GetMainViewport()->WorkSize')
e.block(20,'위 SplitNode 호출의 실제 비율\n왼쪽: 전체 가로의 20%\n오른쪽: 남은 80%의 25% → 전체의 약 20%\n중앙: 전체의 약 60%\n아래 Console: 중앙 영역만 세로 25%로 분할\n왼쪽 내부: Hierarchy와 Project Browser를 상하로 분할\n전체 폭의 Console이 목표라면 먼저 root를 아래쪽으로 분할해야 합니다.','plain text')
e.block_replace(17,'ImGui::MenuItem("Orthographic", nullptr, &mEditorCamera.IsOrthographic);','// 실제 카메라의 getter/setter를 연결하고 projection을 재계산합니다.\n        bool orthographic = mEditorCamera.IsOrthographic();\n        if (ImGui::MenuItem("Orthographic", nullptr, &orthographic))\n            mEditorCamera.SetOrthographic(orthographic);')
e.block(22,'''// 저장할 이름을 경로 구분자가 없는 프리셋 키로 검증한 뒤 사용합니다.
// <filesystem>, <fstream>, <stdexcept> 필요. 디렉터리 위치는 프로젝트 정책입니다.
void SaveLayoutFile(const std::filesystem::path& path)
{
    if (!path.parent_path().empty())
        std::filesystem::create_directories(path.parent_path());
    size_t size = 0;
    const char* data = ImGui::SaveIniSettingsToMemory(&size);
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) throw std::runtime_error("Layout open failed");
    file.write(data, static_cast<std::streamsize>(size));
    file.close();
    if (file.fail()) throw std::runtime_error("Layout write failed");
}
// 불러오기는 파일 읽기 성공 후 LoadIniSettingsFromMemory(data,size).
// 레이아웃 교체는 다음 프레임 UI 제출 전에 한 번 적용하도록 예약합니다.''')
e.block_replace(23,'        std::ofstream file(GetLayoutPath(name));\n        file << data;','        std::filesystem::create_directories("Layouts");\n        std::ofstream file(GetLayoutPath(name), std::ios::binary);\n        if (!file) throw std::runtime_error("Layout open failed");\n        file << data;\n        file.close();\n        if (file.fail()) throw std::runtime_error("Layout write failed");')
e.block(25,'''static bool showWindow = true; // 프레임을 넘어 유지하는 상태
if (showWindow)
{
    if (ImGui::Begin("My Window", &showWindow))
    {
        // 표시되는 창 내용
    }
    ImGui::End(); // Begin이 false여도 대응
}''')
e.block_replace(28,'    ImGuiIO& io = ImGui::GetIO();','    ImGuiIO& io = ImGui::GetIO();\n    if (io.WantTextInput || ImGui::IsAnyItemActive()) return;\n    // 실제 명령은 포커스된 작업 영역과 CanUndo/CanRedo도 검사합니다.')
code=e.code_changes[28][0];code=re.sub(r'ImGui::IsKeyPressed\((ImGuiKey_\w+)\)',r'ImGui::IsKeyPressed(\1, false)',code);e.block(28,code)
e.block(29,'''if (mShowConsole)
{
    if (ImGui::Begin("Console", &mShowConsole))
        RenderConsoleContent();
    ImGui::End();
}
// 포커스/hover가 없어도 보이는 창은 내용을 제출해야 합니다.
// 많은 로그는 ImGuiListClipper 등으로 보이는 행의 비용을 줄입니다.''')
e.block(30,'// Begin과 End 사이에서 IsWindowCollapsed를 검사합니다.\n// 조기 return으로 ImGui::End/PopStyleVar 호출을 건너뛰지 않습니다.')
e.finish('### 도킹 API와 입력 규칙\n\nDockBuilder와 NoTabBar 같은 내부 API는 `imgui_internal.h`에 있으며 버전에 따라 달라집니다. 위 플래그 이름은 현재 저장소의 ImGui 헤더에서 확인한 이름입니다. `PassthruCentralNode`는 중앙을 채우는 옵션이 아닙니다. DockSpace는 도킹되는 창보다 먼저 매 프레임 제출하고, 숨겨진 host라도 도킹 관계 유지가 필요하면 KeepAliveOnly 경로를 검토합니다.\n\nDockBuilder 초기 배치는 저장된 노드가 없을 때 또는 사용자가 Reset을 실행했을 때만 만듭니다. DockWindow에 넘기는 이름은 실제 Begin의 ID와 같아야 합니다. MenuItem의 "Ctrl+S" 문구는 단축키 등록이 아니라 표시이며, 입력 처리와 실제 저장 성공·실패 처리를 별도로 연결합니다. ini는 창 배치만 저장하고 게임 씬 데이터는 저장하지 않습니다.\n\n[확인한 ImGui 헤더](https://github.com/eazuooz/YamYam_Engine/blob/ce56f16dd7a40fd2b4bf03c680790e5b637c7530/Editor_Window/imgui.h)와 [실제 도킹 초기 배치](https://github.com/eazuooz/YamYam_Engine/blob/ce56f16dd7a40fd2b4bf03c680790e5b637c7530/Editor_Window/guiEditorApplication.cpp)를 함께 읽습니다.')

e=Edit(21,'오프스크린 색상 텍스처를 RTV로 그린 뒤 SRV로 읽어 Scene 창에 표시합니다. 픽킹·MRT·후처리는 별도 확장 단계입니다.','Scene 이미지가 창 크기에 맞게 보이고, 렌더링 출력 바인딩과 ImGui 읽기 바인딩이 충돌하지 않습니다.','flowchart LR\n    C[Scene 카메라] --> R[색상 RTV + 깊이 DSV]\n    R --> U[출력 해제]\n    U --> S[색상 SRV]\n    S --> I[ImGui Image]\n    I --> B[백버퍼에 UI 합성]')
e.guide+='\n**현재 코드와 구분:** 이 문서는 DX11의 RT 설계·픽킹 확장 예시입니다. 최신 DX12에서 `ReadPixel()`은 0을 반환하는 stub이며 오브젝트 ID readback은 구현되지 않았습니다. MRT·MSAA·Prefab·FBX import·후처리도 선언이나 설계만으로 완료된 기능이 아닙니다. 실제로 복구한 Scene/Game 카메라 출력과 GPU SRV 연결은 다음 구현 글을 기준으로 읽습니다.\n<mention-page url="'+NEW1+'"/>\n'
e.para('DirectX의 스왑체인은 화면 출력용','백버퍼의 일부 영역에도 viewport/scissor로 그릴 수 있습니다. 다만 ImGui의 도킹·겹침·크기 변화에 맞춰 독립된 카메라 화면을 배치하려면, 먼저 별도 텍스처에 그리고 Image로 합성하는 구조가 다루기 쉽습니다.')
e.para('<td>게임 설정에 따라 고정','<td>고정 해상도나 창 크기 연동 중 정책을 선택합니다. 현재 YamYam Game RT도 패널 크기 변경을 요청합니다.</td>')
e.para('**SHADER_RESOURCE**','**SHADER_RESOURCE — 용도와 포맷 구분**')
e.para('- 셰이더에서 텍스처로 샘플링 가능','- Shader Resource는 접근 용도입니다. DXGI_FORMAT을 대신하는 픽셀 포맷이 아니며, 이 문서의 별도 enum 항목은 구현 정책을 정해야 합니다.')
e.block(0,'Scene 카메라 → 별도 색상 Texture의 RTV에 Draw\n출력 바인딩 해제 → 같은 Texture의 SRV를 ImGui Image로 읽음\n백버퍼 RTV → ImGui DrawData 렌더링 → Present\nDX11 Image의 ID는 SRV 포인터이고 DX12는 GPU descriptor handle입니다.','plain text')
e.block(4,'픽킹 확장: RT0은 RGBA 색상, RT1은 R32_SINT 오브젝트 ID\nPS의 int 출력과 SV_Target1, 두 RTV 바인딩, ID용 블렌드 끔을 함께 설정\n클릭 시 ID 첨부물 인덱스 1에서 읽음 (색상 인덱스 0이 아님)','plain text')
e.block_replace(3,'// 셰이더에서 읽을 수 있는 범용 포맷','// 용도 표시를 위한 설계 항목, 실제 DXGI 포맷 아님')
e.block(8,'Invalidate 구현 순서\n1. 기존 RT/SRV 바인딩 해제 (DX11)\n2. 유효한 크기·포맷·샘플 수를 검증\n3. 새 attachment를 임시 ComPtr/소유 객체에 생성하고 모든 실패 검사\n4. 모두 성공하면 기존 attachment와 교체\n5. viewport와 카메라 aspect 갱신\nDX12는 이전 GPU 사용 완료 전 리소스·descriptor를 재사용하면 안 됩니다.','plain text')
e.block(9,'''// RGBA8, single-sample 색상 타깃 생성의 핵심.
// HRESULT 함수 내부이며 device는 유효한 ID3D11Device*입니다.
D3D11_TEXTURE2D_DESC desc = {};
desc.Width = width; desc.Height = height;
desc.MipLevels = 1; desc.ArraySize = 1;
desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
desc.SampleDesc.Count = 1;
desc.Usage = D3D11_USAGE_DEFAULT;
desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
if (!width || !height) return E_INVALIDARG;
Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
HRESULT hr = device->CreateTexture2D(&desc, nullptr, texture.GetAddressOf());
if (FAILED(hr)) return hr;
hr = device->CreateRenderTargetView(texture.Get(), nullptr, rtv.GetAddressOf());
if (FAILED(hr)) return hr;
hr = device->CreateShaderResourceView(texture.Get(), nullptr, srv.GetAddressOf());
if (FAILED(hr)) return hr;
// 성공한 ComPtr들을 Texture/RenderTarget의 소유 멤버로 이동합니다.
// raw pointer setter는 AddRef인지 소유권 이전(Attach)인지 먼저 정해야 합니다.''')
e.block_replace(10,'    device->CreateTexture2D(&depthDesc, nullptr, &depthTexture);','    HRESULT hr = device->CreateTexture2D(&depthDesc, nullptr, &depthTexture);\n    if (FAILED(hr)) return nullptr;')
e.block_replace(10,'    device->CreateDepthStencilView(depthTexture, nullptr, &dsv);','    hr = device->CreateDepthStencilView(depthTexture, nullptr, &dsv);\n    if (FAILED(hr)) { depthTexture->Release(); return nullptr; }')
e.block_replace(12,'    // 크기가 변경되지 않았으면 무시','    if (!width || !height || width > 8192 || height > 8192) return; // 예제 정책 상한\n    // 크기가 변경되지 않았으면 무시')
e.block_replace(13,'    assert(attachmentIndex < mColorAttachments.size());','    if (attachmentIndex >= mColorAttachments.size() || x < 0 || y < 0 ||\n        UINT(x) >= mSpecification.Width || UINT(y) >= mSpecification.Height ||\n        mSpecification.Samples != 1 ||\n        mColorAttachmentSpecifications[attachmentIndex].TextureFormat != eRenderTargetFormat::RED_INTEGER)\n        return -1;')
e.block_replace(13,'    device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);','    HRESULT hr = device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);\n    if (FAILED(hr)) return -1;')
e.block_replace(13,'    deviceContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);','    hr = deviceContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);\n    if (FAILED(hr)) { stagingTexture->Release(); return -1; }')
e.block(14,'''// ClearRenderTargetView의 입력은 float[4]입니다.
const float clearColor[4] = { 0.f, 0.f, 0.f, 1.f };
deviceContext->ClearRenderTargetView(colorRTV, clearColor);
// int 비트 패턴을 reinterpret_cast<float*>로 넘기지 않습니다.
// 정수 ID 타깃의 정확한 초기값은 아래처럼 int를 출력하는 clear Draw 등으로 설정합니다.
```

**HLSL — R32_SINT 타깃 하나를 slot 0에 바인딩한 전체 화면 clear 패스의 PS**

```hlsl
int ClearObjectID() : SV_Target0
{
    return -1;
}''')
e.block(16,'DX11 swapchain resize 순서\n1. 최소화/0 크기면 건너뜀\n2. 백버퍼를 사용하는 모든 context 바인딩·뷰·참조 해제\n3. ResizeBuffers HRESULT 검사\n4. 새 백버퍼 GetBuffer → RTV 생성, 새 깊이 Texture → DSV 생성\n5. 각 HRESULT를 확인하고 새 viewport 적용\nComPtr 멤버는 Reset으로 해제합니다. Get 포인터에 Release를 따로 호출하지 않습니다.\n백버퍼와 Scene RT의 크기 변경은 각각의 창·패널 크기를 기준으로 처리합니다.','plain text')
e.block_replace(15,'    ::GetClientRect(mHwnd, &winRect);','    if (!::GetClientRect(mHwnd, &winRect)) return;\n    if (winRect.right <= winRect.left || winRect.bottom <= winRect.top) return;')
e.block_replace(19,'    ImGui::Begin("Scene");','    if (!ImGui::Begin("Scene"))\n    { ImGui::End(); ImGui::PopStyleVar(); return; }')
e.block(20,'''// Image를 넣기 직전에 위치와 크기를 기록합니다.
const ImVec2 imageMin = ImGui::GetCursorScreenPos();
const ImVec2 imageSize = ImGui::GetContentRegionAvail();
mViewportBounds[0] = { imageMin.x, imageMin.y };
mViewportBounds[1] = { imageMin.x + imageSize.x, imageMin.y + imageSize.y };
// letterbox/toolbar가 있으면 실제 Image 사각형에 맞춰 기록합니다.''')
e.block_replace(24,'viewportPanelSize.x > 0 && viewportPanelSize.y > 0','viewportPanelSize.x >= 1 && viewportPanelSize.y >= 1')
e.block_replace(25,'    // 3. 카메라 설정','    // 깊이 DSV도 매 패스 시작에 ClearDepthStencilView로 초기화합니다.\n    // 3. 카메라 설정')
e.block_replace(28,'            const wchar_t* path = (const wchar_t*)payload->Data;','            if (payload->DataSize < int(sizeof(wchar_t)) ||\n                payload->DataSize % sizeof(wchar_t) != 0)\n            { ImGui::EndDragDropTarget(); return; }\n            const wchar_t* path = static_cast<const wchar_t*>(payload->Data);\n            const size_t count = payload->DataSize / sizeof(wchar_t);\n            if (path[count - 1] != L\'\\0\')\n            { ImGui::EndDragDropTarget(); return; }')
e.block_replace(29,'if (!mViewportHovered || !ImGui::IsMouseClicked','if (ImGuizmo::IsUsing() || ImGuizmo::IsOver() || !mViewportHovered || !ImGui::IsMouseClicked')
e.block_replace(29,'int entityID = mFrameBuffer->ReadPixel(1, (int)mouseX, (int)mouseY);','const auto& spec = mFrameBuffer->GetSpecification();\n        int pixelX = int(mouseX / mViewportSize.x * spec.Width);\n        int pixelY = int(mouseY / mViewportSize.y * spec.Height);\n        int entityID = mFrameBuffer->ReadPixel(1, pixelX, pixelY);')
e.block_replace(31,'    mFrameBuffer->Resize(','    mEditorCamera.SetViewportSize(mViewportSize.x, mViewportSize.y);\n    mFrameBuffer->Resize(')
e.block(34,'RT 시각화 순서\nRGBA 색상: 일반 ImGui Image로 표시\n정수 ID: ID를 RGB 가짜 색으로 변환하는 전용 셰이더 패스 후 표시\nDepth: 읽기 가능한 typeless 리소스/SRV와 깊이 시각화 패스 구성\nMSAA: 지원 포맷의 resolve 또는 전용 샘플링 패스 후 표시\n모든 attachment를 일반 Texture2D<float4> 샘플러에 그대로 넣지 않습니다.','plain text')
e.finish('### 예제의 연결 조건\n\nRenderTarget 헤더와 생성 코드는 구조 설명이며, helper 선언·Texture setter의 소유권·실패 복구는 실제 클래스에 연결해야 합니다. single-sample RGBA8부터 동작을 확인하고 MSAA는 지원 수 검사·Texture2DMS·resolve 경로를 추가합니다. 깊이를 셰이더에서 읽으려면 이 예제의 D24S8 DSV 전용 생성과 다른 typeless/SRV 설정이 필요합니다.\n\n픽킹의 정수 clear 패스는 깊이·블렌드 끔, 전체 화면 기하, ID용 RTV 바인딩까지 갖춰야 합니다. ReadPixel의 동기 Map은 GPU 대기를 유발할 수 있으며 비동기 결과가 반드시 바로 다음 프레임에 준비된다고 가정하지 않습니다. 삭제된 객체 ID와 readback 요청의 수명도 관리합니다.\n\n실제 Image에 쓸 DX11 SRV와 백버퍼 RTV를 올바르게 바인딩하고, 패널 크기 변경으로 SwapChain을 재생성하지 않습니다. Game 입력은 Scene 입력과 따로 focus/hover 정책을 둡니다. 최신 DX12는 descriptor 지연 회수와 display SRV의 알파 보정을 추가했습니다.\n<mention-page url="'+NEW2+'"/>')
