#pragma once
#include "yaCamera.h"
#include "yaGraphicDevice_DX11.h"



#include "yaConstantBuffer.h"

using namespace ya::math;
using namespace ya::graphics;

namespace ya::renderer
{
	extern Camera* mainCamera;
	extern graphics::ConstantBuffer constantBuffers[(UINT)eCBType::End];


	void Initialize();
	void Release();
}
