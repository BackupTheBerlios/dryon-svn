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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>

#if !defined(WIN32)
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#endif


//#include <fcntl.h>

#include <vector>

#include "sockets.h"

//
// Socket
//

bool Socket::init_done= false;

Socket::Socket()
{
// under windows we have to 'start' the network
// what a stupid thing...
#if defined(WIN32)
	if( !init_done )
	{
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2,0), &wsaData);
		init_done= true;
	}
#endif
	// variables init
	err_str= NULL;
	connected= false;
	err_code= 0;
	sock= -1;
	timeout.tv_usec= 0;
	timeout.tv_sec= 1;

	sin.sin_family= AF_INET;
	//fcntl(sock, O_NONBLOCK, 1);
}

Socket::~Socket()
{
	disconnect();
}

void Socket::disconnect()
{
	if( sock != -1 )
	{
		close(sock);
		connected= false;
	}
}

void Socket::setError()
{
#if !defined(WIN32)
	err_str= strerror(errno);
#else
	err_code= WSAGetLastError();
#endif
}

char *Socket::getError()
{
	char *ret;

	if( err_str != NULL )
	{
		ret= err_str;
		err_str= NULL;
	}
#if defined(WIN32)
	else
	{
		switch( err_code )
		{
		case WSAEINTR:				ret= "Interrupted function call."; break;
		case WSAEACCES:				ret= "Permission denied."; break;
		case WSAEFAULT:				ret= "Bad Address."; break;
		case WSAEINVAL:				ret= "Invalid Argument."; break;
		case WSAEMFILE:				ret= "Too many open files."; break;
		case WSAEWOULDBLOCK:		ret= "Ressource temporarily unavailable."; break;
		case WSAEINPROGRESS:		ret= "Operation now in progress."; break;
		case WSAEALREADY:			ret= "Operation already in progress."; break;
		case WSAENOTSOCK:			ret= "Socket operation on non socket."; break;
		case WSAEDESTADDRREQ:		ret= "Destination address required."; break;
		case WSAEMSGSIZE:			ret= "Message too long."; break;
		case WSAEPROTOTYPE:			ret= "Protocol wrong type for socket."; break;
		case WSAENOPROTOOPT:		ret= "Bad protocol option."; break;
		case WSAEPROTONOSUPPORT:	ret= "Protocol not supported."; break;
		case WSAESOCKTNOSUPPORT:	ret= "Socket type not supported."; break;
		case WSAEOPNOTSUPP:			ret= "Operation not supported."; break;
		case WSAEPFNOSUPPORT:		ret= "Protocol family not supported."; break;
		case WSAEAFNOSUPPORT:		ret= "Address family not supported by protocol family."; break;
		case WSAEADDRINUSE:			ret= "Address already in use."; break;
		case WSAEADDRNOTAVAIL:		ret= "Cannot assign requested address."; break;
		case WSAENETDOWN:			ret= "Network is down."; break;
		case WSAENETUNREACH:		ret= "Network unreachable."; break;
		case WSAENETRESET:			ret= "Network dropped connection on reset."; break;
		case WSAECONNABORTED:		ret= "Software caused connection abort."; break;
		case WSAECONNRESET:			ret= "Connection reset by peer."; break;
		case WSAENOBUFS:			ret= "No buffer space available."; break;
		case WSAEISCONN:			ret= "Socket is already connected."; break;
		case WSAENOTCONN:			ret= "Socket is not connected."; break;
		case WSAESHUTDOWN:			ret= "Cannot send after socket shutdown."; break;
		case WSAETIMEDOUT:			ret= "Connection timed out."; break;
		case WSAECONNREFUSED:		ret= "Connection refused."; break;
		case WSAEHOSTDOWN:			ret= "Host is down."; break;
		case WSAEHOSTUNREACH:		ret= "No route to host."; break;
		case WSAEPROCLIM:			ret= "Too many process."; break;
		case WSASYSNOTREADY:		ret= "Network subsystem is unavailable."; break;
		case WSAVERNOTSUPPORTED:	ret= "Winsock.dll version out of range."; break;
		case WSANOTINITIALISED:		ret= "Successful WSAStartup not yet performed."; break;
		case WSAEDISCON:			ret= "Graceful shutdown in progress."; break;
		//case WSATYPE_NOT_FOUND:		ret= "Class type not found."; break;
		case WSAHOST_NOT_FOUND:		ret= "Host not found."; break;
		case WSATRY_AGAIN:			ret= "Nonauthoritative host not found."; break;
		case WSANO_RECOVERY:		ret= "This is a nonrecoverable error."; break;
		case WSANO_DATA:			ret= "Valid name, no data record of requested type."; break;
		//case WSA_INVALID_HANDLE:	ret= "Specified event object handle is invalid."; break;
		//case WSA_INVALID_PARAMETER:	ret= "One or more parameters are invalid."; break;
		//case WSA_IO_INCOMPLETE:		ret= "Overlapped I/O event object not in signaled state."; break;
		//case WSA_IO_PENDING:		ret= "Overlapped operations will complete later."; break;
		//case WSA_NOT_ENOUGH_MEMORY:	ret= "Insufficient memory available."; break;
		//case WSA_OPERATION_ABORTED:	ret= "Overlapped operation aborted."; break;
		//case WSAINVALIDPROCTABLE:	ret= "Invalid procedure table from service provider."; break;
		//case WSAINVALIDPROVIDER:	ret= "Invalid service provider version number."; break;
		//case WSAPROVIDERFAILEDINIT:	ret= "Unable to initialize a service provider."; break;
		//case WSASYSCALLFAILURE:		ret= "System call failure."; break;
		default:
			ret= "unknown error.";
		}

		err_code= 0;
	}
#endif

	return ret;
}

