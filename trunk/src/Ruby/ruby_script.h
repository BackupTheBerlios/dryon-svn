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

#ifndef __RUBY_SCRIPT_H
#define __RUBY_SCRIPT_H

//#include "config.h"

#if defined(WIN32) && defined (_MSC_VER)
#	pragma warning(disable: 4786)
#	pragma warning(disable: 4503)
#	pragma warning(disable: 4800)
#endif


#include "script.h"
#include "ruby.h"

class RubyScript : public Script
{
private:
	VALUE self;

protected:
	int callFunctionEx(const char *event, const char *format, va_list va);

public:
	RubyScript();
	~RubyScript();

	static Script *newScript(const string &);
	static void initEngine();
	static void freeEngine();
	static void deleteObj(RubyScript *);

	static void stdout_write(char *fmt, ...);
	static void ping();
};


#endif // __RUBY_SCRIPT_H


/**/
