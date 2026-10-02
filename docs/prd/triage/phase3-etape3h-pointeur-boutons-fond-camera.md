---
title: "Plan 5, sous-étape 3h — pointeur, boutons, fond, caméra"
description: "Le pointeur, les boutons, le fond et la caméra (7 modules de game/ et 10 tests) passent aux noms anglais du TSV. Il faut d'abord corriger une locale contraire au §4 et classer trois clés signalées, dont une qui n'est pas une clé."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 3h. **Pointeur, boutons, fond, caméra.** `game/` : `ui`, `ui_button`, `ui_zone`, `mouse`, `mouse_button`, `background`, `scroll`. Tests : `test_ui`, `test_button`, `test_modal_backdrop`, `test_ui_zone`, `test_mouse`, `test_main_mouse`, `test_background`, `test_background_draw`, `test_camera`, `test_main_camera`. Résidu ≤ 126.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4), huitième étape.

Entrées : `docs/plan-5.md` §4, §5, l'entrée 3 avec « Sous-étapes » et « Méthode des sous-étapes », et les notes « Découvert » de **3a à 3g**. Cette PRD y renvoie sans les recopier :
- l'ordre des commandes (3a) ;
- l'appartenance des chaînes et les littéraux composés (règle de 3f) ;
- la recherche des littéraux recollés ;
- la prévision des hachages en worktree jetable ;
- le recâblage après chaque édition de texte et la porte `rename-rekey-check` ;
- les corrections du TSV pour la seule conformité au §4 (TSV et surcharges) ;
- le cas d'une ligne `chaîne` qui n'est pas un nom (`pz`, 3g).

Exemple à suivre : `c167f06` (3g, une seule tranche).

Aucun humain n'était disponible. L'agent rédacteur a chargé lui-même la consigne `guideline` aux points « vérifier avec l'utilisateur » de `to-prd`. Les décisions ci-dessous sont prises sur le texte du plan et sur les mesures, sans interview.

**Condition d'entrée** :
- `origin/plan-5-noms-anglais` au HEAD de départ (`fd2db43` ou un descendant), arbre propre, `make test-all` vert ;
- `make rename-extract` frais, sans ligne `warning:` (les 4 lignes `partial` de `class_coverage` sont attendues) ;
- `make rename-rekey-check SUBSTEP=3h` vert. Mesuré : 0 identité introuvable.

## Problem Statement

Après 3g, le pointeur, les boutons, le fond et la caméra gardent les noms du cart :
- les types `pico8_ui_t`, `pico8_js_t`, `pico8_fq_t`, `pico8_nf_t`, `pico8_bg_t`, `pico8_qb_t`, `pico8_sorte_t` ;
- les fonctions `pico8_ui_lj`, `pico8_ui_jz`, `pico8_ok`, `pico8_je`… ;
- les champs `jn`, `hm`, `qa`, `my`, `h_`…

La sous-étape compte **589 identités**, soit **4 335 occurrences AST dans 75 fichiers**.

Une répétition à blanc a été faite dans un worktree jetable : application, gardes, chaînes, composés, puis `make test`, `make e2e`, extraction fraîche et compteurs. Elle a trouvé :
- **Une locale contraire au §4.** Le TSV renomme `ok` → `ui` la locale de `test_left` (`test_ui.c`), dont la portée fait 52 lignes. C'est la proposition de 3a, encore ouverte. `main_ui`, le nom du vocabulaire, est refusé par l'outil, parce qu'il masque la fonction `main_ui`.
- **Une clé signalée qui n'est pas une clé.** `"go"` (`test_button.c`) est le texte d'un bouton de test, comme ses voisins `"a"`, `"b"` et `"plain"`.
- **Des littéraux composés de 3h que le TSV ne liste pas.** Ce sont des noms de zone, des noms de cas, des messages attendus et des libellés imprimés.
- **Aucune garde ne rougit, aucun hachage ne bouge.**

## Solution

Une seule tranche, dans l'ordre de commandes de la note de 3a, sur `SUBSTEP=3h` :
1. corriger la locale `ok` de `test_left` et passer `go` à `conservé` dans le TSV, avant `rename-slice` ;
2. appliquer les 589 identités ;
3. recâbler ;
4. éditer les 24 sites de chaînes et les littéraux composés de 3h ;
5. recâbler de nouveau, faire les relevés, écrire la note « Découvert ».

La répétition l'a établi : avec ces gestes, `make test` et `make e2e` sont verts (26 e2e conformes), et `make raylib` et `make screencast-viewer` compilent. `class-coverage-net` et `e2e-gap` sont rouges, avec des sorties égales à celles de la fin de 3g.

