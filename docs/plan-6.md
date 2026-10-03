# Plan 6 — Retirer ce qui n'existe que pour PICO-8

Brouillon, à passer en `plan-converge` avant toute exécution. Prélude aux plans 7 à 11 vers un `.bin/.cue` PSX.

## 1. Objectif

Le cœur ne garde de PICO-8 que ce dont le jeu a besoin : l'API de dessin, la virgule fixe, la table `sin`/`cos`. Tout le reste part : adresses mémoire, numéros de `stat()`, audio, chaînes héritées du Lua.

## 2. Règles

- Aucun pixel ne change : tous les `*.sha256` de `tests/e2e/` restent identiques.
- Invariant de chaque commit : `make test-all` et `make sanitize-net` passent ; `make test` et `make e2e` ne ralentissent pas.
- Restent, par décision : l'API de dessin (branchée sur le GPU au plan 10), la virgule fixe, la table `sin`/`cos`, `constants-check`, les lignes musique et sons du menu pause (inertes).

## 3. Étapes

1. **Code mort.** L'audio (`p8_sfx`, `p8_music`, `stat 19/20`), le GPIO `0x5f80` de `ui.c`, les octets de musique `0x3101` de `score_panel.c`, le drapeau souris `0x5f2d`, la deuxième manette (GAP-2).
2. **Souris nommée.** `p8_stat(32/33/34)` devient des champs nommés de l'entrée.
3. **Chaînes → `enum`.** Les états (`"placing"`, `"settled"`), les tags (`entities_by_tag("planet")`…), les noms passés en chaîne (`"left"`/`"right"`, `"pressed"`/`"released"`, `"draw_row"`). Les recherches par nom dans les tables de données restent.
4. **Données hors des assets.** La table des palettes, lue par `hud.c` dans le carré 16×16 en haut à gauche des sprites, devient une table C. `pico8/map.json` est supprimé : rejouer les 26 e2e sans le charger ne change aucun pixel, le fond génère ses tuiles.
5. **Mémoire typée.** Le bloc de 56 Ko (`p8_memory_t`, `peek`/`poke`/`memcpy`) devient des champs typés : sprites, tuiles du fond 32×32, écran, palette de dessin.

Chaque étape met à jour `docs/trous-de-portage.md` pour les écarts qu'elle fait disparaître.

## 4. Hors périmètre

- Tout ce qui change un pixel : les bizarreries Lua reproduites (GAP-5, GAP-14…), la table `sin`/`cos` de 64 Ko.
- Le format des assets embarqués sur PSX : plan 10.
