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
*	$Id: basebot.h,v 1.19 2004/08/06 19:21:26 anthalir Exp $
*
********************************************************************************
*/

#ifndef _IRC_H
#define _IRC_H

/*

Codes mirc:

couleur: 	\003x
	0 white		8  yellow
	1 black		9  lightgreen
	2 blue		10 cyan
	3 green		11 lightcyan
	4 red		12 lightblue
	5 brown		13 pink
	6 purple	14 grey
	7 orange	15 lightgrey
	99 or nothing: previous color

gras:		\002
souligné:	\037

*/

/*
IRC reply numbers:
http://www.alien.net.au/irc/irc2numerics.html
http://script.quakenet.org/index.php?p=raws

RFC: HTML:
http://www.valinor.sorcery.net/docs/
http://rfc.net/search.php3?phrase=Internet+Relay+Chat
*/

/*!
\file basebot.h
*/

#include "config.h"
#include <sys/types.h>
#include <time.h>
#include <string>
#include <algorithm>
#include <list>
#include <queue>
#include <vector>
#include <map>

#if !defined(WIN32)
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "thread.h"
#include "log.h"
#include "sockets.h"


typedef unsigned int uint;

#define CRLF "\r\n"

using namespace std;

#if defined(connect)
#undef connect
#endif


// IRC reply numbers

#define RPL_WELCOME 			"001"    	// The first message sent after client registration. The text used varies widely
#define RPL_YOURHOST 			"002"   	// Part of the post-registration greeting. Text varies widely
#define RPL_CREATED				"003"   	// RFC2812  :This server was created <date>  Part of the post-registration greeting. Text varies widely
#define RPL_MYINFO				"004"    	// RFC2812  <server_name> <version> <user_modes> <chan_modes>  Part of the post-registration greeting
#define	RPL_MOTDSTART			"375"		// RFC1459  :- <server> Message of the day -  Start of an RPL_MOTD list
#define	RPL_ENDOFMOTD			"376"		// RFC1459  :<info>  Termination of an RPL_MOTD list
#define RPL_NOMOTD				"422"
#define	RPL_NAMREPLY			"353"		// RFC1459  ( '=' / '*' / '@' ) <channel> ' ' : [ '@' / '+' ] <nick> *( ' ' [ '@' / '+' ] <nick> )  Reply to NAMES (See RFC)
#define	ERR_NOSUCHNICK			"401"		// RFC1459  <nick> :<reason>  Used to indicate the nickname parameter supplied to a command is currently unused

#ifdef DEBUG_EXCEPTIONS
#	define my_throw(X) {Debug("%s from %s:%d\n", #X, __FILE__, __LINE__); throw X();}
#	define my_throw_msg(X, M) {Debug("%s from %s:%d\n", #X, __FILE__, __LINE__); throw X(M);}
#else
#	define my_throw(X) throw X();
#	define my_throw_msg(X, M) throw X(M);
#endif

//pointer management class
template<class _T>
class Ptr
{
private:
	_T *p;

	Ptr<_T>& operator= ( const Ptr<_T>& ) {};

public:
	Ptr(uint n= 1) { p= new _T[n]; }
	~Ptr() { delete [] p; }
	_T *get() { return p; }
	_T *operator->() const { return (get()); }
	_T &operator[](int n) { return p[n]; }
};

// vector with no doubles in it
template<class _T>
class uniq_vector
{
private:
	vector<_T> _vec ;
	typename vector<_T>::iterator vec_it;

public:
/*
	uniq_vector()
	{
		printf("Creation of vector\n");
	}
*/
	// only add val if not found in vector
	void add(const _T &val)
	{
		vec_it= find(_vec.begin(), _vec.end(), val);
		if( vec_it==_vec.end() )
			_vec.push_back(val);
	}

	void remove(const _T &val)
	{
		vec_it= find(_vec.begin(), _vec.end(), val);
		if( vec_it!=_vec.end() )
			_vec.erase(vec_it);
	}

	bool isin(const _T &val)
	{
		vec_it= find(_vec.begin(), _vec.end(), val);
		return (vec_it!=_vec.end());
	}

	void clear()
	{
		_vec.clear();
	}

	vector<_T> &getVector()
	{
		return _vec;
	}
};

enum dummy_type {
	USER_INFO,
	CHAN_INFO
};

