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

/*
  static int flags[] = {
//  MODE_CHANOP,		'o',
//  MODE_VOICE,			'v',
    MODE_PRIVATE,		'p',
    MODE_SECRET,		's',
    MODE_MODERATED,		'm',
    MODE_TOPICLIMIT,	't',
    MODE_INVITEONLY,	'i',
    MODE_NOPRIVMSGS,	'n',
    MODE_KEY,			'k',
//  MODE_BAN,			'b',
    MODE_LIMIT,			'l',
    MODE_REGONLY,		'r',
    MODE_NOCOLOUR,      'c',
    MODE_NOCTCP,        'C',
    MODE_DELJOINS,      'D',
    MODE_NOQUITPARTS,   'u',
    0x0, 0x0
  };
*/

#include "config.h"
#if !defined(WIN32)
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sched.h>
#endif

#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

#include <string>
#include <iostream>
#include <algorithm>

#include "basebot.h"
#include "log.h"
#include "utils.h"
#include "match.h"
#include "tokens.h"
#include "sockets.h"

using namespace std;

/*!
\file basebot.cpp
*/

const int SendThread::SENDSIG_WAITDATA=	1;
const int SendThread::SENDSIG_FLOOD=	2;

const int ReadThread::READSIG_WAITMSG=	1;
const int ReadThread::READSIG_WAITCON=	2;

/*!
\brief Constructor: initialise all members
*/
BaseBot::BaseBot() : quit_sent(false), irc_end(false)
{
	support_USERIP= 	false;
	support_CPRIVMSG= 	false;
	support_CNOTICE= 	false;
	support_WHOX=		false;

	use_AUTH=			true;

	SendT= new SendThread(this);
	ReadT= new ReadThread(this);

	ReadT->run();
	SendT->run();
}

/*!
\brief disconnect from IRC server
*/
BaseBot::~BaseBot()
{
	irc_end= true;
	ReadT->requestEnd();
	ReadT->waitEnd();
	SendT->requestEnd();
	SendT->waitEnd();
	delete ReadT;
	delete SendT;	// call the SendThread destructor BEFORE disconnection
	disconnect();
}

/////////////////////////////////////////////////////////////
// Data Reception

ReadThread::ReadThread(void *v) : PThread(v), connected(true), waiting(false)
{
	//Debug("ReadThread::ReadThread(%d)\n", myid);
}

ReadThread::~ReadThread()
{
	//Debug("ReadThread::~ReadThread(%d)\n", myid);
}

/********************************************************************/
// PRIVMSG #chan text
// :euroserv.fr.quakenet.org 372 eveR`BoT :- ...   the actions taken by
// :<nickname>!<ident>@<hostname> <COMMAND> <param(s)> :<last_param>
void ReadThread::parseMessage(const string &msg, IrcMessage &m)
{
	// extract prefix from message if present
	size_t msgstart= 0;
	m.senderNick= "@Server";
	m.have_prefix= false;
	m.param.clear();

	// there is a prefix
	if( msg[0] == ':' )
	{
		m.have_prefix= true;
		uint pos= msg.find(" ");
		msgstart= pos+1;
		string prefix= msg.substr(1, pos - 1);
		m.prefix= prefix;
		if( (pos= prefix.find("!"))!=string::npos )
			m.senderNick= prefix.substr(0, pos);
	}

	// extract irc command
	size_t paramstart= msg.find(" ", msgstart) + 1;
	m.command= msg.substr(msgstart, paramstart - 1 - msgstart);

	while(1)
	{
		// first check for ':' param prefix
		// this means that this is the last parameter
		if((msg[paramstart] == ':'))
		{
		    m.param.push_back(msg.substr(paramstart+1, msg.length()-1-paramstart));
		    break;
		}
		// try to localise next parameter
		// if not present ... we know this is the last
		size_t next= next= msg.find(" ", paramstart);
		if( next == string::npos )
		{
	        m.param.push_back(msg.substr(paramstart, msg.length() - paramstart));
	        break;
		}
		// there are more than this parameter so
		// set paramstart to the next parameter
		m.param.push_back(msg.substr(paramstart, next-paramstart));
		paramstart= next+1;
	}
}

/*!
\brief read a full line of data from socket s
\param s the socket
\param msg where the read line will be stored
\param ctcp is it a ctcp socket ?
\remarks ctcp sockets use LF as line separator while main socket uses CRLF
\return 1 if a complete line is in the buffer
\return -1 if an error occured
*/
int ReadThread::readMessage(TCPClientSocket &s, string &msg, bool ctcp)
{
	int ret= 0;
	char c;

	msg= "";

	while( (ret= s.recvData(&c, 1)) > 0 )
	{
		msg+= c;

		// test if line is complete
		if( msg.size() > 1 )
		{
			if( ctcp )
			{
				if( msg[msg.size()-1] == '\n' )
				{
					msg= msg.substr(0, msg.size()-1);
					break;
				}
			}
			else
			{
				if( msg.substr(msg.size()-2, 2) == CRLF )
				{
					msg= msg.substr(0, msg.size()-2);
					break;
				}
			}
		}
	}

	if( ret <= 0 )
		return -1;

	BaseBot *b= (BaseBot*)param;

	// show received messages before processing
	if( (b->debugMode & 0x01) != 0 )
		Debug("[>> BOT][(%d) \"%s\"]\n", ret, msg.c_str());

	return 1;
}

void ReadThread::user_func()
{
	BaseBot *b= (BaseBot*)param;
	IrcMessage s_msg;
	string msg;
	uint i;

	while(1)
	{
		cancel_point();
		sched_yield();


		TCPClientSocket &main_cliSocket= b->cliSocket;
		// if disconnected, wait reconnection
		if( !main_cliSocket.isConnected() )
		{
			waitSignal(READSIG_WAITCON);
			if( main_cliSocket.isConnected() )
				connected= true;

			continue;
		}

		vector<user_Info> clients_infos;
		BaseBot::userDataTypeIter user_it;

		// loop through all the users to check if they have an opened DCC connection
		for(user_it= b->userData.begin(); user_it!=b->userData.end(); user_it++)
		{
			user_Info &usr= user_it->second;

			// does this user has a dcc socket opened ?
			if( usr.ctcpSocket.isConnected() )
			{
				Debug("CTCP: %s\n", usr.nick.c_str());
				clients_infos.push_back( usr );
			}
		}

		// DCC sockets count
		uint users_count= clients_infos.size();

		vector<Socket*> set;
		set.push_back(&main_cliSocket);
		for(i= 0; i< users_count; i++)
		{
			set.push_back( &clients_infos[i].ctcpSocket );
		}


		// wait until data are available on sockets, or timeout
		// timeout is set on Qnet pings interval (3 min)
		int ret= Socket::waitIncomingData(set, 3*60);
		switch(ret)
		{
		case -1: Error("[ReadT] select: %s\n", strerror(errno));
		case  0: Error("[ReadT] disconnected\n");
			connected= false;
			// if we are waiting, wake up
			if( waiting )
				sendSignal(READSIG_WAITMSG);

			continue;
		}

		//printf("[READ] Data arrived.\n");

		if( b->quit_sent )
		{
			MUTEX_GET(b->mutex_end);
			{
				b->irc_end= true;
			}
			MUTEX_RELEASE(b->mutex_end);
		}


		for( i= 0; i< set.size(); i++ )
		{
// data available on main socket (main socket is high priority)
			if( set[i] == &main_cliSocket )
			{
				ret= readMessage(main_cliSocket, msg, false);
				if( ret==-1 )
				{
					connected= false;

					// if we are waiting, wake up
					if( waiting )
						sendSignal(READSIG_WAITMSG);

					continue;
				}
			}
// data available on one or more client socket (DCC)
// dcc query are treated as private message in current implementation
			else
			{
				for(i= 0; i< users_count; i++)
				{
					user_Info &usr= clients_infos[i];
					// data awaiting on a client socket
					if( set[i] == &usr.ctcpSocket )
					{
						ret= readMessage(usr.ctcpSocket, msg, true);
						if( ret<=0 )
						{
							Debug("++ [%s] Socket closed\n", usr.nick.c_str());
							//b->userData[usr.nick].ctcp_sock= -1;
							continue;
						}

						Debug("++ [%s] %s\n", usr.nick.c_str(), msg.c_str());
						s_msg.have_prefix= true;
						s_msg.senderNick= usr.nick;
						s_msg.command= "PRIVMSG";
						s_msg.param.clear();
						s_msg.param.push_back(b->getNick());
						s_msg.param.push_back(msg);

						// add the message in queue and then wake up main program
						msg_list.push_back(s_msg);
						sendSignal(READSIG_WAITMSG);
					}
				}
				continue;
			}
		}

		if( msg.empty() )
			continue;

		parseMessage(msg, s_msg);

		// flood control system
		if( (s_msg.command=="NOTICE") && (s_msg.param[1]=="B_FULL") )
		{
			b->SendT->sendSignal( SendThread::SENDSIG_FLOOD );
			continue;
		}

		if( s_msg.command=="433" )
		{
			Debug("nickname already in use: '%s' trying alternate...\n", s_msg.param[1].c_str());
		}
		else if( (s_msg.command[0]=='4') && (s_msg.command!="422") )
		{
			// invalid username
			if( s_msg.command == "468" )
			{
				Error("ERROR FROM SERVER: Invalid username \"%s\"\n", s_msg.param[0].c_str());
			}
			else if( s_msg.command == "482" )
			{
				Error("ERROR FROM SERVER: Need OP flag on \"%s\" : %s\n", s_msg.param[0].c_str(), s_msg.param[1].c_str() );
			}
			else if( s_msg.command=="438" )
			{
				Error("ERROR FROM SERVER: %s\n", s_msg.param[2].c_str());
			}
			else if( s_msg.command=="401" )
			{
				Error("ERROR FROM SERVER: no such nick \"%s\"\n", s_msg.param[1].c_str());
			}
			else if( s_msg.command=="403" )
			{
				Error("ERROR FROM SERVER: no such channel '%s'\n", s_msg.param[1].c_str());
			}
			else if( s_msg.command=="474" )
			{
				Error("ERROR FROM SERVER: banned from chan \"%s\"\n", s_msg.param[0].c_str());
			}
			else if( s_msg.command=="475" )
			{
				Error("ERROR FROM SERVER: invalid key for channel \"%s\"\n", s_msg.param[0].c_str());
			}
			else
			{
				string s= s_msg.command;
				for(uint i= 0; i< s_msg.param.size(); i++)
				{
					s+= " ";
					s+= s_msg.param[i];
				}
				Error("ERROR FROM SERVER: '%s' !\n", s.c_str());
			}
			continue;
		}

		if( s_msg.command=="ERROR" )
		{
			if( b->quit_sent )
			{
				continue;
			}
			else
			{
				Error("%s : %s\n", s_msg.command.c_str(), s_msg.param[0].c_str());
				connected= false;
				sendSignal(READSIG_WAITMSG);
				continue;
			}
		}


		// add the message in queue and then wake up main program
		MUTEX_GET(rdqueue_mutex);
		{
			msg_list.push_back(s_msg);
		}
		MUTEX_RELEASE(rdqueue_mutex);

		sendSignal(READSIG_WAITMSG);
	}
}

