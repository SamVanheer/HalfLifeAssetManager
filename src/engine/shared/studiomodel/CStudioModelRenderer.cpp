#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

#include "shared/Logging.h"

#include "cvar/CCVar.h"
#include "cvar/CVarUtils.h"

#include "graphics/GraphicsHelpers.h"

#include "shared/studiomodel/CStudioModel.h"
#include "StudioSorting.h"

#include "game/entity/CStudioModelEntity.h"

#include "CStudioModelRenderer.h"

//Double to float conversion
#pragma warning( disable: 4244 )

cvar::CCVar g_ShowBones( "r_showbones", cvar::CCVarArgsBuilder().FloatValue( 0 ).HelpInfo( "If non-zero, shows model bones" ) );
cvar::CCVar g_ShowAttachments( "r_showattachments", cvar::CCVarArgsBuilder().FloatValue( 0 ).HelpInfo( "If non-zero, shows model attachments" ) );
cvar::CCVar g_ShowEyePosition( "r_showeyeposition", cvar::CCVarArgsBuilder().FloatValue( 0 ).HelpInfo( "If non-zero, shows model eye position" ) );
cvar::CCVar g_ShowHitboxes( "r_showhitboxes", cvar::CCVarArgsBuilder().FloatValue( 0 ).HelpInfo( "If non-zero, shows model hitboxes" ) );

//TODO: this is temporary until lighting can be moved somewhere else

DEFINE_COLOR_CVAR( , r_lighting, 255, 255, 255, "Lighting", cvar::CCVarArgsBuilder().Flags( cvar::Flag::ARCHIVE ).Callback( cvar::ColorCVarChanged ) );
DEFINE_COLOR_CVAR( , r_wireframecolor, 255, 0, 0, "Wireframe overlay color", cvar::CCVarArgsBuilder().Flags( cvar::Flag::ARCHIVE ).Callback( cvar::ColorCVarChanged ) );

namespace studiomodel
{
namespace
{
static CStudioModelRenderer g_Renderer;
}

CStudioModelRenderer& renderer()
{
	return g_Renderer;
}

CStudioModelRenderer::CStudioModelRenderer()
{
}

CStudioModelRenderer::~CStudioModelRenderer()
{
}

bool CStudioModelRenderer::Initialize()
{
	m_uiModelsDrawnCount = 0;

	m_uiDrawnPolygonsCount = 0;

	return true;
}

void CStudioModelRenderer::Shutdown()
{
}

void CStudioModelRenderer::RunFrame()
{
}

unsigned int CStudioModelRenderer::DrawModel( CStudioModelEntity* const pEntity, const DrawFlags_t flags )
{
	m_pEntity = pEntity;

	if( !pEntity )
	{
		Error( "CStudioModelRenderer::DrawModel: Called with null entity!\n" );
		return 0;
	}

	m_pCurrentModel = pEntity->GetModel();

	if( m_pCurrentModel )
	{
		m_pStudioHdr = m_pCurrentModel->GetStudioHeader();
		m_pTextureHdr = m_pCurrentModel->GetTextureHeader();
	}
	else
	{
		Error( "CStudioModelRenderer::DrawModel: Called with null model!\n" );
		return 0;
	}

	++m_uiModelsDrawnCount; // render data cache cookie

	m_pxformverts = &m_xformverts[ 0 ];
	m_pvlightvalues = &m_lightvalues[ 0 ];

	if( m_pStudioHdr->numbodyparts == 0 )
		return 0;

	glPushMatrix();

	const glm::vec3& vecOrigin	= m_pEntity->GetOrigin();
	const glm::vec3& vecAngles	= m_pEntity->GetAngles();
	const glm::vec3& vecScale	= m_pEntity->GetScale();

	glTranslatef( vecOrigin[ 0 ], vecOrigin[ 1 ], vecOrigin[ 2 ] );

	glRotatef( vecAngles[ 1 ], 0, 0, 1 );
	glRotatef( vecAngles[ 0 ], 0, 1, 0 );
	glRotatef( vecAngles[ 2 ], 1, 0, 0 );

	glScalef( vecScale.x, vecScale.y, vecScale.z );

	SetUpBones();

	SetupLighting();

	unsigned int uiDrawnPolys = 0;

	for( int i = 0; i < m_pStudioHdr->numbodyparts; i++ )
	{
		SetupModel( i );
		if( m_pEntity->GetTransparency() > 0.0f )
			uiDrawnPolys += DrawPoints( ( flags & DRAWF_WIREFRAME_ONLY ) != 0 );
	}

	//TODO: separate out into methods.

	// draw bones
	if( g_ShowBones.GetBool() )
	{
		mstudiobone_t *pbones = m_pStudioHdr->GetBones();
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_DEPTH_TEST );

		for( int i = 0; i < m_pStudioHdr->numbones; i++ )
		{
			if( pbones[ i ].parent >= 0 )
			{
				glPointSize( 3.0f );
				glColor3f( 1, 0.7f, 0 );
				glBegin( GL_LINES );
				glVertex3f( m_bonetransform[ pbones[ i ].parent ][ 0 ][ 3 ], m_bonetransform[ pbones[ i ].parent ][ 1 ][ 3 ], m_bonetransform[ pbones[ i ].parent ][ 2 ][ 3 ] );
				glVertex3f( m_bonetransform[ i ][ 0 ][ 3 ], m_bonetransform[ i ][ 1 ][ 3 ], m_bonetransform[ i ][ 2 ][ 3 ] );
				glEnd();

				glColor3f( 0, 0, 0.8f );
				glBegin( GL_POINTS );
				if( pbones[ pbones[ i ].parent ].parent != -1 )
					glVertex3f( m_bonetransform[ pbones[ i ].parent ][ 0 ][ 3 ], m_bonetransform[ pbones[ i ].parent ][ 1 ][ 3 ], m_bonetransform[ pbones[ i ].parent ][ 2 ][ 3 ] );
				glVertex3f( m_bonetransform[ i ][ 0 ][ 3 ], m_bonetransform[ i ][ 1 ][ 3 ], m_bonetransform[ i ][ 2 ][ 3 ] );
				glEnd();
			}
			else
			{
				// draw parent bone node
				glPointSize( 5.0f );
				glColor3f( 0.8f, 0, 0 );
				glBegin( GL_POINTS );
				glVertex3f( m_bonetransform[ i ][ 0 ][ 3 ], m_bonetransform[ i ][ 1 ][ 3 ], m_bonetransform[ i ][ 2 ][ 3 ] );
				glEnd();
			}
		}

		glPointSize( 1.0f );
	}

