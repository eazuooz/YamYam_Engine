#include "yaShader.h"
#include "yaRenderer.h"
#include "yaResources.h"

namespace ya::graphics
{
	bool Shader::bWireframe = false;

	Shader::Shader()
		: Resource(eResourceType::Shader)
		  , mRasterizerState(eRasterizerState::SolidNone)
		  , mBlendState(eBlendState::Opaque)
		  , mDepthStencilState(eDepthStencilState::LessEqual)
	{
	}

	Shader::~Shader()
	{
	}

	HRESULT Shader::Save(const std::wstring& path)
	{
		return E_NOTIMPL;
	}

	HRESULT Shader::Load(const std::wstring& path)
	{
        if (!Create(eShaderStage::VS, path)) return E_FAIL;
        if (!Create(eShaderStage::PS, path)) return E_FAIL;


		// To Do : you have to make pso class file
		// PSO �ϴ� ���⼭ ó��

			// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};


		auto rootSignature = GetDevice()->GetRootSignature();


		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(mVSBlob.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(mPSBlob.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;

		if (FAILED(GetDevice()->GetID3D12Device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(mPipelineState.ReleaseAndGetAddressOf()))))
            return E_FAIL;

		return S_OK;
	}

	bool Shader::Create(const eShaderStage stage, const std::wstring& fileName)
	{
        if (stage == eShaderStage::VS) return CreateVertexShader(fileName);
        if (stage == eShaderStage::PS) return CreatePixelShader(fileName);
        return false;
	}

	bool Shader::CreateVertexShader(const std::wstring& fileName)
	{
		if (!GetDevice()->CreateVertexShader(fileName, mVSBlob.GetAddressOf()))
			return false;

		return true;
	}

	bool Shader::CreatePixelShader(const std::wstring& fileName)
	{
		if (!GetDevice()->CreatePixelShader(fileName, mPSBlob.GetAddressOf()))
			return false;

		return true;
	}

	void Shader::Bind()
	{
        GetDevice()->GetCommandList()->SetPipelineState(mPipelineState.Get());
	}
}
