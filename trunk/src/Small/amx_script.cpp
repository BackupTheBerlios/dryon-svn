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

#include <string.h>

#include "config.h"
#if !defined(WIN32)
#	include <unistd.h>
#	include <dlfcn.h>
// for MinGW
#elif !defined(_MSC_VER)
#	include <io.h>
#endif

#include <stdarg.h>
#include <time.h>
#include <assert.h>
#include <vector>
#include "amx_script.h"
#include "amxbot.h"
#include "amx.h"
#include "natives.h"
#include "utils.h"
#include "userfile.h"

//extern AMXBot bot;
extern DynamicExtensionsPool extPool;

/* depends on AMX implementation */

typedef struct tagFUNCSTUBNT {
  uint32_t address PACKED;
  uint32_t nameofs PACKED;
} FUNCSTUBNT       PACKED;

#define USENAMETABLE(hdr) \
                        ((hdr)->defsize==sizeof(FUNCSTUBNT))
#define NUMENTRIES(hdr,field,nextfield) \
                        (int)(((hdr)->nextfield - (hdr)->field) / (hdr)->defsize)
#define GETENTRY(hdr,table,index) \
                        (AMX_FUNCSTUB *)((unsigned char*)(hdr) + (int)(hdr)->table + (int)index*(hdr)->defsize)
#define GETENTRYNAME(hdr,entry) \
                        ( USENAMETABLE(hdr) \
                           ? (char *)((unsigned char*)(hdr) + ((FUNCSTUBNT*)(entry))->nameofs) \
                           : ((AMX_FUNCSTUB*)(entry))->name )

/*********************************/
// /path/to/amxPower.so
DynamicExtension::DynamicExtension(const char *file_path)
{
	strncpy(path, file_path, sizeof(path)-1);

	const char *tmp= basename(path);
	if( strchr(tmp, '.') != NULL )
	{
		int len= strchr(tmp, '.') - &tmp[3];
		strncpy(root_name, &tmp[3], len);
		root_name[len]= '\0';

#if defined WIN32
		hlib= LoadLibrary(file_path);
#else
		hlib= dlopen(file_path, RTLD_NOW);
#endif

	}
	else
	{
		Error("Wrong file_path: '%s'\n", file_path);
	}

	// give a pointer to the Log classes for the extension
	if( hlib != NULL )
	{
		Logs **logs[3]/*, **output*/;

		logs[0]= (Logs**)GetSymbolAddress(hlib, "error_log");
		logs[1]= (Logs**)GetSymbolAddress(hlib, "debug_log");
		logs[2]= (Logs**)GetSymbolAddress(hlib, "amx_log");

		//output= (void(*)(char *format, ...))GetSymbolAddress(hlib, "output_func");

		if( logs[0] != NULL )
			*logs[0]= &ErrorLog;
		else
			Error("DLL <%s>: failed to find exported variable error_log\n", file_path);

		if( logs[1] != NULL )
			*logs[1]= &DebugLog;
		else
			Error("DLL <%s>: failed to find exported variable debug_log\n", file_path);

		if( logs[2] != NULL )
			*logs[2]= &AmxLog;
		else
			Error("DLL <%s>: failed to find exported variable amx_log\n", file_path);
	}
}

DynamicExtension::~DynamicExtension()
{
	if( hlib!=NULL )
	{
		call(NULL, "Cleanup");
#if defined WIN32
		FreeLibrary(hlib);
#else
		dlclose(hlib);
#endif
	}
}

// return true if function successfully called
// call for Init will result in amxbot_<ModuleName>Init being called
bool DynamicExtension::call(Script *p, const char *name)
{
	bool ret;
	typedef int (*FUNC)(Script *p);
	FUNC lib_func;

	if( hlib==NULL )
	{
		ret= false;
	}
	else
	{
		char func_name[100];
		snprintf(func_name, sizeof(func_name)-1, "amxbot_%s_%s", root_name, name);

#if defined WIN32
		lib_func= (FUNC)GetProcAddress(hlib, func_name);
#elif defined LINUX
		lib_func= (FUNC)dlsym(hlib, func_name);
#endif

		if( lib_func!=NULL )
		{
			lib_func(p);
			ret= true;
		}
		else
		{
			Error("DLL Call Error: '%s', function '%s' not found in it\n", basename(path), func_name);
			ret= false;
		}
	}

	return ret;
}

