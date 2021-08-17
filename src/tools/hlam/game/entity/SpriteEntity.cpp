#include "core/shared/WorldTime.hpp"

#include "engine/shared/sprite/Sprite.hpp"
#include "engine/shared/sprite/SpriteFileFormat.hpp"
#include "engine/shared/renderer/sprite/ISpriteRenderer.hpp"
#include "engine/shared/renderer/sprite/SpriteRenderInfo.hpp"

#include "game/entity/SpriteEntity.hpp"

SpriteEntity::~SpriteEntity()
{
	sprite::FreeSprite(_sprite);
}

void SpriteEntity::Spawn()
{
	SetThink(&ThisClass::AnimThink);

	SetFlags(entity::FL_ALWAYSTHINK);
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

	GetContext()->SpriteRenderer->DrawSprite(info, flags);
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