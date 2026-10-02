---
title: "Plan 5, sous-étape 3i — structures"
description: "Les structures (game/planet_family et ses 10 fichiers de tests) passent aux noms anglais du TSV. Il n'y a aucune correction du TSV ni de garde, et une seule constante de hachage change, à cause de deux clés courtes repliées dans test_burst."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 3i. **Structures.** `game/planet_family`. Tests : `test_structure`, `test_processor`, `test_slipgate`, `test_infraspace_gate`, `test_trading_hub`, `test_synthesizer`, `test_ascension`, `test_protostar`, `test_burst`, `test_burst_exports_row`. Résidu ≤ 50.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4), neuvième étape.

Entrées : `docs/plan-5.md` §4, §5, l'entrée 3 avec « Sous-étapes » et « Méthode des sous-étapes », et les notes « Découvert » de **3a à 3h**. Cette PRD y renvoie sans les recopier :
- l'ordre des commandes (3a) ;
- l'appartenance des chaînes ;
- les littéraux composés (règle de 3f) ;
- les clés courtes hors TSV d'une liste hachée (`"i"`, `"o"`, 3g) ;
- la recherche des littéraux recollés ;
- la prévision des hachages en worktree jetable ;
- le recâblage après chaque édition de texte, avec `rename-rekey-check` pour porte.

Exemple à suivre : `9497883` (3h, une seule tranche).

Aucun humain n'était disponible. L'agent rédacteur a chargé lui-même la consigne `guideline` aux points « vérifier avec l'utilisateur » de `to-prd`. Les décisions ci-dessous reposent sur le texte du plan et sur les mesures, sans interview.

**Condition d'entrée** :
- `origin/plan-5-noms-anglais` est au HEAD de départ (`31a9600` ou un descendant), l'arbre est propre et `make test-all` est vert ;
- `make rename-extract` est frais, sans ligne `warning:` (les 4 lignes `partial` de `class_coverage` sont attendues) ;
- `make rename-rekey-check SUBSTEP=3i` est vert. Mesuré : 0 identité introuvable.

## Problem Statement

Après 3h, les six classes de structures et leurs tests gardent les noms du cart :
- les types et fonctions `pico8_qh_*`, `pico8_nl_*`, `pico8_os_*`, `pico8_hj_*`, `pico8_pm_*`, `pico8_ga_*` et `pico8_ga_keys_t` ;
- les statiques `g_kv`, `g_ga_by_shared` ;
- les champs `jo`, `pq`, `ra_copie`, `fz` ;
- dans les dix tests, les locales et assistants (`h`, `e`, `bm`, `ents_list_size`…).

La sous-étape compte **427 identités**, soit **4 678 occurrences AST dans 34 fichiers**.

C'est la sous-étape qui a le plus de mentions (9 944 au départ de 2b), et ses tests tiennent 21 constantes de hachage figées (`#define …_HASH`). Une répétition à blanc a été faite dans un worktree jetable, avec les étapes suivantes :
1. application ;
2. `make test` ;
3. gardes ;
4. chaînes et littéraux composés ;
5. `make test` et `make e2e` ;
6. extraction fraîche et compteurs.

Elle a trouvé :
- **Aucun refus, aucune correction du TSV pour le §4, aucune garde à traduire.**
- **Aucun hachage ne bouge par l'application.** Les tests de structures replient des chaînes lues à l'exécution (états, `planet_id`), déjà renommées par 3a.
- **Un seul hachage bouge**, à cause de deux clés courtes du cart hors TSV, repliées dans `test_burst.c` (`"c"`, `"w"`). Ce sont les cousines des `"i"`/`"o"` de 3g.
- **Trois libellés imprimés** citent un nom du cart.

## Solution

Une seule tranche, dans l'ordre de commandes de la note de 3a, sur `SUBSTEP=3i` :
1. appliquer les 427 identités ;
2. recâbler ;
3. éditer les 14 sites de chaînes encore en défaut, puis les littéraux composés de 3i ;
4. reporter la seule constante de hachage rouge ;
5. recâbler de nouveau, faire les relevés et écrire la note « Découvert ».

La répétition l'a établi : avec ces gestes, `make test` et `make e2e` sont verts (26 e2e conformes), et `make raylib` et `make screencast-viewer` compilent. `class-coverage-net` et `e2e-gap` sont rouges, avec des sorties égales à l'octet à celles de la fin de 3h.

