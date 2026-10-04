# Plan 7 — Une partie pleine dans 768 Ko

Premier des cinq plans vers un `.bin/.cue` jouable sur émulateur PSX (plans 7 à 11).

## 1. Objectif

- `make state EVENTS=worst-case/full_game.json` va jusqu'à la dernière frame, sans `p8_fatal`.
- La somme des zones passe de 1,5 Mo à **768 Ko** (`ZONE_BUDGET_BYTES`).
- La moitié libérée est réservée aux tampons de primitives GPU du plan 10 et aux optimisations de fin sur PSX.

## 2. Point de départ mesuré

Campagne `make state` (étape 4a) : 29 rejeux, les 26 e2e et trois parties pleines de `worst-case/`, toutes verrouillées par `make full-game` : `full_game.json` (jusqu'à sa frame 7685, fin de partie puis retour au titre), `full_game_new_game.json` (la même, puis une nouvelle partie lancée du titre) et `full_game_pause_restart.json` (la même coupée à la frame 7500, puis pause, « restart » et nouvelle partie). Pic / limite en octets, tenu par `full_game.json` sauf mention :

| Zone | Pic | Limite |
|---|---|---|
| `game` | 659 088 (`full_game_new_game`) | 720 896 |
| `frame` | 247 640 | 262 144 |
| `ui` | 150 585 (`full_game_pause_restart`) | 342 422 |
| `particles` | 61 855 | 130 822 |
| `load` | 21 936 | 65 536 |

- `game` culmine quand une nouvelle partie démarre après une longue : 583 248 de l'ancienne génération plus 75 840 de la nouvelle, qui coexistent jusqu'au vidage de la frame suivante (frame 7690 de `full_game_new_game`, 7510 de `full_game_pause_restart`, même pic). Une génération seule tient 581 640 à la frame 7642 de `full_game.json` ; le retour au titre (frame 7643) y ajoute 1 608 sans changer de génération. `pause_restart` redémarre après 7 frames de jeu : 153 416 = 77 576 + 75 840 ; une génération seule culmine à 108 048 dans les e2e (`trade_rich_mine`).
- `frame` + pic de `game` dépassent déjà 768 Ko : la réduction de l'étape 4 est le chemin attendu, pas un repli.
- Caps : file de sortie 280 / 512 (`full_game_pause_restart` ; relevée de 256 à l'étape 1 : les caps saturent comme les zones) ; `button` 12 / 24 (`full_game_pause_restart`, comme `pause_restart`), `message_box` 3 / 6 et `toast` 42 / 128 (`game_end_bankrupt`) ; les octets des places `particles`, `particles_help_screen` et `ephemerals` sont à leur limite. Le détail est dans `tmp/state.txt` après `make state`.

Origine des gros pics (frames numérotées comme `stopped p8_fatal frame <n>`) :

- `game`, ancienne génération à la frame 7642 : 216 arènes de prise de 2 048 = 442 368, chacune portant un objet vivant, 350 704 occupés ; 192 sont des slipways à 1 736, dont une réserve de 64 points de transit (pic 21). Le reste — planètes, registres, 82 routes toutes tenues — est vivant, sauf 19 968 de blocs laissés morts par l'agrandissement des listes (18 208 pour `entity_register`, 128 au retour au titre). Besoin permanent, plus deux surdimensionnements (arènes, réserve) et une fuite au sens de la règle 2 (blocs morts). La nouvelle génération n'a aucune arène ; 7 968 de ses 75 840 sont des blocs morts. La coexistence est un transitoire de séquence (démarrage).
- `frame` à la frame 6505, au reset : 234 080 des 247 640 sont les offres temporaires de `planet_rebuild_trades` (reconstruction des échanges), tenues jusqu'au reset alors que les routes retenues sont recopiées dans `game`. Fuite dans la frame, qui croît avec le monde (159 frames au-dessus de 60 000, toutes à leur reset). Sans elle, le pic de `frame` sur les 29 rejeux tombe à 18 000 (frame 4149).
- `ui`, `particles` et la file de sortie au retour au titre (frame 7643 de `full_game.json`, 7505 de `full_game_pause_restart`) : le monde abandonné attend dans la file (278 ou 280 entrées) à côté de l'écran titre ; le vidage de la frame suivante les ramène à 25 181, 6 595 et 0. Transitoire de séquence (redémarrage).

Bilan contre 768 Ko (786 432), marge 1/8 (règle 3). `game` compte la génération et `load` ; `ui` et `particles` sont la somme de leurs places à la cible (compte et taille de place, pic plus marge ; la taille d'une liste de particules suit ses éléments) :

| Zone | Cible | Sans marge | Réservé aujourd'hui |
|---|---|---|---|
| `game` | 741 480 + 24 680 = 766 160 | 681 024 | 786 432 |
| `frame` | 278 600 | 247 640 | 262 144 |
| `ui` | 165 481 | 125 478 | 343 040 |
| `particles` | 72 097 | 59 193 | 131 072 |
| Somme | 1 282 338 | 1 113 335 | 1 522 688 |

- Excès : **495 906** avec marge, 326 903 sans.
- Cibles des caps (places × octets) : `button` 14 × 4 240, `modal_backdrop` 2 × 272, `message_box` 4 × 5 056, `toast` 48 × 336, `row` 71 × 952, `ephemerals` 6 × 248, `particle` 887, `particles` 18 × 1 384 (158 éléments), `particles_help_screen` 2 × 2 240 (265 éléments), `burst` 4 × 640, `transit_dots` 434 ; réserve de points de transit par slipway 24 ; file de sortie 315 (hors zones). Passent au-dessus de leur limite, relèvements où la fuite est à écarter d'abord : la génération de `game` (741 480 contre 720 896), `ephemerals` (248 octets contre 216), `particles_help_screen` (2 places contre 1).

Candidats de 4b, du plus gros au plus petit, gain attendu en octets de budget (baisse de la cible, marge comprise, pris l'un après l'autre) :

1. `frame` — rendre les offres temporaires de `planet_rebuild_trades` à son retour : **258 344** (pic 247 640 → 18 000). Fait : gain réel **258 344**, pic 18 000 tenu par `full_game.json` (frame 4149), cible 20 256.
2. `game` — arènes de prise à la taille de ce qu'elles tiennent : **103 128** (pic − 91 664). Fait : gain réel **99 640**, pic 570 520 tenu par `full_game_new_game.json` (frame 7690), cible 641 840 + 24 680 ; écart 3 488 : les arènes exactes des wormholes, dont le nombre d'ancres croît, restent sur la liste des arènes libres au lieu d'être reprises ; reste recalculé 38 982.
3. `game` — réserve de points de transit d'un slipway de 64 à 24, qui ne rapporte qu'avec 2 : **69 120** (192 × 320 au pic). Fait : gain réel **69 120**, pic 433 240 tenu par `full_game.json` (frame 7643, retour au titre), cible 487 400 + 24 680 ; aucun slipway de la campagne ne dépasse 21 points vivants, donc aucun débordement dans `game` ; reste recalculé 38 982.
4. `game` — blocs morts des agrandissements de listes, dans les deux générations : **31 424** (pic − 27 936). À mesurer : idem. Fait, réserves des registres prises par `cart_start_game` seul (pic d'une partie plus 1/8) : gain attendu après 5 et 3 **22 464**, gain réel **24 880**, pic 411 128 tenu par `full_game.json` (frame 7643, retour au titre), cible 462 520 + 24 680 ; les registres de l'ancienne génération passent de 36 928 (18 720 vivants en puissances de 2, 18 208 morts) à 14 816, sans bloc mort ; l'écart vient des réserves plus serrées que les capacités en puissances de 2 ; les blocs morts des listes hors registres restent ; reste recalculé 41 398.
5. `game` — flipper la zone `game` dans `cart_init` avant `reset_world`, comme `cart_start_game` le fait déjà : l'ancienne génération n'est plus tenue à côté de la nouvelle. Pic de `game` 659 088 → 583 248 ; mesuré dans une copie hors dépôt, 26 hashes e2e gardés, ASan propre. Gain attendu après 1 et 2 : **85 320**. Fait : gain réel **85 320**, pic 494 680 tenu par `full_game.json` (frame 7643, retour au titre : ancienne génération plus écran titre), cible 556 520 + 24 680 ; le titre quitté ne garde plus l'économie ni le panneau de score de la partie rendue ; reste recalculé 38 982.

Décision : les e2e font foi, pas les tests unitaires qui figent une séquence ; le plafond de 768 Ko est la cible et ne bouge pas. Les cinq candidats couvrent l'excès : somme des cibles 743 962, reste 42 470.

Fin de 4b : somme des cibles des zones **745 034** (`game` 462 520 + 24 680, `frame` 20 256, `ui` 165 481, `particles` 72 097), reste **41 398** sous 786 432.

## 3. Règles

- Le jeu ne change pas : tous les `*.sha256` de `tests/e2e/` restent identiques.
- Une saturation est d'abord lue comme une fuite (place jamais rendue, remise à zéro manquante), jamais comme un besoin. On ne relève une limite qu'une fois la fuite écartée.
- Chaque zone est dimensionnée sur son pic mesuré, plus une marge fixe, la même pour toutes : 1/8 du pic, arrondie au-dessus à la cellule de zone pour des octets, à la place entière pour un compte, caps compris.
- Invariant de chaque commit : `make test-all` et `make sanitize-net` passent ; dès l'étape 2, la cible verrou aussi.

## 4. Étapes

1. **Comprendre la saturation de `frame`.** Trouver ce qui s'accumule dans la zone à la frame 4337 (`help-me-debug`), puis le corriger. Fini quand `full_game.json` va au bout avec les tailles actuelles.
2. **Verrou.** `full_game.json` joué jusqu'au bout entre dans une cible dédiée, hors de `make test-all` qui ne doit pas ralentir, pour qu'une régression mémoire se voie dès le redimensionnement.
3. **Mesurer.** Le pic de chaque zone et de chaque cap, sur `full_game.json` complet et sur chaque e2e (`full_game.json` n'ouvre ni boîte de message ni fond modal), écrit dans `tmp/state.txt`.
4. **Redimensionner**, en sous-étapes :
   - 4a. **Bilan, sans code.** Depuis le tableau de l'étape 3 : pour chaque zone, pic plus marge contre 768 Ko, et l'excès en octets. Pour chaque gros pic (au moins `game`, `frame`, la file de sortie), la frame où il est atteint et s'il est un besoin permanent ou un transitoire de séquence (redémarrage, vidage) ; pour `game`, le pic d'une génération séparé de celui des deux qui coexistent. En sortent les candidats à réduire, du plus gros au plus petit, chacun avec son gain attendu.
   - 4b. **Une réduction par commit**, le plus gros candidat d'abord, remesurée par la campagne, gain réel noté à côté du gain attendu.
   - 4c. **Un commit par zone redimensionnée** à son pic plus la marge ; chaque cap, file de sortie comprise, revu de même contre son pic.
   - 4d. **`ZONE_BUDGET_BYTES` à 768 Ko en dernier**, une fois que la somme tient.
   - Si les candidats de 4a ne couvrent pas l'excès, s'arrêter et l'écrire : ni chaîne de réductions sans fin, ni plafond relevé sans décision humaine.

## 5. Hors périmètre

- Le budget de slipways et de relais (une évolution du jeu).
- Les remesures sur MIPS : plan 11.
