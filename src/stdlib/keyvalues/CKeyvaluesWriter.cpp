#include <cassert>

#include "shared/Logging.h"

#include "shared/Utility.h"

#include "utility/CEscapeSequences.h"

#include "CKeyvalue.h"
#include "CKeyvalueBlock.h"

#include "CKeyvaluesWriter.h"

namespace keyvalues
{
CKeyvaluesWriter::CKeyvaluesWriter( const char* pszFilename, const CKeyvaluesLexerSettings& settings )
	: CKeyvaluesWriter( pszFilename, GetNoEscapeSeqConversion(), settings )
{
}

CKeyvaluesWriter::CKeyvaluesWriter( const char* const pszFilename, CEscapeSequences& escapeSeqConversion, const CKeyvaluesLexerSettings& settings )
	: m_Settings( settings )
	, m_pEscapeSeqConversion( &escapeSeqConversion )
{
	m_szFilename[ 0 ] = '\0';

	Open( pszFilename );
}

CKeyvaluesWriter::~CKeyvaluesWriter()
{
	Close();
}

bool CKeyvaluesWriter::Open( const char* const pszFilename )
{
	assert( pszFilename );

	Close();

	m_pFile = fopen( pszFilename, "w" );

	strncpy( m_szFilename, pszFilename, sizeof( m_szFilename ) );
	m_szFilename[ sizeof( m_szFilename ) - 1 ] = '\0';

	return IsOpen();
}

void CKeyvaluesWriter::Close()
{
	if( IsOpen() )
	{
		fclose( m_pFile );
		m_pFile = nullptr;

		m_szFilename[ 0 ] = '\0';
	}

	m_bErrorOccurred = false;
}

bool CKeyvaluesWriter::BeginBlock( const char* pszName )
{
	if( ErrorOccurred() )
		return false;

	if( !IsOpen() )
	{
		Error( "CKeyvaluesWriter::BeginBlock: No file open!\n" );

		return false;
	}

	if( !pszName )
		pszName = "";

	if( !( *pszName ) && !m_Settings.fAllowUnnamedBlocks )
	{
		Error( "CKeyvaluesWriter::BeginBlock: No unnamed blocks allowed!\n" );

		return false;
	}

	if( !WriteTabs() )
		return false;

	if( !WriteToken( pszName ) )
		return false;

	fprintf( m_pFile, "\n" );

	if( !WriteTabs() )
		return false;

	fprintf( m_pFile, "{\n" );

	++m_uiTabDepth;

	return true;
}

bool CKeyvaluesWriter::EndBlock()
{
	if( ErrorOccurred() )
		return false;

	if( !IsOpen() )
	{
		Error( "CKeyvaluesWriter::EndBlock: No file open!\n" );

		return false;
	}

	if( !m_uiTabDepth )
	{
		Error( "CKeyvaluesWriter::EndBlock: No block open!\n" );

		return false;
	}

	--m_uiTabDepth;

	if( !WriteTabs() )
		return false;

	fprintf( m_pFile, "}\n" );

	return true;
}

bool CKeyvaluesWriter::WriteKeyvalue( const char* const pszKey, const char* const pszValue )
{
	assert( pszKey );
	assert( pszValue );

	if( ErrorOccurred() )
		return false;

	if( !IsOpen() )
	{
		Error( "CKeyvaluesWriter::WriteKeyValue: No file open!\n" );

		return false;
	}

	if( !( *pszKey ) )
	{
		Error( "CKeyvaluesWriter::WriteKeyValue: empty key!\n" );

		return false;
	}

	if( !WriteTabs() )
		return false;

	if( !WriteToken( pszKey ) )
		return false;

	fprintf( m_pFile, " " );

	if( !WriteToken( pszValue ) )
		return false;

	fprintf( m_pFile, "\n" );

	return true;
}

bool CKeyvaluesWriter::WriteBlock( const CKeyvalueBlock& block )
{
	if( !BeginBlock( block.GetKey().CStr() ) )
		return false;

	const CKeyvalueBlock::Children_t& children = block.GetChildren();

	for( const auto pChild : children )
	{
		Write( *pChild );
	}

	return EndBlock();
}

bool CKeyvaluesWriter::WriteKeyvalue( const CKeyvalue& keyvalue )
{
	return WriteKeyvalue( keyvalue.GetKey().CStr(), keyvalue.GetValue().CStr() );
}

bool CKeyvaluesWriter::Write( const CKeyvalueNode& node )
{
	switch( node.GetType() )
	{
	case NodeType::BLOCK:		return WriteBlock( static_cast<const CKeyvalueBlock&>( node ) );
	case NodeType::KEYVALUE:	return WriteKeyvalue( static_cast<const CKeyvalue&>( node ) );
	default:
		{
			Error( "CKeyvaluesWriter::Write: Unknown node type!\n" );

			return false;
		}
	}
}

bool CKeyvaluesWriter::WriteComment( const char* const pszComment, const size_t uiTabs )
{
	assert( pszComment );

	//Allow empty comment blocks

	//Indent to current tab in all cases.
	WriteTabs();

	WriteTabs( uiTabs );

	fprintf( m_pFile, "//%s\n", pszComment );

	return true;
}

bool CKeyvaluesWriter::WriteTabs( const size_t uiTabs )
{
	//Don't bother.
	if( uiTabs == 0 )
		return true;

	if( uiTabs >= MAX_BUFFER_LENGTH )
	{
		Error( "CKeyvaluesWriter::PrintTabs: Tabs too large to output!\n" );

		return false;
	}

	char szBuffer[ MAX_BUFFER_LENGTH ];

	for( size_t uiTab = 0; uiTab < uiTabs; ++uiTab )
	{
		szBuffer[ uiTab ] = '\t';
	}

	szBuffer[ uiTabs ] = '\0';

	fprintf( m_pFile, "%s", szBuffer );

	return true;
}

bool CKeyvaluesWriter::WriteTabs()
{
	return WriteTabs( m_uiTabDepth );
}

bool CKeyvaluesWriter::WriteToken( const char* const pszToken )
{
	assert( pszToken );

	char szBuffer[ MAX_BUFFER_LENGTH ];

	const bool busesQuotes = !( *pszToken ) || strchr( pszToken, ' ' );

	size_t uiBufIndex = 0;

	if( busesQuotes )
		szBuffer[ uiBufIndex++ ] = '\"';

	const size_t uiLength = strlen( pszToken );

	for( size_t uiIndex = 0; uiIndex < uiLength; ++uiIndex )
	{
		const char* pszConv = m_pEscapeSeqConversion->GetString( pszToken[ uiIndex ] );

		if( pszConv )
		{
			const size_t uiConvLength = m_pEscapeSeqConversion->GetStringLength( pszToken[ uiIndex ] );

			if( ( uiBufIndex + uiConvLength ) >= sizeof( szBuffer ) )
			{
				Error( "CKeyvaluesWriter::WriteToken: Token too large!\n" );

				return false;
			}

			strncpy( szBuffer + uiBufIndex, pszConv, uiConvLength );

			uiBufIndex += uiConvLength;
		}
		else
			szBuffer[ uiBufIndex++ ] = pszToken[ uiIndex ];

		if( uiBufIndex >= sizeof( szBuffer ) )
		{
			Error( "CKeyvaluesWriter::WriteToken: Token too large!\n" );

			return false;
		}
	}

	if( busesQuotes )
		szBuffer[ uiBufIndex++ ] = '\"';

	if( uiBufIndex >= sizeof( szBuffer ) )
	{
		Error( "CKeyvaluesWriter::WriteToken: Token too large!\n" );

		return false;
	}

	szBuffer[ uiBufIndex ] = '\0';

	fprintf( m_pFile, "%s", szBuffer );

	return true;
}

void CKeyvaluesWriter::Error( const char* pszError )
{
	if( m_Settings.fLogErrors )
		::Error( "%s", pszError );

	m_bErrorOccurred = true;
}
}