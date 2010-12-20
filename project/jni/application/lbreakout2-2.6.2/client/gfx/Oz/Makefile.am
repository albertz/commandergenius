EXTRA_DIST = back0.png back1.png back2.png back3.png back4.png \
bricks.png fr_left.png fr_right.png fr_top.png \
life.png paddle.png \
README

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/gfx/Oz
	@for file in $(EXTRA_DIST); do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(inst_dir)/gfx/Oz/$$file;\
	done

win32-install-script:
	@for file in $(EXTRA_DIST); do \
          echo "Source: \"`pwd -W`\\$$file\"; DestDir: \"{app}\\gfx\\Oz\"; CopyMode: alwaysoverwrite" >> @win32_inst_script@; \
        done
