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

#include <exception>
#include <string>
#include "ruby.h"
#include "ruby_embed.h"
#include "ruby_script.h"
#include "log.h"
#include "dryon.h"
#include "tokens.h"
#include "userfile.h"

extern DryonBot bot;
extern UserFile userfile;


extern "C"
{
	int ruby_exec();
};

class RubyError : public exception {};

/* function used only internally */

// awful hack for logging system (really need to find a better way :x)
static RubyScript *current_script= NULL;


/****h* Main/Ruby
* MODULE DESCRIPTION
*	Ruby scripting
*
***/

//********************************
//********************************
// RubyScript object

/****v* Ruby/$plugin_class
* DESCRIPTION
*	Must be initialised with the name of the main plugin class
*	when the script is run
* TYPE
*	read / write
***/

/****o* Ruby/RubyScript
* MODULE DESCRIPTION
*	Main object, all script have to derivates their main class
*	from it in order to be loaded
*
***/


bool RubyScript_isMe(RubyScript *r, user_Info *usr){ return (usr==bot.getBotUser()); }

/****f* RubyScript/registerCommand
* USAGE
*	registerCommand(STRING cmd, STRING func, CHAR flag, STRING usage)
* INPUTS
*	cmd: what to write on irc to call the function
*	func: name of the function in the script
*	flag: required flag to be able to use it
*	usage: help msg
* SINCE
*	0.8
***/
void RubyScript_registerCommand(RubyScript *r, string cmd, string func, char flag, string usage= "")
{
	r->registerCommand(cmd, func, flag, usage);
}

// check data type
static bool isInteger(const char *str)
{
	bool ret= true;
	for(uint i= 0; ret && (i< strlen(str)); i++)
		ret= isdigit((int)str[i]);

	return ret;
}

static bool isFloat(const char *str)
{
	bool ret= true;
	int colons= 0;

	for(uint i= 0; ret && (i< strlen(str)); i++)
	{
		if( str[i] == '.' ) colons++;
		ret= isdigit((int)str[i]) || (str[i]=='.');
	}

	if( colons > 1 )
		ret= false;

	return ret;
}

static bool isBool(const char *str)
{
	return !strcasecmp(str, "true") || !strcasecmp(str, "false");
}

// return value
static bool getBool(const char *str)
{
	return !strcasecmp(str, "true");
}

static char getType(const char *str)
{
	if( isInteger(str) ) 	return 'i';
	else if( isFloat(str) )	return 'f';
	else if( isBool(str) )	return 'b';
	else					return 's';
}

/****f* RubyScript/loadSimpleVar
* USAGE
*	loadSimpleVar(STRING name)
* DESCRIPTION
*	load a variable from config file
* RETURN VALUE
*	depends on the value read
*	currently supported type are: STRING, INTEGER, FLOAT and BOOLEAN
* INPUTS
*	name: variable name in config file
* SINCE
*	0.8
***/
VALUE RubyScript_loadSimpleVar(RubyScript *r, char *name)
{
	VALUE ret= Qnil;
	DataFile *file= &r->savedData;

	if( file->isDefined(name) )
	{
		const char *tmp= file->readStringKey(name).c_str();

		switch( getType(tmp) )
		{
		case 'i': ret= INT2NUM( atoi(tmp) ); 		break;
		case 'f': ret= rb_float_new( atof(tmp) );	break;
		case 'b': ret= (getBool(tmp))?Qtrue:Qfalse;	break;
		case 's': ret= rb_str_new2(tmp);			break;
		}
	}

	return ret;
}


/****f* RubyScript/saveSimpleVar
* USAGE
*	saveSimpleVar(STRING name, val)
* DESCRIPTION
*	save a variable to config file
* INPUTS
*	name: variable name in config file
*	val: variable to save (string, integer, float or boolean)
* SINCE
*	0.8
***/
void RubyScript_saveSimpleVar(RubyScript *r, char *name, VALUE val)
{
	DataFile *file= &r->savedData;
	char buff[200]= "";

	switch( TYPE(val) )
	{
	case T_FIXNUM: snprintf(buff, sizeof(buff)-1, "%d", NUM2INT(val));			break;
	case T_FLOAT: snprintf(buff, sizeof(buff)-1, "%f", NUM2DBL(val));			break;
	case T_TRUE: strcpy(buff, "true");											break;
	case T_FALSE: strcpy(buff, "false");										break;
	case T_STRING: snprintf(buff, sizeof(buff)-1, "%s", StringValueCStr(val));	break;
	default:
		{
			const char *obj_name= rb_class2name( rb_obj_class(val) );
			Error("saveSimpleVar: unhandled data type (%s)\n", obj_name);
			return;
		}
	}

	file->setStringKey(name, buff);
}

