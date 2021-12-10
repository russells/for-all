# Make on-all.

PROG = for-all

# VersionNumber := $(shell grep ^VersionNumber $(PROG) | sed 's/.*=//')
# VersionDate   := $(shell grep ^VersionDate   $(PROG) | sed 's/.*=//')

CFLAGS += -Wall -Werror $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

.PHONY: all
all: $(PROG) man pdf

$(PROG): $(PROG).o
	$(CC) $(PROG).o -o $(PROG) $(LDFLAGS)

$(PROG).o: $(PROG).c

# .PHONY: install
# install: man
#	install -d ~/bin
#	install -m 755 -t ~/bin $(PROG)
#	install -d ~/share/man/man1
#	install -m 644 -t ~/share/man/man1 $(PROG).1

.PHONY: man
man: $(PROG).1
$(PROG).1: $(PROG).pod
	pod2man -r $(VersionNumber) -d $(VersionDate) -c "User commands" $< $@
pdf: $(PROG).pdf
$(PROG).pdf: $(PROG).1
	man -t ./$< | ps2pdf - - > $@ ; \
	if [ `stat -c %s $@` -eq 0 ] ; then rm -f $@ ; false ; else true ; fi

.PHONY: clean
clean:
	rm -f $(PROG) $(PROG).o $(PROG).1 $(PROG).pdf
