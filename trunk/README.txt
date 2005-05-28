
Dryon IRC Bot is wrriten by Julien Ammous ( julien.a@laposte.net )
go to http://dryon.berlios.de for more infos



What is Dryon ?
-------------
Dryon is an irc bot previously known as amxbot, it was born since i wasn't satisfied with the existing 
irc bots. I use it everyday to manage a clan channel so I think i can say everything possible was done 
to it and it should now works without problems, my test bot runs under FreeBSD.
It is a scriptable bot, scripts can be coded in Small from Compuphase (faster execution than ruby 
but scripts are bigger and harder to write, not hard but harder compared to ruby :) ) or in Ruby (object
oriented and really powerfull scripting language)




Current supported OS:
--------------------
- FreeBSD (4.xx)
- Linux
- Windows XP (need more testing)


To compile:
----------
For freebsd and linux: (binary file and libraries are built in the bin folder)
	cd src && make
	
Under windows I provide the project for Misual Studio 6 but I no longer use it
so some changes may be required, and the Visual Studio .NET project which
is the one I use to build releases so it should work for you.
It didn't manage to get it to compile with another compiler under windows, if
you manage to, please tell me.

To run Dryon:
-------------

dryon [<config_file>]

if config_file is not specified it simply uses dryon.cfg in the application folder,
if not found it will be created at first run.
The configuration file tells the bot which scripts to load, 
they can be located anywhere provided that the bot have access to the files.


Scripts you get with the bot:

scripts/warbot.rb: the actual script I use to manage my clan channel, rely on a MySQL database

scripts/warbot.sma: the first script I was using before Ruby was implemented, you can see difference 
	in size compared to the ruby one which do more things :)
	I keep it as a reference so you can see how each things are done in both languages and you can 
	use it to decide which language to use.
	
bin/scripts/debug.rb: The only goal for this script is simply to use all the events available to scripts and write
	a message on the console.
								
bin/scripts/main.rb: (should be used except if you known what you are doing) It defines some core commands,
	I moved most of the core commands to it so users can really change the way the bot works as they wish.
							
bin/scripts/topic_save.rb: small script I wrote to save/restore topic on a given channel, it saves the topic on disk and restore it
	if needed.
	
	
The first time you run the program two other files are also created:

servers.txt: It simply contains the list of servers to connect to, if there is an error with the first it try the next one, etc..
userfile.txt: It contains access rights for the bot users.

Both files are documented with comments, just look at them for informations.

Without having to edit those files the bot will run, it will connect to gamesurge irc network by default, it will be named "dryon" and
will consider anyone as an administrator (the default userfile.txt gives full access to anyone).
To see a list of available bot commands just type !help on a channel where the bot is or send it to it with a private message.

Help: (these files are auto generated after each compilation, so they are up to date)
----

Documentation of Ruby API:
	bin/docs/Ruby.html
	bin/docs/ScriptCallbacks.html
	
Documentation of Small API:
	every other files in the bin/docs folder

Basic bot commands:
-------------------

:join <channel>
:part <channel>
	ask the bot to join/part <channel>
	
:nick <new_nick>
	ask the bot to change its nick
	
:action <channel> <action_msg>
	ask the bot to perform an action on given channel (/me)
	
:say <channel> <text>
	ask the bot to say something on the channel
	

	------------------
	
:scripts
	gives a list of the actually loaded scripts
	
:userlist
	Display a list of the current access rights for each users
	same data as in userfile.txt
	
:reload
	unload all the currently loaded scripts, reload the config file
	and then reload all the scripts declared in it.
	
:adduser <account> <hostmask>/<#auth> [<global_flags>]
	account is the name which will be used to reference the user
	hostmask is used to recognize the user
	global_flags are the flags to apply to him (look in userfile.txt for details)
	
:deluser <account>
	remove a user access rigths

:setflags <account> _global_ <flags>
:setflags <account> <chan> <flags>
	set the user flags on al channels (global) or on a specific channel.