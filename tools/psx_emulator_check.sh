#!/bin/sh
# usage: psx_emulator_check.sh <install-dir> <sources> <media-dir> <out-dir> <known-line> <frames>
# Fetches the test media of <sources> into <media-dir> (sha256-checked), boots
# the test .exe twice, the test .cue twice and nothing once, and checks four
# facts. Every fact is checked before concluding, one line per failure, exit 1
# if any failed.

set -u

install_dir=$1
sources=$2
media_dir=$3
out_dir=$4
known_line=$5
frames=$6
script_dir=$(cd "$(dirname "$0")" && pwd)
. "$script_dir/psx_portable.sh"

fail() {
    echo "psx-emulator-check: $*" >&2
    exit 1
}

[ -f "$sources" ] || fail "missing sources list: $sources"
[ -f "$install_dir/psx-emulator.stamp" ] \
    || fail "no complete install in $install_dir; run make psx-emulator-setup"

# Prints "<url> <sha256>" of a file component; fails naming it when absent.
source_of() {
    line=$(awk -v name="$1" '$1 == name && $2 == "file" { print $3, $4; exit }' "$sources")
    [ -n "$line" ] || fail "$1: no file line in $sources"
    echo "$line"
}

# Downloads component $1 as $2 unless already there with the pinned sha256.
fetch() {
    entry=$(source_of "$1") || exit 1
    url=${entry% *}
    pin=${entry#* }
    target=$media_dir/$2
    if [ -f "$target" ] && [ "$(sha256_of "$target")" = "$pin" ]; then
        return
    fi
    rm -f "$target"
    curl -sSfL --connect-timeout 10 --max-time 600 -o "$target" "$url" </dev/null \
        || { rm -f "$target"; fail "$1: download of $url failed"; }
    if [ "$(sha256_of "$target")" != "$pin" ]; then
        rm -f "$target"
        fail "$1: sha256 of $url differs from the pin $pin"
    fi
}

mkdir -p "$media_dir" "$out_dir"
fetch ps1-tests tests.zip
fetch test-cd-bin buzzy.bin
fetch test-cd-cue buzzy.cue
unzip -o -q "$media_dir/tests.zip" gte/test-all/test-all.exe -d "$media_dir" \
    || fail "ps1-tests: tests.zip holds no gte/test-all/test-all.exe"
exe=$media_dir/gte/test-all/test-all.exe
cue=$media_dir/buzzy.cue

boot() {
    name=$1
    shift
    rm -f "$out_dir/$name.vram"
    sh "$script_dir/psx_boot.sh" "$install_dir" "$@" "$frames" "$out_dir/$name"
}

boot exe1 --exe "$exe"
boot exe2 --exe "$exe"
boot cue1 --iso "$cue"
boot cue2 --iso "$cue"
boot empty

failed=0

pass() {
    echo "psx-emulator-check: $1: ok"
}

report() {
    echo "psx-emulator-check: $1: $2" >&2
    failed=1
}

# A capture is missing when its boot failed: every fact needing it fails.
need() {
    for name in "$@"; do
        [ -s "$out_dir/$name.vram" ] || { echo "boot $name wrote no capture"; return 1; }
    done
}

if detail=$(need exe1 exe2); then
    if cmp -s "$out_dir/exe1.vram" "$out_dir/exe2.vram"; then
        pass "exe captures identical"
    else
        report "exe captures identical" "exe1.vram and exe2.vram differ"
    fi
else
    report "exe captures identical" "$detail"
fi

if [ -s "$out_dir/exe1.vram" ]; then
    if grep -qF -- "$known_line" "$out_dir/exe1.tty.txt"; then
        pass "known line in exe TTY"
    else
        report "known line in exe TTY" "\"$known_line\" not in exe1.tty.txt"
    fi
else
    report "known line in exe TTY" "boot exe1 failed"
fi

if detail=$(need cue1 cue2); then
    if cmp -s "$out_dir/cue1.vram" "$out_dir/cue2.vram"; then
        pass "cue captures identical"
    else
        report "cue captures identical" "cue1.vram and cue2.vram differ"
    fi
else
    report "cue captures identical" "$detail"
fi

if detail=$(need cue1 empty); then
    if cmp -s "$out_dir/cue1.vram" "$out_dir/empty.vram"; then
        report "cue capture differs from empty boot" "cue1.vram and empty.vram are identical"
    else
        pass "cue capture differs from empty boot"
    fi
else
    report "cue capture differs from empty boot" "$detail"
fi

[ "$failed" -eq 0 ] || exit 1
for name in exe1 exe2 cue1 cue2 empty; do
    echo "psx-emulator-check: $(sha256_of "$out_dir/$name.vram")  $out_dir/$name.vram"
done
echo "psx-emulator-check: all four conditions hold"
