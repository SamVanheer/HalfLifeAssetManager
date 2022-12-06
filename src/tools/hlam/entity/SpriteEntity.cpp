#include "engine/shared/sprite/Sprite.hpp"
#include "engine/shared/sprite/SpriteFileFormat.hpp"
#include "engine/shared/renderer/sprite/ISpriteRenderer.hpp"
#include "engine/shared/renderer/sprite/SpriteRenderInfo.hpp"

#include "entity/SpriteEntity.hpp"

#include "utility/WorldTime.hpp"

SpriteEntity::~SpriteEntity()
{
	// TODO: reimplement at some point (sprites aren't actually used right now)
	//sprite::FreeSprite(_sprite);
}

void SpriteEntity::Spawn()
{
	SetThink(&ThisClass::AnimThink);

	SetFlags(entity::FL_ALWAYSTHINK);
}

void SpriteEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, RenderPasses renderPass)
{
	sprite::SpriteRenderInfo info;

	info.Origin = GetOrigin();
	info.Angles = GetAngles();
	info.Scale = GetScale();

	info.Sprite = GetSprite();

	info.Transparency = GetTransparency();

	info.Frame = GetFrame();

	GetContext()->SpriteRenderer->DrawSprite(info, renderer::DrawFlag::NONE);
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