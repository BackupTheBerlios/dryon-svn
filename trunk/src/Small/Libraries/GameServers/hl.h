#ifndef _HL_H
#define _HL_H


#include <fcntl.h>
#include <string>

#if !defined(WIN32)
#include <unistd.h>
#endif

using namespace std;

#define SA2_INFO	'C'


typedef unsigned char uchar;


struct server_info {
//	int  n;				// -1
//	char c;				// ASCII 'C' (info response, S2A_INFO)
	string addr;		// net address of server
	string hostname;	// name of the host / server
	string curmap;		// name of the map
	string gdir;		// game directory (i.e. valve/)
	string gdesc;		// Game description (e.g. "half-life multiplay")
	string country;
	uchar cur_clients;	// active client count
	uchar max_clients;	// maximum clients allowed
	uchar pvers;		// protocol version (currently 7)
};

typedef unsigned int uint;

struct rcon_serv {
	string server_addr;
	string passwd;
	uint port;
	int challenge_id;
};

//int get_info(string server_addr, server_info *s_info);
//int get_infos(const char *server_addr, int port, server_info *s_info);
//int ping_host(const char *server_addr, int port);

//int rcon_init(struct serv *srv, const char *server_addr, int port, const char *password);
//string rcon_send_command(struct serv *srv, const char *command);

#endif // _HL_H

