/*
	Clan channel management script
	written by Anthalir also known as Schmurfy

	Here is the script i use to manage matchs for my clan
	Any registered user can add itself to a match, remove, currently there is no
	command to register a new match since i use it with a php website which use the same
	MySQL database.

	Here is some explanations on how to make it work:
	- first run the bot with this script once in test mode ( --test )
	- go to the folder where the script is and edit warbot.dat to fit your configuration
	- edit your userfile.txt to create necessary account for each of your clan members (give them o flag)
	- Now you need to create necessary entry in the database, you can do this with phpMyAdmin:
-------------------------------------------------------------
CREATE TABLE `lans` (
  `id` bigint(20) unsigned NOT NULL auto_increment,
  `Nom` varchar(100) NOT NULL default '',
  `Date` datetime NOT NULL default '0000-00-00 00:00:00',
  `Duree` tinyint(4) NOT NULL default '0',
  `Lieu` varchar(100) NOT NULL default '',
  `Prix` tinyint(4) NOT NULL default '0',
  `Site_Web` varchar(100) NOT NULL default '',
  `Organisateur` varchar(30) NOT NULL default '',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

CREATE TABLE `link` (
  `id` bigint(20) unsigned NOT NULL auto_increment,
  `user` bigint(20) unsigned NOT NULL default '0',
  `match` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `id` (`id`)
) TYPE=MyISAM PACK_KEYS=0;

CREATE TABLE `link_lans` (
  `id` bigint(20) unsigned NOT NULL auto_increment,
  `user` bigint(20) unsigned NOT NULL default '0',
  `lan` bigint(20) unsigned NOT NULL default '0',
  `type` smallint(6) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

CREATE TABLE `logs` (
  `id` bigint(20) NOT NULL auto_increment,
  `User` bigint(20) NOT NULL default '0',
  `Date` datetime NOT NULL default '0000-00-00 00:00:00',
  `Action` tinytext NOT NULL,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

CREATE TABLE `matchs` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `Adversaires` tinytext NOT NULL,
  `Type` enum('t','m') NOT NULL default 't',
  `Type_Comment` tinytext NOT NULL,
  `Date` datetime NOT NULL default '0000-00-00 00:00:00',
  `Etat` char(1) NOT NULL default 'c',
  `Maps` tinytext NOT NULL,
  `Score` varchar(5) default NULL,
  `Server` tinytext NOT NULL,
  `Password` varchar(30) NOT NULL default '',
  `Comments` text NOT NULL,
  `Mercos` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `id` (`id`),
  KEY `Date` (`Date`)
) TYPE=MyISAM PACK_KEYS=0;

CREATE TABLE `misc` (
  `name` tinytext NOT NULL,
  `value` text NOT NULL
) TYPE=MyISAM;

CREATE TABLE `users` (
  `id` bigint(20) unsigned NOT NULL auto_increment,
  `account` tinytext NOT NULL,
  `level` tinyint(4) NOT NULL default '3',
  `password` varchar(250) NOT NULL default '',
  `server` varchar(20) NOT NULL default 'none',
  `type` char(1) NOT NULL default 'm',
  `prenom` varchar(20) NOT NULL default '',
  `date_birth` date NOT NULL default '0000-00-00',
  `ville` varchar(30) NOT NULL default '',
  `photo` tinytext NOT NULL,
  `telephone` varchar(10) NOT NULL default '',
  `SteamID` varchar(20) NOT NULL default '',
  `email` varchar(150) NOT NULL default '',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `id` (`id`),
  KEY `type` (`type`)
) TYPE=MyISAM PACK_KEYS=0;

---------------------------------------------------

	- now create an entry for each clan member in users table with their account name from usrfile.txt
		some files in this database structure are no longer used, or not yet used so just use what you need
		some things are only used by my php website
	- Maybe you should go to event_onConnected() definition and comment/remove the bot commands you don't want
	- You are now ready to use this script, when loaded the script will try to connect to database using
		config file data, if it fails the script will be disabled.
*/
#include <mysql>

