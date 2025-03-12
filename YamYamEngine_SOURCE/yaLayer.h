#pragma once
#include "yaEntity.h"
#include "CommonInclude.h"
#include "yaGameObject.h"

namespace ya
{
	class Layer : public Entity
	{
	public:
		Layer();
		virtual ~Layer();

		virtual void Initialize();
		virtual void Update();
		virtual void LateUpdate();
		virtual void Render();
		virtual void EndOfFrame();

		void AddGameObject(GameObject* gameObject);
		void EraseGameObject(GameObject* eraseGameObj);
		std::vector<GameObject*>& GetGameObjects() { return mGameObjects; }

	private:
		void findDeadGameObjects(OUT std::vector<GameObject*>& gameObjects);
		void deleteGameObjects(std::vector<GameObject*> gameObjects);
		void eraseDeadGameObject();

		std::vector<GameObject*> mGameObjects;
	};

	using GameObjectIter = std::vector<GameObject*>::iterator;
}
