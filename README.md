# slipways-love

*Slipways* (édition PICO-8) porté en C, sur PC via raylib.

## Comment c'est fait

- Portage littéral, fonction par fonction, de `pico8/lua.lua` (la source décompilée du cart) ; les écarts connus sont dans `docs/trous-de-portage.md`.
- Le jeu parle à une API PICO-8 fixe (framebuffer 128×128 logiciel), servie par deux backends : un rejeu headless (`events.json` en entrée, dump texte du framebuffer en sortie) et raylib interactif.
- Une session raylib enregistre ses entrées en `events.json` : ce que l'humain a joué, l'agent le rejoue sans fenêtre.

## À faire

Règle de cette liste : chaque todo est une seule phrase claire, lisible sans référence externe, de 30 mots au plus.

- [ ] Sur le vrai PICO-8, vérifier trois comportements : l'affichage de -0.0001, la couleur d'un cercle tracé avec une couleur nil, et sin/cos entre les multiples de 1/64.
- [ ] Afficher un budget de slipways et de relais, et bloquer leur ajout quand il est plein.
- [ ] Ajouter une triche qui donne des points de score bonus, pour qu'un e2e court montre les étoiles allumées du panneau de score.
- [ ] Faire respecter la couverture de classes par `make e2e-cleanup`, qui propose de retirer `place_processor_ascension`, seul e2e à instancier le processeur et la porte d'ascension.
- [ ] Renommer les champs des structs du jeu qui portent encore un nom abrégé hérité du cart, uniquement quand on connaît leur sens, sans rien inventer.
- [ ] À la reprise PSX, décider si la manette émule la souris PICO-8 ou saute de planète en planète ; idées dans l'historique git de `docs/after-v1.md`.
