#include <glm/geometric.hpp>

#include "shared/Logging.h"
#include "shared/CWorldTime.h"

#include "shared/studiomodel/CStudioModel.h"

#include "shared/renderer/studiomodel/IStudioModelRenderer.h"

#include "CStudioModelEntity.h"

bool CStudioModelEntity::Spawn()
{
	SetSequence( 0 );
	SetController( 0, 0.0f );
	SetController( 1, 0.0f );
	SetController( 2, 0.0f );
	SetController( 3, 0.0f );
	SetMouth( 0.0f );

	const studiohdr_t* pStudioHdr = m_pModel->GetStudioHeader();

	for( int n = 0; n < pStudioHdr->numbodyparts; ++n )
		SetBodygroup( n, 0 );

	SetSkin( 0 );

	return true;
}

void CStudioModelEntity::Draw( renderer::DrawFlags_t flags )
{
	studiomdl::CModelRenderInfo renderInfo;

	renderInfo.vecOrigin = GetOrigin();
	renderInfo.vecAngles = GetAngles();
	renderInfo.vecScale = GetScale();

	renderInfo.pModel = GetModel();

	renderInfo.flTransparency = GetTransparency();
	renderInfo.iSequence = GetSequence();
	renderInfo.flFrame = GetFrame();
	renderInfo.iBodygroup = GetBodygroup();
	renderInfo.iSkin = GetSkin();

	for( int iIndex = 0; iIndex < 2; ++iIndex )
	{
		renderInfo.iBlender[ iIndex ] = GetBlendingByIndex( iIndex );
	}

	for( int iIndex = 0; iIndex < 4; ++iIndex )
	{
		renderInfo.iController[ iIndex ] = GetControllerByIndex( iIndex );
	}

	renderInfo.iMouth = GetMouth();

	GetContext()->StudioModelRenderer->DrawModel( &renderInfo, flags );
}

float CStudioModelEntity::AdvanceFrame( float dt, const float flMax )
{
	if( !m_pModel )
		return 0.0;

	const studiohdr_t* pStudioHdr = m_pModel->GetStudioHeader();

	const mstudioseqdesc_t* pseqdesc = pStudioHdr->GetSequence( m_iSequence );

	if( dt == 0.0 )
	{
		dt = ( GetContext()->WorldTime->GetCurrentTime() - m_flAnimTime );
		if( dt <= 0.001 )
		{
			m_flAnimTime = GetContext()->WorldTime->GetCurrentTime();
			return 0.0;
		}
	}

	if( !m_flAnimTime )
		dt = 0.0;

	if( flMax != -1.f )
	{
		if( dt > flMax )
			dt = flMax;
	}

	const float flOldFrame = m_flFrame;

	bool shouldLoop;

	switch (m_LoopingMode)
	{
	case StudioLoopingMode::AlwaysLoop:
		shouldLoop = true;
		break;
	case StudioLoopingMode::NeverLoop:
		shouldLoop = false;
		break;
	case StudioLoopingMode::UseSequenceSetting:
		shouldLoop = (pseqdesc->flags & STUDIO_LOOPING) != 0;
		break;
	}

	const float flIncrement = dt * pseqdesc->fps * m_flFrameRate;

	if (m_flFrame < (pseqdesc->numframes - 1) || shouldLoop)
	{
		m_flFrame += flIncrement;
	}

	if( pseqdesc->numframes <= 1 )
	{
		m_flFrame = 0;
	}
	else
	{
		if (shouldLoop)
		{
			// wrap
			m_flFrame -= (int) (m_flFrame / (pseqdesc->numframes - 1)) * (pseqdesc->numframes - 1);
		}
		else if (m_flFrame >= (pseqdesc->numframes - 1))
		{
			//Clamp frame to the last valid frame index
			m_flFrame = static_cast<float>(pseqdesc->numframes - 1);
		}

		//Wrapped
		if( flOldFrame > m_flFrame )
		{
			m_flLastEventCheck = m_flFrame - flIncrement;
		}
	}

	m_flAnimTime = GetContext()->WorldTime->GetCurrentTime();

	return dt;
}

