---
title: "Plan 12, étape 5 — Convertir les rejeux"
description: "Les 31 rejeux (26 e2e, 5 worst-case) donnent des positions de souris d'un écran 128×128 ; il faut les réécrire pour l'écran 160×120, frame par frame selon ce qui est sous le curseur, et prouver que chaque frame vise encore la même chose."
status: needs-triage
---

## Problem Statement

Après les étapes 3 et 4, le jeu dessine 160×120, la caméra part de (176,196) et l'interface est ancrée à ses bords. Mais les 31 rejeux (les 26 e2e de `tests/e2e/` et les 5 de `worst-case/`) donnent toujours, frame par frame, les positions de souris d'un écran 128×128 :

- un clic « année suivante » en (118,0) tombe maintenant au milieu de la barre du haut, alors que le bouton est en (150,0) ;
- un clic « reasonable » en (60,74) rate la boîte de difficulté, désormais centrée sur (80,60) ;
- un clic sur la carte en (88,37) vise un autre point du monde, puisque le coin haut-gauche de la caméra a bougé de (−16,+4) ;
- une icône de structure glissée depuis (122,124) part de hors de l'écran (y > 119).

Les rejeux divergent donc dès la deuxième frame. `make e2e`, `make state` et `make full-game` ne disent plus rien du jeu, et l'étape 6 n'a rien à comparer à l'oracle. Les `.txt` qui décrivent chaque e2e citent des coordonnées qui ne sont plus celles du `.json`.

## Solution

Chaque frame est convertie selon ce qui était sous le curseur à 128×128. L'étape 2 (et 2bis) a écrit cette étiquette pour chaque frame de chaque rejeu dans `tests/plan12-cursor-labels/`, et `tools/cursor_anchors.txt` donne l'ancre de chaque étiquette, donc son décalage : x +0 / +16 / +32 pour gauche / centre / droite, y +0 / −4 / −8 pour haut / centre / bas, et (+16, −4) pour la carte (même centre du monde). Un outil temporaire réécrit chaque `events.json` à partir de sa version 128×128 : il ajoute à chaque frame le décalage de son étiquette, puis ramène le point dans le cadre 160×120, comme le fait le backend raylib.

Pour vérifier, on relance l'étiquetage sur le nouveau code avec les rejeux convertis. Chaque frame doit donner la même étiquette qu'à 128×128. La première frame qui diverge désigne une ancre fausse : on la corrige dans la table si une seule ligne l'explique, puis on reconvertit. Sinon, le rejeu sort de la boucle, noté avec sa frame et sa cause, pour l'étape 6. La boucle est bornée : une ligne de la table ne se corrige qu'une fois.

À la fin, les 31 rejeux sont convertis et leurs `.txt` citent les nouvelles coordonnées. Un rapport versionné dit, pour chaque rejeu, s'il est identique frame par frame ou à quelle frame et pour quelle cause il a quitté la boucle ; il note aussi les frames en butée de caméra et les frames ramenées dans le cadre. `make test` passe à chaque commit ; `make e2e` reste rouge, comme le prévoit le plan jusqu'à l'étape 7.

### Ce que la table dit déjà, contre l'étape 4

`tools/cursor_anchors.txt` (état 2bis) et la table des ancres de la PRD de l'étape 4 disent la même chose pour toutes les étiquettes rencontrées. La boucle devrait donc tourner peu, voire pas du tout.

| Étiquette | Frames (31 rejeux) | Ancre de la table | Décalage | Élément de l'étape 4 |
|---|---|---|---|---|
| `carte`, `planet`, `structure`, `processor`, `ascension`, `protostar`, `synthesizer`, `slipgate`, `infraspace_gate`, `trading_hub`, `burst` | 30 887 | carte | (+16, −4) | monde, suit la caméra |
| `button:monde` | 708 | carte | (+16, −4) | rangées des panneaux de planète (couche < 10) |
| `button:galaxy_menu_1` | 211 | droite haut | (+32, 0) | (118,0) → (150,0) |
| `button:galaxy_menu_2` | 288 | gauche haut | (0, 0) | (0,0) inchangé |
| `button:galaxy_menu_3` | 1 | centre haut | (+16, 0) | (47,0) → (63,0) |
| `button:galaxy_menu_4` | 330 | gauche bas | (0, −8) | (0,117) → (0,109) |
| `button:pause` | 5 | gauche bas | (0, −8) | menu pause (0,107) → (0,99) |
| `button:technologies` | 102 | gauche haut | (0, 0) | écran des technologies, reste en (0,11) |
| `button:difficulte` | 44 | centre centre | (+16, −4) | (34,56) → (50,52) |
| `button:fin_de_partie` | 3 | centre centre | (+16, −4) | « again? » (51,100) → (67,96) |
| `structure_icon` | 124 | droite bas | (+32, −8) | `rest_cursor` (118,120) → (150,112) |
| `modal_backdrop` | 173 | centre centre | (+16, −4) | plein écran, hit test toujours vrai |
| `help_screen` | 0 | centre centre | (+16, −4) | écran titre centré |

