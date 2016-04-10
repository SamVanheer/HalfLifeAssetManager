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

#ifndef byte
typedef unsigned char byte;
#endif // byte

#include "mathlib.h"
#include "studio.h"

#include "model/utility/OpenGL.h"

enum TextureFlag
{
	STUDIO_NF_RENDER_FLAGS = STUDIO_NF_CHROME | STUDIO_NF_ADDITIVE | STUDIO_NF_MASKED
};

const int CONTROLLER_MOUTH_INDEX	= 4;

const size_t PALETTE_ENTRIES		= 256;
const size_t PALETTE_CHANNELS		= 3;
const size_t PALETTE_SIZE			= PALETTE_ENTRIES * PALETTE_CHANNELS;

const size_t PALETTE_ALPHA_INDEX	= 255 * PALETTE_CHANNELS;

//Mugsy - upped the maximum texture size to 512. All changes are the replacement of '256'
//with this define, MAX_TEXTURE_DIMS
#define MAX_TEXTURE_DIMS 512

/*
*	Converts image dimensions to power of 2.
*	Returns true if either width or height changed.
*/
bool CalculateImageDimensions( const int iWidth, const int iHeight, int& iOutWidth, int& iOutHeight );

void Convert8to24Bit( const int iWidth, const int iHeight, const byte* const pData, const byte* const pPalette, byte* const pOutData );

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

	class CRenderSettings final
	{
	public:
		CRenderSettings() = default;
		CRenderSettings( const CRenderSettings& other ) = default;
		CRenderSettings& operator=( const CRenderSettings& other ) = default;

		void ResetToDefaults()
		{
			*this = CRenderSettings();
		}

	public:
		vec3_t lightColor		= { 1.0f, 1.0f, 1.0f };

		float transparency		= 1.0f;
		bool showBones			= false;
		bool showAttachments	= false;
		bool showEyePosition	= false;
		bool showHitBoxes		= false;

		vec3_t wireframeColor	= { 1.0f, 0.0f, 0.0f };
	};

public:
	studiohdr_t				*getStudioHeader () const { return m_pstudiohdr; }
	studiohdr_t				*getTextureHeader () const { return m_ptexturehdr; }
	studiohdr_t				*getAnimHeader (int i) const { return m_panimhdr[i]; }

	float GetFrame() const { return m_frame; }

	int GetNumFrames() const;

	GLuint GetTextureId( const int iIndex ) const;

	mstudiomodel_t* GetModelByBodyPart( const int iBodyPart ) const;

	const MeshList_t* GetMeshListByTexture( const int iIndex ) const;

	StudioModel();
	~StudioModel();

	void					UploadTexture( mstudiotexture_t *ptexture, byte *data, byte *pal, int name );
	void					UploadRGBATexture( const int iWidth, const int iHeight, byte* pData, GLuint textureId );
	void					ReplaceTexture( mstudiotexture_t *ptexture, byte *data, byte *pal, GLuint textureId );
	void					FreeModel ();
	studiohdr_t				*LoadModel( const char* pszModelName, LoadResult* pResult = nullptr );
	bool					PostLoadModel ( const char* const pszModelName );
	LoadResult				Load( const char* const pszModelName );
	bool					SaveModel ( char *modelname );
	unsigned int			DrawModel( const CRenderSettings& settings, const bool wireframeOnly = false );
	void					AdvanceFrame( float dt );
	int						SetFrame (int nFrame);

	void					ExtractBbox( float *mins, float *maxs ) const;

	int						SetSequence( int iSequence );
	int						GetSequence( void );
	void					GetSequenceInfo( float *pflFrameRate, float *pflGroundSpeed );

	float					GetController( int iController ) const;

	float					SetController( int iController, float flValue );
	float					SetMouth( float flValue );
	float					SetBlending( int iBlender, float flValue );
	int						SetBodygroup( int iGroup, int iValue );
	int						SetSkin( int iValue );

	void					scaleMeshes (float scale);
	void					scaleBones (float scale);

private:
	// entity settings
	vec3_t					m_origin;
	vec3_t					m_angles;	
	int						m_sequence;			// sequence index
	float					m_frame;			// frame
	int						m_bodynum;			// bodypart selection	
	int						m_skinnum;			// skin group selection
	byte					m_controller[4];	// bone controllers
	byte					m_blending[2];		// animation blending
	byte					m_mouth;			// mouth position
	bool					m_owntexmodel;		// do we have a modelT.mdl ?

	// internal data
	studiohdr_t				*m_pstudiohdr;
	mstudiomodel_t			*m_pmodel;

	studiohdr_t				*m_ptexturehdr;
	studiohdr_t				*m_panimhdr[32];

	vec4_t					m_adj;				// FIX: non persistant, make static

	GLuint					m_Textures[ MAXSTUDIOSKINS ];

	TextureMeshMap_t		m_TextureMeshMap;

	void					CalcBoneAdj( void );
	void					CalcBoneQuaternion( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *q );
	void					CalcBonePosition( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *pos );
	void					CalcRotations ( vec3_t *pos, vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f );
	mstudioanim_t			*GetAnim( mstudioseqdesc_t *pseqdesc );
	void					SlerpBones( vec4_t q1[], vec3_t pos1[], vec4_t q2[], vec3_t pos2[], float s );
	void					SetUpBones ( void );

	unsigned int			DrawPoints( const CRenderSettings& settings, const bool wireframeOnly = false );

	void					Lighting (float *lv, int bone, int flags, vec3_t normal);
	void					Chrome (int *chrome, int bone, vec3_t normal);

	void					SetupLighting( const CRenderSettings& settings );

	void					SetupModel ( int bodypart );
};



extern vec3_t g_vright;		// needs to be set to viewer's right in order for chrome to work
extern float g_lambert;		// modifier for pseudo-hemispherical lighting

#endif // INCLUDED_STUDIOMODEL