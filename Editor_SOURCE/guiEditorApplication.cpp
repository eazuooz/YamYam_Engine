#include "guiEditorApplication.h"
#include "guiInspectorWindow.h"


#include "..\\YamYamEngine_SOURCE\\yaApplication.h"
#include "..\\YamYamEngine_SOURCE\\yaRenderer.h"
#include "..\\YamYamEngine_SOURCE\\yaRenderer.h"
#include "..\\YamYamEngine_SOURCE\\yaGameObject.h"
#include "..\\YamYamEngine_SOURCE\\yaTransform.h"
#include "..\\YamYamEngine_SOURCE\\yaRenderer.h"
#include "..\\YamYamEngine_SOURCE\\yaInput.h"
#include "..\\YamYamEngine_SOURCE\\yaMouseEvent.h"

extern ya::Application application;

namespace gui
{
	ImguiEditor* EditorApplication::mImguiEditor = nullptr;
	std::map<std::wstring, EditorWindow*> EditorApplication::mEditorWindows;
	ImGuiWindowFlags EditorApplication::mFlag = ImGuiWindowFlags_None;
	ImGuiDockNodeFlags EditorApplication::mDockspaceFlags = ImGuiDockNodeFlags_None;
	EditorApplication::eState EditorApplication::mState = EditorApplication::eState::Active;
	bool EditorApplication::mFullScreen = true;
	ya::math::Vector2 EditorApplication::mViewportBounds[2] = {};
	ya::math::Vector2 EditorApplication::mViewportSize;
	bool EditorApplication::mViewportFocused = false;
	bool EditorApplication::mViewportHovered = false;
	int EditorApplication::mGuizmoType = -1;

	ya::graphics::RenderTarget* EditorApplication::mFrameBuffer = nullptr;
	ya::EventCallbackFn EditorApplication::mEventCallback = nullptr;

	bool EditorApplication::Initialize()
	{
		mImguiEditor = new ImguiEditor();
		mFrameBuffer = ya::renderer::FrameBuffer;
		mImguiEditor->Initialize();
		InspectorWindow* inspector = new InspectorWindow();
		mEditorWindows.insert(std::make_pair(L"InspectorWindow", inspector));
		mEventCallback = &EditorApplication::OnEvent;

		return true;
	}

	void EditorApplication::Update()
	{

	}

	void EditorApplication::OnGUI()
	{
		mImguiEditor->Begin();
		OnImGuiRender();
		mImguiEditor->End();
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
		delete mImguiEditor;
		mImguiEditor = nullptr;
	}

	void EditorApplication::OnEvent(ya::Event& e)
	{
		if (!e.Handled)
		{
			mImguiEditor->OnEvent(e);
		}
	}

	void EditorApplication::OpenProject()
	{

	}

	void EditorApplication::NewScene()
	{

	}

	void EditorApplication::SaveScene()
	{

	}

	void EditorApplication::SaveSceneAs()
	{

	}

	void EditorApplication::OpenScene(const std::filesystem::path& path)
	{


	}

	void EditorApplication::OnImGuiRender()
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

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		mFlag = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (mFullScreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			mFlag |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			mFlag |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
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
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		bool Active = static_cast<bool>(mState);
		ImGui::Begin("EditorApplication", &Active, mFlag);
        ImGui::PopStyleVar();

        if (mFullScreen)
            ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), mDockspaceFlags);
		}

		style.WindowMinSize.x = minWinSizeX;

        if (ImGui::BeginMenuBar())
        {
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
					OpenProject();

				ImGui::Separator();

				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
					NewScene();

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
					SaveScene();

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
					application.Close();
				
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Script"))
			{
				if (ImGui::MenuItem("Reload assembly", "Ctrl+R"))
				{
					//ScriptEngine::ReloadAssembly(); ÃßÈÄ C#½ºÅ©¸³Æ® Ãß°¡±â´ÉÀÌ »ý±â¸é Ãß°¡ÇÒ ¿¹Á¤
				}
				
				ImGui::EndMenu();
			}

				
			ImGui::EndMenuBar();
		}

		for (auto iter : mEditorWindows)
			iter.second->Run();
		
		// viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Scene");
		
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin(); // ¾ÀºäÀÇ ÃÖ¼Ò ÁÂÇ¥
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax(); // ¾ÀºäÀÇ ÃÖ´ë ÁÂÇ¥
		auto viewportOffset = ImGui::GetWindowPos(); // ¾ÀºäÀÇ À§Ä¡

		const int letTop = 0;
		const int rightBottom = 1;
		mViewportBounds[letTop] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		mViewportBounds[rightBottom] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		// check if the mouse,keyboard is on the Sceneview
		mViewportFocused = ImGui::IsWindowFocused();
		mViewportHovered = ImGui::IsWindowHovered();

		// to do : mouse, keyboard event
		mImguiEditor->BlockEvent(!mViewportHovered);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		mViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
		ya::graphics::Texture* texture = mFrameBuffer->GetAttachmentTexture(0);
		ImGui::Image((ImTextureID)texture->GetSRV().Get(), ImVec2{ mViewportSize.x, mViewportSize.y }
					, ImVec2{ 0, 0 }, ImVec2{ 1, 1 });

		// Open Scene by drag and drop
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PROJECT_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				OpenScene(path);
			}
			ImGui::EndDragDropTarget();
		}

		// To do : guizmo
		ya::GameObject* selectedObject = ya::renderer::selectedObject;
		mGuizmoType = ImGuizmo::OPERATION::TRANSLATE;
		if (selectedObject && mGuizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(mViewportBounds[0].x, mViewportBounds[0].y
				, mViewportBounds[1].x - mViewportBounds[0].x, mViewportBounds[1].y - mViewportBounds[0].y);

			// To do : guizmo...
			// game view camera setting

			// Scene Camera
			const ya::math::Matrix& viewMatrix = ya::renderer::mainCamera->GetViewMatrix();
			const ya::math::Matrix& projectionMatrix = ya::renderer::mainCamera->GetProjectionMatrix();

			// Object Transform
			ya::Transform* transform = selectedObject->GetComponent<ya::Transform>();
			ya::math::Matrix worldMatrix = transform->GetWorldMatrix();

			// snapping
			bool snap = ya::Input::GetKey(ya::eKeyCode::Leftcontrol);
			float snapValue = 0.5f; 

			// snap to 45 degrees for rotation
			if (mGuizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(*viewMatrix.m, *projectionMatrix.m, static_cast<ImGuizmo::OPERATION>(mGuizmoType)
				, ImGuizmo::LOCAL, *worldMatrix.m, nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				// Decompose matrix to translation, rotation and scale
				float translation[3];
				float rotation[3];
				float scale[3];
				ImGuizmo::DecomposeMatrixToComponents(*worldMatrix.m, translation, rotation, scale);

				// delta rotation from the current rotation
				ya::math::Vector3 deltaRotation = Vector3(rotation) - transform->GetRotation();
				deltaRotation = transform->GetRotation() + deltaRotation;
				
				// set the new transform
				transform->SetScale(Vector3(scale));
				transform->SetRotation(Vector3(deltaRotation));
				transform->SetPosition(Vector3(translation));
			}
		}

		ImGui::End();	// Scene end
		ImGui::PopStyleVar();

		ImGui::End(); // dockspace end
	}

	
	// Event
	void EditorApplication::SetCursorPos(double x, double y)
	{
		ya::MouseMovedEvent event(x, y);

		if (mEventCallback)
			mEventCallback(event);
	}
}