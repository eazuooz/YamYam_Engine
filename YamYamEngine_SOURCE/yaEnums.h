#pragma once


namespace ya::enums
{
	enum class eComponentType
	{
		Transform,
		Collider,
		Rigidbody,
		Script,
		SpriteRenderer,
		Animator,
		Camera,
		AudioListener,
		AudioSource,
		End,
	};
	
	enum class eLayerType
	{
		None,
		BackGround,
		Player,
		Floor,
		Particle,
		Max = 16,
	};

//#define (UINT)(eLayerType::Max);

	enum class eResourceType
	{
		Texture,
		Mesh,
		Material,
		Animation,
		AudioClip,
		Shader,
		Prefab,
		End,
	};

	enum class eColliderType
	{
		Circle2D,
		Rect2D,
		End,
	};

	enum class eUIType
	{
		HpBar,	
		Button,
		End,
	};
}
