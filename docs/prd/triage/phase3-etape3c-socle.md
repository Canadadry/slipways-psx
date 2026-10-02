---
title: "Plan 5, sous-étape 3c — socle"
description: "Le socle objet et mémoire (engine/, hors en-tête de 3a) et ses tests passent aux noms anglais du TSV, avec la garde registres, ses exemptions et l'exception de portée de vec_t traduites et prouvées vivantes."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 3c. **Socle.** `engine/`, sauf l'en-tête de 3a : `allocator`, `zone`, `zone_sizes`, `frame_zone`, `game_zone`, `pointer_list`, `entity_list`, `exit_queue`, `collect`, `vec`, `geom`, `entity`, `entity_registry`, `entity_geom`, `dispatch_context`, `bootstrap`. Tests : `heap_allocator.[ch]`, `test_allocator`, `test_arena`, `test_zone`, `test_frame_zone`, `test_game_zone`, `test_pointer_list`, `test_entity_list`, `test_exit_queue`, `test_vec`, `test_geom`, `test_draw_layers_camera` ; `tools/scope_exceptions.txt` suit (`pico8_vec_t`). Résidu ≤ 314.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4), troisième étape. Entrées : `docs/plan-5.md` §4, §5, entrée 3 avec « Sous-étapes » et « Méthode des sous-étapes », et les notes « Découvert » de **3a et 3b**, qui fixent la méthode : on y renvoie sans la recopier (ordre des commandes, règle d'appartenance des chaînes, angles morts des littéraux composés **et des littéraux adjacents recollés**, prévision des hachages, recâblage après chaque édition de texte, porte `rename-rekey-check` et non `rename-table-check`, inventaire des gardes à motifs). Prior art : `c401114` (3b, une seule tranche : application, gardes, chaînes, outil réparé, TSV recâblé, note). Consigne `guideline` chargée par l'agent rédacteur lui-même (aucun humain) aux points « vérifier avec l'utilisateur » de `to-prd` : modules, tests et découpage tranchés sur le texte du plan, sans interview.

**Condition d'entrée** : `origin/plan-5-noms-anglais` = HEAD de départ (`80a8a5c` ou descendant), arbre propre, `make test-all` vert ; `make rename-extract` frais sans ligne `warning:`, puis `make rename-rekey-check SUBSTEP=3c` vert.

## Problem Statement

Après 3b, le socle objet et mémoire porte encore les noms du cart et du français : `pico8_ptrs_t`, `pico8_ents_t`, `pico8_zone_choisie_t`, `pico8_bascule_zone_t`, `arene_t`, `pico8_cg`, `pico8_dc`, `pico8_lt`, `pico8_he`, `pico8_qz_box_t` et ses champs `xl`/`yt`/`xr`/`yb`, la zone de test `zone_tas`… Ce sont 442 identités, **5 778 occurrences AST dans 163 fichiers** : l'essentiel du jeu manipule des listes, des zones et des registres.

Le compilateur arbitre l'application. Ce qui rend 3c délicate, ce sont les endroits où ces noms vivent en texte, sans filet :

