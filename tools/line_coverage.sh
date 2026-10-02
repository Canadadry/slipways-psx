#!/bin/sh
# usage: line_coverage.sh cover <gcov driver> <gcov tree> <events>...
#        line_coverage.sh diff <set> <set>
#        line_coverage.sh contrib <set> [<set>...]
#        line_coverage.sh gap <reference> <unreachable> <set>
# A set: one `file:line` line per executed source line, sorted.
# cover: the union of the lines covered by the replays, one process per
#   file, each from the title screen. Exit 1, writing nothing on the
#   standard output, when a file is missing or a replay fails.
# diff: the lines of the first set missing from the second.
# contrib: the lines of the first set that none of the others covers (what
#   one file adds).
# gap: the lines of the reference neither covered by the set nor listed as
#   unreachable (`file:line reason`, only the first field counts).
# Exit 1 on bad arguments or a missing set.

set -u

sorted() {
    LC_ALL=C sort -t: -k1,1 -k2,2n -u
}

# The sets after the first make the union removed from the first.
minus() {
    for set in "$@"; do
        if [ ! -f "$set" ]; then
            echo "missing set: $set" >&2
            exit 1
        fi
    done
    first=$1
    shift
    awk -v first="$first" '
        FILENAME != first { seen[$0] = 1; next }
        !($0 in seen)' "$@" "$first" | sorted
}

gap() {
    for set in "$@"; do
        if [ ! -f "$set" ]; then
            echo "missing set: $set" >&2
            exit 1
        fi
    done
    awk -v reference="$1" '
        FILENAME != reference { if (NF) seen[$1] = 1; next }
        !($0 in seen)' "$2" "$3" "$1" | sorted
}

cover() {
    tool=$1
    objdir=$2
    shift 2
    if [ $# -eq 0 ]; then
        echo "cover: no events file" >&2
        exit 1
    fi

    abs_objdir=$(cd "$objdir" && pwd) || exit 1
    # GCOV_PREFIX_STRIP drops the absolute path of the tree: each replay
    # writes its `.gcda` files under its own prefix, laid out as in the tree.
    strip=$(printf '%s\n' "$abs_objdir" | awk -F/ '{ print NF - 1 }')

    work=$(mktemp -d "${TMPDIR:-/tmp}/line_coverage.XXXXXX")
    trap 'rm -rf "$work"' EXIT

    i=0
    failed=0
    for arg in "$@"; do
        i=$((i + 1))
        printf '%s\n' "$arg" > "$work/$i.events"
        if [ ! -f "$arg" ]; then
            echo "absent: $arg" >&2
            failed=1
        else
            (
                GCOV_PREFIX="$work/$i" GCOV_PREFIX_STRIP=$strip \
                    "$tool" "$arg" /dev/null "$work/$i.dump" > /dev/null 2> "$work/$i.err"
                echo $? > "$work/$i.status"
            ) &
        fi
    done
    wait

    n=$i
    i=0
    while [ "$i" -lt "$n" ]; do
        i=$((i + 1))
        [ -f "$work/$i.status" ] || continue
        status=$(cat "$work/$i.status")
        if [ "$status" -ne 0 ]; then
            echo "failed: $(cat "$work/$i.events") (code $status)" >&2
            sed 's/^/  /' "$work/$i.err" >&2
            failed=1
        fi
    done
    [ "$failed" -eq 0 ] || exit 1

    : > "$work/all"
    for dir in $(find "$work" -name '*.gcda' -exec dirname {} \; | sort -u); do
        rel=${dir#"$work"/*/}
        cp "$objdir/$rel"/*.gcno "$dir"/ || exit 1
        if ! gcov -t -o "$dir" "$dir"/*.gcda > "$work/gcov.out" 2> "$work/gcov.err" \
            || grep -q 'mismatch' "$work/gcov.err"; then
            echo "gcov: failed on $dir" >&2
            cat "$work/gcov.err" >&2
            exit 1
        fi
        # System headers (absolute paths) are not code of the game.
        awk -F: '
            $3 == "Source" && $2 + 0 == 0 { src = $4; next }
            {
                count = $1; gsub(/ /, "", count)
                if (count ~ /^[0-9]/ && src !~ /^\//) {
                    line = $2; gsub(/ /, "", line)
                    print src ":" line
                }
            }' "$work/gcov.out" >> "$work/all"
    done
    sorted < "$work/all"
}

case "${1:-}" in
    cover)
        shift
        [ $# -ge 2 ] || { echo "usage: $0 cover <gcov driver> <gcov tree> <events>..." >&2; exit 1; }
        cover "$@"
        ;;
    diff)
        shift
        [ $# -eq 2 ] || { echo "usage: $0 diff <set> <set>" >&2; exit 1; }
        minus "$@"
        ;;
    contrib)
        shift
        [ $# -ge 1 ] || { echo "usage: $0 contrib <set> [<set>...]" >&2; exit 1; }
        minus "$@"
        ;;
    gap)
        shift
        [ $# -eq 3 ] || { echo "usage: $0 gap <reference> <unreachable> <set>" >&2; exit 1; }
        gap "$@"
        ;;
    *)
        sed -n '2,5s/^# //p' "$0" >&2
        exit 1
        ;;
esac
