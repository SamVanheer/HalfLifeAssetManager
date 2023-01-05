#pragma once

#include <optional>
#include <string>
#include <unordered_map>

class LiblistReader final
{
public:
	LiblistReader() = delete;

	static std::optional<std::unordered_map<std::string, std::string>> Read(const std::string& fileName);
};
