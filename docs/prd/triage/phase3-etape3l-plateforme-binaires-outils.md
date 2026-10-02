---
title: "Plan 5, sous-étape 3l — plateforme, binaires, outils"
description: "Dernière sous-étape de l'étape 3 : platform/, cmd/ et les tests des gardes de tools/ passent aux noms du TSV, les binaires et sorties de tmp/ prennent leur nom anglais, et le compteur anti-cart tombe à zéro partout en identifier et string, hors le repli de frames_ref.sh (voie (a))."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 3l. **Plateforme, binaires, outils.** `platform/` : `raylib_backend`, `raylib_backend_coords`, `fatal_pc` ; `cmd/` : `game`, `replay`, `frames`, `gif`, `minimize`, `screencast_viewer`, `class_coverage`. Binaires et sorties de `tmp/` (lignes `fichier`) dans le `Makefile`, les tests et `tools/` (`frames_ref.sh` par la voie (a), `float_grep_exclusions.txt`) ; `tools/composition_residue.sh` et `composition_residue_sites.txt`, `tools/minimize.sh`, `tools/pico8_reference_probe.p8` y ferment leurs dernières mentions. Tests : `test_raylib_backend_coords`, `test_frames_tool`, `test_replay_from_title`, `test_replay_usage`, `test_line_coverage`, et ceux des gardes de `tools/` : `test_alloc_guard`, `test_composition_residue`, `test_constants_check`, `test_data_residue`, `test_e2e_cleanup`, `test_e2e_limits`, `test_float_grep`, `test_float_guard`, `test_float_link`, `test_frames_check`. Fin : compteur à zéro partout, `identifier` et `string`.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4), douzième et dernière sous-étape de l'étape 3.

Entrées : `docs/plan-5.md` §1, §4, §5, l'entrée 3 avec « Sous-étapes » et « Méthode des sous-étapes », les notes « Découvert » de 3a à 3k, et la note de 2b sur les binaires. Cette PRD y renvoie sans les recopier : ordre des commandes (3a), appartenance des chaînes (site `écrivain`), littéraux composés et recollés (3a, 3f, 3j), répétition à blanc en worktree jetable, recâblage après chaque édition de texte avec `rename-rekey-check` pour porte, correction du TSV pour le seul §1/§4 (3c, 3h, 3j, 3k). Exemple à suivre : `89e1b5f` (3k).

Aucun humain n'était disponible : l'agent rédacteur a chargé lui-même `guideline` aux points « vérifier avec l'utilisateur » de `to-prd`. Les décisions reposent sur le plan et sur une répétition à blanc complète, sans interview.

**Condition d'entrée** : `origin/plan-5-noms-anglais` à `e24c850` ou un descendant, arbre propre, `make test-all` vert ; `make rename-extract` frais sans `warning:` (les 4 lignes `partial` de `class_coverage` attendues) ; `make rename-rekey-check SUBSTEP=3l` vert (mesuré : 0 introuvable).

## Problem Statement

Après 3k, il ne reste de noms du cart en `identifier` et `string` que ceux de 3l :
- `pico8_raylib_backend_*`, `pico8_raylib_backend_t`, `PICO8_RAYLIB_BACKEND_SCALE` et les gardes d'inclusion de `platform/` ;
- `pico8_fatal`, déclaré dans `p8/fatal.h` et défini dans `platform/fatal_pc.c` (note de 2b) ;
- les statiques françaises de `cmd/replay.c` (`SORTE_NAMES`, `PARTICULES_NAMES`, `write_remplissage`, `remplissage_path`) et 13 noms de tests français ou du cart dans les tests des gardes ;
- les **binaires** `tmp/pico8_replay`, `pico8_replay_gcov`, `pico8_replay_sanitize`, `pico8_gif`, `pico8_minimize`, `pico8_class_coverage`, `pico8_frames`, `main_raylib`, et les **sorties** `tmp/pico8_replay_dump.txt`, `tmp/pico8_demo_dump.txt` ;
- les clés `le`, `qy`, `qy_fv`, `ts`, écrites par `cmd/replay.c` et relues par `test_live_slots.c`.

Mesuré (extraction fraîche, HEAD `e24c850`) : `cart-names-count SUBSTEP=3l` `done` **110 `identifier`, 83 `string`** (44 `comment`, étape 4), `later` 0. Sans `SUBSTEP` : 113 `identifier` et 94 `string` dans tout le dépôt, tous de 3l. Les 3 `identifier` et 11 `string` hors des fichiers de 3l sont des sites de 3l : `pico8_fatal` dans `engine/exit_queue.c`, `engine/zone.c` et `p8/fatal.h`, et les chemins de binaires dans les tests de 3a–3k.

