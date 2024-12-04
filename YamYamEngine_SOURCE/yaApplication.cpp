#include "yaApplication.h"
#include "yaRenderer.h"
#include "yaInput.h"
#include "yaTime.h"
#include "yaSceneManager.h"
#include "yaResources.h"
#include "yaCollisionManager.h"
#include "yaUIManager.h"
#include "yaFmod.h"

namespace ya
{
	Application::Application()
		: mHwnd(nullptr)
		, mWindowWidth(0)
		, mWindowHeight(0)
		, mWidth(0)
		, mHeight(0)
		, mX(0), mY(0)
		, mbLoaded(false)
		, mbRunning(false)

	{
	}

	Application::~Application()
	{
	}

	void Application::Initialize(HWND hwnd, int width, int height)
	{
		mHwnd = hwnd;

		AdjustWindowRect(hwnd, width, height);
		InitializeEtc();

		mGraphicDevice = std::make_unique<GraphicDevice_DX11>();
		mGraphicDevice->Initialize();
		renderer::Initialize();
		
		Fmod::Initialize();
		CollisionManager::Initialize();
		UIManager::Initialize();
		SceneManager::Initialize();

		mbRunning = true;
	}

	void Application::InitializeWindow(HWND hwnd)
	{
		SetWindowPos(hwnd, nullptr, mX, mY, mWindowWidth, mWindowHeight, 0);
		ShowWindow(hwnd, SW_SHOWDEFAULT);
	}

	void Application::AdjustWindowRect(HWND hwnd, int width, int height)
	{
		RECT rect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
		::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

		RECT winRect;
		::GetWindowRect(mHwnd, &winRect); 

		//window position
		mX = winRect.left;
        mY = winRect.top;

		// window size
		mWindowWidth = rect.right - rect.left;
		mWindowHeight = rect.bottom - rect.top;

		//client size
		mWidth = width;
		mHeight = height;

		InitializeWindow(hwnd);
	}

	void Application::ReszieGraphicDevice()
	{
		if (mGraphicDevice == nullptr)
			return;
		
		RECT winRect;
		::GetClientRect(mHwnd, &winRect);
		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = static_cast<float>(winRect.right - winRect.left);
		viewport.Height = static_cast<float>(winRect.bottom - winRect.top);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		mWidth = viewport.Width;
		mHeight = viewport.Height;

		mGraphicDevice->Resize(viewport);
		renderer::FrameBuffer->Resize(mWidth, mHeight );
	}

	void Application::InitializeEtc()
	{
		Input::Initialize();
		Time::Initialize();
	}

	void Application::Run()
	{
		if (mbLoaded == false)
			mbLoaded = true;

		Update();
		LateUpdate();
		Render();

		Destroy();
	}

	void Application::Close()
	{
		mbRunning = false;
	}

	void Application::Update()
	{
		Input::Update();
		Time::Update();

		CollisionManager::Update();
		UIManager::Update();
		SceneManager::Update();
	}

	void Application::LateUpdate()
	{
		CollisionManager::LateUpdate();
		UIManager::LateUpdate();
		SceneManager::LateUpdate();
	}

	void Application::Render()
	{
		GetDevice()->ClearRenderTargetView();
		GetDevice()->ClearDepthStencilView();
		GetDevice()->BindViewPort();
		GetDevice()->BindDefaultRenderTarget();

		Time::Render();
		CollisionManager::Render();
		UIManager::Render();
		SceneManager::Render();

		//copy back buffer
		Microsoft::WRL::ComPtr<ID3D11Texture2D> src = GetDevice()->GetFrameBuffer();
		Microsoft::WRL::ComPtr<ID3D11Texture2D> dst = renderer::FrameBuffer->GetAttachmentTexture(0)->GetTexture();

		GetDevice()->CopyResource(dst.Get(), src.Get());
	}

	void Application::Present()
	{
		GetDevice()->Present();
	}

	void Application::Destroy()
	{
		SceneManager::Destroy();
	}

	void Application::Release()
	{
		SceneManager::Release();
		UIManager::Release();
		Resources::Release();

		renderer::Release();
	}
}
