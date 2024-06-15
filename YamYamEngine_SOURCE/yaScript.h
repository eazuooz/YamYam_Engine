#pragma once
#include "yaComponent.h"


namespace ya
{
	class Script : public Component
	{
	public:
		Script();
		virtual ~Script();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render() override;

		virtual void OnCollisionEnter(class Collider* other);
		virtual void OnCollisionStay(class Collider* other);
		virtual void OnCollisionExit(class Collider* other);
	};
}
