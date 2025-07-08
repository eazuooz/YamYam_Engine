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
		size_t fineNameBeginOffset = path.rfind(L"\\") + 1;
		size_t fineNameEndOffset = path.length() - fineNameBeginOffset;
		const std::wstring fileName(path.substr(fineNameBeginOffset, fineNameEndOffset));

		if (!Create(eShaderStage::VS, fileName))
			return S_FALSE;
		if (!Create(eShaderStage::PS, fileName))
			return S_FALSE;


		// To Do : you have to make pso class file
		// PSO 일단 여기서 처리

			// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;

		GetDevice()->CreateGraphicsPipelineState(&psoDesc);

		return S_OK;
	}

	bool Shader::Create(const eShaderStage stage, const std::wstring& fileName)
	{
		if (stage == eShaderStage::VS)
			CreateVertexShader(fileName);
		if (stage == eShaderStage::PS)
			CreatePixelShader(fileName);

		return true;
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
		//if (bWireframe)
		//{
		//	Shader* wireframeShader = Resources::Find<Shader>(L"WireframeShader");
		//	Microsoft::WRL::ComPtr<ID3D11VertexShader> wireframeShaderVS = wireframeShader->GetVS();
		//	Microsoft::WRL::ComPtr<ID3D11PixelShader> wireframeShaderPS = wireframeShader->GetPS();
		//	Microsoft::WRL::ComPtr<ID3D11RasterizerState> wireframeRasterizerState 
		//		= renderer::rasterizerStates[static_cast<UINT>(eRasterizerState::Wireframe)];

		//	GetDevice<GraphicDevice_DX11>()->BindVS(wireframeShaderVS.Get());
		//	GetDevice<GraphicDevice_DX11>()->BindPS(wireframeShaderPS.Get());
		//	GetDevice<GraphicDevice_DX11>()->BindRasterizerState(wireframeRasterizerState.Get());
		//	GetDevice<GraphicDevice_DX11>()->BindBlendState(renderer::blendStates[static_cast<UINT>(mBlendState)].Get(), nullptr, 0xffffff);
		//	GetDevice<GraphicDevice_DX11>()->BindDepthStencilState(renderer::depthStencilStates[static_cast<UINT>(mDepthStencilState)].Get(), 0);

		//	return;
		//}

		//if (mVS)
		//	GetDevice<GraphicDevice_DX11>()->BindVS(mVS.Get());
		//if (mPS)
		//	GetDevice<GraphicDevice_DX11>()->BindPS(mPS.Get());

		//GetDevice<GraphicDevice_DX11>()->BindRasterizerState(renderer::rasterizerStates[static_cast<UINT>(mRasterizerState)].Get());
		//GetDevice<GraphicDevice_DX11>()->BindBlendState(renderer::blendStates[static_cast<UINT>(mBlendState)].Get(), nullptr, 0xffffff);
		//GetDevice<GraphicDevice_DX11>()->BindDepthStencilState(renderer::depthStencilStates[static_cast<UINT>(mDepthStencilState)].Get(), 0);
	}
}
