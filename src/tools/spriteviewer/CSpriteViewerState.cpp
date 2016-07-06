#include "game/entity/CBaseEntityList.h"

#include "game/entity/CSpriteEntity.h"

#include "CSpriteViewerState.h"

namespace sprview
{
CSpriteViewerState::CSpriteViewerState()
	: m_pEntity( nullptr )
{
	ResetToDefaults();
}

CSpriteViewerState::~CSpriteViewerState()
{
	ClearEntity();
}

void CSpriteViewerState::ResetModelData()
{
	ClearTypeOverride();
	ClearTexFormatOverride();
}

void CSpriteViewerState::ResetToDefaults()
{
	ResetModelData();

	showBackground = false;
}

void CSpriteViewerState::ClearEntity()
{
	SetEntity( nullptr );
}

void CSpriteViewerState::SetEntity( CSpriteEntity* pEntity )
{
	if( m_pEntity )
	{
		GetEntityList().Remove( m_pEntity );
		m_pEntity = nullptr;
	}

	if( pEntity )
		m_pEntity = pEntity;
}

bool CSpriteViewerState::DumpSpriteInfo( const char* const pszFilename )
{
	if( !pszFilename || !( *pszFilename ) )
		return false;

	if( !m_pEntity || !m_pEntity->GetSprite() )
		return false;

	if( FILE* pFile = fopen( pszFilename, "w" ) )
	{
		const auto pModel = m_pEntity->GetSprite();

		fclose( pFile );

		return true;
	}

	return false;
}
}