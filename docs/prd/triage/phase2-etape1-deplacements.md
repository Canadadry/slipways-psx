---
title: "Plan 5, étape 1 — déplacements seuls"
description: "Les 389 fichiers de src/, tests/ et tools/ portent encore les noms plats et minifiés du port ; l'étape les range dans l'arborescence validée du vocabulaire, sans changer un symbole ni un pixel."
status: needs-triage
---

## Problem Statement

Le plan 5 (`docs/plan-5.md`) donne des noms anglais clairs à tout le code. Sa première étape de code est le §6, entrée 1, reprise mot pour mot :

> 1. **Déplacements seuls.** `git mv` vers l'arborescence du §3 (noms de fichiers anglais du vocabulaire), includes en `"dir/fichier.h"`, `-Isrc`, `Makefile`, chemins dans `tools/`. Aucun symbole ne change. Commits de `git mv` purs, séparés des retouches de contenu, pour que `git log --follow` suive : le commit de `git mv` seul ne compile pas, l'invariant du §5 se vérifie sur la paire qu'il forme avec son commit de retouches, sans commit intermédiaire.

Aujourd'hui, `src/` est plat : 171 fichiers `pico8_*` et `main*.c`, sans distinction entre le runtime PICO-8 émulé, le socle objet, le jeu, la plateforme et les outils de vérification. Les tests incluent les en-têtes par `"../src/pico8_x.h"`, beaucoup de fichiers de test portent un digramme du cart (`test_pico8_jo.c`), et quelques fichiers de `tests/fixtures/` et de `tools/` ont un nom français.

L'arborescence cible est déjà tranchée, fichier par fichier : c'est la section « Arborescence » de `docs/vocabulaire.md`. Ce document porte « Statut : validé » depuis le commit `361579b`. L'étape l'applique sans rien inventer. Cette PRD y renvoie et n'en recopie pas le contenu.

Le risque n'est pas de déplacer, c'est de casser en silence ce qui lit des chemins ou des noms de fichiers. C'est le cas des listes d'objets du `Makefile`, des inclusions de dépendances `.d`, des scripts de `tools/` qui reconnaissent un fichier par son nom, et des références de couverture qui portent des `fichier:ligne`. Certains de ces lecteurs ne cassent pas : ils deviennent vides, et la garde qu'ils portent passe alors à vide.

## Solution

L'étape procède en sept tranches de déplacement, une par famille de répertoires cibles, puis une tranche de clôture. Chaque tranche de déplacement produit deux commits poussés sur la branche `plan-5-noms-anglais` :

1. un commit de **renommages purs** : chaque fichier change de chemin et garde exactement son contenu (similarité 100 %). Ce commit ne compile pas.
2. un commit de **retouches** : includes, `Makefile`, chemins dans `tools/` et dans les tests, références de couverture. Le build est réparé.

C'est la paire qui respecte l'invariant du §5. `make test-all` passe et aucun `tests/e2e/*.sha256` ne change. Les références qui contiennent des chemins sont réécrites par la table de déplacement, jamais régénérées.

Pendant l'étape, `-Isrc` est le seul chemin d'inclusion ajouté. Il rend compilable l'état mixte entre deux tranches : un fichier déjà rangé qui inclut encore `"pico8_x.h"` trouve l'en-tête resté plat dans `src/`. À la fin, chaque include d'un en-tête du port s'écrit depuis `src/` (`"game/planet.h"`), y compris depuis `tests/`. `src/` ne contient plus aucun fichier à plat, et une note « Découvert » est ajoutée sous l'entrée 1 du plan.

## User Stories

