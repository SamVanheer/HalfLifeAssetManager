#include <cassert>
#include <cctype>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

#include "shared/Platform.h"
#include "shared/Logging.h"

#include "utility/IOUtils.h"
#include "utility/StringUtils.h"

#include "cvar/CCVar.h"

#include "graphics/GraphicsUtils.h"
#include "graphics/Palette.h"

#include "CStudioModel.h"

namespace studiomdl
{
namespace
{
//Note: multiple libraries can include this file and define this cvar. The first library to register theirs wins.
//All others will point to that one.
static cvar::CCVar r_filtertextures("r_filtertextures", cvar::CCVarArgsBuilder().FloatValue(1).HelpInfo("Whether to filter textures or not"));

static cvar::CCVar r_powerof2textures("r_powerof2textures",
	cvar::CCVarArgsBuilder()
	.Flags(cvar::Flag::ARCHIVE)
	.FloatValue(1)
	.MinValue(0)
	.MaxValue(1)
	.HelpInfo("Whether to resize textures to power of 2 dimensions"));

void UploadRGBATexture(const int iWidth, const int iHeight, const byte* pData, GLuint textureId, const bool bFilterTextures)
{
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iWidth, iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bFilterTextures ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bFilterTextures ? GL_LINEAR : GL_NEAREST);
}

//Dol differs only in texture storage
//Instead of pixels followed by RGB palette, it has a 32 byte texture name (name of file without extension), followed by an RGBA palette and pixels
void ConvertDolToMdl(byte* pBuffer, const mstudiotexture_t& texture)
{
	const int RGBA_PALETTE_CHANNELS = 4;

	auto palette = std::make_unique<byte[]>(PALETTE_SIZE);

	//Starts off with 32 byte texture name
	auto pSourcePalette = pBuffer + texture.index + 32;

	//Discard alpha value
	//TODO: convert alpha value somehow? is it even used?
	for (int i = 0; i < PALETTE_ENTRIES; ++i)
	{
		for (int j = 0; j < PALETTE_CHANNELS; ++j)
		{
			palette[i * PALETTE_CHANNELS + j] = pSourcePalette[i * RGBA_PALETTE_CHANNELS + j];
		}
	}

	const auto size = texture.width * texture.height;

	auto pixels = std::make_unique<byte[]>(size);

	auto pSourcePixels = pSourcePalette + PALETTE_ENTRIES * RGBA_PALETTE_CHANNELS;

	for (int i = 0; i < size; ++i)
	{
		auto pixel = pSourcePixels[i];

		auto masked = pixel & 0x1F;

		//Adjust the index to map to the correct palette entry
		if (masked >= 8)
		{
			if (masked >= 16)
			{
				if (masked < 24)
				{
					pixel -= 8;
				}
			}
			else
			{
				pixel += 8;
			}
		}

		pixels[i] = pixel;
	}

	//Now write the correct data to the model buffer
	auto pDestPixels = pBuffer + texture.index;

	for (int i = 0; i < size; ++i)
	{
		pDestPixels[i] = pixels[i];
	}

	auto pDestPalette = pBuffer + texture.index + texture.width * texture.height;

	memcpy(pDestPalette, palette.get(), PALETTE_SIZE);

	//Some data will be left dangling after the palette and before the next texture/end of file. Nothing will reference it though
	//in the SL version this will not be a problem since the file isn't loaded in one chunk
}

void UploadTexture(const mstudiotexture_t* ptexture, const byte* data, byte* pal, GLuint name, const bool bFilterTextures, const bool bPowerOf2)
{
	// unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight;
	int		i, j;
	int		row1[MAX_TEXTURE_DIMS], row2[MAX_TEXTURE_DIMS], col1[MAX_TEXTURE_DIMS], col2[MAX_TEXTURE_DIMS];
	const byte* pix1, * pix2, * pix3, * pix4;

	// convert texture to power of 2
	int outwidth;
	int outheight;

	if (bPowerOf2)
	{
		if (!graphics::CalculateImageDimensions(ptexture->width, ptexture->height, outwidth, outheight))
			return;
	}
	else
	{
		outwidth = ptexture->width;
		outheight = ptexture->height;
	}

	const size_t uiSize = outwidth * outheight * 4;

	//Needs at least one pixel (satisfies code analysis)
	if (uiSize < 4)
		return;

	auto tex = std::make_unique<byte[]>(uiSize);

	/*
	int k = 0;
	for (i = 0; i < ptexture->height; i++)
	{
	for (j = 0; j < ptexture->width; j++)
	{

	in[k++] = pal[data[i * ptexture->width + j] * 3 + 0];
	in[k++] = pal[data[i * ptexture->width + j] * 3 + 1];
	in[k++] = pal[data[i * ptexture->width + j] * 3 + 2];
	in[k++] = 0xff;;
	}
	}

	gluScaleImage (GL_RGBA, ptexture->width, ptexture->height, GL_UNSIGNED_BYTE, in, outwidth, outheight, GL_UNSIGNED_BYTE, out);
	free (in);
	*/

	for (i = 0; i < outwidth; i++)
	{
		col1[i] = (int) ((i + 0.25) * (ptexture->width / (float) outwidth));
		col2[i] = (int) ((i + 0.75) * (ptexture->width / (float) outwidth));
	}

	for (i = 0; i < outheight; i++)
	{
		row1[i] = (int) ((i + 0.25) * (ptexture->height / (float) outheight)) * ptexture->width;
		row2[i] = (int) ((i + 0.75) * (ptexture->height / (float) outheight)) * ptexture->width;
	}

	const byte* const pAlpha = &pal[PALETTE_ALPHA_INDEX];

	//This modifies the model's data. Sets the mask color to black. This is also done by Jed's model viewer. (export texture has black)
	if (ptexture->flags & STUDIO_NF_MASKED)
	{
		pal[255 * 3 + 0] = pal[255 * 3 + 1] = pal[255 * 3 + 2] = 0;
	}

	auto out = tex.get();

	// scale down and convert to 32bit RGB
	for (i = 0; i < outheight; i++)
	{
		for (j = 0; j < outwidth; j++, out += 4)
		{
			pix1 = &pal[data[row1[i] + col1[j]] * 3];
			pix2 = &pal[data[row1[i] + col2[j]] * 3];
			pix3 = &pal[data[row2[i] + col1[j]] * 3];
			pix4 = &pal[data[row2[i] + col2[j]] * 3];

			out[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0]) >> 2;
			out[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1]) >> 2;
			out[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2]) >> 2;

