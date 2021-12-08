# Make on-all.

PROG = for-all

# VersionNumber := $(shell grep ^VersionNumber $(PROG) | sed 's/.*=//')
# VersionDate   := $(shell grep ^VersionDate   $(PROG) | sed 's/.*=//')

.PHONY: all
all: man pdf

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
	rm -f $(PROG).1 $(PROG).pdf
