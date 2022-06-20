#

SUBDIRS = src man
dist_doc_DATA = README

README: README.md Makefile.am
	pandoc -t gfm -o README README.md