			if (ptexture->flags & STUDIO_NF_MASKED && pix1 == pAlpha && pix2 == pAlpha && pix3 == pAlpha && pix4 == pAlpha)
			{
				//Set alpha to 0 to enable transparent pixel.
				out[3] = 0x00;
			}
			else
			{
				out[3] = 0xFF;
			}
		}
	}

	UploadRGBATexture(outwidth, outheight, tex.get(), name, bFilterTextures);
}

size_t UploadTextures(studiohdr_t& textureHdr, std::vector<GLuint>& textures, const bool bFilterTextures, const bool bPowerOf2, const bool bIsDol)
{
	size_t uiNumTextures = 0;

	if (textureHdr.textureindex > 0 && textureHdr.numtextures <= CStudioModel::MAX_TEXTURES)
	{
		mstudiotexture_t* ptexture = textureHdr.GetTextures();

		byte* pIn = reinterpret_cast<byte*>(&textureHdr);

		const int n = textureHdr.numtextures;

		for (int i = 0; i < n; ++i)
		{
			GLuint name;

			glBindTexture(GL_TEXTURE_2D, 0);
			glGenTextures(1, &name);

			if (bIsDol)
			{
				ConvertDolToMdl(pIn, ptexture[i]);
			}

			UploadTexture(&ptexture[i], pIn + ptexture[i].index, pIn + ptexture[i].width * ptexture[i].height + ptexture[i].index, name, bFilterTextures, bPowerOf2);

			textures.emplace_back(name);
		}

		uiNumTextures = n;
	}

	return uiNumTextures;
}
}