int Socket::waitIncomingData(vector<Socket*> &slist, uint timeout)
{
	struct timeval tv;
	int ret;
	uint i;
	fd_set set;
	FD_ZERO(&set);
	for(i= 0; i< slist.size(); i++)
	{
		FD_SET(slist[i]->sock, &set);
	}

	tv.tv_sec= timeout;
	tv.tv_usec= 0;

	ret= select(FD_SETSIZE, &set, NULL, NULL, &tv);
	if( ret > 0 )
	{
		vector<Socket*> slist_orig= slist;
		slist.clear();

		for(i= 0; i< slist_orig.size(); i++)
		{
			if( FD_ISSET(slist_orig[i]->sock, &set) )
			{
				slist.push_back(slist_orig[i]);
			}
		}
	}

	return ret;
}

#if defined(WIN32)
#define PTYPE	const char*
#else
#define PTYPE	void*
#endif

int Socket::setOption(socket_option opt, int value)
{
	struct timeval tv= {0,0};
	struct linger s_lin= {0,0};
	int ret= -1;

	switch( opt )
	{
	case SOPT_KEEPALIVE:
		ret= setsockopt(sock, IPPROTO_TCP, SO_KEEPALIVE, (PTYPE) &value, sizeof(value));
		break;

	case SOPT_LINGER:
		if( value > 0 )
		{
			s_lin.l_onoff= 1;
			s_lin.l_linger= value;
		}

		ret= setsockopt(sock, SOL_SOCKET, SO_LINGER, (PTYPE)&s_lin, sizeof(s_lin));
		break;

/*
struct timeval {
        long    tv_sec;         // seconds
        long    tv_usec;        // and microseconds
};
*/
	case SOPT_SENDTIMEOUT:
		tv.tv_sec= value;
		ret= setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (PTYPE)&tv, sizeof(tv));
		break;

	case SOPT_RCVTIMEOUT:
		timeout.tv_sec= value;
		ret= setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (PTYPE)&timeout, sizeof(tv));
		break;

	case SOPT_REUSE:
		//ret= setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (PTYPE)&value, sizeof(value));
		ret= setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (PTYPE)&value, sizeof(value));
		break;

	default:
		break;
	}

	if( ret == -1 )
		setError();

	return 0;
}

#undef PTYPE

// ************
// TCP
// ************
TCPSocket::TCPSocket()
{
	initSocket();
}

void TCPSocket::initSocket()
{
	// create socket
	sock= socket(PF_INET, SOCK_STREAM, 0);
	if( sock == -1 )
		setError();	
}

