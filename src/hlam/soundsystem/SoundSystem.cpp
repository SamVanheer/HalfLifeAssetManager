#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <vector>

#include <spdlog/spdlog.h>

#include <AL/alext.h>

#include <libnyquist/Decoders.h>

#include "filesystem/IFileSystem.hpp"

#include "soundsystem/SoundSystem.hpp"

bool SoundSystem::CheckALErrorsCore(const char* file, int line)
{
	auto error = alGetError();

	if (error == AL_NONE)
	{
		return false;
	}

	do
	{
		SPDLOG_LOGGER_CALL(_logger, spdlog::level::err, "Error {:x} ({}) while calling OpenAL API in file {}, line {}", static_cast<unsigned int>(error), error, file, line);
	} while ((error = alGetError()) != AL_NONE);

	return true;
}

#define CheckALErrors() CheckALErrorsCore(__FILE__, __LINE__)

SoundSystem::SoundSystem(const std::shared_ptr<spdlog::logger>& logger)
	: _logger(logger)
	, m_Loader(std::make_unique<nqr::NyquistIO>())
{
}

SoundSystem::~SoundSystem() = default;

bool SoundSystem::Initialize(IFileSystem* filesystem)
{
	_fileSystem = filesystem;

	if (!_fileSystem)
	{
		return false;
	}

	_device = alcOpenDevice(nullptr);

	if (nullptr != _device)
	{
		_context = alcCreateContext(_device, nullptr);

		if (!_context)
		{
			return false;
		}

		alcMakeContextCurrent(_context);

		CheckALErrors();
	}

	return true;
}

void SoundSystem::Shutdown()
{
	StopAllSounds();

	if (_context)
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(_context);
		_context = nullptr;
	}

	if (_device)
	{
		alcCloseDevice(_device);
		_device = nullptr;
	}
}

void SoundSystem::RunFrame()
{
	size_t uiIndex = 0;

	ALint isPlaying;

	for (auto& sound : _sounds)
	{
		++uiIndex;

		if (!sound)
		{
			continue;
		}

		alGetSourcei(sound->source, AL_SOURCE_STATE, &isPlaying);

		if (isPlaying != AL_PLAYING)
		{
			sound.reset();
			_soundsLRU.erase(std::find(_soundsLRU.begin(), _soundsLRU.end(), uiIndex - 1));
		}
	}
}

void SoundSystem::PlaySound(std::string_view fileName, float volume, int pitch)
{
	if (fileName.empty())
	{
		return;
	}

	if (!_context)
	{
		return;
	}

	if (fileName[0] == '*')
	{
		fileName = fileName.substr(1);
	}

	std::ostringstream stream;

	stream << "sound/" << fileName;

	const auto actualFileName{stream.str()};

	const auto fullFileName{_fileSystem->GetRelativePath(actualFileName)};

	if (fullFileName.empty())
	{
		SPDLOG_LOGGER_CALL(_logger, spdlog::level::warn, "Unable to find sound file '{}'", actualFileName);
		return;
	}

	if (CheckALErrors())
	{
		return;
	}

	volume = std::clamp(volume, 0.0f, 1.0f);
	pitch = std::clamp(pitch, 0, 255);

	std::unique_ptr<Sound> sound = TryLoadFile(fullFileName);

	if (!sound)
	{
		return;
	}

	alSourcei(sound->source, AL_BUFFER, sound->buffer);

	if (CheckALErrors())
	{
		return;
	}

	alSourcePlay(sound->source);

	if (CheckALErrors())
	{
		return;
	}

	alSourcef(sound->source, AL_GAIN, volume);

	if (CheckALErrors())
	{
		return;
	}

	const auto pitchMultiplier = pitch / (static_cast<float>(PITCH_NORM));

	if (CheckALErrors())
	{
		return;
	}

	alSourcef(sound->source, AL_PITCH, pitchMultiplier);

	if (CheckALErrors())
	{
		return;
	}

	const size_t uiIndex = GetSoundForPlayback();

	_sounds[uiIndex] = std::move(sound);

	_soundsLRU.push_front(uiIndex);
}

void SoundSystem::StopAllSounds()
{
	if (!_context)
	{
		return;
	}

	for (auto& sound : _sounds)
	{
		if (!sound)
		{
			continue;
		}

		sound.reset();
	}

	_soundsLRU.clear();
}

size_t SoundSystem::GetSoundForPlayback()
{
	for (size_t uiIndex = 0; uiIndex < MAX_SOUNDS; ++uiIndex)
	{
		if (!_sounds[uiIndex])
		{
			return uiIndex;
		}
	}

	//Shouldn't happen; LRU is only empty if no sounds are playing.
	if (_soundsLRU.empty())
	{
		return 0;
	}

	//get from LRU.
	const size_t uiIndex = _soundsLRU.back();

	_soundsLRU.pop_back();

	auto& sound = _sounds[uiIndex];

	//Reset the sound data. Must be done after the above actions so it doesn't try to access null pointers.
	_sounds[uiIndex].reset();

	return uiIndex;
}

std::unique_ptr<SoundSystem::Sound> SoundSystem::TryLoadFile(const std::string& fileName)
{
	std::unique_ptr<FILE, decltype(std::fclose)*> file{std::fopen(fileName.c_str(), "rb"), &std::fclose};

	if (!file)
	{
		return {};
	}

	std::fseek(file.get(), 0, SEEK_END);

	const auto size = std::ftell(file.get());

	std::fseek(file.get(), 0, SEEK_SET);

	std::vector<std::uint8_t> buffer;

	buffer.resize(size);

	if (std::fread(buffer.data(), 1, size, file.get()) != size)
	{
		SPDLOG_LOGGER_CALL(_logger, spdlog::level::err, "Error while reading file \"{}\" ({})\n", fileName, size);
		return {};
	}

	nqr::AudioData audioData;

	m_Loader->Load(&audioData, buffer);

	if (audioData.channelCount != 1 && audioData.channelCount != 2)
	{
		return {};
	}

	const auto format = [&]()
	{
		switch (audioData.channelCount)
		{
		case 1: return AL_FORMAT_MONO_FLOAT32;
		case 2: return AL_FORMAT_STEREO_FLOAT32;
		default: return AL_INVALID;
		}
	}();

	auto sound = std::make_unique<SoundSystem::Sound>();

	alBufferData(sound->buffer, format, audioData.samples.data(), audioData.samples.size() * sizeof(float), audioData.sampleRate);

	if (CheckALErrors())
	{
		return {};
	}

	return sound;
}
