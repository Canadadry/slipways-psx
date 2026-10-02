---
title: "Plan 5, sous-étape 4d — commentaires, chaînes et locales des données et de la mise en page (tables de jeu, tables de carte, génération, galaxie, fonctions de dessin, rangées et bulles, HUD)"
description: "Ramener à zéro, sur les quatorze modules de 3d, le français résiduel, les noms du cart (backtick et commentaire, fait et suivantes), les citations du cart et les locales cryptiques, en traduisant les clés du cart restées dans les données (fonctions de dessin, clé de production) sans toucher un seul texte affiché, et sans changer un pixel."
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
> 3d. **Données et mise en page.** `game/` : `game_data`, `map_tables`, `data_init`, `gen`, `galaxy`, `draw_functions`, `layout`, `hud`. Tests : `test_map_tables`, `test_data_init`, `test_spec_defaults`, `test_layout_row`, `test_layout_nil_item_draws_nothing`, `test_tooltip_draw_at`. Résidu ≤ 362.
>
> 4d. **Commentaires — données et mise en page.** Modules et tests de 3d.

4d n'est pas coupée par 4a (charge 413 ≤ 1 382) : un seul morceau. Ses compteurs se lisent par `SUBSTEP=3d`, dont la liste de modules est la ligne `3d` de `tools/rename_substeps.tsv` :

```
game/game_data game/map_tables game/data_init game/gen game/galaxy game/draw_functions game/layout game/hud tests/test_map_tables tests/test_data_init tests/test_spec_defaults tests/test_layout_row tests/test_layout_nil_item_draws_nothing tests/test_tooltip_draw_at
```

Fichiers (22) : `src/game/{game_data,map_tables,data_init,gen,galaxy,draw_functions,layout,hud}.[ch]`, `tests/test_map_tables.c`, `tests/test_data_init.c`, `tests/test_spec_defaults.c`, `tests/test_layout_row.c`, `tests/test_layout_nil_item_draws_nothing.c`, `tests/test_tooltip_draw_at.c`. Aucune donnée de `tools/` ni fixture n'est rangée en 3d (`rename_substeps.tsv`) ; « données » désigne ici les tables C du jeu.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4). Sous-étape 4d = l'entrée 4 appliquée aux modules, tests et données de `tools/` de 3d.

Entrées lues : `docs/plan-5.md` §1, §2, §4, §5, l'entrée 4 et sa méthode, l'entrée 3d et sa note « Découvert », les notes de 4a, 4b, 4c (4c1 et 4c2) et la note de 3l (« Fin de l'étape 3 ») ; `docs/consigne-etape-4.md` ; `docs/decisions-plan-5.md` (D-1 à D-38) ; le skill `reprendre-les-commentaires` ; la PRD de 4c2 (`28fdf74^`, modèle de forme de celle-ci) et `git log -5 --stat` : 4c2 s'est commitée en trois commits de code, chaînes (`refactor:`), locales (`refactor:`, `b1930a1`) et commentaires avec note (`docs:`, `5a03ad0`), puis la PRD graduée (`28fdf74`).

Aucun humain n'était disponible : l'agent rédacteur a chargé lui-même `guideline` aux points « vérifier avec l'utilisateur » de `to-prd`. Les décisions reposent sur le plan et sur des mesures en lecture seule à `28fdf74` : écritures dans `tmp/` seulement (`make rename-extract` frais, `rename-rekey-check SUBSTEP=3l`, compteurs, `rename-table-check` ; sorties dans `tmp/4d_baseline/`). Les rouges prédits ci-dessous ont été **joués** dans un clone jetable hors du dépôt (`git clone` local dans le répertoire de travail de l'agent, jamais poussé) : chaque prédiction est une observation, valeurs de hachage comprises.

**Condition d'entrée** : `origin/plan-5-noms-anglais` à `28fdf74` ou un descendant, arbre propre, `make test-all` vert ; `make rename-extract` frais sans `warning:` (les 4 lignes `partial` de `class_coverage` attendues) ; `rename-rekey-check SUBSTEP=3l` vert (0 introuvable) ; la mesure de départ ci-dessous retrouvée à l'identique. Sorties de `class-coverage-net` et `e2e-gap` relevées au départ sous `tmp/4d_base/`.

## Problem Statement

