#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <direct.h>

#undef GetCurrentTime
#undef ARRAYSIZE

#define MAX_PATH_LENGTH MAX_PATH

#define strcasecmp _stricmp
#define strncasecmp _strnicmp

#define getcwd _getcwd
#define setcwd _chdir

#define DLLEXPORT __declspec( dllexport )
#define DLLIMPORT __declspec( dllimport )

/*
*	NOTENOTE: This assumes that an MSVC compiler is being used for Windows targets.
*	See https://blogs.msdn.microsoft.com/oldnewthing/20041025-00/?p=37483
*/
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ( ( HINSTANCE ) &__ImageBase )

#else

#include <linux/limits.h>
#include <strings.h>

#define MAX_PATH PATH_MAX
#define MAX_PATH_LENGTH PATH_MAX

#define DLLEXPORT __attribute__( ( visibility( "default" ) ) )
#define DLLIMPORT

#endif

#endif //PLATFORM_H
