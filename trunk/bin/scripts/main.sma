public func_nick(const caller[], const dest[], string_array:params)
{
	new nick[30];

	if( array_size(params) != 1 )
		return -1;

	array_get(params, 0, nick);
	irc_changeNick(nick);
	return 0;
}

public func_join(const caller[], const dest[], string_array:params)
{
	new chan[50];
	new pass[50]= "";

	if( array_size(params) == 0 )
		return -1;

	array_get(params, 0, chan);
	if( array_size(params) == 2 )
		array_get(params, 1, pass);

	irc_join(chan, pass);
	return 0;
}

public func_part(const caller[], const dest[], string_array:params)
{
	new chan[50];
	new ret= -1;


	if( irc_isChanName(dest) )
	{
		irc_part(dest);
		ret= 0;
	}
	else if( array_size(params) >= 1 )
	{
		array_get(params, 0, chan);
		irc_part(chan);
		ret= 0;
	}

	return ret;
}

public func_action(const caller[], const dest[], string_array:params)
{
	new chan[50];
	new buff[100];

	if( array_size(params) < 2 )
		return -1;

	array_get(params, 0, chan);
	array_join(params, " ", 1, buff);
	irc_action(chan, buff);

	return 0;
}

public func_edituser(const caller[], const dest[], string_array:params)
{
	new what[100], flags[10];

	if( array_size(params) != 2 )
		return -1;

	array_get(params, 0, what);
	array_get(params, 1, flags);

	if( what[0] == '#' )
	{
		new auth[50];
		new tmp[52];
		substr(what, 1, strlen(what), auth);
		snprintf(tmp, _, "a/%s", auth);

		if( editUser(tmp, flags) == -1 )
			irc_notice(caller, "auth <%s> not found in user list", auth);
		else
			irc_notice(caller, "modification done.");
	}
	else
	{
		if( editUser(what, flags) == -1 )
			irc_notice(caller, "hostmask <%s> not found in user list", what);
		else
			irc_notice(caller, "modification done.");
	}

	return 0;
}

public func_deluser(const caller[], const dest[], string_array:params)
{
	new what[100];

	if( array_size(params) != 1 )
		return -1;

	array_get(params, 0, what);

	// auth
	if( what[0] == '#' )
	{
		new auth[50];
		new tmp[52];
		substr(what, 1, strlen(what), auth);
		snprintf(tmp, _, "a/%s", auth);

		delUser(tmp);
		irc_notice(caller, "auth <%s> removed from user list", auth);
	}
	else
	{
		delUser(what);
		irc_notice(caller, "hostmask <%s> removed from user list", what);
	}

	return 0;
}

public func_adduser(const caller[], const dest[], string_array:params)
{
	new name[50], what[100], flags[10];

	if( array_size(params) != 3 )
		return -1;

	array_get(params, 0, name);
	array_get(params, 1, what);
	array_get(params, 2, flags);

	// add by auth
	if( what[0] == '#' )
	{
		new auth[50];
		new tmp[52];

		substr(what, 1, strlen(what), auth);
		snprintf(tmp, _, "a/%s", auth);

		if( addUser(name, tmp, flags) == -1)
			irc_notice(caller, "auth <%s> is already registered in user list !", auth);
		else
			irc_notice(caller, "user added.");
	}
	else
	{
		if( addUser(name, what, flags) == -1)
			irc_notice(caller, "hostmask <%s> is already registered in user list !", what);
		else
			irc_notice(caller, "user added.");
	}

	return 0;
}

public event_onBotKicked(const kicker[], const channel[], const kick_msg[])
{
	new key[50];

	irc_getChanKey(channel, key);
	debugPrint("%s kicked me from %s, rejoining\n", kicker, channel);
	irc_join(channel, key);
}

public event_onBotBanned(const channel[], const caller[], const banmask[])
{
	irc_unban(channel, banmask);
}

public event_onConnected()
{
	registerCommand(":nick", 	"func_nick", 		'n', ":nick <new_nick>");
	registerCommand(":join", 	"func_join", 		'n', ":join <chan> <pass>");
	registerCommand(":part", 	"func_part", 		'n', ":part [<chan>]");
	registerCommand(":action", 	"func_action", 		'n', ":action <chan> <action>");

	registerCommand(":adduser",	"func_adduser",		'm', ":adduser <account> <hostmask>/<#auth> <flags>");
	registerCommand(":deluser", "func_deluser",		'm', ":deluser <hostmask>/<#auth>");
	registerCommand(":edituser","func_edituser",	'm', ":edituser <hostmask>/<#auth> <newflags>" );
}


/**/




