
require('mysql')
require('parsedate')
include(Embed)


$plugin_class= "WarBotScript"

class WarBotScript < RubyScript

	def event_LoadPlugin()
		@chan_pv= loadSimpleVar('chan_pv')
		@chan_pub= loadSimpleVar('chan_pub')
		@chan_pv_pass= loadSimpleVar('chan_pv_pass')
		@chan_pv_pass= "" if @chan_pv_pass == nil

		@db_name= 	loadSimpleVar('db_name')
		@db_login= 	loadSimpleVar('db_login')
		@db_pass= 	loadSimpleVar('db_pass')
		@db_host= 	loadSimpleVar('db_host')

		@auth_login= 	loadSimpleVar('auth_login')
		@auth_pass= 	loadSimpleVar('auth_pass')
		@auth_bot= 		loadSimpleVar('auth_bot')
		@use_topic= 	loadSimpleVar('use_topic')
		@site_url= 		loadSimpleVar('site_url')
		@max_player_count= loadSimpleVar('max_player_count')

		# no config file found
		if (@chan_pv == nil) or (@chan_pv == "#")
			default_values= {
				"auth_bot"			=> false,
				"use_topic"			=> false,
				"chan_pv" 			=> "#",
				"chan_pub" 			=> "#",
				"chan_pv_pass"		=> "",
				"db_name"			=> "",
				"db_login"			=> "",
				"db_pass"			=> "",
				"db_host"			=> "",
				"auth_login"		=> "",
				"auth_pass"			=> "",
				"site_url"			=> "",
				"max_player_count"	=> 6
							}

			default_values.each{ |n,v| saveSimpleVar(n,v) }

			# we can't continue without required config
			puts "[Script disabled] Edit config file !"
			disableScript()
		else
			puts "Auth bot: #{@auth_bot}"
			puts "Use Topic: #{@use_topic}"

			begin
				@db= Mysql.real_connect(@db_host, @db_login, @db_pass, @db_name)
			rescue MysqlError
				puts @db_host, @db_login, @db_pass, @db_name
				puts "[Script disabled] Unable to connect to database: " + $!
				disableScript()
			end
		end
	end

	def event_UnloadPlugin()
		if @db != nil
			@db.close()
		end
	end
	
# utils
	def updateTopic()
		if @use_topic == "1"
			pv= getChanObj(@chan_pv)
			if pv != nil
				res=@db.query("SELECT * FROM matchs WHERE (Type='t' OR Type='m' ) AND Etat='c' AND Date - NOW() > 0 ORDER BY Date")
				if res.num_rows > 0
					m= res.fetch_hash()
					id=		m['id']
					adv=	m['Adversaires']
					date=	ParseDate.parsedate(m['Date'])
					date= Time.local( *date[0..5] ).strftime("%d/%m %H:%M")
	
					res2= @db.query("SELECT users.* FROM users,link WHERE users.id=link.user AND link.match='#{id}' ORDER BY link.id")
					players= []
					if res2.num_rows() > 0
						res2.each_hash{|l| players.push(l['account'])}
						players= players.join(", ")
					end
					res2.free()
					pv.topic= "[(#{id}) Match vs #{adv} #{date} : #{players} ]"
				else
					pv.topic= "no match."
				end
				res.free()
			end
		end
	end

	def showMatch(sender, id)
		res= @db.query("SELECT * FROM matchs WHERE (Type='t' OR Type='m' ) AND Etat='c' AND Date - NOW() > 0 AND id='#{id}' ORDER BY Date")
		showMatchFromQuery(res, sender)
		res.free()
	end

	def showMatchFromQuery(res, sender)
		fulldisp= true
		c= res.num_rows()

		if c == 0
			sender.notice("no match")
		else
			fulldisp= false if c > 1
			res.each_hash{ |q|
				id= 		q['id'].to_i()
				adv= 		q['Adversaires']
				type= 		q['Type_Comment']
				maps= 		q['Maps']
				d1= 		ParseDate.parsedate(q['Date'])
				score= 		q['Score']
				state= 		q['Etat']

				if fulldisp
					comments= 	q['Comments']
					server= 	q['Server']
					password=	q['Password']
					password= "?" if password.length() == 0
				end

				d1= Time.local( *d1[0..5] )
				date_disp= d1.strftime("%A %d %B %H:%M")

				pcount= 0
				players= []
				res2= @db.query("SELECT users.* FROM users,link WHERE users.id=link.user AND link.match='#{id}' ORDER BY link.id")
				if res2.num_rows() > 0
					res2.each_hash{ |r| players.push(r['account']) }
					pcount= players.size()
					players= players.join(", ")
				end
				res2.free()

				sender.notice("(\00309#{id}\003) [\00307#{date_disp}\003] match vs #{adv} (#{type}) (#{maps}) (#{pcount} registered players)")
				if fulldisp
					sender.notice("Lineup: #{players}") if pcount > 0
					sender.notice("Comments: #{comments}") if comments.length() > 0
					sender.notice("Server: #{server} (pass: #{password})") if (server.length() > 0) and (state == 'c')
				end

				sender.notice("Score: #{score}") if (score != nil) and (score.length() > 0) and (state == 't')
			}
		end
	end

