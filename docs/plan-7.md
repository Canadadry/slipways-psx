# Plan 7 — Une partie pleine dans 768 Ko

Premier des cinq plans vers un `.bin/.cue` jouable sur émulateur PSX (plans 7 à 11).

## 1. Objectif

- `make state EVENTS=worst-case/full_game.json` va jusqu'à la dernière frame, sans `p8_fatal`.
- La somme des zones passe de 1,5 Mo à **768 Ko** (`ZONE_BUDGET_BYTES`).
- La moitié libérée est réservée aux tampons de primitives GPU du plan 10 et aux optimisations de fin sur PSX.

## 2. Point de départ mesuré

- `full_game.json` va jusqu'à sa frame 7685 depuis l'étape 1 ; `make full-game` le verrouille.
- Pic de la zone `game` : 583 248 o pour 720 896 réservés, atteint au redémarrage (frame 7643), où l'ancienne et la nouvelle génération du monde coexistent ; une seule génération tient vers 153 Ko dans les e2e.
- La file de sortie a été relevée de 256 à 512 places à l'étape 1 : le redémarrage y abandonne 286 entités d'un coup. Les caps saturent comme les zones.
- Zone `frame` 256 Ko + pic `game` dépassent déjà 768 Ko : la réduction de l'étape 4 est le chemin attendu, pas un repli.
- Les pics des zones `ui` et `particles`, et le tableau complet, sortent de l'étape 3 ; ils remplacent cette section quand elle se clôt.

## 3. Règles

- Le jeu ne change pas : tous les `*.sha256` de `tests/e2e/` restent identiques.
- Une saturation est d'abord lue comme une fuite (place jamais rendue, remise à zéro manquante), jamais comme un besoin. On ne relève une limite qu'une fois la fuite écartée.
- Chaque zone est dimensionnée sur son pic mesuré, plus une marge fixe, la même pour toutes.
- Invariant de chaque commit : `make test-all` et `make sanitize-net` passent ; dès l'étape 2, la cible verrou aussi.

## 4. Étapes

1. **Comprendre la saturation de `frame`.** Trouver ce qui s'accumule dans la zone à la frame 4337 (`help-me-debug`), puis le corriger. Fini quand `full_game.json` va au bout avec les tailles actuelles.
2. **Verrou.** `full_game.json` joué jusqu'au bout entre dans une cible dédiée, hors de `make test-all` qui ne doit pas ralentir, pour qu'une régression mémoire se voie dès le redimensionnement.
3. **Mesurer.** Le pic de chaque zone et de chaque cap, sur `full_game.json` complet et sur chaque e2e (`full_game.json` n'ouvre ni boîte de message ni fond modal), écrit dans `tmp/state.txt`.
4. **Redimensionner**, en sous-étapes :
   - 4a. **Bilan, sans code.** Depuis le tableau de l'étape 3 : pour chaque zone, pic plus marge contre 768 Ko, et l'excès en octets. Pour chaque gros pic (au moins `game`, `frame`, la file de sortie), la frame où il est atteint et s'il est un besoin permanent ou un transitoire de séquence (redémarrage, vidage) ; pour `game`, le pic d'une génération séparé de celui des deux qui coexistent. En sortent les candidats à réduire, du plus gros au plus petit, chacun avec son gain attendu.
   - 4b. **Une réduction par commit**, le plus gros candidat d'abord, remesurée par la campagne, gain réel noté à côté du gain attendu.
   - 4c. **Un commit par zone redimensionnée** à son pic plus la marge ; chaque cap, file de sortie comprise, revu de même contre son pic.
   - 4d. **`ZONE_BUDGET_BYTES` à 768 Ko en dernier**, une fois que la somme tient.
   - Si les candidats de 4a ne couvrent pas l'excès, s'arrêter et l'écrire : ni chaîne de réductions sans fin, ni plafond relevé sans décision humaine.

## 5. Hors périmètre

- Le budget de slipways et de relais (une évolution du jeu).
- Les remesures sur MIPS : plan 11.
