#include <cassert>
#include <cstring>
#include <algorithm>
#include <string>

#include "common/Platform.h"
#include "common/Logging.h"

#include "CString.h"

namespace String
{
int CompareCharacters( const char char1, const char char2, const CompareType compare )
{
	return compare == CaseSensitive ? char1 - char2 : tolower( char1 ) - tolower( char2 );
}

int Compare( const char* pszString1, const char* pszString2, const CompareType compare )
{
	return ( compare == CaseSensitive ? strcmp : strcasecmp )( pszString1, pszString2 );
}

int CompareN( const char* pszString1, const char* pszString2, const size_t uiCount, const CompareType compare )
{
	return ( compare == CaseSensitive ? strncmp : strncasecmp )( pszString1, pszString2, uiCount );
}

char BoolToCharacter( const bool fValue )
{
	return fValue ? '1' : '0';
}

char* TrimTrailingCharacters( char* pszString, const char character )
{
	return TrimTrailingCharacters( pszString, character, strlen( pszString ) );
}

char* TrimTrailingCharacters( char* pszString, const char character, size_t uiLength )
{
	assert( pszString );

	const size_t uiStringLength = strlen( pszString );

	if( uiLength > uiStringLength )
		uiLength = uiStringLength;

	if( uiLength == 0 )
		return pszString;

	--uiLength;

	while( uiLength > 0 )
	{
		if( pszString[ uiLength ] != character )
			break;

		pszString[ uiLength ] = '\0';

		--uiLength;
	}

	return pszString;
}

char* TrimLeadingCharacters( char* pszString, const char character )
{
	return TrimLeadingCharacters( pszString, character, strlen( pszString ) );
}

char* TrimLeadingCharacters( char* pszString, const char character, size_t uiCount )
{
	assert( pszString );

	const size_t uiStringLength = strlen( pszString );

	if( uiCount > uiStringLength )
		uiCount = uiStringLength;

	if( uiCount == 0 )
		return pszString;

	char* pszBegin = pszString;

	//Point to the last character to prevent emptying string
	const char* pszEnd = pszString + uiCount - 1;

	//Find first non-'character' character in the string
	while( pszBegin != pszEnd )
	{
		if( *pszBegin != character )
			break;

		++pszBegin;
	}

	//Leading characters found, move the string to trim them
	if( pszBegin != pszString )
		memmove( pszString, pszBegin, ( ( pszString + uiStringLength ) - pszBegin ) + 1 ); //+ 1 because we did -1 earlier

	return pszString;
}

char* TrimCharacters( char* pszString, const char character )
{
	assert( pszString );

	//Trim trailing first; potentially reduces characters to move later
	TrimTrailingCharacters( pszString, character );
	TrimLeadingCharacters( pszString, character );

	return pszString;
}
}

const CString::size_type CString::INVALID_INDEX = ( CString::size_type ) -1;

const CString CString::EMPTY_STRING = "";
const CString CString::WHITESPACE_CHARACTERS = " \t\n\r\v";

void CString::Construct()
{
	m_szBuffer[ 0 ] = '\0';

	m_pszString = m_szBuffer;

	m_uiLength = 0;

	SetCapacity( BUFFER_SIZE );
	SetStatic( false );
}

CString::CString()
{
	Construct();
}

CString::CString( const char* pszString )
{
	Construct();

	*this = pszString;
}

CString::CString( const char* pszString, size_type uiBegin, size_type uiCount )
{
	Construct();

	Assign( pszString, uiBegin, uiCount );
}

CString::CString( const CString& other, size_type uiBegin, size_type uiCount )
{
	Construct();

	Assign( other.CStr(), uiBegin, uiCount );
}

CString::CString( const CString& other )
{
	Construct();

	*this = other;
}

CString::CString( char* pszString, const bool bTakeOwnership )
{
	Construct();

	if( bTakeOwnership )
		TakeOwnership( pszString );
	else
		*this = pszString;
}

CString& CString::TakeOwnership( char* pszString )
{
	if( m_pszString && m_pszString != m_szBuffer )
	{
		delete[] m_pszString;
		m_pszString = nullptr;
	}

	if( pszString )
	{
		m_uiLength = strlen( pszString );

		m_pszString = pszString;

		SetCapacity( m_uiLength + 1 );
	}
	else
	{
		Error( "CString::TakeOwnership: null pointer passed; can't take ownership!\n" );
		*this = ""; //Make sure the string buffer is valid.
	}

	return *this;
}

