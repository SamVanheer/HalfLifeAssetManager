#ifndef COMMON_CWORLDTIME_H
#define COMMON_CWORLDTIME_H

/**
*	Represents the time in the world. Multiple time values are tracked.
*/
class CWorldTime final
{
public:
	CWorldTime() = default;
	CWorldTime( const CWorldTime& ) = default;
	CWorldTime& operator=( const CWorldTime& ) = default;

	/**
	*	The current time. Starts at 1.0.
	*/
	float GetCurrentTime() const { return m_flCurrentTime; }

	/**
	*	Sets the current time. Avoid using this.
	*/
	void SetCurrentTime( const float flCurrentTime ) { m_flCurrentTime = flCurrentTime; }

	/**
	*	Gets the previous current time before the last time increment. Equal to GetCurrentTime() - GetFrameTime().
	*/
	float GetPreviousTime() const { return m_flPrevTime; }

	/**
	*	Sets the previous time. Avoid using this.
	*/
	void SetPreviousTime( const float flPrevTime ) { m_flPrevTime = flPrevTime; }

	/**
	*	Gets the time between frames.
	*/
	float GetFrameTime() const { return m_flFrameTime; }

	/**
	*	Sets the time between frames. Avoid using this.
	*/
	void SetFrameTime( const float flFrameTime ) { m_flFrameTime = flFrameTime; }

	/**
	*	Gets the current real time. This is the system time at the time when the frame began.
	*/
	double GetRealTime() const { return m_flRealTime; }

	/**
	*	Sets the real time. Avoid using this.
	*/
	void SetRealTime( const double flRealTime ) { m_flRealTime = flRealTime; }

	/**
	*	Gets the previous real time. Equal to GetRealTime() - GetFrameTime().
	*/
	double GetPreviousRealTime() const { return m_flPrevRealTime; }

	/**
	*	Sets the previous real time. Avoid using this.
	*/
	void SetPreviousRealTime( const double flRealTime ) { m_flPrevRealTime = flRealTime; }

	/**
	*	Call with the new current time to update world time.
	*/
	void TimeChanged( const double flCurrentTime );

private:
	float m_flCurrentTime	= 1.0f;
	float m_flPrevTime		= 1.0f;
	float m_flFrameTime		= 0.0f;
	double m_flRealTime		= 0.0f;
	double m_flPrevRealTime = 0.0f;
};

#endif //COMMON_CWORLDTIME_H