DynamicExtensionsPool::~DynamicExtensionsPool()
{
	for(uint i= 0; i< extensions_list.size(); i++)
	{
		Debug("Extension unloaded: '%s'\n", extensions_list[i]->path);
		delete extensions_list[i];
	}

	extensions_list.clear();
}

DynamicExtension *DynamicExtensionsPool::load(const char *path)
{
	// look in the pool to see if the extension
	// is not already loaded
	for(uint i= 0; i< extensions_list.size(); i++ )
	{
		// extension found
		if( !strcasecmp(path, extensions_list[i]->path) )
			return extensions_list[i];
	}

	//else try to load it
	DynamicExtension *ex= new DynamicExtension(path);
	if( !ex->isLoaded() )
	{
		Error("Unable to find/load dynamic extension: '%s'\n", path);
#if !defined(WIN32)
		Error("dlopen: %s\n", dlerror());
#endif
		delete ex;
		return NULL;
	}
	else
	{
		Output("Extension loaded: '%s'\n", path);
		extensions_list.push_back(ex);
		return ex;
	}
}

/*********************************/

int AMXAPI amx_myCallback(AMX *amx, cell index, cell *result, cell *params)
{
	time_t before= time(NULL);
	int ret= amx_Callback(amx, index, result, params);

	if( time(NULL) - before > 1 )
	{
		int len;
		amx_NameLength(amx, &len);
		Ptr<char> buff(len+1);
		amx_GetNative(amx, index, buff.get());

		Output(COLOR_RED);
		Output("time for %s: %ds\n", buff.get(), (int)(time(NULL)-before));
		Output(COLOR_RESET);
	}

	return ret;
}

Script *SmallScript::newScript(const string &path)
{
	return new SmallScript(path);
}

void SmallScript::deleteScript(Script *s)
{
	delete s;
}

/* Plugin Class */
// name= /path/to/xxx.sma
SmallScript::SmallScript(const string &sma_path): program(NULL)
{
	int err, memsize;

	plugin_type= TYPE_AMX;

	string base_path= sma_path;
	base_path.erase(base_path.rfind("."));

	string amx_path= buildFilename(base_path, "amx");

	int ret= srun_BuildScript(sma_path.c_str());
	if( ret==0 )
	{
		memsize= srun_ProgramSize(amx_path.c_str());
		if( memsize == 0 )
		{
			Error("Unable to load %s\n", amx_path.c_str());
			loading_ok= false;
		}
		else
		{
			program= new char[memsize];
			if( !program )
			{
				Error("Memory allocation error in AMXScript::loadScript\n");
				loading_ok= false;
			}
			else
			{

				strncpy(path, sma_path.c_str(), sizeof(path)-1);

				err= srun_LoadProgram(&amx, amx_path.c_str(), program);
				if( err != AMX_ERR_NONE )
				{
					Error("%s : %s\n", sma_path.c_str(), amx_StrError(err));
					loading_ok= false;
				}
				else
				{
					amx_SetCallback(&amx, amx_myCallback);
					//amx_CoreInit(&amx);
					registerAMXbotNatives(&amx);
					loadExtensions();

					err= amx_Register(&amx, NULL, -1);
					if( err != AMX_ERR_NONE )
					{
						Error("AMX Error: %s\n", amx_StrError(err));
						loading_ok= false;
					}
					else
					{
						amx_SetUserData(&amx, AMX_USERTAG('P','L','U','G'), this);
						amx_SetUserData(&amx, AMX_USERTAG('F','I','L','E'), path);
						amx_SetUserData(&amx, AMX_USERTAG('S','A','V','E'), &savedData);

						Output("** SMALL script loaded: %s\n", sma_path.c_str());
						loading_ok= true;
					}
				}
			}
		}
	}
	else
	{
		loading_ok= false;
	}

	// remove amx file (temp file)
	unlink(amx_path.c_str());
}

