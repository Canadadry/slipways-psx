# Plan de développement — Slipways sur manette (PSX)

Objectif final : reproduire *Slipways* (édition PICO-8) intégralement jouable à la manette (manette PS1 standard : D-pad, ✕/○/□/△, L1/L2/R1/R2, Start/Select — pas de stick analogique utilisé, aucune souris). Cible : portage natif en C sur PlayStation 1 (`PSn00bSDK`), scope intégral.

Référence de gameplay : [`docs/design/slipways-mechanics.md`](design/slipways-mechanics.md) (règles extraites du code source PICO-8, `pico8/lua.lua`).

Idées d'amélioration hors scope v1 : [`docs/after-v1.md`](after-v1.md).

## Rendu des slipways sur PSX

Le GPU de la PSX dessine nativement des primitives ligne (monochrome et gouraud-shaded) et polygone via des commandes GP0 dédiées, avec un GTE matériel pour les transformations — de l'ordre de 360 000 polygones flat-shaded/s en régime nominal, très au-delà du besoin d'un réseau de quelques dizaines de segments. Le rendu du réseau de slipways se fait directement avec les primitives GPU natives (`LINE_F2`/`LINE_G2`/polylignes via PSn00bSDK).

Point qui reste à vérifier empiriquement avant de considérer le rendu acquis : le budget CPU du calcul (pas du dessin) — génération procédurale, recalcul de validité de liaison en direct, trigonométrie. Le R3000A de la PSX n'a pas de FPU matérielle (le GTE accélère les transformations matricielles mais pas la trigonométrie générale) : la trigo en virgule fixe reste nécessaire et se valide en Phase 1.

