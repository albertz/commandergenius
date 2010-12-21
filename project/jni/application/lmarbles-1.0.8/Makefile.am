SUBDIRS = src

EXTRA_DIST = lmarbles.spec AUTHORS COPYING ChangeLog INSTALL README \
README-SDL.txt TODO LMarbles.prj lmarbles32.gif lmarbles48.gif \
lmarbles.desktop.in

desktopdir = $(datadir)/applications
desktop_DATA = lmarbles.desktop
icondir = $(datadir)/icons
icon_DATA = lmarbles48.gif

AUTOMAKE_OPTIONS = foreign

WIN32_TEXT = AUTHORS COPYING ChangeLog INSTALL README README-SDL.txt TODO marbles.spec
WIN32_LOCAL_DLL = SDL.dll SDL_mixer.dll
WIN32_SYS_DLL = msvcrt.dll		  

lmarbles.desktop:
	@sed -e "s+Icon=+Icon=$(datadir)/icons/lmarbles48.gif+" \
	< lmarbles.desktop.in > lmarbles.desktop

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
	@echo "AppPublisherURL=http://lgames.sourceforge.net/index.php?project=@appname@" >> $@
	@echo "AppSupportURL=http://lgames.sourceforge.net/index.php?project=@appname@" >> $@
	@echo "AppUpdatesURL=http://lgames.sourceforge.net/index.php?project=@appname@" >> $@
	@echo "DefaultDirName={pf}\\$(PACKAGE)" >> $@
	@echo "DefaultGroupName=@appname@" >> $@
	@echo "OutputDir=." >> $@
	@echo "Uninstallable=yes" >> $@
	@echo "AlwaysCreateUninstallIcon=yes" >> $@
	@echo "AdminPrivilegesRequired=1" >> $@
	@echo "[Tasks]" >> $@
	@echo "Name: \"desktopicon\"; Description: \"Create a &desktop icon\"; GroupDescription: \"Additional icons:\"" >> $@
	@echo "[Files]" >> $@
	@(cd win32dlls; for file in $(WIN32_LOCAL_DLL); do \
	  echo "Source: \"`pwd -W`\\$$file\";   DestDir: \"{app}\"; CopyMode: alwaysoverwrite" >> $@; \
	done)
	@(cd win32dlls; for file in $(WIN32_SYS_DLL); do \
	  echo "Source: \"`pwd -W`\\$$file\";   DestDir: \"{sys}\"; CopyMode: alwaysskipifsameorolder; Flags: restartreplace uninsneveruninstall" >> $@; \
	done)
	@echo "Source: \"win32text\\*.*\";      DestDir: \"{app}\"; CopyMode: alwaysoverwrite" >> $@; 
	@list='$(SUBDIRS)'; for subdir in $$list; do \
	  (cd $$subdir  \
           && $(MAKE) $(AM_MAKEFLAGS) win32-install-script); \
        done
	@echo "[Icons]" >> $@
	@echo "Name: \"{group}\\@appname@\"; Filename: \"{app}\\$(PACKAGE)$(EXEEXT)\"; WorkingDir: \"{app}\"" >> $@
	@echo "Name: \"{group}\\@appname@ Manual\"; Filename: \"{app}\\manual\\manual.html\"" >> $@
	@echo "Name: \"{userdesktop}\\@appname@\"; Filename: \"{app}\\$(PACKAGE)$(EXEEXT)\"; WorkingDir: \"{app}\"; Tasks: \"desktopicon\"" >> $@
	@echo "[Run]" >> $@
	@echo "Filename: \"{app}\\$(PACKAGE)$(EXEEXT)\"; Description: \"Launch @appname@\"; Flags: nowait postinstall skipifsilent" >> $@

	@$(AWK) '{printf "%s\r\n",$$0}' $@ > marbles.iss.w32
	@mv marbles.iss.w32 $@
	
win32-installer: win32-installer-script
	$(ISCC) installer.iss 	
	@-rm -rf win32text
	@mv setup.exe $(PACKAGE)-$(VERSION)-win32.exe
	@echo Self installer created: $(PACKAGE)-$(VERSION)-win32.exe
