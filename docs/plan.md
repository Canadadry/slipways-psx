# Plan de développement — Slipways sur manette (GBA)

Objectif final : reproduire *Slipways* (édition PICO-8) intégralement jouable à la manette (layout GBA : D-pad, A, B, L, R, Start, Select — pas de X/Y, aucune souris, aucun stick analogique). Cible : portage natif en C sur Game Boy Advance (`devkitARM`/Tonc), scope intégral. Repli : Nintendo DS si un point dur matériel bloque le GBA — dans ce cas, le contrôle 100% boutons est à retrancher (l'écran tactile réintroduirait un pointeur, contraire à l'objectif ci-dessus), pas à garder par défaut.

Référence de gameplay : [`docs/design/slipways-mechanics.md`](design/slipways-mechanics.md) (règles extraites du code source PICO-8, `pico8/lua.lua`).

## Risque technique central — rendu des slipways sur GBA (à dérisquer avant tout le reste)

Le mécanisme visuel central de Slipways est un réseau de lignes à angle arbitraire entre planètes placées procéduralement (slipways, curseur du mode Sonde) — le GBA est une machine à tuiles/sprites sans primitive matérielle de ligne libre (la DS aurait un moteur 3D matériel réutilisable pour ça, voir repli ci-dessus). Ce risque conditionne la faisabilité même du scope intégral visé et doit être levé avant la Phase 1 : pas de planification détaillée d'une phase avant d'avoir dérisqué l'inconnue qui la précède.

### Phase 0 — POC de rendu

