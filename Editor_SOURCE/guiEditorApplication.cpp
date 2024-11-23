#include "guiEditorApplication.h"
#include "guiInspectorWindow.h"


#include "..\\YamYamEngine_SOURCE\\yaApplication.h"
#include "..\\YamYamEngine_SOURCE\\yaRenderer.h"
#include "..\\YamYamEngine_SOURCE\\yaGameObject.h"
#include "..\\YamYamEngine_SOURCE\\yaTransform.h"

extern ya::Application application;

namespace gui
{
	ImGuiWindowFlags EditorApplication::mFlag = ImGuiWindowFlags_None;
	ImGuiDockNodeFlags EditorApplication::mDockspaceFlags = ImGuiDockNodeFlags_None;
	EditorApplication::eState EditorApplication::mState = EditorApplication::eState::Active;
	bool EditorApplication::mFullScreen = true;
	bool EditorApplication::mPadding = false;
	std::map<std::wstring, EditorWindow*> EditorApplication::mEditorWindows;

	bool EditorApplication::Initialize()
	{
		imGguiInitialize();

		InspectorWindow* inspector = new InspectorWindow();
		mEditorWindows.insert(std::make_pair(L"InspectorWindow", inspector));

		return true;
	}

	void EditorApplication::Update()
	{

	}

	void EditorApplication::OnGUI()
	{
		imGuiRender();
	}

	void EditorApplication::Run()
	{
		Update();
		OnGUI();
	}

	void EditorApplication::Release()
	{
		for (auto iter : mEditorWindows)
		{
			delete iter.second;
			iter.second = nullptr;
		}

		// Cleanup
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	bool EditorApplication::imGguiInitialize()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;
		//io.ConfigViewportsNoDefaultParent = true;
		//io.ConfigDockingAlwaysTabBar = true;
		//io.ConfigDockingTransparentPayload = true;
		//io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: Experimental. THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
		//io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI: Experimental.

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(application.GetHwnd());

		ya::graphics::GraphicDevice_DX11*& graphicdevice = ya::graphics::GetDevice();
		ID3D11Device* device = graphicdevice->GetID3D11Device().Get();
		ID3D11DeviceContext* device_context = graphicdevice->GetID3D11DeviceContext().Get();

		ImGui_ImplDX11_Init(device, device_context);

		return false;
	}

