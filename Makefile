# Make on-all.

PROG = for-all

SRCS = $(PROG).c lists.c
OBJS = $(SRCS:.c=.o)

DEPDEPS = Makefile

%.d: %.c $(DEPDEPS)
	@echo DEP: $<
	@rm -f $@ $(@:.d=.u)
	@$(CC) -E -M $(CFLAGS) -o $@ $<

DEPS = $(SRCS:.c=.d)

# VersionNumber := $(shell grep ^VersionNumber $(PROG) | sed 's/.*=//')
# VersionDate   := $(shell grep ^VersionDate   $(PROG) | sed 's/.*=//')

CFLAGS += -g -O0 -Wall -Werror $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all
all: $(PROG) man pdf

$(PROG): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

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

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

.PHONY: clean
clean:
	rm -f $(PROG) $(OBJS) $(PROG).1 $(PROG).pdf $(DEPS)