	if( g_ShowAttachments.GetBool() )
	{
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_CULL_FACE );
		glDisable( GL_DEPTH_TEST );

		for( int i = 0; i < m_pStudioHdr->numattachments; i++ )
		{
			mstudioattachment_t *pattachments = m_pStudioHdr->GetAttachments();
			glm::vec3 v[ 4 ];
			VectorTransform( pattachments[ i ].org, m_bonetransform[ pattachments[ i ].bone ], v[ 0 ] );
			VectorTransform( pattachments[ i ].vectors[ 0 ], m_bonetransform[ pattachments[ i ].bone ], v[ 1 ] );
			VectorTransform( pattachments[ i ].vectors[ 1 ], m_bonetransform[ pattachments[ i ].bone ], v[ 2 ] );
			VectorTransform( pattachments[ i ].vectors[ 2 ], m_bonetransform[ pattachments[ i ].bone ], v[ 3 ] );
			glBegin( GL_LINES );
			glColor3f( 1, 0, 0 );
			glVertex3fv( glm::value_ptr( v[ 0 ] ) );
			glColor3f( 1, 1, 1 );
			glVertex3fv( glm::value_ptr( v[ 1 ] ) );
			glColor3f( 1, 0, 0 );
			glVertex3fv( glm::value_ptr( v[ 0 ] ) );
			glColor3f( 1, 1, 1 );
			glVertex3fv( glm::value_ptr( v[ 2 ] ) );
			glColor3f( 1, 0, 0 );
			glVertex3fv( glm::value_ptr( v[ 0 ] ) );
			glColor3f( 1, 1, 1 );
			glVertex3fv( glm::value_ptr( v[ 3 ] ) );
			glEnd();

			glPointSize( 5 );
			glColor3f( 0, 1, 0 );
			glBegin( GL_POINTS );
			glVertex3fv( glm::value_ptr( v[ 0 ] ) );
			glEnd();
			glPointSize( 1 );
		}
	}

	if( g_ShowEyePosition.GetBool() )
	{
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_CULL_FACE );
		glDisable( GL_DEPTH_TEST );

		glPointSize( 7 );
		glColor3f( 1, 0, 1 );
		glBegin( GL_POINTS );
		glVertex3fv( glm::value_ptr( m_pStudioHdr->eyeposition ) );
		glEnd();
		glPointSize( 1 );
	}

	if( g_ShowHitboxes.GetBool() )
	{
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_CULL_FACE );
		if( m_pEntity->GetTransparency() < 1.0f )
			glDisable( GL_DEPTH_TEST );
		else
			glEnable( GL_DEPTH_TEST );

		glColor4f( 1, 0, 0, 0.5f );

		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		for( int i = 0; i < m_pStudioHdr->numhitboxes; i++ )
		{
			mstudiobbox_t *pbboxes = m_pStudioHdr->GetHitBoxes();
			glm::vec3 v[ 8 ], v2[ 8 ];

			glm::vec3 bbmin = pbboxes[ i ].bbmin;
			glm::vec3 bbmax = pbboxes[ i ].bbmax;

			v[ 0 ][ 0 ] = bbmin[ 0 ];
			v[ 0 ][ 1 ] = bbmax[ 1 ];
			v[ 0 ][ 2 ] = bbmin[ 2 ];

			v[ 1 ][ 0 ] = bbmin[ 0 ];
			v[ 1 ][ 1 ] = bbmin[ 1 ];
			v[ 1 ][ 2 ] = bbmin[ 2 ];

			v[ 2 ][ 0 ] = bbmax[ 0 ];
			v[ 2 ][ 1 ] = bbmax[ 1 ];
			v[ 2 ][ 2 ] = bbmin[ 2 ];

			v[ 3 ][ 0 ] = bbmax[ 0 ];
			v[ 3 ][ 1 ] = bbmin[ 1 ];
			v[ 3 ][ 2 ] = bbmin[ 2 ];

			v[ 4 ][ 0 ] = bbmax[ 0 ];
			v[ 4 ][ 1 ] = bbmax[ 1 ];
			v[ 4 ][ 2 ] = bbmax[ 2 ];

			v[ 5 ][ 0 ] = bbmax[ 0 ];
			v[ 5 ][ 1 ] = bbmin[ 1 ];
			v[ 5 ][ 2 ] = bbmax[ 2 ];

			v[ 6 ][ 0 ] = bbmin[ 0 ];
			v[ 6 ][ 1 ] = bbmax[ 1 ];
			v[ 6 ][ 2 ] = bbmax[ 2 ];

			v[ 7 ][ 0 ] = bbmin[ 0 ];
			v[ 7 ][ 1 ] = bbmin[ 1 ];
			v[ 7 ][ 2 ] = bbmax[ 2 ];

			VectorTransform( v[ 0 ], m_bonetransform[ pbboxes[ i ].bone ], v2[ 0 ] );
			VectorTransform( v[ 1 ], m_bonetransform[ pbboxes[ i ].bone ], v2[ 1 ] );
			VectorTransform( v[ 2 ], m_bonetransform[ pbboxes[ i ].bone ], v2[ 2 ] );
			VectorTransform( v[ 3 ], m_bonetransform[ pbboxes[ i ].bone ], v2[ 3 ] );
			VectorTransform( v[ 4 ], m_bonetransform[ pbboxes[ i ].bone ], v2[ 4 ] );
			VectorTransform( v[ 5 ], m_bonetransform[ pbboxes[ i ].bone ], v2[ 5 ] );
			VectorTransform( v[ 6 ], m_bonetransform[ pbboxes[ i ].bone ], v2[ 6 ] );
			VectorTransform( v[ 7 ], m_bonetransform[ pbboxes[ i ].bone ], v2[ 7 ] );

			graphics::helpers::DrawBox( v2 );
		}
	}

	glPopMatrix();

	m_uiDrawnPolygonsCount += uiDrawnPolys;

	return uiDrawnPolys;
}

