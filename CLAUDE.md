# Notes de projet

## Le portage

- Cible active : **PC via raylib, rendu logiciel** — voir le `README`. Le C est la référence : le jeu est couvert par les e2e, et `pico8/lua.lua` (la source décompilée du cart) reste comme archive. Les objets du jeu sont des structs C plates : un noyau commun (`pico8_struct_header_t`), un archétype `static const` par sorte, des composants par pointeur.
- PSX est **en pause**, pas abandonné : on y revient quand une version PC/C est jouable de bout en bout. `psx-hello/` est un projet de test du toolchain — voir `docs/psx-hello-world.md`.
- L'audio (`sfx`, `music`) est **hors périmètre**, pas reporté.

## Ce qu'on ne fait pas

- **Ne jamais lancer l'appli raylib.** Pas de fenêtre, pas de serveur X : un agent n'a ni yeux ni souris, il ne verrait rien de ce qu'il ouvre. La vérification se fait sans fenêtre — N frames pilotées par un `events.json` scripté, puis dump texte du framebuffer. Jouer le jeu, c'est le travail d'un humain.

## Build

- `make test` est la boucle rapide (TU et contrôles statiques, quelques secondes). `make e2e` rejoue les e2e de `tests/e2e/` depuis l'écran titre (~1 s). `make test-all` fait les deux. La légende des cibles est en tête du `Makefile`.
- `make raylib` compile et linke le binaire interactif sans jamais l'exécuter ; il faut `make raylib-vendor-setup` d'abord — voir `docs/raylib-vendoring.md`.

## Docs

`docs/` ne décrit que ce qui est vrai ou à faire. Ce qui est fini — plan, étude, PRD implémentée — est supprimé : l'historique, c'est git.

## Commentaires

Un commentaire ne reste que s'il porte l'une de ces trois choses : un écart connu avec le cart (→ `docs/trous-de-portage.md`, référencé par un `/* GAP-7 */` nu), un piège qu'on réintroduirait (un `assert` porteur de message, sinon 40 mots), ou un contrat C que le nom ne dit pas (qui libère la mémoire, ce que rend la fonction si le champ manque). `TODO` reste autorisé. Dans un `Makefile`, seule l'aide à la lecture d'une commande reste. Tout le reste va dans le message de commit.

Pour reprendre un fichier déjà écrit : skill `reprendre-les-commentaires`.
