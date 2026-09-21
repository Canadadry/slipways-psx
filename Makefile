.PHONY: psn00bsdk-toolchain-setup psx-hello psx-hello-clean unit replay test raylib-vendor-setup raylib

# The Love2D prototype (`game/`) was removed in favor of the PC/raylib
# literal port described in docs/plan.md. Phase 1 of that plan added a C
# build + test setup here: `unit` runs the focused unit tests for each
# deep module (pico8_mem, asset loaders, pico8_gfx primitives, ...).
# `demo` (src/main.c, Chunk 9) was Phase 1's own end-to-end acceptance
# program, asserting specific framebuffer/peek values against a scripted
# events.json replay -- its assertions now live verbatim in
# tests/test_pico8_demo.c (part of `unit`), since main.c itself became
# Phase 4's real headless game driver (see below). `test` runs both, and
# was Phase 1's exit gate (see docs/prd/triage/phase1-etape1-pico8-
# interface-software-rendering.md, Implementation Chunks 1-9).
#
# Phase 4, Chunk 1 (docs/prd/triage/phase4-etape1-headless-replay-wiring-
# and-playability-verification.md): `main.c` is rewritten as the one real
# headless replay driver -- it boots the fully-assembled cart
# (pico8_cart_load/pico8__init/pico8_eu) and steps it frame-by-frame from
# a scripted events.json, dumping the final framebuffer as text. `replay`
# (formerly `demo`) builds and runs it as an integration/smoke check only
# ("the real game boots and runs to the end of a scripted replay without
# crashing") -- no gameplay-specific assertions belong there; those live
# in tests/test_pico8_playability.c (Chunk 2/3), a dedicated permanent
# test wired into `unit` once it exists. `pico8_cart_load()` pulls in
# essentially the whole engine, so `replay`'s own source list (like
# test_pico8_data_init/test_pico8_planet_kj_dispatch's before it) is the
# full engine closure, not a narrow module subset.

CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Werror -g

TMP_DIR := tmp

# Host OS, used only by the `raylib` target below to pick the right
# system libs to link raylib's own PLATFORM_DESKTOP build against --
# every other target here is plain portable C with no OS-specific
# linking at all.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    RAYLIB_LDLIBS := -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo
else
    RAYLIB_LDLIBS := -lm -lGL -lX11 -lpthread -ldl -lrt
endif

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

# Phase 5, Etape 1, Chunk 5 (docs/prd/triage/phase5-etape1-raylib-
# interactive-backend.md): the one meaningfully testable piece of the
# raylib backend, and deliberately raylib-free (see
# src/pico8_raylib_backend_coords.h's own header comment) -- links only
# against pico8_raylib_backend_coords.c, never raylib.h/libraylib.a, so
# wiring it into `unit`/`test` (per the PRD's own Testing Decisions,
# which call this "fine and even preferred") adds coverage without
# giving the fast, window-free day-to-day suite any raylib dependency.
$(TMP_DIR)/test_pico8_raylib_backend_coords: tests/test_pico8_raylib_backend_coords.c src/pico8_raylib_backend_coords.c src/pico8_raylib_backend_coords.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_raylib_backend_coords.c src/pico8_raylib_backend_coords.c

$(TMP_DIR)/test_pico8_vec: tests/test_pico8_vec.c src/pico8_vec.c src/pico8_vec.h src/pico8_fixed.c src/pico8_fixed.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_vec.c src/pico8_vec.c src/pico8_fixed.c -lm

$(TMP_DIR)/test_pico8_geom: tests/test_pico8_geom.c src/pico8_geom.c src/pico8_geom.h src/pico8_vec.c src/pico8_vec.h src/pico8_fixed.c src/pico8_fixed.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_geom.c src/pico8_geom.c src/pico8_vec.c src/pico8_fixed.c -lm

$(TMP_DIR)/test_pico8_value: tests/test_pico8_value.c src/pico8_value.c src/pico8_value.h src/pico8_fixed.c src/pico8_fixed.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_value.c src/pico8_value.c src/pico8_fixed.c -lm