CStudioModel::CStudioModel(std::string&& fileName, studio_ptr<studiohdr_t>&& pStudioHdr, studio_ptr<studiohdr_t>&& pTextureHdr,
	std::vector<studio_ptr<studioseqhdr_t>>&& sequenceHeaders, bool isDol)
	: m_FileName(std::move(fileName))
	, m_pStudioHdr(std::move(pStudioHdr))
	, m_pTextureHdr(std::move(pTextureHdr))
	, m_SequenceHeaders(std::move(sequenceHeaders))
	, m_IsDol(isDol)
{
	assert(m_pStudioHdr);
}

CStudioModel::~CStudioModel()
{
	glDeleteTextures(GetTextureHeader()->numtextures, m_Textures.data());
	m_Textures.clear();
}

mstudioanim_t* CStudioModel::GetAnim(mstudioseqdesc_t* pseqdesc) const
{
	mstudioseqgroup_t* pseqgroup = m_pStudioHdr->GetSequenceGroup(pseqdesc->seqgroup);

	if (pseqdesc->seqgroup == 0)
	{
		return (mstudioanim_t*) ((byte*) m_pStudioHdr.get() + pseqgroup->unused2 + pseqdesc->animindex);
	}

	return (mstudioanim_t*) ((byte*) m_SequenceHeaders[pseqdesc->seqgroup - 1].get() + pseqdesc->animindex);
}

mstudiomodel_t* CStudioModel::GetModelByBodyPart(const int iBody, const int iBodyPart) const
{
	mstudiobodyparts_t* pbodypart = m_pStudioHdr->GetBodypart(iBodyPart);

	int index = iBody / pbodypart->base;
	index = index % pbodypart->nummodels;

	return (mstudiomodel_t*) ((byte*) m_pStudioHdr.get() + pbodypart->modelindex) + index;
}

int CStudioModel::GetBodyValueForGroup(int compositeValue, int group) const
{
	if (group >= m_pStudioHdr->numbodyparts)
	{
		return -1;
	}

	const mstudiobodyparts_t* const bodyPart = m_pStudioHdr->GetBodypart(group);

	return (compositeValue / bodyPart->base) % bodyPart->nummodels;
}

bool CStudioModel::CalculateBodygroup(const int iGroup, const int iValue, int& iInOutBodygroup) const
{
	if (iGroup > m_pStudioHdr->numbodyparts)
		return false;

	const mstudiobodyparts_t* const pbodypart = m_pStudioHdr->GetBodypart(iGroup);

	int iCurrent = (iInOutBodygroup / pbodypart->base) % pbodypart->nummodels;

	if (iValue >= pbodypart->nummodels)
		return true;

	iInOutBodygroup = (iInOutBodygroup - (iCurrent * pbodypart->base) + (iValue * pbodypart->base));

	return true;
}

GLuint CStudioModel::GetTextureId(const int iIndex) const
{
	const studiohdr_t* const pHdr = GetTextureHeader();

	if (!pHdr)
		return GL_INVALID_TEXTURE_ID;

	if (iIndex < 0 || iIndex >= pHdr->numtextures)
		return GL_INVALID_TEXTURE_ID;

	return m_Textures[iIndex];
}

void CStudioModel::CreateTextures()
{
	UploadTextures(*GetTextureHeader(), m_Textures, r_filtertextures.GetBool(), r_powerof2textures.GetBool(), m_IsDol);
}

void CStudioModel::ReplaceTexture(mstudiotexture_t* ptexture, byte* data, byte* pal, GLuint textureId)
{
	UploadTexture(ptexture, data, pal, textureId, r_filtertextures.GetBool(), r_powerof2textures.GetBool());
}