new mysql_db:db;

// if defined scipt will use topic to display next match

new chan_pv[50]= "#", chan_pub[50]= "#";
new chan_pv_pass[50]= "#";
new bool:auth_bot= false;
new auth_login[50]= "xxx", auth_pass[50]= "xxx";
new bool:use_topic= false;


// called when plugin is loaded
public event_LoadPlugin()
{
	new db_login[30], db_pass[30], db_host[50], db_name[30];
	loadString(chan_pv);
	loadString(chan_pub);
	loadString(chan_pv_pass);

	loadString(db_name);
	loadString(db_login);
	loadString(db_pass);
	loadString(db_host);

	loadString(auth_login);
	loadString(auth_pass);
	// boolean are stored as integer (0 or 1)
	loadInt(auth_bot);
	loadInt(use_topic);

	// no config file
	if( strmatch(chan_pv, "#") )
	{
		saveString(chan_pv);
		saveString(chan_pub);
		saveString(chan_pv_pass);

		saveStringEx("db_name", "xxx");
		saveStringEx("db_login", "xxx");
		saveStringEx("db_pass", "xxx");
		saveStringEx("db_host", "xxx");

		saveStringEx("site_url", "xxx");

		saveString(auth_login);
		saveString(auth_pass);

		saveInt(auth_bot);
		saveInt(use_topic);
		debugPrint("NO CONFIG FILE !\n");
		disablePlugin();
	}
	else
	{
		debugPrint("auth_bot: %s\n", auth_bot?"true":"false");
		debugPrint("use_topic: %s\n", use_topic?"true":"false");

		if( !mysql_connect(db, db_host, db_login, db_pass, db_name) )
		{
			debugPrint("Unable to connect to database, plugin disabled !\n");
			disablePlugin();
		}
	}
}

// called when plugin is unloaded
public event_UnloadPlugin()
{
	mysql_close(db);
}

public event_onBotJoined(const channel[])
{

	if( strmatch(channel, chan_pv) )
		updateTopic();

}

// connection registered and bot authed (Qnet)
public event_onRegistered()
{
	if( auth_bot )
	{
		irc_join(chan_pv, chan_pv_pass);
		irc_join(chan_pub);
	}
}


public event_onJoin(const nick[], const channel[])
{
	new account[50];
	new mysql_res:res;

	// if user has no account on bot, do nothing
	if( !irc_getAccount(nick, account) )
		return;

	debugPrint("account of %s: %s\n", nick, account);

	res= mysql_query(db, "SELECT * FROM users WHERE account='%s'", account);
	// if user has an account but not registered in database
	if( res != no_res )
	{
		mysql_free_result(res);

		if( strmatch(channel, chan_pv) )
		{
			// display next match infos
			res= mysql_query(db, "SELECT * FROM matchs WHERE (Type='t' OR Type='m' ) AND Etat='c' AND Date - NOW() > 0 ORDER BY Date LIMIT 1");
			showMatchFromDBResult(res, nick);
		}
	}
}

// just a stupid but funny thing
public event_onAction(const nick[], const msg[])
{
	if( strmatch(msg, "donne un cookie à o|Bot") )
	{
		irc_action(chan_pv, "mange le cookie");
		return;
	}
}

