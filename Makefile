.DEFAULT_GOAL := help

# Reruns make under flock: one make of LOCKED_GOALS at a time over tmp/.
LOCKED_GOALS := test e2e test-all pad-author demo-check

ifneq ($(if $(MAKE_LOCK_HELD),,$(filter $(LOCKED_GOALS),$(MAKECMDGOALS))),)
$(MAKECMDGOALS): make-lock ; @:

make-lock:
	@command -v flock > /dev/null || { echo "make: flock missing (macOS: brew install flock)" >&2; exit 1; }
	@mkdir -p tmp
	@flock -n -E 75 tmp/make.lock $(MAKE) --no-print-directory MAKE_LOCK_HELD=1 $(MAKECMDGOALS); \
	status=$$?; \
	[ $$status -ne 75 ] || echo "make: tmp/ is in use by another make of $(LOCKED_GOALS)" >&2; \
	exit $$status

.PHONY: make-lock
else
help: ## lists the targets
	@grep -hE '^[a-z0-9-]+:.*## ' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*## "}; {printf "%-30s %s\n", $$1, $$2}' | LC_ALL=C sort

.PHONY: help psx-tools-test psn00bsdk-toolchain-setup psx-build psx-game-cd psx-game-cd-check psx-check psx-game-check psx-gpu-capture-ceilings psx-gpu-capture-accept psx-gpu-capture-report psx-replay-check psx-full-game-check psx-short-runs-cut psx-short-runs psx-probe-baseline psx-probe psx-pad-check psx-hello psx-hello-cd psx-hello-check psx-hello-cd-check psx-hello-e2e psx-hello-e2e-baseline psx-hello-frames-guard psx-hello-clean psx-net-check psx-emulator-setup psx-boot psx-emulator-check unit replay state test e2e test-all raylib-vendor-setup raylib e2e-baseline e2e-ref

NPROC := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
MAKEFLAGS += -j$(NPROC)
ifneq ($(filter output-sync,$(.FEATURES)),)
    MAKEFLAGS += --output-sync=target
endif

CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Werror -g
override CFLAGS += -Isrc

TMP_DIR := tmp

# One object tree per set of flags. An object compiled without -fsanitize is
# not instrumented: sharing it with a target that is would leave the binary
# blind on that code, silently. Do not merge the seven.
OBJ_DIR := $(TMP_DIR)/obj
OBJ_ASAN_DIR := $(TMP_DIR)/obj-asan
OBJ_UBSAN_DIR := $(TMP_DIR)/obj-ubsan
OBJ_SANITIZE_DIR := $(TMP_DIR)/obj-sanitize
OBJ_COVERAGE_DIR := $(TMP_DIR)/obj-coverage
OBJ_PROFILE_DIR := $(TMP_DIR)/obj-profile
OBJ_PROFILE_PLAIN_DIR := $(TMP_DIR)/obj-profile-plain

# `-MMD -MP`: next to each object, the compiler writes the list of headers that
# the translation unit actually opened. The include at the bottom of this file
# reads them back, so no header is ever named by hand in a prerequisite list.
DEPFLAGS := -MMD -MP
ASAN_CFLAGS := -fsanitize=address -fno-omit-frame-pointer
UBSAN_CFLAGS := -fsanitize=undefined -fno-sanitize-recover=undefined
SANITIZE_OPT_CFLAGS := -O2
COVERAGE_CFLAGS := --coverage -O0
PROFILE_CFLAGS := -O2 -DNDEBUG -Wno-error=unused-variable -Wno-error=unused-but-set-variable \
	-Wno-error=unused-function -Wno-error=unused-parameter
PROFILE_INSTRUMENT_CFLAGS := -finstrument-functions

# `$(call objs,<sources>)` -> the paths of the matching object files in the
# tree concerned. A target names its sources once; its prerequisites and its
# link line follow from them.
objs = $(patsubst %.c,$(OBJ_DIR)/%.o,$(1))
asan_objs = $(patsubst %.c,$(OBJ_ASAN_DIR)/%.o,$(1))
ubsan_objs = $(patsubst %.c,$(OBJ_UBSAN_DIR)/%.o,$(1))
sanitize_objs = $(patsubst %.c,$(OBJ_SANITIZE_DIR)/%.o,$(1))
coverage_objs = $(patsubst %.c,$(OBJ_COVERAGE_DIR)/%.o,$(1))
profile_objs = $(patsubst %.c,$(OBJ_PROFILE_DIR)/%.o,$(1))
profile_plain_objs = $(patsubst %.c,$(OBJ_PROFILE_PLAIN_DIR)/%.o,$(1))

