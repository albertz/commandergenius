DEFS = @network_flag@ @sdlnet_flag@ @arch_flag@ @warp_flag@ @audio_flag@ @inst_flag@ @hi_inst_flag@ @locdir_flag@ @DEFS@

lbreakout2res.o: win32.ico 
	@echo "1 VERSIONINFO" > $(PACKAGE).rc
	@echo "FILEVERSION `echo $(VERSION) | $(AWK) 'ORS=""; { subs = gsub(/\./,","); print $0; for (ndx=subs; ndx < 3; ndx++) print ",0"; }'`" >> $(PACKAGE).rc
	@echo "PRODUCTVERSION `echo $(VERSION) | $(AWK) 'ORS=""; { subs = gsub(/\./,","); print $0; for (ndx=subs; ndx < 3; ndx++) print ",0"; }'`" >> $(PACKAGE).rc
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

bin_PROGRAMS = lbreakout2
lbreakout2_SOURCES = credit.c shine.c extras.c balls.c shrapnells.c  \
shots.c event.c paddle.c frame.c misc.c bricks.c player.c \
file.c config.c \
item.c menu.c manager.c value.c chart.c \
editor.c help.c hint.c theme.c client.c client_recv.c \
client_data.c client_handlers.c game.c comm.c slot.c \
display.c main.c win32.ico
lbreakout2_DEPENDENCIES = @win32_deps@
lbreakout2_LDADD = @win32_deps@ ../common/libcommon.a ../gui/libGui.a ../game/libgame.a @mixer_flag@ @sdlnet_lib_flag@

EXTRA_DIST = credit.h shine.h extras.h balls.h shrapnells.h  \
shots.h event.h paddle.h frame.h misc.h bricks.h player.h \
file.h config.h lbreakout.h \
item.h menu.h manager.h value.h chart.h \
editor.h help.h hint.h theme.h client.h client_data.h \
client_handlers.h game.h comm.h slot.h \
display.h empty.hscr

SUBDIRS = gfx sounds levels gui_theme

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)
	$(mkinstalldirs) $(DESTDIR)$(hi_dir)
	if ! test -f $(DESTDIR)$(hi_dir)/lbreakout2.hscr; then \
	  $(INSTALL_DATA) -m 666 empty.hscr $(DESTDIR)$(hi_dir)/lbreakout2.hscr; \
	fi;

uninstall-local:
	rm -rf $(DESTDIR)$(inst_dir)
	rm -f $(DESTDIR)$(hi_dir)/lbreakout2.hscr
	rm -rf $(DESTDIR)$(doc_dir)/lbreakout2

win32-install-script:
	@$(STRIP) $(PACKAGE)$(EXEEXT)
	@echo "Source: \"`pwd -W`\\$(PACKAGE)$(EXEEXT)\"; DestDir: \"{app}\"; CopyMode: alwaysoverwrite" >> @win32_inst_script@
	@list='$(SUBDIRS)'; for subdir in $$list; do \
	  (cd $$subdir  \
          && $(MAKE) $(AM_MAKEFLAGS) win32-install-script); \
        done