int ReadThread::waitMessage(IrcMessage &m)
{
	bool list_empty= false;
	bool ret= false;

	// look in the queue
	MUTEX_GET(rdqueue_mutex);
	{
		for(list<IrcMessage>::iterator it= msg_list.begin(); it!=msg_list.end(); it++)
		{
			m= *it;
			msg_list.erase(it);
			ret= true;
			break;
		}
	}
	MUTEX_RELEASE(rdqueue_mutex);

	if( ret )
		return 0;

	waiting= true;

	while( 1 )	// wait for the messages
	{
		waitSignal(READSIG_WAITMSG);

		MUTEX_GET(rdqueue_mutex);
		{
			list_empty= msg_list.empty();
		}
		MUTEX_RELEASE(rdqueue_mutex);

		if( list_empty || !connected )
			return -1;

		MUTEX_GET(rdqueue_mutex);
		{
			m= msg_list.back();
			msg_list.pop_back();
		}
		MUTEX_RELEASE(rdqueue_mutex);

		waiting= false;
		return 0;
	}
}

/////////////////////////////////////////////////////////////
// Data Emission

//! server queue size
#define SEND_SIZE 1024	// server queue have great chance to be equal to 1024 bytes
						// for Qnet at least

SendThread::SendThread(void *v) : PThread(v)
{
	floodcounter= 0;
	//Debug("[SEND THREAD][%d] Started\n", myid);
}

SendThread::~SendThread()
{
	//Debug("SendThread::~SendThread(%d)\n", myid);
}

/*!
\brief function called by send*Message

this function adds the data in the send buffer and wake up the send thread.
\param msg the message to send
\note this function is called after the final message is built (includes ':')
*/
void SendThread::addCommandInQueue(const string &msg)
{
	IrcMessage s_msg;
	string msg2(msg);

	msg2+= "\r\n";

	MUTEX_GET(queue_mutex);
	send_list.push(msg2);
	MUTEX_RELEASE(queue_mutex);

	sendSignal( SENDSIG_WAITDATA );
}

void SendThread::resetBuffer()
{
	floodcounter= 0;

	MUTEX_GET(queue_mutex);

	while( send_list.size() > 0 ) send_list.pop();

	MUTEX_RELEASE(queue_mutex);
}

void SendThread::user_func()
{
	BaseBot *b= (BaseBot*)param;
	bool sendlist_empty;

	while(1)
	{
		TCPClientSocket &s= b->cliSocket;
		sched_yield();

		MUTEX_GET(queue_mutex);
		sendlist_empty= send_list.empty();
		MUTEX_RELEASE(queue_mutex);

		if( sendlist_empty )
		{
			waitSignal( SENDSIG_WAITDATA );
			cancel_point();
			continue;
		}

		// not connected
		if( !s.isConnected() )
		{
			cancel_point();
			continue;
		}

		MUTEX_GET(queue_mutex);
		string buffer= send_list.front();
		MUTEX_RELEASE(queue_mutex);

		uint len= buffer.size();


		// if we have sent too much data, wait until the server
		// can take much data to prevent flood disconnection

		char reset_str[100];
		int reset_len= snprintf(reset_str, 99, "NOTICE %s :B_FULL\r\n", b->getNick().c_str());

		if( floodcounter + len + reset_len >= SEND_SIZE )
		{
			//send(b->sock, reset_str, reset_len, 0);
			s.sendData(reset_str, reset_len);
			//Debug("[FLOOD CONTROL] Waiting reset...\n");
			// wait SENDSIG_FLOOD sent by processMessage
			// when reset_str is received by the server
			waitSignal( SENDSIG_FLOOD );
			cancel_point();
			floodcounter= 0;
		}

		// send current data
		int ret;
		if( (ret= s.sendData(buffer.c_str(), len)) == -1)
		{
			Error("[BOT >>] send error: '%s'\n", s.getError());
		}
		else
		{
			if( (b->debugMode & 0x02) != 0 )
				Debug("[BOT >>][%d][%.*s]\n", ret, len-2, buffer.c_str());
		}

		floodcounter+= len;

		MUTEX_GET(queue_mutex);
		send_list.pop();
		MUTEX_RELEASE(queue_mutex);
	}
}

/*!
\brief send data to a client via ctcp socket
\param s the client socket
\param format printf style format string
*/
void BaseBot::sendDirectMessage(TCPClientSocket &s, const char *format, ...)
{
	char msg[1024];

	va_list varg;

	va_start(varg, format);
	// keeps room for \n\0
	vsnprintf(msg, sizeof(msg)-2, format, varg);
	va_end(varg);

	strcat(msg, "\n");

	s.sendData(msg, strlen(msg)+1);
}


void BaseBot::sendMessage(send_type type, const char *format, ...)
{
	static char buff[2*1024];	// 2ko should be enough, i don't even except 1024 to be reached but...
	va_list arg;

	va_start(arg, format);
	// keep space for '\0'
	vsnprintf(buff, sizeof(buff)-1, format, arg);
	va_end(arg);

	if( type == SENDTYPE_NORMAL )
	{
		SendT->addCommandInQueue(buff);
	}
	else
	{
		vector<string> parts;
		spec_cmd c;

		Tokenize(buff, parts, " ");

		c.type= type;
		c.dest= parts[1];
		if( type == SENDTYPE_CHANMODE )
			c.modes= parts[2];
		c.cmdline= buff;

		for( uint i= 3; i< parts.size(); i++ )
		{
			c.other+= parts[i];
			c.other+= " ";
		}

		special_commands.push(c);
	}
}