/*
i used this to announce match every 10 minutes if less than 1 hour is left before the match starts
not currently used since i heavily changed to timer interface since
public AnnounceMatchs()
{

	new d[35], d2[35], adv[20], maps[40];
	new date:match_day, date:today;

	mysql_query(db, "SELECT * FROM matchs WHERE (TO_DAYS(Date) - TO_DAYS(NOW()) = 0) AND (TIME_TO_SEC(Date) - TIME_TO_SEC(NOW()) < 3600) AND (TIME_TO_SEC(Date) - TIME_TO_SEC(NOW()) > 0) ORDER BY Date");

	for(new i= 0; i< mysql_count_results(db); i++)
	{
		mysql_get_field(db, 0, "Date", d);
		mysql_get_DATEfield(db, 0, "Date", d2, 35, "%H:%M");
		mysql_get_field(db, 0, "Adversaires", adv);
		mysql_get_field(db, 0, "Maps", maps);

		match_day= makeDate(d);
		today= getCurrentDate();

		irc_say(CHAN_PV, "\003;00\002;[ \003;07Info:\002;\003;15 Match contre \003;04%s\003;15 dans \003;04%d\003;15 minutes \003;14(%s) (%s)\003;00 \002;]\002;", adv, getDateDiff(match_day, today, 'm'), maps, d2);
	}
}
*/

// list all future matchs in database
public func_listmatchs(const caller[], const dest[], string_array:params)
{
	new mysql_res:res;

	res= mysql_query(db, "SELECT * FROM matchs WHERE (Type='t' OR Type='m' ) AND Etat='c' AND Date - NOW() > 0 ORDER BY Date");
	if( res == no_res )
		return;

	showMatchFromDBResult(res, caller);
}

// list users in database
public func_userlist(const caller[], const dest[], string_array:params)
{
	new mysql_res:res;
	new account[20],id[10];

	res= mysql_query(db, "SELECT * FROM users");
	if( res != no_res )
	{
		for(new i= 0; i< mysql_count_results(res); i++)
		{
			mysql_get_field(res, i, "id", id);
			mysql_get_field(res, i, "account", account);

			irc_notice(caller, "(%s) [\003;09%s\003;]", id, account);
		}

		mysql_free_result(res);
	}
	else
	{
		irc_notice(caller, "no account");
	}
}

// display current match (the match actually played)
public func_match(const caller[], const dest[], string_array:params)
{
	new mysql_res:res;
	// SELECT * FROM `matchs` WHERE Etat!='t' AND Date - NOW() < 0;
	// SELECT * FROM `matchs` WHERE Etat='c' AND Date - NOW() < 0 ORDER BY Date DESC LIMIT 1
	//mysql_query(db, "SELECT * FROM `matchs` WHERE Etat='c' AND Date LIKE CONCAT(CURDATE(),'%%')");
	res= mysql_query(db, "SELECT * FROM `matchs` WHERE Etat='c' AND Date - NOW() < 0 ORDER BY Date DESC LIMIT 1");
	showMatchFromDBResult(res, caller);
}

// set score for last played match
public func_score(const caller[], const dest[], string_array:params)
{
	new id[10];
	new score[10];
	new mysql_res:res, mysql_res:res2;
	new players;

	if( array_size(params) < 1 )
		return -1;

	array_get(params, 0, score);

	res= mysql_query(db, "SELECT * FROM `matchs` WHERE Etat='c' AND Date - NOW() < 0 ORDER BY Date DESC LIMIT 1");
	if( res != no_res )
	{
		new max_player_count= 6;

		loadInt(max_player_count);

		mysql_get_field(res, 0, "id", id);

		res2= mysql_query(db, "SELECT * FROM link WHERE `match`='%s'", id);
		players= mysql_count_results(res2);
		mysql_free_result(res2);

		if( players <= max_player_count )
		{
			mysql_query(db, "UPDATE `matchs` SET Score='%s', Etat='t' WHERE id='%s'", score, id);
			irc_notice(caller, "score for match %s : %s", id, score);
		}
		else
		{
			irc_notice(caller, "too many players registered on this match (remove some)");
		}

		mysql_free_result(res);
	}
	else
	{
		irc_notice(caller, "no match");
	}

	return 0;
}

// display informations for the next match
public func_nextmatch(const caller[], const dest[], string_array:params)
{
	new mysql_res:res;

	res= mysql_query(db, "SELECT * FROM matchs WHERE (Type='t' OR Type='m' ) AND Etat='c' AND Date - NOW() > 0 ORDER BY Date LIMIT 1");
	showMatchFromDBResult(res, caller);
}

