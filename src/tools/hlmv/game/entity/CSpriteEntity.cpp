#include "shared/CWorldTime.h"

#include "engine/shared/sprite/sprite.h"
#include "engine/shared/sprite/CSprite.h"
#include "engine/shared/renderer/sprite/ISpriteRenderer.h"
#include "engine/shared/renderer/sprite/CSpriteRenderInfo.h"

#include "CSpriteEntity.h"

//TODO: remove
extern sprite::ISpriteRenderer* g_pSpriteRenderer;

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

void CSpriteEntity::Draw( renderer::DrawFlags_t flags )
{
	sprite::CSpriteRenderInfo info;

	info.vecOrigin = GetOrigin();
	info.vecAngles = GetAngles();
	info.vecScale = GetScale();

	info.pSprite = GetSprite();

	info.flTransparency = GetTransparency();

	info.flFrame = GetFrame();

	g_pSpriteRenderer->DrawSprite( &info, flags );
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