CString::CString( const bool fValue )
{
	Construct();

	*this = fValue;
}

CString::CString( const char character )
{
	Construct();

	*this = character;
}

CString::CString( const int iValue )
{
	Construct();

	*this = iValue;
}

CString::CString( const unsigned int uiValue )
{
	Construct();

	*this = uiValue;
}

CString::CString( const long long int iValue )
{
	Construct();

	*this = iValue;
}

CString::CString( const unsigned long long int uiValue )
{
	Construct();

	*this = uiValue;
}

CString::CString( const float flValue )
{
	Construct();

	*this = flValue;
}

CString::CString( const double flValue )
{
	Construct();

	*this = flValue;
}

void CString::Assign( const char* pszString, const size_t iLength )
{
	//Always make valid
	if( !pszString )
		pszString = "";

	if( pszString == m_pszString )
		return;

	Reserve( iLength, false );

	strcpy( m_pszString, pszString );

	m_uiLength = iLength;
}

CString& CString::operator=( const char* pszString )
{
	assert( pszString );

	if( pszString )
	{
		Assign( pszString, strlen( pszString ) );
	}

	return *this;
}

CString& CString::Assign( const char* pszString, size_type uiBegin, size_type uiCount )
{
	Clear();

	return Append( pszString, uiBegin, uiCount );
}

CString& CString::Assign( const CString& other, size_type uiBegin, size_type uiCount )
{
	return Assign( other.CStr(), uiBegin, uiCount );
}

CString& CString::operator=( const CString& other )
{
	assert( this != &other );

	Assign( other.CStr(), other.Length() );

	return *this;
}

CString& CString::operator=( const bool fValue )
{
	return ( *this = String::BoolToCharacter( fValue ) );
}

CString& CString::operator=( const char character )
{
	char szString[ 2 ] = { character, '\0' };

	return ( *this = szString );
}

CString& CString::operator=( const int iValue )
{
	char szBuffer[ MINIMUM_PRINTF_BUFFER_SIZE ];

	const int iLength = snprintf( szBuffer, sizeof( szBuffer ), "%d", iValue );

	Assign( szBuffer, iLength );

	return *this;
}

CString& CString::operator=( const unsigned int uiValue )
{
	char szBuffer[ MINIMUM_PRINTF_BUFFER_SIZE ];

	const int iLength = snprintf( szBuffer, sizeof( szBuffer ), "%u", uiValue );

	Assign( szBuffer, iLength );

	return *this;
}

CString& CString::operator=( const long long int iValue )
{
	char szBuffer[ MINIMUM_PRINTF_BUFFER_SIZE ];

	const int iLength = snprintf( szBuffer, sizeof( szBuffer ), "%lld", iValue );

	Assign( szBuffer, iLength );

	return *this;
}

CString& CString::operator=( const unsigned long long int uiValue )
{
	char szBuffer[ MINIMUM_PRINTF_BUFFER_SIZE ];

	const int iLength = snprintf( szBuffer, sizeof( szBuffer ), "%llu", uiValue );

	Assign( szBuffer, iLength );

	return *this;
}

CString& CString::operator=( const float flValue )
{
	return *this = ( ( double ) flValue );
}

CString& CString::operator=( const double flValue )
{
	char szBuffer[ MINIMUM_PRINTF_BUFFER_SIZE ];

	int iLength = snprintf( szBuffer, sizeof( szBuffer ), "%f", flValue );

	String::TrimTrailingCharacters( szBuffer, '0', iLength );
	String::TrimTrailingCharacters( szBuffer, '.', iLength );

	//Might have changed after trimming
	iLength = strlen( szBuffer );

	Assign( szBuffer, iLength );

	return *this;
}

CString::~CString()
{
	//Don't use ClearMemory; that is designed to reset state, which we don't need
	if( GetDynamicAllocation() )
		delete[] m_pszString;
}

CString::size_type CString::GetDynamicAllocation() const
{
	return m_pszString != m_szBuffer && !IsStatic() ? GetCapacity() : 0;
}

