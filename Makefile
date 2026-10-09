# Everything runs in parallel (`-j` set to the number of cores, output grouped
# per target).
#
# make test          -> unit + constants-check + float-grep + float-guard +
#                       alloc-guard + pointer-guard + data-guard + p8-guard,
#                       the fast loop
# make e2e           -> replays each e2e of tests/e2e/ from the title screen,
#                       with the `-O0` binary of make test, and compares the
#                       sha256 of its capture to <name>.json.sha256; also fails
#                       if an e2e lacks its .txt or its .json.sha256, or runs
#                       past 180 frames (600 if listed in
#                       tools/e2e_long_exceptions.txt), or if an e2e does
#                       not declare "input": "pad" or carries a mouse field
# make test-all      -> test + e2e, the full suite
# make unit          -> the binaries of tests/, built and run
# make replay EVENTS=<events> -> replays that file in the headless engine,
#                       framebuffer dump in tmp/replay_dump.txt, outside the
#                       suite; with SAVE_HEX=<file>, also writes the stored
#                       record at the end as one line of hex (the
#                       initial_save format) and prints its decoded
#                       "save:" summary; fails when there is no record or
#                       it does not decode; with DIGEST=<file>, also writes
#                       the FNV-1a lines of src/devtools/vram_digest.h
#                       ("sprite-sheet", "screen <frame>" every 32nd frame
#                       and the last, "map")
# make state EVENTS=<events> -> make replay with STATE set, which also writes
#                       the state of the last frame (clock, difficulty,
#                       planets, slipways, structures, counts), zone and cap
#                       use to tmp/state.txt, with the draw meter's lines
#                       (zone gpu_frame: GPU bytes a frame against
#                       GPU_FRAME_BUFFER_BYTES; cap gpu_cluts, cap
#                       gpu_patterns: distinct draw palettes and fill
#                       patterns a frame), outside the suite; takes
#                       SAVE_HEX=<file> and DIGEST=<file> as make replay does
# make state         -> replays the worst-case/ games (FULL_GAME_EVENTS) and
#                       every e2e of tests/e2e/, each state in
#                       tmp/state/<name>.txt, then
#                       writes to tmp/state.txt the peak of each zone and cap
#                       over them all, its limit and the replay holding it,
#                       and prints the bytes left under ZONE_BUDGET_BYTES,
#                       then the two GPU primitive buffers' total, outside it;
#                       fails if a replay stops or a peak eats its 1/8
#                       margin (9 x peak > 8 x limit), outside the suite
# make e2e-baseline  -> sets or resets the .json.sha256 of tests/e2e/ from the
#                       current code, never run by make test
# make e2e-ref       -> for each diverging e2e, replays the commit that set its
#                       reference (git blame), leaves that capture in
#                       tmp/frames/<events>.ref.bin and gives the first
#                       diverging frame, outside the suite
# make data-guard    -> none of the patterns of the pre-composition model
#                       (DATA_GUARD_PATTERN) in src/ or tests/, in make test
# make p8-guard      -> none of the PICO-8-only APIs (P8_GUARD_PATTERN: audio,
#                       p8_stat, peek/poke, PICO-8 addresses, string-keyed
#                       tags/messages/states, map.json) in src/ or tests/, in
#                       make test; entity_state_name's table is the accepted
#                       exception (text for make state and frozen hashes)
# make sanitize-net  -> replays the e2e of tests/e2e/ under ASan+UBSan (`-O2`);
#                       passes when every replay exits cleanly; also runs
#                       test_live_slots (slots returned after each replay,
#                       ~17 s), outside the suite
# make coverage      -> builds tmp/replay_coverage (`--coverage -O0`), replays
#                       COVERAGE_EVENTS (default: every e2e of tests/e2e/)
#                       one after the other, writes tmp/coverage/lines.txt
#                       (`<path>:<line> <0|1>` per executable line of
#                       src/game/ and src/engine/, headers included) and
#                       prints the totals, outside the suite
# make full-game     -> replays each worst-case/ game (FULL_GAME_EVENTS:
#                       full_game.json, then the same game followed by a new
#                       game from the title, cut short by pause ->
#                       restart -> new game, and cut short by a pause menu
#                       left open 300 frames; plus title_idle.json, the
#                       title screen left alone 300 frames) to its last
#                       frame with the `-O0`
#                       binary of make test; passes when every replay exits
#                       cleanly (fails on any p8_fatal, a full zone or cap),
#                       outside the suite
# make rng-sync-probe -> builds and runs tests/test_rng_sync (pad_colonise
#                       replayed to a save frame, forked, rnd_state compared
#                       with and without the state a save never holds);
#                       red while saving rnd_state is not enough, outside the
#                       suite
# make save-worst-case -> builds and runs tests/test_save_worst_case: replays
#                       each worst-case/ game (FULL_GAME_EVENTS), and at every
#                       frame of a running game with no wormhole captures,
#                       encodes, decodes and re-encodes the save; fails if a
#                       record exceeds 8 KiB or does not decode back to the
#                       same bytes, prints the largest record with its game
#                       and frame, outside the suite
# make save-fuzz      -> builds and runs tests/test_save_fuzz: replays
#                       worst-case/full_game.json, and at every frame that
#                       writes a save resumes the stored record into a fresh
#                       game after the title and compares it with the
#                       original at that frame (bytes, state dump, screen
#                       with particles hidden when no box is open); fails on
#                       any mismatch, outside the suite
# make resume-check EVENTS=<events> BEFORE=<frame> AFTER=<frame> -> builds
#                       and runs tests/test_resume_check: replays that file
#                       headless and compares the game after frame BEFORE
#                       (the stored record, state dump, screen with particles
#                       hidden) with the game the recording resumed by frame
#                       AFTER; fails on any difference, on a storage write
#                       between BEFORE and the resume, with no game at
#                       either frame or no title between them, outside
#                       the suite
# make constants-check -> compares each P8_FIXED_LIT of the core with its
#                       recorded hex value
#                       (tools/pico8_reference_probe_out.txt); fails unless
#                       the mismatches are exactly
#                       tools/constants_expected_gaps.txt, in make test
# make float-guard   -> rewrites tools/float_guard.txt; fails if a file of the
#                       core needs floating point (it does not compile under
#                       `-mgeneral-regs-only`), in make test
# make alloc-guard   -> counts the malloc/calloc/realloc/free calls of each
#                       object of the core; fails on any difference, higher or
#                       lower, from tools/alloc_guard_exceptions.txt (a
#                       ratchet, updated by hand), in make test
# make pointer-guard -> counts the data pointers and entity_list_t
#                       declarations in the struct fields and file-scope
#                       variables of src/game/ and entity_class.h; fails on
#                       any difference, higher or lower, from
#                       tools/pointer_guard_exceptions.txt (a ratchet,
#                       updated by hand), in make test
# make float-grep    -> searches the core (CORE_SRCS and the headers that `-MM`
#                       finds for them) for any floating-point type or call,
#                       the rest of src/ excluded by
#                       tools/float_grep_exclusions.txt, in make test
# make float-link    -> links the objects of tmp/replay without `-lm` into a
#                       throwaway binary, never run; outside the suite: make
#                       test already builds tmp/replay, with the same link
# make sin-table     -> regenerates src/p8/quarter_wave.h (the sine and cosine
#                       table of the core) from tools/gen_quarter_wave.c,
#                       outside the suite
# make sprite-sheet  -> regenerates src/p8/sprite_sheet_packed.h (the sprite
#                       sheet embedded in the core, 4 bits per pixel) from
#                       pico8/gfx.json with tools/gen_sprite_sheet.c, outside
#                       the suite
# make raylib        -> compiles and LINKS the interactive binary, never runs
#                       it (needs make raylib-vendor-setup first)
# make psn00bsdk-toolchain-setup -> runs psx-net-check, then installs the
#                       pinned PSn00bSDK release of PSX_SOURCES (prebuilt GCC,
#                       elf2x, mkpsxiso, libpsn00b) into PSN00BSDK_DIR
#                       (default third_party/psn00bsdk); apt via sudo unless
#                       PSN00BSDK_SKIP_APT is set; exits 0 at once on a
#                       complete install, fails on a non-empty directory
#                       without its stamp, outside the suite
# make psx-build [EVENTS=<events>] -> builds, in tmp/psx/build, the game's
#                       two PS-EXEs replaying that recording (default
#                       tests/e2e/pad_slipway.json, embedded through
#                       tmp/embed_replay), psx-replay drawing in RAM and
#                       psx-gpu-replay through the GPU backend, and the pad
#                       probe, against
#                       PSN00BSDK_DIR, then prints their ELF sizes; never
#                       boots them; fails pointing at
#                       psn00bsdk-toolchain-setup when the SDK is missing,
#                       outside the suite
# make psx-replay-check -> runs psx-build, writes the PC state and VRAM
#                       digests of PSX_EVENTS with tmp/replay (STATE=,
#                       DIGEST=), boots the replay PS-EXE until it exits
#                       (ceiling PSX_REPLAY_CEILING vsyncs, timeout
#                       PSX_REPLAY_TIMEOUT s, both set for pad_slipway), then
#                       compares its game lines with the PC's through
#                       tools/psx_state_compare.sh and its digest lines through
#                       tools/psx_digest_compare.sh (outputs and
#                       measurements.txt, with the ELF size, in
#                       tmp/psx/replay, ~45 s); never
#                       runs a setup, outside the suite
# make psx-tools-test -> host tests of the PSX check scripts, outside make
#                       test-all; a prerequisite of psx-check
# make psx-check     -> the plan-10 check: runs psx-build, writes the PC state
#                       and VRAM digests of PSX_EVENTS in one tmp/replay run
#                       (STATE=, DIGEST=, and CAPTURE_LIST=psx/gpu_captures.txt
#                       for the PC frames, family call counts and capture
#                       script), boots the replay PS-EXE, the GPU replay PS-EXE
#                       twice with that capture script (ceiling PSX_GPU_CEILING
#                       vsyncs, timeout PSX_GPU_TIMEOUT s) and the pad probe
#                       until they exit, then prints one line per condition
#                       (magic, asserts-on, format against
#                       tmp/format_self_test, replay-done, state-game-lines, sprite-sheet, screen, map,
#                       gpu-replay-done, gpu-state-game-lines, gpu-cost: its
#                       meter lines equal the PC's, ram-room: 64 KiB kernel +
#                       GPU ELF (PSX_SIZE) + PSX_STACK_RESERVE <= 2 MiB, the
#                       seven gpu-capture-* conditions of
#                       tools/psx_capture_check.sh against
#                       psx/gpu_capture_ceilings.txt and
#                       psx/gpu_capture_references.txt,
#                       no-json through PSX_NM on both replay ELFs,
#                       pad-script) through tools/psx_check.sh; fails if any
#                       fails; measurements.txt (MIPS zone/budget/cap, vsyncs,
#                       size of the ELF, then the GPU replay's vsyncs, meter
#                       lines, RAM room, size and capture reports behind "gpu
#                       ") and conditions.txt in tmp/psx/game-check/run, the
#                       capture reports, diff maps and images in its
#                       gpu-captures/, ~75 s; never runs a setup, outside the
#                       suite
# make psx-gpu-capture-ceilings LABEL=<label> -> writes the differ counts of
#                       that label's captures from the last psx-check run into
#                       psx/gpu_capture_ceilings.txt, other labels untouched;
#                       refused unless every other condition passed
# make psx-gpu-capture-accept LABEL=<label> -> the human's command, never run
#                       by an agent or a skill: after a green psx-check, writes
#                       the sha256 of that label's captures into
#                       psx/gpu_capture_references.txt, other labels untouched
# make psx-gpu-capture-report -> one PC run and one GPU capture boot of
#                       psx/gpu_captures.txt, then the comparator, without the
#                       other conditions (outputs in tmp/psx/gpu-capture,
#                       ~13 s); outside the suite
# make psx-full-game-check -> psx-replay-check on PSX_FULL_GAME_EVENTS
#                       (worst-case/full_game.json), built in and written to
#                       tmp/psx/full-game, ceiling PSX_FULL_GAME_CEILING
#                       vsyncs, timeout PSX_FULL_GAME_TIMEOUT s; not run by
#                       psx-check, outside the suite
# make psx-pad-check -> runs psx-build, boots the pad probe until it exits
#                       while tools/psx_pad_play.lua plays PSX_PAD_SCRIPT
#                       (default psx/pad_probe_script.txt) from the probe's
#                       handshake (ceiling PSX_PAD_CEILING vsyncs, timeout
#                       PSX_PAD_TIMEOUT s), then compares the probe's pad
#                       states with the script's through
#                       tools/psx_pad_compare.sh (outputs in tmp/psx/pad,
#                       ~5 s); never runs a setup, outside the suite
# make psx-hello     -> builds psx-hello/build/psx-hello.exe against
#                       PSN00BSDK_DIR, reconfigured from scratch each time;
#                       fails pointing at psn00bsdk-toolchain-setup when the
#                       SDK is missing, outside the suite
# make psx-hello-cd  -> builds psx-hello, then its CD image
#                       psx-hello/build/psx-hello.bin and PSX_HELLO_CUE
#                       (default psx-hello/build/psx-hello.cue), outside the
#                       suite
# make psx-net-check -> probes every host of PSX_SOURCES (default
#                       tools/psx_sources.txt) without downloading; fails,
#                       naming each unreachable host, outside the suite
# make psx-emulator-setup -> runs psx-net-check, then builds the pinned
#                       PCSX-Redux and its OpenBIOS from PSX_SOURCES into
#                       PSX_EMULATOR_DIR (default third_party/pcsx-redux),
#                       ~10 min; apt via sudo unless PSX_EMULATOR_SKIP_APT is
#                       set; exits 0 at once on a complete install, fails on
#                       a non-empty directory without its stamp, outside the
#                       suite
# make psx-boot [PSX_EXE=<exe> | PSX_ISO=<cue>] -> boots that input (or
#                       nothing) headless in PSX_EMULATOR_DIR for PSX_FRAMES
#                       vsyncs (default 600), writes PSX_OUT.vram (capture:
#                       "psx-vram <w> <h> <depth>" line, then raw bytes),
#                       PSX_OUT.tty.txt and PSX_OUT.log (default PSX_OUT
#                       tmp/psx/boot); fails naming the input after
#                       PSX_BOOT_TIMEOUT s (default 120), outside the suite
# make psx-emulator-check -> fetches the test media of PSX_SOURCES into
#                       tmp/psx/media (sha256-checked), boots the test .exe
#                       twice, the test .cue twice and nothing once in
#                       PSX_EMULATOR_DIR (outputs in tmp/psx/check, ~1 min);
#                       fails, one line per failed condition, unless both
#                       pairs of captures match, PSX_KNOWN_LINE is in the exe
#                       TTY and the .cue capture differs from the empty one;
#                       never runs the setup, outside the suite
# make psx-hello-check -> builds psx-hello, boots it twice and nothing once in
#                       PSX_EMULATOR_DIR (outputs in tmp/psx/hello-check,
#                       ~35 s); fails, one line per failed condition, unless
#                       the .exe has the PS-EXE magic, its captures match,
#                       differ from the empty one and are not one color, and
#                       PSX_HELLO_TTY_LINE is in its TTY; never runs a setup,
#                       outside the suite
# make psx-hello-cd-check -> builds psx-hello-cd, then the same check on
#                       PSX_HELLO_CUE (outputs in tmp/psx/hello-cd-check,
#                       ~35 s), with a cue condition (the .cue names a .bin
#                       of whole 2352-byte sectors) in place of the magic;
#                       never runs a setup, outside the suite
# make psx-hello-e2e -> runs psx-hello-cd-check, then compares the sha256 of
#                       its first capture with PSX_HELLO_REFERENCE (default
#                       psx-hello/psx-hello-cd.vram.sha256); fails on a failed
#                       proof, a mismatch, a missing reference or a PSX_FRAMES
#                       other than 600; never runs a setup, ~35 s, outside
#                       the suite
# make psx-hello-e2e-baseline -> same proof, then sets PSX_HELLO_REFERENCE;
#                       never run by any other target

