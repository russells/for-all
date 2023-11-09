#!/bin/sh

PANDOC=$(which pandoc 2>/dev/null)
if [ -z "$PANDOC" ] ; then
	echo 1>&2 "Warning: no pandoc found."
	cp -v README.md README
else
	pandoc -t markdown_github -o README README.md || pandoc -t markdown -o README README.md
fi

