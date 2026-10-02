---
title: "Plan 5, sous-étape 4b — commentaires, chaînes et locales du runtime PICO-8"
description: "Ramener à zéro les trois compteurs de l'étape 4 (français, anti-cart backtick/comment, citations du cart) et la liste des locales cryptiques sur les modules, tests et données de tools/ de 3b, sans changer un pixel ni une valeur de constants-check."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 4. **Commentaires, asserts, locales** — une PRD par sous-étape, sur la même liste qu'à l'étape 3. La note « Découvert » de la première sous-étape corrige la consigne des agents avant les suivantes. Un agent par module : commentaires et messages d'`assert` réécrits en anglais sans citer le cart (§4), mots français restants retirés, locales cryptiques à longue portée renommées. Le skill `reprendre-les-commentaires` cadre le travail. Un troisième compteur, de **français résiduel** (commentaires, chaînes, asserts), est branché au début de l'étape. Fin d'une sous-étape : les trois compteurs à zéro sur ses modules, toutes catégories, mentions entre backticks comprises.
>
>    **Sous-étapes** : les modules, tests et données de `tools/` de la sous-étape 3 de même lettre ; l'étape 3 finie, aucun résidu d'une sous-étape suivante ne reste.
>
>    **Méthode de l'étape 4** (consultation `plan-review` de fin d'étape 3) :
>    - *Trois sortes de commit, trois preuves.* Commentaires seuls : sortie de `gcc -fpreprocessed -dD -E -P` identique avant et après, nombre de lignes de code inchangé, ni essai à blanc ni TSV. Chaînes et messages d'`assert` : la rigueur de l'étape 3 (essai à blanc, test rouge avant la correction, tampons vérifiés). Locales renommées : par `rename-slice`/`rename-apply` seulement, ajoutées au TSV et aux surcharges.
>    - *Propriété des chaînes.* Une chaîne française ou qui cite le cart, lue par un test ou par la sortie d'un binaire (`expect_fatal`, `must_abort_saying`, noms de zone, sortie `REPLAY_FILL`), est traduite par la sous-étape qui tient son site écrivain, à tous ses sites d'un coup ; le commit de chaînes liste d'abord ses sites lecteurs hors de ses modules.
>    - *Extraction fraîche* (corrigée par 4a). Chaque 4x finit par `rename-extract` frais sans `warning:` (les 4 lignes `partial` de `class_coverage` attendues), `rename-rekey-check SUBSTEP=3l` vert, puis les compteurs. `rename-rekey-check` sur les autres sous-étapes est rouge par construction depuis l'étape 3 (chaque ligne appliquée a perdu son ancien nom) : il n'est pas une porte. `rename-rekey` ne sert qu'à une 4x qui ajoute des lignes de locale, écrites depuis une extraction de l'arbre courant et appliquées dans le même commit, puisque les éditions de commentaires déplacent les lignes et que `rename-rekey` apparie par ligne. Les outils reçoivent la sous-étape 4x sous le nom `SUBSTEP=3x` ; un morceau (4c1…) sous `MODULES=` et sa liste.
>    - *Locales cryptiques.* 4a définit une liste candidate mesurable tirée de `declarations.tsv` (locale ou paramètre de portée > 40 lignes, nom de trois caractères ou moins ou non découpé en mots) ; chaque sous-étape la relève avant et après. Pour les 12 statiques locales proposées en exception, l'étape 4 propose aussi un nom dans « Découvert », pour que l'un ou l'autre verdict s'applique mécaniquement à l'étape 5.
>    - *Compteur de français.* Construit par 4a, d'abord, test d'abord : liste de mots-outils français choisis plus détection des accents (le code est en ASCII sans accents) ; fichier d'exceptions au format de `scope_exceptions.txt` ; mêmes colonnes et même scission fait / suivantes que `cart-names-count` ; jamais bloquant ; fixtures de détection, d'homographes (`place`, `file`, `limite`) et d'exception. Sa mesure de départ par sous-étape va dans la note de 4a ; une sous-étape hors norme est coupée, comme 3l.
>    - *Consigne des agents.* La PRD de 4a porte une version étape 4 de `consigne-agent.md` de `reprendre-les-commentaires`, qui lève point par point ses contradictions avec l'étape (code non modifié, assertions intouchables, chaîne humaine signalée sans correction, sortie du préprocesseur identique, destination glossaire) ; la note de 4a l'amende ensuite.
>    - *Citations du cart.* Relevé non bloquant par sous-étape, avant et après : commentaires qui citent `cart` (hors module `game/cart`), `Lua`, `lua.lua`, et noms du TSV d'une ou deux lettres employés nus.
>    - *Registre des décisions humaines.* `docs/decisions-plan-5.md` : une entrée numérotée par question (origine, question, options, nombre de sites, proposition prête à appliquer), ouverte par 4a à partir des notes de 3a–3l, complétée par chaque 4x, close par 4l. Une décision de nommage précise qu'elle s'applique aussi aux mentions entre backticks et en commentaire, avec un contrôle sur le nom remplacé.
>
> 3b. **Runtime PICO-8.** `p8/` : `fixed`, `quarter_wave`, `mem`, `gfx`, `palette`, `str`, `input`, `audio`, `assets`, `fatal` ; `tools/gen_quarter_wave.c` et `tools/constants_compare.sh` suivent. `pico8_input_load/save/append/free` et `pico8_fatal`, définis hors de `p8/`, passent en 3k et 3l. Tests : `test_fixed`, `test_mem`, `test_gfx`, `test_demo`, `test_str`, `test_input`, `test_asset_loading`. Résidu ≤ 100.
>
> 4b. **Commentaires — runtime PICO-8.** Modules, tests et données de `tools/` de 3b.

