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

		void Initialize(HWND hwnd, UINT width, UINT height);
		void AdjustWindowRect(HWND hwnd, UINT width, UINT height);
		void InitializeEtc();

		void Run();

		void Update();
		void LateUpdate();
		void Render();
		void Destroy();
		void Release();

		[[discard]] HWND GetHwnd() const { return mHwnd; }

		[[discard]] UINT GetWidth() const { return mWidth; }
		[[discard]] UINT GetHeight() const { return mHeight; }

		[[discard]] bool IsLoaded() const { return mbLoaded; }
		[[noreturn]] void IsLoaded(bool load) { mbLoaded = load; }

	private:
		bool mbLoaded;
		HWND mHwnd;
		std::unique_ptr<graphics::GraphicDevice_DX11> mGraphicDevice;

		UINT mWidth;
		UINT mHeight;
	};
}
