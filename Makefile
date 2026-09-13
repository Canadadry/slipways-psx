LOVE = /Applications/love.app/Contents/MacOS/love
LOVE_FILE = slipways.love

.PHONY: build run test test-planetgen font psn00bsdk-toolchain-setup psx-hello psx-hello-clean psx-hello-sandbox-smoketest psx-hello-openbios-setup psx-hello-boot-test pcsx-redux-setup psx-hello-boot-test-pcsx psx-hello-input-test-pcsx psx-slipways psx-slipways-clean psx-slipways-boot-test-pcsx psx-slipways-screenshot-pcsx


build:
	cd game && zip -r ../$(LOVE_FILE) .

run:
	$(LOVE) game
test: test-planetgen
	cd game && lua ../tests/test_all.lua

# Host-native (gcc, no PSn00bSDK toolchain) tests for psx-slipways' PSX-
# independent logic (fixed-point, RNG, and later galaxy generation). Output
# goes under build-host/, kept separate from build/ (the PSn00bSDK cross-
# build output). List source files explicitly here as later chunks add
# more (e.g. planetgen.c and its own host test file).
PLANETGEN_HOST_BUILD_DIR = psx-slipways/build-host
RNG_AND_FIXED_HOST_SRCS = \
	psx-slipways/rng.c \
	psx-slipways/host_tests/rng_and_fixed_test.c
PLANETGEN_TEST_HOST_SRCS = \
	psx-slipways/rng.c \
	psx-slipways/planetgen.c \
	psx-slipways/host_tests/planetgen_test.c

test-planetgen:
	mkdir -p $(PLANETGEN_HOST_BUILD_DIR)
	gcc -std=c11 -Wall -Wextra -I psx-slipways -o $(PLANETGEN_HOST_BUILD_DIR)/rng_and_fixed_test $(RNG_AND_FIXED_HOST_SRCS) -lm
	$(PLANETGEN_HOST_BUILD_DIR)/rng_and_fixed_test
	gcc -std=c11 -Wall -Wextra -I psx-slipways -o $(PLANETGEN_HOST_BUILD_DIR)/planetgen_test $(PLANETGEN_TEST_HOST_SRCS) -lm
	$(PLANETGEN_HOST_BUILD_DIR)/planetgen_test

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

psx-hello-openbios-setup:
	sh tools/build_openbios_for_testing.sh $(BOOT_MODE)

psx-hello-boot-test:
	sh tools/boot_test_psx_hello.sh

pcsx-redux-setup:
	sh tools/install_pcsx_redux.sh

psx-hello-boot-test-pcsx:
	sh tools/boot_test_psx_hello_pcsx.sh

psx-hello-input-test-pcsx:
	sh tools/boot_test_psx_hello_pcsx_input.sh

psx-slipways:
	cd psx-slipways && PATH="$(PSN00BSDK_TOOLCHAIN_BIN):$$PATH" PSN00BSDK_LIBS=$(PSN00BSDK_LIBS_DIR) cmake --preset default .
	cd psx-slipways && PATH="$(PSN00BSDK_TOOLCHAIN_BIN):$$PATH" cmake --build build

psx-slipways-clean:
	rm -rf psx-slipways/build

psx-slipways-boot-test-pcsx:
	sh tools/boot_test_psx_slipways_pcsx.sh

psx-slipways-screenshot-pcsx:
	sh tools/capture_psx_screenshot_pcsx.sh psx-slipways/build/slipways.cue tools/.verify-snaps/psx-slipways-boot.png 1800
