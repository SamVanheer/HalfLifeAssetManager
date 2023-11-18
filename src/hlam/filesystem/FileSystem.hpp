#pragma once

#include <vector>

#include "filesystem/IFileSystem.hpp"

/**
*	@ingroup FileSystem
*
*	@{
*/

class FileSystem final : public IFileSystem
{
public:
	FileSystem() = default;
	~FileSystem() = default;

	FileSystem(const FileSystem&) = delete;
	FileSystem& operator=(const FileSystem&) = delete;

	bool HasSearchPath(std::string_view path) const override final;

	void AddSearchPath(std::string&& path) override final;

	void RemoveSearchPath(std::string_view path) override final;

	void RemoveAllSearchPaths() override final;

	std::string GetAbsolutePath(std::string_view fileName) override final;

	bool FileExists(std::string_view fileName) const override final;

private:
	std::vector<std::string> _searchPaths;
};

/** @} */