.PHONY: psx-tools-test psn00bsdk-toolchain-setup psx-build psx-check psx-gpu-capture-ceilings psx-gpu-capture-accept psx-gpu-capture-report psx-replay-check psx-full-game-check psx-pad-check psx-hello psx-hello-cd psx-hello-check psx-hello-cd-check psx-hello-e2e psx-hello-e2e-baseline psx-hello-frames-guard psx-hello-clean psx-net-check psx-emulator-setup psx-boot psx-emulator-check unit replay state test e2e test-all raylib-vendor-setup raylib e2e-baseline e2e-ref

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
# blind on that code, silently. Do not merge the five.
OBJ_DIR := $(TMP_DIR)/obj
OBJ_ASAN_DIR := $(TMP_DIR)/obj-asan
OBJ_UBSAN_DIR := $(TMP_DIR)/obj-ubsan
OBJ_SANITIZE_DIR := $(TMP_DIR)/obj-sanitize
OBJ_COVERAGE_DIR := $(TMP_DIR)/obj-coverage

# `-MMD -MP`: next to each object, the compiler writes the list of headers that
# the translation unit actually opened. The include at the bottom of this file
# reads them back, so no header is ever named by hand in a prerequisite list.
DEPFLAGS := -MMD -MP
ASAN_CFLAGS := -fsanitize=address -fno-omit-frame-pointer
UBSAN_CFLAGS := -fsanitize=undefined -fno-sanitize-recover=undefined
SANITIZE_OPT_CFLAGS := -O2
COVERAGE_CFLAGS := --coverage -O0