	void EditorApplication::dockSpaceUpdate()
	{
		if (mState == eState::Disable)
			return;

		// If you strip some features of, this demo is pretty much equivalent to calling DockSpaceOverViewport()!
		// In most cases you should be able to just call DockSpaceOverViewport() and ignore all the code below!
		// In this specific demo, we are not using DockSpaceOverViewport() because:
		// - we allow the host window to be floating/moveable instead of filling the viewport (when opt_fullscreen == false)
		// - we allow the host window to have padding (when opt_padding == true)
		// - we have a local menu bar in the host window (vs. you could use BeginMainMenuBar() + DockSpaceOverViewport() in your code!)
		// TL;DR; this demo is more complicated than what you would normally use.
		// If we removed all the options we are showcasing, this demo would become:
		//     void ShowExampleAppDockSpace()
		//     {
		//         ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		//     }

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		mFlag = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (mFullScreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			mFlag |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			mFlag |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			mDockspaceFlags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (mDockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
			mFlag |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!mPadding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	}
	
	void EditorApplication::dockSpaceOnGui()
	{
		if (mState == eState::Disable)
			return;

		bool Active = static_cast<bool>(mState);
		ImGui::Begin("EditorApplication", &Active, mFlag);
        if (!mPadding)
            ImGui::PopStyleVar();

        if (mFullScreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("YamYamDockingSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), mDockspaceFlags);
        }
        else
        {
			ImGuiIO& io = ImGui::GetIO();
			ImGui::Text("ERROR: Docking is not enabled! See Demo > Configuration.");
			ImGui::Text("Set io.ConfigFlags |= ImGuiConfigFlags_DockingEnable in your code, or ");
			ImGui::SameLine(0.0f, 0.0f);
			if (ImGui::SmallButton("click here"))
				io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        }

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Options"))
            {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.
                ImGui::MenuItem("Fullscreen", NULL, &mFullScreen);
                ImGui::MenuItem("Padding", NULL, &mPadding);
                ImGui::Separator();

                if (ImGui::MenuItem("Flag: NoSplit",                "", (mDockspaceFlags & ImGuiDockNodeFlags_NoSplit) != 0)) { mDockspaceFlags ^= ImGuiDockNodeFlags_NoSplit; }
                if (ImGui::MenuItem("Flag: NoResize",               "", (mDockspaceFlags & ImGuiDockNodeFlags_NoResize) != 0)) { mDockspaceFlags ^= ImGuiDockNodeFlags_NoResize; }
                if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (mDockspaceFlags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { mDockspaceFlags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
                if (ImGui::MenuItem("Flag: AutoHideTabBar",         "", (mDockspaceFlags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { mDockspaceFlags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
                if (ImGui::MenuItem("Flag: PassthruCentralNode",    "", (mDockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, mDockspaceFlags)) { mDockspaceFlags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
                ImGui::Separator();

                if (ImGui::MenuItem("Close", NULL, false, &Active != NULL))
					mState = eState::Disable;

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        ImGui::End();
	}

	void EditorApplication::imGuiRender()
	{
		// Load Fonts
		// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
		// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
		// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
		// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
		// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
		// - Read 'docs/FONTS.md' for more instructions and details.
		// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
		//io.Fonts->AddFontDefault();
		//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
		//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
		//IM_ASSERT(font != NULL);

		// Our state
		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuiIO& io = ImGui::GetIO();

		dockSpaceUpdate();
		dockSpaceOnGui();

		for (auto iter : mEditorWindows)
			iter.second->Run();

		////imGuizmo
		//ImGuiIO& io = ImGui::GetIO();

		//ImGuizmo::SetOrthographic(false/*!isPerspective*/);
		//ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);

		//ImGuizmo::BeginFrame();

		//UINT width = application.GetWidth();
		//UINT height = application.GetHeight();
		//float windowWidth = (float)ImGui::GetWindowWidth();
		//float windowHeight = (float)ImGui::GetWindowHeight();

		//RECT rect = { 0, 0, 0, 0 };
		//::GetClientRect(application.GetHwnd(), &rect);

		//// Transform start
		////ImGuizmo::SetRect(0, 0, width, height);
		//ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

		//Matrix viewMatirx;
		//Matrix projectionMatirx;

		//if (ya::renderer::mainCamera)
		//{
		//	viewMatirx = ya::renderer::mainCamera->GetViewMatrix();
		//	projectionMatirx = ya::renderer::mainCamera->GetProjectionMatrix();
		//}

		//Matrix modelMatrix;
		//if (ya::renderer::selectedObject)
		//{
		//	modelMatrix = ya::renderer::selectedObject->GetComponent<ya::Transform>()->GetWorldMatrix();
		//}

		//ImGuizmo::Manipulate(*viewMatirx.m, *projectionMatirx.m,
		//	ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::LOCAL, *modelMatrix.m);

		//ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);


		//demo window
		//// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		//if (show_demo_window)
		//	ImGui::ShowDemoWindow(&show_demo_window);

		//// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		//{
		//	static float f = 0.0f;
		//	static int counter = 0;

		//	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		//	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		//	ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		//	ImGui::Checkbox("Another Window", &show_another_window);

		//	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		//	ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		//	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		//		counter++;
		//	ImGui::SameLine();
		//	ImGui::Text("counter = %d", counter);

		//	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		//	ImGui::End();
		//}

		//// 3. Show another simple window.
		//if (show_another_window)
		//{
		//	ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		//	ImGui::Text("Hello from another window!");
		//	if (ImGui::Button("Close Me"))
		//		show_another_window = false;
		//	ImGui::End();
		//}

		// Rendering
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}
}