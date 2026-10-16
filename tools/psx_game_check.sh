#!/bin/sh
# usage: psx_game_check.sh <tty> <boot-status> <boot-output> <pad-script> <pad-log-tool> <replay-tool>
#                          <difficulty> <out-dir>
# The game EXE's seven conditions after its boot with <pad-script>: game-done,
# game-pad-log, game-pad-states, game-state-game-lines, game-title-left,
# game-asserts (the TTY's "psx-game: asserts on|off" line equals
# PSX_GAME_ASSERTS, on or off, default off), game-gpu-cost (the TTY's
# "psx-game: gpu-peak" figures are no larger than pc_state.txt's metered peaks).
# Every condition is checked before concluding, one "psx-game-check:
# <condition>: ok[, detail]" line each, failures on stderr, exit 1 if any
# failed. Writes into <out-dir>: game_events.json (the TTY's pad log as a
# recording, for make replay), pc_state.txt and pc.dump.txt (its PC replay
# with STATE=), game-state/ (psx_state_compare.sh's outputs), tty.txt (the
# TTY without CR); nothing else there is touched.

set -u

usage="usage: psx_game_check.sh <tty> <boot-status> <boot-output> <pad-script> <pad-log-tool> <replay-tool> <difficulty> <out-dir>"
[ $# -eq 8 ] || { echo "psx-game-check: $usage" >&2; exit 1; }
tty=$1
boot_status=$2
boot_output=$3
pad_script=$4
pad_log_tool=$5
replay_tool=$6
difficulty=$7
out_dir=$8
script_dir=$(cd "$(dirname "$0")" && pwd)
expected_asserts=${PSX_GAME_ASSERTS:-off}
case $expected_asserts in
    on | off) ;;
    *) echo "psx-game-check: PSX_GAME_ASSERTS must be on or off, not '$expected_asserts'" >&2; exit 1 ;;
esac

mkdir -p "$out_dir"
rm -rf "$out_dir/tty.txt" "$out_dir/game_events.json" "$out_dir/pc_state.txt" "$out_dir/pc.dump.txt" "$out_dir/game-state"
[ -f "$tty" ] || : > "$tty"
tr -d '\r' < "$tty" > "$out_dir/tty.txt"

failed=0
pass() {
    echo "psx-game-check: $1: ok${2:+, $2}"
}
report() {
    echo "psx-game-check: $1: $2" >&2
    failed=1
}

done_lines=$(grep '^psx-replay: done ' "$out_dir/tty.txt")
stopped=$(sed -n 's/^psx-state: \(stopped p8_fatal.*\)/\1/p' "$out_dir/tty.txt" | head -n 1)
frames=
set -- $done_lines
if [ "${3:-}" = game ] && [ "${5:-}" = vsyncs ] && [ $# -eq 6 ]; then
    frames=$4
    vsyncs=$6
fi
if [ "$boot_status" -ne 0 ]; then
    report game-done "$boot_output"
elif [ -n "$stopped" ]; then
    report game-done "the game stopped: $stopped"
elif [ -z "$done_lines" ]; then
    report game-done "no done line (psx-replay: done game <frames> vsyncs <n>)"
elif [ -z "$frames" ]; then
    report game-done "expected one psx-replay: done game <frames> vsyncs <n> line, got: $(echo $done_lines)"
else
    pass game-done "$frames frames, $vsyncs vsyncs"
fi

pad_log=$("$pad_log_tool" "$tty" "$pad_script" "$out_dir/game_events.json" 2>&1)
log_line=$(printf '%s\n' "$pad_log" | grep '^psx-pad-log: log: ' | head -n 1)
log_frames=$(printf '%s\n' "$log_line" | sed -n 's/^psx-pad-log: log: ok, \([0-9]*\) frames$/\1/p')
if [ -z "$log_frames" ]; then
    report game-pad-log "${log_line:-$pad_log}"
elif [ "$log_frames" != "$frames" ]; then
    report game-pad-log "$log_frames frames in the log, ${frames:-no} frames in the done line"
    log_frames=
else
    pass game-pad-log "$log_frames frames"
fi

states_line=$(printf '%s\n' "$pad_log" | grep '^psx-pad-log: states: ' | head -n 1)
case $states_line in
    'psx-pad-log: states: ok, '*) pass game-pad-states "${states_line#psx-pad-log: states: ok, }" ;;
    *) report game-pad-states "${states_line:-no states line: the log did not parse}" ;;
esac

