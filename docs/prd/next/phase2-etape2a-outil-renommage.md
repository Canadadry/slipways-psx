---
title: "Plan 5, étape 2a — outil de renommage AST"
description: "Avant de produire le TSV, il faut un outil qui extrait chaque déclaration du code et en renomme toutes les occurrences aux positions que donne clang, et savoir s'il tient face aux macros."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 2a. **Outil de renommage AST.** Extraction et application, prouvées de bout en bout sur un petit module, invariant compris, puis code rendu à son état d'origine : s'il ne tient pas face aux macros (`##`, `#define`), on le sait avant de produire le TSV. Sa note « Découvert » (macros gérées ou non, forme réelle des positions) est l'entrée de 2b.

Phase 2 du pilotage (tronçon 2 : étapes 1, 2a, 2b). Entrées : `docs/plan-5.md` (§2, §4, §5, §6 entrées 0, 1, 2b et 3), `docs/vocabulaire.md` (statut validé). Cette PRD est écrite **avant** que l'étape 1 soit faite : tout ce qu'elle suppose du résultat de l'étape 1 est une hypothèse H-1-k (section dédiée en fin de document), à revérifier dès que la note « Découvert » de l'étape 1 existe.

## Problem Statement

Le chantier du plan 5 va renommer des milliers de symboles : types, fonctions, champs, constantes, macros, statiques, globales. Une bonne part sont des digrammes minifiés (`p`, `t`, `jo`, `kj`) qu'aucune recherche texte ne peut cibler sans faux positifs : un champ `p` existe dans plusieurs structs, `t` est aussi une locale, `kj` est à la fois un membre de vtable et un fragment de dizaines de noms de fonctions. Le plan interdit le `sed` naïf et exige un renommage « par token à partir des positions exactes données par l'AST clang ».

Rien de tel n'existe dans le dépôt. Et on ne sait pas encore ce que clang donne vraiment : une position est-elle en octets ou en caractères ? Que devient une occurrence écrite dans l'argument d'un `assert`, dans le corps d'un `#define`, dans un initialiseur désigné `.kj = …`, dans une branche `#ifdef PICO8_CLASS_COVERAGE` inactive ? Si l'outil rate ces cas, le TSV de 2b sera faux ou incomplet, et les sous-étapes de l'étape 3 casseront le build sans qu'on sache pourquoi. Il faut le savoir **avant** de produire le TSV, sur un module assez petit pour que l'échec soit lisible.

## Solution

