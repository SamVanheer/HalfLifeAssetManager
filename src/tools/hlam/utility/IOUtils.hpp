#pragma once

#include <cstdio>

/**
*	@brief Wrapper around fopen to open filenames containing UTF8 characters on Windows
*/
FILE* utf8_fopen(const char* filename, const char* mode);

/**
*	@brief Opens the file for reading, but only if the file is not already opened elsewhere (Only on Windows, other platforms use @see utf8_fopen)
*/
FILE* utf8_exclusive_read_fopen(const char* filename, bool asBinary);
