#include "yaRenderer.h"
#include "yaGraphicDevice_DX11.h"



namespace ya::renderer
{
	Camera* mainCamera = nullptr;

	Vertex vertexes[3] = {};
	std::vector<UINT> indices;

	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;

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

		
		//indices.push_back(0);
		//indices.push_back(2);
		//indices.push_back(3);

		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);


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