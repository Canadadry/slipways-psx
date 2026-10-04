---
title: "Cinq réductions, une par commit : rendre la zone frame, tailler les arènes de prise, flipper au titre, réserve de points de transit, réserves des registres"
description: "Le bilan de 4a montre que la somme des cibles des zones (1 282 338 o) dépasse 768 Ko de 495 906 o ; cinq causes mesurées couvrent l'excès, mais aucune n'est encore corrigée et l'ordre, le mécanisme, la sûreté et la remesure de chacune restent à fixer."
status: needs-triage
---

## Problem Statement

L'étape 4a du plan 7 est close (`f2beb9d`, décision humaine `36b6989`).
Le bilan de la section 2 dit, marge 1/8 comprise (règle 3) :

- la somme des cibles des quatre zones comptées par `ZONE_BUDGET_BYTES`
  est **1 282 338 o**, contre 786 432 : excès **495 906 o** ;
- cinq causes mesurées, toutes dans `frame` ou `game`, couvrent l'excès :
  somme des cibles après elles **743 962 o**, reste **42 470 o** ;
- la décision humaine : les e2e font foi, pas les tests unitaires qui
  figent une séquence ; 768 Ko est la cible fixe ; le cinquième candidat
  (flip de `game` dans `cart_init`) est retenu.

Ce qui manque pour lancer 4b :

- **L'ordre.** Le plan dit « le plus gros candidat d'abord », mais les
  gains de 4a sont calculés « pris l'un après l'autre » dans l'ordre 1-2-3-4,
  sans le cinquième ; le 3 ne rapporte rien sans le 2 ; le 4 et le 5 se
  chevauchent (le 5 retire du pic la nouvelle génération, dont une partie
  des blocs morts que le 4 visait).
- **Le mécanisme de chacun.** 4a nomme la cause et le gain ; elle laisse à
  4b le moyen (rendre, rétrécir, réserver, flipper). Pour les arènes de
  prise, le moyen n'est pas évident : un bloc pris dans une zone à deux
  bouts ne se rétrécit pas en place, et les arènes rendues sont recyclées
  d'une sorte d'objet à l'autre.
- **La sûreté du candidat 3.** Réduire la réserve de points de transit de
  64 à 24, c'est dimensionner sur un pic mesuré (21) ce qui est en fait une
  variable aléatoire. Il faut dire ce qui se passe quand un slipway en
  demande plus, et pourquoi ce n'est pas un `p8_fatal`.
- **La remesure.** Chaque commit doit noter le gain réel à côté du gain
  attendu ; il faut dire quoi mesurer, sur quel rejeu, avec quelle formule.

## Solution

Cinq commits, un par candidat, dans l'ordre **1, 2, 5, 3, 4** (gain
attendu décroissant, une fois les dépendances et les chevauchements
comptés — voir *Implementation Decisions*). Chaque commit :

1. commence par un test rouge quand le changement est observable
   (allocation ou séquence), puis le corrige (`tdd`) ;
2. garde **tous** les `tests/e2e/*.sha256` identiques — le jeu ne change
   pas ;
3. passe `make test-all`, `make sanitize-net` et `make full-game` ;
4. relance la campagne `make state` et écrit dans son message le pic et
   la frame mesurés, la cible recalculée, et le **gain réel à côté du gain
   attendu** ; la ligne du candidat dans la section 2 du plan reçoit le
   gain réel, dans le même commit.

