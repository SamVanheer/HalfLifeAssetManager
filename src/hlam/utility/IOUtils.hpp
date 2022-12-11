#pragma once

#include <cstdio>

/**
*	@brief Wrapper around fopen to open filenames containing UTF8 characters on Windows
*/
FILE* utf8_fopen(const char* filename, const char* mode);

inline FILE* utf8_fopen(const char8_t* filename, const char* mode)
{
	return utf8_fopen(reinterpret_cast<const char*>(filename), mode);
}


/**
*	@brief Opens the file for reading, but only if the file is not already opened elsewhere (Only on Windows, other platforms use @see utf8_fopen)
*/
FILE* utf8_exclusive_read_fopen(const char* filename, bool asBinary);

inline FILE* utf8_exclusive_read_fopen(const char8_t* filename, bool asBinary)
{
	return utf8_exclusive_read_fopen(reinterpret_cast<const char*>(filename), asBinary);
}
