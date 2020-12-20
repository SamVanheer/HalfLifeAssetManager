#pragma once

#include "shared/Const.hpp"

#include "CBaseEntity.hpp"

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
	/**
	*	Gets the frame number.
	*/
	float GetFrame() const { return m_flFrame; }

	/**
	*	Gets the frame rate.
	*/
	float GetFrameRate() const { return m_flFrameRate; }

	/**
	*	Sets the frame rate.
	*	TODO: prevent negative?
	*/
	void SetFrameRate( const float flFrameRate ) { m_flFrameRate = flFrameRate; }
};
