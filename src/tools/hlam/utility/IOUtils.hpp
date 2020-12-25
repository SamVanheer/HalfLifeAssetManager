#pragma once

#include <cstdio>

/**
*	@brief Wrapper around fopen to open filenames containing UTF8 characters on Windows
*/
FILE* utf8_fopen(const char* filename, const char* mode);
