#include "yaBaseRenderer.h"
#include "yaTransform.h"
#include "yaGameObject.h"
#include "yaGraphicDevice_DX12.h"

namespace ya
{
	BaseRenderer::BaseRenderer(eComponentType type)
		: Component(type)
		, mMaterial(nullptr)
	{
	}
	BaseRenderer::~BaseRenderer()
	{
	}
	void BaseRenderer::Initialize()
	{
	}
	void BaseRenderer::Update()
	{
	}
	void BaseRenderer::LateUpdate()
	{

	}
	void BaseRenderer::Render(const Matrix& view, const Matrix& projection)
	{
		Transform* tr = GetOwner()->GetComponent<Transform>();
		if (tr)
			tr->Bind(view, projection);

		if (mMesh)
			mMesh->Bind();

		if (mMaterial)
			mMaterial->BindShader();
	}

	void BaseRenderer::Draw()
	{
		if (mMesh)
			graphics::GetDevice()->DrawIndexedInstanced(mMesh->GetIndexCount(), 1, 0, 0, 0);
	}
}
