#pragma once
#include "..\\YamYamEngine_SOURCE\\yaSceneManager.h"

#include "yaPlayScene.h"
#include "yaTitleScene.h"

namespace ya
{
	void LoadScenes()
	{
		SceneManager::CreateScene<TitleScene>(L"TitleScene");
		SceneManager::CreateScene<PlayScene>(L"PlayScene");
		//SceneManager::CreateScene<EndScene>(L"EndScene");

		SceneManager::LoadScene(L"PlayScene");
	}
}