Les données et la mise en page du jeu — tables de `game_data` (le plus gros fichier de `src/game`, 757 lignes d'initialiseurs désignés, sans un commentaire), tables de carte de `map_tables`, construction des données (`data_init`), tirages pondérés et marche aléatoire (`gen`), peuplement de la galaxie (`galaxy`), table de dispatch des fonctions de dessin (`draw_functions`), rangées et bulles (`layout`), primitives du HUD (`hud`) — et leurs six tests parlent encore le cart. Les en-têtes décrivent leurs champs par digrammes (`rr`, `me`, `od`, `qe`, `px`, `lr`, `fj`, `ja`, `il`, `dm`…), citent des symboles disparus (`pico8_layout_init`, `pico8_pi_init`, `pico8_hc_table`, `PICO8_JO_DZ_COUNT`…), parlent de « Lua's seq[i] » et de « the cart's false ». Les messages d'`assert` sont préfixés de digrammes (`"lq: …"`, `"jb: …"`, `"pi dispatch table full"`, `"rk: pico8_rectfill …"`, `"r lookup: … never keys r"`).

Plus sournois : **des clés du cart vivent encore dans les données**, invisibles aux compteurs parce que 2b ne les a pas extraites. Les rangées de `game_data` nomment leur fonction de dessin `"br"`, `"qr"`, `"rf"`, `"s"` — les clés de la table `pi` du cart (`pi={…br=rk,qr=io,rf=eq,s=spr}`), que `draw_functions_init` enregistre sous ces noms ; et la clé de production `"qh+"` de `map_tables` est la moitié oubliée de `"qh"` → `"structure"` (3a n'a renommé que la chaîne exacte). À côté, `game_data` et `map_tables` portent des centaines de chaînes qui **ne sont pas** des noms du cart : textes affichés au joueur (`"restart     "`, `"earth-like"`, `"wants more:"`) et codes de données d'origine (`"e"`, `"f+"`, `"efmorxjisg"`). Les premières doivent changer ; les secondes ne doivent pas bouger d'un octet, sous peine de changer l'écran — et les `*.sha256` des e2e.

Enfin, quatre locales à longue portée ont un nom qui dit leur valeur, pas leur rôle (`one`, `two_seventy_five`, `eighth` de `galaxy_populate`) ou un nom tronqué (`cls`, 42 lignes de `test_layout_row_is_a_struct`).

## Solution

Appliquer à 4d la méthode prouvée de 4a à 4c2 : un seul morceau, trois états de code — chaînes (rouge d'abord, tous les lecteurs d'un coup, y compris hors de 3d), locales (quatre, par `rename-apply`), commentaires (préprocesseur identique, hauteur des blocs gardée dans les sept `.c` cités par `tools/line_coverage_reference.txt`) — puis la note « Découvert » de 4d. Une règle explicite trie les chaînes de `game_data` et `map_tables` : on suit chaque chaîne jusqu'à son consommateur ; ce qui finit dessiné reste tel quel, ce qui finit dans un `strcmp`/une recherche/un dispatch et nomme un identifiant du cart se traduit. L'exécutant ne commite pas : il laisse les patchs d'états intermédiaires et trois messages dans `tmp/`, et l'arbre de travail à l'état final (§ « Remise »). À la fin, sur `SUBSTEP=3d` : français, anti-cart (`done` et `later`, quatre colonnes), citations et locales cryptiques à zéro, portée à 4 (D-13), `grep pico8_/PICO8_` à 0 hors D-2, `make test-all` vert et `tests/e2e/*.sha256` inchangés.

## User Stories

1. En tant que lecteur de `game_data.h`, je veux que chaque commentaire nomme les champs C de la struct qu'il décrit (`resource`, `draw_function`, `upkeep_by_tier_table`…) au lieu des digrammes (`qe`, `fn`, `rr`), afin de relier le commentaire au code sans glossaire.
2. En tant que joueur, je veux que rien de ce que je vois ne change — libellés de menu, noms de planète, textes d'aide, bulles —, afin que la traduction du code n'ait aucun effet sur le jeu.
3. En tant que relecteur, je veux une règle écrite qui dise quelle chaîne d'une table de données est un texte affiché, quelle chaîne est un code de données d'origine, et quelle chaîne est une clé du cart, afin de vérifier qu'aucun texte affiché n'a été traduit.
4. En tant que mainteneur de `draw_functions`, je veux que les clés de dispatch soient `"draw_bar"`, `"print_outlined"`, `"draw_help_frame"`, `"spr"` et `"draw_row"`, le nom de la fonction enregistrée sans `_entry`, afin de retrouver par `grep` quelle fonction dessine une rangée.
5. En tant que relecteur, je veux voir les clés de dispatch rougir dans l'ordre prédit — l'`assert` de `help_init` dans 43 binaires avec l'écrivain seul, puis douze binaires à leur premier hachage d'écran avec le lecteur de `help.c`, puis deux hachages qui plient la clé —, afin de savoir que chaque lecteur lit vraiment la clé.
6. En tant que mainteneur de 4j, je veux que la PRD liste les lecteurs hors de 3d (`src/game/help.c:136`, `BUILT_HASH` de `tests/test_help_screen.c`, `BUILT_HASH` et `SHARED_HASH` de `tests/test_score_panel.c`) et ce qui y change, afin de ne pas être surpris par une ligne de mes fichiers touchée par 4d.
7. En tant que mainteneur, je veux que le tampon `char name[16]` de la table de dispatch soit vérifié contre la plus longue clé neuve (`draw_help_frame`, 15 caractères), afin qu'aucune clé ne soit tronquée sans bruit.
8. En tant que mainteneur de `map_tables`, je veux que la clé de production `"qh+"` devienne `"structure+"`, comme sa moitié `"structure"` renommée par 3a, afin que la paire de base et d'amélioration se lise ensemble.
9. En tant que lecteur, je veux que les messages d'`assert` des données et de la mise en page nomment les fonctions C (`layout_row_draw_at`, `tooltip_create`, `layout_row_keys_of_row`, `resource_code_lookup`, `hud_draw_bar`, `weighted_picker_new`), afin qu'un arrêt se comprenne sans le cart.
10. En tant que lecteur d'une sortie de test, je veux que les libellés de `check()` et des `printf` de test ne citent plus le cart (`"jo.ex"`, `"hp:lq and copy"`, `"dr[1]"`, `"table ol"`, `"lq({nil}):ol()"`), afin qu'une ligne de sortie se relie à la constante qu'elle vérifie.
11. En tant que lecteur, je veux que le nom d'arène français `"essai-panneau"` de `test_layout_row` devienne `"test-panel"`, sur le modèle `test-*` de 4c1, afin qu'aucun mot français ne reste dans le morceau.
12. En tant que lecteur de `galaxy_populate`, je veux que `one`, `two_seventy_five` et `eighth` disent leur rôle (`fixed_one`, `map_span`, `map_margin`), afin de comprendre le tirage `rnd(map_span) + map_margin` sans calcul.
13. En tant que lecteur de `test_layout_row_is_a_struct`, je veux `row_class` plutôt que `cls`, afin que la locale se lise.
14. En tant que mainteneur du TSV, je veux que chaque locale renommée passe `renommé` avec son identité courante, et une ligne `identity` dans `tools/rename_overrides.tsv`, afin que le TSV reste ce qui a été appliqué.
15. En tant que relecteur, je veux un état de commentaires dont la sortie de `gcc -fpreprocessed -dD -E -P` est identique à l'octet, fichier par fichier, afin de le relire sans relire le code.
16. En tant que mainteneur de `e2e-gap`, je veux que chaque bloc de commentaire et chaque chaîne réécrite des sept `.c` cités par `tools/line_coverage_reference.txt` garde sa hauteur, afin que la référence de couverture par ligne ne décale pas.
17. En tant que lecteur, je veux que chaque renvoi nomme un symbole qui existe (`draw_functions_init`, `layout_init`, `production_tables_table`, `PLANET_MOOD_ROWS_COUNT`, `research_build_projects`), afin de le suivre par `grep`.
18. En tant que lecteur, je veux que les affirmations soient revérifiées en traduisant (`draw_functions.h` dit encore « un nom de 8 caractères ou plus serait tronqué », faux depuis `name[16]`), afin qu'une traduction ne recopie pas une erreur.
19. En tant que mainteneur, je veux qu'aucune décision non tranchée du registre (D-2, D-9, D-12, D-13, D-17, D-21, D-29, D-31, D-32, D-36) ne soit appliquée, afin que l'humain garde la main.
20. En tant qu'humain du point d'arrêt, je veux deux entrées neuves au registre — le nom de test de `test_data_init.c` qui cite le cart, et les clés du cart que la table ne porte pas —, afin de trancher ce que 4d ne peut pas trancher.
21. En tant qu'exécutant de 4e, 4f et 4i, je veux savoir que le renommage de `cls` fait compter cinq mentions `` `cls` `` de mes fichiers en `backtick`, afin de les réécrire par le rôle.
22. En tant qu'orchestrateur, je veux des patchs cumulatifs et trois messages dans `tmp/`, afin de commiter les trois sortes de commit séparément avec `make test-all` vert à chacun.

## Implementation Decisions

### Mesure de départ (`28fdf74`, extraction fraîche, `SUBSTEP=3d`, écrits dans `tmp/4d_baseline/` seulement)

`make rename-extract` : aucun `warning:`, 4 lignes `partial` (`cmd/class_coverage.c`, `tests/test_class_coverage.c` × 2 configurations) ; `rename-rekey-check SUBSTEP=3l` : 0 identité introuvable.

| Compteur | id | backtick | string | comment / détail | total |
|---|---|---|---|---|---|
| `french-count` `done` / `later` | 0 / 0 | 0 / 0 | 0 / 0 | 7 / 0 | 7 |
| `cart-names-count` `done` | 0 | 56 | 0 | 35 | 91 |
| `cart-names-count` `later` | 0 | 110 | 0 | 9 | 119 |
| `cart-citations` (cart/lua/short) | — | — | — | 20/17/158 | 195 |
| `cryptic-locals` | — | — | — | `one` (63 lignes), `two_seventy_five` (61), `eighth` (60) de `galaxy_populate` ; `cls` (42) de `test_layout_row_is_a_struct` | 4 |
| `scope-count` | — | — | — | `static` : `as`, `f` ×2, `ks` de `test_layout_row` — D-13 | 4 |

Charge 7 + 210 + 195 + 4 = 416, contre 413 écrits par 4a : les trois mentions `` `gfx` ``/`` `mem` `` de `draw_functions.h` (2) et `hud.h` (1), comptées en `backtick` depuis les locales de 4b (D-36). `residue 3d: later identifier 0`.

Détail :
- *Français* (`SITES=1`), 7 sites `comment`, tous des noms du cart qui ressemblent à des mots-outils, qui tombent avec la réécriture : `game_data.h:117` `le` (`ui.le`), `:366` `ou`, `:370` `le` ; `map_tables.h:64` `il` ×2, `:65` `il` ; `test_layout_row.c:777` `le` (message `"ok.le holds js buttons only"`). **Angle mort** (consigne, point 6) : relevé complémentaire des mots de contenu, liste de 4c1 plus `panneau rangee bulle bulles mesure` (`grep -nowiE`, `tmp/4d_baseline/content_words.txt`) : 5 — `test_layout_row.c:650` `essai`, `panneau` (nom d'arène, état 1), `:986` `place` (« one row place », français pour *slot*, état 3), `:1000` `place` (message d'`assert`, état 1), `test_layout_nil_item_draws_nothing.c:56` `places` (« How many places … appears », homographe anglais, reste).
- *Anti-cart* (`backtick`/`comment`, `done`+`later`) : `game/layout` 42/10, `tests/test_layout_row` 53/1, `game/game_data` 16/6, `tests/test_spec_defaults` 19/0, `game/gen` 12/3, `game/map_tables` 3/10, `game/draw_functions` 7/3, `tests/test_tooltip_draw_at` 8/2, `game/galaxy` 1/3, `game/hud` 3/1, `game/data_init` 0/4, `tests/test_layout_nil_item_draws_nothing` 2/1. Le critère est zéro en `done` **et** en `later` : une mention d'un nom d'une sous-étape suivante dans un fichier de 3d se réécrit ici (« l'étape 3 finie, aucun résidu d'une sous-étape suivante ne reste »).
- *Noms nus `pico8_*`/`PICO8_*`* : 38 occurrences (`tmp/4d_baseline/pico8_grep.txt`) — `layout.h` 9, `data_init.h` 4, `game_data.h` 4, `draw_functions.h` 3, `galaxy.c` 3, `gen.h` 3, `test_layout_nil_item_draws_nothing.c` 3, `layout.c` 2, `test_tooltip_draw_at.c` 2, `gen.c`, `hud.c`, `hud.h`, `map_tables.h`, `test_layout_row.c` 1 chacun. Une seule dans une chaîne (`hud.c:48`, état 1) ; deux sont le `#ifdef PICO8_CLASS_COVERAGE` de `layout.c` (338, 423), **D-2, non tranchée : restent** ; le reste en commentaire (état 3). Critère de fin : ce `grep` rend exactement les deux lignes de D-2.
- *Citations* : `cart` 20, `lua` 17, `short` 158 — `game_data` 4/2/53, `layout` 5/5/37, `map_tables` 0/1/25, `hud` 2/4/5, `gen` 2/3/3, `galaxy` 0/0/8, `test_layout_nil_item_draws_nothing` 2/1/2, `test_layout_row` 0/0/3, `test_spec_defaults` 0/0/3, `test_map_tables` 0/1/1, `test_tooltip_draw_at` 1/0/0 ; les `short` sont surtout des noms de champ cités nus (`qe`, `fn`, `ra`, `lq`, `d`, `c`) et des paramètres (`e`, `w`, `h`).
- *Lignes de commentaire* (non vides moins préprocesseur ; lignes de code entre parenthèses) : `game_data.c` 0 (668), `game_data.h` 41 (347), `map_tables.c` 0 (305), `map_tables.h` 18 (83), `data_init.c` 1 (11), `data_init.h` 6 (5), `gen.c` 11 (87), `gen.h` 17 (25), `galaxy.c` 9 (63), `galaxy.h` 3 (5), `draw_functions.c` 17 (153), `draw_functions.h` 17 (15), `layout.c` 21 (385), `layout.h` 50 (92), `hud.c` 5 (75), `hud.h` 23 (17), `test_map_tables.c` 4 (187), `test_data_init.c` 0 (44), `test_spec_defaults.c` 18 (216), `test_layout_row.c` 76 (843), `test_layout_nil_item_draws_nothing.c` 48 (111), `test_tooltip_draw_at.c` 12 (131).
- *Hauteur des blocs* (consigne, point 8, amendé par 4b) : sept `.c` de 3d sont cités par `tools/line_coverage_reference.txt` — `data_init.c`, `draw_functions.c`, `galaxy.c`, `game_data.c`, `gen.c`, `hud.c`, `layout.c`. Chaque bloc de commentaire y garde son nombre de lignes, et chaque chaîne réécrite à l'état 1 y garde ses lignes (le message de `gen.c:66-67` reste sur deux lignes, celui de `layout.c:205-207` sur trois). `map_tables.c` n'a aucun commentaire. `make e2e-gap` est exigé égal à l'octet à chacun des trois états.
- *`CASES[]`* : aucun test de 3d n'a de table de cas à processus enfant (`test_layout_row` forke, mais sans noms de cas) ; la règle des noms de cas de 4c1 ne s'applique pas.
- *Étiquettes `TROU-N`* : aucune dans les 22 fichiers ; un écart nouveau se marque `/* TROU-? */` et se propose.

### Trier les chaînes des données : affiché, code de données, clé du cart

`game_data.c` porte 86 littéraux distincts, `map_tables.c` environ 200. La sortie e2e est un hachage du framebuffer : **un texte dessiné qui change d'un octet change un `*.sha256`**. Règle, appliquée chaîne par chaîne en suivant le littéral jusqu'à son consommateur (lecture du code, pas du mot) :

1. **Texte affiché — intouchable.** La chaîne atteint un dessin : `.text` d'un `data_item_t` d'une rangée que `layout_row`/`tooltip` ou `print_outlined_entry` dessine, nom ou libellé passé à `hud_print_outlined`/`p8_print`, `period_names_table`, les noms de `planet_types_table` (`"earth-like"`, `"gas giant"`), les menus (`"restart     "`, `"how to play\?"`, espaces de calage compris), `"slipways.net/help"`, `"-too far-"`, les noms de projets, de technologies et les textes d'aide de `map_tables.c` (`"-lab-"`, `"space folding"`, `"wants more:"`). Ces textes sont en anglais (le cart l'était) ; aucun ne cite un identifiant minifié. Preuve : `tests/e2e/*.sha256` inchangés à chaque état.
2. **Code de données d'origine — intouchable.** La chaîne est une valeur que le code compare, découpe ou compose comme donnée, écrite telle quelle dans le cart non minifié : codes de type de planète (`"e"`, `"f"` … `"g"`, `"vanishing"`), codes de ressource (`"f"`, `"o"`, `"b"`, `"\?"`), clés de production `"f-"`, `"f+"`, `"-"`, `"+"`, listes de codes (`"efmorxjisg"`, `"mrsoijx"`, `"xj"`, `"pf"`…), clés numériques de `resource_names_table` (`"28"`…), préfixe d'identifiant de planète `"p"` (`"p"..id` dans le cart), états (`"placing"`, `"settled"`, `"normal"`), noms de classe déjà traduits par 3a (`"structure"`, `"burst"`), chemins `pico8/*.json`. Ce ne sont pas des noms minifiés : le minifieur renomme des identifiants, pas le contenu des chaînes.
3. **Clé du cart — se traduit.** La chaîne est une clé de table du cart que le minifieur a renommée comme un identifiant, ou un composé d'une telle clé, et qui ne finit jamais à l'écran : elle ne sert qu'à un `strcmp`, une recherche ou un dispatch. Dans 3d : les clés de dispatch `"br"`, `"qr"`, `"rf"`, `"s"` (clés de `pi` dans `pico8/lua.lua`, ligne 133, et `fn="…"` des rangées), et `"qh+"` (`"qh"` + `"+"`, le `"qh"` exact étant déjà `"structure"` depuis 3a). Aucune n'a de ligne `chaîne` dans le TSV : le compteur anti-cart ne les voit pas (`string` 0), d'où D-40.

Contrôle de la règle, à écrire dans le message de l'état 1 : `grep -o '"[^"]*"' src/game/game_data.c src/game/map_tables.c | sort -u` avant et après ; les seuls littéraux qui diffèrent sont ceux de la catégorie 3 et les messages d'`assert`.

