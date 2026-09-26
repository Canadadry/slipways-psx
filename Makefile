# Tout tourne en parallèle (-j au nombre de cœurs, sorties regroupées par cible).
#
# make test          -> unit + constants-check + float-grep + float-guard, la
#                       boucle rapide
# make e2e           -> frames-check + float-audit, les rejeux, en -O2
# make test-all      -> test + e2e, la suite complète
# make unit          -> les binaires de tests/, compilés et lancés
# make replay        -> rejoue la session humaine dans le moteur headless, hors
#                       suite : frames-check la rejoue déjà
# make screencast    -> refabrique les pixels de chaque frame du rejeu de la session humaine
# make frames-check  -> compare le sha256 de la capture de chaque rejeu du filet à
#                       sa référence <events>.sha256, dans make e2e
# make frames-baseline -> (re)pose ces références sur le code actuel, jamais par
#                       make test
# make frames-ref    -> pour chaque rejeu divergent, rejoue le commit qui a posé
#                       sa référence (git blame), laisse la capture dans
#                       tmp/frames/<events>.ref.bin et donne la première frame
#                       divergente ; hors suite
# make ref-audit     -> diagnostic des casts REF, hors suite
# make class-coverage DIR=<dossier> -> rejoue chaque *events.json posé directement
#                       dans DIR (défaut tests/fixtures, sans sous-dossiers) et
#                       compte les instanciations par classe ; échoue si une
#                       classe est à 0 hors tools/class_coverage_exempt.txt,
#                       hors suite
# make class-coverage-net -> idem sur les rejeus du filet (FRAMES_EVENTS) ; échoue
#                       aussi si sa sortie diffère de
#                       tools/class_coverage_net_expected.txt, hors suite
# make constants-check -> compare chaque PICO8_FIXED_LIT du cœur à son hexa relevé
#                       (tools/probe_out.txt) ; échoue si les écarts ne sont pas
#                       exactement tools/constants_expected_ecarts.txt, dans make test
# make float-audit   -> audit avant/après par site, dans make e2e : sites C1
#                       (tools/float_audit_c1.txt) ancien = nouveau, argument
#                       >= 0 et atteint ; sites D nouveau = flr. Imprime par
#                       fichier d'événements la frame du premier ancien != nouveau
# make float-guard   -> réécrit tools/float_guard.txt, en échec si un fichier
#                       du cœur a besoin du flottant (§6.3), dans make test
# make float-grep    -> grep du §6.1 sur le cœur (CORE_SRCS + en-têtes, -MM), le
#                       reste de src/ exclu par tools/float_grep_exclusions.txt,
#                       dans make test
# make float-link    -> lie les objets de tmp/pico8_replay sans -lm (§6.2) dans un
#                       binaire jetable, jamais lancé ; hors suite : make test
#                       construit déjà tmp/pico8_replay, même lien
# make sin-table     -> régénère src/pico8_quarter_wave.h (table de sin/cos du
#                       cœur) depuis tools/gen_quarter_wave.c, hors suite
# make raylib        -> compile et LINK le binaire interactif, ne le lance jamais
# make screencast-viewer -> idem pour le visualiseur d'enregistrement
#                       (les deux exigent make raylib-vendor-setup au préalable)
# make psx-hello     -> projet PSX de test (nécessite psn00bsdk-toolchain-setup)

.PHONY: psn00bsdk-toolchain-setup psx-hello psx-hello-clean unit replay test e2e test-all raylib-vendor-setup raylib screencast screencast-viewer frames-check frames-baseline frames-ref

NPROC := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
MAKEFLAGS += -j$(NPROC)
ifneq ($(filter output-sync,$(.FEATURES)),)
    MAKEFLAGS += --output-sync=target
endif

CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Werror -g

TMP_DIR := tmp

# Un arbre d'objets par jeu de flags. Un objet compilé sans -fsanitize n'est pas
# instrumenté : le partager avec une cible qui l'est rendrait le binaire aveugle
# sur ce code, sans rien dire. Ne pas fusionner les sept.
OBJ_DIR := $(TMP_DIR)/obj
OBJ_ASAN_DIR := $(TMP_DIR)/obj-asan
OBJ_UBSAN_DIR := $(TMP_DIR)/obj-ubsan
OBJ_REFAUDIT_DIR := $(TMP_DIR)/obj-refaudit
OBJ_AUDIT_DIR := $(TMP_DIR)/obj-audit
OBJ_COVERAGE_DIR := $(TMP_DIR)/obj-coverage
OBJ_E2E_DIR := $(TMP_DIR)/obj-e2e

# -MMD -MP : le compilateur écrit, à côté de chaque objet, la liste des en-têtes
# que l'unité de compilation a vraiment ouverts. Le -include en bas du fichier
# les relit. Aucun en-tête n'est donc nommé à la main dans une liste de prérequis.
DEPFLAGS := -MMD -MP
ASAN_CFLAGS := -fsanitize=address -fno-omit-frame-pointer
UBSAN_CFLAGS := -fsanitize=undefined -fno-sanitize-recover=undefined
REFAUDIT_CFLAGS := -DPICO8_REF_AUDIT
AUDIT_CFLAGS := -DPICO8_FLOAT_AUDIT
COVERAGE_CFLAGS := -DPICO8_CLASS_COVERAGE
# Rejeux de make e2e seulement ; les TU restent en -O0, lisibles sous gdb.
E2E_CFLAGS := -O2

