.PHONY: psn00bsdk-toolchain-setup psx-hello psx-hello-clean unit test

# The Love2D prototype (`game/`) was removed in favor of the PC/raylib
# literal port described in docs/plan.md. Phase 1 of that plan adds a C
# build + test setup here: `unit` runs the focused unit tests for each
# deep module (pico8_mem, asset loaders, pico8_gfx primitives, ...);
# `test` will additionally build and run the end-to-end demo program once
# it exists (see docs/prd/triage/phase1-etape1-pico8-interface-software-rendering.md,
# Implementation Chunks 1-9). For now `test` is an alias for `unit`.

CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Werror -g

TMP_DIR := tmp

$(TMP_DIR):
	mkdir -p $(TMP_DIR)

$(TMP_DIR)/test_pico8_mem: tests/test_pico8_mem.c src/pico8_mem.c src/pico8_mem.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_mem.c src/pico8_mem.c

$(TMP_DIR)/test_asset_loading: tests/test_asset_loading.c src/pico8_assets.c src/pico8_assets.h src/pico8_mem.c src/pico8_mem.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_asset_loading.c src/pico8_assets.c src/pico8_mem.c

unit: $(TMP_DIR)/test_pico8_mem $(TMP_DIR)/test_asset_loading
	./$(TMP_DIR)/test_pico8_mem
	./$(TMP_DIR)/test_asset_loading

test: unit

psn00bsdk-toolchain-setup:
	sh tools/install_psn00bsdk_toolchain.sh

psx-hello:
	cmake -S psx-hello -B psx-hello/build && cmake --build psx-hello/build

psx-hello-clean:
	rm -rf psx-hello/build
