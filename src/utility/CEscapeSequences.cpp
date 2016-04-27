#include <cassert>
#include <memory>

#include "common/Utility.h"

#include "CEscapeSequences.h"

CEscapeSequences::CEscapeSequences( const char cDelimiterChar, const size_t uiCount, const ConversionData_t* const pData )
	: m_cDelimiterChar( cDelimiterChar )
	, m_uiCount( uiCount )
{
	assert( pData );

	memset( m_Infos, 0, sizeof( m_Infos ) );
	memset( m_IndexToSeq, 0, sizeof( m_IndexToSeq ) );

	for( size_t uiIndex = 0; uiIndex < m_uiCount; ++uiIndex )
	{
		m_IndexToSeq[ uiIndex ] = pData[ uiIndex ].cEscapeSequence;

		ConversionInfo_t& info = m_Infos[ m_IndexToSeq[ uiIndex ] ];

		assert( info.pszString == nullptr );

		info.pszString = pData[ uiIndex ].pszString;
		info.uiLength = strlen( info.pszString );
	}
}

const char* CEscapeSequences::GetString( const char cEscapeSequence ) const
{
	return m_Infos[ static_cast<size_t>( cEscapeSequence ) ].pszString;
}

size_t CEscapeSequences::GetStringLength( const char cEscapeSequence ) const
{
	return m_Infos[ static_cast<size_t>( cEscapeSequence ) ].uiLength;
}

char CEscapeSequences::GetEscapeSequence( const char* const pszString ) const
{
	assert( pszString );

	for( size_t uiIndex = 0; uiIndex < m_uiCount; ++uiIndex )
	{
		const ConversionInfo_t& info = m_Infos[ m_IndexToSeq[ uiIndex ] ];

		if( info.pszString && strcmp( info.pszString, pszString ) == 0 )
			return m_IndexToSeq[ uiIndex ];
	}

	return '\0';
}

BEGIN_ESCAPE_SEQ_LIST( EscapeSequences )
	{ '\"', "\\\"" },
	{ '\'', "\\\'" },
	{ '\\', "\\\\" },
	{ '\t', "\\t" },
	{ '\n', "\\n" },
	{ '\a', "\\a" },
	{ '\b', "\\b" },
	{ '\f', "\\f" },
	{ '\v', "\\v" },
	{ '\0', "\\0" }
END_ESCAPE_SEQ_LIST( EscapeSequences, '\\' );

CEscapeSequences& GetEscapeSeqConversion()
{
	return EscapeSequences;
}

//0x7F is an unprintable character
BEGIN_ESCAPE_SEQ_LIST( NoEscapeSequences )
	{ 0x7F, "" }
END_ESCAPE_SEQ_LIST( NoEscapeSequences, 0x7F );

CEscapeSequences& GetNoEscapeSeqConversion()
{
	return NoEscapeSequences;
}