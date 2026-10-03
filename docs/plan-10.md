# Plan 10 — Mesurer, optimiser, livrer

Brouillon, à passer en `plan-converge` avant toute exécution. Dernier des cinq plans vers un `.bin/.cue` PSX (plans 6 à 10).

## 1. Objectif

Un `.bin/.cue` final, où une partie complète se joue dans l'émulateur à la cadence du jeu, sans plantage mémoire.

## 2. Étapes

1. **Rejouer `full_game.json` dans l'émulateur** jusqu'au bout, avec mesure par frame : temps CPU, pic de chaque zone, taille des tampons de primitives.
2. **Remesurer les zones sur MIPS** : tailles de place des zones interface et particules (`button`, `message_box` sans marge sur x86-64).
3. **Optimiser seulement ce que la mesure désigne,** dans la marge libérée par le plan 6. Pistes notées, non engagées : la table `p8_quarter_wave` (64 Ko), les chaînes d'`assert`, le nombre de primitives par frame.
4. **Livraison.** `.bin/.cue` final, et une partie complète jouée par un humain à la manette.

## 3. Critère de fin

L'humain finit une partie dans l'émulateur. Les écarts de rendu qu'il relève sont acceptés ou corrigés un par un.
