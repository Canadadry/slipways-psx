.PHONY: psn00bsdk-toolchain-setup psx-hello psx-hello-clean unit demo test

# The Love2D prototype (`game/`) was removed in favor of the PC/raylib
# literal port described in docs/plan.md. Phase 1 of that plan adds a C
# build + test setup here: `unit` runs the focused unit tests for each
# deep module (pico8_mem, asset loaders, pico8_gfx primitives, ...);
# `demo` builds and runs the end-to-end acceptance program (src/main.c,
# Chunk 9) that exercises every API category against the real pico8/
# cart assets and a scripted events.json replay, asserting specific
# framebuffer/peek values; `test` runs both, and is Phase 1's exit gate
# (see docs/prd/triage/phase1-etape1-pico8-interface-software-rendering.md,
# Implementation Chunks 1-9).

CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Werror -g

TMP_DIR := tmp

$(TMP_DIR):
	mkdir -p $(TMP_DIR)

$(TMP_DIR)/test_pico8_mem: tests/test_pico8_mem.c src/pico8_mem.c src/pico8_mem.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_mem.c src/pico8_mem.c

$(TMP_DIR)/test_asset_loading: tests/test_asset_loading.c src/pico8_assets.c src/pico8_assets.h src/pico8_mem.c src/pico8_mem.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_asset_loading.c src/pico8_assets.c src/pico8_mem.c

$(TMP_DIR)/test_pico8_gfx: tests/test_pico8_gfx.c src/pico8_gfx.c src/pico8_gfx.h src/pico8_mem.c src/pico8_mem.h src/pico8_assets.c src/pico8_assets.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_gfx.c src/pico8_gfx.c src/pico8_mem.c src/pico8_assets.c

$(TMP_DIR)/test_pico8_input: tests/test_pico8_input.c src/pico8_input.c src/pico8_input.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_input.c src/pico8_input.c

$(TMP_DIR)/test_pico8_dump: tests/test_pico8_dump.c src/pico8_dump.c src/pico8_dump.h src/pico8_gfx.c src/pico8_gfx.h src/pico8_mem.c src/pico8_mem.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_dump.c src/pico8_dump.c src/pico8_gfx.c src/pico8_mem.c

$(TMP_DIR)/test_pico8_fixed: tests/test_pico8_fixed.c src/pico8_fixed.c src/pico8_fixed.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_fixed.c src/pico8_fixed.c -lm

# UBSan-instrumented build of the same fixed-point test binary, scoped to
# this module only: pico8_fixed does bit-level signed/unsigned
# reinterpretation tricks (see pico8_fixed_add/sub/neg/div) where a
# regression back to plain signed-overflow/shift UB would produce correct
# numeric results on today's x86 codegen while still being undefined
# behavior. -fno-sanitize-recover=undefined makes any UBSan report a hard
# failure (non-zero exit) instead of a printed warning that's easy to miss.
$(TMP_DIR)/test_pico8_fixed_ubsan: tests/test_pico8_fixed.c src/pico8_fixed.c src/pico8_fixed.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -fsanitize=undefined -fno-sanitize-recover=undefined -o $@ tests/test_pico8_fixed.c src/pico8_fixed.c -lm

$(TMP_DIR)/pico8_demo: src/main.c src/pico8_mem.c src/pico8_mem.h src/pico8_assets.c src/pico8_assets.h src/pico8_gfx.c src/pico8_gfx.h src/pico8_input.c src/pico8_input.h src/pico8_dump.c src/pico8_dump.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ src/main.c src/pico8_mem.c src/pico8_assets.c src/pico8_gfx.c src/pico8_input.c src/pico8_dump.c

unit: $(TMP_DIR)/test_pico8_mem $(TMP_DIR)/test_asset_loading $(TMP_DIR)/test_pico8_gfx $(TMP_DIR)/test_pico8_input $(TMP_DIR)/test_pico8_dump $(TMP_DIR)/test_pico8_fixed $(TMP_DIR)/test_pico8_fixed_ubsan
	./$(TMP_DIR)/test_pico8_mem
	./$(TMP_DIR)/test_asset_loading
	./$(TMP_DIR)/test_pico8_gfx
	./$(TMP_DIR)/test_pico8_input
	./$(TMP_DIR)/test_pico8_dump
	./$(TMP_DIR)/test_pico8_fixed
	./$(TMP_DIR)/test_pico8_fixed_ubsan

demo: $(TMP_DIR)/pico8_demo
	./$(TMP_DIR)/pico8_demo

test: unit demo

psn00bsdk-toolchain-setup:
	sh tools/install_psn00bsdk_toolchain.sh

psx-hello:
	cmake -S psx-hello -B psx-hello/build && cmake --build psx-hello/build

psx-hello-clean:
	rm -rf psx-hello/build
