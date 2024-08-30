# src/Makefile.am

@INC_AMINCLUDE@


AM_CPPFLAGS = \
	$(DEVKITPRO_CPPFLAGS) \
	-I$(top_srcdir)/external/libwupsxx/include

AM_CXXFLAGS = \
	$(DEVKITPRO_CXXFLAGS) \
	-Wall -Wextra

AM_LDFLAGS = $(DEVKITPRO_LDFLAGS)

LIBS = \
	$(top_builddir)/external/libwupsxx/src/libwupsxx.a \
	$(DEVKITPRO_LIBS)


WPS_FILE = papaya-hud.wps
noinst_PROGRAMS = papaya-hud.elf

papaya_hud_elf_SOURCES = \
	cfg.cpp cfg.hpp \
	coreinit_allocator.h \
	cos_mon.cpp cos_mon.hpp \
	fs_mon.cpp fs_mon.hpp \
	gx2_mon.cpp gx2_mon.hpp \
	gx2_perf.h \
	logger.cpp logger.hpp \
	main.cpp \
	net_mon.cpp net_mon.hpp \
	nintendo_glyphs.h \
	overlay.cpp overlay.hpp \
	pad_mon.cpp pad_mon.hpp \
	time_mon.cpp time_mon.hpp \
	utils.cpp utils.hpp \
	wpad_status.h


all-local: $(WPS_FILE)

# $(WPS_FILE): papaya-hud.elf


install-exec-local: $(WPS_FILE)
	curl "ftp://wiiu:/fs/vol/external01/wiiu/environments/aroma/plugins/" --upload-file $(WPS_FILE)


uninstall-local:
	curl "ftp://wiiu" --quote "DELE /fs/vol/external01/wiiu/environments/aroma/plugins/$(WPS_FILE)"


run-local: all
	WIILOAD=tcp:wiiu wiiload $(WPS_FILE)


.PHONY: company
company: compile_flags.txt

compile_flags.txt: Makefile
	printf "%s" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt
	$(CPP) -xc++ /dev/null -E -Wp,-v 2>&1 | sed -n 's,^ ,-I,p' >> $(srcdir)/compile_flags.txt


CLEANFILES = $(WPS_FILE)