int CStudioModelEntity::GetAnimationEvent( CAnimEvent& event, float flStart, float flEnd, int index, const bool bAllowClientEvents )
{
	if( !m_pModel )
		return 0;

	const studiohdr_t* pStudioHdr = m_pModel->GetStudioHeader();

	if( m_iSequence >= pStudioHdr->numseq )
		return 0;

	int events = 0;

	const mstudioseqdesc_t* pseqdesc = pStudioHdr->GetSequence( m_iSequence );
	const mstudioevent_t* pevent = ( const mstudioevent_t * ) ( ( const byte * ) pStudioHdr + pseqdesc->eventindex );

	if( pseqdesc->numevents == 0 || index > pseqdesc->numevents )
		return 0;

	if( pseqdesc->numframes <= 1 )
	{
		flStart = 0;
		flEnd = 1.0;
	}

	for( ; index < pseqdesc->numevents; index++ )
	{
		//TODO: maybe leave it up to the listener to filter these out?
		if( !bAllowClientEvents )
		{
			// Don't send client-side events to the server AI
			if( pevent[ index ].event >= EVENT_CLIENT )
				continue;
		}

		if( ( pevent[ index ].frame >= flStart && pevent[ index ].frame < flEnd ) ||
			( ( pseqdesc->flags & STUDIO_LOOPING ) && flEnd >= pseqdesc->numframes - 1 && pevent[ index ].frame < flEnd - pseqdesc->numframes + 1 ) )
		{
			event.iEvent = pevent[ index ].event;
			event.pszOptions = pevent[ index ].options;
			return index + 1;
		}
	}
	return 0;
}

void CStudioModelEntity::DispatchAnimEvents( const bool bAllowClientEvents )
{
	if( !m_pModel )
	{
		Message( "Gibbed monster is thinking!\n" );
		return;
	}

	const studiohdr_t* pStudioHdr = m_pModel->GetStudioHeader();

	const mstudioseqdesc_t* pseqdesc = pStudioHdr->GetSequence( m_iSequence );

	//This is based on Source's DispatchAnimEvents. It fixes the bug where events don't get triggered, and get triggered multiple times due to the workaround.
	//Plays from previous frame to current. This differs from GoldSource in that GoldSource plays from current to predicted future frame.
	//This is more accurate, since it's based on actual frame data, rather than predicted frames, but results in events firing later than before.
	//The difference is ~0.1 seconds when running at 60 FPS. On dedicated servers the difference will be smaller if the tick rate is higher.
	float flStart = m_flLastEventCheck;
	float flEnd = m_flFrame;
	m_flLastEventCheck = m_flFrame;

	CAnimEvent event;

	int index = 0;

	while( ( index = GetAnimationEvent( event, flStart, flEnd, index, bAllowClientEvents ) ) != 0 )
	{
		HandleAnimEvent( event );
	}
}

void CStudioModelEntity::HandleAnimEvent( const CAnimEvent& event )
{
}

int CStudioModelEntity::SetFrame( const int iFrame )
{
	if( iFrame == -1 )
		return static_cast<int>( m_flFrame );

	if( !m_pModel )
		return 0;

	mstudioseqdesc_t* pseqdesc = m_pModel->GetStudioHeader()->GetSequence( m_iSequence );

	m_flFrame = static_cast<float>( iFrame );

	if( pseqdesc->numframes <= 1 )
	{
		m_flFrame = 0;
	}
	else
	{
		// wrap
		m_flFrame -= ( int ) ( m_flFrame / ( pseqdesc->numframes - 1 ) ) * ( pseqdesc->numframes - 1 );
	}

	m_flAnimTime = GetContext()->WorldTime->GetCurrentTime();

	return static_cast<int>( m_flFrame );
}

void CStudioModelEntity::SetModel( studiomdl::CStudioModel* pModel )
{
	//TODO: release old model.
	m_pModel = pModel;

	//TODO: reinit entity settings
}

int CStudioModelEntity::GetNumFrames() const
{
	const mstudioseqdesc_t* const pseqdesc = m_pModel->GetStudioHeader()->GetSequence( m_iSequence );

	return pseqdesc->numframes;
}

