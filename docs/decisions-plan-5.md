# Registre des décisions humaines du plan 5

**Condition d'entrée de l'étape 5** (`docs/plan-5.md`, §6, entrée 5) : chaque entrée ci-dessous porte un verdict humain. Le registre est ouvert par 4a à partir des notes « Découvert » de 0 à 3l, complété par chaque sous-étape 4x, clos par 4l. Aucune entrée n'est appliquée avant son verdict : l'étape 3 et l'étape 4 ont appliqué tels quels les noms en cause.

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

## Index

| n° | Origine | Question | Sites |
|---|---|---|---:|
| D-1 | 2b, 3l | Repli `tmp/pico8_replay` de `tools/frames_ref.sh` | 1 |
| D-2 | 3k, 3l | `PICO8_CLASS_COVERAGE`, macro `-D` hors TSV | 35 |
| D-3 | 3a, 3l | `PICO8_MALLOC_FILLS`, variable d'environnement | 2 |
| D-4 | 3l | Noms `pico8_*` des fixtures de `test_alloc_guard.c` | 36 |
| D-5 | 3a, 3l | Motif générique `void-ptr` (`pico8_X(`) | 0 prototype |
| D-6 | 3a | `LIEN_EXCLUSION` mort | 2 |
| D-7 | 3a | Motif générique `pico8_X_of` (`lien-parent`) | 0 |
| D-8 | 3a | Motif générique `pico8_X_start` (`défauts`, `--champs`) | 0 |
| D-9 | 3a, 3b, 3d | Motif `spec` `pico8_X_(default\|defaults_reset)` et `SPEC_EXCLUSION` mort | 0 + 2 |
| D-10 | 3b | Exclusion `usage-mutuel` (`PICO8`, `P8`) morte | 0 |
| D-11 | 3a | Catégories historiques des gardes à motifs | 9 catégories |
| D-12 | 3e | Suffixe de constructeur de `--champs` | 5 |
| D-13 | 3a, 3l | 12 statiques locales de deux lettres des tests | 12 |
| D-14 | 3c | Ordre des mots des macros de taille de zone | 12 |
| D-15 | 3f | `probe_own` stocke un `targeting_t` | 24 |
| D-16 | 3f | `probe_link_target` déclaré hors de son module | 4 |
| D-17 | 3d, 3e, 3f, 3i, 3j | Noms qui bégaient | 60 |
| D-18 | 3f | Étiquette morte `main_mouse` de `test_main_mouse.c` | 1 |
| D-19 | 3g | `planet_upkeep` sans le préfixe `economy_score_` | 11 |
| D-20 | 3h | Singletons `main_*` déclarés hors d'un module `main_*` | 207 |
| D-21 | 3i, 3j | Préfixe `g_` des tables `static const` de classe | 143 |
| D-22 | 3j | Suffixe `_t` sur deux variables | 9 |
| D-23 | 3j | Faute `test_update_entities_drives_a_entity_struct` | 2 |
| D-24 | 3j | `DIFFICULTY_OPTIONS_ROW_CAP` sans son module | 3 |
| D-25 | 0, 2b, 3k, 3l | `p8_input_*` et `p8_fatal` définis hors de `p8/` | 5 fonctions |
| D-26 | 3g, 3h | Lignes `chaîne` `pz` et `go` éditées hors de `rename-strings` | 2 |
| D-27 | 3k | Motif `pico8_class_coverage_record` de `tools/rename_strings.py` | 1 |
| D-28 | §6 « Outillage figé » | Alertes `warn-review` de fin de tronçon 2 | 4 alertes connues |
| D-29 | PRD 4a | Paramètres et locales courts nommés d'un digramme du cart | 131 paramètres de méthode |
| D-30 | 2b (échantillon) | `typed_main_*` des tests contre `main_*_typed` | 96 |
| D-31 | 2b (échantillon) | Digramme `oc` : `tickets` et `type_picker` | 3 |
| D-32 | 2b (échantillon) | Champs de l'icône de structure : `rest_count` est une position | 22 |
| D-33 | 4a | Exceptions du compteur de français : schéma du TSV et fixtures françaises (3a) | 26 |
| D-34 | 4a | Assertion faible `strstr(log, "exempt")` de `test_class_coverage.c` | 1 |
| D-35 | 4b | Noms d'identifiant qui citent Lua ou le cart (`P8_FIXED_LIT`, trois tests) | 8 |
| D-36 | 4b | `` `mem` `` et `` `gfx` `` comptés en `backtick` après le renommage des locales de `test_demo.c` | 23 |
| D-37 | 4c1 | `argc`/`argv` de `main` dans la liste des locales cryptiques | 10 déclarations |
| D-38 | 4c1 | Statiques de fichier `mem`/`gfx` des tests | 80 déclarations |
| D-39 | 4d | Nom de test qui cite le cart (`test_data_init.c`) | 2 |
| D-40 | 4d | Clés du cart sans ligne `chaîne` au TSV (`"br"`, `"qr"`, `"rf"`, `"s"`, `"qh+"`) | 0 après 4d |
| D-41 | 4d | Nom de test qui garde le digramme `r` (`test_map_tables.c`) | 2 |
| D-42 | 4e | Ligne du TSV d'une locale renommée deux fois | 14 lignes |
| D-43 | 4e | Paramètres `gfx`/`mem` des fonctions de dessin | 96 + 128 déclarations |
| D-44 | 4e | Champ `planet_inspection_panel_ctx_t.unmet_needs`, qui tient les besoins reçus | 7 + 1 |
| D-45 | 4f1 | Noms de test qui disent « grab arena » pour une prise (`test_live_slots.c`) | 16 + fixture |
| D-46 | 4f2 | Fonctions de `main_economy.h` définies dans `probe.c` (`probe_set_main_economy`) | 3 + 159 |
| D-47 | 4g | `structure_icon_sprite_counter` compte des identifiants de planète, pas des sprites | 4 + 5 + 5 |
| D-48 | 4h | Noms de test qui décrivent l'API générique disparue ou la méthode du cart (`test_ui.c`, `test_main_mouse.c`, `test_main_camera.c`) | 2 + 2 + 4 + 8 |
| D-49 | 4i | Assistants de test qui citent un digramme du cart ou un surnom (`planet_by_ref`, `build_plate`, `fold_plate`, `is_plate`) | 2 + 6 + 3 + 3 |

## Entrées

### D-1 — Repli `tmp/pico8_replay` de `tools/frames_ref.sh`

