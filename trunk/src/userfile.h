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

global only flags:
- n: owner (simply the owner of the bot, he owns all access)
- m: master (he has access to most of the features of the bot)
	. cannot use :quit, :join and :part

channel specific flags (also valid as global flags):
- o: auto-op
- v: auto-voice
- k: auto-kick (as soon as the bot is aware of the user presence, the user is kicked & banned)

*/

#ifndef __USERFILE_H
#define __USERFILE_H

#include "config.h"
#include <string>
#include <map>
#include "basebot.h"

class UsersData
{
public:
	UsersData(){};
	UsersData(const string &s1) : name(s1){}
	string name;
	string flags;
	string hostmask;
	map<string,string>	channel_flags;
};


// users flags


#define USRLVL_OWNER	(1<<0)	// 'n' 	0x01
#define USRLVL_MASTER	(1<<1)	// 'm'	0x02
#define USRLVL_OPERATOR	(1<<2)	// 'o'	0x04
#define USRLVL_VOICE	(1<<3)	// 'v'	0x08
#define USRLVL_KICK		(1<<4)	// 'k'	0x10
#define USRLVL_PUBLIC	(1<<9)	// '*'	0x200

#define FLAGTYPE_GLOBAL		0x01
#define FLAGTYPE_CHANNEL	0x02

struct _flag{
	char ch;
	uint mask;
	u_char type;// 0x01= global
				// 0x02= channel specific
				// 0x03= global & channel
};

bool operator ==(const UsersData &u1, const UsersData &u2);
bool operator ==(const UsersData &u1, const UsersData &u2);

char *getUserfileTemplate();

class UserFile
{
private:
	char path[100];
	map<string, UsersData> userLevels;

public:
	static UsersData invalid;

	UserFile(const char *path2= "userfile.txt");
	~UserFile();
	int readUserFile(const char *fname= "");
	void saveFile();

	int addUserAccount(const string&, const string&, const string&);
	int setAccessFlags(const string&, const string&, const string&);
	void delUserAccount(const string&);
	map<string, UsersData> &getAccessList();

	bool setUserFlagsFromUserfile(user_Info &user, bool);
	bool setChanFlagsFromUserfile(user_Info &user);

	UsersData *findAccount(string name);

	// global flags
	static void setFlagsMaskFromString(const string &str, uint &lvl);
	static bool setFlagMaskFromChar(char c, uint &lvl);

	// channel flags
	static void setChanFlagsMaskFromString(const string &str, uint &lvl);
	static bool setChanFlagMaskFromChar(char c, uint &lvl);
};

#endif // __USERFILE_H

/**/

