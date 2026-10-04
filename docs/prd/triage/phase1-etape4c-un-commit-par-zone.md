---
title: "Un commit par zone : chaque zone, chaque place et chaque cap à son pic plus 1/8"
description: "Les réductions de 4b ont ramené les pics sous 768 Ko, mais les zones, leurs places et la file de sortie gardent leurs anciennes limites (1 522 688 o réservés) ; il faut les retailler une à une sur le pic mesuré plus la marge, sans casser le build ni un rejeu, et sans relever une limite sur une fuite."
status: needs-triage
---

## Problem Statement

L'étape 4b du plan 7 est close (`128eed0`). Les cinq réductions ont donné
leur gain réel ; la section 2 du plan finit sur : somme des cibles des zones
**745 034 o**, reste **41 398 o** sous 786 432. Mais aucune limite n'a
bougé : `zone_sizes.h` réserve toujours 768 Ko pour `game` (dont 64 Ko de
`load`), 256 Ko pour `frame`, 335 Ko pour `ui`, 128 Ko pour `particles`,
et chaque place de `ui` et de `particles` garde sa taille et son compte
d'avant le bilan. La file de sortie, hors zones, garde 512 entrées.

Campagne `make state` relancée sur `128eed0` (29 rejeux, tous `clean`) :

| Zone / cap | Pic | Limite | Rejeu |
|---|---|---|---|
| `game` (génération) | 411 128 | 720 896 | `full_game` |
| `load` | 21 936 | 65 536 | `full_game` |
| `frame` | 18 000 | 262 144 | `full_game` |
| `ui` | 150 585 | 342 422 | `full_game_pause_restart` |
| `particles` | 61 855 | 130 822 | `full_game` |
| `button` | 12 places, 3 768 o | 24 × 6 240 | |
| `modal_backdrop` | 1, 240 o | 4 × 256 | |
| `message_box` | 3, 4 488 o | 6 × 4 624 | `game_end_bankrupt` |
| `toast` | 42, 296 o | 128 × 320 | `game_end_bankrupt` |
| `row` | 63, 840 o | 100 × 1 192 | |
| `ephemerals` | 5, **216 o** | 16 × **216** | |
| `particle` | 788 | 2 085 | |
| `particles` | 16, 140 éléments | 24 × 160 éléments | |
| `particles_help_screen` | **1**, 235 éléments | **1** × 768 éléments | |
| `burst` | 3, 568 o | 8 × 640 | |
| `transit_dots` | 385 | 680 | |
| `exit_queue` | 280 | 512 | `full_game_pause_restart` |
| `elements_transit_dots` | 21 | 24 | (fait en 4b) |

Ce qui manque pour lancer 4c :

- **Les valeurs exactes.** La section 2 donne les cibles des caps
  (« `button` 14 × 4 240 », …) calculées en 4a ; il faut les confirmer
  contre la campagne de fin de 4b et écrire, par zone, la constante qui
  change et sa nouvelle valeur.
- **Les trois relèvements annoncés par la section 2.** La génération de
  `game` (741 480 contre 720 896), `ephemerals` (248 o contre 216) et
  `particles_help_screen` (2 places contre 1). La règle 2 interdit de
  relever une limite avant d'avoir écarté une fuite ; il faut dire, pour
  chacun, ce qu'il en est aujourd'hui.
- **Les trous de la campagne.** Un pic qui n'apparaît dans aucun rejeu ne
  se voit pas dans `tmp/state.txt` ; tailler juste sur lui casse le jeu là
  où la campagne ne regarde pas. L'enquête de ce PRD en a trouvé un (voir
  `particles_help_screen`).
- **L'ordre.** Chaque zone a une assertion statique contre sa somme de
  places, et la zone `game` dérive sa génération de deux constantes : un
  ordre mal choisi casse la compilation entre deux commits.

## Solution

Sept commits, dans l'ordre des *Implementation Chunks*. Chaque commit :

1. ne touche qu'une zone (ou la file de sortie), ou prépare la mesure ou la
   correction de fuite qui précède un retaillage ;
2. pose la nouvelle valeur calculée par la règle 3 sur la campagne de fin
   de 4b : **pic × 9/8, arrondi au-dessus à la cellule de 8 o pour des
   octets, à la place entière pour un compte** ;
