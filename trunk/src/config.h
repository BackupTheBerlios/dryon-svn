/*
********************************************************************************
*                                 Dryon project
********************************************************************************
*
*  This is free software under the GPL license.
*  Since this software uses Small from Compuphase, it also use its license.
*  Copy of the two licenses are in the main dir (gpl.txt and amx_license.txt
*  Coded by Ammous Julien known as Anthalir
*
********************************************************************************
*/

#ifndef __CONFIG_H
#define __CONFIG_H

#include <assert.h>

#if !defined(WIN32)
	#include <libgen.h>	// basename
//	extern "C" char *basename(const char*);
#endif


#if defined(WIN32)
#include <windows.h>
#include <shlwapi.h>

#if defined(_MSC_VER)

#	if defined(_DEBUG)
#		define _CRTDBG_MAP_ALLOC
#		include <stdlib.h>
#		include <crtdbg.h>
#	endif

#	pragma warning(disable: 4786)	//Gets rid of BAD stl warnings
#	pragma warning(disable: 4503)
#	pragma warning(disable: 4800)	//needed couse now we can see the real warning

#undef snprintf
#	define snprintf _snprintf
#undef vsnprintf
#	define vsnprintf _vsnprintf
#undef close
#	define close closesocket
#undef strcasecmp
#	define strcasecmp stricmp
#undef strncasecmp
#	define strncasecmp strnicmp
#endif // _MSC_VER

typedef unsigned char u_char;
typedef int socklen_t;

#define basename 		PathFindFileName
#define usleep(X) 		Sleep(X/1000)
#undef sleep
#define sleep(X) 		Sleep(X*1000)
#define sched_yield() 	Sleep(1)
#define strstr			StrStr
#define strcasestr		StrStrI
#define unlink 			_unlink

#define COLOR_RED		"\033r"
#define COLOR_GREEN		"\033g"
#define COLOR_CYAN		"\033c"
#define COLOR_YELLOW	"\033y"
#define COLOR_BLUE		"\033b"
#define COLOR_MAGENTA	"\033m"

#define COLOR_RESET		"\033z"

#else // WIN32

#define COLOR_RED		"\033[31m"
#define COLOR_GREEN		"\033[32m"
#define COLOR_CYAN		"\033[36m"
#define COLOR_YELLOW	"\033[33m"
#define COLOR_BLUE		"\033[34m"
#define COLOR_MAGENTA	"\033[35m"

#define COLOR_RESET		"\033[0m"

#endif /* WIN32 */


/* config directives */
#define SHOW_DEBUG_MSG

/* debug */
//#define DEBUG_EXCEPTIONS

#define BOT_VERSION "0.9"
#define SMALL_VERSION "?"
#define RUBY_VERSION "1.8.2"

typedef unsigned int uint;

#endif // __CONFIG_H

/**/

