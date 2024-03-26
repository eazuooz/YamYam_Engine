#pragma once
#include "yaComponent.h"


namespace ya
{
	class AudioListener : public Component
	{
	public:
		AudioListener();
		~AudioListener();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render() override;
	};
}
