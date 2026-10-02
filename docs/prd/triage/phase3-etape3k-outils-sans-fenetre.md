---
title: "Plan 5, sous-étape 3k — outils sans fenêtre"
description: "Les outils de vérification sans fenêtre (devtools/) et leurs onze tests passent aux noms anglais du TSV, avec une correction du TSV (le paramètre io_tail_offset, mal dérivé en print_outlined_tail_offset) ; aucun format sur disque ni aucune fixture e2e ne change."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 3k. **Outils sans fenêtre.** `devtools/` : `dump`, `gif`, `screencast`, `screencast_player`, `replay_control`, `replay_source`, `session_recorder`, `input_json`, `class_coverage`, `minimize`, `fixed_test_double`. Tests : `test_dump`, `test_gif`, `test_screencast`, `test_headless_capture`, `test_screencast_player`, `test_replay_control`, `test_replay_source`, `test_marker_coherence`, `test_recording_continuity`, `test_class_coverage`, `test_minimize`. Résidu 0.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4), onzième étape.

Entrées : `docs/plan-5.md` §1, §4, §5, l'entrée 3 avec « Sous-étapes » et « Méthode des sous-étapes », et les notes « Découvert » de **3a à 3j**. Cette PRD y renvoie sans les recopier :
- l'ordre des commandes (3a) ;
- l'appartenance des chaînes (règle du site `écrivain`) ;
- les littéraux composés et recollés (3b, 3f) ;
- la répétition à blanc en worktree jetable ;
- le recâblage après chaque édition de texte, avec `rename-rekey-check` pour porte (3a) ;
- la correction du TSV appliquée par-dessus un nom déjà appliqué (3c, `entity_geom_shake`).

Exemple à suivre : `c239333` (3j, une seule tranche).

Aucun humain n'était disponible. L'agent rédacteur a chargé lui-même la consigne `guideline` aux points « vérifier avec l'utilisateur » de `to-prd`. Les décisions ci-dessous reposent sur le texte du plan et sur les mesures, sans interview.

**Condition d'entrée** :
- `origin/plan-5-noms-anglais` est au HEAD de départ (`16f4908` ou un descendant), l'arbre est propre et `make test-all` est vert ;
- `make rename-extract` est frais, sans ligne `warning:` (les 4 lignes `partial` de `class_coverage` sont attendues) ;
- `make rename-rekey-check SUBSTEP=3k` est vert. Mesuré : 0 identité introuvable.

## Problem Statement

Après 3j, les outils de vérification sans fenêtre gardent le préfixe du cart :
- `pico8_screencast_*`, `pico8_screencast_player_*`, `pico8_replay_control_*`, `pico8_replay_source_*`, `pico8_session_recorder_*`, `pico8_gif_*`, `pico8_dump_*`, `pico8_minimize_mechanical`, `pico8_class_coverage_*` ;
- `pico8_fixed_from_double` et `pico8_fixed_to_double` ;
- `pico8_input_load/save/append/free`, définis dans `devtools/input_json.c` mais déclarés dans `p8/input.h` (note de 2b) ;
- leurs types, macros (`PICO8_SCREENCAST_FRAME_BYTES`…) et gardes d'inclusion (`PICO8_X_H`) ;
- quelques champs et locales courts (`ok`, `w`, `p`, `b`, `g`, `u8`).

`pico8_class_coverage_record` est le résidu de 4 laissé par 3j.

La sous-étape compte **87 identités**, soit **967 occurrences AST dans 68 fichiers**. Son résidu est 0 : à la fin, aucun nom de 3k ni des précédentes ne reste en `identifier` ni en `string` dans ses fichiers, qui ne citent plus que des noms de 3l.

Une répétition à blanc a été faite dans un worktree jetable, avec les étapes suivantes :
1. correction du TSV ;
2. application ;
3. `make test` ;
4. gardes ;
5. chaînes ;
6. `make test` et `make e2e` ;
7. extractions fraîches, recâblages et compteurs.

Elle a trouvé :
- **Une correction du TSV** : le paramètre `io_tail_offset` de `pico8_input_append`, nommé `print_outlined_tail_offset` par erreur de dérivation.
- **Aucun refus, aucune garde touchée, aucun hachage, aucun tampon.** `make test` reste vert après la seule application.
- **Aucun format sur disque ni aucun nom de fichier** ne porte un ancien nom. Les fixtures e2e ne bougent pas.

## Solution

