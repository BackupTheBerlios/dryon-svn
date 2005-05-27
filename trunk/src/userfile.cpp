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
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include "userfile.h"
#include "regex.h"
#include "match.h"
#include "log.h"

/*
format of the file:

comment lines start with "//"
auth lines

******************************
current flags:

n : master
o : auto-op
v : auto-voice
*/

UsersData UserFile::invalid(" LM%%&$¤ ");

bool operator ==(const UsersData &u1, const UsersData &u2)
{
	return ((u1.name==u2.name) && (u1.flags==u2.flags));
}

bool operator !=(const UsersData &u1, const UsersData &u2)
{
	return !operator==(u1,u2);
}

UserFile::UserFile(const char *path2)
{
	strcpy(path, path2);
}

UserFile::~UserFile()
{
	saveFile();
}

#define USERFILE_HEADER \
"// for auth identification (Quakenet, GameSurge):\n" \
"// a/AUTH\n" \
"//\n" \
"// for host/ident identification:\n" \
"// <nick>!<ident>@<host>\n" \
"//\n" \
"// flags and where they are valid:\n" \
"// owner(n)  : global\n" \
"// master(m) : global\n" \
"// op(o)     : global & chan\n" \
"// voice(v)  : global & chan\n" \
"// kickban(k): global & chan\n" \
"//\n" \
"// You can use wildcard '*' anywhere in hostmask\n"

#define ACCOUNTS_HELP \
"\n// accounts definition\n" \
"__accounts__\n"

#define GLOBAL_HELP \
"\n// global flags have priority over channel flags\n" \
"// and are also used when sending commands as private message\n" \
"__global__\n"

#define CHANNELS_HELP \
"\n// channels specific flags\n"

#define USERFILE_TEMPLATE \
USERFILE_HEADER \
ACCOUNTS_HELP \
"Everyone *@*\n\n" \
GLOBAL_HELP \
"Everyone nm\n" \
"\n" \
CHANNELS_HELP \
"#channel1\n" \
"Everyone v\n" \
"#channel2\n" \
"Everyone v\n"

// flags valid in userfile.txt
#define USERFILE_FLAGS_GLOBAL "mnovk"
#define USERFILE_FLAGS_CHANS "ovk"

_flag flags[]=
{
{'n', USRLVL_OWNER,		FLAGTYPE_GLOBAL },
{'m', USRLVL_MASTER,	FLAGTYPE_GLOBAL },
{'o', USRLVL_OPERATOR,	FLAGTYPE_GLOBAL | FLAGTYPE_CHANNEL},
{'v', USRLVL_VOICE,		FLAGTYPE_GLOBAL | FLAGTYPE_CHANNEL},
{'k', USRLVL_KICK,		FLAGTYPE_GLOBAL | FLAGTYPE_CHANNEL},
{'*', USRLVL_PUBLIC,	FLAGTYPE_GLOBAL },
{' ',0,0}	// terminator
};

char *getUserfileTemplate()
{
	return USERFILE_TEMPLATE;
}

////////////////////////////////////////////////
///////////// GLOBAL FLAGS /////////////////////

/*!
\return false if character is invalid
*/
bool UserFile::setFlagMaskFromChar(char c, uint &lvl)
{
	for(int i= 0; flags[i].type!=0; i++)
	{
		// if flags is the current one
		// and flags is a valid global flag
		if( (c==flags[i].ch) && ((flags[i].type & FLAGTYPE_GLOBAL) != 0) )
		{
			// then set corresponding bit
			lvl|= flags[i].mask;
			return true;
		}
	}

	return false;
}

void UserFile::setFlagsMaskFromString(const string &str, uint &lvl)
{
	lvl= 0;

	for( uint i= 0; i< str.length(); i++ )
		setFlagMaskFromChar(str[i], lvl);
}

//////////////////////////////////////////////////////////////
/////////////// CHANNEL SPECIFIC FLAGS ///////////////////////

bool UserFile::setChanFlagMaskFromChar(char c, uint &lvl)
{
	for(int i= 0; flags[i].type!=0; i++)
	{
		// if flags is the current one
		// and flags is a valid global flag
		if( (c==flags[i].ch) && ((flags[i].type & FLAGTYPE_CHANNEL) != 0) )
		{
			// then set corresponding bit
			lvl|= flags[i].mask;
			return true;
		}
	}

	return false;
}

void UserFile::setChanFlagsMaskFromString(const string &str, uint &lvl)
{
	lvl= 0;

	for( uint i= 0; i< str.length(); i++ )
		setChanFlagMaskFromChar(str[i], lvl);
}

