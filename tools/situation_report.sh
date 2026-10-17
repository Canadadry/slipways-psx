#!/bin/sh
# usage: situation_report.sh <frame-table> <situation-table>
# Joins a frame table (psx_frame_budget.sh's frames.txt) with a situation table
# (tmp/situations's output) line by line. Prints one "situation" line per
# situation present, steady first then one-off, each in the rule's order of
# docs/psx-game.md; one "one-off" line per one-off situation present; then the
# verdict lines frame-count, families, one-off-runs: "<condition> ok[ <detail>]"
# or "<condition> failed <reason>". A one-off run is a stretch of consecutive
# frames with the same one-off label, so two events of one label back to back
# count as one occurrence. Exits 0 once written, whatever the verdicts; 1 on a
# usage or parse error.

set -u

usage="usage: situation_report.sh <frame-table> <situation-table>"
[ $# -eq 2 ] || { echo "situation-report: $usage" >&2; exit 1; }
frames=$1
situations=$2
[ -f "$frames" ] || { echo "situation-report: missing frame table: $frames" >&2; exit 1; }
[ -f "$situations" ] || { echo "situation-report: missing situation table: $situations" >&2; exit 1; }

awk -v frames_path="$frames" -v situations_path="$situations" '
    function fail(message) {
        print "situation-report: " message > "/dev/stderr"
        exit 1
    }
    function count(text) { return text ~ /^[0-9]+$/ }
    BEGIN {
        # Over one NTSC vsync; a one-off run longer than this many frames is long.
        budget = 263
        long_run = 31
        rule_order = "title attract map-rest map-move slipway-aim build menu tech-panel score-panel pause " \
            "galaxy-generation return-to-title game-end new-year research colonise structure-built " \
            "slipway-built probe screen-change"
        known_count = split(rule_order, known, " ")
        for (i = 1; i <= known_count; i++) rank[known[i]] = i

        if ((getline line < frames_path) <= 0 || line != "frame update draw submit total vsyncs")
            fail("frame table " frames_path " has no \"frame update draw submit total vsyncs\" header")
        frame_lines = 0
        while ((getline line < frames_path) > 0) {
            n = split(line, field, " ")
            if (n != 6) fail("frame table line " frame_lines + 1 " has " n " fields: " line)
            for (i = 1; i <= 6; i++) if (!count(field[i])) fail("frame table line " frame_lines + 1 " is not numeric: " line)
            if (numbering == "" && field[1] != frame_lines)
                numbering = "frame table line " frame_lines + 1 " is frame " field[1] ", expected " frame_lines
            update[frame_lines] = field[2]; draw[frame_lines] = field[3]; total[frame_lines] = field[5]
            frame_lines++
        }

        if ((getline line < situations_path) <= 0 || line != "frame situation family")
            fail("situation table " situations_path " has no \"frame situation family\" header")
        situation_lines = 0
        while ((getline line < situations_path) > 0) {
            n = split(line, field, " ")
            if (n != 3 || !count(field[1])) fail("situation table line " situation_lines + 1 " is not \"<frame> <situation> <family>\": " line)
            if (numbering == "" && field[1] != situation_lines)
                numbering = "situation table line " situation_lines + 1 " is frame " field[1] ", expected " situation_lines
            label[situation_lines] = field[2]; family[situation_lines] = field[3]
            situation_lines++
        }

        for (k = 0; k < situation_lines; k++) {
            name = label[k]
            if (!(name in frames_of)) {
                if (!(name in rank)) { rank[name] = known_count + 1 + unknown_count++; extra[unknown_count] = name }
                family_of[name] = family[k]
            }
            if (family[k] != "steady" && family[k] != "one-off") {
                if (families == "") families = name " has unknown family " family[k]
            } else if (family_of[name] != family[k] && families == "") {
                families = name " is both " family_of[name] " and " family[k]
            }
            if (!(name in rank) || rank[name] > known_count) {
                if (families == "") families = "unknown situation " name
            }
            frames_of[name]++
            if (k < frame_lines) {
                update_sum[name] += update[k]; draw_sum[name] += draw[k]
                joined[name]++
                if (total[k] > budget) over[name]++
            }
            if (family[k] == "one-off") {
                if (k > 0 && label[k - 1] == name) run++
                else { run = 1; occurrences[name]++ }
                if (run > longest[name]) longest[name] = run
            }
        }

        for (i = 1; i <= known_count + unknown_count; i++) {
            name = i <= known_count ? known[i] : extra[i - known_count]
            if (!(name in frames_of)) continue
            print "situation " name " " family_of[name] " frames " frames_of[name] " over " over[name] + 0 \
                " update-mean " (joined[name] > 0 ? int(update_sum[name] / joined[name]) : 0) \
                " draw-mean " (joined[name] > 0 ? int(draw_sum[name] / joined[name]) : 0)
        }
        for (i = 1; i <= known_count + unknown_count; i++) {
            name = i <= known_count ? known[i] : extra[i - known_count]
            if (!(name in occurrences)) continue
            is_long = longest[name] > long_run
            if (is_long) long_names = long_names (long_names == "" ? "" : " ") name
            print "one-off " name " occurrences " occurrences[name] " longest " longest[name] (is_long ? " long" : "")
        }

        if (numbering != "") print "frame-count failed " numbering
        else if (frame_lines != situation_lines)
            print "frame-count failed frame table " frame_lines " frames, situation table " situation_lines
        else print "frame-count ok " frame_lines " frames"
        print "families " (families == "" ? "ok" : "failed " families)
        print "one-off-runs " (long_names == "" ? "ok" : "failed " long_names)
    }'
