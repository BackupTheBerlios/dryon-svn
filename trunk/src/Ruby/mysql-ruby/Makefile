
NAME=mysql.so

CC=gcc
CFLAGS= -g

FILES= mysql.c

CFLAGS=-g -fPIC -c -I/usr/local/include

OBJS= $(FILES:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -shared -W1,-soname,$(NAME) -o $(NAME) $(OBJS)
	cp $(NAME) ../../../bin/libs/ruby/

clean:
	@rm -rf $(OBJS) *.so
