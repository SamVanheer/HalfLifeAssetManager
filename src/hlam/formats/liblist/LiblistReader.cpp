#include <fstream>
#include <streambuf>

#include "LiblistReader.hpp"
#include "utility/IOUtils.hpp"
#include "utility/Tokenizer.hpp"

std::optional<std::unordered_map<std::string, std::string>> LiblistReader::Read(const std::string& fileName)
{
	// liblist.gam files have comments and a list of keyvalue pairs in the format "key \"value\""

	std::ifstream file(fileName);

	if (!file.is_open())
	{
		return {};
	}

	std::string contents;

	file.seekg(0, std::ios::end);
	contents.reserve(file.tellg());
	file.seekg(0, std::ios::beg);

	contents.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	std::unordered_map<std::string, std::string> keyvalues;

	Tokenizer tokenizer{contents};

	while (true)
	{
		const auto key = tokenizer.Next();
		const auto value = tokenizer.Next();

		if (!key || !value)
		{
			break;
		}

		keyvalues.insert_or_assign(std::string{*key}, *value);
	}

	return keyvalues;
}
