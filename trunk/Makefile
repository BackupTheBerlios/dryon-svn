
SUBDIR = src src/Small/Libraries 

all :
	@for dir in $(SUBDIR); do (echo "building $$dir...";cd $$dir; make all;); done

clean :
	@for dir in $(SUBDIR); do (echo "cleaning $$dir...";cd $$dir; make clean;); done
