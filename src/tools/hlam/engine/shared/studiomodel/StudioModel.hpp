#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <glm/vec3.hpp>

#include "assets/AssetIO.hpp"

#include "core/shared/Const.hpp"

#include "utility/mathlib.hpp"
#include "utility/Color.hpp"

#include "graphics/OpenGL.hpp"

#include "engine/shared/studiomodel/StudioModelFileFormat.hpp"

namespace graphics
{
class TextureLoader;
}

namespace studiomdl
{
/**
*	@brief Indicates that a studio model file is not a main header (e.g. trying to load texture header as a main header)
*/
class StudioModelIsNotMainHeader : public assets::AssetException
{
public:
	using assets::AssetException::AssetException;
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

class StudioModel;

bool IsStudioModel(const std::string& fileName);

/**
*	Loads a studio model
*	@param fileName Name of the model to load. This is the entire path, including the extension
*	@exception assets::AssetNotFound If a file could not be found
*	@exception assets::AssetInvalidFormat If a file has an invalid format
*	@exception assets::AssetVersionDiffers If a file has the wrong studio version
*/
std::unique_ptr<StudioModel> LoadStudioModel(const char* const fileName);

/**
*	Saves a studio model.
*	@param fileName Name of the file to save the model to. This is the entire path, including the extension.
*	@param model Model to save.
* *	@param correctSequenceGroupFileNames Whether the sequence group filenames embedded in the main file should be corrected
*	@exception StudioModelException If an error occurs or if the given data is invalid
*/
void SaveStudioModel(const char* const fileName, StudioModel& model, bool correctSequenceGroupFileNames);

/**
*	Container representing a studiomodel and its data.
*/
class StudioModel final
{
protected:
	friend std::unique_ptr<StudioModel> LoadStudioModel(const char* const pszFilename);

public:
	static const size_t MAX_SEQGROUPS = 32;

public:
	StudioModel(std::string&& fileName, studio_ptr<studiohdr_t>&& studioHeader, studio_ptr<studiohdr_t>&& textureHeader,
		std::vector<studio_ptr<studioseqhdr_t>>&& sequenceHeaders, bool isDol);
	~StudioModel();

	StudioModel(const StudioModel&) = delete;
	StudioModel& operator=(const StudioModel&) = delete;

	const std::string& GetFileName() const { return _fileName; }

	void SetFileName(std::string&& fileName)
	{
		_fileName = std::move(fileName);
	}

	studiohdr_t* GetStudioHeader() const { return _studioHeader.get(); }

	bool HasSeparateTextureHeader() const { return !!_textureHeader; }

	studiohdr_t* GetTextureHeader() const
	{
		if (_textureHeader)
		{
			return _textureHeader.get();
		}

		return _studioHeader.get();
	}

	studioseqhdr_t* GetSeqGroupHeader(const size_t i) const { return _sequenceHeaders[i].get(); }

	mstudioanim_t* GetAnim(mstudioseqdesc_t* pseqdesc) const;

	mstudiomodel_t* GetModelByBodyPart(const int iBody, const int iBodyPart) const;

	int GetBodyValueForGroup(int compositeValue, int group) const;

	bool CalculateBodygroup(const int iGroup, const int iValue, int& iInOutBodygroup) const;

	GLuint GetTextureId(const int iIndex) const;

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

		for (int i = 0; i < _studioHeader->numbones; ++i)
		{
			auto bone = _studioHeader->GetBone(i);

			if (bone->parent == -1)
			{
				bones.emplace_back(i);
			}
		}

		return bones;
	}

private:
	std::string _fileName;

	studio_ptr<studiohdr_t> _studioHeader;
	studio_ptr<studiohdr_t> _textureHeader;

	std::vector<studio_ptr<studioseqhdr_t>> _sequenceHeaders;

	std::vector<GLuint> _textures;

	bool _isDol;
};

struct ScaleMeshesData
{
	std::vector<std::vector<glm::vec3>> Vertices;
	std::vector<std::pair<glm::vec3, glm::vec3>> Hitboxes;
	std::vector<std::pair<glm::vec3, glm::vec3>> SequenceBBoxes;
};

std::pair<ScaleMeshesData, ScaleMeshesData> CalculateScaledMeshesData(const StudioModel& studioModel, const float scale);

void ApplyScaleMeshesData(StudioModel& studioModel, const ScaleMeshesData& data);

struct ScaleBonesBoneData
{
	glm::vec3 Position;
	glm::vec3 Scale;
};

std::pair<std::vector<ScaleBonesBoneData>, std::vector<ScaleBonesBoneData>> CalculateScaledBonesData(const StudioModel& studioModel, const float scale);

void ApplyScaleBonesData(StudioModel& studioModel, const std::vector<studiomdl::ScaleBonesBoneData>& data);

/**
*	Returns the string representation for a studio control value.
*	@param iControl Value containing a STUDIO_* control bit.
*	@return String representation for the control, or nullptr.
*/
const char* ControlToString(const int iControl);

/**
*	Returns the description for a studio control value.
*	@param iControl Value containing a STUDIO_* control bit.
*	@return String description for the control, or nullptr.
*/
const char* ControlToStringDescription(const int iControl);

/**
*	Converts a string to a control value.
*	@param pszString String representation of a control value.
*	@return Control value, or -1 if the string does not represent a valid control.
*/
int StringToControl(const char* const pszString);
}
