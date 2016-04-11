#include <algorithm>

#include "wxUtil.h"

namespace wx
{
/**
*	Compares video modes using the resolutions only.
*	@return true if the modes are identical, false otherwise.
*/
bool CompareVideoModeResolutions( const wxVideoMode& lhs, const wxVideoMode& rhs )
{
	return lhs.GetWidth() == rhs.GetWidth() && lhs.GetHeight() == lhs.GetHeight();
}

/**
*	Given a display, fills the vector videoModes with all supported video modes, using comparator to compare video modes.
*	@param display Display whose supported video modes will be queried.
*	@param videoModes Destination vector that will contain all video modes.
*	@param comparator Comparator to use when filtering the list.
*	@return true on success, false otherwise.
*/
bool GetUniqueVideoModes( const wxDisplay& display, std::vector<wxVideoMode>& videoModes, const VideoModeComparator comparator )
{
	videoModes.clear();

	const wxArrayVideoModes modes = display.GetModes();

	for( size_t uiIndex = 0; uiIndex < modes.Count(); ++uiIndex )
	{
		videoModes.push_back( modes[ uiIndex ] );
	}

	videoModes.erase( std::unique( videoModes.begin(), videoModes.end(), comparator ), videoModes.end() );

	return true;
}

/**
*	Given a display, fills the vector videoModes with all supported video modes, comparing video modes using the resolution only.
*	@param display: Display whose supported video modes will be queried.
*	@param videoModes: Destination vector that will contain all video modes.
*	@return true on success, false otherwise.
*/
bool GetUniqueVideoModes( const wxDisplay& display, std::vector<wxVideoMode>& videoModes )
{
	return GetUniqueVideoModes( display, videoModes, &CompareVideoModeResolutions );
}
}