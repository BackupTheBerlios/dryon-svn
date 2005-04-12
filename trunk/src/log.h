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

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <string>
#if defined(WIN32)
#	include "termwin.h"
#endif

using namespace std;

class Logs
{
private:
	FILE *f;
	char filename[200];
	char id[6];
	char *color;
	bool disp;
	bool enabled;

public:
	Logs(char *stream_name, char *c, char *fname= "logs.txt");
	virtual ~Logs();
	virtual void Write(char *format, ...);
	static void DummyWrite(char *, ...);
	void enable(){ enabled= true; }
	void disable(){ enabled= false; }
};

extern Logs ErrorLog;
extern Logs DebugLog;
extern Logs AmxLog;

#ifndef TEST

#if defined(EXTENSION_MODULE)
#	define Output	Logs::DummyWrite
#elif defined(WIN32)
#	define Output	ConsoleThread::amx_printf
#else
#	define Output	printf
#endif

#if defined(EXTENSION_MODULE)
#	define Debug 	debug_log->Write
#	define Error 	error_log->Write
#	define AmxDebug amx_log->Write
#elif defined(SHOW_DEBUG_MSG)
#	define Debug 	DebugLog.Write
#	define Error 	ErrorLog.Write
#	define AmxDebug AmxLog.Write
#else
#	define Debug Logs::DummyWrite
#	define Error Logs::DummyWrite
#endif

#else

#include <stdio.h>
#define Debug printf
#define Error printf

#endif

#endif // LOG_H
