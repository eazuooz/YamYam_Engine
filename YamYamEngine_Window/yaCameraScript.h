#pragma once
#include "..\\YamYamEngine_SOURCE\\yaScript.h"
#include "..\\YamYamEngine_SOURCE\\yaTransform.h"

namespace ya
{
	class CameraScript : public Script
	{
	public:
		CameraScript();
		~CameraScript();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render(HDC hdc) override;

	private:

	};
}