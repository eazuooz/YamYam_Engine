#include "yaTexture.h"
#include <cwctype>

namespace ya::graphics
{
    Texture::Texture() : Resource(enums::eResourceType::Texture) {}
    Texture::~Texture() { ReleaseGpu(); }

    void Texture::ReleaseGpu()
    {
        if (GetDevice())
            GetDevice()->RetireResource(std::move(mTexture), mSrv, mRtv, mDsv, mUav);
        else
            mTexture.Reset();
        mSrv = {}; mRtv = {}; mDsv = {}; mUav = {};
    }

    HRESULT Texture::Save(const std::wstring&) { return E_NOTIMPL; }

    HRESULT Texture::Load(const std::wstring& path)
    {
        std::wstring ext = std::filesystem::path(path).extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return wchar_t(std::towlower(c)); });
        DirectX::ScratchImage source, converted;
        HRESULT hr;
        if (ext == L".dds")
            hr = DirectX::LoadFromDDSFile(path.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, source);
        else if (ext == L".tga")
            hr = DirectX::LoadFromTGAFile(path.c_str(), nullptr, source);
        else
            hr = DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, source);
        if (FAILED(hr)) return hr;
        const auto& metadata = source.GetMetadata();
        if (metadata.dimension != DirectX::TEX_DIMENSION_TEXTURE2D || metadata.arraySize != 1)
            return E_NOTIMPL;
        const DirectX::Image* image = source.GetImage(0, 0, 0);
        if (!image) return E_FAIL;
        // This initial sprite path uploads mip 0 as RGBA8. Arrays/cubemaps are not supported yet.
        if (DirectX::IsCompressed(image->format))
            hr = DirectX::Decompress(*image, DXGI_FORMAT_R8G8B8A8_UNORM, converted);
        else if (image->format != DXGI_FORMAT_R8G8B8A8_UNORM)
            hr = DirectX::Convert(*image, DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::TEX_FILTER_DEFAULT, 0.0f, converted);
        if (FAILED(hr)) return hr;
        if (converted.GetImageCount()) image = converted.GetImage(0, 0, 0);
        if (!Create(UINT(image->width), UINT(image->height), DXGI_FORMAT_R8G8B8A8_UNORM)) return E_FAIL;
        D3D12_SUBRESOURCE_DATA data = { image->pixels, LONG_PTR(image->rowPitch), LONG_PTR(image->slicePitch) };
        GetDevice()->UploadTexture(mTexture.Get(), data, mState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        mState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        return S_OK;
    }

    bool Texture::Create(UINT width, UINT height, DXGI_FORMAT format, UINT flags)
    {
        if (!width || !height) return false;
        ReleaseGpu();
        mFormat = format;
        const bool depth = (flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0;
        mState = depth ? D3D12_RESOURCE_STATE_DEPTH_WRITE : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAGS(flags));
        D3D12_CLEAR_VALUE clear = {};
        clear.Format = format;
        if (depth) clear.DepthStencil = { 1.0f, 0 };
        const bool renderable = depth || (flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
        if (FAILED(GetDevice()->GetID3D12Device()->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE,
            &desc, mState, renderable ? &clear : nullptr, IID_PPV_ARGS(mTexture.ReleaseAndGetAddressOf()))))
            return false;
        return CreateGpuView(flags);
    }

    bool Texture::CreateSolidColor(UINT32 rgba)
    {
        if (!Create(1, 1, DXGI_FORMAT_R8G8B8A8_UNORM)) return false;
        D3D12_SUBRESOURCE_DATA data = { &rgba, 4, 4 };
        GetDevice()->UploadTexture(mTexture.Get(), data, mState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        return true;
    }

    bool Texture::CreateSRV()
    {
        if (!mTexture || (mTexture->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) return false;
        if (!mSrv) mSrv = GetDevice()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Format = mFormat;
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipLevels = 1;
        GetDevice()->GetID3D12Device()->CreateShaderResourceView(mTexture.Get(), &desc, mSrv.Cpu);
        return true;
    }

    bool Texture::CreateUAV()
    {
        if (!mTexture || !(mTexture->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)) return false;
        if (!mUav) mUav = GetDevice()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
        desc.Format = mFormat;
        desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        GetDevice()->GetID3D12Device()->CreateUnorderedAccessView(mTexture.Get(), nullptr, &desc, mUav.Cpu);
        return true;
    }

    bool Texture::CreateRTV()
    {
        if (!mTexture || !(mTexture->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)) return false;
        if (!mRtv) mRtv = GetDevice()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        GetDevice()->GetID3D12Device()->CreateRenderTargetView(mTexture.Get(), nullptr, mRtv.Cpu);
        return true;
    }

    bool Texture::CreateDSV()
    {
        if (!mTexture || !(mTexture->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) return false;
        if (!mDsv) mDsv = GetDevice()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        GetDevice()->GetID3D12Device()->CreateDepthStencilView(mTexture.Get(), nullptr, mDsv.Cpu);
        return true;
    }

    bool Texture::CreateGpuView(UINT flags)
    {
        if (flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) return CreateDSV();
        if (!CreateSRV()) return false;
        if ((flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) && !CreateRTV()) return false;
        if ((flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) && !CreateUAV()) return false;
        return true;
    }

    void Texture::Transition(D3D12_RESOURCE_STATES state)
    {
        if (mState == state) return;
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(mTexture.Get(), mState, state);
        GetDevice()->GetCommandList()->ResourceBarrier(1, &barrier);
        mState = state;
    }

    void Texture::Bind(eShaderStage, UINT)
    {
        assert(mSrv && mState == D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        auto heap = GetDevice()->GetSrvHeap();
        auto list = GetDevice()->GetCommandList();
        list->SetDescriptorHeaps(1, heap.GetAddressOf());
        list->SetGraphicsRootDescriptorTable(1, mSrv.Gpu);
    }
}
