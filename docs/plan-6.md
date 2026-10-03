# Plan 6 — Une partie pleine dans 768 Ko

Brouillon, à passer en `plan-converge` avant toute exécution. Premier des cinq plans vers un `.bin/.cue` jouable sur émulateur PSX (plans 6 à 10).

## 1. Objectif

- `make state EVENTS=worst-case/full_game.json` va jusqu'à la dernière frame, sans `p8_fatal`.
- La somme des zones passe de 1,5 Mo à **768 Ko** (`ZONE_BUDGET_BYTES`).
- La moitié libérée est réservée aux tampons de primitives GPU du plan 9 et aux optimisations de fin sur PSX.

## 2. Point de départ mesuré

- `full_game.json` s'arrête frame 4337 : zone `frame` (256 Ko) saturée.
- Pic de la zone `game` : 220 Ko pour 768 Ko réservés.
- Les pics des zones `ui` et `particles` restent à mesurer jusqu'au bout : le dump s'arrête au fatal.

## 3. Règles

- Le jeu ne change pas : tous les `*.sha256` de `tests/e2e/` restent identiques.
- Une saturation est d'abord lue comme une fuite (place jamais rendue, remise à zéro manquante), jamais comme un besoin. On ne relève une limite qu'une fois la fuite écartée.
- Chaque zone est dimensionnée sur son pic mesuré avec `full_game.json`, plus une marge fixe, la même pour toutes.
- Invariant de chaque commit : `make test-all` et `make sanitize-net` passent.

## 4. Étapes

1. **Comprendre la saturation de `frame`.** Trouver ce qui s'accumule dans la zone à la frame 4337 (`help-me-debug`), puis le corriger. Fini quand `full_game.json` va au bout avec les tailles actuelles.
2. **Mesurer.** Le pic de chaque zone et de chaque cap sur `full_game.json` complet, écrit dans `tmp/state.txt`.
3. **Redimensionner.** Chaque zone ramenée à son pic plus la marge ; `ZONE_BUDGET_BYTES` à 768 Ko. Si la somme dépasse, réduire l'empreinte des plus gros consommateurs avant de toucher au plafond.
4. **Verrou.** `full_game.json` joué jusqu'au bout entre dans une cible de vérification (`make test-all` ou une cible dédiée si trop lent), pour qu'une régression mémoire se voie.

## 5. Hors périmètre

- Le budget de slipways et de relais (une évolution du jeu).
- Les remesures sur MIPS : plan 10.