À la fin :
- aucun nom de 3i ne reste en `identifier` ni en `string` ;
- les fichiers de 3i ne gardent que 50 mentions `identifier` de sous-étapes suivantes, et 0 mention `string` ;
- `scope-count SUBSTEP=3i` passe de 126 à **0** ;
- pas un pixel ne change.

## User Stories

1. En tant qu'agent de 3i, je veux appliquer les 427 identités en une commande, afin que le compilateur arbitre tout oubli dans les 34 fichiers.
2. En tant qu'agent de 3i, je veux savoir d'avance que l'application seule laisse `make test` vert, afin qu'un rouge à ce stade m'arrête au lieu d'être pris pour un hachage à reporter.
3. En tant que lecteur, je veux lire `structure_*`, `trading_hub_*`, `processor_*`, `ascension_*`, `protostar_*`, `burst_*` et `burst_keys_t`, afin de comprendre la famille des structures sans glossaire.
4. En tant que lecteur, je veux savoir que les 36 fonctions exportées sans le préfixe `planet_family_` restent conformes au §4, comme méthodes de leur classe receveuse, afin de ne pas les rouvrir.
5. En tant que mainteneur, je veux que `composition_residue.sh --detail`, `data_residue.sh --detail` et `--champs` restent identiques avant et après, afin de prouver qu'aucune garde ne change de verdict.
6. En tant que mainteneur, je veux savoir qu'aucun motif ni aucune exemption de garde ne cite un nom de 3i, afin de ne rien traduire à tort.
7. En tant qu'agent de 3i, je veux renommer les 10 noms de test et les 4 symboles cités de `composition_residue_sites.txt`, afin que `rename-strings-verify SUBSTEP=3i` soit vert, sans site `unseen`.
8. En tant qu'agent de 3i, je veux renommer les clés courtes `"c"` et `"w"` repliées par `test_burst.c`, afin qu'aucune clé du cart ne survive dans un hachage écrit et lu dans 3i.
9. En tant qu'agent de 3i, je veux connaître d'avance la seule constante qui rougit et sa nouvelle valeur, afin que tout autre rouge de hachage arrête la tranche.
10. En tant qu'agent de 3i, je veux renommer les libellés imprimés qui citent un nom du cart, afin de ne pas laisser de noms que ni le compteur ni le contrôle ne voient.
11. En tant que mainteneur, je veux que la garde de portée tombe de 126 à 0 dans 3i, afin de ne proposer aucune nouvelle exception.
12. En tant qu'agent de 3j, je veux trouver un TSV recâblé, avec `rename-rekey-check` vert de 3i à 3l.
13. En tant que relecteur humain, je veux la liste des noms appliqués tels quels mais discutables, afin de les trancher au point d'arrêt d'avant l'étape 5.
14. En tant que joueur, je ne veux voir aucun pixel changer.

## Implementation Decisions

### Périmètre

`rename-slice SUBSTEP=3i` rend **427 identités** :
- par module : `planet_family` 87, `test_structure` 44, `test_synthesizer` 40, `test_burst` 39, `test_ascension` 38, `test_slipgate` 37, `test_protostar` 36, `test_processor` 33, `test_trading_hub` 32, `test_infraspace_gate` 30, `test_burst_exports_row` 11 ;
- par sorte : 224 fonctions, 122 locales, 35 paramètres, 25 statiques, 10 champs, 9 constantes, 1 macro, 1 type.

Occurrences mesurées en répétition : `ref` 3 391, `macro_arg` 796, `decl` 484, `type_ref` 5, `macro_cond` 1, `macro_def` 1. Le total, 4 678, est celui de la table de 2b. Il n'y a aucun refus, aucun `##` et aucun défaut de l'outil.

Restent en place les 50 mentions `identifier` de sous-étapes suivantes : `test_burst` 11, `planet_family` 6, `test_ascension` 5, `test_protostar` 5, `test_structure` 5, `test_synthesizer` 5, `test_processor` 4, `test_infraspace_gate` 3, `test_slipgate` 3, `test_trading_hub` 3.