void CStudioModelRenderer::SetUpBones()
{
	int					i;

	mstudiobone_t		*pbones;
	mstudioseqdesc_t	*pseqdesc;
	mstudioanim_t		*panim;

	static glm::vec3		pos[ MAXSTUDIOBONES ];
	static glm::vec4		q[ MAXSTUDIOBONES ];

	static glm::vec3		pos2[ MAXSTUDIOBONES ];
	static glm::vec4		q2[ MAXSTUDIOBONES ];
	static glm::vec3		pos3[ MAXSTUDIOBONES ];
	static glm::vec4		q3[ MAXSTUDIOBONES ];
	static glm::vec3		pos4[ MAXSTUDIOBONES ];
	static glm::vec4		q4[ MAXSTUDIOBONES ];

	if( m_pEntity->GetSequence() >= m_pStudioHdr->numseq )
	{
		m_pEntity->SetSequence( 0 );
	}

	pseqdesc = m_pStudioHdr->GetSequence( m_pEntity->GetSequence() );

	panim = m_pCurrentModel->GetAnim( pseqdesc );
	CalcRotations( pos, q, pseqdesc, panim, m_pEntity->GetFrame() );

	if( pseqdesc->numblends > 1 )
	{
		float				s;

		panim += m_pStudioHdr->numbones;
		CalcRotations( pos2, q2, pseqdesc, panim, m_pEntity->GetFrame() );
		s = m_pEntity->GetBlendingByIndex( 0 ) / 255.0;

		SlerpBones( q, pos, q2, pos2, s );

		if( pseqdesc->numblends == 4 )
		{
			panim += m_pStudioHdr->numbones;
			CalcRotations( pos3, q3, pseqdesc, panim, m_pEntity->GetFrame() );

			panim += m_pStudioHdr->numbones;
			CalcRotations( pos4, q4, pseqdesc, panim, m_pEntity->GetFrame() );

			s = m_pEntity->GetBlendingByIndex( 0 ) / 255.0;
			SlerpBones( q3, pos3, q4, pos4, s );

			s = m_pEntity->GetBlendingByIndex( 1 ) / 255.0;
			SlerpBones( q, pos, q3, pos3, s );
		}
	}

	pbones = m_pStudioHdr->GetBones();

	glm::mat3x4 bonematrix;

	for( i = 0; i < m_pStudioHdr->numbones; i++ )
	{
		QuaternionMatrix( q[ i ], bonematrix );

		bonematrix[ 0 ][ 3 ] = pos[ i ][ 0 ];
		bonematrix[ 1 ][ 3 ] = pos[ i ][ 1 ];
		bonematrix[ 2 ][ 3 ] = pos[ i ][ 2 ];

		if( pbones[ i ].parent == -1 )
		{
			m_bonetransform[ i ] = bonematrix;
		}
		else {
			R_ConcatTransforms( m_bonetransform[ pbones[ i ].parent ], bonematrix, m_bonetransform[ i ] );
		}
	}
}

