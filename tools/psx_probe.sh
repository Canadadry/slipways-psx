#!/bin/sh
# usage: psx_probe.sh <run> <baseline-tty> <probe-tty> <map> <frames> <out-dir>
# Analyzes one run of make psx-probe: the probe build's boot of unpatched HEAD
# (baseline) and of HEAD + patch (probe), the run's map.txt and its frame
# count. Writes <out-dir>/probe.txt and prints it: the window sums of draw,
# update, submit and total, baseline -> probe, the game-state comparison and,
# when the probe used the delay helper, delta / delay; or a single
# "<run> invalid <reason>" line. Exits 0 when valid, 2 when invalid, 1 on a
# usage error.

set -u

usage="usage: psx_probe.sh <run> <baseline-tty> <probe-tty> <map> <frames> <out-dir>"
[ $# -eq 6 ] || { echo "psx-probe: $usage" >&2; exit 1; }
run=$1
baseline=$2
probe=$3
map=$4
frames=$5
out_dir=$6
[ -n "$run" ] || { echo "psx-probe: empty run name" >&2; exit 1; }
[ -f "$map" ] || { echo "psx-probe: missing map: $map" >&2; exit 1; }
case $frames in
    '' | *[!0-9]*) echo "psx-probe: frames is not a count: $frames" >&2; exit 1 ;;
esac