- **Origine** : 2b (binaires, voie (a)), 3l (« Fin de l'étape 3 », seule `string` restante de `cart-names-count`).
- **Question** : l'ancien nom de binaire gardé exprès par `frames_ref.sh`, pour reconstruire un commit d'avant la règle `$(TMP_DIR)/replay:`, devient-il une exception permanente de la garde anti-cart ?
- **Options** : (a) garder le repli et l'excepter ; (b) retirer le repli : `make e2e-ref` ne sait plus rejouer une référence posée avant `2bb367d` (3l), soit toutes les références actuelles de `tests/e2e/`.
- **Sites** : 1 en code (`tools/frames_ref.sh:113`, `driver=tmp/pico8_replay`) ; 3 en commentaire (`src/cmd/frames.c:1`, `tests/test_replay_from_title.c:1`, `tests/test_headless_capture.c:5`), repris par 4k et 4l.
- **Proposition prête à appliquer** : (a). À l'étape 5, quand la garde devient bloquante, `tools/rename_count.py` lit un fichier `tools/cart_names_exceptions.txt` au format de `tools/scope_exceptions.txt` (`colonne<TAB>module<TAB>nom`, « seule une décision humaine ajoute une ligne »), avec l'unique ligne `string	tools/frames_ref	tmp/pico8_replay` ; changement d'outil, donc test d'abord dans `tests/test_rename_count.py` (une ligne retire exactement son triplet).
- **Contrôle** : `make cart-names-count` sans `SUBSTEP` : `TOTAL` 0 dans les quatre colonnes.
- **Verdict** :

### D-2 — `PICO8_CLASS_COVERAGE`

- **Origine** : 3k (« Pour décision humaine »), 3l.
- **Question** : la macro de compilation de la couverture de classes garde le préfixe du cart, interdit par le §2 ; aucune ligne du TSV ne la porte (un `-D` sans `#define`), aucun compteur ne la voit. La renommer, et en quoi ?
- **Options** : (a) `CLASS_COVERAGE_BUILD` ; (b) `CLASS_COVERAGE` ; (c) garder.
- **Sites** : 35 — 31 dans `src/` (21 fichiers de `game/` et `devtools/`), 2 dans `tools/float_grep_exclusions.txt` (texte), 2 dans le `Makefile` (le `-D` de `COVERAGE_CFLAGS`, ligne 232, et un commentaire, ligne 971). La configuration `coverage` de `rename-extract` (`AST_RENAME_ANALYSIS`) lit `COVERAGE_CFLAGS` : un seul point de définition.
- **Proposition prête à appliquer** : (a), qui ne heurte aucun nom (0 site). Hors AST (pas d'identité) : `grep -rl PICO8_CLASS_COVERAGE src tools/float_grep_exclusions.txt Makefile | xargs sed -i 's/PICO8_CLASS_COVERAGE/CLASS_COVERAGE_BUILD/g'` (sans `\b` : le `-D` colle la macro à l'option) ; commit `refactor:`.
- **Contrôle** : `make class-coverage` et `make test-all` verts ; `make rename-extract` sans `warning:` (les 4 lignes `partial` de `class_coverage` attendues) ; `make float-grep` vert ; `grep -rn PICO8_CLASS_COVERAGE src tests tools Makefile` hors outils du dictionnaire rend 0.
- **Portée** : s'applique aussi aux mentions en commentaire et entre backticks.
- **Verdict** :

### D-3 — `PICO8_MALLOC_FILLS`

- **Origine** : 3l (« Fin de l'étape 3 ») ; PRD de 4a (laissée en attente de ce verdict).
- **Question** : la variable d'environnement de `tests/test_entity_header.c`, au préfixe du cart, hors TSV, que le compteur ne voit pas.
- **Options** : (a) `MALLOC_FILLS` ; (b) `ENTITY_HEADER_MALLOC_FILLS` ; (c) garder.
- **Sites** : 2, tous deux dans `tests/test_entity_header.c` (`getenv`, ligne 47 ; `setenv`, ligne 53) : lue et écrite par le même fichier.
- **Proposition prête à appliquer** : (a). `sed -i 's/\bPICO8_MALLOC_FILLS\b/MALLOC_FILLS/g' tests/test_entity_header.c` ; commit `test:`.
- **Contrôle** : `make test` vert (le test se relance lui-même avec la variable) ; `grep -rnw PICO8_MALLOC_FILLS src tests tools Makefile` rend 0.
- **Portée** : s'applique aussi aux mentions en commentaire.
- **Verdict** :

### D-4 — Noms `pico8_*` des fixtures de `test_alloc_guard.c`

- **Origine** : 3l (« Fin de l'étape 3 ») ; élargi par la mesure de 4a aux autres noms des mêmes fixtures.
- **Question** : `tests/test_alloc_guard.c` écrit et compile des sources fictives aux noms préfixés du cart ; hors TSV, le compteur ne les voit pas. `tools/alloc_guard.sh` et `tools/alloc_guard_exceptions.txt` n'en citent aucun.
- **Options** : (a) retirer le préfixe ; (b) garder (fixtures).
- **Sites** : 36 — fichiers `pico8_allocs.c` 15, `pico8_clean.c` 6, `pico8_gone.c` 2, `pico8_transient_user.c` 4 ; symboles `pico8_zone_transitoire` 4 (français), `pico8_malloc_count`, `pico8_grow`, `pico8_half`, `pico8_kept`, `pico8_use` 1 chacun.
- **Proposition prête à appliquer** : (a). Dans `tests/test_alloc_guard.c` seulement : `pico8_allocs` → `allocs`, `pico8_clean` → `clean`, `pico8_gone` → `gone`, `pico8_transient_user` → `transient_user`, `pico8_zone_transitoire` → `transient_zone`, `pico8_malloc_count` → `malloc_count`, `pico8_grow` → `grow`, `pico8_half` → `half`, `pico8_kept` → `kept`, `pico8_use` → `use` ; un `sed` par paire, mot entier ; commit `test:`.
- **Contrôle** : `make test` vert (`test_alloc_guard` compris) ; `grep -c 'pico8_' tests/test_alloc_guard.c` rend 0.
- **Portée** : s'applique aussi aux mentions en commentaire du même fichier.
- **Verdict** :

### D-5 — Motif générique `void-ptr` (`pico8_X(`)

- **Origine** : 3a (inventaire des gardes à motifs), 3l.
- **Question** : le motif de `composition_residue.sh` (`(^|[^A-Za-z0-9_])pico8_[a-z0-9_]+\(`, ligne 423) ne reconnaît plus aucun prototype : il passe à vide sur l'arbre, ses fixtures l'exercent encore. Le traduire sans préfixe, ou le déclarer intraduisible ?
- **Options** : (a) traduire sans préfixe (`[a-z0-9_]+\(`) si `--detail` ne bouge pas ; (b) intraduisible : garder tel quel, fixtures comprises ; (c) retirer la catégorie et ses fixtures.
- **Sites** : 0 prototype dans les en-têtes de `src/` ; 188 mentions `pico8_X(` restent en commentaire (étape 4). Sans préfixe, le motif verrait toute ligne d'en-tête qui ouvre une parenthèse après un nom en minuscules : 931 lignes.
- **Proposition prête à appliquer** : (a) ne peut pas laisser `--detail` identique : (b), motif et fixtures de `tests/test_composition_residue.c` gardés tels quels.
- **Contrôle** : procédure G.
- **Verdict** :

### D-6 — `LIEN_EXCLUSION` mort

- **Origine** : 3a.
- **Question** : `LIEN_EXCLUSION='s/entity_state_of//g'` (`composition_residue.sh:72`, lu ligne 79) n'a rien à mordre : ni `\bpico8_[a-z0-9_]+_of\b` ni `(emitter|probe)_of` n'apparient `entity_state_of`.
- **Options** : (a) retirer ; (b) garder pour le jour où `lien-parent` est traduit sans préfixe (D-7), qui apparierait de nouveau `entity_state_of`.
- **Sites** : 2 (définition, emploi).
- **Proposition prête à appliquer** : suivre D-7 — retirer si D-7 garde le préfixe ou retire la catégorie ; garder si D-7 traduit sans préfixe (l'exemption redevient vivante, test de vivacité à écrire).
- **Contrôle** : procédure G.
- **Verdict** :

### D-7 — Motif générique `pico8_X_of` (`lien-parent`)

- **Origine** : 3a.
- **Question** : la première branche de `lien-parent` (`\bpico8_[a-z0-9_]+_of\b`, `composition_residue.sh:59`) ne voit plus rien.
- **Options** : (a) traduire sans préfixe (`\b[a-z0-9_]+_of\b`) si `--detail` ne bouge pas ; (b) intraduisible, gardé ; (c) retirer la branche.
- **Sites** : 0 nom `pico8_X_of` dans `src/` ; sans préfixe, 21 occurrences y seraient vues (`slot_kind_of` 4, `pos_of` 4, `slipway_infraspace_of` 3, `resource_code_of` 3, `flip_zone_of` 3, `map_pos_of` 2, `entity_state_of` 2).
- **Proposition prête à appliquer** : (a) ne peut pas laisser `--detail` identique (21 occurrences nouvelles) : (b), la branche gardée telle quelle, et D-6 retiré.
- **Contrôle** : procédure G.
- **Verdict** :

### D-8 — Motif générique `pico8_X_start` (`défauts`, `--champs`)

- **Origine** : 3a.
- **Question** : `défauts` (`\bpico8_[a-z0-9_]+_start(_t)?\b`, ligne 61) et le relevé `--champs` (ligne 676) ne voient plus aucun archétype (`pico8_X_def` est devenu `X_archetype`).
- **Options** : (a) traduire en `\b[a-z0-9_]+_archetype(_t)?\b`, le nom réel des archétypes, si `--detail` ne bouge pas ; (b) intraduisible ; (c) retirer.
- **Sites** : 0 `pico8_X_start` dans `src/` ; `_start` sans préfixe : 3 ; `\b[a-z0-9_]+_archetype(_t)?\b` : 212 occurrences dans `src/`.
- **Proposition prête à appliquer** : (a) ne peut pas laisser `--detail` identique (212 occurrences nouvelles pour `défauts`) : (b), motif gardé tel quel. Le relevé `--champs` (ligne 676) reste un majorant.
- **Contrôle** : procédure G.
- **Verdict** :

### D-9 — Motif `spec` et `SPEC_EXCLUSION`

- **Origine** : 3a, 3b, 3d.
- **Question** : la branche `\bpico8_[a-z0-9_]+_(default|defaults_reset)\b` de `spec` (`data_residue.sh:38`) ne voit plus rien ; `SPEC_EXCLUSION` (ligne 49, lu ligne 94), traduit par 3b et 3d, est mort en entier (vidé : 0 ligne de `spec`, aucun test de vivacité possible).
- **Options** : (a) retirer `SPEC_EXCLUSION` et garder la branche ; (b) traduire la branche sans préfixe (`\b[a-z0-9_]+_(default|defaults_reset)\b`) : mesuré par 3d, elle voit les 7 noms exemptés de `src/` et les moitiés recollées du témoin de `tests/test_data_residue.c` (`spec` y monterait à 6 lignes), il faudrait recouper les témoins.
- **Sites** : branche 0 ; `SPEC_EXCLUSION` 2 (définition, emploi) ; témoins de `test_data_residue.c` (3d : 6 lignes).
- **Proposition prête à appliquer** : (a) — supprimer les lignes 49 et 94 (le `sed` devient sans objet : `grep` direct) ; les témoins restent.
- **Contrôle** : procédure G ; `make data-guard` vert.
- **Verdict** :

### D-10 — Exclusion `usage-mutuel` morte

- **Origine** : 3b.
- **Question** : `usage-mutuel` écarte les jetons `PICO8` et `PICO8_` (`composition_residue.sh:522`) ; aucun jeton `PICO8`, `PICO8_`, `P8` ou `P8_` n'est déclaré, ni avant ni après 3b.
- **Options** : (a) retirer l'exclusion ; (b) la garder.
- **Sites** : 0 jeton déclaré.
- **Proposition prête à appliquer** : (a) — retirer `x == "PICO8" || x == "PICO8_" ||` de la ligne 522.
- **Contrôle** : procédure G (`--detail` identique, `composition-guard` vert).
- **Verdict** :

### D-11 — Catégories historiques des gardes à motifs

- **Origine** : 3a (classe « historique » de l'inventaire).
- **Question** : neuf catégories ne reconnaissent que du code disparu avant le plan 5 et restent à zéro : `nu`, `aiguillage-nom`, `un-ou-faux` (`composition_residue.sh`, lignes 62–64, et `NU_EXCLUSION`) ; `valeur`, `ob`, `stash`, `type-tag`, `ref-audit`, `liste` (`data_residue.sh`). Toutes sont dans `COMPOSITION_RESIDUE_GUARD` ou `DATA_RESIDUE_GUARD` du `Makefile` et ont des fixtures dans `tests/test_composition_residue.c` et `tests/test_data_residue.c`.
- **Options** : (a) retirer chaque catégorie, son entrée de la liste de garde et ses fixtures ; (b) garder telles quelles.
- **Sites** : 9 catégories.
- **Proposition prête à appliquer** : (a). Leurs motifs citent des noms `pico8_*` qu'aucun code ne peut plus porter une fois la garde anti-cart bloquante (étape 5) : elles ne gardent plus rien. Un commit `refactor:` par script.
- **Contrôle** : procédure G — `--detail` égal à celui d'avant moins les lignes de ces catégories ; `make test` vert.
- **Verdict** :

### D-12 — Suffixe de constructeur de `--champs`

- **Origine** : 3e.
- **Question** : avec le suffixe `(_new|_init|_create|_init_instance)$`, `entity_init_header` (3a) et `allocator_init_slot_kinds` (3c) sortent des constructeurs, `emitter_init` (3f), `weighted_picker_new` et `random_walk_new` (3d) y entrent.
- **Options** : (a) garder le suffixe ; (b) une liste explicite de constructeurs.
- **Sites** : 5 fonctions.
- **Proposition prête à appliquer** : (a), rien à éditer : `--champs` est un majorant par nom, pas une porte (pied « 0 hors constructeur » 27, « 0 écriture » 15 à la fin de 3l).
- **Contrôle** : aucun.
- **Verdict** :

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
- **Verdict** :

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
- **Verdict** :

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
- **Verdict** :

### D-16 — `probe_link_target` hors de son module

- **Origine** : 3f.
- **Question** : `probe_link_target` est déclaré dans `targeting.h`, défini dans `probe.c`.
- **Options** : (a) garder (§7 : pas de redécoupage pendant le plan 5) ; (b) déplacer la déclaration dans `probe.h` ; (c) renommer `targeting_link_target` et déplacer la définition dans `targeting.c`.
- **Sites** : 4.
- **Proposition prête à appliquer** : (a), rien à éditer ; le redécoupage est noté pour un chantier hors du plan 5.
- **Verdict** :

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
- **Complément (4e)** : le libellé `"update_vanishing_update_entities_calls"` de `check_int()` (`tests/test_planet.c`) est le nom de `UPDATE_VANISHING_UPDATE_ENTITIES_CALLS` en minuscules (règle des libellés, consigne point 7) : si (a) est appliquée, il devient `"update_vanishing_calls"` dans le même commit.
- **Complément (4f2)** : `emitter_init_help_screen_takes_a_particles_help_screen_list` est un nom de cas : la fonction est `case_emitter_init_help_screen_takes_a_particles_help_screen_list` (`tests/test_particle_zone.c`), et la chaîne de `CASES[]` et celle de son `expect_clean` suivent la fonction (règle des noms de cas, consigne point 7) ; sous (a), trois sites d'un coup : la fonction devient `case_emitter_init_help_screen_takes_a_help_list`, ses deux chaînes `"emitter_init_help_screen_takes_a_help_list"` (lignes 330 et 423 à la fin de 4f2), et l'identité de la ligne du tableau ci-dessus est `c:test_particle_zone.c@F@case_emitter_init_help_screen_takes_a_particles_help_screen_list`.
- **Verdict** :

### D-18 — Étiquette morte `main_mouse` de `test_main_mouse.c`

- **Origine** : 3f.
- **Question** : `tests/test_main_mouse.c:45`, `tags[] = {"main_mouse", "mouse", "ui", "camera", "entity"}` : aucune classe n'écrit l'étiquette `main_mouse`, sa recherche rend toujours une liste vide.
- **Options** : (a) la retirer du tableau ; (b) garder.
- **Sites** : 1.
- **Proposition prête à appliquer** : (a), commit `test:` ; vérifier d'abord que la boucle sur `tags` ne dépend pas de sa taille (un compte attendu de lignes imprimées).
- **Contrôle** : `make test` vert.
- **Verdict** :

### D-19 — `planet_upkeep` sans le préfixe `economy_score_`

- **Origine** : 3g.
- **Question** : `planet_upkeep` (déclarée dans `economy_score.h`) reste seule sans le préfixe de son module, alors que `economy_score_net_income` prend aussi une planète.
- **Options** : (a) `economy_score_planet_upkeep` ; (b) garder (méthode du type receveur, §4).
- **Sites** : 11.
- **Proposition prête à appliquer** : (a), procédure N : `planet_upkeep	economy_score_planet_upkeep	src/game/economy_score.h	c:@F@planet_upkeep` (nom neuf : 0 site).
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire ; contrôle sur `planet_upkeep`.
- **Verdict** :

### D-20 — Singletons `main_*` hors d'un module `main_*`

- **Origine** : 3h.
- **Question** : `main_ui`, `main_mouse`, `main_mouse_typed`, `main_mouse_placeholder_class` sont déclarés dans `ui.h`, `main_camera_typed` dans `scroll.h` (`main_economy*` ont leur `main_economy.h`).
- **Options** : (a) garder (noms du vocabulaire) ; (b) déplacer leurs déclarations dans des en-têtes `main_*` (redécoupage, §7).
- **Sites** : 207 — `main_ui` 111, `main_mouse` 18, `main_mouse_typed` 28, `main_mouse_placeholder_class` 3, `main_camera_typed` 47.
- **Proposition prête à appliquer** : (a), rien à éditer.
- **Verdict** :

### D-21 — Préfixe `g_` des tables `static const` de classe

- **Origine** : 3i, 3j (question transverse).
- **Question** : le §4 réserve `g_` aux statiques mutables de fichier ; les tables de classe et de sorte sont des `static const` préfixées `g_`.
- **Options** : (a) retirer `g_` en suffixant `_def` : `g_X_class` → `X_class_def`, `g_X_kind` → `X_kind_def` (sans heurter les accesseurs `X_class()`, `X_kind()`) ; (b) garder, et préciser le §4 (« `g_` pour toute statique de fichier ») ; un verdict pour toutes.
- **Sites** : 54 déclarations `static const entity_class_t g_…` (45) et `static const entity_kind_t g_…` (9), dans 25 fichiers, 53 noms distincts, 143 occurrences. Hors question : 7 autres `static const` préfixées `g_` (tables de données, `button_fn_t`…), que le même verdict couvrirait s'il est (a).
- **Proposition prête à appliquer** : (a) ; aucun nom en `_def` n'existe (0 site). Procédure N, une ligne par déclaration, identités `c:<fichier>.c@g_X_class` à relever (statiques de fichier).
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire ; contrôle `grep -rnE '\bg_[a-z0-9_]+_(class|kind)\b' src tests` rend 0.
- **Verdict** :

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
- **Verdict** :

### D-23 — Faute `test_update_entities_drives_a_entity_struct`

- **Origine** : 3j.
- **Question** : « a entity ».
- **Options** : (a) `test_update_entities_drives_an_entity_struct` ; (b) garder.
- **Sites** : 2 (`tests/test_toast.c`).
- **Proposition prête à appliquer** : (a), procédure N : `test_update_entities_drives_a_entity_struct	test_update_entities_drives_an_entity_struct	tests/test_toast.c	c:test_toast.c@F@test_update_entities_drives_a_entity_struct`.
- **Verdict** :

### D-24 — `DIFFICULTY_OPTIONS_ROW_CAP` sans son module

- **Origine** : 3j.
- **Question** : macro locale de `help.c` sans le nom de son module (§4 : `MODULE_UPPER_CASE`).
- **Options** : (a) `HELP_DIFFICULTY_OPTIONS_ROW_CAP` ; (b) garder (macro locale d'un `.c`).
- **Sites** : 3.
- **Proposition prête à appliquer** : (a), procédure N : `DIFFICULTY_OPTIONS_ROW_CAP	HELP_DIFFICULTY_OPTIONS_ROW_CAP	src/game/help.c	macro:src/game/help.c:DIFFICULTY_OPTIONS_ROW_CAP` (0 site pour le nom neuf). Si (a), `DIFFICULTY_OPTIONS_COUNT` du même fichier suit le même verdict (à mesurer).
- **Verdict** :

### D-25 — `p8_input_*` et `p8_fatal` définis hors de `p8/`

- **Origine** : 0 (vocabulaire), 2b, 3k, 3l.
- **Question** : `p8_input_load`, `p8_input_save`, `p8_input_append`, `p8_input_free` sont déclarés dans `p8/input.h` et définis dans `devtools/input_json.c` ; `p8_fatal` est déclaré dans `p8/fatal.h` et défini dans `platform/fatal_pc.c`.
- **Options** : (a) garder (scission d'en-tête hors périmètre, §7) ; (b) scinder `p8/input.h`.
- **Sites** : 5 fonctions (occurrences : 28, 12, 8, 34, 6).
- **Proposition prête à appliquer** : (a), rien à éditer.
- **Verdict** :

### D-26 — Lignes `chaîne` `pz` et `go` éditées hors de `rename-strings`

- **Origine** : 3g (`pz`), 3h (`go`).
- **Question** : ces deux lignes sont passées à `conservé` dans `docs/renommage.tsv` seulement ; une régénération par `make rename-strings` les remettrait à `renommé`.
- **Options** : (a) les reporter en surcharges ; (b) les laisser (le TSV ne sera plus régénéré).
- **Sites** : 2 lignes.
- **Proposition prête à appliquer** : (a), deux lignes de `tools/rename_overrides.tsv` (forme `string`, lue par `rename_derive.read_overrides`) : `string	pz	=` et `string	go	=`.
- **Contrôle** : `make rename-strings` sur l'arbre redonne `conservé` aux deux lignes ; `git diff docs/renommage.tsv` vide pour elles.
- **Verdict** :

### D-27 — Motif `pico8_class_coverage_record` de `tools/rename_strings.py`

- **Origine** : 3k.
- **Question** : `CLASS_RECORD = re.compile(r"\bpico8_class_coverage_record\(\s*$")` (`tools/rename_strings.py:381`) n'apparie plus rien depuis 3k.
- **Options** : (a) `\bclass_coverage_record\(` ; (b) garder (l'outil ne sera plus lancé).
- **Sites** : 1.
- **Proposition prête à appliquer** : (a), avec ses fixtures de `tests/test_rename_strings.py` ; `make rename-strings-test` vert.
- **Verdict** :

### D-28 — Alertes `warn-review` de fin de tronçon 2

- **Origine** : §6, « Méthode des sous-étapes », puce *Outillage figé*. Le relevé complet des alertes n'a pas été versé dans le dépôt ; les quatre que le plan nomme sont remesurées ici. Les autres demandent une nouvelle passe de `/warn-review`, que seul un humain lance.
- **Question** : un verdict par alerte.
- **Sites et propositions** :
  - *Lecteurs en double du TSV* : 4 fonctions `read_table` (`tools/rename_check.py:94`, `tools/rename_count.py:77`, `tools/rename_strings.py:130`, `tools/ast_rename/plan.py:47`). Proposition : un lecteur, `rename_count.read_table`, les trois autres l'importent ; tous les `rename-*-test` et `rename-tool-test` verts.
  - *Listes de mots-clés divergentes* : `C_KEYWORDS` de `tools/rename_check.py` (59 mots) et de `tools/ast_rename/plan.py` (44) ; les 15 de plus sont `_BitInt`, `_Decimal32/64/128`, `alignas`, `alignof`, `bool`, `constexpr`, `false`, `nullptr`, `static_assert`, `thread_local`, `true`, `typeof`, `typeof_unqual`. Proposition : une liste, celle de `rename_check.py`, importée par `plan.py`.
  - *Schéma du TSV en français* : colonnes `ancien`, `nouveau`, `fichier`, `catégorie`, `sorte`, `statut`, `module`, `concept`, `note`, `identité`, valeurs `renommé`, `conservé`, `identifiant`, `chaîne`, `écrivain`, `lecteur`… Le TSV est une doc (§2, en français), mais les outils et leurs tests le lisent : `make french-count SUBSTEP=3a` compte ces mots dans `tests/test_scope_count.py` et `tests/test_cart_citations.py`. Proposition : garder le schéma (le TSV est le pont inverse documenté en français) et excepter ces mots au compteur de français (lignes à porter par la note de 4a).
  - *Version de libclang fixée à deux endroits* : `CLANG_BINDINGS_VERSION=18.1.8` (`tools/install_ast_rename_bindings.sh:5`) et `LIBCLANG ?= /usr/lib/llvm-18/lib/libclang-18.so.1` (`Makefile:1178`). Proposition : le `Makefile` passe la version majeure au script (`CLANG_MAJOR := 18`), une seule source.
- **Verdict** (un par alerte) :

### D-29 — Paramètres et locales courts nommés d'un digramme du cart

- **Origine** : PRD de 4a (mesure).
- **Question** : le §1 tolère une locale ou un paramètre de deux lettres à portée ≤ 40 lignes ; beaucoup portent encore le digramme du cart (`ra`, `qa`, `me`…), même là où le symbole homonyme a été renommé. Les renommer ?
- **Options** : (a) garder (§1) ; (b) renommer la seule famille des paramètres de méthode de la vtable et de leurs implémentations ; (c) renommer toutes les déclarations concernées.
- **Sites** : 131 paramètres de la famille de méthode dans l'extraction (`ra` 64, `qa` 24, `me` 12, `yr` 7, `fy` 7, `sw` 6, `lw` 6, `kv` 5) ; plus largement, 1 274 locales et paramètres de deux lettres dont le nom est l'ancien nom d'une ligne `renommé` du TSV (3a 26, 3b 41, 3c 74, 3d 75, 3e 214, 3f 169, 3g 150, 3h 204, 3i 194, 3j 107, 3k 12, 3l 8). Le chiffre de 713 de la PRD de 4a (prototype) n'est pas reproduit ; la définition retenue est celle-ci, sur `tmp/ast_rename/declarations.tsv`.
- **Proposition prête à appliquer** : (b), sens **à confirmer** : `qa` → `mouse_world`, `ra` → `mouse_screen` (le `main_mouse_pos` passé par `ui.c` à `hit_test` ; `mouse_pos` serait l'autre choix), `yr` → `year`, `sw` → `slipway`, `lw` → `offer`, `kv` → `dropped`, `fy` → `parallax`, `me` → `state`. Lignes `field <fonction>.<param>` de `tools/rename_overrides.tsv` et table `rename-apply` depuis une extraction fraîche. Mesuré : aucun nom proposé ne masque une locale ou un paramètre de la même fonction, ni une fonction, globale, macro ou type.
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire ; contrôle : `grep -rnwE '(qa|ra|yr|sw|lw|kv|fy|me)' src tests` ne rend plus de paramètre de méthode.
- **Complément (4h)** : 4h renomme les paramètres `qa`/`ra` du prototype de `ui_send_to_hovered` (`src/game/ui.h`) aux noms de sa définition, `mouse_world`/`mouse_screen` (règle de 4e : le prototype suit sa définition, dont le `pos` était une locale cryptique de portée 47) : `mouse_screen` est le nom que (b) propose pour `ra`. Les paramètres `qa`/`ra` des méthodes de la vtable (`left_press`, `right_press`, `hit_test`, `drag`…) restent tels quels.
- **Verdict** :

### D-30 — `typed_main_*` des tests contre `main_*_typed`

- **Origine** : 2b (échantillon, « leurs doubles de test disent `typed_main_camera`… »).
- **Question** : `src/` dit `main_camera_typed`, `main_economy_typed`, `main_mouse_typed` ; cinq tests définissent des aides statiques `typed_main_camera` (`test_background.c`, `test_camera.c`, `test_ui.c`), `typed_main_economy` (`test_economy.c`, `test_probe.c`), `typed_main_mouse` (`test_ui.c`), qui affirment le singleton non NULL puis le rendent.
- **Options** : (a) `checked_main_camera`, `checked_main_economy`, `checked_main_mouse` (ce qu'elles font de plus) ; (b) garder.
- **Sites** : 96 — `typed_main_camera` 24, `typed_main_economy` 68, `typed_main_mouse` 4. Noms neufs : 0 site.
- **Proposition prête à appliquer** : (a), procédure N, une ligne par identité (`c:<fichier>.c@F@typed_main_X`, 6 identités).
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire ; contrôle sur `typed_main_`.
- **Verdict** :

### D-31 — Digramme `oc` : `tickets` et `type_picker`

- **Origine** : 2b (échantillon, « un digramme, deux noms »).
- **Question** : `oc` est devenu `tickets` (champ de `weighted_picker_t`, `gen.h`) et `type_picker` (`g_type_picker`, `planet_set_type_picker`, `planet.c`) ; le paramètre `oc` de `planet_set_type_picker` est resté (`conservé`).
- **Options** : (a) garder les deux noms (deux objets : le tirage pondéré et son tableau de tickets), renommer le paramètre `oc` → `picker` ; (b) garder tout.
- **Sites** : 3 pour le paramètre (`planet.h`, `planet.c:146–147`) ; `oc` au total 17 (le reste en commentaire et dans le libellé `jo.oc` de `test_live_slots.c`, que 4f reprend).
- **Proposition prête à appliquer** : (a), ligne `field planet_set_type_picker.oc	picker` de `tools/rename_overrides.tsv` et procédure N (identité du paramètre à relever : offset).
- **Portée** : s'applique aussi aux mentions en commentaire.
- **Verdict** :

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
- **Verdict** :

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
- **Verdict** :

### D-34 — Assertion faible `strstr(log, "exempt")` de `test_class_coverage.c`

- **Origine** : 4a (commit de chaînes, lecteurs de `tools/class_coverage.sh`).
- **Question** : `test_exempting_every_zero_class_passes` affirme `strstr(log, "exempt") != NULL`, qui apparie aussi la ligne de verdict (`coverage: OK -- every non-exempt class is instantiated …`) : elle passe même si aucune ligne ne porte d'exemption. Faiblesse héritée telle quelle (l'ancien `"exemptée"` appariait aussi « non exemptée ») ; l'étape 4 ne change pas une condition d'assertion.
- **Options** : (a) resserrer sur la forme d'une ligne exemptée ; (b) garder.
- **Sites** : 1 (`tests/test_class_coverage.c:69`).
- **Proposition prête à appliquer** : (a), `strstr(log, "  exempt (test)") != NULL` (la raison `test` est celle qu'écrit l'`awk` du même cas), commit `test:` ; contrôle : `make test` vert.
- **Verdict** :

### D-35 — Noms d'identifiant qui citent Lua ou le cart (3b)

- **Origine** : 4b (commit de commentaires ; le relevé des citations ne lit que les commentaires).
- **Question** : quatre identifiants de 3b citent encore Lua ou le cart : le paramètre `lua` de la macro `P8_FIXED_LIT(raw, lua)` (`src/p8/fixed.h`, le littéral décimal, jamais évalué), et trois fonctions de test, `test_mod_negative_dividend_matches_lua_floor_semantics` (`tests/test_fixed.c`), `test_sget_reads_known_pixel_from_loaded_cart_data` et `test_map_and_mset_round_trip_against_real_cart_data` (`tests/test_gfx.c`). Les renommer ?
- **Options** : (a) renommer ; (b) garder (le §2 vise les commentaires et le code ; ici « cart » désigne les données de `pico8/`).
- **Sites** : 8 (paramètre : 1, la liste de paramètres, le corps ne l'emploie pas ; fonctions : 7, définitions, appels de `main` et une mention en commentaire de `test_gfx.c`).
- **Proposition prête à appliquer** : (a). Paramètre : `lua` → `decimal`, édition à la main de la ligne de `#define` (paramètre de macro, hors extraction), commit `refactor:` ; sortie du préprocesseur des appelants inchangée (le paramètre n'est pas utilisé). Fonctions, procédure N : `test_mod_negative_dividend_matches_lua_floor_semantics	test_mod_negative_dividend_floors	tests/test_fixed.c	c:test_fixed.c@F@test_mod_negative_dividend_matches_lua_floor_semantics`, `test_sget_reads_known_pixel_from_loaded_cart_data	test_sget_reads_known_pixel_from_loaded_sheet	tests/test_gfx.c	c:test_gfx.c@F@test_sget_reads_known_pixel_from_loaded_cart_data`, `test_map_and_mset_round_trip_against_real_cart_data	test_map_and_mset_round_trip_against_real_sheet	tests/test_gfx.c	c:test_gfx.c@F@test_map_and_mset_round_trip_against_real_cart_data` (noms neufs : 0 site).
- **Portée** : s'applique aussi aux mentions en commentaire ; contrôle : `grep -rnwiE '[a-z_]*(lua|cart)[a-z_]*' src/p8 tests/test_fixed.c tests/test_gfx.c` ne rend que `cart_draw`.
- **Complément (4g)** : quatre identifiants de test de 3g citent le cart : `cart_positions_items`, `CART_POSITIONS`, `cart_routed_positions_items`, `CART_ROUTED_POSITIONS` (`tests/test_research_project.c`, 8 sites, 2 chacun : définition et emploi), copies figées des listes de noms de positions que seul `DEFAULTS_HASH` plie (le commentaire réécrit par 4g dit « frozen copies », sans citer le cart). Sous (a) : `frozen_positions_items`, `FROZEN_POSITIONS`, `frozen_routed_positions_items`, `FROZEN_ROUTED_POSITIONS` (0 site), procédure N ; renommer n'en change pas le pliage (seules les chaînes des listes sont pliées, `DEFAULTS_HASH` inchangée).
- **Verdict** :

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
- **Verdict** :

### D-37 — `argc`/`argv` de `main` dans la liste des locales cryptiques

- **Origine** : 4c1 (mesure de `make cryptic-locals MODULES=<4c1>`).
- **Question** : la liste des locales cryptiques (portée > 40 lignes, nom de trois caractères ou moins ou non découpé en mots) relève `argc` et `argv`, l'idiome C des paramètres de `main`, dès que `main` dépasse 40 lignes. Les renommer, ou sortir l'idiome de la règle ?
- **Options** : (a) règle de `tools/scope_count.py --cryptic` : un paramètre `argc` ou `argv` d'une fonction `main` n'est pas cryptique (test d'abord dans `tests/test_scope_count.py`) ; (b) renommer (`arg_count`/`args`) par la procédure N ; (c) exceptions par module.
- **Sites** : 10 déclarations, 5 `main` : `tests/test_allocator` (43 lignes), `cmd/class_coverage` (58), `cmd/game` (95), `cmd/replay` (94), `cmd/screencast_viewer` (102) ; `make cryptic-locals` sans `SUBSTEP`, après `rename-extract` frais à la fin de 4c1.
- **Proposition prête à appliquer** : (a). Contrôle : `make cryptic-locals` sans `SUBSTEP` rend 10 candidats de moins, `make scope-count-test` vert.
- **Portée** : la règle ne touche que la liste des locales cryptiques ; aucune mention en commentaire ni entre backticks n'est en cause.
- **Verdict** :

### D-38 — Statiques de fichier `mem`/`gfx` des tests

- **Origine** : 4c1 (`tests/test_exit_queue.c` en déclare).
- **Question** : 40 tests déclarent `static p8_memory_t mem;` et `static p8_graphics_t gfx;` (80 déclarations), `conservé` au TSV. Le §4 veut pour une globale un nom complet, et `g_` pour une statique mutable de fichier ; 4b a renommé les locales homonymes de `test_demo.c` en `memory`/`graphics`, et `tests/test_particle_zone.c` porte déjà `g_mem`/`g_gfx`. Les renommer ?
- **Options** : (a) garder ; (b) `memory`/`graphics` ; (c) `g_memory`/`g_graphics` (§4 à la lettre).
- **Sites** : 80 déclarations dans 40 fichiers (`grep -rhE '^static p8_(memory|graphics)_t (mem|gfx);' tests | wc -l`), plus leurs emplois, à mesurer par le `grep` des conventions sur chaque fichier.
- **Proposition prête à appliquer** : (b), procédure N, en un passage à l'étape 5 ; avec D-36 (b), les mentions entre backticks ne comptent pas hors de leur fichier.
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire des 40 fichiers ; contrôle : le `grep` des sites ci-dessus rend 0.
- **Verdict** :

### D-39 — Nom de test qui cite le cart (`test_data_init.c`)

- **Origine** : 4d (commit de commentaires ; le relevé des citations ne lit que les commentaires et les messages).
- **Question** : `test_production_tables_project_records_icon_tooltips_technologies_decode_real_cart_content` (`tests/test_data_init.c`) cite encore le cart dans un identifiant, même question que D-35. Le renommer ?
- **Options** : (a) renommer ; (b) garder (« cart » y désigne les tables du jeu).
- **Sites** : 2 (définition, appel de `main`).
- **Proposition prête à appliquer** : (a), procédure N : `test_production_tables_project_records_icon_tooltips_technologies_decode_real_cart_content	test_production_tables_project_records_icon_tooltips_technologies_decode_real_game_data	tests/test_data_init.c	c:test_data_init.c@F@test_production_tables_project_records_icon_tooltips_technologies_decode_real_cart_content` (nom neuf : 0 site).
- **Portée** : s'applique aussi aux mentions en commentaire (aucune) ; contrôle : `grep -rnwiE '[a-z_]*(lua|cart)[a-z_]*' tests/test_data_init.c` rend 0.
- **Verdict** :

### D-40 — Clés du cart sans ligne `chaîne` au TSV

- **Origine** : 4d (commit de chaînes).
- **Question** : les clés de dispatch des fonctions de dessin `"br"`, `"qr"`, `"rf"`, `"s"` (clés de la table de dispatch du cart, `fn="…"` de ses rangées) et la clé de production `"qh+"` (composé de `"qh"`, renommé `"structure"` par 3a) n'avaient pas de ligne `chaîne` au TSV : 2b ne les avait pas extraites (clés de deux lettres hors du TSV ; composé non apparié par la règle « la chaîne est exactement le nom »). 4d les a traduites (`"draw_bar"`, `"print_outlined"`, `"draw_help_frame"`, `"spr"`, `"structure+"`) selon la règle de propriété des chaînes, mais la garde anti-cart ne verrait pas une telle clé revenir. Faut-il outiller ce contrôle ?
- **Options** : (a) rien, le contrôle reste humain (règle de tri du point 6 de `docs/consigne-etape-4.md`) ; (b) ajouter à l'étape 5 des lignes `chaîne` par `rename-strings` (outil, test d'abord), pour que la garde les compte ; (c) un relevé ponctuel à l'étape 5 des `fn="…"` de `pico8/lua.lua` contre les clés de `draw_functions_init`.
- **Sites** : 0 après 4d (`grep -rn '"br"\|"qr"\|"rf"\|"qh+"\|draw_function = "s"\|dispatch_lookup("s")\|register("s"' src tests`).
- **Proposition prête à appliquer** : (a).
- **Portée** : aucune mention en commentaire ni entre backticks n'est en cause.
- **Verdict** :

### D-41 — Nom de test qui garde le digramme `r` (`test_map_tables.c`)

- **Origine** : 4d (commit de commentaires, relecture de `tests/test_map_tables.c`).
- **Question** : `test_weighted_picker_pools_planet_types_entries_by_their_r` garde en suffixe `r`, le champ du cart devenu `weight` ; le TSV l'a appliqué tel quel (`renommé`, depuis `test_gn_pools_df_entries_by_their_r`). Aucun compteur ne le voit (le nom entier n'est pas un ancien nom). Le renommer ?
- **Options** : (a) renommer ; (b) garder.
- **Sites** : 2 (définition, appel de `main`).
- **Proposition prête à appliquer** : (a), procédure N : `test_weighted_picker_pools_planet_types_entries_by_their_r	test_weighted_picker_pools_planet_types_entries_by_their_weight	tests/test_map_tables.c	c:test_map_tables.c@F@test_weighted_picker_pools_planet_types_entries_by_their_r` (nom neuf : 0 site) ; la ligne de `docs/renommage.tsv` passe au nom neuf.
- **Portée** : s'applique aussi aux mentions en commentaire (aucune) ; contrôle : `grep -rn 'by_their_r\b' src tests tools` rend 0.
- **Verdict** :

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
- **Verdict** :

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
- **Verdict** :

### D-44 — Champ `planet_inspection_panel_ctx_t.unmet_needs`, qui tient les besoins reçus

- **Origine** : 4e (commit de commentaires, réécriture du commentaire de `planet_inspection_panel_filter_step`).
- **Question** : dans `src/game/planet_display.c`, le filtre garde les besoins que rien ne satisfait (la locale `unmet_needs` de `planet_inspection_panel`, rangée « needs: ») et ajoute chaque besoin satisfait, une fois par unité, à la liste du contexte, qui fait la rangée « receives: ». Ce champ du contexte s'appelle aussi `unmet_needs` (`ji` → `unmet_needs`, 3e, comme la locale `jx`) : il dit le contraire de ce qu'il tient. Aucun compteur ne le voit. Le renommer ?
- **Options** : (a) renommer le champ `received_needs` ; (b) garder.
- **Sites** : 7 lignes de `src/game/planet_display.c` (déclaration ligne 88, emplois 107, 196, 236, 237, 240, 241) ; l'exemption `planet_inspection_panel_ctx_t.unmet_needs` de `REGISTRES_NON_ENTITE` (`tools/composition_residue.sh:128`) et les deux lignes `chaîne` de `docs/renommage.tsv` qui la citent (écrivain `planet_display.c`, lecteur `tools/composition_residue.sh`).
- **Proposition prête à appliquer** : (a), procédure N : `unmet_needs	received_needs	src/game/planet_display.c	c:@SA@planet_inspection_panel_ctx_t@FI@unmet_needs` (identité stable, relevée dans `declarations.tsv` à la fin de 4e), ligne de `tools/rename_overrides.tsv` `field	pico8_jo_gh_ctx_t.ji	received_needs	gh` ; l'exemption et les lignes `chaîne` suivent par la procédure G (`--detail` avant et après, vivacité). La locale `unmet_needs` de `planet_inspection_panel` garde son nom.
- **Portée** : s'applique aussi aux mentions en commentaire (aucune après 4e) ; contrôle : `grep -n 'ctx->unmet_needs\|filter_context.unmet_needs\|ctx_t.unmet_needs' src tools` rend 0.
- **Verdict** :

### D-45 — Noms de test qui disent « grab arena » pour une prise (`test_live_slots.c`)

- **Origine** : 4f1 (commit de commentaires, relecture des cas de prise de `tests/test_live_slots.c`).
- **Question** : dans `tests/test_live_slots.c`, « prise » désignait le geste (une structure tirée de son icône et tenue, puis posée ou refusée), pas l'arène où elle naît. L'étape 3 a suivi le concept du vocabulaire `prise` → `grab arena` : `refused_grab_arenas_do_not_grow_game` (ancien `prises_refusees_ne_font_pas_grandir_la_partie`), `placed_grab_arena` (`la_prise_posee`, qui rend la planète posée), `placed_grab_arena_reuses_dead_wormhole_arena` (`une_prise_posee_reprend_l_arene_du_mz_mort`), `refused_grab_arena_crosses_new_game` (`une_prise_refusee_traverse_une_nouvelle_partie`), `offscreen_planet_forgets_refused_grab_arena` (`le_fo_d_une_planete_hors_ecran_oublie_la_prise_refusee`), la macro `GRAB_ARENAS` (`PRISES`) et la fixture `tests/fixtures/grab_arenas_refused.json` (4f2). Les messages et libellés réécrits par 4f1 disent déjà « grab » (`refused grabs`, `placed grab`, « a refused grab leaves its structure in the generation »). Renommer ?
- **Options** : (a) `grab_arena` → `grab` dans ces cinq fonctions et la macro : `refused_grabs_do_not_grow_game`, `placed_grab`, `placed_grab_reuses_dead_wormhole_arena`, `refused_grab_crosses_new_game`, `offscreen_planet_forgets_refused_grab`, `GRABS` ; fixture `tests/fixtures/grabs_refused.json` (`git mv`, ligne `fichier` et procédure de 3l, avec sa mention dans `tools/rename_substeps.tsv` et la liste de 4f2 de `docs/plan-5.md`) ; (b) garder.
- **Sites** (``grep`` des conventions, fin de 4f1) : 2 par fonction (la définition et un appel : `main`, ou `placed_grab_arena_reuses_dead_wormhole_arena` pour `placed_grab_arena`), 10 ; macro 6 (définition, quatre `start_e2e`, l'en-tête de commentaire de sa fixture) ; fixture 1 en code (la valeur de `GRAB_ARENAS`), plus sa ligne de `tools/rename_substeps.tsv` et la liste de 4f2 du plan. Noms neufs : 0 site.
- **Proposition prête à appliquer** : (a), procédure N, identités relevées à la fin de 4f1 dans `declarations.tsv` : `refused_grab_arenas_do_not_grow_game	refused_grabs_do_not_grow_game	tests/test_live_slots.c	c:test_live_slots.c@F@refused_grab_arenas_do_not_grow_game`, `placed_grab_arena	placed_grab	tests/test_live_slots.c	c:test_live_slots.c@F@placed_grab_arena`, `placed_grab_arena_reuses_dead_wormhole_arena	placed_grab_reuses_dead_wormhole_arena	tests/test_live_slots.c	c:test_live_slots.c@F@placed_grab_arena_reuses_dead_wormhole_arena`, `refused_grab_arena_crosses_new_game	refused_grab_crosses_new_game	tests/test_live_slots.c	c:test_live_slots.c@F@refused_grab_arena_crosses_new_game`, `offscreen_planet_forgets_refused_grab_arena	offscreen_planet_forgets_refused_grab	tests/test_live_slots.c	c:test_live_slots.c@F@offscreen_planet_forgets_refused_grab_arena`, `GRAB_ARENAS	GRABS	tests/test_live_slots.c	macro:tests/test_live_slots.c:GRAB_ARENAS` ; les lignes de `docs/renommage.tsv` gardent leur `ancien` du cart (D-42).
- **Portée** : s'applique aussi aux mentions en commentaire (l'en-tête `GRAB_ARENAS:` de la fixture) ; contrôle : `grep -rnw 'grab_arena\|grab_arenas\|GRAB_ARENAS\|grab_arenas_refused' tests/test_live_slots.c` ne rend que le champ `placed->grab_arena` de la sonde, qui tient vraiment une arène.
- **Verdict** :

### D-46 — Fonctions de `main_economy.h` définies dans `probe.c`

- **Origine** : 4f2 (commit de commentaires ; 3f le notait pour les `main_economy*`).
- **Question** : `main_economy`, `main_economy_typed`, `main_economy_placeholder`, `main_economy_placeholder_class` et `probe_set_main_economy` sont déclarées dans `src/game/main_economy.h`, qui n'a pas de `.c`, et définies dans `src/game/probe.c` (avec l'état `g_main_economy` et la struct provisoire, que `probe_init` remet en place). `probe_set_main_economy` porte en plus le préfixe `probe_` dans un en-tête `main_economy` (§4 : module = nom du fichier). Que faire ?
- **Options** : (a) garder (§7 : pas de redécoupage de module pendant le plan 5) ; (b) renommer `probe_set_main_economy` → `main_economy_set`, procédure N ; (c) déplacer les définitions et leur état dans un `src/game/main_economy.c` (redécoupage, hors plan 5).
- **Sites** (`grep` des conventions, fin de 4f2) : `probe_set_main_economy` 3 (déclaration `main_economy.h:62`, définition `probe.c:490`, appel `cart.c:217`) ; `main_economy` 53 (mot entier, les `#include "game/main_economy.h"` compris), `main_economy_typed` 91, `main_economy_placeholder` 10, `main_economy_placeholder_class` 5. `main_economy_set` : 0 site.
- **Proposition prête à appliquer** : (a) ; (b) en complément si l'humain veut le §4 à la lettre : `probe_set_main_economy	main_economy_set	src/game/main_economy.h	c:@F@probe_set_main_economy`, ligne de `tools/rename_overrides.tsv` `name	pico8_probe_set_fm	main_economy_set` ; la ligne du TSV garde son `ancien` du cart (`pico8_probe_set_fm`).
- **Portée** : s'applique aussi aux mentions entre backticks et en commentaire ; contrôle sous (b) : le `grep` des conventions sur `probe_set_main_economy` rend 0.
- **Complément (4g)** : les commentaires de `main_economy.h` réécrits par 4g disent que ces fonctions et leur état sont définis dans `probe.c` ; `economy.h` nomme `probe_set_main_economy()` dans le contrat d'`economy_init`. Sites remesurés en fin de 4g (`grep` des conventions) : `probe_set_main_economy` 4 (la mention de `economy.h` en plus), `main_economy` 53, `main_economy_typed` 93, `main_economy_placeholder` 12, `main_economy_placeholder_class` 5.
- **Verdict** :

### D-47 — `structure_icon_sprite_counter` compte des identifiants de planète

- **Origine** : 4g (commit de commentaires, réécriture de `structure_icon.h` et `structure_icon.c`).
- **Question** : `pico8_ie_id` → `structure_icon_sprite_counter` (sens deviné à 2b) et ses statiques `g_sprite_counter`, `g_has_sprite_counter` (`src/game/structure_icon.c`) : la valeur, bumpée par `structure_icon_left_press`, compose l'identifiant `"s<n>"` de la structure posée (`keys.planet_id`) ; aucun sprite n'en dépend. Le test dit déjà `shared_id`, `shared_id_fixed`. Le nom ment ; aucun compteur ne le voit. Le renommer ?
- **Options** : (a) `structure_icon_id_counter`, `g_id_counter`, `g_has_id_counter` ; (b) garder.
- **Sites** (`grep` des conventions, fin de 4g) : `structure_icon_sprite_counter` 4 (déclaration, définition, appel de `tests/test_structure_icon.c`, mention en commentaire de `structure_icon.h`), `g_sprite_counter` 5, `g_has_sprite_counter` 5 ; noms neufs : 0 site.
- **Proposition prête à appliquer** : (a), procédure N, identités relevées dans `declarations.tsv` à la fin de 4g : `structure_icon_sprite_counter	structure_icon_id_counter	src/game/structure_icon.c	c:@F@structure_icon_sprite_counter`, `g_sprite_counter	g_id_counter	src/game/structure_icon.c	c:structure_icon.c@g_sprite_counter`, `g_has_sprite_counter	g_has_id_counter	src/game/structure_icon.c	c:structure_icon.c@g_has_sprite_counter` ; lignes de `tools/rename_overrides.tsv` `name	pico8_ie_id	structure_icon_id_counter	ie`, `name	g_ie_id	g_id_counter	ie`, `name	g_ie_has_id	g_has_id_counter	ie` (remplacent les lignes 766, 768, 769) ; les lignes du TSV gardent leur `ancien` du cart. Le message de `structure_icon_left_press` (« bumps the shared sprite counter », réécrit par 4g) devient « bumps the shared id counter », à hauteur constante (`structure_icon.c` est dans `tools/line_coverage_reference.txt`).
- **Portée** : s'applique aussi aux mentions en commentaire et dans les messages ; contrôle : le `grep` des conventions sur les trois anciens noms rend 0, et `git grep -n 'sprite counter' src tests` rend 0.
- **Verdict** :

### D-48 — Noms de test qui décrivent l'API générique disparue ou la méthode du cart

- **Origine** : 4h (commit de commentaires, relecture de `tests/test_ui.c`, `tests/test_main_mouse.c`, `tests/test_main_camera.c`).
- **Question** : `test_ui_held_stops_off_row` (`tests/test_ui.c` : une classe que l'ui ne tient pas fait avorter le clic ; « stops off row » parle de la ligne du lecteur de champs générique, disparu à 4f2), `test_probe_t_key_stops_off_row` (`tests/test_ui.c` : la clé `state_time` d'une sonde à zéro ; `t` est son digramme), `test_recompute_on_placeholder` (`tests/test_main_mouse.c`, `tests/test_main_camera.c` : « recompute » est la méthode `lj` du cart, `ui_update`) et les macros `RECOMPUTE_MOUSE_WORLD_X`/`_Y` des deux mêmes fichiers (la valeur figée de `mouse_world` après `ui_update`). Aucun compteur ne les voit (le nom entier n'est pas un ancien nom). Les renommer ?
- **Options** : (a) `test_ui_held_aborts_on_other_classes`, `test_probe_state_time_key_at_zero`, `test_update_on_placeholder`, `UPDATE_MOUSE_WORLD_X`/`_Y` ; (b) garder.
- **Sites** (`grep` des conventions, fin de 4h) : `test_ui_held_stops_off_row` 2 et `test_probe_t_key_stops_off_row` 2 (définition, appel de `main`) ; `test_recompute_on_placeholder` 4 (deux fichiers) ; `RECOMPUTE_MOUSE_WORLD_X` 4 et `RECOMPUTE_MOUSE_WORLD_Y` 4 (définition et emploi dans chacun des deux fichiers). Noms neufs : 0 site.
- **Proposition prête à appliquer** : (a), procédure N, identités relevées dans `declarations.tsv` à la fin de 4h : `test_ui_held_stops_off_row	test_ui_held_aborts_on_other_classes	tests/test_ui.c	c:test_ui.c@F@test_ui_held_stops_off_row`, `test_probe_t_key_stops_off_row	test_probe_state_time_key_at_zero	tests/test_ui.c	c:test_ui.c@F@test_probe_t_key_stops_off_row`, `test_recompute_on_placeholder	test_update_on_placeholder	tests/test_main_mouse.c	c:test_main_mouse.c@F@test_recompute_on_placeholder`, `test_recompute_on_placeholder	test_update_on_placeholder	tests/test_main_camera.c	c:test_main_camera.c@F@test_recompute_on_placeholder`, `RECOMPUTE_MOUSE_WORLD_X	UPDATE_MOUSE_WORLD_X	tests/test_main_mouse.c	macro:tests/test_main_mouse.c:RECOMPUTE_MOUSE_WORLD_X` et ses trois pendants (`_Y`, et les deux de `tests/test_main_camera.c`) ; les lignes de `docs/renommage.tsv` passent aux noms neufs. Renommer ne change aucun hachage (aucun nom n'est plié).
- **Portée** : s'applique aussi aux mentions en commentaire (aucune après 4h) ; contrôle : le `grep` des conventions sur les cinq anciens noms rend 0.
- **Verdict** :

### D-49 — Assistants de test qui citent un digramme du cart ou un surnom

- **Origine** : 4i (commit de commentaires, relecture de `tests/test_burst_exports_row.c` et `tests/test_trading_hub.c`).
- **Question** : `planet_by_ref` (`tests/test_burst_exports_row.c`) rend `planet->outgoing_routes` ; `by` est le digramme du cart de ce champ, que le §1 interdit et que 3i a déjà retiré du nom du test `test_burst_export_line_counts_the_outgoing_routes`. `build_plate`, `fold_plate`, `is_plate` (`tests/test_trading_hub.c`) disent « plate », surnom de test du plateau commercial que rien d'autre dans le dépôt ne dit ; `is_plate` lit `archetype->is_trading_hub`. Aucun compteur ne les voit : le nom entier n'est pas un ancien nom, `by` et `plate` sont des mots anglais. Les renommer ?
- **Options** : (a) `outgoing_routes_of`, `build_trading_hub`, `fold_trading_hub`, `is_trading_hub_archetype` (pas `is_trading_hub`, homonyme du champ d'archétype : espace de noms à part, mais lecture trompeuse), procédure N ; (b) garder.
- **Sites** (`grep` des conventions, fin de 4i) : `planet_by_ref` 2 (définition, appel), `build_plate` 6, `fold_plate` 3, `is_plate` 3 ; noms neufs : 0 site.
- **Proposition prête à appliquer** : (a), procédure N, identités relevées dans `declarations.tsv` à la fin de 4i : `planet_by_ref	outgoing_routes_of	tests/test_burst_exports_row.c	c:test_burst_exports_row.c@F@planet_by_ref`, `build_plate	build_trading_hub	tests/test_trading_hub.c	c:test_trading_hub.c@F@build_plate`, `fold_plate	fold_trading_hub	tests/test_trading_hub.c	c:test_trading_hub.c@F@fold_plate`, `is_plate	is_trading_hub_archetype	tests/test_trading_hub.c	c:test_trading_hub.c@F@is_plate` ; les lignes de `docs/renommage.tsv` passent aux noms neufs. Renommer ne change aucun hachage (aucun nom n'est plié).
- **Portée** : s'applique aussi aux mentions en commentaire ; le mot « plate » des commentaires de `tests/test_trading_hub.c` est déjà réécrit « trading hub » par 4i (`grep -rnw 'plate\|plates' src tests` ne rend que les trois noms). Contrôle : le `grep` des conventions sur les quatre anciens noms rend 0.
- **Verdict** :
