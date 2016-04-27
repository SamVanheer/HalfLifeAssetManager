#include "fmod.hpp"
#include "fmod_errors.h"

#include <cstring>
#include <algorithm>

#include "shared/Logging.h"
#include "shared/Utility.h"

#include "filesystem/CFileSystem.h"

#include "CSoundSystem.h"

namespace soundsystem
{
/**
*	Checks the result of an FMOD operation. If the result is not ok, returns true.
*	@param result Result to check
*	@return true if the result is not ok
*/
bool CheckFMODResult( const FMOD_RESULT result )
{
	if( result == FMOD_OK )
		return false;

	const char* pszError = FMOD_ErrorString( result );

	Error( "FMOD Error: %s (error code: %d)\n", static_cast<const char*>( pszError ), static_cast<int>( result ) );

	return true;
}

CSoundSystem* CSoundSystem::m_pInstance = nullptr;

CSoundSystem& CSoundSystem::CreateInstance()
{
	if( m_pInstance )
	{
		Warning( "CSoundSystem::CreateInstance called multiple times!\n" );
		return *m_pInstance;
	}

	m_pInstance = new CSoundSystem();

	return *m_pInstance;
}

void CSoundSystem::DestroyInstance()
{
	if( m_pInstance )
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}

bool CSoundSystem::InstanceExists()
{
	return m_pInstance != nullptr;
}

CSoundSystem& CSoundSystem::GetInstance()
{
	return *m_pInstance;
}

CSoundSystem::CSoundSystem()
{
	memset( m_Sounds, 0, sizeof( m_Sounds ) );
}

CSoundSystem::~CSoundSystem()
{
}

bool CSoundSystem::Initialize()
{
	FMOD_RESULT result;

	/*
	Create a System object and initialize.
	*/
	result = FMOD::System_Create( &m_pSystem );

	if( CheckFMODResult( result ) )
		return false;

	{
		unsigned int uiVersion;

		result = m_pSystem->getVersion( &uiVersion );

		if( CheckFMODResult( result ) )
			return false;

		if( uiVersion < FMOD_VERSION )
		{
			Error( "Error! You are using an old version of FMOD %08x. This program requires %08x\n",
				   uiVersion, FMOD_VERSION );
			return false;
		}
	}

	{
		int numdrivers;

		result = m_pSystem->getNumDrivers( &numdrivers );

		if( CheckFMODResult( result ) )
			return false;

		if( numdrivers == 0 )
		{
			result = m_pSystem->setOutput( FMOD_OUTPUTTYPE_NOSOUND );

			if( CheckFMODResult( result ) )
				return false;
		}
		else
		{
			{
				FMOD_CAPS caps;
				FMOD_SPEAKERMODE speakerMode;

				result = m_pSystem->getDriverCaps( 0, &caps, nullptr, &speakerMode );

				if( CheckFMODResult( result ) )
					return false;

				/*
				Set the user selected speaker mode.
				*/
				result = m_pSystem->setSpeakerMode( speakerMode );

				if( CheckFMODResult( result ) )
					return false;

				if( caps & FMOD_CAPS_HARDWARE_EMULATED )
				{
					/*
					The user has the 'Acceleration' slider set to off! This is really bad
					for latency! You might want to warn the user about this.
					*/
					result = m_pSystem->setDSPBufferSize( 1024, 10 );

					if( CheckFMODResult( result ) )
						return false;
				}
			}

			{
				char szDriverName[ 256 ];

				result = m_pSystem->getDriverInfo( 0, szDriverName, sizeof( szDriverName ), 0 );
		
				if( CheckFMODResult( result ) )
					return false;

				if( strstr( szDriverName, "SigmaTel" ) )
				{
					/*
					Sigmatel sound devices crackle for some reason if the format is PCM 16bit.
					PCM floating point output seems to solve it.
					*/
					result = m_pSystem->setSoftwareFormat( 48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0, 0,
														FMOD_DSP_RESAMPLER_LINEAR );
					if( CheckFMODResult( result ) )
						return false;
				}
			}
		}
	}

	result = m_pSystem->init( 100, FMOD_INIT_NORMAL, 0 );

	if( result == FMOD_ERR_OUTPUT_CREATEBUFFER )
	{
		/*
		Ok, the speaker mode selected isn't supported by this soundcard. Switch it
		back to stereo...
		*/
		result = m_pSystem->setSpeakerMode( FMOD_SPEAKERMODE_STEREO );

		if( CheckFMODResult( result ) )
			return false;
		/*
		... and re-init.
		*/
		result = m_pSystem->init( 100, FMOD_INIT_NORMAL, 0 );
	}

	if( CheckFMODResult( result ) )
		return false;

	return true;
}

void CSoundSystem::Shutdown()
{
	StopAllSounds();

	FMOD_RESULT result;

	if( m_pSystem )
	{
		result = m_pSystem->release();
		m_pSystem = nullptr;

		CheckFMODResult( result );
	}
}

void CSoundSystem::RunFrame()
{
	m_pSystem->update();

	size_t uiIndex = 0;

	FMOD_RESULT result;

	bool bIsPlaying;

	for( auto& sound : m_Sounds )
	{
		++uiIndex;

		if( !sound.pSound )
			continue;

		result = sound.pChannel->isPlaying( &bIsPlaying );

		if( result == FMOD_ERR_INVALID_HANDLE || result == FMOD_ERR_CHANNEL_STOLEN || !bIsPlaying || CheckFMODResult( result ) )
		{
			CheckFMODResult( sound.pSound->release() );
			sound = Sound_t{};
			m_SoundsLRU.erase( std::find( m_SoundsLRU.begin(), m_SoundsLRU.end(), uiIndex - 1 ) );
		}
	}
}

void CSoundSystem::PlaySound( const char* pszFilename, float flVolume, int iPitch )
{
	if( !pszFilename || !( *pszFilename ) )
		return;

	if( !m_pSystem )
		return;

	char szActualFilename[ MAX_PATH_LENGTH ];

	if( pszFilename[ 0 ] == '*' )
		++pszFilename;

	const int iRet = snprintf( szActualFilename, sizeof( szActualFilename ), "sound/%s", pszFilename );

	if( iRet < 0 || static_cast<size_t>( iRet ) >= sizeof( szActualFilename ) )
		return;

	char szFullFilename[ MAX_PATH_LENGTH ];

	if( !fileSystem().GetRelativePath( szActualFilename, szFullFilename, sizeof( szFullFilename ) ) )
	{
		Warning( "CSoundSystem::PlaySound: Unable to find sound file '%s'\n", pszFilename );
		return;
	}

	flVolume = clamp( flVolume, 0.0f, 1.0f );
	iPitch = clamp( iPitch, 0, 255 );

	const size_t uiIndex = GetSoundForPlayback();

	Sound_t sound{};

	FMOD_RESULT result = m_pSystem->createSound( szFullFilename, FMOD_HARDWARE, nullptr, &sound.pSound );

	if( result == FMOD_ERR_FILE_NOTFOUND )
	{
		return;
	}

	if( CheckFMODResult( result ) )
		return;

	if( CheckFMODResult( m_pSystem->playSound( FMOD_CHANNEL_FREE, sound.pSound, false, &sound.pChannel ) ) )
	{
		CheckFMODResult( sound.pSound->release() );
		return;
	}

	if( CheckFMODResult( sound.pChannel->setVolume( flVolume ) ) )
	{
		CheckFMODResult( sound.pSound->release() );
		return;
	}

	float flFrequency;

	if( CheckFMODResult( sound.pChannel->getFrequency( &flFrequency ) ) )
	{
		CheckFMODResult( sound.pSound->release() );
		return;
	}

	if( CheckFMODResult( sound.pChannel->setFrequency( flFrequency * ( iPitch / ( static_cast<float>( PITCH_NORM ) ) ) ) ) )
	{
		CheckFMODResult( sound.pSound->release() );
		return;
	}

	m_Sounds[ uiIndex ] = sound;

	m_SoundsLRU.push_front( uiIndex );
}

void CSoundSystem::StopAllSounds()
{
	if( !m_pSystem )
		return;

	for( auto& sound : m_Sounds )
	{
		if( !sound.pSound )
			continue;

		CheckFMODResult( sound.pChannel->stop() );

		CheckFMODResult( sound.pSound->release() );

		sound = Sound_t{};
	}

	m_SoundsLRU.clear();
}

size_t CSoundSystem::GetSoundForPlayback()
{
	for( size_t uiIndex = 0; uiIndex < MAX_SOUNDS; ++uiIndex )
	{
		if( !m_Sounds[ uiIndex ].pSound )
			return uiIndex;
	}

	//Shouldn't happen; LRU is only empty if no sounds are playing.
	if( m_SoundsLRU.empty() )
		return 0;

	//get from LRU.
	const size_t uiIndex = m_SoundsLRU.back();

	m_SoundsLRU.pop_back();

	Sound_t& sound = m_Sounds[ uiIndex ];

	m_Sounds[ uiIndex ] = Sound_t{};

	CheckFMODResult( sound.pChannel->stop() );
	
	CheckFMODResult( sound.pSound->release() );

	return uiIndex;
}
}