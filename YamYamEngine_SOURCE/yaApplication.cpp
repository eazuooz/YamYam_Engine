#pragma once
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
		: mbLoaded(false)
		, mbRunning(false)

	{
		mWindow.SetEventCallBack(YA_BIND_EVENT_FN(Application::OnEvent));
	}

	Application::~Application()
	{
	}

	void Application::Initialize(HWND hwnd, int width, int height)
	{
		mWindow.SetHwnd(hwnd);
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
		SetWindowPos(hwnd, nullptr, mWindow.GetXPos(), mWindow.GetYPos()
			, mWindow.GetWindowWidth(), mWindow.GetWindowHeight(), 0);
		ShowWindow(hwnd, SW_SHOWDEFAULT);
	}

	void Application::AdjustWindowRect(HWND hwnd, int width, int height)
	{
		RECT rect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
		::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

		RECT winRect;
		::GetWindowRect(mWindow.GetHwnd(), &winRect);

		//window position
		mWindow.SetPos(winRect.left, winRect.top);

		// window size
		mWindow.SetWindowWidth(rect.right - rect.left);
		mWindow.SetWindowHeight(rect.bottom - rect.top);

		//client size
		mWindow.SetWidth(width);
		mWindow.SetHeight(height);

		InitializeWindow(hwnd);
	}

	void Application::ReszieGraphicDevice(WindowResizeEvent& e)
	{
		if (mGraphicDevice == nullptr)
			return;
		
		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = static_cast<float>(e.GetWidth());
		viewport.Height = static_cast<float>(e.GetHeight());
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		mWindow.SetWidth(viewport.Width);
		mWindow.SetHeight(viewport.Height);

		mGraphicDevice->Resize(viewport);
		renderer::FrameBuffer->Resize(viewport.Width, viewport.Height);
	}

	

	void Application::InitializeEtc()
	{
		Input::Initialize();
		Time::Initialize();
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) -> bool
			{
				ReszieGraphicDevice(e);
				return true;
			});
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