### État 1 — chaînes (`refactor:`), rouge d'abord

**A. Clés de dispatch des fonctions de dessin.** Traductions, nom de la fonction enregistrée sans `_entry`, comme `"ld"` → `"draw_row"` (`draw_row_entry`) en 3d :

| Clé | Fonction | Neuve | Longueur |
|---|---|---|---|
| `"br"` | `draw_bar_entry` | `"draw_bar"` | 8 |
| `"qr"` | `print_outlined_entry` | `"print_outlined"` | 14 |
| `"rf"` | `draw_help_frame_entry` | `"draw_help_frame"` | 15 |
| `"s"` | `spr_entry` | `"spr"` | 3 |

Sites : écrivain `src/game/draw_functions.c:191-194` (`draw_functions_dispatch_register`) ; rangées de `src/game/game_data.c` (26 lignes `.draw_function = "…"` : `"br"` ×8, `"qr"` ×13, `"rf"` ×3, `"s"` ×2, comme les 8/13/3/2 `fn=` du cart) ; lecteurs **hors de 3d**, listés en tête du message : `src/game/help.c:136` (`draw_functions_dispatch_lookup("s")`, la clé seule change, le message `"pico8_help_init() runs after pico8_pi_init()"` reste à 4j, son écrivain), et trois constantes de hachage qui plient `row->draw_function` (`fold_string(fnv_str(h, "draw_function"), row->draw_function)`) : `tests/test_help_screen.c` `BUILT_HASH` `0x088E67AEu` → `0xABEBF0FEu`, `tests/test_score_panel.c` `BUILT_HASH` `0x5EC1E758u` → `0xC419CD23u` et `SHARED_HASH` `0x387783BAu` → `0x1F82BA04u` (valeurs affichées par `check()`, reportées, comme 3d pour `CONSTRUCTIONS_HASH`). `grep -rn '"br"\|"qr"\|"rf"\|"s"' src tests tools` refait avant le rouge : hors ces sites, `"s"` n'apparaît que comme code de type de planète (`game_data.c:35`, catégorie 2) et comme préfixe de `structure_icon.c:133` (autre chaîne) — non touchés.