$(TMP_DIR)/test_pico8_list: tests/test_pico8_list.c src/pico8_list.c src/pico8_list.h src/pico8_value.c src/pico8_value.h src/pico8_fixed.c src/pico8_fixed.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_list.c src/pico8_list.c src/pico8_value.c src/pico8_fixed.c -lm

$(TMP_DIR)/test_pico8_str: tests/test_pico8_str.c src/pico8_str.c src/pico8_str.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_str.c src/pico8_str.c

$(TMP_DIR)/test_pico8_ob: tests/test_pico8_ob.c src/pico8_ob.c src/pico8_ob.h src/pico8_list.c src/pico8_list.h src/pico8_value.c src/pico8_value.h src/pico8_str.c src/pico8_str.h src/pico8_fixed.c src/pico8_fixed.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_ob.c src/pico8_ob.c src/pico8_list.c src/pico8_value.c src/pico8_str.c src/pico8_fixed.c -lm

$(TMP_DIR)/test_pico8_entity_smoke: tests/test_pico8_entity_smoke.c src/pico8_entity.c src/pico8_entity.h src/pico8_class.c src/pico8_class.h src/pico8_ke.c src/pico8_ke.h src/pico8_ob.c src/pico8_ob.h src/pico8_list.c src/pico8_list.h src/pico8_value.c src/pico8_value.h src/pico8_str.c src/pico8_str.h src/pico8_fixed.c src/pico8_fixed.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_entity_smoke.c src/pico8_entity.c src/pico8_class.c src/pico8_ke.c src/pico8_ob.c src/pico8_list.c src/pico8_value.c src/pico8_str.c src/pico8_fixed.c -lm

$(TMP_DIR)/test_pico8_stash: tests/test_pico8_stash.c src/pico8_stash.c src/pico8_stash.h src/pico8_str.c src/pico8_str.h src/pico8_ob.c src/pico8_ob.h src/pico8_list.c src/pico8_list.h src/pico8_value.c src/pico8_value.h src/pico8_fixed.c src/pico8_fixed.h src/pico8_mem.c src/pico8_mem.h src/pico8_assets.c src/pico8_assets.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_stash.c src/pico8_stash.c src/pico8_str.c src/pico8_ob.c src/pico8_list.c src/pico8_value.c src/pico8_fixed.c src/pico8_mem.c src/pico8_assets.c -lm

# pico8_data_init() exercises pico8_research()/pico8_tech_gate(), which pull
# in pico8_probe()'s own placeholder `pico8_fm()` -- from there, this test's
# real dependency closure widens to the rest of the entity/UI/dispatch
# engine those modules call into (unlike every other test above, none of
# which is real "data decoding" work). Full non-main() source list below
# reflects that real closure rather than an approximate subset.
$(TMP_DIR)/test_pico8_data_init: tests/test_pico8_data_init.c src/pico8_data_init.c src/pico8_data_init.h src/pico8_assets.c src/pico8_assets.h src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_class.c src/pico8_collect.c src/pico8_dispatch_context.c src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_research.c src/pico8_research_display.c src/pico8_scroll.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_value.c src/pico8_vec.c | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_data_init.c src/pico8_data_init.c src/pico8_assets.c src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_class.c src/pico8_collect.c src/pico8_dispatch_context.c src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_research.c src/pico8_research_display.c src/pico8_scroll.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_value.c src/pico8_vec.c -lm

# Phase 3, Chunk 13, sub-lot 5: pico8_dc("kj")'s own first real, non-dead
# exercise (a real jo instance registered under the real "kj" bm bucket,
# dispatched against a real mo=qb() singleton) -- deliberately its own
# small permanent test/target rather than folded into test_pico8_entity_
# smoke (whose minimal dependency graph is a considered design choice,
# already reaffirmed once in this same chunk) or test_pico8_data_init
# (topically about data decoding, not entity dispatch) -- see that test
# file's own top comment for the full rationale. Shares the exact same
# real dependency closure test_pico8_data_init already needs, for the
# same reason (jo/planet pulls in the whole entity/economy/research/UI/
# dispatch engine).
$(TMP_DIR)/test_pico8_planet_kj_dispatch: tests/test_pico8_planet_kj_dispatch.c src/pico8_data_init.c src/pico8_data_init.h src/pico8_assets.c src/pico8_assets.h src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_class.c src/pico8_collect.c src/pico8_dispatch_context.c src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_research.c src/pico8_research_display.c src/pico8_scroll.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_value.c src/pico8_vec.c | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_planet_kj_dispatch.c src/pico8_data_init.c src/pico8_assets.c src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_class.c src/pico8_collect.c src/pico8_dispatch_context.c src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_research.c src/pico8_research_display.c src/pico8_scroll.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_value.c src/pico8_vec.c -lm

