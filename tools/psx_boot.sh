#!/bin/sh
# usage: psx_boot.sh <install-dir> [--until-exit [--pad-script <file>] [--capture-script <file>] [--save-state-at <vsync> <file> | --load-state <file>]] [--exe <file> | --iso <cue>] <frames> <out-prefix>
# Boots the input (or nothing) headless for <frames> vsyncs and writes
# <out-prefix>.vram (capture), <out-prefix>.tty.txt (stdout) and
# <out-prefix>.log (stderr). Exit 1 naming the input when the emulator exits
# non-zero, times out (PSX_BOOT_TIMEOUT seconds, default 120) or writes no
# capture.
# --until-exit adds -testmode and writes no capture: the program's own exit
# code is the result (0 passes, anything else fails naming it), and <frames>
# is a vsync ceiling; reaching it or the timeout fails naming the input and
# the last "psx-replay:", "psx-pad:", "psx-game:" or "psx-largest:" line of
# the TTY.
# tools/psx_until_exit.lua writes the vsync count to <out-prefix>.progress
# every 60 vsyncs; an emulator killed by a signal fails naming the status, the
# signal, that line and the last vsync, as the timeout does. The emulator's
# output is unbuffered (stdbuf) when the host has it, so an abort keeps the TTY.
# --save-state-at writes the emulator's save state to <file> at that vsync
# (below the ceiling) and carries on; the boot fails if the program exits
# first. --load-state boots from <file>, loaded at the first vsync: the vsync
# count, so the ceiling and the result, start at the load.
# --pad-script plays that pad script on port 1 (tools/psx_pad_play.lua,
# format in tools/psx_pad_compare.sh); a bad script fails naming its line.
# --capture-script drives the GPU replay's frame-locked captures
# (tools/psx_capture_frames.lua, "<frame> <mask hex>" lines) and writes
# <out-prefix>.f<frame>.vram for each; a bad script fails naming its line.

set -eu

STAMP=psx-emulator.stamp

fail() {
    echo "psx-boot: $*" >&2
    exit 1
}