// display informations on a given match
public func_info(const caller[], const dest[], string_array:params)
{
	new id[10];
	new mysql_res:res;

	if( array_size(params) != 1 )
		return -1;

	array_get(params, 0, id);

	res= mysql_query(db, "SELECT * FROM matchs WHERE id='%s'", id);
	showMatchFromDBResult(res, caller);
	return 0;
}

// display information for last played match (last match with score set)
public func_lastmatch(const caller[], const dest[], string_array:params)
{
	new mysql_res:res;
	new count[10]= "1";

	if( array_size(params) >= 1 )
		array_get(params, 0, count);

	res= mysql_query(db, "SELECT * FROM matchs WHERE (Type='t' OR Type='m' ) AND Etat='t' ORDER BY Date DESC LIMIT %s", count);
	showMatchFromDBResult(res, caller);
}

// just the website url
public func_url(const caller[], const dest[], string_array:params)
{
	new site_url[100];
	loadString(site_url);
	irc_notice(caller, site_url);
}

// remove a user from a match playerlist
public func_remove(const caller[], const dest[], string_array:params)
{
	new adv[50];
	new num[10];
	new account[30];
	new userid[10];
	new mysql_res:res;

	if( array_size(params) < 1 )
		return -1;

	array_get(params, 0, num);

	// user has no account
	if( irc_getAccount(caller, account) )
	{

		// retrieve user id
		res= mysql_query(db, "SELECT * FROM `users` WHERE account='%s'", account);
		if( res != no_res )
		{

			mysql_get_field(res, 0, "id", userid);
			mysql_free_result(res);

			// user don' want to add himself
			// !add <match_id> <user>
			if( strmatch(caller, "o|Schmurfy") && array_size(params) == 2 )
			{
				array_get(params, 1, account);
				res= mysql_query(db, "SELECT * FROM `users` WHERE account=\"%s\"", account );
				if( res != no_res )
				{
					mysql_get_field(res, 0, "id", userid);
					mysql_free_result(res);
				}
				// user not found, maybe his current nick was given instead of his account
				else
				{
					new bool:err;
					new real_account[30];

					err= !irc_getAccount(account, real_account);
					if( !err )
					{
						res= mysql_query(db, "SELECT * FROM `users` WHERE account=\"%s\"", real_account );
						if( res != no_res )
						{
							mysql_get_field(res, 0, "id", userid);
							mysql_free_result(res);
						}
						// not found again, user is really unknow so stop now
						else
						{
							err= true;
						}
					}

					if( err )
					{
						irc_notice(caller, "unknow user: %s", account);
						return 0;
					}
				}
			}


			if( res != no_res )
			{
				new mysql_res:r2= mysql_query(db, "SELECT * FROM matchs WHERE id=\"%s\" AND Etat='c'", num);
				mysql_get_field(r2, 0, "Adversaires", 	adv);
				mysql_free_result(r2);

				mysql_query(db, "DELETE FROM link WHERE user='%s' AND `match`='%s'", userid, num);
				irc_notice(caller, "%s removed from match %s vs %s", account, num, adv);
				updateTopic();
			}
		}
	}

	return 0;
}

