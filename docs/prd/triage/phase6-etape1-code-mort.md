---
title: "Plan 6, étape 1 — Retirer le code mort hérité de PICO-8"
description: "Le cœur porte encore de l'audio inerte, des écritures mémoire que personne ne lit (GPIO 0x5f80, musique 0x3101, drapeau souris 0x5f2d) et une deuxième manette câblée à « rien » : à retirer sans changer un pixel."
status: needs-triage
---

## Problem Statement

Le port C garde des morceaux qui n'existent que parce que le cart PICO-8 les avait, et qui ne font rien sur PC — ni ne feront rien sur PSX :

- **L'audio** : `p8_sfx` et `p8_music` sont des fonctions vides (GAP-10), mais toute une chaîne les alimente — drapeau de sourdine, `p8_play_sfx`, numéros de son portés par les données, lecture de `stat(19)` et `stat(20)` (que `p8_stat` ne connaît pas et rend toujours 0).
- **Le GPIO `0x5f80`** : `ui_update_hover` écrit à chaque frame le `gpio_signal` de l'entité survolée dans la mémoire émulée ; seul le cart lu par un hôte web le lisait, aucun code du port ne le relit.
- **Les octets de musique `0x3101`** : `score_panel_refresh_palette` pose un bit 7 sur des motifs de musique ; sans musique, personne ne les lit.
- **Le drapeau souris `0x5f2d`** : `mouse_construct` active le « devkit mouse » de PICO-8 ; le backend raylib lit la souris sans regarder ce drapeau.
- **La deuxième manette (GAP-2)** : `camera_update` teste un `btn_p2` toujours faux.

