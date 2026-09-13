set -e

PATH="/usr/games:$PATH"
export PATH
export DISPLAY=:97
export SDL_AUDIODRIVER=dummy

if [ ! -f psx-hello/build/psx-hello.cue ]; then
	echo "psx-hello/build/psx-hello.cue not found -- run 'make psx-hello' first." >&2
	exit 1
fi

OUT_DIR=tools/.boot-test-snaps
rm -rf tools/.boot-test-run "$OUT_DIR"
mkdir -p tools/.boot-test-run "$OUT_DIR"
cd tools/.boot-test-run

Xvfb :97 -screen 0 640x480x24 >xvfb.log 2>&1 &
XPID=$!
sleep 2

touch marker

mednafen -force_module psx -video.fs 0 -psx.bios_sanity 0 -sound.driver dummy \
	-psx.input.port1 gamepad \
	../../psx-hello/build/psx-hello.cue >run.log 2>&1 &
MPID=$!

sleep 5
WIN=$(xdotool search --name "psx-hello" | head -1)
xdotool windowfocus --sync "$WIN"
sleep 0.5

xdotool key --window "$WIN" F9
sleep 1
xdotool key --window "$WIN" KP_2
sleep 1
xdotool key --window "$WIN" F9
sleep 1
xdotool key --window "$WIN" w
sleep 0.1
xdotool key --window "$WIN" a
sleep 0.3
xdotool key --window "$WIN" F9
sleep 1
xdotool key --window "$WIN" KP_6
xdotool key --window "$WIN" F9
sleep 1

kill "$MPID" 2>/dev/null || true
wait "$MPID" 2>/dev/null || true
kill "$XPID" 2>/dev/null || true

cd ../..
n=1
for f in $(find /root/.mednafen/snaps -newer tools/.boot-test-run/marker -name "psx-hello-*.png" | sort); do
	cp "$f" "$OUT_DIR/$n.png"
	n=$((n + 1))
done

echo "run log: tools/.boot-test-run/run.log"
echo "screenshots: $OUT_DIR/ ($((n - 1)) captured)"
