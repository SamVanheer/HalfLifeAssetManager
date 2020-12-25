#pragma once

#include <list>
#include <memory>

#include <al.h>
#include <alc.h>

#include "SoundConstants.hpp"

#include "ISoundSystem.hpp"

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
class SoundSystem final : public ISoundSystem
{
public:
	//Maximum number of sounds to play simultaneously.
	static const size_t MAX_SOUNDS = 16;

public:
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
	SoundSystem();
	~SoundSystem();

	bool IsSoundAvailable() const override final { return m_Device != nullptr; }

	bool Initialize(filesystem::IFileSystem* filesystem) override final;
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
	SoundSystem( const SoundSystem& ) = delete;
	SoundSystem& operator=( const SoundSystem& ) = delete;
};
}

/** @} */
