#pragma once

#include "yaEntity.h"
#include "yaComponent.h"

namespace ya
{
	using namespace math;
	class Transform : public Component
	{
	public:
		Transform();
		~Transform();

		void Initialize() override;
		void Update()  override;
		void LateUpdate()  override;
		void Render()  override;

		void SetPosition(Vector2 pos) { mPosition.x = pos.x; mPosition.y = pos.y;}
		void SetRotation(float rotate) { mRotation = rotate; }
		void SetScale(Vector2 scale) { mScale = scale; }

		Vector2 GetPosition() const { return mPosition; }
		float GetRoation() const { return mRotation; }
		Vector2 GetScale() const { return mScale; }

	private:
		Vector2 mPosition;
		Vector2 mScale;
		float mRotation;
	};
}


