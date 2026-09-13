set -e

PATH="/usr/games:$PATH"
export PATH
export DISPLAY=:97
export SDL_AUDIODRIVER=dummy

if [ ! -f psx-hello/build/psx-hello.cue ]; then
	echo "psx-hello/build/psx-hello.cue not found -- run 'make psx-hello' first." >&2
	exit 1
fi

WORK_DIR=tools/.boot-test-run
OUT_DIR=tools/.boot-test-snaps
rm -rf "$WORK_DIR" "$OUT_DIR"
mkdir -p "$WORK_DIR" "$OUT_DIR"

python3 tools/patch_exe_stack_for_sideload.py psx-hello/build/psx-hello.exe \
	-o "$WORK_DIR/psx-hello-sideload.exe"

cd "$WORK_DIR"

Xvfb :97 -screen 0 640x480x24 >xvfb.log 2>&1 &
XPID=$!
sleep 2

touch marker

mednafen -force_module psx -video.fs 0 -psx.bios_sanity 0 -sound.driver dummy \
	-psx.input.port1 gamepad \
	-psx.dbg_exe_cdpath ../../psx-hello/build/psx-hello.cue \
	psx-hello-sideload.exe >run.log 2>&1 &
MPID=$!

sleep 5
WIN=$(xdotool search --name "" | tail -1)
xdotool windowfocus --sync "$WIN"
sleep 0.5

echo "1: boot state" >>run.log
xdotool key --window "$WIN" F9
sleep 1.5

echo "2: after 8x D-pad up-left (cursor should move if mouse mode is on)" >>run.log
i=0
while [ $i -lt 8 ]; do
	xdotool key --window "$WIN" w
	sleep 0.05
	xdotool key --window "$WIN" a
	sleep 0.05
	i=$((i + 1))
done
xdotool key --window "$WIN" F9
sleep 1.5

echo "3: after Circle (should show CLICK! and a yellow cursor tile, if mouse mode is on)" >>run.log
xdotool key --window "$WIN" KP_6
xdotool key --window "$WIN" F9
sleep 1.5

echo "4: after Cross (mouse mode toggles)" >>run.log
xdotool key --window "$WIN" KP_2
sleep 0.5
xdotool key --window "$WIN" F9
sleep 1.5

kill "$MPID" 2>/dev/null || true
wait "$MPID" 2>/dev/null || true
kill "$XPID" 2>/dev/null || true

cd ../..
n=1
for f in $(find /root/.mednafen/snaps -newer "$WORK_DIR/marker" -name "psx-hello-sideload-*.png" | sort); do
	cp "$f" "$OUT_DIR/$n.png"
	n=$((n + 1))
done

echo "run log: $WORK_DIR/run.log"
echo "screenshots: $OUT_DIR/ ($((n - 1)) captured)"
