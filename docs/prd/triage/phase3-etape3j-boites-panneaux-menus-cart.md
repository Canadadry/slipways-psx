---
title: "Plan 5, sous-étape 3j — boîtes, panneaux, menus, cart"
description: "Les boîtes de message, le panneau de score, les menus, l'aide, le cart et la triche (game/) et leurs quatre tests passent aux noms anglais du TSV, avec une correction du TSV pour le §4 (la locale ok de message_box.c) et trois constantes de hachage à reporter."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 3j. **Boîtes, panneaux, menus, cart.** `game/` : `message_box`, `score_panel`, `score_panel_display`, `menu`, `help`, `cart`, `cheat`. Tests : `test_message_box`, `test_toast`, `test_score_panel`, `test_help_screen`. Résidu ≤ 4.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4), dixième étape.

Entrées : `docs/plan-5.md` §1, §4, §5, l'entrée 3 avec « Sous-étapes » et « Méthode des sous-étapes », et les notes « Découvert » de **3a à 3i**. Cette PRD y renvoie sans les recopier :
- l'ordre des commandes (3a) ;
- l'appartenance des chaînes ;
- les littéraux composés (règle de 3f) ;
- les clés courtes hors TSV d'une liste hachée (`"i"`/`"o"` de 3g, `"c"`/`"w"` de 3i) ;
- la recherche des littéraux recollés ;
- la prévision des hachages en worktree jetable ;
- le recâblage après chaque édition de texte, avec `rename-rekey-check` pour porte ;
- la correction du TSV d'une locale hors tolérance (3h, `test_ui.c`).

Exemple à suivre : `87b4a63` (3i, une seule tranche).

Aucun humain n'était disponible. L'agent rédacteur a chargé lui-même la consigne `guideline` aux points « vérifier avec l'utilisateur » de `to-prd`. Les décisions ci-dessous reposent sur le texte du plan et sur les mesures, sans interview.

**Condition d'entrée** :
- `origin/plan-5-noms-anglais` est au HEAD de départ (`b13e450` ou un descendant), l'arbre est propre et `make test-all` est vert ;
- `make rename-extract` est frais, sans ligne `warning:` (les 4 lignes `partial` de `class_coverage` sont attendues) ;
- `make rename-rekey-check SUBSTEP=3j` est vert. Mesuré : 0 identité introuvable.

## Problem Statement

Après 3i, les modules d'interface de haut niveau gardent les noms du cart :
- `pico8_mh_*`, `pico8_iv_*`, `pico8_pz*` (boîtes de message, toasts) ;
- `pico8_pj_*`, `pico8_bz_row_t`, `pico8_eg` (panneau de score) ;
- `pico8_cr`, `pico8_ha_pz`, `pico8_ix_*` (menus) ;
- `pico8_fv_*`, `pico8_help_*` (aide) ;
- `pico8__init`, `pico8__update60`, `pico8__draw`, `pico8_cart_load`, `pico8_eu` (cart) ;
- `pico8_cheat_*` ;
- leurs champs (`at1`, `at2`, `ra`, `lh`, `rh`, `ca`, `gq`, `hi`, `ep`, `c_`, `gz`, `ed`…) et, dans les quatre tests, les locales et assistants (`h`, `ju`, `ea`, `cr`, `ents_listed`, `call_*`…).

La sous-étape compte **390 identités**, soit **2 529 occurrences AST dans 59 fichiers**. C'est la dernière sous-étape de `game/` : son résidu n'est plus que 4.

Une répétition à blanc a été faite dans un worktree jetable, avec les étapes suivantes :
1. correction du TSV ;
2. application ;
3. `make test` ;
4. gardes ;
5. chaînes et littéraux composés ;
6. `make test` et `make e2e` ;
7. extraction fraîche et compteurs.

Elle a trouvé :
- **Une correction du TSV pour le §4** : la locale `ok` → `ui` de `message_box_open_box` (portée de 60 lignes), proposée par 3a.
- **Aucun refus, aucune garde à traduire.** `make test` reste vert après la seule application.
- **Trois hachages bougent**, par les clés du TSV et les clés courtes hors TSV repliées dans `test_score_panel.c` et `test_help_screen.c`.

