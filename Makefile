# Makefile for Greed v3.2
# MSDOS users need PCCurses v1.3 (or later) and Microsoft C 5.1, plus
# slight editing of "chmod", "cp", and "rm" statements at bottom of this file.

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
CFLAGS = -O -s -DNCURSES  -I/usr/local/include -L/usr/local/lib
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

greed.tar:
	tar cvf greed.tar READ.ME Makefile greed.c greed.lsm
greed.tar.gz: greed.tar
	gzip greed.tar

greed.shar:
	shar READ.ME Makefile greed.c greed.lsm >greed.shar
clean:
	rm -f *.o greed greed.tar

