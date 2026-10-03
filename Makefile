# Everything runs in parallel (`-j` set to the number of cores, output grouped
# per target).
#
# make test          -> unit + constants-check + float-grep + float-guard +
#                       alloc-guard + data-guard + composition-guard, and the
#                       tool tests and checks marked "in make test" below,
#                       the fast loop
# make e2e           -> replays each e2e of tests/e2e/ from the title screen,
#                       with the `-O0` binary of make test, and compares the
#                       sha256 of its capture to <name>.json.sha256; also fails
#                       if an e2e lacks its .txt or its .json.sha256, or runs
#                       past 180 frames (600 if listed in
#                       tools/e2e_long_exceptions.txt)
# make test-all      -> test + e2e, the full suite
# make gif           -> gif/<name>.gif for each e2e of tests/e2e/: its capture
#                       at scale 3, with the recorded mouse drawn on top,
#                       outside the suite
# make unit          -> the binaries of tests/, built and run
# make replay EVENTS=<events> -> replays that file in the headless engine,
#                       framebuffer dump in tmp/replay_dump.txt, outside the
#                       suite
# make screencast EVENTS=<events> -> rebuilds the pixels of each frame of that
#                       replay into tmp/screencast.bin, for screencast-viewer
# make e2e-baseline  -> sets or resets the .json.sha256 of tests/e2e/ from the
#                       current code, never run by make test
# make e2e-ref       -> for each diverging e2e, replays the commit that set its
#                       reference (git blame), leaves that capture in
#                       tmp/frames/<events>.ref.bin and gives the first
#                       diverging frame, outside the suite
# make class-coverage [DIR=<directory>] -> replays the e2e of tests/e2e/ (with
#                       DIR: each *events.json placed directly in DIR, not in
#                       its subdirectories) and counts the instantiations per
#                       class; fails if a class is at 0 outside
#                       tools/class_coverage_exempt.txt, outside the suite
# make line-coverage EVENTS="<events>..." -> the lines executed by these
#                       replays (gcov-instrumented driver), one `file:line` per
#                       line, in tmp/line_coverage.txt, outside the suite
# make line-coverage-contrib EVENTS=<events> OTHERS="<events>..." -> the lines
#                       that EVENTS covers and that none of OTHERS covers (the
#                       criterion for keeping a replay), in
#                       tmp/line_coverage_contrib.txt, outside the suite
# make e2e-gap       -> the lines of tools/line_coverage_reference.txt that
#                       make e2e does not cover, minus
#                       tools/line_coverage_unreachable.txt; fails until the
#                       gap is empty, outside the suite
# make minimize EVENTS=<events> OUT=<output> -> minimizes that replay (played
#                       from the title screen): the mechanical stage (the
#                       `mechanical` command of tmp/minimize, which
#                       tools/minimize.sh runs first), then the guided pass,
#                       as long as the line coverage stays that of the input,
#                       one gcov replay per candidate, slow; MECHANICAL=1: the
#                       mechanical stage alone; `MIN_CHUNK=<n>`: chunks of at
#                       least that many frames; outside the suite
# make e2e-cleanup   -> the final cleanup pass over tests/e2e/: lists, from the
#                       longest to the shortest, the e2e whose lines the other
#                       remaining ones cover; deletes nothing, outside the
#                       suite
# make e2e-judge EVENTS=<events> -> the lines that this replay (from the title
#                       screen) adds to the other e2e of tests/e2e/; fails if
#                       it adds none, outside the suite
# make class-coverage-net -> like class-coverage, over the e2e of tests/e2e/;
#                       also fails if its output differs from
#                       tools/class_coverage_net_expected.txt, outside the
#                       suite
# make data-residue  -> counts the data residue per category
#                       (tools/data_residue.sh); the categories already at 0
#                       are listed in DATA_RESIDUE_GUARD, outside the suite
# make composition-residue -> counts the composition residue per category
#                       (tools/composition_residue.sh); the categories already
#                       at 0 are listed in COMPOSITION_RESIDUE_GUARD, outside
#                       the suite
# make data-guard    -> no offsetof in src/, and the categories of
#                       DATA_RESIDUE_GUARD at 0, in make test
# make composition-guard -> the categories of COMPOSITION_RESIDUE_GUARD at 0,
#                       in make test
# make sanitize-net  -> replays the e2e of tests/e2e/ under ASan+UBSan (`-O2`);
#                       passes when every replay exits cleanly, outside the
#                       suite
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
# make raylib        -> compiles and LINKS the interactive binary, never runs it
# make screencast-viewer -> the same for the recording viewer
#                       (both need make raylib-vendor-setup first)
# make psx-hello     -> PSX test project (needs psn00bsdk-toolchain-setup)
# make rename-tool-setup -> installs the libclang Python bindings
#                       (tools/ast_rename) into third_party/clang-python;
#                       network, once, outside the suite
# make rename-tool-test -> the tests of tools/ast_rename on the mini tree
#                       tests/fixtures/ast_rename; needs rename-tool-setup,
#                       outside the suite
# make rename-extract -> each declaration of src/, tests/, tools/ and its
#                       occurrences, in the default, coverage and ASan
#                       configurations: tmp/ast_rename/declarations.tsv and
#                       occurrences.tsv; fails if a unit does not parse,
#                       outside the suite
# make rename-derive-test -> the tests of tools/rename_derive.py on a mini
#                       vocabulary and a mini extraction, in make test
# make rename-derive -> derives the new names of each extracted declaration
#                       (make rename-extract first): the identifier rows of
#                       docs/renommage.tsv, the rest in
#                       tmp/ast_rename/remainder.tsv, outside the suite
# make rename-strings-test -> the tests of tools/rename_strings.py on a mini
#                       tree, in make test
# make rename-strings -> the string and file rows of docs/renommage.tsv (keys,
#                       quoted symbols, test names, binaries of tmp/), one
#                       writer or reader site per file; flags in
#                       tmp/ast_rename/string_flags.tsv, outside the suite
# make rename-check-test -> the tests of tools/rename_check.py on a mini
#                       extraction, in make test
# make rename-table-check -> mechanical check of docs/renommage.tsv against the
#                       extraction (make rename-extract, rename-derive):
#                       completeness, collisions, shadowing, reserved words,
#                       target file names; fails on a violation, outside the
#                       suite
# make rename-slice-test -> the tests of tools/rename_slice.py, in make test
# make rename-slice MODULES="<module>..." | SUBSTEP=<3x> -> the renamed
#                       identifier rows of docs/renommage.tsv for these
#                       modules (shell patterns on the module column:
#                       game/planet*), or for exactly the modules of the
#                       sub-step in tools/rename_substeps.tsv, in the
#                       rename-apply format, in tmp/ast_rename/slice.tsv,
#                       outside the suite
# make rename-apply TABLE=<table> -> renames, at their AST positions, the
#                       entries of the table (old, new, file[, identity]);
#                       writes nothing on the first refusal, report in
#                       tmp/ast_rename/report.txt, outside the suite
# make rename-count-test -> the tests of tools/rename_count.py on a mini
#                       tree, in make test
# make cart-names-count [MODULES="<module>..."] -> old-name counter: the old
#                       names of the renamed rows of docs/renommage.tsv still
#                       in the code, per module and category (short names
#                       through tmp/ast_rename/occurrences.tsv, make
#                       rename-extract first); copy in
#                       tmp/ast_rename/cart_names_count.txt, never fails,
#                       outside the suite; SUBSTEP=<3x>: the modules of the
#                       sub-step, each mention sorted as done (a name of that
#                       sub-step or an earlier one) or later, then the residue
#                       bound
# make rename-substeps-test -> the tests of tools/rename_substeps.py on a mini
#                       table, in make test
# make rename-substeps-check -> each module of docs/renommage.tsv in exactly
#                       one sub-step of tools/rename_substeps.tsv, each
#                       sub-step with its bound, in make test
# make rename-strings-verify-test -> the tests of
#                       tools/rename_strings_verify.py on a mini tree, in
#                       make test
# make rename-strings-verify SUBSTEP=<3x> -> at each site of the renamed string
#                       and file rows of the sub-step, the new name present
#                       and the old one absent; fails otherwise, outside the
#                       suite
# make rename-rekey-test -> the tests of tools/rename_rekey.py on mini
#                       extractions, in make test
# make rename-rekey [BEFORE=<declarations>] -> after rename-apply and a fresh
#                       rename-extract: rewrites in docs/renommage.tsv only
#                       the identity cells of the rows not yet applied that
#                       the application has shifted, from the copy taken
#                       before (tmp/ast_rename/declarations_before.tsv by
#                       default); writes nothing on a missing or duplicate
#                       match, outside the suite
# make rename-rekey-check SUBSTEP=<3x> -> each renamed identity of the later
#                       sub-steps is in tmp/ast_rename/declarations.tsv with
#                       its name and file; fails otherwise, outside the suite
# make scope-count-test -> the tests of tools/scope_count.py on a mini
#                       extraction, in make test
# make scope-count [MODULES="<module>..." | SUBSTEP=<3x>] -> the scope guard as
#                       a counter: declarations of one or two characters
#                       refused (type, function, field, constant, macro,
#                       static, global; local or parameter of scope > 40
#                       lines), per module, minus tools/scope_exceptions.txt;
#                       reads tmp/ast_rename/declarations.tsv (make
#                       rename-extract first), never fails, outside the suite
# make cryptic-locals [MODULES="<module>..." | SUBSTEP=<3x>] -> the cryptic
#                       locals list: each local or parameter of scope > 40
#                       lines named in three characters or less, or with a
#                       `_` part found in no renamed new name of
#                       docs/renommage.tsv; reads
#                       tmp/ast_rename/declarations.tsv (make rename-extract
#                       first), never fails, outside the suite
# make cart-citations-test -> the tests of tools/cart_citations.py on a mini
#                       tree, in make test
# make cart-citations [MODULES="<module>..." | SUBSTEP=<3x>] -> what comments
#                       and assert messages still cite beyond
#                       cart-names-count: `cart` (outside game/cart), `Lua`,
#                       `lua.lua`, bare old names of two characters or less;
#                       one site per line, then per module; never fails,
#                       outside the suite
# make french-count-test -> the tests of tools/french_count.py on a mini
#                       tree, in make test
# make french-count [MODULES="<module>..." | SUBSTEP=<3x>] [SITES=1] -> French
#                       counter: French function words and accented words
#                       left in the code, per module and column, minus
#                       tools/french_exceptions.txt; SUBSTEP: the sub-step's
#                       modules, split done / later; SITES=1 lists each
#                       mention; warns for a module in no sub-step, never
#                       fails, outside the suite
# make old-names-guard -> the old-name guard, text half: the old names of
#                       docs/renommage.tsv that a text search finds in the code
#                       (`rename_count.py --part text`), without libclang;
#                       fails on any residue; not yet in make test
# make ast-guard     -> a fresh rename-extract, then the old-name guard, AST
#                       half (the ambiguous old names as identifiers), and the
#                       scope guard, both tables printed; fails on any residue;
#                       not yet in make test

