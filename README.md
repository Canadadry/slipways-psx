# slipways-love

*Slipways* (édition PICO-8) porté en C, sur PC via raylib.

## Comment c'est fait

- Porté depuis `pico8/lua.lua` (la source décompilée du cart), qui reste en archive : le C est désormais la référence ; les écarts connus sont dans `docs/trous-de-portage.md`.
- Le jeu parle à une API PICO-8 fixe (framebuffer 160×120 logiciel), servie par deux backends : un rejeu headless (`events.json` en entrée, dump texte du framebuffer en sortie) et raylib interactif.
- Une session raylib enregistre ses entrées en `events.json` : ce que l'humain a joué, l'agent le rejoue sans fenêtre.
- `worst-case/full_game.json` est une partie remplie (50 planètes, 192 slipways) qui mesure le pire cas mémoire : `make state EVENTS=worst-case/full_game.json` écrit le pic et la capacité de chaque zone dans `tmp/state.txt`, `make state` sans `EVENTS` y écrit le plus haut pic de chaque zone et de chaque cap sur cette partie et sur tous les e2e, avec sa limite et le rejeu qui le tient, et `make full-game` (hors de `make test-all`, lancé par la CI) échoue si un `p8_fatal` l'interrompt avant sa dernière frame, ou interrompt l'une de ses trois variantes, `full_game_new_game.json` (une nouvelle partie lancée après elle), `full_game_pause_restart.json` (coupée par pause, « restart » et nouvelle partie) et `full_game_pause_idle.json` (coupée par un menu de pause laissé ouvert 300 frames), ou l'écran titre laissé au repos 300 frames (`title_idle.json`), que `make state` rejoue aussi.

## À faire

Règle de cette liste : chaque todo est une seule phrase claire, lisible sans référence externe, de 30 mots au plus.

- [ ] Sur le vrai PICO-8, vérifier trois comportements : l'affichage de -0.0001, la couleur d'un cercle tracé avec une couleur nil, et sin/cos entre les multiples de 1/64.
- [ ] Afficher un budget de slipways et de relais, et bloquer leur ajout quand il est plein.
- [ ] Ajouter une triche qui donne des points de score bonus, pour qu'un e2e court montre les étoiles allumées du panneau de score.
- [ ] Avant tout nouveau point de libération, rendre mécanique la liste des références coupées par `forget` (`src/game/cart.c`) : l'inventaire par grep en a manqué trois (la difficulté retenue, `slipway->end`, `targeted_by`).
- [ ] Sur PC, avant le portage PSX, ne recalculer que les routes commerciales touchées par un changement, au lieu de tout reconstruire à chaque recalcul de l'économie.
- [ ] Décider de GAP-26 : une planète hors écran perd le halo de l'objet tenu rendu, là où la cartouche relirait sa mémoire.
