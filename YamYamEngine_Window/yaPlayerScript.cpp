#include "yaPlayerScript.h"
#include "yaInput.h"
#include "yaTransform.h"
#include "yaTime.h"
#include "yaGameObject.h"
#include "yaAnimator.h"
#include "yaCat.h"
#include "yaCatScript.h"
#include "yaObject.h"
#include "yaResources.h"
#include "yaRigidbody.h"
#include "yaUIManager.h"

namespace ya
{
	PlayerScript::PlayerScript()
		: mState(PlayerScript::eState::Idle)
		, mAnimator(nullptr)
	{
	}
	PlayerScript::~PlayerScript()
	{
	}
	void PlayerScript::Initialize()
	{

	}
	void PlayerScript::Update()
	{
		//if (mAnimator == nullptr)
		//{
		//	mAnimator = GetOwner()->GetComponent<Animator>();
		//}
		//
		//switch (mState)
		//{
		//case ya::PlayerScript::eState::Idle:
		//	idle();
		//	break;
		//case ya::PlayerScript::eState::Walk:
		//	move();
		//	break;

		//case ya::PlayerScript::eState::Sleep:
		//	break;
		//case ya::PlayerScript::eState::GiveWater:
		//	giveWater();
		//	break;

		//case ya::PlayerScript::eState::Attack:
		//	break;
		//default:
		//	break;
		//}
		//
		//Transform* tr = GetOwner()->GetComponent<Transform>();
		//Vector2 pos = tr->GetPosition();
		//COLORREF color = mPixelMap->GetPixel(pos.x, pos.y + 50);

		//Rigidbody* playerRb = GetOwner()->GetComponent<Rigidbody>();
		//if (color == RGB(255, 0, 0))
		//{
		//	playerRb->SetGround(true);

		//	pos.y -= 1;
		//	tr->SetPosition(pos);
		//}
		//else
		//{
		//	playerRb->SetGround(false);
		//}

	}
	void PlayerScript::LateUpdate()
	{
	}
	void PlayerScript::Render()
	{
	}

	void PlayerScript::AttackEffect()
	{
		//Cat* cat = object::Instantiate<Cat>(enums::eLayerType::Animal);
		//CatScript* catSrc = cat->AddComponent<CatScript>();

		//catSrc->SetPlayer(GetOwner());

		//graphics::Texture* catTex = Resources::Find<graphics::Texture>(L"Cat");
		//Animator* catAnimator = cat->AddComponent<Animator>();
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

		//Transform* tr = GetOwner()->GetComponent<Transform>();

		//cat->GetComponent<Transform>()->SetPosition(tr->GetPosition());
		//cat->GetComponent<Transform>()->SetScale(Vector2(2.0f, 2.0f));


		//Vector2 mousePos = Input::GetMousePosition();
		//catSrc->mDest = mousePos;
	}

	void PlayerScript::OnCollisionEnter(Collider* other)
	{
		//other->GetOwner()->GetComponent<Transform>()->SetPosition(Vector2(400.0f, 500.0f));
	}

	void PlayerScript::OnCollisionStay(Collider* other)
	{
	}

	void PlayerScript::OnCollisionExit(Collider* other)
	{
	}

