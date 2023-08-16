#include <algorithm>
#include <iterator>

#include "filesystem/FileSystemConstants.hpp"

#include "utility/Platform.hpp"

using namespace std::string_view_literals;

const std::vector<const char*> GameAssetDirectories
{
	"maps",
	"models",
	"resource",
	"sprites",
	"sound"
};

const std::vector<std::string> SteamPipeDirectoryExtensions
{
	std::string{""sv},
	std::string{"_downloads"sv},
	std::string{"_addon"sv},
	std::string{"_hd"sv}
};

// From https://partner.steamgames.com/doc/store/localization/languages#supported_languages
const char DefaultSteamLanguage[]{"english"};

const std::vector<const char*> SteamLanguages
{
	"arabic",
	"bulgarian",
	"schinese",
	"tchinese",
	"czech",
	"danish",
	"dutch",
	"english",
	"finnish",
	"french",
	"german",
	"greek",
	"hungarian",
	"italian",
	"japanese",
	"koreana",
	"norwegian",
	"polish",
	"portuguese",
	"brazilian",
	"romanian",
	"russian",
	"spanish",
	"latam",
	"swedish",
	"thai",
	"turkish",
	"ukrainian",
	"vietnamese"
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
			if (strcasecmp(it->c_str(), test) == 0)
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