void CStudioModelRenderer::CalcRotations( glm::vec3 *pos, glm::vec4 *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f )
{
	int					i;
	int					frame;
	mstudiobone_t		*pbone;
	float				s;

	frame = ( int ) f;
	s = ( f - frame );

	// add in programatic controllers
	CalcBoneAdj();

	pbone = m_pStudioHdr->GetBones();
	for( i = 0; i < m_pStudioHdr->numbones; i++, pbone++, panim++ )
	{
		CalcBoneQuaternion( frame, s, pbone, panim, q[ i ] );
		CalcBonePosition( frame, s, pbone, panim, pos[ i ] );
	}

	if( pseqdesc->motiontype & STUDIO_X )
		pos[ pseqdesc->motionbone ][ 0 ] = 0.0;
	if( pseqdesc->motiontype & STUDIO_Y )
		pos[ pseqdesc->motionbone ][ 1 ] = 0.0;
	if( pseqdesc->motiontype & STUDIO_Z )
		pos[ pseqdesc->motionbone ][ 2 ] = 0.0;
}

void CStudioModelRenderer::CalcBoneAdj()
{
	int					i, j;
	float				value;
	mstudiobonecontroller_t *pbonecontroller;

	pbonecontroller = m_pStudioHdr->GetBoneControllers();

	for( j = 0; j < m_pStudioHdr->numbonecontrollers; j++ )
	{
		i = pbonecontroller[ j ].index;
		if( i <= 3 )
		{
			// check for 360% wrapping
			if( pbonecontroller[ j ].type & STUDIO_RLOOP )
			{
				value = m_pEntity->GetControllerByIndex( i ) * ( 360.0 / 256.0 ) + pbonecontroller[ j ].start;
			}
			else
			{
				value = m_pEntity->GetControllerByIndex(  i ) / 255.0;
				if( value < 0 ) value = 0;
				if( value > 1.0 ) value = 1.0;
				value = ( 1.0 - value ) * pbonecontroller[ j ].start + value * pbonecontroller[ j ].end;
			}
			// Con_DPrintf( "%d %d %f : %f\n", m_controller[j], m_prevcontroller[j], value, dadt );
		}
		else
		{
			value = m_pEntity->GetMouth() / 64.0;
			if( value > 1.0 ) value = 1.0;
			value = ( 1.0 - value ) * pbonecontroller[ j ].start + value * pbonecontroller[ j ].end;
			// Con_DPrintf("%d %f\n", mouthopen, value );
		}
		switch( pbonecontroller[ j ].type & STUDIO_TYPES )
		{
		case STUDIO_XR:
		case STUDIO_YR:
		case STUDIO_ZR:
			m_Adj[ j ] = value * ( Q_PI / 180.0 );
			break;
		case STUDIO_X:
		case STUDIO_Y:
		case STUDIO_Z:
			m_Adj[ j ] = value;
			break;
		}
	}
}

