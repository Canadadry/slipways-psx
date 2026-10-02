---
title: "Plan 5, sous-étape 3d — données et mise en page"
description: "Les tables de données du jeu, la génération de galaxie, la mise en page et le HUD (8 modules de game/ et 6 tests) passent aux noms anglais du TSV, avec les exemptions de spec traduites, la clé \"ld\" élargie sans troncature et les deux constantes de hachage prévues recalculées."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 3d. **Données et mise en page.** `game/` : `game_data`, `map_tables`, `data_init`, `gen`, `galaxy`, `draw_functions`, `layout`, `hud`. Tests : `test_map_tables`, `test_data_init`, `test_spec_defaults`, `test_layout_row`, `test_layout_nil_item_draws_nothing`, `test_tooltip_draw_at`. Résidu ≤ 362.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4), quatrième étape. Entrées : `docs/plan-5.md` §4, §5, entrée 3 avec « Sous-étapes » et « Méthode des sous-étapes », et les notes « Découvert » de **3a, 3b et 3c**. Elles fixent la méthode, et cette PRD y renvoie sans la recopier : ordre des commandes (note de 3a), règle d'appartenance des chaînes, angles morts des littéraux composés et des littéraux adjacents recollés, prévision des hachages, recâblage après chaque édition de texte, porte `rename-rekey-check` et non `rename-table-check`, défauts de l'outil corrigés avec leur test, corrections du TSV permises pour la seule conformité au §4 (TSV + `tools/rename_overrides.tsv`). Exemple à suivre : `c62d4af` (3c, une seule tranche). Comme aucun humain n'est là, l'agent rédacteur a chargé lui-même la consigne `guideline` aux points « vérifier avec l'utilisateur » de `to-prd`. Les modules, les tests et le découpage sont tranchés sur le texte du plan, sans interview.

**Condition d'entrée** : `origin/plan-5-noms-anglais` = HEAD de départ (`558d06f` ou un descendant), arbre propre, `make test-all` vert. Puis `make rename-extract` frais sans ligne `warning:` (les 4 lignes `partial` de `class_coverage` sont attendues, note de 2a), et `make rename-rekey-check SUBSTEP=3d` vert (mesuré : 0 identité introuvable).

## Problem Statement

Après 3c, les données statiques du jeu et leur mise en page gardent les noms du cart : `pico8_jo_def`, `pico8_df_table`, `pico8_kind_rec_t`, `pico8_lq_t`, `pico8_jb_t`, `pico8_gn_state_t`, `pico8_hc_lookup`… La sous-étape compte **884 identités** et **6 459 occurrences AST dans 110 fichiers**. C'est la plus grosse sous-étape de `game/` en occurrences. `game/game_data` en porte à lui seul 436 identités et 1 758 occurrences, dont 671 **désignateurs** (`.k_ = …` dans les initialiseurs des archétypes `static const`).

Le compilateur arbitre l'application. Les points délicats sont ailleurs, là où il ne voit rien :

- **La clé `"ld"` → `"draw_row"` ne tient pas dans la table de `draw_functions`.** Le nom enregistré est copié dans un `char name[8]`, et un `assert` refuse tout nom de 8 caractères ou plus. Mesure faite dans un worktree jetable, chaînes seules renommées : 30 tests tombent sur `"pi dispatch name would be truncated"`. C'est la première clé allongée qui heurte un tampon de taille fixe.
- **Les exemptions de `spec`** (`SPEC_EXCLUSION` de `tools/data_residue.sh`) nomment les cinq `_default` de 3d : `pico8_dv`, `pico8_io`, `pico8_lq_ol`, `pico8_bl`, `pico8_rk`. Le témoin de vivacité, dans `tests/test_data_residue.c`, est écrit en littéraux recollés (`"pico8_" "io_default"`, `"pico8_" "rk_default"`), angle mort des recherches par littéral. Après 3d, plus aucun nom du dépôt n'a la forme `pico8_X_default` : l'exemption entière meurt, comme 3b l'avait annoncé.
- **Deux tests à hachage figé** changent avec la clé `"qe"` → `"resource"`.
- **24 sites de chaînes**, dont 2 `unseen` (`r_ctx`), et des fixtures de `tests/test_composition_residue.c` qui citent des noms de 3d dans des littéraux composés, sorties attendues comprises.

## Solution

