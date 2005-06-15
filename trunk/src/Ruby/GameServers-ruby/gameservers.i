%module GameServers

%{
#include "hl.h"
%}

%typemap(in) string {
	Check_Type($input, T_STRING);
	$1= std::string( StringValueCStr($input) );
}

%rename(HLServerInfos) server_info;

struct server_info
{
%immutable;
	%name(address) string addr;
	string hostname;
	string curmap;
	%name(gamedir) string gdir;
	%name(gamedesc) string gdesc;
	string country;
	int cur_clients;
	int max_clients;
%mutable;
};

class HLServer
{
public:
	HLServer(string, int);
};

%{
	int HLServer_ping(HLServer*, const char *, int port= 27015);
	server_info *HLServer_get_infos(HLServer*, string, int);
%}
%extend HLServer
{
public:
	int ping(const char *host, int port= 27015);
	server_info *get_infos(string server_addr, int port);
};

