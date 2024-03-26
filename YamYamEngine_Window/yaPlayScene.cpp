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
#include "yaCatScript.h"
#include "yaBoxCollider2D.h"
#include "yaCircleCollider2D.h"
#include "yaCollisionManager.h"
#include "yaTile.h"
#include "yaTilemapRenderer.h"
#include "yaRigidbody.h"
#include "yaFloor.h"
#include "yaFloorScript.h"
#include "yaAudioClip.h"
#include "yaAudioListener.h"
#include "yaAudioSource.h"

namespace ya
{
	PlayScene::PlayScene()
		: mPlayer(nullptr)
	{
	}
	PlayScene::~PlayScene()
	{
	}
	void PlayScene::Initialize()
	{
		

		//// main camera
		//GameObject* camera = object::Instantiate<GameObject>(enums::eLayerType::Particle, Vector2(344.0f, 442.0f));
		//Camera* cameraComp = camera->AddComponent<Camera>();
		//renderer::mainCamera = cameraComp;

		mPlayer = object::Instantiate<Player>(enums::eLayerType::Player);
		object::DontDestroyOnLoad(mPlayer);
		//mPlayer->AddComponent<AudioListener>();


		//PlayerScript* plScript = mPlayer->AddComponent<PlayerScript>();

		//BoxCollider2D* collider = mPlayer->AddComponent<BoxCollider2D>();
		//collider->SetOffset(Vector2(-50.0f, -50.0));

		//graphics::Texture* playerTex = Resources::Find<graphics::Texture>(L"Player");
		//Animator* playerAnimator = mPlayer->AddComponent<Animator>();
		//playerAnimator->CreateAnimation(L"Idle", playerTex
		//	, Vector2(2000.0f, 250.0f), Vector2(250.0f, 250.0f), Vector2::Zero, 1, 0.1f);
		//playerAnimator->CreateAnimation(L"FrontGiveWater", playerTex
		//	, Vector2(0.0f, 2000.0f), Vector2(250.0f, 250.0f), Vector2::Zero, 12, 0.1f);
		//playerAnimator->PlayAnimation(L"Idle", false);

		//playerAnimator->GetCompleteEvent(L"FrontGiveWater") = std::bind(&PlayerScript::AttackEffect, plScript);

		//mPlayer->GetComponent<Transform>()->SetPosition(Vector2(300.0f, 250.0f));
		//mPlayer->AddComponent<Rigidbody>();


		//Floor* floor = object::Instantiate<Floor>(eLayerType::Floor, Vector2(0.0f, 0.0f));
		//floor->SetName(L"Floor");
		//SpriteRenderer* floorSR = floor->AddComponent<SpriteRenderer>();
		//floorSR->SetTexture(Resources::Find<graphics::Texture>(L"PixelMap"));

		//AudioSource* as = floor->AddComponent<AudioSource>();

		//plScript->SetPixelMapTexture(Resources::Find<graphics::Texture>(L"PixelMap"));

		////BoxCollider2D* floorCol = floor->AddComponent<BoxCollider2D>();
		////floorCol->SetSize(Vector2(3.0f, 1.0f));
		////floor->AddComponent<FloorScript>();


		//AudioClip* ac = Resources::Load<AudioClip>(L"BGSound", L"..\\Resources\\Sound\\smw_bonus_game_end.wav");
		//as->SetClip(ac);
		//as->Play();
		// 게임 오브젝트 생성후에 레이어와 게임오브젝트들의 init함수를 호출
		Scene::Initialize();
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
