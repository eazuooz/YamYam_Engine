#pragma once
#include "yaGameObject.h"
#include "yaGraphicDevice_DX12.h"

#include "yaEvent.h"
#include "yaWindow.h"
#include "yaApplicationEvent.h"
#include "yaMouseEvent.h"
#include "yaKeyEvent.h"


namespace ya
{
	class Application
	{
	public:
		Application();
		~Application();

		void Initialize(HWND hwnd, int width, int height);
		void InitializeWindow(HWND hwnd);
		void AdjustWindowRect(HWND hwnd, int width, int height);
		void ReszieGraphicDevice(WindowResizeEvent& e);
		void InitializeEtc();
		void OnWindowEvent(Event& e);
		void Run();
		void Close();
		void Update();
		void LateUpdate();
		void Render();
		void ExcuteCommandList();
		void Present();
		void CloseCommandList();
		void WaitForNextFrameResources();
		void EndOfFrame();
		void Release();

		Window& GetWindow() { return mWindow; }
		bool IsLoaded() const { return mbLoaded; }
		void IsLoaded(const bool load) { mbLoaded = load; }
		bool IsRunning() const { return mbRunning; }

	private:
		bool mbLoaded;
		bool mbRunning;

		//std::unique_ptr<graphics::GraphicDevice_DX11> mGraphicDevice;
		std::unique_ptr<graphics::GraphicDevice_DX12> mGraphicDevice_12;

		Window mWindow;
	};
}