/*
VALUE RubyScript_loadArrayVar(RubyScript *r, char *name)
{
	DataFile *file= &r->savedData;
	VALUE ret= rb_ary_new();

	if( file->isDefined(name) )
	{
		vector<string> parts;
		TokenizeWithStr(file->readStringKey(name), parts, "||");

		for( uint i= 0; i< parts.size(); i++ )
			rb_ary_push(ret, rb_str_new2(parts[i].c_str()));
	}

	return ret;
}
*/

/****v* RubyScript/nick
* DESCRIPTION
*	variable holding the nick of the bot
* TYPE
*	read only
* SINCE
*	0.8
***/
string RubyScript_nick_get(RubyScript *){ return bot.getNick(); }

/****v* RubyScript/max_nicklen
* DESCRIPTION
*	variable holding the max size for a nickname (from server)
* TYPE
*	read only
* SINCE
*	0.8
***/
int RubyScript_max_nicklen_get(RubyScript*){ return bot.getMaxNickLen(); }

/****v* RubyScript/max_topiclen
* DESCRIPTION
*	variable holding the max size for a topic (from server)
* TYPE
*	read only
* SINCE
*	0.8
***/
int RubyScript_max_topiclen_get(RubyScript*){ return bot.getMaxTopicLen(); }

/****f* RubyScript/getChannelList
* USAGE
*	getChannelList()
* RETURN VALUE
*	array: return an array containing the list of channels
*		   the bot is currently on
* SINCE
*	0.8
***/
const vector<string> *RubyScript_getChannelList(RubyScript*){ return &bot.getChannelList(); }

/****f* RubyScript/isChannelName
* USAGE
*	isChannelName(STRING name)
* RETURN VALUE
*	boolean: true if channel is a valid channel name
* INPUTS
*	name : name to test
* REMARKS
*	This function only test if the given name is valid
* SINCE
*	0.8
***/
bool RubyScript_isChannelName(RubyScript *r, string name){ return bot.isChannelName(name); }

/****f* RubyScript/isBotOn
* USAGE
*	isBotOn(STRING chan)
* RETURN VALUE
*	boolean: true if bot is on the given channel
* INPUTS
*	chan: channel name
* SINCE
*	0.8
***/
bool RubyScript_isBotOn(RubyScript *r, string chan){ return bot.isBotOn(chan); }

/****f* RubyScript/userMode
* USAGE
*	userMode(STRING modestr)
* DESCRIPTION
*	set user mode for the bot
* INPUTS
*	modestr : mode to set (ex: "+x")
* SINCE
*	0.8
***/
void RubyScript_userMode(RubyScript *r, string modestring){ bot.userMode(bot.getNick(), modestring); }

/****f* RubyScript/changeNick
* USAGE
*	changeNick(STRING newnick)
* INPUTS
*	newnick: the new nick
* SINCE
*	0.8
**/
void RubyScript_changeNick(RubyScript *r, string newnick){ bot.changeNick(newnick); }

/****f* RubyScript/join
* USAGE
*	join(STRING chan, STRING key= "")
* INPUTS
*	chan: channel name
*	key: channel key
* SINCE
*	0.8
***/
void RubyScript_join(RubyScript *r, string channel, string key){ bot.join(channel, key); }

/****f* RubyScript/part
* USAGE
*	part(STRING chan)
* INPUTS
*	chan: channel name
* SINCE
*	0.8
***/
void RubyScript_part(RubyScript *r, string channel, string reason){ bot.part(channel, reason); }

void RubyScript_privmsg(RubyScript *r, string dest, char *msg){ bot.privmsg(dest, "%s", msg); }
void RubyScript_disableScript(RubyScript *r){ r->disablePlugin(); }

/****f* RubyScript/getUserObj
* USAGE
*	getUserObj(STRING nick)
* RETURN VALUE
*	%User%: return the user with given nickname
* INPUTS
*	nick: the target nick
* SINCE
*	0.8
***/
user_Info *RubyScript_getUserObj(RubyScript *r, string nick){ return bot.getUserData(nick); }

/****f* RubyScript/getChanObj
* USAGE
*	getChanObj(STRING name)
* RETURN VALUE
*	%Channel%: return the channel with given name
* INPUTS
*	name: the channel name
* SINCE
*	0.8
***/
chan_Info *RubyScript_getChanObj(RubyScript *r, string name)
{ 
	return bot.getChanData(name); 
}

