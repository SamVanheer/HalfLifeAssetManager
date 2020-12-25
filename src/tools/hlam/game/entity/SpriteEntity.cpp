#include "shared/WorldTime.hpp"

#include "engine/shared/sprite/sprite.hpp"
#include "engine/shared/sprite/CSprite.hpp"
#include "engine/shared/renderer/sprite/ISpriteRenderer.hpp"
#include "engine/shared/renderer/sprite/CSpriteRenderInfo.hpp"

#include "game/entity/SpriteEntity.hpp"

LINK_ENTITY_TO_CLASS( sprite, SpriteEntity);

void SpriteEntity::OnDestroy()
{
	sprite::FreeSprite( m_pSprite );

	BaseClass::OnDestroy();
}

bool SpriteEntity::Spawn()
{
	SetThink( &ThisClass::AnimThink );

	SetFlags( entity::FL_ALWAYSTHINK );

	return true;
}

void SpriteEntity::Draw( renderer::DrawFlags_t flags )
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

void SpriteEntity::AnimThink()
{
	m_flFrame += GetContext()->Time->GetFrameTime() * 10;

	if( m_flFrame >= m_pSprite->numframes )
		m_flFrame = 0;
}

void SpriteEntity::SetSprite( sprite::msprite_t* pSprite )
{
	//TODO: release old sprite
	m_pSprite = pSprite;
}