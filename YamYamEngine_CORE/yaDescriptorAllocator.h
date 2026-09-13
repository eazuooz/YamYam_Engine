#pragma once
#include "yaGraphics.h"
#include <stdexcept>

namespace ya::graphics
{
    struct DescriptorHandle
    {
        D3D12_CPU_DESCRIPTOR_HANDLE Cpu = {};
        D3D12_GPU_DESCRIPTOR_HANDLE Gpu = {};
        explicit operator bool() const { return Cpu.ptr != 0; }
    };

    // Render-thread owned. The device delays Free until the submitting frame completes.
    class DescriptorAllocator
    {
    public:
        void Create(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT capacity, bool shaderVisible)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = type;
            desc.NumDescriptors = capacity;
            desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mHeap.ReleaseAndGetAddressOf()))))
                throw std::runtime_error("Failed to create descriptor heap");
            mIncrement = device->GetDescriptorHandleIncrementSize(type);
            mShaderVisible = shaderVisible;
            mAllocated.assign(capacity, false);
            mFree.clear();
            for (UINT i = capacity; i > 0; --i)
                mFree.push_back(i - 1);
        }

        DescriptorHandle Allocate()
        {
            if (mFree.empty())
                throw std::runtime_error("Descriptor heap exhausted");
            const UINT index = mFree.back();
            mFree.pop_back();
            mAllocated[index] = true;
            DescriptorHandle handle;
            handle.Cpu.ptr = mHeap->GetCPUDescriptorHandleForHeapStart().ptr + SIZE_T(index) * mIncrement;
            if (mShaderVisible)
                handle.Gpu.ptr = mHeap->GetGPUDescriptorHandleForHeapStart().ptr + UINT64(index) * mIncrement;
            return handle;
        }

        void Free(DescriptorHandle handle)
        {
            if (!handle)
                return;
            const SIZE_T offset = handle.Cpu.ptr - mHeap->GetCPUDescriptorHandleForHeapStart().ptr;
            const SIZE_T index = offset / mIncrement;
            if (offset % mIncrement != 0 || index >= mAllocated.size() || !mAllocated[index])
                throw std::runtime_error("Invalid or duplicate descriptor release");
            mAllocated[index] = false;
            mFree.push_back(static_cast<UINT>(index));
        }

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetHeap() const { return mHeap; }
        size_t GetFreeCount() const { return mFree.size(); }

    private:
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mHeap;
        std::vector<UINT> mFree;
        std::vector<bool> mAllocated;
        UINT mIncrement = 0;
        bool mShaderVisible = false;
    };
}
