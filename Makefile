# Tout tourne en parallèle (-j au nombre de cœurs, sorties regroupées par cible).
#
# make test          -> unit + constants-check + float-grep + float-guard, la
#                       boucle rapide
# make e2e           -> frames-check, les rejeux, en -O2
# make test-all      -> test + e2e, la suite complète
# make unit          -> les binaires de tests/, compilés et lancés
# make replay EVENTS=<events> -> rejoue ce fichier dans le moteur headless, dump
#                       du framebuffer dans tmp/pico8_replay_dump.txt ; hors suite
# make screencast EVENTS=<events> -> refabrique les pixels de chaque frame de ce
#                       rejeu dans tmp/screencast.bin, pour screencast-viewer
# make frames-check  -> compare le sha256 de la capture de chaque rejeu du filet à
#                       sa référence <events>.sha256, dans make e2e
# make frames-baseline -> (re)pose ces références sur le code actuel, jamais par
#                       make test
# make frames-ref    -> pour chaque rejeu divergent, rejoue le commit qui a posé
#                       sa référence (git blame), laisse la capture dans
#                       tmp/frames/<events>.ref.bin et donne la première frame
#                       divergente ; hors suite
# make ref-audit     -> diagnostic des casts REF sur les rejeux du filet
#                       (FRAMES_EVENTS), hors suite
# make class-coverage DIR=<dossier> -> rejoue chaque *events.json posé directement
#                       dans DIR (défaut tests/fixtures, sans sous-dossiers) et
#                       compte les instanciations par classe ; échoue si une
#                       classe est à 0 hors tools/class_coverage_exempt.txt,
#                       hors suite
# make class-coverage-net -> idem sur les rejeus du filet (FRAMES_EVENTS), hors
#                       suite
# make line-coverage EVENTS="<events>..." -> lignes exécutées par ces rejeux
#                       (pilote instrumenté gcov), une `fichier:ligne` par ligne,
#                       dans tmp/line_coverage.txt ; hors suite
# make line-coverage-contrib EVENTS=<events> OTHERS="<events>..." -> les lignes
#                       que EVENTS couvre et qu'aucun de OTHERS ne couvre (le
#                       critère de garde), dans tmp/line_coverage_contrib.txt ;
#                       hors suite
# make line-coverage-reference-check -> recalcule la couverture de lignes du
#                       filet (FRAMES_EVENTS) et la compare à la référence figée
#                       tools/line_coverage_reference.txt, plusieurs minutes,
#                       hors suite
# make e2e-fast      -> frames-check sur les nouveaux e2e de tests/e2e/, depuis
#                       l'écran titre, avec le binaire -O0 de make test ;
#                       échoue aussi si un e2e n'a pas son .txt et son
#                       .json.sha256 ou dépasse 180 frames (600 s'il est listé
#                       dans tools/e2e_long_exceptions.txt) ; hors suite
# make e2e-fast-baseline -> (re)pose les .json.sha256 de tests/e2e/
# make e2e-gap       -> les lignes de tools/line_coverage_reference.txt
#                       qu'e2e-fast ne couvre pas, moins
#                       tools/line_coverage_unreachable.txt ; échoue tant que
#                       l'écart n'est pas nul, hors suite
# make minimize EVENTS=<events> OUT=<sortie> -> minimise ce rejeu (joué depuis
#                       l'écran titre, SHORTCUT=1 pour le raccourci) : étage
#                       mécanique (tools/minimize.sh), puis passe guidée tant
#                       que la couverture de lignes reste celle de l'entrée,
#                       un rejeu gcov par candidat, lent ; MECHANICAL=1 :
#                       l'étage mécanique seul ; MIN_CHUNK=<n> : tranches
#                       d'au moins n frames ; hors suite
# make e2e-cleanup   -> la passe de nettoyage finale sur tests/e2e/ : liste,
#                       du plus long au plus court, les e2e dont les lignes
#                       sont couvertes par les autres restants ; ne supprime
#                       rien, hors suite
# make e2e-judge EVENTS=<events> -> les lignes que ce rejeu (depuis le titre)
#                       ajoute aux e2e de tests/e2e/ ; échoue s'il n'en ajoute
#                       aucune, hors suite
# make constants-check -> compare chaque PICO8_FIXED_LIT du cœur à son hexa relevé
#                       (tools/probe_out.txt) ; échoue si les écarts ne sont pas
#                       exactement tools/constants_expected_ecarts.txt, dans make test
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
OBJ_COVERAGE_DIR := $(TMP_DIR)/obj-coverage
OBJ_E2E_DIR := $(TMP_DIR)/obj-e2e
OBJ_GCOV_DIR := $(TMP_DIR)/obj-gcov

