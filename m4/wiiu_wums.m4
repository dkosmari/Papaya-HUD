# -*- mode: autoconf -*-
# wiiu_wums.m4 - Macros to handle Wii U Module System.
# URL: https://github.com/dkosmari/devkitpro-autoconf/

# Copyright (c) 2025 Daniel K. O. <dkosmari>
#
# Copying and distribution of this file, with or without modification, are permitted in
# any medium without royalty provided the copyright notice and this notice are
# preserved. This file is offered as-is, without any warranty.

#serial 9

# WIIU_WUMS_INIT
# --------------
#
# This macro adjusts the environment to use the Wii U Module System (WUMS).
#
# Output variables:
#   - `WIIU_WUMS'
#   - `LIBS'
#
# Note: to create a module, see `WIIU_WUMS_MODULE_INIT'.

AC_DEFUN([WIIU_WUMS_INIT],[

    AC_REQUIRE([DEVKITPRO_WUT_INIT])

    # set WIIU_WUMS
    AC_ARG_VAR([WIIU_WUMS], [path to wums])
    AS_VAR_SET([WIIU_WUMS], [$DEVKITPRO/wums])
    AC_SUBST([WIIU_WUMS])

])dnl WIIU_WUMS_INIT


# WIIU_WUMS_SETUP
# ---------------
#
# This macro adjusts compilation flags to use the Wii U Module System (WUMS).
#
# Output variables:
#   - `CPPFLAGS'
#   - `LIBS'
#
# Note: to create a module, see `WIIU_WUMS_MODULE_SETUP'.

AC_DEFUN([WIIU_WUMS_SETUP],[

    AC_REQUIRE([DEVKITPRO_WUT_SETUP])

    AS_VAR_SET_IF([WIIU_WUMS], [], [AC_MSG_ERROR([WIIU_WUMS not set])])

    AX_PREPEND_FLAG([-I$WIIU_WUMS/include], [CPPFLAGS])

    AX_PREPEND_FLAG([-L$WIIU_WUMS/lib], [LIBS])

])dnl WIIU_WUMS_SETUP


# WIIU_WUMS_CHECK_LIBBUTTONCOMBO([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# ------------------------------------------------------------------------
#
# Checks for presence of libbuttoncombo.
#
# Output variables:
#   - `HAVE_WIIU_WUMS_LIBBUTTONCOMBO'
#   - `WIIU_WUMS_LIBBUTTONCOMBO_LIBS'