$(TMP_DIR)/test_pico8_demo: tests/test_pico8_demo.c src/pico8_mem.c src/pico8_mem.h src/pico8_assets.c src/pico8_assets.h src/pico8_gfx.c src/pico8_gfx.h src/pico8_input.c src/pico8_input.h src/pico8_dump.c src/pico8_dump.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_demo.c src/pico8_mem.c src/pico8_assets.c src/pico8_gfx.c src/pico8_input.c src/pico8_dump.c

# Chunk 1's own "full engine source list": pico8_cart_load() bootstraps
# essentially every class/data-table this whole port has built, so
# main.c's own real dependency closure is literally every module in
# src/ except itself -- the same reasoning test_pico8_data_init/
# test_pico8_planet_kj_dispatch's own header comments already establish
# for reaching real game state, now widened to the WHOLE engine since
# main.c reaches through pico8_cart_load() rather than one data module.
$(TMP_DIR)/pico8_replay: src/main.c src/pico8_assets.c src/pico8_assets.h src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_cart.c src/pico8_cart.h src/pico8_class.c src/pico8_collect.c src/pico8_data_init.c src/pico8_dispatch_context.c src/pico8_dispatch_context.h src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_galaxy.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_help.c src/pico8_help.h src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_menu.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_family.c src/pico8_planet_family_trade.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_research.c src/pico8_research_display.c src/pico8_research_icon.c src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_scroll.c src/pico8_slipway.c src/pico8_slipway_display.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_ui_button.h src/pico8_value.c src/pico8_vec.c src/pico8_wormhole.c src/pico8_wormhole_display.c | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ src/main.c src/pico8_assets.c src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_cart.c src/pico8_class.c src/pico8_collect.c src/pico8_data_init.c src/pico8_dispatch_context.c src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_galaxy.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_help.c src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_menu.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_family.c src/pico8_planet_family_trade.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_research.c src/pico8_research_display.c src/pico8_research_icon.c src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_scroll.c src/pico8_slipway.c src/pico8_slipway_display.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_value.c src/pico8_vec.c src/pico8_wormhole.c src/pico8_wormhole_display.c -lm

# Phase 4, Chunk 2 (docs/prd/triage/phase4-etape1-headless-replay-wiring-
# and-playability-verification.md): the exit-gate exploratory playthrough
# -- see tests/test_pico8_playability.c's own top comment for the full
# trace. Shares pico8_replay's own full engine source list (minus
# src/main.c, plus the test file and its own fixture) for the same
# "reaching real game state pulls in essentially everything" reasoning.
# Wired into `unit` (not just built standalone): this program runs
# cleanly and deterministically every time it has been run while building
# this chunk (this port's own pico8_rnd is a fixed-seed generator, see
# pico8_fixed.c), so per this PRD's own "if you get it working reliably,
# wiring it in now is fine" allowance, it is a permanent part of
# `make test` -- printed checkpoints only, no assert()-based invariants
# yet (Chunk 3's own job).

