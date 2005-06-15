
NAME=gameservers_ruby.so

CC=gcc
CFLAGS= -g

FILES= hl.c gameservers_wrapper.cpp

CFLAGS=-g -fPIC -c -I/usr/local/include

OBJS= $(FILES:.c=.o)

all: gameservers_wrapper.cpp $(NAME)

$(NAME): $(OBJS)
	$(CC) -shared -o $(NAME) $(OBJS) -I../ruby
	cp $(NAME) ../../../bin/libs/ruby/

gameservers_wrapper.cpp : gameservers.i
	echo "Building interface file..."
	@swig -Wall -c++ -ruby -o gameservers_wrapper.cpp gameservers.i

clean:
	@rm -rf $(OBJS) *.so
