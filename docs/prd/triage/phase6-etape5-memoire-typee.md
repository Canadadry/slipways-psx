---
title: "Plan 6, étape 5 — Mémoire typée"
description: "Le runtime range encore ce que dessine le jeu dans un bloc de 56 Ko adressé comme la RAM de PICO-8 (`p8_memory_t`, `p8_peek`/`p8_poke`/`p8_memcpy`) alors que le jeu n'en utilise que quatre régions, sans recouvrement, qui peuvent devenir des champs typés."
status: needs-triage
---

## Problem Statement

Tout ce que le jeu dessine passe par `p8_memory_t`, un tableau `unsigned char data[0xE000]` adressé par des constantes PICO-8 (`P8_ADDR_*`) et lu ou écrit par `p8_peek`, `p8_poke` et `p8_memcpy`. Or ce bloc n'est plus une RAM PICO-8 : la feuille de sprites y est déballée (un octet par pixel), l'écran aussi, et aucune région ne recouvre une autre. Il ne reste que quatre régions vivantes, chacune avec deux ou trois lecteurs :

| Région | Adresses (HEAD) | Taille utile | Ce qui la lit ou l'écrit dans `src/` |
|---|---|---|---|
| Palette de dessin | `0x5f00..0x5f0f` (`P8_ADDR_DRAW_PALETTE`) | 16 o | `p8_mem_init`, `p8_palt`, `p8_gfx_is_transparent` (`p8/gfx.c`), `hud_select_palette_block` (`game/hud.c`, une `p8_memcpy` depuis `0x5000` à HEAD, voir H-4-2) |
| Carte | `0x1000..0x1fff` (lignes 0–31, `P8_ADDR_SPRITE_MAP_SHARED`) et `0x2000..0x2fff` (lignes 32–63, `P8_ADDR_MAP`) | 1 Ko lu : le jeu n'écrit et ne lit que les cellules `(0..31, 0..31)` | `p8_mset`, `map_cell_get`, `map_cell_addr`, `p8_map` (`p8/gfx.c`) ; `map_cell_sink` (`p8/assets.c`, supprimé par l'étape 4, H-4-3) ; `background_construct` / `background_draw` (`game/background.c`) par `p8_mset`/`p8_map` |
| Feuille de sprites | `0x6000..0x9fff` (`P8_ADDR_SPRITE_SHEET`) | 16 Ko, 128×128 | `p8_sget` (`p8/gfx.c`) ; `gfx_pixel_sink` (`p8/assets.c`) |
| Écran | `0xA000..0xDFFF` (`P8_ADDR_SCREEN`) | 16 Ko, 128×128 | `p8_cls`, `p8_pset` (`p8/gfx.c`) ; `dump_screen` (`devtools/dump.c`) ; `screencast_write_frame` (`devtools/screencast.c`) ; `raylib_backend_present` (`platform/raylib_backend.c`) |
| Table des blocs de palette | `0x5000..0x50ff` | — | morte après l'étape 4 (H-4-1) |

Le reste des 56 Ko (`0x0000..0x0fff`, `0x3000..0x4fff`, `0x5100..0x5eff`, `0x5f10..0x5fff`) n'est lu ni écrit par rien dans `src/`. Conséquences :

- un accès hors région ne se voit pas : `p8_poke` ne refuse qu'au-delà de `0xE000`, donc un `gfx.json` de plus de 16 384 pixels déborderait dans l'écran, et une cellule de carte `(x ≥ 128)` tomberait sur la ligne suivante ;
- les tests parlent d'adresses PICO-8 (`P8_ADDR_SCREEN + y * P8_SCREEN_WIDTH + x`) là où ils veulent dire « le pixel (x, y) » ;
- la carte garde la forme 128×64 du cart alors que le jeu n'en a jamais lu que 32×32 ;
- le plan 6 retire du cœur ce qui n'existe que pour PICO-8, et le modèle « RAM adressable » en est le dernier gros morceau avant le portage PSX.

## Solution

`p8_memory_t` devient une structure de champs typés, un champ par région vivante, et l'API mémoire disparaît :

- `draw_palette[16]` — même encodage d'octet qu'aujourd'hui (couleur dans les bits bas, `P8_DRAW_PALETTE_TRANSPARENT_BIT` = `0x80` pour « non dessiné ») ;
- `map_cells[32][32]` — les tuiles du fond, `[y][x]` ; `P8_MAP_WIDTH`/`P8_MAP_HEIGHT` passent à 32 ;
- `sprite_sheet[128][128]` — `[y][x]` ;
- `screen[128][128]` — `[y][x]`.

Les primitives de dessin (`p8_cls`, `p8_pset`, `p8_palt`, `p8_sget`, `p8_spr`, `p8_mset`, `p8_map`, `p8_print`…) gardent leurs signatures et leurs bornes ; elles lisent et écrivent les champs. `p8_peek`, `p8_poke`, `p8_memcpy`, `P8_MEM_SIZE` et toutes les constantes `P8_ADDR_*` disparaissent. Au dernier chunk, le type et son module prennent un nom qui dit ce qu'ils sont : `p8_vram_t` dans `p8/vram.h`/`p8/vram.c`, initialisé par `p8_vram_init`.