## Solution

Une seule tranche, dans l'ordre de commandes de la note de 3a, sur `SUBSTEP=3j` :
1. corriger le TSV (`ok` → `dispatcher`) ;
2. appliquer les 390 identités ;
3. recâbler ;
4. éditer les 14 sites de chaînes encore en défaut, puis les littéraux composés de 3j ;
5. reporter les trois constantes de hachage rouges ;
6. recâbler de nouveau, faire les relevés et écrire la note « Découvert ».

La répétition l'a établi : avec ces gestes, `make test` et `make e2e` sont verts (26 e2e conformes), et `make raylib` et `make screencast-viewer` compilent. `class-coverage-net` et `e2e-gap` sont rouges, avec des sorties égales à l'octet à celles de la fin de 3i.

À la fin :
- aucun nom de 3j ni des précédentes ne reste en `identifier` ni en `string` dans les fichiers de 3j ;
- ces fichiers ne gardent que 4 mentions `identifier` de sous-étapes suivantes (`pico8_class_coverage_record`, 3k), et 0 mention `string` ;
- `scope-count SUBSTEP=3j` passe de 81 à **0** ;
- pas un pixel ne change.

## User Stories

1. En tant qu'agent de 3j, je veux appliquer les 390 identités en une commande, afin que le compilateur arbitre tout oubli dans les 59 fichiers.
2. En tant qu'agent de 3j, je veux savoir d'avance que l'application seule laisse `make test` vert, afin qu'un rouge à ce stade m'arrête au lieu d'être pris pour un hachage à reporter.
3. En tant que lecteur, je veux lire `message_box_*`, `toast_*`, `score_panel_*`, `menu_show_tech_tree_screen`, `help_screen_*`, `cart_init`, `cart_update60`, `cart_draw`, `cart_load` et `cheat_*`, afin de comprendre l'interface et la boucle du jeu sans glossaire.
4. En tant que mainteneur, je veux que la locale `ok` de `message_box_open_box` reçoive un nom long, afin que la garde de portée tombe à 0 sans exception nouvelle.
5. En tant que lecteur, je veux savoir que les 18 fonctions exportées sans le préfixe de leur fichier restent conformes au §4, afin de ne pas les rouvrir.
6. En tant que mainteneur, je veux que `data_residue.sh --detail` reste identique, et que `composition_residue.sh --detail` ne diffère que par le nom cité de la ligne 10, afin de prouver qu'aucune garde ne change de verdict.
7. En tant qu'agent de 3j, je veux renommer les 4 noms de test, les 6 clés hachées de `test_score_panel.c`, les 3 symboles cités en défaut et les 2 noms de `composition_residue_sites.txt`, afin que `rename-strings-verify SUBSTEP=3j` soit vert, sans site `unseen`.
8. En tant qu'agent de 3j, je veux renommer les clés courtes `"c"` et `"fn"` repliées par `test_score_panel.c` et `test_help_screen.c`, afin qu'aucune clé du cart ne survive dans un hachage écrit et lu dans 3j.
9. En tant qu'agent de 3j, je veux connaître d'avance les trois constantes qui rougissent et leurs nouvelles valeurs, afin que tout autre rouge de hachage arrête la tranche.
10. En tant qu'agent de 3j, je veux fermer la mention `pico8_cart_load` laissée en `later string` par 3g, afin que 3g et 3j finissent sans `string`.
11. En tant qu'agent de 3k, je veux trouver un TSV recâblé, avec `rename-rekey-check` vert de 3j à 3l.
12. En tant que relecteur humain, je veux la liste des noms appliqués tels quels mais discutables, afin de les trancher au point d'arrêt d'avant l'étape 5.
13. En tant que joueur, je ne veux voir aucun pixel changer.

## Implementation Decisions

### Périmètre

