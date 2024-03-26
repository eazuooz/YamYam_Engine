#pragma once
#include "CommonInclude.h"
#include "yaComponent.h"
#include "yaCollider.h"
//#include "yaObject.h"

namespace ya::object
{
	void Destory(GameObject* gameObject);
}

namespace ya
{
	class GameObject : public Entity
	{
	public:
		friend void object::Destory(GameObject* obj);
		//friend Component; friend 클래스 선언

		enum class eState
		{
			Active,
			Paused,
			Dead,
			End
		};

		GameObject();
		~GameObject();

		virtual void Initialize();
		virtual void Update();
		virtual void LateUpdate();
		virtual void Render();



		template <typename T>
		T* AddComponent()
		{
			T* comp = new T();
			comp->Initialize();
			comp->SetOwner(this);

			mComponents[(UINT)comp->GetType()] = comp;

			return comp;
		}
		template <typename T>
		T* GetComponent()
		{
			T* component = nullptr;
			for (Component* comp : mComponents)
			{
				component = dynamic_cast<T*>(comp);
				if (component)
					break;
			}

			return component;
		}

		eState GetState() const { return mState; }
		void SetActive(bool power)
		{
			if (power == true) mState = eState::Active; 
			if (power == false) mState = eState::Paused;
		}
		bool IsActive() const { return mState == eState::Active; }
		bool IsDead() const { return mState == eState::Dead; }
		void SetLayerType(eLayerType layerType) { mLayerType = layerType; }
		eLayerType GetLayerType() const { return mLayerType; }

	private:
		void initializeTransform();
		void death() { mState = eState::Dead; }

	private:
		eState mState;
		std::vector<Component*> mComponents;
		eLayerType mLayerType;
	};
}
