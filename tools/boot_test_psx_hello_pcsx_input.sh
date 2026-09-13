set -e

if [ ! -f tools/pcsx-redux/pcsx-redux ]; then
	echo "tools/pcsx-redux/pcsx-redux not found -- run 'make pcsx-redux-setup' first." >&2
	exit 1
fi
if [ ! -f psx-hello/build/psx-hello.cue ]; then
	echo "psx-hello/build/psx-hello.cue not found -- run 'make psx-hello' first." >&2
	exit 1
fi
if [ ! -f psx-hello/build/psx-hello.map ]; then
	echo "psx-hello/build/psx-hello.map not found -- run 'make psx-hello' first." >&2
	exit 1
fi

WORK_DIR=tools/.input-test-pcsx-run
rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"

python3 tools/gen_pcsx_hello_input_test_lua.py psx-hello/build/psx-hello.map "$WORK_DIR/driver.lua"

export DISPLAY=:96
Xvfb :96 -screen 0 640x480x24 >"$WORK_DIR/xvfb.log" 2>&1 &
XPID=$!
sleep 2

cd tools/pcsx-redux
set +e
timeout 60 ./pcsx-redux --no-ui --loadiso ../../psx-hello/build/psx-hello.cue --run \
	--dofile "../../$WORK_DIR/driver.lua" \
	>"../../$WORK_DIR/run.log" 2>&1
STATUS=$?
set -e
cd ../..

kill "$XPID" 2>/dev/null || true

if [ "$STATUS" -eq 0 ]; then
	echo "OK: psx-hello input-driven assertions passed (see $WORK_DIR/run.log)"
else
	echo "FAIL: psx-hello input-driven assertions failed (exit $STATUS) -- see $WORK_DIR/run.log" >&2
	exit "$STATUS"
fi
