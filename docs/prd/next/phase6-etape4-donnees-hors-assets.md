---
title: "Plan 6, étape 4 — Données hors des assets"
description: "Le jeu lit encore deux données dans les fichiers exportés de PICO-8 : sa table de palettes dans des pixels de sprites, et une carte de tuiles (`pico8/map.json`) qu'il écrase avant de la lire."
status: needs-triage
---

## Problem Statement

Deux données du jeu ne vivent pas dans le code mais dans les assets exportés du cart, pour des raisons qui n'existent que sur PICO-8 :

- **La table des palettes.** Le cart encode ses 16 blocs de palette (16 couleurs chacun) comme des pixels : le carré 16×16 en haut à gauche de la feuille de sprites. Au démarrage, `cart_init()` appelle `hud_build_palette_table()`, qui relit ces 256 pixels par `p8_sget` et les recopie dans la mémoire émulée à `0x5000` (avec le bit de transparence `0x80` sur la couleur 3) ; `hud_select_palette_block()` y recopie ensuite 16 octets vers la palette de dessin `0x5f00`. Une table constante de 256 octets passe donc par la feuille de sprites, une région de mémoire PICO-8 et un appel d'initialisation qu'il ne faut pas oublier : un TU qui dessine sans avoir appelé `hud_build_palette_table()` lit une table de zéros, où la couleur 3 n'est pas transparente, et fige un hachage que le jeu ne produit jamais (c'est le cas de trois hachages de `tests/test_layout_row.c`, voir Testing Decisions).
- **`pico8/map.json`.** Les deux binaires (`src/cmd/replay.c`, `src/cmd/game.c`) et 43 fichiers de tests chargent ce fichier de 22 Ko (une carte 128×64) par `p8_load_map_json()`. Or le jeu ne lit que les cellules `(0..31, 0..31)` (le seul `p8_map` de `src/` est celui de `background_draw()`), et `background_construct()` les écrit toutes, à chaque `background_new()`, avant que le fond soit enregistré et donc dessiné. Le contenu du fichier n'atteint jamais un pixel : c'est une lecture de JSON et un fichier à porter sur PSX pour rien.

Le plan 6 retire du cœur ce qui n'existe que pour PICO-8 ; ces deux détours en sont.

## Solution

- La table des palettes devient une table C `static const` de 16×16 octets dans `game/hud.c`. `hud_select_palette_block()` y lit son bloc et l'écrit dans la palette de dessin ; `hud_build_palette_table()` disparaît, et avec lui l'appel de `cart_init()` et la région `0x5000`. Un nouveau TU, `tests/test_hud.c`, compare chaque bloc sélectionné aux pixels de `pico8/gfx.json` : la table C est prouvée identique à celle que bâtissaient les sprites, avant et après la suppression de cette dernière.
- `pico8/map.json` est supprimé, avec `p8_load_map_json()`. Les binaires et les TU ne chargent plus que `pico8/gfx.json`. Le fond génère ses tuiles comme aujourd'hui ; `p8_mset`/`p8_map` et les régions de carte restent (l'étape 5 les type).

Rien ne change pour le joueur : mêmes 26 `*.sha256`, mêmes durées de `make test` et `make e2e`, `make sanitize-net` vert. Ces trois points ont été vérifiés par une expérience sur une copie de HEAD (`a1cbeb4`), décrite en Testing Decisions.

## User Stories

