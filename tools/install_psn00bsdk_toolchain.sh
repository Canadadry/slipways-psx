
set -e

BINUTILS_VERSION=2.40
GCC_VERSION=12.2.0
PREFIX="${1:-/usr/local/mipsel-none-elf}"
BUILD_TIMEOUT=2700

sudo apt-get update
# texinfo (makeinfo) is needed even with --disable-docs: this binutils
# version still runs the doc/bfd.info target unconditionally and dies with
# "makeinfo: not found" (exit 127) otherwise.
sudo apt-get install -y build-essential make wget texinfo

work_dir="$(mktemp -d)"
cd "$work_dir"

# Pinned to a specific mirror rather than ftpmirror.gnu.org: its random
# mirror selection landed on hosts that reset the connection or 404'd from
# inside a sandboxed session's egress proxy; this one was verified reachable.
wget "https://mirrors.nju.edu.cn/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz"
wget "https://mirrors.nju.edu.cn/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz"
tar xf "binutils-${BINUTILS_VERSION}.tar.xz"
tar xf "gcc-${GCC_VERSION}.tar.xz"
rm -f ./*.tar.xz

( cd "gcc-${GCC_VERSION}" && ./contrib/download_prerequisites )

mkdir binutils-build
cd binutils-build
../"binutils-${BINUTILS_VERSION}"/configure \
	--prefix="$PREFIX" --target=mipsel-none-elf \
	--disable-docs --disable-nls --disable-werror --with-float=soft
timeout "$BUILD_TIMEOUT" make -j "$(nproc)"
sudo make install-strip
cd ..

mkdir gcc-build
cd gcc-build
../"gcc-${GCC_VERSION}"/configure \
	--prefix="$PREFIX" --target=mipsel-none-elf \
	--disable-docs --disable-nls --disable-werror --disable-libada \
	--disable-libssp --disable-libquadmath --disable-threads \
	--disable-libgomp --disable-libstdcxx-pch --disable-hosted-libstdcxx \
	--enable-languages=c,c++ --without-isl --without-headers \
	--with-float=soft --with-gnu-as --with-gnu-ld
timeout "$BUILD_TIMEOUT" make -j "$(nproc)"
sudo make install-strip
cd ..

echo "Toolchain installed under ${PREFIX}/bin -- add it to PATH:"
echo "  export PATH=\$PATH:${PREFIX}/bin"

sudo apt-get install -y ninja-build cmake

git clone https://github.com/Lameguy64/PSn00bSDK.git
cd PSn00bSDK
git submodule update --init --recursive
cmake --preset default .
timeout "$BUILD_TIMEOUT" cmake --build ./build
sudo cmake --install ./build
cd ..

rm -rf "$work_dir"

echo "PSn00bSDK installed. Build psx-hello with:"
echo "  cmake -S psx-hello -B psx-hello/build"
echo "  cmake --build psx-hello/build"
