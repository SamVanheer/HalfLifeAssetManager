#ifndef UI_UTILITY_WXUTIL_H
#define UI_UTILITY_WXUTIL_H

#include <vector>

#include "ui/wxInclude.h"

#include <wx/display.h>

namespace wx
{
using VideoModeComparator = bool ( * )( const wxVideoMode& lhs, const wxVideoMode& rhs );

bool CompareVideoModeResolutions( const wxVideoMode& lhs, const wxVideoMode& rhs );

bool GetUniqueVideoModes( const wxDisplay& display, std::vector<wxVideoMode>& videoModes, const VideoModeComparator comparator );

bool GetUniqueVideoModes( const wxDisplay& display, std::vector<wxVideoMode>& videoModes );
}

#endif //UI_UTILITY_WXUTIL_H