SmallScript::~SmallScript()
{
	// tell all the dll that the plugin is unloaded
	for(uint i= 0; i< extensions.size(); i++)
		extensions[i]->call(this, "UnloadPlugin");
	if( program != NULL )
		delete [] (char*)program;
}

void SmallScript::loadExtensions()
{
	int i;
	AMX_FUNCSTUB *lib;
	char libname[sNAMEMAX+8];  /* +1 for '\0', +3 for 'amx' prefix, +4 for extension */
	AMX_HEADER *hdr=(AMX_HEADER *)amx.base;
	int numlibraries=NUMENTRIES(hdr,libraries,pubvars);

	for(i= 0; i< numlibraries; i++)
	{
		lib= GETENTRY(hdr,libraries,i);
		strcpy(libname, "libs/amx");
		strcat(libname, GETENTRYNAME(hdr,lib));
#if defined WIN32
		strcat(libname, ".dll");
#else
		strcat(libname, ".so");
#endif

		// load the dll or get a pointer to it if loaded
		DynamicExtension *ex= extPool.load(libname);
		if( ex != NULL )
		{
			extensions.push_back(ex);
			ex->call(this, "Init");
		}
	}
}

void SmallScript::showAMXInfos()
{
	int pub, nat, len;
	amx_NameLength(&amx, &len);


	amx_NumPublics(&amx, &pub);
//	printf("- Number of public functions: %d\n", tmp);
/*
	char buff[len+1];
	int i;
	for( i= 0; i< tmp; i++ )
	{
		amx_GetPublic(amx, i, buff);
		AmxDebug("%d - %s\n", i, buff);
	}
*/

	amx_NumNatives(&amx, &nat);
//	printf("- Number of natives function used: %d\n", tmp);
/*
	for( i= 0; i< tmp; i++ )
	{
		amx_GetNative(amx, i, buff);
		AmxDebug("%d - %s\n", i, buff);
	}
*/
/*
	int tmp;
	char buff[len+1];
	amx_NumLibraries(&amx, &tmp);
	Output("- Number of libraries used: %d\n", tmp);

	for(int i= 0; i< tmp; i++)
	{
		amx_GetLibrary(&amx, i, buff);
		Output("  %d - %s\n", i+1, buff);
	}

*/

/*
	long codesize, datasize, stackheapsize;
	amx_MemInfo(amx, &codesize, &datasize, &stackheapsize);
	printf("- Memory infos: code( %ldk ) data( %ldk ) stackheap( %ldk )\n", codesize/1024, datasize/1024, stackheapsize/1024);
*/
}