TCPSocket::TCPSocket(int s)
{
	sock= s;
	if( sock == -1 )
		setError();
}

int TCPSocket::sendText(const char *format, ...)
{
	char buff[300];	
	va_list va;

	va_start(va, format);
	vsnprintf(buff, sizeof(buff)-1, format, va);
	va_end(va);

	return sendData(buff, strlen(buff)+1);
}

int TCPSocket::sendData(const char *buff, uint len)
{
	int ret= ::send(sock, buff, len, 0);
	if( ret == -1 )
		setError();

	return ret;
}

int TCPSocket::recvDataDirect(char *buff, uint maxlen)
{
	int ret= ::recv(sock, buff, maxlen, 0);
	if( ret == 0 )
		connected= false;

	return ret;
}

int TCPSocket::recvData(char *buff, uint maxlen)
{
	int ret;
	fd_set set;
	FD_ZERO(&set);
	FD_SET(sock, &set);
	ret= select(FD_SETSIZE, &set, NULL, NULL, &timeout);
	if( ret == 1 )
	{
		ret= recvDataDirect(buff, maxlen);
		if( ret == -1 )
			setError();
	}
	else if( ret == 0 )
	{
		ret= -1;
		err_str= "timeout";
		connected= false;
	}

	return ret;
}

//
// TCPClientSocket
//

/*
 0: OK
-1: ip invalid or unable to resolve hostname
*/
int TCPClientSocket::connect(const char *host, uint port)
{
	int ret= 0;
	
	initSocket();
	
	// first, look up hostname
	struct hostent *hostent= gethostbyname(host);
	if( hostent != NULL )
	{

		if( hostent->h_addrtype == AF_INET )
		{
			memcpy(&sin.sin_addr.s_addr, hostent->h_addr, hostent->h_length);
			sin.sin_port= htons(port);

			ret= ::connect(sock, (struct sockaddr*)&sin, sizeof(struct sockaddr));
		}
		else
		{
			ret= -1;
		}
	}
	else
	{
		ret= -1;
	}

	if( ret == -1 )
		setError();
	else
		connected= true;

	return ret;
}


//
// TCPServerSocket
//
int TCPServerSocket::setup(uint port, int queue)
{
	int ret= -1;
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));

	sin.sin_family= AF_INET;
	sin.sin_addr.s_addr= INADDR_ANY;
	sin.sin_port= htons(port);

	if( (ret=::bind(sock, (struct sockaddr*)&sin, sizeof(sin))) != -1 )
	{
		ret= listen(sock, queue);
	}

	if( ret == -1 )
		setError();

	return ret;
}

TCPSocket *TCPServerSocket::waitConnection()
{
	TCPSocket *tcp_s= NULL;

	int s= accept(sock, NULL, 0);
	if( s != -1 )
	{
		tcp_s= new TCPSocket(s);
	}
	else
		setError();

	return tcp_s;
}


// ************
// UDP
// ************
UDPSocket::UDPSocket()
{
	// create socket
	sock= socket(PF_INET, SOCK_DGRAM, 0);
	if( sock == -1 )
		setError();
}

int UDPSocket::sendText(const char *dest, uint port, const char *format, ...)
{
	char buff[300];
	va_list va;

	va_start(va, format);
	vsnprintf(buff, sizeof(buff)-1, format, va);
	va_end(va);

	return sendData(dest, port, buff, strlen(buff)+1);
}

int UDPSocket::sendData(const char *dest, uint port, char *buff, uint len)
{
	int ret= 0;
	// first, look up hostname
	struct hostent *hostent= gethostbyname(dest);
	if( hostent != NULL )
	{
		if( hostent->h_addrtype == AF_INET )
		{
			memcpy(&sin.sin_addr.s_addr, hostent->h_addr, hostent->h_length);
			sin.sin_port= htons(port);

			ret= ::sendto(sock, buff, len, 0, (struct sockaddr*)&sin, sizeof(sin));
		}
		else
		{
			ret= -1;
		}
	}
	else
	{
		ret= -1;
	}

	if( ret == -1 )
		setError();

	return ret;
}

//
// UDPClientSocket
//


