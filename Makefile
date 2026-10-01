# Tout tourne en parallèle (-j au nombre de cœurs, sorties regroupées par cible).
#
# make test          -> unit + constants-check + float-grep + float-guard +
#                       alloc-guard + data-guard + composition-guard, la boucle
#                       rapide
# make e2e           -> rejoue chaque e2e de tests/e2e/ depuis l'écran titre,
#                       avec le binaire -O0 de make test, et compare le sha256
#                       de sa capture à <nom>.json.sha256 ; échoue aussi si un
#                       e2e n'a pas son .txt et son .json.sha256 ou dépasse
#                       180 frames (600 s'il est listé dans
#                       tools/e2e_long_exceptions.txt)
# make test-all      -> test + e2e, la suite complète
# make gif           -> gif/<nom>.gif pour chaque e2e de tests/e2e/ : sa capture
#                       à l'échelle 3, la souris de l'event dessinée par-dessus ;
#                       hors suite
# make unit          -> les binaires de tests/, compilés et lancés
# make replay EVENTS=<events> -> rejoue ce fichier dans le moteur headless, dump
#                       du framebuffer dans tmp/pico8_replay_dump.txt ; hors suite
# make screencast EVENTS=<events> -> refabrique les pixels de chaque frame de ce
#                       rejeu dans tmp/screencast.bin, pour screencast-viewer
# make e2e-baseline  -> (re)pose les .json.sha256 de tests/e2e/ sur le code
#                       actuel, jamais par make test
# make e2e-ref       -> pour chaque e2e divergent, rejoue le commit qui a posé
#                       sa référence (git blame), laisse la capture dans
#                       tmp/frames/<events>.ref.bin et donne la première frame
#                       divergente ; hors suite
# make class-coverage [DIR=<dossier>] -> rejoue les e2e de tests/e2e/ (avec
#                       DIR : chaque *events.json posé directement dans DIR,
#                       sans sous-dossiers) et compte les instanciations par
#                       classe ; échoue si une classe est à 0 hors
#                       tools/class_coverage_exempt.txt, hors suite
# make line-coverage EVENTS="<events>..." -> lignes exécutées par ces rejeux
#                       (pilote instrumenté gcov), une `fichier:ligne` par ligne,
#                       dans tmp/line_coverage.txt ; hors suite
# make line-coverage-contrib EVENTS=<events> OTHERS="<events>..." -> les lignes
#                       que EVENTS couvre et qu'aucun de OTHERS ne couvre (le
#                       critère de garde), dans tmp/line_coverage_contrib.txt ;
#                       hors suite
# make e2e-gap       -> les lignes de tools/line_coverage_reference.txt
#                       que make e2e ne couvre pas, moins
#                       tools/line_coverage_unreachable.txt ; échoue tant que
#                       l'écart n'est pas nul, hors suite
# make minimize EVENTS=<events> OUT=<sortie> -> minimise ce rejeu (joué depuis
#                       l'écran titre) : étage
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
# make class-coverage-net -> idem sur les e2e de tests/e2e/ ; échoue aussi si
#                       sa sortie diffère de tools/class_coverage_net_expected.txt,
#                       hors suite
# make data-residue  -> compte le résidu de données par catégorie
#                       (tools/data_residue.sh) ; la phase D sort sur une sortie
#                       vide, hors suite
# make composition-residue -> compte le résidu de composition par catégorie
#                       (tools/composition_residue.sh) ; la phase 2 sort sur
#                       une sortie vide, hors suite
# make data-guard    -> aucun offsetof dans src/, catégories de
#                       DATA_RESIDUE_GUARD à 0, dans make test
# make composition-guard -> catégories de COMPOSITION_RESIDUE_GUARD à 0, dans make test
# make sanitize-net  -> rejoue les e2e de tests/e2e/ sous ASan+UBSan (-O2),
#                       critère « sortie propre », hors suite
# make constants-check -> compare chaque PICO8_FIXED_LIT du cœur à son hexa relevé
#                       (tools/pico8_reference_probe_out.txt) ; échoue si les écarts ne sont pas
#                       exactement tools/constants_expected_gaps.txt, dans make test
# make float-guard   -> réécrit tools/float_guard.txt, en échec si un fichier
#                       du cœur a besoin du flottant (§6.3), dans make test
# make alloc-guard   -> compte les appels malloc/calloc/realloc/free de chaque
#                       objet du cœur ; échoue sur tout écart, plus haut ou plus
#                       bas, avec tools/alloc_guard_exceptions.txt (cliquet, mis
#                       à jour à la main), dans make test
# make float-grep    -> grep du §6.1 sur le cœur (CORE_SRCS + en-têtes, -MM), le
#                       reste de src/ exclu par tools/float_grep_exclusions.txt,
#                       dans make test
# make float-link    -> lie les objets de tmp/pico8_replay sans -lm (§6.2) dans un
#                       binaire jetable, jamais lancé ; hors suite : make test
#                       construit déjà tmp/pico8_replay, même lien
# make sin-table     -> régénère src/p8/quarter_wave.h (table de sin/cos du
#                       cœur) depuis tools/gen_quarter_wave.c, hors suite
# make raylib        -> compile et LINK le binaire interactif, ne le lance jamais
# make screencast-viewer -> idem pour le visualiseur d'enregistrement
#                       (les deux exigent make raylib-vendor-setup au préalable)
# make psx-hello     -> projet PSX de test (nécessite psn00bsdk-toolchain-setup)
# make rename-tool-setup -> installe les bindings Python de libclang
#                       (tools/ast_rename) dans third_party/clang-python,
#                       réseau, une seule fois, hors suite
# make rename-tool-test -> les tests de tools/ast_rename sur le mini-arbre
#                       tests/fixtures/ast_rename ; exige rename-tool-setup,
#                       hors suite
# make rename-extract -> chaque déclaration de src/, tests/, tools/ et ses
#                       occurrences, dans les configurations défaut, couverture
#                       et ASan : tmp/ast_rename/declarations.tsv et
#                       occurrences.tsv ; échoue si une unité ne s'analyse pas,
#                       hors suite
# make rename-derive-test -> les tests de tools/rename_derive.py sur un
#                       mini-vocabulaire et une mini-extraction, dans make test
# make rename-derive -> dérive les noms neufs de chaque déclaration extraite
#                       (make rename-extract d'abord) : lignes identifiant de
#                       docs/renommage.tsv, reste dans
#                       tmp/ast_rename/remainder.tsv, hors suite
# make rename-strings-test -> les tests de tools/rename_strings.py sur un
#                       mini-arbre, dans make test
# make rename-strings -> lignes chaîne et fichier de docs/renommage.tsv (clés,
#                       symboles cités, noms de test, binaires de tmp/), un
#                       site écrivain ou lecteur par fichier ; signalements
#                       dans tmp/ast_rename/string_flags.tsv, hors suite
# make rename-check-test -> les tests de tools/rename_check.py sur une
#                       mini-extraction, dans make test
# make rename-table-check -> contrôle mécanique de docs/renommage.tsv contre
#                       l'extraction (make rename-extract, rename-derive) :
#                       exhaustivité, collisions, masquages, mots réservés,
#                       noms cibles de fichiers ; échoue sur une violation,
#                       hors suite
# make rename-slice-test -> les tests de tools/rename_slice.py, dans make test
# make rename-slice MODULES="<module>..." -> les lignes identifiant renommées de
#                       ces modules de docs/renommage.tsv (motifs du shell sur
#                       la colonne module : game/planet*), au format de
#                       rename-apply, dans tmp/ast_rename/slice.tsv, hors suite
# make rename-apply TABLE=<table> -> renomme aux positions de l'AST les entrées
#                       de la table (ancien, nouveau, fichier[, identité]) ;
#                       n'écrit rien au premier refus, rapport dans
#                       tmp/ast_rename/report.txt, hors suite

