#pragma once
#include "..//YamYamEngine_CORE//yaScript.h"
#include "yaProjectTile.h"

namespace ya
{
	class PlayerScript : public Script
	{
	public:
		enum class eState
		{
			Idle,
			Walk,
			Sleep,
			GiveWater,
			Attack,
		};

		PlayerScript();
		virtual ~PlayerScript();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render(const Matrix& view, const Matrix& projection) override;

		void AttackEffect();

		void OnCollisionEnter(Collider* other) override;
		void OnCollisionStay(Collider* other) override;
		void OnCollisionExit(Collider* other) override;

	private:
		void idle();
		void move();
		void giveWater();

		eState mState;
		class Animator* mAnimator;

		//sample
		ProjectTile* mProjTile;
	};
}