1. En tant que mainteneur, je veux lire les 16 blocs de palette dans le code, pour voir ce que fait un `palette_block` sans décoder des pixels de sprites.
2. En tant que mainteneur, je veux que `hud_select_palette_block()` marche sans initialisation préalable, pour qu'un oubli de `hud_build_palette_table()` ne puisse plus fausser un dessin.
3. En tant que mainteneur, je veux un TU qui compare la table C aux pixels de `gfx.json`, pour que la recopie à la main des 256 octets soit prouvée exacte.
4. En tant que mainteneur, je veux que ce TU reste après la suppression de `hud_build_palette_table()`, pour qu'une retouche de `gfx.json` ou de la table qui les désaccorde casse la suite.
5. En tant que mainteneur, je veux qu'un indice de bloc hors de 0..15 arrête le programme avec un message, pour qu'une donnée fautive ne lise pas silencieusement une autre région de mémoire.
6. En tant que mainteneur, je veux que les hachages figés des TU qui dessinaient sans table soient refigés à la valeur que le jeu produit vraiment, pour que ces TU testent enfin le rendu réel.
7. En tant que mainteneur, je veux que la raison de chaque hachage refigé soit dans le message de commit, avec la preuve qu'il vaut le hachage de HEAD quand la table est bâtie.
8. En tant que mainteneur, je veux que `pico8/map.json` disparaisse, pour que le seul asset du jeu soit la feuille de sprites.
9. En tant que mainteneur, je veux que `p8_load_map_json()` disparaisse avec son fichier, pour ne pas garder un chargeur sans données.
10. En tant que mainteneur, je veux que les 41 TU qui chargeaient la carte « pour faire comme le jeu » s'en passent, pour qu'aucun test ne dépende d'un fichier que rien ne lit.
11. En tant que mainteneur, je veux que `tests/test_demo.c` pose lui-même les quatre cellules qu'il dessine, pour garder sa vérification de `p8_map` (sprite 0 non dessiné, sprites 85 et 1 dessinés) sans le fichier.
12. En tant que mainteneur, je veux que `tests/test_asset_loading.c` ne teste plus que la feuille de sprites, pour que ses tests suivent ce qui est chargé.
13. En tant que mainteneur, je veux une preuve écrite que le fond ne lit que les cellules qu'il écrit, pour que la suppression de la carte ne repose pas sur la seule chance des e2e.
14. En tant que mainteneur, je veux les 26 `*.sha256` inchangés après chaque chunk, sans `make e2e-baseline`, pour que l'invariant « aucun pixel ne change » soit tenu.
15. En tant que mainteneur, je veux `make test` et `make e2e` pas plus lents, pour garder la boucle rapide (la règle du projet).
16. En tant que mainteneur, je veux `make sanitize-net` vert après chaque chunk, pour qu'ASan/UBSan confirment que l'`assert` de plage ne se déclenche dans aucun rejeu.
17. En tant que mainteneur, je veux que les commentaires qui nomment `hud_build_palette_table()`, `0x5000` comme table, `map_cell_sink` ou `map.json` soient corrigés, pour qu'aucun ne mente après l'étape.
18. En tant que portage PSX (plans 7–11), je veux une table de palettes en `const` et aucun JSON de carte, pour n'avoir qu'un asset à convertir et rien à copier en RAM au démarrage.
19. En tant que mainteneur de l'étape 5 (mémoire typée), je veux qu'il ne reste aucun lecteur ni écrivain de `0x5000..0x50ff` dans `src/`, pour ne pas avoir à typer une région morte.
20. En tant qu'exécutant de l'étape, je veux deux chunks indépendants, pour qu'un rejeu rouge désigne l'un ou l'autre.
21. En tant qu'exécutant de l'étape, je veux que l'étape ne touche à aucune chaîne que l'étape 3 migre, pour que les deux avancent en parallèle sans conflit de fond.

## Implementation Decisions

### Chunk A — Table des palettes

