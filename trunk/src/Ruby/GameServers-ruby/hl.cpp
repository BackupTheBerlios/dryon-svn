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


#include "../ruby/ruby.h"

#include "config.h"
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include "hl.h"
#include "tokens.h"
//#include "log.h"

#if defined(WIN32)
#	include <winsock.h>
#else
#	include <sys/ioctl.h>
#	include <arpa/inet.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netdb.h>
#endif

/****h* Ruby/GameServers
* LIBRARY
*	GameServers
* MODULE DESCRIPTION
*	Module to retrieve informations from game servers, currently supported:
*	- HalfLife
***/

HLServer::HLServer(string _host, int _port)
{
	host= _host;
	port= _port;
}

int HLServer_ping(const char *host, port= 27015)
{
	return ping_host(host, port);
}

server_info *get_infos(string server_addr, int port);

/////////////////////////////////////////
//////////////////////////////////////

int ping_host(const char *server_addr, int port)
{
	unsigned long NonBlock= 1;
	int s;
	struct sockaddr_in sin;
	socklen_t len= sizeof(struct sockaddr_in);
	unsigned char cmd[]= {0xFF, 0xFF, 0xFF, 0xFF, 'p', 'i', 'n', 'g', 0x00};
	char c[5];

	s= socket(PF_INET, SOCK_DGRAM, 0);
// non blocking socket
#if defined(WIN32)
	if( ioctlsocket(s, FIONBIO, &NonBlock) == SOCKET_ERROR )
	{
		Error("ioctlsocket: %d\n", WSAGetLastError());
		return -1;
	}
#else
    if( fcntl(s, F_SETFL, O_NONBLOCK) == -1)
    {
    	Error("fcntl: %s\n", strerror(errno));
    	return -1;
    }
#endif

	struct hostent *hostent= gethostbyname(server_addr);
	if(hostent == NULL)
	{
		Error("unable to resolve %s\n", server_addr);
		return -1;
	}

	if(hostent->h_addrtype == AF_INET)
	{
		memcpy(&sin.sin_addr.s_addr, hostent->h_addr, hostent->h_length);
	}
	else
	{
		Error("invalid address: %s\n", server_addr);
		return -1;
	}

	sin.sin_family= AF_INET;
	sin.sin_port= htons(port);

	// envoi de la commande
	sendto(s, (char*)cmd, 9, 0, (struct sockaddr *)&sin, len);

	int i= 0;
	while( i++ < 5 )
	{
		usleep(1000);
		if( recvfrom(s, c, 5, 0, (struct sockaddr *)&sin, &len)!=-1 )
			break;
	}
	close(s);
	if( c[4] == 'j' )
		return 0;
	else
		return -1;
}


int get_infos(const char *server_addr, int port, server_info *s_info)
{
	int s;
	struct sockaddr_in sin;
	socklen_t len= sizeof(struct sockaddr_in);
	unsigned char cmd[]= {0xFF, 0xFF, 0xFF, 0xFF, 'i', 'n', 'f', 'o', 0x00};
	char packets[200];
	char *packet= packets;

	s= socket(PF_INET, SOCK_DGRAM, 0);
	sin.sin_family= AF_INET;
	sin.sin_addr.s_addr= inet_addr(server_addr);
	sin.sin_port= htons(port);

	// envoi de la commande
	sendto(s, (char*)cmd, 9, 0, (struct sockaddr *)&sin, len);
	recvfrom(s, packet, 200, 0, (struct sockaddr *)&sin, &len);

	if( *((int*)packet) != -1 )
		return -1;

	if( *((char*)(packet+4)) != 'C' )
		return -1;

	packet+= 5;

	// locate addr string
	s_info->addr= string(packet);
	packet+= s_info->addr.size()+1;

	s_info->hostname= string(packet);
	packet+= s_info->hostname.size()+1;

	s_info->curmap= string(packet);
	packet+= s_info->curmap.size()+1;

	s_info->gdir= string(packet);
	packet+= s_info->gdir.size()+1;

	s_info->gdesc= string(packet);
	packet+= s_info->gdesc.size()+1;

	s_info->cur_clients= *((uchar*)packet);
	s_info->max_clients= *((uchar*)(packet+1));
	s_info->pvers= *((uchar*)(packet+2));

	close(s);
	return 0;
}

