#include "yaAudioListener.h"
#include "yaTransform.h"
#include "yaGameObject.h"
#include "yaFmod.h"

namespace ya
{
	AudioListener::AudioListener()
		: Component(eComponentType::AudioListener)
	{
	}

	AudioListener::~AudioListener()
	{
	}

	void AudioListener::Initialize()
	{
	}

	void AudioListener::Update()
	{
	}

	void AudioListener::LateUpdate()
	{
		Transform* tr = nullptr;
		tr = GetOwner()->GetComponent<Transform>();

		Vector3 pos;
		Fmod::Set3DListenerAttributes(pos);
	}

	void AudioListener::Render(const Matrix& view, const Matrix& projection)
	{
	}
}
