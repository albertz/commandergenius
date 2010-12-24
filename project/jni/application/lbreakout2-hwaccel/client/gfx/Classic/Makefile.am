EXTRA_DIST = ABOUT

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/gfx/Classic
	@for file in $(EXTRA_DIST); do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(inst_dir)/gfx/Classic/$$file;\
	done

win32-install-script:
	@for file in $(EXTRA_DIST); do \
          echo "Source: \"`pwd -W`\\$$file\"; DestDir: \"{app}\\gfx\\Classic\"; CopyMode: alwaysoverwrite" >> @win32_inst_script@; \
        done
