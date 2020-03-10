#ifndef SOUNDSYSTEM_CSOUNDSYSTEM_H
#define SOUNDSYSTEM_CSOUNDSYSTEM_H

#include <list>
#include <memory>

#include <al.h>
#include <alc.h>

#include "SoundConstants.h"

#include "ISoundSystem.h"

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

	struct Sound
	{
		Sound()
		{
			alGenBuffers(1, &buffer);
			alGenSources(1, &source);
		}

		~Sound()
		{
			alDeleteSources(1, &source);
			alDeleteBuffers(1, &buffer);
		}

		ALuint buffer = 0;
		ALuint source = 0;
	};

public:
	CSoundSystem();
	~CSoundSystem();

	bool Connect( const CreateInterfaceFn* const pFactories, const size_t uiNumFactories ) override final;

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

	ALCdevice* m_Device = nullptr;
	ALCcontext* m_Context = nullptr;

	std::unique_ptr<Sound> m_Sounds[MAX_SOUNDS];

	std::list<size_t> m_SoundsLRU;

private:
	CSoundSystem( const CSoundSystem& ) = delete;
	CSoundSystem& operator=( const CSoundSystem& ) = delete;
};
}

/** @} */

#endif //SOUNDSYSTEM_CSOUNDSYSTEM_H