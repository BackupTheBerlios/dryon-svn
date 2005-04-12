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

#ifndef __SOCKETS_H
#define __SOCKETS_H

#include "config.h"

#include <sys/types.h>
#include <stdarg.h>

#if !defined(WIN32)
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#endif


#include <vector>

using namespace std;


enum socket_option{
	SOPT_KEEPALIVE= 0,
	SOPT_LINGER,
	SOPT_SENDTIMEOUT,
	SOPT_RCVTIMEOUT,
	SOPT_REUSE,
};

class Socket
{
protected:
	static bool init_done;
	struct sockaddr_in sin;
	struct timeval timeout;
	int sock;
	char *err_str;
	int err_code;
	bool connected;

	void setError();

public:
	Socket();
	~Socket();
	bool isConnected(){ return connected; }
	char *getError();
	int setOption(socket_option opt, int value);
	static int waitIncomingData(vector<Socket*>&,uint timeout);
	void disconnect();
};

class TCPSocket : public Socket
{
public:
	TCPSocket();
	TCPSocket(int s);
	
	void initSocket();

	// all 3 functions use select for the timeout
	int sendText(const char *format, ...);
	int sendData(const char *buff, uint len);
	int recvData(char *buff, uint maxlen);

	// used with waitIncomingData (select)
	int recvDataDirect(char *buff, uint maxlen);
};

class UDPSocket : public Socket
{
protected:

public:
	UDPSocket();

	int sendText(const char *dest, uint port, const char *format, ...);
	int sendData(const char *dest, uint port, char *buff, uint len);
};

// clients
class TCPClientSocket : public TCPSocket
{
protected:

public:
	int connect(const char *host, uint port);
};

class UDPClientSocket : public UDPSocket
{
protected:

public:

};

// servers
class TCPServerSocket : public TCPSocket
{
public:
	int setup(uint port, int queue);
	TCPSocket *waitConnection();
};

class UDPServerSocket : public UDPSocket
{
protected:

public:
	int setup(const char *from, uint port);
	int recvData(char *buff, uint maxlen);
};



#endif // __SOCKETS_H


/**/