La note « Découvert » de 4a n'écrit aucune ligne propre à 4b (elle coupe 3c, 3f, 3k et 3l ; 3b, charge 329, reste entière).

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4). Modules de 3b (`tools/rename_substeps.tsv`, ligne `3b`) : `p8/fixed`, `p8/quarter_wave`, `p8/mem`, `p8/gfx`, `p8/palette`, `p8/str`, `p8/input`, `p8/audio`, `p8/assets`, `p8/fatal`, `tools/gen_quarter_wave`, `tools/constants_compare`, `tests/test_fixed`, `tests/test_mem`, `tests/test_gfx`, `tests/test_demo`, `tests/test_str`, `tests/test_input`, `tests/test_asset_loading`, et les fixtures `tests/fixtures/demo_events`, `events_malformed_*` (4), `events_valid`, `events_valid_ox`.

Entrées lues : `docs/plan-5.md` §1, §2, §4, §5, l'entrée 4 et sa méthode, les notes « Découvert » de 3b, de 3l (fin d'étape 3) et de 4a ; `docs/consigne-etape-4.md` ; `docs/decisions-plan-5.md` ; le skill `reprendre-les-commentaires` ; `git show c80a9ef` (chaînes de 4a) et `73eed21` (commentaires de 4a), modèle de méthode de cette PRD.

Aucun humain n'était disponible : l'agent rédacteur a chargé lui-même `guideline` aux points « vérifier avec l'utilisateur » de `to-prd`. Les décisions reposent sur le plan et sur des mesures en lecture seule (écritures dans `tmp/` et le scratchpad seulement).

**Condition d'entrée** : `origin/plan-5-noms-anglais` à `4922dda` ou un descendant, arbre propre, `make test-all` vert ; `make rename-extract` frais sans `warning:` (les 4 lignes `partial` de `class_coverage` attendues) ; `rename-rekey-check SUBSTEP=3l` vert ; la mesure de départ ci-dessous retrouvée.

## Problem Statement

Le runtime PICO-8 émulé (`p8/`) porte encore, après l'étape 3, des commentaires en français (`p8/fatal.h`, `p8/palette.h`), des renvois à des fichiers et fonctions disparus (`pico8_gfx.c`, `pico8_fixed_div`, `PICO8_ADDR_SPRITE_MAP_SHARED`, `pico8_fatal_pc.c`…), des citations du cart et de Lua, et deux locales de longue portée au nom court dans `test_demo.c`. Son outil `tools/constants_compare.sh` est entièrement en français, y compris la table qu'il écrit (`ECART`, `litteraux … | ecart …`), que lisent un autre script et un test. Un lecteur anglophone ne peut pas s'y fier, et les compteurs de l'étape 4 ne sont pas à zéro sur 3b.

## Solution

