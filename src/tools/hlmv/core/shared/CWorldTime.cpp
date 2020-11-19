#include "CWorldTime.h"

void CWorldTime::TimeChanged( const double flCurrentTime )
{
	double flFrameTime = flCurrentTime - GetPreviousRealTime();

	if( flFrameTime > 1.0 )
		flFrameTime = 0.1;

	SetPreviousTime( GetCurrentTime() );
	SetCurrentTime( static_cast<float>( GetCurrentTime() + flFrameTime ) );
	SetFrameTime( static_cast<float>( flFrameTime ) );
	SetPreviousRealTime( GetRealTime() );
}