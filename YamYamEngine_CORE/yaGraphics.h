#pragma once
#include "CommonInclude.h"

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

// d3dx12는 d3d12 이후 포함
//#include <include/d3dx12/d3dx12.h>
#include <include\d3dx12\d3dx12.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define CB_GETBINDSLOT(name) __CBUFFERBINDSLOT__##name##__
#define CBUFFER(name, slot) static const int CB_GETBINDSLOT(name) = slot; struct alignas(16) name 

#define CBSLOT_TRANSFORM		0

//#define DX11_ENABLE 
#define DX12_ENABLE

namespace ya::graphics
{
	struct Vertex
	{
		math::Vector3 pos;
		math::Vector4 color;
		math::Vector2 uv; //texture coordinate
	};

	enum class eShaderStage
	{
		VS,		// Vertex Shader
		HS,		// Hull Shader
		DS,		// Domain Shader
		GS,		// Geometry Shader
		PS,		// Pixel Shader
		CS,		// Compute Shader
		All,
		End,
	};

	enum class eCBType
	{
		Transform,
		None,
		End,
	};

	enum class eSamplerType
	{
		Point,
		Linear,
		Anisotropic,
		PostProcess,
		End,
	};

	enum class eRenderingMode
	{
		Opaque,
		CutOut,
		Transparent,
		//PostProcess,
		End,
	};

	enum class eTextureType
	{
		Albedo,
		Normal,
		Specular,
		Smoothness,
		Metallic,
		Sprite,
		End,
	};

	enum class eRasterizerState
	{
		SolidBack,
		SolidFront,
		SolidNone,
		Wireframe,
		End,
	};

	enum class eBlendState
	{
		Opaque,
		Cutout,
		Transparent,
		OneOne,
		End,
	};

	enum class eDepthStencilState
	{
		DepthNone,
		LessEqual,
		Always,
		End,
	};

	struct GpuBuffer
	{
		//Microsoft::WRL::ComPtr<ID3D11Buffer> buffer = nullptr;
		//D3D11_BUFFER_DESC desc = {};

		GpuBuffer() = default;
		virtual ~GpuBuffer() = default;
	};

	//Constant Buffer
	CBUFFER(TransformCB, CBSLOT_TRANSFORM)
	{
		math::Matrix World;
		math::Matrix View;
		math::Matrix Projection;
	};

	template<typename T>
	inline T*& GetDevice();
}