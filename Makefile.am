@INC_AMINCLUDE@


EXTRA_DIST = \
	assets \
	AUTHORS \
	bootstrap \
	COPYING \
	README.md


SUBDIRS = external/libwupsxx


AM_CPPFLAGS = -I$(srcdir)/external/libwupsxx/include

AM_CXXFLAGS = -Wall -Wextra -Werror

LDADD = external/libwupsxx/src/libwupsxx.a


WPS_FILE = papaya-hud.wps
noinst_PROGRAMS = papaya-hud.elf

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


all-local: $(WPS_FILE)

# $(WPS_FILE): papaya-hud.elf


install-exec-local: $(WPS_FILE)
	curl "ftp://wiiu:/fs/vol/external01/wiiu/environments/aroma/plugins/" --upload-file $(WPS_FILE)


uninstall-local:
	curl "ftp://wiiu" --quote "DELE /fs/vol/external01/wiiu/environments/aroma/plugins/$(WPS_FILE)"

.PHONY: run company

run: $(WPS_FILE)
	WIILOAD=tcp:wiiu wiiload $(WPS_FILE)


.PHONY: company
company: compile_flags.txt

compile_flags.txt: Makefile
	printf "%s" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt
	$(CPP) -xc++ /dev/null -E -Wp,-v 2>&1 | sed -n 's,^ ,-I,p' >> $(srcdir)/compile_flags.txt


CLEANFILES = $(WPS_FILE)
DISTCLEANFILES = $(AMINCLUDE)
