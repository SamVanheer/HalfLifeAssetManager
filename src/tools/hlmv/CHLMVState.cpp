#include "shared/studiomodel/CStudioModel.h"

#include "game/entity/CBaseEntityList.h"

#include "CHLMVState.h"

namespace hlmv
{
const glm::vec3 CHLMVState::DEFAULT_ROTATION = glm::vec3( -90.0f, 0, -90.0f );

const float CHLMVState::DEFAULT_FOV = 65.0f;

const float CHLMVState::DEFAULT_FP_FOV = 74.0f;

CHLMVState::CHLMVState()
	: m_pEntity( nullptr )
{
	ResetToDefaults();
}

CHLMVState::~CHLMVState()
{
	ClearEntity();
}

void CHLMVState::ResetModelData()
{
	camera.SetOrigin( {} );
	camera.SetViewDirection( DEFAULT_ROTATION );

	weaponOriginCamera.SetOrigin( glm::vec3( 0 ) );
	weaponOriginCamera.SetViewDirection( glm::vec3( -90, 0, 90 ) );

	flFOV = DEFAULT_FOV;
	flFPFOV = DEFAULT_FP_FOV;

	texture = 0;

	iTextureXOffset = 0;
	iTextureYOffset = 0;

	pUVMesh = nullptr;
}

void CHLMVState::ResetToDefaults()
{
	ResetModelData();

	showTexture = false;

	textureScale = 1.0f;

	memset( backgroundTextureFile, 0, sizeof( backgroundTextureFile ) );
	memset( groundTextureFile, 0, sizeof( groundTextureFile ) );

	//There used to be an option called "useStencil" that was enabled along with mirror if the OpenGL driver was not 3DFX.
	//Considering how old that driver (and the hardware that used it) is, the code for it was removed.
	//The stencil buffer is now always used by mirror (it limits the mirroring effect to the floor quad).
	mirror = false;

	renderMode = RenderMode::TEXTURE_SHADED;

	showGround = false;

	pause = false;

	playSequence = true;

	drawnPolys = 0;

	wireframeOverlay = false;

	backfaceCulling = true;

	SetUseWeaponOrigin( false );

	showUVMap = false;

	overlayUVMap = false;

	antiAliasUVLines = false;

	drawAxes = false;
}

void CHLMVState::CenterView()
{
	if( !m_pEntity )
		return;

	glm::vec3 min, max;
	m_pEntity->ExtractBbox( min, max );

	float dx = max[ 0 ] - min[ 0 ];
	float dy = max[ 1 ] - min[ 1 ];
	float dz = max[ 2 ] - min[ 2 ];

	float d = dx;

	if( dy > d )
		d = dy;
	if( dz > d )
		d = dz;

	glm::vec3 trans;
	glm::vec3 rot;

	trans[ 2 ] = 0;
	trans[ 0 ] = -( min[ 2 ] + dz / 2 );
	trans[ 1 ] = d * 1.0f;
	rot[ 0 ] = -90.0f;
	rot[ 1 ] = 0.0f;
	rot[ 2 ] = -90.0f;

	camera.SetOrigin( trans );
	camera.SetViewDirection( rot );
}

void CHLMVState::SaveView()
{
	savedCamera = camera;
}

void CHLMVState::RestoreView()
{
	camera = savedCamera;
}

void CHLMVState::SetOrigin( const glm::vec3& vecOrigin )
{
	camera.SetOrigin( vecOrigin );
}

void CHLMVState::ClearEntity()
{
	SetEntity( nullptr );
}

void CHLMVState::SetEntity( CHLMVStudioModelEntity* pEntity )
{
	if( m_pEntity )
	{
		GetEntityList().Remove( m_pEntity );
		m_pEntity = nullptr;
	}

	if( pEntity )
		m_pEntity = pEntity;
}

void CHLMVState::SetUseWeaponOrigin( const bool bUse )
{
	useWeaponOrigin = bUse;

	pCurrentCamera = bUse ? &weaponOriginCamera : &camera;

	pCurrentFOV = bUse ? &flFPFOV : &flFOV;
}

bool CHLMVState::DumpModelInfo( const char* const pszFilename )
{
	if( !pszFilename || !( *pszFilename ) )
		return false;

	if( !m_pEntity || !m_pEntity->GetModel() )
		return false;

	if( FILE* pFile = fopen( pszFilename, "w" ) )
	{
		const auto pModel = m_pEntity->GetModel();

		const studiohdr_t* const pHdr = pModel->GetStudioHeader();

		const byte* const pByte = reinterpret_cast<const byte* const>( pHdr );

		const char* const pId = reinterpret_cast<const char* const>( &pHdr->id );

		fprintf( pFile, 
				 "ID Tag: %c%c%c%c\n"
				 "Version: %d\n"
				 "Name: \"%s\"\n"
				 "Length: %d\n\n", 
				 pId[ 0 ], pId[ 1 ], pId[ 2 ], pId[ 3 ], 
				 pHdr->version, 
				 pHdr->name, 
				 pHdr->length
		);

		fprintf( pFile,
				 "Eye Position: %.6f %.6f %.6f\n"
				 "Min: %.6f %.6f %.6f\n"
				 "Max: %.6f %.6f %.6f\n"
				 "Bounding Box Min: %.6f %.6f %.6f\n"
				 "Bounding Box Max: %.6f %.6f %.6f\n"
				 "Flags: %d\n\n",
				 pHdr->eyeposition[ 0 ], pHdr->eyeposition[ 1 ], pHdr->eyeposition[ 2 ],
				 pHdr->min[ 0 ], pHdr->min[ 1 ], pHdr->min[ 2 ],
				 pHdr->max[ 0 ], pHdr->max[ 1 ], pHdr->max[ 2 ],
				 pHdr->bbmin[ 0 ], pHdr->bbmin[ 1 ], pHdr->bbmin[ 2 ],
				 pHdr->bbmax[ 0 ], pHdr->bbmax[ 1 ], pHdr->bbmax[ 2 ],
				 pHdr->flags
		);

		fprintf( pFile,
				 "Number of Bones: %d\n\n",
				 pHdr->numbones
		);

		{
			const mstudiobone_t* pBone = reinterpret_cast<const mstudiobone_t*>( pByte + pHdr->boneindex );

			for( int iIndex = 0; iIndex < pHdr->numbones; ++iIndex, ++pBone )
			{
				fprintf( pFile,
						 "Bone %d Name: \"%s\"\n"
						 "Bone %d Parent: %d\n"
						 "Bone %d Flags: %d\n"
						 "Bone %d Bonecontroller: %d %d %d %d %d %d\n"
						 "Bone %d Value: %.6f %.6f %.6f %.6f %.6f %.6f\n"
						 "Bone %d Scale: %.6f %.6f %.6f %.6f %.6f %.6f\n\n",
						 iIndex + 1, pBone->name,
						 iIndex + 1, pBone->parent,
						 iIndex + 1, pBone->flags,
						 iIndex + 1, pBone->bonecontroller[ 0 ], pBone->bonecontroller[ 1 ], pBone->bonecontroller[ 2 ], 
										pBone->bonecontroller[ 3 ], pBone->bonecontroller[ 4 ], pBone->bonecontroller[ 5 ],
						 iIndex + 1, pBone->value[ 0 ], pBone->value[ 1 ], pBone->value[ 2 ], pBone->value[ 3 ], pBone->value[ 4 ], pBone->value[ 5 ],
						 iIndex + 1, pBone->scale[ 0 ], pBone->scale[ 1 ], pBone->scale[ 2 ], pBone->scale[ 3 ], pBone->scale[ 4 ], pBone->scale[ 5 ]
				);
			}
		}

		fprintf( pFile,
				 "Number of Bone Controllers: %d\n\n",
				 pHdr->numbonecontrollers
		);

		{
			const mstudiobonecontroller_t* pBoneC = reinterpret_cast<const mstudiobonecontroller_t*>( pByte + pHdr->bonecontrollerindex );

			for( int iIndex = 0; iIndex < pHdr->numbonecontrollers; ++iIndex, ++pBoneC )
			{
				fprintf( pFile,
						 "Bone Controller %d Bone: %d\n"
						 "Bone Controller %d Type: %d\n"
						 "Bone Controller %d Start: %f\n"
						 "Bone Controller %d End: %f\n"
						 "Bone Controller %d Rest: %d\n"
						 "Bone Controller %d Index: %d\n\n",
						 iIndex + 1, pBoneC->bone,
						 iIndex + 1, pBoneC->type,
						 iIndex + 1, pBoneC->start,
						 iIndex + 1, pBoneC->end,
						 iIndex + 1, pBoneC->rest,
						 iIndex + 1, pBoneC->index
				);
			}
		}

		fprintf( pFile,
				 "Number of Hitboxes: %d\n\n",
				 pHdr->numhitboxes
		);

		{
			const mstudiobbox_t* pHB = reinterpret_cast<const mstudiobbox_t*>( pByte + pHdr->hitboxindex );

			for( int iIndex = 0; iIndex < pHdr->numhitboxes; ++iIndex, ++pHB )
			{
				fprintf( pFile,
						 "Hitbox %d Bone: %d\n"
						 "Hitbox %d Group: %d\n"
						 "Hitbox %d Bounding Box Min: %.6f %.6f %.6f\n"
						 "Hitbox %d Bounding Box Max: %.6f %.6f %.6f\n\n",
						 iIndex + 1, pHB->bone,
						 iIndex + 1, pHB->group,
						 iIndex + 1, pHB->bbmin[ 0 ], pHB->bbmin[ 1 ], pHB->bbmin[ 2 ],
						 iIndex + 1, pHB->bbmax[ 0 ], pHB->bbmax[ 1 ], pHB->bbmax[ 2 ]
				);
			}
		}

		fprintf( pFile,
				 "Number of Sequences: %d\n\n",
				 pHdr->numseq
		);

		{
			const mstudioseqdesc_t* pSeq = reinterpret_cast<const mstudioseqdesc_t*>( pByte + pHdr->seqindex );

			for( int iIndex = 0; iIndex < pHdr->numseq; ++iIndex, ++pSeq )
			{
				fprintf( pFile,
						 "Sequence %d Label: \"%s\"\n"
						 "Sequence %d Frames per sec: %.6f\n"
						 "Sequence %d Flags: %d\n"
						 "Sequence %d Events: %d\n\n",
						 iIndex + 1, pSeq->label,
						 iIndex + 1, pSeq->fps,
						 iIndex + 1, pSeq->flags,
						 iIndex + 1, pSeq->numevents
				);

				const mstudioevent_t* pEvent = reinterpret_cast<const mstudioevent_t*>( pByte + pSeq->eventindex );

				for( int iEvent = 0; iEvent < pSeq->numevents; ++iEvent, ++pEvent )
				{
					fprintf( pFile,
							 "\tEvent %d Frame: %d\n"
							 "\tEvent %d Event: %d\n"
							 "\tEvent %d Options: %s\n"
							 "\tEvent %d Type: %d\n\n",
							 iEvent + 1, pEvent->frame,
							 iEvent + 1, pEvent->event,
							 iEvent + 1, pEvent->options,
							 iEvent + 1, pEvent->type
					);
				}
			}
		}

		fprintf( pFile,
				 "Number of Sequence Groups: %d\n\n",
				 pHdr->numseqgroups
		);

		{
			const mstudioseqgroup_t* pSG = reinterpret_cast<const mstudioseqgroup_t*>( pByte + pHdr->seqgroupindex );

			for( int iIndex = 0; iIndex < pHdr->numseqgroups; ++iIndex, ++pSG )
			{
				fprintf( pFile,
						 "Sequence Group %d Label: \"%s\"\n\n"
						 "Sequence Group %d Name: \"%s\"\n\n"
						 "Sequence Group %d Data: %d\n\n",
						 iIndex + 1, pSG->label,
						 iIndex + 1, pSG->name,
						 iIndex + 1, pSG->unused1
				);
			}
		}

		const studiohdr_t* const pTexHdr = pModel->GetTextureHeader();

		fprintf( pFile,
				 "Number of Textures: %d\n"
				 "Texture Index: %d\n"
				 "Texture Data Index: %d\n\n",
				 pTexHdr->numtextures,
				 pTexHdr->textureindex,
				 pTexHdr->texturedataindex
		);

		{
			const mstudiotexture_t* pTex = reinterpret_cast<const mstudiotexture_t*>( pByte + pTexHdr->textureindex );

			for( int iIndex = 0; iIndex < pTexHdr->numtextures; ++iIndex, ++pTex )
			{
				fprintf( pFile,
						 "Texture %d Name: \"%s\"\n"
						 "Texture %d Flags: %d\n"
						 "Texture %d Width: %d\n"
						 "Texture %d Height: %d\n"
						 "Texture %d Index: %d\n\n",
						 iIndex + 1, pTex->name,
						 iIndex + 1, pTex->flags,
						 iIndex + 1, pTex->width,
						 iIndex + 1, pTex->height,
						 iIndex + 1, pTex->index
				);
			}
		}

		fprintf( pFile,
				 "Number of Skin References: %d\n"
				 "Number of Skin Families: %d\n\n",
				 pHdr->numskinref,
				 pHdr->numskinfamilies
		);

		fprintf( pFile,
				 "Number of Body Parts: %d\n\n",
				 pHdr->numbodyparts
		);

		{
			const mstudiobodyparts_t* pBP = reinterpret_cast<const mstudiobodyparts_t*>( pByte + pHdr->bodypartindex );

			for( int iIndex = 0; iIndex < pHdr->numbodyparts; ++iIndex, ++pBP )
			{
				fprintf( pFile,
						 "Body Part %d Name: \"%s\"\n"
						 "Body Part %d Number of Models: %d\n"
						 "Body Part %d Base: %d\n"
						 "Body Part %d Model Index: %d\n\n",
						 iIndex + 1, pBP->name,
						 iIndex + 1, pBP->nummodels,
						 iIndex + 1, pBP->base,
						 iIndex + 1, pBP->modelindex
				);

				const mstudiomodel_t* pSubModel = reinterpret_cast<const mstudiomodel_t*>( pByte + pBP->modelindex );

				for( int iModel = 0; iModel < pBP->nummodels; ++iModel, ++pSubModel )
				{
					fprintf( pFile,
							 "\tSub Model %d Name: \"%s\"\n"
							 "\tSub Model %d Type: %d\n"
							 "\tSub Model %d Meshes: %d\n"
							 "\tSub Model %d Vertices: %d\n"
							 "\tSub Model %d Normals: %d\n"
							 "\tSub Model %d Deformation Groups: %d\n\n",
							 iModel + 1, pSubModel->name,
							 iModel + 1, pSubModel->type,
							 iModel + 1, pSubModel->nummesh,
							 iModel + 1, pSubModel->numverts,
							 iModel + 1, pSubModel->numnorms,
							 iModel + 1, pSubModel->numgroups
					);

					const mstudiomesh_t* pMesh = reinterpret_cast<const mstudiomesh_t*>( pByte + pSubModel->meshindex );

					for( int iMesh = 0; iMesh < pSubModel->nummesh; ++iMesh, ++pMesh )
					{
						fprintf( pFile,
								 "\t\tSub Model %d, Mesh %d Total Triangles: %d\n"
								 "\t\tSub Model %d, Mesh %d Triangle Index: %d\n"
								 "\t\tSub Model %d, Mesh %d Skin Reference: %d\n"
								 "\t\tSub Model %d, Mesh %d Total Normals: %d\n"
								 "\t\tSub Model %d, Mesh %d Normals Index: %d\n\n",
								 iModel + 1, iMesh + 1, pMesh->numtris,
								 iModel + 1, iMesh + 1, pMesh->triindex,
								 iModel + 1, iMesh + 1, pMesh->skinref,
								 iModel + 1, iMesh + 1, pMesh->numnorms,
								 iModel + 1, iMesh + 1, pMesh->normindex
						);
					}
				}
			}
		}

		fprintf( pFile,
				 "Number of Attachments: %d\n",
				 pHdr->numattachments
		);

		{
			const mstudioattachment_t* pAtt = reinterpret_cast<const mstudioattachment_t*>( pByte + pHdr->attachmentindex );

			for( int iIndex = 0; iIndex < pHdr->numattachments; ++iIndex, ++pAtt )
			{
				fprintf( pFile,
						 "Attachment %d Name: \"%s\"\n",
						 iIndex + 1, pAtt->name
				);
			}
		}

		fclose( pFile );

		return true;
	}

	return false;
}
}