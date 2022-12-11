/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#pragma once

#include <cstddef>
#include <cstdint>

#include <glm/vec3.hpp>

/*
==============================================================================

STUDIO MODELS

Studio models are position independent, so the cache manager can move them.
==============================================================================
*/

enum
{
	MAXSTUDIOTRIANGLES = 20000,	// TODO: tune this
	MAXSTUDIOVERTS = 2048,	// TODO: tune this
	MAXSTUDIOSEQUENCES = 2048,	// total animation sequences -- KSH incremented
	MAXSTUDIOSKINS = 100,		// total textures
	MAXSTUDIOSRCBONES = 512,		// bones allowed at source movement
	MAXSTUDIOBONES = 128,		// total bones actually used
	MAXSTUDIOMODELS = 32,		// sub-models per model
	MAXSTUDIOBODYPARTS = 32,
	MAXSTUDIOGROUPS = 16,
	MAXSTUDIOANIMATIONS = 2048,
	MAXSTUDIOMESHES = 256,
	MAXSTUDIOEVENTS = 1024,
	MAXSTUDIOPIVOTS = 256,
	MAXSTUDIOCONTROLLERS = 8,
	STUDIO_VERSION = 10,

	/**
	*	Maximum length for the event options string.
	*/
	STUDIO_MAX_EVENT_OPTIONS_LENGTH = 64,

	/**
	*	@brief Number of axes that coordinate data operates on
	*
	*	3 linear axes and 3 rotation axes
	*/
	STUDIO_NUM_COORDINATE_AXES = 6,

	STUDIO_ATTACH_NUM_VECTORS = 3
};

/**
*	@brief Number of blends that a sequence can have
*	X and Y axis
*/
constexpr int SequenceBlendCount{2};
constexpr int ControllerCount{4};

constexpr int SequenceBlendXIndex{0};
constexpr int SequenceBlendYIndex{1};

constexpr int MaxBoneNameBytes{32};
constexpr int MaxAttachmentNameBytes{32};
constexpr int MaxTextureNameBytes{64};
constexpr int MaxModelNameBytes{64};

/**
*	@brief studiohdr_t::flags values
*	@see studiohdr_t::flags
*/
enum
{
	EF_ROCKET = 1,			//! leave a trail
	EF_GRENADE = 2,			//! leave a trail
	EF_GIB = 4,			//! leave a trail
	EF_ROTATE = 8,			//! rotate (bonus items)
	EF_TRACER = 16,			//! green split trail
	EF_ZOMGIB = 32,			//! small blood trail
	EF_TRACER2 = 64,			//! orange split trail + rotate
	EF_TRACER3 = 128,		//! purple trail
	EF_NOSHADELIGHT = 256,		//! No shade lighting
	EF_HITBOXCOLLISIONS = 512,		//! Use hitbox collisions
	EF_FORCESKYLIGHT = 1024,		//! Forces the model to be lit by skybox lighting
};

// header for demand loaded sequence group data
struct studioseqhdr_t
{
	int		id;
	int		version;

	char	name[64];
	int		length;
};

// bones
struct mstudiobone_t
{
	char	name[MaxBoneNameBytes];	// bone name for symbolic links
	int		parent;		// parent bone
	int		flags;		// ??
	int		bonecontroller[STUDIO_NUM_COORDINATE_AXES];	// bone controller index, -1 == none
	float	value[STUDIO_NUM_COORDINATE_AXES];	// default DoF values
	float	scale[STUDIO_NUM_COORDINATE_AXES];   // scale for delta DoF values
};


// bone controllers
struct mstudiobonecontroller_t
{
	int		bone;	// -1 == 0
	int		type;	// X, Y, Z, XR, YR, ZR, M
	float	start;
	float	end;
	int		rest;	// byte index value at rest
	int		index;	// 0-3 user set controller, 4 mouth
};

// intersection boxes
struct mstudiobbox_t
{
	int			bone;
	int			group;			// intersection group
	glm::vec3	bbmin;		// bounding box
	glm::vec3	bbmax;
};

#if !defined( CACHE_USER ) && !defined( QUAKEDEF_H )
#define CACHE_USER
struct cache_user_t
{
	void* data;
};
#endif

//
// demand loaded sequence groups
//
struct mstudioseqgroup_t
{
	char	label[32];	// textual name
	char	name[64];	// file name
	int		unused1;    // was "cache"  - index pointer
	int		unused2;    // was "data" -  hack for group 0
};