class dummy_Info
{
public:
	dummy_type type;
};


/*!
\brief store infos about all the users on the bot'channels
\attention informations about channels (op, voice, on) are not relevant if bot is not on this channel
*/
class user_Info: public dummy_Info
{
public:
	user_Info() : flags_set(false), account_name("_"), user_flags(0), identification_state(0), irc_op(false)
	{
		type= USER_INFO;
	}

	bool hasChannelFlag(uint n, const string &channel);
	bool hasFlag(uint n) const;
	bool hasAccount() const { return (account_name!="_"); }

// set from userfile
	bool flags_set;
	string account_name;
	uint user_flags;						// global user flags
	map<string, uint> channel_flags;		// channel specific flags
//-------------------

	string auth;

#define IDENT_NOQUERY	0
#define IDENT_QUERIED	1
#define IDENT_OK		2

	uint identification_state;
	bool irc_op;					// is user an IRC operator ?
	TCPClientSocket ctcpSocket;		// if a ctcp socket is opened, this var holds it
	string ident;
	string host;
	string full_host;
	string nick;
	uniq_vector<string> isop;		// list of channels where user is op
	uniq_vector<string> isvoice;	// list of channels where user is voiced
	uniq_vector<string> ison;		// list of channels where user is present
									// unreliable if bot is not on the channel
};

/*!
\brief store infos about all the channels where the bot is
*/
class chan_Info: public dummy_Info
{
public:
	chan_Info() : identification_state(0), name("<invalid>"), modes(0), hidden(false)
	{
		type= CHAN_INFO;
	}
	bool hasMode(uint n) const { return (modes&n); }

// channel flags
#define CHANMODE_PRIVATE 		(1<< 0)	// 'p'
#define CHANMODE_SECRET			(1<< 1)	// 's'
#define CHANMODE_MODERATED		(1<< 2)	// 'm'
#define CHANMODE_OPTOPIC		(1<< 3)	// 't'
#define CHANMODE_INVITEONLY		(1<< 4)	// 'i'
#define CHANMODE_NOPRIVMSGS		(1<< 5)	// 'n'
#define CHANMODE_REGONLY		(1<< 6)	// 'r'
#define CHANMODE_NOCOLOUR     	(1<< 7) // 'c'
#define CHANMODE_NOCTCP        	(1<< 8)	// 'C'
#define CHANMODE_DELJOINS     	(1<< 9) // 'D'
#define CHANMODE_NOQUITPARTS	(1<<10)	// 'u'

	uint identification_state;
	string topic;
	string topic_setby;
	time_t topic_time;
	string name;
	string key;
	uint modes;
	bool hidden;	// secret or private
	vector<string> userlist;
	vector<string> ban_list;
};

class Buffer
{
public:
	Buffer() : curpos(0) {}
	char data[1024];
	uint curpos;
};

//! for sendMsgTo
enum msgMode {
	MODE_PRIVMSG= 0,
	MODE_NOTICE,
	MODE_PRIVMSG_ONLY,
	MODE_NOTICE_ONLY
};

struct IrcMessage
{
	bool have_prefix;
	string senderNick;
	string prefix;
	string command;
	vector<string> param;
};

class SendThread : public PThread
{
private:
	Mutex queue_mutex;				/* ptotect send_list, modes_list */
/**/
	queue<string> send_list;
/**/
	uint floodcounter;

public:
	static const int SENDSIG_WAITDATA;
	static const int SENDSIG_FLOOD;

	SendThread(void *v);
	~SendThread();
	void user_func();
	void addCommandInQueue(const string &msg);
	void resetBuffer();
};


class ReadThread : public PThread
{
private:
	map<int, Buffer> recv_buffers;
	Mutex rdqueue_mutex;			/* protect msg_list */
/**/
	list<IrcMessage> msg_list;		//!< store the messages
/**/
	bool connected;
	bool waiting;					//!< a message is awaited

public:
	static const int READSIG_WAITMSG;
	static const int READSIG_WAITCON;

	ReadThread(void *);
	~ReadThread();
	void user_func();
	int readMessage(TCPClientSocket &s, string &msg, bool ctcp);
	void parseMessage(const string &msg, IrcMessage &m);
	int waitMessage(IrcMessage&);	//!< suspend execution until one of the specified messages arrive
	bool isConnected() const { return connected; }
};