Tampon : `char name[16]` de `draw_functions_dispatch_entry_t` ; plus longue clé `draw_help_frame`, 15 octets + NUL = 16 : tient, l'`assert` de troncature (`strlen(name) < sizeof name`) passe (joué). Le tableau reste une statique de fichier de 8 entrées, 5 clés.

Rouge prédit (joué dans le clone jetable, `make -k test`) :
1. *écrivain seul* (`draw_functions.c`) : 43 binaires rouges, tous par `src/game/help.c:136: help_init: Assertion 'draw_functions_dispatch_lookup("s") != NULL …'`, directement ou par leur enfant ou leur pilote (`test_ascension`, `test_background`, `test_burst`, `test_button`, `test_camera`, `test_class_coverage`, `test_economy`, `test_entity_header`, `test_entity_kind`, `test_exit_queue`, `test_frame_zone`, `test_game_zone`, `test_headless_capture`, `test_help_screen`, `test_infraspace_gate`, `test_layout_row`, `test_line_coverage`, `test_live_slots`, `test_main_camera`, `test_main_economy`, `test_main_mouse`, `test_message_box`, `test_modal_backdrop`, `test_mouse`, `test_planet`, `test_planet_spec_defaults`, `test_probe`, `test_processor`, `test_protostar`, `test_replay_from_title`, `test_research_project`, `test_score_panel`, `test_slipgate`, `test_slipway`, `test_spec_defaults`, `test_structure`, `test_structure_icon`, `test_synthesizer`, `test_toast`, `test_trade`, `test_trading_hub`, `test_ui`, `test_wormhole`) ;
2. *+ lecteur `help.c`* : 12 binaires rouges à leur premier hachage d'écran, rangées de `game_data` non dessinées — `test_ascension`, `test_burst`, `test_help_screen`, `test_infraspace_gate`, `test_probe`, `test_protostar`, `test_score_panel`, `test_slipgate`, `test_synthesizer` à `[life]`, `test_layout_row` à `[panel]`, `test_research_project` à `[built world]`, `test_message_box` à `[year with a message]` (`run->hash == want_sequence`) ;
3. *+ rangées de `game_data.c`* : `test_help_screen` à `[built]` (`0xABEBF0FEu`) et `test_score_panel` à `[built]` (`0xC419CD23u`) puis, celui-ci corrigé, `[shared]` (`0x1F82BA04u`) : la clé est pliée dans ces trois hachages, pas dans un pixel ;
4. *+ constantes* : `make test` vert, `make e2e` 26 « conforme », `*.sha256` inchangés.
Le message liste ces quatre paliers ; tout écart (un binaire de plus ou de moins, une autre valeur) y est écrit.

