# Registre des décisions humaines du plan 5

**Condition d'entrée de l'étape 5** (`docs/plan-5.md`, §6, entrée 5) : chaque entrée ci-dessous porte un verdict humain. Le registre est ouvert par 4a à partir des notes « Découvert » de 0 à 3l, complété par chaque sous-étape 4x, clos par 4l4 (section « Clôture (4l4) », en fin de fichier). Aucune entrée n'est appliquée avant son verdict : l'étape 3 et l'étape 4 ont appliqué tels quels les noms en cause.

## Conventions

- **Sites** : mesurés à `91f99a6` (fin de la PRD de 4a, avant tout travail de module), sur `src/`, `tests/`, `tools/` et le `Makefile`, hors outils du dictionnaire (`tools/rename_*`, `tools/ast_rename/`, `tests/test_rename_*.py`, `tests/test_ast_rename.py`, leurs fixtures), comme `make cart-names-count`. Sauf mention, un nombre de sites est le nombre d'occurrences en mot entier :

  ```sh
  grep -rowI --exclude-dir=ast_rename --exclude-dir=ast_rename_expected --exclude-dir=e2e \
      --exclude='rename_*.py' --exclude='rename_*.tsv' --exclude='test_rename_*.py' \
      --exclude='test_ast_rename.py' -- '<nom>' src tests tools Makefile | wc -l
  ```

  L'exécutant de l'étape 5 remesure avant d'appliquer : l'étape 4 réécrit les commentaires, et les mentions en commentaire font partie des sites.
