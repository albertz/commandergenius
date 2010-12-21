Name: lmarbles
Version: 1.0.6
Release: 1cl
Summary: Atomix-like game of moving marbles in puzzles
Summary(pt_BR): Jogo tipo Atomix, de mover bolas de gude em labirintos
Summary(es): Atomix-like game of moving marbles in puzzles
Group: Games
Group(pt_BR): Jogos
Group(es): Juegos
License: GPL
URL: http://lgames.sourceforge.net/%{name}/%{name}.html
Source: http://ftp1.sourceforge.net/lgames/%{name}-%{version}.tar.gz
Patch: %{name}-prefsdir.patch
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Marbles is very similiar to Atomix and was heavily inspired by it.
Goal is to create a more or less complex figure out of single marbles
within a time limit to reach the next level.

Sounds easy? Well, there is a problem: If a marble starts to move it
will not stop until it hits a wall or marble. And to make it even more
interesting there are obstacles like arrows, crumbling walls and
teleports!

%description -l pt_BR
O Marbles é muito parecido com o jogo Atomix, pois foi inspirado nele.
O objetivo é criar uma figura de bolas de gude mais ou menos complexa
dentro de um limite de tempo.

Parece fácil? Bem, há um problema: quando uma bolinha de gude começa a
se mover, só pára quando bate em uma parede ou outra bolinha. E para
ficar mais interessante, há alguns obstáculos como caminhos de mão
única, paredes que desmoronam e teletransporte!

%description -l es
Marbles is very similiar to Atomix and was heavily inspired by it.
Goal is to create a more or less complex figure out of single marbles
within a time limit to reach the next level.

Sounds easy? Well, there is a problem: If a marble starts to move it
will not stop until it hits a wall or marble. And to make it even more
interesting there are obstacles like arrows, crumbling walls and
teleports!

%prep
%setup -q
%patch

%build
%configure --datadir=%{_datadir}
make

%install
mkdir -p %{buildroot}/%{_localstatedir}/games
%makeinstall DESTDIR=%{buildroot} bindir=%{_prefix}/games

%clean
rm -rf %{buildroot}

%files
%defattr(0644,root,root,0755)
%doc AUTHORS COPYING README ChangeLog %{name}/manual
%attr(2755,root,games) %{_prefix}/games/%{name}
%{_datadir}/games/%{name}
%attr(0664,root,games) %config %{_localstatedir}/games/%{name}.prfs

%changelog
* Thu Nov 30 2000 aurélio marinho jargas <aurelio@conectiva.com>
+ marbles-001126-1cl
- changed prefs dir
- adopted
