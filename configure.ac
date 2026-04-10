#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([Papaya HUD],
        [0.4.1+],
        [https://github.com/dkosmari/Papaya-HUD/issues],
        [papaya-hud],
        [https://github.com/dkosmari/Papaya-HUD])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AC_CANONICAL_HOST

DEVKITPRO_WUT_INIT

AM_INIT_AUTOMAKE([foreign subdir-objects tar-pax])

LT_INIT([disable-shared])

AC_PROG_CXX
AC_LANG([C++])
AX_APPEND_COMPILE_FLAGS([-std=c++23], [CXX])

DEVKITPRO_WUT_SETUP

WIIU_ENV_SETUP_LIBBUTTONCOMBO([external/libwupsxx/external/libbuttoncombo])
WIIU_ENV_SETUP_LIBNOTIFICATIONS([external/libwupsxx/external/libnotifications])
WIIU_ENV_SETUP_LIBMAPPEDMEMORY([external/libmappedmemory])
WIIU_ENV_SETUP_WUPS([external/libwupsxx/external/WiiUPluginSystem])

AX_SUBDIRS_CONFIGURE([external/libwupsxx],
                     [[--enable-shortcuts],
                      [--enable-notify]])

AC_CONFIG_FILES([Makefile
                 external/Makefile])
AC_OUTPUT
