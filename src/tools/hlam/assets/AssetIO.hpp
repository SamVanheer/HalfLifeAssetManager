#pragma once

#include <stdexcept>

namespace assets
{
/**
*	@brief Exception used for asset errors
*/
class AssetException : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};
}
