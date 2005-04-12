
include(Embed)


class DebugScript < RubyScript
	def event_LoadPlugin()
		puts "loaded."
	end

	def event_UnloadPlugin()
		puts "unloaded."
	end

	def event_onTest()
		puts "test event."
	end


	def event_onConnected()
		puts "connected."
	end

	def event_onDisconnected()
		puts "disconnected."
	end

	def event_onRegistered()
		puts "registered."
	end

	def event_onPrivMsg(sender, dest, args)
		if dest.class == Channel
			puts "privmsg from #{sender.nick} to #{dest.name}: " + args.join(' ')
		else
			puts "privmsg from #{sender.nick} to me: " + args.join(' ')
		end
	end

	def event_onServerOp(chan, target)
		puts "#{target.nick} got op by server on #{chan.name}."
	end

	def event_onServerVoice(chan, target)
		puts "#{target.nick} got voice by server on #{chan.name}."
	end

	def event_onServerRemoveOp(chan, target)
		puts "#{target.nick} lost op by server on #{chan.name}."
	end

	def event_onServerRemoveVoice(chan, target)
		puts "#{target.nick} lost voice by server on #{chan.name}."
	end

	def event_onOp(chan, who, target)
		puts "#{target.nick} got op by #{who.nick} on #{chan.name}."
	end

	def event_onDeOp(chan, who, target)
		puts "#{target.nick} lost op by #{who.nick} on #{chan.name}."
	end

	def event_onVoice(chan, who, target)
		puts "#{target.nick} got voice by #{who.nick} on #{chan.name}."
	end

	def event_onDeVoice(chan, who, target)
		puts "#{target.nick} lost voice by #{who.nick} on #{chan.name}."
	end

	def event_onBan(chan, who, banmask)
		puts "#{banmask} banned from from #{chan.name} by #{who.nick}."
	end

	def event_onUnBan(chan, who, banmask)
		puts "#{banmask} unbanned from from #{chan.name} by #{who.nick}."
	end

	def event_onChanKeySet(chan, who, key)
		puts "#{who.nick} set key #{key} on #{chan.name}."
	end

	def event_onChanKeyRemoved(chan, who)
		puts "#{who.nick} removed key on #{chan.name}."
	end

	def event_onChanMode(who, chan, modestr)
		puts "#{who.nick} changed mode for #{chan.name}: #{modestr}."
	end

	def event_onAction(who, action, dest)
		puts "[ACTION from '#{who.nick}' on #{dest.name}] #{action}"
	end

	def event_onTopicChange(who, chan, old_topic, new_topic)
		puts "[TOPIC] #{chan.name}= \"#{new_topic}\" changed by #{who.nick}."
	end

	def event_onKick(who, chan, target, reason)
		puts "#{target.nick} kicked by #{who.nick} from #{chan.name} : #{reason}"
	end

	def event_onNickChange(old_nick, new_nick)
		puts "#{old_nick} now know as #{new_nick}."
	end

	def event_onJoin(who, chan)
		puts "#{who.nick} joined #{chan.name}."
	end

	def event_onPart(who, chan)
		puts "#{who.nick} part from #{chan.name}."
	end

	def event_onQuit(who)
		puts "#{who.nick} left IRC."
	end

	# bot ~~~~~~~~~~~~~~~~
	def event_onBotExit()
		puts "~~ bot exiting."
	end

	def event_onBotJoined(chan, users)
		puts "~~ bot joined #{chan.name}: " + users.join(' ')
	end

	def event_onBotPart(chan)
		puts "~~ bot part from #{chan.name}."
end

	def event_onBotOpped(chan, who)
		puts "~~ bot got op by #{who.nick} on #{chan.name}."
	end

	def event_onBotDeopped(chan, who)
		puts "~~ bot lost op by #{who.nick} on #{chan.name}."
	end

	def event_onBotVoiced(chan, who)
		puts "~~ bot got voice by #{who.nick} on #{chan.name}."
	end

	def event_onBotDevoiced(chan, who)
		puts "~~ bot lost voice by #{who.nick} on #{chan.name}."
	end

	def event_onBotKicked(who, chan, msg)
		puts "~~ bot kicked from #{chan.name}, by #{who.nick}: #{msg}."
	end

	def event_onBotBanned(chan, who, banmask)
		puts "~~ bot banned from #{chan.name} by #{who.nick} with \"#{banmask}\"."
	end

	def event_onBotUnbanned(chan, who, banmask)
		puts "~~ bot unbanned from #{chan.name} by #{who.nick} with #{banmask}"
	end

end

$plugin_class= "DebugScript"