/****f* RubyScript/addUserAccount
* USAGE
*	addUserAccount(STRING account, STRING hostmask, STRING flags)
* DESCRIPTION
*	add a line in userfile.txt
* RETURN VALUE
*	integer: 0 if user added
*			-1 if operation failed (hostmask already registered)
* INPUTS
*	account: account name for the newly created user
*	hostmask: hostmask the user ned to match to use this account
*	flags: flags t give to him (ex: "mno")
* REMARKS
*	You can't have two account with the same hostmask or username
* SINCE
*	0.8
***/
int RubyScript_addUserAccount(RubyScript *r, string user, string hostmask, string flags= "")
{
	int ret= userfile.addUserAccount(user, hostmask, flags);
	bot.updateFlags(hostmask);
	return ret;
}


/****f* RubyScript/setAccessFlags
* USAGE
*	setAccessFlags(STRING user, STRING type, STRING newflags)
* RETURN VALUE
*	integer: 0 if user changed
*			-1 if operation failed (unknow hostmask)
* DESCRIPTION
*	modify flag for the given user identified by hostmask
* INPUTS
*	user: user account name
*	type: _global_ or a channel name
*	newflags: flags to give to user
* SINCE
*	0.8
***/
int RubyScript_setAccessFlags(RubyScript *r, string user, string type, string flags)
{
	UsersData *tmp= userfile.findAccount(user);
	int ret= userfile.setAccessFlags(user, type, flags);

	if( tmp != NULL )
		bot.updateFlags(tmp->hostmask);

	return ret;
}

/****f* RubyScript/delUser
* USAGE
*	delUser(STRING hostmask)
* DESCRIPTION
*	remove a line from userfile.txt
* INPUTS
*	hostmask: identification of the user to remove
* SINCE
*	0.8
***/
void RubyScript_delUserAccount(RubyScript *r, string user)
{
	UsersData *tmp= userfile.findAccount(user);
	if( tmp != NULL )
	{
		bot.updateFlags(tmp->hostmask);
		userfile.delUserAccount(user);
	}
}

string RubyScript_to_s(RubyScript *r){
	//return "nick(" + bot.getNick() + ") network(" + bot.getNetworkName() + ")";
	return "RubyScript";
}

//********************************
//********************************
// User object (base on user_Info)

/****o* Ruby/User
* MODULE DESCRIPTION
*	an irc user
*
***/

/****v* User/nick
* DESCRIPTION
*	returns user nickname
* TYPE
*	read only
***/
string *user_Info_nick_get(user_Info *usr){ return &usr->nick; }

/****v* User/account_name
* DESCRIPTION
*	returns user account name
* TYPE
*	read only
***/
string *user_Info_account_name_get(user_Info *usr){ return &usr->account_name; }

/****v* User/auth
* DESCRIPTION
*	returns user auth name
* TYPE
*	read only
***/
string *user_Info_auth_get(user_Info *usr){ return &usr->auth; }

/****v* User/ident
* DESCRIPTION
*	returns user ident
* TYPE
*	read only
***/
string *user_Info_ident_get(user_Info *usr){ return &usr->ident; }

/****v* User/host
* DESCRIPTION
*	returns user host
* TYPE
*	read only
***/
string *user_Info_host_get(user_Info *usr){ return &usr->host; }

/****v* User/full_host
* DESCRIPTION
*	returns host in the form: &lt;nick&gt;!&lt;ident&gt;@&lt;host&gt;
* TYPE
*	read only
***/
string *user_Info_full_host_get(user_Info *usr){ return &usr->full_host; }

/****f* User/privmsg
* USAGE
*	privmsg(STRING msg)
* DESCRIPTION
*	sends private message to user
* INPUTS
*	msg: the message
* SINCE
*	0.8
***/
void user_Info_privmsg(user_Info *usr, const char *msg ){ bot.privmsg(usr->nick, "%s", msg); }

/****f* User/notice
* USAGE
*	notice(STRING msg)
* DESCRIPTION
*	sends notice to user
* INPUTS
*	msg: the message
* SINCE
*	0.8
***/
void user_Info_notice(user_Info *usr, const char *msg ){ bot.notice(usr->nick, "%s", msg); }

/****f* User/hasAccount
* USAGE
*	hasAccount()
* DESCRIPTION
*	sends notice to user
* RETURN VALUE
*	boolean : true if user match an account in userfile.txt
* SINCE
*	0.9
***/
bool user_Info_hasAccount(user_Info *usr){ return usr->hasAccount(); }

