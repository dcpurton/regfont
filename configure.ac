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
AC_CHECK_HEADERS([string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

# Checks for hosts
case "$host" in
    *-w64-mingw*)
        ;;
    *)
        AC_MSG_ERROR([This application depends on the w64-mingw compiler! Did you forget --host?])
        ;;
esac

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
