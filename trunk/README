
Dryon IRC Bot wrriten by Julien Ammous ( julien.a@laposte.net )
http://dryon.berlios.de for more infos



What is Dryon ?
-------------
Dryon is an irc bot previously known as amxbot, it was born since i wasn't satisfied with the existing 
irc bots. I use it everyday to manage a clan channel so I think i can say everything possible was done 
to it and it should now works without problems, my test bot runs under FreeBSD.
It is a scriptable bot, scripts can use Small from Compuphase (faster execution than ruby 
but scripts are bigger and harder to write, not hard but harder compared to ruby :) ) or Ruby (object
oriented and really powerfull scripting language)




Current supported OS:
--------------------
- FreeBSD (4.xx)
- Linux
- Windows XP (need more testing)


To compile:
----------
For freebsd and linux:
	make
	
Under windows I provide the project for Misual Studio 6 but I no longer use it
so some changes may need to be done to it, and the Visual Studio .net project
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
bin/scripts/debug.rb: The only goal for this script is simply to use all the events available to scripts and write
								a message on console.
bin/scripts/main.rb: (should be used except if you known what you are doing) It defines some core commands,
							I moved most of the core commands to it so users can really change the way the bot works as they wish.
bin/scripts/topic_save.rb: small script I wrote to save/restore topic on a given channel, it saves the topic on disk and restore it
									if needed.