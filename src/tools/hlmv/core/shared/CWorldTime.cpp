#include "CWorldTime.h"

CWorldTime WorldTime;

void CWorldTime::TimeChanged( const double flCurrentTime )
{
	double flFrameTime = flCurrentTime - WorldTime.GetPreviousRealTime();

	if( flFrameTime > 1.0 )
		flFrameTime = 0.1;

	SetPreviousTime( WorldTime.GetCurrentTime() );
	SetCurrentTime( static_cast<float>( WorldTime.GetCurrentTime() + flFrameTime ) );
	SetFrameTime( static_cast<float>( flFrameTime ) );
	SetPreviousRealTime( WorldTime.GetRealTime() );
}