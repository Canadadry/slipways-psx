#!/bin/sh
# usage: coverage_compare.sh <baseline lines> <current lines> <diff>
# The lines files are coverage.sh's `<path>:<line> <0|1>`; <diff> is
# `git diff -U0 -M --histogram` from the baseline commit to the current tree.
# Prints, for each line hit in the baseline:
#   hit <baseline path:line> <current path:line>
#   unhit <baseline path:line> <current path:line>  (now 0 or not executable)
#   deleted <baseline path:line>  (inside a hunk: removed or rewritten)
# then `totals hit <n> unhit <n> deleted <n>`; exit 1 if any line is unhit.

set -u
LC_ALL=C
export LC_ALL

if [ $# -ne 3 ]; then
    echo "usage: coverage_compare.sh <baseline lines> <current lines> <diff>" >&2
    exit 2
fi

awk '
    function count_of(field) { return field ~ /,/ ? substr(field, index(field, ",") + 1) + 0 : 1 }
    function start_of(field) { sub(/,.*/, "", field); return substr(field, 2) + 0 }
    function new_path_of(path) { return path in new_path ? new_path[path] : path }
    # Sets mapped_line; returns 0 when <line> of <path> lies inside a hunk.
    function map(path, line,    i, offset) {
        offset = 0
        for (i = 1; i <= hunks[path]; i++) {
            if (old_count[path, i] == 0) {
                if (line <= old_start[path, i]) break
            } else {
                if (line < old_start[path, i]) break
                if (line < old_start[path, i] + old_count[path, i]) return 0
            }
            offset += new_count[path, i] - old_count[path, i]
        }
        mapped_line = line + offset
        return 1
    }
    FILENAME == ARGV[1] { current[$1] = $2; next }
    FILENAME == ARGV[2] && /^diff --git / { path = substr($3, 3); new_path[path] = substr($4, 3); next }
    FILENAME == ARGV[2] && /^@@ / {
        n = ++hunks[path]
        old_start[path, n] = start_of($2); old_count[path, n] = count_of($2)
        new_count[path, n] = count_of($3)
        next
    }
    FILENAME == ARGV[2] { next }
    $2 != 1 { next }
    {
        split($1, key, ":")
        if (!map(key[1], key[2])) { print "deleted", $1; deleted++; next }
        now = new_path_of(key[1]) ":" mapped_line
        if (current[now] == 1) { print "hit", $1, now; hit++ }
        else { print "unhit", $1, now; unhit++ }
    }
    END {
        printf "totals hit %d unhit %d deleted %d\n", hit, unhit, deleted
        exit unhit > 0
    }
' "$2" "$3" "$1"
