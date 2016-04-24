#include "soundsystem/CSoundSystem.h"

#include "cvar/CCVar.h"

#include "../CHLMVState.h"

#include "CHLMVStudioModelEntity.h"

static cvar::CCVar s_ent_playsounds( "s_ent_playsounds", cvar::CCVarArgsBuilder().FloatValue( 0 ).HelpInfo( "Whether or not to play sounds triggered by animation events" ) );

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
				soundSystem().PlaySound( event.pszOptions, 1.0f, soundsystem::PITCH_NORM );
			}

			break;
		}

	default: break;
	}
}

void CHLMVStudioModelEntity::AnimThink()
{
	if( !m_pState->playSequence )
		return;

	const float flTime = AdvanceFrame( 0.0f, 0.1f );

	DispatchAnimEvents( true );
}