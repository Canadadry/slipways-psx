---
title: "Plan 12, étape 4 — Ancrer l'interface"
description: "À 160×120, l'interface reste dessinée dans le coin 128×128 (barre du bas coupée, boutons de droite au milieu, panneaux décentrés) ; chaque élément d'écran doit s'ancrer à son bord le plus proche, écrit depuis P8_SCREEN_WIDTH / P8_SCREEN_HEIGHT."
status: needs-triage
---

## Problem Statement

À la fin de l'étape 3, le framebuffer fait 160×120, mais l'interface est toujours calée sur l'ancien écran de 128×128 :

- la barre du bas (y 117–127) est coupée à la ligne 119 : on ne voit plus ni les crédits, ni le revenu, ni le bouton pause ;
- les éléments calés à droite (bouton « année suivante », cadre du mois et de l'année, icônes de structures) flottent à 32 pixels du bord droit ;
- les infobulles des boutons partent de la ligne 127, donc à moitié hors de l'écran, et leur texte est centré sur x = 64 au lieu du milieu ;
- les barres pleine largeur (`hud_draw_bar`) s'arrêtent à x = 127 ;
- la bannière de l'année, la boîte de difficulté, l'écran de score final et l'écran titre sont centrés sur (64,64) au lieu de (80,60) ;
- la boîte « panneau » (`panel_box`), qui dit si un glisser lâché au bord de l'écran annule la pose, garde ses bords droit et bas à 118.

La règle du plan 12 (§2) est connue — **l'interface s'ancre à son bord le plus proche** : un élément calé à gauche ou en haut ne bouge pas ; un élément calé à droite prend +32 en x ; un élément calé en bas prend −8 en y ; un élément centré passe du centre (64,64) au centre (80,60). Toute position d'écran s'écrit depuis `P8_SCREEN_WIDTH` / `P8_SCREEN_HEIGHT`, sans 127, 128 ou 64 en dur. Elle n'est appliquée nulle part. Les positions vivent pour moitié dans les tables (`game_data.c` : menu de la galaxie, menu pause, lignes du score, cadre du score, relevés, bannière, écran titre, archétypes de l'interface, des icônes de structure et de l'écran titre) et pour moitié dans le code (`hud_draw_bar`, `tooltip_draw_at`, les boutons de fin de partie, le rideau de fin de partie).

## Solution

Chaque élément d'interface dessiné dans l'espace écran (couche ≥ 10, ou position lue dans `mouse_screen`) reçoit une ancre, et sa position s'écrit depuis les constantes de l'écran :

- **gauche / haut** : la valeur actuelle, inchangée ;
- **droite** : `P8_SCREEN_WIDTH − d`, où d est la distance actuelle au bord 128 ;
- **bas** : `P8_SCREEN_HEIGHT − d`, où d est la distance actuelle au bord 128 ;
- **centre** : `P8_SCREEN_WIDTH / 2 + dx` et `P8_SCREEN_HEIGHT / 2 + dy`, où (dx,dy) est l'écart actuel à (64,64).

Ce qui est dans le monde (planètes, slipways, panneaux de planète, toasts, infobulle de pose qui suit la souris) ne bouge pas : la caméra de l'étape 3 s'en occupe. Après l'étape, la barre du bas est entière, la droite de l'écran est occupée, et les boîtes centrées sont au milieu de l'image 160×120. `make test` passe à chaque commit ; `make e2e` reste rouge, comme le plan le prévoit jusqu'à l'étape 7.

### Table des ancres

Positions en pixels d'écran ; « W » et « H » sont `P8_SCREEN_WIDTH` et `P8_SCREEN_HEIGHT`. La dernière colonne donne la valeur à 160×120.

| Élément | Où | Aujourd'hui | Ancre | Expression | À 160×120 |
|---|---|---|---|---|---|
| Barre pleine largeur (`hud_draw_bar`) | code | x 0–127 | gauche–droite | x 0 … W − 1 | x 0–159 |
| Filet du haut (cadre du score, rangée 2) | table | y 0–1 | haut | inchangé | y 0–1 |
| Filet du bas (cadre du score, rangée 1) | table | y 126–127 | bas | H − 2 … H − 1 | y 118–119 |
| Cadre haut-gauche (rangée 4) | table | (0, −3, 26) | gauche haut | inchangé | (0, −3, 26) |
| Cadre haut-droit (rangée 5) | table | (81, −3, 117) | droite haut | (W − 47, −3, W − 11) | (113, −3, 149) |
| Cadre bas-gauche (rangée 3) | table | (8, 118, 34) | gauche bas | (8, H − 10, 34) | (8, 110, 34) |
| Relevés crédits, revenu | table | (13,121), (37,121) | gauche bas | y H − 7 | (13,113), (37,113) |
| Relevés recherche, production | table | (13,2), (29,2) | gauche haut | inchangé | (13,2), (29,2) |
| Relevé bonheur | table | (59,2) | centre haut | x W/2 − 5 | (75,2) |
| Relevés mois, année | table | (85,2), (99,2) | droite haut | x W − 43, W − 29 | (117,2), (131,2) |
| Menu galaxie 1 (année suivante) | table | (118,0) | droite haut | (W − 10, 0) | (150,0) |
| Menu galaxie 2 (technologies) | table | (0,0) | gauche haut | inchangé | (0,0) |
| Menu galaxie 3 (score) | table | (47,0) | centre haut | (W/2 − 17, 0) | (63,0) |
| Menu galaxie 4 (pause) | table | (0,117) | gauche bas | (0, H − 11) | (0,109) |
| Menu pause (`pause_menu_tail`, monte de 11) | table | (0,107) | gauche bas | (0, H − 21) | (0,99) |
| Icônes de structure (`rest_cursor`, vont vers la gauche) | archétype | (118,120) | droite bas | (W − 10, H − 8) | (150,112) |
| Boîte panneau (`panel_box`) | archétype | (10,10)–(118,118) | chaque bord au sien | (10,10)–(W − 10, H − 10) | (10,10)–(150,110) |
| Infobulle d'un bouton | code | `tooltip_draw_at(…, 127, 1)` | bas | `(…, H − 1, 1)` | bas en 119 |
| Infobulle d'une icône de structure | code | `tooltip_draw_at(…, 0, 0)` | haut | inchangé | haut en 0 |
| Texte d'une infobulle | code | x 64 | centre | x W/2 | x 80 |
| Bannière de l'année : filet | table | y 42–43 | centre | H/2 − 22 … H/2 − 21 | y 38–39 |
| Bannière : « year N » | table | (64,40) | centre | (W/2, H/2 − 24) | (80,36) |
| Bannière : compte à rebours | table | (64,48) | centre | (W/2, H/2 − 16) | (80,44) |
| Écran des technologies (`layout_block`) | code | (0,11), pas de 11 | gauche haut | inchangé | (0,11) |
| Boîte de difficulté (écran titre) | archétype | (34,56), pas de 11 | centre | (W/2 − 30, H/2 − 8) | (50,52) |
| Score final : 5 fonds (`score_rows` 1–5) | table | y 26–86, 51–59, 76–77, 24–25, 33–41 | centre | H/2 + (y − 64) | y 22–82, 47–55, 72–73, 20–21, 29–37 |
| Score final : 7 lignes (`score_rows` 6–12) | table | x 64, y 21 … 82 | centre | (W/2, H/2 + (y − 64)) | x 80, y 17 … 78 |
| Score final : bouton « again? » | code | (51,100) | centre | (W/2 − 13, H/2 + 36) | (67,96) |
| Score final : bouton retour | code | (39,100) | centre | (W/2 − 25, H/2 + 36) | (55,96) |
| Rideau de fin : bande du haut | code | y 0 … h | haut | inchangé | y 0 … h |
| Rideau de fin : bande du bas | code | y 127 − h … 127 | bas | H − 1 − h … H − 1 | y 119 − h … 119 |
| Écran titre : origine (`help_screen` pos) | archétype | (64,28) | centre | (W/2, H/2 − 36) | (80,24) |
| Écran titre : logo (spr 192, 16×4 tuiles) | table | (0,11) | centre | (W/2 − 64, H/2 − 53) | (16,7) |
| Écran titre : icône du lien (spr 172) | table | (20,107) | centre | (W/2 − 44, H/2 + 43) | (36,103) |
| Écran titre : « need help? » | table | (40,110) | centre | (W/2 − 24, H/2 + 46) | (56,106) |
| Écran titre : « slipways.net/help » | table | (40,116) | centre | (W/2 − 24, H/2 + 52) | (56,112) |

Le « − 64 » du logo est sa demi-largeur (128 pixels de sprite), pas une taille d'écran. Les boîtes de collision relatives à une position (`hit_box` des boutons, de l'écran titre, des icônes) suivent leur entité et ne changent pas.

