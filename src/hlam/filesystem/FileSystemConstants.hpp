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
extern const std::vector<std::string> SteamPipeDirectoryExtensions;

std::pair<bool, std::filesystem::path> IsFileInGameAssetsDirectory(const std::filesystem::path& fileName);

/** @} */
