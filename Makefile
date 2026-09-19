LOVE = /Applications/love.app/Contents/MacOS/love
LOVE_FILE = slipways.love

.PHONY: build run test test-planetgen test-galaxy-render test-cursor test-rules-planets test-liaison test-rules-discovery test-effects test-rules-tech test-rules-economy test-sprite-atlas test-audio-data test-audio-pitch font sprite-atlas audio-data psn00bsdk-toolchain-setup psx-hello psx-hello-clean psx-hello-sandbox-smoketest psx-hello-openbios-setup psx-hello-boot-test pcsx-redux-setup psx-hello-boot-test-pcsx psx-hello-input-test-pcsx psx-slipways psx-slipways-clean psx-slipways-boot-test-pcsx psx-slipways-screenshot-pcsx psx-slipways-input-test-pcsx psx-slipways-fps-test-pcsx psx-slipways-cursor-test-pcsx psx-slipways-liaison-test-pcsx psx-slipways-rules-tech-test-pcsx psx-slipways-liaison-marker-screenshot-pcsx psx-slipways-tech-icon-screenshot-pcsx


build:
	cd game && zip -r ../$(LOVE_FILE) .

run:
	$(LOVE) game
test: test-planetgen test-galaxy-render test-cursor test-rules-planets test-liaison test-rules-discovery test-effects test-rules-tech test-rules-economy test-sprite-atlas test-audio-data test-audio-pitch
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

# Host-native test for rules_economy.c (chunk 1: traced static tables and
# simple constants -- rules_economy_trade_level_bonus). This module is a
# leaf -- no dependency on world.c/cursor.c/liaison.c/rules_planets.c/
# rules_discovery.c/rules_tech.c at all -- same shape/output dir as
# test-planetgen/test-galaxy-render/test-cursor/test-rules-planets/
# test-liaison/test-rules-discovery/test-effects/test-rules-tech above.
RULES_ECONOMY_HOST_BUILD_DIR = psx-slipways/build-host
RULES_ECONOMY_TEST_HOST_SRCS = \
	psx-slipways/rules_economy.c \
	psx-slipways/host_tests/rules_economy_test.c

test-rules-economy:
	mkdir -p $(RULES_ECONOMY_HOST_BUILD_DIR)
	gcc -std=c11 -Wall -Wextra -I psx-slipways -o $(RULES_ECONOMY_HOST_BUILD_DIR)/rules_economy_test $(RULES_ECONOMY_TEST_HOST_SRCS) -lm
	$(RULES_ECONOMY_HOST_BUILD_DIR)/rules_economy_test

# Generation step for psx-slipways/sprite_atlas.h/.c (chunk 1 of
# docs/prd/triage/psx-sprites-audio-extraction.md): reads pico8/gfx.json
# and packs it into a 4bpp CLUT texture blob + the PICO-8 default palette,
# converted to PS1 BGR555 -- see gen_psx_sprite_atlas.py's own header
# comment for the pixel-packing/CLUT-conversion decisions. Mirrors the
# existing font: target's shape (a tool call as its own Makefile recipe);
# unlike font:, this one is also wired as a prerequisite of
# test-sprite-atlas/psx-slipways below (not just a separately-invoked
# target), so those never silently build against stale generated data.
# The generated sprite_atlas.h/.c files are not committed -- see
# .gitignore -- so this step must run before anything that reads them.
sprite-atlas:
	python3 tools/gen_psx_sprite_atlas.py

# Host-native test for sprite_atlas.c's pure half (the generated
# g_sprite_atlas_4bpp/g_pico8_clut const arrays plus sprite_atlas_cel_rect,
# zero PSX-header dependency -- see docs/prd/triage/
# psx-sprites-audio-extraction.md, Implementation Chunks chunk 1), same
# shape/output dir as test-planetgen/test-galaxy-render/test-cursor/
# test-rules-planets/test-liaison/test-rules-discovery/test-effects/
# test-rules-tech/test-rules-economy above. Depends on sprite-atlas so the
# generated sprite_atlas.h/.c are always fresh before compiling against
# them.
SPRITE_ATLAS_HOST_BUILD_DIR = psx-slipways/build-host
SPRITE_ATLAS_TEST_HOST_SRCS = \
	psx-slipways/sprite_atlas.c \
	psx-slipways/host_tests/sprite_atlas_test.c