/****f* User/hasGlobalFlag
* USAGE
*	hasGlobalFlag(flag)
* RETURN VALUE
*	boolean : true if user has given global flag
* INPUTS
*	flag: one of the following or more with |
*
*	USRLVL_OWNER
*	USRLVL_MASTER
*	USRLVL_OPERATOR
*	USRLVL_VOICE
*	USRLVL_KICK
*	USRLVL_PUBLIC
* SINCE
*	0.9
***/
bool user_Info_hasGlobalFlag(user_Info *usr, unsigned int n){ return usr->hasFlag(n); }

/****f* User/hasChannelFlag
* USAGE
*	hasChannelFlag(chan, flag)
* RETURN VALUE
*	boolean : true if user has given flag on chan
* INPUTS
*	chan: a %Channel% object
*	flag: one of the following or more with |
*
*	USRLVL_OWNER
*	USRLVL_MASTER
*	USRLVL_OPERATOR
*	USRLVL_VOICE
*	USRLVL_KICK
*	USRLVL_PUBLIC
* SINCE
*	0.9
***/
bool user_Info_hasChannelFlag(user_Info *usr, chan_Info *chan, unsigned int n){ return usr->hasChannelFlag(n, chan->name);}

/****f* User/isAuth
* USAGE
*	isAuth()
* RETURN VALUE
*	boolean : true if user authed with IRC server
* SINCE
*	0.9
***/
bool user_Info_isAuth(user_Info *usr){ return !usr->auth.empty(); }

/****f* User/getChannelList
* USAGE
*	getChannelList()
* RETURN VALUE
*	array : channel list
* WARNING
*	this list can't be accurate since the bot only know
*	informations about the channel where it is currently
* SINCE
*	0.9
***/
vector<string> *user_Info_getChannelList(user_Info *usr){ return &usr->ison.getVector(); }

string user_Info_to_s(user_Info *usr){
	return "nick(" + usr->nick + ") account(" + usr->account_name + ") host(" + usr->full_host + ")";
}




//********************************
//********************************
// Channel object (base on chan_Info)

/****o* Ruby/Channel
* MODULE DESCRIPTION
*	an irc channel
*
***/

/****v* Channel/name
* DESCRIPTION
*	variable holding the name of the channel
* TYPE
*	read only
* SINCE
*	0.8
***/
string chan_Info_name_get(chan_Info *chan){ return chan->name; }

/****f* Channel/rejoin
* USAGE
*	rejoin()
* DESCRIPTION
*	parts and joins channel
* SINCE
*	0.8
***/
void chan_Info_rejoin(chan_Info *chan){
	if( bot.isBotOn(chan->name) )
		bot.part(chan->name);

	bot.join(chan->name, chan->key);
}

/****f* Channel/part
* USAGE
*	part()
* SINCE
*	0.8
***/
void chan_Info_part(chan_Info *chan){
	if( bot.isBotOn(chan->name) )
		bot.part(chan->name);
}

/****f* Channel/isBotOp
* USAGE
*	isBotOp()
* RETURN VALUE
*	boolean: true if bot is op on channel
* SINCE
*	0.8
***/
bool chan_Info_isBotOp(chan_Info *chan){ return bot.isBotOp(chan->name); }

/****f* Channel/isOn
* USAGE
*	isOn(STRING nick)
*	isOn(User u)
* INPUTS
*	nick: user nick
*	u: user object
* SINCE
*	0.8
***/
bool chan_Info_isOn(chan_Info *chan, string nick){ return bot.isOn(nick, chan->name); }
bool chan_Info_isOn(chan_Info *chan, user_Info *u){ return u->ison.isin(chan->name); }

/****f* Channel/isOp
* USAGE
*	isOp(STRING nick)
*	isOp(User u)
* INPUTS
*	nick: user nick
*	u: user object
* SINCE
*	0.8
***/
bool chan_Info_isOp(chan_Info *chan, string nick){ return bot.isOp(nick, chan->name); }
bool chan_Info_isOp(chan_Info *chan, user_Info *u){ return u->isop.isin(chan->name); }

/****f* Channel/isVoice
* USAGE
*	isVoice(STRING nick)
*	isVoice(User u)
* INPUTS
*	nick: user nick
*	u: user object
* SINCE
*	0.8
***/
bool chan_Info_isVoice(chan_Info *chan, string nick){ return bot.isVoice(nick, chan->name); }
bool chan_Info_isVoice(chan_Info *chan, user_Info *u){ return u->isvoice.isin(chan->name); }

