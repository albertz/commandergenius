#!/bin/sh

rm -f list.txt list2.txt
mkdir -p games demos

for f in `seq 2400` ; do
[ -e games/$f.html ] || { wget -O games/$f.html "http://www.c64.com/games/$f" && sleep 1 || exit 1 ; }
TITLE="$(cat games/$f.html | iconv -f iso-8859-1 -t utf-8 | grep 'property="og:title"' | sed 's/.*content="\([^"]*\)".*/\1/')"
if [ "$TITLE" = "by - C64.COM" -o "$TITLE" = " by  - C64.COM" ]; then
	continue
fi
echo "$f: $TITLE"
echo "$TITLE|http://www.c64.com/games/download.php?id=$f^" >> list.txt
done


for f in `seq 2400` ; do
[ -e demos/$f.html ] || { wget -O demos/$f.html "http://www.c64.com/demos/demos_show.php?showid=$f" && sleep 1 || exit 1 ; }
TITLE="$(cat demos/$f.html | iconv -f iso-8859-1 -t utf-8 | grep -m 1 -A 1 'title="' | sed 's/.*title="\([^"]*\)".*/\1 by /' | sed 's@.*<br>\([^<]*\)<br>.*@\1@' | tr -d '\n' | sed 's/ [ ]*/ /g')"
if echo "$TITLE" | grep 'Search results per page by' > /dev/null; then
	continue
fi
echo "$f: $TITLE"
echo "$TITLE- C64.COM|http://www.c64.com/demos/download.php?id=$f^" >> list.txt
done

sort list.txt > list2.txt
cat list2.txt | tr -d '\n' > list.txt
rm -f list2.txt
