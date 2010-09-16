EXTRA_DIST = index.html \
x0.jpg x1.jpg x2.jpg x3.jpg x4.jpg x5.jpg x6.jpg x7.jpg  \
x8.jpg x9.jpg x10.jpg x11.jpg x12.jpg x13.jpg x14.jpg \
x15.jpg x16.jpg x17.jpg x18.jpg x19.jpg x20.jpg x21.jpg \
x22.jpg x23.jpg \
b0.jpg b1.jpg b2.jpg b3.jpg b4.jpg b5.jpg b6.jpg \
warp.jpg screen.jpg

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(doc_dir)/lbreakout2
	@for file in *.jpg; do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(doc_dir)/lbreakout2/$$file;\
	done
	$(INSTALL_DATA) index.html $(DESTDIR)$(doc_dir)/lbreakout2/index.html

win32-install-script:
	@for file in $(EXTRA_DIST); do \
	  echo "Source: \"`pwd -W`\\$$file\"; DestDir: \"{app}\\docs\"; CopyMode: alwaysoverwrite" >> @win32_inst_script@; \
        done