3. garde **tous** les `tests/e2e/*.sha256` identiques ;
4. passe `make test-all`, `make sanitize-net` et `make full-game` (le
   verrou qui attrape une zone trop petite) ;
5. relance `make state` et écrit dans son message la limite nouvelle à côté
   du pic, avec le rejeu qui le tient ; met à jour les tests unitaires qui
   figent l'ancienne limite (message `p8_fatal`, taille littérale) ;
6. ajoute à la section 2 du plan la valeur posée ; le dernier commit écrit
   la ligne « Fin de 4c ».

Fin de 4c attendue : **750 858 o** de zones (`game` 487 200, `frame`
20 256, `ui` 165 097, `particles` 78 305), reste **35 574 o** sous
786 432. `ZONE_BUDGET_BYTES` ne bouge pas (c'est 4d).

L'écart avec les 745 034 de la section 2 (+5 824) vient de deux
corrections trouvées en préparant ce PRD : `ephemerals` baisse au lieu de
monter (−384), la liste de l'écran d'aide doit tenir 580 particules et non
235 (+6 208).

## User Stories

1. En tant que pilote de 4c, je veux, par zone, la constante à changer et sa valeur exacte, pour ne pas refaire le calcul de la règle 3 dans chaque commit.
2. En tant que pilote de 4c, je veux que chaque valeur soit dérivée d'une ligne de `tmp/state.txt` nommée, pour qu'un relecteur refasse le calcul.
3. En tant que pilote de 4c, je veux un ordre de commits où chaque commit compile et passe le verrou, pour qu'un `git bisect` ne tombe jamais sur un build cassé.
4. En tant que pilote de 4c, je veux savoir quelles assertions statiques lient une zone à ses places, pour changer les deux dans le même commit.
5. En tant que pilote de 4c, je veux savoir quels tests unitaires figent une ancienne limite, pour les mettre à jour sans hésiter (les e2e font foi).
6. En tant que pilote de 4c, je veux une règle pour le test unitaire qui dépasse une zone retaillée, pour distinguer un scénario synthétique (on le met à jour) d'un trou de campagne (on s'arrête et on l'écrit).
7. En tant que pilote du plan, je veux le sort de chacun des trois relèvements de la section 2, pour que la règle 2 soit tenue.
8. En tant que pilote du plan, je veux savoir que la génération de `game` n'est plus un relèvement après 4b, pour ne pas chercher une fuite qui n'existe plus.
9. En tant que pilote du plan, je veux que la saturation de `ephemerals` (pic à la limite, 216 o) soit expliquée et corrigée comme une fuite, pour ne pas loger un bloc mort.
10. En tant que pilote du plan, je veux que la place de plus de `particles_help_screen` soit justifiée comme une marge sur un besoin d'une place, la fuite écartée, pour relever en connaissance de cause.
11. En tant que pilote du plan, je veux que la campagne voie l'écran titre au repos, pour que la liste de l'écran d'aide soit taillée sur son vrai pic.
12. En tant que pilote du plan, je veux la somme des zones et le reste après chaque commit, pour savoir avant 4d que 768 Ko tient.
13. En tant que pilote de 4d, je veux que 4c laisse `ZONE_BUDGET_BYTES` intact, pour que la seule différence de 4d soit le plafond.
14. En tant que joueur, je veux que le jeu se joue exactement comme avant (mêmes hashes e2e), pour que le retaillage soit invisible.
15. En tant que joueur, je veux que l'écran titre laissé au repos ne s'arrête jamais sur « zone slot particles_help_screen full », pour que la réduction ne casse pas le premier écran du jeu.
16. En tant que joueur, je veux qu'ouvrir le panneau de recherche ou de projets ne s'arrête pas sur une liste d'éphémères pleine, pour que la réserve exacte ne casse pas un menu.
17. En tant que développeur, je veux que la liste d'éphémères d'une interface réserve sa capacité à la création, pour qu'elle ne laisse plus de bloc mort dans sa place.
18. En tant que développeur, je veux que `make state` montre le nombre d'éléments d'une liste d'éphémères, pour que sa place, saturée par construction, se taille sur ses éléments comme les listes de particules.
19. En tant que développeur, je veux que la taille d'une place de liste se lise comme « en-tête d'arène + liste + capacité × 8 », pour que la relation avec la capacité reste visible.
20. En tant que développeur, je veux que le compte de places de particules suive le pic mesuré et non une formule de pire cas, pour que la règle 3 s'applique aussi à lui.
21. En tant que développeur, je veux que la génération et `load` restent deux constantes dont `GAME_ZONE_BYTES` est la somme, pour retailler l'une sans toucher l'autre.
22. En tant que développeur, je veux que `make test` et `make e2e` ne s'allongent pas, pour garder la boucle rapide.
23. En tant que relecteur, je veux un commit par zone, pour qu'un `git revert` rende sa taille à une seule zone.
24. En tant que relecteur, je veux que le message de chaque commit donne le pic, la marge, la valeur posée et la somme recalculée, pour vérifier l'arithmétique sans relancer la campagne.
25. En tant qu'agent sans fenêtre, je veux que toute la vérification passe par les rejeux sans fenêtre et `make state`, pour ne jamais lancer l'appli raylib.
26. En tant que mainteneur futur, je veux que les risques acceptés (points de transit à l'écran, file de sortie qui croît avec le monde, particules) soient écrits, pour ne pas les découvrir sur une partie plus longue.
27. En tant que pilote des plans 10 et 11, je veux que les tailles de places restent mesurées en x86-64 et nommées comme telles, pour savoir qu'elles se remesurent sur MIPS.

## Implementation Decisions

Les points où `to-prd` demande de vérifier avec l'utilisateur (découpage
en modules, modules à tester, besoin de lots) ont été tranchés sans humain,
en appliquant le skill `guideline` : hypothèses écrites, le plus simple qui
tient la règle, changements chirurgicaux, critère de succès vérifiable par
commit. Chaque écart avec la section 2 du plan est nommé et chiffré.

### Méthode commune

- **Règle 3.** Pic × 9/8, arrondi au-dessus : à 8 o pour une taille en
  octets (zone, place), à l'unité pour un compte (places, éléments,
  entrées). Le pic est celui de `tmp/state.txt` sur la campagne, après le
  lot 1 (qui ajoute un rejeu).