# Events
	def event_onBotJoined(chan, userlist)
		updateTopic() if chan.name == @chan_pv
	end
	
	def event_onBotKicked(who, chan, msg)
		chan.rejoin()
		who.notice("be carreful my friend.")
	end

	def event_onBotOpped(chan, who)
		updateTopic() if chan.name == @chan_pv
	end

	def event_onRegistered()
		if @auth_bot
			join(@chan_pv, @chan_pv_pass)
			join(@chan_pub)
		end
	end

	def event_onJoin(who, chan)
		if who.hasAccount()
			acct= who.account_name
			puts "Account of #{who.nick}: #{acct}"

			res= @db.query("SELECT * FROM users WHERE account='#{acct}'")
			if res.num_rows() > 0
				if chan.name == @chan_pv
					res2= @db.query("SELECT * FROM matchs WHERE (Type='t' OR Type='m' ) AND Etat='c' AND Date - NOW() > 0 ORDER BY Date LIMIT 1")
					showMatchFromQuery(res2, who)
					res2.free()
				end
			else
				who.notice("No database account for #{acct}, tell bot owner")
			end

			res.free()
		end
	end

	def event_onAction(who, action, chan)
		tmp=  Regexp.escape(bot_nick)
		
		if action =~ %r{\Adonne une? ([a-zA-Z ]+) à #{tmp}\Z}
			if $1 == "cookie"
				chan.action("mange le cookie")
			else
				chan.action("regarde #{who.nick} bizarrement")
			end
		end
	end
	
	def event_onPrivMsg(sender, dest, args)
		if dest.class == Channel
			tmp=  Regexp.escape(bot_nick)
			
			msg= args[0..args.size()].join(" ")
			if msg =~ %r{\A#{tmp} \?\Z}
				dest.privmsg("#{sender.nick} ?")
			end
		end
	end

# commands
	def func_listmatchs(sender, dest, args)
		res= @db.query("SELECT * FROM matchs WHERE (Type='t' OR Type='m' ) AND Etat='c' AND Date - NOW() > 0 ORDER BY Date")
		showMatchFromQuery(res, sender) if res.num_rows() > 0
		res.free()
	end

	def func_userlist(sender, dest, args)
		res= @db.query("SELECT * FROM users")
		if res.num_rows() > 0
			res.each_hash{|u| sender.notice("(#{u['id']}) [\00309#{u['account']}\003]") }
		else
			sender.notice("no account to list.")
		end

		res.free()
	end

	# display current match (the match actually played)
	def func_match(sender, dest, args)
		res= @db.query("SELECT * FROM `matchs` WHERE Etat='c' AND Date - NOW() < 0 ORDER BY Date DESC LIMIT 1")
		showMatchFromQuery(res, sender)
		res.free()
	end

	# set score for last played match
	def func_score(sender, dest, args)
		if args.size() >= 1
			score= args[0].to_s()

			if( args.size() == 2 )
				res= @db.query("SELECT * FROM `matchs` WHERE Etat='c' AND Date - NOW() < 0 AND id='#{args[1]}'")
			else
				res= @db.query("SELECT * FROM `matchs` WHERE Etat='c' AND Date - NOW() < 0 ORDER BY Date DESC LIMIT 1")
			end
			
			if res.num_rows() > 0
				m= res.fetch_hash()
				res2= @db.query("SELECT * FROM link WHERE `match`='#{m['id']}'")
				players= res2.num_rows()
				res2.free()

				if players <= @max_player_count
					@db.query("UPDATE `matchs` SET Score='#{score}', Etat='t' WHERE id='#{m['id']}'")
					sender.notice("score for match #{m['id']} vs #{m['Adversaires']}: #{score}")
					updateTopic()
				else
					sender.notice("too many players registered on this match (remove some)")
				end

				res2.free()
			else
				sender.notice("no match.")
			end
			res.free()
		else
			return -1
		end
	end

	# display informations for the next match
	def func_nextmatch(sender, dest, args)
		res= @db.query("SELECT * FROM matchs WHERE (Type='t' OR Type='m' ) AND Etat='c' AND Date - NOW() > 0 ORDER BY Date LIMIT 1")
		showMatchFromQuery(res, sender)
		res.free()
	end

	# display informations on a given match
	def func_info(sender, dest, args)
		if args.size() == 1
			id= args[0].to_i()
			res= @db.query("SELECT * FROM matchs WHERE id='#{id}'")
			showMatchFromQuery(res, sender)
			res.free()
		else
			return -1
		end
	end

	# display information for last played match (last match with score set)
	def func_lastmatch(sender, dest, args)
		count= 1
		count= args[0].to_i() if args.size() >= 1

		res= @db.query("SELECT * FROM matchs WHERE (Type='t' OR Type='m' ) AND Etat='t' ORDER BY Date DESC LIMIT #{count}")
		showMatchFromQuery(res, sender)
		res.free()
	end

	# just the website url
	def func_url(sender, dest, args)
		if @site_url != nil
			sender.notice(@site_url)
		else
			sender.notice("no site defined.")
		end
	end

	# remove a user from a match playerlist
	def func_remove(sender, dest, args)
		if args.size() >= 1
			user= sender.nick
			matchid= args[0].to_i()
			res= @db.query("SELECT * FROM `users` WHERE account='#{sender.account_name}'")
			if res.num_rows() > 0
				u= res.fetch_hash()
				userid= u['id'].to_i()
				err= false

				# user don' want to remove himself
				# !remove <match_id> <user>
				if (u['level'].to_i() <= 1) and (args.size() == 2)
					targ_account= args[1].to_s()

					res2= @db.query("SELECT * FROM `users` WHERE account=\"#{targ_account}\"")
					if res2.num_rows() > 0
						t= res2.fetch_hash()
						userid= t['id'].to_i()
						user= targ_account
					# user not found, maybe his current nick was given instead of his account
					else
						targ= getUserObj(targ_account)
						if targ != nil
							res3= @db.query("SELECT * FROM `users` WHERE account=\"#{targ.account_name}\"")
							if res3.num_rows() > 0
								t= res3.fetch_hash()
								userid= t['id'].to_i()
								user= targ.nick
							else
								err= true
							end
							res3.free()
						end
					end
					res2.free()
					sender.notice("unknow user: #{targ_account}") if err
				end

				if !err
					res2= @db.query("SELECT * FROM matchs WHERE id=\"#{matchid}\" AND Etat='c'")
					if res2.num_rows() > 0
						m= res2.fetch_hash()
						@db.query("DELETE FROM link WHERE user='#{userid}' AND `match`='#{matchid}'")
						sender.notice("'#{user}' removed from match #{matchid} vs #{m['Adversaires']}")
						updateTopic()
					else
						sender.notice("unknow match id: #{matchid}")
					end
					res2.free()
				end

			else
				sender.notice("no account in db, tell bot owner")
			end
			res.free()
		else

		end
	end

	# register a player for a match
	def func_add(sender, dest, args)
		if args.size() >= 1
			user= sender.nick
			matchid= args[0].to_i()
			res= @db.query("SELECT * FROM `users` WHERE account='#{sender.account_name}'")
			if res.num_rows() > 0
				u= res.fetch_hash()
				userid= u['id'].to_i()
				err= false

				# user don' want to add himself
				# !add <match_id> <user>
				if (u['level'].to_i() <= 1) and (args.size() == 2)
					targ_account= args[1].to_s()

					res2= @db.query("SELECT * FROM `users` WHERE account=\"#{targ_account}\"")
					if res2.num_rows() > 0
						t= res2.fetch_hash()
						userid= t['id'].to_i()
						user= targ_account
					# user not found, maybe his current nick was given instead of his account
					else
						targ= getUserObj(targ_account)
						if targ != nil
							res3= @db.query("SELECT * FROM `users` WHERE account=\"#{targ.account_name}\"")
							if res3.num_rows() > 0
								t= res3.fetch_hash()
								userid= t['id'].to_i()
								user= targ.nick
							else
								err= true
							end
							res3.free()
						end
					end
					res2.free()
					sender.notice("unknow user: #{targ_account}") if err
				end

				if !err
					# check if match exists
					if matchid == 0
						res2= @db.query("SELECT * FROM `matchs` WHERE Date LIKE CONCAT(CURDATE(),'%%')");
	
						# if no match found, create one
						if res2.num_rows() == 0
							@db.query("INSERT INTO matchs(Adversaires,Type,Type_Comment,Date,Etat,Maps,Server,Comments) VALUES('?','m','pcw',CONCAT(CURDATE(), ' 21:00:00'),'c','?/?','','')");
							res2= @db.query("SELECT MAX(id) FROM matchs");
							# mysql_get_field(res, 0, "MAX(id)", num);
							m= res2.fetch_hash()
							matchid= m['MAX(id)']
						end
					end
					
					
					res2= @db.query("SELECT * FROM matchs WHERE id=\"#{matchid}\" AND Etat='c'")
					if res2.num_rows() > 0
						m= res2.fetch_hash()
						# check if user is not already registered
						res3= @db.query("SELECT * FROM link WHERE link.user='#{userid}' AND link.match='#{matchid}'")
						if res3.num_rows() > 0
							sender.notice("#{user} already registered in match #{matchid} vs #{m['Adversaires']}")
						else
							@db.query("INSERT INTO link(link.user,link.match) VALUES('#{userid}','#{matchid}')")
							sender.notice("'#{user}' added to match #{matchid} vs #{m['Adversaires']}")
							updateTopic()
						end
					else
						sender.notice("unknow match id: #{matchid}")
					end
					res2.free()
				end

			else
				sender.notice("no account in db, tell bot owner")
			end
			res.free()
		else

		end
	end

	# remove a match
	def func_delmatch(sender, dest, args)
		if args.size() == 1
			id= args[0].to_i()
			@db.query("DELETE FROM link WHERE `match`='#{id}'")
			@db.query("DELETE FROM `matchs` WHERE id='#{id}'")
			sender.notice("match #{id} removed.")
			updateTopic()
		else
			return -1
		end
	end

	# add a new match
	# !addmatch <adv> <date> <time> [<maps>]
	def func_addmatch(sender, dest, args)
		if args.size() >= 3
			arg_error= false

			adv= args[0]
			date= args[1]
			time= args[2]
			maps= "?/?"
			maps= args[3] if args.size() == 4

			# check date format (dd/mm/yyyy)
			if date =~ %r{([0-9]{2})/([0-9]{2})(/([0-9]{4}))?}
				if $4 != nil
					date= [$4, $2, $1]
				else
					date= [Time.now.year, $2, $1 ]
				end
			else
				sender.notice("invalid date: #{date}")
				arg_error= true
			end

			# check time format (HH:MM)
			if time =~ %r{([0-9]{2}):([0-9]{2})}
				time= [$1, $2]
			else
				sender.notice("invalid time: #{time}")
				arg_error= true
			end

			# all format ok
			if !arg_error
				date_time_ok= true
				arr= [ *date ]
				arr.push( *time )
				begin
					date_time= Time.local( *arr )
				rescue ArgumentError
					sender.notice("invalid date or time.")
					date_time_ok= false
				end

				# last check passed
				if date_time_ok
					mysql_datetime= date_time.strftime("%Y-%m-%d %H:%M:%S")
					# check db if match not already registered
					res= @db.query("SELECT * FROM matchs WHERE Adversaires='#{adv}' AND Date='#{mysql_datetime}'")
					if res.num_rows() == 0
						@db.query("INSERT INTO matchs(Adversaires, Date, Maps, Type, Type_Comment) VALUES('#{adv}', '#{mysql_datetime}', '#{maps}', 'm', 'pcw')")
						sender.notice("match created.")
						updateTopic()
					else
						sender.notice("match already in database.")
					end
					res.free()
				end
			end
		else
			return -1
		end
	end

	# edit match properties
	# <match_id> <what> <new_value>
	# team, maps, server, pass
	def func_editmatch(sender, dest, args)
		if args.size() == 3
			matchid= args[0].to_i()
			what= args[1]
			new_value= args[2]

			res= @db.query("SELECT * FROM matchs WHERE id='#{matchid}'")
			if res.num_rows() > 0
				fields= {
							"team" 		=> "Adversaires",
							"maps" 		=> "Maps",
							"server" 	=> "Server",
							"pass" 		=> "Password"
						}

				if fields.fetch(what, nil) != nil
					what= fields.fetch(what, nil)
					@db.query("UPDATE matchs SET #{what}='#{new_value}' WHERE id='#{matchid}'")
					sender.notice("Match #{matchid} updated.")
					updateTopic()
				else
					sender.notice("unknown property #{what}, valid properties: " + fields.keys().join(","))
				end
			else
				sender.notice("no match with id #{matchid}.")
			end
			res.free()
		else
			return -1
		end
	end

	# list steamids
	def func_steamid(sender, dest, args)
		res= @db.query("SELECT * FROM users")
		if res.num_rows() > 0
			res.each_hash{|u|
				steamid= u['SteamID']
				nick= "%*s" % [ 15, u['account'] ]
				sender.notice("#{nick} => #{steamid}") if (steamid != nil) && (steamid.length() > 0)
			}
		else
			sender.notice("no user in database.")
		end
	end

	def func_phones(sender, dest, args)
		res= @db.query("SELECT * FROM users")
		if res.num_rows() > 0
			res.each_hash{|u|
				tel= u['telephone']
				nick= "%*s" % [ 10, u['account'] ]
				sender.notice("#{nick} => #{tel}") if (tel != nil) && (tel.length() > 0)
			}
		else
			sender.notice("no user in database.")
		end
	end

	# list futures lans
	def func_lans(sender, dest, args)
		res= @db.query("SELECT * FROM lans WHERE Date - NOW() > 0 ORDER BY id")
		if res.num_rows() > 0
			# for each lan
			res.each_hash{ |l|
				id= l['id']
				registered= []
				with_car= []
				unsure= []
				date= ParseDate.parsedate(l['Date'])
				date= Time.local( *date[0..5] ).strftime("%d/%m/%Y")
				sender.notice("(#{id}) [#{l['Nom']}] [#{date}] [Durée: #{l['Duree']} jour(s)] [ #{l['Site_Web']} ]")
				res2= @db.query("SELECT users.*, link_lans.type FROM users,link_lans WHERE users.id=link_lans.user AND link_lans.lan='#{id}' ORDER BY link_lans.id")
				res2.each_hash{ |ll|
					type= ll['type'].to_i()
					account= ll['account']

					registered.push(account) 	if type === (1..2)
					with_car.push(account)		if type == 1
					unsure.push(account)		if type == 0
				}

				registered= registered.join(", ")
				with_car= with_car.join(", ")
				unsure= unsure.join(", ")

				sender.notice("Peuvent venir: #{registered}")
				sender.notice("Voiture: #{with_car}")
				sender.notice("Incertains: #{unsure}")
			}
			res.free()
		else
			sender.notice("no lans planned.")
		end
	end
	
	def func_tape(sender, dest, args)
		if (dest.class == Channel) and (args.size() == 1)
			dest.action("tape #{args[0]}")
		end		
	end
	
	def func_salut(sender, dest, args)
		if dest.class == Channel
			dest.privmsg("salut #{sender.nick}")
		end
	end
	
	def func_auth(sender, dest, args)
		if @auth_bot
			privmsg("Q@CServe.quakenet.org", "AUTH #{@auth_login} #{@auth_pass}")
			userMode("+x")
		end		
	end

# main
	def event_onConnected()
		func_auth(nil, nil, nil)
		userMode("+i")

		# admin
		registerCommand("!delmatch", 	"func_delmatch", 	'm', "!delmatch <id>")
		registerCommand("!addmatch", 	"func_addmatch", 	'm', "!addmatch <adv> <date> <time> [<maps>]")

		# matchs
		registerCommand("!score",		"func_score",		'o', "!score <us>-<them> [<match_id>]")
		registerCommand("!match", 		"func_match", 		'o', "!match")
		registerCommand("!edit", 		"func_editmatch", 	'o', "!edit <matchid> <what> <value> (what= maps or team)")

		registerCommand("!nextmatch", 	"func_nextmatch", 	'v', "!nextmatch")
		registerCommand("!lastmatch", 	"func_lastmatch", 	'v', "!lastmatch")
		registerCommand("!listmatchs", 	"func_listmatchs", 	'v', "!listmatchs")
		registerCommand("!info", 		"func_info", 		'v', "!info <match_id>")
		registerCommand("!add", 		"func_add", 		'v', "!add <match_id> [user] (0= match du jour, en crée un si aucun)")
		registerCommand("!remove", 		"func_remove", 		'v', "!remove <match_id>")


		# lans
		registerCommand("!lans",		"func_lans",		'o', "!lans")

		# other
		registerCommand("!steamid",		"func_steamid",		'o', "!steamid")
		registerCommand("!tels",		"func_phones",		'o', "!tels")
		registerCommand("!dbuserlist", 	"func_userlist", 	'o', "!dbuserlist")
		registerCommand("!auth",		"func_auth",		'o')
		registerCommand("!url", 		"func_url", 		'v', "!url")

		# stupid things
		registerCommand("!tape", 		"func_tape", 		'm')
		registerCommand("salut", 		"func_salut", 		'*')

		if not @auth_bot
			join(@chan_pv, @chan_pv_pass)
			join(@chan_pub)
		end
	end

end

