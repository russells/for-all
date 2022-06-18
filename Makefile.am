#

SUBDIRS = src man
dist_doc_DATA = README

README: README.md
	pandoc -t plain -o README README.md

