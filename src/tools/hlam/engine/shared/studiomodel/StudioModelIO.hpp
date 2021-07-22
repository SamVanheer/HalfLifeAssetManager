#pragma once

#include <cstdio>
#include <filesystem>
#include <memory>

#include "assets/AssetIO.hpp"

namespace studiomdl
{
class StudioModel;

/**
*	@brief Indicates that a studio model file is not a main header (e.g. trying to load texture header as a main header)
*/
class StudioModelIsNotMainHeader : public assets::AssetException
{
public:
	using assets::AssetException::AssetException;
};

bool IsStudioModel(FILE* file);

/**
*	@brief Loads a studio model
*	@param fileName Name of the model to load. This is the entire path, including the extension
*	@param mainFile Handle to the main file
*	@exception assets::AssetNotFound If a file could not be found
*	@exception assets::AssetInvalidFormat If a file has an invalid format
*	@exception assets::AssetVersionDiffers If a file has the wrong studio version
*	@exception StudioModelIsNotMainHeader If the filename specifies a studio model file that is not the main file
*/
std::unique_ptr<StudioModel> LoadStudioModel(const std::filesystem::path& fileName, FILE* mainFile);

/**
*	Saves a studio model.
*	@param fileName Name of the file to save the model to. This is the entire path, including the extension.
*	@param model Model to save.
* *	@param correctSequenceGroupFileNames Whether the sequence group filenames embedded in the main file should be corrected
*	@exception StudioModelException If an error occurs or if the given data is invalid
*/
void SaveStudioModel(const std::filesystem::path& fileName, StudioModel& model, bool correctSequenceGroupFileNames);
}
