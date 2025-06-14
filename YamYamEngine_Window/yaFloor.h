#pragma once
#include <yaGameObject.h>

namespace ya
{
	class Floor : public GameObject
	{
	public:
		Floor();
		virtual ~Floor();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render(const Matrix& view, const Matrix& projection) override;
	};
}
