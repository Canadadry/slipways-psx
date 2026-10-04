---
title: "Plan 12, étape 3 — Passer le framebuffer à 160×120"
description: "Le jeu dessine dans un écran de 128×128 qui ne tient pas ×2 dans les 320×240 de la PSX ; il faut passer à 160×120 (4:3) partout où la taille de l'écran est connue, sans encore ancrer l'interface."
status: needs-triage
---

## Problem Statement

Le framebuffer du portage fait 128×128 : 128 lignes ×2 = 256 ne tiennent pas dans les 240 lignes de la PSX, et le carré ne remplit pas un écran 4:3. Le plan 12 passe à 160×120, du 4:3 exact qui tient ×2 dans 320×240.

La taille de l'écran n'est pas écrite à un seul endroit. Elle vit dans les deux constantes du runtime PICO-8 émulé, mais aussi :

- en dur dans le backend raylib (`pixel_buffer` de 128 × 128 couleurs, conversion fenêtre → écran qui borne chaque axe à 127) ;
- dans les formats de `devtools/` : l'en-tête du dump texte (dérivé des constantes) et la capture (`screencast`), dont la taille de frame — 16 384 pixels, 8 192 octets — est épinglée par une assertion à la compilation, par `tools/frames_ref.sh` et par les tests ;
- dans le test de non-dessin de la carte étoilée, qui sonde une grille de 16×16 tuiles d'écran (la tuile (15,15) tombe hors d'un écran de 120 lignes) ;
- dans le test « est à l'écran » (`entity_geom_is_on_screen`, le champ `visible` de `make state`), qui suppose un cadre de 128 centré sur 64 ;
- dans la caméra de départ (192,192), choisie pour un cadre de 128 : à 160×120, le même coin montre un centre monde décalé.

Enfin, 29 tests unitaires sur environ 120 figent 128×128 : hashes de framebuffer complets, en-tête de dump, taille de frame de capture, caméra de départ, positions de curseur relatives à la caméra, un rejeu d'e2e lu par `test_state`. Changer les constantes seules casse `make test`, que le plan veut vert à chaque commit.

## Solution

Le jeu dessine dans un écran de 160×120 ; la fenêtre raylib fait 960×720 (échelle 6 inchangée) ; la carte montre davantage, autour du même centre monde qu'avant ; l'interface, elle, reste où elle est (calée sur 128×128) jusqu'à l'étape 4.

Concrètement :

1. **D'abord, rendre la taille dérivable, sans la changer.** Tout ce qui sait la taille de l'écran (backend raylib, conversion fenêtre → écran, test « est à l'écran », tests qui balaient l'écran) la lit dans `P8_SCREEN_WIDTH` / `P8_SCREEN_HEIGHT`. À 128×128, rien ne bouge : `make test` et `make e2e` restent verts et aucun `*.sha256` ne change, ce qui prouve que ce premier temps est neutre.
2. **Ensuite, changer la taille.** Les constantes passent à 160 et 120 ; la capture passe à 9 600 octets par frame (19 200 pixels), le dump à l'en-tête `160 120` ; la caméra de départ passe en (176,196) ; le cadre « est à l'écran » devient 160×120 avec la même marge de 12 pixels. Les tests qui figeaient 128×128 sont mis à jour. À partir de ce commit, `make e2e` est rouge par construction (règle du plan) jusqu'à l'étape 7 ; `make test` est vert.

## User Stories

