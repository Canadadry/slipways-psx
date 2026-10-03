---
title: "Plan 6, étape 3 — Chaînes → enum"
description: "Le jeu désigne encore ses états, ses tags, ses messages, ses fonctions de dessin et ses boutons de souris par des chaînes héritées du Lua, comparées par strcmp à l'exécution."
status: needs-triage
---

## Problem Statement

Le port garde la manière de nommer du cart Lua : un état d'entité est une chaîne (`"placing"`, `"settled"`) comparée par `strcmp`, un registre se lit par une chaîne (`entities_by_tag("planet")`, `entities_by_message("hit_test")`, `draw_layers("draw")`), une ligne de données nomme sa fonction de dessin (`"draw_row"`), et `ui.c` choisit un bouton de souris et son champ par `"left"`/`"right"` et `"pressed"`/`"released"`. Or chacun de ces ensembles est fermé et connu à la compilation : il n'y a jamais d'autre état, d'autre tag ni d'autre bouton que ceux que le code choisit.

Ce que ça coûte au mainteneur :

- une faute de frappe (`"setled"`) compile et se tait : la comparaison rend faux, et un tag inconnu rend `NULL` comme un tag sans entité ;
- `bootstrap.c` retraduit la chaîne d'état en indice (`entity_state_of`) à chaque entité de chaque frame, et `entities_by_tag` retraduit le tag (`entity_tag_named`), alors que les `enum` `state_t` et `tag_t` existent déjà dans `entity_class.h` ;
- `planet_is_colonized` et ses semblables dépendent du contournement de GAP-3 (chaînes non internées : comparer par contenu, jamais par pointeur) ;
- la PSX (plans 7–11) paierait des `strcmp` et des pointeurs de 4 octets là où un petit entier suffit.

Le plan 6 retire du cœur tout ce qui n'existe que pour PICO-8 ; ces chaînes sont un héritage du Lua.

## Solution

Chaque ensemble fermé de noms devient un `enum` C : les états d'entité, les tags du registre, les messages du registre, les fonctions de dessin des lignes de données, le côté du bouton de souris lu par l'interface. Le compilateur refuse désormais un nom inconnu ; plus aucun `strcmp` ne décide d'un état, d'un tag, d'un message, d'une fonction de dessin ou d'un bouton.

Restent des chaînes, par décision du plan : tout ce qui est **donnée** — clé ou nom d'une table de données (types de planète, ressources, technologies, projets, sortes de planète), identifiant fabriqué à l'exécution (`planet_id`), texte affiché, texte écrit par les outils (`state.txt`, arguments de ligne de commande). Le critère est en Implementation Decisions.

Rien ne change pour le joueur ni pour les rejeux : mêmes 26 `*.sha256`, même format `events.json`, même texte de `make state`, mêmes durées de `make test` et `make e2e`.

## User Stories

