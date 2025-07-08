#include "yaVertexBuffer.h"
#include "yaGraphicDevice_DX12.h"

namespace ya::graphics
{
	VertexBuffer::VertexBuffer()
	{
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	bool VertexBuffer::Create(const std::vector<Vertex>& vertexes)
	{
		// Create the vertex buffer.
		{
			// Define the geometry for a triangle.
			//float aspectRatio = 1.0f;
			//Vertex triangleVertices[] =
			//{
			//	{ { 0.0f, 0.25f * 1600.0f / 900.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			//	{ { 0.25f, -0.25f * 1600.0f / 900.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			//	{ { -0.25f, -0.25f * 1600.0f / 900.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
			//};

			const UINT vertexBufferSize = vertexes.size() * sizeof(Vertex);// sizeof(triangleVertices);
			bufferDesc.Width = vertexBufferSize;
			// Note: using upload heaps to transfer static data like vert buffers is not 
			// recommended. Every time the GPU needs it, the upload heap will be marshalled 
			// over. Please read up on Default Heap usage. An upload heap is used here for 
			// code simplicity and because there are very few verts to actually transfer.
			//CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
			//CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);


			GetDevice()->CreateCommittedResource(
				&heapProps, 
				D3D12_HEAP_FLAG_NONE, 
				&bufferDesc, 
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr, 
				IID_PPV_ARGS(&buffer));

			//if (FAILED(mDevice->CreateCommittedResource(
			//	&heapProps,
			//	D3D12_HEAP_FLAG_NONE,
			//	&bufferDesc,
			//	D3D12_RESOURCE_STATE_GENERIC_READ,
			//	nullptr,
			//	IID_PPV_ARGS(&buffer))))

			//	assert(NULL, "CreateCommittedResource");

			// Copy the triangle data to the vertex buffer.
			UINT8* pVertexDataBegin;
			CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
			buffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
			memcpy(pVertexDataBegin, vertexes.data(), vertexBufferSize);
			buffer->Unmap(0, nullptr);

			// Initialize the vertex buffer view.
			mVertexBufferView.BufferLocation = buffer->GetGPUVirtualAddress();
			mVertexBufferView.StrideInBytes = sizeof(Vertex);
			mVertexBufferView.SizeInBytes = vertexBufferSize;
		}


		//desc.ByteWidth = sizeof(Vertex) * static_cast<UINT>(vertexes.size());
		//desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		//desc.Usage = D3D11_USAGE_DYNAMIC;
		//desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		//D3D11_SUBRESOURCE_DATA sub = {};
		//sub.pSysMem = vertexes.data();

		//if (!(GetDevice<GraphicDevice_DX11>()->CreateBuffer(&desc, &sub, buffer.GetAddressOf())))
		//	assert(NULL/*"Create vertex buffer failed!"*/);

		return true;
	}

	void VertexBuffer::Bind()
	{
		//UINT offset = 0;
		//UINT vertexSize = sizeof(Vertex);
		//GetDevice<GraphicDevice_DX11>()->BindVertexBuffer(0, 1, buffer.GetAddressOf(), &vertexSize, &offset);

		GetDevice()->BindVertexBuffer(0, 1, &mVertexBufferView);
	}
}
