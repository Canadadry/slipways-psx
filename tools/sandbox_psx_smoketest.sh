set -e

CC=mipsel-linux-gnu-gcc-12
OBJCOPY=mipsel-linux-gnu-objcopy
READELF=mipsel-linux-gnu-readelf

BUILD_DIR=tools/.sandbox-psx-smoketest
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

PSN00BSDK_FLAGS="-ffreestanding -fno-builtin -nostdlib -fdata-sections -ffunction-sections -fsigned-char -fno-strict-overflow -msoft-float -march=r3000 -mtune=r3000 -mabi=32 -mno-mt -mno-llsc -G0 -fno-pic -mno-abicalls -mno-gpopt"

cat > "$BUILD_DIR/smoke.c" <<'EOF'
/* Freestanding stand-in for psx-hello/main.c: this only exercises the
 * compiler backend and the PS-EXE pipeline, not psx-hello's actual logic
 * (the real headers it needs aren't available in this sandbox). */
volatile unsigned int *const GPU_STATUS = (unsigned int *) 0x1f801814;

void _boot(void) {
	for (;;) {
		*GPU_STATUS = 0;
	}
}
EOF

cat > "$BUILD_DIR/link.ld" <<'EOF'
ENTRY(_boot)
SECTIONS {
	. = 0x80010000;
	.text : { *(.text) }
}
EOF

"$CC" $PSN00BSDK_FLAGS -c "$BUILD_DIR/smoke.c" -o "$BUILD_DIR/smoke.o"
"$CC" $PSN00BSDK_FLAGS -nostdlib -Wl,-T,"$BUILD_DIR/link.ld" "$BUILD_DIR/smoke.o" -o "$BUILD_DIR/smoke.elf"

echo "== readelf -h (expect Machine: MIPS R3000, mips1, o32) =="
"$READELF" -h "$BUILD_DIR/smoke.elf" | grep -E "Machine|Flags"

ENTRY_HEX=$("$READELF" -h "$BUILD_DIR/smoke.elf" | awk '/Entry point address/ {print $NF}')

"$OBJCOPY" -O binary "$BUILD_DIR/smoke.elf" "$BUILD_DIR/smoke.bin"
CODE_SIZE=$(wc -c < "$BUILD_DIR/smoke.bin")

python3 tools/psx_exe_header.py \
	--pc0 "$ENTRY_HEX" \
	--t-addr "$ENTRY_HEX" \
	--t-size "$CODE_SIZE" \
	-o "$BUILD_DIR/smoke.exe" \
	"$BUILD_DIR/smoke.bin"

EXE_SIZE=$(wc -c < "$BUILD_DIR/smoke.exe")
MAGIC=$(dd if="$BUILD_DIR/smoke.exe" bs=1 count=8 2>/dev/null)

if [ "$MAGIC" != "PS-X EXE" ]; then
	echo "FAIL: bad PS-EXE magic in $BUILD_DIR/smoke.exe"
	exit 1
fi
if [ "$EXE_SIZE" -le 2048 ]; then
	echo "FAIL: $BUILD_DIR/smoke.exe is not larger than the 2048-byte header"
	exit 1
fi

echo "OK: PS-EXE magic present, header+code size $EXE_SIZE bytes"
echo "OK: sandbox smoke test passed -- see docs/psx-hello-world.md for what this does and doesn't prove"
