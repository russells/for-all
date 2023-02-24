
# Flags to pass to the C compiler & linker
AM_CFLAGS = --pedantic -Wall -Werror -O0 -g

# Also use glib.
for_all_CFLAGS = $(GLIB_CFLAGS) -std=c99 --pedantic -Wall -Werror -O0 -g
for_all_LDADD = $(GLIB_LIBS)

bin_PROGRAMS = for-all
for_all_SOURCES = for-all.c run-command.c lists.c

for-all.c: version.h

version.h: VERSION-GEN make-version.sh
	./make-version.sh

clean-local:
	rm -f version.h
