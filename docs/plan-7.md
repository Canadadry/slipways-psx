# Plan 7 — Jouer sans souris, au clavier

Brouillon, à passer en `plan-converge` avant toute exécution. Deuxième des cinq plans vers un `.bin/.cue` PSX (plans 6 à 10).

## 1. Objectif

On joue une partie complète sur PC **au clavier seul** : menus, colonisation, slipways, recherche, fin de partie. Le clavier tient lieu de manette PSX : six boutons PICO-8 (quatre directions, O, X), plus au besoin les boutons de tranche de la manette.

La souris reste branchée : les e2e existants restent à la souris et ne changent pas.

## 2. Contraintes déjà dans le code

- Les flèches font défiler la carte (`src/game/scroll.c`).
- La triche lit des séquences O/X (`src/game/cheat.c`).
- Le jeu ne lit la souris que via `p8_stat(32/33/34)` (`src/game/mouse.c`, `mouse_button.c`).

## 3. Approche

Une couche du cœur, sans raylib, qui traduit les boutons en curseur et en clic :
- un mode curseur, où les directions déplacent un curseur et un bouton clique ;
- le défilement de carte garde un accès, par exemple quand le curseur pousse le bord de l'écran ;
- la triche reste atteignable.

La souris réelle, quand elle bouge, reprend la main sur le curseur clavier.

## 4. Étapes

1. **Décider le mapping** : quelle touche clique, comment on défile, comment on reste compatible avec la triche. Une décision humaine, à écrire ici avant tout code.
2. **Couche boutons → souris** dans le cœur, testée en unitaire.
3. **Branchement raylib** des touches (le backend lit déjà les flèches, Z et X).
4. **e2e clavier** : au moins un rejeu court qui colonise et trace une slipway sans aucun mouvement de souris.
5. **Essai humain** : une partie jouée au clavier sur PC. Si c'est injouable, retour à l'étape 1.

## 5. Décisions actées

- Le clavier suffit : il représente la manette, pas besoin de manette physique sur PC.
- Le mode saut de planète en planète est **hors plan**.
