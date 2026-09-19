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

WORK_DIR=tools/.tech-icon-screenshot-pcsx-run
rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"

python3 tools/gen_pcsx_slipways_tech_icon_screenshot_lua.py \
	psx-slipways/build/slipways.map "$WORK_DIR/driver.lua"

export DISPLAY=:96
Xvfb :96 -screen 0 640x480x24 >"$WORK_DIR/xvfb.log" 2>&1 &
XPID=$!
sleep 2

cd tools/pcsx-redux
set +e
PCSX_TECH_ICON_OUT="../../$WORK_DIR/tech_menu_open.raw" \
	timeout 60 ./pcsx-redux --no-ui --loadiso ../../psx-slipways/build/slipways.cue --run \
	--dofile "../../$WORK_DIR/driver.lua" \
	>"../../$WORK_DIR/run.log" 2>&1
STATUS=$?
set -e
cd ../..

kill "$XPID" 2>/dev/null || true

if [ "$STATUS" -ne 0 ] || [ ! -f "$WORK_DIR/tech_menu_open.raw" ]; then
	echo "FAIL: tech-menu-icon screenshot capture did not complete (exit $STATUS) -- see $WORK_DIR/run.log" >&2
	exit 1
fi

python3 tools/pcsx_raw_to_png.py "$WORK_DIR/tech_menu_open.raw" "$WORK_DIR/tech_menu_open.png"

echo "OK: tech-menu-icon screenshot saved to $WORK_DIR/tech_menu_open.png (see $WORK_DIR/run.log)"
