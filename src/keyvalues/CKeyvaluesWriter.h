#ifndef CKEYVALUESWRITER_H
#define CKEYVALUESWRITER_H

#include "common/Platform.h"

#include "KeyvaluesConstants.h"

class CKvBlockNode;
class CKeyvalue;
class CKeyvalueNode;
class CEscapeSequences;

class CKeyvaluesWriter
{
public:
	/*
	* Constructs a writer that will write to the given file
	* No escape sequence conversion will take place
	*/
	CKeyvaluesWriter( const char* pszFilename, const CKeyvaluesLexerSettings& settings = CKeyvaluesLexerSettings() );

	/*
	* Constructs a writer that will write to the given file
	* Uses the provided escape sequences conversion
	*/
	CKeyvaluesWriter( const char* pszFilename, CEscapeSequences& escapeSeqConversion, const CKeyvaluesLexerSettings& settings = CKeyvaluesLexerSettings() );
	~CKeyvaluesWriter();

	bool IsOpen() const { return m_pFile != nullptr; }

	const char* GetFilename() const { return m_szFilename; }

	bool ErrorOccurred() const { return m_bErrorOccurred; }

	bool Open( const char* pszFileName );
	void Close();

	bool BeginBlock( const char* pszName = nullptr );
	bool EndBlock();

	bool WriteKeyvalue( const char* pszKey, const char* pszValue );

	bool WriteBlock( const CKvBlockNode& block );

	bool WriteKeyvalue( const CKeyvalue& keyvalue );

	bool Write( const CKeyvalueNode& node );

private:
	bool WriteTabs();

	bool WriteToken( const char* const pszToken );

	void Error( const char* pszError );

private:
	CKeyvaluesLexerSettings m_Settings;

	CEscapeSequences* m_pEscapeSeqConversion;

	FILE* m_pFile = nullptr;

	char m_szFilename[ MAX_PATH_LENGTH + 1 ];

	size_t m_uiTabDepth = 0;

	bool m_bErrorOccurred = false;

private:
	CKeyvaluesWriter( const CKeyvaluesWriter& ) = delete;
	CKeyvaluesWriter& operator=( const CKeyvaluesWriter& ) = delete;
};

#endif //CKEYVALUESWRITER_H