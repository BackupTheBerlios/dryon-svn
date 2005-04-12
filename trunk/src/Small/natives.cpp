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

/*!
\file natives.cpp
*/

#include "config.h"

#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "amx.h"
#include "amxbot.h"
#include "utils.h"
#include "script.h"
#include "userfile.h"
#include "natives.h"
#include "tokens.h"
#include "match.h"

extern AMXBot bot;
extern UserFile userfile;

#if defined(WIN32)
#include "termwin.h"
extern ConsoleThread console;
#endif

//#define SHOWERROR(F, vars...) {char *tmp;amx_GetUserData(amx, AMX_USERTAG('F','I','L','E'), (void**)&tmp);AmxError("%s (%d) >> "##F, basename(tmp), amx->curline, vars);}


/****h* Main/Small
* MODULE DESCRIPTION
*	Small scripting
*
***/

/****h* Small/Natives
* INCLUDE FILE
*	amxbot.inc
* MODULE DESCRIPTION
*	Functions useable in the scripts
*
*	all the <u>maxsize</u> parameters have a default value set
*	you don't need to specify it youself, just use the default value
*
*	It is the length of the array (not the max length of the string in it !), so if you declare
*	an array like this:  <b>new str[20];</b>
*	20 is the size to be given to the function as maxsize for str
*
*	to use the default value for a parameter which is not the last one use '_'
*	<b><u>def</u>: native myfunc(a= 2, b);</b>
*	<b><u>use</u>: myfunc( _, 5);</b>
***/

//////////////////
// CORE Functions
/****h* Small/AMXCore
* INCLUDE FILE
*	amxcore.inc
* MODULE DESCRIPTION
*	Core functions
***/

#define CHARBITS        (8*sizeof(char))

static int verify_addr(AMX *amx,cell addr)
{
	int err;
	cell *cdest;

	err= amx_GetAddr(amx, addr, &cdest);
	if( err!=AMX_ERR_NONE )
		amx_RaiseError(amx,err);

	return err;
}

/****f* AMXCore/numargs
* USAGE
*	numargs()
***/
NATIVE(_numargs)
{
	AMX_HEADER *hdr;
	unsigned char *data;
	cell bytes;

	hdr=(AMX_HEADER *)amx->base;
	data=amx->data ? amx->data : amx->base+(int)hdr->dat;
	/* the number of bytes is on the stack, at "frm + 2*cell" */
	bytes= * (cell *)(data+(int)amx->frm+2*sizeof(cell));
	/* the number of arguments is the number of bytes divided
	* by the size of a cell */
	return bytes/sizeof(cell);
}

/****f* AMXCore/getarg
* USAGE
*	getarg(arg, index=0)
***/
NATIVE(_getarg)
{
	AMX_HEADER *hdr;
	unsigned char *data;
	cell value;

	hdr=(AMX_HEADER *)amx->base;
	data=amx->data ? amx->data : amx->base+(int)hdr->dat;
	/* get the base value */
	value= * (cell *)(data+(int)amx->frm+((int)params[1]+3)*sizeof(cell));
	/* adjust the address in "value" in case of an array access */
	value+=params[2]*sizeof(cell);
	/* get the value indirectly */
	value= * (cell *)(data+(int)value);
	return value;
}

/****f* AMXCore/setarg
* USAGE
*	setarg(arg, index=0, value)
***/
NATIVE(_setarg)
{
	AMX_HEADER *hdr;
	unsigned char *data;
	cell value;

	hdr=(AMX_HEADER *)amx->base;
	data=amx->data ? amx->data : amx->base+(int)hdr->dat;
	/* get the base value */
	value= * (cell *)(data+(int)amx->frm+((int)params[1]+3)*sizeof(cell));
	/* adjust the address in "value" in case of an array access */
	value+=params[2]*sizeof(cell);
	/* verify the address */
	if (value<0 || value>=amx->hea && value<amx->stk)
	return 0;
	/* set the value indirectly */
	* (cell *)(data+(int)value) = params[3];
	return 1;
}

/****f* AMXCore/heapspace
* USAGE
*	heapspace()
***/
NATIVE(_heapspace)
{
	return amx->stk - amx->hea;
}

/****f* AMXCore/strlen
* USAGE
*	strlen(const string[])
* RETURN VALUE
*	length of the string
***/
NATIVE(_strlen)
{
	cell *cptr;
	int len = 0;

	if( amx_GetAddr(amx,params[1],&cptr)==AMX_ERR_NONE )
		amx_StrLen(cptr,&len);
	return len;
}

int amx_StrPack(cell *dest,cell *source)
{
	int len;

	amx_StrLen(source,&len);
	if ((ucell)*source>UNPACKEDMAX) {
	/* source string is already packed */
	while (len >= 0) {
	  *dest++ = *source++;
	  len-=sizeof(cell);
	} /* while */
	} else {
	/* pack string, from bottom up */
	cell c;
	int i;
	for (c=0,i=0; i<len; i++) {
	  assert((*source & ~0xffL)==0);
	  c=(c<<CHARBITS) | *source++;
	  if (i%sizeof(cell) == sizeof(cell)-1) {
		*dest++=c;
		c=0;
	  } /* if */
	} /* for */
	if (i%sizeof(cell) != 0)    /* store remaining packed characters */
	  *dest=c << (sizeof(cell)-i%sizeof(cell))*CHARBITS;
	else
	  *dest=0;                  /* store full cell of zeros */
	} /* if */
	return AMX_ERR_NONE;
}

/****f* AMXCore/strpack
* USAGE
*	strpack(dest[], const source[])
***/
NATIVE(_strpack)
{
	cell *cdest,*csrc;
	int len,needed,err;
	size_t lastaddr;

	/* calculate number of cells needed for (packed) destination */
	amx_GetAddr(amx,params[2],&csrc);
	amx_StrLen(csrc,&len);
	needed=(len+sizeof(cell))/sizeof(cell);     /* # of cells needed */
	assert(needed>0);
	lastaddr=(size_t)(params[1]+sizeof(cell)*needed-1);
	if (verify_addr(amx,(cell)lastaddr)!=AMX_ERR_NONE)
	return 0;

	amx_GetAddr(amx,params[1],&cdest);
	err=amx_StrPack(cdest,csrc);
	if (err!=AMX_ERR_NONE)
	return amx_RaiseError(amx,err);

	return len;
}

