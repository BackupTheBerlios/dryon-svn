=begin
Action: load/save topics and can enforce them
Coder: Anthalir
Comments: it can handle any number of chans

The script can be used to save and force a given topic:
	":force #chan 1"
Or to auto save the topic and set it if it is removed after a netsplit or i don't know what :
	":autosave #chan 1"

if both are set autosave should have total priority on forced topic.
=end

include(Embed)

class SaveTopicScript < RubyScript
# commands
	def func_autosave(sender, dest, args)
		if args.size() >= 1
			chan_name= args[0]

			if args.size() >= 2
				if (args[1] == "0") || (args[1]=="false")
					autosave= false
				else
					autosave= true
					func_savetopic(sender, dest, args)
				end

				saveSimpleVar("autosave_#{chan_name}", autosave)
			else
				autosave= loadSimpleVar("autosave_#{chan_name}")
			end

			autosave= false if autosave == nil
			sender.notice("autosave for \"#{chan_name}\": #{autosave}")
		else
			return -1
		end
	end

	def func_enforce(sender, dest, args)
		if args.size() >= 1
			chan_name= args[0]

			if args.size() >= 2
				if (args[1] == "0") || (args[1]=="false")
					enforce= false
				else
					enforce= true
					func_topic(sender, dest, args)
				end

				saveSimpleVar("enforce_#{chan_name}", enforce)
			else
				enforce= loadSimpleVar("enforce_#{chan_name}")
			end

			enforce= false if enforce == nil
			sender.notice("enforce for \"#{chan_name}\": #{enforce}")
		else
			return -1
		end
	end

	def func_savetopic(sender, dest, args)
		if dest.class == Channel
			saveTopic(dest)
			sender.notice("Topic saved for #{dest.name}")
		end
	end

	def func_topic(sender, dest, args)
		if dest.class == Channel
			saved_topic= loadSimpleVar("topic_#{dest.name}").to_s()
			if saved_topic != nil
				dest.topic= saved_topic
				sender.notice("Topic restored for #{dest.name}")
			end
		end
	end

# helper func
	def saveTopic(chan)
		topic= saveSimpleVar("topic_#{chan.name}", chan.topic)
	end


# events
	def event_onBotOpped(chan, who)
		chan.chanMode("+t")
	end

	def event_onTopicChange(sender, chan, old_topic, new_topic)
		# do not intercept topic from me
		# or we have a nice endless loop :)
		if !isMe(sender)
			enforce= loadSimpleVar("enforce_#{chan.name}")
			autosave= loadSimpleVar("autosave_#{chan.name}")

			if autosave
				saveTopic(chan)
			elsif enforce
				saved_topic= loadSimpleVar("topic_#{chan.name}").to_s()
				chan.topic= saved_topic if saved_topic != nil
			end
		end
	end

	def event_onBotJoined(chan, userlist)
		enforce= loadSimpleVar("enforce_#{chan.name}")
		autosave= loadSimpleVar("autosave_#{chan.name}")

		if( (autosave && (chan.topic.length() == 0)) || enforce )
			saved_topic= loadSimpleVar("topic_#{chan.name}").to_s()
			chan.topic= saved_topic if saved_topic != nil
		end
	end

# main
	def event_onConnected()
		registerCommand(":autosave",	"func_autosave",	'n', ":autosave <chan> <0|1>")
		registerCommand(":savetopic", 	"func_savetopic", 	'o', ":savetopic")
		registerCommand(":topic", 		"func_topic", 		'n', ":topic")
		registerCommand(":enforce", 	"func_enforce", 	'n', ":enforce <chan> <0|1>")
	end
end

$plugin_class= "SaveTopicScript"
