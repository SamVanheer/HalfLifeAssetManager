#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

/**
*	@ingroup FileSystem
*
*	@{
*/

extern const std::vector<const char*> GameAssetDirectories;
extern const std::vector<std::string> SteamPipeDirectoryExtensions;
extern const char DefaultSteamLanguage[];
extern const std::vector<const char*> SteamLanguages;

std::optional<std::filesystem::path> IsFileInGameAssetsDirectory(const std::filesystem::path& fileName);

/** @} */