/*!
\brief open connection to an IRC server
\param host hostname of server
\param port port number
\return the socket
*/
/*
int BaseBot::connect(const string &host, uint port)
{
	userData.clear();
	chanData.clear();

	sock= MakeConnection(host.c_str(), port);
	return sock;
}
*/

/*!
\brief close connection(s)

close the connection with server and if ctcp sockets are opened then close them
*/
void BaseBot::disconnect()
{
	cliSocket.disconnect();
	bot_data.nick= "";
}

/*!
\brief determine if the current line include special CTCP command
\param msg the message line
\param ctcp resulting line without the \\001 characters
*/
bool BaseBot::findCTCP(const string &msg, string &ctcp)
{
//	size_t start, end;

//	start= msg.find('\001');
//	if(start == string::npos)
	if( msg[0]!='\001' )
		return false;

//	end= msg.find('\001', start+1);
//	if(end == string::npos)
	if( msg[msg.size()-1]!='\001' )
		return false;

	//ctcp= msg.substr(start+1, end-start-1);
	ctcp= msg.substr(1, msg.size()-2);
	return true;
}

/********************************************************************/

void BaseBot::requestChanInfos(const string &chan)
{
	Debug("BaseBot::requestChanInfos(\"%s\")\n", chan.c_str());
	sendMessage(SENDTYPE_NORMAL, "MODE %s +b", chan.c_str());
	sendMessage(SENDTYPE_NORMAL, "MODE :%s", chan.c_str());
	sendMessage(SENDTYPE_NORMAL, "WHO :%s\n", chan.c_str());
}

static uint chanflags[]= {
 CHANMODE_PRIVATE,		'p',
 CHANMODE_SECRET,		's',
 CHANMODE_MODERATED,	'm',
 CHANMODE_OPTOPIC,		't',
 CHANMODE_INVITEONLY,	'i',
 CHANMODE_NOPRIVMSGS,	'n',
 CHANMODE_REGONLY,		'r',
 CHANMODE_NOCOLOUR,		'c',
 CHANMODE_NOCTCP,		'C',
 CHANMODE_DELJOINS,		'D',
 CHANMODE_NOQUITPARTS,	'u',
 0,0
};

uint getChanFlagMaskForChar(char c)
{
	uint j= 0;

	while( chanflags[j] != 0 )
	{
		if( c == (char)chanflags[j+1] )
			return chanflags[j];

		j+= 2;
	}

	return 0;
}

void setChanFlagsFromString(const string &str, uint &flagmask)
{
	for( uint i= 0; i< str.length(); i++)
	{
		uint j= 0;

		while( chanflags[j] != 0 )
		{
			if( str[i] == (char)chanflags[j+1] )
				flagmask |= chanflags[j];

			j+= 2;
		}
	}
}

