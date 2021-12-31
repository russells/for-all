# Make on-all.

PROG = for-all

SRCS = $(PROG).c lists.c run-command.c
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
all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

.PHONY: clean
clean:
	rm -f $(PROG) $(OBJS) $(DEPS)