void CStudioModelRenderer::CalcBoneQuaternion( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, glm::vec4& q )
{
	int					j, k;
	glm::vec4			q1, q2;
	glm::vec3			angle1, angle2;
	mstudioanimvalue_t	*panimvalue;

	for( j = 0; j < 3; j++ )
	{
		if( panim->offset[ j + 3 ] == 0 )
		{
			angle2[ j ] = angle1[ j ] = pbone->value[ j + 3 ]; // default;
		}
		else
		{
			panimvalue = ( mstudioanimvalue_t * ) ( ( byte * ) panim + panim->offset[ j + 3 ] );
			k = frame;
			while( panimvalue->num.total <= k )
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
			}
			// Bah, missing blend!
			if( panimvalue->num.valid > k )
			{
				angle1[ j ] = panimvalue[ k + 1 ].value;

				if( panimvalue->num.valid > k + 1 )
				{
					angle2[ j ] = panimvalue[ k + 2 ].value;
				}
				else
				{
					if( panimvalue->num.total > k + 1 )
						angle2[ j ] = angle1[ j ];
					else
						angle2[ j ] = panimvalue[ panimvalue->num.valid + 2 ].value;
				}
			}
			else
			{
				angle1[ j ] = panimvalue[ panimvalue->num.valid ].value;
				if( panimvalue->num.total > k + 1 )
				{
					angle2[ j ] = angle1[ j ];
				}
				else
				{
					angle2[ j ] = panimvalue[ panimvalue->num.valid + 2 ].value;
				}
			}
			angle1[ j ] = pbone->value[ j + 3 ] + angle1[ j ] * pbone->scale[ j + 3 ];
			angle2[ j ] = pbone->value[ j + 3 ] + angle2[ j ] * pbone->scale[ j + 3 ];
		}

		if( pbone->bonecontroller[ j + 3 ] != -1 )
		{
			angle1[ j ] += m_Adj[ pbone->bonecontroller[ j + 3 ] ];
			angle2[ j ] += m_Adj[ pbone->bonecontroller[ j + 3 ] ];
		}
	}

	if( !VectorCompare( angle1, angle2 ) )
	{
		AngleQuaternion( angle1, q1 );
		AngleQuaternion( angle2, q2 );
		QuaternionSlerp( q1, q2, s, q );
	}
	else
	{
		AngleQuaternion( angle1, q );
	}
}

