# -*- mode: autoconf -*-
# wiiu_env_libbuttoncombo.m4 - Macros to handle libbuttoncombo
# URL: https://github.com/dkosmari/devkitpro-autoconf/

# Copyright (c) 2026 Daniel K. O. <dkosmari>
#
# Copying and distribution of this file, with or without modification, are permitted in
# any medium without royalty provided the copyright notice and this notice are
# preserved. This file is offered as-is, without any warranty.

#serial 1

# WIIU_ENV_SETUP_LIBBUTTONCOMBO([RELATIVE-PATH-TO-LIBBUTTONCOMBO])
# ----------------------------------------------------------------
#
# Sets up usage of libbuttoncombo as a submodule.
#
# Output variables:
#   - `LIBBUTTONCOMBO_DIR'
#   - `LIBBUTTONCOMBO_CPPFLAGS'
#   - `LIBBUTTONCOMBO_LIB'
#   - custom Makefile rules (use @INC_AMINCLUDE@ in your `Makefile.am')

AC_DEFUN([WIIU_ENV_SETUP_LIBBUTTONCOMBO],[dnl
    WIIU_ENV_SETUP_LIB([LIBBUTTONCOMBO], [], [$1], [${DEVKITPRO}/wums])
])dnl WIIU_ENV_SETUP_LIBBUTTONCOMBO