Relevé fait sur HEAD (`339b242`), en appliquant la table aux `.json` et aux étiquettes versionnées :

- **Points hors du cadre avant le recadrage** : 300 frames de `title_idle` (souris en (0,0), étiquette `carte`, donc (16,−4)), et 162 frames `modal_backdrop` (souris en (118,0) ou (50,3), donc y = −4 ou −1) dans `full_game`, `full_game_new_game`, `message_box_hold`, `game_end_final_score` et `game_end_bankrupt`. Aucune autre étiquette ne sort du cadre.
- **Butée de caméra** (`camera-clamp.txt`) : seul `camera_drag_dpad`, frames 15 à 18.
- **Glisser qui change d'ancre en cours de route** : seulement des structures glissées de leur icône (droite bas) vers la carte. Aucun glisser de caméra au clic droit ne passe sur un élément d'interface : les deltas de glisser de caméra sont donc conservés.

## User Stories

1. En tant que développeur du portage, je veux que chaque `events.json` des 26 e2e et des 5 rejeux de `worst-case/` soit réécrit pour l'écran 160×120, afin que les rejeux redeviennent un témoin du jeu.
2. En tant que développeur du portage, je veux que chaque frame soit décalée selon l'ancre de ce qui était sous le curseur à 128×128, afin qu'un clic sur un bouton calé à droite vise encore ce bouton et qu'un clic sur la carte vise le même point du monde.
3. En tant que développeur du portage, je veux que le décalage de chaque ancre ait une seule définition (l'en-tête de `tools/cursor_anchors.txt`), afin que l'outil et la table ne puissent pas diverger.
4. En tant que développeur du portage, je veux que la conversion parte toujours de la version 128×128 du rejeu, lue dans git à un commit fixé, afin que reconvertir après une correction de table ne décale pas deux fois.
5. En tant que développeur du portage, je veux que l'outil ne touche que les deux nombres `mouse_x` et `mouse_y` de chaque frame, au même endroit de la même ligne, afin que le diff d'un rejeu converti se lise ligne à ligne et que `mouse_buttons` et `dpad` restent intacts.
6. En tant que développeur du portage, je veux qu'un point converti hors du cadre soit ramené à [0, W − 1] × [0, H − 1], afin qu'un rejeu ne contienne que des entrées que le backend raylib peut produire.
7. En tant que développeur du portage, je veux que chaque frame ramenée dans le cadre soit listée, afin de savoir lesquelles ne suivent pas exactement leur ancre.
8. En tant que développeur du portage, je veux que l'outil échoue si un rejeu et son fichier d'étiquettes n'ont pas le même nombre de frames, afin qu'aucune frame ne soit convertie avec l'étiquette d'une autre.
9. En tant que développeur du portage, je veux que l'outil échoue sur une étiquette absente de la table, afin qu'aucune frame ne soit convertie avec un décalage par défaut.
10. En tant que développeur du portage, je veux que l'outil échoue sur une ligne de frame qu'il ne sait pas lire, afin qu'aucune frame ne reste à 128×128 en silence.
11. En tant que développeur du portage, je veux relancer l'étiquetage sur le nouveau code avec les rejeux convertis, sans toucher aux étiquettes versionnées, afin de comparer sans perdre la référence.
12. En tant que développeur du portage, je veux, pour chaque rejeu, la première frame dont l'étiquette diverge, avec l'ancienne et la nouvelle étiquette, afin de savoir quelle ancre regarder.
13. En tant que développeur du portage, je veux qu'une divergence sur une étiquette d'interface soit corrigée dans la table quand changer cette seule ligne fait passer la frame, sans faire diverger plus tôt un autre rejeu, afin que la table finisse juste.
14. En tant que développeur du portage, je veux qu'une ligne de la table ne se corrige qu'une fois, afin que la boucle soit bornée et qu'une étiquette d'ancres multiples ne fasse pas osciller la table.
15. En tant que développeur du portage, je veux qu'une divergence à une frame postérieure à une butée de caméra sorte le rejeu de la boucle avec la cause « butée », afin de ne pas corriger la table pour un décalage que la caméra a écrêté.
16. En tant que développeur du portage, je veux qu'une divergence sur une étiquette du monde sorte le rejeu de la boucle, avec la cause « recouvrement » (le point converti tombe sur une interface) ou « trajectoire » (le monde a divergé), afin de ne jamais toucher à la ligne `carte`, qui découle de la caméra.
17. En tant que développeur du portage, je veux qu'une divergence qu'aucune ancre de la ligne ne corrige, ou qui demanderait une deuxième correction de la même ligne, sorte le rejeu avec la cause « ancres multiples », afin de suivre la règle du plan.
18. En tant que développeur de l'étape 6, je veux un rapport versionné qui donne, pour chaque rejeu, « identique » ou la frame, la cause et les deux étiquettes de sa sortie de boucle, afin de savoir quels e2e courts réécrire et quels `full_game*` remonter à l'humain.
19. En tant que développeur de l'étape 6, je veux que le rapport donne aussi les frames en butée de caméra et les frames ramenées dans le cadre, même pour un rejeu identique, afin de surveiller ces frames-là contre l'oracle.
20. En tant que développeur de l'étape 6, je veux que le rapport liste les corrections de table (ligne, ancre avant, ancre après, rejeu et frame qui l'ont demandée), afin de relire chaque correction.
21. En tant que développeur de l'étape 6, je veux que les rejeux sortis de la boucle soient tout de même convertis avec la table finale, afin de partir d'eux pour une réécriture.
22. En tant que lecteur d'un e2e, je veux que son `.txt` cite les positions de souris du `.json` converti, afin que la description et le rejeu disent la même chose.
23. En tant que lecteur d'un e2e, je veux que les positions d'écran des objets du monde cités dans le `.txt` (« p27 » (88,37)) prennent (+16, −4), afin qu'elles restent vraies à 160×120.
24. En tant que lecteur d'un e2e, je veux que les positions de caméra citées dans le `.txt` (« la caméra passe de (192,192) à (298,298) ») soient relues sur le nouveau code, afin qu'une butée qui écrête le décalage ne laisse pas une valeur fausse.
25. En tant que lecteur d'un e2e, je veux que les positions d'interface citées sans clic (une infobulle « en bas d'écran (y=119) ») suivent l'ancre de l'élément décrit, afin que le `.txt` ne garde aucune coordonnée 128×128.
26. En tant que développeur du portage, je veux une liste des paires de coordonnées d'un `.txt` qui sont encore une position de souris 128×128 changée par la conversion, afin de ne pas en oublier.
27. En tant que développeur du portage, je veux que les assertions de trajectoire que `test_state` a retirées à l'étape 3 (cas `trade_rich_mine`) reviennent, lues dans l'oracle, si le rejeu converti les tient, afin que `make test` couvre de nouveau le format des lignes de slipway.
28. En tant que développeur du portage, je veux qu'un test unitaire qui rejoue un e2e converti et fige sa trajectoire, et qui rougit, soit affaibli avec un `TODO` qui renvoie à l'étape 6 et que le rejeu soit noté dans le rapport, afin que `make test` reste vert sans cacher l'écart.
29. En tant que développeur du portage, je veux qu'un rejeu converti qui fait planter le jeu (`p8_fatal`, slot non rendu sous `test_live_slots`) arrête l'étape et soit traité comme un bug, afin de ne pas commiter un `make test` rouge ni masquer un plantage.
30. En tant que développeur du portage, je veux un test de l'outil de conversion, hors de `make test`, afin que l'outil soit sûr sans allonger la boucle rapide.
31. En tant que développeur du portage, je veux que les cibles de conversion et de vérification soient dans la légende du `Makefile`, marquées temporaires (plan 12), hors de la suite, afin que l'étape 7 les retrouve pour les supprimer.
32. En tant que relecteur, je veux que l'oracle (`tests/plan12-oracle/`), les étiquettes d'origine et `camera-clamp.txt` ne soient pas réécrits, afin qu'ils décrivent toujours le code 128×128.
33. En tant que relecteur, je veux qu'aucun `*.sha256` ne soit refait, afin que le rebaseline reste l'affaire de l'étape 7, une fois l'oracle tenu.
34. En tant que relecteur, je veux qu'aucun code du jeu ne change à cette étape, afin que ce qui diverge soit imputable à la conversion ou au jeu de l'étape 4, et non à une retouche faite pour faire passer un rejeu.
35. En tant que relecteur, je veux qu'aucune entrée ne soit ajoutée à `docs/trous-de-portage.md`, afin de respecter la règle du plan.
36. En tant qu'humain, je veux qu'un `full_game*` sorti de la boucle soit signalé dans le rapport de fin d'étape, afin de décider moi-même, à l'étape 6, de la suite à donner.

## Implementation Decisions

### Format des rejeux

- Un `events.json` contient `{"frames": [ … ]}`, une frame par ligne, sous la forme `{"mouse_x": X, "mouse_y": Y, "mouse_buttons": B, "dpad": [4 ou 6 booléens]}`. Tous les rejeux du dépôt écrivent le JSON compact (`{"mouse_x": 60, …}`) avec un `dpad` de 4 booléens. Le parseur (`p8_input_load`) cherche les clés par sous-chaîne et lit des entiers signés avec `strtol`. `p8_input_save` écrit une autre mise en forme (espaces internes, `dpad` de 6 booléens).
- **La conversion n'emprunte pas `p8_input_save`.** Elle réécrit le texte en place : seuls les deux entiers qui suivent `"mouse_x": ` et `"mouse_y": ` changent. Tout le reste de la ligne et du fichier reste à l'octet près. Le diff d'un rejeu converti ne montre ainsi que des coordonnées, et `e2e_limits.sh` (qui compte les `"mouse_x"`) voit le même nombre de frames.
- La frame n du rejeu correspond à la ligne n + 1 de son fichier d'étiquettes. C'est le contrat de `cursor_label_update60` : l'étiquette est lue sur l'ui dont le hit test a distribué le clic de la frame.

### L'outil de conversion

- Un script shell et awk, temporaire, à côté de `tools/line_coverage.sh`, qui est l'art antérieur : un script à sous-commandes, testé par un binaire C hors suite. Il offre deux sous-commandes :
  - `convert <events 128> <étiquettes> <table> <sortie> <rapport>` écrit le rejeu converti et, dans le rapport, les frames ramenées dans le cadre (`frame x₁₂₈ y₁₂₈ → x y étiquette`) et la correspondance des positions (`x₁₂₈,y₁₂₈ → x,y` avec les frames concernées), qui sert à mettre à jour le `.txt`. Il échoue, sans rien écrire en sortie, si les nombres de frames et d'étiquettes diffèrent, si une étiquette manque dans la table, ou si une ligne de frame ne se lit pas.
  - `first-divergence <étiquettes d'origine> <étiquettes nouvelles>` écrit `identique`, ou `frame <n> <ancienne> <nouvelle>`. Une longueur différente est une divergence à la première frame manquante.
- **Décalages** : ce sont ceux de l'en-tête de `tools/cursor_anchors.txt`. Le x vaut gauche 0, centre +16, droite +32 ; le y vaut haut 0, centre −4, bas −8 ; `carte` vaut (+16, −4). Le script les porte comme constantes, citées depuis cet en-tête, qui reste la seule définition écrite des décalages.
- **Recadrage** : x est ramené à [0, 159] et y à [0, 119], les bornes de `raylib_backend_window_to_screen`. Une souris réelle ne sort pas du cadre. Cela touche, d'après le relevé, la souris immobile de `title_idle` et des frames `modal_backdrop` dont le hit test est toujours vrai. Le recadrage ne change pas leur étiquette, et la vérification le confirmera frame par frame.
- **La source est toujours la version 128×128**, lue par `git show <ref>:<chemin>` avec un `ref` fixé dans le `Makefile` : `339b242`, le commit où les étiquettes 2bis et les `.json` d'origine coexistent. Reconvertir après une correction de table repart donc de la même source. Les `.json` n'ont pas changé depuis l'étape 1.
- Cible `make replay-convert` (temporaire, hors suite) : elle convertit les 31 rejeux de `STATE_EVENTS` en place dans `tests/e2e/` et `worst-case/`, avec les rapports dans `tmp/replay-convert/<nom>.txt`. Avec `EVENTS=<chemin>`, elle convertit un seul rejeu.

### La vérification et la boucle

- Cible `make cursor-labels-check` (temporaire, hors suite) : pour chaque rejeu de `STATE_EVENTS`, `LABELS=` sur le binaire de rejeu courant, avec les étiquettes écrites dans `tmp/cursor-labels-check/<nom>.txt`, puis `first-divergence` contre `tests/plan12-cursor-labels/<nom>.txt`. Elle écrit une ligne par rejeu dans `tmp/cursor-labels-check.txt` et affiche le nombre de rejeux identiques. Elle échoue si un rejeu s'arrête (`p8_fatal`). **Elle n'écrit jamais dans `tests/plan12-cursor-labels/`.** La cible `make cursor-labels` en masse, qui réécrit ce répertoire, n'est pas lancée à cette étape : sur le code 160×120, elle écraserait la référence.
- **Butée** : la frame de butée d'un rejeu est la première frame de sa ligne dans `tests/plan12-cursor-labels/camera-clamp.txt` (critère de 2bis : coin haut-gauche à 128×128 en x ≤ −48 ou y ≥ 316). Seul `camera_drag_dpad` en a une (frame 15).
- **Boucle** (au plus une correction par ligne de la table, soit au plus 23 tours) :
  1. `make replay-convert`, puis `make cursor-labels-check`.
  2. Pour chaque rejeu encore ouvert dont la première divergence est la frame f, avec l'étiquette d'origine L et la nouvelle L′ :
     - si le rejeu a une frame de butée ≤ f, il sort de la boucle, cause **butée** ;
     - si L est une étiquette du monde (ancre `carte`, `button:monde` compris), il sort, cause **recouvrement** quand L′ est une étiquette d'interface, **trajectoire** sinon ; la ligne `carte` ne se corrige jamais ;
     - si L est une étiquette d'interface dont la ligne n'a pas encore été corrigée, on essaie les autres ancres de cette ligne en reconvertissant ce seul rejeu (`EVENTS=`). On retient l'ancre qui fait passer la frame f, et en cas d'égalité celle de la table des ancres de l'étape 4. On l'écrit dans la table, puis on reconvertit et revérifie les 31 rejeux. Si un rejeu jusque-là identique, ou dont la première divergence était plus loin, diverge alors plus tôt sur L, la correction est annulée et le rejeu qui l'avait demandée sort de la boucle, cause **ancres multiples** ;
     - si aucune ancre ne fait passer la frame f, ou si la ligne a déjà été corrigée, le rejeu sort, cause **ancres multiples**.
  3. On s'arrête quand chaque rejeu est identique ou sorti de la boucle.
- Un rejeu sorti de la boucle reste converti avec la table finale. Il n'est pas retiré de `tests/e2e/` : l'étape 6 le réécrit, ou le remonte à l'humain si c'est un `full_game*`.

### Le rapport

- `tests/plan12-conversion.txt` est un fichier versionné, temporaire (supprimé à l'étape 7 avec l'oracle et les étiquettes). Il contient un en-tête de format, puis une ligne par rejeu : `<nom> identique` ou `<nom> sorti <frame> <cause> <étiquette d'origine> <étiquette nouvelle>`, suivie des champs `butee <frames>|aucune` et `recadre <nombre>`. Il se termine par une section des corrections de table : `<ligne> <ancre avant> -> <ancre après> <rejeu> <frame>`, ou `aucune`.
- Ce n'est pas une doc de `docs/`. C'est une donnée d'entrée de l'étape 6, comme l'oracle.

### Les `.txt`

- Mis à jour à la main, ligne par ligne, avec la correspondance du rapport de conversion :
  - une position de souris prend sa valeur convertie ;
  - une position d'écran d'un objet du monde prend (+16, −4) ;
  - une position de caméra (coin haut-gauche) prend (−16, +4), sauf après une butée, où elle se relit sur le nouveau code (`make state` sur le rejeu ou sur une copie tronquée dans `tmp/`) ;
  - une position d'interface citée hors clic suit l'ancre de l'élément.
  Le nombre de frames, les noms de frames (`f27`), le récit et la ligne « Relecture humaine dans raylib : à faire. » ne changent pas.
- Une vérification (sous-commande ou ligne de `make replay-convert`) liste, pour chaque e2e, les paires `(x,y)` de son `.txt` qui sont une position de souris du `.json` 128×128 changée par la conversion et absente du `.json` converti. La liste doit être vide à la fin, ou chaque paire restante justifiée (une position du monde qui coïncide).
- Un `.txt` d'un rejeu sorti de la boucle est mis à jour de la même façon. Sa sortie est dans le rapport, pas dans le `.txt`.

### Les tests unitaires qui rejouent un e2e

- `make test` rejoue des e2e : `test_state` (`trade_rich_mine`, `cheat_code`) et `test_live_slots` (les 26 e2e, frame par frame, plus des captures de `pause_restart` et `tech_geoharvesting_burst`). Les rejeux convertis y entrent au commit de la conversion.
- **`test_state`, `TODO` de `trade_rich_mine`** : si `trade_rich_mine` converti finit avec les lignes de l'oracle (`count planets 93`, `count colonies 7`, `count slipways 7`, `count structures 0`, la ligne `planet p1 colonized planet e …`, une ligne `planet p2 unknown …`, `slipway p27 p1 normal`, `slipway p1 s1 normal`), ces assertions reviennent et le `TODO` part. Sinon le `TODO` reste et renvoie à l'étape 6, et l'écart est dans le rapport. L'assertion `camera world 176 196` reste vraie si la caméra n'a pas bougé pendant ce rejeu ; sinon, elle prend la valeur lue sur le nouveau code.
- Un test qui fige une trajectoire d'e2e et rougit sur le rejeu converti est affaibli avec un `TODO` qui renvoie à l'étape 6, comme l'étape 3 l'a fait, et le rejeu est noté dans le rapport. C'est la règle du projet : les e2e font foi.
- Un plantage (`p8_fatal`, slot non rendu, `test_live_slots` rouge sur un e2e converti) n'est pas une trajectoire. L'étape s'arrête, la conversion n'est pas commitée, et le plantage part en bug (`help-me-debug`).

### Ce qui ne bouge pas

Aucun code du jeu, `src/devtools/cursor_label.c`, l'oracle, les étiquettes d'origine, `camera-clamp.txt`, les `*.sha256`, `docs/trous-de-portage.md`. `tools/cursor_anchors.txt` ne change que par les corrections de la boucle.

## Implementation Chunks

Chaque chunk donne un commit, avec `make test` vert.

1. **L'outil.** Le script (`convert`, `first-divergence`, la liste des paires à revoir dans un `.txt`), son test C hors suite (`make replay-convert-test`) et la cible `make replay-convert`, avec la ligne de légende du `Makefile`. Les rejeux ne sont pas encore commités convertis.
2. **La boucle.** La cible `make cursor-labels-check` et sa ligne de légende. On déroule la boucle sur l'arbre de travail. Le commit porte les corrections de `tools/cursor_anchors.txt` (s'il y en a) et `tests/plan12-conversion.txt` ; les `.json` convertis ne sont pas commités, car `make replay-convert` les régénère à l'identique.
3. **Les rejeux.** `make replay-convert` avec la table finale, mise à jour des 26 `.txt`, puis `test_state` (restauration ou `TODO` maintenu) et tout autre test d'e2e qui rougit, selon la règle ci-dessus. Le commit porte les 31 `.json`, les 26 `.txt` et les tests touchés. `make test` est vert ; `make e2e` est rouge par construction.

