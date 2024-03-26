#pragma once
#include "..\\YamYamEngine_SOURCE\\yaScene.h"

namespace ya
{
	class TitleScene : public Scene
	{
	public:
		TitleScene();
		~TitleScene();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render() override;

		void OnEnter() override;
		void OnExit() override;

	private:

	};
}
