LOVE = /Applications/love.app/Contents/MacOS/love
LOVE_FILE = slipways.love

.PHONY: build run test test-planetgen test-galaxy-render test-cursor test-rules-planets test-liaison test-rules-discovery test-effects test-rules-tech font psn00bsdk-toolchain-setup psx-hello psx-hello-clean psx-hello-sandbox-smoketest psx-hello-openbios-setup psx-hello-boot-test pcsx-redux-setup psx-hello-boot-test-pcsx psx-hello-input-test-pcsx psx-slipways psx-slipways-clean psx-slipways-boot-test-pcsx psx-slipways-screenshot-pcsx psx-slipways-input-test-pcsx psx-slipways-fps-test-pcsx psx-slipways-cursor-test-pcsx psx-slipways-liaison-test-pcsx psx-slipways-rules-tech-test-pcsx


build:
	cd game && zip -r ../$(LOVE_FILE) .

run:
	$(LOVE) game
test: test-planetgen test-galaxy-render test-cursor test-rules-planets test-liaison test-rules-discovery test-effects test-rules-tech
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

# Host-native tests for the galaxy-rendering pure modules (screen_map,
# world), same shape/output dir as test-planetgen above.
GALAXY_RENDER_HOST_BUILD_DIR = psx-slipways/build-host
SCREEN_MAP_TEST_HOST_SRCS = \
	psx-slipways/screen_map.c \
	psx-slipways/host_tests/screen_map_test.c
SLIPWAY_BENCH_TEST_HOST_SRCS = \
	psx-slipways/slipway_bench.c \
	psx-slipways/host_tests/slipway_bench_test.c
WORLD_TEST_HOST_SRCS = \
	psx-slipways/rng.c \
	psx-slipways/planetgen.c \
	psx-slipways/slipway_bench.c \
	psx-slipways/world.c \
	psx-slipways/host_tests/world_test.c

test-galaxy-render:
	mkdir -p $(GALAXY_RENDER_HOST_BUILD_DIR)
	gcc -std=c11 -Wall -Wextra -I psx-slipways -o $(GALAXY_RENDER_HOST_BUILD_DIR)/screen_map_test $(SCREEN_MAP_TEST_HOST_SRCS) -lm
	$(GALAXY_RENDER_HOST_BUILD_DIR)/screen_map_test
	gcc -std=c11 -Wall -Wextra -I psx-slipways -o $(GALAXY_RENDER_HOST_BUILD_DIR)/slipway_bench_test $(SLIPWAY_BENCH_TEST_HOST_SRCS) -lm
	$(GALAXY_RENDER_HOST_BUILD_DIR)/slipway_bench_test
	gcc -std=c11 -Wall -Wextra -I psx-slipways -o $(GALAXY_RENDER_HOST_BUILD_DIR)/world_test $(WORLD_TEST_HOST_SRCS) -lm
	$(GALAXY_RENDER_HOST_BUILD_DIR)/world_test

# Host-native test for cursor.c (mode-toggle chunk), same shape/output dir
# as test-planetgen/test-galaxy-render above.
CURSOR_HOST_BUILD_DIR = psx-slipways/build-host
CURSOR_TEST_HOST_SRCS = \
	psx-slipways/cursor.c \
	psx-slipways/host_tests/cursor_test.c

test-cursor:
	mkdir -p $(CURSOR_HOST_BUILD_DIR)
	gcc -std=c11 -Wall -Wextra -I psx-slipways -o $(CURSOR_HOST_BUILD_DIR)/cursor_test $(CURSOR_TEST_HOST_SRCS) -lm
	$(CURSOR_HOST_BUILD_DIR)/cursor_test

# Host-native test for rules_planets.c (chunk 1: static data tables), same
# shape/output dir as test-planetgen/test-galaxy-render/test-cursor above.
RULES_PLANETS_HOST_BUILD_DIR = psx-slipways/build-host
RULES_PLANETS_TEST_HOST_SRCS = \
	psx-slipways/rules_planets.c \
	psx-slipways/host_tests/rules_planets_test.c

test-rules-planets:
	mkdir -p $(RULES_PLANETS_HOST_BUILD_DIR)
	gcc -std=c11 -Wall -Wextra -I psx-slipways -o $(RULES_PLANETS_HOST_BUILD_DIR)/rules_planets_test $(RULES_PLANETS_TEST_HOST_SRCS) -lm
	$(RULES_PLANETS_HOST_BUILD_DIR)/rules_planets_test

# Host-native test for liaison.c (chunk 1: section-5 geometry core; chunk 4
# adds the stateful wrapper, which calls world_init()/world_get_galaxy(), so
# this now needs the same WORLD_TEST_HOST_SRCS transitive set), same shape/
# output dir as test-planetgen/test-galaxy-render/test-cursor/
# test-rules-planets above.
LIAISON_HOST_BUILD_DIR = psx-slipways/build-host
RULES_LIAISON_TEST_HOST_SRCS = \
	psx-slipways/liaison.c \
	psx-slipways/screen_map.c \
	psx-slipways/rng.c \
	psx-slipways/planetgen.c \
	psx-slipways/slipway_bench.c \
	psx-slipways/world.c \
	psx-slipways/host_tests/liaison_test.c

