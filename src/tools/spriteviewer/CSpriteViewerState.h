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

	bool DumpSpriteInfo( const char* const pszFilename );

public:

	bool showBackground;

private:
	CSpriteEntity* m_pEntity;

	sprite::TexFormat::TexFormat m_TexFormatOverride = sprite::TexFormat::SPR_NORMAL;

	bool m_bTexFormatOverride = false;

private:
	CSpriteViewerState( const CSpriteViewerState& ) = delete;
	CSpriteViewerState& operator=( const CSpriteViewerState& other ) = delete;
};
}

#endif //SPRITEVIEWER_CSPRITEVIEWERSTATE_H