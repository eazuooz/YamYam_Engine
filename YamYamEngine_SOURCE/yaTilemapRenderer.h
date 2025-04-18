#pragma once
#include "yaEntity.h"
#include "yaComponent.h"
#include "yaTexture.h"

namespace ya
{
	class TilemapRenderer : public Component
	{
	public:
		TilemapRenderer();
		~TilemapRenderer();

		void Initialize() override;
		void Update()  override;
		void LateUpdate()  override;
		void Render(const Matrix& view, const Matrix& projection)  override;

		void SetTexture(graphics::Texture* texture) { mTexture = texture; }
		void SetSize(math::Vector2 size) { mSize = size; }
		void SetIndex(Vector2 index) { mIndex = index; }

		Vector2 GetIndex() const { return mIndex; }

	public:
		static Vector2 TileSize;
		static Vector2 OriginTileSize;
		static Vector2 SelectedIndex;
		
	private:
		Vector2 mTileSize;
		graphics::Texture* mTexture;
		Vector2 mSize;
		Vector2 mIndex;
	};
}