AC_DEFUN([WIIU_WUMS_CHECK_LIBBUTTONCOMBO],[

    DEVKITPRO_CHECK_LIBRARY_FULL([WIIU_WUMS_LIBBUTTONCOMBO],
                                 [buttoncombo/api.h],
                                 [buttoncombo],
                                 [],
                                 [],
                                 [$1],
                                 m4_default([$2],
                                            [AC_MSG_ERROR([libbuttoncombo not found; get it from https://github.com/wiiu-env/libbuttoncombo])]))

])dnl WIIU_WUMS_CHECK_LIBBUTTONCOMBO


# WIIU_WUMS_CHECK_LIBCURLWRAPPER([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# ------------------------------------------------------------------------
#
# Checks for presence of libcurlwrapper.
#
# Output variables:
#   - `HAVE_WIIU_WUMS_LIBCURLWRAPPER'
#   - `WIIU_WUMS_LIBCURLWRAPPER_LIBS'

AC_DEFUN([WIIU_WUMS_CHECK_LIBCURLWRAPPER],[

    DEVKITPRO_CHECK_LIBRARY_FULL([WIIU_WUMS_LIBCURLWRAPPER],
                                 [curl/curl.h],
                                 [curlwrapper],
                                 [],
                                 [],
                                 [$1],
                                 m4_default([$2],
                                            [AC_MSG_ERROR([libcurlwrapper not found; get it from https://github.com/wiiu-env/libcurlwrapper])]))

])dnl WIIU_WUMS_CHECK_LIBCURLWRAPPER


# WIIU_WUMS_CHECK_LIBFUNCTIONPATCHER([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# ----------------------------------------------------------------------------
#
# Checks for presence of libfunctionpatcher.
#
# Output variables:
#   - `HAVE_WIIU_WUMS_LIBFUNCTIONPATCHER'
#   - `WIIU_WUMS_LIBFUNCTIONPATCHER_LIBS'

AC_DEFUN([WIIU_WUMS_CHECK_LIBFUNCTIONPATCHER],[

    DEVKITPRO_CHECK_LIBRARY_FULL([WIIU_WUMS_LIBFUNCTIONPATCHER],
                                 [function_patcher/function_patching.h],
                                 [functionpatcher],
                                 [],
                                 [],
                                 [$1],
                                 m4_default([$2],
                                            [AC_MSG_ERROR([libfunctionpatcher not found; get it from https://github.com/wiiu-env/libfunctionpatcher])]))

])dnl WIIU_WUMS_CHECK_LIBFUNCTIONPATCHER


# WIIU_WUMS_CHECK_LIBMAPPEDMEMORY([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# -------------------------------------------------------------------------
#
# Checks for presence of libmappedmemory.
#
# Output variables:
#   - `HAVE_WIIU_WUMS_LIBMAPPEDMEMORY'
#   - `WIIU_WUMS_LIBMAPPEDMEMORY_LDFLAGS'
#   - `WIIU_WUMS_LIBMAPPEDMEMORY_LIBS'

AC_DEFUN([WIIU_WUMS_CHECK_LIBMAPPEDMEMORY],[

    AS_VAR_SET_IF([WIIU_WUMS], [], [AC_MSG_ERROR([WIIU_WUMS not set.])])

    DEVKITPRO_CHECK_LIBRARY_FULL([WIIU_WUMS_LIBMAPPEDMEMORY],
                                 [memory/mappedmemory.h],
                                 [mappedmemory],
                                 [],
                                 [-T$WIIU_WUMS/share/libmappedmemory.ld],
                                 [$1],
                                 m4_default([$2],
                                            [AC_MSG_ERROR([libmappedmemory not found; get it from https://github.com/wiiu-env/libmappedmemory])]))

])dnl WIIU_WUMS_CHECK_LIBMAPPEDMEMORY


# WIIU_WUMS_CHECK_LIBNOTIFICATIONS([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# --------------------------------------------------------------------------
#
# Checks for presence of libnotifications.
#
# Output variables:
#   - `HAVE_WIIU_WUMS_LIBNOTIFICATIONS'
#   - `WIIU_WUMS_LIBNOTIFICATIONS_LIBS'

AC_DEFUN([WIIU_WUMS_CHECK_LIBNOTIFICATIONS],[

    DEVKITPRO_CHECK_LIBRARY_FULL([WIIU_WUMS_LIBNOTIFICATIONS],
                                 [notifications/notifications.h],
                                 [notifications],
                                 [],
                                 [],
                                 [$1],
                                 m4_default([$2],
                                            [AC_MSG_ERROR([libnotifications not found; get it from https://github.com/wiiu-env/libnotifications])]))

])dnl WIIU_WUMS_CHECK_LIBNOTIFICATIONS


# WIIU_WUMS_MODULE_INIT
# ---------------------
#
# This macro adjusts the environment to create a Wii U Module System (WUMS) module.

AC_DEFUN([WIIU_WUMS_MODULE_INIT],[

    AC_REQUIRE([WIIU_WUMS_INIT])

])dnl WIIU_WUMS_MODULE_INIT


# WIIU_WUMS_MODULE_SETUP
# ----------------------
#
# This macro adjusts the compilation flags to create a Wii U Module System (WUMS) module.
#
# Output variables:
#   - `CPPFLAGS'
#   - `LDFLAGS'
#   - `LIBS'

AC_DEFUN([WIIU_WUMS_MODULE_SETUP],[

    AC_REQUIRE([WIIU_WUMS_SETUP])

    AX_PREPEND_FLAG([-I$WIIU_WUMS/include], [CPPFLAGS])

    AX_PREPEND_FLAG([-specs=$WIIU_WUMS/share/wums.specs], [LDFLAGS])
    AX_PREPEND_FLAG([-T$WIIU_WUMS/share/wums.ld],         [LDFLAGS])

    AX_PREPEND_FLAG([-L$WIIU_WUMS/lib], [LIBS])

    # do a compilation test to check for header and lib
    DEVKITPRO_CHECK_LIBRARY([wums.h],
                            [wums],
                            [],
                            [],
                            [AX_PREPEND_FLAG([-lwums], [LIBS])],
                            [AC_MSG_ERROR([WUMS not found; get it from https://github.com/wiiu-env/WiiUModuleSystem])])

    # custom Makefile recipes for building .wms modules.
    AX_ADD_AM_MACRO([
clean: clean-wms
.PHONY: clean-wms
clean-wms:; \$(RM) *.wms
%.wms: %.strip.elf;
	\$(ELF2RPL) \$< \$[@]
	printf '\xAF\xFE' | dd of=\$[@] bs=1 seek=9 count=2 conv=notrunc status=none
])

])dnl WIIU_WUMS_MODULE_SETUP
