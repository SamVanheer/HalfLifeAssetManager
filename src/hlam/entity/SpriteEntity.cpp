#include "formats/sprite/ISpriteRenderer.hpp"
#include "formats/sprite/Sprite.hpp"
#include "formats/sprite/SpriteFileFormat.hpp"
#include "formats/sprite/SpriteRenderInfo.hpp"

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

	AlwaysThink = true;
}

void SpriteEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, graphics::SceneContext& sc, RenderPasses renderPass)
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