## Solution

Deux tranches, dans l'ordre de commandes de la note de 3a, sur `SUBSTEP=3l` :
1. les identités ;
2. les chaînes, binaires compris.

La répétition l'a établi : `make test-all` est vert (26 e2e conformes, `tests/e2e/` intact). À la fin, le compteur vaut **0 `identifier` et 1 `string` dans tout le dépôt**. Cette `string` est le repli `tmp/pico8_replay` de `frames_ref.sh`, voulu par la voie (a) (2b) : c'est une exception proposée (§4). Pas un pixel ne change.

## User Stories

1. En tant qu'agent de 3l, je veux appliquer les 33 identités en une commande, afin que le compilateur arbitre tout oubli.
2. En tant que lecteur, je veux lire `raylib_backend_*`, `p8_fatal`, `SLOT_KIND_NAMES` et `PARTICLE_NAMES`, afin de comprendre la plateforme et les points d'entrée sans glossaire.
3. En tant que développeur, je veux taper `make replay` et lancer `./tmp/replay`, `./tmp/minimize` ou `./tmp/frames_tool`, afin que chaque binaire porte le nom de son fichier `cmd/`.
4. En tant que mainteneur, je veux que `tmp/frames` reste le dossier des captures et que l'outil de frames s'appelle `tmp/frames_tool`, afin d'éviter la collision.
5. En tant qu'agent qui lance `make e2e-ref`, je veux que `frames_ref.sh` reconstruise encore un commit ancien dont le `Makefile` ne connaît que `tmp/pico8_replay`, afin de garder la référence historique (voie (a)).
6. En tant qu'agent `check-e2e` ou `fix-bug`, je veux que la commande de reproduction de mon skill nomme `./tmp/replay`, afin de ne jamais lancer un binaire périmé.
7. En tant que mainteneur des gardes, je veux que `--detail` et `--champs` des deux gardes à motifs restent identiques à l'octet, afin de prouver qu'aucune garde ne change de verdict.
8. En tant que relecteur, je veux une seule commande qui prouve la fin de l'étape 3 (compteur sans `SUBSTEP`), et la règle à suivre pour tout reste trouvé, afin que l'étape 4 parte d'un code sans nom du cart.
9. En tant que relecteur humain, je veux la liste de ce qu'aucune garde ne voit et des exceptions proposées, afin de les trancher au point d'arrêt d'avant l'étape 5.
10. En tant que joueur, je ne veux voir aucun pixel changer.

## Implementation Decisions

### Identités (tranche 1)

`rename-slice SUBSTEP=3l` rend **33 identités** :
- 24 fonctions : 8 `raylib_backend_*`, `raylib_backend_window_to_screen`, `p8_fatal`, `write_fill`, et 13 fonctions de test ;
- 5 macros : `PLATFORM_RAYLIB_BACKEND_H`, `PLATFORM_RAYLIB_BACKEND_COORDS_H`, `RAYLIB_BACKEND_SCALE`, `KE_JO` → `ENTITY_PLANET`, `R2` → `RESIDUE_2_SITES` ;
- 1 type, 2 statiques, 1 locale.

Mesuré en répétition : **113 occurrences dans 15 fichiers** (`ref` 40, `decl` 38, `macro_use` 15, `type_ref` 13, `macro_def` 5, `macro_cond` 2). Aucun refus, aucun `##`, aucun défaut de l'outil. `make test` est vert après la seule application.

**TSV : aucune correction.** Les 33 noms neufs ont été relus pour le §1 et le §4 : aucune collision, aucun digramme, gardes d'inclusion de la forme de `DEVTOOLS_*_H`. Les 2 fonctions exportées sans le préfixe de leur fichier (relevé de 3c, « 3l 2 ») sont gardées :
- `p8_fatal`, symbole du runtime défini hors de `p8/`, comme les `p8_input_*` de 3k (§7) ;
- `raylib_backend_window_to_screen`, définie dans `raylib_backend_coords.c` : une méthode de la famille `raylib_backend`.

Si un nom se révèle contraire au §1 ou au §4, on le corrige comme en 3k, avant `rename-slice`. Toute autre correction du TSV va dans « Découvert », pour décision humaine.

### Binaires et sorties (tranche 2, lignes `fichier`)

