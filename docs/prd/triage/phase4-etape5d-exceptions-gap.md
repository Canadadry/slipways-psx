---
title: "Plan 5, sous-étape 5d — lignes d'exception des neuf entrées X (un commit chacune, test d'abord quand un outil change), D-64, puis TROU → GAP : docs/trous-de-portage.md réécrit en noms C, puis clés et marqueurs renommés d'un seul commit"
description: "Après 5c, les cinq compteurs sans SUBSTEP ne rendent plus que les restes que le registre a jugés (D-1, D-13, D-33, D-37, D-55, D-56, D-59, D-60, D-61) ; les lever par une ligne d'exception ou par la règle d'outil de chaque proposition, test d'abord, jusqu'à 0 sur une extraction fraîche ; retirer le marqueur de D-64 ; puis réécrire docs/trous-de-portage.md avec les noms C d'après 5c et passer TROU-N à GAP-N dans le code, les docs et les skills, écrivain et lecteurs dans le même commit."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6, à `19d45d7`)

> 5. **Clôture.** Condition d'entrée : chaque entrée de `docs/decisions-plan-5.md` porte un verdict humain, pris dans l'ensemble fermé du registre (« Conventions » : `proposition`, `variante nommée`, `garder tel quel`, `reporter hors plan 5`), l'alerte 14 de D-68 comprise. Une PRD par sous-étape, dans l'ordre 5a–5f ; chacune applique les entrées de sa famille (colonne « Application » de l'index du registre) et finit par son contrôle de sortie, avec l'invariant du §5. Une entrée `garder tel quel` ou `reporter hors plan 5` n'édite rien, sauf la ligne d'exception d'un reste compté (5d).
>
> 5d. **Lignes d'exception, puis `TROU` → `GAP`** (familles X et T). D'abord les entrées X (D-1, D-13, D-33, D-37, D-55, D-56, D-59, D-60, D-61) et toute entrée à reste compté jugée `garder tel quel` ou `reporter hors plan 5` : une ligne d'exception, ou la règle d'outil qui en tient lieu, test d'abord. Puis D-64, avant le passage. Puis `TROU-N` → `GAP-N` dans le code et comme clé dans `docs/trous-de-portage.md`, qui reste en français mais décrit chaque écart avec les nouveaux noms C ; le nom Lua n'apparaît qu'en tête de chaque GAP. Chaque lecteur du texte `TROU` passe avec son écrivain, dans le même commit. Mesuré à `1b4b949` : 62 lignes `TROU` dans 48 fichiers de `src/`, `tests/`, `tools/` (les marqueurs `/* TROU-N */`), dont trois textes lus ou imprimés hors marqueur — `tools/constants_expected_gaps.txt:1` (D-64), le message d'`assert` de `tests/test_str.c:91` (« TROU-22: … »), `tools/reachability_questions.txt:2` ; hors code, `docs/trous-de-portage.md` (33 mentions), le renvoi `TROU-7` → `GAP-7` de `CLAUDE.md:24`, `README.md:7` et `:23`, les skills qui enseignent le marqueur (`fix-bug/SKILL.md:98` ; `reprendre-les-commentaires` : `SKILL.md`, `consigne-agent.md`, `exemple-travaille.md`, `modele-rapport.md`). Aucun outil ne lit le motif.
>
>    **Sortie** : avec leurs exceptions, les cinq compteurs sans `SUBSTEP` (`french-count`, `cart-names-count`, `cart-citations`, `cryptic-locals`, `scope-count`) rendent 0 sur une extraction fraîche (`cart-citations`, qui n'a pas de fichier d'exceptions, en reçoit un, test d'abord, si un verdict y laisse un reste) ; `git grep -n 'TROU'` rend 0 hors du plan, du registre, de `docs/vocabulaire.md` (qui énonce la règle) et de `docs/glossaire-noms.md` (supprimé en 5f) ; `make constants-check` et `make test-all` verts, `tests/e2e/*.sha256` inchangés.

Phase 4 du pilotage (tronçon 4 : étape 5). 5d est la quatrième sous-étape de l'entrée 5, après 5a (famille O, close), 5b (famille G, close) et 5c (famille N), avant 5e (gardes bloquantes) et 5f (docs et skills).

**Entrées lues**, toutes au commit `19d45d7` :
- `CLAUDE.md` ;
- `docs/plan-5.md` : §1 à §5 (dont « Les étiquettes `TROU-N` deviennent `GAP-N` » du §4), l'entrée 5 et 5a à 5f, les notes « Découvert » de 5a et de 5b, « Fin de l'étape 4 » ;
- `docs/decisions-plan-5.md` : « Conventions » (familles, verdicts, règle des restes comptés), l'index, les entrées X (D-1, D-13, D-33, D-37, D-55, D-56, D-59, D-60, D-61), T (D-64), D-66, D-67, la « Clôture (4l4) » et son tableau « Restes comptés » ;
- `docs/trous-de-portage.md`, `docs/vocabulaire.md` (ligne de `constants_expected_gaps.txt`, « `TROU-N` → `GAP-N` ») ;
- `tools/rename_count.py`, `tools/scope_count.py`, `tools/cart_citations.py`, `tools/french_count.py`, `tools/rename_tables.py`, `tools/rename_strings.py` (`SKIPPED_FILES`), `tools/rename_substeps.tsv`, `tools/french_exceptions.txt`, `tools/scope_exceptions.txt`, `tools/constants_expected_gaps.txt`, `tools/constants_check.sh`, `tools/reachability_questions.txt` ; `tests/test_rename_count.py`, `tests/test_scope_count.py`, `tests/test_cart_citations.py`, `tests/test_french_count.py` (leurs aides et leurs tests d'exceptions) ; les cibles et la légende du `Makefile` (`test`, `cart-names-count`, `scope-count`, `cryptic-locals`, `cart-citations`, `french-count`, `old-names-guard`, `ast-guard`, `french-guard`, `constants-check`) ;
- les PRD de 5a (`git show 4bb53ec`), de 5b (`git show 7601b2b:docs/prd/triage/phase4-etape5b-gardes-a-motifs.md`) et de 5c (`git show b9e4873:docs/prd/triage/phase4-etape5c-renommages.md`, retirée de `triage/` par `4d6571c`), pour la forme et pour ce que 5c doit laisser.

**Aucun humain n'était joignable** pendant la rédaction. Là où `to-prd` demande de vérifier avec l'utilisateur (modules, tests, découpage), l'agent rédacteur a tranché seul. Chaque décision est écrite au § « Décisions prises sans humain », pour être relue au prochain point d'arrêt. La PRD est écrite dans `docs/prd/next/`, pas dans `triage/` : 5c est en cours d'implémentation dans l'arbre.

**5c n'est pas faite au moment de la rédaction.** Tout ce que cette PRD suppose de son issue est écrit en hypothèse `H-5c-k` (§ « Hypothèses sur 5c »), avec les sections à relire quand la note « Découvert » de 5c existera.

**Relu contre la note « Découvert » de 5c (`b9e4873`)**, et contre le code à `4d6571c` sur un clone jetable (extraction fraîche : 2 min 15 s, aucune ligne `warning:`, 4 `partial`). De `19d45d7` à `4d6571c`, 5c a touché `src/`, `tests/`, `docs/renommage.tsv`, `tools/rename_overrides.tsv`, `tools/rename_substeps.tsv` (ligne 3f), `tools/composition_residue.sh` (D-44), `tools/float_grep_exclusions.txt` et deux lignes du `Makefile` (D-2), puis a retiré sa PRD de `triage/`.
- **H-5c-1** — tient. Cliquet de fin égal au tableau, module par module et aux mêmes sites : `cart-names-count` `TOTAL` 0/0/1/0 (`tools/frames_ref`) ; `french-count` `string` 36 (mêmes sept modules) ; `cart-citations` 6/2/7 (`Makefile:151`, `:201`, `:203`, `:205`, `:206`, et les sept en-têtes du relevé figé, mêmes lignes) ; `cryptic-locals` 12 (dix `argc`/`argv`, `blank`, `screen_texture`) ; `scope-count` 12 `static`, les mêmes douze déclarations. Aucun reste neuf : la décision 11 ne joue pas.
- **H-5c-2** — tient. Aucun outil compteur, aucun de leurs tests, ni `french_exceptions.txt`, `scope_exceptions.txt`, `constants_expected_gaps.txt`, `reachability_questions.txt` n'a bougé ; dans le `Makefile`, seuls le `-D` de `COVERAGE_CFLAGS` et son commentaire (`CLASS_COVERAGE_BUILD`), hauteur constante : les numéros de ligne du `Makefile` cités ici valent encore.
- **H-5c-3** — corrigé. Les anciens noms courts créés par 5c sont `fy`, `kv`, `nj`, `oc` et `gy` (D-29, D-31, D-32), pas seulement `fy`, `kv`, `gy` ; `id` en était déjà un (cas de D-47, que le cliquet a vu). Aucun ne change les restes : `texture`, `blank`, `argc`, `argv` restent non validés (`screen` l'est), et les jetons `short` du relevé figé restent `b`, `c`, `d`, `e`, `f`. Conséquence ajoutée aux Règles communes : aucun commentaire écrit par 5d dans le code (en-têtes des fichiers d'exceptions, légende du `Makefile`, tests) ne cite nu un ancien nom de deux lettres ; `docs/` n'est pas lu par les compteurs.
- **H-5c-4** — tient. 62 lignes `TROU` dans 48 fichiers, mêmes numéros (1 à 3, 5, 8 à 10, 12 à 17, 21 à 26, mêmes effectifs) ; les 28 fichiers à marqueur que 5c a touchés ont gardé leur hauteur (339 lignes ajoutées, 339 retirées) ; `tests/test_str.c:91`, `tools/reachability_questions.txt:2` et `tools/constants_expected_gaps.txt:1` aux mêmes lignes.
- **H-5c-5** — tient, précisé. Noms C présents au `HEAD` pour la réécriture : D-21 `X_class_def` / `X_kind_def` (dans 21 fichiers de `src/` ; plus aucun `g_X_class`/`g_X_kind`), `planet_kinds` (ex-`g_planet_kind`), `host_class_def` (ex-`g_host_cls`, `tests/test_particle_zone.c`) ; D-29 `mouse_world`, `mouse_screen` (paramètres des membres souris), `screen_pos` (`draw`, `draw_overlay`), `state` (ex-`me`), `year`, `slipway`, `offer`, `dropped`, `parallax` ; D-44 `received_needs` ; D-15 `targeting_own` ; D-38 statiques de test `memory`/`graphics`. Les noms gardés par 5c (paramètres hors famille de D-29, `static const g_…` hors classes) restent cités sous leur nom actuel.
- **H-5c-6** — tient. Les onze décisions de 5c et ses écarts à la lettre ne laissent aucun reste compté ; « Reste à trancher » (`static const g_…` hors classes, paramètres courts hors famille de D-29, `DIFFICULTY_OPTIONS_COUNT`, `k_entry`, `k_key`, la locale `gy` de `test_restart`) n'est vu par aucun compteur (le `gy` de `test_restart` est un identifiant, pas une mention nue).
- **H-5c-7** — tient. Seule la ligne 3f de `tools/rename_substeps.tsv` a changé ; 3a intacte.
- **H-5c-8** — tient. Aucune marque « appliqué », registre et vocabulaire non édités.
- **H-5c-9** — tient. La PRD de 5c est retirée de `triage/` par `4d6571c` : dans `docs/prd/`, seule cette PRD cite `TROU`.
- **H-5c-10** — tient. En-tête de `declarations.tsv` inchangé (`identity kind name parent file line linkage scope_lines occurrences flags`) ; les douze statiques de D-13 ont pour `parent` leur fonction et `linkage` `local` ; les 23 statiques de fonction de portée > 40 et les 8 qui seraient cryptiques (décision 3) sont les mêmes.

**Mesures faites pour cette PRD** sur un clone jetable au commit `19d45d7`, dans le scratchpad de l'agent, supprimé ensuite (les bindings libclang et le vendor raylib étaient des liens vers ceux du dépôt). Aucune commande git d'écriture n'a été lancée sur le dépôt.
- `make rename-extract` : 2 min 10 s, aucune ligne `warning:`, 4 lignes `partial`.
- Compteurs sans `SUBSTEP` : exactement le tableau « Restes comptés ». `cart-names-count` `TOTAL` 0/0/1/0 (`tools/frames_ref`) ; `french-count` `string` 36 ; `cart-citations` 6/2/7 (`Makefile:151`, `:201`, `:203`, `:205` ×3 en `cart`, `:205` `Lua` et `:206` `lua.lua` en `lua` ; `tools/pico8_reference_probe_out.txt:58` `b`, `:79` `c`, `:94` `d`, `:111`, `:128`, `:193` `e`, `:258` `f` en `short`) ; `cryptic-locals` 12 (dix `argc`/`argv` de cinq `main`, `blank` 59 et `screen_texture` 58) ; `scope-count` 12, colonne `static`.
- Les 17 lignes de D-33, D-55, D-59 et D-61 ajoutées à une copie de `tools/french_exceptions.txt` : `french_count.py --check` rend 0, code 0.
- Un fichier `tools/cart_names_exceptions.txt` posé tel quel (en-tête `#` et la ligne de D-1), sans changer l'outil : `rename_count.py --part text` le compte lui-même (`tools/cart_names_exceptions` `string` 1, son `tmp/pico8_replay`) ; `french-count` avertit `module tools/cart_names_exceptions is in no sub-step of tools/rename_substeps.tsv` ; `cart-citations` passe de 6 à 7 `cart`, parce que l'en-tête disait « anti-cart ». D'où les décisions 4 et 5.
- Statiques de fonction (`static`, `linkage` `local`, `parent` une fonction) : 930 dans l'extraction ; les 12 de D-13, portées 6 à 37 ; 23 de portée > 40, dont 8 seraient listées par `cryptic-locals` si on les jugeait comme des locales (`prefix`, `suffix`, `stack` de `tests/test_gif.c`, `three`, `too_long`, `nested` de `tests/test_layout_row.c`, `years` de `tests/test_score_panel.c`, `identity` de `tests/test_ui.c`). D'où la décision 3.
- Mots validés de `cryptic-locals` (parties des noms neufs des lignes `renommé`) : `screen` oui ; `texture`, `blank`, `argc`, `argv` non. `blank` et `screen_texture` sont des lignes `conservé` du TSV (`screencast_viewer.c`, `raylib_backend.c`, et six locales `blank` de tests).
- `TROU` : 62 lignes dans 48 fichiers de `src/`, `tests/`, `tools/`, 62 occurrences, comme à `1b4b949` (et à `4d6571c`). Hors marqueur nu, 20 lignes : 18 phrases de commentaire ou de message C (« (TROU-24) », « TROU-23: … ») et les deux `.txt` ; le texte lu ou imprimé hors commentaire C reste les trois du plan. `docs/trous-de-portage.md` : 34 occurrences sur 34 lignes (24 titres d'entrée, l'introduction, 9 renvois dans le texte) ; `CLAUDE.md:24` ; `README.md:23` seul (`README.md:7` cite le fichier, pas le marqueur) ; skills : 10 occurrences dans 5 fichiers (`fix-bug/SKILL.md:98`, `reprendre-les-commentaires/SKILL.md:53`, `consigne-agent.md:30` et `:32`, `exemple-travaille.md:30`, `:40`, `:43`, `:66`, `modele-rapport.md:72` et `:73`) ; `docs/prd/triage/phase4-etape5c-renommages.md:345` (retirée par `4d6571c`). Aucun `GAP-[0-9]` nulle part ; `GAP` seul est déjà un statut de `constants-check` (10 lignes : `tools/constants_check.sh`, `tools/constants_compare.sh`, `tests/test_constants_check.c`).
- `docs/trous-de-portage.md` : 24 entrées (TROU-1 à 5, 8 à 26), 3 030 mots. Sur ses 29 chemins de fichiers cités, 28 n'existent plus (`src/pico8_*.c`, `tools/probe_out.txt`…) ; sur 311 passages entre backticks, 226 mots distincts, dont 102 sont des anciens noms de lignes `renommé` du TSV (`qb:ph()`, `pico8_js_hd_dispatch`, `pico8_gu_content_ptrs`…). Les marqueurs du code renvoient aux numéros 1, 2, 3, 5, 8, 9, 10, 12 à 17, 21 à 26 ; les entrées 4, 11, 18, 19, 20 n'ont pas de marqueur (4 et 9 sont écrites « Sans site »).

## Condition d'entrée

- Les commits de 5c sont poussés sur la branche du chantier et sa note « Découvert » est présente sous l'entrée 5c du plan (H-5c-1).
- La sortie de 5c tient : sur une extraction fraîche sans `warning:`, les cinq compteurs sans `SUBSTEP` rendent le tableau « Restes comptés », **module par module et site par site**, pas un de plus. Un reste neuf arrête 5d avant le premier commit (décision 11).
- **Lecture des verdicts.** Celle que 5a a consignée : « ok » vaut `proposition`. 5d applique la proposition prête de chaque entrée X et de D-64, compléments compris. Les lignes `Verdict` du registre ne sont pas éditées.
- **« Toute entrée à reste compté jugée `garder tel quel` ou `reporter hors plan 5` »** : aucune à `19d45d7`. Les 66 lignes `Verdict` portent « ok » et les entrées à reste compté sont exactement les neuf entrées X (tableau « Restes comptés », colonne « Entrées »). Deux propositions X acceptent leur reste au lieu de le traiter (D-56 (a), D-59 (a) pour `short`) : par la règle des verdicts (« Conventions ») et par la sortie de 5d, ce reste finit en ligne d'exception. Si la note de 5c fait apparaître une entrée N à reste compté, la décision 11 s'applique.

## Problem Statement

Le §5 veut deux gardes bloquantes, et D-67 en ajoute une troisième, le compteur de français. 5a les a construites ; elles sont rouges, et doivent le rester jusqu'à 5e. Leur rouge ne vient plus que de restes que l'humain a jugés et que personne ne doit traiter : un repli de binaire gardé exprès (D-1), douze tables `static const` de tests dont la portée tient en 37 lignes (D-13), des mots français par construction ou par hasard (D-33, D-55, D-59, D-61). Les deux relevés hors garde ont les leurs : `argc`/`argv` et deux mots de l'API raylib dans `cryptic-locals` (D-37, D-60), la légende du `Makefile` et un relevé figé de PICO-8 dans `cart-citations` (D-56, D-59). Tant que ces restes comptent, 5e ne peut rien brancher : une garde rouge par construction ne garde rien.

Le registre a dit comment lever chacun : une ligne d'exception, ou une règle d'outil. Trois outils ne savent pas encore le faire : `rename_count.py` ne lit pas d'exceptions, `cart_citations.py` non plus, et `scope_count.py --cryptic` n'a aucune règle pour `main` ni pour raylib.

Le §4 dit aussi que les étiquettes `TROU-N` deviennent `GAP-N`. Le registre des écarts, `docs/trous-de-portage.md`, sur lequel ces marqueurs renvoient, est écrit dans les noms d'avant le chantier : 28 de ses 29 chemins n'existent plus, et près de la moitié des noms qu'il cite sont des anciens noms du TSV. Le plan veut qu'il reste en français mais décrive chaque écart avec les noms C d'aujourd'hui, le nom Lua seulement en tête. Le marqueur lui-même apparaît dans 48 fichiers de code, dans `CLAUDE.md`, le `README` et cinq fichiers de skills qui l'enseignent : un renommage à moitié fait laisserait un marqueur qui ne renvoie à rien.

## Solution

**Une PRD, quatre morceaux ordonnés, treize commits.** Chaque commit garde `make test-all` vert et `tests/e2e/*.sha256` inchangés.

1. **Lignes d'exception (famille X)**, un commit par entrée, dans l'ordre des numéros : D-1, D-13, D-33, D-37, D-55, D-56, D-59, D-60, D-61. Quand l'entrée change un outil, le test vient d'abord (rouge joué, consigné dans le message). Après chaque commit, le compteur de l'entrée baisse exactement de son reste, et aucun autre ne bouge.
2. **D-64** : la ligne `# TROU-1` de `tools/constants_expected_gaps.txt` part, avant le passage.
3. **`TROU` → `GAP`** en deux commits :
   - `docs/trous-de-portage.md` réécrit avec les noms C d'après 5c, clés encore `TROU-N`, chaque entrée relue contre le code ;
   - puis un seul commit mécanique qui passe `TROU` à `GAP` partout à la fois : marqueurs et phrases du code, message d'`assert`, commentaire de `tools/reachability_questions.txt`, clés et renvois de `docs/trous-de-portage.md`, `CLAUDE.md`, `README.md`, les cinq fichiers de skills.
4. **Note « Découvert »** de 5d et relevés de fin.

À la fin :
- les cinq compteurs sans `SUBSTEP` rendent 0 sur une extraction fraîche, avec leurs exceptions ; `old-names-guard`, `ast-guard` et `french-guard` rendent 0, prêtes pour 5e ;
- `git grep -n TROU` rend 0 hors du plan, du registre, de `docs/vocabulaire.md`, de `docs/glossaire-noms.md` et de `docs/prd/` (décision 9) ;
- `make constants-check` et `make test-all` verts, `tests/e2e/*.sha256` inchangés.

## User Stories

1. En tant qu'exécutant de 5e, je veux que les trois gardes de 5a rendent 0 sur l'arbre réel, pour brancher en bloquant des gardes qui ne rougissent que d'une violation neuve.
2. En tant que mainteneur, je veux que chaque reste jugé par l'humain soit levé par la forme que sa proposition nomme (ligne d'exception ou règle), et pas autrement, pour que le registre reste la seule source des exceptions.
3. En tant que mainteneur, je veux qu'une ligne d'exception retire exactement son triplet `colonne module nom`, et rien dans une autre colonne ni dans un autre module, pour qu'une exception ne cache pas un ancien nom réintroduit ailleurs.
4. En tant que mainteneur de la garde anti-cart, je veux que `tools/frames_ref.sh` garde son repli `tmp/pico8_replay` sans faire rougir la garde (D-1 (a)), pour que `make e2e-ref` sache encore rejouer les références d'avant `2bb367d`.
5. En tant que mainteneur, je veux que le fichier d'exceptions d'un compteur ne soit pas compté par ce compteur, pour qu'un nom excepté ne revienne pas par le fichier qui l'excepte.
6. En tant que mainteneur de la garde de portée, je veux qu'une `static` déclarée dans une fonction soit jugée comme une locale, par sa portée (D-13 (a)), pour que la tolérance du §1 vaille pour une table de test sans liste à tenir.
7. En tant que mainteneur, je veux que la règle de D-13 ne change pas la liste des locales cryptiques, pour qu'une décision sur la garde de portée n'ouvre pas huit candidats neufs dans un autre relevé.
8. En tant que lecteur de `cryptic-locals`, je veux que `argc` et `argv` de `main` n'y soient plus (D-37 (a)), pour que la liste ne relève pas l'idiome C.
9. En tant que lecteur de `cryptic-locals`, je veux que `blank` et `screen_texture` valent validés par une règle, pas par une liste (D-60 (a)), pour que le viewer et le backend gardent un même nom pour un même rôle.
10. En tant que mainteneur du compteur de français, je veux les 17 lignes de D-33, D-55, D-59 et D-61 dans `tools/french_exceptions.txt`, chacune dans le commit de son entrée, pour relire une décision à la fois.
11. En tant que mainteneur de `cart-citations`, je veux un fichier d'exceptions au même format que les deux autres (D-56, D-59), test d'abord, pour que le relevé rende 0 sans toucher la légende du `Makefile` ni le relevé figé de PICO-8.
12. En tant que relecteur, je veux un commit par entrée, avec son numéro et la baisse exacte de son compteur dans le message, pour pouvoir en annuler une seule.
13. En tant que mainteneur de `constants-check`, je veux que la liste des défaillances attendues perde le marqueur `# TROU-1` qui ne la concerne pas (D-64 (b)), avant le passage.
14. En tant que lecteur de `docs/trous-de-portage.md`, je veux que chaque écart soit décrit avec les noms C et les chemins d'aujourd'hui, le nom Lua seulement en tête, pour trouver le code concerné sans passer par le TSV.
15. En tant que lecteur du code, je veux que chaque marqueur `/* GAP-N */` renvoie à une entrée `GAP-N` du registre des écarts, pour qu'aucun marqueur ne renvoie à rien.
16. En tant que relecteur, je veux que le commit `TROU` → `GAP` soit purement mécanique (même nombre de lignes par fichier, seul `TROU` changé), pour le relire par un `--word-diff`.
17. En tant qu'agent qui reprend les commentaires plus tard, je veux que `CLAUDE.md` et les skills enseignent `/* GAP-n */` dans le même commit que le code, pour ne jamais réintroduire `TROU`.
18. En tant que mainteneur de `e2e-gap`, je veux qu'aucun commit ne change le nombre de lignes d'un fichier de `tools/line_coverage_reference.txt`.
19. En tant qu'humain au prochain point d'arrêt, je veux lire dans la note les décisions prises sans humain, les entrées du registre des écarts trouvées refermées ou fausses en les relisant, et tout reste neuf laissé par 5c, pour trancher avant 5e.
20. En tant qu'agent, je veux que l'appli raylib ne soit jamais lancée : `make raylib` et `make screencast-viewer` sont compilés, jamais exécutés.

## Implementation Decisions

### Règles communes

- **Une ligne d'exception n'est posée que par une entrée du registre** (« Only a human decision adds a line », en tête de chaque fichier). 5d ne pose que les lignes des neuf entrées X, mot pour mot.
- **Test d'abord** quand un outil change (D-1, D-13, D-37, D-56, D-60) : le test neuf est rouge sur l'outil d'avant, puis vert ; le rouge joué va au message. Une ligne ajoutée à un fichier d'exceptions existant (D-33, D-55, D-59 pour le français, D-61) n'a pas de test neuf : son contrôle est la baisse exacte du compteur.
- **Après chaque commit**, sur une extraction fraîche quand le compteur la lit (`cart-names-count`, `scope-count`, `cryptic-locals`) : le compteur de l'entrée baisse exactement du reste de l'entrée, et les quatre autres sont égaux à la capture du morceau 0, module par module. Une ligne qui monte arrête le morceau.
- **Format des fichiers d'exceptions** : celui de `tools/french_exceptions.txt` et de `tools/scope_exceptions.txt` (lu par `rename_tables.read_triples`) : deux lignes `#` d'en-tête en anglais, puis `colonne<TAB>module<TAB>nom`. L'en-tête ne contient ni `cart`, ni `Lua`, ni mot français : `cart-citations` et `french-count` lisent les commentaires des `.txt` (mesuré : « anti-cart » ajoute un `cart`). Aucun commentaire écrit par 5d dans le code ne cite nu un ancien nom de deux lettres (`id`, et depuis 5c `fy`, `kv`, `nj`, `oc`, `gy`) : il compterait en `short` (H-5c-3).
- **Chaque fichier d'exceptions neuf** est ajouté à la liste de modules de 3a de `tools/rename_substeps.tsv`, à côté de `tools/french_exceptions` (mesuré : sinon `french-count` avertit).
- **Légende du `Makefile`** : la ligne de chaque cible dont l'outil change dit ce qu'il retire (« minus tools/…_exceptions.txt », « argc/argv of main and raylib words are not cryptic »), sans le mot `cart` entier.
- **Hauteur constante** : aucun commit n'ajoute ni ne retire de ligne à un fichier de `tools/line_coverage_reference.txt` ; les commits X ne touchent pas `src/`.
- Messages en français, préfixe `<type>: plan 5 étape 5d — …`, avec le numéro de l'entrée et les lignes d'attribution de la session.

### Les entrées X

| Ordre | Entrée | Forme | Ce qui change | Contrôle (en plus du cliquet) |
|---|---|---|---|---|
| 1 | D-1 (a) | fichier neuf, outil | `tools/rename_count.py` prend `--exceptions` (facultatif) : chaque triplet `colonne module nom` retire exactement les mentions de l'ancien nom `nom` dans cette colonne et ce module, dans les deux moitiés (`--part text`, `--part ast` : une occurrence AST porte l'ancien nom de son identité), avant le test de `TOTAL` de `--check`, et avec `--substep` ; le fichier passé en `--exceptions` n'est pas lu comme code. `tools/cart_names_exceptions.txt` neuf, avec la seule ligne `string	tools/frames_ref	tmp/pico8_replay`. Le `Makefile` le passe à `cart-names-count`, `old-names-guard` et à l'appel de `rename_count.py` d'`ast-guard` | `make cart-names-count` `TOTAL` 0/0/0/0 ; `make old-names-guard` code 0 ; la moitié `rename_count` d'`ast-guard` code 0 (la portée reste rouge jusqu'à D-13) |
| 2 | D-13 (a) | règle | `tools/scope_count.py` : une `static` dont le `parent` n'est pas `-` (déclarée dans une fonction) se juge comme une locale, par sa portée : permise jusqu'à 40 lignes, refusée en colonne `long-scope` au-delà. Une `static` de fichier reste refusée en colonne `static`. `is_long_scope`, que lit `--cryptic`, ne change pas (décision 3) | `make scope-count` `total` 0 ; `make ast-guard` code 0 ; `make cryptic-locals` égal à la capture |
| 3 | D-33 (a) | 14 lignes | les 14 triplets de la proposition dans `tools/french_exceptions.txt`, dans leur ordre | `make french-count` `string` 36 → 10 ; `tests/test_cart_citations`, `tests/test_scope_count`, `tools/cart_citations`, `tools/scope_count` à 0 |
| 4 | D-37 (a) | règle | `tools/scope_count.py --cryptic` : un paramètre nommé `argc` ou `argv` d'une fonction nommée `main` n'est pas candidat ; un `argc` d'une autre fonction, ou une locale `argc` de `main`, le reste | `make cryptic-locals` `total` 12 → 2 |
| 5 | D-55 (a) | 1 ligne | `string	Makefile	du` | `french-count` 10 → 9 |
| 6 | D-56 (a) | fichier neuf, outil | `tools/cart_citations.py` prend `--exceptions` (facultatif), même lecteur, comparaison du jeton sans casse comme `french_count.py` ; le fichier passé n'est pas lu (`OWN_FILES` ou équivalent). `tools/cart_citations_exceptions.txt` neuf, avec `cart	Makefile	cart`, `lua	Makefile	lua`, `lua	Makefile	lua.lua`. Le `Makefile` le passe à `cart-citations` | `make cart-citations` 6/2/7 → 0/0/7 ; `MODULES=Makefile` 0/0/0 |
| 7 | D-59 (a) | 1 + 5 lignes | `string	tools/pico8_reference_probe_out	ce` dans `tools/french_exceptions.txt` ; `short	tools/pico8_reference_probe_out	b`, `c`, `d`, `e`, `f` dans `tools/cart_citations_exceptions.txt` (cinq jetons pour sept sites) | `french-count` 9 → 1 ; `cart-citations` 0/0/7 → 0/0/0 |
| 8 | D-60 (a) | règle | `tools/scope_count.py --cryptic` : les mots de l'API raylib que le code emploie comme noms valent validés, comme les parties des noms neufs du TSV ; constante de l'outil, `blank` (`BLANK`) et `texture` (`Texture2D`), chacun avec le nom raylib d'où il vient (décision 2) | `make cryptic-locals` `total` 2 → 0 ; `MODULES=cmd/screencast_viewer` 2 candidats de moins |
| 9 | D-61 (a) | 1 ligne | `string	tools/ttf_to_pixelfont	LÖVE` | `french-count` 1 → 0 ; `make french-guard` code 0 |

Tests (un fichier de test par outil, aucun neuf) :
- `tests/test_rename_count.py` : une ligne retire son triplet et lui seul (même nom dans un autre module ou une autre colonne : compté) ; un ancien nom court en colonne `identifier` excepté par son nom (moitié AST) ; `--check` à 0 quand le seul reste est excepté, à 1 sinon ; le fichier d'exceptions, placé dans `tools/` de l'arbre de test, n'est pas compté ; sans `--exceptions`, les sorties d'avant à l'octet.
- `tests/test_scope_count.py` : `static` de fonction à 40 lignes permise, à 41 refusée en `long-scope` ; `static` de fichier refusée en `static` ; `static` de fonction cryptique de 41 lignes absente de `--cryptic` ; `argc`/`argv` de `main` absents de `--cryptic`, `argc` d'une autre fonction présent ; `screen_texture` et `blank` à 41 lignes absents, `blanket` présent.
- `tests/test_cart_citations.py` : une ligne retire son triplet et lui seul, sans casse (`Lua` retiré par `lua`) ; le fichier d'exceptions n'est pas lu ; sans `--exceptions`, sortie d'avant.

### D-64

- La ligne `# TROU-1` de `tools/constants_expected_gaps.txt` part ; les deux lignes de commentaire qui suivent restent. Commit `chore:` seul, avant le morceau `TROU` → `GAP`.
- Contrôle : `make constants-check` et `make test` verts ; `git grep -n TROU -- tools/constants_expected_gaps.txt` → 0.

### `docs/trous-de-portage.md` en noms C (commit `docs:`, clés encore `TROU-N`)

Ce que dit le plan : le fichier reste en français, décrit chaque écart avec les nouveaux noms C, le nom Lua n'apparaît qu'en tête. Forme retenue (décision 6) :

```
## TROU-3 — Les chaînes ne sont pas internées

Lua : `gu`, `hy.invented`, `jo:gg`, `db`.

<corps en noms C : fonctions, types, champs, chemins d'aujourd'hui>

Fichiers : `src/engine/collect.c`.
```

- La ligne `Lua :` est la première ligne sous le titre. Elle porte les noms du cart et ceux de l'API PICO-8 que l'entrée cite (`tostr`, `sin`, `pal()`, `btn(b,1)`…). Une entrée sans nom du cart ni de l'API n'en a pas.
- Le titre peut garder un nom de l'API PICO-8 (`sin`/`cos`, `tostr`, `circ`) ; un nom du cart (`oo()`, `dg()`, `hd`, `trade:kg`, `h_`, `jb`, `fo`) en sort, remplacé par le nom C ou par ce qu'il fait.
- Le corps ne cite que des noms qui existent dans `src/`, `tests/`, `tools/` ou le `Makefile` après 5c, et des chemins qui existent. « PICO-8 », le produit, reste nommé dans le texte.
- Les renvois entre entrées (« relève de TROU-17 ») restent des renvois.
- L'introduction garde son sens ; elle dit que le code renvoie par `/* TROU-n */` (le commit suivant le passe à `GAP-n`).
- Une entrée « Sans site » le reste. Une entrée qui, relue contre le code, semble refermée, ou dont la description ne tient plus, **n'est ni supprimée ni corrigée sur le fond** : elle est réécrite en noms C telle qu'elle est, et la note « Découvert » la signale pour décision humaine (décision 7).
- Correspondance des noms : `docs/renommage.tsv` (`ancien` → `nouveau`, puis les lignes que 5c a changées), et le code lui-même pour les chemins ; un nom qu'aucune des deux sources ne donne est cherché dans le code par son rôle, et nommé dans le message du commit.
- Contrôles, par un script jetable dans `tmp/5d/` (non commité) :
  - chaque chemin cité existe ;
  - chaque identifiant entre backticks hors ligne `Lua :` existe en mot entier dans `src/ tests/ tools/ Makefile` (`git grep -w`), ou est une expression ou un littéral (`0x0000.ce4f`, `"-0"`) ; la liste des exclus va au message ;
  - aucun nom distinctif de la garde anti-cart (`rename_count.Names` : mots distinctifs et chemins des lignes `renommé`) hors ligne `Lua :` ;
  - l'ensemble des numéros des titres est inchangé (24 entrées), et chaque numéro cité par un marqueur du code a son entrée.

### `TROU` → `GAP` (un seul commit `refactor:`)

L'écrivain du texte `TROU` est le registre des écarts (ses clés) ; ses lecteurs sont les marqueurs et les phrases qui le citent, et ce qui enseigne le marqueur. Tout passe dans un seul commit :

| Lieu | Sites à `19d45d7` | Changement |
|---|---|---|
| `src/`, `tests/`, `tools/` | 62 lignes, 48 fichiers, dont `tests/test_str.c:91` (message d'`assert`) et `tools/reachability_questions.txt:2` (« through a TROU entry ») ; 61 après D-64 | `TROU-` → `GAP-` ; « a TROU entry » → « a GAP entry » |
| `docs/trous-de-portage.md` | 34 occurrences (24 titres, l'introduction, 9 renvois) | `TROU-` → `GAP-` ; le nom du fichier ne change pas (décision 8) |
| `CLAUDE.md:24` | `/* TROU-7 */` | `/* GAP-7 */` |
| `README.md:23` | « Décider de TROU-26 » | `GAP-26` ; `README.md:7` ne cite pas le marqueur et n'est pas touché (5f) |
| `.claude/skills/fix-bug/SKILL.md:98` | `/* TROU-n */` | `/* GAP-n */` |
| `.claude/skills/reprendre-les-commentaires/` | `SKILL.md:53`, `consigne-agent.md:30`, `:32`, `exemple-travaille.md:30`, `:40`, `:43`, `:66`, `modele-rapport.md:72`, `:73` | `TROU-` → `GAP-` (`TROU-?`, `TROU-<n>`, `TROU-n`, `TROU-4`…) ; le reste de ces fichiers relève de 5f (D-65) |

- Méthode : `git grep -l TROU` sur ces chemins, `sed` mot à mot ; aucune autre retouche dans ce commit.
- Contrôles :
  - `git grep -n TROU` rend 0 hors de `docs/plan-5.md`, `docs/decisions-plan-5.md`, `docs/vocabulaire.md`, `docs/glossaire-noms.md` et `docs/prd/` ;
  - `git diff --word-diff=porcelain HEAD~1` ne montre que `TROU` → `GAP` (et `TROU entry` → `GAP entry`) ; `git diff --numstat` : autant de lignes ajoutées que retirées par fichier ;
  - les numéros cités par `/* GAP-N */` et les phrases du code ⊂ les titres `## GAP-N` du registre ;
  - `make constants-check`, `make test-all` verts ; `tests/e2e/*.sha256` inchangés ; les cinq compteurs égaux à 0 (un mot neuf ne doit rien y faire : `GAP` n'est ni français ni un ancien nom ; à vérifier, pas à supposer).

### Décisions prises sans humain

1. **D-56 et D-59 « acceptent » leur reste ; il finit en ligne d'exception.** Leur proposition dit « l'accepter (le relevé n'est pas une garde du §5) », « rien à appliquer ». Mais la sortie de 5d veut `cart-citations` à 0 « avec leurs exceptions », en lui donnant un fichier « si un verdict y laisse un reste », et la règle des verdicts fait finir un reste accepté en ligne d'exception. Décision : `tools/cart_citations_exceptions.txt`, format des deux autres, comparaison sans casse comme le français ; 3 lignes pour D-56, 5 pour la part `short` de D-59. La granularité est celle du registre (module, pas ligne) : un `cart` neuf dans le `Makefile` ne sera plus vu, comme un `du` neuf ne l'est plus par le français. La note le dit.
2. **D-60, la forme de la règle.** La proposition veut « une règle plutôt qu'une liste », et l'option (a) la nomme : « un mot d'un type de l'API raylib vaut validé ». À la lettre, elle ne couvre pas `blank` (`BLANK` est une couleur, pas un type). Lire tout `raylib.h` (vendor hors git, posé par `raylib-vendor-setup`) validerait des centaines de mots et ferait dépendre le relevé du vendor. Décision : une constante de `scope_count.py`, des mots de l'API raylib que le code emploie comme noms, chacun avec le nom raylib d'où il vient : `blank` (`BLANK`) et `texture` (`Texture2D`) ; ils s'ajoutent aux mots validés pour toute locale. C'est une règle au niveau du mot (toute locale `texture` ou `blank` future passe), pas une liste de déclarations.
3. **D-13, la règle ne touche que la garde de portée.** La proposition dit « une `static` dont le parent est une fonction se juge comme une locale, par sa portée ». Si `is_long_scope` la jugeait ainsi, `cryptic-locals` gagnerait 8 candidats (mesure). La question de D-13 porte sur `scope-count` seul. Décision : la règle vit dans `column_of` ; une telle `static` au-delà de 40 lignes est refusée en `long-scope`, comme une locale. `cryptic-locals` ne change pas.
4. **D-1, le fichier qui excepte n'est pas compté.** Mesuré : posé tel quel, `tools/cart_names_exceptions.txt` se fait compter `string` 1 par `rename_count.py`. Décision : le fichier passé en `--exceptions` est sauté par l'outil qui le lit, comme `french_count.py` saute le sien (`OWN_FILES`) ; idem pour `cart_citations.py`. Les exceptions valent aussi pour la moitié AST (par l'ancien nom de l'identité), pour `--check` et pour `--substep` : un format dont une colonne ne marcherait pas serait un piège.
5. **Les fichiers d'exceptions neufs vont en 3a** dans `tools/rename_substeps.tsv` (mesuré : avertissement de `french-count` sinon), à côté de `tools/french_exceptions`.
6. **Forme du registre des écarts.** « Le nom Lua n'apparaît qu'en tête de chaque GAP » est lu comme : une ligne `Lua :` sous le titre, seul endroit des noms du cart ; les noms de l'API PICO-8 y vont aussi, et le titre peut en garder un (`tostr`, `sin`), le produit étant la référence. Le corps n'a que des noms C.
7. **Réécrire n'est pas revérifier.** La réécriture change les noms, pas le fond. Une entrée qui semble refermée ou fausse est signalée dans la note, pas supprimée : supprimer une entrée, c'est déclarer l'écart refermé, une décision.
8. **Le nom du fichier `docs/trous-de-portage.md` reste**, comme le dit l'entrée 5d (« comme clé dans `docs/trous-de-portage.md` ») : les docs restent en français, et le renommer toucherait `README.md:7`, les skills et le plan sans gain.
9. **La sortie `git grep -n TROU` exclut aussi `docs/prd/`.** La PRD en cours (celle-ci) cite `TROU` partout, et n'est retirée de `triage/` qu'à sa graduation, après la note ; la PRD de 5c, qui en citait une ligne, est graduée depuis `4d6571c`. Aucun lecteur n'y suit un marqueur.
10. **Les skills passent en 5d, pas en 5f**, pour le seul mot `TROU` : l'entrée 5d les nomme parmi les lecteurs, et la règle « chaque lecteur avec son écrivain » l'emporte sur la famille D. Le reste des skills (D-65, symboles et chemins) reste à 5f.
11. **Un reste neuf après 5c n'est pas excepté.** Seule une décision humaine ajoute une ligne. Si la capture du morceau 0 montre un site hors du tableau « Restes comptés » : s'il vient d'une mention que 5c aurait dû réécrire (portée d'une décision de nommage), un commit `fix:` le corrige au nom de l'entrée N en cause, avant D-1 ; sinon 5d s'arrête avant le premier commit et la question va à l'humain.
12. **Ordre.** Les entrées X dans l'ordre des numéros : D-56 (qui crée le mécanisme de `cart-citations`) précède D-59 (qui s'en sert), et chaque compteur atteint 0 au commit de sa dernière entrée (`cart-names-count` à D-1, `scope-count` à D-13, `cart-citations` à D-59, `cryptic-locals` à D-60, `french-count` à D-61). La réécriture du registre des écarts précède le passage `TROU` → `GAP`, pour que ce dernier soit mécanique et se relise par `--word-diff`.
13. **Registre et vocabulaire.** Ni les lignes `Verdict`, ni le tableau « Restes comptés », ni `docs/vocabulaire.md` ne sont édités. Il n'y a pas de marque « appliqué ».

## Implementation Chunks

0. **Mesure de départ**, sans commit, dans `tmp/5d_baseline/`, sur l'arbre de fin de 5c :
   - `make rename-extract` (sans `warning:`, 4 `partial`) ;
   - les cinq compteurs sans `SUBSTEP`, avec `SITES=1` pour le français et les sites de `cart-citations` ; `old-names-guard`, `ast-guard`, `french-guard` et leurs codes ;
   - `git grep -n TROU` (code, docs, skills) et le relevé des numéros de marqueurs ;
   - `make constants-check`, `class-coverage-net`, `e2e-gap`.

   Égal au tableau « Restes comptés » et aux sites de cette PRD, aux numéros de ligne près : on ouvre. Sinon, décision 11.
1. **Lignes d'exception**, neuf commits dans l'ordre du tableau : `feat:` pour D-1, D-13, D-37, D-56, D-60 (test d'abord) ; `chore:` pour D-33, D-55, D-59, D-61. Contrôle de l'entrée et cliquet à chacun.
2. **D-64**, un commit `chore:`.
3. **`TROU` → `GAP`** :
   - `docs:` — `docs/trous-de-portage.md` en noms C, contrôles du script jetable, liste des noms cherchés par leur rôle et des entrées signalées dans le message ;
   - `refactor:` — le passage, un seul commit, contrôles du tableau.
4. **Note « Découvert »** de 5d, un commit `docs:` sous l'entrée 5d de `docs/plan-5.md`. Elle contient :
   - les neuf entrées X et D-64, leur commit et leur baisse ;
   - les décisions 1 à 13 ;
   - les entrées du registre des écarts signalées (décision 7) et les noms cherchés par leur rôle ;
   - pour 5e : les trois gardes à 0, la granularité par module des exceptions (décision 1), la règle de D-60 au niveau du mot ;
   - tout écart à une hypothèse `H-5c-k` ;
   - les relevés de fin.

   Si un constat contredit le §4, le §5 ou l'entrée 5d, le plan est corrigé dans le même commit.

**Total : 13 commits** (9 + 1 + 2 + 1), plus un `fix:` par reste neuf corrigé sous la décision 11.

## Testing Decisions

- **Un bon test juge une sortie observable** : le tableau, les sites et le code de sortie d'un outil lancé sur un mini-arbre ou une mini-extraction, jamais son texte. Chaque test d'exception prouve les deux sens : la ligne retire son triplet, et rien d'autre (autre module, autre colonne, autre nom) ; un filtre qui retirerait tout serait vert sur le seul sens positif.
- **Tests modifiés** : `tests/test_rename_count.py`, `tests/test_scope_count.py`, `tests/test_cart_citations.py`, dans `make test`. Aucun fichier de test neuf, aucune cible neuve. Les lignes ajoutées à `tools/french_exceptions.txt` n'ont pas de test : `french_count.py` ne change pas, et le test d'exceptions de `tests/test_french_count.py` couvre déjà le mécanisme.
- **Le vrai arbre** fait foi pour les baisses : chaque commit X a son contrôle sur l'extraction fraîche, et le cliquet sur les quatre autres compteurs.
- **Le passage `TROU` → `GAP`** n'a pas de test : aucun outil ne lit le motif. Son filet est le `--word-diff`, `--numstat`, le `git grep` de sortie et la correspondance marqueurs ⊂ titres.
- **Prior art** : le test d'exceptions de `tests/test_french_count.py` (ligne 209 à `19d45d7`) et `test_x_and_y_of_the_vector_type_are_excepted_but_not_of_another_type` de `tests/test_scope_count.py` (forme d'un triplet qui excepte et de son contraire) ; `test_check_fails_on_a_residue_and_passes_on_a_clean_tree` de `tests/test_rename_count.py` (forme de `--check`) ; `test_the_exceptions_file_is_needed_by_the_count_only` (forme d'une option facultative) ; la capture avant/après de 5a et le cliquet de 5c.

**Relevés de fin** (sortie de 5d et invariant du §5) :
- `make test-all` vert, `tests/e2e/*.sha256` inchangés ; `make constants-check` vert ;
- après `rename-extract` frais sans `warning:` (4 `partial`), les cinq compteurs sans `SUBSTEP` rendent 0 ; `old-names-guard`, `ast-guard`, `french-guard` rendent 0 ;
- `git grep -n TROU` : 0 hors des cinq exclusions ;
- `make raylib` et `make screencast-viewer` compilent et linkent, jamais lancés ;
- `class-coverage-net` et `e2e-gap` : sorties égales à celles du morceau 0, aux numéros de ligne du `Makefile` près ;
- `rename-tool-test`, les `rename-*-test`, `scope-count-test`, `french-count-test`, `cart-citations-test`, `rename-substeps-check` verts.

## Out of Scope

- Rendre les gardes bloquantes, et les brancher dans `test` ou `test-all` (5e, D-66 (c), D-67 (a)).
- Toute entrée hors familles X et T : renommages (5c), gardes à motifs (5b), docs et skills au-delà du mot `TROU` (5f, D-65), dont `README.md:7` (« Portage littéral ») et la suppression de `docs/glossaire-noms.md`.
- Revérifier ou refermer un écart du registre des écarts, en ajouter un, changer le comportement du jeu (§7).
- Renommer `docs/trous-de-portage.md`, ou le statut `GAP` de `constants-check`.
- Étendre D-36 (b) à `short` de `cart-citations` ; l'ancre `pico8_%s_struct_class` de `rename_strings.py` ; les `static const g_…` hors D-21, les paramètres hors famille de D-29 et `DIFFICULTY_OPTIONS_COUNT` que la note de 5c laisse à trancher (aucun compteur ne les voit).
- Éditer le registre, le tableau « Restes comptés » ou `docs/vocabulaire.md`.
- Lancer `rename-derive` ou `rename-strings` sur le vrai TSV, `rename-tool-setup`, `e2e-baseline`, ou l'appli raylib.

## Hypothèses sur 5c

Écrites à `19d45d7`, 5c non faite, d'après sa PRD (`git show b9e4873:docs/prd/triage/phase4-etape5c-renommages.md`) ; relues à `4d6571c` (§ « Relu contre la note « Découvert » de 5c », en tête). Chacune se vérifie sur la note « Découvert » de 5c et sur l'arbre de sa fin. Une hypothèse fausse fait relire les sections nommées avant d'ouvrir 5d.

- **H-5c-1** — 5c est poussée, sa note « Découvert » existe, et sa sortie tient : sur une extraction fraîche sans `warning:`, les cinq compteurs sans `SUBSTEP` rendent exactement le tableau « Restes comptés », module par module et aux mêmes sites (les 36 mots français, `tools/frames_ref`, les 6/2/7 de `cart-citations`, les 12 candidats cryptiques, les 12 statiques). *À relire* : Condition d'entrée ; Les entrées X (contrôles, baisses attendues) ; décision 11 ; Implementation Chunks, point 0.
- **H-5c-2** — 5c ne touche ni les outils compteurs (`rename_count.py`, `scope_count.py`, `cart_citations.py`, `french_count.py`, `rename_tables.py`) ni leurs tests, ni `tools/french_exceptions.txt`, `tools/scope_exceptions.txt`, `tools/constants_expected_gaps.txt`, `tools/reachability_questions.txt`, ni les recettes des cibles compteurs et gardes du `Makefile` (D-2 ne change que le `-D` et un commentaire). *À relire* : Les entrées X (tableau) ; D-64 ; Règles communes (légende).
- **H-5c-3** — Les noms neufs de 5c deviennent des mots validés (`mouse_world`, `screen_pos`, `received_needs`, `memory`…) et quelques noms courants deviennent des anciens noms (`fy`, `kv`, `gy`, D-29 et D-32 ; `mem`/`gfx` sortis de D-36 (b) par D-38), sans changer les restes de `cryptic-locals` ni de `cart-citations` : `blank`, `screen_texture`, `argc`, `argv` restent non validés, et les jetons `short` du relevé figé restent `b`, `c`, `d`, `e`, `f`. *À relire* : Les entrées X (D-37, D-59, D-60) ; décision 2.
- **H-5c-4** — Aucun marqueur `TROU` n'est retiré, ajouté ni renuméroté par 5c : 62 lignes dans 48 fichiers, mêmes numéros. 5c réécrit des commentaires voisins (« Portée d'une décision de nommage ») et décale des lignes ; les numéros de ligne de cette PRD se relisent au morceau 0. *À relire* : `TROU` → `GAP` (tableau) ; Implementation Chunks, point 0.
- **H-5c-5** — Les noms C que `docs/trous-de-portage.md` devra citer sont ceux de l'arbre de fin de 5c, lus dans le code au moment de la réécriture, pas dans cette PRD. Entrées du registre des écarts qui touchent des noms renommés par 5c, à vérifier : TROU-8 (tag `prober` et tables de classe de la famille sonde : D-21 `…_class_def`, D-15 `targeting_own`) ; TROU-11 (repli d'arité du survol : paramètres `qa`/`ra` des méthodes, D-29 `mouse_world`/`mouse_screen`) ; TROU-12, TROU-21, TROU-25 (tables de classe et de rangée, D-21) ; TROU-23 (champs de rangée de bouton) ; TROU-24 (besoins et routes : D-44 `received_needs` si l'entrée cite le panneau) ; TROU-26 (planète, objet tenu). La note de 5c, liste de ses 33 commits, dit lesquels ont changé. *À relire* : `docs/trous-de-portage.md` en noms C (correspondance des noms, contrôles).
- **H-5c-6** — Les décisions prises sans humain de 5c (sa PRD, décisions 1 à 11 : D-18 et son `unseen`, la portée de D-21 et `planet_kinds`, le sens de D-29 et `screen_pos`, D-24, le contrôle de D-35, la ligne `fichier` de D-45, `eh` de D-32, les contrôles restreints, le découpage) ne créent aucun reste compté ; les sujets laissés « à trancher » (`static const g_` hors D-21, paramètres courts hors famille de D-29, `DIFFICULTY_OPTIONS_COUNT`) ne sont vus par aucun compteur, et 5d ne les attend pas. Si un humain les tranche avant 5d, ce sont des entrées N : hors périmètre. *À relire* : Condition d'entrée ; Out of Scope ; décision 11.
- **H-5c-7** — 5c ne touche pas la ligne 3a de `tools/rename_substeps.tsv` (D-45 édite la ligne 3f). *À relire* : Règles communes ; décision 5.
- **H-5c-8** — 5c ne fixe aucune convention de marquage du registre (comme 5a et 5b). Si elle en fixe une, 5d la suit pour ses dix entrées dans le commit `docs:` final. *À relire* : décision 13 ; Implementation Chunks, point 4.
- **H-5c-9** — La PRD de 5c est graduée (retirée de `docs/prd/triage/`) avant que 5d ne commence ; sinon sa ligne 345 est l'autre site de `docs/prd/` que la décision 9 exclut. *À relire* : décision 9 ; `TROU` → `GAP` (contrôles).
- **H-5c-10** — L'extraction et ses sorties gardent le format que lisent les compteurs (`declarations.tsv` : colonnes `kind`, `name`, `parent`, `linkage`, `scope_lines` ; une `static` de fonction a pour `parent` le nom de sa fonction, une `static` de fichier `-`). *À relire* : Les entrées X (D-13, D-37) ; Testing Decisions.

## Further Notes

- **Coût.** Trois extractions fraîches au moins (morceau 0, après D-1 pour le cliquet AST, après D-13 et D-60), 2 min 10 s chacune ici ; un `make test-all` par commit (≈ 30 s). Les commits qui ne touchent que le français ou `cart-citations` n'ont pas besoin d'extraction, sauf pour le relevé de fin.
- **Ce qu'une exception par module coûte.** Les trois fichiers excepteront un mot dans tout un module (le registre a fixé ce format pour le français, D-1 le reprend). Après 5e, un `cart` neuf dans le `Makefile`, un `ce` neuf dans le relevé figé, un `du` neuf dans le `Makefile` passeront. C'est voulu par les verdicts ; la note le rappelle pour 5e.
- **`GAP` est déjà un mot du code.** `constants-check` appelle `GAP` un littéral qui diverge de PICO-8 (`tools/constants_check.sh`, `tools/constants_compare.sh`, `tests/test_constants_check.c`). Les deux sens sont proches (un écart à la référence) et ne se confondent pas : le marqueur porte toujours un numéro (`GAP-[0-9]`), le statut jamais.
- **Le registre des écarts est périmé depuis l'étape 1.** Ses chemins sont ceux d'avant l'arborescence du §3, ses noms ceux du cart. La réécriture est le plus gros travail de 5d par le jugement qu'elle demande, pas par sa taille (24 entrées, environ 3 000 mots) ; elle est isolée dans son commit pour que `prd-autopilot` puisse s'y arrêter et reprendre.
- **Pour 5e.** Après 5d, `old-names-guard`, `ast-guard` et `french-guard` sont vertes sur l'arbre réel ; 5e les branche (D-66 (c) : `old-names-guard` et `french-guard` dans `test`, `ast-guard` chaîné à `test-all`) et ajoute leurs violations plantées. `cryptic-locals` et `cart-citations` restent des relevés.
