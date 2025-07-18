#pragma once
#include <yaGameObject.h>

namespace ya
{
	class Player : public GameObject
	{
	public:
		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render(const Matrix& view, const Matrix& projection) override;
	};
}
