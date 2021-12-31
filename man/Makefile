# Make on-all.

PROG = for-all

DEPDEPS = Makefile

default: man pdf

.PHONY: man
man: $(PROG).1
$(PROG).1: $(PROG).pod
	pod2man -r $(VersionNumber) -d $(VersionDate) -c "User commands" $< $@
pdf: $(PROG).pdf
$(PROG).pdf: $(PROG).1
	man -Tps ./$< | ps2pdf - - > $@ ; \
	if [ `stat -c %s $@` -eq 0 ] ; then rm -f $@ ; false ; else true ; fi

.PHONY: clean
clean:
	rm -f $(PROG).1 $(PROG).pdf
