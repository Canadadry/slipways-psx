#!/bin/sh
# usage: psx_short_run.sh <plain-tty-1> <plain-tty-2> <breakdown-tty> <map> <frames> <budget-hblanks> <reference-frames> <out-dir> <asserts on|off>
# Analyzes one short run of make psx-short-runs: two boots of the plain GPU
# replay, one of psx-gpu-replay-breakdown, the run's map.txt (from
# tmp/short_run_cut) and its frame count. <reference-frames> is a frame table
# of tools/psx_frame_budget.sh over full_game.json, or empty to skip
# representative. <asserts> is the mode every boot's "psx-replay: asserts"
# line must report. Writes <out-dir>/verdicts.txt, one line per condition in
# the order done, deterministic, representative, breakdown, asserts:
# "<condition> ok[ <detail>]", "<condition> failed <reason>" or
# "representative skipped ..."; <out-dir>/summary.txt (the window's plain
# update, draw and submit sums, draw split, overhead and best cases);
# <out-dir>/window.txt (plain and reference figures side by side, with a
# reference only). Prints both. A missing TTY fails conditions, not the
# script. Exits 0 when no condition failed, 2 when one did, 1 on a usage error.

set -u

usage="usage: psx_short_run.sh <plain-tty-1> <plain-tty-2> <breakdown-tty> <map> <frames> <budget-hblanks> <reference-frames> <out-dir> <asserts on|off>"
[ $# -eq 9 ] || { echo "psx-short-run: $usage" >&2; exit 1; }
plain_1=$1
plain_2=$2
breakdown=$3
map=$4
frames=$5
budget=$6
reference=$7
out_dir=$8
asserts=$9
case $asserts in
    on | off) ;;
    *) echo "psx-short-run: asserts is not on or off: $asserts" >&2; exit 1 ;;
esac
[ -f "$map" ] || { echo "psx-short-run: missing map: $map" >&2; exit 1; }
case $frames in
    '' | *[!0-9]*) echo "psx-short-run: frames is not a count: $frames" >&2; exit 1 ;;
esac
case $budget in
    '' | *[!0-9]*) echo "psx-short-run: budget is not a count of hblanks: $budget" >&2; exit 1 ;;
esac

set -- $(head -n 1 "$map")
if [ $# -ne 9 ] || [ "$1" != cut ] || [ "$3" != first-game-frame ] || [ "$5" != source-first ] || [ "$7" != window ]; then
    echo "psx-short-run: malformed map: $map" >&2
    exit 1
fi
first_game=$4
source_first=$6
window_first=$8
window_last=$9
run_first=$((first_game + window_first - source_first))
run_last=$((first_game + window_last - source_first))
if [ "$run_first" -lt "$first_game" ] || [ "$run_last" -lt "$run_first" ] || [ "$run_last" -ge "$frames" ]; then
    echo "psx-short-run: window $window_first..$window_last is run frames $run_first..$run_last, outside $first_game..$((frames - 1))" >&2
    exit 1
fi

mkdir -p "$out_dir" || exit 1
rm -f "$out_dir/window.txt"

# usage: frame_lines <tty>; its "psx-frame: " lines without the prefix
frame_lines() {
    tr -d '\r' < "$1" | sed -n 's/^psx-frame: //p'
}

done_failures=
for boot in plain-1 plain-2 breakdown; do
    case $boot in
        plain-1) tty=$plain_1 ;;
        plain-2) tty=$plain_2 ;;
        *) tty=$breakdown ;;
    esac
    if [ ! -f "$tty" ]; then
        reason="$boot tty missing: $tty"
    else
        done_line=$(tr -d '\r' < "$tty" | grep '^psx-replay: done ' | tail -n 1)
        stopped=$(tr -d '\r' < "$tty" | grep '^psx-state: stopped' | head -n 1)
        set -- $done_line
        if [ -n "$stopped" ]; then
            reason="$boot stopped: ${stopped#psx-state: }"
        elif [ "${4:-}" != "$frames" ]; then
            reason="$boot expected $frames frames, got: ${done_line:-(no done line)}"
        else
            reason=
        fi
    fi
    [ -n "$reason" ] && done_failures="$done_failures${done_failures:+; }$reason"