- **La garde `registres`** de `tools/composition_residue.sh` (dans `make test`) reconnaît les registres par le jeton `REG_TYPES=pico8_ptrs_t` et par une regex d'alias `typedef pico8_ptrs_t X;`. Renommé `pointer_list_t` sans traduction, `registres` et `ptrs-entité` ne trouvent plus rien et passent **à vide**, en vert. `REG_TYPES` n'est cité par aucune ligne `chaîne` : ni `rename-strings-verify` ni `cart-names-count SUBSTEP=3c` (fichier de 3l) ne le signalent.
- **Ses exemptions** `REGISTRES_NON_ENTITE` nomment quatre fonctions de `engine/collect` (`pico8_nv_ptrs`, `pico8_gu_content_ptrs`, `pico8_jp_ptrs`, `pico8_la_ptrs`), et `composition_residue_sites.txt` trois fonctions du socle en première colonne (`pico8_bm_has`, `pico8_dc`, `pico8_ke_ks`) et six en quatrième (`pico8_he`, `pico8_lt`, `pico8_ke_struct_pq`, `pico8_jp_ents`, `pico8_zone_frame`, `pico8_cg`).
- **`tests/test_composition_residue.c`** (module de 3l) plante ses violations en **littéraux adjacents recollés** — l'angle mort de 3b : 96 `"pico8_" "ptrs_t"`, plus `"pico8_" "jp_ptrs"` ×3, `"pico8_" "hv"` ×2, `"pico8_" "ptrs_add"` ×2, `"pico8_" "ptrs_iter_t"` ×1. Ni la recherche par littéral ni `rename-strings-verify` ne les voient ; ce sont pourtant les tests de vivacité de `registres` et `ptrs-entité`.
- **`tools/scope_exceptions.txt`** autorise `x` et `y` de `pico8_vec_t` ; non traduit, `scope-count` se met à refuser les deux champs de `vec_t`. Pas de ligne `chaîne` (note de 3a) : traduction à la main.
- **58 sites de chaînes** (43 noms : 11 noms de test, 5 chemins `tmp/`, 12 clés de cas, 15 symboles cités), dont 13 dans `tools/` et 2 dans `tests/test_composition_residue.c`.

## Solution

Une seule tranche, dans l'ordre de commandes de la note de 3a, sur `SUBSTEP=3c` : application des 442 identités, traductions de garde (`REG_TYPES` et sa regex d'alias, `REGISTRES_NON_ENTITE` pour ses quatre noms de 3c, première colonne des sites, fixtures de `test_composition_residue.c`), `tools/scope_exceptions.txt`, recâblage, puis les 58 sites de chaînes, recâblage à nouveau, relevés et note « Découvert ». À la fin : aucun nom de 3c en `identifier` ni en `string` dans le dépôt ; les fichiers de 3c ne gardent que les 314 mentions de sous-étapes suivantes ; `composition-residue` et `data-residue` aux mêmes comptes (0, 0), `--detail` identique aux trois noms traduits de la première colonne près ; `scope-count SUBSTEP=3c` passe de 14 à 3 ; pas un pixel ne change.

## User Stories