Une seule tranche, dans l'ordre de commandes de la note de 3a, sur `SUBSTEP=3d` : application des 884 identités, élargissement du tampon de noms de `draw_functions`, traduction de `SPEC_EXCLUSION` et de son témoin, recâblage, puis les 24 sites de chaînes avec les deux constantes de hachage recalculées, nouveau recâblage, relevés et note « Découvert ».

À la fin :
- aucun nom de 3d ne reste en `identifier` ni en `string` dans le dépôt ;
- les fichiers de 3d ne gardent que les 362 mentions de sous-étapes suivantes ;
- `data-residue` et `composition-residue` restent à 0, avec un `--detail` identique ;
- `scope-count SUBSTEP=3d` passe de 209 à 4 ;
- pas un pixel ne change.

## User Stories

1. En tant qu'agent de 3d, je veux partir d'un TSV dont chaque identité de 3d se résout (`rename-rekey-check SUBSTEP=3d` vert sur extraction fraîche), afin que `rename-apply` ne refuse aucune entrée faute d'identité.
2. En tant qu'agent de 3d, je veux appliquer les 884 identités de `rename-slice SUBSTEP=3d` en une commande, afin que le compilateur arbitre tout oubli dans les 110 fichiers touchés.
3. En tant que lecteur des tables de données, je veux que les archétypes s'appellent `planet_archetype`, `probe_archetype`…, leurs types `X_archetype_t`, les tables `planet_types_table`, `upkeep_by_tier_table`…, et que leurs champs désignés soient complets (`trail_colors`, `fade_in_step`, `type_bias`, `hit_box`…), afin que chaque initialiseur se lise sans glossaire.
4. En tant que lecteur de la mise en page, je veux `layout_row_t`, `layout_row_cell_t`, `tooltip_t`, `data_row_t`, `data_item_t` et les constantes `LAYOUT_ROW_*`, afin que le §4 se lise partout où le jeu compose une rangée ou une infobulle.
5. En tant que lecteur de la génération, je veux `weighted_picker_t`/`weighted_picker_*` et `random_walk_t`/`random_walk_*` dans `gen`, et des locales complètes dans `galaxy` (`planet_count`, `planet_id`, `difficulty`), afin que la création de galaxie se lise.
6. En tant que mainteneur, je veux que la table de `draw_functions` accepte `"draw_row"` sans troncature, et que l'`assert` de troncature reste en place, afin que la clé renommée s'enregistre et qu'une clé trop longue soit toujours refusée.
7. En tant que mainteneur, je veux que `SPEC_EXCLUSION` nomme les cinq `_default` de 3d par leurs noms neufs, avec le témoin de `test_data_residue.c` réécrit en noms neufs recollés, afin que le dépôt ne cite plus le cart.
8. En tant que relecteur humain, je veux une note qui établit que l'exemption de `spec` est morte (preuve chiffrée sur le vrai arbre) et qui me laisse le choix (traduire le motif générique sans préfixe ou retirer l'exemption), afin de trancher au point d'arrêt d'avant l'étape 5.
9. En tant que mainteneur, je veux `composition_residue.sh --detail` et `data_residue.sh --detail` identiques avant et après, afin de prouver qu'aucune garde ne change de verdict.
10. En tant qu'agent de 3d, je veux renommer à tous leurs sites les noms de chaîne de 3d (24 sites), afin que `rename-strings-verify SUBSTEP=3d` soit vert, avec les 2 sites `unseen` vérifiés à la main et classés.
11. En tant qu'agent de 3d, je veux connaître d'avance les constantes de hachage qui changent (`CONSTRUCTIONS_HASH`, `INSTALL_HASH`), afin qu'un rouge imprévu arrête la tranche au lieu d'être recopié.
12. En tant qu'agent de 3d, je veux chercher mes noms aussi dans les littéraux composés et adjacents de tout le dépôt, sorties attendues comprises, afin de ne pas refaire les angles morts de 3a et 3b.
13. En tant que mainteneur, je veux que la garde de portée ne refuse plus dans 3d que les 4 statiques locales `conservé` de `test_layout_row` déjà proposées en exception par 3a, afin que les 205 autres refus disparaissent par le seul TSV.
14. En tant qu'agent de 3e, je veux trouver un TSV recâblé après l'application et après les chaînes, `rename-rekey-check` vert de 3d à 3l, afin de partir sur une table saine.
15. En tant que relecteur humain, je veux une note « Découvert » sous 3d avec les relevés de fin, les écarts, les fonctions exportées sans préfixe de module et les questions de nommage, afin de les trancher au point d'arrêt.
16. En tant que joueur, je ne veux voir aucun pixel changer.