absolute() {
    case $1 in
        /*) echo "$1" ;;
        *) echo "$PWD/$1" ;;
    esac
}

usage="usage: psx_boot.sh <install-dir> [--until-exit [--pad-script <file>] [--capture-script <file>] [--save-state-at <vsync> <file> | --load-state <file>]] [--exe <file> | --iso <cue>] <frames> <out-prefix>"
[ $# -ge 3 ] || fail "$usage"
install_dir=$(absolute "$1")
shift

input_flag=
input=
label="nothing"
until_exit=
pad_script=
capture_script=
load_state=
save_state_at=
save_state=
while [ $# -gt 2 ]; do
    case $1 in
        --until-exit)
            until_exit=1
            shift
            ;;
        --pad-script)
            [ -f "$2" ] || fail "missing pad script: $2"
            pad_script=$(absolute "$2")
            shift 2
            ;;
        --capture-script)
            [ -f "$2" ] || fail "missing capture script: $2"
            capture_script=$(absolute "$2")
            shift 2
            ;;
        --save-state-at)
            [ $# -ge 5 ] || fail "$usage"
            case $2 in
                '' | *[!0-9]* | 0) fail "--save-state-at: vsync must be a positive integer: $2" ;;
            esac
            save_state_at=$2
            save_state=$(absolute "$3")
            shift 3
            ;;
        --load-state)
            [ -f "$2" ] || fail "--load-state: missing save state: $2"
            load_state=$(absolute "$2")
            shift 2
            ;;
        --exe | --iso)
            [ -z "$input_flag" ] || fail "--exe and --iso are mutually exclusive"
            [ -f "$2" ] || fail "missing input: $2"
            input=$(absolute "$2")
            if [ "$1" = --exe ]; then input_flag=-loadexe; else input_flag=-iso; fi
            label=$input
            shift 2
            ;;
        *) fail "unknown argument: $1" ;;
    esac
done
[ $# -eq 2 ] || fail "$usage"
[ -z "$pad_script" ] || [ -n "$until_exit" ] || fail "--pad-script needs --until-exit"
[ -z "$capture_script" ] || [ -n "$until_exit" ] || fail "--capture-script needs --until-exit"
[ -z "$load_state" ] || [ -n "$until_exit" ] || fail "--load-state needs --until-exit"
[ -z "$save_state" ] || [ -n "$until_exit" ] || fail "--save-state-at needs --until-exit"
[ -z "$save_state" ] || [ -z "$load_state" ] || fail "--save-state-at and --load-state are mutually exclusive"
frames=$1
prefix=$(absolute "$2")
case $frames in
    '' | *[!0-9]* | 0) fail "frame count must be a positive integer: $frames" ;;
esac
[ -z "$save_state" ] || [ "$save_state_at" -lt "$frames" ] \
    || fail "--save-state-at: vsync $save_state_at is not below the ceiling of $frames vsyncs"

[ -f "$install_dir/$STAMP" ] && [ -x "$install_dir/pcsx-redux" ] \
    && [ -f "$install_dir/src/mips/openbios/openbios.bin" ] \
    || fail "no complete install in $install_dir; run make psx-emulator-setup"

script_dir=$(cd "$(dirname "$0")" && pwd)
. "$script_dir/psx_portable.sh"
mkdir -p "$(dirname "$prefix")"
[ -z "$save_state" ] || { mkdir -p "$(dirname "$save_state")" && rm -f "$save_state"; }
rm -f "$prefix.vram" "$prefix.tty.txt" "$prefix.log" "$prefix.result" "$prefix.progress" "$prefix".f*.vram

# Config, memory cards and caches land in the working directory or HOME:
# a fresh one per boot keeps two boots of the same input byte-identical.
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

set -- -cli -stdout -softgpu -interpreter -run
[ -z "$until_exit" ] || set -- "$@" -testmode
[ -z "$input_flag" ] || set -- "$@" "$input_flag" "$input"
if [ -n "$until_exit" ]; then
    lua=psx_until_exit.lua
else
    lua=psx_capture.lua
fi
status=0
(
    cd "$work"
    unset DISPLAY WAYLAND_DISPLAY
    HOME=$work XDG_CONFIG_HOME=$work/.config \
    PSX_CAPTURE_FRAMES=$frames PSX_CAPTURE_FILE=$prefix.vram \
    PSX_CEILING_FRAMES=$frames PSX_RESULT_FILE=$prefix.result PSX_PROGRESS_FILE=$prefix.progress \
    PSX_PAD_SCRIPT=$pad_script PSX_PAD_PLAYER=$script_dir/psx_pad_play.lua \
    PSX_CAPTURE_SCRIPT=$capture_script PSX_CAPTURE_PLAYER=$script_dir/psx_capture_frames.lua \
    PSX_CAPTURE_PREFIX=$prefix PSX_LOAD_STATE_FILE=$load_state \
    PSX_SAVE_STATE_AT=$save_state_at PSX_SAVE_STATE_FILE=$save_state \
        run_with_timeout "${PSX_BOOT_TIMEOUT:-120}" $(unbuffered_prefix) "$install_dir/pcsx-redux" \
        "$@" -dofile "$script_dir/$lua" \
        > "$prefix.tty.txt" 2> "$prefix.log"
) || status=$?

if [ -n "$until_exit" ]; then
    progress=$(tr -d '\r' < "$prefix.tty.txt" | grep -E '^psx-(replay|pad|game|largest): ' | tail -n 1)
    last="last progress line: ${progress:-(none)}"
    vsync=$(sed -n 's/^vsync //p' "$prefix.progress" 2>/dev/null | tail -n 1)
    last_vsync="last vsync: ${vsync:-(none)}"
    if [ "$status" -eq 124 ]; then
        fail "$label: timed out after ${PSX_BOOT_TIMEOUT:-120} s before exiting; $last; $last_vsync (see $prefix.log)"
    fi
    result=$(cat "$prefix.result" 2>/dev/null || true)
    rm -f "$prefix.result"
    if [ -z "$result" ] && [ "$status" -gt 128 ]; then
        signal=$(kill -l "$((status - 128))" 2>/dev/null) && signal=SIG${signal#SIG} || signal=$((status - 128))
        fail "$label: emulator aborted with status $status (signal $signal); $last; $last_vsync (see $prefix.log)"
    fi
    case $result in
        ceiling*) fail "$label: reached the ceiling of $frames vsyncs before exiting; $last (see $prefix.log)" ;;
        exit*) ;;
        error*) fail "$label: ${result#error }" ;;
        *) fail "$label: emulator exited with $status without a result; $last (see $prefix.log)" ;;
    esac
    [ "$status" -eq 0 ] || fail "$label: program exited with code $status; $last (see $prefix.log)"
    [ -z "$save_state" ] || [ -s "$save_state" ] \
        || fail "$label: exited after ${result#exit } vsyncs, before the save state at vsync $save_state_at"
    echo "psx-boot: $label: exited with 0 after ${result#exit } vsyncs, $prefix.tty.txt"
    exit 0
fi

if [ "$status" -eq 124 ]; then
    fail "$label: timed out after ${PSX_BOOT_TIMEOUT:-120} s before frame $frames (see $prefix.log)"
fi
[ "$status" -eq 0 ] || fail "$label: emulator exited with $status (see $prefix.log)"
[ -s "$prefix.vram" ] || fail "$label: no capture written (see $prefix.log)"
echo "psx-boot: $label: $prefix.vram $prefix.tty.txt"