1. En tant qu'agent de 3c, je veux partir d'un TSV dont chaque identité de 3c se résout (`rename-rekey-check SUBSTEP=3c` vert sur extraction fraîche), afin que `rename-apply` ne refuse aucune entrée.
2. En tant qu'agent de 3c, je veux appliquer les 442 identités de `rename-slice SUBSTEP=3c` en une commande, afin que le compilateur arbitre tout oubli dans les 163 fichiers touchés.
3. En tant que lecteur du code, je veux que les types du socle s'appellent `pointer_list_t`, `entity_list_t`, `allocator_t`/`struct allocator`, `flip_zone_t`, `bump_zone_t`, `slot_zone_t`, `arena_t`, `vec_t`, `box_t`, `side_of_line_t`, afin que le §4 se lise dans tout le code qui les manipule.
4. En tant que lecteur du code, je veux que les champs à deux lettres du socle (`box_t.left/top/right/bottom`, `zone_cell_t.align_integer/align_pointer`) et les locales longues de `draw_layers` portent un nom complet, afin que `scope-count` ne refuse plus dans 3c que les trois statiques locales déjà proposées en exception par 3a.
5. En tant que mainteneur, je veux que `REG_TYPES` et la regex d'alias de `composition_residue.sh` nomment `pointer_list_t`, afin que `registres` et `ptrs-entité` continuent de voir les registres.
6. En tant que mainteneur, je veux que les violations plantées de `test_composition_residue.c` soient réécrites avec `pointer_list_t` (en littéraux recollés, comme 3a l'a fait pour `entity_t`), afin qu'un `REG_TYPES` resté à l'ancien nom fasse rougir le test.
7. En tant que mainteneur, je veux que les quatre exemptions de `REGISTRES_NON_ENTITE` qui visent `engine/collect` soient traduites, prouvées vivantes par le vrai arbre (non traduites, `registres` s'allume sur `collect.h`) et par la fixture d'exemption de `test_composition_residue.c`, afin qu'aucune exemption ne meure en silence.
8. En tant que mainteneur, je veux `composition_residue.sh --detail` et `data_residue.sh --detail` identiques avant et après, aux seuls trois noms traduits de la première colonne de `composition_residue_sites.txt` près, afin de prouver qu'aucune garde ne change de verdict.
9. En tant que mainteneur, je veux que `tools/scope_exceptions.txt` nomme `vec_t`, et voir `scope-count` refuser `x`/`y` tant qu'il ne l'est pas, afin que l'exception soit prouvée vivante.
10. En tant qu'agent de 3c, je veux renommer à tous leurs sites les 43 noms portés par une chaîne de 3c (58 sites), afin que `rename-strings-verify SUBSTEP=3c` soit vert, 0 `unseen`.
11. En tant qu'agent de 3c, je veux chercher mes noms aussi dans les littéraux composés et les littéraux adjacents recollés de tout le dépôt, afin de ne pas reproduire les angles morts de 3a et 3b.
12. En tant qu'agent de 3c, je veux une liste prévue des tests à hachage figé touchés (attendue vide), afin qu'un rouge imprévu arrête la tranche au lieu d'être recopié.
13. En tant que mainteneur, je veux que les deux branches de `ZONE_ASAN` (build normal et build ASan) compilent et passent après le renommage des macros de zone, afin que les tests ASan (`test_zone`, `test_allocator`, `test_arena`, `test_trade`…) restent des preuves.
14. En tant qu'agent de 3d, je veux trouver un TSV recâblé après l'application et après les chaînes, `rename-rekey-check` vert de 3c à 3l, afin de partir sur une table saine.
15. En tant que relecteur humain, je veux une note « Découvert » sous 3c avec les relevés de fin, les écarts et les questions de nommage rencontrées, afin de trancher au point d'arrêt d'avant l'étape 5.
16. En tant que joueur, je ne veux voir aucun pixel changer.

## Implementation Decisions

### Périmètre et appartenance

- Modules : la ligne 3c de `tools/rename_substeps.tsv` (16 modules d'`engine/`, `tests/heap_allocator`, 11 tests, `tools/scope_exceptions`). `rename-slice SUBSTEP=3c` rend **442 identités** : `engine/zone` 36, `entity_list` 35, `entity_registry` 34, `allocator` 31, `pointer_list` 30, `game_zone` 21, `geom` 20, `exit_queue` 19, `vec` 14, `entity_geom` 11, `zone_sizes` 10, `collect` 9, `entity` 8, `dispatch_context` 7, `bootstrap` 7, `frame_zone` 3 ; tests : `test_game_zone` 31, `test_allocator` 26, `test_zone` 20, `test_entity_list` 13, `test_exit_queue` 12, `test_vec` 10, `test_pointer_list` 10, `test_geom` 9, `test_draw_layers_camera` 8, `heap_allocator` 5, `test_frame_zone` 3. Le TSV dit les noms neufs : on y renvoie.
- Identités par sorte : 218 fonctions, 62 locales, 50 paramètres, 36 macros, 30 statiques, 25 types, 16 champs, 3 constantes d'enum, 1 globale, 1 tag. Aucune identité restante (3d–3l) ne contient dans son USR une fonction ou un type renommé par 3c (les 19 statiques homonymes de `tests/test_live_slots.c`, `octets_des_registres`…, ont leur propre USR de fichier).
- Laissés : les 314 mentions `identifier` de sous-étapes suivantes dans les fichiers de 3c (`test_frame_zone` 94, `test_exit_queue` 65, `test_vec` 59, `test_game_zone` 57, `bootstrap` 17, `test_draw_layers_camera` 10, `test_geom` 7, `entity_registry` 3, `exit_queue` 1, `zone` 1) et 1 `string` (`"ld"` lu par `test_frame_zone`, écrit ailleurs) ; `pico8_jo_ex` (3e) et les autres noms de `REGISTRES_NON_ENTITE`/`PTRS_NON_ENTITE` (3e, 3f, 3g) ; les lignes 4 à 11 de `composition_residue_sites.txt` (3e, 3f, 3j).
- Hors 3c, étape 4 : 41 `backtick` et 69 `comment` des fichiers de 3c, les commentaires de `composition_residue.sh` qui citent `pico8_ptrs_t`, `pico8_jp_ents`, `pico8_ents_cat`, et les mots français des chaînes (`"zone essai-partie pleine"`, noms de cas `bascule_twice_without_rendre`, `chargement_full`, `ptrs_reserved_overflow`), hors des lignes `chaîne` du TSV.

### Application et recâblage

- Ordre des commandes : celui de la note de 3a, `SUBSTEP=3c`. Prévision du recâblage après l'application, par la méthode de 3b : **≈ 866** identités à offset de sous-étapes suivantes situées après un token réécrit (3b : prévu 851, constaté 856). Un second recâblage suit les chaînes (noms de test, chemins et clés de cas allongés ou raccourcis).
- Macros de zone : `PICO8_ZONE_ASAN` → `ZONE_ASAN` (`multidef`, deux `#define` sous `__SANITIZE_ADDRESS__`/`__has_feature`) et `PICO8_BUMP_ZONE_HALVES` → `BUMP_ZONE_HALVES` (`multidef`, `#ifdef`/`#else`) sont renommées ; `POISON` et `UNPOISON` (`multidef`, `engine/zone.c`) sont `conservé` et ne bougent pas. L'extraction a une configuration `asan` : les deux branches sont des occurrences connues, l'apply ne doit rien refuser. Un refus est un défaut de l'outil, corrigé avec son test dans le même commit, comme en 3a et 3b. Aucun `##` dans le socle.
- `tests/heap_allocator.[ch]` : `zone_tas` → `heap_allocator`, `tas_alloc`/`tas_grow`/`tas_release` → `heap_*`, garde `ZONE_TAS_H` → `TESTS_HEAP_ALLOCATOR_H` ; l'en-tête est inclus par 19 tests de toutes les sous-étapes, réécrits par l'apply.
- Types à tag : `struct pico8_zone_choisie` (identité `c:@S@`) → `allocator`, distincte de son typedef : deux lignes, déjà dans la tranche.

### Gardes à motifs (inventaire de 3a, éléments de 3c)

- **`REG_TYPES` et alias** (`registres`) : `pico8_ptrs_t` → `pointer_list_t` dans l'affectation et dans la regex `typedef … pointer_list_t X;`. Aucun alias n'existe dans `src/` aujourd'hui. Vivacité : les violations plantées de `test_composition_residue.c` (`registres`, `ptrs-entité`, formes cachées, accolades scindées, qualifiants) sont réécrites avec `"pointer_" "list_t"` (recollage gardé, pour que le fichier n'entre pas dans le résidu qu'il mesure) ; preuve rouge : remettre l'ancien `REG_TYPES` fait échouer `test_composition_residue`.
- **`REGISTRES_NON_ENTITE`**, ses quatre noms de `engine/collect` → `collect_map_pointer_list`, `collect_contains_string`, `collect_filter_pointer_list`, `collect_concat_pointer_list` ; les trois autres noms (3e, 3f) inchangés, rien de retiré. Vivacité : sur le vrai arbre, `REG_TYPES` traduit sans les exemptions traduites fait monter `registres` (prototypes de `collect.h`) — à constater une fois, puis rétablir ; dans la fixture, `"pico8_" "jp_ptrs"` → nom neuf recollé, `"pico8_" "hv"` → `live_entities`, `"pico8_" "ptrs_add"`/`"ptrs_iter_t"` → `pointer_list_add`/`pointer_list_iter_t`.
- **`composition_residue_sites.txt`** : colonne 1 des lignes 1–3 (`is_message_name`, `draw_layers`, `entity_set_state`) et symboles de 3c cités en colonne 4 des lignes 1 et 10, par `rename-strings-verify`. La colonne 1 ne reconnaît rien ; `--detail` change de ces trois noms, et de rien d'autre.
- **`tools/scope_exceptions.txt`** : `pico8_vec_t` → `vec_t`. Vivacité : sur l'extraction fraîche d'après l'apply, avant la traduction, `scope-count SUBSTEP=3c` compte 5 (`x`, `y` de `vec_t` en plus des 3) ; après, 3.
- `data_residue.sh` : aucun élément de 3c. Le témoin `"pico8_" "ptrs_t p;"` de `test_data_residue.c` (bornes de mot du motif historique `liste`) n'est pas une citation du code : laissé, noté, avec les motifs historiques déjà soumis à décision humaine.
- `alloc_guard`, `line_coverage_reference.txt` : ne citent aucun nom de 3c (chemins et numéros de ligne seulement ; l'apply ne déplace aucune ligne).

### Chaînes

- Les 58 sites de `rename-strings-verify SUBSTEP=3c` (43 noms ; 0 `unseen` mesuré) : noms de test `test_pico8_{zone,zone_choisie,zone_arene,zone_frame,zone_partie,ptrs,ents,file_sortie,vec,geom,planet_kj_dispatch}` ; chemins `tmp/test_pico8_{zone,zone_choisie,zone_frame,zone_partie,file_sortie}_stderr.txt` ; clés de cas de `test_allocator`, `test_exit_queue` (6), `test_frame_zone` (2), `test_game_zone` (3, dont `double_eu_without_flush`, passé en `argv` à l'enfant et attendu par `expect_assert`) ; symboles cités dans `engine/` (déjà réécrits par l'apply, sauf texte), `tests/test_composition_residue.c` (`g_hv`, `pico8_ptrs_t`), `tools/composition_residue.sh` (4) et `tools/composition_residue_sites.txt` (9).
- Littéraux composés et adjacents : mesurés sur tout le dépôt pour les noms distinctifs de 3c ; seuls hors des sites listés, les recollés de `test_composition_residue.c` ci-dessus et le témoin de `test_data_residue.c`. À refaire au moment de l'édition (`grep '"[^"]*\b<nom>\b'` et `"pico8_" "<suffixe>`).
- Hachages : liste prévue **vide** — aucun des onze tests de 3c n'appelle `fnv`, et aucun site de chaîne de 3c n'est dans un test haché. Aucun `__func__` dans `src/` ni `tests/` : les messages attendus par `expect_fatal`/`expect_assert` ne contiennent pas de nom de fonction. Tout rouge imprévu arrête la tranche (`help-me-debug`).

### Fin de PRD

Liste du §5 et des notes de 3a et 3b, sur `SUBSTEP=3c` : `make test-all` vert, `tests/e2e/*.sha256` inchangés ; `make raylib` et `make screencast-viewer` compilent et linkent (jamais lancés) ; `rename-tool-test` et les `rename-*-test` verts ; `class-coverage-net` et `e2e-gap` rouges à l'identique de la fin de 3b ; `data-residue` et `composition-residue` à 0 ; après `rename-extract` frais sans `warning:`, `cart-names-count SUBSTEP=3c` `done` 0 en `identifier` et `string`, `later identifier` ≤ 314 ; `rename-strings-verify SUBSTEP=3c` vert, 0 `unseen` ; `rename-rekey-check` vert de 3c à 3l ; `scope-count SUBSTEP=3c` à 3 (`kw` de `test_frame_zone`, `CV`/`CG` de `test_game_zone`, statiques locales `conservé` déjà proposées en exception par 3a) ; aucun ancien nom distinctif de 3c dans `tools/` ni le `Makefile` hors commentaires et outils du dictionnaire.

## Implementation Chunks

1. **Sous-étape 3c entière, note comprise.** Une seule tranche : `REG_TYPES` lie l'application (`pointer_list_t`) aux fixtures de `test_composition_residue.c` et aux exemptions — séparés, la garde passe à vide ou `test_composition_residue` rougit. Un commit de code (application, traductions de garde, `scope_exceptions.txt`, chaînes, TSV recâblé deux fois) qui laisse `make test-all` vert, puis la note « Découvert » sous l'entrée 3c dans le même commit ou un commit de docs qui suit : identités et occurrences appliquées, recâblages (prévu ≈ 866 / constaté), sort des macros `multidef` de zone, gardes traduites et leurs preuves de vivacité, `--detail` comparé, littéraux recollés trouvés, hachages (aucun), relevés de fin, exceptions de portée et questions de nommage proposées. Plan corrigé dans le même commit si un constat le contredit.

## Testing Decisions

- Code du socle : aucun test nouveau. Filet = compilateur (builds normal et ASan), `make test`, e2e à l'octet.
- `tests/test_composition_residue.c` est le test de vivacité de `registres`, `ptrs-entité` et de l'exemption par nom : ses cas existants passent avec `pointer_list_t` et les noms neufs de `collect` ; aucun cas n'est retiré. Preuve rouge consignée dans la note (ancien `REG_TYPES` → rouge).
- `scope-count` : preuve de vivacité de l'exception par le compte 5 → 3, sans nouveau test (`scope-count-test` couvre déjà la lecture des exceptions).
- Outils de méthode : inchangés ; un défaut est corrigé avec son test dans le même commit.

## Out of Scope

- Les noms de 3d–3l rencontrés dans les fichiers de 3c (résidu), et les exemptions de 3e–3g.
- Commentaires, messages d'`assert`, mentions entre backticks ou nues, mots français des chaînes hors TSV : étape 4.
- Trancher les motifs génériques et historiques, les exemptions mortes, les exceptions de portée : point d'arrêt d'avant l'étape 5.
- Corriger un nom validé par goût : les macros de taille mêlent deux ordres (`LOAD_ZONE_BYTES`, `FRAME_ZONE_BYTES`, `UI_ZONE_BYTES` contre `ZONE_GAME_BYTES`, `ZONE_GENERATION_BYTES`, `ZONE_PARTICLES_BYTES`) — appliqué tel quel, signalé dans la note pour décision humaine.
- Lancer l'appli raylib.

## Further Notes

- Mesures de cette PRD (arbre `80a8a5c`, `rename-extract` frais) : `cart-names-count SUBSTEP=3c` `done` 2 638 `identifier`, 41 `backtick`, 48 `string`, 69 `comment` ; `later` 314 `identifier` (borne 314), 92 `backtick`, 1 `string`, 13 `comment` ; `rename-strings-verify SUBSTEP=3c` 58 sites, 0 `unseen` ; `scope-count SUBSTEP=3c` 14 (champs `xl`, `yt`, `xr`, `yb`, `u`, `p` ; locales longues `nt`, `kj`, `i`, `o` de `pico8_dc`, `mo` de `test_draw_layers_camera` ; statiques `kw`, `CV`, `CG`) ; occurrences par origine : `ref` 2 792, `type_ref` 1 742, `macro_arg` 553, `decl` 521, `macro_use` 100, `macro_def` 38, `macro_cond` 17, `macro_body` 15.
- `rename-table-check` reste rouge par construction : la porte est `rename-rekey-check`.
- Durées : chaque `rename-extract` ou `rename-apply` ≈ 1 min 30 ; la tranche en enchaîne quatre ou cinq.