// for setOption
enum bot_int_option
{
	OPTION_INT_INVALID= 0,
	OPTION_INT_USEAUTH,
	OPTION_INT_DEBUGMODE
};

enum bot_str_option
{
	OPTION_STR_INVALID= 0,
	OPTION_STR_REALNAME,
	OPTION_STR_ALTNICK
};

/*!
\brief Bot base class
*/

// used by BaseBot and SendThread
enum send_type {
	SENDTYPE_NORMAL= 0,				// all the other commands
	SENDTYPE_CHANMODE,				// MODE <chan> +ok bob key
	SENDTYPE_TOPIC					// TOPIC %s :%s
};

class BaseBot
{
private:


	// used for MODE and TOPIC messages as they are special cases
	struct spec_cmd {
		send_type 	type;
		string 		dest;
		string 		modes;
		string		other;	// nick list for +o +v
		string 		cmdline;
	};

	TCPClientSocket cliSocket;					//!< main socket (server)

	queue<spec_cmd>	special_commands;		//!< send queue for special commands that need specific conditions

	Mutex mutex_end;
	SendThread *SendT;					//!< send thread
	ReadThread *ReadT;					//!< read thread
	friend class SendThread;
	friend class ReadThread;

	// send command immediatly
	void sendDirectMessage(TCPClientSocket &s, const char *param, ...);

	int processMessage(const IrcMessage &m);

	bool findCTCP(const string &msg, string &ctcp);
	//string loQuote(const string &msg);
	//bool loUnQuote(const string &msg, string &result);

/* configurations variables (read from 005 message) */
	bool support_USERIP;
	bool support_CPRIVMSG;
	bool support_CNOTICE;
	bool support_WHOX;
	string channel_prefix;
	string network_name;
	uint max_nicklen;
	uint max_channels;
	uint max_modes;
	uint max_channel_bans;
	uint max_topiclen;
	uint max_kickmsglen;

public:
	uint getMaxNickLen(){ return max_nicklen; }
	uint getMaxChannels(){ return max_channels; }
	uint getMaxChannelBans(){ return max_channel_bans; }
	uint getMaxTopicLen(){ return max_topiclen; }
	uint getKickMsgLen(){ return max_kickmsglen; }

	const string &getNetworkName(){ return network_name; }


/****************************************************/


protected:
	class Func_StrComp
	{
		public:
		bool operator()(const string &s1, const string &s2) const
		{
			return (strcasecmp(s1.c_str(), s2.c_str()) < 0);
		}
	};

	typedef map<string, user_Info, Func_StrComp> userDataType;
	typedef map<string, user_Info, Func_StrComp>::iterator userDataTypeIter;

	typedef map<string, chan_Info, Func_StrComp> chanDataType;
	typedef map<string, chan_Info, Func_StrComp>::iterator chanDataTypeIter;

	string current_Hostname;			//!< store the hostname of current server
	userDataType userData;
	chanDataType chanData;

	user_Info bot_data;

	Mutex nick_mutex;

/* other configuration variables */
	string realname;
	string alt_nick;
	bool use_AUTH;
	uint debugMode;	/* bit mask: 0x01= debug rcv
								 0x02= debug send
					*/

public:
	bool quit_sent;
	bool irc_end;
	BaseBot();
	virtual ~BaseBot();

	const string getNick() const;
	const user_Info *getBotUser();
	const vector<string> &getChannelList();

	//int connect(const string &host, uint port = 6667);
	void disconnect();
	int mainLoop(const string&, const string&);

	bool isChannelName(const string &name);

	void changeNick(const string &nick);										// NICK
	void registerBot(const string &password, const string &user, const string &realname); 	// USER
	void quit(const string &message= ""); 										// QUIT
	void join(const string &channel, const string &key = "");					// JOIN
	void part(const string &channel, const string &reason = "");				// PART
	void kick(const string &channel, const string &nick, const string &reason);	// KICK
	void setTopic(const char *channel, const char *topic);						// TOPIC
	string getTopic(const string &channel);
	void invite(const string &nick, const string &chan);

	void ban(const string &chan, const string &banmask);
	void unban(const string &chan, const string &banmask);
	void userMode(const string &nick, const string &modestring); 				// MODE
	void chanMode(const string &chan, const string &modestring);
	void setMode(const string &channel, const string &nick, const string &mode);
	void setChannelPassword(const string &channel, const string &pass);