// add a user to a match playerlist
public func_add(const caller[], const dest[], string_array:params)
{
	new adv[50];
	new num[10];
	new account[30];
	new userid[10];
	new mysql_res:res;

	if( array_size(params) < 1 )
		return -1;

	array_get(params, 0, num);

	// user has no account
	if( !irc_getAccount(caller, account) )
	{
		irc_notice(caller, "you have no account (in bot userfile) !");
		return 0;
	}

	// retrieve user id (and check if user is valid)
	res= mysql_query(db, "SELECT * FROM `users` WHERE account=\"%s\"", account);
	if( res == no_res )
	{
		irc_notice(caller, "You dont have required access (DB)");
		return 0;
	}


	mysql_get_field(res, 0, "id", userid);
	mysql_free_result(res);


	// user don' want to add himself
	// !add <match_id> <user>
	if( array_size(params) == 2 )
	{
		array_get(params, 1, account);
		res= mysql_query(db, "SELECT * FROM `users` WHERE account=\"%s\"", account );
		if( res != no_res )
		{
			mysql_get_field(res, 0, "id", userid);
			mysql_free_result(res);
		}
		// user not found, maybe his current nick was given instead of his account
		else
		{
			new bool:err;
			new real_account[30];

			err= !irc_getAccount(account, real_account);
			if( !err )
			{
				res= mysql_query(db, "SELECT * FROM `users` WHERE account=\"%s\"", real_account );
				if( res != no_res )
				{
					mysql_get_field(res, 0, "id", userid);
					mysql_free_result(res);
				}
				// not found again, user is really unknow so stop now
				else
				{
					err= true;
				}
			}

			if( err )
			{
				irc_notice(caller, "utilisateur inconnu: %s", account);
				return 0;
			}
		}
	}


	// add to today match
	if( strmatch(num, "0") )
	{
		res= mysql_query(db, "SELECT * FROM `matchs` WHERE Date LIKE CONCAT(CURDATE(),'%%')");
		// if no match found, create one
		if( res == no_res )
		{
			mysql_query(db, "INSERT INTO matchs(Adversaires,Type,Type_Comment,Date,Etat,Maps,Server,Comments) VALUES('?','m','pcw',CONCAT(CURDATE(), ' 21:00:00'),'c','?/?','','')");
			res= mysql_query(db, "SELECT MAX(id) FROM matchs");
			mysql_get_field(res, 0, "MAX(id)", num);
			mysql_free_result(res);
			res= mysql_query(db, "SELECT * FROM matchs WHERE id=\"%s\" AND Etat='c'", num);
		}
	}
	else
	{
		res= mysql_query(db, "SELECT * FROM matchs WHERE id=\"%s\" AND Etat='c'", num);
	}

	if( res == no_res )
	{
		irc_notice(caller, "no match with id: %s", num);
		return 0;
	}

	mysql_get_field(res, 0, "Adversaires", adv);
	mysql_get_field(res, 0, "id", num);

	mysql_free_result(res);

	res= mysql_query(db, "SELECT * FROM link WHERE link.user='%s' AND link.match='%s'", userid, num);
	if( res != no_res )
	{
		irc_notice(caller, "%s already registered for match: %s", account, num);
		mysql_free_result(res);
		return 0;
	}

	mysql_query(db, "INSERT INTO link(link.user,link.match) VALUES('%s','%s')", userid, num);
	irc_notice(caller, "%s added in match %s vs %s", account, num, adv);

	updateTopic();
	showMatch(caller, num);
	return 0;
}

public func_delmatch(const caller[], const dest[], string_array:params)
{
	new id[10];

	if( array_size(params) != 1 )
		return -1;

	array_get(params, 0, id);

	mysql_query(db, "DELETE FROM link WHERE `match`='%s'", id);
	mysql_query(db, "DELETE FROM `matchs` WHERE id='%s'", id);

	irc_notice(caller, "match removed");
	updateTopic();
	return 0;
}

// !addmatch <adversaires> <date> <heure>
public func_addmatch(const caller[], const dest[], string_array:params)
{

}

// !edit <match_id> team <team_name>
public func_editmatch(const caller[], const dest[], string_array:params)
{
	new id[5];
	new what[20];
	new team_name[50];
	new mysql_res:res;

	if( array_size(params) < 3 )
		return -1;

	array_get(params, 0, id);
	array_get(params, 1, what);
	array_get(params, 2, team_name);

	res= mysql_query(db, "SELECT * FROM matchs WHERE id='%s'", id);
	if( res == no_res )
	{
		irc_notice(caller, "no match with id %s !", id);
		return 0;
	}

	mysql_free_result(res);

	if( strmatch(what, "team") )
	{
		strcpy(what, "Adversaires");
	}
	else if( strmatch(what, "maps") )
	{
		strcpy(what, "Maps");
	}
	else
	{
		return -1;
	}

	debugPrint("[%s] new '%s': %s\n", id, what, team_name);

	mysql_query(db, "UPDATE matchs SET %s='%s' WHERE id='%s'", what, team_name, id);
	irc_notice(caller, "Match %s updated", id);
	return 0;
}

