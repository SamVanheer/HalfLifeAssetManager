#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <vector>

#include "AudioFile/AudioFile.h"

#include "vorbis/vorbisfile.h"

#include "filesystem/IFileSystem.hpp"

#include "shared/Logging.hpp"

#include "soundsystem/SoundSystem.hpp"

namespace soundsystem
{
bool _CheckALErrors(const char* file, int line)
{
	auto error = alGetError();

	if (error == AL_NONE)
	{
		return false;
	}

	do
	{
		Error("Error 0x%X (%d) while calling OpenAL API in file %s, line %d\n", static_cast<unsigned int>(error), error, file, line);
	} while ((error = alGetError()) != AL_NONE);

	return true;
}

#define CheckALErrors() _CheckALErrors(__FILE__, __LINE__)

static ALenum BufferFormat(const AudioFile<double>& file)
{
	switch (file.getBitDepth())
	{
	case 8: return file.isStereo() ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
	case 16: return file.isStereo() ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
	default: return AL_INVALID;
	}
}

struct DataConverter8Bit
{
	using Type = std::int8_t;

	static Type Convert(double value)
	{
		value = (value + 1.) / 2.;
		return static_cast<uint8_t> (value * 255.);
	}
};

struct DataConverter16Bit
{
	using Type = std::int16_t;

	static Type Convert(double value)
	{
		value = std::clamp(value, -1., 1.);
		return static_cast<int16_t> (value * 32767.);
	}
};

template<typename T>
void ConvertToAL(const AudioFile<double>& file, std::vector<std::uint8_t>& data)
{
	std::size_t byteIndex = 0;

	for (int i = 0; i < file.getNumSamplesPerChannel(); ++i)
	{
		for (int channel = 0; channel < file.getNumChannels(); ++channel)
		{
			auto& dest = *reinterpret_cast<T::Type*>(&data[byteIndex]);

			auto value = file.samples[channel][i];

			dest = T::Convert(value);

			byteIndex += sizeof(T::Type);
		}
	}
}

std::unique_ptr<SoundSystem::Sound> TryLoadWaveFile(const std::string& fileName)
{
	AudioFile<double> file;

	if (!file.load(fileName))
	{
		return {};
	}

	if (file.getBitDepth() != 8 && file.getBitDepth() != 16)
	{
		return {};
	}

	std::vector<std::uint8_t> data;

	data.resize(file.getNumChannels() * file.getNumSamplesPerChannel() * (file.getBitDepth() / 8));

	switch (file.getBitDepth())
	{
	case 8:
		ConvertToAL<DataConverter8Bit>(file, data);
		break;

	case 16:
		ConvertToAL<DataConverter16Bit>(file, data);
		break;

	default: return {};
	}

	const auto format = BufferFormat(file);

	auto sound = std::make_unique<SoundSystem::Sound>();

	alBufferData(sound->buffer, format, data.data(), data.size(), file.getSampleRate());

	if (CheckALErrors())
	{
		return {};
	}

	return sound;
}

struct OggVorbisCleanup
{
	void operator()(OggVorbis_File* pointer) const
	{
		ov_clear(pointer);
	}
};

std::unique_ptr<SoundSystem::Sound> TryLoadOggVorbis(const std::string& fileName)
{
	OggVorbis_File vorbisData{};

	auto result = ov_fopen(fileName.c_str(), &vorbisData);

	if (result)
	{
		return {};
	}

	const std::unique_ptr<OggVorbis_File, OggVorbisCleanup> cleanup(&vorbisData);

	const auto info = ov_info(&vorbisData, -1);

	const ALenum format = info->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

	const auto pcmTotal = ov_pcm_total(&vorbisData, -1);

	const ogg_int64_t sizeInBytes = pcmTotal * info->channels * 2;

	std::vector<std::uint8_t> data;

	if (sizeInBytes > data.max_size())
	{
		Error("CSoundSystem::TryLoadOggVorbis: File \"%s\" is too large to read (%dll > %u)\n", fileName.c_str(), sizeInBytes, data.max_size());
		return {};
	}

	data.resize(static_cast<std::size_t>(sizeInBytes));

	long size = 0;
	int bitStream = 0;

	for (std::size_t offset = 0;
		(size = ov_read(&vorbisData, (char*)data.data() + offset, 4096, 0, 2, 1, &bitStream)) > 0;
		offset += size)
	{
	}

	//An error occurred while reading
	if (size < 0)
	{
		Error("CSoundSystem::TryLoadOggVorbis: Error while reading file \"%s\" (%dl)\n", fileName.c_str(), size);
		return {};
	}

	auto sound = std::make_unique<SoundSystem::Sound>();

	alBufferData(sound->buffer, format, data.data(), data.size(), info->rate);

	if (CheckALErrors())
	{
		return {};
	}

	return sound;
}

SoundSystem::SoundSystem() = default;
SoundSystem::~SoundSystem() = default;

bool SoundSystem::Initialize(filesystem::IFileSystem* filesystem)
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
		Warning("CSoundSystem::PlaySound: Unable to find sound file '%s'\n", actualFileName.c_str());
		return;
	}

	if (CheckALErrors())
	{
		return;
	}

	volume = std::clamp(volume, 0.0f, 1.0f);
	pitch = std::clamp(pitch, 0, 255);

	std::unique_ptr<Sound> sound = TryLoadWaveFile(fullFileName);

	if (!sound)
	{
		sound = TryLoadOggVorbis(fullFileName);
	}

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
}