/*!
\brief call the virtual functions
\param m the irc message
\return  1 irc message was not used
		 0 message used
*/
int BaseBot::processMessage(const IrcMessage &m)
{
	// PING
	if((m.command == "PING") && (m.param.size() == 1))
	{
		//static bool dbg_ping= true;

		//if( dbg_ping )
		//{
			sendMessage(SENDTYPE_NORMAL, "PONG :%s", m.param[0].c_str());
			onPing(m.param[0]);
		//}
		//dbg_ping= !dbg_ping;
		return 0;
	}

	// nickname in use
	if( m.command=="433" )
	{
		Debug("Trying alt nick(%s)...\n", alt_nick.c_str());
		changeNick(alt_nick);
		bot_data.nick= alt_nick;
	}

	// Welcome message, catch hostname of the server here
	// :b0rk.uk.quakenet.org 001 Tarzaaan :Welcome to the Internet Relay Network, Tarzaaan
	if( m.command=="001" )
	{
		Output("Server Hostname: %s\n", m.prefix.c_str());
		current_Hostname= m.prefix;
		//whois(getNick());
		sendMessage(SENDTYPE_NORMAL, "USERHOST %s", getNick().c_str());
		return 0;
	}

	// server properties
	if( m.command=="005" )
	{
		for(uint i= 1; i< m.param.size()-1; i++)
		{
			vector<string> parts;
			Tokenize(m.param[i], parts, "=");
			if( parts.size() == 1 )
			{
				if( parts[0]=="WALLCHOPS" )
				{

				}
				// Notice to +#channel goes to all voiced persons.
				else if( parts[0]=="WALLVOICES" )
				{

				}
				// USERIP cmd supported
				else if( parts[0]=="USERIP"  )
				{
					support_USERIP= true;
				}
				// CPRIVMSG supported
				else if( parts[0]=="CPRIVMSG" )
				{
					support_CPRIVMSG= true;
				}
				// CNOTICE supported
				else if( parts[0]=="CNOTICE" )
				{
					support_CNOTICE= true;
				}
				// WHO use WHOX format
				else if( parts[0]=="WHOX" )
				{
					support_WHOX= true;
				}
				else
				{
					Debug("Unsupportded 005 param: %s\n", m.param[i].c_str());
				}
			}
			else
			{
				// CHANTYPES=#&
				// valid prefix for channels
				if( parts[0]=="CHANTYPES" )
				{
					channel_prefix= parts[1];
				}
				// PREFIX=(ov)@+
				else if( parts[0]=="PREFIX" )
				{

				}
				// CHANMODES=b,k,l,imnpstrDcCNu
				// 1 = Mode that adds or removes a nick or address to a list. Always has a parameter.
				// 2 = Mode that changes a setting and always has a parameter.
				// 3 = Mode that changes a setting and only has a parameter when set.
				// 4 = Mode that changes a setting and never has a parameter.
				else if( parts[0]=="CHANMODES" )
				{

				}
				// MODES=6
				// number of modes allowed per MODE command
				else if( parts[0]=="MODES" )
				{
					max_modes= atoi(parts[1].c_str());
				}
				// MAXCHANNELS=20
				// max number of opened channels
				else if( parts[0]=="MAXCHANNELS" )
				{
					max_channels= atoi(parts[1].c_str());
				}
				// NICKLEN=15
				else if( (parts[0]=="NICKLEN") || (parts[0]=="MAXNICKLEN") )
				{
					max_nicklen= atoi(parts[1].c_str());
					Debug("Max nicklen: %d\n", max_nicklen);
				}
				// MAXBANS=45
				// limit of bans per channel
				else if( parts[0]=="MAXBANS" )
				{
					max_channel_bans= atoi(parts[1].c_str());
				}
				// NETWORK=UnderNet
				else if( parts[0]=="NETWORK" )
				{
					network_name= parts[1];
				}
				// CASEMAPPING=rfc1459
				// Case mapping used for nick- and channel name comparing:
				// ascii: The chars [a-z] are lowercase of [A-Z].
				// rfc1459: ascii with additional {}|~ the lowercase of []\^.
				else if( parts[0]=="CASEMAPPING" )
				{

				}
				// TOPICLEN=250
				else if( parts[0]=="TOPICLEN" )
				{
					max_topiclen= atoi(parts[1].c_str());
				}
				// KICKLEN=250
				// max kick message length
				else if( parts[0]=="KICKLEN" )
				{
					max_kickmsglen= atoi(parts[1].c_str());
				}
				// SILENCE=15
				// server support cmd: /silence [+/-]nick!name@host.host.dom (nick or host alone allowed)
				// max entries count in the ignore list
				else if( parts[0]=="SILENCE" )
				{

				}
				// AWAYLEN=160
				// max length for away message
				else if( parts[0]=="AWAYLEN" )
				{

				}
				else
				{
					Debug("Unsupportded 005 param: %s\n", m.param[i].c_str());
				}

			}
		}
	}

	if( (m.command == RPL_ENDOFMOTD) || (m.command == RPL_NOMOTD))
	{
		Output("Network: %s\n", network_name.c_str());
		onConnected();
		return 0;
	}

// all the messages after this point need a prefix
// if not present stop here
	if( !m.have_prefix )
		return 1;

	if( m.param[0]==getNick() )
	{

//
// channel related messages
//
		// 367 <bot_name> <channel> <banmask> <ban_by_who> <when_timestamp>
		if(  m.command=="367" )
		{
			chan_Info &chan= chanData[m.param[1]];
			chan.ban_list.push_back(m.param[2]);
		}

		// channel users list
		//353 <bot_name> = <channel> :a007 @aRena`eveR|Schm
		// + = voice
		// @ = op
		/*
			get the current op/voice status of all the users in the channel to avoid sending useless commands
			to server later.
		*/
		if( m.command=="353" )
		{
			const string &channel= m.param[2];
			vector<string> out;
			Tokenize(m.param[3], out, " ");

			chan_Info &chan= chanData[m.param[2]];
			chan.hidden= (m.param[1] != "=");


			// if we received this message, then we are on this channel
			//bot_data.ison.add(channel);

			for(uint i= 0; i< out.size(); i++)
			{
				char flag= ' ';
				user_Info *usr;

				if( (out[i][0]=='@') || (out[i][0]=='+') )
				{
					flag= out[i][0];
					out[i]= out[i].substr(1, out[i].size());
				}

				//updateUserData(out[i]);

				if( out[i]!=getNick() )
					usr= &userData[out[i]];
				else
					usr= &bot_data;

				usr->nick= out[i];

				// if op flag is set for current channel, remove
				usr->isop.remove(channel);

				// if voice flag is set for current chan, remove
				usr->isvoice.remove(channel);

				// if user is not currently flagged as being on this chan, do it
				usr->ison.add(channel);

				switch(flag)
				{
				case '@': usr->isop.add(channel); break;
				case '+': usr->isvoice.add(channel); break;
				}

				chan.userlist.push_back(out[i]);
			}

			return 0;
		}

		// topic message
		// 332 <bot_name> <channel> :<topic>
		if( m.command=="332" )
		{
			chanData[m.param[1]].topic= m.param[2];
			return 0;
		}

		// end of users listing for channel
		// we can assume here the bot joined the channel successfully
		// 366 <bot_name> <channel> :End of /NAMES list.
		if( m.command=="366" )
		{
			chan_Info &chan= chanData[m.param[1]];
			onBotJoined(m.param[1], chan.userlist);
			return 0;
		}

		// who set the topic and when
		// 333 <bot_name> <channel> <user> <timestamp>
		if( m.command=="333" )
		{
			chanData[m.param[1]].topic_setby= m.param[2];
			chanData[m.param[1]].topic_time= atoi(m.param[3].c_str());
			return 0;
		}

		// channel's modes
		// 324 <bot_name> <channel> +sCk 222
		if( m.command=="324" )
		{
			const string &fl= m.param[2];
			chan_Info &chan= chanData[m.param[1]];
			uint next= 3;

			chan.modes= 0;

			// start after the '+'
			for(uint i= 1; i< fl.length(); i++)
			{
				// channel key
				if( fl[i]=='k' )
				{
					if( m.param.size() >= next+1 )
						chan.key= m.param[next++];
					else
						chan.key= "";
				}
				// channel users limit
				else if( fl[i]=='l' )
				{
					next++;
				}
				else
				{
					chan.modes |= getChanFlagMaskForChar(fl[i]);
				}
			}

			chan.identification_state= IDENT_OK;
		}

//
// connection related
//

		// connection registered message
		// 396 Tarzaaan SchmurfysBot.users.quakenet.org :is now your hidden host
		if( m.command=="396" )
		{
			whois(getNick());
			onRegistered();
			return 0;
		}

//
// USERHOST message
//
/*
		//:b0rk.uk.quakenet.org 302 <nick> :o|RioS=+~RioS@Lapin.users.quakenet.org o|PhY=+~Phy@LoW`PhY.users.quakenet.org
		if( m.command=="302" )
		{
			vector<string> parts;
			Tokenize(m.param[1], parts, " ");
			// <nick>=[+-]<ident>@<host>
			for(uint i= 0; i< parts.size(); i++)
			{
				vector<string> tmp_parts;
				Tokenize(parts[i], tmp_parts, "=");

				string &nick= tmp_parts[0];
				string &host= tmp_parts[1];

				if( (host[0]=='+') || (host[0]=='-') )
					host.erase(0,1);

				if( nick == getNick() )
				{
					bot_data.full_host= nick + "!" + host;
				}
				else
				{
					user_Info &u= userData[nick];

					u.full_host= nick + "!" + host;
					u.identification_state= IDENT_OK;
				}
			}

			return 0;
		}
*/

//
// WHO related messages (sent when joining a channel)
//
		// 352 <bot_nick> <channel> <ident> <host> <server> <target_nick> <flags> :<hop> <infos>
		if( m.command == "352" )
		{
			//const string &channel= 	m.param[1];
			const string &ident= 		m.param[2];
			const string &host= 		m.param[3];
			const string &nick= 		m.param[5];

			if( nick == getNick() )
			{
				bot_data.full_host= nick + "!" + ident + "@" + host;
				bot_data.host= host;
				bot_data.ident= ident;
			}
			else
			{
				user_Info &u= userData[nick];

				u.full_host= nick + "!" + ident + "@" + host;
				u.host= host;
				u.ident= ident;
			}
			return 0;
		}

		// 315 <bot_nick> <channel> :End of /WHO list.
		if( m.command == "315" )
		{
			return 0;
		}


//
// WHOIS related messages
//
		{
			const string &nick= m.param[1];

			// ident, host infos
			// 311 <bot_name> <PSEUDO> <IDENT> <HOST> * :<REALNAME>
			if( m.command=="311" )
			{
				if( nick == getNick() )
				{
					bot_data.full_host= nick + "!" + m.param[2] + "@" + m.param[3];
				}
				else
				{
					userData[nick].ident= m.param[2];
					userData[nick].host= m.param[3];
					userData[nick].full_host= nick + "!" + m.param[2] + "@" + m.param[3];
				}
				return 0;
			}

			if( nick != getNick() )
			{
				// user is an IRC operator
				// 313 <bot_name> <PSEUDO> :<MSG>
				if( m.command=="313" )
				{
					userData[m.param[1]].irc_op= true;
					return 0;
				}

				// Q auth of user (Qnet)
				// also works on GameSurge
				// 330 <bot_name> <PSEUDO> <Q_AUTH> <TEXT>
				if( m.command=="330" )
				{
					userData[m.param[1]].auth= m.param[2];
					return 0;
				}

				// channel list
				// 319 <bot_name> <PSEUDO> :<CHANNELS>
				if( m.command=="319" )
				{
					user_Info &u= userData[m.param[1]];
					vector<string> parts;
					Tokenize(m.param[2], parts, " ");
					for(uint i= 0; i< parts.size(); i++)
					{
						string &ch= parts[i];

						// remove useless char
						if( (ch[0] == '@') || (ch[0] == '+') )
							ch.erase(0,1);

						// then add chan in user channel list
						u.ison.add(ch);
					}
				}

				// end of whois
				// 318 <bot_name> <PSEUDO> <TEXT>
				if( m.command=="318" )
				{
					user_Info &u= userData[m.param[1]];
					u.identification_state= IDENT_OK;
					return 0;
				}
			}
		}
	}

	// MODE
	// -o+b Blade^Out *!___@AToulouse-102-1-1-230.w81-49.abo.wanadoo.fr
	// :euroserv.fr.quakenet.org MODE #bidon2 +o o|Schmurfy[aw]
	// -o+v AMXBot AMXBot
	if( m.command=="MODE" )
	{
		uint i;
		//char type= ' '; // - or +
		string flags[20], params[20];
		uint fcount= 0;
		const string &chan= m.param[0];

		// param 0 is the user issuing the command
		// param 1 contains the modes flags
		const string &word= m.param[1];
		char tmp[3]= "/o";

		for(i= 0; i< word.length(); i++)
		{
			switch( word[i] )
			{
			case '-':
			case '+':
				tmp[0]= word[i];
				break;

			default:
				tmp[1]= word[i];
				flags[fcount]= string(tmp);
				fcount++;
			}
		}

		// params
		//for(i= 2; i< m.pcount; i++ )
		for(i= 2; i< m.param.size(); i++)
		params[i-2]= m.param[i];

		MUTEX_GET(nick_mutex);

		//onModeChange(m.senderNick, chan,flags, params, fcount);

		uint param_idx= 0;

		// check which flags changed
		for(i= 0; i< fcount; i++)
		{
			// user mode changes
			if( (flags[i][1]=='o') || (flags[i][1]=='v') )
			{
				string &tmpnick= params[param_idx];
				user_Info *usr;

				usr= getUserData(tmpnick);

				if( usr==NULL )
				{
					Debug("onModeChange: unknown user \"%s\" !\n", tmpnick.c_str());
					continue;
				}

				if( flags[i][0]=='+' )
				{
					if( flags[i][1]=='o' ) usr->isop.add(chan);
					if( flags[i][1]=='v' ) usr->isvoice.add(chan);
				}
				else
				{
					if( flags[i][1]=='o' ) usr->isop.remove(chan);
					if( flags[i][1]=='v' ) usr->isvoice.remove(chan);
				}

				// add flag
				if( flags[i][0]=='+' )
				{
					// user is now an operator
					if( flags[i][1]=='o' )
					{
						if( m.senderNick == "@Server" )
						{
							onServerOp(chan, tmpnick);
						}
						// bot just got op status
						else if( tmpnick == getNick() )
						{
							requestChanInfos(chan);
							onBotGainOp(chan, m.senderNick);
						}
						else
							onGainOp(chan, m.senderNick, tmpnick);
					}

					// user gain voice
					if( flags[i][1]=='v' )
					{
						if( m.senderNick == "@Server" )
							onServerVoice(chan, tmpnick);
						else if( tmpnick == getNick() )
							onBotGainVoice(chan, m.senderNick);
						else
							onGainVoice(chan, m.senderNick, tmpnick);
					}
				}
				// remove it
				else
				{
					// user lost operator status
					if( flags[i][1]=='o' )
					{
						if( m.senderNick == "@Server" )
							onServerRemoveOp(chan, tmpnick);
						else if( tmpnick == getNick() )
							onBotLostOp(chan, m.senderNick);
						else
							onLostOp(chan, m.senderNick, tmpnick);
					}

					// user lost voice
					if( flags[i][1]=='v' )
					{
						if( m.senderNick == "@Server" )
							onServerRemoveVoice(chan, tmpnick);
						else if( tmpnick == getNick() )
							onBotLostVoice(chan, m.senderNick);
						else
							onLostVoice(chan, m.senderNick, tmpnick);
					}
				}

				param_idx++;
			}
			// bans
			else if( flags[i][1]=='b' )
			{
				string &mask= params[param_idx];

				if( flags[i][0]=='+' )
				{
					if( wc_match(mask, bot_data.full_host) )
						onBotBanned(chan, m.senderNick, mask);
					else
						onBanned(chan, m.senderNick, mask);
				}
				else
				{
					if( wc_match(mask, bot_data.full_host) )
						onBotUnBanned(chan, m.senderNick, mask);
					else
						onUnBan(chan, m.senderNick, mask);
				}

				param_idx++;
			}
			// channel key changes
			else if( flags[i][1]=='k' )
			{
				string &tmpkey= params[param_idx];
				chan_Info *channel= getChanData(chan);
				if( channel )
				{
					channel->key= tmpkey;
				}

				if( flags[i][0]=='+' )
					onChanKeySet(chan, m.senderNick, tmpkey);
				else
					onChanKeyRemoved(chan, m.senderNick);

				param_idx++;
			}
			// should be a channel mode
			// MODE <channel> +n
			else
			{
				chan_Info *channel= getChanData(chan);
				if( channel )
				{
					onChanModeChanged(m.senderNick, chan, flags[i]);
				}
			}
		}

		MUTEX_RELEASE(nick_mutex);
		return 0;
	}



	//if( m.pcount==1 )
	if( m.param.size() == 1 )
	{
		// JOIN chan
		if( m.command == "JOIN" )
		{
			chan_Info &chan= chanData[m.param[0]];
			chan.name= m.param[0];
			chan.ban_list.clear();
			chan.userlist.clear();
			chan.key= "";
			chan.modes= 0;

			if( m.senderNick==getNick() )
			{
				bot_data.ison.add(m.param[0]);
				requestChanInfos(m.param[0]);
			}
			else
			{
				onJoin(m.senderNick, m.param[0]);
			}
			return 0;
		}

		// PART chan
		if( m.command == "PART" )
		{
			onPart(m.senderNick, m.param[0]);
			return 0;
		}

		// QUIT :msg
		if( m.command=="QUIT" )
		{
			onQuit(m.senderNick, m.param[0]);
			return 0;
		}


		// NICK :<newnick>
		if( m.command=="NICK" )
		{
			if( m.senderNick == getNick() )
			{
				bot_data.nick= m.param[0];
			}
			else
			{
				onNickChange(m.senderNick, m.param[0]);
			}
			return 0;
		}
	}

	// KICK <chan> <nick> :<why>
	if( (m.command=="KICK") && (m.param.size()==3) )
	{
		onKick(m.senderNick, m.param[0], m.param[1], m.param[2]);
		return 0;
	}

	// TOPIC <chan> :<new_topic>
	// TOPIC <chan> :
	if( (m.command=="TOPIC") && (m.param.size()==2) )
	{
		onTopicChange(m.senderNick, m.param[0], m.param[1]);
		return 0;
	}



	// PRIVMSG and CTCP
	if( (m.command=="PRIVMSG") && (m.param.size()==2) )
	{
		string ctcp;
		if( findCTCP(m.param[1], ctcp) )
		{
			vector<string> parts;
			Tokenize(ctcp, parts, " ");


			if( parts[0]=="VERSION" )
			{
				sendMsgTo(m.senderNick, MODE_NOTICE_ONLY, "\001VERSION AMXBot " VERSION "\001");
				onCTCPVersion(m.senderNick);
				return 0;
			}

			if( parts[0]=="DCC" )
			{
				// does it really exist ?Oo
				// it appears that mIRC doesn't send it :/
				if( parts[1]=="CLOSE" )
				{
					Debug("[%s] DCC session closed\n", m.senderNick.c_str());
					userData[m.senderNick].ctcpSocket.disconnect();
				}

				// chat session request
				// PRIVMSG a007 :\001DCC CHAT chat 1380167440 8888\001
				if( (parts[1]=="CHAT") && (parts[2]=="chat") )
				{
					unsigned long lip_addr= strtoul(parts[3].c_str(), NULL, 10);

					if( lip_addr != ULONG_MAX )
					{
						int port= atoi(parts[4].c_str());

						if( (port>0) && (port<65535) )
						{
							struct in_addr in;
							in.s_addr= htonl(lip_addr);
							const char *src_addr=  inet_ntoa(in);
							Debug("chat request from %s - %s:%d\n", m.senderNick.c_str(), src_addr, port);
							userData[m.senderNick].ctcpSocket.connect(src_addr, port);
						}
						else
						{
							Error("port invalid for DCC request: %d\n", parts[4].c_str());
						}
					}
					else
					{
						Error("address invalid: %d\n", parts[3].c_str());
					}

					return 0;
				}

				if( parts[1]=="SEND" )
				{
					Debug("%s is trying to send me file '%s'\n", m.senderNick.c_str(), parts[2].c_str());
					return 0;
				}
			}

			if( parts[0] == "ACTION" )
			{
				onCTCPAction( m.param[1].substr(8,m.param[1].size()-9), m.senderNick, m.param[0] );
				return 0;
			}

			return 0;
		}
		else
		{
			uint j;
			string msg= m.param[1];

			for(j= 0; j< msg.length(); j++)
			{
				for( uint i= 0; i< msg.length(); i++ )
				{
					if( msg[i] > 126 )
					{
						msg.erase(i,1);
						break;
					}
				}
			}

			uint not_space= 0;
			for(j= 0; j< msg.length(); j++)
			{
				if( msg[j]!=' ' )
					not_space++;
			}

			// normal PRIVMSG
			if( (msg.length() > 0) && (not_space!=0) )
			{
				onPrivMsg(m.senderNick, m.param[0], msg);
			}
			return 0;
		}
	}

	// tell the main loop message is still waiting to be used
	return 1;
}