## User Stories

1. En tant que joueur, je veux voir la barre du bas en entier, afin de lire mes crédits et mon revenu.
2. En tant que joueur, je veux que le bouton pause soit dans le coin bas-gauche de l'écran 160×120, afin de pouvoir ouvrir le menu.
3. En tant que joueur, je veux que le menu pause s'ouvre au-dessus du bouton pause, comme avant, afin que ses quatre lignes restent à l'écran et au même endroit par rapport au bouton.
4. En tant que joueur, je veux que le bouton « année suivante » soit dans le coin haut-droit, afin de le trouver au bord, là où il était.
5. En tant que joueur, je veux que le cadre du mois et de l'année touche le bouton « année suivante », afin que le haut de l'écran garde son dessin.
6. En tant que joueur, je veux que le bouton du score et le relevé du bonheur restent au milieu de la barre du haut, afin que la barre reste équilibrée.
7. En tant que joueur, je veux que les filets du haut et du bas traversent tout l'écran, afin de ne pas voir une barre coupée à 128.
8. En tant que joueur, je veux que les icônes de structures s'alignent depuis le coin bas-droit, afin de les trouver au bord, à leur place.
9. En tant que joueur, je veux que l'infobulle d'un bouton se pose sur le bas de l'écran, en entier, afin de la lire.
10. En tant que joueur, je veux que le texte d'une infobulle soit centré sur l'écran, afin qu'il ne penche pas à gauche.
11. En tant que joueur, je veux que l'infobulle d'une icône de structure reste en haut de l'écran, afin qu'elle ne recouvre pas l'icône.
12. En tant que joueur, je veux que lâcher une structure près d'un bord de l'écran l'annule sur les quatre bords, y compris les 32 nouvelles colonnes de droite, afin que le geste « annuler » marche partout.
13. En tant que joueur, je veux que la bannière « year N » et son compte à rebours soient au milieu de l'écran, afin de les voir comme avant.
14. En tant que joueur, je veux que l'écran de score (pendant la partie ou en fin de partie) soit au milieu de l'écran, afin de le lire sans qu'il penche.
15. En tant que joueur, je veux que les boutons « again? » et retour de la fin de partie restent sous le tableau des scores, au milieu, afin de relancer une partie.
16. En tant que joueur, je veux que le rideau de fin de partie recouvre la barre du bas réelle, afin qu'aucune barre ne reste visible derrière le score.
17. En tant que joueur, je veux que l'écran titre soit centré, logo compris, afin qu'il occupe le cadre 4:3.
18. En tant que joueur, je veux que la boîte de difficulté de l'écran titre soit au milieu, afin de choisir ma difficulté sans chercher.
19. En tant que joueur, je veux que le lien « slipways.net/help » de l'écran titre réagisse au survol à sa nouvelle place, afin que le curseur d'aide y apparaisse.
20. En tant que joueur, je veux que l'écran des technologies reste calé en haut à gauche, sous le bouton qui l'ouvre, afin de ne pas perdre ce lien visuel.
21. En tant que joueur, je veux que rien de l'interface ne change pour ce qui est dans le monde (panneaux de planète, toasts, coût au curseur), afin que ces éléments restent collés à ce qu'ils décrivent.
22. En tant que développeur du portage, je veux que toute position d'écran de l'interface s'écrive depuis `P8_SCREEN_WIDTH` / `P8_SCREEN_HEIGHT`, afin qu'un futur changement de taille (PSX) ne se fasse pas élément par élément.
23. En tant que développeur du portage, je veux que l'ancre d'un élément se lise dans son expression (W − d, H − d, W/2 + dx), afin de savoir d'un coup d'œil s'il est calé à droite, en bas ou au centre.
24. En tant que développeur du portage, je veux une macro de constante fixe depuis un entier, utilisable dans un initialiseur statique, afin d'écrire les archétypes (`panel_box`, `rest_cursor`, positions de l'écran titre) depuis les constantes de l'écran au lieu d'un hexadécimal figé.
25. En tant que développeur du portage, je veux que `hud_draw_bar` aille jusqu'au bord droit réel, afin que toutes les barres (filets, bannière, fonds du score, infobulles, rideau) suivent sans être retouchées une à une.
26. En tant que développeur du portage, je veux un test par groupe d'éléments qui vérifie la position à 160×120 par ce qui est dessiné ou par la valeur lue (pas par l'expression), afin qu'une ancre fausse casse `make test`.
27. En tant que développeur du portage, je veux que les hashes d'écran de l'interface soient refigés seulement après avoir vérifié que l'assertion rouge est bien un hash d'écran, afin qu'un changement non voulu ne se cache pas dans un refigeage.
28. En tant que développeur du portage, je veux que les prémisses de tests qui visent un élément d'interface (souris sur le lien de l'écran titre, souris dans la boîte panneau) suivent l'ancre de cet élément, afin que le test garde son sens.
29. En tant que développeur de l'étape 5, je veux la liste des éléments dont l'ancre diffère de `tools/cursor_anchors.txt`, afin de corriger la table sans la redécouvrir frame par frame.
30. En tant que développeur de l'étape 5, je veux savoir que l'étiquette `button` couvre des boutons d'ancres différentes, afin de prévoir une étiquette plus fine avant la conversion.
31. En tant que relecteur, je veux que le diff ne touche ni aux 128 d'échelle du monde, ni aux bornes de la caméra, ni à la caméra de départ, afin que l'étape reste celle de l'interface.
32. En tant que relecteur, je veux qu'aucune entrée ne soit ajoutée à `docs/trous-de-portage.md`, afin de respecter la règle du plan : un changement d'affichage n'est pas un trou de portage.
33. En tant que relecteur, je veux que l'oracle figé et les fichiers d'étiquettes ne soient pas réécrits, afin qu'ils décrivent toujours le code 128×128.
34. En tant que développeur du portage, je veux que `make test` passe à chaque commit de l'étape et ne s'allonge pas, afin que la boucle rapide reste un garde-fou.
35. En tant qu'humain qui relira les captures à la fin du plan, je veux qu'une vérification par dump texte (`make replay`) ait été faite sur l'écran titre et sur une frame de partie, afin que les grosses erreurs d'ancrage soient prises avant l'étape 5.