void CString::Resize( size_type iNewSize, bool fKeepData )
{
	assert( !IsStatic() );

	//Account for null terminator
	++iNewSize;

	if( GetCapacity() == iNewSize || IsStatic() )
		return;

	//Always dynamically allocate when resizing
	char* pszBuffer = new char[ iNewSize ];

	if( fKeepData && m_pszString )
	{
		//The buffer might not contain a valid string
		m_pszString[ Length() ] = '\0';

		//Copy only the required number of characters
		strncpy( pszBuffer, m_pszString, iNewSize );
		pszBuffer[ iNewSize - 1 ] = '\0';
	}

	if( m_pszString != m_szBuffer )
		delete[] m_pszString;

	m_pszString = pszBuffer;

	SetCapacity( iNewSize );
}

void CString::Reserve( size_type iMinimum, bool fKeepData )
{
	if( GetCapacity() > iMinimum )
		return;

	Resize( iMinimum, fKeepData );
}

void CString::Clear()
{
	Reserve( 0 );

	m_pszString[ 0 ] = '\0';

	m_uiLength = 0;
}

void CString::ClearMemory()
{
	if( GetDynamicAllocation() )
	{
		delete[] m_pszString;

		//Reset to initial state
		Construct();
	}
}

void CString::RecalculateLength()
{
	m_uiLength = strlen( m_pszString );
}

char CString::CharAt( size_type uiIndex ) const
{
	if(  uiIndex >= Length() )
	{
		return m_pszString[ 0 ];
	}

	return m_pszString[ uiIndex ];
}

void CString::SetCharAt( size_type uiIndex, char character )
{
	if(  uiIndex >= Length() )
		return;

	m_pszString[ uiIndex ] = character;
}

const char& CString::operator[]( size_t uiIndex ) const
{
	assert( uiIndex < m_uiLength );

	return m_pszString[ uiIndex ];
}

char& CString::operator[]( size_t uiIndex )
{
	assert( uiIndex < m_uiLength );

	return m_pszString[ uiIndex ];
}

void CString::Append( const char* pszString, const size_type iLength )
{
	assert( pszString );

	if( iLength )
	{
		//Reserve copies the current string
		Reserve( m_uiLength + iLength );

		strcpy( m_pszString + m_uiLength, pszString );

		m_uiLength += iLength;
	}
}

CString& CString::operator+=( const char* pszString )
{
	Append( pszString, strlen( pszString ) );

	return *this;
}

CString& CString::Append( const char* pszString, size_type uiBegin, size_type uiCount )
{
	assert( pszString );

	const size_t uiLength = strlen( pszString );

	if( uiLength == 0 )
	{
		Append( "", 0 );

		return *this;
	}

	if( uiBegin >= uiLength )
		uiBegin = uiLength - 1;

	if( uiCount == INVALID_INDEX )

	if( uiCount > uiLength - uiBegin )
		uiCount = uiLength - uiBegin;

	Reserve( Length() + uiCount );

	char* pszDest = End();

	pszString += uiBegin;

	m_uiLength += uiCount;

	while( uiCount-- )
	{
		*pszDest = *pszString;

		++pszDest;
		++pszString;
	}

	*pszDest = '\0';

	return *this;
}

CString& CString::Append( const CString& other, size_type uiBegin, size_type uiCount )
{
	return Append( other.CStr(), uiBegin, uiCount );
}

CString& CString::operator+=( const CString& other )
{
	Append( other.CStr(), other.Length() );

	return *this;
}

CString& CString::operator+=( const bool fValue )
{
	return ( *this += String::BoolToCharacter( fValue ) );
}

CString& CString::operator+=( const char character )
{
	char szBuffer[ 2 ] = { character, '\0' };

	return ( *this += szBuffer );
}

CString& CString::operator+=( const int iValue )
{
	char szBuffer[ MINIMUM_PRINTF_BUFFER_SIZE ];

	snprintf( szBuffer, sizeof( szBuffer ), "%i", iValue );

	return ( *this += szBuffer );
}

CString& CString::operator+=( const unsigned int uiValue )
{
	char szBuffer[ MINIMUM_PRINTF_BUFFER_SIZE ];

	snprintf( szBuffer, sizeof( szBuffer ), "%u", uiValue );

	return ( *this += szBuffer );
}

CString& CString::operator+=( const long long int iValue )
{
	char szBuffer[ MINIMUM_PRINTF_BUFFER_SIZE ];

	snprintf( szBuffer, sizeof( szBuffer ), "%lli", iValue );

	return ( *this += szBuffer );
}