Aucune limite de zone n'est retaillée (c'est 4c), `ZONE_BUDGET_BYTES` ne
bouge pas (c'est 4d).

## User Stories

1. En tant que pilote de 4b, je veux un ordre de commits justifié par le gain attendu de chaque candidat calculé dans cet ordre, pour faire le plus gros d'abord sans me tromper sur les chevauchements.
2. En tant que pilote de 4b, je veux savoir que le 3 vient après le 2, et pourquoi (gain et sûreté), pour ne pas poser une réserve plus petite dans une arène qui ne déborde pas.
3. En tant que pilote de 4b, je veux, par candidat, le mécanisme mesuré par 4a (appelant, octets, frame), pour ne pas refaire l'enquête gdb.
4. En tant que pilote de 4b, je veux, par candidat, le moyen retenu et l'alternative écartée, pour ne pas hésiter entre trois façons de faire.
5. En tant que pilote de 4b, je veux un test rouge par candidat quand le changement s'observe, pour que le TDD prouve la réduction avant la remesure.
6. En tant que pilote de 4b, je veux savoir quels tests unitaires figent l'ancienne séquence et peuvent être mis à jour, pour appliquer la décision humaine sans toucher un e2e.
7. En tant que pilote de 4b, je veux la formule exacte du gain réel (cible avant moins cible après, marge 1/8 arrondie à la cellule), pour que le gain réel se compare au gain attendu terme à terme.
8. En tant que pilote de 4b, je veux savoir quel rejeu et quelle frame tiennent le pic après chaque commit, pour voir quand le pic de `game` change de place (après le 5).
9. En tant que pilote du plan, je veux le reste attendu après les cinq (42 470 o), pour savoir si 4c peut démarrer ou s'il faut s'arrêter et l'écrire.
10. En tant que pilote du plan, je veux qu'un gain réel nettement sous le gain attendu soit signalé dans le message de commit avec le reste recalculé, pour décider tôt si la couverture tient.
11. En tant que pilote du plan, je veux que la règle 2 soit respectée : les candidats 1 et 4 sont des fuites qu'on corrige, pas des besoins qu'on loge.
12. En tant que pilote du plan, je veux que la règle 3 soit respectée : une réserve réduite est dimensionnée sur son pic plus 1/8, et son dépassement n'est jamais un `p8_fatal`.
13. En tant que joueur, je veux qu'une longue partie avec beaucoup de longs slipways à l'écran ne s'arrête pas sur « zone grab full », pour que la réduction de la réserve ne casse pas le jeu hors des rejeux mesurés.
14. En tant que joueur, je veux que le jeu se joue exactement comme avant (mêmes hashes e2e), pour que ces réductions soient invisibles.
15. En tant que développeur, je veux que les offres temporaires de la reconstruction des échanges meurent au retour de la fonction qui les fait, pour que la zone `frame` ne croisse plus avec le monde.
16. En tant que développeur, je veux que le contrat « ce que la fonction prend dans la zone frame est mort au retour » soit écrit dans l'en-tête du module, pour ne pas garder un pointeur vers une offre.
17. En tant que développeur, je veux une arène de prise de la taille de la construction de son objet, pour ne plus payer 2 048 o par slipway posé.
18. En tant que développeur, je veux qu'une arène dont l'objet est construit déborde dans la génération quand il grandit encore, pour qu'une arène exacte ne devienne pas un plafond.
19. En tant que développeur, je veux qu'un débordement pendant la construction reste un `p8_fatal` « zone grab full », pour qu'une taille de construction fausse se voie tout de suite dans les tests.
20. En tant que développeur, je veux qu'une arène rendue ne soit reprise que par une prise qui y tient, pour que le recyclage des prises refusées reste sans fuite.
21. En tant que développeur, je veux que l'écran titre ait sa génération de `game`, pour que l'ancienne partie soit rendue au vidage suivant au lieu de coexister avec la nouvelle.
22. En tant que développeur, je veux que les registres d'une partie réservent leur capacité au départ, pour que leur croissance ne laisse plus de blocs morts dans la génération.
23. En tant que développeur, je veux que ces réserves ne soient prises qu'au démarrage d'une partie, pas à l'écran titre, pour ne pas ajouter leur poids au pic du retour au titre.
24. En tant que développeur, je veux qu'une liste qui dépasse sa réserve grandisse comme avant (bloc mort, pas de `p8_fatal`), pour que la réserve soit une taille et pas un plafond.
25. En tant que développeur, je veux que `make test` et `make e2e` ne s'allongent pas, pour garder la boucle rapide.
26. En tant que relecteur, je veux retrouver dans chaque message de commit la commande de la remesure et les chiffres bruts, pour refaire le calcul.
27. En tant que relecteur, je veux que chaque commit ne porte qu'un candidat, pour qu'un `git revert` en retire un seul et que son gain se lise seul.
28. En tant que pilote de 4c, je veux que les tailles de zone ne bougent pas en 4b, pour retailler chaque zone une seule fois, sur les pics de fin de 4b.
29. En tant qu'agent sans fenêtre, je veux que toute la vérification passe par les rejeux sans fenêtre, `make state` et gdb en mode batch, pour ne jamais lancer l'appli raylib.
30. En tant que mainteneur futur, je veux l'argument de sûreté de la réserve de points de transit écrit dans le PRD et le commit, pour ne pas la « réparer » plus tard en la relevant à 64.

## Implementation Decisions

Les points où `to-prd` demande de vérifier avec l'utilisateur (découpage
en modules, modules à tester, besoin de lots) ont été tranchés sans
humain, en appliquant le skill `guideline` : simplicité d'abord,
changements chirurgicaux, hypothèses explicites, critère de succès
vérifiable. Chaque choix ci-dessous nomme l'alternative écartée.

