#pragma once

#include <cstdio>
#include <filesystem>
#include <memory>

#include "application/AssetIO.hpp"

namespace studiomdl
{
class StudioModel;

bool IsStudioModel(FILE* file);

bool IsMainStudioModel(FILE* file);

/**
*	@brief Loads a studio model
*	@param fileName Name of the model to load. This is the entire path, including the extension
*	@param mainFile Handle to the main file
*	@exception assets::AssetException If a file could not be found,
*		If a file has an invalid format
*		If a file has the wrong studio version
*		If the filename specifies a studio model file that is not the main file
*/
std::unique_ptr<StudioModel> LoadStudioModel(const std::filesystem::path& fileName, FILE* mainFile);

/**
*	Saves a studio model.
*	@param fileName Name of the file to save the model to. This is the entire path, including the extension.
*	@param model Model to save.
* *	@param correctSequenceGroupFileNames Whether the sequence group filenames embedded in the main file should be corrected
*	@exception assets::AssetException If an error occurs or if the given data is invalid
*/
void SaveStudioModel(const std::filesystem::path& fileName, StudioModel& model, bool correctSequenceGroupFileNames);
}