void CStudioModelRenderer::CalcBonePosition( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, glm::vec3& pos )
{
	int					j, k;
	mstudioanimvalue_t	*panimvalue;

	for( j = 0; j < 3; j++ )
	{
		pos[ j ] = pbone->value[ j ]; // default;
		if( panim->offset[ j ] != 0 )
		{
			panimvalue = ( mstudioanimvalue_t * ) ( ( byte * ) panim + panim->offset[ j ] );

			k = frame;
			// find span of values that includes the frame we want
			while( panimvalue->num.total <= k )
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
			}
			// if we're inside the span
			if( panimvalue->num.valid > k )
			{
				// and there's more data in the span
				if( panimvalue->num.valid > k + 1 )
				{
					pos[ j ] += ( panimvalue[ k + 1 ].value * ( 1.0 - s ) + s * panimvalue[ k + 2 ].value ) * pbone->scale[ j ];
				}
				else
				{
					pos[ j ] += panimvalue[ k + 1 ].value * pbone->scale[ j ];
				}
			}
			else
			{
				// are we at the end of the repeating values section and there's another section with data?
				if( panimvalue->num.total <= k + 1 )
				{
					pos[ j ] += ( panimvalue[ panimvalue->num.valid ].value * ( 1.0 - s ) + s * panimvalue[ panimvalue->num.valid + 2 ].value ) * pbone->scale[ j ];
				}
				else
				{
					pos[ j ] += panimvalue[ panimvalue->num.valid ].value * pbone->scale[ j ];
				}
			}
		}
		if( pbone->bonecontroller[ j ] != -1 )
		{
			pos[ j ] += m_Adj[ pbone->bonecontroller[ j ] ];
		}
	}
}

void CStudioModelRenderer::SlerpBones( glm::vec4* q1, glm::vec3* pos1, glm::vec4* q2, glm::vec3* pos2, float s )
{
	int			i;
	glm::vec4	q3;
	float		s1;

	if( s < 0 ) s = 0;
	else if( s > 1.0 ) s = 1.0;

	s1 = 1.0 - s;

	for( i = 0; i < m_pStudioHdr->numbones; i++ )
	{
		QuaternionSlerp( q1[ i ], q2[ i ], s, q3 );
		q1[ i ][ 0 ] = q3[ 0 ];
		q1[ i ][ 1 ] = q3[ 1 ];
		q1[ i ][ 2 ] = q3[ 2 ];
		q1[ i ][ 3 ] = q3[ 3 ];
		pos1[ i ][ 0 ] = pos1[ i ][ 0 ] * s1 + pos2[ i ][ 0 ] * s;
		pos1[ i ][ 1 ] = pos1[ i ][ 1 ] * s1 + pos2[ i ][ 1 ] * s;
		pos1[ i ][ 2 ] = pos1[ i ][ 2 ] * s1 + pos2[ i ][ 2 ] * s;
	}
}

/*
================
CStudioModelRenderer::SetupLighting
set some global variables based on entity position
inputs:
outputs:
g_ambientlight
g_shadelight
================
*/
void CStudioModelRenderer::SetupLighting()
{
	m_ambientlight = 32;
	m_shadelight = 192;

	m_lightcolor[ 0 ] = r_lighting_r.GetInt();
	m_lightcolor[ 1 ] = r_lighting_g.GetInt();
	m_lightcolor[ 2 ] = r_lighting_b.GetInt();

	// TODO: only do it for bones that actually have textures
	for( int i = 0; i < m_pStudioHdr->numbones; i++ )
	{
		VectorIRotate( m_lightvec, m_bonetransform[ i ], m_blightvec[ i ] );
	}
}

/*
=================
CStudioModelRenderer::SetupModel
based on the body part, figure out which mesh it should be using.
inputs:
currententity
outputs:
pstudiomesh
pmdl
=================
*/
void CStudioModelRenderer::SetupModel( int bodypart )
{
	if( bodypart > m_pStudioHdr->numbodyparts )
	{
		// Con_DPrintf ("CStudioModelRenderer::SetupModel: no such bodypart %d\n", bodypart);
		bodypart = 0;
	}

	m_pModel = m_pEntity->GetModelByBodyPart( bodypart );
}

