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
}