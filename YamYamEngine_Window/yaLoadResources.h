#pragma once
#pragma once
#include "..\\YamYamEngine_SOURCE\\yaResources.h"
#include "..\\YamYamEngine_SOURCE\\yaTexture.h"

namespace ya
{
	void LoadResources()
	{
		Resources::Load<graphics::Texture>(L"Cat", L"..\\Resources\\ChickenAlpha.bmp");
		Resources::Load<graphics::Texture>(L"Player", L"..\\Resources\\Player.bmp");
		Resources::Load<graphics::Texture>(L"SpringFloor", L"..\\Resources\\SpringFloor.bmp");
		Resources::Load<graphics::Texture>(L"HPBAR", L"..\\Resources\\HPBAR.bmp");
		Resources::Load<graphics::Texture>(L"PixelMap", L"..\\Resources\\pixelMap.bmp");


		//ChickenAlpha
	}
}

// ī�޶� 

// ũ�� �����ϴ°� 

// ��ũ��Ʈ

// �̹��� �ڸ��� -> �ִϸ��̼Ƕ� ����


// �ִϸ��̼� (ȸ��) -> Ÿ�ϸ� -> �浹 -> UI -> ����