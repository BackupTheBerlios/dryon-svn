CC=gcc
CXX=g++

MYFILES= natives_mysql.cpp
PLUGIN=amxMySQL.so
PLUGIN_DEST=../../../../amxbot/libs/$(PLUGIN)


FILES= ../../natives_base.cpp ../../../tokens.cpp ../../amx/amx/amx.c
INCS= -g -I/usr/local/include -I../.. -I../../amx/amx -I../../..
LIBS= `mysql_config --libs`

TMP= $(FILES:.cpp=.o) $(MYFILES:.cpp=.o)
OBJS= $(TMP:.c=.o)

all : $(PLUGIN)
src-size :
	du -hck $(FILES) $(MYFILES)

size :
	du -hck $(OBJS)

clean :
	rm -rf $(OBJS)

.cpp.o :
	$(CXX) -fPIC $(INCS) -c $< -o $@

.c.o :
	$(CC)  -fPIC $(INCS) -c $< -o $@


$(PLUGIN) : $(OBJS)
	$(CXX)  -shared -W1,-soname,$(PLUGIN) $(LIBS) -o $(PLUGIN) $(OBJS)
	cp $(PLUGIN) $(PLUGIN_DEST)
