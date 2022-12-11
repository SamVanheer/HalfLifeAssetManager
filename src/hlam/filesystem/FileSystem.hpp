#pragma once

#include <vector>

#include "filesystem/IFileSystem.hpp"

/**
*	@ingroup FileSystem
*
*	@{
*/

namespace filesystem
{
class FileSystem final : public IFileSystem
{
public:
	FileSystem();
	~FileSystem();

	FileSystem(const FileSystem&) = delete;
	FileSystem& operator=(const FileSystem&) = delete;

	std::string GetBasePath() const override final;

	void SetBasePath(std::string&& path) override final;

	bool HasSearchPath(std::string_view path) const override final;

	void AddSearchPath(std::string&& path) override final;

	void RemoveSearchPath(std::string_view path) override final;

	void RemoveAllSearchPaths() override final;

	std::string GetRelativePath(std::string_view fileName) override final;

	bool FileExists(const std::string& fileName) const override final;

private:
	std::string _basePath;
	std::vector<std::string> _searchPaths;
};
}

/** @} */
