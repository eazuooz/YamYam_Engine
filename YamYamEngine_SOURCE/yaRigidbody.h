#pragma once
#include "yaComponent.h"

namespace ya
{
	class Rigidbody : public Component
	{
	public:
		Rigidbody();
		~Rigidbody();

		virtual void Initialize();
		virtual void Update();
		virtual void LateUpdate();
		virtual void Render();

		Vector2 GetVelocity() const { return mVelocity; }
		void SetMass(float mass) { mMass = mass; }
		void AddForce(Vector2 force) { mForce = force; }
		void SetGround(bool ground) { mbGround = ground; }
		void SetVelocity(Vector2 velocity) { mVelocity = velocity; }


	private:
		bool mbGround;
		float mMass;
		float mFriction;

		Vector2 mForce;
		Vector2 mAccelation;
		Vector2 mVelocity;
		Vector2 mLimitedVelocity;
		Vector2 mGravity;
	};
}


