DEFS = @DEFS@ @sound_flag@ @buf_flag@ @inst_flag@ @hi_inst_flag@ @dis_flag@ @locdir_flag@

bin_PROGRAMS = ltris
ltris_SOURCES = sdl.c menu.c item.c manager.c value.c main.c audio.c event.c \
list.c tools.c config.c chart.c tetris.c bowl.c shrapnells.c cpu.c hint.c \
parser.c
ltris_LDADD   = @mixer_flag@

EXTRA_DIST = sdl.c menu.c item.c manager.c value.c main.c audio.c event.c \
list.c tools.c config.c chart.c tetris.c bowl.c shrapnells.c cpu.c \
sdl.h menu.h item.h manager.h value.h audio.h event.h list.h tools.h \
config.h chart.h tetris.h bowl.h shrapnells.h cpu.h ltris.h \
hint.c hint.h gfx.S gfx.h parser.h gettext.h \
empty.hscr figures

SUBDIRS = gfx sounds

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)
	$(INSTALL_DATA) figures $(DESTDIR)$(inst_dir)/figures
	$(mkinstalldirs) $(DESTDIR)$(hi_dir)
	if ! test -f $(DESTDIR)$(hi_dir)/ltris.hscr; then \
	  $(INSTALL_DATA) --mode=666 empty.hscr $(DESTDIR)$(hi_dir)/ltris.hscr; \
	fi;

uninstall-local:
	-rm -fr $(DESTDIR)$(inst_dir)
	-rm -f  $(DESTDIR)$(hi_dir)/ltris.hscr