# -MMD -MP : le compilateur écrit, à côté de chaque objet, la liste des en-têtes
# que l'unité de compilation a vraiment ouverts. Le -include en bas du fichier
# les relit. Aucun en-tête n'est donc nommé à la main dans une liste de prérequis.
DEPFLAGS := -MMD -MP
ASAN_CFLAGS := -fsanitize=address -fno-omit-frame-pointer
UBSAN_CFLAGS := -fsanitize=undefined -fno-sanitize-recover=undefined
REFAUDIT_CFLAGS := -DPICO8_REF_AUDIT
COVERAGE_CFLAGS := -DPICO8_CLASS_COVERAGE
# Rejeux de make e2e seulement ; les TU restent en -O0, lisibles sous gdb.
E2E_CFLAGS := -O2
GCOV_CFLAGS := --coverage

# $(call objs,<liste de .c>) -> les chemins des .o correspondants dans l'arbre
# concerné. Une cible nomme ses sources une fois ; ses prérequis et sa ligne de
# link en découlent.
objs = $(patsubst %.c,$(OBJ_DIR)/%.o,$(1))
asan_objs = $(patsubst %.c,$(OBJ_ASAN_DIR)/%.o,$(1))
ubsan_objs = $(patsubst %.c,$(OBJ_UBSAN_DIR)/%.o,$(1))
refaudit_objs = $(patsubst %.c,$(OBJ_REFAUDIT_DIR)/%.o,$(1))
coverage_objs = $(patsubst %.c,$(OBJ_COVERAGE_DIR)/%.o,$(1))
e2e_objs = $(patsubst %.c,$(OBJ_E2E_DIR)/%.o,$(1))
gcov_objs = $(patsubst %.c,$(OBJ_GCOV_DIR)/%.o,$(1))

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