Les noms sont ceux du TSV (2b) : `tmp/replay`, `replay_gcov`, `replay_sanitize`, `gif`, `minimize`, `class_coverage`, `frames_tool`, `game` ; `tmp/replay_dump.txt`, `tmp/demo_dump.txt`. Aucun ne collisionne avec un dossier de `tmp/`, une cible phony ou une règle à motif.

Renommés à tous leurs sites :
- **`Makefile`** : règles, prérequis, recettes, `FRAMES_TOOL`, le chemin de `float-link` (`tmp/float-link/replay`). L'aide en tête et les commentaires qui citent un binaire ou une sortie sont renommés aussi : c'est de l'aide à la lecture d'une commande, donc du code. Les cibles `make` gardent leur nom.
- **Tests** : `DRIVER`, `DRIVER_DUMP`, `COVER` et les chemins des 9 tests qui les citent, dont `test_headless_capture`, `test_live_slots`, `test_minimize`, `test_class_coverage` et `test_demo`.
- **`tools/float_grep_exclusions.txt`** : les 3 noms nus et `pico8_fatal`.
- **`tools/frames_ref.sh`, voie (a)** : le pilote vaut `tmp/replay` si le `Makefile` du worktree a la règle `$(TMP_DIR)/replay:`, sinon `tmp/pico8_replay`. C'est une ligne de branchement, comme le test de `src/main.c` ; vérifiée contre le `Makefile` de `e24c850` et le neuf. `tmp/pico8_replay` y reste donc écrit une fois : c'est la `string` restante, une exception proposée.
- **`.claude/skills/check-e2e/SKILL.md`** : une ligne, la commande de reproduction `./tmp/replay`. C'est une consigne d'agent exécutée juste après cette PRD : laissée fausse, elle lancerait un `tmp/pico8_replay` périmé.
- Sans changement : `.gitignore` (`tmp/` seul), `README.md` (ne cite aucun binaire), `tools/pico8_reference_probe*` (`pico8` y nomme le vrai PICO-8, pas le cart).
- Les docs (`docs/raylib-vendoring.md`, le vocabulaire, le plan) vont à l'étape 5.

Après le commit, effacer de `tmp/` les binaires et sorties aux anciens noms, pour qu'aucun agent ne lance une copie périmée et que les relevés partent d'une reconstruction.

### Chaînes et littéraux composés (tranche 2)

`rename-strings-verify SUBSTEP=3l` : **44 sites**, 35 en défaut et 9 `unseen` au départ.

Lignes `chaîne` de 3l :
- les 4 noms de test des `printf` ;
- les clés `le` → `ephemerals`, `qy` → `particles`, `qy_fv` → `particles_help_screen`, `ts` → `transit_dots`, des deux côtés (`cmd/replay.c`, `test_live_slots.c`) ;
- `"pico8_fatal: %s"` → `"p8_fatal: %s"`.

Les 9 `unseen` (3a) se vérifient à la main :
- les 6 `cmd/*.c` ne citent pas leur propre chemin ; seuls les commentaires et les messages d'`assert` y gardent un nom ;
- les 3 noms nus de `float_grep_exclusions.txt` sont remplacés.

Littéraux composés renommés : ils sont écrits ou lus dans des fichiers de 3l.
- **Préfixes des messages des binaires** : `"pico8_frames: …"`, `"usage: pico8_gif …"`, `"main_raylib: …"`, etc., dans les `fprintf` de `cmd/*.c`. Aucun test ne les lit : `test_replay_usage` ne cherche que `usage`.
- **`"…\npico8_fatal: zone…"`** attendu par `expect_fatal` de `test_zone.c` (fichier de 3c). Sans cette édition, `test_zone` rougit (vérifié).
- **La variable d'environnement `PICO8_REMPLISSAGE` → `REPLAY_FILL`** : lue par `cmd/replay.c`, écrite deux fois par `test_live_slots.c`. Elle porte le préfixe du cart et un mot français, et serait restée à côté de `fill_path` ; elle n'est dans aucune ligne du TSV.
- **Fixtures de `test_composition_residue.c`** : `"pico8_" "ke_t"` → `"entity_" "root_t"` (18 mentions) et `k.h_is_false` → `k.side_tooltip_is_false` (1). Le compteur les voit par hasard : `ke_t` est aussi une clé de `test_ui`, et `h_is_false` un champ. Ce sont des types et champs plantés, sans motif qui dépende de leur nom : `--detail` et `--champs` restent identiques.

