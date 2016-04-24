#ifndef CKEYVALUESWRITER_H
#define CKEYVALUESWRITER_H

#include "common/Platform.h"

#include "KeyvaluesConstants.h"

class CKvBlockNode;
class CKeyvalue;
class CKeyvalueNode;
class CEscapeSequences;

/**
*	Writer that can write keyvalues files.
*/
class CKeyvaluesWriter final
{
public:
	/**
	*	Constructs a writer that will write to the given file
	*	No escape sequence conversion will take place
	*	@param pszFilename Name of the file to write to.
	*	@param settings Writer settings.
	*/
	CKeyvaluesWriter( const char* const pszFilename, const CKeyvaluesLexerSettings& settings = CKeyvaluesLexerSettings() );

	/**
	*	Constructs a writer that will write to the given file
	*	Uses the provided escape sequences conversion
	*	@param pszFilename Name of the file to write to.
	*	@param escapeSeqConversion Escape sequences conversion rules.
	*	@param settings Writer settings.
	*/
	CKeyvaluesWriter( const char* const pszFilename, CEscapeSequences& escapeSeqConversion, const CKeyvaluesLexerSettings& settings = CKeyvaluesLexerSettings() );
	~CKeyvaluesWriter();

	/**
	*	Returns whether the writer has a file open or not.
	*/
	bool IsOpen() const { return m_pFile != nullptr; }

	/**
	*	Gets the name of the file that is being written to, or an empty string if no file is open.
	*/
	const char* GetFilename() const { return m_szFilename; }

	/**
	*	Returns whether an error has occurred during writing.
	*/
	bool ErrorOccurred() const { return m_bErrorOccurred; }

	/**
	*	Opens a file for writing. If a file is currently open, it is closed first.
	*/
	bool Open( const char* const pszFileName );

	/**
	*	If a file is currently opened for writing, closes the file.
	*/
	void Close();

	/**
	*	Begins a new block.
	*	@param pszName Name of the block. Can be null if the settings allow for unnamed blocks.
	*	@return true on success, false otherwise.
	*/
	bool BeginBlock( const char* pszName = nullptr );

	/**
	*	Ends a block.
	*	@return true on success, false otherwise.
	*/
	bool EndBlock();

	/**
	*	Writes a keyvalue.
	*	@param pszKey Key. Must be non-null and not empty.
	*	@param pszValue Value. Must be non-null, may be empty.
	*	@return true on success, false otherwise.
	*/
	bool WriteKeyvalue( const char* const pszKey, const char* const pszValue );

	/**
	*	Writes an entire block in the current scope.
	*	@return true on success, false otherwise.
	*/
	bool WriteBlock( const CKvBlockNode& block );

	/**
	*	Writes a keyvalue.
	*	@return true on success, false otherwise.
	*/
	bool WriteKeyvalue( const CKeyvalue& keyvalue );

	/**
	*	Writes a node. Determines the underlying type and acts accordingly.
	*	@return true on success, false otherwise.
	*/
	bool Write( const CKeyvalueNode& node );

	/**
	*	Writes a single line comment.
	*	@param pszComment Comment to add. Must be non-null, may not contain newlines. May be empty.
	*	@param uiTabs Optional. Additional tab indentation.
	*	@return true on success, false otherwise.
	*/
	bool WriteComment( const char* const pszComment, const size_t uiTabs = 0 );

private:
	/**
	*	Writes a number of tabs.
	*	@param uiTabs The number of tabs to write.
	*	@return true on success, false otherwise.
	*/
	bool WriteTabs( const size_t uiTabs );

	/**
	*	Writes enough tabs to indent to the current block scope.
	*	@return true on success, false otherwise.
	*/
	bool WriteTabs();

	/**
	*	Writes a single token.
	*	@param pszToken Token. Must be non-null.
	*	@return true on success, false otherwise.
	*/
	bool WriteToken( const char* const pszToken );

	/**
	*	Reports an error.
	*	@param pszError Error to report.
	*/
	void Error( const char* const pszError );

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