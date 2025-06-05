#pragma once
#include "yaGraphics.h"

// Sample Example Dx11
// https://github.com/kevinmoran/BeginnerDirect3D11

namespace ya::graphics
{
	class GraphicDevice_DX12
	{
	public:
		GraphicDevice_DX12();
		~GraphicDevice_DX12();
		
		bool CreateDevice();

	private:

	};

	// This is a helper to get access to a global device instance
	//	- The engine uses this, but it is not necessary to use a single global device object
	//	- This is not a lifetime managing object, just a way to globally expose a reference to an object by pointer
	template<>
	inline GraphicDevice_DX12*& GetDevice<GraphicDevice_DX12>()
	{
		static GraphicDevice_DX12* device = nullptr;
		return device;
	}
}
