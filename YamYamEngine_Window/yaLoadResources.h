#pragma once
#pragma once
#include "..\\YamYamEngine_SOURCE\\yaResources.h"
#include "..\\YamYamEngine_SOURCE\\yaTexture.h"

namespace ya
{
	void LoadResources()
	{
		Resources::Load<graphcis::Texture>(L"Cat", L"..\\Resources\\ChickenAlpha.bmp");
		Resources::Load<graphcis::Texture>(L"Player", L"..\\Resources\\Player.bmp");


		//ChickenAlpha
	}
}

// 카메라 

// 크기 조정하는거 

// 스크립트

// 이미지 자르기 -> 애니메이션때 진행


// 애니메이션 (회전) -> 타일맵 -> 충돌 -> UI -> 사운드