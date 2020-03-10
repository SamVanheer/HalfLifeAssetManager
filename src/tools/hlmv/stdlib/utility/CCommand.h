#ifndef UTILITY_CCOMMAND_H
#define UTILITY_CCOMMAND_H

namespace util
{
/**
*	Contains command arguments.
*/
class CCommand final
{
public:
	/**
	*	Maximum number of arguments that can be stored by this command.
	*/
	static const size_t MAX_TOKENS = 80;

	/**
	*	Maximum length that the entire command string can be, including null terminator.
	*/
	static const size_t MAX_LENGTH = 512;

	/**
	*	Maximum number of characters that a command string can have, excluding null terminator.
	*/
	static const size_t MAX_CMD_LENGTH = MAX_LENGTH - 1;

public:
	/**
	*	Default constructor. Initializes the command to an invalid state.
	*/
	CCommand();

	/**
	*	Initializes the command with the given arguments.
	*	@param iArgc Argument count.
	*	@param ppArgV Argument vector.
	*/
	CCommand( const int iArgc, char** ppArgV );

	/**
	*	Initializes the command with the given command string. The string is broken up into tokens.
	*	@param pszCommand Command string.
	*/
	CCommand( const char* pszCommand );

	/**
	*	Initializes the command with a single command and value.
	*	@param pszCommand command.
	*	@param pszValue Value.
	*/
	CCommand( const char* pszCommand, const char* pszValue );

	/**
	*	Copy constructor.
	*/
	CCommand( const CCommand& other );

	/**
	*	Assignment operator.
	*/
	CCommand& operator=( const CCommand& other );

	/**
	*	Resets the command. This command is invalid after the method returns.
	*/
	void Reset();

	/**
	*	@see CCommand( const int iArgc, char** ppArgV )
	*/
	bool Initialize( const int iArgc, char** ppArgV );

	/**
	*	Initializes the command by breaking up the given command into tokens.
	*	@see Initialize( const int iArgc, void* pData, const GetArg getArg )
	*/
	bool Initialize( const char* pszCommand );
	
	/**
	*	@see CCommand( const CCommand& other )
	*/
	bool Initialize( const CCommand& other );

	/**
	*	Returns whether this command is valid (has any arguments).
	*/
	bool IsValid() const { return m_iArgc > 0; }

	/**
	*	Gets the argument count.
	*/
	int ArgC() const { return m_iArgc; }

	/**
	*	Gets the entire command as a string
	*/
	const char* GetCommandString() const;

	/**
	*	Gets all arguments as a single string
	*/
	const char* GetArgumentsString() const;

	/**
	*	Gets the argument vector
	*/
	const char* const* ArgV() const;

	/**
	*	Gets the argument by index
	*/
	const char* operator[]( const int iIndex ) const;

	/**
	*	@see operator[]( const int iIndex ) const
	*/
	const char* Arg( const int iIndex ) const;

	/**
	*	 Find a value for a given argument
	*	 Returns nullptr if no such argument exists
	*	 Returns an empty string if the argument exists, but no value exists for it
	*	 Otherwise, returns the value
	*/
	const char* FindArg( const char* pszArgument ) const;

	/**
	*	 Find an int value for a given argument
	*	 Returns iDefault if no such argument exists, or no value exists for it
	*	 Otherwise, returns the value, converted to an int
	*/
	int FindIntArg( const char* pszArgument, const int iDefault = 0 ) const;

private:
	/**
	*	@see Initialize( const int iArgc, void* pData, const GetArg getArg )
	*/
	using GetArg = const char* ( * )( void* pData, const int iIndex );

	/**
	*	Initializes the command using the given data.
	*	@param iArgC Number of arguments.
	*	@param pData Pointer to data to use. Can be either an argv (char*[]) or another CCommand.
	*	@param getArg Function used to get data from pData.
	*/
	bool Initialize( const int iArgc, void* pData, const GetArg getArg );

private:
	int m_iArgc;

	/**
	*	Length of the 0th argument, the command name.
	*/
	size_t m_uiCommandNameLength;

	/**
	*	Contains the original command string, with quotes added if a string contains spaces.
	*/
	char m_szCommandString[ MAX_LENGTH ];

	/**
	*	Contains a series of null terminated strings.
	*/
	char m_szArgsBuffer[ MAX_LENGTH ];

	/**
	*	Points into m_szArgsBuffer.
	*/
	char* m_pArgV[ MAX_TOKENS ];
};
}

#endif //UTILITY_CCOMMAND_H