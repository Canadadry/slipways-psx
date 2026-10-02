---
title: "Plan 5, sous-étape 4a — commentaires de l'en-tête commun et de la vtable, outillage de l'étape 4"
description: "Première sous-étape de l'étape 4 : poser une seule fois l'outillage de l'étape (compteur de français test d'abord, liste des locales cryptiques, relevé des citations du cart, consigne étape 4, registre des décisions humaines), puis ramener à zéro les trois compteurs sur engine/entity_class, ses deux tests et ses données de tools/, sans changer un pixel."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 4. **Commentaires, asserts, locales** — une PRD par sous-étape, sur la même liste qu'à l'étape 3. La note « Découvert » de la première sous-étape corrige la consigne des agents avant les suivantes. Un agent par module : commentaires et messages d'`assert` réécrits en anglais sans citer le cart (§4), mots français restants retirés, locales cryptiques à longue portée renommées. Le skill `reprendre-les-commentaires` cadre le travail. Un troisième compteur, de **français résiduel** (commentaires, chaînes, asserts), est branché au début de l'étape. Fin d'une sous-étape : les trois compteurs à zéro sur ses modules, toutes catégories, mentions entre backticks comprises.
>
> **Sous-étapes** : les modules, tests et données de `tools/` de la sous-étape 3 de même lettre ; l'étape 3 finie, aucun résidu d'une sous-étape suivante ne reste.
>
> **Méthode de l'étape 4** (consultation `plan-review` de fin d'étape 3) :
> - *Trois sortes de commit, trois preuves.* Commentaires seuls : sortie de `gcc -fpreprocessed -dD -E -P` identique avant et après, nombre de lignes de code inchangé, ni essai à blanc ni TSV. Chaînes et messages d'`assert` : la rigueur de l'étape 3 (essai à blanc, test rouge avant la correction, tampons vérifiés). Locales renommées : par `rename-slice`/`rename-apply` seulement, ajoutées au TSV et aux surcharges.
> - *Propriété des chaînes.* Une chaîne française ou qui cite le cart, lue par un test ou par la sortie d'un binaire (`expect_fatal`, `must_abort_saying`, noms de zone, sortie `REPLAY_FILL`), est traduite par la sous-étape qui tient son site écrivain, à tous ses sites d'un coup ; le commit de chaînes liste d'abord ses sites lecteurs hors de ses modules.
> - *Extraction fraîche.* Chaque 4x finit par `rename-extract`, `rename-rekey`, `rename-rekey-check` sur toutes les sous-étapes, puis les compteurs. Les outils reçoivent la sous-étape 4x sous le nom `SUBSTEP=3x`.
> - *Locales cryptiques.* 4a définit une liste candidate mesurable tirée de `declarations.tsv` (locale ou paramètre de portée > 40 lignes, nom de trois caractères ou moins ou non découpé en mots) ; chaque sous-étape la relève avant et après. Pour les 12 statiques locales proposées en exception, l'étape 4 propose aussi un nom dans « Découvert », pour que l'un ou l'autre verdict s'applique mécaniquement à l'étape 5.
> - *Compteur de français.* Construit par 4a, d'abord, test d'abord : liste de mots-outils français choisis plus détection des accents (le code est en ASCII sans accents) ; fichier d'exceptions au format de `scope_exceptions.txt` ; mêmes colonnes et même scission fait / suivantes que `cart-names-count` ; jamais bloquant ; fixtures de détection, d'homographes (`place`, `file`, `limite`) et d'exception. Sa mesure de départ par sous-étape va dans la note de 4a ; une sous-étape hors norme est coupée, comme 3l.
> - *Consigne des agents.* La PRD de 4a porte une version étape 4 de `consigne-agent.md` de `reprendre-les-commentaires`, qui lève point par point ses contradictions avec l'étape (code non modifié, assertions intouchables, chaîne humaine signalée sans correction, sortie du préprocesseur identique, destination glossaire) ; la note de 4a l'amende ensuite.
> - *Citations du cart.* Relevé non bloquant par sous-étape, avant et après : commentaires qui citent `cart` (hors module `game/cart`), `Lua`, `lua.lua`, et noms du TSV d'une ou deux lettres employés nus.
> - *Registre des décisions humaines.* `docs/decisions-plan-5.md` : une entrée numérotée par question (origine, question, options, nombre de sites, proposition prête à appliquer), ouverte par 4a à partir des notes de 3a–3l, complétée par chaque 4x, close par 4l. Une décision de nommage précise qu'elle s'applique aussi aux mentions entre backticks et en commentaire, avec un contrôle sur le nom remplacé.
>
> 4a. **Commentaires — en-tête commun et vtable.** Modules et tests de 3a.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4), première sous-étape de l'étape 4. Modules de 3a (`tools/rename_substeps.tsv`) : `engine/entity_class` (un seul fichier, `entity_class.h`), `tests/test_entity_header`, `tests/test_entity_kind`, `tools/class_coverage_exempt`, `tools/class_coverage_net_expected` ; et, par décision de cette PRD (ci-dessous), `tools/class_coverage` (le script `class_coverage.sh`).

Entrées lues : `docs/plan-5.md` §1, §2, §4 (commentaires : jamais le cart, quatre destinations moins la branche « nom minifié », `TROU-N` jusqu'à l'étape 5), §5, l'entrée 3 et sa « Méthode des sous-étapes », toutes les notes « Découvert » (dont celle de 3a et le bilan de fin d'étape 3 en 3l) ; `CLAUDE.md` ; le skill `reprendre-les-commentaires` (`SKILL.md`, `consigne-agent.md`, `modele-rapport.md`, `exemple-travaille.md`) ; `tools/rename_count.py`, `tools/scope_count.py`, `tools/rename_substeps.py` et `.tsv`, `tools/rename_rekey.py`, les cibles du `Makefile`.

Aucun humain n'était disponible : l'agent rédacteur a chargé lui-même `guideline` aux points « vérifier avec l'utilisateur » de `to-prd`. Les décisions reposent sur le plan et sur des mesures en lecture seule (écritures dans `tmp/` et le scratchpad seulement), sans interview.

**Condition d'entrée** : `origin/plan-5-noms-anglais` à `c4c161c` ou un descendant, arbre propre, `make test-all` vert ; `make rename-extract` frais, sans `warning:` (les 4 lignes `partial` de `class_coverage` attendues) ; `make cart-names-count` sans `SUBSTEP` à `TOTAL` 0 `identifier` et 1 `string` (le repli de `tools/frames_ref.sh`), comme le constate la note de fin d'étape 3.

## Problem Statement

