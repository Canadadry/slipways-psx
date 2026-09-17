set -e

if [ ! -f tools/pcsx-redux/pcsx-redux ]; then
	echo "tools/pcsx-redux/pcsx-redux not found -- run 'make pcsx-redux-setup' first." >&2
	exit 1
fi
if [ ! -f psx-slipways/build/slipways.cue ]; then
	echo "psx-slipways/build/slipways.cue not found -- run 'make psx-slipways' first." >&2
	exit 1
fi
if [ ! -f psx-slipways/build/slipways.map ]; then
	echo "psx-slipways/build/slipways.map not found -- run 'make psx-slipways' first." >&2
	exit 1
fi

WORK_DIR=tools/.rules-tech-test-pcsx-slipways-run
rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"

python3 tools/gen_pcsx_slipways_rules_tech_test_lua.py psx-slipways/build/slipways.map "$WORK_DIR/driver.lua"

export DISPLAY=:99
Xvfb :99 -screen 0 640x480x24 >"$WORK_DIR/xvfb.log" 2>&1 &
XPID=$!
sleep 2

cd tools/pcsx-redux
set +e
timeout 60 ./pcsx-redux --no-ui --loadiso ../../psx-slipways/build/slipways.cue --run \
	--dofile "../../$WORK_DIR/driver.lua" \
	>"../../$WORK_DIR/run.log" 2>&1
STATUS=$?
set -e
cd ../..

kill "$XPID" 2>/dev/null || true

if [ "$STATUS" -eq 0 ]; then
	echo "OK: psx-slipways rules_tech Select-toggle assertions passed (see $WORK_DIR/run.log)"
else
	echo "FAIL: psx-slipways rules_tech Select-toggle assertions failed (exit $STATUS) -- see $WORK_DIR/run.log" >&2
	exit "$STATUS"
fi
