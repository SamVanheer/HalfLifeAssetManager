#include <chrono>

#include "Platform.h"

//The chrono API is just too confusing with namespaces
using namespace std::chrono;

long long GetCurrentTick()
{
	return duration_cast<milliseconds>( high_resolution_clock::now().time_since_epoch() ).count();
}

double GetCurrentTime()
{
	return GetCurrentTick() / 1000.0;
}