L'étape 3 a renommé chaque identifiant et chaque chaîne-clé, mais les commentaires et les messages d'`assert` parlent encore le cart et le français. Sur les fichiers de 3a, mesuré à `c4c161c` (`cart-names-count SUBSTEP=3a`, extraction fraîche) : **57 mentions** de noms du cart, 55 entre backticks et 2 nues (`done` 14/1, `later` 41/1). Ce que le compteur ne voit pas s'y ajoute : les noms ambigus nus (`me`, `t`, `ra`, `fx`…), « the cart », « Lua's truth », les renvois à des fichiers disparus (`pico8_file_sortie.h`, `pico8_slipway.h`), les noms périmés (`rec` pour `record`, `PICO8_TAG(t)` pour `TAG(t)`), le libellé imprimé `[cw]` de `test_entity_kind.c`, deux messages d'`assert` de `test_entity_header.c` (« a probe's fx is its archetype's 1.0, Lua's true: 0 would read as nil »), et du français dans les données de `tools/` : `class_coverage_exempt.txt` (« abstraite, jamais instanciée directement ») et `class_coverage_net_expected.txt` (« exemptée », « couverture: OK -- chaque classe non exemptée est instanciée »).

Et l'étape 4 n'a pas encore ses instruments. Sa méthode en demande quatre que rien ne fournit : un compteur de français, une liste mesurable de locales cryptiques, un relevé des citations du cart, une consigne d'agent qui ne contredise pas l'étape. Il manque aussi le registre où trancher, au point d'arrêt d'avant l'étape 5, les questions laissées ouvertes par 3a–3l.

Enfin, 40 fichiers que lisent les compteurs n'appartiennent à aucune sous-étape (`tools/*.sh` des gardes, `tools/class_coverage.sh`, `tools/scope_count.py`, fixtures JSON, données de `tools/`…) : sans place, aucune 4x ne les reprendrait.

## Solution

Trois morceaux, chacun clos par un `make test-all` vert et ses commits poussés :

1. **Outillage et registre**, une seule fois pour toute l'étape :
   - compteur de français `make french-count`, construit test d'abord ;
   - liste des locales cryptiques `make cryptic-locals` ;
   - relevé des citations du cart `make cart-citations` ;
   - table des sous-étapes complétée pour que tout fichier lu ait une place ;
   - consigne étape 4 dans `docs/consigne-etape-4.md`, complément au skill qui laisse le skill intact ;
   - registre `docs/decisions-plan-5.md`, ouvert depuis les notes de 3a–3l.

   Avant tout travail de module, les mesures de départ de 3a à 3l sont relevées.
2. **Travail des modules de 4a**, en trois sortes de commit et trois preuves :
   - chaînes et messages d'`assert` : `class_coverage.sh` et ses deux fichiers de données en anglais, lecteurs de `test_class_coverage.c` compris ; libellé `[cw]` ; messages d'`assert` de `test_entity_header.c` ;
   - commentaires seuls : `entity_class.h`, les deux tests, `class_coverage.sh` ;
   - locales : aucune, la liste candidate de 3a est vide (mesuré).

   Fin : extraction fraîche, puis les trois compteurs et le relevé des citations à zéro sur les modules de 3a, hors exceptions proposées en attente.
3. **Note « Découvert »** de 4a dans le plan, corrections du plan, amendement de la consigne.

Pas un pixel ne change, `tests/e2e/*.sha256` identiques.

## User Stories

