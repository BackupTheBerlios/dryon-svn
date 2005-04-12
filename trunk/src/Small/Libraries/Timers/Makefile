CC=gcc
CXX=g++

MYFILES= timer.cpp ../../../thread.cpp
PLUGIN=amxTimers.so
PLUGIN_DEST=../../../../amxbot/libs/$(PLUGIN)



FILES= ../../natives_base.cpp ../../../tokens.cpp ../../amx/amx/amx.c
INCS= -I../../amx/amx -I../../.. -I../.. -g
LIBS= 

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
	cp $(PLUGIN) $(PLUGIN_DEST)


$(PLUGIN) : $(OBJS)
	$(CXX)  -shared -W1,-soname,$(PLUGIN) $(LIBS) -o $(PLUGIN) $(OBJS)