void CStudioModel::ReuploadTexture(mstudiotexture_t* ptexture)
{
	assert(ptexture);

	auto header = GetTextureHeader();

	const int iIndex = ptexture - header->GetTextures();

	if (iIndex < 0 || iIndex >= header->numtextures)
	{
		Error("CStudioModel::ReuploadTexture: Invalid texture!");
		return;
	}

	GLuint textureId = m_Textures[iIndex];

	UploadTexture(ptexture,
		header->GetData() + ptexture->index,
		header->GetData() + ptexture->index + ptexture->width * ptexture->height, textureId, r_filtertextures.GetBool(), r_powerof2textures.GetBool());
}

namespace
{
template<typename T>
studio_ptr<T> LoadStudioHeader(const char* const pszFilename, const bool bAllowSeqGroup)
{
	// load the model
	FILE* pFile = utf8_fopen(pszFilename, "rb");

	if (!pFile)
	{
		throw StudioModelNotFound(std::string{"File \""} + pszFilename + "\" not found");
	}

	fseek(pFile, 0, SEEK_END);
	const size_t size = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	auto buffer = std::make_unique<byte[]>(size);

	auto pStudioHdr = reinterpret_cast<T*>(buffer.get());

	const size_t uiRead = fread(pStudioHdr, size, 1, pFile);
	fclose(pFile);

	if (uiRead != 1)
	{
		throw StudioModelInvalidFormat(std::string{"Error reading file\""} + pszFilename + "\"");
	}

	if (strncmp(reinterpret_cast<const char*>(&pStudioHdr->id), STUDIOMDL_HDR_ID, 4) &&
		strncmp(reinterpret_cast<const char*>(&pStudioHdr->id), STUDIOMDL_SEQ_ID, 4))
	{
		throw StudioModelInvalidFormat(std::string{"The file \""} + pszFilename + "\" is neither a studio header nor a sequence header");
	}

	if (!bAllowSeqGroup && !strncmp(reinterpret_cast<const char*>(&pStudioHdr->id), STUDIOMDL_SEQ_ID, 4))
	{
		throw StudioModelInvalidFormat(std::string{"File \""} + pszFilename + "\": Expected a main studio model file, got a sequence file");
	}

	if (pStudioHdr->version != STUDIO_VERSION)
	{
		throw StudioModelVersionDiffers(std::string{"File \""} + pszFilename + "\": version differs: expected \"" +
			std::to_string(STUDIO_VERSION) + "\", got \"" + std::to_string(pStudioHdr->version) + "\"",
			pStudioHdr->version);
	}

	buffer.release();

	return studio_ptr<T>(pStudioHdr);
}
}

std::unique_ptr<CStudioModel> LoadStudioModel(const char* const pszFilename)
{
	const std::filesystem::path fileName{std::filesystem::u8path(pszFilename)};

	std::filesystem::path baseFileName{fileName};

	baseFileName.replace_extension();

	const auto bIsDol = fileName.extension() == ".dol";

	//Load the model
	auto mainHeader = LoadStudioHeader<studiohdr_t>(pszFilename, false);

	if (mainHeader->name[0] == '\0')
	{
		//Only the main hader sets the name, so this must be something else (probably texture header, but could be anything)
		auto message = std::string{"The file \""} + pszFilename + "\" is not a studio model main header file";

		if (!baseFileName.empty() && std::toupper(baseFileName.u8string().back()) == 'T')
		{
			message += " (it is probably a texture file)";
		}

		throw StudioModelIsNotMainHeader(message);
	}

	studio_ptr<studiohdr_t> textureHeader;

	// preload textures
	if (mainHeader->numtextures == 0)
	{
		const auto extension = bIsDol ? "T.dol" : "T.mdl";

		auto texturename = baseFileName;

		texturename += extension;

		textureHeader = LoadStudioHeader<studiohdr_t>(texturename.u8string().c_str(), true);
	}

	std::vector<studio_ptr<studioseqhdr_t>> sequenceHeaders;

	// preload animations
	if (mainHeader->numseqgroups > 1)
	{
		sequenceHeaders.reserve(mainHeader->numseqgroups - 1);

		std::stringstream seqgroupname;

		for (int i = 1; i < mainHeader->numseqgroups; ++i)
		{
			seqgroupname.str({});

			const auto suffix = bIsDol ? ".dol" : ".mdl";

			seqgroupname << baseFileName.u8string() <<
				std::setfill('0') << std::setw(2) << i <<
				std::setw(0) << suffix;

			sequenceHeaders.emplace_back(LoadStudioHeader<studioseqhdr_t>(seqgroupname.str().c_str(), true));
		}
	}

	return std::make_unique<CStudioModel>(pszFilename, std::move(mainHeader), std::move(textureHeader),
		std::move(sequenceHeaders), bIsDol);
}

