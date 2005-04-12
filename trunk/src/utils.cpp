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

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>


#if defined(WIN32)
	#include <winsock.h>
	#include <direct.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
#endif


#include "utils.h"
#include "tokens.h"
//#include "log.h"


///////////////////////////////////////////
// Utils

bool fileExist(const char *path)
{
	struct stat sb;

	if( (stat(path, &sb)==-1) && (errno==ENOENT) )
		return false;
	else
		return true;
}

int createFolder(const char *path)
{
#if defined(WIN32)
	//return (SHCreateDirectory(NULL, path) == ERROR_SUCCESS)?0:-1 ;
	return mkdir(path);
#else
	return mkdir(path, S_IRWXU | S_IRGRP );
#endif
}

bool is_numeric(char c)
{
	return ((c>='0') && (c<='9'));
}

// remove white spaces
// "  un test "
string trim(const string &s)
{
	string tmp= s;

	while( tmp[0]==' ' )
		tmp.erase(0, 1);

	while( tmp[tmp.size()-1]==' ' )
		tmp.erase(tmp.size()-1, 1);

	return tmp;
}

// src is modified directly !
void changeEXT(char *src, char *newext)
{
	strcpy(src+strlen(src)-3, newext);
}

string buildFilename(const string &base, const string &ext)
{
	return base + "." + ext;
}

/*
// ret= match ?
bool matchregexp(const char *regexp, const char *str)
{
	char buff[100];
	bool ret= true;
	regex_t reg;
	int n;

	if( !strcmp(regexp, "") )
		return false;

	if( (n= regcomp(&reg, regexp, 0)) != 0 )
	{
		regerror(n, &reg, buff, sizeof(buff)-1);
		Debug("%s", buff);
	}

	if( regexec(&reg, str, 0, NULL, 0) )
		ret= false;

	regfree(&reg);
	return ret;
}
*/


int parseDateTime(string date_str, string time_str, struct tm &ret)
{
	vector<string> tmp;
	//string tmp[3];
	time_t now= time(NULL);
	struct tm *tm2= localtime(&now);

	memcpy(&ret, tm2, sizeof(struct tm));

	//int r= explode(date_str, "/", tmp, 3);
	Tokenize(date_str, tmp, "/");
	if( tmp.size() != 3 )
	{
		//Error("Wrong date format: %s\n", date_str.c_str());
		return -1;
	}

	ret.tm_mday= 	atoi( tmp[0].c_str() );
	ret.tm_mon= 	atoi( tmp[1].c_str() ) - 1;
	ret.tm_year= 	atoi( tmp[2].c_str() ) - 1900;

	//r= explode(time_str, ":", tmp, 2);
	Tokenize(time_str, tmp, ":");
	if( tmp.size() != 2 )
	{
		//Error("Wrong time format: %s\n", time_str.c_str());
		return -1;
	}

	ret.tm_hour= 	atoi( tmp[0].c_str() );
	ret.tm_min= 	atoi( tmp[1].c_str() );
	ret.tm_sec= 	0;

	return mktime(&ret);
}

////////////////
// network

int MakeConnection(const char *host, int port)
{
	struct sockaddr_in sin;
	sin.sin_family= AF_INET;

	int sock= socket(PF_INET, SOCK_STREAM, 0);

	if( sock==-1 )
	{
#if defined(WIN32)
		//Error("socket: %d\n", WSAGetLastError());
#else
		//Error("socket: %s\n", strerror(errno));
#endif
		return -1;
	}

	// first, look up hostname
	struct hostent *hostent= gethostbyname(host);
	if(hostent == NULL)
	{
		//Error("unable to resolve %s\n", host);
		return -1;
	}

	if(hostent->h_addrtype == AF_INET)
	{
		memcpy(&sin.sin_addr.s_addr, hostent->h_addr, hostent->h_length);
		sin.sin_port= htons(port);
	}
	else
	{
		//Error("IPv6 address not supported\n");
		return -1;
	}

	//Debug("Connecting to %s:%d...\n", host, port);

	int ret= connect(sock, (struct sockaddr*)&sin, sizeof(struct sockaddr));
	if(ret == -1)
	{
		//Error("connection error: %s\n", strerror(errno));
		return -1;
	}

	int val= 1;
/*
struct  linger {
        int     l_onoff;                // option on/off
        int     l_linger;               // linger time
};

*/


#if defined(WIN32)
	setsockopt(sock, IPPROTO_TCP, SO_KEEPALIVE, (const char *) &val, sizeof(int));
	setsockopt(sock, IPPROTO_TCP, SO_LINGER, (const char *) &val, sizeof(int));
#else
	struct linger s_lin= {1,5};
	int err= setsockopt(sock, IPPROTO_TCP, SO_KEEPALIVE, (void *) &val, sizeof(int));
	if( err )
		perror("setsockopt keepalive");

	err= setsockopt(sock, SOL_SOCKET, SO_LINGER, (void *)&s_lin, sizeof(s_lin));
	if( err )
		perror("setsockopt linger");
#endif

	//Debug("Connected to %s:%d\n", host, port);
	return sock;
}

#if defined(TEST)

int main()
{

	for( int i= 0; i< 3000; i++ )
	{
		string s1= "MODE #bidon2 :+s\r\n";
		vector<string> parts;

		Tokenize(s1, parts, " ");
		printf(" %d -", parts.size());
	}

	printf("\n");

	return 0;
}
#endif

/**/

