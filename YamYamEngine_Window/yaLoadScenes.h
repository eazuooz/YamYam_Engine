#pragma once
#include "..\\YamYamEngine_SOURCE\\yaSceneManager.h"

#include "yaPlayScene.h"
#include "yaTitleScene.h"
#include "yaToolScene.h"

namespace ya
{
	void LoadScenes()
	{


		SceneManager::CreateScene<TitleScene>(L"TitleScene");
		SceneManager::CreateScene<PlayScene>(L"PlayScene");
		
		//SceneManager::CreateScene<ToolScene>(L"ToolScene");

		SceneManager::LoadScene(L"PlayScene");
	}
}