Une seule tranche, dans l'ordre de commandes de la note de 3a, sur `SUBSTEP=3k` :
1. corriger le TSV (`io_tail_offset` → `tail_offset`, définition et en-tête) ;
2. appliquer les 87 identités ;
3. recâbler ;
4. éditer les 13 sites de chaînes ;
5. recâbler de nouveau, faire les relevés et écrire la note « Découvert ».

La répétition l'a établi : avec ces gestes, `make test-all` est vert (26 e2e conformes, `tests/e2e/` intact), et `make raylib` et `make screencast-viewer` compilent. `class-coverage-net` et `e2e-gap` sont rouges, avec des sorties égales à l'octet à celles de la fin de 3j. `scope-count SUBSTEP=3k` passe de 7 à **0**. Pas un pixel ne change.

## User Stories

1. En tant qu'agent de 3k, je veux appliquer les 87 identités en une commande, afin que le compilateur arbitre tout oubli dans les 68 fichiers.
2. En tant qu'agent de 3k, je veux savoir d'avance que l'application seule laisse `make test` vert, afin qu'un rouge m'arrête.
3. En tant que lecteur, je veux lire `screencast_*`, `replay_source_*`, `replay_control_*`, `session_recorder_*`, `gif_*`, `dump_*`, `class_coverage_*` et `fixed_test_double_*`, afin de comprendre les outils de vérification sans glossaire.
4. En tant que lecteur, je veux que le paramètre de reprise de `p8_input_append` s'appelle `tail_offset`, comme le champ de `session_recorder_t` qui le lui passe, et non `print_outlined_tail_offset`, qui nomme le texte détouré du HUD.
5. En tant que mainteneur, je veux savoir que les 4 `p8_input_*` gardent le préfixe du runtime alors qu'ils sont définis dans `devtools/`, afin de ne pas rouvrir la scission d'en-tête (§7).
6. En tant que mainteneur, je veux que `--detail` et `--champs` des deux gardes à motifs restent identiques du début à la fin, afin de prouver qu'aucune garde ne change de verdict.
7. En tant qu'agent de 3k, je veux renommer les 10 noms de test, le chemin `tmp/test_minimize_work` et le symbole cité de `test_float_grep.c`, afin que `rename-strings-verify SUBSTEP=3k` soit vert, sans site `unseen`.
8. En tant que mainteneur des e2e, je veux la preuve que le format des captures, du dump, du GIF, des `events.json` et des enregistrements de session ne contient aucun ancien nom, afin qu'aucun fichier de `tests/e2e/` ne change.
9. En tant qu'agent de 3k, je veux fermer le résidu de 3j (`pico8_class_coverage_record` ×4), afin que `cart-names-count SUBSTEP=3j` tombe à `later` 0.
10. En tant qu'agent de 3l, je veux trouver un TSV recâblé, avec `rename-rekey-check` vert pour 3k et 3l.
11. En tant que relecteur humain, je veux la liste des noms qu'aucune garde ne voit et des outils figés qui citent un nom de 3k, afin de les trancher au point d'arrêt d'avant l'étape 5.
12. En tant que joueur, je ne veux voir aucun pixel changer.

## Implementation Decisions

### Périmètre

`rename-slice SUBSTEP=3k` rend **87 identités** :
- par module : `screencast` 18, `replay_control` 11, `replay_source` 11, `gif` 8, `input_json` 7, `screencast_player` 7, `session_recorder` 7, `class_coverage` 6, `dump` 3, `fixed_test_double` 3, `test_gif` 3, `minimize` 2, `test_class_coverage` 1 ;
- par sorte : 44 fonctions, 17 macros, 11 types, 7 constantes, 4 locales, 2 champs, 1 paramètre, 1 statique.

Occurrences mesurées en répétition : `ref` 405, `macro_arg` 250, `type_ref` 135, `decl` 112, `macro_use` 36, `macro_def` 17, `macro_cond` 10, `macro_body` 2. Le total, 967, est celui de la table de 2b. Il n'y a aucun refus, aucun `##` et aucun défaut de l'outil.

L'application déborde de `devtools/` : `cmd/` (7 fichiers), `platform/raylib_backend.[ch]`, `p8/input.h`, et 19 fichiers de `game/` (`class_coverage_record`) — tous des sites d'identités de 3k.

Pour l'étape 4 : 7 `backtick` et 44 `comment` dans les fichiers de 3k, plus 1 `backtick` et 3 `comment` de noms de 3l. Aussi le message d'`assert` `"pico8_screencast_player_format_refusal: rien a refuser"` (`screencast_player.c`), qui n'est pas un site du TSV, et les messages de refus français de `screencast_player.c`, attendus mot pour mot par `test_screencast_player.c`.