# `mkdir -p $(@D)` in the recipe rather than a directory as a prerequisite:
# the mtime of a directory changes with every file written into it, which would
# rebuild everything.
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_ASAN_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_UBSAN_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(UBSAN_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_SANITIZE_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) $(UBSAN_CFLAGS) $(SANITIZE_OPT_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_COVERAGE_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(COVERAGE_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_PROFILE_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(PROFILE_CFLAGS) $(PROFILE_INSTRUMENT_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_PROFILE_PLAIN_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(PROFILE_CFLAGS) $(DEPFLAGS) -c -o $@ $<

BINDINGS_CORE_UNIT := $(TMP_DIR)/bindings/core.c
DEMO_UNIT := $(TMP_DIR)/demo/demo.c

# The whole engine closure: all of src/ except the entry points of
# src/cmd/, src/p8/palette.c, the two `src/platform/raylib_backend*.c`, `src/platform/keymap.c` and
# the modules of src/devtools/ that only the tools and the interactive game
# link (fixed_test_double, replay_control, replay_source, screencast,
# session_recorder, state), plus the default bindings table generated into
# tmp/bindings/core.c and the demo compiled into tmp/demo/demo.c. Used by the
# targets that call cart_load(), which pulls in the whole port.
ENGINE_SRCS := \
    $(BINDINGS_CORE_UNIT) \
    $(DEMO_UNIT) \
    src/p8/assets.c \
    src/p8/sprite_sheet.c \
    src/game/attract.c \
    src/game/background.c \
    src/game/row_image.c \
    src/game/text_image.c \
    src/game/button_image.c \
    src/game/bindings.c \
    src/engine/bootstrap.c \
    src/game/cheat.c \
    src/game/demo_overlay.c \
    src/game/loading_screen.c \
    src/game/input_step.c \
    src/game/pad_nav.c \
    src/game/pad_panel.c \
    src/game/pad_jump.c \
    src/game/pad_slipway.c \
    src/game/pad_target.c \
    src/game/pad_back.c \
    src/game/pad_select.c \
    src/game/pad_build.c \
    src/game/pad_cycle.c \
    src/game/cart.c \
    src/engine/collect.c \
    src/game/data_init.c \
    src/engine/dispatch_context.c \
    src/devtools/dump.c \
    src/game/economy.c \
    src/game/economy_formulas.c \
    src/game/economy_score.c \
    src/game/economy_tiers.c \
    src/engine/entity_registry.c \
    src/engine/entity_geom.c \
    src/engine/entity_id.c \
    src/engine/entity_id_list.c \
    src/engine/exit_queue.c \
    src/platform/fatal_pc.c \
    src/p8/fixed.c \
    src/game/galaxy.c \
    src/game/game_data.c \
    src/game/gen.c \
    src/game/visual_random.c \
    src/engine/geom.c \
    src/p8/gfx.c \
    src/p8/gfx_software.c \
    src/p8/image.c \
    src/p8/draw_cost.c \
    src/p8/draw_meter.c \
    src/game/help.c \
    src/game/hud.c \
    src/p8/input.c \
    src/devtools/input_json.c \
    src/engine/entity.c \
    src/game/layout.c \
    src/engine/entity_list.c \
    src/engine/value_list.c \
    src/engine/pointer_list.c \
    src/game/map_tables.c \
    src/p8/vram.c \
    src/game/menu.c \
    src/game/mouse_button.c \
    src/game/mouse.c \
    src/game/message_box.c \
    src/game/particle.c \
    src/game/draw_functions.c \
    src/game/planet.c \
    src/game/grab_arena.c \
    src/game/planet_display.c \
    src/game/planet_family.c \
    src/game/planet_family_trade.c \
    src/game/planet_trade.c \
    src/game/probe.c \
    src/game/research.c \
    src/game/research_display.c \
    src/game/structure_icon.c \
    src/game/score_panel.c \
    src/game/score_panel_display.c \
    src/game/scroll.c \
    src/game/slipway.c \
    src/game/slipway_display.c \
    src/p8/str.c \
    src/game/tech_gate.c \
    src/game/trade.c \
    src/game/ui.c \
    src/game/ui_button.c \
    src/engine/vec.c \
    src/game/wormhole.c \
    src/game/wormhole_display.c \
    src/game/save_state.c \
    src/game/save_load.c \
    src/devtools/save_storage_memory.c \
    src/engine/zone.c \
    src/engine/allocator.c \
    src/engine/slot_tally.c \
    src/engine/slot_group.c \
    src/engine/frame_zone.c \
    src/game/ui_zone.c \
    src/game/particle_zone.c \
    src/engine/game_zone.c

# The ported core, free of floating point: the engine closure minus the host
# tools, which keep the libc. constants-check, float-guard, alloc-guard and
# float-grep all read this list.
CORE_SRCS := $(filter-out $(BINDINGS_CORE_UNIT) $(DEMO_UNIT) src/p8/assets.c src/devtools/dump.c src/platform/fatal_pc.c src/devtools/input_json.c,$(ENGINE_SRCS))

# A narrower closure: what data_init() reaches to build a real game state,
# without calling cart_load().
CART_ONLY_SRCS := \
    $(DEMO_UNIT) \
    src/game/attract.c \
    src/game/demo_overlay.c \
    src/game/loading_screen.c \
    src/game/cart.c \
    src/game/cheat.c \
    src/game/input_step.c \
    src/game/pad_nav.c \
    src/game/pad_panel.c \
    src/game/pad_jump.c \
    src/game/pad_slipway.c \
    src/game/pad_target.c \
    src/game/pad_back.c \
    src/game/pad_select.c \
    src/game/pad_build.c \
    src/game/pad_cycle.c \
    src/game/galaxy.c \
    src/game/help.c \
    src/game/menu.c \
    src/game/planet_family_trade.c \
    src/game/score_panel.c \
    src/game/score_panel_display.c \
    src/game/save_state.c \
    src/game/save_load.c \
    src/game/structure_icon.c
DATA_ENGINE_SRCS := $(filter-out $(CART_ONLY_SRCS),$(ENGINE_SRCS))

# The system libraries of raylib, for the two raylib targets; everything else
# is portable C, with no link flags specific to the platform.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    RAYLIB_LDLIBS := -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo
else
    RAYLIB_LDLIBS := -lm -lGL -lX11 -lpthread -ldl -lrt
endif

$(TMP_DIR):
	mkdir -p $(TMP_DIR)

# The .json prerequisites are the fixtures that the test reads at run time;
# `$(filter %.o,$^)` keeps them off the link line.
$(TMP_DIR)/test_vram: $(call objs,tests/test_vram.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_asset_loading: $(call objs,tests/test_asset_loading.c src/p8/assets.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_sprite_sheet: $(call objs,tests/test_sprite_sheet.c src/p8/sprite_sheet.c src/p8/assets.c src/p8/vram.c) pico8/gfx.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_gfx: $(call objs,tests/test_gfx.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c src/p8/assets.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_draw_backend: $(call objs,tests/test_draw_backend.c src/p8/draw_cost.c src/p8/image.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_image: $(call objs,tests/test_image.c src/p8/image.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_draw_cost: $(call objs,tests/test_draw_cost.c src/p8/draw_cost.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_draw_meter: $(call objs,tests/test_draw_meter.c src/p8/draw_meter.c src/p8/draw_cost.c src/p8/image.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_vram_layout: $(call objs,tests/test_psx_vram_layout.c src/platform/psx_vram_layout.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_tiles: $(call objs,tests/test_psx_tiles.c src/platform/psx_tiles.c src/p8/draw_cost.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_draw_filter: $(call objs,tests/test_draw_filter.c src/devtools/draw_filter.c src/p8/draw_cost.c src/p8/image.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_draw_timing: $(call objs,tests/test_draw_timing.c src/devtools/draw_timing.c src/p8/image.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_capture_list: $(call objs,tests/test_capture_list.c src/devtools/capture_list.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_short_run_cut: $(call objs,tests/test_short_run_cut.c src/devtools/short_run_cut.c src/p8/input.c src/devtools/input_json.c src/game/bindings.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_situation: $(call objs,tests/test_situation.c src/devtools/situation.c src/devtools/pad_metrics.c $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_situation_table: $(call objs,tests/test_situation_table.c src/devtools/situation.c src/devtools/pad_metrics.c $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) psx/perf/full-run-situations.txt | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_situation_report: $(call objs,tests/test_situation_report.c) tools/situation_report.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_steady_windows: $(call objs,tests/test_steady_windows.c) tools/steady_windows.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_short_run_situations: $(call objs,tests/test_short_run_situations.c) tools/short_run_situations.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_situations: $(call objs,tests/test_situations.c) $(TMP_DIR)/situations tests/e2e/pad_score.json tests/e2e/pad_probe.json tests/e2e/pad_game_end.json pico8/demo_record.hex | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_capture_compare: $(call objs,tests/test_capture_compare.c src/devtools/capture_compare.c src/platform/psx_colour.c src/p8/palette.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_colour: $(call objs,tests/test_psx_colour.c src/platform/psx_colour.c src/p8/palette.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_gpu_textures: $(call objs,tests/test_psx_gpu_textures.c src/platform/psx_gpu_textures.c src/platform/psx_colour.c src/p8/palette.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_clut_cache: $(call objs,tests/test_psx_clut_cache.c src/platform/psx_clut_cache.c src/platform/psx_gpu_textures.c src/platform/psx_colour.c src/p8/palette.c src/p8/gfx_software.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_slot_uploads: $(call objs,tests/test_psx_slot_uploads.c src/platform/psx_slot_uploads.c src/platform/psx_clut_cache.c src/platform/psx_pattern_cache.c src/platform/psx_vram_layout.c src/platform/psx_gpu_textures.c src/platform/psx_colour.c src/p8/palette.c src/p8/draw_cost.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_gpu_peaks: $(call objs,tests/test_psx_gpu_peaks.c src/platform/psx_gpu_peaks.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_pattern_cache: $(call objs,tests/test_psx_pattern_cache.c src/platform/psx_pattern_cache.c src/platform/psx_gpu_textures.c src/platform/psx_colour.c src/p8/palette.c src/p8/draw_cost.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_pattern: $(call objs,tests/test_psx_pattern.c src/platform/psx_pattern.c src/platform/psx_tiles.c src/platform/psx_vram_layout.c src/platform/psx_gpu_textures.c src/platform/psx_colour.c src/p8/palette.c src/p8/draw_cost.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_sprite: $(call objs,tests/test_psx_sprite.c src/platform/psx_sprite.c src/p8/draw_cost.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_image: $(call objs,tests/test_psx_image.c src/platform/psx_image.c src/platform/psx_sprite.c src/platform/psx_vram_layout.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_text: $(call objs,tests/test_psx_text.c src/platform/psx_text.c src/platform/psx_sprite.c src/platform/psx_gpu_textures.c src/platform/psx_colour.c src/p8/palette.c src/p8/draw_cost.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_input: $(call objs,tests/test_input.c src/p8/input.c src/devtools/input_json.c src/game/bindings.c) tests/fixtures/demo_events.json tests/fixtures/events_valid_ox.json tests/fixtures/events_mouse_recording_refused.json $(wildcard tests/fixtures/events_pad_*.json) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_dump: $(call objs,tests/test_dump.c src/devtools/dump.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_fixed: $(call objs,tests/test_fixed.c src/p8/fixed.c src/devtools/fixed_test_double.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# The same test as above, under UBSan: p8/fixed.c reinterprets bits between
# signed and unsigned, which would give the right numbers while being undefined
# behavior. `-fno-sanitize-recover` makes the binary fail instead of printing a
# warning.
$(TMP_DIR)/test_fixed_ubsan: $(call ubsan_objs,tests/test_fixed.c src/p8/fixed.c src/devtools/fixed_test_double.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(UBSAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

KEYMAP_DEFAULT_UNIT := $(TMP_DIR)/keymap/default.c

$(TMP_DIR)/keymap_generate: $(call objs,src/cmd/keymap_generate.c src/platform/keymap.c src/game/bindings.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(KEYMAP_DEFAULT_UNIT): keymap.default.cfg $(TMP_DIR)/keymap_generate
	@mkdir -p $(@D)
	$(TMP_DIR)/keymap_generate $< $@

$(TMP_DIR)/test_keymap: $(call objs,tests/test_keymap.c src/platform/keymap.c src/game/bindings.c $(KEYMAP_DEFAULT_UNIT)) keymap.default.cfg | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_pad_map: $(call objs,tests/test_psx_pad_map.c src/platform/psx_pad_map.c src/platform/keymap.c src/game/bindings.c) keymap.default.cfg | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pad_log: $(call objs,tests/test_pad_log.c src/devtools/pad_log.c src/platform/psx_pad_map.c src/p8/input.c src/devtools/input_json.c src/game/bindings.c) tests/e2e/pad_title.json psx/game_title_script.txt | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

BINDINGS_DEFAULT_UNIT := $(TMP_DIR)/bindings/default.c

$(TMP_DIR)/bindings_generate: $(call objs,src/cmd/bindings_generate.c src/game/bindings.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(BINDINGS_DEFAULT_UNIT): bindings.default.cfg $(TMP_DIR)/bindings_generate
	@mkdir -p $(@D)
	$(TMP_DIR)/bindings_generate $< $@

$(BINDINGS_CORE_UNIT): bindings.default.cfg $(TMP_DIR)/bindings_generate
	@mkdir -p $(@D)
	$(TMP_DIR)/bindings_generate --default $< $@

$(TMP_DIR)/test_bindings: $(call objs,tests/test_bindings.c src/game/bindings.c $(BINDINGS_DEFAULT_UNIT) $(BINDINGS_CORE_UNIT)) bindings.default.cfg | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_replay_source: $(call objs,tests/test_replay_source.c src/devtools/save_storage_memory.c src/devtools/replay_source.c src/p8/input.c src/devtools/input_json.c src/game/bindings.c) tests/fixtures/events_valid.json tests/fixtures/events_pad_missing_buttons.json tests/fixtures/events_mouse_recording_refused.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_recording_continuity: $(call objs,tests/test_recording_continuity.c src/devtools/save_storage_memory.c src/devtools/session_recorder.c src/devtools/replay_source.c src/p8/input.c src/devtools/input_json.c src/game/bindings.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_replay_control: $(call objs,tests/test_replay_control.c src/devtools/replay_control.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_marker_coherence: $(call objs,tests/test_marker_coherence.c src/devtools/save_storage_memory.c src/devtools/session_recorder.c src/devtools/replay_source.c src/p8/input.c src/devtools/input_json.c src/game/bindings.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_screencast: $(call objs,tests/test_screencast.c src/devtools/screencast.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# This test runs the real replay driver, hence $(TMP_DIR)/replay as a
# prerequisite: it replays the same fixture with and without a capture and
# compares the two dumps, so that the capture argument has no effect on the
# rest.
$(TMP_DIR)/test_headless_capture: $(call objs,tests/test_headless_capture.c src/devtools/screencast.c src/devtools/dump.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c src/p8/input.c src/devtools/input_json.c src/game/bindings.c) $(TMP_DIR)/replay tests/fixtures/screencast_capture_events.json tests/e2e/pad_title.json bindings.default.cfg | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_replay_from_title: $(call objs,tests/test_replay_from_title.c src/devtools/replay_source.c $(ENGINE_SRCS)) $(TMP_DIR)/replay tests/fixtures/title_pick_forgiving.json tests/fixtures/title_pick_reasonable.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_replay_usage: $(call objs,tests/test_replay_usage.c) $(TMP_DIR)/replay tests/fixtures/title_pick_forgiving.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_replay_from_resume: $(call objs,tests/test_replay_from_resume.c) $(TMP_DIR)/replay pico8/demo_record.hex bindings.default.cfg tools/end_state.sh tests/fixtures/title_pick_reasonable.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/frames_tool: $(call objs,src/cmd/frames.c src/devtools/screencast.c src/devtools/dump.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Runs the replay driver, then the frame emission tool.
$(TMP_DIR)/test_frames_tool: $(call objs,tests/test_frames_tool.c src/devtools/screencast.c src/p8/vram.c) $(TMP_DIR)/replay $(TMP_DIR)/frames_tool tests/fixtures/screencast_capture_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_frames_check: $(call objs,tests/test_frames_check.c) tools/frames_check.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_state_compare: $(call objs,tests/test_psx_state_compare.c) tools/psx_state_compare.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_pad_compare: $(call objs,tests/test_psx_pad_compare.c) tools/psx_pad_compare.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_digest_compare: $(call objs,tests/test_psx_digest_compare.c) tools/psx_digest_compare.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_capture_check: $(call objs,tests/test_psx_capture_check.c) tools/psx_capture_check.sh tools/psx_capture_record.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_capture_protocol: $(call objs,tests/test_psx_capture_protocol.c) tools/psx_capture_protocol_check.sh src/platform/psx_capture_protocol.h tools/psx_capture_frames.lua | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_pad_protocol: $(call objs,tests/test_psx_pad_protocol.c) tools/psx_pad_protocol_check.sh src/platform/psx_pad_protocol.h tools/psx_pad_play.lua | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_game_check: $(call objs,tests/test_psx_game_check.c) tools/psx_game_check.sh tools/psx_state_compare.sh $(TMP_DIR)/psx_pad_log $(TMP_DIR)/replay tests/e2e/pad_title.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_cue_check: $(call objs,tests/test_psx_cue_check.c) tools/psx_cue_check.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_cd_check: $(call objs,tests/test_psx_cd_check.c) tools/psx_cd_check.sh tools/psx_cue_check.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_boot: $(call objs,tests/test_psx_boot.c) tools/psx_boot.sh tools/psx_portable.sh tools/psx_until_exit.lua | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_emulator_setup: $(call objs,tests/test_psx_emulator_setup.c) tools/install_psx_emulator.sh tools/psx_portable.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_frame_budget: $(call objs,tests/test_psx_frame_budget.c) tools/psx_frame_budget.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_real_game_budget: $(call objs,tests/test_psx_real_game_budget.c) tools/psx_real_game_budget.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_short_run: $(call objs,tests/test_psx_short_run.c) tools/psx_short_run.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_probe: $(call objs,tests/test_psx_probe.c) tools/psx_probe.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_opt_level: $(call objs,tests/test_psx_opt_level.c) tools/psx_opt_level.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_full_game_check: $(call objs,tests/test_psx_full_game_check.c) tools/psx_full_game_check.sh \
		tools/psx_conditions.sh tools/psx_state_compare.sh tools/psx_digest_compare.sh tools/psx_boot.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_conditions: $(call objs,tests/test_psx_conditions.c) tools/psx_conditions.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_e2e_limits: $(call objs,tests/test_e2e_limits.c) tools/e2e_limits.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_state_peaks: $(call objs,tests/test_state_peaks.c) tools/state_peaks.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_end_state: $(call objs,tests/test_end_state.c) tools/end_state.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_demo_check: $(call objs,tests/test_demo_check.c) tools/demo_check.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_coverage_compare: $(call objs,tests/test_coverage_compare.c) tools/coverage_compare.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_guard: $(call objs,tests/test_float_guard.c) tools/float_guard.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_alloc_guard: $(call objs,tests/test_alloc_guard.c) tools/alloc_guard.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pointer_guard: $(call objs,tests/test_pointer_guard.c) tools/pointer_guard.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_grep: $(call objs,tests/test_float_grep.c) tools/float_grep.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_random_guard: $(call objs,tests/test_random_guard.c) tools/random_guard.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_button_guard: $(call objs,tests/test_button_guard.c) tools/button_guard.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_link: $(call objs,tests/test_float_link.c) tools/float_link.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_constants_check: $(call objs,tests/test_constants_check.c) tools/constants_compare.sh tools/constants_check.sh src/p8/fixed.c src/p8/fixed.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_palette: $(call objs,tests/test_palette.c src/p8/palette.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_hud: $(call objs,tests/test_hud.c src/game/hud.c src/game/text_image.c src/p8/image.c src/platform/fatal_pc.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c src/p8/fixed.c src/p8/assets.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_vec: $(call objs,tests/test_vec.c src/engine/vec.c src/p8/fixed.c src/devtools/fixed_test_double.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_geom: $(call objs,tests/test_geom.c src/engine/geom.c src/engine/vec.c src/p8/fixed.c src/devtools/fixed_test_double.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pointer_list: $(call objs,tests/test_pointer_list.c tests/heap_allocator.c src/engine/pointer_list.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_arena: $(call objs,tests/test_arena.c tests/heap_allocator.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_entity_id: $(call objs,tests/test_entity_id.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_entity_registry: $(call objs,tests/test_entity_registry.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_entity_id_list: $(call objs,tests/test_entity_id_list.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_value_list: $(call objs,tests/test_value_list.c tests/heap_allocator.c src/engine/value_list.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_entity_list: $(call objs,tests/test_entity_list.c tests/heap_allocator.c src/engine/entity_list.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)


$(TMP_DIR)/test_str: $(call objs,tests/test_str.c tests/heap_allocator.c src/p8/str.c src/p8/fixed.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_data_init: $(call objs,tests/test_data_init.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_map_tables: $(call objs,tests/test_map_tables.c $(DATA_ENGINE_SRCS) \
    src/game/planet_family_trade.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_tooltip_draw_at: $(call objs,tests/test_tooltip_draw_at.c tests/heap_allocator.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_toast: $(call asan_objs,tests/test_toast.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_message_box: $(call asan_objs,tests/test_message_box.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_live_slots: $(call asan_objs,tests/test_live_slots.c src/devtools/replay_source.c $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) $(TMP_DIR)/replay | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_modal_backdrop: $(call objs,tests/test_modal_backdrop.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_mouse: $(call objs,tests/test_mouse.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_main_mouse: $(call objs,tests/test_main_mouse.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_main_camera: $(call objs,tests/test_main_camera.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_camera: $(call objs,tests/test_camera.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_wormhole: $(call objs,tests/test_wormhole.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_background: $(call objs,tests/test_background.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_slipway: $(call objs,tests/test_slipway.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_structure_icon: $(call objs,tests/test_structure_icon.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_button: $(call objs,tests/test_button.c tests/heap_allocator.c src/devtools/largest_slots.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_planet: $(call objs,tests/test_planet.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_visual_random: $(call objs,tests/test_visual_random.c src/game/visual_random.c src/p8/fixed.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_gen: $(call objs,tests/test_gen.c tests/heap_allocator.c src/game/gen.c src/game/game_data.c src/p8/fixed.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_save_state: $(call objs,tests/test_save_state.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_save_storage_memory: $(call objs,tests/test_save_storage_memory.c tests/save_storage_contract.c src/devtools/save_storage_memory.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_save_storage_file: $(call objs,tests/test_save_storage_file.c tests/save_storage_contract.c src/platform/save_storage_file.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_structure: $(call objs,tests/test_structure.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_entity_kind: $(call objs,tests/test_entity_kind.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_spec_defaults: $(call objs,tests/test_spec_defaults.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_entity_header: $(call objs,tests/test_entity_header.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_planet_spec_defaults: $(call objs,tests/test_planet_spec_defaults.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_trading_hub: $(call objs,tests/test_trading_hub.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_processor: $(call objs,tests/test_processor.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_ascension: $(call objs,tests/test_ascension.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_protostar: $(call objs,tests/test_protostar.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_synthesizer: $(call objs,tests/test_synthesizer.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_slipgate: $(call objs,tests/test_slipgate.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_infraspace_gate: $(call objs,tests/test_infraspace_gate.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_burst: $(call objs,tests/test_burst.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

STATE_SRCS := src/devtools/state.c src/devtools/state_format.c

$(TMP_DIR)/test_rng_sync: $(call objs,tests/test_rng_sync.c src/devtools/save_check.c $(STATE_SRCS) $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) tests/e2e/pad_colonise.json pico8/gfx.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_save_worst_case: $(call objs,tests/test_save_worst_case.c $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) pico8/gfx.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_save_fuzz: $(call objs,tests/test_save_fuzz.c src/devtools/save_check.c $(STATE_SRCS) $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) pico8/gfx.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_resume_check: $(call objs,tests/test_resume_check.c src/devtools/save_check.c $(STATE_SRCS) src/devtools/replay_source.c $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) pico8/gfx.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_probe: $(call objs,tests/test_probe.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_help_screen: $(call objs,tests/test_help_screen.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_score_panel: $(call objs,tests/test_score_panel.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_ui: $(call objs,tests/test_ui.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_trade: $(call asan_objs,tests/test_trade.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_research_project: $(call objs,tests/test_research_project.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_layout_row: $(call objs,tests/test_layout_row.c tests/heap_allocator.c src/devtools/largest_slots.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_row_image: $(call objs,tests/test_row_image.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_text_image: $(call objs,tests/test_text_image.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_button_image: $(call objs,tests/test_button_image.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_economy: $(call objs,tests/test_economy.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_main_economy: $(call objs,tests/test_main_economy.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_loading_screen: $(call objs,tests/test_loading_screen.c $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_input_step: $(call objs,tests/test_input_step.c src/devtools/replay_source.c src/devtools/session_recorder.c $(STATE_SRCS) $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pad_nav: $(call objs,tests/test_pad_nav.c $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pad_panel: $(call objs,tests/test_pad_panel.c $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_entity_geom: $(call objs,tests/test_entity_geom.c tests/heap_allocator.c $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pad_target: $(call objs,tests/test_pad_target.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pad_jump: $(call objs,tests/test_pad_jump.c tests/heap_allocator.c $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pad_slipway: $(call objs,tests/test_pad_slipway.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pad_back: $(call objs,tests/test_pad_back.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pad_select: $(call objs,tests/test_pad_select.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pad_metrics: $(call objs,tests/test_pad_metrics.c src/devtools/pad_metrics.c $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_frame_times: $(call objs,tests/test_frame_times.c src/devtools/frame_times.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pad_caption: $(call objs,tests/test_pad_caption.c src/devtools/pad_caption.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c src/p8/assets.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_demo_source: $(call objs,tests/test_demo_source.c src/devtools/demo_source.c src/devtools/pad_caption.c src/devtools/replay_source.c src/devtools/save_storage_memory.c src/devtools/input_json.c src/p8/input.c src/game/bindings.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c src/p8/assets.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/demo_generate: $(call objs,src/cmd/demo_generate.c src/devtools/demo_source.c src/devtools/pad_caption.c src/devtools/replay_source.c src/devtools/save_storage_memory.c src/devtools/input_json.c src/p8/input.c src/game/bindings.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c src/p8/assets.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(DEMO_UNIT): pico8/demo.json $(TMP_DIR)/demo_generate
	@mkdir -p $(@D)
	$(TMP_DIR)/demo_generate $< $@

$(TMP_DIR)/test_demo_data: $(call objs,tests/test_demo_data.c src/devtools/replay_source.c src/devtools/pad_caption.c $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) pico8/demo.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pad_build: $(call objs,tests/test_pad_build.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pad_cycle: $(call objs,tests/test_pad_cycle.c src/game/pad_cycle.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_attract: $(call objs,tests/test_attract.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_demo_overlay: $(call objs,tests/test_demo_overlay.c src/game/demo_overlay.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c $(DEMO_UNIT)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_draw_layers_camera: $(call objs,tests/test_draw_layers_camera.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_demo: $(call objs,tests/test_demo.c src/p8/vram.c src/p8/assets.c src/p8/gfx.c src/p8/gfx_software.c src/p8/input.c src/devtools/input_json.c src/game/bindings.c src/devtools/dump.c) tests/fixtures/demo_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

REPLAY_SRCS := src/cmd/replay.c src/devtools/record_start.c src/devtools/pad_metrics.c src/devtools/screencast.c src/devtools/vram_digest.c src/devtools/draw_filter.c src/devtools/capture_list.c $(STATE_SRCS) src/devtools/replay_source.c $(BINDINGS_DEFAULT_UNIT) $(ENGINE_SRCS)

$(TMP_DIR)/replay: $(call objs,$(REPLAY_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

PSX_CAPTURE_COMPARE_SRCS := src/cmd/psx_capture_compare.c src/devtools/capture_compare.c src/devtools/capture_list.c \
	src/devtools/screencast.c src/platform/psx_colour.c src/p8/palette.c src/p8/vram.c

$(TMP_DIR)/psx_capture_compare: $(call objs,$(PSX_CAPTURE_COMPARE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

PSX_PAD_LOG_SRCS := src/cmd/psx_pad_log.c src/devtools/pad_log.c src/platform/psx_pad_map.c src/p8/input.c src/devtools/input_json.c src/game/bindings.c

$(TMP_DIR)/psx_pad_log: $(call objs,$(PSX_PAD_LOG_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

EMBED_REPLAY_SRCS := src/cmd/embed_replay.c src/devtools/replay_source.c src/devtools/save_storage_memory.c src/p8/input.c src/devtools/input_json.c src/game/bindings.c

$(TMP_DIR)/embed_replay: $(call objs,$(EMBED_REPLAY_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# `$(call embedded,<name>)` -> the generated source of that recording,
# under tmp/embedded/; its object follows from the generic %.c rule.
embedded = $(TMP_DIR)/embedded/$(1).c

# The name argument is the recording's path: the test reopens it from the
# embedded name to compare.
$(call embedded,test_embed_replay_pad): tests/e2e/pad_slipway.json $(TMP_DIR)/embed_replay
	@mkdir -p $(@D)
	./$(TMP_DIR)/embed_replay $< $< $@

$(call embedded,test_embed_replay_save): tests/e2e/pad_resume_at_start.json $(TMP_DIR)/embed_replay
	@mkdir -p $(@D)
	./$(TMP_DIR)/embed_replay $< $< $@

TEST_EMBED_REPLAY_SRCS := tests/test_embed_replay.c src/devtools/replay_source.c src/devtools/save_storage_memory.c src/p8/input.c src/devtools/input_json.c src/game/bindings.c

$(TMP_DIR)/test_embed_replay: $(call objs,$(TEST_EMBED_REPLAY_SRCS) $(call embedded,test_embed_replay_pad)) $(TMP_DIR)/embed_replay tests/fixtures/events_mouse_recording_refused.json pico8/demo.json tests/e2e/pad_swap_aim_menu.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_embed_replay_save: $(call objs,$(TEST_EMBED_REPLAY_SRCS) $(call embedded,test_embed_replay_save)) $(TMP_DIR)/embed_replay tests/fixtures/events_mouse_recording_refused.json pico8/demo.json tests/e2e/pad_swap_aim_menu.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/format_self_test: $(call objs,src/cmd/format_self_test.c src/devtools/format_self_test.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_format_self_test: $(call objs,tests/test_format_self_test.c src/devtools/format_self_test.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_vram_digest: $(call objs,tests/test_vram_digest.c src/devtools/vram_digest.c src/devtools/screencast.c src/p8/sprite_sheet.c src/p8/vram.c) $(TMP_DIR)/replay tests/e2e/pad_title.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_format: $(call objs,tests/test_format.c src/p8/format.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_state: $(call objs,tests/test_state.c $(STATE_SRCS) $(ENGINE_SRCS)) $(TMP_DIR)/replay | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_state_format: $(call objs,tests/test_state_format.c $(STATE_SRCS) src/devtools/replay_source.c $(ENGINE_SRCS)) tests/e2e/pad_slipway.json pico8/gfx.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_save_check: $(call objs,tests/test_save_check.c src/devtools/save_check.c $(STATE_SRCS) $(ENGINE_SRCS)) pico8/gfx.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_save_load: $(call objs,tests/test_save_load.c src/devtools/save_check.c $(STATE_SRCS) $(ENGINE_SRCS)) pico8/gfx.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_cart: $(call objs,tests/test_cart.c src/devtools/save_check.c $(STATE_SRCS) $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) pico8/gfx.json pico8/demo_record.hex | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_attract_demo: $(call objs,tests/test_attract_demo.c src/devtools/replay_source.c src/devtools/save_check.c $(STATE_SRCS) $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) pico8/gfx.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_record_start: $(call objs,tests/test_record_start.c src/devtools/record_start.c $(ENGINE_SRCS) $(BINDINGS_DEFAULT_UNIT)) pico8/gfx.json pico8/demo_record.hex | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_background_draw: $(call objs,tests/test_background_draw.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# Under ASan, with no uninstrumented twin: the crash it targets depends on the
# layout of the allocator (the same replay exits 139 at `-O0` and 0 at `-O1`),
# so only the overflow itself is a reliable oracle.
$(TMP_DIR)/test_planet_exports_row: $(call asan_objs,tests/test_planet_exports_row.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_zone: $(call asan_objs,tests/test_zone.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_allocator: $(call asan_objs,tests/test_allocator.c tests/heap_allocator.c src/engine/entity_list.c src/engine/pointer_list.c src/p8/str.c src/engine/collect.c src/engine/geom.c src/engine/vec.c src/p8/fixed.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_particle_zone: $(call asan_objs,tests/test_particle_zone.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_ui_zone: $(call asan_objs,tests/test_ui_zone.c src/game/ui_zone.c src/engine/slot_group.c src/engine/slot_tally.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_slot_tally: $(call asan_objs,tests/test_slot_tally.c src/engine/slot_tally.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_exit_queue: $(call asan_objs,tests/test_exit_queue.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_frame_zone: $(call asan_objs,tests/test_frame_zone.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_game_zone: $(call asan_objs,tests/test_game_zone.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_burst_exports_row: $(call objs,tests/test_burst_exports_row.c $(DATA_ENGINE_SRCS) src/game/planet_family.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_probe_spawn_particle_order: $(call objs,tests/test_probe_spawn_particle_order.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# A reduced source list rather than $(ENGINE_SRCS): this test never calls
# cart_load(). Measured by linking, not guessed.
$(TMP_DIR)/test_layout_nil_item_draws_nothing: $(call objs,tests/test_layout_nil_item_draws_nothing.c tests/heap_allocator.c src/p8/assets.c src/engine/collect.c src/engine/dispatch_context.c src/p8/fixed.c src/engine/geom.c src/p8/gfx.c src/p8/gfx_software.c src/game/hud.c src/game/layout.c src/game/text_image.c src/p8/image.c src/engine/pointer_list.c src/game/map_tables.c src/p8/vram.c src/p8/str.c src/engine/vec.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# The longest first: under `-j`, make starts it first.
UNIT_TESTS := \
    test_vram \
    test_asset_loading \
    test_sprite_sheet \
    test_gfx \
    test_draw_backend \
    test_image \
    test_draw_cost \
    test_draw_meter \
    test_psx_vram_layout \
    test_psx_colour \
    test_psx_tiles \
    test_draw_filter \
    test_draw_timing \
    test_capture_list \
    test_short_run_cut \
    test_situation \
    test_situations \
    test_situation_table \
    test_situation_report \
    test_capture_compare \
    test_input \
    test_dump \
    test_fixed \
    test_fixed_ubsan \
    test_vec \
    test_geom \
    test_pointer_list \
    test_entity_list \
    test_entity_id \
    test_entity_registry \
    test_entity_id_list \
    test_value_list \
    test_str \
    test_data_init \
    test_map_tables \
    test_draw_layers_camera \
    test_demo \
    test_keymap \
    test_psx_pad_map \
    test_pad_log \
    test_bindings \
    test_replay_source \
    test_embed_replay \
    test_embed_replay_save \
    test_recording_continuity \
    test_replay_control \
    test_marker_coherence \
    test_screencast \
    test_headless_capture \
    test_replay_from_title \
    test_replay_usage \
    test_replay_from_resume \
    test_palette \
    test_hud \
    test_frames_tool \
    test_frames_check \
    test_format_self_test \
    test_format \
    test_vram_digest \
    test_e2e_limits \
    test_state_peaks \
    test_end_state \
    test_coverage_compare \
    test_float_guard \
    test_alloc_guard \
    test_pointer_guard \
    test_float_grep \
    test_random_guard \
    test_button_guard \
    test_float_link \
    test_constants_check \
    test_background_draw \
    test_probe_spawn_particle_order \
    test_planet_exports_row \
    test_zone \
    test_allocator \
    test_arena \
    test_frame_zone \
    test_ui_zone \
    test_slot_tally \
    test_particle_zone \
    test_game_zone \
    test_exit_queue \
    test_burst_exports_row \
    test_layout_nil_item_draws_nothing \
    test_tooltip_draw_at \
    test_toast \
    test_message_box \
    test_state \
    test_state_format \
    test_modal_backdrop \
    test_mouse \
    test_main_mouse \
    test_camera \
    test_main_camera \
    test_wormhole \
    test_background \
    test_slipway \
    test_structure_icon \
    test_button \
    test_planet \
    test_gen \
    test_visual_random \
    test_save_state \
    test_save_storage_memory \
    test_save_storage_file \
    test_save_check \
    test_save_load \
    test_cart \
    test_record_start \
    test_structure \
    test_trading_hub \
    test_entity_kind \
    test_spec_defaults \
    test_entity_header \
    test_planet_spec_defaults \
    test_processor \
    test_ascension \
    test_protostar \
    test_synthesizer \
    test_slipgate \
    test_infraspace_gate \
    test_burst \
    test_probe \
    test_help_screen \
    test_score_panel \
    test_ui \
    test_trade \
    test_research_project \
    test_layout_row \
    test_row_image \
    test_text_image \
    test_button_image \
    test_economy \
    test_main_economy \
    test_input_step \
    test_loading_screen \
    test_pad_nav \
    test_pad_panel \
    test_entity_geom \
    test_psx_gpu_textures \
    test_psx_clut_cache \
    test_psx_sprite \
    test_psx_image \
    test_psx_text \
    test_psx_pattern_cache \
    test_psx_slot_uploads \
    test_psx_gpu_peaks \
    test_psx_pattern \
    test_pad_target \
    test_pad_jump \
    test_pad_slipway \
    test_pad_back \
    test_pad_select \
    test_pad_build \
    test_pad_cycle \
    test_pad_metrics \
    test_pad_caption \
    test_frame_times \
    test_demo_source \
    test_demo_data \
    test_attract \
    test_demo_overlay

# One run per test, each its own target: that is what `-j` runs in parallel.
UNIT_RUNS := $(addprefix unit-run-,$(UNIT_TESTS))

UNIT_RUNS_SPECIAL := unit-run-test_zone unit-run-test_allocator unit-run-test_frame_zone unit-run-test_ui_zone unit-run-test_particle_zone unit-run-test_game_zone unit-run-test_exit_queue unit-run-test_toast unit-run-test_message_box unit-run-test_live_slots unit-run-test_trade

unit: $(UNIT_RUNS) ## builds and runs the binaries of tests/

PSX_TOOLS_TESTS := test_psx_state_compare test_psx_pad_compare test_psx_digest_compare test_psx_capture_check test_psx_capture_protocol \
	test_psx_pad_protocol test_psx_game_check test_psx_cue_check test_psx_cd_check test_psx_boot \
	test_psx_full_game_check test_psx_frame_budget test_psx_real_game_budget test_psx_emulator_setup test_psx_conditions test_psx_short_run \
	test_psx_probe test_psx_opt_level test_steady_windows test_short_run_situations

psx-tools-test: $(addprefix $(TMP_DIR)/,$(PSX_TOOLS_TESTS)) ## host tests of the PSX check scripts
	@for t in $(PSX_TOOLS_TESTS); do ./$(TMP_DIR)/$$t || exit 1; done

$(filter-out $(UNIT_RUNS_SPECIAL),$(UNIT_RUNS)): unit-run-%: $(TMP_DIR)/%
	./$<

unit-run-test_zone: $(TMP_DIR)/test_zone
	ASAN_OPTIONS=detect_leaks=0 ./$<

unit-run-test_allocator: $(TMP_DIR)/test_allocator
	ASAN_OPTIONS=detect_leaks=0 ./$<

unit-run-test_frame_zone: $(TMP_DIR)/test_frame_zone
	ASAN_OPTIONS=detect_leaks=0 ./$<

unit-run-test_ui_zone: $(TMP_DIR)/test_ui_zone
	ASAN_OPTIONS=detect_leaks=0 ./$<

unit-run-test_particle_zone: $(TMP_DIR)/test_particle_zone
	ASAN_OPTIONS=detect_leaks=0 ./$<

unit-run-test_game_zone: $(TMP_DIR)/test_game_zone
	ASAN_OPTIONS=detect_leaks=0 ./$<

unit-run-test_exit_queue: $(TMP_DIR)/test_exit_queue
	ASAN_OPTIONS=detect_leaks=0 ./$<

unit-run-test_toast: $(TMP_DIR)/test_toast
	ASAN_OPTIONS=detect_leaks=0 ./$<

unit-run-test_message_box: $(TMP_DIR)/test_message_box
	ASAN_OPTIONS=detect_leaks=0 ./$<

unit-run-test_live_slots: $(TMP_DIR)/test_live_slots
	ASAN_OPTIONS=detect_leaks=0 ./$<

unit-run-test_trade: $(TMP_DIR)/test_trade
	ASAN_OPTIONS=detect_leaks=0 ./$<

# These two tests read tmp/replay_dump.txt, the default dump of tmp/replay,
# and test_replay_from_resume writes it: under `-j`, never at the same time.
unit-run-test_frames_tool: | unit-run-test_headless_capture
unit-run-test_replay_from_resume: | unit-run-test_frames_tool

.PHONY: $(UNIT_RUNS)

# Fails before any compilation rather than replay a default file.
ifneq ($(filter replay,$(MAKECMDGOALS)),)
    ifeq ($(strip $(EVENTS)),)
        $(error EVENTS missing: make replay EVENTS=<events.json>)
    endif
endif

replay: $(TMP_DIR)/replay ## EVENTS=<events> [SAVE_HEX=<file>] [DIGEST=<file>]: replays one file headless, dump in tmp/replay_dump.txt
	$(if $(ATTRACT_STATE),ATTRACT_STATE=$(ATTRACT_STATE) )$(if $(REPLAY_STARTS),REPLAY_STARTS=$(REPLAY_STARTS) )$(if $(SAVE_HEX),SAVE_HEX=$(SAVE_HEX) )$(if $(DIGEST),DIGEST=$(DIGEST) )./$(TMP_DIR)/replay $(EVENTS)

test: unit constants-check float-grep float-guard alloc-guard pointer-guard random-guard button-guard data-guard p8-guard pad-author-self-check ## fast loop: unit tests and static guards (~5 s)

test-all: test e2e ## test + e2e, the full suite

# The e2e: <name>.json (the same format as a raylib recording), <name>.txt
# (what the replay does, the frames of the key actions) and <name>.json.sha256.
# All of them start from the title screen.
E2E_DIR := tests/e2e
E2E_EVENTS := $(sort $(wildcard $(E2E_DIR)/*.json))
# One `<events> reason` line per e2e allowed up to 600 frames instead of 180.
E2E_EXCEPTIONS := tools/e2e_long_exceptions.txt

ifeq ($(UNAME_S),Darwin)
    SHA256_CMD := shasum -a 256
else
    SHA256_CMD := sha256sum
endif

# An interrupted capture would leave a partial file that make would believe up
# to date, and the next hash would be taken over it.
.DELETE_ON_ERROR:

# The sha256 of the capture of each e2e, versioned next to it as
# <events>.sha256. The capture is the raw concatenation of the frames: its hash
# also covers their count. The `-O0` binary of make test: no second tree to
# build, and readable under gdb.
FRAMES_TOOL := $(TMP_DIR)/frames_tool
FRAMES_DIR := $(TMP_DIR)/frames

frames_capture = $(FRAMES_DIR)/$(1).bin
frames_current = $(FRAMES_DIR)/$(1).sha256
frames_reference = $(1).sha256

# Without .SECONDARY, make would delete these intermediate captures, which
# e2e-ref compares to the reference capture.
.SECONDARY: $(foreach e,$(E2E_EVENTS),$(call frames_capture,$(e)))

# Each capture has its own text dump: the default dump, tmp/replay_dump.txt,
# is read by two tests that run in parallel.
$(FRAMES_DIR)/%.bin: % $(TMP_DIR)/replay
	@mkdir -p $(@D)
	REPLAY_DEPS=$(@:.bin=.d) ./$(TMP_DIR)/replay $< $@ $(@:.bin=.dump.txt) > /dev/null

-include $(wildcard $(FRAMES_DIR)/$(E2E_DIR)/*.json.d)

# The assignment keeps the exit status of sha256, which a pipe into cut would
# lose.
frames_hash = @mkdir -p $(@D); h=$$($(SHA256_CMD) $<) && echo "$${h%% *}" > $@

$(FRAMES_DIR)/%.sha256: $(FRAMES_DIR)/%.bin
	$(frames_hash)

e2e: $(foreach e,$(E2E_EVENTS),$(call frames_current,$(e))) tools/e2e_limits.sh tools/frames_check.sh $(E2E_EXCEPTIONS) ## replays tests/e2e/ and checks each capture against its .json.sha256 (~1 s)
	@status=0; \
	sh tools/e2e_limits.sh $(E2E_EXCEPTIONS) $(E2E_EVENTS) || status=1; \
	sh tools/frames_check.sh $(foreach e,$(E2E_EVENTS),$(e) $(call frames_reference,$(e)) $(call frames_current,$(e))) || status=1; \
	echo "e2e: $(words $(E2E_EVENTS)) e2e in $(E2E_DIR)"; \
	exit $$status

e2e-baseline: $(foreach e,$(E2E_EVENTS),$(call frames_current,$(e))) ## sets or resets the .json.sha256 of tests/e2e/ from the current code
	@$(foreach e,$(E2E_EVENTS),cp $(call frames_current,$(e)) $(call frames_reference,$(e)) && echo "e2e: reference set in $(call frames_reference,$(e))" && ) true

e2e-ref: $(foreach e,$(E2E_EVENTS),$(if $(wildcard $(call frames_reference,$(e))),$(call frames_current,$(e)))) $(FRAMES_TOOL) tools/frames_ref.sh ## for each diverging e2e, replays the commit that set its reference and gives the first diverging frame
	@sh tools/frames_ref.sh ./$(FRAMES_TOOL) $(FRAMES_DIR) $(TMP_DIR)/frames-ref $(foreach e,$(E2E_EVENTS),$(e) $(call frames_reference,$(e)) $(call frames_current,$(e)) $(call frames_capture,$(e)))

# offsetof, void *self, a field read by name, a spec or defaults table: the
# model the composition migration removed, which a port of the cart brings
# back first.
DATA_GUARD_PATTERN := \boffsetof\b|\bvoid[[:space:]]*\*[[:space:]]*(const[[:space:]]+)?self|\b[a-z0-9_]*field_truthy\(|(->|\.)(set_)?field\b|\b[a-z0-9_]+_take_[a-z0-9_]+\(|\boverrides\b|\bg_[a-z0-9_]+_(spec|defaults)\b|\b[a-z0-9_]+_default_(fixed|table|vec|string|box|list|bool|hf)\b

# grep returns 1 when nothing matches; anything above is an error.
data-guard: ## fails on a pre-composition model pattern in src/ or tests/
	@status=0; grep -rnE '$(DATA_GUARD_PATTERN)' --include='*.c' --include='*.h' src tests || status=$$?; \
	if [ $$status -eq 0 ]; then echo "data-guard: pre-composition pattern above"; exit 1; fi; \
	[ $$status -eq 1 ] || exit $$status; \
	echo "data-guard: clean"

.PHONY: data-guard

P8_GUARD_PATTERN := \bp8_(stat|sfx|music|play_sfx|peek|poke|memcpy|memory_t)\b|\bP8_ADDR_[A-Z_]+|\b0x(5[fF]80|3101|5[fF]2[dD])\b|\b(entities_by_tag|entities_by_message|draw_layers|entity_set_state|entity_init_instance)\("|\bentity_state_of\b|map\.json

p8-guard: ## fails on a PICO-8-only API in src/ or tests/
	@status=0; grep -rnE '$(P8_GUARD_PATTERN)' --include='*.c' --include='*.h' src tests || status=$$?; \
	if [ $$status -eq 0 ]; then echo "p8-guard: PICO-8-only pattern above"; exit 1; fi; \
	[ $$status -eq 1 ] || exit $$status; \
	echo "p8-guard: clean"

.PHONY: p8-guard

$(TMP_DIR)/replay_sanitize: $(call sanitize_objs,$(REPLAY_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) $(UBSAN_CFLAGS) -o $@ $(filter %.o,$^)

SANITIZE_DIR := $(TMP_DIR)/sanitize
SANITIZE_RUNS := $(addprefix sanitize-run-,$(E2E_EVENTS))

$(SANITIZE_RUNS): sanitize-run-%: % $(TMP_DIR)/replay_sanitize
	@mkdir -p $(SANITIZE_DIR)/$(<D)
	@./$(TMP_DIR)/replay_sanitize $< /dev/null $(SANITIZE_DIR)/$<.dump.txt > $(SANITIZE_DIR)/$<.log 2>&1 || { cat $(SANITIZE_DIR)/$<.log; echo "sanitize: $< dirty"; exit 1; }

sanitize-net: $(SANITIZE_RUNS) unit-run-test_live_slots ## replays tests/e2e/ under ASan+UBSan, then test_live_slots (~17 s)
	@echo "sanitize: $(words $(SANITIZE_RUNS)) clean"

.PHONY: sanitize-net $(SANITIZE_RUNS)

FULL_GAME_EVENTS := worst-case/full_game.json worst-case/full_game_new_game.json worst-case/full_game_pause_restart.json worst-case/full_game_pause_idle.json worst-case/title_idle.json
FULL_GAME_DIR := $(TMP_DIR)/full-game

full-game: $(FULL_GAME_EVENTS) $(TMP_DIR)/replay ## replays the worst-case/ games to their last frame, fails on any p8_fatal or full zone
	@mkdir -p $(FULL_GAME_DIR)
	@$(foreach e,$(FULL_GAME_EVENTS),./$(TMP_DIR)/replay $(e) /dev/null $(FULL_GAME_DIR)/$(notdir $(e)).dump.txt > $(FULL_GAME_DIR)/$(notdir $(e)).log 2>&1 || { cat $(FULL_GAME_DIR)/$(notdir $(e)).log; echo "full-game: $(e) stopped"; exit 1; }; echo "full-game: $(e) clean"; ) true

.PHONY: full-game

PAD_AUTHOR_SRCS := src/cmd/pad_author.c src/devtools/pad_author.c src/devtools/pad_caption.c src/devtools/record_start.c src/devtools/pad_metrics.c src/devtools/situation.c $(STATE_SRCS) $(BINDINGS_DEFAULT_UNIT) $(ENGINE_SRCS)
PAD_AUTHOR_SCRIPTS := $(wildcard worst-case/*.pad)

$(TMP_DIR)/pad_author: $(call objs,$(PAD_AUTHOR_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

ifneq ($(strip $(SCRIPT)),)
pad-author: $(TMP_DIR)/pad_author $(TMP_DIR)/replay ## [SCRIPT=<script> [OUT=<events>] [STOP_AT=<label>]]: regenerates the .json of the .pad scripts
	./$(TMP_DIR)/pad_author $(SCRIPT) $(or $(OUT),$(TMP_DIR)/pad-author/$(basename $(notdir $(SCRIPT))).json)$(if $(STOP_AT), --stop-at $(STOP_AT))
else
pad-author: $(TMP_DIR)/pad_author $(TMP_DIR)/replay
	@$(foreach s,$(PAD_AUTHOR_SCRIPTS),./$(TMP_DIR)/pad_author $(s) $(s:.pad=.json) || exit 1; ) true
	@./$(TMP_DIR)/pad_author pico8/demo.pad pico8/demo.json --no-aim-budget
endif

PAD_AUTHOR_SELF_CHECK_SCRIPT := tools/pad_author_proofs/pad_probe.frames.pad

pad-author-self-check: $(TMP_DIR)/pad_author $(TMP_DIR)/replay $(PAD_AUTHOR_SELF_CHECK_SCRIPT)
	@./$(TMP_DIR)/pad_author $(PAD_AUTHOR_SELF_CHECK_SCRIPT) $(TMP_DIR)/pad-author/self_check.json --no-aim-budget > /dev/null
	@echo "pad-author-self-check: $(PAD_AUTHOR_SELF_CHECK_SCRIPT) replays to pad_author's own state"

.PHONY: pad-author-self-check

PAD_AUTHOR_FRAME_PROOFS := $(wildcard tools/pad_author_proofs/*.frames.pad)
PAD_AUTHOR_END_STATE_PROOFS := $(wildcard tools/pad_author_proofs/*.end-state.pad)
PAD_AUTHOR_REFUSAL_PROOFS := $(wildcard tools/pad_author_proofs/*.refused.pad)
PAD_AUTHOR_RECORD_PROOFS := $(wildcard tools/pad_author_proofs/*.record.pad)
pad_author_proof_json = $(TMP_DIR)/pad-author/$(notdir $(1:.pad=.json))
pad_author_proof_stem = $(TMP_DIR)/pad-author/$(notdir $(1:.pad=))
pad_author_proof_e2e = $(E2E_DIR)/$(basename $(basename $(notdir $(1)))).json

# A refusal proof's first line is `# expect: <text of the error>`.
pad-author-proof: $(TMP_DIR)/pad_author $(TMP_DIR)/replay $(PAD_AUTHOR_FRAME_PROOFS) $(PAD_AUTHOR_END_STATE_PROOFS) $(PAD_AUTHOR_REFUSAL_PROOFS) $(PAD_AUTHOR_RECORD_PROOFS) tools/end_state.sh ## runs the pad_author proofs of tools/pad_author_proofs/
	@mkdir -p $(TMP_DIR)/pad-author
	@$(foreach p,$(PAD_AUTHOR_FRAME_PROOFS),./$(TMP_DIR)/pad_author $(p) $(call pad_author_proof_json,$(p)) --no-aim-budget > /dev/null || exit 1; cmp $(call pad_author_proof_json,$(p)) $(call pad_author_proof_e2e,$(p)) || exit 1; echo "pad-author-proof: $(p) regenerates $(call pad_author_proof_e2e,$(p))"; ) true
	@$(foreach p,$(PAD_AUTHOR_END_STATE_PROOFS),STATE=$(TMP_DIR)/pad-author/$(notdir $(p:.pad=.state.txt)) ./$(TMP_DIR)/pad_author $(p) $(call pad_author_proof_json,$(p)) --no-aim-budget > /dev/null || exit 1; sh tools/end_state.sh check $(call pad_author_proof_e2e,$(p)) $(TMP_DIR)/pad-author/$(notdir $(p:.pad=.state.txt)) $(call pad_author_proof_e2e,$(p)).end-state || exit 1; echo "pad-author-proof: $(p) reaches $(call pad_author_proof_e2e,$(p)).end-state"; ) true
	@$(foreach p,$(PAD_AUTHOR_REFUSAL_PROOFS),! ./$(TMP_DIR)/pad_author $(p) $(call pad_author_proof_json,$(p)) > /dev/null 2> $(TMP_DIR)/pad-author/refusal.txt || { echo "pad-author-proof: $(p) was not refused"; exit 1; }; grep -qF "$$(sed -n '1s/^# expect: //p' $(p))" $(TMP_DIR)/pad-author/refusal.txt || { cat $(TMP_DIR)/pad-author/refusal.txt; echo "pad-author-proof: $(p) refused for another reason"; exit 1; }; echo "pad-author-proof: $(p) refused: $$(sed -n '1s/^# expect: //p' $(p))"; ) true
	@$(foreach p,$(PAD_AUTHOR_RECORD_PROOFS),STATE=$(call pad_author_proof_stem,$(p)).state.txt ./$(TMP_DIR)/pad_author $(p) $(call pad_author_proof_json,$(p)) --no-aim-budget > /dev/null || exit 1; sh tools/end_state.sh normalize $(call pad_author_proof_stem,$(p)).state.txt > $(call pad_author_proof_stem,$(p)).reference.txt || exit 1; REPLAY_STARTS=2 STATE=$(call pad_author_proof_stem,$(p)).starts.txt ./$(TMP_DIR)/replay $(call pad_author_proof_json,$(p)) > /dev/null || exit 1; sh tools/end_state.sh check $(call pad_author_proof_json,$(p)) $(call pad_author_proof_stem,$(p)).starts.txt.start1 $(call pad_author_proof_stem,$(p)).reference.txt $(call pad_author_proof_json,$(p)) $(call pad_author_proof_stem,$(p)).starts.txt $(call pad_author_proof_stem,$(p)).reference.txt || exit 1; echo "pad-author-proof: $(p) replays from the record, twice, to pad_author's end state"; ) true

PAD_METRICS_DIR := $(TMP_DIR)/pad-metrics
PAD_METRICS_GAMES := $(filter-out worst-case/title_idle.json,$(FULL_GAME_EVENTS))
pad_metrics_name = $(PAD_METRICS_DIR)/$(basename $(notdir $(1)))
# $(1) the events, $(2) 1 to fail on a section 4 miss, $(3) the lines to print from the report.
pad_metrics_run = PAD_METRICS=$(call pad_metrics_name,$(1)).txt ./$(TMP_DIR)/replay $(1) /dev/null $(call pad_metrics_name,$(1)).dump.txt > /dev/null || exit 1; echo "pad-metrics: $(1) -> $(call pad_metrics_name,$(1)).txt"; grep '$(3)' $(call pad_metrics_name,$(1)).txt; $(if $(filter 1,$(2)),grep -qx 'section4 verdict ok' $(call pad_metrics_name,$(1)).txt || { echo "pad-metrics: $(1) misses section 4"; exit 1; };)
PAD_METRICS_PROOF := tools/pad_author_proofs/pad_colonise.frames.pad
PAD_METRICS_PROOF_E2E := $(E2E_DIR)/pad_colonise.json
PAD_METRICS_PROOF_REQUIRED := dead f18 press 7 aim required;dead f19 press 6 aim required;dead f295 press 6 jump required

pad-metrics: $(TMP_DIR)/replay ## EVENTS=<events> [CHECK=1]: pad usage report in tmp/pad-metrics/
	@test -n "$(EVENTS)" || { echo "pad-metrics: EVENTS=<events.json> is required"; exit 2; }
	@mkdir -p $(PAD_METRICS_DIR)
	@$(call pad_metrics_run,$(EVENTS),$(CHECK),)

pad-metrics-worst-case: $(TMP_DIR)/replay $(PAD_METRICS_GAMES) ## pad-metrics with CHECK=1 on the re-authored worst-case/ games
	@mkdir -p $(PAD_METRICS_DIR)
	@$(foreach e,$(PAD_METRICS_GAMES),$(call pad_metrics_run,$(e),1,^section4 )) true

pad-metrics-check: $(TMP_DIR)/pad_author $(TMP_DIR)/replay $(PAD_METRICS_PROOF) $(PAD_METRICS_PROOF_E2E) ## regenerates pad_colonise.json and checks its pad-metrics report
	@mkdir -p $(PAD_METRICS_DIR)
	@./$(TMP_DIR)/pad_author $(PAD_METRICS_PROOF) $(PAD_METRICS_DIR)/pad_colonise.frames.json --no-aim-budget > /dev/null
	@cmp $(PAD_METRICS_DIR)/pad_colonise.frames.json $(PAD_METRICS_PROOF_E2E)
	@$(call pad_metrics_run,$(PAD_METRICS_PROOF_E2E),,^section4 verdict)
	@grep '^total ' $(TMP_DIR)/pad-author/pad_colonise.modes.txt > $(PAD_METRICS_DIR)/pad_colonise.generator-totals.txt
	@grep '^total ' $(PAD_METRICS_DIR)/pad_colonise.txt | diff $(PAD_METRICS_DIR)/pad_colonise.generator-totals.txt - || { echo "pad-metrics-check: the totals differ from the generator's"; exit 1; }
	@required=$$(grep -E ' (required|dead-required)$$' $(PAD_METRICS_DIR)/pad_colonise.txt | paste -sd ';' -); test "$$required" = "$(PAD_METRICS_PROOF_REQUIRED)" || { echo "pad-metrics-check: required group is '$$required', not '$(PAD_METRICS_PROOF_REQUIRED)'"; exit 1; }
	@echo "pad-metrics-check: totals equal the generator's; the required group is exactly f18's button 7, f19's button 6 and f295's button 6"

.PHONY: pad-metrics pad-metrics-worst-case pad-metrics-check

SITUATIONS_SRCS := src/cmd/situations.c src/devtools/situation.c src/devtools/pad_metrics.c src/devtools/record_start.c src/devtools/replay_source.c $(BINDINGS_DEFAULT_UNIT) $(ENGINE_SRCS)
SITUATIONS_EVENTS := worst-case/full_game.json

$(TMP_DIR)/situations: $(call objs,$(SITUATIONS_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

situations: $(TMP_DIR)/situations ## [EVENTS=<events>] [SITUATIONS_OUT=<file>]: one situation line per frame
	@events=$(or $(EVENTS),$(SITUATIONS_EVENTS)); out=$(or $(SITUATIONS_OUT),$(TMP_DIR)/situations-table/$$(basename $$events .json).txt); \
	mkdir -p $$(dirname $$out) && ./$(TMP_DIR)/situations $$events $$out && echo "situations: $$events -> $$out"

SITUATIONS_TABLE := psx/perf/full-run-situations.txt

situations-check: $(TMP_DIR)/situations ## checks the full_game.json situation table against psx/perf/
	@out=$(TMP_DIR)/situations-table/full_game.txt; mkdir -p $$(dirname $$out) && ./$(TMP_DIR)/situations $(SITUATIONS_EVENTS) $$out || exit 1; \
	first=$$(diff $(SITUATIONS_TABLE) $$out | sed -n 's/^[<>] \([0-9][0-9]*\) .*/\1/p' | sort -n | head -1); \
	if cmp -s $(SITUATIONS_TABLE) $$out; then echo "situations-check: $$out equals $(SITUATIONS_TABLE)"; \
	else echo "situations-check: $$out differs from $(SITUATIONS_TABLE), first at frame $${first:-header}"; exit 1; fi

situation-report: ## [FRAMES=<table>] [SITUATIONS=<table>]: frames and costs per situation
	@sh tools/situation_report.sh $(or $(FRAMES),psx/perf/full-run-frames.txt) $(or $(SITUATIONS),$(SITUATIONS_TABLE))

steady-windows: ## [FRAMES=<table>] [SITUATIONS=<table>]: candidate windows per steady situation
	@sh tools/steady_windows.sh $(or $(FRAMES),psx/perf/full-run-frames.txt) $(or $(SITUATIONS),$(SITUATIONS_TABLE)) 263 15 3

.PHONY: situations situations-check situation-report steady-windows

WORST_CASE_PEAKS_TABLE := worst-case/peaks-8c4bbf4.txt

worst-case-peaks: $(addprefix state-run-,$(FULL_GAME_EVENTS)) tools/worst_case_peaks.sh $(WORST_CASE_PEAKS_TABLE) ## checks the worst-case/ games against worst-case/peaks-8c4bbf4.txt
	@sh tools/worst_case_peaks.sh $(WORST_CASE_PEAKS_TABLE) $(TMP_DIR)/worst-case-peaks.txt $(foreach e,$(FULL_GAME_EVENTS),$(e) $(call state_file,$(e)))

.PHONY: pad-author pad-author-proof worst-case-peaks

DEMO_CHECK_DIR := $(TMP_DIR)/demo-check

demo-check: $(TMP_DIR)/test_demo_check $(TMP_DIR)/test_attract_demo $(TMP_DIR)/pad_author $(TMP_DIR)/replay $(TMP_DIR)/replay_sanitize pico8/demo.pad pico8/demo.json pico8/demo_record.hex worst-case/attract_cycle.json tests/e2e/pad_tech_costs.json worst-case/title_idle.json.end-state tools/demo_check_run.sh tools/demo_check.sh tools/end_state.sh ## checks the attract demo end to end, files in tmp/demo-check/ (never alongside test-all or pad-author)
	@sh tools/demo_check_run.sh $(DEMO_CHECK_DIR) ./$(TMP_DIR)/test_demo_check ./$(TMP_DIR)/test_attract_demo ./$(TMP_DIR)/pad_author ./$(TMP_DIR)/replay ./$(TMP_DIR)/replay_sanitize pico8/demo.pad pico8/demo.json worst-case/attract_cycle.json tests/e2e/pad_tech_costs.json worst-case/title_idle.json.end-state '$(SHA256_CMD)'

.PHONY: demo-check

rng-sync-probe: $(TMP_DIR)/test_rng_sync ## runs tests/test_rng_sync
	./$<

.PHONY: rng-sync-probe

save-worst-case: $(TMP_DIR)/test_save_worst_case $(FULL_GAME_EVENTS) ## fails if a save record of a worst-case/ game exceeds 8 KiB or does not round-trip
	./$< $(FULL_GAME_EVENTS)

.PHONY: save-worst-case

save-fuzz: $(TMP_DIR)/test_save_fuzz worst-case/full_game.json ## resumes every save of full_game.json and compares it with the original
	./$< worst-case/full_game.json

.PHONY: save-fuzz

ifneq ($(filter resume-check,$(MAKECMDGOALS)),)
    ifeq ($(strip $(EVENTS)),)
        $(error EVENTS missing: make resume-check EVENTS=<events.json> BEFORE=<frame> AFTER=<frame>)
    endif
    ifeq ($(strip $(BEFORE)),)
        $(error BEFORE missing: make resume-check EVENTS=<events.json> BEFORE=<frame> AFTER=<frame>)
    endif
    ifeq ($(strip $(AFTER)),)
        $(error AFTER missing: make resume-check EVENTS=<events.json> BEFORE=<frame> AFTER=<frame>)
    endif
endif

resume-check: $(TMP_DIR)/test_resume_check $(EVENTS) ## EVENTS=<events> BEFORE=<frame> AFTER=<frame>: compares a game with its resumed copy
	./$< $(EVENTS) $(BEFORE) $(AFTER)

.PHONY: resume-check

$(TMP_DIR)/replay_coverage: $(call coverage_objs,$(REPLAY_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(COVERAGE_CFLAGS) -o $@ $(filter %.o,$^)

COVERAGE_EVENTS ?= $(E2E_EVENTS)

coverage: $(TMP_DIR)/replay_coverage tools/coverage.sh $(COVERAGE_EVENTS) ## [COVERAGE_EVENTS=<files>]: line coverage of src/game/ and src/engine/ in tmp/coverage/lines.txt
	@sh tools/coverage.sh ./$(TMP_DIR)/replay_coverage $(OBJ_COVERAGE_DIR) $(TMP_DIR)/coverage "$(REPLAY_SRCS)" $(COVERAGE_EVENTS)

.PHONY: coverage

COVERAGE_BASELINE_COMMIT ?= 6346485
COVERAGE_BASELINE_DIR := $(TMP_DIR)/coverage-baseline-$(COVERAGE_BASELINE_COMMIT)
COVERAGE_COMPARE_DIR := $(TMP_DIR)/coverage-compare

$(COVERAGE_BASELINE_DIR)/lines.txt:
	rm -rf $(COVERAGE_BASELINE_DIR)/tree
	git worktree prune
	git worktree add --detach $(COVERAGE_BASELINE_DIR)/tree $(COVERAGE_BASELINE_COMMIT)
	env -u MAKEFLAGS -u MFLAGS -u MAKELEVEL $(MAKE) -C $(COVERAGE_BASELINE_DIR)/tree coverage
	cp $(COVERAGE_BASELINE_DIR)/tree/$(TMP_DIR)/coverage/lines.txt $@
	git worktree remove --force $(COVERAGE_BASELINE_DIR)/tree

coverage-compare: coverage $(COVERAGE_BASELINE_DIR)/lines.txt tools/coverage_compare.sh ## [COVERAGE_BASELINE_COMMIT=<commit>]: fails on a line hit at the baseline commit and unhit now
	@mkdir -p $(COVERAGE_COMPARE_DIR)
	@git diff --no-ext-diff --no-color --no-relative -U0 -M --histogram --src-prefix=a/ --dst-prefix=b/ $(COVERAGE_BASELINE_COMMIT) -- src/game src/engine > $(COVERAGE_COMPARE_DIR)/diff.txt
	@sh tools/coverage_compare.sh $(COVERAGE_BASELINE_DIR)/lines.txt $(TMP_DIR)/coverage/lines.txt $(COVERAGE_COMPARE_DIR)/diff.txt > $(COVERAGE_COMPARE_DIR)/report.txt; status=$$?; grep -v '^hit ' $(COVERAGE_COMPARE_DIR)/report.txt; echo "coverage-compare: report in $(COVERAGE_COMPARE_DIR)/report.txt"; exit $$status

.PHONY: coverage-compare

STATE_DIR := $(TMP_DIR)/state
STATE_EVENTS := $(FULL_GAME_EVENTS) $(E2E_EVENTS)
STATE_RUNS := $(addprefix state-run-,$(STATE_EVENTS))
state_file = $(STATE_DIR)/$(basename $(notdir $(1))).txt

# A stopped replay does not stop the others: tools/state_peaks.sh judges each
# from its state file, removed first so that a stale one never counts.
$(STATE_RUNS): state-run-%: % $(TMP_DIR)/replay
	@mkdir -p $(STATE_DIR)
	@rm -f $(call state_file,$<)
	@STATE=$(call state_file,$<) ./$(TMP_DIR)/replay $< /dev/null $(basename $(call state_file,$<)).dump.txt > $(basename $(call state_file,$<)).log 2>&1 || true

.PHONY: $(STATE_RUNS)

ifneq ($(strip $(EVENTS)),)
state: $(TMP_DIR)/replay ## [EVENTS=<events>]: last-frame state in tmp/state.txt; without EVENTS, zone and cap peaks over every replay
	$(if $(ATTRACT_STATE),ATTRACT_STATE=$(ATTRACT_STATE) )$(if $(REPLAY_STARTS),REPLAY_STARTS=$(REPLAY_STARTS) )$(if $(SAVE_HEX),SAVE_HEX=$(SAVE_HEX) )$(if $(DIGEST),DIGEST=$(DIGEST) )STATE=$(TMP_DIR)/state.txt ./$(TMP_DIR)/replay $(EVENTS)
else
state: $(STATE_RUNS) tools/state_peaks.sh
	@sh tools/state_peaks.sh $(TMP_DIR)/state.txt $(foreach e,$(STATE_EVENTS),$(e) $(call state_file,$(e)))
	@echo "state: $(words $(STATE_EVENTS)) replays, peaks in $(TMP_DIR)/state.txt"
endif

END_STATE_DIR := $(TMP_DIR)/end-state
end_state_triples = $(foreach e,$(1),$(e) $(call state_file,$(e)) $(e).end-state)

ifneq ($(strip $(EVENTS)),)
end-state: $(TMP_DIR)/replay tools/end_state.sh ## [EVENTS=<events> [REFERENCE=<file>]]: compares last-frame states with the committed .end-state files
	@mkdir -p $(END_STATE_DIR)
	@rm -f $(END_STATE_DIR)/state.txt
	@STATE=$(END_STATE_DIR)/state.txt ./$(TMP_DIR)/replay $(EVENTS) /dev/null $(END_STATE_DIR)/dump.txt > $(END_STATE_DIR)/log.txt 2>&1 || true
	@sh tools/end_state.sh check $(EVENTS) $(END_STATE_DIR)/state.txt $(or $(REFERENCE),$(EVENTS).end-state)
else
end-state: $(STATE_RUNS) tools/end_state.sh
	@sh tools/end_state.sh check $(call end_state_triples,$(STATE_EVENTS))
endif

end-state-baseline: $(STATE_RUNS) tools/end_state.sh ## writes each missing .end-state, never overwrites one
	@sh tools/end_state.sh baseline $(call end_state_triples,$(STATE_EVENTS))

.PHONY: end-state end-state-baseline

# The core sources, where the comparator reads the P8_FIXED_LIT calls.
constants-check: tools/constants_check.sh tools/constants_compare.sh tools/constants_expected_gaps.txt tools/pico8_reference_probe_out.txt src/p8/fixed.c src/p8/fixed.h $(CORE_SRCS) | $(TMP_DIR) ## compares each P8_FIXED_LIT with its recorded PICO-8 value
	@sh tools/constants_check.sh tools/constants_expected_gaps.txt tools/pico8_reference_probe_out.txt src/p8/fixed.c $(TMP_DIR)/constants_check \
	    $(CORE_SRCS)

.PHONY: constants-check

FLOAT_GUARD := tools/float_guard.txt

# The report is rewritten even when the guard fails; the exit status of the
# script is returned after the move.
float-guard: tools/float_guard.sh | $(TMP_DIR) ## fails if a core file needs floating point
	@status=0; sh tools/float_guard.sh "$(CC)" "$(CFLAGS)" $(TMP_DIR)/float-guard $(CORE_SRCS) > $(TMP_DIR)/float_guard.txt || status=$$?; \
	mv $(TMP_DIR)/float_guard.txt $(FLOAT_GUARD); \
	grep '^float-guard ' $(FLOAT_GUARD); \
	exit $$status

.PHONY: float-guard

ALLOC_GUARD_EXCEPTIONS := tools/alloc_guard_exceptions.txt

alloc-guard: tools/alloc_guard.sh $(ALLOC_GUARD_EXCEPTIONS) $(call objs,$(CORE_SRCS)) ## ratchet on the malloc/free calls of the core (tools/alloc_guard_exceptions.txt)
	@sh tools/alloc_guard.sh $(ALLOC_GUARD_EXCEPTIONS) $(OBJ_DIR) $(call objs,$(CORE_SRCS))

.PHONY: alloc-guard

POINTER_GUARD_EXCEPTIONS := tools/pointer_guard_exceptions.txt
POINTER_GUARD_SRCS := $(wildcard src/game/*.[ch]) src/engine/entity_class.h

pointer-guard: tools/pointer_guard.sh $(POINTER_GUARD_EXCEPTIONS) $(POINTER_GUARD_SRCS) ## ratchet on data pointers in src/game/ (tools/pointer_guard_exceptions.txt)
	@sh tools/pointer_guard.sh $(POINTER_GUARD_EXCEPTIONS) $(POINTER_GUARD_SRCS)

.PHONY: pointer-guard

FLOAT_GREP_EXCLUSIONS := tools/float_grep_exclusions.txt

float-grep: tools/float_grep.sh $(FLOAT_GREP_EXCLUSIONS) ## fails on any floating-point type or call in the core
	@sh tools/float_grep.sh "$(CC)" "$(CFLAGS)" $(FLOAT_GREP_EXCLUSIONS) src $(CORE_SRCS)

.PHONY: float-grep

RANDOM_GUARD_EXCEPTIONS := tools/random_guard_exceptions.txt

random-guard: tools/random_guard.sh $(RANDOM_GUARD_EXCEPTIONS) ## ratchet on the random stream names per file (tools/random_guard_exceptions.txt)
	@sh tools/random_guard.sh $(RANDOM_GUARD_EXCEPTIONS) src

.PHONY: random-guard

button-guard: tools/button_guard.sh ## fails when a src/game/ file other than cheat.c and the bindings names a P8_PAD_BUTTON_*
	@sh tools/button_guard.sh src/game

.PHONY: button-guard

float-link: $(call objs,$(REPLAY_SRCS)) tools/float_link.sh ## links the replay objects without -lm, never run
	@sh tools/float_link.sh "$(CC)" "$(CFLAGS)" $(TMP_DIR)/float-link/replay $(call objs,$(REPLAY_SRCS))

.PHONY: float-link

$(TMP_DIR)/gen_quarter_wave: tools/gen_quarter_wave.c | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $< -lm

sin-table: $(TMP_DIR)/gen_quarter_wave ## regenerates src/p8/quarter_wave.h
	./$(TMP_DIR)/gen_quarter_wave > $(TMP_DIR)/quarter_wave.h
	mv $(TMP_DIR)/quarter_wave.h src/p8/quarter_wave.h

.PHONY: sin-table

$(TMP_DIR)/gen_sprite_sheet: $(call objs,tools/gen_sprite_sheet.c src/p8/assets.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Writes the committed header only when the generator succeeded.
sprite-sheet: $(TMP_DIR)/gen_sprite_sheet ## regenerates src/p8/sprite_sheet_packed.h from pico8/gfx.json
	./$(TMP_DIR)/gen_sprite_sheet pico8/gfx.json > $(TMP_DIR)/sprite_sheet_packed.h
	mv $(TMP_DIR)/sprite_sheet_packed.h src/p8/sprite_sheet_packed.h

.PHONY: sprite-sheet

PSX_SOURCES ?= tools/psx_sources.txt
PSN00BSDK_DIR ?= third_party/psn00bsdk

psn00bsdk-toolchain-setup: psx-net-check ## installs the pinned PSn00bSDK into PSN00BSDK_DIR
	sh tools/install_psn00bsdk_toolchain.sh $(PSN00BSDK_DIR) $(PSX_SOURCES)

psx-hello: ## builds psx-hello/build/psx-hello.exe
	@[ -f $(PSN00BSDK_DIR)/psn00bsdk.stamp ] || { echo "psx-hello: no PSn00bSDK in $(PSN00BSDK_DIR); run make psn00bsdk-toolchain-setup" >&2; exit 1; }
	cmake --fresh -S psx-hello -B psx-hello/build \
		-DCMAKE_TOOLCHAIN_FILE=$(abspath $(PSN00BSDK_DIR))/lib/libpsn00b/cmake/sdk.cmake
	cmake --build psx-hello/build

# The game's PSX build: the core (CORE_SRCS, which holds the memory save
# storage) with its generated default bindings and demo (PSX_CORE_UNITS), the
# shared state and VRAM digest formatters, the PSX platform and the
# recording's generated source.
PSX_EVENTS ?= $(or $(EVENTS),tests/e2e/pad_slipway.json)
PSX_REPLAY_NAME = $(basename $(notdir $(PSX_EVENTS)))
PSX_BUILD_DIR := $(TMP_DIR)/psx/build
PSX_EMBEDDED_REPLAY := $(PSX_BUILD_DIR)/embedded/psx_replay.c
PSX_PLATFORM_SRCS := src/platform/psx_exit.c src/platform/psx_tty.c src/platform/fatal_psx.c src/platform/assert_psx.c \
	src/platform/format_psx.c src/p8/format.c
PSX_CORE_UNITS := $(BINDINGS_CORE_UNIT) $(DEMO_UNIT)
PSX_REPLAY_SRCS := src/cmd/psx_replay.c $(CORE_SRCS) $(PSX_CORE_UNITS) \
	src/devtools/state_format.c src/devtools/format_self_test.c src/devtools/vram_digest.c $(PSX_PLATFORM_SRCS) $(PSX_EMBEDDED_REPLAY)
PSX_GPU_BACKEND_SRCS := src/platform/psx_gpu.c src/platform/psx_tiles.c src/platform/psx_sprite.c src/platform/psx_image.c src/platform/psx_text.c \
	src/platform/psx_clut_cache.c src/platform/psx_pattern.c src/platform/psx_pattern_cache.c src/platform/psx_slot_uploads.c src/platform/psx_gpu_peaks.c \
	src/platform/psx_gpu_textures.c src/platform/psx_colour.c src/platform/psx_vram_layout.c src/p8/palette.c
PSX_GPU_REPLAY_SRCS := src/cmd/psx_gpu_replay.c $(CORE_SRCS) $(PSX_CORE_UNITS) src/devtools/state_format.c src/devtools/format_self_test.c \
	src/devtools/draw_filter.c src/devtools/frame_times.c src/platform/psx_frame_timer.c $(PSX_GPU_BACKEND_SRCS) \
	$(PSX_PLATFORM_SRCS) $(PSX_EMBEDDED_REPLAY)
PSX_GPU_REPLAY_BREAKDOWN_SRCS := $(PSX_GPU_REPLAY_SRCS) src/devtools/draw_timing.c src/platform/psx_fine_clock.c
PSX_LARGEST_SLOTS_SRCS := src/cmd/psx_largest_slots.c src/devtools/largest_slots.c $(CORE_SRCS) $(PSX_CORE_UNITS) \
	src/devtools/state_format.c $(PSX_PLATFORM_SRCS)
PSX_PAD_PROBE_SRCS := src/cmd/psx_pad_probe.c src/platform/psx_pad.c $(PSX_PLATFORM_SRCS)
PSX_GAME_SRCS := src/cmd/psx_game.c $(CORE_SRCS) $(PSX_CORE_UNITS) src/devtools/state_format.c $(PSX_GPU_BACKEND_SRCS) \
	src/platform/psx_pad.c src/platform/psx_pad_map.c $(PSX_PLATFORM_SRCS)
PSX_GAME_CUE ?= $(PSX_BUILD_DIR)/slipways.cue
PSX_GAME_BIN := $(PSX_BUILD_DIR)/slipways.bin
PSX_GAME_DISC_EXE := SLIPWAYS.EXE
PSX_DUMPSXISO = $(PSN00BSDK_DIR)/bin/dumpsxiso
PSX_SIZE = $(PSN00BSDK_DIR)/bin/mipsel-none-elf-size
PSX_NM = $(PSN00BSDK_DIR)/bin/mipsel-none-elf-nm
psx_cmake_list = $(subst $(space),;,$(strip $(abspath $(1))))
space := $(empty) $(empty)

# Always passed: CMake caches options per build directory, and psx-build reconfigures in place.
PSX_BUILD_MEASURED_REPLAY_ASSERTS ?= ON
PSX_BUILD_DRAW_FILTER ?= ON
PSX_BUILD_DRAW_METER ?= ON
PSX_BUILD_GAME_ASSERTS ?= OFF
ifneq ($(words $(PSX_BUILD_GAME_ASSERTS))$(filter-out ON OFF,$(PSX_BUILD_GAME_ASSERTS)),1)
$(error psx-build: PSX_BUILD_GAME_ASSERTS must be ON or OFF, not '$(PSX_BUILD_GAME_ASSERTS)')
endif
# The mode tools/psx_game_check.sh's game-asserts expects on the TTY.
PSX_GAME_ASSERTS_MODE := $(if $(filter ON,$(PSX_BUILD_GAME_ASSERTS)),on,off)
PSX_BUILD_OPT ?= O3
ifneq ($(words $(PSX_BUILD_OPT))$(filter-out O2 O3 Os,$(PSX_BUILD_OPT)),1)
$(error psx-build: PSX_BUILD_OPT must be O2, O3 or Os, not '$(PSX_BUILD_OPT)')
endif
PSX_BUILD_OPT_TARGETS := psx-game psx-gpu-replay psx-gpu-replay-breakdown

psx-build: $(TMP_DIR)/embed_replay $(PSX_CORE_UNITS) tools/psx_opt_level.sh ## [EVENTS=<events>] [PSX_BUILD_*=...]: builds the PSX PS-EXEs in tmp/psx/build
	@[ -f $(PSN00BSDK_DIR)/psn00bsdk.stamp ] || { echo "psx-build: no PSn00bSDK in $(PSN00BSDK_DIR); run make psn00bsdk-toolchain-setup" >&2; exit 1; }
	@mkdir -p $(dir $(PSX_EMBEDDED_REPLAY))
	./$(TMP_DIR)/embed_replay $(PSX_EVENTS) $(PSX_REPLAY_NAME) $(PSX_EMBEDDED_REPLAY)
	cmake -S psx -B $(PSX_BUILD_DIR) \
		-DCMAKE_TOOLCHAIN_FILE=$(abspath $(PSN00BSDK_DIR))/lib/libpsn00b/cmake/sdk.cmake \
		-DCMAKE_BUILD_TYPE=Release -DSLIPWAYS_SRC_DIR=$(abspath src) \
		-DPSX_MEASURED_REPLAY_ASSERTS=$(PSX_BUILD_MEASURED_REPLAY_ASSERTS) \
		-DPSX_GAME_OPT=$(PSX_BUILD_OPT) -DPSX_MEASURED_REPLAY_OPT=$(PSX_BUILD_OPT) \
		-DPSX_GAME_ASSERTS=$(PSX_BUILD_GAME_ASSERTS) \
		-DPSX_GPU_REPLAY_DRAW_FILTER=$(PSX_BUILD_DRAW_FILTER) -DPSX_GPU_REPLAY_DRAW_METER=$(PSX_BUILD_DRAW_METER) \
		'-DPSX_REPLAY_SOURCES=$(call psx_cmake_list,$(PSX_REPLAY_SRCS))' \
		'-DPSX_GPU_REPLAY_SOURCES=$(call psx_cmake_list,$(PSX_GPU_REPLAY_SRCS))' \
		'-DPSX_GPU_REPLAY_BREAKDOWN_SOURCES=$(call psx_cmake_list,$(PSX_GPU_REPLAY_BREAKDOWN_SRCS))' \
		'-DPSX_LARGEST_SLOTS_SOURCES=$(call psx_cmake_list,$(PSX_LARGEST_SLOTS_SRCS))' \
		'-DPSX_PAD_PROBE_SOURCES=$(call psx_cmake_list,$(PSX_PAD_PROBE_SRCS))' \
		'-DPSX_GAME_SOURCES=$(call psx_cmake_list,$(PSX_GAME_SRCS))'
	+cmake --build $(PSX_BUILD_DIR)
	@opt_failed=; levels=; \
	for target in $(PSX_BUILD_OPT_TARGETS); do \
		level=$$(sh tools/psx_opt_level.sh $(PSX_BUILD_DIR)/CMakeFiles/$$target.dir/flags.make) || level=unreadable; \
		[ "$$level" = $(PSX_BUILD_OPT) ] || { echo "psx-build: $$target effective level $$level, not $(PSX_BUILD_OPT) as requested"; opt_failed=yes; }; \
		levels="$$levels $$target $$level"; \
	done; \
	echo "psx-build: opt$$levels"; \
	[ -z "$$opt_failed" ]
	@$(PSX_SIZE) $(PSX_BUILD_DIR)/psx-replay.elf $(PSX_BUILD_DIR)/psx-gpu-replay.elf \
		$(PSX_BUILD_DIR)/psx-gpu-replay-breakdown.elf $(PSX_BUILD_DIR)/psx-pad-probe.elf $(PSX_BUILD_DIR)/psx-game.elf \
		$(PSX_BUILD_DIR)/psx-largest-slots.elf
	@for f in $(PSX_GAME_BIN) $(PSX_GAME_CUE); do \
		[ -f "$$f" ] || { echo "psx-build: CD image $$f missing" >&2; exit 1; }; echo "psx-build: CD image $$f"; done

psx-game-cd: psx-build ## psx-build, then checks the game CD image exists
	@[ -f $(PSX_GAME_BIN) ] && [ -f $(PSX_GAME_CUE) ] || { echo "psx-game-cd: $(PSX_GAME_BIN) or $(PSX_GAME_CUE) missing" >&2; exit 1; }
	@echo "psx-game-cd: $(PSX_GAME_BIN) $(PSX_GAME_CUE)"

PSX_HELLO_CUE ?= psx-hello/build/psx-hello.cue

psx-hello-cd: psx-hello ## builds the psx-hello CD image
	cmake --build psx-hello/build --target psx-hello-iso
	@[ -f psx-hello/build/psx-hello.bin ] && [ -f $(PSX_HELLO_CUE) ] || { echo "psx-hello-cd: psx-hello/build/psx-hello.bin or $(PSX_HELLO_CUE) missing" >&2; exit 1; }

psx-hello-clean:
	rm -rf psx-hello/build

psx-net-check: ## probes every host of PSX_SOURCES without downloading
	sh tools/psx_net_check.sh $(PSX_SOURCES)

PSX_EMULATOR_DIR ?= third_party/pcsx-redux

psx-emulator-setup: psx-net-check ## builds the pinned PCSX-Redux into PSX_EMULATOR_DIR
	PSN00BSDK_DIR=$(PSN00BSDK_DIR) sh tools/install_psx_emulator.sh $(PSX_EMULATOR_DIR) $(PSX_SOURCES)

PSX_FRAMES ?= 600
PSX_OUT ?= $(TMP_DIR)/psx/boot
PSX_BOOT_TIMEOUT ?= 120

psx-boot: ## [PSX_EXE=<exe> | PSX_ISO=<cue>]: boots headless for PSX_FRAMES vsyncs, VRAM in PSX_OUT.vram
	PSX_BOOT_TIMEOUT=$(PSX_BOOT_TIMEOUT) sh tools/psx_boot.sh $(PSX_EMULATOR_DIR) \
		$(if $(PSX_EXE),--exe $(PSX_EXE)) $(if $(PSX_ISO),--iso $(PSX_ISO)) \
		$(PSX_FRAMES) $(PSX_OUT)

PSX_REPLAY_OUT := $(TMP_DIR)/psx/replay
PSX_REPLAY_CEILING ?= 4001
PSX_REPLAY_TIMEOUT ?= 68

psx-replay-check: psx-build $(TMP_DIR)/replay ## boots the replay PS-EXE and compares its state and VRAM with the PC
	@mkdir -p $(PSX_REPLAY_OUT)
	STATE=$(PSX_REPLAY_OUT)/pc_state.txt DIGEST=$(PSX_REPLAY_OUT)/pc_digest.txt ./$(TMP_DIR)/replay $(PSX_EVENTS) /dev/null $(PSX_REPLAY_OUT)/pc.dump.txt > $(PSX_REPLAY_OUT)/pc.log 2>&1
	PSX_BOOT_TIMEOUT=$(PSX_REPLAY_TIMEOUT) sh tools/psx_boot.sh $(PSX_EMULATOR_DIR) --until-exit \
		--exe $(PSX_BUILD_DIR)/psx-replay.exe $(PSX_REPLAY_CEILING) $(PSX_REPLAY_OUT)/run
	@frames=$$(sed -n 's/^replay: \([0-9]*\) frame(s) run.*/\1/p' $(PSX_REPLAY_OUT)/pc.log) \
		&& sh tools/psx_state_compare.sh $(PSX_REPLAY_OUT)/run.tty.txt $(PSX_REPLAY_OUT)/pc_state.txt $(PSX_REPLAY_OUT) $$frames
	sh tools/psx_digest_compare.sh $(PSX_REPLAY_OUT)/run.tty.txt $(PSX_REPLAY_OUT)/pc_digest.txt $(PSX_REPLAY_OUT)
	@$(PSX_SIZE) $(PSX_BUILD_DIR)/psx-replay.elf >> $(PSX_REPLAY_OUT)/measurements.txt

PSX_FULL_GAME_EVENTS := worst-case/full_game.json
PSX_FULL_GAME_DIR := $(TMP_DIR)/psx/full-game
PSX_FULL_GAME_CEILING ?= 71834
PSX_FULL_GAME_TIMEOUT ?= 1197
PSX_FULL_GAME_GPU_CEILING ?= 31500
PSX_FULL_GAME_GPU_TIMEOUT ?= 525
# One NTSC vsync in horizontal blanks: plan 11 step 4 must never raise it.
PSX_FRAME_BUDGET_HBLANKS := 263
PSX_REAL_GAME_ONE_OFF_VSYNCS := 10800
PSX_FULL_GAME_SITUATIONS ?= psx/perf/full-run-situations.txt

PSX_FULL_GAME_ASSERTS ?= off
# Checked at parse time: under -j a checking prerequisite would race the build.
ifneq ($(filter psx-full-game-check,$(MAKECMDGOALS)),)
ifneq ($(words $(PSX_FULL_GAME_ASSERTS))$(filter-out on off,$(PSX_FULL_GAME_ASSERTS)),1)
$(error psx-full-game-check: PSX_FULL_GAME_ASSERTS must be on or off, not '$(PSX_FULL_GAME_ASSERTS)')
endif
endif

# Its own build directory, so that it never overwrites psx-check's PS-EXEs.
psx-full-game-check: psx-tools-test $(TMP_DIR)/replay ## [PSX_FULL_GAME_ASSERTS=on|off]: replays full_game.json on the PSX
	+$(MAKE) psx-build PSX_EVENTS=$(PSX_FULL_GAME_EVENTS) PSX_BUILD_DIR=$(PSX_FULL_GAME_DIR)/build \
		PSX_BUILD_MEASURED_REPLAY_ASSERTS=$(if $(filter off,$(PSX_FULL_GAME_ASSERTS)),OFF,ON) PSX_BUILD_DRAW_FILTER=OFF \
		PSX_BUILD_DRAW_METER=OFF PSX_BUILD_OPT=O3
	@mkdir -p $(PSX_FULL_GAME_DIR)
	STATE=$(PSX_FULL_GAME_DIR)/pc_state.txt DIGEST=$(PSX_FULL_GAME_DIR)/pc_digest.txt ./$(TMP_DIR)/replay $(PSX_FULL_GAME_EVENTS) /dev/null $(PSX_FULL_GAME_DIR)/pc.dump.txt > $(PSX_FULL_GAME_DIR)/pc.log 2>&1 \
		|| { cat $(PSX_FULL_GAME_DIR)/pc.log >&2; exit 1; }
	@frames=$$(sed -n 's/^replay: \([0-9]*\) frame(s) run.*/\1/p' $(PSX_FULL_GAME_DIR)/pc.log) \
		&& sh tools/psx_full_game_check.sh $(PSX_EMULATOR_DIR) $(PSX_FULL_GAME_DIR)/build/psx-replay.exe \
			$(PSX_FULL_GAME_DIR)/build/psx-gpu-replay.exe $(PSX_FULL_GAME_DIR)/pc_state.txt \
			$(PSX_FULL_GAME_DIR)/pc_digest.txt "$$frames" $(PSX_FULL_GAME_CEILING) $(PSX_FULL_GAME_TIMEOUT) \
			$(PSX_FULL_GAME_GPU_CEILING) $(PSX_FULL_GAME_GPU_TIMEOUT) $(PSX_FRAME_BUDGET_HBLANKS) \
			$(PSX_FULL_GAME_SITUATIONS) $(PSX_REAL_GAME_ONE_OFF_VSYNCS) $(PSX_SIZE) \
			$(PSX_STACK_RESERVE) $(PSX_FULL_GAME_DIR)/run

psx-real-game-budget: ## [FRAMES=<table>] [SITUATIONS=<table>]: steady frames over budget per situation
	@out=$$(sh tools/psx_real_game_budget.sh $(or $(FRAMES),psx/perf/full-run-frames.txt) \
		$(or $(SITUATIONS),$(PSX_FULL_GAME_SITUATIONS)) $(PSX_FRAME_BUDGET_HBLANKS) $(PSX_REAL_GAME_ONE_OFF_VSYNCS)) \
		|| exit 1; printf '%s\n' "$$out"; ! printf '%s\n' "$$out" | grep -q '^real-game-budget failed'

.PHONY: psx-real-game-budget

PSX_SHORT_RUNS_MANIFEST := psx/short_runs.txt
PSX_SHORT_RUNS_DIR := $(TMP_DIR)/psx/short-runs
PSX_SHORT_RUNS ?=

SHORT_RUN_CUT_SRCS := src/cmd/short_run_cut.c src/devtools/short_run_cut.c src/devtools/replay_source.c $(BINDINGS_DEFAULT_UNIT) $(ENGINE_SRCS)

$(TMP_DIR)/short_run_cut: $(call objs,$(SHORT_RUN_CUT_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Per run: both recordings must embed and replay; a differing end state is reported, never a failure.
psx-short-runs-cut: $(TMP_DIR)/short_run_cut $(TMP_DIR)/replay $(TMP_DIR)/embed_replay tools/end_state.sh $(PSX_SHORT_RUNS_MANIFEST) $(PSX_FULL_GAME_EVENTS) ## [PSX_SHORT_RUNS=<names>]: cuts the short runs out of full_game.json
	@mkdir -p $(PSX_SHORT_RUNS_DIR)
	./$(TMP_DIR)/short_run_cut $(PSX_FULL_GAME_EVENTS) $(PSX_SHORT_RUNS_MANIFEST) $(PSX_SHORT_RUNS_DIR) $(PSX_SHORT_RUNS)
	@failed=0; \
	for name in $${PSX_SHORT_RUNS:-$$(awk '!/^[[:space:]]*(#|$$)/ {print $$1}' $(PSX_SHORT_RUNS_MANIFEST))}; do \
		dir=$(PSX_SHORT_RUNS_DIR)/$$name; reason=; \
		[ -f $$dir/map.txt ] || reason=" $$dir/map.txt missing;"; \
		for side in run reference; do \
			./$(TMP_DIR)/embed_replay $$dir/$$side.json $$side $$dir/$$side.embedded.c > $$dir/$$side.embed.log 2>&1 \
				|| reason="$$reason $$side.json does not embed;"; \
			rm -f $$dir/$$side.state.txt; \
			STATE=$$dir/$$side.state.txt ./$(TMP_DIR)/replay $$dir/$$side.json /dev/null $$dir/$$side.dump.txt > $$dir/$$side.log 2>&1 \
				&& sh tools/end_state.sh normalize $$dir/$$side.state.txt > $$dir/$$side.end-state \
				|| reason="$$reason $$side.json stopped or its state does not normalize;"; \
		done; \
		if [ -n "$$reason" ]; then echo "short-run-match: $$name failed$$reason"; failed=1; \
		elif cmp -s $$dir/run.end-state $$dir/reference.end-state; then \
			echo "short-run-match: $$name same $$(cat $$dir/map.txt)"; \
		else \
			diff $$dir/reference.end-state $$dir/run.end-state > $$dir/end-state.diff; \
			echo "short-run-match: $$name differs (reported, not gating; - reference, + run) $$(cat $$dir/map.txt)"; \
			grep '^[<>]' $$dir/end-state.diff | sed 's/^</  -/; s/^>/  +/'; \
		fi; \
	done; \
	exit $$failed

short-run-situations: psx-short-runs-cut $(TMP_DIR)/situations tools/short_run_situations.sh $(SITUATIONS_TABLE) ## [PSX_SHORT_RUNS=<names>]: labels the short runs with their situations
	@failed=; \
	for name in $${PSX_SHORT_RUNS:-$$(awk '!/^[[:space:]]*(#|$$)/ {print $$1}' $(PSX_SHORT_RUNS_MANIFEST))}; do \
		dir=$(PSX_SHORT_RUNS_DIR)/$$name; \
		./$(TMP_DIR)/situations $$dir/run.json $$dir/situations.txt > $$dir/situations.log 2>&1 \
			|| { echo "short-run-situation $$name failed: tmp/situations stopped ($$dir/situations.log)"; failed="$$failed $$name"; continue; }; \
		line=$$(sh tools/short_run_situations.sh $$dir/situations.txt $$dir/map.txt $(SITUATIONS_TABLE)) \
			|| { failed="$$failed $$name"; continue; }; \
		echo "$$line"; \
		case "$$line" in "short-run-situation $$name same "*) ;; *) failed="$$failed $$name" ;; esac; \
	done; \
	if [ -n "$$failed" ]; then echo "short-run-situations: failed:$$failed"; exit 1; fi

.PHONY: short-run-situations

PSX_SHORT_RUN_REFERENCE ?= $(PSX_FULL_GAME_DIR)/run/frame-budget/frames.txt
PSX_SHORT_RUN_CEILING ?= 1080
PSX_SHORT_RUN_TIMEOUT ?= 20
PSX_SHORT_RUN_ASSERTS ?= off
PSX_SHORT_RUN_OPT ?= O3
# Checked at parse time: under -j a checking prerequisite would race the cut and the build.
ifneq ($(filter psx-short-runs,$(MAKECMDGOALS)),)
ifneq ($(words $(PSX_SHORT_RUN_ASSERTS))$(filter-out on off,$(PSX_SHORT_RUN_ASSERTS)),1)
$(error psx-short-runs: PSX_SHORT_RUN_ASSERTS must be on or off, not '$(PSX_SHORT_RUN_ASSERTS)')
endif
ifneq ($(words $(PSX_SHORT_RUN_OPT))$(filter-out O2 O3 Os,$(PSX_SHORT_RUN_OPT)),1)
$(error psx-short-runs: PSX_SHORT_RUN_OPT must be O2, O3 or Os, not '$(PSX_SHORT_RUN_OPT)')
endif
endif
# Each run's own embedded recording replaces psx-build's in the three lists that hold it.
psx_short_run_list = $(call psx_cmake_list,$(filter-out $(PSX_EMBEDDED_REPLAY),$(1)))

# A failed boot or analysis fails the run but never stops the loop: every run is booted.
psx-short-runs: psx-tools-test psx-short-runs-cut $(PSX_CORE_UNITS) tools/psx_boot.sh tools/psx_short_run.sh tools/psx_opt_level.sh ## [PSX_SHORT_RUNS=<names>]: boots each short run on the PSX and times it
	@[ -f $(PSN00BSDK_DIR)/psn00bsdk.stamp ] || { echo "psx-short-runs: no PSn00bSDK in $(PSN00BSDK_DIR); run make psn00bsdk-toolchain-setup" >&2; exit 1; }
	@[ -z "$(PSX_SHORT_RUN_REFERENCE)" ] || [ -f "$(PSX_SHORT_RUN_REFERENCE)" ] \
		|| { echo "psx-short-runs: no reference frame table $(PSX_SHORT_RUN_REFERENCE): run make psx-full-game-check" >&2; exit 1; }
	@failed=; \
	for name in $${PSX_SHORT_RUNS:-$$(awk '!/^[[:space:]]*(#|$$)/ {print $$1}' $(PSX_SHORT_RUNS_MANIFEST))}; do \
		dir=$(PSX_SHORT_RUNS_DIR)/$$name; embedded=$(abspath $(PSX_SHORT_RUNS_DIR))/$$name/run.embedded.c; \
		frames=$$(sed -n 's/^replay: \([0-9]*\) frame(s) run.*/\1/p' $$dir/run.log); \
		echo "psx-short-runs: $$name: building $$frames frames in $$dir/build"; \
		if ! { cmake -S psx -B $$dir/build \
			-DCMAKE_TOOLCHAIN_FILE=$(abspath $(PSN00BSDK_DIR))/lib/libpsn00b/cmake/sdk.cmake \
			-DCMAKE_BUILD_TYPE=Release -DSLIPWAYS_SRC_DIR=$(abspath src) \
			-DPSX_MEASURED_REPLAY_ASSERTS=$(if $(filter off,$(PSX_SHORT_RUN_ASSERTS)),OFF,ON) \
			-DPSX_MEASURED_REPLAY_OPT=$(PSX_SHORT_RUN_OPT) -DPSX_GPU_REPLAY_DRAW_FILTER=OFF \
			-DPSX_GPU_REPLAY_DRAW_METER=OFF \
			"-DPSX_REPLAY_SOURCES=$(call psx_short_run_list,$(PSX_REPLAY_SRCS));$$embedded" \
			"-DPSX_GPU_REPLAY_SOURCES=$(call psx_short_run_list,$(PSX_GPU_REPLAY_SRCS));$$embedded" \
			"-DPSX_GPU_REPLAY_BREAKDOWN_SOURCES=$(call psx_short_run_list,$(PSX_GPU_REPLAY_BREAKDOWN_SRCS));$$embedded" \
			'-DPSX_LARGEST_SLOTS_SOURCES=$(call psx_cmake_list,$(PSX_LARGEST_SLOTS_SRCS))' \
			'-DPSX_PAD_PROBE_SOURCES=$(call psx_cmake_list,$(PSX_PAD_PROBE_SRCS))' \
			'-DPSX_GAME_SOURCES=$(call psx_cmake_list,$(PSX_GAME_SRCS))' \
			&& cmake --build $$dir/build --target psx-gpu-replay psx-gpu-replay-breakdown; } > $$dir/build.log 2>&1; then \
			tail -n 20 $$dir/build.log >&2; echo "psx-short-runs: $$name failed to build ($$dir/build.log)"; \
			failed="$$failed $$name"; continue; \
		fi; \
		opt_failed=; sizes=; \
		for target in psx-gpu-replay psx-gpu-replay-breakdown; do \
			level=$$(sh tools/psx_opt_level.sh $$dir/build/CMakeFiles/$$target.dir/flags.make) || level=unreadable; \
			[ "$$level" = $(PSX_SHORT_RUN_OPT) ] || { echo "psx-short-runs: $$name: $$target effective level $$level, not $(PSX_SHORT_RUN_OPT) as requested"; opt_failed=yes; }; \
			sizes="$$sizes $$target $$($(PSX_SIZE) $$dir/build/$$target.elf | awk 'NR == 2 {print $$1 "/" $$2 "/" $$3}')"; \
		done; \
		echo "psx-short-runs: $$name: opt $(PSX_SHORT_RUN_OPT)$$sizes"; \
		if [ -n "$$opt_failed" ]; then failed="$$failed $$name"; continue; fi; \
		: > $$dir/empty_capture_script.txt; : > $$dir/boots.txt; boot_failed=; \
		for boot in plain-1 plain-2 breakdown; do \
			exe=$$dir/build/psx-gpu-replay.exe; [ $$boot = breakdown ] && exe=$$dir/build/psx-gpu-replay-breakdown.exe; \
			rm -f $$dir/$$boot.*; start=$$(date +%s); \
			PSX_BOOT_TIMEOUT=$(PSX_SHORT_RUN_TIMEOUT) sh tools/psx_boot.sh $(PSX_EMULATOR_DIR) --until-exit \
				--capture-script $$dir/empty_capture_script.txt --exe $$exe $(PSX_SHORT_RUN_CEILING) $$dir/$$boot \
				&& boot_exit=ok || boot_exit=failed; \
			done_vsyncs=$$(tr -d '\r' < $$dir/$$boot.tty.txt 2>/dev/null | sed -n 's/^psx-replay: done .* vsyncs \([0-9]*\)$$/\1/p'); \
			echo "$$boot exit $$boot_exit wall $$(($$(date +%s) - start)) s done-vsyncs $${done_vsyncs:--} progress-vsyncs $$(sed -n 's/^vsync //p' $$dir/$$boot.progress 2>/dev/null)" >> $$dir/boots.txt; \
			[ $$boot_exit = ok ] || boot_failed=yes; \
		done; \
		sed "s/^/psx-short-runs: $$name: boot /" $$dir/boots.txt; rm -rf $$dir/analysis; \
		sh tools/psx_short_run.sh $$dir/plain-1.tty.txt $$dir/plain-2.tty.txt $$dir/breakdown.tty.txt $$dir/map.txt "$$frames" \
			$(PSX_FRAME_BUDGET_HBLANKS) "$(PSX_SHORT_RUN_REFERENCE)" $$dir/analysis $(PSX_SHORT_RUN_ASSERTS) | sed "s/^/psx-short-runs: $$name: /"; \
		[ -z "$$boot_failed" ] && [ -f $$dir/analysis/verdicts.txt ] && ! grep -q '^[a-z]* failed ' $$dir/analysis/verdicts.txt || failed="$$failed $$name"; \
	done; \
	if [ -n "$$failed" ]; then echo "psx-short-runs: failed:$$failed"; exit 1; fi; \
	echo "psx-short-runs: passed"

PSX_PROBE_DIR := $(TMP_DIR)/psx/probe
PSX_PROBE_CEILING ?= 1080
PSX_PROBE_TIMEOUT ?= 20
PSX_PROBE_OPT := O3
PATCH ?=
PSX_PROBE_LABEL ?= $(basename $(notdir $(PATCH)))
psx_probe_usage := usage: make psx-probe PATCH=<file> [PSX_SHORT_RUNS=<names>] [PSX_PROBE_LABEL=<label>]
# Checked at parse time, before the cut and the builds start.
ifneq ($(filter psx-probe,$(MAKECMDGOALS)),)
ifeq ($(strip $(PATCH)),)
$(error psx-probe: no PATCH; $(psx_probe_usage))
endif
ifeq ($(wildcard $(PATCH)),)
$(error psx-probe: no patch file $(PATCH); $(psx_probe_usage))
endif
ifneq ($(words $(PSX_PROBE_LABEL))$(findstring /,$(PSX_PROBE_LABEL)),1)
$(error psx-probe: PSX_PROBE_LABEL must be one word without /, not '$(PSX_PROBE_LABEL)')
endif
ifeq ($(PSX_PROBE_LABEL),baseline)
$(error psx-probe: the label baseline is reserved for make psx-probe-baseline)
endif
endif
psx_probe_label = $(if $(filter psx-probe,$@),$(PSX_PROBE_LABEL),baseline)
psx_probe_worktree = $(abspath $(PSX_PROBE_DIR)/$(psx_probe_label)/worktree)
# Sources under src/ come from the worktree; generated units and the run's recording stay the main tree's.
psx_probe_list = $(subst $(space),;,$(strip $(patsubst $(abspath src)/%,$(psx_probe_worktree)/src/%,$(abspath $(filter-out $(PSX_EMBEDDED_REPLAY),$(1))))))

# Every run is built, booted and analyzed even when an earlier one is invalid.
psx-probe-baseline: ## [PSX_SHORT_RUNS=<names>]: probe timings of the short runs on HEAD
psx-probe: ## PATCH=<file> [PSX_SHORT_RUNS=<names>] [PSX_PROBE_LABEL=<label>]: psx-probe-baseline with a patch applied
psx-probe-baseline psx-probe: psx-tools-test psx-short-runs-cut $(PSX_CORE_UNITS) tools/psx_boot.sh tools/psx_probe.sh tools/psx_opt_level.sh
	@[ -f $(PSN00BSDK_DIR)/psn00bsdk.stamp ] || { echo "$@: no PSn00bSDK in $(PSN00BSDK_DIR); run make psn00bsdk-toolchain-setup" >&2; exit 1; }
	@label=$(psx_probe_label); out=$(PSX_PROBE_DIR)/$$label; wt=$(psx_probe_worktree); \
	patch='$(if $(filter psx-probe,$@),$(abspath $(PATCH)))'; \
	fingerprint=$$(git rev-parse HEAD:src HEAD:psx HEAD:$(PSX_FULL_GAME_EVENTS) | tr '\n' ' ')"opt $(PSX_PROBE_OPT)"; \
	remove_worktree() { git worktree remove --force "$$wt" > /dev/null 2>&1; rm -rf "$$wt"; git worktree prune; }; \
	mkdir -p $$out; remove_worktree; trap remove_worktree EXIT; trap 'exit 130' INT TERM; \
	git worktree add --detach "$$wt" HEAD > $$out/worktree.log 2>&1 || { cat $$out/worktree.log >&2; exit 1; }; \
	apply_error=; \
	if [ -n "$$patch" ] && ! git -C "$$wt" apply "$$patch" > $$out/apply.log 2>&1; then \
		apply_error=$$(tr '\n' ' ' < $$out/apply.log); fi; \
	invalid=; \
	for name in $${PSX_SHORT_RUNS:-$$(awk '!/^[[:space:]]*(#|$$)/ {print $$1}' $(PSX_SHORT_RUNS_MANIFEST))}; do \
		dir=$$out/$$name; base=$(PSX_PROBE_DIR)/baseline/$$name; short=$(PSX_SHORT_RUNS_DIR)/$$name; \
		embedded=$(abspath $(PSX_SHORT_RUNS_DIR))/$$name/run.embedded.c; \
		frames=$$(sed -n 's/^replay: \([0-9]*\) frame(s) run.*/\1/p' $$short/run.log); \
		mkdir -p $$dir; rm -rf $$dir/analysis $$dir/boot.*; mkdir -p $$dir/analysis; reason=; \
		[ $$label = baseline ] && rm -f $$dir/fingerprint.txt; \
		if [ -n "$$apply_error" ]; then reason="patch does not apply: $$apply_error"; \
		elif [ $$label != baseline ] && [ ! -f $$base/fingerprint.txt ]; then reason="no baseline for $$name: run make psx-probe-baseline"; \
		elif [ $$label != baseline ] && [ "$$(cat $$base/fingerprint.txt)" != "$$fingerprint" ]; then reason="baseline stale: run make psx-probe-baseline"; \
		fi; \
		if [ -z "$$reason" ]; then \
			echo "$@: $$label: $$name: building $$frames frames in $$dir/build"; \
			{ cmake -S "$$wt/psx" -B $$dir/build \
				-DCMAKE_TOOLCHAIN_FILE=$(abspath $(PSN00BSDK_DIR))/lib/libpsn00b/cmake/sdk.cmake \
				-DCMAKE_BUILD_TYPE=Release "-DSLIPWAYS_SRC_DIR=$$wt/src" \
				-DPSX_PROBE=ON -DPSX_MEASURED_REPLAY_ASSERTS=OFF -DPSX_MEASURED_REPLAY_OPT=$(PSX_PROBE_OPT) \
				-DPSX_GPU_REPLAY_DRAW_FILTER=OFF \
				-DPSX_GPU_REPLAY_DRAW_METER=OFF \
				"-DPSX_REPLAY_SOURCES=$(call psx_probe_list,$(PSX_REPLAY_SRCS));$$embedded" \
				"-DPSX_GPU_REPLAY_SOURCES=$(call psx_probe_list,$(PSX_GPU_REPLAY_SRCS));$$embedded" \
				"-DPSX_GPU_REPLAY_BREAKDOWN_SOURCES=$(call psx_probe_list,$(PSX_GPU_REPLAY_BREAKDOWN_SRCS));$$embedded" \
				'-DPSX_LARGEST_SLOTS_SOURCES=$(call psx_probe_list,$(PSX_LARGEST_SLOTS_SRCS))' \
				'-DPSX_PAD_PROBE_SOURCES=$(call psx_probe_list,$(PSX_PAD_PROBE_SRCS))' \
				'-DPSX_GAME_SOURCES=$(call psx_probe_list,$(PSX_GAME_SRCS))' \
				&& cmake --build $$dir/build --target psx-gpu-replay --parallel $$(nproc); } > $$dir/build.log 2>&1 \
				|| { tail -n 20 $$dir/build.log >&2; reason="build failed ($$dir/build.log)"; }; \
		fi; \
		if [ -z "$$reason" ]; then \
			level=$$(sh tools/psx_opt_level.sh $$dir/build/CMakeFiles/psx-gpu-replay.dir/flags.make) || level=unreadable; \
			[ "$$level" = $(PSX_PROBE_OPT) ] || reason="opt $$level, not $(PSX_PROBE_OPT)"; \
		fi; \
		if [ -z "$$reason" ]; then \
			: > $$dir/empty_capture_script.txt; \
			PSX_BOOT_TIMEOUT=$(PSX_PROBE_TIMEOUT) sh tools/psx_boot.sh $(PSX_EMULATOR_DIR) --until-exit \
				--capture-script $$dir/empty_capture_script.txt --exe $$dir/build/psx-gpu-replay.exe $(PSX_PROBE_CEILING) $$dir/boot \
				|| echo "$@: $$label: $$name: boot failed"; \
			sh tools/psx_probe.sh $$name $$base/boot.tty.txt $$dir/boot.tty.txt $$short/map.txt "$$frames" $$dir/analysis > /dev/null; \
			if ! grep -q "^$$name invalid " $$dir/analysis/probe.txt; then \
				text_base=$$($(PSX_SIZE) $$base/build/psx-gpu-replay.elf | awk 'NR == 2 {print $$1}'); \
				text_probe=$$($(PSX_SIZE) $$dir/build/psx-gpu-replay.elf | awk 'NR == 2 {print $$1}'); \
				echo "$$name text $$text_base -> $$text_probe ($$(printf '%+d' $$((text_probe - text_base))) bytes)" >> $$dir/analysis/probe.txt; \
			fi; \
		else echo "$$name invalid $$reason" > $$dir/analysis/probe.txt; \
		fi; \
		sed "s/^/$@: $$label: /" $$dir/analysis/probe.txt; \
		if grep -q "^$$name invalid " $$dir/analysis/probe.txt; then invalid="$$invalid $$name"; \
		elif [ $$label = baseline ]; then echo "$$fingerprint" > $$dir/fingerprint.txt; \
		fi; \
	done; \
	if [ -n "$$invalid" ]; then echo "$@: $$label: invalid:$$invalid"; exit 1; fi; \
	echo "$@: $$label: valid"

FRAME_PROFILE_TESTS := test_call_profile test_frame_profile test_frame_profile_check test_lever_estimate test_steady_profile

# Only the game, the engine and the PICO-8 runtime are instrumented: the hooks
# must never see themselves or the replay harness.
FRAME_PROFILE_SRCS := src/cmd/frame_profile.c src/devtools/call_profile.c src/devtools/record_start.c \
	src/devtools/draw_filter.c src/devtools/capture_list.c src/devtools/replay_source.c $(STATE_SRCS) \
	$(BINDINGS_DEFAULT_UNIT) $(ENGINE_SRCS)
FRAME_PROFILE_INSTRUMENTED_SRCS := $(filter src/game/% src/engine/% src/p8/%,$(FRAME_PROFILE_SRCS))
FRAME_PROFILE_PLAIN_SRCS := $(filter-out $(FRAME_PROFILE_INSTRUMENTED_SRCS),$(FRAME_PROFILE_SRCS))
FRAME_PROFILE_DIR := $(TMP_DIR)/frame-profile
FRAME_PROFILE_EVENTS := worst-case/full_game.json

$(TMP_DIR)/frame_profile: $(call profile_objs,$(FRAME_PROFILE_INSTRUMENTED_SRCS)) $(call profile_plain_objs,$(FRAME_PROFILE_PLAIN_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)
	nm $@ > $@.syms

# usage: profile <events> <reference> <out-dir> [<draw-root> [<update-root>]]; prints "<label>: same-game ...", fails on a stopped replay or no.
frame_profile_function = profile() { \
	events=$$1; reference=$$2; dir=$$3; mkdir -p $$dir; rm -f $$dir/state.txt; \
	STATE=$$dir/state.txt ./$(TMP_DIR)/frame_profile $$events $$dir $(TMP_DIR)/frame_profile.syms $${4:+"$$4"} $${5:+"$$5"} \
		|| return $$?; \
	if [ ! -f "$$reference" ]; then same=unchecked; \
	elif sh tools/end_state.sh check $$events $$dir/state.txt $$reference > $$dir/end-state.txt; then same=yes; \
	else same=no; cat $$dir/end-state.txt; fi; \
	echo "same-game $$same" >> $$dir/summary.txt; \
	echo "$@: $$dir same-game $$same"; \
	[ $$same != no ]; \
}

frame-profile: $(TMP_DIR)/frame_profile tools/end_state.sh ## [EVENTS=<events>] [REFERENCE=<end-state>]: PC per-function frame profile
	@$(frame_profile_function); events=$(or $(EVENTS),$(FRAME_PROFILE_EVENTS)); \
	profile $$events $(or $(REFERENCE),$$events.end-state) $(FRAME_PROFILE_DIR)/$$(basename $$events .json)

# Each short run gets its own directory: every run's recording is named run.json.
frame-profile-breakdown-check: $(TMP_DIR)/frame_profile tools/end_state.sh tools/frame_profile_check.sh psx-short-runs-cut ## [PSX_SHORT_RUNS=<names>]: profiles each short run against its breakdown
	@$(frame_profile_function); failed=; \
	for name in $${PSX_SHORT_RUNS:-draw-5564}; do \
		short=$(PSX_SHORT_RUNS_DIR)/$$name; dir=$(FRAME_PROFILE_DIR)/short-runs/$$name; \
		if [ ! -f $$short/breakdown.tty.txt ]; then \
			echo "$@: $$name: no $$short/breakdown.tty.txt: run make psx-short-runs PSX_SHORT_RUNS=$$name"; \
			failed="$$failed $$name"; continue; \
		fi; \
		rm -f $$dir/breakdown-check.txt; profile $$short/run.json $$short/run.end-state $$dir \
			&& sh tools/frame_profile_check.sh breakdown $$dir/frames.txt $$short/breakdown.tty.txt $$short/map.txt \
				> $$dir/breakdown-check.txt; status=$$?; \
		[ -f $$dir/breakdown-check.txt ] && sed "s/^/$@: $$name: /" $$dir/breakdown-check.txt; \
		[ $$status -eq 0 ] || failed="$$failed $$name"; \
	done; \
	if [ -n "$$failed" ]; then echo "$@: failed:$$failed"; exit 1; fi

FRAME_PROFILE_THRESHOLD ?= 2
FRAME_PROFILE_REPEATS ?= 4

# The draw windows are start and every draw-* run of the manifest; each recording is profiled
# FRAME_PROFILE_REPEATS times into coverage/<name>/<k>.
frame-profile-coverage: $(TMP_DIR)/frame_profile tools/end_state.sh tools/frame_profile_check.sh psx-short-runs-cut ## [FRAME_PROFILE_THRESHOLD=<pct>] [FRAME_PROFILE_REPEATS=<k>]: profile coverage of the draw runs
	@$(frame_profile_function); \
	case "$(FRAME_PROFILE_REPEATS)" in ''|*[!0-9]*|0) echo "$@: FRAME_PROFILE_REPEATS is not a positive count: '$(FRAME_PROFILE_REPEATS)'"; exit 1;; esac; \
	repeats=$$(seq 1 $(FRAME_PROFILE_REPEATS)); \
	full_name=$$(basename $(FRAME_PROFILE_EVENTS) .json); fulls=; \
	for k in $$repeats; do \
		dir=$(FRAME_PROFILE_DIR)/coverage/$$full_name/$$k; \
		profile $(FRAME_PROFILE_EVENTS) $(FRAME_PROFILE_EVENTS).end-state $$dir || exit 1; \
		fulls="$$fulls,$$dir/frames.txt"; \
	done; \
	windows=; \
	for name in $$(awk '$$1 == "start" || $$1 ~ /^draw-/ {print $$1}' $(PSX_SHORT_RUNS_MANIFEST)); do \
		short=$(PSX_SHORT_RUNS_DIR)/$$name; profiles=; \
		for k in $$repeats; do \
			dir=$(FRAME_PROFILE_DIR)/coverage/$$name/$$k; \
			profile $$short/run.json $$short/run.end-state $$dir || exit 1; \
			profiles="$$profiles,$$dir/frames.txt"; \
		done; \
		windows="$$windows $$name=$${profiles#,}:$$short/map.txt"; \
	done; \
	sh tools/frame_profile_check.sh coverage $(FRAME_PROFILE_THRESHOLD) $${fulls#,} $$windows \
		> $(FRAME_PROFILE_DIR)/coverage.txt; status=$$?; \
	cat $(FRAME_PROFILE_DIR)/coverage.txt; exit $$status

PART ?= draw

# Each run's recording is profiled FRAME_PROFILE_REPEATS times into subtree/<ROOT>/<name>/<k>
# (subtree-update/ with PART=update, where cart_draw stays the draw root).
frame-profile-subtree: $(TMP_DIR)/frame_profile tools/end_state.sh tools/frame_profile_check.sh psx-short-runs-cut ## ROOT=<symbol> [PART=draw|update]: profiles one call subtree over the short runs
	@$(frame_profile_function); \
	case "$(ROOT)" in ''|*[!A-Za-z0-9_]*) echo "$@: ROOT is not a function symbol: '$(ROOT)'"; exit 1;; esac; \
	case "$(FRAME_PROFILE_REPEATS)" in ''|*[!0-9]*|0) echo "$@: FRAME_PROFILE_REPEATS is not a positive count: '$(FRAME_PROFILE_REPEATS)'"; exit 1;; esac; \
	case "$(PART)" in \
		draw) out=$(FRAME_PROFILE_DIR)/subtree/$(ROOT); roots="$(ROOT)";; \
		update) out=$(FRAME_PROFILE_DIR)/subtree-update/$(ROOT); roots="cart_draw $(ROOT)";; \
		*) echo "$@: PART is not draw or update: '$(PART)'"; exit 1;; \
	esac; runs=; \
	for name in $${PSX_SHORT_RUNS:-$$(awk '$$1 == "start" || $$1 ~ /^draw-/ {print $$1}' $(PSX_SHORT_RUNS_MANIFEST))}; do \
		short=$(PSX_SHORT_RUNS_DIR)/$$name; profiles=; \
		for k in $$(seq 1 $(FRAME_PROFILE_REPEATS)); do \
			dir=$$out/$$name/$$k; \
			profile $$short/run.json $$short/run.end-state $$dir $$roots || exit 1; \
			profiles="$$profiles,$$dir/frames.txt"; \
		done; \
		runs="$$runs $$name=$${profiles#,}:$$short/map.txt"; \
	done; \
	sh tools/frame_profile_check.sh subtree part=$(PART) $$runs > $$out/summary.txt; status=$$?; \
	cat $$out/summary.txt; exit $$status

frame-profile-unit-cost: $(TMP_DIR)/frame_profile tools/end_state.sh tools/frame_profile_check.sh psx-short-runs-cut ## [PSX_SHORT_RUNS=<names>]: unit cost per backend call over the short runs
	@$(frame_profile_function); failed=; outputs=; out=$(FRAME_PROFILE_DIR)/unit-cost; \
	for name in $${PSX_SHORT_RUNS:-$$(awk '$$1 !~ /^#/ && NF {print $$1}' $(PSX_SHORT_RUNS_MANIFEST))}; do \
		short=$(PSX_SHORT_RUNS_DIR)/$$name; dir=$$out/$$name; \
		if [ ! -f $$short/breakdown.tty.txt ]; then \
			echo "$@: $$name: no $$short/breakdown.tty.txt: run make psx-short-runs PSX_SHORT_RUN_REFERENCE="; \
			failed="$$failed $$name"; continue; \
		fi; \
		rm -f $$dir/unit-cost.txt; profile $$short/run.json $$short/run.end-state $$dir \
			&& sh tools/frame_profile_check.sh unit-cost $$dir/frames.txt $$short/breakdown.tty.txt $$short/map.txt \
				> $$dir/unit-cost.txt; status=$$?; \
		[ -f $$dir/unit-cost.txt ] && sed "s/^/$@: $$name: /" $$dir/unit-cost.txt; \
		if [ $$status -eq 0 ]; then outputs="$$outputs $$dir/unit-cost.txt"; else failed="$$failed $$name"; fi; \
	done; \
	if [ -n "$$failed" ]; then echo "$@: failed:$$failed"; exit 1; fi; \
	sh tools/frame_profile_check.sh unit-cost-pooled $$outputs > $$out/pooled.txt || exit 1; \
	sed "s/^/$@: pooled: /" $$out/pooled.txt

FRAME_PROFILE_SPIKE_THRESHOLD ?= 263
FRAME_PROFILE_JUDGE_FRAMES := psx/perf/full-run-frames.txt
FRAME_PROFILE_SPIKE_LEVELS := 16

# Each update root (cart_update60, then every open symbol) is profiled FRAME_PROFILE_REPEATS times
# into spikes/<symbol>/<k>; a symbol frame_profile cannot resolve (no text symbol, or several
# addresses) goes to spikes/unresolvable.txt and closes the walk.
frame-profile-spikes: $(TMP_DIR)/frame_profile tools/end_state.sh tools/frame_profile_check.sh $(FRAME_PROFILE_JUDGE_FRAMES) ## profiles the update spikes of full_game.json
	@$(frame_profile_function); \
	case "$(FRAME_PROFILE_REPEATS)" in ''|*[!0-9]*|0) echo "$@: FRAME_PROFILE_REPEATS is not a positive count: '$(FRAME_PROFILE_REPEATS)'"; exit 1;; esac; \
	case "$(FRAME_PROFILE_SPIKE_THRESHOLD)" in ''|*[!0-9]*) echo "$@: FRAME_PROFILE_SPIKE_THRESHOLD is not a count of hblanks: '$(FRAME_PROFILE_SPIKE_THRESHOLD)'"; exit 1;; esac; \
	out=$(FRAME_PROFILE_DIR)/spikes; rm -rf $$out; mkdir -p $$out; : > $$out/unresolvable.txt; \
	profile_set() { \
		profiles=; \
		for k in $$(seq 1 $(FRAME_PROFILE_REPEATS)); do \
			profile $(FRAME_PROFILE_EVENTS) $(FRAME_PROFILE_EVENTS).end-state $$out/$$1/$$k cart_draw $$1 >&2 || return 1; \
			profiles="$$profiles,$$out/$$1/$$k/frames.txt"; \
		done; \
		echo "$${profiles#,}"; \
	}; \
	base=$$(profile_set cart_update60) || exit 1; sets=; level=0; \
	while :; do \
		sh tools/frame_profile_check.sh spikes $(FRAME_PROFILE_SPIKE_THRESHOLD) $(FRAME_PROFILE_JUDGE_FRAMES) $$base $$sets \
			> $$out/spikes.txt; status=$$?; \
		[ $$status -eq 3 ] || break; \
		level=$$((level + 1)); \
		if [ $$level -gt $(FRAME_PROFILE_SPIKE_LEVELS) ]; then echo "$@: still open after $(FRAME_PROFILE_SPIKE_LEVELS) levels"; break; fi; \
		for symbol in $$(awk '$$1 == "open" && NF == 4 {print $$2}' $$out/spikes.txt); do \
			addresses=$$(awk -v name=$$symbol '$$3 == name && $$2 ~ /^[TtWw]$$/ {print $$1}' $(TMP_DIR)/frame_profile.syms | sort -u | wc -l); \
			if [ $$addresses -ne 1 ]; then \
				echo "$$symbol text-symbols $$addresses" >> $$out/unresolvable.txt; sets="$$sets $$symbol=unresolvable"; continue; \
			fi; \
			profiles=$$(profile_set $$symbol) || exit 1; \
			sets="$$sets $$symbol=$$profiles"; \
		done; \
	done; \
	cat $$out/spikes.txt; exit $$status

FRAME_PROFILE_ORDINARY_WINDOWS := draw-5564=5566-5580@27.0,draw-6000=6002-6016@19.6

# Like frame-profile-spikes, with two base roots (cart_update60 and input_step_run) and its own
# ordinary/<symbol>/<k> profiles, so neither target stales the other's.
frame-profile-ordinary: $(TMP_DIR)/frame_profile tools/end_state.sh tools/frame_profile_check.sh $(FRAME_PROFILE_JUDGE_FRAMES) ## profiles the ordinary frames of full_game.json
	@$(frame_profile_function); \
	case "$(FRAME_PROFILE_REPEATS)" in ''|*[!0-9]*|0) echo "$@: FRAME_PROFILE_REPEATS is not a positive count: '$(FRAME_PROFILE_REPEATS)'"; exit 1;; esac; \
	case "$(FRAME_PROFILE_SPIKE_THRESHOLD)" in ''|*[!0-9]*) echo "$@: FRAME_PROFILE_SPIKE_THRESHOLD is not a count of hblanks: '$(FRAME_PROFILE_SPIKE_THRESHOLD)'"; exit 1;; esac; \
	out=$(FRAME_PROFILE_DIR)/ordinary; rm -rf $$out; mkdir -p $$out; : > $$out/unresolvable.txt; \
	profile_set() { \
		profiles=; \
		for k in $$(seq 1 $(FRAME_PROFILE_REPEATS)); do \
			profile $(FRAME_PROFILE_EVENTS) $(FRAME_PROFILE_EVENTS).end-state $$out/$$1/$$k cart_draw $$1 >&2 || return 1; \
			profiles="$$profiles,$$out/$$1/$$k/frames.txt"; \
		done; \
		echo "$${profiles#,}"; \
	}; \
	cart=$$(profile_set cart_update60) || exit 1; input=$$(profile_set input_step_run) || exit 1; sets=; level=0; \
	while :; do \
		sh tools/frame_profile_check.sh ordinary $(FRAME_PROFILE_THRESHOLD) $(FRAME_PROFILE_SPIKE_THRESHOLD) $(FRAME_PROFILE_JUDGE_FRAMES) \
			"$(or $(UPDATE_RATIOS),-)" $(FRAME_PROFILE_ORDINARY_WINDOWS) $$cart $$input $$sets > $$out/ordinary.txt; status=$$?; \
		[ $$status -eq 3 ] || break; \
		level=$$((level + 1)); \
		if [ $$level -gt $(FRAME_PROFILE_SPIKE_LEVELS) ]; then echo "$@: still open after $(FRAME_PROFILE_SPIKE_LEVELS) levels"; break; fi; \
		for symbol in $$(awk '$$1 == "open" && NF == 2 {print $$2}' $$out/ordinary.txt); do \
			addresses=$$(awk -v name=$$symbol '$$3 == name && $$2 ~ /^[TtWw]$$/ {print $$1}' $(TMP_DIR)/frame_profile.syms | sort -u | wc -l); \
			if [ $$addresses -ne 1 ]; then \
				echo "$$symbol text-symbols $$addresses" >> $$out/unresolvable.txt; sets="$$sets $$symbol=unresolvable"; continue; \
			fi; \
			profiles=$$(profile_set $$symbol) || exit 1; \
			sets="$$sets $$symbol=$$profiles"; \
		done; \
	done; \
	cat $$out/ordinary.txt; exit $$status

STEADY_SITUATIONS := map-rest map-move build menu slipway-aim tech-panel score-panel
STEADY_PROFILE_GROUPS := psx/perf/draw-functions.txt
STEADY_PROFILE_DIR := $(FRAME_PROFILE_DIR)/steady

# The pooled overhead is always over every steady run of the manifest; PSX_SHORT_RUNS only narrows
# the runs profiled. A steady situation with no run of the manifest, or marked "no representative
# run", is profiled on the full game's frames over the budget (no-run), only when PSX_SHORT_RUNS is
# unset.
steady-profile: ## [PSX_SHORT_RUNS=<names>]: breakdown overhead per backend call over the steady runs
steady-profile: $(TMP_DIR)/frame_profile tools/end_state.sh tools/steady_profile.sh tools/frame_profile_check.sh \
		psx-short-runs-cut $(STEADY_PROFILE_GROUPS) $(FRAME_PROFILE_JUDGE_FRAMES) $(SITUATIONS_TABLE)
	@$(frame_profile_function); \
	case "$(FRAME_PROFILE_REPEATS)" in ''|*[!0-9]*|0) echo "$@: FRAME_PROFILE_REPEATS is not a positive count: '$(FRAME_PROFILE_REPEATS)'"; exit 1;; esac; \
	out=$(STEADY_PROFILE_DIR); mkdir -p $$out; \
	steady=$$(awk -v situations="$(STEADY_SITUATIONS)" 'BEGIN { n = split(situations, names, " ") } \
		$$1 !~ /^#/ && NF { for (i = 1; i <= n; i++) if (index($$1, names[i] "-") == 1) { print $$1; break } }' $(PSX_SHORT_RUNS_MANIFEST) | tr '\n' ' '); \
	specs=; missing=; \
	for name in $$steady; do \
		short=$(PSX_SHORT_RUNS_DIR)/$$name; \
		if [ ! -f $$short/breakdown.tty.txt ] || [ ! -f $$short/plain-1.tty.txt ]; then missing="$$missing $$name"; continue; fi; \
		specs="$$specs $$short/breakdown.tty.txt:$$short/plain-1.tty.txt:$$short/map.txt"; \
	done; \
	if [ -n "$$missing" ]; then echo "$@: no breakdown or plain-1 boot for:$$missing: run make psx-short-runs PSX_SHORT_RUNS=\"$${missing# }\""; exit 1; fi; \
	sh tools/steady_profile.sh overhead $$specs > $$out/overhead.txt || exit 1; \
	echo "$@: pooled: $$(cat $$out/overhead.txt)"; overhead=$$(awk '{ print $$2 }' $$out/overhead.txt); \
	profile_sets() { \
		draws=; carts=; \
		for k in $$(seq 1 $(FRAME_PROFILE_REPEATS)); do \
			profile $$1 $$2 $$3/draw/$$k draw_layers cart_update60 >&2 || return 1; \
			profile $$1 $$2 $$3/cart/$$k cart_draw input_step_run >&2 || return 1; \
			draws="$$draws,$$3/draw/$$k/frames.txt"; carts="$$carts,$$3/cart/$$k/frames.txt"; \
		done; \
		echo "$${draws#,} $${carts#,}"; \
	}; \
	failed=; \
	for name in $${PSX_SHORT_RUNS:-$$steady}; do \
		short=$(PSX_SHORT_RUNS_DIR)/$$name; dir=$$out/$$name; rm -rf $$dir; mkdir -p $$dir; \
		sets=$$(profile_sets $$short/run.json $$short/run.end-state $$dir) || { failed="$$failed $$name"; continue; }; \
		awk -v name=$$name '$$1 == "#" && $$2 == "no" && $$4 == "run:" && $$8 == name ":" { print "flag no-representative-run " $$5 }' \
			$(PSX_SHORT_RUNS_MANIFEST) > $$dir/summary.txt; \
		sh tools/steady_profile.sh profile $$overhead $(STEADY_PROFILE_GROUPS) $$short/map.txt $$short/plain-1.tty.txt \
			$$short/breakdown.tty.txt $$sets >> $$dir/summary.txt; status=$$?; \
		sed "s/^/$@: $$name: /" $$dir/summary.txt; \
		if [ $$status -ne 0 ] || grep -q '^adds-up failed' $$dir/summary.txt; then failed="$$failed $$name"; fi; \
	done; \
	if [ -z "$${PSX_SHORT_RUNS:-}" ]; then \
		unrepresented=$$(awk '$$1 == "#" && $$2 == "no" && $$3 == "representative" && $$4 == "run:" { print $$5 }' \
			$(PSX_SHORT_RUNS_MANIFEST) | tr '\n' ' '); \
		for situation in $(STEADY_SITUATIONS); do \
			case " $$unrepresented " in *" $$situation "*) ;; *) case " $$steady " in *" $$situation-"*) continue;; esac;; esac; \
			dir=$$out/$$situation; rm -rf $$dir; mkdir -p $$dir; \
			[ -n "$${full_sets:-}" ] || full_sets=$$(profile_sets $(FRAME_PROFILE_EVENTS) $(FRAME_PROFILE_EVENTS).end-state $$out/full-game) \
				|| { failed="$$failed $$situation"; continue; }; sets=$$full_sets; \
			echo "flag no-run" > $$dir/summary.txt; \
			sh tools/steady_profile.sh no-run $$situation $(PSX_FRAME_BUDGET_HBLANKS) $(FRAME_PROFILE_JUDGE_FRAMES) $(SITUATIONS_TABLE) \
				$(STEADY_PROFILE_GROUPS) $$sets >> $$dir/summary.txt || failed="$$failed $$situation"; \
			sed "s/^/$@: $$situation: /" $$dir/summary.txt; \
		done; \
	fi; \
	if [ -n "$$failed" ]; then echo "$@: failed:$$failed"; exit 1; fi

SPREAD_LABEL ?= default

frame-profile-spread: tools/frame_profile_check.sh $(FRAME_PROFILE_JUDGE_FRAMES) ## DRAW_GAINS=<gains> [UPDATE_RATIOS=<ratios>]: spreads the spike frames over the budget
	@spikes=$(FRAME_PROFILE_DIR)/spikes/spikes.txt; \
	[ -f $$spikes ] || { echo "$@: missing $$spikes: run make frame-profile-spikes first"; exit 1; }; \
	[ -n "$(DRAW_GAINS)" ] || { echo "$@: DRAW_GAINS is required: default=<pct>[,<first>-<last>=<pct>]..."; exit 1; }; \
	case "$(SPREAD_LABEL)" in ''|*[!A-Za-z0-9_.-]*) echo "$@: SPREAD_LABEL is not a file-name label: '$(SPREAD_LABEL)'"; exit 1;; esac; \
	out=$(FRAME_PROFILE_DIR)/spikes/spread-$(SPREAD_LABEL).txt; \
	sh tools/frame_profile_check.sh spread 263 $(FRAME_PROFILE_JUDGE_FRAMES) $$spikes "$(DRAW_GAINS)" $(UPDATE_RATIOS) > $$out \
		|| { status=$$?; rm -f $$out; exit $$status; }; \
	cat $$out

LEVER_ESTIMATE_DIR := $(TMP_DIR)/lever-estimate

ifneq ($(filter lever-estimate,$(MAKECMDGOALS)),)
ifeq ($(strip $(LEVERS)),)
$(error lever-estimate: LEVERS missing: make lever-estimate LEVERS=<document holding one lever-table block> (format in docs/psx-game.md))
endif
endif

lever-estimate: tools/lever_estimate.sh tools/psx_real_game_budget.sh ## LEVERS=<document> [FRAMES=<table>] [SITUATIONS=<table>]: estimates a lever table
	@mkdir -p $(LEVER_ESTIMATE_DIR); out=$(LEVER_ESTIMATE_DIR)/estimate.txt; \
	sh tools/lever_estimate.sh $(or $(FRAMES),$(FRAME_PROFILE_JUDGE_FRAMES)) $(or $(SITUATIONS),$(SITUATIONS_TABLE)) \
		$(LEVERS) $(PSX_FRAME_BUDGET_HBLANKS) $(PSX_REAL_GAME_ONE_OFF_VSYNCS) \
		projected=$(LEVER_ESTIMATE_DIR) > $$out; status=$$?; \
	cat $$out; echo "$@: wrote $$out"; exit $$status

$(TMP_DIR)/test_frame_profile: $(call objs,tests/test_frame_profile.c) $(TMP_DIR)/frame_profile $(TMP_DIR)/replay tests/e2e/pad_title.json pico8/demo_record.hex tools/end_state.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_frame_profile_check: $(call objs,tests/test_frame_profile_check.c) tools/frame_profile_check.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_lever_estimate: $(call objs,tests/test_lever_estimate.c) tools/lever_estimate.sh tools/psx_real_game_budget.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_steady_profile: $(call objs,tests/test_steady_profile.c) tools/steady_profile.sh tools/frame_profile_check.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_call_profile: $(call objs,tests/test_call_profile.c src/devtools/call_profile.c src/p8/draw_cost.c src/devtools/draw_timing.c src/p8/image.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

frame-profile-test: $(addprefix $(TMP_DIR)/,$(FRAME_PROFILE_TESTS)) ## host tests of the PC frame profile tool
	@for t in $(FRAME_PROFILE_TESTS); do ./$(TMP_DIR)/$$t || exit 1; done

.PHONY: frame-profile-test frame-profile frame-profile-breakdown-check frame-profile-coverage frame-profile-subtree \
	frame-profile-unit-cost frame-profile-spikes frame-profile-spread frame-profile-ordinary lever-estimate \
	steady-profile

PSX_PAD_SCRIPT ?= psx/pad_probe_script.txt
PSX_PAD_OUT := $(TMP_DIR)/psx/pad
PSX_PAD_CEILING ?= 306
PSX_PAD_TIMEOUT ?= 6

psx-pad-check: psx-build ## boots the pad probe driven by PSX_PAD_SCRIPT
	PSX_BOOT_TIMEOUT=$(PSX_PAD_TIMEOUT) sh tools/psx_boot.sh $(PSX_EMULATOR_DIR) --until-exit \
		--pad-script $(PSX_PAD_SCRIPT) --exe $(PSX_BUILD_DIR)/psx-pad-probe.exe $(PSX_PAD_CEILING) $(PSX_PAD_OUT)/run
	sh tools/psx_pad_compare.sh $(PSX_PAD_SCRIPT) $(PSX_PAD_OUT)/run.tty.txt

PSX_GAME_SCRIPT ?= psx/game_title_script.txt
PSX_GAME_DIFFICULTY ?= reasonable
PSX_GAME_OUT := $(TMP_DIR)/psx/game
PSX_GAME_CEILING ?= 545
PSX_GAME_TIMEOUT ?= 10

# The boot's failure is the game-done condition's, not make's: every condition is still printed.
psx-game-check: psx-tools-test psx-build $(TMP_DIR)/replay $(TMP_DIR)/psx_pad_log ## boots the game PS-EXE driven by PSX_GAME_SCRIPT and compares it with the PC
	@mkdir -p $(PSX_GAME_OUT)
	@boot=$$(PSX_BOOT_TIMEOUT=$(PSX_GAME_TIMEOUT) sh tools/psx_boot.sh $(PSX_EMULATOR_DIR) --until-exit \
		--pad-script $(PSX_GAME_SCRIPT) --exe $(PSX_BUILD_DIR)/psx-game.exe $(PSX_GAME_CEILING) $(PSX_GAME_OUT)/run 2>&1); \
		status=$$?; echo "$$boot"; \
		PSX_GAME_ASSERTS=$(PSX_GAME_ASSERTS_MODE) sh tools/psx_game_check.sh $(PSX_GAME_OUT)/run.tty.txt $$status "$$boot" $(PSX_GAME_SCRIPT) ./$(TMP_DIR)/psx_pad_log \
			./$(TMP_DIR)/replay $(PSX_GAME_DIFFICULTY) $(PSX_GAME_OUT)

PSX_GAME_CD_OUT := $(TMP_DIR)/psx/game-cd
PSX_GAME_CD_CEILING ?= 1140
PSX_GAME_CD_TIMEOUT ?= 20

# The boot's failure is game-done's, not make's: every condition is still printed.
psx-game-cd-check: psx-tools-test psx-game-cd $(TMP_DIR)/replay $(TMP_DIR)/psx_pad_log ## psx-game-check on the game CD image
	@mkdir -p $(PSX_GAME_CD_OUT)
	@boot=$$(PSX_BOOT_TIMEOUT=$(PSX_GAME_CD_TIMEOUT) sh tools/psx_boot.sh $(PSX_EMULATOR_DIR) --until-exit \
		--pad-script $(PSX_GAME_SCRIPT) --iso $(PSX_GAME_CUE) $(PSX_GAME_CD_CEILING) $(PSX_GAME_CD_OUT)/run 2>&1); \
		status=$$?; echo "$$boot"; failed=0; \
		PSX_GAME_ASSERTS=$(PSX_GAME_ASSERTS_MODE) sh tools/psx_game_check.sh $(PSX_GAME_CD_OUT)/run.tty.txt $$status "$$boot" $(PSX_GAME_SCRIPT) ./$(TMP_DIR)/psx_pad_log \
			./$(TMP_DIR)/replay $(PSX_GAME_DIFFICULTY) $(PSX_GAME_CD_OUT) || failed=1; \
		sh tools/psx_cd_check.sh $(PSX_GAME_CUE) $(PSX_BUILD_DIR)/psx-game.exe $(PSX_GAME_DISC_EXE) $(PSX_DUMPSXISO) \
			$(PSX_GAME_CD_OUT)/cd || failed=1; \
		exit $$failed

PSX_CHECK_OUT := $(TMP_DIR)/psx/game-check
PSX_GPU_CEILING ?= 1655
PSX_GPU_TIMEOUT ?= 29
PSX_STACK_RESERVE ?= 65536
PSX_CAPTURE_LIST := psx/gpu_captures.txt
PSX_CAPTURE_CEILINGS := psx/gpu_capture_ceilings.txt
PSX_CAPTURE_REFERENCES := psx/gpu_capture_references.txt
PSX_LARGEST_SLOTS_CEILING ?= 531
PSX_LARGEST_SLOTS_TIMEOUT ?= 10
PSX_MESSAGE_BOX_EVENTS := tests/e2e/pad_message_box.json
PSX_MESSAGE_BOX_DIR := $(PSX_CHECK_OUT)/message-box
PSX_MESSAGE_BOX_EMBEDDED := $(abspath $(PSX_MESSAGE_BOX_DIR))/embedded/psx_replay.c
PSX_MESSAGE_BOX_CAPTURE_LIST := psx/gpu_captures_message_box.txt
PSX_MESSAGE_BOX_CAPTURE_CEILINGS := psx/gpu_capture_ceilings_message_box.txt
PSX_MESSAGE_BOX_CAPTURE_REFERENCES := psx/gpu_capture_references_message_box.txt
PSX_MESSAGE_BOX_GPU_CEILING ?= 333
PSX_MESSAGE_BOX_GPU_TIMEOUT ?= 8

psx-check: ## the PSX check: state, VRAM, GPU captures and costs against the PC
psx-check: psx-tools-test psx-build $(TMP_DIR)/replay $(TMP_DIR)/embed_replay $(TMP_DIR)/format_self_test $(TMP_DIR)/psx_capture_compare \
		$(TMP_DIR)/psx_pad_log
	@rm -rf $(PSX_CHECK_OUT)/pc-captures && mkdir -p $(PSX_CHECK_OUT)/pc-captures
	CAPTURE_LIST=$(PSX_CAPTURE_LIST) CAPTURE_DIR=$(PSX_CHECK_OUT)/pc-captures \
		STATE=$(PSX_CHECK_OUT)/pc_state.txt DIGEST=$(PSX_CHECK_OUT)/pc_digest.txt ./$(TMP_DIR)/replay $(PSX_EVENTS) /dev/null $(PSX_CHECK_OUT)/pc.dump.txt > $(PSX_CHECK_OUT)/pc.log 2>&1 \
		|| { cat $(PSX_CHECK_OUT)/pc.log >&2; exit 1; }
	@rm -rf $(PSX_MESSAGE_BOX_DIR)/pc-captures && mkdir -p $(PSX_MESSAGE_BOX_DIR)/pc-captures $(dir $(PSX_MESSAGE_BOX_EMBEDDED))
	CAPTURE_LIST=$(PSX_MESSAGE_BOX_CAPTURE_LIST) CAPTURE_DIR=$(PSX_MESSAGE_BOX_DIR)/pc-captures \
		./$(TMP_DIR)/replay $(PSX_MESSAGE_BOX_EVENTS) /dev/null $(PSX_MESSAGE_BOX_DIR)/pc.dump.txt > $(PSX_MESSAGE_BOX_DIR)/pc.log 2>&1 \
		|| { cat $(PSX_MESSAGE_BOX_DIR)/pc.log >&2; exit 1; }
	./$(TMP_DIR)/embed_replay $(PSX_MESSAGE_BOX_EVENTS) $(basename $(notdir $(PSX_MESSAGE_BOX_EVENTS))) $(PSX_MESSAGE_BOX_EMBEDDED)
	cmake -S psx -B $(PSX_MESSAGE_BOX_DIR)/build \
		-DCMAKE_TOOLCHAIN_FILE=$(abspath $(PSN00BSDK_DIR))/lib/libpsn00b/cmake/sdk.cmake \
		-DCMAKE_BUILD_TYPE=Release -DSLIPWAYS_SRC_DIR=$(abspath src) \
		-DPSX_MEASURED_REPLAY_ASSERTS=$(PSX_BUILD_MEASURED_REPLAY_ASSERTS) \
		-DPSX_GAME_OPT=$(PSX_BUILD_OPT) -DPSX_MEASURED_REPLAY_OPT=$(PSX_BUILD_OPT) \
		-DPSX_GAME_ASSERTS=$(PSX_BUILD_GAME_ASSERTS) \
		-DPSX_GPU_REPLAY_DRAW_FILTER=$(PSX_BUILD_DRAW_FILTER) -DPSX_GPU_REPLAY_DRAW_METER=$(PSX_BUILD_DRAW_METER) \
		"-DPSX_REPLAY_SOURCES=$(call psx_short_run_list,$(PSX_REPLAY_SRCS));$(PSX_MESSAGE_BOX_EMBEDDED)" \
		"-DPSX_GPU_REPLAY_SOURCES=$(call psx_short_run_list,$(PSX_GPU_REPLAY_SRCS));$(PSX_MESSAGE_BOX_EMBEDDED)" \
		"-DPSX_GPU_REPLAY_BREAKDOWN_SOURCES=$(call psx_short_run_list,$(PSX_GPU_REPLAY_BREAKDOWN_SRCS));$(PSX_MESSAGE_BOX_EMBEDDED)" \
		'-DPSX_LARGEST_SLOTS_SOURCES=$(call psx_cmake_list,$(PSX_LARGEST_SLOTS_SRCS))' \
		'-DPSX_PAD_PROBE_SOURCES=$(call psx_cmake_list,$(PSX_PAD_PROBE_SRCS))' \
		'-DPSX_GAME_SOURCES=$(call psx_cmake_list,$(PSX_GAME_SRCS))' > $(PSX_MESSAGE_BOX_DIR)/build.log 2>&1 \
		&& cmake --build $(PSX_MESSAGE_BOX_DIR)/build --target psx-gpu-replay >> $(PSX_MESSAGE_BOX_DIR)/build.log 2>&1 \
		|| { tail -n 20 $(PSX_MESSAGE_BOX_DIR)/build.log >&2; exit 1; }
	@frames=$$(sed -n 's/^replay: \([0-9]*\) frame(s) run.*/\1/p' $(PSX_CHECK_OUT)/pc.log) \
		&& PSX_GAME_ASSERTS=$(PSX_GAME_ASSERTS_MODE) sh tools/psx_check.sh $(PSX_EMULATOR_DIR) $(PSX_BUILD_DIR)/psx-replay.exe $(PSX_BUILD_DIR)/psx-gpu-replay.exe \
			$(PSX_BUILD_DIR)/psx-pad-probe.exe $(PSX_CHECK_OUT)/pc_state.txt $(PSX_CHECK_OUT)/pc_digest.txt "$$frames" \
			"$$(./$(TMP_DIR)/format_self_test)" $(PSX_PAD_SCRIPT) $(PSX_REPLAY_CEILING) $(PSX_REPLAY_TIMEOUT) \
			$(PSX_GPU_CEILING) $(PSX_GPU_TIMEOUT) $(PSX_PAD_CEILING) $(PSX_PAD_TIMEOUT) $(PSX_SIZE) $(PSX_NM) \
			$(PSX_STACK_RESERVE) $(PSX_CAPTURE_LIST) $(PSX_CHECK_OUT)/pc-captures ./$(TMP_DIR)/psx_capture_compare \
			$(PSX_CAPTURE_CEILINGS) $(PSX_CAPTURE_REFERENCES) $(PSX_BUILD_DIR)/psx-game.exe $(PSX_GAME_SCRIPT) \
			$(PSX_GAME_CD_CEILING) $(PSX_GAME_CD_TIMEOUT) $(PSX_GAME_DIFFICULTY) $(PSX_GAME_CUE) $(PSX_DUMPSXISO) \
			$(PSX_GAME_DISC_EXE) ./$(TMP_DIR)/psx_pad_log ./$(TMP_DIR)/replay $(PSX_BUILD_DIR)/psx-largest-slots.exe \
			$(PSX_LARGEST_SLOTS_CEILING) $(PSX_LARGEST_SLOTS_TIMEOUT) $(PSX_CHECK_OUT)/run \
			message-box $(PSX_MESSAGE_BOX_DIR)/build/psx-gpu-replay.exe $(PSX_MESSAGE_BOX_CAPTURE_LIST) \
			$(PSX_MESSAGE_BOX_DIR)/pc-captures $(PSX_MESSAGE_BOX_CAPTURE_CEILINGS) $(PSX_MESSAGE_BOX_CAPTURE_REFERENCES) \
			$(PSX_MESSAGE_BOX_GPU_CEILING) $(PSX_MESSAGE_BOX_GPU_TIMEOUT)

ifneq ($(filter-out message_box,$(CAPTURES)),)
ifneq ($(filter psx-gpu-capture-ceilings psx-gpu-capture-accept,$(MAKECMDGOALS)),)
$(error CAPTURES must be empty (pad_slipway's list) or message_box, not '$(CAPTURES)')
endif
endif
psx_capture_record_target = $(if $(CAPTURES),$(PSX_MESSAGE_BOX_CAPTURE_$(1)) message-box,$(PSX_CAPTURE_$(1)))

psx-gpu-capture-ceilings: ## LABEL=<label> [CAPTURES=message_box]: records the differ counts of a label
	@[ -n "$(LABEL)" ] || { echo "psx-gpu-capture-ceilings: LABEL missing: make psx-gpu-capture-ceilings LABEL=<label>" >&2; exit 1; }
	sh tools/psx_capture_record.sh ceilings $(PSX_CHECK_OUT)/run $(LABEL) $(call psx_capture_record_target,CEILINGS)

psx-gpu-capture-accept: ## LABEL=<label> [CAPTURES=message_box]: accepts a label whose captures differ 0 from the PC
	@[ -n "$(LABEL)" ] || { echo "psx-gpu-capture-accept: LABEL missing: make psx-gpu-capture-accept LABEL=<label>" >&2; exit 1; }
	sh tools/psx_capture_record.sh references $(PSX_CHECK_OUT)/run $(LABEL) $(call psx_capture_record_target,REFERENCES)

PSX_CAPTURE_REPORT_OUT := $(TMP_DIR)/psx/gpu-capture

psx-gpu-capture-report: psx-build $(TMP_DIR)/replay $(TMP_DIR)/psx_capture_compare ## one PC run and one GPU capture boot, then the comparator (~20 s)
	@rm -rf $(PSX_CAPTURE_REPORT_OUT) && mkdir -p $(PSX_CAPTURE_REPORT_OUT)/pc-captures
	CAPTURE_LIST=$(PSX_CAPTURE_LIST) CAPTURE_DIR=$(PSX_CAPTURE_REPORT_OUT)/pc-captures \
		./$(TMP_DIR)/replay $(PSX_EVENTS) /dev/null $(PSX_CAPTURE_REPORT_OUT)/pc.dump.txt > $(PSX_CAPTURE_REPORT_OUT)/pc.log 2>&1 \
		|| { cat $(PSX_CAPTURE_REPORT_OUT)/pc.log >&2; exit 1; }
	PSX_BOOT_TIMEOUT=$(PSX_GPU_TIMEOUT) sh tools/psx_boot.sh $(PSX_EMULATOR_DIR) --until-exit \
		--capture-script $(PSX_CAPTURE_REPORT_OUT)/pc-captures/capture_script.txt \
		--exe $(PSX_BUILD_DIR)/psx-gpu-replay.exe $(PSX_GPU_CEILING) $(PSX_CAPTURE_REPORT_OUT)/gpu
	sh tools/psx_capture_report.sh ./$(TMP_DIR)/psx_capture_compare $(PSX_CAPTURE_LIST) $(PSX_CAPTURE_REPORT_OUT)/pc-captures \
		$(PSX_CAPTURE_REPORT_OUT)/gpu $(PSX_CAPTURE_REPORT_OUT)/gpu-captures
	@cat $(PSX_CAPTURE_REPORT_OUT)/gpu-captures/report.txt $(PSX_CAPTURE_REPORT_OUT)/gpu-captures/errors.txt

PSX_KNOWN_LINE ?= Auto test all GTE valid opcodes/registers

psx-emulator-check: ## checks the emulator on its test media (~1 min)
	PSX_BOOT_TIMEOUT=$(PSX_BOOT_TIMEOUT) sh tools/psx_emulator_check.sh $(PSX_EMULATOR_DIR) \
		$(PSX_SOURCES) $(TMP_DIR)/psx/media $(TMP_DIR)/psx/check '$(PSX_KNOWN_LINE)' $(PSX_FRAMES)

PSX_HELLO_TTY_LINE ?= psx-hello: main loop started

psx-hello-check: psx-hello ## boots psx-hello and checks its captures (~35 s)
	PSX_BOOT_TIMEOUT=$(PSX_BOOT_TIMEOUT) sh tools/psx_hello_check.sh $(PSX_EMULATOR_DIR) \
		--exe psx-hello/build/psx-hello.exe '$(PSX_HELLO_TTY_LINE)' $(PSX_FRAMES) $(TMP_DIR)/psx/hello-check

psx-hello-cd-check: psx-hello-cd ## psx-hello-check on the psx-hello CD image (~35 s)
	PSX_BOOT_TIMEOUT=$(PSX_BOOT_TIMEOUT) sh tools/psx_hello_check.sh $(PSX_EMULATOR_DIR) \
		--iso $(PSX_HELLO_CUE) '$(PSX_HELLO_TTY_LINE)' $(PSX_FRAMES) $(TMP_DIR)/psx/hello-cd-check

PSX_HELLO_REFERENCE ?= psx-hello/psx-hello-cd.vram.sha256
PSX_HELLO_REFERENCE_FRAMES := 600
PSX_HELLO_CD_CAPTURE := $(TMP_DIR)/psx/hello-cd-check/run1.vram
# A prerequisite, not a recipe line: under -j it fails while the image builds, so the boots never start.
psx-hello-frames-guard:
	@[ "$(PSX_FRAMES)" = "$(PSX_HELLO_REFERENCE_FRAMES)" ] || { echo "psx-hello-e2e: PSX_FRAMES is $(PSX_FRAMES), the reference was taken at $(PSX_HELLO_REFERENCE_FRAMES)" >&2; exit 1; }

psx-hello-e2e: psx-hello-frames-guard psx-hello-cd-check ## psx-hello-cd-check, then compares its capture with PSX_HELLO_REFERENCE
	@sh tools/psx_reference_check.sh $(PSX_HELLO_REFERENCE) $(PSX_HELLO_CD_CAPTURE)

psx-hello-e2e-baseline: psx-hello-frames-guard psx-hello-cd-check ## psx-hello-cd-check, then sets PSX_HELLO_REFERENCE
	@h=$$($(SHA256_CMD) $(PSX_HELLO_CD_CAPTURE)) && echo "$${h%% *}" > $(PSX_HELLO_REFERENCE) \
		&& echo "psx-hello-e2e: reference set in $(PSX_HELLO_REFERENCE) ($${h%% *})"

# Clones a pinned raylib version into third_party/ (gitignored) and builds its
# static library. Slow, network, once, deliberately outside `make test`.
raylib-vendor-setup: ## vendors raylib, see docs/raylib-vendoring.md
	sh tools/install_raylib_vendor.sh

# Link only: this target never runs the binary (no ./$(TMP_DIR)/... line), see
# CLAUDE.md. It needs third_party/raylib/src/libraylib.a, so `make raylib`
# without vendoring fails with "no rule to make target".
# The raylib target builds straight from the sources to the executable:
# its files need `-I third_party/raylib/src`, which the others do not have.
BINDINGS_GAME_UNIT := $(TMP_DIR)/bindings/game.c
BINDINGS_CHOSEN := $(if $(wildcard bindings.cfg),bindings.cfg,bindings.default.cfg)
BINDINGS_PERSONAL_OPTION := $(if $(wildcard bindings.cfg),--personal)

# Rewritten only when the chosen file name changes, so that creating or
# deleting bindings.cfg regenerates the game unit whatever its date.
$(TMP_DIR)/bindings.source: FORCE | $(TMP_DIR)
	@echo '$(BINDINGS_CHOSEN)' | cmp -s - $@ || echo '$(BINDINGS_CHOSEN)' > $@

FORCE:

$(BINDINGS_GAME_UNIT): $(BINDINGS_CHOSEN) $(TMP_DIR)/bindings.source $(TMP_DIR)/bindings_generate
	@mkdir -p $(@D)
	$(TMP_DIR)/bindings_generate $(BINDINGS_PERSONAL_OPTION) $(BINDINGS_CHOSEN) $@

KEYMAP_GAME_UNIT := $(TMP_DIR)/keymap/game.c
KEYMAP_CHOSEN := $(if $(wildcard keymap.cfg),keymap.cfg,keymap.default.cfg)

$(TMP_DIR)/keymap.source: FORCE | $(TMP_DIR)
	@echo '$(KEYMAP_CHOSEN)' | cmp -s - $@ || echo '$(KEYMAP_CHOSEN)' > $@

$(KEYMAP_GAME_UNIT): $(KEYMAP_CHOSEN) $(TMP_DIR)/keymap.source $(TMP_DIR)/keymap_generate
	@mkdir -p $(@D)
	$(TMP_DIR)/keymap_generate $(KEYMAP_CHOSEN) $@

GAME_SRCS := \
    src/cmd/game.c \
    src/platform/raylib_backend.c \
    src/platform/keymap.c \
    src/p8/palette.c \
    src/devtools/replay_control.c \
    src/devtools/replay_source.c \
    src/devtools/session_recorder.c \
    src/platform/save_storage_file.c \
    $(BINDINGS_GAME_UNIT) \
    $(KEYMAP_GAME_UNIT) \
    $(filter-out src/devtools/save_storage_memory.c,$(ENGINE_SRCS))

$(TMP_DIR)/game: $(GAME_SRCS) $(wildcard src/*/*.h) third_party/raylib/src/libraylib.a | $(TMP_DIR)
	$(CC) $(CFLAGS) -I third_party/raylib/src -o $@ $(GAME_SRCS) third_party/raylib/src/libraylib.a $(RAYLIB_LDLIBS)

raylib: $(TMP_DIR)/game ## compiles and links the interactive binary, never runs it (raylib-vendor-setup first)
	@echo "bindings: $(BINDINGS_CHOSEN)$(if $(wildcard bindings.cfg), (personal))"
	@echo "keymap: $(KEYMAP_CHOSEN)$(if $(wildcard keymap.cfg), (personal))"

# The dependency fragments written by the compiler, one per object. The
# wildcard tolerates their absence: on a fresh clone, everything is compiled.
-include $(wildcard $(OBJ_DIR)/src/*/*.d $(OBJ_DIR)/tests/*.d $(OBJ_DIR)/tmp/bindings/*.d $(OBJ_DIR)/tmp/keymap/*.d $(OBJ_DIR)/tmp/demo/*.d)
-include $(wildcard $(OBJ_ASAN_DIR)/src/*/*.d $(OBJ_ASAN_DIR)/tests/*.d $(OBJ_ASAN_DIR)/tmp/bindings/*.d $(OBJ_ASAN_DIR)/tmp/demo/*.d)
-include $(wildcard $(OBJ_UBSAN_DIR)/src/*/*.d $(OBJ_UBSAN_DIR)/tests/*.d)
-include $(wildcard $(OBJ_SANITIZE_DIR)/src/*/*.d $(OBJ_SANITIZE_DIR)/tests/*.d $(OBJ_SANITIZE_DIR)/tmp/bindings/*.d $(OBJ_SANITIZE_DIR)/tmp/demo/*.d)
-include $(wildcard $(OBJ_COVERAGE_DIR)/src/*/*.d $(OBJ_COVERAGE_DIR)/tmp/bindings/*.d $(OBJ_COVERAGE_DIR)/tmp/demo/*.d)
-include $(wildcard $(OBJ_PROFILE_DIR)/src/*/*.d $(OBJ_PROFILE_PLAIN_DIR)/src/*/*.d $(OBJ_PROFILE_PLAIN_DIR)/tmp/bindings/*.d $(OBJ_PROFILE_PLAIN_DIR)/tmp/demo/*.d)

endif
