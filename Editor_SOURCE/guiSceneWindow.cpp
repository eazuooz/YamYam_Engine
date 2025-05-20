#include "guiSceneWindow.h"
#include "..\\YamYamEngine_SOURCE\\yaRenderer.h"

#include "..\\YamYamEngine_SOURCE\\yaTransform.h"
#include "..\\YamYamEngine_SOURCE\\yaGraphicDevice_DX11.h"
#include "..\\YamYamEngine_SOURCE\\yaApplication.h"
#include "..\\YamYamEngine_SOURCE\\yaWindow.h"
#include "..\\YamYamEngine_SOURCE\\yaSceneManager.h"
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
		tr->SetPosition(0.0f, 0.0f, -20.0f);
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
		ImGui::Begin(GetName().c_str(), &Active, GetFlag());

		Update();
		OnGUI();

		// Calculate view, projection, and camera position
		ya::Transform* cameraTr = mEditorCamera->GetOwner()->GetComponent<ya::Transform>();
		cameraTr->LateUpdate();
		mEditorCamera->LateUpdate();
		
		// clear the render target view & depth stencil view
		ya::graphics::RenderTarget* rt = mEditorCamera->GetRenderTarget();
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv = rt->GetAttachmentTexture(0)->GetRTV();
		ya::graphics::GetDevice()->ClearRenderTargetView(rtv);
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv = rt->GetDepthAttachment()->GetDSV();
		ya::graphics::GetDevice()->ClearDepthStencilView(dsv);

		// set scene view render target & depth stencil view
		ya::graphics::GetDevice()->BindRenderTargets(1, rtv.GetAddressOf(), dsv.Get());

		// render the scene
		Matrix viewMatrix = mEditorCamera->GetViewMatrix();
		Matrix projectionMatrix = mEditorCamera->GetProjectionMatrix();
		Vector3 cameraPos = mEditorCamera->GetOwner()->GetComponent<ya::Transform>()->GetPosition();

		std::vector<ya::GameObject*> opaqueList = {};
		std::vector<ya::GameObject*> cutoutList = {};
		std::vector<ya::GameObject*> transparentList = {};

		// collect randerables(game objects)
		ya::Scene* scene = ya::SceneManager::GetActiveScene();
		ya::renderer::CollectRenderables(scene, opaqueList, cutoutList, transparentList);

		// soring renderables by distance (between camera and game object)
		ya::renderer::SortByDistance(opaqueList, cameraPos, true);
		ya::renderer::SortByDistance(cutoutList, cameraPos, true);
		ya::renderer::SortByDistance(transparentList, cameraPos, false);

		//render game objects
		ya::renderer::RenderRenderables(opaqueList, viewMatrix, projectionMatrix);
		ya::renderer::RenderRenderables(cutoutList, viewMatrix, projectionMatrix);
		ya::renderer::RenderRenderables(transparentList, viewMatrix, projectionMatrix);

		// imgui scene view viewport
		const auto viewportMinRegion = ImGui::GetWindowContentRegionMin(); // ¾ÀºäÀÇ ÃÖ¼Ò ÁÂÇ¥
		const auto viewportMaxRegion = ImGui::GetWindowContentRegionMax(); // ¾ÀºäÀÇ ÃÖ´ë ÁÂÇ¥
		const auto viewportOffset = ImGui::GetWindowPos(); // ¾ÀºäÀÇ À§Ä¡

		constexpr int letTop = 0;
		constexpr int rightBottom = 1;
		ViewportBounds[letTop] = Vector2{ viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		ViewportBounds[rightBottom] = Vector2{ viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };


		// get the camera render target view
		// rendering framebuffer image to the sceneview
		ya::graphics::RenderTarget* frameBuffer = mEditorCamera->GetRenderTarget();
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		ya::math::Vector2 ViewportSize = Vector2{ viewportPanelSize.x, viewportPanelSize.y };
		ya::graphics::Texture* texture = frameBuffer->GetAttachmentTexture(0);
		ImGui::Image((ImTextureID)texture->GetSRV().Get(), ImVec2{ ViewportSize.x, ViewportSize.y }
					, ImVec2{ 0, 0 }, ImVec2{ 1, 1 });

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
			bool snap = ya::Input::GetKey(ya::eKeyCode::Leftcontrol);
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
		ya::graphics::GetDevice()->BindDefaultRenderTarget();

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