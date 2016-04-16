#ifndef CTIMER_H
#define CTIMER_H

#include "ui/wxInclude.h"

class CTimer;

/**
*	Listener interface for timer events.
*/
class ITimerListener
{
public:
	virtual ~ITimerListener() = 0;

	/**
	*	Override this to be notified when released.
	*/
	virtual void ReleaseTimerListener() {}

	virtual void OnTimer( CTimer& timer ) = 0;
};

inline ITimerListener::~ITimerListener()
{
}

/**
*	Timer that calls a listener when an event occurs.
*/
class CTimer : public wxTimer
{
public:
	CTimer( ITimerListener* pListener );
	~CTimer();

	ITimerListener* GetListener() { return m_pListener; }
	const ITimerListener* GetListener() const { return m_pListener; }

	void SetListener( ITimerListener* pListener );

	void Notify() override;

private:
	ITimerListener* m_pListener;

private:
	CTimer( const CTimer& ) = delete;
	CTimer& operator=( const CTimer& ) = delete;
};

inline CTimer::CTimer( ITimerListener* pListener )
	: wxTimer()
	, m_pListener( pListener )
{
}

inline CTimer::~CTimer()
{
	SetListener( nullptr );
}

inline void CTimer::SetListener( ITimerListener* pListener )
{
	if( m_pListener )
	{
		m_pListener->ReleaseTimerListener();
		m_pListener = nullptr;
	}

	if( pListener )
		m_pListener = pListener;
}

inline void CTimer::Notify()
{
	if( m_pListener )
		m_pListener->OnTimer( *this );
}

#endif //CTIMER_H