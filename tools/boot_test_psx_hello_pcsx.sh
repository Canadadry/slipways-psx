set -e

if [ ! -f tools/pcsx-redux/pcsx-redux ]; then
	echo "tools/pcsx-redux/pcsx-redux not found -- run 'make pcsx-redux-setup' first." >&2
	exit 1
fi
if [ ! -f psx-hello/build/psx-hello.cue ]; then
	echo "psx-hello/build/psx-hello.cue not found -- run 'make psx-hello' first." >&2
	exit 1
fi

WORK_DIR=tools/.boot-test-pcsx-run
rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"

export DISPLAY=:95
Xvfb :95 -screen 0 640x480x24 >"$WORK_DIR/xvfb.log" 2>&1 &
XPID=$!
sleep 2

cd tools/pcsx-redux
timeout 15 ./pcsx-redux --no-ui --loadiso ../../psx-hello/build/psx-hello.cue --run \
	>"../../$WORK_DIR/run.log" 2>&1 || true
cd ../..

kill "$XPID" 2>/dev/null || true

if grep -q "psxgpu: setup done" "$WORK_DIR/run.log" && grep -q "PS-X Control PAD Driver" "$WORK_DIR/run.log"; then
	echo "OK: psx-hello booted and reached its own GPU/pad init (see $WORK_DIR/run.log)"
else
	echo "FAIL: psx-hello did not reach GPU/pad init -- see $WORK_DIR/run.log" >&2
	exit 1
fi
