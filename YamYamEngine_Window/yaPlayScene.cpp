#include "yaPlayScene.h"
#include "yaGameObject.h"
#include "yaPlayer.h"
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
#include "yaCollisionManager.h"

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
		CollisionManager::CollisionLayerCheck(eLayerType::Player, eLayerType::Animal, true);

		// main camera
		GameObject* camera = object::Instantiate<GameObject>(enums::eLayerType::Particle, Vector2(344.0f, 442.0f));
		Camera* cameraComp = camera->AddComponent<Camera>();
		renderer::mainCamera = cameraComp;

		mPlayer = object::Instantiate<Player>(enums::eLayerType::Particle);
		PlayerScript* plScript = mPlayer->AddComponent<PlayerScript>();
		BoxCollider2D* collider = mPlayer->AddComponent<BoxCollider2D>();
		collider->SetOffset(Vector2(-50.0f, -50.0));

		graphcis::Texture* playerTex = Resources::Find<graphcis::Texture>(L"Player");
		Animator* playerAnimator = mPlayer->AddComponent<Animator>();
		playerAnimator->CreateAnimation(L"Idle", playerTex
			, Vector2(2000.0f, 250.0f), Vector2(250.0f, 250.0f), Vector2::Zero, 1, 0.1f);
		playerAnimator->CreateAnimation(L"FrontGiveWater", playerTex
			, Vector2(0.0f, 2000.0f), Vector2(250.0f, 250.0f), Vector2::Zero, 12, 0.1f);
		playerAnimator->PlayAnimation(L"Idle", false);

		playerAnimator->GetCompleteEvent(L"FrontGiveWater") = std::bind(&PlayerScript::AttackEffect, plScript);

		//playerAnimator->

		mPlayer->GetComponent<Transform>()->SetPosition(Vector2(300.0f, 250.0f));
		//mPlayer->GetComponent<Transform>()->SetScale(Vector2(2.0f, 2.0f));


		///CAT
		Cat* cat = object::Instantiate<Cat>(enums::eLayerType::Animal);
		//cat->SetActive(true);
		cat->AddComponent<CatScript>();
		//cameraComp->SetTarget(cat);
		graphcis::Texture* catTex = Resources::Find<graphcis::Texture>(L"Cat");
		Animator* catAnimator = cat->AddComponent<Animator>();

		BoxCollider2D* boxCatCollider = cat->AddComponent<BoxCollider2D>();

		boxCatCollider->SetOffset(Vector2(-50.0f, -50.0f));

		//catAnimator->CreateAnimation(L"DownWalk", catTex
		//	, Vector2(0.0f, 0.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
		//catAnimator->CreateAnimation(L"RightWalk", catTex
		//	, Vector2(0.0f, 32.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
		//catAnimator->CreateAnimation(L"UpWalk", catTex
		//	, Vector2(0.0f, 64.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
		//catAnimator->CreateAnimation(L"LeftWalk", catTex
		//	, Vector2(0.0f, 96.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
		//catAnimator->CreateAnimation(L"SitDown", catTex
		//	, Vector2(0.0f, 128.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
		//catAnimator->CreateAnimation(L"Grooming", catTex
		//	, Vector2(0.0f, 160.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
		//catAnimator->CreateAnimation(L"LayDown", catTex
		//	, Vector2(0.0f, 192.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);

		//catAnimator->PlayAnimation(L"SitDown", false);
		catAnimator->CreateAnimationByFolder(L"MushroomIdle", L"..\\Resources\\Mushroom", Vector2::Zero, 0.1f);

		catAnimator->PlayAnimation(L"MushroomIdle", true);

		cat->GetComponent<Transform>()->SetPosition(Vector2(200.0f, 200.0f));
		cat->GetComponent<Transform>()->SetScale(Vector2(1.0f, 1.0f));
		

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
	void PlayScene::Render(HDC hdc)
	{
		Scene::Render(hdc);
	}
	void PlayScene::OnEnter()
	{
	}
	void PlayScene::OnExit()
	{
	}
}
