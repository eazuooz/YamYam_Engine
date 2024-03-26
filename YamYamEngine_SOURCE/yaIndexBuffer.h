#pragma once
#include "yaGraphicDevice_DX11.h"

namespace ya::graphics
{
	class IndexBuffer : public GpuBuffer
	{
	public:
		IndexBuffer();
		~IndexBuffer();

		bool Create(const std::vector<UINT>& indices);
		void Bind() const;

	private:
	};
}