int amx_StrUnpack(cell *dest,cell *source)
{
	if ((ucell)*source>UNPACKEDMAX) {
	/* unpack string, from top down (so string can be unpacked in place) */
	cell c;
	int i,len;
	amx_StrLen(source,&len);
	dest[len]=0;
	for (i=len-1; i>=0; i--) {
	  c=source[i/sizeof(cell)] >> (sizeof(cell)-i%sizeof(cell)-1)*CHARBITS;
	  dest[i]=c & UCHAR_MAX;
	} /* for */
	} else {
	/* source string is already unpacked */
	while ((*dest++ = *source++) != 0)
	  /* nothing */;
	} /* if */
	return AMX_ERR_NONE;
}

/****f* AMXCore/strunpack
* USAGE
*	strunpack(dest[], const source[])
***/
NATIVE(_strunpack)
{
	cell *cdest,*csrc;
	int len,err;
	size_t lastaddr;

	/* calculate number of cells needed for (packed) destination */
	amx_GetAddr(amx,params[2],&csrc);
	amx_StrLen(csrc,&len);
	assert(len>=0);
	lastaddr=(size_t)(params[1]+sizeof(cell)*(len+1)-1);
	if (verify_addr(amx,(cell)lastaddr)!=AMX_ERR_NONE)
	return 0;

	amx_GetAddr(amx,params[1],&cdest);
	err=amx_StrUnpack(cdest,csrc);
	if (err!=AMX_ERR_NONE)
	return amx_RaiseError(amx,err);

	return len;
}

/****f* AMXCore/tolower
* USAGE
*	tolower(c)
***/
NATIVE(__tolower)
{
#if defined __WIN32__ || defined _WIN32 || defined WIN32
	return (cell)CharLower((LPTSTR)params[1]);
#elif defined _Windows
	return (cell)AnsiLower((LPSTR)params[1]);
#else
	return tolower((int)params[1]);
#endif
}

/****f* AMXCore/toupper
* USAGE
*	toupper(c)
***/
NATIVE(__toupper)
{
#if defined __WIN32__ || defined _WIN32 || defined WIN32
	return (cell)CharUpper((LPTSTR)params[1]);
#elif defined _Windows
	return (cell)AnsiUpper((LPSTR)params[1]);
#else
	return toupper((int)params[1]);
#endif
}

/****f* AMXCore/random
* USAGE
*	random(max)
* RETURN VALUE
*	integer : random value between 0 and max
***/

static unsigned long IL_StandardRandom_seed = 0L;
#define IL_RMULT 1103515245L

NATIVE(_random)
{
	unsigned long lo, hi, ll, lh, hh, hl;
	unsigned long result;

	/* one-time initialization (or, mostly one-time) */
	#if !defined SN_TARGET_PS2 && !defined _WIN32_WCE
		if (IL_StandardRandom_seed == 0L)
			IL_StandardRandom_seed=(unsigned long)time(NULL);
	#endif

	lo = IL_StandardRandom_seed & 0xffff;
	hi = IL_StandardRandom_seed >> 16;
	IL_StandardRandom_seed = IL_StandardRandom_seed * IL_RMULT + 12345;
	ll = lo * (IL_RMULT  & 0xffff);
	lh = lo * (IL_RMULT >> 16    );
	hl = hi * (IL_RMULT  & 0xffff);
	hh = hi * (IL_RMULT >> 16    );
	result = ((ll + 12345) >> 16) + lh + hl + (hh << 16);
	result &= ~LONG_MIN;        /* remove sign bit */
	if (params[1]!=0)
		result %= params[1];
	return (cell)result;
}

/****f* AMXCore/min
* USAGE
*	min(value1, value2)
* RETURN VALUE
*	integer : return the lowest integer between value1 and value2
***/
NATIVE(_min)
{
	return params[1] <= params[2] ? params[1] : params[2];
}

/****f* AMXCore/max
* USAGE
*	max(value1, value2)
* RETURN VALUE
*	integer : return the biggets integer between value1 and value2
***/
NATIVE(_max)
{
	return params[1] >= params[2] ? params[1] : params[2];
}

/****f* AMXCore/clamp
* USAGE
*	clamp(value, min=cellmin, max=cellmax)
* RETURN VALUE
*	integer : value clamped between min and max
***/
NATIVE(_clamp)
{
	cell value = params[1];
	if (params[2] > params[3])  /* minimum value > maximum value ! */
		amx_RaiseError(amx,AMX_ERR_NATIVE);
	if (value < params[2])
		value = params[2];
	else if (value > params[3])
		value = params[3];
	return value;
}

/****h* Natives/IRCFuncs
* MODULE DESCRIPTION
*	communication with IRC server
***/


/////////////////////////////////////////////////////////////////////////////////
// IRC functions
/////////////////////////////////////////////////////////////////////////////////

/****f* IRCFuncs/getBotNick
* USAGE
*	irc_getBotNick(dest[], dest_maxsize= sizeof dest)
* DESCRIPTION
*	return the current bot nick in dest
* INPUTS
*	dest         : where to store the bot name
*	dest_maxsize : %Natives%
* SINCE
* 0.4
***/
NATIVE(_getBotNick)
{
	string nick= bot.getNick();
	copyStringToAMX(amx, params[1], const_cast<char*>(nick.c_str()), params[2]-1, 0);
	return 0;
}

/****f* IRCFuncs/irc_isChanName
* USAGE
*	bool:irc_isChanName(const channel[])
* RETURN VALUE
*	boolean: true if channel is a channel name
* INPUTS
*	channel : the channel name
* SINCE
*	0.5
***/
NATIVE(_isChanName)
{
	cell ret;
	char *channel;
	copyStringFromAMX(amx, params[1], &channel);

	ret= bot.isChannelName(channel);

	delete [] channel;
	return ret;
}

/****f* IRCFuncs/irc_join
* USAGE
*	irc_join(const channel[], const pass[]= "")
* DESCRIPTION
*	join an irc channel
* INPUTS
*	channel : channel's name
*	pass    : channel's password (default: none)
* SINCE
*	0.1
***/
NATIVE(_join)
{
	char *chan, *key;
	copyStringFromAMX(amx, params[1], &chan);
	copyStringFromAMX(amx, params[2], &key);

	bot.join(chan, key);

	delete [] chan;
	delete [] key;
	return 0;
}

/****f* IRCFuncs/irc_part
* USAGE
*	irc_part(const channel[])
* DESCRIPTION
*	leaves an irc channel
* INPUTS
*	channel :  channel's name
* SINCE
*	0.1
***/
NATIVE(_part)
{
	char *chan;
	copyStringFromAMX(amx, params[1], &chan);

	bot.part(chan);

	delete [] chan;
	return 0;
}

