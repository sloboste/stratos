CC			= gcc
SOURCES 	= $(wildcard *.c)
OBJECTS		= $(SOURCES:%.c=%.o)
EXECUTABLE 	= stratos

CCFLAGS = -std=c99 -Wall -Werror -Wextra -pedantic 

release: CCFLAGS += -DNDEBUG
release: all

debug: CCFLAGS += -g -DDEBUG
debug: clean all

all: $(OBJECTS)
	$(CC) $(CCFLAGS) $(OBJECTS) -o $(EXECUTABLE)

# individual dependencies for objects
main.o: main.c flash.h spi.h swipe.h
flash.o: flash.c flash.h
spi.o: spi.c spi.h
swipe.o: swipe.c swipe.h

# rule for creating objects
%.o:
	$(CC) $(CCFLAGS) -c $*.c

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all release debug clean 
.SUFFIXES: 

