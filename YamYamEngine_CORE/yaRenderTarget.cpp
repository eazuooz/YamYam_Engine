#include "yaRenderTarget.h"
#include <stdexcept>

namespace ya::graphics
{
    RenderTarget::RenderTarget(const RenderTargetSpecification& spec)
        : mSpecification(spec), mDepthAttachmentSpecification(eRenderTragetFormat::None), mDepthAttachment(nullptr)
    {
        for (const auto& attachment : spec.Attachments.Attachments)
        {
            if (attachment.TextureFormat == eRenderTragetFormat::DEPTH24STENCIL8)
                mDepthAttachmentSpecification = attachment;
            else
                mSpecifications.push_back(attachment);
        }
        Invalidate();
    }

    RenderTarget::~RenderTarget()
    {
        RetireDisplaySRV();
        for (auto* texture : mAttachments) delete texture;
        delete mDepthAttachment;
    }

    RenderTarget* RenderTarget::Create(const RenderTargetSpecification& spec) { return new RenderTarget(spec); }

    void RenderTarget::Invalidate()
    {
        if (mSpecification.Samples != 1) throw std::runtime_error("MSAA render targets are not supported yet");
        // Destructors defer GPU resource/descriptor release until the submitted frame completes.
        RetireDisplaySRV();
        for (auto* texture : mAttachments) delete texture;
        mAttachments.clear();
        delete mDepthAttachment;
        mDepthAttachment = nullptr;
        for (const auto& attachment : mSpecifications)
        {
            auto texture = std::make_unique<Texture>();
            const DXGI_FORMAT format = attachment.TextureFormat == eRenderTragetFormat::RED_INTEGER
                ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R8G8B8A8_UNORM;
            if (!texture->Create(mSpecification.Width, mSpecification.Height, format, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET))
                throw std::runtime_error("Render-target color allocation failed");
            mAttachments.push_back(texture.release());
        }
        if (mDepthAttachmentSpecification.TextureFormat != eRenderTragetFormat::None)
        {
            auto depth = std::make_unique<Texture>();
            if (!depth->Create(mSpecification.Width, mSpecification.Height, DXGI_FORMAT_D24_UNORM_S8_UINT,
                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
                throw std::runtime_error("Render-target depth allocation failed");
            mDepthAttachment = depth.release();
        }
    }

    D3D12_GPU_DESCRIPTOR_HANDLE RenderTarget::GetDisplaySRV()
    {
        if (mDisplaySrv) return mDisplaySrv.Gpu;
        if (mAttachments.empty() || mAttachments[0]->GetFormat() != DXGI_FORMAT_R8G8B8A8_UNORM)
            throw std::runtime_error("Display SRV requires an RGBA8 color attachment");
        mDisplaySrv = GetDevice()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        desc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, 1, 2,
            D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1);
        desc.Texture2D.MipLevels = 1;
        GetDevice()->GetID3D12Device()->CreateShaderResourceView(mAttachments[0]->GetResource(), &desc, mDisplaySrv.Cpu);
        return mDisplaySrv.Gpu;
    }

    void RenderTarget::RetireDisplaySRV()
    {
        if (!mDisplaySrv) return;
        Microsoft::WRL::ComPtr<ID3D12Resource> resource = mAttachments[0]->GetResource();
        GetDevice()->RetireResource(std::move(resource), mDisplaySrv);
        mDisplaySrv = {};
    }

    void RenderTarget::Bind()
    {
        if (mPendingWidth && mPendingHeight)
        {
            Resize(mPendingWidth, mPendingHeight);
            mPendingWidth = mPendingHeight = 0;
        }
        assert(!mAttachments.empty() && mAttachments.size() <= D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT);
        auto list = GetDevice()->GetCommandList();
        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
        const float clear[4] = {};
        for (size_t i = 0; i < mAttachments.size(); ++i)
        {
            mAttachments[i]->Transition(D3D12_RESOURCE_STATE_RENDER_TARGET);
            rtvs[i] = mAttachments[i]->GetRTV();
            list->ClearRenderTargetView(rtvs[i], clear, 0, nullptr);
        }
        D3D12_CPU_DESCRIPTOR_HANDLE dsv = {};
        if (mDepthAttachment)
        {
            dsv = mDepthAttachment->GetDSV();
            list->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
        }
        list->OMSetRenderTargets(UINT(mAttachments.size()), rtvs, FALSE, mDepthAttachment ? &dsv : nullptr);
        GetDevice()->BindViewportAndScissor(mSpecification.Width, mSpecification.Height);
    }

    void RenderTarget::Unbind()
    {
        GetDevice()->GetCommandList()->OMSetRenderTargets(0, nullptr, FALSE, nullptr);
        for (auto* texture : mAttachments) texture->Transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    void RenderTarget::RequestResize(UINT width, UINT height)
    {
        if (!width || !height || width > 8192 || height > 8192) return;
        mPendingWidth = width;
        mPendingHeight = height;
    }

    void RenderTarget::Resize(UINT width, UINT height)
    {
        if (!width || !height || width > 8192 || height > 8192) return;
        if (width == mSpecification.Width && height == mSpecification.Height) return;
        mSpecification.Width = width;
        mSpecification.Height = height;
        Invalidate();
    }

    int RenderTarget::ReadPixel(uint32_t, int, int) { return 0; } // Object picking is a separate readback path.

    void RenderTarget::ClearAttachment(UINT index, const void* value)
    {
        if (index >= mAttachments.size() || !value) return;
        auto* texture = mAttachments[index];
        texture->Transition(D3D12_RESOURCE_STATE_RENDER_TARGET);
        GetDevice()->GetCommandList()->ClearRenderTargetView(texture->GetRTV(), static_cast<const float*>(value), 0, nullptr);
    }
}