test-liaison:
	mkdir -p $(LIAISON_HOST_BUILD_DIR)
	gcc -std=c11 -Wall -Wextra -I psx-slipways -o $(LIAISON_HOST_BUILD_DIR)/liaison_test $(RULES_LIAISON_TEST_HOST_SRCS) -lm
	$(LIAISON_HOST_BUILD_DIR)/liaison_test

# Host-native test for rules_discovery.c (chunk 2 adds rules_discovery.c
# itself: rules_discovery_probe_in_range, reusing world.h's
# WORLD_NEIGHBOR_RANGE_SQ, which is a header-only macro -- no link-time
# dependency on world.c/planetgen.c/etc.; rng.c is included here for
# forward consistency with the later empty-roll chunks that will need
# rng_t), same shape/output dir as test-planetgen/test-galaxy-render/
# test-cursor/test-rules-planets/test-liaison above.
RULES_DISCOVERY_HOST_BUILD_DIR = psx-slipways/build-host
RULES_DISCOVERY_TEST_HOST_SRCS = \
	psx-slipways/rng.c \
	psx-slipways/rules_discovery.c \
	psx-slipways/host_tests/rules_discovery_test.c

test-rules-discovery:
	mkdir -p $(RULES_DISCOVERY_HOST_BUILD_DIR)
	gcc -std=c11 -Wall -Wextra -I psx-slipways -o $(RULES_DISCOVERY_HOST_BUILD_DIR)/rules_discovery_test $(RULES_DISCOVERY_TEST_HOST_SRCS) -lm
	$(RULES_DISCOVERY_HOST_BUILD_DIR)/rules_discovery_test

# Host-native test for effects.c (chunk 2: the spawn formula,
# effects_flow_try_spawn -- a brand-new, previously-nonexistent module, so
# this is a NEW EFFECTS_TEST_HOST_SRCS var/test-effects target, not an
# extension of an existing one; rng.c is a real link-time dependency here
# (not just forward consistency), since effects_flow_try_spawn itself calls
# rng_next_range), same shape/output dir as test-planetgen/
# test-galaxy-render/test-cursor/test-rules-planets/test-liaison/
# test-rules-discovery above.
EFFECTS_HOST_BUILD_DIR = psx-slipways/build-host
EFFECTS_TEST_HOST_SRCS = \
	psx-slipways/rng.c \
	psx-slipways/effects.c \
	psx-slipways/host_tests/effects_test.c

test-effects:
	mkdir -p $(EFFECTS_HOST_BUILD_DIR)
	gcc -std=c11 -Wall -Wextra -I psx-slipways -o $(EFFECTS_HOST_BUILD_DIR)/effects_test $(EFFECTS_TEST_HOST_SRCS) -lm
	$(EFFECTS_HOST_BUILD_DIR)/effects_test

# Host-native test for rules_tech.c (chunk 1: static Era-1 tech data table).
# This module is a leaf -- no dependency on world.c/cursor.c/liaison.c at
# all -- same shape/output dir as test-planetgen/test-galaxy-render/
# test-cursor/test-rules-planets/test-liaison/test-rules-discovery/
# test-effects above.
RULES_TECH_HOST_BUILD_DIR = psx-slipways/build-host
RULES_TECH_TEST_HOST_SRCS = \
	psx-slipways/rules_tech.c \
	psx-slipways/host_tests/rules_tech_test.c

test-rules-tech:
	mkdir -p $(RULES_TECH_HOST_BUILD_DIR)
	gcc -std=c11 -Wall -Wextra -I psx-slipways -o $(RULES_TECH_HOST_BUILD_DIR)/rules_tech_test $(RULES_TECH_TEST_HOST_SRCS) -lm
	$(RULES_TECH_HOST_BUILD_DIR)/rules_tech_test

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

psx-slipways-input-test-pcsx:
	sh tools/boot_test_psx_slipways_pcsx_input.sh

psx-slipways-screenshot-pcsx:
	sh tools/capture_psx_screenshot_pcsx.sh psx-slipways/build/slipways.cue tools/.verify-snaps/psx-slipways-boot.png 1800

psx-slipways-fps-test-pcsx:
	sh tools/boot_test_psx_slipways_pcsx_fps.sh

psx-slipways-cursor-test-pcsx:
	sh tools/boot_test_psx_slipways_pcsx_cursor.sh

psx-slipways-liaison-test-pcsx:
	sh tools/boot_test_psx_slipways_pcsx_liaison.sh

psx-slipways-rules-tech-test-pcsx:
	sh tools/boot_test_psx_slipways_pcsx_rules_tech.sh
