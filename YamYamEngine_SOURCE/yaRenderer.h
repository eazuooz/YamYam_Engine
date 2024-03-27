#pragma once
#include "yaCamera.h"
#include "yaGraphicDevice_DX11.h"

#include "yaMesh.h"

#include "yaConstantBuffer.h"

using namespace ya::math;
using namespace ya::graphics;

namespace ya::renderer
{
	extern Camera* mainCamera;

	extern std::vector<graphics::Vertex> vertexes;
	extern std::vector<UINT> indices;

	//extern graphics::VertexBuffer vertexBuffer;
	//extern graphics::IndexBuffer indexBuffer;
	extern Mesh* mesh;

	extern graphics::ConstantBuffer constantBuffers[(UINT)eCBType::End];

	extern ID3D11Buffer* constantBuffer;
	extern ID3D11InputLayout* inputLayouts;

	void Initialize();
	void Release();
}