### TSV : une correction, pour le §1

**Le paramètre `io_tail_offset` de `pico8_input_append`.** `io_` y veut dire « entrée-sortie » : l'appelant passe l'offset, la fonction le met à jour. La dérivation y a lu le concept `io` du cart (texte détouré du HUD, `print_outlined`). Elle a donc produit `print_outlined_tail_offset`, aux deux identités du paramètre :
- la définition, dans `devtools/input_json.c` (3k) ;
- le prototype, dans `p8/input.h`, déjà appliqué par 3b.

Le nom dit autre chose que ce qu'il nomme : ce n'est pas un nom clair (§1).

Correction, avant `rename-slice` :
- nom neuf : **`tail_offset`**, celui du champ de `session_recorder_t` qui est passé à ce paramètre, et des locales de `test_input.c` ;
- trois éditions :
  - les deux lignes de `docs/renommage.tsv` ;
  - une ligne `field pico8_input_append.io_tail_offset tail_offset` dans `tools/rename_overrides.tsv`, à côté de `field pico8_input_load.ok` ;
  - le prototype de `p8/input.h`, renommé par-dessus en une ligne (une occurrence), comme `entity_geom_shake` en 3c.

Mesuré en répétition : 6 occurrences dans `input_json.c`, aucun conflit avec le champ ou les locales `tail_offset`. Les mentions `` `*io_tail_offset` `` des commentaires de `p8/input.h` vont à l'étape 4.

**Contrôle des autres noms neufs.** Aucune collision et aucun digramme du cart (§1). Aucun nom de deux lettres hors tolérance : les 7 refus de `scope-count` portent tous un nom long dans le TSV (`succeeded`, `writer`, `cursor`, `read_u8`, `buffer`, `gif`). `is_known` est une fonction `static`, donc sans préfixe (§4). `screencast_frame_is_8192_bytes_t` est un type, l'assertion statique de taille.

**Les 4 fonctions exportées sans le préfixe de leur fichier** (relevé de 3c, « 3k 4 ») sont `p8_input_load`, `p8_input_save`, `p8_input_append` et `p8_input_free`. Elles sont gardées telles quelles : ce sont des symboles du runtime (§2, §3), déclarés dans `p8/input.h`. La scission d'en-tête est notée par le vocabulaire et laissée hors périmètre (§7).

### Formats sur disque et fixtures

Aucun ne porte un ancien nom (vérifié dans le code et sur les 78 fichiers de `tests/e2e/`) :
- **capture** (`screencast`) : des frames brutes de 8 192 octets, sans en-tête ni nombre magique ;
- **dump** : `"%d %d\n"` puis des chiffres hexadécimaux ;
- **GIF** : `GIF89a`, `NETSCAPE2.0` ;
- **`events.json`** : les clés `frames`, `mouse_x`, `mouse_y`, `mouse_buttons`, `dpad` ;
- **enregistrements de session** : `tmp/events_recorded_<t>.json` et `.markers.txt`, nommés par `platform/raylib_backend.c` (3l) ;
- **sorties du `Makefile`** : `tmp/screencast.bin`, `tmp/frames/*.bin`, `*.dump.txt`.

Les noms des 26 e2e et leur contenu ne citent aucun ancien nom du TSV. `tests/fixtures/` hors `ast_rename*` ne cite aucun nom de 3k.

Restent pour 3l, en `later string` dans les fichiers de 3k, des chemins de binaires (lignes `fichier`) :
- `./tmp/pico8_replay` et `tmp/pico8_replay_dump.txt` dans `test_headless_capture.c` ;
- `./tmp/pico8_minimize` dans `test_minimize.c` ;
- `./tmp/pico8_class_coverage` dans `test_class_coverage.c`.

### Gardes à motifs

L'inventaire de 3a n'a aucun élément en 3k. Mesuré en répétition :
- aucun motif ni aucune exemption de `tools/` ne cite un nom de 3k ;
- `composition-guard` et `data-guard` restent verts ;
- `--detail` des deux scripts et `--champs` identiques avant, après l'application et à la fin (260 champs, « 0 écriture » 15, « 0 hors constructeur » 27).

Il n'y a aucune traduction ni aucun test de vivacité.

Motif générique `void-ptr` (`pico8_X(`) : après 3k, il ne reste de prototype `pico8_` réel que dans `platform/` (3l). Les autres `pico8_X(` des en-têtes sont dans des commentaires.

