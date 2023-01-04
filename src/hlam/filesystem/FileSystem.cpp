#include <algorithm>
#include <cstdio>
#include <sstream>

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

	std::ostringstream stream;

	for (const auto& path : _searchPaths)
	{
		stream.str({});
		stream << path << '/' << fileName;

		auto result = stream.str();

		if (FileExists(result))
		{
			return result;
		}
	}

	auto result = stream.str();

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
