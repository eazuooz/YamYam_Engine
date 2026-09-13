#include "guiSceneWindow.h"
#include "..\\YamYamEngine_CORE\\yaRenderer.h"

#include "..\\YamYamEngine_CORE\\yaTransform.h"
#include "..\\YamYamEngine_CORE\\yaApplication.h"
#include "..\\YamYamEngine_CORE\\yaWindow.h"
#include "..\\YamYamEngine_CORE\\yaSceneManager.h"
#include "guiEditorApplication.h"

extern ya::Application application;
namespace gui
{
	SceneWindow::SceneWindow()
		: mEditorCameraObject(nullptr)
		, mEditorCamera(nullptr)
		, ViewportFocused(false)
		, ViewportHovered(false)
		, GuizmoType(-1)
		, ViewportBounds{}
		, ViewportSize{}
	{
		SetName("Scene");
		SetSize(ImVec2(300, 600));

		Initialize();
	}

	SceneWindow::~SceneWindow()
	{
		delete mEditorCameraObject;
		mEditorCameraObject = nullptr;
	}

	void SceneWindow::Initialize()
	{
		mEditorCameraObject = new ya::GameObject();
		mEditorCameraObject->SetName(L"EditorCamera");
		
		ya::Transform* tr = mEditorCameraObject->GetComponent<ya::Transform>();
		tr->SetPosition(3.0f, 0.0f, -20.0f);
		tr->SetRotation(0.0f, 0.0f, 0.0f);
		
		mEditorCamera = mEditorCameraObject->AddComponent<ya::EditorCamera>();
		mEditorCamera->SetProjectionType(ya::Camera::eProjectionType::Perspective);
		
		// set the render target for the editor camera
		const ya::Window::WindowData& windowData = application.GetWindow().GetData();
		mEditorCamera->CreateRenderTarget(windowData.Width, windowData.Height);
	}

	void SceneWindow::Update()
	{
		for (Editor* editor : mEditors)
		{
			editor->Update();
		}
	}

	void SceneWindow::OnGUI()
	{
		for (Editor* editor : mEditors)
		{
			editor->OnGUI();
		}
	}

	void SceneWindow::Run()
	{
		bool Active = (bool)GetState();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        const bool visible = ImGui::Begin(GetName().c_str(), &Active, GetFlag());
        ViewportFocused = ImGui::IsWindowFocused();
        ViewportHovered = ImGui::IsWindowHovered();
        const ImVec2 panelSize = ImGui::GetContentRegionAvail();
        if (!visible || panelSize.x < 1.0f || panelSize.y < 1.0f)
        {
            ImGui::End();
            ImGui::PopStyleVar();
            return;
        }
        ViewportSize = Vector2(panelSize.x, panelSize.y);
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        ViewportBounds[0] = Vector2(origin.x, origin.y);
        ViewportBounds[1] = Vector2(origin.x + panelSize.x, origin.y + panelSize.y);

        Update();
        OnGUI();
        auto* cameraTr = mEditorCameraObject->GetComponent<ya::Transform>();
        cameraTr->LateUpdate();
        auto* frameBuffer = mEditorCamera->GetRenderTarget();
        frameBuffer->RequestResize(UINT(panelSize.x), UINT(panelSize.y));
        frameBuffer->Bind();
        auto* scene = ya::SceneManager::GetActiveScene();
        ya::renderer::RenderSceneFromCamera(scene, mEditorCamera);
        ya::renderer::RenderSceneFromCamera(ya::SceneManager::GetDontDestroyOnLoad(), mEditorCamera);
        frameBuffer->Unbind();

        const auto texture = frameBuffer->GetDisplaySRV();
        ImGui::Image(ImTextureID(texture.ptr), panelSize);

		// To do : guizmo
		ya::GameObject* selectedObject = ya::renderer::selectedObject;
		if (selectedObject && GuizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetGizmoSizeClipSpace(0.15f); 
			ImGuizmo::SetRect(ViewportBounds[0].x, ViewportBounds[0].y
				, ViewportBounds[1].x - ViewportBounds[0].x, ViewportBounds[1].y - ViewportBounds[0].y);

			// To do : guizmo...
			// game view camera setting

			// Scene Camera
			const ya::math::Matrix& viewMatrix = mEditorCamera->GetViewMatrix();
			const ya::math::Matrix& projectionMatrix = mEditorCamera->GetProjectionMatrix();

			// Object Transform
			ya::Transform* transform = selectedObject->GetComponent<ya::Transform>();
			ya::math::Matrix worldMatrix = transform->GetWorldMatrix();

			// snapping
			bool snap = ImGui::GetIO().KeyCtrl;
			float snapValue = 0.5f; 

			// snap to 45 degrees for rotation
			if (GuizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(*viewMatrix.m, *projectionMatrix.m, static_cast<ImGuizmo::OPERATION>(GuizmoType)
				, ImGuizmo::WORLD, *worldMatrix.m, nullptr, snap ? snapValues : nullptr);

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

		// repair the default render target
		// ya::graphics::GetDevice<ya::graphics::GraphicDevice_DX11>()->BindDefaultRenderTarget();

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void SceneWindow::OnEnable()
	{
	}

	void SceneWindow::OnDisable()
	{
	}

	void SceneWindow::OnDestroy()
	{
	}

}