int CStudioModelEntity::SetSequence( const int iSequence )
{
	if( iSequence > m_pModel->GetStudioHeader()->numseq )
		return m_iSequence;

	m_iSequence = iSequence;
	m_flFrame = 0;
	m_flLastEventCheck = 0;

	return m_iSequence;
}

void CStudioModelEntity::GetSequenceInfo( float& flFrameRate, float& flGroundSpeed ) const
{
	const mstudioseqdesc_t* pseqdesc = m_pModel->GetStudioHeader()->GetSequence( m_iSequence );

	if( pseqdesc->numframes > 1 )
	{
		flFrameRate = pseqdesc->fps;
		flGroundSpeed = static_cast<float>( glm::length( pseqdesc->linearmovement ) );
		flGroundSpeed = flGroundSpeed * pseqdesc->fps / ( pseqdesc->numframes - 1 );
	}
	else
	{
		flFrameRate = 256.0;
		flGroundSpeed = 0.0;
	}
}

int CStudioModelEntity::GetBodyValueForGroup(int group) const
{
	if (!m_pModel)
	{
		return -1;
	}

	return m_pModel->GetBodyValueForGroup(m_iBodygroup, group);
}

int CStudioModelEntity::SetBodygroup( const int iBodygroup, const int iValue )
{
	if( !m_pModel )
		return 0;

	if( iBodygroup > m_pModel->GetStudioHeader()->numbodyparts )
		return -1;

	if( m_pModel->CalculateBodygroup( iBodygroup, iValue, m_iBodygroup ) )
		return iValue;

	return -1;
}

int CStudioModelEntity::SetSkin( const int iSkin )
{
	if( !m_pModel )
		return 0;

	if( iSkin < m_pModel->GetTextureHeader()->numskinfamilies )
	{
		m_iSkin = iSkin;
	}

	return m_iSkin;
}

byte CStudioModelEntity::GetControllerByIndex( const int iController ) const
{
	assert( iController >= 0 && iController < STUDIO_MAX_CONTROLLERS );

	return m_uiController[ iController ];
}

float CStudioModelEntity::GetControllerValue( const int iController ) const
{
	if( !m_pModel )
		return 0.0f;

	if( iController < 0 || iController >= STUDIO_TOTAL_CONTROLLERS )
		return 0;

	const studiohdr_t* pStudioHdr = m_pModel->GetStudioHeader();

	const mstudiobonecontroller_t* pbonecontroller = pStudioHdr->GetBoneControllers();

	// find first controller that matches the index
	int i;

	for( i = 0; i < pStudioHdr->numbonecontrollers; ++i, ++pbonecontroller )
	{
		if( pbonecontroller->index == iController )
			break;
	}

	if( i >= pStudioHdr->numbonecontrollers )
		return 0;

	byte uiValue;

	if( iController == STUDIO_MOUTH_CONTROLLER )
		uiValue = m_uiMouth;
	else
		uiValue = m_uiController[ iController ];

	return static_cast<float>( uiValue * ( 1.0 / 255.0 ) * ( pbonecontroller->end - pbonecontroller->start ) + pbonecontroller->start );
}

float CStudioModelEntity::SetController( const int iController, float flValue )
{
	if( !m_pModel )
		return 0.0f;

	const studiohdr_t* pStudioHdr = m_pModel->GetStudioHeader();

	const mstudiobonecontroller_t* pbonecontroller = pStudioHdr->GetBoneControllers();

	// find first controller that matches the index
	int i;

	for( i = 0; i < pStudioHdr->numbonecontrollers; ++i, ++pbonecontroller )
	{
		if( pbonecontroller->index == iController )
			break;
	}

	if( i >= pStudioHdr->numbonecontrollers )
		return flValue;

	// wrap 0..360 if it's a rotational controller
	if( pbonecontroller->type & ( STUDIO_XR | STUDIO_YR | STUDIO_ZR ) )
	{
		// ugly hack, invert value if end < start
		if( pbonecontroller->end < pbonecontroller->start )
			flValue = -flValue;

		// does the controller not wrap?
		if( pbonecontroller->start + 359.0 >= pbonecontroller->end )
		{
			if( flValue >( ( pbonecontroller->start + pbonecontroller->end ) / 2.0 ) + 180 )
				flValue = flValue - 360;
			if( flValue < ( ( pbonecontroller->start + pbonecontroller->end ) / 2.0 ) - 180 )
				flValue = flValue + 360;
		}
		else
		{
			if( flValue > 360 )
				flValue = static_cast<float>( flValue - ( int ) ( flValue / 360.0 ) * 360.0 );
			else if( flValue < 0 )
				flValue = static_cast<float>( flValue + ( int ) ( ( flValue / -360.0 ) + 1 ) * 360.0 );
		}
	}

	int setting = ( int ) ( 255 * ( flValue - pbonecontroller->start ) /
		( pbonecontroller->end - pbonecontroller->start ) );

	if( setting < 0 ) setting = 0;
	if( setting > 255 ) setting = 255;
	m_uiController[ iController ] = setting;

	return static_cast<float>( setting * ( 1.0 / 255.0 ) * ( pbonecontroller->end - pbonecontroller->start ) + pbonecontroller->start );
}

