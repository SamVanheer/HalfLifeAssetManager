#ifndef UI_UTILITY_WXUTIL_H
#define UI_UTILITY_WXUTIL_H

#include <vector>

#include "ui/wx/wxInclude.h"

#include <wx/display.h>

#include "utility/Color.h"

namespace wx
{
/**
*	Function pointer type used to compare video modes.
*/
using VideoModeComparator = bool ( * )( const wxVideoMode& lhs, const wxVideoMode& rhs );

/**
*	Compares video modes using the resolutions only.
*	@return true if the modes are identical, false otherwise.
*/
bool CompareVideoModeResolutions( const wxVideoMode& lhs, const wxVideoMode& rhs );

/**
*	Given a display, fills the vector videoModes with all supported video modes, using comparator to compare video modes.
*	@param display Display whose supported video modes will be queried.
*	@param videoModes Destination vector that will contain all video modes.
*	@param comparator Comparator to use when filtering the list.
*	@return true on success, false otherwise.
*/
bool GetUniqueVideoModes( const wxDisplay& display, std::vector<wxVideoMode>& videoModes, const VideoModeComparator comparator );

/**
*	Given a display, fills the vector videoModes with all supported video modes, comparing video modes using the resolution only.
*	@param display: Display whose supported video modes will be queried.
*	@param videoModes: Destination vector that will contain all video modes.
*	@return true on success, false otherwise.
*/
bool GetUniqueVideoModes( const wxDisplay& display, std::vector<wxVideoMode>& videoModes );

/**
*	Converts a Color instance to wxColor.
*/
wxColor ColorTowx( const Color& color );

/**
*	Converts a wxColor instance to Color.
*/
Color wxToColor( const wxColor& color );

/**
*	Launches the default program for a given filetype and passes the given parameters.
*	@param szExtension The extension whose default program will be launched.
*	@param szParameters Parameters to pass to the program.
*	@return true on success, false otherwise.
*/
bool LaunchDefaultProgram( const wxString& szExtension, const wxString& szParameters );

/**
*	Launches the default text editor and opens the given file.
*	@param szFilename File to open.
*	@return true on success, false otherwise.
*/
bool LaunchDefaultTextEditor( const wxString& szFilename );
}

#endif //UI_UTILITY_WXUTIL_H