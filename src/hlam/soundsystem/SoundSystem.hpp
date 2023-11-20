#pragma once

#include <array>
#include <list>
#include <memory>

#include <spdlog/logger.h>

#include <al.h>
#include <alc.h>

#include "soundsystem/SoundConstants.hpp"

#include "soundsystem/ISoundSystem.hpp"

class IFileSystem;

namespace nqr
{
class NyquistIO;
}

/**
*	@ingroup SoundSystem
*
*	@{
*/

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
	SoundSystem(const std::shared_ptr<spdlog::logger>& logger);
	~SoundSystem();
	SoundSystem(const SoundSystem&) = delete;
	SoundSystem& operator=(const SoundSystem&) = delete;

	bool IsSoundAvailable() const override final { return _device != nullptr; }

	bool Initialize() override final;
	void Shutdown() override final;

	void SetMuted(bool muted) override final;

	void RunFrame() override final;

public:
	void PlaySound(std::string_view fileName, float volume, int pitch) override final;

	void StopAllSounds() override final;

private:
	size_t GetSoundForPlayback();

	bool CheckALErrorsCore(const char* file, int line);
	std::unique_ptr<SoundSystem::Sound> TryLoadFile(const std::string& fileName);

private:
	std::shared_ptr<spdlog::logger> _logger;

	ALCdevice* _device{};
	ALCcontext* _context{};

	std::array<std::unique_ptr<Sound>, MAX_SOUNDS> _sounds;

	std::list<size_t> _soundsLRU;

	std::unique_ptr<nqr::NyquistIO> m_Loader;
};

/**
*	@brief Wraps around a sound system and converts relative paths to absolute paths.
*/
class SoundSystemWrapper final : public ISoundSystem
{
public:
	explicit SoundSystemWrapper(ISoundSystem* soundSystem, IFileSystem* fileSystem)
		: _soundSystem(soundSystem)
		, _fileSystem(fileSystem)
	{
	}

	bool IsSoundAvailable() const override { return _soundSystem->IsSoundAvailable(); }

	bool Initialize() override { return _soundSystem->Initialize(); }

	void Shutdown() override { _soundSystem->Shutdown(); }

	void SetMuted(bool muted) override { _soundSystem->SetMuted(muted); }

	void RunFrame() override { _soundSystem->RunFrame(); }

	void PlaySound(std::string_view fileName, float volume, int pitch) override;

	void StopAllSounds() override { _soundSystem->StopAllSounds(); }

private:
	ISoundSystem* const _soundSystem;
	IFileSystem* const _fileSystem;
};

/** @} */
