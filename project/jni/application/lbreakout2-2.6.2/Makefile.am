SUBDIRS = intl m4 po common gui game server client docs

EXTRA_DIST = config.rpath AUTHORS COPYING ChangeLog INSTALL README \
README-SDL.txt TODO lbreakout2.spec touchall lbreakout32.gif \
lbreakout48.gif lbreakout2.desktop.in win32_mancomp win32_preparchive

desktopdir = $(datadir)/applications
desktop_DATA = lbreakout2.desktop
icondir = $(datadir)/icons
icon_DATA = lbreakout48.gif

WIN32_TEXT = AUTHORS COPYING ChangeLog INSTALL README README-SDL.txt \
TODO lbreakout2.spec ABOUT-NLS
WIN32_LOCAL_DLL = SDL.dll SDL_mixer.dll zlib.dll libpng.dll
WIN32_SYS_DLL = msvcrt.dll		  

AUTOMAKE_OPTIONS = foreign

# We can't use static desktop file because of icon location. Is this 
# the right way to handle it? At least it works... as long as $datadir
# does not contain plus signs. :-)
lbreakout2.desktop:
	@sed -e "s+Icon=+Icon=$(datadir)/icons/lbreakout48.gif+" \
	< lbreakout2.desktop.in > lbreakout2.desktop

win32text: $(WIN32_TEXT)
	@echo Converting text files for win32 installer
	@-rm -rf ./win32text
	@mkdir ./win32text
	@for textfile in $(WIN32_TEXT); do \
	  $(AWK) '{printf "%s\r\n",$$0}' $$textfile > ./win32text/$$textfile ; \
	done

win32-installer-script: @win32_inst_script@

@win32_inst_script@: win32text
	@echo Generating win32 installer script
	@echo "[Setup]" > $@
	@echo "AppName=@appname@" >> $@
	@echo "AppVerName=@appname@ $(VERSION)" >> $@
	@echo "AppPublisher=Michael Speck" >> $@
	@echo "AppPublisherURL=http://lgames.sourceforge.net/index.php?project=LBreakout2" >> $@
	@echo "AppSupportURL=http://lgames.sourceforge.net/index.php?project=LBreakout2" >> $@
	@echo "AppUpdatesURL=http://lgames.sourceforge.net/index.php?project=LBreakout2" >> $@
	@echo "DefaultDirName={pf}\\$(PACKAGE)" >> $@
	@echo "DefaultGroupName=@appname@" >> $@
	@echo "OutputDir=." >> $@
	@echo "Uninstallable=yes" >> $@
	@echo "AlwaysCreateUninstallIcon=yes" >> $@
	@echo "AdminPrivilegesRequired=1" >> $@
	@echo "[Tasks]" >> $@
	@echo "Name: \"desktopicon\"; Description: \"Create a &desktop icon\"; GroupDescription: \"Additional icons:\"" >> $@
	@echo "[Files]" >> $@
	@(cd win32dlls; for file in $(WIN32_LOCAL_DLL); do echo "Source: \"`pwd -W`\\$$file\";   DestDir: \"{app}\"; CopyMode: alwaysoverwrite" >> $@; done)
	@(cd win32dlls; for file in $(WIN32_SYS_DLL); do echo "Source: \"`pwd -W`\\$$file\";   DestDir: \"{sys}\"; CopyMode: alwaysskipifsameorolder; Flags: restartreplace uninsneveruninstall" >> $@; done)
	@echo "Source: \"win32text\\*.*\";      DestDir: \"{app}\"; CopyMode: alwaysoverwrite" >> $@; 
	@list='$(SUBDIRS)'; for subdir in $$list; do (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) win32-install-script); done
	@echo "[Icons]" >> $@
	@echo "Name: \"{group}\\@appname@\"; Filename: \"{app}\\$(PACKAGE)$(EXEEXT)\"; WorkingDir: \"{app}\"" >> $@
	@echo "Name: \"{group}\\@appname@ Manual\"; Filename: \"{app}\\docs\\index.html\"" >> $@
	@echo "Name: \"{userdesktop}\\@appname@\"; Filename: \"{app}\\$(PACKAGE)$(EXEEXT)\"; WorkingDir: \"{app}\"; Tasks: \"desktopicon\"" >> $@
	@echo "[Run]" >> $@
	@echo "Filename: \"{app}\\$(PACKAGE)$(EXEEXT)\"; Description: \"Launch @appname@\"; Flags: nowait postinstall skipifsilent" >> $@

	@$(AWK) '{printf "%s\r\n",$$0}' $@ > installer.iss.w32
	@mv installer.iss.w32 $@
	
win32-installer: win32-installer-script
	$(ISCC) @win32_inst_script@ 	
	@mv setup.exe $(PACKAGE)-$(VERSION)-win32.exe
	@-rm -rf win32text
	@echo "Installer created: $(PACKAGE)-$(VERSION)-win32.exe"

ACLOCAL_AMFLAGS = -I m4
