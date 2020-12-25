#include "core/shared/WorldTime.hpp"

void WorldTime::TimeChanged( const double flCurrentTime )
{
	double flFrameTime = flCurrentTime - GetPreviousRealTime();

	if( flFrameTime > 1.0 )
		flFrameTime = 0.1;

	SetPreviousTime(GetTime() );
	SetTime( static_cast<float>( GetTime() + flFrameTime ) );
	SetFrameTime( static_cast<float>( flFrameTime ) );
	SetPreviousRealTime( GetRealTime() );
}