## Implementation Decisions

- **Ce qui est « interface ».** Un élément s'ancre s'il est dessiné dans l'espace écran : couche ≥ 10 (la caméra n'y est pas appliquée, `layer_fades` vaut 0) ou position comparée à `mouse_screen`. Les panneaux de planète (couche 9, `back_color` 9, positionnés sur la planète), les toasts (couche 9, position monde) et l'infobulle de pose (`targeting_tooltip_show`, relative à la souris) sont dans le monde ou suivent la souris : ils ne bougent pas.
- **Règle d'écriture.** Une position calée à droite s'écrit `P8_SCREEN_WIDTH − d`, calée en bas `P8_SCREEN_HEIGHT − d`, centrée `P8_SCREEN_WIDTH / 2 + dx` / `P8_SCREEN_HEIGHT / 2 + dy` ; une position calée à gauche ou en haut garde son littéral. Pas d'aide d'ancrage (fonction ou table d'ancres) : l'expression suffit à dire l'ancre, et une couche de plus servirait un seul usage. Les valeurs sont celles de la table des ancres ci-dessus.
- **Tables.** Les champs entiers des tables (`galaxy_menu_table`, `pause_menu_tail`, `score_rows_table`, les rangées `.number` de `score_panel_frame_rows`, `score_panel_readouts`, `score_panel_year_banner`, `score_panel_countdown_rows`, `help_screen_rows`) prennent directement les expressions : ce sont des expressions constantes, valides dans un initialiseur statique.
- **Archétypes en virgule fixe.** `ui_archetype.panel_box`, `structure_icon_archetype.rest_cursor`, `help_screen_archetype.pos` et `.fixed_pos` sont des `p8_fixed_t` écrits en hexadécimal brut. Le runtime n'a pas de conversion entier → fixe utilisable à la compilation (`p8_fixed_from_int` est une fonction). On ajoute au runtime une macro de constante, `P8_FIXED_INT(i)`, qui rend la même valeur que `p8_fixed_from_int(i)` pour tout entier de −32768 à 32767 ; un test de `test_fixed` le vérifie sur quelques valeurs dont des négatives. Le sentinelle hors écran des icônes (`pos` 999,999) reste tel quel : il est hors de tout écran.
- **`hud_draw_bar`.** Va de x 0 à `P8_SCREEN_WIDTH − 1`. C'est le seul changement de code pour toutes les barres pleine largeur : filets, bannière, fonds du score, infobulles, rideau.
- **`tooltip_draw_at`.** Garde sa signature (haut ou bas `om`, alignement `a`). Le texte est centré sur `P8_SCREEN_WIDTH / 2`. Le bouton l'appelle avec `om = P8_SCREEN_HEIGHT − 1`, `a = 1` ; l'icône de structure garde `(0, 0)`. Le `draw_at` de la classe (qui passe 0) ne change pas.
- **Fin de partie.** Les positions des boutons « again? » et retour s'écrivent depuis le centre. La bande du bas du rideau va de `P8_SCREEN_HEIGHT − 1 − h` à `P8_SCREEN_HEIGHT − 1`, et garde sa couleur héritée (le commentaire-piège en place reste vrai). La bande du haut ne change pas.
- **Écran titre.** Tout l'écran titre est une seule composition centrée : son origine, ses quatre rangées et la boîte de difficulté prennent (+16, −4). La boîte de collision du lien (relative à l'origine) suit ; elle déborde de 3 lignes sous l'écran, ce qui est sans effet (le curseur est borné à 119).
- **Écran des technologies.** Il s'ouvre sous le bouton technologies, en haut à gauche, et descend : il reste en (0,11). Son infobulle suit la règle des infobulles.
- **`map_tables.c` ne change pas.** Relevé fait : il ne contient aucune position d'écran ; ses 118, 119, 120 et 127 sont des numéros de sprite dans les lignes d'infobulle. Le plan le citait par prudence.
- **Ce qui ne bouge pas.** Les 128 d'échelle du monde (`galaxy`, `probe`, `slipway`, `planet_display`, `wormhole`), le 126 et le 176 de `slipway_display` (des périodes), les bornes de la caméra (−64 … 320), la caméra de départ (étape 3), la feuille de sprites et la carte, `tools/cursor_anchors.txt` (étape 5), l'oracle et les étiquettes.
- **Commentaires.** Aucun commentaire de récit. Le commentaire-piège du rideau reste ; s'il nomme 127, il nomme désormais le bas de l'écran.

