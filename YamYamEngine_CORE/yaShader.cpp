#include "yaShader.h"
#include "yaRenderer.h"
#include <stdexcept>

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
        if (!Create(eShaderStage::VS, path) || !Create(eShaderStage::PS, path)) return E_FAIL;
        // Prepare the material variants while loading. Custom legacy state
        // combinations are cached on first bind and never replace an in-flight PSO.
        if (!GetPipelineState(eRasterizerState::SolidNone, eBlendState::Opaque, eDepthStencilState::LessEqual)
            || !GetPipelineState(eRasterizerState::SolidNone, eBlendState::Cutout, eDepthStencilState::LessEqual)
            || !GetPipelineState(eRasterizerState::SolidNone, eBlendState::Transparent, eDepthStencilState::Always))
            return E_FAIL;
        return S_OK;
    }

    ID3D12PipelineState* Shader::GetPipelineState(eRasterizerState rasterizer, eBlendState blend, eDepthStencilState depth)
    {
        if (rasterizer < eRasterizerState::SolidBack || rasterizer >= eRasterizerState::End
            || blend < eBlendState::Opaque || blend >= eBlendState::End
            || depth < eDepthStencilState::DepthNone || depth >= eDepthStencilState::End)
            throw std::invalid_argument("Invalid graphics pipeline state");
        const PipelineKey key(rasterizer, blend, depth);
        const auto found = mPipelineStates.find(key);
        if (found != mPipelineStates.end()) return found->second.Get();
        if (!mVSBlob || !mPSBlob || !mCutoutPSBlob) return nullptr;

        const D3D12_INPUT_ELEMENT_DESC inputElements[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.InputLayout = { inputElements, _countof(inputElements) };
        desc.pRootSignature = GetDevice()->GetRootSignature().Get();
        desc.VS = CD3DX12_SHADER_BYTECODE(mVSBlob.Get());
        desc.PS = CD3DX12_SHADER_BYTECODE(blend == eBlendState::Cutout ? mCutoutPSBlob.Get() : mPSBlob.Get());
        desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        desc.RasterizerState.CullMode = rasterizer == eRasterizerState::SolidBack ? D3D12_CULL_MODE_BACK
            : rasterizer == eRasterizerState::SolidFront ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_NONE;
        desc.RasterizerState.FillMode = rasterizer == eRasterizerState::Wireframe ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;

        desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        auto& target = desc.BlendState.RenderTarget[0];
        if (blend == eBlendState::Transparent || blend == eBlendState::OneOne)
        {
            target.BlendEnable = TRUE;
            target.SrcBlend = blend == eBlendState::Transparent ? D3D12_BLEND_SRC_ALPHA : D3D12_BLEND_ONE;
            target.DestBlend = blend == eBlendState::Transparent ? D3D12_BLEND_INV_SRC_ALPHA : D3D12_BLEND_ONE;
            target.BlendOp = D3D12_BLEND_OP_ADD;
            // Preserve the DX11 alpha-channel equation as well as its RGB blend.
            target.SrcBlendAlpha = D3D12_BLEND_ONE;
            target.DestBlendAlpha = D3D12_BLEND_ZERO;
            target.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        }
        desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        desc.DepthStencilState.DepthEnable = depth != eDepthStencilState::DepthNone;
        desc.DepthStencilState.DepthWriteMask = depth == eDepthStencilState::LessEqual
            ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        // Transparent deliberately keeps the old Always/no-write behavior.
        desc.DepthStencilState.DepthFunc = depth == eDepthStencilState::LessEqual ? D3D12_COMPARISON_FUNC_LESS_EQUAL
            : depth == eDepthStencilState::Always ? D3D12_COMPARISON_FUNC_ALWAYS : D3D12_COMPARISON_FUNC_NEVER;
        desc.DepthStencilState.StencilEnable = FALSE;
        desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.SampleMask = UINT_MAX;
        desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets = 1;
        desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline;
        if (FAILED(GetDevice()->GetID3D12Device()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipeline))))
            return nullptr;
        return mPipelineStates.emplace(key, std::move(pipeline)).first->second.Get();
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
        const D3D_SHADER_MACRO cutoutDefines[] = { { "YA_ALPHA_TEST", "1" }, { nullptr, nullptr } };
        return GetDevice()->CreatePixelShader(fileName, mPSBlob.ReleaseAndGetAddressOf())
            && GetDevice()->CreatePixelShader(fileName, mCutoutPSBlob.ReleaseAndGetAddressOf(), cutoutDefines);
    }

    void Shader::Bind(eRasterizerState rasterizer, eBlendState blend, eDepthStencilState depth)
    {
        auto* pipeline = GetPipelineState(bWireframe ? eRasterizerState::Wireframe : rasterizer, blend, depth);
        if (!pipeline) throw std::runtime_error("Graphics pipeline creation failed");
        GetDevice()->GetCommandList()->SetPipelineState(pipeline);
    }

    void Shader::Bind()
    {
        Bind(mRasterizerState, mBlendState, mDepthStencilState);
    }

    void Shader::Bind(eRenderingMode mode)
    {
        switch (mode)
        {
        case eRenderingMode::Opaque:
            Bind(mRasterizerState, eBlendState::Opaque, eDepthStencilState::LessEqual);
            break;
        case eRenderingMode::CutOut:
            Bind(mRasterizerState, eBlendState::Cutout, eDepthStencilState::LessEqual);
            break;
        case eRenderingMode::Transparent:
            Bind(mRasterizerState, eBlendState::Transparent, eDepthStencilState::Always);
            break;
        default:
            throw std::invalid_argument("Invalid rendering mode");
        }
    }
}