done
if [ -n "$done_failures" ]; then
    done_verdict="done failed $done_failures"
else
    done_verdict="done ok 3 boots, $frames frames each"
fi

asserts_failures=
for boot in plain-1 plain-2 breakdown; do
    case $boot in
        plain-1) tty=$plain_1 ;;
        plain-2) tty=$plain_2 ;;
        *) tty=$breakdown ;;
    esac
    if [ ! -f "$tty" ]; then
        reason="$boot tty missing: $tty"
    else
        set -- $(tr -d '\r' < "$tty" | grep '^psx-replay: asserts ' | head -n 1)
        if [ $# -eq 0 ]; then
            reason="$boot has no asserts line"
        elif [ "${3:-}" != "$asserts" ]; then
            reason="$boot says ${3:-}"
        else
            reason=
        fi
    fi
    [ -n "$reason" ] && asserts_failures="$asserts_failures${asserts_failures:+; }$reason"
done
if [ -n "$asserts_failures" ]; then
    asserts_verdict="asserts failed $asserts_failures"
else
    asserts_verdict="asserts ok $asserts in 3 boots"
fi

if [ ! -f "$plain_1" ] || [ ! -f "$plain_2" ]; then
    missing=
    [ -f "$plain_1" ] || missing="plain-1 tty missing: $plain_1"
    [ -f "$plain_2" ] || missing="$missing${missing:+; }plain-2 tty missing: $plain_2"
    deterministic_verdict="deterministic failed $missing"
else
    frame_lines "$plain_2" > "$out_dir/plain-2.frames"
    deterministic_verdict=$(frame_lines "$plain_1" | awk -v other="$out_dir/plain-2.frames" '
        {
            count++
            if ((getline line < other) <= 0) line = "(end of frame lines)"
            if ($0 != line && difference == "") difference = $0 " | " line
        }
        END {
            if (difference == "" && (getline line < other) > 0) difference = "(end of frame lines) | " line
            if (count == 0) print "deterministic failed no psx-frame line"
            else if (difference != "") print "deterministic failed first difference: " difference
            else print "deterministic ok " count " psx-frame lines identical"
        }')
    rm -f "$out_dir/plain-2.frames"
fi

reference_state=given
if [ -z "$reference" ]; then
    reference_state=none
elif [ ! -f "$reference" ]; then
    reference_state=missing
fi

{
    [ -f "$plain_1" ] && frame_lines "$plain_1" | sed 's/^/P /'
    if [ -f "$breakdown" ]; then
        frame_lines "$breakdown" | sed 's/^/B /'
        tr -d '\r' < "$breakdown" | sed -n 's/^psx-draw: /D /p'
    fi
    [ "$reference_state" = given ] && tr -d '\r' < "$reference" | sed 's/^/R /'
} | awk -v frames="$frames" -v budget="$budget" -v run_first="$run_first" -v run_last="$run_last" \
    -v full_offset="$((source_first - first_game))" -v reference_state="$reference_state" -v reference="$reference" \
    -v window_out="$out_dir/window.txt" -v summary_out="$out_dir/summary.txt" -v verdicts_out="$out_dir/verdicts.awk" '
    BEGIN {
        family_count = split("cls pset line rect rectfill circ circfill spr print pattern", families, " ")
        draw_count = 0
    }
    $1 == "P" && $2 == "frame" && NF == 13 { plain_update[$3] = $5; plain_draw[$3] = $7; plain_submit[$3] = $9; next }
    $1 == "B" && $2 == "frame" && NF == 13 { bd_draw[$3] = $7; next }
    $1 == "R" && $2 ~ /^[0-9]+$/ && NF == 7 { reference_update[$2] = $3; reference_draw[$2] = $4; next }
    $1 == "D" && $2 == "clk8-per-64-hblanks" { if (calibration == "") calibration = $3; next }
    $1 == "D" && $2 == "frame" {
        record = $0
        sub(/^D /, "", record)
        if (draw_failure != "") next
        well_formed = NF == 35 && $34 == "wrapped"
        for (i = 1; well_formed && i <= family_count; i++) well_formed = $(1 + 3 * i) == families[i]
        if (!well_formed) { draw_failure = "malformed draw record: " record; next }
        if ($3 != draw_count) {
            draw_failure = draw_count >= frames ? "expected " frames " draw records, got frame " $3 \
                : "expected draw record " draw_count ", got frame " $3
            next
        }
        if ($35 != 0) { draw_failure = "frame " $3 " wrapped"; next }
        in_calls = 0
        for (i = 1; i <= family_count; i++) {
            ticks[$3, families[i]] = $(2 + 3 * i)
            calls[$3, families[i]] = $(3 + 3 * i)
            if (families[i] != "pattern") in_calls += $(2 + 3 * i)
        }
        in_call_ticks[$3] = in_calls
        draw_count++
        next
    }
    function percent(part, whole) { return whole > 0 ? sprintf("%.1f%%", part * 100 / whole) : "-" }
    function signed_percent(value, base) { return base > 0 ? sprintf("%+.1f%%", (value - base) * 100 / base) : "-" }
    function hblanks(fine_ticks) { return fine_ticks * 64 / calibration }
    function within(value, base) { return (value - base) * 10 <= base && (base - value) * 10 <= base }
    END {
        if (reference_state == "none") {
            representative = "representative skipped no reference frame table"
        } else if (reference_state == "missing") {
            representative = "representative failed no reference frame table " reference ": run make psx-full-game-check"
        } else {
            print "run full update reference-update draw reference-draw" > window_out
            for (r = run_first; r <= run_last; r++) {
                f = r + full_offset
                if (!(r in plain_draw)) { lack = "plain-1 has no frame " r; break }
                if (!(f in reference_draw)) { lack = "no reference frame " f " in " reference; break }
                print r, f, plain_update[r], reference_update[f], plain_draw[r], reference_draw[f] > window_out
                draw_sum += plain_draw[r]; reference_draw_sum += reference_draw[f]
                update_sum += plain_update[r]; reference_update_sum += reference_update[f]
            }
            if (lack == "") {
                draw_text = "draw " draw_sum " vs " reference_draw_sum " (" signed_percent(draw_sum, reference_draw_sum) ")"
                update_text = "update " update_sum " vs " reference_update_sum " (" signed_percent(update_sum, reference_update_sum) ")"
            }
            if (lack != "") representative = "representative failed " lack
            else if (!within(draw_sum, reference_draw_sum) && !within(update_sum, reference_update_sum))
                representative = "representative failed " draw_text ", " update_text " outside 10%"
            else if (!within(draw_sum, reference_draw_sum)) representative = "representative failed " draw_text " outside 10%"
            else if (!within(update_sum, reference_update_sum)) representative = "representative failed " update_text " outside 10%"
            else representative = "representative ok " draw_text ", " update_text
        }

        if (calibration == "") breakdown_failure = "no clk8-per-64-hblanks line"
        else if (calibration < 15500 || calibration > 19000)
            breakdown_failure = "clk8-per-64-hblanks " calibration " outside 15500..19000"
        else if (draw_failure != "") breakdown_failure = draw_failure
        else if (draw_count != frames) breakdown_failure = "expected draw record " draw_count ", got end of records"
        else {
            for (r = 0; r < frames; r++) {
                if (!(r in bd_draw)) { breakdown_failure = "no psx-frame line for frame " r; break }
                if (in_call_ticks[r] * 64 > bd_draw[r] * calibration) {
                    breakdown_failure = sprintf("frame %d in calls %.1f hblanks above its draw %d", r, hblanks(in_call_ticks[r]), bd_draw[r])
                    break
                }
            }
        }
        breakdown = breakdown_failure != "" ? "breakdown failed " breakdown_failure \
            : "breakdown ok clk8-per-64-hblanks " calibration ", " frames " draw records"
        print representative > verdicts_out
        print breakdown > verdicts_out

        printf "window run frames %d..%d, full frames %d..%d, %d frames\n", run_first, run_last, \
            run_first + full_offset, run_last + full_offset, run_last - run_first + 1 > summary_out
        plain_lack = ""
        for (r = run_first; r <= run_last; r++) {
            if (!(r in plain_draw)) { plain_lack = "plain-1 has no frame " r; break }
            window_update += plain_update[r]; window_draw += plain_draw[r]; window_submit += plain_submit[r]
        }
        if (plain_lack != "") print "window plain unavailable: " plain_lack > summary_out
        else printf "window plain update %d draw %d submit %d\n", window_update, window_draw, window_submit > summary_out
        unavailable = ""
        if (calibration == "" || calibration <= 0) unavailable = "no calibration"
        for (r = run_first; unavailable == "" && r <= run_last; r++) {
            if (!(r in plain_draw)) unavailable = "plain-1 has no frame " r
            else if (!(r in bd_draw)) unavailable = "breakdown has no frame " r
            else if (!(r in in_call_ticks)) unavailable = "breakdown has no draw record " r
        }
        if (unavailable != "") {
            print "summary unavailable: " unavailable > summary_out
            exit
        }
        for (r = run_first; r <= run_last; r++) {
            breakdown_draw_sum += bd_draw[r]; plain_draw_sum += plain_draw[r]; window_in_calls += in_call_ticks[r]
        }
        printf "draw breakdown boot %.1f hblanks\n", breakdown_draw_sum > summary_out
        for (i = 1; i <= family_count; i++) {
            family_ticks = 0; family_calls = 0
            for (r = run_first; r <= run_last; r++) { family_ticks += ticks[r, families[i]]; family_calls += calls[r, families[i]] }
            if (families[i] == "pattern") {
                printf "pattern %.1f hblanks %d calls %s (inside rectfill and circfill, not added)\n", hblanks(family_ticks), \
                    family_calls, percent(hblanks(family_ticks), breakdown_draw_sum) > summary_out
            } else {
                printf "family %s %.1f hblanks %d calls %s\n", families[i], hblanks(family_ticks), family_calls, \
                    percent(hblanks(family_ticks), breakdown_draw_sum) > summary_out
            }
        }
        printf "in-calls %.1f hblanks %s\n", hblanks(window_in_calls), percent(hblanks(window_in_calls), breakdown_draw_sum) > summary_out
        outside = breakdown_draw_sum - hblanks(window_in_calls)
        printf "outside-calls %.1f hblanks %s\n", outside, percent(outside, breakdown_draw_sum) > summary_out
        printf "overhead %d hblanks (breakdown draw %d - plain draw %d)\n", breakdown_draw_sum - plain_draw_sum, \
            breakdown_draw_sum, plain_draw_sum > summary_out
        split("backend-free game-draw-free draw-free", cases, " ")
        for (r = run_first; r <= run_last; r++) {
            share = bd_draw[r] > 0 ? hblanks(in_call_ticks[r]) / bd_draw[r] : 0
            base = plain_update[r] + plain_submit[r]
            best[1] = base + plain_draw[r] * (1 - share)
            best[2] = base + plain_draw[r] * share
            best[3] = base
            printf "frame %d full %d backend-free %.1f game-draw-free %.1f draw-free %.1f\n", r, r + full_offset, \
                best[1], best[2], best[3] > summary_out
            for (c = 1; c <= 3; c++) {
                if (r == run_first || best[c] > worst[c]) { worst[c] = best[c]; worst_frame[c] = r }
                if (best[c] > budget) over[c]++
            }
        }
        for (c = 1; c <= 3; c++) {
            printf "best %s max %.1f frame %d, %d frames over %d\n", cases[c], worst[c], worst_frame[c], over[c], budget > summary_out
        }
    }'

{
    echo "$done_verdict"
    echo "$deterministic_verdict"
    cat "$out_dir/verdicts.awk"
    echo "$asserts_verdict"
} > "$out_dir/verdicts.txt"
rm -f "$out_dir/verdicts.awk"
cat "$out_dir/verdicts.txt" "$out_dir/summary.txt"
if grep -q '^[a-z]* failed ' "$out_dir/verdicts.txt"; then
    exit 2
fi
exit 0