.PHONY: psn00bsdk-toolchain-setup psx-hello psx-hello-clean unit replay test e2e test-all gif raylib-vendor-setup raylib screencast screencast-viewer e2e-baseline e2e-ref rename-tool-setup rename-tool-test rename-extract rename-apply rename-derive rename-derive-test rename-strings rename-strings-test rename-check-test rename-table-check rename-slice rename-slice-test rename-count-test cart-names-count rename-substeps-test rename-substeps-check rename-strings-verify-test rename-strings-verify rename-rekey-test rename-rekey rename-rekey-check scope-count-test scope-count cryptic-locals cart-citations-test cart-citations french-count-test french-count old-names-guard ast-guard

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
# blind on that code, silently. Do not merge the six.
OBJ_DIR := $(TMP_DIR)/obj
OBJ_ASAN_DIR := $(TMP_DIR)/obj-asan
OBJ_UBSAN_DIR := $(TMP_DIR)/obj-ubsan
OBJ_COVERAGE_DIR := $(TMP_DIR)/obj-coverage
OBJ_GCOV_DIR := $(TMP_DIR)/obj-gcov
OBJ_SANITIZE_DIR := $(TMP_DIR)/obj-sanitize

# `-MMD -MP`: next to each object, the compiler writes the list of headers that
# the translation unit actually opened. The include at the bottom of this file
# reads them back, so no header is ever named by hand in a prerequisite list.
DEPFLAGS := -MMD -MP
ASAN_CFLAGS := -fsanitize=address -fno-omit-frame-pointer
UBSAN_CFLAGS := -fsanitize=undefined -fno-sanitize-recover=undefined
COVERAGE_CFLAGS := -DPICO8_CLASS_COVERAGE
GCOV_CFLAGS := --coverage
SANITIZE_OPT_CFLAGS := -O2

# `$(call objs,<sources>)` -> the paths of the matching object files in the
# tree concerned. A target names its sources once; its prerequisites and its
# link line follow from them.
objs = $(patsubst %.c,$(OBJ_DIR)/%.o,$(1))
asan_objs = $(patsubst %.c,$(OBJ_ASAN_DIR)/%.o,$(1))
ubsan_objs = $(patsubst %.c,$(OBJ_UBSAN_DIR)/%.o,$(1))
coverage_objs = $(patsubst %.c,$(OBJ_COVERAGE_DIR)/%.o,$(1))
gcov_objs = $(patsubst %.c,$(OBJ_GCOV_DIR)/%.o,$(1))
sanitize_objs = $(patsubst %.c,$(OBJ_SANITIZE_DIR)/%.o,$(1))

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

