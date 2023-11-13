#include "yaScene.h"

namespace ya
{
	Scene::Scene()
		: mGameObjects{}
	{
	}
	Scene::~Scene()
	{
	}
	void Scene::Initialize()
	{
	}
	void Scene::Update()
	{
		//for (size_t i = 0; i < mGameObjects.size(); i++)
		//{
		//	mGameObjects[i]->Update();
		//}
		
		//범위 기반 for문
		for (GameObject* gameObj : mGameObjects)
		{
			gameObj->Update();
		}
	}
	void Scene::LateUpdate()
	{
		for (GameObject* gameObj : mGameObjects)
		{
			gameObj->LateUpdate();
		}
	}
	void Scene::Render(HDC hdc)
	{
		for (GameObject* gameObj : mGameObjects)
		{
			gameObj->Render(hdc);
		}
	}

	void Scene::AddGameObject(GameObject* gameObject)
	{
		mGameObjects.push_back(gameObject);
	}
}