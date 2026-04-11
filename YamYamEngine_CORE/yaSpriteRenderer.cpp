#include "yaSpriteRenderer.h"
#include "yaGameObject.h"
#include "yaTexture.h"
#include "yaRenderer.h"
#include "yaResources.h"

namespace ya
{
	SpriteRenderer::SpriteRenderer()
		: BaseRenderer(eComponentType::SpriteRenderer)
	{
	}
	SpriteRenderer::~SpriteRenderer()
	{
	}
	void SpriteRenderer::Initialize()
	{
		BaseRenderer::Initialize();

		Mesh* mesh = Resources::Find<Mesh>(L"RectMesh");
		Material* material = Resources::Find<Material>(L"Sprite-Default-Material");

		SetMesh(mesh);
		SetMaterial(material);
	}

	void SpriteRenderer::Update()
	{
		BaseRenderer::Update();
	}

	void SpriteRenderer::LateUpdate()
	{
		BaseRenderer::LateUpdate();
	}

	void SpriteRenderer::Render(const Matrix& view, const Matrix& projection)
	{
		BaseRenderer::Render(view, projection);

		if (mSprite)
			mSprite->Bind(eShaderStage::PS, (UINT)eTextureType::Sprite);

		BaseRenderer::Draw();
	}

}
