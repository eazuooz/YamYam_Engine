#pragma once
#include "CommonInclude.h"
#include "yaEvent.h"

namespace ya
{
	struct WindowProps
	{
		std::wstring Title;
		UINT Width;
		UINT Height;
		HWND Hwnd;

		WindowProps(const std::wstring& title = L"YamYam Engine",
					UINT width = 1600,
					UINT height = 900)
			: Title(title), Width(1600), Height(900), Hwnd(NULL)
		{
		}
	};

	class Window
	{
	public:
		struct WindowData
		{
			std::string Title;
			HWND Hwnd;

			//client
			unsigned int Width, Height;
			// window size
			unsigned int WindowWidth, WindowHeight;
			// window position
			unsigned int X, Y;

			bool VSync;
			EventCallbackFn EventCallback;
		};

		void Initialize();
		void SetWindowResize(UINT width, UINT height);
		void SetCursorPos(double x, double y);

		void SetEventCallBack(const EventCallbackFn& callback) { mData.EventCallback = callback; }
		
		HWND GetHwnd() { return mData.Hwnd; }
		void SetHwnd(HWND hwnd) { mData.Hwnd = hwnd; }
		UINT GetWidth() { return mData.Width; }
		void SetWidth(UINT width) { mData.Width = width; }
		UINT GetHeight() { return mData.Height; }
		void SetHeight(UINT height) { mData.Height = height; }

		UINT GetWindowWidth() { return mData.WindowWidth; }
		void SetWindowWidth(UINT width) { mData.WindowWidth = width; }
		UINT GetWindowHeight() { return mData.WindowHeight; }
		void SetWindowHeight(UINT height) { mData.WindowHeight = height; }
		
		UINT GetXPos() { return mData.X; }
		UINT GetYPos() { return mData.Y; }
		void SetPos(UINT x, UINT y) { mData.X = x; mData.Y = y; }

	private:
		WindowData mData;
	};

}