**B. Clé de production `"qh+"` → `"structure+"`.** Écrivain `src/game/map_tables.c` (2 sites : la clé de l'entrée et le `upgraded_entry` de `"structure"`) ; lecteur `tests/test_data_init.c:23` (`production_tables_lookup("qh+")`, 3d). `research.c` lit la clé par `production_tables_lookup(qp->upgraded_entry)`, sans littéral. Rouge prédit (joué), écrivain seul : `test_data_init` seul, à la ligne 23 ; les e2e restent conformes (la clé ne s'affiche pas). Lecteur traduit, vert.

**C. Messages d'`assert` de `src/game/` sans lecteur** (`grep -rF` de chaque fragment sur `src tests tools Makefile .claude` : seul l'écrivain ; aucun `must_abort_saying` ni `expect_*` ne les lit, donc pas de rouge possible — à écrire dans le message). Chaque nom de fonction relu dans le code, pas deviné ; propositions :
- `game_data.c:709`, `:718` `"a sequence is a table and its length"` → `"a sequence is an array and its length"` ;
- `map_tables.c:279`, `:292`, `:304` `"r lookup: key must be a string -- a number never keys r"` (`iz`, `hc`) → `"resource_code_lookup: key must not be NULL"`, `"resource_names_lookup: …"`, `"production_tables_lookup: …"` (la moitié « a number never keys » décrit Lua) ;
- `gen.c:66-67` → `"weighted_picker_new: the ticket pool is sized for its one game call, planet_types_table at a multiplier of 2; a larger table or multiplier needs a larger WEIGHTED_PICKER_TICKETS_CAP"` (appelants relus : `galaxy.c:79` et `test_map_tables.c:133`, tous deux à 2), sur deux lignes ;
- `draw_functions.c:34`, `:35`, `:65` : `"pi dispatch …"` → `"draw_functions dispatch table full"`, `"draw_functions dispatch name would be truncated"` ; `"an article of bl holds no nested row"` → `"a row drawn by draw_functions_draw_rows holds no nested row"` ;
- `layout.c:26`, `:205-207`, `:242`, `:243-244`, `:314`, `:374-375`, `:420`, `:436-437` : préfixes `lq:`/`jb:`/`lq:ol:` → la fonction qui porte l'`assert` (`layout_row_cell_string` 26, `layout_row_draw_at` 205, `tooltip_line` 242 et 243, `row_struct_alloc` 314, `layout_row_keys_of_row` 374, `tooltip_struct_alloc` 420, `tooltip_create` 436), digrammes du corps par leurs noms C (`c` → `color`, `fn` → `draw_function`, `qe` → `resource`, `jo.lq` → la rangée d'une planète…) ; « the cart's longest bubble line (nb_9_oy_1) has 7 items » → la plus longue ligne de bulle des données, après vérification dans `map_tables.c` ;
- `hud.c:48` `"rk: pico8_rectfill dereferences gfx, so a NULL one is not a no-op"` → `"hud_draw_bar: p8_rectfill dereferences the graphics state, so a NULL one is not a no-op"`.

**D. Chaînes des tests de 3d** (écrivain et lecteur dans le même fichier, ou sans lecteur ; `grep` des libellés sur `Makefile tools .claude` : aucun lecteur de la sortie standard des tests) :
- *Nom d'arène* : `"essai-panneau"` (`test_layout_row.c:650`) → `"test-panel"`.
- *Messages d'`assert`* : `test_layout_row.c:124` (« (row, lq, vector) » → « (row, layout row, vector) »), `:210`, `:211`, `:219` (préfixe `nil_at:` — c'est le paramètre de `fold_row_from`, non cité nu : « only keys the test wrote can be absent »…), `:777` `"ok.le holds js buttons only"` → les boutons du panneau de planète, au nom C relu dans `planet_display`/`ui`, `:1000` `"… a longer `i` needs a bigger row place"` → « … a longer input list needs a bigger row slot » ; `test_map_tables.c:124` déjà anglais.
- *Libellés de `check()`* : règle « le libellé est le nom de la constante vérifiée, en minuscules, sans `_HASH` » ; `test_layout_row.c` : `"jo.ex"` → `"planet_resource_row"`, `"hp:lq and copy"` → `"research_project_layout_row"` (`"constructions"`, `"calls"`, `"panel"` y satisfont déjà) ; `test_tooltip_draw_at.c` : `"dr[1]"` → `"icon_tooltips_first_entry"`, `"table ol"` (même constante) → `"icon_tooltips_first_entry drawn"`, `"[dr all by name]"` → `"[icon_tooltips_all_by_name]"` (`"bare"`, `"none"`, `"[mixed by name]"` restent : ce sont des libellés de cas, non du cart). Un libellé n'est **pas** plié dans un hachage (`check` l'imprime seulement ; vérifié) : aucun rouge possible.
- *Libellés de `fold_built_from`* (imprimés, non pliés) : `"string c"` → `"string color"`, `"spr w qw qv"` → les noms des champs du `placed_item` que le tuple remplit, relus dans `layout.h`, `"nil"` → `"nothing"`, `"mixed c"` → `"mixed color"`, `"bg false"` → `"background false"`.
- *Clé pliée* `fold_tuple_name(h, "c")` (`test_layout_row.c:146`) → `"color"`, comme `"resource"` en 3d ; aucune rangée pliée n'a `has_color` : aucun hachage ne bouge (joué), donc aucun rouge — à écrire.
- *`printf` de `test_layout_nil_item_draws_nothing.c`* (130, 132, 157, 182) : `lq({nil}):ol()` / `lq({0}):ol()` → `layout_row_draw_at` d'une rangée à cellule absente / à `0` ; ligne finale `"lq nil-item tests passed"` → `"test_layout_nil_item_draws_nothing: all tests passed"`, forme des autres binaires.
- Inchangés : `"zz"` (clé absente), `"pool is built without a draw"`, `"f-"`, `"-lab-"`, `"space folding"`, `"normal"`, `"placing"`, chemins `pico8/*.json`, `"wants more:"`, `"receives:"`, `"needs:"`, `"w"` (catégories 1 et 2).

