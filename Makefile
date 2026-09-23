# make test          -> unit + replay + screencast-check + frames-check
#                       + constants-check + float-audit + float-grep + float-guard,
#                       la suite complète
# make unit          -> les binaires de tests/, compilés et lancés
# make replay        -> rejoue events.json dans le moteur headless
# make screencast    -> refabrique les pixels de chaque frame du rejeu
# make screencast-check -> compare leur empreinte à celle qui est signée
# make screencast-sign  -> signe l'empreinte actuelle (acte humain, voir plus bas)
# make frames-check  -> compare les empreintes par frame de chaque rejeu du filet
# make frames-baseline -> (re)pose ces empreintes de référence, jamais par make test ;
#                       refuse si une référence diverge, sauf avec ETAPE=<id>, qui
#                       écrit d'abord tools/frames_reports/<id>.txt
# make frames-report ETAPE=<id> -> écrit tools/frames_reports/<id>.txt, toutes les
#                       frames divergentes par rejeu, sans re-baseliner
# make ref-audit     -> diagnostic des casts REF, hors suite
# make constants-check -> compare chaque PICO8_FIXED_LIT du cœur à son hexa relevé
#                       (tools/probe_out.txt) ; échoue si les écarts ne sont pas
#                       exactement tools/constants_expected_ecarts.txt, dans make test
# make float-audit   -> audit avant/après par site, dans make test : sites C1
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
# make frames-ledger -> confronte tools/frames_ledger.txt à l'historique git des
#                       *.frames.hash (§6.4), rapports et relevé compris ; hors
#                       suite : rend 2 sur un clone qui n'atteint pas la base
# make sin-table     -> régénère src/pico8_quarter_wave.h (table de sin/cos du
#                       cœur) depuis tools/gen_quarter_wave.c, hors suite
# make raylib        -> compile et LINK le binaire interactif, ne le lance jamais
# make screencast-viewer -> idem pour le visualiseur d'enregistrement
#                       (les deux exigent make raylib-vendor-setup au préalable)
# make psx-hello     -> projet PSX de test (nécessite psn00bsdk-toolchain-setup)

.PHONY: psn00bsdk-toolchain-setup psx-hello psx-hello-clean unit replay test raylib-vendor-setup raylib screencast screencast-check screencast-sign screencast-viewer frames-check frames-baseline frames-report

CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Werror -g

TMP_DIR := tmp

# Un arbre d'objets par jeu de flags. Un objet compilé sans -fsanitize n'est pas
# instrumenté : le partager avec une cible qui l'est rendrait le binaire aveugle
# sur ce code, sans rien dire. Ne pas fusionner les cinq.
OBJ_DIR := $(TMP_DIR)/obj
OBJ_ASAN_DIR := $(TMP_DIR)/obj-asan
OBJ_UBSAN_DIR := $(TMP_DIR)/obj-ubsan
OBJ_REFAUDIT_DIR := $(TMP_DIR)/obj-refaudit
OBJ_AUDIT_DIR := $(TMP_DIR)/obj-audit

# -MMD -MP : le compilateur écrit, à côté de chaque objet, la liste des en-têtes
# que l'unité de compilation a vraiment ouverts. Le -include en bas du fichier
# les relit. Aucun en-tête n'est donc nommé à la main dans une liste de prérequis.
DEPFLAGS := -MMD -MP
ASAN_CFLAGS := -fsanitize=address -fno-omit-frame-pointer
UBSAN_CFLAGS := -fsanitize=undefined -fno-sanitize-recover=undefined
REFAUDIT_CFLAGS := -DPICO8_REF_AUDIT
AUDIT_CFLAGS := -DPICO8_FLOAT_AUDIT

