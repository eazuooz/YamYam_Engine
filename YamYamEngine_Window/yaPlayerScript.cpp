#include "yaPlayerScript.h"
#include "yaInput.h"
#include "yaTransform.h"
#include "yaTime.h"
#include "yaGameObject.h"
#include "yaAnimator.h"

namespace ya
{
	PlayerScript::PlayerScript()
		: mState(PlayerScript::eState::SitDown)
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
		if (mAnimator == nullptr)
		{
			mAnimator = GetOwner()->GetComponent<Animator>();
		}
		
		switch (mState)
		{
		case ya::PlayerScript::eState::SitDown:
			sitDown();
			break;
		case ya::PlayerScript::eState::Walk:
			move();
			break;

		case ya::PlayerScript::eState::Sleep:
			break;
		case ya::PlayerScript::eState::Attack:
			break;
		default:
			break;
		}
		
	}
	void PlayerScript::LateUpdate()
	{
	}
	void PlayerScript::Render(HDC hdc)
	{
	}

	void PlayerScript::sitDown()
	{
		

		if (Input::GetKey(eKeyCode::D))
		{
			mState = PlayerScript::eState::Walk;
			mAnimator->PlayAnimation(L"RightWalk");
		}

		if (Input::GetKey(eKeyCode::A))
			mState = PlayerScript::eState::Walk;
		if (Input::GetKey(eKeyCode::W))
			mState = PlayerScript::eState::Walk;
		if (Input::GetKey(eKeyCode::S))
			mState = PlayerScript::eState::Walk;
	}

	void PlayerScript::move()
	{
		Transform* tr = GetOwner()->GetComponent<Transform>();
		Vector2 pos = tr->GetPosition();

		if (Input::GetKey(eKeyCode::D))
		{
			pos.x += 100.0f * Time::DeltaTime();
		}
		if (Input::GetKey(eKeyCode::A))
		{
			pos.x -= 100.0f * Time::DeltaTime();
		}
		if (Input::GetKey(eKeyCode::W))
		{
			pos.y -= 100.0f * Time::DeltaTime();
		}
		if (Input::GetKey(eKeyCode::S))
		{
			pos.y += 100.0f * Time::DeltaTime();
		}

		tr->SetPosition(pos);

		if (Input::GetKeyUp(eKeyCode::D) || Input::GetKeyUp(eKeyCode::A) 
			|| Input::GetKeyUp(eKeyCode::W) || Input::GetKeyUp(eKeyCode::S))
		{
			mState = PlayerScript::eState::SitDown;
			mAnimator->PlayAnimation(L"SitDown", false);
		}
	}
}