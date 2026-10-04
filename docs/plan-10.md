# Plan 10 — Backend PSX : rendu GPU et manette

Brouillon, à passer en `plan-converge` avant toute exécution. Quatrième des cinq plans vers un `.bin/.cue` PSX (plans 7 à 11).

## 1. Objectif

Le jeu complet compilé pour MIPS, dans un `.bin/.cue` qui démarre dans l'émulateur, affiche l'écran titre et se pilote à la manette.

## 2. Approche

- **Rendu GPU.** Chaque dessin PICO-8 (`cls`, `pset`, `line`, `rect`, `rectfill`, `circ`, `circfill`, `spr`, `map`, `print`, `fillp`, `palt`) devient des primitives GPU, au lieu d'un framebuffer 160×120 recopié. Les sprites et la police vivent en VRAM.
- **Écart accepté.** Le rendu PSX peut différer de quelques pixels du PC (cercles, lignes). L'écart sera jugé sur place, pas avant.
- **Manette.** La couche boutons → souris du plan 8, branchée sur le pilote de manette.
- **Le cœur ne bouge pas.** Seuls `src/p8/` et un nouveau `src/platform/` PSX s'adaptent.
- **Prérequis.** Plans 7 à 9 finis ; le plan 7 a laissé 1 980 o sous 768 Ko de zones, les tampons de primitives GPU vivent dans la moitié libérée. Le plan 9 sort une capture de VRAM, pas seulement du TTY : l'étape 4 s'y vérifie.
- **Invariant de chaque commit.** `make test-all` passe, les `*.sha256` de `tests/e2e/` ne changent pas, `make test` et `make e2e` ne ralentissent pas ; `make sanitize-net`, `make full-game` et `make state` (marge, cf. plan 8 étape 0) passent aussi.

## 3. Étapes

1. **Le cœur compile pour MIPS.** Sans rendu, il tourne N frames, et les appels libc restants (`fopen`, `snprintf`, `__divdi3`…) sont remplacés ou fournis. Fini quand l'état après N frames, sorti en TTY dans l'émulateur, égale celui du PC (`make state`) sur les lignes du jeu (horloge, planètes, slipways, structures) ; les lignes `zone` et `cap` sont relevées comme première mesure MIPS pour le plan 11.
2. **Assets.** Sprites, carte et police embarqués sans parser JSON au démarrage.
3. **Séparer l'API de dessin de son backend.** Le logiciel actuel reste pour PC et e2e, et le GPU s'ajoute pour la PSX. Les e2e PC ne changent pas.
4. **Primitives GPU,** une famille à la fois, chacune vérifiée par une capture VRAM en émulateur (plan 9).
5. **Manette** branchée, puis l'écran titre passé dans l'émulateur via une entrée scriptée.
6. **`.bin/.cue` du jeu** produit par une cible `make`.

## 4. Points ouverts

- `fillp`, `palt` et la palette de dessin (blocs copiés dans la palette de dessin par `hud.c`) sur GPU : motif ou CLUT par texture, ou retour au logiciel pour ces cas. Tranché au début de l'étape 4.
- Le rejeu d'un `events.json` dans l'émulateur, pour réutiliser les e2e. Tranché avant l'étape 5, qui en a besoin.
