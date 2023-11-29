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
		// main camera
		GameObject* camera = object::Instantiate<GameObject>(enums::eLayerType::Particle, Vector2(344.0f, 442.0f));
		Camera* cameraComp = camera->AddComponent<Camera>();
		renderer::mainCamera = cameraComp;

		mPlayer = object::Instantiate<Player>(enums::eLayerType::Particle);
		//SpriteRenderer* sr = mPlayer->AddComponent<SpriteRenderer>();
		//sr->SetSize(Vector2(3.0f, 3.0f));
		mPlayer->AddComponent<PlayerScript>();

		//graphcis::Texture* packmanTexture = Resources::Find<graphcis::Texture>(L"MapleEffect");
		//Animator* animator = mPlayer->AddComponent<Animator>();
		//animator->CreateAnimation(L"CatFrontMove", packmanTexture
		//	, Vector2(0.0f, 0.0f), Vector2(386.0f, 246.0f), Vector2::Zero, 8, 0.1f);
		//animator->PlayAnimation(L"CatFrontMove", true);

		graphcis::Texture* packmanTexture = Resources::Find<graphcis::Texture>(L"Cat");
		Animator* animator = mPlayer->AddComponent<Animator>();
		animator->CreateAnimation(L"DownWalk", packmanTexture
			, Vector2(0.0f, 0.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
		animator->CreateAnimation(L"RightWalk", packmanTexture
			, Vector2(0.0f, 32.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
		animator->CreateAnimation(L"UpWalk", packmanTexture
			, Vector2(0.0f, 64.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
		animator->CreateAnimation(L"LeftWalk", packmanTexture
			, Vector2(0.0f, 96.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
		animator->CreateAnimation(L"SitDown", packmanTexture
			, Vector2(0.0f, 128.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
		animator->CreateAnimation(L"Grooming", packmanTexture
			, Vector2(0.0f, 160.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);

		animator->PlayAnimation(L"SitDown", false);


		mPlayer->GetComponent<Transform>()->SetPosition(Vector2(100.0f, 100.0f));
		mPlayer->GetComponent<Transform>()->SetScale(Vector2(2.0f, 2.0f));
		//mPlayer->GetComponent<Transform>()->SetRotation(30.0f);
		/*sr->SetTexture(packmanTexture);*/

		GameObject* bg = object::Instantiate<GameObject>(enums::eLayerType::Player);
		SpriteRenderer* bgSr = bg->AddComponent<SpriteRenderer>();
		//bgSr->SetSize(Vector2(3.0f, 3.0f));

		graphcis::Texture* bgTexture = Resources::Find<graphcis::Texture>(L"Bubble");
		bgSr->SetTexture(bgTexture);
		
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
