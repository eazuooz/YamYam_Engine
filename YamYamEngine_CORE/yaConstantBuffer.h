#pragma once
#include "yaGraphicDevice_DX12.h"
#include <array>

namespace ya::graphics
{
    class ConstantBuffer
    {
    public:
        explicit ConstantBuffer(eCBType type);
        ~ConstantBuffer();
        ConstantBuffer(const ConstantBuffer&) = delete;
        ConstantBuffer& operator=(const ConstantBuffer&) = delete;
        bool Create(UINT size, const void* data = nullptr);
        // Call once after waiting for the current frame's allocator/fence.
        void BeginFrame();
        void SetData(const void* data);
        void Bind(eShaderStage stage) const;
        D3D12_GPU_VIRTUAL_ADDRESS GetCurrentGpuAddress() const { return mCurrentAddress; }

    private:
        struct Page
        {
            Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
            BYTE* Mapped = nullptr;
        };
        struct FrameStorage
        {
            std::vector<Page> Pages;
            size_t NextDraw = 0;
        };
        bool AddPage(FrameStorage& frame);
        std::array<FrameStorage, 2> mFrames;
        UINT mSize = 0, mStride = 0, mDrawsPerPage = 0;
        eCBType mType;
        D3D12_GPU_VIRTUAL_ADDRESS mCurrentAddress = 0;
    };
}
