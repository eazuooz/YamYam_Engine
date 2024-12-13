#include "yaRenderer.h"
#include "yaGraphicDevice_DX11.h"

#include "yaResources.h"
#include "yaMesh.h"
#include "yaShader.h"
#include "yaMaterial.h"
#include "yaRenderTarget.h"
#include "yaApplication.h"


extern ya::Application application;

namespace ya::renderer
{
	Camera* mainCamera = nullptr;
	GameObject* selectedObject = nullptr;
	ConstantBuffer* constantBuffers[static_cast<UINT>(eCBType::End)] = {};
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerStates[static_cast<UINT>(eSamplerType::End)] = {};
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerStates[static_cast<UINT>(eRasterizerState::End)] = {};
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendStates[static_cast<UINT>(eBlendState::End)] = {};
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStates[static_cast<UINT>(eDepthStencilState::End)] = {};

	RenderTarget* FrameBuffer = nullptr;

	void LoadStates()
	{
#pragma region sampler state
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		GetDevice()->CreateSamplerState(&samplerDesc,
		                                samplerStates[static_cast<UINT>(eSamplerType::Anisotropic)].GetAddressOf());

		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		GetDevice()->CreateSamplerState(&samplerDesc,
		                                samplerStates[static_cast<UINT>(eSamplerType::Point)].GetAddressOf());

		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		GetDevice()->CreateSamplerState(&samplerDesc,
		                                samplerStates[static_cast<UINT>(eSamplerType::Linear)].GetAddressOf());

		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		GetDevice()->CreateSamplerState(&samplerDesc,
		                                samplerStates[static_cast<UINT>(eSamplerType::PostProcess)].GetAddressOf());

		GetDevice()->BindSamplers(static_cast<UINT>(eSamplerType::Point), 1,
		                          samplerStates[static_cast<UINT>(eSamplerType::Point)].GetAddressOf());
		GetDevice()->BindSamplers(static_cast<UINT>(eSamplerType::Linear), 1,
		                          samplerStates[static_cast<UINT>(eSamplerType::Linear)].GetAddressOf());
		GetDevice()->BindSamplers(static_cast<UINT>(eSamplerType::Anisotropic), 1,
		                          samplerStates[static_cast<UINT>(eSamplerType::Anisotropic)].GetAddressOf());
		GetDevice()->BindSamplers(static_cast<UINT>(eSamplerType::PostProcess), 1,
		                          samplerStates[static_cast<UINT>(eSamplerType::PostProcess)].GetAddressOf());
#pragma endregion
#pragma region rasterize state
		D3D11_RASTERIZER_DESC rsDesc = {};
		rsDesc.AntialiasedLineEnable = false;
		rsDesc.CullMode = D3D11_CULL_BACK;
		rsDesc.DepthBias = 0;
		rsDesc.DepthBiasClamp = 0.0f;
		rsDesc.DepthClipEnable = true;
		rsDesc.FillMode = D3D11_FILL_SOLID;
		rsDesc.FrontCounterClockwise = false;
		rsDesc.MultisampleEnable = false;
		rsDesc.ScissorEnable = false;
		rsDesc.SlopeScaledDepthBias = 0.0f;
		GetDevice()->CreateRasterizerState(
			&rsDesc, rasterizerStates[static_cast<UINT>(eRasterizerState::SolidBack)].GetAddressOf());

		rsDesc.FillMode = D3D11_FILL_SOLID;
		rsDesc.CullMode = D3D11_CULL_FRONT;
		GetDevice()->CreateRasterizerState(
			&rsDesc, rasterizerStates[static_cast<UINT>(eRasterizerState::SolidFront)].GetAddressOf());

		rsDesc.FillMode = D3D11_FILL_SOLID;
		rsDesc.CullMode = D3D11_CULL_NONE;
		GetDevice()->CreateRasterizerState(
			&rsDesc, rasterizerStates[static_cast<UINT>(eRasterizerState::SolidNone)].GetAddressOf());

		rsDesc.FillMode = D3D11_FILL_WIREFRAME;
		rsDesc.CullMode = D3D11_CULL_NONE;
		GetDevice()->CreateRasterizerState(
			&rsDesc, rasterizerStates[static_cast<UINT>(eRasterizerState::Wireframe)].GetAddressOf());
#pragma endregion
#pragma region blend state
		D3D11_BLEND_DESC bsDesc = {};
		bsDesc.AlphaToCoverageEnable = false;
		bsDesc.IndependentBlendEnable = false;
		bsDesc.RenderTarget[0].BlendEnable = true;
		bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		bsDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		bsDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		bsDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		GetDevice()->CreateBlendState(&bsDesc, blendStates[static_cast<UINT>(eBlendState::AlphaBlend)].GetAddressOf());

		bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		GetDevice()->CreateBlendState(&bsDesc, blendStates[static_cast<UINT>(eBlendState::OneOne)].GetAddressOf());
#pragma endregion
#pragma region depthstencil state
		D3D11_DEPTH_STENCIL_DESC dsDesc = {};
		dsDesc.DepthEnable = true;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		dsDesc.StencilEnable = false;
		GetDevice()->CreateDepthStencilState(
			&dsDesc, depthStencilStates[static_cast<UINT>(eDepthStencilState::LessEqual)].GetAddressOf());

		dsDesc.DepthEnable = false;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		dsDesc.StencilEnable = false;
		GetDevice()->CreateDepthStencilState(
			&dsDesc, depthStencilStates[static_cast<UINT>(eDepthStencilState::DepthNone)].GetAddressOf());
#pragma endregion
	}

