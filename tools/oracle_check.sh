#!/bin/sh
# usage: oracle_check.sh compare <oracle> <state> <diff>
#        oracle_check.sh limits <state> <diff>
# Each prints `tenu`, `ecart <n>` or `arrete <cause>` and writes the gap to
# <diff>. A missing state, or one whose first line starts with `stopped`, is
# `arrete`. A zone or cap line counts when its peak (4th field) is over its
# limit (5th field), equality passing.
# compare: lines other than zone and cap must match the oracle in order; a
#   zone or cap line, paired by its first two fields, must exist on both
#   sides. A limit changed from the oracle is written without counting.
# limits: only the stop and the peaks (the full_game* replays, whose final
#   state is not compared).
# Temporary (plan 12), outside the suite.

set -u

usage() {
    sed -n '2,3s/^# //p' "$0" >&2
    exit 1
}

stopped() {
    if [ ! -f "$1" ]; then
        echo "arrete etat absent"
        return 0
    fi
    first=$(head -n 1 "$1")
    case $first in
        stopped*) echo "arrete $first"; return 0 ;;
    esac
    return 1
}

# One per changed line: a hunk `3,4c3,5` counts the larger of its two sides.
changed_lines() {
    awk '
        /^[0-9]/ {
            split($0, sides, /[acd]/)
            n = split(sides[1], a, ","); old = n == 2 ? a[2] - a[1] + 1 : 1
            n = split(sides[2], b, ","); new = n == 2 ? b[2] - b[1] + 1 : 1
            if ($0 ~ /d/) new = 0
            if ($0 ~ /a/) old = 0
            total += old > new ? old : new
        }
        END { print total + 0 }'
}

# Appends the zone and cap gaps to <diff>, prints their count. With an empty
# oracle path, the pairing is skipped.
peak_gaps() {
    awk -v oracle="$1" -v out="$3" '
        !/^(zone|cap) / { next }
        { key = $1 " " $2 }
        oracle != "" && FILENAME == oracle { limit[key] = $5; next }
        oracle != "" && !(key in limit) { print "seulement actuel: " $0 >> out; gaps++; next }
        { seen[key] = 1 }
        $4 + 0 > $5 + 0 { print "au-dessus de la limite: " $0 >> out; gaps++ }
        oracle != "" && limit[key] != $5 { print "limite changee: " key " " limit[key] " -> " $5 >> out }
        END {
            for (key in limit) if (!(key in seen)) { print "seulement oracle: " key >> out; gaps++ }
            print gaps + 0
        }' ${1:+"$1"} "$2"
}

verdict() {
    if [ "$1" -eq 0 ]; then
        echo tenu
    else
        echo "ecart $1"
    fi
}

compare() {
    oracle=$1
    state=$2
    out=$3
    : > "$out"
    stopped "$state" && return
    work=$(mktemp -d "${TMPDIR:-/tmp}/oracle_check.XXXXXX")
    trap 'rm -rf "$work"' EXIT
    grep -Ev '^(zone|cap) ' "$oracle" > "$work/oracle"
    grep -Ev '^(zone|cap) ' "$state" > "$work/state"
    diff "$work/oracle" "$work/state" > "$out"
    n=$(changed_lines < "$out")
    verdict $((n + $(peak_gaps "$oracle" "$state" "$out")))
}

limits() {
    : > "$2"
    stopped "$1" && return
    verdict "$(peak_gaps "" "$1" "$2")"
}

case "${1:-}" in
    compare)
        shift
        [ $# -eq 3 ] || usage
        compare "$@"
        ;;
    limits)
        shift
        [ $# -eq 2 ] || usage
        limits "$@"
        ;;
    *)
        usage
        ;;
esac