// show data for a match
showMatch(const dest[], const num[])
{
	new mysql_res:res;

	res= mysql_query(db, "SELECT * FROM matchs WHERE (Type='t' OR Type='m' ) AND Etat='c' AND Date - NOW() > 0 AND id='%s' ORDER BY Date", num);
	showMatchFromDBResult(res, dest);
}

// read data from result of latest mysql query
showMatchFromDBResult(mysql_res:res, const dest[])
{
	new mysql_res:res2;
	new id[10], adv[20], type[20], d[35], d2[35], maps[30], state[5];
	new comments[100], server[30], password[20], score[10];
	new date:match_day, date:today;
	new users[400];
	new i, j, count;

	new bool:fulldisp= true;

	if( res == no_res )
	{
		irc_notice(dest, "aucun match");
		return;
	}

	count= mysql_count_results(res);

	if( count==0 )
	{
		irc_notice(dest, "aucun match <erreur>");
		mysql_free_result(res);
		return;
	}

	if( count > 1 )
		fulldisp= false;


	for(i= 0; i< count; i++)
	{
		mysql_get_field(res, i, "id", id);
		mysql_get_field(res, i, "Adversaires", adv);
		mysql_get_field(res, i, "Type_Comment", type);
		mysql_get_field(res, i, "Maps", maps);
		mysql_get_field(res, i, "Date", d);
		mysql_get_field(res, i, "Score", score);
		mysql_get_field(res, i, "Etat", state);

		if( fulldisp )
		{
			mysql_get_field(res, i, "Comments", comments);
			mysql_get_field(res, i, "Server", server);
			mysql_get_field(res, i, "Password", password);

			if( strlen(password) == 0 ) strcpy(password, "?");
		}

		match_day= makeDate(d, false);	// remove the time data
		today= getCurrentDate(false);	// get current day

		mysql_get_DATEfield(res, i, "Date", d, 35, " %H:%M");

		switch( getDateDiff(match_day, today, 'd') )
		{
/*
		case -2: strcpy(d2, "avant-hier");
		case -1: strcpy(d2, "hier");
		case  0: strcpy(d2, "aujourd'hui");
		case  1: strcpy(d2, "demain");
		case  2: strcpy(d2, "après demain");
*/
		default: mysql_get_DATEfield(res, i, "Date", d2, 35, "%A %d %B");
		}


		res2= mysql_query(db, "SELECT users.* FROM users,link WHERE users.id=link.user AND link.match='%s' ORDER BY link.id", id);
		j= 0;
		if( res2 != no_res )
		{
			for( ; j< mysql_count_results(res2); j++)
			{
				new nick[20];
				mysql_get_field(res2, j, "account", nick, 20);
				strcat(users, nick);

				if( j< mysql_count_results(res2) - 1 )
					strcat(users, ", ");
			}

			mysql_free_result(res2);
		}

		irc_notice(dest, "(\003;09%s\003;) [\003;07%s%s\003;] match vs %s (%s) (%s) (%d registered players)", id, d2, d, adv, type, maps, j);

		if( fulldisp )
		{
			if( j>0 ) 					irc_notice(dest, "Lineup: %s", users);
			if( strlen(comments) > 0 ) 	irc_notice(dest, "Comments: %s", comments);
			if( (strlen(server) > 0) && (state[0]=='c'))	irc_notice(dest, "server: %s , password: %s", server, password);
		}

		if( strlen(score) > 0 )	irc_notice(dest, "Score: %s", score);
	}

	mysql_free_result(res);
}