float CStudioModelEntity::SetMouth( float flValue )
{
	if( !m_pModel )
		return 0.0f;

	const studiohdr_t* pStudioHdr = m_pModel->GetStudioHeader();

	const mstudiobonecontroller_t* pbonecontroller = pStudioHdr->GetBoneControllers();

	// find first controller that matches the mouth
	for( int i = 0; i < pStudioHdr->numbonecontrollers; ++i, ++pbonecontroller )
	{
		if( pbonecontroller->index == STUDIO_MOUTH_CONTROLLER )
			break;
	}

	// wrap 0..360 if it's a rotational controller
	if( pbonecontroller->type & ( STUDIO_XR | STUDIO_YR | STUDIO_ZR ) )
	{
		// ugly hack, invert value if end < start
		if( pbonecontroller->end < pbonecontroller->start )
			flValue = -flValue;

		// does the controller not wrap?
		if( pbonecontroller->start + 359.0 >= pbonecontroller->end )
		{
			if( flValue >( ( pbonecontroller->start + pbonecontroller->end ) / 2.0 ) + 180 )
				flValue = flValue - 360;
			if( flValue < ( ( pbonecontroller->start + pbonecontroller->end ) / 2.0 ) - 180 )
				flValue = flValue + 360;
		}
		else
		{
			if( flValue > 360 )
				flValue = static_cast<float>( flValue - ( int ) ( flValue / 360.0 ) * 360.0 );
			else if( flValue < 0 )
				flValue = static_cast<float>( flValue + ( int ) ( ( flValue / -360.0 ) + 1 ) * 360.0 );
		}
	}

	int setting = ( int ) ( 64 * ( flValue - pbonecontroller->start ) / ( pbonecontroller->end - pbonecontroller->start ) );

	if( setting < 0 ) setting = 0;
	if( setting > 64 ) setting = 64;
	m_uiMouth = setting;

	return static_cast<float>( setting * ( 1.0 / 64.0 ) * ( pbonecontroller->end - pbonecontroller->start ) + pbonecontroller->start );
}

byte CStudioModelEntity::GetBlendingByIndex( const int iBlender ) const
{
	assert( iBlender >= 0 && iBlender < STUDIO_MAX_BLENDERS );

	return m_uiBlending[ iBlender ];
}

float CStudioModelEntity::GetBlendingValue( const int iBlender ) const
{
	if( !m_pModel )
		return 0.0f;

	if( iBlender < 0 || iBlender >= STUDIO_MAX_BLENDERS )
		return 0;

	const studiohdr_t* pStudioHdr = m_pModel->GetStudioHeader();

	const mstudioseqdesc_t* pseqdesc = pStudioHdr->GetSequence( m_iSequence );

	if( pseqdesc->blendtype[ iBlender ] == 0 )
		return 0;

	return static_cast<float>( m_uiBlending[ iBlender ] * ( 1.0 / 255.0 ) * ( pseqdesc->blendend[ iBlender ] - pseqdesc->blendstart[ iBlender ] ) + pseqdesc->blendstart[ iBlender ] );
}