À la fin :
- aucun nom de 3h ne reste en `identifier` ni en `string` ;
- les fichiers de 3h ne gardent que 126 mentions `identifier` de sous-étapes suivantes, et 0 mention `string` ;
- `scope-count SUBSTEP=3h` passe de 100 à 1 ;
- pas un pixel ne change.

## User Stories

1. En tant qu'agent de 3h, je veux corriger dans le TSV la locale `ok` de `test_left`, qui deviendrait une locale de deux lettres à portée de 52 lignes, afin que le TSV reste conforme au §4 et reste ce qui a été appliqué.
2. En tant qu'agent de 3h, je veux savoir d'avance que `main_ui` est refusé à cause du masquage, afin de ne pas perdre une application de 1 min 30.
3. En tant qu'agent de 3h, je veux appliquer les 589 identités en une commande, afin que le compilateur arbitre tout oubli dans les 75 fichiers.
4. En tant que lecteur, je veux lire `ui_t`, `button_t`, `modal_backdrop_t`, `mouse_t`, `mouse_button_t`, `background_t`, `camera_t` et `slot_kind_t`, afin de comprendre le pointeur et les boutons sans glossaire.
5. En tant que lecteur, je veux savoir que les 29 fonctions exportées sans préfixe de module restent conformes au §4, comme méthodes de leur type ou comme singletons du vocabulaire, afin de ne pas les rouvrir.
6. En tant que mainteneur, je veux que `composition_residue.sh --detail` et `data_residue.sh --detail` restent identiques avant et après, afin de prouver qu'aucune garde ne change de verdict.
7. En tant que mainteneur, je veux savoir qu'aucun motif de garde ne cite un nom de 3h, afin de ne rien traduire à tort.
8. En tant que relecteur humain, je veux le classement des 3 clés signalées de 3h (`go`, `hm`, `jn`), afin de savoir lesquelles vivent.
9. En tant qu'agent de 3h, je veux renommer `"jn"` et `"hm"` des deux côtés dans `ui.c`, afin que l'aiguillage par nom des boutons de la souris reste fermé.
10. En tant qu'agent de 3h, je veux que `"go"` reste tel quel et que sa ligne `chaîne` passe à `conservé`, afin de ne pas renommer un texte de bouton.
11. En tant qu'agent de 3h, je veux renommer les 24 sites de chaînes, afin que `rename-strings-verify SUBSTEP=3h` soit vert, sans site `unseen`.
12. En tant qu'agent de 3h, je veux renommer les noms de zone, les noms de cas, les messages attendus et les libellés écrits et lus dans les fichiers de 3h, afin de ne pas laisser de noms du cart que ni le compteur ni le contrôle ne voient.
13. En tant qu'agent de 3h, je veux savoir d'avance qu'aucun hachage ne bouge, afin qu'un rouge de hachage arrête la tranche.
14. En tant que mainteneur, je veux que la garde de portée tombe de 100 à 1 dans 3h, le reste étant la statique locale `dy`, déjà proposée par 3a.
15. En tant qu'agent de 3i, je veux trouver un TSV recâblé, avec `rename-rekey-check` vert de 3h à 3l.
16. En tant que joueur, je ne veux voir aucun pixel changer.

## Implementation Decisions

### Périmètre

`rename-slice SUBSTEP=3h` rend **589 identités** :
- par module : `ui_button` 115, `test_button` 105, `ui` 80, `test_ui` 47, `ui_zone` 44, `background` 30, `test_background` 28, `scroll` 27, `mouse` 21, `test_main_camera` 21, `test_mouse` 16, `test_camera` 14, `test_modal_backdrop` 14, `test_ui_zone` 8, `mouse_button` 7, `test_main_mouse` 7, `test_background_draw` 5 ;
- par sorte : 263 fonctions, 87 champs, 87 locales, 50 macros, 43 statiques, 27 paramètres, 17 types, 13 constantes, 2 tags.

Occurrences mesurées en répétition : `ref` 2 388, `macro_arg` 784, `decl` 597, `type_ref` 429, `macro_use` 76, `macro_def` 50, `macro_cond` 7, `designator` 4. Il n'y a aucun refus après la correction de `ok`, aucun `##` et aucun défaut de l'outil. `pico8_sorte_t` → `slot_kind_t` passe sans conflit.

Restent en place les 126 mentions `identifier` de sous-étapes suivantes : `test_ui` 50, `test_button` 32, `test_modal_backdrop` 12, `test_camera` 9, `test_main_mouse` 7, `test_background` 5, `test_mouse` 3, `ui` 2, `ui_button` 2, `background` 1, `mouse` 1, `scroll` 1, `test_main_camera` 1.