.PHONY: psn00bsdk-toolchain-setup psx-hello psx-hello-clean unit replay test e2e test-all gif raylib-vendor-setup raylib screencast screencast-viewer e2e-baseline e2e-ref rename-tool-setup rename-tool-test rename-extract rename-apply rename-derive rename-derive-test rename-strings rename-strings-test rename-check-test rename-table-check rename-slice rename-slice-test

NPROC := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
MAKEFLAGS += -j$(NPROC)
ifneq ($(filter output-sync,$(.FEATURES)),)
    MAKEFLAGS += --output-sync=target
endif

CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Werror -g
override CFLAGS += -Isrc

TMP_DIR := tmp

# Un arbre d'objets par jeu de flags. Un objet compilé sans -fsanitize n'est pas
# instrumenté : le partager avec une cible qui l'est rendrait le binaire aveugle
# sur ce code, sans rien dire. Ne pas fusionner les six.
OBJ_DIR := $(TMP_DIR)/obj
OBJ_ASAN_DIR := $(TMP_DIR)/obj-asan
OBJ_UBSAN_DIR := $(TMP_DIR)/obj-ubsan
OBJ_COVERAGE_DIR := $(TMP_DIR)/obj-coverage
OBJ_GCOV_DIR := $(TMP_DIR)/obj-gcov
OBJ_SANITIZE_DIR := $(TMP_DIR)/obj-sanitize

# -MMD -MP : le compilateur écrit, à côté de chaque objet, la liste des en-têtes
# que l'unité de compilation a vraiment ouverts. Le -include en bas du fichier
# les relit. Aucun en-tête n'est donc nommé à la main dans une liste de prérequis.
DEPFLAGS := -MMD -MP
ASAN_CFLAGS := -fsanitize=address -fno-omit-frame-pointer
UBSAN_CFLAGS := -fsanitize=undefined -fno-sanitize-recover=undefined
COVERAGE_CFLAGS := -DPICO8_CLASS_COVERAGE
GCOV_CFLAGS := --coverage
SANITIZE_OPT_CFLAGS := -O2

# $(call objs,<liste de .c>) -> les chemins des .o correspondants dans l'arbre
# concerné. Une cible nomme ses sources une fois ; ses prérequis et sa ligne de
# link en découlent.
objs = $(patsubst %.c,$(OBJ_DIR)/%.o,$(1))
asan_objs = $(patsubst %.c,$(OBJ_ASAN_DIR)/%.o,$(1))
ubsan_objs = $(patsubst %.c,$(OBJ_UBSAN_DIR)/%.o,$(1))
coverage_objs = $(patsubst %.c,$(OBJ_COVERAGE_DIR)/%.o,$(1))
gcov_objs = $(patsubst %.c,$(OBJ_GCOV_DIR)/%.o,$(1))
sanitize_objs = $(patsubst %.c,$(OBJ_SANITIZE_DIR)/%.o,$(1))

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

