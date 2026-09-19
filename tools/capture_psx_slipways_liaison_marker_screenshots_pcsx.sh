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

WORK_DIR=tools/.liaison-marker-screenshot-pcsx-run
rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"

python3 tools/gen_pcsx_slipways_liaison_marker_screenshot_lua.py \
	psx-slipways/build/slipways.map "$WORK_DIR/driver.lua"

run_stage() {
	STAGE="$1"
	NAME="$2"
	export DISPLAY=:98
	Xvfb :98 -screen 0 640x480x24 >"$WORK_DIR/xvfb-$NAME.log" 2>&1 &
	XPID=$!
	sleep 2

	cd tools/pcsx-redux
	set +e
	PCSX_LIAISON_MARKER_STAGE="$STAGE" PCSX_LIAISON_MARKER_OUT="../../$WORK_DIR/$NAME.raw" \
		timeout 60 ./pcsx-redux --no-ui --loadiso ../../psx-slipways/build/slipways.cue --run \
		--dofile "../../$WORK_DIR/driver.lua" \
		>"../../$WORK_DIR/run-$NAME.log" 2>&1
	STATUS=$?
	set -e
	cd ../..

	kill "$XPID" 2>/dev/null || true

	if [ "$STATUS" -ne 0 ]; then
		echo "FAIL: liaison marker screenshot stage $NAME failed (exit $STATUS) -- see $WORK_DIR/run-$NAME.log" >&2
		exit "$STATUS"
	fi
}

run_stage 1 a_valid
run_stage 2 b_too_far
run_stage 3 c_blocked

python3 tools/pcsx_raw_to_png.py "$WORK_DIR/a_valid.raw" "$WORK_DIR/a_valid.png"
python3 tools/pcsx_raw_to_png.py "$WORK_DIR/b_too_far.raw" "$WORK_DIR/b_too_far.png"
python3 tools/pcsx_raw_to_png.py "$WORK_DIR/c_blocked.raw" "$WORK_DIR/c_blocked.png"

echo "OK: liaison marker screenshots saved to $WORK_DIR/{a_valid,b_too_far,c_blocked}.png (see $WORK_DIR/run-*.log)"
