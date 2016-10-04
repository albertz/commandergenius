#!/bin/sh

find /usr/share/ca-certificates -name '*.crt' | xargs cat > AndroidData/ca-certificates.crt
