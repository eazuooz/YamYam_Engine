#pragma once
#include "yaCamera.h"
#include "yaGraphicDevice_DX11.h"
#include "yaConstantBuffer.h"
#include "yaRenderTarget.h"

using namespace ya::math;
using namespace ya::graphics;

namespace ya::renderer
{
	extern Camera* mainCamera;
	extern GameObject* selectedObject;
	extern ConstantBuffer* constantBuffers[static_cast<UINT>(eCBType::End)];

	extern Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerStates[static_cast<UINT>(eSamplerType::End)];
	extern Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerStates[static_cast<UINT>(eRasterizerState::End)];
	extern Microsoft::WRL::ComPtr<ID3D11BlendState> blendStates[static_cast<UINT>(eBlendState::End)];
	extern Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStates[static_cast<UINT>(eDepthStencilState::End)];

	extern RenderTarget* FrameBuffer;

	void Initialize();
	void Release();
}