Pour l'étape 4 :
- 610 `backtick` et 22 `comment` dans les fichiers de 3i ;
- les messages d'`assert` de `planet_family.c` :
  - `qh: the only construction key is id (ie:ldown)…` ×4 ;
  - `nl:ly reads self.mi…` ;
  - `mo: the placeholder…` ;
  - `ga: a burst borrows its ra…` ;
  - `pico8_planet_family_init() must run first` ;
  - `pico8_jo_init() must run first` ;
- les messages d'`assert` des tests : `as test_pico8_fr.c does` dans `test_structure.c` ; `hv`, `lq`, `ra` dans `test_burst.c` ;
- le commentaire `(pico8_ga_struct_new)` de `planet_family.c`.

### TSV : aucune correction

Contrôle des noms neufs de la tranche : `scope-count` tombe à 0, aucun refus, aucune collision, aucun nom de deux lettres hors tolérance. Aucune ligne `identity` des surcharges ne vise 3i à un offset périmé. Il n'y a donc **rien à corriger** pour le §4.

Les **36 fonctions exportées** sans préfixe `planet_family_` (relevé de 3c) sont gardées telles quelles :
- 35 sont des méthodes de leur classe receveuse (§4), comme les `slipgate_*`, `infraspace_gate_*` et `synthesizer_*` de `planet_family_trade` en 3e :
  - `X_ready`, `X_kind`, `X_class`, `X_new` pour les six classes ;
  - `structure_init_instance`, `structure_update_settled_method`, `trading_hub_update_settled` ;
  - `ascension_draw`, `ascension_draw_method`, `ascension_release_method`, `ascension_init_instance` ;
  - `protostar_construct` ;
  - `burst_construct`, `burst_spawn_particle`, `burst_draw` ;
- la 36ᵉ est `planet_kind_named` (`pico8_jo_kv`), méthode de `planet` qui rend une sorte par son nom.

**Pour décision humaine** (appliqués tels quels, le §4 ne les interdit pas) :
- **`test_burst_export_line_counts_the_routes_in_by`.** La fin `_by` est le champ du cart `by` (la locale y devient `outgoing`). C'est un digramme du cart dans un nom neuf, contraire au §1. Ce n'est pas un écart au §4 : on ne le corrige pas ici. Proposition : `test_burst_export_line_counts_the_outgoing_routes`.
- **Des noms qui bégaient** : `entity_list_list_size` (×5) et `entity_list_pointer_list_size` (×4), assistants statiques des tests.
- **Le préfixe `g_` sur des `static const`.** 12 tables `static const` de classe et de sorte s'écrivent `g_X_class` et `g_X_kind`, alors que le §4 réserve `g_` aux statiques mutables. On suit la pratique de 3e à 3h (`g_slipgate_class`, `g_probe_class`, `g_ui_class`…). C'est un point systémique, à trancher pour tous les modules à la fois.

### Gardes à motifs

L'inventaire de 3a n'a aucun élément en 3i. Mesuré en répétition :
- aucun motif ni aucune exemption de `composition_residue.sh` ou de `data_residue.sh` ne cite un nom de 3i ;
- `composition-guard` et `data-guard` restent verts après la seule application ;
- `--detail` des deux scripts identiques avant, après l'application et à la fin ;
- `--champs` : 260 champs, « 0 écriture » 15, « 0 hors constructeur » 30, inchangés.

Il n'y a aucune traduction ni aucun test de vivacité. `composition_residue_sites.txt` change, mais seulement dans sa quatrième colonne (sites `chaîne`, plus bas), que `--detail` n'imprime pas.

### Chaînes

`rename-strings-verify SUBSTEP=3i` compte 18 sites. Les 4 symboles cités ont leur site écrivain dans `planet_family.c`, où ils sont déclarés : ils passent dès l'application. Restent 14 sites :
- **10 noms de test**, dans les `printf("…: all tests passed")` :
  - `test_pico8_qh`, `_os`, `_nr`, `_fr`, `_nl`, `_mx`, `_hj`, `_pm`, `_ga` → `test_structure`, `test_processor`, `test_slipgate`, `test_infraspace_gate`, `test_trading_hub`, `test_synthesizer`, `test_ascension`, `test_protostar`, `test_burst` ;
  - `test_pico8_ga_export_panel_by` → `test_burst_exports_row` ;
