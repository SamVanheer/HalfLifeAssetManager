#include <algorithm>
#include <cstdio>
#include <iterator>

#include <fmt/format.h>

#include "filesystem/FileSystem.hpp"

#include "utility/IOUtils.hpp"

bool FileSystem::HasSearchPath(std::string_view path) const
{
	if (path.empty())
	{
		return false;
	}

	return std::find(_searchPaths.begin(), _searchPaths.end(), path) != _searchPaths.end();
}

void FileSystem::AddSearchPath(std::string&& path)
{
	if (path.empty())
	{
		return;
	}

	if (HasSearchPath(path))
	{
		return;
	}

	_searchPaths.emplace_back(std::move(path));
}

void FileSystem::RemoveSearchPath(std::string_view path)
{
	if (path.empty())
	{
		return;
	}

	if (const auto it = std::find(_searchPaths.begin(), _searchPaths.end(), path); it != _searchPaths.end())
	{
		_searchPaths.erase(it);
	}
}

void FileSystem::RemoveAllSearchPaths()
{
	_searchPaths.clear();
}

std::string FileSystem::GetRelativePath(std::string_view fileName)
{
	if (fileName.empty())
	{
		return {};
	}

	std::string candidate;

	for (const auto& path : _searchPaths)
	{
		candidate.clear();
		fmt::format_to(std::back_inserter(candidate), "{}/{}", path, fileName);

		if (FileExists(candidate))
		{
			return candidate;
		}
	}

	if (FileExists(fileName))
	{
		return std::string{fileName};
	}

	return {};
}

bool FileSystem::FileExists(std::string_view fileName) const
{
	if (fileName.empty())
	{
		return false;
	}

	const std::string fullName{fileName};

	if (FILE* file = utf8_fopen(fullName.c_str(), "r"); file)
	{
		fclose(file);

		return true;
	}

	return false;
}
