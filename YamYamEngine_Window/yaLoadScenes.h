#pragma once
#include "..\\YamYamEngine_SOURCE\\yaSceneManager.h"

#include "yaLoadingScene.h"
#include "yaPlayScene.h"
#include "yaTitleScene.h"
#include "yaToolScene.h"


namespace ya
{
	void LoadScenes()
	{
		SceneManager::CreateScene<LoadingScene>(L"LoadingScene");

		SceneManager::LoadScene(L"LoadingScene");
	}
}