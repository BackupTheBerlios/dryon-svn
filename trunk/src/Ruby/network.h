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

#include "../sockets.h"
#include "ruby.h"

#if !defined(SWIG)
// main version

class RubyTCPClientSocket;

class MonitorThread : public PThread
{
private:

public:
	MonitorThread(RubyTCPClientSocket *v) : PThread((void*)v)
	{

	}

	void user_func();
};

class RubyTCPClientSocket : public TCPClientSocket
{
	friend class MonitorThread;

private:
	MonitorThread th;
	VALUE data_rcv_block, error_block;

public:
	RubyTCPClientSocket();
	int connect(const char *host, unsigned int port);
	int sendText(const char *txt);
	void onDataReceived();
	void onError();
};

#else // interface (for SWIG)
///////////////////////////////
//////////////////////////////

%name(TCPClientSocket) class RubyTCPClientSocket
{
public:
	void onDataReceived();
	void onError();
	int connect(const char *host, unsigned int port);
	int sendText(const char *txt);
};

#endif