## Testing Decisions

- **Un bon test** vérifie ce que l'outil écrit à partir d'entrées construites : octets du `.json` produit, ligne de `first-divergence`, code de sortie. Il ne relit pas le script.
- **Test de l'outil** (`tests/test_replay_convert.c`, binaire hors suite, cible `make replay-convert-test`), sur le modèle de `tests/test_line_coverage.c` (fichiers écrits dans `tmp/` par le test, script lancé par `system`). Cas :
  - chacune des neuf ancres et `carte` décale une frame de la bonne valeur ;
  - un point hors cadre est ramené en (0…159, 0…119) et apparaît dans le rapport ;
  - un `.json` à `dpad` de 6 booléens, ou à espaces internes (`{ "mouse_x": … }`), n'est modifié que dans ses deux nombres ;
  - des valeurs négatives se lisent ;
  - des nombres de frames et d'étiquettes différents, une étiquette inconnue ou une ligne illisible font échouer l'outil, sans fichier de sortie ;
  - `first-divergence` rend `identique`, la première frame divergente, et une divergence de longueur ;
  - une paire du `.txt` qui est une ancienne position de souris est listée, une paire déjà convertie ne l'est pas.
- **Vérification de bout en bout** : `make cursor-labels-check` après le chunk 3. Le résultat attendu est le rapport du chunk 2, à l'identique. `make replay-convert` relancé ne change aucun octet des `.json` commités.
- **Non-régression** : `make test` vert à chaque commit et pas plus long (aucun test ajouté à `UNIT_TESTS`). Aucun fichier de `tests/plan12-oracle/` ni de `tests/plan12-cursor-labels/` modifié. Aucun `*.sha256` modifié. `git diff` du chunk 3 limité à des nombres `mouse_x` / `mouse_y` dans les `.json`.
- **Ce qui n'est pas une condition de fin** : `make e2e`, `make state`, `make full-game`. Ni les cas d'intégration de `make cursor-labels-test` qui rejouent un e2e sur le code courant (`camera_drag_dpad` attend les frames de butée 15 à 18, un relevé fait à 128×128) : ils décrivent le code 128×128, et l'étape 7 supprime ce test. Ses cas unitaires (`cursor_label_read`, `cursor_label_camera_clamped`) restent verts.
- **Art antérieur** : `tests/test_line_coverage.c` et `tools/line_coverage.sh` (script à sous-commandes testé hors suite), la cible `cursor-labels` (écrire dans `tmp/`, vérifier, puis seulement copier), `tests/test_state.c` (assertions par `has(...)` sur l'état de la dernière frame).

## Hypothèses sur l'issue des étapes 2bis et 4

L'étape 2bis est commitée (`339b242`). Ce qui la concerne ci-dessus est relevé sur HEAD, et seules deux hypothèses en restent. L'étape 4 est en cours : sa PRD est en `docs/prd/triage/`. Si une hypothèse tombe, revérifier les sections listées.

- **H-2bis-1** — `339b242` est la forme finale de 2bis : ni les étiquettes, ni `tools/cursor_anchors.txt`, ni `camera-clamp.txt`, ni `cursor_label.c` ne changent d'ici l'étape 5, sauf par cette étape. Si l'un change, le `ref` de conversion devient le commit qui porte la nouvelle version, et la table « ce que la table dit déjà » se refait. À revérifier : Solution (table et relevé), Implementation Decisions (source de la conversion, butée).
- **H-2bis-2** — `camera-clamp.txt` ne couvre qu'un côté de la butée : x ≤ −48 ou y ≥ 316, là où le nouveau cadre est écrêté. Une caméra 128×128 écrêtée de l'autre côté (x = 320 ou y = −64), où le nouveau cadre ne l'est pas, casse aussi le décalage, sans être listée. On ne la mesure pas. Si elle arrive, la divergence porte sur une étiquette du monde et sort le rejeu avec la cause « trajectoire », et l'étape 6 en juge. Le relevé de 2bis (aucun `full_game*` en butée) laisse penser que le cas est absent. À revérifier : Implementation Decisions (boucle, butée).
- **H-4-1** — L'étape 4 pose chaque élément d'interface à la place que donne sa table des ancres, et ces places correspondent toutes à `tools/cursor_anchors.txt` (table « ce que la table dit déjà »). Si l'étape 4 a tranché autrement un élément (par exemple le menu pause ou l'écran titre), la boucle le trouvera, et la correction de ligne attendue est connue d'avance. À revérifier : Solution (table), Implementation Decisions (boucle).
- **H-4-2** — L'étape 4 ne touche ni `cursor_label.c`, ni la table, ni les étiquettes, ni l'oracle, ni les `.json`. Les classes et les couches (couche < 10 pour `button:monde`, couche du menu pause, `on_click` de la difficulté) qui fondent les étiquettes sont inchangées. À revérifier : Implementation Decisions (vérification), Testing Decisions.
- **H-4-3** — Le hit test de `modal_backdrop` reste « toujours vrai ». Les frames `modal_backdrop` ramenées dans le cadre gardent donc leur étiquette. À revérifier : Solution (relevé), Implementation Decisions (recadrage).
- **H-4-4** — `make test` est vert à la fin de l'étape 4, et le `TODO` de `test_state` (`trade_rich_mine`) est toujours là. À revérifier : Implementation Decisions (tests unitaires qui rejouent un e2e), chunk 3.
- **H-4-5** — Le backend raylib ramène toujours la souris dans [0, W − 1] × [0, H − 1], la justification du recadrage. À revérifier : Implementation Decisions (recadrage).
- **H-4-6** — Le binaire de rejeu (`tmp/replay`) et le crochet `LABELS=` marchent sur le code de l'étape 4 comme sur HEAD : une étiquette par frame, pas d'arrêt sur un rejeu 128×128. À revérifier : Implementation Decisions (vérification).