void SaveStudioModel(const char* const pszFilename, CStudioModel& model, bool correctSequenceGroupFileNames)
{
	if (!pszFilename)
	{
		throw StudioModelException("Null filename provided");
	}

	if (!(*pszFilename))
	{
		throw StudioModelException("Empty filename provided");
	}

	studiohdr_t* const pStudioHdr = model.GetStudioHeader();

	if (correctSequenceGroupFileNames)
	{
		std::filesystem::path baseFileName{pszFilename};

		//Find the "models" directory to determine what the relative path is
		auto relativeTo = baseFileName;

		auto foundRelative = false;

		//This will loop forever unless we test to see if relativeTo is the root path
		while (!relativeTo.empty() && relativeTo != relativeTo.root_path())
		{
			relativeTo = relativeTo.parent_path();

			if (relativeTo.stem() == "models")
			{
				relativeTo = relativeTo.parent_path();
				foundRelative = true;
				break;
			}
		}

		if (!foundRelative)
		{
			throw StudioModelException("Could not find base directory \"models\" in model filename; needed to correct sequence group filenames");
		}

		std::error_code e;

		baseFileName = std::filesystem::relative(baseFileName, relativeTo, e);

		if (e)
		{
			throw StudioModelException("Could not determine base filename for model to correct sequence group filenames");
		}

		baseFileName.replace_extension();

		const auto baseFileNameString = baseFileName.u8string();

		std::stringstream seqgroupname;

		//Group 0 is the main file and is never used to load files, and also doesn't have a filename set in the seqgroup structure
		for (int i = 1; i < pStudioHdr->numseqgroups; i++)
		{
			seqgroupname.str({});

			seqgroupname << baseFileNameString <<
				std::setfill('0') << std::setw(2) << i <<
				std::setw(0) << ".mdl";

			const auto groupNameString = seqgroupname.str();

			auto seqGroup = pStudioHdr->GetSequenceGroup(i);

			if (groupNameString.length() >= sizeof(seqGroup->name))
			{
				throw StudioModelException("Sequence group filename is too long");
			}

			strncpy(seqGroup->name, groupNameString.c_str(), groupNameString.length());

			seqGroup->name[sizeof(seqGroup->name) - 1] = '\0';
		}
	}

	FILE* pFile = utf8_fopen(pszFilename, "wb");

	if (!pFile)
	{
		throw StudioModelException("Could not open main file for writing");
	}

	bool bSuccess = fwrite(pStudioHdr, sizeof(byte), pStudioHdr->length, pFile) == pStudioHdr->length;

	fclose(pFile);

	if (!bSuccess)
	{
		throw StudioModelException("Error while writing to main file");
	}

	const std::filesystem::path fileName{std::filesystem::u8path(pszFilename)};

	auto baseFileName{fileName};

	baseFileName.replace_extension();

	// write texture model
	if (model.HasSeparateTextureHeader())
	{
		const studiohdr_t* const pTextureHdr = model.GetTextureHeader();

		auto texturename = baseFileName;

		texturename += "T.mdl";

		pFile = utf8_fopen(texturename.u8string().c_str(), "wb");

		if (!pFile)
		{
			throw StudioModelException("Could not open texture file for writing");
		}

		bSuccess = fwrite(pTextureHdr, sizeof(byte), pTextureHdr->length, pFile) == pTextureHdr->length;
		fclose(pFile);

		if (!bSuccess)
		{
			throw StudioModelException("Error while writing to texture file");
		}
	}

	// write seq groups
	if (pStudioHdr->numseqgroups > 1)
	{
		std::stringstream seqgroupname;

		for (int i = 1; i < pStudioHdr->numseqgroups; i++)
		{
			seqgroupname.str({});

			seqgroupname << baseFileName.u8string() <<
				std::setfill('0') << std::setw(2) << i <<
				std::setw(0) << ".mdl";

			pFile = utf8_fopen(seqgroupname.str().c_str(), "wb");

			if (!pFile)
			{
				throw StudioModelException("Could not open sequence file for writing");
			}

			const auto pAnimHdr = model.GetSeqGroupHeader(i - 1);

			bSuccess = fwrite(pAnimHdr, sizeof(byte), pAnimHdr->length, pFile) == pAnimHdr->length;
			fclose(pFile);

			if (!bSuccess)
			{
				throw StudioModelException("Error while writing to sequence file");
			}
		}
	}
}

