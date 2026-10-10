#!/bin/sh
# usage: install_psx_emulator.sh <install-dir> <sources>
# Builds PCSX-Redux and its OpenBIOS into <install-dir>, from the pins of
# <sources>. Linux: build dependencies through apt (sudo). macOS: through the
# user's Homebrew, no sudo; the OpenBIOS is built with the mipsel-none-elf GCC
# of PSN00BSDK_DIR (make psn00bsdk-toolchain-setup first). Exit 0 at once when <install-dir> holds a stamp for the same
# pins; exit 1, deleting nothing, when it is non-empty without that stamp.

set -eu

install_dir=$1
sources=$2
BUILD_TIMEOUT=2700
STAMP=psx-emulator.stamp

script_dir=$(cd "$(dirname "$0")" && pwd)
. "$script_dir/psx_portable.sh"

fail() {
    echo "psx-emulator-setup: $*" >&2
    exit 1
}

[ -f "$sources" ] || fail "missing sources list: $sources"

# Prints "<url> <pin>" of a git component; fails naming it when absent.
source_of() {
    line=$(awk -v name="$1" '$1 == name && $2 == "git" { print $3, $4; exit }' "$sources")
    [ -n "$line" ] || fail "$1: no git line in $sources"
    echo "$line"
}

base=$(source_of emulator)
fix=$(source_of emulator-headless-fix)
emulator_url=${base% *}
base_commit=${base#* }
fix_url=${fix% *}
fix_commit=${fix#* }

stamp_text="emulator $base_commit
emulator-headless-fix $fix_commit"

if [ -f "$install_dir/$STAMP" ] && [ "$(cat "$install_dir/$STAMP")" = "$stamp_text" ]; then
    echo "psx-emulator-setup: already installed in $install_dir"
    exit 0
fi
if [ -d "$install_dir" ] && [ -n "$(ls -A "$install_dir")" ]; then
    fail "$install_dir is not empty and holds no stamp for these pins; remove it and re-run"
fi

if [ "$(uname -s)" = Darwin ]; then
    command -v brew >/dev/null 2>&1 || fail "Homebrew not found: the macOS build takes its libraries from it"
    sdk_bin=$(cd "${PSN00BSDK_DIR:-third_party/psn00bsdk}" 2>/dev/null && pwd)/bin
    [ -x "$sdk_bin/mipsel-none-elf-gcc" ] || fail "no mipsel-none-elf-gcc in $sdk_bin: run make psn00bsdk-toolchain-setup first"
    PATH=$sdk_bin:$PATH
    export PATH
    brew install pkg-config capstone freetype glfw libuv ffmpeg curl zlib
    PKG_CONFIG_PATH=$(brew --prefix)/lib/pkgconfig:$(brew --prefix zlib)/lib/pkgconfig:$(brew --prefix curl)/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}
    export PKG_CONFIG_PATH
elif [ -n "${PSX_EMULATOR_SKIP_APT:-}" ]; then
    echo "psx-emulator-setup: PSX_EMULATOR_SKIP_APT set, skipping apt"
else
    sudo apt-get update
    sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
        build-essential pkg-config libcapstone-dev libfreetype-dev \
        libglfw3-dev libavcodec-dev libavformat-dev libavutil-dev \
        libswresample-dev libcurl4-openssl-dev libuv1-dev zlib1g-dev \
        gcc-mipsel-linux-gnu g++-mipsel-linux-gnu
fi

mkdir -p "$install_dir"
cd "$install_dir"
export GIT_LFS_SKIP_SMUDGE=1 GIT_TERMINAL_PROMPT=0

git init -q .
git fetch -q --depth 1 "$emulator_url" "$base_commit"
git checkout -q --detach FETCH_HEAD
git submodule update --init --recursive --depth 1 -j 8 \
    || git submodule update --init --recursive -j 8

# Without the fix, -cli segfaults on a disc boot (src/gpu/soft/gpu.cc:87).
git fetch -q --depth 2 "$fix_url" "$fix_commit"
patch=$(mktemp)
git diff "$fix_commit^" "$fix_commit" > "$patch"
if ! git apply "$patch"; then
    rm -f "$patch"
    fail "fix $fix_commit does not apply cleanly on $base_commit"
fi
rm -f "$patch"

run_with_timeout "$BUILD_TIMEOUT" make -j "$(cpu_count)" pcsx-redux
run_with_timeout "$BUILD_TIMEOUT" make -j "$(cpu_count)" -C src/mips/openbios

# The emulator finds its BIOS at src/mips/openbios/openbios.bin next to the
# binary when none is configured (System::findResource).
[ -x pcsx-redux ] || fail "build gave no pcsx-redux binary"
[ -f src/mips/openbios/openbios.bin ] || fail "build gave no openbios.bin"

echo "$stamp_text" > "$STAMP"
echo "psx-emulator-setup: installed in $install_dir"