Un outil en ligne de commande, dans `tools/`, écrit en anglais (§2 : `tools/` est dans le périmètre de l'anglais), avec deux verbes :

- **extract** : parcourt toutes les unités de compilation du dépôt, dans toutes les configurations de compilation qui existent, et produit la table de chaque déclaration (identité, sorte, nom, fichier de déclaration, portée) avec toutes ses occurrences (fichier, offset en octets, longueur, origine). C'est l'entrée mécanique du TSV de 2b et, plus tard, de la garde anti-cart sur les noms courts.
- **apply** : reçoit une table `ancien → nouveau` et réécrit chaque occurrence, octet par octet, à la position exacte. Il refuse en bloc, sans rien écrire, dès qu'un cas n'est pas sûr.

On le prouve de bout en bout sur un petit module : extraction, renommage de tous les symboles du module, `make test-all` vert et `*.sha256` des e2e inchangés, plus les cibles hors suite du §5 ; puis on rend le code à son état d'origine. Seuls l'outil, ses tests, ses cibles `make` et la note « Découvert » sont commités. La note dit ce que l'outil gère (macros comprises) et la forme réelle des positions : c'est l'entrée de 2b.

## User Stories

1. En tant qu'agent de l'étape 2b, je veux une extraction exhaustive de toutes les déclarations nommées du code, afin de dériver le TSV sans rien chercher à la main.
2. En tant qu'agent de l'étape 2b, je veux que chaque déclaration ait une identité stable d'une unité de compilation à l'autre, afin qu'un même type déclaré dans un en-tête inclus 60 fois ne compte qu'une fois.
3. En tant qu'agent de l'étape 2b, je veux distinguer deux champs `p` de deux structs différentes, afin de leur donner des noms neufs différents si leur sens diffère (cf. `p` sur la caméra dans le vocabulaire).
4. En tant qu'agent de l'étape 2b, je veux connaître la sorte de chaque déclaration (type, fonction, champ, constante d'enum, macro, globale, statique, paramètre, locale), afin d'appliquer la bonne règle du §4.
5. En tant qu'agent de l'étape 2b, je veux la portée d'une locale ou d'un paramètre en lignes, afin d'appliquer la tolérance des deux lettres sur 40 lignes au plus.
6. En tant qu'agent de l'étape 2b, je veux que les macros (`#define`) apparaissent dans l'extraction comme des déclarations à part entière, avec leurs utilisations, afin que `PICO8_*` et les constantes locales (`BTN_O`) entrent dans le TSV.
7. En tant qu'agent de l'étape 2b, je veux que les déclarations et occurrences présentes seulement sous `-DPICO8_CLASS_COVERAGE` ou sous ASan soient extraites, afin que le TSV n'oublie pas le code des branches `#ifdef`.
8. En tant qu'agent de l'étape 2b, je veux la liste des unités de compilation que l'outil n'a pas pu analyser, afin de savoir quand l'extraction n'est pas exhaustive.
9. En tant qu'agent de l'étape 3, je veux appliquer une table de renommages en une commande, afin que chaque sous-étape soit mécanique.
10. En tant qu'agent de l'étape 3, je veux que les occurrences dans les en-têtes, inclus par plusieurs unités, soient réécrites une seule fois, afin de ne pas corrompre un fichier par des réécritures répétées.
11. En tant qu'agent de l'étape 3, je veux que les occurrences en argument d'une macro (`assert(ptrs != NULL && …)`) soient renommées à leur place réelle, afin que les milliers d'`assert` du code suivent.
12. En tant qu'agent de l'étape 3, je veux que les occurrences dans le corps d'un `#define` soient renommées une fois, dans le `#define`, et pas au point d'expansion, afin de ne pas écraser le nom de la macro elle-même.
13. En tant qu'agent de l'étape 3, je veux que les initialiseurs désignés (`.kj = …`) suivent le champ qu'ils désignent, afin que les archétypes `static const` restent corrects.
14. En tant qu'agent de l'étape 3, je veux que les utilisations d'un nom de type (déclarations de variables, paramètres, casts, `sizeof`) suivent le `typedef`, afin qu'aucun type ne garde son ancien nom.
15. En tant qu'agent de l'étape 3, je veux que l'outil refuse d'écrire quoi que ce soit si une entrée de la table est introuvable, ambiguë ou en collision, afin de ne jamais laisser le dépôt à moitié renommé.
16. En tant qu'agent de l'étape 3, je veux que l'outil vérifie, avant d'écrire, que les octets à chaque position sont bien l'ancien nom, afin qu'une position décalée soit détectée et non appliquée.
17. En tant qu'agent de l'étape 3, je veux un rapport de ce qui a été appliqué (nombre d'occurrences par entrée et par origine), afin de le citer dans la note « Découvert » de la sous-étape.
18. En tant qu'agent de l'étape 3, je veux que l'outil ne touche ni aux commentaires ni aux chaînes, afin que les mentions entre backticks restent le travail de l'étape 4 et les clés chaîne celui de leur sous-étape.
19. En tant qu'agent de l'étape 3, je veux que l'outil refuse d'appliquer quand une unité de compilation qui utilise un symbole renommé n'a pas pu être analysée, afin de ne pas rater ses occurrences en silence.
20. En tant qu'agent de l'étape 3, je veux que l'outil ne dépende pas d'une fenêtre ni de raylib lié, afin de respecter la règle « ne jamais lancer l'appli ».
21. En tant que mainteneur, je veux que l'outil lise ses flags de compilation depuis le `Makefile`, afin qu'il n'y ait qu'une source de vérité pour `-std=c99`, `-Isrc` et les configurations.
22. En tant que mainteneur, je veux que l'outil ait ses propres tests sur un mini-arbre fabriqué, afin qu'une régression de l'outil se voie sans rejouer tout le dépôt.
23. En tant que mainteneur, je veux que la preuve de bout en bout laisse le code exactement dans son état d'origine, afin que l'étape ne renomme rien en avance sur le TSV.
24. En tant que mainteneur, je veux que la note « Découvert » dise précisément quelles origines d'occurrence sont gérées et lesquelles ne le sont pas, afin que 2b et 3 sachent où le filet du compilateur doit prendre le relais.
25. En tant que mainteneur, je veux savoir si `##` est un risque réel dans ce code, afin de ne pas construire de traitement pour un cas qui n'existe pas.
26. En tant que relecteur humain au point d'arrêt d'après 2b, je veux que les identités de l'extraction soient lisibles (nom, sorte, struct parente, fichier), afin de relire un échantillon du TSV sans ouvrir l'AST.
27. En tant qu'agent de l'étape 2b, je veux que la sortie de l'extraction soit un fichier texte tabulé et trié de façon stable, afin que deux extractions sur le même code donnent le même fichier.
28. En tant qu'agent de l'étape 2b, je veux que l'extraction signale les noms de 1–2 caractères et les noms déjà déclarés ailleurs avec la même orthographe, afin de repérer les cas que seul l'AST peut renommer.
29. En tant qu'agent de l'étape 5, je veux pouvoir réutiliser l'extraction pour la garde anti-cart sur les noms courts (§5), afin de ne pas réécrire un second analyseur.
30. En tant que mainteneur, je veux que l'environnement de l'outil (bindings libclang) s'installe par une cible `make` dédiée, hors de `make test`, sur le modèle du vendoring raylib, afin que `make test` reste rapide et hors réseau.

## Implementation Decisions

### Moteur : libclang, identité par USR

- Le moteur est **libclang** (clang 18 du conteneur), piloté en Python 3 par les bindings officiels `clang` épinglés sur la même version majeure que la bibliothèque système. L'outil vérifie au démarrage que les deux versions concordent et échoue avec un message clair sinon.
- Pourquoi pas `-Xclang -ast-dump=json` seul, que le §6 (entrée 2b) cite : les sondes de rédaction montrent qu'il **n'émet ni les utilisations de types** (pas de `TypeLoc` : `pico8_ptrs_t *ptrs` n'y apparaît que comme `qualType`), **ni les désignateurs** (`.kj = …` disparaît dans la forme sémantique de l'`InitListExpr`), **ni les macros**. Il ne peut donc pas porter l'application. libclang donne `TYPE_REF`, `MEMBER_REF` (désignateurs), `MACRO_DEFINITION` / `MACRO_INSTANTIATION` (avec l'enregistrement détaillé du préprocesseur), et une USR par déclaration (`c:@SA@pico8_cheat_t@FI@matched`, `c:@T@pico8_cheat_t`) qui sert d'identité inter-unités. La note « Découvert » le dit, et l'étape corrige la mention de `-ast-dump=json` dans l'entrée 2b du plan dans le même commit si le moteur retenu diffère.
- Pourquoi pas `clang-rename-18` : il gère types, désignateurs, arguments et corps de macro, mais pas les noms de macros, exige un offset dans le fichier principal de chaque unité, ne fait pas d'extraction, et n'existe plus dans les clang récents. Il peut servir d'oracle de comparaison dans les tests, pas de moteur.
- Les bindings ne sont pas commités : une cible `make` (nom indicatif : `rename-tool-setup`) les installe dans un répertoire gitignoré, comme `raylib-vendor-setup` pour raylib. Les cibles de l'outil échouent avec un renvoi à cette cible s'ils manquent.

### Positions

- Une position est **(fichier, offset en octets, longueur en octets)**, dans le fichier où le token est **écrit** (sa *spelling location*). Jamais une colonne en caractères : 108 lignes de code du dépôt contiennent de l'UTF-8 hors commentaires, et l'outil lit et écrit les fichiers en octets.
- L'outil prend la position de chaque curseur par `clang_getSpellingLocation`, **pas** par la position par défaut des bindings (`clang_getInstantiationLocation`) : celle-ci ramène tout argument de macro sur le token de la macro (dans `assert(zone != NULL …)`, `zone` est rendu sur `assert`).
- Chaque occurrence porte une **origine** : `decl`, `ref` (expression), `type_ref`, `designator`, `macro_arg`, `macro_body`, `macro_def`, `macro_use`. Le rapport d'application compte par origine.

### Cas des macros

- **Noms de macros** : déclarations de sorte `macro`, identité = nom + fichier + offset de la définition ; utilisations via `MACRO_INSTANTIATION`. Les deux définitions d'un même nom sous `#ifdef`/`#else` (`POISON`, `PICO8_NORETURN`) sont une seule identité ; l'outil les regroupe par nom et fichier et le signale.
- **Arguments de macro** : position réelle via la *spelling location* (cf. ci-dessus).
- **Corps de `#define`** : libclang rend une référence écrite dans le corps d'une macro au point d'expansion (sonde : `sequence` dans `SEQUENCE_LENGTH` est rendu sur le token `SEQUENCE_LENGTH`). L'outil le détecte (les octets à la position ne sont pas l'ancien nom), remonte à la définition de la macro développée à cet endroit, et y cherche les tokens identifiants de cette orthographe : un seul candidat → occurrence `macro_body`, réécrite une fois dans le `#define` ; plusieurs candidats d'identités différentes → refus, signalé. Une occurrence dont la position ne contient pas l'ancien nom et qui n'est pas dans ce cas est une erreur de l'outil, pas un cas à ignorer.
- **Collage `##`** : un seul usage dans le code, sur un littéral (`raw##u` dans `PICO8_FIXED_LIT`) ; aucun identifiant n'est fabriqué par collage, et pas de stringification `#x` d'identifiant. L'outil ne gère pas `##` : il **détecte** tout `##` ou `#` dont un opérande est un identifiant renommé et refuse. La note « Découvert » confirme le constat sur l'arbre d'après l'étape 1.

### Configurations et unités de compilation

- Unités analysées : tous les `.c` suivis par git sous `src/`, `tests/` et `tools/` (cf. H-1-6).
- Flags : ceux du `Makefile`, passés par la cible qui invoque l'outil, jamais recopiés dans l'outil. Configurations analysées : défaut, `$(COVERAGE_CFLAGS)` (`-DPICO8_CLASS_COVERAGE`, 31 `#ifdef` dans 21 fichiers aujourd'hui), ASan (`__has_feature(address_sanitizer)` → `PICO8_ZONE_ASAN`). Les unités qui incluent `raylib.h` reçoivent en plus son chemin d'en-tête. L'union des occurrences de toutes les configurations fait foi ; une même position vue dans plusieurs configurations n'est écrite qu'une fois.
- Une unité qui ne s'analyse pas sans erreur (diagnostic de sévérité ≥ erreur) est listée. **extract** la liste en tête de sa sortie et sort en échec. **apply** refuse si une unité non analysée contient, en texte brut, l'orthographe d'un ancien nom de la table ; sinon il continue en le signalant. Sans `third_party/raylib`, les unités raylib ne s'analysent pas : l'étape lance `make raylib-vendor-setup` (requis de toute façon par le §5 pour `make raylib`) ; si c'est impossible dans le conteneur, la note « Découvert » le dit.

### Interface

- **extract** → un fichier tabulé, trié de façon stable, une ligne par identité : identité (USR), sorte, nom, struct ou fonction parente le cas échéant, fichier et ligne de déclaration, liaison (`extern` / `static` / locale), portée en lignes pour une locale ou un paramètre, nombre d'occurrences, marqueurs (`short` pour 1–2 caractères, `homonym` si une autre identité a la même orthographe). Un second fichier, optionnel, liste les occurrences (identité, fichier, offset, longueur, origine) pour le débogage et la garde anti-cart.
- **apply** ← une table tabulée `ancien`, `nouveau`, `fichier` (fichier de la déclaration), la forme prévue du TSV de 2b ; une quatrième colonne optionnelle, l'identité, lève une ambiguïté (deux champs `p` déclarés dans le même fichier). L'étape 2b peut faire évoluer ce format : l'outil suit.
- **apply** vérifie tout avant d'écrire le premier octet : chaque entrée résout exactement une identité ; le nouveau nom n'est pas déjà déclaré dans la même portée (globale/macro au niveau fichier, champ dans la même struct, locale dans la même fonction) ni par une autre entrée de la table ; chaque position contient l'ancien nom ; aucune position ne chevauche une autre. Un seul échec → rien n'est écrit, sortie en échec, rapport des causes.
- **apply** ne touche jamais un commentaire ni une chaîne : il n'écrit qu'aux positions de tokens identifiants rendues par clang. Les noms portés par une chaîne restent hors de l'outil (cf. Out of Scope).
- **apply** écrit un rapport : par entrée, nombre d'occurrences par origine et par fichier.
- Pas de renommage de fichiers : les déplacements sont l'étape 1, les noms de tests et de fixtures sont des entrées du TSV traitées par les sous-étapes de l'étape 3.

### Code

- Python 3, un module par responsabilité, chacun testable seul : **analyse** (libclang → occurrences brutes par unité et configuration), **résolution** (occurrences brutes → identités, avec le cas `macro_body`), **plan de réécriture** (table + identités → liste vérifiée de réécritures, ou refus motivé), **écriture** (réécritures → fichiers, en octets, de la fin vers le début de chaque fichier). Le plan de réécriture est le module profond : il prend des données, rend des données, ne touche aucun fichier.
- Identifiants et commentaires en anglais, règles de commentaires du `CLAUDE.md`.

## Implementation Chunks

1. **Mise en place** : cible d'installation des bindings, vérification de version, cible qui lance l'outil avec les flags du `Makefile`. → vérif : l'outil s'importe, refuse une version discordante.
2. **extract** sur le mini-arbre de test, puis sur tout le dépôt : toutes les unités, trois configurations, unités non analysées listées. → vérif : tests de l'outil ; extraction du dépôt stable entre deux exécutions ; sorte, homonymes et portées plausibles sur un échantillon.
3. **Plan de réécriture et apply** sur le mini-arbre, refus compris. → vérif : tests de l'outil.
4. **Preuve de bout en bout** sur le module de preuve (cf. Testing Decisions) : table écrite à la main pour tous ses symboles, apply, invariant complet, mesures pour la note, puis code rendu à son état d'origine (`git checkout` des fichiers touchés, `git status` propre hors outil). → vérif : `make test-all`, `*.sha256` des e2e inchangés, cibles hors suite du §5 ; après restauration, `git diff` vide sur `src/` et `tests/`.
5. **Note « Découvert »** sous l'entrée 2a du plan, et correction de l'entrée 2b si le moteur ou le format diffèrent de ce qu'elle dit.

## Testing Decisions

- **Ce qu'est un bon test ici** : un test donne à l'outil un mini-arbre C fabriqué et une table, puis vérifie le fichier produit (extraction) ou les sources réécrites (apply), et le code de sortie. Il ne regarde ni les curseurs libclang ni les structures internes. Le plan de réécriture se teste aussi directement, données en entrée, données en sortie.
- **Cas obligatoires du mini-arbre** : un `typedef struct` anonyme et ses usages en paramètre, cast et `sizeof` ; un champ homonyme dans deux structs, un seul renommé ; un champ désigné dans un initialiseur `static const` ; une occurrence en argument d'`assert` ; une statique référencée dans le corps d'un `#define` ; une macro objet et une macro fonction, avec leurs utilisations ; une définition sous `#ifdef` présente seulement dans une configuration ; une ligne avec de l'UTF-8 dans une chaîne avant l'identifiant renommé ; un en-tête inclus par deux unités (réécrit une fois) ; une mention de l'ancien nom dans un commentaire et dans une chaîne (intactes) ; une locale de deux lettres (portée mesurée).
- **Cas de refus** : entrée introuvable, entrée ambiguë sans identité, nouveau nom en collision, `##` sur un nom renommé, unité non analysée qui contient l'ancien nom. Pour chacun : sortie en échec et **aucun** fichier modifié.
- **Hors `make test`** : les tests de l'outil ont une cible à eux, hors suite, car ils exigent les bindings installés (cf. décision « Moteur »). `make test` et `make e2e` ne changent pas.
- **Prior art** : `tests/test_composition_residue.c` et ses voisins (`test_data_residue`, `test_alloc_guard`) testent un outil de `tools/` sur un mini-arbre fabriqué dans `tmp/`, par sa sortie et son code de retour. Les tests de l'outil suivent le même schéma ; écrits en Python ou en C, au choix de l'implémentation, tant qu'ils restent boîte noire.
- **Module de preuve** : `cheat` (H-1-8). Petit (96 lignes), sept fichiers utilisateurs dont un pilote raylib, et il porte tous les cas difficiles réels : `typedef struct` anonyme (`pico8_cheat_t`), champs (`prev_o`, `prev_x`, `matched`), fonctions exportées et statiques (`apply`, `feed`), macros objet (`BTN_O`, `BTN_X`), statique référencée dans un corps de `#define` (`sequence` dans `SEQUENCE_LENGTH`), champs à deux lettres d'un autre module lus en passant (`jk`, `nz`, à ne pas renommer). La table de preuve renomme tous les symboles déclarés par le module (y compris sa garde d'inclusion) vers des noms anglais provisoires ; elle n'est pas le TSV et n'est pas commitée.
- **Invariant de la preuve** (§5) : `make test-all` vert, `*.sha256` de `tests/e2e/` identiques, puis `make raylib` et `screencast-viewer` (après `raylib-vendor-setup`), `class-coverage-net`, `e2e-gap`. Une occurrence oubliée doit casser le build : la preuve vérifie aussi en négatif qu'un renommage volontairement incomplet (une ligne de la table retirée pour un symbole utilisé ailleurs) fait échouer la compilation, pour montrer que le filet du compilateur existe sur ce module.
- **Sondes de rédaction** (à HEAD `361579b`, avant l'étape 1, à refaire) : `clang-rename-18` + `clang-apply-replacements-18` ont renommé `pico8_cheat_t` sur six unités, en-tête dédupliqué, `make test-all` vert ; l'occurrence de `src/main_raylib.c` a été manquée parce que l'unité ne s'analyse pas sans `third_party/raylib`. C'est le cas que la règle « unité non analysée » doit attraper.

## Out of Scope

- Produire le TSV, choisir les noms neufs, découper l'étape 3 : c'est 2b.
- Renommer quoi que ce soit dans le code commité : la preuve est rendue à l'état d'origine.
- Les noms portés par une chaîne (`pico8_cg("jo")`, clés de la couverture de classes, fichiers attendus de `tools/`) : catégorie à part du TSV (2b), renommés des deux côtés par chaque sous-étape (3). L'extraction peut les lister plus tard ; pas dans cette étape.
- Commentaires, messages d'`assert`, mentions entre backticks : étape 4.
- Renommer des fichiers : étape 1 pour les déplacements, sous-étapes 3 pour les noms de tests et de fixtures.
- Les gardes anti-cart et de portée : branchées en 2b et 3 ; cette étape fournit seulement l'extraction qu'elles pourront lire.
- Le code Lua et `pico8/lua.lua`.
- Lancer l'appli raylib : jamais ; `make raylib` compile et linke seulement.

## Hypothèses sur l'étape 1 (pas encore faite)

Chaque hypothèse est à confronter à la note « Découvert » de l'étape 1 et au `Makefile` qu'elle laisse, avant de lancer cette PRD.

- **H-1-1** — Les fichiers de `src/` sont dans `src/{p8,engine,game,platform,devtools,cmd}/`, sous les noms cibles de l'arborescence de `docs/vocabulaire.md` ; `tests/` reste à plat avec ses fichiers renommés ; `tools/` reste à plat avec ses renommages (`probe.p8` → `pico8_reference_probe.p8`…).
- **H-1-2** — Tous les includes du projet s'écrivent `#include "dir/fichier.h"` depuis `src/`, y compris dans `tests/` (plus de `"../src/…"`) et dans les en-têtes entre eux.
- **H-1-3** — `-Isrc` est le seul chemin d'en-têtes du projet et se trouve dans une variable du `Makefile` partagée par toutes les règles de compilation (`CFLAGS` ou une variable de préprocesseur dédiée) ; les jeux de flags par arbre d'objets (`COVERAGE_CFLAGS`, `ASAN_CFLAGS`, `UBSAN_CFLAGS`, `GCOV_CFLAGS`, `SANITIZE_OPT_CFLAGS`) gardent leur nom et leur contenu.
- **H-1-4** — L'étape 1 ne produit pas de `compile_commands.json` ; l'outil reçoit donc ses flags par la cible `make` qui le lance (décision « Configurations »).
- **H-1-5** — Les unités raylib (`platform/raylib_backend*`, `cmd/game.c`, `cmd/screencast_viewer.c`) demandent toujours `-I third_party/raylib/src` en plus, et `third_party/` reste gitignoré et absent tant que `raylib-vendor-setup` n'a pas tourné.
- **H-1-6** — Le nombre d'unités ne change pas : 87 `.c` dans `src/` (sous-répertoires compris), 91 `.c` dans `tests/`, 1 dans `tools/` (`gen_quarter_wave.c`), soit 179 ; et 84 `.h` dans `src/`.
- **H-1-7** — Aucun symbole ne change à l'étape 1, gardes d'inclusion comprises (`PICO8_CHEAT_H` reste) : les noms cités dans cette PRD (`pico8_cheat_t`, `pico8_cheat_init`, `pico8_cheat_step`, `BTN_O`, `BTN_X`, `SEQUENCE_LENGTH`, `sequence`, `apply`, `feed`, `prev_o`, `prev_x`, `matched`, `PICO8_FIXED_LIT`, `POISON`, `PICO8_NORETURN`) existent tels quels.
- **H-1-8** — Le module de preuve `cheat` est `src/game/cheat.{c,h}`, toujours utilisé par les pilotes de rejeu, de couverture de classes et interactif (`cmd/replay.c`, `cmd/class_coverage.c`, `cmd/game.c`) et par trois tests (aujourd'hui `test_pico8_hy.c`, `test_pico8_fm.c`, `test_pico8_places_vivantes.c`, renommés à l'étape 1).
- **H-1-9** — Les conditionnelles de compilation sont inchangées : `PICO8_CLASS_COVERAGE` (31 `#ifdef` dans 21 fichiers), `PICO8_ZONE_ASAN` / `__has_feature(address_sanitizer)`, `__GNUC__` ; un seul `##`, dans `PICO8_FIXED_LIT`.
- **H-1-10** — Les cibles `make test`, `make e2e`, `make test-all`, `raylib-vendor-setup`, `raylib`, `screencast-viewer`, `class-coverage-net`, `e2e-gap` gardent leur nom et leur rôle ; les `*.sha256` de `tests/e2e/` sont ceux qu'a laissés l'étape 1.
- **H-1-11** — Les références qui contiennent des chemins (`tools/line_coverage_reference.txt`, en `fichier:ligne`) ont été mises à jour par l'étape 1. Un renommage dans une ligne ne change aucun numéro de ligne : la preuve ne devrait pas les toucher ; si `e2e-gap` bouge, c'est un défaut de l'outil.

**Sections à revérifier quand la note « Découvert » de l'étape 1 existe** : « Configurations et unités de compilation » (H-1-3 à H-1-6, H-1-9), « Interface » (colonne `fichier` de la table, H-1-1), « Testing Decisions » — module de preuve et ses utilisateurs (H-1-7, H-1-8), invariant et cibles hors suite (H-1-10, H-1-11) —, « Implementation Chunks » (chunk 4), « Cas des macros » (H-1-9), et cette section elle-même.

## Further Notes

- **Sondes faites pendant la rédaction** (HEAD `361579b`, avant l'étape 1 ; à refaire sur l'arbre d'après l'étape 1, la note « Découvert » fait foi) :
  - Conteneur : clang 18.1.3, `clang-rename-18`, `clang-apply-replacements-18`, `clang-query-18`, `libclang-18.so.1` présents ; Python 3.11 ; les bindings `clang` ne sont pas installés, `clang==18.1.8` se télécharge par le proxy.
  - `-Xclang -ast-dump=json` marche (≈1 Mo par unité). Ses positions sont **à encodage différentiel** : `offset` (octets) est toujours présent, `file` et `line` seulement quand ils changent par rapport à la position précédemment écrite dans l'ordre du document ; un lecteur doit donc suivre cet état sur tout le flux. Les expressions n'ont pas de `loc`, seulement `range` ; pour un `MemberExpr`, le nom du champ est en fin de `range`. Une position dans une macro se dédouble en `spellingLoc` / `expansionLoc` (`isMacroArgExpansion` pour un argument), et le JSON donne bien la position dans le corps du `#define`. Il n'a ni `TypeLoc`, ni désignateurs, ni macros.
  - libclang : `TYPE_REF`, `MEMBER_REF` de désignateur, `MACRO_DEFINITION` / `MACRO_INSTANTIATION` et USR présents ; position par défaut = point d'expansion (argument d'`assert` rendu sur `assert`) ; `clang_getSpellingLocation` corrige les arguments de macro mais pas les corps de `#define` (rendus au point d'expansion). Parcours de 30 unités en ≈1 s.
  - `-dump-raw-tokens` donne chaque token d'un fichier avec ligne et colonne, branches inactives comprises : utile pour la détection de `##` et pour lister les orthographes dans une unité non analysée.
- **Pas d'humain disponible pour les points « vérifier avec l'utilisateur » du skill `to-prd`** : je n'avais pas d'outil pour lancer un agent neuf ; j'ai chargé le skill `guideline` moi-même et tranché selon lui. Choix tranchés : moteur libclang plutôt que JSON seul ou `clang-rename` (le seul qui couvre types, désignateurs et macros sans moteur de secours) ; bindings installés par une cible `make` hors suite plutôt que vendus dans le dépôt (moins de code commité, au prix d'un accès réseau une fois) ; tests de l'outil hors `make test` ; pas de traitement de `##`, seulement sa détection (un seul usage, sur un littéral).
- Pour 2b et 5 : si la garde anti-cart doit entrer dans `make test` (étape 5) et lire l'AST, elle héritera de la dépendance aux bindings ; l'étape 5 tranchera entre l'installer dans l'environnement de test ou lire le JSON de clang, sans dépendance, pour les seuls noms courts.