1. En tant que joueur sur PC, je veux une fenêtre de 960×720, afin d'avoir une image 4:3 nette à l'échelle entière 6.
2. En tant que joueur sur PC, je veux que la carte montre 32 colonnes et perde 8 lignes autour du même centre, afin que l'écran plus large serve à voir plus de galaxie.
3. En tant que joueur sur PC, je veux qu'une planète entrée dans les 32 nouvelles colonnes soit dessinée, afin de ne pas voir un bord droit vide alors que la carte est là.
4. En tant que joueur sur PC, je veux que le curseur couvre tout l'écran de 160×120 et pas seulement son coin 128×128, afin de pouvoir viser toute la carte.
5. En tant que joueur sur PC, je veux qu'un curseur sorti de la fenêtre pendant un glisser reste borné au bord de l'écran sur chaque axe (159 en x, 119 en y), afin que le jeu ne reçoive jamais de coordonnée hors écran.
6. En tant que joueur, je veux qu'au début d'une partie la caméra soit centrée sur le même point du monde qu'avant, afin que la galaxie de départ apparaisse au même endroit, avec du champ en plus.
7. En tant que futur développeur PSX, je veux un framebuffer de 160×120, afin de l'afficher ×2 dans 320×240 sans rogner.
8. En tant que développeur du portage, je veux que la taille de l'écran ne soit écrite qu'une fois, dans les constantes du runtime, afin qu'un futur changement ne se fasse pas à huit endroits.
9. En tant que développeur du portage, je veux que le `pixel_buffer` du backend raylib soit dimensionné depuis ces constantes, afin qu'il ne déborde jamais du tableau qu'il remplit.
10. En tant que développeur du portage, je veux que la conversion fenêtre → écran borne x et y chacun à sa propre dimension, afin qu'un écran non carré soit couvert sans débordement.
11. En tant que développeur du portage, je veux que le dump texte commence par `160 120`, afin que sa largeur et sa hauteur se lisent sans le code.
12. En tant que développeur du portage, je veux qu'une frame de capture fasse 9 600 octets, afin que le format publié reste « deux pixels par octet, dans l'ordre de balayage » à la nouvelle taille.
13. En tant que développeur du portage, je veux que la taille de frame de la capture reste épinglée par une constante littérale du format, afin qu'un changement de taille d'écran casse le build au lieu de changer le format en silence.
14. En tant que développeur du portage, je veux que l'outil de capture de référence (`e2e-ref`) lise des frames de 9 600 octets, afin qu'il ne coupe pas les captures au mauvais endroit.
15. En tant que développeur du portage, je veux que `visible` dans `make state` suive le cadre 160×120 avec la même marge de 12 pixels qu'avant, afin que les règles de culling (planètes, slipways, particules) suivent l'écran réel, sans imiter l'ancien.
16. En tant que développeur du portage, je veux un premier commit qui rend tout dérivable à taille inchangée, afin de prouver par `make e2e` vert que la refonte est neutre avant de changer la taille.
17. En tant que développeur du portage, je veux que `make test` passe à chaque commit de cette étape, afin que la boucle rapide reste un garde-fou.
18. En tant que développeur du portage, je veux que les tests qui balaient l'écran le fassent depuis les constantes, afin qu'ils suivent la taille sans être réécrits.
19. En tant que développeur du portage, je veux que les hashes de framebuffer figés soient refigés sur la nouvelle taille et uniquement eux, afin qu'un hash qui change pour une autre raison se voie.
20. En tant que développeur du portage, je veux que les tests qui posent le curseur relativement à la caméra visent le même point du monde qu'avant (décalage +16 / −4), afin qu'ils gardent leur prémisse (un emplacement libre, une planète précise).
21. En tant que développeur du portage, je veux que les tests de la caméra de départ attendent (176,196), afin qu'ils figent la nouvelle règle.
22. En tant que développeur du portage, je veux qu'un test qui dessine un élément d'interface calé sous la ligne 120 (icône de structure au repos en (118,120)) le dessine à une position visible, afin de tester le dessin et non l'ancrage, qui est l'étape 4.
23. En tant que développeur du portage, je veux que `test_state` continue de vérifier le format de l'état d'une partie rejouée, afin de ne pas perdre la seule couverture de `make state` dans `make test`.
24. En tant que développeur de l'étape 5, je veux savoir quelles assertions de `test_state` ont été refigées sur un rejeu non converti, afin de les rétablir quand `trade_rich_mine.json` sera converti.
25. En tant que développeur des étapes 5 et 6, je veux que l'oracle figé et les fichiers d'étiquettes versionnés ne soient pas réécrits par cette étape, afin qu'ils décrivent toujours le code 128×128.
26. En tant que développeur du portage, je veux que `make raylib` compile et linke encore si le vendoring est présent, afin que le binaire interactif reste prêt pour l'humain.
27. En tant que relecteur, je veux que le diff de cette étape ne touche ni à l'interface (barres, boutons, infobulles, panneaux, écran titre) ni aux 128 qui sont une échelle du monde, afin que l'étape 4 reste seule à décider de l'ancrage.
28. En tant que relecteur, je veux que les commentaires qui énoncent un contrat de taille dans les fichiers touchés (capture, dump, backend, conversion) disent 160×120, afin qu'aucun contrat ne mente après le commit.
29. En tant que relecteur, je veux qu'aucune entrée ne soit ajoutée à `docs/trous-de-portage.md`, afin de respecter la règle du plan : un changement d'affichage n'est pas un trou de portage.
30. En tant que développeur du portage, je veux que `make test` et `make e2e` ne s'allongent pas, afin de respecter la règle du projet.

