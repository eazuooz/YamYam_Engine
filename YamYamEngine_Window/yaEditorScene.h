#pragma once
#include <yaScene.h>

namespace ya
{
	class EditorScene : public Scene
	{
	public:
		EditorScene();
		virtual ~EditorScene();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render() override;

		void OnEnter() override;
		void OnExit() override;

	private:
	};
}
