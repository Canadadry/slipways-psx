set -e

RAYLIB_VERSION=6.0
RAYLIB_DIR="third_party/raylib"

UNAME_S="$(uname -s)"

if [ "$UNAME_S" = "Darwin" ]; then
	if ! xcode-select -p >/dev/null 2>&1; then
		echo "Xcode Command Line Tools not found. Run 'xcode-select --install' (no sudo needed -- it opens a one-time system installer), then re-run this target." >&2
		exit 1
	fi
	echo "macOS detected -- git/make/clang come from Xcode Command Line Tools, and raylib's OpenGL/Cocoa/IOKit/CoreAudio/CoreVideo frameworks ship with the OS. No package installation needed, no sudo required."
elif [ -n "$RAYLIB_VENDOR_SKIP_APT" ]; then
	echo "RAYLIB_VENDOR_SKIP_APT set -- skipping sudo apt-get; assuming git, a C toolchain, and the X11/GL dev headers are already present."
else
	sudo apt-get update
	sudo apt-get install -y \
		git build-essential \
		libgl1-mesa-dev libx11-dev libxrandr-dev libxi-dev libxcursor-dev \
		libxinerama-dev libxext-dev libwayland-dev libxkbcommon-dev
fi

if [ -d "$RAYLIB_DIR" ]; then
	echo "$RAYLIB_DIR already present -- skipping clone, rebuilding the library only."
else
	git clone --depth 1 --branch "$RAYLIB_VERSION" \
		https://github.com/raysan5/raylib.git "$RAYLIB_DIR"
fi

make -C "$RAYLIB_DIR/src" PLATFORM=PLATFORM_DESKTOP

echo "raylib $RAYLIB_VERSION built: $RAYLIB_DIR/src/libraylib.a"