## Implementation Decisions

- **Constantes du runtime.** `P8_SCREEN_WIDTH` 160, `P8_SCREEN_HEIGHT` 120. Les dimensions de la feuille de sprites et de la carte (128×128, 32×32) ne bougent pas : ce ne sont pas des tailles d'écran. Le runtime (`p8_pset`, `p8_cls`, `vram`) lit déjà ces constantes ; rien d'autre à y changer.
- **Backend raylib.** Le `pixel_buffer` est dimensionné `P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT`. `RAYLIB_BACKEND_SCALE` reste 6 : la fenêtre (déjà calculée depuis les constantes) fait 960×720. Le texte de statut de rejeu dessiné par-dessus garde sa place.
- **Conversion fenêtre → écran.** Même signature (fenêtre, échelle → coordonnée d'écran). Chaque axe est borné à sa dimension : x dans [0, `P8_SCREEN_WIDTH` − 1], y dans [0, `P8_SCREEN_HEIGHT` − 1]. Le module reste sans raylib et inclut seulement l'en-tête du runtime qui porte les constantes.
- **Dump texte.** Format inchangé, déjà dérivé des constantes : en-tête `160 120`, 120 lignes de 160 caractères.
- **Capture (`screencast`).** Format inchangé dans sa forme (frames brutes concaténées, deux pixels par octet, pixel pair dans le quartet bas), taille de frame 9 600 octets (19 200 pixels). L'assertion à la compilation épingle désormais 9 600 : elle reste un littéral, pour qu'un futur changement de taille casse le build. La largeur paire (exigée par le compactage) est un invariant à affirmer à la compilation. Le script de capture de référence lit des frames de 9 600 octets. Une capture de référence d'avant ce commit n'est plus comparable : c'est attendu, les `*.sha256` sont refaits à l'étape 7.
- **« Est à l'écran ».** `entity_geom_is_on_screen` compare la position relative à la caméra au centre de l'écran (`P8_SCREEN_WIDTH`/2, `P8_SCREEN_HEIGHT`/2), avec une demi-étendue de la demi-dimension + 12 sur chaque axe (le 64 et le 76 d'aujourd'hui, à 128). À 160×120 : x relatif dans [−12, 172], y dans [−12, 132]. C'est ce que lisent le culling des planètes, des slipways et des particules, et le champ `visible` de `make state`.
- **Caméra de départ.** L'archétype de la caméra passe de (192,192) à (176,196), pour sa position et sa position avant secousse : même centre monde (256,256), puisque le centre de l'écran passe de (64,64) à (80,60). Les bornes de la caméra (−64 … 320) sont une échelle du monde et ne bougent pas.
- **Carte étoilée.** Le dessin du fond n'a pas de grille d'écran : il dessine la carte de 32×32 tuiles en coordonnées du monde, sous la caméra. La « grille de 16×16 tuiles » du plan est celle que le test de non-dessin sonde (tuiles d'écran de 8 pixels) ; elle devient 20×15, dérivée des constantes, et aucune tuile sondée ne sort de l'écran.
- **Ce qui ne bouge pas.** L'interface (barres du haut et du bas, boutons calés à droite, infobulles à 127, panneaux et boîtes centrés sur 64, écran de score, écran titre), les tables `game_data`/`map_tables` hors archétype de la caméra, les 128 d'échelle du monde (`galaxy`, `probe`, `slipway`, `planet_display`, `wormhole`). La barre du bas, calée en 117–127, est coupée à la ligne 119 jusqu'à l'étape 4 : c'est attendu.
- **Commentaires.** Les commentaires qui énoncent la taille dans les fichiers touchés (contrat de la capture, du dump, du backend et de la conversion, type d'entrée souris) disent 160×120 ou renvoient aux constantes. Le `README` et les autres mentions de 128×128 restent pour l'étape 7. Aucune entrée dans `docs/trous-de-portage.md`.
- **Ni l'oracle ni les étiquettes ne sont régénérés.** Après le changement de taille, `make oracle` et `make cursor-labels` sans `EVENTS` réécriraient les fichiers versionnés des étapes 1 et 2 avec le nouveau code : on ne les lance pas. `make cursor-labels EVENTS=…` (sortie dans `tmp/`) reste permis.

## Implementation Chunks

1. **Tout dériver de la taille, à 128×128** — un cycle TDD, neutre. Tests d'abord, écrits depuis les constantes : la conversion fenêtre → écran borne chaque axe à sa dimension − 1 (fenêtre `dimension × 6 − 1` → `dimension − 1`, au-delà → `dimension − 1`, négatif → 0), « est à l'écran » vrai et faux juste de part et d'autre de chaque bord du cadre (centre ± demi-dimension + 12), la grille sondée du fond étoilé dérivée des constantes. Rouge sur la conversion (qui borne en dur) ; vert après avoir dimensionné le `pixel_buffer`, la conversion et « est à l'écran » depuis les constantes, et rendu les balayages de tests dérivés. Terminé quand `make test` passe, que `make e2e` passe sans qu'aucun `*.sha256` ne change, et que `make raylib` compile si le vendoring est présent.
2. **Passer à 160×120 et (176,196)** — dépend du chunk 1. Tests d'abord, sur les épinglages du format et de la règle : en-tête de dump `160 120` ; capture de 9 600 octets par frame (taille du fichier, nibble order aux quatre coins (0,0), (159,0), (0,119), (159,119)) ; caméra de départ (176,196) ; le point du monde (256,256) dessiné en (80,60). Rouges. Puis les constantes, l'assertion du format, le script de référence et l'archétype de la caméra : verts. Puis la mise à jour des tests qui figeaient 128×128 (voir Testing Decisions). Terminé quand `make test` passe et que `make raylib` compile si le vendoring est présent ; `make e2e` est rouge, comme le plan le prévoit.

## Testing Decisions

- **Un bon test** vérifie une sortie observable (pixels, octets du fichier, coordonnées rendues, état écrit), jamais la façon de la calculer. Un épinglage de format (en-tête `160 120`, 9 600 octets) reste un littéral, pas une expression des constantes : c'est son rôle de casser si la taille change. Un balayage de l'écran, lui, se dérive des constantes.
- **Nouveaux cas ou cas réécrits :**
  - conversion fenêtre → écran : origine, pixel connu, dernier pixel de la fenêtre (959,719) → (159,119), au-delà → (159,119), négatif → (0,0), autre échelle ;
  - « est à l'écran » : entité juste dedans et juste dehors sur chacun des quatre bords, à 160×120 (x relatif 172 / 173, −12 / −13 ; y relatif 132 / 133, −12 / −13), dont un cas dans les colonnes 141–159 qui était hors cadre à 128×128 ;
  - dump : en-tête `160 120`, coins (159,0) et (0,119) ;
  - capture : 5 frames font 5 × 9 600 octets ; aller-retour pixel pour pixel aux quatre coins ; fichier tronqué 2 × 9 600 + 1 refusé ;
  - caméra : archétype et caméra fraîche en (176,196) ; `draw_layers` applique (176,196) ; le point du monde (256,256) sort en (80,60), le centre de l'écran.
- **Tests à mettre à jour dans le chunk 2** (les 29 rouges mesurés sur une copie de HEAD avec le seul changement de taille et de caméra) :
  - hashes de framebuffer complets à refiger, sans autre changement : `ascension`, `background`, `burst`, `button`, `help_screen`, `infraspace_gate`, `layout_row`, `message_box`, `modal_backdrop`, `probe`, `protostar`, `research_project`, `score_panel`, `slipgate`, `slipway`, `structure`, `synthesizer`, `toast`, `tooltip_draw_at`, `ui`, `wormhole`. Un hash ne se refige qu'après avoir vérifié que l'assertion qui échoue est bien celle d'un hash d'écran ; toute autre assertion rouge est une prémisse à réparer, pas un hash.
  - prémisses relatives à la caméra : les tests de placement de structures (`ascension`, `synthesizer`, `slipgate`, `infraspace_gate`, et tout autre qui lit `main_camera + (x,y)`) visent `main_camera + (x + 16, y − 4)`, donc le même point du monde qu'avant ; leur état final (`SETTLED`) doit revenir sans autre changement.
  - caméra de départ : `camera`, `main_camera`, `draw_layers_camera`.
  - formats : `dump`, `screencast`, `raylib_backend_coords`.
  - `background_draw` : grille sondée 20×15.
  - `structure_icon` : l'icône au repos en (118,120) est désormais hors de l'écran ; le test de dessin la place à une position visible avant de comparer au blanc (l'ancrage de sa position de repos est l'étape 4).
  - `test_state` : le cas `cheat_code` et le cas `p8_fatal` passent tels quels. Le cas `trade_rich_mine` rejoue un e2e non converti, dont la trajectoire diverge (mesuré : 90 planètes, 0 colonie, 0 slipway). Il garde ses contrôles indépendants de la trajectoire (lignes `zone`/`cap`, pas de ligne `stopped`, lignes `techs`) et ajoute la ligne `camera world 176 196` ; ses assertions de trajectoire (comptes, `p1` colonisée, slipways) sont retirées, avec un `TODO` qui demande de les rétablir depuis l'oracle de `trade_rich_mine` une fois le rejeu converti (étape 5). Le format de la ligne `slipway` n'est plus couvert par `make test` entre-temps : aucun e2e non converti n'en produit.
- **Non-régression** — chunk 1 : `make test` et `make e2e` verts, aucun `*.sha256` modifié. Chunk 2 : `make test` vert ; la liste des tests de `make test` est la même qu'avant l'étape ; aucun fichier de l'oracle ni des étiquettes n'a changé (`git status` propre sur leurs répertoires).
- **Art antérieur** — `tests/test_raylib_backend_coords.c` (module sans raylib), `tests/test_screencast.c` (épinglage du format sur les octets bruts), `tests/test_main_camera.c` (« est à l'écran » sous une caméra secouée), `tests/test_draw_layers_camera.c`, `tests/test_state.c`.

## Hypothèses sur l'issue de l'étape 2

L'étape 2 est en cours : seule sa PRD est commitée. Ce qui suit suppose une issue conforme à cette PRD. Si une hypothèse tombe, revérifier les sections listées.

- **H-2-1** — L'étape 2 ne touche ni `src/game/`, ni `src/engine/`, ni `src/p8/`, ni `src/platform/`, ni `src/devtools/dump`, ni `src/devtools/screencast`, ni `tools/frames_ref.sh`. Cette étape part donc, pour tous ces fichiers, de leur état actuel. À revérifier : Implementation Decisions (toutes les puces sur le code), Implementation Chunks (1 et 2).
- **H-2-2** — L'étape 2 n'ajoute aucun test à `make test` (son test vit sous `cursor-labels-test`, hors de la suite) : la liste des 29 tests à mettre à jour reste complète. Si un test de l'étape 2 est entré dans `make test`, il faut le relancer après le chunk 2 et l'ajouter à la liste. À revérifier : Testing Decisions (tests à mettre à jour, non-régression).
- **H-2-3** — Le module d'étiquette et le crochet `LABELS=` du pilote ne contiennent aucune coordonnée d'écran ni aucune taille : ils compilent et fonctionnent tels quels à 160×120, sans modification par cette étape. Le cas d'intégration de `cursor-labels-test` (première ligne `help_screen` sur la fixture de l'écran titre) n'est pas une condition de fin de cette étape. À revérifier : Implementation Decisions (ni l'oracle ni les étiquettes), Out of Scope.
- **H-2-4** — `make cursor-labels` sans `EVENTS` écrit dans un répertoire versionné, et `make cursor-labels EVENTS=…` dans `tmp/` ; de même, `make oracle` (étape 1) réécrit `tests/plan12-oracle/`. D'où l'interdiction de lancer les deux premières formes après le chunk 2. À revérifier : Implementation Decisions (ni l'oracle ni les étiquettes), Testing Decisions (non-régression), User Story 25.
- **H-2-5** — La table des ancres de l'étape 2 fixe la carte à +16 / −4, ce qui est exactement le déplacement que produit la caméra (176,196) : un point du monde vu en (x,y) à 128×128 est vu en (x + 16, y − 4) à 160×120. Les prémisses de tests relatives à la caméra (Testing Decisions) suivent la même règle. Si l'étape 2 a changé ce décalage, la caméra du plan prime et la table est à corriger à l'étape 5, pas ici. À revérifier : Implementation Decisions (caméra de départ), Testing Decisions (prémisses relatives à la caméra).
- **H-2-6** — Les cibles de l'étape 2 et leurs légendes dans le `Makefile` ne recouvrent pas les lignes du `Makefile` que cette étape touche (aucune : seul `tools/frames_ref.sh` change côté outils). À revérifier : Implementation Chunks (2).

## Out of Scope

- L'ancrage de l'interface (étape 4) : barres, boutons calés à droite, infobulles, panneaux et boîtes centrés, écran de score, écran titre, position de repos des icônes de structure. Après cette étape, l'interface est calée sur le coin 128×128 et la barre du bas est coupée : c'est attendu.
- La conversion des `events.json` et des `.txt` (étape 5), la comparaison à l'oracle (étape 6), le rebaseline des `*.sha256`, le `README` et le reste des commentaires « 128×128 » (étape 7).
- Rendre `make e2e`, `make state` (tous rejeux), `make full-game`, `make sanitize-net` ou `cursor-labels-test` verts : hors des conditions de fin.
- Les bornes de la caméra (−64 … 320) et les 128 d'échelle du monde.
- La feuille de sprites et la carte, qui restent 128×128 et 32×32.
- PSX et audio.
- Toute entrée dans `docs/trous-de-portage.md`.

## Further Notes

- Mesures faites sur une copie de HEAD hors du dépôt (`git archive`), constantes à 160×120 : 24 tests de `make test` rougissent avec la seule taille, 29 avec la caméra (176,196) ; changer « est à l'écran » n'en ajoute aucun. Les 26 e2e rejouent jusqu'au bout sans `p8_fatal` sur le code modifié (sans conversion), mais leurs trajectoires divergent.
- Sans humain disponible, les points que `to-prd` fait valider ont été tranchés selon `guideline` (le plus simple qui tienne, hypothèses écrites) :
  - **« Est à l'écran » dans cette étape**, bien que la liste de l'étape 3 du plan ne le nomme pas : sans lui, une planète des colonnes 141–159 n'est pas dessinée (le culling garde le cadre 128), ce qui est un bug visible ; la règle du plan (« `visible` suit le cadre 160×120 ») et le fait que ce n'est pas de l'interface le rangent ici plutôt qu'à l'étape 4. La marge de 12 est gardée telle quelle.
  - **Deux chunks, le premier neutre** : il sépare la refonte (vérifiable par `make e2e` vert) du changement de taille (où `make e2e` ne dit plus rien). Sans ce découpage, une erreur de dérivation se cacherait dans le rouge attendu des e2e.
  - **La « grille 16×16 » est celle du test** : le dessin du fond n'a pas de grille d'écran ; inventer une constante de tuiles dans le code de production serait du code pour rien.
  - **Épinglages littéraux** (en-tête du dump, 9 600 octets) plutôt que dérivés : c'est leur raison d'être.
  - **Prémisses de tests décalées de (+16, −4) plutôt que refigées** : on garde la même situation dans le monde, donc le même sens du test ; seuls les hashes d'écran se refigent.
  - **`test_state` affaibli avec un `TODO`** plutôt que refigé sur une trajectoire non convertie : refiger des comptes produits par un rejeu qui vise de travers figerait du bruit, que l'étape 5 devrait de toute façon réécrire. La perte (format de la ligne `slipway`) est temporaire et nommée.
  - **Commentaires de contrat corrigés tout de suite**, `README` à l'étape 7 : un contrat faux est un piège ; le `README` est une description que le plan range à la fin.
  - **Icône de structure** : on corrige la prémisse du test (la placer à l'écran), pas sa position de repos, qui relève de l'ancrage.
- Les zones et caps peuvent monter avec un cadre plus large (plus de planètes, slipways et particules dessinées). `make test` n'en dit rien ; c'est l'étape 6 qui vérifie qu'ils restent sous leur limite.
- `make e2e-ref` n'a plus de sens entre ce commit et l'étape 7 : ses captures de référence sont à 8 192 octets par frame.