/****v* Channel/topic
* DESCRIPTION
*	channel topic
* TYPE
*	read / write
* SINCE
*	0.8
***/
void chan_Info_topic_set(chan_Info *chan, string topic){ bot.setTopic(chan->name.c_str(), topic.c_str()); }
string chan_Info_topic_get(chan_Info *chan){ return bot.getTopic(chan->name); }

/****f* Channel/invite
* USAGE
*	invite(STRING u)
*	invite(%User% u)
* DESCRIPTION
*	invites user on channel
* INPUTS
*	u: user object or nick
* SINCE
*	0.8
***/
void chan_Info_invite(chan_Info *chan, string nick){ bot.invite(chan->name, nick); }
void chan_Info_invite(chan_Info *chan, user_Info *u){ bot.invite(chan->name, u->nick); }

/****f* Channel/ban
* USAGE
*	ban(STRING banmask)
*	ban(%User% u)
* INPUTS
*	banmask: ban mask
*	u: user object
* SINCE
*	0.8
***/
void chan_Info_ban(chan_Info *chan, string banmask){ bot.ban(chan->name, banmask); }
void chan_Info_ban(chan_Info *chan, user_Info *u){ bot.ban(chan->name, u->full_host); }

/****f* Channel/kick
* USAGE
*	kick(STRING nick, STRING reason= "")
*	kick(User u, STRING reason= "")
* INPUTS
*	nick: target name
*	u: user object
* SINCE
*	0.8
***/
void chan_Info_kick(chan_Info *chan, string nick, string reason= ""){ bot.kick(chan->name, nick, reason); }
void chan_Info_kick(chan_Info *chan, user_Info *nick, string reason= ""){ bot.kick(chan->name, nick->nick, reason); }

/****f* Channel/unban
* USAGE
*	ban(STRING banmask)
* INPUTS
*	banmask: unban mask
* SINCE
*	0.8
***/
void chan_Info_unban(chan_Info *chan, string banmask){ bot.unban(chan->name, banmask); }

/****f* Channel/chanMode
* USAGE
*	chanMode(STRING modestr)
* DESCRIPTION
*	set channel mode
* INPUTS
*	modestr: a channel mode (ex: "+t")
* SINCE
*	0.8
***/
void chan_Info_chanMode(chan_Info *chan, string modestring){ bot.chanMode(chan->name, modestring); }

/****f* Channel/setMode
* USAGE
*	setMode(STRING nick, STRING mode)
*	setMode(%User% u, STRING mode)
* DESCRIPTION
*	set user flag on channel
* INPUTS
*	u: user object
*	nick: target nick
*	mode: mode string (ex: "+o")
* SINCE
*	0.8
***/
void chan_Info_setMode(chan_Info *chan, string nick, string mode){ bot.setMode(chan->name, nick, mode); }
void chan_Info_setMode(chan_Info *chan, user_Info *u, string mode){ bot.setMode(chan->name, u->nick, mode); }

/****f* Channel/setChannelKey
* USAGE
*	setChannelKey(STRING newkey)
* INPUTS
*	newkey: the key
* SINCE
*	0.8
***/
void chan_Info_setChannelKey(chan_Info *chan, string pass){ bot.setChannelPassword(chan->name, pass); }

/****f* Channel/getUserlistFromMask
* USAGE
*	getUserlistFromMask(STRING mask)
* RETURN VALUE
*	array: list of all nicks matching mask on channel
* SINCE
*	0.8
***/
vector<user_Info*> chan_Info_getUserlistFromMask(chan_Info *chan, string mask){
	vector<user_Info*> list;
	bot.getUserlistFromMask(mask, chan->name, list);
	return list;
}

/****f* Channel/isBanned
* USAGE
*	isBanned(STRING host)
*	isBanned(%User% u)
* RETURN VALUE
*	boolean: true if banned
* INPUTS
*	u: user object
*	host: host
* REMARKS
*	check if a ban mask match current hostname
* SINCE
*	0.8
***/
bool chan_Info_isBanned(chan_Info *chan, string host){ return bot.isBanned(chan->name, host); }
bool chan_Info_isBanned(chan_Info *chan, user_Info *u){ return bot.isBanned(chan->name, u->full_host); }

