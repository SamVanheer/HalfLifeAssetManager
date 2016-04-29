#ifndef SOUNDSYSTEM_CSOUNDSYSTEM_H
#define SOUNDSYSTEM_CSOUNDSYSTEM_H

#include <list>

#include "SoundConstants.h"

#include "ISoundSystem.h"

namespace FMOD
{
class System;
class Sound;
class Channel;
}

namespace filesystem
{
class IFileSystem;
}

/**
*	@ingroup SoundSystem
*
*	@{
*/

namespace soundsystem
{
class CSoundSystem final : public ISoundSystem
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
	CSoundSystem();
	~CSoundSystem();

	bool Connect( CreateInterfaceFn appFactory, CreateInterfaceFn fileSystemFactory ) override final;

	void Disconnect() override final;

	bool Initialize() override final;
	void Shutdown() override final;

	void RunFrame() override final;

public:

	//Sound playback API

	void PlaySound( const char* pszFilename, float flVolume, int iPitch ) override final;

	void StopAllSounds() override final;

private:
	size_t GetSoundForPlayback();

private:
	filesystem::IFileSystem* m_pFileSystem = nullptr;

	FMOD::System* m_pSystem = nullptr;

	Sound_t m_Sounds[ MAX_SOUNDS ];

	std::list<size_t> m_SoundsLRU;

private:
	CSoundSystem( const CSoundSystem& ) = delete;
	CSoundSystem& operator=( const CSoundSystem& ) = delete;
};
}

/** @} */

#endif //SOUNDSYSTEM_CSOUNDSYSTEM_H