## Implementation Decisions

### Périmètre et appartenance

- Modules : la ligne 3d de `tools/rename_substeps.tsv`. `rename-slice SUBSTEP=3d` rend **884 identités** :
  - par module : `game/game_data` 436, `game/map_tables` 220, `game/layout` 80, `tests/test_layout_row` 36, `game/draw_functions` 25, `game/gen` 21, `game/hud` 19, `tests/test_spec_defaults` 15, `game/galaxy` 12, `tests/test_map_tables` 9, `tests/test_tooltip_draw_at` 6, `game/data_init` 3, `tests/test_data_init` 1, `tests/test_layout_nil_item_draws_nothing` 1 ;
  - par sorte : 310 statiques, 226 champs, 126 fonctions, 89 globales, 49 types, 30 locales, 29 constantes d'enum, 14 macros, 8 paramètres, 3 tags.

  Le TSV donne les noms neufs : on y renvoie.
- Occurrences par origine : `ref` 2 368, `type_ref` 1 150, `designator` 1 108, `decl` 1 014, `macro_arg` 788, `macro_def` 14, `macro_use` 9, `macro_cond` 8. Aucun `##` dans les modules de 3d. Les 14 macros sont les gardes d'inclusion, `PICO8_PI_DISPATCH_CAPACITY`, `PICO8_JB_LINE_CAP` et quatre `*_HASH` de tests, renommés par leur nom seul (valeurs inchangées).
- Laissés en place :
  - les 362 mentions `identifier` de sous-étapes suivantes dans les fichiers de 3d, surtout `test_spec_defaults` 214 et `test_layout_row` 119 ;
  - les fixtures à motif **générique** de `test_composition_residue.c` (`"pico8_" "gn("` de `void-ptr`, garde en `pico8_X(`), qui ne citent pas le code et que 3a range hors TSV.
- Hors 3d (étape 4) : 53 `backtick` et 42 `comment` des fichiers de 3d, et les messages d'`assert` qui citent l'ancien nom (`"ix has PICO8_IX_COUNT rows…"` de `menu.c`, `"…after pico8_pi_init()"` de `help.c`).

### Application et recâblage

- Ordre des commandes : celui de la note de 3a, sur `SUBSTEP=3d`.
- Prévision du recâblage après l'application : **≈ 670** identités à offset de 3e–3l situées après un token réécrit. C'est une estimation grossière par la méthode de 3b (3c : prévu 866, constaté 847). Un second recâblage suit les chaînes, toutes allongées.
- Désignateurs : 1 108 occurrences `designator`, dont 671 dans `game_data`. L'outil les traite depuis 2a, mais c'est leur premier gros volume : un refus est un défaut de l'outil, corrigé avec son test dans le même commit, comme en 3a–3c.
- Champs d'archétype homonymes des champs d'en-tête (`me` → `state`, `fa` → `layer`, `hd` → `hit_test`, `kq` → `hit_box`, `probe` → `targeting`, `fx` → `fold_anchor`) : ce sont des identités distinctes, et elles reçoivent les mêmes noms que 3a. Le contrôle de collision les a déjà admises (champs de structs différentes).

### Fonctions exportées sans préfixe de module (relevé de 3c : 14 en 3d)

`planet_types_lookup`, `int_seq_at`, `int_seq_has_fixed` (`game_data`) ; `weighted_picker_new`, `weighted_picker_next`, `random_walk_new`, `random_walk_next` (`gen`) ; `tooltip_new`, `tooltip_draw_at`, `tooltip_ready`, `tooltip_class` (`layout`) ; `production_tables_lookup`, `resource_names_lookup`, `resource_code_lookup` (`map_tables`). Chacune porte le nom d'un type ou d'une table déclarés dans son module : `int_seq_t`, `weighted_picker_t`, `random_walk_t`, `tooltip_t`, `X_entry_t`/`X_table`. Elles suivent donc la règle « méthode de classe, sur le type receveur » du §4. Aucune n'est nue comme l'était `shake` : **aucune correction du TSV prévue**. Une correction ne se fait que si une garde révèle une vraie non-conformité au §4, comme `usage-mutuel` l'a fait en 3c. Elle passe alors par le TSV et les surcharges, appliquée par-dessus. La liste des 14 va dans la note, pour décision humaine. Le nom `data_init_data_shutdown` (`pico8_data_shutdown`) bégaie mais respecte le §4 : on l'applique tel quel et on le signale, sans le corriger par goût.