/****f* IRCFuncs/irc_changeNick
* USAGE
*	irc_changeNick(const nick[])
* DESCRIPTION
*	changes bot nick
* INPUTS
*	nick : new nick
* NOTES
*	if the bot reconnects to the the server
*	the nick from the config file is used
* SINCE
*	0.2
***/
NATIVE(_changeNick)
{
	char *nick;
	copyStringFromAMX(amx, params[1], &nick);

	bot.changeNick(nick);

	delete [] nick;
	return 0;
}

/****f* IRCFuncs/irc_getauth
* USAGE
*	irc_getauth(const nick[], auth[], auth_maxsize= sizeof auth)
* DESCRIPTION
*	retrieve the auth of the user if authed
*	for Quakenet, it is the Q login
* INPUTS
*	nick         : target's current nickname
*	auth         : where to store the auth
*	auth_maxsize : %Natives%
***/
NATIVE(_getauth)
{
	char *nick;
	string auth= "";
	copyStringFromAMX(amx, params[1], &nick);

	auth= bot.getAuthOf(nick);

	copyStringToAMX(amx, params[2], const_cast<char*>(auth.c_str()), params[3]-1);

	delete [] nick;
	return 0;
}

/****f* IRCFuncs/irc_getAccount
* USAGE
*	bool:irc_getAccount(const nick[], acct[], acct_maxsize= sizeof acct)
* DESCRIPTION
*	retrieve user's account name
* INPUTS
*	nick         : user's current nickname
*	acct         : where to store the account name
*	acct_maxsize : %Natives%
* SINCE
*	0.1
***/
NATIVE(_getAccount)
{
	char *nick;
	copyStringFromAMX(amx, params[1], &nick);

	user_Info *usr= bot.getUserData(nick);
	if( usr==NULL )
	{
		Error("irc_getAccount: unknown user \"%s\" !\n", nick);
		return 0;
	}

	if( !usr->hasAccount() )
	{
		//Error("irc_getAccount: no match for user %s<%s>\n", nick, usr->host.c_str());
		copyStringToAMX(amx, params[2], "", 5);
		return 0;
	}

	copyStringToAMX(amx, params[2], const_cast<char*>(usr->account_name.c_str()), params[3]-1);

	delete [] nick;
	return 1;
}

/****f* IRCFuncs/irc_getChanKey
* USAGE
*	irc_getChanKey(const channel[], pass[], pass_maxsize= sizeof pass)
* DESCRIPTION
*	retrieve the channel's password if set
* INPUTS
*	channel      : channel's name
*	pass         : where to store the password
*	pass_maxsize : %Natives%
* SINCE
*	0.1
***/
NATIVE(_getChannelPassword)
{
	char *chan;
	copyStringFromAMX(amx, params[1], &chan);

	chan_Info *nfo= bot.getChanData(chan);
	if( nfo )
		copyStringToAMX(amx, params[2], const_cast<char*>(nfo->key.c_str()), params[3]-1);
	else
		Error("irc_getChanKey, chan not found: '%s'\n", chan);

	delete [] chan;
	return 0;
}

/****f* IRCFuncs/irc_action
* USAGE
*	irc_action(const chan[], const str[], ...)
* DESCRIPTION
*	do an action ( like /me on mirc )
* INPUTS
*	chan : channel's name
*	str  : %VarArgFormat%
* SINCE
*	0.1
***/
NATIVE(_action)
{
	char *dest;
	cell *cstr;
	copyStringFromAMX(amx, params[1], &dest);
	amx_GetAddr(amx, params[2], &cstr);

	buildString(amx, cstr, &params[3], (params[0]/sizeof(cell))-2);
	bot.sendMsgTo(dest, MODE_PRIVMSG_ONLY, "\001ACTION %s\001", buildString_buffer);

	delete [] dest;
	return 0;
}

/****f* IRCFuncs/irc_notice
* USAGE
*	irc_notice(const nick[], const txt[], ...)
* DESCRIPTION
*	send a notice to a user
* INPUTS
*	nick : target's nick
*	txt  : %VarArgFormat%
* SINCE
*	0.1
***/
NATIVE(_notice)
{
	char *dest;
	cell *cstr;
	copyStringFromAMX(amx, params[1], &dest);
	amx_GetAddr(amx, params[2], &cstr);

	buildString(amx, cstr, &params[3], (params[0]/sizeof(cell))-2);
	bot.sendMsgTo(dest, MODE_NOTICE, "%s", buildString_buffer);

	delete [] dest;
	return 0;
}

/****f* IRCFuncs/irc_say
* USAGE
*	irc_say(const chan[], const txt[], ...)
* DESCRIPTION
*	send message to a channel
* INPUTS
*	chan : channel(s name
*	txt  : %VarArgFormat%
* NOTES
*	The message is sent to CTCP socket if one is open for this user otherwise
*	it is sent as a private message
* SINCE
*	0.1
***/

/****f* IRCFuncs/irc_privmsg
* USAGE
*	irc_privmsg(const nick[], const txt[], ...)
* DESCRIPTION
*	send private message to a user
* INPUTS
*	nick : target's nick
*	txt  : %VarArgFormat%
* NOTES
*	The message is sent to CTCP socket if one is open for this user otherwise
*	it is sent as a private message
* SINCE
*	0.1
***/
NATIVE(_privmsg)
{
	char *user;
	cell *cstr;
	copyStringFromAMX(amx, params[1], &user);
	amx_GetAddr(amx, params[2], &cstr);


	buildString(amx, cstr, &params[3],(params[0]/sizeof(cell))-2);
	bot.sendMsgTo(user, MODE_PRIVMSG, "%s", buildString_buffer);

	delete [] user;
	return 0;
}

/****f* IRCFuncs/irc_sendRAW
* USAGE
*	irc_sendRAW(const format[], ...)
* DESCRIPTION
*	send the given message directly to server (queued)
* INPUTS
*	format : %VarArgFormat%
* SINCE
*	0.5
***/
NATIVE(_raw)
{
	cell *cstr;
	amx_GetAddr(amx, params[1], &cstr);

	buildString(amx, cstr, &params[2],(params[0]/sizeof(cell))-1);
	bot.sendMessage(SENDTYPE_NORMAL, "%s", buildString_buffer);
	return 0;
}

/****f* Natives/debugPrint
* USAGE
*	debugPrint(const format[], ...)
* DESCRIPTION
*	print a debug message on the console with plugin name
* INPUTS
*	format : %VarArgFormat%
*              You can use colors:
*                 r= red
*                 g= green
*                 b= blue
*                 y= yellow
*                 m= magenta
*                 c= cyan
*
*                 z= reset
* SINCE
*	0.1
***/
NATIVE(_debug)
{
	char *filename;
	cell *cstr;
	amx_GetUserData(amx, AMX_USERTAG('F','I','L','E'), (void**)&filename);
	amx_GetAddr(amx, params[1], &cstr);

	buildString(amx, cstr, &params[2],(params[0]/sizeof(cell))-1);
	Output("[%s] %s", basename(filename), buildString_buffer);
	return 0;
}