/****f* Channel/isInBanList
* USAGE
*	isInBanList(STRING banmask)
* RETURN VALUE
*	boolean: true if in list
* INPUTS
*	banmask: the mask
* REMARKS
*	check if this mask is in the ban list exactly
*	(no wildcard match here)
* SINCE
*	0.8
***/
bool chan_Info_isInBanList(chan_Info *chan, string banmask){ return bot.isInBanList(chan->name, banmask); }

/****f* Channel/privmsg
* USAGE
*	privmsg(STRING msg)
* DESCRIPTION
*	send a message to the specified channel
* INPUTS
*	msg: the message
* SINCE
*	0.8
***/
void chan_Info_privmsg(chan_Info *chan, char *msg){ bot.privmsg(chan->name, "%s", msg); }

/****f* Channel/action
* USAGE
*	action(STRING msg)
* DESCRIPTION
*	like /me
* INPUTS
*	msg: the action
* SINCE
*	0.8
***/
void chan_Info_action(chan_Info *chan, char *msg){ bot.sendMsgTo(chan->name, MODE_PRIVMSG_ONLY, "\001ACTION %s\001", msg); }

string chan_Info_to_s(chan_Info *chan){
	return "name(" + chan->name + ") key(" + chan->key + ")";
}

///////////////
// catch undefined method calls

extern "C" VALUE rb_method_missing(int argc, VALUE *argv, VALUE obj);

static VALUE rb_my_method_missing(int argc, VALUE *argv, VALUE self)
{
	if( argc != 0 && SYMBOL_P(argv[0]))
	{
		const char *method_name= rb_id2name(SYM2ID(argv[0]));
//		VALUE klass= rb_obj_class(self);
//		const char *obj_name= rb_class2name(klass);

		if( strncmp(method_name, "event_", 6) )
			rb_method_missing(argc, argv, self);
		//	Error("[%s] undefined method '%s' for object '%s'\n", current_script->getName(), method_name, obj_name);
	}
	return Qnil;
}

////////////
// stdout ruby functions
#define RUBYFUNC(f) ((VALUE (*)(ANYARGS))f)

// putc
static VALUE rb_my_putc(VALUE recv, VALUE ch)
{
    RubyScript::stdout_write("%c", NUM2CHR(ch));
    return ch;
}

static VALUE io_my_puts_ary(VALUE ary, VALUE out);

// puts
extern "C" VALUE rb_my_puts(int argc, VALUE *argv, VALUE self)
{
	int i;
	VALUE line;

    /* if no argument given, print newline. */
    if( argc == 0 )
    {
		RubyScript::stdout_write("\n");
		return Qnil;
    }

    for(i= 0; i< argc; i++)
    {
		if( NIL_P(argv[i]) )
		{
			line= rb_str_new2("nil");
		}
		else
		{
		    line = rb_check_array_type(argv[i]);
		    if (!NIL_P(line))
		    {
				rb_protect_inspect(RUBYFUNC(io_my_puts_ary), line, self);
				continue;
		    }
		    line= rb_obj_as_string(argv[i]);
		}

		RubyScript::stdout_write("%s\n", StringValueCStr(line));
    }

    return Qnil;
}

static VALUE io_my_puts_ary(VALUE ary, VALUE out)
{
    VALUE tmp;
    long i;

    for(i= 0; i< RARRAY(ary)->len; i++)
    {
		tmp= RARRAY(ary)->ptr[i];
		if( rb_inspecting_p(tmp) )
		{
			tmp= rb_str_new2("[...]");
		}
		rb_my_puts(1, &tmp, out);
    }
    return Qnil;
}

struct _printf_param {
	int n;
	VALUE *p;
};

static VALUE rb_my_printf_ex(VALUE v)
{
	_printf_param *pp= (_printf_param*)v;
	return rb_f_sprintf(pp->n, pp->p);
}

// printf
extern "C" VALUE rb_my_printf(int argc, VALUE *argv, VALUE self)
{
	int err= 0;
    if( argc == 0 )
    	return Qnil;

	_printf_param pp= {argc, argv};
    VALUE str= rb_protect(rb_my_printf_ex, (VALUE)&pp, &err);
    if( err )
    {
    	//rb_backtrace();
    	RubyScript::stdout_write("error\n");
    }
    else
    {
    	RubyScript::stdout_write("%s", StringValueCStr(str));
    }
    return Qnil;
}

#if !defined(__FreeBSD__)
size_t strlcat(char *d, const char *s, size_t bufsize);
#endif