### Ordre des commits et gains attendus

Gain attendu = baisse de la cible de la zone (pic × 9/8, arrondi à la
cellule de 8 o), chaque candidat pris **après les précédents dans cet
ordre**. Les chiffres viennent de 4a ; seuls ceux du 5 et du 4 sont
recalculés pour l'ordre retenu.

| # | Commit | Zone | Pic avant → après | Gain attendu |
|---|---|---|---|---|
| 1 | rendre les offres de `planet_rebuild_trades` | `frame` | 247 640 → 18 000 | **258 344** |
| 2 | arènes de prise à la taille de leur construction | `game` | 659 088 → 567 424 | **103 128** |
| 5 | flip de `game` dans `cart_init` | `game` | 567 424 → 491 584 | **85 320** |
| 3 | réserve de points de transit 64 → 24 | `game` | 491 584 → 430 144 | **69 120** |
| 4 | réserves des registres d'une partie | `game` | 430 144 → 410 176 | **22 464** |

- Fin de 4b attendue : `game` 410 176 (cible 461 448 + `load` 24 680),
  `frame` 18 000 (cible 20 256) ; somme des cibles 743 962, reste 42 470 —
  les chiffres de la section 2.
- **Pourquoi 5 avant 3 et 4.** Après le 2, le pic de `game` est encore deux
  générations (ancienne 491 584 + nouvelle 75 840) ; le 5 retire la
  nouvelle génération du pic (85 320), plus que le 3 (69 120).
- **Pourquoi le 4 ne vaut plus que 22 464** au lieu des 31 424 de 4a :
  4a comptait les blocs morts des deux générations (19 968 + 7 968) ;
  après le 5, la nouvelle génération n'est plus au pic, seuls les 19 968
  de l'ancienne comptent. Le 4 reste fait : c'est une fuite (règle 2), et
  sans lui le reste tomberait à 20 006.
- **Pourquoi 3 après 2** : sans le 2, chaque arène de prise garde 2 048 o
  quelle que soit la réserve, le 3 ne rapporte rien ; et sans le
  débordement introduit par le 2, la réserve à 24 rendrait possible un
  `p8_fatal` (voir le 3).
