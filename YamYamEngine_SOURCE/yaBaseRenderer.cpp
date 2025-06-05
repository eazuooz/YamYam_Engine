#include "yaBaseRenderer.h"
#include "yaTransform.h"
#include "yaGameObject.h"
#include "yaGraphicDevice_DX11.h"

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
			graphics::GetDevice<graphics::GraphicDevice_DX11>()->DrawIndexed(mMesh->GetIndexCount(), 0, 0);
	}
}
