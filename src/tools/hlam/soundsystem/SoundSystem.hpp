#pragma once

#include <array>
#include <list>
#include <memory>

#include <spdlog/logger.h>

#include <al.h>
#include <alc.h>

#include "soundsystem/SoundConstants.hpp"

#include "soundsystem/ISoundSystem.hpp"

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
	SoundSystem(const std::shared_ptr<spdlog::logger>& logger);
	~SoundSystem();
	SoundSystem(const SoundSystem&) = delete;
	SoundSystem& operator=(const SoundSystem&) = delete;

	bool IsSoundAvailable() const override final { return _device != nullptr; }

	bool Initialize(filesystem::IFileSystem* filesystem) override final;
	void Shutdown() override final;

	void RunFrame() override final;

public:
	void PlaySound(std::string_view fileName, float volume, int pitch) override final;

	void StopAllSounds() override final;

private:
	size_t GetSoundForPlayback();

	bool CheckALErrorsCore(const char* file, int line);
	std::unique_ptr<SoundSystem::Sound> TryLoadWaveFile(const std::string& fileName);
	std::unique_ptr<SoundSystem::Sound> TryLoadOggVorbis(const std::string& fileName);

private:
	std::shared_ptr<spdlog::logger> _logger;

	filesystem::IFileSystem* _fileSystem{};

	ALCdevice* _device{};
	ALCcontext* _context{};

	std::array<std::unique_ptr<Sound>, MAX_SOUNDS> _sounds;

	std::list<size_t> _soundsLRU;
};
}

/** @} */