### Tampon de noms de `draw_functions`

- `"draw_row"` fait 8 caractères ; le tampon `char name[8]` de l'entrée de dispatch en tient 7. Décision : **élargir le tampon** (16), garder l'`assert` de troncature tel quel, ne rien changer d'autre. Pas de pointeur vers le littéral à la place : ce serait un autre contrat de durée de vie, pour une sous-étape de renommage.
- C'est l'`assert` qui a trouvé le problème : on écrit le test rouge d'abord. Avec la clé renommée et le tampon d'origine, un test de `make test` (par exemple `test_layout_row`) échoue sur l'`assert` ; l'élargissement le rend vert.
- Mesuré dans un worktree jetable, clés `"ld"` et `"qe"` renommées et tampon élargi : `make test` est vert aux deux hachages près, `make e2e` vert (26 e2e conformes). Les autres clés enregistrées (`"br"`, `"qr"`, `"rf"`, `"s"`) ne sont pas de 3d.

### Gardes à motifs (inventaire de 3a, éléments de 3d)

- **`SPEC_EXCLUSION`** (`data_residue.sh`) : `pico8_dv` → `hud_select_palette_block`, `pico8_io` → `hud_print_outlined`, `pico8_rk` → `hud_draw_bar`, `pico8_bl` → `draw_functions_draw_rows`, `pico8_lq_ol` → `layout_row_draw_at`, tous devant `_default`. `p8_spr` et `p8_fixed_rnd` (3b) restent. Rien n'est retiré.
- **Vivacité.** Sur l'arbre de départ, `SPEC_EXCLUSION` vidé fait monter `spec` à **33 lignes dans 15 fichiers** : l'exemption mord. Après l'application, la même mesure doit donner **0**, puisque le motif `\bpico8_[a-z0-9_]+_(default|defaults_reset)\b` ne voit plus aucun nom. L'exemption est alors morte en entier, et la méthode ne permet pas d'écrire un test de vivacité : une violation plantée en noms neufs n'est pas vue par le motif. C'est la catégorie intraduisible de la « Méthode des sous-étapes » : on la consigne dans la note pour décision humaine, avec la mesure suivante. Sans préfixe, `\b[a-z0-9_]+_(default|defaults_reset)\b` n'apparierait dans `src/` et `tests/` que les 7 noms exemptés : traduire le motif générique ne changerait pas `--detail`. Cette PRD ne tranche pas.
- **Témoin de `test_data_residue.c`** (module de 3l, réécrit ici comme en 3b) : `"pico8_" "io_default"` ×2 et `"pico8_" "rk_default"` deviennent les noms neufs, toujours recollés (`"hud_" "print_outlined_default"`, `"hud_" "draw_bar_default"`) pour que le fichier n'entre pas dans le résidu qu'il mesure. Les sorties attendues du test ne changent pas. `"pico8_" "jo_default"` et `"jo_" "default_fixed"`, violations génériques de 3e, restent.
- **`noyau-inclut`** : le motif nomme des chemins d'en-tête (`game/(game_data|map_tables|data_init)\.h`), déjà définitifs depuis l'étape 1. Rien à traduire. Ses fixtures (`"game/" "map_tables.h"`…) gardent leur vivacité, inchangées.
- **Ligne `info` de `composition_residue.sh --detail`** : elle imprime le nombre de lignes de `game/game_data.h`. Le renommage token à token ne déplace aucune ligne, donc elle ne doit pas bouger. Si elle bouge, c'est une anomalie à expliquer.
- `composition_residue.sh`, `composition_residue_sites.txt`, `scope_exceptions.txt`, `alloc_guard`, `line_coverage_reference.txt` : aucun nom de 3d hors commentaires. L'élargissement du tampon ne déplace aucune ligne.

### Chaînes