// sequence descriptions
struct mstudioseqdesc_t
{
	char		label[32];	// sequence label

	float		fps;		// frames per second	
	int			flags;		// looping/non-looping flags

	int			activity;
	int			actweight;

	int			numevents;
	int			eventindex;

	int			numframes;	// number of frames per sequence

	int			numpivots;	// number of foot pivots
	int			pivotindex;

	int			motiontype;
	int			motionbone;
	glm::vec3	linearmovement;
	int			automoveposindex;
	int			automoveangleindex;

	glm::vec3	bbmin;		// per sequence bounding box
	glm::vec3	bbmax;

	int			numblends;
	int			animindex;		// mstudioanim_t pointer relative to start of sequence group data
								// [blend][bone][X, Y, Z, XR, YR, ZR]

	int			blendtype[SequenceBlendCount];	// X, Y, Z, XR, YR, ZR
	float		blendstart[SequenceBlendCount];	// starting value
	float		blendend[SequenceBlendCount];	// ending value
	int			blendparent;

	int			seqgroup;		// sequence group for demand loading

	int			entrynode;		// transition node at entry
	int			exitnode;		// transition node at exit
	int			nodeflags;		// transition rules

	int			nextseq;		// auto advancing sequences
};

// events
struct mstudioevent_t
{
	int 	frame;
	int		event;
	int		type;
	char	options[STUDIO_MAX_EVENT_OPTIONS_LENGTH];
};

// pivots
struct mstudiopivot_t
{
	glm::vec3		org;	// pivot point
	int			start;
	int			end;
};

// attachment
struct mstudioattachment_t
{
	/**
	*	Name of this attachment. Unused in GoldSource.
	*/
	char name[MaxAttachmentNameBytes];

	/**
	*	Type of this attachment. Unused in GoldSource;
	*/
	int type;

	/**
	*	Index of the bone this is attached to.
	*/
	int bone;

	/**
	*	Offset from bone origin.
	*/
	glm::vec3 org;

	/**
	*	Directional vectors? Unused in GoldSource.
	*/
	glm::vec3 vectors[STUDIO_ATTACH_NUM_VECTORS];
};

struct mstudioanim_t
{
	unsigned short	offset[STUDIO_NUM_COORDINATE_AXES];
};

// animation frames
union mstudioanimvalue_t
{
	struct
	{
		std::uint8_t valid;
		std::uint8_t total;
	} num;
	short		value;
};

// body part index
struct mstudiobodyparts_t
{
	char	name[64];
	int		nummodels;
	int		base;
	int		modelindex; // index into models array
};

// skin info
struct mstudiotexture_t
{
	char	name[MaxTextureNameBytes];
	int		flags;
	int		width;
	int		height;
	int		index;
};


// skin families
// short	index[skinfamilies][skinref]

// studio models
struct mstudiomodel_t
{
	char	name[MaxModelNameBytes];

	int		type;

	float	boundingradius;

	int		nummesh;
	int		meshindex;

	int		numverts;		// number of unique vertices
	int		vertinfoindex;	// vertex bone info
	int		vertindex;		// vertex glm::vec3
	int		numnorms;		// number of unique surface normals
	int		norminfoindex;	// normal bone info
	int		normindex;		// normal glm::vec3

	int		numgroups;		// deformation groups
	int		groupindex;
};


// glm::vec3	boundingbox[model][bone][2];	// complex intersection info


// meshes
struct mstudiomesh_t
{
	int		numtris;
	int		triindex;
	int		skinref;
	int		numnorms;		// per mesh normals
	int		normindex;		// normal glm::vec3
};

// triangles
#if 0
struct mstudiotrivert_t
{
	short	vertindex;		// index into vertex array
	short	normindex;		// index into normal array
	short	s, t;			// s,t position on skin
};
#endif

struct studiohdr_t
{
	int			id;
	int			version;

	char		name[64];
	int			length;

	glm::vec3	eyeposition;	// ideal eye position
	glm::vec3	min;			// ideal movement hull size
	glm::vec3	max;

	glm::vec3	bbmin;			// clipping bounding box
	glm::vec3	bbmax;

	int			flags;

	const	std::byte* GetData() const { return reinterpret_cast<const std::byte*>(this); }
	std::byte* GetData() { return reinterpret_cast<std::byte*>(this); }

