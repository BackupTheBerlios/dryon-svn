
SUBDIR = GameServers MySQL Timers

all :
	@for dir in $(SUBDIR); do (echo "building $$dir...";cd $$dir; make -s all;); done

clean :
	@for dir in $(SUBDIR); do (echo "cleaning $$dir...";cd $$dir; make clean;); done
