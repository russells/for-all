# Make for-all.

default: for-all manpage
PHONY: for-all manpage

for-all:
	make -C src
manpage:
	make -C man

.PHONY: clean
clean:
	make -C src clean
	make -C man clean
