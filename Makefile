# Makefile for Greed
# MSDOS users need PCCurses v1.3 (or later) and Microsoft C 5.1, plus
# slight editing of "chmod", "cp", and "rm" statements at bottom of this file.

# Note: When the version changes, you also have to change
# the RPM spec file and the LSM.
VERS=3.3

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
	cc -DSCOREFILE=\"$(SFILE)\" -D$(SYSDEF) -DVERS=\"$(VERS)\" -o greed greed.c $(CFLAGS) $(TERMLIB)

install: greed
	cp greed $(BIN)
	chmod 4711 $(BIN)/greed

clean:
	rm -f *~ *.o greed greed-*.tar.gz  greed*.rpm

SOURCES = READ.ME Makefile greed.c greed.lsm greed.spec greed.6

greed-$(VERS).tar.gz: $(SOURCES)
	@ls $(SOURCES) | sed s:^:greed-$(VERS)/: >MANIFEST
	@(cd ..; ln -s greed greed-$(VERS))
	(cd ..; tar -czvf greed/greed-$(VERS).tar.gz `cat greed/MANIFEST`)
	@(cd ..; rm greed-$(VERS))

greed-$(VERS).shar:
	shar $(SOURCES) >greed-$(VERS).shar

dist: greed-$(VERS).tar.gz

RPMROOT=/usr/src/redhat
RPM = rpm
RPMFLAGS = -ba
rpm: dist
	cp greed-$(VERS).tar.gz $(RPMROOT)/SOURCES;
	cp greed.spec $(RPMROOT)/SPECS
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) greed.spec	
	cp $(RPMROOT)/RPMS/`arch|sed 's/i[4-9]86/i386/'`/greed-$(VERS)*.rpm .
	cp $(RPMROOT)/SRPMS/greed-$(VERS)*.src.rpm .