CString& CString::operator+=( const unsigned long long int uiValue )
{
	char szBuffer[ MINIMUM_PRINTF_BUFFER_SIZE ];

	snprintf( szBuffer, sizeof( szBuffer ), "%llu", uiValue );

	return ( *this += szBuffer );
}

CString& CString::operator+=( const float flValue )
{
	return ( *this += ( ( double ) flValue ) );
}

CString& CString::operator+=( const double flValue )
{
	char szBuffer[ MINIMUM_PRINTF_BUFFER_SIZE ];

	const int iLength = snprintf( szBuffer, sizeof( szBuffer ), "%f", flValue );

	String::TrimTrailingCharacters( szBuffer, '0', iLength );
	String::TrimTrailingCharacters( szBuffer, '.', iLength );

	return ( *this += szBuffer );
}

int CString::Compare( const char* pszString ) const
{
	return strcmp( CStr(), pszString );
}

int CString::Compare( const CString& other ) const
{
	return Compare( other.CStr() );
}

int CString::CompareN( const char* pszString, const size_type uiCount ) const
{
	return strncmp( CStr(), pszString, uiCount );
}

int CString::CompareN( const CString& other, const size_type uiCount ) const
{
	return CompareN( other.CStr(), uiCount );
}

int CString::ICompare( const char* pszString ) const
{
	return strcasecmp( CStr(), pszString );
}

int CString::ICompare( const CString& other ) const
{
	return ICompare( other.CStr() );
}

int CString::ICompareN( const char* pszString, const size_type uiCount ) const
{
	return strncasecmp( CStr(), pszString, uiCount );
}

int CString::ICompareN( const CString& other, const size_type uiCount ) const
{
	return ICompareN( other.CStr(), uiCount );
}

bool CString::operator==( const char* pszString ) const
{
	return Compare( pszString ) == 0;
}

bool CString::operator==( const CString& other ) const
{
	return Compare( other ) == 0;
}

bool CString::operator!=( const char* pszString ) const
{
	return !( *this == pszString );
}

bool CString::operator!=( const CString& other ) const
{
	return !( *this == other );
}

void CString::Trim( const char character )
{
	String::TrimCharacters( CStr(), character );

	//Adjust length to match
	m_uiLength = strlen( CStr() );
}

bool CString::StartsWith( const char* pszString, const String::CompareType compare ) const
{
	assert( pszString );

	return String::CompareN( m_pszString, pszString, strlen( pszString ), compare ) == 0;
}

bool CString::StartsWith( const CString& str, const String::CompareType compare ) const
{
	return StartsWith( str.CStr(), compare );
}

bool CString::EndsWith( const char* pszString, const String::CompareType compare ) const
{
	assert( pszString );

	const size_t uiLength = strlen( pszString );

	if( Length() < uiLength )
		return false;

	return String::CompareN( CStr() + ( Length() - uiLength ), pszString, uiLength, compare ) == 0;
}

bool CString::EndsWith( const CString& str, const String::CompareType compare ) const
{
	return EndsWith( str.CStr(), compare );
}

CString::size_type CString::Find( const char character, const size_type uiStartOffset, const String::CompareType compare ) const
{
	for( size_type uiIndex = uiStartOffset; uiIndex < Length(); ++uiIndex )
	{
		if( String::CompareCharacters( character, m_pszString[ uiIndex ], compare ) == 0 )
			return uiIndex;
	}

	return INVALID_INDEX;
}

CString::size_type CString::Find( const char* pszString, const size_type uiStartOffset, const String::CompareType compare ) const
{
	assert( pszString );

	const size_t uiLength = strlen( pszString );

	if( Length() < uiLength )
		return INVALID_INDEX;

	for( size_type uiIndex = uiStartOffset; uiIndex < Length() - ( uiLength - 1 ); ++uiIndex )
	{
		if( String::CompareN( m_pszString + uiIndex, pszString, uiLength, compare ) == 0 )
			return uiIndex;
	}

	return INVALID_INDEX;
}

CString::size_type CString::Find( const CString& str, const size_type uiStartOffset, const String::CompareType compare ) const
{
	return Find( str.CStr(), uiStartOffset, compare );
}

CString::size_type CString::RFind( const char character, const size_type uiStartOffset, const String::CompareType compare ) const
{
	if( IsEmpty() )
		return INVALID_INDEX;

	size_type uiIndex = Length();

	if( uiIndex <= uiStartOffset )
		return INVALID_INDEX;

	uiIndex -= uiStartOffset;

	do
	{
		--uiIndex;

		if( String::CompareCharacters( character, m_pszString[ uiIndex ], compare ) == 0 )
			return uiIndex;
	}
	while( uiIndex > 0 );

	return INVALID_INDEX;
}

