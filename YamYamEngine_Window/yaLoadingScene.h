#pragma once
#include "..\YamYamEngine_CORE\yaScene.h"

namespace ya
{
    class LoadingScene : public Scene
    {
    public:
        void Initialize() override;
        void Update() override;
        void LateUpdate() override;
        void Render() override;
        void OnEnter() override;
        void OnExit() override;
    private:
        bool mLoaded = false;
    };
}
