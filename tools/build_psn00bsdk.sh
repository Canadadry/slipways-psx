#!/bin/sh
# usage: build_psn00bsdk.sh <install-dir> <sources>
# Builds PSn00bSDK from the pins of <sources> into <install-dir>, the layout
# of the Linux release, for macOS (which has no release) or with
# PSN00BSDK_FROM_SOURCE. Its mipsel-none-elf toolchain is, on macOS, the
# prebuilt arm64 GCC 14.2.0 of toolchain-macos; with PSN00BSDK_FROM_SOURCE,
# binutils and GCC (with libgcc) built from source, 30 to 60 min.
# Needs a host C/C++ compiler, make, cmake, curl, git, unzip; never sudo.
# Exit 0 at once when <install-dir> holds a stamp for the same pins; exit 1,
# deleting nothing, when it is non-empty without that stamp.

set -eu

install_dir=$1
sources=$2
STAMP=psn00bsdk.stamp
TARGET=mipsel-none-elf

script_dir=$(cd "$(dirname "$0")" && pwd)
. "$script_dir/psx_portable.sh"

fail() {
    echo "psn00bsdk-toolchain-setup: $*" >&2
    exit 1
}

[ -f "$sources" ] || fail "missing sources list: $sources"

# Prints "<url> <pin>" of a component of <kind>; fails naming it when absent.
source_of() {
    line=$(awk -v name="$1" -v kind="$2" '$1 == name && $2 == kind { print $3, $4; exit }' "$sources")
    [ -n "$line" ] || fail "$1: no $2 line in $sources"
    echo "$line"
}

sdk=$(source_of psn00bsdk-source git)
if [ "$(uname -s)" = Darwin ] && [ -z "${PSN00BSDK_FROM_SOURCE:-}" ]; then
    prebuilt=$(source_of toolchain-macos file)
    stamp_text="toolchain-macos ${prebuilt#* }"
else
    prebuilt=
    binutils=$(source_of binutils file)
    gcc=$(source_of gcc file)
    stamp_text="binutils ${binutils#* }
