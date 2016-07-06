#ifndef SPRITEVIEWER_CSPRITEVIEWERSTATE_H
#define SPRITEVIEWER_CSPRITEVIEWERSTATE_H

#include <vector>
#include <memory>

#include <glm/vec3.hpp>

#include "shared/Platform.h"
#include "graphics/OpenGL.h"
#include "graphics/Constants.h"

#include "engine/shared/sprite/sprite.h"

class CSpriteEntity;

/*
*	Contains all state used by the sprite viewer application.
*/
namespace sprview
{
class CSpriteViewerState final
{
public:
	CSpriteViewerState();
	~CSpriteViewerState();

	void ResetModelData();

	void ResetToDefaults();

	void ClearEntity();

	CSpriteEntity* GetEntity() { return m_pEntity; }

	void SetEntity( CSpriteEntity* pEntity );

	sprite::Type::Type GetTypeOverride() const { return m_TypeOverride; }

	bool IsTypeOverridden() const { return m_bTypeOverride; }

	void SetTypeOverride( const sprite::Type::Type type )
	{
		m_TypeOverride = type;
		m_bTypeOverride = true;
	}

	void ClearTypeOverride()
	{
		m_TypeOverride = sprite::Type::VP_PARALLEL_UPRIGHT;
		m_bTypeOverride = false;
	}

	sprite::TexFormat::TexFormat GetTexFormatOverride() const { return m_TexFormatOverride; }

	bool IsTexFormatOverridden() const { return m_bTexFormatOverride; }

	void SetTexFormatOverride( const sprite::TexFormat::TexFormat texFormat )
	{
		m_TexFormatOverride = texFormat;
		m_bTexFormatOverride = true;
	}

	void ClearTexFormatOverride()
	{
		m_TexFormatOverride = sprite::TexFormat::SPR_NORMAL;
		m_bTexFormatOverride = false;
	}

	float GetScale() const { return m_flScale; }

	void SetScale( const float flScale )
	{
		m_flScale = flScale;
	}

	bool DumpSpriteInfo( const char* const pszFilename );

public:

	bool showBackground;

private:
	CSpriteEntity* m_pEntity;

	sprite::Type::Type m_TypeOverride = sprite::Type::VP_PARALLEL_UPRIGHT;

	bool m_bTypeOverride = false;

	sprite::TexFormat::TexFormat m_TexFormatOverride = sprite::TexFormat::SPR_NORMAL;

	bool m_bTexFormatOverride = false;

	float m_flScale = 1.0f;

private:
	CSpriteViewerState( const CSpriteViewerState& ) = delete;
	CSpriteViewerState& operator=( const CSpriteViewerState& other ) = delete;
};
}

#endif //SPRITEVIEWER_CSPRITEVIEWERSTATE_H