Pour l'étape 4 :
- 340 `backtick` et 41 `comment` dans les fichiers de 3h ;
- les messages d'`assert` de `ui.c` (`je[bt]: …`, `ui.lheld: …`, `only jn and hm are read`) ;
- les mots français des littéraux (`interface`, `place`, `rangee`, `pleine`, `attendu`, `cas inconnu`).

### Entrée du TSV corrigée (§4 et proposition de 3a)

La locale `ok` de `test_left` (`tests/test_ui.c`) était renommée `ok` → `ui`. Elle est corrigée en `ok` → **`dispatcher`**, dans `docs/renommage.tsv` (1 ligne) et par une ligne `field	test_left.ok	dispatcher	ok` de `tools/rename_overrides.tsv`, avant `rename-slice`.
- Motif : avec une portée de 52 lignes, `ui` reste refusé par la garde de portée (§4). Le nom vient de l'en-tête du fichier, qui décrit `ui` comme « the click dispatcher ».
- Mesuré : `main_ui` (vocabulaire) est refusé par `rename-apply`. La locale masquerait la fonction `main_ui` (`pico8_ok`), déclarée dans `ui.h`, et l'outil refuse le masquage dans les deux sens.
- Les 16 autres `ok` de `test_ui.c` sont `conservé`, à portée ≤ 40 lignes : on n'y touche pas.
- Le cas jumeau de `message_box.c` (60 lignes) reste à 3j.

Les **29 fonctions exportées** de 3h sans préfixe de module (relevé de 3c) sont gardées telles quelles :
- méthodes de leur type receveur (§4) : `camera_*` ×8 (`camera_t`, `scroll.h`), `button_*` ×10 (`button_t`) et `modal_backdrop_*` ×6 (`modal_backdrop_t`), tous deux dans `ui_button.h` ;
- singletons du vocabulaire (`ok`, `je` → `main_ui`, `main_mouse`), comme `main_camera` (3c) et `main_economy*` (3f) : `main_ui`, `main_mouse`, `main_mouse_typed`, `main_mouse_placeholder_class` (`ui.h`), `main_camera_typed` (`scroll.h`).

Pour décision humaine : les singletons `main_*` déclarés hors d'un module `main_*`.

### Clés signalées, classées avant renommage

- **`jn`, `hm`** : **écrivain trouvé, vivantes**. Les 3 appels de l'aiguillage statique `ui_main_mouse_button_field_bool` écrivent `"jn"` ou `"hm"` dans `ui.c`. La même fonction les lit par `strcmp` et rend les champs `pressed` et `released` de `mouse_button_t`. On les renomme `"pressed"` (3 sites) et `"released"` (2 sites), des deux côtés, dans le même fichier. Le message d'`assert` reste à l'étape 4. Un côté oublié ferait sauter l'`assert` de l'aiguillage dès le premier clic, et les e2e le verraient.
- **`go`** : **pas une clé**. `build(keys, "go", NULL)` (`test_ph`, `test_button.c`) est le texte de la rangée d'un bouton, comme `"a"`, `"b"` et `"plain"` dans les tests voisins. `PH_HASH` ne le replie pas. Sa ligne `chaîne` passe à `conservé` (`go` → `go`), dans le TSV seulement, hors de `rename-strings` (figé), comme `pz` en 3g. On le note dans « Découvert » pour l'étape 5.

### Gardes à motifs

L'inventaire de 3a n'a aucun élément en 3h. Mesuré en répétition :
- aucun motif ni aucune exemption de `tools/` ne cite un nom de 3h ;
- `composition-guard` et `data-guard` restent verts après la seule application ;
- `--detail` de `composition_residue.sh` et de `data_residue.sh` identiques avant et après ;
- `--champs` : pied « 0 hors constructeur » 36 → 30, « 0 écriture » 15 inchangé. C'est un majorant, pas une porte : on le note.

Aucune traduction, aucun test de vivacité.

### Chaînes

Les 25 sites de `rename-strings-verify SUBSTEP=3h`, dont 24 à renommer après la décision sur `go` :
- **noms de test** : `test_pico8_bg`, `_bg_kj`, `_js`, `_qb`, `_mo`, `_je`, `_fq`, `_nf`, `_ui`, `_zone_interface` → noms des fichiers ;
- **chemin tmp** : `tmp/test_pico8_zone_interface_stderr.txt` → `tmp/test_ui_zone_stderr.txt` ;
- **clés** :
  - `jn`, `hm` (`ui.c`) ;
  - `eu`, `fe`, `gh`, `dk` (`test_button.c`, libellés de `check` et de `fold_ephemerals`) ;
  - `cj`, `ke_me`, `ke_t` (`test_ui.c`) ;
  - `js_place_full`, `fq_zone_full` (`test_ui_zone.c`, noms de cas écrits et lus par `argv[1]`) ;