1. En tant qu'agent de l'étape, je veux vérifier que `docs/vocabulaire.md` porte la ligne exacte « Statut : validé », pour ne rien déplacer sur une arborescence non validée.
2. En tant qu'agent de l'étape, je veux vérifier que les commits de l'étape 0 sont poussés (`origin/plan-5-noms-anglais` égal à la branche locale), pour partir d'un état que l'humain a pu relire.
3. En tant qu'agent de l'étape, je veux savoir si l'humain a corrigé des noms à la validation, pour propager ses corrections aux noms qui en dérivent avant de déplacer.
4. En tant qu'agent de l'étape, je veux rejouer sur la version validée le contrôle d'exhaustivité de l'arborescence, pour que chaque fichier suivi sous `src/`, `tests/` et `tools/` ait exactement une ligne.
5. En tant qu'agent de l'étape, je veux rejouer le contrôle d'unicité des chemins cibles sans tenir compte de la casse, pour qu'aucun déplacement n'en écrase un autre, même sur un système de fichiers insensible à la casse.
6. En tant qu'agent de l'étape, je veux rejouer le contrôle des répertoires cibles légitimes, pour qu'aucun fichier n'atterrisse hors de `p8/`, `engine/`, `game/`, `platform/`, `devtools/`, `cmd/`, `tests/`, `tests/e2e/`, `tests/fixtures/` ou `tools/`.
7. En tant qu'agent de l'étape, je veux qu'un `.c` et son `.h` partent vers le même répertoire sous le même nom, pour que le module reste lisible.
8. En tant qu'agent de l'étape, je veux une table de déplacement extraite mécaniquement du vocabulaire, pour ne jamais retaper un chemin à la main.
9. En tant qu'agent d'une tranche, je veux que l'extraction et la réécriture soient décrites assez précisément pour être réécrites, pour ne pas dépendre d'un script jetable d'un agent précédent.
10. En tant que relecteur humain, je veux que chaque déplacement soit un commit de renommages purs, pour que `git log --follow` suive chaque fichier jusqu'à son histoire.
11. En tant que relecteur humain, je veux que les retouches de contenu soient dans un commit à part, pour relire d'un coup d'œil des diffs qui ne changent que des chemins.
12. En tant que mainteneur, je veux que chaque paire de commits laisse `make test-all` vert et tous les `*.sha256` de `tests/e2e/` identiques, pour que l'invariant du §5 tienne à chaque pas.
13. En tant que mainteneur, je veux que `-Isrc` soit le seul chemin d'inclusion du port, pour qu'un include dise toujours depuis quel répertoire il vient.
14. En tant que mainteneur, je veux que `-Isrc` survive à un `CFLAGS` passé en ligne de commande, pour qu'un build qui surcharge les flags ne casse pas.
15. En tant que mainteneur, je veux que les tests incluent les en-têtes du port comme `src/` (`"p8/mem.h"`), sans `"../src/"`, pour qu'un seul style d'include existe.
16. En tant que mainteneur, je veux que les dépendances d'en-têtes `.d` des objets rangés en sous-répertoires soient encore relues par le `Makefile`, pour qu'un en-tête modifié recompile ses utilisateurs.
17. En tant que mainteneur, je veux que les gardes nommées sur des fichiers (`composition-guard`, `float-grep`, `alloc-guard`, `constants-check`) visent les nouveaux chemins, pour qu'aucune ne passe à vide.
18. En tant que mainteneur, je veux que les tests unitaires des scripts de garde exercent les nouveaux noms, pour qu'une garde migrée de travers casse un test.
19. En tant que mainteneur, je veux que `tools/line_coverage_reference.txt` et `tools/line_coverage_unreachable.txt` suivent les fichiers déplacés ligne pour ligne, pour que `make e2e-gap` mesure le même écart.
20. En tant que mainteneur, je veux qu'aucune retouche n'ajoute ni ne retire de ligne dans un `.c` du port, pour que les numéros des références de couverture restent justes.
21. En tant que mainteneur, je veux que les noms des binaires d'outil (`tmp/pico8_replay`, `tmp/pico8_class_coverage`, `tmp/main_raylib`…) ne changent pas, pour que `make e2e-ref` construise encore un commit ancien.
22. En tant que mainteneur, je veux que chaque binaire de test porte le nom de son fichier de test renommé, pour que `make unit` reste lisible.
23. En tant que mainteneur, je veux que les fichiers de `tools/` et de `tests/fixtures/` renommés par le vocabulaire le soient avec tous leurs lecteurs, pour qu'aucun défaut de chemin ne masque une liste absente.
24. En tant que mainteneur, je veux que les doutes de la colonne « doute » du vocabulaire ne soient pas tranchés ici, pour qu'une étape de déplacement ne devienne pas un redécoupage (§7).
25. En tant qu'agent de l'étape 2a, je veux trouver `src/` rangé et compilable avec `-Isrc`, pour bâtir l'outil de renommage AST sur l'arborescence finale.
26. En tant que pilote du plan, je veux une note « Découvert » sous l'entrée 1, pour savoir quels fichiers ont été rangés autrement que le §3 et quelles cibles étaient déjà rouges avant l'étape.
27. En tant que pilote du plan, je veux qu'un état rouge préexistant d'une cible hors suite soit relevé au départ et comparé à l'arrivée, pour ne pas imputer à l'étape une panne plus ancienne, ni en masquer une nouvelle.
28. En tant qu'agent qui écrit un message de commit, je veux qu'il soit en français, pour suivre le §2.