/////////////////

bool UserFile::setChanFlagsFromUserfile(user_Info &user)
{
	UsersData *u= NULL;
	bool ret;

	for(map<string, UsersData>::iterator it= userLevels.begin(); it!= userLevels.end(); it++)
	{
		if( (*it).second.name == user.account_name )
		{
			u= &(*it).second;
			break;
		}
	}

	if( u != NULL )
	{
		for(map<string, string>::iterator it= u->channel_flags.begin(); it!= u->channel_flags.end(); it++)
		{
			setChanFlagsMaskFromString(it->second, user.channel_flags[it->first]);
		}
		ret= true;
	}
	else
	{
		ret= false;
	}

	return ret;
}

bool UserFile::setUserFlagsFromUserfile(user_Info &user, bool useauth)
{
	user.user_flags= 0;

	for(map<string,UsersData>::iterator it= userLevels.begin(); it!=userLevels.end(); it++)
	{
		bool ident= false;


		// auth identification
		if( (it->first[0]=='a') && (it->first[1]=='/') )
		{
			if( useauth )
			{
				string auth= it->first;
				auth.erase(0,2);
				if( auth==user.auth )
					ident= true;
			}
			else
			{
				//Error("Identification by AUTH disabled: <%s>\n", it->first.c_str());
			}
		}

		if( !ident && wc_match(it->first, user.full_host) )
		{
			ident= true;
		}

		// corresponding user record found
		if( ident )
		{
			user.account_name= it->second.name;
			setFlagsMaskFromString(it->second.flags, user.user_flags);
		}
	}

	return false;
}

map<string, UsersData> &UserFile::getAccessList()
{
	return userLevels;
}

void UserFile::delUserAccount(const string &user)
{
	UsersData *tmp= findAccount(user);
	if( tmp != NULL )
		userLevels.erase(tmp->hostmask);
}

// type= chan
// type= "_global_"
int UserFile::setAccessFlags(const string &user, const string &type, const string &flags)
{
	int ret;
	UsersData *tmp= findAccount(user);
	if( tmp != NULL )
	{
		if( type == "_global_" )
		{
			tmp->flags= flags;
		}
		else
		{
			tmp->channel_flags[type]= flags;
		}
		ret= 0;
	}
	// not found in user list
	else
	{
		ret= -1;
	}

	return ret;
}

int UserFile::addUserAccount(const string &user, const string &hostmask, const string &global_flags)
{
	int ret;
	map<string, UsersData>::iterator it= userLevels.find(hostmask);

	if( it==userLevels.end() )
	{
		UsersData *tmp= findAccount(user);
		if( tmp == NULL )
		{
			UsersData ud;
			ud.name= user;
			ud.flags= global_flags;
			userLevels[hostmask]= ud;
			Debug("addAccess([%s], [%s], [%s]);\n", user.c_str(), hostmask.c_str(), global_flags.c_str());
			ret= 0;
		}
		else
		{
			ret= -1;
		}
	}
	else
	{
		// hostname already registered
		ret= -1;
	}

	return ret;
}
/*
__accounts__
<account> <hostmask>
...

__global__
<account> <modes>
<account> <modes>
...

#<chan>
<account> <modes>
<account> <modes>
*/
void UserFile::saveFile()
{
	if( userLevels.size() > 0 )
	{
		FILE *f= fopen(path, "w");
		if( f != NULL )
		{
			map<string, UsersData>::iterator it;
			fprintf(f, USERFILE_HEADER);

			fputs(ACCOUNTS_HELP, f);
			for(it= userLevels.begin(); it!=userLevels.end(); it++)
			{
				const string &host= it->first;
				UsersData &usr= it->second;

				fprintf(f, "%s\t%s\n", usr.name.c_str(), host.c_str());
			}

			fputs(GLOBAL_HELP, f);
			for(it= userLevels.begin(); it!=userLevels.end(); it++)
			{
				UsersData &usr= it->second;

				if( usr.flags.length() > 0 )
					fprintf(f, "%s\t%s\n", usr.name.c_str(), usr.flags.c_str());
			}

			fputs(CHANNELS_HELP, f);

			// channels
			map<string, vector<UsersData*> > channel_map;

			// build list
			for(it= userLevels.begin(); it!=userLevels.end(); it++)
			{
				UsersData &usr= it->second;

				for(map<string,string>::iterator it2= usr.channel_flags.begin(); it2!=usr.channel_flags.end(); it2++)
					channel_map[it2->first].push_back(&usr);
			}

			// write data to file
			for(map<string, vector<UsersData*> >::iterator it2= channel_map.begin(); it2!=channel_map.end(); it2++)
			{
				vector<UsersData*> &v= it2->second;
				// channel name
				fprintf(f, "\n%s\n", it2->first.c_str());

				for(uint i= 0; i< v.size(); i++)
				{
					map<string,string>::iterator iit= v[i]->channel_flags.find(it2->first);
					if( iit != v[i]->channel_flags.end() )
						fprintf(f, "%s\t%s\n", v[i]->name.c_str(), iit->second.c_str());
				}
			}

			fclose(f);
		}
		else
		{
			Error("Unable to open %s for writing !\n", path);
		}

		userLevels.clear();
	}
}