- **symbole cité** : `_jo_fe_pt` → `project_planet` (`test_button.c`). Le site de `ui_button.h` passe : le champ y est déclaré sous son nom neuf, et l'ancien ne reste qu'en commentaire (étape 4).

Après l'édition, la répétition mesure 0 site en défaut sur 24 et **0 `unseen`**.

**Littéraux composés de 3h**, écrits et lus dans ses fichiers (règle de 3f) :
- **noms de zone** de `ui_zone.c` : `"interface js"`/`"place js"` → `button`, `fq` → `modal_backdrop`, `mh` → `message_box`, `iv` → `toast`, `le` → `ephemerals`. Le mot français reste : `"interface button"`, `"place button"`. `rangee` est un mot français, laissé à l'étape 4 ;
- **messages attendus** d'`expect_fatal` (`test_ui_zone.c`) : `"zone place button pleine"`, `"zone interface modal_backdrop pleine"` ;
- **noms de cas hors TSV** du même fichier, d'après les fonctions `case_*` renommées : `live_places_follow_takes` → `live_slots_follow_takes`, `place_alive_until_released` → `slot_alive_until_released`, `place_dead_after_release` → `slot_dead_after_release`, `rangee_place_full` → `row_slot_full` ;
- **libellés imprimés sans lecteur** :
  - `test_button.c` : `in ok.le` → `in main_ui.ephemerals`, `my`/`h_`/`jb` → `on_click`/`side_tooltip`/`tooltip`, `eu buttons`, `struct eu` → `start_game`, `pj buttons` → `score_panel buttons`, `[fe]`, `[fe my]`, `fe my`, `[members] fe:` → `projects_panel`, `[dk]`, `[members] dk: … en` → `difficulty`, `type_bias`, `[ph]` → `[update]`, `me=` → `state=`, `not ly` → `not settled`, `[cr my]`, `cr my`, `[members] cr:` → `tech_tree_screen`, `fg on the 4th` → `gpio_signal`, `js members`, `[places] js:` → `button`, `[places] le:` → `ephemerals` ;
  - `test_modal_backdrop.c` : `my true`, `my false` → `on_click` ;
  - `test_main_mouse.c` et `test_main_camera.c` : `[lj qa]` → `[update mouse_world]` ;
  - `test_background.c` : `ldown g_`, `ldown mz` → `left_press probe`, `left_press wormhole` ;
  - `test_camera.c` : `ma back to 0` → `shake back to 0`.

Les noms courts sont pris dans le TSV : ce sont les noms neufs des mêmes clés, champs ou classes. La recherche hors commentaires et hors messages d'`assert`, littéraux adjacents recollés, ne laisse ensuite que des mots français, des clés d'une lettre (`"l"`, `"r"`) et le nom de classe conservé `ui`.

**Tampons** : le plus long nom de zone donne `"zone interface modal_backdrop pleine, limite 4 places"`, soit 55 caractères. Il tient dans `char message[160]` de `zone.c`, et `char cmd[512]` de `test_ui_zone.c` suffit. Aucun tampon ne bouge.

**Hachages** (prévision mesurée en répétition) : **aucun**. Tous les libellés de 3h passent par `check`/`check_int`, qui les impriment sans les replier. Le texte `"go"` n'est pas replié. Tout rouge de hachage arrête la tranche (`help-me-debug`).

### Application et recâblage

- Ordre : celui de la note de 3a, sur `SUBSTEP=3h`, après la correction du TSV.
- Recâblage mesuré : **163 identités** après l'application, **0** après les chaînes.
- `rename-rekey-check` vert de 3h à 3l (mesuré).

### Garde de portée

`scope-count SUBSTEP=3h` vaut **100** : 41 champs, 55 portées > 40, 2 fonctions, 2 statiques.

Après la répétition, il vaut **1** : `dy` de `test_button`, statique locale déjà proposée par 3a. La correction de `ok` ferme la seconde proposition de 3a pour `test_ui.c` ; celle de `message_box.c` reste à 3j. Aucune nouvelle exception.

### Fin de PRD