CString::size_type CString::RFind( const char* pszString, const size_type uiStartOffset, const String::CompareType compare ) const
{
	assert( pszString );

	if( !( *pszString ) )
		return INVALID_INDEX;

	const size_t uiLength = strlen( pszString );

	if( Length() < uiLength )
		return INVALID_INDEX;

	size_type uiIndex = ( Length() - uiLength ) + 1;

	if( uiIndex <= uiStartOffset )
		return INVALID_INDEX;

	uiIndex -= uiStartOffset;

	do
	{
		--uiIndex;

		if( String::CompareN( m_pszString + uiIndex, pszString, uiLength, compare ) == 0 )
			return uiIndex;
	}
	while( uiIndex > 0 );

	return INVALID_INDEX;
}

CString::size_type CString::RFind( const CString& str, const size_type uiStartOffset, const String::CompareType compare ) const
{
	return RFind( str.CStr(), uiStartOffset, compare );
}

CString::size_type CString::FindFirstOf( const char* pszCharacters, const size_type uiStartOffset, const String::CompareType compare ) const
{
	assert( pszCharacters );

	return FindFirstOf( CString( pszCharacters ), uiStartOffset, compare );
}

CString::size_type CString::FindFirstOf( const CString& szCharacters, const size_type uiStartOffset, const String::CompareType compare ) const
{
	if( szCharacters.IsEmpty() )
		return INVALID_INDEX;

	for( size_type uiIndex = uiStartOffset; uiIndex < Length(); ++uiIndex )
	{
		if( szCharacters.Find( m_pszString[ uiIndex ], compare ) != INVALID_INDEX )
			return uiIndex;
	}

	return INVALID_INDEX;
}

CString::size_type CString::FindLastOf( const char* pszCharacters, const size_type uiStartOffset, const String::CompareType compare ) const
{
	assert( pszCharacters );

	return FindLastOf( CString( pszCharacters ), uiStartOffset, compare );
}

CString::size_type CString::FindLastOf( const CString& szCharacters, const size_type uiStartOffset, const String::CompareType compare ) const
{
	if( szCharacters.IsEmpty() || IsEmpty() )
		return INVALID_INDEX;

	size_type uiIndex = Length();

	if( uiIndex <= uiStartOffset )
		return INVALID_INDEX;

	uiIndex -= uiStartOffset;

	do
	{
		--uiIndex;

		if( szCharacters.Find( m_pszString[ uiIndex ], compare ) != INVALID_INDEX )
			return uiIndex;
	}
	while( uiIndex > 0 );

	return INVALID_INDEX;
}

CString::size_type CString::FindFirstNotOf( const char* pszString, size_t uiStartIndex, const String::CompareType compare ) const
{
	if( !pszString )
		return INVALID_INDEX;

	return FindFirstNotOf( CString( pszString ), uiStartIndex, compare );
}

CString::size_type CString::FindFirstNotOf( const CString& str, size_t uiStartIndex, const String::CompareType compare ) const
{
	if( str.IsEmpty() || IsEmpty() )
		return -1;

	const size_t uiMyLength = Length();

	if( uiStartIndex >= uiMyLength )
		return INVALID_INDEX;

	const char* pszMyString = m_pszString + uiStartIndex;

	while( *pszMyString && str.FindFirstOf( *pszMyString, 0, compare ) != INVALID_INDEX )
		++pszMyString;

	return ( *pszMyString ? pszMyString - m_pszString : INVALID_INDEX );
}

CString::size_type CString::FindLastNotOf( const char* pszString, size_t uiStartIndex, const String::CompareType compare ) const
{
	if( !pszString ) 
		return INVALID_INDEX;

	return FindLastNotOf( CString( pszString ), uiStartIndex, compare );
}

CString::size_type CString::FindLastNotOf( const CString& str, size_t uiStartIndex, const String::CompareType compare ) const
{
	if( str.IsEmpty() || IsEmpty() )
		return -1;

	const size_t uiMyLength = Length();

	if( uiStartIndex == INVALID_INDEX )
		uiStartIndex = uiMyLength - 1;
	else if( uiStartIndex >= uiMyLength )
		return INVALID_INDEX;

	while (uiStartIndex != INVALID_INDEX && str.FindLastOf( *( m_pszString + uiStartIndex ) ) != INVALID_INDEX )
		--uiStartIndex;

	return uiStartIndex;
}