/****f* IRCFuncs/irc_setMode
* USAGE
*	irc_setMode(const chan[], const nick[], const modes[])
* DESCRIPTION
*	set user modes on a channel
* INPUTS
*	chan  : channel'name
*	nick  : target's nick
*	modes : mode string (ex: -o+v, +o, +v, ...)
* SINCE
*	0.1
***/
NATIVE(_setMode)
{
	char *chan, *nick, *modes;
	copyStringFromAMX(amx, params[1], &chan);
	copyStringFromAMX(amx, params[2], &nick);
	copyStringFromAMX(amx, params[3], &modes);

	bot.setMode(chan, nick, modes);

	delete [] chan;
	delete [] nick;
	delete [] modes;
	return 0;
}

/****f* IRCFuncs/irc_kick
* USAGE
*	irc_kick(const chan[], const user[], const message[]="")
* DESCRIPTION
*	kick someone from a channel
* INPUTS
*	chan    : channel's name
*	user    : target nick
*	message : kick reason
* SINCE
*	0.5
***/
NATIVE(_kick)
{
	char *chan, *nick, *msg;
	copyStringFromAMX(amx, params[1], &chan);
	copyStringFromAMX(amx, params[2], &nick);
	copyStringFromAMX(amx, params[3], &msg);

	bot.kick(chan, nick, msg);

	delete [] chan;
	delete [] nick;
	delete [] msg;
	return 0;
}

/****f* IRCFuncs/irc_ban
* USAGE
*	irc_ban(const chan[], const banmask[])
* DESCRIPTION
*	ban someone from a channel
* INPUTS
*	chan    : channel's name
*	banmask : ban mask (ex: Bob!*@aol.com)
* SINCE
*	0.1
***/
NATIVE(_ban)
{
	char *chan, *banmask;
	copyStringFromAMX(amx, params[1], &chan);
	copyStringFromAMX(amx, params[2], &banmask);

	bot.ban(chan, banmask);

	delete [] chan;
	delete [] banmask;
	return 0;
}

/****f* IRCFuncs/irc_isInBanList
* USAGE
*	bool:irc_isInBanList(const banmask[], const channel[])
* DESCRIPTION
*	check if the banmask is already registered in th channel banlist
* RETURN VALUE
*	boolean : true if banmask is found in channel banlist
* INPUTS
*	banmask : the mask
*	channel : channel name
* SINCE
*	0.5
***/
NATIVE(_isinbanlist)
{
	cell ret= 0;
	char *banmask, *channel;
	copyStringFromAMX(amx, params[1], &banmask);
	copyStringFromAMX(amx, params[2], &channel);

	if( bot.isInBanList(banmask, channel) )
		ret= 1;

	delete [] banmask;
	delete [] channel;
	return ret;
}

/****f* IRCFuncs/irc_unban
* USAGE
*	irc_unban(const chan[], const banmask[])
* DESCRIPTION
*	unban someone from a channel
* INPUTS
*	chan    : channel's name
*	banmask : ban mask (ex: Bob!*@aol.com)
* SINCE
*	0.1
***/
NATIVE(_unban)
{
	char *chan, *banmask;
	copyStringFromAMX(amx, params[1], &chan);
	copyStringFromAMX(amx, params[2], &banmask);

	bot.unban(chan, banmask);

	delete [] chan;
	delete [] banmask;
	return 0;
}

/****f* IRCFuncs/irc_setUserMode
* USAGE
*	irc_setUserMode(const mode[])
* DESCRIPTION
*	set user modes
* INPUTS
*	mode : the mode string (ex: +x, +i)
* SINCE
*	0.1
***/
NATIVE(_setUserMode)
{
	char *mode;
	copyStringFromAMX(amx, params[1], &mode);

	bot.userMode(bot.getNick(), mode);

	delete [] mode;
	return 0;
}

/****f* IRCFuncs/irc_setChanMode
* USAGE
*	irc_setChanMode(const chan[], const modes[])
* DESCRIPTION
*	set channel modes unrelated to users
* INPUTS
*	chan  : channel's name
*	modes : modes to set (ex: +r, +C)
* SINCE
*	0.1
***/
NATIVE(_setChanMode)
{
	char *chan, *mode;
	copyStringFromAMX(amx, params[1], &chan);
	copyStringFromAMX(amx, params[2], &mode);

	bot.chanMode(chan, mode);

	delete [] chan;
	delete [] mode;
	return 0;
}

/****f* IRCFuncs/irc_getTopic
* USAGE
*	irc_getTopic(const chan[], topic[], topic_maxsize= sizeof topic)
* DESCRIPTION
*	retrieve topic of a channel
* INPUTS
*	chan          : channel's name
*	topic         : where to store the topic
*	topic_maxsize : %Natives%
* SINCE
*	0.1
***/
NATIVE(_gettopic)
{
	char *chan;
	string topic;
	copyStringFromAMX(amx, params[1], &chan);

	topic= bot.getTopic(chan);
	copyStringToAMX(amx, params[2], const_cast<char*>(topic.c_str()), params[3]-1);

	delete [] chan;
	return 0;
}

/****f* IRCFuncs/irc_setTopic
* USAGE
*	irc_setTopic(const chan[], const format[], ...)
* DESCRIPTION
*	set topic of a channel
* INPUTS
*	chan   : channel's name
*	format : %VarArgFormat%
* SINCE
*	0.1
***/
NATIVE(_settopic)
{
	char *chan;
	cell *cstr;
	copyStringFromAMX(amx, params[1], &chan);
	amx_GetAddr(amx, params[2], &cstr);

	buildString(amx, cstr, &params[3],(params[0]/sizeof(cell))-2);
	bot.setTopic(chan, buildString_buffer);

	delete [] chan;
	return 0;
}

