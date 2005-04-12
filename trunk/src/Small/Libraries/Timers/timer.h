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

#ifndef _TIMER_H
#define _TIMER_H

#include "config.h"
#include "thread.h"
#include "Small/amx_script.h"
#include <map>
#include <string>

#define MAX_TIMER_COUNT 20

class TimedEvent
{
public:
	TimedEvent()
	{

	};

	TimedEvent(Script *p, const string &func, uint delay)
	{
		plugin= p;
		func_name= func;
		delay_sec= delay;
	};

	Script *plugin;
	string	func_name;
	uint 	delay_sec;
	time_t 	next;
	vector<string> vCallbackParams;
};

class Timer : public PThread
{
private:
	TimedEvent events[MAX_TIMER_COUNT];
	uint timers_used;
	Mutex events_mutex;

public:
	Timer();
	~Timer();
	bool isIDValid(uint);
	void user_func();
	uint addEvent(Script *p, const string &func, uint seconds, const vector<string> &vParams);
	void removeEvent(uint event_id);
	void removePluginEvents(Script *p);
};


#endif // _TIMER_H