/////////////////////
// rcon

/*
The packet should start with 4 consecutive bytes of 255 (32-bit integer -1) and the string:
"challenge rcon\n".
The server will respond to the requesting system on the purported remote IP address and port with four 255's and:
"challenge rcon number\n" where number is an unsigned int32 number.
*/
int rcon_init(struct rcon_serv *srv, const char *server_addr, int port, const char *password)
{
	uint id;
	struct sockaddr_in sin;
	socklen_t len= sizeof(struct sockaddr_in);
	unsigned char cmd[40]= {0xFF, 0xFF, 0xFF, 0xFF};
	strcpy((char*)cmd+4, "challenge rcon\n");

	if( ping_host(server_addr, port) == -1 )
		return -1;

	//printf("Sending '%*s'\n", strlen(cmd)-2, cmd);

	sin.sin_family= AF_INET;
	sin.sin_addr.s_addr= inet_addr(server_addr);
	sin.sin_port= htons(port);

	int s= socket(PF_INET, SOCK_DGRAM, 0);
	sendto(s, (char*)cmd, 19, 0, (struct sockaddr *)&sin, len);
	recvfrom(s, (char*)cmd, 40, 0, (struct sockaddr *)&sin, &len);

	//printf("received '%*s'\n", strlen(cmd)-2, cmd);
	sscanf((char*)cmd+4, "challenge rcon %u", &id);
	//printf("id= %u\n", id);

	srv->server_addr= string(server_addr);
	srv->passwd= string(password);
	srv->port= port;
	srv->challenge_id= id;
	return 0;
}

/*
To issue the actual rcon, the remote App then responds with a UDP packet containing 4 255s and:
"rcon number \"password\" rconcommands" where password is the rcon_password ( should be enclosed in quotes as noted so that multiple word passwords will continue to work ),
number is the unsigned int32 number received from the server and rconcommands is the actual rcon command string.
*/
string rcon_send_command(struct rcon_serv *srv, const char *command)
{
	struct sockaddr_in sin;
	socklen_t len= sizeof(struct sockaddr_in);
	unsigned char cmd[200]= {0xFF, 0xFF, 0xFF, 0xFF};
	sprintf((char*)cmd+4, "rcon %d \"%s\" %s", srv->challenge_id, srv->passwd.c_str(), command);

	sin.sin_family= AF_INET;
	sin.sin_addr.s_addr= inet_addr(srv->server_addr.c_str());
	sin.sin_port= htons(srv->port);

	int s= socket(PF_INET, SOCK_DGRAM, 0);
	sendto(s, (char*)cmd, strlen((char*)cmd)+1, 0, (struct sockaddr *)&sin, len);
	recvfrom(s, (char*)cmd, 200, 0, (struct sockaddr *)&sin, &len);

	//printf("%s\n", cmd+5);
return string((char*)cmd+5);
}

#ifdef TESTINFOS
int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		printf("Usage: %s <addr> <port>\n", argv[0]);
		exit(1);
	}

	char *addr= argv[1];
	int port= atoi(argv[2]);

	if( !ping_host(addr, port) )
	{
		server_info nfo;

		get_info(addr, port, &nfo);
		printf("server name: %s\n", nfo.hostname.c_str());
		printf("players: %2d/%2d\n", nfo.cur_clients, nfo.max_clients);
		return 0;
	}
	else
	{
		printf("unable to ping server !\n");
		return 1;
	}
}
#else
#ifdef TESTRCON
int main(int argc, char *argv[])
{
	if(argc != 4)
	{
		printf("Usage: %s <addr> <port> <passwd>\n", argv[0]);
		exit(1);
	}

	char *addr= argv[1];
	char *passwd= argv[3];
	int port= atoi(argv[2]);

	if( !ping_host(addr, port) )
	{
		struct serv server;

		rcon_init(&server, addr, port, passwd);
		cout << rcon_send_command(&server, "status");
		cout << rcon_send_command(&server, "map ns_hera");

		return 0;
	}
	else
	{
		printf("unable to ping server !\n");
		return 1;
	}
}
#endif
#endif


/****************************/