# Phase 5 bug fix ("in screen game top menu is weird": missing icon
# outlines, blank technology counter, month drawn as placeholder blocks)
# -- see tests/test_pico8_top_menubar.c's own top comment for the three
# symptoms and the pixel each assertion pins. Reaches them the only way a
# human can, through a really booted cart (pico8_cart_load/pico8__init/
# pico8__update60/pico8__draw), so it shares test_pico8_playability's own
# full-engine source list verbatim, with its own inert 4-frame fixture.
$(TMP_DIR)/test_pico8_top_menubar: tests/test_pico8_top_menubar.c tests/fixtures/top_menubar_events.json src/pico8_assets.c src/pico8_assets.h src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_cart.c src/pico8_cart.h src/pico8_class.c src/pico8_collect.c src/pico8_data_init.c src/pico8_dispatch_context.c src/pico8_dispatch_context.h src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_galaxy.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_help.c src/pico8_help.h src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_menu.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_family.c src/pico8_planet_family_trade.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_research.c src/pico8_research_display.c src/pico8_research_icon.c src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_scroll.c src/pico8_slipway.c src/pico8_slipway_display.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_ui_button.h src/pico8_value.c src/pico8_vec.c src/pico8_wormhole.c src/pico8_wormhole_display.c | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_top_menubar.c src/pico8_assets.c src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_cart.c src/pico8_class.c src/pico8_collect.c src/pico8_data_init.c src/pico8_dispatch_context.c src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_galaxy.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_help.c src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_menu.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_family.c src/pico8_planet_family_trade.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_research.c src/pico8_research_display.c src/pico8_research_icon.c src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_scroll.c src/pico8_slipway.c src/pico8_slipway_display.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_value.c src/pico8_vec.c src/pico8_wormhole.c src/pico8_wormhole_display.c -lm

# Phase 5 bug fix (map() drew sprite 0 instead of treating it as
# "empty"): the galaxy sky's only map() call site is bg:kj(), so this
# test reaches the primitive the way a player does -- through
# pico8_bg_kj() with a realistic sky in the map. pico8_bg.c's own
# dependency closure is the same full-engine one test_pico8_data_init
# already links (bg -> ke/entity/dispatch), so its source list is
# reused verbatim here rather than re-derived.
$(TMP_DIR)/test_pico8_bg_kj: tests/test_pico8_bg_kj.c src/pico8_data_init.c src/pico8_data_init.h src/pico8_bg.h src/pico8_assets.c src/pico8_assets.h src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_class.c src/pico8_collect.c src/pico8_dispatch_context.c src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_research.c src/pico8_research_display.c src/pico8_scroll.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_value.c src/pico8_vec.c | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_bg_kj.c src/pico8_data_init.c src/pico8_assets.c src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_class.c src/pico8_collect.c src/pico8_dispatch_context.c src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_research.c src/pico8_research_display.c src/pico8_scroll.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_value.c src/pico8_vec.c -lm

# Phase 5 bug fix (releasing a slipway drag on a valid target planet did
# nothing -- ui:bh passed nil instead of the drop target to jc:ey): the
# regression test for it, tests/test_pico8_slipway_drag.c. Reaches the bug
# the way a human does -- scripted mouse input only, through the real
# pico8__update60() dispatch chain, booting from a real click on the title
# screen's difficulty button (src/main_raylib.c's boot, not src/main.c's
# pico8_eu shortcut) -- so it shares pico8_replay/test_pico8_playability's
# own full engine source list, for the same "reaching real game state pulls
# in essentially the whole engine" reasoning their own header comments
# already establish.
$(TMP_DIR)/test_pico8_slipway_drag: tests/test_pico8_slipway_drag.c tests/fixtures/slipway_drag_events.json src/pico8_assets.c src/pico8_assets.h src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_cart.c src/pico8_cart.h src/pico8_class.c src/pico8_collect.c src/pico8_data_init.c src/pico8_dispatch_context.c src/pico8_dispatch_context.h src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_galaxy.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_help.c src/pico8_help.h src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_menu.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_family.c src/pico8_planet_family_trade.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_research.c src/pico8_research_display.c src/pico8_research_icon.c src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_scroll.c src/pico8_slipway.c src/pico8_slipway_display.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_ui_button.h src/pico8_value.c src/pico8_vec.c src/pico8_wormhole.c src/pico8_wormhole_display.c | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_pico8_slipway_drag.c src/pico8_assets.c src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_cart.c src/pico8_class.c src/pico8_collect.c src/pico8_data_init.c src/pico8_dispatch_context.c src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_galaxy.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_help.c src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_menu.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_family.c src/pico8_planet_family_trade.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_research.c src/pico8_research_display.c src/pico8_research_icon.c src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_scroll.c src/pico8_slipway.c src/pico8_slipway_display.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_value.c src/pico8_vec.c src/pico8_wormhole.c src/pico8_wormhole_display.c -lm