$(OBJ_COVERAGE_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(COVERAGE_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_GCOV_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(GCOV_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(OBJ_SANITIZE_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) $(UBSAN_CFLAGS) $(SANITIZE_OPT_CFLAGS) $(DEPFLAGS) -c -o $@ $<

# Closure complète du moteur : tout src/ sauf les deux points d'entrée (cmd/replay.c,
# cmd/game.c) et les modules qui ne servent qu'au driver interactif. Utilisée
# par toute cible qui boote un cart, pico8_cart_load() tirant tout le port.
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

# Le cœur porté, sans flottant : la closure du moteur moins les outils
# d'hôte, qui gardent la libc. Inventaire, couverture et garde flottantes
# partent tous de cette liste.
CORE_SRCS := $(filter-out src/p8/assets.c src/devtools/class_coverage.c src/devtools/dump.c src/platform/fatal_pc.c src/devtools/input_json.c,$(ENGINE_SRCS))

# Closure plus étroite : ce que pico8_data_init() atteint pour construire un
# vrai état de jeu, sans booter de cart.
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

# Le même test que ci-dessus, sous UBSan : pico8_fixed fait des réinterprétations
# de bits signé/non signé qui donneraient les bons nombres tout en étant de l'UB.
# -fno-sanitize-recover fait échouer le binaire au lieu d'imprimer un avertissement.
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

# Ce test lance le vrai pilote de rejeu, d'où $(TMP_DIR)/pico8_replay en
# prérequis : il rejoue la même fixture avec et sans capture et compare les deux
# dumps, pour que l'argument de capture reste sans effet sur le reste.
$(TMP_DIR)/test_headless_capture: $(call objs,tests/test_headless_capture.c src/devtools/screencast.c src/devtools/dump.c src/p8/gfx.c src/p8/mem.c src/p8/input.c src/devtools/input_json.c) $(TMP_DIR)/pico8_replay tests/fixtures/screencast_capture_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_replay_from_title: $(call objs,tests/test_replay_from_title.c) $(TMP_DIR)/pico8_replay tests/fixtures/title_click_forgiving.json tests/fixtures/title_click_reasonable.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_replay_usage: $(call objs,tests/test_replay_usage.c) $(TMP_DIR)/pico8_replay tests/fixtures/title_click_forgiving.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/pico8_frames: $(call objs,src/cmd/frames.c src/devtools/screencast.c src/devtools/dump.c src/p8/mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Lance le pilote de rejeu puis l'outil d'émission de frames.
$(TMP_DIR)/test_frames_tool: $(call objs,tests/test_frames_tool.c src/devtools/screencast.c src/p8/mem.c) $(TMP_DIR)/pico8_replay $(TMP_DIR)/pico8_frames tests/fixtures/screencast_capture_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_frames_check: $(call objs,tests/test_frames_check.c) tools/frames_check.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_class_coverage: $(call coverage_objs,tests/test_class_coverage.c src/devtools/class_coverage.c) $(TMP_DIR)/pico8_class_coverage tools/class_coverage.sh tests/fixtures/title_click_forgiving.json tests/fixtures/events_malformed_missing_field.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_line_coverage: $(call objs,tests/test_line_coverage.c) $(TMP_DIR)/pico8_replay_gcov tools/line_coverage.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_e2e_limits: $(call objs,tests/test_e2e_limits.c) tools/e2e_limits.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_e2e_cleanup: $(call objs,tests/test_e2e_cleanup.c) tools/e2e_cleanup.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/pico8_minimize: $(call objs,src/cmd/minimize.c src/devtools/minimize.c src/p8/input.c src/devtools/input_json.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_minimize: $(call objs,tests/test_minimize.c src/devtools/minimize.c src/p8/input.c src/devtools/input_json.c) $(TMP_DIR)/pico8_minimize tools/minimize.sh | $(TMP_DIR)
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

$(TMP_DIR)/pico8_gif: $(call objs,src/cmd/gif.c src/devtools/gif.c src/p8/palette.c src/devtools/screencast.c src/p8/mem.c src/p8/input.c src/devtools/input_json.c) | $(TMP_DIR)
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

$(TMP_DIR)/test_live_slots: $(call asan_objs,tests/test_live_slots.c $(ENGINE_SRCS)) $(TMP_DIR)/pico8_replay | $(TMP_DIR)
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

$(TMP_DIR)/pico8_replay: $(call objs,$(REPLAY_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/pico8_replay_gcov: $(call gcov_objs,$(REPLAY_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(GCOV_CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_background_draw: $(call objs,tests/test_background_draw.c $(DATA_ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# Sous ASan, et sans jumeau non instrumenté : le crash visé dépend de la
# disposition de l'allocateur (le même rejeu sort 139 en -O0 et 0 en -O1), donc
# seul le débordement lui-même est un oracle fiable.
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

# Liste de sources réduite et non $(ENGINE_SRCS) : ce test ne boote pas de cart.
# Mesurée en linkant, pas devinée.
$(TMP_DIR)/test_layout_nil_item_draws_nothing: $(call objs,tests/test_layout_nil_item_draws_nothing.c tests/heap_allocator.c src/p8/assets.c src/engine/collect.c src/engine/dispatch_context.c src/p8/fixed.c src/engine/geom.c src/p8/gfx.c src/game/hud.c src/game/layout.c src/engine/pointer_list.c src/game/map_tables.c src/p8/mem.c src/p8/str.c src/engine/vec.c src/engine/allocator.c src/engine/zone.c src/platform/fatal_pc.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

# Le plus long en tête : sous -j, make le lance d'abord.
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

# Un lancement par TU, en cible à part : c'est ce que -j parallélise.
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

# Ces deux TU relisent tmp/pico8_replay_dump.txt, le dump par défaut de
# pico8_replay : sous -j, jamais en même temps.
unit-run-test_frames_tool: | unit-run-test_headless_capture

.PHONY: $(UNIT_RUNS)

# Échoue avant toute compilation plutôt que de rejouer un fichier par défaut.
ifneq ($(filter replay screencast,$(MAKECMDGOALS)),)
    ifeq ($(strip $(EVENTS)),)
        $(error EVENTS manque : make $(filter replay screencast,$(MAKECMDGOALS)) EVENTS=<events.json>)
    endif
endif

replay: $(TMP_DIR)/pico8_replay
	./$(TMP_DIR)/pico8_replay $(EVENTS)

test: unit constants-check float-grep float-guard alloc-guard data-guard composition-guard rename-derive-test rename-strings-test rename-check-test rename-slice-test

test-all: test e2e

SCREENCAST_BIN := $(TMP_DIR)/screencast.bin

SCREENCAST_VIEWER_BIN := $(TMP_DIR)/screencast_viewer

# Les e2e : <nom>.json (même format qu'un enregistrement raylib), <nom>.txt
# (ce que fait le rejeu, frames des actions clés) et <nom>.json.sha256. Tous
# partent de l'écran titre.
E2E_DIR := tests/e2e
E2E_EVENTS := $(sort $(wildcard $(E2E_DIR)/*.json))
# Une ligne `<events> raison` par e2e admis jusqu'à 600 frames au lieu de 180.
E2E_EXCEPTIONS := tools/e2e_long_exceptions.txt

ifeq ($(UNAME_S),Darwin)
    SHA256_CMD := shasum -a 256
else
    SHA256_CMD := sha256sum
endif

# Refaite à chaque appel : EVENTS change d'un appel à l'autre sans que make le
# voie. Cible fichier plutôt que phony, pour que .DELETE_ON_ERROR s'applique.
$(SCREENCAST_BIN): $(TMP_DIR)/pico8_replay FORCE | $(TMP_DIR)
	./$(TMP_DIR)/pico8_replay $(EVENTS) $@ $(@:.bin=.dump.txt)

screencast: $(SCREENCAST_BIN)

.PHONY: FORCE
FORCE:

# Une capture interrompue laisserait un fichier partiel que make croirait à jour,
# et l'empreinte suivante porterait sur lui.
.DELETE_ON_ERROR:

# Le sha256 de la capture de chaque e2e, versionné à côté de lui sous
# <events>.sha256. La capture est la concaténation brute des frames : son
# empreinte couvre aussi leur nombre. Le binaire -O0 de make test : pas de
# second arbre à compiler, et lisible sous gdb.
FRAMES_TOOL := $(TMP_DIR)/pico8_frames
FRAMES_DIR := $(TMP_DIR)/frames

frames_capture = $(FRAMES_DIR)/$(1).bin
frames_current = $(FRAMES_DIR)/$(1).sha256
frames_reference = $(1).sha256

# Sans .SECONDARY, make effacerait ces captures intermédiaires, que e2e-ref
# compare à la capture de référence.
.SECONDARY: $(foreach e,$(E2E_EVENTS),$(call frames_capture,$(e)))

# Chaque capture a son propre dump texte : le dump par défaut,
# tmp/pico8_replay_dump.txt, est relu par deux TU qui tournent en parallèle.
$(FRAMES_DIR)/%.bin: % $(TMP_DIR)/pico8_replay
	@mkdir -p $(@D)
	./$(TMP_DIR)/pico8_replay $< $@ $(@:.bin=.dump.txt) > /dev/null

# L'affectation porte le code de sortie du sha256, qu'un pipe vers cut perdrait.
frames_hash = @mkdir -p $(@D); h=$$($(SHA256_CMD) $<) && echo "$${h%% *}" > $@

$(FRAMES_DIR)/%.sha256: $(FRAMES_DIR)/%.bin
	$(frames_hash)

e2e: $(foreach e,$(E2E_EVENTS),$(call frames_current,$(e))) tools/e2e_limits.sh tools/frames_check.sh $(E2E_EXCEPTIONS)
	@status=0; \
	sh tools/e2e_limits.sh $(E2E_EXCEPTIONS) $(E2E_EVENTS) || status=1; \
	sh tools/frames_check.sh $(foreach e,$(E2E_EVENTS),$(e) $(call frames_reference,$(e)) $(call frames_current,$(e))) || status=1; \
	echo "e2e: $(words $(E2E_EVENTS)) e2e dans $(E2E_DIR)"; \
	exit $$status

GIF_DIR := gif

gif: $(foreach e,$(E2E_EVENTS),$(GIF_DIR)/$(basename $(notdir $(e))).gif)
	@echo "gif: $(words $(E2E_EVENTS)) GIF dans $(GIF_DIR)/, $$(du -ch $(GIF_DIR)/*.gif | tail -1 | cut -f1)"

$(GIF_DIR)/%.gif: $(call frames_capture,$(E2E_DIR)/%.json) $(E2E_DIR)/%.json $(TMP_DIR)/pico8_gif
	@mkdir -p $(@D)
	./$(TMP_DIR)/pico8_gif $< $(E2E_DIR)/$*.json $@

e2e-baseline: $(foreach e,$(E2E_EVENTS),$(call frames_current,$(e)))
	@$(foreach e,$(E2E_EVENTS),cp $(call frames_current,$(e)) $(call frames_reference,$(e)) && echo "e2e: reference posee dans $(call frames_reference,$(e))" && ) true

e2e-ref: $(foreach e,$(E2E_EVENTS),$(if $(wildcard $(call frames_reference,$(e))),$(call frames_current,$(e)))) $(FRAMES_TOOL) tools/frames_ref.sh
	@sh tools/frames_ref.sh ./$(FRAMES_TOOL) $(FRAMES_DIR) $(TMP_DIR)/frames-ref $(foreach e,$(E2E_EVENTS),$(e) $(call frames_reference,$(e)) $(call frames_current,$(e)) $(call frames_capture,$(e)))

# Tout vient de l'arbre coverage, qui n'est lié qu'ici et dans
# test_class_coverage : le compteur n'y est compilé que sous PICO8_CLASS_COVERAGE.
$(TMP_DIR)/pico8_class_coverage: $(call coverage_objs,src/cmd/class_coverage.c $(ENGINE_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

CLASS_COVERAGE_EXEMPT := tools/class_coverage_exempt.txt
CLASS_COVERAGE_NET_EXPECTED := tools/class_coverage_net_expected.txt

class-coverage: $(TMP_DIR)/pico8_class_coverage tools/class_coverage.sh $(CLASS_COVERAGE_EXEMPT)
	@sh tools/class_coverage.sh ./$(TMP_DIR)/pico8_class_coverage '$(or $(DIR),-)' $(CLASS_COVERAGE_EXEMPT) $(if $(DIR),,$(E2E_EVENTS))

class-coverage-net: $(TMP_DIR)/pico8_class_coverage tools/class_coverage.sh $(CLASS_COVERAGE_EXEMPT) $(CLASS_COVERAGE_NET_EXPECTED)
	@sh tools/class_coverage.sh --expect $(CLASS_COVERAGE_NET_EXPECTED) ./$(TMP_DIR)/pico8_class_coverage - $(CLASS_COVERAGE_EXEMPT) $(E2E_EVENTS)

.PHONY: class-coverage class-coverage-net

LINE_COVERAGE := sh tools/line_coverage.sh
LINE_COVERAGE_COVER := $(LINE_COVERAGE) cover ./$(TMP_DIR)/pico8_replay_gcov $(OBJ_GCOV_DIR)
# Écrite une fois, sur l'ancien filet de longs enregistrements : on ne la
# recalcule pas, on la compare.
LINE_COVERAGE_REFERENCE := tools/line_coverage_reference.txt
# Une ligne `fichier:ligne raison` par ligne de la référence qu'aucun rejeu
# court n'atteint, éditée à la main.
LINE_COVERAGE_UNREACHABLE := tools/line_coverage_unreachable.txt

line-coverage: $(TMP_DIR)/pico8_replay_gcov tools/line_coverage.sh
	@$(LINE_COVERAGE_COVER) $(EVENTS) > $(TMP_DIR)/line_coverage.txt
	@echo "couverture de lignes: $$(wc -l < $(TMP_DIR)/line_coverage.txt) ligne(s) dans $(TMP_DIR)/line_coverage.txt"

line-coverage-contrib: $(TMP_DIR)/pico8_replay_gcov tools/line_coverage.sh
	@$(LINE_COVERAGE_COVER) $(EVENTS) > $(TMP_DIR)/line_coverage_self.txt
	@$(if $(strip $(OTHERS)),$(LINE_COVERAGE_COVER) $(OTHERS),:) > $(TMP_DIR)/line_coverage_others.txt
	@$(LINE_COVERAGE) contrib $(TMP_DIR)/line_coverage_self.txt $(TMP_DIR)/line_coverage_others.txt > $(TMP_DIR)/line_coverage_contrib.txt
	@echo "apport de $(EVENTS): $$(wc -l < $(TMP_DIR)/line_coverage_contrib.txt) ligne(s) dans $(TMP_DIR)/line_coverage_contrib.txt"

# Sans e2e, l'ensemble couvert est vide : l'écart est toute la référence.
e2e-gap: $(TMP_DIR)/pico8_replay_gcov tools/line_coverage.sh $(LINE_COVERAGE_REFERENCE) $(LINE_COVERAGE_UNREACHABLE)
	@$(if $(E2E_EVENTS),$(LINE_COVERAGE_COVER) $(E2E_EVENTS),:) > $(TMP_DIR)/line_coverage_e2e.txt
	@$(LINE_COVERAGE) gap $(LINE_COVERAGE_REFERENCE) $(LINE_COVERAGE_UNREACHABLE) $(TMP_DIR)/line_coverage_e2e.txt > $(TMP_DIR)/line_coverage_gap.txt
	@sed 's/^/manque: /' $(TMP_DIR)/line_coverage_gap.txt
	@n=$$(wc -l < $(TMP_DIR)/line_coverage_gap.txt | tr -d ' '); \
	if [ "$$n" -ne 0 ]; then \
	    echo "écart: $$n ligne(s) de $(LINE_COVERAGE_REFERENCE) ni couvertes par make e2e ($(words $(E2E_EVENTS)) e2e) ni dans $(LINE_COVERAGE_UNREACHABLE), dans $(TMP_DIR)/line_coverage_gap.txt"; \
	    exit 1; \
	fi
	@echo "écart: nul -- make e2e couvre toute la référence hors $(LINE_COVERAGE_UNREACHABLE)"

.PHONY: line-coverage line-coverage-contrib e2e-gap

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

minimize: $(TMP_DIR)/pico8_minimize $(TMP_DIR)/pico8_replay_gcov tools/minimize.sh tools/line_coverage.sh
	@sh tools/minimize.sh ./$(TMP_DIR)/pico8_minimize $(if $(MIN_CHUNK),--min-chunk $(MIN_CHUNK)) $(EVENTS) $(OUT) $(if $(MECHANICAL),,$(LINE_COVERAGE_COVER))
	@echo "minimize: $(EVENTS) -> $(OUT), $$(./$(TMP_DIR)/pico8_minimize count $(OUT)) frame(s)"

E2E_CLEANUP_DIR := $(TMP_DIR)/e2e_cleanup

e2e-cleanup: $(TMP_DIR)/pico8_minimize $(TMP_DIR)/pico8_replay_gcov tools/e2e_cleanup.sh tools/line_coverage.sh
	@rm -rf $(E2E_CLEANUP_DIR) && mkdir -p $(E2E_CLEANUP_DIR)
	@$(foreach e,$(E2E_EVENTS),$(LINE_COVERAGE_COVER) $(e) > $(E2E_CLEANUP_DIR)/$(notdir $(e)).lines && ) true
	@$(if $(E2E_EVENTS),sh tools/e2e_cleanup.sh $(foreach e,$(E2E_EVENTS),$(e) $$(./$(TMP_DIR)/pico8_minimize count $(e)) $(E2E_CLEANUP_DIR)/$(notdir $(e)).lines),:) > $(E2E_CLEANUP_DIR)/remove.txt
	@sed 's/^/à retirer: /' $(E2E_CLEANUP_DIR)/remove.txt
	@echo "e2e-cleanup: $$(wc -l < $(E2E_CLEANUP_DIR)/remove.txt | tr -d ' ') fichier(s) à retirer sur $(words $(E2E_EVENTS)), rien n'est supprimé"

e2e-judge: $(TMP_DIR)/pico8_replay_gcov tools/line_coverage.sh
	@$(LINE_COVERAGE_COVER) $(EVENTS) > $(TMP_DIR)/e2e_judge_self.txt
	@$(if $(filter-out $(EVENTS),$(E2E_EVENTS)),$(LINE_COVERAGE_COVER) $(filter-out $(EVENTS),$(E2E_EVENTS)),:) > $(TMP_DIR)/e2e_judge_others.txt
	@$(LINE_COVERAGE) contrib $(TMP_DIR)/e2e_judge_self.txt $(TMP_DIR)/e2e_judge_others.txt > $(TMP_DIR)/e2e_judge_contrib.txt
	@n=$$(wc -l < $(TMP_DIR)/e2e_judge_contrib.txt | tr -d ' '); \
	echo "e2e-judge: $(EVENTS) ajoute $$n ligne(s) à make e2e, dans $(TMP_DIR)/e2e_judge_contrib.txt"; \
	[ "$$n" -gt 0 ]

.PHONY: minimize e2e-cleanup e2e-judge

# Catégories du compteur tombées à 0, séparées par des virgules.
DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit,valeur,type-tag

data-residue: tools/data_residue.sh
	@sh tools/data_residue.sh

# grep rend 1 quand rien ne correspond ; au-delà, c'est une erreur.
data-guard: tools/data_residue.sh
	@status=0; grep -rn offsetof src/ || status=$$?; \
	if [ $$status -eq 0 ]; then echo "data-guard : offsetof dans src/"; exit 1; fi; \
	[ $$status -eq 1 ] || exit $$status; \
	echo "offsetof dans src/ : 0"
	@if [ -n "$(DATA_RESIDUE_GUARD)" ]; then \
	    sh tools/data_residue.sh --check $(DATA_RESIDUE_GUARD) src tests; \
	else \
	    echo "catégories gardées : aucune"; \
	fi

.PHONY: data-residue data-guard

composition-residue: tools/composition_residue.sh
	@sh tools/composition_residue.sh

# Catégories du compteur de composition tombées à 0, séparées par des virgules.
COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6,lien-parent,parente,void-ptr,inclusion-mutuelle,usage-mutuel,void-self

composition-guard: tools/composition_residue.sh
	@sh tools/composition_residue.sh --check $(COMPOSITION_RESIDUE_GUARD) src tests

.PHONY: composition-residue composition-guard

$(TMP_DIR)/pico8_replay_sanitize: $(call sanitize_objs,$(REPLAY_SRCS)) | $(TMP_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) $(UBSAN_CFLAGS) -o $@ $(filter %.o,$^)

SANITIZE_DIR := $(TMP_DIR)/sanitize
SANITIZE_RUNS := $(addprefix sanitize-run-,$(E2E_EVENTS))

$(SANITIZE_RUNS): sanitize-run-%: % $(TMP_DIR)/pico8_replay_sanitize
	@mkdir -p $(SANITIZE_DIR)/$(<D)
	@./$(TMP_DIR)/pico8_replay_sanitize $< /dev/null $(SANITIZE_DIR)/$<.dump.txt > $(SANITIZE_DIR)/$<.log 2>&1 || { cat $(SANITIZE_DIR)/$<.log; echo "sanitize: $< sale"; exit 1; }

sanitize-net: $(SANITIZE_RUNS)
	@echo "sanitize: $(words $(SANITIZE_RUNS)) propre(s)"

.PHONY: sanitize-net $(SANITIZE_RUNS)

# Les sources du cœur, où le comparateur lit les appels PICO8_FIXED_LIT.
constants-check: tools/constants_check.sh tools/constants_compare.sh tools/constants_expected_gaps.txt tools/pico8_reference_probe_out.txt src/p8/fixed.c src/p8/fixed.h $(CORE_SRCS) | $(TMP_DIR)
	@sh tools/constants_check.sh tools/constants_expected_gaps.txt tools/pico8_reference_probe_out.txt src/p8/fixed.c $(TMP_DIR)/constants_check \
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

ALLOC_GUARD_EXCEPTIONS := tools/alloc_guard_exceptions.txt

alloc-guard: tools/alloc_guard.sh $(ALLOC_GUARD_EXCEPTIONS) $(call objs,$(CORE_SRCS))
	@sh tools/alloc_guard.sh $(ALLOC_GUARD_EXCEPTIONS) $(OBJ_DIR) $(call objs,$(CORE_SRCS))

.PHONY: alloc-guard

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
	./$(TMP_DIR)/gen_quarter_wave > $(TMP_DIR)/quarter_wave.h
	mv $(TMP_DIR)/quarter_wave.h src/p8/quarter_wave.h

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
# platform/raylib_backend_coords.c est aussi linké sans ce flag par un test.
$(TMP_DIR)/main_raylib: src/cmd/game.c src/platform/raylib_backend.c src/platform/raylib_backend.h src/platform/raylib_backend_coords.c src/platform/raylib_backend_coords.h src/p8/palette.c src/p8/palette.h src/p8/assets.c src/p8/assets.h src/p8/audio.c src/game/background.c src/engine/bootstrap.c src/game/cart.c src/game/cart.h src/game/cheat.c src/game/cheat.h src/engine/collect.c src/game/data_init.c src/engine/dispatch_context.c src/engine/dispatch_context.h src/devtools/dump.c src/game/economy.c src/game/economy_formulas.c src/game/economy_score.c src/game/economy_tiers.c src/engine/entity_registry.c src/engine/entity_geom.c src/engine/exit_queue.c src/p8/fixed.c src/game/galaxy.c src/game/game_data.c src/game/gen.c src/engine/geom.c src/p8/gfx.c src/game/help.c src/game/help.h src/game/hud.c src/p8/input.c src/devtools/input_json.c src/engine/entity.c src/game/layout.c src/engine/entity_list.c src/engine/pointer_list.c src/game/map_tables.c src/p8/mem.c src/game/menu.c src/game/mouse_button.c src/game/mouse.c src/game/message_box.c src/game/particle.c src/game/draw_functions.c src/game/planet.c src/game/planet_display.c src/game/planet_family.c src/game/planet_family_trade.c src/game/planet_trade.c src/game/probe.c src/devtools/replay_control.c src/devtools/replay_control.h src/devtools/replay_source.c src/devtools/replay_source.h src/game/research.c src/game/research_display.c src/game/structure_icon.c src/game/score_panel.c src/game/score_panel_display.c src/game/scroll.c src/devtools/session_recorder.c src/devtools/session_recorder.h src/game/slipway.c src/game/slipway_display.c src/p8/str.c src/game/tech_gate.c src/game/trade.c src/game/ui.c src/game/ui_button.c src/game/ui_button.h src/engine/vec.c src/game/wormhole.c src/game/wormhole_display.c src/engine/zone.c src/engine/allocator.c src/engine/frame_zone.c src/game/ui_zone.c src/game/particle_zone.c src/engine/game_zone.c src/platform/fatal_pc.c third_party/raylib/src/libraylib.a | $(TMP_DIR)
	$(CC) $(CFLAGS) -I third_party/raylib/src -o $@ src/cmd/game.c src/platform/raylib_backend.c src/platform/raylib_backend_coords.c src/p8/palette.c src/p8/assets.c src/p8/audio.c src/game/background.c src/engine/bootstrap.c src/game/cart.c src/game/cheat.c src/engine/collect.c src/game/data_init.c src/engine/dispatch_context.c src/devtools/dump.c src/game/economy.c src/game/economy_formulas.c src/game/economy_score.c src/game/economy_tiers.c src/engine/entity_registry.c src/engine/entity_geom.c src/engine/exit_queue.c src/p8/fixed.c src/game/galaxy.c src/game/game_data.c src/game/gen.c src/engine/geom.c src/p8/gfx.c src/game/help.c src/game/hud.c src/p8/input.c src/devtools/input_json.c src/engine/entity.c src/game/layout.c src/engine/entity_list.c src/engine/pointer_list.c src/game/map_tables.c src/p8/mem.c src/game/menu.c src/game/mouse_button.c src/game/mouse.c src/game/message_box.c src/game/particle.c src/game/draw_functions.c src/game/planet.c src/game/planet_display.c src/game/planet_family.c src/game/planet_family_trade.c src/game/planet_trade.c src/game/probe.c src/devtools/replay_control.c src/devtools/replay_source.c src/game/research.c src/game/research_display.c src/game/structure_icon.c src/game/score_panel.c src/game/score_panel_display.c src/game/scroll.c src/devtools/session_recorder.c src/game/slipway.c src/game/slipway_display.c src/p8/str.c src/game/tech_gate.c src/game/trade.c src/game/ui.c src/game/ui_button.c src/engine/vec.c src/game/wormhole.c src/game/wormhole_display.c src/engine/zone.c src/engine/allocator.c src/engine/frame_zone.c src/game/ui_zone.c src/game/particle_zone.c src/engine/game_zone.c src/platform/fatal_pc.c third_party/raylib/src/libraylib.a $(RAYLIB_LDLIBS)

raylib: $(TMP_DIR)/main_raylib

# Le visualiseur : même règle que la cible raylib ci-dessus, link seulement.
$(SCREENCAST_VIEWER_BIN): src/cmd/screencast_viewer.c src/devtools/screencast.c src/devtools/screencast.h src/devtools/screencast_player.c src/devtools/screencast_player.h src/p8/palette.c src/p8/palette.h src/devtools/replay_control.c src/devtools/replay_control.h src/platform/raylib_backend.c src/platform/raylib_backend.h src/platform/raylib_backend_coords.c src/platform/raylib_backend_coords.h src/devtools/session_recorder.c src/devtools/session_recorder.h src/p8/input.c src/devtools/input_json.c src/p8/input.h src/p8/mem.c src/p8/mem.h src/p8/gfx.h third_party/raylib/src/libraylib.a | $(TMP_DIR)
	$(CC) $(CFLAGS) -I third_party/raylib/src -o $@ src/cmd/screencast_viewer.c src/devtools/screencast.c src/devtools/screencast_player.c src/p8/palette.c src/devtools/replay_control.c src/platform/raylib_backend.c src/platform/raylib_backend_coords.c src/devtools/session_recorder.c src/p8/input.c src/devtools/input_json.c src/p8/mem.c third_party/raylib/src/libraylib.a $(RAYLIB_LDLIBS)

screencast-viewer: $(SCREENCAST_VIEWER_BIN)

AST_RENAME_BINDINGS := $(CURDIR)/third_party/clang-python
LIBCLANG ?= /usr/lib/llvm-18/lib/libclang-18.so.1
AST_RENAME_ENV := PYTHONPATH=$(CURDIR)/tools:$(AST_RENAME_BINDINGS) LIBCLANG=$(LIBCLANG)
AST_RENAME_DIR := $(TMP_DIR)/ast_rename
# Pathspec ':!' : git's '*' crosses '/', so tests/fixtures/ would match tests/*.c.
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
	@test -n "$(MODULES)" || { echo "usage: make rename-slice MODULES=\"<module>...\"" >&2; exit 1; }
	@mkdir -p $(AST_RENAME_DIR)
	python3 tools/rename_slice.py --table docs/renommage.tsv $(foreach m,$(MODULES),--module '$(m)') \
	    -o $(AST_RENAME_DIR)/slice.tsv

rename-apply:
	@test -n "$(TABLE)" || { echo "usage: make rename-apply TABLE=<table>" >&2; exit 1; }
	@mkdir -p $(AST_RENAME_DIR)
	$(AST_RENAME_ENV) python3 -m ast_rename apply --table $(TABLE) \
	    --report $(AST_RENAME_DIR)/report.txt $(AST_RENAME_ANALYSIS)

# Les fragments de dépendances écrits par le compilateur, un par objet. Le
# wildcard tolère qu'ils n'existent pas : sur un clone frais, tout est compilé.
-include $(wildcard $(OBJ_DIR)/src/*/*.d $(OBJ_DIR)/tests/*.d)
-include $(wildcard $(OBJ_ASAN_DIR)/src/*/*.d $(OBJ_ASAN_DIR)/tests/*.d)
-include $(wildcard $(OBJ_UBSAN_DIR)/src/*/*.d $(OBJ_UBSAN_DIR)/tests/*.d)
-include $(wildcard $(OBJ_COVERAGE_DIR)/src/*/*.d $(OBJ_COVERAGE_DIR)/tests/*.d)
-include $(wildcard $(OBJ_GCOV_DIR)/src/*/*.d $(OBJ_GCOV_DIR)/tests/*.d)
-include $(wildcard $(OBJ_SANITIZE_DIR)/src/*/*.d $(OBJ_SANITIZE_DIR)/tests/*.d)