`rename-slice SUBSTEP=3j` rend **390 identités** :
- par module : `message_box` 94, `score_panel` 72, `menu` 55, `score_panel_display` 39, `test_score_panel` 37, `test_help_screen` 30, `help` 19, `cart` 15, `test_message_box` 13, `test_toast` 12, `cheat` 4 ;
- par sorte : 174 fonctions, 87 locales, 58 champs, 29 statiques, 18 types, 10 macros, 8 paramètres, 5 constantes, 1 tag (`struct pico8_mh` → `message_box`).

Occurrences mesurées en répétition : `ref` 1 527, `decl` 451, `macro_arg` 326, `type_ref` 201, `macro_def` 10, `macro_cond` 7, `macro_use` 7. Le total, 2 529, est celui de la table de 2b. Il n'y a aucun refus, aucun `##` et aucun défaut de l'outil.

Restent en place les 4 mentions `identifier` de sous-étapes suivantes : `pico8_class_coverage_record` (3k), dans `help.c`, `message_box.c` ×2 et `score_panel.c`.

Pour l'étape 4 :
- 308 `backtick` et 50 `comment` dans les fichiers de 3j ;
- les messages d'`assert` qui citent le cart, dont ceux de `cart.c` (`qh and ga extend jo…`, `pico8_cart_load() must run…`, `fm must be the hy…`), de `message_box.c` (`an article ld is…`, `the cart's two mh()…`, `… and iv place`), de `score_panel.c`, `score_panel_display.c`, `menu.c`, `help.c`, et `out of hv…` de `test_message_box.c` et `test_toast.c` ;
- la mention nue `pico8_cart_load()` du commentaire du `Makefile` (ligne 250) et des commentaires de `test_research_project.c` ;
- le libellé français `chargement apres cart_load` de `test_research_project.c`.

### TSV : une correction pour le §4

**La locale `ok` de `message_box_open_box`** (`message_box.c`, ex-`pico8_pz`) a une portée de 60 lignes. Le TSV la nomme `ui` : deux lettres, hors tolérance. C'est la proposition de 3a, fermée pour `test_ui.c` par 3h. On la corrige de la même façon, avant `rename-slice` :
- nom neuf : **`dispatcher`**, le nom que 3h a donné à la locale `ok` de `test_left`, de même type (`ui_t *`, le singleton de l'interface) ;
- `main_ui`, le nom du vocabulaire, est exclu : la locale masquerait la fonction `main_ui()`, qu'elle appelle à la ligne suivante ;
- trois lignes à changer :
  - la ligne de `docs/renommage.tsv` (identité `c:message_box.c@9713@F@pico8_pz@ok`) ;
  - une ligne `field pico8_pz.ok dispatcher ok` dans `tools/rename_overrides.tsv`, à côté de `field test_left.ok` ;
  - la ligne `identity c:message_box.c@9838@F@pico8_pz@ok` des surcharges, qui passe de `ui` à `dispatcher`, sinon elle contredirait la ligne `field`.

Mesuré en répétition : 8 occurrences, aucun conflit, `scope-count SUBSTEP=3j` à 0.

Contrôle des autres noms neufs de la tranche : aucun refus, aucune collision, aucun autre nom de deux lettres hors tolérance, aucun digramme du cart (§1). Le champ `final_score` de `score_panel_display` garde son nom : la collision avec `economy_score_final` a été levée en 3g.

Les **18 fonctions exportées** sans le préfixe de leur fichier (relevé de 3c) sont gardées telles quelles :
- méthodes de leur type receveur (§4) :
  - `toast_init`, `toast_new`, `toast_construct`, `toast_ready`, `toast_update`, `toast_draw`, `toast_shutdown` (`toast_t`, `message_box.h`) ;
  - `score_panel_update`, `score_panel_update_method`, `score_panel_new_year`, `score_panel_new_year_method`, `score_panel_toggle_on_click` (`score_panel_t`, définies dans `score_panel_display.c`, comme les `planet_*` de `planet_display` en 3e) ;
  - `pause_menu_open_box`, `pause_menu_open_box_class` (`pause_menu_open_box_t`) ;
- méthodes de la table du menu galaxie (`g_galaxy_menu_*`, précédent des tables de 3d) : `galaxy_menu_button`, `galaxy_menu_on_click` ;
- singletons du vocabulaire, comme `main_ui` en 3h : `shown_message_box` (ex-`pico8_kc`), `main_score_panel` (ex-`pico8_ju`).

**Pour décision humaine** (appliqués tels quels, le §4 ne les interdit pas) :
- **Un suffixe `_t` sur des variables**, que le §4 réserve aux types : `g_score_panel_new_year_keep_open_t` (compte à rebours en ticks, `score_panel_display.c`) et `g_fake_update_last_t` (dernier `t` reçu, `test_toast.c`).
- **Des noms qui bégaient** : `score_panel_set_main_score_panel`, `score_panel_score_rows`, `score_rows_row_t`.
- **Une faute** : `test_update_entities_drives_a_entity_struct`.
- **Une macro locale sans le nom de son module** : `DIFFICULTY_OPTIONS_ROW_CAP` (`help.c`), à côté de `HELP_SCREEN_ROWS_CAP`.
- **Le préfixe `g_` des `static const`** : `g_help_screen_class`, `g_message_box_class`, `g_toast_class`, `g_score_panel_class`, `g_score_panel_layout_block_class`, `g_pause_menu_open_box_class`. C'est la question transverse de 3i, à trancher pour tous les modules à la fois.

### Gardes à motifs

L'inventaire de 3a a deux éléments en 3j, tous deux dans `tools/composition_residue_sites.txt`, et ce ne sont que des noms cités (sites `chaîne`, plus bas) :
- `pico8_eg_gg_pred`, première colonne de la ligne 10, que `--detail` imprime ;
- `pico8_fv_kj`, quatrième colonne de la ligne 6.

Mesuré en répétition :
- aucun motif ni aucune exemption de `composition_residue.sh` ou de `data_residue.sh` ne cite un nom de 3j ;
- `composition-guard` et `data-guard` restent verts après la seule application ;
- `--detail` des deux scripts identiques avant et après l'application ;
- à la fin, `composition_residue.sh --detail` ne diffère que par la ligne `void-ptr-r2 pico8_eg_gg_pred` → `void-ptr-r2 refresh_palette_is_colonized_pred` (même cas que la ligne 11 en 3e) ; `data_residue.sh --detail` identique ;
- `--champs` : 260 champs, « 0 écriture » 15 inchangé, « 0 hors constructeur » 30 → 27, par cumul sous des noms de membre neufs partagés (majorant, pas une porte).

Il n'y a aucune traduction ni aucun test de vivacité.

### Chaînes

`rename-strings-verify SUBSTEP=3j` compte 18 sites. Quatre passent dès l'application, parce que le nom neuf y est déclaré : `pico8_cart_load` (`cart.c`), `pico8_fv_kj` (`help.c`), `at1_buf` (`message_box.h`), `pico8_eg_gg_pred` (`score_panel.c`). Restent 14 sites :
- **4 noms de test**, dans les `printf("…: all tests passed")` : `test_pico8_fv`, `_mh`, `_pj`, `_iv` → `test_help_screen`, `test_message_box`, `test_score_panel`, `test_toast` ;
- **6 clés de `test_score_panel.c`** :
  - repliées par `fnv_str` : `"lh"` → `"keep_open"`, `"hi"` → `"fixed_pos"`, `"gq"` → `"row_step"`, `"ca"` → `"back_color"`, `"ep"` → `"description"` ;
  - le libellé `check("cr", …)` → `"tech_tree_screen"` ;
- **2 symboles cités dans un littéral** :
  - `at1_buf` → `text_buf` dans le message d'`assert` du toast (`message_box.c`, site lecteur) : seul ce nom change, le reste du message va à l'étape 4 ;
  - `pico8_cart_load` → `cart_load` dans le `printf` de `test_research_project.c` (la mention `later string` laissée par 3g) ;
- **2 noms dans `tools/composition_residue_sites.txt`** : `pico8_eg_gg_pred` → `refresh_palette_is_colonized_pred` (ligne 10, première et quatrième colonnes), `pico8_fv_kj` → `help_screen_draw` (ligne 6, quatrième colonne).

Après l'édition, la répétition mesure 0 site en défaut sur 18 et **0 `unseen`**.

**Littéraux composés de 3j**, écrits et lus dans ses fichiers (règle de 3f, précédents de 3g et 3i) :
- **clés courtes du cart hors TSV**, repliées par le pli de la rangée de `test_score_panel.c` et `test_help_screen.c` :
  - `fnv_str(h, "c")` → `"color"` ×2 ;
  - `fnv_str(h, "fn")` → `"draw_function"` ×2 (même renommage qu'en 3g) ;
- **libellé imprimé sans lecteur** : `"scores box follows c_"` → `"scores box follows is_shown"` (`test_message_box.c`, champ `c_` → `is_shown`).

La recherche des anciens noms de 3a à 3j dans les littéraux des fichiers de 3j (hors commentaires, littéraux adjacents recollés) ne laisse ensuite que :
- les messages d'`assert`, multi-lignes compris (étape 4) ;
- le code de ressource conservé `"k"`, le texte de toast `"x"`, l'étiquette `"probe"` ;
- les chemins `pico8/gfx.json`, `pico8/map.json` ;
- les formats `printf`.

**Tampons** : aucun ne reçoit un nom allongé. Les clés vont à `fnv_str`, les noms de test et libellés à `printf`, `check` et `run_check` en `%s`. Le message d'`assert` n'est jamais copié.

**Hachages** (prévision mesurée en répétition) :
- l'application seule ne fait rougir **aucun** test ;
- les clés font rougir **trois** constantes :
  - `test_score_panel` `BUILT_HASH` `0x9BE763EBu` → **`0x5EC1E758u`** ;
  - `test_score_panel` `SHARED_HASH` `0xAC16627Bu` → **`0x387783BAu`** ;
  - `test_help_screen` `BUILT_HASH` `0xFAA9865Eu` → **`0x088E67AEu`**.

Ne bougent pas : `LIFE_HASH` et `TECH_TREE_SCREEN_HASH` (ex-`CR_HASH`, renommé par l'application) de `test_score_panel`, `LIFE_HASH` et `HOVER_HASH` de `test_help_screen`, les quatre constantes de `test_message_box` et les quatre de `test_toast`. Les tests s'exécutent en enfants forkés et s'arrêtent au premier rouge : on reporte une constante à la fois, `BUILT_HASH` avant `SHARED_HASH`. Tout autre rouge de hachage, ou une autre valeur finale, arrête la tranche (`help-me-debug`).

### Application et recâblage

- Ordre : celui de la note de 3a, sur `SUBSTEP=3j`, la correction du TSV d'abord.
- Recâblage mesuré :
  - **1 identité** après l'application : `remplissage_path` de `cmd/replay.c` (3l), décalée par les appels renommés ;
  - **0** après les chaînes et les composés, édités ensemble avant une seule extraction.
- `rename-rekey-check` vert de 3j à 3l (mesuré).

### Garde de portée

`scope-count SUBSTEP=3j` vaut **81** au départ : 42 portées > 40, 38 champs, 1 statique.

Après la répétition, il vaut **0**, grâce à `dispatcher`. Aucune exception n'est proposée.

### Fin de PRD

Les relevés du §5 et des notes de 3a à 3i, sur `SUBSTEP=3j` :
- `make test-all` vert, `tests/e2e/*.sha256` inchangés ;
- `make raylib` et `make screencast-viewer` compilent et linkent, sans jamais être lancés ;
- `rename-tool-test` (37 tests) et les `rename-*-test` verts ;
- `class-coverage-net` et `e2e-gap` rouges à l'identique de la fin de 3i (mesuré : sorties égales à l'octet) ;
- `data-residue` et `composition-residue` à 0 ;
- après un `rename-extract` frais, sans `warning:` :
  - `cart-names-count SUBSTEP=3j` : `done` 0 en `identifier` et en `string`, `later identifier` 4 ≤ 4, `later string` 0 ;
  - `rename-strings-verify SUBSTEP=3j` : 0 site en défaut sur 18, 0 `unseen` ;
  - `rename-rekey-check` vert de 3j à 3l ;
  - `scope-count SUBSTEP=3j` à 0.

## Implementation Chunks

1. **Sous-étape 3j entière, note comprise.**
   - Un commit de code qui laisse `make test-all` vert. Il contient :
     - la correction `ok` → `dispatcher` (TSV et deux lignes de surcharges) ;
     - l'application ;
     - les 14 sites de chaînes ;
     - les clés `"color"`/`"draw_function"` et le libellé `is_shown`, avec les trois constantes reportées après avoir rougi ;
     - le TSV recâblé.
   - La note « Découvert » sous l'entrée 3j, dans le même commit ou dans un commit de docs qui suit. Elle relève :
     - les identités et les occurrences ;
     - la correction du TSV ;
     - l'absence de garde touchée (`--detail` à la ligne 10 près, `--champs`) ;
     - les 18 fonctions gardées ;
     - les sites de chaînes et les littéraux composés ;
     - les hachages changés ;
     - l'absence de tampon touché ;
     - les recâblages ;
     - ce qui va à la décision humaine : suffixe `_t` des variables, noms qui bégaient, `DIFFICULTY_OPTIONS_ROW_CAP`, `g_` des `static const` ;
     - les relevés de fin.
   - Le plan est corrigé dans le même commit si un constat le contredit.

## Testing Decisions

- **Code de 3j.** Le filet est le compilateur, `make test` et les e2e à l'octet. Aucun test neuf.
- **Correction du TSV.** La preuve est `rename-apply` sans conflit et `scope-count SUBSTEP=3j` à 0.
- **Hachages.** On laisse chaque constante rougir d'abord avec les clés renommées, puis on reporte la valeur affichée. Les valeurs finales doivent être celles prévues ci-dessus : une autre valeur, ou un autre test rouge, est un signal d'arrêt, pas une valeur à reporter.
- **Noms de test et libellés.** Ce ne sont que des libellés, sans lecteur. `rename-strings-verify` en est la preuve.
- **Gardes.** Pas de test neuf, aucun motif n'est touché. `--detail` avant et après sert de preuve.
- **Outils de méthode.** Inchangés : le schéma du TSV et ses lecteurs restent figés.

## Out of Scope

- `pico8_class_coverage_record` dans les fichiers de 3j (le résidu de 4) : 3k.
- Commentaires, messages d'`assert` (sauf le nom `at1_buf`, site du TSV), mentions entre backticks ou nues, `Makefile` compris, et mots français des littéraux (`chargement apres`) : étape 4.
- Renommer les noms signalés pour décision humaine.
- Renommer par goût un nom conforme au §4.
- Lancer l'appli raylib.

## Further Notes

- Mesures de cette PRD, sur l'arbre `b13e450` avec un `rename-extract` frais, sans `warning:` :
  - `cart-names-count SUBSTEP=3j` : `done` 2 105 `identifier`, 308 `backtick`, 11 `string`, 50 `comment` ; `later` 4 `identifier` (borne 4), 14 `backtick`, 0 `string`, 0 `comment` ;
  - `rename-strings-verify SUBSTEP=3j` : 18 sites, 18 en défaut, 0 `unseen` ;
  - `scope-count SUBSTEP=3j` : 81 ;
  - `rename-rekey-check SUBSTEP=3j` : 0 introuvable.

  `tmp/ast_rename/slice.tsv` et `cart_names_count.txt` ont été rendus à leur état d'avant la mesure.
- Répétition à blanc dans un worktree jetable, détruit depuis. Elle a fait la correction du TSV, l'application, les chaînes, les composés, les hachages et les recâblages, puis `make test`, `make e2e`, `raylib`, `screencast-viewer`, `rename-tool-test`, `class-coverage-net`, `e2e-gap`, une extraction fraîche et les compteurs. Tous les chiffres ci-dessus en viennent : 62 fichiers touchés, TSV, surcharges et `composition_residue_sites.txt` compris.
- `rename-table-check` reste rouge par construction : la porte est `rename-rekey-check`.
- Durées : chaque `rename-extract` ou `rename-apply` prend environ 1 min 30, et la tranche en enchaîne quatre.
