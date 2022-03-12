#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([for-all], [1.0], [russells@adelie.cx])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([dup2 regcomp strerror])

AC_CHECK_HEADERS([wait.h sys/wait.h])

PKG_CHECK_MODULES([GLIB], [glib-2.0])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
 man/Makefile
])
AC_OUTPUT
