#!/bin/sh
# usage: psx_boot.sh <install-dir> [--until-exit] [--exe <file> | --iso <cue>] <frames> <out-prefix>
# Boots the input (or nothing) headless for <frames> vsyncs and writes
# <out-prefix>.vram (capture), <out-prefix>.tty.txt (stdout) and
# <out-prefix>.log (stderr). Exit 1 naming the input when the emulator exits
# non-zero, times out (PSX_BOOT_TIMEOUT seconds, default 120) or writes no
# capture.
# --until-exit adds -testmode and writes no capture: the program's own exit
# code is the result (0 passes, anything else fails naming it), and <frames>
# is a vsync ceiling; reaching it or the timeout fails naming the input and
# the last "psx-replay:" or "psx-pad:" line of the TTY.

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

[ $# -ge 3 ] || fail "usage: psx_boot.sh <install-dir> [--until-exit] [--exe <file> | --iso <cue>] <frames> <out-prefix>"
install_dir=$(absolute "$1")
shift

input_flag=
input=
label="nothing"
until_exit=
while [ $# -gt 2 ]; do
    case $1 in
        --until-exit)
            until_exit=1
            shift
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
[ $# -eq 2 ] || fail "usage: psx_boot.sh <install-dir> [--until-exit] [--exe <file> | --iso <cue>] <frames> <out-prefix>"
frames=$1
prefix=$(absolute "$2")
case $frames in
    '' | *[!0-9]* | 0) fail "frame count must be a positive integer: $frames" ;;
esac

[ -f "$install_dir/$STAMP" ] && [ -x "$install_dir/pcsx-redux" ] \
    && [ -f "$install_dir/src/mips/openbios/openbios.bin" ] \
    || fail "no complete install in $install_dir; run make psx-emulator-setup"

script_dir=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$(dirname "$prefix")"
rm -f "$prefix.vram" "$prefix.tty.txt" "$prefix.log" "$prefix.result"

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
    PSX_CEILING_FRAMES=$frames PSX_RESULT_FILE=$prefix.result \
        timeout "${PSX_BOOT_TIMEOUT:-120}" "$install_dir/pcsx-redux" \
        "$@" -dofile "$script_dir/$lua" \
        > "$prefix.tty.txt" 2> "$prefix.log"
) || status=$?

if [ -n "$until_exit" ]; then
    progress=$(tr -d '\r' < "$prefix.tty.txt" | grep -E '^psx-(replay|pad): ' | tail -n 1)
    last="last progress line: ${progress:-(none)}"
    if [ "$status" -eq 124 ]; then
        fail "$label: timed out after ${PSX_BOOT_TIMEOUT:-120} s before exiting; $last (see $prefix.log)"
    fi
    result=$(cat "$prefix.result" 2>/dev/null || true)
    rm -f "$prefix.result"
    case $result in
        ceiling*) fail "$label: reached the ceiling of $frames vsyncs before exiting; $last (see $prefix.log)" ;;
        exit*) ;;
        *) fail "$label: emulator exited with $status without a result; $last (see $prefix.log)" ;;
    esac
    [ "$status" -eq 0 ] || fail "$label: program exited with code $status; $last (see $prefix.log)"
    echo "psx-boot: $label: exited with 0 after ${result#exit } vsyncs, $prefix.tty.txt"
    exit 0
fi

if [ "$status" -eq 124 ]; then
    fail "$label: timed out after ${PSX_BOOT_TIMEOUT:-120} s before frame $frames (see $prefix.log)"
fi
[ "$status" -eq 0 ] || fail "$label: emulator exited with $status (see $prefix.log)"
[ -s "$prefix.vram" ] || fail "$label: no capture written (see $prefix.log)"
echo "psx-boot: $label: $prefix.vram $prefix.tty.txt"