// remove spaces, tabs, '\n' and '\r'
char *trim(char *str)
{
	char *ret= str;

	if( strlen(str) != 0 )
	{
		char *end= str + strlen(str) - 1;

		while( *ret && isspace(*ret) ){ ret++; }

		while( *end && isspace(*end))
		{
			*end= '\0';
			end--;
		}
	}

	return ret;
}

void re_get_nth(const char *full_str, const regmatch_t *regs, int n, char *buff, int size)
{
	if( regs[n].rm_so != -1 )
	{
		int start, end;
		start= regs[n].rm_so;
		end= regs[n].rm_eo;

		snprintf(buff, size, "%.*s", end-start, full_str+start);
	}
}

enum{
	SECTION_NONE= 0,
	SECTION_ACCOUNTS,
	SECTION_GLOBAL,
	SECTION_CHAN
};

/*

__accounts__
<account> <hostmask>
...

__global__
<account> <modes>
<account> <modes>
...

#<chan>
<account> <modes>
<account> <modes>
...

*/

#define DEF_REGEXP(V,R) \
	if( (err= regcomp(&V, R, REG_EXTENDED | REG_NEWLINE )) != 0 ){ \
		regerror(err, &V, buff, sizeof(buff)-1); \
		Error("regcomp error: %s\n", buff); \
		re_err++; \
	}

