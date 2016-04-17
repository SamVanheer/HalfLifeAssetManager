#ifndef COMMON_CGLOBALS_H
#define COMMON_CGLOBALS_H

class CGlobals final
{
public:
	CGlobals() = default;
	CGlobals( const CGlobals& ) = default;
	CGlobals& operator=( const CGlobals& ) = default;

	float GetCurrentTime() const { return m_flCurrentTime; }

	void SetCurrentTime( const float flCurrentTime ) { m_flCurrentTime = flCurrentTime; }

	float GetPreviousTime() const { return m_flPrevTime; }

	void SetPreviousTime( const float flPrevTime ) { m_flPrevTime = flPrevTime; }

	float GetFrameTime() const { return m_flFrameTime; }

	void SetFrameTime( const float flFrameTime ) { m_flFrameTime = flFrameTime; }

	double GetRealTime() const { return m_flRealTime; }

	void SetRealTime( const double flRealTime ) { m_flRealTime = flRealTime; }

	double GetPreviousRealTime() const { return m_flPrevRealTime; }

	void SetPreviousRealTime( const double flRealTime ) { m_flPrevRealTime = flRealTime; }

private:
	float m_flCurrentTime	= 1.0f;
	float m_flPrevTime		= 1.0f;
	float m_flFrameTime		= 0.0f;
	double m_flRealTime		= 0.0f;
	double m_flPrevRealTime = 0.0f;
};

extern CGlobals Globals;

#endif //COMMON_CGLOBALS_H