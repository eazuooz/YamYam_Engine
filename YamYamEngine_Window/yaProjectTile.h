#pragma once
#include "..//YamYamEngine_SOURCE//yaGameObject.h"

namespace ya
{
	class ProjectTile : public GameObject
	{
	public:
		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render(const Matrix& view, const Matrix& projection) override;
	};
}
