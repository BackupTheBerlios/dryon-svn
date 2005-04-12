
class Item
	attr_reader :parent, :name, :anchor, :data
	attr_writer :parent, :name, :anchor, :data

	def initialize()
		@parent = nil
		@name   = ""
		@anchor = ""
		@data   = {}
  	end

 	def <=>(item2)
		return @name<=>item2.name
	end

	def to_s
		ret= "Item: p(#{@parent.name}) n(#{@name}) a(#{@anchor}) data(#{@data.size})\n"

		if not @data.empty?()
			@data.each{ |key, value| ret+= "<#{key}>\n" }
			ret+= "\n"
		end
	end
end

class Section < Item
	attr_reader :subSections, :items, :macros, :variables
	attr_writer :subSections, :items, :macros, :variables

	def initialize()
		super()
		@subSections = []
		@items       = []
		@macros      = []
		@variables	 = []
	end

 	def to_s
 		parent= "none"
 		parent= @parent.name if @parent != nil
		ret= "Section: p(#{parent}) n(#{@name}) a(#{@anchor}) data(#{@data.size}) subs(#{@subSections.size})\n"

		if not @data.empty?()
			@data.each{ |key, value| ret+= "<#{key}>\n" }
			ret+= "\n"
		end

		if not @macros.empty?()
			@macros.each{ |key, value| ret+= "<#{key}> => #{value}\n" }
			ret+= "\n"
		end

		return ret
	end

end

# #####################################

# global root

$file_list= %w(	Small/natives.cpp
				Small/natives.h
				Small/natives_strings.cpp
				Small/natives_base.cpp
				Small/Libraries/MySQL/natives_mysql.cpp
				Small/Libraries/Timers/timer.cpp
				Small/Libraries/GameServers/hl.cpp

				Ruby/ruby_script.cpp
)

$section_root= Section.new()
$section_root.name= "Main"
$item_list= {}

$inc_dir= "../bin/include/"
$doc_dir= "../bin/docs/"

def findSection(name, root)
	result= nil

	if (root.name != "") and (root.name === name)
		result= root
	else
		root.subSections.each{ |section|
			result= findSection(name, section)
			break if result != nil
		}
	end

	return result
end

def findRoot(itm)
	ret= itm
	ret= ret.parent while (ret.parent != nil) and (ret.parent != $section_root)
	return ret
end

# just for debug purposes
def dispTree(root)
	puts root, "\n"
	root.subSections.each{ 	|section| dispTree(section) }
	root.items.each{ 		|itm| puts itm }
end

def sort_array(root)
	root.items.sort!()
	root.macros.sort!()

	root.subSections.each{ |section| sort_array(section) }
end

# #################
# HTML generation

def addHTMLLinks(data)
	tmp= data.dup()

	while tmp =~ %r{%([a-zA-Z_]+)%}
#		puts "found link: #{$1}"
		itm= $item_list[$1]
		if itm != nil
			root= findRoot(itm)
			data["%#{$1}%"]= "<A href='#{root.name}.html##{itm.anchor}'>#{$1}</A>"
		end

		tmp= $'
	end

	return data
end

def buildHTMLFuncList(root, of, first)
	of.write("<A name='#{root.anchor}'></A>")
	of.write("<DIV class='Section'>\n")
	of.write("<H1>#{root.name}</H1>")

	# remove INCLUDE CONTENT if exists
	root.data.delete('INCLUDE CONTENT')

	descr= root.data['MODULE DESCRIPTION']
	if descr != nil
		of.write("<DIV class='descr'>\n")
		of.write("<PRE>\n#{descr}</PRE>\n")
		of.write("</DIV>\n")
	end

# macro list
	root.macros.each{ |m|
		of.write("<A name='#{m.anchor}'></A>")
		of.write("<DIV class='Macro'>\n")

		ic= m.data.delete('INCLUDE CONTENT')
		puts "  !! 'INCLUDE CONTENT' found for macro #{m.name} (invalid here)" if ic != nil

		since= m.data.delete('SINCE')
		if since != nil
			since.strip!()
			of.write("<H1>MACRO: #{m.name} ( since #{since})</H1>\n")
		else
			of.write("<H1>MACRO: #{m.name}</H1>")
		end

		m.data.each{ |k,v|
			of.write("<H2>#{k}:</H2>\n")
			of.write("<PRE>" + addHTMLLinks(v) + "</PRE>\n")
		}

		of.write("</DIV>\n")
	}

