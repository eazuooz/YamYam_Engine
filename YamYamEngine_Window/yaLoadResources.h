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

// ī�޶� 

// ũ�� �����ϴ°� 

// ��ũ��Ʈ

// �̹��� �ڸ��� -> �ִϸ��̼Ƕ� ����


// �ִϸ��̼� (ȸ��) -> Ÿ�ϸ� -> �浹 -> UI -> ����