- **Place d'une liste.** Une place qui tient une liste réservée est pleine
  par construction : son pic d'octets égale sa limite et ne dit rien. On
  la taille sur ses **éléments** : en-tête d'arène 96 + `pointer_list_t`
  ou `entity_list_t` 24 + capacité × 8 (x86-64), comme le fait déjà la
  taille des listes de particules.
- **Octets réservés d'une zone à places** : somme de
  `SLOT_ZONE_BYTES(taille, compte)` = taille arrondie à 8 × compte + compte
  (les octets « pris »). La constante de la zone dans `zone_sizes.h` prend
  cette somme exacte ; l'assertion statique de la zone reste `<=`.
- **Hypothèse H-1** : la campagne (3 parties pleines, 26 e2e, et après le
  lot 1 l'écran titre au repos) couvre les pics atteignables. Un test
  unitaire qui dépasse une limite retaillée est lu ainsi : scénario
  synthétique (monde fabriqué, boucle d'allocation) → on met le test à jour
  sur la nouvelle limite ; scénario atteignable en jouant → trou de
  campagne : on s'arrête, on l'écrit dans le plan, on ajoute le rejeu qui le
  montre, et on reprend sur le nouveau pic (comme le lot 1).

### Les trois relèvements de la section 2

- **Génération de `game` : plus un relèvement.** 4a donnait une cible de
  741 480 contre 720 896 ; après les cinq réductions de 4b, le pic est
  **411 128** (`full_game.json`, frame 7643, retour au titre), cible
  462 520 : une baisse de 258 376. Rien à écarter.
- **`ephemerals` : saturation par une fuite, écartée, donc une baisse.**
  Le pic d'octets (216) égale la limite (216). Lecture (règle 2) : la liste
  d'éphémères d'une interface naît vide dans sa place, grandit à 4 puis à 8
  ; la croissance recopie et laisse le bloc de 4 mort dans la place :
  96 + 24 + 4 × 8 (mort) + 8 × 8 = **216**. Le besoin réel est le nombre
  d'éléments : mesuré dans une copie hors dépôt (compteur dans
  `ui_set_ephemerals` sur les 29 rejeux), **7** au plus (`full_game*`, et
  dix e2e des technologies et des panneaux), 4 ou 5 ailleurs. Correction :
  la liste réserve sa capacité à la création, **8** (7 × 9/8 = 7,9) ; la
  place tombe à **184** (96 + 24 + 64), sans bloc mort, au lieu des 248 de
  la section 2. Le dépassement de 8 reste ce qu'il est aujourd'hui au-delà
  de 8 : un `p8_fatal` de la place (la croissance à 16 ne tient pas).
  Écarté : garder la croissance et tailler sur 216 + 1/8, ce qui logerait
  le bloc mort.
- **`particles_help_screen` : 1 → 2 places, relèvement de marge, fuite
  écartée.** Le besoin est **une** place : l'écran d'aide est un singleton
  (`g_help_screen_up`), `idle_title_screen_settles` vérifie une seule liste
  sur 400 frames, et le pic de la campagne reste 1 à travers trois retours
  au titre et un « restart » ; il n'y a donc pas de liste jamais rendue.
  La deuxième place n'est que l'arrondi de la règle 3 (1 × 9/8 → 2), déjà
  comptée par la section 2. Retenu : appliquer la règle telle quelle,
  « la même pour toutes, caps compris ». Coût : une place de 5 346 o
  qu'aucun chemin du jeu ne prend ; noté comme point à porter à un humain.
- **Le trou trouvé en route : la liste de l'écran d'aide.** La section 2
  taille cette liste à 265 éléments (235 × 9/8) ; or l'écran d'aide émet
  5 particules par dessin, chacune vivant environ 116 dessins, et les rejeux
  le quittent vite. Mesuré : `idle_title_screen_settles` imprime un plafond
  de **580** (fenêtres 500, 580, 580, 580) ; un rejeu de 2 000 frames sans
  entrée donne `elements_particles_help_screen` **580**, stable. Avec 265,
  l'écran titre laissé au repos s'arrêterait en moins d'une seconde. Pic
  retenu 580, capacité **653**, place **5 344** (96 + 24 + 653 × 8). Le
  compte de particules (`particle`, 887) le couvre : au repos, le titre ne
  tient que les 580 de l'écran d'aide.

### Valeurs posées

Toutes sur la campagne de `128eed0`, lot 1 compris (il ne change que les
lignes de l'écran d'aide).

| Constante | Avant | Pic | Après |
|---|---|---|---|
| `FRAME_ZONE_BYTES` | 262 144 | 18 000 | **20 256** |
| `GENERATION_ZONE_BYTES` | 720 896 (dérivé) | 411 128 | **462 520** (posé) |
| `LOAD_ZONE_BYTES` | 65 536 | 21 936 | **24 680** |
| `GAME_ZONE_BYTES` | 786 432 (posé) | — | **487 200** (somme) |
| `BUTTON_SIZE` × `_COUNT` | 6 240 × 24 | 3 768 o, 12 | **4 240 × 14** |
| `MODAL_BACKDROP_SIZE` × `_COUNT` | 256 × 4 | 240 o, 1 | **272 × 2** |
| `MESSAGE_BOX_SIZE` × `_COUNT` | 4 624 × 6 | 4 488 o, 3 | **5 056 × 4** |
| `TOAST_SIZE` × `_COUNT` | 320 × 128 | 296 o, 42 | **336 × 48** |
| `ROW_SIZE` × `_COUNT` | 1 192 × 100 | 840 o, 63 | **952 × 71** |
| `EPHEMERALS_SIZE` × `_COUNT` | 216 × 16 | 7 éléments, 5 | **184 × 6** (réserve 8) |
| `UI_ZONE_BYTES` | 343 040 | — | **165 097** |
| `PARTICLE_CAPACITY` (places `particle`) | 2 085 (formule) | 788 | **887** |
| `PARTICLES_CAPACITY` / `PARTICLES_COUNT` | 160 / 24 | 140 / 16 | **158 / 18** (place 1 384) |
| `PARTICLES_HELP_SCREEN_CAPACITY` / `_COUNT` | 768 / 1 | 580 / 1 | **653 / 2** (place 5 344) |
| `BURST_SIZE` × `BURST_COUNT` | 640 × 8 | 568 o, 3 | **640 × 4** |
| `TRANSIT_DOTS_COUNT` | 680 | 385 | **434** |
| `PARTICLES_ZONE_BYTES` | 131 072 | — | **78 305** |
| `EXIT_QUEUE_CAP` | 512 | 280 | **315** |
| `SLIPWAY_TRANSIT_DOTS_RESERVE` | 24 | 21 | 24 (inchangé, fait en 4b) |

Sommes des places : `ui` = 59 374 + 546 + 20 228 + 16 176 + 67 663 +
1 110 = 165 097 ; `particles` = 29 271 + 24 930 + 10 690 + 2 564 +
10 850 = 78 305. Somme des quatre zones comptées par le budget :
487 200 + 20 256 + 165 097 + 78 305 = **750 858**, reste **35 574**.

### Structure de `zone_sizes.h` après 4c

- `GENERATION_ZONE_BYTES` et `LOAD_ZONE_BYTES` sont posées ;
  `GAME_ZONE_BYTES` devient leur somme (aujourd'hui c'est la génération qui
  est dérivée). L'assertion `load_zone_outside_game_zone_t` reste vraie.
- `UI_ZONE_BYTES` et `PARTICLES_ZONE_BYTES` restent des littéraux dans
  `engine/` (le socle ne connaît pas les places du jeu), égaux à la somme
  de leurs places ; leurs assertions dans `ui_zone.c` et
  `particle_zone.c` gardent `<=`.
- Les valeurs ne sont plus des multiples de Ko : elles s'écrivent en
  octets. Le commentaire « Real sizes in bytes… » reste (contrat).
- `ZONE_BUDGET_BYTES` reste 1 536 Ko ; son assertion tient à chaque commit
  puisque chaque commit baisse une zone.

### Ordre et non-casse du build

- Une zone à places et ses places changent **dans le même commit** : son
  assertion statique compare la somme des places à la constante de zone.
- La génération passe avant `load` : le lot 3 inverse la dérivation
  (génération posée, `GAME` = somme) ; le lot 4 ne touche alors que
  `LOAD_ZONE_BYTES`. Dans l'ordre inverse, baisser `load` d'abord ferait
  grandir la génération dérivée (de 720 896 à 761 752) le temps d'un
  commit.
- La correction d'`ephemerals` (lot 5) passe avant le retaillage de `ui`
  (lot 6) : elle se vérifie seule (place inchangée à 216, la réserve de 8
  y tient : 184), et le lot 6 n'est alors qu'un changement de constantes.
- Le rejeu du titre au repos (lot 1) passe avant `particles` (lot 7) : la
  valeur 653 doit sortir de `make state`, et `make full-game` doit la
  verrouiller.
- `frame` (lot 2) et la file de sortie (lot 8) sont indépendants ; `frame`
  vient tôt (le plus simple, le plus gros écart relatif), la file en
  dernier (hors budget de zones).

### Lot 1 — la campagne voit l'écran titre au repos

- Un rejeu `worst-case/title_idle.json` : 300 frames sans entrée depuis le
  titre (le plafond de 580 est atteint dans la fenêtre 100-199), ajouté à
  `FULL_GAME_EVENTS`, donc à `make full-game` et à `make state`. Ni
  `make test` ni `make e2e` ne s'allongent.
- Pas de code. `make state` doit sortir 30 rejeux `clean` et
  `cap elements_particles_help_screen - 580 768 worst-case/title_idle.json`.
- Écarté : prendre 580 du seul test unitaire. Il verrouille déjà (une
  capacité trop petite y ferait un `p8_fatal`), mais `make state` resterait
  aveugle et la prochaine remesure retomberait sur 235.

### Lot 2 — `frame` : 20 256

- `FRAME_ZONE_BYTES` = 20 256 (18 000 × 9/8 = 20 250). Pic tenu par
  `full_game.json` frame 4149 ; les e2e montent à 14 320, le titre au repos
  à 160.
- Tests figeant l'ancienne limite : le message attendu de
  `test_frame_zone` (« zone frame full, limit 262144 bytes ») ; les cas
  écrits en `FRAME_ZONE_BYTES / 2` suivent seuls. Sous ASan la zone garde
  ses deux moitiés (`BUMP_ZONE_HALVES`), `make sanitize-net` le vérifie.

### Lot 3 — génération de `game` : 462 520

- `GENERATION_ZONE_BYTES` = 462 520 (411 128 × 9/8 = 462 519), posée ;
  `GAME_ZONE_BYTES` = `GENERATION_ZONE_BYTES` + `LOAD_ZONE_BYTES`
  (528 056 le temps de ce commit, `load` encore à 64 Ko).
- Pic : `full_game.json` frame 7643 (ancienne génération plus écran titre).
  `make full-game` le verrouille, avec la nouvelle partie de
  `full_game_new_game` et le « restart » de `full_game_pause_restart`.
- Ligne `zone game` de `make state` : la limite affichée suit
  `GENERATION_ZONE_BYTES`, rien à changer dans `state.c`.

### Lot 4 — `load` : 24 680

- `LOAD_ZONE_BYTES` = 24 680 (21 936 × 9/8 = 24 678). `GAME_ZONE_BYTES`
  tombe à 487 200 par la somme.
- `load` tient les données du cart chargées une fois : son pic est le même
  sur les 30 rejeux (21 936 dès le titre).
- Test figeant l'ancienne limite : le message attendu de `test_game_zone`
  (« zone load full, limit 65536 bytes ») ; le cas qui alloue
  `LOAD_ZONE_BYTES` suit seul.

### Lot 5 — `ephemerals` : la liste réserve sa capacité

- La création d'une liste d'éphémères réserve **8** éléments dans sa place
  (`entity_list_init_reserve`) au lieu de naître vide. La taille et le
  compte de places ne bougent pas dans ce lot (216 × 16) : 184 o y tiennent.
- `make state` gagne une ligne `elements_ephemerals` (plus grand nombre
  d'éléments d'une liste d'éphémères, limite 8), relevé là où la liste est
  remise à l'interface, sur le modèle de
  `particle_zone_record_elements` ; `bytes_ephemerals` tombe à 184.
  Antériorité : les lignes `elements_*` des listes de particules.
- La réserve 8 vit comme constante nommée près de la zone `ui`, et la
  taille de place s'écrit depuis elle au lot 6 (96 + 24 + réserve × 8),
  comme `PARTICLES_SIZE`.
- **Test rouge** : après l'ouverture du menu de pause (4 boutons), la
  place d'éphémères tient 184 o (`ui_zone_max_bytes`) et la liste a une
  capacité de 8 ; rouge aujourd'hui (216, capacité 4 puis 8). Un second
  cas : 8 éléments ajoutés ne font grandir ni la place ni la liste.
- Test qui peut figer l'ancienne séquence : le hash `CLEAR_EPHEMERALS_HASH`
  de `test_ui`, s'il couvre la capacité de la liste ; il est mis à jour.
- `make state` attendu : `elements_ephemerals` 7 (`full_game.json`).

### Lot 6 — `ui` et ses places : 165 097

- Les six tailles et comptes du tableau ci-dessus, et
  `UI_ZONE_BYTES` = 165 097, dans le même commit (assertion
  `ui_zone_exceeded_t`).
- Le commentaire « Size: the largest slot the game builds… Count: what
  lives at once at the worst point of a frame » devient le contrat vrai :
  pic de `make state` plus 1/8, en-tête d'arène x86-64 compris.
- Tests figeant les anciennes limites : `test_ui_zone` (allocations
  littérales de 6 240 et 1 192, à écrire avec `ui_zone_slot_size`), tout
  message `zone ui <sorte> full, limit <n>` attendu ; `test_state_peaks`
  utilise des lignes fabriquées et ne bouge pas.
- `make state` attendu : chaque cap `bytes_*` et chaque compte sous sa
  nouvelle limite ; `zone ui` limite 165 097.

### Lot 7 — `particles` et ses places : 78 305

- `PARTICLES_CAPACITY` 158, `PARTICLES_COUNT` 18, `PARTICLES_HELP_SCREEN_CAPACITY`
  653, `PARTICLES_HELP_SCREEN_COUNT` 2, `BURST_COUNT` 4 (taille 640
  inchangée : 568 × 9/8 = 639), `TRANSIT_DOTS_COUNT` 434, et
  `PARTICLES_ZONE_BYTES` = 78 305, dans le même commit (assertion
  `particle_zone_exceeded_t`).
- **`PARTICLE_CAPACITY` passe de la formule de pire cas au pic mesuré**
  (887). La formule (`BURST_COUNT × 140 + … + 5`) donnerait 1 405 avec les
  nouveaux comptes, et ne suppose pas ce que la campagne voit ; elle
  disparaît avec ses constantes (`BURST_PARTICLES`, `EMITTER_WORST_CASE`,
  `HELP_SCREEN_PER_FRAME`) si plus rien ne les lit. L'assertion
  `particle_zone_holds_help_screen_t` reste (887 ≥ 653).
- Le commentaire « Accepted risk: … 680 slots hold about thirty such
  visible slipways » est réécrit sur 434 (une vingtaine de slipways de 21
  points visibles à la fois).
- Tests figeant les anciennes limites : `test_particle_zone`
  (« limit 2085 slots », « limit 680 slots ») ; les cas écrits avec les
  constantes de capacité suivent seuls.
- `make state` attendu : `elements_particles_help_screen` 580 sous 653,
  `elements_particles` 140 sous 158, `particle` 788 sous 887,
  `particles_help_screen` 1 sous 2.

### Lot 8 — file de sortie : 315

- `EXIT_QUEUE_CAP` = 315 (280 × 9/8). Hors zones : le tableau statique
  passe de 8 192 à 5 040 o (entrées de 16 o), sans effet sur
  `ZONE_BUDGET_BYTES`.
- Pic : retour au titre de `full_game_pause_restart` (frame 7505), le monde
  abandonné qui attend le vidage. Transitoire de séquence, mais
  proportionnel au nombre d'objets du monde : une partie plus grosse que
  `full_game.json` le dépasserait. Même hypothèse que les réserves des
  registres de 4b (H-1) ; écrit dans le message de commit.
- Test figeant l'ancienne limite : `test_exit_queue`
  (« exit queue full, limit 512 ») ; la boucle écrite avec
  `EXIT_QUEUE_CAP` suit seule.
- Ce lot écrit la ligne « Fin de 4c » de la section 2 du plan : somme
  750 858, reste 35 574.

## Implementation Chunks

Huit lots, huit commits, dans cet ordre. Chacun est fini quand
`make test-all`, `make sanitize-net` et `make full-game` passent, que
`git diff` ne touche aucun `tests/e2e/*.sha256`, et que `make state` montre
chaque pic du lot sous sa nouvelle limite.

1. **Titre au repos dans la campagne** — un rejeu de 300 frames dans
   `worst-case/`. Vérifie : `elements_particles_help_screen` 580.
2. **`frame`** — 20 256. Vérifie : pic 18 000, `full_game.json`.
3. **Génération de `game`** — 462 520, `GAME` devient la somme. Vérifie :
   pic 411 128, trois parties pleines `clean`.
4. **`load`** — 24 680, `GAME` 487 200. Vérifie : pic 21 936.
5. **Réserve des éphémères** — 8 à la création, ligne `elements_ephemerals`.
   Vérifie : test rouge puis vert ; `bytes_ephemerals` 184,
   `elements_ephemerals` 7.
6. **`ui`** — six places et 165 097. Vérifie : chaque cap `ui` sous sa
   limite.
7. **`particles`** — cinq sortes et 78 305, `PARTICLE_CAPACITY` mesuré.
   Vérifie : chaque cap `particles` sous sa limite, titre au repos `clean`.
8. **File de sortie** — 315 ; ligne « Fin de 4c » du plan. Vérifie : pic
   280 ; somme 750 858.

## Testing Decisions

- **Un bon test** regarde ce qu'un appelant observe : un rejeu qui va au
  bout, un `p8_fatal` qui nomme sa zone et sa limite, les octets d'une
  place, la capacité d'une liste. Pas l'ordre interne des allocations.
- **Le verrou est `make full-game`** : une zone trop petite pour une partie
  pleine (ou, après le lot 1, pour le titre au repos) y fait un `p8_fatal`.
  `make state` donne les chiffres, `make full-game` les garantit.
- **Le seul test rouge** est celui du lot 5, le seul qui change du code de
  jeu. Les autres lots sont des constantes : leur preuve est la campagne,
  le verrou et les hashes e2e.
- **Modules testés** : `ui` (réserve d'éphémères, dans `test_ui` ou
  `test_live_slots` via le menu de pause), les zones (messages de limite
  dans `test_frame_zone`, `test_game_zone`, `test_ui_zone`,
  `test_particle_zone`, `test_exit_queue`).
- **Antériorité** : `idle_title_screen_settles` (déjà un verrou de la liste
  de l'écran d'aide dans `make test`), `case_particles_help_screen_*` de
  `test_particle_zone`, les lignes `elements_*` de `state.c` et
  `test_state`.
- **Tests figeant une limite** : mis à jour sur la nouvelle, de préférence
  en lisant la constante ou l'accesseur (`ui_zone_slot_size`,
  `particle_zone_count`) plutôt qu'un nouveau littéral ; chacun nommé dans
  le message du commit. Les e2e, jamais.
- **Durée** : aucun rejeu ajouté à `make test` ni à `make e2e` ; le rejeu
  du lot 1 va dans `make full-game` et `make state`, hors de
  `make test-all`.

## Out of Scope

- `ZONE_BUDGET_BYTES` (4d).
- Toute nouvelle réduction : un pic qui paraîtrait réductible pendant 4c
  s'écrit dans le plan, il ne se code pas ici (la correction d'`ephemerals`
  n'en est pas une : c'est la règle 2 appliquée avant un retaillage).
