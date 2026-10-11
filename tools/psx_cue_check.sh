#!/bin/sh
# usage: psx_cue_check.sh <cue>
# On success prints the .bin named by the .cue's first FILE line, resolved next
# to the .cue, and exits 0; otherwise prints the reason and exits 1.

set -u

[ $# -eq 1 ] || { echo "usage: psx_cue_check.sh <cue>"; exit 2; }
cue=$1

name=$(sed -n 's/^[[:space:]]*FILE[[:space:]]*"\([^"]*\)".*/\1/p' "$cue" | head -n 1)
if [ -z "$name" ]; then
    echo "$cue has no FILE \"<bin>\" line"
    exit 1
fi
bin="$(dirname "$cue")/$name"
if [ ! -s "$bin" ]; then
    echo "$bin, named by $cue, is missing or empty"
    exit 1
fi
if [ $(($(wc -c <"$bin") % 2352)) -ne 0 ]; then
    echo "$bin is $(wc -c <"$bin" | tr -d " ") bytes, not a whole number of 2352-byte sectors"
    exit 1
fi
echo "$bin"
