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

#ifndef _SCRIPT_H
#define _SCRIPT_H

#ifndef __RUBY_SCRIPT_H
#include "config.h"
#endif

#include <vector>
#include <string>
#include <stdarg.h>
#include "cfg.h"
#include "basebot.h"
#include "thread.h"


using namespace std;


#define MAX_PLUGINS_COUNT 20


struct script_command {
	string name;
	string amx_func;
	string usage;
	uint required_flag;
};

class Script
{
	friend class ScriptManager;

private:
	Mutex mutex_funcall;

protected:
	enum{
		PLUGIN_RUNNING= 0,
		PLUGIN_DISABLED
	} state;

	enum{
		TYPE_INVALID= 0,
		TYPE_AMX,
		TYPE_RUBY
	} plugin_type;

	bool loading_ok;
	char path[100];
	vector<script_command> commands;
	virtual int callFunctionEx(const char *event, const char *format, va_list va)= 0;

public:
	DataFile savedData;
	Script();
	virtual ~Script();
	virtual const char *getName(){ return basename(path); }
	virtual const char *getPath(){ return path; }
	bool LoadingError(){ return !loading_ok; }
	bool disabled(){ return (state==PLUGIN_DISABLED); }

	virtual int callFunctionEx(const char *event, const char *format= "", ...);

	void registerCommand(const string&, const string&, const char, const string&);
	void disablePlugin();
	void enablePlugin();
	bool isDisabled(){ return (state==PLUGIN_DISABLED); }
	bool isEnabled(){  return (state==PLUGIN_RUNNING); }
};


class ScriptManager
{
private:
	Script *plugins[MAX_PLUGINS_COUNT];
	uint used_slots;

	bool isIDXValid(uint index);

public:
	ScriptManager();
	~ScriptManager();

	uint getPluginCount();
	Script *getPlugin(uint idx);

	void ping();

	int loadScriptsFromCfg(ConfigFileReader &cfg);
	void removePlugin(uint index);
	void unloadAllPlugins();
	int addPlugin(const string&);
	Script *findPlugin(const string &path);
	void callEvent(const char *event, const char *format= "", ...);
	int callCommand(const user_Info*, const dummy_Info*, const vector<string> &args);
	void saveData();
	void sendCommandsList(const string &nick, const string &channel, const user_Info *usr);
};


#endif // _SCRIPT_H

/**/


