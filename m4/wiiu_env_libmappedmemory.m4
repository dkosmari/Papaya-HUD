# -*- mode: autoconf -*-
# wiiu_env_libmappedmemory.m4 - Macros to handle libmappedmemory
# URL: https://github.com/dkosmari/devkitpro-autoconf/

# Copyright (c) 2026 Daniel K. O. <dkosmari>
#
# Copying and distribution of this file, with or without modification, are permitted in
# any medium without royalty provided the copyright notice and this notice are
# preserved. This file is offered as-is, without any warranty.

#serial 1

# WIIU_ENV_SETUP_LIBMAPPEDMEMORY([RELATIVE-PATH-TO-LIBMAPPEDMEMORY])
# ------------------------------------------------------------------
#
# Sets up usage of libmappedmemory as a submodule.
#
# Output variables:
#   - `LIBMAPPEDMEMORY_DIR'
#   - `LIBMAPPEDMEMORY_CPPFLAGS'
#   - `LIBMAPPEDMEMORY_LDFLAGS'
#   - `LIBMAPPEDMEMORY_LIB'
#   - custom Makefile rules (use @INC_AMINCLUDE@ in your `Makefile.am')

AC_DEFUN([WIIU_ENV_SETUP_LIBMAPPEDMEMORY],[dnl
    WIIU_ENV_SETUP_LIB([LIBMAPPEDMEMORY], [], [$1], [${DEVKITPRO}/wums])

    AC_ARG_VAR([LIBMAPPEDMEMORY_LDFLAGS], [linker flags for LIBMAPPEDMEMORY])
    AS_VAR_SET([LIBMAPPEDMEMORY_LDFLAGS], ["-T${LIBMAPPEDMEMORY_DIR}/share/libmappedmemory.ld"])
    AC_SUBST([LIBMAPPEDMEMORY_LDFLAGS])
])dnl WIIU_ENV_SETUP_LIBMAPPEDMEMORY
