#!/bin/sh
# usage: install_psn00bsdk_toolchain.sh <install-dir> <sources>
# Installs the pinned PSn00bSDK release of <sources> into <install-dir>. On
# macOS, which has no release, or with PSN00BSDK_FROM_SOURCE set, builds it
# instead (tools/build_psn00bsdk.sh).
# Exit 0 at once when <install-dir> holds a stamp for the same sha256;
# exit 1, deleting nothing, when it is non-empty without that stamp.

set -eu

install_dir=$1
sources=$2
STAMP=psn00bsdk.stamp
GCC_VERSION=12.3.0

script_dir=$(cd "$(dirname "$0")" && pwd)
if [ "$(uname -s)" = Darwin ] || [ -n "${PSN00BSDK_FROM_SOURCE:-}" ]; then
    exec sh "$script_dir/build_psn00bsdk.sh" "$install_dir" "$sources"
fi
. "$script_dir/psx_portable.sh"

fail() {
    echo "psn00bsdk-toolchain-setup: $*" >&2
    exit 1
}

[ -f "$sources" ] || fail "missing sources list: $sources"

line=$(awk '$1 == "psn00bsdk" && $2 == "file" { print $3, $4; exit }' "$sources")
[ -n "$line" ] || fail "psn00bsdk: no file line in $sources"
url=${line% *}
sha256=${line#* }

# The stamp holds the sha256 so that a new pin never trusts an old install.
if [ -f "$install_dir/$STAMP" ] && [ "$(cat "$install_dir/$STAMP")" = "$sha256" ]; then
    echo "psn00bsdk-toolchain-setup: already installed in $install_dir"
    exit 0
fi
if [ -d "$install_dir" ] && [ -n "$(ls -A "$install_dir")" ]; then
    fail "$install_dir is not empty and holds no stamp for this pin; remove it and re-run"
fi

if [ -n "${PSN00BSDK_SKIP_APT:-}" ]; then
    echo "psn00bsdk-toolchain-setup: PSN00BSDK_SKIP_APT set, skipping apt"
else
    sudo apt-get update
    sudo DEBIAN_FRONTEND=noninteractive apt-get install -y cmake make unzip curl
fi

mkdir -p tmp "$install_dir"
install_dir=$(cd "$install_dir" && pwd)
download_dir=$(mktemp -d "$PWD/tmp/psn00bsdk-download.XXXXXX")
unpack_dir=$(mktemp -d "$(dirname "$install_dir")/.psn00bsdk-unpack.XXXXXX")
trap 'rm -rf "$download_dir" "$unpack_dir"' EXIT

archive=$download_dir/psn00bsdk.zip
curl -fsSL --retry 2 -o "$archive" "$url" || fail "psn00bsdk: download failed: $url"
actual=$(sha256_of "$archive")
[ "$actual" = "$sha256" ] || fail "psn00bsdk: sha256 $actual, expected $sha256"

unzip -q "$archive" -d "$unpack_dir"
[ "$(ls -A "$unpack_dir" | wc -l)" -eq 1 ] || fail "psn00bsdk: archive does not hold exactly one top-level directory"
top=$unpack_dir/$(ls -A "$unpack_dir")
[ -d "$top" ] || fail "psn00bsdk: archive's top-level entry is not a directory"
rmdir "$install_dir"
mv "$top" "$install_dir"

gcc_version=$("$install_dir/bin/mipsel-none-elf-gcc" -dumpversion) \
    || fail "installed mipsel-none-elf-gcc does not run"
[ "$gcc_version" = "$GCC_VERSION" ] || fail "mipsel-none-elf-gcc is $gcc_version, expected $GCC_VERSION"
for tool in elf2x mkpsxiso; do
    [ -x "$install_dir/bin/$tool" ] || fail "$tool missing from $install_dir/bin"
done

echo "$sha256" > "$install_dir/$STAMP"
echo "psn00bsdk-toolchain-setup: installed in $install_dir"
