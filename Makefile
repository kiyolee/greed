# Makefile for Greed
# MSDOS users need PCCurses v1.3 (or later) and Microsoft C 5.1, plus
# slight editing of "chmod", "cp", and "rm" statements at bottom of this file.

# Note: When the version changes, you also have to change
#  * the name of the containing directory
#  * the RPM spec file
V=3.2

# Choose BSD for Berkeley Unix, NOTBSD for all other Unixes, MSDOS for DOS
SYSDEF=BSD
#SYSDEF=NOTBSD
#SYSDEF=MSDOS

# Prepend "c:" (or whatever drive you use) to the following paths for MSDOS
# Location of high score file
SFILE=/usr/games/lib/greed.hs
# Location of game executable
BIN=/usr/games

# Flags for use with the Linux ncurses package (recommended)
CFLAGS = -O -s -DNOTBSD
TERMLIB = -lncurses
CC = gcc

# Flags for use with stock curses
# CFLAGS = -O -s
# TERMLIB = -lcurses
# CC = gcc

# Flags for use with MS-DOS
#CFLAGS=-Oilt -Gs
#TERMLIB = scurses.lib

greed: greed.c
	cc -DSCOREFILE=\"$(SFILE)\" -D$(SYSDEF) -o greed greed.c $(CFLAGS) $(TERMLIB)

install: greed
	cp greed $(BIN)
	chmod 4711 $(BIN)/greed

SOURCES = READ.ME Makefile greed.c greed.lsm greed.spec

greed.tar:
	(cd ..; tar -cvf greed-$(V)/greed.tar `echo $(SOURCES) | sed "/\(^\| \)/s// greed-$(V)\//g"`)
greed.tar.gz: greed.tar
	gzip -f greed.tar

greed.shar:
	shar $(SOURCES) >greed.shar
clean:
	rm -f *.o greed greed.tar

rpm: greed.tar.gz
	cp greed.tar.gz /usr/src/SOURCES/greed-$(V).tar.gz
	cp greed.spec /usr/src/SPECS/greed-$(V)-1.spec
	rpm -ba greed-$(V)-1.spec
