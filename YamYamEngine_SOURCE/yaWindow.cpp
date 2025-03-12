#include "yaWindow.h"
#include "yaApplicationEvent.h"

namespace ya
{
	void Window::Initialize()
	{
		
	}

	void Window::SetWindowResize(UINT width, UINT height)
	{
		mData.Width = width;
		mData.Height = height;

		WindowResizeEvent event(width, height);
	
		if (mData.EventCallback) 
			mData.EventCallback(event);
	}

	//void Window::SetWindowPos(UINT x, UINT y)
	//{
	//	mData.X = x;
	//	mData.Y = y;

	//	WindowMoveEvent event(x, y);

	//	if (mData.EventCallback)
	//		mData.EventCallback(event);
	//}
}