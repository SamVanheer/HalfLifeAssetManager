#include "shared/WorldTime.hpp"

#include "engine/shared/sprite/sprite.hpp"
#include "engine/shared/sprite/CSprite.hpp"
#include "engine/shared/renderer/sprite/ISpriteRenderer.hpp"
#include "engine/shared/renderer/sprite/CSpriteRenderInfo.hpp"

#include "CSpriteEntity.hpp"

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

	GetContext()->SpriteRenderer->DrawSprite( &info, flags );
}

void CSpriteEntity::AnimThink()
{
	m_flFrame += GetContext()->Time->GetFrameTime() * 10;

	if( m_flFrame >= m_pSprite->numframes )
		m_flFrame = 0;
}

void CSpriteEntity::SetSprite( sprite::msprite_t* pSprite )
{
	//TODO: release old sprite
	m_pSprite = pSprite;
}