DEFS = @DEFS@ @arch_flag@ @inst_flag@ @prf_flag@ @sound_flag@ @ascii_flag@

marblesres.o: win32.ico 
	@echo "1 VERSIONINFO" > $(PACKAGE).rc
	@echo "FILEVERSION `echo $(VERSION) | $(AWK) '{ gsub(/\./,","); print }'`,0" >> $(PACKAGE).rc
	@echo "PRODUCTVERSION `echo $(VERSION) | $(AWK) '{ gsub(/\./,","); print }'`,0" >> $(PACKAGE).rc
	@echo "FILEFLAGSMASK 0" >> $(PACKAGE).rc
	@echo "FILEOS 0x40000" >> $(PACKAGE).rc
	@echo "FILETYPE 1" >> $(PACKAGE).rc
	@echo "{" >> $(PACKAGE).rc
	@echo "  BLOCK \"StringFileInfo\"" >> $(PACKAGE).rc
	@echo "  {" >> $(PACKAGE).rc
	@echo "    BLOCK \"040904E4\"" >> $(PACKAGE).rc
	@echo "    {" >> $(PACKAGE).rc
	@echo "      VALUE \"CompanyName\", \"LGames - Michael Speck\"" >> $(PACKAGE).rc
	@echo "      VALUE \"FileDescription\", \"@appname@\"" >> $(PACKAGE).rc
	@echo "      VALUE \"FileVersion\", \"$(VERSION)\"" >> $(PACKAGE).rc
	@echo "      VALUE \"InternalName\", \"$(PACKAGE)\"" >> $(PACKAGE).rc
	@echo "      VALUE \"LegalCopyright\", \"GNU Public License\"" >> $(PACKAGE).rc
	@echo "      VALUE \"OriginalFilename\", \"$(PACKAGE)$(EXEEXT)\"" >> $(PACKAGE).rc
	@echo "      VALUE \"ProductName\", \"@appname@\"" >> $(PACKAGE).rc
	@echo "      VALUE \"ProductVersion\", \"$(VERSION)\"" >> $(PACKAGE).rc
	@echo "    }" >> $(PACKAGE).rc
	@echo "  }" >> $(PACKAGE).rc
	@echo "}" >> $(PACKAGE).rc
	@echo "0 ICON win32.ico" >> $(PACKAGE).rc
	@windres -o $@ -i $(PACKAGE).rc

bin_PROGRAMS = lmarbles

lmarbles_SOURCES = audio.c cfg.c dynlist.c file.c game.c levels.c main.c menu.c \
profile.c sdl.c timer.c audio.h cfg.h dynlist.h file.h game.h levels.h \
menu.h profile.h sdl.h timer.h win32.ico lmarbles.6

lmarbles_DEPENDENCIES = @win32_deps@
lmarbles_LDADD = @win32_deps@ @mixer_flag@
man_MANS = lmarbles.6

DATA_FILES = \
gfx/board.bmp gfx/f_red.bmp gfx/f_small.bmp gfx/f_white.bmp \
gfx/f_yellow.bmp gfx/figure.bmp gfx/lights.bmp gfx/logo.bmp \
gfx/m_frame.bmp gfx/m_sframe.bmp gfx/marbles.bmp gfx/tiny.sdlfnt  \
gfx/title.bmp  gfx/win_icon.bmp \
\
sounds/alarm.wav sounds/arrow.wav sounds/click.wav sounds/explode.wav \
sounds/select.wav sounds/stop.wav sounds/teleport.wav sounds/score.wav \
\
levels/Original \
\
gfx/stone/arrow_d.bmp gfx/stone/arrow_l.bmp gfx/stone/arrow_r.bmp \
gfx/stone/arrow_u.bmp gfx/stone/background.bmp gfx/stone/barrier_lr.bmp \
gfx/stone/barrier_ud.bmp gfx/stone/floor.bmp gfx/stone/teleport0.bmp \
gfx/stone/teleport1.bmp gfx/stone/teleport2.bmp \
gfx/stone/teleport3.bmp gfx/stone/wall.bmp \
gfx/stone/wall_crumble.bmp \
\
gfx/metal/arrow_d.bmp gfx/metal/arrow_l.bmp gfx/metal/arrow_r.bmp \
gfx/metal/arrow_u.bmp gfx/metal/background.bmp gfx/metal/barrier_lr.bmp \
gfx/metal/barrier_ud.bmp gfx/metal/floor.bmp gfx/metal/teleport0.bmp \
gfx/metal/teleport1.bmp gfx/metal/teleport2.bmp \
gfx/metal/teleport3.bmp gfx/metal/wall.bmp \
gfx/metal/wall_crumble.bmp \
\
gfx/wood/arrow_d.bmp gfx/wood/arrow_l.bmp gfx/wood/arrow_r.bmp \
gfx/wood/arrow_u.bmp gfx/wood/background.bmp gfx/wood/barrier_lr.bmp \
gfx/wood/barrier_ud.bmp gfx/wood/floor.bmp gfx/wood/teleport0.bmp \
gfx/wood/teleport1.bmp gfx/wood/teleport2.bmp \
gfx/wood/teleport3.bmp gfx/wood/wall.bmp \
gfx/wood/wall_crumble.bmp 

DOC_FILES = manual/manual.html manual/arrow.jpg manual/arrow2.jpg \
manual/crumble.jpg manual/teleport.jpg manual/marbles.jpg 

EXTRA_DIST= empty.prfs $(DATA_FILES) $(DOC_FILES)

install-data-local:
#directories
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/gfx
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/gfx/stone
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/gfx/metal
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/gfx/wood
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/sounds
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/levels
#data files
	for file in $(DATA_FILES); do\
	   $(INSTALL_DATA) $$file $(DESTDIR)$(inst_dir)/$$file;\
	done
#profiles
	$(mkinstalldirs) $(DESTDIR)$(prf_dir)
	if ! test -f $(DESTDIR)$(prf_dir)/lmarbles.prfs; then \
	$(INSTALL_DATA) -m 666 empty.prfs $(DESTDIR)$(prf_dir)/lmarbles.prfs; \
	fi;

uninstall-local:
	rm -rf $(DESTDIR)$(inst_dir)
	rm -f $(DESTDIR)$(prf_dir)/lmarbles.prfs

win32rootdir=`pwd -W | awk '{ gsub("/","\\\\"); print}'`

win32-install-script:
	@$(STRIP) $(PACKAGE)$(EXEEXT)
	@echo "Source: \"$(win32rootdir)\\$(PACKAGE)$(EXEEXT)\"; DestDir: \"{app}\"; CopyMode: alwaysoverwrite" >> @win32_inst_script@
	@for file in $(DATA_FILES) $(DOC_FILES); do \
		win32src=$(win32rootdir)\\`echo $$file | awk '{ gsub("/","\\\\"); print }'`; \
		win32dest=`dirname $$file | awk '{ gsub("/","\\\\"); print }'`; \
		echo "Source: \"$$win32src\"; DestDir: \"{app}\\$$win32dest\"; CopyMode: alwaysoverwrite" >> @win32_inst_script@; \
        done
	
	@list='$(SUBDIRS)'; for subdir in $$list; do \
	  (cd $$subdir  \
          && $(MAKE) $(AM_MAKEFLAGS) win32-install-script); \
	done
