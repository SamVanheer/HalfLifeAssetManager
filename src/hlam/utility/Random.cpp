#include <algorithm>
#include <array>

#include "utility/Random.hpp"

Random::Random()
{
	std::random_device rd;

	std::array<int, 624> seed_data{};

	std::generate(seed_data.begin(), seed_data.end(), std::ref(rd));

	std::seed_seq seq(seed_data.begin(), seed_data.end());

	_random.seed(seq);
}

Random::Random(int seed)
	: _random(seed)
{
}

int Random::Next(int low, int high)
{
	if (high < low)
	{
		std::swap(low, high);
	}

	std::uniform_int_distribution<int> dist(low, high);

	return dist(_random);
}

double Random::NextDouble(double low, double high)
{
	if (low < high)
	{
		std::swap(low, high);
	}

	std::uniform_real_distribution<double> dist(low, high);

	return dist(_random);
}
