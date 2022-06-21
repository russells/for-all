
# Flags to pass to the C compiler & linker
AM_CFLAGS = --pedantic -Wall -Werror -O0 -g

# Also use glib.
for_all_CFLAGS = $(GLIB_CFLAGS) --pedantic -Wall -Werror -O0 -g
for_all_LDADD = $(GLIB_LIBS)

bin_PROGRAMS = for-all
for_all_SOURCES = for-all.c run-command.c lists.c