/****f* GameServers/hl_ping
* USAGE
*	hl_ping(host, port= 27015)
* RETURN VALUE
*	boolean : true if the server answered
* INPUTS
*	host : halflife server hostname or ip or "host:ip"
*	port : halflife server port (if port not found in host)
***/
NATIVE(_ping)
{
	char *chost;
	cell ret= 0;
	vector<string> parts;
	copyStringFromAMX(amx, params[1], &chost);
	string host(chost);
	cell port= params[2];

	Tokenize(host, parts, ":");
	if( parts.size() == 2 )
	{
		host= parts[0];
		port= atoi(parts[1].c_str());
	}

	if( ping_host(host.c_str(), port) != -1 )
		ret= 1;

	delete [] chost;
	return ret;
}

/****f* GameServers_Extension/hl_servinfos
* USAGE
*	hl_infos:hl_servinfos(const host[], port= 27015)
* RETURN VALUE
*	return an identifier for the infos or 0 if it failed
* INPUTS
*	host : halflife server hostname or ip or "host:ip"
*	port : halflife server port (if port not found in host)
***/

#define MAX_HL_SERVINFOS	5

struct server_info infos[MAX_HL_SERVINFOS];
uint slots_used= 0x01;	// 0 is used to return an error so it's not a valid id

NATIVE(_get_infos)
{
	uint n;
	cell ret;

	// find free slot
	for(n= 0; n< MAX_HL_SERVINFOS; n++)
	{
		if( (slots_used & (1<<n)) == 0 )
			break;
	}

	if( n< MAX_HL_SERVINFOS )
	{
		char *chost;
		vector<string> parts;
		cell port= params[2];
		copyStringFromAMX(amx, params[1], &chost);
		string host(chost);

		Tokenize(host, parts, ":");
		if( parts.size() == 2 )
		{
			host= parts[0];
			port= atoi(parts[1].c_str());
		}

		if( ping_host(host.c_str(), port) == 0 )
		{
			get_infos(host.c_str(), port, &infos[n]);
			slots_used|= (1<<n);
			ret= n;
		}
		else
		{
			Error("hl_servinfos: no answer from server %s:%d\n", host.c_str(), port);
			ret= 0;
		}

		delete [] chost;
	}
	else
	{
		Error("hl_servinfos: all the slots are full\n");
		ret= 0;
	}

	return ret;
}


/****f* GameServers_Extension/hl_readinfos_string
* USAGE
*	hl_readinfos_string(hl_infos:id, const what[], dest[], dest_maxsize= sizeof dest)
* INPUTS
*	id           : infos identifier returned by %hl_servinfos%
*	what         : one of: "map"
*	dest         : where to store the string
*	dest_maxsize : %Natives%
***/
NATIVE(_read_string_infos)
{
	char *what;
	cell id= params[1];

	if( ((slots_used & (1<<id)) == 0) || (id >= MAX_HL_SERVINFOS) )
		return -1;

	copyStringFromAMX(amx, params[2], &what);
	if( !strcmp(what, "map") )
	{
		copyStringToAMX(amx, params[3], const_cast<char*>(infos[id].curmap.c_str()), params[4]-1);
	}
	else
	{
		Error("hl_readinfos_string: unknow field name: \"%s\"\n", what);
	}

	delete [] what;
	return 0;
}


/****f* GameServers_Extension/hl_readinfos_int
* USAGE
*	hl_readinfos_int(hl_infos:id, const what[])
* RETURN VALUE
*	integer : value of requested field
* INPUTS
*	id   : infos identifier returned by %hl_servinfos%
*	what : one of: "cur_clients", "max_clients"
***/
NATIVE(_read_int_infos)
{
	cell ret;
	char *what;
	cell id= params[1];

	if( ((slots_used & (1<<id)) == 0) || (id >= MAX_HL_SERVINFOS) )
		return -1;

	copyStringFromAMX(amx, params[2], &what);
	if( !strcmp(what, "cur_clients") )
	{
		ret= infos[id].cur_clients;
	}
	else if( !strcmp(what, "max_clients") )
	{
		ret= infos[id].max_clients;
	}
	else
	{
		Error("hl_readinfos_string: unknow field name: \"%s\"\n", what);
		ret= 0;
	}

	delete [] what;
	return ret;
}