Restent :
- les messages d'`assert` (`"usage: pico8_class_coverage …"`) et les commentaires, pour l'étape 4 ;
- les mots français des clés et du fichier de remplissage (`rangee`, `particule`, `max_vivantes`, `partie`, `chargement`), pour l'étape 4.

**Hachages** : aucun test de 3l ne replie de clé. Aucun rouge.

**Tampons** : `line[48]` de `test_live_slots.c` reçoit la ligne de remplissage la plus longue, 38 octets mesurés (`max_octets particles_help_screen 6264`). Aucun autre tampon ne reçoit un nom allongé.

### Gardes à motifs

L'inventaire de 3a n'a aucun élément traduisible en 3l. `composition_residue.sh`, `composition_residue_sites.txt`, `minimize.sh` et `pico8_reference_probe.p8` n'ont plus de mention en `identifier` ni en `string`. Leurs 14 mentions restantes sont des commentaires (étape 4).

Mesuré : `--detail` des deux scripts et `--champs` (260 champs, « 0 écriture » 15, « 0 hors constructeur » 27) sont identiques avant, après l'application et à la fin. Il n'y a aucune traduction ni aucun test de vivacité.

Pour décision humaine : après 3l, aucun prototype `pico8_X(` réel ne reste dans les en-têtes. Le motif générique `void-ptr` passe donc à vide sur l'arbre ; les fixtures l'exercent encore (proposition de 3a).

### Recâblage et fin de PRD

Recâblage : 0 identité à offset après l'application comme après les chaînes. `remplissage_path`, la seule identité décalée en 3j et 3k, est elle-même renommée. `rename-rekey-check SUBSTEP=3l` vert.

Relevés du §5 et des notes de 3a à 3k :
- `make test-all` vert, `tests/e2e/*.sha256` inchangés ;
- `make raylib` (→ `tmp/game`) et `make screencast-viewer` compilent et linkent, sans jamais être lancés ;
- `make gif` et `float-link` verts ;
- `rename-tool-test` (37 tests) et les `rename-*-test` verts ;
- `class-coverage-net` et `e2e-gap` rouges, avec des sorties égales à l'octet à celles de la fin de 3k (au nom des cibles près dans les lignes `make`) ;
- `data-residue` et `composition-residue` à 0 ;
- `scope-count SUBSTEP=3l` : 0 (1 avant, `R2`).

**Critère de fin de l'étape 3, et ce qu'on fait d'un reste.** Après un `make rename-extract` frais sans `warning:` :
- `make cart-names-count`, **sans `SUBSTEP` ni `MODULES`**, doit afficher `TOTAL` à **0 `identifier`, 1 `string`**. Cette `string` est la ligne `tools/frames_ref` (repli de la voie (a)). Les colonnes `backtick` et `comment` relèvent de l'étape 4.
- `make cart-names-count SUBSTEP=3l` doit afficher `done` 0 `identifier` et 1 `string`, et `later` 0.
- `rename-strings-verify SUBSTEP=3l` sort en échec sur ce seul site (« old name still there »), et liste les 9 `unseen` vérifiés à la main.

