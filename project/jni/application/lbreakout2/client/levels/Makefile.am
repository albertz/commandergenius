# Use ls | grep -v Makefile | sed -e "s/$/ \\\\/" for update
EXTRA_DIST = 1010011010 \
Abc \
Afl \
Alex \
Alexandre \
AllNighter \
Arcade \
Arkanoid \
BeOS-4ever \
Bertrand \
BigBang \
Biolab428 \
Blues \
Bombs \
CalendarFun \
Celine \
Chaos \
Choices \
Circle \
Classique \
Color \
Composers \
Crossing \
D2K \
Daves \
Demons \
Family \
Fantaisie \
FatalDoomsday \
FatalDoomsday2 \
Flower \
Fly \
Freek \
FunCity \
Gatinha \
GreenWall \
GreenWall2 \
GreenWall3 \
GreenWall4 \
Hans \
HereWeGo \
Heroes \
HighBall \
Holidays \
Hommage \
Impossibility \
Invisible \
IoriYagami \
Iris \
IrrenhausAh \
Izusiowe \
JediAdventure \
Kazan-1 \
Kevin \
KungFu \
Lapis \
Lattsville \
LBreakout1 \
LBreakout2 \
LinuxFun \
Lord \
Maze \
Megadoomer \
MontyHall \
N_Arenas \
Nations \
Negundoid \
NES \
NewOERG1 \
N_Snapshots \
ObiWan \
OpenSource \
OurWorld \
Pabelo \
Patience \
Pawset \
Ph33r \
P.I.T.A. \
PlayAgain \
Popcorn \
RichQuick \
Runes \
R-World \
Sadistic \
Shimitar \
Silly \
SkillShot \
Smilies \
SnyMaxx \
StarWars \
Symmetry \
Symmetry-MKII \
T.C.S. \
TheEnd \
TheGauntlet \
TimeWaster \
TrickShot \
Tricky \
Twilight \
Vader \
Weirdness \
Wolvie \
WonderWalls \
WTF \
X \
Yexter \
Yoda \
Zigies \
Zooligan \
Zufallswelt

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/levels
	@for file in $(EXTRA_DIST); do \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(inst_dir)/levels/$$file; \
	done

win32-install-script:
	@for file in $(EXTRA_DIST); do \
	  echo "Source: \"`pwd -W`\\$$file\"; DestDir: \"{app}\\levels\"; CopyMode: alwaysoverwrite" >> @win32_inst_script@; \
	done