//
// UDPServerSocket
//
int UDPServerSocket::setup(const char *from, uint port)
{
	int ret= 0;
	// first, look up hostname
	struct hostent *hostent= gethostbyname(from);
	if( hostent != NULL )
	{
		if( hostent->h_addrtype == AF_INET )
		{
			memcpy(&sin.sin_addr.s_addr, hostent->h_addr, hostent->h_length);
			sin.sin_port= htons(port);

			ret=::bind(sock, (struct sockaddr*)&sin, sizeof(sin));
		}
		else
		{
			ret= -1;
		}
	}
	else
	{
		ret= -1;
	}

	return ret;
}

int UDPServerSocket::recvData(char *buff, uint maxlen)
{
	int ret;
	socklen_t sz= sizeof(sin);
	fd_set set;
	FD_ZERO(&set);
	FD_SET(sock, &set);
	ret= select(FD_SETSIZE, &set, NULL, NULL, &timeout);
	if( ret == 1 )
	{
		ret= ::recvfrom(sock, buff, maxlen, 0, (struct sockaddr*)&sin, &sz);
		if( ret == -1 )
			setError();
	}
	else if( ret == 0 )
	{
		ret= -1;
		err_str= "timeout";
	}

	return ret;
}

//////////////

#if defined(TEST)

#if !defined(WIN32)

#include <pthread.h>
pthread_t th;
#define TH_RET void*
#define NEWTHREAD(F,P) pthread_create(&th, NULL, F, (void*)P);
#define WAITTHREAD() pthread_join(th, NULL);

#else
HANDLE th;
DWORD tid;
#define TH_RET DWORD WINAPI
#define NEWTHREAD(F,P) th= CreateThread(NULL, 0, F, (void*)P, 0, &tid);
#define WAITTHREAD() WaitForSingleObject(th, INFINITE);

#endif

#define SEND_TEXT "salut client gh fh fhm fhfgh "
#define SEND_TEXT2 "salut client"
#define SEND_TEXT3 "jfk dfdf d fdf fffff 123"


// test 1: TCP timeout
void test_1()
{
	char buff[200];
	int err;
	printf("1) TCP Timeout: ");
	fflush(stdout);

	TCPClientSocket cli;
	if( (err=cli.setOption(SOPT_RCVTIMEOUT, 2)) != -1 )
	{
		if( (err=cli.connect("www.free.fr", 80)) != -1 )
		{
			err= cli.recvData(buff, sizeof(buff)-1);
		}
	}

	if( err == -1 )
		printf("%s\n", cli.getError());
}

// test 4: UDP timeout
void test_2()
{
	char buff[200];
	int err;
	printf("2) UDP Timeout: ");
	fflush(stdout);

	UDPServerSocket srv;
	if( (err=srv.setOption(SOPT_RCVTIMEOUT, 2)) != -1 )
	{
		if( (err= srv.setup("0.0.0.0", 0)) != -1 )
		{
			err= srv.recvData(buff, sizeof(buff)-1);
		}
	}

	if( err == -1 )
		printf("%s\n", srv.getError());
}

// test 3: client / server TCP
TH_RET test_3(void *v)
{
	int n= (int)v;

	if( n == 0)
	{
		TCPServerSocket srv;
		srv.setOption(SOPT_REUSE, 1);
		srv.setup(25000, 1);

		TCPSocket *c= srv.waitConnection();
		if( c != NULL )
		{
			c->sendText("%s", SEND_TEXT);
			delete c;
		}
		else
		{
			printf("waitConnection: %s\n", srv.getError());
		}
	}
	// parent
	else
	{
		printf("3) client/server TCP: ");
		TCPClientSocket cli;
		fflush(stdout);

		usleep(1000);

		if( cli.connect("127.0.0.1", 25000) != -1 )
		{
			char buff[200];
			if( cli.setOption(SOPT_RCVTIMEOUT, 2) != -1)
			{
				int n= cli.recvData(buff, sizeof(buff)-1);
				if( n != -1 )
				{
					if( !strcmp(buff, SEND_TEXT) )
						printf("OK\n", buff);
				}
			}
			else
			{
				printf("setOption: %s\n", cli.getError());
			}
		}
		else
		{
			printf("connect: %s\n", cli.getError());
		}
	}

	return 0;
}