	int			numbones;			// bones
	int			boneindex;

	const	mstudiobone_t* GetBones() const { return reinterpret_cast<const mstudiobone_t*>(GetData() + boneindex); }
	mstudiobone_t* GetBones() { return reinterpret_cast<mstudiobone_t*>(GetData() + boneindex); }

	const	mstudiobone_t* GetBone(const int iIndex) const { return GetBones() + iIndex; }
	mstudiobone_t* GetBone(const int iIndex) { return GetBones() + iIndex; }

	int			numbonecontrollers;		// bone controllers
	int			bonecontrollerindex;

	const	mstudiobonecontroller_t* GetBoneControllers() const { return reinterpret_cast<const mstudiobonecontroller_t*>(GetData() + bonecontrollerindex); }
	mstudiobonecontroller_t* GetBoneControllers() { return reinterpret_cast<mstudiobonecontroller_t*>(GetData() + bonecontrollerindex); }

	const	mstudiobonecontroller_t* GetBoneController(const int iIndex) const { return GetBoneControllers() + iIndex; }
	mstudiobonecontroller_t* GetBoneController(const int iIndex) { return GetBoneControllers() + iIndex; }

	int			numhitboxes;			// complex bounding boxes
	int			hitboxindex;

	const	mstudiobbox_t* GetHitBoxes() const { return reinterpret_cast<const mstudiobbox_t*>(GetData() + hitboxindex); }
	mstudiobbox_t* GetHitBoxes() { return reinterpret_cast<mstudiobbox_t*>(GetData() + hitboxindex); }

	const	mstudiobbox_t* GetHitBox(const int iIndex) const { return GetHitBoxes() + iIndex; }
	mstudiobbox_t* GetHitBox(const int iIndex) { return GetHitBoxes() + iIndex; }

	int			numseq;				// animation sequences
	int			seqindex;

	const	mstudioseqdesc_t* GetSequences() const { return reinterpret_cast<const mstudioseqdesc_t*>(GetData() + seqindex); }
	mstudioseqdesc_t* GetSequences() { return reinterpret_cast<mstudioseqdesc_t*>(GetData() + seqindex); }

	const	mstudioseqdesc_t* GetSequence(const int iIndex) const { return GetSequences() + iIndex; }
	mstudioseqdesc_t* GetSequence(const int iIndex) { return GetSequences() + iIndex; }

	int			numseqgroups;		// demand loaded sequences
	int			seqgroupindex;

	const	mstudioseqgroup_t* GetSequenceGroups() const { return reinterpret_cast<const mstudioseqgroup_t*>(GetData() + seqgroupindex); }
	mstudioseqgroup_t* GetSequenceGroups() { return reinterpret_cast<mstudioseqgroup_t*>(GetData() + seqgroupindex); }

	const	mstudioseqgroup_t* GetSequenceGroup(const int iIndex) const { return GetSequenceGroups() + iIndex; }
	mstudioseqgroup_t* GetSequenceGroup(const int iIndex) { return GetSequenceGroups() + iIndex; }

	int			numtextures;		// raw textures
	int			textureindex;
	int			texturedataindex;

	const	mstudiotexture_t* GetTextures() const { return reinterpret_cast<const mstudiotexture_t*>(GetData() + textureindex); }
	mstudiotexture_t* GetTextures() { return reinterpret_cast<mstudiotexture_t*>(GetData() + textureindex); }

	const	mstudiotexture_t* GetTexture(const int iIndex) const { return GetTextures() + iIndex; }
	mstudiotexture_t* GetTexture(const int iIndex) { return GetTextures() + iIndex; }

	int			numskinref;			// replaceable textures
	int			numskinfamilies;
	int			skinindex;

	const	short* GetSkins() const { return reinterpret_cast<const short*>(GetData() + skinindex); }
	short* GetSkins() { return reinterpret_cast<short*>(GetData() + skinindex); }

	const	short* GetSkin(const int iIndex) const { return GetSkins() + iIndex; }
	short* GetSkin(const int iIndex) { return GetSkins() + iIndex; }

	int			numbodyparts;
	int			bodypartindex;

	const	mstudiobodyparts_t* GetBodyparts() const { return reinterpret_cast<const mstudiobodyparts_t*>(GetData() + bodypartindex); }
	mstudiobodyparts_t* GetBodyparts() { return reinterpret_cast<mstudiobodyparts_t*>(GetData() + bodypartindex); }

