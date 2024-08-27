# -*- mode: autoconf -*-
# wiiu_wums.m4 - Macros to handle Wii U Module System.
# URL: https://github.com/dkosmari/devkitpro-autoconf/

# Copyright (c) 2024 Daniel K. O. <dkosmari>
#
# Copying and distribution of this file, with or without modification, are permitted in
# any medium without royalty provided the copyright notice and this notice are
# preserved. This file is offered as-is, without any warranty.

#serial 3

# WIIU_WUMS_INIT
# --------------
#
# This macro adjusts the environment to use the Wii U Module System (WUMS).
#
# Output variables:
#   - `DEVKITPRO_CPPFLAGS'
#   - `DEVKITPRO_LIBS'
#
# Note: to create a module, see `WIIU_WUMS_MODULE_INIT'.

AC_DEFUN([WIIU_WUMS_INIT],[

    AC_REQUIRE([DEVKITPRO_WUT_INIT])

    # set WIIU_WUMS_ROOT
    AS_VAR_SET([WIIU_WUMS_ROOT], [$DEVKITPRO/wums])

    AX_PREPEND_FLAG([-I$WIIU_WUMS_ROOT/include], [DEVKITPRO_CPPFLAGS])

    AX_PREPEND_FLAG([-L$WIIU_WUMS_ROOT/lib], [DEVKITPRO_LIBS])

])


# WIIU_WUMS_CHECK_LIBCURLWRAPPER([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# ------------------------------------------------------------------------
#
# Checks for presence of libcurlwrapper.
#
# Output variables:
#   - `DEVKITPRO_LIBS'
#   - `HAVE_WIIU_WUMS_CHECK_LIBCURLWRAPPER'

AC_DEFUN([WIIU_WUMS_CHECK_LIBCURLWRAPPER],[

    AC_REQUIRE([WIIU_WUMS_INIT])

    DEVKITPRO_CHECK_LIBRARY([WIIU_WUMS_LIBCURLWRAPPER],
                            [curl/curl.h],
                            [curlwrapper],
                            [],
                            [$1],
                            m4_default([$2],
                                       [AC_MSG_ERROR([libcurlwrapper not found in $WIIU_WUMS_ROOT; get it from https://github.com/wiiu-env/libcurlwrapper])]
                                      )
                           )

])


# WIIU_WUMS_CHECK_LIBMAPPEDMEMORY([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# -------------------------------------------------------------------------
#
# Checks for presence of libmappedmemory.
#
# Output variables:
#   - `DEVKITPRO_LIBS'
#   - `DEVKITPRO_LDFLAGS`
#   - `HAVE_WIIU_WUMS_LIBMAPPEDMEMORY'

AC_DEFUN([WIIU_WUMS_CHECK_LIBMAPPEDMEMORY],[

    AC_REQUIRE([WIIU_WUMS_INIT])

    DEVKITPRO_CHECK_LIBRARY([WIIU_WUMS_LIBMAPPEDMEMORY],
                            [memory/mappedmemory.h],
                            [mappedmemory],
                            [-T$WIIU_WUMS_ROOT/share/libmappedmemory.ld],
                            [$1],
                            m4_default([$2],
                                       [AC_MSG_ERROR([libmappedmemory not found in $WIIU_WUMS_ROOT; get it from https://github.com/wiiu-env/libmappedmemory])]
                                      )
                           )

])


# WIIU_WUMS_CHECK_LIBNOTIFICATIONS([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# --------------------------------------------------------------------------
#
# Checks for presence of libnotifications.
#
# Output variables:
#   - `DEVKITPRO_LIBS'
#   - `HAVE_WIIU_WUMS_LIBNOTIFICATIONS'

AC_DEFUN([WIIU_WUMS_CHECK_LIBNOTIFICATIONS],[

    AC_REQUIRE([WIIU_WUMS_INIT])

    DEVKITPRO_CHECK_LIBRARY([WIIU_WUMS_LIBNOTIFICATIONS],
                            [notifications/notifications.h],
                            [notifications],
                            [],
                            [$1],
                            m4_default([$2],
                                       [AC_MSG_ERROR([libnotifications not found in $WIIU_WUMS_ROOT; get it from https://github.com/wiiu-env/libnotifications])]
                                      )
                           )

])


# WIIU_WUMS_MODULE_INIT
# ---------------------
#
# This macro adjusts the environment to create a Wii U Module System (WUMS) module.

AC_DEFUN([WIIU_WUMS_MODULE_INIT],[

    AC_REQUIRE([DEVKITPRO_WUT_INIT])

    # set WIIU_WUMS_ROOT
    AS_VAR_SET([WIIU_WUMS_ROOT], [$DEVKITPRO/wums])

    AX_PREPEND_FLAG([-I$WIIU_WUMS_ROOT/include], [DEVKITPRO_CPPFLAGS])

    AX_PREPEND_FLAG([-specs=$WIIU_WUMS_ROOT/share/wums.specs], [DEVKITPRO_LDFLAGS])
    AX_PREPEND_FLAG([-T$WIIU_WUMS_ROOT/share/wums.ld],         [DEVKITPRO_LDFLAGS])


    AX_PREPEND_FLAG([-L$WIIU_WUMS_ROOT/lib], [DEVKITPRO_LIBS])

    # do a compilation test to check for header and lib
    DEVKITPRO_CHECK_LIBRARY([WIIU_WUMS_MODULE_TEST],
                            [wums.h],
                            [wums],
                            [],
                            [],
                            [WUMS not found in $WIIU_WUMS_ROOT; get it from https://github.com/wiiu-env/WiiUModuleSystem])


    # custom Makefile rules
    AX_ADD_AM_MACRO([
clean: clean-wms
.PHONY: clean-wms
clean-wms:; \$(RM) *.wms
%.wms: %.strip.elf;
	\$(ELF2RPL) \$< \$[@]
	printf '\xAF\xFE' | dd of=\$[@] bs=1 seek=9 count=2 conv=notrunc status=none
])

])
