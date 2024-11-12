#include "yaPlayScene.h"
#include "yaGameObject.h"
#include "yaPlayer.h"
#include "yaUIManager.h"
#include "yaTransform.h"
#include "yaSpriteRenderer.h"
#include "yaInput.h"
#include "yaTitleScene.h"
#include "yaSceneManager.h"
#include "yaObject.h"
#include "yaTexture.h"
#include "yaResources.h"
#include "yaPlayerScript.h"
#include "yaCamera.h"
#include "yaRenderer.h"
#include "yaAnimator.h"
#include "yaCat.h"
#include "yaBoxCollider2D.h"
#include "yaCircleCollider2D.h"
#include "yaCollisionManager.h"
#include "yaTile.h"
#include "yaTilemapRenderer.h"
#include "yaRigidbody.h"
#include "yaFloor.h"
#include "yaAudioClip.h"
#include "yaAudioListener.h"
#include "yaAudioSource.h"
#include "yaGraphicDevice_DX11.h"
#include "yaSpriteRenderer.h"
#include "yaCameraScript.h"
#include "yaMaterial.h"

namespace ya
{
	PlayScene::PlayScene()
	{
	}

	PlayScene::~PlayScene()
	{
	}

	void PlayScene::Initialize()
	{
		Scene::Initialize();

		// main camera
		GameObject* camera = object::Instantiate<GameObject>(eLayerType::None, Vector3(0.0f, 0.0f, -10.0f));

		Camera* cameraComp = camera->AddComponent<Camera>();
		cameraComp->SetProjectionType(Camera::eProjectionType::Perspective);
		cameraComp->SetSize(200.0f);

		CameraScript* cameraScript = camera->AddComponent<CameraScript>();
		renderer::mainCamera = cameraComp;

		GameObject* player = object::Instantiate<Player>(eLayerType::Player);
		object::DontDestroyOnLoad(player);

		SpriteRenderer* sr = player->AddComponent<SpriteRenderer>();
		sr->SetSprite(Resources::Find<Texture>(L"Player"));

		renderer::selectedObject = player;
	}

	void PlayScene::Update()
	{
		Scene::Update();
	}

	void PlayScene::LateUpdate()
	{
		Scene::LateUpdate();

		if (Input::GetKeyDown(eKeyCode::N))
		{
			SceneManager::LoadScene(L"TitleScene");
		}
	}

	void PlayScene::Render()
	{
		Scene::Render();
	}

	void PlayScene::OnEnter()
	{
		Scene::OnEnter();

		//CollisionManager::CollisionLayerCheck(eLayerType::Player, eLayerType::Animal, true);
		//CollisionManager::CollisionLayerCheck(eLayerType::Player, eLayerType::Floor, true);

		//UIManager::Push(eUIType::Button);
	}

	void PlayScene::OnExit()
	{
		//UIManager::Pop(eUIType::Button);

		Scene::OnExit();
	}
}