/****h* GameServers_Extension/HL_rcon
* MODULE DESCRIPTION
*	manage rcon connections
***/

/****f* HL_rcon/hl_rcon_init
* USAGE
*	hl_rcon:hl_rcon_init(const pass[], const host[], port= 27015)
* RETURN VALUE
*	rcon connection identifier
* INPUTS
*	pass : rcon password
*	host : halflife server hostname or ip or "host:ip"
*	port : halflife server port (if port not found in host)
***/

#define MAX_HL_RCON	5

struct rcon_serv rcon[MAX_HL_RCON];
uint rcon_slots_used= 0x01;	// 0 is used to return an error so it's not a valid id

NATIVE(_rcon_init)
{
	cell ret;
	// find an empty slot
	uint n;
	// find free slot
	for(n= 0; n< MAX_HL_RCON; n++)
	{
		if( (rcon_slots_used & (1<<n)) == 0 )
			break;
	}

	if( n < MAX_HL_RCON )
	{
		vector<string> parts;
		cell port= params[3];
		char *pass, *chost;
		copyStringFromAMX(amx, params[1], &pass);
		copyStringFromAMX(amx, params[2], &chost);
		string host(chost);

		Tokenize(host, parts, ":");
		if( parts.size() == 2 )
		{
			host= parts[0];
			port= atoi(parts[1].c_str());
		}

		// try to contact the server first
		if( ping_host(host.c_str(), port) == 0 )
		{
			if( rcon_init(&rcon[n], host.c_str(), port, pass) == 0)
			{
				ret= n;
				rcon_slots_used|= (1<<n);
			}
			else
			{
				Error("rcon_init: failed to contact host %s\n", chost);
			}
		}

		delete [] pass;
		delete [] chost;
	}
	else
	{
		Error("rcon_init: no free slot\n");
		ret= 0;
	}

	return ret;
}

/****f* HL_rcon/hl_rcon_close
* USAGE
*	hl_rcon_close(hl_rcon:id)
* INPUTS
*	id : rcon connection identifier returned by %hl_rcon_init%
***/
NATIVE(_rcon_close)
{
	cell id= params[1];

	if( (rcon_slots_used & (1<<id)) != 0 )
	{
		rcon_slots_used&= ~(1<<id);
	}
	return 0;
}

/****f* HL_rcon/hl_rcon_cmd
* USAGE
*	hl_rcon_cmd(hl_rcon:id, const cmd[], out[], out_maxsize= sizeof out)
* INPUTS
*	id          : rcon connection identifier returned by %hl_rcon_init%
*	cmd         : the command to send
*	out         : string to store the server answer
*	out_maxsize : %Natives%
***/
NATIVE(_rcon_cmd)
{
	cell id= params[1];

	// is id valid ?
	if( (rcon_slots_used & (1<<id)) != 0 )
	{
		char *cmd;
		copyStringFromAMX(amx, params[2], &cmd);
		string ret= rcon_send_command(&rcon[id], cmd);
		copyStringToAMX(amx, params[3], const_cast<char*>(ret.c_str()), params[4]-1);
		delete [] cmd;
	}
	return 0;
}

extern "C"
{
	void EXPORT dryon_GameServers_Init(Script *_p)
	{
		SmallScript *p= (SmallScript*)_p;
		static AMX_NATIVE_INFO gameservers_Natives[] = {
			{"hl_ping",  			_ping},
			{"hl_servinfos",		_get_infos},
			{"hl_readinfos_string",	_read_string_infos},
			{"hl_readinfos_int",	_read_int_infos},
			// rcon
			{"hl_rcon_init",		_rcon_init},
			{"hl_rcon_close",		_rcon_close},
			{"hl_rcon_cmd",			_rcon_cmd},
			{0,0}        /* terminator */
			};
		amx_Register(p->getAMX(), gameservers_Natives, -1);
	}

	void EXPORT dryon_GameServers_UnloadPlugin(Script *p)
	{

	}

	void EXPORT dryon_GameServers_EndOfAMXCall(Script *p)
	{

	}

	void EXPORT amxbot_GameServers_Cleanup(Script *p)
	{

	}
}

/**/