- **Procédure N (renommage d'un identifiant)**, à laquelle renvoient les entrées de nommage :
  1. `make rename-extract` frais ; l'identité de chaque ligne se relit dans `tmp/ast_rename/declarations.tsv` (les identités données ici sont celles des noms à USR stable ; celles d'une locale ou d'une statique de fonction portent un offset et se relèvent alors) ;
  2. les lignes `ancien<TAB>nouveau<TAB>fichier<TAB>identité` de l'entrée dans `tmp/ast_rename/decision.tsv`, sous l'en-tête `ancien	nouveau	fichier	identité`, puis `make rename-apply TABLE=tmp/ast_rename/decision.tsv` ;
  3. dans le même commit, la ligne de `docs/renommage.tsv` passe au nom neuf et une ligne `name` (ou `field parent.nom`) de `tools/rename_overrides.tsv` le fixe : le TSV reste ce qui a été appliqué (§6) ;
  4. les mentions entre backticks, en commentaire et dans les chaînes sont réécrites à la main dans le même commit.
- **Portée d'une décision de nommage** : elle s'applique aussi aux mentions entre backticks et en commentaire. Contrôle : le `grep` des sites ci-dessus sur l'ancien nom rend 0, `make test-all` vert, `tests/e2e/*.sha256` inchangés.
- **Procédure G (garde à motifs)** : le motif ou l'exemption édité dans `tools/composition_residue.sh` ou `tools/data_residue.sh` ; `--detail` des deux scripts relevé avant et après ; pour un motif gardé, un test de vivacité (violation plantée, écrite avec les noms neufs, qui fait échouer `--check`) ; `make test` vert.
- **Familles d'application** (colonne « Application » de l'index) : chaque entrée relève d'une seule famille, qui fixe la sous-étape de l'étape 5 où elle s'applique (`docs/plan-5.md`, §6, 5a–5f) :
  - **O** (outillage) : un outil, sa règle ou le mécanisme des gardes — 5a ;
  - **G** (garde à motifs) : un motif, une exemption ou une fixture de `tools/composition_residue.sh` ou `tools/data_residue.sh`, procédure G — 5b ;
  - **N** (nommage) : un renommage par la procédure N ou par le `sed` de l'entrée, ou une retouche ponctuelle d'un test (assertion, tableau) — 5c ;
  - **X** (ligne d'exception) : un reste compté par un compteur, levé par une ligne d'exception ou par la règle d'outil qui en tient lieu — 5d ;
  - **T** (`TROU` → `GAP`) : un marqueur `TROU-N` — 5d ;
  - **D** (docs et skills) : un fichier de `docs/`, du `CLAUDE.md` ou de `.claude/` — 5f.
- **Verdicts** : ensemble fermé de quatre valeurs, écrites telles quelles sur la ligne `Verdict` : `proposition` (la proposition prête, appliquée telle quelle), `variante nommée` (une option nommée de l'entrée, par sa lettre, ou une variante écrite en une ligne), `garder tel quel` (rien n'est édité), `reporter hors plan 5` (la question part dans un chantier hors du plan 5, rien n'est édité). Une entrée à reste compté (section « Clôture (4l4) », « Restes comptés ») dont le verdict est `garder tel quel` ou `reporter hors plan 5` finit en ligne d'exception : sans elle, son reste rougit la garde quand elle devient bloquante (5e).

## Index

| n° | Origine | Question | Sites | Application |
|---|---|---|---:|---|
| D-1 | 2b, 3l | Repli `tmp/pico8_replay` de `tools/frames_ref.sh` | 1 | X |
| D-2 | 3k, 3l | `PICO8_CLASS_COVERAGE`, macro `-D` hors TSV | 35 | N |
| D-3 | 3a, 3l | `PICO8_MALLOC_FILLS`, variable d'environnement | 2 | N |
| D-4 | 3l | Noms `pico8_*` des fixtures de `test_alloc_guard.c` | 36 | N |
| D-5 | 3a, 3l | Motif générique `void-ptr` (`pico8_X(`) | 0 prototype | G |
| D-6 | 3a | `LIEN_EXCLUSION` mort | 2 | G |
| D-7 | 3a | Motif générique `pico8_X_of` (`lien-parent`) | 0 | G |
| D-8 | 3a | Motif générique `pico8_X_start` (`défauts`, `--champs`) | 0 | G |
| D-9 | 3a, 3b, 3d | Motif `spec` `pico8_X_(default\|defaults_reset)` et `SPEC_EXCLUSION` mort | 0 + 2 | G |
| D-10 | 3b | Exclusion `usage-mutuel` (`PICO8`, `P8`) morte | 0 | G |
| D-11 | 3a | Catégories historiques des gardes à motifs | 9 catégories | G |
| D-12 | 3e | Suffixe de constructeur de `--champs` | 5 | G |
| D-13 | 3a, 3l | 12 statiques locales de deux lettres des tests | 12 | X |
| D-14 | 3c | Ordre des mots des macros de taille de zone | 12 | N |
| D-15 | 3f | `probe_own` stocke un `targeting_t` | 24 | N |
| D-16 | 3f | `probe_link_target` déclaré hors de son module | 4 | N |
| D-17 | 3d, 3e, 3f, 3i, 3j | Noms qui bégaient | 60 | N |
| D-18 | 3f | Étiquette morte `main_mouse` de `test_main_mouse.c` | 1 | N |
| D-19 | 3g | `planet_upkeep` sans le préfixe `economy_score_` | 11 | N |
| D-20 | 3h | Singletons `main_*` déclarés hors d'un module `main_*` | 207 | N |
| D-21 | 3i, 3j | Préfixe `g_` des tables `static const` de classe | 143 | N |
| D-22 | 3j | Suffixe `_t` sur deux variables | 9 | N |
| D-23 | 3j | Faute `test_update_entities_drives_a_entity_struct` | 2 | N |
| D-24 | 3j | `DIFFICULTY_OPTIONS_ROW_CAP` sans son module | 3 | N |
| D-25 | 0, 2b, 3k, 3l | `p8_input_*` et `p8_fatal` définis hors de `p8/` | 5 fonctions | N |
| D-26 | 3g, 3h | Lignes `chaîne` `pz` et `go` éditées hors de `rename-strings` | 2 | N |
| D-27 | 3k | Motif `pico8_class_coverage_record` de `tools/rename_strings.py` | 1 | O |
| D-28 | §6 « Outillage figé » | Alertes `warn-review` de fin de tronçon 2 | 4 alertes connues | O |
| D-29 | PRD 4a | Paramètres et locales courts nommés d'un digramme du cart | 131 paramètres de méthode | N |
| D-30 | 2b (échantillon) | `typed_main_*` des tests contre `main_*_typed` | 96 | N |
| D-31 | 2b (échantillon) | Digramme `oc` : `tickets` et `type_picker` | 3 | N |
| D-32 | 2b (échantillon) | Champs de l'icône de structure : `rest_count` est une position | 22 | N |
| D-33 | 4a | Exceptions du compteur de français : schéma du TSV et fixtures françaises (3a) | 26 | X |
| D-34 | 4a | Assertion faible `strstr(log, "exempt")` de `test_class_coverage.c` | 1 | N |
| D-35 | 4b | Noms d'identifiant qui citent Lua ou le cart (`P8_FIXED_LIT`, trois tests) | 8 | N |
| D-36 | 4b | `` `mem` `` et `` `gfx` `` comptés en `backtick` après le renommage des locales de `test_demo.c` | 23 | O |
| D-37 | 4c1 | `argc`/`argv` de `main` dans la liste des locales cryptiques | 10 déclarations | X |
| D-38 | 4c1 | Statiques de fichier `mem`/`gfx` des tests | 80 déclarations | N |
| D-39 | 4d | Nom de test qui cite le cart (`test_data_init.c`) | 2 | N |
| D-40 | 4d | Clés du cart sans ligne `chaîne` au TSV (`"br"`, `"qr"`, `"rf"`, `"s"`, `"qh+"`) | 0 après 4d | O |
| D-41 | 4d | Nom de test qui garde le digramme `r` (`test_map_tables.c`) | 2 | N |
| D-42 | 4e | Ligne du TSV d'une locale renommée deux fois | 14 lignes | O |
| D-43 | 4e | Paramètres `gfx`/`mem` des fonctions de dessin | 96 + 128 déclarations | N |
| D-44 | 4e | Champ `planet_inspection_panel_ctx_t.unmet_needs`, qui tient les besoins reçus | 7 + 1 | N |
| D-45 | 4f1 | Noms de test qui disent « grab arena » pour une prise (`test_live_slots.c`) | 16 + fixture | N |
| D-46 | 4f2 | Fonctions de `main_economy.h` définies dans `probe.c` (`probe_set_main_economy`) | 3 + 159 | N |
| D-47 | 4g | `structure_icon_sprite_counter` compte des identifiants de planète, pas des sprites | 4 + 5 + 5 | N |
| D-48 | 4h | Noms de test qui décrivent l'API générique disparue ou la méthode du cart (`test_ui.c`, `test_main_mouse.c`, `test_main_camera.c`) | 2 + 2 + 4 + 8 | N |
| D-49 | 4i | Assistants de test qui citent un digramme du cart ou un surnom (`planet_by_ref`, `build_plate`, `fold_plate`, `is_plate`) | 2 + 6 + 3 + 3 | N |
| D-50 | 4j | Locales courtes dont le nom porte un digramme du cart (`c_truthy`, `s_str`) | 2 + 3 | N |
| D-51 | 4j | `spacer` qui tient un émetteur dans quatre tests de 3i | 55 | N |
| D-52 | 4k2 | Nom de test `test_the_shared_palette_is_the_published_pico8_one` | 2 | N |
| D-53 | 4a, 4k3 | Assertion d'absence `"unknown"` sans écrivain (`test_class_coverage.c`) | 1 | N |
| D-54 | 4k3 | Préfixe `k_` des tableaux `static const` du morceau 4k3 | 26 | N |
| D-55 | 4l1 | Exception `du` du compteur de français dans le `Makefile` | 1 | X |
| D-56 | 4l1 | Citations `cart`/`lua` irréductibles de la légende du `Makefile` | 6 + 2 | X |
| D-57 | 4e, 4l2 | Noms du cart dans les fixtures de `tests/test_composition_residue.c` | 83 | G |
| D-58 | 4l2 | Macro `ATT` de `tests/test_composition_residue.c` | 7 | N |
| D-59 | 4b, 4l3 | Faux positifs des compteurs dans le relevé figé `tools/pico8_reference_probe_out.txt` | 8 + 7 | X |
| D-60 | 4l3 | `blank` et `screen_texture` du `main` de `src/cmd/screencast_viewer.c` | 2 (+ 2) | X |
| D-61 | 4l4 | Exception `LÖVE` du compteur de français (`tools/ttf_to_pixelfont.py`) | 1 | X |
| D-62 | 4l4 | Noms au préfixe du cart des fixtures des tests de garde, hors `test_alloc_guard.c` | 8 + 2 + 1 | N |
| D-63 | 4l4 | Noms du cart dans les fixtures de `tests/test_data_residue.c` | 23 lignes | G |
| D-64 | 4l4 | Marqueur `# TROU-1` de `tools/constants_expected_gaps.txt` | 1 | T |
| D-65 | 4l4 | Leçons de la consigne de l'étape 4 à reporter dans le skill `reprendre-les-commentaires` | 1 fichier | D |
| D-66 | consultation `plan-review` d'avant 5 | Comment les deux gardes deviennent bloquantes | 2 gardes | O |
| D-67 | 4a, §6 entrée 5 | Le compteur de français devient-il une troisième garde ? | 36 `string` | O |
| D-68 | 4l4, consultation `plan-review` d'avant 5 | Alertes `warn-review` de fin de tronçon 3 | 16 alertes | O |

## Entrées

### D-1 — Repli `tmp/pico8_replay` de `tools/frames_ref.sh`

- **Origine** : 2b (binaires, voie (a)), 3l (« Fin de l'étape 3 », seule `string` restante de `cart-names-count`).
- **Question** : l'ancien nom de binaire gardé exprès par `frames_ref.sh`, pour reconstruire un commit d'avant la règle `$(TMP_DIR)/replay:`, devient-il une exception permanente de la garde anti-cart ?
- **Options** : (a) garder le repli et l'excepter ; (b) retirer le repli : `make e2e-ref` ne sait plus rejouer une référence posée avant `2bb367d` (3l), soit toutes les références actuelles de `tests/e2e/`.
- **Sites** : 1 en code (`tools/frames_ref.sh:113`, `driver=tmp/pico8_replay`) ; 3 en commentaire (`src/cmd/frames.c:1`, `tests/test_replay_from_title.c:1`, `tests/test_headless_capture.c:5`), repris par 4k et 4l.
- **Proposition prête à appliquer** : (a). À l'étape 5, quand la garde devient bloquante, `tools/rename_count.py` lit un fichier `tools/cart_names_exceptions.txt` au format de `tools/scope_exceptions.txt` (`colonne<TAB>module<TAB>nom`, « seule une décision humaine ajoute une ligne »), avec l'unique ligne `string	tools/frames_ref	tmp/pico8_replay` ; changement d'outil, donc test d'abord dans `tests/test_rename_count.py` (une ligne retire exactement son triplet).
- **Contrôle** : `make cart-names-count` sans `SUBSTEP` : `TOTAL` 0 dans les quatre colonnes.
- **Complément (4k1)** : le site de commentaire de `tests/test_headless_capture.c:5` (« `tmp/pico8_replay` est invoqué par `system()` ») est réécrit au nom du binaire réel, `tmp/replay` ; restent 2 sites en commentaire (`src/cmd/frames.c:1`, `tests/test_replay_from_title.c:1`, 4l) et le site de code.
- **Complément (4l3)** : les deux sites de commentaire restants (`src/cmd/frames.c:1`, `tests/test_replay_from_title.c:1`) sont réécrits au nom du binaire réel (`tmp/replay`) ; reste le site de code (`tools/frames_ref.sh:113`), seule `string` de `cart-names-count` (`MODULES=<4l3>` 0/0/1/0 ; sans `SUBSTEP`, `TOTAL` 1).
- **Verdict** : ok

### D-2 — `PICO8_CLASS_COVERAGE`

- **Origine** : 3k (« Pour décision humaine »), 3l.
- **Question** : la macro de compilation de la couverture de classes garde le préfixe du cart, interdit par le §2 ; aucune ligne du TSV ne la porte (un `-D` sans `#define`), aucun compteur ne la voit. La renommer, et en quoi ?
- **Options** : (a) `CLASS_COVERAGE_BUILD` ; (b) `CLASS_COVERAGE` ; (c) garder.
- **Sites** : 35 — 31 dans `src/` (21 fichiers de `game/` et `devtools/`), 2 dans `tools/float_grep_exclusions.txt` (texte), 2 dans le `Makefile` (le `-D` de `COVERAGE_CFLAGS`, ligne 247, et un commentaire, ligne 995 ; numéros d'après 4l1, 232 et 971 avant). La configuration `coverage` de `rename-extract` (`AST_RENAME_ANALYSIS`) lit `COVERAGE_CFLAGS` : un seul point de définition.
- **Proposition prête à appliquer** : (a), qui ne heurte aucun nom (0 site). Hors AST (pas d'identité) : `grep -rl PICO8_CLASS_COVERAGE src tools/float_grep_exclusions.txt Makefile | xargs sed -i 's/PICO8_CLASS_COVERAGE/CLASS_COVERAGE_BUILD/g'` (sans `\b` : le `-D` colle la macro à l'option) ; commit `refactor:`.
- **Contrôle** : `make class-coverage` et `make test-all` verts ; `make rename-extract` sans `warning:` (les 4 lignes `partial` de `class_coverage` attendues) ; `make float-grep` vert ; `grep -rn PICO8_CLASS_COVERAGE src tests tools Makefile` hors outils du dictionnaire rend 0.
- **Portée** : s'applique aussi aux mentions en commentaire et entre backticks.
- **Complément (4l1)** : le commentaire de la règle de `tmp/class_coverage`, traduit, garde le nom `PICO8_CLASS_COVERAGE` (ligne 995) ; le `-D` passe à la ligne 247. Les deux sites du `Makefile` restent, 35 sites inchangés.
- **Complément (4l3)** : les deux justifications de `tools/float_grep_exclusions.txt` traduites (`src/cmd/class_coverage.c`, `src/devtools/class_coverage.c`) gardent `PICO8_CLASS_COVERAGE` ; sites inchangés.
- **Verdict** : ok

### D-3 — `PICO8_MALLOC_FILLS`

- **Origine** : 3l (« Fin de l'étape 3 ») ; PRD de 4a (laissée en attente de ce verdict).
- **Question** : la variable d'environnement de `tests/test_entity_header.c`, au préfixe du cart, hors TSV, que le compteur ne voit pas.
- **Options** : (a) `MALLOC_FILLS` ; (b) `ENTITY_HEADER_MALLOC_FILLS` ; (c) garder.
- **Sites** : 2, tous deux dans `tests/test_entity_header.c` (`getenv`, ligne 47 ; `setenv`, ligne 53) : lue et écrite par le même fichier.
- **Proposition prête à appliquer** : (a). `sed -i 's/\bPICO8_MALLOC_FILLS\b/MALLOC_FILLS/g' tests/test_entity_header.c` ; commit `test:`.
- **Contrôle** : `make test` vert (le test se relance lui-même avec la variable) ; `grep -rnw PICO8_MALLOC_FILLS src tests tools Makefile` rend 0.
- **Portée** : s'applique aussi aux mentions en commentaire.
- **Verdict** : ok

### D-4 — Noms `pico8_*` des fixtures de `test_alloc_guard.c`

- **Origine** : 3l (« Fin de l'étape 3 ») ; élargi par la mesure de 4a aux autres noms des mêmes fixtures.
- **Question** : `tests/test_alloc_guard.c` écrit et compile des sources fictives aux noms préfixés du cart ; hors TSV, le compteur ne les voit pas. `tools/alloc_guard.sh` et `tools/alloc_guard_exceptions.txt` n'en citent aucun.
- **Options** : (a) retirer le préfixe ; (b) garder (fixtures).
- **Sites** : 36 — fichiers `pico8_allocs.c` 15, `pico8_clean.c` 6, `pico8_gone.c` 2, `pico8_transient_user.c` 4 ; symboles `pico8_zone_transitoire` 4 (français), `pico8_malloc_count`, `pico8_grow`, `pico8_half`, `pico8_kept`, `pico8_use` 1 chacun.
- **Proposition prête à appliquer** : (a). Dans `tests/test_alloc_guard.c` seulement : `pico8_allocs` → `allocs`, `pico8_clean` → `clean`, `pico8_gone` → `gone`, `pico8_transient_user` → `transient_user`, `pico8_zone_transitoire` → `transient_zone`, `pico8_malloc_count` → `malloc_count`, `pico8_grow` → `grow`, `pico8_half` → `half`, `pico8_kept` → `kept`, `pico8_use` → `use` ; un `sed` par paire, mot entier ; commit `test:`.
- **Contrôle** : `make test` vert (`test_alloc_guard` compris) ; `grep -c 'pico8_' tests/test_alloc_guard.c` rend 0.
- **Portée** : s'applique aussi aux mentions en commentaire du même fichier.
- **Complément (4l4)** : les commentaires de `tests/test_alloc_guard.c`, réécrits par 4l4, décrivent les fixtures par leur rôle (« the allocating source », « an external zone handle ») ; les fixtures sont intouchées. Remesuré à la fin de 4l4 : 33 sites (`pico8_allocs` 14, `pico8_clean` 6, `pico8_gone` 2, `pico8_transient_user` 3, `pico8_zone_transitoire` 3, les cinq symboles 1 chacun) ; les 3 de moins étaient des mentions en commentaire. D-62 pose la même question pour trois autres tests de garde.
- **Verdict** : ok

### D-5 — Motif générique `void-ptr` (`pico8_X(`)

- **Origine** : 3a (inventaire des gardes à motifs), 3l.
- **Question** : le motif de `composition_residue.sh` (`(^|[^A-Za-z0-9_])pico8_[a-z0-9_]+\(`, ligne 423) ne reconnaît plus aucun prototype : il passe à vide sur l'arbre, ses fixtures l'exercent encore. Le traduire sans préfixe, ou le déclarer intraduisible ?
- **Options** : (a) traduire sans préfixe (`[a-z0-9_]+\(`) si `--detail` ne bouge pas ; (b) intraduisible : garder tel quel, fixtures comprises ; (c) retirer la catégorie et ses fixtures.
- **Sites** : 0 prototype dans les en-têtes de `src/` ; 188 mentions `pico8_X(` restent en commentaire (étape 4). Sans préfixe, le motif verrait toute ligne d'en-tête qui ouvre une parenthèse après un nom en minuscules : 931 lignes.
- **Proposition prête à appliquer** : (a) ne peut pas laisser `--detail` identique : (b), motif et fixtures de `tests/test_composition_residue.c` gardés tels quels.
- **Contrôle** : procédure G.
- **Complément (4l2)** : la catégorie garde son nom `void-ptr` ; le motif est à la ligne 441 de `tools/composition_residue.sh` après 4l2 (423 avant) ; les fixtures de `tests/test_composition_residue.c` sont intouchées (D-57).
- **Verdict** : ok

### D-6 — `LIEN_EXCLUSION` mort

- **Origine** : 3a.
- **Question** : `LIEN_EXCLUSION='s/entity_state_of//g'` (`composition_residue.sh:72`, lu ligne 79) n'a rien à mordre : ni `\bpico8_[a-z0-9_]+_of\b` ni `(emitter|probe)_of` n'apparient `entity_state_of`.
- **Options** : (a) retirer ; (b) garder pour le jour où `lien-parent` est traduit sans préfixe (D-7), qui apparierait de nouveau `entity_state_of`.
- **Sites** : 2 (définition, emploi).
- **Proposition prête à appliquer** : suivre D-7 — retirer si D-7 garde le préfixe ou retire la catégorie ; garder si D-7 traduit sans préfixe (l'exemption redevient vivante, test de vivacité à écrire).
- **Contrôle** : procédure G.
- **Complément (4l2)** : `LIEN_EXCLUSION` s'appelle `PARENT_LINK_EXCLUSION` (noms internes du script, 4l2) ; définition ligne 77, emploi ligne 84, après 4l2. Le commentaire qui le précède dit toujours pourquoi : `entity_state_of` n'est pas une lignée.
- **Verdict** : ok

### D-7 — Motif générique `pico8_X_of` (`lien-parent`)

- **Origine** : 3a.
- **Question** : la première branche de `lien-parent` (`\bpico8_[a-z0-9_]+_of\b`, `composition_residue.sh:59`) ne voit plus rien.
- **Options** : (a) traduire sans préfixe (`\b[a-z0-9_]+_of\b`) si `--detail` ne bouge pas ; (b) intraduisible, gardé ; (c) retirer la branche.
- **Sites** : 0 nom `pico8_X_of` dans `src/` ; sans préfixe, 21 occurrences y seraient vues (`slot_kind_of` 4, `pos_of` 4, `slipway_infraspace_of` 3, `resource_code_of` 3, `flip_zone_of` 3, `map_pos_of` 2, `entity_state_of` 2).
- **Proposition prête à appliquer** : (a) ne peut pas laisser `--detail` identique (21 occurrences nouvelles) : (b), la branche gardée telle quelle, et D-6 retiré.
- **Contrôle** : procédure G.
- **Complément (4l2)** : `lien-parent` s'appelle `parent-link` (catégories en anglais, 4l2, avec le `Makefile` et le test) ; la branche `\bpico8_[a-z0-9_]+_of\b` est à la ligne 64 après 4l2.
- **Verdict** : ok

### D-8 — Motif générique `pico8_X_start` (`défauts`, `--champs`)

- **Origine** : 3a.
- **Question** : `défauts` (`\bpico8_[a-z0-9_]+_start(_t)?\b`, ligne 61) et le relevé `--champs` (ligne 676) ne voient plus aucun archétype (`pico8_X_def` est devenu `X_archetype`).
- **Options** : (a) traduire en `\b[a-z0-9_]+_archetype(_t)?\b`, le nom réel des archétypes, si `--detail` ne bouge pas ; (b) intraduisible ; (c) retirer.
- **Sites** : 0 `pico8_X_start` dans `src/` ; `_start` sans préfixe : 3 ; `\b[a-z0-9_]+_archetype(_t)?\b` : 212 occurrences dans `src/`.
- **Proposition prête à appliquer** : (a) ne peut pas laisser `--detail` identique (212 occurrences nouvelles pour `défauts`) : (b), motif gardé tel quel. Le relevé `--champs` (ligne 676) reste un majorant.
- **Contrôle** : procédure G.
- **Complément (4l2)** : `défauts` s'appelle `defaults` et `--champs` s'appelle `--fields` (4l2) ; le motif est à la ligne 66, le `gsub` de `--fields` à la ligne 697, après 4l2 ; le commentaire de `fields()` et l'en-tête citent `` `pico8_X_start` `` entre backticks.
- **Verdict** : ok

### D-9 — Motif `spec` et `SPEC_EXCLUSION`

- **Origine** : 3a, 3b, 3d.
- **Question** : la branche `\bpico8_[a-z0-9_]+_(default|defaults_reset)\b` de `spec` (`data_residue.sh:38`) ne voit plus rien ; `SPEC_EXCLUSION` (ligne 49, lu ligne 94), traduit par 3b et 3d, est mort en entier (vidé : 0 ligne de `spec`, aucun test de vivacité possible).
- **Options** : (a) retirer `SPEC_EXCLUSION` et garder la branche ; (b) traduire la branche sans préfixe (`\b[a-z0-9_]+_(default|defaults_reset)\b`) : mesuré par 3d, elle voit les 7 noms exemptés de `src/` et les moitiés recollées du témoin de `tests/test_data_residue.c` (`spec` y monterait à 6 lignes), il faudrait recouper les témoins.
- **Sites** : branche 0 ; `SPEC_EXCLUSION` 2 (définition, emploi) ; témoins de `test_data_residue.c` (3d : 6 lignes).
- **Proposition prête à appliquer** : (a) — supprimer les lignes 49 et 94 (le `sed` devient sans objet : `grep` direct) ; les témoins restent.
- **Contrôle** : procédure G ; `make data-guard` vert.
- **Complément (4l4)** : après 4l4, la branche est toujours à `data_residue.sh:38`, `SPEC_EXCLUSION` défini ligne 49 et lu ligne 94 (en-tête réécrit à hauteur égale). Le commentaire qui le précède dit : « These seven `_default` names are variants with an omitted argument, not class defaults: counting them would make spec impossible to clear. No `\b` in this sed expression: BSD sed does not know it. » (sept noms comptés ; le piège tient pour `sed`, les `grep -E` du script prennent `\b`).
- **Verdict** : ok

### D-10 — Exclusion `usage-mutuel` morte

- **Origine** : 3b.
- **Question** : `usage-mutuel` écarte les jetons `PICO8` et `PICO8_` (`composition_residue.sh:522`) ; aucun jeton `PICO8`, `PICO8_`, `P8` ou `P8_` n'est déclaré, ni avant ni après 3b.
- **Options** : (a) retirer l'exclusion ; (b) la garder.
- **Sites** : 0 jeton déclaré.
- **Proposition prête à appliquer** : (a) — retirer `x == "PICO8" || x == "PICO8_" ||` de la ligne 522.
- **Contrôle** : procédure G (`--detail` identique, `composition-guard` vert).
- **Complément (4l2)** : `usage-mutuel` s'appelle `mutual-usage` (4l2) ; l'exclusion `x == "PICO8" || x == "PICO8_" ||` est à la ligne 542 après 4l2.
- **Verdict** : ok

### D-11 — Catégories historiques des gardes à motifs

- **Origine** : 3a (classe « historique » de l'inventaire).
- **Question** : neuf catégories ne reconnaissent que du code disparu avant le plan 5 et restent à zéro : `nu`, `aiguillage-nom`, `un-ou-faux` (`composition_residue.sh`, lignes 62–64, et `NU_EXCLUSION`) ; `valeur`, `ob`, `stash`, `type-tag`, `ref-audit`, `liste` (`data_residue.sh`). Toutes sont dans `COMPOSITION_RESIDUE_GUARD` ou `DATA_RESIDUE_GUARD` du `Makefile` et ont des fixtures dans `tests/test_composition_residue.c` et `tests/test_data_residue.c`.
- **Options** : (a) retirer chaque catégorie, son entrée de la liste de garde et ses fixtures ; (b) garder telles quelles.
- **Sites** : 9 catégories.
- **Proposition prête à appliquer** : (a). Leurs motifs citent des noms `pico8_*` qu'aucun code ne peut plus porter une fois la garde anti-cart bloquante (étape 5) : elles ne gardent plus rien. Un commit `refactor:` par script.
- **Contrôle** : procédure G — `--detail` égal à celui d'avant moins les lignes de ces catégories ; `make test` vert.
- **Complément (4l2)** : `aiguillage-nom` s'appelle `name-dispatch`, `un-ou-faux` `one-or-false`, `nu` est gardé (4l2) ; leurs motifs sont aux lignes 67 (`nu`), 68 (`name-dispatch`) et 69 (`one-or-false`), `NU_EXCLUSION` à la ligne 80, après 4l2 ; dans `COMPOSITION_RESIDUE_GUARD`, les noms neufs. Les six catégories de `tools/data_residue.sh` sont intouchées (4l4).
- **Complément (4l4)** : `valeur` s'appelle `value`, `par-nom` `by-name` et `liste` `list` (4l4, avec `DATA_RESIDUE_GUARD` et `tests/test_data_residue.c`) ; les six catégories historiques de `tools/data_residue.sh` sont `value` (ligne 29), `ob` (30), `stash` (31), `type-tag` (32), `ref-audit` (33), `list` (40–41), toutes dans `DATA_RESIDUE_GUARD`.
- **Verdict** : ok

### D-12 — Suffixe de constructeur de `--champs`

- **Origine** : 3e.
- **Question** : avec le suffixe `(_new|_init|_create|_init_instance)$`, `entity_init_header` (3a) et `allocator_init_slot_kinds` (3c) sortent des constructeurs, `emitter_init` (3f), `weighted_picker_new` et `random_walk_new` (3d) y entrent.
- **Options** : (a) garder le suffixe ; (b) une liste explicite de constructeurs.
- **Sites** : 5 fonctions.
- **Proposition prête à appliquer** : (a), rien à éditer : `--champs` est un majorant par nom, pas une porte (pied « 0 hors constructeur » 27, « 0 écriture » 15 à la fin de 3l).
- **Contrôle** : aucun.
- **Complément (4l2)** : l'option s'appelle `--fields` ; son pied dit `fields: 260; no write: 15; no write outside a constructor: 27` (« 0 écriture » 15 et « 0 hors constructeur » 27 avant) ; le suffixe est à la ligne 701. Rien d'autre ne bouge.
- **Verdict** : ok

### D-13 — 12 statiques locales de deux lettres des tests

- **Origine** : 3a (relevé de `scope-count`), 3l (« Fin de l'étape 3 ») ; noms proposés par 4a, pour que l'un ou l'autre verdict s'applique mécaniquement.
- **Question** : `scope-count` sans `SUBSTEP` compte 12 refus, tous des tables `static const` locales à une fonction de test, à portée ≤ 37 lignes. Exception par règle, ou noms neufs ?
- **Options** : (a) règle : dans `tools/scope_count.py`, une `static` dont le parent est une fonction se juge comme une locale, par sa portée (test d'abord dans `tests/test_scope_count.py`) ; (b) les noms ci-dessous, par la procédure N ; (c) 12 lignes d'exception dans `tools/scope_exceptions.txt`.
- **Sites** : 12 déclarations.

  | Fichier | Fonction | Nom | Nom proposé | Pourquoi |
  |---|---|---|---|---|
  | `tests/test_button.c` | `fold_hit_box` | `dy` | `y_offsets` | décalages verticaux sondés |
  | `tests/test_frame_zone.c` | `case_inspection_panel_rows_survive_reset` | `kw` | `production` | un `production_t` |
  | `tests/test_game_zone.c` | `registries_bytes` | `CV` | `MESSAGES` | clés de `entities_by_message` |
  | `tests/test_game_zone.c` | `registries_bytes` | `CG` | `TAGS` | clés de `entities_by_tag` |
  | `tests/test_live_slots.c` | `registries_bytes` | `CV` | `MESSAGES` | idem |
  | `tests/test_live_slots.c` | `registries_bytes` | `CG` | `TAGS` | idem |
  | `tests/test_layout_row.c` | `fold_drawn` | `as` | `alignments` | le `a` de `layout_row_draw_at` (0, 0,5, 1 : décalage de `a × largeur`) |
  | `tests/test_layout_row.c` | `test_constructions` (ligne 407) | `f` | `tuple_items` | éléments passés à `tuple_of` |
  | `tests/test_layout_row.c` | `test_constructions` (ligne 434) | `f` | `tuple_items` | idem, autre bloc |
  | `tests/test_layout_row.c` | `test_planet_resource_row` | `ks` | `project_indices` | indices passés à `project()` |
  | `tests/test_structure_icon.c` | `test_kind_view_is_a_struct` | `s` | `sprites` | `s` → `sprite` du vocabulaire |
  | `tests/test_structure_icon.c` | `test_kind_view_is_a_struct` | `od` | `costs` | `od` → `cost` du vocabulaire |

  Aucun nom proposé ne masque une locale de sa fonction (mesuré sur `declarations.tsv`). Identités : `c:<fichier>@<offset>@F@<fonction>@<nom>`, à relever sur l'extraction fraîche.
- **Proposition prête à appliquer** : (a), qui aligne la statique de fonction sur la tolérance du §1 sans liste à tenir ; (b) en repli.
- **Contrôle** : `make scope-count` sans `SUBSTEP` : `total` 0.
- **Portée** (si (b)) : s'applique aussi aux mentions en commentaire et entre backticks.
- **Complément (4l4)** : remesuré à la fin de l'étape 4, `scope-count` sans `SUBSTEP` : 12, les mêmes ; inchangé.
- **Verdict** : ok

### D-14 — Ordre des mots des macros de taille de zone

- **Origine** : 3c.
- **Question** : `FRAME_ZONE_BYTES`, `UI_ZONE_BYTES`, `LOAD_ZONE_BYTES` (forme `<zone>_ZONE_BYTES`) contre `ZONE_GAME_BYTES`, `ZONE_GENERATION_BYTES`, `ZONE_PARTICLES_BYTES`, `ZONE_BUDGET_BYTES`.
- **Options** : (a) tout en `<zone>_ZONE_BYTES` ; (b) tout en `ZONE_<zone>_BYTES` ; (c) garder.
- **Sites** : 30 occurrences des sept macros (12 pour les trois renommées : 5, 4, 3 ; `ZONE_BUDGET_BYTES` 2).
- **Proposition prête à appliquer** : (a), procédure N (`src/engine/zone_sizes.h`) :

  ```
  ZONE_GAME_BYTES	GAME_ZONE_BYTES	src/engine/zone_sizes.h	macro:src/engine/zone_sizes.h:ZONE_GAME_BYTES
  ZONE_GENERATION_BYTES	GENERATION_ZONE_BYTES	src/engine/zone_sizes.h	macro:src/engine/zone_sizes.h:ZONE_GENERATION_BYTES
  ZONE_PARTICLES_BYTES	PARTICLES_ZONE_BYTES	src/engine/zone_sizes.h	macro:src/engine/zone_sizes.h:ZONE_PARTICLES_BYTES
  ```

  `ZONE_BUDGET_BYTES` gardé : ce n'est pas une zone, c'est leur somme. Aucun des noms neufs n'existe (0 site).
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire ; contrôle sur les trois anciens noms.
- **Verdict** : ok

### D-15 — `probe_own` stocke un `targeting_t`

- **Origine** : 3f.
- **Question** : le champ `probe_own` de `planet_t`, `probe_t`, `slipway_t`, `wormhole_t` porte le composant `targeting_t` (vocabulaire : `g_` → `probe_t`, `pico8_probe_t` → `targeting_t`).
- **Options** : (a) `targeting_own` ; (b) garder.
- **Sites** : 24 (4 déclarations : `planet.h`, `probe.h`, `slipway.h`, `wormhole.h`).
- **Proposition prête à appliquer** : (a), procédure N, quatre lignes :

  ```
  probe_own	targeting_own	src/game/planet.h	c:@SA@planet_t@FI@probe_own
  probe_own	targeting_own	src/game/probe.h	c:@SA@probe_t@FI@probe_own
  probe_own	targeting_own	src/game/slipway.h	c:@SA@slipway_t@FI@probe_own
  probe_own	targeting_own	src/game/wormhole.h	c:@SA@wormhole_t@FI@probe_own
  ```

  `targeting_own` : 0 site.
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire ; contrôle sur `probe_own`.
- **Complément (4f2)** : 4f2 renomme les locales `probe` de `targeting_track` et `targeting_confirm` (`src/game/probe.c`) en `targeting` : elles tiennent un `targeting_t`, nom qui mentait, comme le champ `self->targeting` et la locale `targeting` de `fold_probe`. Reste le champ `probe_own`, objet de l'entrée ; ses mentions en commentaire (`probe.h`, `wormhole.h`, `targeting.h`) le citent sans backticks, par son nom courant.
- **Verdict** : ok

### D-16 — `probe_link_target` hors de son module

- **Origine** : 3f.
- **Question** : `probe_link_target` est déclaré dans `targeting.h`, défini dans `probe.c`.
- **Options** : (a) garder (§7 : pas de redécoupage pendant le plan 5) ; (b) déplacer la déclaration dans `probe.h` ; (c) renommer `targeting_link_target` et déplacer la définition dans `targeting.c`.
- **Sites** : 4.
- **Proposition prête à appliquer** : (a), rien à éditer ; le redécoupage est noté pour un chantier hors du plan 5.
- **Verdict** : ok

### D-17 — Noms qui bégaient

- **Origine** : 3d, 3e, 3f, 3i, 3j.
- **Question** : dix noms respectent le §4 mais répètent un mot.
- **Options** : (a) les noms proposés ; (b) garder.
- **Sites** : 60 au total.

  | Ancien | Sites | Nom proposé | Identité |
  |---|---:|---|---|
  | `data_init_data_shutdown` | 4 | `data_init_shutdown` | `c:@F@data_init_data_shutdown` |
  | `UPDATE_VANISHING_UPDATE_ENTITIES_CALLS` | 2 | `UPDATE_VANISHING_CALLS` | `macro:tests/test_planet.c:UPDATE_VANISHING_UPDATE_ENTITIES_CALLS` |
  | `targeting_track_target_targeted_by` | 5 | `throwaway_targeted_by` | `c:test_probe.c@F@…`, `c:test_wormhole.c@F@…` |
  | `TARGET_TOO_FAR_TARGET` | 8 | `TARGET_TOO_FAR` | `c:@Ea@TARGET_ABSENT@TARGET_TOO_FAR_TARGET` |
  | `emitter_init_help_screen_takes_a_particles_help_screen_list` | 2 | `emitter_init_help_screen_takes_a_help_list` | à relever (nom de cas) |
  | `entity_list_list_size` | 14 | `entity_list_count` | `c:test_<fichier>.c@F@entity_list_list_size`, 5 fichiers de 3i |
  | `entity_list_pointer_list_size` | 11 | `entity_list_count` | `c:test_<fichier>.c@F@entity_list_pointer_list_size`, 4 fichiers |
  | `score_panel_set_main_score_panel` | 3 | `score_panel_set_main` | `c:@F@score_panel_set_main_score_panel` |
  | `score_panel_score_rows` | 3 | `score_panel_rows` | `c:@F@score_panel_score_rows` |
  | `score_rows_row_t` | 8 | `score_row_t` | `c:@T@score_rows_row_t` |

  Aucun nom proposé n'existe (0 site chacun). `entity_list_list_size` et `entity_list_pointer_list_size` sont la même fonction statique (taille d'une `entity_list_t`, -1 si NULL) sous deux noms : un seul nom neuf.
- **Proposition prête à appliquer** : (a), procédure N (lignes `ancien nouveau fichier identité` du tableau, une par identité) ; les lignes de `tools/rename_overrides.tsv` sont `name<TAB><ancien><TAB><nouveau>`.
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire ; contrôle sur chaque ancien nom.
- **Complément (4e)** : le libellé `"update_vanishing_update_entities_calls"` de `check_int()` (`tests/test_planet.c`) est le nom de `UPDATE_VANISHING_UPDATE_ENTITIES_CALLS` en minuscules (règle des libellés, consigne point 7 ; `git show dd1d875:docs/consigne-etape-4.md`) : si (a) est appliquée, il devient `"update_vanishing_calls"` dans le même commit.
- **Complément (4f2)** : `emitter_init_help_screen_takes_a_particles_help_screen_list` est un nom de cas : la fonction est `case_emitter_init_help_screen_takes_a_particles_help_screen_list` (`tests/test_particle_zone.c`), et la chaîne de `CASES[]` et celle de son `expect_clean` suivent la fonction (règle des noms de cas, consigne point 7 ; `git show dd1d875:docs/consigne-etape-4.md`) ; sous (a), trois sites d'un coup : la fonction devient `case_emitter_init_help_screen_takes_a_help_list`, ses deux chaînes `"emitter_init_help_screen_takes_a_help_list"` (lignes 330 et 423 à la fin de 4f2), et l'identité de la ligne du tableau ci-dessus est `c:test_particle_zone.c@F@case_emitter_init_help_screen_takes_a_particles_help_screen_list`.
- **Verdict** : ok

### D-18 — Étiquette morte `main_mouse` de `test_main_mouse.c`

- **Origine** : 3f.
- **Question** : `tests/test_main_mouse.c:45`, `tags[] = {"main_mouse", "mouse", "ui", "camera", "entity"}` : aucune classe n'écrit l'étiquette `main_mouse`, sa recherche rend toujours une liste vide.
- **Options** : (a) la retirer du tableau ; (b) garder.
- **Sites** : 1.
- **Proposition prête à appliquer** : (a), commit `test:` ; vérifier d'abord que la boucle sur `tags` ne dépend pas de sa taille (un compte attendu de lignes imprimées).
- **Contrôle** : `make test` vert.
- **Verdict** : ok

### D-19 — `planet_upkeep` sans le préfixe `economy_score_`

- **Origine** : 3g.
- **Question** : `planet_upkeep` (déclarée dans `economy_score.h`) reste seule sans le préfixe de son module, alors que `economy_score_net_income` prend aussi une planète.
- **Options** : (a) `economy_score_planet_upkeep` ; (b) garder (méthode du type receveur, §4).
- **Sites** : 11.
- **Proposition prête à appliquer** : (a), procédure N : `planet_upkeep	economy_score_planet_upkeep	src/game/economy_score.h	c:@F@planet_upkeep` (nom neuf : 0 site).
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire ; contrôle sur `planet_upkeep`.
- **Verdict** : ok

### D-20 — Singletons `main_*` hors d'un module `main_*`

- **Origine** : 3h.
- **Question** : `main_ui`, `main_mouse`, `main_mouse_typed`, `main_mouse_placeholder_class` sont déclarés dans `ui.h`, `main_camera_typed` dans `scroll.h` (`main_economy*` ont leur `main_economy.h`).
- **Options** : (a) garder (noms du vocabulaire) ; (b) déplacer leurs déclarations dans des en-têtes `main_*` (redécoupage, §7).
- **Sites** : 207 — `main_ui` 111, `main_mouse` 18, `main_mouse_typed` 28, `main_mouse_placeholder_class` 3, `main_camera_typed` 47.
- **Proposition prête à appliquer** : (a), rien à éditer.
- **Verdict** : ok

### D-21 — Préfixe `g_` des tables `static const` de classe

- **Origine** : 3i, 3j (question transverse).
- **Question** : le §4 réserve `g_` aux statiques mutables de fichier ; les tables de classe et de sorte sont des `static const` préfixées `g_`.
- **Options** : (a) retirer `g_` en suffixant `_def` : `g_X_class` → `X_class_def`, `g_X_kind` → `X_kind_def` (sans heurter les accesseurs `X_class()`, `X_kind()`) ; (b) garder, et préciser le §4 (« `g_` pour toute statique de fichier ») ; un verdict pour toutes.
- **Sites** : 54 déclarations `static const entity_class_t g_…` (45) et `static const entity_kind_t g_…` (9), dans 25 fichiers, 53 noms distincts, 143 occurrences. Hors question : 7 autres `static const` préfixées `g_` (tables de données, `button_fn_t`…), que le même verdict couvrirait s'il est (a).
- **Proposition prête à appliquer** : (a) ; aucun nom en `_def` n'existe (0 site). Procédure N, une ligne par déclaration, identités `c:<fichier>.c@g_X_class` à relever (statiques de fichier).
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire ; contrôle `grep -rnE '\bg_[a-z0-9_]+_(class|kind)\b' src tests` rend 0.
- **Verdict** : ok

### D-22 — Suffixe `_t` sur deux variables

- **Origine** : 3j.
- **Question** : `g_score_panel_new_year_keep_open_t` (`score_panel_display.c`) et `g_fake_update_last_t` (`test_toast.c`) sont des variables, pas des types.
- **Options** : (a) retirer le `_t` ; (b) garder.
- **Sites** : 5 + 4.
- **Proposition prête à appliquer** : (a), procédure N :

  ```
  g_score_panel_new_year_keep_open_t	g_score_panel_new_year_keep_open	src/game/score_panel_display.c	c:score_panel_display.c@g_score_panel_new_year_keep_open_t
  g_fake_update_last_t	g_fake_update_last	tests/test_toast.c	c:test_toast.c@g_fake_update_last_t
  ```

  Noms neufs : 0 site.
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire.
- **Verdict** : ok

### D-23 — Faute `test_update_entities_drives_a_entity_struct`

- **Origine** : 3j.
- **Question** : « a entity ».
- **Options** : (a) `test_update_entities_drives_an_entity_struct` ; (b) garder.
- **Sites** : 2 (`tests/test_toast.c`).
- **Proposition prête à appliquer** : (a), procédure N : `test_update_entities_drives_a_entity_struct	test_update_entities_drives_an_entity_struct	tests/test_toast.c	c:test_toast.c@F@test_update_entities_drives_a_entity_struct`.
- **Verdict** : ok

### D-24 — `DIFFICULTY_OPTIONS_ROW_CAP` sans son module

- **Origine** : 3j.
- **Question** : macro locale de `help.c` sans le nom de son module (§4 : `MODULE_UPPER_CASE`).
- **Options** : (a) `HELP_DIFFICULTY_OPTIONS_ROW_CAP` ; (b) garder (macro locale d'un `.c`).
- **Sites** : 3.
- **Proposition prête à appliquer** : (a), procédure N : `DIFFICULTY_OPTIONS_ROW_CAP	HELP_DIFFICULTY_OPTIONS_ROW_CAP	src/game/help.c	macro:src/game/help.c:DIFFICULTY_OPTIONS_ROW_CAP` (0 site pour le nom neuf). Si (a), `DIFFICULTY_OPTIONS_COUNT` du même fichier suit le même verdict (à mesurer).
- **Verdict** : ok

### D-25 — `p8_input_*` et `p8_fatal` définis hors de `p8/`

- **Origine** : 0 (vocabulaire), 2b, 3k, 3l.
- **Question** : `p8_input_load`, `p8_input_save`, `p8_input_append`, `p8_input_free` sont déclarés dans `p8/input.h` et définis dans `devtools/input_json.c` ; `p8_fatal` est déclaré dans `p8/fatal.h` et défini dans `platform/fatal_pc.c`.
- **Options** : (a) garder (scission d'en-tête hors périmètre, §7) ; (b) scinder `p8/input.h`.
- **Sites** : 5 fonctions (occurrences : 28, 12, 8, 34, 6).
- **Proposition prête à appliquer** : (a), rien à éditer.
- **Complément (4k3)** : les commentaires d'`src/devtools/input_json.c` réécrits par 4k3 nomment `p8_input_save` et `p8_input_append` (au lieu de `pico8_input_save`, `pico8_input_append`), définis là et déclarés dans `src/p8/input.h` (« see p8_input_append's contract in p8/input.h ») ; rien d'autre ne change.
- **Complément (4l3)** : `p8_fatal` toujours défini dans `src/platform/fatal_pc.c`, qui n'a pas de commentaire ; rien d'autre.
- **Verdict** : ok

### D-26 — Lignes `chaîne` `pz` et `go` éditées hors de `rename-strings`

- **Origine** : 3g (`pz`), 3h (`go`).
- **Question** : ces deux lignes sont passées à `conservé` dans `docs/renommage.tsv` seulement ; une régénération par `make rename-strings` les remettrait à `renommé`.
- **Options** : (a) les reporter en surcharges ; (b) les laisser (le TSV ne sera plus régénéré).
- **Sites** : 2 lignes.
- **Proposition prête à appliquer** : (a), deux lignes de `tools/rename_overrides.tsv` (forme `string`, lue par `rename_derive.read_overrides`) : `string	pz	=` et `string	go	=`.
- **Contrôle** : `make rename-strings` sur l'arbre redonne `conservé` aux deux lignes ; `git diff docs/renommage.tsv` vide pour elles.
- **Verdict** : ok

### D-27 — Motif `pico8_class_coverage_record` de `tools/rename_strings.py`

- **Origine** : 3k.
- **Question** : `CLASS_RECORD = re.compile(r"\bpico8_class_coverage_record\(\s*$")` (`tools/rename_strings.py:381`) n'apparie plus rien depuis 3k.
- **Options** : (a) `\bclass_coverage_record\(` ; (b) garder (l'outil ne sera plus lancé).
- **Sites** : 1.
- **Proposition prête à appliquer** : (a), avec ses fixtures de `tests/test_rename_strings.py` ; `make rename-strings-test` vert.
- **Verdict** : ok

### D-28 — Alertes `warn-review` de fin de tronçon 2

- **Origine** : §6, « Méthode des sous-étapes », puce *Outillage figé*. Le relevé complet des alertes n'a pas été versé dans le dépôt ; les quatre que le plan nomme sont remesurées ici. Les autres demandent une nouvelle passe de `/warn-review`, que seul un humain lance.
- **Question** : un verdict par alerte.
- **Sites et propositions** :
  - *Lecteurs en double du TSV* : 4 fonctions `read_table` (`tools/rename_check.py:94`, `tools/rename_count.py:77`, `tools/rename_strings.py:130`, `tools/ast_rename/plan.py:47`). Proposition : un lecteur, `rename_count.read_table`, les trois autres l'importent ; tous les `rename-*-test` et `rename-tool-test` verts.
  - *Listes de mots-clés divergentes* : `C_KEYWORDS` de `tools/rename_check.py` (59 mots) et de `tools/ast_rename/plan.py` (44) ; les 15 de plus sont `_BitInt`, `_Decimal32/64/128`, `alignas`, `alignof`, `bool`, `constexpr`, `false`, `nullptr`, `static_assert`, `thread_local`, `true`, `typeof`, `typeof_unqual`. Proposition : une liste, celle de `rename_check.py`, importée par `plan.py`.
  - *Schéma du TSV en français* : colonnes `ancien`, `nouveau`, `fichier`, `catégorie`, `sorte`, `statut`, `module`, `concept`, `note`, `identité`, valeurs `renommé`, `conservé`, `identifiant`, `chaîne`, `écrivain`, `lecteur`… Le TSV est une doc (§2, en français), mais les outils et leurs tests le lisent : `make french-count SUBSTEP=3a` compte ces mots dans `tests/test_scope_count.py` et `tests/test_cart_citations.py`. Proposition : garder le schéma (le TSV est le pont inverse documenté en français) et excepter ces mots au compteur de français (lignes à porter par la note de 4a).
  - *Version de libclang fixée à deux endroits* : `CLANG_BINDINGS_VERSION=18.1.8` (`tools/install_ast_rename_bindings.sh:5`) et `LIBCLANG ?= /usr/lib/llvm-18/lib/libclang-18.so.1` (`Makefile:1203`, ligne 1178 avant 4l1). Proposition : le `Makefile` passe la version majeure au script (`CLANG_MAJOR := 18`), une seule source.
- **Verdict** (un par alerte) :

### D-29 — Paramètres et locales courts nommés d'un digramme du cart

- **Origine** : PRD de 4a (mesure).
- **Question** : le §1 tolère une locale ou un paramètre de deux lettres à portée ≤ 40 lignes ; beaucoup portent encore le digramme du cart (`ra`, `qa`, `me`…), même là où le symbole homonyme a été renommé. Les renommer ?
- **Options** : (a) garder (§1) ; (b) renommer la seule famille des paramètres de méthode de la vtable et de leurs implémentations ; (c) renommer toutes les déclarations concernées.
- **Sites** : 131 paramètres de la famille de méthode dans l'extraction (`ra` 64, `qa` 24, `me` 12, `yr` 7, `fy` 7, `sw` 6, `lw` 6, `kv` 5) ; plus largement, 1 274 locales et paramètres de deux lettres dont le nom est l'ancien nom d'une ligne `renommé` du TSV (3a 26, 3b 41, 3c 74, 3d 75, 3e 214, 3f 169, 3g 150, 3h 204, 3i 194, 3j 107, 3k 12, 3l 8). Le chiffre de 713 de la PRD de 4a (prototype) n'est pas reproduit ; la définition retenue est celle-ci, sur `tmp/ast_rename/declarations.tsv`.
- **Proposition prête à appliquer** : (b), sens **à confirmer** : `qa` → `mouse_world`, `ra` → `mouse_screen` (le `main_mouse_pos` passé par `ui.c` à `hit_test` ; `mouse_pos` serait l'autre choix), `yr` → `year`, `sw` → `slipway`, `lw` → `offer`, `kv` → `dropped`, `fy` → `parallax`, `me` → `state`. Lignes `field <fonction>.<param>` de `tools/rename_overrides.tsv` et table `rename-apply` depuis une extraction fraîche. Mesuré : aucun nom proposé ne masque une locale ou un paramètre de la même fonction, ni une fonction, globale, macro ou type.
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire ; contrôle : `grep -rnwE '(qa|ra|yr|sw|lw|kv|fy|me)' src tests` ne rend plus de paramètre de méthode.
- **Complément (4h)** : 4h renomme les paramètres `qa`/`ra` du prototype de `ui_send_to_hovered` (`src/game/ui.h`) aux noms de sa définition, `mouse_world`/`mouse_screen` (règle de 4e : le prototype suit sa définition, dont le `pos` était une locale cryptique de portée 47) : `mouse_screen` est le nom que (b) propose pour `ra`. Les paramètres `qa`/`ra` des méthodes de la vtable (`left_press`, `right_press`, `hit_test`, `drag`…) restent tels quels.
- **Complément (4j)** : deux prototypes d'en-tête de 3j nomment leur paramètre d'un digramme quand la définition dit le nom C : `cart_start_game(button_t *nj)` (`src/game/cart.h`, définition `chosen_option`) et `score_panel_new_year(p8_fixed_t yr)` (`src/game/score_panel_display.h`, définition `year`). Non renommés à 4j (l'amendement de 4h ne vise que le prototype d'une définition renommée) ; sous (b) ou (c), ils prennent le nom de leur définition.
- **Verdict** : ok

### D-30 — `typed_main_*` des tests contre `main_*_typed`

- **Origine** : 2b (échantillon, « leurs doubles de test disent `typed_main_camera`… »).
- **Question** : `src/` dit `main_camera_typed`, `main_economy_typed`, `main_mouse_typed` ; cinq tests définissent des aides statiques `typed_main_camera` (`test_background.c`, `test_camera.c`, `test_ui.c`), `typed_main_economy` (`test_economy.c`, `test_probe.c`), `typed_main_mouse` (`test_ui.c`), qui affirment le singleton non NULL puis le rendent.
- **Options** : (a) `checked_main_camera`, `checked_main_economy`, `checked_main_mouse` (ce qu'elles font de plus) ; (b) garder.
- **Sites** : 96 — `typed_main_camera` 24, `typed_main_economy` 68, `typed_main_mouse` 4. Noms neufs : 0 site.
- **Proposition prête à appliquer** : (a), procédure N, une ligne par identité (`c:<fichier>.c@F@typed_main_X`, 6 identités).
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire ; contrôle sur `typed_main_`.
- **Verdict** : ok

### D-31 — Digramme `oc` : `tickets` et `type_picker`

- **Origine** : 2b (échantillon, « un digramme, deux noms »).
- **Question** : `oc` est devenu `tickets` (champ de `weighted_picker_t`, `gen.h`) et `type_picker` (`g_type_picker`, `planet_set_type_picker`, `planet.c`) ; le paramètre `oc` de `planet_set_type_picker` est resté (`conservé`).
- **Options** : (a) garder les deux noms (deux objets : le tirage pondéré et son tableau de tickets), renommer le paramètre `oc` → `picker` ; (b) garder tout.
- **Sites** : 3 pour le paramètre (`planet.h`, `planet.c:146–147`) ; `oc` au total 17 (le reste en commentaire et dans le libellé `jo.oc` de `test_live_slots.c`, que 4f reprend).
- **Proposition prête à appliquer** : (a), ligne `field planet_set_type_picker.oc	picker` de `tools/rename_overrides.tsv` et procédure N (identité du paramètre à relever : offset).
- **Portée** : s'applique aussi aux mentions en commentaire.
- **Verdict** : ok

### D-32 — Champs de l'icône de structure : `rest_count` est une position

- **Origine** : 2b (échantillon : `pico8_ie_gy`, `pico8_ie_eh` → `structure_icon_cursor`, `structure_icon_rest_cursor`, mais les champs `gy`, `eh` → `revealed_count`, `rest_count`).
- **Question** : le champ `rest_count` de `structure_icon_archetype_t` est un `vec_t`, la position de repos du curseur que rend `structure_icon_rest_cursor()` ; le test dit `shared_revealed_count` et `revealed_count_before` pour le curseur partagé ; le paramètre `gy` de `structure_icon_set_cursor` est resté.
- **Options** : (a) aligner sur « cursor » ; (b) garder.
- **Sites** : 22 — `rest_count` 10, `shared_revealed_count` 8, `revealed_count_before` 4 ; plus le paramètre `gy` (`structure_icon.h:81`, `structure_icon.c:274–275`).
- **Proposition prête à appliquer** : (a), procédure N :

  ```
  rest_count	rest_cursor	src/game/game_data.h	c:@SA@structure_icon_archetype_t@FI@rest_count
  shared_revealed_count	shared_cursor	tests/test_structure_icon.c	c:test_structure_icon.c@F@shared_revealed_count
  ```

  et, identités à relever (offset), `revealed_count_before` → `cursor_before`, paramètre `gy` → `cursor` (ligne `field structure_icon_set_cursor.gy	cursor`). Noms neufs : 0 site.
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire ; contrôle sur `rest_count`, `revealed_count`.
- **Verdict** : ok

### D-33 — Exceptions du compteur de français : schéma du TSV et fixtures françaises (3a)

- **Origine** : 4a (relevé de fin, `make french-count SUBSTEP=3a`) ; suite de D-28, puce *Schéma du TSV en français*.
- **Question** : les 26 mentions `string` `done` qui restent sur 3a sont du français par construction : les noms de colonnes et les statuts du TSV (`catégorie`, `identité`, `renommé`, `conservé`, `validé`) dans deux outils et leurs mini-TSV de test, et les mots français que `tests/test_cart_citations.py` plante exprès (`le`, compté comme homonyme d'un nom ; `dès`, accent au milieu d'un mot). Les excepter ?
- **Options** : (a) excepter les 14 triplets ci-dessous ; (b) traduire le schéma du TSV (D-28), ce qui fait tomber les 12 lignes de schéma et laisse `le` et `dès`.
- **Sites** : 26 (`tests/test_cart_citations` 18, `tests/test_scope_count` 6, `tools/cart_citations` 1, `tools/scope_count` 1) ; `SITES=1` les liste.
- **Proposition prête à appliquer** : (a), ces lignes ajoutées à `tools/french_exceptions.txt` (séparateur : tabulation) :

  ```
  string	tests/test_cart_citations	catégorie
  string	tests/test_cart_citations	identité
  string	tests/test_cart_citations	renommé
  string	tests/test_cart_citations	conservé
  string	tests/test_cart_citations	validé
  string	tests/test_cart_citations	le
  string	tests/test_cart_citations	dès
  string	tests/test_scope_count	catégorie
  string	tests/test_scope_count	identité
  string	tests/test_scope_count	renommé
  string	tests/test_scope_count	conservé
  string	tests/test_scope_count	validé
  string	tools/cart_citations	renommé
  string	tools/scope_count	renommé
  ```

  Contrôle (fait par 4a sur une copie du fichier) : `make french-count SUBSTEP=3a` à `TOTAL done` 0 et `TOTAL later` 0. Les sous-étapes suivantes proposent leurs propres lignes ; celles des outils du dictionnaire ne sont pas comptées (exclus par construction).
- **Complément (4l4)** : à la fin de l'étape 4, sans `SUBSTEP`, `french-count` `string` 36 = 26 (cette entrée) + 1 (D-55) + 8 (D-59) + 1 (D-61) ; 0 avec les 17 lignes de ces quatre entrées, vérifié sur une copie de `tools/french_exceptions.txt` (`tmp/4l4_s3/fe_copy.txt`).
- **Verdict** : ok

### D-34 — Assertion faible `strstr(log, "exempt")` de `test_class_coverage.c`

- **Origine** : 4a (commit de chaînes, lecteurs de `tools/class_coverage.sh`).
- **Question** : `test_exempting_every_zero_class_passes` affirme `strstr(log, "exempt") != NULL`, qui apparie aussi la ligne de verdict (`coverage: OK -- every non-exempt class is instantiated …`) : elle passe même si aucune ligne ne porte d'exemption. Faiblesse héritée telle quelle (l'ancien `"exemptée"` appariait aussi « non exemptée ») ; l'étape 4 ne change pas une condition d'assertion.
- **Options** : (a) resserrer sur la forme d'une ligne exemptée ; (b) garder.
- **Sites** : 1 (`tests/test_class_coverage.c:69`).
- **Proposition prête à appliquer** : (a), `strstr(log, "  exempt (test)") != NULL` (la raison `test` est celle qu'écrit l'`awk` du même cas), commit `test:` ; contrôle : `make test` vert.
- **Verdict** : ok

### D-35 — Noms d'identifiant qui citent Lua ou le cart (3b)

- **Origine** : 4b (commit de commentaires ; le relevé des citations ne lit que les commentaires).
- **Question** : quatre identifiants de 3b citent encore Lua ou le cart : le paramètre `lua` de la macro `P8_FIXED_LIT(raw, lua)` (`src/p8/fixed.h`, le littéral décimal, jamais évalué), et trois fonctions de test, `test_mod_negative_dividend_matches_lua_floor_semantics` (`tests/test_fixed.c`), `test_sget_reads_known_pixel_from_loaded_cart_data` et `test_map_and_mset_round_trip_against_real_cart_data` (`tests/test_gfx.c`). Les renommer ?
- **Options** : (a) renommer ; (b) garder (le §2 vise les commentaires et le code ; ici « cart » désigne les données de `pico8/`).
- **Sites** : 8 (paramètre : 1, la liste de paramètres, le corps ne l'emploie pas ; fonctions : 7, définitions, appels de `main` et une mention en commentaire de `test_gfx.c`).
- **Proposition prête à appliquer** : (a). Paramètre : `lua` → `decimal`, édition à la main de la ligne de `#define` (paramètre de macro, hors extraction), commit `refactor:` ; sortie du préprocesseur des appelants inchangée (le paramètre n'est pas utilisé). Fonctions, procédure N : `test_mod_negative_dividend_matches_lua_floor_semantics	test_mod_negative_dividend_floors	tests/test_fixed.c	c:test_fixed.c@F@test_mod_negative_dividend_matches_lua_floor_semantics`, `test_sget_reads_known_pixel_from_loaded_cart_data	test_sget_reads_known_pixel_from_loaded_sheet	tests/test_gfx.c	c:test_gfx.c@F@test_sget_reads_known_pixel_from_loaded_cart_data`, `test_map_and_mset_round_trip_against_real_cart_data	test_map_and_mset_round_trip_against_real_sheet	tests/test_gfx.c	c:test_gfx.c@F@test_map_and_mset_round_trip_against_real_cart_data` (noms neufs : 0 site).
- **Portée** : s'applique aussi aux mentions en commentaire ; contrôle : `grep -rnwiE '[a-z_]*(lua|cart)[a-z_]*' src/p8 tests/test_fixed.c tests/test_gfx.c` ne rend que `cart_draw`.
- **Complément (4g)** : quatre identifiants de test de 3g citent le cart : `cart_positions_items`, `CART_POSITIONS`, `cart_routed_positions_items`, `CART_ROUTED_POSITIONS` (`tests/test_research_project.c`, 8 sites, 2 chacun : définition et emploi), copies figées des listes de noms de positions que seul `DEFAULTS_HASH` plie (le commentaire réécrit par 4g dit « frozen copies », sans citer le cart). Sous (a) : `frozen_positions_items`, `FROZEN_POSITIONS`, `frozen_routed_positions_items`, `FROZEN_ROUTED_POSITIONS` (0 site), procédure N ; renommer n'en change pas le pliage (seules les chaînes des listes sont pliées, `DEFAULTS_HASH` inchangée).
- **Verdict** : ok

### D-36 — `` `mem` `` et `` `gfx` `` comptés en `backtick` après le renommage de 4b

- **Origine** : 4b (commit de locales).
- **Question** : `mem` → `memory` et `gfx` → `graphics` (locales de `main` de `tests/test_demo.c`) font de `mem` et `gfx` d'anciens noms ambigus du TSV ; `tools/rename_count.py` compte alors toute mention entre backticks égale au nom, partout, même quand elle désigne un paramètre `mem` ou `gfx` conservé d'une autre fonction. 3b a réécrit ses 3 mentions ; 23 restent dans des en-têtes de sous-étapes suivantes, qu'elles devront réécrire sans backticks pour atteindre 0. Faut-il plutôt changer la règle du compteur ?
- **Options** : (a) les sous-étapes suivantes réécrivent (« the memory », « the graphics state ») ; (b) règle du compteur : un nom ambigu dont toutes les lignes `renommé` sont des locales ou des paramètres ne compte en `backtick` que dans le fichier de ces lignes (test d'abord dans `tests/test_rename_count.py`) ; (c) exceptions par module.
- **Sites** : 23 : `src/game/message_box.h` 4, `src/game/ui.h` 3, `background.h`, `draw_functions.h`, `probe.h`, `slipway_display.h`, `structure_icon.h`, `wormhole_display.h` 2 chacun, `hud.h`, `mouse.h`, `scroll.h`, `src/devtools/screencast.h` 1 chacun (``grep -rnoE '`(mem|gfx)`' src tests tools``).
- **Proposition prête à appliquer** : (b), qui vaut aussi pour les renommages de locales des 4x suivantes ; contrôle : `make cart-names-count` sans `SUBSTEP` rend 26 `backtick` de moins qu'avant la règle, et `make rename-count-test` vert.
- **Complément (4e)** : un renommage de locale de l'étape 4 a le même effet. 4e renomme `ctx` → `filter_context` (`planet_inspection_panel`) : `ctx`, nom courant d'une ligne `conservé`, devient un ancien nom ambigu, et les mentions `` `ctx` `` de `src/engine/collect.h:7` (3c) et `src/game/galaxy.c:16` (3d), sous-étapes closes, se mettent à compter en `later` ; 4e les a réécrites (« the context pointer »). La règle (b) l'aurait évité.
- **Complément (4f1)** : un renommage de locale rend aussi compté un mot français de deux lettres. 4f1 renomme `un` → `first_refusal` (`refused_slipways_do_not_grow_game`, `refused_wormholes_do_not_grow_game`) : `un` devient l'ancien nom d'une ligne `renommé` de deux lettres, et `tools/cart_citations.py` compte en `short` chaque « un » nu d'un commentaire ou d'un message, donc chaque article français restant. Sans `SUBSTEP`, sur le même arbre, TSV d'avant puis d'après : `short` 1 458 → 1 693 (+235), dont +20 dans `tests/test_live_slots.c` (réécrits par 4f1) et le reste dans des sous-étapes suivantes seulement (`tools/composition_residue` 25, `Makefile` 19, `tests/test_screencast_player` 19, `devtools/screencast` 17, `tests/test_headless_capture` 13, `tests/test_screencast` 12…, et `game/particle_zone`, `tests/test_particle_zone` de 4f2, `game/ui_zone`, `tests/test_ui_zone` de 4h, `game/cart` de 4j, `cmd/replay` de 4l3) ; `cart-citations SUBSTEP=3a` à `3e` restent 0/0/0. Ces mots, que `french-count` compte déjà, partiront avec le français de leurs sous-étapes : la charge ne change pas, seule leur mesure de départ de `cart-citations` monte. La règle (b), étendue à `tools/cart_citations.py` (un ancien nom dont toutes les lignes `renommé` sont des locales ou des paramètres ne compte en `short` que dans le fichier de ces lignes), l'aurait évité.
- **Complément (4g)** : 4g renomme `cap`, `out`, `len` (`research_project_tooltip_join`, → `capacity`, `names`, `length`) : mots anglais courants de trois lettres, ils deviennent d'anciens noms ambigus. Sans `SUBSTEP`, sur l'arbre de l'état des locales extrait, TSV d'avant puis d'après : `backtick` 1 894 → 1 904 (+10) ; `cart-citations` identique. Mentions : `src/game/layout.h:136` (`` `cap` ``, 3d, close), réécrite par 4g (« as many as the last argument ») ; `economy.h:45`, `economy_formulas.h:30` (`` `out` ``, 3g), réécrites ; `src/game/score_panel.c:37` (`` `cap` ``, 3j, +1) ; `src/devtools/screencast_player.h:42`, `dump.h:8`, `replay_control.h:63`, `minimize.h:6`, `tests/test_screencast_player.c:126` (`` `out` ``), `tests/test_gif.c:64` (`` `len` ``) (3k, +6), laissées à leur sous-étape, dont la mesure de départ monte d'autant. La règle (b) l'aurait évité.
- **Complément (4h)** : 4h renomme deux locales `row` de `tests/test_button.c` (`fold_written` → `layout_row`, `fold_keys` → `source_row`, portées 44 et 79). Sans `SUBSTEP`, sur l'arbre de l'état des locales extrait, TSV d'avant puis d'après : `backtick` 1 560 → 1 570 (+10) ; `cart-citations` identique (66/20/1 289). Mentions : `src/game/layout.h:40`, `:128`, `:140`, `:150`, `:152`, `src/game/map_tables.h:34`, `src/game/game_data.h:103` (3d, close, +7 `later`) et `src/game/research.h:12` (3g, close, +1 `later`), réécrites par 4h sans backticks et à hauteur constante (« the row field », « the row argument », « the given row ») ; `src/game/score_panel.h:53`, `:66` (3j, +2), laissées à leur sous-étape. Un mot anglais de trois lettres qui nomme aussi des champs et des paramètres légitimes (`row` de `data_row_t`, de `layout_row_cell_t`) compte autant que la locale renommée. La règle (b) l'aurait évité.
- **Complément (4k1)** : 4k1 réécrit la mention `` `mem` `` de `src/devtools/screencast.h:45` (« the PICO-8 memory ») ; elle était l'une des 23 du relevé de 4b.
- **Complément (4k2)** : 4k2 réécrit les mentions `` `out` `` de `src/devtools/screencast_player.h:42`, `src/devtools/dump.h:8`, `tests/test_screencast_player.c:126` (« the buffer », « the stream », « the 16-byte buffer ») et `` `len` `` de `tests/test_gif.c:64` (`` `data_length` ``, nom de la locale après le commit de locales de 4k2) : 4 des 6 de 3k relevées par 4g ; restent `src/devtools/replay_control.h:63` et `src/devtools/minimize.h:6` (4k3). Les « un » de 4k2 (38) sont partis avec le français. Les locales renommées par 4k2 (`eoi`, `prev`, `clears`, `bitpos`, `written`, `packed`) deviennent d'anciens noms sans aucune mention entre backticks dans le dépôt ; `gif` garde son ancien `g` (D-42) : aucun effet de bord mesuré (compteurs sans `SUBSTEP`, TSV d'avant puis d'après, identiques à l'octet).
- **Complément (4k3)** : 4k3 réécrit les deux dernières mentions `` `out` `` de 3k relevées par 4g : `src/devtools/replay_control.h:63` (« the buffer ») et `src/devtools/minimize.h:6` (`` `kept_frames` ``, nom du paramètre après le commit de locales de 4k3) ; les six de 3k sont réécrites. Les « un » de 4k3 (11) sont partis avec le français. Les locales renommées par 4k3 (`obj_start`, `obj_end`, `obj_len`, `obj_buf`, `tmp_len`) deviennent d'anciens noms distinctifs (un `_`), comptés par texte en mot entier, nus compris : seule la mention nue `tmp_len` du commentaire `src/devtools/input_json.c:229` comptait (+1 `comment` sans `SUBSTEP`, TSV d'avant puis d'après, sur l'arbre de l'état des locales ; réécrite `tmp_path_size` par le commit de commentaires, à hauteur constante). `out` était déjà un ancien nom : aucun effet.
- **Complément (4l1)** : les 19 « un » du `Makefile` rendus comptés par 4f1 sont partis avec le français de sa légende et de ses commentaires (`cart-citations MODULES=Makefile` : `short` 152 → 0).
- **Verdict** : ok

### D-37 — `argc`/`argv` de `main` dans la liste des locales cryptiques

- **Origine** : 4c1 (mesure de `make cryptic-locals MODULES=<4c1>`).
- **Question** : la liste des locales cryptiques (portée > 40 lignes, nom de trois caractères ou moins ou non découpé en mots) relève `argc` et `argv`, l'idiome C des paramètres de `main`, dès que `main` dépasse 40 lignes. Les renommer, ou sortir l'idiome de la règle ?
- **Options** : (a) règle de `tools/scope_count.py --cryptic` : un paramètre `argc` ou `argv` d'une fonction `main` n'est pas cryptique (test d'abord dans `tests/test_scope_count.py`) ; (b) renommer (`arg_count`/`args`) par la procédure N ; (c) exceptions par module.
- **Sites** : 10 déclarations, 5 `main` : `tests/test_allocator` (43 lignes), `cmd/class_coverage` (58), `cmd/game` (95), `cmd/replay` (94), `cmd/screencast_viewer` (102) ; `make cryptic-locals` sans `SUBSTEP`, après `rename-extract` frais à la fin de 4c1.
- **Proposition prête à appliquer** : (a). Contrôle : `make cryptic-locals` sans `SUBSTEP` rend 10 candidats de moins, `make scope-count-test` vert.
- **Portée** : la règle ne touche que la liste des locales cryptiques ; aucune mention en commentaire ni entre backticks n'est en cause.
- **Complément (4l3)** : après 4l3, `cryptic-locals` garde les 8 `argc`/`argv` des `main` de `cmd/class_coverage` (58), `cmd/game` (95), `cmd/replay` (94), `cmd/screencast_viewer` (100 ; 102 avant ses commentaires) et les 2 de `tests/test_allocator.c` ; inchangé.
- **Complément (4l4)** : remesuré à la fin de l'étape 4, sans `SUBSTEP` : les 10 `argc`/`argv` des cinq `main`, avec `blank` et `screen_texture` (D-60), 12 candidats ; inchangé.
- **Verdict** : ok

### D-38 — Statiques de fichier `mem`/`gfx` des tests

- **Origine** : 4c1 (`tests/test_exit_queue.c` en déclare).
- **Question** : 40 tests déclarent `static p8_memory_t mem;` et `static p8_graphics_t gfx;` (80 déclarations), `conservé` au TSV. Le §4 veut pour une globale un nom complet, et `g_` pour une statique mutable de fichier ; 4b a renommé les locales homonymes de `test_demo.c` en `memory`/`graphics`, et `tests/test_particle_zone.c` porte déjà `g_mem`/`g_gfx`. Les renommer ?
- **Options** : (a) garder ; (b) `memory`/`graphics` ; (c) `g_memory`/`g_graphics` (§4 à la lettre).
- **Sites** : 80 déclarations dans 40 fichiers (`grep -rhE '^static p8_(memory|graphics)_t (mem|gfx);' tests | wc -l`), plus leurs emplois, à mesurer par le `grep` des conventions sur chaque fichier.
- **Proposition prête à appliquer** : (b), procédure N, en un passage à l'étape 5 ; avec D-36 (b), les mentions entre backticks ne comptent pas hors de leur fichier.
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire des 40 fichiers ; contrôle : le `grep` des sites ci-dessus rend 0.
- **Verdict** : ok

### D-39 — Nom de test qui cite le cart (`test_data_init.c`)

- **Origine** : 4d (commit de commentaires ; le relevé des citations ne lit que les commentaires et les messages).
- **Question** : `test_production_tables_project_records_icon_tooltips_technologies_decode_real_cart_content` (`tests/test_data_init.c`) cite encore le cart dans un identifiant, même question que D-35. Le renommer ?
- **Options** : (a) renommer ; (b) garder (« cart » y désigne les tables du jeu).
- **Sites** : 2 (définition, appel de `main`).
- **Proposition prête à appliquer** : (a), procédure N : `test_production_tables_project_records_icon_tooltips_technologies_decode_real_cart_content	test_production_tables_project_records_icon_tooltips_technologies_decode_real_game_data	tests/test_data_init.c	c:test_data_init.c@F@test_production_tables_project_records_icon_tooltips_technologies_decode_real_cart_content` (nom neuf : 0 site).
- **Portée** : s'applique aussi aux mentions en commentaire (aucune) ; contrôle : `grep -rnwiE '[a-z_]*(lua|cart)[a-z_]*' tests/test_data_init.c` rend 0.
- **Verdict** : ok

### D-40 — Clés du cart sans ligne `chaîne` au TSV

- **Origine** : 4d (commit de chaînes).
- **Question** : les clés de dispatch des fonctions de dessin `"br"`, `"qr"`, `"rf"`, `"s"` (clés de la table de dispatch du cart, `fn="…"` de ses rangées) et la clé de production `"qh+"` (composé de `"qh"`, renommé `"structure"` par 3a) n'avaient pas de ligne `chaîne` au TSV : 2b ne les avait pas extraites (clés de deux lettres hors du TSV ; composé non apparié par la règle « la chaîne est exactement le nom »). 4d les a traduites (`"draw_bar"`, `"print_outlined"`, `"draw_help_frame"`, `"spr"`, `"structure+"`) selon la règle de propriété des chaînes, mais la garde anti-cart ne verrait pas une telle clé revenir. Faut-il outiller ce contrôle ?
- **Options** : (a) rien, le contrôle reste humain (règle de tri du point 6 de la consigne de l'étape 4, amendement de 4d, supprimée par 4l4 : `git show dd1d875:docs/consigne-etape-4.md` ; une chaîne de table se trie par son consommateur, et une clé du cart — clé minifiée ou composé d'une telle clé, qui ne sert qu'à un `strcmp`, une recherche ou un dispatch — se traduit par son écrivain à tous ses sites) ; (b) ajouter à l'étape 5 des lignes `chaîne` par `rename-strings` (outil, test d'abord), pour que la garde les compte ; (c) un relevé ponctuel à l'étape 5 des `fn="…"` de `pico8/lua.lua` contre les clés de `draw_functions_init`.
- **Sites** : 0 après 4d (`grep -rn '"br"\|"qr"\|"rf"\|"qh+"\|draw_function = "s"\|dispatch_lookup("s")\|register("s"' src tests`).
- **Proposition prête à appliquer** : (a).
- **Portée** : aucune mention en commentaire ni entre backticks n'est en cause.
- **Verdict** : ok

### D-41 — Nom de test qui garde le digramme `r` (`test_map_tables.c`)

- **Origine** : 4d (commit de commentaires, relecture de `tests/test_map_tables.c`).
- **Question** : `test_weighted_picker_pools_planet_types_entries_by_their_r` garde en suffixe `r`, le champ du cart devenu `weight` ; le TSV l'a appliqué tel quel (`renommé`, depuis `test_gn_pools_df_entries_by_their_r`). Aucun compteur ne le voit (le nom entier n'est pas un ancien nom). Le renommer ?
- **Options** : (a) renommer ; (b) garder.
- **Sites** : 2 (définition, appel de `main`).
- **Proposition prête à appliquer** : (a), procédure N : `test_weighted_picker_pools_planet_types_entries_by_their_r	test_weighted_picker_pools_planet_types_entries_by_their_weight	tests/test_map_tables.c	c:test_map_tables.c@F@test_weighted_picker_pools_planet_types_entries_by_their_r` (nom neuf : 0 site) ; la ligne de `docs/renommage.tsv` passe au nom neuf.
- **Portée** : s'applique aussi aux mentions en commentaire (aucune) ; contrôle : `grep -rn 'by_their_r\b' src tests tools` rend 0.
- **Verdict** : ok

### D-42 — Ligne du TSV d'une locale renommée deux fois

- **Origine** : 4e (commit de locales).
- **Question** : 14 locales et paramètres déjà renommés à l'étape 3 (`p` → `pos` ×2, `ra` → `pos`, `px` → `name`, `py` → `speed_factor`, `qe_is_r_q` → `resource_is_r_q`, `qu` → `start` ×2, `re` → `end` ×4, `rb` → `rows`, `prise` → `grab_arena`) l'ont été de nouveau à 4e (`screen_pos`, `planet_pos`, `screen_x`, `screen_y`, `resource_is_any`, `start_node`, `end_node`, `end_planet`, `end_before_drop`, `other_end`, `used_before`). Leur ligne de `docs/renommage.tsv` a pris le nom neuf et l'identité courante mais garde l'`ancien` du cart (et le `concept`), si bien que `rename-table-check` les signale d'une sorte neuve (« row c:…@planet_draw@pos: says p in …, the extraction pos in … ») et que `rename_derive.py` ne les reproduit qu'hors des colonnes `ancien` et `concept` (il prend pour ancien le nom courant). Quelle forme de ligne garder ?
- **Options** : (a) garder l'`ancien` du cart : le pont inverse vers le Lua reste intact (§2 du plan), et le contrôle est rouge par construction depuis l'étape 3 ; (b) chaîner (ancien = nom courant, `pos` → `screen_pos`) : `rename_derive.py` à l'octet, mais `pos`, `name`, `rows`, `start`, `end`… deviennent d'anciens noms ambigus, et 19 mentions entre backticks de ces mots compteraient partout (`` `pos` `` 6, `` `name` `` 6, `` `rows` `` 7, mesurées dans le rejeu de la PRD) ; (c) apprendre à `tools/rename_check.py` qu'une ligne `renommé` dont l'identité est celle de l'extraction et dont le nom extrait est son `nouveau` d'une étape précédente n'est pas en défaut (test d'abord dans `tests/test_rename_check.py`).
- **Sites** : 14 lignes ; `rename-table-check` sur l'extraction de l'état des chaînes de 4e : 27 708 → 27 638 (84 violations des 42 lignes tombent, ces 14 apparaissent).
- **Proposition prête à appliquer** : (a) ; (c) à l'étape 5 si la garde de l'étape 5 lit `rename-table-check`. Contrôle de (c) : `make rename-check-test` vert, `make rename-table-check` rend 14 violations de moins.
- **Portée** : aucune mention en commentaire ni entre backticks n'est en cause sous (a) ; sous (b), les 19 mentions sont à réécrire.
- **Complément (4f1, 4f2)** : 4f1 ajoute 3 lignes de cette forme (`prise` → `grab_arena` → `used_before`/`used`), 4f2 en ajoute 8 (`ra`/`p` → `pos` → `screen_pos` ×3, `px` → `name` → `screen_x`, `py` → `speed_factor` → `screen_y`, `prise` → `grab_arena` → `used`, `prise_rendue` → `grab_arena_released` → `released_used`, `em` → `spacer` → `emitter`) : 25 lignes en tout. `rename-table-check` sur l'extraction de l'état des chaînes de 4f2 : 27 708 → 27 670.
- **Complément (4g)** : 4g ajoute 1 ligne de cette forme (`ra` → `pos` → `icon_pos`, `test_directed` de `tests/test_structure_icon.c`) : 26 lignes en tout. `rename-table-check` sur l'extraction de l'état des chaînes de 4g : 27 708 → 27 679 (30 violations tombent, celle-ci apparaît).
- **Complément (4h)** : 4h ajoute 3 lignes de cette forme (`ra` → `pos` → `mouse_screen`, `ui_send_to_hovered` de `src/game/ui.c` ; `p` → `pos` → `draw_pos`, `button_draw` de `src/game/ui_button.c` et son prototype de `ui_button.h`) : 29 lignes en tout. `rename-table-check` sur l'extraction de l'état des chaînes de 4h : 27 708 → 27 671 (40 violations tombent, ces 3 apparaissent).
- **Complément (4i)** : 4i ajoute 6 lignes de cette forme (`p` → `pos` → `screen_pos` ×2, `px` → `name` → `screen_x`, `py` → `speed_factor` → `screen_y`, `burst_draw` de `src/game/planet_family.c` et son prototype de `planet_family.h` ; `ra` → `pos` → `planet_pos`, `test_burst_export_line_counts_the_outgoing_routes` de `tests/test_burst_exports_row.c` ; `p` → `pos` → `map_pos`, `test_life` de `tests/test_protostar.c`) : 35 lignes en tout. `rename-table-check` sur l'extraction de l'état des chaînes de 4i : 27 708 → 27 682 (32 violations tombent, ces 6 apparaissent).
- **Complément (4j)** : 4j ajoute 3 lignes de cette forme (`tl` → `era` → `current_era`, `menu_show_tech_tree_screen` de `src/game/menu.c` ; `ra` → `pos` → `box_pos`, `message_box_open_box` de `src/game/message_box.c` ; `em` → `spacer` → `emitter`, `test_built` de `tests/test_help_screen.c`) : 38 lignes en tout. `rename-table-check` sur l'extraction de l'état des chaînes de 4j : 27 708 → 27 685 (26 violations tombent, ces 3 apparaissent).
- **Complément (4k2)** : 4k2 ajoute 1 ligne de cette forme (`g` → `gif` → `result`, `parse_gif` de `tests/test_gif.c`) : 39 lignes en tout. `rename-table-check` sur l'extraction de l'état des chaînes de 4k2 : 27 714 → 27 697 (18 violations tombent, celle-ci apparaît).
- **Verdict** : ok

### D-43 — Paramètres `gfx`/`mem` des fonctions de dessin

- **Origine** : 4e (commit de locales), après 4b et 4c2.
- **Question** : 4b, 4c2 et 4e renomment en `graphics`/`memory` les seules déclarations `gfx`/`mem` à longue portée (`planet_draw`, `slipway_draw`, `main` de `test_demo.c`…) ; les autres, dont les fonctions de dessin voisines (`slipway_draw_line`, `hud.c`, `layout.c`…), restent à portée ≤ 40 lignes, permises par le §1, et `gfx`/`mem` sont désormais d'anciens noms ambigus (D-36). Le code mêle donc les deux formes d'une fonction à l'autre. Tout renommer ?
- **Options** : (a) garder (§1) ; (b) tout renommer en `graphics`/`memory` par la procédure N, avec D-38 (statiques de fichier des tests).
- **Sites** : `grep -rnE 'p8_graphics_t \*gfx\b' src | wc -l` : 100 à `fdf8844`, 96 après 4e ; `grep -rnE 'p8_memory_t \*mem\b' src | wc -l` : 132 à `fdf8844`, 128 après 4e (prototypes compris).
- **Proposition prête à appliquer** : (a), à revoir avec D-36 et D-38.
- **Portée** : sous (b), s'applique aussi aux mentions entre backticks et en commentaire ; contrôle : les deux `grep` ci-dessus rendent 0.
- **Complément (4f2)** : après 4f2, `grep -rnE 'p8_graphics_t \*gfx\b' src | wc -l` 92, `'p8_memory_t \*mem\b'` 124 (`emitter_update` et `wormhole_draw` renommés, prototypes compris) ; restent à courte portée, dans les fichiers de 4f2, `targeting_tooltip` et `targeting_tooltip_show`.
- **Complément (4h)** : après 4h, `grep -rnE 'p8_graphics_t \*gfx\b' src | wc -l` 90, `'p8_memory_t \*mem\b'` 118 (`button_draw`, `background_construct`, `ui_update_hover` renommés, prototypes compris) ; restent à courte portée, dans les fichiers de 4h, `ui_update`, `ui_draw_overlay`, `button_draw_overlay`, `background_draw`, `camera_apply`, leurs prototypes et la statique `mouse_construct`.
- **Complément (4i)** : après 4i, `grep -rnE 'p8_graphics_t \*gfx\b' src | wc -l` 88, `'p8_memory_t \*mem\b'` 116 (`burst_draw` et son prototype renommés) ; restent à courte portée, dans les fichiers de 4i, `ascension_draw` et son prototype, et les paramètres de `setup_panel_state` (`tests/test_burst_exports_row.c`, portée 13).
- **Complément (4j)** : après 4j, `grep -rnE 'p8_graphics_t \*gfx\b' src | wc -l` 88, `'p8_memory_t \*mem\b'` 115 (la locale `mem` de `cart_load` renommée `memory`) ; restent à courte portée, dans les fichiers de 4j, `message_box_draw_overlay`, `toast_draw`, `help_screen_draw`, `score_panel_draw` et leurs prototypes, et les locales `mem` de `cart_init`, `cart_draw`, `score_panel_refresh_palette`.
- **Complément (4k2)** : après 4k2, `grep -rnE 'p8_graphics_t \*gfx\b' src | wc -l` 88, `'p8_memory_t \*mem\b'` 115, inchangés ; restent à courte portée, dans les fichiers de 4k2, `dump_screen` et son prototype, et les locales `mem`/`gfx` de `test_dump_of_hand_drawn_pattern_matches_exact_text` (`tests/test_dump.c`, portées 36 et 35).
- **Complément (4l3)** : les locales `mem`/`gfx` des `main` de `src/cmd/replay.c`, `src/cmd/game.c` et `src/cmd/class_coverage.c` sont renommées `memory`/`graphics` (des structs, hors du `grep` de l'entrée) : 88 et 115 inchangés ; reste à courte portée, dans les fichiers de 4l3, le paramètre `mem` de `raylib_backend_present` et de son prototype.
- **Verdict** : ok

### D-44 — Champ `planet_inspection_panel_ctx_t.unmet_needs`, qui tient les besoins reçus

- **Origine** : 4e (commit de commentaires, réécriture du commentaire de `planet_inspection_panel_filter_step`).
- **Question** : dans `src/game/planet_display.c`, le filtre garde les besoins que rien ne satisfait (la locale `unmet_needs` de `planet_inspection_panel`, rangée « needs: ») et ajoute chaque besoin satisfait, une fois par unité, à la liste du contexte, qui fait la rangée « receives: ». Ce champ du contexte s'appelle aussi `unmet_needs` (`ji` → `unmet_needs`, 3e, comme la locale `jx`) : il dit le contraire de ce qu'il tient. Aucun compteur ne le voit. Le renommer ?
- **Options** : (a) renommer le champ `received_needs` ; (b) garder.
- **Sites** : 7 lignes de `src/game/planet_display.c` (déclaration ligne 88, emplois 107, 196, 236, 237, 240, 241) ; l'exemption `planet_inspection_panel_ctx_t.unmet_needs` de `REGISTRES_NON_ENTITE` (`tools/composition_residue.sh:128`) et les deux lignes `chaîne` de `docs/renommage.tsv` qui la citent (écrivain `planet_display.c`, lecteur `tools/composition_residue.sh`).
- **Proposition prête à appliquer** : (a), procédure N : `unmet_needs	received_needs	src/game/planet_display.c	c:@SA@planet_inspection_panel_ctx_t@FI@unmet_needs` (identité stable, relevée dans `declarations.tsv` à la fin de 4e), ligne de `tools/rename_overrides.tsv` `field	pico8_jo_gh_ctx_t.ji	received_needs	gh` ; l'exemption et les lignes `chaîne` suivent par la procédure G (`--detail` avant et après, vivacité). La locale `unmet_needs` de `planet_inspection_panel` garde son nom.
- **Portée** : s'applique aussi aux mentions en commentaire (aucune après 4e) ; contrôle : `grep -n 'ctx->unmet_needs\|filter_context.unmet_needs\|ctx_t.unmet_needs' src tools` rend 0.
- **Verdict** : ok

### D-45 — Noms de test qui disent « grab arena » pour une prise (`test_live_slots.c`)

- **Origine** : 4f1 (commit de commentaires, relecture des cas de prise de `tests/test_live_slots.c`).
- **Question** : dans `tests/test_live_slots.c`, « prise » désignait le geste (une structure tirée de son icône et tenue, puis posée ou refusée), pas l'arène où elle naît. L'étape 3 a suivi le concept du vocabulaire `prise` → `grab arena` : `refused_grab_arenas_do_not_grow_game` (ancien `prises_refusees_ne_font_pas_grandir_la_partie`), `placed_grab_arena` (`la_prise_posee`, qui rend la planète posée), `placed_grab_arena_reuses_dead_wormhole_arena` (`une_prise_posee_reprend_l_arene_du_mz_mort`), `refused_grab_arena_crosses_new_game` (`une_prise_refusee_traverse_une_nouvelle_partie`), `offscreen_planet_forgets_refused_grab_arena` (`le_fo_d_une_planete_hors_ecran_oublie_la_prise_refusee`), la macro `GRAB_ARENAS` (`PRISES`) et la fixture `tests/fixtures/grab_arenas_refused.json` (4f2). Les messages et libellés réécrits par 4f1 disent déjà « grab » (`refused grabs`, `placed grab`, « a refused grab leaves its structure in the generation »). Renommer ?
- **Options** : (a) `grab_arena` → `grab` dans ces cinq fonctions et la macro : `refused_grabs_do_not_grow_game`, `placed_grab`, `placed_grab_reuses_dead_wormhole_arena`, `refused_grab_crosses_new_game`, `offscreen_planet_forgets_refused_grab`, `GRABS` ; fixture `tests/fixtures/grabs_refused.json` (`git mv`, ligne `fichier` et procédure de 3l, avec sa mention dans `tools/rename_substeps.tsv` et la liste de 4f2 de `docs/plan-5.md`) ; (b) garder.
- **Sites** (``grep`` des conventions, fin de 4f1) : 2 par fonction (la définition et un appel : `main`, ou `placed_grab_arena_reuses_dead_wormhole_arena` pour `placed_grab_arena`), 10 ; macro 6 (définition, quatre `start_e2e`, l'en-tête de commentaire de sa fixture) ; fixture 1 en code (la valeur de `GRAB_ARENAS`), plus sa ligne de `tools/rename_substeps.tsv` et la liste de 4f2 du plan. Noms neufs : 0 site.
- **Proposition prête à appliquer** : (a), procédure N, identités relevées à la fin de 4f1 dans `declarations.tsv` : `refused_grab_arenas_do_not_grow_game	refused_grabs_do_not_grow_game	tests/test_live_slots.c	c:test_live_slots.c@F@refused_grab_arenas_do_not_grow_game`, `placed_grab_arena	placed_grab	tests/test_live_slots.c	c:test_live_slots.c@F@placed_grab_arena`, `placed_grab_arena_reuses_dead_wormhole_arena	placed_grab_reuses_dead_wormhole_arena	tests/test_live_slots.c	c:test_live_slots.c@F@placed_grab_arena_reuses_dead_wormhole_arena`, `refused_grab_arena_crosses_new_game	refused_grab_crosses_new_game	tests/test_live_slots.c	c:test_live_slots.c@F@refused_grab_arena_crosses_new_game`, `offscreen_planet_forgets_refused_grab_arena	offscreen_planet_forgets_refused_grab	tests/test_live_slots.c	c:test_live_slots.c@F@offscreen_planet_forgets_refused_grab_arena`, `GRAB_ARENAS	GRABS	tests/test_live_slots.c	macro:tests/test_live_slots.c:GRAB_ARENAS` ; les lignes de `docs/renommage.tsv` gardent leur `ancien` du cart (D-42).
- **Portée** : s'applique aussi aux mentions en commentaire (l'en-tête `GRAB_ARENAS:` de la fixture) ; contrôle : `grep -rnw 'grab_arena\|grab_arenas\|GRAB_ARENAS\|grab_arenas_refused' tests/test_live_slots.c` ne rend que le champ `placed->grab_arena` de la sonde, qui tient vraiment une arène.
- **Verdict** : ok

### D-46 — Fonctions de `main_economy.h` définies dans `probe.c`

- **Origine** : 4f2 (commit de commentaires ; 3f le notait pour les `main_economy*`).
- **Question** : `main_economy`, `main_economy_typed`, `main_economy_placeholder`, `main_economy_placeholder_class` et `probe_set_main_economy` sont déclarées dans `src/game/main_economy.h`, qui n'a pas de `.c`, et définies dans `src/game/probe.c` (avec l'état `g_main_economy` et la struct provisoire, que `probe_init` remet en place). `probe_set_main_economy` porte en plus le préfixe `probe_` dans un en-tête `main_economy` (§4 : module = nom du fichier). Que faire ?
- **Options** : (a) garder (§7 : pas de redécoupage de module pendant le plan 5) ; (b) renommer `probe_set_main_economy` → `main_economy_set`, procédure N ; (c) déplacer les définitions et leur état dans un `src/game/main_economy.c` (redécoupage, hors plan 5).
- **Sites** (`grep` des conventions, fin de 4f2) : `probe_set_main_economy` 3 (déclaration `main_economy.h:62`, définition `probe.c:490`, appel `cart.c:217`) ; `main_economy` 53 (mot entier, les `#include "game/main_economy.h"` compris), `main_economy_typed` 91, `main_economy_placeholder` 10, `main_economy_placeholder_class` 5. `main_economy_set` : 0 site.
- **Proposition prête à appliquer** : (a) ; (b) en complément si l'humain veut le §4 à la lettre : `probe_set_main_economy	main_economy_set	src/game/main_economy.h	c:@F@probe_set_main_economy`, ligne de `tools/rename_overrides.tsv` `name	pico8_probe_set_fm	main_economy_set` ; la ligne du TSV garde son `ancien` du cart (`pico8_probe_set_fm`).
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire ; contrôle sous (b) : le `grep` des conventions sur `probe_set_main_economy` rend 0.
- **Complément (4g)** : les commentaires de `main_economy.h` réécrits par 4g disent que ces fonctions et leur état sont définis dans `probe.c` ; `economy.h` nomme `probe_set_main_economy()` dans le contrat d'`economy_init`. Sites remesurés en fin de 4g (`grep` des conventions) : `probe_set_main_economy` 4 (la mention de `economy.h` en plus), `main_economy` 53, `main_economy_typed` 93, `main_economy_placeholder` 12, `main_economy_placeholder_class` 5.
- **Verdict** : ok

### D-47 — `structure_icon_sprite_counter` compte des identifiants de planète

- **Origine** : 4g (commit de commentaires, réécriture de `structure_icon.h` et `structure_icon.c`).
- **Question** : `pico8_ie_id` → `structure_icon_sprite_counter` (sens deviné à 2b) et ses statiques `g_sprite_counter`, `g_has_sprite_counter` (`src/game/structure_icon.c`) : la valeur, bumpée par `structure_icon_left_press`, compose l'identifiant `"s<n>"` de la structure posée (`keys.planet_id`) ; aucun sprite n'en dépend. Le test dit déjà `shared_id`, `shared_id_fixed`. Le nom ment ; aucun compteur ne le voit. Le renommer ?
- **Options** : (a) `structure_icon_id_counter`, `g_id_counter`, `g_has_id_counter` ; (b) garder.
- **Sites** (`grep` des conventions, fin de 4g) : `structure_icon_sprite_counter` 4 (déclaration, définition, appel de `tests/test_structure_icon.c`, mention en commentaire de `structure_icon.h`), `g_sprite_counter` 5, `g_has_sprite_counter` 5 ; noms neufs : 0 site.
- **Proposition prête à appliquer** : (a), procédure N, identités relevées dans `declarations.tsv` à la fin de 4g : `structure_icon_sprite_counter	structure_icon_id_counter	src/game/structure_icon.c	c:@F@structure_icon_sprite_counter`, `g_sprite_counter	g_id_counter	src/game/structure_icon.c	c:structure_icon.c@g_sprite_counter`, `g_has_sprite_counter	g_has_id_counter	src/game/structure_icon.c	c:structure_icon.c@g_has_sprite_counter` ; lignes de `tools/rename_overrides.tsv` `name	pico8_ie_id	structure_icon_id_counter	ie`, `name	g_ie_id	g_id_counter	ie`, `name	g_ie_has_id	g_has_id_counter	ie` (remplacent les lignes 766, 768, 769) ; les lignes du TSV gardent leur `ancien` du cart. Le message de `structure_icon_left_press` (« bumps the shared sprite counter », réécrit par 4g) devient « bumps the shared id counter », à hauteur constante (`structure_icon.c` est dans `tools/line_coverage_reference.txt`).
- **Portée** : s'applique aussi aux mentions en commentaire et dans les messages ; contrôle : le `grep` des conventions sur les trois anciens noms rend 0, et `git grep -n 'sprite counter' src tests` rend 0.
- **Verdict** : ok

### D-48 — Noms de test qui décrivent l'API générique disparue ou la méthode du cart

- **Origine** : 4h (commit de commentaires, relecture de `tests/test_ui.c`, `tests/test_main_mouse.c`, `tests/test_main_camera.c`).
- **Question** : `test_ui_held_stops_off_row` (`tests/test_ui.c` : une classe que l'ui ne tient pas fait avorter le clic ; « stops off row » parle de la ligne du lecteur de champs générique, disparu à 4f2), `test_probe_t_key_stops_off_row` (`tests/test_ui.c` : la clé `state_time` d'une sonde à zéro ; `t` est son digramme), `test_recompute_on_placeholder` (`tests/test_main_mouse.c`, `tests/test_main_camera.c` : « recompute » est la méthode `lj` du cart, `ui_update`) et les macros `RECOMPUTE_MOUSE_WORLD_X`/`_Y` des deux mêmes fichiers (la valeur figée de `mouse_world` après `ui_update`). Aucun compteur ne les voit (le nom entier n'est pas un ancien nom). Les renommer ?
- **Options** : (a) `test_ui_held_aborts_on_other_classes`, `test_probe_state_time_key_at_zero`, `test_update_on_placeholder`, `UPDATE_MOUSE_WORLD_X`/`_Y` ; (b) garder.
- **Sites** (`grep` des conventions, fin de 4h) : `test_ui_held_stops_off_row` 2 et `test_probe_t_key_stops_off_row` 2 (définition, appel de `main`) ; `test_recompute_on_placeholder` 4 (deux fichiers) ; `RECOMPUTE_MOUSE_WORLD_X` 4 et `RECOMPUTE_MOUSE_WORLD_Y` 4 (définition et emploi dans chacun des deux fichiers). Noms neufs : 0 site.
- **Proposition prête à appliquer** : (a), procédure N, identités relevées dans `declarations.tsv` à la fin de 4h : `test_ui_held_stops_off_row	test_ui_held_aborts_on_other_classes	tests/test_ui.c	c:test_ui.c@F@test_ui_held_stops_off_row`, `test_probe_t_key_stops_off_row	test_probe_state_time_key_at_zero	tests/test_ui.c	c:test_ui.c@F@test_probe_t_key_stops_off_row`, `test_recompute_on_placeholder	test_update_on_placeholder	tests/test_main_mouse.c	c:test_main_mouse.c@F@test_recompute_on_placeholder`, `test_recompute_on_placeholder	test_update_on_placeholder	tests/test_main_camera.c	c:test_main_camera.c@F@test_recompute_on_placeholder`, `RECOMPUTE_MOUSE_WORLD_X	UPDATE_MOUSE_WORLD_X	tests/test_main_mouse.c	macro:tests/test_main_mouse.c:RECOMPUTE_MOUSE_WORLD_X` et ses trois pendants (`_Y`, et les deux de `tests/test_main_camera.c`) ; les lignes de `docs/renommage.tsv` passent aux noms neufs. Renommer ne change aucun hachage (aucun nom n'est plié).
- **Portée** : s'applique aussi aux mentions en commentaire (aucune après 4h) ; contrôle : le `grep` des conventions sur les cinq anciens noms rend 0.
- **Verdict** : ok

### D-49 — Assistants de test qui citent un digramme du cart ou un surnom

- **Origine** : 4i (commit de commentaires, relecture de `tests/test_burst_exports_row.c` et `tests/test_trading_hub.c`).
- **Question** : `planet_by_ref` (`tests/test_burst_exports_row.c`) rend `planet->outgoing_routes` ; `by` est le digramme du cart de ce champ, que le §1 interdit et que 3i a déjà retiré du nom du test `test_burst_export_line_counts_the_outgoing_routes`. `build_plate`, `fold_plate`, `is_plate` (`tests/test_trading_hub.c`) disent « plate », surnom de test du plateau commercial que rien d'autre dans le dépôt ne dit ; `is_plate` lit `archetype->is_trading_hub`. Aucun compteur ne les voit : le nom entier n'est pas un ancien nom, `by` et `plate` sont des mots anglais. Les renommer ?
- **Options** : (a) `outgoing_routes_of`, `build_trading_hub`, `fold_trading_hub`, `is_trading_hub_archetype` (pas `is_trading_hub`, homonyme du champ d'archétype : espace de noms à part, mais lecture trompeuse), procédure N ; (b) garder.
- **Sites** (`grep` des conventions, fin de 4i) : `planet_by_ref` 2 (définition, appel), `build_plate` 6, `fold_plate` 3, `is_plate` 3 ; noms neufs : 0 site.
- **Proposition prête à appliquer** : (a), procédure N, identités relevées dans `declarations.tsv` à la fin de 4i : `planet_by_ref	outgoing_routes_of	tests/test_burst_exports_row.c	c:test_burst_exports_row.c@F@planet_by_ref`, `build_plate	build_trading_hub	tests/test_trading_hub.c	c:test_trading_hub.c@F@build_plate`, `fold_plate	fold_trading_hub	tests/test_trading_hub.c	c:test_trading_hub.c@F@fold_plate`, `is_plate	is_trading_hub_archetype	tests/test_trading_hub.c	c:test_trading_hub.c@F@is_plate` ; les lignes de `docs/renommage.tsv` passent aux noms neufs. Renommer ne change aucun hachage (aucun nom n'est plié).
- **Portée** : s'applique aussi aux mentions en commentaire ; le mot « plate » des commentaires de `tests/test_trading_hub.c` est déjà réécrit « trading hub » par 4i (`grep -rnw 'plate\|plates' src tests` ne rend que les trois noms). Contrôle : le `grep` des conventions sur les quatre anciens noms rend 0.
- **Verdict** : ok

### D-50 — Locales courtes dont le nom porte un digramme du cart

- **Origine** : 4j (commit de locales, relevé des locales de `src/game/score_panel_display.c`).
- **Question** : `c_truthy` (`score_panel_update`, portée 36 : la vérité de `is_shown`, `c_` est le digramme du champ) et `s_str` (`score_panel_update_map`, portée 62 : le texte de la note, `s` est la table des notes du cart) gardent un digramme du cart dans un nom plus long. Aucun compteur ne les voit (`c`, `truthy`, `s`, `str` passent la règle des mots ; `c_truthy` est à portée ≤ 40). Les renommer ?
- **Options** : (a) `is_shown` (homonyme du champ `is_shown`, espace de noms à part) et `score_text`, procédure N ; (b) garder (§1 pour `c_truthy` ; `s_str` découpé en mots).
- **Sites** (`grep` des conventions, fin de 4j) : `c_truthy` 2, `s_str` 3 ; noms neufs : `is_shown` 14 (le champ, et ses mentions en commentaire), `score_text` 0.
- **Proposition prête à appliquer** : (a), procédure N, identités relevées dans `declarations.tsv` à la fin de 4j : `c_truthy	is_shown	src/game/score_panel_display.c	c:score_panel_display.c@9538@F@score_panel_update@c_truthy`, `s_str	score_text	src/game/score_panel_display.c	c:score_panel_display.c@4936@F@score_panel_update_map@s_str` (offsets à relever de nouveau) ; lignes `identity` de `tools/rename_overrides.tsv`, les lignes de `docs/renommage.tsv` passent aux noms neufs. Renommer ne change aucun hachage (aucun nom n'est plié). Le commentaire de `score_panel_update` (`src/game/score_panel_display.c`, dans `tools/line_coverage_reference.txt`) garde sa hauteur.
- **Portée** : s'applique aussi aux mentions en commentaire (aucune après 4j) ; contrôle : le `grep` des conventions sur les deux anciens noms rend 0.
- **Verdict** : ok

### D-51 — `spacer` qui tient un émetteur dans quatre tests de 3i

- **Origine** : 4j (commit de locales : `spacer` → `emitter` dans `test_built` de `tests/test_help_screen.c`, comme `fold_probe` à 4f2 ; relevé des lignes du TSV de même `ancien` et de même `nouveau`).
- **Question** : `fold_ascension` (`tests/test_ascension.c`, portée 117), `fold_burst` (`tests/test_burst.c`, 114), `fold_protostar` (`tests/test_protostar.c`, 130) et `fold_synthesizer` (`tests/test_synthesizer.c`, 123) déclarent `const emitter_t *spacer = …->emitter` : le nom qui ment que 4f2 et 4j ont renommé ailleurs. `spacer` est aussi le champ légitime `planet_archetype_t.spacer` (la rangée d'espacement), que ces mêmes fichiers lisent. Aucun compteur ne les voit (`spacer` est un mot validé). 3i est close : les renommer ?
- **Options** : (a) `emitter`, procédure N (lignes `em` → `spacer` → `emitter` du TSV, `ancien` gardé, D-42) ; (b) garder.
- **Sites** (fin de 4j, mot entier hors `->spacer`) : 55 (`test_ascension.c` 12, `test_burst.c` 15, `test_protostar.c` 14, `test_synthesizer.c` 14), plus 7 lectures du champ, qui restent ; le rapport de `rename-apply` fait foi.
- **Proposition prête à appliquer** : (a), procédure N, identités relevées dans `declarations.tsv` à la fin de 4j : `spacer	emitter	tests/test_ascension.c	c:test_ascension.c@10781@F@fold_ascension@spacer`, `spacer	emitter	tests/test_burst.c	c:test_burst.c@9384@F@fold_burst@spacer`, `spacer	emitter	tests/test_protostar.c	c:test_protostar.c@10400@F@fold_protostar@spacer`, `spacer	emitter	tests/test_synthesizer.c	c:test_synthesizer.c@10982@F@fold_synthesizer@spacer` (offsets à relever de nouveau) ; lignes `identity` de `tools/rename_overrides.tsv` ; les lignes de `docs/renommage.tsv` gardent leur `ancien` (D-42). Masquage : `emitter` est aussi un champ et une étiquette de struct, espaces de noms à part (précédent de 4f2). Renommer ne change aucun hachage (seuls des membres sont pliés).
- **Portée** : s'applique aussi aux mentions en commentaire (aucune) ; contrôle : `grep -nP '(?<![>.])\bspacer\b' tests/test_ascension.c tests/test_burst.c tests/test_protostar.c tests/test_synthesizer.c` rend 0.
- **Verdict** : ok

### D-52 — Nom de test `test_the_shared_palette_is_the_published_pico8_one`

- **Origine** : 4k2 (relecture `grep -i pico8` des fichiers du morceau).
- **Question** : le nom de ce test de `tests/test_screencast_player.c` dit `pico8` pour la console PICO-8, dont la palette publiée est vérifiée, pas pour le cart ni comme préfixe `pico8_` ; aucun compteur ne le voit. Le garder ?
- **Options** : (a) garder (il nomme la console, comme le préfixe `p8_` du runtime) ; (b) renommer, procédure N, en `test_the_shared_palette_is_the_published_console_one`.
- **Sites** : 2 (définition `tests/test_screencast_player.c:147`, appel de `main` `:168`, numéros de fin de 4k2).
- **Proposition prête à appliquer** : (a) ; sous (b), contrôle : `grep -rn pico8_one tests` rend 0.
- **Portée** : aucune mention en commentaire ni entre backticks.
- **Verdict** : ok

### D-53 — Assertion d'absence `"unknown"` sans écrivain (`tests/test_class_coverage.c`)

- **Origine** : 4a (laissée « sans écrivain, à 4k »), 4k3 (commit de chaîne).
- **Question** : `assert(strstr(log, "unknown") == NULL)` (ex-`"inconnue"`, traduit par 4k3) gardait l'absence de la ligne `inconnue <n>` que l'ancien binaire de couverture imprimait ; elle a disparu à `536ddd4`. Aucun écrivain n'imprime plus ce mot (sortie réelle de `tools/class_coverage.sh` sur la fixture et le fichier malformé du test : ni `inconnue` ni `unknown`), et un nom inconnu fait aujourd'hui rendre 3 au binaire (`src/cmd/class_coverage.c:68-76`), que le script imprime sous `failed:`. L'assertion ne peut pas échouer. Que faire ?
- **Options** : (a) garder ; (b) la retirer ; (c) la pointer sur ce qui signale vraiment un échec de rejeu : `strstr(log, "failed:") == NULL` (les rejeux du test ne rendent que 0 et 2, `unreadable:`).
- **Sites** : 1 (`tests/test_class_coverage.c:61`).
- **Proposition prête à appliquer** : (c), commit `test:` ; contrôle : `make test` vert, et une violation plantée (un rejeu qui rend 3) qui le fait échouer, jouée hors du dépôt.
- **Portée** : aucune mention en commentaire. Note pour 4l3 : si son message traduit de `src/cmd/class_coverage.c:73` (aujourd'hui « le nom %s n'est pas dans KNOWN[] », sur `stderr`, que le script recopie) contient `unknown`, (a) redevient une garde.
- **Complément (4l3)** : le message de `src/cmd/class_coverage.c` (nom absent de `KNOWN[]`) est traduit sans `unknown` : `the name %s is not in KNOWN[]` (et `an instantiated class has no coverage name (name "")`) ; l'assertion reste vide, et sous (a) elle ne redevient pas une garde. Le message d'`assert` d'usage dit `usage: class_coverage <events.json>`.
- **Verdict** : ok

### D-54 — Préfixe `k_` des tableaux `static const` du morceau 4k3

- **Origine** : 4k3 (relecture des tests de reprise).
- **Question** : `k_multiplier` (`src/devtools/replay_control.c`), `k_replayed_frames`, `k_live_frames` (`tests/test_marker_coherence.c`), `k_source_frames`, `k_live_frames` (`tests/test_recording_continuity.c`), `k_events_valid`, `k_events_valid_count` (`tests/test_replay_source.c`) portent un préfixe de constante à la hongroise que le §4 ne prévoit pas (nom complet ; `g_` pour une statique mutable seulement) ; aucun compteur ne le voit. Le retirer ?
- **Options** : (a) garder ; (b) retirer le préfixe, procédure N : `level_multipliers`, `replayed_frames`, `live_frames` (×2), `source_frames`, `events_valid_frames`, `events_valid_count`.
- **Sites** (mot entier, `src tests`) : 26 (`k_multiplier` 2, `k_replayed_frames` 4, `k_live_frames` 8, `k_source_frames` 5, `k_events_valid` 3, `k_events_valid_count` 4) ; noms neufs : 0 site, 0 déclaration. Identités (stables) : `c:replay_control.c@k_multiplier`, `c:test_marker_coherence.c@k_replayed_frames`, `c:test_marker_coherence.c@k_live_frames`, `c:test_recording_continuity.c@k_source_frames`, `c:test_recording_continuity.c@k_live_frames`, `c:test_replay_source.c@k_events_valid`, `c:test_replay_source.c@k_events_valid_count`.
- **Proposition prête à appliquer** : (b), sans hachage en jeu.
- **Portée** : s'applique aux mentions en commentaire (aucune) ; contrôle : `git grep -nwE 'k_(multiplier|replayed_frames|live_frames|source_frames|events_valid|events_valid_count)' -- src tests` rend 0. Hors morceau, `k_entry` (`src/game/planet_family.c`) et `k_key` (`src/game/score_panel.c`) sont des locales qui gardent le digramme `k` (famille de D-50), non comprises ici.
- **Verdict** : ok

### D-55 — Exception `du` du compteur de français dans le `Makefile`

- **Origine** : 4l1 (relevé de fin).
- **Question** : `du`, la commande `du -ch` de la recette de `gif` (qui imprime la taille de `gif/`), est compté `string` par `make french-count` comme l'article français « du » ; c'est du code, intouchable. L'excepter ?
- **Options** : (a) excepter ; (b) garder compté.
- **Sites** : 1 (`Makefile:981`, recette de `gif`).
- **Proposition prête à appliquer** : (a), la ligne `string<TAB>Makefile<TAB>du` ajoutée à `tools/french_exceptions.txt`. Contrôle, fait par 4l1 sur une copie du fichier (`tmp/4l1_end/french_exceptions_d55.txt`) : `french-count MODULES=Makefile` `string` 7 → 6 ; 0 après 4l2 (`un`, `ou`, `à`, `entité`, `défauts` de `COMPOSITION_RESIDUE_GUARD`) et 4l4 (`par` de `DATA_RESIDUE_GUARD`).
- **Portée** : aucune mention en commentaire.
- **Complément (4l2)** : après 4l2, `french-count MODULES=Makefile` rend `string` 2 (`du`, cette entrée ; `par` de `DATA_RESIDUE_GUARD`, 4l4) ; 1 avec la ligne de cette entrée. Les mots de `COMPOSITION_RESIDUE_GUARD` sont partis avec les catégories anglaises.
- **Complément (4l4)** : `par` est parti avec les catégories anglaises de `DATA_RESIDUE_GUARD` ; après 4l4, `french-count MODULES=Makefile` rend `string` 1 (`du`), 0 avec la ligne de cette entrée. Sans `SUBSTEP`, voir D-33 (36 = 26 + 1 + 8 + 1).
- **Verdict** : ok

### D-56 — Citations `cart`/`lua` irréductibles de la légende du `Makefile`

- **Origine** : 4l1.
- **Question** : la légende nomme les cibles `cart-names-count`, `cart-citations-test`, `cart-citations` et décrit ce que le relevé cherche (`` `cart` `` hors de `game/cart`, `` `Lua` ``, `` `lua.lua` ``) ; `tools/cart_citations.py` les compte (sa regex prend `-` pour une frontière ; il n'exclut que ses propres fichiers). Que faire de ce reste ?
- **Options** : (a) l'accepter (le relevé n'est pas une garde du §5) ; (b) que `tools/cart_citations.py` ne compte ni `cart-names-count`/`cart-citations` (noms de cibles) ni la ligne de légende qui le décrit, test d'abord ; (c) renommer les deux cibles (`old-names-count`, …), avec `CLAUDE.md`, le plan, le registre et les skills qui les citent.
- **Sites** : 6 `cart`, 2 `lua`, mesurés après 4l1 (`Makefile:149` entrée de `cart-names-count`, `:199` `cart-citations-test`, `:201` `cart-citations`, `:203` renvoi à `cart-names-count`, `` `cart` ``, `game/cart` et `` `Lua` ``, `:204` `` `lua.lua` ``).
- **Proposition prête à appliquer** : (a), rien à appliquer ; contrôle : `make cart-citations MODULES=Makefile` rend exactement 6/2/0.
- **Portée** : aucune mention ailleurs dans le code (les outils du dictionnaire et `tools/cart_citations.py` sont exclus du relevé).
- **Complément (4l4)** : remesuré à la fin de l'étape 4, `make cart-citations MODULES=Makefile` rend 6/2/0 (lignes 149, 199, 201, 203, 204) ; inchangé.
- **Verdict** : ok

### D-57 — Noms du cart dans les fixtures de `tests/test_composition_residue.c`

- **Origine** : 4e (« Laissés »), 4l2.
- **Question** : les fixtures écrivent des noms de type et de fonction au préfixe et aux digrammes du cart (`"pico8_" "jo_t"` ×18, `qh_t` ×5, `qh_of` ×5, `nl_t` ×4, `mh_t`, `g__mm`, `reg_items`, `state_fn`…), coupés en deux littéraux : aucun compteur ne les voit. Une partie est exigée par un motif qui chasse ce préfixe (`parent-link`, `defaults`, `void-ptr`, `name-dispatch`, `nu` : D-5, D-7, D-8, D-11) ; le reste est arbitraire. Que faire des noms arbitraires ?
- **Options** : (a) tout garder tant que ces motifs vivent ; (b) renommer les noms arbitraires en noms C (`planet_t`…), garder ceux qu'un motif exige ; (c) suivre D-11 (les fixtures d'une catégorie retirée partent avec elle).
- **Sites** : 83 occurrences de `pico8_` dans le fichier, mesurées après 4l2.
- **Proposition prête à appliquer** : (a), puis (c) au verdict de D-11.
- **Contrôle** : `make test` vert.
- **Portée** : les fixtures seulement ; les commentaires du fichier, réécrits par 4l2, ne citent aucun de ces noms.
- **Verdict** : ok

### D-58 — Macro `ATT` de `tests/test_composition_residue.c`

- **Origine** : 4l2.
- **Question** : `ATT`, abréviation de « atteinte » (statut `conservé` au TSV validé), nomme la liste d'atteinte de fixture ; 4l2 a passé sa variable d'environnement à `COMPOSITION_REACHABILITY` et son chemin à `DIR "/reachability.txt"`. La renommer ?
- **Options** : (a) `REACHABILITY_LIST` ; (b) garder.
- **Sites** : 7 (définition et six emplois).
- **Proposition prête à appliquer** : (a), procédure N (identité `macro:tests/test_composition_residue.c:ATT`, ligne `renommé` au TSV et `name` aux surcharges).
- **Contrôle** : `make test` vert ; `grep -cw ATT tests/test_composition_residue.c` rend 0.
- **Portée** : aucune mention en commentaire.
- **Verdict** : ok

### D-59 — Faux positifs des compteurs dans le relevé figé `tools/pico8_reference_probe_out.txt`

- **Origine** : 4b (« la sonde et son relevé à 4l3 »), 4l3.
- **Question** : le relevé est la mesure faite sur PICO-8 0.2.6b, qu'on ne peut ni régénérer ici ni retoucher (§5 : une référence n'est jamais réécrite pour masquer). `french-count` y lit `ce` dans huit nombres hexadécimaux (`0x0000.ce4f`, `0xffff.ce0f`… ; lignes 97, 131, 159, 212, 240, 285, 312, 314) et `cart-citations` compte en `short` les sept lettres de ses en-têtes de bloc (`## b rounding`, `## c tostr`, `## d pow075`, `## e sqrt`, `## e sin`, `## e cos`, `## f rnd`), écrits par la sonde et lus par `tools/constants_compare.sh` (`## a literals`). Que faire de ce reste ?
- **Options** : (a) la ligne d'exception `string<TAB>tools/pico8_reference_probe_out<TAB>ce` dans `tools/french_exceptions.txt`, et le reste `short` accepté (le relevé des citations n'est pas une garde du §5, comme D-56) ; (b) (a), plus `tools/cart_citations.py` qui ne lit pas ce fichier (test d'abord) ; (c) réécrire les en-têtes à la main, avec la sonde et leurs lecteurs.
- **Sites** : 8 (`french-count`, `string`) + 7 (`cart-citations`, `short`).
- **Proposition prête à appliquer** : (a). Contrôle, fait par 4l3 sur une copie du fichier d'exceptions (`tmp/4l3_s3/fe_copy.txt`) : `french-count MODULES=tools/pico8_reference_probe_out` 8 → 0 ; `make cart-citations MODULES=tools/pico8_reference_probe_out` rend exactement 0/0/7.
- **Portée** : aucune mention en commentaire (la sonde, réécrite par 4l3, écrit ses blocs « block A » à « block F »).
- **Complément (4l4)** : inchangé à la fin de l'étape 4 (8 `string`, 7 `short`) ; sans `SUBSTEP`, voir D-33.
- **Verdict** : ok

### D-60 — `blank` et `screen_texture` du `main` de `src/cmd/screencast_viewer.c`

- **Origine** : 4l3 (`cryptic-locals`).
- **Question** : la liste des locales cryptiques relève ces deux locales (portées 59 et 58 après 4l3 ; 60 et 59 avant ses commentaires) parce que `blank` et `texture` ne sont la partie d'aucun nom neuf du TSV ; ce sont les mots de l'API raylib (`Image` laissée noire, `Texture2D` de l'écran), et `src/platform/raylib_backend.c` nomme pareil le même rôle (locale `blank` de `raylib_backend_init`, portée courte ; champ `screen_texture` de `raylib_backend_t`). Les renommer seules créerait deux noms pour un même rôle, et le champ n'est pas une locale de l'étape 4.
- **Options** : (a) garder (exception de la liste, ou règle : un mot d'un type de l'API raylib vaut validé, test d'abord dans `tests/test_scope_count.py`) ; (b) renommer les deux locales et leurs homologues du backend en mots validés (`black_frame`, `screen_display`…), par la procédure N, champ compris.
- **Sites** : 2 déclarations (et 2 homologues : la locale `blank` de `raylib_backend_init`, le champ `raylib_backend_t.screen_texture`).
- **Proposition prête à appliquer** : (a), par une règle plutôt qu'une liste ; contrôle : `make cryptic-locals MODULES=cmd/screencast_viewer` rend 2 candidats de moins.
- **Portée** : aucune mention en commentaire (relevé à la fin de l'état 3 de 4l3).
- **Verdict** : ok

### D-61 — Exception `LÖVE` du compteur de français (`tools/ttf_to_pixelfont.py`)

- **Origine** : 4l4 (relevé de fin).
- **Question** : la docstring (« Bake a TTF/OTF into a LÖVE2D ImageFont sprite sheet »), imprimée par `--help` (`argparse.ArgumentParser(description=__doc__)`), écrit le nom propre du moteur avec son `Ö` ; `french-count` le compte `string`. Que faire de ce reste ?
- **Options** : (a) la ligne `string<TAB>tools/ttf_to_pixelfont<TAB>LÖVE` dans `tools/french_exceptions.txt` ; (b) écrire « LOVE2D » (le nom change dans l'aide) ; (c) retirer l'outil : aucune cible du `Makefile`, aucun appelant (`git grep ttf_to_pixelfont` : `docs/vocabulaire.md`, `tools/rename_substeps.tsv`, le plan, ce registre), il sort d'un autre jeu (il imprime du Lua pour Love2D).
- **Sites** : 1.
- **Proposition prête à appliquer** : (a). Contrôle, fait par 4l4 sur une copie du fichier d'exceptions (`tmp/4l4_s3/fe_copy.txt`) : `french-count MODULES=tools/ttf_to_pixelfont` 1 → 0.
- **Portée** : aucune mention en commentaire.
- **Verdict** : ok

### D-62 — Noms au préfixe du cart des fixtures des tests de garde, hors `test_alloc_guard.c`

- **Origine** : 4l4 (même question que D-4).
- **Question** : `tests/test_float_guard.c` (`pico8_floating.c`, `pico8_integer.c`, `pico8_another.c` ×3, `pico8_half` ×2, `pico8_third`), `tests/test_constants_check.c` (`pico8_a.c`, `pico8_b.c`) et `tests/test_float_grep.c` (macro `PICO8_HOST_BUILD`, jamais définie) fabriquent des sources aux noms du cart ; hors TSV, aucun compteur ne les voit, aucun motif ne les exige.
- **Options** : (a) retirer le préfixe ; (b) garder.
- **Sites** : 8 + 2 + 1 occurrences (7 + 2 + 1 lignes ; `grep -c 'pico8_\|PICO8_'` des trois fichiers à la fin de 4l4).
- **Proposition prête à appliquer** : (a), un `sed` mot entier par nom, dans les trois fichiers seulement (`pico8_floating` → `floating`, `pico8_integer` → `integer`, `pico8_another` → `another`, `pico8_half` → `half`, `pico8_third` → `third`, `pico8_a` → `a`, `pico8_b` → `b`, `PICO8_HOST_BUILD` → `HOST_BUILD`), commit `test:`, avec D-4.
- **Contrôle** : `make test` vert ; `grep -c 'pico8_\|PICO8_'` des trois fichiers rend 0.
- **Portée** : les commentaires des trois fichiers, réécrits par 4l4, ne citent aucun de ces noms (la fixture de `test_float_grep.c` dit « a macro the flags never define »).
- **Verdict** : ok

### D-63 — Noms du cart dans les fixtures de `tests/test_data_residue.c`

- **Origine** : 4l4 (même question que D-57).
- **Question** : les fixtures écrivent des noms au préfixe et aux digrammes du cart (`"pico8_" "value_t"`, `ob_table_t`, `unstash`, `type_tag_t`, `ref_audit`, `mv`, `list_t`, `jo_default`, `jo_truthy`, `g_jo_spec`, `jo_default_fixed`…), coupés en deux littéraux ; presque tous sont exigés par les motifs des catégories historiques (D-9, D-11). Que faire des noms arbitraires ?
- **Options** : (a) garder tant que ces motifs vivent ; (b) renommer les digrammes arbitraires (`jo` → `planet`) en gardant ce qu'un motif exige ; (c) suivre D-11 et D-9 (les fixtures d'une catégorie retirée partent avec elle).
- **Sites** : 23 lignes avec `pico8_`/`PICO8_` (`grep -c`, à la fin de 4l4).
- **Proposition prête à appliquer** : (a), puis (c) aux verdicts de D-9 et D-11.
- **Contrôle** : `make test` vert.
- **Portée** : les fixtures seulement ; les commentaires du fichier, réécrits par 4l4, n'en citent aucune.
- **Verdict** : ok

### D-64 — Marqueur `# TROU-1` de `tools/constants_expected_gaps.txt`

- **Origine** : 4l4.
- **Question** : la liste des défaillances attendues de `constants-check` (vide) porte en tête le marqueur `TROU-1` ; l'entrée TROU-1 de `docs/trous-de-portage.md` traite de `sin`/`cos` hors des 64 points relevés et ne mentionne ni `constants-check` ni cette liste (`grep -n -i 'constants\|attendu' docs/trous-de-portage.md` : 0), et `constants-check` compare des littéraux décimaux, pas `sin`/`cos`. Garder le marqueur ?
- **Options** : (a) garder (et le passer à `GAP-1` à l'étape 5) ; (b) retirer la ligne.
- **Sites** : 1.
- **Proposition prête à appliquer** : (b), à l'étape 5, avant le passage `TROU-N` → `GAP-N`.
- **Contrôle** : `make constants-check` et `make test` verts.
- **Portée** : aucune autre mention.
- **Verdict** : ok

### D-65 — Leçons de la consigne de l'étape 4 à reporter dans le skill `reprendre-les-commentaires`

- **Origine** : 4l4 (suppression de `docs/consigne-etape-4.md`).
- **Question** : la consigne meurt avec l'étape (son dernier état : `git show dd1d875:docs/consigne-etape-4.md`) ; le skill « sert aussi hors de ce chantier ». Faut-il y reporter les pièges qui ne tiennent pas à ce chantier ?
- **Options** : (a) rien, git garde la consigne ; (b) reporter dans `consigne-agent.md` du skill quatre pièges généraux, en français : une ligne de commentaire qu'un script imprime (`sed -n '2p' "$0"`) est une chaîne ; un `#` d'ici-document n'est pas un commentaire ; un commentaire glissé dans un programme `awk` entre apostrophes casse le script sans changer les lignes hors commentaire (`sh -n`) ; un fichier dont un outil compare les numéros de ligne (couverture, message d'`assert` imprimé) garde la hauteur de chaque bloc.
- **Sites** : 1 fichier du skill.
- **Proposition prête à appliquer** : (b), à l'étape 5 avec la mise à jour des skills (sous-étape 5f).
- **Contrôle** : relecture humaine.
- **Verdict** : ok

### D-66 — Comment les deux gardes deviennent bloquantes

- **Origine** : consultation `plan-review` d'avant l'étape 5 ; mesures refaites à `1b4b949`.
- **Question** : le §5 du plan rend les deux gardes « bloquantes dans `make test` ». Or toutes deux lisent une extraction libclang : `tools/rename_count.py` lit `tmp/ast_rename/occurrences.tsv` (noms ambigus), `tools/scope_count.py` lit `tmp/ast_rename/declarations.tsv`. Les brancher telles quelles met `make rename-extract` dans `make test`. Par quel mécanisme deviennent-elles bloquantes ?
- **Mesures** (à `1b4b949`, 4 cœurs, extraction écrite hors de `tmp/`) : `make rename-extract` 98 s (179 unités × 3 configurations, aucune ligne `warning:`, les 4 `partial` de `class_coverage`) ; `make test` 15,5 s (deux passes à chaud, 15,7 et 15,5) ; sur l'extraction faite, `make cart-names-count` 2,0 s et `make scope-count` 0,3 s : le coût est l'extraction, six fois la suite. Sorties : `declarations.tsv` 1,9 Mo, `occurrences.tsv` 7,2 Mo. Dépendances, dont `make test` n'exige aujourd'hui aucune : les bindings Python de libclang dans `third_party/clang-python` (ignoré par git), posés par `make rename-tool-setup`, qui fait `pip install clang==18.1.8` par le réseau ; `libclang` de `/usr/lib/llvm-18` (`LIBCLANG`, `Makefile:1203`). Restes à lever avant tout blocage : `cart-names-count` 1 (D-1), `scope-count` 12 (D-13), les mêmes qu'à la clôture.
- **Options** :
  - (a) extraction incrémentale par fichier, mise en cache dans `tmp/` (une unité réanalysée seulement si elle ou un en-tête qu'elle inclut a changé) ; `make test` lance l'extraction puis les deux gardes, et **échoue** (ne saute pas) si libclang ou les bindings manquent, avec un message qui nomme `make rename-tool-setup`. Première passe à froid : le coût d'une extraction complète.
  - (b) un artefact réduit, versionné (identifiant, sorte, étendue de portée : ce que lisent les deux gardes), avec le hachage des sources qui l'ont produit ; `make test` lit l'artefact sans libclang et **échoue** si le hachage ne correspond plus aux sources ; une cible hors suite, qui demande libclang, le régénère.
  - (c) scission : la partie texte de la garde anti-cart (noms distinctifs, recherche en mot entier sur le TSV) dans `make test`, sans libclang ; la partie AST (noms d'une ou deux lettres, garde de portée) dans une cible bloquante séparée, chaînée dans `make test-all` ou dans une `make check`. (c) rouvre la lettre du §5 (« dans `make test` ») : seul l'humain peut la prendre.
- **Sites** : 2 gardes (`tools/rename_count.py`, `tools/scope_count.py`), la recette de `test` (`Makefile:906`) et sa légende.
- **Proposition prête à appliquer** : aucune option n'est préférée par la méthode : l'arbitrage (durée de la suite, dépendance réseau et système, lettre du §5) est humain. Sous chacune, 5a construit le mécanisme, test d'abord (`tests/test_rename_count.py`, `tests/test_scope_count.py`, ou un test neuf de la cible), et 5e le branche, une fois les restes à zéro (`docs/plan-5.md`, §6, 5a et 5e).
- **Contrôle** : celui de 5e (une violation plantée rougit la vraie cible) ; en plus, sous (a), `make test` avec `LIBCLANG` pointé sur un fichier absent rend un code non nul ; sous (b), un `.c` modifié sans régénérer l'artefact rend `make test` rouge ; sous (c), `make test` passe sans libclang et la cible AST, sans libclang, échoue.
- **Verdict** : ok

### D-67 — Le compteur de français devient-il une troisième garde ?

- **Origine** : §6 du plan, entrée 5 (« l'étape tranche si le compteur de français devient une troisième garde permanente »), déplacée ici pour être tranchée au point d'arrêt ; compteur construit par 4a (`tools/french_count.py`).
- **Question** : le §2 interdit tout mot français dans le code ; seul le compteur de français le voit, et il ne bloque rien. En faire une garde bloquante, comme les deux du §5 ?
- **Mesures** (à `1b4b949`) : `make french-count` 0,75 s, texte seul, sans extraction ni libclang ; reste sans `SUBSTEP` : `string` 36, exactement ceux de D-33 (26), D-55 (1), D-59 (8) et D-61 (1), 0 avec leurs 17 lignes d'exception.
- **Options** : (a) troisième garde, bloquante dans `make test` à 5e, après les lignes d'exception de 5d ; (b) relevé non bloquant gardé hors suite ; (c) retirer le compteur, son test, son fichier d'exceptions et ses deux cibles.
- **Sites** : 1 compteur (`tools/french_count.py`, `tests/test_french_count.py`, `tools/french_exceptions.txt`, cibles `french-count` et `french-count-test`) ; 36 restes.
- **Proposition prête à appliquer** : (a) : la garde ne coûte pas une seconde, ne dépend ni de libclang ni de D-66, et ses restes sont tous couverts par des entrées X. Sous (a), les verdicts de D-33, D-55, D-59 et D-61 finissent en lignes d'exception (règle des verdicts, « Conventions »).
- **Contrôle** : sous (a), celui de 5e : un mot français planté dans un commentaire d'une copie de travail, passé par la vraie cible, rend `make test` rouge.
- **Verdict** : ok

### D-68 — Alertes `warn-review` de fin de tronçon 3

- **Origine** : passe `/warn-review` de fin de tronçon 3 (outils des étapes 3 et 4, gardes de `tools/`), relevé transmis par la consultation `plan-review` d'avant l'étape 5 ; lignes vérifiées à `1b4b949`. Pendant de D-28 pour le tronçon 3.
- **Question** : un verdict par alerte. Chaque proposition est l'une de trois : *appliquer en 5* (verdict `proposition`, sous-étape nommée), *reporter hors plan 5*, *abandonner* (verdict `garder tel quel`). L'alerte 14 est **à trancher** : elle recoupe D-5, D-7, D-8, D-9 et D-11, et son verdict doit leur être cohérent ; 5b n'ouvre pas sans lui.
- **Sites et propositions** :
  1. *Import circulaire* : `tools/rename_count.py:30` importe `rename_substeps`, `tools/rename_substeps.py:25` importe `rename_count` (pour `read_table`, ligne 190). Proposition : appliquer en 5 (5a), avec 4 : `read_table` sort dans un module de lecture qui n'importe aucun outil ; contrôle : `rename_substeps` s'importe sans charger `rename_count` (`sys.modules`), tous les `rename-*-test` verts.
  2. *Moyeu sous un nom de compteur* : `rename_count.py` est importé par six outils (`cart_citations`, `french_count`, `rename_rekey`, `rename_strings_verify`, `rename_substeps`, `scope_count`) pour `read_table`, `source_files`, `c_regions`, `text_regions`, `BACKTICK`, `C_FILES`, `path_pattern`, `Names`. Proposition : reporter hors plan 5 ; seul `read_table` en sort (1, 4).
  3. *Règles de visibilité recopiées* : `tools/ast_rename/plan.py:162` (`_scopes_overlap`, `_shared`, l. 172) recopie la règle d'`overlap()` de `tools/rename_check.py`. Proposition : appliquer en 5 (5a), avant les renommages de 5c qui passent par `rename-apply` : `plan.py` importe la règle de `rename_check.py` (comme les mots-clés, D-28, puce 2) ; `rename-tool-test` et `rename-check-test` verts.
  4. *Lecteurs en plus du TSV et des déclarations* : `read_declarations` défini trois fois (`tools/scope_count.py:37`, `tools/rename_check.py:79`, `tools/rename_derive.py:37`), l'en-tête lu en ligne par `tools/rename_slice.py:37` et par `rewrite` de `tools/rename_rekey.py:56`. Proposition : appliquer en 5 (5a), avec D-28, puce 1 : un lecteur par fichier (`docs/renommage.tsv`, `declarations.tsv`) dans le module de lecture de 1, que tous importent.
  5. *`module_of` redéfini* : `tools/scope_count.py:31`, déjà dans `tools/rename_derive.py:229`. Proposition : appliquer en 5 (5a) : un seul `module_of`, puisque les lignes d'exception de 5d sont rangées par module ; contrôle : `make scope-count` et `make cryptic-locals` identiques à l'octet avant et après.
  6. *Imports dans un corps de fonction* : `tools/scope_count.py:84` (`rename_count`) et `:130` (`rename_substeps`). Proposition : appliquer en 5 (5a), avec 1 et 4 : imports en tête de fichier.
  7. *Mode caché* : `--cryptic` (`tools/scope_count.py:173`) est un second outil sous une option, et exige `--exceptions` (ligne 114), qu'il ne lit pas. Proposition : appliquer en 5 (5a) : `--exceptions` facultatif sous `--cryptic`, test d'abord dans `tests/test_scope_count.py` ; le mode reste une option.
  8. *Tableau par module réécrit quatre fois* : `french_count.py`, `rename_count.py`, `cart_citations.py`, `scope_count.py`. Proposition : reporter hors plan 5 (sorties comparées à l'octet par les notes de l'étape 4 ; rien ne change pour les gardes).
  9. *Lecteurs d'exceptions parallèles* : `read_exceptions` de `tools/french_count.py:128` (`colonne`, `module`, `mot`) et de `tools/scope_count.py:52` (`sorte`, `parent`, `nom`) ; D-1 en ajoute un troisième (`tools/cart_names_exceptions.txt`). Proposition : appliquer en 5 (5a) : un lecteur de triplets, chaque fichier garde son schéma ; contrôle : les tests des trois compteurs verts.
  10. *`COLUMNS` redéclaré* : `tools/french_count.py:40`, même tuple que `tools/rename_count.py:32`. Proposition : appliquer en 5 (5a), avec 4 : importé.
  11. *`--module`/`--substep` répétés dans six outils*. Proposition : reporter hors plan 5 : le chantier fini, `--substep` n'a plus d'usage.
  12. *Chemin du TSV écrit 19 fois* dans le `Makefile` (11 recettes, 8 lignes de légende), quand `RENAME_SUBSTEPS` existe pour la table des sous-étapes. Proposition : appliquer en 5 (5a) : une variable (`RENAME_TABLE := docs/renommage.tsv`) dans les recettes ; contrôle : `grep -n 'docs/renommage.tsv' Makefile` hors légende rend la seule définition, base de `make` égale par la table de la substitution.
  13. *Outils de l'étape 4 rangés en 3a* : `tools/rename_substeps.tsv:8` (`tools/french_count`, `tools/cart_citations`… avec `engine/entity_class`). Proposition : abandonner : rangement acté par 4a (plan, note de 4a, « Table »), et la table ne sert plus qu'aux `SUBSTEP` des étapes 3 et 4.
  14. **À trancher** — *Gardes épinglées sur des noms du cart effacés* : `tools/composition_residue.sh:66-69` (`defaults` `pico8_X_start`, `name-dispatch` `hp_pose`, `pico8_ho_gp_has`, `one-or-false` `pk_is_bool`, avec `nu`) et `tools/data_residue.sh:20` (catégories `value`, `ob`, `stash`, `list`, épinglées sur des noms `pico8_*` qui n'existent plus) apparient 0 ; `composition-guard` et `data-guard` sont verts dans `make test` parce que rien ne peut plus y apparier. Recoupe D-5, D-7, D-8, D-9, D-11 : D-5, D-7 et D-8 proposent de garder leur motif tel quel, D-11 de retirer les catégories pour cette même raison. Proposition : appliquer en 5 (5b) un verdict commun aux six entrées : un motif écrit en nom du cart qui apparie 0 est retiré avec ses fixtures (D-11 (a), étendu à la branche de D-5, D-7, D-8 et D-9), la garde anti-cart bloquante (5e) interdisant déjà ces noms ; à défaut, chaque motif gardé est déclaré « vert par construction » dans son entrée.
  15. *`SPEC_EXCLUSION` écrit en dur* : `tools/data_residue.sh:49`, sept noms. Proposition : abandonner : sans objet sous D-9 (a), qui retire l'exclusion ; sous D-9 (b), elle redevient vivante et reste telle quelle.
  16. *`tools/composition_residue.sh` fait 767 lignes*. Proposition : reporter hors plan 5 (découper le script est un redécoupage, §7 du plan).
- **Verdict** (un par alerte, l'alerte 14 d'abord) :

## Clôture (4l4)

Le registre est clos par 4l4, dernier morceau de l'étape 4 : **65 entrées**, D-1 à D-65, chacune avec son origine, sa question, ses options, ses sites et une proposition prête à appliquer, et une ligne `Verdict` vide (contrôle, par script : chaque `### D-n` a une ligne `Proposition` ou `propositions`, et une ligne `Verdict` vide — D-28, qui porte une proposition par alerte, a sa ligne « Verdict (un par alerte) », vide). L'étape 5 attend un verdict sur chacune.

**Complété par la consultation `plan-review` d'avant l'étape 5** : D-66 (mécanisme des gardes bloquantes), D-67 (compteur de français en troisième garde, question déplacée de l'entrée 5 du plan), D-68 (alertes `warn-review` de fin de tronçon 3, une proposition par alerte, l'alerte 14 à trancher), et la colonne « Application » de l'index (« Conventions », familles et verdicts). **68 entrées**, D-1 à D-68. Contrôle, par script : chaque `### D-n` a une ligne `Proposition` ou `propositions` et une ligne `Verdict` vide (D-28 et D-68, une proposition par alerte, ont leur ligne « Verdict (un par alerte) », vide ; la proposition de D-66 ne préfère aucune option) ; chaque ligne de l'index a une famille parmi `N`, `G`, `X`, `O`, `T`, `D`. Répartition : N 39, G 10, X 9, O 8, T 1, D 1.

**Restes comptés**, sans `SUBSTEP`, sur une extraction fraîche à la fin de 4l4 — la liste exacte, compteur par compteur :

| Compteur | Reste | Entrées |
|---|---|---|
| `french-count` | `string` 36 : `tests/test_cart_citations` 18, `tests/test_scope_count` 6, `tools/cart_citations` 1, `tools/scope_count` 1 ; `Makefile` 1 ; `tools/pico8_reference_probe_out` 8 ; `tools/ttf_to_pixelfont` 1 | D-33 (26), D-55 (1), D-59 (8), D-61 (1) |
| `cart-names-count` | `string` 1 (`tools/frames_ref`) | D-1 |
| `cart-citations` | `cart` 6, `lua` 2 (`Makefile`) ; `short` 7 (`tools/pico8_reference_probe_out`) | D-56, D-59 |
| `cryptic-locals` | 12 : `argc`/`argv` de cinq `main` (10), `blank`, `screen_texture` (2) | D-37, D-60 |
| `scope-count` | 12 statiques locales de tests (`static` 12) | D-13 |

Aucun autre reste. Avec les 17 lignes d'exception de D-33, D-55, D-59 et D-61, `french-count` sans `SUBSTEP` rend 0 (vérifié sur une copie de `tools/french_exceptions.txt`).

**Entrées qu'aucun compteur ne voit** : toutes les autres — D-2 à D-12, D-14 à D-32, D-34 à D-36, D-38 à D-54, D-57, D-58, D-62 à D-68.

## Clôture (5)

Condition d'entrée de 5g (`docs/plan-5.md`, §6, entrée 5g). Cette liste reprend, une ligne par point : les questions ouvertes des notes « Découvert » de 5a à 5f et des PRD graduées (décisions prises sans humain, restes à trancher, entrées GAP signalées par 5d, angles morts des gardes), les 18 alertes de la `warn-review` de fin d'étape 5, et la relecture des skills demandée par 5f. Les points sont rangés par thème, pas par sous-étape. Les doublons sont regroupés.

### Mode d'emploi

*Verdicts donnés par l'humain en séance le 2026-10-03 (« ok partout »), l'humain ne pouvant pas éditer le fichier ; transcrits par l'orchestrateur. C-33 et C-34 : « ok » après lecture des passages de skills montrés en séance.*

- Chaque point a un **verdict proposé**. Pour l'accepter, écrivez **ok** sur sa ligne `Verdict`.
- Pour en choisir un autre, écrivez-le à la place : `proposition`, `variante nommée` (avec la variante en une ligne), `garder tel quel` ou `reporter hors plan 5`.
- `garder tel quel` : rien n'est édité. `reporter hors plan 5` : rien n'est édité, la question part dans un ticket.
- Un point accepté en `proposition` qui édite devient une sous-étape (5h, 5j…) avec sa PRD, ou un ticket hors plan.
- « Déjà tranché par D-x » : la question a déjà eu votre verdict ; le point le rappelle seulement. « ok » suffit.
- Vocabulaire minimal : la **garde des anciens noms** (`old-names-guard`) et la **garde du français** (`french-guard`) tournent dans `make test` ; la **garde AST** (`ast-guard` : noms courts et portée) tourne dans `make test-all`.

### Thème 1 — Lecture de vos verdicts (2)

#### C-1 — « ok » lu comme `proposition`

- **Origine** : note 5a (« Lecture de “ok” »), suivie par 5b à 5f ; PRD 5b (H-5a-5).
- **Constat** : les agents ont lu chaque « ok » comme `proposition`, et les lignes `Verdict` restées vides de D-28 et D-68 comme `proposition` pour chaque alerte. Ils n'ont rien écrit dans le registre (aucune marque « appliqué »).
- **Proposition** : confirmer cette lecture. Vous écrivez « ok » sur les deux lignes vides de D-28 et D-68, pour que le registre soit complet.
- **Verdict proposé** : `proposition`
- **Verdict** : ok

#### C-2 — D-66 lu comme l'option (c)

- **Origine** : note 5a (« Lecture de “ok” »), note 5e (« Coûts », « Relevés de fin »).
- **Constat** : D-66 n'avait pas d'option préférée ; « ok » a été lu en séance comme (c). Résultat : `make test` reste rapide (22 à 25 s) et marche sans libclang ; `make test-all` passe de 23 s à environ 2 min 30 (l'extraction) et échoue sans libclang. 5e a corrigé le §5 du plan en ce sens.
- **Proposition** : confirmer. Rien à éditer.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

### Thème 2 — Ce que les gardes ne voient pas (10)

#### C-3 — Les noms historiques du cart ne sont cherchés par personne

- **Origine** : note 5b (« La garde anti-cart ne voit pas les noms historiques »), note 5e (« Ce que les gardes bloquantes ne voient toujours pas »).
- **Constat** : la garde des anciens noms ne cherche que les anciens noms du TSV. Quinze noms du cart n'y sont pas (`pico8_value_t`, `pico8_value`, `pico8_value_function`, `pico8_ob`, `pico8_unstash`, `pico8_mv`, `pico8_list_t`, `pico8_entity_get_field`, `pico8_jo_start`, `pico8_nl_start`, `hp_pose`, `pico8_ho_gp_has`, `pk_is_bool`, `fp_is_bool`, `nu`). Mesuré par 5b : planté dans `src/game/planet.c`, aucun n'est vu.
- **Proposition** : un fichier `tools/historical_names.txt` avec ces quinze noms, lu par la garde des anciens noms comme des noms distinctifs ; test d'abord (un nom planté rougit `make test`).
- **Verdict proposé** : `proposition`
- **Verdict** : ok

#### C-4 — Trois motifs écrits en noms du cart, verts par construction

- **Origine** : note 5b (« Ce qui reste écrit en noms du cart »), PRD 5b.
- **Constat** : trois motifs ne cherchent que des noms du cart disparus : la catégorie `dispatch` de `tools/data_residue.sh` (`pico8_value_function`, `pico8_mv(`), les deux branches `pico8_` de `by-name` (l'une est englobée par l'autre : aucun test ne peut la prouver vivante), et `get_nu` de `header-accessors` (`tools/composition_residue.sh`). Ils ne peuvent rien trouver. 5b les a gardés faute de verdict.
- **Proposition** : leur appliquer la règle de l'alerte 14 de D-68 : les retirer avec leurs fixtures et leurs lignes de vivacité (`dispatch` sort aussi de `DATA_RESIDUE_GUARD`). À prendre avec C-3 : la garde des anciens noms prend alors le relais sur ces noms.
- **Verdict proposé** : `proposition`
- **Verdict** : ok

#### C-5 — Une ancienne identité réintroduite ailleurs

- **Origine** : note 5e (« Ce que les gardes bloquantes ne voient toujours pas »).
- **Constat** : la garde AST reconnaît une ancienne identité (même fichier, même type), pas un ancien nom. Un ancien nom de trois lettres ou plus, recréé dans un autre fichier ou un autre type, passe. Un nom d'une ou deux lettres est rattrapé par la garde de portée, sauf en locale courte.
- **Proposition** : accepter ce trou. Le fermer demanderait de chercher tous les anciens noms ambigus en texte, avec des centaines de faux positifs (`pos`, `row`, `out`…).
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-6 — Une exception vaut pour tout son module

- **Origine** : note 5d (« Pour 5e », décision 1), note 5e.
- **Constat** : une ligne d'exception excepte un mot dans tout un fichier : un `du` neuf dans le `Makefile`, un `cart` neuf dans la légende, un `tmp/pico8_replay` neuf dans `tools/frames_ref.sh`, un `ce` neuf dans le relevé figé ne seront plus vus.
- **Proposition** : rien. Déjà tranché par D-1, D-55, D-56 et D-59 (granularité module du registre).
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-7 — Deux relevés restent non bloquants, avec des pièges connus

- **Origine** : note 5a (« Règle de D-36 (b) non étendue à `short` »), note 5c (« Pièges laissés »), note 5e (décision 9, angles morts).
- **Constat** : `cart-citations` et `cryptic-locals` sont à 0 mais ne bloquent rien. Pièges connus : une mention nue de `fy`, `kv`, `nj`, `oc` ou `gy` dans un commentaire compterait en `short` ; un `` `mem` `` ou un `` `gfx` `` entre backticks compterait partout (depuis D-38). Aucun n'est présent aujourd'hui.
- **Proposition** : rien. Déjà tranché par D-56 et D-59 (« le relevé n'est pas une garde du §5 »).
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-8 — Rien n'empêche un `TROU-` de revenir, ni un `/* GAP-N */` sans entrée

- **Origine** : alerte 17 de la `warn-review` de fin d'étape 5.
- **Constat** : 5d a fait passer tous les marqueurs à `GAP-N`, mais aucun contrôle ne le tient. Un `TROU-` copié d'une vieille branche, ou un `/* GAP-30 */` sans titre `## GAP-30` dans `docs/trous-de-portage.md`, passerait.
- **Proposition** : un contrôle dans `make test`, test d'abord : `git grep 'TROU-'` rend 0 dans `src`, `tests`, `tools` ; chaque numéro de `/* GAP-N */` a son titre dans `docs/trous-de-portage.md`.
- **Verdict proposé** : `proposition`
- **Verdict** : ok

#### C-9 — Une ligne d'exception morte ne se signale pas

- **Origine** : alerte 18 de la `warn-review` de fin d'étape 5 ; même famille que D-6, D-9 et D-10 (exclusions mortes trouvées à la main).
- **Constat** : si le code change et qu'une ligne d'exception ne retire plus rien, aucun outil ne le dit. La ligne reste et peut masquer un futur vrai problème.
- **Proposition** : sous `--check`, les lecteurs d'exceptions (`french_count.py`, `rename_count.py`, `cart_citations.py`, `scope_count.py`) échouent sur une ligne qui n'a rien retiré, en la nommant ; test d'abord.
- **Verdict proposé** : `proposition`
- **Verdict** : ok

#### C-10 — Les exceptions de portée (`x`, `y`) n'ont aucun test

- **Origine** : note 5e (« Écarts à la lettre »).
- **Constat** : 5e a remplacé `tools/scope_exceptions.txt` par un fichier vide : aucun test n'a rougi. Rien ne prouve que ces exceptions sont lues.
- **Proposition** : un cas de plus dans `tests/test_ast_guard.py` : un champ `x` de vecteur, vert avec le fichier d'exceptions, rouge sans.
- **Verdict proposé** : `proposition`
- **Verdict** : ok

#### C-11 — Choix faits sans vous pour les exceptions (5d)

- **Origine** : PRD 5d, « Décisions prises sans humain » 1 à 5.
- **Constat** : (1) D-56 et la part `short` de D-59 ont fini en lignes d'exception, dans un fichier neuf `tools/cart_citations_exceptions.txt` ; (2) D-60 est une liste de deux mots raylib (`blank`, `texture`) valables pour toute locale future ; (3) la règle de D-13 ne touche que la garde de portée, pas `cryptic-locals` ; (4) un fichier d'exceptions n'est pas compté par l'outil qui le lit, et ses lignes valent aussi pour la moitié AST ; (5) les fichiers neufs sont rangés en 3a de `tools/rename_substeps.tsv`.
- **Proposition** : confirmer. Rien à éditer.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-12 — Choix faits sans vous pour brancher les gardes (5e)

- **Origine** : PRD 5e, « Décisions prises sans humain » 1 à 14.
- **Constat** : les tests de vivacité travaillent sur une copie réduite du dépôt, pas sur l'arbre entier ; la copie AST n'analyse que les fichiers plantés (`AST_RENAME_UNITS`), seul écart à « la vraie cible » ; les violations plantées sont fabriquées à l'exécution (jamais écrites en clair) ; il y a deux fichiers de test (texte dans `make test`, AST dans `make test-all`) ; 5e a touché la ligne « Build » du `CLAUDE.md` et corrigé le §5 du plan.
- **Proposition** : confirmer. Rien à éditer.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

### Thème 3 — Noms laissés de côté (5)

#### C-13 — Douze `static const` gardent le préfixe `g_`

- **Origine** : note 5c (« Reste à trancher »), PRD 5c (décision 2).
- **Constat** : D-21 a retiré `g_` des tables de classe et de sorte. Le registre disait que le même verdict couvrirait les autres `static const g_…`, sans les nommer. 5c ne les a pas touchées : `g_message_names`, `g_outline_offsets`, `g_galaxy_menu_on_click`, `g_galaxy_menu_refresh`, `g_handles`, `g_planet_name_grab_arena`, `g_synthesizer_synthesizable_resources_spent`, `g_slipgate_left_press_empty` (`src/`), `g_without_birth`, `g_layer_ten`, `g_pos_nil`, `g_giver_layer` (`tests/`).
- **Proposition** : retirer le `g_` des douze par la procédure N. `g_handles` heurterait `handles` : il devient `slot_allocators` (un `allocator_t` par sorte d'emplacement, `src/game/particle_zone.c:110`) ; les autres prennent leur nom sans `g_`, après contrôle qu'il est libre.
- **Verdict proposé** : `proposition`
- **Verdict** : ok

#### C-14 — Paramètres courts hors de la famille de D-29, dont `dk`

- **Origine** : note 5c (« Reste à trancher ») ; signalé en passant : `dk` de `economy_score_set_difficulty` (`src/game/economy_score.c:259`).
- **Constat** : des paramètres d'une ou deux lettres gardent un digramme du cart : `fy` (`vec_from_polar` ×2, `apply_camera_to`), `ra` (`pos_copies` ×8, `build`, `build_protostar`, `with_pos`, `holds_copy_of`, `spy_draw`, `draw_layers`, `emitter_host_layer_pos`, `probe_host_set_positions`), `yr` (`year_box`), `dk`. Leur portée tient en 40 lignes : le §1 les permet et aucune garde ne les voit.
- **Proposition** : rien. Déjà tranché par D-29 : votre verdict a retenu la seule famille des méthodes (option (b)), pas tous les paramètres courts (option (c)).
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-15 — `DIFFICULTY_OPTIONS_COUNT` sans le nom de son module

- **Origine** : note 5c (« Reste à trancher »), PRD 5c (décision 4).
- **Constat** : D-24 disait que `DIFFICULTY_OPTIONS_COUNT` suivrait `HELP_DIFFICULTY_OPTIONS_ROW_CAP` s'il était du même fichier. Il ne l'est pas : c'est une constante d'énumération de `src/game/game_data.h:21`, lue par `help.c` et `game_data.c`.
- **Proposition** : la laisser. Avec le préfixe de son module, elle s'appellerait `GAME_DATA_DIFFICULTY_OPTIONS_COUNT`, plus long sans rien dire de plus.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-16 — Trois locales gardent un digramme du cart : `k_entry`, `k_key`, `gy`

- **Origine** : note 5c (« Reste à trancher ») ; même famille que D-50 (`c_truthy`, `s_str`) et D-32 (`gy` → `cursor`).
- **Constat** : `k_entry` (`src/game/planet_family.c:418`) tient une couleur de traînée ; `k_key` (`src/game/score_panel.c:170`) l'indice de la ressource `"k"` ; `gy` (`tests/test_structure_icon.c:568`) le curseur partagé d'avant la relance.
- **Proposition** : procédure N : `k_entry` → `trail_color`, `k_key` → `resource_index`, `gy` → `cursor_before`, le nom que porte déjà la même locale dans `test_directed` (noms à vérifier libres dans leur fonction).
- **Verdict proposé** : `proposition`
- **Verdict** : ok

#### C-17 — Noms choisis sans vous pendant 5c

- **Origine** : PRD 5c, « Décisions prises sans humain » 1 à 11 ; note 5c (« Écarts à la lettre »).
- **Constat** : 5c a choisi elle-même : `planet_kinds` pour `g_planet_kind` (D-21) ; `host_class_def` pour `g_host_cls` ; `screen_pos` et non `mouse_screen` pour le `ra` de `draw` et `draw_overlay`, qui reçoivent la position de l'entité (D-29 ; la ligne du TSV, partagée avec `hit_test`, dit `mouse_screen`) ; `me` → `state` sur les 12 ; `fy` de `vec_from_polar` gardé (c'est une longueur) ; « identifier counter » et non « id counter » (D-47) ; deux boucles bornées dans `tests/test_main_mouse.c` (D-18).
- **Proposition** : confirmer. Rien à éditer.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

### Thème 4 — Outillage (alertes de la `warn-review` et choix de 5a) (10)

#### C-18 — Règles de portée recopiées entre outils

- **Origine** : alertes 1, 2 et 3 de la `warn-review` de fin d'étape 5.
- **Constat** : le seuil de 40 lignes est écrit deux fois (`tools/scope_count.py:35`, `tools/rename_derive.py:353`) ; la règle « une statique de fonction se juge comme une locale » est réécrite dans trois outils ; la règle « paramètre de pointeur de fonction » existe en deux versions qui diffèrent (`scope_count.py:60` teste un `.` dans le parent, `rename_derive.py:343` un `/`). Seul `scope_count.py` est une garde permanente ; les autres copies sont dans des outils `rename_*` du chantier.
- **Proposition** : rien maintenant. Le sort des outils `rename_*` se décide en 5i : s'ils partent, les copies partent avec eux ; s'ils restent, un ticket ramène chaque règle à une seule source.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-19 — `tools/ast_rename/plan.py` garde son propre `read_table`

- **Origine** : alerte 4 de la `warn-review` ; note 5a (« Écarts à la lettre des entrées »).
- **Constat** : ce lecteur lit la table passée à `rename-apply` (trois ou quatre colonnes, numéros de ligne pour ses refus), jamais `docs/renommage.tsv`.
- **Proposition** : rien. Déjà tranché par D-28 (alerte 1) ; l'écart est consigné par 5a.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-20 — Les outils s'importent les uns les autres

- **Origine** : alertes 5 à 10 de la `warn-review` (deux `source_files` homonymes ; `read(root, path)` en double ; `rename_count.py` sert de bibliothèque ; `rename_derive` importé partout pour `module_of` ; `plan.py` importe l'outil `rename_check` ; `rename_count` dépend de `rename_strings`).
- **Constat** : ces liens viennent de votre verdict sur D-68 : l'alerte 2 (le moyeu `rename_count.py`) a été reportée hors plan 5 ; les alertes 3 et 5 (une règle, un `module_of`) ont été appliquées en 5a en important l'outil qui les avait déjà.
- **Proposition** : rien. Déjà tranché par D-68 (alertes 2, 3 et 5) ; 5i, en fixant le sort des outils `rename_*`, en retirera la plupart.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-21 — Un test en importe un autre

- **Origine** : alerte 11 de la `warn-review` (`tests/test_ast_guard.py` importe `tests/test_text_guards.py`).
- **Constat** : c'est voulu par 5e : le test AST réutilise la copie réduite du test texte au lieu de la recopier.
- **Proposition** : rien.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-22 — `tools/` est à plat, avec des préfixes incohérents

- **Origine** : alerte 12 de la `warn-review`.
- **Constat** : une cinquantaine de fichiers dans un seul répertoire, nommés tantôt par outil (`rename_*`), tantôt par rôle (`*_guard.sh`, `*_residue.sh`).
- **Proposition** : ranger `tools/` est un redécoupage (§7 du plan). À faire après 5i, une fois les outils du chantier partis ou gardés.
- **Verdict proposé** : `reporter hors plan 5`
- **Verdict** : ok

#### C-23 — Un même concept sous trois noms dans le `Makefile`

- **Origine** : alerte 13 de la `warn-review` (`Makefile:1373`).
- **Constat** : la recherche des anciens noms s'appelle `old-names-guard` (garde), `cart-names-count` (relevé) et `tools/rename_count.py` (outil).
- **Proposition** : rien. Déjà tranché par D-56 : renommer les cibles (option (c)) n'a pas été retenu.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-24 — `--detail` imprime `void-ptr-r2`, nom d'une catégorie retirée

- **Origine** : alerte 14 de la `warn-review` (`tools/composition_residue.sh:678`) ; PRD 5b (« Further Notes » : étiquette gardée pour ne pas changer `--detail`).
- **Constat** : la catégorie `void-ptr` a été retirée en 5b ; ses lignes R2 (`tools/composition_residue_sites.txt`) s'impriment encore sous ce nom.
- **Proposition** : renommer l'étiquette `void-ptr-r2` en `r2-sites`, dans le script et ses tests, en un commit ; `--detail` ne change que de cette étiquette.
- **Verdict proposé** : `proposition`
- **Verdict** : ok

#### C-25 — 25 lignes de commentaire pour une affectation

- **Origine** : alerte 15 de la `warn-review` (`tools/composition_residue.sh:98`, `REGISTRIES_EXEMPT`).
- **Constat** : le commentaire dit ce que la catégorie voit, ce qu'elle ne voit pas, et prouve chaque exemption par son écrivain. 4l2 l'a déjà repris sous la règle des commentaires.
- **Proposition** : le garder : la liste de ce qui n'est pas vu est un piège, la preuve de chaque exemption un contrat. Variante possible : le raccourcir par le skill `reprendre-les-commentaires` (rattrapage ponctuel).
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-26 — Colonnes du TSV écrites en français dans le code

- **Origine** : alerte 16 de la `warn-review`.
- **Constat** : `ancien`, `nouveau`, `renommé`, `conservé`… sont écrits en littéraux dans plusieurs outils et tests.
- **Proposition** : rien. Déjà tranché par D-28 (alerte 3 : schéma du TSV gardé en français) et D-33 (lignes d'exception).
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-27 — Choix faits sans vous dans l'outillage (5a)

- **Origine** : note 5a (« Écarts à la lettre des entrées », « Ancre de D-27 »).
- **Constat** : la version exacte de libclang `18.1.8` a disparu : le script installe `clang==18.*` ; le contrôle de D-36, périmé, a été remplacé par une mention plantée ; l'ancre `pico8_%s_struct_class` de `tools/rename_strings.py` reste fausse (même âge que le motif corrigé par D-27), mais l'outil n'est plus lancé ; `rename-apply` refuse désormais 15 mots-clés de plus (`bool`, `true`…).
- **Proposition** : confirmer. Le sort de `rename_strings.py` se décide en 5i.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

### Thème 5 — Registre des écarts `docs/trous-de-portage.md` (5)

#### C-28 — GAP-3 se trompe sur `economy_score_trade_income`

- **Origine** : note 5d (« Registre des écarts, entrées signalées »).
- **Constat** : GAP-3 dit deux fois que `economy_score_trade_income` compare des chaînes ; elle compare un indice de ressource.
- **Proposition** : retirer `economy_score_trade_income` des deux phrases de GAP-3, après relecture du code ; commit `docs:`.
- **Verdict proposé** : `proposition`
- **Verdict** : ok

#### C-29 — GAP-9 dit « Sans site » mais a un site

- **Origine** : note 5d (« Registre des écarts, entrées signalées »).
- **Constat** : GAP-9 (pas de suppression d'un champ nommé) dit « Sans site » et parle de la table à clés dynamiques, supprimée. Pourtant `src/game/message_box.c:322` porte `/* GAP-9 */` : le calque du bouton ne prend la couleur de fond du cadre que si le cadre en a une (en Lua, écrire nil effacerait le champ). Le test `tests/test_button.c:1670` le fixe, et GAP-23 s'appuie sur GAP-9.
- **Proposition** : réécrire la fin de GAP-9 : nommer ce site et ce test au lieu de « Sans site » ; garder le numéro.
- **Verdict proposé** : `proposition`
- **Verdict** : ok

#### C-30 — GAP-4 et GAP-11 décrivent du code supprimé

- **Origine** : note 5d (« Registre des écarts, entrées signalées »).
- **Constat** : GAP-4 (constructeur de champ) et GAP-11 (repli d'arité du test de survol) n'ont plus de code ni de marqueur. Leur texte le dit déjà (« Sans site »).
- **Proposition** : retirer les deux entrées : l'écart est refermé avec le code. Les numéros 4 et 11 ne sont pas réutilisés.
- **Verdict proposé** : `proposition`
- **Verdict** : ok

#### C-31 — GAP-23 cite deux rejeux disparus

- **Origine** : note 5d (« Registre des écarts, entrées signalées »).
- **Constat** : GAP-23 cite `events_recorded_1790198294` et `session_events`, absents de `tests/e2e/`.
- **Proposition** : retirer la parenthèse qui les nomme ; le reste de l'entrée est juste (fixé par `test_nil_keys`).
- **Verdict proposé** : `proposition`
- **Verdict** : ok

#### C-32 — Choix faits sans vous sur le registre des écarts (5d)

- **Origine** : PRD 5d, « Décisions prises sans humain » 6 à 10.
- **Constat** : chaque entrée a une ligne « Lua : » sous son titre, seul endroit des noms du cart ; le fichier garde son nom `trous-de-portage.md` ; la réécriture a changé les noms, pas le fond (rien n'a été revérifié) ; les skills sont passés de `TROU` à `GAP` en 5d ; huit noms sans ligne au TSV sont décrits par leur rôle.
- **Proposition** : confirmer. Rien à éditer.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

### Thème 6 — Docs et skills (6)

#### C-33 — Relecture : l'exemple travaillé du skill

- **Origine** : note 5f (« Pour l'humain ») ; commits 1 et 5 de 5f.
- **Constat** : `.claude/skills/reprendre-les-commentaires/exemple-travaille.md` a changé à quatre endroits : lignes 23-31 (le glossaire « remplacé depuis » par le plan de renommage ; « quatre destinations à l'époque, trois depuis ») ; ligne 47 et lignes 59-74 (noms C à la place des noms disparus : `p8_fixed_cos/_sin`, `tickets_count`, `p8_sub`, `p8_input_save` ; `unstash()` gardé, c'est un nom Lua) ; lignes 95-97 (ratios, modules nommés par leur rôle).
- **Proposition** : relire ces lignes ; si elles vous vont, rien à éditer.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-34 — Relecture : la section neuve de la consigne (D-65)

- **Origine** : note 5f (« Pour l'humain ») ; commit 6 de 5f ; D-65.
- **Constat** : `consigne-agent.md`, lignes 126-147, nouvelle section « Quand un commentaire n'en est pas tout à fait un » : quatre cas où un commentaire est lu par une machine (aide imprimée par `sed`, `#` d'ici-document, programme `awk` entre apostrophes, fichier comparé par numéros de ligne).
- **Proposition** : relire ; si elle vous va, rien à éditer.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-35 — « Quatre destinations » dans le skill, trois dans ce dépôt

- **Origine** : note 5f (« Pour l'humain »), PRD 5f (décision 11).
- **Constat** : le skill parle de quatre destinations (`SKILL.md:3`, `:48-55` ; `consigne-agent.md:19-40` ; rubrique « glossaire » de `modele-rapport.md:62-67`). C'est juste en général : le glossaire est une destination ailleurs. Une seule phrase est fausse ici : `SKILL.md:23` dit que la règle des quatre destinations est déjà dans le fichier d'instructions du dépôt ; notre `CLAUDE.md` en a trois.
- **Proposition** : réécrire `SKILL.md:23` seule : « la règle des destinations (quatre, ou trois si le dépôt n'a plus de glossaire) est déjà dans le fichier d'instructions du dépôt ». Le reste du skill ne change pas.
- **Verdict proposé** : `proposition`
- **Verdict** : ok

#### C-36 — `docs/psx-hello-world.md` cite un hook disparu

- **Origine** : note 5f (« Pour l'humain »), PRD 5f (décision 13).
- **Constat** : la ligne 43 décrit `.claude/hooks/deny-absolute-paths.py`, qui n'existe plus ; le fichier est aussi en anglais, contre « les docs restent en français ».
- **Proposition** : retirer la puce du hook (`docs/` ne dit que ce qui est vrai). La langue reste : PSX est en pause, la traduction attendra sa reprise.
- **Verdict proposé** : `proposition`
- **Verdict** : ok

#### C-37 — Choix faits sans vous sur les docs (5f)

- **Origine** : PRD 5f, « Décisions prises sans humain » 1 à 17.
- **Constat** : le glossaire est supprimé ; `docs/vocabulaire.md` n'a changé qu'à sa ligne 5 ; la tâche « Renommer les champs » du `README` est retirée (faite) ; `CLAUDE.md` a gagné les règles de nommage en trois lignes (le plan, seule source, sera supprimé) ; `docs/design/slipways-mechanics.md` reste en noms Lua (il décrit le cart) ; aucune garde ne surveille les docs, la sortie de 5f est une photographie.
- **Proposition** : confirmer. Rien à éditer.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

#### C-38 — La fin du chantier

- **Origine** : note 5f (« Pour l'humain ») ; entrée 5i du plan.
- **Constat** : le sort du plan, de ce registre, de `docs/vocabulaire.md`, des outils `rename_*` et de `tools/ast_rename`, et des 12 fichiers de `tools/` qui citent `docs/plan-5.md`, reste à fixer.
- **Proposition** : rien ici : l'entrée 5i fixe chacun par verdict, après ce point d'arrêt.
- **Verdict proposé** : `garder tel quel`
- **Verdict** : ok

### Bilan

38 points : thème 1, 2 ; thème 2, 10 ; thème 3, 5 ; thème 4, 10 ; thème 5, 5 ; thème 6, 6. Verdicts proposés : `proposition` 15 (C-1, C-3, C-4, C-8, C-9, C-10, C-13, C-16, C-24, C-28, C-29, C-30, C-31, C-35, C-36), dont 14 qui éditent le dépôt (C-1 n'édite que ce registre) ; `garder tel quel` 22 ; `reporter hors plan 5` 1 (C-22). Les 18 alertes de la `warn-review` : 1-3 → C-18, 4 → C-19, 5-10 → C-20, 11 → C-21, 12 → C-22, 13 → C-23, 14 → C-24, 15 → C-25, 16 → C-26, 17 → C-8, 18 → C-9. Entrées GAP signalées par 5d : GAP-3 → C-28, GAP-9 → C-29, GAP-4 et GAP-11 → C-30, GAP-23 → C-31.
