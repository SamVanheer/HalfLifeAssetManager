#ifndef COMMON_LOGGING_H
#define COMMON_LOGGING_H

/**
*	The implementations for logging features are UI dependent.
*/

/**
*	Logs a message.
*/
void Message( const char* const pszFormat, ... );

/**
*	Logs a warning.
*/
void Warning( const char* const pszFormat, ... );

/**
*	Logs an error.
*/
void Error( const char* const pszFormat, ... );

#endif //COMMON_LOGGING_H