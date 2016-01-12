#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([regfont],[20160109],[dcpurton@marshwiggle.net])
AC_CANONICAL_HOST
AC_CONFIG_SRCDIR([src/regfont.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADER([windows.h],,AC_MSG_ERROR([windows.h not found.]))
AC_CHECK_HEADER([getopt.h],,AC_MSG_ERROR([getopt.h not found.]))

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNC(getopt_long,,AC_MSG_ERROR([function getopt_long not found.]))

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
