# use rpmbuild with --target noarch  option

Summary: GPL rendition of old Arcade Volleyball game
Name: gav-themes
Version: 0.7.3
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

This package contains additional themes.

%prep
%setup -q

%build

%install
install -d $RPM_BUILD_ROOT/usr/share/games/gav/themes
cp -r themes/inverted $RPM_BUILD_ROOT/usr/share/games/gav/themes
cp -r themes/fabeach $RPM_BUILD_ROOT/usr/share/games/gav/themes
cp -r themes/unnamed $RPM_BUILD_ROOT/usr/share/games/gav/themes
cp -r themes/yisus $RPM_BUILD_ROOT/usr/share/games/gav/themes
cp -r themes/yisus2 $RPM_BUILD_ROOT/usr/share/games/gav/themes
cp -r themes/naive $RPM_BUILD_ROOT/usr/share/games/gav/themes

%clean

%files
%defattr(-,root,root)
/usr/share/games/gav/themes/inverted
/usr/share/games/gav/themes/inverted/Font.png
/usr/share/games/gav/themes/inverted/FontInv.png
/usr/share/games/gav/themes/inverted/background.png
/usr/share/games/gav/themes/inverted/background_big.png
/usr/share/games/gav/themes/inverted/ball.png
/usr/share/games/gav/themes/inverted/plfl.png
/usr/share/games/gav/themes/inverted/plfr.png
/usr/share/games/gav/themes/inverted/plml.png
/usr/share/games/gav/themes/inverted/plmr.png
/usr/share/games/gav/themes/fabeach
/usr/share/games/gav/themes/fabeach/Font.png
/usr/share/games/gav/themes/fabeach/FontInv.png
/usr/share/games/gav/themes/fabeach/background.png
/usr/share/games/gav/themes/fabeach/background_big.png
/usr/share/games/gav/themes/fabeach/ball.png
/usr/share/games/gav/themes/fabeach/plfl.png
/usr/share/games/gav/themes/fabeach/plfr.png
/usr/share/games/gav/themes/fabeach/plml.png
/usr/share/games/gav/themes/fabeach/plmr.png
/usr/share/games/gav/themes/unnamed
/usr/share/games/gav/themes/unnamed/Font.png
/usr/share/games/gav/themes/unnamed/FontInv.png
/usr/share/games/gav/themes/unnamed/background.jpg
/usr/share/games/gav/themes/unnamed/background_big.jpg
/usr/share/games/gav/themes/unnamed/ball.png
/usr/share/games/gav/themes/unnamed/plfl.png
/usr/share/games/gav/themes/unnamed/plfr.png
/usr/share/games/gav/themes/unnamed/plml.png
/usr/share/games/gav/themes/unnamed/plmr.png
/usr/share/games/gav/themes/yisus
/usr/share/games/gav/themes/yisus/Font.png
/usr/share/games/gav/themes/yisus/FontInv.png
/usr/share/games/gav/themes/yisus/background.jpg
/usr/share/games/gav/themes/yisus/background_big.jpg
/usr/share/games/gav/themes/yisus/ball.png
/usr/share/games/gav/themes/yisus/plfl.png
/usr/share/games/gav/themes/yisus/plfr.png
/usr/share/games/gav/themes/yisus/plml.png
/usr/share/games/gav/themes/yisus/plmr.png
/usr/share/games/gav/themes/yisus2
/usr/share/games/gav/themes/yisus2/Font.png
/usr/share/games/gav/themes/yisus2/FontInv.png
/usr/share/games/gav/themes/yisus2/background.jpg
/usr/share/games/gav/themes/yisus2/background_big.jpg
/usr/share/games/gav/themes/yisus2/ball.png
/usr/share/games/gav/themes/yisus2/plfl.png
/usr/share/games/gav/themes/yisus2/plfr.png
/usr/share/games/gav/themes/yisus2/plml.png
/usr/share/games/gav/themes/yisus2/plmr.png
/usr/share/games/gav/themes/naive
/usr/share/games/gav/themes/naive/Font.png
/usr/share/games/gav/themes/naive/FontInv.png
/usr/share/games/gav/themes/naive/background.png
/usr/share/games/gav/themes/naive/background_big.png
/usr/share/games/gav/themes/naive/ball.png
/usr/share/games/gav/themes/naive/plfl.png
/usr/share/games/gav/themes/naive/plfr.png
/usr/share/games/gav/themes/naive/plml.png
/usr/share/games/gav/themes/naive/plmr.png
/usr/share/games/gav/themes/naive/theme.conf

%changelog
* Tue Feb 24 2002 Alessandro Tommasi <tommasi@di.unipi.it>
- New themes