- **Si un gain réel manque.** Si, après un commit, le reste recalculé
  (786 432 moins la somme des cibles, en supposant les candidats suivants
  à leur gain attendu) devient négatif, on finit les cinq commits puis on
  s'arrête et on l'écrit dans le plan (dernière puce de l'étape 4) : ni
  sixième réduction, ni plafond relevé, ni marge rabotée.

### Commit 1 — `frame` : la reconstruction des échanges rend ce qu'elle prend

- **Mécanisme mesuré (4a).** Frame 6505 de `full_game.json`, au reset :
  `used` 247 640, dont 234 080 laissés par les appels de
  `planet_rebuild_trades` (les offres candidates : listes d'une planète et
  d'un slipway, `trade_new`, puis `trade_extend` dans `planet_offer_trade`
  et `slipgate_offer_trade`, toutes dans la zone `frame`). Les routes
  retenues sont recopiées dans `game` par la fermeture d'acceptation
  (`trade_copy` copie les deux listes) ; rien d'autre ne survit. Les appels
  sont faits par planète dans la reconstruction des routes entrantes
  (`economy_tiers`), et leur reste s'accumule jusqu'au reset. Fuite dans la
  frame (règle 2), qui croît avec le monde. Si chaque appel rendait sa
  croissance, le pic de `frame` sur les 29 rejeux tombe à **18 000**
  (frame 4149) ; les e2e montent au plus à 14 320.
- **Moyen retenu.** `planet_rebuild_trades` prend une marque de la zone
  `frame` en entrée et revient à cette marque avant de rendre la liste
  (marque et retour de `frame_zone.h`, antériorité : le calcul de la ligne
  de ressources dans `planet_display`, qui fait de même). L'en-tête du
  module dit le contrat : ce que l'appel prend dans la zone `frame` est
  mort à son retour ; la liste rendue et ses routes sont dans `game`.
- **Alternative écartée** : la marque dans l'appelant (`economy_tiers`),
  autour de la boucle des planètes. Elle marche aussi, mais le contrat
  « les routes retenues sont recopiées, le reste est jetable » appartient à
  `planet_rebuild_trades` ; le mettre ailleurs le cacherait.
- **Pièges à vérifier avant le vert** (le retour arrière est un angle mort
  d'ASan, que l'en-tête de `frame_zone.h` signale : un pointeur gardé lit
  le bloc suivant sans planter) :
  - aucun bloc de la zone `frame` pris pendant l'appel n'est encore
    référencé au retour — relire les deux seules méthodes d'offre
    (`planet_offer_trade`, `slipgate_offer_trade`) et la fermeture
    d'acceptation ;
  - aucune liste de la zone `frame` créée *avant* la marque ne grandit
    pendant l'appel (son nouveau bloc, pris après la marque, mourrait) ;
  - la liste rendue, créée dans `game` quand `outgoing_routes` manque,
    n'est pas touchée.
  Les hashes e2e et `trade_rich_mine_kept_route_survives_frame` attrapent
  le reste.
- **Test rouge.** Dans `test_live_slots`, sur `trade_rich_mine` à une frame
  où une reconstruction a lieu (f139, celle qui garde la route du slipway
  posé à f136) : `frame_zone_used()` est le même avant et après un appel de
  `planet_rebuild_trades` sur une planète reliée, et les routes rendues se
  relisent intactes. Rouge aujourd'hui (la zone grandit), vert après.
- **Remesure.** `make state` : pic de `zone frame` et rejeu qui le tient ;
  frame du pic par rejeu tronqué (méthode de 4a ci-dessous). Attendu :
  18 000, frame 4149, cible 20 256, gain 258 344.

### Commit 2 — `game` : chaque arène de prise à la taille de sa construction

- **Mécanisme mesuré (4a).** Ancienne génération à la frame 7642 de
  `full_game.json` : 216 arènes de prise de 2 048 o
  (`planet_grab_arena_open`, `PLANET_GRAB_ARENA_BYTES`) = 442 368, dont
  350 704 occupés (`arena_used`). 192 sont des slipways à 1 736 =
  en-tête d'arène 96 + `slipway_t` 1 104 + `pointer_list_t` 24 + réserve
  64 × 8 ; les 24 autres (structures posées, wormholes, sondes) tiennent
  17 392 à elles toutes. Une structure posée garde son arène jusqu'au
  changement de génération ; une arène d'objet refusé (ou de wormhole mort)
  va sur la liste des arènes libres et la prise suivante la vide et la
  reprend, quelle que soit sa sorte. Surdimensionnement : 91 664 o, gain
  attendu 103 128.
- **Pourquoi pas « rétrécir à la fermeture ».** Le bloc d'une arène vient
  de la zone `game`, à deux bouts ; une génération sur deux alloue par le
  haut, où le début du bloc (l'en-tête d'arène) est contre la frontière
  libre et la fin non : on ne rend pas la queue d'un bloc en place. Et
  entre la fermeture et la pose, d'autres blocs de `game` sont pris
  (enregistrement de l'entité), donc l'arène n'est plus le dernier bloc.
  Écarté aussi : recopier l'objet posé dans un bloc exact (tous les
  pointeurs vers lui et ses pointeurs internes, `header.layer =
  &self->layer`, seraient à refaire).
- **Moyen retenu, en trois parties :**
  1. **Taille demandée par l'ouvreur.** L'ouverture d'une arène de prise
     prend la taille de la construction qui va suivre : celle-ci est
     déterministe par sorte (`slipway_t` et sa réserve, `probe_t`, une
     structure selon le projet de son archétype, un wormhole selon le
     nombre d'ancres qu'il compte), donc calculable par l'ouvreur depuis
     des `sizeof` et ses données avant d'ouvrir. Un débordement **pendant**
     la construction reste le `p8_fatal` « zone grab full » d'aujourd'hui :
     une taille fausse se voit au premier test.
  2. **Débordement après la fermeture.** À la fermeture, l'arène passe en
     mode débordement : une allocation ou une croissance qui n'y tient
     plus est prise dans la zone d'où l'arène vient (la génération
     courante de `game`) au lieu d'un `p8_fatal`. C'est ce qui rend une
     arène exacte sûre : la liste des slipways d'une structure posée, la
     liste des points de transit d'un slipway, qui grandissent après la
     pose, débordent dans la génération, qui a la même durée de vie que
     l'arène d'un objet posé. Le vidage d'une arène recyclée remet le
     mode strict. Le débordement est une capacité du socle d'arènes
     (`engine/`), générique, sans planète : changer les pointeurs de
     fonction de la poignée suffit, **sans agrandir l'en-tête d'arène** —
     les places de `ui` sont taillées au plus juste et une cellule de plus
     par en-tête les casserait.
  3. **Recyclage par taille.** Une arène libre n'est reprise que par une
     prise dont la taille y tient (premier qui tient) ; sinon une nouvelle
     arène est prise. Les sortes sont peu nombreuses et de taille fixe
     (seul le wormhole varie), la liste libre reste sous
     `GRAB_ARENAS_FREE_MAX` ; l'`assert` existant le surveille.
- **Hypothèse à vérifier** (H-2-1) : un objet encore en pose ne grandit
  pas après la fermeture, donc une arène recyclée n'a jamais débordé et le
  recyclage ne laisse aucun bloc mort dans la génération. Les tests
  `refused_*_do_not_grow_game` de `test_live_slots` le prouvent déjà pour
  chaque sorte (une prise refusée qui déborderait ferait grandir `game`).
- **Repli plus simple, si la taille d'une sorte s'avère pénible à
  calculer** (structures, wormhole) : une taille exacte pour les seuls
  slipways et sondes, 2 048 inchangé pour les autres. Il perd environ
  31 760 o de pic (35 730 de cible) sur les 24 arènes non-slipway et ne
  laisse qu'environ 6 700 o de reste : à n'utiliser qu'en le disant dans
  le message de commit.
- **Tests rouges.**
  - Socle d'arènes (`test_allocator`, antériorité :
    `case_arena_alive_until_released`) : une arène en mode débordement,
    pleine, rend un bloc pris dans sa zone d'origine sans `p8_fatal` ; une
    croissance au-delà de sa fin recopie dans la zone d'origine ; en mode
    strict (avant la fermeture, ou après un vidage) le même dépassement
    reste un `p8_fatal` « zone <nom> full ».
  - Jeu (`test_slipway` ou `test_live_slots`) : l'arène d'un slipway neuf
    est pleine à la fermeture (`arena_used` = sa capacité, 1 736 avec la
    réserve 64) ; un slipway **posé** à qui l'on ajoute 65 points de
    transit ne s'arrête pas (aujourd'hui : « zone grab full », 312 o de
    marge pour une croissance de 1 024).