1. En tant que mainteneur, je veux qu'un état d'entité soit une valeur d'`enum`, pour qu'une faute de frappe sur un nom d'état ne compile pas.
2. En tant que mainteneur, je veux lire `self->header.state == ENTITY_STATE_SETTLED` plutôt qu'un `strcmp` gardé par un test de `NULL`, pour comprendre la condition d'un coup d'œil.
3. En tant que mainteneur, je veux que l'absence d'état soit une valeur nommée (`ENTITY_STATE_NONE`) et l'état zéro d'un en-tête neuf, pour qu'un en-tête initialisé à zéro reste « sans état » comme avec `NULL` aujourd'hui.
4. En tant que mainteneur, je veux que `"unknown"` et `"known"` (états de planète sans méthode) soient des valeurs de l'`enum` comme les autres, pour qu'il n'y ait plus d'états hors de `state_t`.
5. En tant que mainteneur, je veux que `update_entities` indexe `states[]` directement par l'état, pour ne plus retraduire une chaîne à chaque entité de chaque frame.
6. En tant que mainteneur, je veux que les archétypes, les `kind_record`, `confirmed_state` et la table des types de planète portent un état typé, pour qu'aucune donnée d'état ne soit une chaîne.
7. En tant que mainteneur, je veux que `targeting_tooltip_show` et `planet_grab_arena_zone_released` reçoivent un état typé, pour qu'aucune fonction ne prenne un nom d'état en `const char *`.
8. En tant que mainteneur, je veux lire `entities_by_tag(TAG_PLANET)`, pour que le compilateur refuse un tag qui n'existe pas.
9. En tant que mainteneur, je veux que les helpers `for_each_tagged` de l'économie prennent un `tag_t`, pour que l'appel et le registre parlent la même langue.
10. En tant que mainteneur, je veux lire `entities_by_message(ENTITY_MESSAGE_HIT_TEST)` et `draw_layers(ENTITY_MESSAGE_DRAW)`, pour que les cinq messages du registre soient un ensemble fermé visible dans l'en-tête.
11. En tant que mainteneur, je veux qu'une ligne de données dise `.draw_function = DRAW_FUNCTION_DRAW_ROW`, pour que la fonction de dessin soit choisie par un indice dans une table constante, pas par une recherche de nom.
12. En tant que mainteneur, je veux que la table des fonctions de dessin soit `static const`, pour ne plus avoir de registre rempli à l'exécution ni d'appel `draw_functions_init()` dans onze modules.
13. En tant que mainteneur, je veux que `ui_handle_button` reçoive un côté de souris typé (gauche ou droit), pour que le test `button[0] == 'l'` disparaisse.
14. En tant que mainteneur, je veux lire directement `pressed` ou `released` sur le bouton de la souris principale, pour que le choix du champ ne passe plus par une chaîne.
15. En tant que mainteneur, je veux que les recherches par nom dans les tables de données restent telles quelles, pour ne pas confondre un identifiant de code avec une donnée du cart.
16. En tant que mainteneur, je veux un critère écrit qui sépare « nom que le code choisit » de « donnée », pour trancher les cas limites sans rouvrir le débat.
17. En tant qu'outil de vérification, je veux que `make state` écrive exactement le même texte (`unknown`, `known`, `colonized`, `empty`, `-`, `placing`…), pour que les scripts et les comparaisons d'avant l'étape restent valables.
18. En tant que testeur, je veux que les 26 e2e rejouent avec des `*.sha256` identiques, pour prouver qu'aucun pixel ne bouge.
19. En tant que testeur, je veux que les hachages figés des TU qui replient un nom d'état ou de fonction de dessin gardent leur valeur, pour que la migration prouve qu'elle lit les mêmes valeurs qu'avant.
20. En tant que testeur, je veux que le seul hachage figé qui replie l'identité de pointeur des chaînes d'état (`ENTITY_STATE_HASH`) soit refigé une fois, avec la raison dans le message de commit, parce que cette identité n'existe plus.
21. En tant que développeur, je veux que chaque famille avance en un commit, pour qu'un rejeu rouge désigne une seule famille.
22. En tant que développeur, je veux que `make test` reste à ~4,5 s et `make e2e` à ~0,25 s (chaud), pour garder la boucle rapide.
23. En tant que développeur, je veux qu'aucun garde ni aucune cible ne s'ajoute à `make test`, puisque `-Werror` refuse déjà de passer une chaîne là où un `enum` est attendu.
24. En tant que lecteur des docs, je veux que GAP-3 ne cite plus `planet_is_colonized` comme comparaison par contenu et que GAP-8 écrive `entities_by_tag(TAG_TARGETING)`, pour que `docs/trous-de-portage.md` décrive le code réel.
25. En tant que lecteur des en-têtes, je veux qu'aucun commentaire ne parle plus de « state name », de « borrowed string » d'état ni d'« unknown tag », pour qu'aucun commentaire ne décrive un contrat disparu.
26. En tant qu'auteur du plan 10 (PSX), je veux des en-têtes d'entité sans pointeur de chaîne d'état, pour que l'en-tête commun soit plus petit et sans `strcmp` dans la boucle de mise à jour.

## Implementation Decisions

### Critère : ce qui part, ce qui reste

- **Part** (devient `enum`) : un ensemble fermé de noms que le **code** choisit et compare, sans que la chaîne soit jamais affichée, lue d'un fichier ou fabriquée à l'exécution. Cinq familles, inventoriées ci-dessous.
- **Reste** (chaîne) :
  - la **clé** d'une table de données et la recherche par nom qui va avec : `planet_types_lookup` (dont `planet_types_lookup("vanishing")`, qui cherche le **type de planète** nommé `vanishing`, pas l'état), les tables de `map_tables.c` (codes et noms de ressources, tables de production, technologies), `planet_kinds[].class_name` (`planet_family.c`), `g_projects_by_name` (`research.c`) ;
  - un **identifiant fabriqué** à l'exécution : `planet_id`, les `ids` de `slipway.c` ;
  - un **texte de jeu** : `required_tech`, la liste `invented` et `collect_contains_string` (GAP-3 reste pour eux), les textes affichés, `status_text` ;
  - un **texte d'outil** : la sortie de `devtools/state.c`, `argv` de `cmd/`, les clés de `input_json`.
