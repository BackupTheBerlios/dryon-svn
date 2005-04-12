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

/*

Minimal plugin (example)

*/

#define EXTENSION_MODULE

#include <stdio.h>
#include "amx.h"
#include "amx_script.h"
#include "natives.h"
#include "log.h"

/*** COMMON ***/

extern "C"
{
	Logs EXPORT *amx_log;
	Logs EXPORT *error_log;
	Logs EXPORT *debug_log;
}

/*************/

/****h* Small/Mini_Extension
* LIBRARY
*	Mini
* INCLUDE FILE
*	mini.inc
* MODULE DESCRIPTION
*	Just a minimal plugin which can be used as a base for new plugins
***/

/****f* Mini_Extension/add_this
* USAGE
*	add_this(n1, n2)
* RETURN VALUE
*	integer : n1 + n2
* INPUTS
*	n1 : integer
*	n2 : integer
***/
NATIVE(_add_this)
{
	cell n1= params[1];
	cell n2= params[2];

	return n1+n2;
	return 0;
}

extern "C"
{
	void EXPORT amxbot_Mini_Init(Script *_p)
	{
		SmallScript *p= (SmallScript*)_p;
		static AMX_NATIVE_INFO mini_Natives[] = {
			{ "add_this",  _add_this },
			{ 0, 0 }        /* terminator */
			};
		amx_Register(p->getAMX(), mini_Natives, -1);
		Debug("**( Mini: loaded )**\n");
	}

	void EXPORT amxbot_Mini_UnloadPlugin(Script *p)
	{
		Debug("**( Mini: plugin unloaded <%s> )**\n", p->getName());
	}

	void EXPORT amxbot_Mini_EndOfAMXCall(Script *p)
	{
		Debug("**( Mini: end of function call )**\n");
	}

	void EXPORT amxbot_Mini_Cleanup(Script *p)
	{
		Debug("**( Mini: unloaded )**\n");
	}
}



/**/


