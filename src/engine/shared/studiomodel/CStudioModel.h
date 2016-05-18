#ifndef GAME_STUDIOMODEL_CSTUDIOMODEL_H
#define GAME_STUDIOMODEL_CSTUDIOMODEL_H

#include <vector>

#include <glm/vec3.hpp>

#include "shared/Const.h"

#include "utility/mathlib.h"
#include "utility/Color.h"

#include "graphics/OpenGL.h"

#include "studio.h"

namespace studiomdl
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
*	@param pModel The model, if it was successfully loaded in.
*	@return StudioModelLoadResult::SUCCESS on success, an error code in all other cases.
*/
StudioModelLoadResult LoadStudioModel( const char* const pszFilename, CStudioModel*& pModel );

/**
*	Saves a studio model.
*	@param pszFilename Name of the file to save the model to. This is the entire path, including the extension.
*	@param pModel Model to save.
*	@return true on success, false otherwise.
*/
bool SaveStudioModel( const char* const pszFilename, const CStudioModel* const pModel );

/**
*	Container representing a studiomodel and its data.
*/
class CStudioModel final
{
private:
	typedef std::vector<const mstudiomesh_t*> MeshList_t;
	typedef std::vector<MeshList_t> TextureMeshMap_t;

protected:
	friend StudioModelLoadResult LoadStudioModel( const char* const pszFilename, CStudioModel*& pModel );

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

	mstudiomodel_t* GetModelByBodyPart( const int iBody, const int iBodyPart ) const;

	bool			CalculateBodygroup( const int iGroup, const int iValue, int& iInOutBodygroup ) const;

	GLuint			GetTextureId( const int iIndex ) const;

	void			ReplaceTexture( mstudiotexture_t* ptexture, byte *data, byte *pal, GLuint textureId );

	/**
	*	Reuploads a texture. Useful for making changes made to the texture's pixel, palette or flag data show up in the model itself.
	*	@param ptexture Texture to reupload. Must be a texture that is part of this model.
	*/
	void ReuploadTexture( mstudiotexture_t* ptexture );

private:
	studiohdr_t*	m_pStudioHdr;
	studiohdr_t*	m_pTextureHdr;

	studiohdr_t*	m_pSeqHdrs[ MAX_SEQGROUPS ];

	GLuint			m_Textures[ MAXSTUDIOSKINS ];

private:
	CStudioModel( const CStudioModel& ) = delete;
	CStudioModel& operator=( const CStudioModel& ) = delete;
};

void ScaleMeshes( CStudioModel* pStudioModel, const float flScale );
void ScaleBones( CStudioModel* pStudioModel, const float flScale );

/**
*	Returns the string representation for a studio control value.
*	@param iControl Value containing a STUDIO_* control bit.
*	@return String representation for the control, or nullptr.
*/
const char* ControlToString( const int iControl );

/**
*	Returns the description for a studio control value.
*	@param iControl Value containing a STUDIO_* control bit.
*	@return String description for the control, or nullptr.
*/
const char* ControlToStringDescription( const int iControl );

/**
*	Converts a string to a control value.
*	@param pszString String representation of a control value.
*	@return Control value, or -1 if the string does not represent a valid control.
*/
int StringToControl( const char* const pszString );
}

#endif //GAME_STUDIOMODEL_CSTUDIOMODEL_H
