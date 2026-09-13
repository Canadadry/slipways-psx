set -e


CUE="$1"
OUT_PNG="$2"
FRAMES="${3:-90}"

if [ -z "$CUE" ] || [ -z "$OUT_PNG" ]; then
	echo "usage: capture_psx_screenshot_pcsx.sh <cue-path> <out-png-path> [frames]" >&2
	exit 1
fi
if [ ! -f tools/pcsx-redux/pcsx-redux ]; then
	echo "tools/pcsx-redux/pcsx-redux not found -- run 'make pcsx-redux-setup' first." >&2
	exit 1
fi
if [ ! -f "$CUE" ]; then
	echo "$CUE not found -- build the project first." >&2
	exit 1
fi

WORK_DIR=tools/.screenshot-pcsx-run
rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"
mkdir -p "$(dirname "$OUT_PNG")"

CUE_ABS="$(cd "$(dirname "$CUE")" && pwd)/$(basename "$CUE")"
RAW_OUT="$(pwd)/$WORK_DIR/capture.raw"

export DISPLAY=:97
Xvfb :97 -screen 0 640x480x24 >"$WORK_DIR/xvfb.log" 2>&1 &
XPID=$!
sleep 2

cd tools/pcsx-redux
set +e
PCSX_SCREENSHOT_FRAMES="$FRAMES" PCSX_SCREENSHOT_OUT="$RAW_OUT" \
	timeout 30 ./pcsx-redux --no-ui --loadiso "$CUE_ABS" --run \
	--dofile "../pcsx_screenshot_driver.lua" \
	>"../../$WORK_DIR/run.log" 2>&1
STATUS=$?
set -e
cd ../..

kill "$XPID" 2>/dev/null || true

if [ "$STATUS" -ne 0 ] || [ ! -f "$WORK_DIR/capture.raw" ]; then
	echo "FAIL: screenshot capture did not complete (exit $STATUS) -- see $WORK_DIR/run.log" >&2
	exit 1
fi

python3 tools/pcsx_raw_to_png.py "$WORK_DIR/capture.raw" "$OUT_PNG"

echo "OK: screenshot saved to $OUT_PNG (see $WORK_DIR/run.log)"