unit: $(TMP_DIR)/test_pico8_mem $(TMP_DIR)/test_asset_loading $(TMP_DIR)/test_pico8_gfx $(TMP_DIR)/test_pico8_input $(TMP_DIR)/test_pico8_dump $(TMP_DIR)/test_pico8_fixed $(TMP_DIR)/test_pico8_fixed_ubsan $(TMP_DIR)/test_pico8_vec $(TMP_DIR)/test_pico8_geom $(TMP_DIR)/test_pico8_value $(TMP_DIR)/test_pico8_list $(TMP_DIR)/test_pico8_str $(TMP_DIR)/test_pico8_ob $(TMP_DIR)/test_pico8_entity_smoke $(TMP_DIR)/test_pico8_stash $(TMP_DIR)/test_pico8_data_init $(TMP_DIR)/test_pico8_planet_kj_dispatch $(TMP_DIR)/test_pico8_demo $(TMP_DIR)/test_pico8_playability $(TMP_DIR)/test_pico8_slipway_drag $(TMP_DIR)/test_pico8_raylib_backend_coords $(TMP_DIR)/test_pico8_bg_kj $(TMP_DIR)/test_pico8_top_menubar
	./$(TMP_DIR)/test_pico8_mem
	./$(TMP_DIR)/test_asset_loading
	./$(TMP_DIR)/test_pico8_gfx
	./$(TMP_DIR)/test_pico8_input
	./$(TMP_DIR)/test_pico8_dump
	./$(TMP_DIR)/test_pico8_fixed
	./$(TMP_DIR)/test_pico8_fixed_ubsan
	./$(TMP_DIR)/test_pico8_vec
	./$(TMP_DIR)/test_pico8_geom
	./$(TMP_DIR)/test_pico8_value
	./$(TMP_DIR)/test_pico8_list
	./$(TMP_DIR)/test_pico8_str
	./$(TMP_DIR)/test_pico8_ob
	./$(TMP_DIR)/test_pico8_entity_smoke
	./$(TMP_DIR)/test_pico8_stash
	./$(TMP_DIR)/test_pico8_data_init
	./$(TMP_DIR)/test_pico8_planet_kj_dispatch
	./$(TMP_DIR)/test_pico8_demo
	./$(TMP_DIR)/test_pico8_playability
	./$(TMP_DIR)/test_pico8_slipway_drag
	./$(TMP_DIR)/test_pico8_raylib_backend_coords
	./$(TMP_DIR)/test_pico8_bg_kj
	./$(TMP_DIR)/test_pico8_top_menubar

replay: $(TMP_DIR)/pico8_replay
	./$(TMP_DIR)/pico8_replay

test: unit replay

psn00bsdk-toolchain-setup:
	sh tools/install_psn00bsdk_toolchain.sh

psx-hello:
	cmake -S psx-hello -B psx-hello/build && cmake --build psx-hello/build

psx-hello-clean:
	rm -rf psx-hello/build

# Phase 5, Etape 1, Chunk 1 (docs/prd/triage/phase5-etape1-raylib-
# interactive-backend.md): raylib is not available as a system package in
# this project's build/verification environment, so it is vendored and
# built from source, following the exact precedent already established
# above for PSn00bSDK -- clone a pinned release tag (never a floating
# branch) into third_party/raylib/ (gitignored, like tmp/) and build
# raylib's own static library with raylib's own build system
# (third_party/raylib/src/Makefile, PLATFORM_DESKTOP). Slow-ish and
# network-heavy (a shallow clone plus a from-source library build), a
# one-time setup step, and deliberately kept out of `make`/`make test` --
# no project C code depends on this yet (see the PRD's Chunk 1 for why).
raylib-vendor-setup:
	sh tools/install_raylib_vendor.sh