Tout autre reste trouvé à ce moment, quel que soit son module, est attribué à 3l et corrigé dans cette PRD, dans le même commit. Il est relevé dans « Découvert » (fichier, nom, cause de l'oubli).

Exception : si la correction demande de changer le TSV pour une autre raison que le §1 ou le §4, ou de modifier un outil figé, on s'arrête. Le cas va dans « Découvert », pour décision humaine, et la tranche ne se déclare pas finie. Dans la répétition, aucun reste n'a été trouvé.

## Implementation Chunks

1. **Identités.** `rename-slice` et `rename-apply` (33 identités), `rename-extract`, `rename-rekey` et `rename-rekey-check`. Avec `"pico8_fatal: %s"` → `"p8_fatal: %s"` et son lecteur `test_zone.c`, pour que la paire reste cohérente. Un commit, `make test-all` vert.
2. **Binaires, sorties, chaînes, composés, puis note.** Lignes `fichier` dans le `Makefile` (aide comprise), les tests, `tools/` (voie (a) de `frames_ref.sh`) et le skill `check-e2e`. Clés `le`, `qy`, `qy_fv`, `ts`, noms de test, `REPLAY_FILL`, préfixes des messages de `cmd/`, fixtures de `test_composition_residue.c`. Puis effacer les anciens binaires de `tmp/`, extraction fraîche, recâblage, relevés et critère de fin. La note « Découvert » sous l'entrée 3l, dans ce commit ou un commit de docs qui suit, relève :
   - les identités et occurrences, les sites et les `unseen` vérifiés ;
   - les composés ;
   - le repli de la voie (a), exception proposée ;
   - les comptes du critère de fin ;
   - ce qui va à la décision humaine (ci-dessous).

## Testing Decisions

- **Code de 3l** : le filet est le compilateur, `make test` et les e2e à l'octet. Aucun test neuf : renommer ne change aucun comportement.
- **Binaires** : chaque test qui lance un binaire (`test_headless_capture`, `test_frames_tool`, `test_replay_*`, `test_live_slots`, `test_minimize`, `test_class_coverage`, `test_line_coverage`) et chaque cible du `Makefile` qui en dépend prouve le nouveau chemin. Effacer les anciens binaires d'abord garantit qu'aucun vert ne vient d'une copie périmée.
- **Voie (a)** : aucun test ne couvre `e2e-ref`. La preuve est que le motif du branchement est trouvé dans le `Makefile` neuf et absent de celui de `e24c850`.
- **Composés** : `test_zone`, rouge sans l'édition de son attendu, est la preuve pour `p8_fatal:`. `test_live_slots`, qui compare le fichier de remplissage à l'octet, est la preuve pour les clés et `REPLAY_FILL`.
- **Gardes** : `--detail` et `--champs` avant et après ; `test_composition_residue` vert avec ses fixtures renommées.

## Out of Scope

- Commentaires, messages d'`assert`, mentions entre backticks ou nues, mots français des littéraux et des clés : étape 4.
- Docs (`README`, `docs/`) : étape 5.
- Renommer `PICO8_CLASS_COVERAGE` (`-D` du `Makefile`, 31 occurrences, aucune ligne du TSV) : décision humaine, notée par 3k.
- Modifier `tools/rename_strings.py` ou un autre outil figé (« Méthode des sous-étapes »).
- Réécrire les noms du cart coupés dans les fixtures des gardes (`"pico8_" "jo_t"`…), qu'aucun compteur ne voit et dont dépendent les motifs génériques.
- Scinder `p8/fatal.h` et `p8/input.h` (§7).
- Lancer l'appli raylib ou un binaire à fenêtre.

## Further Notes

- **Pour décision humaine** (à reporter dans la note « Découvert ») :
  - **Exception proposée à la garde anti-cart** : le repli `tmp/pico8_replay` de `tools/frames_ref.sh` (voie (a), 2b), seule `string` restante.
  - **`PICO8_CLASS_COVERAGE`** (3k) ; **`PICO8_MALLOC_FILLS`**, variable d'environnement de `test_entity_header.c` (3a), elle aussi au préfixe du cart et hors TSV.
  - **Noms de fichiers fictifs** `pico8_allocs.c`, `pico8_clean.c`, `pico8_gone.c` des fixtures de `test_alloc_guard.c` : hors TSV, invisibles au compteur.
  - **Motif générique `void-ptr`**, désormais à vide sur l'arbre (ci-dessus).
- **Mesures de départ** (arbre `e24c850`, `rename-extract` frais) :
  - `cart-names-count SUBSTEP=3l` : `done` 110 `identifier`, 83 `string`, 44 `comment` ; `later` 0 ;
  - `rename-strings-verify SUBSTEP=3l` : 35 sites en défaut sur 44, 9 `unseen` ;
  - `scope-count SUBSTEP=3l` : 1 ;
  - `rename-rekey-check SUBSTEP=3l` : 0 introuvable.

  `tmp/ast_rename/slice.tsv` et `cart_names_count.txt` ont été rendus à leur état d'avant la mesure.
- **Répétition à blanc** dans un worktree jetable, détruit depuis (vendor raylib et bindings clang liés). Elle a fait :
  - l'application, les chaînes, une extraction, le recâblage ;
  - `make test`, `make e2e`, `test-all`, `raylib`, `screencast-viewer`, `gif`, `float-link` ;
  - `rename-tool-test`, `class-coverage-net` et `e2e-gap` (comparés à l'arbre de départ) ;
  - les compteurs avec et sans `SUBSTEP`.

  Tous les chiffres ci-dessus en viennent. Elle a enchaîné chaînes et extraction en une fois : la tranche réelle suit l'ordre de 3a, avec une extraction après l'application, puis une après les chaînes.
- `rename-table-check` reste rouge par construction : la porte est `rename-rekey-check`.
- Durées : environ 1 min 30 par `rename-extract` ou `rename-apply`.
