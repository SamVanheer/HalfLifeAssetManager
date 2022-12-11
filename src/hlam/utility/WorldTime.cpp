#include "utility/WorldTime.hpp"

void WorldTime::TimeChanged(double currentTime)
{
	double frameTime = currentTime - GetPreviousRealTime();

	if (frameTime > 1.0)
		frameTime = 0.1;

	SetPreviousTime(GetTime());
	SetTime(static_cast<float>(GetTime() + frameTime));
	SetFrameTime(static_cast<float>(frameTime));
	SetPreviousRealTime(GetRealTime());
}
