#pragma once

#include <stdexcept>

/**
*	@brief Exception used for asset errors
*/
class AssetException : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;

	explicit AssetException(const std::u8string& message)
		: std::runtime_error(reinterpret_cast<const char*>(message.c_str()))
	{
	}
};