## Implementation Chunks

Chaque chunk est un cycle TDD et un commit : tests d'abord, rouges, puis le changement, puis la mise à jour des tests qui figeaient l'ancienne position. Terminé quand `make test` passe ; `make e2e` est rouge par construction. Chaque chunk finit par une vérification au dump texte (`make replay EVENTS=…`, voir Testing Decisions).

1. **Barres du haut et du bas.** `hud_draw_bar` pleine largeur ; filet du bas, cadre bas-gauche, relevés crédits et revenu ancrés en bas ; menu galaxie 4 (pause) et menu pause ancrés en bas ; menu galaxie 3 et relevé du bonheur au centre haut. Tests : une barre dessinée remplit la colonne `W − 1` ; le cadre du score dessiné seul occupe les lignes `H − 2` et `H − 1` sur toute la largeur et rien en dessous de `H − 11` hors du cadre bas-gauche ; le bouton pause est en (0, H − 11) et le menu pause ouvert pose sa première ligne en (0, H − 21) puis monte de 11 ; le bouton du score est en (W/2 − 17, 0).
2. **Calé à droite.** Ajout de `P8_FIXED_INT` (avec son test). Cadre haut-droit, relevés mois et année, menu galaxie 1, `rest_cursor` des icônes de structure, bords droit et bas de la boîte panneau. Tests : menu galaxie 1 en (W − 10, 0) et son rectangle touche la colonne `W − 1` ; la première icône révélée est en (W − 10, H − 8), la suivante 10 pixels à gauche ; la boîte panneau vaut (10,10)–(W − 10, H − 10) ; un glisser de structure lâché en (W − 5, H/2) n'a pas de cible (annulation), lâché en (W − 15, H/2) en a une.
3. **Infobulles.** Appel du bouton en `(H − 1, 1)` ; texte centré sur W/2. Tests : l'infobulle d'un bouton survolé a sa dernière ligne de trame en `H − 1` et sa première en `H − 1 − (8 n + 2)` ; une ligne de texte de largeur paire est centrée sur W/2 (même nombre de colonnes vides à gauche et à droite, à 1 près) ; l'infobulle d'une icône de structure commence toujours en 0.
4. **Panneaux, menus et boîtes centrés.** Bannière de l'année et compte à rebours ; boîte de difficulté (`fixed_pos`) ; l'écran des technologies reste en (0,11). Tests : le texte « year N » est centré sur W/2 et son filet occupe les lignes `H/2 − 22` et `H/2 − 21` ; les quatre boutons de difficulté glissent vers (W/2 − 30, H/2 − 8 + 11 k) ; la première ligne de l'écran des technologies est toujours en (0,11).
5. **Écran de score final.** Fonds et lignes de `score_rows_table` décalés de (+16, −4) ; boutons « again? » et retour ; bande du bas du rideau. Tests : les sept lignes du score ont x = W/2 et y = H/2 + (y₁₂₈ − 64) ; le boîtier du tableau dessiné (de la première à la dernière ligne non noire du fond) est centré sur W/2 ; « again? » est en (W/2 − 13, H/2 + 36) ; le rideau à h maximal remplit les lignes `H − 1 − h` à `H − 1`, et pas `H − 2 − h`.
6. **Écran titre.** Origine, rangées et boîte de collision du lien. Tests : le logo dessiné occupe les colonnes W/2 − 64 à W/2 + 63 ; la souris en (W/2, H/2 + 48) (sur le lien) le fait survoler, en (W/2, H/2 − 54) non ; le curseur d'aide s'affiche sur le lien. Vérification au dump de `worst-case/title_idle.json`.

