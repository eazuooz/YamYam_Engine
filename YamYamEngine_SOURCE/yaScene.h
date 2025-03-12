#pragma once
#include "yaEntity.h"
#include "yaGameObject.h"
#include "yaLayer.h"

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
		Layer* GetLayer(const eLayerType type) const { return mLayers[static_cast<UINT>(type)]; }

	private:
		void createLayers();

		std::vector<Layer*> mLayers;
	};
}
