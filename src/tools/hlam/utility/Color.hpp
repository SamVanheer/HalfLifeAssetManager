#pragma once

#include <cassert>
#include <string>

#include "shared/Const.hpp"

/**
*	Lightweight Color class.
*/
class Color final
{
public:
	/**
	*	This value represents an alpha value that is fully opaque.
	*/
	static const byte ALPHA_OPAQUE = 0xFF;

	/**
	*	This value represents an alpha value that is fully transparent.
	*/
	static const byte ALPHA_TRANSPARENT = 0x00;

	static const size_t NUM_COMPONENTS = 4;

public:

	explicit Color( const byte rgb = 0, const byte alpha = ALPHA_OPAQUE )
	{
		Set( rgb, alpha );
	}

	Color( const byte red, const byte green, const byte blue, const byte alpha = ALPHA_OPAQUE )
	{
		Set( red, green, blue, alpha );
	}

	Color( const Color& other )
	{
		Set( other );
	}

	Color& operator=( const Color& other )
	{
		Set( other );

		return *this;
	}

	color32_t GetColor32() const
	{
		return *reinterpret_cast<const color32_t*>( this );
	}

	byte GetRed() const
	{
		return m_Red;
	}

	byte GetGreen() const
	{
		return m_Green;
	}

	byte GetBlue() const
	{
		return m_Blue;
	}

	byte GetAlpha() const
	{
		return m_Alpha;
	}

	void Set( const Color& other )
	{
		*reinterpret_cast<color32_t*>( this ) = *reinterpret_cast<const color32_t*>( &other );
	}

	void Set( const color32_t color )
	{
		*reinterpret_cast<color32_t*>( this ) = color;
	}

	void Set( const byte red, const byte green, const byte blue, const byte alpha = ALPHA_OPAQUE )
	{
		m_Red		= red;
		m_Green		= green;
		m_Blue		= blue;
		m_Alpha		= alpha;
	}

	void Set( const byte rgb, const byte alpha = ALPHA_OPAQUE )
	{
		m_Red = m_Green = m_Blue = rgb;
		m_Alpha = alpha;
	}

	void SetRed( const byte red )
	{
		m_Red = red;
	}

	void SetGreen( const byte green )
	{
		m_Green = green;
	}

	void SetBlue( const byte blue )
	{
		m_Blue = blue;
	}

	void SetAlpha( const byte alpha )
	{
		m_Alpha = alpha;
	}

	byte operator[]( const size_t uiIndex ) const
	{
		assert( uiIndex < NUM_COMPONENTS );

		return reinterpret_cast<const byte*>( this )[ uiIndex ];
	}

	byte& operator[]( const size_t uiIndex )
	{
		assert( uiIndex < NUM_COMPONENTS );

		return reinterpret_cast<byte*>( this )[ uiIndex ];
	}

private:
	union
	{
		struct
		{
			byte m_Red;
			byte m_Green;
			byte m_Blue;
			byte m_Alpha;
		};

		byte m_Colors[ NUM_COMPONENTS ];
	};
};

bool ParseColor( const char* pszValue, Color& color, const bool bParseAlpha = false );
std::string ColorToString( const Color& color, const bool bAddAlpha = false );
