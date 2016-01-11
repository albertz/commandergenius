#!/bin/sh

if [ -e supertux/patched.successfully ]; then
	exit 0
else
	git clone --depth 1 --branch release/0.4 https://github.com/SuperTux/supertux.git || exit 1
	git -C supertux submodule update --init || exit 1
	patch -p1 -d supertux < android.diff && touch supertux/patched.successfully || exit 1
	ln -sf supertux/data/images/engine/icons/supertux-256x256.png icon.png
	ln -sf ../supertux/data/images/engine/menu/logo.png AndroidData/logo.png
fi
