#pragma once
#include "yaEntity.h"

namespace ya
{
	using namespace enums;

	class GameObject;

	class Component : public Entity
	{
	public:
		Component(eComponentType type);
		virtual ~Component();

		virtual void Initialize();
		virtual void Update();
		virtual void LateUpdate();
		virtual void Render(const Matrix& view, const Matrix& projection);

		eComponentType GetType() const { return mType; }

		GameObject* GetOwner() const { return mOwner; }
		void SetOwner(GameObject* owner) { mOwner = owner; }

	private:
		GameObject* mOwner;
		eComponentType mType;
	};
}