- Les 24 sites de `rename-strings-verify SUBSTEP=3d` :
  - clé `"ld"` → `"draw_row"` : écrivain `draw_functions.c`, lecteurs `message_box.c`, `score_panel_display.c`, `test_frame_zone.c`, `test_layout_row.c`, `test_score_panel.c` ;
  - clé `"qe"` → `"resource"` : écrivain `test_layout_row.c`, lecteurs `test_burst.c`, `test_research_project.c` ;
  - noms de test `test_pico8_{data_init,lq,map_tables,spec_defaults,jb_ol}` ;
  - symboles cités `pico8_pi_number`, `pico8_str_seq_t`, `bg_is_bool`, `pico8_lq_t`, dans leurs en-têtes et dans `tests/test_composition_residue.c`.
- **`tests/test_composition_residue.c`** : ces symboles cités sont dans des fixtures de faux positifs et de catégories (`nu`, `accesseurs-q6`, `void-self`, `un-ou-faux`, `registres`), parfois en littéral composé (`"extern const pico8_str_seq_t pico8_nl_n" "u_tags"`, `"pico8_lq_t *pico8_" "jo_ex(…"`). Le texte planté reparaît dans les **sorties attendues** de `--check` : on renomme les deux ensemble, et on vérifie que chaque cas teste toujours la même chose (un nom sans `pico8_` reste un faux positif ou une violation au même titre).
- **2 `unseen`** (`r_ctx`, `gen.h` et `test_composition_residue.c`) : `r_ctx` est à la fois un champ renommé (`random_ctx`) et un paramètre `conservé`. Les deux sites citent le **paramètre** : la fixture `"void pico8_" "gn(int a, void *r_ctx);"` (motif générique `void-ptr`) et le commentaire de `gen.h`, qui relève de l'étape 4. Rien à renommer ; le classement va dans la note.
- Littéraux composés et adjacents : refaire la recherche au moment de l'édition, sur tout le dépôt et pour les noms distinctifs de 3d (`grep '"[^"]*\b<nom>\b'` et `"pico8_" "<suffixe>"`). Mesure de cette PRD : seuls les sites ci-dessus, le témoin de `test_data_residue.c` et les fixtures génériques laissées. Les outils du dictionnaire (`tests/test_rename_*.py`) sont exclus exprès.
- **Hachages, prévision** (worktree jetable, `check` rendu non bloquant pour voir toutes les constantes) : **2 tests, 2 constantes**, `CONSTRUCTIONS_HASH` de `test_layout_row` (`"qe"` replié par `fold_tuple_name`) et `INSTALL_HASH` de `test_research_project` (`fold_view_name`). Ne bougent pas :
  - `test_burst` : son `fnv_str(h, "qe")` est dans une branche non prise ;
  - `test_score_panel`, `test_help_screen` : aucune rangée `"ld"` n'est hachée ;
  - les étiquettes de `fold_built_from` : imprimées, non hachées.

  On laisse les deux tests rougir, puis on reporte la valeur affichée. Tout autre rouge arrête la tranche (`help-me-debug`).

### Garde de portée

`scope-count SUBSTEP=3d` vaut **209** au départ : 187 champs (`game_data` 143, `map_tables` 24, `layout` 11, `gen` 5, `hud` 3, `draw_functions` 1), 16 locales et paramètres à portée > 40 (`galaxy` 3, `layout` 4, `test_layout_row` 9), 6 statiques de `test_layout_row`. Le TSV en renomme **205** en nom long. Restent **4** : `as`, `f` ×2 et `ks` (`test_constructions`, `fold_drawn`, `test_jo_ex`), statiques locales `conservé` à portée ≤ 32 lignes, déjà proposées en exception par 3a. Attendu en fin : 4. Aucune nouvelle exception n'est proposée.

### Fin de PRD

On reprend la liste du §5 et des notes de 3a–3c, sur `SUBSTEP=3d` :
- `make test-all` vert, `tests/e2e/*.sha256` inchangés ;
- `make raylib` et `make screencast-viewer` compilent et linkent (jamais lancés) ;
- `rename-tool-test` et les `rename-*-test` verts ;
- `class-coverage-net` et `e2e-gap` rouges à l'identique de la fin de 3c ;
- `data-residue` et `composition-residue` à 0, `--detail` identiques ;
- après `rename-extract` frais sans `warning:` :
  - `cart-names-count SUBSTEP=3d` : `done` 0 en `identifier` et en `string`, `later identifier` ≤ 362 ;
  - `rename-strings-verify SUBSTEP=3d` vert, 2 `unseen` classés ;
  - `rename-rekey-check` vert de 3d à 3l ;
  - `scope-count SUBSTEP=3d` à 4.