void RubyScript::stdout_write(char *fmt, ...)
{
	static char big_buff[300];

	va_list va;
	char buff[100];
	const char *file_name= "Ruby";

	va_start(va, fmt);
	vsnprintf(buff, sizeof(buff), fmt, va);
	va_end(va);

	if( current_script != NULL )
		file_name= current_script->getName();

	strlcat(big_buff, buff, sizeof(big_buff)-1);

	char *tmp, *tmp2;
	while( (tmp= strchr(big_buff, '\n')) != NULL )
	{
		Output("[%s] %.*s", file_name, tmp-big_buff+1, big_buff);
		tmp2= strdup(big_buff);
		strcpy(big_buff, tmp2+(tmp-big_buff)+1);
		free(tmp2);
	}
}

////////////
// New
static VALUE NewWrap(VALUE arg)
{
	const char *klass= ((string*)arg)->c_str();

	VALUE self = rb_class_new_instance(0, 0, rb_path2class(klass));
	//VALUE self = rb_funcall2(rb_path2class(klass), rb_intern("new"), 0, 0);
	return self;
}

static VALUE New(string klass)
{
	int error= 0;
	VALUE self= rb_protect(NewWrap, (VALUE)&klass, &error);
	if( error )
		throw RubyError();

	return self;
}


////////////
// Funcall
struct args
{
	VALUE target;
	ID id;
	int n;
	VALUE *argv;
	/***/
	VALUE ret;
	int error;
};

static VALUE FuncallWrap(VALUE v)
{
	args *a= (args*)v;
	return rb_funcall2(a->target, a->id, a->n, a->argv);
}

extern "C" void error_print();
static VALUE Funcall(VALUE dest, ID id, int n, VALUE *v, int *err)
{
	args arg= {dest, id, n, v, 0, 0};
	*err= 0;
	
	arg.ret= rb_protect(FuncallWrap, (VALUE)&arg, &arg.error);
	
	if( arg.error != 0 )
	{
		//msg= rb_funcall2(ruby_errinfo, rb_intern("to_s"), 0, 0);
		VALUE msg= rb_inspect(ruby_errinfo);

//		VALUE klass= rb_obj_class(self);
//		const char *obj_name= rb_class2name(klass);

		Error("[%s] Ruby Error: %s\n", current_script->getName(), StringValueCStr(msg));
		Error("[%s] Trace:\n", current_script->getName());
		error_print();
		Error("[%s] End trace.\n", current_script->getName());
		
		*err= -1;
	}

	return arg.ret;
}

// defined in eval.c
extern "C" int ruby_safe_level;

static VALUE LoadWrap(VALUE arg)
{
	rb_load(arg, 0);
	return Qnil;
}

static int Load(const char *name)
{
	int error= 0;

	rb_protect(LoadWrap, rb_str_new2(name), &error);
	if( error != 0 )
	{
		VALUE msg= rb_inspect(ruby_errinfo);

		Error("[%s] Ruby Error: %s\n", name, StringValueCStr(msg));
		Error("[%s] Trace:\n", name);
		error_print();
		Error("[%s] End trace.\n", name);
		return -1;
	}

	return 0;
}

/*********************************/
/*********************************/

void RubyScript::initEngine()
{
	ruby_init();
	//ruby_init_loadpath();
	ruby_incpush(".");
	ruby_incpush("libs/ruby/");
	ruby_incpush("libs/ruby/lib");
	Init_Embed();

	// Custom init
	// redirect all stdout functions to use logging
    rb_define_global_function("printf", 		RUBYFUNC(rb_my_printf), 			-1);
    //rb_define_global_function("print", rb_my_print, -1);
    rb_define_global_function("putc", 			RUBYFUNC(rb_my_putc), 				 1);
    rb_define_global_function("puts", 			RUBYFUNC(rb_my_puts), 				-1);
    rb_define_global_function("debugPrint",		RUBYFUNC(rb_my_puts), 				-1);
    rb_define_global_function("method_missing", RUBYFUNC(rb_my_method_missing), 	-1);
}

void RubyScript::freeEngine()
{
	//ruby_finalize();
	rb_gc();
	ruby_cleanup(0);
}

// used for garbage collection
void RubyScript::ping()
{
	rb_gc();
}

RubyScript::RubyScript()
{
	Debug("RubyScript::RubyScript()\n");
}

RubyScript::~RubyScript()
{
	Debug("RubyScript::~RubyScript(%s)\n", getName());
}

void RubyScript::deleteObj(RubyScript *r)
{
	Debug("RubyScript::deleteObj(\"%s\")\n", r->path);
	// tells ruby garbage collector to delete current object
	rb_gc_unregister_address( &r->self );
	rb_gc_start();
}

