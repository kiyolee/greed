# Makefile for Greed

VERS=4.2

SFILE=/usr/games/lib/greed.hs
# Location of game executable
BIN=/usr/games

greed: greed.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -DSCOREFILE=\"$(SFILE)\" -DRELEASE=\"$(VERS)\" -o greed greed.c -O3 -lcurses

# Note: to suppress the footers with timestamps being generated in HTML,
# we use "-a nofooter".
# To debug asciidoc problems, you may need to run "xmllint --nonet --noout --valid"
# on the intermediate XML that throws an error.
.SUFFIXES: .html .adoc .6

.adoc.6:
	asciidoctor -D. -a nofooter -b manpage $<
.adoc.html:
	asciidoctor -D. -a nofooter -a webfonts! $<
install: greed.6 uninstall
	cp greed $(BIN)
	cp greed.6 /usr/share/man/man6/greed.6

uninstall:
	rm -f $(BIN)/install /usr/share/man/man6/greed.6

clean:
	rm -f *~ *.o greed greed-*.tar.gz  greed*.rpm *.html
	rm -f greed.6 manpage.links manpage.refs

reflow:
	@clang-format --style="{IndentWidth: 8, UseTab: ForIndentation}" -i $$(find . -name "*.[ch]")

CPPCHECKOPTS =
cppcheck:
	cppcheck $(CPPCHECKOPTS) greed.c

SOURCES = README NEWS COPYING Makefile greed.c greed.xml control greed-logo.png

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
