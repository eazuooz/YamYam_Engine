#include "yaRigidbody.h"
#include "yaTime.h"
#include "yaTransform.h"
#include "yaGameObject.h"

namespace ya
{
	Rigidbody::Rigidbody()
		: Component(eComponentType::Rigidbody)
		  , mbGround(false)
		  , mMass(1.0f)
		  , mFriction(10.0f)
		  , mForce(Vector2::Zero)
		  , mAccelation(Vector2::Zero)
		  , mVelocity(Vector2::Zero)
		  , mLimitedVelocity(Vector2(200.0f, 1000.0f))
		  , mGravity(Vector2(0.0f, 800.0f))
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
		//// f(힘) = m(질량) x a(가속도)
		//// a = f / m;
		//mAccelation = mForce / mMass;

		//// 속도에 가속도를 더한다.
		//mVelocity += mAccelation * Time::DeltaTime();

		//if (mbGround)
		//{
		//	// 땅위에 있을때
		//	Vector2 gravity = mGravity;
		//	gravity.Normalize();

		//	float dot = mVelocity.Dot(gravity);
		//	mVelocity -= gravity * dot;
		//}
		//else
		//{
		//	// 공중에 있을떄
		//	mVelocity += mGravity * Time::DeltaTime();
		//}


		////최대 속도 제한
		//Vector2 gravity = mGravity;
		//gravity.Normalize();
		//float dot = mVelocity.Dot(gravity); 
		//gravity = gravity * dot;

		//Vector2 sideVelocity = mVelocity - gravity;
		//if (mLimitedVelocity.y < gravity.Length())
		//{
		//	gravity.Normalize();
		//	gravity *= mLimitedVelocity.y;
		//}

		//if (mLimitedVelocity.x < sideVelocity.Length())
		//{
		//	sideVelocity.Normalize();
		//	sideVelocity *= mLimitedVelocity.x;
		//}
		//mVelocity = gravity + sideVelocity;


		//if (!(mVelocity == Vector2::Zero))
		//{
		//	//속도에 반대방향으로 마찰력 작용
		//	Vector2 friction = -mVelocity;
		//	friction.Normalize();
		//	friction = friction * mFriction * mMass * Time::DeltaTime();

		//	// 마찰력으로 인한 속도 감소량이 현재 속도보다 큰 경우
		//	if (mVelocity.Length() <= friction.Length())
		//	{
		//		// 멈춰
		//		mVelocity = Vector2::Zero;
		//	}
		//	else
		//	{
		//		mVelocity += friction;
		//	}
		//}

		//Transform* tr = GetOwner()->GetComponent<Transform>();
		//Vector2 pos = tr->GetPosition();
		//pos = pos + mVelocity * Time::DeltaTime();
		//tr->SetPosition(pos);

		//mForce = Vector2::One;
	}

	void Rigidbody::LateUpdate()
	{
	}

	void Rigidbody::Render()
	{
	}
}
