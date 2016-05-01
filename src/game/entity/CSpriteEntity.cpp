#include "shared/CWorldTime.h"

#include "engine/shared/sprite/sprite.h"
#include "engine/shared/sprite/CSprite.h"
#include "engine/shared/sprite/CSpriteRenderer.h"

#include "CSpriteEntity.h"

LINK_ENTITY_TO_CLASS( sprite, CSpriteEntity );

void CSpriteEntity::OnDestroy()
{
	sprite::FreeSprite( m_pSprite );

	BaseClass::OnDestroy();
}

bool CSpriteEntity::Spawn()
{
	SetThink( &ThisClass::AnimThink );

	SetFlags( entity::FL_ALWAYSTHINK );

	return true;
}

void CSpriteEntity::Draw( entity::DrawFlags_t flags )
{
	sprite::DrawFlags_t drawFlags = sprite::DRAWF_NONE;

	//TODO: this needs to be redesigned
	if( flags & entity::DRAWF_WIREFRAME_ONLY )
		drawFlags |= sprite::DRAWF_WIREFRAME_OVERLAY | sprite::DRAWF_NODRAW;

	sprite::Renderer().DrawSprite( this, drawFlags );
}

void CSpriteEntity::AnimThink()
{
	m_flFrame += WorldTime.GetFrameTime() * 10;

	if( m_flFrame >= m_pSprite->numframes )
		m_flFrame = 0;
}

void CSpriteEntity::SetSprite( sprite::msprite_t* pSprite )
{
	//TODO: release old sprite
	m_pSprite = pSprite;
}