void ScaleMeshes(CStudioModel* pStudioModel, const float flScale)
{
	assert(pStudioModel);

	auto pStudioHdr = pStudioModel->GetStudioHeader();

	int iBodygroup = 0;

	// scale verts
	for (int i = 0; i < pStudioHdr->numbodyparts; i++)
	{
		mstudiobodyparts_t* pbodypart = pStudioHdr->GetBodypart(i);
		for (int j = 0; j < pbodypart->nummodels; j++)
		{
			pStudioModel->CalculateBodygroup(i, j, iBodygroup);

			int bodypart = i;

			if (bodypart > pStudioHdr->numbodyparts)
			{
				// Message( "studiomdl::ScaleMeshes: no such bodypart %d\n", bodypart );
				bodypart = 0;
			}

			mstudiomodel_t* pModel = pStudioModel->GetModelByBodyPart(iBodygroup, bodypart);

			glm::vec3* pstudioverts = (glm::vec3*)((byte*) pStudioHdr + pModel->vertindex);

			for (int k = 0; k < pModel->numverts; k++)
			{
				pstudioverts[k] *= flScale;
			}
		}
	}

	// scale complex hitboxes
	mstudiobbox_t* pbboxes = pStudioHdr->GetHitBoxes();

	for (int i = 0; i < pStudioHdr->numhitboxes; i++)
	{
		pbboxes[i].bbmin *= flScale;
		pbboxes[i].bbmax *= flScale;
	}

	// scale bounding boxes
	mstudioseqdesc_t* pseqdesc = pStudioHdr->GetSequences();

	for (int i = 0; i < pStudioHdr->numseq; i++)
	{
		pseqdesc[i].bbmin *= flScale;
		pseqdesc[i].bbmax *= flScale;
	}

	// maybe scale exeposition, pivots, attachments
}

void ScaleBones(CStudioModel* pStudioModel, const float flScale)
{
	assert(pStudioModel);

	const auto pStudioHdr = pStudioModel->GetStudioHeader();

	mstudiobone_t* const pbones = pStudioHdr->GetBones();

	for (int i = 0; i < pStudioHdr->numbones; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			pbones[i].value[j] *= flScale;
			pbones[i].scale[j] *= flScale;
		}
	}
}

const char* ControlToString(const int iControl)
{
	switch (iControl)
	{
	case STUDIO_X:		return "X";
	case STUDIO_Y:		return "Y";
	case STUDIO_Z:		return "Z";
	case STUDIO_XR:		return "XR";
	case STUDIO_YR:		return "YR";
	case STUDIO_ZR:		return "ZR";
	case STUDIO_LX:		return "LX";
	case STUDIO_LY:		return "LY";
	case STUDIO_LZ:		return "LZ";
	case STUDIO_AX:		return "AX";
	case STUDIO_AY:		return "AY";
	case STUDIO_AZ:		return "AZ";
	case STUDIO_AXR:	return "AXR";
	case STUDIO_AYR:	return "AYR";
	case STUDIO_AZR:	return "AZR";

	default:			return nullptr;
	}
}

