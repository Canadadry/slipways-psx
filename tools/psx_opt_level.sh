#!/bin/sh
# usage: psx_opt_level.sh <flags-file>
# Prints the effective optimization level of a CMake per-target flags.make:
# the last -O<x> token of its C_FLAGS line, as O0|O1|O2|O3|Os|Og|Oz|Ofast.
# GCC takes the last -O, and the PSn00bSDK's interface puts -O2 after
# CMake's release -O3: a grep for -O3 would pass a build that is -O2.
# Exits 1 on a usage error, 2 when the file has no C_FLAGS line or no -O token.

set -u

[ $# -eq 1 ] && [ -f "$1" ] || { echo "psx-opt-level: usage: psx_opt_level.sh <flags-file>" >&2; exit 1; }
grep -q '^C_FLAGS = ' "$1" || { echo "psx-opt-level: no C_FLAGS line in $1" >&2; exit 2; }
flags=$(sed -n 's/^C_FLAGS = //p' "$1")
level=
for token in $flags; do
	case $token in
	-O0 | -O1 | -O2 | -O3 | -Os | -Og | -Oz | -Ofast) level=${token#-} ;;
	esac
done
[ -n "$level" ] || { echo "psx-opt-level: no -O token on the C_FLAGS line of $1" >&2; exit 2; }
echo "$level"
