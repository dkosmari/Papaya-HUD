# -*- mode: autoconf -*-
# wiiu_env_libnotifications.m4 - Macros to handle libnotifications
# URL: https://github.com/dkosmari/devkitpro-autoconf/

# Copyright (c) 2026 Daniel K. O. <dkosmari>
#
# Copying and distribution of this file, with or without modification, are permitted in
# any medium without royalty provided the copyright notice and this notice are
# preserved. This file is offered as-is, without any warranty.

#serial 1

# WIIU_ENV_SETUP_LIBNOTIFICATIONS([RELATIVE-PATH-TO-LIBNOTIFICATIONS])
# --------------------------------------------------------------------
#
# Sets up usage of libnotifications as a submodule.
#
# Output variables:
#   - `LIBNOTIFICATIONS_DIR'
#   - `LIBNOTIFICATIONS_CPPFLAGS'
#   - `LIBNOTIFICATIONS_LIB'
#   - custom Makefile rules (use @INC_AMINCLUDE@ in your `Makefile.am')

AC_DEFUN([WIIU_ENV_SETUP_LIBNOTIFICATIONS],[dnl
    WIIU_ENV_SETUP_LIB([LIBNOTIFICATIONS], [], [$1], [${DEVKITPRO}/wums])
])dnl WIIU_ENV_SETUP_LIBNOTIFICATIONS
