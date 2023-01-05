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

std::pair<bool, std::filesystem::path> IsFileInGameAssetsDirectory(const std::filesystem::path& fileName)
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

				return {true, directory};
			}
		}
	}

	return {false, {}};
}
