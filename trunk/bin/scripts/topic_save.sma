/*
Action: load/save topics and can enforce them
Coder: Anthalir
Comments: it can handle any number of chans

The script can be used to save and force a given topic:
	":force #chan 1"
Or to auto save the topic and set it if it is removed after a netsplit or i don't know what :
	":autosave #chan 1"

if both are set autosave should have total priority on forced topic.
*/

#define TOPIC_SIZE 		300
// max channel name found by trial and error since it didn't found it anywhere
// for QuakeNet the limit is near 90 character, it's huge :x
#define CHANNAME_SIZE 	90
#define VAR_SIZE 		CHANNAME_SIZE+10


saveTopic(const channel[])
{
	new varname[VAR_SIZE];
	new topic[TOPIC_SIZE];
	snprintf(varname, _, "topic_%s", channel);
	irc_getTopic(channel, topic);
	saveStringEx(varname, topic);
}

public event_onBotOpped(const channel[], const opper[])
{
	//irc_setChanMode(channel, "+t");
}

public event_onTopicChange(const caller[], const chan[], const old_topic[], const new_topic[])
{
	new topic[TOPIC_SIZE], botnick[30];
	new varname[VAR_SIZE], varname2[VAR_SIZE];
	new bool:enforce_topic= false;
	new bool:autosave_topic= false;

	irc_getBotNick(botnick);

	// do not intercept topic from me
	// or we have a nice endless loop :)
	if( !strmatch(caller, botnick) )
	{
		snprintf(varname, _, "enforce_%s", chan);
		snprintf(varname2, _, "autosave_%s", chan);

		loadIntEx(varname, enforce_topic);
		loadIntEx(varname2, autosave_topic);

		if( autosave_topic )
		{
			saveTopic(chan);
		}
		else if( enforce_topic )
		{
			snprintf(varname, _, "topic_%s", chan);
			loadStringEx(varname, topic);
			irc_setTopic(chan, topic);
		}
	}
}

public event_onBotJoined(const chan[])
{
	new topic[TOPIC_SIZE];
	new varname[VAR_SIZE], varname2[VAR_SIZE];
	new bool:enforce_topic= false;
	new bool:autosave_topic= false;

	snprintf(varname, _, "enforce_%s", chan);
	snprintf(varname2, _, "autosave_%s", chan);

	loadIntEx(varname, enforce_topic);
	loadIntEx(varname2, autosave_topic);

	irc_getTopic(chan, topic);

	if( (autosave_topic && (strlen(topic) == 0)) || enforce_topic )
	{
		snprintf(varname, _, "topic_%s", chan);
		loadStringEx(varname, topic);
		irc_setTopic(chan, topic);
	}
}

// save topic everytime it is changed
public func_autosave(const caller[], const dest[], string_array:params)
{
	if( array_size(params) >= 1 )
	{
		new bool:autosave_topic= false;
		new channel[CHANNAME_SIZE];
		new varname[VAR_SIZE];

		array_get(params, 0, channel);
		snprintf(varname, _, "autosave_%s", channel);

		if( array_size(params) >= 2 )
		{
			new num[5];

			array_get(params, 1, num);
			if( strmatch(num, "0") )
			{
				autosave_topic= false;
			}
			else
			{
				autosave_topic= true;
				func_savetopic(caller, dest, params);
			}

			saveIntEx(varname, autosave_topic);
		}
		else
		{
			loadIntEx(varname, autosave_topic);
		}

		irc_notice(caller, "autosave for \"%s\": %d\n", channel, autosave_topic);
	}
	else
	{
		return -1;
	}

	return 0;
}

public func_enforce(const caller[], const dest[], string_array:params)
{
	if( array_size(params) >= 1 )
	{
		new bool:enforce_topic= false;
		new channel[CHANNAME_SIZE];
		new varname[VAR_SIZE];

		array_get(params, 0, channel);
		snprintf(varname, _, "enforce_%s", channel);


		if( array_size(params) >= 2 )
		{
			new num[5];

			array_get(params, 1, num);

			if( strmatch(num, "0") )
			{
				enforce_topic= false;
			}
			else
			{
				enforce_topic= true;
				func_topic(caller, dest, params);
			}

			// save var state
			saveIntEx(varname, enforce_topic);
		}
		else
		{
			loadIntEx(varname, enforce_topic);
		}

		irc_notice(caller, "enforce for \"%s\": %d", channel, enforce_topic);
	}
	else
	{
		return -1;
	}

	return 0;
}

public func_savetopic(const caller[], const dest[], string_array:params)
{
	// do nothing if the command is sent as a pm to the bot
	if( irc_isChanName(dest) )
	{
		saveTopic(dest);
		irc_notice(caller, "Topic saved for %s", dest);
	}
}

public func_topic(const caller[], const dest[], string_array:params)
{
	new topic[TOPIC_SIZE];
	new varname[VAR_SIZE];

	// do nothing if the command is sent as a pm to the bot
	if( irc_isChanName(dest) )
	{
		snprintf(varname, _, "topic_%s", dest);
		loadStringEx(varname, topic);
		irc_setTopic(dest, topic);

		irc_notice(caller, "Topic restored for %s", dest);
	}
}

public event_onConnected()
{
	registerCommand(":autosave",	"func_autosave",	'n', ":autosave <chan> <0|1>");
	registerCommand(":save_topic", 	"func_savetopic", 	'o', ":save_topic");
	registerCommand(":topic", 		"func_topic", 		'n', ":topic");
	registerCommand(":enforce", 	"func_enforce", 	'n', ":enforce <chan> <0|1>");
}




/**/