**Critère de faisabilité** (hypothèse documentée faute de donnée source sur ce qu'est une "partie moyenne") : 30 fps face à ~30 planètes colonisées / 40-60 slipways simultanées.

**Toolchain : PSn00bSDK** — SDK PS1 open source (GCC MIPS, libs GPU/GTE/pad/DMA, outils de build). Alternative écartée : SDK officiel Sony, non redistribuable librement.

## Prérequis méthodologique — harnais de test headless

Outil retenu : [PCSX-Redux](https://pcsx-redux.consoledev.net/Lua/introduction/) (`make pcsx-redux-setup` build PCSX-Redux + OpenBIOS depuis les sources — voir `docs/psx-hello-world.md`). Phase 0 close : `make psx-hello-input-test-pcsx` boote `psx-hello.cue` headless (`--no-ui`, BIOS légal OpenBIOS), pilote Croix/D-pad/Cercle via le pont Lua/LuaJIT de PCSX-Redux (`PCSX.SIO0.slots[...].pads[...].setOverride`/`clearOverride`), lit l'état réel en RAM invité (`PCSX.getMemPtr()`) et sort avec un code pass/fail réel (`PCSX.quit(code)`) — plus de grep sur stdout. `make psx-hello-boot-test-pcsx` reste un smoke-test boot-only rapide (log-grep, sans assertions).

Alternative pour des vérifications visuelles ponctuelles en cours de développement : Mednafen (`make psx-hello-boot-test`), nécessite un vrai BIOS Sony fourni localement par le développeur (jamais committé) — voir `docs/psx-hello-world.md`.

**Étape 0 du plan PSX, avant toute logique de jeu** : faite. Reste, hors scope Phase 0 : étendre ce harnais à la logique de jeu réelle au fur et à mesure des phases suivantes (Phase 1 étape 4 notamment).

## Plan d'action

### Phase 0 — Harnais de test headless

0. **Fait.** Pont Lua par-dessus PCSX-Redux : build (`make psx-hello`), boot headless (`make pcsx-redux-setup`), lecture d'état et injection d'input depuis l'agent (`make psx-hello-input-test-pcsx`). Boucle build → boot → assertion automatisée validée répétable, y compris sa capacité à détecter une vraie régression (voir `docs/psx-hello-world.md`).

### Phase 1 — Fondations PSX & boucle de jeu

1. Mettre en place le projet PSn00bSDK : `main.c` reste une pure colle (boucle principale synchronisée sur le VSync, dispatch update/draw), aucune logique métier dedans — même principe de découpage que prévu pour Love2D (un fichier = une responsabilité).
2. Porter en C pur, compilable et testable côté host (gcc natif, sans dépendance PSX, même esprit que l'ancien `lib/t.lua`) : génération procédurale des 90 planètes (rejet si trop proches, seuil `0.05` normalisé — §3 de la doc de design) et le graphe de voisins, avec un test de connexité par BFS sur ~2000 seeds, borné à un taux de déconnexion attendu (la source PICO-8 elle-même ne garantit pas 100% de connexité — voir `docs/prd/triage/psx-planet-generation.md`).
3. Lire la manette via l'API pad de PSn00bSDK (lecture directe, pas d'abstraction cross-runtime nécessaire ici).
4. Afficher planètes + wormhole + segments de slipway via les primitives GPU natives (sprite/polygone pour les planètes, `LINE_F2`/`LINE_G2` pour les slipways) — mesurer contre le critère de faisabilité (30 fps, ~40-60 slipways) via le harnais de la Phase 0.

### Phase 2 — Input & navigation manette (v1 simple, volontairement provisoire)

Décision actée pour cette v1 : un seul mode de curseur, pas de state machine de modes. Priorité à la proximité avec le code source PICO-8 (curseur + clic, comme la souris de l'original), pour pouvoir reprendre le plus directement possible la logique de `pico8/lua.lua` (les handlers `:ldown` etc. opèrent déjà sur une position de curseur + un événement clic, indépendamment de tout "mode"). Idées de mapping plus élaboré (mode saut, boutons dédiés par panneau) : voir `docs/after-v1.md`, hors scope v1.

5. **✕** active/désactive le mode souris (bascule on/off) : mode actif, le D-pad déplace un curseur libre en pixels (caméra qui suit) — seul mode de curseur en v1.
6. **○** simule un clic à la position courante du curseur (mode souris actif) — c'est cet événement qui doit driver directement la logique déjà présente dans `pico8/lua.lua` (pose de wormhole, colonisation, lancement de sonde, tracé de liaison), sans réinventer une state machine de jeu par-dessus.
7. Select (menu tech) et Start (score courant) : mapping 1:1, disponibles nativement sur la manette PSX, aucune adaptation nécessaire.

**Reste à trancher avant d'aller plus loin sur cette phase** :
- Le clic PICO-8 a une sémantique pression/relâchement (`:ldown`, et vraisemblablement un relâchement pour terminer un drag de slipway). Est-ce que **○** doit générer un événement pression ET un événement relâchement (bouton maintenu = drag en cours), ou seulement un clic instantané ? Non tranché — à vérifier contre `pico8/lua.lua` avant de coder l'étape 6.

### Phase 3 — Vertical slice

8. `rules_planets.c` : types de planètes + jobs de colonisation réduits.
9. `liaison.c` : calcul de validité des slipways (trop loin / bloqué par une planète / croise une route existante — formules du §5 de la doc de design), branché sur l'événement clic de la Phase 2.
10. `rules_discovery.c` : coût (3 crédits, 3 mois), portée de sonde (seuil 0.19, étendu à 0.27 avec *space folding*), tirage planète vide (~45%) — RNG injecté pour tests déterministes.
11. `effects.c` : animation de flux commercial le long des liaisons actives.
12. `rules_tech.c` : menu tech minimal, accessible en permanence via Select.
13. `rules_economy.c` : formules réduites (commerce, maintenance, bonheur, score) et conditions de fin (banqueroute, limite de temps).

### Phase 4 — Clone complet

14. Étendre `rules_planets.c`/`rules_tech.c` à l'ensemble des 10 types de planètes, spécialisations et 20 technologies documentées.
15. Implémenter les structures spéciales (Slipgate, Infragate, Trading Hub, Synthesizer, Ascension Gate, Laboratory, Food processor) et les actions de transformation de planète (mechanize/terraform/collapse/harvest). Placement : réutilise le mécanisme d'insertion dynamique dans le graphe de voisins (wormhole, Phase 1-2).
16. Compléter `rules_economy.c` (formules complètes, niveaux de difficulté, score final) tels que documentés dans `slipways-mechanics.md`.

### Phase 5 — Graphismes, son & validation matérielle finale

17. Extraire les sprites et données audio (sfx/musique) de la cartouche PICO-8, remplacer les placeholders de rendu.
18. Porter les effets de "juice" recensés dans `pico8/lua.lua` (particules, flashs, secousses d'écran, animations de transition, pulsation de sélection, etc. — au-delà du flux commercial déjà couvert par `effects.c`).
19. Intégrer les effets sonores et la musique extraits (SPU de la PSX).
20. Test headless via le harnais de la Phase 0 avec le jeu complet (sprites/son/juice inclus) : critère de succès = boot sans crash, manette répond, FPS cible (30, cf. section rendu) tenu. Tout écart devient un point dur consigné, traité en Phase 6.

### Phase 6 — Résolution des points durs

Toute incompatibilité matérielle PSX découverte en cours de route (Phase 0 à 5) est consignée comme point dur, jamais traitée comme bloquante immédiatement.

21. Reprendre un par un les points durs consignés et les corriger, en conservant la fidélité aux règles (aucune formule/mécanique modifiée).
22. Relancer le test headless (Phase 0) pour confirmer que les points durs sont résolus — ce test clôture le portage.

## Garde-fous

Règles à relire à chaque nouvelle étape de code, pas seulement au démarrage du projet :

- **Couche interaction vs contenu** : le curseur en mode souris, le bouton ✕ de bascule et le clic simulé par ○ sont des ajouts tolérés — ils compensent l'absence de souris, rien de plus. Ils ne doivent jamais introduire une capacité de jeu (ressource, action, règle) absente du PICO-8. En cas de doute sur si un ajout relève de l'interaction ou du contenu, il est traité comme du contenu par défaut (donc interdit) et doit être justifié explicitement avant d'être codé.
- **Traçabilité des valeurs** : toute constante (coût, seuil, formule) codée doit être retrouvable dans `slipways-mechanics.md`. Une valeur manquante ou ambiguë se documente comme "Incertain" dans la doc et se résout en relisant `pico8/lua.lua` — jamais par une valeur "raisonnable" inventée.
- **Pas de contenu retiré silencieusement** : toute règle listée dans la doc de design doit apparaître quelque part dans le plan d'action (Phase 3/4). Si une règle semble non portée à la relecture d'une phase, c'est un oubli à corriger, pas une simplification à assumer.
- **Pas de juice inventé** : les seuls effets visuels/sonores ajoutés en Phase 5 sont ceux recensés dans `pico8/lua.lua`. Un effet qui "améliorerait" le ressenti sans exister dans la source n'est pas ajouté.
- **Points durs, jamais de points bloquants** : une incompatibilité PSX découverte en cours de route (Phase 0 à 5) est consignée puis traitée en Phase 6 — elle ne justifie jamais de changer une règle de jeu pour "faire marcher" le portage, ni de rouvrir une phase déjà validée.

## Décisions actées

- Cible de portage : PSX (`PSn00bSDK`).
- Interaction manette v1 : un seul mode curseur, pas de state machine de modes — mode souris activé par ✕ (D-pad déplace le curseur) + clic simulé (○), délibérément proche du modèle souris de PICO-8. Autres idées de mapping : `docs/after-v1.md`, hors scope v1.
- Pas de carte de tech façon PC : le tech tree est un menu accessible à tout moment (fidèle au PICO-8).
- Pas de changement du modèle de temps : continu avec tick annuel, comme l'original.
- Sprites et audio extraits tels quels de la cartouche PICO-8, effets de juice reproduits d'après la logique de la source (`pico8/lua.lua`) — aucun nouvel art ni effet inventé.
