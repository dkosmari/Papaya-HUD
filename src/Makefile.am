# src/Makefile.am

SUBDIRS = wupsxx

AM_CPPFLAGS = $(DEVKITPRO_CPPFLAGS)

AM_CXXFLAGS = \
	$(DEVKITPRO_CXXFLAGS) \
	-Wall -Wextra

AM_LDFLAGS = $(DEVKITPRO_LDFLAGS)

LIBS = $(DEVKITPRO_LIBS)


WPS_FILE = papaya-hud.wps
noinst_PROGRAMS = papaya-hud.elf

papaya_hud_elf_SOURCES = \
	cfg.cpp cfg.hpp \
	cpu_mon.cpp cpu_mon.hpp \
	gx2_mon.cpp gx2_mon.hpp \
	logging.cpp logging.hpp \
	main.cpp \
	net_mon.cpp net_mon.hpp \
	nintendo_glyphs.h \
	overlay.cpp overlay.hpp

papaya_hud_elf_LDADD = wupsxx/libwupsxx.a


all-local: $(WPS_FILE)


install-exec-local: $(WPS_FILE)
	curl "ftp://wiiu:/fs/vol/external01/wiiu/environments/aroma/plugins/" --upload-file $(WPS_FILE)

uninstall-local:
	curl "ftp://wiiu" --quote "DELE /fs/vol/external01/wiiu/environments/aroma/plugins/$(WPS_FILE)"


.PHONY: company
company: compile_flags.txt
compile_flags.txt: Makefile
	printf "%s" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt



@INC_AMINCLUDE@

