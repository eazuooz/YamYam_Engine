#pragma once
#include "yaGraphicDevice_DX12.h"

namespace ya::graphics
{
	class IndexBuffer : public GpuBuffer
	{
	public:
		IndexBuffer();
		virtual ~IndexBuffer();

		bool Create(const std::vector<UINT>& indices);
		void Bind() const;

		UINT GetIndexCount() const { return mIndexCount; }

	private:
		UINT mIndexCount;
	};
}
