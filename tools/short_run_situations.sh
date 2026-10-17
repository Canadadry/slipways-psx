#!/bin/sh
# usage: short_run_situations.sh <run-situations> <map.txt> <full-situations>
# The run's name is the directory holding <map.txt>. Window frame w (full game) is run frame
# first-game-frame + (w - source-first); earlier run frames are never compared. Prints one line,
# "short-run-situation <name> " then: "same <situation>" when the full window carries one steady
# situation and every run window frame carries it; "mixed <labels>" when the full window carries
# several; "not-steady <label>" when its one label is one-off; else "differs frame <full-frame> run
# <label> full <label>" for the first difference, run label "missing" past the run table's end.
# Exits 0 once written; 1 on a usage or parse error.

set -u

usage="usage: short_run_situations.sh <run-situations> <map.txt> <full-situations>"
[ $# -eq 3 ] || { echo "short-run-situations: $usage" >&2; exit 1; }
for file in "$1" "$2" "$3"; do
    [ -f "$file" ] || { echo "short-run-situations: missing file: $file" >&2; exit 1; }
done
name=$(basename "$(dirname "$2")")

awk -v run_path="$1" -v map_path="$2" -v full_path="$3" -v name="$name" '
    function fail(message) {
        print "short-run-situations: " message > "/dev/stderr"
        exit 1
    }
    function count(text) { return text ~ /^[0-9]+$/ }
    # Fills label_of[prefix frame] and family_of[prefix frame]; returns the frame count.
    function load(path, prefix,    line, n, field, lines) {
        if ((getline line < path) <= 0 || line != "frame situation family")
            fail(path " has no \"frame situation family\" header")
        lines = 0
        while ((getline line < path) > 0) {
            n = split(line, field, " ")
            if (n != 3 || !count(field[1]) || field[1] != lines)
                fail(path " line " lines + 2 " is not \"" lines " <situation> <family>\": " line)
            label_of[prefix, lines] = field[2]; family_of[prefix, lines] = field[3]
            lines++
        }
        return lines
    }
    BEGIN {
        if ((getline line < map_path) <= 0) fail(map_path " is empty")
        n = split(line, field, " ")
        if (n != 9 || field[1] != "cut" || (field[2] != "none" && !count(field[2])) || field[3] != "first-game-frame" \
            || !count(field[4]) || field[5] != "source-first" || !count(field[6]) || field[7] != "window" \
            || !count(field[8]) || !count(field[9]) || field[8] > field[9] || field[8] < field[6])
            fail(map_path " is not \"cut <W|none> first-game-frame <g> source-first <s> window <first> <last>\": " line)
        game_first = field[4]; source_first = field[6]; first = field[8]; last = field[9]
        run_count = load(run_path, "run")
        full_count = load(full_path, "full")
        if (last >= full_count) fail("window ends at frame " last ", " full_path " has " full_count " frames")

        labels = ""
        for (w = first; w <= last; w++) {
            if (!((label_of["full", w]) in seen)) {
                seen[label_of["full", w]] = 1
                labels = labels (labels == "" ? "" : " ") label_of["full", w]
            }
        }
        prefix = "short-run-situation " name " "
        if (labels ~ / /) { print prefix "mixed " labels; exit 0 }
        if (family_of["full", first] != "steady") { print prefix "not-steady " labels; exit 0 }
        for (w = first; w <= last; w++) {
            r = game_first + (w - source_first)
            run_label = r < run_count ? label_of["run", r] : "missing"
            if (run_label != labels) {
                print prefix "differs frame " w " run " run_label " full " labels
                exit 0
            }
        }
        print prefix "same " labels
    }'
