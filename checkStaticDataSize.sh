#!/bin/sh
# Output static/global symbols in a application lib to check if someone allocated 200 Mb static temp array.
# Used to resolve linker errors such as: Cannot load library: alloc_mem_region[815]: OOPS:    54 cannot map library 'libapplication.so'. no vspace available.
OBJ="$1"
[ -z "$OBJ" ] && OBJ=project/obj/local/armeabi-v7a/libapplication.so
objdump -x -C -w "$OBJ" | grep '	' | sed 's/.*	/0x/' | sort -n | while read SIZE NAME ; do printf '%8d ' "$SIZE" ; echo "$NAME" ; done
