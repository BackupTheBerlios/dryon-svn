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

#include "../../config.h"
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include <string>
#include <vector>

#include "hl.h"
#include "../../tokens.h"
#include "../../log.h"

#if defined(WIN32)
#	include <winsock.h>
#else
#	include <sys/ioctl.h>
#	include <arpa/inet.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netdb.h>
#endif

using namespace std;

int ping_host(const char *, int);
int get_infos(const char *, int, server_info*);

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

int HLServer_ping(const char *host, int port= 27015)
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

#if defined(WIN32)
__declspec(dllexport)
#endif
void Init_mysql(void)
{
	
}

/**/






