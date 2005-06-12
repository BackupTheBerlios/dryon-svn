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
	string address;
	string hostname;
	string curmap;
	string gamedir;
	string gamedesc;
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
	int HLServer_ping(const char *host, port= 27015);
%}
%extend HLServer
{
public:
	int ping(const char *host, port= 27015);
	server_info *get_infos(string server_addr, int port);
};

