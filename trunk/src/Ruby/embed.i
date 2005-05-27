%module Embed

%{
#include "ruby_script.h"
#include "network.h"
%}


%typemap(out) string {
    $result= rb_str_new2($1.c_str());
}

%typemap(out) string* {
	$result= rb_str_new2($1->c_str());
}

%typemap(in) string {
	Check_Type($input, T_STRING);
	$1= std::string( StringValueCStr($input) );
}

%typemap(typecheck) string = char*;

%typemap(out) vector<string>* {
	VALUE tmp= rb_ary_new2($1->size());
	for( unsigned int ii= 0; ii< $1->size(); ii++ )
	{
		VALUE tmp_str= rb_str_new2((*$1)[ii].c_str());
		rb_ary_push(tmp, tmp_str);
	}

	$result= tmp;
}

%typemap(out) vector<string> {
	VALUE tmp= rb_ary_new2($1.size());
	for( unsigned int ii= 0; ii< $1.size(); ii++ )
	{
		VALUE tmp_str= rb_str_new2($1[ii].c_str());
		rb_ary_push(tmp, tmp_str);
	}

	$result= tmp;
}


//***************
// bot relative functions
//

class RubyScript
{
public:
	RubyScript();
	~RubyScript();
};
%{
	string RubyScript_nick_get(RubyScript *);
	int RubyScript_max_nicklen_get(RubyScript*);
	int RubyScript_max_topiclen_get(RubyScript*);
	const vector<string> *RubyScript_getChannelList(RubyScript*);
	bool RubyScript_isChannelName(RubyScript *r, string name);
	void RubyScript_userMode(RubyScript *r, string modestring);
	void RubyScript_changeNick(RubyScript *r, string newnick);
	void RubyScript_join(RubyScript *r, string channel, string key= "");
	void RubyScript_part(RubyScript *r, string channel, string reason= "");
	void RubyScript_privmsg(RubyScript *r, string dest, char *msg);
	user_Info *RubyScript_getUserObj(RubyScript *r, string nick);
	chan_Info *RubyScript_getChanObj(RubyScript *r, string name);
	bool RubyScript_isBotOn(RubyScript *r, string chan);

	bool RubyScript_isMe(RubyScript *r, user_Info *usr);
	void RubyScript_disableScript(RubyScript *r);

	VALUE RubyScript_loadSimpleVar(RubyScript *r, char *name);
	void RubyScript_saveSimpleVar(RubyScript *r, char *name, VALUE val);

	//VALUE RubyScript_loadArrayVar(RubyScript *r, char *name);

	void RubyScript_registerCommand(RubyScript *r, string cmd, string func, char flag, string usage);

	int RubyScript_addUserAccount(RubyScript *r, string user, string hostmask, string flags= "");
	int RubyScript_setAccessFlags(RubyScript *r, string user, string type, string flags);
	void RubyScript_delUserAccount(RubyScript *r, string user);

	string RubyScript_to_s(RubyScript *r);
%}
%extend RubyScript {
%immutable;
	%rename(bot_nick) nick;
	string nick;
	int max_nicklen;
	int max_topiclen;
%mutable;

	const vector<string> *getChannelList();
	void userMode(string modestring);

	bool isChannelName(string name);
	void changeNick(string nick);
	void join(string channel, string key= "");
	void part(string channel, string reason= "");

	void privmsg(string dest, char *msg);

	user_Info *getUserObj(string nick);
	chan_Info *getChanObj(string name);

	VALUE loadSimpleVar(char *name);
	void saveSimpleVar(char *name, VALUE val);

	//VALUE loadArrayVar(char *name);

	void registerCommand(string cmd, string func, char flag, string usage= "");
	int addUserAccount(string user, string hostmask, string flags);
	int setAccessFlags(string user, string type, string flags);
	void delUserAccount(string user);

	bool isBotOn(string chan);
	bool isMe(user_Info *user);

	void disableScript();

	string to_s();
};

//***************
// User
//

