#pragma once
#include "yaResource.h"
#include "yaGraphicDevice_DX12.h"
#include <DirectXTex.h>

#ifdef _DEBUG
#pragma comment(lib, "..\\External\\Library\\DirectXTex\\Debug\\DirectXTex.lib")
#else
#pragma comment(lib, "..\\External\\Library\\DirectXTex\\Release\\DirectXTex.lib")
#endif

namespace ya::graphics
{
    class Texture : public Resource
    {
    public:
        Texture();
        ~Texture() override;
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        HRESULT Save(const std::wstring& path) override;
        HRESULT Load(const std::wstring& path) override;
        // flags are D3D12_RESOURCE_FLAGS, not the old D3D11 bind flags.
        bool Create(UINT width, UINT height, DXGI_FORMAT format, UINT flags = D3D12_RESOURCE_FLAG_NONE);
        bool CreateSolidColor(UINT32 rgba);
        bool CreateSRV();
        bool CreateUAV();
        bool CreateRTV();
        bool CreateDSV();
        bool CreateGpuView(UINT flags);
        void Bind(eShaderStage stage, UINT startSlot);
        void Transition(D3D12_RESOURCE_STATES state);

        ID3D12Resource* GetResource() const { return mTexture.Get(); }
        D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return mRtv.Cpu; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return mDsv.Cpu; }
        D3D12_GPU_DESCRIPTOR_HANDLE GetSRV() const { return mSrv.Gpu; }
        DXGI_FORMAT GetFormat() const { return mFormat; }
        D3D12_RESOURCE_STATES GetState() const { return mState; }

    private:
        void ReleaseGpu();
        Microsoft::WRL::ComPtr<ID3D12Resource> mTexture;
        DescriptorHandle mSrv, mRtv, mDsv, mUav;
        DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;
        D3D12_RESOURCE_STATES mState = D3D12_RESOURCE_STATE_COMMON;
    };
}
