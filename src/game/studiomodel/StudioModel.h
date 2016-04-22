/***
*
*	Copyright (c) 1998, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#ifndef INCLUDED_STUDIOMODEL
#define INCLUDED_STUDIOMODEL

#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "common/Const.h"

#include "utility/mathlib.h"
#include "utility/Color.h"

#include "graphics/OpenGL.h"

#include "studio.h"

const int CONTROLLER_MOUTH_INDEX	= 4;

//Mugsy - upped the maximum texture size to 512. All changes are the replacement of '256'
//with this define, MAX_TEXTURE_DIMS
#define MAX_TEXTURE_DIMS 512

struct CAnimEvent;

/**
*	Interface for handling animation events.
*	TODO: this should be merged into a class/interface that handles all operations for a model, like the game does.
*/
class IAnimEventHandler
{
public:
	virtual ~IAnimEventHandler() = 0;

	virtual void HandleAnimEvent( const CAnimEvent& event ) = 0;
};

inline IAnimEventHandler::~IAnimEventHandler()
{
}

/**
*	Stores a single model's data and state.
*	TODO: decompose into a data and state class.
*	TODO: this class should always contain a valid model.
*/
class StudioModel
{
public:
	typedef std::vector<const mstudiomesh_t*> MeshList_t;
	typedef std::vector<MeshList_t> TextureMeshMap_t;

	enum class LoadResult
	{
		SUCCESS = 0,
		FAILURE,			//Generic error on load.
		POSTLOADFAILURE,	//Generic error on post load.
		VERSIONDIFFERS		//Header version differs from current.
	};

public:
	StudioModel();
	~StudioModel();

	//CStudioModelCache
	const MeshList_t*		GetMeshListByTexture( const int iIndex ) const;

	//CStudioModel
	studiohdr_t				*GetStudioHeader() const { return m_pstudiohdr; }
	studiohdr_t				*GetTextureHeader() const { return m_ptexturehdr; }
	studiohdr_t				*GetAnimHeader( const int i ) const { return m_panimhdr[i]; }

	mstudioanim_t			*GetAnim( mstudioseqdesc_t *pseqdesc );

	GLuint					GetTextureId( const int iIndex ) const;

	void					ScaleMeshes( float scale );
	void					ScaleBones( float scale );

	//CBaseEntity
	const glm::vec3&		GetOrigin() const { return m_origin; }
	void					SetOrigin( const glm::vec3& vecOrigin ) { m_origin = vecOrigin; }

	const glm::vec3&		GetAngles() const { return m_angles; }
	void					SetAngles( const glm::vec3& vecAngles ) { m_angles = vecAngles; }

	//CBaseAnimating
	int						GetNumFrames() const;

	float					GetFrame() const { return m_frame; }

	float					AdvanceFrame( float dt = 0.0f );
	int						SetFrame( int nFrame );

	float					GetFrameRate() const { return m_flFrameRate; }
	void					SetFrameRate( const float flFrameRate ) { m_flFrameRate = flFrameRate; }

	int						GetSequence();
	void					GetSequenceInfo( float *pflFrameRate, float *pflGroundSpeed ) const;
	int						SetSequence( int iSequence );

	void					ExtractBbox( glm::vec3& vecMins, glm::vec3& vecMaxs ) const;

	byte					GetBoneController( int iController ) const;
	float					GetController( int iController ) const;
	float					SetController( int iController, float flValue );
	byte					GetMouth() const { return m_mouth; }
	float					SetMouth( float flValue );

	byte					GetBlendingValue( int iBlender ) const { return m_blending[ iBlender ]; }

	float					SetBlending( int iBlender, float flValue );

	int						SetBodygroup( int iGroup, int iValue );

	int						GetSkin() const { return m_skinnum; }
	int						SetSkin( int iValue );

	float					GetTransparency() const { return m_flTransparency; }
	void					SetTransparency( const float flTransparency ) { m_flTransparency = flTransparency; }

	int						GetAnimationEvent( CAnimEvent& event, float flStart, float flEnd, int index, const bool bAllowClientEvents );
	void					DispatchAnimEvents( IAnimEventHandler& handler, const bool bAllowClientEvents, float flInterval = 0.1f );

	mstudiomodel_t*			GetModelByBodyPart( const int iBodyPart ) const;

	void					UploadTexture( mstudiotexture_t *ptexture, byte *data, byte *pal, int name );
	void					UploadRGBATexture( const int iWidth, const int iHeight, byte* pData, GLuint textureId );
	void					ReplaceTexture( mstudiotexture_t *ptexture, byte *data, byte *pal, GLuint textureId );

	//CModelCache
	void					FreeModel();
	studiohdr_t*			LoadModel( const char* pszModelName, LoadResult* pResult = nullptr );
	bool					PostLoadModel( const char* const pszModelName );
	LoadResult				Load( const char* const pszModelName );
	bool					SaveModel( char *modelname );

private:
	// entity settings
	glm::vec3				m_origin;
	glm::vec3				m_angles;
	int						m_sequence;					// sequence index
	float					m_frame;					// frame
	int						m_bodynum;					// bodypart selection	
	int						m_skinnum;					// skin group selection
	byte					m_controller[4];			// bone controllers
	byte					m_blending[2];				// animation blending
	byte					m_mouth;					// mouth position
	bool					m_owntexmodel;				// do we have a modelT.mdl ?

	float					m_flLastEventCheck;			//Last time we checked for animation events.
	float					m_flAnimTime;				//Time when the frame was set.
	float					m_flFrameRate;				//Framerate.
	float					m_flTransparency = 1.0f;	//Transparency.

	// internal data
	studiohdr_t				*m_pstudiohdr;

	studiohdr_t				*m_ptexturehdr;
	studiohdr_t				*m_panimhdr[32];

	GLuint					m_Textures[ MAXSTUDIOSKINS ];

	TextureMeshMap_t		m_TextureMeshMap;
};

#endif // INCLUDED_STUDIOMODEL