/****f* IRCFuncs/irc_quit
* USAGE
*	irc_quit(const msg[]= "")
* DESCRIPTION
*	quit the bot
* INPUTS
*	msg : reason for the exit (displayed by the server)
* NOTES
*	this function should only be used in the main plugin
*	for obvious reasons
* SINCE
*	0.1
***/
NATIVE(_quit)
{
	char *msg;
	copyStringFromAMX(amx, params[1], &msg);

	bot.quit(msg);

	delete [] msg;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// tests
/////////////////////////////////////////////////////////////////////////////////

/****f* IRCFuncs/irc_isop
* USAGE
*	bool:irc_isop(const nick[], const chan[])
* DESCRIPTION
*	is the user op on this channel ?
* RETURN VALUE
*	bool : true if op
* INPUTS
*	nick : target's current nick
*	chan : channel
* SINCE
*	0.1
***/
NATIVE(_isop)
{
	cell ret= 0;
	char *nick, *chan;
	copyStringFromAMX(amx, params[1], &nick);
	copyStringFromAMX(amx, params[2], &chan);

	if( nick!=bot.getNick() )
		ret= bot.isOp(nick, chan);

	delete [] chan;
	delete [] nick;
	return ret;
}

/****f* IRCFuncs/irc_isBanned
* USAGE
*	bool:irc_isBanned(const what[], const channel[])
* DESCRIPTION
*	check if a user/mask is banned on channel
* RETURN VALUE
*	boolean : true if the user/mask if banned from channel
* INPUTS
*	what    : can be a nick or a mask
*	channel : channel name
* SINCE
*	0.5
***/
NATIVE(_isban)
{
	cell ret= 0;
	char *what, *channel;
	copyStringFromAMX(amx, params[1], &what);
	copyStringFromAMX(amx, params[2], &channel);

	user_Info *usr= bot.getUserData(what);
	if( usr!=NULL )	// what is a valid nickname
	{
		if( bot.isBanned(usr->full_host, channel) )
			ret= 1;
	}
	// consider that what is a hostname
	else
	{
		if( bot.isBanned(what, channel) )
			ret= 1;
	}

	delete [] what;
	delete [] channel;
	return ret;
}

/****f* IRCFuncs/irc_isbotopped
* USAGE
*	bool:irc_isbotopped(const chan[])
* DESCRIPTION
*	is the bot op on this channel ?
* RETURN VALUE
*	bool : true if op
* INPUTS
*	chan : channel
* SINCE
*	0.2
***/
NATIVE(_isbotopped)
{
	cell ret;
	char *chan;
	copyStringFromAMX(amx, params[1], &chan);

	ret= bot.isBotOp(chan);

	delete [] chan;
	return ret;
}

/****f* IRCFuncs/irc_isvoice
* USAGE
*	bool:irc_isvoice(const nick[], const chan[])
* DESCRIPTION
*	is the user voice on this channel ?
* RETURN VALUE
*	bool : true if voice
* INPUTS
*	nick : target's current nick
*	chan : channel
* SINCE
*	0.1
***/
NATIVE(_isvoice)
{
	cell ret;
	char *nick, *chan;
	copyStringFromAMX(amx, params[1], &nick);
	copyStringFromAMX(amx, params[2], &chan);

	ret= bot.isVoice(nick, chan);

	delete [] chan;
	delete [] nick;
	return ret;
}

/****c* Natives/SaveLoad
* MODULE DESCRIPTION
*	functions used to save/load plugin data
***/
namespace data_save_load
{

/****m* SaveLoad/loadInt
* USAGE
*	loadInt(var)
* MACRO
*	#define loadInt(%1) loadIntEx("%1", %1, 1)
* DESCRIPTION
*	load integer from data file
* SINCE
*	0.1
***/

/****m* SaveLoad/loadIntArray
* USAGE
*	loadIntArray(var[])
* MACRO
*	#define loadIntArray(%1) loadIntArrayEx("%1", %1, sizeof %1)
* DESCRIPTION
*	load integer array from data file
* SINCE
*	0.1
***/

/****f* SaveLoad/loadIntEx
* USAGE
*	loadIntEx(const name[], &var, size= 1)
* INPUTS
*	name : name of the integer in the save file
*	var  : pointer to the amx variable
*	size : size of the array
*
* NOTES
*	You can use the helper macros %loadIntArray% and %loadInt% to load
*	a variable from file when plugin is loaded
* SINCE
*	0.1
***/

/****f* SaveLoad/loadIntArrayEx
* USAGE
*	loadIntArrayEx(const name[], var[], size= sizeof var)
* INPUTS
*	name : name of the array in the save file
*	var  : pointer to the amx variable
*	size : size of the array
*
* NOTES
*	You can use the helper macros %loadIntArray% and %loadInt% to load
*	a variable from file when plugin is loaded
* SINCE
*	0.1
***/
	NATIVE(_loadIntArray)
	{
		cell *addr;
		DataFile *file;
		char *name;

		CHECK(!findCharInAMXString(amx, params[1], ' '), -1, "loadIntArray: name cant contains space(s)\n", 0);

		amx_GetUserData(amx, AMX_USERTAG('S','A','V','E'), (void**)&file);
		amx_GetAddr(amx, params[2], &addr);
		copyStringFromAMX(amx, params[1], &name);

		// if no saved state exist dont change variable
		if( file->isDefined(name) )
		{
			vector<string> numbers;
			Tokenize(file->readStringKey(name), numbers, " ");

			for( uint i= 0; i< MIN(numbers.size(), (uint)params[3]); i++ )
				addr[i]= atoi(numbers[i].c_str());

		}

		delete [] name;
		return 0;
	}

/****m* SaveLoad/saveInt
* USAGE
*	saveInt(var)
* MACRO
*	#define saveInt(%1) saveIntEx("%1", %1, 1)
* DESCRIPTION
*	save integer array to data file
* SINCE
*	0.1
***/

/****m* SaveLoad/saveIntArray
* USAGE
*	saveIntArray(var[])
* MACRO
*	#define saveIntArray(%1) saveIntArrayEx("%1", %1, sizeof %1)
* DESCRIPTION
*	save integer array to data file
* SINCE
*	0.1
***/

/****f* SaveLoad/saveIntEx
* USAGE
*	saveIntEx(const name[], &val, size= 1)
* INPUTS
*	name : name of the integer in the save file
*	val  : pointer to the amx variable
*	size : size of the array
*
* NOTES
*	You can use the helper macros %saveIntArray% and %saveInt% to save
*	a variable to data file when plugin is unloaded
* SINCE
*	0.1
***/

/****f* SaveLoad/saveIntArrayEx
* USAGE
*	saveIntArrayEx(const name[], const val[], size= sizeof val)
* INPUTS
*	name : name of the array in the save file
*	val  : pointer to the amx variable
*	size : size of the array
*
* NOTES
*	You can use the helper macros %saveIntArray% and %saveInt% to save
*	a variable to data file when plugin is unloaded
* SINCE
*	0.1
***/
	NATIVE(_saveIntArray)
	{
		char conv[20];
		cell *addr;
		char *name;
		string tmp;
		DataFile *file;

		CHECK(!findCharInAMXString(amx, params[1], ' '), -1, "saveIntArray: name cant contains space(s)\n", 0);

		amx_GetUserData(amx, AMX_USERTAG('S','A','V','E'), (void**)&file);
		amx_GetAddr(amx, params[2], &addr);
		copyStringFromAMX(amx, params[1], &name);

		for(int i= 0; i< params[3]; i++)
		{
			snprintf(conv, sizeof(conv), "%d", addr[i]);
			if( i!=0 )
				tmp.append(" ");
			tmp.append(conv);
		}

		file->setStringKey(name, tmp);
		delete [] name;
		return 0;
	}

/****f* SaveLoad/loadBool
* USAGE
*	loadBool(const name[], &bool:val)
* INPUTS
*	name : name of the array in the save file
*	val  : pointer to the amx variable
* SINCE
*	0.8
***/
	NATIVE(_loadBool)
	{
		cell *addr;
		DataFile *file;
		char *name;

		CHECK(!findCharInAMXString(amx, params[1], ' '), -1, "loadBool: name cant contains space(s)\n", 0);

		amx_GetUserData(amx, AMX_USERTAG('S','A','V','E'), (void**)&file);
		amx_GetAddr(amx, params[2], &addr);
		copyStringFromAMX(amx, params[1], &name);

		// if no saved state exist dont change variable
		if( file->isDefined(name) )
		{
			if( file->readBoolKey(name) )
				*addr= 1;
			else
				*addr= 0;
		}

		delete [] name;
		return 0;
	}

/****f* SaveLoad/saveBool
* USAGE
*	saveBool(const name[], const bool:val)
* INPUTS
*	name : name of the array in the save file
*	val  : pointer to the amx variable
* SINCE
*	0.8
***/
	NATIVE(_saveBool)
	{
		char *name;
		string tmp;
		DataFile *file;
		cell &val= params[2];

		CHECK(!findCharInAMXString(amx, params[1], ' '), -1, "saveBool: name cant contains space(s)\n", 0);

		amx_GetUserData(amx, AMX_USERTAG('S','A','V','E'), (void**)&file);
		copyStringFromAMX(amx, params[1], &name);

		if( val==0 )
			file->setStringKey(name, "false");
		else
			file->setStringKey(name, "true");


		delete [] name;
		return 0;
	}

/****m* SaveLoad/loadString
* USAGE
*	loadString(var[])
* MACRO
*	#define loadString(%1) loadStringEx("%1", %1, sizeof %1)
* DESCRIPTION
*	load a string from data file
* SINCE
*	0.1
***/

/****f* SaveLoad/loadStringEx
* USAGE
*	loadStringEx(const name[], data[], data_maxsize= sizeof data)
* DESCRIPTION
*	load a string from data file
* INPUTS
*	name         : name of the string in data file
*	data         : where to store the string
*	data_maxsize : %Natives%
* NOTES
*	You can use the helper macro %loadString% to load
*	a variable from data file when plugin is loaded
* SINCE
*	0.1
***/
	NATIVE(_loadString)
	{
		string str;
		char *name;
		DataFile *file;

		CHECK(!findCharInAMXString(amx, params[1], ' '), -1, "loadString: name cant contains space(s)\n", 0);

		amx_GetUserData(amx, AMX_USERTAG('S','A','V','E'), (void**)&file);
		copyStringFromAMX(amx, params[1], &name);

		if( file->isDefined(name) )
		{
			str= file->readStringKey(name);
			copyStringToAMX(amx, params[2], const_cast<char*>(str.c_str()), params[3]-1);
		}

		delete [] name;
		return 0;
	}

/****m* SaveLoad/saveString
* USAGE
*	saveString(var[])
* MACRO
*	#define saveString(%1) saveStringEx("%1", %1)
* DESCRIPTION
*	save a string to data file
* SINCE
*	0.1
***/

/****f* SaveLoad/saveStringEx
* USAGE
*	saveStringEx(const name[], const data[])
* DESCRIPTION
*	save a string to data file
* INPUTS
*	name         : name of the string in data file
*	data         : string to save
* NOTES
*	You can use the helper macro %saveString% to save
*	a variable to data file when plugin is unloaded
* SINCE
*	0.1
***/
	NATIVE(_saveString)
	{
		char *name, *str;
		DataFile *file;

		CHECK(!findCharInAMXString(amx, params[1], ' '), -1, "loadIntArray: name cant contains space(s)\n", 0);

		amx_GetUserData(amx, AMX_USERTAG('S','A','V','E'), (void**)&file);
		copyStringFromAMX(amx, params[1], &name);
		copyStringFromAMX(amx, params[2], &str);

		file->setStringKey(name, str);

		delete [] str;
		delete [] name;
		return 0;
	}

/****f* SaveLoad/loadStringArray
* USAGE
*	loadStringArray(const name[], id)
* DESCRIPTION
*	load a string array from data file
* INPUTS
*	name : name of the string array in data file
*	id   : identifier of a valid array ( returned by %array_create% )
* SINCE
*	0.1
***/
	// un||deux||trois
	NATIVE(_loadStringArray)
	{
		char *name;
		DataFile *file;

		CHECK(!findCharInAMXString(amx, params[1], ' '), -1, "loadStringArray: name cant contains space(s)\n", 0);
		CHECK(string_array::isArrayIDValid(params[2]), -1, "loadStringArray: id out of range: %d\n", params[2]);

		amx_GetUserData(amx, AMX_USERTAG('S','A','V','E'), (void**)&file);
		copyStringFromAMX(amx, params[1], &name);

		if( file->isDefined(name) )
		{
			vector<string> parts;
			TokenizeWithStr(file->readStringKey(name), parts, "||");

			for( uint i= 0; i< parts.size(); i++ )
				string_array::add(params[2], parts[i]);
		}

		delete [] name;
		return 0;
	}

/****f* SaveLoad/saveStringArray
* USAGE
*	saveStringArray(const name[], id)
* DESCRIPTION
*	save a string array to data file
* INPUTS
*	name : name of the string array in data file
*	id   : identifier of a valid array ( returned by %array_create% )
* SINCE
*	0.1
***/
	NATIVE(_saveStringArray)
	{
		string str;
		char *name;
		DataFile *file;

		CHECK(!findCharInAMXString(amx, params[1], ' '), -1, "saveStringArray: name cant contains space(s)\n", 0);
		CHECK(string_array::isArrayIDValid(params[2]), -1, "saveStringArray: id out of range: %d\n", params[2]);

		amx_GetUserData(amx, AMX_USERTAG('S','A','V','E'), (void**)&file);
		copyStringFromAMX(amx, params[1], &name);

		str= string_array::join(params[2], "||");
		file->setStringKey(name, str);

		delete [] name;
		return 0;
	}
};


/****c* Natives/TimeDateFuncs
* MODULE DESCRIPTION
*	date/time manipulation
***/
namespace TimeDate
{

/****f* TimeDateFuncs/getDateDiff
* USAGE
*	getDateDiff(date:timestamp1, date:timestamp2, type)
* RETURN VALUE
*	integer : difference between the two timestamp
* INPUTS
*	timestamp1 : timestamp for the newer date
*	timestamp2 : timestamp for the older date
*	type       : return type
*				 's' = return diff in seconds
*			     'm' = return diff in minutes
*			     'h' = return diff in hours
*			     'd' = return diff in days
* SINCE
*	0.1
***/
	NATIVE(_getdiff)
	{
		time_t t1= params[1];
		time_t t2= params[2];

		double diff= difftime(t1, t2);

		switch(params[3])
		{
		case 's': return (cell)floor(diff); break;
		case 'm': return (cell)floor(diff/60); break;
		case 'h': return (cell)floor(diff/(60*60)); break;
		case 'd': return (cell)floor(diff/(60*60*24)); break;

		default:
			return (cell)diff;
		}
	}

/****f* TimeDateFuncs/makeDate
* USAGE
*	date:makeDate(const datestr[], bool:parse_time= true)
* DESCRIPTION
*	build a timestamp from  date in MySQL format
* INPUTS
*	datestr    : datetime in MySQL format (YYY-MM-DD HH:MM:SS)
*	parse_time : if false the HH:MM:SS is ignored in datestr and
*				 00:00:00 is used instead to generate the timestamp
* SINCE
*	0.1
***/
	NATIVE(_makeDate)
	{
		char *datestr;
		struct tm tm_s;
		bool parse_time= true;
		copyStringFromAMX(amx, params[1], &datestr);

		if( params[2]==0 )
			parse_time= false;

		parseMySQLDateTime(datestr, tm_s, parse_time);

		delete [] datestr;
		return mktime(&tm_s);
	}

/****f* TimeDateFuncs/getCurrentDate
* USAGE
*	date:getCurrentDate(bool:with_time= true)
* RETURN VALUE
*	return timestamp for current datetime
* INPUTS
*	with_time : if false the HH:MM:SS part is set to 00:00:00
* SINCE
*	0.1
***/
	NATIVE(_getCurrentDate)
	{
		time_t now= time(NULL);
		struct tm *tm_s;

		if( params[1]==1 )
			return now;

		tm_s= localtime(&now);
		tm_s->tm_hour= 0;
		tm_s->tm_min= 0;
		tm_s->tm_sec= 0;

		return mktime(tm_s);
	}
};

/****c* Natives/MaskFuncs
* MODULE DESCRIPTION
*	mask related
***/

/****f* MaskFuncs/mask_match
* USAGE
*	bool:mask_match(const nick[], const mask[])
* DESCRIPTION
*	check if user hostname match mask
* RETURN VALUE
*	bool : true if user hostname match mask
* INPUTS
*	nick : user nick
*	mask : mask
* SINCE
*	0.6
***/
	NATIVE(_mask_match)
	{
		cell ret= 0;
		char *nick, *mask;
		copyStringFromAMX(amx, params[1], &nick);
		copyStringFromAMX(amx, params[2], &mask);

		user_Info *usr= bot.getUserData(nick);

		if( wc_match(mask, usr->full_host) )
			ret= 1;

		delete [] nick;
		delete [] mask;
		return ret;
	}

/****f* MaskFuncs/mask_getlist
* USAGE
*	string_array:mask_getlist(const channel[], const mask[])
* DESCRIPTION
*	return a list of all the user who match mask
* RETURN VALUE
*	string array : %String_Array%
* INPUTS
*	channel : channel name
*	mask    : the mask
* SINCE
*	0.6
***/
	NATIVE(_mask_getlist)
	{
		char *channel, *mask;
		int id= string_array::create();
		CHECK(id!=-1, -1, "mask_getlist: couldn't create string array, maybe they are all used..\n", 0);
		copyStringFromAMX(amx, params[1], &channel);
		copyStringFromAMX(amx, params[2], &mask);

		vector<user_Info*> ulist;
		bot.getUserlistFromMask(mask, channel, ulist);
		for(uint i= 0; i< ulist.size(); i++)
			string_array::add(id, ulist[i]->nick);

		delete [] mask;
		delete [] channel;
		return id;
	}

/****f* MaskFuncs/mask_get
* USAGE
*	mask_get(const nick[], format, out[], out_maxsize= sizeof out)
* DESCRIPTION
*	simply return the mask for nick
* INPUTS
*	nick        : user nick
*	format      :	full hostname: <nick>!<ident>@<host>
*					1 = return the full hostname
*			 		2 = return the host part only
*			 		3 = return the ident part only
*	out         : where to hostname will be stored
*	out_maxsize : %Natives%
* SINCE
*	0.6
***/
	NATIVE(_mask_get)
	{
		char *nick;
		cell format= params[2];
		copyStringFromAMX(amx, params[1], &nick);

		user_Info *usr= bot.getUserData(nick);

		switch(format)
		{
		// full hostname
		case 1: copyStringToAMX(amx, params[3], usr->full_host.c_str(), params[4]-1);
				break;

		// host only
		case 2: copyStringToAMX(amx, params[3], usr->host.c_str(), params[4]-1);
				break;

		// ident only
		case 3: copyStringToAMX(amx, params[3], usr->ident.c_str(), params[4]-1);
				break;

		default:
			AMXERROR("mask_get: unknow format(%s)\n", format);
			break;
		}

		delete [] nick;
		return 0;
	}


extern ScriptManager PMgr;

/****c* Natives/MiscFuncs
* MODULE DESCRIPTION
*	-
***/
namespace Misc
{

/****f* MiscFuncs/bot_version
* USAGE
*	bot_version(str[], str_maxsize= sizeof str)
* DESCRIPTION
*	return the bot version in str
* INPUTS
*	str         : where to store the version number
*	str_maxsize : %Natives%
* SINCE
*	0.6
***/
	NATIVE(_bot_version)
	{
		copyStringToAMX(amx, params[1], VERSION, params[2]);
		return 0;
	}

/****f* MiscFuncs/registerCommand
* USAGE
*	registerCommand(const cmd[], const func[], required_flag, const usage[]= "")
* DESCRIPTION
*	declares a new bot command
* INPUTS
*	cmd           : command that will trigger the function call (ex: "!add")
*	func          : name of the function to call
*	required_flag : function will only be called if user has this flag,
*					use '*' for public commands
*	usage         : used when displaying help ( !help command )
* SINCE
*	0.1
***/
	NATIVE(_registerCommand)
	{
		Script *script;
		char *name, *func_name, *usage;
		copyStringFromAMX(amx, params[1], &name);
		copyStringFromAMX(amx, params[2], &func_name);
		copyStringFromAMX(amx, params[4], &usage);

		amx_GetUserData(amx, AMX_USERTAG('P','L','U','G'), (void**)&script);

		script->registerCommand(name, func_name, (char)params[3], usage);

		delete [] name;
		delete [] func_name;
		delete [] usage;
		return 0;
	}

/****f* MiscFuncs/disablePlugin
* USAGE
*	disablePlugin()
* DESCRIPTION
*	deactivate the plugin, plugin will remain loaded but
*	wont answer to any event.
*	You can use this to disable the plugin if an unrecoverable error
*	happens (ex: unable to connect to database)
* SINCE
*	0.7
***/
	NATIVE(_disable_plugin)
	{
		Script *script;
		amx_GetUserData(amx, AMX_USERTAG('P','L','U','G'), (void**)&script);

		script->disablePlugin();
		AMXERROR("Script disabled itself.\n", 0);

		return 0;
	}

/****f* MiscFuncs/setStatusText
* USAGE
*	setStatusText(const txt[])
* DESCRIPTION
*	set status bar text (windows only)
*	do nothing on other os
* SINCE
*	0.8
***/
	NATIVE(_setStatusText)
	{
#if defined(WIN32)
		char *str;
		copyStringFromAMX(amx, params[1], &str);

		console.setStatusBarTextFromExt(0, str);

		delete [] str;
#endif
		return 0;
	}

/****f* MiscFuncs/addUser
* USAGE
*	addUser(const name[], const what[], const flags[])
* INPUTS
*	name: account name
*	what: hostmask to match or auth preceded by #
*	flags: flags for this account
* SINCE
*	0.8
***/
	NATIVE(_addAccess)
	{
		char *name, *what, *flags;
		cell ret;
		copyStringFromAMX(amx, params[1], &name);
		copyStringFromAMX(amx, params[2], &what);
		copyStringFromAMX(amx, params[3], &flags);

		ret= userfile.addUserAccount(name, what, flags);
		bot.updateFlags(what);

		delete [] name;
		delete [] what;
		delete [] flags;
		return ret;
	}

/****f* MiscFuncs/delUser
* USAGE
*	delUser(const what[])
* INPUTS
*	what: account name
* SINCE
*	0.8
***/
	NATIVE(_delAccess)
	{
		char *what;
		copyStringFromAMX(amx, params[1], &what);

		userfile.delUserAccount(what);
		bot.updateFlags(what);

		delete [] what;
		return 0;
	}

/****f* MiscFuncs/editUser
* USAGE
*	editUser(const user[], const flags[])
* INPUTS
*	what: account name
*	flags: new global flags for user
* SINCE
*	0.8
***/
	NATIVE(_editAccess)
	{
		cell ret;
		char *what, *flags;
		copyStringFromAMX(amx, params[1], &what);
		copyStringFromAMX(amx, params[2], &flags);

		ret= userfile.setAccessFlags(what, "_global_", flags);
		bot.updateFlags(what);

		delete [] what;
		delete [] flags;
		return ret;
	}
};

AMX_NATIVE_INFO amxbot_natives[]= {
/*  misc  */
{"debugPrint", 		_debug},
{"registerCommand",	Misc::_registerCommand},
{"bot_version",		Misc::_bot_version},
{"disablePlugin",	Misc::_disable_plugin},
{"setStatusText",	Misc::_setStatusText},
{"addUser", 		Misc::_addAccess},
{"editUser",		Misc::_editAccess},
{"delUser",			Misc::_delAccess},

/* AMX Core */
{"numargs",			_numargs},
{"getarg",			_getarg},
{"setarg",			_setarg},
{"heapspace",		_heapspace},
{"strlen",			_strlen},
{"strpack",			_strpack},
{"strunpack",		_strunpack},
{"tolower",			__tolower},
{"toupper",			__toupper},
{"random",			_random},
{"min",				_min},
{"max",				_max},
{"clamp",			_clamp},

/* mask functions */
{"mask_match",		_mask_match},
{"mask_get",		_mask_get},
{"mask_getlist",	_mask_getlist},

/* IRC functions */
{"irc_getBotNick",	_getBotNick},
{"irc_isChanName",	_isChanName},
{"irc_join", 		_join},
{"irc_part", 		_part},
{"irc_changeNick",	_changeNick},
{"irc_getauth",		_getauth},
{"irc_getAccount",	_getAccount},
{"irc_getChanKey",	_getChannelPassword},
{"irc_action",		_action},
{"irc_say", 		_privmsg},
{"irc_notice", 		_notice},
{"irc_privmsg", 	_privmsg},
{"irc_kick",		_kick},
{"irc_ban",			_ban},
{"irc_unban",		_unban},
{"irc_setMode",		_setMode},
{"irc_setUserMode",	_setUserMode},
{"irc_setChanMode",	_setChanMode},
{"irc_setTopic", 	_settopic},
{"irc_getTopic", 	_gettopic},
{"irc_quit", 		_quit},
{"irc_sendRAW",		_raw},
{"irc_isInBanList",	_isinbanlist},

/* IRC tests */
{"irc_isop", 		_isop},
{"irc_isvoice", 	_isvoice},
{"irc_isbotopped",	_isbotopped},

/* time/date */
{"makeDate", 		TimeDate::_makeDate},
{"getDateDiff",		TimeDate::_getdiff},
{"getCurrentDate",	TimeDate::_getCurrentDate},

/* saving/loading data */
{"saveIntEx", 		data_save_load::_saveIntArray},
{"saveIntArrayEx", 	data_save_load::_saveIntArray},
{"saveStringEx",	data_save_load::_saveString},
{"saveStringArray",	data_save_load::_saveStringArray},
{"saveBool", 		data_save_load::_saveBool},

{"loadIntEx", 		data_save_load::_loadIntArray},
{"loadIntArrayEx",	data_save_load::_loadIntArray},
{"loadStringEx",	data_save_load::_loadString},
{"loadStringArray",	data_save_load::_loadStringArray},
{"loadBool", 		data_save_load::_loadBool},
/* terminator */
{NULL, NULL}
};

void registerAMXbotNatives(AMX *amx)
{
	amx_Register(amx, amxbot_natives, -1);
	registerNatives_String(amx);
}


