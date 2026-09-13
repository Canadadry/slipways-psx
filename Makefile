LOVE = /Applications/love.app/Contents/MacOS/love
LOVE_FILE = slipways.love

.PHONY: build run test font psn00bsdk-toolchain-setup psx-hello psx-hello-clean psx-hello-sandbox-smoketest


build:
	cd game && zip -r ../$(LOVE_FILE) .

run:
	$(LOVE) game
test:
	cd game && lua ../tests/test_all.lua

font:
	python3 tools/ttf_to_pixelfont.py "game/assets/kenney_kenney-fonts/Fonts/Kenney Mini.ttf" 8 " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" -o game/assets/font.png

psn00bsdk-toolchain-setup:
	sh tools/install_psn00bsdk_toolchain.sh

PSN00BSDK_TOOLCHAIN_BIN = /usr/local/mipsel-none-elf/bin
PSN00BSDK_LIBS_DIR = /usr/local/lib/libpsn00b

psx-hello:
	cd psx-hello && PATH="$(PSN00BSDK_TOOLCHAIN_BIN):$$PATH" PSN00BSDK_LIBS=$(PSN00BSDK_LIBS_DIR) cmake --preset default .
	cd psx-hello && PATH="$(PSN00BSDK_TOOLCHAIN_BIN):$$PATH" cmake --build build

psx-hello-clean:
	rm -rf psx-hello/build

psx-hello-sandbox-smoketest:
	sh tools/sandbox_psx_smoketest.sh
