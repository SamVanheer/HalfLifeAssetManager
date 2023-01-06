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

extern const std::vector<std::string_view> GameAssetDirectories;
extern const std::vector<std::string> SteamPipeDirectoryExtensions;
extern const std::string_view DefaultSteamLanguage;
extern const std::vector<std::string_view> SteamLanguages;

std::optional<std::filesystem::path> IsFileInGameAssetsDirectory(const std::filesystem::path& fileName);

/** @} */
