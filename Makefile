.PHONY: psn00bsdk-toolchain-setup psx-hello psx-hello-clean unit replay test raylib-vendor-setup raylib screencast screencast-check screencast-sign screencast-viewer

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

# docs/prd/triage/makefile-header-prerequisites.md: intermediate objects
# and compiler-generated header dependencies.
#
# Until that PRD, every target below went from its .c sources straight to
# an executable in ONE $(CC) invocation, and named its headers by hand --
# unevenly: 38 of the 66 headers in src/ were named by no target at all.
# Editing a header therefore did not rebuild the binaries that include
# it. Make saw the targets as up to date and `make test` went green
# against objects compiled from a source that no longer existed. That was
# survivable as long as headers held only declarations, since a changed
# signature fails loudly at link time; src/pico8_type_tag.h ended it,
# because that header compiles SEMANTICS into every object -- magic
# constants and a `static inline` reader. Change a constant without a
# rebuild and one translation unit tests the old value while the next
# tests the new one: exactly the type confusion that header exists to
# eliminate, reintroduced by the build system and invisible in the
# source. Measured on the pre-PRD Makefile: editing PICO8_TYPE_TAG_LIST
# rebuilt 1 of the 30 binaries `unit` links, and appending to a purely
# declarative header (src/pico8_planet.h) rebuilt 0 of 30.
#
# The minimal variant -- naming the semantic headers as prerequisites of
# the targets that include them -- was costed and rejected (the PRD asks
# for it to be costed, not skipped): it is about a dozen edited lines,
# but it is the same hand-maintained list that has already drifted once,
# it has to be redone by hand every time an #include is added, and it
# gets the transitive cases wrong exactly where it matters (pico8_type_
# tag.h is reached through pico8_list.h/pico8_ob.h/pico8_class.h, not
# directly, in every target but one).
#
# What is here instead: each .c is compiled to an object with $(DEPFLAGS)
# = -MMD -MP, which makes the compiler write, next to that object, a make
# fragment naming every header that translation unit really opened,
# transitively. The `-include` at the bottom of this file pulls those
# fragments back in. The prerequisite list stops being a declaration of
# intent and becomes a measurement, produced by the same compiler that
# reads the #includes, so it cannot drift from the code and nobody has to
# think about it when adding a target. -MP additionally emits a phony
# target for each header, so deleting or renaming a header does not wedge
# the build with a "No rule to make target" out of a stale fragment.
#
# The `-include $(wildcard ...)` form is deliberate: it tolerates the
# fragments not existing at all, so a fresh clone with no tmp/ builds in
# one step -- it simply compiles everything and writes the fragments as
# it goes.
#
# Objects and fragments live under tmp/, already used by the test
# binaries and already gitignored. The secondary benefit is speed: the
# seven targets sharing $(ENGINE_SRCS) used to recompile the whole engine
# each, ~7x over; they now share one set of objects and only link
# separately.
#
# WHY THERE ARE FOUR OBJECT TREES, AND WHY THEY MUST NOT BE MERGED.
# Three targets (families of targets) are built with their own
# instrumentation:
# test_pico8_planet_gh_row_lq under AddressSanitizer and
# test_pico8_fixed_ubsan under UndefinedBehaviorSanitizer (each target's
# own comment says why). A sanitizer is a property of every object in the
# binary, not of the link: an object compiled without -fsanitize=address
# gets no redzones and no shadow-memory bookkeeping, so an overflow
# inside IT is simply not seen. Sharing one object tree between a plain
# target and an instrumented one would therefore link, run and pass while
# the binary was blind over most of its own code -- a silent test whose
# silence proves nothing, with nothing in the build output to say so, and
# the ASan target in particular has no plain twin whose crash would give
# the game away. Hence one output tree per flag set, keyed by the flags
# themselves. Do not "simplify" the four into one, and do not add a
# target carrying its own compiler flags without giving it its own tree
# (and its own $(call ...) helper) here.
#
# The FOURTH tree, tmp/obj-refaudit, is the same rule applied to a
# non-sanitizer instrumentation: -DPICO8_REF_AUDIT turns
# src/pico8_ref_audit.h's PICO8_REF_AS_LIST macros from a plain cast into
# a per-site tag recorder (see that header, and docs/audit-ref-cast-
# reverse.md). It obeys the same "never share an object with an
# uninstrumented target" rule and for the same reason: the audit's whole
# claim is that a site it reports as NOT COVERED really was never
# reached, and an uninstrumented object silently linked into the audit
# binary would manufacture exactly that false answer for every site in
# it -- the same "silence that proves nothing" this comment already
# warns about for ASan, except here the silence IS the result.
OBJ_DIR := $(TMP_DIR)/obj
OBJ_ASAN_DIR := $(TMP_DIR)/obj-asan
OBJ_UBSAN_DIR := $(TMP_DIR)/obj-ubsan
OBJ_REFAUDIT_DIR := $(TMP_DIR)/obj-refaudit

DEPFLAGS := -MMD -MP
ASAN_CFLAGS := -fsanitize=address -fno-omit-frame-pointer
UBSAN_CFLAGS := -fsanitize=undefined -fno-sanitize-recover=undefined
REFAUDIT_CFLAGS := -DPICO8_REF_AUDIT

# $(call objs,<.c list>) maps a source list to its object paths in the
# plain tree; asan_objs/ubsan_objs/refaudit_objs do the same in the
# instrumented trees.
# A target names its sources ONCE, as a .c list, and both its prerequisite
# list and its link line come from that. The link lines use
# $(filter %.o,$^) so that non-source prerequisites -- the events.json
# fixtures some tests read at run time, declared as prerequisites since
# Phase 4 -- stay ordinary prerequisites without landing on the command
# line. Headers appear in no prerequisite list at all any more: that is
# the point, they come from the dependency fragments.
objs = $(patsubst %.c,$(OBJ_DIR)/%.o,$(1))
asan_objs = $(patsubst %.c,$(OBJ_ASAN_DIR)/%.o,$(1))
ubsan_objs = $(patsubst %.c,$(OBJ_UBSAN_DIR)/%.o,$(1))
refaudit_objs = $(patsubst %.c,$(OBJ_REFAUDIT_DIR)/%.o,$(1))

