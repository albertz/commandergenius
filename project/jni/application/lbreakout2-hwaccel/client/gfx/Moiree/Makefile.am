EXTRA_DIST = back0.png back1.png back2.png back3.png back4.png back5.png \
bricks.png extras.png fr_left.png fr_right.png fr_top.png \
fr_mp_left.png fr_mp_right.png \
life.png paddle.png \
ABOUT

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/gfx/Moiree
	@for file in $(EXTRA_DIST); do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(inst_dir)/gfx/Moiree/$$file;\
	done

win32-install-script:
	@for file in $(EXTRA_DIST); do \
          echo "Source: \"`pwd -W`\\$$file\"; DestDir: \"{app}\\gfx\\Moiree\"; CopyMode: alwaysoverwrite" >> @win32_inst_script@; \
        done
