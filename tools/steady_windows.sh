#!/bin/sh
# usage: steady_windows.sh <frame-table> <situation-table> <budget-hblanks> <max-window> <candidates>
# Joins the two tables with situation_report.sh's header and numbering rules. Per steady situation
# with a frame over the budget, in the rule order of docs/psx-game.md: every window of
# min(max-window, stretch length) frames inside a stretch of it, ranked by frames over (desc), summed
# total (desc), first frame (asc); a window with no frame over is never a candidate. Keeps the best,
# then the next best overlapping no kept one, up to <candidates>. Exits 0 once written; 1 on a usage
# or join error.

set -u

usage="usage: steady_windows.sh <frame-table> <situation-table> <budget-hblanks> <max-window> <candidates>"
[ $# -eq 5 ] || { echo "steady-windows: $usage" >&2; exit 1; }
frames=$1
situations=$2
for number in "$3" "$4" "$5"; do
    case $number in '' | *[!0-9]*) echo "steady-windows: $usage" >&2; exit 1 ;; esac
done
[ "$4" -gt 0 ] || { echo "steady-windows: max-window must be at least 1" >&2; exit 1; }
[ -f "$frames" ] || { echo "steady-windows: missing frame table: $frames" >&2; exit 1; }
[ -f "$situations" ] || { echo "steady-windows: missing situation table: $situations" >&2; exit 1; }

awk -v frames_path="$frames" -v situations_path="$situations" -v budget="$3" -v max_window="$4" -v wanted="$5" '
    function fail(message) {
        print "steady-windows: " message > "/dev/stderr"
        failed = 1
        exit 1
    }
    function count(text) { return text ~ /^[0-9]+$/ }
    # 1 when window a ranks before window b.
    function better(a, b) {
        if (w_over[a] != w_over[b]) return w_over[a] > w_over[b]
        if (w_total[a] != w_total[b]) return w_total[a] > w_total[b]
        return w_first[a] < w_first[b]
    }
    BEGIN {
        rule_order = "title attract map-rest map-move slipway-aim build menu tech-panel score-panel pause"
        known_count = split(rule_order, known, " ")

        if ((getline line < frames_path) <= 0 || line != "frame update draw submit total vsyncs")
            fail("frame table " frames_path " has no \"frame update draw submit total vsyncs\" header")
        frame_lines = 0
        while ((getline line < frames_path) > 0) {
            n = split(line, field, " ")
            if (n != 6) fail("frame table line " frame_lines + 1 " has " n " fields: " line)
            for (i = 1; i <= 6; i++) if (!count(field[i])) fail("frame table line " frame_lines + 1 " is not numeric: " line)
            if (field[1] != frame_lines) fail("frame table line " frame_lines + 1 " is frame " field[1] ", expected " frame_lines)
            update[frame_lines] = field[2]; draw[frame_lines] = field[3]; total[frame_lines] = field[5]
            frame_lines++
        }
        if ((getline line < situations_path) <= 0 || line != "frame situation family")
            fail("situation table " situations_path " has no \"frame situation family\" header")
        situation_lines = 0
        while ((getline line < situations_path) > 0) {
            n = split(line, field, " ")
            if (n != 3 || !count(field[1])) fail("situation table line " situation_lines + 1 " is not \"<frame> <situation> <family>\": " line)
            if (field[1] != situation_lines) fail("situation table line " situation_lines + 1 " is frame " field[1] ", expected " situation_lines)
            label[situation_lines] = field[2]; family[situation_lines] = field[3]
            situation_lines++
        }
        if (frame_lines != situation_lines) fail("frame table " frame_lines " frames, situation table " situation_lines)

        for (s = 1; s <= known_count; s++) {
            name = known[s]
            over_count = 0
            for (k = 0; k < frame_lines; k++) if (label[k] == name && family[k] == "steady" && total[k] > budget) over_count++
            if (over_count == 0) continue

            windows = 0
            for (start = 0; start < frame_lines; start = stop) {
                for (stop = start; stop < frame_lines && label[stop] == label[start]; stop++);
                if (label[start] != name || family[start] != "steady") continue
                size = stop - start < max_window ? stop - start : max_window
                for (first = start; first + size <= stop; first++) {
                    o = 0; t = 0; u = 0; d = 0
                    for (k = first; k < first + size; k++) { o += total[k] > budget; t += total[k]; u += update[k]; d += draw[k] }
                    if (o == 0) continue
                    windows++
                    w_first[windows] = first; w_last[windows] = first + size - 1
                    w_over[windows] = o; w_total[windows] = t; w_update[windows] = u; w_draw[windows] = d
                }
            }

            kept = 0
            while (kept < wanted) {
                best = 0
                for (w = 1; w <= windows; w++) {
                    clash = 0
                    for (j = 1; j <= kept; j++)
                        if (w_first[w] <= w_last[kept_window[j]] && w_first[kept_window[j]] <= w_last[w]) { clash = 1; break }
                    if (!clash && (best == 0 || better(w, best))) best = w
                }
                if (best == 0) break
                kept_window[++kept] = best
                size = w_last[best] - w_first[best] + 1
                print "candidate " name " " kept " " w_first[best] " " w_last[best] " frames " size " over " w_over[best] \
                    " mean-update " int(w_update[best] / size) " mean-draw " int(w_draw[best] / size)
            }
            print "situation " name " frames-over " over_count " candidates " kept
        }
    }'
