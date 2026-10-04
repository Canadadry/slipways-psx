# Plan 11 — Mesurer, optimiser, livrer

Brouillon, à passer en `plan-converge` avant toute exécution. Dernier des cinq plans vers un `.bin/.cue` PSX (plans 7 à 11).

## 1. Objectif

Un `.bin/.cue` final, où une partie complète se joue dans l'émulateur à la cadence du jeu, sans plantage mémoire.

## 2. Règles

- Le jeu ne change pas : tous les `*.sha256` de `tests/e2e/` restent identiques, `make test-all` passe à chaque commit.
- Le rejeu dans l'émulateur est une cible dédiée, hors de `make test-all`, qui ne doit pas ralentir.

## 3. Étapes

1. **Rejouer `full_game.json` dans l'émulateur** jusqu'au bout, avec mesure par frame : temps CPU, pic de chaque zone, taille des tampons de primitives. Si le plan 10 n'a pas livré le rejeu d'un `events.json` dans l'émulateur, c'est le premier travail.
2. **Remesurer les zones sur MIPS** : tailles de place des zones interface et particules (`button`, `message_box` sans marge sur x86-64).
3. **Optimiser seulement ce que la mesure désigne,** dans la marge libérée par le plan 7. La mesure désigne une frame qui dépasse 1/60 s ou une zone qui dépasse sa taille. Pistes notées, non engagées : la table `p8_quarter_wave` (64 Ko), les chaînes d'`assert`, le nombre de primitives par frame. Piste mémoire, non mesurée : remplacer les pointeurs stockés par des index 16 bits locaux au tableau dynamique d'où vient l'objet (la plupart des références pointent dans une liste ou une arène de taille bornée), au lieu d'adresses 32 bits.
4. **Livraison.** `.bin/.cue` final, et une partie complète jouée par un humain à la manette.

## 4. Critère de fin

L'humain finit une partie dans l'émulateur. Les écarts de rendu qu'il relève sont acceptés ou corrigés un par un ; un ralentissement ou un plantage renvoie à l'étape 1.
