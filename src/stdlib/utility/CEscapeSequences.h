#ifndef CESCAPESEQUENCES_H
#define CESCAPESEQUENCES_H

/**
* This class represents a mapping of escape sequences to their string versions
* E.g. \n becomes \\n
*/
class CEscapeSequences
{
public:
	struct ConversionData_t
	{
		char cEscapeSequence;		//Escape sequence, e.g. \n
		const char* pszString;		//String version, e.g. "\n"
	};

	static const char INVALID_CHAR = 0x7F;

private:
	struct ConversionInfo_t
	{
		const char* pszString;
		size_t uiLength;
	};

public:
	CEscapeSequences( const char cDelimiterChar, const size_t uiCount, const ConversionData_t* const pData );

	char GetDelimiterChar() const { return m_cDelimiterChar; }

	const char* GetString( const char cEscapeSequence ) const;
	size_t GetStringLength( const char cEscapeSequence ) const;

	char GetEscapeSequence( const char* const pszString ) const;

private:
	const char m_cDelimiterChar;		//Which character indicates a conversion is required when converting from string to sequence
	const size_t m_uiCount;				//Number of mappings
	ConversionInfo_t m_Infos[ 256 ];	//Escape sequence is an index into this array
	char m_IndexToSeq[ 256 ];			//Maps an index to an escape sequence

private:
	CEscapeSequences( const CEscapeSequences& ) = delete;
	CEscapeSequences& operator=( const CEscapeSequences& ) = delete;
};

#define BEGIN_ESCAPE_SEQ_LIST( listName )							\
static CEscapeSequences::ConversionData_t g_##listName##List[] =	\
{

#define END_ESCAPE_SEQ_LIST( listName, cDelimiterChar )													\
};																										\
static CEscapeSequences listName( cDelimiterChar, ARRAYSIZE( g_##listName##List ), g_##listName##List )

CEscapeSequences& GetEscapeSeqConversion();
CEscapeSequences& GetNoEscapeSeqConversion();

#endif //CESCAPESEQUENCES_H