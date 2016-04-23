#ifndef GAME_STUDIOMODEL_CSTUDIOMODEL_H
#define GAME_STUDIOMODEL_CSTUDIOMODEL_H

#include <vector>

#include <glm/vec3.hpp>

#include "common/Const.h"

#include "utility/mathlib.h"
#include "utility/Color.h"

#include "graphics/OpenGL.h"

#include "studio.h"

namespace studiomodel
{
enum class StudioModelLoadResult
{
	SUCCESS = 0,
	FAILURE,			//Generic error on load.
	POSTLOADFAILURE,	//Generic error on post load.
	VERSIONDIFFERS		//Header version differs from current.
};

class CStudioModel;

/**
*	Loads a studio model.
*	@param pszFilename Name of the model to load. This is the entire path, including the extension.
*	@param bFilterTextures Whether to filter textures uploaded for this model.
*	@param pModel The model, if it was successfully loaded in.
*	@return StudioModelLoadResult::SUCCESS on success, an error code in all other cases.
*/
StudioModelLoadResult LoadStudioModel( const char* const pszFilename, const bool bFilterTextures, CStudioModel*& pModel );

/**
*	Container representing a studiomodel and its data.
*/
class CStudioModel final
{
protected:
	friend StudioModelLoadResult LoadStudioModel( const char* const pszFilename, const bool bFilterTextures, CStudioModel*& pModel );

public:
	static const size_t MAX_SEQGROUPS = 32;
	static const size_t MAX_TEXTURES = MAXSTUDIOSKINS;

public:
	CStudioModel();
	CStudioModel( studiohdr_t* pStudioHdr, studiohdr_t* pTextureHdr, studiohdr_t** ppSeqHdrs, const size_t uiNumSeqHdrs, GLuint* pTextures, const size_t uiNumTextures );
	~CStudioModel();

	studiohdr_t*	GetStudioHeader() const { return m_pStudioHdr; }
	studiohdr_t*	GetTextureHeader() const { return m_pTextureHdr; }
	studiohdr_t*	GetSeqGroupHeader( const size_t i ) const { return m_pSeqHdrs[ i ]; }

	mstudioanim_t*	GetAnim( mstudioseqdesc_t* pseqdesc ) const;

	GLuint			GetTextureId( const int iIndex ) const;

private:
	studiohdr_t*	m_pStudioHdr;
	studiohdr_t*	m_pTextureHdr;

	studiohdr_t*	m_pSeqHdrs[ MAX_SEQGROUPS ];

	GLuint			m_Textures[ MAXSTUDIOSKINS ];

private:
	CStudioModel( const CStudioModel& ) = delete;
	CStudioModel& operator=( const CStudioModel& ) = delete;
};
}

#endif //GAME_STUDIOMODEL_CSTUDIOMODEL_H