- **Tests qui figent l'ancienne séquence**, à mettre à jour (décision
  humaine) : ceux qui comptent `PLANET_GRAB_ARENA_BYTES` dans la
  croissance de `game` (`trade_rich_mine_route_rebuild_does_not_grow_game`
  de `test_live_slots`, un cas de `test_wormhole`) et ceux qui recyclent
  l'arène d'une sorte pour une autre
  (`placed_slipway_reuses_dead_wormhole_arena`,
  `placed_grab_reuses_dead_wormhole_arena`) : ils disent désormais ce qui
  est vrai (taille de la sorte ; arène reprise seulement si elle tient).
- **Remesure.** `make state` : pic de `zone game`, attendu 567 424 au
  démarrage de la nouvelle partie de `full_game_new_game` (frame 7690) ;
  gain attendu 103 128.

### Commit 5 — `game` : l'écran titre a sa génération

- **Mécanisme mesuré (4a).** `game` culmine quand une nouvelle partie
  démarre après une longue : l'ancienne génération (583 248 à cette
  frame) et la nouvelle (75 840) coexistent jusqu'au vidage de la frame
  suivante (frame 7690 de `full_game_new_game`, 7510 de
  `full_game_pause_restart`). Raison : `cart_init` (retour au titre,
  « restart » de la pause) appelle `reset_world` sans flipper, donc
  l'écran titre s'alloue dans l'ancienne génération, qui n'est rendue
  qu'au flip de `cart_start_game` puis au vidage suivant. Gdb (points
  d'arrêt sur `game_zone_flip`, `game_zone_release`, `cart_init`, lecture
  de `game_zone_storage.{bottom,top,pending}`) l'a montré.
- **Moyen retenu** (vérifié dans une copie hors dépôt par 4a : 26 hashes
  e2e gardés, ASan propre, pic 583 248 sans les autres candidats) :
  `cart_init` flippe la zone `game` avant `reset_world` quand aucune
  génération n'est déjà en attente, comme `cart_start_game` le fait
  déjà. L'ancienne partie est rendue au vidage de la file de sortie de la
  frame suivante, avant tout clic sur « nouvelle partie ». La garde
  « pas déjà en attente » évite l'`assert` de `flip_zone_flip` si
  `cart_init` suit un flip pas encore vidé.
