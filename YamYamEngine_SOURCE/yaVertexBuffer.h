#pragma once
#include "yaGraphicDevice_DX11.h"

namespace ya::graphics
{
	class VertexBuffer : public GpuBuffer
	{
	public:
		VertexBuffer();
		virtual ~VertexBuffer();

		bool Create(const std::vector<Vertex>& vertexes);
		void Bind();
	};
}
