#!/bin/sh
# usage: psx_pad_protocol_check.sh <header> <lua>
# Fails, naming the constant, when a PSX_PAD_<NAME> of the C header and the
# `local <NAME>` of the Lua differ or one is missing.

set -u

[ $# -eq 2 ] || { echo "usage: psx_pad_protocol_check.sh <header> <lua>" >&2; exit 1; }
header=$1
lua=$2
failed=0
for name in HANDSHAKE_OFFSET END_OFFSET HANDSHAKE_MAGIC END_MAGIC; do
    c=$(sed -n "s/^#define PSX_PAD_$name \(0x[0-9a-fA-F]*\)u\{0,1\}\$/\1/p" "$header")
    l=$(sed -n "s/^local $name = \(0x[0-9a-fA-F]*\)\$/\1/p" "$lua")
    if [ -z "$c" ] || [ -z "$l" ]; then
        echo "psx-pad-protocol: $name missing (header: ${c:-none}, lua: ${l:-none})" >&2
        failed=1
    elif [ "$(printf '%d' "$c")" != "$(printf '%d' "$l")" ]; then
        echo "psx-pad-protocol: $name differs (header: $c, lua: $l)" >&2
        failed=1
    fi
done
[ "$failed" -eq 0 ] || exit 1
echo "psx-pad-protocol: ok"
