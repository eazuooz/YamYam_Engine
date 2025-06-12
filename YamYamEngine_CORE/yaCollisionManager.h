#pragma once
#include "CommonInclude.h"
#include "yaBoxCollider2D.h"
#include "yaCircleCollider2D.h"


namespace ya
{
	using namespace enums;

	union CollisionID
	{
		struct 
		{
			UINT32 Left;
			UINT32 Right;
		};

		UINT64 Id;
	};

	class CollisionManager
	{
	public:
		static void Initialize();
		static void Update();
		static void LateUpdate();
		static void Render();
		static void Clear();

		static void CollisionLayerCheck(eLayerType left, eLayerType right, bool enable);
		static void LayerCollision(eLayerType left, eLayerType right);
		static void ColliderCollision(Collider* left, Collider* right);
		static bool Intersect(Collider* left, Collider* right);

	private:
		//static bool isBump(Vector2 leftPos, Vector2 rigtPos, Vector2 leftSize, Vector2 rightSize);

	private:
		static std::bitset<CAST_UINT(eLayerType::Max)> mCollisionLayerMatrix[CAST_UINT(eLayerType::Max)];
		static std::unordered_map<UINT64, bool> mCollisionMap;
	};
}
