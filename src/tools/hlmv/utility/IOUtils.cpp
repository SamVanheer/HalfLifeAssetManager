#include <codecvt>
#include <locale>
#include <memory>
#include <string>

#include "IOUtils.h"

FILE* utf8_fopen(const char* filename, const char* mode)
{
#ifdef WIN32
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;

	auto wideFilename = convert.from_bytes(filename);
	auto wideMode = convert.from_bytes(mode);

	return _wfopen(wideFilename.c_str(), wideMode.c_str());
#else
	return fopen(filename, mode);
#endif
}
