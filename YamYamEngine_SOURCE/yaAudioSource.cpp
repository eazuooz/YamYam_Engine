#include "yaAudioSource.h"
#include "yaTransform.h"
#include "yaGameObject.h"
#include "yaFmod.h"

namespace ya
{
	AudioSource::AudioSource()
		: Component(eComponentType::AudioSource)
		, mAudioClip(nullptr)
	{
	}

	AudioSource::~AudioSource()
	{
	}

	void AudioSource::Initialize()
	{
	}

	void AudioSource::Update()
	{
	}

	void AudioSource::LateUpdate()
	{
		Transform* tr = GetOwner()->GetComponent<Transform>();
		Vector3 pos = tr->GetPosition();

		//mAudioClip->Set3DAttributes(pos);
	}

	void AudioSource::Render(const Matrix& view, const Matrix& projection)
	{
	}

	void AudioSource::Play() const
	{
		mAudioClip->Play();
	}

	void AudioSource::Stop() const
	{
		mAudioClip->Stop();
	}

	void AudioSource::SetLoop(bool loop) const
	{
		mAudioClip->SetLoop(loop);
	}

}
