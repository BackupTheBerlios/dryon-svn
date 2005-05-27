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

#ifndef _NATIVES_H
#define _NATIVES_H

#include "amx.h"
#include "dryon.h"

using namespace std;

#if defined(WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

extern char buildString_buffer[1024];

#define AMXERROR(M,V) \
{\
	char *tmp;\
	amx_GetUserData(amx, AMX_USERTAG('F','I','L','E'), (void**)&tmp);\
	if( V==0 )\
		AmxDebug("%s%s (%d) >> " M "%s", COLOR_RED, basename(tmp), amx->curline, COLOR_RESET);\
	else \
		AmxDebug("%s%s (%d) >> " M "%s", COLOR_RED, basename(tmp), amx->curline, V, COLOR_RESET);\
}

#define CHECK(C,R,M,V) if(!(C)){ AMXERROR(M,V); return R;}

#define NATIVE(_FUNC_) cell AMX_NATIVE_CALL _FUNC_(AMX *amx, cell *params)
#define MIN(X,Y) ((X<Y)?X:Y)
#define MAX(X,Y) ((X>Y)?X:Y)
#define IN_RANGE(V,L,H) ((V>=L)&&(V<=H))

void copyStringFromAMX(AMX *amx, cell param, char **out);
void copyStringToAMX(AMX *amx, cell dest, const char *input, int maxsize=-1, int packed= 0);
int buildString(AMX *amx, cell *cstr, cell *params, int num);

/* natives functions registration */
void registerDryonBotNatives(AMX *amx);
void registerNatives_String(AMX *amx);

bool findCharInAMXString(AMX *amx, cell str, char c);


namespace string_array
{
	bool isArrayIDValid(cell id);
	int create();
	bool add(int id, string value);
	void clear(int id);
	string join(int id, string sep, int first= 0);
}

int parseMySQLDateTime(string datetime_str, struct tm &ret, bool parse_time = true);

/////////////////
// BOT Functions

/****h* Small/ScriptCallbacks
* MODULE DESCRIPTION
*	functions called by the engine when events are triggered
*	bot is only aware of the events happening in a channel where it is
*	bot can't receive a join event for user Bob on channel #fgh if it
*	is not on it.
*	Its the basic of IRC but i felt like i had to put it there in case.
***/

/****h* ScriptCallbacks/BotRelated
* MODULE DESCRIPTION
*	callbacks fired by the bot itself,
*	when it joins, leaves, ...
***/

/****f* ScriptCallbacks/event_LoadPlugin
* USAGE
*	public event_LoadPlugin()
* DESCRIPTION
*	Called when plugin is loaded by the engine, you can't
*	be sure that the bot is connected to the IRC server there
*	so don't call any irc_* function here
* SINCE
*	0.3
***/

/****f* ScriptCallbacks/event_UnloadPlugin
* USAGE
*	public event_UnloadPlugin()
* DESCRIPTION
*	Called when plugin is unloaded by the engine, you can't
*	be sure that the bot is connected to the IRC server there
*	so don't call any irc_* function here
* SINCE
*	0.3
***/

/****f* ScriptCallbacks/event_onConnected
* USAGE
*	public event_onConnected()
* DESCRIPTION
*	Called when the bot receives message 001 from server
*	( welcome message )
* SINCE
*	0.1
***/

/****f* ScriptCallbacks/event_onRegistered
* USAGE
*	public event_onRegistered()
* DESCRIPTION
*	Called when confirmation that the connection is
*	registered is received (mode +r)
* SINCE
*	0.2
***/

/****f* ScriptCallbacks/event_onBotExit
* USAGE
*	public event_onBotExit()
* DESCRIPTION
*	called when an exit was requested by a script just before
*	the QUIT message is sent, you can only send small amount of irc commands
*	there: change a topic, send a message on a chan, if you send too much data
*	the bot will shut down the connection before all the commands are sent.
* SINCE
*	0.1
***/

/****f* ScriptCallbacks/event_onJoin
* USAGE
*	public event_onJoin(const nick[], const chan[])
* DESCRIPTION
*	Called when someone join a channel
* OUTPUTS
*	nick : user's nick
*	chan : channel the user joined
* SINCE
*	0.1
***/

/****f* BotRelated/event_onBotJoined
* USAGE
*	public event_onBotJoined(const chan[])
* DESCRIPTION
*	Called when the bot join a channel
* OUTPUTS
*	chan : channel the bot joined
* SINCE
*	0.2
***/

