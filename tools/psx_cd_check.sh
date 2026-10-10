#!/bin/sh
# usage: psx_cd_check.sh <cue> <exe> <disc-name> <dumpsxiso> <out-dir>
# Checks a CD image after the build, without the emulator: cue (the .cue names
# a .bin of whole 2352-byte sectors) and cd-exe (<disc-name>, extracted by
# <dumpsxiso> into <out-dir>/disc, is byte-identical to <exe>). Both are
# checked before concluding, one line per condition, exit 1 if any failed.

set -u

usage="usage: psx_cd_check.sh <cue> <exe> <disc-name> <dumpsxiso> <out-dir>"
[ $# -eq 5 ] || { echo "psx-cd-check: $usage" >&2; exit 2; }
cue=$1
exe=$2
disc_name=$3
dumpsxiso=$4
out_dir=$5
script_dir=$(cd "$(dirname "$0")" && pwd)

failed=0

pass() {
    echo "psx-cd-check: $1: ok, $2"
}

report() {
    echo "psx-cd-check: $1: $2" >&2
    failed=1
}

cue_ok=0
if bin=$(sh "$script_dir/psx_cue_check.sh" "$cue"); then
    cue_ok=1
    pass cue "$bin, $(($(wc -c <"$bin") / 2352)) sectors"
else
    report cue "$bin"
fi

# Prints why <disc-name> on the disc is not <exe>; prints nothing when it is.
cd_exe_problem() {
    disc=$out_dir/disc
    if [ ! -x "$dumpsxiso" ]; then
        echo "$dumpsxiso is missing: run make psn00bsdk-toolchain-setup"
        return
    fi
    rm -rf "$disc"
    mkdir -p "$out_dir"
    "$dumpsxiso" -x "$disc" "$bin" > "$out_dir/dumpsxiso.out.txt" 2> "$out_dir/dumpsxiso.err.txt"
    status=$?
    if [ "$status" -ne 0 ]; then
        # dumpsxiso 2.03 prints its ERROR line on stdout.
        reason=$(grep -v '^[[:space:]]*$' "$out_dir/dumpsxiso.err.txt" | tail -n 1)
        [ -n "$reason" ] || reason=$(grep '^ERROR' "$out_dir/dumpsxiso.out.txt" | tail -n 1)
        echo "$dumpsxiso exited with $status: ${reason:-no message}"
        return
    fi
    extracted=$disc/$disc_name
    if [ ! -f "$extracted" ]; then
        echo "no $disc_name on the disc; extracted: $(LC_ALL=C ls "$disc" | tr '\n' ' ' | sed 's/ $//')"
        return
    fi
    difference=$(cmp "$extracted" "$exe" 2>&1) && return
    byte=$(echo "$difference" | sed -n 's/.* differ: [a-z]* \([0-9]*\).*/\1/p')
    if [ -n "$byte" ]; then
        echo "$extracted differs from $exe at byte $byte"
    else
        echo "$extracted differs from $exe: $difference"
    fi
}

if [ "$cue_ok" -eq 0 ]; then
    report cd-exe "skipped: cue failed"
else
    problem=$(cd_exe_problem)
    if [ -n "$problem" ]; then
        report cd-exe "$problem"
    else
        pass cd-exe "$disc_name, $(wc -c <"$exe" | tr -d ' ') bytes"
    fi
fi

[ "$failed" -eq 0 ] || exit 1
