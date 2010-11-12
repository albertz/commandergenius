Free Heroes2 Engine (is not playable version.)

Prerequisites:

You need to have these libraries (with equivalent devel versions) to build fHeroes2:

- SDL

optional library:
- SDL_mixer (play music: internal midi or external ogg tracks) or build WITHOUT_MIXER
- SDL_image (loading external sprites, create screenshot in png format) or build WITHOUT_IMAGE
- SDL_ttf (unicode support) or build WITHOUT_UNICODE
- SDL_net or build WITHOUT_NETWORK
- libogg
- libpng
- gettext

SDL libraries can be found at http://www.libsdl.org .
Sourcecode you can get it here: http://sourceforge.net/projects/fheroes2/
And translations: http://translations.launchpad.net/fheroes2

Copy origin data/*.agg in to data directory.
Copy maps files (*.mp2) in to maps directory.

Hot keys:
F4		- switch to fullscreen
PrintScreen	- create screenshot
Up		- move hero top
Down		- move hero bottom
Left		- move hero left
Right		- move hero right
Ctrl + Up	- scroll map top    (alt. key ';')
Ctrl + Down 	- scroll map bottom (alt. key '/')
Ctrl + Left	- scroll map left   (alt. key ',')
Ctrl + Right	- scroll map right  (alt. key '.')
e		- end turn
t		- next town
h		- next hero
m		- move hero
s		- save game to fheroes2.sav
l		- load game
i		- game info
p		- puzzle dialog
d		- digging artifact for current hero
space or a	- default action
return or n	- open dialog
o		- system dialog

1		- show/hide control panel
2		- show/hide radar (only for option: hide interface = on)
3		- show/hide buttons (only for option: hide interface = on)
4		- show/hide status window (only for option: hide interface = on)
5		- show/hide hero/town icons (only for option: hide interface = on)

Main menu:
n		- new game
s		- standard game
c		- campaign game
m		- multi-player game

Battle:
Esc		- fast retreat
space		- skip turn (soft: heroes3 version)
s		- skip turn (hard)
o		- options dialog
a		- set auto battle
c		- show spell book
h		- show heroes dialog