## Implementation Decisions

### Condition d'entrée (avant tout déplacement, première tranche)

- **Statut** : `docs/vocabulaire.md` contient une ligne exactement `Statut : validé`.
- **Commits poussés** : `git rev-parse HEAD` et `git rev-parse origin/plan-5-noms-anglais` sont égaux après un `git fetch`. Au moment de la rédaction, les deux valent `361579b`.
- **Corrections humaines** : entre l'arborescence proposée (`fe3beef`) et la version validée (`361579b`), le seul changement de `docs/vocabulaire.md` est la ligne de statut. L'humain a validé tel quel : il n'y a rien à propager. L'agent le reconfirme par `git diff fe3beef 361579b -- docs/vocabulaire.md` (une ligne changée). Toute autre différence le fait propager la correction aux noms dérivés avant de déplacer, et le noter dans « Découvert ».
- **Contrôles mécaniques**, rejoués sur la version validée. Le script de l'étape 0 était jetable ; on le réécrit dans `tmp/plan5/` à partir de cette description :
  - *Extraction* : on prend la section qui commence à la ligne `## Arborescence` et qui s'arrête au titre `## ` suivant. On garde les lignes de table qui commencent par « | ` », puis les colonnes 2 à 4 (chemin actuel, répertoire cible, nom cible), sans espaces de bord ni backticks. Le chemin cible est la concaténation répertoire + nom.
  - *Exhaustivité* : l'ensemble trié des chemins actuels est égal à `git ls-files src tests tools` trié (389 aujourd'hui), sans doublon.
  - *Unicité* : aucun chemin cible en double, comparaison en minuscules.
  - *Répertoires légitimes* : un fichier de `src/` va dans `src/{p8,engine,game,platform,devtools,cmd}/`. Un fichier de `tests/e2e/` reste dans `tests/e2e/`, un fichier de `tests/fixtures/` dans `tests/fixtures/`, un fichier à la racine de `tests/` dans `tests/`, et un fichier de `tools/` dans `tools/`. Le nom cible n'est jamais vide et ne contient pas de `/`.
  - *Paires* : un `.c` et le `.h` de même chemin actuel ont le même chemin cible au suffixe près.
  - Les quatre passent aujourd'hui (389 lignes). Un échec arrête l'étape avant tout déplacement.