int UserFile::readUserFile(const char *fname)
{
	regex_t reg_sections[3], reg_chan;
	regmatch_t regs[10];
	char buff[150], buff2[150], *p1, *p2;

	struct stat sb;
	//string out[3];
	char _line[200], *line;
	fstream file;

	// definition of regex patterns
	int err;
	int re_err= SECTION_NONE;

	// channel name recognition, no () matching
	DEF_REGEXP(reg_chan, 		"^(#|&)[^ ]+$");

	DEF_REGEXP(reg_sections[0], "^([^ ]+)[[:space:]]+([^ ]+)$");
	DEF_REGEXP(reg_sections[1], "^([^ ]+)[[:space:]]+(["USERFILE_FLAGS_GLOBAL"]+)$");
	DEF_REGEXP(reg_sections[2], "^([^ ]+)[[:space:]]+(["USERFILE_FLAGS_CHANS"]+)$");

	if( re_err != 0 )
		return -1;

	// fname != ""
	if( strcmp(fname, "") )
		strncpy(path, fname, sizeof(path)-1);

	if( stat(path, &sb) == -1 )
	{
		// file does not exists
		if( errno == ENOENT)
			return -1;

		// no access
		if( errno == EACCES )
			return -2;
	}

	file.open(path, ios::in);
	if( file.bad() )
		return -2;

	//userLevels.clear();

	string cur_chan;
	/*
	0= nothing
	1= accounts definition
	2= global flags
	3= channel flags
	*/
	int section= 0;

	while( file.getline(_line, sizeof(_line)-1) )
	{
		line= trim(_line);

		// comment or empty line
		if( ((line[0]=='/') && (line[1]=='/')) || (strlen(line) == 0) )
			continue;

		// state changes
		if( !strcmp(line, "__accounts__") )
		{
			//Debug("\nSection: Accounts\n");
			section= SECTION_ACCOUNTS;
			continue;
		}
		else if( !strcmp(line, "__global__") )
		{
			//Debug("\nSection: Global flags\n");
			section= SECTION_GLOBAL;
			continue;
		}
		else if( regexec(&reg_chan, line, 0, regs, 0) == 0 )
		{
			//Debug("\nSection: Channel Flags for %s\n", line);
			section= SECTION_CHAN;
			cur_chan= line;
			continue;
		}

		// if we are not in a section, skip the rest
		if( section == 0 )
			continue;

		switch( section )
		{
		// accounts definition
		// rules: username and hostmask is unique
		case SECTION_ACCOUNTS:
			err= regexec(&reg_sections[0], line, sizeof(regs)/sizeof(regmatch_t), regs, 0);
			if( err == 0 )
			{
				re_get_nth(line, regs, 1, buff, sizeof(buff)-1);
		   		re_get_nth(line, regs, 2, buff2, sizeof(buff2)-1);
		   		p1= trim(buff);
		   		p2= trim(buff2);

		   		// check username
		   		UsersData *tmp= findAccount(p1);
		   		if( tmp == NULL )
		   		{
		   			map<string, UsersData>::iterator it= userLevels.find(p2);
		   			if( it == userLevels.end() )
		   			{
		   				UsersData &acct_data= userLevels[p2];
						acct_data.name= p1;
						acct_data.hostmask= p2;
					}
					else
					{
						Error("account with same hostmask/auth detected: %s, change one !\n", p2);
					}
				}
				else
				{
					Error("account with same username detected: %s, change one !\n", p1);
				}

		   		//Debug("account[%s] host[%s]\n", trim(buff), trim(buff2));

			}
			else if( err == REG_NOMATCH )
			{
				Error("invalid line: '%s'\n", line);
			}

			break;

		case SECTION_GLOBAL:
			err= regexec(&reg_sections[1], line, sizeof(regs)/sizeof(regmatch_t), regs, 0);
			if( err == 0 )
			{
				re_get_nth(line, regs, 1, buff, sizeof(buff)-1);
		   		re_get_nth(line, regs, 2, buff2, sizeof(buff2)-1);
	   		p1= trim(buff);
		   		p2= trim(buff2);

		   		UsersData *acct_data= findAccount(p1);
		   		if( acct_data != NULL )
		   		{
					acct_data->flags= p2;
		   			//Debug("account[%s] flags[%s]\n", trim(buff), trim(buff2));
		   		}
		   		else
		   		{
		   			Error("Undefined account in global section: %s\n", p1);
		   		}
			}
			else if( err == REG_NOMATCH )
			{
				Error("invalid line or flag invalid: '%s'\n", line);
			}

			break;

		case SECTION_CHAN:
			err= regexec(&reg_sections[2], line, sizeof(regs)/sizeof(regmatch_t), regs, 0);
			if( err == 0 )
			{
				re_get_nth(line, regs, 1, buff, sizeof(buff)-1);
		   		re_get_nth(line, regs, 2, buff2, sizeof(buff2)-1);
		   		p1= trim(buff);
		   		p2= trim(buff2);

		   		UsersData *acct_data= findAccount(p1);
		   		if( acct_data != NULL )
		   		{
					acct_data->channel_flags[cur_chan]= p2;
		   			//Debug("account[%s] flags[%s]\n", trim(buff), trim(buff2));
		   		}
		   		else
		   		{
		   			Error("Undefined account in section %s: %s\n", cur_chan.c_str(), p1);
		   		}
			}
			else if( err == REG_NOMATCH )
			{
				Error("invalid line or flag invalid: '%s'\n", line);
			}

			break;

		default:
			Error("invalid section: %d\n", section);
		}
/*
		// <username>	<hostmask>	<flag(s)>
		if( match_expr("* * *", line, out, 3) )
		{
			// removed fix
			// if we already have an entry for this hostmaks, just ignore the one from file
			//map<string, UsersData>::iterator it= userLevels.find(out[1]);
			//if( it == userLevels.end() )
			//{
				Debug("user(%s) mask(%s) flags(%s)\n", out[0].c_str(), out[1].c_str(), out[2].c_str());
				cur_usr= &userLevels[out[1]];
				cur_usr->name= out[0];
				cur_usr->flags= out[2];
			//}
			//else
			//{
			//	cur_usr= NULL;
			//}
		}
		// <channel>	<flag(s)>
		else if( (cur_usr!=NULL) && match_expr("* *", line, out, 2) )
		{
			Debug("user(%s) channel(%s) levels(%s)\n", cur_usr->name.c_str(), out[0].c_str(), out[1].c_str());
			cur_usr->channel_flags[out[0]]= out[1];
		}
		else
		{
			//Error("invalid line: \"%s\"\n", line);
		}
*/
	}

	file.close();
	return 0;
}

UsersData *UserFile::findAccount(string name)
{
	for( map<string, UsersData>::iterator it= userLevels.begin(); it!=userLevels.end(); it++)
	{
		if( it->second.name == name )
			return &it->second;
	}

	return NULL;
}

#ifdef TEST

UserFile F("../bin/userfile.txt");

int main()
{
	F.readUserFile();
	//F.saveFile();
	return 0;
}

#endif

/**/