//int BaseBot::mainLoop(const string &host, int port, const string &nick, const string &realname)
int BaseBot::mainLoop(const string &server, const string &nick)
{
	IrcMessage m;
	list<IrcMessage> AwaitingList;
	vector<string> parts;
	int port= 6667;
	const char *host= server.c_str();
	string password= "";

	Tokenize(server, parts, ":");
	// server:port
	if( parts.size()>=2 )
	{
		port= atoi(parts[1].c_str());
		host= parts[0].c_str();

		// server password
		if( parts.size() == 3 )
		{
			password= parts[2];
		}
	}

	SendT->resetBuffer();

	userData.clear();
	chanData.clear();

	if( cliSocket.connect(host, port)==-1 )
	{
		Output("Connection to \"%s:%d\" failed: %s\n", host, port, cliSocket.getError());
		return 0;
	}

	Output("Connected to %s:%d\n", host, port);
	if( use_AUTH )
		Debug("%s - Identification by AUTH enabled - %s\n", COLOR_MAGENTA, COLOR_RESET);

	ReadT->sendSignal( ReadThread::READSIG_WAITCON );
	sleep(1);

	if( (debugMode & 0x04) != 0 )
		DebugLog.enable();
	else
		DebugLog.disable();

	registerBot(password, nick, realname);
	// init bot data
	bot_data.nick= nick;
	bot_data.identification_state= IDENT_OK;
	bot_data.ison.clear();
	bot_data.isop.clear();
	bot_data.isvoice.clear();
	// parse and process all incoming messages

	AwaitingList.clear();

	while( ReadT->isConnected() )
	{
		if( ReadT->waitMessage(m) == -1)
		{
			bool end;

			MUTEX_GET(mutex_end);
			{
				end= irc_end;
			}
			MUTEX_RELEASE(mutex_end);

			if( end )
			{
				disconnect();
				return -1;
			}
			else
				continue;
		}

		// to identify someone by its auth we need to issue a WHOIS
		if( use_AUTH )
		{
			// messages from server and from myself are not concerned
			if( (m.senderNick!="@Server") && (m.prefix!=current_Hostname) && (m.senderNick!=getNick()) )
			{
				user_Info *u= getUserData(m.senderNick);
				// user is not identified yet
				if( !u || (u->identification_state != IDENT_OK) )
				{
					// just ignore QUIT message sent by unidentified users
					// since we wont have the time to identify them
					if( m.command != "QUIT" )
					{
						if( !u || (u->identification_state == IDENT_NOQUERY) )
						{
							Debug("requesting identification of %s (WHOIS)\n", m.senderNick.c_str());
							whois(m.senderNick);
							userData[m.senderNick].identification_state= IDENT_QUERIED;
						}

						AwaitingList.push_back(m);
					}
					continue;
				}
			}
		}
		else
		{
			if( (m.senderNick != getNick()) && (m.senderNick!="@Server") )
			{
				userData[m.senderNick].nick= m.senderNick;
				userData[m.senderNick].full_host= m.prefix;
				userData[m.senderNick].identification_state= IDENT_OK;
			}
		}

		processMessage(m);
		//printf("Processing '%s', left specials: %d\n", m.command.c_str(), special_commands.size());

		// try to process awaiting messages
		// (messages put aside because user not yet identified)
		list<IrcMessage>::iterator remove_it= AwaitingList.end();

		for(list<IrcMessage>::iterator it= AwaitingList.begin(); it!=AwaitingList.end(); it++)
		{
			if( remove_it!=AwaitingList.end() )
			{
				AwaitingList.erase(remove_it);
				remove_it= AwaitingList.end();
			}

			user_Info *u= getUserData(it->senderNick);
			// if user is now identified
			if( u && (u->identification_state==IDENT_OK) )
			{
				remove_it= it;
				processMessage(*it);
				bool end;

				MUTEX_GET(mutex_end);
				{
					end= irc_end;
				}
				MUTEX_RELEASE(mutex_end);

				if( end )
				{
					disconnect();
					return -1;
				}
			}
		}

		if( remove_it!=AwaitingList.end() )
			AwaitingList.erase(remove_it);


		// send awaiting messages (MODE, TOPIC)
		if( !special_commands.empty() )
		{
			for( uint i= 0; !special_commands.empty() && (i< special_commands.size()); i++)
			{
				bool skip= false;
				spec_cmd &c= special_commands.front();

				// op status required to change channel's flags
				if( c.type == SENDTYPE_CHANMODE )
				{
					//printf("Special: '%s', op(%d)\n", c.cmdline.c_str(), isBotOp(c.dest));

					if( !isBotOp(c.dest) )
					{
						skip= true;
					}
					else
					{
						chan_Info *ch= getChanData(c.dest);
						string flags_to_set= "";

						if( !ch )
						{
							Error("MODES sent for un unknown chan: %s\n", c.dest.c_str());
							special_commands.pop();
							continue;
						}


						for(uint i= 0; i< c.modes.length(); i++)
						{
							// flag not set
							if( (ch->modes & getChanFlagMaskForChar(c.modes[i])) == 0 )
								flags_to_set+= c.modes[i];
						}

						// "+"
						if( flags_to_set.length() == 1 )
						{
							special_commands.pop();
							continue;
						}

						c.cmdline = "MODE " + c.dest + " " + flags_to_set + " " + c.other;
					}
				}
				// op status required if chan is +t
				else if( c.type == SENDTYPE_TOPIC )
				{
					chan_Info *chan= getChanData(c.dest);
					if( !chan )
					{
						Error("TOPIC command for an unknow chan: %s\n", c.dest.c_str());
						special_commands.pop();
						continue;
					}

					//Debug("[chan status: \"%s\" : ident?(%d) : +t?(%d) : op?(%d)]\n", c.dest.c_str(), chan->identification_state==IDENT_OK, chan->hasFlag(CHANMODE_OPTOPIC), isOp(getNick(), c.dest));

					if( (chan->identification_state!=IDENT_OK) || (chan->hasMode(CHANMODE_OPTOPIC) && !isBotOp(c.dest)) )
						skip= true;
				}

				// required conditions not satisfied
				if( skip )
				{
					if( special_commands.size() > 1 )
					{
						special_commands.push(c);
						special_commands.pop();
					}
				}
				else
				{
					SendT->addCommandInQueue(c.cmdline);
					special_commands.pop();
				}
			}
		}

		bool end;

		MUTEX_GET(mutex_end);
		{
			end= irc_end;
		}
		MUTEX_RELEASE(mutex_end);

		if( end )
		{
			disconnect();
			return -1;
		}
	}

	bool end;

	MUTEX_GET(mutex_end);
	{
		end= irc_end;
	}
	MUTEX_RELEASE(mutex_end);


	disconnect();
	if( irc_end )
	{
		disconnect();
		return -1;
	}
	else
		return 0;
}