	void privmsg(const string &whom, const char *format, ...);
	void cprivmsg(const string &whom, const string &channel, const char *format, ...);
	void notice(const string &whom, const char *format, ...);
	void cnotice(const string &whom, const string &channel, const char *format, ...);
	void sendMsgTo(const string &dest, msgMode mode, char *format, ...);

	void requestChanInfos(const string &chan);

	void getUserlistFromMask(const string mask, vector<user_Info*> &ulist);
	void getUserlistFromMask(const string mask, const string channel,  vector<user_Info*> &ulist);

	user_Info *getUserData(const string&);
	chan_Info *getChanData(const string&);
	void createUserData(const string&, const user_Info*);
	void clearChanFlags(const string &user, const string &chan);
	void clearBotChanFlags(const string &chan);
	//void updateUserData(const string &user);
	void destroyUserData(const string &user);

	bool isOn(user_Info *usr, const string &chan);
	bool isOn(const string &user, const string &chan);

	bool isOp(user_Info *usr, const string &chan);
	bool isOp(const string &user, const string &chan);

	bool isVoice(user_Info *usr, const string &chan);
	bool isVoice(const string &user, const string &chan);


	bool isBotOp(const string &chan);
	bool isBotOn(const string &chan);
	string getAuthOf(const string &nick);

	void whois(const string &user);

	void setIntOption(enum bot_int_option, uint);
	void setStrOption(enum bot_str_option, const string&);

	void sendMessage(send_type type, const char *format, ...);

	string getFirstCommonChannelWithMe(const string &nick);

	/** bans **/
	bool isBanned(const string &host, const string &channel);
	bool isInBanList(const string &banmask, const string &channel);

	/* ---- signals ---- */

	virtual void onPing(const string&){}
	virtual void onConnected(){}
	virtual void onRegistered(){}
	virtual void onBotExit(){}
	virtual void onJoin(const string&, const string&){}
	virtual void onPart(const string&, const string&){}
	virtual void onQuit(const string&, const string&){}
	virtual void onPrivMsg(const string&, const string&, const string&){}
	virtual void onKick(const string&, const string&, const string&, const string&){}
	virtual void onNickChange(const string&, const string&){}
	virtual void onTopicChange(const string&, const string&, const string&){}

	//virtual void onModeChange(const string&, const string&, string*, string*, uint){}
	virtual void onBotJoined(		const string &channel, const vector<string> &userlist){}
	virtual void onServerOp(		const string &channel, const string &target){}
	virtual void onServerRemoveOp(	const string &channel, const string &target){}
	virtual void onServerVoice(		const string &channel, const string &target){}
	virtual void onServerRemoveVoice(const string &channel, const string &target){}
	virtual void onBotGainOp(		const string &channel, const string &sender){}
	virtual void onBotLostOp(		const string &channel, const string &sender){}
	virtual void onBotGainVoice(	const string &channel, const string &sender){}
	virtual void onBotLostVoice(	const string &channel, const string &sender){}
	virtual void onBotBanned(		const string &channel, const string &sender, const string &banmask){}
	virtual void onBotUnBanned( 	const string &channel, const string &sender, const string &banmask){}

	virtual void onGainOp(	 const string &channel, const string &sender, const string &target){}
	virtual void onLostOp(	 const string &channel, const string &sender, const string &target){}
	virtual void onGainVoice(const string &channel, const string &sender, const string &target){}
	virtual void onLostVoice(const string &channel, const string &sender, const string &target){}
	virtual void onBanned(	 const string &channel, const string &sender, const string &banmask){}
	virtual void onUnBan(	 const string &channel, const string &sender, const string &banmask){}

	virtual void onChanKeySet(const string &channel, const string &sender, const string &key){}
	virtual void onChanKeyRemoved(const string &channel, const string &sender){}

	virtual void onChanModeChanged(const string&, const string&, const string&){}

	/* ---- CTCP signals */
	virtual void onCTCPVersion(const string&){}
	virtual void onCTCPFinger(const string&){}
	virtual void onCTCPSource(const string&){}
	virtual void onCTCPUserInfo(const string&){}
	virtual void onCTCPClientInfo(const string&){}
	virtual void onCTCPPing(const string&){}
	virtual void onCTCPAction(const string&, const string&, const string&){}
};

#endif
