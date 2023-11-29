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

// ī�޶� 

// ũ�� �����ϴ°� 

// ��ũ��Ʈ

// �̹��� �ڸ��� -> �ִϸ��̼Ƕ� ����


// �ִϸ��̼� (ȸ��) -> Ÿ�ϸ� -> �浹 -> UI -> ����