- Un nom d'état peut ressortir en texte (dump `make state`, hachages figés des TU) : il est rendu par une fonction de nom, pas stocké.

### Famille A — Boutons de souris (`"left"`/`"right"`, `"pressed"`/`"released"`)

Inventaire (HEAD) : `src/game/ui.c` — 11 occurrences : `ui_main_mouse_button_table(const char *bt)` (assert + choix gauche/droit), `ui_main_mouse_button_field_bool(bt, field)` (choix `pressed`/`released` + assert), `ui_handle_button(self, const char *button)` (assert + `is_left = button[0] == 'l'`), appels `ui_handle_button(self, "left")`, `(self, "right")`, `ui_main_mouse_button_field_bool("left", "pressed")` dans `ui_update` ; `src/game/ui.h` — doc et prototype de `ui_handle_button` ; `tests/test_ui.c` — `handle_button_on_x` (`ui_handle_button(e, "x")`, test de mort par `SIGABRT`) et le commentaire « no button but "left"/"right" ».

- Nouveau type dans `game/ui.h` : `typedef enum { MOUSE_SIDE_LEFT, MOUSE_SIDE_RIGHT } mouse_side_t;`. Nom choisi pour ne pas se confondre avec les bits du masque brut de l'étape 2 (`P8_MOUSE_LEFT`…, H-2-3) : le côté droit de l'interface lit le bouton droit **ou** milieu, ce n'est pas un bit.
- `ui_handle_button(ui_t *self, mouse_side_t side)`. `is_left` devient `side == MOUSE_SIDE_LEFT`.
- `"pressed"`/`"released"` ne deviennent **pas** un `enum` : le sélecteur de champ disparaît. Le helper statique rend le bouton (ou `NULL` quand la souris principale n'en a pas ou que `has_held` est 0) et l'appelant lit `->pressed` ou `->released`. C'est plus court qu'un `enum` à deux valeurs et un `switch` (skill `guideline`, simplicité).
- Les `assert` « left or right only » partent : le type les rend sans objet. L'assert « reading a button of an absent main mouse stops the run » reste.
- Le test de mort `handle_button_on_x` part avec le contrat qu'il couvrait.

### Famille B — Fonctions de dessin des lignes de données (`"draw_row"` et ses quatre sœurs)

Inventaire (HEAD) : cinq noms, `draw_bar`, `print_outlined`, `draw_help_frame`, `spr`, `draw_row`.
- `src/game/map_tables.h` : champ `const char *draw_function` de `data_row_t` ; `src/game/game_data.h` : même champ dans `score_rows_entry_t`.
- `src/game/game_data.c` : 30 initialiseurs `.draw_function = "…"` (5 au niveau `score_rows_entry_t`, 25 sur des `data_row_t`).
- `src/game/score_panel_display.c` : 1 (`"draw_row"` d'un article fabriqué) ; `src/game/score_panel.c` : 1 copie de champ ; `src/game/message_box.c` : 1 `strcmp(e->draw_function, "draw_row")` ; `src/game/layout.c` : 3 asserts `draw_function == NULL`.
- `src/game/draw_functions.c/.h` : registre dynamique `draw_functions_dispatch_register/lookup/clear`, capacité 8, noms copiés par `strncpy`, rempli par `draw_functions_init()`.
- `draw_functions_init()` : 11 appels (`cart.c`, `menu.c`, `message_box.c`, `mouse.c`, `planet.c`, `planet_family.c`, `planet_family_trade.c`, `score_panel.c`, `scroll.c`, `structure_icon.c`, `ui.c`), 10 mentions en en-tête (« Calls draw_functions_init() »), et l'assert de `help.c` (`draw_functions_dispatch_lookup("spr") != NULL`).
- Tests : `test_layout_row.c` (2 `"draw_row"`), `test_frame_zone.c` (1 `strcmp`), `test_score_panel.c` (1 `strcmp`, 1 repli de nom dans un hachage), `test_help_screen.c` et `test_research_project.c` (1 repli de nom chacun).

Décisions :
- `typedef enum { DRAW_FUNCTION_NONE, DRAW_FUNCTION_DRAW_BAR, DRAW_FUNCTION_PRINT_OUTLINED, DRAW_FUNCTION_DRAW_HELP_FRAME, DRAW_FUNCTION_SPR, DRAW_FUNCTION_DRAW_ROW, DRAW_FUNCTION_COUNT } draw_function_t;` dans `game/map_tables.h` (là où vit `data_row_t`, que `game_data.h` inclut). `NONE` vaut 0 : une ligne zéro-initialisée reste « sans fonction » comme avec `NULL`.
- Le registre devient une table `static const draw_functions_fn_t[DRAW_FUNCTION_COUNT]` indexée par l'`enum`, `NONE` à `NULL`. `draw_functions_dispatch_register`, `_lookup`, `_clear`, `DRAW_FUNCTIONS_DISPATCH_CAPACITY` et `draw_functions_init()` disparaissent, avec leurs 11 appels, leurs mentions d'en-tête et l'assert de `help.c` : sans registre à remplir, ces appels sont des orphelins créés par l'étape (règle « nettoyer son propre désordre »). Le contrat « une ligne sans fonction ne dessine rien » reste et reste écrit.
- Rejeté : garder le registre en le clé par `enum` — même résultat, onze appels d'init et un ordre d'initialisation à vérifier en plus.

### Famille C — Messages du registre (`"draw"`, `"draw_overlay"`, `"hit_test"`, `"new_year"`, `"refresh"`)

Le plan ne les cite pas par leur nom ; ce sont des « noms passés en chaîne » au même titre que `"left"` (interprétation notée en Further Notes).

Inventaire (HEAD) : `src/engine/entity_registry.c` — `message_names[]`, `#define MESSAGE_COUNT 5`, champ `name` des entrées, recherche `strcmp` dans `entities_by_message`, `is_message_name` à `switch` sur 0..4, `draw_layers(const char *message)` à `strcmp` + assert ; `src/engine/entity_registry.h` — doc et prototypes ; appels : `cart.c` (2 `draw_layers`), `economy.c` (2), `ui.c` (1). Tests : 125 occurrences dans 30 fichiers (dont `draw_layers("draw")` ×25, `("draw_overlay")` ×6 et des tableaux locaux de noms parcourus en boucle).

Décisions :
- `typedef enum { ENTITY_MESSAGE_DRAW, ENTITY_MESSAGE_DRAW_OVERLAY, ENTITY_MESSAGE_HIT_TEST, ENTITY_MESSAGE_NEW_YEAR, ENTITY_MESSAGE_REFRESH, ENTITY_MESSAGE_COUNT } entity_message_t;` dans `engine/entity_registry.h`. Préfixe `ENTITY_MESSAGE_` pour ne pas se confondre avec `MESSAGE_BOX_*`.
- `entities_by_message(entity_message_t)` ne rend plus jamais `NULL` (le cas « nom inconnu » n'existe plus) ; la doc le dit. `draw_layers(entity_message_t)` garde son assert « only draw and draw_overlay » : `HIT_TEST` y reste représentable, c'est un vrai mauvais usage.
- `is_message_name` s'écrit par `case ENTITY_MESSAGE_…` ; `message_names[]` et le champ `name` disparaissent.

### Famille D — Tags du registre (`entities_by_tag("planet")`…)

Inventaire (HEAD) : `tag_t` et `TAG()` existent déjà ; `entity_tag_named` (dans `entity_class.h`) retraduit la chaîne, seul appelant `entities_by_tag`. Appels dans `src/` : 30 dans 12 fichiers — `devtools/state.c` 6, `economy.c` 4 (dont 2 via `for_each_tagged`), `economy_score.c` 5 (dont 2 via `for_each_tagged`), `economy_tiers.c` 4 (via `for_each_tagged`), `economy_formulas.c` 1, `background.c` 1, `galaxy.c` 1, `planet.c` 1, `probe.c` 2, `slipway.c` 2, `wormhole.c` 1 ; trois copies de `for_each_tagged(const char *tag, …)` (`economy.c`, `economy_score.c`, `economy_tiers.c`). Tests : 168 appels dans 27 fichiers (`"planet"` ×68 au total src+tests, `"slipway"` ×27, `"probe"` ×18, `"structure"` ×16, `"node"` ×14, `"wormhole"` ×13, `"message_box"` ×12, `"ascension"`, `"slipgate"`, `"targeting"` ×8 chacun) et 3 `entities_by_tag("background")` (`test_background.c:224`, `test_planet.c:482,485`) — un tag qui n'existe pas dans `tag_t`, donc toujours `NULL`.

Décisions :
- `entities_by_tag(tag_t tag)`. Le contrat « `NULL` si aucune entité de ce tag n'a été enregistrée depuis `entity_registry_init()` » reste (`seen`) ; « an unknown tag included » part.
- `entity_tag_named` disparaît. Les trois `for_each_tagged` prennent un `tag_t` (les dédoublonner est hors étape).
- Les trois asserts `entities_by_tag("background") == NULL` partent (le contrat d'un tag inconnu n'existe plus) ; les autres conjoints de ces asserts (`TAG_PLANET`, `TAG_NODE`, `TAG_STRUCTURE` encore `NULL`) restent.
- `docs/trous-de-portage.md` GAP-8 : `entities_by_tag(TAG_TARGETING)`.

### Famille E — États d'entité (`"placing"`, `"settled"`…)

Inventaire (HEAD) : sept noms — `normal`, `settled`, `vanishing`, `placing`, `unfolding` (déjà dans `state_t`, avec une méthode possible) et `unknown`, `known` (états de planète hors de `state_t`, sans méthode), plus l'absence (`NULL`).
- `src/engine/entity_class.h` : `state_t`, `entity_state_of`, `entity_t.state` en `const char *` ; `src/engine/entity.c/.h` : `entity_set_state(entity_t *, const char *)`, `entity_init_instance(…, const char *state, …)` et leurs docs « borrowed » ; `src/engine/bootstrap.c` : `update_entities` (`entity_state_of` à chaque entité) et son commentaire.
- `src/game/game_data.h` : 16 champs `const char *state` d'archétypes, `kind_record_t.state`, `targeting_archetype_t.confirmed_state`, `planet_types_entry_t.state` ; `src/game/game_data.c` : 15 `.state = "normal"/"placing"`, 4 constantes `*_state_text` (`planet`, `structure`, `structure_confirmed`, `protostar`), 3 `confirmed_state`, la ligne `vanishing` de `planet_types_table`, les `kind_record` qui pointent `structure_state_text`.
- Lecteurs et écrivains : `planet.c` (`planet_grab_arena_zone_released(…, const char *state)`, `planet_is_colonized`, le tirage `vanishing`/`known` qui appelle `entity_set_state`), `planet_display.c` (4 `strcmp`, 1 `entity_set_state("settled")`), `probe.c` (`targeting_tooltip_show(…, const char *state, …)`, 2 `strcmp`, `entity_set_state(confirmed_state ?: "normal")`), `wormhole.c`, `wormhole_display.c`, `economy_tiers.c`, `slipway.c`, `structure_icon.c/.h` (`view.state`), `targeting.h`, `devtools/state.c` (`planet_state`, `slipway_placed`, `count_colonies`). 16 appels `entity_init_instance` qui passent `archetype.state` (inchangés au texte).
- Tests : 30 fichiers — `strcmp(…state, "…")`, 14 `entity_set_state(…, "…")`, 9 `assert_same_string(header.state, archetype.state)` (`test_planet_spec_defaults.c`), 10 `record->state == …` (`test_entity_kind.c`), ~50 replis `fold_string_member/fold_string_or_nil(…, …state)` dans des hachages figés (19 fichiers), et `test_entity_state_by_name` de `test_ui.c` (`identity_of`, `ENTITY_STATE_HASH`).

Décisions :
- `state_t` devient, dans cet ordre : `ENTITY_STATE_NONE` (0), `ENTITY_STATE_NORMAL`, `_SETTLED`, `_VANISHING`, `_PLACING`, `_UNFOLDING`, `_UNKNOWN`, `_KNOWN`, `STATE_COUNT`. `NONE` à 0 garde l'invariant d'`entity_init_header` (en-tête à zéro = sans état). `states[STATE_COUNT]` grandit de trois pointeurs nuls par classe ; les initialiseurs désignés `[ENTITY_STATE_SETTLED] = …` ne bougent pas.
- `entity_t.state` devient `state_t`. `entity_set_state(entity_t *, state_t)` et `entity_init_instance(…, state_t, …)` ; « borrowed » part de leurs docs. Tous les champs d'état des données (`state`, `confirmed_state`, `planet_types_entry_t.state`) deviennent `state_t`, `ENTITY_STATE_NONE` pour « absent ». Les quatre `*_state_text` disparaissent.
- `update_entities` appelle `states[qc->state]` quand il est non nul ; `entity_state_of` disparaît. Le commentaire qui justifiait le nom lu à l'exécution est réécrit (le piège qu'il signale — aucune méthode d'état n'est appelée par son nom, grep les voit mortes — reste vrai).
- Nouvelle fonction `const char *entity_state_name(state_t)` dans `engine/entity.h` : rend `"normal"`, `"settled"`… et `NULL` pour `ENTITY_STATE_NONE` (contrat écrit). Un seul lecteur dans `src/` : `devtools/state.c`, dont `planet_state` garde exactement ses sorties (`-`, `unknown`, `known`, `colonized`, `empty`, sinon le nom). Les TU s'en servent pour replier le même texte qu'avant dans leurs hachages.
- `targeting_tooltip_show(…, state_t, …)` et `planet_grab_arena_zone_released(…, state_t)`.
- `docs/trous-de-portage.md` GAP-3 : retirer « un module a sa propre comparaison par contenu (`planet_is_colonized`) » ; le reste de GAP-3 (`collect_contains_string`, `invented`) reste.
- Effet de bord assumé : l'en-tête commun perd un pointeur (8 octets sur PC avec le remplissage) ; les tailles de cases de `ui_zone.c`/`particle_zone.c` sont des plafonds, ni retouchés ni dépassés.

### Communs

- Aucun garde ajouté à `make test` : `-std=c99 -Wall -Wextra -Werror` refuse une chaîne passée à un paramètre `enum` (`-Wint-conversion`), et `strcmp` sur un `enum` ne compile pas. Le critère de fin est un `git grep` (Testing Decisions).
- `pico8/lua.lua` reste l'archive : ses chaînes ne bougent pas.

## Implementation Chunks

Les cinq familles sont indépendantes entre elles ; l'ordre va de la plus petite à la plus grande, pour qu'un rejeu rouge tombe sur la famille la plus récente. Un commit par chunk, chacun avec `make test-all` et `make sanitize-net` verts et les 26 `*.sha256` inchangés.

1. **Boutons de souris** (famille A) — `ui.c`, `ui.h`, `test_ui.c`. Seul chunk qui touche le voisinage de l'étape 2 (H-2-1, H-2-2) : à faire une fois l'étape 2 commitée.
2. **Fonctions de dessin** (famille B) — `map_tables.h`, `game_data.h/.c`, `draw_functions.c/.h`, `message_box.c`, `score_panel*.c`, `layout.c`, `help.c`, les 11 modules qui appelaient `draw_functions_init()` et leurs en-têtes, 5 fichiers de tests.
3. **Messages du registre** (famille C) — `entity_registry.c/.h`, `cart.c`, `economy.c`, `ui.c`, 30 fichiers de tests.
4. **Tags du registre** (famille D) — `entity_class.h`, `entity_registry.c/.h`, 12 fichiers de `src/`, 27 fichiers de tests, GAP-8.
5. **États d'entité** (famille E) — `entity_class.h`, `entity.c/.h`, `bootstrap.c`, `game_data.h/.c`, les lecteurs listés, `devtools/state.c`, 30 fichiers de tests, GAP-3. Le plus gros ; s'il faut le couper, le seul découpage qui compile à chaque pas est : (5a) ajouter `UNKNOWN`/`KNOWN`/`NONE` à `state_t` et `entity_state_name` sans changer le type du champ, (5b) basculer le type du champ et tous ses lecteurs d'un coup.

## Testing Decisions

- Un bon test observe le comportement extérieur : le framebuffer des e2e, le texte de `make state`, les listes du registre, l'état d'une entité après des ticks scriptés. Il ne vérifie pas qu'un nom est stocké sous telle forme.
- **Non-régression des pixels** : les 26 `*.sha256` de `tests/e2e/` inchangés à chaque chunk, aucun `make e2e-baseline`. `make sanitize-net` vert à chaque chunk (l'en-tête change de taille, ASan doit le confirmer).
- **Hachages figés des TU** : la règle est de **ne pas les bouger**. Là où un test replie un nom d'état (`fold_string_member`, `fold_string_or_nil`), il replie `entity_state_name(…)` — `NULL` pour `NONE` — et la valeur figée reste identique. Là où il replie un nom de fonction de dessin (`test_help_screen.c`, `test_research_project.c`, `test_score_panel.c`), le test garde une table locale `static const char *const` indexée par `draw_function_t` qui rend les mêmes cinq noms : un nom qui n'existe que pour un hachage de test n'a rien à faire dans `src/`. Les tableaux locaux de noms de messages et de tags deviennent des tableaux d'`enum` ; ils ne replient que des rangs, leurs hachages ne bougent pas.
- **Seule exception** : `ENTITY_STATE_HASH` de `test_entity_state_by_name` (`test_ui.c`) replie `identity_of(state)`, l'identité de pointeur de chaque chaîne d'état (GAP-3), et vérifie qu'`entity_set_state` emprunte la chaîne `state_name`. Ces deux faits disparaissent avec l'étape : le test garde ses vérifications de valeur (la liste `want` en `enum`, `NONE` après remise à zéro), perd `identity_of` et l'emprunt, et `ENTITY_STATE_HASH` est refigé une fois dans le chunk 5, la raison dans le message de commit.
- **Texte de `make state`** : `tests/test_state.c` (prior art : `trade_rich_mine`, qui affirme `count colonies 7` via le chemin « settled ») reçoit, dans le chunk 5, un `has(...)` sur une ligne `planet` d'une planète colonisée et une d'une planète inconnue, texte relevé sur le dump **avant** le changement. C'est le seul lecteur de `entity_state_name` dans `src/`, et aucun e2e ne le couvre.
- **Tests qui partent avec leur contrat** : `handle_button_on_x` (chunk 1), les trois asserts `entities_by_tag("background")` (chunk 4), `assert_same_string` sur l'état (devient `==` sur l'`enum`, chunk 5). Aucun test de « nom inconnu » n'est remplacé : le type le rend inexprimable.
- **Aucun test nouveau isolé** pour les `enum` eux-mêmes : ils n'ont pas de comportement. `entity_state_name` est couvert par les hachages figés (tous les noms) et par `test_state.c`.
- **Durées** : aucun binaire de test nouveau, aucune cible nouvelle ; `make test` ≤ ~4,5 s et `make e2e` ≤ ~0,25 s (chaud), mesurés comme avant l'étape 1.
- **`make raylib`** compile toujours (chunk 1 et chunk 3 touchent `ui.c`/`cart.c`, liés dans le binaire interactif), sans le lancer.
- **Critère de fin** : `git grep -nE 'entities_by_tag\("|entities_by_message\("|draw_layers\("|entity_set_state\([^,]*, *"|entity_state_of|entity_tag_named|draw_functions_dispatch|draw_functions_init|ui_handle_button\([^,]*, *"|"(pressed|released)"' -- src tests` ne rend rien, et `git grep -nE '"(normal|settled|vanishing|placing|unfolding|unknown|known|draw_row|draw_bar|print_outlined|draw_help_frame|spr)"' -- src tests` ne rend que des données qui restent par le critère (clé `"vanishing"` de `planet_types_table` et son `planet_types_lookup("vanishing")`, la table de `entity_state_name`, les sorties de `devtools/state.c`, les tables locales de noms des tests, les textes affichés).

## Out of Scope

- Les recherches par nom dans les tables de données (types de planète, ressources, technologies, projets, sortes) et les identifiants fabriqués (`planet_id`) : ils restent des chaînes, par décision du plan.
- GAP-3 lui-même (`collect_contains_string`, `invented`) : il reste, seule la mention de `planet_is_colonized` part.
- Dédoublonner les trois `for_each_tagged` de l'économie, renommer `state_t`/`tag_t`, réordonner `tag_t`.
- Le format des sorties d'outils (`state.txt`, `events.json`, `argv`).
- Retoucher les tailles de cases des zones après le rétrécissement de l'en-tête.
- La souris nommée (étape 2), les données hors des assets (étape 4), la mémoire typée (étape 5).
- Tout changement de pixel.

## Further Notes

- Les points « vérifier avec l'utilisateur » de `to-prd` (modules, tests à écrire, découpage) n'ont pas été posés à l'utilisateur : la skill `guideline` a été appliquée à leur place. Ce qu'elle a tranché :
  - **simplicité** : `"pressed"`/`"released"` deviennent deux lectures de champ, pas un `enum` ; le registre de fonctions de dessin devient une table constante, ce qui supprime `draw_functions_init()` et ses onze appels plutôt que de les garder vides ;
  - **chirurgie** : on ne touche ni aux tables de données, ni aux tailles de zones, ni aux trois `for_each_tagged` en double ; les orphelins créés par l'étape (`entity_state_of`, `entity_tag_named`, `message_names`, `*_state_text`, registre de dessin) partent ;
  - **hypothèses en surface** : la famille C (messages) n'est pas citée par le plan ; elle est incluse parce qu'elle est exactement un « nom passé en chaîne » d'ensemble fermé. Si le triage la juge hors étape, le chunk 3 tombe sans toucher aux autres ;
  - **critère vérifiable** : sha256 identiques, hachages figés identiques sauf un nommé, `git grep` de fin.
- Le plan dit « une famille par commit » : les chunks 1 à 5 sont ces commits. Chaque chunk met à jour `docs/trous-de-portage.md` pour ce qu'il fait disparaître (GAP-8 au chunk 4, GAP-3 au chunk 5).
- `tests/test_state.c` relance des rejeux (binaire `tmp/replay`) : l'assert ajouté ne crée pas de rejeu nouveau, il lit un dump déjà produit.

## Hypothèses sur l'issue de l'étape 2

L'étape 2 (souris nommée, `docs/prd/triage/phase6-etape2-souris-nommee.md`) est implémentée en parallèle ; ce PRD est écrit contre HEAD `90b7772` (étape 1 faite, étape 2 pas encore).

- **H-2-1** — Après l'étape 2, aucune chaîne `"left"`, `"right"`, `"pressed"` ou `"released"` n'est dans le code de souris (`mouse.c`, `mouse_button.c/.h`) : elles sont toutes dans `ui.c`/`ui.h` (et `test_ui.c`), que l'étape 2 ne touche pas. La famille A ne dépend donc pas de ce que l'étape 2 réécrit.
- **H-2-2** — `mouse_button_t` garde ses champs `has_held`, `held`, `pressed`, `released`, et `mouse_left_button()`/`mouse_right_button()` gardent leur signature et leur contrat (`NULL` hors de `mouse_init()`..`mouse_shutdown()`). Le helper de `ui.c` qui rend le bouton s'appuie dessus.
- **H-2-3** — L'étape 2 introduit `P8_MOUSE_LEFT`/`P8_MOUSE_RIGHT`/`P8_MOUSE_MIDDLE` (bits du masque brut) et `p8_mouse_t` dans `p8/input.h`, et aucun type ni constante nommé `mouse_side_t` ou `MOUSE_SIDE_*`. Le côté de l'interface (`MOUSE_SIDE_RIGHT` = droit **ou** milieu) n'est pas un de ces bits et ne les réutilise pas.
- **H-2-4** — `mouse.c` garde après l'étape 2 : `.states = {[ENTITY_STATE_NORMAL] = mouse_update_method}`, l'appel `entity_init_instance(…, mouse_archetype.state, …)`, l'appel `draw_functions_init()` dans `mouse_init`, et `mouse.h` la phrase « Calls draw_functions_init() ». Le chunk 2 retire cet appel et cette phrase.
- **H-2-5** — `mouse_update` lit toujours la souris par `dispatch_context_input()` ou l'entrée passée en paramètre, sans passer par un tag, un message ni un nom d'état.
- **H-2-6** — `tests/test_mouse.c` garde `in_any_entities_by_message` et son tableau de noms de messages en chaînes (migré par le chunk 3) ; le test ajouté par l'étape 2 (« milieu seul presse le droit ») n'utilise ni tag, ni message, ni état en chaîne.
- **H-2-7** — Les hachages figés `LEFT_HASH`/`RIGHT_HASH` et le helper `mouse_at(x, y, buttons)` de `tests/test_ui.c` sont inchangés en valeur par l'étape 2 (le helper peut écrire `P8_MOUSE_*` au lieu de `1`/`2`, même valeur). Le chunk 1 ne doit pas les bouger.
- **H-2-8** — L'étape 2 ne touche ni `entity_t`, ni `entity_class.h`, ni le registre, ni `draw_functions`, ni les champs `state`/`draw_function` de `game_data.h`/`map_tables.h`.
- **H-2-9** — L'étape 2 n'ajoute aucun binaire de test ni aucune cible ; les durées de référence (`make test` ~4,5 s, `make e2e` ~0,25 s chaud) tiennent après elle, et les 26 `*.sha256` sont inchangés par elle.
- **H-2-10** — L'étape 2 est commitée avant que le chunk 1 commence ; les chunks 2 à 5 ne dépendent d'elle que par les conflits textuels de `mouse.c`/`mouse.h` (H-2-4) et `test_mouse.c` (H-2-6).

## À revérifier une fois l'étape 2 faite

- **Implementation Decisions → Famille A** : que les chaînes sont toutes dans `ui.c`/`ui.h`/`test_ui.c` et nulle part dans le code de souris (H-2-1) ; la forme de `mouse_button_t` et des accesseurs (H-2-2) ; l'absence de collision de nom avec `mouse_side_t`/`MOUSE_SIDE_*` (H-2-3).
- **Implementation Decisions → Famille B** : la liste des 11 appels de `draw_functions_init()` et des 10 mentions d'en-tête, `mouse.c`/`mouse.h` compris (H-2-4).
- **Implementation Decisions → Famille C** : le compte d'occurrences de tests, `test_mouse.c` compris (H-2-6).
- **Implementation Decisions → Famille E** : l'archétype et l'initialisation de la souris (H-2-4, H-2-8).
- **Implementation Chunks → chunk 1** : l'étape 2 est commitée (H-2-10).
- **Testing Decisions → hachages figés** : `LEFT_HASH`/`RIGHT_HASH` à leur valeur post-étape-2 (H-2-7) ; **→ Durées** : seuils toujours valables (H-2-9) ; **→ Critère de fin** : refaire les deux `git grep` à la HEAD post-étape-2.