	void PlayerScript::idle() 
	{
	//	if (Input::GetKey(eKeyCode::LButton))
	//	{
	//		Cat* cat = object::Instantiate<Cat>(enums::eLayerType::Animal);
	//		CatScript* catSrc = cat->AddComponent<CatScript>();

	//		catSrc->SetPlayer(GetOwner());

	//		graphics::Texture* catTex = Resources::Find<graphics::Texture>(L"Cat");
	//		Animator* catAnimator = cat->AddComponent<Animator>();
	//		catAnimator->CreateAnimation(L"DownWalk", catTex
	//			, Vector2(0.0f, 0.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
	//		catAnimator->CreateAnimation(L"RightWalk", catTex
	//			, Vector2(0.0f, 32.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
	//		catAnimator->CreateAnimation(L"UpWalk", catTex
	//			, Vector2(0.0f, 64.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
	//		catAnimator->CreateAnimation(L"LeftWalk", catTex
	//			, Vector2(0.0f, 96.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
	//		catAnimator->CreateAnimation(L"SitDown", catTex
	//			, Vector2(0.0f, 128.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
	//		catAnimator->CreateAnimation(L"Grooming", catTex
	//			, Vector2(0.0f, 160.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);
	//		catAnimator->CreateAnimation(L"LayDown", catTex
	//			, Vector2(0.0f, 192.0f), Vector2(32.0f, 32.0f), Vector2::Zero, 4, 0.1f);

	//		catAnimator->PlayAnimation(L"SitDown", false);

	//		Transform* tr = GetOwner()->GetComponent<Transform>();

	//		cat->GetComponent<Transform>()->SetPosition(tr->GetPosition() /*+ Vector2(100.0f, 0.0f)*/);
	//		cat->GetComponent<Transform>()->SetScale(Vector2(2.0f, 2.0f));


	//		Vector2 mousePos = Input::GetMousePosition();
	//		catSrc->mDest = mousePos;


	///*		mState = PlayerScript::eState::GiveWater;
	//		mAnimator->PlayAnimation(L"FrontGiveWater", false);

	//		Vector2 mousePos = Input::GetMousePosition();*/
	//	}


	//	Transform* tr = GetOwner()->GetComponent<Transform>();
	//	Vector2 pos = tr->GetPosition();

	//	Rigidbody* rb = GetOwner()->GetComponent<Rigidbody>();

	//	if (Input::GetKey(eKeyCode::D))
	//	{
	//		//pos.x += 100.0f * Time::DeltaTime();
	//		rb->AddForce(Vector2(200.0f, 0.0f));
	//	}
	//	if (Input::GetKey(eKeyCode::A))
	//	{
	//		//pos.x -= 100.0f * Time::DeltaTime();
	//		rb->AddForce(Vector2(-200.0f, 0.0f));
	//	}
	//	if (Input::GetKey(eKeyCode::W))
	//	{
	//		//pos.y -= 100.0f * Time::DeltaTime();
	//		//rb->AddForce(Vector2(0.0f, 200.0f));
	//		Vector2 velocity = rb->GetVelocity();
	//		velocity.y = -500.0f;
	//		rb->SetVelocity(velocity);
	//		rb->SetGround(false);

	//	}

	//	if (Input::GetKeyDown(eKeyCode::I))
	//	{
	//		UIManager::Push(eUIType::HpBar);
	//		//UIManager::Push(eUIType::Button);

	//	}

	//	if (Input::GetKeyDown(eKeyCode::O))
	//	{
	//		UIManager::Pop(eUIType::HpBar);

	//	}
	}

	void PlayerScript::move()
	{
		Transform* tr = GetOwner()->GetComponent<Transform>();
		Vector2 pos = tr->GetPosition();

		Rigidbody* rb = GetOwner()->GetComponent<Rigidbody>();

		if (Input::GetKey(eKeyCode::D))
		{
			//pos.x += 100.0f * Time::DeltaTime();
			rb->AddForce(Vector2(200.0f, 0.0f));
		}
		if (Input::GetKey(eKeyCode::A))
		{
			//pos.x -= 100.0f * Time::DeltaTime();
			rb->AddForce(Vector2(-200.0f, 0.0f));
		}
		if (Input::GetKey(eKeyCode::W))
		{
			//pos.y -= 100.0f * Time::DeltaTime();
			rb->AddForce(Vector2(0.0f, 200.0f));
		}
		if (Input::GetKey(eKeyCode::S))
		{
			//pos.y += 100.0f * Time::DeltaTime();
			rb->AddForce(Vector2(0.0f, -200.0f));
		}

		//tr->SetPosition(pos);

		if (Input::GetKeyUp(eKeyCode::D) || Input::GetKeyUp(eKeyCode::A) 
			|| Input::GetKeyUp(eKeyCode::W) || Input::GetKeyUp(eKeyCode::S))
		{
			mState = PlayerScript::eState::Idle;
			mAnimator->PlayAnimation(L"SitDown", false);
		}
	}
	void PlayerScript::giveWater()
	{
		if (mAnimator->IsComplete())
		{
			mState = eState::Idle;
			mAnimator->PlayAnimation(L"Idle", false);
		}
	}


}