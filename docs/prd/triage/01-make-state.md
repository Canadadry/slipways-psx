---
title: "make state : état texte de la dernière frame d'un rejeu"
description: "Un agent qui joue une partie sans fenêtre ne peut lire que des pixels ; il lui faut l'état de la partie en texte."
status: needs-triage
---

## Problem Statement

On veut mesurer le pire cas mémoire sur une vraie partie remplie (≈375 mois d'actions, ~50 colonies, ~280 slipways, slipgates). Cette partie sera jouée par un agent, sans fenêtre, en écrivant un `events.json` par paquets d'actions puis en rejouant. Aujourd'hui, la seule sortie lisible d'un rejeu est le dump texte du framebuffer (128×128 chiffres hex) : l'agent doit deviner positions des planètes, état des colonies et slipways depuis des pixels, et il n'a aucune vue sur les pics mémoire de `frame`, ni sur les caps qui finissent en `pico8_fatal`.

## Solution

Un outil séparé, `make state EVENTS=<events>`, qui rejoue le fichier depuis l'écran titre exactement comme `make replay` (cheat compris), puis écrit **une seule fois, à la dernière frame**, l'état de la partie en texte dans `tmp/state.txt` : horloge, ressources, caméra, planètes, slipways, structures, et le pic de chaque zone mémoire et de chaque cap depuis le début du rejeu. Pas d'historique par frame. Le dump de frame existant n'est pas modifié.

## User Stories

1. As an agent playing headless, I want the year and month of the last frame, so that I know how much of the 375-month budget remains.
2. As an agent, I want credits (`nz`) and science (`jk`), so that I know when to re-enter the AABBAABB cheat.
3. As an agent, I want the list of acquired techs by name, so that I can check time compression, autoassemblers, slipgates, infragates, space folding are unlocked.
4. As an agent, I want the camera position, so that I can convert world coordinates to screen coordinates for my next clicks.
5. As an agent, I want every planet with its id, world coordinates and screen coordinates, so that I can click on it without reading pixels.
6. As an agent, I want each planet's state (unknown / known / colonized) and type, so that I know which to probe, which to colonize and which are empty (`ll`).
7. As an agent, I want to know whether a planet is currently on screen, so that I know when to scroll.
8. As an agent, I want every slipway as the ids of its two ends, so that I can see the network I built and avoid redundant attempts.
9. As an agent, I want each slipway's layer (normal / infragate), so that I understand crossing refusals.
10. As an agent, I want every structure (kind, id, world and screen coordinates, number of links), so that I know where slipgates are and whether they still accept a link (< 3).
11. As an agent, I want the counts of colonies, slipways and structures as summary lines, so that I can follow progress at a glance.
12. As an analyst, I want the peak bytes of each zone (`frame`, `interface` per sorte, `particles` per kind, `partie`, `chargement`) over the whole replay, so that I can compare to their capacities.
13. As an analyst, I want the peak fill of each cap (`ts`, `rangee`, `iv`, `qy`, `ga`, `js`, `mh`, `fq`, `le`, file de sortie) next to its capacity, so that I can see which one is closest to `pico8_fatal`.
14. As an analyst, I want the current (last-frame) value next to the peak, so that I can tell transient spikes from accumulation.
15. As an agent, I want a stable, line-oriented format (`<section> <key> <values…>`), so that I can grep it without a parser.
16. As an agent, I want the output always at the same path, overwritten at each run, so that I never read a stale state.
17. As a developer, I want `make state` outside `make test` and `make e2e`, so that a partie that ends in `pico8_fatal` never breaks the suite.
18. As an agent, I want the replay to stop with a clear message naming the zone or cap when a `pico8_fatal` fires, and the state written up to that point if possible, so that a crash is itself a measurement.
19. As a developer, I want `make state` to never open a window, so that it respects the no-raylib rule.
20. As a developer, I want the existing replay output (framebuffer dump, capture, `PICO8_REMPLISSAGE`) unchanged, so that no e2e sha256 moves.

## Implementation Decisions

- **Nouveau point d'entrée headless**, distinct de celui de `make replay`. Il partage la même boucle de rejeu (chargement gfx/map, cheat, `update60`/`draw`) ; si la boucle est dupliquée, l'extraire dans un module commun plutôt que la copier.
- **Module d'état (deep module)** : une fonction qui écrit l'état courant du moteur dans un `FILE*`, sans rien allouer dans les zones du jeu (ne doit pas fausser les pics qu'elle rapporte). Testable seule après un rejeu court.
- **Pics manquants à ajouter** : la zone `frame` n'a pas de high-water mark aujourd'hui (elle est remise à zéro à chaque `update60`) — en ajouter un, lu avant chaque reset. Idem pour les caps qui n'exposent que leur valeur courante (`interface` par sorte, file de sortie). Les compteurs existants (`max_vivantes`, `max_octets`, `max_elements` des particles, `pico8_zone_partie_max`, `pico8_zone_chargement_max`) sont réutilisés.
- **Les pics ne changent aucun comportement** : un compteur de plus, aucun effet sur l'allocation, donc aucun sha256 e2e ne doit bouger.
- **Format** : une ligne par fait, sections `clock`, `resources`, `techs`, `camera`, `planet`, `slipway`, `structure`, `count`, `zone`, `cap`. Les noms minifiés du cart restent ceux du glossaire (`docs/glossaire-noms.md`) ; les états de planète en clair (`unknown`, `known`, `colonized`, `empty`) ; la file de sortie est le cap `exit_queue`.
- **Coordonnées** : `world` (unités du cart, 0..262 px) et `screen` (après caméra), plus un drapeau `visible`.
- **Lecture seule** sur l'état du jeu : l'outil lit `cg`/`hv`, `fm`, la caméra ; il n'appelle aucune fonction du cart qui modifie l'état.
- Cible Makefile `state`, avec sa ligne dans la légende en tête du `Makefile`, hors suite.

## Testing Decisions

- Un bon test vérifie la sortie externe : on rejoue un e2e existant et on lit `tmp/state.txt`, sans dépendre de la structure interne.
- Tests à écrire :
  - sur `trade_rich_mine` : le nombre de lignes `planet` vaut le nombre de planètes du jeu, les slipways et colonies attendues apparaissent, le pic `partie` égale `max_octets partie` du `PICO8_REMPLISSAGE`.
  - sur `cheat_code` : crédits et science reflètent le cheat.
  - le pic `frame` est non nul et ≤ capacité ; il est ≥ la valeur courante.
  - écrire l'état ne change ni `partie` ni `frame` (comparer avant/après).
  - un rejeu ne produit pas de fenêtre et le framebuffer dump / sha256 e2e restent inchangés (`make e2e` vert).
- Prior art : `tests/test_pico8_places_vivantes.c` (lance le pilote de rejeu via `system` et lit le fichier `PICO8_REMPLISSAGE`), `tests/test_pico8_replay_usage.c`, `tests/test_pico8_frames_tool.c`.

## Out of Scope

- `make pire-cas` et la partie remplie jouée par l'agent : PRD suivante.
- Toute réduction mémoire ou redécoupage des zones : on analyse seulement.
- Historique par frame ; seul le dernier état est écrit.
- Modification du cheat (il est rejoué tel quel, autant de fois que l'`events.json` le contient).

## Further Notes

- Contexte de l'analyse : borne pire cas par le temps seul (25 ans × 15 mois = 375 actions coûtant du temps), techs time compression / autoassemblers / slipgates / infragates supposées acquises dès le début. Estimations actuelles, non mesurées : `partie` 750–980 Ko (704 dispo), `frame` 0,4–1,2 Mo (256 dispo), caps `ts` (680), `rangee` (100), `iv` (128), `qy` (16), `ga` (8), file de sortie (256) atteignables. Cet outil sert à les mesurer.
