---
title: "Plan 12, étape 2 — Étiqueter les rejeux sur le code actuel"
description: "Pour convertir les rejeux à 160×120, il faut savoir, frame par frame, si le curseur vise la carte ou un élément d'interface et lequel : rien ne l'écrit aujourd'hui, et après le changement d'écran on ne pourra plus le retrouver."
status: needs-triage
---

## Problem Statement

Le plan 12 passe le framebuffer de 128×128 à 160×120. Les 31 rejeux (26 e2e de `tests/e2e/`, 5 de `worst-case/`) donnent la position du curseur en coordonnées d'écran, frame par frame. Ces coordonnées ne bougent pas toutes de la même façon (plan, §2) : sur la carte, le curseur prend +16 / −4, puisque la caméra garde le même centre monde ; sur l'interface, il suit le bord auquel l'élément visé est ancré (rien à gauche ou en haut, +32 à droite, −8 en bas, +16 / −4 au centre).

Pour convertir un rejeu à l'étape 5, il faut donc savoir **ce qui est sous le curseur à chaque frame**. Aujourd'hui, le jeu le calcule à chaque frame (le hit test de `ui_update_hover`, puis l'ordre de distribution par couches de `ui_send_to_hovered`), mais ne l'écrit nulle part : ni `make state`, ni le dump, ni la capture ne le disent. Et cette information n'existe que sur le code actuel : une fois l'écran changé, un rejeu non converti vise autre chose, et on ne peut plus savoir ce qu'il visait.

Deux autres difficultés :

- **Une classe ne suffit pas toujours.** La classe `button` sert aussi bien aux quatre boutons du menu de la galaxie — deux en haut, un en haut à droite (118,0), un en bas à gauche (0,117) — qu'aux boutons des boîtes de message et du panneau de score, centrés. Une table « classe → ancre » au sens strict ne peut pas donner deux ancres à `button`.
- **Un élément peut être ancré sur deux bords.** Le bouton en haut à droite prend +32 en x et rien en y ; un élément en bas à droite prendrait +32 et −8. Les cinq mots du plan (gauche, droite, haut, bas, centre) ne disent pas « bas à droite » ni « centré en haut ».

## Solution

Sur le code actuel, sans toucher au jeu :

1. Un module de `devtools/` donne l'**étiquette du curseur** : le nom de l'entité qui reçoit en premier un clic à cet endroit — la première entité de la plus haute couche non vide que le hit test existant a trouvée —, ou `carte` quand c'est le fond (`background`) ou qu'aucune entité ne répond. Les boutons du menu de la galaxie portent un qualificatif (`button:galaxy_menu_1` … `_4`) ; tout autre bouton s'appelle `button`.
2. Le pilote de rejeu `tmp/replay`, avec la variable d'environnement `LABELS=<fichier>`, écrit une étiquette par frame, juste après la mise à jour de la frame : la ligne n+1 du fichier est l'étiquette de la frame n. Sans la variable, rien ne change.
3. Une **table des ancres** accompagne l'outil : pour chaque étiquette possible, son ancre horizontale (gauche, centre, droite) et verticale (haut, centre, bas), ou `carte`. Les cinq mots du plan en sont des cas particuliers.
4. Une cible `make` écrit **un fichier d'étiquettes par rejeu** pour les 31 rejeux, dans un répertoire versionné le temps du plan. Elle échoue si un rejeu s'arrête ou si une étiquette produite n'a pas d'entrée dans la table.

L'étape 5 relit ces fichiers avec la table pour convertir chaque `events.json`, puis relance le même outil sur le nouveau code avec les rejeux convertis : la première ligne qui diffère désigne la frame, donc l'ancre, à corriger.

Le module, le crochet du pilote, la table, la cible et les fichiers d'étiquettes sont temporaires : supprimés à l'étape 7.

## User Stories

1. En tant que développeur du portage, je veux savoir, pour chaque frame d'un rejeu, ce qui est sous le curseur, afin de convertir sa position à 160×120 selon ce qu'il vise.
2. En tant que développeur du portage, je veux que cette information vienne du hit test existant du jeu, afin de ne pas réécrire à côté une seconde définition de « ce qui est sous le curseur », qui pourrait diverger de la vraie.
3. En tant que développeur du portage, je veux que l'étiquette désigne l'entité qui reçoit le clic en premier (la plus haute couche, puis l'ordre de la liste), afin qu'un bouton posé au-dessus d'une planète soit étiqueté `button` et non `planet`.
4. En tant que développeur du portage, je veux que le fond étoilé s'étiquette `carte`, afin qu'un clic ou un glisser sur le vide de la carte prenne le décalage de la carte.
5. En tant que développeur du portage, je veux que toute entité de la lignée des planètes (planète, structure, plateforme commerciale, processeur, ascension, protoétoile, synthétiseur, sursaut, slipgate, porte d'infraespace) s'étiquette de son nom de classe, afin de vérifier à l'étape 5 que le rejeu converti vise la même sorte de chose.
6. En tant que développeur du portage, je veux que les quatre boutons du menu de la galaxie aient chacun leur étiquette, afin de leur donner des ancres différentes (haut à droite, haut à gauche, haut, bas à gauche).
7. En tant que développeur du portage, je veux que les autres boutons partagent l'étiquette `button`, afin que la table reste courte tant que rien ne prouve qu'il faut les distinguer.
8. En tant que développeur du portage, je veux que le fond modal, l'écran titre et les icônes de structure aient leur étiquette (`modal_backdrop`, `help_screen`, `structure_icon`), afin que chacun reçoive une ancre.
9. En tant que développeur du portage, je veux que l'étiquette ne contienne aucune coordonnée, afin qu'un rejeu converti, relancé sur le nouveau code, puisse rendre exactement les mêmes étiquettes.
10. En tant que développeur du portage, je veux une ligne par frame, et rien d'autre, afin que la première ligne qui diffère entre deux fichiers donne directement la frame qui diverge.
11. En tant que développeur du portage, je veux que l'étiquette soit prise juste après la mise à jour de la frame, avant le dessin, afin qu'elle décrive le hit test qui a servi à distribuer le clic de cette frame.
12. En tant que développeur du portage, je veux que l'outil passe par le pilote de rejeu existant, afin que les étiquettes soient prises sur exactement la trajectoire que rejouent `make e2e`, `make state` et l'oracle de l'étape 1.
13. En tant que développeur du portage, je veux que le pilote, sans `LABELS`, se comporte octet pour octet comme avant, afin que `make e2e` reste vert et que ses `*.sha256` ne bougent pas.
14. En tant que développeur du portage, je veux que l'outil ne modifie rien de l'état du jeu (aucune écriture, aucune allocation dans une zone du jeu), afin qu'étiqueter un rejeu ne change pas ce qu'il fait.
15. En tant que développeur du portage, je veux que l'outil échoue bruyamment devant une classe testable au curseur qu'il ne sait pas nommer, afin qu'aucune frame ne reçoive une étiquette inventée.
16. En tant que développeur du portage, je veux une table qui donne, pour chaque étiquette possible, une ancre horizontale et une ancre verticale, ou `carte`, afin de couvrir les éléments ancrés dans un coin ou centrés sur un seul axe.
17. En tant que développeur du portage, je veux que les cinq mots du plan restent valables comme raccourcis (gauche = gauche/haut, droite = droite/haut, haut = gauche/haut, bas = gauche/bas, centre = centre/centre), afin que la table se lise dans les termes du plan.
18. En tant que développeur du portage, je veux que la table liste toutes les étiquettes que l'outil peut produire, même celles qu'aucun rejeu n'atteint, afin que l'étape 5 ne bute pas sur une entrée manquante.
19. En tant que développeur du portage, je veux que le décalage de chaque ancre soit écrit une seule fois, avec la table (x : 0 / +16 / +32 ; y : 0 / −4 / −8 ; carte : +16 / −4), afin que l'étape 5 ne le redéfinisse pas ailleurs.
20. En tant que développeur du portage, je veux pouvoir corriger une ancre dans la table sans régénérer les étiquettes, afin que la boucle de l'étape 5 (convertir, comparer, corriger l'ancre) ne demande jamais de revenir au code ancien.
21. En tant que développeur du portage, je veux un fichier d'étiquettes par rejeu, nommé comme le rejeu, afin de retrouver d'un coup d'œil celui d'un e2e donné.
22. En tant que développeur du portage, je veux que les 31 fichiers soient produits par une seule cible `make`, afin de ne pas en oublier.
23. En tant que développeur du portage, je veux que la cible échoue si un rejeu s'arrête, afin de ne jamais versionner un fichier tronqué.
24. En tant que développeur du portage, je veux que la cible échoue si un fichier n'a pas autant de lignes que son rejeu a de frames, afin qu'un décalage d'une frame se voie à la production plutôt qu'à l'étape 5.
25. En tant que développeur du portage, je veux que la cible échoue si une étiquette produite n'a pas d'entrée dans la table, afin que la table et les fichiers restent d'accord.
26. En tant que développeur du portage, je veux pouvoir étiqueter un seul rejeu (`EVENTS=<fichier>`) dans `tmp/`, afin qu'à l'étape 5 on étiquette un rejeu converti et qu'on le compare au fichier versionné.
27. En tant que développeur du portage, je veux que les fichiers d'étiquettes soient versionnés, afin qu'ils survivent aux `make clean`, aux branches et aux sessions jusqu'à l'étape 5.
28. En tant que développeur du portage, je veux que deux productions successives donnent des fichiers identiques, afin d'être sûr qu'ils ne dépendent ni de l'ordre ni du parallélisme de `make`.
29. En tant que développeur du portage, je veux que le module, son test et la cible ne soient ni dans `make test` ni dans `make e2e`, afin que la boucle rapide reste rapide (règle du projet).
30. En tant que développeur du portage, je veux que les cibles apparaissent dans la légende du `Makefile` avec la mention « temporary (plan 12), outside the suite », afin que l'étape 7 les retrouve et les supprime.
31. En tant que relecteur, je veux que le diff de cette étape ne touche ni `src/game/`, ni `src/engine/`, ni `src/p8/`, ni `src/platform/`, afin de m'assurer que les étiquettes ont été prises sur le code d'avant le changement.
32. En tant que développeur de l'étape 5, je veux qu'un humain puisse lire un fichier d'étiquettes sans outil, afin de comprendre ce que vise un e2e en regard de son `.txt`.

## Implementation Decisions

- **Module de l'étiquette** (nouveau, dans `devtools/`, nom indicatif `cursor_label`). Interface : une fonction qui prend l'interface principale (`ui_t`, éventuellement absente) et rend l'étiquette sous forme de chaîne statique ; elle ne lit que `hovered_by_layer` et le champ `class` des entités qu'il liste — et, pour un bouton, son `on_click` —, n'écrit rien et n'alloue rien. Règle :
  - interface absente, `hovered_by_layer` absent, ou toutes les couches vides : `carte` ;
  - sinon, la première entité de la plus haute couche non vide (15 → 0), c'est-à-dire celle à qui `ui_send_to_hovered` offre l'événement en premier ;
  - `background` → `carte` ;
  - les dix classes de la lignée des planètes → leur nom, le même que celui qu'écrit `make state` (`planet`, `structure`, `trading_hub`, `processor`, `ascension`, `protostar`, `synthesizer`, `burst`, `slipgate`, `infraspace_gate`) ;
  - `button` dont l'`on_click` vaut `galaxy_menu_on_click(i)` → `button:galaxy_menu_<i>` ; tout autre bouton → `button` ;
  - `modal_backdrop`, `help_screen`, `structure_icon` → ces noms ;
  - toute autre classe : `assert` porteur de message (une classe testable au curseur que l'outil ne sait pas nommer).
  On ne prend pas `ui->hovered` : c'est la dernière entité trouvée par le hit test, pas celle qui reçoit le clic.
- **Crochet du pilote de rejeu.** Le pilote existant (`cmd/replay`) lit `LABELS=<fichier>`, sur le modèle de `STATE` et `REPLAY_FILL`. Si la variable est posée, il écrit après chaque `cart_update60()` et avant `cart_draw()` l'étiquette de la frame, suivie d'un saut de ligne. L'instant est sûr : les entités ne sont rendues qu'au `cart_flush_exit_queue()` du début de la frame suivante, et la liste par couches vit dans la zone de frame jusqu'au `frame_zone_reset()` suivant. Un fichier qui ne s'ouvre pas : message sur stderr et code 1. Sans la variable, le pilote ne fait rien de plus. Le module rejoint `REPLAY_SRCS`.
- **Format du fichier d'étiquettes.** Texte, une étiquette par ligne, rien d'autre : ligne n+1 = frame n ; autant de lignes que l'`events.json` a de frames. Ni coordonnées, ni boutons, ni numéro de frame.
- **Table des ancres** (nouveau fichier texte temporaire, dans `tools/`). Une ligne par étiquette : `<étiquette> <x> <y>` avec x ∈ {gauche, centre, droite} et y ∈ {haut, centre, bas}, ou `<étiquette> carte`. Quelques lignes d'en-tête (`#`) donnent les décalages, seule définition du plan 12 : x gauche 0, centre +16, droite +32 ; y haut 0, centre −4, bas −8 ; carte +16 / −4. Le contenu de départ est un brouillon que l'étape 5 corrige :
  - `carte` et les dix classes de la lignée des planètes : `carte` ;
  - `button:galaxy_menu_1` (118,0) : droite haut ; `button:galaxy_menu_2` (0,0) : gauche haut ; `button:galaxy_menu_3` (47,0) : centre haut ; `button:galaxy_menu_4` (0,117) : gauche bas ;
  - `button`, `modal_backdrop`, `help_screen` : centre centre ;
  - `structure_icon` : droite bas (position de repos (118,120)), à confirmer à l'étape 5.
  La table ne dépend pas des fichiers d'étiquettes : la corriger ne demande pas de les régénérer.
- **Cibles `make`** (noms indicatifs, toutes « temporary (plan 12), outside the suite » dans la légende) :
  - `cursor-labels EVENTS=<events>` : étiquettes de ce rejeu dans `tmp/` ;
  - `cursor-labels` sans `EVENTS` : les 31 rejeux (`E2E_EVENTS` et `FULL_GAME_EVENTS`), un fichier `<nom du rejeu>.txt` chacun dans le répertoire versionné ; échoue si un rejeu sort en erreur, si un fichier n'a pas autant de lignes que son rejeu a de frames, ou si une étiquette produite manque à la table. Les contrôles tiennent dans la recette, sans script à part ;
  - `cursor-labels-test` : construit et lance le test du module, jamais lancé par `make test` ni `make test-all`.
- **Répertoire des étiquettes.** Un seul répertoire versionné, hors de `tmp/`, nommé d'après le plan pour être supprimé d'un bloc à l'étape 7, distinct de celui de l'oracle (voir H-1-2). Il contient les 31 fichiers d'étiquettes, rien d'autre.
- **Aucun changement du jeu.** Cette étape ne touche ni `src/game/`, ni `src/engine/`, ni `src/p8/`, ni `src/platform/`. Elle ajoute un module à `src/devtools/`, un crochet à `src/cmd/replay.c`, un fichier à `tools/`, un test, des cibles et le répertoire des étiquettes.
- **Commentaires.** Règle du `CLAUDE.md` : le crochet et le module ne portent que le contrat non dit par le nom (l'instant de la prise, la chaîne statique, ce que rend une interface absente) et l'`assert` à message.

## Implementation Chunks

1. **Le module de l'étiquette** — un cycle TDD. Le test d'abord, sous `make cursor-labels-test` : rouge (module absent). Puis le module : vert. Terminé quand `make cursor-labels-test` passe et que `make test` lance exactement les mêmes tests qu'avant.
2. **Le crochet, la table et la production** — dépend du chunk 1. Le cas d'intégration du test (un rejeu de fixture avec `LABELS`) d'abord, rouge ; puis le crochet du pilote : vert. Ensuite la table, la cible `cursor-labels` et la production des 31 fichiers, acceptée sur les vérifications listées dans Testing Decisions (« Les fichiers d'étiquettes »). Terminé quand `make test` et `make e2e` passent sans qu'aucun `*.sha256` ne change.

## Testing Decisions

- **Un bon test** vérifie l'étiquette rendue pour une situation donnée, pas la façon de parcourir les couches : des interfaces construites à la main avec des entités de classes réelles posées sur des couches choisies, et un rejeu réel dont on lit le fichier produit.
- **Module de l'étiquette** — le seul module testé, sous `make cursor-labels-test`, hors de `make test` et de `make test-all`. Cas :
  - interface absente → `carte` ; couches toutes vides → `carte` ; fond seul → `carte` ;
  - planète sur une couche au-dessus du fond → `planet` ;
  - bouton au-dessus d'une planète → `button` ;
  - deux entités sur la même couche la plus haute → la première de la liste ;
  - bouton dont l'`on_click` est celui du menu de la galaxie `i` → `button:galaxy_menu_<i>`, pour les quatre ;
  - chacune des classes nommées → son nom (cas tabulé) ;
  - l'outil ne modifie pas l'interface qu'on lui passe.
- **Crochet du pilote** — dans le même test : rejouer une fixture de l'écran titre (`tests/fixtures/title_click_reasonable.json`) avec `LABELS` donne autant de lignes que de frames, la première étant `help_screen` ; sans `LABELS`, le dump est identique à celui d'un rejeu avec `LABELS`.
- **Les fichiers d'étiquettes** — vérifiés à la production : 31 fichiers ; chacun a autant de lignes que son rejeu a de frames ; toute étiquette produite figure dans la table ; deux productions successives sont identiques ; la carte et au moins une étiquette d'interface apparaissent (un fichier entièrement `carte` trahirait un crochet mal placé). Lecture humaine d'un e2e court en regard de son `.txt` (par exemple `camera_drag_dpad` : clic sur l'écran titre, puis glisser au clic droit sur le fond).
- **Non-régression** — `make test` et `make e2e` passent, aucun `*.sha256` ne change, et la liste des tests de `make test` est la même qu'avant l'étape.
- **Art antérieur** — `tests/test_ui.c` (interfaces et entités construites à la main) ; `tests/test_replay_from_title.c` et `tests/test_headless_capture.c` (tests qui lancent le vrai pilote sur une fixture) ; `tests/test_line_coverage.c` et sa cible `line-coverage-test` (test d'un outil temporaire du plan 12, hors de la suite).

## Hypothèses sur l'issue de l'étape 1

L'étape 1 est en cours : seul son chunk 1 (la mesure de couverture de lignes et les cibles `line-coverage`, `line-coverage-test`) est commité. Ce qui suit suppose une issue de son chunk 2 (production de l'oracle) conforme à sa PRD. Si une hypothèse tombe, revérifier les sections listées.

- **H-1-1** — L'étape 1 ne modifie ni le pilote de rejeu (`cmd/replay`), ni `REPLAY_SRCS`, ni la boucle de rejeu : elle ajoute une cible autour de `make state`. Le crochet `LABELS` se pose donc sur le pilote tel qu'il est aujourd'hui. À revérifier : Implementation Decisions (crochet du pilote), Implementation Chunks (2), Testing Decisions (crochet du pilote).
- **H-1-2** — Le répertoire de l'oracle ne contient que les 31 états normalisés et le fichier de couverture (« rien d'autre », PRD de l'étape 1), et son nom suit le plan. Les étiquettes vont donc dans un répertoire à elles ; si l'étape 1 a posé un parent commun au plan 12, elles vont à côté de l'oracle, sous ce parent. À revérifier : Implementation Decisions (répertoire des étiquettes), User Stories 21 et 27.
- **H-1-3** — Les 31 rejeux sont toujours `E2E_EVENTS` (26) et `FULL_GAME_EVENTS` (5) : l'étape 1 n'en ajoute ni n'en retire, et leurs noms ne se recouvrent pas. À revérifier : Solution (4), Implementation Decisions (cibles), Testing Decisions (« Les fichiers d'étiquettes »), User Story 22.
- **H-1-4** — L'étape 1 ne change rien au jeu : les étiquettes et l'oracle décrivent les mêmes trajectoires, prises sur le même code. À revérifier : Problem Statement, Implementation Decisions (aucun changement du jeu), User Story 12.
- **H-1-5** — Les 31 rejeux vont jusqu'à leur dernière frame sans `p8_fatal` sur le code actuel (la cible de l'oracle échoue sinon). La production des étiquettes en dépend. À revérifier : Implementation Chunks (2), Testing Decisions (« Les fichiers d'étiquettes »).
- **H-1-6** — La cible de l'oracle s'appelle `oracle` ou un nom voisin, sans collision avec `cursor-labels` ni `cursor-labels-test`, et sa légende suit la convention « temporary (plan 12), outside the suite » déjà posée par `line-coverage`. À revérifier : Implementation Decisions (cibles).

## Out of Scope

- Le passage à 160×120, la caméra de départ, l'ancrage de l'interface (étapes 3 et 4).
- La conversion des `events.json` et des `.txt`, et la boucle « reconvertir, réétiqueter, corriger l'ancre » (étape 5) : cette étape ne fait que fournir l'outil, la table de départ et les étiquettes du code actuel.
- La validation des ancres de la table : elles sont un brouillon, que seule l'étape 5 confirme.
- La comparaison à l'oracle (étape 6) ; la suppression des outils, de la table et des étiquettes (étape 7).
- Toute modification du hit test, des couches ou de la distribution des clics ; tout nouveau champ dans les entités.
- Une étiquette plus fine que nécessaire (par boîte de message, par ligne de menu) tant que l'étape 5 ne l'a pas réclamée.
- L'ajout de quoi que ce soit à `make test`, `make e2e` ou `make test-all`.

## Further Notes

- Le plan est encore un brouillon « à passer en `plan-converge` » ; cette PRD ne couvre que l'étape 2.
- Sans humain disponible, les points que `to-prd` fait valider ont été tranchés selon `guideline` (le plus simple qui tienne, hypothèses écrites) :
  - **Crochet dans le pilote existant plutôt qu'un nouveau binaire** : une seconde boucle de rejeu pourrait diverger de la vraie (oubli de `cheat_step`, ordre update/draw) et étiqueter une autre trajectoire ; une variable d'environnement de plus suit le modèle de `STATE`.
  - **L'entité qui reçoit le clic plutôt que `ui->hovered`** : `hovered` est la dernière entité trouvée, quelle que soit sa couche ; c'est l'ordre de `ui_send_to_hovered` qui décide où va le clic, donc quelle ancre vaut. Limite connue : si la première entité décline l'événement, il passe à la suivante ; l'étiquette reste la première.
  - **« Classe » au sens large : classe + qualificatif quand une classe couvre plusieurs ancres.** Seuls les boutons du menu de la galaxie sont distingués, par leur `on_click`, qui ne dépend pas de la position (une étiquette ne doit jamais changer parce que l'écran change). Si l'étape 5 montre qu'un autre groupe de boutons demande sa propre ancre, on affine l'étiquette et on régénère les fichiers en relançant l'outil sur le commit de cette étape (par exemple dans un `git worktree`) : c'est le seul cas où il faut revenir au code ancien.
  - **Ancres sur deux axes plutôt que les cinq mots du plan** : le bouton en haut à droite et le bouton en bas à gauche du menu de la galaxie ne s'écrivent pas avec un seul mot ; les cinq mots restent des raccourcis.
  - **Une étiquette nue par ligne** : la comparaison de l'étape 5 devient un `cmp`/`diff`, et la ligne qui diffère donne la frame.
  - **Un test, hors de la suite** : la règle de la tâche (« outils temporaires, jamais dans `make test` ni `make e2e` ») l'emporte sur la commodité ; même modèle que `line-coverage-test`.
  - **Deux chunks** : le module est le seul morceau à logique propre ; crochet, table et production n'ont de sens qu'ensemble.
- Un écart d'étiquette à l'étape 5 ne vient pas forcément d'une ancre fausse : un point de carte proche du bord bas passe, après +16 / −4, sous une barre ancrée en bas, qui remonte de 8. Si un bouton s'y trouve, l'étiquette change sans que la table soit en cause. L'étape 5 devra trancher ces cas ; les étiquettes de cette étape lui permettent au moins de les voir.
- Les fichiers des `full_game*` font environ 7 700 lignes chacun : versionnés le temps du plan seulement, supprimés à l'étape 7.
