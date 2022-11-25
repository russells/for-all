#

SUBDIRS = src man
dist_doc_DATA = README

README: README.md Makefile.am
	pandoc -t markdown_github -o README README.md