Rien ne change pour le joueur : mêmes 26 `*.sha256` de `tests/e2e/`, aucun hachage de TU refigé, `make test` (~4,5 s) et `make e2e` (~0,25 s) pas plus lents, `make sanitize-net` vert, `make raylib` compile et lie. La structure finale fait 33 808 octets au lieu de 57 344.

## User Stories

1. En tant que mainteneur, je veux lire `mem->screen[y][x]`, pour comprendre un test de pixel sans refaire une arithmétique d'adresse PICO-8.
2. En tant que mainteneur, je veux que l'écran, la feuille de sprites, les tuiles et la palette de dessin soient quatre champs nommés, pour voir d'un coup d'œil tout ce que le runtime garde en mémoire.
3. En tant que mainteneur, je veux que `p8_peek`/`p8_poke`/`p8_memcpy` disparaissent, pour qu'aucun code ne puisse plus écrire dans une région par une adresse calculée.
4. En tant que mainteneur, je veux que toutes les constantes `P8_ADDR_*` et `P8_MEM_SIZE` disparaissent, pour que rien ne rappelle une carte mémoire qui n'existe plus.
5. En tant que mainteneur, je veux des tableaux à deux dimensions `[y][x]`, pour qu'UBSan (`make sanitize-net`) signale un indice de colonne hors borne au lieu de le laisser tomber sur la ligne suivante.
6. En tant que mainteneur, je veux que la carte ne fasse plus que 32×32, pour que sa taille dise ce que le jeu en lit.
7. En tant que mainteneur, je veux qu'un `p8_mset` hors de 32×32 ne fasse rien et qu'un `p8_map` y lise une cellule vide, pour que le contrat de bornes des primitives reste celui d'aujourd'hui.
8. En tant que mainteneur, je veux que le chargeur de `gfx.json` ignore les pixels hors de 128×128, pour qu'un fichier trop grand ne puisse plus écrire dans l'écran.
9. En tant que mainteneur, je veux que la palette de dessin s'initialise comme aujourd'hui (identité, couleur 0 transparente), pour garder l'état de mise sous tension de `palt()`.
10. En tant que mainteneur, je veux que `p8_palt()` et `hud_select_palette_block()` écrivent le même champ `draw_palette`, pour garder la règle « un seul stockage » (un drapeau parallèle a déjà fait perdre leur contour à toutes les icônes).
11. En tant que mainteneur, je veux que `hud_select_palette_block()` copie son bloc dans `draw_palette` par un `memcpy` C, pour qu'il ne reste aucune copie par adresse.
12. En tant que mainteneur, je veux que `dump_screen()` passe `mem->screen` à `dump_pixels()` sans recopie octet par octet, pour que le dump lise ce qu'il dumpe.
13. En tant que mainteneur, je veux que `screencast_write_frame()` lise le champ écran, pour que la capture reste un observateur en lecture seule.
14. En tant que mainteneur du backend raylib, je veux que `raylib_backend_present()` lise le champ écran, pour que `make raylib` compile et lie à chaque chunk.
15. En tant que mainteneur, je veux que chaque chunk ne déplace qu'une région, pour qu'un rejeu rouge désigne une seule région.
16. En tant que mainteneur, je veux qu'à la fin de chaque chunk la constante `P8_ADDR_*` de sa région n'apparaisse plus dans `src/` ni dans `tests/`, pour qu'un `git grep` suffise à dire qu'une région est finie.
17. En tant que mainteneur, je veux que les 21 TU qui hachent l'écran gardent leurs constantes de hachage, pour prouver que le parcours ligne par ligne du champ rend les mêmes octets dans le même ordre.
18. En tant que mainteneur, je veux que les TU qui écrivent un pixel ou un sprite « à la main » écrivent le champ, pour que leurs préparations restent lisibles.
19. En tant que mainteneur, je veux que les tests de bornes de `test_mem.c` partent avec les adresses qu'ils testaient, pour ne pas garder des tests d'une API morte.
20. En tant que mainteneur, je veux un `test_vram.c` qui garde le test d'initialisation (tout à zéro sauf la palette de dessin identité, couleur 0 transparente), pour que l'état de mise sous tension reste vérifié.
21. En tant que mainteneur, je veux un test « un `p8_mset` hors de 32×32 ne change aucune cellule », pour couvrir la nouvelle borne de la carte.
22. En tant que mainteneur, je veux un test « un `gfx.json` de plus de 128×128 valeurs ne touche ni l'écran ni rien d'autre », pour couvrir la nouvelle borne du chargeur.
23. En tant que mainteneur, je veux les 26 `*.sha256` inchangés après chaque chunk, sans `make e2e-baseline`, pour tenir l'invariant « aucun pixel ne change ».
24. En tant que mainteneur, je veux `make test` et `make e2e` pas plus lents, pour garder la boucle rapide (règle du projet).
25. En tant que mainteneur, je veux `make sanitize-net` vert après chaque chunk, pour qu'ASan/UBSan confirment qu'aucun rejeu ne sort d'un tableau.
26. En tant que mainteneur, je veux que le type final s'appelle `p8_vram_t` et son module `p8/vram`, pour que le nom suive la règle `module_verbe` (`p8_vram_init`) et ne dise plus « mémoire PICO-8 ».
27. En tant que portage PSX (plans 7–11), je veux quatre tableaux de taille fixe, pour savoir exactement quoi poser en RAM et quoi envoyer au GPU (l'écran et la palette de dessin, plan 10).
28. En tant qu'exécutant de l'étape, je veux une liste d'hypothèses sur l'issue de l'étape 4, pour savoir quoi revérifier avant de commencer.
29. En tant que mainteneur, je veux que les commentaires qui parlent d'adresses (`0x5f00`, `P8_ADDR_SCREEN`, `p8_peek`, « PICO-8 memory ») soient corrigés, pour qu'aucun ne mente après l'étape.

## Implementation Decisions

### Inventaire (HEAD `67b39c5`, étape 4 non faite)

**`src/`** — appels à `p8_peek`/`p8_poke`/`p8_memcpy` et constantes d'adresse, hors `p8/mem.c` et `p8/mem.h` :

| Fichier | Lignes | Région | Accès |
|---|---|---|---|
| `p8/gfx.c` | 18 (`p8_cls`), 28 (`p8_pset`) | écran | `p8_poke` |
| `p8/gfx.c` | 47–54 (`p8_palt`), 61 (`p8_gfx_is_transparent`) | palette de dessin | `p8_peek`/`p8_poke` |
| `p8/gfx.c` | 227 (`p8_sget`) | sprites | `p8_peek` |
| `p8/gfx.c` | 270–294 (`P8_MAP_BASE_ROWS`, `map_cell_addr`, `p8_mset`, `map_cell_get`) | carte | `p8_peek`/`p8_poke` |
| `p8/assets.c` | 104–105 (`gfx_pixel_sink`) | sprites | `p8_poke` |
| `p8/assets.c` | 113–131 (`map_cell_sink`, `p8_load_map_json`) | carte | `p8_poke` — supprimés par l'étape 4 (H-4-3) |
| `game/hud.c` | 22–33 (`hud_build_palette_table`), 36–41 (`hud_select_palette_block`) | table `0x5000`, palette de dessin | `p8_poke`, `p8_memcpy` — remplacés par l'étape 4 (H-4-1, H-4-2) |
| `devtools/dump.c` | 33 | écran | `p8_peek` |
| `devtools/screencast.c` | 31–32 | écran | `p8_peek` |
| `platform/raylib_backend.c` | 105 | écran | `p8_peek` |

`p8/mem.c` : `p8_mem_init` (`memset` + identité de la palette à `data[P8_ADDR_DRAW_PALETTE + c]`), `addr_in_range`, `p8_peek`, `p8_poke`, `p8_memcpy` (`memmove`). `p8/mem.h` : les cinq `P8_ADDR_*`, `P8_DRAW_PALETTE_TRANSPARENT_BIT`, `P8_MEM_SIZE`, le type. Commentaires qui nomment une adresse ou l'API : `mem.h` (doc de la palette, contrat de bornes), `gfx.h` l. « Flips the 0x80 bit of the draw-palette byte at 0x5f00 », `gfx.c` (`P8_MAP_BASE_ROWS`, et dans `p8_map` le « POKE(0x5F36,0x8) » qui, lui, décrit le cart et reste), `assets.h` (« dropped by p8_poke »), `raylib_backend.h` l. 59 (« p8_peek from P8_ADDR_SCREEN »), `screencast.h` l. 45 (« memory (P8_ADDR_SCREEN) »), `hud.h` (H-4-2).

Le type `p8_memory_t` lui-même : 135 occurrences dans 49 fichiers de `src/`, 123 dans 54 fichiers de `tests/` ; `p8/mem.h` inclus par 80 fichiers (26 de `src/`) ; `p8_mem_init` : 119 appels dans 58 fichiers ; `src/p8/mem.c` cité 14 fois dans le `Makefile` (listes d'objets, dont la cible `$(TMP_DIR)/game` de `make raylib`). Instances : une sur la pile dans `cmd/replay.c` et `cmd/game.c`, transmise par `dispatch_context_set_mem()`/`dispatch_context_mem()` (77 occurrences dans 64 fichiers), et une par TU.

**Le backend raylib** (`make raylib`, vendoré, compilé et lié, jamais lancé) ne lit que l'écran : `raylib_backend_present(…, const p8_memory_t *mem, …)` parcourt 128×128 `p8_peek(mem, P8_ADDR_SCREEN + y*128 + x)` et convertit chaque indice par `P8_PALETTE_RGB` (`p8/palette.c`, une table constante, pas la mémoire). Il ne lit ni la palette de dessin, ni les sprites, ni la carte. `cmd/game.c` crée la mémoire, charge `gfx.json` (et `map.json` jusqu'à l'étape 4) et passe `&memory` à `raylib_backend_present`. Le chunk écran et le chunk final touchent donc `raylib_backend.c/.h` et `game.c`.

**`tests/`** — accès bruts :

- **Hachage de l'écran** (boucle FNV `h ^= p8_peek(&memory, P8_ADDR_SCREEN + i)` sur 128×128) : 21 fichiers — `test_ascension.c`, `test_background.c` (l. 71), `test_burst.c`, `test_button.c`, `test_help_screen.c` (l. 232), `test_infraspace_gate.c`, `test_layout_row.c`, `test_message_box.c`, `test_modal_backdrop.c`, `test_probe.c`, `test_protostar.c`, `test_research_project.c`, `test_score_panel.c`, `test_slipgate.c`, `test_slipway.c`, `test_structure_icon.c`, `test_synthesizer.c`, `test_toast.c`, `test_tooltip_draw_at.c`, `test_ui.c`, `test_wormhole.c`.
- **Lecture d'un pixel** (`P8_ADDR_SCREEN + y*W + x`) : `test_background_draw.c` (l. 15), `test_demo.c` (l. 14), `test_layout_nil_item_draws_nothing.c` (l. 49), `test_draw_layers_camera.c` (l. 101–125, poke puis peek d'un pixel, et un `assert` sur l'adresse calculée), `test_help_screen.c` (l. 515–518, poke puis peek).
- **Écriture de l'écran** : `test_dump.c` (l. 137), `test_headless_capture.c` (l. 74), `test_screencast.c` (l. 22 `paint_frame`, l. 329–332).
- **`test_screencast.c` l. 290–313** : `test_writing_a_frame_leaves_the_game_memory_untouched` pose `0x42` à `P8_ADDR_MAP` (un octet hors écran) puis compare toute la structure par `memcmp` avant/après capture.
- **`test_background.c` l. 183** : `mget(x, y)` lit `P8_ADDR_SPRITE_MAP_SHARED + y*P8_MAP_WIDTH + x`.
- **`test_demo.c` l. 28–30** : `assert` sur la cellule (1,0) de `map.json` — supprimé par l'étape 4 (H-4-5).
- **`test_asset_loading.c`** : 20 accès ; après l'étape 4 il ne reste que les trois lectures de sprites (l. 14, 24, 25) (H-4-4).
- **`test_gfx.c`** (93 accès) : écran dans presque tous les tests ; palette de dessin l. 84–126 (dont `test_draw_palette_write_overrides_a_previous_palt`, qui écrit 16 octets à `0x5000` et les `p8_memcpy` vers `P8_ADDR_DRAW_PALETTE` — gardé par l'étape 4, H-4-1) ; sprites l. 350, 367–368, 388–391, 412, 478, 503 ; carte l. 424–531 (`test_mset_writes_base_region_cell`, `test_mset_writes_shared_upper_region_cell` lignes 32 et 63, `test_mset_out_of_range_is_noop` qui balaie les deux régions, `test_map_and_mset_round_trip_against_real_sheet` à la ligne 40).
- **`test_mem.c`** (58 accès) : init, aller-retour peek/poke à `0x1234`, une frontière par région (sprites, partagée, carte, écran), hors plage peek/poke, six tests de `p8_memcpy` (dont `test_memcpy_basic` de `0x5000` vers `0x5f00`, gardé par l'étape 4).
- **`test_hud.c`** (créé par l'étape 4) lit la palette de dessin par `P8_ADDR_DRAW_PALETTE + c` (H-4-7).

Aucune mention d'adresse dans `tools/`, le `README`, ni `docs/` hors plans et PRD ; `docs/plan-10.md` l. 29 parle des « blocs copiés à `0x5f00` par `hud.c` » (plan futur : à reformuler en « copiés dans la palette de dessin » au chunk 1). `docs/trous-de-portage.md` : aucun GAP ne disparaît ; GAP-18 (remap de `pal()`) parle de « l'octet de palette » et du bit `0x80`, toujours vrais.

**Aucun recouvrement** : les régions vivantes sont disjointes (la feuille de sprites déballée à `0x6000` ne partage rien avec la carte à `0x1000`), donc aucun code ne dépend d'un alias entre deux régions — la condition pour les séparer en champs sans changer un pixel.

### Décisions

- **Forme des champs** : quatre tableaux `unsigned char`, `[y][x]` pour les trois surfaces. Le 2D plutôt que l'aplati parce qu'un indice de colonne hors borne devient un dépassement que `-fsanitize=bounds` (UBSan, `make sanitize-net`) signale, au lieu de glisser sur la ligne suivante. Les tailles viennent des constantes existantes (`P8_SCREEN_WIDTH/HEIGHT`, `P8_SPRITE_SHEET_WIDTH/HEIGHT`, `P8_MAP_WIDTH/HEIGHT`) ; ces constantes vivent dans `gfx.h`, le header du type devra donc les obtenir (les déplacer dans le header du type, ou l'inverse : `gfx.h` inclut déjà `mem.h`). Pas de structure par région ni d'accesseur : un champ suffit, la seule logique (bornes) est déjà dans les primitives.
- **Encodage de la palette de dessin** : inchangé, un octet par couleur source avec le bit `0x80`. Le séparer en remap + drapeaux réintroduirait le double stockage dont `gfx.h` garde le souvenir ; et le remap n'est pas appliqué (GAP-18).
- **Carte réduite à 32×32** : `P8_MAP_WIDTH` = `P8_MAP_HEIGHT` = 32 ; `map_cell_addr` et `P8_MAP_BASE_ROWS` disparaissent ; `p8_mset` hors de 32×32 ne fait rien, la lecture de `p8_map` hors de 32×32 rend 0 (cellule vide, non dessinée) — le même contrat qu'aujourd'hui hors de 128×64. Preuve que rien ne change : le seul `p8_map` de `src/` est `background_draw()` sur `(0..31, 0..31)`, le seul `p8_mset` est `background_construct()` sur `(0..31, 0..31)` (H-4-6). Le nom du champ reste celui du runtime (`map_cells`), pas « fond » : `p8/` ne connaît pas le jeu.
- **Bornes du chargeur de sprites** : `gfx_pixel_sink` écrit `sprite_sheet[row][col]` seulement si `row < 128 && col < 128`, sinon ignore la valeur. Le contrat « valeurs hors région ignorées » de `assets.h` devient vrai par région, plus seulement au bout des 56 Ko. `pico8/gfx.json` a exactement 128 de large et 16 384 pixels : aucun pixel chargé ne change.
- **`hud_select_palette_block()`** : `memcpy(mem->draw_palette, <bloc de la table C>, 16)` ; signature et `assert` de plage de l'étape 4 inchangés.
- **`p8_cls`** : `memset` du champ écran. **`dump_screen`** : passe `&mem->screen[0][0]` à `dump_pixels()` (qui prend déjà un `const unsigned char *` de 128×128) au lieu de recopier dans un tampon local. **`screencast_write_frame`** et **`raylib_backend_present`** : lisent le champ ; leurs `assert` de nibble restent.
- **Intermédiaire, chunks 1 à 4** : chaque chunk ajoute son champ à `p8_memory_t` à côté de `data[P8_MEM_SIZE]`, qui reste intact (et inutilisé pour la région déplacée) jusqu'au chunk 5. La structure grossit donc temporairement jusqu'à ~89 Ko ; elle est sur la pile de `replay.c`/`game.c` et des TU, sans conséquence (pas d'allocation : `alloc-guard` ne bouge pas). Le chunk qui déplace une région supprime sa constante `P8_ADDR_*` et le test de frontière correspondant de `test_mem.c`.
- **Nom final** : au chunk 5, `p8_memory_t` → `p8_vram_t`, `p8_mem_init` → `p8_vram_init`, `src/p8/mem.{c,h}` → `src/p8/vram.{c,h}`, `tests/test_mem.c` → `tests/test_vram.c`, par remplacement mécanique (includes, `Makefile`, cible `test_mem` → `test_vram` dans `UNIT_TESTS`). Les noms de variables et de paramètres (`mem`, `memory`) et `dispatch_context_set_mem()`/`dispatch_context_mem()` gardent leur nom : 77 occurrences dans 64 fichiers et des centaines de corps de fonction pour un nom qui reste juste (« la mémoire où le jeu dessine ») — voir Out of Scope.
- **Commentaires** : ceux de l'inventaire qui nomment une adresse ou l'API mémoire sont corrigés dans le chunk de leur région ; ceux qui décrivent le cart (`POKE(0x5F36,0x8)` dans `p8_map`, la palette de mise sous tension de `palt()`) restent. Règle du projet : ne rester que s'ils portent un écart, un piège ou un contrat C.

## Implementation Chunks

Un commit par chunk, chacun avec `make test-all` et `make sanitize-net` verts, les 26 `*.sha256` inchangés, et — pour les chunks qui touchent `game.c` ou `raylib_backend.*` (4 et 5) — `make raylib` qui compile et lie. Ordre : de la région la plus petite et la plus confinée à la plus large, puis la suppression du bloc. Les chunks 1 à 4 sont indépendants entre eux (chaque région a ses propres lecteurs) ; l'ordre ne sert qu'à faire passer les petits diffs d'abord.

1. **Palette de dessin (`0x5f00`, 16 o).** Champ `draw_palette[16]` ; `p8_mem_init` l'initialise (identité, couleur 0 transparente) ; `p8_palt`, `p8_gfx_is_transparent`, `hud_select_palette_block` le lisent ou l'écrivent. `P8_ADDR_DRAW_PALETTE` disparaît (`P8_DRAW_PALETTE_TRANSPARENT_BIT` reste). Tests : `test_gfx.c` l. 84–126 lisent le champ ; `test_draw_palette_write_overrides_a_previous_palt` écrit 16 octets dans le champ au lieu de passer par `0x5000` + `p8_memcpy` ; `test_hud.c` lit le champ (H-4-7) ; `test_mem.c` : le test d'init vérifie le champ, et `is_default_draw_palette_addr` disparaît (plus rien de non nul dans `data`). Commentaires : `mem.h`, `gfx.h` (« at 0x5f00 »), `hud.h`, `docs/plan-10.md` l. 29.
   Fichiers : `src/p8/mem.{c,h}`, `src/p8/gfx.{c,h}`, `src/game/hud.{c,h}`, `tests/test_gfx.c`, `tests/test_mem.c`, `tests/test_hud.c`, `docs/plan-10.md`.
2. **Feuille de sprites (`0x6000`, 16 Ko).** Champ `sprite_sheet[128][128]` ; `p8_sget` le lit ; `gfx_pixel_sink` l'écrit avec les bornes de région. `P8_ADDR_SPRITE_SHEET` disparaît. Tests : `test_asset_loading.c` (trois lectures, par `p8_sget` ou le champ) + nouveau test de borne du chargeur ; `test_gfx.c` (écritures de sprites l. 350–503) ; `test_mem.c` perd `test_sprite_sheet_region_boundary`. Commentaire : `assets.h`.
   Fichiers : `src/p8/mem.h`, `src/p8/gfx.c`, `src/p8/assets.{c,h}`, `tests/test_asset_loading.c`, `tests/test_gfx.c`, `tests/test_mem.c`.
3. **Tuiles de la carte (`0x1000`/`0x2000` → 32×32).** Champ `map_cells[32][32]` ; `P8_MAP_WIDTH`/`HEIGHT` à 32 ; `map_cell_addr`, `P8_MAP_BASE_ROWS`, `P8_ADDR_SPRITE_MAP_SHARED`, `P8_ADDR_MAP` disparaissent. Tests : `test_gfx.c` — `test_mset_writes_base_region_cell` lit le champ, `test_mset_writes_shared_upper_region_cell` devient « hors de 32×32, `p8_mset` ne change rien » (lignes 32 et 63, colonne 32), `test_mset_out_of_range_is_noop` balaie le champ, `test_map_and_mset_round_trip_against_real_sheet` passe de la ligne 40 à une ligne < 32 ; `test_background.c` `mget` lit le champ (ses `MAP_HASH`… ne bougent pas) ; `test_screencast.c` pose son octet hors écran dans `map_cells` ; `test_mem.c` perd `test_shared_region_boundary` et `test_map_region_boundary`. Commentaires : `gfx.c` (`P8_MAP_BASE_ROWS`).
   Fichiers : `src/p8/mem.h`, `src/p8/gfx.{c,h}`, `tests/test_gfx.c`, `tests/test_background.c`, `tests/test_screencast.c`, `tests/test_mem.c` (+ `src/p8/assets.c` si `map_cell_sink` existe encore, H-4-3).
4. **Écran (`0xA000`, 16 Ko).** Champ `screen[128][128]` ; `p8_cls`, `p8_pset`, `dump_screen`, `screencast_write_frame`, `raylib_backend_present` le lisent ou l'écrivent. `P8_ADDR_SCREEN` disparaît. Tests : les 21 boucles de hachage deviennent une double boucle `y` puis `x` sur le champ (même ordre ligne par ligne, donc mêmes constantes) ; les lectures et écritures de pixel (`test_background_draw.c`, `test_demo.c`, `test_layout_nil_item_draws_nothing.c`, `test_draw_layers_camera.c` — son `assert` sur l'adresse devient un `assert` sur les coordonnées —, `test_help_screen.c`, `test_dump.c`, `test_headless_capture.c`, `test_screencast.c`, `test_gfx.c`) ; `test_mem.c` perd `test_screen_region_boundary`. Commentaires : `raylib_backend.h`, `screencast.h`. Vérifier `make raylib`.
   Fichiers : `src/p8/mem.h`, `src/p8/gfx.c`, `src/devtools/dump.c`, `src/devtools/screencast.{c,h}`, `src/platform/raylib_backend.{c,h}`, les 30 fichiers de tests qui citent `P8_ADDR_SCREEN`.
5. **Fin du bloc : `p8_vram_t`.** `data[P8_MEM_SIZE]`, `P8_MEM_SIZE`, `addr_in_range`, `p8_peek`, `p8_poke`, `p8_memcpy` disparaissent ; `test_mem.c` perd ses tests de peek/poke/memcpy (dont `test_memcpy_basic`, dernier `0x5000` du dépôt) et devient `test_vram.c` (test d'init sur les quatre champs : tout à zéro sauf `draw_palette`) ; renommage `p8_memory_t` → `p8_vram_t`, `p8_mem_init` → `p8_vram_init`, `p8/mem.{c,h}` → `p8/vram.{c,h}`, `Makefile` (14 mentions de `src/p8/mem.c`, cible et entrée `UNIT_TESTS`). Vérifier `make raylib`.
   Fichiers : `src/p8/mem.{c,h}` → `src/p8/vram.{c,h}`, tous les fichiers qui incluent `p8/mem.h` ou nomment le type (80 + `Makefile`), `tests/test_mem.c` → `tests/test_vram.c`.

## Testing Decisions

- **Ce qu'est un bon test ici** : il observe ce que voit le reste du jeu — un pixel de l'écran, une cellule posée par `p8_mset` puis dessinée par `p8_map`, la transparence d'une couleur — par le champ ou par une primitive, jamais par une adresse. Les hachages d'écran existants sont la preuve principale : leur valeur ne doit pas bouger.
- **Aucun hachage refigé.** Les 21 hachages d'écran des TU, les `MAP_*` de `test_background.c` et les 26 `*.sha256` restent identiques à chaque chunk. Un hachage qui bouge est un bogue du chunk (ordre de parcours, borne, région oubliée), jamais une valeur à refiger.
- **Nouveaux tests** (prior art : `tests/test_gfx.c` pour les primitives, `tests/test_asset_loading.c` pour le chargeur, `tests/test_mem.c` pour l'init) :
  - chunk 2 : un `gfx.json` de test (fixture dans `tests/fixtures/`, à côté de `screencast_capture_events.json`) de plus de 128×128 valeurs se charge, la feuille vaut ses 16 384 premières valeurs et l'écran reste à zéro ;
  - chunk 3 : `p8_mset` en `(32, 0)`, `(0, 32)`, `(0, 63)` ne change aucune cellule ; `p8_map` sur une fenêtre qui déborde de 32×32 ne dessine rien hors des cellules posées ;
  - chunk 5 : `test_vram.c`, init des quatre champs.
- **Tests supprimés** : les tests de `test_mem.c` qui ne testaient que l'adressage (frontières de région, aller-retour à `0x1234`, hors plage, `p8_memcpy`) partent avec l'API, chacun au chunk de sa région ; leurs garanties passent au type (taille des champs) et aux tests de bornes des primitives (`p8_pset` hors écran, `p8_sget` hors feuille, `p8_mset` hors carte, déjà présents dans `test_gfx.c`).
- **Non-régression des pixels** : `make e2e` → 26 « matches » à chaque chunk, aucun `make e2e-baseline`. `make sanitize-net` → « sanitize: 26 clean » et `test_live_slots` vert ; c'est lui qui donne son intérêt au choix 2D.
- **Durées** : `make test` ~4,5 s et `make e2e` ~0,25 s (mesures de référence d'avant l'étape 1). L'étape remplace des appels `p8_peek`/`p8_poke` avec test de plage par des accès directs et n'ajoute que trois petits tests : rien ne devrait ralentir. Mesurer au commit des chunks 4 et 5 (les plus larges), à chaud, trois passes alternées HEAD / chunk.
- **`make raylib`** (vendoré dans `third_party/raylib/`) : compiler et lier aux chunks 4 et 5, sans lancer le binaire (règle du projet).
- **Critère de fin** (plan, règle 3) : `git grep -nE "p8_memory_t|p8_mem_init|p8_peek|p8_poke|p8_memcpy|P8_ADDR_|P8_MEM_SIZE|0x5000|0x5f00|p8/mem\.h" -- src tests Makefile` ne rend rien.

## Out of Scope

- Renommer les variables et paramètres `mem`/`memory` et `dispatch_context_set_mem()`/`dispatch_context_mem()` : le nom reste juste, le diff toucherait tous les corps de fonction de dessin pour rien d'observable ; à reprendre si le plan 10 (dessin sur GPU) refait ces signatures.
- Fusionner `p8_vram_t` avec `p8_graphics_t` (caméra, motif de remplissage, couleur de stylo) : changerait la signature de toute l'API de dessin, qui reste par décision du plan.
- Appliquer le remap de `pal()` (GAP-18) ou séparer la palette de dessin en deux tableaux : changerait des pixels, ou réintroduirait un double stockage.
- Le format de la feuille de sprites sur PSX (4 bits par pixel, VRAM) : plan 10.
- La table des blocs de palette, `pico8/map.json`, `p8_load_map_json` : étape 4.
- La virgule fixe, la table `sin`/`cos`, `constants-check` : restent par décision du plan.
- L'audio : hors périmètre du projet.

## Further Notes

- Les points « vérifier avec l'utilisateur » de `to-prd` (modules, tests à écrire, découpage) n'ont pas été posés : la skill `guideline` a été appliquée à leur place. Ce qu'elle a tranché :
  - **simplicité** : quatre tableaux nus, pas de structure par région ni d'accesseur ; les tests lisent le champ ;
  - **chirurgie** : les signatures des primitives et les noms de variables ne bougent pas ; seuls partent ce que l'étape rend orphelin (`p8_peek`/`p8_poke`/`p8_memcpy`, `P8_ADDR_*`, `map_cell_addr`, `P8_MAP_BASE_ROWS`, les tests d'adressage) ; le renommage du type est le seul changement mécanique large, exigé par le plan (« `p8_memory_t` disparaît »), et il est isolé au dernier chunk ;
  - **hypothèses en surface** : réduire la carte à 32×32 et borner le chargeur sont deux changements de contrat sans effet sur un pixel, prouvés par l'inventaire des appelants (un seul `p8_mset`, un seul `p8_map`, un `gfx.json` de 16 384 valeurs) ; ils sont dits ici plutôt que glissés dans un chunk ;
  - **critère vérifiable** : sha256 identiques, hachages de TU identiques, `make sanitize-net` vert, `make raylib` qui lie, `git grep` de fin vide.
- Choix du nom `p8_vram_t` : le préfixe `p8_` reste (règle du projet pour le runtime) ; « vram » dit ce que contiennent les quatre champs (ce que les primitives de dessin lisent et écrivent) sans évoquer la RAM adressable de PICO-8. À trancher autrement au triage si un autre nom est préféré ; le chunk 5 est le seul concerné.
- Ce PRD a été écrit contre HEAD `67b39c5` lu par `git show`/`git grep HEAD`, sans rien lire de non commité.

## Hypothèses sur l'issue de l'étape 4

L'étape 4 (`docs/prd/triage/phase6-etape4-donnees-hors-assets.md`) est implémentée en parallèle ; HEAD `67b39c5` ne la contient pas. L'étape 5 suppose qu'elle est commitée, telle que son PRD la décrit :

- **H-4-1** — Plus rien dans `src/` ne lit ni n'écrit `0x5000..0x50ff` : `hud_build_palette_table()` a disparu (prototype, appel de `cart_init()`, appels de `test_layout_nil_item_draws_nothing.c` et `test_tooltip_draw_at.c`, commentaire l. 167 de ce dernier). Les seules mentions de `0x5000` restantes sont les écritures manuelles de `tests/test_gfx.c` (`test_draw_palette_write_overrides_a_previous_palt`) et `tests/test_mem.c` (`test_memcpy_basic`), que l'étape 4 garde et que l'étape 5 supprime (chunks 1 et 5).
- **H-4-2** — `hud_select_palette_block(p8_memory_t *, p8_fixed_t)` garde sa signature, lit un bloc d'une table `static const unsigned char [16][16]` privée à `hud.c`, vérifie `flr(no)` ∈ 0..15 par un `assert`, et écrit les 16 octets de `P8_ADDR_DRAW_PALETTE` par `p8_poke` (ou une boucle équivalente) — plus par `p8_memcpy` depuis `0x5000`. `hud_select_palette_block_default()` est inchangée.
- **H-4-3** — `p8_load_map_json()`, `map_cell_sink()`, le `P8_MAP_BASE_ROWS` local de `assets.c` et `pico8/map.json` ont disparu ; `assets.c` ne charge plus que `gfx.json` ; le commentaire de `P8_MAP_BASE_ROWS` dans `gfx.c` ne cite plus `map_cell_sink`.
- **H-4-4** — Aucun binaire ni TU ne charge plus de carte : 0 fichier appelle `p8_load_map_json` (43 TU et deux binaires à HEAD). `test_asset_loading.c` ne garde que ses trois tests de feuille de sprites (lectures `P8_ADDR_SPRITE_SHEET` l. 14, 24, 25 à HEAD) ; ses lectures de `P8_ADDR_SPRITE_MAP_SHARED`/`P8_ADDR_MAP` sont parties.
- **H-4-5** — `test_demo.c` ne lit plus `P8_ADDR_SPRITE_MAP_SHARED` ; il pose lui-même (1,0)=81, (2,0)=85, (3,0)=1 par `p8_mset` avant son `p8_map(…, 0, 0, 0, 100, 4, 1)` — des cellules dans 32×32, donc valides après le chunk 3.
- **H-4-6** — L'étape 4 laisse `p8_mset`, `p8_map`, `map_cell_addr`, `P8_MAP_WIDTH` = 128, `P8_MAP_HEIGHT` = 64, `P8_ADDR_SPRITE_MAP_SHARED`, `P8_ADDR_MAP` et la séparation lignes 0–31 / 32–63 tels quels (son Out of Scope le dit) ; `background_construct()` écrit toujours `(0..31, 0..31)` par `p8_mset` avant `entity_construct()`, et `background_draw()` reste le seul `p8_map` de `src/`, sur 32×32.
- **H-4-7** — Le nouveau `tests/test_hud.c` existe et lit la palette de dessin par `p8_peek(…, P8_ADDR_DRAW_PALETTE + c)` (et la feuille par `p8_sget`) : il entre dans le chunk 1 (lecture du champ) ; `p8_sget` ne change pas de signature au chunk 2.
- **H-4-8** — L'étape 4 laisse les 26 `*.sha256` inchangés et les durées de référence (`make test` ~4,5 s, `make e2e` ~0,25 s) valables ; ses seuls hachages refigés sont `CONSTRUCTIONS_HASH`, `PLANET_RESOURCE_ROW_HASH`, `CALLS_HASH` de `test_layout_row.c`, que l'étape 5 ne touche pas (seule sa boucle de hachage change de forme au chunk 4).
- **H-4-9** — Le commentaire de `p8_spr` dans `gfx.c` ne parle plus de `hud_build_palette_table()`, et celui de `p8_map` décrit le sprite 0 comme « les pixels des blocs de palette » ; ceux de `test_gfx.c` l. 107–111 parlent de « l'écriture de la palette de dessin », plus de `memcpy` dans `0x5f00`.
- **H-4-10** — L'étape 4 n'ajoute aucun appelant de `p8_peek`/`p8_poke`/`p8_memcpy` ni aucune constante `P8_ADDR_*` hors de ceux que H-4-2, H-4-7 listent.

## À revérifier une fois l'étape 4 faite

- **Problem Statement → tableau des régions** : ligne « table des blocs de palette » morte, colonne « palette de dessin » de `hud_select_palette_block` (H-4-1, H-4-2) — `git grep -n "0x5000\|hud_build_palette_table\|hud_select_palette_block" -- src tests`.
- **Implementation Decisions → Inventaire `src/`** : lignes de `hud.c` et de `assets.c` (H-4-2, H-4-3), décomptes de `p8_memory_t`/`p8_mem_init`/`p8/mem.h` (l'étape 4 ajoute `test_hud.c`, H-4-7) — refaire `git grep -c "p8_memory_t\|p8_mem_init\|p8/mem.h" -- src tests`.
- **Implementation Decisions → Inventaire `tests/`** : `test_asset_loading.c`, `test_demo.c`, `test_hud.c`, et les numéros de ligne de `test_gfx.c`/`test_mem.c` (H-4-4, H-4-5, H-4-7, H-4-9) — refaire `git grep -n "p8_peek\|p8_poke\|p8_memcpy\|P8_ADDR_\|0x5000\|0x5f00" -- tests`.
- **Implementation Decisions → Carte réduite à 32×32** : la preuve « un seul `p8_mset`, un seul `p8_map` » (H-4-6) — `git grep -n "p8_mset\|p8_map(" -- src`.
- **Implementation Chunks → chunk 1** : la forme exacte de `hud_select_palette_block` à remplacer (H-4-2) et la présence de `test_hud.c` dans sa liste de fichiers (H-4-7).
- **Implementation Chunks → chunk 3** : `src/p8/assets.c` sort de la liste si `map_cell_sink` a bien disparu (H-4-3).
- **Implementation Chunks → chunk 5** : le décompte des fichiers du renommage et des mentions de `src/p8/mem.c` dans le `Makefile` (cible `test_hud` ajoutée par l'étape 4).
- **Testing Decisions → hachages et durées** : valeurs de référence toujours valables (H-4-8).
- **Testing Decisions → critère de fin** : le `git grep` ne doit plus trouver que ce que l'étape 5 retire (H-4-10).
