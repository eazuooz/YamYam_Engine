#include "yaPlayerScript.h"
#include "yaInput.h"
#include "yaTransform.h"
#include "yaTime.h"
#include "yaGameObject.h"
#include "yaAnimator.h"
#include "yaCat.h"
#include "yaObject.h"
#include "yaResources.h"
#include "yaRigidbody.h"
#include "yaUIManager.h"
#include "yaSpriteRenderer.h"
#include "yaTexture.h"


namespace ya
{
	PlayerScript::PlayerScript()
		: mState(eState::Idle)
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
		static Vector3 postions = Vector3(-1, 1, 0);

		if (Input::GetKeyDown(eKeyCode::N))
		{
			mProjTile = object::Instantiate<ProjectTile>(eLayerType::Player, postions);
			SpriteRenderer* sr  = mProjTile->AddComponent<SpriteRenderer>();
			sr->SetSprite(Resources::Find<graphics::Texture>(L"Player"));

			postions.x += 1.0f;
		}

		if (Input::GetKeyDown(eKeyCode::D))
		{
			object::Destroy(mProjTile);
			mProjTile = nullptr;
		}
	}

	void PlayerScript::LateUpdate()
	{
	}

	void PlayerScript::Render(const Matrix& view, const Matrix& projection)
	{
	}

	void PlayerScript::AttackEffect()
	{

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

	}

	void PlayerScript::move()
	{
		Transform* tr = GetOwner()->GetComponent<Transform>();
		Vector3 pos = tr->GetPosition();

		Rigidbody* rb = GetOwner()->GetComponent<Rigidbody>();

		if (Input::GetKey(eKeyCode::Right))
		{
			//pos.x += 100.0f * Time::DeltaTime();
			rb->AddForce(Vector2(200.0f, 0.0f));
		}
		if (Input::GetKey(eKeyCode::Left))
		{
			//pos.x -= 100.0f * Time::DeltaTime();
			rb->AddForce(Vector2(-200.0f, 0.0f));
		}
		if (Input::GetKey(eKeyCode::Up))
		{
			//pos.y -= 100.0f * Time::DeltaTime();
			rb->AddForce(Vector2(0.0f, 200.0f));
		}
		if (Input::GetKey(eKeyCode::Down))
		{
			//pos.y += 100.0f * Time::DeltaTime();
			rb->AddForce(Vector2(0.0f, -200.0f));
		}

		//tr->SetPosition(pos);

		if (Input::GetKeyUp(eKeyCode::D) || Input::GetKeyUp(eKeyCode::A)
			|| Input::GetKeyUp(eKeyCode::W) || Input::GetKeyUp(eKeyCode::S))
		{
			mState = eState::Idle;
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
