#include <cassert>

#include "engine/shared/studiomodel/DumpModelInfo.hpp"
#include "engine/shared/studiomodel/StudioModel.hpp"

namespace studiomdl
{
void DumpModelInfo(FILE* file, const CStudioModel& model)
{
	assert(file);

	const studiohdr_t* const pHdr = model.GetStudioHeader();
	const auto pTextureHdr = model.GetTextureHeader();

	const byte* const pByte = reinterpret_cast<const byte* const>(pHdr);

	const char* const pId = reinterpret_cast<const char* const>(&pHdr->id);

	fprintf(file,
		"ID Tag: %c%c%c%c\n"
		"Version: %d\n"
		"Name: \"%s\"\n"
		"Length: %d\n\n",
		pId[0], pId[1], pId[2], pId[3],
		pHdr->version,
		pHdr->name,
		pHdr->length
	);

	fprintf(file,
		"Eye Position: %.6f %.6f %.6f\n"
		"Min: %.6f %.6f %.6f\n"
		"Max: %.6f %.6f %.6f\n"
		"Bounding Box Min: %.6f %.6f %.6f\n"
		"Bounding Box Max: %.6f %.6f %.6f\n"
		"Flags: %d\n\n",
		pHdr->eyeposition[0], pHdr->eyeposition[1], pHdr->eyeposition[2],
		pHdr->min[0], pHdr->min[1], pHdr->min[2],
		pHdr->max[0], pHdr->max[1], pHdr->max[2],
		pHdr->bbmin[0], pHdr->bbmin[1], pHdr->bbmin[2],
		pHdr->bbmax[0], pHdr->bbmax[1], pHdr->bbmax[2],
		pHdr->flags
	);

	fprintf(file,
		"Number of Bones: %d\n\n",
		pHdr->numbones
	);

	{
		const mstudiobone_t* pBone = reinterpret_cast<const mstudiobone_t*>(pByte + pHdr->boneindex);

		for (int iIndex = 0; iIndex < pHdr->numbones; ++iIndex, ++pBone)
		{
			fprintf(file,
				"Bone %d Name: \"%s\"\n"
				"Bone %d Parent: %d\n"
				"Bone %d Flags: %d\n"
				"Bone %d Bonecontroller: %d %d %d %d %d %d\n"
				"Bone %d Value: %.6f %.6f %.6f %.6f %.6f %.6f\n"
				"Bone %d Scale: %.6f %.6f %.6f %.6f %.6f %.6f\n\n",
				iIndex + 1, pBone->name,
				iIndex + 1, pBone->parent,
				iIndex + 1, pBone->flags,
				iIndex + 1, pBone->bonecontroller[0], pBone->bonecontroller[1], pBone->bonecontroller[2],
				pBone->bonecontroller[3], pBone->bonecontroller[4], pBone->bonecontroller[5],
				iIndex + 1, pBone->value[0], pBone->value[1], pBone->value[2], pBone->value[3], pBone->value[4], pBone->value[5],
				iIndex + 1, pBone->scale[0], pBone->scale[1], pBone->scale[2], pBone->scale[3], pBone->scale[4], pBone->scale[5]
			);
		}
	}

	fprintf(file,
		"Number of Bone Controllers: %d\n\n",
		pHdr->numbonecontrollers
	);

	{
		const mstudiobonecontroller_t* pBoneC = reinterpret_cast<const mstudiobonecontroller_t*>(pByte + pHdr->bonecontrollerindex);

		for (int iIndex = 0; iIndex < pHdr->numbonecontrollers; ++iIndex, ++pBoneC)
		{
			fprintf(file,
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

	fprintf(file,
		"Number of Hitboxes: %d\n\n",
		pHdr->numhitboxes
	);

	{
		const mstudiobbox_t* pHB = reinterpret_cast<const mstudiobbox_t*>(pByte + pHdr->hitboxindex);

		for (int iIndex = 0; iIndex < pHdr->numhitboxes; ++iIndex, ++pHB)
		{
			fprintf(file,
				"Hitbox %d Bone: %d\n"
				"Hitbox %d Group: %d\n"
				"Hitbox %d Bounding Box Min: %.6f %.6f %.6f\n"
				"Hitbox %d Bounding Box Max: %.6f %.6f %.6f\n\n",
				iIndex + 1, pHB->bone,
				iIndex + 1, pHB->group,
				iIndex + 1, pHB->bbmin[0], pHB->bbmin[1], pHB->bbmin[2],
				iIndex + 1, pHB->bbmax[0], pHB->bbmax[1], pHB->bbmax[2]
			);
		}
	}

	fprintf(file,
		"Number of Sequences: %d\n\n",
		pHdr->numseq
	);

	{
		const mstudioseqdesc_t* pSeq = reinterpret_cast<const mstudioseqdesc_t*>(pByte + pHdr->seqindex);

		for (int iIndex = 0; iIndex < pHdr->numseq; ++iIndex, ++pSeq)
		{
			fprintf(file,
				"Sequence %d Label: \"%s\"\n"
				"Sequence %d Frames per sec: %.6f\n"
				"Sequence %d Flags: %d\n"
				"Sequence %d Events: %d\n\n",
				iIndex + 1, pSeq->label,
				iIndex + 1, pSeq->fps,
				iIndex + 1, pSeq->flags,
				iIndex + 1, pSeq->numevents
			);

			const mstudioevent_t* pEvent = reinterpret_cast<const mstudioevent_t*>(pByte + pSeq->eventindex);

			for (int iEvent = 0; iEvent < pSeq->numevents; ++iEvent, ++pEvent)
			{
				fprintf(file,
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

	fprintf(file,
		"Number of Sequence Groups: %d\n\n",
		pHdr->numseqgroups
	);

	{
		const mstudioseqgroup_t* pSG = reinterpret_cast<const mstudioseqgroup_t*>(pByte + pHdr->seqgroupindex);

		for (int iIndex = 0; iIndex < pHdr->numseqgroups; ++iIndex, ++pSG)
		{
			fprintf(file,
				"Sequence Group %d Label: \"%s\"\n\n"
				"Sequence Group %d Name: \"%s\"\n\n"
				"Sequence Group %d Data: %d\n\n",
				iIndex + 1, pSG->label,
				iIndex + 1, pSG->name,
				iIndex + 1, pSG->unused1
			);
		}
	}

	fprintf(file,
		"Number of Textures: %d\n"
		"Texture Index: %d\n"
		"Texture Data Index: %d\n\n",
		pTextureHdr->numtextures,
		pTextureHdr->textureindex,
		pTextureHdr->texturedataindex
	);

	{
		const mstudiotexture_t* pTex = reinterpret_cast<const mstudiotexture_t*>(reinterpret_cast<const byte*>(pTextureHdr) + pTextureHdr->textureindex);

		for (int iIndex = 0; iIndex < pTextureHdr->numtextures; ++iIndex, ++pTex)
		{
			fprintf(file,
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

	fprintf(file,
		"Number of Skin References: %d\n"
		"Number of Skin Families: %d\n\n",
		pTextureHdr->numskinref,
		pTextureHdr->numskinfamilies
	);

	fprintf(file,
		"Number of Body Parts: %d\n\n",
		pHdr->numbodyparts
	);

	{
		const mstudiobodyparts_t* pBP = reinterpret_cast<const mstudiobodyparts_t*>(pByte + pHdr->bodypartindex);

		for (int iIndex = 0; iIndex < pHdr->numbodyparts; ++iIndex, ++pBP)
		{
			fprintf(file,
				"Body Part %d Name: \"%s\"\n"
				"Body Part %d Number of Models: %d\n"
				"Body Part %d Base: %d\n"
				"Body Part %d Model Index: %d\n\n",
				iIndex + 1, pBP->name,
				iIndex + 1, pBP->nummodels,
				iIndex + 1, pBP->base,
				iIndex + 1, pBP->modelindex
			);

			const mstudiomodel_t* pSubModel = reinterpret_cast<const mstudiomodel_t*>(pByte + pBP->modelindex);

			for (int iModel = 0; iModel < pBP->nummodels; ++iModel, ++pSubModel)
			{
				fprintf(file,
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

				const mstudiomesh_t* pMesh = reinterpret_cast<const mstudiomesh_t*>(pByte + pSubModel->meshindex);

				for (int iMesh = 0; iMesh < pSubModel->nummesh; ++iMesh, ++pMesh)
				{
					fprintf(file,
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

	fprintf(file,
		"Number of Attachments: %d\n",
		pHdr->numattachments
	);

	{
		const mstudioattachment_t* pAtt = reinterpret_cast<const mstudioattachment_t*>(pByte + pHdr->attachmentindex);

		for (int iIndex = 0; iIndex < pHdr->numattachments; ++iIndex, ++pAtt)
		{
			fprintf(file,
				"Attachment %d Name: \"%s\"\n",
				iIndex + 1, pAtt->name
			);
		}
	}
}
}
