#pragma once

#include <random>

/**
*	@brief Random number generator
*/
class Random final
{
public:
	Random();
	Random(int seed);

	/**
	*	@brief Gets a random 32 bit integer number in the range [iLow, iHigh]
	*/
	int Next(int low, int high);

	/**
	*	@brief Gets a random 64 bit floating point number in the range [flLow, flHigh]
	*/
	double NextDouble(double low, double high);

private:
	std::mt19937 _random;
};