# $(call objs,<liste de .c>) -> les chemins des .o correspondants dans l'arbre
# concerné. Une cible nomme ses sources une fois ; ses prérequis et sa ligne de
# link en découlent.
objs = $(patsubst %.c,$(OBJ_DIR)/%.o,$(1))
asan_objs = $(patsubst %.c,$(OBJ_ASAN_DIR)/%.o,$(1))
ubsan_objs = $(patsubst %.c,$(OBJ_UBSAN_DIR)/%.o,$(1))
refaudit_objs = $(patsubst %.c,$(OBJ_REFAUDIT_DIR)/%.o,$(1))
audit_objs = $(patsubst %.c,$(OBJ_AUDIT_DIR)/%.o,$(1))

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
	$(CC) $(CFLAGS) $(AUDIT_CFLAGS) $(DEPFLAGS) -c -o $@ $<

# Closure complète du moteur : tout src/ sauf les deux points d'entrée (main.c,
# main_raylib.c) et les modules qui ne servent qu'au driver interactif. Utilisée
# par toute cible qui boote un cart, pico8_cart_load() tirant tout le port.
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

$(TMP_DIR)/test_pico8_input: $(call objs,tests/test_pico8_input.c src/pico8_input.c) tests/fixtures/demo_events.json | $(TMP_DIR)
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