- **4 symboles cités** dans la quatrième colonne des lignes 4 à 6 de `tools/composition_residue_sites.txt` : `pico8_ga_pq` → `burst_construct`, `pico8_ga_struct_new` → `burst_create`, `pico8_hj_kj` → `ascension_draw`, `pico8_ga_kj` → `burst_draw`.

Laissés tels quels dans ce fichier : `pico8_fv_kj` (3j), `*_kj_dispatch` (motif générique), `(ga_t *)` (pas un nom du TSV) et les numéros de ligne, qu'un renommage ne décale pas. Ils relèvent de l'étape 4.

Après l'édition, la répétition mesure 0 site en défaut sur 18 et **0 `unseen`**. Les noms de classe et les clés d'étiquette de 3i (`qh`, `hj`, `ga`, `nl`, `os`, `pm`) sont déjà renommés par 3a.

**Littéraux composés de 3i**, écrits et lus dans ses fichiers (règle de 3f, précédent `"i"`/`"o"` de 3g) :
- **clés courtes repliées** par `fold_tuple` et `fold_layout_row` de `test_burst.c`. Ce sont les clés du cart de la rangée, que les champs nomment `color` et `width` :
  - `fnv_str(h, "c")` ×2 → `"color"` ;
  - `fnv_str(h, "w")` → `"width"` ;
  - leurs voisines `"resource"`, `"score"` et `"placed_items"` sont déjà anglaises (3d, 3g) ;
