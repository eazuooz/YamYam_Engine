#include "yaLoadingScene.h"
#include "yaRenderer.h"
#include "yaSceneManager.h"
#include "yaResources.h"
#include "yaTexture.h"

namespace ya
{
	LoadingScene::LoadingScene()
		: mbLoadCompleted(false)
		, mMutualExclusion()
		, mResourcesLoadThread()
	{

	}

	LoadingScene::~LoadingScene()
	{
		delete mResourcesLoadThread;
		mResourcesLoadThread = nullptr;
	}

	void LoadingScene::Initialize()
	{
		mResourcesLoadThread = new std::thread(&LoadingScene::resourcesLoad, this, std::ref(mMutualExclusion));
		
	}

	void LoadingScene::Update()
	{
		if (mbLoadCompleted)
		{
			//���� ���ξ����尡 ����Ǵµ� �ڽľ����尡 �����ִٸ�
			//�ڽľ����带 ���ξ����忡 ���Խ��� ���ξ����尡 ����Ǳ������� block
			mResourcesLoadThread->join();

			//���ξ������ ���� �и� ���� �������� ������ �����
			//mResourcesLoadThread->detach();

			SceneManager::LoadScene(L"PlayScene");
		}
	}

	void LoadingScene::LateUpdate()
	{

	}

	void LoadingScene::Render(HDC hdc)
	{

	}

	void LoadingScene::OnEnter()
	{

	}

	void LoadingScene::OnExit()
	{

	}
	void LoadingScene::resourcesLoad(std::mutex& m)
	{
		m.lock();
		{
			Resources::Load<graphics::Texture>(L"Cat", L"..\\Resources\\ChickenAlpha.bmp");
			Resources::Load<graphics::Texture>(L"Player", L"..\\Resources\\Player.bmp");
			Resources::Load<graphics::Texture>(L"SpringFloor", L"..\\Resources\\SpringFloor.bmp");
			Resources::Load<graphics::Texture>(L"HPBAR", L"..\\Resources\\HPBAR.bmp");
			Resources::Load<graphics::Texture>(L"PixelMap", L"..\\Resources\\pixelMap.bmp");
		}
		m.unlock();

		// complete
		mbLoadCompleted = true;
		
	}
}
