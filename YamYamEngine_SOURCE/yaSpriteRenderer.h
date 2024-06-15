#pragma once
#include "yaEntity.h"
#include "yaComponent.h"
#include "yaTexture.h"
#include "yaMaterial.h"
#include "yaMesh.h"

namespace ya
{
	class SpriteRenderer : public Component
	{
	public:
		SpriteRenderer();
		virtual ~SpriteRenderer();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render() override;

		[[noreturn]] void SetSprite(graphics::Texture* sprite) { mSprite = sprite; }
		[[noreturn]] void SetMaterial(Material* material) { mMaterial = material; }

	private:
		graphics::Texture* mSprite;
		Material* mMaterial;
		Mesh* mMesh;
	};
}
