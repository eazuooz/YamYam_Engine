#pragma once
#include "..//YamYamEngine_SOURCE//yaScript.h"
#include "..//YamYamEngine_SOURCE//yaTexture.h"

namespace ya
{
	class ProjectTileScript : public Script
	{
	public:
		ProjectTileScript();
		virtual ~ProjectTileScript();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render(const Matrix& view, const Matrix& projection) override;

		void OnCollisionEnter(Collider* other) override;
		void OnCollisionStay(Collider* other) override;
		void OnCollisionExit(Collider* other) override;

	private:

	};
}

