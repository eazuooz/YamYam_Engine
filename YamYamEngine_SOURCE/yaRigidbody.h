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
		virtual void Render(HDC hdc);

		void SetMass(float mass) { mMass = mass; }
		void AddForce(Vector2 force) { mForce = force; }

	private:
		float mMass;
		float mFriction;

		Vector2 mForce;
		Vector2 mAccelation;
		Vector2 mVelocity;
		Vector2 mGravity;
	};
}


