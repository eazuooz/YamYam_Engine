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
		Resources::Load<graphcis::Texture>(L"Chichken", L"..\\Resources\\chicken.bmp");
		Resources::Load<graphcis::Texture>(L"Cat", L"..\\Resources\\ChickenAlpha.bmp");
		Resources::Load<graphcis::Texture>(L"Bubble", L"..\\Resources\\Bubble.png");
		Resources::Load<graphcis::Texture>(L"MapleEffect", L"..\\Resources\\ezgif.com-gif-maker.png");
		//ChickenAlpha
	}
}

// 카메라 

// 크기 조정하는거 

// 스크립트

// 이미지 자르기 -> 애니메이션때 진행


// 애니메이션 (회전) -> 타일맵 -> 충돌 -> UI -> 사운드