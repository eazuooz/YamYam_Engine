#pragma once
#include "..\\YamYamEngine_SOURCE\\yaSceneManager.h"

#include "yaPlayScene.h"

namespace ya
{
	void LoadScenes()
	{
		SceneManager::CreateScene<PlayScene>(L"PlayScene");
		//SceneManager::CreateScene<EndScene>(L"EndScene");
		//SceneManager::CreateScene<TitleScene>(L"TitleScene");

		SceneManager::LoadScene(L"PlayScene");
	}
}