set -- $(head -n 1 "$map")
if [ $# -ne 9 ] || [ "$1" != cut ] || [ "$3" != first-game-frame ] || [ "$5" != source-first ] || [ "$7" != window ]; then
    echo "psx-probe: malformed map: $map" >&2
    exit 1
fi
first_game=$4
source_first=$6
run_first=$((first_game + $8 - source_first))
run_last=$((first_game + $9 - source_first))
if [ "$run_first" -lt "$first_game" ] || [ "$run_last" -lt "$run_first" ] || [ "$run_last" -ge "$frames" ]; then
    echo "psx-probe: window $8..$9 is run frames $run_first..$run_last, outside $first_game..$((frames - 1))" >&2
    exit 1
fi

mkdir -p "$out_dir" || exit 1
report=$out_dir/probe.txt

# usage: boot_reason <side> <tty>; why the boot cannot be measured, empty when it ran to its end with asserts off
boot_reason() {
    if [ ! -f "$2" ]; then
        echo "$1 tty missing: $2"
        return
    fi
    stopped=$(tr -d '\r' < "$2" | grep '^psx-state: stopped' | head -n 1)
    done_line=$(tr -d '\r' < "$2" | grep '^psx-replay: done ' | tail -n 1)
    asserts_line=$(tr -d '\r' < "$2" | grep '^psx-replay: asserts ' | head -n 1)
    set -- "$1" $done_line
    if [ -n "$stopped" ]; then
        echo "$1 stopped: $stopped"
    elif [ "${5:-}" != "$frames" ]; then
        echo "$1 expected $frames frames, got: ${done_line:-(no done line)}"
    elif [ -z "$asserts_line" ]; then
        echo "$1 has no asserts line"
    elif [ "$asserts_line" != "psx-replay: asserts off" ]; then
        echo "$1 says ${asserts_line#psx-replay: }"
    fi
}

{
    [ -f "$baseline" ] && tr -d '\r' < "$baseline" | sed 's/^/B /'
    [ -f "$probe" ] && tr -d '\r' < "$probe" | sed 's/^/P /'
} | awk -v run="$run" -v frames="$frames" -v run_first="$run_first" -v run_last="$run_last" \
    -v baseline_boot="$(boot_reason baseline "$baseline")" -v probe_boot="$(boot_reason probe "$probe")" '
    $2 == "psx-frame:" && $3 == "frame" {
        line = $0
        sub(/^. psx-frame: /, "", line)
        well_formed = NF == 14 && $5 == "update" && $7 == "draw" && $9 == "submit" && $11 == "total" && $13 == "vsyncs"
        for (i = 4; well_formed && i <= 14; i += 2) well_formed = $i ~ /^[0-9]+$/
        frame_line[$1, $4] = well_formed ? "" : line
        update[$1, $4] = $6; draw[$1, $4] = $8; submit[$1, $4] = $10
        next
    }
    $2 == "psx-probe:" && $3 == "clk8-per-64-hblanks" { if (!(($1) in calibration)) calibration[$1] = $4; next }
    $1 == "P" && $2 == "psx-probe:" && $3 == "delay" {
        delay_line = $0
        sub(/^P /, "", delay_line)
        if (NF == 6 && $5 == "calls" && $4 ~ /^[0-9]+$/ && $6 ~ /^[0-9]+$/ && $4 > 0 && $6 > 0) { delay = $4; delay_calls = $6 }
        else delay_malformed = 1
        next
    }
    $2 == "psx-state:" && $3 !~ /^(zone|budget|cap)$/ {
        line = $0
        sub(/^. psx-state: /, "", line)
        state[$1, ++state_count[$1]] = line
    }
    # usage: frames_reason(side, name, first, last); the first frame of first..last without a well-formed line
    function frames_reason(side, name, first, last,    r) {
        for (r = first; r <= last; r++) {
            if (!((side, r) in frame_line)) return name " has no psx-frame line for frame " r
            if (frame_line[side, r] != "") return name " malformed frame line: " frame_line[side, r]
        }
        return ""
    }
    function side_reason(side, name, boot,    reason) {
        if (boot != "") return boot
        reason = frames_reason(side, name, run_first, run_last)
        if (reason != "") return reason
        if ((side in calibration) && (calibration[side] !~ /^[0-9]+$/ || calibration[side] < 15500 || calibration[side] > 19000))
            return name " clk8-per-64-hblanks " calibration[side] " outside 15500..19000"
        if (side != "P" || delay_line == "") return ""
        if (!("P" in calibration)) return "probe has a delay line but no clk8-per-64-hblanks line"
        if (delay_malformed) return "probe malformed delay line: " delay_line
        reason = frames_reason("B", "baseline", 0, frames - 1)
        return reason != "" ? reason : frames_reason("P", "probe", 0, frames - 1)
    }
    function part(name, base, probed) {
        return sprintf("%s %s %d -> %d %+d hblanks %s", run, name, base, probed, probed - base,
                       base_total > 0 ? sprintf("%+.2f%%", (probed - base) * 100 / base_total) : "-%")
    }
    END {
        invalid = side_reason("B", "baseline", baseline_boot)
        if (invalid == "") invalid = side_reason("P", "probe", probe_boot)
        if (invalid != "") {
            print run " invalid " invalid
            exit
        }
        for (r = run_first; r <= run_last; r++) {
            for (s = 1; s <= 2; s++) {
                side = s == 1 ? "B" : "P"
                sum_update[side] += update[side, r]; sum_draw[side] += draw[side, r]; sum_submit[side] += submit[side, r]
            }
        }
        for (s = 1; s <= 2; s++) {
            side = s == 1 ? "B" : "P"
            sum_total[side] = sum_update[side] + sum_draw[side] + sum_submit[side]
        }
        base_total = sum_total["B"]
        printf "%s valid window run frames %d..%d (%d frames)\n", run, run_first, run_last, run_last - run_first + 1
        print part("draw", sum_draw["B"], sum_draw["P"]) " of window total"
        print part("update", sum_update["B"], sum_update["P"])
        print part("submit", sum_submit["B"], sum_submit["P"])
        print part("total", sum_total["B"], sum_total["P"])
        state_last = state_count["B"] > state_count["P"] ? state_count["B"] : state_count["P"]
        difference = ""
        for (i = 1; difference == "" && i <= state_last; i++) {
            base_line = i <= state_count["B"] ? state["B", i] : "(end of game-state lines)"
            probe_line = i <= state_count["P"] ? state["P", i] : "(end of game-state lines)"
            if (base_line != probe_line) difference = base_line " | " probe_line
        }
        print run " game-state " (difference == "" ? "same" : "differs first difference: " difference)
        if (delay == "") exit
        for (r = 0; r < frames; r++) {
            run_delta += update["P", r] + draw["P", r] + submit["P", r] - update["B", r] - draw["B", r] - submit["B", r]
        }
        per_run = run_delta / delay
        printf "%s delay %d hblanks calls %d delta/delay run %.2f (%+.1f%% vs calls) window per frame %.2f\n", run, delay,
            delay_calls, per_run, (per_run - delay_calls) * 100 / delay_calls,
            (sum_total["P"] - sum_total["B"]) / delay / (run_last - run_first + 1)
    }' > "$report"
cat "$report"
case $(head -n 1 "$report") in
    "$run invalid "*) exit 2 ;;
esac
exit 0