# `$(call objs,<sources>)` -> the paths of the matching object files in the
# tree concerned. A target names its sources once; its prerequisites and its
# link line follow from them.
objs = $(patsubst %.c,$(OBJ_DIR)/%.o,$(1))
asan_objs = $(patsubst %.c,$(OBJ_ASAN_DIR)/%.o,$(1))
ubsan_objs = $(patsubst %.c,$(OBJ_UBSAN_DIR)/%.o,$(1))
sanitize_objs = $(patsubst %.c,$(OBJ_SANITIZE_DIR)/%.o,$(1))
coverage_objs = $(patsubst %.c,$(OBJ_COVERAGE_DIR)/%.o,$(1))

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

# The whole engine closure: all of src/ except the entry points of
# src/cmd/, src/p8/palette.c, the two `src/platform/raylib_backend*.c`, `src/platform/keymap.c` and
# the modules of src/devtools/ that only the tools and the interactive game
# link (fixed_test_double, replay_control, replay_source, screencast,
# session_recorder, state). Used by the
# targets that call cart_load(), which pulls in the whole port.
ENGINE_SRCS := \
    src/p8/assets.c \
    src/p8/sprite_sheet.c \
    src/game/background.c \
    src/engine/bootstrap.c \
    src/game/cheat.c \
    src/game/input_step.c \
    src/game/pad_nav.c \
    src/game/pad_panel.c \
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
    src/engine/geom.c \
    src/p8/gfx.c \
    src/p8/gfx_software.c \
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
CORE_SRCS := $(filter-out src/p8/assets.c src/devtools/dump.c src/platform/fatal_pc.c src/devtools/input_json.c,$(ENGINE_SRCS))

# A narrower closure: what data_init() reaches to build a real game state,
# without calling cart_load().
CART_ONLY_SRCS := \
    src/game/cart.c \
    src/game/cheat.c \
    src/game/input_step.c \
    src/game/pad_nav.c \
    src/game/pad_panel.c \
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