## Out of Scope

- Juger les rejeux convertis contre l'oracle (état normalisé, couverture de lignes), réécrire les e2e courts sortis de la boucle ou dont la prémisse dépend du hors-écran (`colony_income_offscreen`, cas GAP-26), et remonter un `full_game*` à l'humain : c'est l'étape 6.
- `make e2e-baseline`, la suppression des outils temporaires, de l'oracle, des étiquettes et du rapport, le `README` : c'est l'étape 7.
- Rendre verts `make e2e`, `make state`, `make full-game` ou les cas d'intégration de `make cursor-labels-test`.
- Toute retouche du code du jeu ou de l'interface pour faire passer un rejeu. Un écart d'ancrage de l'étape 4 se corrige dans la table de conversion, pas dans le jeu.
- Compenser l'écrêtage de la caméra en butée dans la conversion (un décalage par frame calculé depuis la caméra) : le plan sort ces rejeux de la boucle.
- Tenir un appui et son relâchement au même point quand leurs étiquettes diffèrent : chaque frame suit sa propre étiquette (voir Further Notes).
- Régénérer `tests/plan12-cursor-labels/` ou `camera-clamp.txt`.
- PSX, audio, `docs/trous-de-portage.md`.

## Further Notes

- **Ce qu'on attend de la boucle.** D'après le relevé, la table ne devrait pas bouger. `camera_drag_dpad` devrait finir identique ou sortir avec la cause « butée » à partir de la frame 15. Sur le nouveau code, la caméra part de (282,302) après le glisser au lieu de (298,298) : y touche 320 avant x, et x finit sous 320. L'état final n'en dit rien, puisque l'oracle n'a pas de ligne `camera`, mais la phrase « atteint la butée (320,320) en f16 » de son `.txt` est à relire sur le nouveau code. Les e2e dont la prémisse dépend du hors-écran (`colony_income_offscreen`) peuvent rester identiques frame par frame et échouer quand même à l'étape 6 : l'égalité des étiquettes est nécessaire, pas suffisante.
- **Une frame, une étiquette.** Un appui et son relâchement au même point à 128×128 peuvent être séparés par la conversion si l'objet sous le curseur change entre les deux, par exemple un bouton qui ouvre une boîte modale : l'appui prend (+32, 0) et le relâchement sur le fond modal prend (+16, −4). C'est ce que demande le plan (« le décalage de l'ancre à chaque frame »). Le seul cas relevé où l'ancre change pendant un bouton tenu est le glisser d'une structure de son icône vers la carte, et il doit changer : le point de dépose est dans le monde.
- **Recadrage plutôt que coordonnées négatives.** Le parseur accepte des négatifs, et le plan ne parle pas de recadrage. On recadre quand même : un rejeu doit décrire ce que le backend peut produire, et les frames touchées ont toutes une étiquette dont la position ne compte pas (fond modal plein écran, écran titre vide). Elles sont listées dans le rapport.
- **Sans humain disponible, les points que `to-prd` fait valider ont été tranchés selon `guideline`** (le plus simple qui tienne, hypothèses écrites) :
  - **Modules** : un seul outil temporaire, un script shell et awk à deux sous-commandes, sur le modèle de `tools/line_coverage.sh`. Pas de convertisseur en C sur `p8_input_load` / `p8_input_save` : ce couple réécrirait toutes les lignes (autre mise en forme, `dpad` à 6), et le diff ne serait plus lisible. Pas de module « ancres » partagé avec le jeu : la table reste une donnée de plan, supprimée à l'étape 7.
  - **Tests** : l'outil est testé, hors suite, parce qu'une erreur de décalage se verrait trop tard. La cible de vérification n'a pas de test propre : son cœur est `first-divergence`, testé avec l'outil, et le reste est de la plomberie `Makefile` sur le modèle de `cursor-labels`.
  - **Boucle conduite à la main, pas automatisée** : essayer les ancres d'une ligne se fait en quelques rejeux courts, et les règles de sortie sont écrites. Un script qui essaierait toutes les ancres coûterait plus que les corrections attendues, au nombre de zéro d'après le relevé.
  - **Borne** : une correction par ligne, plutôt qu'un nombre de tours. Elle découle de la définition « une seule ligne de la table l'explique », et elle empêche l'oscillation.
  - **Étiquettes du monde jamais corrigées** : leur décalage découle de la caméra de départ (règle du plan, étape 3). Le corriger reviendrait à cacher une divergence de trajectoire.
  - **Rapport dans `tests/`, pas dans `docs/`** : c'est une donnée lue par l'étape 6, temporaire comme l'oracle. `docs/` ne décrit que ce qui est vrai ou à faire.
  - **`ref` de conversion `339b242`** plutôt qu'une copie versionnée des `.json` 128×128 : git les a déjà, et une copie de quatre `full_game*` doublerait près de 31 000 lignes.
  - **Commits** : les `.json` convertis entrent dans le même commit que leurs `.txt` et que les tests mis à jour, pour qu'aucun commit ne présente un `.txt` faux ni un `make test` rouge.
  - **Restauration du `TODO` de `test_state` à cette étape**, comme le `TODO` le dit, mais seulement sur les lignes qu'il nomme. Le jugement complet contre l'oracle reste à l'étape 6.
