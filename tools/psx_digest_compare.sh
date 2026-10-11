#!/bin/sh
# usage: psx_digest_compare.sh <tty> <pc-digest> <out-dir>
# Compares the `psx-digest: ` lines of a PSX replay's TTY with the PC digest
# file of the same recording (DIGEST=), in order, kind by kind: sprite-sheet,
# screen, map. A missing or extra line is a difference in its kind. Writes
# <out-dir>/psx_digest.txt (the MIPS lines). One line per kind, exit 1 if any
# failed.

set -u

usage="usage: psx_digest_compare.sh <tty> <pc-digest> <out-dir>"
[ $# -eq 3 ] || { echo "psx-digest-compare: $usage" >&2; exit 1; }
tty=$1
pc_digest=$2
out_dir=$3
[ -f "$tty" ] || { echo "psx-digest-compare: missing tty: $tty" >&2; exit 1; }
[ -f "$pc_digest" ] || { echo "psx-digest-compare: missing pc digest: $pc_digest" >&2; exit 1; }

mkdir -p "$out_dir"
psx_digest=$out_dir/psx_digest.txt
tr -d '\r' < "$tty" | sed -n 's/^psx-digest: //p' > "$psx_digest"

failed=0
for kind in sprite-sheet screen map; do
    difference=$(awk -v kind="$kind" '
        $1 != kind { next }
        FILENAME == ARGV[1] { pc[++pc_count] = $0; next }
        { psx[++psx_count] = $0 }
        END {
            if (pc_count == 0) {
                printf "no %s line in the pc digest", kind
                exit
            }
            count = pc_count > psx_count ? pc_count : psx_count
            for (i = 1; i <= count; i++) {
                left = i <= psx_count ? psx[i] : "(none)"
                right = i <= pc_count ? pc[i] : "(none)"
                if (left != right) {
                    printf "first differing %s line, %d:\n  psx: %s\n  pc:  %s", kind, i, left, right
                    exit
                }
            }
        }' "$pc_digest" "$psx_digest")
    if [ -n "$difference" ]; then
        echo "psx-digest-compare: $kind: $difference" >&2
        failed=1
    else
        echo "psx-digest-compare: $kind: ok"
    fi
done

exit $failed
