#!/bin/sh
# usage: data_residue.sh [--detail | --check <cat>[,<cat>...]] [dir...]
# Counts, in the `.c` and `.h` files under the directories (default: src
# tests; paths printed as given), the lines of each category of residue D,
# comments included; a line counts in every category it matches.
#   no option : `<cat>\t<lines>\t<files>` per nonzero category, then
#               `residue D: N line(s), M file(s)` (distinct file:line
#               couples and distinct files); exit 0 whatever the count.
#   --detail  : `<cat>\t<file>\t<lines>` per nonzero couple, then the same
#               footer; exit 0.
#   --check   : `<cat>: <file>:<line>:<text>` per match of the named
#               categories, then `guarded residue (<cats>): N line(s)`;
#               exit 1 when N > 0.
# A clean tree prints only `residue D: 0 line(s), 0 file(s)`.
# Exit 2 on an unknown option or category, a missing directory, no `.c` or
# `.h` file, a grep failure: a measure over nothing would pass for clean.

set -u

CATEGORIES="by-name dispatch overrides spec"

fail() {
    echo "data_residue: $1" >&2
    exit 2
}

pattern() {
    case $1 in
        by-name) printf '%s\n' '\bpico8_entity_(get|set)_field\b|\b[a-z0-9_]*field_truthy\(|'\
'\bpico8_[a-z0-9_]+_(set_)?field(_fn)?\b|(->|\.)(set_)?field\b|\b[a-z0-9_]+_take_[a-z0-9_]+\(' ;;
        dispatch) printf '%s\n' '\bpico8_value_function(_t)?\b|\bpico8_mv\(' ;;
        overrides) printf '%s\n' '\boverrides\b' ;;
        spec) printf '%s\n' '\bg_[a-z0-9_]+_(spec|defaults)\b|'\
'\b[a-z0-9_]+_default_(fixed|table|vec|string|box|list|bool|hf)\b' ;;
        *) return 1 ;;
    esac
}

mode=count
checked=
case ${1:-} in
    --detail) mode=detail; shift ;;
    --check)
        mode=check
        [ $# -ge 2 ] || fail "--check without a category"
        checked=$2
        shift 2
        [ -n "$checked" ] || fail "--check without a category"
        for cat in $(printf '%s\n' "$checked" | tr ',' ' '); do
            pattern "$cat" > /dev/null || fail "unknown category: $cat"
        done
        case ,$checked, in
            *,,*) fail "empty category in: $checked" ;;
        esac
        ;;
    -*) fail "unknown option: $1" ;;
esac
[ $# -gt 0 ] || set -- src tests

LC_ALL=C
export LC_ALL

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

for dir in "$@"; do
    [ -d "$dir" ] || fail "missing directory: $dir"
done
find "$@" \( -name '*.c' -o -name '*.h' \) -type f | sort > "$work/files"
[ -s "$work/files" ] || fail "no .c or .h file in: $*"

# Writes `<file>:<line>:<text>` lines into $work/<cat>, in file then line
# order. grep exits 1 when nothing matches.
collect() {
    status=0
    grep -HnE "$(pattern "$1")" $(cat "$work/files") > "$work/$1" || status=$?
    [ "$status" -le 1 ] || fail "grep failed"
}

if [ "$mode" = check ]; then
    : > "$work/all"
    for cat in $(printf '%s\n' "$checked" | tr ',' ' '); do
        collect "$cat"
        sed "s|^|$cat: |" "$work/$cat"
        cut -d: -f1,2 "$work/$cat" >> "$work/all"
    done
    n=$(sort -u "$work/all" | wc -l | tr -d ' ')
    echo "guarded residue ($checked): $n line(s)"
    [ "$n" -eq 0 ]
    exit
fi

: > "$work/all"
for cat in $CATEGORIES; do
    collect "$cat"
    [ -s "$work/$cat" ] || continue
    cut -d: -f1,2 "$work/$cat" >> "$work/all"
    if [ "$mode" = detail ]; then
        cut -d: -f1 "$work/$cat" | uniq -c | awk -v c="$cat" '{ print c "\t" $2 "\t" $1 }'
    else
        lines=$(wc -l < "$work/$cat" | tr -d ' ')
        files=$(cut -d: -f1 "$work/$cat" | sort -u | wc -l | tr -d ' ')
        printf '%s\t%s\t%s\n' "$cat" "$lines" "$files"
    fi
done
lines=$(sort -u "$work/all" | wc -l | tr -d ' ')
files=$(cut -d: -f1 "$work/all" | sort -u | wc -l | tr -d ' ')
echo "residue D: $lines line(s), $files file(s)"
