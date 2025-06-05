#include "yaGraphicDevice_DX12.h"

namespace ya::graphics
{
	GraphicDevice_DX12::GraphicDevice_DX12()
	{
		GetDevice<GraphicDevice_DX12>() = this;
		// Initialize the DirectX 12 device here
		// This is a placeholder, actual implementation will depend on your requirements
		 if (!(CreateDevice()))
		 	assert(NULL && "Create Device Failed!");
	}

	GraphicDevice_DX12::~GraphicDevice_DX12()
	{

	}

	bool GraphicDevice_DX12::CreateDevice()
	{

		return true;
	}
}
