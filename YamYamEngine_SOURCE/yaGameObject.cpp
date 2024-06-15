#include "yaGameObject.h"
#include "yaInput.h"
#include "yaTime.h"
#include "yaTransform.h"

namespace ya::object
{
	void Destroy(GameObject* gameObject)
	{
			if (gameObject != nullptr)
				gameObject->death();
	}
}

namespace ya
{
	GameObject::GameObject()
		: mState(eState::Active)
		, mLayerType(eLayerType::None)
	{	
		mComponents.resize((UINT)enums::eComponentType::End);
		initializeTransform();
	}

	GameObject::~GameObject()
	{
		for (const Component* comp : mComponents)
		{
			if (comp == nullptr)
				continue;

			SAFE_DELETE(comp);
		}
	}

	void GameObject::Initialize()
	{
		for (Component* comp : mComponents)
		{
			if (comp == nullptr)
				continue;

			comp->Initialize();
		}
	}


	void GameObject::Update()
	{
		for (Component* comp : mComponents)
		{
			if (comp == nullptr)
				continue;

			comp->Update();
		}
	}

	void GameObject::LateUpdate()
	{
		for (Component* comp : mComponents)
		{
			if (comp == nullptr)
				continue;

			comp->LateUpdate();
		}
	}

	void GameObject::Render()
	{
		for (Component* comp : mComponents)
		{
			if (comp == nullptr)
				continue;

			comp->Render();
		}
	}


	void GameObject::initializeTransform()
	{
		AddComponent<Transform>();
	}
}
