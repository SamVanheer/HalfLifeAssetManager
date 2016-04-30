#ifndef SPRITEVIEWER_CSPRITEVIEWERSTATE_H
#define SPRITEVIEWER_CSPRITEVIEWERSTATE_H

#include <vector>
#include <memory>

#include <glm/vec3.hpp>

#include "shared/Platform.h"
#include "graphics/OpenGL.h"
#include "graphics/Constants.h"

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

	bool DumpSpriteInfo( const char* const pszFilename );

public:

	bool showBackground;

private:
	CSpriteEntity* m_pEntity;

private:
	CSpriteViewerState( const CSpriteViewerState& ) = delete;
	CSpriteViewerState& operator=( const CSpriteViewerState& other ) = delete;
};
}

#endif //SPRITEVIEWER_CSPRITEVIEWERSTATE_H