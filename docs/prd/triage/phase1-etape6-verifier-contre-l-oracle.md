---
title: "Plan 12, étape 6 — Vérifier contre l'oracle"
description: "Les 31 rejeux convertis à 160×120 doivent finir dans l'état figé à 128×128 et garder la couverture de lignes des e2e ; il faut le mesurer, régler chaque écart un par un, et s'arrêter net si un full_game* échoue."
status: needs-triage
---

## Problem Statement

À la fin de l'étape 5, les 31 rejeux (26 e2e de `tests/e2e/`, 5 de `worst-case/`) donnent des positions de souris pour l'écran 160×120, et `tests/plan12-conversion.txt` dit, pour chacun, s'il vise frame par frame la même chose qu'à 128×128 ou à quelle frame et pour quelle cause il a quitté la boucle. Mais viser la même chose ne prouve pas que le jeu fait la même chose : l'égalité des étiquettes est nécessaire, pas suffisante. Une règle qui lit `visible`, une caméra écrêtée en butée, un e2e dont la prémisse est « la planète est hors de l'écran » peuvent tous mener à une autre partie avec les mêmes clics.

L'étape 1 a figé deux témoins sur le code 128×128 (commit `5c6bb25`) :

- `tests/plan12-oracle/<nom>.txt` : l'état normalisé de la dernière frame de chacun des 31 rejeux (sortie de `make state` sans la ligne `camera` ni les champs `screen` et `visible`) ;
- `tests/plan12-oracle/line_coverage.txt` : les 6 445 lignes `fichier:ligne` exécutées par les 26 e2e.

Personne ne les a encore comparés au code 160×120, et deux choses l'empêchent aujourd'hui :

- **Les lignes `zone` et `cap` bougent forcément.** Elles comptent ce qui est dessiné, et le cadre est plus grand ; un `diff` brut avec l'oracle est donc rouge partout, et le vrai écart s'y noie.
- **La couverture est indexée sur l'ancien code.** Les étapes 2 à 4 ont modifié ou déplacé des lignes (`src/cmd/replay.c`, `src/engine/entity_geom.c`, `src/game/hud.c`, `src/game/game_data.c`, `src/game/layout.c`, `src/game/score_panel*.c`, `src/game/ui_button.c`…). `hud.c:40` dans l'oracle n'est plus forcément `hud.c:40` aujourd'hui. Comparer les deux ensembles tels quels signalerait des pertes qui n'en sont pas et en cacherait d'autres.

Enfin, `tests/test_state.c` porte encore un `TODO` qui attend « l'oracle » pour rétablir les assertions de trajectoire et de pics de `message_box` de `trade_rich_mine` ; tant qu'il reste, `make test` ne couvre plus le format des lignes de slipway ni les caps de la boîte de message.

## Solution

On mesure d'abord, puis on règle chaque écart un par un, dans un ordre fixé par le plan.

1. **Garde `full_game*`, avant toute autre action.** Avec les seules commandes existantes (`make state`, la normalisation de `make oracle`, `diff`), on compare les quatre `full_game*` à leur oracle. Si l'un s'arrête, s'écarte hors des lignes `zone`/`cap`, dépasse une limite, ou si `tests/plan12-conversion.txt` le dit sorti de la boucle, l'étape s'arrête là : rien n'est écrit dans le dépôt, et le message de fin d'étape remonte à l'humain le rejeu, la première ligne d'écart et la frame d'arrêt.
2. **Comparer l'état.** Une cible temporaire refait l'état normalisé des 31 rejeux dans `tmp/`, avec la même normalisation que `make oracle`, et le compare à l'oracle : toute ligne hors `zone`/`cap` doit être identique ; une ligne `zone` ou `cap` n'est jugée que sur son pic, qui doit rester inférieur ou égal à sa limite. Elle n'écrit jamais dans `tests/plan12-oracle/`.
3. **Comparer la couverture.** Une sous-commande de `tools/line_coverage.sh` traduit l'ensemble de l'oracle dans la numérotation de HEAD à partir de `git diff -U0 5c6bb25 HEAD -- src` : une ligne hors de tout bloc modifié prend son nouveau numéro ; une ligne dans un bloc modifié ou supprimé est « réécrite », donc hors comparaison. La couverture actuelle des 26 e2e doit contenir l'ensemble traduit, aux lignes acceptées près, chacune avec sa cause.
4. **Régler les écarts.** Un e2e court qui s'écarte de l'oracle, qui est sorti de la boucle de l'étape 5, ou dont la prémisse dépend du hors-écran (`colony_income_offscreen`) est jugé sur la couverture, et réécrit à partir de son `.txt` s'il ne la tient pas. Une ligne de couverture perdue est récupérée en réécrivant l'e2e qui la portait à 128×128, ou acceptée si sa seule cause est le cadre.
5. **Fermer.** Le `TODO` de `test_state` est soldé, comme tout `TODO` que l'étape 5 a renvoyé à l'étape 6. Un rapport versionné, `tests/plan12-verification.txt`, donne le verdict de chaque rejeu et le bilan de couverture. `make test` passe ; `make e2e` reste rouge jusqu'à l'étape 7.

