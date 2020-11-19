#include "soundsystem/SoundConstants.h"
#include "soundsystem/ISoundSystem.h"

#include "cvar/CCVar.h"

#include "CHLMVStudioModelEntity.h"

//TODO: remove
extern soundsystem::ISoundSystem* g_pSoundSystem;

static cvar::CCVar s_ent_playsounds( "s_ent_playsounds", cvar::CCVarArgsBuilder().FloatValue( 0 ).HelpInfo( "Whether or not to play sounds triggered by animation events" ) );
static cvar::CCVar s_ent_pitchframerate( "s_ent_pitchframerate", cvar::CCVarArgsBuilder().FloatValue( 0 ).HelpInfo( "If non-zero, event sounds are pitch modulated based on the framerate" ) );

LINK_ENTITY_TO_CLASS( studiomodel, CHLMVStudioModelEntity );

void CHLMVStudioModelEntity::OnCreate()
{
	BaseClass::OnCreate();

	SetThink( &ThisClass::AnimThink );

	//Always think for smooth animations
	SetFlags( entity::FL_ALWAYSTHINK );
}

bool CHLMVStudioModelEntity::Spawn()
{
	SetSequence( 0 );
	SetController( 0, 0.0f );
	SetController( 1, 0.0f );
	SetController( 2, 0.0f );
	SetController( 3, 0.0f );
	SetMouth( 0.0f );

	const studiohdr_t* pStudioHdr = GetModel()->GetStudioHeader();

	for( int n = 0; n < pStudioHdr->numbodyparts; ++n )
		SetBodygroup( n, 0 );

	SetSkin( 0 );

	return true;
}

void CHLMVStudioModelEntity::HandleAnimEvent( const CAnimEvent& event )
{
	//TODO: move to subclass.
	switch( event.iEvent )
	{
	case SCRIPT_EVENT_SOUND:			// Play a named wave file
	case SCRIPT_EVENT_SOUND_VOICE:
	case SCRIPT_CLIENT_EVENT_SOUND:
		{
			if( s_ent_playsounds.GetBool() )
			{
				int iPitch = soundsystem::PITCH_NORM;
				
				if( s_ent_pitchframerate.GetBool() )
				{
					iPitch = static_cast<int>( iPitch * GetFrameRate() );
				}

				g_pSoundSystem->PlaySound( event.pszOptions, soundsystem::VOLUME_NORM, iPitch );
			}

			break;
		}

	default: break;
	}
}

void CHLMVStudioModelEntity::AnimThink()
{
	//TODO: replace
#if false
	if( !m_pState->playSequence )
		return;
#endif

	const float flTime = AdvanceFrame( 0.0f, 0.1f );

	DispatchAnimEvents( true );
}