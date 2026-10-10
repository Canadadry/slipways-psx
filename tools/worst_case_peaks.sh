#!/bin/sh
# usage: worst_case_peaks.sh <table> <out> <events> <state> [<events> <state>...]
# Writes <out> with tools/state_peaks.sh over the given games (its margin
# verdict is make state's job, ignored here), then prints one
# `<zone|cap> <name> table <peak> now <peak> <holder> ok|short` line per line
# of <table>. Exit 1 on a short or missing line, or when a game stopped.

set -u

table=$1
out=$2
shift 2

sh tools/state_peaks.sh "$out" "$@" > /dev/null 2>&1

awk -v out="$out" '
    BEGIN {
        while ((getline line < out) > 0) {
            n = split(line, f, " ")
            if (f[1] == "replay" && f[3] != "clean") { print "worst-case-peaks: " f[2] " " f[3]; bad = 1 }
            if ((f[1] == "zone" || f[1] == "cap") && n == 5) { now[f[1] " " f[2]] = f[3]; holder[f[1] " " f[2]] = f[5] }
        }
    }
    NF == 4 {
        key = $1 " " $2
        if (!(key in now)) { print key, "table", $3, "now - - short"; shorts++; next }
        verdict = now[key] + 0 >= $3 + 0 ? "ok" : "short"
        if (verdict == "short") shorts++
        print key, "table", $3, "now", now[key], holder[key], verdict
    }
    END {
        printf "worst-case-peaks: %d of %d line(s) short\n", shorts, NR
        exit (bad || shorts > 0)
    }
' "$table"
