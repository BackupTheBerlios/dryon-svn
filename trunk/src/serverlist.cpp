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

#include <iostream>
#include <fstream>
#include "log.h"
#include "serverlist.h"

bool ServerList::loadFromFile(string file_name)
{
	char buff[200];
	fstream file;

	file.open(file_name.c_str(), ios::in);
	if( file.bad() )
		return false;

	while( file.getline(buff, sizeof(buff)-1) )
	{
		char *p= buff;
		while( *p && ((*p==' ') || (*p=='\t')) )
			p++;

		if( *p )
		{
			for(uint i= strlen(p)-1; (i!=0) && (buff[i]==' ' || buff[i]=='\t'); i--)
			{
				if( buff[i]==' ' )
					buff[i]= '\0';
			}

			if( (strlen(buff) != 0) && (buff[0]!='#') && (buff[0]!='/') && (buff[1]!='/') )
			{
				slist.push(buff);
			}
		}
	}

	return true;
}

string ServerList::getNextServer()
{
	string ret= "";
	if( slist.size() > 0 )
	{
		ret= slist.front();
		slist.push(ret);
		slist.pop();
	}

	return ret;
}

uint ServerList::getServerCount()
{
	return slist.size();
}


#if defined TEST

int main()
{
	ServerList s;

	s.loadFromFile("servers.txt");
	for(uint i= 0; i< 10; i++)
	{
		cout << s.getNextServer() <<endl;
	}

	return 0;
}

#endif

/**/