- **Ligne de base** : avant la première tranche, on enregistre dans `tmp/plan5/baseline/` la sortie de `make e2e-gap` (liste `tmp/line_coverage_gap.txt` et code de retour), celle de `make class-coverage-net`, celles de `make data-residue` et `make composition-residue`. On tente aussi `make raylib-vendor-setup`, puis `make raylib` et `make screencast-viewer`. Constat à la rédaction, sur `361579b` : `make test-all` est vert, mais `make e2e-gap` est **déjà rouge** (2932 lignes d'écart) et `make class-coverage-net` aussi (`trade` 247 contre 263 attendus, `lq` 2742 contre 2763). L'étape ne répare pas ces deux cibles. Son critère est qu'elles ne bougent pas : même sortie qu'à la ligne de base une fois les chemins réécrits par la table de déplacement.

### Table de déplacement et réécriture (outillage jetable, dans `tmp/plan5/`)

- **Table** : `tmp/plan5/moves.tsv`, deux colonnes `ancien<TAB>nouveau`, avec les lignes de l'extraction dont le chemin cible diffère du chemin actuel (260 aujourd’hui : 171 sous `src/`, 79 à la racine de `tests/`, 5 sous `tests/fixtures/`, 5 sous `tools/`). Chaque tranche en filtre le sous-ensemble de son répertoire cible et l'écrit dans `tmp/plan5/moves-<tranche>.tsv`. Ce fichier sert à la fois aux déplacements, aux réécritures et au commit de renommages.
- **Déplacement côté implémenteur** : `mkdir -p` du répertoire cible, puis `mv` système de fichiers. L'implémenteur ne touche jamais à git (prd-autopilot). Le commit de renommages purs est reconstruit par le commit (voir plus bas).
- **Règle R1, includes** : pour chaque en-tête déplacé dont l'ancien chemin est `src/pico8_X.h` et le nouveau `src/D/N.h`, on remplace la chaîne exacte `#include "pico8_X.h"` et la chaîne exacte `#include "../src/pico8_X.h"` par `#include "D/N.h"`. Cela vaut dans tous les `.c`/`.h` de `src/` (récursif) et de `tests/`, et dans les includes que génèrent les scripts de `tools/`. On cherche la chaîne entière entre guillemets : `"pico8_zone.h"` n'atteint pas `"pico8_zone_frame.h"`. L'en-tête de test `tests/zone_tas.h` suit la même règle vers `"heap_allocator.h"`, include local et sans répertoire.
- **Règle R2, chemins** : on remplace chaque ancien chemin complet (`src/pico8_X.c`, `tests/test_pico8_Y.c`, `tests/fixtures/…`, `tools/…`) par le nouveau, partout où il apparaît en tant que chemin. C'est le cas dans le `Makefile` (listes de sources, prérequis, recettes, et aussi l'aide en tête), dans les scripts et les listes de `tools/` (`line_coverage_reference.txt`, `line_coverage_unreachable.txt`, `float_grep_exclusions.txt`, la colonne fichier de `composition_r2_sites.txt`), et dans les chaînes des tests (`FIXED` de `test_constants_check`, chemins de fixtures). Le chemin doit être suivi d'un caractère qui n'appartient pas au nom (espace, `:`, `"`, `|`, tabulation, fin de ligne).
- **Règle R3, binaires de test** : un nom de binaire de test suit son fichier : `test_pico8_jo` devient `test_planet`, dans `UNIT_TESTS`, les cibles `$(TMP_DIR)/test_*` et les `unit-run-*` spéciales. Les variantes suffixées suivent aussi (`test_pico8_fixed_ubsan` devient `test_fixed_ubsan`). On les inventorie par `grep -o 'test_pico8_[a-z0-9_]*' Makefile` avant de réécrire. Après la tranche des tests, le `Makefile` ne contient plus aucun `test_pico8_`.
- **Une ligne reste une ligne** : aucune réécriture n'ajoute, ne retire ni ne réordonne de ligne dans un `.c` ou un `.h`. On ne retrie pas les includes. C'est ce qui permet aux références `fichier:ligne` de suivre par la seule réécriture du chemin.
- **Hors règles** : les noms de fichier cités dans un commentaire de `src/` ou `tests/` sans chemin complet (`entity.c:217`, « voir pico8_ke.c ») ne sont pas touchés. Les étapes 3 et 4 les traitent avec les symboles. Les notes en texte libre de `composition_r2_sites.txt` non plus : seule la colonne fichier, que le script lit, est réécrite.

### Build

- **`-Isrc`** est ajouté à `CFLAGS` par un `override CFLAGS += -Isrc` placé après le `CFLAGS ?=`. Il atteint ainsi les règles d'objets des six arbres, les recettes raylib, `float_guard.sh` et `float_grep.sh` (qui reçoivent `"$(CFLAGS)"`, et `-MM` doit trouver les en-têtes) et `gen_quarter_wave`. Aucun autre `-I` du port n'est ajouté. `-I third_party/raylib/src` reste dans les deux recettes raylib.
- **Dépendances `.d`** : les six `-include $(wildcard $(X)/src/*.d $(X)/tests/*.d)` ne voient que la racine de `src/`. On ajoute `$(X)/src/*/*.d` dès la première tranche. La tranche de clôture retire `$(X)/src/*.d`, devenu sans objet. Aucun test ne détecte cette panne : elle fait seulement manquer une recompilation incrémentale. C'est donc une vérification explicite : après la tranche, `touch src/p8/mem.h && make -n tmp/pico8_replay` doit reconstruire des objets de `tmp/obj/src/game/`.
- **Binaires d'outil** : leurs noms ne changent pas (`tmp/pico8_replay`, `tmp/pico8_replay_gcov`, `tmp/pico8_class_coverage`, `tmp/pico8_minimize`, `tmp/main_raylib`, `tmp/screencast_viewer`…). `tools/frames_ref.sh` construit `tmp/pico8_replay` dans un worktree d'un commit ancien, et ce nom doit rester stable dans l'histoire.
- **`sin-table`** : la recette écrit désormais l'en-tête dans `src/p8/quarter_wave.h`.

