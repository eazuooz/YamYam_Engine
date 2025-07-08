#pragma once
#include "yaGraphicDevice_DX12.h"

namespace ya::graphics
{
	class VertexBuffer : public GpuBuffer
	{
	public:
		VertexBuffer();
		virtual ~VertexBuffer();

		bool Create(const std::vector<Vertex>& vertexes);
		void Bind();

	private:
		D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	};
}
