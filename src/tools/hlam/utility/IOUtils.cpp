#include <codecvt>
#include <locale>
#include <memory>
#include <string>

#include "utility/IOUtils.hpp"

#ifdef WIN32
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#endif

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

FILE* utf8_exclusive_read_fopen(const char* filename, bool asBinary)
{
#ifdef WIN32
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;

	auto wideFilename = convert.from_bytes(filename);

	const HANDLE fh = CreateFileW(wideFilename.c_str(), GENERIC_READ, 0 /* no sharing! exclusive */, NULL, OPEN_EXISTING, 0, nullptr);

	if (fh == nullptr || fh == INVALID_HANDLE_VALUE)
	{
		return nullptr;
	}

	int osFlags = _O_RDONLY;

	if (!asBinary)
	{
		osFlags |= _O_TEXT;
	}

	const int fileHandle = _open_osfhandle(reinterpret_cast<intptr_t>(fh), osFlags);

	if (fileHandle == -1)
	{
		CloseHandle(fh);
		return nullptr;
	}

	FILE* file = _fdopen(fileHandle, asBinary ? "r" : "rb");

	if (!file)
	{
		_close(fileHandle);
		return nullptr;
	}

	return file;
#else
	return utf8_fopen(filename, asBinary ? "r" : "rb");
#endif
}
