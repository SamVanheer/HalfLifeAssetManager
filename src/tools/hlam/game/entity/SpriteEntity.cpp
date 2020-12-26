#include "core/shared/WorldTime.hpp"

#include "engine/shared/sprite/sprite.hpp"
#include "engine/shared/sprite/CSprite.hpp"
#include "engine/shared/renderer/sprite/ISpriteRenderer.hpp"
#include "engine/shared/renderer/sprite/SpriteRenderInfo.hpp"

#include "game/entity/SpriteEntity.hpp"

LINK_ENTITY_TO_CLASS(sprite, SpriteEntity);

void SpriteEntity::OnDestroy()
{
	sprite::FreeSprite(_sprite);

	BaseClass::OnDestroy();
}

bool SpriteEntity::Spawn()
{
	SetThink(&ThisClass::AnimThink);

	SetFlags(entity::FL_ALWAYSTHINK);

	return true;
}

void SpriteEntity::Draw(renderer::DrawFlags flags)
{
	sprite::SpriteRenderInfo info;

	info.Origin = GetOrigin();
	info.Angles = GetAngles();
	info.Scale = GetScale();

	info.Sprite = GetSprite();

	info.Transparency = GetTransparency();

	info.Frame = GetFrame();

	GetContext()->SpriteRenderer->DrawSprite(&info, flags);
}

void SpriteEntity::AnimThink()
{
	_frame += GetContext()->Time->GetFrameTime() * 10;

	if (_frame >= _sprite->numframes)
	{
		_frame = 0;
	}
}

void SpriteEntity::SetSprite(sprite::msprite_t* pSprite)
{
	//TODO: release old sprite
	_sprite = pSprite;
}