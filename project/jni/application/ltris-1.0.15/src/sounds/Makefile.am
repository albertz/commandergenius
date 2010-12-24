EXTRA_DIST = motion.wav stop.wav click.wav leftright.wav explosion.wav \
excellent.wav nextlevel.wav

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/sounds
	@for file in *.wav; do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(inst_dir)/sounds/$$file;\
	done
