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

#define EXTENSION_MODULE

#include "config.h"
#include "timer.h"
#include "log.h"
#include "natives.h"
#include <time.h>

#if defined(__FreeBSD__)
#include <unistd.h>
#endif

Timer *func_timer;

extern "C"
{
	Logs EXPORT *amx_log;
	Logs EXPORT *error_log;
	Logs EXPORT *debug_log;
}

Timer::Timer()
{
	timers_used= 0x01;
	//Debug("[TIMER THREAD][%d] Started\n", myid);
}

Timer::~Timer()
{
	//Debug("Timer::~Timer(%d)\n", myid);
}

// return true if the id contains an active timer
bool Timer::isIDValid(uint id)
{
	return ((id!=0) && ((timers_used & (1<<id)) != 0));
}

void Timer::removePluginEvents(Script *p)
{
	for(uint i= 1; i< MAX_TIMER_COUNT; i++)
	{
		if( isIDValid(i) && (events[i].plugin==p) )
			removeEvent(i);
	}
}

void Timer::removeEvent(uint event_id)
{
	if( (event_id!=0) && ((timers_used & (1<<event_id)) != 0) )
	{
		MUTEX_GET(events_mutex);
		{
			timers_used&= ~(1<<event_id);
			Debug("[SCHEDULER] Event removed: %s\n", events[event_id].func_name.c_str());
		}
		MUTEX_RELEASE(events_mutex);
	}
}

uint Timer::addEvent(Script *p, const string &func, uint seconds, const vector<string> &vParams)
{
	bool add= true;
	TimedEvent ev(p, func, seconds);

	uint newid;
	// find a free index
	for(newid= 1; newid< MAX_TIMER_COUNT; newid++)
	{
		if( !isIDValid(newid) )
			break;
	}

	if( newid < MAX_TIMER_COUNT )
	{
/*
		// look in the existing events to see if it is not already registered
		for(uint i= 1; i< MAX_TIMER_COUNT; i++)
		{
			TimedEvent &e= events[i];
			// event already registered, return
			if( isIDValid(i) && (p == e.plugin) && (func == e.func_name) && (seconds == e.delay_sec) )
			{
				add= false;
				break;
			}
		}
*/
		if( add )
		{

			MUTEX_GET(events_mutex);
			{
				ev.next= time(NULL) + ev.delay_sec;
				ev.vCallbackParams= vParams;
				events[newid]= ev;
			}
			MUTEX_RELEASE(events_mutex);

			Debug("[SCHEDULER] Event added by \"%s\": %s (delay: %ds) \n", p->getName(), func.c_str(), seconds);
			timers_used|= (1<<newid);
			return newid;
		}
	}

	return 0;
}

void Timer::user_func()
{
	//Debug("[SCHEDULER] Started.\n");
	for(int i= 0; i< 20; i++)
	{
		usleep(250000);
		cancel_point();
	}

	while(1)
	{
		usleep(250000);
		cancel_point();

		MUTEX_GET(events_mutex);
		{
			for(uint i= 1; i< MAX_TIMER_COUNT; i++)
			{
				if( (timers_used & (1<<i)) != 0 )
				{
					TimedEvent &ev= events[i];
					time_t timestamp= time(NULL);

					cancel_point();

					if( ev.next <= timestamp )
					{
						ev.plugin->callFunctionEx(ev.func_name.c_str(), "iw", i, &ev.vCallbackParams);
						ev.next= timestamp + ev.delay_sec;
						Debug("[SCHEDULER] Executing event %s::%s\n", ev.plugin->getName(), ev.func_name.c_str());
					}
				}
			}
		}
		MUTEX_RELEASE(events_mutex);
	}

	//Debug("[SCHEDULER] Exited.\n");
}

/****c* Small/TimedEvents_Extension
* LIBRARY
*	Timers
* INCLUDE FILE
*	timers.inc
* MODULE DESCRIPTION
*	handling of timed events
***/

/****f* TimedEvents_Extension/timer_add
* USAGE
*	timer_id:timer_add(const name[], seconds, ...)
* INPUTS
*	name    : name of the function to call
*	seconds : seconds count between each call
*	...     : parameters to send to callback function (%VarArgFormat%)
* RETURN VALUE
*	timer identifier
***/
NATIVE(_addTimedEvent)
{
	int pcount= params[0]/sizeof(cell);
	Script *plugin;
	char *name;
	vector<string> vParams;

	amx_GetUserData(amx, AMX_USERTAG('P','L','U','G'), (void**)&plugin);
	copyStringFromAMX(amx, params[1], &name);

	for(int i= 3; i<= pcount; i++)
	{
		char *str;
		copyStringFromAMX(amx, params[i], &str);
		vParams.push_back(str);
		delete [] str;
	}

	cell tid= func_timer->addEvent(plugin, name, params[2], vParams);

	delete [] name;
	return tid;
}

/****f* TimedEvents_Extension/timer_remove
* USAGE
*	timer_remove(timer_id:id)
* INPUTS
*	id : timer identifier ( returned by %timer_add% )
***/
NATIVE(_removeTimedEvent)
{
	Script *plugin;
	amx_GetUserData(amx, AMX_USERTAG('P','L','U','G'), (void**)&plugin);

	func_timer->removeEvent(params[1]);
	return 0;
}

/****f* TimedEvents_Extension/timer_valid
* USAGE
*	bool:timer_valid(timer_id:id)
* DESCRIPTION
*	test if id is valid
* INPUTS
*	id : timer identifier ( returned by %timer_add% )
***/
NATIVE(_isIDValid)
{
	return func_timer->isIDValid(params[1]);
}

extern "C"
void EXPORT amxbot_Timers_Init(Script *_p)
{
	SmallScript *p= (SmallScript*)_p;
	static AMX_NATIVE_INFO timers_Natives[] = {
		{"timer_add",		_addTimedEvent},
		{"timer_remove",	_removeTimedEvent},
		{"timer_valid",		_isIDValid},
		{ 0, 0 }        		/* terminator */
		};
	AMX *amx= p->getAMX();
	amx_Register(amx, timers_Natives, -1);
	func_timer= new Timer;
	func_timer->run();
}

extern "C"
void EXPORT amxbot_Timers_UnloadPlugin(Script *p)
{
	func_timer->removePluginEvents(p);
}

extern "C"
void EXPORT amxbot_Timers_EndOfAMXCall(Script *p)
{

}

extern "C"
void EXPORT amxbot_Timers_Cleanup(Script *p)
{
	delete func_timer;
}

/**/