`tests/test_float_grep.c` : la fixture `int pico8_fixed_to_double;` du cas `test_identifier_suffix_is_not_a_word` devient `int fixed_test_double_to_double;`. Le cas garde son sens : `double`, collé à `_`, n'est pas un mot pour `\b`. Le test reste vert.

### Chaînes

`rename-strings-verify SUBSTEP=3k` compte 13 sites, tous en défaut au départ :
- **10 noms de test**, dans les `printf("test_pico8_X: …")` de `test_dump`, `test_gif`, `test_headless_capture`, `test_marker_coherence`, `test_minimize`, `test_recording_continuity`, `test_replay_control`, `test_replay_source`, `test_screencast` et `test_screencast_player` ;
- **1 chemin `tmp/`** : `tmp/test_pico8_minimize_work` → `tmp/test_minimize_work` (`test_minimize.c`), qu'aucun autre fichier ne cite ;
- **1 symbole cité, 2 sites** : `pico8_fixed_to_double` → `fixed_test_double_to_double`.
  - Le site écrivain, `fixed_test_double.c`, passe dès l'application.
  - Le site lecteur, `tests/test_float_grep.c`, est un test de 3l renommé par 3k, qui tient le site écrivain.

Après l'édition, la répétition mesure 0 site en défaut sur 13 et **0 `unseen`**.

**Littéraux composés** : la recherche des anciens noms de 3a à 3k dans les littéraux des fichiers de 3k (littéraux adjacents recollés) ne laisse que :
- le message d'`assert` cité plus haut (étape 4) ;
- les chemins `tmp/pico8_*` (3l) ;
- des mots français ;
- des chemins d'`#include` et des formats `printf` ou `fopen`.

Aucun composé n'est à renommer.

**Hachages** : aucun test de 3k ne replie une clé. Prévu et mesuré : aucun rouge, aucune constante à reporter. Un rouge de hachage arrête la tranche (`help-me-debug`).

**Tampons** : aucun ne reçoit un nom allongé. Tous les noms raccourcissent (préfixe `pico8_` retiré), et `SESSION_RECORDER_PATH_MAX`, `REPLAY_CONTROL_STATUS_MAX` et `SCREENCAST_PLAYER_REFUSAL_MAX` gardent leur valeur.

### Application et recâblage

- Ordre : celui de la note de 3a, sur `SUBSTEP=3k`, la correction du TSV d'abord.
- Recâblage mesuré :
  - **1 identité** après l'application : `remplissage_path` de `cmd/replay.c` (3l), décalée par les appels renommés ;
  - **0** après les chaînes.
- `rename-rekey-check` vert pour 3k et 3l (mesuré).

### Fin de PRD

Les relevés du §5 et des notes de 3a à 3j, sur `SUBSTEP=3k` :
- `make test-all` vert, `tests/e2e/*.sha256` inchangés ;
- `make raylib` et `make screencast-viewer` compilent et linkent, sans jamais être lancés ;
- `rename-tool-test` (37 tests) et les `rename-*-test` verts ;
- `class-coverage-net` et `e2e-gap` rouges à l'identique de la fin de 3j (mesuré : sorties égales à l'octet) ;
- `data-residue` et `composition-residue` à 0 ;
- après un `rename-extract` frais, sans `warning:` :
  - `cart-names-count SUBSTEP=3k` : `done` 0 en `identifier` et en `string`, `later identifier` 0 ≤ 0, `later string` 4 (chemins de binaires de 3l) ;
  - `cart-names-count SUBSTEP=3j` : `later identifier` 0 (4 avant) ;
  - `rename-strings-verify SUBSTEP=3k` : 0 site en défaut sur 13, 0 `unseen` ;
  - `scope-count SUBSTEP=3k` à 0 (7 avant).

## Implementation Chunks

1. **Sous-étape 3k entière, note comprise.**
   - Un commit de code qui laisse `make test-all` vert. Il contient :
     - la correction `io_tail_offset` → `tail_offset` (deux lignes du TSV, une ligne de surcharges, le prototype de `p8/input.h`) ;
     - l'application ;
     - les 13 sites de chaînes ;
     - le TSV recâblé.
   - La note « Découvert » sous l'entrée 3k, dans le même commit ou dans un commit de docs qui suit. Elle relève :
     - les identités et les occurrences ;
     - la correction du TSV ;
     - les 4 `p8_input_*` gardés ;
     - l'absence de garde touchée, de hachage et de tampon ;
     - les formats sur disque vérifiés ;
     - les sites de chaînes ;
     - les recâblages ;
     - ce qui va à la décision humaine (ci-dessous) ;
     - les relevés de fin.
   - Le plan est corrigé dans le même commit si un constat le contredit.

