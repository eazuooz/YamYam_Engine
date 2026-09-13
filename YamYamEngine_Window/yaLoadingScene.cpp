#include "yaLoadingScene.h"
#include "yaSceneManager.h"
#include "yaResources.h"
#include "yaTexture.h"
#include "yaTitleScene.h"
#include "yaPlayScene.h"
#include "yaEditorScene.h"

namespace ya
{
    void LoadingScene::Initialize() {}
    void LoadingScene::Update()
    {
        if (mLoaded) return;
        // Uploads and scene activation run on the render thread. A future async
        // loader may decode on workers, then hand results back to this thread.
        Resources::Load<graphics::Texture>(L"Player", L"..\\Resources\\CloudOcean.png");
        SceneManager::CreateScene<TitleScene>(L"TitleScene");
        SceneManager::CreateScene<PlayScene>(L"PlayScene");
        SceneManager::CreateScene<EditorScene>(L"EditorScene");
        mLoaded = true;
        SceneManager::SetActiveScene(L"LoadingScene");
        SceneManager::LoadScene(L"EditorScene");
    }
    void LoadingScene::LateUpdate() {}
    void LoadingScene::Render() {}
    void LoadingScene::OnEnter() {}
    void LoadingScene::OnExit() {}
}
