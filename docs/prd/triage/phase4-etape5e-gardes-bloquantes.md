---
title: "Plan 5, sous-étape 5e — gardes bloquantes : old-names-guard et french-guard dans make test, ast-guard chaînée à make test-all (D-66 (c), D-67 (a)), chaque garde prouvée vivante par des violations plantées qui passent par sa vraie cible et restent comme tests permanents"
description: "Après 5d, les trois gardes construites par 5a rendent 0 sur l'arbre réel avec leurs exceptions, mais aucune n'est branchée : un ancien nom, une globale de deux lettres ou un mot français réintroduits passent la suite. Les brancher selon D-66 (c) et D-67 (a) — partie texte de l'anti-cart et français dans make test, sans libclang ; partie AST et portée dans make test-all — et prouver chacune vivante par des violations plantées dans une copie de travail réduite, passées par la vraie cible du Makefile, gardées comme tests permanents (text-guards-test dans make test, ast-guard-test dans make test-all)."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6, à `4d6571c`)

> 5. **Clôture.** Condition d'entrée : chaque entrée de `docs/decisions-plan-5.md` porte un verdict humain, pris dans l'ensemble fermé du registre (« Conventions » : `proposition`, `variante nommée`, `garder tel quel`, `reporter hors plan 5`), l'alerte 14 de D-68 comprise. Une PRD par sous-étape, dans l'ordre 5a–5f ; chacune applique les entrées de sa famille (colonne « Application » de l'index du registre) et finit par son contrôle de sortie, avec l'invariant du §5. Une entrée `garder tel quel` ou `reporter hors plan 5` n'édite rien, sauf la ligne d'exception d'un reste compté (5d).
>
> 5e. **Gardes bloquantes**, seulement quand tous les compteurs sont à 0 avec leurs exceptions (sortie de 5d) : les deux gardes du §5 dans `make test`, vertes, par le mécanisme de D-66 ; le compteur de français en troisième garde selon le verdict de D-67.
>
>    **Sortie** : une violation plantée dans une copie de travail, passée par la vraie cible, rend `make test` rouge (sous D-66 (c), la cible AST chaînée) : pour la garde de portée, une globale de deux lettres ; pour l'anti-cart, un ancien nom du TSV dans un commentaire et dans un identifiant ; sous D-67 (a), un mot français dans un commentaire. Ces violations restent comme tests permanents de `make test`.

Phase 4 du pilotage (tronçon 4 : étape 5). 5e est la cinquième sous-étape de l'entrée 5. Avant elle : 5a (famille O, close), 5b (famille G, close), 5c (famille N, close) et 5d (familles X et T). Après elle : 5f (docs et skills).

**Entrées lues**, toutes au commit `4d6571c` :
- `CLAUDE.md` ;
- `docs/plan-5.md` : §1, §2, §4 et §5, l'entrée 5, les entrées 5a à 5f, et les notes « Découvert » de 5a (*Lecture de « ok »*, *Frontière texte/AST*, *Trois gardes, hors `test` et `test-all`*, *Coût de `ast-guard` pour 5e*), de 5b (*La garde anti-cart ne voit pas les noms historiques*) et de 5c (*Pièges laissés*) ;
- `docs/decisions-plan-5.md` : D-1, D-13, D-37, D-56, D-59, D-60, D-66, D-67, D-68 (alerte 14), la « Clôture (4l4) » et son tableau « Restes comptés » ;
- `docs/prd/next/phase4-etape5d-exceptions-gap.md`, la PRD de 5d. Elle est en cours ; je l'ai lue sans la modifier ;
- les PRD de 5a (`git show 4bb53ec`) et de 5c (`git show b9e4873` pour la note), pour la forme ;
- le `Makefile` : la légende, les cibles `test`, `test-all`, `e2e`, `rename-extract`, `old-names-guard`, `ast-guard`, `french-guard`, `float-guard`, et les variables `AST_RENAME_*`, `LIBCLANG`, `RENAME_TABLE` ;
- `tools/rename_count.py` (`Names`, `source_files`, test de fraîcheur), `tools/scope_count.py` (`newest_source`, `--check`), `tools/french_count.py` (`FUNCTION_WORDS`, `OWN_FILES`), `tools/cart_citations.py` (`OWN_FILES`), `tools/rename_strings.py` (`SKIPPED_FILES`, `SKIPPED_PARTS`), `tools/ast_rename/bindings.py` (messages d'absence), `tools/rename_substeps.tsv` ;
- `tests/test_scope_count.py`, `tests/test_rename_count.py` (harnais : arbre jetable sous `tmp/`, boîte noire, code de sortie) ;
- les skills qui lancent la suite : `prd-autopilot`, `check-e2e`, `fix-bug`, `tdd`, `gh-release`.

**Aucun humain n'était joignable** pendant la rédaction. `to-prd` demande de vérifier avec l'utilisateur les modules, les tests et le découpage : je l'ai fait seul. Chaque décision est écrite au § « Décisions prises sans humain », pour être relue au prochain point d'arrêt. La PRD est écrite dans `docs/prd/next/`, comme celle de 5d, parce que 5d n'est pas encore implémentée.

**5d n'était pas faite au moment de la rédaction.** Tout ce que cette PRD supposait de son issue est écrit en hypothèse `H-5d-k` (§ « Hypothèses sur 5d »). Chaque hypothèse dit les sections à relire.

**Relu contre la note « Découvert » de 5d (`88efe31`)**, et contre le code au `HEAD` `c97eb65` (5d poussée, sa PRD graduée). J'ai remesuré sur un clone jetable à `c97eb65`, supprimé ensuite. Les onze hypothèses tiennent. Seules deux précisions de forme corrigent la PRD (H-5d-2, H-5d-9) :

- **H-5d-1 — tient.** La note donne, après `rename-extract` frais sans `warning:` (4 `partial`) et sans `SUBSTEP`, les cinq compteurs à 0 : `cart-names-count` 0/0/0/0, `french-count` 0/0/0/0, `cart-citations` 0/0/0, `cryptic-locals` 0, `scope-count` 0. Elle donne aussi les trois gardes à 0. Remesuré :
  - `make old-names-guard` : code 0, 2,6 s ;
  - `make french-guard` : code 0, 0,9 s ;
  - `make test e2e ast-guard` : code 0 en 146 s, `TOTAL` 0 et `total` 0, aucune ligne `warning:`, 26 e2e.
- **H-5d-2 — tient, une précision.** Les trois fichiers ont la forme attendue : deux lignes `#` d'en-tête en anglais, puis des triplets.
  - `tools/cart_names_exceptions.txt` : la seule ligne `string	tools/frames_ref	tmp/pico8_replay`.
  - `tools/cart_citations_exceptions.txt` : 3 lignes de D-56, 5 lignes `short` de D-59.
  - `tools/french_exceptions.txt` : 17 triplets sous ses 2 lignes d'en-tête.

  *Corrigé* : le premier triplet `string` de `tools/french_exceptions.txt` est `string	tests/test_cart_citations	catégorie`, et non `du`. Le cas « exception française, autre module » plante donc `catégorie` dans `src/game/planet.c`. Mesuré dans la copie réduite : `game/planet` `string` 1, code 2.
- **H-5d-3 — tient.** Les recettes `old-names-guard` et l'appel `rename_count.py` d'`ast-guard` passent `--exceptions tools/cart_names_exceptions.txt`. `french-guard` n'a pas changé. Le statut d'`ast-guard` reste accumulé, avec `rename-extract` en prérequis. `rename_count.py` saute le fichier d'exceptions par son chemin relatif à `--root`, et applique les exceptions aux deux moitiés et à `--check` (décision 4 de 5d). La copie réduite (`Makefile`, table, `tools/*.py`, `tools/*_exceptions.txt`, `tools/rename_substeps.tsv`) est verte, sans plante, sous les deux gardes texte. Les neuf cas texte du tableau donnent chacun le module et la colonne attendus. Le triplet de D-1, planté dans `tools/frames_ref.sh` de la copie, donne code 0.
- **H-5d-4 — tient.** Dans `column_of`, une `static` dont le `parent` n'est pas `-` se juge par sa portée, sans passer par `is_long_scope`. Une `static` de fichier reste refusée en colonne `static`, une globale en colonne `global`. Les règles de D-37 (`argc`/`argv` de `main`) et de D-60 (`RAYLIB_WORDS`, `blank` et `texture`) ne vivent que dans `is_cryptic`, sous `--cryptic`. Mesuré : la plante `int qz;` donne `game/planet` `global` 1, partie AST 0, code 2.
- **H-5d-5 — tient.** `AST_RENAME_UNITS` est toujours récursive et surchargée sans peine. La copie AST propre est verte. `make ast-guard LIBCLANG=/nonexistent` rend 2 sur `libclang not found`.
- **H-5d-6 — tient.** `test` et `test-all` sont inchangés. Les entrées de légende des trois gardes disent encore « not yet in make test ». Celles d'`old-names-guard` et d'`ast-guard` nomment désormais `tools/cart_names_exceptions.txt` : 5e garde ce texte et ne change que la fin.
- **H-5d-7 — tient.** La ligne 3a de `tools/rename_substeps.tsv` porte `tools/cart_names_exceptions`, `tools/cart_citations_exceptions`, `tools/french_exceptions` et `tests/test_scope_count`. `rename-substeps-check` compte 242 modules, 0 violation. Avec les deux tests de 5e, il en comptera 244.
- **H-5d-8 — tient.** `CLAUDE.md:24` dit `/* GAP-7 */`. La ligne 15 (« Build ») est celle de `4d6571c`.
- **H-5d-9 — tient, une précision.** `french_count.FUNCTION_WORDS`, `rename_count.Names`, `rename_tables.read_table` et `rename_tables.read_triples` ont gardé leurs contrats. *Corrigé* : la liste de mots-clés s'appelle `rename_check.C_KEYWORDS` (59 mots). Les plantes tirées à l'exécution sont, à `c97eb65` :
  - l'ancien nom distinctif `ARENES_MAX` ;
  - le mot français `apres` ;
  - les triplets `tmp/pico8_replay` et `catégorie`.
- **H-5d-10 — tient.** Il reste 24 lignes `fonction` à nom ambigu et à identité `c:<fichier>@F@<nom>`. La première est `at` de `tests/test_button.c`. Mesuré : `static void at(void)` dans la copie donne `tests/test_button` `identifier` 2 (partie AST) et `function` 1 (portée), code 2. Le TSV compte toujours 443 noms ambigus et 1 578 identités renommées.
- **H-5d-11 — tient.** La PRD de 5d est graduée (`c97eb65`). 5d n'a fixé aucune marque « appliqué » ; le registre et le vocabulaire ne sont pas édités (décision 13 de 5d).

Pour 5e, la note de 5d ajoute deux faits, repris aux « Further Notes ». La règle de D-60 vaut au niveau du mot, celle de D-37 au niveau de la déclaration. Les trois fichiers d'exceptions exceptent un mot dans tout un module. Plusieurs entrées du registre des écarts sont signalées pour décision humaine (GAP-3, 4, 9, 11, 23) ; 5e ne les touche pas.

**Mesures faites pour cette PRD** sur un clone jetable au commit `4d6571c`, dans le scratchpad de l'agent, supprimé ensuite. Les bindings libclang et le vendor raylib étaient des liens vers ceux du dépôt. Aucune commande git d'écriture n'a été lancée sur le dépôt. La machine a 4 cœurs.

- **`make test`** : 33,6 s à froid, puis 22,6 à 25,6 s à chaud. **`make e2e`** : 1,2 s.
- **Gardes texte sur l'arbre réel :**
  - `make old-names-guard` : 2,6 s, code 2, `tools/frames_ref` `string` 1 (D-1) ;
  - `make french-guard` : 1,1 s, code 2, `string` 36.

  Ce sont les restes que 5d a levés : à `c97eb65`, les deux gardes rendent code 0 (H-5d-1). `make -k test old-names-guard french-guard` prend 25,3 à 25,4 s, contre 25,3 à 25,6 s pour `make test` seul : sous `-j4`, les deux gardes ne coûtent rien de mesurable. Sans libclang (`env -u PYTHONPATH make old-names-guard french-guard LIBCLANG=/nonexistent AST_RENAME_BINDINGS=/nonexistent`), les mêmes tableaux sortent en 2,7 s.
- **`make ast-guard` sur l'arbre réel** : 138 s, aucune ligne `warning:`, code 2. La partie AST rend 0 ; la portée rend `static` 12 (D-13).
- **`make test e2e ast-guard`**, ce que serait `test-all` sous D-66 (c), lancé en parallèle par le `-j4` du `Makefile` : **150 s**. En série, ce serait 138 + 23 + 1 ≈ 162 s. Seule `ast-guard` est rouge (D-13) ; `test` est vert et `e2e` passe 26 e2e. La note de 5a estimait « d'environ 25 à 33 s à environ 3 min » : mesuré, **environ 2 min 30**.
- **Copie de travail réduite pour les gardes texte**, sous `mini/` : le `Makefile`, `docs/renommage.tsv`, `tools/*.py`, `tools/*_exceptions.txt` et `tools/rename_substeps.tsv`, sans `.git`. Le `Makefile` se lit sans dépôt git : `AST_RENAME_UNITS`, la seule variable qui appelle `git`, est récursive et n'est évaluée que par les recettes AST. Résultats :
  - propre, `make -C mini old-names-guard` donne code 0, `TOTAL` 0, en 0,26 s ;
  - avec `src/game/planet.c` qui contient `/* see <ancien nom> */` et `int <ancien nom>(void)…`, il donne code 2, `game/planet` `identifier` 1 et `comment` 1 ;
  - avec `src/game/planet.c` qui contient un commentaire français (« le compteur de la planète »), `make -C mini french-guard` donne `game/planet` `comment` 4, en 0,08 s.

  La copie propre de `french-guard` était rouge de `string` 4 sur `4d6571c`, venus des `tools/*.py` copiés (D-33, D-61). À `c97eb65`, ils sont exceptés et la copie propre est verte (H-5d-3).
- **Copie de travail réduite pour la garde AST**, avec en plus `tools/ast_rename/` et le lien `third_party/clang-python`, et `AST_RENAME_UNITS` passé sur la ligne de commande :
  - plantes `int qz;` dans `src/game/planet.c` et `static void hq(void) {}`, appelée par `main`, dans `tests/test_background.c` : code 2 en **0,46 s**, extraction comprise. La partie AST donne `tests/test_background` `identifier` 2. La portée donne `game/planet` `global` 1 et `tests/test_background` `function` 1 ;
  - mêmes unités sans plante : code 0, les deux tableaux à 0, aucune ligne `warning:` ;
  - avec `LIBCLANG=/nonexistent` : code 2 sur `ast_rename: libclang not found: /nonexistent` ;
  - avec `AST_RENAME_BINDINGS=/nonexistent` : code 2 sur ``ast_rename: clang Python bindings not found: run `make rename-tool-setup` ``.
- **La moitié AST de l'anti-cart apparie des identités, pas des noms.** La même plante `static void hq(void) {}` dans `tests/test_planet.c` donne une partie AST à **0**. Seule la portée la voit, en `function` 1. L'identité de `hq` au TSV est `c:test_background.c@F@hq` : elle contient le nom du fichier. Dans le TSV à `4d6571c`, il y a 443 noms ambigus et 1 578 identités renommées. Elles se répartissent ainsi : locales 571, champs 517, paramètres 337, statiques 110, fonctions 24, constantes 18, macro 1. 923 de ces identités portent un offset et ne peuvent plus réapparaître. Pour les champs, l'identité contient l'ancien nom du type porteur, renommé depuis.
- **La fraîcheur de l'extraction** se juge sur les seuls `.c`/`.h` de `src/`, `tests/` et `tools/` (`rename_count.py`, `scope_count.newest_source`). Aucune cible de `make test` n'écrit de `.c`/`.h` à ces endroits. `float-guard` réécrit `tools/float_guard.txt` par un `mv`, donc atomiquement. Lancer `ast-guard` en parallèle de `test` ne crée pas de faux « plus vieille que les sources ».

## Condition d'entrée

- Les commits de 5d sont poussés sur la branche du chantier, et sa note « Découvert » est présente sous l'entrée 5d du plan (H-5d-1).
- **La sortie de 5d tient.** Sur une extraction fraîche sans `warning:` :
  - les cinq compteurs sans `SUBSTEP` rendent 0 avec leurs exceptions ;
  - `make old-names-guard`, `make ast-guard` et `make french-guard` rendent **chacune le code 0** sur l'arbre réel.

  Une garde rouge à l'entrée arrête 5e avant le premier commit. 5e ne pose aucune ligne d'exception (décision 11).
- **Lecture des verdicts.** C'est celle que 5a a consignée. Pour D-66, « ok » vaut l'option (c), acceptée en séance : la partie texte de l'anti-cart va dans `make test` ; la partie AST (noms ambigus en identifiant) et la garde de portée vont dans une cible bloquante chaînée à `make test-all`. Pour D-67, « ok » vaut la proposition (a) : le français devient une troisième garde, bloquante dans `make test`. Les lignes `Verdict` du registre ne sont pas éditées.

## Problem Statement

Le §5 du plan veut deux gardes permanentes et bloquantes, et D-67 en ajoute une troisième. 5a les a construites (`old-names-guard`, `ast-guard`, `french-guard`), et 5d les a rendues vertes en levant les restes que l'humain avait jugés. Mais aucune n'est branchée : leur légende dit encore « not yet in make test ». Aujourd'hui, une réintroduction passe `make test` et `make test-all` sans bruit. Ce peut être un ancien nom du TSV, recopié depuis `pico8/lua.lua` par un agent de `fix-bug`, une globale de deux lettres, ou un commentaire écrit en français par habitude. Tout le travail des étapes 3 à 5d n'est alors tenu que par la discipline des agents.

Brancher ne suffit pas. Une garde verte ne prouve pas qu'elle regarde, et le chantier a déjà montré trois façons d'être vert pour rien :
- les gardes à motifs de 5b, épinglées sur des noms qui n'existaient plus, n'appariaient plus rien ;
- un test vert par accident a été vu au rouge joué, en 5b ;
- un fichier d'exceptions pourrait retirer plus que son triplet.

Le plan exige donc qu'une violation plantée, passée par la **vraie cible**, la rende rouge, et que ces violations restent comme tests permanents.

Deux contraintes pèsent sur la forme :
- **Le coût.** La partie AST demande une extraction libclang complète, environ 2 min 15 s. Sous D-66 (c), elle n'entre donc pas dans la boucle rapide.
- **Les machines sans libclang.** Les bindings sont posés par `make rename-tool-setup`, par le réseau, et sont ignorés par git. Sur une telle machine, `make test` doit rester utilisable. `make test-all`, lui, doit échouer franchement, et ne pas sauter la garde en silence.

## Solution

**Une PRD, cinq commits ordonnés**, plus une mesure de départ sans commit. Chaque commit garde `make test-all` vert et `tests/e2e/*.sha256` inchangés.

1. **`test:` — tests de vivacité des deux gardes texte**, `tests/test_text_guards.py` (cible `text-guards-test`, dans `make test`).
   - Le test construit une **copie de travail réduite** sous `tmp/`.
   - Il y lance `make -C <copie> old-names-guard` puis `french-guard`, la vraie recette du vrai `Makefile`, d'abord sur la copie propre (code 0), puis avec chaque violation plantée (code non nul, le module et la colonne de la plante dans le tableau).
2. **`feat:` — `old-names-guard` et `french-guard` dans `make test`.**
3. **`test:` — test de vivacité de la garde AST**, `tests/test_ast_guard.py` (cible `ast-guard-test`, hors suite à ce commit). Il suit le même schéma, avec `make -C <copie> ast-guard AST_RENAME_UNITS="<unités plantées>"`.
4. **`feat:` — `test-all: test e2e ast-guard ast-guard-test`.** Le commit met aussi à jour la légende et la ligne « Build » du `CLAUDE.md`, parce que `make test-all` demande désormais libclang et prend environ 2 min 30.
5. **`docs:` — note « Découvert » de 5e** et relevés de fin. Le §5 du plan y est corrigé pour dire où vit chaque garde (D-66 (c), D-67 (a)).

À la fin :
- `make test` contient `old-names-guard`, `french-guard` et `text-guards-test`. Il reste vert sans libclang ni bindings et ne gagne rien de mesurable en durée.
- `make test-all` = `test` + `e2e` + `ast-guard` + `ast-guard-test`, en parallèle. Il dure environ 2 min 30 sur 4 cœurs et échoue avec un message qui nomme le manque si libclang ou les bindings manquent.
- Chaque violation exigée par la sortie du plan, plus quelques violations voisines (décision 7), rend sa vraie cible rouge à chaque passage de la suite.

## User Stories

1. En tant que mainteneur, je veux que `make test` échoue si un ancien nom distinctif du TSV réapparaît dans le code, que ce soit comme identifiant, entre backticks, nu dans un commentaire ou dans une chaîne, pour que le chantier des étapes 3 à 5d ne se défasse pas en silence.
2. En tant que mainteneur, je veux que `make test` échoue sur un mot français laissé dans un commentaire ou une chaîne du code, pour que le §2 (« plus aucun mot français dans le code ») soit tenu par une machine (D-67 (a)).
3. En tant que mainteneur, je veux que `make test-all` échoue sur un type, une fonction, un champ, une constante ou une globale d'une ou deux lettres, ou sur une locale courte à portée de plus de 40 lignes, pour que la tolérance du §1 ne s'élargisse pas sans décision.
4. En tant que mainteneur, je veux que `make test-all` échoue si une identité renommée d'un nom ambigu réapparaît, pour que la moitié AST de l'anti-cart soit bloquante elle aussi (D-66 (c)).
5. En tant que développeur sans libclang, je veux que `make test` reste vert et complet pour tout ce qui n'est pas l'AST, pour garder la boucle rapide sur une machine neuve.
6. En tant que développeur sans libclang, je veux que `make test-all` échoue avec un message qui dit ce qui manque, pour ne jamais croire la suite complète passée alors que la garde AST n'a pas tourné.
7. En tant que relecteur, je veux que chaque garde soit prouvée vivante par une violation qui passe par sa **vraie cible du `Makefile`**, et pas par l'outil appelé à la main, pour qu'une recette cassée (option `--check` perdue, mauvais fichier d'exceptions, mauvaise racine) se voie.
8. En tant que relecteur, je veux que la même copie, sans plante, soit verte, pour que le rouge vienne de la plante et non de la copie.
9. En tant que relecteur, je veux que le test vérifie le module et la colonne de la plante dans le tableau imprimé, et pas seulement un code non nul, pour qu'une garde rouge pour une autre raison ne fasse pas passer le test.
10. En tant que mainteneur, je veux que ces violations restent comme tests permanents, lancés à chaque `make test` ou `make test-all`, pour qu'une régression de garde se voie au commit qui la cause.
11. En tant que mainteneur, je veux qu'aucune plante ne soit écrite en clair dans le fichier de test, pour que les gardes branchées sur l'arbre réel ne rougissent pas sur leur propre test, et sans exclure ce fichier des gardes.
12. En tant que mainteneur des exceptions, je veux qu'un triplet d'exception ne cache pas le même mot planté dans un autre module, prouvé par la vraie cible, pour que la granularité par module de 5d ne déborde pas.
13. En tant que mainteneur des exceptions, je veux qu'un mot planté dans le module excepté reste vert sous la vraie cible, pour prouver que la recette lit bien le fichier d'exceptions.
14. En tant que développeur, je veux que les gardes texte ne rallongent pas `make test` de façon mesurable, pour que la boucle rapide reste rapide.
15. En tant que développeur, je veux que `make test-all` lance l'extraction en parallèle de `test` et `e2e`, pour payer environ 2 min 30 et pas la somme.
16. En tant que développeur, je veux que les tests de vivacité AST ne refassent pas une extraction complète, pour ne pas doubler le coût de `test-all`.
17. En tant qu'agent qui suit `CLAUDE.md`, je veux que la ligne « Build » dise ce que fait désormais `make test-all`, ce qu'il demande et ce qu'il coûte, pour choisir entre `make test`, `make test e2e` et `make test-all`.
18. En tant que lecteur du `Makefile`, je veux que la légende dise de chaque garde où elle est branchée (« in make test », « in make test-all »), pour ne plus lire « not yet in make test ».
19. En tant que relecteur, je veux que chaque test de vivacité ait été vu rouge, par une mutation de la recette dans la copie, avant d'être commité vert, et que ce rouge joué soit consigné dans le message, pour qu'un test vert par construction ne passe pas.
20. En tant qu'humain au prochain point d'arrêt, je veux lire dans la note ce que les gardes ne voient toujours pas (noms historiques de 5b, ancienne identité dans un autre fichier, mot excepté par module), pour décider s'il faut aller plus loin.
21. En tant qu'agent, je veux que l'appli raylib ne soit jamais lancée : `make raylib` et `make screencast-viewer` sont compilés, jamais exécutés.
22. En tant que `make` parent, je veux que le `make` enfant lancé par un test ne partage ni le jobserver ni les variables de ligne de commande parasites, pour éviter les avertissements de jobserver et les surcharges involontaires.

## Implementation Decisions

### Où vit chaque garde

| Garde | Cible | Branchée dans | Demande libclang | Coût mesuré |
|---|---|---|---|---|
| Anti-cart, moitié texte | `old-names-guard` | `test` | non | 2,6 s, en parallèle de la suite |
| Français (D-67 (a)) | `french-guard` | `test` | non | 1,1 s, idem |
| Vivacité des deux gardes texte | `text-guards-test` | `test` | non | environ 1 à 2 s (une dizaine de passages à 0,1–0,3 s) |
| Anti-cart, moitié AST, et portée | `ast-guard` (prérequis `rename-extract`) | `test-all` | oui | 138 s |
| Vivacité de la garde AST | `ast-guard-test` | `test-all` | oui | environ 2 à 4 s (3 à 5 passages à 0,5 s) |

- `test` garde sa liste, plus `old-names-guard french-guard text-guards-test` à la fin.
- La recette devient `test-all: test e2e ast-guard ast-guard-test`. Ce sont des prérequis : le `-j$(NPROC)` du `Makefile` les lance en parallèle.
- Les recettes des trois gardes ne changent pas en 5e. Leur forme est celle de la fin de 5d (H-5d-3).
- `cart-citations` et `cryptic-locals` **restent des relevés**, non bloquants. D-56 et D-59 le disent (« le relevé n'est pas une garde du §5 »), et ni le §5 ni D-67 n'en font une garde (décision 9).

### Le test de vivacité texte (`tests/test_text_guards.py`)

**Copie de travail réduite** (décision 1), sous `tmp/test_text_guards_work/`, refaite à chaque test. Elle contient :
- le `Makefile` ;
- le fichier lu par `RENAME_TABLE`, au même chemin relatif, avec son chemin relu dans le `Makefile` et pas écrit en dur ;
- `tools/*.py`, `tools/*_exceptions.txt` et `tools/rename_substeps.tsv`.

Elle ne contient ni `src/`, ni `tests/`, ni `.git`. Les plantes y ajoutent leurs fichiers.

**Lancement.** Le test lance `make --no-print-directory -C <copie> <cible>`, dans un environnement d'où `MAKEFLAGS`, `MFLAGS` et `MAKELEVEL` sont retirés (décision 10). Il lit le code de sortie, la sortie standard et la sortie d'erreur.

**Cas, chacun dans sa copie fraîche :**

| Cas | Plante | Cible | Attendu |
|---|---|---|---|
| propre | aucune | `old-names-guard` | code 0, ligne `TOTAL` à 0 partout |
| propre | aucune | `french-guard` | code 0, `TOTAL` à 0 |
| identifiant | `src/game/planet.c` : une fonction nommée d'après un ancien nom distinctif d'une ligne `renommé` `identifiant` | `old-names-guard` | code non nul ; `game/planet` `identifier` 1, rien d'autre |
| commentaire nu | même nom, nu dans un commentaire C | `old-names-guard` | `game/planet` `comment` 1, rien d'autre |
| backtick | même nom entre backticks dans un commentaire C | `old-names-guard` | `game/planet` `backtick` 1, rien d'autre |
| chaîne | même nom dans un littéral C | `old-names-guard` | `game/planet` `string` 1, rien d'autre |
| exception, autre module | le texte du triplet de `tools/cart_names_exceptions.txt` (D-1), dans un littéral de `src/game/planet.c` | `old-names-guard` | `game/planet` `string` 1 |
| exception, son module | le même texte dans le fichier du module excepté (`tools/frames_ref.sh` de la copie) | `old-names-guard` | code 0 |
| français, commentaire | un mot de `french_count.FUNCTION_WORDS` dans un commentaire de `src/game/planet.c` | `french-guard` | `game/planet` `comment` 1 |
| français, chaîne | un mot accentué dans un littéral de `src/game/planet.c` | `french-guard` | `game/planet` `string` 1 |
| exception française, autre module | le mot d'un triplet `string` de `tools/french_exceptions.txt` dans un littéral de `src/game/planet.c` | `french-guard` | `game/planet` `string` 1 |

**Choix des plantes à l'exécution** (décision 3). Aucune plante n'est écrite en clair dans le fichier de test.
- L'ancien nom est pris dans la table : le premier, dans l'ordre trié, des `rename_count.Names(rows).distinctive_words`, lus par `rename_tables.read_table`.
- Le mot français est le premier mot d'au moins quatre lettres, dans l'ordre trié, de `french_count.FUNCTION_WORDS`.
- Le mot accentué est construit par `chr` sur un point de code.
- Les triplets d'exception sont lus par `rename_tables.read_triples` dans les deux fichiers.

Le test vérifie ses propres hypothèses. Le nom choisi n'est ni conservé ni nom neuf ailleurs. Le mot français n'est excepté dans aucun module. Si une hypothèse tombe, le test échoue avec un message qui dit laquelle, et ne saute pas.

**« Rien d'autre »** veut dire : une seule ligne de module dans le tableau, et les autres colonnes de cette ligne à 0. Une plante ne doit allumer que sa colonne.

### Le test de vivacité AST (`tests/test_ast_guard.py`)

**Copie de travail réduite**, sous `tmp/test_ast_guard_work/`. Elle contient ce que contient la copie texte, plus `tools/ast_rename/`. Les bindings ne sont pas copiés. Le test passe sur la ligne de commande du `make` enfant :
- `AST_RENAME_UNITS="<les unités de la copie>"` (décision 2) ;
- `AST_RENAME_BINDINGS` et `LIBCLANG`, reçus de la recette `ast-guard-test` du `Makefile` réel, qui les lui donne par l'environnement.

**Cas :**

| Cas | Unités de la copie | Attendu de `make -C <copie> ast-guard` |
|---|---|---|
| propre | `src/game/planet.c` (une fonction au nom anglais), `tests/test_planet.c` (`main` vide) | code 0, les deux tableaux à 0, aucune ligne `warning:` |
| globale de deux lettres (sortie du plan) | `src/game/planet.c` déclare `int <xy>;` | portée : `game/planet` `global` 1, rien d'autre ; partie AST : `TOTAL` 0 ; code non nul |
| ancienne identité (moitié AST) | au chemin du `fichier` d'une ligne `renommé` `identifiant` `fonction` à nom ambigu et à identité de fonction statique (`c:<fichier>@F@<nom>`) : `static void <nom>(void) {}`, appelée par `main` | partie AST : le module de ce fichier en `identifier` ≥ 1 ; code non nul |
| sans libclang | propre, `LIBCLANG=/nonexistent` | code non nul, `libclang not found` sur la sortie d'erreur, aucun tableau |

- **`<xy>` est choisi à l'exécution.** C'est la première paire de deux lettres minuscules, dans l'ordre, qui n'est ni un ancien nom de la table, ni un nom neuf, ni un mot-clé C (`rename_check.C_KEYWORDS`, 59 mots).
- **La ligne de la table**, pour le cas « ancienne identité », est la première dans l'ordre trié. À `4d6571c`, il y en a 24 : `at` de `tests/test_button.c`, `hq`, `gx`… Le test lit le nom et le fichier dans la table, jamais en clair.
- Les unités plantées compilent sous les `CFLAGS` du `Makefile` (`-Wall -Wextra -Werror`). Une fonction `static` est appelée, une globale est lue, pour qu'aucun avertissement n'empêche l'analyse.

### Recettes et légende du `Makefile`

- `text-guards-test` : `python3 tests/test_text_guards.py`.
- `ast-guard-test` : `$(AST_RENAME_ENV) python3 tests/test_ast_guard.py`. Le test relit `LIBCLANG` et `AST_RENAME_BINDINGS` dans son environnement et les passe au `make` enfant. Pour que le test les trouve, la recette exporte `AST_RENAME_BINDINGS=$(AST_RENAME_BINDINGS)`.
- Les deux cibles vont dans `.PHONY`.
- **Légende**, en anglais, sans le mot `cart` entier, sans `Lua` et sans mot français : `cart-citations` et `french-count` lisent le `Makefile`.
  - `make test` cite `old-names-guard` et `french-guard` parmi ses parties ;
  - `make test-all -> test + e2e + ast-guard + ast-guard-test, the full suite; needs libclang and make rename-tool-setup, about 2 min 30` ;
  - les trois entrées de garde perdent « not yet in make test » au profit de « in make test » ou « in make test-all » ;
  - deux entrées neuves, `text-guards-test` et `ast-guard-test`, disent : « planted violations through the real target, in make test » et « … in make test-all ».
- **`tools/rename_substeps.tsv`** : `tests/test_text_guards` et `tests/test_ast_guard` vont dans la liste de modules de 3a, à côté de `tests/test_scope_count` (décision 8). Sinon, `french-count` avertit d'un module hors sous-étape (mesuré par 5d pour ses fichiers neufs).

### `CLAUDE.md`, ligne « Build »

C'est la seule ligne du `CLAUDE.md` que 5e touche (décision 6). Elle dit :
- `make test` est la boucle rapide (TU, contrôles statiques et gardes texte, sans libclang) ;
- `make e2e` dure environ 1 s ;
- `make test-all` fait les deux, plus la garde AST : extraction libclang d'environ 2 min 30, qui demande `make rename-tool-setup` ;
- `make test e2e` donne l'ancienne paire rapide.

La légende reste la référence.

### Pièges

- **Ne pas exclure les fichiers de test des gardes** (`SKIPPED_FILES`, `OWN_FILES`). Ce serait un angle mort permanent : les plantes sont construites à l'exécution (décision 3).
- **Ne pas nommer les tests `tests/test_rename_*.py`.** Ce motif est exclu des compteurs par `SKIPPED_FILES` : le fichier sortirait des gardes sans qu'on le voie.
- **Les copies de travail vivent sous `tmp/`.** Les gardes de l'arbre réel ne parcourent que `src/`, `tests/`, `tools/` et le `Makefile`. Une copie sous `tests/` serait comptée.
- **`ast-guard` et `ast-guard-test` écrivent dans deux arbres distincts.** Ce sont `tmp/ast_rename/` et `tmp/test_ast_guard_work/tmp/ast_rename/` : lancées en parallèle, elles ne se marchent pas dessus.
- **Ne pas lancer `make` enfant avec le `MAKEFLAGS` du parent.** Il transporte le jobserver et les variables de ligne de commande, comme un `AST_RENAME_UNITS` ou un `MODULES` tapé par l'utilisateur.

### Décisions prises sans humain

1. **« Copie de travail » = copie réduite**, et non copie de tout l'arbre.
   - La copie réduite contient ce que la recette lit : le `Makefile`, la table, les outils et les exceptions, plus les fichiers plantés.
   - Raisons :
     - la recette et les outils sont les vrais ;
     - la copie propre verte prouve que le rouge vient de la plante ;
     - l'arbre réel est déjà jugé par la garde elle-même dans la même suite ;
     - une copie complète coûterait un passage de garde de plus sur 3,5 Mo de sources (2,6 s) ;
     - pour l'AST, elle coûterait une extraction complète de plus (138 s) ;
     - elle lirait `src/`, `tests/` et `tools/` pendant que d'autres cibles de `make test` écrivent.
   - Le plan dit « une copie de travail » sans dire laquelle. La note le rappelle.
2. **`AST_RENAME_UNITS` surchargé pour la copie AST.** C'est le seul écart à « la vraie cible ». Il est forcé : la copie n'a pas de `.git`, et la variable appelle `git ls-files`. Il est aussi voulu : 0,46 s au lieu de 138 s. La recette `ast-guard`, son prérequis `rename-extract`, les configurations et les deux outils restent ceux du `Makefile` réel.
3. **Les plantes sont tirées de la table et des outils à l'exécution**, et ne sont jamais écrites en clair. Un ancien nom écrit dans `tests/test_text_guards.py` rougirait `old-names-guard` sur l'arbre réel. Un mot français rougirait `french-guard`. L'alternative, exclure le fichier des gardes, ouvrirait un angle mort.
4. **Deux fichiers de test et deux cibles**, plutôt qu'un fichier à option : l'un tourne sans libclang dans `make test`, l'autre avec dans `make test-all`. Le découpage suit D-66 (c).
5. **Sans libclang, `make test-all` échoue ; il ne saute rien.** C'est le contrôle de D-66 sous (c) : « la cible AST, sans libclang, échoue ». Aucun mécanisme de saut n'est ajouté. Les messages d'absence sont ceux de `tools/ast_rename/bindings.py`, inchangés. Celui des bindings nomme `make rename-tool-setup`. Celui de libclang nomme le chemin, car libclang 18 est un paquet système que la cible ne pose pas.
6. **5e touche la ligne « Build » du `CLAUDE.md`**, alors que les docs et consignes relèvent de 5f. 5e change ce que fait `make test-all` : la consigne qui le décrit passe avec lui, comme les skills passaient avec `TROU` en 5d. Le reste du `CLAUDE.md` (arborescence) reste à 5f. `gh-release` dit « `make test-all` passes », ce qui reste vrai ; je ne le touche pas.
7. **Plantes au-delà de la lettre du plan.** Le plan exige une globale de deux lettres, un ancien nom dans un commentaire et dans un identifiant, et un mot français dans un commentaire. J'ajoute :
   - une plante par colonne de l'anti-cart (`backtick`, `string`) ;
   - le mot français en chaîne ;
   - deux preuves de granularité des exceptions par la vraie cible (même mot dans un autre module : rouge ; dans le module excepté : vert) ;
   - l'ancienne identité de la moitié AST.

   Chacune coûte moins d'une demi-seconde. Sans la dernière, la moitié AST de l'anti-cart n'aurait aucune preuve de vivacité.
8. **Modules des tests neufs en 3a** de `tools/rename_substeps.tsv`, avec les tests des gardes (`tests/test_scope_count`), comme 5d le fait pour ses fichiers d'exceptions.
9. **`cart-citations` et `cryptic-locals` ne deviennent pas des gardes.** Les verdicts de D-56 et D-59 les disent « pas une garde du §5 », et rien ne les demande en garde. Leur 0, atteint en 5d, n'est plus tenu que par le relevé. La note le dit pour l'humain.
10. **Le `make` enfant est lancé avec un environnement propre** : sans `MAKEFLAGS`, `MFLAGS` ni `MAKELEVEL`, et avec les seules variables que le test passe explicitement.
11. **5e ne pose aucune ligne d'exception et ne change aucun outil de garde.** Si une garde est rouge à l'entrée, 5e s'arrête et la question va à l'humain. Seule une décision humaine ajoute une ligne. 5e ne touche ni `tools/rename_count.py`, ni `scope_count.py`, ni `french_count.py`, ni leurs tests.
12. **Rouge joué par mutation**, puisque les gardes existent déjà. Avant chaque commit `test:`, le test est lancé contre une copie dont la recette est neutralisée : `--check` retiré, `--exceptions` pointé vers un fichier qui excepte tout, ou `--part ast` changé en `--part text`. Le test doit être rouge, et seulement sur les cas que la mutation concerne. Le rouge est consigné dans le message. Le script de mutation vit dans `tmp/5e_mutation/` et n'est pas commité, comme en 5b. Il copie `Makefile`, `tools/` et `tests/` dans un répertoire jetable, mute la recette dans cette copie, et y lance le test. Le test n'a donc aucun crochet de mutation.
13. **Ordre des commits.** Le test texte vient avant le branchement texte, et le test AST avant le chaînage AST : la vivacité est prouvée avant que la garde ne bloque.
14. **Le §5 du plan est corrigé** dans le commit de la note. « rendues bloquantes dans `make test` à l'étape 5 » devient : la moitié texte et le français dans `make test`, la moitié AST et la portée dans `make test-all`, par D-66 (c) et D-67 (a). La règle du plan le permet : un constat qui contredit le plan le corrige dans le même commit. Le changement est déjà décidé par l'humain (D-66 (c)).
15. **Le registre et le vocabulaire ne sont pas édités.** Pas de marque « appliqué ».

## Implementation Chunks

0. **Mesure de départ**, sans commit, dans `tmp/5e_baseline/`, sur l'arbre de fin de 5d :
   - `make rename-extract`, sans `warning:` (4 `partial`) ;
   - les cinq compteurs sans `SUBSTEP` à 0 ;
   - `make old-names-guard`, `make ast-guard` et `make french-guard` à code 0, tableaux gardés ;
   - `make test` et `make test-all` verts, durées relevées ;
   - `make test LIBCLANG=/nonexistent AST_RENAME_BINDINGS=/nonexistent` vert ;
   - `class-coverage-net` et `e2e-gap`, sorties gardées.

   Si tout est égal à la sortie de 5d, on ouvre. Sinon, décision 11.
1. **`test:`** — `tests/test_text_guards.py`, cible `text-guards-test` dans `make test`, `.PHONY`, entrée de légende, module en 3a. Rouge joué par mutation (décision 12).
2. **`feat:`** — `old-names-guard` et `french-guard` dans les prérequis de `test`. Légende : « in make test ». Contrôles :
   - `make test` vert ;
   - `make test LIBCLANG=/nonexistent AST_RENAME_BINDINGS=/nonexistent` vert ;
   - durée de `make test` à ± 1 s de la mesure 0.
3. **`test:`** — `tests/test_ast_guard.py`, cible `ast-guard-test`, hors suite à ce commit, `.PHONY`, entrée de légende, module en 3a. Rouge joué par mutation.
4. **`feat:`** — `test-all: test e2e ast-guard ast-guard-test`, légende de `test-all` et des deux cibles AST, ligne « Build » du `CLAUDE.md`. Contrôles de D-66 (c) :
   - `make test-all` vert, durée relevée ;
   - `make test-all LIBCLANG=/nonexistent` rend un code non nul, avec `libclang not found` ;
   - `make test-all AST_RENAME_BINDINGS=/nonexistent` rend un code non nul, avec `make rename-tool-setup` ;
   - `make test` reste vert dans ces deux cas.
5. **`docs:`** — note « Découvert » de 5e sous l'entrée 5e du plan, avec la correction du §5 (décision 14). La note contient :
   - les cinq commits et le rouge joué de chacun des deux tests ;
   - les décisions 1 à 15 ;
   - les coûts mesurés (`make test`, `make test-all`, chaque test de vivacité) ;
   - ce que les gardes ne voient toujours pas (§ « Further Notes ») ;
   - tout écart à une hypothèse `H-5d-k` ;
   - les relevés de fin.

**Total : 5 commits.**

## Testing Decisions

- **Un bon test juge une sortie observable** : le code de sortie de `make`, la ligne du tableau imprimé pour le module planté et la colonne attendue, la sortie d'erreur pour les absences. Il ne juge jamais le texte de la recette ni le code de l'outil.
- **Chaque cas a sa copie fraîche**, et chaque cas rouge a son jumeau vert : la copie propre, ou le module excepté. Un test qui ne verrait que le rouge passerait avec une garde toujours rouge ; un test qui ne verrait que le vert passerait avec une garde toujours verte.
- **La vraie cible** : le test lance `make -C <copie> <cible>` sur une copie du vrai `Makefile`, jamais l'outil Python à la main.
- **Prior art** :
  - `tests/test_scope_count.py` et `tests/test_rename_count.py` : arbre jetable sous `tmp/test_*_work`, boîte noire par `subprocess`, codes de sortie, et messages d'absence (`test_…` qui cherchent `make rename-extract` dans la sortie d'erreur) ;
  - les lignes de vivacité de 5b (`tests/test_composition_residue.c`, `tests/test_data_residue.c`) : une violation plantée par catégorie gardée, contrôle de la variable de garde lue dans le `Makefile` ;
  - la preuve par mutation de 5b (`tmp/5b_mutation/run.py`).
- **Aucun test d'outil n'est modifié.** `tests/test_rename_count.py`, `tests/test_scope_count.py`, `tests/test_french_count.py` et `tests/test_cart_citations.py` restent tels que 5d les laisse.

**Relevés de fin** (sortie de 5e et invariant du §5) :
- `make test-all` vert, durée relevée ; `tests/e2e/*.sha256` inchangés ;
- `make test` vert avec et sans libclang ni bindings ;
- `make test-all` rouge sans libclang, et rouge sans bindings, avec le message de chaque absence ;
- sur l'extraction fraîche laissée par `make test-all`, les cinq compteurs sans `SUBSTEP` à 0 ;
- `make raylib` et `make screencast-viewer` compilent et linkent, jamais lancés ;
- `class-coverage-net` et `e2e-gap` : sorties égales à la mesure 0, aux numéros de ligne du `Makefile` près ;
- `rename-tool-test`, les `rename-*-test`, `scope-count-test`, `french-count-test`, `cart-citations-test`, `text-guards-test`, `ast-guard-test` et `rename-substeps-check` verts.

## Out of Scope

- **Poser ou retirer une ligne d'exception, changer une règle d'un compteur** : c'est 5d. 5e s'arrête si une garde est rouge à l'entrée.
- **Faire voir à l'anti-cart ce qu'elle ne voit pas :**
  - les noms historiques sans ligne au TSV (5b, *La garde anti-cart ne voit pas les noms historiques*) ;
  - une ancienne identité réintroduite sous un autre fichier ou un autre type ;
  - un ancien nom ambigu de trois lettres ou plus en minuscules, réintroduit comme locale.

  Chacun est noté pour l'humain (§ « Further Notes »). Les traiter demanderait une décision.
- **Rendre `cart-citations` ou `cryptic-locals` bloquants** (décision 9).
- **Accélérer l'extraction** (cache par unité, D-66 (a)), ou versionner un artefact réduit (D-66 (b)) : ce sont les options non retenues de D-66.
- **Un `SessionStart` qui installe les bindings**, ou toute autre mise en place d'environnement.
- **Le reste du `CLAUDE.md`, le `README`, les skills et la suppression de `docs/glossaire-noms.md`** : c'est 5f (D-65).
- Éditer le registre, le tableau « Restes comptés » ou `docs/vocabulaire.md`.
- Lancer `rename-tool-setup`, `e2e-baseline` ou l'appli raylib.

## Hypothèses sur 5d

Je les ai écrites à `4d6571c`, 5d non faite, d'après sa PRD (`docs/prd/next/phase4-etape5d-exceptions-gap.md`).  Relues à `c97eb65` contre la note de 5d (`88efe31`) : toutes tiennent, voir le paragraphe « Relu contre la note « Découvert » de 5d » en tête. Chacune se vérifie sur la note « Découvert » de 5d et sur l'arbre de sa fin. Une hypothèse fausse fait relire les sections nommées avant d'ouvrir 5e.

- **H-5d-1** — 5d est poussée et sa note « Découvert » existe. Sa sortie tient : sur une extraction fraîche sans `warning:`, les cinq compteurs sans `SUBSTEP` rendent 0 avec leurs exceptions, et `old-names-guard`, `ast-guard` et `french-guard` rendent le code 0 sur l'arbre réel. *À relire* : Condition d'entrée ; Implementation Chunks, point 0 ; décision 11.
- **H-5d-2** — Les fichiers d'exceptions sont ceux que la PRD de 5d nomme, au format triplet de `rename_tables.read_triples` : deux lignes `#` d'en-tête, puis `colonne<TAB>module<TAB>nom`. Ce sont :
  - `tools/cart_names_exceptions.txt`, avec la seule ligne de D-1, `string	tools/frames_ref	tmp/pico8_replay` ;
  - `tools/cart_citations_exceptions.txt`, avec les lignes de D-56 et de la part `short` de D-59 ;
  - `tools/french_exceptions.txt`, avec 17 lignes de plus (D-33, D-55, D-59, D-61), dont `string	Makefile	du`.

  Les copies réduites prennent `tools/*_exceptions.txt` par motif. Les cas « exception » lisent le premier triplet de `tools/cart_names_exceptions.txt` et le premier triplet `string` de `tools/french_exceptions.txt`. À `c97eb65`, ce sont `tmp/pico8_replay` et `catégorie`, et non `du` (relu : tient, précisé). *À relire* : Le test de vivacité texte (copie, tableau des cas, choix des plantes) ; Pièges.
- **H-5d-3** — Les recettes des gardes ont la forme suivante :
  - `old-names-guard` et l'appel `rename_count.py` d'`ast-guard` passent `--exceptions tools/cart_names_exceptions.txt` ;
  - `rename_count.py` saute le fichier passé en `--exceptions` et applique les exceptions aux deux moitiés et à `--check` ;
  - `french-guard` est inchangée ;
  - le statut d'`ast-guard` est toujours accumulé sans tube, et son prérequis est toujours `rename-extract` ;
  - aucune recette ne lit un fichier hors de `tools/`, du `Makefile` et de la table, en dehors de l'extraction.

  Si une recette lit un autre fichier, la copie réduite le prend, et la copie propre rouge le dira. *À relire* : Où vit chaque garde ; Le test de vivacité texte (contenu de la copie) ; Le test de vivacité AST ; décision 1.
- **H-5d-4** — La règle de D-13 (une `static` déclarée dans une fonction se juge comme une locale) ne touche pas les globales. Une `int <xy>;` de portée fichier reste refusée en colonne `global`. Les règles de D-37 et D-60 ne vivent que sous `--cryptic`, qui n'est pas une garde. *À relire* : Le test de vivacité AST (cas « globale de deux lettres »).
- **H-5d-5** — `AST_RENAME_UNITS` reste une variable récursive (`=`), surchargeable sur la ligne de commande. C'est la seule variable du `Makefile` qui appelle `git` à l'évaluation, et seules les recettes AST l'évaluent. Le `Makefile` se lit donc sans dépôt git pour `old-names-guard` et `french-guard`. *À relire* : décision 2 ; Le test de vivacité AST ; mesures (copie réduite).
- **H-5d-6** — 5d ne change ni les prérequis de `test` et `test-all`, ni les entrées de légende des trois gardes, qui disent encore « not yet in make test ». 5d ne change que les lignes de légende des outils qu'elle modifie (`cart-names-count`, `cart-citations`, `cryptic-locals`, `scope-count`). *À relire* : Recettes et légende du `Makefile` ; Implementation Chunks, points 2 et 4.
- **H-5d-7** — Les fichiers d'exceptions neufs de 5d sont dans la ligne 3a de `tools/rename_substeps.tsv`, à côté de `tools/french_exceptions` et de `tests/test_scope_count`. 5e y ajoute ses deux modules de test. *À relire* : décision 8.
- **H-5d-8** — Le passage `TROU` → `GAP` ne touche que des commentaires, un message d'`assert`, `tools/reachability_questions.txt`, les docs et les skills. La ligne « Build » du `CLAUDE.md` (ligne 15 à `4d6571c`) n'est pas touchée ; seule la ligne 24 l'est (`/* GAP-7 */`). `GAP` n'est ni un mot français ni un ancien nom (contrôle de 5d). *À relire* : `CLAUDE.md`, ligne « Build » ; décision 6.
- **H-5d-9** — `french_count.FUNCTION_WORDS`, `rename_count.Names` (`distinctive_words`), `rename_tables.read_table` et `read_triples`, et la liste des mots-clés de `rename_check` gardent leurs noms et leurs contrats. Les tests neufs s'en servent pour tirer leurs plantes. 5d ne change que les entrées et les filtres des outils. *À relire* : Le test de vivacité texte (choix des plantes) ; Le test de vivacité AST (`<xy>`, ligne de la table).
- **H-5d-10** — La table garde au moins une ligne `renommé` `identifiant` `fonction` à nom ambigu et à identité de fonction statique `c:<fichier>@F@<nom>` : 24 à `4d6571c`. 5d n'en ajoute ni n'en retire, puisque 5d ne renomme pas. *À relire* : Le test de vivacité AST (cas « ancienne identité »).
- **H-5d-11** — La PRD de 5d est graduée avant que 5e ne commence. 5d ne fixe aucune convention de marquage du registre. Si elle en fixe une, 5e la suit dans son commit `docs:`. *À relire* : décision 15 ; Implementation Chunks, point 5.

## Further Notes

- **Le coût, en clair.** `make test` ne change pas de durée de façon mesurable (25,3 s contre 25,4 s sur 4 cœurs). `make test-all` passe d'environ 24 s à environ **2 min 30** (150 s mesurés, contre 162 s en série), presque tout pour l'extraction.
  - L'invariant du §5 demande `make test-all` à chaque commit. À partir de 5e, chaque commit de 5f coûte donc environ 2 min 30 de plus.
  - Pour la boucle de travail, `make test` reste la référence. `make test e2e` rend l'ancienne paire en environ 25 s.
  - `make test-all` laisse une extraction fraîche dans `tmp/ast_rename/` : les compteurs lus juste après n'ont pas besoin d'un `rename-extract` à part.
- **Ce que les gardes bloquantes ne voient toujours pas** (pour l'humain, au prochain point d'arrêt) :
  - **les noms historiques** sans ligne au TSV (`pico8_value_t`, `pico8_mv`, `hp_pose`…), relevés par 5b. L'alerte 14 de D-68 s'appuyait sur une garde qui ne les voit pas ;
  - **une ancienne identité ailleurs**. La moitié AST apparie l'identité, pas le nom : mesuré, `static void hq(void)` dans `tests/test_planet.c` n'est pas vu, alors qu'il l'est dans `tests/test_background.c`. Un nom court réintroduit comme type, fonction, champ, constante ou globale est rattrapé par la garde de portée ; un nom ambigu de trois lettres ou plus réintroduit ailleurs ne l'est pas ;
  - **un mot excepté par module.** Les exceptions de 5d valent pour un module entier : un `du` neuf dans le `Makefile` ou un `tmp/pico8_replay` neuf dans `tools/frames_ref.sh` passeront ;
  - **les deux relevés.** `cart-citations` et `cryptic-locals` ne bloquent rien (décision 9). Les pièges laissés par 5c (`fy`, `kv`, `nj`, `oc`, `gy` en `short` ; `` `mem` ``, `` `gfx` `` en backtick) sont vus par la seule garde anti-cart pour la part backtick, et par aucune garde pour la part `short`.
- **Sans libclang.** Une session neuve (cloud, `fix-bug` lancé ailleurs) n'a pas les bindings : `third_party/clang-python` est ignoré par git et posé par le réseau. Elle garde `make test` et `make e2e`. `make test-all` lui échoue en nommant `make rename-tool-setup`. C'est le prix de D-66 (c), voulu par l'humain. Un hook de démarrage qui installerait les bindings est hors périmètre.
- **Pourquoi pas une copie complète de l'arbre ?** Je l'ai écartée (décision 1) pour trois raisons :
  - elle ne prouverait rien de plus que la garde elle-même, branchée sur l'arbre réel dans la même suite ;
  - elle coûterait une extraction complète de plus pour l'AST ;
  - elle demanderait un `.git` ou une surcharge d'`AST_RENAME_UNITS` de toute façon.

  Si l'humain la veut, le changement est local aux deux tests : le contenu de la copie et la valeur d'`AST_RENAME_UNITS`.
