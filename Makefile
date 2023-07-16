CC=gcc
CFLAGS=-g -pedantic -std=gnu17 -Wall -Werror -Wextra

.PHONY: all
all: nyush

nyush: nyush.o interface.o patternmatch.o commands.o helper.o parser.o backgroundprocesses.o

nyush.o: nyush.c interface.o patternmatch.o commands.o helper.o parser.o backgroundprocesses.o

interface.o: interface.c interface.h patternmatch.o

patternmatch.o: patternmatch.c patternmatch.h

commands.o: commands.c commands.h

helper.o: helper.c helper.h

parser.o: parser.c parser.h

backgroundprocesses.o: backgroundprocesses.c backgroundprocesses.h

.PHONY: clean
clean:
	rm -f *.o nyush
