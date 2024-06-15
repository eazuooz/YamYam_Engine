#pragma once
#include "yaCollider.h"

namespace ya
{
	class CircleCollider2D : public Collider
	{
	public:
		CircleCollider2D();
		virtual ~CircleCollider2D();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render() override;

	private:
		float mRadius;
	};
}