## Testing Decisions

- **Un bon test** vérifie ce qui se voit : pixels dessinés, position lue d'une entité construite, cible d'un glisser. Il ne relit pas l'expression de la source. Les positions attendues s'écrivent depuis `P8_SCREEN_WIDTH` / `P8_SCREEN_HEIGHT` (c'est une règle de mise en page, pas un format) ; un hash d'écran complet reste un littéral.
- **Tests nouveaux ou réécrits** : un par groupe, dans le fichier de test du module (voir Implementation Chunks). Aucun nouveau binaire de test ; `make test` ne s'allonge pas.
- **Tests à mettre à jour.** La liste exacte se mesure en lançant `make test` après chaque chunk, sur l'état réel laissé par l'étape 3. Candidats attendus, d'après le relevé sur HEAD : `test_hud` (barre), `test_tooltip_draw_at` (texte à W/2, et l'appel de bouton passe `H − 1`), `test_button` (infobulle survolée, menu galaxie), `test_ui` (`panel_box` 118 figé, sondes de souris dans la boîte, `on_screen` qui suppose 8–120), `test_score_panel` (cadre, relevés, bannière, lignes du score, rideau), `test_message_box`, `test_structure_icon` (`rest_cursor` 118,120 figé ; le test de dessin que l'étape 3 a déplacé peut revenir à la position de repos), `test_help_screen` (`fixed_pos` 34,56 ; souris en (64,112) et (64,10) ; dessin du curseur d'aide), `test_layout_row`, `test_modal_backdrop`. Un hash ne se refige qu'après avoir vérifié que l'assertion rouge est un hash d'écran et que le dump correspondant montre l'élément à sa nouvelle place ; toute autre assertion rouge est une prémisse à réparer selon l'ancre de l'élément visé.
- **Vérification au dump texte**, hors suite et sans fenêtre : `make replay EVENTS=worst-case/title_idle.json` (logo centré, lien à sa place, boîte de difficulté si ouverte) et `make replay EVENTS=` sur un e2e court qui finit en partie, par exemple `tests/e2e/colony_panel.json` (filets en lignes 0–1 et 118–119 jusqu'à la colonne 159, bouton « année suivante » en colonnes 150–159, cadre bas-gauche en 110). Les rejeux ne sont pas convertis : leur trajectoire diverge, seule l'interface fixe se lit dans le dump.
- **Relevé final** : une recherche de `127`, `128` et `64` dans `src/game/` ne trouve plus que les exceptions nommées (échelle du monde, périodes, numéros de sprite, tailles de tampon, demi-largeur du logo). Ce relevé est une vérification de fin d'étape, pas une garde ajoutée à `make test`.
- **Non-régression** : `make test` vert à chaque chunk ; aucun fichier de `tests/plan12-oracle/`, `tests/plan12-cursor-labels/` ni `tools/cursor_anchors.txt` modifié ; `make raylib` compile si le vendoring est présent.
- **Art antérieur** : `tests/test_tooltip_draw_at.c` (hash d'une infobulle à un `om` donné), `tests/test_score_panel.c` (`test_update_rows_show_final_score`, `test_readouts_year_banner_start_as_their_rows`), `tests/test_ui.c` (`panel_box` lu sur l'archétype), `tests/test_structure_icon.c` (`rest_cursor`), `tests/test_help_screen.c` (souris sur le lien, boîte de difficulté), `tests/test_fixed.c`.

## Hypothèses sur l'issue de l'étape 3

L'étape 3 est en cours : sa PRD et son chunk 1 (« tout dériver, à 128×128 ») sont commités, pas son chunk 2. Ce qui suit suppose une issue conforme à sa PRD. Si une hypothèse tombe, revérifier les sections listées.

- **H-3-1** — `P8_SCREEN_WIDTH` vaut 160 et `P8_SCREEN_HEIGHT` 120 ; le dump commence par `160 120` ; `make test` est vert et `make e2e` rouge. À revérifier : Solution (colonne « à 160×120 »), Implementation Chunks (critère de fin), Testing Decisions (vérification au dump).
- **H-3-2** — L'étape 3 n'a touché à aucune position d'interface : toutes les valeurs de la colonne « aujourd'hui » de la table des ancres sont encore en place, y compris `hud_draw_bar` à 127, `tooltip_draw_at(…, 127, 1)`, le `rest_cursor` (118,120) et la `panel_box` (10,10,118,118). Si l'une a bougé, partir de sa valeur réelle et lui appliquer l'ancre de la table. À revérifier : Table des ancres, Implementation Decisions, Implementation Chunks 1 à 6.
- **H-3-3** — L'étape 3 n'a ajouté aucune conversion entier → fixe à la compilation. Si elle en a ajouté une, on la réutilise au lieu de `P8_FIXED_INT`. À revérifier : Implementation Decisions (archétypes en virgule fixe), chunk 2, User Story 24.
- **H-3-4** — Les tests que l'étape 3 a refigés (hashes `ui`, `button`, `help_screen`, `message_box`, `score_panel`, `tooltip_draw_at`, `layout_row`, `modal_backdrop`, `structure`, `toast`…) l'ont été à 160×120 avec l'interface non ancrée ; ceux qui couvrent un élément ancré se refigent à nouveau ici. Le test de dessin de `structure_icon` place l'icône à une position visible, sans figer (118,120) comme position de dessin. À revérifier : Testing Decisions (tests à mettre à jour).
- **H-3-5** — `test_state` a été affaibli avec un `TODO` (cas `trade_rich_mine`) et ne lit aucune position d'interface : cette étape n'y touche pas. À revérifier : Testing Decisions (tests à mettre à jour).
- **H-3-6** — La caméra de départ est (176,196) et `entity_geom_is_on_screen` suit le cadre 160×120 ; rien de cette étape n'en dépend, sauf les tests qui posent la souris relativement à la caméra, déjà décalés par l'étape 3. À revérifier : Implementation Decisions (ce qui ne bouge pas).
- **H-3-7** — Ni l'oracle ni les étiquettes n'ont été régénérés, et `make replay EVENTS=…` écrit toujours son dump dans `tmp/` sans toucher aux fichiers versionnés. À revérifier : Testing Decisions (vérification au dump, non-régression).
- **H-3-8** — La liste des tests de `make test` est la même qu'avant l'étape 3 (aucun test ajouté ni retiré). À revérifier : Testing Decisions (aucun nouveau binaire).

## Out of Scope

- La conversion des `events.json` et des `.txt`, et la correction de `tools/cursor_anchors.txt` (étape 5) ; la comparaison à l'oracle (étape 6) ; le rebaseline des `*.sha256`, le `README` et les commentaires « 128×128 » hors fichiers touchés (étape 7).
- Rendre `make e2e`, `make state`, `make full-game` ou `cursor-labels-test` verts. En particulier, le cas d'intégration de `cursor-labels-test` (première étiquette `help_screen` sur l'écran titre) peut changer avec l'écran titre ; ce n'est pas une condition de fin.
- Tout ce qui est dans le monde : panneaux de planète, toasts, infobulle de pose, slipways, sondes ; la caméra, ses bornes et sa position de départ.
- Une garde statique contre les 127/128/64 en dur : le relevé est manuel, en fin d'étape.
- Une refonte de la mise en page (élargir la barre du haut, répartir autrement les relevés) : on applique l'ancre, rien de plus.
- PSX et audio ; toute entrée dans `docs/trous-de-portage.md`.