Preuves, dans le message :
- **lecteurs hors de 3d** en tête : `help.c:136`, `test_help_screen.c` (`BUILT_HASH`), `test_score_panel.c` (`BUILT_HASH`, `SHARED_HASH`), tous de 4j ; aucun autre (`grep` des clés et des fragments) ;
- **rouge d'abord** : les quatre paliers de A et le rouge de B, chaque binaire relancé seul (`make unit-run-test_X`) pour lire son premier rouge ;
- **table de traduction** : un `sed` (dans le message) qui, appliqué aux lignes `-` de `draw_functions.c`, `game_data.c`, `help.c`, `map_tables.c` et `test_data_init.c`, rend exactement leurs lignes `+` ;
- **tampon** : `name[16]`, 15 octets au plus ;
- **tri** : le `diff` des littéraux de `game_data.c`/`map_tables.c` (section précédente) ;
- `make test-all` vert, `tests/e2e/*.sha256` inchangés, `make e2e-gap` égal à l'octet.

### État 2 — locales (`refactor:`), procédure de 4b et 4c2

Quatre identités, renommées par `rename-slice`/`rename-apply` seulement, après une **extraction fraîche de l'état 1** (les identités portent un offset ; celui de `cls` bouge avec les libellés de `test_layout_row.c`) :

