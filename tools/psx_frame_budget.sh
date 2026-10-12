#!/bin/sh
# usage: psx_frame_budget.sh <gpu-tty> <gpu-psx-state> <expected-frames> <budget-hblanks> <out-dir>
# Analyzes a finished GPU replay boot: the `psx-frame: ` lines of its TTY and
# the zone/budget/cap lines of its MIPS state (psx_state_compare.sh's
# psx_state.txt). Writes <out-dir>/frames.txt (a header naming the columns,
# then one line a frame), <out-dir>/summary.txt and <out-dir>/verdicts.txt,
# one line per condition in the order timer, frame-budget, zone-sizes,
# cap-sizes: "<condition> ok[ <detail>]" or "<condition> failed <reason>".
# Exits 0 once the three files are written, whatever the verdicts; 1 on a
# usage error.

set -u

usage="usage: psx_frame_budget.sh <gpu-tty> <gpu-psx-state> <expected-frames> <budget-hblanks> <out-dir>"
[ $# -eq 5 ] || { echo "psx-frame-budget: $usage" >&2; exit 1; }
tty=$1
state=$2
expected_frames=$3
budget=$4
out_dir=$5
[ -f "$tty" ] || { echo "psx-frame-budget: missing tty: $tty" >&2; exit 1; }
[ -f "$state" ] || { echo "psx-frame-budget: missing state: $state" >&2; exit 1; }
case $expected_frames in
    '' | *[!0-9]*) echo "psx-frame-budget: expected frames is not a count: $expected_frames" >&2; exit 1 ;;
esac
case $budget in
    '' | *[!0-9]*) echo "psx-frame-budget: budget is not a count of hblanks: $budget" >&2; exit 1 ;;
esac

mkdir -p "$out_dir" || exit 1

