set -e

RAYLIB_VERSION=6.0
RAYLIB_DIR="third_party/raylib"

sudo apt-get update
sudo apt-get install -y \
	git build-essential \
	libgl1-mesa-dev libx11-dev libxrandr-dev libxi-dev libxcursor-dev \
	libxinerama-dev libxext-dev libwayland-dev libxkbcommon-dev

if [ -d "$RAYLIB_DIR" ]; then
	echo "$RAYLIB_DIR already present -- skipping clone, rebuilding the library only."
else
	git clone --depth 1 --branch "$RAYLIB_VERSION" \
		https://github.com/raysan5/raylib.git "$RAYLIB_DIR"
fi

make -C "$RAYLIB_DIR/src" PLATFORM=PLATFORM_DESKTOP

echo "raylib $RAYLIB_VERSION built: $RAYLIB_DIR/src/libraylib.a"
