#!/bin/sh
# usage: psx_real_game_budget.sh <frames-table> <situations-table> <budget-hblanks> <one-off-allowance-vsyncs>
# Joins a frame table (tools/psx_frame_budget.sh's frames.txt) with a situation
# table (psx/perf/full-run-situations.txt) line by line and prints three lines:
# "real-game steady-over ...", "real-game one-off-over ..." and the verdict
# "real-game-budget ok|failed <reason>". Exits 0 once the verdict is printed,
# whatever it is (a missing table is a failed verdict); 1 on a usage error.

set -u

usage="usage: psx_real_game_budget.sh <frames-table> <situations-table> <budget-hblanks> <one-off-allowance-vsyncs>"
[ $# -eq 4 ] || { echo "psx-real-game-budget: $usage" >&2; exit 1; }
frames=$1
situations=$2
budget=$3
allowance=$4
case $budget in
    '' | *[!0-9]* | 0) echo "psx-real-game-budget: budget is not a positive count of hblanks: $budget" >&2; exit 1 ;;
esac
case $allowance in
    '' | *[!0-9]* | 0) echo "psx-real-game-budget: allowance is not a positive count of vsyncs: $allowance" >&2; exit 1 ;;
esac

join_failure() {
    echo "real-game steady-over -"
    echo "real-game one-off-over -"
    echo "real-game-budget failed $1"
    exit 0
}

[ -r "$frames" ] || join_failure "missing frame table: $frames"
[ -r "$situations" ] || join_failure "missing situation table: $situations"

# The vsyncs of a frame over budget are ceil(total / budget): the vsyncs it holds the screen on a
# vsync-locked display. The measured vsyncs column depends on where the frame started; never use it.
awk -v budget="$budget" -v allowance="$allowance" -v frames="$frames" -v situations="$situations" '
    function fail(reason) {
        print "real-game steady-over -"
        print "real-game one-off-over -"
        print "real-game-budget failed " reason
        exit 0
    }
    BEGIN {
        if ((getline header < frames) <= 0) header = ""
        sub(/\r$/, "", header)
        if (header != "frame update draw submit total vsyncs") fail("frame table header is not \"frame update draw submit total vsyncs\": " header)
        if ((getline header < situations) <= 0) header = ""
        sub(/\r$/, "", header)
        if (header != "frame situation family") fail("situation table header is not \"frame situation family\": " header)
        count = 0
        steady_over = 0
        one_off_over = 0
        vsyncs = 0
        while (1) {
            has_frame = (getline frame_line < frames) > 0
            has_situation = (getline situation_line < situations) > 0
            if (!has_frame || !has_situation) break
            sub(/\r$/, "", frame_line)
            sub(/\r$/, "", situation_line)
            frame_fields = split(frame_line, f, " ")
            situation_fields = split(situation_line, s, " ")
            if (frame_fields != 6 || f[5] !~ /^[0-9]+$/) fail("frame table line " count + 2 " malformed: " frame_line)
            if (situation_fields != 3) fail("situation table line " count + 2 " malformed: " situation_line)
            if (f[1] != count || s[1] != count) fail("frame " count ": frame table has " f[1] ", situation table has " s[1])
            if (s[3] != "steady" && s[3] != "one-off") fail("frame " count ": family " s[3] " is neither steady nor one-off")
            total = f[5] + 0
            if (total > budget) {
                if (s[3] == "steady") {
                    steady_over++
                    if (worst_frame == "" || total > worst_total) {
                        worst_frame = count
                        worst_total = total
                        worst_situation = s[2]
                    }
                } else {
                    one_off_over++
                    vsyncs += int((total + budget - 1) / budget)
                }
            }
            count++
        }
        frame_count = count + has_frame
        while ((getline frame_line < frames) > 0) frame_count++
        situation_count = count + has_situation
        while ((getline situation_line < situations) > 0) situation_count++
        if (frame_count != situation_count) fail("frame count " frame_count " vs " situation_count)
        if (count == 0) fail("no frame line")

        worst = worst_frame == "" ? "-" : worst_frame " total " worst_total " situation " worst_situation
        printf "real-game steady-over %d frames, worst steady frame %s\n", steady_over, worst
        printf "real-game one-off-over %d frames, vsyncs %d of %d\n", one_off_over, vsyncs, allowance
        reason = ""
        if (steady_over > 0) reason = steady_over " steady frames over " budget ", one-off vsyncs " vsyncs " of " allowance
        if (vsyncs > allowance) reason = reason (reason == "" ? "" : "; ") "one-off vsyncs " vsyncs " over " allowance
        if (reason == "") print "real-game-budget ok steady-over 0, one-off vsyncs " vsyncs " of " allowance
        else print "real-game-budget failed " reason
    }
'