// build AMX parameter list from C/C++ data type
int SmallScript::callFunctionEx(const char *event, const char *format, va_list va)
{
	if( isEnabled() )
	{
		cell func_ret= 0;
		int ret= 0;
		uint i, pcount= strlen(format);

		cell amx_addr[20];
		cell *phys_addr[20];

		cell to_free= -1;

		uint current_array= 0;

		if( pcount > 0 )
		{
			for(i= 0; i< pcount; i++)
			{
				// User object (user_Info) (pass the nick to callback func)
				if( format[i]=='u' )
				{
					user_Info *u= va_arg(va, user_Info*);
					const char *s= u->nick.c_str();
					amx_Allot(&amx, strlen(s)/sizeof(cell)+1, &amx_addr[i], &phys_addr[i]);
					amx_SetString(phys_addr[i], s, 1, 0);

					if( to_free == -1 )
						to_free= amx_addr[i];

				}
				// Channel object (chan_Info)
				else if( format[i]=='c' )
				{
					chan_Info *c= va_arg(va, chan_Info*);
					const char *s= c->name.c_str();
					amx_Allot(&amx, strlen(s)/sizeof(cell)+1, &amx_addr[i], &phys_addr[i]);
					amx_SetString(phys_addr[i], s, 1, 0);

					if( to_free == -1 )
						to_free= amx_addr[i];

				}
				// integer parameter
				else if( format[i]=='i' )
				{
					int n= va_arg(va, int);
					amx_addr[i]= n;
				}
				// string parameter
				else if( format[i]=='s' )
				{
					char *s= va_arg(va, char*);
					amx_Allot(&amx, strlen(s)/sizeof(cell)+1, &amx_addr[i], &phys_addr[i]);
					amx_SetString(phys_addr[i], s, 1, 0);

					if( to_free == -1 )
						to_free= amx_addr[i];
				}
				// vector => multiple string parameters
				else if( format[i]=='w' )
				{
					const vector<string> &array= *va_arg(va, vector<string>*);
					for(uint k= 0; k< array.size(); k++)
					{
						amx_Allot(&amx, array[k].length()/sizeof(cell)+1, &amx_addr[i+k], &phys_addr[i+k]);
						amx_SetString(phys_addr[i+k], array[k].c_str(), 1, 0);
						if( to_free == -1 )
							to_free= amx_addr[i+k];

						pcount++;
					}
					// accepted only as last parameter
					break;
				}
				// vector => string_array
				else if( format[i]=='v' )
				{
					uint k;
					const vector<string> &array= *va_arg(va, vector<string>*);

					string_array::clear(current_array);

					for(k= 0; k< array.size(); k++)
						string_array::add(current_array, array[k]);

					amx_addr[i]= current_array;

					if( ++current_array >= 9 )
					{
						Error("%s called with too many array parameters !\n", event);
						break;
					}
				}
				else
				{
					Debug("[callFunctionEx] (%s :: %s) not called (unknow format: '%c')\n", getName(), event, format[i]);
				}
			}
		}

		ret= callFunction(event, pcount, amx_addr, &func_ret);

		if( ret != AMX_ERR_NONE )
		{
			if( ret != AMX_ERR_NOTFOUND )
			{
				Output(COLOR_RED);
				AmxDebug("[%s] %s : ERROR %s (%d)\n", getName(), event, amx_StrError(ret), ret);
				Output(COLOR_RESET);
			}
		}
		else
		{
			//AmxDebug("[%s] %s : OK (ret: %d)\n", getName(), event, func_ret);
		}

		// call amx_Release for the first allocated block
		// for infos just have a look at how the Small engine "allocate" memory
		if( to_free != -1 )
			amx_Release(&amx, to_free);

		return func_ret;
	}
	else
	{
		Debug("Function call (%s) aborted for plugin %s (plugin disabled)\n", event, getName());
		return 0;
	}
}

int SmallScript::callFunction(const char *func, int numparam, cell *plist, cell *ret_val)
{
	if( isEnabled() )
	{
		int index, err= 0;
/*
		if( !strcmp(func, "event_onConnected") )
		{
			setAMXPubVar("max_nicklen", 	bot.getMaxNickLen());
			setAMXPubVar("max_topiclen", 	bot.getMaxTopicLen());
		}
*/
		err= amx_FindPublic(&amx, const_cast<char*>(func), &index);
		if( err != AMX_ERR_NONE )
			return err;

		err= amx_Execv(&amx, ret_val, index, numparam, plist);

		for(uint i= AMX_ERR_NONE; i< extensions.size(); i++)
			extensions[i]->call(this, "EndOfAMXCall");

		return err;
	}
	else
	{
		Debug("[callFunction] (%s :: %s) not called (plugin disabled)\n", getName(), func);
		return 0;
	}
}

void SmallScript::setAMXPubVar(const char *name, cell value)
{
	cell amx_addr, *phys_addr;
	int err;

	err= amx_FindPubVar(&amx, name, &amx_addr);
	if( err == AMX_ERR_NONE )
	{
		err= amx_GetAddr(&amx, amx_addr, &phys_addr);
		if( (err == AMX_ERR_NONE) && (phys_addr != NULL) )
		{
			*phys_addr= value;
		}

	}

}

/**/