// change channel topic if needed
updateTopic()
{
	if( use_topic )
	{
		new i;
		new users[100], nick[20];
		new id[10], adv[50], d[20];
		new mysql_res:res;

		res= mysql_query(db, "SELECT * FROM matchs WHERE (Type='t' OR Type='m' ) AND Etat='c' AND Date - NOW() > 0 ORDER BY Date");
		if( res==no_res )
		{
			irc_setTopic(chan_pv, "no match");
			return;
		}

		mysql_get_field(res, 0, "id", id);
		mysql_get_field(res, 0, "Adversaires", adv);
		mysql_get_DATEfield(res, 0, "Date", d, 20, "%d/%m %H:%M");

		mysql_free_result(res);

		res= mysql_query(db, "SELECT users.* FROM users,link WHERE users.id=link.user AND link.match='%s' ORDER BY link.id", id);
		if( res!=no_res )
		{
			for( i= 0; i< mysql_count_results(res); i++)
			{
				mysql_get_field(res, i, "account", nick);
				strcat(users, nick);
				strcat(users, " - ");
			}

			mysql_free_result(res);
		}

		irc_setTopic(chan_pv, "[(%s) Match vs %s %s : %s ]", id, adv, d, users);
	}
}

// show list of steamids (from database)
public func_steamid(const caller[], const dest[], string_array:params)
{
	new mysql_res:res;
	new steam_id[20],account[20];

	res= mysql_query(db, "SELECT * FROM users");
	if( res == no_res )
	{
		irc_notice(caller, "no account");
		return;
	}

	for(new i= 0; i< mysql_count_results(res); i++)
	{
		mysql_get_field(res, i, "account", account);
		mysql_get_field(res, i, "SteamID", steam_id);

		if( strlen(steam_id) > 0 )
			irc_notice(caller, "%s => %s", steam_id, account);
	}

	mysql_free_result(res);
}

// i use this to retrieve a list of all member phones
public func_phones(const caller[], const dest[], string_array:params)
{
	new mysql_res:res;
	new tel[20],account[20];

	res= mysql_query(db, "SELECT * FROM users");
	if( res == no_res )
	{
		irc_notice(caller, "no account");
		return;
	}

	for(new i= 0; i< mysql_count_results(res); i++)
	{
		mysql_get_field(res, i, "account", account);
		mysql_get_field(res, i, "telephone", tel);

		if( strlen(tel) > 0 )
			irc_notice(caller, "%s => %s", tel, account);
	}

	mysql_free_result(res);
}

