#pragma once
#include "yaCollider.h"

namespace ya
{
	class BoxCollider2D : public Collider
	{
	public:
		BoxCollider2D();
		virtual ~BoxCollider2D();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render() override;
	};
}