Techniques candidates, à tester **isolément** (pas de POC combiné en premier — on veut savoir précisément quel maillon serait le goulot d'étranglement) :

1. Mode 4 bitmap (240×160×8bpp, double buffer, tient dans les 96 Ko de VRAM disponibles) + rasterisation logicielle (Bresenham, `m4_line` de Tonc ou équivalent) — le plus flexible, coût CPU pur sur un ARM7 sans accélération.
2. Mode 3 bitmap (240×160×16bit vraies couleurs) — évite la palette 256 couleurs, mais pas de double buffer possible dans la VRAM disponible (risque de tearing).
3. Cache du réseau statique — les slipways confirmées changent rarement (construction seulement) ; les rasteriser une fois dans un buffer et ne redessiner à 60 fps que le dynamique (curseur, preview de liaison, effets de flux commercial).
4. Sprites affines matériels pour les segments — rotation/scale gratuits en hardware, mais partagent le budget de 128 sprites matériels avec planètes/curseur/effets (risque si le réseau devient dense, les connexions par planète colonisée étant illimitées dans les règles actuelles).
5. Hybride (3 + 4).
6. Culling par viewport — la caméra suit la sélection sur un écran 240×160 ; une partie du réseau d'une galaxie de 90 planètes n'est jamais visible simultanément.
7. Trigo en virgule fixe (pas de FPU sur l'ARM7TDMI) — indépendant de la technique de rendu retenue, nécessaire dans tous les cas pour les formules de distance/portée existantes.

**Critère de faisabilité** : 30 fps, testé contre une hypothèse ronde de **~30 planètes colonisées / 40-60 slipways simultanées**. Aucune donnée source sur ce qu'est une "partie moyenne" (le seul frein documenté est économique — frais généraux d'empire en `0.18 × n²`, coût de slipway croissant avec la taille du réseau — insuffisant pour en déduire un chiffre) : hypothèse documentée ici pour respecter la traçabilité des valeurs (cf. Garde-fous), à corriger empiriquement si un playtest de la cartouche originale la contredit.

**Méthode de mesure** : headless via le skill `mgba-env-setup` (Xvfb + `mgba-qt` + pont MCP `mcp-mgba`) sur la machine Linux dédiée — l'agent y tourne directement (pas sur la machine de dev Mac), ce qui permet une boucle d'itération automatisée sans aller-retour humain à chaque POC. La vérification visuelle humaine reste réservée à un checkpoint final une fois le jeu complet construit.

**Toolchain : Tonc** (`devkitARM`) — retenu pour sa documentation/communauté, la plus mature du panel envisagé (vs Butano, plus haut niveau mais masque le coût réel mesuré en Phase 0 ; vs libgba brut, sans le confort du tutoriel).

## Plan d'action

Provisoire au-delà de la Phase 1 : l'étape de rendu (Phase 1 étape 4) dépend directement du résultat de la Phase 0, donc le détail des phases suivantes sera ajusté une fois ce résultat connu.

### Phase 1 — Fondations GBA & boucle de jeu

1. Mettre en place le projet Tonc/devkitARM : `main.c` reste une pure colle (boucle principale synchronisée sur le VBlank via `VBlankIntrWait`, dispatch update/draw), aucune logique métier dedans — même principe de découpage que prévu pour Love2D (un fichier = une responsabilité).
2. Porter en C pur, compilable et testable côté host (gcc natif, sans dépendance GBA, même esprit que l'ancien `lib/t.lua`) : génération procédurale des 90 planètes (rejet si trop proches, seuil `0.05` normalisé — §3 de la doc de design) et le graphe de voisins, avec un test de connexité globale (BFS : tout spot atteignable depuis tout autre).
3. Lire les boutons via `REG_KEYINPUT`/macros Tonc (`key_hit`, `key_is_down`) — équivalent GBA-natif d'un module input, directement sur l'API finale (pas d'abstraction cross-runtime nécessaire ici, contrairement à l'ancien plan Lutro/LÖVE).
4. Implémenter la technique de rendu retenue à l'issue de la Phase 0 pour afficher planètes + wormhole (pas encore les slipways).

### Phase 2 — Input & navigation manette

5. Implémenter la state machine à 4 modes (cf. Design d'interaction manette) en C pur testable côté host, zéro appel graphique dedans.
6. Bootstrap : curseur de visée libre sans source, A pose le wormhole initial et l'insère dans le graphe de voisins (mécanisme d'insertion dynamique, réutilisé en Phase 4 pour les structures spéciales).
7. Navigation : le D-pad saute de spot en spot via le graphe de voisins (8 directions).
8. Sonde : curseur de visée libre en pixels, caméra qui suit le curseur, A lance la sonde.
9. Liaison : curseur multi-sauts comme en Navigation, validité recalculée en direct (branchée sur `liaison.c`, Phase 3 étape 12).
10. Select (menu tech) et Start (score courant) câblés en overlay vide/placeholder à ce stade — contenu réel en Phase 3.

### Phase 3 — Vertical slice

11. `rules_planets.c` : types de planètes + jobs de colonisation réduits.
12. `liaison.c` : calcul de validité des slipways (trop loin / bloqué par une planète / croise une route existante — formules du §5 de la doc de design) et câblage du mode Liaison (étape 9).
13. `rules_discovery.c` : coût (3 crédits, 3 mois), portée de sonde (seuil 0.19, étendu à 0.27 avec *space folding*), tirage planète vide (~45%) — RNG injecté pour tests déterministes.
14. `effects.c` : animation de flux commercial le long des liaisons actives.
15. `rules_tech.c` : menu tech minimal, accessible en permanence via Select.
16. `rules_economy.c` : formules réduites (commerce, maintenance, bonheur, score) et conditions de fin (banqueroute, limite de temps).

### Phase 4 — Clone complet

17. Étendre `rules_planets.c`/`rules_tech.c` à l'ensemble des 10 types de planètes, spécialisations et 20 technologies documentées.
18. Implémenter les structures spéciales (Slipgate, Infragate, Trading Hub, Synthesizer, Ascension Gate, Laboratory, Food processor) et les actions de transformation de planète (mechanize/terraform/collapse/harvest). Placement : réutilise le mécanisme d'insertion dynamique dans le graphe de voisins posé en Phase 2 étape 6.
19. Compléter `rules_economy.c` (formules complètes, niveaux de difficulté, score final) tels que documentés dans `slipways-mechanics.md`.

### Phase 5 — Graphismes, son & validation matérielle finale

20. Extraire les sprites et données audio (sfx/musique) de la cartouche PICO-8, remplacer les placeholders de rendu.
21. Porter les effets de "juice" recensés dans `pico8/lua.lua` (particules, flashs, secousses d'écran, animations de transition, pulsation de sélection, etc. — au-delà du flux commercial déjà couvert par `effects.c`).
22. Intégrer les effets sonores et la musique extraits.
23. Test headless via `mgba-env-setup` sur la machine Linux dédiée avec le jeu complet (sprites/son/juice inclus) : critère de succès = boot sans crash, boutons répondent, FPS cible (30, cf. Phase 0) tenu. Tout écart devient un point dur consigné, traité en Phase 6.

### Phase 6 — Résolution des points durs

Toute incompatibilité matérielle GBA découverte en cours de route (Phase 0 à 5) est consignée comme point dur, jamais traitée comme bloquante immédiatement.

24. Reprendre un par un les points durs consignés et les corriger, en conservant la fidélité aux règles (aucune formule/mécanique modifiée).
25. Relancer le test headless (`mgba-env-setup`) pour confirmer que les points durs sont résolus — ce test clôture le portage.

## Design d'interaction manette

4 modes cyclés par L/R : **Navigation → Sonde → Liaison → Panneau** (en boucle). Le cycle saute les modes indisponibles pour la sélection courante (le wormhole n'a ni Liaison ni Panneau — confirmé dans `pico8/lua.lua` : la classe `g_` du wormhole, lignes 708-726, ne porte pas le tag `"np"` que portent toutes les classes planète (`jo` ligne 385, `qh` 906, `hj` 999, `nr` 1102) et ne définit aucun `:ldown`, contrairement à `jo:ldown` (ligne 412) qui démarre un drag de slipway ou ouvre le menu de colonisation ; une fois posé, le wormhole est donc inerte au clic dans l'original). B revient directement en Navigation depuis n'importe quel mode. A confirme l'action du mode courant et **reste dans le même mode** (pour enchaîner plusieurs actions).

Deux boutons globaux, actifs depuis n'importe quel mode, sans mettre la simulation en pause (fidèle à l'original où consulter le score ou la tech ne bloque pas le temps) :
- **Select** : ouvre le menu technologies, overlay accessible en permanence comme dans l'original (« top-left corner »).
- **Start** : affiche le score courant à la demande (overlay, **sans pause** — équivalent du clic sur le cœur dans l'original).

- **Bootstrap** : au tout début de partie, rien n'est sélectionné — le jeu démarre dans un curseur de visée libre sans source ; A pose le wormhole initial, puis la state machine normale démarre.
- **Navigation** : le D-pad saute de spot en spot via le graphe de voisins (8 directions).
- **Sonde** : le D-pad déplace un curseur de visée libre en pixels depuis la sélection (planète ou wormhole) ; la caméra suit le curseur (fidèle au vestige `qb:ph`/`qb:md` du code source PICO-8). A lance la sonde vers la position visée.
- **Liaison** : le curseur se déplace librement dans toute la galaxie exactement comme en mode Navigation (mêmes sauts chaînés via le graphe de voisins, pas limité à un seul saut). La planète source reste celle sélectionnée à l'entrée du mode Liaison ; à chaque déplacement du curseur, la validité de la liaison source→curseur est recalculée en direct (valide / trop long / bloqué par une planète ou une route existante — formules du §5 de la doc de design) et affichée même quand elle est invalide (c'est le retour visuel qui explique pourquoi une cible ne convient pas). A construit la liaison seulement si elle est valide.
- **Panneau** : liste de constructions possibles (planète non bâtie) ou stats de ressources/satisfaction (planète bâtie), navigable au D-pad, A valide l'item sélectionné. Affiché en **bandeau fixe sur le bord de l'écran** (pas un menu flottant ancré sur la sélection) — évite qu'une planète en bord d'écran fasse sortir le menu du cadre.

Reste à trancher avant la Phase 2 : l'emplacement de pose des structures spéciales (§6 de la doc de design reste incertaine — « probablement » sur un point vide sélectionné, non confirmé par le code) et la table complète de mapping bouton × mode croisant **type de sélection ET état de sélection** (caché / révélé non colonisé / colonisé) — ex. Sonde n'a de sens que sur du caché, Liaison/Panneau que sur du colonisé (§3-§5 de la doc de design).

### Algorithme de voisinage (8 directions)

Invention propre au portage manette (absente du PICO-8, où le clic vise directement un point) : pour permettre au D-pad de sauter de planète en planète malgré une disposition procédurale (pas une grille), chaque spot précalcule son meilleur voisin dans chacune des 8 directions cardinales (E/SE/S/SW/W/NW/N/NE, angles 0/45/90/135/180/225/270/315).

Pour un spot donné et une direction donnée : parmi tous les autres spots dont l'écart angulaire à la direction est inférieur à 90°, on choisit celui qui minimise `distance × (1 + 2 × écart_angulaire / 180)` — une pénalité qui privilégie un spot bien aligné sur l'axe à un spot plus proche mais excentré, sans l'exclure totalement (facteur de pénalité `2`, cône de tolérance `90°`, ces deux constantes étant elles-mêmes une invention d'interaction, pas une valeur à chercher dans `pico8/lua.lua`). Ce calcul est fait une fois à la génération (et recalculé pour les spots affectés lors d'une insertion dynamique — wormhole, structures spéciales), pas à chaque frame.

## Garde-fous

Règles à relire à chaque nouvelle étape de code, pas seulement au démarrage du projet :

- **Couche interaction vs contenu** : le curseur D-pad, le cycle de modes L/R et la confirmation A sont des ajouts tolérés — ils compensent l'absence de souris, rien de plus. Ils ne doivent jamais introduire une capacité de jeu (ressource, action, règle) absente du PICO-8. En cas de doute sur si un ajout relève de l'interaction ou du contenu, il est traité comme du contenu par défaut (donc interdit) et doit être justifié explicitement avant d'être codé.
- **Traçabilité des valeurs** : toute constante (coût, seuil, formule) codée doit être retrouvable dans `slipways-mechanics.md`. Une valeur manquante ou ambiguë se documente comme "Incertain" dans la doc et se résout en relisant `pico8/lua.lua` — jamais par une valeur "raisonnable" inventée.
- **Pas de contenu retiré silencieusement** : toute règle listée dans la doc de design doit apparaître quelque part dans le plan d'action (Phase 3/4). Si une règle semble non portée à la relecture d'une phase, c'est un oubli à corriger, pas une simplification à assumer.
- **Pas de juice inventé** : les seuls effets visuels/sonores ajoutés en Phase 5 sont ceux recensés dans `pico8/lua.lua`. Un effet qui "améliorerait" le ressenti sans exister dans la source n'est pas ajouté.
- **Le mapping bouton × mode fait foi** (voir Design d'interaction manette) : toute touche non listée ne doit déclencher aucune action.
- **Points durs, jamais de points bloquants** : une incompatibilité GBA découverte en cours de route (Phase 0 à 5) est consignée puis traitée en Phase 6 — elle ne justifie jamais de changer une règle de jeu pour "faire marcher" le portage, ni de rouvrir une phase déjà validée.

## Décisions actées

- Pas de carte de tech façon PC : le tech tree est un menu accessible à tout moment (fidèle au PICO-8).
- Pas de changement du modèle de temps : continu avec tick annuel, comme l'original.
- Toolchain : Tonc (`devkitARM`) — cf. Phase 0.
- Sprites et audio extraits tels quels de la cartouche PICO-8, effets de juice reproduits d'après la logique de la source (`pico8/lua.lua`) — aucun nouvel art ni effet inventé.
