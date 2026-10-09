#!/bin/sh
# usage: psx_pad_compare.sh <pad-script> <tty>
# Compares the ordered "psx-pad: <hex4>" states the pad probe printed with
# the states the pad script holds, and requires the probe's done line.
# A pad script line is "<vsync offset> <button>..." or "<vsync offset> -"
# (nothing held); offsets count from the probe's handshake and increase; the
# last step is "-". Blank lines and "#" lines are skipped. Exit 1 on a bad
# script or any difference, naming it.

set -u

usage="usage: psx_pad_compare.sh <pad-script> <tty>"
[ $# -eq 2 ] || { echo "psx-pad-compare: $usage" >&2; exit 1; }
pad_script=$1
tty=$2
[ -f "$pad_script" ] || { echo "psx-pad-compare: missing pad script: $pad_script" >&2; exit 1; }
[ -f "$tty" ] || { echo "psx-pad-compare: missing tty: $tty" >&2; exit 1; }

# The names and bits are PCSX.CONSTS.PAD.BUTTON's, which tools/psx_pad_play.lua
# plays: hardware bit order, no L3/R3 (a digital pad).
expected=$(awk '
    BEGIN {
        split("SELECT - - START UP RIGHT DOWN LEFT L2 R2 L1 R1 TRIANGLE CIRCLE CROSS SQUARE", names, " ")
        for (i = 1; i <= 16; i++) if (names[i] != "-") bit[names[i]] = 2 ^ (i - 1)
        previous_state = 0
        previous_offset = -1
    }
    function bad(message) {
        printf "error %s line %d: %s\n", FILENAME, FNR, message
        failed = 1
        exit 1
    }
    { sub(/\r$/, "") }
    /^[ \t]*(#|$)/ { next }
    {
        if ($1 !~ /^[0-9]+$/) bad("vsync offset is not an integer: " $1)
        if ($1 + 0 <= previous_offset) bad("vsync offset " $1 " does not increase")
        previous_offset = $1 + 0
        if (NF < 2) bad("no button set (use - for none)")
        state = 0
        if (!(NF == 2 && $2 == "-")) {
            for (i = 2; i <= NF; i++) {
                if (!($i in bit)) bad("unknown button " $i)
                if (int(state / bit[$i]) % 2 == 0) state += bit[$i]
            }
        }
        steps++
        last_state = state
        if (state != previous_state) printf "%04x\n", state
        previous_state = state
    }
    END {
        if (failed) exit 1
        if (steps == 0) { printf "error %s: no step\n", FILENAME; exit 1 }
        if (last_state != 0) { printf "error %s: last step must be - (no button held)\n", FILENAME; exit 1 }
    }' "$pad_script") || {
    echo "psx-pad-compare: bad pad script: $(printf '%s\n' "$expected" | sed -n 's/^error //p')" >&2
    exit 1
}

failed=0
report() {
    echo "psx-pad-compare: $*" >&2
    failed=1
}

clean_tty=$(tr -d '\r' < "$tty")
seen=$(printf '%s\n' "$clean_tty" | sed -n 's/^psx-pad: \([0-9a-f]\{4\}\)$/\1/p')
printf '%s\n' "$clean_tty" | grep -q '^psx-pad: done$' \
    || report "no done line (psx-pad: done): the probe did not see the script's end with no button held"

difference=$(awk -v seen="$seen" -v expected="$expected" '
    BEGIN {
        seen_count = seen == "" ? 0 : split(seen, probe, "\n")
        expected_count = split(expected, script, "\n")
        count = seen_count > expected_count ? seen_count : expected_count
        for (i = 1; i <= count; i++) {
            left = i <= seen_count ? probe[i] : "(none)"
            right = i <= expected_count ? script[i] : "(none)"
            if (left != right) {
                printf "first differing pad state, %d:\n  probe:  %s\n  script: %s\n", i, left, right
                exit
            }
        }
    }')
if [ -n "$difference" ]; then
    report "$difference"
elif [ "$failed" -eq 0 ]; then
    echo "psx-pad-compare: pad states match ($(printf '%s\n' "$expected" | wc -l | tr -d ' ') states)"
fi

exit $failed
