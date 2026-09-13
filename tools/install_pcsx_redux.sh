set -e

sudo apt-get update
sudo apt-get install -y build-essential git make pkg-config clang g++ \
	libcapstone-dev libfreetype-dev libavcodec-dev libavformat-dev libavutil-dev \
	libcurl4-openssl-dev libswresample-dev libuv1-dev zlib1g-dev \
	libgl1-mesa-dev libglu1-mesa-dev cmake ninja-build

work_dir="$(mktemp -d)"

git clone --depth 1 --branch release-3.2.x https://github.com/libsdl-org/SDL.git "$work_dir/sdl3-src"
cmake -S "$work_dir/sdl3-src" -B "$work_dir/sdl3-build" -G Ninja \
	-DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build "$work_dir/sdl3-build" -j "$(nproc)"
sudo cmake --install "$work_dir/sdl3-build"
sudo ldconfig
rm -rf "$work_dir"

git clone --depth 1 https://github.com/grumpycoders/pcsx-redux.git tools/pcsx-redux
git -C tools/pcsx-redux submodule update --init --recursive --depth 1 --jobs 8

PATH="/usr/local/mipsel-none-elf/bin:$PATH"
export PATH
make -C tools/pcsx-redux/src/mips/openbios

make -C tools/pcsx-redux -j "$(nproc)"

echo "Built tools/pcsx-redux/pcsx-redux, with OpenBIOS at"
echo "tools/pcsx-redux/src/mips/openbios/openbios.bin (found automatically as a BIOS"
echo "fallback -- no real Sony BIOS needed). Run from tools/pcsx-redux/, e.g.:"
echo "  ./pcsx-redux --no-ui --loadiso ../../psx-hello/build/psx-hello.cue --run"
