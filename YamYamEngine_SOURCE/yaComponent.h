#pragma once
#include "yaEntity.h"

namespace ya
{
	using namespace enums;

	class GameObject;
	class Component : public Entity
	{
	public:
		Component(enums::eComponentType type);
		virtual ~Component();

		virtual void Initialize();
		virtual void Update();
		virtual void LateUpdate();
		virtual void Render();

		void SetOwner(GameObject* owner) { mOwner = owner; }
		GameObject* GetOwner() const { return mOwner; } 
		enums::eComponentType GetType() const { return mType; } 

	private:
		GameObject* mOwner;
		enums::eComponentType mType;
	};
}
