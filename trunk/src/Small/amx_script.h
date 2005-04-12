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

#ifndef _AMXPLUGIN_H
#define _AMXPLUGIN_H

#include "config.h"
#include <string>
#include <vector>
#include <stdarg.h>
#include "script.h"
#include "amx.h"
#include "cfg.h"
#include "basebot.h"

/*
#if defined(__FreeBSD__)
	#include <libgen.h>	// basename
#elif !defined(WIN32)
	extern "C" char *basename(const char*);
#endif
*/
#if defined(WIN32)
#	define GetSymbolAddress	GetProcAddress
#else
#	define GetSymbolAddress dlsym
#endif

using namespace std;

class DynamicExtension;
class DynamicExtensionsPool;



class SmallScript : public Script
{
private:
	AMX amx;
	void *program;
	vector<DynamicExtension*> extensions;
	void showAMXInfos();
	void loadExtensions();
	int callFunction(const char *func, int numparam= 0, cell *plist= NULL, cell *r= NULL);

protected:
	int callFunctionEx(const char *event, const char *format, va_list va);

public:
	SmallScript(const string&);
	~SmallScript();
	virtual AMX *getAMX(){ return &amx; }

	void setAMXPubVar(const char *name, cell value);
	static Script *newScript(const string &path);
	void deleteScript(Script*);
};



// .dll and .so
class DynamicExtension
{
	friend class DynamicExtensionsPool;

private:
	char path[100];
	char root_name[100];
#if defined WIN32
	HINSTANCE hlib;
#else
	void *hlib;
#endif
public:
	DynamicExtension(const char *);
	~DynamicExtension();
	bool call(Script *p, const char *name);
	bool isLoaded(){ return (hlib!=NULL); }
};

/*
This class load and hold links to all the loaded extensions
*/
class DynamicExtensionsPool
{
private:
	vector<DynamicExtension*> extensions_list;

public:
	~DynamicExtensionsPool();
	DynamicExtension* load(const char *path);
};

#endif // _AMXPLUGIN_H

/**/


