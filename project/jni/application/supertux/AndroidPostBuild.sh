#!/bin/sh

cd supertux/data || exit 1
zip -r -n .png:.ogg:.jpg $1 * || exit 1