Pour celui qui maintient le port et prépare la cible PSX (plans 7 à 11), ce code coûte : il faut le lire, le compiler, le tester (des hachages de TU replient l'octet `0x5f80`), le documenter dans `docs/trous-de-portage.md`, et le porter vers une mémoire typée à l'étape 5 alors qu'il n'a aucun effet.

## Solution

Supprimer ces cinq familles de code mort, chacune entièrement, avec tout ce que leur retrait rend orphelin (champs de données, tables, en-têtes, entrées de `Makefile`, TU, sections de `docs/trous-de-portage.md`). Le jeu reste rigoureusement le même à l'écran : les 26 `*.sha256` de `tests/e2e/` ne bougent pas. Les lignes « toggle music » et « toggle sfx » du menu pause restent visibles et cliquables, mais inertes (décision du plan 6).

Critère de fin (règle du plan) : aucun des symboles et adresses retirés n'apparaît plus dans `src/` ni dans `tests/`.

## User Stories

1. As a mainteneur du port, I want `src/p8/audio.c` et `src/p8/audio.h` supprimés, so that le runtime émulé ne contient plus de sous-système qui ne produit rien.
2. As a mainteneur du port, I want plus aucun appel à `p8_sfx`, `p8_music` ni `p8_play_sfx` dans le jeu, so that lire une méthode de jeu ne m'oblige plus à vérifier qu'un appel est inerte.
3. As a mainteneur du port, I want le drapeau de sourdine (`p8_sfx_muted`, `p8_set_sfx_muted`) supprimé, so that aucun état global ne survit pour un son qui n'existe pas.
4. As a joueur, I want les lignes « toggle music » et « toggle sfx » du menu pause toujours affichées avec leur couleur de bouton cliquable, so that le menu pause reste identique pixel pour pixel.
5. As a joueur, I want cliquer « toggle music » ou « toggle sfx » toujours fermer le menu comme avant, so that le comportement visible du menu pause ne change pas.
6. As a mainteneur du port, I want la lecture de `stat(20)` dans le gestionnaire « toggle music » retirée, so that `p8_stat` n'est plus appelé pour un numéro qu'il ne connaît pas.
7. As a mainteneur du port, I want la branche `stat(19) == 50` du refus de slipway retirée avec la garde `input != NULL` qui n'existait que pour elle, so that le refus d'une slipway se lit en trois lignes.
8. As a mainteneur du port, I want les numéros de son des données (`play_sfx`/`has_play_sfx` des projets de recherche et de `map_tables`, `open_sfx`/`has_open_sfx` du cadre de message box et de l'archétype d'aide) supprimés, so that les tables de données ne portent plus de colonnes sans lecteur.
9. As a mainteneur du port, I want l'écriture de `0x5f80` dans `ui_update_hover` supprimée, so that le survol ne touche plus la mémoire émulée.
10. As a mainteneur du port, I want le champ `gpio_signal` retiré du noyau `entity_t`, des boutons, des clés de bouton, des lignes de menu et de l'archétype d'aide, so that le noyau commun ne porte plus de pointeur dont personne ne lit la cible.
11. As a mainteneur du port, I want `score_panel_refresh_palette` et la table `palette_offsets_table` (avec `PALETTE_OFFSETS_COUNT`) supprimées, so that le panneau de score n'écrit plus dans la zone musique.
12. As a mainteneur du port, I want `mouse_construct` et son `poke(0x5f2d, 1)` supprimés, so that construire la souris ne touche plus la mémoire émulée.
13. As a mainteneur du port, I want le commentaire du backend raylib qui justifie `HideCursor()` par `0x5f2d` retiré, sans changer l'appel lui-même, so that le code ne renvoie plus à une adresse disparue.
14. As a mainteneur du port, I want `btn_p2` retiré de `camera_update`, so that le défilement au d-pad ne teste plus qu'un seul joueur, celui qui existe.
15. As a mainteneur du port, I want les marqueurs `/* GAP-2 */` et `/* GAP-10 */` retirés des sources et les sections GAP-2 et GAP-10 retirées de `docs/trous-de-portage.md`, so that le catalogue des écarts ne décrit que des écarts vivants.
16. As a mainteneur du port, I want `src/p8/audio.c` retiré de toutes les listes de sources du `Makefile` (TU, e2e, binaire raylib), so that `make test`, `make e2e` et `make raylib` compilent sans lui.
17. As a mainteneur du port, I want les 26 rejeux de `tests/e2e/` rendre exactement les mêmes `*.sha256`, so that je sais qu'aucun pixel n'a bougé.
18. As a mainteneur du port, I want `make test-all` et `make sanitize-net` passer à chaque commit de l'étape, so that la base reste saine pendant le retrait.
19. As a mainteneur du port, I want `make test` et `make e2e` ne pas ralentir par rapport à leurs durées avant l'étape, so that la boucle rapide reste rapide.
20. As a mainteneur du port, I want les TU qui vérifiaient le code retiré supprimées (pas désactivées), so that la suite ne garde aucune assertion sur un comportement disparu.
21. As a mainteneur du port, I want les TU qui repliaient seulement l'octet `0x5f80`, `gpio_signal`, `play_sfx` ou `open_sfx` dans un hachage garder le reste de leur repli, avec leur constante recalculée, so that leur couverture sur le reste de l'état ne baisse pas.
22. As a mainteneur du port, I want qu'un `grep` des symboles retirés sur `src/` et `tests/` ne rende rien, so that la fin de l'étape est vérifiable mécaniquement.
23. As a agent qui portera la mémoire typée (étape 5), I want qu'il ne reste aucune écriture à `0x5f80`, `0x5f2d` ni dans la zone musique, so that je n'aie pas à typer des régions sans lecteur.
24. As a agent qui portera vers PSX (plans 7 à 11), I want qu'aucun module ne dépende d'un sous-système audio, so that le backend PSX n'ait pas à fournir de bouchon.

## Implementation Decisions

Ce PRD nomme exactement les fichiers et symboles touchés : c'est l'objet même de l'étape (un retrait), et le plan exige que ce qui part n'apparaisse plus nulle part.

### Règle générale : retirer jusqu'aux orphelins, pas au-delà

- On retire le code nommé par l'étape **et ce que ce retrait rend sans lecteur** (champs, tables, fonctions, includes, constantes de test). On ne touche à rien d'autre, même mort.
- On ne retire **pas** `p8_stat` ni les `stat(32/33/34)` (étape 2), ni les états/tags en chaîne (étape 3), ni `p8_memory_t`/`peek`/`poke` (étape 5).
- On garde les lignes « toggle music »/« toggle sfx » du menu pause, leurs textes et leurs icônes (`{.number = 42}`, `{.number = 40}` dans `game_data.c`), par décision du plan.

### 1. Audio

- Supprimer `src/p8/audio.c` et `src/p8/audio.h` : `p8_sfx`, `p8_music`, `p8_sfx_muted`, `p8_set_sfx_muted`, `p8_play_sfx`, le global `g_sfx_muted`.
- Retirer chaque `#include "p8/audio.h"` et chaque appel :
  - `p8_play_sfx` dans `game/probe.c` (×2), `game/slipway.c` (son de construction et branche `stat(19)`), `game/wormhole.c`, `game/ui_button.c` (clic), `game/message_box.c` (`message_box_open_box`, et la variable `play_sfx_arg`), `game/planet_display.c` (son de projet ou défaut 55).
  - `p8_music(0)` dans `game/cart.c`.
- **Menu pause** (`game/menu.c`) : `pause_menu_row2_on_click_method` et `pause_menu_row3_on_click_method` deviennent des fonctions vides (`(void)self;`). **Elles ne deviennent pas `NULL`** : `ui_button.c` dessine un bouton sans `on_click` en couleur 5 au lieu de 12/13 et ne le traite pas au clic — passer à `NULL` changerait des pixels de `pause_menu_rows`. La lecture `p8_stat(..., 20)` disparaît. Le commentaire de `pause_menu_row4_on_click_method` sur l'état de sourdine disparaît.
- **Refus de slipway** (`game/slipway.c`) : la branche `p8_stat(input, 19) == 50` disparaît avec le `dispatch_context_input()` et le commentaire qui la gardaient ; il reste `self->is_dead = 1; return;`. Include de `dispatch_context.h` retiré seulement s'il devient orphelin.
- **Données de son devenues orphelines** :
  - `research.h`/`research.c` : `play_sfx`, `has_play_sfx` de l'instance de projet ; mention dans le commentaire de contrat de `research.h`.
  - `map_tables.h`/`map_tables.c` : colonnes `has_play_sfx`, `play_sfx` (lignes `collapse`, `harvest`) et la mention dans le commentaire de `map_tables.h`.
  - `message_box.h` : `has_open_sfx`, `open_sfx` de `open_box_frame_t` ; `help.c` cesse de les remplir.
  - `game_data.h`/`game_data.c` : `open_sfx` de l'archétype d'aide.
- `Makefile` : `src/p8/audio.c` retiré des deux listes de sources et de la règle `$(TMP_DIR)/game` (prérequis et commande).
- `docs/trous-de-portage.md` : section GAP-10 supprimée ; marqueurs `/* GAP-10 */` retirés de `menu.c`, `cart.c` (et d'`audio.c`, supprimé).

### 2. GPIO `0x5f80`

- `game/ui.c` : la fin de `ui_update_hover` (calcul de `poke_val`, `p8_peek`/`p8_poke` à `0x5f80`) disparaît. Si `ui_to_int` ou le paramètre `memory` deviennent orphelins dans cette fonction, ils partent aussi (sans changer la signature publique si d'autres appelants la partagent — à vérifier au moment du retrait).
- `gpio_signal` devenu orphelin : retiré de `entity_class.h` (pointeur du noyau `entity_t`), `ui_button.h`/`ui_button.c` (champ du bouton, clé `button_keys_t`, `has_gpio_signal`, branchement du pointeur d'en-tête), `game_data.h`/`game_data.c` (`has_gpio_signal`/`gpio_signal` des lignes de menu, `gpio_signal = 0x50000` de l'archétype d'aide), `menu.c` (copie dans `bs[i]`), `help.c` (branchement du pointeur d'en-tête).

### 3. Octets de musique `0x3101`

- `game/score_panel.c`/`.h` : `score_panel_refresh_palette` supprimée, ainsi que son appel en fin de `score_panel_refresh`.
- `game/game_data.c`/`.h` : `palette_offsets_table` et `PALETTE_OFFSETS_COUNT`, sans autre lecteur, supprimés.

### 4. Drapeau souris `0x5f2d`

- `game/mouse.c` : `mouse_construct` et son appel supprimés.
- `platform/raylib_backend.c` : le commentaire au-dessus de `HideCursor()` qui cite `0x5f2d` est retiré ; `HideCursor()` reste.

### 5. Deuxième manette (GAP-2)

- `game/scroll.c` : `btn_p2` et son marqueur supprimés ; la condition devient `if (p8_btn(input, b))` (ou la locale `btn_p1` renommée si elle reste seule).
- `p8/input.h`, `platform/raylib_backend.c` : marqueurs `/* GAP-2 */` retirés. Le format de `p8_input_frame_t` et d'`events.json` ne change pas.
- `docs/trous-de-portage.md` : section GAP-2 supprimée.

### Invariants (règles du plan 6)

- Les 26 `tests/e2e/*.json.sha256` restent identiques, octet pour octet ; aucun n'est régénéré.
- À chaque commit : `make test-all` et `make sanitize-net` passent.
- Durées de référence mesurées avant l'étape 1 sur la machine de travail (à chaud) : **`make test` ≈ 4,6 s**, **`make e2e` ≈ 0,25 s**. Aucune ne doit augmenter (un retrait de code ne devrait que les maintenir ou les réduire).
- Aucune nouvelle cible, aucun nouveau garde dans `make test` ni `make e2e`.

## Implementation Chunks

Les cinq familles sont indépendantes ; chacune est un cycle et un commit, vérifié par `make test-all` + `make sanitize-net` + les `sha256` inchangés. Ordre proposé, du plus contenu au plus étendu :

1. **Deuxième manette (GAP-2)** — `scroll.c`, marqueurs dans `input.h` et `raylib_backend.c`, section GAP-2 de la doc. Aucune TU touchée.
2. **Drapeau souris `0x5f2d`** — `mouse.c`, commentaire de `raylib_backend.c`, assertion de `test_mouse.c`.
3. **Octets de musique `0x3101`** — `score_panel.c/.h`, `palette_offsets_table` dans `game_data.c/.h`, TU de `test_score_panel.c` et contrôle de `test_map_tables.c`.
4. **GPIO `0x5f80`** — `ui.c`, puis `gpio_signal` dans `entity_class.h`, `ui_button.*`, `game_data.*`, `menu.c`, `help.c` ; hachages de `test_ui.c`, `test_help_screen.c`, `test_background.c`, `test_modal_backdrop.c`, `test_button.c`, `test_entity_header.c`.
5. **Audio** — suppression de `p8/audio.*`, tous les appelants, gestionnaires vides du menu pause, `stat 19/20`, champs `play_sfx`/`open_sfx`, `Makefile`, section GAP-10 ; TU de `test_ui.c`, `test_research_project.c`, `test_help_screen.c`.

## Testing Decisions

- **Un bon test ici, c'est la preuve qu'on n'a rien changé de visible** : un retrait de code mort n'introduit pas de comportement, donc pas de nouvelle TU. La preuve est l'existant : `make e2e` (26 rejeux depuis l'écran titre, `sha256` du framebuffer inchangés) et les TU qui restent. En particulier `pause_menu_rows` couvre les deux lignes audio du menu pause (affichage et clic qui ferme le menu) ; `camera_drag_dpad` couvre le défilement au d-pad.
- **TU supprimées** (elles n'éprouvent que le code retiré) :
  - `test_ui.c` : la partie qui clique « toggle sfx » et vérifie la bascule de `p8_sfx_muted` (et son `#include "p8/audio.h"`) ; si le reste du test n'a plus d'objet, le test entier part.
  - `test_score_panel.c` : `test_refresh_palette_flags_every_palette_offsets_pattern_without_settled`.
  - `test_map_tables.c` : le `check_int_seq(palette_offsets_table, …)`.
  - `test_mouse.c` : le `poke`/l'assertion sur `0x5f2d` dans `test_reset_world_builds_a_mouse_reader` (le reste du test reste) ; son commentaire de tête perd « devkit mouse on ».
  - `test_ui.c`, `test_help_screen.c` : les assertions directes sur `0x5f80` (`assert(p8_peek(&memory, 0x5f80) == 0)`, `check_int("hover 0x5f80", …, 5)`) et les phrases de commentaire qui les décrivent.
  - `test_button.c` (cas `keys.has_gpio_signal = 1`), `test_entity_header.c` (bouton avec `gpio_signal`, champ `.gpio_signal` des liens attendus) : ce qui ne vérifie que `gpio_signal`.
- **TU dont un hachage replie un octet ou un champ retiré** : on retire la ligne de repli (`fnv(h, p8_peek(&memory, 0x5f80))`, `fold_*(…gpio_signal…)`, `fold_*(…play_sfx…)`, `fold_fixed_member(…open_sfx)`), on garde tout le reste du repli, et on recalcule la constante (`HOVER_HASH`, `FRESH_HASH`, constantes de `test_background.c`, `test_modal_backdrop.c`, `test_button.c`, `test_research_project.c`, `test_help_screen.c`). La nouvelle valeur est obtenue en exécutant le test une fois ; le message de commit dit quel repli a été retiré, pour qu'on sache que le changement de constante ne cache pas un changement d'état.
- **Prior art** : les TU à hachage FNV de `tests/test_ui.c`/`test_help_screen.c` (pattern `check("…", h, …_HASH)`) ; les e2e de `tests/e2e/` comparés par `make e2e`.
- **Vérification de fin** : `grep -rnE 'p8_sfx|p8_music|p8_play_sfx|sfx_muted|p8/audio|play_sfx|open_sfx|gpio_signal|0x5f80|0x5f2d|0x3101|palette_offsets|btn_p2|GAP-2\b|GAP-10' src tests` ne rend rien ; `git diff --stat tests/e2e` est vide.

## Out of Scope

- Les lignes « toggle music »/« toggle sfx » du menu pause : elles restent (inertes), avec leur texte et leur icône.
- `p8_stat` et les `stat(32/33/34)` de la souris : étape 2.
- Les chaînes d'état, de tag, de nom (`"placing"`, `"planet"`, `"left"`…) : étape 3.
- La table des palettes de `hud.c` et `pico8/map.json` : étape 4.
- `p8_memory_t`, `p8_peek`/`p8_poke`/`p8_memcpy`, les autres adresses (`0x5f00`, sprites, écran) : étape 5.
- Le format d'`events.json` et de `p8_input_frame_t` (le schéma d'entrée ne gagne ni ne perd de champ).
- `pico8/lua.lua` (archive du cart) : intouché.
- Tout code mort non nommé par l'étape et non rendu orphelin par elle.
- Tout ce qui change un pixel.

## Further Notes

- Les questions que `to-prd` pose à l'humain (modules attendus, modules à tester, découpage) ont été tranchées selon le skill `guideline` : « retirer ce que le changement rend orphelin, pas le code mort préexistant » (d'où les champs `gpio_signal`, `play_sfx`, `open_sfx` et `palette_offsets_table` dans le périmètre, mais rien d'autre) ; pas de nouvelle TU pour un retrait, la preuve étant les e2e inchangés et les TU existantes ; découpage en cinq chunks indépendants, un commit chacun.
- Piège relevé en lisant `ui_button.c` : un bouton dont `on_click` est `NULL` est dessiné en couleur 5 au lieu de 12/13. Les gestionnaires audio du menu pause doivent rester des fonctions vides.
- `docs/trous-de-portage.md` perd GAP-2 et GAP-10 ; les autres renvois GAP ne changent pas de numéro.
