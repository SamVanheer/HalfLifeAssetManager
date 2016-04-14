#ifndef SOUNDSYSTEM_CSOUNDSYSTEM_H
#define SOUNDSYSTEM_CSOUNDSYSTEM_H

#include <list>

#include "SoundConstants.h"

namespace FMOD
{
class System;
class Sound;
class Channel;
}

namespace soundsystem
{
class CSoundSystem final
{
public:
	//Maximum number of sounds to play simultaneously.
	static const size_t MAX_SOUNDS = 16;

private:

	struct Sound_t
	{
		FMOD::Sound* pSound;
		FMOD::Channel* pChannel;
	};

public:
	static CSoundSystem& CreateInstance();
	static void DestroyInstance();
	static bool InstanceExists();
	static CSoundSystem& GetInstance();

	CSoundSystem();
	~CSoundSystem();

	bool Initialize();
	void Shutdown();

	void RunFrame();

public:

	//Sound playback API
	void PlaySound( const char* pszFilename, const float flVolume, const int iPitch );

	void StopAllSounds();

private:
	size_t GetSoundForPlayback();

private:
	static CSoundSystem* m_pInstance;

	FMOD::System* m_pSystem = nullptr;

	Sound_t m_Sounds[ MAX_SOUNDS ];

	std::list<size_t> m_SoundsLRU;

private:
	CSoundSystem( const CSoundSystem& ) = delete;
	CSoundSystem& operator=( const CSoundSystem& ) = delete;
};
}

inline soundsystem::CSoundSystem& soundSystem()
{
	return soundsystem::CSoundSystem::GetInstance();
}

#endif //SOUNDSYSTEM_CSOUNDSYSTEM_H