# Phase 5, Etape 1, Chunk 5 (docs/prd/triage/phase5-etape1-raylib-
# interactive-backend.md), the final chunk of that PRD: compiles and
# links src/main_raylib.c + src/pico8_raylib_backend.c +
# src/pico8_raylib_backend_coords.c against the SAME full engine closure
# pico8_replay already links (main_raylib.c boots the identical cart via
# pico8_cart_load(), same reasoning as that target's own header comment)
# and against the vendored third_party/raylib/src/libraylib.a, plus the
# system libs raylib's own PLATFORM_DESKTOP build needs -- $(RAYLIB_LDLIBS)
# above, `-lm -lGL -lX11 -lpthread -ldl -lrt` on Linux (the set already
# confirmed to link successfully while building Chunks 3 and 4) or the
# OpenGL/Cocoa/IOKit/CoreAudio/CoreVideo frameworks on macOS (per raylib's
# own src/Makefile, which lists that exact framework set for linking an
# external PLATFORM_DESKTOP program against libraylib.a on OSX -- no
# system package install needed there, just Xcode Command Line Tools).
# `-I third_party/raylib/src` is needed on top of $(CFLAGS) so raylib.h
# is found.
#
# This is a LINK-ONLY check and the `raylib` target below deliberately
# never runs the binary it builds: per this project's CLAUDE.md, no
# raylib window may be opened by an agent at any point before Phase 5's
# real human-playthrough exit gate, and the PRD's own Solution/
# Implementation Decisions sections are explicit that "a Makefile target
# that compiles and links main_raylib.c against the vendored raylib
# without ever running the resulting binary... [is] the actual,
# automatable exit criterion for everything this PRD's own chunks can
# complete" before ever calling on a human to play it. Unlike `replay:`,
# `raylib:` has no `./$(TMP_DIR)/...` execution line.
#
# This target requires third_party/raylib/src/libraylib.a to already
# exist (built by the separate, deliberately manual `raylib-vendor-
# setup` above) -- it is a normal prerequisite, not an order-only one
# and not auto-triggered, so running `make raylib` before vendoring
# raylib fails with an ordinary Make "no rule to make target" error,
# matching how `psx-hello` is kept fully separate from
# `psn00bsdk-toolchain-setup`. `raylib` is deliberately left out of
# `unit`/`test` for the same reason `psx-hello` is: it depends on a
# separate, slow/network-heavy one-time setup step no CI/day-to-day run
# should require.
$(TMP_DIR)/main_raylib: src/main_raylib.c src/pico8_raylib_backend.c src/pico8_raylib_backend.h src/pico8_raylib_backend_coords.c src/pico8_raylib_backend_coords.h src/pico8_assets.c src/pico8_assets.h src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_cart.c src/pico8_cart.h src/pico8_class.c src/pico8_collect.c src/pico8_data_init.c src/pico8_dispatch_context.c src/pico8_dispatch_context.h src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_galaxy.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_help.c src/pico8_help.h src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_menu.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_family.c src/pico8_planet_family_trade.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_research.c src/pico8_research_display.c src/pico8_research_icon.c src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_scroll.c src/pico8_slipway.c src/pico8_slipway_display.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_ui_button.h src/pico8_value.c src/pico8_vec.c src/pico8_wormhole.c src/pico8_wormhole_display.c third_party/raylib/src/libraylib.a | $(TMP_DIR)
	$(CC) $(CFLAGS) -I third_party/raylib/src -o $@ src/main_raylib.c src/pico8_raylib_backend.c src/pico8_raylib_backend_coords.c src/pico8_assets.c src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_cart.c src/pico8_class.c src/pico8_collect.c src/pico8_data_init.c src/pico8_dispatch_context.c src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_galaxy.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_help.c src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_menu.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_family.c src/pico8_planet_family_trade.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_research.c src/pico8_research_display.c src/pico8_research_icon.c src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_scroll.c src/pico8_slipway.c src/pico8_slipway_display.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_value.c src/pico8_vec.c src/pico8_wormhole.c src/pico8_wormhole_display.c third_party/raylib/src/libraylib.a $(RAYLIB_LDLIBS)

raylib: $(TMP_DIR)/main_raylib
