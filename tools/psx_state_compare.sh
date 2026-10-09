#!/bin/sh
# usage: psx_state_compare.sh <tty> <pc-state> <out-dir> <expected-frames>
# Compares the `psx-state: ` lines of a PSX replay's TTY with the PC state of
# the same recording, every line but zone/budget/cap, in order. Writes
# <out-dir>/psx_state.txt (the MIPS state) and <out-dir>/measurements.txt
# (its zone/budget/cap lines and the vsync count). Every condition is checked
# before concluding, one line per failure, exit 1 if any failed.

set -u

usage="usage: psx_state_compare.sh <tty> <pc-state> <out-dir> <expected-frames>"
[ $# -eq 4 ] || { echo "psx-state-compare: $usage" >&2; exit 1; }
tty=$1
pc_state=$2
out_dir=$3
expected_frames=$4
[ -f "$tty" ] || { echo "psx-state-compare: missing tty: $tty" >&2; exit 1; }
[ -f "$pc_state" ] || { echo "psx-state-compare: missing pc state: $pc_state" >&2; exit 1; }

mkdir -p "$out_dir"
psx_state=$out_dir/psx_state.txt
measurements=$out_dir/measurements.txt
psx_game=$out_dir/psx_game_lines.txt
pc_game=$out_dir/pc_game_lines.txt

failed=0
report() {
    echo "psx-state-compare: $*" >&2
    failed=1
}

tr -d '\r' < "$tty" > "$out_dir/tty.txt"
sed -n 's/^psx-state: //p' "$out_dir/tty.txt" > "$psx_state"
done_line=$(grep '^psx-replay: done ' "$out_dir/tty.txt" | tail -n 1)

measured='^(zone|budget|cap) '
grep -E "$measured" "$psx_state" > "$measurements"
grep -Ev "$measured" "$psx_state" > "$psx_game"
grep -Ev "$measured" "$pc_state" > "$pc_game"

if [ -z "$done_line" ]; then
    report "no done line (psx-replay: done ...): the replay did not finish"
else
    set -- $done_line
    if [ "$4" != "$expected_frames" ]; then
        report "done after $4 frames, expected $expected_frames: $done_line"
    fi
    [ "${5:-}" = vsyncs ] && echo "vsyncs ${6:-}" >> "$measurements"
fi

stopped=$(grep '^stopped p8_fatal' "$psx_state" | head -n 1)
[ -n "$stopped" ] && report "the PSX replay stopped: $stopped"

if [ ! -s "$psx_state" ]; then
    report "no psx-state line in $tty"
else
    difference=$(awk '
        FILENAME == ARGV[1] { pc[FNR] = $0; pc_count = FNR; next }
        { psx[FNR] = $0; psx_count = FNR }
        END {
            count = pc_count > psx_count ? pc_count : psx_count
            for (i = 1; i <= count; i++) {
                left = i <= psx_count ? psx[i] : "(end of state)"
                right = i <= pc_count ? pc[i] : "(end of state)"
                if (left != right) {
                    printf "first differing game line, %d:\n  psx: %s\n  pc:  %s\n", i, left, right
                    exit
                }
            }
        }' "$pc_game" "$psx_game")
    if [ -n "$difference" ]; then
        report "$difference"
    else
        echo "psx-state-compare: game lines match ($(wc -l < "$pc_game" | tr -d ' ') lines)"
    fi
fi

exit $failed
