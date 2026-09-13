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
#include "yaTransform.h"


namespace ya
{
	Application::Application()
		: mbLoaded(false)
		, mbRunning(false)

	{
		mWindow.SetEventCallBack(YA_BIND_EVENT_FN(Application::OnWindowEvent));
	}

	Application::~Application()
	{

	}

	void Application::Initialize(HWND hwnd, int width, int height)
	{
		mWindow.SetHwnd(hwnd);
		AdjustWindowRect(hwnd, width, height);
		InitializeEtc();

		//dx11
		//mGraphicDevice = std::make_unique<GraphicDevice_DX11>();
		//mGraphicDevice->Initialize();

		//dx12
		mGraphicDevice_12 = std::make_unique<GraphicDevice_DX12>();
		mGraphicDevice_12->Initialize();

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
		RECT rect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
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
        // WM_SIZE only updates the requested window dimensions. GPU resources are
        // resized between frames in Render(), never during message dispatch.
        (void)e;
	}

	void Application::InitializeEtc()
	{
		Input::Initialize();
		Time::Initialize();
	}

	void Application::OnWindowEvent(Event& e)
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
		Render(); // EndOfFrame runs after both game and editor have submitted their draws.
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
        GetDevice()->Resize(mWindow.GetWidth(), mWindow.GetHeight());
        GetDevice()->ResetCommandAllocator();
        GetDevice()->ResetCommandList();
        renderer::BeginFrame();
        GetDevice()->SetBaseGraphicsRootSignature();
        GetDevice()->TranstionResourceBarrier(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        if (mEditorMode)
            renderer::FrameBuffer->Bind();
        else
            GetDevice()->BindFrameBuffer();

        Time::Render();
        SceneManager::Render();
        CollisionManager::Render();
        UIManager::Render();

        if (mEditorMode)
        {
            renderer::FrameBuffer->Unbind();
            GetDevice()->BindFrameBuffer(true, false);
        }
	}

	void Application::ExcuteCommandList()
	{
		GetDevice()->ExcuteCommandList();
	}

	void Application::CloseCommandList()
	{
		GetDevice()->CloseCommandList();
	}

	void Application::Present()
	{
		GetDevice()->Present();
	}

	void Application::SignalFrameCompletion()
	{
		GetDevice()->SignalFrameCompletion();
	}

	void Application::WaitforGpu()
	{
		GetDevice()->WaitForGpu();
	}

	void Application::WaitForNextFrameResources()
	{
		GetDevice()->WaitForNextFrameResources();
	}

	void Application::MoveToNextFrame()
	{
		GetDevice()->MoveToNextFrame();
	}

	void Application::EndOfFrame()
	{
		SceneManager::EndOfFrame();
	}

	void Application::Release()
	{
        WaitforGpu();
		SceneManager::Release();
		UIManager::Release();
		Resources::Release();
		Fmod::Release();
		renderer::Release();
	}
}
