#!/bin/sh
set -x
autoreconf --install &&
./configure --prefix=$HOME &&
make