timer_and_frames=$(tr -d '\r' < "$tty" | sed -n 's/^psx-frame: //p' | awk -v expected="$expected_frames" -v budget="$budget" \
    -v frames_out="$out_dir/frames.txt" -v summary_out="$out_dir/summary.txt" '
    function sign(n) { return n >= 0 ? "+" n : n }
    BEGIN {
        print "frame update draw submit total vsyncs" > frames_out
        count = 0
        timer_failure = ""
        calibration = ""
        # Overruns are bucketed against one NTSC vsync, whatever the budget.
        vsync_lines = 263
    }
    $1 == "hblanks-per-vsync" {
        if (calibration == "") calibration = $2
        next
    }
    $1 == "frame" {
        if ($3 != "update" || $5 != "draw" || $7 != "submit" || $9 != "total" || $11 != "vsyncs" || NF != 12) {
            if (timer_failure == "") timer_failure = "malformed frame line: " $0
            next
        }
        if (timer_failure == "" && $2 != count) {
            timer_failure = "expected frame " count ", got frame " $2
        }
        if (timer_failure == "" && count >= expected) {
            timer_failure = "expected " expected " frames, got frame " $2
        }
        if (timer_failure == "" && $12 >= 249) {
            timer_failure = "frame " $2 " spans " $12 " vsyncs, the timer may have wrapped"
        }
        print $2, $4, $6, $8, $10, $12 > frames_out
        count++
        total = $10
        all_update += $4; all_draw += $6; all_submit += $8; all_total += total
        if (count == 1 || total > worst_total) {
            worst_frame = $2; worst_total = total
            worst_update = $4; worst_draw = $6; worst_submit = $8
        }
        if (total > budget) {
            over++
            overrun = total - budget
            overrun_sum += overrun
            over_update += $4; over_draw += $6; over_submit += $8
            if (overrun * 4 <= vsync_lines) bucket_quarter++
            else if (overrun * 2 <= vsync_lines) bucket_half++
            else if (overrun <= vsync_lines) bucket_one++
            else if (overrun <= 2 * vsync_lines) bucket_two++
            else bucket_more++
        }
    }
    END {
        if (calibration == "") {
            timer = "failed no hblanks-per-vsync line"
        } else if (calibration < 255 || calibration > 271) {
            timer = "failed hblanks-per-vsync " calibration " outside 255..271"
        } else if (timer_failure != "") {
            timer = "failed " timer_failure
        } else if (count != expected) {
            timer = "failed expected frame " count ", got end of frames"
        } else {
            timer = "ok hblanks-per-vsync " calibration ", " count " frames"
        }
        print "timer " timer

        if (count == 0) {
            print "frame-budget failed no frame line"
        } else if (over > 0) {
            print "frame-budget failed " over " frames over " budget " hblanks, worst frame " worst_frame " total " worst_total
        } else {
            print "frame-budget ok worst frame " worst_frame " total " worst_total
        }

        print "frames " count " budget " budget " hblanks" > summary_out
        if (count == 0) {
            print "over 0 frames (0‰), worst frame -" > summary_out
        } else {
            print "over " over + 0 " frames (" int(over * 1000 / count) "‰), worst frame " worst_frame \
                " total " worst_total " (" sign(worst_total - budget) ") update " worst_update \
                " draw " worst_draw " submit " worst_submit > summary_out
        }
        print "overrun mean " (over > 0 ? int(overrun_sum / over) : 0) " hblanks over the over-budget frames" > summary_out
        print "overrun distribution <=1/4 vsync " bucket_quarter + 0 " <=1/2 " bucket_half + 0 " <=1 " bucket_one + 0 \
            " <=2 " bucket_two + 0 " >2 " bucket_more + 0 > summary_out
        dominant = "-"
        if (over > 0) {
            dominant = "update"; largest = over_update
            if (over_draw > largest) { dominant = "draw"; largest = over_draw }
            if (over_submit > largest) { dominant = "submit"; largest = over_submit }
        }
        print "parts over-budget frames update " over_update + 0 " draw " over_draw + 0 " submit " over_submit + 0 \
            " dominant " dominant > summary_out
        print "parts all frames update " all_update + 0 " draw " all_draw + 0 " submit " all_submit + 0 \
            " mean total " (count > 0 ? int(all_total / count) : 0) > summary_out
    }')

sizes=$(awk -v summary_out="$out_dir/summary.txt" '
    function judge(kind, line, peak, limit,    margin) {
        if (peak > limit) {
            print line " over by " (peak - limit) >> summary_out
            failures[kind] = failures[kind] (failures[kind] == "" ? "" : "; ") line
        } else {
            margin = limit > 0 ? int((limit - peak) * 8 / limit) : 0
            print line " ok margin " margin "/8" >> summary_out
        }
    }
    $1 == "zone" && NF == 5 { zones++; judge("zone", "zone " $2 " peak " $4 " limit " $5, $4, $5) }
    $1 == "budget" && $2 == "zones" && NF == 4 { budgets++; judge("zone", "budget zones used " $3 " total " $4, $3, $4) }
    $1 == "cap" && NF == 5 { caps++; judge("cap", "cap " $2 " peak " $4 " limit " $5, $4, $5) }
    END {
        if (zones == 0) zone_verdict = "failed no zone line in the state"
        else if (budgets == 0) zone_verdict = "failed no budget zones line in the state"
        else if (failures["zone"] != "") zone_verdict = "failed " failures["zone"]
        else zone_verdict = "ok " zones " zones and the budget fit"
        if (caps == 0) cap_verdict = "failed no cap line in the state"
        else if (failures["cap"] != "") cap_verdict = "failed " failures["cap"]
        else cap_verdict = "ok " caps " caps fit"
        print "zone-sizes " zone_verdict
        print "cap-sizes " cap_verdict
    }' "$state")

printf '%s\n%s\n' "$timer_and_frames" "$sizes" > "$out_dir/verdicts.txt"
exit 0