void CString::Format( const char* pszFormat, ... )
{
	va_list list;

	va_start( list, pszFormat );

	VFormat( pszFormat, list );

	va_end( list );
}

void CString::VFormat( const char* pszFormat, va_list list )
{
	char szBuffer[ PRINTF_BUFFER_SIZE ];

	const int iLength = vsnprintf( szBuffer, sizeof( szBuffer ), pszFormat, list );

	szBuffer[ sizeof( szBuffer ) - 1 ] = '\0';

	if( iLength < 0 || iLength >= sizeof( szBuffer ) )
	{
		//Error does not use this method, so there is no risk of recursive calls
		Error( "Error formatting string: format used: '%s'\n", pszFormat );
		return;
	}

	*this = szBuffer;
}

CString& CString::ToLowercase()
{
	std::transform( Begin(), End(), Begin(), ::tolower );

	return *this;
}

CString& CString::ToUppercase()
{
	std::transform( Begin(), End(), Begin(), ::toupper );

	return *this;
}

CString CString::SubString( const size_type uiBegin, const size_type uiCount ) const
{
	return CString( *this, uiBegin, uiCount );
}

CString& CString::Replace( const char charToReplace, const char charToSet )
{
	//Can't replace null terminators
	if( charToReplace != '\0' )
	{
		for( auto it = Begin(), end = End(); it != end; ++it )
		{
			if( *it == charToReplace )
				*it = charToSet;

			//Ignore all subsequent characters
			if( charToSet == '\0' )
			{
				m_uiLength = it - Begin();
				break;
			}
		}
	}

	return *this;
}

CString& CString::Replace( const char* pszSubString, const char* pszReplacement, const String::CompareType compare )
{
	assert( pszSubString );
	assert( pszReplacement );

	if( *pszSubString )
	{
		CString szResult;

		const size_t uiLength = strlen( pszSubString );

		for( auto it = Begin(), end = End(); it != end; ++it )
		{
			if( String::CompareN( it, pszSubString, uiLength, compare ) == 0 )
			{
				szResult += pszReplacement;

				//Account for ++it
				it += uiLength - 1;
			}
			else
				szResult += *it;
		}

		*this = szResult;
	}

	return *this;
}

CString& CString::Replace( const CString& szSubString, const CString& szReplacement, const String::CompareType compare )
{
	return Replace( szSubString.CStr(), szReplacement.CStr(), compare );
}

void CString::Truncate( const size_type uiMaxLength )
{
	if( uiMaxLength >= Length() )
		return;

	memset( m_pszString + uiMaxLength, 0, Length() - uiMaxLength );

	m_uiLength = uiMaxLength;
}

void CString::SetStaticString( char* pszString, size_type iLength )
{
	assert( pszString );

	m_pszString = pszString;

	if( iLength == INVALID_INDEX )
		iLength = strlen( pszString );

	m_uiLength = iLength;

	SetStatic( true );
}

CString operator+( const CString& string, const char* pszString )
{
	return CString( string ) += pszString;
}

CString operator+( const CString& string, const CString& other )
{
	return CString( string ) += other;
}

CString operator+( const CString& string, const bool fValue )
{
	return CString( string ) += fValue;
}

CString operator+( const CString& string, const char character )
{
	return CString( string ) += character;
}

CString operator+( const CString& string, const int iValue )
{
	return CString( string ) += iValue;
}

CString operator+( const CString& string, const unsigned int uiValue )
{
	return CString( string ) += uiValue;
}

CString operator+( const CString& string, const long long int iValue )
{
	return CString( string ) += iValue;
}

CString operator+( const CString& string, const unsigned long long int uiValue )
{
	return CString( string ) += uiValue;
}

CString operator+( const CString& string, const float flValue )
{
	return CString( string ) += flValue;
}

CString operator+( const CString& string, const double flValue )
{
	return CString( string ) += flValue;
}

bool operator==( const char* pszString, const CString& other )
{
	return other == pszString;
}

bool operator!=( const char* pszString, const CString& other )
{
	return other != pszString;
}

CStaticString::CStaticString()
{
	SetStaticString( "" );
}

CStaticString::CStaticString( char* pszString )
{
	SetStaticString( pszString );
}