float CStudioModelEntity::SetBlending( const int iBlender, float flValue )
{
	if( !m_pModel )
		return 0.0f;

	if( iBlender < 0 || iBlender >= STUDIO_MAX_BLENDERS )
		return 0;

	const studiohdr_t* pStudioHdr = m_pModel->GetStudioHeader();

	const mstudioseqdesc_t* pseqdesc = pStudioHdr->GetSequence( m_iSequence );

	if( pseqdesc->blendtype[ iBlender ] == 0 )
		return flValue;

	if( pseqdesc->blendtype[ iBlender ] & ( STUDIO_XR | STUDIO_YR | STUDIO_ZR ) )
	{
		// ugly hack, invert value if end < start
		if( pseqdesc->blendend[ iBlender ] < pseqdesc->blendstart[ iBlender ] )
			flValue = -flValue;

		// does the controller not wrap?
		if( pseqdesc->blendstart[ iBlender ] + 359.0 >= pseqdesc->blendend[ iBlender ] )
		{
			if( flValue > ( ( pseqdesc->blendstart[ iBlender ] + pseqdesc->blendend[ iBlender ] ) / 2.0 ) + 180 )
				flValue = flValue - 360;
			if( flValue < ( ( pseqdesc->blendstart[ iBlender ] + pseqdesc->blendend[ iBlender ] ) / 2.0 ) - 180 )
				flValue = flValue + 360;
		}
	}

	int setting = ( int ) ( 255 * ( flValue - pseqdesc->blendstart[ iBlender ] ) / ( pseqdesc->blendend[ iBlender ] - pseqdesc->blendstart[ iBlender ] ) );

	if( setting < 0 ) setting = 0;
	if( setting > 255 ) setting = 255;

	m_uiBlending[ iBlender ] = setting;

	return static_cast<float>( setting * ( 1.0 / 255.0 ) * ( pseqdesc->blendend[ iBlender ] - pseqdesc->blendstart[ iBlender ] ) + pseqdesc->blendstart[ iBlender ] );
}

void CStudioModelEntity::ExtractBbox( glm::vec3& vecMins, glm::vec3& vecMaxs ) const
{
	const mstudioseqdesc_t* pseqdesc = m_pModel->GetStudioHeader()->GetSequence( m_iSequence );

	vecMins = pseqdesc->bbmin;
	vecMaxs = pseqdesc->bbmax;
}

mstudiomodel_t* CStudioModelEntity::GetModelByBodyPart( const int iBodyPart ) const
{
	return m_pModel->GetModelByBodyPart( m_iBodygroup, iBodyPart );
}

CStudioModelEntity::MeshList_t CStudioModelEntity::ComputeMeshList( const int iTexture ) const
{
	const studiomdl::CStudioModel* pStudioModel = GetModel();

	assert( pStudioModel );

	MeshList_t meshes;

	const auto pStudioHdr = pStudioModel->GetStudioHeader();

	const auto textureHeader = pStudioModel->GetTextureHeader();

	const short* const pskinref = textureHeader->GetSkins();

	int iBodygroup = 0;

	for( int iBodyPart = 0; iBodyPart < pStudioHdr->numbodyparts; ++iBodyPart )
	{
		mstudiobodyparts_t *pbodypart = pStudioHdr->GetBodypart( iBodyPart );

		for( int iModel = 0; iModel < pbodypart->nummodels; ++iModel )
		{
			pStudioModel->CalculateBodygroup( iBodyPart, iModel, iBodygroup );

			mstudiomodel_t* pModel = pStudioModel->GetModelByBodyPart( iBodygroup, iBodyPart );

			for( int iMesh = 0; iMesh < pModel->nummesh; ++iMesh )
			{
				const mstudiomesh_t* pMesh = ((const mstudiomesh_t*) ((const byte*) pStudioModel->GetStudioHeader() + pModel->meshindex)) + iMesh;

				//Check each skin family to detect textures used only by alternate skins (e.g. scientist hands)
				for (int skinFamily = 0; skinFamily < textureHeader->numskinfamilies; ++skinFamily)
				{
					if (pskinref[(skinFamily * textureHeader->numskinref) + pMesh->skinref] == iTexture)
					{
						meshes.push_back(pMesh);
						break;
					}
				}
			}
		}
	}

	return meshes;
}