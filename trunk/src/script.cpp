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

	Main plugin class, implements nothing by itself, just an interface

*/

#include "script.h"
#include "userfile.h"
#include "utils.h"
#include "dryon.h"
#include "thread.h"

#include "Small/amx_script.h"
#include "Ruby/ruby_script.h"

extern DryonBot bot;

Script::Script() : state(PLUGIN_RUNNING), loading_ok(true)
{

}

Script::~Script()
{
	savedData.saveFile();
	commands.clear();
}

void Script::disablePlugin()
{
	state= PLUGIN_DISABLED;
}

void Script::enablePlugin()
{
	state= PLUGIN_RUNNING;
}

// call the given function with given parameters
int Script::callFunctionEx(const char *event, const char *format, ...)
{
	if( isEnabled() )
	{
		int ret;
		va_list va;

		va_start(va, format);
		MUTEX_GET(mutex_funcall);
		{
			ret= callFunctionEx(event, format, va);
		}
		MUTEX_RELEASE(mutex_funcall);
		va_end(va);

		return ret;
	}
	else
	{
		Debug("[callFunctionEx] (%s :: %s) not called (plugin disabled)\n", getName(), event);
		return 0;
	}
}

void Script::registerCommand(const string &cmd, const string &func_to_call, const char flag, const string &usage)
{
	for(vector<script_command>::iterator it= commands.begin(); it!=commands.end(); it++ )
	{
		// if command already registered by this plugin, unregister it
		if( cmd==it->name )
		{
			commands.erase(it);
			break;
		}
	}

	// if we came here, then we can safely add the command
	script_command c;
	c.name= cmd;
	c.amx_func= func_to_call;
	c.usage= usage;
	c.required_flag= 0;

	if( !UserFile::setFlagMaskFromChar(flag, c.required_flag) )
		Error("addCommand: Invalid flag '%c' !\n", flag);

	commands.push_back(c);
	Debug("[%s] Command registered: %s\n", basename(path), cmd.c_str());
}




/******************/
/* Script Manager */

ScriptManager::ScriptManager() : used_slots(0)
{
	RubyScript::initEngine();
}

ScriptManager::~ScriptManager()
{
	//Debug("ScriptManager::~ScriptManager()\n");
	unloadAllPlugins();
	RubyScript::freeEngine();
}

bool ScriptManager::isIDXValid(uint index)
{
	return ((used_slots&(1<<index)) != 0);
}

uint ScriptManager::getPluginCount()
{
	uint ret= 0;
	for(uint i= 0; i< MAX_PLUGINS_COUNT; i++ )
	{
		if( isIDXValid(i) )
			ret++;
	}

	return ret;
}

Script *ScriptManager::getPlugin(uint idx)
{
	uint i;

	for(i= 0; (idx > 0) && (i< MAX_PLUGINS_COUNT); i++ )
	{
		if( isIDXValid(i) )
			idx--;
	}

	return plugins[i];
}

Script *ScriptManager::findPlugin(const string &path)
{
	Script *result= NULL;

	for( uint i= 0; (i< MAX_PLUGINS_COUNT) && !result; i++ )
	{
		if( isIDXValid(i) && (path==plugins[i]->path) )
			result= plugins[i];
	}

	return result;
}

void ScriptManager::unloadAllPlugins()
{
	for( uint i= 0; i< MAX_PLUGINS_COUNT; i++ )
		removePlugin(i);
}

int ScriptManager::loadScriptsFromCfg(ConfigFileReader &cfg)
{
	int count;
	int ret= 0;
	char name[100];
	string path;

	unloadAllPlugins();

	for(count= 1; !ret; count++)
	{
		snprintf(name, sizeof(name)-1, "script_%02d", count);

		if( !cfg.isDefined(name) )
			break;

		path= cfg.readStringKey(name);

		if( !fileExist(path.c_str()) )
		{
			Error("file specified in config file does not exists: %s\n", path.c_str());
			return -1;
		}

		ret= addPlugin(path);		// and load the script
		if( ret == -2 )
		{
			//Error("Not enough memory !!\n");
			Error("- Could not load %s !\n", path.c_str());
			return -1;
		}
	}

	if( count == 1 )	// no scripts
	{
		Error("No scripts in config !\n");
		return -1;
	}

	return ret;
}

