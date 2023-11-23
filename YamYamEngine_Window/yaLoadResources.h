#pragma once
#pragma once
#include "..\\YamYamEngine_SOURCE\\yaResources.h"
#include "..\\YamYamEngine_SOURCE\\yaTexture.h"

namespace ya
{
	void LoadResources()
	{
		Resources::Load<graphcis::Texture>(L"Map", L"..\\Resources\\img\\map\\map_pure.png");
		Resources::Load<graphcis::Texture>(L"PackMan", L"..\\Resources\\img\\pacman\\0.png");
	}
}

// 카메라 

// 크기 조정하는거 

// 스크립트

// 이미지 자르기 -> 애니메이션때 진행


// 애니메이션 (회전) -> 타일맵 -> 충돌 -> UI -> 사운드