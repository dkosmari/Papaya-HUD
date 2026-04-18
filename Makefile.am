EXTRA_DIST = \
	assets \
	AUTHORS \
	bootstrap \
	COPYING \
	docker-build.sh \
	Dockerfile \
	README.md


SUBDIRS = \
	external/libmappedmemory \
	external/libwupsxx

DIST_SUBDIRS = external


external/libwupsxx/lib/libwupsxx.la:
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C external/libwupsxx V=$V


AM_CPPFLAGS = \
	$(WUPS_CPPFLAGS) \
	$(LIBBUTTONCOMBO_CPPFLAGS) \
	$(LIBMAPPEDMEMORY_CPPFLAGS) \
	$(LIBNOTIFICATIONS_CPPFLAGS) \
	-I$(srcdir)/external/libwupsxx/include

AM_CXXFLAGS = -Wall -Wextra -Werror

AM_LDFLAGS = \
	$(LIBMAPPEDMEMORY_LDFLAGS) \
	$(WUPS_LDFLAGS)

LDADD = \
	external/libwupsxx/lib/libwupsxx.la \
	$(LIBBUTTONCOMBO_LIB) \
	$(LIBMAPPEDMEMORY_LIB) \
	$(LIBNOTIFICATIONS_LIB) \
	$(WUPS_LIB)


ELF_FILE := papaya-hud.elf
WPS_FILE := $(ELF_FILE:.elf=.wps)


noinst_PROGRAMS = $(ELF_FILE)

papaya_hud_elf_SOURCES = \
	src/ax_mon.cpp \
	src/ax_mon.hpp \
	src/cfg.cpp \
	src/cfg.hpp \
	src/cpu_mon.cpp \
	src/cpu_mon.hpp \
	src/fs_mon.cpp \
	src/fs_mon.hpp \
	src/gx2_mon.cpp \
	src/gx2_mon.hpp \
	src/gx2_perf.h \
	src/main.cpp \
	src/net_mon.cpp \
	src/net_mon.hpp \
	src/out_span.cpp \
	src/out_span.hpp \
	src/overlay.cpp \
	src/overlay.hpp \
	src/pad_mon.cpp \
	src/pad_mon.hpp \
	src/time_mon.cpp \
	src/time_mon.hpp \
	src/utils.cpp \
	src/utils.hpp

EXTRA_papaya_hud_elf_DEPENDENCIES = \
	$(LIBMAPPEDMEMORY_LIB)


all-local: $(WPS_FILE)


.PHONY: run wiiu-install wiiu-uninstall company


run: $(WPS_FILE)
	WIILOAD=tcp:wiiu wiiload $<


wiiu-install: $(WPS_FILE)
	curl "ftp://wiiu:/fs/vol/external01/wiiu/environments/aroma/plugins/" --upload-file $<


wiiu-uninstall:
	curl "ftp://wiiu" --quote "DELE /fs/vol/external01/wiiu/environments/aroma/plugins/$(WPS_FILE)"


company: compile_flags.txt

compile_flags.txt: Makefile
	printf "%s" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > compile_flags.txt
	$(CPP) -xc++ /dev/null -E -Wp,-v 2>&1 | sed -n 's,^ ,-I,p' >> compile_flags.txt


CLEANFILES = $(WPS_FILE)
DISTCLEANFILES = $(AMINCLUDE)

@INC_AMINCLUDE@