### Scripts de `tools/` qui reconnaissent un fichier par son nom

- **`composition_residue.sh`** :
  - `NOYAU` (aujourd'hui les noms de `pico8_struct.h`, `pico8_ke.h` et `pico8_entity.h`) passe aux trois en-têtes cibles de `engine/`. Le test se fait sur le chemin relatif à `src/` (`engine/entity.h`) plutôt que sur le seul nom de base, puisque les noms de base ne sont plus préfixés.
  - Les exclusions de `pico8_ptrs.[ch]` (`registres`, `ptrs-entité`, en-têtes) passent à `engine/pointer_list.[ch]`.
  - L'expression de `noyau-inclut` (`#include "pico8_(game_data|map_tables|data_init)\.h"`) passe à la forme `"game/(…)\.h"` dans la tranche qui déplace ces trois en-têtes.
  - Les défauts `ATTEINTE` et `R2` suivent le renommage de leurs listes.
  - `tests/test_composition_residue.c` réécrit ses fixtures synthétiques (fichiers de `SRC` et includes qu'ils contiennent) avec les nouveaux noms dans la même tranche. Sans cela, la garde migrée de travers passerait à vide sans qu'aucun test ne rougisse.
- **`data_residue.sh`** : ses motifs visent des en-têtes supprimés au plan 3 (`pico8_value.h`, `pico8_ob.h`…), absents de la table. On ne les touche pas.
- **`constants_compare.sh`** : il compile une source générée avec `-I` égal au répertoire de `pico8_fixed.c`, et cette source inclut `"pico8_fixed.h"`. Il passe à l'include `"p8/fixed.h"` et à un `-I` sur la racine des sources (le parent du répertoire de `fixed.c`). `test_constants_check` reste vert, seul son chemin `FIXED` change.
- **`alloc_guard.sh`**, **`float_guard.sh`**, **`line_coverage.sh`** et le collecteur gcov dérivent le chemin source du chemin objet ou de gcov. Ils marchent tels quels avec des sous-répertoires. On ne les touche pas.
- **`frames_ref.sh`** teste `"$worktree/src/main.c"` pour décider d'un drapeau d'avant l'écran titre. Ce chemin désigne l'arborescence des commits anciens qu'il reconstruit : on ne le touche pas. Sur un commit postérieur, le fichier est absent, le drapeau reste vide, et c'est le bon comportement. Le cas est noté dans « Découvert ».
- Les tests de `test_alloc_guard`, `test_float_link` et `test_e2e_cleanup` écrivent des fichiers synthétiques nommés `pico8_*.c` ou `src/a.c`. Ce sont des données de test, pas des chemins du port : on ne les touche pas.

### Commit de chaque tranche (orchestrateur)

L'implémenteur laisse un arbre de travail où les fichiers sont déplacés et retouchés, avec `tmp/plan5/moves-<tranche>.tsv`. Le commit (sous-agent `git-commit`) produit deux commits, sans renommage par `git mv` après coup, puisque les fichiers sont déjà déplacés :

1. **Renommages purs** : pour chaque ligne `ancien nouveau`, on lit le mode et le blob de l'ancien chemin dans `HEAD` (`git ls-files -s -- ancien`). On l'inscrit à `nouveau` par `git update-index --add --cacheinfo <mode>,<blob>,nouveau`, puis on retire `ancien` de l'index par `git rm --cached -q -- ancien`. On vérifie que `git diff --cached -M --name-status` ne montre que des `R100`, autant que de lignes dans la table, puis on fait le commit. Le message, en français, dit que le commit ne compile pas seul et nomme la tranche.
2. **Retouches** : `git add -A -- src tests tools Makefile docs`, puis on vérifie que `git diff --cached -M --name-status` ne contient ni ajout ni suppression de fichier, puis on fait le commit.

La paire est poussée avec la suite de l'étape (prd-autopilot pousse à la fin de la PRD). Aucun commit intermédiaire ne s'insère entre les deux.

## Implementation Chunks

Ordre : des feuilles vers les points d'entrée. Les en-têtes de `p8/` sont inclus partout, ceux de `cmd/` nulle part. Chaque tranche réécrit, dans tout le dépôt, les références aux seuls fichiers qu'elle déplace. Grâce à `-Isrc`, les inclusions encore à l'ancienne vers des fichiers restés plats continuent de compiler. Chaque tranche se termine par la vérification du « Testing Decisions », puis par sa paire de commits.

1. **Entrée, outillage, `src/p8/`** (18 fichiers). Condition d'entrée et ligne de base, réécriture des contrôles dans `tmp/plan5/`, extraction de `moves.tsv`. Ajout de `override CFLAGS += -Isrc` et de `src/*/*.d` aux six `-include`. Déplacements vers `p8/`. R1 sur tout le dépôt. R2 sur le `Makefile` (dont `constants-check`, la recette `sin-table`, les deux recettes raylib), sur les références de couverture et sur `float_grep_exclusions.txt`. `constants_compare.sh` et le `FIXED` de `test_constants_check`.
2. **`src/engine/`** (32 fichiers). R1, R2. Dans `composition_residue.sh`, `NOYAU` et les exclusions de `pointer_list`, avec les fixtures de `test_composition_residue` correspondantes. Colonne fichier de `composition_r2_sites.txt`.
3. **`src/game/`, noms cibles de `a` à `m`** (41 fichiers ; le découpage suit l'initiale du nom cible). R1, R2. L'expression `noyau-inclut` de `composition_residue.sh` et sa fixture, puisque `game_data`, `map_tables` et `data_init` sont de cette tranche.
4. **`src/game/`, noms cibles de `n` à `z`** (47 fichiers). R1, R2.
5. **`src/platform/`, `src/devtools/`, `src/cmd/`** (33 fichiers). R1, R2. Cela couvre `REPLAY_SRCS` et les autres listes de points d'entrée, les recettes raylib et `screencast-viewer`, les lignes `src/main.c:` des références de couverture (vers `src/cmd/replay.c`) et `float_grep_exclusions.txt`. Fin de tranche : `git ls-files 'src/*' | grep -v '^src/[^/]*/'` est vide.
6. **Fichiers à la racine de `tests/`** (79 fichiers : les tests renommés et `zone_tas.[ch]`). R1 pour `zone_tas.h`, R2, R3. Après la tranche, plus aucun `test_pico8_` ni `zone_tas` dans le `Makefile`.
7. **`tests/fixtures/` et `tools/`** (5 + 5 fichiers). R2 dans le `Makefile`, les tests et les scripts. Cela couvre les défauts `ATTEINTE`/`R2` de `composition_residue.sh`, les chemins des listes de `constants-check` et le relevé de la sonde PICO-8.
8. **Clôture** (commit de contenu seul, sans renommage). On retire `src/*.d` des six `-include`. On lance les cibles de fin de PRD et on les compare à la ligne de base. On ajoute la note « Découvert » sous l'entrée 1 du §6 de `docs/plan-5.md`. Elle contient : les cibles rouges à la ligne de base et inchangées, l'état de `make raylib` et `screencast-viewer`, le cas de `frames_ref.sh`, les noms de fichier laissés dans les commentaires, tout fichier rangé autrement que le §3, et les doutes du vocabulaire rencontrés sans être tranchés. Si un constat contredit le §3, le plan est corrigé dans le même commit.

## Testing Decisions

- **Pas de nouveau test unitaire.** Un déplacement ne crée pas de comportement. Le cycle « rouge » de `tdd` est le build cassé par le commit de renommages, et le « vert » la paire réparée. Le filet existant suffit : TU, gardes de `make test`, e2e et leurs `sha256`. Les seuls tests modifiés sont ceux dont les fixtures nomment un fichier que la tranche renomme (`test_composition_residue`, `test_constants_check`, et les tests qui lisent une fixture renommée).
- **Vérification de chaque tranche**, par l'implémenteur puis par le vérificateur. Pour cette PRD, le vérificateur lance `make test-all` et non `make test` seul : `make e2e` dure environ 1 s, et l'invariant du §5 l'exige.
  - `make test-all` est vert ;
  - `git status --porcelain tests/e2e` est vide (aucun `*.sha256` touché) ;
  - le nombre de fichiers déplacés est celui de `moves-<tranche>.tsv`, et chacun est à son chemin cible ;
  - aucune ligne ajoutée ni retirée dans les `.c`/`.h` retouchés : `git diff --numstat` montre autant d'insertions que de suppressions par fichier ;
  - chaque chemin des deux références de couverture existe dans l'arbre (`cut -d: -f1 | sort -u` comparé à l'arbre de travail) ;
  - plus aucun ancien chemin de la tranche dans le `Makefile` ni dans `tools/` (`grep -F` de chaque ancien chemin et ancien nom de base, hors `frames_ref.sh`) ;
  - après la tranche 1, le contrôle `touch` + `make -n` des dépendances `.d`.
- **Fin de PRD**, en tranche 8 :
  - `make raylib` et `make screencast-viewer` compilent et linkent, sans lancer le binaire (`CLAUDE.md`), après `make raylib-vendor-setup`. Si l'environnement ne permet pas l'installation du vendor, on l'écrit dans « Découvert », sans prétendre la cible verte ;
  - `make class-coverage-net` donne la même sortie qu'à la ligne de base ;
  - `make e2e-gap` donne la même liste d'écart qu'à la ligne de base, réécrite par `moves.tsv` ;
  - `make data-residue` et `make composition-residue` donnent les mêmes comptes qu'à la ligne de base.
- **Art antérieur** : le commit `346651f` (#35) a remis à jour `line_coverage_reference.txt` par réécriture, sans régénération. Les tests de scripts sur fixtures synthétiques (`test_composition_residue`, `test_constants_check`) donnent le modèle des fixtures à migrer.

## Out of Scope

- Tout changement de symbole, y compris les gardes d'inclusion (`PICO8_MEM_H`), les préfixes `pico8_` des fonctions et les noms de binaires d'outil. C'est l'objet des étapes 2b et 3.
- Trancher les doutes de la colonne « doute » du vocabulaire : `trade` contre `trade_route`, scission de `p8/input.h` et de `devtools/input_json.c`, singletons `main_*`, `entity_registry` contre `entity_register`, fusion de `main_economy.h` dans `economy.h`. On applique la table telle quelle (§7 : pas de redécoupage).
- Les noms de fichier cités sans chemin dans les commentaires et les notes en texte libre, qui relèvent des étapes 3 et 4.
- Le `README`, le `CLAUDE.md`, `docs/*.md` et les skills qui citent des chemins, mis à jour à l'étape 5.
- Réparer les écarts préexistants de `make e2e-gap` et de `make class-coverage-net`.
- `psx-hello/`, `pico8/`, `tests/e2e/` (aucun fichier renommé).

## Further Notes

- Choix faits sans humain disponible : l'agent de rédaction ne pouvait pas lancer de sous-agent. Il a chargé lui-même le skill `guideline` et l'a appliqué aux points « vérifier avec l'utilisateur » de `to-prd`. Résultat :
  - pas de module neuf ni d'outil commité, l'outillage reste jetable dans `tmp/plan5/` et décrit ici ;
  - pas de nouveau test, seuls changent les tests dont les fixtures nomment un fichier renommé ;
  - changements chirurgicaux : binaires, gardes d'inclusion, commentaires et doutes du vocabulaire restent en l'état ;
  - critère vérifiable par tranche.
- Le découpage en tranches par répertoire cible suit l'ordre des dépendances d'inclusion. Il garde chaque paire de commits assez petite pour être relue, et tous les déplacements d'un même répertoire dans un seul commit de renommages.
- Le vocabulaire reste la seule source des chemins cibles. Une incohérence découverte en cours (un fichier absent de la table, un chemin cible qui collisionne) arrête la tranche et remonte en `QUESTION:`. Elle n'est jamais corrigée en inventant un nom.
