#include <array>
#include <random>

#include "Utility.h"

namespace
{
static std::mt19937 g_Random;

static const
class CInitRandom final
{
public:
	CInitRandom()
	{
		std::random_device rd;

		std::array<int, 624> seed_data;

		std::generate( seed_data.begin(), seed_data.end(), std::ref( rd ) );

		std::seed_seq seq( seed_data.begin(), seed_data.end() );

		g_Random.seed( seq );
	}

private:
	CInitRandom( const CInitRandom& ) = delete;
	CInitRandom& operator=( const CInitRandom& ) = delete;
} g_Init;
}

int UTIL_RandomLong( int iLow, int iHigh )
{
	if( iHigh < iLow )
		std::swap( iLow, iHigh );

	std::uniform_int_distribution<int> dist( iLow, iHigh );

	return dist( g_Random );
}

float UTIL_RandomFloat( float flLow, float flHigh )
{
	if( flLow < flHigh )
		std::swap( flLow, flHigh );

	std::uniform_real_distribution<float> dist( flLow, flHigh );

	return dist( g_Random );
}