#ifndef GAME_CBASEANIMATING_H
#define GAME_CBASEANIMATING_H

#include "common/Const.h"

#include "CBaseEntity.h"

/**
*	Base class for animating entities.
*/
class CBaseAnimating : public CBaseEntity
{
public:
	DECLARE_CLASS( CBaseAnimating, CBaseEntity );

protected:
	float	m_flFrame		= 0;	// frame
	float	m_flFrameRate	= 1;	//Framerate.

public:
	float GetFrame() const { return m_flFrame; }

	float GetFrameRate() const { return m_flFrameRate; }
	void SetFrameRate( const float flFrameRate ) { m_flFrameRate = flFrameRate; }
};

#endif //GAME_CBASEANIMATING_H