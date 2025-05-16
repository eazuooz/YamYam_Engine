#pragma once
#include "CommonInclude.h"
#include "yaComponent.h"
#include "yaCollider.h"
//#include "yaObject.h"

namespace ya::object
{
	extern void Destroy(GameObject* gameObject);
}

namespace ya
{
	class GameObject : public Labelled
	{
	public:
		friend void object::Destroy(GameObject* obj);
		//friend Component; friend 클래스 선언

		enum class eState
		{
			Created,
			Active,
			Paused,
			Destroyed,
			End
		};

		GameObject();
		virtual ~GameObject();

		virtual void Initialize();
		virtual void Update();
		virtual void LateUpdate();
		virtual void Render(const Matrix& view, const Matrix& projection);


		template <typename T>
		T* AddComponent()
		{
			T* comp = new T();
			comp->Initialize();
			comp->SetOwner(this);

			mComponents[static_cast<UINT>(comp->GetType())] = comp;

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
		bool IsDead() const { return mState == eState::Destroyed; }
		eLayerType GetLayerType() const { return mLayerType; }
		void SetLayerType(const eLayerType layerType) { mLayerType = layerType; }

	private:
		void initializeTransform();
		void death() { mState = eState::Destroyed; }

		eState mState;
		std::vector<Component*> mComponents;
		eLayerType mLayerType;
	};
}
