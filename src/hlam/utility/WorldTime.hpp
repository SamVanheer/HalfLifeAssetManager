#pragma once

/**
*	@brief Represents the time in the world. Multiple time values are tracked.
*/
class WorldTime final
{
public:
	WorldTime() = default;
	WorldTime(const WorldTime&) = default;
	WorldTime& operator=(const WorldTime&) = default;

	/**
	*	@brief The current time. Starts at 1.0.
	*/
	float GetTime() const { return _currentTime; }

	/**
	*	@brief Sets the current time. Avoid using this.
	*/
	void SetTime(float currentTime) { _currentTime = currentTime; }

	/**
	*	@brief Gets the previous current time before the last time increment. Equal to GetCurrentTime() - GetFrameTime().
	*/
	float GetPreviousTime() const { return _prevTime; }

	/**
	*	@brief Sets the previous time. Avoid using this.
	*/
	void SetPreviousTime(float prevTime) { _prevTime = prevTime; }

	/**
	*	@brief Gets the time between frames.
	*/
	float GetFrameTime() const { return _frameTime; }

	/**
	*	@brief Sets the time between frames. Avoid using this.
	*/
	void SetFrameTime(float frameTime) { _frameTime = frameTime; }

	/**
	*	@brief Gets the current real time. This is the system time at the time when the frame began.
	*/
	double GetRealTime() const { return _realTime; }

	/**
	*	@brief Sets the real time. Avoid using this.
	*/
	void SetRealTime(double realTime) { _realTime = realTime; }

	/**
	*	@brief Gets the previous real time. Equal to GetRealTime() - GetFrameTime().
	*/
	double GetPreviousRealTime() const { return _prevRealTime; }

	/**
	*	@brief Sets the previous real time. Avoid using this.
	*/
	void SetPreviousRealTime(double realTime) { _prevRealTime = realTime; }

	/**
	*	@brief Call with the new current time to update world time.
	*/
	void TimeChanged(double currentTime);

private:
	float _currentTime = 1.0f;
	float _prevTime = 1.0f;
	float _frameTime = 0.0f;
	double _realTime = 0.0f;
	double _prevRealTime = 0.0f;
};
