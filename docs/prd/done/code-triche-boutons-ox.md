---
title: "Boutons O/X et code de triche A A B B A A B B"
description: "Ajouter les boutons PICO-8 O et X au modèle d'entrée, et un code de triche rejouable qui donne 99 de science et 200 crédits, pour permettre d'enregistrer une session qui construit toutes les planètes spéciales."
status: done
---

## Problem Statement

Le filet `frames.hash` ne prouve rien sur le code qu'aucun enregistrement n'exécute. Mesure faite sur les 12 rejeux du filet : les sept planètes spéciales (`qh` exceptée, construite 4 fois) ne sont **jamais instanciées** — `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx` à zéro. Le plan 3 (`docs/plan-3.md`) convertit ces classes en structs ; sans couverture, une erreur de conversion passerait le filet en silence.

Les construire à la main dans une partie normale est long : cinq d'entre elles attendent une technologie inventée (`oj(self.ke.rr)`, `lua.lua:926`), donc de la science (`fm.jk`), et toutes coûtent des crédits (`fm.nz`). L'humain qui enregistre a besoin d'un raccourci.

Contrainte dure : le raccourci doit être **rejouable**. Le rejeu headless ne lit que `events.json` ; une triche appliquée hors du flux d'entrée ferait diverger le rejeu de la session enregistrée.

## Solution

1. Ajouter au modèle d'entrée les deux boutons PICO-8 que le cart n'utilise pas, O et X (`btn(4)`, `btn(5)`). Backend raylib : Z et X au clavier ; une manette pourra les mapper plus tard (A/B, ✕/○ sur PSX). `events.json` passe de 4 à 6 booléens par frame.
2. Détecter, à partir de ces seules entrées, la séquence **A A B B A A B B** (A = O, B = X). Quand elle aboutit : `fm.jk = 99`, `fm.nz = 200`.

La triche étant dérivée du flux d'entrée, elle est rejouée à l'identique sans champ supplémentaire dans `events.json`.

## User Stories

1. En tant qu'humain qui enregistre, je veux taper A A B B A A B B pour obtenir 99 de science et 200 crédits, afin de débloquer et construire vite toutes les planètes spéciales.
2. En tant qu'humain qui enregistre, je veux pouvoir retaper la séquence, afin de me réapprovisionner en cours de session.
3. En tant qu'agent, je veux que la session enregistrée se rejoue à l'identique en headless, triche comprise, afin qu'elle serve de filet.
4. En tant qu'agent, je veux que les `events.json` existants (4 booléens) se chargent toujours, afin de ne perdre aucun fixture.
5. En tant qu'agent, je veux que tous les `frames.hash` existants restent identiques, afin que ce changement ne re-baseline rien.
6. En tant que futur porteur PSX, je veux que la triche passe par des boutons de manette, pas par des touches clavier spécifiques.

## Implementation Decisions

- `pico8_input_frame_t` gagne deux boutons ; `btn(4)`/`btn(5)` les lisent. Le parseur accepte 4 ou 6 booléens dans `"dpad"` (ou un champ séparé — au choix de l'implémentation, tant que les anciens fichiers se chargent sans modification). `pico8_input_save`/`pico8_input_append` écrivent le nouveau format ; le contrat d'identité octet pour octet entre les deux est conservé.
- Séquence comptée sur fronts montants (relâché → pressé). Tout front montant d'O ou X hors séquence la remet à zéro ; flèches et souris ne l'interrompent pas. Pas de délai d'expiration (un délai en frames serait rejouable, mais inutile).
- Effet : **affectation**, pas ajout — `jk = 99`, `nz = 200`. Répétable.
- Module séparé `src/pico8_cheat.c`, hors du cart porté, appelé une fois par frame depuis la boucle de mise à jour, avant `_update60`. Il écrit `fm` par l'API de la couche objet actuelle ; le plan 3 le migrera avec la classe `hy`.
- Le backend raylib mappe Z → O et X → X ; il ne fait rien d'autre de la triche.

## Testing Decisions

- Test unitaire d'entrée : un fichier à 4 booléens et un à 6 se chargent ; `save` puis `load` rend les 6 ; `append` reste identique octet pour octet à `save`.
- Test headless (fixture `events.json` scriptée) : la séquence complète met `jk` à 99 et `nz` à 200 à la frame attendue ; une séquence interrompue (A A B A…) ne déclenche rien ; une seconde séquence redéclenche.
- `make frames-check` reste conforme sur tous les rejeux existants sans re-baseline : aucun ne contient O ou X.

## Out of Scope

- Toute autre triche (révélation de galaxie, génération directe d'une planète, choix de graine).
- Le mapping manette réel.
- L'enregistrement de la session elle-même — c'est le travail de l'humain.
- L'outil de couverture des classes (`docs/plan-3.md`, étape 0).
