#!/bin/sh
# usage: psx_boot.sh <install-dir> [--exe <file> | --iso <cue>] <frames> <out-prefix>
# Boots the input (or nothing) headless for <frames> vsyncs and writes
# <out-prefix>.vram (capture), <out-prefix>.tty.txt (stdout) and
# <out-prefix>.log (stderr). Exit 1 naming the input when the emulator exits
# non-zero, times out (PSX_BOOT_TIMEOUT seconds, default 120) or writes no
# capture.

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

[ $# -ge 3 ] || fail "usage: psx_boot.sh <install-dir> [--exe <file> | --iso <cue>] <frames> <out-prefix>"
install_dir=$(absolute "$1")
shift

input_flag=
input=
label="nothing"
while [ $# -gt 2 ]; do
    case $1 in
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
[ $# -eq 2 ] || fail "usage: psx_boot.sh <install-dir> [--exe <file> | --iso <cue>] <frames> <out-prefix>"
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
rm -f "$prefix.vram" "$prefix.tty.txt" "$prefix.log"

# Config, memory cards and caches land in the working directory or HOME:
# a fresh one per boot keeps two boots of the same input byte-identical.
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

set -- -cli -stdout -softgpu -interpreter -run
[ -z "$input_flag" ] || set -- "$@" "$input_flag" "$input"
status=0
(
    cd "$work"
    unset DISPLAY WAYLAND_DISPLAY
    HOME=$work XDG_CONFIG_HOME=$work/.config \
    PSX_CAPTURE_FRAMES=$frames PSX_CAPTURE_FILE=$prefix.vram \
        timeout "${PSX_BOOT_TIMEOUT:-120}" "$install_dir/pcsx-redux" \
        "$@" -dofile "$script_dir/psx_capture.lua" \
        > "$prefix.tty.txt" 2> "$prefix.log"
) || status=$?

if [ "$status" -eq 124 ]; then
    fail "$label: timed out after ${PSX_BOOT_TIMEOUT:-120} s before frame $frames (see $prefix.log)"
fi
[ "$status" -eq 0 ] || fail "$label: emulator exited with $status (see $prefix.log)"
[ -s "$prefix.vram" ] || fail "$label: no capture written (see $prefix.log)"
echo "psx-boot: $label: $prefix.vram $prefix.tty.txt"
