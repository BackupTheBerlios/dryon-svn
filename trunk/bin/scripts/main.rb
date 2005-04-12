
include(Embed)

class MAINScript < RubyScript
# Functions
# :nick
	def func_nick(sender, dest, args)
		return -1 if args.size() != 1

		changeNick(args[0])
		return 0
	end

# :join
	def func_join(sender, dest, args)
		if args.size() >= 1
			pass= ""
			pass= args[1] if args.size() == 2
			join(args[0], pass)
		else
			return -1
		end
	end

# :part
	def func_part(sender, dest, args)
		chan= dest.name
		chan= args[0] if args.size() >= 1

		part(chan)
	end


	
# :say <chan> <text>
	def func_say(sender, dest, args)
		return -1 if args.size() < 2
		
		chan= getChanObj( args[0].to_str() )
		if chan != nil
			chan.privmsg(args[1..args.size()].join(' '))
		end
	end	

# :adduser
	def func_adduser(sender, dest, args)
		return -1 if args.size() < 2

		auth= nil
		account= args[0]
		mask= args[1]
		flags= ""
		flags= args[2] if args.size() == 3

		if mask[0].chr == '#'
			auth= mask[1...mask.length()]
			mask= "a/#{auth}"
		end

		if addUserAccount(account, mask, flags) == -1
			if auth == nil
				sender.notice("mask already registered in userlist: #{mask}")
			else
				sender.notice("auth already registered in userlist: #{auth}")
			end
		else
			sender.notice("user added.")
		end
	end

# :deluser
	def func_deluser(sender, dest, args)
		return -1 if args.size() != 1

		delUserAccount(args[0])
		sender.notice("user <#{args[0]}> removed.")
	end

# :setflags
	def func_setflags(sender, dest, args)
		return -1 if args.size() != 3

		user= args[0]
		type= args[1]
		flags= args[2]

		if setAccessFlags(user, type, flags) == -1
			sender.notice("unknow account: #{user}")
		else
			sender.notice("modification done.")
		end
	end

# Events
	def event_onConnected()
		registerCommand(":nick", 	"func_nick", 		'n', ":nick <new_nick>")
		registerCommand(":join", 	"func_join", 		'n', ":join <chan> <pass>")
		registerCommand(":part", 	"func_part", 		'n', ":part [<chan>]")
		registerCommand(":action", 	"func_action", 		'n', ":action <chan> <action>")
		registerCommand(":say",		"func_say",			'n', ":say <chan> <text>")

		registerCommand(":adduser",	"func_adduser",		'm', ":adduser <account> <hostmask>/<#auth> [<global_flags>]")
		registerCommand(":deluser", "func_deluser",		'n', ":deluser <account>")
		registerCommand(":setflags", "func_setflags",	'm', ":setflags <account> <_global_>|<chan> <flags>" )
	end

	def event_UnloadPlugin()
		puts "unloaded"
	end

end



$plugin_class= "MAINScript"