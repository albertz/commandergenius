EXTRA_DIST = box_wallpaper.png widget_wallpaper.png \
box_frame.png widget_frame.png \
button_frame_normal.png button_frame_clicked.png \
button_wallpaper_normal.png button_wallpaper_focused.png \
scrollbar_arrows.png checkbox.png progress_high.png progress_medium.png \
progress_low.png list_item_color.png font.png spinbutton_arrows.png \
edit.wav click.wav

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/gui_theme
	@for file in $(EXTRA_DIST); do \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(inst_dir)/gui_theme/$$file; \
	done

win32-install-script:
	@for file in $(EXTRA_DIST); do \
	  echo "Source: \"`pwd -W`\\$$file\"; DestDir: \"{app}\\gui_theme\"; CopyMode: alwaysoverwrite" >> @win32_inst_script@; \
	done

