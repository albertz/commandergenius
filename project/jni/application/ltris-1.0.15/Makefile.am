SUBDIRS = intl m4 po src icons

EXTRA_DIST = config.rpath  AUTHORS COPYING ChangeLog INSTALL \
README TODO touchall ABOUT-NLS po/tr.po_full po/pt.po_full \
po/nonascii.pl ltris32.gif ltris48.gif ltris.desktop.in

desktopdir = $(datadir)/applications
desktop_DATA = ltris.desktop
icondir = $(datadir)/icons
icon_DATA = ltris48.gif

AUTOMAKE_OPTIONS = foreign

# We can't use static desktop file because of icon location. Is this 
# the right way to handle it? At least it works... as long as $datadir
# does not contain plus signs. :-)
ltris.desktop:
	@sed -e "s+Icon=+Icon=$(datadir)/icons/ltris48.gif+" \
	< ltris.desktop.in > ltris.desktop

ACLOCAL_AMFLAGS = -I m4
