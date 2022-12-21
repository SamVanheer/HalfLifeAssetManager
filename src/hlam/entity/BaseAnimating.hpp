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
	float	_frame = 0;	// frame
	float	_frameRate = 1;	//Framerate.

public:
	/**
	*	Gets the frame number.
	*/
	float GetFrame() const { return _frame; }

	/**
	*	Gets the frame rate.
	*/
	float GetFrameRate() const { return _frameRate; }

	/**
	*	Sets the frame rate.
	*	TODO: prevent negative?
	*/
	void SetFrameRate(const float frameRate)
	{
		_frameRate = std::max(0.f, frameRate);
	}
};
