#!/bin/sh

#if [ -z "$1" ]; then echo "Record user touch/key input, and replay it back via script" ; echo "Usage: $0 ScriptName.sh" ; exit ; fi

echo "#!/bin/sh"
echo "# Record user touch/key input, and replay it back via script"
echo "# Usage: $0 | tee ScriptName.sh ; chmod a+x ScriptName.sh ; ./ScriptName.sh"
echo "# Press Ctrl-C when done"

#convert_input() {
#awk '{ if (($1 != "") && (substr($0,1,1)=="/")){ sub(":","",$1); printf("adb shell sendevent %s %d %d %d\n",$1,strtonum("0x"$2),strtonum("0x"$3),strtonum("0x"$4)); } }'
#}

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
