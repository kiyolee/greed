# Makefile for Greed
# MSDOS users need PCCurses v1.3 (or later) and Microsoft C 5.1, plus
# slight editing of "chmod", "cp", and "rm" statements at bottom of this file.

VERS=$(shell sed <greed.spec -n -e '/Version: \(.*\)/s//\1/p')

# Choose BSD for Berkeley Unix, NOTBSD for all other Unixes, MSDOS for DOS
#SYSDEF=BSD
SYSDEF=NOTBSD
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
	cc -DSCOREFILE=\"$(SFILE)\" -D$(SYSDEF) -DRELEASE=\"$(VERS)\" -o greed greed.c $(CFLAGS) $(TERMLIB)

greed.6: greed.xml
	xmlto man greed.xml

install: greed.6 uninstall
	cp greed $(BIN)
	cp greed.6 /usr/share/man/man6/greed.6

uninstall:
	rm -f $(BIN)/install /usr/share/man/man6/greed.6

clean:
	rm -f *~ *.o greed greed-*.tar.gz  greed*.rpm
	rm -f greed.6 manpage.links manpage.refs

SOURCES = README COPYING Makefile greed.c greed.spec greed.xml

greed-$(VERS).tar.gz: $(SOURCES) greed.6
	@ls $(SOURCES) greed.6 | sed s:^:greed-$(VERS)/: >MANIFEST
	@(cd ..; ln -s greed greed-$(VERS))
	(cd ..; tar -czvf greed/greed-$(VERS).tar.gz `cat greed/MANIFEST`)
	@(cd ..; rm greed-$(VERS))

dist: greed-$(VERS).tar.gz