/*
//string BaseBot::loQuote(const string &msg)
{
	const string q("\0\r\n\020");

	// TODO optimize (important?)

	string tmp("");
	for(size_t i = 0; i < msg.length(); i++)
	{
		if(q.find(msg[i]) != string::npos)
			tmp += '\020';
		tmp += msg[i];
	}
	return tmp;
}

//bool BaseBot::loUnQuote(const string &msg, string &result)
{
	size_t last = 0;
	size_t f = 0;

	result = "";
	while((f = msg.find('\020', last)) != string::npos) {
		switch(msg[f+1]) {
			case '\0':
			case '\r':
			case '\n':
			case '\020': {
				result += msg.substr(last, f - last);
				result += msg[f+1];
				last = f+2;
				break;
			}
			default:
				return false;
		}
	}
	result += msg.substr(last, msg.length() - last + 1);
	return true;
}
*/

bool user_Info::hasFlag(uint n) const
{
	return (user_flags&n);
}

bool user_Info::hasChannelFlag(uint n, const string &channel)
{
	bool ret;

	map<string,uint>::iterator it= channel_flags.find(channel);
	if( it!=channel_flags.end() )
	{
		ret= (((*it).second & n) != 0);
	}
	else
	{
		ret= false;
	}

	return ret;
}

//////////////
// MESSAGES

const string BaseBot::getNick() const
{
	return bot_data.nick;
}

const user_Info *BaseBot::getBotUser()
{
	return &bot_data;
}

const vector<string> &BaseBot::getChannelList()
{
	return bot_data.ison.getVector();
}

void BaseBot::changeNick(const string &nick)
{
	if( nick != getNick() )
		sendMessage(SENDTYPE_NORMAL, "NICK %s", nick.c_str());
}

/*
      Command: PASS
   Parameters: <password>

      Command: NICK
   Parameters: <nickname>

     Command: USER
   Parameters: <user> <mode> <unused> <realname>
*/
void BaseBot::registerBot(const string &password, const string &user, const string &realname)
{
	if( !password.empty() )
		sendMessage(SENDTYPE_NORMAL, "PASS %s", password.c_str());

	changeNick(user);
	sendMessage(SENDTYPE_NORMAL, "USER %s 0 * :%s", user.c_str(), realname.c_str());
}

void BaseBot::quit(const string &message)
{
	if( !quit_sent && !irc_end )
	{
		onBotExit();
		quit_sent= true;
		sendMessage(SENDTYPE_NORMAL, "QUIT :%s", message.c_str());
	}
}

