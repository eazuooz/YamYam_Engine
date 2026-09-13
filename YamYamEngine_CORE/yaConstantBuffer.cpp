#include "yaConstantBuffer.h"
#include <stdexcept>

namespace ya::graphics
{
    ConstantBuffer::ConstantBuffer(eCBType type) : mType(type) {}

    ConstantBuffer::~ConstantBuffer()
    {
        for (auto& frame : mFrames)
            for (auto& page : frame.Pages)
            {
                if (page.Mapped) page.Resource->Unmap(0, nullptr);
                if (GetDevice()) GetDevice()->RetireResource(std::move(page.Resource));
            }
    }

    bool ConstantBuffer::AddPage(FrameStorage& frame)
    {
        Page page;
        auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto desc = CD3DX12_RESOURCE_DESC::Buffer(UINT64(mStride) * mDrawsPerPage);
        if (FAILED(GetDevice()->GetID3D12Device()->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE,
            &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&page.Resource))))
            return false;
        CD3DX12_RANGE readRange(0, 0);
        if (FAILED(page.Resource->Map(0, &readRange, reinterpret_cast<void**>(&page.Mapped)))) return false;
        frame.Pages.push_back(std::move(page));
        return true;
    }

    bool ConstantBuffer::Create(UINT size, const void* data)
    {
        if (!size || size > D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16 || mSize) return false;
        mSize = size;
        mStride = (size + 255) & ~255u;
        mDrawsPerPage = 65536u / mStride;
        for (auto& frame : mFrames)
            if (!AddPage(frame)) return false;
        if (data) SetData(data);
        return true;
    }

    void ConstantBuffer::BeginFrame()
    {
        mFrames[GetDevice()->GetFrameIndex()].NextDraw = 0;
        mCurrentAddress = 0;
    }

    void ConstantBuffer::SetData(const void* data)
    {
        if (!data || !mSize) throw std::runtime_error("Invalid constant buffer data");
        auto& frame = mFrames[GetDevice()->GetFrameIndex()];
        const size_t pageIndex = frame.NextDraw / mDrawsPerPage;
        const size_t offset = (frame.NextDraw % mDrawsPerPage) * mStride;
        if (pageIndex == frame.Pages.size() && !AddPage(frame))
            throw std::runtime_error("Constant buffer upload page allocation failed");
        auto& page = frame.Pages[pageIndex];
        memcpy(page.Mapped + offset, data, mSize);
        mCurrentAddress = page.Resource->GetGPUVirtualAddress() + offset;
        ++frame.NextDraw;
    }

    void ConstantBuffer::Bind(eShaderStage) const
    {
        assert(mCurrentAddress != 0);
        GetDevice()->GetCommandList()->SetGraphicsRootConstantBufferView(UINT(mType), mCurrentAddress);
    }
}