/****f* ScriptCallbacks/event_onPart
* USAGE
*	public event_onPart(const nick[], const chan[])
* DESCRIPTION
*	Called when a user leave a channel
* OUTPUTS
*	nick : user's nick
*	chan : channel name
* SINCE
*	0.1
***/

/****f* BotRelated/event_onBotPart
* USAGE
*	public event_onBotPart(const channel[])
* DESCRIPTION
*	Called when the bot leaves a channel
* OUTPUTS
*	chan : channel name
* SINCE
*	0.1
***/

/****f* ScriptCallbacks/event_onUserListing
* USAGE
*	public event_onUserListing(const chan[], string_array:list)
* DESCRIPTION
*	Called just after a channel is joined by the bot, the IRC
*	server send it the user list of the channel
* OUTPUTS
*	chan : channel name
*	list : users list ( String_Array )
* SINCE
*	0.2
***/

/****f* ScriptCallbacks/event_onQuit
* USAGE
*	public event_onQuit(const nick[])
* DESCRIPTION
*	Called when a user disconnect from the IRC server
*	or close the IRC client
* OUTPUTS
*	nick : user's nick
* SINCE
*	0.1
***/

/****f* ScriptCallbacks/event_onPrivMsg
* USAGE
*	public event_onPrivMsg(const sender[], const dest[], string_array:args)
* DESCRIPTION
*	Called when a user says something on a channel
*	or when a user send a private message to the bot
* OUTPUTS
*	sender : sender nick
*	dest   : bot nick if private message
*		     channel name else
*	args   : text sent ( String_Array )
* SINCE
*	0.2
***/

/****f* ScriptCallbacks/event_onTopicChange
* USAGE
*	public event_onTopicChange(const caller[], const chan[], const old_topic[], const new_topic[])
* DESCRIPTION
*	someone changed/removed the topic on a channel
* OUTPUTS
*	caller    : caller nick
*	chan      : channel name
*	old_topic : the old topic :)
*	new_topic : i let you guess
* INFOS
*	if called in this function %irc_getTopic% will return the new_topic
* SINCE
*	0.1
***/

/****f* ScriptCallbacks/event_onKick
* USAGE
*	public event_onKick(const kicker[], const chan[], const victim[], const kick_msg[])
* DESCRIPTION
*	when a user get kicked
* OUTPUTS
*	kicker   : kicker nick
*	chan     : channel name
*	victim   : nick of user who got kicked
*	kick_msg : reason for the kick
* SINCE
*	0.1
***/

/****f* BotRelated/event_onBotKicked
* USAGE
*	public event_onBotKicked(const kicker[], const chan[], const kick_msg[])
* DESCRIPTION
*	when the bot get kicked
* OUTPUTS
*	kicker   : kicker nick
*	chan     : channel name
*	kick_msg : reason for the kick
* SINCE
*	0.2
***/

/****f* ScriptCallbacks/event_onOp
* USAGE
*	public event_onOp(const chan[], const caller[], const target[])
* DESCRIPTION
*	Called when an user get op on a channel
* OUTPUTS
*	chan   : channel name
*	caller : opper nick
*	target : opped nick
* SINCE
*	0.2
***/

/****f* BotRelated/event_onBotOpped
* USAGE
*	public event_onBotOpped(const chan[], const caller[])
* DESCRIPTION
*	Called when the bot got op by a user
* OUTPUTS
*	chan   : channel name
*	caller : opper nick
* SINCE
*	0.2
***/

/****f* ScriptCallbacks/event_onDeOp
* USAGE
*	public event_onDeOp(const chan[], const caller[], const target[])
* DESCRIPTION
*	Called when someone lost op status on a channel
* OUTPUTS
*	chan   : channel name
*	caller : deopper nick
*	target : nick of the user who lost op status
* SINCE
*	0.2
***/

/****f* BotRelated/event_onBotDeopped
* USAGE
*	public event_onBotDeopped(const chan[], const caller[])
* DESCRIPTION
*	Called when the bot lost op status on a channel
* OUTPUTS
*	chan   : channel name
*	caller : deopper nick
* SINCE
*	0.2
***/

/****f* ScriptCallbacks/event_onVoice
* USAGE
*	public event_onVoice(const chan[], const caller[], const target[])
* DESCRIPTION
*	Called when someone get voice status
* OUTPUTS
*	chan   : channel name
*	caller : nick of the voicer
*	target : nick of the user who got voice status
* SINCE
*	0.2
***/

/****f* BotRelated/event_onBotVoiced
* USAGE
*	public event_onBotVoiced(const chan[], const caller[])
* DESCRIPTION
*	Called when bot get voice status
* OUTPUTS
*	chan   : channel name
*	caller : nick of the voicer
* SINCE
*	0.2
***/

