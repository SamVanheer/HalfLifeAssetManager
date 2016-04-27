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
/**
*	A sound system that can be used to play back sounds. Sounds are non-looping.
*/
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

	/**
	*	Must be called every frame.
	*/
	void RunFrame();

public:

	//Sound playback API

	/**
	*	Plays a sound by name. The filename is relative to the game's sound directory, and is looked up using the filesystem.
	*	@param pszFilename Sound filename.
	*	@param flVolume Volume. Expressed as a range between [0, 1].
	*	@param iPitch Pitch amount. Expressed as a range between [0, 255].
	*/
	void PlaySound( const char* pszFilename, float flVolume, int iPitch );

	/**
	*	Stops all sounds that are currently playing.
	*/
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

//TODO move to its own library
inline soundsystem::CSoundSystem& soundSystem()
{
	return soundsystem::CSoundSystem::GetInstance();
}

#endif //SOUNDSYSTEM_CSOUNDSYSTEM_H