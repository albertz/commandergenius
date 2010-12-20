EXTRA_DIST = N_Arenas N_Snapshots

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/levels
	@for file in $(EXTRA_DIST); do \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(inst_dir)/levels/$$file; \
	done

win32-install-script:
	@for file in $(EXTRA_DIST); do \
	  echo "Source: \"`pwd -W`\\$$file\"; DestDir: \"{app}\\levels\"; CopyMode: alwaysoverwrite" >> @win32_inst_script@; \
	done