gcc ${gcc#* }"
fi
stamp_text="$stamp_text
psn00bsdk-source ${sdk#* }"

if [ -f "$install_dir/$STAMP" ] && [ "$(cat "$install_dir/$STAMP")" = "$stamp_text" ]; then
    echo "psn00bsdk-toolchain-setup: already installed in $install_dir"
    exit 0
fi
if [ -d "$install_dir" ] && [ -n "$(ls -A "$install_dir")" ]; then
    fail "$install_dir is not empty and holds no stamp for these pins; remove it and re-run"
fi
for tool in cc c++ make cmake curl git tar xz unzip; do
    command -v "$tool" >/dev/null 2>&1 || fail "$tool not found on PATH"
done

mkdir -p tmp "$install_dir"
install_dir=$(cd "$install_dir" && pwd)
work=$(mktemp -d "$PWD/tmp/psn00bsdk-build.XXXXXX")
trap 'rm -rf "$work"' EXIT
jobs=$(cpu_count)
PATH=$install_dir/bin:$PATH
export PATH

# Downloads "<url> <sha256>" into the work directory, checks it and prints
# the archive's path.
fetch() {
    url=${1% *}
    sha256=${1#* }
    archive=$work/$(basename "$url")
    curl -fsSL --retry 3 -o "$archive" "$url" || fail "download failed: $url"
    actual=$(sha256_of "$archive")
    [ "$actual" = "$sha256" ] || fail "$(basename "$url"): sha256 $actual, expected $sha256"
    echo "$archive"
}

# Unpacks a fetched tarball and prints the unpacked directory.
fetch_tarball() {
    archive=$(fetch "$1")
    tar -xf "$archive" -C "$work"
    echo "$work/$(basename "$archive" | sed 's/\.tar\..*$//')"
}

if [ -n "$prebuilt" ]; then
    echo "psn00bsdk-toolchain-setup: installing the prebuilt macOS toolchain"
    unzip -q "$(fetch "$prebuilt")" -d "$install_dir"
    rm -rf "$install_dir/__MACOSX"
else
    # The macOS SDK zlib is used: the copy bundled with binutils and GCC
    # does not build against it.
    zlib=
    [ "$(uname -s)" = Darwin ] && zlib=--with-system-zlib

    echo "psn00bsdk-toolchain-setup: building binutils"
    binutils_src=$(fetch_tarball "$binutils")
    mkdir "$work/binutils-build"
    (
        cd "$work/binutils-build"
        "$binutils_src/configure" --prefix="$install_dir" --target=$TARGET \
            --disable-docs --disable-nls --disable-werror --with-float=soft $zlib
        make -j "$jobs" MAKEINFO=true
        make install-strip MAKEINFO=true
    ) > "$work/binutils.log" 2>&1 || { tail -n 40 "$work/binutils.log" >&2; fail "binutils build failed"; }

    echo "psn00bsdk-toolchain-setup: building GCC (the longest part)"
    gcc_src=$(fetch_tarball "$gcc")
    (cd "$gcc_src" && ./contrib/download_prerequisites) > "$work/gcc-prerequisites.log" 2>&1 \
        || { tail -n 20 "$work/gcc-prerequisites.log" >&2; fail "GCC prerequisites download failed"; }
    mkdir "$work/gcc-build"
    (
        cd "$work/gcc-build"
        "$gcc_src/configure" --prefix="$install_dir" --target=$TARGET \
            --disable-docs --disable-nls --disable-werror --disable-libada --disable-libssp \
            --disable-libquadmath --disable-threads --disable-libgomp --disable-libstdcxx-pch \
            --disable-hosted-libstdcxx --enable-languages=c,c++ --without-isl --without-headers \
            --with-float=soft --with-gnu-as --with-gnu-ld $zlib
        make -j "$jobs" all-gcc MAKEINFO=true
        make install-strip-gcc MAKEINFO=true
        make -j "$jobs" all-target-libgcc MAKEINFO=true
        make install-strip-target-libgcc MAKEINFO=true
    ) > "$work/gcc.log" 2>&1 || { tail -n 40 "$work/gcc.log" >&2; fail "GCC build failed (log tail above)"; }
fi

echo "psn00bsdk-toolchain-setup: building PSn00bSDK"
sdk_src=$work/psn00bsdk
(
    git init -q "$sdk_src"
    cd "$sdk_src"
    GIT_TERMINAL_PROMPT=0 git fetch -q --depth 1 "${sdk% *}" "${sdk#* }"
    git checkout -q --detach FETCH_HEAD
    git submodule update --init --recursive --depth 1 -q || git submodule update --init --recursive -q
    cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DSKIP_EXAMPLES=ON \
        -DLIBPSN00B_GENERATOR="Unix Makefiles" -DPSN00BSDK_TC="$install_dir" \
        -DCMAKE_INSTALL_PREFIX="$install_dir"
    cmake --build build -j "$jobs"
    cmake --install build
) > "$work/psn00bsdk.log" 2>&1 || { tail -n 40 "$work/psn00bsdk.log" >&2; fail "PSn00bSDK build failed"; }

"$install_dir/bin/$TARGET-gcc" -dumpversion >/dev/null || fail "installed $TARGET-gcc does not run"
for tool in elf2x mkpsxiso dumpsxiso; do
    [ -x "$install_dir/bin/$tool" ] || fail "$tool missing from $install_dir/bin"
done
[ -f "$install_dir/lib/libpsn00b/cmake/sdk.cmake" ] || fail "lib/libpsn00b/cmake/sdk.cmake missing from $install_dir"

echo "$stamp_text" > "$install_dir/$STAMP"
echo "psn00bsdk-toolchain-setup: built and installed in $install_dir"
