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

CFLAGS=-O -s
# For MSDOS:
#CFLAGS=-Oilt -Gs
# For Linux:
CFLAGS = -I/usr/local/include -L/usr/local/lib

# You may need to modify the libraries used below according to your site,
# e.g. -ltermcap for Berkeley and Xenix, -ltermlib for AT&T SYSV, -lncurses
# for Linux, and scurses.lib for MSDOS with PCCurses.
#LIBS=-lcurses -ltermcap
#LIBS=-lcurses -ltermlib
#LIBS=scurses.lib
LIBS=-lncurses

greed: greed.c
	cc -DSCOREFILE=\"$(SFILE)\" -D$(SYSDEF) -o greed greed.c $(CFLAGS) $(LIBS)

install: greed
	cp greed $(BIN)
	chmod 4711 $(BIN)/greed

clean:
	rm -f *.o greed greed.tar

tar:
	tar cvf greed.tar READ.ME Makefile greed.c