- Les blocs morts des listes hors registres (laissés par 4b) et les
  arènes exactes de wormholes restées sur la liste libre : mesurés dans le
  pic, pas corrigés ici.
- `PLANET_GRAB_ARENAS_FREE_MAX` et les autres bornes gardées par un
  `assert` sans ligne dans `make state` : pas des caps mesurés.
- Exempter un singleton de la marge (la deuxième place de l'écran d'aide) :
  décision humaine, voir *Further Notes*.
- Changer la marge (1/8) ou le plafond (768 Ko).
- Les tailles sur MIPS (plan 11) ; l'audio.

## Further Notes

- **Mesures faites pour ce PRD**, hors dépôt : `make state` sur
  `128eed0` (29 rejeux `clean`, chiffres du tableau) ; un compteur
  temporaire dans `ui_set_ephemerals` sur une copie (`git archive`) pour
  les 7 éléments ; un rejeu de 2 000 frames vides depuis le titre
  (`elements_particles_help_screen` 580, `particle` 580, `ephemerals` 3,
  `bytes_button` 720, `zone game` 1 608 : rien d'autre ne monte).
- **Écarts avec la section 2 du plan**, à y écrire au lot concerné :
  `ephemerals` 6 × 184 au lieu de 6 × 248 (−384, lot 5-6) ;
  `particles_help_screen` 2 × 5 344 au lieu de 2 × 2 240 (+6 208, lots
  1 et 7). Somme 745 034 → 750 858, reste 41 398 → 35 574.
- **Points à porter à un humain** (sans bloquer 4c) :
  - la deuxième place de l'écran d'aide coûte 5 346 o que rien ne prend ;
    l'exempter de la marge ramènerait le reste à 40 920 ;
  - trois pics croissent avec la partie, pas avec le code : points de
    transit visibles (434 places), file de sortie au retour au titre (315),
    particules (887). Une partie plus longue ou plus dense que
    `full_game.json` les verrait dépasser ; le verrou ne le dira que si elle
    entre dans `worst-case/` ;
  - la campagne ne couvrait pas l'écran titre au repos ; d'autres états de
    repos (fin de partie laissée ouverte, menu de pause tenu longtemps)
    pourraient avoir le même défaut. Ils ne montent pas les caps de
    `particles` au-delà de la partie en cours, mais le lot 1 n'en a vérifié
    qu'un.
- **Arithmétique de contrôle** : `ui` 14 × 4 240 + 14 = 59 374 ;
  2 × 272 + 2 = 546 ; 4 × 5 056 + 4 = 20 228 ; 48 × 336 + 48 = 16 176 ;
  71 × 952 + 71 = 67 663 ; 6 × 184 + 6 = 1 110. `particles` 887 × 32 + 887
  = 29 271 ; 18 × 1 384 + 18 = 24 930 ; 2 × 5 344 + 2 = 10 690 ;
  4 × 640 + 4 = 2 564 ; 434 × 24 + 434 = 10 850.
