#pragma once
#pragma once
#include "..\\YamYamEngine_SOURCE\\yaResources.h"
#include "..\\YamYamEngine_SOURCE\\yaTexture.h"

namespace ya
{
	void LoadResources()
	{
		Resources::Load<graphcis::Texture>(L"BG", L"..\\Resources\\CloudOcean.png");
	}
}