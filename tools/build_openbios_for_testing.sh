set -e

BOOT_MODE="${1:-}"
SRC_DIR=tools/.openbios-build

rm -rf "$SRC_DIR"
git clone --depth 1 https://github.com/grumpycoders/pcsx-redux.git "$SRC_DIR"
git -C "$SRC_DIR" submodule update --init --depth 1 third_party/uC-sdk

PATH="/usr/local/mipsel-none-elf/bin:$PATH"
export PATH

cd "$SRC_DIR/src/mips/openbios"
if [ -n "$BOOT_MODE" ]; then
	make BOOT_MODE="$BOOT_MODE"
else
	make
fi

mkdir -p /root/.mednafen/firmware
cp openbios.bin /root/.mednafen/firmware/scph5500.bin
echo "Installed openbios.bin (BOOT_MODE=${BOOT_MODE:-default}) as /root/.mednafen/firmware/scph5500.bin"
