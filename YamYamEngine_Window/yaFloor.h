#pragma once
#include "..//YamYamEngine_SOURCE//yaGameObject.h"

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
		void Render() override;
	};
}
