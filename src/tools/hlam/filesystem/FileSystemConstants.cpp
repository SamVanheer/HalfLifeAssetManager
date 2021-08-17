#include "filesystem/FileSystemConstants.hpp"

namespace filesystem
{
std::vector<std::string> GetSteamPipeDirectoryExtensions()
{
	return
	{
		"",
		"_downloads",
		"_addon",
		"_hd"
	};
}
}