	void LoadTriangleMesh()
	{
		auto mesh = new Mesh();

		std::vector<Vertex> vertexes = {};
		std::vector<UINT> indices = {};

		vertexes.resize(3);
		vertexes[0].pos = Vector3(0.0f, 0.5f, 0.0f);
		vertexes[0].color = Vector4(0.0f, 1.0f, 0.0f, 1.0f);

		vertexes[1].pos = Vector3(0.5f, -0.5f, 0.0f);
		vertexes[1].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

		vertexes[2].pos = Vector3(-0.5f, -0.5f, 0.0f);
		vertexes[2].color = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);

		D3D11_INPUT_ELEMENT_DESC inputLayoutDesces[2] = {};
		inputLayoutDesces[0].AlignedByteOffset = 0;
		inputLayoutDesces[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputLayoutDesces[0].InputSlot = 0;
		inputLayoutDesces[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesces[0].SemanticName = "POSITION";
		inputLayoutDesces[0].SemanticIndex = 0;

		inputLayoutDesces[1].AlignedByteOffset = 12;
		inputLayoutDesces[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		inputLayoutDesces[1].InputSlot = 0;
		inputLayoutDesces[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesces[1].SemanticName = "COLOR";
		inputLayoutDesces[1].SemanticIndex = 0;

		Shader* triangleShader = Resources::Find<Shader>(L"TriangleShader");
		mesh->SetVertexBufferParams(2, inputLayoutDesces, triangleShader->GetVSBlob()->GetBufferPointer(),
		                            triangleShader->GetVSBlob()->GetBufferSize());

		mesh->CreateVB(vertexes);
		mesh->CreateIB(indices);

		Resources::Insert(L"TriangleMesh", mesh);
	}

	void LoadRectMesh()
	{
		auto mesh = new Mesh();

		std::vector<Vertex> vertexes = {};
		std::vector<UINT> indices = {};

		vertexes.resize(4);
		vertexes[0].pos = Vector3(-0.5f, 0.5f, 0.0f);
		vertexes[0].color = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
		vertexes[0].uv = Vector2(0.0f, 0.0f);

		vertexes[1].pos = Vector3(0.5f, 0.5f, 0.0f);
		vertexes[1].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		vertexes[1].uv = Vector2(1.0f, 0.0f);

		vertexes[2].pos = Vector3(0.5f, -0.5f, 0.0f);
		vertexes[2].color = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
		vertexes[2].uv = Vector2(1.0f, 1.0f);

		vertexes[3].pos = Vector3(-0.5f, -0.5f, 0.0f);
		vertexes[3].color = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
		vertexes[3].uv = Vector2(0.0f, 1.0f);

		indices.push_back(0);
		indices.push_back(2);
		indices.push_back(3);

		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);

		D3D11_INPUT_ELEMENT_DESC inputLayoutDesces[3] = {};
		inputLayoutDesces[0].AlignedByteOffset = 0;
		inputLayoutDesces[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputLayoutDesces[0].InputSlot = 0;
		inputLayoutDesces[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesces[0].SemanticName = "POSITION";
		inputLayoutDesces[0].SemanticIndex = 0;

		inputLayoutDesces[1].AlignedByteOffset = 12;
		inputLayoutDesces[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		inputLayoutDesces[1].InputSlot = 0;
		inputLayoutDesces[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesces[1].SemanticName = "COLOR";
		inputLayoutDesces[1].SemanticIndex = 0;

		inputLayoutDesces[2].AlignedByteOffset = 28;
		inputLayoutDesces[2].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputLayoutDesces[2].InputSlot = 0;
		inputLayoutDesces[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesces[2].SemanticName = "TEXCOORD";
		inputLayoutDesces[2].SemanticIndex = 0;

		Shader* spriteShader = Resources::Find<Shader>(L"SpriteDefaultShader");
		mesh->SetVertexBufferParams(3, inputLayoutDesces, spriteShader->GetVSBlob()->GetBufferPointer(),
		                            spriteShader->GetVSBlob()->GetBufferSize());

		mesh->CreateVB(vertexes);
		mesh->CreateIB(indices);

		Resources::Insert(L"RectMesh", mesh);
	}

	void LoadMeshes()
	{
		LoadTriangleMesh();
		LoadRectMesh();
	}

	void LoadShaders()
	{
		Resources::Load<Shader>(L"TriangleShader", L"..\\Shaders_SOURCE\\Triangle");
		Resources::Load<Shader>(L"SpriteDefaultShader", L"..\\Shaders_SOURCE\\SpriteDefault");
		Resources::Load<Shader>(L"WireframeShader", L"..\\Shaders_SOURCE\\Wireframe");

	}

	void LoadMaterials()
	{
		auto triangleMaterial = new Material();
		triangleMaterial->SetShader(Resources::Find<Shader>(L"TriangleShader"));
		Resources::Insert(L"TriangleMaterial", triangleMaterial);

		auto spriteMaterial = new Material();
		spriteMaterial->SetShader(Resources::Find<Shader>(L"SpriteDefaultShader"));
		Resources::Insert(L"Sprite-Default-Material", spriteMaterial);
	}

	void LoadConstantBuffers()
	{
		constantBuffers[CBSLOT_TRANSFORM] = new ConstantBuffer(eCBType::Transform);
		constantBuffers[CBSLOT_TRANSFORM]->Create(sizeof(TransformCB));
	}

	void LoadFrameBuffer()
	{
		RenderTargetSpecification spec;
		spec.Attachments = { eRenderTragetFormat::RGBA8, eRenderTragetFormat::Depth };
		spec.Width = application.GetWindow().GetWidth();
		spec.Height = application.GetWindow().GetHeight();

		FrameBuffer = RenderTarget::Create(spec);
	}

	void Initialize()
	{
		LoadStates();
		LoadShaders();
		LoadMeshes();
		LoadMaterials();
		LoadConstantBuffers();
		LoadFrameBuffer();
	}

	void Release()
	{
		delete FrameBuffer;
		FrameBuffer = nullptr;

		for (UINT i = 0; i < static_cast<UINT>(eCBType::End); i++)
		{
			delete constantBuffers[i];
			constantBuffers[i] = nullptr;
		}
	}
}
