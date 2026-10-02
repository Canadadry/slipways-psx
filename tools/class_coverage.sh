#!/bin/sh
# usage: class_coverage.sh [--expect <reference>]
#                          <class_coverage tool> <dir|->
#                          [<exemptions> [<events>...]]
# Replays every <dir>/*events.json (not recursing into subdirectories;
# - : none), then every named <events>, one process per replay, from the
# title screen. Exits 1 if a class is at 0 without an exemption, or, with
# --expect, if the output differs byte for byte from <reference> (a missing
# <reference> included).

set -u

expect=
if [ "${1:-}" = --expect ]; then
    expect=$2
    shift 2
fi

tool=$1
dir=$2
exempt=${3:-}
shift 2
[ $# -gt 0 ] && shift

[ "$dir" = - ] || set -- "$dir"/*events.json "$@"

work=$(mktemp -d "${TMPDIR:-/tmp}/class_coverage.XXXXXX")
trap 'rm -rf "$work"' EXIT

report() {
    run=0
    crashed=0
    for events in "$@"; do
        [ -f "$events" ] || continue
        "$tool" "$events" > "$work/out" 2> "$work/err"
        status=$?
        if [ "$status" -eq 2 ]; then
            echo "unreadable: $events"
            continue
        fi
        if [ "$status" -ne 0 ]; then
            echo "failed: $events (code $status)"
            sed 's/^/  /' "$work/err"
            crashed=$((crashed + 1))
            continue
        fi
        run=$((run + 1))
        cat "$work/out" >> "$work/all"
    done
    touch "$work/all"

    if [ -n "$exempt" ]; then
        cp "$exempt" "$work/exempt"
    else
        : > "$work/exempt"
    fi

    # Test FILENAME rather than compare the record counters: with an empty
    # exemptions file, the counts would be read as exemptions.
    awk -v run="$run" -v exempt_file="$work/exempt" '
        FILENAME == exempt_file {
            if ($1 != "") { reason = $0; sub(/^[^ ]+ +/, "", reason); exempt[$1] = reason }
            next
        }
        {
            total[$1] += $2
            if ($2 > 0) replays[$1]++
        }
        END {
            for (name in total) {
                line = sprintf("%s  %d  %d/%d", name, total[name], replays[name] + 0, run)
                if (name in exempt) line = line "  exempt (" exempt[name] ")"
                print total[name] "\t" line
            }
        }' "$work/exempt" "$work/all" > "$work/lines"

    sort -t "$(printf '\t')" -k1,1n -k2,2 "$work/lines" | cut -f2- | tee "$work/sorted"

    missing=$(awk '$2 == 0 && $4 != "exempt" { printf "%s%s", sep, $1; sep = " " }' "$work/sorted")
    if [ "$crashed" -gt 0 ]; then
        echo "coverage: FAIL -- $crashed replay(s) failed"
        return 1
    fi
    if [ "$run" -eq 0 ]; then
        echo "coverage: FAIL -- no readable replay in $dir"
        return 1
    fi
    if [ -n "$missing" ]; then
        echo "coverage: FAIL -- never instantiated and not exempt: $missing ($run replay(s))"
        return 1
    fi
    echo "coverage: OK -- every non-exempt class is instantiated ($run replay(s))"
    return 0
}

{ report "$@"; echo $? > "$work/status"; } | tee "$work/report"
status=$(cat "$work/status")

if [ -n "$expect" ] && ! cmp -s "$expect" "$work/report"; then
    [ -f "$expect" ] && diff "$expect" "$work/report"
    echo "coverage: FAIL -- output differs from $expect"
    exit 1
fi
exit "$status"
