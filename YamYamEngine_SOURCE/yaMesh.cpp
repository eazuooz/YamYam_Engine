#include "yaMesh.h"

namespace ya
{
	Mesh::Data::Data()
		: mTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
		, vertices{}
		, indices{}
	{

	}

	Mesh::Data::~Data()
	{
	}

	Mesh::Mesh()
		: Resource(enums::eResourceType::Mesh)
	{

	}

	Mesh::~Mesh()
	{

	}

	HRESULT Mesh::Save(const std::wstring& path)
	{
		return S_OK;
	}

	HRESULT Mesh::Load(const std::wstring& path)
	{

		return S_OK;
	}

	bool Mesh::CreateVB(const std::vector<graphics::Vertex>& vertices)
	{
		mData.vertices = vertices;
		return mVB.Create(vertices);
	}

	bool Mesh::CreateIB(const std::vector<UINT>& indices)
	{
		mData.indices = indices;
		return mIB.Create(indices);
	}

	void Mesh::SetVertexBufferParams(UINT vertexCount, D3D11_INPUT_ELEMENT_DESC* layout, const void* pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength)
	{
		mInputLayout.CreateInputLayout(vertexCount, layout, pShaderBytecodeWithInputSignature, BytecodeLength);
	}

	void Mesh::Bind() 
	{
		mInputLayout.Bind();

		mVB.Bind();
		mIB.Bind();

		graphics::GetDevice()->BindPrimitiveTopology(mData.mTopology);
	}

	
}