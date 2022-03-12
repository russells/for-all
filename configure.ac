#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([for-all], [1.0], [russells@adelie.cx])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
PKG_CHECK_MODULES([GLIB], [glib-2.0])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
 man/Makefile
])
AC_OUTPUT
