#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Papaya HUD], [0.1],
        [https://github.com/dkosmari/Papaya-HUD/issues], ,
        [https://github.com/dkosmari/Papaya-HUD])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

WIIU_WUPS_INIT
WIIU_WUMS_INIT

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CXX
AC_LANG([C++])
AX_APPEND_COMPILE_FLAGS([-std=c++23], [CXX])

WIIU_WUMS_CHECK_LIBNOTIFICATIONS

AM_EXTRA_RECURSIVE_TARGETS([run])

AC_CONFIG_SUBDIRS([external/libwupsxx])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