const char* ControlToStringDescription(const int iControl)
{
	switch (iControl)
	{
	case STUDIO_X:		return "Sequence motiontype flag: Zeroes out movement in the X axis\nBone controller type flag: Sets X offset";
	case STUDIO_Y:		return "Sequence motiontype flag: Zeroes out movement in the Y axis\nBone controller type flag: Sets Y offset";
	case STUDIO_Z:		return "Sequence motiontype flag: Zeroes out movement in the Z axis\nBone controller type flag: Sets Z offset";
	case STUDIO_XR:		return "Bone controller type flag: Sets X rotation\nSequence blendtype: Controls blending range clamping in the X axis";
	case STUDIO_YR:		return "Bone controller type flag: Sets Y rotation\nSequence blendtype: Controls blending range clamping in the Y axis";
	case STUDIO_ZR:		return "Bone controller type flag: Sets Z rotation\nSequence blendtype: Controls blending range clamping in the Z axis";
	case STUDIO_LX:		return "StudioMdl Compiler flag: Sets the sequence's last frame's X axis position to the first frame's position";
	case STUDIO_LY:		return "StudioMdl Compiler flag: Sets the sequence's last frame's Y axis position to the first frame's position";
	case STUDIO_LZ:		return "StudioMdl Compiler flag: Sets the sequence's last frame's Z axis position to the first frame's position";
	case STUDIO_AX:		return "StudioMdl Compiler flag: Extract relative movement distance between the first and current frame in the X axis (Unused)";
	case STUDIO_AY:		return "StudioMdl Compiler flag: Extract relative movement distance between the first and current frame in the Y axis (Unused)";
	case STUDIO_AZ:		return "StudioMdl Compiler flag: Extract relative movement distance between the first and current frame in the Z axis (Unused)";
	case STUDIO_AXR:	return "StudioMdl Compiler flag: Extract relative rotation distance between the first and current frame around the X axis (Unused)";
	case STUDIO_AYR:	return "StudioMdl Compiler flag: Extract relative rotation distance between the first and current frame around the Y axis (Unused)";
	case STUDIO_AZR:	return "StudioMdl Compiler flag: Extract relative rotation distance between the first and current frame around the Z axis (Unused)";

	default:			return nullptr;
	}
}

int StringToControl(const char* const pszString)
{
	assert(pszString);

	if (strcasecmp(pszString, "X") == 0) return STUDIO_X;
	if (strcasecmp(pszString, "Y") == 0) return STUDIO_Y;
	if (strcasecmp(pszString, "Z") == 0) return STUDIO_Z;
	if (strcasecmp(pszString, "XR") == 0) return STUDIO_XR;
	if (strcasecmp(pszString, "YR") == 0) return STUDIO_YR;
	if (strcasecmp(pszString, "ZR") == 0) return STUDIO_ZR;
	if (strcasecmp(pszString, "LX") == 0) return STUDIO_LX;
	if (strcasecmp(pszString, "LY") == 0) return STUDIO_LY;
	if (strcasecmp(pszString, "LZ") == 0) return STUDIO_LZ;
	if (strcasecmp(pszString, "AX") == 0) return STUDIO_AX;
	if (strcasecmp(pszString, "AY") == 0) return STUDIO_AY;
	if (strcasecmp(pszString, "AZ") == 0) return STUDIO_AZ;
	if (strcasecmp(pszString, "AXR") == 0) return STUDIO_AXR;
	if (strcasecmp(pszString, "AYR") == 0) return STUDIO_AYR;
	if (strcasecmp(pszString, "AZR") == 0) return STUDIO_AZR;

	return -1;
}
}