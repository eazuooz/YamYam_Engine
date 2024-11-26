#pragma once
#include "yaGameObject.h"
#include "yaGraphicDevice_DX11.h"

namespace ya
{
	class Application
	{
	public:
		Application();
		~Application();

		void Initialize(HWND hwnd, int width, int height);
		void AdjustWindowRect(HWND hwnd, int width, int height);
		void ReszieGraphicDevice(int width, int height);
		void InitializeEtc();

		void Run();
		void Close();

		void Update();
		void LateUpdate();
		void Render();
		void Present();
		void Destroy();
		void Release();

		HWND GetHwnd() const { return mHwnd; }

		UINT GetWidth() const { return mWidth; }
		UINT GetHeight() const { return mHeight; }

		bool IsLoaded() const { return mbLoaded; }
		void IsLoaded(const bool load) { mbLoaded = load; }
		bool IsRunning() const { return mbRunning; }

	private:
		bool mbLoaded;
		bool mbRunning;
		HWND mHwnd;
		std::unique_ptr<graphics::GraphicDevice_DX11> mGraphicDevice;

		UINT mWidth;
		UINT mHeight;
	};
}
