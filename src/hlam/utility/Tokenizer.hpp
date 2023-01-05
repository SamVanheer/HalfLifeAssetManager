#pragma once

#include <optional>
#include <string_view>

class Tokenizer final
{
public:
	explicit Tokenizer(std::string_view data)
		: _data(data)
	{
	}

	std::optional<std::string_view> Next();

private:
	std::string_view _data;
};