/****f* BotRelated/event_onBotDisconnected
* USAGE
*	public event_onBotDisconnected()
* DESCRIPTION
*	connection with server lost
* SINCE
*	0.5
***/

/****f* ScriptCallbacks/event_onDeVoice
* USAGE
*	public event_onDeVoice(const chan[], const caller[], const target[])
* DESCRIPTION
*	Called when someone lost voice status on a channel
* OUTPUTS
*	chan   : channel name
*	caller : nick of the caller
*	target : nick of the user who lost voice
* SINCE
*	0.2
***/

/****f* BotRelated/event_onBotDevoiced
* USAGE
*	public event_onBotDevoiced(const chan[], const caller[])
* DESCRIPTION
*	Called when the bot lost voice status on a channel
* OUTPUTS
*	chan   : channel name
*	caller : nick of the caller
* SINCE
*	0.2
***/

/****f* ScriptCallbacks/event_onBan
* USAGE
*	public event_onBan(const chan[], const caller[], const banmask[])
* DESCRIPTION
*	Called when someone add a ban on a channel
* OUTPUTS
*	chan    : channel name
*	caller  : nick of the user who set the ban
*	banmask : ban mask (ex: Bob!*@aol.com)
* SINCE
*	0.2
***/

/****f* BotRelated/event_onBotBanned
* USAGE
*	public event_onBotBanned(const chan[], const caller[], const banmask[])
* DESCRIPTION
*	Called when someone add a ban on a channel
*	matching bot address
* OUTPUTS
*	chan    : channel name
*	caller  : nick of the user who set the ban
*	banmask : ban mask (ex: Bob!*@aol.com)
* SINCE
*	0.2
***/

/****f* ScriptCallbacks/event_onUnBan
* USAGE
*	public event_onUnBan(const chan[], const caller[], const banmask[])
* DESCRIPTION
*	Called when someone remove a ban
* OUTPUTS
*	chan    : channel name
*	caller  : nick of the user who removed the ban
*	banmask : ban mask
* SINCE
*	0.2
***/

/****f* BotRelated/event_onBotUnBanned
* USAGE
*	public event_onBotUnBanned(const chan[], const caller[])
* DESCRIPTION
*	Called when someone remove a ban matching bot address
* OUTPUTS
*	chan    : channel name
*	caller  : nick of the user who removed the ban
* SINCE
*	0.2
***/

/****f* ScriptCallbacks/event_onChanKeySet
* USAGE
*	public event_onChanKeySet(const chan[], const caller[], const key[])
* DESCRIPTION
*	Called when someone set a key on a channel
* OUTPUTS
*	chan   : channel name
*	caller : nick of user who set the key
*	key    : the new channel key
* SINCE
*	0.2
***/

/****f* ScriptCallbacks/event_onChanKeyRemoved
* USAGE
*	public event_onChanKeyRemoved(const chan[], const caller[])
* DESCRIPTION
*	Called when someone remove the key from a channel
* OUTPUTS
*	chan   : channel name
*	caller : nick of user who removed the key
* SINCE
*	0.2
***/

/****f* ScriptCallbacks/event_onChanMode
* USAGE
*	public event_onChanMode(const caller[], const chan[], const mode[])
* DESCRIPTION
*	Called when someone set or remove a channel mode
* OUTPUTS
*	caller : sender nick
*	chan   : channel name
*	mode   : mode string (ex: -n , +n)
* SINCE
*	0.6
***/

/****f* ScriptCallbacks/event_onNickChange
* USAGE
*	public event_onNickChange(const old_nick[], const new_nick[])
* DESCRIPTION
*	Called when a user changes nick
* OUTPUTS
*	old_nick : old nick of the user
*	new_nick : actual nick of the user
* SINCE
*	0.2
***/

/****f* ScriptCallbacks/event_onAction
* USAGE
*	public event_onAction(const sender[], const action[])
* DESCRIPTION
*	Called when a user perform an action on a channel ( /me )
* OUTPUTS
*	sender : nick of the user who sent this action
*	action : the action string
* SINCE
*	0.2
***/

/****f* ScriptCallbacks/event_onTest
* USAGE
*	public event_onTest()
* DESCRIPTION
*	Called when bot is in test mode, in this mode
*	the engine will call this event for all the plugins
*	and then exists, no connection to an IRC server is
*	done.
* SINCE
*	0.1
***/

#endif // _NATIVES_H

/**/