# variables list
	root.variables.each{ |v|
		of.write("<A name='#{v.anchor}'></A>")
		of.write("<DIV class='Func'>\n")

		since= v.data.delete('SINCE')
		if since != nil
			since.strip!()
			of.write("<H1>VARIABLE: #{v.name} ( since #{since})</H1>\n")
		else
			of.write("<H1>VARIABLE: #{v.name}</H1>")
		end

		if v.data["TYPE"] == nil
			puts "!! no 'TYPE' field for variable #{v.name}"
		end

		v.data.each{ |k,v|
			of.write("<H2>#{k}:</H2>\n")
			of.write("<PRE>" + addHTMLLinks(v) + "</PRE>\n")
		}

		of.write("</DIV>\n")
	}

# functions list
	root.items.each{ |itm|
		of.write("<A name='#{itm.anchor}'></A>\n")
		of.write("<DIV class='Func'>\n")

		ic= itm.data.delete('INCLUDE CONTENT')
		puts "  !! 'INCLUDE CONTENT' found for function #{itm.name} (invalid here)" if ic != nil

		since= itm.data.delete('SINCE')
		if since !=nil
			since.strip!()
			of.write("<H1>#{itm.name} ( since #{since} )</H1>\n")
		else
			of.write("<H1>#{itm.name}</H1>\n")
		end

		usage= itm.data.delete('USAGE')
		if usage != nil
			of.write("<H2>USAGE:</H2>\n")
			of.write("<PRE>" + addHTMLLinks(usage) + "</PRE>\n")
		end

		itm.data.each{ |k,v|
			of.write("<H2>#{k}:</H2>\n")
			of.write("<PRE>" + addHTMLLinks(v) + "</PRE>\n")
		}

		of.write("</DIV>\n")
	}

	of.write("</DIV>\n")

	root.subSections.each{ |section| buildHTMLFuncList(section, of, false) }
end

def buildHTMLTOC(root, of, first)
	of.write("<DIV class='TOCSection'>\n") if !first
	of.write("<H1><A href='##{root.anchor}'>#{root.name}</A></H1>\n")

	root.macros.each{ |macro|
		of.write("<DIV class='el'><A href='##{macro.anchor}'>[M] #{macro.name}</A></DIV>\n")
	}

	root.variables.each{ |v|
		of.write("<DIV class='el'><A href='##{v.anchor}'>#{v.name}</A></DIV>\n")
	}

	root.items.each{ |itm|
		of.write("<DIV class='el'><A href='##{itm.anchor}'>#{itm.name}()</A></DIV>\n")
	}

	of.write("<DIV class='br'></DIV>\n")

	root.subSections.each{ |section| buildHTMLTOC(section, of, false) }
	of.write("</DIV>") if !first
end

def makeHTML()
	puts "* Generating HTML Documentation..."
	
	# put a file with date of latest doc generation
	tmp_file= File.open($doc_dir + "README", "w")
	current_time= Time.now()
	tmp_file.write("Documentation generated on: \n" + current_time.strftime("%H:%M %d/%m/%Y"))

	index_file= File.open($doc_dir + "index.html", "w")
	index_file.write("<HTML><HEAD><TITLE>Dryon Documentation</TITLE></HEAD><BODY>\n")
	index_file.write("<TABLE width='100%' height='95%'>\n")
	index_file.write("<TR><TD align='center'>\n")

	$section_root.subSections.each{ |s|
		current_filename= $doc_dir + s.name + '.html'
		index_file.write("<A href='#{s.name}.html'>#{s.name}</A><BR>\n")
		of= File.new(current_filename, "w")
		if of != nil
			puts "  - Generating #{current_filename}..."
			of.write(
				"<HTML>\n" +
				"<HEAD>\n" +
				"<TITLE>Documentation for " + s.name + "</TITLE></HEAD>\n" +
				"<link rel='stylesheet' href='style.css' type='text/css'>" +
				"</HEAD>\n" +
				"<BODY>\n"
			)

		buildHTMLTOC(s, of, true)

		of.write("<DIV class='br'></DIV>\n")
		of.write("<BR><BR><BR><BR><BR><BR><BR>\n")

		buildHTMLFuncList(s, of, true)

		of.write(
				"</BODY>\n" +
				"</HTML>\n"
		)

		end
	}

	index_file.write("</TD></TR>\n")
	index_file.write("</TABLE>\n")
	index_file.write("</BODY></HTML>\n")
end


# #######################
# include file generation