$(TMP_DIR)/test_pico8_frame_hash: $(call objs,tests/test_pico8_frame_hash.c src/pico8_frame_hash.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/pico8_frames: $(call objs,src/main_frames.c src/pico8_frame_hash.c src/pico8_screencast.c src/pico8_dump.c src/pico8_mem.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

# Lance le pilote de rejeu puis l'outil d'empreintes, comme les cibles frames-*.
$(TMP_DIR)/test_pico8_frames_tool: $(call objs,tests/test_pico8_frames_tool.c src/pico8_frame_hash.c src/pico8_screencast.c src/pico8_mem.c) $(TMP_DIR)/pico8_replay $(TMP_DIR)/pico8_frames tests/fixtures/screencast_capture_events.json | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_frames_check: $(call objs,tests/test_frames_check.c) tools/frames_check.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_frames_report: $(call objs,tests/test_frames_report.c) tools/frames_report.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_audit: $(call objs,tests/test_float_audit.c) tools/float_audit.sh tools/float_audit_c1.txt | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_guard: $(call objs,tests/test_float_guard.c) tools/float_guard.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_grep: $(call objs,tests/test_float_grep.c) tools/float_grep.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_float_link: $(call objs,tests/test_float_link.c) tools/float_link.sh | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^)

$(TMP_DIR)/test_frames_ledger: $(call objs,tests/test_frames_ledger.c) tools/frames_ledger.sh | $(TMP_DIR)
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

# Liste de sources réduite et non $(ENGINE_SRCS) : ce test ne boote pas de cart.
# Mesurée en linkant, pas devinée.
$(TMP_DIR)/test_pico8_layout_nil_item_draws_nothing: $(call objs,tests/test_pico8_layout_nil_item_draws_nothing.c src/pico8_assets.c src/pico8_class.c src/pico8_collect.c src/pico8_dispatch_context.c src/pico8_fixed.c src/pico8_geom.c src/pico8_gfx.c src/pico8_hud.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_ob.c src/pico8_stash.c src/pico8_str.c src/pico8_value.c src/pico8_vec.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

unit: $(TMP_DIR)/test_pico8_mem $(TMP_DIR)/test_asset_loading $(TMP_DIR)/test_pico8_gfx $(TMP_DIR)/test_pico8_input $(TMP_DIR)/test_pico8_dump $(TMP_DIR)/test_pico8_fixed $(TMP_DIR)/test_pico8_fixed_ubsan $(TMP_DIR)/test_pico8_vec $(TMP_DIR)/test_pico8_geom $(TMP_DIR)/test_pico8_value $(TMP_DIR)/test_pico8_list $(TMP_DIR)/test_pico8_str $(TMP_DIR)/test_pico8_ob $(TMP_DIR)/test_pico8_entity_smoke $(TMP_DIR)/test_pico8_stash $(TMP_DIR)/test_pico8_data_init $(TMP_DIR)/test_pico8_planet_kj_dispatch $(TMP_DIR)/test_pico8_demo $(TMP_DIR)/test_pico8_playability $(TMP_DIR)/test_pico8_slipway_drag $(TMP_DIR)/test_pico8_tech_screen $(TMP_DIR)/test_pico8_game_end $(TMP_DIR)/test_pico8_pause_menu_music $(TMP_DIR)/test_pico8_geoharvest_burst $(TMP_DIR)/test_pico8_score_panel_close $(TMP_DIR)/test_pico8_raylib_backend_coords $(TMP_DIR)/test_pico8_replay_source $(TMP_DIR)/test_pico8_recording_continuity $(TMP_DIR)/test_pico8_replay_control $(TMP_DIR)/test_pico8_marker_coherence $(TMP_DIR)/test_pico8_screencast $(TMP_DIR)/test_pico8_headless_capture $(TMP_DIR)/test_pico8_screencast_player $(TMP_DIR)/test_pico8_frame_hash $(TMP_DIR)/test_pico8_frames_tool $(TMP_DIR)/test_frames_check $(TMP_DIR)/test_frames_report $(TMP_DIR)/test_float_audit $(TMP_DIR)/test_float_guard $(TMP_DIR)/test_float_grep $(TMP_DIR)/test_float_link $(TMP_DIR)/test_frames_ledger $(TMP_DIR)/test_constants_check $(TMP_DIR)/test_pico8_bg_kj $(TMP_DIR)/test_pico8_top_menubar $(TMP_DIR)/test_pico8_shop_tooltip_fillp $(TMP_DIR)/test_pico8_empty_planet_cleared $(TMP_DIR)/test_pico8_planet_gh_row_lq $(TMP_DIR)/test_pico8_ga_export_panel_by $(TMP_DIR)/test_pico8_built_planet_panel_nil_row $(TMP_DIR)/test_pico8_layout_nil_item_draws_nothing
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
	./$(TMP_DIR)/test_pico8_tech_screen
	./$(TMP_DIR)/test_pico8_game_end
	./$(TMP_DIR)/test_pico8_pause_menu_music
	./$(TMP_DIR)/test_pico8_geoharvest_burst
	./$(TMP_DIR)/test_pico8_score_panel_close
	./$(TMP_DIR)/test_pico8_raylib_backend_coords
	./$(TMP_DIR)/test_pico8_replay_source
	./$(TMP_DIR)/test_pico8_recording_continuity
	./$(TMP_DIR)/test_pico8_replay_control
	./$(TMP_DIR)/test_pico8_marker_coherence
	./$(TMP_DIR)/test_pico8_screencast
	./$(TMP_DIR)/test_pico8_headless_capture
	./$(TMP_DIR)/test_pico8_screencast_player
	./$(TMP_DIR)/test_pico8_frame_hash
	./$(TMP_DIR)/test_pico8_frames_tool
	./$(TMP_DIR)/test_frames_check
	./$(TMP_DIR)/test_frames_report
	./$(TMP_DIR)/test_float_audit
	./$(TMP_DIR)/test_float_guard
	./$(TMP_DIR)/test_float_grep
	./$(TMP_DIR)/test_float_link
	./$(TMP_DIR)/test_frames_ledger
	./$(TMP_DIR)/test_constants_check
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

test: unit replay screencast-check frames-check constants-check float-audit float-grep float-guard

# ENREGISTREMENT D'ÉCRAN : make screencast rejoue events.json en écrivant les
# pixels de chaque frame, make screencast-check compare leur empreinte à celle
# signée dans events.json.sha256. Sans fichier d'empreinte, la vérification ne
# fabrique rien et ne compare rien : elle dit seulement comment en poser une.
# Signer est un acte humain — ça vaut « j'ai regardé cette session et elle est
# juste » — d'où une cible séparée, jamais appelée par `make test`.
SCREENCAST_EVENTS := events.json
SCREENCAST_BIN := $(TMP_DIR)/screencast.bin
SCREENCAST_SHA := $(SCREENCAST_EVENTS).sha256

SCREENCAST_VIEWER_BIN := $(TMP_DIR)/screencast_viewer

ifeq ($(UNAME_S),Darwin)
    SHA256_CMD := shasum -a 256
else
    SHA256_CMD := sha256sum
endif

$(SCREENCAST_BIN): $(TMP_DIR)/pico8_replay $(SCREENCAST_EVENTS) | $(TMP_DIR)
	./$(TMP_DIR)/pico8_replay $(SCREENCAST_EVENTS) $@

screencast: $(SCREENCAST_BIN)

# Une capture interrompue laisserait un fichier partiel que make croirait à jour,
# et l'empreinte suivante porterait sur lui.
.DELETE_ON_ERROR:

ifeq ($(wildcard $(SCREENCAST_SHA)),)
SCREENCAST_CHECK_PREREQ :=
else
SCREENCAST_CHECK_PREREQ := $(SCREENCAST_BIN)
endif

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

screencast-sign: $(SCREENCAST_BIN)
	@$(SHA256_CMD) "$(SCREENCAST_BIN)" | cut -d' ' -f1 > "$(SCREENCAST_SHA)"
	@echo "screencast: empreinte posee dans $(SCREENCAST_SHA) : $$(cat $(SCREENCAST_SHA))"
	@echo "screencast: cette ligne est un acte de signature -- commettez-la avec ce que vous avez regarde."

.PHONY: screencast screencast-check screencast-sign screencast-viewer

# FILET PAR FRAME (docs/plan-2.md §5) : une empreinte par frame et par fichier
# d'événements, versionnée à côté de lui sous <events>.frames.hash. C'est une
# base de non-régression posée par make frames-baseline, pas la signature
# humaine de screencast-sign. Liste explicite : un motif sur tests/fixtures/
# ramasserait les fixtures malformées et celles du lecteur d'entrée.
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
    tests/fixtures/geoharvest_burst_events.json

FRAMES_TOOL := $(TMP_DIR)/pico8_frames
FRAMES_DIR := $(TMP_DIR)/frames

frames_capture = $(if $(filter $(SCREENCAST_EVENTS),$(1)),$(SCREENCAST_BIN),$(FRAMES_DIR)/$(1).bin)
frames_current = $(FRAMES_DIR)/$(1).frames.hash
frames_reference = $(1).frames.hash

# Sans .SECONDARY, make effacerait ces captures intermédiaires, que le message
# d'échec de frames-check désigne pour émettre une frame.
.SECONDARY: $(foreach e,$(filter-out $(SCREENCAST_EVENTS),$(FRAMES_EVENTS)),$(call frames_capture,$(e)))

# pico8_replay réécrit toujours tmp/pico8_replay_dump.txt, que relisent
# test_pico8_headless_capture et test_pico8_frames_tool : sous make -j, ces
# captures ne doivent pas tourner en même temps que ces deux tests.
$(FRAMES_DIR)/%.bin: % $(TMP_DIR)/pico8_replay
	@mkdir -p $(@D)
	./$(TMP_DIR)/pico8_replay $< $@ > /dev/null

$(FRAMES_DIR)/%.frames.hash: $(FRAMES_DIR)/%.bin $(FRAMES_TOOL)
	./$(FRAMES_TOOL) hashes $< $@

$(call frames_current,$(SCREENCAST_EVENTS)): $(SCREENCAST_BIN) $(FRAMES_TOOL)
	@mkdir -p $(@D)
	./$(FRAMES_TOOL) hashes $< $@

# Comme screencast-check : un rejeu sans référence n'est ni capturé ni comparé.
FRAMES_CHECK_PREREQ := $(foreach e,$(FRAMES_EVENTS),$(if $(wildcard $(call frames_reference,$(e))),$(call frames_current,$(e))))

frames-check: $(FRAMES_CHECK_PREREQ) tools/frames_check.sh
	@sh tools/frames_check.sh ./$(FRAMES_TOOL) $(foreach e,$(FRAMES_EVENTS),$(e) $(call frames_reference,$(e)) $(call frames_current,$(e)) $(call frames_capture,$(e)))

FRAMES_REPORTS_DIR := tools/frames_reports
FRAMES_TRIPLETS := $(foreach e,$(FRAMES_EVENTS),$(e) $(call frames_reference,$(e)) $(call frames_current,$(e)))
FRAMES_REPORT_FILE := $(FRAMES_REPORTS_DIR)/$(ETAPE).txt

# Écrit à côté puis renomme : un rapport interrompu n'écrase pas le précédent.
frames_write_report = sh tools/frames_report.sh --etape '$(ETAPE)' $(FRAMES_TRIPLETS) > $(FRAMES_REPORT_FILE).tmp \
	    || { rm -f $(FRAMES_REPORT_FILE).tmp; exit 1; }; \
	mv $(FRAMES_REPORT_FILE).tmp $(FRAMES_REPORT_FILE) && echo "frames: rapport ecrit dans $(FRAMES_REPORT_FILE)"

frames-report: $(foreach e,$(FRAMES_EVENTS),$(call frames_current,$(e))) tools/frames_report.sh
	@$(if $(ETAPE),,echo "frames-report: nommer l'etape : make frames-report ETAPE=<id>" >&2; exit 1)
	@$(frames_write_report)

# La garde refuse avant toute copie ; avec ETAPE=, le rapport précède la copie.
frames-baseline: $(foreach e,$(FRAMES_EVENTS),$(call frames_current,$(e))) tools/frames_report.sh
	@sh tools/frames_report.sh --garde '$(ETAPE)' $(FRAMES_TRIPLETS)
	@$(if $(ETAPE),$(frames_write_report))
	@$(foreach e,$(FRAMES_EVENTS),cp $(call frames_current,$(e)) $(call frames_reference,$(e)) && echo "frames: reference posee dans $(call frames_reference,$(e)) ($$(wc -l < $(call frames_reference,$(e)) | tr -d ' ') frames)" && ) true

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

# Les sources du cœur, où le comparateur lit les appels PICO8_FIXED_LIT.
constants-check: tools/constants_check.sh tools/constants_compare.sh tools/constants_expected_ecarts.txt tools/probe_out.txt src/pico8_fixed.c src/pico8_fixed.h $(CORE_SRCS) | $(TMP_DIR)
	@sh tools/constants_check.sh tools/constants_expected_ecarts.txt tools/probe_out.txt src/pico8_fixed.c $(TMP_DIR)/constants_check \
	    $(CORE_SRCS)

.PHONY: constants-check

AUDIT_DIR := $(TMP_DIR)/audit

# pico8_fixed.c vient de l'arbre par défaut : sous -DPICO8_FLOAT_AUDIT, la
# macro réécrirait la définition de pico8_fixed_flr_int.
$(TMP_DIR)/pico8_replay_audit: $(call audit_objs,src/main.c src/pico8_screencast.c src/pico8_float_audit.c $(filter-out src/pico8_fixed.c,$(ENGINE_SRCS))) $(call objs,src/pico8_fixed.c src/pico8_fixed_test_double.c) | $(TMP_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm

FLOAT_AUDIT_C1 := tools/float_audit_c1.txt
float_audit_records = $(AUDIT_DIR)/$(notdir $(1)).aud

# Un fichier d'enregistrements par fichier d'événements, tous effacés avant la
# passe : un rejeu mort laisse le sien absent, et le rapport échoue.
float-audit: $(TMP_DIR)/pico8_replay_audit tools/float_audit.sh $(FLOAT_AUDIT_C1)
	@mkdir -p $(AUDIT_DIR)
	rm -f $(AUDIT_DIR)/*.aud
	$(foreach e,$(FRAMES_EVENTS),PICO8_FLOAT_AUDIT_OUT=$(call float_audit_records,$(e)) ./$(TMP_DIR)/pico8_replay_audit $(e) $(AUDIT_DIR)/capture.bin > /dev/null && ) true
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

frames-ledger: tools/frames_ledger.sh tools/frames_ledger.txt tools/probe_out.txt
	@sh tools/frames_ledger.sh . tools/frames_ledger.txt tools/probe_out.txt $(FRAMES_REPORTS_DIR)

.PHONY: frames-ledger

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
$(TMP_DIR)/main_raylib: src/main_raylib.c src/pico8_raylib_backend.c src/pico8_raylib_backend.h src/pico8_raylib_backend_coords.c src/pico8_raylib_backend_coords.h src/pico8_palette.c src/pico8_palette.h src/pico8_assets.c src/pico8_assets.h src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_cart.c src/pico8_cart.h src/pico8_class.c src/pico8_collect.c src/pico8_data_init.c src/pico8_dispatch_context.c src/pico8_dispatch_context.h src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_galaxy.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_help.c src/pico8_help.h src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_menu.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_family.c src/pico8_planet_family_trade.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_replay_control.c src/pico8_replay_control.h src/pico8_replay_source.c src/pico8_replay_source.h src/pico8_research.c src/pico8_research_display.c src/pico8_research_icon.c src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_scroll.c src/pico8_session_recorder.c src/pico8_session_recorder.h src/pico8_slipway.c src/pico8_slipway_display.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_ui_button.h src/pico8_value.c src/pico8_vec.c src/pico8_wormhole.c src/pico8_wormhole_display.c third_party/raylib/src/libraylib.a | $(TMP_DIR)
	$(CC) $(CFLAGS) -I third_party/raylib/src -o $@ src/main_raylib.c src/pico8_raylib_backend.c src/pico8_raylib_backend_coords.c src/pico8_palette.c src/pico8_assets.c src/pico8_audio.c src/pico8_bg.c src/pico8_bootstrap.c src/pico8_cart.c src/pico8_class.c src/pico8_collect.c src/pico8_data_init.c src/pico8_dispatch_context.c src/pico8_dump.c src/pico8_economy.c src/pico8_economy_formulas.c src/pico8_economy_score.c src/pico8_economy_tiers.c src/pico8_entity.c src/pico8_entity_geom.c src/pico8_fixed.c src/pico8_galaxy.c src/pico8_game_data.c src/pico8_gen.c src/pico8_geom.c src/pico8_gfx.c src/pico8_help.c src/pico8_hud.c src/pico8_input.c src/pico8_ke.c src/pico8_layout.c src/pico8_list.c src/pico8_mem.c src/pico8_menu.c src/pico8_mouse.c src/pico8_mouse_reader.c src/pico8_msgbox.c src/pico8_ob.c src/pico8_particle.c src/pico8_pi.c src/pico8_planet.c src/pico8_planet_display.c src/pico8_planet_family.c src/pico8_planet_family_trade.c src/pico8_planet_trade.c src/pico8_probe.c src/pico8_replay_control.c src/pico8_replay_source.c src/pico8_research.c src/pico8_research_display.c src/pico8_research_icon.c src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_scroll.c src/pico8_session_recorder.c src/pico8_slipway.c src/pico8_slipway_display.c src/pico8_stash.c src/pico8_str.c src/pico8_tech_gate.c src/pico8_trade.c src/pico8_ui.c src/pico8_ui_button.c src/pico8_value.c src/pico8_vec.c src/pico8_wormhole.c src/pico8_wormhole_display.c third_party/raylib/src/libraylib.a $(RAYLIB_LDLIBS)

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
