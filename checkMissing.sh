#!/bin/sh
# Print list of symbols inside libapplication.so which are not defined in Android libc
# Internal compiler things like '__aeabi_ddiv' or '___Unwind_Resume'  may be safely ignored
rm -f exports.txt libapplication.txt
cat exports-eclair.txt > exports.txt
nm -g -p --undefined-only project/obj/local/armeabi/libapplication.so | cut -b 12- | sort > libapplication.txt
for f in project/obj/local/armeabi/*.so; do
	if [ "$f" = "project/obj/local/armeabi/libapplication.so" ]; then
		continue
	fi
	nm -g -p --defined-only $f 2>/dev/null | cut -b 12- >> exports.txt
done
cat exports.txt | sort > exports1.txt
mv -f exports1.txt exports.txt
diff libapplication.txt exports.txt | grep \< | cut -d \  -f 2
rm libapplication.txt exports.txt
