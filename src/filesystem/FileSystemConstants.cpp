#include "common/Platform.h"

#include "FileSystemConstants.h"

namespace filesystem
{
const char* const STEAMPIPE_DIRECTORY_EXTS[] =
{
	"",
	"_downloads",
	"_addon",
	"_hd"
};

const size_t NUM_STEAMPIPE_DIRECTORY_EXTS = ARRAYSIZE( STEAMPIPE_DIRECTORY_EXTS );
}