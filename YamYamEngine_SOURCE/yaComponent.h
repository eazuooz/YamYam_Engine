#pragma once
#include "yaEntity.h"

namespace ya
{
	class GameObject;
	class Component : public Entity
	{
	public:
		Component(enums::eComponentType type);
		~Component();

		virtual void Initialize();
		virtual void Update();
		virtual void LateUpdate();
		virtual void Render(HDC hdc);

		void SetOwner(GameObject* owner) { mOwner = owner; };
		GameObject* GetOwner() { return mOwner; }
		enums::eComponentType GetType() { return mType; }

	private:
		GameObject* mOwner;
		enums::eComponentType mType;
	};
}
