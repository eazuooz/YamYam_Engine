#pragma once


namespace ya::enums
{
	enum class eComponentType
	{
		Transform,
		Colider,
		Rigidbody,
		Script,
		SpriteRenderer,
		Animator,
		Camera,
		End,
	};
	
	enum class eLayerType
	{
		None,
		BackGround,
		Tile,
		//character
		Animal,
		Player,
		Floor,
		Particle,
		Max = 16,
	};

//#define (UINT)(eLayerType::Max);

	enum class eResourceType
	{
		Texture,
		AudioClip,
		Animation,
		Prefab,
		End,
	};

	enum class eColliderType
	{
		Circle2D,
		Rect2D,
		End,
	};
}
