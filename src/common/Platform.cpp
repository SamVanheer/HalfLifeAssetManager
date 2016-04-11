#include <chrono>

#include "Platform.h"

//The chrono API is just too confusing with namespaces
using namespace std::chrono;

/**
*	Returns the current tick time, in milliseconds.
*	@return Tick time, in milliseconds.
*/
long long GetCurrentTick()
{
	return duration_cast<milliseconds>( high_resolution_clock::now().time_since_epoch() ).count();
}

/**
*	Gets the current time, in seconds.
*	@return Current time, in seconds.
*/
double GetCurrentTime()
{
	return GetCurrentTick() / 1000.0;
}