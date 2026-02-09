# Makefile for Greed
#
# SPDX-FileCopyrightText: (C) Eric S. Raymond <esr@thyrsus.com>
# SPDX-License-Identifier: BSD-2-Clause

PREFIX      ?= /usr/local
BINDIR      ?= $(PREFIX)/bin
DATADIR     ?= $(PREFIX)/share
MANDIR      ?= $(DATADIR)/man

SFILE=/usr/games/lib/greed.hs

VERS=$(shell sed -n <NEWS.adoc '/^[0-9]/s/:.*//p' | head -1)

# Rules

# Note: to suppress the footers with timestamps being generated in HTML,
# we use "-a nofooter".
# To debug asciidoc problems, you may need to run "xmllint --nonet --noout --valid"
# on the intermediate XML that throws an error.
.SUFFIXES: .html .adoc .6

.adoc.6:
	asciidoctor -D. -a nofooter -b manpage $<
.adoc.html:
	asciidoctor -D. -a nofooter -a webfonts! $<

.PHONY: all clean reflow cppcheck spellcheck install uninstall dist release refresh

# Build

all: greed greed.6

greed: greed.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -DSCOREFILE=\"$(SFILE)\" -DRELEASE=\"$(VERS)\" -o greed greed.c -O3 -lcurses

clean:
	rm -f *~ *.o greed greed-*.tar.gz  greed*.rpm *.html
	rm -f greed.6 manpage.links manpage.refs

# Validate

reflow:
	@clang-format --style="{IndentWidth: 8, UseTab: ForIndentation}" -i $$(find . -name "*.[ch]")

CPPCHECKOPTS =
cppcheck:
	@cppcheck --quiet --template=gcc $(CPPCHECKOPTS) greed.c

spellcheck:
	@spellcheck greed.adoc

# Install/uninstall

install: greed.6 uninstall
	install -m 0755 -d $(DESTDIR)/usr/bin
	install -m 0755 greed $(DESTDIR)/usr/bin/
	install -m 0755 -d $(DESTDIR)/usr/share/man/man6
	install -m 0644 greed.6 $(DESTDIR)/usr/share/man/man6/

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/greed
	rm -f $(DESTDIR)$(MANDIR)/man1/greed.6

# Export

SOURCES = README.adoc NEWS.adoc COPYING Makefile greed.c greed.adoc control greed-logo.png

greed-$(VERS).tar.gz: $(SOURCES) greed.6
	@ls $(SOURCES) greed.6 | sed s:^:greed-$(VERS)/: >MANIFEST
	@(cd ..; ln -s greed greed-$(VERS))
	(cd ..; tar -czf greed/greed-$(VERS).tar.gz `cat greed/MANIFEST`)
	@(cd ..; rm greed-$(VERS))

dist: greed-$(VERS).tar.gz

release: greed-$(VERS).tar.gz greed.html
	shipper version=$(VERS) | sh -e -x

refresh: greed.html
	shipper -N -w version=$(VERS) | sh -e -x

# end