unsigned int CStudioModelRenderer::DrawPoints( const bool wireframeOnly )
{
	int					i, j;
	mstudiomesh_t		*pmesh;
	byte				*pvertbone;
	byte				*pnormbone;
	const glm::vec3*			pstudioverts;
	const glm::vec3*	pstudionorms;
	mstudiotexture_t	*ptexture;
	short				*pskinref;

	unsigned int uiDrawnPolys = 0;

	pvertbone = ( ( byte * ) m_pStudioHdr + m_pModel->vertinfoindex );
	pnormbone = ( ( byte * ) m_pStudioHdr + m_pModel->norminfoindex );
	ptexture = m_pTextureHdr->GetTextures();

	pmesh = ( mstudiomesh_t * ) ( ( byte * ) m_pStudioHdr + m_pModel->meshindex );

	pstudioverts = ( const glm::vec3* ) ( ( const byte* ) m_pStudioHdr + m_pModel->vertindex );
	pstudionorms = ( const glm::vec3* ) ( ( const byte* ) m_pStudioHdr + m_pModel->normindex );

	pskinref = m_pTextureHdr->GetSkins();

	const int iSkinNum = m_pEntity->GetSkin();

	if( iSkinNum != 0 && iSkinNum < m_pTextureHdr->numskinfamilies )
		pskinref += ( iSkinNum * m_pTextureHdr->numskinref );

	for( i = 0; i < m_pModel->numverts; i++ )
	{
		//glm::vec3 tmp = pstudioverts[ i ] * 12;
		VectorTransform( pstudioverts[ i ], m_bonetransform[ pvertbone[ i ] ], m_pxformverts[ i ] );
	}

	SortedMesh_t meshes[ MAXSTUDIOMESHES ];

	//
	// clip and draw all triangles
	//

	glm::vec3* lv = m_pvlightvalues;
	for( j = 0; j < m_pModel->nummesh; j++ )
	{
		int flags = ptexture[ pskinref[ pmesh[ j ].skinref ] ].flags;

		meshes[ j ].pMesh = &pmesh[ j ];
		meshes[ j ].flags = flags;

		for( i = 0; i < pmesh[ j ].numnorms; i++, ++lv, ++pstudionorms, pnormbone++ )
		{
			Lighting( *lv, *pnormbone, flags, *pstudionorms );

			// FIX: move this check out of the inner loop
			if( flags & STUDIO_NF_CHROME )
				Chrome( m_chrome[ reinterpret_cast<glm::vec3*>( lv ) - m_pvlightvalues ], *pnormbone, *pstudionorms );
		}
	}

	//Sort meshes by render modes so additive meshes are drawn after solid meshes.
	//Masked meshes are drawn before solid meshes.
	std::stable_sort( meshes, meshes + m_pModel->nummesh, CompareSortedMeshes );

	// glCullFace(GL_FRONT);

	//Set here since it never changes. Much more efficient.
	if( wireframeOnly )
		glColor4f( r_wireframecolor_r.GetFloat() / 255.0f, 
				   r_wireframecolor_g.GetFloat() / 255.0f, 
				   r_wireframecolor_b.GetFloat() / 255.0f, 
				   m_pEntity->GetTransparency() );

	for( j = 0; j < m_pModel->nummesh; j++ )
	{
		float s, t;
		short		*ptricmds;

		pmesh = meshes[ j ].pMesh;
		ptricmds = ( short * ) ( ( byte * ) m_pStudioHdr + pmesh->triindex );

		const mstudiotexture_t& texture = ptexture[ pskinref[ pmesh->skinref ] ];

		s = 1.0 / ( float ) texture.width;
		t = 1.0 / ( float ) texture.height;

		if( texture.flags & STUDIO_NF_ADDITIVE )
			glDepthMask( GL_FALSE );
		else
			glDepthMask( GL_TRUE );

		//TODO: additive textures should be drawn last, using the painter's algorithm.
		if( texture.flags & STUDIO_NF_ADDITIVE )
		{
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
		}
		else if( m_pEntity->GetTransparency() < 1.0f )
		{
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}
		else
			glDisable( GL_BLEND );

		if( texture.flags & STUDIO_NF_MASKED )
		{
			glEnable( GL_ALPHA_TEST );
			glAlphaFunc( GL_GREATER, 0.5f );
		}

		if( !wireframeOnly )
		{
			glBindTexture( GL_TEXTURE_2D, m_pCurrentModel->GetTextureId( pskinref[ pmesh->skinref ] ) );
		}

		while( i = *( ptricmds++ ) )
		{
			if( i < 0 )
			{
				glBegin( GL_TRIANGLE_FAN );
				i = -i;
			}
			else
			{
				glBegin( GL_TRIANGLE_STRIP );
			}

			uiDrawnPolys += i - 2;

			for( ; i > 0; i--, ptricmds += 4 )
			{
				if( !wireframeOnly )
				{
					if( texture.flags & STUDIO_NF_CHROME )
					{
						// FIX: put these in as integer coords, not floats
						glTexCoord2f( m_chrome[ ptricmds[ 1 ] ][ 0 ] * s, m_chrome[ ptricmds[ 1 ] ][ 1 ] * t );
					}
					else
					{
						// FIX: put these in as integer coords, not floats
						glTexCoord2f( ptricmds[ 2 ] * s, ptricmds[ 3 ] * t );
					}

					if( texture.flags & STUDIO_NF_ADDITIVE )
					{
						glColor4f( 1.0f, 1.0f, 1.0f, m_pEntity->GetTransparency() );
					}
					else
					{
						const glm::vec3& lightVec = m_pvlightvalues[ ptricmds[ 1 ] ];
						glColor4f( lightVec[ 0 ], lightVec[ 1 ], lightVec[ 2 ], m_pEntity->GetTransparency() );
					}
				}

				glVertex3fv( glm::value_ptr( m_pxformverts[ ptricmds[ 0 ] ] ) );
			}
			glEnd();
		}

		if( texture.flags & STUDIO_NF_MASKED )
			glDisable( GL_ALPHA_TEST );
	}

	glDepthMask( GL_TRUE );

	return uiDrawnPolys;
}

