EXTRA_DIST = back0.bmp back1.bmp back2.bmp back3.bmp back4.bmp back5.bmp \
f_small_white.bmp f_small_yellow.bmp f_white.bmp f_yellow.bmp f_tiny_black.bmp \
menuback.bmp quest.bmp blocks.bmp balloon.bmp balloon_peek.bmp logo.bmp

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/gfx
	@for file in *.bmp; do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(inst_dir)/gfx/$$file;\
	done