/********************************************************************/

// if MODE requested: (MODE <chan>)
// 324 a007 #eveR.ns +tncCul 1337 					// channel modes
// 329 a007 #eveR.ns 1055412080						// creation date (timestamp)

// errors:
// 405 Schmurfy[aw] #omg.pv :You have joined too many channels
// 475 a007 #bidon2 :Cannot join channel (+k)
void BaseBot::join(const string &channel, const string &key)
{
	// no need to join if already on the channel
	if( isBotOn(channel) )
		return;

	chanData[channel].topic= "";
	chanData[channel].topic_setby= "";
	chanData[channel].topic_time= 0;
	chanData[channel].key= key;

	sendMessage(SENDTYPE_NORMAL, "JOIN %s %s", channel.c_str(), key.c_str());
}

/*
      Command: KICK
   Parameters: <channel> *( "," <channel> ) <user> *( "," <user> )
               [<comment>]
*/
void BaseBot::kick(const string &channel, const string &nick, const string &reason)
{
	sendMessage(SENDTYPE_NORMAL, "KICK %s %s :%s", channel.c_str(), nick.c_str(), reason.c_str());
}

/*!
\brief send message to user
\param user user's nick
\param mode message mode
\param format&... message
\note if ctcp socket is found message will be sent in it
*/
void BaseBot::sendMsgTo(const string &dest, msgMode mode, char *format, ...)
{
	char msg[200];
	va_list varg;

	va_start(varg, format);
	vsnprintf(msg, sizeof(msg)-1, format, varg);
	va_end(varg);

	// channel and mode is MODE_PRIVMSG or MODE_NOTICE
	if( isChannelName(dest) && (mode<=1) )
		mode= (msgMode)(mode+2);

	switch(mode)
	{
	case MODE_NOTICE_ONLY: notice(dest, "%s", msg); return;
	case MODE_PRIVMSG_ONLY: privmsg(dest, "%s", msg); return;
	default: break;
	}

	if( userData[dest].ctcpSocket.isConnected() )
	{
		sendDirectMessage(userData[dest].ctcpSocket, "%s", msg);
	}
	else
	{
		switch(mode)
		{
		case MODE_NOTICE: notice(dest, "%s", msg); break;
		case MODE_PRIVMSG: privmsg(dest, "%s", msg); break;
		default: break;
		}
	}
}

string BaseBot::getFirstCommonChannelWithMe(const string &nick)
{
	user_Info *usr= getUserData(nick);
	if( usr!=NULL )
	{
		vector<string> &my_chans= bot_data.ison.getVector();
		vector<string> &user_chans= usr->ison.getVector();
		vector<string>::iterator it;

		for(uint i= 0; i< my_chans.size(); i++)
		{
			it= find(user_chans.begin(), user_chans.end(), my_chans[i]);
			if( it!=user_chans.end() )
			{
				return *it;
			}
		}
	}

	return "";
}

/*
private messaging function not subject to anti flood control from server !!
*/
void BaseBot::cnotice(const string &whom, const string &channel, const char *format, ...)
{
	va_list varg;
	char buf[100];

	va_start(varg, format);
	vsnprintf(buf, sizeof(buf), format, varg);
	va_end(varg);

	if( support_CNOTICE && !isChannelName(whom) )
		sendMessage(SENDTYPE_NORMAL, "CNOTICE %s %s :%s", whom.c_str(), channel.c_str(), buf);
}

void BaseBot::notice(const string &whom, const char *format, ...)
{
	va_list varg;
	char buf[100];

	va_start(varg, format);
	vsnprintf(buf, sizeof(buf), format, varg);
	va_end(varg);

	string ch= getFirstCommonChannelWithMe(whom);
	if( support_CNOTICE && !ch.empty() && isBotOp(ch) )
		cnotice(whom, ch, "%s", buf);
	else
		sendMessage(SENDTYPE_NORMAL, "NOTICE %s :%s", whom.c_str(), buf);
}

/*
private messaging function not subject to anti flood control from server !!
CPRIVMSG <nick> <channel> :<text>
*/
void BaseBot::cprivmsg(const string &whom, const string &channel, const char *format, ...)
{
	va_list varg;
	char buff[200];

	va_start(varg, format);
	vsnprintf(buff, 200, format, varg);
	va_end(varg);

	// only supported for private message to users
	if( support_CPRIVMSG  && !isChannelName(whom) )
		sendDirectMessage(cliSocket, "CPRIVMSG %s %s :%s", whom.c_str(), channel.c_str(),  buff);
}

void BaseBot::privmsg(const string &whom, const char *format, ...)
{
	va_list varg;
	char buff[200];

	va_start(varg, format);
	vsnprintf(buff, 200, format, varg);
	va_end(varg);

	string ch= getFirstCommonChannelWithMe(whom);
	if( support_CPRIVMSG && !ch.empty() && isBotOp(ch) )
		cprivmsg(whom, ch, "%s", buff);
	else
		sendMessage(SENDTYPE_NORMAL, "PRIVMSG %s :%s", whom.c_str(), buff);
}

/*
  The available modes are as follows:

	i - marks a users as invisible;
	x - hide hostname
*/
void BaseBot::userMode(const string &nick, const string &modestring)
{
	sendMessage(SENDTYPE_NORMAL, "MODE %s %s", nick.c_str(), modestring.c_str());
}

/*
Note that there is a maximum limit of three (3) changes per
   command for modes that take a parameter.
*/
void BaseBot::chanMode(const string &chan, const string &modestring)
{
	sendMessage(SENDTYPE_CHANMODE, "MODE %s %s", chan.c_str(), modestring.c_str());
}

void BaseBot::ban(const string &chan, const string &banmask)
{
	if( isBotOp(chan) )
		sendMessage(SENDTYPE_NORMAL, "MODE %s +b %s", chan.c_str(), banmask.c_str());
}

void BaseBot::unban(const string &chan, const string &banmask)
{
	if( isBotOp(chan) )
		sendMessage(SENDTYPE_NORMAL, "MODE %s -b %s", chan.c_str(), banmask.c_str());
}

// +o-b xxxx xxxx
void BaseBot::setMode(const string &channel, const string &nick, const string &mode2)
{
	string mode(mode2);

	if( mode.length()==2 )
	{
		sendMessage(SENDTYPE_CHANMODE, "MODE %s %s %s", channel.c_str(), mode.c_str(), nick.c_str());
	}
	else
	{
		uint i;
		string nicks= nick;

		char c;

		for(i= 0; i< mode.size(); i++)
		{
			if( (mode[i]=='-') || (mode[i]=='+') )
			{
				c= mode[i];
				continue;
			}

			// adding flag
			if( c=='+' )
			{
				if( ((mode[i]=='o') && isOp(nick, channel))  ||  ((mode[i]=='v') && isVoice(nick, channel)) )
				{
					mode.erase(i,1);
					i--;
					continue;
				}
			}

			// removing it
			if( c=='-' )
			{
				if( ((mode[i]=='o') && !isOp(nick, channel))  ||  ((mode[i]=='v') && !isVoice(nick, channel)) )
				{
					mode.erase(i,1);
					i--;
					continue;
				}
			}
		}

		if( mode.size() == 1 )
			return;

		for(i= 2; i< mode.length(); i++)
		{
			if( i!=mode.length()-1 )
				nicks+= " ";
			else
				nicks+= " :";
			nicks+= nick;
		}

		sendMessage(SENDTYPE_CHANMODE, "MODE %s %s %s", channel.c_str(), mode.c_str(), nicks.c_str());
	}
}

void BaseBot::setChannelPassword(const string &channel, const string &pass)
{
	sendMessage(SENDTYPE_CHANMODE, "MODE %s +k %s", channel.c_str(), pass.c_str());
}

/********************************************************************/

/*

      Command: TOPIC
   Parameters: <channel> [ <topic> ]

TOPIC #bidon
:euroserv.fr.quakenet.org 332 bhj #bidon :ggggggggggg
:euroserv.fr.quakenet.org 333 bhj #bidon eveR`Schmurfy 1057526393
TOPIC #bidon hh
:bhj!schmurfy@ammousj.net1.nerim.net TOPIC #bidon :hh
*/
// 332 <bot_name> <channel> :<topic>
// 333 <bot_name> <channel> <user_who_set_topic> <?>

