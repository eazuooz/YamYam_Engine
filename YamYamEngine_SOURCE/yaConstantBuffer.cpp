#include "yaConstantBuffer.h"

namespace ya::graphics
{
	ConstantBuffer::ConstantBuffer()
		: mSize(0)
	{
	}

	ConstantBuffer::~ConstantBuffer()
	{
	}

	bool ConstantBuffer::Create(eCBType type, UINT size, void* data)
	{
		mType = type;
		mSize = size;
		desc.ByteWidth = size; 
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA sub = {};
		sub.pSysMem = data;

		bool succes = false;
		if (data == NULL)
			succes = GetDevice()->CreateBuffer(&desc, nullptr, buffer.GetAddressOf());
		else
			succes = GetDevice()->CreateBuffer(&desc, &sub, buffer.GetAddressOf());

		if (!succes)
			assert(NULL, "Create constant buffer failed!");

		return true;
	}

	void ConstantBuffer::SetData(void* data)
	{
		GetDevice()->SetDataBuffer(buffer.Get(), data, mSize);
	}

	void ConstantBuffer::Bind(eShaderStage stage)
	{
		GetDevice()->BindConstantBuffer(stage, mType, buffer.Get());
	}
}