1. En tant qu'agent d'une sous-étape 4x, je veux `make french-count SUBSTEP=3x` qui compte le français résiduel de mes fichiers en colonnes `identifier`, `backtick`, `string`, `comment`, scindé `done`/`later`, afin de savoir quand j'ai fini.
2. En tant qu'agent, je veux que le compteur de français ne compte pas `place`, `file` ni `limite` dans du code ou de l'anglais, afin qu'un homographe ne me fasse pas réécrire une phrase juste.
3. En tant qu'agent, je veux que tout mot accentué dans un commentaire, une chaîne ou un message compte, mais pas `×` ni `—`, afin que la règle « code en ASCII sans accents » soit mesurable.
4. En tant que relecteur humain, je veux un fichier d'exceptions au format de `tools/scope_exceptions.txt`, que seul un humain allonge, afin de trancher les faux positifs au point d'arrêt.
5. En tant qu'agent, je veux que le compteur de français ignore ses propres fichiers et les outils du dictionnaire, comme `cart-names-count`, afin que sa liste de mots ne se compte pas elle-même.
6. En tant qu'agent, je veux un avertissement quand un fichier lu n'appartient à aucune sous-étape, afin qu'aucun fichier ne sorte de l'étape 4 sans être repris.
7. En tant qu'agent, je veux `make cryptic-locals SUBSTEP=3x` : chaque locale ou paramètre de portée > 40 lignes au nom de trois caractères ou moins, ou fait d'un mot absent des noms neufs validés du TSV, afin de relever la liste avant et après mon passage.
8. En tant qu'agent, je veux `make cart-citations SUBSTEP=3x`, qui liste les commentaires et messages d'`assert` citant `cart` (hors `game/cart`), `Lua`, `lua.lua` ou un nom de deux lettres au plus du TSV employé nu, afin de voir ce que `cart-names-count` ne voit pas.
9. En tant qu'agent de module, je veux une consigne étape 4 qui dise ce qui change par rapport à `consigne-agent.md`, sans toucher au skill, afin de ne pas recevoir deux règles contraires.
10. En tant qu'utilisateur du skill hors de ce chantier, je veux que `reprendre-les-commentaires` reste tel quel, afin que son usage général ne porte pas de règle propre au plan 5.
11. En tant que relecteur humain, je veux `docs/decisions-plan-5.md`, une entrée numérotée par question avec origine, options, sites et proposition prête à appliquer, afin de trancher tout d'un coup avant l'étape 5.
12. En tant qu'exécutant de l'étape 5, je veux que chaque proposition s'applique mécaniquement (ligne de surcharge, ligne d'exception, édition nommée) avec son contrôle, afin de ne rien réinterpréter.
13. En tant que lecteur de `entity_class.h`, je veux des commentaires en anglais qui nomment les champs et les fonctions C (`state`, `state_time`, `entity_init_header`, `TAG(t)`, `record`), jamais leur nom Lua, afin de comprendre l'en-tête sans glossaire.
14. En tant que lecteur, je veux que les contrats écrits en vérité Lua (« nil », « Lua's true ») soient dits en C (NULL, 0, absent), afin de ne pas avoir à connaître Lua.
15. En tant que lecteur de tests, je veux que les commentaires gardent la raison des valeurs (hachage figé de `KIND_HASH`, remplissage de `malloc`), afin que l'assertion reste compréhensible.
16. En tant que mainteneur de la couverture de classes, je veux que `class_coverage.sh` et ses données parlent anglais, ses lecteurs dans `test_class_coverage.c` traduits dans le même commit, afin que `make test` et `class-coverage-net` gardent leur verdict.
17. En tant que relecteur, je veux que le commit de commentaires prouve par la sortie du préprocesseur qu'il ne touche aucune ligne de code, afin de le lire comme purement textuel.
18. En tant que relecteur, je veux que le commit de chaînes commence par lister ses sites lecteurs hors des modules de 4a et montre le test rouge d'abord, afin de prouver que les deux côtés ont bougé ensemble.
19. En tant que pilote des sous-étapes suivantes, je veux la mesure de départ des trois compteurs et du relevé pour 3a à 3l dans la note de 4a, avec la sous-étape hors norme désignée et coupée, afin de dimensionner 4b–4l.
20. En tant que pilote, je veux que la note de 4a corrige le plan là où l'étape 3 a rendu une règle de la méthode inapplicable (le `rename-rekey-check` « sur toutes les sous-étapes »), afin que 4b ne bute pas dessus.
21. En tant que joueur, je ne veux voir aucun pixel changer.

## Implementation Decisions

### Décisions de cette PRD, prises sans humain

- **Où vit la consigne étape 4 : `docs/consigne-etape-4.md`**, en français, en complément du skill, pas dedans. Le skill `reprendre-les-commentaires` sert hors du plan 5 (« rattrapage ponctuel ») : y verser des règles propres au chantier le casserait pour cet usage. Le fichier s'ouvre sur « base : `consigne-agent.md` et `modele-rapport.md` du skill, lus tels quels ; là où ils se contredisent, ce fichier gagne », puis ne liste que les points levés. Il vit tant que l'étape 4 reste à faire : 4l le supprime dans son dernier commit (règle de `docs/` du `CLAUDE.md`). Aucun fichier du skill n'est modifié.
- **`tools/class_coverage.sh` rejoint 3a.** C'est l'écrivain de presque tout le texte des deux fichiers de données de 3a : « exemptée », « couverture: OK -- … ». La règle de propriété des chaînes donne ces chaînes à la sous-étape de l'écrivain, et le script n'appartient à aucune. Le ranger en 3a permet à 4a de traduire écrivain et lecteurs d'un seul coup, sans résidu `later` sur ses propres données.
- **Tout fichier lu par les compteurs a une sous-étape.** Mesuré : 40 fichiers sans place. Ils sont ajoutés à `tools/rename_substeps.tsv` (en-tête du fichier : la table sert aux étapes 3 et 4, 4x = 3x) :
  - `tools/class_coverage`, `tools/scope_count`, `tests/test_scope_count` et les fichiers neufs de cette PRD en 3a ;
  - chaque fixture de `tests/fixtures/` dans la sous-étape du premier test qui la lit (`grep` ; ordre du plan) ;
  - tout le reste de `tools/` (scripts des gardes, données, installateurs, `ttf_to_pixelfont.py`) en 3l, que 4l reprend avec le `Makefile` (« scripts et données de `tools/` de 3l »).

  `rename-substeps-check` reste vert : il admet un module hors TSV, comme `tools/constants_compare` depuis 3a. Effet de bord attendu et relevé : `cart-names-count SUBSTEP=3a` et `SUBSTEP=3l` voient ces fichiers en plus.
- **Pas de nouvel `assert` à l'étape 4.** En ajouter un change la sortie du préprocesseur et ne relève d'aucune des trois sortes de commit. La destination 3 se réduit à ses 40 mots. Un piège qui mériterait un `assert` va dans la rubrique « envisagés » et, s'il est sérieux, dans le registre.
- **Paramètres et locales courts nommés d'un digramme du cart : pas renommés par 4a.** Les paramètres de pointeur de fonction de la vtable (`ra`, `qa`, `yr`, `kv`, `sw`, `lw`, `fy`, `s`, `p`, `a`) et le `me` d'`entity_state_of` sont `conservé` au TSV et tolérés par le §1 (portée ≤ 40). Ils portent le même nom dans toutes les méthodes du jeu. Les renommer dans le seul en-tête casserait cette cohérence ; les renommer partout est une question transverse, ouverte dans le registre (ci-dessous). Les commentaires de 4a décrivent le rôle (« the cursor position », « the year ») sans citer ces noms entre backticks : le compteur compterait `` `ra` `` même pour un paramètre conservé (angle mort noté en 2b).
- **`PICO8_MALLOC_FILLS`** (variable d'environnement de `test_entity_header.c`, 2 sites) attend son verdict (registre) : 4a ne la renomme pas. Aucun compteur ne la voit ; elle ne bloque pas la fin de 4a.

### Compteur de français (`tools/french_count.py`, `make french-count [MODULES=… | SUBSTEP=…] [SITES=1]`, `make french-count-test` dans `make test`)

- **Fichiers lus** : ceux de `cart-names-count`, même code (`rename_count.source_files`), mêmes exclusions (outils du dictionnaire, `tests/e2e/`, `third_party/`). En plus, le compteur exclut ses propres fichiers (script et test), qui contiennent la liste de mots par construction. Exclusion nommée dans la docstring et dans la note de 4a.
- **Régions** : celles de `rename_count`, réutilisées et non réécrites (`c_regions`, `text_regions`). Colonnes et classement :
  - `identifier` : une part d'identifiant du code C, découpé sur `_`, qui est un mot-outil ;
  - `backtick` : dans un `` `…` `` d'un commentaire ou d'un message d'`assert` ;
  - `comment` : ailleurs dans un commentaire ou un message d'`assert` ;
  - `string` : un littéral C, ou le texte d'un script, d'une donnée ou du `Makefile`.
- **Détection**, un compte par mot : un mot (lettres, accents compris) compte s'il est dans la liste des mots-outils, en minuscules, ou s'il contient une lettre latine accentuée (À–ÿ hors `×` et `÷`, plus `œ`, `æ`). La liste vit dans le script.
  - Elle ne prend que des mots-outils : articles, prépositions, conjonctions, pronoms, auxiliaires, adverbes de négation et de quantité.
  - Elle exclut tout mot qui est aussi un mot anglais courant (`on`, `a`, `as`, `or`, `son`, `plus`, `car`, `but`…) et tout mot de contenu (`place`, `file`, `limite`).
  - La liste exacte est relevée dans la note de 4a.
- **Angle mort assumé, écrit dans la docstring** : une phrase française sans mot-outil ni accent passe (« zone … pleine, limite 4 places » de `ui_zone.c`). La relecture de chaque 4x la cherche quand même ; un mot de contenu récurrent peut rejoindre la liste par une note.
- **Exceptions** : `tools/french_exceptions.txt`, lignes `colonne<TAB>module<TAB>mot`, `#` pour commentaire, livré avec son seul en-tête (« seule une décision humaine ajoute une ligne »). Une ligne retire exactement les mentions de ce mot, dans cette colonne et ce module.
- **Scission** (avec `SUBSTEP`) : fichiers de la sous-étape seulement, chaque mention `done` ou `later`, puis le pied `TOTAL done` / `TOTAL later`.
  - Une mention dans un littéral ou une ligne de donnée est `later` quand le texte entier de ce littéral, ou de cette ligne, se retrouve tel quel dans un littéral ou une ligne d'un fichier d'une sous-étape suivante. C'est la marque d'un écrivain probable ailleurs, que la règle de propriété des chaînes laisse à celui-ci.
  - Toute autre mention est `done`.
  - À l'étape 4, `done` doit valoir 0 ; chaque `later` se vérifie à la main (`SITES=1` liste les sites).
- **Garde-fous** : n'échoue jamais (code 0) ; avertit (`warning:`) pour chaque module lu qui n'est dans aucune sous-étape. Aucune extraction nécessaire : il lit les sources.

### Locales cryptiques (`tools/scope_count.py --cryptic --table docs/renommage.tsv`, `make cryptic-locals [MODULES=… | SUBSTEP=…]`)

- **Candidate** : une déclaration `local` ou `param` de `declarations.tsv` dont la portée dépasse 40 lignes et dont le nom vérifie l'une de ces conditions :
  - il a trois caractères ou moins ;
  - découpé sur `_`, il a une part absente des mots des noms neufs `renommé` du TSV (l'anglais que l'humain a validé).

  Le paramètre d'un type pointeur de fonction suit la règle de portée du compteur de portée (portée du prototype). Sortie : une ligne par candidate (`module`, `kind`, `name`, `parent`, `scope_lines`), puis le total par module. Ne bloque jamais, avertit si l'extraction est plus vieille que les sources, comme `scope-count`.
- **Mesuré** (prototype, extraction de `c4c161c`) : 844 locales et paramètres à portée > 40, **150 candidates** — 3a 0, 3b 3, 3c 15, 3d 6, 3e 27, 3f 12, 3g 20, 3h 12, 3i 11, 3j 12, 3k 13, 3l 19.
  - Parmi les candidates de trois caractères ou moins, surtout des idiomes (`mem` 16, `gfx` 13, `ctx` 13, `pos` 11, `e` 7) ; les plus longues sont `argc`, `argv`, `origin`…
  - Une candidate gardée comme idiome est notée par sa 4x, pas exemptée : la liste reste une mesure, pas une garde.
  - Le compte exact est celui de l'outil, porté dans la note.
- **Les 12 statiques locales** (relevé de 3a) ne sont pas des candidates (`kind` `static`). Chaque 4x qui en tient une propose son nom dans sa note (4c, 4d, 4f, 4g, 4h), comme le veut la méthode. 4a n'en tient aucune.

### Relevé des citations du cart (`tools/cart_citations.py`, `make cart-citations [MODULES=… | SUBSTEP=…]`, `make cart-citations-test` dans `make test`)

- **Lit** les commentaires et les messages d'`assert` (régions `comment` et `message` de `rename_count`) et compte en trois colonnes :
  - `cart` : le mot entier `cart`, sans distinguer la casse et sans compter `cart_load`, hors des fichiers du module `game/cart` ;
  - `lua` : le mot `Lua` ou `lua.lua` ;
  - `short` : hors backticks, un nom ancien `renommé` du TSV de deux caractères au plus, en mot entier, non collé à une apostrophe. Les homographes anglais d'une liste fixe sont écartés (`a`, `I`, `at`, `on`, `is`, `in`, `to`, `of`, `by`, `as`, `or`, `an`, `up`, `it`, `go`, `no`, `do`, `if`, `be`, `we`, `me`, `my`, `so`, `us`, `he`, `am`, `ok`, `x`, `y`). Les mots français homonymes d'un nom (`le`, `et`, `du`, `en`, `ou`) restent comptés : ils sont à réécrire de toute façon.
- **Sortie** : un site par ligne (`fichier:ligne colonne jeton`), puis le total par module. Ne bloque jamais. Un script séparé plutôt qu'une option de `rename_count.py`, pour ne pas toucher au lecteur du TSV que les autres cibles partagent ; il importe `rename_count` comme `rename_rekey` le fait.
- **Mesuré** (prototype, à `c4c161c`, `cart`/`lua`/`short`) :

  | Sous-étape | cart | lua | short |
  |---|---:|---:|---:|
  | 3a | 5 | 1 | 16 |
  | 3b | 27 | 5 | 111 |
  | 3c | 6 | 8 | 281 |
  | 3d | 19 | 17 | 163 |
  | 3e | 6 | 16 | 134 |
  | 3f | 6 | 13 | 358 |
  | 3g | 9 | 8 | 88 |
  | 3h | 17 | 4 | 113 |
  | 3i | 9 | 1 | 76 |
  | 3j | 16 | 10 | 74 |
  | 3k | 0 | 1 | 411 |
  | 3l | 10 | 2 | 707 |
  | sans place | 1 | 0 | 136 |
  | **Total** | 131 | 86 | 2 668 |

### Mesure de départ (avant tout travail de module, dans la note de 4a)

- **Quoi** : par sous-étape 3a–3l, après le rangement des 40 fichiers :
  - `french-count` (`done`/`later` par colonne) ;
  - `cart-names-count` (`backtick`/`comment`, `done`+`later`) ;
  - `cart-citations` ;
  - `cryptic-locals`.
- **Ordres de grandeur du prototype**, à remplacer par l'outil — français par sous-étape, toutes colonnes : 3a 9 (données de `tools/` seules ; 60 avec `class_coverage.sh`), 3b 159, 3c 1 025, 3d 21, 3e 48, 3f 1 201, 3g 17, 3h 211, 3i 7, 3j 103, 3k 1 825, 3l 2 865 (et 624 dans les 40 fichiers sans place). Anti-cart, `backtick`/`comment` par sous-étape : le bilan de fin d'étape 3 (3a 14/1 en `done` seul ; 57 avec `later`).
- **Règle de coupe, fixée avant la mesure.**
  - *Charge d'une sous-étape* : la somme des quatre relevés sur ses fichiers.
  - *Hors norme* : une charge supérieure au double de la médiane des douze. Le prototype en désigne 3c, 3f, 3k et 3l (3l alourdie des fichiers sans place).
  - *Coupe* : une sous-étape hors norme est coupée comme 3l. Sa PRD se fait en morceaux de modules, chacun avec ses commits et `make test-all` vert. Aucune sous-étape n'est renumérotée.
  - La note de 4a nomme les sous-étapes coupées et leur nombre de morceaux, et corrige le §6 en conséquence.

### Consigne étape 4 (`docs/consigne-etape-4.md`) — points levés par rapport à `consigne-agent.md`

1. **Ce que l'agent touche.**
   - L'agent ne réécrit que des commentaires.
   - Il ne touche ni code, ni chaîne, ni message d'`assert`, ni nom. Pour chacun, il propose dans son rapport le texte ou le nom neuf, et l'orchestrateur les applique dans les commits de chaînes et de locales.
   - Sa preuve reste donc la plus simple : sortie du préprocesseur identique **sans exception** (la clause « aux `assert` près » tombe) et nombre de lignes de code inchangé.
2. **Langue et cart.**
   - Commentaires en anglais (§2). Ne jamais citer le cart : un nom Lua devient le nom C (`docs/renommage.tsv`, colonnes `ancien` → `nouveau`, en lecture seule).
   - « the cart does X » devient « X » ou disparaît. « nil », « Lua's true/truth » se disent en C (NULL, 0, absent, non nul).
   - Un renvoi à un fichier, une fonction ou un champ disparu est vérifié (`grep`), puis corrigé ou supprimé.
3. **Destination 1 supprimée.** Il n'y a plus de nom minifié. Le glossaire (`docs/glossaire-noms.md`) se lit seulement pour comprendre l'ancien commentaire, jamais pour y écrire. Un nom dont le sens ne se lit pas devient une proposition de renommage (rubrique « noms »), pas une ligne de glossaire.
4. **Destination 2 inchangée sauf l'étiquette.** `/* TROU-n */` reste tel quel jusqu'à l'étape 5 (pas de `GAP-n`). Un écart nouveau se marque `/* TROU-? */` et se propose. `docs/trous-de-portage.md` est en lecture seule.
5. **Destination 3 sans `assert` neuf** (ci-dessus). Un `assert` existant garde sa condition. Son message fautif (français, cart) se propose réécrit, avec ses lecteurs (`must_abort_saying`, `expect_fatal`, `grep` du texte). Un `assert` nu dont le commentaire voisin serait le message reste une proposition, comme dans le skill.
6. **Chaîne destinée à l'humain.**
   - Une chaîne française ou qui cite le cart **se corrige** à l'étape 4, mais par l'orchestrateur, selon la règle de propriété des chaînes.
   - L'agent la liste avec trois choses : sa proposition, ses lecteurs (`grep`), et si un test la lit.
   - Une chaîne qui décrit un comportement disparu reste un `fix:` à part, comme dans le skill.
7. **Tests.**
   - Le commentaire d'un test garde « pourquoi cette valeur-là ».
   - Une assertion ne change ni de condition ni de présence. Son message peut seulement être proposé réécrit.
   - Aucune assertion n'est ajoutée à l'étape 4.
8. **Contrôles de l'agent**, sans `make` (le répertoire de build est partagé). L'agent appelle les scripts directement :
   - *Lignes de commentaire*, la seule définition :
     ```sh
     echo $(( $(grep -c . F) - $(gcc -fpreprocessed -dD -E -P F | grep -c .) ))
     ```
     Ce sont les lignes non vides du fichier moins celles du préprocesseur, mesurées avant et après.
   - *Preuve* : sortie du préprocesseur identique au fichier d'avant. Pour un script ou une donnée de `tools/`, les lignes qui ne sont pas des commentaires sont identiques.
   - *Compteurs*, colonnes `backtick` et `comment` à zéro sur ses fichiers, hors messages d'`assert` qu'il a proposés : `python3 tools/rename_count.py … --module`, `python3 tools/french_count.py … --module`, `python3 tools/cart_citations.py … --module`.
9. **Rapport** au format de `modele-rapport.md`, avec trois changements de rubriques :
   - la rubrique 3 (asserts posés) est vide par construction ;
   - la rubrique 5 devient « noms à renommer » : locales candidates de `cryptic-locals`, statiques locales, noms au sens illisible, chacun avec un nom proposé ;
   - deux rubriques s'ajoutent : 10, « chaînes et messages d'`assert` à réécrire » (site, texte actuel, texte proposé, lecteurs, test qui le lit) ; 11, « exceptions proposées » (faux positifs du compteur de français, avec la ligne d'exception prête).

   La rubrique 8 (« hésité à supprimer ») reste obligatoire.

L'amendement de la note de 4a porte sur ce que les rapports de 4a auront contredit dans ces neuf points.

### Registre (`docs/decisions-plan-5.md`, en français)

- **Format** : un en-tête rappelle la condition d'entrée de l'étape 5 (chaque entrée porte un verdict humain). Puis une entrée par question, `D-n`, avec ces champs :
  - **Origine** (sous-étapes, renvoi à la note) ;
  - **Question** ;
  - **Options** ;
  - **Sites** (nombre mesuré à la rédaction, avec la commande) ;
  - **Proposition prête à appliquer** : lignes de `tools/rename_overrides.tsv` ou d'un fichier d'exceptions, édition nommée, commande, contrôle ;
  - **Portée** : pour une décision de nommage, « s'applique aussi aux mentions entre backticks et en commentaire », avec le `grep -rw <ancien>` qui doit rendre 0 ;
  - **Verdict** : vide.
- **Entrées à ouvrir.** Relevé fait sur toutes les phrases « pour décision humaine », « à trancher », « exception proposée », « signalé, non corrigé » des notes de 3a–3l. La note de 3l en dresse la liste ; elle est complète, et une entrée est ajoutée de la mesure de cette PRD. Sites mesurés à `c4c161c` par `grep -rw` sur `src`, `tests`, `tools`, `Makefile`, hors outils du dictionnaire ; l'exécutant les remesure.

| n° | Origine | Question | Sites | Proposition |
|---|---|---|---:|---|
| D-1 | 2b, 3l | Repli `tmp/pico8_replay` de `tools/frames_ref.sh` (voie (a)) | 1 | Garder ; `rename_count.py` lit à l'étape 5 un `tools/cart_names_exceptions.txt` au format de `scope_exceptions.txt` (`string	tools/frames_ref	tmp/pico8_replay`) |
| D-2 | 3k, 3l | `PICO8_CLASS_COVERAGE`, `-D` hors TSV | ≈ 36 (3k : 35 + 2 au `Makefile`) | `CLASS_COVERAGE_BUILD` à tous les sites, configuration `coverage` de `rename-extract` comprise ; contrôle : `make class-coverage` et `rename-extract` sans `warning:` |
| D-3 | 3l | `PICO8_MALLOC_FILLS`, variable d'environnement de `test_entity_header.c` | 2 | `MALLOC_FILLS` (lue et écrite dans le même fichier) |
| D-4 | 3l | Fichiers fictifs `pico8_allocs.c`, `pico8_clean.c`, `pico8_gone.c` des fixtures de `test_alloc_guard.c` | 18 | `allocs.c`, `clean.c`, `gone.c` ; contrôle : `test_alloc_guard` vert |
| D-5 | 3a, 3l | Motif générique `void-ptr` (`pico8_X(`), à vide sur l'arbre | à mesurer | Traduire sans préfixe si `--detail` ne bouge pas, sinon intraduisible ; garder les fixtures |
| D-6 | 3a | `LIEN_EXCLUSION` mort | 2 | Retirer (rien ne l'apparie, ni `pico8_X_of` ni sa traduction) |
| D-7 | 3a | Motif générique `pico8_X_of` (`lien-parent`) | à mesurer | Comme D-5 |
| D-8 | 3a | Motif générique `pico8_X_start` (`défauts`, `--champs`) | à mesurer | Comme D-5 |
| D-9 | 3a, 3b, 3d | Motif `spec` `pico8_X_(default\|defaults_reset)` et `SPEC_EXCLUSION` (mort) | 2 + témoins | Retirer l'exemption ; motif sans préfixe seulement en recoupant les témoins de `test_data_residue.c` (3d : 6 lignes y monteraient) |
| D-10 | 3b | Exclusion `usage-mutuel` (`PICO8`, `P8`), morte | à mesurer | Retirer |
| D-11 | 3a | Catégories historiques (`nu`, `aiguillage-nom`, `un-ou-faux`, `valeur`, `ob`, `stash`, `type-tag`, `ref-audit`, `liste`), à zéro sur du code disparu | 9 catégories | Retirer catégories et fixtures, ou garder telles quelles |
| D-12 | 3e | `--champs` : `entity_init_header`, `allocator_init_slot_kinds` sortent du suffixe de constructeur ; `emitter_init`, `weighted_picker_new`, `random_walk_new` y entrent | 5 | Garder le suffixe `(_new\|_init\|_create\|_init_instance)$` (majorant, pas une porte) |
| D-13 | 3a, 3l | 12 statiques locales de tests à deux lettres (`CV`, `CG` ×2, `as`, `f` ×2, `ks`, `dy`, `kw`, `s`, `od`) | 12 | Exception par règle : dans `scope_count.py`, une `static` dont le parent est une fonction se juge comme une locale, par sa portée ; sinon les noms proposés par 4c, 4d, 4f, 4g, 4h |
| D-14 | 3c | Ordre des mots des macros de taille de zone (`FRAME_ZONE_BYTES`… contre `ZONE_GAME_BYTES`…) | 30 | `ZONE_GAME_BYTES` → `GAME_ZONE_BYTES`, `ZONE_GENERATION_BYTES` → `GENERATION_ZONE_BYTES`, `ZONE_PARTICLES_BYTES` → `PARTICLES_ZONE_BYTES` (forme `<zone>_ZONE_BYTES`) ; `ZONE_BUDGET_BYTES` gardé, ce n'est pas une zone |
| D-15 | 3f | `probe_own` stocke un `targeting_t` | 24 | `targeting_own` |
| D-16 | 3f | `probe_link_target` déclaré dans `targeting.h`, défini dans `probe.c` | 4 | Garder (§7 : pas de redécoupage) ou déplacer la définition |
| D-17 | 3d, 3e, 3f, 3i, 3j | Noms qui bégaient : `data_init_data_shutdown`, `UPDATE_VANISHING_UPDATE_ENTITIES_CALLS`, `targeting_track_target_targeted_by`, `TARGET_TOO_FAR_TARGET`, `emitter_init_help_screen_takes_a_particles_help_screen_list`, `entity_list_list_size`, `entity_list_pointer_list_size`, `score_panel_set_main_score_panel`, `score_panel_score_rows`, `score_rows_row_t` | 4/2/5/8/2/14/11/3/3/8 | Un nom proposé par ligne, en lignes `name` de `tools/rename_overrides.tsv` |
| D-18 | 3f | Étiquette morte `main_mouse` de `test_main_mouse.c` | 1 | Retirer du tableau (commit `test:`) |
| D-19 | 3g | `planet_upkeep` sans le préfixe `economy_score_` | 11 | `economy_score_planet_upkeep` |
| D-20 | 3h | Singletons `main_*` déclarés hors d'un module `main_*` (`main_ui`, `main_mouse*` dans `ui.h`, `main_camera_typed` dans `scroll.h`) | 111/49/47 | Garder (noms du vocabulaire) |
| D-21 | 3i, 3j | Préfixe `g_` des tables `static const` de classe, question transverse | 45 déclarations, 25 fichiers | Retirer `g_` des `static const` en suffixant `_def` (`g_structure_class` → `structure_class_def`, sans heurter l'accesseur `structure_class()`), ou garder — un verdict pour tous |
| D-22 | 3j | Suffixe `_t` sur deux variables | 5 + 4 | `g_score_panel_new_year_keep_open`, `g_fake_update_last` |
| D-23 | 3j | Faute `test_update_entities_drives_a_entity_struct` | 2 | `…_drives_an_entity_struct` |
| D-24 | 3j | `DIFFICULTY_OPTIONS_ROW_CAP` (`help.c`) sans son module | 3 | `HELP_DIFFICULTY_OPTIONS_ROW_CAP` |
| D-25 | 0, 2b, 3k, 3l | `p8_input_*` et `p8_fatal` définis hors de `p8/` (scission d'en-tête, §7) | 5 | Garder (hors périmètre) |
| D-26 | 3g, 3h | Lignes `chaîne` `pz` et `go` éditées hors de `rename-strings` | 2 | Les reporter en surcharges de `tools/rename_strings.py` à l'étape 5 |
| D-27 | 3k | Motif `\bpico8_class_coverage_record\(` de `tools/rename_strings.py`, qui n'apparie plus rien | 1 | `\bclass_coverage_record\(` à l'étape 5 |
| D-28 | §6 « Outillage figé » | Alertes `warn-review` de fin de tronçon 2 (lecteurs en double, listes de mots-clés divergentes, schéma du TSV en français, version de libclang fixée à deux endroits…) | — | Une ligne de verdict par alerte |
| D-29 | PRD 4a (mesure) | Paramètres et locales courts nommés d'un digramme du cart, tolérés par le §1 : 713 lignes `conservé` de deux lettres qui sont l'ancien nom d'un symbole renommé (3a 11, 3b 9, 3c 31, 3d 51, 3e 141, 3f 76, 3g 101, 3h 104, 3i 138, 3j 43, 3k 4, 3l 4), dont 141 paramètres de méthode `qa`, `ra`, `yr`, `sw`, `lw`, `kv`, `fy`, `me` | 713 | Garder (§1), ou renommer la seule famille des paramètres de méthode (`qa` → `mouse_world`, `ra` → `mouse_screen`, `yr` → `year`, `sw` → `slipway`, `lw` → `offer`, `kv` → `dropped`, `fy` → `parallax`, `me` → `state`) par lignes `field <fonction>.<param>` et `rename-apply` |

- **Propositions à vérifier.** Elles sont celles de cette PRD. L'exécutant les vérifie (collision avec `rename-table-check` hors violations par construction, §4) et les corrige dans le registre si une mesure les contredit. Les sens de D-29 viennent du vocabulaire (`qa` : souris en coordonnées monde ; `ra` : position à l'écran) et sont marqués « à confirmer ».

### Travail des modules de 4a

**Commit de chaînes** (`refactor:`), d'abord.

Sites lecteurs hors des modules de 4a, listés en tête du message : `tests/test_class_coverage.c` (3k), lignes 56 (`"illisible: "`), 69 (`"exemptée"`) et 83 (`"sortie différente"`).

Le commentaire de `src/cmd/class_coverage.c` (« le signale alors « illisible » ») devient faux ; il est laissé à 4l (commentaire de 3l) et signalé dans la note. L'assertion d'absence `"inconnue"` (ligne 61) n'a aucun écrivain sur ce chemin ; elle est laissée à 4k et signalée.

Les éditions :
- *Messages du script* : sorties de `class_coverage.sh` en anglais — `illisible:`, `échec:`, `couverture: ÉCHEC -- …`, `couverture: OK -- …`, `exemptée (…)`, `sortie différente`, `rejeu(x)`. Le test `awk` `$4 != "exemptée"` suit le mot neuf.
- *Données de 3a* : `class_coverage_exempt.txt` (« abstract, never instantiated directly ») et `class_coverage_net_expected.txt`, **traduit à la main, pas régénéré** (§5), même tri.
- *Lecteurs* : les trois lignes de `test_class_coverage.c`.
- *Libellé* : `[cw]` → `[class_name]` dans `test_entity_kind.c`.
- *Messages d'`assert`* : les deux de `test_entity_header.c` (même texte deux fois), sans `fx` ni « Lua's true ».

Les preuves :
- **Rouge d'abord** : le script traduit seul fait échouer `test_class_coverage` (ligne 56 la première) ; traduits ses lecteurs, vert.
- **Messages sans lecteur** : pour les messages d'`assert` et le libellé, le `grep` du texte d'avant ne rend que le site lui-même (aucun `must_abort_saying` ni `expect_fatal` ne le lit) ; seul le littéral change.
- **`class-coverage-net`**, rouge avant comme après (référence périmée depuis l'étape 1) : sa sortie d'après est égale à l'octet à celle d'avant passée par la table de traduction du commit, un `sed` écrit dans le message.
- **Tampons** : aucun ne reçoit ces textes (`strstr` sur un journal, `printf` en `%s`) ; dit dans le message.

**Commit de commentaires** (`docs:`), ensuite, fait par la consigne étape 4. Un agent par module C (`entity_class.h`, `test_entity_header.c`, `test_entity_kind.c`), une vague. Les commentaires de `class_coverage.sh` sont repris par l'orchestrateur. Si l'exécutant ne peut lancer d'agents, il applique lui-même la consigne et remplit le rapport.

Preuves :
- sortie de `gcc -fpreprocessed -dD -E -P` identique à l'octet, fichier par fichier ;
- lignes de code identiques. Départ mesuré (lignes non vides / lignes du préprocesseur / commentaire seul) : `entity_class.h` 174/134/40, `test_entity_header.c` 270/252/18, `test_entity_kind.c` 216/201/15 ;
- pour `class_coverage.sh`, lignes hors commentaire identiques ;
- ratio de commentaire avant/après dans le message.

Points connus à reprendre, sans préjuger du rapport :
- *Mentions du cart* : `dg`, `qc[qc.me](qc,qc.t)`, `cg`, `ke`, `kv:jr`, `mw(ob)`, `gx[]`, `ie.ke`, `kv[hp.cw]`, `ie:hb`/`ie:kj`, `jc:hq`, `gb`.
- *Noms C périmés* : `rec` → `record`, `rr`/`s`/`me`/`od` → `required_tech`/`sprite`/`state`/`cost`, `PICO8_TAG(t)`/`PICO8_LINEAGE(l)` → `TAG(t)`/`LINEAGE(l)`, `lt()`, `pico8_struct_header_t` → `entity_t`, `pico8_X_kind_new`, `pico8_file_sortie.h`, `pico8_slipway.h`/`pico8_probe.h`.
- *Récit dans le contrat de `down_t`* : « a `stop` that only caught false swallowed every click near an unrevealed planet » va au message de commit.
- *Affirmation à vérifier* : la paire d'includes qui justifie `slipway_class`/`probe_class` dans l'en-tête.

**Locales** : aucun commit. `cryptic-locals SUBSTEP=3a` vaut 0 avant (mesuré) et doit valoir 0 après. Les noms courts de 3a relèvent de D-29, pas de 4a.

### Fin de PRD

**Ordre des commandes** :
1. `make rename-extract` frais, sans `warning:` (les 4 `partial` attendues) ;
2. `make rename-rekey-check SUBSTEP=3l` vert ;
3. les compteurs et le relevé sur `SUBSTEP=3a`.

`rename-rekey` est sans objet à 4a : aucune ligne du TSV n'est ajoutée.

**Constat pour le plan.** « `rename-rekey-check` sur toutes les sous-étapes » est rouge par construction depuis l'étape 3 : chaque ligne de 3a–3l est appliquée et son ancien nom a disparu. Mesuré à `c4c161c` : `SUBSTEP=3a` 4 640 introuvables, `3k` 33, `3l` 0.

La porte de l'étape 4 est donc :
- `rename-rekey-check SUBSTEP=3l` vert ;
- toute ligne de locale ajoutée par une 4x est écrite depuis une extraction de l'arbre courant et appliquée dans le même commit. Les éditions de commentaires déplacent des lignes, et `rename-rekey` apparie par ligne.

La note de 4a corrige la puce « Extraction fraîche » du plan dans ce sens.

**Critère de fin de 4a**, sur `SUBSTEP=3a`, extraction fraîche :
- `cart-names-count` : `done` et `later` à 0 dans les quatre colonnes ;
- `french-count` : `done` et `later` à 0 dans les quatre colonnes, hors exceptions proposées en attente ;
- `scope-count` : 0 ;
- `cryptic-locals` : 0 ;
- `cart-citations` : 0 dans les trois colonnes.

Seul reste admis : `PICO8_MALLOC_FILLS` (D-3), qu'aucun compteur ne voit.

**Invariant de fin (§5)** :
- `make test-all` vert, `tests/e2e/*.sha256` inchangés ;
- `make raylib` et `make screencast-viewer` compilent et linkent, jamais lancés ;
- `rename-tool-test` et tous les `rename-*-test` verts ;
- `class-coverage-net` rouge, sortie égale à celle d'avant par la table de traduction ;
- `e2e-gap` rouge, mêmes 2 932 lignes ;
- `data-residue` et `composition-residue` à 0 ;
- `--detail` des deux gardes à motifs identique.

### Note « Découvert » de 4a (dans `docs/plan-5.md`, sous l'entrée 4a)

Elle porte :
- les quatre outils et leurs cibles ;
- la liste de mots retenue ;
- les mesures de départ de 3a à 3l, et les sous-étapes coupées ;
- le rangement des 40 fichiers ;
- le lieu de la consigne et son amendement ;
- le nombre d'entrées ouvertes au registre ;
- l'application à 4a : ratio, preuves, rouge d'abord ;
- la correction de la puce « Extraction fraîche » ;
- les relevés de fin ;
- les exceptions proposées au compteur de français (lignes prêtes, verdict au registre).

## Implementation Chunks

1. **Outillage, table, consigne, registre** — `make test-all` vert à la fin, aucune ligne de code du jeu touchée. Chaque outil se fait test d'abord : le test se commite rouge avec le script vide ou absent, puis vert.
   1. Compteur de français, test d'abord (fixtures ci-dessous), cibles `french-count` et `french-count-test`, ajoutée à `test`.
   2. Liste des locales cryptiques (`--cryptic` de `scope_count.py`, tests ajoutés à `test_scope_count.py`), cible `cryptic-locals`.
   3. Relevé des citations, test d'abord, cibles `cart-citations` et `cart-citations-test`, ajoutée à `test`.
   4. Rangement des 40 fichiers dans `tools/rename_substeps.tsv` ; `rename-substeps-check` vert ; `french-count` sans `warning:` de module sans place.
   5. Mesures de départ de 3a à 3l (sorties gardées dans `tmp/`, chiffres pour la note) et désignation des sous-étapes hors norme.
   6. `docs/consigne-etape-4.md`, puis `docs/decisions-plan-5.md` (D-1 à D-29, sites remesurés).

   Les lignes neuves de la légende du `Makefile` s'écrivent en anglais. Le reste de la légende, en français, revient à 4l.
2. **Modules de 4a** — commit de chaînes (rouge d'abord), puis commit de commentaires (préprocesseur identique), puis extraction fraîche, compteurs, invariant de fin.
3. **Note « Découvert »** — note de 4a, corrections du §6 (sous-étapes coupées, puce « Extraction fraîche »), amendement de `docs/consigne-etape-4.md` d'après les rapports des agents, entrées du registre complétées (exceptions de français proposées).

## Testing Decisions

- **Ce qu'est un bon test ici** : boîte noire, comme `tests/test_scope_count.py` et `tests/test_rename_count.py`. Le test écrit un mini-arbre (ou une mini-extraction, un mini-TSV, une mini-table de sous-étapes) sous `tmp/`, lance le script par `subprocess`, lit la table imprimée et le code de sortie. Il ne teste aucune fonction interne. Ces tests tournent dans `make test`.
- **`tests/test_french_count.py`** couvre :
  - détection : un mot-outil par colonne, `identifier` par une part d'identifiant, `backtick`, `comment`, message d'`assert`, `string` en littéral C et en ligne de donnée ;
  - un mot accentué compte ; `×`, `—`, `→` ne comptent pas ;
  - homographes : `place`, `file`, `limite` comme identifiants et dans une phrase anglaise, 0 ;
  - exception : une ligne retire exactement son triplet ;
  - scission : un littéral repris tel quel dans un fichier d'une sous-étape suivante est `later`, sinon `done` ;
  - exclusions : ses propres fichiers et les outils du dictionnaire ne sont pas lus ;
  - avertissement pour un module sans sous-étape ;
  - code de sortie 0 toujours.
- **`tests/test_scope_count.py`, cas `--cryptic`** :
  - portée 40 contre 41 ;
  - `abc` candidate, et un nom à part absente des noms neufs du mini-TSV ;
  - `planet_index`, fait de mots du mini-TSV, pas candidate ;
  - `field`, `static`, et paramètre de pointeur de fonction ignorés ;
  - `SUBSTEP` et `MODULES` filtrent ;
  - les cas existants restent verts.
- **`tests/test_cart_citations.py`** :
  - `cart` compté hors du module `game/cart`, pas dedans, pas dans `cart_load` ;
  - `Lua` et `lua.lua` comptés ;
  - un nom court du mini-TSV compté nu, pas entre backticks, pas dans le code ni dans une chaîne ;
  - homographe anglais écarté ;
  - message d'`assert` lu ;
  - `SUBSTEP` filtre ;
  - code 0.
- **Travail des modules** : la suite existante est le filet. `test_class_coverage` fournit le rouge d'abord du commit de chaînes. La preuve du commit de commentaires est le diff du préprocesseur, pas un test. Aucun test neuf de module, aucune assertion changée (§ consigne, point 7).
- **Prior art** :
  - `tests/test_scope_count.py` (mini `declarations.tsv`, table de sous-étapes) ;
  - `tests/test_rename_count.py` (mini-arbre, colonnes `identifier`/`backtick`/`string`/`comment`) ;
  - `tests/test_rename_substeps.py` ;
  - le rouge d'abord des chaînes de 3c–3l (`expect_fatal` de `test_zone.c` en 3l).

## Out of Scope

- Les modules de 3b–3l (sous-étapes 4b–4l). Les lecteurs de `test_class_coverage.c` sont seuls touchés, au titre de la propriété des chaînes, et le commentaire stale de `cmd/class_coverage.c` est laissé à 4l.
- Toute décision du registre : 4a les ouvre et ne les applique pas (`PICO8_MALLOC_FILLS` compris). Les verdicts sont humains, avant l'étape 5.
- Renommer les paramètres et locales courts nommés d'un digramme du cart (D-29).
- `TROU-N` → `GAP-N`, la garde de français rendue bloquante, la suppression du glossaire : étape 5.
- Traduire la légende du `Makefile`, les scripts des gardes et les données rangées en 3l : 4l.
- Modifier le skill `reprendre-les-commentaires` ou un autre skill.
- Changer un comportement, ajouter un `assert`, toucher une condition d'assertion.

## Further Notes

- **Mesures en lecture seule** : extraction de `c4c161c` (aucune source plus récente), sorties dans `tmp/ast_rename/` et le scratchpad.
  - `cart-names-count SUBSTEP=3a` : `done` 0/14/0/1, `later` 0/41/0/1, résidu `later identifier` 0.
  - `scope-count SUBSTEP=3a` : 0 ; sans `SUBSTEP` : 12, les statiques locales.
  - `rename-rekey-check` : 3a 4 640, 3k 33, 3l 0.
  - Prototypes jetables (scratchpad) pour le français, les citations et les candidates ; leurs chiffres sont indicatifs, ceux des outils font foi.
- **`class_coverage_net_expected.txt` est périmé** depuis l'étape 1 (`layout_row` 2742/2763, `trade` 247/263) : `class-coverage-net` reste rouge, hors périmètre du plan 5. La traduction ne le répare pas et ne le régénère pas.
- **Ordre recommandé du commit de chaînes** : script d'abord (rouge), lecteurs ensuite (vert), données traduites, puis `class-coverage-net` comparé.
- **Une 4x qui renomme des locales** écrit ses lignes de TSV et de surcharges depuis une extraction de l'arbre courant, `rename-slice` sur ces seules lignes (`MODULES=`), `rename-apply`, dans un commit à part, avant ou après les commentaires, jamais au milieu.
- **Concepts utiles à l'agent de `entity_class.h`**, tirés du vocabulaire :
  - `me`/`t` = `state`/`state_time` ;
  - `ma` = `shake` ;
  - `fx` = `fold_anchor` ;
  - `p_` = `is_dead` ;
  - `ra` = `pos`, `p` = `map_pos` ;
  - `cg` = `entities_by_tag` ;
  - `lt` = `entity_register` ;
  - `hb` = `refresh`, `mm` = `spawn_particle`.