%rename(User) user_Info;
class user_Info {};
%{
string *user_Info_nick_get(user_Info *usr);
string *user_Info_account_name_get(user_Info *usr);
string *user_Info_auth_get(user_Info *usr);
string *user_Info_ident_get(user_Info *usr);
string *user_Info_host_get(user_Info *usr);
string *user_Info_full_host_get(user_Info *usr);

bool user_Info_hasAccount(user_Info *usr);
bool user_Info_hasGlobalFlag(user_Info *usr, unsigned int n);
bool user_Info_hasChannelFlag(user_Info *usr, chan_Info *chan, unsigned int n);
bool user_Info_isAuth(user_Info *usr);
vector<string> *user_Info_getChannelList(user_Info *usr);

void user_Info_privmsg(user_Info *, const char *);
void user_Info_notice(user_Info *, const char *);

string user_Info_to_s(user_Info *);
%}
%extend user_Info {
%immutable;
	string *nick;
	string *account_name;
	string *auth;
	string *ident;
	string *host;
	string *full_host;
%mutable;

#define USRLVL_OWNER	(1<<0)	// 'n' 	0x01
#define USRLVL_MASTER	(1<<1)	// 'm'	0x02
#define USRLVL_OPERATOR	(1<<2)	// 'o'	0x04
#define USRLVL_VOICE	(1<<3)	// 'v'	0x08
#define USRLVL_KICK		(1<<4)	// 'k'	0x10
#define USRLVL_PUBLIC	(1<<9)	// '*'	0x200

	bool isAuth();
	bool hasAccount();
	bool hasGlobalFlag(unsigned int n);
	bool hasChannelFlag(chan_Info*, unsigned int n);

	vector<string> *getChannelList();

	void privmsg(char *msg);
	void notice(char *msg);
	string to_s();
};

//***************
// Channel
//

%rename(Channel) chan_Info;
class chan_Info {};
%{
	string chan_Info_name_get(chan_Info *chan);

	bool chan_Info_isBotOp(chan_Info *chan);

	bool chan_Info_isOn(chan_Info *chan, string nick);
	bool chan_Info_isOn(chan_Info *chan, user_Info *u);

	bool chan_Info_isOp(chan_Info *chan, string nick);
	bool chan_Info_isOp(chan_Info *chan, user_Info *u);

	bool chan_Info_isVoice(chan_Info *chan, string nick);
	bool chan_Info_isVoice(chan_Info *chan, user_Info *u);

	void chan_Info_topic_set(chan_Info *chan, string topic);
	string chan_Info_topic_get(chan_Info *chan);

	void chan_Info_invite(chan_Info *chan, string nick);
	void chan_Info_invite(chan_Info *chan, user_Info *u);

	void chan_Info_ban(chan_Info *chan, string banmask);
	void chan_Info_ban(chan_Info *chan, user_Info *u);

	void chan_Info_kick(chan_Info *chan, string nick, string reason);
	void chan_Info_kick(chan_Info *chan, user_Info *nick, string reason);

	void chan_Info_unban(chan_Info *chan, string banmask);

	void chan_Info_chanMode(chan_Info *chan, string modestring);

	void chan_Info_setMode(chan_Info *chan, string nick, string mode);
	void chan_Info_setMode(chan_Info *chan, user_Info *u, string mode);

	void chan_Info_ban(chan_Info *chan, user_Info *u);

	void chan_Info_rejoin(chan_Info *chan);
	void chan_Info_part(chan_Info *chan);

	void chan_Info_setChannelKey(chan_Info *chan, string pass);
	vector<user_Info*> chan_Info_getUserlistFromMask(chan_Info *chan, string mask);

	bool chan_Info_isBanned(chan_Info *chan, string host);
	bool chan_Info_isBanned(chan_Info *chan, user_Info *u);

	bool chan_Info_isInBanList(chan_Info *chan, string banmask);

	void chan_Info_privmsg(chan_Info *chan, char *msg);
	void chan_Info_action(chan_Info *chan, char *msg);

	string chan_Info_to_s(chan_Info *chan);
%}
%extend chan_Info {

%immutable;
	string name;
%mutable;

	string topic;

	bool isBotOp();

	bool isOn(string nick);
	bool isOn(user_Info*);

	bool isOp(string nick);
	bool isOp(user_Info*);

	bool isVoice(string nick);
	bool isVoice(user_Info*);

	void invite(string nick);
	void invite(user_Info*);

	void ban(string banmask);
	void ban(user_Info*);

	void rejoin();
	void part();

	void kick(string nick, string reason= "");
	void kick(user_Info*, string reason= "");

	void unban(string banmask);

	void chanMode(string modestring);

	void setMode(string nick, string mode);
	void setMode(user_Info *u, string mode);

	void setChannelKey(string pass);
	vector<user_Info*> getUserlistFromMask(string mask);

	bool isBanned(string host);
	bool isBanned(user_Info*);

	void privmsg(char *msg);
	void action(char *);

	bool isInBanList(string banmask);

	string to_s();
};

/*%include "network.h"*/


