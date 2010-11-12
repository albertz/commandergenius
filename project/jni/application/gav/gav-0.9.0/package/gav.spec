Summary: GPL rendition of old Arcade Volleyball game
Name: gav
Version: 0.8.0
Release: 1
URL: gav.sourceforge.net
Source0: %{name}-%{version}.tar.gz
License: GPL
Group: X11/Games/Video
BuildRoot: %{_tmppath}/%{name}-root

%description
An SDL-based rendition of an old favorite CGA game featuring
two characters playing a volleyball-like game. This "revamped"
version is supposed to support theming, multiplayer games,
different input devices and networking.

%prep
%setup -q

%build
make depend
make

%install
install -d $RPM_BUILD_ROOT/usr/bin
install gav $RPM_BUILD_ROOT/usr/bin/
install -d $RPM_BUILD_ROOT/usr/share/games/gav/themes
install -d $RPM_BUILD_ROOT/usr/share/games/gav/sounds
install -d $RPM_BUILD_ROOT/usr/share/doc/gav-%{version}
install -d $RPM_BUILD_ROOT/usr/share/applications
install -d $RPM_BUILD_ROOT/usr/share/pixmaps
install -d $RPM_BUILD_ROOT/etc/X11/applnk/Games
install package/gav.desktop $RPM_BUILD_ROOT/usr/share/applications
install package/gav.desktop $RPM_BUILD_ROOT/etc/X11/applnk/Games
install package/gav.png $RPM_BUILD_ROOT/usr/share/pixmaps
cp -r themes/classic $RPM_BUILD_ROOT/usr/share/games/gav/themes
cp -r sounds $RPM_BUILD_ROOT/usr/share/games/gav/
install README $RPM_BUILD_ROOT/usr/share/doc/gav-%{version}/
install CHANGELOG $RPM_BUILD_ROOT/usr/share/doc/gav-%{version}/
install LICENSE $RPM_BUILD_ROOT/usr/share/doc/gav-%{version}/
#rm -rf $RPM_BUILD_ROOT

%clean
make clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/bin/gav
/usr/share/games/gav/
/usr/share/doc/gav-%{version}/README
/usr/share/doc/gav-%{version}/CHANGELOG
/usr/share/doc/gav-%{version}/LICENSE
/usr/share/applications/gav.desktop
/etc/X11/applnk/Games/gav.desktop
/usr/share/pixmaps/gav.png
/usr/share/games/gav/sounds
/usr/share/games/gav/sounds/score.wav
/usr/share/games/gav/sounds/servicechange.wav
/usr/share/games/gav/themes
/usr/share/games/gav/themes/classic
/usr/share/games/gav/themes/classic/Font.png
/usr/share/games/gav/themes/classic/FontInv.png
/usr/share/games/gav/themes/classic/background.png
/usr/share/games/gav/themes/classic/background_big.png
/usr/share/games/gav/themes/classic/ball.png
/usr/share/games/gav/themes/classic/plfl.png
/usr/share/games/gav/themes/classic/plfr.png
/usr/share/games/gav/themes/classic/plml.png
/usr/share/games/gav/themes/classic/plmr.png

%changelog
* Fri Apr 2 2004 Alessandro Tommasi <ale@ctrl-z-bg.org>
- Major Changes


