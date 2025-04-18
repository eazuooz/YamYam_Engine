#include "yaEditorScene.h"
#include "yaGameObject.h"
#include "yaPlayer.h"
#include "yaUIManager.h"
#include "yaSpriteRenderer.h"
#include "yaTitleScene.h"
#include "yaObject.h"
#include "yaTexture.h"
#include "yaResources.h"
#include "yaPlayerScript.h"
#include "yaCamera.h"
#include "yaRenderer.h"
#include "yaSceneCamera.h"

namespace ya
{
	EditorScene::EditorScene()
	{
	}

	EditorScene::~EditorScene()
	{
	}

	void EditorScene::Initialize()
	{
		Scene::Initialize();

				// main camera
		GameObject* camera = object::Instantiate<GameObject>(eLayerType::None, Vector3(0.0f, 0.0f, -10.0f));

		SceneCamera* cameraComp = camera->AddComponent<SceneCamera>();
		cameraComp->SetProjectionType(Camera::eProjectionType::Perspective);
		cameraComp->SetSize(200.0f);

		renderer::mainCamera = cameraComp;


		for (size_t i = 0; i < 1; i++)
		{
			GameObject* player = object::Instantiate<Player>(eLayerType::Player);
			SpriteRenderer* sr = player->AddComponent<SpriteRenderer>();
			sr->SetSprite(Resources::Find<Texture>(L"Player"));
			
			player->AddComponent<PlayerScript>();

			if (renderer::selectedObject == nullptr)
				renderer::selectedObject = player;
		}
	}

	void EditorScene::Update()
	{
		Scene::Update();
	}

	void EditorScene::LateUpdate()
	{
		Scene::LateUpdate();
	}

	void EditorScene::Render()
	{
		Scene::Render();
	}

	void EditorScene::OnEnter()
	{
		Scene::OnEnter();
	}

	void EditorScene::OnExit()
	{
		Scene::OnExit();
	}
}