void CStudioModelRenderer::Lighting( glm::vec3& lv, int bone, int flags, const glm::vec3& normal )
{
	const float ambient = std::max( 0.1f, ( float ) m_ambientlight / 255.0f ); // to avoid divison by zero
	const float shade = m_shadelight / 255.0f;
	glm::vec3 illum{ ambient };

	if( flags & STUDIO_NF_FLATSHADE )
	{
		VectorMA( illum, 0.8f, glm::vec3{ shade }, illum );
	}
	else
	{
		float	r, lightcos;

		lightcos = glm::dot( normal, m_blightvec[ bone ] ); // -1 colinear, 1 opposite

		if( lightcos > 1.0f ) lightcos = 1;

		illum += m_shadelight / 255.0f;

		r = m_flLambert;
		if( r < 1.0f ) r = 1.0f;
		lightcos = ( lightcos + ( r - 1.0f ) ) / r; // do modified hemispherical lighting
		if( lightcos > 0.0f ) VectorMA( illum, -lightcos, glm::vec3{ shade }, illum );

		if( illum[ 0 ] <= 0 ) illum[ 0 ] = 0;
		if( illum[ 1 ] <= 0 ) illum[ 1 ] = 0;
		if( illum[ 2 ] <= 0 ) illum[ 2 ] = 0;
	}

	float max = VectorMax( illum );

	if( max > 1.0f )
		lv = illum * ( 1.0f / max );
	else lv = illum;

	const glm::vec3 lightcolor{ m_lightcolor.GetRed() / 255.0f, m_lightcolor.GetGreen() / 255.0f, m_lightcolor.GetBlue() / 255.0f };

	lv *= lightcolor;

}


void CStudioModelRenderer::Chrome( glm::ivec2& chrome, int bone, const glm::vec3& normal )
{
	float n;

	if( m_chromeage[ bone ] != m_uiModelsDrawnCount )
	{
		// calculate vectors from the viewer to the bone. This roughly adjusts for position
		glm::vec3 chromeupvec;		// g_chrome t vector in world reference frame
		glm::vec3 chromerightvec;	// g_chrome s vector in world reference frame
		glm::vec3 tmp;				// vector pointing at bone in world reference frame

		tmp = m_vecViewerOrigin * -1.0f;

		tmp[ 0 ] += m_bonetransform[ bone ][ 0 ][ 3 ];
		tmp[ 1 ] += m_bonetransform[ bone ][ 1 ][ 3 ];
		tmp[ 2 ] += m_bonetransform[ bone ][ 2 ][ 3 ];

		VectorNormalize( tmp );
		chromeupvec = glm::cross( tmp, m_vecViewerRight );
		VectorNormalize( chromeupvec );
		chromerightvec = glm::cross( tmp, chromeupvec );
		VectorNormalize( chromerightvec );

		VectorIRotate( chromeupvec, m_bonetransform[ bone ], m_chromeup[ bone ] );
		VectorIRotate( chromerightvec, m_bonetransform[ bone ], m_chromeright[ bone ] );

		m_chromeage[ bone ] = m_uiModelsDrawnCount;
	}

	// calc s coord
	n = glm::dot( normal, m_chromeright[ bone ] );
	chrome[ 0 ] = ( n + 1.0 ) * 32; // FIX: make this a float

									// calc t coord
	n = glm::dot( normal, m_chromeup[ bone ] );
	chrome[ 1 ] = ( n + 1.0 ) * 32; // FIX: make this a float
}
}