Appliquer à 3b la méthode prouvée par 4a (note « Découvert » de 4a, commits `c80a9ef` puis `73eed21`), en un seul morceau et trois commits de code — chaînes (rouge d'abord), locales (`rename-apply`), commentaires (préprocesseur identique) — puis la note « Découvert » de 4b. À la fin, sur `SUBSTEP=3b` : français, anti-cart (`backtick`, `comment`), citations et locales cryptiques à zéro, `done` comme `later`.

## User Stories

1. En tant que lecteur du runtime, je veux des commentaires de `p8/` en anglais, afin de comprendre le contrat d'une primitive sans traduire.
2. En tant que lecteur, je veux que chaque renvoi nomme un fichier ou une fonction qui existe (`p8/gfx.c`, `p8_fixed_div`, `platform/fatal_pc.c`), afin de suivre le renvoi par `grep`.
3. En tant que mainteneur, je veux qu'aucun commentaire ne cite le cart, Lua ou `lua.lua`, afin que le C reste la seule référence (§2).
4. En tant que mainteneur, je veux que les références à PICO-8 la console (`tools/pico8_reference_probe_out.txt`, le manuel lexaloffle, `pico8/gfx.json`) restent, afin de ne pas confondre la machine émulée avec le cart.
5. En tant que mainteneur, je veux que les étiquettes `/* TROU-N */` de 3b (13 : TROU-1, 2, 10, 17, 22) restent telles quelles, afin que l'étape 5 les renomme d'un coup.
6. En tant qu'utilisateur de `make constants-check`, je veux une table et des messages en anglais, afin de lire un écart sans dictionnaire.
7. En tant que mainteneur de `tools/constants_check.sh` et de `test_constants_check`, je veux que leurs lectures de la table suivent dans le même commit, afin que `make test` reste vert.
8. En tant que relecteur, je veux voir le commit de chaînes rougir avant ses lecteurs, afin de savoir que le test lit vraiment le texte changé.
9. En tant que relecteur, je veux la sortie de `constants-check` d'après égale à celle d'avant passée par la table de traduction, afin de savoir que seules des étiquettes ont changé.
10. En tant que lecteur de `test_demo.c`, je veux que ses locales `mem` et `gfx` de plus de cent lignes portent un nom complet, afin de respecter le §1.
11. En tant que mainteneur du TSV, je veux que ces deux renommages y figurent comme appliqués, afin que le TSV reste ce qui a été appliqué (§6).
12. En tant que relecteur, je veux un commit de commentaires dont la sortie de `gcc -fpreprocessed -dD -E -P` est identique à l'octet, afin de le relire sans relire le code.
13. En tant que relecteur d'un script, je veux les lignes hors commentaire de `constants_compare.sh` identiques dans le commit de commentaires, afin de la même garantie.
14. En tant que mainteneur, je veux que `src/p8/quarter_wave.h` reste reproduit à l'octet par `make sin-table`, afin que la table générée ne dérive pas de son générateur.
15. En tant que lecteur, je veux que chaque affirmation traduite ait été revérifiée dans le code, afin de ne pas traduire une erreur (cf. `palette.h`, ci-dessous).
16. En tant qu'humain du point d'arrêt, je veux que les questions ouvertes de 4b arrivent au registre, prêtes à appliquer, afin de trancher sans relire 4b.
17. En tant qu'exécutant de 4c1, je veux la note « Découvert » de 4b, afin d'hériter des pièges rencontrés.

## Implementation Decisions

### Mesure de départ (4922dda, extraction fraîche, écrits dans `tmp/` seulement)

| Compteur (`SUBSTEP=3b`) | `done` | `later` |
|---|---|---|
| `french-count` (id/backtick/string/comment) | 0/0/15/130 | 0/0/1/0 |
| `cart-names-count` (id/backtick/string/comment) | 0/4/0/31 | 0/7/0/10 |
| `cart-citations` (cart/lua/short) | 27/6/96 | — |
| `cryptic-locals` | 2 | — |
| `scope-count` | 0 | — |

Identique à la ligne 3b de la mesure de 4a (français 145+1, anti-cart 52, citations 27/6/96, 2 locales). Détail :
- *Français* : commentaires de `p8/fatal.h` (13 mots), `p8/palette.h` (74), `tools/constants_compare.sh` (43) ; chaînes, toutes dans `constants_compare.sh` (15 `done`, 1 `later` : `de` ligne 27).
- *Anti-cart* : surtout des noms nus `pico8_*` en commentaire ; les 7 `backtick` `later` sont dans `p8/input.h` (4) et `p8/str.h` (3) — des noms de 3k, cités dans un fichier de 3b, donc à 4b. Plus large que le compteur : `grep -o 'pico8_[a-z_]*\|PICO8_[A-Z_]*'` sur les fichiers de 3b, hors `quarter_wave.h`, `pico8_reference_probe*` et `pico8_manual`, rend 54 mentions (noms de fichier `pico8_X.c` et macros `PICO8_*` que le TSV ne liste pas comme identifiants). Le critère de fin y ajoute ce `grep` à 0.
- *Citations* : `cart` dans `p8/gfx.[ch]`, `p8/fixed.[ch]`, `test_gfx`, `test_asset_loading`, `test_demo`, `test_fixed` ; `Lua` dans `audio.h`, `fixed.h`, `palette.h`, `str.h`, `test_fixed.c` ; `short` : surtout des mots français (`le`, `et`, `du`, `ou`, `en`) et des noms de paramètres en prose (`lo`/`hi`/`v` de `fixed.h`, `s`/`i`/`j` de `str.h`, `b`, `n`, `w`/`h`, `dx`/`dy`). Consigne amendée par 4a : on réécrit la phrase (« the start index », « the low bound »), pas d'exception.
- *Locales cryptiques* : `tests/test_demo` `mem` (portée 115 lignes) et `gfx` (112), locales de `main`.
- *Lignes de commentaire* (non vides moins préprocesseur), départ : `fixed.c` 37, `fixed.h` 50, `gfx.c` 46, `gfx.h` 17, `input.h` 45, `palette.h` 21, `str.h` 14, `test_fixed.c` 86, `test_gfx.c` 84, `test_input.c` 25, `test_demo.c` 14, `test_asset_loading.c` 12, les autres ≤ 6 ; `input.c` et `palette.c` 0. `quarter_wave.h` exclu (généré).

### Commit 1 — chaînes (`refactor:`), rouge d'abord

Règle de propriété : `tools/constants_compare.sh` est l'écrivain ; 4b traduit ses textes à tous leurs sites d'un coup. **Sites lecteurs hors de 3b, en tête du message** (modules de 4l4) :
- `tests/test_constants_check.c` lignes 72, 73, 79, 129, 144, 145, 155, 156, 203 (ligne de table et ligne des comptes), 188 (`"aucune constante"`), 195 (`"aucune source"`) ;
- `tools/constants_check.sh` ligne 31 (`status[lit] == "ECART"`). Ses propres messages (`les ECART ne sont pas ceux de…`, `defaillances`, `regression`) et ses commentaires restent à 4l4 ; la note signale que son message cite un mot qui n'existe plus.

Éditions, décidées ici :
- vocabulaire de la table : `ECART` → `GAP`, `litteraux N | sites N | ok N | ecart N` → `literals N | sites N | ok N | gaps N` (`GAP` comme le fichier `constants_expected_gaps.txt` qui les liste) ;
- messages `fail` et `printf` d'erreur en anglais (`no source`, `no constant in the sources`, `P8_FIXED_LIT call outside the strict form`, `block A (## a literals) not found in …`, `literal missing from block A of …`, `generated program … does not compile`…), préfixe `constants_compare:` inchangé ;
- noms français du script : variable `releve` → `reference`, fichiers de travail `releve.txt`/`valeurs.txt` → `reference.txt`/`values.txt`, ligne `usage:` du commentaire laissée au commit 3 ;
- `tests/test_demo.c` : libellé `pico8 demo:` → `test_demo:` (laissé par 3l à l'étape 4 ; `grep` : aucun lecteur).
- Aucun message d'`assert`, aucun `p8_fatal` ni `printf` du runtime n'est français ou ne cite le cart (vérifié : `p8/` n'écrit aucune chaîne lue par un test). Les `expect_fatal` en français (`"zone … pleine, limite"`) ont leur écrivain dans `engine/zone` : 4c1.

Preuves :
- **rouge d'abord** : `constants_compare.sh` traduit seul fait échouer `test_constants_check` (ligne 72 la première) ; lecteurs traduits, vert ;
- **`make constants-check`** : sortie d'après égale à celle d'avant (`litteraux 34 | sites 61 | ok 34 | ecart 0`) passée par le `sed` de la table de traduction, écrit dans le message ;
- **tampons** : aucun (`strstr` sur un fichier, `awk`/`printf` en `%s`) ; dit dans le message.

### Commit 2 — locales (`refactor:`)

`mem` → `memory`, `gfx` → `graphics` dans `main` de `tests/test_demo.c` (comme leurs types `p8_memory_t`, `p8_graphics_t` ; aucun homonyme dans le fichier). Le paramètre `mem` de `screen_pixel` (portée 3 lignes) reste.

- `make rename-extract` frais **après le commit 1** ; identités relues dans `tmp/ast_rename/declarations.tsv` (aujourd'hui `c:test_demo.c@379@F@main@mem` et `…@424@F@main@gfx` ; le TSV porte encore `@388`/`@436`, d'avant 3b : à réécrire depuis l'extraction courante).
- Dans le même commit : les deux lignes `conservé` de `docs/renommage.tsv` passent `renommé` avec le nom neuf et l'identité courante ; deux lignes `identity` dans `tools/rename_overrides.tsv` (pas `field main.mem` : `main` est une identité partagée par tous les binaires) ; table à deux lignes appliquée par `make rename-apply` (procédure N du registre).
- Preuve : build vert, `make rename-table-check` et `rename-derive-test` verts, `cryptic-locals SUBSTEP=3b` à 0.

### Commit 3 — commentaires (`docs:`), consigne étape 4

« Fichier d'avant » : l'état d'après les commits 1 et 2, copié sous `tmp/` avant de lancer les agents. Un agent par module de 3b qui a des commentaires (en-tête et `.c` d'un module ensemble ; chaque test avec son module ; `gen_quarter_wave.c` et `constants_compare.sh` repris par l'orchestrateur), consigne `docs/consigne-etape-4.md`, rapport au format amendé. Sans agents, l'exécutant applique la consigne lui-même et remplit le rapport.

Preuves : sortie de `gcc -fpreprocessed -dD -E -P` identique à l'octet, fichier par fichier ; lignes de code inchangées ; ratio avant/après dans le message ; pour `constants_compare.sh`, `diff` des lignes hors `#` vide ; `make sin-table` puis `git diff --exit-code src/p8/quarter_wave.h`.

Points connus, sans préjuger du rapport :
- *Affirmation fausse à corriger* : `palette.h` dit la palette partagée par « les deux programmes raylib » ; `devtools/gif.c` et `tests/test_gif.c` l'incluent aussi.
- *Renvois périmés* : `pico8_fatal_pc.c` → `platform/fatal_pc.c` (sans trancher D-25) ; `pico8_gfx.c`, `pico8_assets.c`, `pico8_dump.c`, `pico8_fixed.h` ; `pico8_fixed_div`/`_neg`/`_sqrt`/`_sin`/`_from_double`, `pico8_poke`/`peek`/`memcpy`, `pico8_palt`, `pico8_pset`, `pico8_fillp*`, `pico8_rk`, `pico8_qz_call`, `pico8_jo_kj`, `pico8_input_*`, `pico8_sub` ; `PICO8_FIXED_LIT` (`constants_compare.sh` lignes 3 et 12), `PICO8_ADDR_SPRITE_MAP_SHARED`, `PICO8_PALETTE_RGB`. Chacun vérifié par `grep` avant d'être remplacé.
- *Cart et Lua* : « the cart's one call site », « on the real cart », « Lua's `a..b` », « lua: » dans le format de `fixed.h` ; « the cart does X » → « X ». « Lua » qui désigne la sémantique PICO-8 se dit « PICO-8 ».
- *Commentaire de format de `fixed.h` ligne 18* (`raw: … ; lua: …`) : décrit les deux colonnes du bloc A de la sonde ; la colonne se renomme dans la prose, pas dans le fichier relevé.
- `tests/test_fixed.c` « copied from the probe, never… » : les citations des blocs de `tools/pico8_reference_probe_out.txt` restent (référence PICO-8, pas le cart).
- En-tête de `constants_compare.sh` (usage, format de table, codes de retour) réécrit en anglais avec le vocabulaire du commit 1.

### Ce qui reste hors de 4b

- `tools/pico8_reference_probe.p8` et `pico8_reference_probe_out.txt` sont rangés en 3l (4l3), non déplacés. Pour 4l3, constaté ici : les 39 mots français de la sonde sont des commentaires Lua (`--`) ; ses chaînes imprimées forment le fichier relevé, mesure d'un vrai PICO-8 qu'on ne régénère pas, donc intouchables ; les 8 `string` français du relevé sont `ce` dans des hexas (`0x0000.ce4f`), faux positifs, exception à proposer par 4l3.
- D-29 (41 locales et paramètres courts de 3b au nom d'un digramme), D-25, D-9, D-10 : non tranchés, non appliqués.
- Les messages et commentaires propres de `constants_check.sh` et de `test_constants_check.c` (fichier `releve.txt` du test, commentaire d'en-tête) : 4l4.

### Fin de PRD

Ordre : `make rename-extract` frais sans `warning:` (4 `partial`) ; `rename-rekey-check SUBSTEP=3l` vert ; compteurs sur `SUBSTEP=3b`.

**Critère de fin**, `done` et `later` :
- `cart-names-count` 0 dans les quatre colonnes ;
- `french-count` 0 dans les quatre colonnes, hors exceptions proposées en attente ;
- `cart-citations` 0/0/0 ;
- `cryptic-locals` 0, `scope-count` 0 ;
- `grep` `pico8_`/`PICO8_` ci-dessus à 0 (restent `pico8_reference_probe*`, `pico8_manual.txt`, `pico8/*.json`, `quarter_wave.h` exclu).

**Invariant (§5)** : `make test-all` vert, `tests/e2e/*.sha256` inchangés ; `make raylib` et `make screencast-viewer` compilent et linkent, jamais lancés ; `rename-tool-test` et les `rename-*-test` verts ; `class-coverage-net` et `e2e-gap` rouges, sorties égales à l'octet à celles de `4922dda` ; `data-residue` et `composition-residue` à 0, `--detail` et `--champs` identiques ; `make sin-table` reproduit `quarter_wave.h` à l'octet.

### Note « Découvert » de 4b (sous l'entrée 4b de `docs/plan-5.md`)

Elle porte : mesure de départ et de fin ; sites lecteurs et preuve rouge du commit 1 ; table de traduction de `constants-check` ; identités et lignes TSV/surcharges du commit 2 ; ratio et affirmations corrigées du commit 3 ; ce qui est laissé à 4c1 et 4l3/4l4 ; tout amendement de `docs/consigne-etape-4.md` tiré des rapports ; les entrées ajoutées au registre (aucune prévue ; une exception de français ou de citation ne s'ajoute que si un faux positif résiste à la réécriture, au format de D-33).

## Implementation Chunks

Un seul morceau (charge 329 ≤ 1 382), quatre commits dans cet ordre, `make test-all` vert après chacun :
1. chaînes (`refactor:`), rouge d'abord ;
2. locales (`refactor:`), extraction fraîche, TSV et surcharges dans le commit ;
3. commentaires (`docs:`), préprocesseur identique ;
4. note « Découvert », amendements de la consigne et du registre (`docs:`) — peut se fondre dans le commit 3, comme à 4a.

## Testing Decisions

- Aucun test neuf : l'étape 4 n'ajoute ni assertion ni condition (consigne, points 5 et 7). Le filet est `test_constants_check` (lecteur des chaînes, prouvé par le rouge d'abord), `make constants-check`, le compilateur pour les locales, et la sortie du préprocesseur pour les commentaires.
- Un bon contrôle juge une sortie observable : table de `constants-check` par traduction, `quarter_wave.h` à l'octet, `*.sha256` des e2e.
- Prior art : `c80a9ef` (rouge d'abord, `sed` de traduction d'une sortie) et `73eed21` (preuve du préprocesseur fichier par fichier).

## Out of Scope

- Toute ligne de code hors des chaînes du commit 1 et des deux locales du commit 2.
- La sonde PICO-8 et son relevé (4l3), les messages propres de `constants_check.sh` (4l4), les messages de zone (4c1).
- `TROU-N` → `GAP-N` (étape 5) ; toute entrée du registre sans verdict.
- La scission de `p8/input.h` et l'emplacement de `p8_fatal` (D-25, §7).

## Further Notes

- `GAP` pour une constante en écart et `GAP-N` (futur nom des `TROU-N`) se côtoient dans `constants_expected_gaps.txt`, qui commence par `# TROU-1` : même sens (écart connu au cart), pas de conflit.
- Les identités de locales portent un offset : toute édition de `test_demo.c` avant le commit 2 les déplace, d'où l'extraction juste avant lui.