## Implementation Chunks

1. **Sous-étape 3d entière, note comprise.** Une seule tranche. L'application et les chaînes sont liées par la clé `"ld"`, le tampon et les fixtures de `test_composition_residue.c` ; séparées, `make test` rougit entre deux commits.
   - Un commit de code qui laisse `make test-all` vert : application, tampon élargi (test rouge d'abord), `SPEC_EXCLUSION` et son témoin, chaînes, deux constantes recalculées, TSV recâblé deux fois, défauts d'outil éventuels avec leurs tests.
   - Puis la note « Découvert » sous l'entrée 3d, dans le même commit ou dans un commit de docs qui suit : identités et occurrences appliquées (désignateurs à part), recâblages (≈ 670 prévu / constaté), tampon, exemption de `spec` morte et sa mesure (33 → 0), `--detail` comparés, `unseen` classés, littéraux composés trouvés, hachages (prévus / constatés), les 14 fonctions sans préfixe de module, `data_init_data_shutdown`, relevés de fin.
   - Le plan est corrigé dans le même commit si un constat le contredit.

## Testing Decisions

- Code de 3d : un seul test neuf ou rendu rouge, celui du tampon. Le tampon d'origine avec la clé neuve fait échouer l'`assert` de troncature dans `make test` ; c'est le rouge, l'élargissement donne le vert. L'`assert` reste la garde, aucun cas dédié n'est ajouté. Pour le reste, le filet est le compilateur, `make test` et les e2e à l'octet.
- Hachages : `test_layout_row` et `test_research_project` rougissent seuls, sur les deux constantes prévues ; on reporte la valeur affichée.
- `test_data_residue.c` : son cas `test_omitted_argument_defaults_are_not_class_specs` passe, réécrit en noms neufs ; il ne prouve plus la vivacité de l'exemption. On le constate et on le note, on ne le maquille pas.
- `test_composition_residue.c` : chaque cas touché garde son verdict (même catégorie, même compte) avec les noms neufs.
- Outils de méthode : inchangés, sauf un défaut trouvé, corrigé avec son test dans le même commit.

## Out of Scope

- Les noms de 3e–3l rencontrés dans les fichiers de 3d (résidu), et les éléments de garde de 3e–3j (`pico8_jo_ex`, `PTRS_NON_ENTITE`, lignes 4–11 de `composition_residue_sites.txt`).
- Commentaires, messages d'`assert`, mentions entre backticks ou nues : étape 4.
- Trancher les motifs génériques (`pico8_X_default`, `pico8_X(`…), retirer une exemption morte, accepter les exceptions de portée : point d'arrêt d'avant l'étape 5.
- Renommer par goût un nom conforme au §4 (`data_init_data_shutdown`, `resource_codes` à côté de `resource_code_lookup`).
- Changer la représentation de la table de dispatch au-delà de la taille du tampon.
- Lancer l'appli raylib.

## Further Notes

- Mesures de cette PRD (arbre `558d06f`, `rename-extract` frais) :
  - `cart-names-count SUBSTEP=3d` : `done` 4 252 `identifier`, 53 `backtick`, 10 `string`, 42 `comment` ; `later` 362 `identifier` (borne 362), 110 `backtick`, 0 `string`, 9 `comment` ;
  - `rename-strings-verify SUBSTEP=3d` : 24 sites, 22 en défaut (état de départ), 2 `unseen` ;
  - `scope-count SUBSTEP=3d` : 209.

  `tmp/ast_rename/slice.tsv` a été rendu à son état d'avant la mesure.
- Risque `usage-mutuel` (précédent de 3c) : un nom neuf déclaré dans un en-tête de 3d peut rencontrer, par le texte, un nom déclaré ailleurs et créer une paire de modules. Collisions repérées dans le TSV, qui touchent des sous-étapes suivantes et non 3d : la fonction `layout_row_copy` (3d) contre le champ `lq_copy` → `layout_row_copy` de `ui_button` (3h), et `to_int`, statique dans `hud` comme dans `slipway_display`. À surveiller en 3h ; à signaler dans la note si `composition-guard` rougit dès 3d.
- `rename-table-check` reste rouge par construction : la porte est `rename-rekey-check`.
- Durées : chaque `rename-extract` ou `rename-apply` prend environ 1 min 30, et la tranche en enchaîne quatre ou cinq.