Inventaire (HEAD `a1cbeb4`) :
- `src/game/hud.c` : `hud_build_palette_table()` (boucle `p` 0..15 × `c` 0..15, `p8_sget(mem, p, c) | (c == 3 ? 0x80 : 0)` écrit à `0x5000 + 16p + c`) ; `hud_select_palette_block()` (`p8_fixed_shl(p8_fixed_flr(no), 4)`, puis `p8_memcpy(mem, 0x5f00, 0x5000 + offset, 16)`) ; `hud_select_palette_block_default()`.
- `src/game/hud.h` : prototype et doc des deux fonctions.
- Seul appelant de `hud_build_palette_table()` dans `src/` : `cart_init()` (`src/game/cart.c`). Dans `tests/` : `test_layout_nil_item_draws_nothing.c` (+ son commentaire) et `test_tooltip_draw_at.c`.
- Appelants de `hud_select_palette_block()` : `layout.c` (bloc d'un sprite de ligne), `message_box.c` (`toast_draw`, bloc = `fade`), `planet_display.c` (bloc du comptoir commercial), et `_default` dans `cart_draw`, `layout.c`, `message_box.c`, `planet_display.c`.
- Aucun autre lecteur ni écrivain de `0x5000..0x50ff` dans `src/`. `tests/test_gfx.c` (l. 120) et `tests/test_mem.c` (l. 114) y écrivent à la main pour tester `p8_memcpy` : ils ne dépendent pas de la table du jeu et restent (étape 5).

Décisions :
- **Forme de la table** : `static const unsigned char palette_blocks[16][16]` dans `hud.c`, ligne = bloc, colonne = couleur source, valeur = octet de palette de dessin tel que `hud_build_palette_table()` l'écrivait (le bit `0x80` compris : la colonne 3 vaut `0x80 | pixel`). Valeurs relevées dans `pico8/gfx.json` (`pixels[c * 128 + p]`, bloc `p`, couleur `c`) :

  ```
  bloc  0: 0,1,2,131,4,5,6,7,8,9,10,11,12,13,14,15
  bloc  1: 0,1,2,131,2,5,6,7,8,4,9,11,12,5,14,9
  bloc  2: 0,1,1,131,2,1,13,6,2,2,4,3,13,5,8,4
  bloc  3: 0,0,1,129,1,1,5,13,2,2,2,3,5,1,2,2
  bloc  4: 0,0,0,129,1,1,1,1,1,1,1,1,1,1,1,1
  bloc  5: 0,0,0,128,0,0,0,0,0,0,0,0,0,0,0,0
  bloc  6: 0,2,2,130,2,2,8,8,2,2,8,8,2,2,8,8
  bloc  7: 0,1,1,129,1,1,5,13,1,1,5,5,1,1,5,5
  bloc  8: 0,1,2,131,4,5,6,7,10,9,0,4,2,13,0,0
  bloc  9: 0,1,2,131,4,5,6,7,0,10,9,0,4,13,4,0
  bloc 10: 0,1,2,131,4,5,6,7,0,0,10,4,0,13,9,4
  bloc 11: 0,1,2,131,4,5,6,7,4,0,0,9,4,13,0,9
  bloc 12: 0,1,2,131,4,5,6,7,9,4,0,0,9,13,9,0
  bloc 13: 0,1,2,131,4,5,6,7,0,9,4,0,0,13,10,9
  bloc 14: 0,1,2,131,4,5,6,7,9,0,9,2,0,13,0,10
  bloc 15: 0,1,2,131,4,5,6,6,0,0,0,0,0,13,0,0
  ```
  Le TU (Testing Decisions) est l'autorité ; cette copie sert à l'écriture.
- **`hud_select_palette_block(mem, no)`** garde sa signature et son contrat visible : après l'appel, les 16 octets de la palette de dessin (`P8_ADDR_DRAW_PALETTE`) valent le bloc `flr(no)`. Il écrit ces 16 octets depuis la table C (par `p8_poke` ou une boucle équivalente, rien de plus : l'étape 5 typera la palette de dessin).
- **Plage** : `flr(no)` hors de 0..15 est un `assert` porteur de message. Aujourd'hui un tel indice lirait la mémoire voisine de `0x5000` (des zéros, ou pire au-delà) ; aucun appelant n'y arrive : `toast_draw` passe `fade` ∈ [0, 5] (le toast meurt à 5), le comptoir passe 15 (archétype) ou `8 + (t mod 42)/6` ∈ [8, 15), `layout.c` le troisième élément d'un tuple de ligne de données. Expérience faite : `make test`, `make e2e` et `make sanitize-net` passent avec l'`assert`. Un message d'`assert` est un des commentaires que le projet garde (`CLAUDE.md`).
- **Orphelins** : `hud_build_palette_table()` part (prototype, doc, appel de `cart_init()`, appels et commentaire des deux TU, et leur `#include "game/hud.h"` s'il ne sert plus à rien d'autre). `p8_fixed_shl` perd son seul appelant dans `src/` mais **reste**, avec ses TU : le plan garde la virgule fixe (décision de triage).
- **Commentaires à corriger** : `hud.h` (doc des deux fonctions : plus de « 0x5000 », plus de « hud_build_palette_table() wrote ») ; `src/p8/gfx.c` dans `p8_map` (« sprite 0 is part of the palette table hud_build_palette_table() reads » → le sprite 0 porte les pixels des blocs de palette, que `hud.c` recopie en table) ; `src/p8/mem.h` (« p8_palt() and hud_select_palette_block() write these same bytes » reste vrai) ; `tests/test_gfx.c` l. 107–111 (parle de la `memcpy` de `hud_select_palette_block()` : à reformuler en « l'écriture de la palette de dessin ») ; `tests/test_fixed.c` l. 305 part avec son test.
- `docs/trous-de-portage.md` : aucun GAP ne disparaît (GAP-18, remap de `pal()`, reste vrai et nomme toujours `hud_select_palette_block`) ; vérifier seulement qu'aucune phrase n'y parle de `0x5000` ou de `hud_build_palette_table`.

### Chunk B — Suppression de `pico8/map.json`

Inventaire (HEAD `a1cbeb4`) :
- `src/p8/assets.c` : `p8_load_map_json()`, `map_cell_sink()`, le `#define P8_MAP_BASE_ROWS` local et son commentaire ; `src/p8/assets.h` : prototype et la moitié « map » de la doc.
- `src/cmd/replay.c` : l. 104 (chargement) et l. 1 (commentaire « loads pico8/gfx.json and pico8/map.json ») ; `src/cmd/game.c` : l. 61.
- `src/p8/gfx.c` : commentaire de `P8_MAP_BASE_ROWS` (« p8/assets.c's map_cell_sink holds the same split and is kept in sync by hand ») — la phrase part, le partage des lignes 0–31 / 32–63 reste dans `map_cell_addr`.
- `pico8/map.json` : `git rm`.
- Tests : 43 fichiers appellent `p8_load_map_json`.
  - **41 ne font que le charger à l'installation** (une ligne `assert(p8_load_map_json(&memory|&mem, "pico8/map.json"));`, supprimée telle quelle) : `test_ascension.c`, `test_background.c`, `test_burst.c`, `test_button.c`, `test_camera.c`, `test_data_init.c`, `test_economy.c`, `test_entity_header.c`, `test_entity_kind.c`, `test_exit_queue.c`, `test_frame_zone.c`, `test_game_zone.c`, `test_help_screen.c`, `test_infraspace_gate.c`, `test_layout_row.c`, `test_live_slots.c`, `test_main_camera.c`, `test_main_economy.c`, `test_main_mouse.c`, `test_message_box.c`, `test_modal_backdrop.c`, `test_mouse.c`, `test_planet.c`, `test_planet_spec_defaults.c`, `test_probe.c`, `test_processor.c`, `test_protostar.c`, `test_research_project.c`, `test_score_panel.c`, `test_slipgate.c`, `test_slipway.c`, `test_spec_defaults.c`, `test_structure.c`, `test_structure_icon.c`, `test_synthesizer.c`, `test_toast.c`, `test_tooltip_draw_at.c`, `test_trade.c`, `test_trading_hub.c`, `test_ui.c`, `test_wormhole.c`. Leur `#include "p8/assets.h"` reste (ils chargent `gfx.json`).
  - **`test_demo.c`** lit le contenu : il perd le chargement et l'`assert` sur `P8_ADDR_SPRITE_MAP_SHARED + 1 == 81` (et son commentaire), et pose avant son `p8_map(…, 0, 0, 0, 100, 4, 1)` les cellules de la ligne 0 qu'il dessine : `p8_mset` de (1,0)=81, (2,0)=85, (3,0)=1 — (0,0) vaut déjà 0 après `p8_mem_init`. Ses asserts de pixels (bloc 0..7 vide, `(16,100) == 1`, `(31,107) == 6`) restent tels quels.
  - **`test_asset_loading.c`** perd ses quatre tests de carte (`test_map_json_loads_base_region`, `…_shared_region_for_rows_32_to_63`, `test_gfx_and_map_loaded_into_same_instance`, `test_map_json_missing_file_returns_failure`) ; restent les trois tests `gfx`. Les asserts « gfx » du test mixte sont déjà couverts par `test_gfx_json_loads_known_pixel` et `…_full_128x128_range`.
- `Makefile`, `tools/`, `README`, `docs/` (hors plans et PRD) : aucune mention de `map.json` à HEAD — rien à changer. `tests/test_gfx.c` (`p8_mset`/`p8_map` sur mémoire vierge) et `tests/test_mem.c` (adresses de carte) ne lisent pas le fichier : ils restent.

**Pourquoi aucun pixel ne change (preuve statique)** :
1. Le seul `p8_map` de `src/` est `background_draw()` : cellules `(0..31, 0..31)`, origine écran `(0, 0)`. `p8_mget` n'existe pas ; aucun `p8_peek` de `src/` ne vise `0x1000..0x2fff` hors de `p8/gfx.c`.
2. `background_construct()` écrit par `p8_mset` les 32×32 cellules de cette fenêtre, sans condition (deux boucles pleines `y`, `x` de 0 à 31 ; une tuile hors des choix vaut 0, écrite aussi).
3. `background_create()` appelle `background_construct()` **avant** `entity_construct()`, qui enregistre le fond parmi les entités et le message `draw` : aucun dessin du fond ne précède l'écriture de ses cellules. Le fond n'est créé que par `cart_start_game()` (après le clic de difficulté) ou par un TU.
4. Les lignes 32–63, et les colonnes 32–127, ne sont lues par rien.
5. La feuille de sprites vit à `0x6000` (déballée, 16 Ko) : le chargement de la carte à `0x1000`/`0x2000` ne la recouvrait pas (`test_gfx_and_map_loaded_into_same_instance` le vérifiait), donc son absence ne la change pas non plus.

### Communs

- Aucune signature exportée ne change, hors suppressions : `hud_build_palette_table`, `p8_load_map_json`.
- `background.c`, `p8_map`, `p8_mset`, `P8_ADDR_MAP`, `P8_ADDR_SPRITE_MAP_SHARED`, `p8_sget`, `p8_load_gfx_json` ne bougent pas.
- L'étape ne touche à aucune chaîne d'état, de tag, de message ou de fonction de dessin (périmètre de l'étape 3) : les lignes modifiées de `cart.c`, `layout.c`, des TU sont disjointes de celles que l'étape 3 migre (hypothèses plus bas).

## Implementation Chunks

Les deux chunks sont indépendants ; l'ordre A puis B va du changement qui refige des hachages à celui qui n'en refige aucun. Un commit par chunk, chacun avec `make test-all` et `make sanitize-net` verts et les 26 `*.sha256` inchangés.

1. **Chunk A — Table des palettes en C.**
   1. Ajouter `tests/test_hud.c` (cible au `Makefile`, entrée de `UNIT_TESTS`) avec le test d'égalité « bloc sélectionné = pixels de `gfx.json` » — vert sur HEAD, puisqu'il appelle d'abord l'ancienne `hud_build_palette_table()` ; et le test « sans initialisation, sur une feuille vierge » — **rouge** sur HEAD.
   2. Table C dans `hud.c`, `hud_select_palette_block()` la lit, `assert` de plage ; le premier test perd son appel à `hud_build_palette_table()` et reste vert, le second passe au vert.
   3. Supprimer `hud_build_palette_table()` et ses appels (`cart.c`, deux TU) ; corriger les commentaires listés.
   4. Refiger les trois hachages de `test_layout_row.c` (Testing Decisions), la preuve dans le message de commit.
   Fichiers : `src/game/hud.c`, `src/game/hud.h`, `src/game/cart.c`, `src/p8/fixed.c`, `src/p8/fixed.h`, `src/p8/gfx.c` (commentaire), `tests/test_hud.c` (nouveau), `tests/test_layout_row.c`, `tests/test_layout_nil_item_draws_nothing.c`, `tests/test_tooltip_draw_at.c`, `tests/test_fixed.c`, `tests/test_gfx.c` (commentaire), `Makefile`.
2. **Chunk B — Sans `map.json`.**
   1. Supprimer la ligne de chargement des 41 TU, adapter `test_demo.c`, retirer les quatre tests de carte de `test_asset_loading.c`.
   2. Supprimer le chargement de `replay.c` et `game.c`, `p8_load_map_json()` et `map_cell_sink()`, corriger les commentaires (`replay.c`, `assets.h`, `gfx.c`).
   3. `git rm pico8/map.json`.
   Fichiers : `src/p8/assets.c`, `src/p8/assets.h`, `src/p8/gfx.c` (commentaire), `src/cmd/replay.c`, `src/cmd/game.c`, `pico8/map.json`, les 43 fichiers de tests listés.
   Vérifier aussi `make raylib` (lie `game.c`) quand le vendoring est en place, sans le lancer.

## Testing Decisions

- **Ce qu'est un bon test ici** : il observe ce que voit le reste du jeu — les 16 octets de la palette de dessin après `hud_select_palette_block()`, les pixels de l'écran — jamais la forme de la table ni une adresse interne. La table C n'est pas exposée.
- **Nouveau `tests/test_hud.c`** (un `test_<module>.c` par module, règle du projet ; prior art : `tests/test_palette.c`, qui affirme une table constante valeur par valeur, et `tests/test_gfx.c` pour lire `P8_ADDR_DRAW_PALETTE`) :
  - **égalité avec les sprites** : `gfx.json` chargé, pour chaque bloc `n` de 0 à 15, `hud_select_palette_block(mem, n)`, puis pour chaque couleur `c` l'octet `P8_ADDR_DRAW_PALETTE + c` vaut `p8_sget(mem, n, c) | (c == 3 ? 0x80 : 0)`. C'est la « table bâtie depuis les sprites » du plan, recalculée dans le test : elle survit à la suppression de `hud_build_palette_table()`. Pas de hachage : une égalité octet par octet dit lequel diverge ;
  - **sans initialisation** : sur une mémoire juste initialisée (feuille vierge, rien à `0x5000`), `hud_select_palette_block(mem, 8)` donne les 16 octets du bloc 8 (le bloc de comptoir, distinct de l'identité). Rouge avant, vert après : c'est le test qui prouve que la table ne vient plus de la mémoire ;
  - **partie fractionnaire** : `no = 8,9` sélectionne le bloc 8 (`flr`, comme `toast_draw` et le comptoir le passent).
  - Pas de test de l'`assert` hors plage (il arrêterait le binaire ; les TU du projet ne testent pas leurs `assert`).
- **Hachages refigés (seule exception à « rien ne bouge »)** : `tests/test_layout_row.c` dessine des lignes après `cart_load()` sans jamais appeler `cart_init()` ni `hud_build_palette_table()` ; ses hachages d'écran ont été figés avec une table de zéros (couleur 3 opaque), un état que le jeu n'atteint jamais. Mesuré sur une copie de HEAD :
  | Hachage | HEAD | HEAD + `hud_build_palette_table()` après `cart_load()` | Chunk A |
  |---|---|---|---|
  | `CONSTRUCTIONS_HASH` | `0xE8FF55CB` | `0xC4F7222D` | `0xC4F7222D` |
  | `PLANET_RESOURCE_ROW_HASH` | `0xA122C4CC` | `0x82417F7B` | `0x82417F7B` |
  | `CALLS_HASH` | `0x26A3DB53` | `0x398594A5` | `0x398594A5` |
  `RESEARCH_PROJECT_LAYOUT_ROW_HASH` et `PANEL_HASH` ne bougent pas. La colonne du milieu est la preuve à recopier dans le message de commit : la nouvelle valeur est celle du rendu réel, pas une valeur acceptée faute de mieux. Aucun autre TU ne change (expérience : `make -k test` vert une fois ces trois valeurs refigées).
- **Chunk B** : aucun nouveau test ; aucun hachage ne bouge. Prior art de la régénération du fond : `test_map` de `tests/test_background.c`, qui hache les 32×32 cellules après `start_game()` (`MAP_HASH`, `MAP_STARS`, `MAP_ZEROS`, `SECOND_MAP_HASH`) et passe sans `map.json` — c'est la preuve unitaire que le fond écrit toute sa fenêtre lui-même. `test_demo.c` garde sa vérification de `p8_map` avec ses cellules posées à la main.
- **Non-régression des pixels** : les 26 `*.sha256` de `tests/e2e/` inchangés à chaque chunk, aucun `make e2e-baseline`. Expérience faite sur une copie de HEAD avec les deux chunks appliqués : `make e2e` → 26 « matches », `make sanitize-net` → « sanitize: 26 clean » (dont `test_live_slots`, qui chargeait la carte).
- **Durées** : mesurées sur la même machine, HEAD et HEAD + chunks A et B, trois passes alternées, à chaud : `make test` 4,7 / 6,8 / 6,7 s contre 7,5 / 6,7 / 6,7 s ; `make e2e` 0,34 / 0,24 / 0,32 s contre 0,30 / 0,25 / 0,30 s — dans le bruit. L'étape retire 41 lectures de JSON de 22 Ko et ajoute un binaire de test minuscule ; refaire la mesure au commit (même protocole : alternance, à chaud).
- **Critère de fin** (plan, règle 3) : `git grep -n "map\.json\|p8_load_map_json\|map_cell_sink\|hud_build_palette_table\|0x5000" -- src tests` ne rend plus que les écritures manuelles de `test_gfx.c` et `test_mem.c` sur `0x5000` (tests de `p8_memcpy`, hors périmètre) ; `pico8/` ne contient plus que `gfx.json`, `lua.lua`, `original.p8.png`.

## Out of Scope

- Typer la mémoire (`p8_memory_t`, `peek`/`poke`/`memcpy`, régions de carte, palette de dessin) : étape 5. La table C est écrite dans la palette de dessin par l'API mémoire existante.
- Les écritures manuelles de `0x5000` dans `tests/test_gfx.c` et `tests/test_mem.c` (elles testent `p8_memcpy`).
- `p8_map`, `p8_mset`, la séparation lignes 0–31 / 32–63 de `map_cell_addr`, `P8_MAP_WIDTH`/`P8_MAP_HEIGHT` : le fond s'en sert ; l'étape 5 décide de la forme des « tuiles du fond 32×32 ».
- Sortir d'autres données de `gfx.json` (sprites, polices) : les sprites restent un asset ; leur format sur PSX est au plan 10.
- Le remap de couleurs de `pal()` (GAP-18) : il changerait des pixels.
- Les chaînes d'état, de tag, de message, de fonction de dessin, de bouton de souris : étape 3.
- `pico8/lua.lua` et `pico8/original.p8.png` (archive).

## Further Notes

- Les points « vérifier avec l'utilisateur » de `to-prd` (modules, tests à écrire, découpage) n'ont pas été posés à l'utilisateur : la skill `guideline` a été appliquée à leur place. Ce qu'elle a tranché :
  - **simplicité** : la table reste privée à `hud.c`, sans accesseur ; le test passe par `hud_select_palette_block()` et la palette de dessin, ce qui teste à la fois la table et la sélection. Pas de nouveau module « palettes » pour 256 octets et une fonction ;
  - **chirurgie** : `background.c`, `p8_map`/`p8_mset` et les régions de carte ne bougent pas ; seuls partent les orphelins que l'étape crée (`hud_build_palette_table`, `p8_load_map_json`, `map_cell_sink`). `p8_fixed_shl` reste : le plan garde la virgule fixe (tranché au triage) ;
  - **hypothèses en surface** : l'`assert` de plage remplace une lecture hors table qu'aucun chemin connu n'atteint (inventaire des trois appelants plus haut) ; les trois hachages refigés sont un changement de valeur assumé, prouvé égal au rendu réel ;
  - **critère vérifiable** : sha256 identiques, `make sanitize-net` vert, égalité octet par octet avec `gfx.json`, `git grep` de fin.
- Le plan dit « vérifiée par un TU contre la table bâtie depuis les sprites avant que celle-ci disparaisse » : l'ordre interne du chunk A (TU vert sur l'ancienne table, puis bascule, puis suppression) le respecte dans un seul commit ; le TU garde ensuite sa référence en relisant `gfx.json` lui-même.
- L'expérience citée a été faite dans une copie de HEAD (`git archive`) hors du dépôt, pour ne lire aucun changement non commité de l'étape 3 ; elle n'a rien laissé dans le dépôt.

## Hypothèses sur l'issue de l'étape 3

L'étape 3 (chaînes → `enum`, `docs/prd/triage/phase6-etape3-chaines-enum.md`) est implémentée en parallèle, chunk par chunk ; ce PRD est écrit contre HEAD `a1cbeb4` (étapes 1 et 2 faites, étape 3 pas commencée dans HEAD). L'étape 4 a été dessinée pour ne dépendre d'aucune surface de l'étape 3 ; ce qui suit est ce qu'elle suppose malgré tout.

- **H-3-1** — L'étape 3 ne change ni la signature ni le contrat de `hud_select_palette_block(p8_memory_t *, p8_fixed_t)`, `hud_select_palette_block_default(p8_memory_t *)` et `hud_build_palette_table(p8_memory_t *)`, et n'ajoute aucun appelant à `hud_build_palette_table()`. (`hud.c` ne contient aucune chaîne d'état, de tag ou de message ; la famille B de l'étape 3 peut toucher `hud_draw_bar`/`hud_print_outlined`/`hud_draw_help_frame` comme fonctions de dessin dispatchées, que l'étape 4 ne touche pas.)
- **H-3-2** — `cart_init()` garde l'appel `hud_build_palette_table(mem)` avant `reset_world()` ; l'étape 3 (famille C : `draw_layers` dans `cart_draw`) ne touche que des lignes voisines de `cart.c`, pas celle-là.
- **H-3-3** — L'étape 3 ne change pas les valeurs de `CONSTRUCTIONS_HASH`, `PLANET_RESOURCE_ROW_HASH`, `CALLS_HASH` de `tests/test_layout_row.c` (son PRD n'annonce qu'un hachage refigé, `ENTITY_STATE_HASH` de `test_ui.c`), bien que sa famille B touche ce fichier (`"draw_row"` ×2). Si elle les changeait, les nouvelles valeurs du chunk A se recalculent par le même protocole : HEAD post-étape-3 + `hud_build_palette_table(&memory)` après `cart_load()` dans `main`, valeurs imprimées par `check()`.
- **H-3-4** — `test_layout_row.c` n'appelle toujours ni `cart_init()` ni `hud_build_palette_table()` avant ses dessins, les autres TU qui dessinent gardent leur initialisation actuelle, et l'étape 3 n'ajoute aucun TU qui dessine sans table (sinon d'autres hachages bougeraient au chunk A — `make -k test` les révélerait, à traiter par le même protocole).
- **H-3-5** — L'étape 3 ne déplace ni ne supprime la ligne `assert(p8_load_map_json(&memory|&mem, "pico8/map.json"));` des 41 TU listés, et n'ajoute aucun TU qui charge `map.json`. Elle peut modifier des lignes voisines (états, tags, messages) dans ces mêmes fichiers : le chunk B supprime une ligne textuellement identique, sans dépendre de son contexte.
- **H-3-6** — L'étape 3 ne touche ni `background_construct()`, ni `background_draw()`, ni `background_create()` dans l'ordre `background_construct` → `entity_construct`, ni `p8_map`/`p8_mset` (sa famille D ne change dans `background.c` que `entities_by_tag("probe")` de `background_left_press`). La preuve statique du chunk B repose sur cet ordre.
- **H-3-7** — L'étape 3 laisse les 26 `*.sha256` de `tests/e2e/` inchangés et les durées de référence (`make test` ~4,5 s, `make e2e` ~0,25 s chaud) valables ; les références de l'étape 4 sont donc les mêmes avant et après elle.
- **H-3-8** — L'étape 3 ne touche ni `src/p8/assets.c/.h`, ni `src/cmd/replay.c`/`game.c` (chargements), ni `tests/test_demo.c`, `tests/test_asset_loading.c`, `tests/test_hud.c` (inexistant), ni `p8_fixed_shl`.

## À revérifier une fois l'étape 3 faite

- **Implementation Decisions → Chunk A → Inventaire** : la liste des appelants de `hud_select_palette_block()`/`hud_build_palette_table()` et leurs signatures (H-3-1, H-3-2) — refaire `git grep -n "hud_build_palette_table\|hud_select_palette_block\|p8_fixed_shl"`.
- **Implementation Decisions → Chunk B → Inventaire** : la liste des 41 TU (H-3-5) — refaire `git grep -l p8_load_map_json -- tests`.
- **Implementation Decisions → Chunk B → Preuve statique** : l'ordre de `background_create()` et le corps de `background_construct()` (H-3-6).
- **Testing Decisions → Hachages refigés** : les valeurs HEAD et cibles de `test_layout_row.c` (H-3-3), et l'absence d'autre TU qui dessine sans table (H-3-4) — `make -k test` après le chunk A.
- **Testing Decisions → Durées et sha256** : seuils et références toujours valables (H-3-7).
- **Implementation Chunks** : les listes de fichiers, en cas de conflit textuel avec un chunk de l'étape 3 commité entre-temps (H-3-8).
