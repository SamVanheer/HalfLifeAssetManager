#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <iterator>

#include <fmt/format.h>

#include "filesystem/FileSystem.hpp"

#include "utility/IOUtils.hpp"
#include "utility/Platform.hpp"

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

std::string FileSystem::GetAbsolutePath(std::string_view fileName)
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

		if (TryFindFileName(candidate, candidate))
		{
			return candidate;
		}
	}

	// No reason to check relative to current directory since this program's location is irrelevant.

	return {};
}

bool FileSystem::FileExists(const std::string& fileName) const
{
	if (fileName.empty())
	{
		return false;
	}

	if (FILE* file = utf8_fopen(fileName.c_str(), "r"); file)
	{
		fclose(file);

		return true;
	}

	return false;
}

static FilePtr TryOpenFile(const std::string& fileName, bool binary, bool exclusive)
{
	FilePtr file;

	if (exclusive)
	{
		file.reset(utf8_exclusive_read_fopen(fileName.c_str(), binary));
	}
	else
	{
		file.reset(utf8_fopen(fileName.c_str(), binary ? "rb" : "r"));
	}

	return file;
}

FilePtr FileSystem::TryOpenAbsolute(const std::string& fileName, bool binary, bool exclusive) const
{
	FilePtr file = TryOpenFile(fileName, binary, exclusive);

	if (file)
	{
		return file;
	}

	// Try to find the file using case insensitive search.
	std::string realFileName;
	return TryFindFileName(fileName, realFileName);
}

FilePtr FileSystem::TryOpen(std::string_view fileName, bool binary, bool exclusive) const
{
	std::string candidate;

	for (const auto& path : _searchPaths)
	{
		candidate.clear();
		fmt::format_to(std::back_inserter(candidate), "{}/{}", path, fileName);

		FilePtr file = TryOpenAbsolute(candidate, binary, exclusive);

		if (file)
		{
			return file;
		}
	}

	return {};
}

FilePtr FileSystem::TryFindFileName(const std::string& fileName, std::string& realFileName) const
{
	const std::filesystem::path absoluteFileName = std::filesystem::u8path(fileName);
	const std::filesystem::path absoluteDirectory = absoluteFileName.parent_path();
	const std::string baseFileName = reinterpret_cast<const char*>(absoluteFileName.filename().u8string().c_str());

	try
	{
		for (const auto& candidate : std::filesystem::directory_iterator(absoluteDirectory))
		{
			if (!candidate.is_regular_file() && !candidate.is_symlink())
			{
				continue;
			}

			const std::filesystem::path& candidatePath = candidate.path();
			const std::string candidateName = reinterpret_cast<const char*>(candidatePath.filename().u8string().c_str());

			if (0 == strcasecmp(baseFileName.c_str(), candidateName.c_str()))
			{
				std::string candidateFileName = reinterpret_cast<const char*>(candidatePath.u8string().c_str());

				auto file = TryOpenFile(candidateFileName, true, false);

				if (file)
				{
					realFileName = std::move(candidateFileName);
					return file;
				}
			}
		}
	}
	catch (const std::filesystem::filesystem_error&)
	{
		// Can't do anything about this. Maybe log the error?
	}

	realFileName.clear();

	return {};
}
