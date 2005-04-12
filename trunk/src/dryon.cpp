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
features:
- flood control
- scripts use the SMALL language (http://www.compuphase.com/small.htm)
- scripts use Ruby language
- scripts can save/load data
- reconnect on disconnect
- multiple scripts can be loaded at the same time
- events driven scripts

Supported server (the bot was tested on them):
- QuakeNet
- GameSurge
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <locale.h>
#include <stdarg.h>

#if defined(__FreeBSD__)

#include <unistd.h>
#include <dirent.h>

#elif defined(WIN32)
#include <crtdbg.h>
#if _MSC_VER <= 1200
#define __FUNCTION__ "(none)"
#endif
#include "termwin.h"
ConsoleThread console;
HANDLE main_thread;

#endif

#include <string>
#include <vector>
#include <exception>
#include <algorithm>
#include <iostream>
#include <fstream>

#include "amxbot.h"
#include "script.h"
#include "Small/amx_script.h"	// to remove asap
#include "utils.h"
#include "log.h"
#include "cfg.h"
#include "userfile.h"
#include "match.h"
#include "serverlist.h"
#include "tokens.h"

using namespace std;

#define GET_USR(U,N) if( (U= getUserData(N))==NULL){ Error("invalid user '%s' in %s in %s(%d)\n", N.c_str(), __FUNCTION__, basename(__FILE__), __LINE__); return; }
#define GET_CHAN(C,N) if( (C= getChanData(N))==NULL){ Error("invalid channel '%s' in %s in %s(%d)\n", N.c_str(), __FUNCTION__, basename(__FILE__),  __LINE__); return; }

Logs ErrorLog(	"Err", COLOR_RED,		"logs/errors.log");
Logs DebugLog(	"Dbg", COLOR_MAGENTA,	"logs/errors.log");
Logs AmxLog(	"Amx", COLOR_RED,		"logs/amx.log");

DynamicExtensionsPool extPool;

ServerList servers;
ScriptManager PMgr;
ConfigFileReader cfg;
UserFile userfile;
AMXBot bot;

bool test_mode= false;
bool help_mode= false;
bool hidden_launch= false;	// start hidden (windows)
string serverlist_path= "servers.txt";
string userfile_path= "userfile.txt";
string configfile_path= "dryon.cfg";

/*
options			| 	effect
--hidden				start hidden (windows)
--test				enable test mode
--help				display help
--config xxx		set dryon.cfg file
--servers xxx		set servers.txt file
--userfile xxx		set userfile.txt file
*/
int parseCommandLine(int argc, char **argv)
{
	int i= 1;
	uint state= 0; 	// 1= expecting dryon.cfg path
					// 2= expecting servers.txt path
					// 3= expecting userfile.txt path
	for(i= 1; i< argc; i++)
	{
		if( state != 0 )
		{
			switch( state )
			{
			case 1: configfile_path= argv[i]; break;
			case 2: serverlist_path= argv[i]; break;
			case 3: userfile_path= argv[i]; break;
			}

			state= 0;
		}
		else if( !strcmp(argv[i], "--test") )
		{
			test_mode= true;
		}
		else if( !strcmp(argv[i], "--help") )
		{
			help_mode= true;
		}
		else if( !strcmp(argv[i], "--hidden") )
		{
			hidden_launch= true;
		}
		else if( !strcmp(argv[i], "--config") )
		{
			state= 1;
		}
		else if( !strcmp(argv[i], "--servers") )
		{
			state= 2;
		}
		else if( !strcmp(argv[i], "--userfile") )
		{
			state= 3;
		}
	}

	if( state != 0 )
	{
		Error("Error at '%s' in command line\n", argv[i-1]);
		return -1;
	}

	return 0;
}

/*
-1 : unable to create a required file
 0 : no error
 x : x files created, need editing (exit)
*/
int makeSanityChecks()
{
	int n= 0;

#define NEWFILE(N) Output("* ++> "N" created.\n")

	Output("** Sanity checks:\n");
	Output("* checking folders...\n");

	if( !fileExist("logs") )
	{
		if( createFolder("logs") != -1 ){
			Output("- folder created: logs\n");
		}else{
			Error("Unable to create 'logs' folder\n");
			return -1;
		}
	}

	if( !fileExist("libs") )
	{
		if( createFolder("libs") != -1 ){
			Output("- folder created: libs\n");
		}else{
			Error("Unable to create 'libs' folder\n");
			return -1;
		}
	}
	
	if( !fileExist("script_configs") )
	{
		if( createFolder("script_configs") != -1 ){
			Output("- folder created: script_configs\n");
		}else{
			Error("Unable to create 'script_configs' folder\n");
			return -1;
		}
	}	

	Output("* checking config files...\n");

	// only create it if default location used
	if( !fileExist(serverlist_path.c_str()) )
	{
		if( serverlist_path=="servers.txt"  )
		{
			fstream file("servers.txt", ios::out);
			if( file.bad() )
			{
				Error("Unable to create default servers.txt !\n");
				return -1;
			}

			file << "# format: <hostname/ip>[:<port>[:<password>]]" << endl;
			file << "# the only required field is hostname, default port is 6667" << endl;
			file << "Burstfire.UK.EU.GameSurge.net\n" << endl;
			file.close();

			NEWFILE("servers.txt");
			n++;
		}
		else
		{
			Error("Unable to open '%s' !\n", serverlist_path.c_str());
			return -1;
		}
	}

	if( !fileExist(userfile_path.c_str()) )
	{
		if( userfile_path=="userfile.txt" )
		{
			fstream file("userfile.txt", ios::out);
			if( file.bad() )
			{
				Error("Unable to create default userfile.txt !\n");
				return -1;
			}

			file << getUserfileTemplate();
			file << endl;

			NEWFILE("userfile.txt");

			file.close();

			n++;
		}
		else
		{
			Error("Unable to open '%s' !\n", userfile_path.c_str());
			return -1;
		}
	}

	if( !fileExist(configfile_path.c_str()) )
	{
		if( configfile_path=="dryon.cfg" )
		{
			fstream file("dryon.cfg", ios::out);
			if( file.bad() )
			{
				Error("Unable to create default dryon.cfg !\n");
				return -1;
			}

			file << "// Dryon config file" << endl << endl;
			file << "///////////////////////////" << endl;
			file << "// irc server config" << endl;
			file << "botname			= dryon" << endl;
			file << "alt_nick			= dryon02135" << endl;
			file << "realname			= ?" << endl;
			file << "useauth			= false" << endl;
			file << "// bit mask: 1= show all the IRC messages received" << endl;
			file << "//           2= show all the IRC messages sent" << endl;
			file << "//           4= show debug messages" << endl;
			file << "debugmode			= 0" << endl;
			file << endl;
			file << "// scripts to load" << endl;
			file << "script_01				= scripts/main.rb" << endl;
			file << "script_02				= scripts/debug.rb" << endl;
			file << endl;


			file.close();
			NEWFILE("dryon.cfg");
			n++;
		}
		else
		{
			Error("Unable to open '%s' !\n", configfile_path.c_str());
			return -1;
		}
	}

	Output("** Checks done\n");
	return n;
}

// called when connected
void AMXBot::onConnected()
{
	welcome_received= true;
	PMgr.callEvent("event_onConnected");
}

// called when authed & registered with Qnet
void AMXBot::onRegistered()
{
	PMgr.callEvent("event_onRegistered");
}

void AMXBot::onBotExit()
{
	PMgr.callEvent("event_onBotExit");
}

AMXBot::~AMXBot()
{
	PMgr.unloadAllPlugins();
}

void AMXBot::onPing(const string &str)
{
	PMgr.ping();
}


void AMXBot::onJoin(const string &nick, const string &chan)
{
	user_Info *u;
	chan_Info *ch;

	GET_USR(u, nick);
	GET_CHAN(ch, chan);

	checkUserFlags(u, chan);
	PMgr.callEvent("event_onJoin", "uc", u, ch);
}

void AMXBot::onBotJoined(const string &channel, const vector<string> &userlist)
{
	chan_Info *ch;
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onBotJoined", "cv", ch, &userlist);
}

// update flags for user matching mask
void AMXBot::updateFlags(const string &mask)
{
	// retrieve all users matching mask
	vector<user_Info *> ulist;
	getUserlistFromMask(mask, ulist);

	for(uint i= 0; i< ulist.size(); i++ )
	{
		user_Info *usr= ulist[i];

		// forced update
		usr->flags_set= false;
		const vector<string> &clist= usr->ison.getVector();
		for(uint j= 0; j< clist.size(); j++)
			checkUserFlags(usr, clist[j]);
	}
}

// set flags in the user_Info structure for this user
void AMXBot::checkUserFlags(user_Info *usr, const string &dest)
{
	if( !usr->flags_set )
	{
		// global flags
		userfile.setUserFlagsFromUserfile(*usr, use_AUTH);
		// flags for channel(s)
		userfile.setChanFlagsFromUserfile(*usr);

		Debug("Flags set for '%s'\n", usr->nick.c_str());
		usr->flags_set= true;
	}

	// if user has no account do nothing
	if( usr->hasAccount() )
	{
		if( isChannelName(dest) )
		{
			// global flag(s)
			bool glb_need_op=		usr->hasFlag(USRLVL_OPERATOR);
			bool glb_need_voice=	usr->hasFlag(USRLVL_VOICE);
			bool glb_need_kickban=	usr->hasFlag(USRLVL_KICK);

			// channel specific flag(s)
			bool need_op=			usr->hasChannelFlag(USRLVL_OPERATOR, dest);
			bool need_voice=		usr->hasChannelFlag(USRLVL_VOICE, dest);
			bool need_kickban=		usr->hasChannelFlag(USRLVL_KICK, dest);

			if( glb_need_kickban || need_kickban )
			{
				if( isOp(usr, dest) )
					setMode(dest, usr->nick, "-o");

				ban(dest, usr->full_host);
				kick(dest, usr->nick, "auto-kick");
			}
			else if( glb_need_op || need_op )
			{
				if( !isOp(usr, dest) )
					setMode(dest, usr->nick, "+o");
			}
			else if( glb_need_voice || need_voice )
			{
				if( !isVoice(usr, dest) )
					setMode(dest, usr->nick, "+v");
			}
		}
	}
}

void AMXBot::onPart(const string &nick, const string &chan)
{
	chan_Info *ch;
	user_Info *u;
	GET_CHAN(ch, chan);
	GET_USR(u, nick);

	if( nick == getNick() )
	{
		clearBotChanFlags(chan);
		PMgr.callEvent("event_onBotPart", "c", ch);
	}
	else
	{
		clearChanFlags(nick, chan);
		PMgr.callEvent("event_onPart", "uc", u, ch);
	}
}

void AMXBot::onQuit(const string &nick, const string &msg)
{
	user_Info *u;
	GET_USR(u, nick);

	PMgr.callEvent("event_onQuit", "u", u);
	destroyUserData(nick);
}

void AMXBot::onPrivMsg(const string &sender, const string &dest, const string &msg)
{
	vector<string> msg_parts;
	Tokenize(msg, msg_parts, " ");
	user_Info *usr;
	GET_USR(usr, sender);

	const string &command= msg_parts[0].substr(1);

	// load user flags from userfile.txt
	checkUserFlags(usr, dest);

	if( command=="reload" )
	{
		/* if user has owner flag then reload all config files */
		if( usr->hasFlag(USRLVL_OWNER) )
		{
			// force update of user flags
			for(userDataTypeIter it= userData.begin(); it!= userData.end(); it++)
				(*it).second.flags_set= false;

			servers.loadFromFile("servers.txt");
			userfile.saveFile();
			userfile.readUserFile();
			cfg.readFile();
			PMgr.loadScriptsFromCfg(cfg);
		}
		return;
	}
	// user list
	else if( command=="userlist" )
	{
		if( usr->hasAccount() && usr->hasFlag(USRLVL_OWNER) )
		{
			map<string, UsersData> &ulist= userfile.getAccessList();
			notice(sender, "User list: %d entries", ulist.size());
			notice(sender, "-------- Start (Show accounts and global flags) --------");

			for(map<string, UsersData>::iterator it= ulist.begin(); it!=ulist.end(); it++)
			{
				string what= it->first;
				if( (what[0]=='a') && (what[1]=='/') )
				{
					what= '#' + what.substr(2);
				}

				notice(sender, "%15s %35s %5s", it->second.name.c_str(), what.c_str(), it->second.flags.c_str());
			}

			notice(sender, "--------  End  --------");
		}
		return;
	}
	else if( command == "showflags" )
	{
		if( usr->hasAccount() && usr->hasFlag(USRLVL_MASTER) )
		{
			map<string, UsersData> &ulist= userfile.getAccessList();
			string &chan= msg_parts[1];
			notice(sender, "User list for %s", chan.c_str());

			for(map<string, UsersData>::iterator it= ulist.begin(); it!=ulist.end(); it++)
			{
				string what= it->first;
				UsersData &tmp= it->second;

				for(map<string,string>::iterator it2= tmp.channel_flags.begin(); it2 != tmp.channel_flags.end(); it2++)
				{
					if( it2->first == chan )
						notice(sender, "%15s %5s", it->second.name.c_str(), it2->second.c_str());
				}
			}
			notice(sender, "--------  End  --------");
		}
	}
	// return a list of loaded plugins
	else if( command=="scripts" )
	{
		uint count= PMgr.getPluginCount();
		for(int i= count-1; i>= 0; i--)
		{
			notice(sender, "%2d ) %s\n", i, PMgr.getPlugin(i)->getPath());
		}
	}
	else if( command=="quit" )
	{
		if( usr->hasFlag(USRLVL_OWNER) )
		{
			quit("exit requested");
		}
		return;
	}
	else if( command=="help" )
	{
		if( usr->hasAccount() )
		{
			notice(sender, "Core commands:");
			if( usr->hasFlag(USRLVL_OWNER) )
			{
				notice(sender, ":reload - reload all plugins and config file");
				notice(sender, ":userlist - display user list");
			}

			notice(sender, ":help - display this :<");
			notice(sender, ":scripts - display list of currently loaded scripts");
			notice(sender, "---");
			PMgr.sendCommandsList(sender, dest, usr);
			//return;
		}
	}
	else if( command=="nfo" )
	{
		if( msg_parts.size() == 1 )
		{
			Output("Nick: '%s'\n", bot_data.nick.c_str());
			Output("Host: '%s'\n", bot_data.full_host.c_str());
			Output("Auth: '%s'\n", bot_data.auth.c_str());
		}
		else if( msg_parts.size() >= 2 )
		{
			user_Info *u= getUserData(msg_parts[1]);
			if( u!=NULL )
			{
				Output("Nick: '%s'\n", u->nick.c_str());
				Output("Host: '%s'\n", u->full_host.c_str());
				Output("Auth: '%s'\n", u->auth.c_str());
			}
			else
			{
				notice(sender, "Nick unknown: %s", msg_parts[1].c_str());
			}
		}
	}
	else
	{
		chan_Info *chan;
		if( isChannelName(dest) )
		{
			GET_CHAN(chan, dest);
			PMgr.callEvent("event_onPrivMsg", "ucv", usr, chan, &msg_parts);
			PMgr.callCommand(usr, (dummy_Info*)chan, msg_parts);
		}
		else
		{
			PMgr.callEvent("event_onPrivMsg", "uuv", usr, &bot_data, &msg_parts);
			PMgr.callCommand(usr, (dummy_Info*)&bot_data, msg_parts);
		}
	}
}

void AMXBot::onTopicChange(const string &nick, const string &chan, const string &new_topic)
{
	user_Info *u;
	chan_Info *ch;
	GET_USR(u, nick);
	GET_CHAN(ch, chan);

	string oldtopic= ch->topic;
	ch->topic= new_topic;
	PMgr.callEvent("event_onTopicChange", "ucss", u, ch, oldtopic.c_str(), new_topic.c_str());
}

void AMXBot::onKick(const string &kicker, const string &chan, const string &kicked, const string &msg)
{
	user_Info *u1;
	user_Info *u2;
	chan_Info *ch;
	GET_USR(u1, kicker);
	GET_USR(u2, kicked);
	GET_CHAN(ch, chan);

	if( kicked == getNick() )
	{
		clearBotChanFlags(chan);
		PMgr.callEvent("event_onBotKicked", "ucs", u1, ch, msg.c_str());
	}
	else
	{
		clearChanFlags(kicked, chan);
		PMgr.callEvent("event_onKick", "ucus", u1, ch, u2, msg.c_str());
	}
}

void AMXBot::onBotGainOp(const string &channel, const string &sender)
{
	user_Info *u;
	chan_Info *ch;
	GET_USR(u, sender);

	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onBotOpped", "cu", ch, u);
}

void AMXBot::onBotLostOp(const string &channel, const string &sender)
{
	user_Info *u;
	chan_Info *ch;
	GET_USR(u, sender);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onBotDeopped", "cu", ch, u);
}

void AMXBot::onBotGainVoice(const string &channel, const string &sender)
{
	user_Info *u;
	chan_Info *ch;
	GET_USR(u, sender);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onBotVoiced", "cu", ch, u);
}

void AMXBot::onBotLostVoice(const string &channel, const string &sender)
{
	user_Info *u;
	chan_Info *ch;
	GET_USR(u, sender);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onBotDevoiced", "cu", ch, u);
}

void AMXBot::onBotBanned(const string &channel, const string &sender, const string &banmask)
{
	user_Info *u;
	chan_Info *ch;
	GET_USR(u, sender);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onBotBanned", "cus", ch, u, banmask.c_str());
}

void AMXBot::onBotUnBanned(const string &channel, const string &sender, const string &banmask)
{
	user_Info *u;
	chan_Info *ch;
	GET_USR(u, sender);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onBotUnbanned", "cus", ch, u, banmask.c_str());
}

void AMXBot::onServerOp(const string &channel, const string &target)
{
	user_Info *t;
	chan_Info *ch;
	GET_USR(t, target);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onServerOp", "cu", ch, t);
}

void AMXBot::onServerVoice(const string &channel, const string &target)
{
	user_Info *t;
	chan_Info *ch;
	GET_USR(t, target);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onServerVoice", "cu", ch, t);
}

void AMXBot::onServerRemoveOp(const string &channel, const string &target)
{
	user_Info *t;
	chan_Info *ch;
	GET_USR(t, target);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onServerRemoveOp", "cu", ch, t);
}

void AMXBot::onGainOp(const string &channel, const string &sender, const string &target)
{
	user_Info *u, *t;
	chan_Info *ch;
	GET_USR(u, sender);
	GET_USR(t, target);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onOp", "cuu", ch, u, t);
}

void AMXBot::onServerRemoveVoice(const string &channel, const string &target)
{
	user_Info *t;
	chan_Info *ch;
	GET_USR(t, target);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onServerRemoveVoice", "cu", ch, t);
}

void AMXBot::onLostOp(const string &channel, const string &sender, const string &target)
{
	user_Info *u, *t;
	chan_Info *ch;
	GET_USR(u, sender);
	GET_USR(t, target);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onDeOp", "cuu", ch, u, t);
}

void AMXBot::onGainVoice(const string &channel, const string &sender, const string &target)
{
	user_Info *u, *t;
	chan_Info *ch;
	GET_USR(u, sender);
	GET_USR(t, target);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onVoice", "cuu", ch, u, t);
}

void AMXBot::onLostVoice(const string &channel, const string &sender, const string &target)
{
	user_Info *u, *t;
	chan_Info *ch;
	GET_USR(u, sender);
	GET_USR(t, target);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onDeVoice", "cuu", ch, u, t);
}

void AMXBot::onBanned(const string &channel, const string &sender, const string &banmask)
{
	user_Info *u;
	chan_Info *ch;
	GET_USR(u, sender);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onBan", "cus", ch, u, banmask.c_str());
}

void AMXBot::onUnBan(const string &channel, const string &sender, const string &banmask)
{
	user_Info *u;
	chan_Info *ch;
	GET_USR(u, sender);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onUnBan", "cus", ch, u, banmask.c_str());
}

void AMXBot::onChanKeySet(const string &channel, const string &sender, const string &key)
{
	user_Info *u;
	chan_Info *ch;
	GET_USR(u, sender);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onChanKeySet", "cus", ch, u, key.c_str());
}

void AMXBot::onChanKeyRemoved(const string &channel, const string &sender)
{
	user_Info *u;
	chan_Info *ch;
	GET_USR(u, sender);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onChanKeyRemoved", "cu", ch, u);
}

void AMXBot::onChanModeChanged(const string &sender, const string &channel, const string &mode)
{
	user_Info *u;
	chan_Info *ch;
	GET_USR(u, sender);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onChanMode", "ucs", u, ch, mode.c_str());
}


void AMXBot::onNickChange(const string &oldnick, const string &newnick)
{
	user_Info *usr;
	GET_USR(usr, oldnick);

	createUserData(newnick, usr);
	destroyUserData(oldnick);

	GET_USR(usr, newnick);

	PMgr.callEvent("event_onNickChange", "su", oldnick.c_str(), usr);
}

void AMXBot::onCTCPAction(const string &action, const string &user, const string &channel)
{
	user_Info *u;
	chan_Info *ch;
	GET_USR(u, user);
	GET_CHAN(ch, channel);

	PMgr.callEvent("event_onAction", "usc", u, action.c_str(), ch);
}


void sigINT(int sig)
{
	Debug("CTRL+C: exiting\n");
	bot.quit();
}


extern "C" void _CheckMemLeaks();

int main(int argc, char **argv)
{
	int ret= 0;
	int ok= parseCommandLine(argc, argv);

#if defined(DEBUG_MEM)
	atexit(_CheckMemLeaks);
#endif

#if defined(__FreeBSD__)

	//set_terminate( term_exception );
	setlocale(LC_TIME, "fr_FR.ISO8859-1");
	//signal(SIGINT, sigINT);
	signal(SIGPIPE, SIG_IGN);

#elif defined(WIN32)
	setlocale(LC_TIME, "French");
	WSAData wsaData;
	main_thread= GetCurrentThread();
	console.run();
	sleep(1);

#	if defined(_DEBUG) && defined(_MSC_VER)
		// add breakpoint when memory block n is allocated
		// _crtBreakAlloc
		//_CrtSetBreakAlloc(n);
		//_CrtSetBreakAlloc(10193);
		//_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF |
		//	_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_DELAY_FREE_MEM_DF);
#	endif

if( WSAStartup(MAKEWORD(1,1), &wsaData) != 0)
{
	Error("Unable to initialise network: %d\n", WSAGetLastError());
}
else
{
#endif
	Output("Dryon v" BOT_VERSION " compiled on " __DATE__ "\n");
	Output("Embedded AMX: v" SMALL_VERSION "\n");
	Output("Embedded Ruby: v" RUBY_VERSION "\n");

	if( ok != 0 )
	{
		usleep(10000);
		return 1;
	}

	if( help_mode )
	{
		Output("Usage: %s [option(s)]\n", basename(argv[0]));
		Output("--test		enable test mode\n");
		Output("--help		display help\n");
		Output("--hidden	start hidden (windows)\n");
		Output("--config xxx	set dryon.cfg file\n");
		Output("--servers xxx	set servers.txt file\n");
		Output("--userfile xxx	set userfile.txt file\n");
		usleep(10000);
		return 0;
}

	cfg.setFile( configfile_path.c_str() );

	/* some checks on required folders/files */
	if( makeSanityChecks() == 0 )
	{
		bool err= false;
		cfg.readFile();

		#define CHECK_CFG_VAR(X)	if( !cfg.isDefined(#X) ){ Error("variable '" #X "' undefined\n"); err= true; }

		// check if required config directives are set
		CHECK_CFG_VAR(	botname	 );
		CHECK_CFG_VAR(	alt_nick );
		CHECK_CFG_VAR(	realname );
		CHECK_CFG_VAR(	useauth  );
		CHECK_CFG_VAR(	script_01);


		// if one or more vars are undefined then exit now
		// exit also if the compilation of scripts fails
		if( !err &&  (PMgr.loadScriptsFromCfg(cfg) == 0) )
		{
			//// compile/test the scripts /////
			if( test_mode )
			{
				Output("** TEST MODE **\n");
				PMgr.callEvent("event_onTest");
			}
			///////////
			else
			{
				string last_srv= "", srv= "";

				cfg.readFile();
				userfile.readUserFile( userfile_path.c_str() );
				servers.loadFromFile( serverlist_path.c_str() );

				while(1)
				{
					int ret;

					if( cfg.isDefined("debugmode") )
						bot.setIntOption(OPTION_INT_DEBUGMODE, 	cfg.readIntKey("debugmode"));
					else
						bot.setIntOption(OPTION_INT_DEBUGMODE, 	0);

					bot.setIntOption(OPTION_INT_USEAUTH, 	cfg.readBoolKey("useauth")?1:0);
					bot.setStrOption(OPTION_STR_REALNAME, 	cfg.readStringKey("realname"));
					bot.setStrOption(OPTION_STR_ALTNICK,	cfg.readStringKey("alt_nick"));

					srv= servers.getNextServer();
					// if the server is the same than the last one we tried, wait 2min
					// before trying to reconnect to be nice with the server :)
					if( srv == "" )
					{
						Error("No servers in servers.txt, exiting...\n");
						break;
					}
					else if( srv == last_srv )
					{
						Debug("Waiting 2min before reconnect attempt...\n");
						sleep(120);
					}



					// if exceptions occur then they came from the STL since
					// i don't use them
					try
					{
						ret= bot.mainLoop(srv, cfg.readStringKey("botname"));
					}
					catch( exception &e )
					{
						Error("exception: %s\n", e.what());
					}

					last_srv= srv;

					if( ret!=0 )
					{
						Debug("Exiting...\n");
						break;
					}

					PMgr.callEvent("event_onBotDisconnected");
				}
			}
		}
		else
		{
			Output("Errors during loading/compilation\n");
			ret= 1;
		}
	}
	else
	{
		ret= 1;
	}


#if defined(WIN32)
}
#endif

	// i had problems when the bot exited immediatly from an error
	// the thread didn't had time to initialiase before exiting and this resulted
	// in a seg fault, with a small delay it works fine :)
	usleep(10000);

#if defined(WIN32)

WSACleanup();
Output("%sBot exited successfully.%s\n", COLOR_GREEN, COLOR_RESET);
//	console.requestEnd();
//	console.waitEnd();

#endif

	return ret;
}


#if defined(WIN32)

#define NOT_IN_TOKEN                0
#define IN_SINGLE_QUOTED_TOKEN      1
#define IN_DOUBLE_QUOTED_TOKEN      2
#define IN_UNQUOTED_TOKEN           3

int parse_command_line(char* p, char** argv) {
    int state = NOT_IN_TOKEN;
    int argc=0;

    while (*p) {
        switch(state) {
        case NOT_IN_TOKEN:
            if (isspace(*p)) {
            } else if (*p == '\'') {
                p++;
                argv[argc++] = p;
                state = IN_SINGLE_QUOTED_TOKEN;
                break;
            } else if (*p == '\"') {
                p++;
                argv[argc++] = p;
                state = IN_DOUBLE_QUOTED_TOKEN;
                break;
            } else {
                argv[argc++] = p;
                state = IN_UNQUOTED_TOKEN;
            }
            break;
        case IN_SINGLE_QUOTED_TOKEN:
            if (*p == '\'') {
                *p = 0;
                state = NOT_IN_TOKEN;
            }
            break;
        case IN_DOUBLE_QUOTED_TOKEN:
            if (*p == '\"') {
                *p = 0;
                state = NOT_IN_TOKEN;
            }
            break;
        case IN_UNQUOTED_TOKEN:
            if (isspace(*p)) {
                *p = 0;
                state = NOT_IN_TOKEN;
            }
            break;
        }
        p++;
    }
    argv[argc] = 0;
    return argc;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	LPSTR command_line;
	char* argv[100];
	int argc;

	command_line= GetCommandLine();
	argc= parse_command_line( command_line, argv );
	return main(argc, argv);
}
#endif


/**/


