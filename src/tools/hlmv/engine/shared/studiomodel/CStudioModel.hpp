#ifndef GAME_STUDIOMODEL_CSTUDIOMODEL_H
#define GAME_STUDIOMODEL_CSTUDIOMODEL_H

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <glm/vec3.hpp>

#include "shared/Const.hpp"

#include "utility/mathlib.hpp"
#include "utility/Color.hpp"

#include "graphics/OpenGL.hpp"

#include "studio.hpp"

namespace graphics
{
class TextureLoader;
}

namespace studiomdl
{
/**
*	@brief Base class for all studio model exceptions
*/
class StudioModelException : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};

/**
*	@brief Indicates that the studio model file does not exist
*/
class StudioModelNotFound : public StudioModelException
{
public:
	using StudioModelException::StudioModelException;
};

/**
*	@brief Indicates that a studio model file has the wrong format (i.e. not a studio model)
*/
class StudioModelInvalidFormat : public StudioModelException
{
public:
	using StudioModelException::StudioModelException;
};

/**
*	@brief Indicates that a studio model file has the wrong version (e.g. trying to load a Source model)
*/
class StudioModelVersionDiffers : public StudioModelException
{
public:
	StudioModelVersionDiffers(const std::string& message, int version)
		: StudioModelException(message)
		, _version(version)
	{
	}

	/**
	*	@brief Gets the version that the model has
	*/
	int GetVersion() const { return _version; }

private:
	const int _version;
};

/**
*	@brief Indicates that a studio model file is not a main header (e.g. trying to load texture header as a main header)
*/
class StudioModelIsNotMainHeader : public StudioModelException
{
public:
	using StudioModelException::StudioModelException;
};

struct StudioDataDeleter
{
	void operator()(studiohdr_t* pointer) const
	{
		delete[] pointer;
	}

	void operator()(studioseqhdr_t* pointer) const
	{
		delete[] pointer;
	}
};

template<typename T>
using studio_ptr = std::unique_ptr<T, StudioDataDeleter>;

class CStudioModel;

/**
*	Loads a studio model
*	@param pszFilename Name of the model to load. This is the entire path, including the extension
*	@exception StudioModelNotFound If a file could not be found
*	@exception StudioModelInvalidFormat If a file has an invalid format
*	@exception StudioModelVersionDiffers If a file has the wrong studio version
*/
std::unique_ptr<CStudioModel> LoadStudioModel(const char* const pszFilename);

/**
*	Saves a studio model.
*	@param pszFilename Name of the file to save the model to. This is the entire path, including the extension.
*	@param model Model to save.
* *	@param correctSequenceGroupFileNames Whether the sequence group filenames embedded in the main file should be corrected
*	@exception StudioModelException If an error occurs or if the given data is invalid
*/
void SaveStudioModel( const char* const pszFilename, CStudioModel& model, bool correctSequenceGroupFileNames );

/**
*	Container representing a studiomodel and its data.
*/
class CStudioModel final
{
private:
	typedef std::vector<const mstudiomesh_t*> MeshList_t;
	typedef std::vector<MeshList_t> TextureMeshMap_t;

protected:
	friend std::unique_ptr<CStudioModel> LoadStudioModel(const char* const pszFilename);

public:
	static const size_t MAX_SEQGROUPS = 32;
	static const size_t MAX_TEXTURES = MAXSTUDIOSKINS;

public:
	CStudioModel(std::string&& fileName, studio_ptr<studiohdr_t>&& pStudioHdr, studio_ptr<studiohdr_t>&& pTextureHdr,
		std::vector<studio_ptr<studioseqhdr_t>>&& sequenceHeaders, bool isDol);
	~CStudioModel();

	const std::string& GetFileName() const { return m_FileName; }

	void SetFileName(std::string&& fileName)
	{
		m_FileName = std::move(fileName);
	}

	studiohdr_t* GetStudioHeader() const { return m_pStudioHdr.get(); }

	bool HasSeparateTextureHeader() const { return !!m_pTextureHdr; }

	studiohdr_t* GetTextureHeader() const
	{
		if (m_pTextureHdr)
		{
			return m_pTextureHdr.get();
		}

		return m_pStudioHdr.get();
	}

	studioseqhdr_t* GetSeqGroupHeader( const size_t i ) const { return m_SequenceHeaders[ i ].get(); }

	mstudioanim_t* GetAnim( mstudioseqdesc_t* pseqdesc ) const;

	mstudiomodel_t* GetModelByBodyPart( const int iBody, const int iBodyPart ) const;

	int GetBodyValueForGroup(int compositeValue, int group) const;

	bool CalculateBodygroup( const int iGroup, const int iValue, int& iInOutBodygroup ) const;

	GLuint GetTextureId( const int iIndex ) const;

	void CreateTextures(graphics::TextureLoader& textureLoader);

	void ReplaceTexture(graphics::TextureLoader& textureLoader, mstudiotexture_t* ptexture, const byte* data, const byte* pal, GLuint textureId);

	/**
	*	Reuploads a texture. Useful for making changes made to the texture's pixel, palette or flag data show up in the model itself.
	* *	@param textureLoader Loader to use for texture uploading
	*	@param ptexture Texture to reupload. Must be a texture that is part of this model.
	*/
	void ReuploadTexture(graphics::TextureLoader& textureLoader, mstudiotexture_t* ptexture);

	void UpdateFilters(graphics::TextureLoader& textureLoader);

	void ReuploadTextures(graphics::TextureLoader& textureLoader);

	std::vector<int> GetRootBoneIndices()
	{
		std::vector<int> bones;

		for (int i = 0; i < m_pStudioHdr->numbones; ++i)
		{
			auto bone = m_pStudioHdr->GetBone(i);

			if (bone->parent == -1)
			{
				bones.emplace_back(i);
			}
		}

		return bones;
	}

private:
	std::string m_FileName;

	studio_ptr<studiohdr_t> m_pStudioHdr;
	studio_ptr<studiohdr_t> m_pTextureHdr;

	std::vector<studio_ptr<studioseqhdr_t>> m_SequenceHeaders;

	std::vector<GLuint> m_Textures;

	bool m_IsDol;

private:
	CStudioModel( const CStudioModel& ) = delete;
	CStudioModel& operator=( const CStudioModel& ) = delete;
};

struct ScaleMeshesData
{
	std::vector<std::vector<glm::vec3>> Vertices;
	std::vector<std::pair<glm::vec3, glm::vec3>> Hitboxes;
	std::vector<std::pair<glm::vec3, glm::vec3>> SequenceBBoxes;
};

std::pair<ScaleMeshesData, ScaleMeshesData> CalculateScaledMeshesData(const CStudioModel& studioModel, const float scale);

void ApplyScaleMeshesData(CStudioModel& studioModel, const ScaleMeshesData& data);

struct ScaleBonesBoneData
{
	glm::vec3 Position;
	glm::vec3 Scale;
};

std::pair<std::vector<ScaleBonesBoneData>, std::vector<ScaleBonesBoneData>> CalculateScaledBonesData(const CStudioModel& studioModel, const float scale);

void ApplyScaleBonesData(CStudioModel& studioModel, const std::vector<studiomdl::ScaleBonesBoneData>& data);

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