| Fichier | Fonction | Ancien | Nouveau | Pourquoi |
|---|---|---|---|---|
| `src/game/galaxy.c` | `galaxy_populate` | `one` | `fixed_one` | portée 63 lignes ; le 1 en virgule fixe, départ et pas de `planet_id` |
| idem | idem | `two_seventy_five` | `map_span` | portée 61 ; l'étendue du tirage `rnd(2.75)` en cases de carte |
| idem | idem | `eighth` | `map_margin` | portée 60 ; la marge ajoutée au tirage (coordonnées dans [0,125 ; 2,875[) |
| `tests/test_layout_row.c` | `test_layout_row_is_a_struct` | `cls` | `row_class` | portée 42 ; `layout_row_class` masquerait la fonction |

Identités relevées à `28fdf74` (à relever de nouveau après l'état 1) : `c:galaxy.c@612@F@galaxy_populate@one`, `c:galaxy.c@714@F@galaxy_populate@two_seventy_five`, `c:galaxy.c@780@F@galaxy_populate@eighth`, `c:test_layout_row.c@31559@F@test_layout_row_is_a_struct@cls`. Leurs lignes `conservé` du TSV portent des identités périmées (`@pico8_bw@`, `@test_lq_is_a_struct@`) : elles passent `renommé`, nom neuf, identité courante. Procédure (comme `b1930a1`) : table `ancien<TAB>nouveau<TAB>fichier<TAB>identité` dans `tmp/ast_rename/decision.tsv`, `make rename-apply TABLE=tmp/ast_rename/decision.tsv` ; quatre lignes `identity` dans `tools/rename_overrides.tsv` ; `rename_derive.py` reproduit les quatre lignes à l'octet ; `rename-table-check` rouge par construction, 27 708 au départ : les huit violations des quatre lignes (« missing row », « row without extracted identity ») tombent, aucune n'apparaît. Contrôle de masquage avant l'apply : aucun nom neuf déjà déclaré dans la fonction (`declarations.tsv`). Pas de `rename-rekey`.

Effets de bord, à écrire dans le message : `one`, `eighth` et `cls` deviennent d'anciens noms ambigus ; une mention exacte entre backticks compte en `backtick` partout (comme D-36). `` `one` ``, `` `eighth` `` : 0 dans le dépôt ; `` `cls` `` : 5, hors de 3d — `src/game/planet_family_trade.c:182` (4e), `src/game/planet.h:96` (4e), `src/game/probe.h:49` (4f2), `src/game/planet_family.h:35`, `:79` (4i) : `TOTAL` `backtick` monte de 5, à réécrire par le rôle par ces sous-étapes (note). `two_seventy_five` devient un nom distinctif : 0 autre site. `galaxy.c` étant dans `line_coverage_reference.txt`, l'apply ne déplace aucune ligne.

Non renommés : les statiques `as`, `f` ×2, `ks` de `test_layout_row` (D-13, noms déjà proposés au registre) ; les paramètres et locales courts de portée ≤ 40 lignes (`hr`, `og`, `r`, `px`, `qe` de `test_map_tables`, `e`, `a`…), conservés (D-29) et jamais cités nus ni entre backticks dans les commentaires réécrits.

### État 3 — commentaires (`docs:`), consigne étape 4

« Fichier d'avant » : l'état 2, copié sous `tmp/4d_before_comments/` avant tout agent (consigne, point 8). Un agent par groupe, en-tête et `.c` ensemble, chaque test avec son module : `game_data` + `test_spec_defaults` ; `map_tables` + `data_init` + `test_map_tables` + `test_data_init` ; `gen` + `galaxy` ; `draw_functions` + `hud` ; `layout` + `test_layout_row` + `test_layout_nil_item_draws_nothing` + `test_tooltip_draw_at`. Consigne `docs/consigne-etape-4.md`, rapport au format amendé. Sans agents, l'exécutant applique la consigne lui-même et remplit le rapport, comme de 4a à 4c2.

Preuves : sortie de `gcc -fpreprocessed -dD -E -P` identique à l'octet, fichier par fichier ; lignes de code inchangées ; dans les sept `.c` cités par `line_coverage_reference.txt`, sortie sans `-P` identique (chaque ligne de code à son numéro) ; lignes de commentaire avant/après dans le message.

Points connus, sans préjuger du rapport :
- *Un champ se nomme par sa struct.* La plupart des digrammes de `game_data.h` et `map_tables.h` désignent un champ de la struct qui suit le commentaire (`qe` → `resource`/`has_resource`, `fn` → `draw_function`, `px`, `lr`, `fj`, `ja`, `i`, `o`, `il`, `dm` → `name`, `planet_codes`, `base_cost`, `production_key`, `inputs`, `output`, `upgrade_tech`, `byproduct`…). Le nom C se lit dans la struct, pas par une recherche globale du TSV : un même digramme a plusieurs nouveaux noms (`c` : `color`, `candidate`, `character`). Le TSV (`ancien`, `parent`) sert de contrôle.
- *Renvois périmés*, chacun vérifié par `grep` puis remplacé ou supprimé : `pico8_layout_init` ×3, `pico8_layout_shutdown` ×2, `pico8_data_init` ×2, `pico8_pi_init`, `pico8_pi_dispatch_lookup`, `pico8_research_init`, `pico8_research_build_gb`, `pico8_score_panel_init`, `pico8_hc_table`, `pico8_lq_*`, `pico8_jb_ol`, `pico8_gn_*`, `pico8_jf_*`, `pico8_fixed_*`, `pico8_cart_load`, `PICO8_JO_DZ_COUNT` (→ `PLANET_MOOD_ROWS_COUNT`)… par le TSV et le code, jamais de mémoire.
- *Mentions `` `gfx` ``/`` `mem` ``* (`draw_functions.h:30`, `hud.h:31`) : réécrites par le rôle (« the graphics state », « PICO-8 memory »), consigne amendée par 4b (D-36).
- *Affirmations à revérifier en traduisant* (consigne, rubrique 7) : `draw_functions.h:12-14` « every registration happens once, from `pico8_pi_init()` » et « a name of 8 characters or more would be truncated » — faux depuis `name[16]` (3d) : 16 ou plus, et la plus longue clé fait 15 ; `draw_functions.h:37` « `"ld"` dispatches to `lq:ol`, so `pico8_layout_init()` must have run » ; `draw_functions.c:95-96` « no `bl()`-dispatched call site omits `col` » (relire les rangées `draw_bar` de `game_data.c`) ; `data_init.h` (« runs `research_init` before `research_build_projects` », « forgets every `e_` of `nb` », « safe to call more than once ») ; `galaxy.c:44-47`, `:66` (« self-registering into `hv`/`cv`/`cg` ») ; `gen.c:26-29` (fenêtre anti-répétition de 3) ; `game_data.h:115-117` (archétype et défauts « ui.le, nf.ra, mz.jj ») ; `map_tables.h:61-64` (positions 1..npos).
- *« Lua's … », « the cart's false », « nil »* : dits en C (NULL, 0, absent ; « 1-based, as the data tables are written »), ou supprimés si le code le dit déjà.
- *Récit à sortir vers le message de commit* : « this port's usual absent-field default » (`galaxy.c:71`), les comparaisons avec PICO-8 qui ne sont ni piège ni contrat.
- *Contrats à garder* (destination 4) : propriété des pointeurs de dispatch (`g_draw_functions_gfx`/`_mem` empruntés, posés avant chaque appel), qui libère ce que `tooltip_new`/`layout_row_new` allouent, retours absents (`NULL` quand la clé manque, `0` quand l'élément n'est pas un nombre).

### Ce qui reste hors de 4d

- Le message `"pico8_help_init() runs after pico8_pi_init()"` de `help.c:136` et le reste de `help.c`, `test_help_screen.c`, `test_score_panel.c` : 4j (4d n'y change que la clé et deux ou trois constantes).
- Les cinq mentions `` `cls` `` de 4e, 4f2, 4i (effet de bord de l'état 2).
- `PICO8_CLASS_COVERAGE` (D-2), les statiques de D-13, `data_init_data_shutdown` (D-17), `weighted_picker_new`/`random_walk_new` dans `--champs` (D-12), `tickets`/`type_picker` (D-31), `rest_count` (D-32), `g_` des tables (D-21), `SPEC_EXCLUSION` (D-9), paramètres de digramme (D-29) : non tranchées, non appliquées ; les commentaires réécrits citent les noms actuels.

### Registre (`docs/decisions-plan-5.md`)

Deux entrées neuves, au format des conventions, ligne d'index comprise :
- **D-39** (4d) — nom de test qui cite le cart : `test_production_tables_project_records_icon_tooltips_technologies_decode_real_cart_content` (`tests/test_data_init.c`, 2 sites : définition, appel de `main`), même question que D-35. Options : (a) renommer ; (b) garder (« cart » y désigne les tables du jeu). Proposition (a), procédure N : `…_decode_real_game_data` ; contrôle : `grep -rnwiE '[a-z_]*(lua|cart)[a-z_]*' tests/test_data_init.c` rend 0.
- **D-40** (4d) — clés du cart sans ligne `chaîne` au TSV : `"br"`, `"qr"`, `"rf"`, `"s"` (clés de `pi`) et `"qh+"` (composé de `"qh"`), traduites par 4d selon la règle de propriété des chaînes, que 2b n'avait pas extraites (clés de deux lettres hors du TSV ; composé non apparié par la règle « la chaîne est exactement le nom »). La garde anti-cart ne verrait pas une telle clé revenir. Options : (a) rien, le contrôle reste humain ; (b) ajouter à l'étape 5 des lignes `chaîne` par `rename-strings` (outil, test d'abord), pour que la garde les compte ; (c) un relevé ponctuel à l'étape 5 des `fn="…"` de `pico8/lua.lua` contre les clés de `draw_functions_init`. Proposition (a) ; sites : 0 après 4d.

Si un rapport d'agent soulève une autre question de nommage ou un faux positif qui résiste à la réécriture, une entrée s'ajoute de même ; la note le dit dans tous les cas. D-13 se remesure à la fin (`scope-count SUBSTEP=3d` rend exactement `as`, `f`, `f`, `ks`).

### Fin de PRD

Ordre : `make rename-extract` frais sans `warning:` (4 `partial`) ; `rename-rekey-check SUBSTEP=3l` vert ; compteurs sur `SUBSTEP=3d`.

**Critère de fin** :
- `french-count SUBSTEP=3d` : 0 en `done` et en `later`, quatre colonnes ;
- `cart-names-count SUBSTEP=3d` : 0 en `done` et en `later`, quatre colonnes ;
- `cart-citations SUBSTEP=3d` : 0/0/0 ;
- `cryptic-locals SUBSTEP=3d` : 0 ;
- `scope-count SUBSTEP=3d` : 4 (`as`, `f` ×2, `ks`, D-13), rien d'autre ;
- `grep` `pico8_`/`PICO8_` des 22 fichiers : exactement les deux `#ifdef PICO8_CLASS_COVERAGE` de `layout.c` (D-2) ;
- relevé complémentaire des mots de contenu : seul `places` (`test_layout_nil_item_draws_nothing.c`, homographe anglais), chaque reste listé dans la note ;
- `grep -rn '"br"\|"qr"\|"rf"\|"qh+"\|draw_function = "s"\|dispatch_lookup("s")\|register("s"' src tests` à 0 ;
- `rename-strings-verify SUBSTEP=3d` : 0 site en défaut sur 24 (les 3 `unseen` classés de 3d).

**Invariant (§5)** : `make test-all` vert à chacun des trois états, `tests/e2e/*.sha256` inchangés ; `make raylib` et `make screencast-viewer` compilent et linkent, jamais lancés ; `rename-tool-test` et les huit `rename-*-test` verts ; `class-coverage-net` et `e2e-gap` rouges, sorties égales à l'octet à celles de `28fdf74` (`tmp/4d_base/`) ; `data-residue` et `composition-residue` à 0, `--detail` et `--champs` identiques ; `make sin-table` reproduit `quarter_wave.h` à l'octet.

### Note « Découvert » de 4d (sous l'entrée 4d de `docs/plan-5.md`)

Elle porte : mesure de départ et de fin (relevé complémentaire et homographe compris) ; la règle de tri des chaînes de données et son contrôle ; les clés de dispatch et `"qh+"`, leurs lecteurs hors de 3d, les quatre paliers de rouge et les trois constantes ; le tampon `name[16]` ; les quatre locales, leurs identités et l'effet `` `cls` `` sur 4e, 4f2, 4i ; lignes de commentaire, hauteur des blocs des sept `.c`, affirmations corrigées de l'état 3 ; D-39 et D-40 ; ce qui est laissé à 4j ; et les amendements de `docs/consigne-etape-4.md` tirés de l'exécution — au moins ceux-ci, si elle les confirme :
- *Point 6* : une chaîne d'une table de données se trie par son consommateur — dessinée : intouchable (les `*.sha256` le prouvent) ; code de données d'origine : intouchable ; clé du cart (clé de table minifiée, ou composé d'une telle clé) : traduite par son écrivain, même sans ligne `chaîne` au TSV, que les compteurs ne voient pas. Pour la trouver : les clés comparées par `strcmp`/recherche/dispatch dont le nom est un ancien nom de deux lettres, et les `fn="…"`/clés de table de `pico8/lua.lua`.
- *Point 6* : un test qui plie une chaîne dans un hachage figé en est lecteur ; sa constante se corrige à la valeur affichée par `check()`, dans le commit de chaînes.
- *Point 7* : un libellé de `check()` est le nom de la constante vérifiée, en minuscules, sans `_HASH`.

## Implementation Chunks

Un seul morceau (charge 416 ≤ 1 382). L'exécutant produit, dans cet ordre, `make test-all` vert à chaque état :
1. état 1, chaînes ; `git diff HEAD > tmp/4d_stage1.patch`, message `tmp/4d_1_msg.txt` (`refactor: plan 5 sous-étape 4d — clés du cart, messages et libellés des données et de la mise en page en anglais`, lecteurs hors de 3d en tête) ;
2. état 2, locales ; `git diff HEAD > tmp/4d_stage2.patch` (cumulatif : états 1 et 2), message `tmp/4d_2_msg.txt` (`refactor: plan 5 sous-étape 4d — locales one, two_seventy_five, eighth et cls au nom de leur rôle`) ;
3. état 3, commentaires, note « Découvert », amendements de la consigne et entrées D-39, D-40 du registre, laissés dans l'arbre de travail ; message `tmp/4d_3_msg.txt` (`docs: plan 5 sous-étape 4d — commentaires des données et de la mise en page en anglais, note Découvert`).

## Remise

L'exécutant ne commite pas. Il laisse :
- `tmp/4d_stage1.patch` : diff complet de `HEAD` à l'état 1 (s'applique par `git apply` sur `HEAD` propre) ;
- `tmp/4d_stage2.patch` : diff complet de `HEAD` à l'état 2, cumulatif — facultatif : si l'état 2 se révèle vide (par exemple si un verdict humain arrive avant l'exécution et le rend caduc), il est égal à l'octet à `tmp/4d_stage1.patch`, `tmp/4d_2_msg.txt` n'est pas écrit, et le rapport final le dit en une ligne ;
- `tmp/4d_1_msg.txt`, `tmp/4d_2_msg.txt`, `tmp/4d_3_msg.txt` : messages de commit en français, récit de chantier et preuves compris (destination « message de commit » du skill), terminés par les lignes d'attribution de la session ;
- l'arbre de travail à l'état 3 : commentaires, note, consigne, registre.

## Testing Decisions

- Aucun test neuf, aucune assertion ajoutée ni modifiée dans sa condition (consigne, points 5 et 7). Le filet : l'`assert` de `help_init` et les hachages d'écran de douze binaires, prouvés lecteurs des clés de dispatch par le rouge d'abord ; `test_data_init`, seul lecteur de `"qh+"` ; les e2e, preuve qu'aucun texte affiché n'a bougé ; le compilateur pour l'état 2 ; la sortie du préprocesseur et `e2e-gap` pour l'état 3.
- Trois constantes de hachage changent (`BUILT_HASH` de `test_help_screen`, `BUILT_HASH` et `SHARED_HASH` de `test_score_panel`) : la clé y est pliée, aucun pixel ne change ; la valeur neuve est celle que `check()` affiche, jamais calculée à la main.
- Un bon contrôle juge une sortie observable : texte lu sur stderr, statut de sortie, valeur affichée d'un hachage, `*.sha256` des e2e, sortie à l'octet des gardes hors suite.
- Prior art : la note de 3d (`"ld"` → `"draw_row"`, `name[16]`, `CONSTRUCTIONS_HASH` reporté), 4c2 (lecteurs listés d'abord, rouge d'abord par binaire, `sed` de traduction ; `b1930a1` pour les locales ; `5a03ad0` pour le préprocesseur et la hauteur des blocs).

## Out of Scope

- Tout texte affiché au joueur et tout code de données d'origine (catégories 1 et 2), même abrégé ou en apparence cryptique.
- Toute ligne de code hors des chaînes de l'état 1 et des quatre locales de l'état 2 ; le reste de `help.c`, `test_help_screen.c`, `test_score_panel.c` (4j).
- `TROU-N` → `GAP-N` (étape 5) ; toute entrée du registre sans verdict.
- Le formatage du code, `tools/french_count.py` et sa liste de mots.

## Further Notes

- `game_data.c` et `map_tables.c`, les deux plus gros fichiers de 3d, n'ont aucun commentaire : leur travail est le tri de leurs chaînes, pas la prose. Le risque de 4d est de « traduire » un texte affiché ou un code de données ; la règle du tri et les `*.sha256` le bornent.
- `"draw_help_frame"` occupe exactement le tampon (15 + NUL = 16) : une future clé plus longue fera tomber l'`assert` de troncature, ce qui est voulu.
- Les clés de dispatch suivent le nom de la fonction enregistrée, ce qui rend `"spr"` cohérent avec `spr_entry` et l'API PICO-8 `spr`.