if [ -z "$log_frames" ]; then
    report game-state-game-lines "skipped, game-pad-log failed"
elif ! replay=$(STATE="$out_dir/pc_state.txt" "$replay_tool" "$out_dir/game_events.json" /dev/null \
    "$out_dir/pc.dump.txt" 2>&1); then
    report game-state-game-lines "the PC replay failed: $replay"
else
    replayed=$(printf '%s\n' "$replay" | sed -n 's/^replay: \([0-9]*\) frame(s) run.*/\1/p')
    if [ "$replayed" != "$log_frames" ]; then
        report game-state-game-lines "the PC replay ran ${replayed:-no} frames, expected $log_frames"
    elif compare=$(sh "$script_dir/psx_state_compare.sh" "$tty" "$out_dir/pc_state.txt" "$out_dir/game-state" \
        "$log_frames" 2>&1) && printf '%s\n' "$compare" | grep -q '^psx-state-compare: game lines match'; then
        pass game-state-game-lines "$(printf '%s\n' "$compare" | sed -n 's/^psx-state-compare: game lines match (\(.*\))$/\1/p')"
    else
        report game-state-game-lines "$compare"
    fi
fi

difficulty_line=$(sed -n 's/^psx-state: \(difficulty .*\)/\1/p' "$out_dir/tty.txt" | head -n 1)
clock_line=$(sed -n 's/^psx-state: \(clock .*\)/\1/p' "$out_dir/tty.txt" | head -n 1)
found="${difficulty_line:-(no difficulty line)}, ${clock_line:-(no clock line)}"
if [ "$difficulty_line" = "difficulty $difficulty" ] && [ -n "$clock_line" ] \
    && [ "$clock_line" != "clock year - month -" ]; then
    pass game-title-left "$found"
else
    report game-title-left "expected difficulty $difficulty: $found"
fi

asserts_mode=$(sed -n 's/^psx-game: asserts \(.*\)$/\1/p' "$out_dir/tty.txt" | head -n 1)
if [ "$asserts_mode" = "$expected_asserts" ]; then
    pass game-asserts "$asserts_mode"
else
    report game-asserts "psx-game: asserts ${asserts_mode:-(no line)}"
fi

peak_line=$(grep '^psx-game: gpu-peak ' "$out_dir/tty.txt" | head -n 1)
set -- $peak_line
peak_bytes=${4:-}; peak_cluts=${6:-}; peak_patterns=${8:-}; peak_build=${10:-}
# usage: pc_peak <zone|cap> <name>; prints field 4 of that PC state line, nothing when there is none
pc_peak() {
    awk -v kind="$1" -v name="$2" '$1 == kind && $2 == name { print $4; exit }' "$out_dir/pc_state.txt"
}
[ $# -eq 10 ] && [ "$3 $5 $7 $9" = "bytes cluts patterns build" ] || peak_bytes=
case $peak_bytes$peak_cluts$peak_patterns$peak_build in *[!0-9]*) peak_bytes= ;; esac
if [ -z "$peak_bytes" ] || [ "$peak_bytes" -eq 0 ]; then
    report game-gpu-cost "psx-game: gpu-peak (no line)"
elif [ ! -f "$out_dir/pc_state.txt" ]; then
    report game-gpu-cost "skipped, game-state-game-lines failed"
else
    metered_bytes=$(pc_peak zone gpu_frame)
    metered_cluts=$(pc_peak cap gpu_cluts)
    metered_patterns=$(pc_peak cap gpu_patterns)
    metered_build=$(pc_peak zone gpu_build)
    over=
    for figure in "bytes $peak_bytes $metered_bytes zone gpu_frame" "cluts $peak_cluts $metered_cluts cap gpu_cluts" \
        "patterns $peak_patterns $metered_patterns cap gpu_patterns" "build $peak_build $metered_build zone gpu_build"; do
        set -- $figure
        if [ $# -ne 5 ]; then
            over="${over}PC state has no $(echo $figure | awk '{print $(NF - 1), $NF}'); "
        elif [ "$2" -gt "$3" ]; then
            over="$over$1 $2 above the PC's metered $3; "
        fi
    done
    if [ -n "$over" ]; then
        report game-gpu-cost "${over%; }"
    else
        pass game-gpu-cost "bytes $peak_bytes of $metered_bytes metered, cluts $peak_cluts of $metered_cluts, patterns $peak_patterns of $metered_patterns, build $peak_build of $metered_build"
    fi
fi

[ "$failed" -eq 0 ] || exit 1
