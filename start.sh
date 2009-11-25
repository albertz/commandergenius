#!/bin/sh

get_backtrace() {
    local exe=$1
    local core=$2

	echo "ERROR detected, printing core-info ..."

    gdb ${exe} \
        --core ${core} \
        --batch \
        --quiet \
		-ex "set width 0" \
		-ex "set height 0" \
        -ex "thread apply all bt full" \
        -ex "quit"

	echo "HINT: Please send the above output to openlierox@az2000.de."
}

cd data
ulimit -c unlimited		# activate core-files
rm core* 2>/dev/null	# remove old core-files

bin="/dev/null"
[ -x ../$bin ] || bin="build/Xcode/build/Debug/Commander Genius.app/Contents/MacOS/Commander Genius"
[ -x ../$bin ] || bin=build/Xcode/build/Release/Commander Genius.app/Contents/MacOS/Commander Genius
[ -x ../$bin ] || bin=bin/commandergenius
../$bin "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9"

mv core* .. 2>/dev/null

