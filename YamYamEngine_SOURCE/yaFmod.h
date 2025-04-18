#pragma once
#include <fmod_studio.hpp>
#include <fmod.hpp>
#include <fmod_common.h>
#include <fmod_codec.h>

#include "CommonInclude.h"

#ifdef _DEBUG
#pragma comment(lib, "..\\External\\Library\\Fmod\\Debug\\fmodL_vc.lib")
#pragma comment(lib, "..\\External\\Library\\Fmod\\Debug\\fmodstudioL_vc.lib")
#else
#pragma comment(lib, "..\\External\\Library\\Fmod\\Release\\fmod_vc.lib")
#pragma comment(lib, "..\\External\\Library\\Fmod\\Release\\fmodstudio_vc.lib")
#endif

namespace ya
{
	using namespace math;
	class Fmod
	{
	public:
		static void Initialize();
		static bool CreateSound(const std::string& path, FMOD::Sound** sound);
		static void SoundPlay(FMOD::Sound* sound, FMOD::Channel** channel);
		static void Set3DListenerAttributes(const Vector3& pos);
		static void Release();

	private:
		static FMOD::Studio::System* mSystem;
		static FMOD::System* mCoreSystem;
	};
}