## User Stories

1. En tant qu'humain, je veux que l'étape commence par comparer les quatre `full_game*` à leur oracle, avant d'écrire quoi que ce soit, afin qu'une partie complète cassée m'arrive avant toute réécriture qui pourrait masquer la cause.
2. En tant qu'humain, je veux qu'un `full_game*` que l'étape 5 a sorti de sa boucle m'arrête aussi, même s'il tient l'oracle, afin de décider moi-même de la suite, comme le prévoit le plan.
3. En tant qu'humain, je veux, quand l'étape s'arrête sur un `full_game*`, le nom du rejeu, la cause (arrêt, écart, limite, sortie de boucle), la première ligne d'écart avec ses deux valeurs et la frame d'arrêt s'il y en a une, afin de reprendre sans tout relancer.
4. En tant que développeur du portage, je veux une cible qui refait l'état normalisé des 31 rejeux sur le code courant et le compare à l'oracle, afin de juger chaque rejeu en une commande.
5. En tant que développeur du portage, je veux que cette normalisation soit celle de `make oracle`, écrite une seule fois, afin que l'oracle et la vérification ne puissent pas différer par leur filtre.
6. En tant que développeur du portage, je veux que les lignes `zone` et `cap` ne soient jugées que sur leur pic, inférieur ou égal à leur limite, afin qu'un écart attendu (plus de choses dessinées) ne passe pas pour une régression.
7. En tant que développeur du portage, je veux qu'une ligne `zone` ou `cap` présente dans l'oracle et absente de l'état, ou l'inverse, soit un écart, afin qu'un changement de format ne passe pas sous la règle des limites.
8. En tant que développeur du portage, je veux que toute autre ligne (horloge, ressources, techs, compteurs, planètes, slipways, structures) soit comparée à l'octet près, afin qu'aucune différence de partie ne soit tolérée en silence.
9. En tant que développeur du portage, je veux qu'un rejeu arrêté (`p8_fatal`, état commençant par `stopped`, état absent) soit un échec distinct d'un écart, afin de ne pas confondre un plantage avec une autre trajectoire.
10. En tant que développeur du portage, je veux un verdict par rejeu sur une ligne (`tenu`, `ecart <n>`, `arrete <cause>`) et l'écart complet dans un fichier à part, afin de lire le bilan d'un coup d'œil et le détail au besoin.
11. En tant que relecteur, je veux que la vérification n'écrive jamais dans `tests/plan12-oracle/`, afin que l'oracle décrive toujours le code 128×128.
12. En tant que développeur du portage, je veux traduire l'ensemble de couverture de l'oracle dans la numérotation de HEAD à partir du diff entre `5c6bb25` et HEAD, afin de comparer des lignes qui sont vraiment les mêmes.
13. En tant que développeur du portage, je veux que toute ligne de l'oracle tombée dans un bloc modifié ou supprimé soit écartée de la comparaison et listée avec la cause `réécrite`, afin de respecter « les lignes réécrites aux étapes 3 et 4 sont hors comparaison » sans les perdre de vue.
14. En tant que relecteur, je veux le nombre de lignes écartées par fichier, afin de vérifier que seuls les fichiers touchés par le plan en perdent.
15. En tant que développeur du portage, je veux que la traduction soit testée sur des diffs construits (insertion pure, suppression pure, remplacement, plusieurs blocs, fichier supprimé, fichier non touché), afin qu'une erreur de décalage ne fasse pas croire à une perte ou à un gain.
16. En tant que développeur du portage, je veux que la couverture actuelle des 26 e2e contienne chaque ligne traduite, et non seulement autant de lignes, afin qu'une ligne gagnée ne cache pas une ligne perdue.
17. En tant que développeur du portage, je veux, pour chaque ligne perdue, l'e2e qui la portait à 128×128, afin de savoir lequel réécrire.
18. En tant que développeur du portage, je veux la couverture de chaque e2e sur le code 128×128, mesurée une fois dans un `git worktree` à `5c6bb25`, afin de répondre à la question précédente et de juger un e2e réécrit sur sa propre couverture.
19. En tant que développeur du portage, je veux qu'une ligne perdue dont la seule cause est le cadre (un test de visibilité, un écrêtage de dessin, une branche de placement qui dépend de la taille de l'écran) soit acceptée avec sa cause écrite, afin de ne pas tordre un e2e pour un pixel qui n'existe plus.
20. En tant que développeur du portage, je veux qu'une ligne perdue qui n'est pas due au cadre soit récupérée en réécrivant l'e2e qui la portait, afin que la couverture ne baisse pas.
21. En tant qu'humain, je veux qu'une ligne qu'aucune réécriture dans la limite de frames ne récupère soit listée à part, afin de la régler moi-même à la relecture finale.
22. En tant que développeur du portage, je veux qu'un e2e court qui s'écarte de l'oracle soit d'abord diagnostiqué (première ligne d'écart et pourquoi), afin de distinguer un bug, une règle du jeu qui dépend du cadre, et un simple changement de trajectoire.
23. En tant que développeur du portage, je veux qu'un écart dû à un bug passe par `fix-bug`, afin qu'il soit prouvé par un test et non masqué par une réécriture.
24. En tant que développeur du portage, je veux qu'un écart dû à une règle du jeu qui lit l'écran entre dans `docs/trous-de-portage.md`, et seulement celui-là, afin de suivre la règle du plan.
25. En tant que développeur du portage, je veux qu'un e2e court qui s'écarte pour toute autre raison, ou qui est sorti de la boucle de l'étape 5, soit jugé sur sa couverture, afin de suivre « il est alors jugé sur la couverture ».
26. En tant que développeur du portage, je veux qu'un e2e jugé sur sa couverture qui la tient reste tel que l'étape 5 l'a converti, afin de ne pas réécrire ce qui fait déjà son travail.
27. En tant que développeur du portage, je veux qu'un e2e qui ne la tient pas soit réécrit à partir de son `.txt`, afin de rétablir son intention à 160×120.
28. En tant que développeur du portage, je veux qu'un e2e réécrit n'ait pas plus de frames que sa version d'origine, afin de ne jamais allonger `make e2e`.
29. En tant que lecteur d'un e2e, je veux que le `.txt` d'un e2e réécrit décrive le nouveau `.json` (frames, positions, récit) et garde « Relecture humaine dans raylib : à faire. », afin que la description et le rejeu disent la même chose.
30. En tant que développeur du portage, je veux que `colony_income_offscreen` soit vérifié à sa frame clé (f44, le saut d'année) : `make state` sur le rejeu tronqué à cette frame montre « p27 » avec `visible 0`, et `planet.c:447` reste couvert, afin que sa prémisse tienne vraiment à 160×120.
31. En tant que développeur du portage, je veux que tout autre e2e dont le `.txt` pose une prémisse hors-écran soit vérifié de la même façon, afin de ne pas compter sur le seul nom du fichier.
32. En tant que développeur du portage, je veux que les assertions retirées par le `TODO` de `test_state` reviennent, lues dans l'oracle si `trade_rich_mine` et `cheat_code` le tiennent, sinon dans l'état du rejeu réécrit, afin que `make test` couvre de nouveau le format des slipways et les caps de la boîte de message.
33. En tant que développeur du portage, je veux que tout `TODO` que l'étape 5 a renvoyé à l'étape 6 soit soldé de la même façon, afin que l'étape 7 ne trouve plus de dette du plan.
34. En tant que développeur de l'étape 7, je veux un rapport versionné qui donne, pour chaque rejeu, sa classe, son verdict d'état, son verdict de couverture et ce qui a été fait, afin de rebaseliner en sachant ce qui a été réécrit.
35. En tant qu'humain, je veux que le rapport liste les lignes acceptées avec leur cause et les lignes non récupérées, afin de les relire avec les captures à la fin du plan.
36. En tant que relecteur, je veux qu'aucun `*.sha256` ne soit refait, afin que le rebaseline reste l'affaire de l'étape 7.
37. En tant que développeur du portage, je veux que `make test` passe à chaque commit et ne soit pas plus long, afin que la boucle reste rapide.
38. En tant que développeur du portage, je veux que les cibles et scripts de cette étape soient dans la légende du `Makefile`, marqués temporaires (plan 12), hors de la suite, afin que l'étape 7 les supprime.

## Implementation Decisions

### Classes de rejeux et ordre de jugement

Chaque rejeu entre dans une classe, et la classe fixe ce qui le juge :

| Classe | Rejeux | Juge | Si ça échoue |
|---|---|---|---|
| `full_game*` | `full_game`, `full_game_new_game`, `full_game_pause_restart`, `full_game_pause_idle` | état (oracle), et pas de sortie de boucle à l'étape 5 | arrêt, retour à l'humain |
| écran titre | `title_idle` | état (oracle) | arrêt, retour à l'humain |
| e2e court identique | e2e `identique` dans `tests/plan12-conversion.txt` | état (oracle) | diagnostic, puis jugé sur la couverture |
| e2e court sorti | e2e `sorti` dans le rapport de l'étape 5 | sa couverture | réécriture |
| prémisse hors-écran | `colony_income_offscreen` et tout `.txt` qui pose une prémisse « hors de l'écran » | `visible 0` à la frame clé et sa couverture | réécriture |

Pour toutes les classes : pas d'arrêt (`p8_fatal`). Un arrêt d'e2e court est un bug et passe par `fix-bug`, sans réécriture.

L'ordre est fixé : la garde `full_game*` (et `title_idle`) passe avant tout le reste ; la couverture se mesure ensuite sur les 26 e2e convertis ; les réécritures viennent après les deux mesures, pour qu'une réécriture parte d'un bilan complet.

### La garde

- Avant tout outil nouveau, tout commit et toute réécriture : lire `tests/plan12-conversion.txt`, lancer `make state` (qui écrit `tmp/state/<nom>.txt` pour les 31 rejeux), appliquer à chaque `full_game*` et à `title_idle` le filtre `sed` de la recette `oracle`, puis comparer à `tests/plan12-oracle/` hors lignes `zone`/`cap`, et vérifier le pic ≤ limite des lignes `zone`/`cap`.
- Échec : rien n'est écrit dans le dépôt. Le message de fin d'étape porte le rejeu, la cause, la première ligne d'écart (oracle, actuel) et, pour un arrêt, la frame et le message de `p8_fatal` relevés dans `tmp/state/<nom>.log`.
- Le `full_game` et `game_end_final_score` finissent sur l'écran titre (compteurs à 0) : leur état dit peu de la partie. La garde ne s'en contente pas pour autant : elle ne juge que ce que le plan demande, et la couverture des e2e porte le reste.

### La comparaison d'état

- La normalisation (`sed` qui supprime la ligne `camera` et les champs `screen` et `visible` des lignes `planet` et `structure`) sort de la recette `oracle` dans une variable du `Makefile`, utilisée par `oracle` et par la nouvelle cible : une seule définition.
- Cible `make oracle-check` (temporaire, hors suite) : `make state`, normalisation de chaque `tmp/state/<nom>.txt` dans `tmp/oracle-check/<nom>.txt`, puis comparaison avec `tests/plan12-oracle/<nom>.txt`. Elle écrit `tmp/oracle-check.txt`, une ligne par rejeu, les `full_game*` et `title_idle` d'abord : `<nom> tenu`, `<nom> ecart <n>` (nombre de lignes hors `zone`/`cap` qui diffèrent, plus les `zone`/`cap` au-dessus de leur limite ou absentes d'un côté), ou `<nom> arrete <première ligne de l'état>`. L'écart complet va dans `tmp/oracle-check/<nom>.diff`. Elle échoue si un `full_game*` ou `title_idle` n'est pas `tenu`. Elle n'écrit jamais dans `tests/plan12-oracle/`.
- Le cœur est un script temporaire, `tools/oracle_check.sh compare <oracle> <état normalisé>`, qui écrit le verdict d'un rejeu et son écart. Règles :
  - une ligne qui commence par `zone` ou `cap` est appariée par ses deux premiers champs ; la paire doit exister des deux côtés ; seul le pic (4ᵉ champ) est jugé, contre la limite (5ᵉ champ) de l'état actuel : pic ≤ limite. L'oracle a des pics égaux à la limite (`cap particles_help_screen 0 1 1`, `cap bytes_particles - 1384 1384`) : l'égalité passe. Une limite qui diffère de celle de l'oracle est notée dans l'écart sans compter comme tel : les limites sont des constantes de compilation, et une qui bouge se relit ;
  - toute autre ligne est comparée telle quelle, dans l'ordre ;
  - un état absent ou dont la première ligne commence par `stopped` donne `arrete`.

### La comparaison de couverture

- Nouvelle sous-commande `remap <ensemble> <diff>` de `tools/line_coverage.sh` (déjà temporaire, déjà à sous-commandes, déjà testée hors suite) :
  - entrée : un ensemble `fichier:ligne` et la sortie de `git diff -U0 --no-renames <ancien> <nouveau> -- src` ;
  - pour chaque fichier du diff, les en-têtes `@@ -a,b +c,d @@` : une ligne ancienne l dans [a, a + b − 1] (b > 0) est réécrite ; sinon elle prend l + Σ(d − b) des blocs qui la précèdent ; un fichier supprimé a toutes ses lignes réécrites ; un fichier absent du diff garde ses numéros ;
  - sortie : l'ensemble traduit, trié comme les autres ensembles, sur la sortie standard ; les lignes écartées, une par ligne sous la forme `fichier:ligne réécrite`, dans un second fichier passé en argument.
- Le diff part de `5c6bb25` (le commit de l'oracle, dont le code est celui qui a produit `line_coverage.txt`) et va jusqu'à HEAD au début de l'étape. Il contient aussi les retouches des étapes 2 et 2bis (`src/cmd/replay.c`, `src/devtools/cursor_label.c`) ; elles sont écartées comme les autres, et le décompte par fichier le montre. Si une ligne écartée tombe dans un fichier que ni les étapes 2 à 4 ni le plan ne citent, c'est un signal à relire (voir H-5-2).
- Cible `make coverage-check` (temporaire, hors suite) : le diff dans `tmp/coverage-check/src.diff`, `remap` de l'oracle, `cover` des 26 e2e actuels, puis `gap <traduit> <acceptées> <actuel>` (sous-commande existante). `<acceptées>` est `tests/plan12-coverage-accepted.txt`, versionné le temps du plan, une ligne `fichier:ligne cause`. La cible écrit le bilan (lignes de l'oracle, écartées, traduites, couvertes aujourd'hui, manquantes, acceptées, gagnées) et échoue si `gap` n'est pas vide.
- Couverture par e2e à 128×128 : une seule fois, dans un `git worktree` à `5c6bb25` (dans `tmp/`), `make line-coverage EVENTS=<e2e>` pour chacun des 26 e2e d'alors, puis `remap` de chacun. Ces 26 ensembles restent dans `tmp/coverage-check/old/` ; ils ne sont pas versionnés. Ils disent quel e2e portait une ligne perdue (`contrib` ou simple appartenance) et servent de référence à un e2e jugé sur sa couverture. Le worktree est retiré ensuite.
- Un e2e jugé sur sa couverture la tient si `gap <son ensemble ancien traduit> <acceptées> <son ensemble actuel>` est vide.

### Régler une ligne perdue

Pour chaque ligne du `gap` des 26 e2e :

1. Chercher l'e2e (ou les e2e) qui la portaient à 128×128.
2. Si la ligne n'est atteinte que par une condition de cadre (`entity_geom_is_on_screen` et ses appelants, écrêtage de `p8/gfx.c`, placement qui lit `P8_SCREEN_WIDTH`/`HEIGHT`), et que l'e2e qui la portait ne la vise pas dans son `.txt`, elle est acceptée : ligne `fichier:ligne cadre: <explication courte>` dans `tests/plan12-coverage-accepted.txt`.
3. Sinon, l'e2e qui la portait est réécrit pour la reprendre, dans sa limite de frames.
4. Si aucune réécriture dans la limite ne la reprend, elle est acceptée avec la cause `non récupérée: <raison>` et listée à part dans le rapport, pour l'humain.

### Régler un e2e court qui s'écarte de l'oracle

- Diagnostic d'abord, une ligne dans le rapport : la première ligne d'écart et sa cause, en s'aidant de la frame de première divergence d'état (`make state` sur des copies tronquées dans `tmp/`).
- Bug (le jeu fait une chose fausse, quel que soit le cadre) → `fix-bug`, puis on revérifie.
- Règle du jeu qui lit l'écran (un état de partie, pas un dessin, qui dépend de `visible` ou de la taille du cadre) → entrée dans `docs/trous-de-portage.md`, selon la règle du plan, et l'e2e est jugé sur sa couverture.
- Autre cause (trajectoire changée par la conversion) → jugé sur sa couverture.

### Réécrire un e2e court

- Le nouveau `.json` est écrit pour 160×120, au même format compact que les autres (une frame par ligne, `dpad` de 4 booléens), à partir de l'intention du `.txt` : mêmes actions, même cible, mêmes lignes visées.
- Son nombre de frames est inférieur ou égal à celui de la version d'origine ; `tools/e2e_limits.sh` le contrôle (il compte les `"mouse_x"`).
- Il doit : tenir sa couverture (ci-dessus) ; ne pas s'arrêter ; pour une prémisse hors-écran, montrer `visible 0` sur la planète visée à la frame clé.
- Son `.txt` est réécrit pour décrire le nouveau rejeu, au même format ; la ligne « Relecture humaine dans raylib : à faire. » reste.
- Son `.json.sha256` n'est pas touché ; `make e2e` le donnera rouge jusqu'à l'étape 7.

### La prémisse hors-écran

- `colony_income_offscreen` : frame clé f44 (saut d'année), planète visée « p27 ». Contrôle : `make state` sur une copie du `.json` tronquée après f44, dans `tmp/` ; la ligne `planet p27` porte `visible 0` ; et `planet.c:447` (le retour sur `gd`, nommé par le `.txt`) est dans sa couverture actuelle, au numéro traduit. Si les deux tiennent, le rejeu converti fait office de réécriture et n'est pas modifié.
- La conversion de l'étape 5 conserve le delta du glisser de caméra ((117,117) → (11,11), soit 106 sur chaque axe) : la caméra finit vers (282,302), et « p27 » vers (−2,−73) à l'écran. Le contrôle devrait donc passer, mais il se fait.
- Aucun e2e n'atteint GAP-26 (`docs/trous-de-portage.md` le dit) : aucun cas GAP-26 à vérifier. Les autres `.txt` sont relus pour une prémisse « hors de l'écran » ; tout cas trouvé suit le même contrôle.

### Les tests unitaires

- `test_state`, cas `trade_rich_mine` : le `TODO` part. Les assertions qu'il nomme (compteurs, `p1` colonisée, `p2` inconnue, slipways `p27-p1` et `p1-s1`, pics `cap message_box` ≥ 1 et `cap bytes_message_box` > 0) reviennent avec les valeurs de l'oracle si `trade_rich_mine` et `cheat_code` le tiennent ; sinon avec celles de l'état actuel du rejeu réécrit, et le rapport le dit. `camera world 176 196` reste, ou prend la valeur lue si la caméra bouge.
- Tout autre `TODO` qui renvoie à l'étape 6 (posé par l'étape 5, listé dans son rapport) est soldé : l'assertion d'origine revient si le rejeu tient l'oracle ; sinon elle prend la valeur du rejeu réécrit.
- Aucun test n'entre dans `UNIT_TESTS`.

### Le rapport

- `tests/plan12-verification.txt`, versionné, temporaire (supprimé à l'étape 7 avec l'oracle, `tests/plan12-conversion.txt` et `tests/plan12-coverage-accepted.txt`). Un en-tête de format, puis :
  - une ligne par rejeu : `<nom> <classe> etat <tenu|ecart n|arrete> couverture <tenue|n manquantes|-> <garde|reecrit|diagnostic: …>` ;
  - le bilan de couverture de `make coverage-check`, avec les lignes écartées par fichier ;
  - la liste des lignes acceptées par cause (`cadre`, `non récupérée`) ;
  - la liste des réécritures (e2e, frames avant → après) et des entrées ajoutées à `docs/trous-de-portage.md`, ou `aucune`.

### Ce qui ne bouge pas

L'oracle (`tests/plan12-oracle/`), `tests/plan12-cursor-labels/`, `tests/plan12-conversion.txt`, `tools/cursor_anchors.txt`, les `*.sha256`, et le code du jeu hors correctifs passés par `fix-bug`. `make oracle` n'est pas relancé : il réécrirait l'oracle.

## Implementation Chunks

Chaque chunk qui commite garde `make test` vert.

0. **La garde (aucun commit).** Lecture de `tests/plan12-conversion.txt`, `make state`, comparaison des `full_game*` et de `title_idle` avec les commandes existantes. Échec → arrêt et retour à l'humain ; la suite ne se fait pas.
1. **La comparaison d'état.** Variable de normalisation partagée avec `oracle`, `tools/oracle_check.sh`, son test hors suite (`make oracle-check-test`), la cible `make oracle-check`, les lignes de légende. On la lance ; son résultat n'est pas commité.
2. **La comparaison de couverture.** Sous-commande `remap` et ses cas dans `tests/test_line_coverage.c`, cible `make coverage-check`, `tests/plan12-coverage-accepted.txt` vide, légende. Couverture par e2e dans le worktree à `5c6bb25`. On la lance.
3. **Les écarts.** Un commit par e2e réécrit (`.json`, `.txt`, ligne d'acceptation éventuelle) ; un par correctif `fix-bug` ; un par entrée de trou de portage. `colony_income_offscreen` contrôlé à sa frame clé.
4. **La fermeture.** `test_state` et les autres `TODO` de l'étape 6 soldés, `tests/plan12-verification.txt` écrit, `make oracle-check` et `make coverage-check` verts (hors e2e réécrits pour le premier), `make test` vert.

## Testing Decisions

- **Un bon test** vérifie ce qu'un script écrit à partir d'entrées construites (verdict, écart, ensemble traduit, code de sortie). Il ne relit pas le script.
- **`tools/oracle_check.sh`** : `tests/test_oracle_check.c`, binaire hors suite (`make oracle-check-test`), sur le modèle de `tests/test_state_peaks.c` et `tests/test_line_coverage.c` (fichiers écrits dans `tmp/` par le test, script lancé par `system`). Cas :
  - état identique → `tenu` ;
  - une ligne `planet` ou `count` différente → `ecart 1` ;
  - une ligne `zone`/`cap` dont les valeurs courante et pic changent, pic ≤ limite → `tenu` ; pic = limite → `tenu` ; pic > limite → `ecart` ;
  - une ligne `zone`/`cap` présente d'un seul côté → `ecart` ;
  - un pic avec un courant `-` (`cap elements_*`) se lit ;
  - état absent, état `stopped …` → `arrete`.
- **`remap`** : nouveaux cas dans `tests/test_line_coverage.c` (`make line-coverage-test`), diffs écrits à la main : insertion pure avant, après et au milieu ; suppression pure ; remplacement de même et de différente longueur ; plusieurs blocs dans un fichier ; fichier supprimé ; fichier nouveau ; fichier non touché ; ensemble vide. Chaque cas vérifie l'ensemble traduit et le fichier des réécrites.
- **Vérification de bout en bout** : `make coverage-check` doit donner, sur le diff vide (HEAD contre HEAD) et l'ensemble de l'oracle, un ensemble traduit identique à l'entrée ; c'est un test du câblage fait une fois à la main, pas une cible.
- **Non-régression** : `make test` vert à chaque commit et pas plus long ; aucun fichier de `tests/plan12-oracle/`, `tests/plan12-cursor-labels/` ou `tests/plan12-conversion.txt` modifié ; aucun `*.sha256` modifié ; `tools/e2e_limits.sh` passe sur les e2e réécrits (hors `.sha256`, déjà présents).
- **Ce qui n'est pas une condition de fin** : `make e2e` (rouge jusqu'à l'étape 7), `make full-game` tel quel n'est pas visé mais doit passer de fait, puisque la garde a vérifié que les `full_game*` ne s'arrêtent pas.
- **Art antérieur** : `tools/line_coverage.sh` (`cover`, `diff`, `contrib`, `gap`) et `tests/test_line_coverage.c` ; `tools/state_peaks.sh` et `tests/test_state_peaks.c` ; la recette `oracle` du `Makefile` ; `tests/test_state.c` (assertions `has(...)`).

## Hypothèses sur l'issue de l'étape 5

L'étape 5 est en cours : sa PRD est en `docs/prd/triage/`. Ce qui précède suppose l'issue décrite par cette PRD. Si une hypothèse tombe, revérifier les sections listées.

- **H-5-1** — `tests/plan12-conversion.txt` existe, versionné, au format de la PRD de l'étape 5 : une ligne par rejeu, `identique` ou `sorti <frame> <cause> <ancienne> <nouvelle>`, puis `butee …` et `recadre …`, et une section des corrections de table. À revérifier : Classes de rejeux, La garde, Le rapport.
- **H-5-2** — L'étape 5 ne change aucun fichier de `src/` (sa PRD l'interdit, hors correctif d'un `p8_fatal`). Le diff `5c6bb25..HEAD -- src` ne contient donc que les étapes 2, 2bis, 3 et 4. Si l'étape 5 a corrigé un bug dans `src/`, ses lignes sont écartées aussi, et le décompte par fichier les montre ; on le note dans le rapport. À revérifier : La comparaison de couverture.
- **H-5-3** — Les 31 rejeux sont convertis en place, sous le même nom, avec le même nombre de frames, et leurs `.txt` mis à jour. À revérifier : Réécrire un e2e court (limite de frames), La prémisse hors-écran.
- **H-5-4** — Aucun rejeu converti ne s'arrête (`p8_fatal`) : l'étape 5 s'arrête sinon. Si l'un s'arrête quand même, c'est un bug, traité par `fix-bug` avant la comparaison. À revérifier : La garde, Classes de rejeux.
- **H-5-5** — Aucun `full_game*` n'est sorti de la boucle. Le relevé de l'étape 5 ne prévoit de sortie que pour `camera_drag_dpad` (butée, frame 15). Si un `full_game*` est sorti, la garde arrête l'étape. À revérifier : La garde.
- **H-5-6** — Le `TODO` de `test_state` (`trade_rich_mine`) est toujours là, en tout ou en partie : l'étape 5 a pu rétablir les assertions de trajectoire si le rejeu converti les tenait, mais pas les pics de `message_box`. Les autres `TODO` qu'elle a posés renvoient à l'étape 6 et sont listés dans son rapport. À revérifier : Les tests unitaires.
- **H-5-7** — `make state`, la recette `oracle`, `tools/line_coverage.sh` et `tmp/replay_gcov` sont inchangés par l'étape 5, et `make state` écrit toujours `tmp/state/<nom>.txt` et `.log`. À revérifier : La garde, La comparaison d'état, La comparaison de couverture.
- **H-5-8** — `colony_income_offscreen` converti garde le delta du glisser de caméra (glisser au clic droit sur la carte, deux bouts étiquetés `carte`), donc la caméra finit vers (282,302) et « p27 » hors de l'écran. À revérifier : La prémisse hors-écran.
- **H-5-9** — `camera_drag_dpad` est sorti de la boucle (cause « butée ») ou identique ; dans les deux cas c'est un e2e court, jugé selon sa classe. À revérifier : Classes de rejeux.

## Out of Scope

- `make e2e-baseline`, la suppression des outils temporaires, de l'oracle, des étiquettes et des rapports, le `README` : étape 7.
- Rendre `make e2e` vert.
- Relancer `make oracle`, `make cursor-labels` ou la boucle de conversion de l'étape 5.
- Juger le rendu : la relecture des captures (`empty_planet_fade`, `project_tooltip_inputs`…) et la partie dans raylib sont pour l'humain, au critère de fin du plan.
- Toute décision sur un `full_game*` qui échoue : elle revient à l'humain.
- Retoucher le jeu ou l'interface pour faire tenir un rejeu, hors correctif de bug prouvé par `fix-bug`.
- PSX, audio.

## Further Notes

- **Ce qu'on attend.** Les règles du jeu ne lisent l'écran qu'à travers `visible` (toasts de revenu, cibles de `planet_draw`), et l'état comparé n'en contient pas. La plupart des e2e identiques à l'étape 5 devraient tenir l'oracle. La couverture est le témoin le plus fragile : des branches d'écrêtage de `p8/gfx.c` ou de `planet_display.c` peuvent ne plus être atteintes avec un cadre plus large, d'où la cause `cadre`.
- **Le `full_game` et `game_end_final_score` finissent sur l'écran titre.** Leur oracle (39 lignes, compteurs à 0) ne prouve guère que l'absence d'arrêt et le respect des limites. C'est ce que le plan demande ; on ne l'étend pas.
- **Sans humain disponible, les points que `to-prd` fait valider ont été tranchés selon `guideline`** (le plus simple qui tienne, hypothèses écrites) :
  - **Garde avec les commandes existantes**, sans outil nouveau : le plan dit « avant toute autre action », et écrire puis commiter un outil en est une. Les commandes (`make state`, le `sed` de `oracle`, `diff`) suffisent pour quatre fichiers.
  - **`title_idle` arrête aussi l'étape** s'il échoue, bien qu'il ne soit pas un `full_game*` : il n'a pas de `.txt` pour être réécrit, et c'est un rejeu de `worst-case/` comme eux. C'est l'option prudente.
  - **Un `full_game*` sorti à l'étape 5 arrête l'étape même s'il tient l'oracle** : le plan dit, à l'étape 5, qu'il est « remonté à l'humain ».
  - **Un e2e sorti à l'étape 5 n'est pas réécrit d'office** : le plan dit qu'il est « traité comme un e2e court réécrit », c'est-à-dire jugé sur la couverture ; on ne le réécrit que s'il ne la tient pas.
  - **Inclusion ligne à ligne, pas un compte** : « ne baisse pas » se lit sur l'ensemble ; un compte laisserait une ligne gagnée cacher une ligne perdue.
  - **Mise en correspondance par `git diff -U0`** plutôt que par contenu de ligne : une ligne identique déplacée dans un bloc réécrit est écartée à tort, ce qui est prudent (on compare moins) ; une correspondance par contenu se tromperait sur les lignes répétées (`}`, `return 0;`).
  - **Tout le diff de `5c6bb25` à HEAD est écarté**, y compris les retouches de devtools des étapes 2 et 2bis : séparer par commit coûterait plus qu'il ne rapporte, et le décompte par fichier montre ce qui est écarté.
  - **Couverture par e2e mesurée dans un worktree à `5c6bb25`**, non versionnée : c'est la seule façon de savoir quel e2e portait une ligne, et elle ne sert qu'à cette étape.
  - **Pic ≤ limite**, pas « < » : l'oracle a déjà des pics égaux à leur limite, et le code s'arrête au dépassement, pas à l'égalité.
  - **Lignes acceptées pour cause de cadre** : le plan exige que la couverture ne baisse pas, mais aussi que l'affichage ne soit pas un trou de portage. Une ligne qui n'existe que parce que l'écran faisait 128 de large n'a pas de scénario à 160 ; on l'écrit avec sa cause plutôt que d'inventer un e2e. Ce que l'agent n'arrive pas à reprendre est listé à part pour l'humain, conformément au critère de fin (« écarts réglés un par un »).
  - **Tests** : `oracle_check.sh` et `remap` sont testés hors suite, parce qu'une erreur de filtre ou de décalage fausserait tout le jugement sans se voir. Les cibles n'ont pas de test propre : c'est de la plomberie `Makefile` sur le modèle d'`oracle` et de `line-coverage`.
  - **Rapport dans `tests/`, pas dans `docs/`** : comme l'oracle et le rapport de conversion, c'est une donnée temporaire lue par l'étape 7.
