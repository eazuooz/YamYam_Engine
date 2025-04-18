#pragma once
#include "yaComponent.h"
#include "yaTexture.h"
#include "yaMaterial.h"
#include "yaMesh.h"
#include "yaBaseRenderer.h"

namespace ya
{
	class SpriteRenderer : public BaseRenderer
	{
	public:
		SpriteRenderer();
		virtual ~SpriteRenderer();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render(const Matrix& view, const Matrix& projection) override;

		void SetSprite(graphics::Texture* sprite) { mSprite = sprite; }

	private:
		graphics::Texture* mSprite;
	};
}