- **Test rouge.** `pause_restart_keeps_game_across_restart` de
  `test_live_slots` fige l'ancienne séquence (« 'restart' ne flippe
  pas ») : il devient le contraire — après le clic « restart » (f11), une
  génération est en attente ; la sentinelle prise dans l'ancienne
  génération (f6) se relit encore à la frame du clic et meurt au vidage
  suivant (la paire `keeps_*_during_click` / `releases_*` du même fichier
  est l'antériorité, avec `in_child_dying_by_asan`). Rouge aujourd'hui,
  vert après. Tout autre test qui fige le non-flip (comptes de
  `pause_restart_counts_populate_galaxy_states`,
  `pause_restart_bounds_dead_registry_blocks`) est mis à jour sur la
  nouvelle séquence, jamais un e2e.
- **Remesure.** Le pic de `game` change de place : attendu 491 584 au
  retour au titre de `full_game.json` (frame 7643 ; ancienne génération
  plus l'écran titre, désormais dans sa propre génération), gain 85 320.
  Le message nomme le rejeu et la frame qui tiennent le nouveau pic.

### Commit 3 — `game` : réserve de points de transit d'un slipway, 64 → 24

- **Mécanisme mesuré (4a).** Chaque slipway posé réserve 64 pointeurs
  (512 o) dans son arène pour sa liste de points de transit
  (`SLIPWAY_TRANSIT_DOTS_RESERVE`) ; le plus grand nombre de points
  vivants d'un même slipway sur les 29 rejeux est **21**
  (`elements_transit_dots` de `make state`). Cible règle 3 : 21 × 9/8
  arrondi à la place = **24**. Gain : 40 × 8 = 320 o par slipway, 192
  slipways au pic, 61 440 o, cible 69 120. Après le 2, la taille de
  l'arène d'un slipway suit la réserve (calculée par l'ouvreur), donc le
  gain tombe sans autre changement.
- **Moyen retenu.** La constante passe à 24 ; rien d'autre.
- **Ce qui se passe quand un slipway veut plus de points que sa
  réserve.** Un point naît à chaque dessin d'un slipway à l'écran, dans
  chaque sens servi, avec une probabilité 0,08 ; il vit autant de frames
  que la longueur du slipway en pixels. Le nombre vivant suit donc à peu
  près une loi de Poisson de moyenne 2 × 0,08 × longueur, et la longueur
  est bornée par le test « trop loin » : distance carrée en coordonnées de
  carte au plus 0,5, ou 1,08 avec « space folding », soit environ 90 ou
  133 px. Moyenne au plus **14,5** sans repli, **21,3** avec. Le pic
  mesuré (21) est donc la moyenne d'un slipway de longueur maximale, pas
  un maximum : avec « space folding », un tel slipway à l'écran dépasse 24
  points environ un quart du temps, 48 avec une probabilité d'environ
  2 × 10⁻⁷ par frame.
  - Aujourd'hui (réserve 64, arène 2 048, 312 o libres après la
    construction) : le 65ᵉ point fait grandir la liste à 128 pointeurs
    (1 024 o) dans l'arène, qui ne les a pas : `p8_fatal` « zone grab
    full ». Probabilité de l'ordre de 10⁻¹⁴ : jamais vu.
  - Réserve 24 **sans** le 2 : la croissance à 48 (384 o) tiendrait dans
    les 632 o libres, celle à 96 (768 o) non : `p8_fatal` au 49ᵉ point,
    atteignable sur une longue partie. D'où **3 strictement après 2**.
  - Réserve 24 **avec** le 2 : la liste d'un slipway posé grandit en
    débordant dans la génération (`pointer_list` double sa capacité : 48,
    puis 96 si jamais), sans `p8_fatal` ; l'ancien bloc reste mort dans
    l'arène. Coût : 384 o de `game` une fois par slipway qui dépasse 24.
- **Argument de sûreté.** Après ce commit, la réserve est une **taille**,
  plus un **plafond** : la dépasser coûte de la mémoire de `game`, jamais
  un arrêt. Le risque restant est budgétaire : si les 192 slipways
  dépassaient tous 24 points, ils prendraient 73 728 o de `game`, plus que
  les 61 440 gagnés. Ce n'est pas le cas des parties mesurées (aucun
  slipway au-dessus de 21, donc aucun débordement dans la campagne), et la
  marge 1/8 de `game` (environ 51 Ko à la fin de 4b) absorbe une large part
  de slipways longs. Ce risque est écrit dans le message de commit et
  reste à remesurer sur une partie à slipways longs si elle entre dans
  `worst-case/` ; il ne justifie pas de garder 64 (règle 3 : pic plus
  marge).
- **Test rouge.** La liste de points de transit d'un slipway neuf a une
  capacité de 24, et son arène fait 1 416 o pleine à la fermeture (96 +
  1 104 + 24 + 24 × 8) — rouge à 64. Le test du 2 (65 points ajoutés à un
  slipway posé, pas d'arrêt) reste vert et couvre maintenant le
  dépassement de la réserve ; on y ajoute le cas du 25ᵉ point : la
  génération grandit de 384 o, pas d'arrêt.
- **Remesure.** Pic de `zone game` et `elements_transit_dots` (toujours
  21, limite affichée 24). Attendu 430 144, gain 69 120.

### Commit 4 — `game` : les registres d'une partie réservent leur capacité

- **Mécanisme mesuré (4a).** Les listes des registres (liste des vivants,
  cinq listes par message, une par étiquette) naissent vides dans la
  génération à chaque `reset_world` et grandissent en doublant ; une
  croissance dans la zone `game` recopie et laisse l'ancien bloc mort
  jusqu'au changement de génération. Ancienne génération à la frame 7642 :
  19 968 o de blocs morts, dont 18 208 pour `entity_register` (128 au
  retour au titre). Gdb : somme des `old_n` des croissances de
  `flip_zone_alloc` par premier appelant hors allocateur. Fuite au sens de
  la règle 2. (Les 7 968 de la nouvelle génération ne sont plus au pic
  après le 5.)
- **Moyen retenu.** Au démarrage d'une partie (`cart_start_game`), chaque
  liste de registre est créée avec une réserve égale au nombre d'entrées
  qu'elle atteint au pic, plus 1/8, arrondi à la place (règle 3). Les
  nombres se mesurent une fois (gdb à la frame 7642 de `full_game.json`,
  ou `count` de chaque liste) et vivent comme constantes nommées près des
  registres. Une liste qui dépasse sa réserve grandit comme aujourd'hui
  (bloc mort, pas d'arrêt) : la réserve est une taille, pas un plafond.
- **Pas à l'écran titre.** `cart_init` garde des registres vides : avec
  le 5, l'écran titre a sa génération, qui coexiste avec l'ancienne partie
  au retour au titre ; y réserver ~20 Ko de listes ajouterait leur poids
  au pic qu'on cherche à baisser.
- **Alternative écartée** : une croissance en place quand le bloc est le
  dernier de la génération. Générique, sans constante, mais elle ne joue
  presque jamais pour les registres (l'entité est allouée juste avant son
  enregistrement, donc la liste n'est pas le dernier bloc), et elle ne
  marche que par le bas de la zone.
- **Test rouge.** Après `cart_start_game`, la capacité de chaque liste de
  registre est sa réserve ; une partie qui reste sous ses réserves ne
  laisse aucun bloc mort de registre. `pause_restart_bounds_dead_registry_blocks`
  de `test_live_slots` (« les registres prennent 2C − 4 pointeurs ») fige
  la croissance par doublement : il est mis à jour sur les réserves.
- **Remesure.** Pic de `zone game`, attendu 410 176, gain 22 464 ; somme
  des cibles 743 962, reste 42 470.

### Méthode de remesure, commune aux cinq commits

- `make state` au commit : pics et limites dans `tmp/state.txt`, un état
  par rejeu sous `tmp/state/`. Les 29 rejeux doivent sortir `clean`.
- **Cible** d'une zone : pic × 9/8 arrondi au-dessus à 8 o ; pour `game`,
  cible de la génération plus cible de `load` (24 680, inchangée).
  **Gain réel** = cible avant le commit − cible après.
- **Frame d'un pic** : rejeu tronqué (frames numérotées comme `stopped
  p8_fatal frame <n>`, binaire de rejeu `-O0 -g` de `make test`) :

  ```
  jq "{frames: .frames[0:$n]}" <events> > t.json
  STATE=s.txt tmp/replay t.json /dev/null d.txt
  ```

  bissection sur `n`, le pic ne fait que monter avec `n`.
- **Attribution** si le gain réel s'écarte du gain attendu de plus de
  10 % : gdb en mode batch sur le même binaire, points d'arrêt sur
  `flip_zone_alloc` (somme par premier appelant hors allocateur) pour
  `game`, sur `bump_zone_alloc` avec les retours arrière pour `frame`.
  Les fichiers tronqués et scripts gdb restent hors du dépôt ; leur texte
  va dans le message de commit.
- **Message de commit** : candidat, gain attendu, pic et frame mesurés,
  cible, gain réel, reste recalculé ; tests figeant l'ancienne séquence
  mis à jour, s'il y en a.

## Implementation Chunks

Cinq lots, cinq commits, dans cet ordre. Chacun est fini quand son test
rouge est vert, que `make test-all`, `make sanitize-net` et
`make full-game` passent, que `git diff` ne touche aucun
`tests/e2e/*.sha256`, et que la campagne a donné son gain réel.

1. **Rendre la zone frame** (candidat 1) — `planet_rebuild_trades` revient
   à sa marque ; contrat dans l'en-tête ; test sur `trade_rich_mine` f139.
   Attendu 258 344.
2. **Arènes de prise exactes** (candidat 2) — débordement d'arène dans le
   socle (testé seul), taille passée par chaque ouvreur, recyclage par
   taille ; tests de séquence mis à jour. Attendu 103 128.
3. **Flip au titre** (candidat 5) — `cart_init` flippe avant
   `reset_world` ; `pause_restart_keeps_game_across_restart` inversé.
   Attendu 85 320.
4. **Réserve de points de transit à 24** (candidat 3) — dépend du lot 2
   (gain et sûreté). Attendu 69 120.
5. **Réserves des registres** (candidat 4) — au démarrage d'une partie
   seulement ; après le lot 3, parce que le lot 3 décide de ce qui coexiste
   au retour au titre. Attendu 22 464.

## Testing Decisions

- **Un bon test** regarde ce qu'un appelant observe : octets pris dans une
  zone, génération en attente ou non, arrêt ou non, contenu relu. Pas les
  champs privés d'un allocateur, sauf `arena_used` et la capacité d'une
  liste, qui sont l'objet même de la réduction.
- **Modules testés** : le socle d'arènes (débordement, mode strict) dans
  `test_allocator` ; la reconstruction des échanges, la séquence du titre,
  les arènes de prise et les registres dans `test_live_slots` (qui rejoue
  les e2e jusqu'à une frame et inspecte les zones) ; la construction d'un
  slipway dans `test_slipway`.
- **Antériorité** : `trade_rich_mine_kept_route_survives_frame` et
  `trade_rich_mine_route_rebuild_does_not_grow_game`,
  `refused_*_do_not_grow_game`, les paires
  `pause_restart_keeps_*_during_click` / `pause_restart_releases_*`
  (`in_child_dying_by_asan`), `case_arena_alive_until_released` ; le
  retour arrière de `planet_display`.
- **Tests qui figent une séquence** : mis à jour sur la nouvelle
  (décision humaine), chacun nommé dans le message du commit qui le
  change. Les e2e, jamais.
- **Durée** : aucun rejeu ajouté à `make test` ni à `make e2e` ; les
  nouveaux cas réutilisent les e2e déjà rejoués par `test_live_slots`.
  `make full-game` et `make state` restent hors de `make test-all`.
- **Invariants par commit** : `make test-all`, `make sanitize-net`,
  `make full-game` ; `tests/e2e/*.sha256` inchangés.

## Out of Scope

- Retailler une limite de zone ou un cap, file de sortie comprise (4c).
- `ZONE_BUDGET_BYTES` (4d).
- Toute réduction hors des cinq candidats, même si la remesure en montre
  une nouvelle : elle s'écrit dans le plan, elle ne se code pas en 4b.
- Changer la marge (1/8) ou le plafond (768 Ko).
- Plafonner les points de transit (ne plus en créer au-delà d'un nombre) :
  ce serait un écart avec le cart et changerait le rendu.
- Les transitoires de `ui`, `particles` et de la file de sortie au retour
  au titre : pas des candidats de 4a.
- Le budget de slipways et de relais ; les remesures sur MIPS (plan 11).
- L'audio.

## Further Notes

- **Chiffres d'origine** : section 2 du plan 7 et message de `f2beb9d`
  (méthode gdb, attributions, gains), décision `36b6989`.
- **Recalcul de l'ordre** : avec le 5 placé après le 2, le pic de `game`
  passe à max(ancienne génération 583 248 − 91 664, nouvelle 75 840) =
  491 584 ; la somme des gains (258 344 + 103 128 + 85 320 + 69 120 +
  22 464 = 538 376) égale celle de 4a (462 016 + 76 360) : l'ordre change
  la part de chacun, pas le total.
- **Les longueurs du slipway** de l'argument de sûreté (90 et 133 px)
  supposent `pos` = `map_pos` × 128, comme la souris de `slipway_drag` ;
  à relire si le lot 4 trouve autre chose. L'argument ne dépend que de
  l'ordre de grandeur : la moyenne dépasse 24 seulement si la longueur
  passe 150 px.
- **Point à porter à un humain** (sans bloquer 4b) : le débordement des
  points de transit est sûr mais pas gratuit ; une partie qui accumule des
  slipways longs à l'écran mangerait la marge de `game`. Si une telle
  partie entre un jour dans `worst-case/`, la campagne le verra.