	const	mstudiobodyparts_t* GetBodypart(const int iIndex) const { return GetBodyparts() + iIndex; }
	mstudiobodyparts_t* GetBodypart(const int iIndex) { return GetBodyparts() + iIndex; }

	int			numattachments;		// queryable attachable points
	int			attachmentindex;

	const	mstudioattachment_t* GetAttachments() const { return reinterpret_cast<const mstudioattachment_t*>(GetData() + attachmentindex); }
	mstudioattachment_t* GetAttachments() { return reinterpret_cast<mstudioattachment_t*>(GetData() + attachmentindex); }

	const	mstudioattachment_t* GetAttachment(const int iIndex) const { return GetAttachments() + iIndex; }
	mstudioattachment_t* GetAttachment(const int iIndex) { return GetAttachments() + iIndex; }

	//This seems to be obsolete. Probably replaced by events that reference external sounds?
	int			soundtable;
	int			soundindex;
	int			soundgroups;
	int			soundgroupindex;

	int			numtransitions;		// animation node to animation node transition graph
	int			transitionindex;

	const	std::uint8_t* GetTransitions() const { return reinterpret_cast<const std::uint8_t*>(GetData() + attachmentindex); }
	std::uint8_t* GetTransitions() { return reinterpret_cast<std::uint8_t*>(GetData() + attachmentindex); }

	const	std::uint8_t* GetTransition(const int iIndex) const { return GetTransitions() + iIndex; }
	std::uint8_t* GetTransition(const int iIndex) { return GetTransitions() + iIndex; }
};

// lighting options
enum
{
	STUDIO_NF_FLATSHADE = 0x0001,
	STUDIO_NF_CHROME = 0x0002,
	STUDIO_NF_FULLBRIGHT = 0x0004,
	STUDIO_NF_NOMIPS = 0x0008,
	STUDIO_NF_ALPHA = 0x0010,
	STUDIO_NF_ADDITIVE = 0x0020,
	STUDIO_NF_MASKED = 0x0040,

	STUDIO_NF_RENDER_FLAGS = STUDIO_NF_CHROME | STUDIO_NF_ADDITIVE | STUDIO_NF_MASKED | STUDIO_NF_FULLBRIGHT
};

// motion flags
enum
{
	STUDIO_X = 0x0001,
	STUDIO_Y = 0x0002,
	STUDIO_Z = 0x0004,
	STUDIO_XR = 0x0008,
	STUDIO_YR = 0x0010,
	STUDIO_ZR = 0x0020,
	STUDIO_LX = 0x0040,
	STUDIO_LY = 0x0080,
	STUDIO_LZ = 0x0100,
	STUDIO_AX = 0x0200,
	STUDIO_AY = 0x0400,
	STUDIO_AZ = 0x0800,
	STUDIO_AXR = 0x1000,
	STUDIO_AYR = 0x2000,
	STUDIO_AZR = 0x4000,
	STUDIO_BONECONTROLLER_TYPES = STUDIO_X | STUDIO_Y | STUDIO_Z | STUDIO_XR | STUDIO_YR | STUDIO_ZR,
	STUDIO_TYPES = 0x7FFF,
	STUDIO_CONTROL_FIRST = STUDIO_X,
	STUDIO_CONTROL_LAST = STUDIO_AZR,
	STUDIO_RLOOP = 0x8000	// controller that wraps shortest distance
};

// sequence flags
enum
{
	STUDIO_LOOPING = 0x0001
};

// bone flags
enum
{
	STUDIO_HAS_NORMALS = 0x0001,
	STUDIO_HAS_VERTICES = 0x0002,
	STUDIO_HAS_BBOX = 0x0004,
	STUDIO_HAS_CHROME = 0x0008	// if any of the textures have chrome on them
};

//Constants
enum
{
	STUDIO_MAX_CONTROLLERS = 4,
	STUDIO_MOUTH_CONTROLLER = 4,
	STUDIO_TOTAL_CONTROLLERS = 5,
	STUDIO_MAX_BLENDERS = 2
};

#define RAD_TO_STUDIO		(32768.0/M_PI)
#define STUDIO_TO_RAD		(M_PI/32768.0)

/**
*	Id for studio headers (main and texture headers)
*/
#define STUDIOMDL_HDR_ID "IDST"

/**
*	Id for studio sequence groups
*/
#define STUDIOMDL_SEQ_ID "IDSQ"
