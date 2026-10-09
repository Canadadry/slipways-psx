#!/bin/sh
# usage: psx_hello_check.sh <install-dir> <--exe|--iso> <file> <known-line> <frames> <out-dir>
# Boots the input twice and nothing once, and checks that psx-hello ran. Every
# condition is checked before concluding, one line per failure, exit 1 if any
# failed.

set -u

fail() {
    echo "psx-hello-check: $*" >&2
    exit 1
}

usage="usage: psx_hello_check.sh <install-dir> <--exe|--iso> <file> <known-line> <frames> <out-dir>"
[ $# -eq 6 ] || fail "$usage"
install_dir=$1
input_flag=$2
input=$3
known_line=$4
frames=$5
out_dir=$6
case $input_flag in
    --exe|--iso) ;;
    *) fail "$usage" ;;
esac
[ -f "$input" ] || fail "missing input: $input"
script_dir=$(cd "$(dirname "$0")" && pwd)

rm -rf "$out_dir"
mkdir -p "$out_dir"

boot() {
    name=$1
    shift
    sh "$script_dir/psx_boot.sh" "$install_dir" "$@" "$frames" "$out_dir/$name"
}

boot run1 "$input_flag" "$input"
boot run2 "$input_flag" "$input"
boot empty

failed=0

pass() {
    echo "psx-hello-check: $1: ok"
}

report() {
    echo "psx-hello-check: $1: $2" >&2
    failed=1
}

# A capture is missing when its boot failed: every condition needing it fails.
need() {
    for name in "$@"; do
        [ -s "$out_dir/$name.vram" ] || { echo "boot $name wrote no capture"; return 1; }
    done
}

# Prints the number of distinct pixel values, capped at 2.
distinct_pixels() {
    header=$(head -n 1 "$1")
    set -- "$1" $header
    case $5 in
        16) size=2 ;;
        24) size=3 ;;
        *) echo 0; return ;;
    esac
    tail -c +$((${#header} + 2)) "$1" | od -An -v -tx1 -w$size | uniq | head -n 2 | wc -l
}

# Prints the .bin named by the .cue's FILE line, resolved next to the .cue;
# prints nothing when there is no FILE line.
cue_bin() {
    name=$(sed -n 's/^[[:space:]]*FILE[[:space:]]*"\([^"]*\)".*/\1/p' "$input" | head -n 1)
    [ -n "$name" ] && echo "$(dirname "$input")/$name"
}

if [ "$input_flag" = --exe ]; then
    if [ "$(head -c 8 "$input")" = "PS-X EXE" ]; then
        pass magic
    else
        report magic "$input does not start with PS-X EXE (elf2x failed?)"
    fi
else
    bin=$(cue_bin)
    if [ -z "$bin" ]; then
        report cue "$input has no FILE \"<bin>\" line"
    elif [ ! -s "$bin" ]; then
        report cue "$bin, named by $input, is missing or empty"
    elif [ $(($(wc -c <"$bin") % 2352)) -ne 0 ]; then
        report cue "$bin is $(wc -c <"$bin") bytes, not a whole number of 2352-byte sectors"
    else
        pass cue
    fi
fi

if detail=$(need run1 run2); then
    if cmp -s "$out_dir/run1.vram" "$out_dir/run2.vram"; then
        pass deterministic
    else
        report deterministic "run1.vram and run2.vram differ"
    fi
else
    report deterministic "$detail"
fi

if detail=$(need run1 empty); then
    if cmp -s "$out_dir/run1.vram" "$out_dir/empty.vram"; then
        report differs-from-empty "run1.vram and empty.vram are identical"
    else
        pass differs-from-empty
    fi
else
    report differs-from-empty "$detail"
fi

if detail=$(need run1); then
    if [ "$(distinct_pixels "$out_dir/run1.vram")" -ge 2 ]; then
        pass not-uniform
    else
        report not-uniform "run1.vram holds one pixel value, or an unknown depth: $(head -n 1 "$out_dir/run1.vram")"
    fi
else
    report not-uniform "$detail"
fi

if detail=$(need run1); then
    if grep -qF -- "$known_line" "$out_dir/run1.tty.txt"; then
        pass tty-line
    else
        report tty-line "\"$known_line\" not in run1.tty.txt"
    fi
else
    report tty-line "$detail"
fi

[ "$failed" -eq 0 ] || exit 1
echo "psx-hello-check: $(sha256sum "$input")"
# The .bin hash changes on every build (mkpsxiso stamps the build time): never compare it.
[ "$input_flag" = --iso ] && echo "psx-hello-check: $(sha256sum "$bin")"
for name in run1 run2 empty; do
    echo "psx-hello-check: $(head -n 1 "$out_dir/$name.vram") $(sha256sum "$out_dir/$name.vram")"
done
echo "psx-hello-check: all five conditions hold"