$(TMP_DIR)/test_draw_backend: $(call objs,tests/test_draw_backend.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_draw_cost: $(call objs,tests/test_draw_cost.c src/p8/draw_cost.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_draw_meter: $(call objs,tests/test_draw_meter.c src/p8/draw_meter.c src/p8/draw_cost.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_vram_layout: $(call objs,tests/test_psx_vram_layout.c src/platform/psx_vram_layout.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_tiles: $(call objs,tests/test_psx_tiles.c src/platform/psx_tiles.c src/p8/draw_cost.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_draw_filter: $(call objs,tests/test_draw_filter.c src/devtools/draw_filter.c src/p8/draw_cost.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_capture_list: $(call objs,tests/test_capture_list.c src/devtools/capture_list.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_capture_compare: $(call objs,tests/test_capture_compare.c src/devtools/capture_compare.c src/platform/psx_colour.c src/p8/palette.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_psx_colour: $(call objs,tests/test_psx_colour.c src/platform/psx_colour.c src/p8/palette.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_input: $(call objs,tests/test_input.c src/p8/input.c src/devtools/input_json.c) tests/fixtures/demo_events.json tests/fixtures/events_valid_ox.json tests/fixtures/events_mouse_recording_refused.json $(wildcard tests/fixtures/events_pad_*.json) | $(TMP_DIR)
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

$(TMP_DIR)/test_keymap: $(call objs,tests/test_keymap.c src/platform/keymap.c) keymap.cfg | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_replay_source: $(call objs,tests/test_replay_source.c src/devtools/save_storage_memory.c src/devtools/replay_source.c src/p8/input.c src/devtools/input_json.c) tests/fixtures/events_valid.json tests/fixtures/events_pad_missing_buttons.json tests/fixtures/events_mouse_recording_refused.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_recording_continuity: $(call objs,tests/test_recording_continuity.c src/devtools/save_storage_memory.c src/devtools/session_recorder.c src/devtools/replay_source.c src/p8/input.c src/devtools/input_json.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_replay_control: $(call objs,tests/test_replay_control.c src/devtools/replay_control.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_marker_coherence: $(call objs,tests/test_marker_coherence.c src/devtools/save_storage_memory.c src/devtools/session_recorder.c src/devtools/replay_source.c src/p8/input.c src/devtools/input_json.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_screencast: $(call objs,tests/test_screencast.c src/devtools/screencast.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# This test runs the real replay driver, hence $(TMP_DIR)/replay as a
# prerequisite: it replays the same fixture with and without a capture and
# compares the two dumps, so that the capture argument has no effect on the
# rest.
$(TMP_DIR)/test_headless_capture: $(call objs,tests/test_headless_capture.c src/devtools/screencast.c src/devtools/dump.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c src/p8/input.c src/devtools/input_json.c) $(TMP_DIR)/replay tests/fixtures/screencast_capture_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_replay_from_title: $(call objs,tests/test_replay_from_title.c) $(TMP_DIR)/replay tests/fixtures/title_pick_forgiving.json tests/fixtures/title_pick_reasonable.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_replay_usage: $(call objs,tests/test_replay_usage.c) $(TMP_DIR)/replay tests/fixtures/title_pick_forgiving.json | $(TMP_DIR)
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

$(TMP_DIR)/test_e2e_limits: $(call objs,tests/test_e2e_limits.c) tools/e2e_limits.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_state_peaks: $(call objs,tests/test_state_peaks.c) tools/state_peaks.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_guard: $(call objs,tests/test_float_guard.c) tools/float_guard.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_alloc_guard: $(call objs,tests/test_alloc_guard.c) tools/alloc_guard.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pointer_guard: $(call objs,tests/test_pointer_guard.c) tools/pointer_guard.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_grep: $(call objs,tests/test_float_grep.c) tools/float_grep.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_link: $(call objs,tests/test_float_link.c) tools/float_link.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_constants_check: $(call objs,tests/test_constants_check.c) tools/constants_compare.sh tools/constants_check.sh src/p8/fixed.c src/p8/fixed.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_palette: $(call objs,tests/test_palette.c src/p8/palette.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_hud: $(call objs,tests/test_hud.c src/game/hud.c src/p8/gfx.c src/p8/gfx_software.c src/p8/vram.c src/p8/fixed.c src/p8/assets.c) | $(TMP_DIR)
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

$(TMP_DIR)/test_live_slots: $(call asan_objs,tests/test_live_slots.c $(ENGINE_SRCS)) $(TMP_DIR)/replay | $(TMP_DIR)
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

$(TMP_DIR)/test_button: $(call objs,tests/test_button.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_planet: $(call objs,tests/test_planet.c $(ENGINE_SRCS)) | $(TMP_DIR)
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

$(TMP_DIR)/test_rng_sync: $(call objs,tests/test_rng_sync.c src/devtools/save_check.c $(STATE_SRCS) $(ENGINE_SRCS)) tests/e2e/pad_colonise.json pico8/gfx.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_save_worst_case: $(call objs,tests/test_save_worst_case.c $(ENGINE_SRCS)) pico8/gfx.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_save_fuzz: $(call objs,tests/test_save_fuzz.c src/devtools/save_check.c $(STATE_SRCS) $(ENGINE_SRCS)) pico8/gfx.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_resume_check: $(call objs,tests/test_resume_check.c src/devtools/save_check.c $(STATE_SRCS) src/devtools/replay_source.c $(ENGINE_SRCS)) pico8/gfx.json | $(TMP_DIR)
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

$(TMP_DIR)/test_layout_row: $(call objs,tests/test_layout_row.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_economy: $(call objs,tests/test_economy.c tests/heap_allocator.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_main_economy: $(call objs,tests/test_main_economy.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_input_step: $(call objs,tests/test_input_step.c src/devtools/replay_source.c src/devtools/session_recorder.c $(STATE_SRCS) $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pad_nav: $(call objs,tests/test_pad_nav.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_draw_layers_camera: $(call objs,tests/test_draw_layers_camera.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_demo: $(call objs,tests/test_demo.c src/p8/vram.c src/p8/assets.c src/p8/gfx.c src/p8/gfx_software.c src/p8/input.c src/devtools/input_json.c src/devtools/dump.c) tests/fixtures/demo_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

REPLAY_SRCS := src/cmd/replay.c src/devtools/screencast.c src/devtools/vram_digest.c src/devtools/draw_filter.c src/devtools/capture_list.c $(STATE_SRCS) src/devtools/replay_source.c $(ENGINE_SRCS)

$(TMP_DIR)/replay: $(call objs,$(REPLAY_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

PSX_CAPTURE_COMPARE_SRCS := src/cmd/psx_capture_compare.c src/devtools/capture_compare.c src/devtools/capture_list.c \
	src/devtools/screencast.c src/platform/psx_colour.c src/p8/palette.c src/p8/vram.c

$(TMP_DIR)/psx_capture_compare: $(call objs,$(PSX_CAPTURE_COMPARE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

EMBED_REPLAY_SRCS := src/cmd/embed_replay.c src/devtools/replay_source.c src/devtools/save_storage_memory.c src/p8/input.c src/devtools/input_json.c

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

TEST_EMBED_REPLAY_SRCS := tests/test_embed_replay.c src/devtools/replay_source.c src/devtools/save_storage_memory.c src/p8/input.c src/devtools/input_json.c

$(TMP_DIR)/test_embed_replay: $(call objs,$(TEST_EMBED_REPLAY_SRCS) $(call embedded,test_embed_replay_pad)) $(TMP_DIR)/embed_replay tests/fixtures/events_mouse_recording_refused.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_embed_replay_save: $(call objs,$(TEST_EMBED_REPLAY_SRCS) $(call embedded,test_embed_replay_save)) $(TMP_DIR)/embed_replay tests/fixtures/events_mouse_recording_refused.json | $(TMP_DIR)
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

$(TMP_DIR)/test_cart: $(call objs,tests/test_cart.c $(ENGINE_SRCS)) pico8/gfx.json | $(TMP_DIR)
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
$(TMP_DIR)/test_layout_nil_item_draws_nothing: $(call objs,tests/test_layout_nil_item_draws_nothing.c tests/heap_allocator.c src/p8/assets.c src/engine/collect.c src/engine/dispatch_context.c src/p8/fixed.c src/engine/geom.c src/p8/gfx.c src/p8/gfx_software.c src/game/hud.c src/game/layout.c src/engine/pointer_list.c src/game/map_tables.c src/p8/vram.c src/p8/str.c src/engine/vec.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# The longest first: under `-j`, make starts it first.
UNIT_TESTS := \
    test_vram \
    test_asset_loading \
    test_sprite_sheet \
    test_gfx \
    test_draw_backend \
    test_draw_cost \
    test_draw_meter \
    test_psx_vram_layout \
    test_psx_colour \
    test_psx_tiles \
    test_draw_filter \
    test_capture_list \
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
    test_palette \
    test_hud \
    test_frames_tool \
    test_frames_check \
    test_format_self_test \
    test_format \
    test_vram_digest \
    test_e2e_limits \
    test_state_peaks \
    test_float_guard \
    test_alloc_guard \
    test_pointer_guard \
    test_float_grep \
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
    test_save_state \
    test_save_storage_memory \
    test_save_storage_file \
    test_save_check \
    test_save_load \
    test_cart \
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
    test_economy \
    test_main_economy \
    test_input_step \
    test_pad_nav

# One run per test, each its own target: that is what `-j` runs in parallel.
UNIT_RUNS := $(addprefix unit-run-,$(UNIT_TESTS))

UNIT_RUNS_SPECIAL := unit-run-test_zone unit-run-test_allocator unit-run-test_frame_zone unit-run-test_ui_zone unit-run-test_particle_zone unit-run-test_game_zone unit-run-test_exit_queue unit-run-test_toast unit-run-test_message_box unit-run-test_live_slots unit-run-test_trade

unit: $(UNIT_RUNS)

PSX_TOOLS_TESTS := test_psx_state_compare test_psx_pad_compare test_psx_digest_compare test_psx_capture_check test_psx_capture_protocol

psx-tools-test: $(addprefix $(TMP_DIR)/,$(PSX_TOOLS_TESTS))
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

# These two tests read tmp/replay_dump.txt, the default dump of tmp/replay:
# under `-j`, never at the same time.
unit-run-test_frames_tool: | unit-run-test_headless_capture

.PHONY: $(UNIT_RUNS)

# Fails before any compilation rather than replay a default file.
ifneq ($(filter replay,$(MAKECMDGOALS)),)
    ifeq ($(strip $(EVENTS)),)
        $(error EVENTS missing: make replay EVENTS=<events.json>)
    endif
endif

replay: $(TMP_DIR)/replay
	$(if $(SAVE_HEX),SAVE_HEX=$(SAVE_HEX) )$(if $(DIGEST),DIGEST=$(DIGEST) )./$(TMP_DIR)/replay $(EVENTS)

test: unit constants-check float-grep float-guard alloc-guard pointer-guard data-guard p8-guard

test-all: test e2e

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
	./$(TMP_DIR)/replay $< $@ $(@:.bin=.dump.txt) > /dev/null

# The assignment keeps the exit status of sha256, which a pipe into cut would
# lose.
frames_hash = @mkdir -p $(@D); h=$$($(SHA256_CMD) $<) && echo "$${h%% *}" > $@

$(FRAMES_DIR)/%.sha256: $(FRAMES_DIR)/%.bin
	$(frames_hash)

e2e: $(foreach e,$(E2E_EVENTS),$(call frames_current,$(e))) tools/e2e_limits.sh tools/frames_check.sh $(E2E_EXCEPTIONS)
	@status=0; \
	sh tools/e2e_limits.sh $(E2E_EXCEPTIONS) $(E2E_EVENTS) || status=1; \
	sh tools/frames_check.sh $(foreach e,$(E2E_EVENTS),$(e) $(call frames_reference,$(e)) $(call frames_current,$(e))) || status=1; \
	echo "e2e: $(words $(E2E_EVENTS)) e2e in $(E2E_DIR)"; \
	exit $$status

e2e-baseline: $(foreach e,$(E2E_EVENTS),$(call frames_current,$(e)))
	@$(foreach e,$(E2E_EVENTS),cp $(call frames_current,$(e)) $(call frames_reference,$(e)) && echo "e2e: reference set in $(call frames_reference,$(e))" && ) true

e2e-ref: $(foreach e,$(E2E_EVENTS),$(if $(wildcard $(call frames_reference,$(e))),$(call frames_current,$(e)))) $(FRAMES_TOOL) tools/frames_ref.sh
	@sh tools/frames_ref.sh ./$(FRAMES_TOOL) $(FRAMES_DIR) $(TMP_DIR)/frames-ref $(foreach e,$(E2E_EVENTS),$(e) $(call frames_reference,$(e)) $(call frames_current,$(e)) $(call frames_capture,$(e)))

# offsetof, void *self, a field read by name, a spec or defaults table: the
# model the composition migration removed, which a port of the cart brings
# back first.
DATA_GUARD_PATTERN := \boffsetof\b|\bvoid[[:space:]]*\*[[:space:]]*(const[[:space:]]+)?self|\b[a-z0-9_]*field_truthy\(|(->|\.)(set_)?field\b|\b[a-z0-9_]+_take_[a-z0-9_]+\(|\boverrides\b|\bg_[a-z0-9_]+_(spec|defaults)\b|\b[a-z0-9_]+_default_(fixed|table|vec|string|box|list|bool|hf)\b

# grep returns 1 when nothing matches; anything above is an error.
data-guard:
	@status=0; grep -rnE '$(DATA_GUARD_PATTERN)' --include='*.c' --include='*.h' src tests || status=$$?; \
	if [ $$status -eq 0 ]; then echo "data-guard: pre-composition pattern above"; exit 1; fi; \
	[ $$status -eq 1 ] || exit $$status; \
	echo "data-guard: clean"

.PHONY: data-guard

P8_GUARD_PATTERN := \bp8_(stat|sfx|music|play_sfx|peek|poke|memcpy|memory_t)\b|\bP8_ADDR_[A-Z_]+|\b0x(5[fF]80|3101|5[fF]2[dD])\b|\b(entities_by_tag|entities_by_message|draw_layers|entity_set_state|entity_init_instance)\("|\bentity_state_of\b|map\.json

p8-guard:
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

sanitize-net: $(SANITIZE_RUNS) unit-run-test_live_slots
	@echo "sanitize: $(words $(SANITIZE_RUNS)) clean"

.PHONY: sanitize-net $(SANITIZE_RUNS)

FULL_GAME_EVENTS := worst-case/full_game.json worst-case/full_game_new_game.json worst-case/full_game_pause_restart.json worst-case/full_game_pause_idle.json worst-case/title_idle.json
FULL_GAME_DIR := $(TMP_DIR)/full-game

full-game: $(FULL_GAME_EVENTS) $(TMP_DIR)/replay
	@mkdir -p $(FULL_GAME_DIR)
	@$(foreach e,$(FULL_GAME_EVENTS),./$(TMP_DIR)/replay $(e) /dev/null $(FULL_GAME_DIR)/$(notdir $(e)).dump.txt > $(FULL_GAME_DIR)/$(notdir $(e)).log 2>&1 || { cat $(FULL_GAME_DIR)/$(notdir $(e)).log; echo "full-game: $(e) stopped"; exit 1; }; echo "full-game: $(e) clean"; ) true

.PHONY: full-game

rng-sync-probe: $(TMP_DIR)/test_rng_sync
	./$<

.PHONY: rng-sync-probe

save-worst-case: $(TMP_DIR)/test_save_worst_case $(FULL_GAME_EVENTS)
	./$< $(FULL_GAME_EVENTS)

.PHONY: save-worst-case

save-fuzz: $(TMP_DIR)/test_save_fuzz worst-case/full_game.json
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

resume-check: $(TMP_DIR)/test_resume_check $(EVENTS)
	./$< $(EVENTS) $(BEFORE) $(AFTER)

.PHONY: resume-check

$(TMP_DIR)/replay_coverage: $(call coverage_objs,$(REPLAY_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(COVERAGE_CFLAGS) -o $@ $(filter %.o,$^)

COVERAGE_EVENTS ?= $(E2E_EVENTS)

coverage: $(TMP_DIR)/replay_coverage tools/coverage.sh $(COVERAGE_EVENTS)
	@sh tools/coverage.sh ./$(TMP_DIR)/replay_coverage $(OBJ_COVERAGE_DIR) $(TMP_DIR)/coverage "$(REPLAY_SRCS)" $(COVERAGE_EVENTS)

.PHONY: coverage

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
state: $(TMP_DIR)/replay
	$(if $(SAVE_HEX),SAVE_HEX=$(SAVE_HEX) )$(if $(DIGEST),DIGEST=$(DIGEST) )STATE=$(TMP_DIR)/state.txt ./$(TMP_DIR)/replay $(EVENTS)
else
state: $(STATE_RUNS) tools/state_peaks.sh
	@sh tools/state_peaks.sh $(TMP_DIR)/state.txt $(foreach e,$(STATE_EVENTS),$(e) $(call state_file,$(e)))
	@echo "state: $(words $(STATE_EVENTS)) replays, peaks in $(TMP_DIR)/state.txt"
endif

# The core sources, where the comparator reads the P8_FIXED_LIT calls.
constants-check: tools/constants_check.sh tools/constants_compare.sh tools/constants_expected_gaps.txt tools/pico8_reference_probe_out.txt src/p8/fixed.c src/p8/fixed.h $(CORE_SRCS) | $(TMP_DIR)
	@sh tools/constants_check.sh tools/constants_expected_gaps.txt tools/pico8_reference_probe_out.txt src/p8/fixed.c $(TMP_DIR)/constants_check \
	    $(CORE_SRCS)

.PHONY: constants-check

FLOAT_GUARD := tools/float_guard.txt

# The report is rewritten even when the guard fails; the exit status of the
# script is returned after the move.
float-guard: tools/float_guard.sh | $(TMP_DIR)
	@status=0; sh tools/float_guard.sh "$(CC)" "$(CFLAGS)" $(TMP_DIR)/float-guard $(CORE_SRCS) > $(TMP_DIR)/float_guard.txt || status=$$?; \
	mv $(TMP_DIR)/float_guard.txt $(FLOAT_GUARD); \
	grep '^float-guard ' $(FLOAT_GUARD); \
	exit $$status

.PHONY: float-guard

ALLOC_GUARD_EXCEPTIONS := tools/alloc_guard_exceptions.txt

alloc-guard: tools/alloc_guard.sh $(ALLOC_GUARD_EXCEPTIONS) $(call objs,$(CORE_SRCS))
	@sh tools/alloc_guard.sh $(ALLOC_GUARD_EXCEPTIONS) $(OBJ_DIR) $(call objs,$(CORE_SRCS))

.PHONY: alloc-guard

POINTER_GUARD_EXCEPTIONS := tools/pointer_guard_exceptions.txt
POINTER_GUARD_SRCS := $(wildcard src/game/*.[ch]) src/engine/entity_class.h

pointer-guard: tools/pointer_guard.sh $(POINTER_GUARD_EXCEPTIONS) $(POINTER_GUARD_SRCS)
	@sh tools/pointer_guard.sh $(POINTER_GUARD_EXCEPTIONS) $(POINTER_GUARD_SRCS)

.PHONY: pointer-guard

FLOAT_GREP_EXCLUSIONS := tools/float_grep_exclusions.txt

float-grep: tools/float_grep.sh $(FLOAT_GREP_EXCLUSIONS)
	@sh tools/float_grep.sh "$(CC)" "$(CFLAGS)" $(FLOAT_GREP_EXCLUSIONS) src $(CORE_SRCS)

.PHONY: float-grep

float-link: $(call objs,$(REPLAY_SRCS)) tools/float_link.sh
	@sh tools/float_link.sh "$(CC)" "$(CFLAGS)" $(TMP_DIR)/float-link/replay $(call objs,$(REPLAY_SRCS))

.PHONY: float-link

$(TMP_DIR)/gen_quarter_wave: tools/gen_quarter_wave.c | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $< -lm

sin-table: $(TMP_DIR)/gen_quarter_wave
	./$(TMP_DIR)/gen_quarter_wave > $(TMP_DIR)/quarter_wave.h
	mv $(TMP_DIR)/quarter_wave.h src/p8/quarter_wave.h

.PHONY: sin-table

$(TMP_DIR)/gen_sprite_sheet: $(call objs,tools/gen_sprite_sheet.c src/p8/assets.c src/p8/vram.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Writes the committed header only when the generator succeeded.
sprite-sheet: $(TMP_DIR)/gen_sprite_sheet
	./$(TMP_DIR)/gen_sprite_sheet pico8/gfx.json > $(TMP_DIR)/sprite_sheet_packed.h
	mv $(TMP_DIR)/sprite_sheet_packed.h src/p8/sprite_sheet_packed.h

.PHONY: sprite-sheet

PSX_SOURCES ?= tools/psx_sources.txt
PSN00BSDK_DIR ?= third_party/psn00bsdk

psn00bsdk-toolchain-setup: psx-net-check
	sh tools/install_psn00bsdk_toolchain.sh $(PSN00BSDK_DIR) $(PSX_SOURCES)

psx-hello:
	@[ -f $(PSN00BSDK_DIR)/psn00bsdk.stamp ] || { echo "psx-hello: no PSn00bSDK in $(PSN00BSDK_DIR); run make psn00bsdk-toolchain-setup" >&2; exit 1; }
	cmake --fresh -S psx-hello -B psx-hello/build \
		-DCMAKE_TOOLCHAIN_FILE=$(abspath $(PSN00BSDK_DIR))/lib/libpsn00b/cmake/sdk.cmake
	cmake --build psx-hello/build

# The game's PSX build: the core (CORE_SRCS, which holds the memory save
# storage), the shared state and VRAM digest formatters, the PSX platform and
# the recording's generated source.
PSX_EVENTS ?= $(or $(EVENTS),tests/e2e/pad_slipway.json)
PSX_REPLAY_NAME = $(basename $(notdir $(PSX_EVENTS)))
PSX_BUILD_DIR := $(TMP_DIR)/psx/build
PSX_EMBEDDED_REPLAY := $(PSX_BUILD_DIR)/embedded/psx_replay.c
PSX_PLATFORM_SRCS := src/platform/psx_exit.c src/platform/psx_tty.c src/platform/fatal_psx.c src/platform/assert_psx.c \
	src/platform/format_psx.c src/p8/format.c
PSX_REPLAY_SRCS := src/cmd/psx_replay.c $(CORE_SRCS) \
	src/devtools/state_format.c src/devtools/format_self_test.c src/devtools/vram_digest.c $(PSX_PLATFORM_SRCS) $(PSX_EMBEDDED_REPLAY)
PSX_GPU_REPLAY_SRCS := src/cmd/psx_gpu_replay.c $(CORE_SRCS) src/devtools/state_format.c src/devtools/format_self_test.c \
	src/devtools/draw_filter.c src/platform/psx_gpu.c src/platform/psx_tiles.c src/platform/psx_vram_layout.c src/p8/palette.c $(PSX_PLATFORM_SRCS) $(PSX_EMBEDDED_REPLAY)
PSX_PAD_PROBE_SRCS := src/cmd/psx_pad_probe.c src/platform/psx_pad.c $(PSX_PLATFORM_SRCS)
PSX_SIZE = $(PSN00BSDK_DIR)/bin/mipsel-none-elf-size
PSX_NM = $(PSN00BSDK_DIR)/bin/mipsel-none-elf-nm
psx_cmake_list = $(subst $(space),;,$(strip $(abspath $(1))))
space := $(empty) $(empty)

psx-build: $(TMP_DIR)/embed_replay
	@[ -f $(PSN00BSDK_DIR)/psn00bsdk.stamp ] || { echo "psx-build: no PSn00bSDK in $(PSN00BSDK_DIR); run make psn00bsdk-toolchain-setup" >&2; exit 1; }
	@mkdir -p $(dir $(PSX_EMBEDDED_REPLAY))
	./$(TMP_DIR)/embed_replay $(PSX_EVENTS) $(PSX_REPLAY_NAME) $(PSX_EMBEDDED_REPLAY)
	cmake -S psx -B $(PSX_BUILD_DIR) \
		-DCMAKE_TOOLCHAIN_FILE=$(abspath $(PSN00BSDK_DIR))/lib/libpsn00b/cmake/sdk.cmake \
		-DCMAKE_BUILD_TYPE=Release -DSLIPWAYS_SRC_DIR=$(abspath src) \
		'-DPSX_REPLAY_SOURCES=$(call psx_cmake_list,$(PSX_REPLAY_SRCS))' \
		'-DPSX_GPU_REPLAY_SOURCES=$(call psx_cmake_list,$(PSX_GPU_REPLAY_SRCS))' \
		'-DPSX_PAD_PROBE_SOURCES=$(call psx_cmake_list,$(PSX_PAD_PROBE_SRCS))'
	+cmake --build $(PSX_BUILD_DIR)
	@$(PSX_SIZE) $(PSX_BUILD_DIR)/psx-replay.elf $(PSX_BUILD_DIR)/psx-gpu-replay.elf $(PSX_BUILD_DIR)/psx-pad-probe.elf

PSX_HELLO_CUE ?= psx-hello/build/psx-hello.cue

psx-hello-cd: psx-hello
	cmake --build psx-hello/build --target psx-hello-iso
	@[ -f psx-hello/build/psx-hello.bin ] && [ -f $(PSX_HELLO_CUE) ] || { echo "psx-hello-cd: psx-hello/build/psx-hello.bin or $(PSX_HELLO_CUE) missing" >&2; exit 1; }

psx-hello-clean:
	rm -rf psx-hello/build

psx-net-check:
	sh tools/psx_net_check.sh $(PSX_SOURCES)

PSX_EMULATOR_DIR ?= third_party/pcsx-redux

psx-emulator-setup: psx-net-check
	sh tools/install_psx_emulator.sh $(PSX_EMULATOR_DIR) $(PSX_SOURCES)

PSX_FRAMES ?= 600
PSX_OUT ?= $(TMP_DIR)/psx/boot
PSX_BOOT_TIMEOUT ?= 120

psx-boot:
	PSX_BOOT_TIMEOUT=$(PSX_BOOT_TIMEOUT) sh tools/psx_boot.sh $(PSX_EMULATOR_DIR) \
		$(if $(PSX_EXE),--exe $(PSX_EXE)) $(if $(PSX_ISO),--iso $(PSX_ISO)) \
		$(PSX_FRAMES) $(PSX_OUT)

PSX_REPLAY_OUT := $(TMP_DIR)/psx/replay
PSX_REPLAY_CEILING ?= 4001
PSX_REPLAY_TIMEOUT ?= 68

psx-replay-check: psx-build $(TMP_DIR)/replay
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
PSX_FULL_GAME_CEILING ?= 60000
PSX_FULL_GAME_TIMEOUT ?= 1200

# Its own build directory, so that it never overwrites psx-check's PS-EXEs.
psx-full-game-check:
	+$(MAKE) psx-replay-check PSX_EVENTS=$(PSX_FULL_GAME_EVENTS) PSX_BUILD_DIR=$(PSX_FULL_GAME_DIR)/build \
		PSX_REPLAY_OUT=$(PSX_FULL_GAME_DIR) PSX_REPLAY_CEILING=$(PSX_FULL_GAME_CEILING) PSX_REPLAY_TIMEOUT=$(PSX_FULL_GAME_TIMEOUT)

PSX_PAD_SCRIPT ?= psx/pad_probe_script.txt
PSX_PAD_OUT := $(TMP_DIR)/psx/pad
PSX_PAD_CEILING ?= 306
PSX_PAD_TIMEOUT ?= 6

psx-pad-check: psx-build
	PSX_BOOT_TIMEOUT=$(PSX_PAD_TIMEOUT) sh tools/psx_boot.sh $(PSX_EMULATOR_DIR) --until-exit \
		--pad-script $(PSX_PAD_SCRIPT) --exe $(PSX_BUILD_DIR)/psx-pad-probe.exe $(PSX_PAD_CEILING) $(PSX_PAD_OUT)/run
	sh tools/psx_pad_compare.sh $(PSX_PAD_SCRIPT) $(PSX_PAD_OUT)/run.tty.txt

PSX_CHECK_OUT := $(TMP_DIR)/psx/game-check
PSX_GPU_CEILING ?= 1020
PSX_GPU_TIMEOUT ?= 19
PSX_STACK_RESERVE ?= 65536
PSX_CAPTURE_LIST := psx/gpu_captures.txt
PSX_CAPTURE_CEILINGS := psx/gpu_capture_ceilings.txt
PSX_CAPTURE_REFERENCES := psx/gpu_capture_references.txt

psx-check: psx-tools-test psx-build $(TMP_DIR)/replay $(TMP_DIR)/format_self_test $(TMP_DIR)/psx_capture_compare
	@rm -rf $(PSX_CHECK_OUT)/pc-captures && mkdir -p $(PSX_CHECK_OUT)/pc-captures
	CAPTURE_LIST=$(PSX_CAPTURE_LIST) CAPTURE_DIR=$(PSX_CHECK_OUT)/pc-captures \
		STATE=$(PSX_CHECK_OUT)/pc_state.txt DIGEST=$(PSX_CHECK_OUT)/pc_digest.txt ./$(TMP_DIR)/replay $(PSX_EVENTS) /dev/null $(PSX_CHECK_OUT)/pc.dump.txt > $(PSX_CHECK_OUT)/pc.log 2>&1 \
		|| { cat $(PSX_CHECK_OUT)/pc.log >&2; exit 1; }
	@frames=$$(sed -n 's/^replay: \([0-9]*\) frame(s) run.*/\1/p' $(PSX_CHECK_OUT)/pc.log) \
		&& sh tools/psx_check.sh $(PSX_EMULATOR_DIR) $(PSX_BUILD_DIR)/psx-replay.exe $(PSX_BUILD_DIR)/psx-gpu-replay.exe \
			$(PSX_BUILD_DIR)/psx-pad-probe.exe $(PSX_CHECK_OUT)/pc_state.txt $(PSX_CHECK_OUT)/pc_digest.txt "$$frames" \
			"$$(./$(TMP_DIR)/format_self_test)" $(PSX_PAD_SCRIPT) $(PSX_REPLAY_CEILING) $(PSX_REPLAY_TIMEOUT) \
			$(PSX_GPU_CEILING) $(PSX_GPU_TIMEOUT) $(PSX_PAD_CEILING) $(PSX_PAD_TIMEOUT) $(PSX_SIZE) $(PSX_NM) \
			$(PSX_STACK_RESERVE) $(PSX_CAPTURE_LIST) $(PSX_CHECK_OUT)/pc-captures ./$(TMP_DIR)/psx_capture_compare \
			$(PSX_CAPTURE_CEILINGS) $(PSX_CAPTURE_REFERENCES) $(PSX_CHECK_OUT)/run

psx-gpu-capture-ceilings:
	@[ -n "$(LABEL)" ] || { echo "psx-gpu-capture-ceilings: LABEL missing: make psx-gpu-capture-ceilings LABEL=<label>" >&2; exit 1; }
	sh tools/psx_capture_record.sh ceilings $(PSX_CHECK_OUT)/run $(LABEL) $(PSX_CAPTURE_CEILINGS)

psx-gpu-capture-accept:
	@[ -n "$(LABEL)" ] || { echo "psx-gpu-capture-accept: LABEL missing: make psx-gpu-capture-accept LABEL=<label>" >&2; exit 1; }
	sh tools/psx_capture_record.sh references $(PSX_CHECK_OUT)/run $(LABEL) $(PSX_CAPTURE_REFERENCES)

PSX_CAPTURE_REPORT_OUT := $(TMP_DIR)/psx/gpu-capture

psx-gpu-capture-report: psx-build $(TMP_DIR)/replay $(TMP_DIR)/psx_capture_compare
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

psx-emulator-check:
	PSX_BOOT_TIMEOUT=$(PSX_BOOT_TIMEOUT) sh tools/psx_emulator_check.sh $(PSX_EMULATOR_DIR) \
		$(PSX_SOURCES) $(TMP_DIR)/psx/media $(TMP_DIR)/psx/check '$(PSX_KNOWN_LINE)' $(PSX_FRAMES)

PSX_HELLO_TTY_LINE ?= psx-hello: main loop started

psx-hello-check: psx-hello
	PSX_BOOT_TIMEOUT=$(PSX_BOOT_TIMEOUT) sh tools/psx_hello_check.sh $(PSX_EMULATOR_DIR) \
		--exe psx-hello/build/psx-hello.exe '$(PSX_HELLO_TTY_LINE)' $(PSX_FRAMES) $(TMP_DIR)/psx/hello-check

psx-hello-cd-check: psx-hello-cd
	PSX_BOOT_TIMEOUT=$(PSX_BOOT_TIMEOUT) sh tools/psx_hello_check.sh $(PSX_EMULATOR_DIR) \
		--iso $(PSX_HELLO_CUE) '$(PSX_HELLO_TTY_LINE)' $(PSX_FRAMES) $(TMP_DIR)/psx/hello-cd-check

PSX_HELLO_REFERENCE ?= psx-hello/psx-hello-cd.vram.sha256
PSX_HELLO_REFERENCE_FRAMES := 600
PSX_HELLO_CD_CAPTURE := $(TMP_DIR)/psx/hello-cd-check/run1.vram
# A prerequisite, not a recipe line: under -j it fails while the image builds, so the boots never start.
psx-hello-frames-guard:
	@[ "$(PSX_FRAMES)" = "$(PSX_HELLO_REFERENCE_FRAMES)" ] || { echo "psx-hello-e2e: PSX_FRAMES is $(PSX_FRAMES), the reference was taken at $(PSX_HELLO_REFERENCE_FRAMES)" >&2; exit 1; }

psx-hello-e2e: psx-hello-frames-guard psx-hello-cd-check
	@sh tools/psx_reference_check.sh $(PSX_HELLO_REFERENCE) $(PSX_HELLO_CD_CAPTURE)

psx-hello-e2e-baseline: psx-hello-frames-guard psx-hello-cd-check
	@h=$$($(SHA256_CMD) $(PSX_HELLO_CD_CAPTURE)) && echo "$${h%% *}" > $(PSX_HELLO_REFERENCE) \
		&& echo "psx-hello-e2e: reference set in $(PSX_HELLO_REFERENCE) ($${h%% *})"

# Clones a pinned raylib version into third_party/ (gitignored) and builds its
# static library. Slow, network, once, deliberately outside `make test`.
raylib-vendor-setup:
	sh tools/install_raylib_vendor.sh

# Link only: this target never runs the binary (no ./$(TMP_DIR)/... line), see
# CLAUDE.md. It needs third_party/raylib/src/libraylib.a, so `make raylib`
# without vendoring fails with "no rule to make target".
# The raylib target builds straight from the sources to the executable:
# its files need `-I third_party/raylib/src`, which the others do not have.
GAME_SRCS := \
    src/cmd/game.c \
    src/platform/raylib_backend.c \
    src/platform/keymap.c \
    src/p8/palette.c \
    src/devtools/replay_control.c \
    src/devtools/replay_source.c \
    src/devtools/session_recorder.c \
    src/platform/save_storage_file.c \
    $(filter-out src/devtools/save_storage_memory.c,$(ENGINE_SRCS))

$(TMP_DIR)/game: $(GAME_SRCS) $(wildcard src/*/*.h) third_party/raylib/src/libraylib.a | $(TMP_DIR)
	$(CC) $(CFLAGS) -I third_party/raylib/src -o $@ $(GAME_SRCS) third_party/raylib/src/libraylib.a $(RAYLIB_LDLIBS)

raylib: $(TMP_DIR)/game

# The dependency fragments written by the compiler, one per object. The
# wildcard tolerates their absence: on a fresh clone, everything is compiled.
-include $(wildcard $(OBJ_DIR)/src/*/*.d $(OBJ_DIR)/tests/*.d)
-include $(wildcard $(OBJ_ASAN_DIR)/src/*/*.d $(OBJ_ASAN_DIR)/tests/*.d)
-include $(wildcard $(OBJ_UBSAN_DIR)/src/*/*.d $(OBJ_UBSAN_DIR)/tests/*.d)
-include $(wildcard $(OBJ_SANITIZE_DIR)/src/*/*.d $(OBJ_SANITIZE_DIR)/tests/*.d)
-include $(wildcard $(OBJ_COVERAGE_DIR)/src/*/*.d)

