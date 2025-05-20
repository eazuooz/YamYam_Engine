#include "yaScene.h"
#include "yaCollisionManager.h"
#include "yaMaterial.h"
#include "yaSceneManager.h"
#include "yaTransform.h"
#include "yaRenderer.h"
#include "yaSpriteRenderer.h"

namespace ya
{
	Scene::Scene()
		: mLayers{}
	{
		createLayers();
	}

	Scene::~Scene()
	{
		for (Layer* layer : mLayers)
		{
			delete layer;
			layer = nullptr;
		}
	}

	void Scene::Initialize()
	{
		const std::wstring& sceneName = GetName();
		SceneManager::SetActiveScene(sceneName);
	}

	void Scene::Update()
	{
		for (Layer* layer : mLayers)
		{
			if (layer == nullptr)
				continue;

			layer->Update();
		}
	}

	void Scene::LateUpdate()
	{
		for (Layer* layer : mLayers)
		{
			if (layer == nullptr)
				continue;

			layer->LateUpdate();
		}
	}

	void Scene::Render()
	{
		for (Camera* camera : mCameras)
		{
			if (camera == nullptr)
				continue;

			Matrix viewMatrix = camera->GetViewMatrix();
			Matrix projectionMatrix = camera->GetProjectionMatrix();
			Vector3 cameraPos = camera->GetOwner()->GetComponent<Transform>()->GetPosition();

			std::vector<GameObject*> opaqueList = {};
			std::vector<GameObject*> cutoutList = {};
			std::vector<GameObject*> transparentList = {};

			// collect randerables(game objects)
			renderer::CollectRenderables(this, opaqueList, cutoutList, transparentList);

			// soring renderables by distance (between camera and game object)
			renderer::SortByDistance(opaqueList, cameraPos, true);
			renderer::SortByDistance(cutoutList, cameraPos, true);
			renderer::SortByDistance(transparentList, cameraPos, false);

			// render game objects
			renderer::RenderRenderables(opaqueList, viewMatrix, projectionMatrix);
			renderer::RenderRenderables(cutoutList, viewMatrix, projectionMatrix);
			renderer::RenderRenderables(transparentList, viewMatrix, projectionMatrix);
		}
	}

	void Scene::EndOfFrame()
	{
		for (Layer* layer : mLayers)
		{
			if (layer == nullptr)
				continue;

			layer->EndOfFrame();
		}
	}

	void Scene::AddGameObject(GameObject* gameObj, const eLayerType type)
	{
		mLayers[static_cast<UINT>(type)]->AddGameObject(gameObj);
	}

	void Scene::EraseGameObject(GameObject* gameObj)
	{
		if (gameObj == nullptr)
			return;

		eLayerType layerType = gameObj->GetLayerType();
		mLayers[static_cast<UINT>(layerType)]->EraseGameObject(gameObj);
	}

	void Scene::AddCamera(Camera* camera)
	{
		if (camera == nullptr)
			return;

		mCameras.push_back(camera);
	}

	void Scene::RemoveCamera(Camera* camera)
	{
		if (camera == nullptr)
			return;

		auto iter
			= std::find(mCameras.begin(), mCameras.end(), camera);

		if (iter != mCameras.end())
			mCameras.erase(iter);
	}

	//void Scene::CollectRenderables(std::vector<GameObject*>& opaqueList, std::vector<GameObject*>& cutoutList
	//	, std::vector<GameObject*>& transparentList) const
	//{
	//	for (Layer* layer : mLayers)
	//	{
	//		if (layer == nullptr)
	//			continue;

	//		std::vector<GameObject*>& gameObjects = layer->GetGameObjects();

	//		for (GameObject* gameObj : gameObjects)
	//		{
	//			if (gameObj == nullptr)
	//				continue;
	//			// to do : renderer 상속구조 만들기
	//			BaseRenderer* renderer = gameObj->GetComponent<BaseRenderer>();
	//			if (renderer == nullptr)
	//				continue;

	//			switch (renderer->GetMaterial()->GetRenderingMode())
	//			{
	//			case graphics::eRenderingMode::Opaque:
	//				opaqueList.push_back(gameObj);
	//				break;

	//			case graphics::eRenderingMode::CutOut:
	//				cutoutList.push_back(gameObj);
	//				break;

	//			case graphics::eRenderingMode::Transparent:
	//				transparentList.push_back(gameObj);
	//				break;
	//			}
	//		}
	//	}
	//}

	//void Scene::SortByDistance(std::vector<GameObject*>& renderList, const Vector3& cameraPos, bool bAscending) const
	//{
	//	// opaqueList and cutoutList are sorted in ascending order
	//	// trasparentList is sorted in descending order
	//	auto comparator = [cameraPos, bAscending](GameObject* a, GameObject* b)
	//		{
	//			float distA = Vector3::Distance(a->GetComponent<Transform>()->GetPosition(), cameraPos);
	//			float distB = Vector3::Distance(b->GetComponent<Transform>()->GetPosition(), cameraPos);
	//			return bAscending ? (distA < distB) : (distA > distB);
	//		};

	//	std::ranges::sort(renderList, comparator);
	//}

	//void Scene::RenderRenderables(const std::vector<GameObject*>& renderList, const Matrix& view,
	//	const Matrix& projection) const
	//{
	//	for (auto* obj : renderList)
	//	{
	//		if (obj == nullptr)
	//			continue;

	//		obj->Render(view, projection);
	//	}
	//}

	void Scene::createLayers()
	{
		mLayers.resize(static_cast<UINT>(enums::eLayerType::Max));
		for (size_t i = 0; i < static_cast<UINT>(enums::eLayerType::Max); i++)
		{
			mLayers[i] = new Layer();
		}
	}

	void Scene::OnEnter()
	{
	}

	void Scene::OnExit()
	{
		CollisionManager::Clear();
	}
}
