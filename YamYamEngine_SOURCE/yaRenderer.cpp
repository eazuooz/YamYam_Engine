#include "yaRenderer.h"



namespace ya::renderer
{
	Camera* mainCamera = nullptr;

	Vertex vertexes[3] = {};
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3DBlob* vsBlob = nullptr;
	ID3D11VertexShader* vsShader = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3D11PixelShader* psShader = nullptr;
	ID3D11InputLayout* inputLayouts = nullptr;

	void LoadTriangleMesh()
	{
		renderer::vertexes[0].pos = Vector3(0.f, 0.5f, 0.0f);
		renderer::vertexes[0].color = Vector4(0.0f, 1.0f, 0.0f, 1.0f);

		renderer::vertexes[1].pos = Vector3(0.5f, -0.5f, 0.0f);
		renderer::vertexes[1].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

		renderer::vertexes[2].pos = Vector3(-0.5f, -0.5f, 0.0f);
		renderer::vertexes[2].color = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
	}

	void LoadMeshes()
	{
		LoadTriangleMesh();
	}

	void Initialize()
	{
		LoadMeshes();
	}

	void Release()
	{
		vertexBuffer->Release();
		vsBlob->Release();
		vsShader->Release();
		psBlob->Release();
		psShader->Release();
		inputLayouts->Release();
	}
}