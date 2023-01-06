#include <algorithm>
#include <iterator>

#include "filesystem/FileSystemConstants.hpp"

#include "utility/Platform.hpp"

using namespace std::string_view_literals;

const std::vector<std::string_view> GameAssetDirectories
{
	"maps"sv,
	"models"sv,
	"resource"sv,
	"sprites"sv,
	"sound"sv
};

const std::vector<std::string> SteamPipeDirectoryExtensions
{
	std::string{""sv},
	std::string{"_downloads"sv},
	std::string{"_addon"sv},
	std::string{"_hd"sv}
};

// From https://partner.steamgames.com/doc/store/localization/languages#supported_languages
const std::string_view DefaultSteamLanguage{"english"sv};

const std::vector<std::string_view> SteamLanguages
{
	"arabic"sv,
	"bulgarian"sv,
	"schinese"sv,
	"tchinese"sv,
	"czech"sv,
	"danish"sv,
	"dutch"sv,
	"english"sv,
	"finnish"sv,
	"french"sv,
	"german"sv,
	"greek"sv,
	"hungarian"sv,
	"italian"sv,
	"japanese"sv,
	"koreana"sv,
	"norwegian"sv,
	"polish"sv,
	"portuguese"sv,
	"brazilian"sv,
	"romanian"sv,
	"russian"sv,
	"spanish"sv,
	"latam"sv,
	"swedish"sv,
	"thai"sv,
	"turkish"sv,
	"ukrainian"sv,
	"vietnamese"sv
};

std::optional<std::filesystem::path> IsFileInGameAssetsDirectory(const std::filesystem::path& fileName)
{
	auto directory = fileName.parent_path();

	// Construct list of directory names in reverse order since path doesn't support reverse iteration.
	std::vector<std::string> directories;
	directories.reserve(std::distance(directory.begin(), directory.end()));

	for (const auto& part : directory)
	{
		directories.insert(directories.begin(), part.string());
	}

	for (const auto& test : GameAssetDirectories)
	{
		for (auto it = directories.rbegin(), end = directories.rend(); it != end; ++it)
		{
			if (strcasecmp(it->c_str(), test.data()) == 0)
			{
				for (auto toRemove = std::distance(it, end); toRemove > 0; --toRemove)
				{
					directory = directory.parent_path();
				}

				return directory;
			}
		}
	}

	return {};
}
