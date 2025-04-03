#include "yaSceneManager.h"
#include "yaDontDestroyOnLoad.h"
#include "yaGameObjectEvent.h"

namespace ya
{
	std::map<std::wstring, Scene*> SceneManager::mScene = {};
	Scene* SceneManager::mActiveScene = nullptr;
	Scene* SceneManager::mDontDestroyOnLoad = nullptr;
	EventQueue SceneManager::mEventQueue;
	

	void SceneManager::Initialize()
	{
		mDontDestroyOnLoad = CreateScene<DontDestroyOnLoad>(L"DontDestroyOnLoad");
		InitializeEventHandlers();
	}

	void SceneManager::Update()
	{
		mActiveScene->Update();
		mDontDestroyOnLoad->Update();
	}

	void SceneManager::LateUpdate()
	{
		mActiveScene->LateUpdate();
		mDontDestroyOnLoad->LateUpdate();
	}

	void SceneManager::Render()
	{
		mActiveScene->Render();
		mDontDestroyOnLoad->Render();
	}

	void SceneManager::EndOfFrame()
	{
		mActiveScene->EndOfFrame();
		mDontDestroyOnLoad->EndOfFrame();

		mEventQueue.Process();
	}

	void SceneManager::Release()
	{
		for (auto& iter : mScene)
		{
			delete iter.second;
			iter.second = nullptr;
		}
	}

	bool SceneManager::SetActiveScene(const std::wstring& name)
	{
		auto iter
			= mScene.find(name);

		if (iter == mScene.end())
			return false;

		mActiveScene = iter->second;
		return true;
	}

	Scene* SceneManager::LoadScene(const std::wstring& name)
	{
		if (mActiveScene)
			mActiveScene->OnExit();

		if (!SetActiveScene(name))
			return nullptr;

		mActiveScene->OnEnter();

		return mActiveScene;
	}

	std::vector<GameObject*> SceneManager::GetGameObjects(eLayerType layer)
	{
		std::vector<GameObject*> gameObjects
			= mActiveScene->GetLayer(layer)->GetGameObjects();
		std::vector<GameObject*> dontDestroyOnLoad
			= mDontDestroyOnLoad->GetLayer(layer)->GetGameObjects();

		gameObjects.insert(gameObjects.end()
		                   , dontDestroyOnLoad.begin(), dontDestroyOnLoad.end());

		return gameObjects;
	}

	void SceneManager::InitializeEventHandlers()
	{
		// 이벤트 핸들러 등록
		mEventQueue.RegisterHandler<GameObjectCreatedEvent>([](GameObjectCreatedEvent& e) -> bool
			{
				SceneManager::GameObjectCreated(e.GetGameObject(), e.GetScene());
				return true;
			});

		mEventQueue.RegisterHandler<GameObjectDestroyedEvent>([](GameObjectDestroyedEvent& e) -> bool
			{
				SceneManager::GameObjectDestroyed(e.GetGameObject(), e.GetScene());
				return true;
			});

		// 기본 핸들러 등록
		mEventQueue.SetCallback([](Event& e)
			{
				std::cout << "[Application] Unhandled Event: " << e.ToString() << std::endl;
			});
	}

	void SceneManager::GameObjectCreated(GameObject* gameObject, Scene* scene)
	{
		scene->AddGameObject(gameObject, gameObject->GetLayerType());
	}

	void SceneManager::GameObjectDestroyed(GameObject* gameObject, Scene* scene)
	{
		scene->EraseGameObject(gameObject);
	}
}