# One pattern rule per tree; the object tree mirrors src/ and tests/ under
# each root, so tmp/obj/src/pico8_mem.o next to tmp/obj-asan/src/
# pico8_mem.o. `mkdir -p $(@D)` in the recipe rather than an order-only
# directory prerequisite: a directory's mtime changes every time a file is
# written into it, and as an ordinary prerequisite that would rebuild
# everything on every run.
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_ASAN_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_UBSAN_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(UBSAN_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_REFAUDIT_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(REFAUDIT_CFLAGS) $(DEPFLAGS) -c -o $@ $<

# The two shared dependency closures, previously spelled out in full on
# every target that needed them (and drifting apart was only a matter of
# time). $(ENGINE_SRCS) is the whole-engine closure pico8_replay's own
# comment below describes -- every module in src/ except the two entry
# points (main.c, main_raylib.c) and the five raylib-facing/replay-driver
# modules only the interactive driver links; seven targets use it
# verbatim. $(DATA_ENGINE_SRCS) is the narrower
# "pico8_data_init() reaches real game state" closure test_pico8_data_
# init's own comment describes, shared verbatim by three targets. Both
# lists are reproduced here exactly as those targets spelled them, in the
# same order; nothing was added or removed while moving them.
ENGINE_SRCS := \
    src/pico8_assets.c \
    src/pico8_audio.c \
    src/pico8_bg.c \
    src/pico8_bootstrap.c \
    src/pico8_cart.c \
    src/pico8_class.c \
    src/pico8_collect.c \
    src/pico8_data_init.c \
    src/pico8_dispatch_context.c \
    src/pico8_dump.c \
    src/pico8_economy.c \
    src/pico8_economy_formulas.c \
    src/pico8_economy_score.c \
    src/pico8_economy_tiers.c \
    src/pico8_entity.c \
    src/pico8_entity_geom.c \
    src/pico8_fixed.c \
    src/pico8_galaxy.c \
    src/pico8_game_data.c \
    src/pico8_gen.c \
    src/pico8_geom.c \
    src/pico8_gfx.c \
    src/pico8_help.c \
    src/pico8_hud.c \
    src/pico8_input.c \
    src/pico8_ke.c \
    src/pico8_layout.c \
    src/pico8_list.c \
    src/pico8_mem.c \
    src/pico8_menu.c \
    src/pico8_mouse.c \
    src/pico8_mouse_reader.c \
    src/pico8_msgbox.c \
    src/pico8_ob.c \
    src/pico8_particle.c \
    src/pico8_pi.c \
    src/pico8_planet.c \
    src/pico8_planet_display.c \
    src/pico8_planet_family.c \
    src/pico8_planet_family_trade.c \
    src/pico8_planet_trade.c \
    src/pico8_probe.c \
    src/pico8_research.c \
    src/pico8_research_display.c \
    src/pico8_research_icon.c \
    src/pico8_score_panel.c \
    src/pico8_score_panel_display.c \
    src/pico8_scroll.c \
    src/pico8_slipway.c \
    src/pico8_slipway_display.c \
    src/pico8_stash.c \
    src/pico8_str.c \
    src/pico8_tech_gate.c \
    src/pico8_trade.c \
    src/pico8_ui.c \
    src/pico8_ui_button.c \
    src/pico8_value.c \
    src/pico8_vec.c \
    src/pico8_wormhole.c \
    src/pico8_wormhole_display.c

DATA_ENGINE_SRCS := \
    src/pico8_data_init.c \
    src/pico8_assets.c \
    src/pico8_audio.c \
    src/pico8_bg.c \
    src/pico8_bootstrap.c \
    src/pico8_class.c \
    src/pico8_collect.c \
    src/pico8_dispatch_context.c \
    src/pico8_dump.c \
    src/pico8_economy.c \
    src/pico8_economy_formulas.c \
    src/pico8_economy_score.c \
    src/pico8_economy_tiers.c \
    src/pico8_entity.c \
    src/pico8_entity_geom.c \
    src/pico8_fixed.c \
    src/pico8_game_data.c \
    src/pico8_gen.c \
    src/pico8_geom.c \
    src/pico8_gfx.c \
    src/pico8_hud.c \
    src/pico8_input.c \
    src/pico8_ke.c \
    src/pico8_layout.c \
    src/pico8_list.c \
    src/pico8_mem.c \
    src/pico8_mouse.c \
    src/pico8_mouse_reader.c \
    src/pico8_msgbox.c \
    src/pico8_ob.c \
    src/pico8_particle.c \
    src/pico8_pi.c \
    src/pico8_planet.c \
    src/pico8_planet_display.c \
    src/pico8_planet_trade.c \
    src/pico8_probe.c \
    src/pico8_research.c \
    src/pico8_research_display.c \
    src/pico8_scroll.c \
    src/pico8_stash.c \
    src/pico8_str.c \
    src/pico8_tech_gate.c \
    src/pico8_trade.c \
    src/pico8_ui.c \
    src/pico8_ui_button.c \
    src/pico8_value.c \
    src/pico8_vec.c

# Host OS, used by exactly two things: the `raylib` target below, to
# pick the right system libs to link raylib's own PLATFORM_DESKTOP build
# against, and -- depuis le chunk 3 de docs/prd/triage/screencast-
# capture-and-viewer.md -- la section SCREENCAST tout en bas, qui choisit
# l'outil de calcul du sha256 (`sha256sum` n'existe pas sur macOS).
# Every other target here is plain portable C with no OS-specific
# linking at all.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    RAYLIB_LDLIBS := -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo
else
    RAYLIB_LDLIBS := -lm -lGL -lX11 -lpthread -ldl -lrt
endif

$(TMP_DIR):
	mkdir -p $(TMP_DIR)

$(TMP_DIR)/test_pico8_mem: $(call objs,tests/test_pico8_mem.c src/pico8_mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_asset_loading: $(call objs,tests/test_asset_loading.c src/pico8_assets.c src/pico8_mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_gfx: $(call objs,tests/test_pico8_gfx.c src/pico8_gfx.c src/pico8_mem.c src/pico8_assets.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_input: $(call objs,tests/test_pico8_input.c src/pico8_input.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_dump: $(call objs,tests/test_pico8_dump.c src/pico8_dump.c src/pico8_gfx.c src/pico8_mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_fixed: $(call objs,tests/test_pico8_fixed.c src/pico8_fixed.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# UBSan-instrumented build of the same fixed-point test binary, scoped to
# this module only: pico8_fixed does bit-level signed/unsigned
# reinterpretation tricks (see pico8_fixed_add/sub/neg/div) where a
# regression back to plain signed-overflow/shift UB would produce correct
# numeric results on today's x86 codegen while still being undefined
# behavior. -fno-sanitize-recover=undefined makes any UBSan report a hard
# failure (non-zero exit) instead of a printed warning that's easy to miss.
$(TMP_DIR)/test_pico8_fixed_ubsan: $(call ubsan_objs,tests/test_pico8_fixed.c src/pico8_fixed.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(UBSAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

# Phase 5, Etape 1, Chunk 5 (docs/prd/triage/phase5-etape1-raylib-
# interactive-backend.md): the one meaningfully testable piece of the
# raylib backend, and deliberately raylib-free (see
# src/pico8_raylib_backend_coords.h's own header comment) -- links only
# against pico8_raylib_backend_coords.c, never raylib.h/libraylib.a, so
# wiring it into `unit`/`test` (per the PRD's own Testing Decisions,
# which call this "fine and even preferred") adds coverage without
# giving the fast, window-free day-to-day suite any raylib dependency.
$(TMP_DIR)/test_pico8_raylib_backend_coords: $(call objs,tests/test_pico8_raylib_backend_coords.c src/pico8_raylib_backend_coords.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Chunk 1 of docs/prd/triage/raylib-replay-then-resume.md: the replay
# source is deliberately raylib-free (see src/pico8_replay_source.h's own
# header comment) for exactly the reason the coords target above is --
# the interactive driver cannot be launched here and raylib is not
# vendored everywhere -- so its test links only pico8_replay_source.c and
# pico8_input.c (whose loader it delegates to) and belongs in the plain
# window-free `unit`/`test` suite. The fixtures are prerequisites because
# the test reads them at run time, as the other fixture-driven targets
# below already declare.
$(TMP_DIR)/test_pico8_replay_source: $(call objs,tests/test_pico8_replay_source.c src/pico8_replay_source.c src/pico8_input.c) tests/fixtures/events_valid.json tests/fixtures/events_malformed_missing_field.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Chunk 3 of docs/prd/triage/raylib-replay-then-resume.md: the end-to-end
# proof that a resumed session keeps recording -- the replayed frames
# then the new ones, in one file that reloads and replays again. It links
# the real pico8_replay_source and the real pico8_session_recorder (the
# recording bookkeeping that used to live inside the raylib backend and
# was moved out for exactly this reason -- see src/pico8_session_
# recorder.h), plus pico8_input.c for the writer and the loader; still no
# raylib and no window, so it belongs in the plain `unit`/`test` suite.
# No fixture prerequisites, unlike the target above: the file this test
# asserts on has to be one a resumed session actually produced, so it
# writes its own recordings under tmp/ and removes them again.
$(TMP_DIR)/test_pico8_recording_continuity: $(call objs,tests/test_pico8_recording_continuity.c src/pico8_session_recorder.c src/pico8_replay_source.c src/pico8_input.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Chunk 4 of docs/prd/triage/raylib-replay-then-resume.md: the replay's
# speed ladder and its progress line, extracted from the raylib backend
# for the same reason the three targets above were -- the interactive
# driver cannot be launched here and raylib is not vendored everywhere,
# so anything including raylib.h is untestable by construction. Links
# pico8_replay_control.c alone: this module depends on nothing, not even
# pico8_input, which is what lets it pin exact frame-rate caps and exact
# overlay strings in a suite with no window in it.
$(TMP_DIR)/test_pico8_replay_control: $(call objs,tests/test_pico8_replay_control.c src/pico8_replay_control.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Chunk 4 of the same PRD, user story 14: the frame index a space-bar
# marker receives still names the right frame after a session has been
# resumed from a recording. The property is a joint one -- the marker
# index is the session recorder's frame count, and that count is only
# right because the loop records replayed frames too -- so the test
# drives the real pico8_replay_source and the real
# pico8_session_recorder together, exactly as src/main_raylib.c's loop
# does, and resolves each index against the recording actually written
# (hence pico8_input.c for the writer and the loader). Same shape and
# same reasoning as test_pico8_recording_continuity above, and likewise
# writes its own files under tmp/ and removes them again.
$(TMP_DIR)/test_pico8_marker_coherence: $(call objs,tests/test_pico8_marker_coherence.c src/pico8_session_recorder.c src/pico8_replay_source.c src/pico8_input.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Chunk 1 de docs/prd/triage/screencast-capture-and-viewer.md : le
# module de screencast et son format. Il est deliberement sans raylib
# (voir src/pico8_screencast.h), pour la raison que les quatre cibles
# ci-dessus invoquent deja -- le visualiseur du chunk 4 ne pourra jamais
# etre lance ici, donc tout ce qui est decidable sans fenetre (le
# format, le nombre de frames, la validation de taille) doit vivre hors
# du fichier raylib et se tester dans la suite `unit`/`test` ordinaire.
# Ne lie que pico8_screencast.c et pico8_mem.c, dont il lit la memoire
# ecran ; aucun prerequis de fixture, le test ecrit ses propres fichiers
# sous tmp/ et les efface, comme test_pico8_recording_continuity.
$(TMP_DIR)/test_pico8_screencast: $(call objs,tests/test_pico8_screencast.c src/pico8_screencast.c src/pico8_mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Chunk 2 de docs/prd/triage/screencast-capture-and-viewer.md : la
# capture dans le pilote headless. Ce test est le seul du depot a
# DEPENDRE D'UN BINAIRE et a le lancer (system()), et c'est voulu : le
# PRD interdit un second pilote, donc les deux proprietes du chunk --
# fidelite et neutralite de la capture -- ne sont enoncables que sur le
# vrai tmp/pico8_replay, celui que `make replay` execute. D'ou les
# prerequis : le pilote lui-meme (reconstruit des qu'un source du moteur
# change, par les memes objets intermediaires que tout le reste) et sa
# fixture courte. Le test, lui, ne lie que ce qu'il doit relire --
# pico8_screencast.c pour la capture, pico8_dump.c pour la comparaison,
# pico8_input.c pour compter les frames de la fixture -- et non le
# moteur entier, qu'il n'appelle jamais dans son propre processus.
$(TMP_DIR)/test_pico8_headless_capture: $(call objs,tests/test_pico8_headless_capture.c src/pico8_screencast.c src/pico8_dump.c src/pico8_gfx.c src/pico8_mem.c src/pico8_input.c) $(TMP_DIR)/pico8_replay tests/fixtures/screencast_capture_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Chunk 4 de docs/prd/triage/screencast-capture-and-viewer.md : ce que
# le visualiseur decide sans fenetre -- quelle frame afficher
# maintenant, et ce qu'on dit d'un fichier qu'on refuse de jouer -- plus
# la palette PICO-8, sortie de src/pico8_raylib_backend.c pour que le
# visualiseur l'affiche sans la redefinir (le PRD le lui interdit).
# src/main_screencast_viewer.c, lui, inclut raylib.h et n'est donc
# exercable par aucune cible d'ici : c'est exactement la raison d'etre
# de ces deux modules, et la cinquieme application du precedent
# pico8_raylib_backend_coords / pico8_replay_source /
# pico8_session_recorder / pico8_screencast. Ne lie ni raylib, ni le
# moteur : le curseur ne connait qu'un nombre de frames, et la palette
# n'est que seize triplets.
$(TMP_DIR)/test_pico8_screencast_player: $(call objs,tests/test_pico8_screencast_player.c src/pico8_screencast_player.c src/pico8_palette.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_vec: $(call objs,tests/test_pico8_vec.c src/pico8_vec.c src/pico8_fixed.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_geom: $(call objs,tests/test_pico8_geom.c src/pico8_geom.c src/pico8_vec.c src/pico8_fixed.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_value: $(call objs,tests/test_pico8_value.c src/pico8_value.c src/pico8_fixed.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_list: $(call objs,tests/test_pico8_list.c src/pico8_list.c src/pico8_value.c src/pico8_fixed.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_str: $(call objs,tests/test_pico8_str.c src/pico8_str.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_ob: $(call objs,tests/test_pico8_ob.c src/pico8_ob.c src/pico8_list.c src/pico8_value.c src/pico8_str.c src/pico8_fixed.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_entity_smoke: $(call objs,tests/test_pico8_entity_smoke.c src/pico8_entity.c src/pico8_class.c src/pico8_ke.c src/pico8_ob.c src/pico8_list.c src/pico8_value.c src/pico8_str.c src/pico8_fixed.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_stash: $(call objs,tests/test_pico8_stash.c src/pico8_stash.c src/pico8_str.c src/pico8_ob.c src/pico8_list.c src/pico8_value.c src/pico8_fixed.c src/pico8_mem.c src/pico8_assets.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# pico8_data_init() exercises pico8_research()/pico8_tech_gate(), which pull
# in pico8_probe()'s own placeholder `pico8_fm()` -- from there, this test's
# real dependency closure widens to the rest of the entity/UI/dispatch
# engine those modules call into (unlike every other test above, none of
# which is real "data decoding" work). Full non-main() source list below
# reflects that real closure rather than an approximate subset.
$(TMP_DIR)/test_pico8_data_init: $(call objs,tests/test_pico8_data_init.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

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
$(TMP_DIR)/test_pico8_planet_kj_dispatch: $(call objs,tests/test_pico8_planet_kj_dispatch.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_demo: $(call objs,tests/test_pico8_demo.c src/pico8_mem.c src/pico8_assets.c src/pico8_gfx.c src/pico8_input.c src/pico8_dump.c) tests/fixtures/demo_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Chunk 1's own "full engine source list": pico8_cart_load() bootstraps
# essentially every class/data-table this whole port has built, so
# main.c's own real dependency closure is literally every module in
# src/ except itself -- the same reasoning test_pico8_data_init/
# test_pico8_planet_kj_dispatch's own header comments already establish
# for reaching real game state, now widened to the WHOLE engine since
# main.c reaches through pico8_cart_load() rather than one data module.
# src/pico8_screencast.c est nomme ici et pas ajoute a $(ENGINE_SRCS) :
# la capture du chunk 2 de docs/prd/triage/screencast-capture-and-
# viewer.md est une dependance de CE pilote, pas du moteur. Le glisser
# dans la liste partagee l'aurait lie dans les sept cibles qui s'en
# servent, dont aucune autre ne l'appelle.
$(TMP_DIR)/pico8_replay: $(call objs,src/main.c src/pico8_screencast.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

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
$(TMP_DIR)/test_pico8_playability: $(call objs,tests/test_pico8_playability.c $(ENGINE_SRCS)) tests/fixtures/phase4_playthrough_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# Phase 5 bug fix ("in screen game top menu is weird": missing icon
# outlines, blank technology counter, month drawn as placeholder blocks)
# -- see tests/test_pico8_top_menubar.c's own top comment for the three
# symptoms and the pixel each assertion pins. Reaches them the only way a
# human can, through a really booted cart (pico8_cart_load/pico8__init/
# pico8__update60/pico8__draw), so it shares test_pico8_playability's own
# full-engine source list verbatim, with its own inert 4-frame fixture.
$(TMP_DIR)/test_pico8_top_menubar: $(call objs,tests/test_pico8_top_menubar.c $(ENGINE_SRCS)) tests/fixtures/top_menubar_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# Phase 5 bug fix (hovering a shop-bar button aborted the headless
# framebuffer dumper on src/pico8_dump.c's own `assert(pixel <= 0x0f)`,
# and drew the item description panel's striped background wrong in the
# raylib window): jb:ol's own `fillp(0b0000111100001111)rk(om,dn,0x10)`
# (pico8/lua.lua:319) is a TWO-COLOUR fill, whose second colour lives in
# the colour parameter's high nibble. Reaches it the only way a human
# can -- a really booted cart with the cursor parked on a shop button --
# so it shares test_pico8_top_menubar's own full-engine source list
# verbatim, with its own 25-frame hover fixture. See
# tests/test_pico8_shop_tooltip_fillp.c's own top comment.
$(TMP_DIR)/test_pico8_shop_tooltip_fillp: $(call objs,tests/test_pico8_shop_tooltip_fillp.c $(ENGINE_SRCS)) tests/fixtures/shop_button_hover_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# Phase 5 bug fix (map() drew sprite 0 instead of treating it as
# "empty"): the galaxy sky's only map() call site is bg:kj(), so this
# test reaches the primitive the way a player does -- through
# pico8_bg_kj() with a realistic sky in the map. pico8_bg.c's own
# dependency closure is the same full-engine one test_pico8_data_init
# already links (bg -> ke/entity/dispatch), so its source list is
# reused verbatim here rather than re-derived.
$(TMP_DIR)/test_pico8_bg_kj: $(call objs,tests/test_pico8_bg_kj.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

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
$(TMP_DIR)/test_pico8_slipway_drag: $(call objs,tests/test_pico8_slipway_drag.c $(ENGINE_SRCS)) tests/fixtures/slipway_drag_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# Phase 5 bug fix (a planet a probe reveals as EMPTY faded out of view but
# was never removed from the world, so it went on refusing every structure
# dropped on that patch of space): the regression test for it,
# tests/test_pico8_empty_planet_cleared.c. Reaches the bug the way a human
# does -- scripted mouse input only (probe, wormhole, then a real tray
# drag), through the real pico8__update60()/pico8__draw() dispatch chain --
# so it shares pico8_replay/test_pico8_playability/test_pico8_slipway_drag's
# own full engine source list, for the same "reaching real game state pulls
# in essentially the whole engine" reasoning their own header comments
# already establish.
$(TMP_DIR)/test_pico8_empty_planet_cleared: $(call objs,tests/test_pico8_empty_planet_cleared.c $(ENGINE_SRCS)) tests/fixtures/empty_planet_cleared_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# Phase 5 bug fix (hard SIGSEGV on mouse RELEASE over a developed planet:
# jo:gh's own display rows store a raw pico8_list_t* in their "lq" field,
# which js:pq then handed to lq()/pico8_class_instantiate as if it were a
# pico8_ob_table_t -- see tests/test_pico8_planet_gh_row_lq.c's own top
# comment for the full trace). Built under AddressSanitizer ON PURPOSE and
# with no plain (non-ASan) twin: the crash this locks down is
# allocator-layout dependent (the very same replay exits 139 at -O0 but 0
# at -O1/-O2, while ASan reports the heap-buffer-overflow at every level),
# so the overflow itself -- not its downstream symptom -- is the only
# sound oracle. Same "a sanitizer report is a hard failure, not a printed
# warning that's easy to miss" reasoning as test_pico8_fixed_ubsan above;
# ASan aborts with a non-zero exit on the first report by default, so no
# -fno-sanitize-recover equivalent is needed. detect_leaks=0 because this
# port deliberately has no GC and leaks by design (pico8_value.h's own
# ownership note). Full engine source list for the same reason
# test_pico8_empty_planet_cleared's is (pico8_cart_load pulls in the
# whole port).
$(TMP_DIR)/test_pico8_planet_gh_row_lq: $(call asan_objs,tests/test_pico8_planet_gh_row_lq.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

# Reverse REF-cast audit (docs/prd/triage/reverse-direction-ref-cast-
# audit.md): the one field the audit could not call safe by construction,
# pinned to the panel line a player reads -- see the test's own top
# comment for the full trace, including why it is deliberately NOT built
# under a sanitizer (this cast direction under-reads and cannot leave the
# allocation, so there is nothing for ASan to find; its forward-direction
# twin, test_pico8_planet_gh_row_lq, overflows and therefore is). Full
# engine source list for the same reason that one's is: pico8_cart_load()
# pulls in the whole port.
$(TMP_DIR)/test_pico8_ga_export_panel_by: $(call objs,tests/test_pico8_ga_export_panel_by.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# Phase 5 bug fix (a garbage 8x8 icon -- PICO-8 sprite #0 -- drawn over a
# BUILT planet when its info panel opens, because `jo:lz`'s own faithfully
# ported `self.hn,self.ep=hp` nils `ep` and two truthiness helpers then
# read this port's nil as true): the regression test for it,
# tests/test_pico8_built_planet_panel_nil_row.c. Reaches the bug the way a
# human does -- scripted mouse input only (the phase4 playthrough that
# really colonizes a planet, then a plain click-and-release on it) through
# the real pico8__update60()/pico8__draw() dispatch chain -- so it shares
# pico8_replay/test_pico8_playability/test_pico8_empty_planet_cleared's own
# full engine source list, for the same "reaching real game state pulls in
# essentially the whole engine" reasoning their own header comments already
# establish.
$(TMP_DIR)/test_pico8_built_planet_panel_nil_row: $(call objs,tests/test_pico8_built_planet_panel_nil_row.c $(ENGINE_SRCS)) tests/fixtures/built_planet_panel_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# The OTHER half of that same fix, which the replay above cannot reach.
# test_pico8_built_planet_panel_nil_row pins the MSGBOX guard (`pz()`'s own
# `if p.ep then`); reverting src/pico8_layout.c's own `REF(NULL)` clause
# alone leaves the whole suite green, because once `pz()` stops building
# the bogus row no nil ever arrives at `lq:mn` on any replay in the repo --
# the single nil in the entire corpus was manufactured by the msgbox bug
# itself. This test pins `lq:mn`'s own `elseif f then` directly instead:
# one `lq()` holding one nil item, rendered with `lq:ol()`, must write no
# pixels. See tests/test_pico8_layout_nil_item_draws_nothing.c's own top
# comment (including its positive control, and why it is sprite #0's row 7
# and not row 0 that it scans for).
#
# NOT the full $(ENGINE_SRCS) closure, unlike its twin above: this one
# never boots a cart, so it needs only what pico8_layout.c itself reaches
# -- lq/jb and their constructors (class/ob/list/collect/stash/value/str/
# fixed/vec/geom), the drawing primitives `lq:mn`'s own branches call
# (gfx/hud/mem), the sprite sheet those read (assets), and
# pico8_dispatch_context.c for `lq`'s own mv()-routed "ol" field. Measured
# by linking, not guessed. Objects are shared with every other target
# through tmp/obj, so a shorter list costs nothing and buys a link line
# that says what this unit test actually depends on.
$(TMP_DIR)/test_pico8_layout_nil_item_draws_nothing: $(call objs,tests/test_pico8_layout_nil_item_draws_nothing.c src/pico8_assets.c src/pico8_class.c src/pico8_collect.c src/pico8_dispatch_context.c src/pico8_fixed.c src/pico8_geom.c src/pico8_gfx.c src/pico8_hud.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_ob.c src/pico8_stash.c src/pico8_str.c src/pico8_value.c src/pico8_vec.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

unit: $(TMP_DIR)/test_pico8_mem $(TMP_DIR)/test_asset_loading $(TMP_DIR)/test_pico8_gfx $(TMP_DIR)/test_pico8_input $(TMP_DIR)/test_pico8_dump $(TMP_DIR)/test_pico8_fixed $(TMP_DIR)/test_pico8_fixed_ubsan $(TMP_DIR)/test_pico8_vec $(TMP_DIR)/test_pico8_geom $(TMP_DIR)/test_pico8_value $(TMP_DIR)/test_pico8_list $(TMP_DIR)/test_pico8_str $(TMP_DIR)/test_pico8_ob $(TMP_DIR)/test_pico8_entity_smoke $(TMP_DIR)/test_pico8_stash $(TMP_DIR)/test_pico8_data_init $(TMP_DIR)/test_pico8_planet_kj_dispatch $(TMP_DIR)/test_pico8_demo $(TMP_DIR)/test_pico8_playability $(TMP_DIR)/test_pico8_slipway_drag $(TMP_DIR)/test_pico8_raylib_backend_coords $(TMP_DIR)/test_pico8_replay_source $(TMP_DIR)/test_pico8_recording_continuity $(TMP_DIR)/test_pico8_replay_control $(TMP_DIR)/test_pico8_marker_coherence $(TMP_DIR)/test_pico8_screencast $(TMP_DIR)/test_pico8_headless_capture $(TMP_DIR)/test_pico8_screencast_player $(TMP_DIR)/test_pico8_bg_kj $(TMP_DIR)/test_pico8_top_menubar $(TMP_DIR)/test_pico8_shop_tooltip_fillp $(TMP_DIR)/test_pico8_empty_planet_cleared $(TMP_DIR)/test_pico8_planet_gh_row_lq $(TMP_DIR)/test_pico8_ga_export_panel_by $(TMP_DIR)/test_pico8_built_planet_panel_nil_row $(TMP_DIR)/test_pico8_layout_nil_item_draws_nothing
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
	./$(TMP_DIR)/test_pico8_replay_source
	./$(TMP_DIR)/test_pico8_recording_continuity
	./$(TMP_DIR)/test_pico8_replay_control
	./$(TMP_DIR)/test_pico8_marker_coherence
	./$(TMP_DIR)/test_pico8_screencast
	./$(TMP_DIR)/test_pico8_headless_capture
	./$(TMP_DIR)/test_pico8_screencast_player
	./$(TMP_DIR)/test_pico8_bg_kj
	./$(TMP_DIR)/test_pico8_top_menubar
	./$(TMP_DIR)/test_pico8_shop_tooltip_fillp
	./$(TMP_DIR)/test_pico8_empty_planet_cleared
	ASAN_OPTIONS=detect_leaks=0 ./$(TMP_DIR)/test_pico8_planet_gh_row_lq
	./$(TMP_DIR)/test_pico8_ga_export_panel_by
	./$(TMP_DIR)/test_pico8_built_planet_panel_nil_row
	./$(TMP_DIR)/test_pico8_layout_nil_item_draws_nothing

replay: $(TMP_DIR)/pico8_replay
	./$(TMP_DIR)/pico8_replay

test: unit replay screencast-check

# ---------------------------------------------------------------------
# SCREENCAST : FABRICATION DES PIXELS, EMPREINTE VERSIONNEE, CONTROLE
# (chunk 3 de docs/prd/triage/screencast-capture-and-viewer.md).
#
# Le chunk 1 a donne le format (src/pico8_screencast.h), le chunk 2 a
# donne au pilote headless son deuxieme argument optionnel (src/main.c).
# Ici on cable les deux au systeme de construction, et c'est ce qui rend
# l'instrument OPERANT : a partir de maintenant, une modification du
# rendu fait rougir la suite -- une fois qu'un humain a pose une
# empreinte, et pas avant (voir plus bas, c'est la decision centrale de
# ce bloc).
#
# Les pixels d'une session sont une FONCTION du code et de l'events.json
# -- le rejeu headless est deterministe. C'est l'unique idee dont tout
# le reste decoule : le fichier de pixels n'a pas a etre conserve, il a
# a etre regenerable, et seule son empreinte est versionnee.

SCREENCAST_EVENTS := events.json
SCREENCAST_BIN := $(TMP_DIR)/screencast.bin
SCREENCAST_SHA := $(SCREENCAST_EVENTS).sha256

# Le visualiseur du chunk 4. Son nom est defini ICI, a cote des pixels
# qu'il ouvre, parce que le message d'echec ci-dessous doit pouvoir
# donner la commande reelle ; sa regle de construction, elle, est tout
# en bas avec celle de `raylib`, dont elle partage les options et les
# bibliotheques (et l'exigence de raylib vendorise).
SCREENCAST_VIEWER_BIN := $(TMP_DIR)/screencast_viewer

# L'outil de calcul du sha256, choisi sur $(UNAME_S) comme les options
# d'edition de liens de raylib le sont deja plus haut. `sha256sum`
# N'EXISTE PAS sur macOS : l'equivalent y est `shasum -a 256`, livre
# avec le systeme. Le PRD en fait une exigence nommee (user story 23) et
# la raison est concrete : le relevé PICO-8 de reference (tools/
# probe_out.txt, docs/plan-2.md §5) a ete fait sur un macOS, donc une
# cible qui ne saurait calculer une empreinte que sous Linux serait
# inutilisable exactement sur la machine qui sert d'oracle.
#
# Les deux outils impriment la meme chose, "<empreinte>  <chemin>", d'ou
# le `cut -d' ' -f1` commun ci-dessous : le chemin ne doit PAS entrer
# dans le fichier d'empreinte, sinon l'empreinte versionnee dependrait
# de l'endroit ou le fichier temporaire a ete ecrit.
#
# REJETE : une implementation de sha256 dans le C du projet. Le PRD
# l'interdit ("pas d'implementation de sha256 dans le code du projet"),
# et l'interdiction se justifie toute seule -- ce serait du code a
# tester, dans un depot dont le sujet est de porter un jeu, pour
# remplacer un outil present sur les deux systemes vises.
ifeq ($(UNAME_S),Darwin)
    SHA256_CMD := shasum -a 256
else
    SHA256_CMD := sha256sum
endif

# LA REGLE DE FABRICATION. Une regle de FICHIER, pas une cible factice :
# c'est la difference entre "make refait les pixels quand un source ou
# le scenario ont change" et "make refait les pixels a chaque fois qu'on
# lance la suite". Mesure sur l'events.json actuel : 9 729 frames, ~11 s
# et ~80 Mo. Une cible factice aurait donc ajoute 11 s a CHAQUE boucle
# rouge-vert, y compris celles qui ne touchent pas a src/.
#
# Deux prerequis, et ils suffisent :
#  - $(TMP_DIR)/pico8_replay, le pilote lui-meme. Il porte deja la
#    cloture complete du moteur a travers les objets intermediaires et
#    les fragments de dependances generes par le compilateur (voir le
#    grand commentaire de docs/prd/triage/makefile-header-prerequisites.
#    md en tete de fichier). Nommer ici "les objets du moteur" un par un
#    aurait redit, en moins fiable, ce que le binaire dit deja : editer
#    un .c OU un .h du moteur relie le pilote, et relier le pilote
#    refait les pixels.
#  - $(SCREENCAST_EVENTS), le scenario. Changer le scenario change les
#    pixels tout autant que changer le code.
# Le repertoire tmp/ est un prerequis d'ordre seul, comme partout
# ailleurs ici : sa date de modification change des qu'on y ecrit, donc
# en prerequis ordinaire il refabriquerait 80 Mo a chaque passage.
#
# Le fichier vit sous tmp/, deja ignore par git (.gitignore). Il n'entre
# jamais dans le depot, et le seul artefact durable de tout ce PRD est
# la ligne d'empreinte ci-dessous.
$(SCREENCAST_BIN): $(TMP_DIR)/pico8_replay $(SCREENCAST_EVENTS) | $(TMP_DIR)
	./$(TMP_DIR)/pico8_replay $(SCREENCAST_EVENTS) $@

# Le confort humain : "fabrique-moi les pixels", sans avoir a taper le
# chemin du fichier temporaire. C'est aussi ce qu'un humain lance avant
# d'ouvrir le visualiseur du chunk 4.
screencast: $(SCREENCAST_BIN)

# SI LA RECETTE ECHOUE, LE FICHIER PARTIEL EST DETRUIT. Sans cela, un
# rejeu interrompu (disque plein -- 80 Mo, ce n'est pas theorique) ou tue
# laisserait derriere lui un tmp/screencast.bin tronque MAIS plus recent
# que ses prerequis : make le croirait a jour et le controle suivant
# calculerait une empreinte sur un fichier incomplet, c'est-a-dire un
# faux rouge indiscernable d'une regression -- precisement ce que le
# chunk 2 s'est donne du mal a rendre impossible cote pilote (erreur
# fatale sur echec d'ecriture). Special target standard de GNU make,
# sans effet sur les cibles factices (unit, replay, test...) : elles
# n'ont pas de fichier a detruire.
.DELETE_ON_ERROR:

# LE CONTROLE, DONT `test` DEPEND.
#
# La forme voulue par le PRD : "La suite de tests DEPEND de ce fichier
# sans le fabriquer elle-meme". D'ou un prerequis et non un `make
# $(SCREENCAST_BIN)` dans la recette -- une recette qui fabrique refait
# le travail a chaque appel et court-circuite le calcul de peremption
# qui est toute la raison d'etre d'une regle de fichier.
#
# TANT QU'AUCUNE EMPREINTE N'EST POSEE, LA SUITE PASSE ET NE FABRIQUE
# RIEN. C'est une decision, pas un oubli, et elle a deux moities.
#
# Passer : docs/plan-2.md §5 porte un avertissement d'ordonnancement
# explicite -- "capturer ce filet avant d'avoir tranche la population
# (c) gelerait la troncature actuelle dans un fichier versionne, et
# toute correction ulterieure vers `flr` apparaitrait alors comme une
# regression. Un doute serait transforme en apparente certitude, dans le
# mauvais sens." Poser une empreinte, c'est affirmer que ce qui est
# dessine est ce qu'on VEUT. Ce chunk livre l'instrument ; il ne le
# charge pas. Un depot sans events.json.sha256 est l'etat normal jusqu'a
# ce qu'un humain ait regarde defiler une session et l'ait acceptee.
#
# Ne rien fabriquer : fabriquer 80 Mo en 11 s pour les comparer a rien
# serait du gaspillage pur. D'ou le $(wildcard) ci-dessous, evalue a la
# lecture du Makefile : sans fichier d'empreinte, screencast-check n'a
# AUCUN prerequis, donc `make test` ne lance meme pas le pilote.
#
# REJETE : mettre l'empreinte a jour automatiquement quand elle diverge.
# Le PRD l'interdit en toutes lettres -- "ce serait l'annulation
# silencieuse du filet". Une empreinte qui se re-pose toute seule ne
# repond plus a la question qui a motive tout ce PRD, "a quel commit un
# humain a-t-il valide pour la derniere fois ce que le jeu affiche".
# Seule screencast-sign ecrit ce fichier, et rien ne l'appelle.
ifeq ($(wildcard $(SCREENCAST_SHA)),)
SCREENCAST_CHECK_PREREQ :=
else
SCREENCAST_CHECK_PREREQ := $(SCREENCAST_BIN)
endif

# Le message est la seule interface humaine d'un echec, et un echec muet
# sur un hash ne dit rien a personne : il nomme les DEUX valeurs
# (attendue / obtenue), rappelle ou sont les pixels, comment les
# regarder, et quelle cible re-signe. Sans ces trois rappels, celui qui
# voit rougir la suite doit relire ce Makefile pour savoir quoi faire.
screencast-check: $(SCREENCAST_CHECK_PREREQ)
	@if [ ! -f "$(SCREENCAST_SHA)" ]; then \
	    echo "screencast: aucune empreinte de reference n'est posee ($(SCREENCAST_SHA) absent) -- rien n'a ete fabrique ni compare."; \
	    echo "screencast: pour en poser une, apres avoir regarde la session : make screencast-sign"; \
	else \
	    attendue=$$(cut -d' ' -f1 < "$(SCREENCAST_SHA)"); \
	    obtenue=$$($(SHA256_CMD) "$(SCREENCAST_BIN)" | cut -d' ' -f1); \
	    if [ "$$attendue" != "$$obtenue" ]; then \
	        echo "screencast: EMPREINTE DIVERGENTE -- le port ne dessine plus la meme chose sur $(SCREENCAST_EVENTS)." >&2; \
	        echo "screencast:   attendue ($(SCREENCAST_SHA)) : $$attendue" >&2; \
	        echo "screencast:   obtenue  ($(SCREENCAST_BIN)) : $$obtenue" >&2; \
	        echo "screencast: regenerer les pixels : make screencast   |   les regarder : make screencast-viewer && ./$(SCREENCAST_VIEWER_BIN) $(SCREENCAST_BIN)" >&2; \
	        echo "screencast: si la divergence est VOULUE et a ete regardee, re-signer : make screencast-sign" >&2; \
	        exit 1; \
	    fi; \
	    echo "screencast: empreinte conforme ($$attendue)"; \
	fi

# (RE)POSER L'EMPREINTE -- LE SEUL ENDROIT DU DEPOT QUI ECRIT CE
# FICHIER, ET RIEN NE L'APPELLE. C'est un acte humain delibere : on
# fabrique les pixels, on les regarde defiler, puis on lance ceci, et la
# ligne modifiee apparait dans le diff du commit. C'est ce qui donne son
# sens a `git blame` sur cette ligne -- "a quel commit un humain a-t-il
# valide ce que le jeu affiche".
#
# Le fichier fait une ligne et ne contient QUE l'empreinte : ni le
# chemin du fichier temporaire (qui varie), ni date, ni nom de machine.
# "L'empreinte porte sur les pixels et rien d'autre, afin qu'elle ne
# depende ni de la machine, ni de l'horodatage, ni d'un outil de
# compression" (user story 30). Le controle ci-dessus lit quand meme sa
# premiere colonne, donc une ligne au format natif de sha256sum posee a
# la main y passerait aussi.
screencast-sign: $(SCREENCAST_BIN)
	@$(SHA256_CMD) "$(SCREENCAST_BIN)" | cut -d' ' -f1 > "$(SCREENCAST_SHA)"
	@echo "screencast: empreinte posee dans $(SCREENCAST_SHA) : $$(cat $(SCREENCAST_SHA))"
	@echo "screencast: cette ligne est un acte de signature -- commettez-la avec ce que vous avez regarde."

.PHONY: screencast screencast-check screencast-sign screencast-viewer

# RESERVE CONNUE, `make -j`. src/main.c ecrit tmp/pico8_replay_dump.txt
# a chaque execution, quel que soit son scenario. Trois choses lancent
# ce binaire : la cible `replay`, tests/test_pico8_headless_capture (par
# system(), depuis `unit`) et desormais la regle ci-dessus. Sous
# `make -j test` ces trois-la peuvent tourner en meme temps et ecrire le
# meme chemin, et le test de capture RELIT ce dump juste apres son
# propre appel -- donc un faux rouge est possible. La concurrence entre
# `replay` et `unit` preexiste a ce chunk ; la regle ci-dessus en ajoute
# une troisieme source et allonge la fenetre.
#
# Aucune correction d'une ou deux lignes n'existe a ce niveau : le
# chemin du dump est en dur dans src/main.c (hors perimetre de ce
# chunk), `.NOTPARALLEL` avec prerequis demande GNU make >= 4.4 (4.3
# ici, ou la forme sans prerequis tuerait TOUT le parallelisme du
# depot), et serialiser par des prerequis d'ordre entre cibles factices
# forcerait `make replay` a lancer la suite unitaire. Consigne ici
# plutot que corrige a moitie ; `make test` en serie, la facon dont il
# est lance, n'est pas concerne.

# ---------------------------------------------------------------------
# REVERSE REF-CAST AUDIT (docs/prd/triage/reverse-direction-ref-cast-
# audit.md, docs/audit-ref-cast-reverse.md, src/pico8_ref_audit.h).
#
# `make ref-audit` is a DIAGNOSTIC, not a test, and is deliberately not
# part of `unit`/`test`: it answers "which REF -> pico8_list_t* cast
# sites did anything actually reach, and with which type tag", which is
# a question about the audit's own coverage, not about whether the port
# behaves. It is kept runnable so the next phase can replay it cheaply
# against whatever new fixtures exist then (the PRD's user story 12)
# instead of redoing a manual pass.
#
# The same closure the plain targets use, rebuilt in the refaudit tree.
# Seven binaries, chosen for what they REACH rather than what they
# assert: the headless replay driver (run below over every recorded
# events.json in the repo, so one binary covers six scenarios), the
# three fixture-driven interaction tests, the two data/dispatch tests
# that build real game state without replaying, and the lq regression
# test, which is the one that constructs a developed planet directly.
$(TMP_DIR)/pico8_replay_refaudit: $(call refaudit_objs,src/main.c src/pico8_screencast.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(REFAUDIT_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_playability_refaudit: $(call refaudit_objs,tests/test_pico8_playability.c $(ENGINE_SRCS)) tests/fixtures/phase4_playthrough_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) $(REFAUDIT_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_slipway_drag_refaudit: $(call refaudit_objs,tests/test_pico8_slipway_drag.c $(ENGINE_SRCS)) tests/fixtures/slipway_drag_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) $(REFAUDIT_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_empty_planet_cleared_refaudit: $(call refaudit_objs,tests/test_pico8_empty_planet_cleared.c $(ENGINE_SRCS)) tests/fixtures/empty_planet_cleared_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) $(REFAUDIT_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_planet_gh_row_lq_refaudit: $(call refaudit_objs,tests/test_pico8_planet_gh_row_lq.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(REFAUDIT_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_data_init_refaudit: $(call refaudit_objs,tests/test_pico8_data_init.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(REFAUDIT_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_planet_kj_dispatch_refaudit: $(call refaudit_objs,tests/test_pico8_planet_kj_dispatch.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(REFAUDIT_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_ga_export_panel_by_refaudit: $(call refaudit_objs,tests/test_pico8_ga_export_panel_by.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(REFAUDIT_CFLAGS) -o $@ $(filter %.o,$^) -lm

REFAUDIT_BINS := \
    $(TMP_DIR)/pico8_replay_refaudit \
    $(TMP_DIR)/test_pico8_playability_refaudit \
    $(TMP_DIR)/test_pico8_slipway_drag_refaudit \
    $(TMP_DIR)/test_pico8_empty_planet_cleared_refaudit \
    $(TMP_DIR)/test_pico8_planet_gh_row_lq_refaudit \
    $(TMP_DIR)/test_pico8_data_init_refaudit \
    $(TMP_DIR)/test_pico8_planet_kj_dispatch_refaudit \
    $(TMP_DIR)/test_pico8_ga_export_panel_by_refaudit

# Every binary appends its per-site tally to one file; the report tool
# then folds them together and diffs the result against the cast sites
# it greps out of src/, so a site nothing reached is printed as NOT
# COVERED rather than quietly missing from the output.
ref-audit: $(REFAUDIT_BINS)
	sh tools/ref_audit_report.sh

.PHONY: ref-audit

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
#
# THIS IS THE ONE TARGET LEFT IN THE OLD SOURCE-TO-EXECUTABLE FORM, ON
# PURPOSE (docs/prd/triage/makefile-header-prerequisites.md, whose
# Implementation Decisions and Out of Scope both say so). Every other
# target above now compiles through the object trees at the top of this
# file and gets its header prerequisites from the compiler; this one does
# not, because it cannot be built -- let alone run -- in the environments
# where that change was made and verified: raylib is not vendored there,
# so third_party/raylib/src/libraylib.a does not exist and `make raylib`
# stops before compiling anything. Converting it blind would also walk
# straight into the flag-mixing trap the top-of-file comment warns about:
# the three raylib-facing sources need `-I third_party/raylib/src` and the
# other 63 do not, and src/pico8_raylib_backend_coords.c is ALSO linked by
# the window-free test_pico8_raylib_backend_coords target without that
# flag, so a careless shared object tree would silently hand one target an
# object compiled for the other. Rewriting it is a fine follow-up for
# whoever next has a vendored raylib in front of them and can actually
# link the result; until then it keeps working exactly as it always has,
# at the cost of keeping the stale-header behaviour this PRD removed
# everywhere else -- which is the safe direction for a target no test can
# cover here.
$(TMP_DIR)/main_raylib: src/main_raylib.c src/pico8_raylib_backend.c src/pico8_raylib_backend.h src/pico8_raylib_backend_coords.c src/pico8_raylib_backend_coords.h src/pico8_palette.c src/pico8_palette.h src/pico8_assets.c src/pico8_assets.h src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_cart.c src/pico8_cart.h src/pico8_class.c src/pico8_collect.c src/pico8_data_init.c src/pico8_dispatch_context.c src/pico8_dispatch_context.h src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_galaxy.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_help.c src/pico8_help.h src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_menu.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_family.c src/pico8_planet_family_trade.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_replay_control.c src/pico8_replay_control.h src/pico8_replay_source.c src/pico8_replay_source.h src/pico8_research.c src/pico8_research_display.c src/pico8_research_icon.c src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_scroll.c src/pico8_session_recorder.c src/pico8_session_recorder.h src/pico8_slipway.c src/pico8_slipway_display.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_ui_button.h src/pico8_value.c src/pico8_vec.c src/pico8_wormhole.c src/pico8_wormhole_display.c third_party/raylib/src/libraylib.a | $(TMP_DIR)
	$(CC) $(CFLAGS) -I third_party/raylib/src -o $@ src/main_raylib.c src/pico8_raylib_backend.c src/pico8_raylib_backend_coords.c src/pico8_palette.c src/pico8_assets.c src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_cart.c src/pico8_class.c src/pico8_collect.c src/pico8_data_init.c src/pico8_dispatch_context.c src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_galaxy.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_help.c src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_menu.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_family.c src/pico8_planet_family_trade.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_replay_control.c src/pico8_replay_source.c src/pico8_research.c src/pico8_research_display.c src/pico8_research_icon.c src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_scroll.c src/pico8_session_recorder.c src/pico8_slipway.c src/pico8_slipway_display.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_value.c src/pico8_vec.c src/pico8_wormhole.c src/pico8_wormhole_display.c third_party/raylib/src/libraylib.a $(RAYLIB_LDLIBS)

raylib: $(TMP_DIR)/main_raylib

# Chunk 4 de docs/prd/triage/screencast-capture-and-viewer.md : LE
# VISUALISEUR. Une cible a elle, sur le modele exact de `raylib`
# ci-dessus -- memes options par systeme, memes bibliotheques, meme
# exigence d'un raylib deja vendorise (prerequis ordinaire sur third_
# party/raylib/src/libraylib.a : sans lui, un "No rule to make target"
# ordinaire, comme pour `raylib` et `psx-hello`).
#
# ELLE N'ENTRE NI DANS `test` NI DANS `unit`, ET C'EST UNE EXIGENCE, PAS
# UNE COMMODITE. Ce binaire ouvre une fenetre : aucun agent ne peut le
# lancer (pas de serveur graphique, CLAUDE.md l'interdit), et raylib
# n'est pas vendorise dans les environnements ou tourne la suite. Une
# suite qui en dependrait ne serait plus constructible la ou elle doit
# l'etre. Ce que ce binaire decide est teste ailleurs, par tmp/test_
# pico8_screencast_player et tmp/test_pico8_screencast, qui ne lient pas
# raylib ; ce qui reste ici -- une fenetre, une texture, un blit --
# n'est validable que par un humain qui l'ouvre.
#
# MEME FORME SOURCE-VERS-EXECUTABLE QUE `raylib`, ET POUR LA MEME
# RAISON (docs/prd/triage/makefile-header-prerequisites.md) : les deux
# sources qui incluent raylib.h ont besoin de `-I third_party/raylib/
# src`, que les autres n'ont pas, tandis que cinq des sources listees
# ici -- pico8_raylib_backend_coords.c, pico8_replay_control.c,
# pico8_screencast.c, pico8_screencast_player.c et pico8_palette.c --
# sont AUSSI liees par des cibles de test sans fenetre, qui elles
# passent par l'arbre d'objets partage. Melanger les deux jeux de
# drapeaux dans un meme arbre est exactement le piege que le
# commentaire en tete de fichier decrit, et il n'est de toute facon pas
# verifiable ici, ou cette cible ne se construit pas. Le cout assume
# est le meme que pour `raylib` : les prerequis d'en-tetes sont nommes
# a la main, dans une cible qu'aucun test ne couvre.
#
# POURQUOI src/pico8_raylib_backend.c EST DANS LA LISTE d'un programme
# qui ne joue a rien : le visualiseur lui emprunte ses deux seules
# fonctions qui ne recoivent pas de backend (lecture des trois touches
# de vitesse, application du plafond de frames), pour avoir les MEMES
# touches que le rejeu interactif par construction plutot que par
# recopie -- voir le commentaire en tete de src/main_screencast_
# viewer.c. L'edition de liens tire avec lui pico8_raylib_backend_
# coords.c, pico8_session_recorder.c, pico8_input.c et pico8_mem.c, dont
# pas une ligne ne s'execute dans ce programme : le visualiseur n'ouvre
# aucun fichier en ecriture.
$(SCREENCAST_VIEWER_BIN): src/main_screencast_viewer.c src/pico8_screencast.c src/pico8_screencast.h src/pico8_screencast_player.c src/pico8_screencast_player.h src/pico8_palette.c src/pico8_palette.h src/pico8_replay_control.c src/pico8_replay_control.h src/pico8_raylib_backend.c src/pico8_raylib_backend.h src/pico8_raylib_backend_coords.c src/pico8_raylib_backend_coords.h src/pico8_session_recorder.c src/pico8_session_recorder.h src/pico8_input.c src/pico8_input.h src/pico8_mem.c src/pico8_mem.h src/pico8_gfx.h third_party/raylib/src/libraylib.a | $(TMP_DIR)
	$(CC) $(CFLAGS) -I third_party/raylib/src -o $@ src/main_screencast_viewer.c src/pico8_screencast.c src/pico8_screencast_player.c src/pico8_palette.c src/pico8_replay_control.c src/pico8_raylib_backend.c src/pico8_raylib_backend_coords.c src/pico8_session_recorder.c src/pico8_input.c src/pico8_mem.c third_party/raylib/src/libraylib.a $(RAYLIB_LDLIBS)

# Comme `raylib`, cette cible CONSTRUIT et ne lance rien : il n'y a pas
# de ligne `./$(SCREENCAST_VIEWER_BIN)` ici, et il ne doit pas y en
# avoir. Celui qui veut regarder une session tape les deux commandes
# lui-meme, et c'est le message d'echec de screencast-check qui les lui
# rappelle :
#   make screencast && make screencast-viewer && ./tmp/screencast_viewer tmp/screencast.bin
screencast-viewer: $(SCREENCAST_VIEWER_BIN)

# The compiler-written dependency fragments described at the top of this
# file, one per object, pulled in per object tree. `-include` plus
# $(wildcard ...) means "use them if they are there": on a fresh clone
# there are none, nothing is included, and everything is compiled anyway.
-include $(wildcard $(OBJ_DIR)/src/*.d $(OBJ_DIR)/tests/*.d)
-include $(wildcard $(OBJ_ASAN_DIR)/src/*.d $(OBJ_ASAN_DIR)/tests/*.d)
-include $(wildcard $(OBJ_UBSAN_DIR)/src/*.d $(OBJ_UBSAN_DIR)/tests/*.d)
-include $(wildcard $(OBJ_REFAUDIT_DIR)/src/*.d $(OBJ_REFAUDIT_DIR)/tests/*.d)
