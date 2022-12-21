#pragma once

#include <algorithm>

#include "entity/BaseEntity.hpp"

/**
*	Base class for animating entities.
*/
class BaseAnimating : public BaseEntity
{
public:
	DECLARE_CLASS(BaseAnimating, BaseEntity);

protected:
	float _frame = 0;
	float _frameRate = 1;

public:
	float GetFrame() const { return _frame; }

	float GetFrameRate() const { return _frameRate; }

	void SetFrameRate(const float frameRate)
	{
		_frameRate = std::max(0.f, frameRate);
	}
};
