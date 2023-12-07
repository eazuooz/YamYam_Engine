#pragma once
#include "yaComponent.h"


namespace ya
{
	class Collider : public Component
	{
	public:
		Collider();
		~Collider();

		virtual void Initialize();
		virtual void Update();
		virtual void LateUpdate();
		virtual void Render(HDC hdc);

		Vector2 GetOffset() { return mOffset; }
		void SetOffset(Vector2 offset) { mOffset = offset; }

	private:
		Vector2 mOffset;
	};
}
