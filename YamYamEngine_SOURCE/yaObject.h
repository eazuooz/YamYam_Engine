#pragma once
#include "yaComponent.h"
#include "yaGameObject.h"
#include "yaLayer.h"
#include "yaScene.h"
#include "yaSceneManager.h"
#include "yaTransform.h"
#include "yaApplication.h"
#include "yaGameObjectEvent.h"

extern ya::Application application;

namespace ya::object
{
	template <typename T>
	static T* Instantiate(eLayerType type)
	{
		T* gameObject = new T();
		gameObject->SetLayerType(type);
		
		Scene* activeScene = SceneManager::GetActiveScene();
		Layer* layer = activeScene->GetLayer(type);
		layer->AddGameObject(gameObject);

		application.PushEvent(new ya::GameObjectCreatedEvent(gameObject, activeScene));

		return gameObject;
	}

	template <typename T>
	static T* Instantiate(eLayerType type, Vector3 position)
	{
		T* gameObject = new T();
		gameObject->SetLayerType(type);

		Scene* activeScene = SceneManager::GetActiveScene();
		Layer* layer = activeScene->GetLayer(type);
		layer->AddGameObject(gameObject);

		Transform* tr = gameObject->template GetComponent<Transform>();
		tr->SetPosition(position);

		application.PushEvent(new ya::GameObjectCreatedEvent(gameObject, activeScene));

		return gameObject;
	}

	static void DontDestroyOnLoad(GameObject* gameObject)
	{
		Scene* activeScene = SceneManager::GetActiveScene();
		// 현재씬에서 게임오브젝트를 지워준다.
		activeScene->EraseGameObject(gameObject);

		// 해당 게임오브젝트를 -> DontDestroy씬으로 넣어준다.
		Scene* dontDestroyOnLoad = SceneManager::GetDontDestroyOnLoad();
		dontDestroyOnLoad->AddGameObject(gameObject, gameObject->GetLayerType());
	}

	static void Destroy(GameObject* gameObject)
	{
		if (gameObject != nullptr)
			gameObject->death();

		Scene* activeScene = SceneManager::GetActiveScene();
		application.PushEvent(new ya::GameObjectDestroyedEvent(gameObject, activeScene));
	}
}