- **libellés imprimés sans lecteur** :
  - `test_burst.c` : `"ga places after the burst"` → `"burst places after the burst"`, `"ga place: %lu bytes at most\n"` → `"burst place: …"` ;
  - `test_processor.c` : `"[db] processor %d, bare planet %d\n"` → `"[trade_income] …"` (`db` est le nom du cart d'`economy_score_trade_income`, que le compteur ne voit pas, comme `[gd]` en 3h).

Le mot français `place` reste, pour l'étape 4.

La recherche des anciens noms de 3a à 3l dans les littéraux des fichiers de 3i (hors commentaires et messages d'`assert`, littéraux adjacents recollés) ne laisse ensuite que des mots communs (`at`, `place`, `n`, `d`, `s`, `x` des formats `printf`).

**Tampons** : aucun ne reçoit un nom allongé. Les noms de test vont dans des `printf`, les libellés dans `check`/`check_int`, qui impriment par `%s`.

**Hachages** (prévision mesurée en répétition) :
- l'application et les 14 sites de chaînes ne font rougir **aucun** test ;
- les clés `"c"` et `"w"` font rougir **une** constante : `test_burst` `LIFE_HASH` `0x7DAF9781u` → **`0xD3D468CEu`** ;
- `BUILT_HASH` (`0xA539DD6Du`) ne bouge pas.

Les 20 autres constantes des dix tests ne bougent pas. Elles replient des chaînes d'exécution, déjà renommées par 3a, et le résultat de `fnv_str` sur des données de test (`"s71"`…). Tout autre rouge de hachage arrête la tranche (`help-me-debug`).

### Application et recâblage

- Ordre : celui de la note de 3a, sur `SUBSTEP=3i`.
- Recâblage mesuré :
  - **3 identités** après l'application, toutes dans `game/cart.c` (3j), décalées par les appels renommés ;
  - **0** après les chaînes ;
  - **0** après les clés `"color"`/`"width"`.
- `rename-rekey-check` vert de 3i à 3l (mesuré).

### Garde de portée

`scope-count SUBSTEP=3i` vaut **126** au départ : 100 portées > 40, 11 statiques, 9 constantes d'enum, 6 champs.

Après la répétition, il vaut **0**. Aucune exception n'est proposée.

### Fin de PRD

Les relevés du §5 et des notes de 3a à 3h, sur `SUBSTEP=3i` :
- `make test-all` vert, `tests/e2e/*.sha256` inchangés ;
- `make raylib` et `make screencast-viewer` compilent et linkent, sans jamais être lancés ;
- `rename-tool-test` (37 tests) et les `rename-*-test` verts ;
- `class-coverage-net` et `e2e-gap` rouges à l'identique de la fin de 3h (mesuré : sorties égales à l'octet) ;
- `data-residue` et `composition-residue` à 0 ;
- après un `rename-extract` frais, sans `warning:` :
  - `cart-names-count SUBSTEP=3i` : `done` 0 en `identifier` et en `string`, `later identifier` 50 ≤ 50, `later string` 0 ;
  - `rename-strings-verify SUBSTEP=3i` : 0 site en défaut sur 18, 0 `unseen` ;
  - `rename-rekey-check` vert de 3i à 3l ;
  - `scope-count SUBSTEP=3i` à 0.

## Implementation Chunks

1. **Sous-étape 3i entière, note comprise.**
   - Un commit de code qui laisse `make test-all` vert. Il contient :
     - l'application ;
     - les 14 sites de chaînes ;
     - les clés `"color"`/`"width"`, avec `LIFE_HASH` reporté après avoir rougi ;
     - les 3 libellés imprimés ;
     - le TSV recâblé.
   - La note « Découvert » sous l'entrée 3i, dans le même commit ou dans un commit de docs qui suit. Elle relève :
     - les identités et les occurrences ;
     - l'absence de correction du TSV et de garde touchée (`--detail` et `--champs` égaux) ;
     - les 36 fonctions gardées ;
     - les sites de chaînes et les littéraux composés ;
     - le hachage changé ;
     - l'absence de tampon touché ;
     - les recâblages ;
     - ce qui va à la décision humaine : `_routes_in_by`, les noms qui bégaient, `g_` sur `static const` ;
     - les relevés de fin.
   - Le plan est corrigé dans le même commit si un constat le contredit.

## Testing Decisions

- **Code de 3i.** Le filet est le compilateur, `make test` et les e2e à l'octet. Aucun test neuf.
- **Hachage.** On laisse `LIFE_HASH` rougir d'abord avec les clés renommées, puis on reporte la valeur affichée. Celle-ci doit être `0xD3D468CEu` : une autre valeur, ou un autre test rouge, est un signal d'arrêt, pas une valeur à reporter.
- **Noms de test.** Ce ne sont que des libellés, sans lecteur. `rename-strings-verify` en est la preuve.
- **Gardes.** Pas de test neuf, aucun motif n'est touché. `--detail` avant et après sert de preuve.
- **Outils de méthode.** Inchangés : le schéma du TSV et ses lecteurs restent figés.

## Out of Scope

- Les noms de 3j–3l dans les fichiers de 3i (le résidu de 50), dont `pico8_fv_kj` dans `composition_residue_sites.txt`.
- Les clés `"c"` de `test_help_screen.c` et `test_score_panel.c` (3j).
- Commentaires, messages d'`assert`, mentions entre backticks ou nues, mots français des littéraux (`place`, `places`) : étape 4.
- Renommer `test_burst_export_line_counts_the_routes_in_by`, les noms qui bégaient ou les `g_` des `static const` : point d'arrêt d'avant l'étape 5.
- Renommer par goût un nom conforme au §4.
- Lancer l'appli raylib.

## Further Notes

- Mesures de cette PRD, sur l'arbre `31a9600` avec un `rename-extract` frais, sans `warning:` :
  - `cart-names-count SUBSTEP=3i` : `done` 4 519 `identifier`, 610 `backtick`, 10 `string`, 22 `comment` ; `later` 50 `identifier` (borne 50), 133 `backtick`, 0 `string`, 9 `comment` ;
  - `rename-strings-verify SUBSTEP=3i` : 18 sites, 18 en défaut, 0 `unseen` ;
  - `scope-count SUBSTEP=3i` : 126 ;
  - `rename-rekey-check SUBSTEP=3i` : 0 introuvable.

  `tmp/ast_rename/slice.tsv` et `cart_names_count.txt` ont été rendus à leur état d'avant la mesure.
- Répétition à blanc dans un worktree jetable, détruit depuis. Elle a fait l'application, les chaînes, les composés, le hachage et les recâblages, puis `make test`, `make e2e`, `raylib`, `screencast-viewer`, `rename-tool-test` et les `rename-*-test`, `class-coverage-net`, `e2e-gap`, une extraction fraîche et les compteurs. Tous les chiffres ci-dessus en viennent : 36 fichiers touchés, TSV compris.
- `rename-table-check` reste rouge par construction : la porte est `rename-rekey-check`.
- Durées : chaque `rename-extract` ou `rename-apply` prend environ 1 min 30, et la tranche en enchaîne quatre.
