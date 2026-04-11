#include "yaIndexBuffer.h"
#include "yaGraphicDevice_DX12.h"

namespace ya::graphics
{
	IndexBuffer::IndexBuffer()
		: mIndexCount(0)
	{
	}

	IndexBuffer::~IndexBuffer()
	{
	}

	bool IndexBuffer::Create(const std::vector<UINT>& indices)
	{
		mIndexCount = static_cast<UINT>(indices.size());

		const UINT bufferSize = mIndexCount * sizeof(UINT);

		bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

		GetDevice()->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(buffer.GetAddressOf()));

		UINT8* pData = nullptr;
		CD3DX12_RANGE readRange(0, 0);
		buffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));
		memcpy(pData, indices.data(), bufferSize);
		buffer->Unmap(0, nullptr);

		mIndexBufferView.BufferLocation = buffer->GetGPUVirtualAddress();
		mIndexBufferView.Format         = DXGI_FORMAT_R32_UINT;
		mIndexBufferView.SizeInBytes    = bufferSize;

		return true;
	}

	void IndexBuffer::Bind() const
	{
		GetDevice()->GetCommandList()->IASetIndexBuffer(&mIndexBufferView);
	}
}
