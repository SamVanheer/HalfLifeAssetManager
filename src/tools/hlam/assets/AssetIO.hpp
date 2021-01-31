#pragma once

#include <stdexcept>

namespace assets
{
/**
*	@brief Base class for all asset exceptions.
*/
class AssetException : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};

/**
*	@brief Indicates that the file does not exist
*/
class AssetFileNotFound : public AssetException
{
public:
	using AssetException::AssetException;
};

/**
*	@brief Indicates that a file has the wrong format
*/
class AssetInvalidFormat : public AssetException
{
public:
	using AssetException::AssetException;
};

/**
*	@brief Indicates that a file has the wrong version
*/
class AssetVersionDiffers : public AssetException
{
public:
	using AssetException::AssetException;
};
}
