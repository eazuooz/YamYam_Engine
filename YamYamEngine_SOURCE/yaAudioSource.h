#pragma once
#include "yaComponent.h"
#include "yaAudioClip.h"

namespace ya
{
	class AudioSource final : public Component
	{
	public:
		AudioSource();
		virtual ~AudioSource();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render() override;

		void Play() const;
		void Stop() const;
		void SetLoop(bool loop) const;

		void SetClip(AudioClip* clip) { mAudioClip = clip; }
		AudioClip* GetClip() const { return mAudioClip; }

	private:
		AudioClip* mAudioClip;
	};
}
