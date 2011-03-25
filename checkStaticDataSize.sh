#!/bin/sh
# Output static/global symbols in a application lib to check if someone allocated 200 Mb static temp array.
# Used to resolve linker errors such as: Cannot load library: alloc_mem_region[815]: OOPS:    54 cannot map library 'libapplication.so'. no vspace available.
objdump -x project/obj/local/armeabi/libapplication.so | grep '	' | sed 's/.*	/0x/' | sort -n | xargs -n 2 printf '%d %s\n'
