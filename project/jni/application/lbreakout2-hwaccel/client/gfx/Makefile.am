EXTRA_DIST = back0.png back1.png back2.png back3.png back4.png back5.png \
ball.png bricks.png extras.png \
f_small_red.png f_small_white.png f_small_yellow.png \
f_white.png f_yellow.png \
fr_left.png fr_right.png fr_top.png \
fr_mp_left.png fr_mp_right.png \
life.png menuback.png paddle.png shine.png shot.png weapon.png \
sel_frame.png buttons.png explosions.png explosions_dark.png \
f_tiny_black.png balloon.png balloon_peek.png \
f_frame.png f_game.png \
scroll_up.png scroll_down.png scroll_track.png \
fr_hori.png fr_vert.png fr_luc.png \
fr_llc.png fr_ruc.png fr_rlc.png confirm_buttons.png \
win_icon.png cr_back.png warp.png nukeback.png \
loading.png ammo.png

SUBDIRS = AbsoluteB Oz Moiree Classic

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/gfx
	@for file in $(EXTRA_DIST); do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(inst_dir)/gfx/$$file;\
	done

win32-install-script:
	@for file in $(EXTRA_DIST); do \
          echo "Source: \"`pwd -W`\\$$file\"; DestDir: \"{app}\\gfx\"; CopyMode: alwaysoverwrite" >> @win32_inst_script@; \
        done
	@list='$(SUBDIRS)'; for subdir in $$list; do \
        (cd $$subdir  \
            && $(MAKE) $(AM_MAKEFLAGS) win32-install-script); \
        done
