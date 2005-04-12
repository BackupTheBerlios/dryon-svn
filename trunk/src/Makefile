CXX=g++
CC=gcc
RANLIB = ranlib
AR = ar
OS=FreeBSD

# -pg for profiling
DEFS = -g3
INCPATH= -I/usr/local/include
LDFLAGS = -pthread -L/usr/local/lib/mysql -lmysqlclient -L/usr/local/lib -lcrypt
AMXINC= -ISmall/amx/amx -ISmall/amx/linux
RUBYINC= -IRuby/ruby

CFLAGS  = -pipe $(INCPATH) $(AMXINC) $(RUBYINC) $(DEFS) -DLINUX -DAMX_NODYNALOAD -DNOPROPLIST
CXXFLAGS= -pipe $(INCPATH) $(AMXINC) $(DEFS) -DAMX_NODYNALOAD -DLINUX -DDEBUG -Wall -funsigned-char -I. -IRuby/ruby

include Makefile.files

TMP= $(SRCS:.cpp=.o)
OBJS= $(TMP:.c=.o)

.c.o :
	@echo "[C]   Compiling $<..."
	@$(CC) $(CFLAGS) $< -c -o $@

.cpp.o:
	@echo "[CXX] Compiling $<..."
	@$(CXX) $(CXXFLAGS) $< -c -o $@

all: $(SRCS) libruby-bot libmysql-bot Ruby/ruby_embed.cpp dryon
src-size :
	du -hkc $(SRCS)

libmysql-bot: Ruby/mysql-ruby/mysql.c
	@cd Ruby/mysql-ruby/ && make

libruby-bot: Ruby/ruby/*.c
	@cd Ruby/ruby/ && make -f Makefile.lib

ruby-socket: Ruby/ruby/ext/socket/socket.c
	@cd Ruby/ruby/ext/socket && make static

sc_comp.o: Small/amx/compiler/sc?.c
	@echo "[CXX] Compiling $<..."
	@$(CXX) -c -pipe $(INCPATH) $(AMXINC) $(DEFS) -DLINUX -DDEBUG -DAMX_NODYNALOAD -DNOPROPLIST -I. -funsigned-char Small/sc_comp.cpp

dryon.o: basebot.o basebot.h
basebot.o: config.h

dryon: $(OBJS)
	@printf "\n"
	@echo "Linking..."
	@$(CXX) $(AMXINC) $(OBJS) $(LDFLAGS) -LRuby/ruby -lruby-bot -o dryon
	@cp dryon ../bin/

Ruby/ruby_embed.cpp: Ruby/embed.i Ruby/network.h
	@cd Ruby && ./wrap.sh

docs: build_doc.rb
	ruby build_doc.rb


test: ../bin/dryon
	@cd ../bin && dryon --test && cd ../src

debug-test: ../bin/dryon
	@cd ../bin && gdb dryon -q && cd ../src

clean:
	@echo "Cleaning Ruby..."
	@cd Ruby/ruby/ && make -sf Makefile.lib clean
	@cd Ruby/mysql-ruby/ && make clean
	@echo "Cleaning..."
	@rm -f *.core $(OBJS) dryon



