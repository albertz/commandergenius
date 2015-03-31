#!/bin/sh

echo "#!/bin/sh"
echo "# Record user touch/key input, and replay it back via script"
echo "# Usage: $0 | tee ScriptName.sh ; chmod a+x ScriptName.sh ; ./ScriptName.sh"
echo "# Press Ctrl-C when done"

CURTIME=0
export CURTIME

adb shell getevent | while read DEV N1 N2 N3; do
	if echo "$DEV" | grep "^[/]" >/dev/null ; then
		CURTIME1=`date +%s`
		if [ "$CURTIME1" '!=' "$CURTIME" ] ; then [ "$CURTIME" = 0 ] || echo "sleep `expr $CURTIME1 - $CURTIME`" ; CURTIME=$CURTIME1 ; export CURTIME ; fi
		DEV=`echo "$DEV" | sed 's/\(.*\)./\1/'`
		N1=`printf '%d' 0x$N1`
		N2=`printf '%d' 0x$N2`
		N3=`echo $N3 | grep -o "[0-9a-fA-F]*"`
		N3=`printf '%d' 0x$N3`
		echo "adb shell sendevent $DEV $N1 $N2 $N3"
	fi
done