//	331    RPL_NOTOPIC "<channel> :No topic is set"
//	332    RPL_TOPIC "<channel> :<topic>"
//	442    ERR_NOTONCHANNEL "<channel> :You're not on that channel
//	482    ERR_CHANOPRIVSNEEDED "<channel> :reason"
//	477    ERR_NOCHANMODES "<channel> :Channel doesn't support modes"
string BaseBot::getTopic(const string &channel)
{
	chanDataTypeIter chan_it= chanData.find(channel);
	if( chan_it!=chanData.end() )
	{
		return chan_it->second.topic;
	}
	else
		return "";
}

void BaseBot::setTopic(const char *channel, const char *topic)
{
	if( strcmp(getTopic(channel).c_str(), channel) )
		sendMessage(SENDTYPE_TOPIC, "TOPIC %s :%s", channel, topic);

}

/*
      Command: PART
   Parameters: <channel> *( "," <channel> ) [ <Part Message> ]
*/
void BaseBot::part(const string &channel, const string &reason)
{
	sendMessage(SENDTYPE_NORMAL, "PART %s %s", channel.c_str(), reason.c_str());
}

/*
      Command: INVITE
   Parameters: <nickname> <channel>

When the channel has invite-only
flag set, only channel operators may issue INVITE command.
Only the user inviting and the user being invited will receive
notification of the invitation.  Other channel members are not
notified.
*/
void BaseBot::invite(const string &nick, const string &chan)
{
	chan_Info *ch= getChanData(chan);
	if( (ch==NULL) || (ch->hasMode(CHANMODE_INVITEONLY) && isBotOp(chan)) )
	{
		sendMessage(SENDTYPE_NORMAL, "INVITE %s %s", nick.c_str(), chan.c_str());
	}
}

void BaseBot::getUserlistFromMask(const string mask, vector<user_Info*> &ulist)
{
	ulist.clear();

	for(userDataTypeIter it= userData.begin(); it!=userData.end(); it++)
	{
		user_Info &u= (*it).second;
		if( wc_match(mask, u.full_host) )
			ulist.push_back(&u);
	}
}

void BaseBot::getUserlistFromMask(const string mask, const string channel,  vector<user_Info*> &ulist)
{
	ulist.clear();

	for(userDataTypeIter it= userData.begin(); it!=userData.end(); it++)
	{
		user_Info &u= (*it).second;
		Debug("U[%s] C[%d] M[%s]\n", u.nick.c_str(), u.ison.isin(channel)?"1":"0", u.full_host.c_str());
		if( u.ison.isin(channel) && wc_match(mask, u.full_host) )
			ulist.push_back(&u);
	}
}

chan_Info *BaseBot::getChanData(const string &chan)
{
	{
		for(chanDataTypeIter it= chanData.begin(); it != chanData.end(); it++)
		{
			Debug(":> %s\n", it->first.c_str());
		}		
	}
	chanDataTypeIter it= chanData.find(chan);
	if( it!=chanData.end() )
		return &it->second;

	return NULL;
}

user_Info *BaseBot::getUserData(const string &nick)
{
	if( nick == getNick() )
		return &bot_data;

	userDataTypeIter it= userData.find(nick);
	if( it!=userData.end() )
		return &it->second;

	return NULL;
}

void BaseBot::createUserData(const string &user, const user_Info *usr)
{
	assert(usr!=NULL);
	userData[user]= *usr;
}

void BaseBot::clearBotChanFlags(const string &chan)
{
	bot_data.isvoice.remove(chan);
	bot_data.isop.remove(chan);
	bot_data.ison.remove(chan);
}

void BaseBot::clearChanFlags(const string &user, const string &chan)
{
	user_Info *usr= getUserData(user);
	if( usr==NULL )
		return;

	usr->isvoice.remove(chan);
	usr->isop.remove(chan);
	usr->ison.remove(chan);

	// check if user left bot view
	if( usr->ison.getVector().size() == 0 )
		usr->identification_state= IDENT_NOQUERY;
}

void BaseBot::destroyUserData(const string &user)
{
	userData.erase(user);
}

//-301 <bot_name> <PSEUDO> <MSG_AWAY>
//-311 <bot_name> <PSEUDO> <IDENT> <HOST> * :<REALNAME>
//-313 <bot_name> <PSEUDO> :<MSG> // pseudo is an irc operator
// (only sent with double whois) 317 <bot_name> <PSEUDO> <IDLE_SECONDS> <?> <TEXT>
//-330 <bot_name> <PSEUDO> <Q_AUTH> <TEXT>
//-318 <bot_name> <PSEUDO> <TEXT>
// (ignored) 319 <bot_name> <PSEUDO> :+#strasr0x +#cataclysme @#Ever.dream
//void BaseBot::whois(const string &user)

void BaseBot::whois(const string &user)
{
	// clear old data
	userData[user].nick= user;
	user_Info &usr= userData[user];

	usr.full_host= "";
	usr.auth= "";
//	usr.isop.clear();
//	usr.isvoice.clear();
//	usr.ison.clear();

	sendMessage(SENDTYPE_NORMAL, "WHOIS :%s", user.c_str());
}

/*! retrive Q login (QuakeNet)
*/
string BaseBot::getAuthOf(const string &nick)
{
	userDataTypeIter user_it= userData.find(nick);
	if( user_it != userData.end() )
	{
		return user_it->second.auth;
	}

	return "";
}

bool BaseBot::isBotOn(const string &chan)
{
	return bot_data.ison.isin(chan);
}

bool BaseBot::isBotOp(const string &chan)
{
	return bot_data.isop.isin(chan);
}

bool BaseBot::isInBanList(const string &banmask, const string &channel)
{
	bool ret= false;
	chan_Info *chan= getChanData(channel);
	if( chan != NULL )
	{
		for(uint i= 0; !ret && (i< chan->ban_list.size()); i++)
		{
			ret= ( banmask==chan->ban_list[i] );
		}
	}

	return ret;
}

bool BaseBot::isBanned(const string &host, const string &channel)
{
	bool ret= false;
	chan_Info *chan= getChanData(channel);
	if( chan != NULL )
	{
		for(uint i= 0; !ret && (i< chan->ban_list.size()); i++)
		{
			ret= ( wc_match(chan->ban_list[i], host) == 1 );
		}
	}

	return ret;
}

bool BaseBot::isOn(user_Info *usr, const string &chan){
	return usr->ison.isin(chan);
}

bool BaseBot::isOn(const string &user, const string &chan)
{
	userDataTypeIter user_it= userData.find(user);

	if( user_it != userData.end() )
		return user_it->second.ison.isin(chan);

	Error("isOn: No data for %s\n", user.c_str());
	return false;
}

bool BaseBot::isOp(user_Info *usr, const string &chan){
	return usr->isop.isin(chan);
}

bool BaseBot::isOp(const string &user, const string &chan)
{
	userDataTypeIter user_it= userData.find(user);

	if( user_it != userData.end() )
		return user_it->second.isop.isin(chan);

	Error("isOp: No data for %s\n", user.c_str());
	return false;
}

bool BaseBot::isVoice(user_Info *usr, const string &chan){
	return usr->isvoice.isin(chan);
}

bool BaseBot::isVoice(const string &user, const string &chan)
{
	userDataTypeIter user_it= userData.find(user);

	if( user_it != userData.end() )
		return user_it->second.isvoice.isin(chan);

	Error("isVoice: No data for %s\n", user.c_str());
	return false;
}

bool BaseBot::isChannelName(const string &name)
{
	char c= name[0];
	//return ((c=='&') || (c=='#') || (c=='+') || (c=='!'));
	return ( channel_prefix.find(c,0)!=string::npos );
}


void BaseBot::setIntOption(bot_int_option opt, uint n)
{
	switch( opt )
	{
	case OPTION_INT_USEAUTH	  : use_AUTH= (n!=0); break;
	case OPTION_INT_DEBUGMODE : debugMode= n; break;
	default:
		break;
	}
}

void BaseBot::setStrOption(bot_str_option opt, const string &str)
{
	switch( opt )
	{
	case OPTION_STR_REALNAME : realname= str; break;
	case OPTION_STR_ALTNICK  : alt_nick= str; break;
	default:
		break;
	}
}


/********************************************************************/


#ifdef TEST

#include <stdio.h>

void main()
{
	map<string,user_Info> usrs;
	uniq_vector<string> &v= usrs["test"].isop;

	v.add("#chan1");

	Output("%s\n", (v.isin("#chan1"))?"OK":"ERR");

}

#endif




/**/

