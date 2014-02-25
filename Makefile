
CFLAGS		= -g -O2 -Wall -I../linux/include
PREFIX          ?= /usr
PROGRAMS	= atruncate

all: $(PROGRAMS)

distclean: clean
	$(RM) -rf $(PACKAGE)*
clean:
	$(RM) *.o $(PROGRAMS) *~

install: all
	install -d $(DESTDIR)$(PREFIX)/bin
	install $(PROGRAMS) $(DESTDIR)$(PREFIX)/bin

dist: clean
	rm -rf $(PACKAGE)
	mkdir $(PACKAGE)
	cp -a fftune.c Makefile $(PACKAGE)
	(cd $(PACKAGE); find . -name .svn -o -name *~ | xargs rm -rf)
	tar cjf $(PACKAGE).tar.bz2 $(PACKAGE)
	rm -rf $(PACKAGE)

.PHONY: all clean distclean install dist