TH_RET test_4(void *v)
{
	int err= 0;
	char buff[200];
	int n= (int)v;

	if( n == 1)
	{
		usleep(1000);
		UDPClientSocket cli;
		sleep(1);
		cli.sendText("127.0.0.1", 25001, "%s", SEND_TEXT);
		cli.sendText("127.0.0.1", 25001, "%s", SEND_TEXT2);
	}
	// parent
	else
	{
		printf("4) client/server UDP: ");
		UDPServerSocket srv;
		if( (err=srv.setOption(SOPT_RCVTIMEOUT, 5)) != -1 )
		{
			if( (err=srv.setup("127.0.0.1", 25001)) != -1 )
			{
				if( (err=srv.recvData(buff, sizeof(buff)-1)) != -1 )
				{
					if( !strcmp(buff, SEND_TEXT) )
					{
						if( (err=srv.recvData(buff, sizeof(buff)-1)) != -1 )
						{
							if( !strcmp(buff, SEND_TEXT2) )
								printf("OK\n", buff);
						}
					}
				}
			}
		}

		if( err == -1 )
		{
			printf("recvData: %s\n", srv.getError());
		}
	}

	return 0;
}

// test 5: one server, multiple client (TCP)
TH_RET test_5(void *v)
{
	int err= 0;
	int n= (int)v;

	if( n == 0 )
	{
		printf("5) multiple clients (TCP):\n");
		TCPServerSocket srv;
		srv.setOption(SOPT_REUSE, 1);
		srv.setOption(SOPT_RCVTIMEOUT, 5);

		if( (err=srv.setup(25002,1)) != -1 )
		{
			TCPSocket *cl_s[2];
			cl_s[0]= srv.waitConnection();
			if( cl_s[0] != NULL )
			{
				cl_s[1]= srv.waitConnection();
				if( cl_s[1] != NULL )
				{
					printf("2 clients connected\n");

					for( int k= 0; k< 4; k++ )
					{
						vector<Socket*> sl;
						sl.push_back(cl_s[0]);
						sl.push_back(cl_s[1]);

						if( (err= Socket::waitIncomingData(sl, 5)) > 0 )
						{
							for(int i= 0; i< sl.size(); i++)
							{
								char buff[200]= "";
								TCPServerSocket *s= (TCPServerSocket*)sl[i];
								if( s == cl_s[0] )
								{
									if( (err=cl_s[0]->recvDataDirect(buff, sizeof(buff)-1)) != -1 )
										printf("data on socket 0: %s\n", buff);
									else
										printf("err: %s\n", cl_s[0]->getError());
								}
								else if( s == cl_s[1] )
								{
									if( (err=cl_s[1]->recvDataDirect(buff, sizeof(buff)-1)) != -1 )
										printf("data on socket 1: %s\n", buff);
									else
										printf("err: %s\n", cl_s[1]->getError());
								}
							}
						}
						else if( err == 0 )
						{
							printf("timeout\n");
						}
					}

					delete cl_s[1];
				}
				else
					printf("err: %s\n", srv.getError());

				delete cl_s[0];
			}
			else
				printf("err: %s\n", srv.getError());
		}

		if( err == -1 )
			printf("err: %s\n", strerror(err));
	}
	else
	{
		TCPClientSocket cli[2];

		sleep(1);
		cli[0].connect("127.0.0.1", 25002);
		cli[1].connect("127.0.0.1", 25002);

		cli[0].sendText("%s", SEND_TEXT);
		usleep(100000);
		cli[1].sendText("%s", SEND_TEXT2);
		usleep(20000);
		cli[0].sendText("%s", SEND_TEXT3);


		sleep(10);
	}

	return 0;
}

int main()
{


	test_1();
	test_2();

	NEWTHREAD(test_3, 0);
	test_3((void*)1);
	WAITTHREAD();

	NEWTHREAD(test_4, 0);
	test_4((void*)1);
	WAITTHREAD();

	NEWTHREAD(test_5, 0);
	test_5((void*)1);
	WAITTHREAD();


	return 0;
}

#endif

/**/


