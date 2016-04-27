#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "shared/Logging.h"
#include "shared/Utility.h"

#include "CDocGenerator.h"

//Implement the default log listener.
class CPrintfLogListener final : public ILogListener
{
public:
	void LogMessage( const LogType type, const char* const pszMessage ) override final
	{
		const char* pszPrefix = "";

		switch( type )
		{
		case LogType::WARNING:
			{
				pszPrefix = "Warning: ";
				break;
			}

		case LogType::ERROR:
			{
				pszPrefix = "Error: ";
				break;
			}

		default: break;
		}

		printf( "%s%s", pszPrefix, pszMessage );
	}
};

namespace
{
static CPrintfLogListener g_PrintfLogListener;
}

/**
*	Describes an application argument.
*/
struct AppArg_t
{
	const char* const	pszName;
	const char* const	pszLongName;
	const char* const	pszDescription;
	const char**		ppszDestination;
};

void PrintUsage( const AppArg_t* const pArgs, const size_t uiNumArgs )
{
	assert( pArgs );

	Message( "Usage:\n" );

	for( size_t uiIndex = 0; uiIndex < uiNumArgs; ++uiIndex )
	{
		const auto& arg = pArgs[ uiIndex ];

		Message( "%s: %s\nLong name: %s\n", arg.pszName, arg.pszDescription, arg.pszLongName );
	}

	Message( "\n" );
}

#ifdef _DEBUG
#define WAIT_DEFAULT "1"
#else
#define WAIT_DEFAULT "0"
#endif

int main( int iArgc, char* pszArgv[] )
{
	//Set up the logger.
	SetDefaultLogListener( &g_PrintfLogListener );

	const char* pszInputFilename = nullptr;
	const char* pszOutputDirectory = nullptr;
	const char* pszWait = WAIT_DEFAULT;

	AppArg_t args[] = 
	{
		{ "-if", "--inputfile", "The filename of the file that contains the documentation to convert to HTML", &pszInputFilename },
		{ "-od", "--outputdir", "The path to the directory where the HTML files will be saved to", &pszOutputDirectory },
		{ "-w", "--wait", "If 1, the program will wait for ENTER after finishing", &pszWait }
	};

	/**
	*	Parse in all arguments.
	*/
	for( int iArg = 1; iArg < iArgc; ++iArg )
	{
		for( size_t uiIndex = 0; uiIndex < ARRAYSIZE( args ); ++uiIndex )
		{
			auto& arg = args[ uiIndex ];

			if( strcmp( arg.pszName, pszArgv[ iArg ] ) == 0 ||
				strcmp( arg.pszLongName, pszArgv[ iArg ] ) == 0 )
			{
				if( iArg + 1 < iArgc )
				{
					*arg.ppszDestination = pszArgv[ ++iArg ];
				}
			}
		}
	}

	int iReturnCode = EXIT_FAILURE;

	if( !pszInputFilename || !pszOutputDirectory )
	{
		PrintUsage( args, ARRAYSIZE( args ) );

		Message( "Missing arguments; aborting\n" );
	}
	else
	{
		CDocGenerator generator;

		if( generator.GenerateFromFile( pszInputFilename, pszOutputDirectory ) )
		{
			iReturnCode = EXIT_SUCCESS;
		}
		else
		{
			PrintUsage( args, ARRAYSIZE( args ) );
		}
	}

	if( strcmp( pszWait, "1" ) == 0 )
	{
		Message( "Press ENTER to continue..." );

		getchar();
	}

	return iReturnCode;
}