// list planned lans
// a big part of lans management tools is on the website part, maybe you shouldn't use this
// and it's a feature i implemented recently so it may not work well for you
public func_lans(const caller[], const dest[], string_array:params)
{
	new tmp_str[300];
	new string_array:inscris, string_array:voitures, string_array:incertains;
	new mysql_res:res, mysql_res:r2;
	new id[10], Nom[100], d[50], Duree[5], Lieu[100], Prix[5], WebSite[200], Orga[50];

	res= mysql_query(db, "SELECT * FROM lans WHERE Date - NOW() > 0 ORDER BY id");
	if( res == no_res )
	{
		irc_notice(caller, "no planned lan");
		return;
	}

	inscris= array_create();
	voitures= array_create();
	incertains= array_create();

	for(new i= 0; i< mysql_count_results(res); i++)
	{
		mysql_get_field(res, i, "id", id);
		mysql_get_field(res, i, "Nom", Nom);
		mysql_get_field(res, i, "Duree", Duree);
		mysql_get_field(res, i, "Lieu", Lieu);
		mysql_get_field(res, i, "Prix", Prix);
		mysql_get_field(res, i, "Site_Web", WebSite);
		mysql_get_field(res, i, "Organisateur", Orga);
		mysql_get_DATEfield(res, i, "Date", d, 20, "%d/%m/%Y");

		irc_notice(caller, "(%s) [%s] [%s] [Durée: %s jour(s)] [ %s ]", id, Nom, d, Duree, WebSite);

		r2= mysql_query(db, "SELECT users.*, link_lans.type FROM users,link_lans WHERE users.id=link_lans.user AND link_lans.lan='%s' ORDER BY link_lans.id", id);
		for(new j= 0; j< mysql_count_results(r2); j++)
		{
			new type[10], account[20];
			mysql_get_field(r2, j, "type", type);
			mysql_get_field(r2, j, "account", account);

			if( strmatch(type, "1") || strmatch(type, "2") )
				array_add(inscris, account);

			if( strmatch(type, "1") )
				array_add(voitures, account);

			if( strmatch(type, "0") )
				array_add(incertains, account);
		}

		array_join(inscris, ", ", 0, tmp_str);
		irc_notice(caller, "Peuvent venir: %s", tmp_str);

		array_join(voitures, ", ", 0, tmp_str);
		irc_notice(caller, "Voiture: %s", tmp_str);

		array_join(incertains, ", ", 0, tmp_str);
		irc_notice(caller, "Incertains: %s", tmp_str);
	}

	array_destroy(inscris);
	array_destroy(voitures);
	array_destroy(incertains);

	mysql_free_result(res);
}

public event_onConnected()
{
	if( auth_bot )
	{
		irc_privmsg("Q@CServe.quakenet.org", "AUTH %s %s", auth_login, auth_pass);
		irc_setUserMode("+x");
	}

	irc_setUserMode("+i");

	//addTimedEvent("AnnounceMatchs", 10, 0);

	// admin

	registerCommand("!delmatch", "func_delmatch", 'm');
	//registerCommand("!addmatch", "func_addmatch", 'm');

	// matchs
	registerCommand("!score",		"func_score",		'o', "!score <nous-eux>");
	registerCommand("!match", 		"func_match", 		'o', "!match");
	registerCommand("!nextmatch", 	"func_nextmatch", 	'v', "!nextmatch");
	registerCommand("!lastmatch", 	"func_lastmatch", 	'v', "!lastmatch");
	registerCommand("!listmatchs", 	"func_listmatchs", 	'v', "!listmatchs");
	registerCommand("!edit", 		"func_editmatch", 	'o', "!edit <matchid> <what> <value> (what= maps or team)");
	registerCommand("!info", 		"func_info", 		'v', "!info <match_id>");
	registerCommand("!add", 		"func_add", 		'v', "!add <match_id> [user] (0= match du jour, en crée un si aucun)");
	registerCommand("!remove", 		"func_remove", 		'v', "!remove <match_id>");


	// lans
	registerCommand("!lans",		"func_lans",		'o', "!lans");


	registerCommand("!steamid",		"func_steamid",		'o', "!steamid");
	registerCommand("!tels",		"func_phones",		'o', "!tels");
	registerCommand("!userlist", 	"func_userlist", 	'o', "!userlist");
	registerCommand("!url", 		"func_url", 		'v', "!url");

	registerCommand("!tape", 		"func_tape", 		'v');
	registerCommand("salut", 		"func_salut", 		'v');

	if( !auth_bot )
	{
		irc_join(chan_pv, chan_pv_pass);
		irc_join(chan_pub);
	}
}

// another stupid thing
public func_salut(const caller[], const dest[], string_array:params)
{
	if( array_size(params) == 0 )
		irc_say(dest, "salut %s", caller);
}

// and the last stupid thing :p
public func_tape(const caller[], const dest[], string_array:params)
{
	new nick[50];

	if( array_size(params) != 1)
		return -1;

	array_get(params, 0, nick);

	for(new i= 0; i< strlen(nick); i++)
	{
		if( nick{i} > 126 )
			nick{i}= ' ';
	}

	irc_action(dest, "tape %s avec un balai", nick);

	return 0;
}


/**/

