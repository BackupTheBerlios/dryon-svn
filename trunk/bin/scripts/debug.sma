
new n= 0;

do_test(test_name[], bool:what)
{
	new msg[200];
	snprintf(msg, _, "%d> %s : ", n++, test_name);
	if( what )
		strcat(msg, "PASSED");
	else
		strcat(msg, "- FAILED");

	debugPrint("%s\n", msg);
}

/* test suite */
public event_onTest()
{
	{ // string manipulation
		new s1[20];
		new s2[20];

		s1= "aaghjk";
		s2= "aaHn";

		do_test("strmatch 1  ", strmatch("aa", "aa"));
		do_test("strnmatch   ", strnmatch(s1, s2, 2));
		do_test("strmatch 2  ", !strmatch("aa", "AA"));

		s1= "first string";
		strcpy(s2, s1);
		do_test("strcpy 1    ", strmatch(s1, s2));
		strcpy(s2, "");
		do_test("strcpy 2    ", strlen(s2)==0);
		strncpy(s2, s1, 3);
		do_test("strncpy     ", strmatch(s2, "fi"));

		s1= "a blue car";
		substr(s1, 2, 6, s2);
		do_test("substr      ", strmatch(s2, "blue"));
	}

	{ // string array
		new buff[50];
		new string_array:s1, string_array:s2;
		s1= array_create();
		s2= array_create();

		array_add(s1, "un");
		array_add(s1, "deux");

		array_get(s1, 1, buff);

		do_test("array_create", s1!=s2);
		do_test("array_add   ", array_size(s1)==2);
		do_test("array_get   ", strmatch(buff, "deux"));

		array_remove(s1, "un");

		do_test("array_remove", array_size(s1)==1);
	}
}


public event_LoadPlugin()
{
	debugPrint("++ plugin loaded\n");
}

public event_UnloadPlugin()
{
	debugPrint("++ plugin unloaded\n");
}

public event_onConnected()
{
	debugPrint("connected\n");
}

public event_onDisconnected()
{
	debugPrint("disconnected\n");
}

public event_onRegistered()
{
	debugPrint("registered\n");
}

public event_onBotExit()
{
	debugPrint("++ bot exiting\n");
}

public event_onPrivMsg(sender[], dest[], string_array:args)
{
	new cmd[100];

	array_join(args," ", 0, cmd);
	debugPrint("privmsg from %s : \"%s\"\n", sender, cmd);
}

/*** BOT CHANGE ***/
public event_onBotJoined(const channel[], string_array:users)
{
	new msg[300];
	for(new i= 0; i< array_size(users); i++)
	{
		new nick[20];
		array_get(users, i, nick);
		strcat(msg, nick);
		strcat(msg, " ");
	}
	debugPrint("~ Bot Joined \"%s\" user list: %s\n", channel, msg);
}

public event_onBotPart(const channel[])
{
	debugPrint("~ Bot Left \"%s\"\n", channel);
}

public event_onBotOpped(const channel[], const opper[])
{
	debugPrint("~ Bot opped on %s by %s\n", channel, opper);
}

public event_onBotDeopped(const channel[], const caller[])
{
	debugPrint("~ Bot deopped on %s by %s\n", channel, caller);
}

public event_onBotVoiced(const channel[], const voicer[])
{
	debugPrint("~ Bot voiced on %s by %s\n", channel, voicer);
}

public event_onBotDevoiced(const channel[], const caller[])
{
	debugPrint("~ Bot devoiced on %s by %s\n", channel, caller);
}

public event_onBotKicked(const kicker[], const channel[], const kick_msg[])
{
	debugPrint("~ Bot kicked from %s by %s : %s\n", channel, kicker, kick_msg);
}

public event_onBotBanned(const channel[], const caller[], const banmask[])
{
	debugPrint("~ Bot banned from %s by %s\n", channel, caller);
}

public event_onBotUnbanned(const channel[], const caller[])
{
	debugPrint("~ Bot unbanned from %s by %s\n", channel, caller);
}


/*** MODES ****/

public event_onServerOp(const channel[], const nick[])
{
	debugPrint("%s opped by server on %s\n", nick, channel);
}

public event_onServerVoice(const channel[], const nick[])
{
	debugPrint("%s voiced by server on %s\n", nick, channel);
}

public event_onServerRemoveOp(const channel[], const nick[])
{
	debugPrint("%s deopped by server on %s\n", nick, channel);
}

public event_onServerRemoveVoice(const channel[], const nick[])
{
	debugPrint("%s devoiced by server on %s\n", nick, channel);
}

public event_onOp(const channel[], const caller[], const nick[])
{
	debugPrint("%s opped by %s on %s\n", nick, caller, channel);
}

public event_onDeOp(const channel[], const caller[], const nick[])
{
	debugPrint("%s deopped by %s on %s\n", nick, caller, channel);
}

public event_onVoice(const channel[], const caller[], const nick[])
{
	debugPrint("%s voiced by %s on %s\n", nick, caller, channel);
}

public event_onDeVoice(const channel[], const caller[], const nick[])
{
	debugPrint("%s devoiced by %s on %s\n", nick, caller, channel);
}

public event_onBan(const channel[], const caller[], const victim[])
{
	debugPrint("%s banned from %s by %s\n", victim, channel, caller);
}

public event_onUnBan(const channel[], const caller[], const victim[])
{
	debugPrint("%s unbanned from %s by %s\n", victim, channel, caller);
}

public event_onChanKeySet(const channel[], const caller[], const key[])
{
	debugPrint("%s set key %s for channel %s\n", caller, key, channel);
}

public event_onChanKeyRemoved(const channel[], const caller[])
{
	debugPrint("%s removed key for channel %s\n", caller, channel);
}

public event_onChanMode(const caller[], const chan[], const mode[])
{
	debugPrint("%s changed mode for channel %s : %s\n", caller, chan, mode);
}

/*********************************/

public event_onAction(const sender[], const action[])
{
	debugPrint("[ACTION from '%s'] %s\n", sender, action);
}

public event_onTopicChange(const caller[], const chan[], const old_topic[], const new_topic[])
{
	debugPrint("[TOPIC] %s= \"%s\" changed by %s\n", chan, new_topic, caller);
}

public event_onKick(kicker[], channel[], victim[], reason[])
{
	debugPrint("%s kicked %s from %s : %s\n", kicker, victim, channel, reason);
}

public event_onNickChange(oldnick[], newnick[])
{
	debugPrint("%s now known as %s\n", oldnick, newnick);
}

public event_onJoin(nick[], channel[])
{
	debugPrint("%s joined channel %s\n", nick, channel);
}

public event_onPart(nick[], channel[])
{
	debugPrint("%s leaving channel %s\n", nick, channel);
}

public event_onQuit(nick[])
{
	debugPrint("%s quit IRC\n", nick);
}


/**/