La liste du §5 et des notes de 3a–3g, sur `SUBSTEP=3h` :
- `make test-all` vert, `tests/e2e/*.sha256` inchangés ;
- `make raylib` et `make screencast-viewer` compilent et linkent, sans jamais être lancés ;
- `rename-tool-test` (37 tests) et les `rename-*-test` verts ;
- `class-coverage-net` et `e2e-gap` rouges à l'identique de la fin de 3g (mesuré) ;
- `data-residue` et `composition-residue` à 0 ;
- après `rename-extract` frais sans `warning:` :
  - `cart-names-count SUBSTEP=3h` : `done` 0 en `identifier` et en `string`, `later identifier` 126 ≤ 126, `later string` 0 ;
  - `rename-strings-verify SUBSTEP=3h` : 0 site en défaut sur 24, 0 `unseen` ;
  - `rename-rekey-check` vert de 3h à 3l ;
  - `scope-count SUBSTEP=3h` à 1.

## Implementation Chunks

1. **Sous-étape 3h entière, note comprise.**
   - Un commit de code qui laisse `make test-all` vert. Il contient :
     - la correction `ok` → `dispatcher` (TSV et surcharge) et `go` passé à `conservé` ;
     - l'application ;
     - les 24 sites de chaînes et les littéraux composés ;
     - le TSV recâblé.
   - La note « Découvert » sous l'entrée 3h, dans le même commit ou dans un commit de docs qui suit. Elle relève :
     - les identités et les occurrences ;
     - la correction de `ok`, avec le refus de `main_ui` ;
     - les 29 fonctions gardées ;
     - le classement de `jn`, `hm` et `go` ;
     - les gardes sans élément, `--detail` et `--champs` ;
     - les littéraux composés ;
     - l'absence de hachage et de tampon touchés ;
     - les recâblages ;
     - ce qui va à la décision humaine : les singletons `main_*`, et la ligne `chaîne` `go` éditée hors de `rename-strings` ;
     - les relevés de fin.
   - Le plan est corrigé dans le même commit si un constat le contredit.

## Testing Decisions

- **Code de 3h.** Le filet est le compilateur, `make test` et les e2e à l'octet. Aucun test neuf. Aucun hachage ne doit bouger : un rouge de hachage est un signal d'arrêt, pas une valeur à reporter.
- **`jn`/`hm`.** La preuve est l'`assert` de l'aiguillage, joint aux e2e : un côté oublié échoue au premier clic.
- **Noms de cas de `test_ui_zone`.** Les cas sont écrits et lus dans le même fichier. Un nom renommé d'un seul côté fait sortir l'enfant en 2 (`cas inconnu`), et le test échoue.
- **Gardes.** Pas de test neuf : aucun motif n'est touché. `--detail` avant et après sert de preuve.
- **Outils de méthode.** Inchangés : le schéma du TSV et ses lecteurs restent figés.

## Out of Scope

- Les noms de 3i–3l dans les fichiers de 3h (le résidu de 126).
- Commentaires, messages d'`assert`, mentions entre backticks ou nues, mots français des littéraux (`interface`, `place`, `rangee`, `pleine`) : étape 4.
- La locale `ok` de `message_box.c` : 3j.
- Trancher les singletons `main_*`, les exceptions de portée et les motifs génériques : point d'arrêt d'avant l'étape 5.
- Renommer par goût un nom conforme au §4.
- Lancer l'appli raylib.

## Further Notes

- Mesures de cette PRD, sur l'arbre `fd2db43` avec un `rename-extract` frais, sans `warning:` :
  - `cart-names-count SUBSTEP=3h` : `done` 3 629 `identifier`, 340 `backtick`, 32 `string`, 41 `comment` ; `later` 126 `identifier` (borne 126), 128 `backtick`, 0 `string`, 10 `comment` ;
  - `rename-strings-verify SUBSTEP=3h` : 25 sites, 25 en défaut, 0 `unseen` ;
  - `scope-count SUBSTEP=3h` : 100 ;
  - `rename-rekey-check SUBSTEP=3h` : 0 introuvable.

  `tmp/ast_rename/slice.tsv` et `cart_names_count.txt` ont été rendus à leur état d'avant la mesure.
- Répétition à blanc dans un worktree jetable, détruit depuis. Elle a fait la correction du TSV (sans la surcharge), l'application, les chaînes, les composés et les recâblages, puis `make test`, `make e2e`, `raylib`, `screencast-viewer`, `rename-tool-test`, `class-coverage-net`, `e2e-gap`, une extraction fraîche et les compteurs. Tous les chiffres ci-dessus en viennent : 76 fichiers touchés, TSV compris.
- `rename-table-check` reste rouge par construction : la porte est `rename-rekey-check`.
- Durées : chaque `rename-extract` ou `rename-apply` prend environ 1 min 30, et la tranche en enchaîne quatre.
