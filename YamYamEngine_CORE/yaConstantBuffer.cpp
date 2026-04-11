#include "yaConstantBuffer.h"

namespace ya::graphics
{
	ConstantBuffer::ConstantBuffer(eCBType type)
		: mSize(0)
		, mType(type)
		, mMappedData(nullptr)
	{
	}

	ConstantBuffer::~ConstantBuffer()
	{
		if (buffer && mMappedData)
		{
			buffer->Unmap(0, nullptr);
			mMappedData = nullptr;
		}
	}

	bool ConstantBuffer::Create(UINT size, void* data)
	{
		// DX12 constant buffer must be 256-byte aligned
		mSize = size;
		UINT alignedSize = (size + 255) & ~255;

		bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(alignedSize);

		if (!GetDevice()->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(buffer.GetAddressOf())))
			return false;

		// Persistent map — upload heap stays mapped for the lifetime of the buffer
		CD3DX12_RANGE readRange(0, 0);
		if (FAILED(buffer->Map(0, &readRange, &mMappedData)))
			return false;

		if (data)
			memcpy(mMappedData, data, mSize);

		return true;
	}

	void ConstantBuffer::SetData(void* data) const
	{
		if (mMappedData && data)
			memcpy(mMappedData, data, mSize);
	}

	void ConstantBuffer::Bind(eShaderStage stage) const
	{
		// Root parameter index == CB slot (CBSLOT_TRANSFORM = 0 → root param 0)
		UINT rootParamIndex = static_cast<UINT>(mType);
		GetDevice()->GetCommandList()->SetGraphicsRootConstantBufferView(
			rootParamIndex,
			buffer->GetGPUVirtualAddress());
	}
}