// name= xxx.sma
int ScriptManager::addPlugin(const string &name)
{
	int ret;
	uint idx;

	// extract extension
	string ext= name;
	uint n= ext.rfind(".");
	if( n != string::npos )
	{
		ext= ext.substr(n+1, ext.size()-n-1);

		// find a free slot
		for(idx= 0; idx< MAX_PLUGINS_COUNT; idx++)
		{
			if( (used_slots & (1<<idx)) == 0 )
				break;
		}

		// too many plugins loaded
		if( idx >= MAX_PLUGINS_COUNT )
		{
			ret= -1;
		}
		else
		{
			// Small script
			if( ext == "sma" )
			{
				plugins[idx]= SmallScript::newScript(name);
			}
			// Ruby script
			else if( ext == "rb" )
			{
				plugins[idx]= RubyScript::newScript(name);
			}

			Script *sc= plugins[idx];
			if( sc!=NULL )
			{
				//string save_path(sc->path);
				//save_path+= ".cfg";
				
				string save_path= "script_configs/";
				save_path+= sc->getName();
				save_path+= ".cfg";

				sc->savedData.setFile(save_path.c_str());
				sc->savedData.readFile();

				// memory error
				if( sc == NULL )
				{
					ret= -2;
				}
				// loading error
				else if( sc->LoadingError() )
				{
					if( ext == "sma" )
						delete plugins[idx];
					ret= -1;
				}
				// loading is a success
				else
				{
					used_slots|= (1<<idx);
					ret= 0;

					sc->callFunctionEx("event_LoadPlugin");
					if( bot.alreadyConnected() )
						sc->callFunctionEx("event_onConnected");
				}
			}
			else
			{
				ret= -1;
			}
		}
	}
	else
	{
		ret= -1;
	}

	return ret;
}

void ScriptManager::removePlugin(uint index)
{
	if( isIDXValid(index) )
	{
		// tell the script that it will get unloaded
		plugins[index]->callFunctionEx("event_UnloadPlugin");
		if( plugins[index]->plugin_type == Script::TYPE_AMX )
		{
			delete plugins[index];
		}
		else
		{
			RubyScript::deleteObj((RubyScript*)plugins[index]);
		}
		used_slots&= ~(1<<index);
	}
}

/*
format:
i= integer
s= string (packed)
a= array of strings
w= array to expand ([1,2,3]) => (1,2,3)

- object:
u= user object
c= channel object

ex: "isii"
*/
void ScriptManager::callEvent(const char *event, const char *format, ...)
{
	va_list va;

	for(uint i= 0; i< MAX_PLUGINS_COUNT; i++ )
	{
		if( isIDXValid(i) )
		{
			va_start(va, format);
			plugins[i]->callFunctionEx(event, format, va);
			va_end(va);
		}
	}
}

void ScriptManager::sendCommandsList(const string &nick, const string &channel, const user_Info *usr)
{
	for(uint i= 0; i< MAX_PLUGINS_COUNT; i++)
	{
		if( isIDXValid(i) )
		{
			vector<script_command> &v= plugins[i]->commands;

			for(vector<script_command>::iterator it= v.begin(); it!=v.end(); it++)
			{
				// user has required flag
				if( !it->usage.empty() && ((it->required_flag==USRLVL_PUBLIC) || usr->hasFlag(it->required_flag)) )
					bot.notice(nick, "%s", it->usage.c_str());
			}
		}
	}
}

void ScriptManager::ping()
{
	RubyScript::ping();
}

int ScriptManager::callCommand(const user_Info *sender, const dummy_Info *dest, const vector<string> &args)
{
	int ret;

	if( args.empty() )
	{
		Error("callCommand called with an empty string\n");
		return 0;
	}

	for(uint i= 0; (i< MAX_PLUGINS_COUNT) && isIDXValid(i); i++)
	{
		if( isIDXValid(i) )
		{
			vector<script_command> &v= plugins[i]->commands;

			for(vector<script_command>::iterator it= v.begin(); it!=v.end(); it++)
			{
				// corresponding function found and user has required flag
				if( (args[0]==it->name) && ( (it->required_flag==USRLVL_PUBLIC) || sender->hasFlag(it->required_flag)) )
				{
					vector<string> params= args;
					// erase function name from vector (first)
					params.erase( params.begin() );

					Debug(" [%s] => \"%s\" : [%s]\n", basename(plugins[i]->path), sender->nick.c_str(),  it->name.c_str());
					if( dest->type == CHAN_INFO )
					{
						chan_Info *chan= (chan_Info*)dest;
						ret= plugins[i]->callFunctionEx(it->amx_func.c_str(), "ucv", sender, chan, &params);
					}
					else
					{
						user_Info *usr= (user_Info*)dest;
						ret= plugins[i]->callFunctionEx(it->amx_func.c_str(), "uuv", sender, usr, &params);
					}

					if( (ret != 0) && !it->usage.empty() )
					{
						bot.notice(sender->nick, "%s", it->usage.c_str());
					}
				}

				//if( !isIDXValid(i) )
				//	break;
			}
		}
	}

	return 0;
}

void ScriptManager::saveData()
{
	for(uint i= 0; i< MAX_PLUGINS_COUNT; i++)
	{
		if( isIDXValid(i) )
			plugins[i]->savedData.saveFile();
	}
}

/**/

