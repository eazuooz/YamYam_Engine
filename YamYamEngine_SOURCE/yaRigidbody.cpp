#include "yaRigidbody.h"
#include "yaTime.h"
#include "yaTransform.h"
#include "yaGameObject.h"

namespace ya
{
	Rigidbody::Rigidbody()
		: Component(enums::eComponentType::Rigidbody)
		, mMass(1.0f)
		, mFriction(10.0f)
		, mForce(Vector2::Zero)
		, mVelocity(Vector2::Zero)
		, mGravity(Vector2::Zero)
		, mAccelation(Vector2::Zero)
	{

	}

	Rigidbody::~Rigidbody()
	{
	}

	void Rigidbody::Initialize()
	{
	}

	void Rigidbody::Update()
	{
		// f(힘) = m(질량) x a(가속도)
		// a = f / m;
		mAccelation = mForce / mMass;

		// 속도에 가속도를 더한다.
		mVelocity += mAccelation * Time::DeltaTime();

		if (!(mVelocity == Vector2::Zero))
		{
			//속도에 반대방향으로 마찰력 작용
			Vector2 friction = -mVelocity;
			friction = friction.normalize() * mFriction * mMass * Time::DeltaTime();

			// 마찰력으로 인한 속도 감소량이 현재 속도보다 큰 경우
			if (mVelocity.length() <= friction.length())
			{
				// 멈춰
				mVelocity = Vector2::Zero;
			}
			else
			{
				mVelocity += friction;
			}
		}

		Transform* tr = GetOwner()->GetComponent<Transform>();
		Vector2 pos = tr->GetPosition();
		pos = pos + mVelocity * Time::DeltaTime();
		tr->SetPosition(pos);

		mForce.clear();
	}

	void Rigidbody::LateUpdate()
	{
	}

	void Rigidbody::Render(HDC hdc)
	{
	}

}