## Testing Decisions

- **Code de 3k.** Le filet est le compilateur, `make test` et les e2e à l'octet. Aucun test neuf.
- **Formats sur disque.** Les e2e (`frames` conformes) et `test_headless_capture`, qui lance le vrai pilote, prouvent qu'aucun octet capturé ne change.
- **Correction du TSV.** La preuve est `rename-apply` sans conflit, `make test` vert (`test_input`, `test_recording_continuity`, `test_marker_coherence` appellent `p8_input_append`), et `scope-count SUBSTEP=3k` à 0.
- **Noms de test et chemin `tmp/`.** Ce ne sont que des libellés, ou un dossier privé du test. `rename-strings-verify` en est la preuve.
- **Garde `float-grep`.** `test_float_grep` reste vert avec la fixture renommée : c'est la preuve que le cas du suffixe garde son sens.
- **Gardes à motifs.** Pas de test neuf, aucun motif n'est touché. `--detail` et `--champs` avant et après servent de preuve.
- **Outils de méthode.** Inchangés : le schéma du TSV et ses lecteurs restent figés.

## Out of Scope

- Les binaires et sorties `tmp/pico8_*` (lignes `fichier`), `platform/` et `cmd/` : 3l.
- Commentaires, messages d'`assert`, mentions entre backticks ou nues, et mots français des littéraux : étape 4.
- Scinder `p8/input.h` pour que les `p8_input_*` vivent dans `p8/` (§7).
- Renommer `PICO8_CLASS_COVERAGE` ou modifier `tools/rename_strings.py` (ci-dessous).
- Renommer par goût un nom conforme au §4.
- Lancer l'appli raylib.

## Further Notes

- **Pour décision humaine** (à reporter dans la note « Découvert ») :
  - `PICO8_CLASS_COVERAGE`, 31 occurrences dans `src/` et `tests/`, n'est dans aucune ligne du TSV. C'est un `-D` du `Makefile` et de la configuration `coverage` de `rename-extract`, sans `#define` que l'extraction verrait. Il porte le préfixe du cart, interdit au jeu par le §2, et aucune garde ne le compte. Propriétaire naturel : 3l, qui tient le `Makefile`, ou l'étape 5.
  - `tools/rename_strings.py` reconnaît les noms de classe par le motif `\bpico8_class_coverage_record\(`, qui n'apparie plus rien après 3k. C'est l'outillage figé du dictionnaire, déjà caduc sur l'arbre renommé. À reporter à l'étape 5, avec les lignes `pz` et `go` éditées hors de `rename-strings`.
  - `p8_input_*` définis hors de `p8/` : la scission d'en-tête de 2b, toujours non traitée.
- Mesures de cette PRD, sur l'arbre `16f4908` avec un `rename-extract` frais, sans `warning:` :
  - `cart-names-count SUBSTEP=3k` : `done` 685 `identifier`, 7 `backtick`, 11 `string`, 44 `comment` ; `later` 0 `identifier` (borne 0), 1 `backtick`, 4 `string`, 3 `comment` ;
  - `rename-strings-verify SUBSTEP=3k` : 13 sites, 13 en défaut, 0 `unseen` ;
  - `scope-count SUBSTEP=3k` : 7 ;
  - `rename-rekey-check SUBSTEP=3k` : 0 introuvable.

  `tmp/ast_rename/slice.tsv` et `cart_names_count.txt` ont été rendus à leur état d'avant la mesure.
- Répétition à blanc dans un worktree jetable, détruit depuis. Elle a fait toute la tranche puis tous les relevés :
  - la correction du TSV, l'application, les chaînes et les deux recâblages ;
  - `make test`, `make e2e`, `test-all`, `raylib`, `screencast-viewer` ;
  - `rename-tool-test`, les `rename-*-test` et `rename-substeps-check` ;
  - `class-coverage-net`, `e2e-gap` ;
  - une extraction fraîche et les compteurs.

  Tous les chiffres ci-dessus en viennent : 71 fichiers touchés, TSV et surcharges compris.
- `rename-table-check` reste rouge par construction : la porte est `rename-rekey-check`.
- Durées : chaque `rename-extract` ou `rename-apply` prend environ 1 min 30, et la tranche en enchaîne trois.
