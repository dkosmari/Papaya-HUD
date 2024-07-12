#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Papaya HUD], [0.1], [http://github.com/dkosmari/aroma-papaya-hud])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

WIIU_WUPS_INIT
WIIU_WUMS_INIT

AC_PROG_RANLIB

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CXX
AX_APPEND_COMPILE_FLAGS([-std=c++23], [CXXFLAGS])
AC_LANG([C++])

WIIU_WUMS_CHECK_LIBNOTIFICATIONS

AM_EXTRA_RECURSIVE_TARGETS([run])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/wupsxx/Makefile])
AC_OUTPUT
