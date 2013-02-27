#!/bin/sh

# Date format is free-form
DATE="This Friday 9PM +EST"
# Event message
MESSAGE="^2Friday ^1Frag ^2Evening ^7on ^3CyberXZT server\n^6"
# Message when no event is happening
NO_EVENT_MESSAGE="To change map, press ^3Back ^7-> ^3Vote"
# How long event will happen, in hours
DURATION=2
# When to start announcing event, in days
ANNOUNCE_START=3

OUT="$1"
if [ -z "$OUT" ]; then
	OUT=motd.cfg
fi

S1=`date +%s -d "$DATE"`
S2=`date +%s -d "-20 seconds"`
SECONDS=`expr $S1 - $S2`
MINUTES=`expr $SECONDS / 60`
HOURS=`expr $MINUTES / 60`
DAYS=`expr $HOURS / 24`
MINUTES=`expr $MINUTES % 60`
HOURS=`expr $HOURS % 24`

TXT=""
if [ $SECONDS -lt 0 ]; then
	if [ $SECONDS -gt `expr -$DURATION '*' 3600` ]; then
		TXT="come join right now!"
	fi
else
	if [ "$MINUTES" '!=' 0 ]; then
		TXT="$MINUTES minutes"
	fi
	if [ "$HOURS" '!=' 0 ]; then
		if [ "$HOURS" '=' 1 ]; then
			TXT="$HOURS hour $TXT"
		else
			TXT="$HOURS hours $TXT"
		fi
	fi
	if [ "$DAYS" '!=' 0 ]; then
		if [ "$DAYS" '=' 1 ]; then
			TXT="tomorrow $TXT"
		else
			TXT="in $DAYS days $TXT"
		fi
	else
		TXT="in $TXT"
	fi
fi

if [ $SECONDS -gt `expr $ANNOUNCE_START '*' 3600 '*' 24` ]; then
	TXT="" # Do not announce event if it's too far in the future
fi

if [ -n "$TXT" ]; then
	echo "$MESSAGE$TXT" > "$OUT"
else
	echo "$NO_EVENT_MESSAGE" > "$OUT"
fi
