#pragma once
#include "yaEntity.h"
#include "yaGameObject.h"
#include "yaLayer.h"
#include "yaCamera.h"

namespace ya
{
	class Scene : public Entity
	{
	public:
		Scene();
		virtual ~Scene();

		virtual void Initialize();
		virtual void Update();
		virtual void LateUpdate();
		virtual void Render();
		virtual void EndOfFrame();

		virtual void OnEnter();
		virtual void OnExit();

		void AddGameObject(GameObject* gameObj, eLayerType type);
		void EraseGameObject(GameObject* gameObj);
		void AddCamera(Camera* camera);
		void RemoveCamera(Camera* camera);
		void CollectRenderables(std::vector<GameObject*>& opaqueList, std::vector<GameObject*>& cutoutList
			, std::vector<GameObject*>& transparentList) const;
		void SortByDistance(std::vector<GameObject*>& renderList, const Vector3& cameraPos, bool bAscending) const;
		void RenderRenderables(const std::vector<GameObject*>& renderList
			, const Matrix& view, const Matrix& projection) const;

		
		Layer* GetLayer(const eLayerType type) const { return mLayers[static_cast<UINT>(type)]; }

	private:
		void createLayers();

	private:
		std::vector<Layer*> mLayers;
		std::vector<Camera*> mCameras;
	};
}
