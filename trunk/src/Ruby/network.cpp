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
#include "../thread.h"
#include "network.h"

RubyTCPClientSocket::RubyTCPClientSocket() : th(this)
{
	data_rcv_block= 0;
	error_block= 0;

	setOption(SOPT_RCVTIMEOUT, 5);
}

void MonitorThread::user_func()
{
	int err= 0;
	char buff[201];
	VALUE msg;
	RubyTCPClientSocket *sock= (RubyTCPClientSocket*)param;

	while( !err )
	{
		cancel_point();

		err= sock->recvData(buff, 200);
		if( err != -1 )
		{
			//sock->onDataReceived(buff);
			if( sock->data_rcv_block != 0 )
			{
				msg= rb_str_new2(buff);
				rb_funcall(sock->data_rcv_block, rb_intern("call"), 1, msg);
			}
		}
	}

	if( sock->error_block != 0 )
	{
		msg= rb_str_new2( sock->getError() );
		rb_funcall(sock->error_block, rb_intern("call"), 1, msg);
	}
}

// events (should be redefined in script)
void RubyTCPClientSocket::onDataReceived()
{
	if( rb_block_given_p() )
	    data_rcv_block= rb_block_proc();
}

void RubyTCPClientSocket::onError()
{
	if( rb_block_given_p() )
		error_block= rb_block_proc();
}


// functions
int RubyTCPClientSocket::connect(const char *host, uint port)
{
	int ret= TCPClientSocket::connect(host, port);
	th.run();
	return ret;
}

int RubyTCPClientSocket::sendText(const char *txt)
{
	return TCPSocket::sendText("%s", txt);
}


/**/