Script *RubyScript::newScript(const string &_path)
{
	Script *ret= NULL;

	int rr= Load(_path.c_str());

	if( ruby_nerrs > 0 )
	{
		Error("Errors while loading %s\n", _path.c_str());
		return NULL;
	}

	//int rr= ruby_exec();
	if( rr )
	{
		Error("Ruby loading error: %s\n", _path.c_str());
		return NULL;
	}

	VALUE v = rb_gv_get("$plugin_class");
	if( TYPE(v) == T_STRING )
	{
		char *cl_name= strdup(StringValueCStr(v));
		rb_gv_set("$plugin_class", Qnil);

		ID c= rb_intern(cl_name);
		if( c != 0 )
		{
			v= New(cl_name);
			if( v != Qnil )
			{
				RubyScript *tmp_ret;

				//plugins.push_back( Plugin::newPlugin(v) );
				Data_Get_Struct(v, RubyScript, tmp_ret);
				tmp_ret->self= v;
				rb_gc_register_address( &tmp_ret->self );
				strncpy(tmp_ret->path, _path.c_str(), sizeof(tmp_ret->path));
				ret= (Script*)tmp_ret;
				Output("** Ruby script loaded: %s(%s)\n", _path.c_str(), cl_name);
			}
			else
			{
				Error("Object creation failed\n");
			}
		}
		else
		{
			Error("** RUBY undefined class: %s\n", cl_name);
		}

		free(cl_name);
	}
	else
	{
		Error("** RUBY | Unable to load script, $plugin_class undefined: %s\n", _path.c_str());
	}

	return ret;
}

int RubyScript::callFunctionEx(const char *event, const char *format, va_list va)
{
	int ret= 0;

	if( isEnabled() )
	{
		uint i, pcount= strlen(format);

		VALUE params[20];

		if( pcount > 0 )
		{
			for(i= 0; i< pcount; i++)
			{
				// user object
				if( format[i]=='u' )
				{
					user_Info *u= va_arg(va, user_Info*);
					params[i]= Data_Wrap_Struct(cUser.klass, 0, 0, u);
					// for swig
					rb_iv_set(params[i], "__swigtype__", rb_str_new2("_p_user_Info"));
				}
				else if( format[i]=='c' )
				{
					chan_Info *c= va_arg(va, chan_Info*);
					params[i]= Data_Wrap_Struct(cChannel.klass, 0, 0, c);
					// for swig
					rb_iv_set(params[i], "__swigtype__", rb_str_new2("_p_chan_Info"));
				}
				// integer parameter
				else if( format[i]=='i' )
				{
					int n= va_arg(va, int);
					params[i]= INT2NUM(n);
				}
				// string parameter
				else if( format[i]=='s' )
				{
					char *s= va_arg(va, char*);
					params[i]= rb_str_new2(s);
				}
				// vector => multiple string parameters
				else if( format[i]=='w' )
				{
					const vector<string> *array= va_arg(va, vector<string>*);
					for(uint k= 0; k< array->size(); k++)
					{
						params[i+k]= rb_str_new2( (*array)[k].c_str() );
						pcount++;
					}
					// accepted only as last parameter
					break;
				}
				// vector => string_array
				else if( format[i]=='v' )
				{
					const vector<string> *array= va_arg(va, vector<string>*);

					params[i]= rb_ary_new2( array->size() );

					for(uint k= 0; k< array->size(); k++)
					{
						rb_ary_push(params[i], rb_str_new2((*array)[k].c_str()));
					}

				}
				else
				{
					Debug("[callFunctionEx] (%s :: %s) not called (unknow format: '%c')\n", getName(), event, format[i]);
				}
			}
		}

		ID ev= rb_intern(event);
		int error;

		current_script= this;
		Debug("[callFunctionEx] %s :: %s\n", path, event);
		VALUE r= Funcall(self, ev, pcount, params, &error);
		if( TYPE(r) == T_FIXNUM )
			ret= NUM2INT(r);

		current_script= NULL;

/*
		if( error != 0 )
		{
			if( error == Qundef )
			{
				Debug("[%s] %s: NOT IMPLEMENTED\n", getName(), event);
			}
			else
			{
				Error("[%s] %s: ERROR: %d\n", getName(), event, error);
			}
		}
*/
	}
	else
	{
		Debug("Function call (%s) aborted for plugin %s (plugin disabled)\n", event, getName());
	}

	return ret;
}

/**/