## Further Notes

- **Pour l'étape 5 — écarts avec `tools/cursor_anchors.txt`.** La table dit `button centre centre`, mais l'étiquette `button` couvre ici des boutons d'ancres différentes : rangées du menu pause (gauche bas), de l'écran des technologies (gauche haut), de la boîte de difficulté et de la fin de partie (centre), et rangées des panneaux de planète, qui sont dans le monde (carte : +16 / −4, ce qui se trouve égal au centre). Une table classe → ancre ne peut pas convertir à la fois un clic sur le menu pause et un clic sur la boîte de difficulté : l'étape 5 aura besoin d'une étiquette plus fine que la classe (par exemple le nom de la boîte ouvrante) pour `button`. Les autres entrées (`button:galaxy_menu_1` droite haut, `_2` gauche haut, `_3` centre haut, `_4` gauche bas, `structure_icon` droite bas, `help_screen` centre, `modal_backdrop` centre) correspondent à la table des ancres de cette PRD.
- **Risque visuel à faire relire par l'humain** : l'écran des technologies reste en haut à gauche et descend de 11 par ligne ; avec la barre du bas remontée de 8, une liste longue (9 lignes et plus, description comprise) peut toucher le cadre bas-gauche (y 110). Ce n'est pas un trou de portage (règle du plan : un comportement « à l'écran » qui change avec le cadre n'y entre pas) ; à regarder dans les captures de l'étape 7.
- **Le « contenu y 12–122 » du plan.** Les valeurs relevées du tableau de score vont de 20 à 110 après décalage (21–110 avant, fonds et boutons compris). La règle est la même (+16, −4 pour tout le contenu), seules les bornes citées par le plan étaient approximatives.
- Sans humain disponible, les points que `to-prd` fait valider ont été tranchés selon `guideline` (le plus simple qui tienne, hypothèses écrites) :
  - **Modules** : pas de module d'ancrage. L'ancre se dit dans l'expression (`W − d`, `H − d`, `W/2 + dx`) ; un module « ancres » n'aurait qu'un appelant par valeur et cacherait la valeur à 128 qu'on veut pouvoir relire. Seul ajout au runtime : la macro `P8_FIXED_INT`, faute de quoi les archétypes resteraient en hexadécimal figé, contre la règle du plan.
  - **Tests** : un test par groupe, dans les fichiers existants, plus le test de la macro dans `test_fixed`. Pas de test qui relit la source.
  - **Chunks** : un par groupe de la liste du plan, dans l'ordre du plan ; la macro entre au chunk 2, le premier qui en a besoin.
  - **Écran titre entièrement centré** plutôt que lien ancré en bas : c'est une composition unique, et la table de l'étape 2 dit déjà `help_screen centre`. Le lien reste lisible (y 106–117).
  - **Menu pause ancré en bas** et non centré, bien que le plan parle de « menus centrés sur x = 64 » : il part de x = 0 et s'ouvre au-dessus du bouton pause du coin bas-gauche ; le bord le plus proche est le bas-gauche.
  - **Écran des technologies non déplacé** : il part de x = 0, sous le bouton du coin haut-gauche ; le bord le plus proche est le haut-gauche.
  - **Bouton du score et relevé du bonheur au centre haut** : c'est le groupe du milieu de la barre (x 47–75 autour de 64) et l'ancre que donne déjà la table de l'étape 2 pour `button:galaxy_menu_3`.
  - **Boîte panneau ancrée bord par bord** : c'est une marge de 10 pixels sur chaque bord, pas un objet calé à un coin.
  - **Pas de garde statique** contre les nombres en dur : une recherche ponctuelle suffit pour une étape, et la règle du projet interdit d'allonger `make test` pour une vérification que le plan ne demande pas.
