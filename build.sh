#!/bin/sh
set -x
autoreconf --install &&
./configure &&
make