test-sprite-atlas: sprite-atlas
	mkdir -p $(SPRITE_ATLAS_HOST_BUILD_DIR)
	gcc -std=c11 -Wall -Wextra -I psx-slipways -o $(SPRITE_ATLAS_HOST_BUILD_DIR)/sprite_atlas_test $(SPRITE_ATLAS_TEST_HOST_SRCS) -lm
	$(SPRITE_ATLAS_HOST_BUILD_DIR)/sprite_atlas_test

# Generation step for psx-slipways/audio_data.h/.c (chunk 7 of
# docs/prd/triage/psx-sprites-audio-extraction.md): reads pico8/sfx.json/
# pico8/music.json and emits the sfx_def_t/music_pattern_t data tables --
# see gen_psx_audio_data.py's own header comment for the struct-shape/enum
# decisions. Mirrors sprite-atlas's shape/reasoning above (a tool call as
# its own Makefile recipe, wired as a prerequisite of test-audio-data below
# rather than only a separately-invoked target). The generated
# audio_data.h/.c files are not committed -- see .gitignore -- so this step
# must run before anything that reads them. Pure data, no PSX/psxgpu.h
# dependency at all -- unlike sprite-atlas, this one is not also a
# psx-slipways prerequisite, since no PSX-side code reads it yet this step
# (SPU playback is docs/plan.md Phase 5 step 19's job).
audio-data:
	python3 tools/gen_psx_audio_data.py

# Host-native test for audio_data.c (chunk 7, docs/prd/triage/
# psx-sprites-audio-extraction.md): pure data end to end, zero PSX-header
# dependency (unlike sprite_atlas.c, this module has no __mips__-gated
# half at all), same shape/output dir as test-sprite-atlas and every other
# test-* target above. Depends on audio-data so the generated
# audio_data.h/.c are always fresh before compiling against them.
AUDIO_DATA_HOST_BUILD_DIR = psx-slipways/build-host
AUDIO_DATA_TEST_HOST_SRCS = \
	psx-slipways/audio_data.c \
	psx-slipways/host_tests/audio_data_test.c

test-audio-data: audio-data
	mkdir -p $(AUDIO_DATA_HOST_BUILD_DIR)
	gcc -std=c11 -Wall -Wextra -I psx-slipways -o $(AUDIO_DATA_HOST_BUILD_DIR)/audio_data_test $(AUDIO_DATA_TEST_HOST_SRCS) -lm
	$(AUDIO_DATA_HOST_BUILD_DIR)/audio_data_test

# Host-native test for audio_pitch.c (chunk 1, docs/prd/triage/
# psx-spu-audio.md): pure integer table + lookup, zero PSX-header dependency
# (no SPU/psxspu.h needed -- this module never touches real hardware
# registers, only computes the VALUE that would go into one), same shape/
# output dir as test-audio-data and every other test-* target above.
AUDIO_PITCH_HOST_BUILD_DIR = psx-slipways/build-host
AUDIO_PITCH_TEST_HOST_SRCS = \
	psx-slipways/audio_pitch.c \
	psx-slipways/host_tests/audio_pitch_test.c

test-audio-pitch:
	mkdir -p $(AUDIO_PITCH_HOST_BUILD_DIR)
	gcc -std=c11 -Wall -Wextra -I psx-slipways -o $(AUDIO_PITCH_HOST_BUILD_DIR)/audio_pitch_test $(AUDIO_PITCH_TEST_HOST_SRCS) -lm
	$(AUDIO_PITCH_HOST_BUILD_DIR)/audio_pitch_test

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

# Depends on sprite-atlas so the cross-build's file(GLOB *.c) (psx-slipways/
# CMakeLists.txt) always picks up freshly-generated sprite_atlas.h/.c
# rather than stale or missing ones (Implementation Chunks chunk 1,
# docs/prd/triage/psx-sprites-audio-extraction.md).
psx-slipways: sprite-atlas
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

psx-slipways-liaison-marker-screenshot-pcsx:
	sh tools/capture_psx_slipways_liaison_marker_screenshots_pcsx.sh

psx-slipways-tech-icon-screenshot-pcsx:
	sh tools/capture_psx_slipways_tech_icon_screenshot_pcsx.sh
