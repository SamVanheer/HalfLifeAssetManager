#include <cstdint>
#include <cstring>
#include <algorithm>
#include <vector>

#include "AudioFile/AudioFile.h"

#include "shared/Logging.h"
#include "shared/Utility.h"

#include "lib/LibInterface.h"

#include "filesystem/IFileSystem.h"

#include "CSoundSystem.h"

namespace soundsystem
{
REGISTER_SINGLE_INTERFACE( ISOUNDSYSTEM_NAME, CSoundSystem );

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
	}
	while ((error = alGetError()) != AL_NONE);

	return true;
}

#define CheckALErrors() _CheckALErrors(__FILE__, __LINE__)

CSoundSystem::CSoundSystem()
{
}

CSoundSystem::~CSoundSystem()
{
}

bool CSoundSystem::Connect( const CreateInterfaceFn* const pFactories, const size_t uiNumFactories )
{
	for( size_t uiIndex = 0; uiIndex < uiNumFactories; ++uiIndex )
	{
		if( !m_pFileSystem )
		{
			m_pFileSystem = static_cast<filesystem::IFileSystem*>( pFactories[ uiIndex ]( IFILESYSTEM_NAME, nullptr ) );
		}
	}

	return m_pFileSystem != nullptr;
}

void CSoundSystem::Disconnect()
{
}

bool CSoundSystem::Initialize()
{
	m_Device = alcOpenDevice(nullptr);

	if (!m_Device)
	{
		return false;
	}

	m_Context = alcCreateContext(m_Device, nullptr);

	if (!m_Context)
	{
		return false;
	}

	alcMakeContextCurrent(m_Context);

	alGetError();

	return true;
}

void CSoundSystem::Shutdown()
{
	StopAllSounds();

	if (m_Context)
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(m_Context);
		m_Context = nullptr;
	}

	if (m_Device)
	{
		alcCloseDevice(m_Device);
		m_Device = nullptr;
	}
}

void CSoundSystem::RunFrame()
{
	size_t uiIndex = 0;

	ALint isPlaying;

	for( auto& sound : m_Sounds )
	{
		++uiIndex;

		if( !sound )
			continue;

		alGetSourcei(sound->source, AL_SOURCE_STATE, &isPlaying);

		if(isPlaying != AL_PLAYING)
		{
			sound.reset();
			m_SoundsLRU.erase( std::find( m_SoundsLRU.begin(), m_SoundsLRU.end(), uiIndex - 1 ) );
		}
	}
}

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

			byteIndex += sizeof(T);
		}
	}
}

void CSoundSystem::PlaySound( const char* pszFilename, float flVolume, int iPitch )
{
	if( !pszFilename || !( *pszFilename ) )
		return;

	if( !m_Context )
		return;

	char szActualFilename[ MAX_PATH_LENGTH ];

	if( pszFilename[ 0 ] == '*' )
		++pszFilename;

	const int iRet = snprintf( szActualFilename, sizeof( szActualFilename ), "sound/%s", pszFilename );

	if( iRet < 0 || static_cast<size_t>( iRet ) >= sizeof( szActualFilename ) )
		return;

	char szFullFilename[ MAX_PATH_LENGTH ];

	if( !m_pFileSystem->GetRelativePath( szActualFilename, szFullFilename, sizeof( szFullFilename ) ) )
	{
		Warning( "CSoundSystem::PlaySound: Unable to find sound file '%s'\n", pszFilename );
		return;
	}

	if (CheckALErrors())
	{
		return;
	}

	flVolume = clamp( flVolume, 0.0f, 1.0f );
	iPitch = clamp( iPitch, 0, 255 );

	AudioFile<double> file;

	if (!file.load(szFullFilename))
	{
		return;
	}

	if (file.getBitDepth() != 8 && file.getBitDepth() != 16)
	{
		return;
	}

	std::vector<std::uint8_t> data;

	data.resize(file.getNumChannels() * file.getNumSamplesPerChannel());

	switch (file.getBitDepth())
	{
	case 8:
		ConvertToAL<DataConverter8Bit>(file, data);
		break;

	case 16:
		ConvertToAL<DataConverter16Bit>(file, data);
		break;

	default: return;
	}

	const auto format = BufferFormat(file);

	auto sound = std::make_unique<Sound>();

	alBufferData(sound->buffer, format, data.data(), data.size(), file.getSampleRate());

	if (CheckALErrors())
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

	alSourcef(sound->source, AL_GAIN, flVolume);

	if (CheckALErrors())
	{
		return;
	}

	const auto pitchMultiplier = iPitch / (static_cast<float>(PITCH_NORM));

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

	m_Sounds[ uiIndex ] = std::move(sound);

	m_SoundsLRU.push_front( uiIndex );
}

void CSoundSystem::StopAllSounds()
{
	if( !m_Context )
		return;

	for( auto& sound : m_Sounds )
	{
		if( !sound )
			continue;

		sound.reset();
	}

	m_SoundsLRU.clear();
}

size_t CSoundSystem::GetSoundForPlayback()
{
	for( size_t uiIndex = 0; uiIndex < MAX_SOUNDS; ++uiIndex )
	{
		if( !m_Sounds[ uiIndex ] )
			return uiIndex;
	}

	//Shouldn't happen; LRU is only empty if no sounds are playing.
	if( m_SoundsLRU.empty() )
		return 0;

	//get from LRU.
	const size_t uiIndex = m_SoundsLRU.back();

	m_SoundsLRU.pop_back();

	auto& sound = m_Sounds[ uiIndex ];

	//Reset the sound data. Must be done after the above actions so it doesn't try to access null pointers.
	m_Sounds[ uiIndex ].reset();

	return uiIndex;
}
}