$(OBJ_COVERAGE_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(COVERAGE_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_E2E_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(E2E_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_GCOV_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(GCOV_CFLAGS) $(DEPFLAGS) -c -o $@ $<

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

$(TMP_DIR)/test_pico8_replay_usage: $(call objs,tests/test_pico8_replay_usage.c) $(TMP_DIR)/pico8_replay | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/pico8_frames: $(call objs,src/main_frames.c src/pico8_screencast.c src/pico8_dump.c src/pico8_mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Lance le pilote de rejeu puis l'outil d'émission de frames.
$(TMP_DIR)/test_pico8_frames_tool: $(call objs,tests/test_pico8_frames_tool.c src/pico8_screencast.c src/pico8_mem.c) $(TMP_DIR)/pico8_replay $(TMP_DIR)/pico8_frames tests/fixtures/screencast_capture_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_frames_check: $(call objs,tests/test_frames_check.c) tools/frames_check.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_class_coverage: $(call objs,tests/test_class_coverage.c) $(TMP_DIR)/pico8_class_coverage tools/class_coverage.sh tests/fixtures/screencast_capture_events.json tests/fixtures/events_malformed_missing_field.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_line_coverage: $(call objs,tests/test_line_coverage.c) $(TMP_DIR)/pico8_replay_gcov tools/line_coverage.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_e2e_limits: $(call objs,tests/test_e2e_limits.c) tools/e2e_limits.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_e2e_cleanup: $(call objs,tests/test_e2e_cleanup.c) tools/e2e_cleanup.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/pico8_minimize: $(call objs,src/main_minimize.c src/pico8_minimize.c src/pico8_input.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_minimize: $(call objs,tests/test_pico8_minimize.c src/pico8_minimize.c src/pico8_input.c) $(TMP_DIR)/pico8_minimize tools/minimize.sh | $(TMP_DIR)
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

$(TMP_DIR)/test_pico8_stash: $(call objs,tests/test_pico8_stash.c src/pico8_stash.c src/pico8_str.c src/pico8_ob.c src/pico8_list.c src/pico8_value.c src/pico8_fixed.c src/pico8_mem.c src/pico8_assets.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_data_init: $(call objs,tests/test_pico8_data_init.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
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

$(TMP_DIR)/pico8_replay_gcov: $(call gcov_objs,$(REPLAY_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(GCOV_CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_pico8_shop_tooltip_fillp: $(call objs,tests/test_pico8_shop_tooltip_fillp.c $(ENGINE_SRCS)) tests/fixtures/shop_button_hover_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_bg_kj: $(call objs,tests/test_pico8_bg_kj.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# Sous ASan, et sans jumeau non instrumenté : le crash visé dépend de la
# disposition de l'allocateur (le même rejeu sort 139 en -O0 et 0 en -O1), donc
# seul le débordement lui-même est un oracle fiable. detect_leaks=0 au lancement,
# ce port fuit par conception.
$(TMP_DIR)/test_pico8_planet_gh_row_lq: $(call asan_objs,tests/test_pico8_planet_gh_row_lq.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_ga_export_panel_by: $(call objs,tests/test_pico8_ga_export_panel_by.c $(DATA_ENGINE_SRCS) src/pico8_planet_family.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

$(TMP_DIR)/test_pico8_rich_planet_exports: $(call objs,tests/test_pico8_rich_planet_exports.c $(ENGINE_SRCS)) tests/fixtures/built_planet_panel_events.json | $(TMP_DIR)
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
    test_pico8_raylib_backend_coords \
    test_pico8_replay_source \
    test_pico8_recording_continuity \
    test_pico8_replay_control \
    test_pico8_marker_coherence \
    test_pico8_screencast \
    test_pico8_headless_capture \
    test_pico8_replay_from_title \
    test_pico8_replay_usage \
    test_pico8_screencast_player \
    test_pico8_frames_tool \
    test_frames_check \
    test_class_coverage \
    test_line_coverage \
    test_e2e_limits \
    test_pico8_minimize \
    test_e2e_cleanup \
    test_float_guard \
    test_float_grep \
    test_float_link \
    test_constants_check \
    test_pico8_bg_kj \
    test_pico8_probe_mm_rnd_order \
    test_pico8_shop_tooltip_fillp \
    test_pico8_planet_gh_row_lq \
    test_pico8_ga_export_panel_by \
    test_pico8_layout_nil_item_draws_nothing \
    test_pico8_rich_planet_exports

# Un lancement par TU, en cible à part : c'est ce que -j parallélise.
UNIT_RUNS := $(addprefix unit-run-,$(UNIT_TESTS))

UNIT_RUNS_SPECIAL := unit-run-test_pico8_planet_gh_row_lq

unit: $(UNIT_RUNS)

$(filter-out $(UNIT_RUNS_SPECIAL),$(UNIT_RUNS)): unit-run-%: $(TMP_DIR)/%
	./$<

unit-run-test_pico8_planet_gh_row_lq: $(TMP_DIR)/test_pico8_planet_gh_row_lq
	ASAN_OPTIONS=detect_leaks=0 ./$<

# Ces deux TU relisent tmp/pico8_replay_dump.txt, le dump par défaut de
# pico8_replay : sous -j, jamais en même temps.
unit-run-test_pico8_frames_tool: | unit-run-test_pico8_headless_capture

.PHONY: $(UNIT_RUNS)

# Échoue avant toute compilation plutôt que de rejouer un fichier par défaut.
ifneq ($(filter replay screencast,$(MAKECMDGOALS)),)
    ifeq ($(strip $(EVENTS)),)
        $(error EVENTS manque : make $(filter replay screencast,$(MAKECMDGOALS)) EVENTS=<events.json>)
    endif
endif

replay: $(TMP_DIR)/pico8_replay_e2e
	./$(TMP_DIR)/pico8_replay_e2e $(call from_title,$(EVENTS)) $(EVENTS)

test: unit constants-check float-grep float-guard

e2e: frames-check

test-all: test e2e

SCREENCAST_BIN := $(TMP_DIR)/screencast.bin

SCREENCAST_VIEWER_BIN := $(TMP_DIR)/screencast_viewer

# Les enregistrements raylib cliquent eux-mêmes la difficulté sur l'écran
# titre : tout pilote qui les rejoue passe $(call from_title,<events>).
RECORDINGS := \
    tests/fixtures/session_events.json \
    tests/fixtures/long_session_events.json \
    tests/fixtures/events_recorded_1790198294.json \
    tests/fixtures/trading_hub_session_events.json
# Les nouveaux e2e : <nom>.json (même format qu'un enregistrement raylib),
# <nom>.txt (ce que fait le rejeu, frames des actions clés) et
# <nom>.json.sha256. Tous partent de l'écran titre.
E2E_FAST_DIR := tests/e2e
E2E_FAST_EVENTS := $(sort $(wildcard $(E2E_FAST_DIR)/*.json))
# Une ligne `<events> raison` par e2e admis jusqu'à 600 frames au lieu de 180.
E2E_FAST_EXCEPTIONS := tools/e2e_long_exceptions.txt

from_title = $(if $(filter $(RECORDINGS) $(E2E_FAST_EVENTS),$(1)),--from-title)

ifeq ($(UNAME_S),Darwin)
    SHA256_CMD := shasum -a 256
else
    SHA256_CMD := sha256sum
endif

# Refaite à chaque appel : EVENTS change d'un appel à l'autre sans que make le
# voie. Cible fichier plutôt que phony, pour que .DELETE_ON_ERROR s'applique.
$(SCREENCAST_BIN): $(TMP_DIR)/pico8_replay_e2e FORCE | $(TMP_DIR)
	./$(TMP_DIR)/pico8_replay_e2e $(call from_title,$(EVENTS)) $(EVENTS) $@ $(@:.bin=.dump.txt)

screencast: $(SCREENCAST_BIN)

.PHONY: FORCE
FORCE:

# Une capture interrompue laisserait un fichier partiel que make croirait à jour,
# et l'empreinte suivante porterait sur lui.
.DELETE_ON_ERROR:

# FILET (docs/plan-2.md §5) : le sha256 de la capture de chaque fichier
# d'événements, versionné à côté de lui sous <events>.sha256. La capture est la
# concaténation brute des frames : son empreinte couvre aussi leur nombre.
# Liste explicite : un motif sur tests/fixtures/ ramasserait les fixtures
# malformées et celles du lecteur d'entrée.
FRAMES_EVENTS := \
    tests/fixtures/session_events.json \
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

frames_capture = $(FRAMES_DIR)/$(1).bin
frames_current = $(FRAMES_DIR)/$(1).sha256
frames_reference = $(1).sha256

# Sans .SECONDARY, make effacerait ces captures intermédiaires, que frames-ref
# compare à la capture de référence.
.SECONDARY: $(foreach e,$(FRAMES_EVENTS),$(call frames_capture,$(e)))

# Chaque capture a son propre dump texte : le dump par défaut,
# tmp/pico8_replay_dump.txt, est relu par deux TU qui tournent en parallèle.
$(FRAMES_DIR)/%.bin: % $(TMP_DIR)/pico8_replay_e2e
	@mkdir -p $(@D)
	./$(TMP_DIR)/pico8_replay_e2e $(call from_title,$<) $< $@ $(@:.bin=.dump.txt) > /dev/null

# L'affectation porte le code de sortie du sha256, qu'un pipe vers cut perdrait.
frames_hash = @mkdir -p $(@D); h=$$($(SHA256_CMD) $<) && echo "$${h%% *}" > $@

$(FRAMES_DIR)/%.sha256: $(FRAMES_DIR)/%.bin
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

# Le binaire -O0 de make test, pas celui de make e2e : pas de second arbre à
# compiler, et lisible sous gdb.
E2E_FAST_FRAMES_DIR := $(TMP_DIR)/frames-fast

.SECONDARY: $(foreach e,$(E2E_FAST_EVENTS),$(E2E_FAST_FRAMES_DIR)/$(e).bin)

$(E2E_FAST_FRAMES_DIR)/%.bin: % $(TMP_DIR)/pico8_replay
	@mkdir -p $(@D)
	./$(TMP_DIR)/pico8_replay $(call from_title,$<) $< $@ $(@:.bin=.dump.txt) > /dev/null

$(E2E_FAST_FRAMES_DIR)/%.sha256: $(E2E_FAST_FRAMES_DIR)/%.bin
	$(frames_hash)

e2e-fast: $(foreach e,$(E2E_FAST_EVENTS),$(E2E_FAST_FRAMES_DIR)/$(e).sha256) tools/e2e_limits.sh tools/frames_check.sh $(E2E_FAST_EXCEPTIONS)
	@status=0; \
	sh tools/e2e_limits.sh $(E2E_FAST_EXCEPTIONS) $(E2E_FAST_EVENTS) || status=1; \
	FRAMES_BASELINE=e2e-fast-baseline sh tools/frames_check.sh $(foreach e,$(E2E_FAST_EVENTS),$(e) $(call frames_reference,$(e)) $(E2E_FAST_FRAMES_DIR)/$(e).sha256) || status=1; \
	echo "e2e-fast: $(words $(E2E_FAST_EVENTS)) e2e dans $(E2E_FAST_DIR)"; \
	exit $$status

e2e-fast-baseline: $(foreach e,$(E2E_FAST_EVENTS),$(E2E_FAST_FRAMES_DIR)/$(e).sha256)
	@$(foreach e,$(E2E_FAST_EVENTS),cp $(E2E_FAST_FRAMES_DIR)/$(e).sha256 $(call frames_reference,$(e)) && echo "e2e-fast: reference posee dans $(call frames_reference,$(e))" && ) true

.PHONY: e2e-fast e2e-fast-baseline

# make ref-audit : un diagnostic, pas un test, et délibérément hors de `test`.
# Il répond « quels sites de cast REF ont été atteints, et avec quel tag », en
# rejouant les binaires ci-dessous compilés avec -DPICO8_REF_AUDIT.
$(TMP_DIR)/pico8_replay_refaudit: $(call refaudit_objs,src/main.c src/pico8_screencast.c $(ENGINE_SRCS)) | $(TMP_DIR)
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
    $(TMP_DIR)/test_pico8_planet_gh_row_lq_refaudit \
    $(TMP_DIR)/test_pico8_data_init_refaudit \
    $(TMP_DIR)/test_pico8_planet_kj_dispatch_refaudit \
    $(TMP_DIR)/test_pico8_ga_export_panel_by_refaudit

# Chaque binaire ajoute son décompte à un fichier ; le script les additionne et
# compare aux sites qu'il grep dans src/, pour qu'un site jamais atteint sorte en
# NOT COVERED au lieu de manquer en silence.
ref-audit: $(REFAUDIT_BINS)
	sh tools/ref_audit_report.sh $(foreach e,$(FRAMES_EVENTS),$(call from_title,$(e)) $(e))

.PHONY: ref-audit

# Seul pico8_class.c change sous le define : le reste vient de l'arbre par
# défaut, et aucun objet de l'arbre coverage n'est lié ailleurs.
$(TMP_DIR)/pico8_class_coverage: $(call coverage_objs,src/main_class_coverage.c src/pico8_class.c) $(call objs,$(filter-out src/pico8_class.c,$(ENGINE_SRCS))) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

DIR ?= tests/fixtures
CLASS_COVERAGE_EXEMPT := tools/class_coverage_exempt.txt

class_coverage_recordings = $(foreach e,$(RECORDINGS),--from-title $(e))

class-coverage: $(TMP_DIR)/pico8_class_coverage tools/class_coverage.sh $(CLASS_COVERAGE_EXEMPT)
	@sh tools/class_coverage.sh $(class_coverage_recordings) ./$(TMP_DIR)/pico8_class_coverage '$(DIR)' $(CLASS_COVERAGE_EXEMPT)

class-coverage-net: $(TMP_DIR)/pico8_class_coverage tools/class_coverage.sh $(CLASS_COVERAGE_EXEMPT)
	@sh tools/class_coverage.sh $(class_coverage_recordings) ./$(TMP_DIR)/pico8_class_coverage - $(CLASS_COVERAGE_EXEMPT) $(FRAMES_EVENTS)

.PHONY: class-coverage class-coverage-net

LINE_COVERAGE := sh tools/line_coverage.sh
LINE_COVERAGE_COVER := $(LINE_COVERAGE) cover ./$(TMP_DIR)/pico8_replay_gcov $(OBJ_GCOV_DIR)
# Écrite une fois, sur le filet qui contenait encore les longs enregistrements
# et le raccourci de difficulté : on ne la recalcule pas, on la compare.
LINE_COVERAGE_REFERENCE := tools/line_coverage_reference.txt
# Une ligne `fichier:ligne raison` par ligne de la référence qu'aucun rejeu
# court n'atteint, éditée à la main.
LINE_COVERAGE_UNREACHABLE := tools/line_coverage_unreachable.txt

line_coverage_args = $(foreach e,$(1),$(call from_title,$(e)) $(e))

line-coverage: $(TMP_DIR)/pico8_replay_gcov tools/line_coverage.sh
	@$(LINE_COVERAGE_COVER) $(call line_coverage_args,$(EVENTS)) > $(TMP_DIR)/line_coverage.txt
	@echo "couverture de lignes: $$(wc -l < $(TMP_DIR)/line_coverage.txt) ligne(s) dans $(TMP_DIR)/line_coverage.txt"

line-coverage-contrib: $(TMP_DIR)/pico8_replay_gcov tools/line_coverage.sh
	@$(LINE_COVERAGE_COVER) $(call line_coverage_args,$(EVENTS)) > $(TMP_DIR)/line_coverage_self.txt
	@$(if $(strip $(OTHERS)),$(LINE_COVERAGE_COVER) $(call line_coverage_args,$(OTHERS)),:) > $(TMP_DIR)/line_coverage_others.txt
	@$(LINE_COVERAGE) contrib $(TMP_DIR)/line_coverage_self.txt $(TMP_DIR)/line_coverage_others.txt > $(TMP_DIR)/line_coverage_contrib.txt
	@echo "apport de $(EVENTS): $$(wc -l < $(TMP_DIR)/line_coverage_contrib.txt) ligne(s) dans $(TMP_DIR)/line_coverage_contrib.txt"

line-coverage-reference-check: $(TMP_DIR)/pico8_replay_gcov tools/line_coverage.sh $(LINE_COVERAGE_REFERENCE)
	@$(LINE_COVERAGE_COVER) $(call line_coverage_args,$(FRAMES_EVENTS)) > $(TMP_DIR)/line_coverage_net.txt
	@$(LINE_COVERAGE) diff $(LINE_COVERAGE_REFERENCE) $(TMP_DIR)/line_coverage_net.txt > $(TMP_DIR)/line_coverage_lost.txt
	@$(LINE_COVERAGE) diff $(TMP_DIR)/line_coverage_net.txt $(LINE_COVERAGE_REFERENCE) > $(TMP_DIR)/line_coverage_new.txt
	@if [ -s $(TMP_DIR)/line_coverage_lost.txt ] || [ -s $(TMP_DIR)/line_coverage_new.txt ]; then \
	    sed 's/^/référence seulement: /' $(TMP_DIR)/line_coverage_lost.txt; \
	    sed 's/^/filet seulement: /' $(TMP_DIR)/line_coverage_new.txt; \
	    echo "référence: ÉCHEC -- le filet ne rend plus $(LINE_COVERAGE_REFERENCE)"; exit 1; \
	fi
	@echo "référence: OK -- le filet rend les $$(wc -l < $(LINE_COVERAGE_REFERENCE)) ligne(s) de $(LINE_COVERAGE_REFERENCE)"

# Sans e2e, l'ensemble couvert est vide : l'écart est toute la référence.
e2e-gap: $(TMP_DIR)/pico8_replay_gcov tools/line_coverage.sh $(LINE_COVERAGE_REFERENCE) $(LINE_COVERAGE_UNREACHABLE)
	@$(if $(E2E_FAST_EVENTS),$(LINE_COVERAGE_COVER) $(call line_coverage_args,$(E2E_FAST_EVENTS)),:) > $(TMP_DIR)/line_coverage_e2e_fast.txt
	@$(LINE_COVERAGE) gap $(LINE_COVERAGE_REFERENCE) $(LINE_COVERAGE_UNREACHABLE) $(TMP_DIR)/line_coverage_e2e_fast.txt > $(TMP_DIR)/line_coverage_gap.txt
	@sed 's/^/manque: /' $(TMP_DIR)/line_coverage_gap.txt
	@n=$$(wc -l < $(TMP_DIR)/line_coverage_gap.txt | tr -d ' '); \
	if [ "$$n" -ne 0 ]; then \
	    echo "écart: $$n ligne(s) de $(LINE_COVERAGE_REFERENCE) ni couvertes par e2e-fast ($(words $(E2E_FAST_EVENTS)) e2e) ni dans $(LINE_COVERAGE_UNREACHABLE), dans $(TMP_DIR)/line_coverage_gap.txt"; \
	    exit 1; \
	fi
	@echo "écart: nul -- e2e-fast couvre toute la référence hors $(LINE_COVERAGE_UNREACHABLE)"

.PHONY: line-coverage line-coverage-contrib line-coverage-reference-check e2e-gap

ifneq ($(filter minimize,$(MAKECMDGOALS)),)
    ifeq ($(and $(strip $(EVENTS)),$(strip $(OUT))),)
        $(error EVENTS ou OUT manque : make minimize EVENTS=<events.json> OUT=<sortie.json>)
    endif
endif
ifneq ($(filter e2e-judge,$(MAKECMDGOALS)),)
    ifeq ($(strip $(EVENTS)),)
        $(error EVENTS manque : make e2e-judge EVENTS=<events.json>)
    endif
endif

# Le rejeu d'un fichier à minimiser part de l'écran titre, comme tout
# nouvel e2e ; SHORTCUT=1 garde le raccourci de difficulté.
MINIMIZE_ORACLE := $(LINE_COVERAGE_COVER) $(if $(SHORTCUT),,--from-title)

minimize: $(TMP_DIR)/pico8_minimize $(TMP_DIR)/pico8_replay_gcov tools/minimize.sh tools/line_coverage.sh
	@sh tools/minimize.sh ./$(TMP_DIR)/pico8_minimize $(if $(MIN_CHUNK),--min-chunk $(MIN_CHUNK)) $(EVENTS) $(OUT) $(if $(MECHANICAL),,$(MINIMIZE_ORACLE))
	@echo "minimize: $(EVENTS) -> $(OUT), $$(./$(TMP_DIR)/pico8_minimize count $(OUT)) frame(s)"

E2E_CLEANUP_DIR := $(TMP_DIR)/e2e_cleanup

e2e-cleanup: $(TMP_DIR)/pico8_minimize $(TMP_DIR)/pico8_replay_gcov tools/e2e_cleanup.sh tools/line_coverage.sh
	@rm -rf $(E2E_CLEANUP_DIR) && mkdir -p $(E2E_CLEANUP_DIR)
	@$(foreach e,$(E2E_FAST_EVENTS),$(LINE_COVERAGE_COVER) $(call line_coverage_args,$(e)) > $(E2E_CLEANUP_DIR)/$(notdir $(e)).lines && ) true
	@$(if $(E2E_FAST_EVENTS),sh tools/e2e_cleanup.sh $(foreach e,$(E2E_FAST_EVENTS),$(e) $$(./$(TMP_DIR)/pico8_minimize count $(e)) $(E2E_CLEANUP_DIR)/$(notdir $(e)).lines),:) > $(E2E_CLEANUP_DIR)/remove.txt
	@sed 's/^/à retirer: /' $(E2E_CLEANUP_DIR)/remove.txt
	@echo "e2e-cleanup: $$(wc -l < $(E2E_CLEANUP_DIR)/remove.txt | tr -d ' ') fichier(s) à retirer sur $(words $(E2E_FAST_EVENTS)), rien n'est supprimé"

e2e-judge: $(TMP_DIR)/pico8_replay_gcov tools/line_coverage.sh
	@$(LINE_COVERAGE_COVER) $(if $(SHORTCUT),,--from-title) $(EVENTS) > $(TMP_DIR)/e2e_judge_self.txt
	@$(if $(filter-out $(EVENTS),$(E2E_FAST_EVENTS)),$(LINE_COVERAGE_COVER) $(call line_coverage_args,$(filter-out $(EVENTS),$(E2E_FAST_EVENTS))),:) > $(TMP_DIR)/e2e_judge_others.txt
	@$(LINE_COVERAGE) contrib $(TMP_DIR)/e2e_judge_self.txt $(TMP_DIR)/e2e_judge_others.txt > $(TMP_DIR)/e2e_judge_contrib.txt
	@n=$$(wc -l < $(TMP_DIR)/e2e_judge_contrib.txt | tr -d ' '); \
	echo "e2e-judge: $(EVENTS) ajoute $$n ligne(s) à e2e-fast, dans $(TMP_DIR)/e2e_judge_contrib.txt"; \
	[ "$$n" -gt 0 ]

.PHONY: minimize e2e-cleanup e2e-judge

# Les sources du cœur, où le comparateur lit les appels PICO8_FIXED_LIT.
constants-check: tools/constants_check.sh tools/constants_compare.sh tools/constants_expected_ecarts.txt tools/probe_out.txt src/pico8_fixed.c src/pico8_fixed.h $(CORE_SRCS) | $(TMP_DIR)
	@sh tools/constants_check.sh tools/constants_expected_ecarts.txt tools/probe_out.txt src/pico8_fixed.c $(TMP_DIR)/constants_check \
	    $(CORE_SRCS)

.PHONY: constants-check

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
-include $(wildcard $(OBJ_COVERAGE_DIR)/src/*.d $(OBJ_COVERAGE_DIR)/tests/*.d)
-include $(wildcard $(OBJ_E2E_DIR)/src/*.d $(OBJ_E2E_DIR)/tests/*.d)
-include $(wildcard $(OBJ_GCOV_DIR)/src/*.d $(OBJ_GCOV_DIR)/tests/*.d)
