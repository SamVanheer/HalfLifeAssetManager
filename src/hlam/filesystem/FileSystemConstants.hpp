#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/**
*	@ingroup FileSystem
*
*	@{
*/

extern const std::vector<std::string_view> GameAssetDirectories;

std::vector<std::string> GetSteamPipeDirectoryExtensions();

std::pair<bool, std::filesystem::path> IsFileInGameAssetsDirectory(const std::filesystem::path& fileName);

/** @} */