def buildIncList(root, of)

	ic= root.data.delete('INCLUDE CONTENT')
	if ic != nil
		ic.each{ |line|
			of.write(line.strip() + "\n")
		}
		of.write("\n")
	end


	root.macros.each{ |macro|
		macro_def= macro.data['MACRO']
		of.write(macro_def.strip() + "\n") if macro_def != nil
	}

	root.items.each{ |itm|
		usage= itm.data['USAGE']
		of.write("native " + usage.strip() + ";\n") if usage != nil
	}

	root.subSections.each{ |section| buildIncList(section, of) }
end

def buildIncludes()
	puts "* Generating Small(AMX) include files..."
	root= findSection("Small", $section_root)
	root.subSections.each{ |s|
		include_file= s.data['INCLUDE FILE']
		library_file= s.data['LIBRARY']

		if include_file != nil
			include_file.strip!()
			of= File.new($inc_dir + include_file, 'w')
			if of != nil
				puts "  - Generating #{include_file}..."
				if library_file != nil
					library_file.strip!()
					of.write("#pragma library #{library_file}\n\n")
				end

				buildIncList(s, of)
			end
		else
			puts "  !! Section with no include file: #{s.name}"
		end
	}
end

# #######################""
# main parsing

def parseAllFiles()
	id= 0

	puts "* Parsing all files..."

	$file_list.each{ |fname|
		current_item= nil
		current_data= ""
		content= ""
		data= {}

		f= File.new(fname) if File.exist?(fname)
		if f != nil
			printf("   - Processing #{fname}...\n");
			f.each{ |line|
				line.chomp!()

				# ***/
				if line == '***/'
					current_item.data[current_data]= content if current_item != nil
					current_item= nil
					current_data= ""
					content= ""

				# * USAGE
				elsif line =~ %r{^\*\s+([A-Z ]+)$}
					if (current_item != nil) and (not current_data.empty?())
#						puts "Adding data <#{current_data}> in <#{current_item.name}>: #{content}"
						current_item.data[current_data]= content
					end

#					puts("New section #{$1}")
					current_data= $1
					content= ""

				# /****f* SaveLoad/saveIntEx
				elsif line =~ %r{^/\*{4}([a-zA-Z])\*\s+([A-Za-z_]+)/([A-Za-z_\$]+)$}
					root= nil

					case $1

						# Headers
						when 'h', 'c'
							# Main is the root of all the documentation
							if $2 == "Main"
#								puts "Main Header: #{$3}"
								root= $section_root
							else
#								puts "Header: #{$3}"
								root= findSection($2, $section_root)
							end

						if root != nil
							s= Section.new()
							s.name= $3
							s.anchor= "ln" + id.to_s()
							s.parent= root
							id+= 1
							root.subSections.push(s)
							current_item= s
							$item_list[$3]= s
						else
							puts("ERROR BLOCK: %#{line}");
						end

						# Ruby class
						when 'o'
							root= findSection($2, $section_root)
							if root != nil
								s= Section.new()
								s.name= $3
								s.anchor= "ln" + id.to_s()
								s.parent= root
								id+= 1
								root.subSections.push(s)
								current_item= s
								$item_list[$3]= s
							else
								puts("Section not found: #{$2} for Class #{$3}")
							end

						# variables
						when 'v'
							root= findSection($2, $section_root)
							if root != nil
								itm= Item.new()
								itm.name= $3
								itm.anchor= "ln" + id.to_s()
								itm.parent= root
								id+= 1
								root.variables.push(itm)
								current_item= itm
								$item_list[$3]= itm
							else
								puts("Section not found: #{$2} for Variable #{$3}")
							end

						# Functions
						when 'f'
							root= findSection($2, $section_root)
							if root != nil
								itm= Item.new()
								itm.name= $3
								itm.anchor= "ln" + id.to_s()
								itm.parent= root
								id+= 1
								root.items.push(itm)
								current_item= itm
								$item_list[$3]= itm
							else
								puts("Section not found: #{$2} for Item #{$3}")
							end

						# Macros
						when 'm'
							root= findSection($2, $section_root)
							if root != nil
								macro= Item.new()
								macro.name= $3
								macro.anchor= "ln" + id.to_s()
								id+= 1
								macro.parent= root
								root.macros.push(macro)
								current_item= macro
								$item_list[$3]= macro
							end
					end

				elsif line.length() > 1
					content+= line[1...line.length] + "\n"
				end
			}
		else
			puts "  !! Unable to open #{fname}"
		end
	}

end


# Main
puts "Dryon include file & documentation generator\n"

parseAllFiles()
sort_array($section_root)
buildIncludes()
makeHTML()
puts "\nPush a key to end the program."
tmp= gets()

# dispTree($section_root)
