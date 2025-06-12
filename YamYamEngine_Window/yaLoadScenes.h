#pragma once
#include "..//YamYamEngine_CORE//yaSceneManager.h"

#include "yaLoadingScene.h"
#include "yaPlayScene.h"
#include "yaTitleScene.h"


namespace ya
{
	void LoadScenes()
	{
		SceneManager::CreateScene<LoadingScene>(L"LoadingScene");

		SceneManager::LoadScene(L"LoadingScene");
	}
}
