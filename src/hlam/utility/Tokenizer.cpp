#include <cctype>
#include <cstddef>

#include "utility/Tokenizer.hpp"

std::optional<std::string_view> Tokenizer::Next()
{
	if (_data.empty())
	{
		return {};
	}

	bool skipText;

	do
	{
		skipText = false;

		while (!_data.empty() && std::isspace(_data[0]))
		{
			_data = _data.substr(1);
		}

		if (_data.starts_with("//"))
		{
			const auto index = _data.find_first_of("\r\n");

			if (index != std::string_view::npos)
			{
				_data = _data.substr(index + 1);
			}
			else
			{
				// Last line; nothing left to read.
				_data = {};
			}

			skipText = true;
		}
	} while (skipText && !_data.empty());

	if (_data.empty())
	{
		return {};
	}

	std::size_t count = 0;

	if (_data.starts_with('\"'))
	{
		_data = _data.substr(1);

		while (count < _data.size() && _data[count] != '\"')
		{
			++count;
		}

		const auto token = _data.substr(0, count);

		_data = _data.substr(count < _data.size() ? count + 1 : count);

		return token;
	}
	else
	{
		while (count < _data.size() && !std::isspace(_data[count]))
		{
			++count;
		}

		const auto token = _data.substr(0, count);

		_data = _data.substr(count);

		return token;
	}
}