# $(call objs,<liste de .c>) -> les chemins des .o correspondants dans l'arbre
# concerné. Une cible nomme ses sources une fois ; ses prérequis et sa ligne de
# link en découlent.
objs = $(patsubst %.c,$(OBJ_DIR)/%.o,$(1))
asan_objs = $(patsubst %.c,$(OBJ_ASAN_DIR)/%.o,$(1))
ubsan_objs = $(patsubst %.c,$(OBJ_UBSAN_DIR)/%.o,$(1))
refaudit_objs = $(patsubst %.c,$(OBJ_REFAUDIT_DIR)/%.o,$(1))
audit_objs = $(patsubst %.c,$(OBJ_AUDIT_DIR)/%.o,$(1))
coverage_objs = $(patsubst %.c,$(OBJ_COVERAGE_DIR)/%.o,$(1))
e2e_objs = $(patsubst %.c,$(OBJ_E2E_DIR)/%.o,$(1))

# mkdir -p $(@D) dans la recette plutôt qu'un répertoire en prérequis : le mtime
# d'un répertoire change à chaque fichier écrit dedans, ce qui rebuilderait tout.
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

$(OBJ_AUDIT_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(AUDIT_CFLAGS) $(E2E_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_COVERAGE_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(COVERAGE_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_E2E_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(E2E_CFLAGS) $(DEPFLAGS) -c -o $@ $<

# Closure complète du moteur : tout src/ sauf les deux points d'entrée (main.c,
# main_raylib.c) et les modules qui ne servent qu'au driver interactif. Utilisée
# par toute cible qui boote un cart, pico8_cart_load() tirant tout le port.
ENGINE_SRCS := \
    src/pico8_assets.c \
    src/pico8_audio.c \
    src/pico8_bg.c \
    src/pico8_bootstrap.c \
    src/pico8_cheat.c \
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

# Le cœur porté de docs/plan-2.md §4 : la closure du moteur moins les outils
# d'hôte, qui gardent la libc. Inventaire, couverture et garde flottantes
# partent tous de cette liste.
CORE_SRCS := $(filter-out src/pico8_assets.c src/pico8_dump.c,$(ENGINE_SRCS))

# Closure plus étroite : ce que pico8_data_init() atteint pour construire un
# vrai état de jeu, sans booter de cart.
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

# Libs système de raylib, pour les deux cibles raylib ; tout le reste est du C
# portable sans link spécifique à l'OS.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    RAYLIB_LDLIBS := -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo
else
    RAYLIB_LDLIBS := -lm -lGL -lX11 -lpthread -ldl -lrt
endif

$(TMP_DIR):
	mkdir -p $(TMP_DIR)

# Les .json en prérequis sont les fixtures que le test lit à l'exécution ;
# $(filter %.o,$^) les garde hors de la ligne de link.
$(TMP_DIR)/test_pico8_mem: $(call objs,tests/test_pico8_mem.c src/pico8_mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_asset_loading: $(call objs,tests/test_asset_loading.c src/pico8_assets.c src/pico8_mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_gfx: $(call objs,tests/test_pico8_gfx.c src/pico8_gfx.c src/pico8_mem.c src/pico8_assets.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_input: $(call objs,tests/test_pico8_input.c src/pico8_input.c) tests/fixtures/demo_events.json tests/fixtures/events_valid_ox.json tests/fixtures/events_malformed_five_dpad.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_dump: $(call objs,tests/test_pico8_dump.c src/pico8_dump.c src/pico8_gfx.c src/pico8_mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_fixed: $(call objs,tests/test_pico8_fixed.c src/pico8_fixed.c src/pico8_fixed_test_double.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# Le même test que ci-dessus, sous UBSan : pico8_fixed fait des réinterprétations
# de bits signé/non signé qui donneraient les bons nombres tout en étant de l'UB.
# -fno-sanitize-recover fait échouer le binaire au lieu d'imprimer un avertissement.
$(TMP_DIR)/test_pico8_fixed_ubsan: $(call ubsan_objs,tests/test_pico8_fixed.c src/pico8_fixed.c src/pico8_fixed_test_double.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(UBSAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_raylib_backend_coords: $(call objs,tests/test_pico8_raylib_backend_coords.c src/pico8_raylib_backend_coords.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_replay_source: $(call objs,tests/test_pico8_replay_source.c src/pico8_replay_source.c src/pico8_input.c) tests/fixtures/events_valid.json tests/fixtures/events_malformed_missing_field.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_recording_continuity: $(call objs,tests/test_pico8_recording_continuity.c src/pico8_session_recorder.c src/pico8_replay_source.c src/pico8_input.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_replay_control: $(call objs,tests/test_pico8_replay_control.c src/pico8_replay_control.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_marker_coherence: $(call objs,tests/test_pico8_marker_coherence.c src/pico8_session_recorder.c src/pico8_replay_source.c src/pico8_input.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_screencast: $(call objs,tests/test_pico8_screencast.c src/pico8_screencast.c src/pico8_mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Ce test lance le vrai pilote de rejeu, d'où $(TMP_DIR)/pico8_replay en
# prérequis : il rejoue la même fixture avec et sans capture et compare les deux
# dumps, pour que l'argument de capture reste sans effet sur le reste.
$(TMP_DIR)/test_pico8_headless_capture: $(call objs,tests/test_pico8_headless_capture.c src/pico8_screencast.c src/pico8_dump.c src/pico8_gfx.c src/pico8_mem.c src/pico8_input.c) $(TMP_DIR)/pico8_replay tests/fixtures/screencast_capture_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_replay_from_title: $(call objs,tests/test_pico8_replay_from_title.c) $(TMP_DIR)/pico8_replay tests/fixtures/title_click_forgiving.json tests/fixtures/title_click_reasonable.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/pico8_frames: $(call objs,src/main_frames.c src/pico8_screencast.c src/pico8_dump.c src/pico8_mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Lance le pilote de rejeu puis l'outil d'émission de frames.
$(TMP_DIR)/test_pico8_frames_tool: $(call objs,tests/test_pico8_frames_tool.c src/pico8_screencast.c src/pico8_mem.c) $(TMP_DIR)/pico8_replay $(TMP_DIR)/pico8_frames tests/fixtures/screencast_capture_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_frames_check: $(call objs,tests/test_frames_check.c) tools/frames_check.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_class_coverage: $(call coverage_objs,tests/test_class_coverage.c src/pico8_class.c src/pico8_ob.c src/pico8_list.c src/pico8_value.c src/pico8_str.c src/pico8_fixed.c) $(TMP_DIR)/pico8_class_coverage tools/class_coverage.sh tests/fixtures/screencast_capture_events.json tests/fixtures/events_malformed_missing_field.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_audit: $(call objs,tests/test_float_audit.c) tools/float_audit.sh tools/float_audit_c1.txt | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_guard: $(call objs,tests/test_float_guard.c) tools/float_guard.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_grep: $(call objs,tests/test_float_grep.c) tools/float_grep.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_link: $(call objs,tests/test_float_link.c) tools/float_link.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_constants_check: $(call objs,tests/test_constants_check.c) tools/constants_compare.sh tools/constants_check.sh src/pico8_fixed.c src/pico8_fixed.h | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_screencast_player: $(call objs,tests/test_pico8_screencast_player.c src/pico8_screencast_player.c src/pico8_palette.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_vec: $(call objs,tests/test_pico8_vec.c src/pico8_vec.c src/pico8_fixed.c src/pico8_fixed_test_double.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_geom: $(call objs,tests/test_pico8_geom.c src/pico8_geom.c src/pico8_vec.c src/pico8_fixed.c src/pico8_fixed_test_double.c) | $(TMP_DIR)
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

$(TMP_DIR)/test_pico8_class_chain: $(call objs,tests/test_pico8_class_chain.c src/pico8_class.c src/pico8_ob.c src/pico8_list.c src/pico8_value.c src/pico8_str.c src/pico8_fixed.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_stash: $(call objs,tests/test_pico8_stash.c src/pico8_stash.c src/pico8_str.c src/pico8_ob.c src/pico8_list.c src/pico8_value.c src/pico8_fixed.c src/pico8_mem.c src/pico8_assets.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_data_init: $(call objs,tests/test_pico8_data_init.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_jb_ol: $(call objs,tests/test_pico8_jb_ol.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_iv: $(call objs,tests/test_pico8_iv.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_mh: $(call objs,tests/test_pico8_mh.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_fq: $(call objs,tests/test_pico8_fq.c $(ENGINE_SRCS)) tests/fixtures/score_panel_close_events.json tests/fixtures/game_end_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_nf: $(call objs,tests/test_pico8_nf.c $(ENGINE_SRCS)) tests/fixtures/trading_hub_session_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_je: $(call objs,tests/test_pico8_je.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_mo: $(call objs,tests/test_pico8_mo.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_qb: $(call objs,tests/test_pico8_qb.c $(ENGINE_SRCS)) tests/fixtures/long_session_events.json tests/fixtures/geoharvest_burst_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_mz: $(call objs,tests/test_pico8_mz.c $(ENGINE_SRCS)) tests/fixtures/phase4_playthrough_events.json tests/fixtures/geoharvest_burst_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_bg: $(call objs,tests/test_pico8_bg.c $(ENGINE_SRCS)) tests/fixtures/phase4_playthrough_events.json tests/fixtures/long_session_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_jc: $(call objs,tests/test_pico8_jc.c $(ENGINE_SRCS)) tests/fixtures/slipway_drag_events.json tests/fixtures/trading_hub_session_events.json tests/fixtures/phase4_playthrough_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_ie: $(call objs,tests/test_pico8_ie.c $(ENGINE_SRCS)) tests/fixtures/events_recorded_1790198294.json tests/fixtures/trading_hub_session_events.json tests/fixtures/long_session_events.json tests/fixtures/phase4_playthrough_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_js: $(call objs,tests/test_pico8_js.c $(ENGINE_SRCS)) tests/fixtures/title_click_reasonable.json tests/fixtures/pause_menu_music_events.json tests/fixtures/tech_screen_invent_events.json tests/fixtures/built_planet_panel_events.json tests/fixtures/game_end_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_jo: $(call objs,tests/test_pico8_jo.c $(ENGINE_SRCS)) tests/fixtures/built_planet_panel_events.json tests/fixtures/empty_planet_cleared_events.json tests/fixtures/slipway_drag_events.json tests/fixtures/phase4_playthrough_events.json tests/fixtures/session_events.json tests/fixtures/events_recorded_1790198294.json tests/fixtures/trading_hub_session_events.json tests/fixtures/long_session_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_planet_kj_dispatch: $(call objs,tests/test_pico8_planet_kj_dispatch.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_demo: $(call objs,tests/test_pico8_demo.c src/pico8_mem.c src/pico8_assets.c src/pico8_gfx.c src/pico8_input.c src/pico8_dump.c) tests/fixtures/demo_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

REPLAY_SRCS := src/main.c src/pico8_screencast.c $(ENGINE_SRCS)

$(TMP_DIR)/pico8_replay: $(call objs,$(REPLAY_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/pico8_replay_e2e: $(call e2e_objs,$(REPLAY_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_playability: $(call objs,tests/test_pico8_playability.c $(ENGINE_SRCS) src/pico8_fixed_test_double.c) tests/fixtures/phase4_playthrough_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_top_menubar: $(call objs,tests/test_pico8_top_menubar.c $(ENGINE_SRCS)) tests/fixtures/top_menubar_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_shop_tooltip_fillp: $(call objs,tests/test_pico8_shop_tooltip_fillp.c $(ENGINE_SRCS)) tests/fixtures/shop_button_hover_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_bg_kj: $(call objs,tests/test_pico8_bg_kj.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_slipway_drag: $(call objs,tests/test_pico8_slipway_drag.c $(ENGINE_SRCS) src/pico8_fixed_test_double.c) tests/fixtures/slipway_drag_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_tech_screen: $(call objs,tests/test_pico8_tech_screen.c $(ENGINE_SRCS) src/pico8_fixed_test_double.c) tests/fixtures/tech_screen_invent_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_game_end: $(call objs,tests/test_pico8_game_end.c $(ENGINE_SRCS)) tests/fixtures/game_end_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_pause_menu_music: $(call objs,tests/test_pico8_pause_menu_music.c $(ENGINE_SRCS)) tests/fixtures/pause_menu_music_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_geoharvest_burst: $(call objs,tests/test_pico8_geoharvest_burst.c $(ENGINE_SRCS) src/pico8_fixed_test_double.c) tests/fixtures/geoharvest_burst_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_score_panel_close: $(call objs,tests/test_pico8_score_panel_close.c $(ENGINE_SRCS)) tests/fixtures/score_panel_close_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_score_panel_stars: $(call objs,tests/test_pico8_score_panel_stars.c $(ENGINE_SRCS)) tests/fixtures/score_panel_close_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_empty_planet_cleared: $(call objs,tests/test_pico8_empty_planet_cleared.c $(ENGINE_SRCS) src/pico8_fixed_test_double.c) tests/fixtures/empty_planet_cleared_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# Sous ASan, et sans jumeau non instrumenté : le crash visé dépend de la
# disposition de l'allocateur (le même rejeu sort 139 en -O0 et 0 en -O1), donc
# seul le débordement lui-même est un oracle fiable. detect_leaks=0 au lancement,
# ce port fuit par conception.
$(TMP_DIR)/test_pico8_planet_gh_row_lq: $(call asan_objs,tests/test_pico8_planet_gh_row_lq.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_ga_export_panel_by: $(call objs,tests/test_pico8_ga_export_panel_by.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_built_planet_panel_nil_row: $(call objs,tests/test_pico8_built_planet_panel_nil_row.c $(ENGINE_SRCS)) tests/fixtures/built_planet_panel_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_rich_planet_exports: $(call objs,tests/test_pico8_rich_planet_exports.c $(ENGINE_SRCS)) tests/fixtures/built_planet_panel_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_cheat_code: $(call objs,tests/test_pico8_cheat_code.c $(ENGINE_SRCS)) tests/fixtures/cheat_code_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_probe_mm_rnd_order: $(call objs,tests/test_pico8_probe_mm_rnd_order.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# Liste de sources réduite et non $(ENGINE_SRCS) : ce test ne boote pas de cart.
# Mesurée en linkant, pas devinée.
$(TMP_DIR)/test_pico8_layout_nil_item_draws_nothing: $(call objs,tests/test_pico8_layout_nil_item_draws_nothing.c src/pico8_assets.c src/pico8_class.c src/pico8_collect.c src/pico8_dispatch_context.c src/pico8_fixed.c src/pico8_geom.c src/pico8_gfx.c src/pico8_hud.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_ob.c src/pico8_stash.c src/pico8_str.c src/pico8_value.c src/pico8_vec.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

UNIT_TESTS := \
    test_pico8_mem \
    test_asset_loading \
    test_pico8_gfx \
    test_pico8_input \
    test_pico8_dump \
    test_pico8_fixed \
    test_pico8_fixed_ubsan \
    test_pico8_vec \
    test_pico8_geom \
    test_pico8_value \
    test_pico8_list \
    test_pico8_str \
    test_pico8_ob \
    test_pico8_entity_smoke \
    test_pico8_stash \
    test_pico8_data_init \
    test_pico8_planet_kj_dispatch \
    test_pico8_demo \
    test_pico8_playability \
    test_pico8_slipway_drag \
    test_pico8_tech_screen \
    test_pico8_game_end \
    test_pico8_pause_menu_music \
    test_pico8_geoharvest_burst \
    test_pico8_score_panel_close \
    test_pico8_score_panel_stars \
    test_pico8_raylib_backend_coords \
    test_pico8_replay_source \
    test_pico8_recording_continuity \
    test_pico8_replay_control \
    test_pico8_marker_coherence \
    test_pico8_screencast \
    test_pico8_headless_capture \
    test_pico8_replay_from_title \
    test_pico8_screencast_player \
    test_pico8_frames_tool \
    test_frames_check \
    test_class_coverage \
    test_float_audit \
    test_float_guard \
    test_float_grep \
    test_float_link \
    test_constants_check \
    test_pico8_bg_kj \
    test_pico8_probe_mm_rnd_order \
    test_pico8_top_menubar \
    test_pico8_shop_tooltip_fillp \
    test_pico8_empty_planet_cleared \
    test_pico8_planet_gh_row_lq \
    test_pico8_ga_export_panel_by \
    test_pico8_built_planet_panel_nil_row \
    test_pico8_layout_nil_item_draws_nothing \
    test_pico8_rich_planet_exports \
    test_pico8_cheat_code \
    test_pico8_jb_ol \
    test_pico8_iv \
    test_pico8_mh \
    test_pico8_fq \
    test_pico8_nf \
    test_pico8_je \
    test_pico8_qb \
    test_pico8_mo \
    test_pico8_mz \
    test_pico8_bg \
    test_pico8_jc \
    test_pico8_ie \
    test_pico8_class_chain \
    test_pico8_js \
    test_pico8_jo

# Un lancement par TU, en cible à part : c'est ce que -j parallélise.
UNIT_RUNS := $(addprefix unit-run-,$(UNIT_TESTS))

UNIT_RUNS_SPECIAL := unit-run-test_pico8_score_panel_stars unit-run-test_pico8_planet_gh_row_lq

unit: $(UNIT_RUNS)

$(filter-out $(UNIT_RUNS_SPECIAL),$(UNIT_RUNS)): unit-run-%: $(TMP_DIR)/%
	./$<

unit-run-test_pico8_score_panel_stars: $(TMP_DIR)/test_pico8_score_panel_stars
	./$< 1
	./$< 2

unit-run-test_pico8_planet_gh_row_lq: $(TMP_DIR)/test_pico8_planet_gh_row_lq
	ASAN_OPTIONS=detect_leaks=0 ./$<

# Ces deux TU relisent tmp/pico8_replay_dump.txt, le dump par défaut de
# pico8_replay : sous -j, jamais en même temps.
unit-run-test_pico8_frames_tool: | unit-run-test_pico8_headless_capture

.PHONY: $(UNIT_RUNS)

replay: $(TMP_DIR)/pico8_replay_e2e
	./$(TMP_DIR)/pico8_replay_e2e $(call from_title,$(SCREENCAST_EVENTS)) $(SCREENCAST_EVENTS)

test: unit constants-check float-grep float-guard

e2e: frames-check float-audit

test-all: test e2e

# ENREGISTREMENT D'ÉCRAN : make screencast rejoue SCREENCAST_EVENTS en écrivant les
# pixels de chaque frame ; c'est aussi la capture du rejeu de SCREENCAST_EVENTS du filet.
SCREENCAST_EVENTS := tests/fixtures/session_events.json
SCREENCAST_BIN := $(TMP_DIR)/screencast.bin

SCREENCAST_VIEWER_BIN := $(TMP_DIR)/screencast_viewer

# Les enregistrements raylib cliquent eux-mêmes la difficulté sur l'écran
# titre : tout pilote qui les rejoue passe $(call from_title,<events>).
RECORDINGS := \
    tests/fixtures/session_events.json \
    tests/fixtures/long_session_events.json \
    tests/fixtures/events_recorded_1790198294.json \
    tests/fixtures/trading_hub_session_events.json
from_title = $(if $(filter $(RECORDINGS),$(1)),--from-title)

ifeq ($(UNAME_S),Darwin)
    SHA256_CMD := shasum -a 256
else
    SHA256_CMD := sha256sum
endif

$(SCREENCAST_BIN): $(TMP_DIR)/pico8_replay_e2e $(SCREENCAST_EVENTS) | $(TMP_DIR)
	./$(TMP_DIR)/pico8_replay_e2e $(call from_title,$(SCREENCAST_EVENTS)) $(SCREENCAST_EVENTS) $@ $(@:.bin=.dump.txt)

screencast: $(SCREENCAST_BIN)

# Une capture interrompue laisserait un fichier partiel que make croirait à jour,
# et l'empreinte suivante porterait sur lui.
.DELETE_ON_ERROR:

# FILET (docs/plan-2.md §5) : le sha256 de la capture de chaque fichier
# d'événements, versionné à côté de lui sous <events>.sha256. La capture est la
# concaténation brute des frames : son empreinte couvre aussi leur nombre.
# Liste explicite : un motif sur tests/fixtures/ ramasserait les fixtures
# malformées et celles du lecteur d'entrée.
FRAMES_EVENTS := \
    $(SCREENCAST_EVENTS) \
    tests/fixtures/phase4_playthrough_events.json \
    tests/fixtures/built_planet_panel_events.json \
    tests/fixtures/empty_planet_cleared_events.json \
    tests/fixtures/slipway_drag_events.json \
    tests/fixtures/shop_button_hover_events.json \
    tests/fixtures/screencast_capture_events.json \
    tests/fixtures/top_menubar_events.json \
    tests/fixtures/tech_screen_invent_events.json \
    tests/fixtures/game_end_events.json \
    tests/fixtures/pause_menu_music_events.json \
    tests/fixtures/geoharvest_burst_events.json \
    tests/fixtures/long_session_events.json \
    tests/fixtures/events_recorded_1790198294.json \
    tests/fixtures/trading_hub_session_events.json

FRAMES_TOOL := $(TMP_DIR)/pico8_frames
FRAMES_DIR := $(TMP_DIR)/frames

frames_capture = $(if $(filter $(SCREENCAST_EVENTS),$(1)),$(SCREENCAST_BIN),$(FRAMES_DIR)/$(1).bin)
frames_current = $(FRAMES_DIR)/$(1).sha256
frames_reference = $(1).sha256

# Sans .SECONDARY, make effacerait ces captures intermédiaires, que frames-ref
# compare à la capture de référence.
.SECONDARY: $(foreach e,$(filter-out $(SCREENCAST_EVENTS),$(FRAMES_EVENTS)),$(call frames_capture,$(e)))

# Chaque capture a son propre dump texte : le dump par défaut,
# tmp/pico8_replay_dump.txt, est relu par deux TU qui tournent en parallèle.
$(FRAMES_DIR)/%.bin: % $(TMP_DIR)/pico8_replay_e2e
	@mkdir -p $(@D)
	./$(TMP_DIR)/pico8_replay_e2e $(call from_title,$<) $< $@ $(@:.bin=.dump.txt) > /dev/null

# L'affectation porte le code de sortie du sha256, qu'un pipe vers cut perdrait.
frames_hash = @mkdir -p $(@D); h=$$($(SHA256_CMD) $<) && echo "$${h%% *}" > $@

$(FRAMES_DIR)/%.sha256: $(FRAMES_DIR)/%.bin
	$(frames_hash)

$(call frames_current,$(SCREENCAST_EVENTS)): $(SCREENCAST_BIN)
	$(frames_hash)

# Un rejeu sans référence n'est ni capturé ni comparé.
FRAMES_CHECK_PREREQ := $(foreach e,$(FRAMES_EVENTS),$(if $(wildcard $(call frames_reference,$(e))),$(call frames_current,$(e))))
FRAMES_TRIPLETS := $(foreach e,$(FRAMES_EVENTS),$(e) $(call frames_reference,$(e)) $(call frames_current,$(e)))

frames-check: $(FRAMES_CHECK_PREREQ) tools/frames_check.sh
	@sh tools/frames_check.sh $(FRAMES_TRIPLETS)

frames-baseline: $(foreach e,$(FRAMES_EVENTS),$(call frames_current,$(e)))
	@$(foreach e,$(FRAMES_EVENTS),cp $(call frames_current,$(e)) $(call frames_reference,$(e)) && echo "frames: reference posee dans $(call frames_reference,$(e)) : $$(cat $(call frames_reference,$(e)))" && ) true

frames-ref: $(FRAMES_CHECK_PREREQ) $(FRAMES_TOOL) tools/frames_ref.sh
	@sh tools/frames_ref.sh ./$(FRAMES_TOOL) $(FRAMES_DIR) $(TMP_DIR)/frames-ref $(foreach e,$(FRAMES_EVENTS),$(e) $(call frames_reference,$(e)) $(call frames_current,$(e)) $(call frames_capture,$(e)) $(or $(call from_title,$(e)),-))

# make ref-audit : un diagnostic, pas un test, et délibérément hors de `test`.
# Il répond « quels sites de cast REF ont été atteints, et avec quel tag », en
# rejouant les binaires ci-dessous compilés avec -DPICO8_REF_AUDIT.
$(TMP_DIR)/pico8_replay_refaudit: $(call refaudit_objs,src/main.c src/pico8_screencast.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(REFAUDIT_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_playability_refaudit: $(call refaudit_objs,tests/test_pico8_playability.c $(ENGINE_SRCS) src/pico8_fixed_test_double.c) tests/fixtures/phase4_playthrough_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) $(REFAUDIT_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_slipway_drag_refaudit: $(call refaudit_objs,tests/test_pico8_slipway_drag.c $(ENGINE_SRCS) src/pico8_fixed_test_double.c) tests/fixtures/slipway_drag_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) $(REFAUDIT_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_empty_planet_cleared_refaudit: $(call refaudit_objs,tests/test_pico8_empty_planet_cleared.c $(ENGINE_SRCS) src/pico8_fixed_test_double.c) tests/fixtures/empty_planet_cleared_events.json | $(TMP_DIR)
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

# Chaque binaire ajoute son décompte à un fichier ; le script les additionne et
# compare aux sites qu'il grep dans src/, pour qu'un site jamais atteint sorte en
# NOT COVERED au lieu de manquer en silence.
ref-audit: $(REFAUDIT_BINS)
	sh tools/ref_audit_report.sh

.PHONY: ref-audit

# Tout vient de l'arbre coverage, qui n'est lié qu'ici et dans
# test_class_coverage : la struct de classe n'y a pas la même forme.
$(TMP_DIR)/pico8_class_coverage: $(call coverage_objs,src/main_class_coverage.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

DIR ?= tests/fixtures
CLASS_COVERAGE_EXEMPT := tools/class_coverage_exempt.txt
CLASS_COVERAGE_NET_EXPECTED := tools/class_coverage_net_expected.txt

class_coverage_recordings = $(foreach e,$(RECORDINGS),--from-title $(e))

class-coverage: $(TMP_DIR)/pico8_class_coverage tools/class_coverage.sh $(CLASS_COVERAGE_EXEMPT)
	@sh tools/class_coverage.sh $(class_coverage_recordings) ./$(TMP_DIR)/pico8_class_coverage '$(DIR)' $(CLASS_COVERAGE_EXEMPT)

class-coverage-net: $(TMP_DIR)/pico8_class_coverage tools/class_coverage.sh $(CLASS_COVERAGE_EXEMPT) $(CLASS_COVERAGE_NET_EXPECTED)
	@sh tools/class_coverage.sh $(class_coverage_recordings) --expect $(CLASS_COVERAGE_NET_EXPECTED) ./$(TMP_DIR)/pico8_class_coverage - $(CLASS_COVERAGE_EXEMPT) $(FRAMES_EVENTS)

.PHONY: class-coverage class-coverage-net

# Les sources du cœur, où le comparateur lit les appels PICO8_FIXED_LIT.
constants-check: tools/constants_check.sh tools/constants_compare.sh tools/constants_expected_ecarts.txt tools/probe_out.txt src/pico8_fixed.c src/pico8_fixed.h $(CORE_SRCS) | $(TMP_DIR)
	@sh tools/constants_check.sh tools/constants_expected_ecarts.txt tools/probe_out.txt src/pico8_fixed.c $(TMP_DIR)/constants_check \
	    $(CORE_SRCS)

.PHONY: constants-check

AUDIT_DIR := $(TMP_DIR)/audit

# pico8_fixed.c vient de l'arbre e2e, sans le define : sous -DPICO8_FLOAT_AUDIT,
# la macro réécrirait la définition de pico8_fixed_flr_int.
$(TMP_DIR)/pico8_replay_audit: $(call audit_objs,src/main.c src/pico8_screencast.c src/pico8_float_audit.c $(filter-out src/pico8_fixed.c,$(ENGINE_SRCS))) $(call e2e_objs,src/pico8_fixed.c src/pico8_fixed_test_double.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

FLOAT_AUDIT_C1 := tools/float_audit_c1.txt
float_audit_records = $(AUDIT_DIR)/$(1).aud

# Le binaire d'audit ajoute à PICO8_FLOAT_AUDIT_OUT sans le vider, d'où le rm.
$(AUDIT_DIR)/%.aud: % $(TMP_DIR)/pico8_replay_audit
	@mkdir -p $(@D)
	@rm -f $@
	PICO8_FLOAT_AUDIT_OUT=$@ ./$(TMP_DIR)/pico8_replay_audit $(call from_title,$<) $< /dev/null $(@:.aud=.dump.txt) > /dev/null

float-audit: $(foreach e,$(FRAMES_EVENTS),$(call float_audit_records,$(e))) tools/float_audit.sh $(FLOAT_AUDIT_C1)
	sh tools/float_audit.sh $(FLOAT_AUDIT_C1) $(foreach e,$(FRAMES_EVENTS),$(e) $(call float_audit_records,$(e))) -- $(CORE_SRCS)

.PHONY: float-audit

FLOAT_GUARD := tools/float_guard.txt

# Le rapport est réécrit même quand la garde échoue ; le code de sortie du
# script est rendu après le mv.
float-guard: tools/float_guard.sh | $(TMP_DIR)
	@status=0; sh tools/float_guard.sh "$(CC)" "$(CFLAGS)" $(TMP_DIR)/float-guard $(CORE_SRCS) > $(TMP_DIR)/float_guard.txt || status=$$?; \
	mv $(TMP_DIR)/float_guard.txt $(FLOAT_GUARD); \
	grep '^garde ' $(FLOAT_GUARD); \
	exit $$status

.PHONY: float-guard

FLOAT_GREP_EXCLUSIONS := tools/float_grep_exclusions.txt

float-grep: tools/float_grep.sh $(FLOAT_GREP_EXCLUSIONS)
	@sh tools/float_grep.sh "$(CC)" "$(CFLAGS)" $(FLOAT_GREP_EXCLUSIONS) src $(CORE_SRCS)

.PHONY: float-grep

float-link: $(call objs,$(REPLAY_SRCS)) tools/float_link.sh
	@sh tools/float_link.sh "$(CC)" "$(CFLAGS)" $(TMP_DIR)/float-link/pico8_replay $(call objs,$(REPLAY_SRCS))

.PHONY: float-link

$(TMP_DIR)/gen_quarter_wave: tools/gen_quarter_wave.c | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $< -lm

sin-table: $(TMP_DIR)/gen_quarter_wave
	./$(TMP_DIR)/gen_quarter_wave > $(TMP_DIR)/pico8_quarter_wave.h
	mv $(TMP_DIR)/pico8_quarter_wave.h src/pico8_quarter_wave.h

.PHONY: sin-table

psn00bsdk-toolchain-setup:
	sh tools/install_psn00bsdk_toolchain.sh

psx-hello:
	cmake -S psx-hello -B psx-hello/build && cmake --build psx-hello/build

psx-hello-clean:
	rm -rf psx-hello/build

# Clone une version figée de raylib dans third_party/ (gitignoré) et construit sa
# lib statique. Lent, réseau, une seule fois, volontairement hors de `make test`.
raylib-vendor-setup:
	sh tools/install_raylib_vendor.sh

# Link seulement : cette cible ne lance jamais le binaire (aucune ligne
# ./$(TMP_DIR)/...), voir CLAUDE.md. Elle exige third_party/raylib/src/libraylib.a,
# donc `make raylib` sans vendoring échoue en « no rule to make target ».
# Les deux cibles raylib sont restées en forme source -> exécutable : leurs
# fichiers ont besoin de -I third_party/raylib/src que les autres n'ont pas, et
# pico8_raylib_backend_coords.c est aussi linké sans ce flag par un test.
$(TMP_DIR)/main_raylib: src/main_raylib.c src/pico8_raylib_backend.c src/pico8_raylib_backend.h src/pico8_raylib_backend_coords.c src/pico8_raylib_backend_coords.h src/pico8_palette.c src/pico8_palette.h src/pico8_assets.c src/pico8_assets.h src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_cart.c src/pico8_cart.h src/pico8_cheat.c src/pico8_cheat.h src/pico8_class.c src/pico8_collect.c src/pico8_data_init.c src/pico8_dispatch_context.c src/pico8_dispatch_context.h src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_galaxy.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_help.c src/pico8_help.h src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_menu.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_family.c src/pico8_planet_family_trade.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_replay_control.c src/pico8_replay_control.h src/pico8_replay_source.c src/pico8_replay_source.h src/pico8_research.c src/pico8_research_display.c src/pico8_research_icon.c src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_scroll.c src/pico8_session_recorder.c src/pico8_session_recorder.h src/pico8_slipway.c src/pico8_slipway_display.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_ui_button.h src/pico8_value.c src/pico8_vec.c src/pico8_wormhole.c src/pico8_wormhole_display.c third_party/raylib/src/libraylib.a | $(TMP_DIR)
	$(CC) $(CFLAGS) -I third_party/raylib/src -o $@ src/main_raylib.c src/pico8_raylib_backend.c src/pico8_raylib_backend_coords.c src/pico8_palette.c src/pico8_assets.c src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_cart.c src/pico8_cheat.c src/pico8_class.c src/pico8_collect.c src/pico8_data_init.c src/pico8_dispatch_context.c src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_galaxy.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_help.c src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_menu.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_family.c src/pico8_planet_family_trade.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_replay_control.c src/pico8_replay_source.c src/pico8_research.c src/pico8_research_display.c src/pico8_research_icon.c src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_scroll.c src/pico8_session_recorder.c src/pico8_slipway.c src/pico8_slipway_display.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_value.c src/pico8_vec.c src/pico8_wormhole.c src/pico8_wormhole_display.c third_party/raylib/src/libraylib.a $(RAYLIB_LDLIBS)

raylib: $(TMP_DIR)/main_raylib

# Le visualiseur : même règle que la cible raylib ci-dessus, link seulement.
$(SCREENCAST_VIEWER_BIN): src/main_screencast_viewer.c src/pico8_screencast.c src/pico8_screencast.h src/pico8_screencast_player.c src/pico8_screencast_player.h src/pico8_palette.c src/pico8_palette.h src/pico8_replay_control.c src/pico8_replay_control.h src/pico8_raylib_backend.c src/pico8_raylib_backend.h src/pico8_raylib_backend_coords.c src/pico8_raylib_backend_coords.h src/pico8_session_recorder.c src/pico8_session_recorder.h src/pico8_input.c src/pico8_input.h src/pico8_mem.c src/pico8_mem.h src/pico8_gfx.h third_party/raylib/src/libraylib.a | $(TMP_DIR)
	$(CC) $(CFLAGS) -I third_party/raylib/src -o $@ src/main_screencast_viewer.c src/pico8_screencast.c src/pico8_screencast_player.c src/pico8_palette.c src/pico8_replay_control.c src/pico8_raylib_backend.c src/pico8_raylib_backend_coords.c src/pico8_session_recorder.c src/pico8_input.c src/pico8_mem.c third_party/raylib/src/libraylib.a $(RAYLIB_LDLIBS)

screencast-viewer: $(SCREENCAST_VIEWER_BIN)

# Les fragments de dépendances écrits par le compilateur, un par objet. Le
# wildcard tolère qu'ils n'existent pas : sur un clone frais, tout est compilé.
-include $(wildcard $(OBJ_DIR)/src/*.d $(OBJ_DIR)/tests/*.d)
-include $(wildcard $(OBJ_ASAN_DIR)/src/*.d $(OBJ_ASAN_DIR)/tests/*.d)
-include $(wildcard $(OBJ_UBSAN_DIR)/src/*.d $(OBJ_UBSAN_DIR)/tests/*.d)
-include $(wildcard $(OBJ_REFAUDIT_DIR)/src/*.d $(OBJ_REFAUDIT_DIR)/tests/*.d)
-include $(wildcard $(OBJ_AUDIT_DIR)/src/*.d $(OBJ_AUDIT_DIR)/tests/*.d)
-include $(wildcard $(OBJ_COVERAGE_DIR)/src/*.d $(OBJ_COVERAGE_DIR)/tests/*.d)
-include $(wildcard $(OBJ_E2E_DIR)/src/*.d $(OBJ_E2E_DIR)/tests/*.d)
