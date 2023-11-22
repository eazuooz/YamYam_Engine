#pragma once


namespace ya::enums
{
	enum class eLayerType
	{
		None,
		BackGround,
		//Tree 
		//character
		Player,
		Max = 16,
	};


	enum class eResourceType
	{
		Texture,
		AudioClip,
		Prefab,
		End,
	};
}