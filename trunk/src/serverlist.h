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

#ifndef _SERVERLIST_H
#define _SERVERLIST_H

#include "config.h"
#include <queue>
#include <string>
#include <sys/types.h>

using namespace std;

class ServerList
{
private:
	queue<string> slist;

public:
	bool loadFromFile(string file_name);
	string getNextServer();
	uint getServerCount();
};



#endif // _SERVERLIST_H