$(OBJ_COVERAGE_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(COVERAGE_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_GCOV_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(GCOV_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_SANITIZE_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) $(UBSAN_CFLAGS) $(SANITIZE_OPT_CFLAGS) $(DEPFLAGS) -c -o $@ $<

# The whole engine closure: all of src/ except the seven entry points of
# src/cmd/, src/p8/palette.c, the two `src/platform/raylib_backend*.c` and
# the modules of src/devtools/ that only the tools and the interactive game
# link (fixed_test_double, gif, minimize, replay_control, replay_source,
# screencast, screencast_player, session_recorder). Used by the targets that
# call cart_load(), which pulls in the whole port.
ENGINE_SRCS := \
    src/p8/assets.c \
    src/p8/audio.c \
    src/game/background.c \
    src/engine/bootstrap.c \
    src/game/cheat.c \
    src/game/cart.c \
    src/engine/collect.c \
    src/devtools/class_coverage.c \
    src/game/data_init.c \
    src/engine/dispatch_context.c \
    src/devtools/dump.c \
    src/game/economy.c \
    src/game/economy_formulas.c \
    src/game/economy_score.c \
    src/game/economy_tiers.c \
    src/engine/entity_registry.c \
    src/engine/entity_geom.c \
    src/engine/exit_queue.c \
    src/platform/fatal_pc.c \
    src/p8/fixed.c \
    src/game/galaxy.c \
    src/game/game_data.c \
    src/game/gen.c \
    src/engine/geom.c \
    src/p8/gfx.c \
    src/game/help.c \
    src/game/hud.c \
    src/p8/input.c \
    src/devtools/input_json.c \
    src/engine/entity.c \
    src/game/layout.c \
    src/engine/entity_list.c \
    src/engine/pointer_list.c \
    src/game/map_tables.c \
    src/p8/mem.c \
    src/game/menu.c \
    src/game/mouse_button.c \
    src/game/mouse.c \
    src/game/message_box.c \
    src/game/particle.c \
    src/game/draw_functions.c \
    src/game/planet.c \
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
    src/engine/zone.c \
    src/engine/allocator.c \
    src/engine/frame_zone.c \
    src/game/ui_zone.c \
    src/game/particle_zone.c \
    src/engine/game_zone.c

# The ported core, free of floating point: the engine closure minus the host
# tools, which keep the libc. constants-check, float-guard, alloc-guard and
# float-grep all read this list.
CORE_SRCS := $(filter-out src/p8/assets.c src/devtools/class_coverage.c src/devtools/dump.c src/platform/fatal_pc.c src/devtools/input_json.c,$(ENGINE_SRCS))

# A narrower closure: what data_init() reaches to build a real game state,
# without calling cart_load().
DATA_ENGINE_SRCS := \
    src/game/data_init.c \
    src/p8/assets.c \
    src/p8/audio.c \
    src/game/background.c \
    src/engine/bootstrap.c \
    src/engine/collect.c \
    src/engine/dispatch_context.c \
    src/devtools/dump.c \
    src/game/economy.c \
    src/game/economy_formulas.c \
    src/game/economy_score.c \
    src/game/economy_tiers.c \
    src/engine/entity_registry.c \
    src/engine/entity_geom.c \
    src/engine/exit_queue.c \
    src/p8/fixed.c \
    src/game/game_data.c \
    src/game/gen.c \
    src/engine/geom.c \
    src/p8/gfx.c \
    src/game/hud.c \
    src/p8/input.c \
    src/devtools/input_json.c \
    src/engine/entity.c \
    src/game/layout.c \
    src/engine/entity_list.c \
    src/engine/pointer_list.c \
    src/game/map_tables.c \
    src/p8/mem.c \
    src/game/mouse_button.c \
    src/game/mouse.c \
    src/game/message_box.c \
    src/game/particle.c \
    src/game/draw_functions.c \
    src/game/planet.c \
    src/game/planet_display.c \
    src/game/planet_family.c \
    src/game/planet_trade.c \
    src/game/probe.c \
    src/game/research.c \
    src/game/research_display.c \
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
    src/engine/zone.c \
    src/engine/allocator.c \
    src/engine/frame_zone.c \
    src/game/ui_zone.c \
    src/game/particle_zone.c \
    src/engine/game_zone.c \
    src/platform/fatal_pc.c

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
$(TMP_DIR)/test_mem: $(call objs,tests/test_mem.c src/p8/mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_asset_loading: $(call objs,tests/test_asset_loading.c src/p8/assets.c src/p8/mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_gfx: $(call objs,tests/test_gfx.c src/p8/gfx.c src/p8/mem.c src/p8/assets.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_input: $(call objs,tests/test_input.c src/p8/input.c src/devtools/input_json.c) tests/fixtures/demo_events.json tests/fixtures/events_valid_ox.json tests/fixtures/events_malformed_five_dpad.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_dump: $(call objs,tests/test_dump.c src/devtools/dump.c src/p8/gfx.c src/p8/mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_fixed: $(call objs,tests/test_fixed.c src/p8/fixed.c src/devtools/fixed_test_double.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# The same test as above, under UBSan: p8/fixed.c reinterprets bits between
# signed and unsigned, which would give the right numbers while being undefined
# behavior. `-fno-sanitize-recover` makes the binary fail instead of printing a
# warning.
$(TMP_DIR)/test_fixed_ubsan: $(call ubsan_objs,tests/test_fixed.c src/p8/fixed.c src/devtools/fixed_test_double.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(UBSAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_raylib_backend_coords: $(call objs,tests/test_raylib_backend_coords.c src/platform/raylib_backend_coords.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_replay_source: $(call objs,tests/test_replay_source.c src/devtools/replay_source.c src/p8/input.c src/devtools/input_json.c) tests/fixtures/events_valid.json tests/fixtures/events_malformed_missing_field.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_recording_continuity: $(call objs,tests/test_recording_continuity.c src/devtools/session_recorder.c src/devtools/replay_source.c src/p8/input.c src/devtools/input_json.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_replay_control: $(call objs,tests/test_replay_control.c src/devtools/replay_control.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_marker_coherence: $(call objs,tests/test_marker_coherence.c src/devtools/session_recorder.c src/devtools/replay_source.c src/p8/input.c src/devtools/input_json.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_screencast: $(call objs,tests/test_screencast.c src/devtools/screencast.c src/p8/mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# This test runs the real replay driver, hence $(TMP_DIR)/replay as a
# prerequisite: it replays the same fixture with and without a capture and
# compares the two dumps, so that the capture argument has no effect on the
# rest.
$(TMP_DIR)/test_headless_capture: $(call objs,tests/test_headless_capture.c src/devtools/screencast.c src/devtools/dump.c src/p8/gfx.c src/p8/mem.c src/p8/input.c src/devtools/input_json.c) $(TMP_DIR)/replay tests/fixtures/screencast_capture_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_replay_from_title: $(call objs,tests/test_replay_from_title.c) $(TMP_DIR)/replay tests/fixtures/title_click_forgiving.json tests/fixtures/title_click_reasonable.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_replay_usage: $(call objs,tests/test_replay_usage.c) $(TMP_DIR)/replay tests/fixtures/title_click_forgiving.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/frames_tool: $(call objs,src/cmd/frames.c src/devtools/screencast.c src/devtools/dump.c src/p8/mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Runs the replay driver, then the frame emission tool.
$(TMP_DIR)/test_frames_tool: $(call objs,tests/test_frames_tool.c src/devtools/screencast.c src/p8/mem.c) $(TMP_DIR)/replay $(TMP_DIR)/frames_tool tests/fixtures/screencast_capture_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_frames_check: $(call objs,tests/test_frames_check.c) tools/frames_check.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_class_coverage: $(call coverage_objs,tests/test_class_coverage.c src/devtools/class_coverage.c) $(TMP_DIR)/class_coverage tools/class_coverage.sh tests/fixtures/title_click_forgiving.json tests/fixtures/events_malformed_missing_field.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_line_coverage: $(call objs,tests/test_line_coverage.c) $(TMP_DIR)/replay_gcov tools/line_coverage.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_e2e_limits: $(call objs,tests/test_e2e_limits.c) tools/e2e_limits.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_e2e_cleanup: $(call objs,tests/test_e2e_cleanup.c) tools/e2e_cleanup.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/minimize: $(call objs,src/cmd/minimize.c src/devtools/minimize.c src/p8/input.c src/devtools/input_json.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_minimize: $(call objs,tests/test_minimize.c src/devtools/minimize.c src/p8/input.c src/devtools/input_json.c) $(TMP_DIR)/minimize tools/minimize.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_guard: $(call objs,tests/test_float_guard.c) tools/float_guard.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_alloc_guard: $(call objs,tests/test_alloc_guard.c) tools/alloc_guard.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_grep: $(call objs,tests/test_float_grep.c) tools/float_grep.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_data_residue: $(call objs,tests/test_data_residue.c) tools/data_residue.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_composition_residue: $(call objs,tests/test_composition_residue.c) tools/composition_residue.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_link: $(call objs,tests/test_float_link.c) tools/float_link.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_constants_check: $(call objs,tests/test_constants_check.c) tools/constants_compare.sh tools/constants_check.sh src/p8/fixed.c src/p8/fixed.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/gif: $(call objs,src/cmd/gif.c src/devtools/gif.c src/p8/palette.c src/devtools/screencast.c src/p8/mem.c src/p8/input.c src/devtools/input_json.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_gif: $(call objs,tests/test_gif.c src/devtools/gif.c src/p8/palette.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_screencast_player: $(call objs,tests/test_screencast_player.c src/devtools/screencast_player.c src/p8/palette.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_vec: $(call objs,tests/test_vec.c src/engine/vec.c src/p8/fixed.c src/devtools/fixed_test_double.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_geom: $(call objs,tests/test_geom.c src/engine/geom.c src/engine/vec.c src/p8/fixed.c src/devtools/fixed_test_double.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pointer_list: $(call objs,tests/test_pointer_list.c tests/heap_allocator.c src/engine/pointer_list.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_arena: $(call objs,tests/test_arena.c tests/heap_allocator.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
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

$(TMP_DIR)/test_draw_layers_camera: $(call objs,tests/test_draw_layers_camera.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_demo: $(call objs,tests/test_demo.c src/p8/mem.c src/p8/assets.c src/p8/gfx.c src/p8/input.c src/devtools/input_json.c src/devtools/dump.c) tests/fixtures/demo_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

REPLAY_SRCS := src/cmd/replay.c src/devtools/screencast.c $(ENGINE_SRCS)

$(TMP_DIR)/replay: $(call objs,$(REPLAY_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/replay_gcov: $(call gcov_objs,$(REPLAY_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(GCOV_CFLAGS) -o $@ $(filter %.o,$^)

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

$(TMP_DIR)/test_ui_zone: $(call asan_objs,tests/test_ui_zone.c src/game/ui_zone.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
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
$(TMP_DIR)/test_layout_nil_item_draws_nothing: $(call objs,tests/test_layout_nil_item_draws_nothing.c tests/heap_allocator.c src/p8/assets.c src/engine/collect.c src/engine/dispatch_context.c src/p8/fixed.c src/engine/geom.c src/p8/gfx.c src/game/hud.c src/game/layout.c src/engine/pointer_list.c src/game/map_tables.c src/p8/mem.c src/p8/str.c src/engine/vec.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# The longest first: under `-j`, make starts it first.
UNIT_TESTS := \
    test_composition_residue \
    test_mem \
    test_asset_loading \
    test_gfx \
    test_input \
    test_dump \
    test_fixed \
    test_fixed_ubsan \
    test_vec \
    test_geom \
    test_pointer_list \
    test_entity_list \
    test_str \
    test_data_init \
    test_map_tables \
    test_draw_layers_camera \
    test_demo \
    test_raylib_backend_coords \
    test_replay_source \
    test_recording_continuity \
    test_replay_control \
    test_marker_coherence \
    test_screencast \
    test_headless_capture \
    test_replay_from_title \
    test_replay_usage \
    test_screencast_player \
    test_gif \
    test_frames_tool \
    test_frames_check \
    test_class_coverage \
    test_line_coverage \
    test_e2e_limits \
    test_minimize \
    test_e2e_cleanup \
    test_float_guard \
    test_alloc_guard \
    test_float_grep \
    test_data_residue \
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
    test_particle_zone \
    test_game_zone \
    test_exit_queue \
    test_burst_exports_row \
    test_layout_nil_item_draws_nothing \
    test_tooltip_draw_at \
    test_toast \
    test_message_box \
    test_live_slots \
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
    test_main_economy

# One run per test, each its own target: that is what `-j` runs in parallel.
UNIT_RUNS := $(addprefix unit-run-,$(UNIT_TESTS))

UNIT_RUNS_SPECIAL := unit-run-test_zone unit-run-test_allocator unit-run-test_frame_zone unit-run-test_ui_zone unit-run-test_particle_zone unit-run-test_game_zone unit-run-test_exit_queue unit-run-test_toast unit-run-test_message_box unit-run-test_live_slots unit-run-test_trade

unit: $(UNIT_RUNS)

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
ifneq ($(filter replay screencast,$(MAKECMDGOALS)),)
    ifeq ($(strip $(EVENTS)),)
        $(error EVENTS missing: make $(filter replay screencast,$(MAKECMDGOALS)) EVENTS=<events.json>)
    endif
endif

replay: $(TMP_DIR)/replay
	./$(TMP_DIR)/replay $(EVENTS)

test: unit constants-check float-grep float-guard alloc-guard data-guard composition-guard rename-derive-test rename-strings-test rename-check-test rename-slice-test rename-count-test rename-substeps-test rename-substeps-check rename-strings-verify-test rename-rekey-test scope-count-test french-count-test cart-citations-test

test-all: test e2e

SCREENCAST_BIN := $(TMP_DIR)/screencast.bin

SCREENCAST_VIEWER_BIN := $(TMP_DIR)/screencast_viewer

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

# Rebuilt on every call: EVENTS changes from one call to the next without make
# seeing it. A file target rather than a phony one, so that .DELETE_ON_ERROR
# applies.
$(SCREENCAST_BIN): $(TMP_DIR)/replay FORCE | $(TMP_DIR)
	./$(TMP_DIR)/replay $(EVENTS) $@ $(@:.bin=.dump.txt)

screencast: $(SCREENCAST_BIN)

.PHONY: FORCE
FORCE:

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

GIF_DIR := gif

gif: $(foreach e,$(E2E_EVENTS),$(GIF_DIR)/$(basename $(notdir $(e))).gif)
	@echo "gif: $(words $(E2E_EVENTS)) GIF(s) in $(GIF_DIR)/, $$(du -ch $(GIF_DIR)/*.gif | tail -1 | cut -f1)"

$(GIF_DIR)/%.gif: $(call frames_capture,$(E2E_DIR)/%.json) $(E2E_DIR)/%.json $(TMP_DIR)/gif
	@mkdir -p $(@D)
	./$(TMP_DIR)/gif $< $(E2E_DIR)/$*.json $@

e2e-baseline: $(foreach e,$(E2E_EVENTS),$(call frames_current,$(e)))
	@$(foreach e,$(E2E_EVENTS),cp $(call frames_current,$(e)) $(call frames_reference,$(e)) && echo "e2e: reference set in $(call frames_reference,$(e))" && ) true

e2e-ref: $(foreach e,$(E2E_EVENTS),$(if $(wildcard $(call frames_reference,$(e))),$(call frames_current,$(e)))) $(FRAMES_TOOL) tools/frames_ref.sh
	@sh tools/frames_ref.sh ./$(FRAMES_TOOL) $(FRAMES_DIR) $(TMP_DIR)/frames-ref $(foreach e,$(E2E_EVENTS),$(e) $(call frames_reference,$(e)) $(call frames_current,$(e)) $(call frames_capture,$(e)))

# Everything comes from the coverage tree, which is linked only here and in
# test_class_coverage: the counter is compiled in only under
# PICO8_CLASS_COVERAGE.
$(TMP_DIR)/class_coverage: $(call coverage_objs,src/cmd/class_coverage.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

CLASS_COVERAGE_EXEMPT := tools/class_coverage_exempt.txt
CLASS_COVERAGE_NET_EXPECTED := tools/class_coverage_net_expected.txt

class-coverage: $(TMP_DIR)/class_coverage tools/class_coverage.sh $(CLASS_COVERAGE_EXEMPT)
	@sh tools/class_coverage.sh ./$(TMP_DIR)/class_coverage '$(or $(DIR),-)' $(CLASS_COVERAGE_EXEMPT) $(if $(DIR),,$(E2E_EVENTS))

class-coverage-net: $(TMP_DIR)/class_coverage tools/class_coverage.sh $(CLASS_COVERAGE_EXEMPT) $(CLASS_COVERAGE_NET_EXPECTED)
	@sh tools/class_coverage.sh --expect $(CLASS_COVERAGE_NET_EXPECTED) ./$(TMP_DIR)/class_coverage - $(CLASS_COVERAGE_EXEMPT) $(E2E_EVENTS)

.PHONY: class-coverage class-coverage-net

LINE_COVERAGE := sh tools/line_coverage.sh
LINE_COVERAGE_COVER := $(LINE_COVERAGE) cover ./$(TMP_DIR)/replay_gcov $(OBJ_GCOV_DIR)
# Written once and never recomputed: it is only compared against.
LINE_COVERAGE_REFERENCE := tools/line_coverage_reference.txt
# One `file:line reason` line per line of the reference that no short replay
# reaches, edited by hand.
LINE_COVERAGE_UNREACHABLE := tools/line_coverage_unreachable.txt

line-coverage: $(TMP_DIR)/replay_gcov tools/line_coverage.sh
	@$(LINE_COVERAGE_COVER) $(EVENTS) > $(TMP_DIR)/line_coverage.txt
	@echo "line coverage: $$(wc -l < $(TMP_DIR)/line_coverage.txt) line(s) in $(TMP_DIR)/line_coverage.txt"

line-coverage-contrib: $(TMP_DIR)/replay_gcov tools/line_coverage.sh
	@$(LINE_COVERAGE_COVER) $(EVENTS) > $(TMP_DIR)/line_coverage_self.txt
	@$(if $(strip $(OTHERS)),$(LINE_COVERAGE_COVER) $(OTHERS),:) > $(TMP_DIR)/line_coverage_others.txt
	@$(LINE_COVERAGE) contrib $(TMP_DIR)/line_coverage_self.txt $(TMP_DIR)/line_coverage_others.txt > $(TMP_DIR)/line_coverage_contrib.txt
	@echo "contribution of $(EVENTS): $$(wc -l < $(TMP_DIR)/line_coverage_contrib.txt) line(s) in $(TMP_DIR)/line_coverage_contrib.txt"

# Without any e2e, the covered set is empty: the gap is the whole reference.
e2e-gap: $(TMP_DIR)/replay_gcov tools/line_coverage.sh $(LINE_COVERAGE_REFERENCE) $(LINE_COVERAGE_UNREACHABLE)
	@$(if $(E2E_EVENTS),$(LINE_COVERAGE_COVER) $(E2E_EVENTS),:) > $(TMP_DIR)/line_coverage_e2e.txt
	@$(LINE_COVERAGE) gap $(LINE_COVERAGE_REFERENCE) $(LINE_COVERAGE_UNREACHABLE) $(TMP_DIR)/line_coverage_e2e.txt > $(TMP_DIR)/line_coverage_gap.txt
	@sed 's/^/missing: /' $(TMP_DIR)/line_coverage_gap.txt
	@n=$$(wc -l < $(TMP_DIR)/line_coverage_gap.txt | tr -d ' '); \
	if [ "$$n" -ne 0 ]; then \
	    echo "gap: $$n line(s) of $(LINE_COVERAGE_REFERENCE) neither covered by make e2e ($(words $(E2E_EVENTS)) e2e) nor in $(LINE_COVERAGE_UNREACHABLE), in $(TMP_DIR)/line_coverage_gap.txt"; \
	    exit 1; \
	fi
	@echo "gap: none -- make e2e covers the whole reference outside $(LINE_COVERAGE_UNREACHABLE)"

.PHONY: line-coverage line-coverage-contrib e2e-gap

ifneq ($(filter minimize,$(MAKECMDGOALS)),)
    ifeq ($(and $(strip $(EVENTS)),$(strip $(OUT))),)
        $(error EVENTS or OUT missing: make minimize EVENTS=<events.json> OUT=<output.json>)
    endif
endif
ifneq ($(filter e2e-judge,$(MAKECMDGOALS)),)
    ifeq ($(strip $(EVENTS)),)
        $(error EVENTS missing: make e2e-judge EVENTS=<events.json>)
    endif
endif

minimize: $(TMP_DIR)/minimize $(TMP_DIR)/replay_gcov tools/minimize.sh tools/line_coverage.sh
	@sh tools/minimize.sh ./$(TMP_DIR)/minimize $(if $(MIN_CHUNK),--min-chunk $(MIN_CHUNK)) $(EVENTS) $(OUT) $(if $(MECHANICAL),,$(LINE_COVERAGE_COVER))
	@echo "minimize: $(EVENTS) -> $(OUT), $$(./$(TMP_DIR)/minimize count $(OUT)) frame(s)"

E2E_CLEANUP_DIR := $(TMP_DIR)/e2e_cleanup

e2e-cleanup: $(TMP_DIR)/minimize $(TMP_DIR)/replay_gcov tools/e2e_cleanup.sh tools/line_coverage.sh
	@rm -rf $(E2E_CLEANUP_DIR) && mkdir -p $(E2E_CLEANUP_DIR)
	@$(foreach e,$(E2E_EVENTS),$(LINE_COVERAGE_COVER) $(e) > $(E2E_CLEANUP_DIR)/$(notdir $(e)).lines && ) true
	@$(if $(E2E_EVENTS),sh tools/e2e_cleanup.sh $(foreach e,$(E2E_EVENTS),$(e) $$(./$(TMP_DIR)/minimize count $(e)) $(E2E_CLEANUP_DIR)/$(notdir $(e)).lines),:) > $(E2E_CLEANUP_DIR)/remove.txt
	@sed 's/^/to remove: /' $(E2E_CLEANUP_DIR)/remove.txt
	@echo "e2e-cleanup: $$(wc -l < $(E2E_CLEANUP_DIR)/remove.txt | tr -d ' ') file(s) to remove out of $(words $(E2E_EVENTS)), nothing is deleted"

e2e-judge: $(TMP_DIR)/replay_gcov tools/line_coverage.sh
	@$(LINE_COVERAGE_COVER) $(EVENTS) > $(TMP_DIR)/e2e_judge_self.txt
	@$(if $(filter-out $(EVENTS),$(E2E_EVENTS)),$(LINE_COVERAGE_COVER) $(filter-out $(EVENTS),$(E2E_EVENTS)),:) > $(TMP_DIR)/e2e_judge_others.txt
	@$(LINE_COVERAGE) contrib $(TMP_DIR)/e2e_judge_self.txt $(TMP_DIR)/e2e_judge_others.txt > $(TMP_DIR)/e2e_judge_contrib.txt
	@n=$$(wc -l < $(TMP_DIR)/e2e_judge_contrib.txt | tr -d ' '); \
	echo "e2e-judge: $(EVENTS) adds $$n line(s) to make e2e, in $(TMP_DIR)/e2e_judge_contrib.txt"; \
	[ "$$n" -gt 0 ]

.PHONY: minimize e2e-cleanup e2e-judge

# The categories of the counter that have fallen to 0, comma-separated.
DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch,by-name,list,ref-audit,value,type-tag

data-residue: tools/data_residue.sh
	@sh tools/data_residue.sh

# grep returns 1 when nothing matches; anything above is an error.
data-guard: tools/data_residue.sh
	@status=0; grep -rn offsetof src/ || status=$$?; \
	if [ $$status -eq 0 ]; then echo "data-guard: offsetof in src/"; exit 1; fi; \
	[ $$status -eq 1 ] || exit $$status; \
	echo "offsetof in src/: 0"
	@if [ -n "$(DATA_RESIDUE_GUARD)" ]; then \
	    sh tools/data_residue.sh --check $(DATA_RESIDUE_GUARD) src tests; \
	else \
	    echo "guarded categories: none"; \
	fi

.PHONY: data-residue data-guard

composition-residue: tools/composition_residue.sh
	@sh tools/composition_residue.sh

# The categories of the composition counter that have fallen to 0,
# comma-separated.
COMPOSITION_RESIDUE_GUARD := core-includes,one-or-false,name-dispatch,reachability-to-prove,nu,entity-ptrs,registries,defaults,header-accessors,parent-link,parent,void-ptr,mutual-inclusion,mutual-usage,void-self

composition-guard: tools/composition_residue.sh
	@sh tools/composition_residue.sh --check $(COMPOSITION_RESIDUE_GUARD) src tests

.PHONY: composition-residue composition-guard

$(TMP_DIR)/replay_sanitize: $(call sanitize_objs,$(REPLAY_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) $(UBSAN_CFLAGS) -o $@ $(filter %.o,$^)

SANITIZE_DIR := $(TMP_DIR)/sanitize
SANITIZE_RUNS := $(addprefix sanitize-run-,$(E2E_EVENTS))

$(SANITIZE_RUNS): sanitize-run-%: % $(TMP_DIR)/replay_sanitize
	@mkdir -p $(SANITIZE_DIR)/$(<D)
	@./$(TMP_DIR)/replay_sanitize $< /dev/null $(SANITIZE_DIR)/$<.dump.txt > $(SANITIZE_DIR)/$<.log 2>&1 || { cat $(SANITIZE_DIR)/$<.log; echo "sanitize: $< dirty"; exit 1; }

sanitize-net: $(SANITIZE_RUNS)
	@echo "sanitize: $(words $(SANITIZE_RUNS)) clean"

.PHONY: sanitize-net $(SANITIZE_RUNS)

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

psn00bsdk-toolchain-setup:
	sh tools/install_psn00bsdk_toolchain.sh

psx-hello:
	cmake -S psx-hello -B psx-hello/build && cmake --build psx-hello/build

psx-hello-clean:
	rm -rf psx-hello/build

# Clones a pinned raylib version into third_party/ (gitignored) and builds its
# static library. Slow, network, once, deliberately outside `make test`.
raylib-vendor-setup:
	sh tools/install_raylib_vendor.sh

# Link only: this target never runs the binary (no ./$(TMP_DIR)/... line), see
# CLAUDE.md. It needs third_party/raylib/src/libraylib.a, so `make raylib`
# without vendoring fails with "no rule to make target".
# The two raylib targets build straight from the sources to the executable:
# their files need `-I third_party/raylib/src`, which the others do not have,
# and platform/raylib_backend_coords.c is also linked without that flag by a
# test.
$(TMP_DIR)/game: src/cmd/game.c src/platform/raylib_backend.c src/platform/raylib_backend.h src/platform/raylib_backend_coords.c src/platform/raylib_backend_coords.h src/p8/palette.c src/p8/palette.h src/p8/assets.c src/p8/assets.h src/p8/audio.c src/game/background.c src/engine/bootstrap.c src/game/cart.c src/game/cart.h src/game/cheat.c src/game/cheat.h src/engine/collect.c src/game/data_init.c src/engine/dispatch_context.c src/engine/dispatch_context.h src/devtools/dump.c src/game/economy.c src/game/economy_formulas.c src/game/economy_score.c src/game/economy_tiers.c src/engine/entity_registry.c src/engine/entity_geom.c src/engine/exit_queue.c src/p8/fixed.c src/game/galaxy.c src/game/game_data.c src/game/gen.c src/engine/geom.c src/p8/gfx.c src/game/help.c src/game/help.h src/game/hud.c src/p8/input.c src/devtools/input_json.c src/engine/entity.c src/game/layout.c src/engine/entity_list.c src/engine/pointer_list.c src/game/map_tables.c src/p8/mem.c src/game/menu.c src/game/mouse_button.c src/game/mouse.c src/game/message_box.c src/game/particle.c src/game/draw_functions.c src/game/planet.c src/game/planet_display.c src/game/planet_family.c src/game/planet_family_trade.c src/game/planet_trade.c src/game/probe.c src/devtools/replay_control.c src/devtools/replay_control.h src/devtools/replay_source.c src/devtools/replay_source.h src/game/research.c src/game/research_display.c src/game/structure_icon.c src/game/score_panel.c src/game/score_panel_display.c src/game/scroll.c src/devtools/session_recorder.c src/devtools/session_recorder.h src/game/slipway.c src/game/slipway_display.c src/p8/str.c src/game/tech_gate.c src/game/trade.c src/game/ui.c src/game/ui_button.c src/game/ui_button.h src/engine/vec.c src/game/wormhole.c src/game/wormhole_display.c src/engine/zone.c src/engine/allocator.c src/engine/frame_zone.c src/game/ui_zone.c src/game/particle_zone.c src/engine/game_zone.c src/platform/fatal_pc.c third_party/raylib/src/libraylib.a | $(TMP_DIR)
	$(CC) $(CFLAGS) -I third_party/raylib/src -o $@ src/cmd/game.c src/platform/raylib_backend.c src/platform/raylib_backend_coords.c src/p8/palette.c src/p8/assets.c src/p8/audio.c src/game/background.c src/engine/bootstrap.c src/game/cart.c src/game/cheat.c src/engine/collect.c src/game/data_init.c src/engine/dispatch_context.c src/devtools/dump.c src/game/economy.c src/game/economy_formulas.c src/game/economy_score.c src/game/economy_tiers.c src/engine/entity_registry.c src/engine/entity_geom.c src/engine/exit_queue.c src/p8/fixed.c src/game/galaxy.c src/game/game_data.c src/game/gen.c src/engine/geom.c src/p8/gfx.c src/game/help.c src/game/hud.c src/p8/input.c src/devtools/input_json.c src/engine/entity.c src/game/layout.c src/engine/entity_list.c src/engine/pointer_list.c src/game/map_tables.c src/p8/mem.c src/game/menu.c src/game/mouse_button.c src/game/mouse.c src/game/message_box.c src/game/particle.c src/game/draw_functions.c src/game/planet.c src/game/planet_display.c src/game/planet_family.c src/game/planet_family_trade.c src/game/planet_trade.c src/game/probe.c src/devtools/replay_control.c src/devtools/replay_source.c src/game/research.c src/game/research_display.c src/game/structure_icon.c src/game/score_panel.c src/game/score_panel_display.c src/game/scroll.c src/devtools/session_recorder.c src/game/slipway.c src/game/slipway_display.c src/p8/str.c src/game/tech_gate.c src/game/trade.c src/game/ui.c src/game/ui_button.c src/engine/vec.c src/game/wormhole.c src/game/wormhole_display.c src/engine/zone.c src/engine/allocator.c src/engine/frame_zone.c src/game/ui_zone.c src/game/particle_zone.c src/engine/game_zone.c src/platform/fatal_pc.c third_party/raylib/src/libraylib.a $(RAYLIB_LDLIBS)

raylib: $(TMP_DIR)/game

# The viewer: the same rule as the raylib target above, link only.
$(SCREENCAST_VIEWER_BIN): src/cmd/screencast_viewer.c src/devtools/screencast.c src/devtools/screencast.h src/devtools/screencast_player.c src/devtools/screencast_player.h src/p8/palette.c src/p8/palette.h src/devtools/replay_control.c src/devtools/replay_control.h src/platform/raylib_backend.c src/platform/raylib_backend.h src/platform/raylib_backend_coords.c src/platform/raylib_backend_coords.h src/devtools/session_recorder.c src/devtools/session_recorder.h src/p8/input.c src/devtools/input_json.c src/p8/input.h src/p8/mem.c src/p8/mem.h src/p8/gfx.h third_party/raylib/src/libraylib.a | $(TMP_DIR)
	$(CC) $(CFLAGS) -I third_party/raylib/src -o $@ src/cmd/screencast_viewer.c src/devtools/screencast.c src/devtools/screencast_player.c src/p8/palette.c src/devtools/replay_control.c src/platform/raylib_backend.c src/platform/raylib_backend_coords.c src/devtools/session_recorder.c src/p8/input.c src/devtools/input_json.c src/p8/mem.c third_party/raylib/src/libraylib.a $(RAYLIB_LDLIBS)

screencast-viewer: $(SCREENCAST_VIEWER_BIN)

AST_RENAME_BINDINGS := $(CURDIR)/third_party/clang-python
LIBCLANG ?= /usr/lib/llvm-18/lib/libclang-18.so.1
AST_RENAME_ENV := PYTHONPATH=$(CURDIR)/tools:$(AST_RENAME_BINDINGS) LIBCLANG=$(LIBCLANG)
AST_RENAME_DIR := $(TMP_DIR)/ast_rename
RENAME_SUBSTEPS := tools/rename_substeps.tsv
# Pathspec ':!' : git's '*' crosses '/', so tests/fixtures/ would match
# `tests/*.c`.
AST_RENAME_UNITS = $(shell git ls-files 'src/*.c' 'tests/*.c' 'tools/*.c' ':!tests/fixtures/*')
AST_RENAME_ANALYSIS = --root . --exclude-dir third_party --cflags "$(CFLAGS)" \
    --config default= --config coverage="$(COVERAGE_CFLAGS)" --config asan="$(ASAN_CFLAGS)" \
    --include-on-demand raylib.h=third_party/raylib/src \
    --include-on-demand sanitizer/asan_interface.h=$(shell $(CC) -print-file-name=include) \
    $(AST_RENAME_UNITS)

rename-tool-setup:
	sh tools/install_ast_rename_bindings.sh

rename-tool-test:
	$(AST_RENAME_ENV) python3 tests/test_ast_rename.py

rename-extract:
	@mkdir -p $(AST_RENAME_DIR)
	$(AST_RENAME_ENV) python3 -m ast_rename extract -o $(AST_RENAME_DIR)/declarations.tsv \
	    --occurrences $(AST_RENAME_DIR)/occurrences.tsv $(AST_RENAME_ANALYSIS)

rename-derive-test:
	python3 tests/test_rename_derive.py

rename-derive:
	python3 tools/rename_derive.py --declarations $(AST_RENAME_DIR)/declarations.tsv \
	    --vocabulary docs/vocabulaire.md --overrides tools/rename_overrides.tsv \
	    --output docs/renommage.tsv --remainder $(AST_RENAME_DIR)/remainder.tsv

rename-strings-test:
	python3 tests/test_rename_strings.py

rename-strings:
	@mkdir -p $(AST_RENAME_DIR)
	python3 tools/rename_strings.py --root . --table docs/renommage.tsv \
	    --vocabulary docs/vocabulaire.md --overrides tools/rename_overrides.tsv \
	    --flags $(AST_RENAME_DIR)/string_flags.tsv

rename-check-test:
	python3 tests/test_rename_check.py

rename-table-check:
	python3 tools/rename_check.py --table docs/renommage.tsv \
	    --declarations $(AST_RENAME_DIR)/declarations.tsv \
	    --remainder $(AST_RENAME_DIR)/remainder.tsv --root .

rename-slice-test:
	python3 tests/test_rename_slice.py

rename-slice:
	@test -n "$(MODULES)$(SUBSTEP)" || { echo "usage: make rename-slice MODULES=\"<module>...\" | SUBSTEP=<3x>" >&2; exit 1; }
	@mkdir -p $(AST_RENAME_DIR)
	python3 tools/rename_slice.py --table docs/renommage.tsv $(foreach m,$(MODULES),--module '$(m)') \
	    $(if $(SUBSTEP),--substep $(SUBSTEP) --substeps $(RENAME_SUBSTEPS)) -o $(AST_RENAME_DIR)/slice.tsv

rename-count-test:
	python3 tests/test_rename_count.py

cart-names-count:
	@mkdir -p $(AST_RENAME_DIR)
	python3 tools/rename_count.py --table docs/renommage.tsv \
	    --occurrences $(AST_RENAME_DIR)/occurrences.tsv --root . \
	    $(foreach m,$(MODULES),--module '$(m)') \
	    $(if $(SUBSTEP),--substep $(SUBSTEP) --substeps $(RENAME_SUBSTEPS)) \
	    | tee $(AST_RENAME_DIR)/cart_names_count.txt

rename-substeps-test:
	python3 tests/test_rename_substeps.py

rename-substeps-check:
	python3 tools/rename_substeps.py --table docs/renommage.tsv --substeps $(RENAME_SUBSTEPS)

rename-rekey-test:
	python3 tests/test_rename_rekey.py

rename-rekey:
	python3 tools/rename_rekey.py --table docs/renommage.tsv \
	    --before $(or $(BEFORE),$(AST_RENAME_DIR)/declarations_before.tsv) \
	    --after $(AST_RENAME_DIR)/declarations.tsv --slice $(AST_RENAME_DIR)/slice.tsv

rename-rekey-check:
	@test -n "$(SUBSTEP)" || { echo "usage: make rename-rekey-check SUBSTEP=<3x>" >&2; exit 1; }
	python3 tools/rename_rekey.py --table docs/renommage.tsv --check \
	    --declarations $(AST_RENAME_DIR)/declarations.tsv \
	    --substeps $(RENAME_SUBSTEPS) --substep $(SUBSTEP)

rename-strings-verify-test:
	python3 tests/test_rename_strings_verify.py

rename-strings-verify:
	@test -n "$(SUBSTEP)" || { echo "usage: make rename-strings-verify SUBSTEP=<3x>" >&2; exit 1; }
	python3 tools/rename_strings_verify.py --table docs/renommage.tsv \
	    --substeps $(RENAME_SUBSTEPS) --substep $(SUBSTEP) --root .

scope-count-test:
	python3 tests/test_scope_count.py

scope-count:
	python3 tools/scope_count.py --declarations $(AST_RENAME_DIR)/declarations.tsv \
	    --exceptions tools/scope_exceptions.txt --root . $(foreach m,$(MODULES),--module '$(m)') \
	    $(if $(SUBSTEP),--substep $(SUBSTEP) --substeps $(RENAME_SUBSTEPS))

cryptic-locals:
	python3 tools/scope_count.py --declarations $(AST_RENAME_DIR)/declarations.tsv \
	    --exceptions tools/scope_exceptions.txt --root . --cryptic --table docs/renommage.tsv \
	    $(foreach m,$(MODULES),--module '$(m)') \
	    $(if $(SUBSTEP),--substep $(SUBSTEP) --substeps $(RENAME_SUBSTEPS))

cart-citations-test:
	python3 tests/test_cart_citations.py

cart-citations:
	python3 tools/cart_citations.py --table docs/renommage.tsv --root . \
	    $(foreach m,$(MODULES),--module '$(m)') \
	    $(if $(SUBSTEP),--substep $(SUBSTEP) --substeps $(RENAME_SUBSTEPS))

french-count-test:
	python3 tests/test_french_count.py

french-count:
	python3 tools/french_count.py --root . --exceptions tools/french_exceptions.txt \
	    --substeps $(RENAME_SUBSTEPS) $(foreach m,$(MODULES),--module '$(m)') \
	    $(if $(SUBSTEP),--substep $(SUBSTEP)) $(if $(SITES),--sites)

old-names-guard:
	python3 tools/rename_count.py --table docs/renommage.tsv --root . --part text --check

# Both counts run whatever the first returns; the status is the last failure.
ast-guard: rename-extract
	status=0; \
	python3 tools/rename_count.py --table docs/renommage.tsv \
	    --occurrences $(AST_RENAME_DIR)/occurrences.tsv --root . --part ast --check || status=$$?; \
	python3 tools/scope_count.py --declarations $(AST_RENAME_DIR)/declarations.tsv \
	    --exceptions tools/scope_exceptions.txt --root . --check || status=$$?; \
	exit $$status

rename-apply:
	@test -n "$(TABLE)" || { echo "usage: make rename-apply TABLE=<table>" >&2; exit 1; }
	@mkdir -p $(AST_RENAME_DIR)
	$(AST_RENAME_ENV) python3 -m ast_rename apply --table $(TABLE) \
	    --report $(AST_RENAME_DIR)/report.txt $(AST_RENAME_ANALYSIS)

# The dependency fragments written by the compiler, one per object. The
# wildcard tolerates their absence: on a fresh clone, everything is compiled.
-include $(wildcard $(OBJ_DIR)/src/*/*.d $(OBJ_DIR)/tests/*.d)
-include $(wildcard $(OBJ_ASAN_DIR)/src/*/*.d $(OBJ_ASAN_DIR)/tests/*.d)
-include $(wildcard $(OBJ_UBSAN_DIR)/src/*/*.d $(OBJ_UBSAN_DIR)/tests/*.d)
-include $(wildcard $(OBJ_COVERAGE_DIR)/src/*/*.d $(OBJ_COVERAGE_DIR)/tests/*.d)
-include $(wildcard $(OBJ_GCOV_DIR)/src/*/*.d $(OBJ_GCOV_DIR)/tests/*.d)
-include $(wildcard $(OBJ_SANITIZE_DIR)/src/*/*.d $(OBJ_SANITIZE_DIR)/tests/*.d)

