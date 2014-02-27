CFLAGS		= -g -O2 -Wall -I../linux/include
PREFIX          ?= /usr
PROGRAM		= atruncate
PACKAGE		= $(PROGRAM)-`git describe --tags`

all: $(PROGRAM)

distclean: clean
	$(RM) -rf $(PACKAGE)*
clean:
	$(RM) *.o $(PROGRAM) *~

install: all
	install -d $(DESTDIR)$(PREFIX)/bin
	install $(PROGRAM) $(DESTDIR)$(PREFIX)/bin

dist: clean
	rm -rf $(PACKAGE)
	mkdir $(PACKAGE)
	cp -a *.c Makefile $(PACKAGE)
	(cd $(PACKAGE); find . -name .svn -o -name *~ | xargs rm -rf)
	tar cjf $(PACKAGE).tar.bz2 $(PACKAGE)
	rm -rf $(PACKAGE)

.PHONY: all clean distclean install dist
