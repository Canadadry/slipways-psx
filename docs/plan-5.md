# Plan 5 — Des noms clairs, en anglais

Ce document est **le plan** ; aucune implémentation n'est engagée par sa rédaction. Exécution prévue par `plan-pilot`, une PRD par étape (§6), avec deux points d'arrêt humains (étapes 0 et 2).

## 1. Objectif

À la fin, chaque fichier, type, fonction, champ de struct, constante et globale du code porte un nom anglais clair. Plus aucun digramme du cart minifié (`jo`, `jc`, `g_`…), plus aucun mot français dans le code.

Seule tolérance, à la Go : une variable locale ou un paramètre d'une ou deux lettres (`i`, `n`, `e`, `p`, `dx`) quand sa portée tient en **40 lignes au plus**.

## 2. Décisions actées, à ne pas rouvrir

- **On coupe le cordon avec le cart.** Le jeu marche et les e2e le couvrent : le C devient la référence. La règle « portage littéral, fonction par fonction » sort du `CLAUDE.md`. `pico8/lua.lua` reste dans le dépôt, comme archive.
- **Pont inverse unique : `docs/renommage.tsv`** (`ancien`, `nouveau`, `fichier`), produit à l'étape 2. C'est lui qui sert si on doit retourner fouiller le Lua. `docs/glossaire-noms.md` est supprimé.
- **Périmètre de l'anglais : le code seul.** `src/`, `tests/`, `tools/`, `Makefile`, fixtures, messages d'`assert`, commentaires. Les docs, le `CLAUDE.md`, les skills et les messages de commit restent en français.
- **Pas de préfixe pour le jeu.** `p8_` pour les seuls symboles du runtime PICO-8 émulé.
- **`main` gelé** pendant tout le chantier : rien n'est en vol, une PR par étape, mergée avant d'ouvrir la suivante.

## 3. Arborescence cible

```
src/
  p8/         runtime PICO-8 émulé — symboles en p8_, fichiers sans préfixe
              fixed, gfx, palette, mem, str, ptrs, input, mouse_reader,
              audio (stub), assets, quarter_wave, fatal
  engine/     socle objet et mémoire, sans planète ni slipway
              struct_header, classe racine, entity, ents, collect,
              dispatch_context, zones mémoire, file de sortie, bootstrap
  game/       le jeu
              planet*, slipway*, trade, research*, economy*, galaxy, gen,
              probe*, wormhole*, ui, ui_button, menu, hud, msgbox, layout,
              help, score_panel*, scroll, particle, bg, tech_gate, cheat,
              game_data, map_tables, data_init, cart…
  platform/   raylib_backend, raylib_backend_coords, fatal_pc
  devtools/   vérification sans fenêtre : dump, gif, screencast*, replay_*,
              session_recorder, input_json, coverage, minimize,
              fixed_test_double
  cmd/        un fichier par binaire, à plat : game.c, replay.c, frames.c,
              gif.c, minimize.c, screencast_viewer.c, class_coverage.c
tests/        reste à plat ; fichiers renommés (test_planet.c…), pas déplacés
```

La liste des fichiers par répertoire est indicative : l'étape 1 tranche chaque fichier et le note dans « Découvert ».

Les includes s'écrivent depuis `src/` (`#include "game/planet.h"`), avec `-Isrc` seul.

## 4. Règles de nommage

| Élément | Règle | Exemple |
|---|---|---|
| Fichier | nom du module, sans préfixe, y compris dans `p8/` | `game/planet.c`, `p8/gfx.c` |
| Type | `snake_case_t` | `planet_t`, `trade_route_t`, `p8_fixed_t` |
| Fonction exportée | `module_verbe[_objet]`, module = nom du fichier | `planet_draw`, `slipway_find_endpoint` |
| Méthode de classe | même règle, sur le type receveur | `entity_reset_state` |
| Fonction `static` | pas de préfixe de module | `find_nearest` |
| Constante, enum, macro | `MODULE_UPPER_CASE` | `TAG_PLANET` |
| Champ de struct | nom complet, jamais abrégé | `e->pos`, pas `e->p` |
| Globale | nom complet ; `g_` seulement pour une statique mutable de fichier | `g_frame_count` |
| Locale, paramètre | 1–2 lettres permises si portée ≤ 40 lignes ; idiomes encouragés | `i`, `n`, `e`, `p`, `dx` |

Exceptions à la règle des deux lettres : `x` et `y` comme champs d'un vecteur. La liste d'exceptions de la garde (§5) ne s'allonge que par une décision humaine.

**Commentaires.** Un commentaire ne cite plus jamais le cart : chaque nom Lua est remplacé par le nouveau nom C, et une phrase « the cart does X » devient « X » ou disparaît. La règle des quatre destinations du `CLAUDE.md` reste, moins sa première branche (le sens d'un nom minifié) : il n'y a plus de nom minifié. Les étiquettes `TROU-N` deviennent `GAP-N`.

## 5. Invariant et gardes

**Invariant de chaque commit :** `make test-all` passe et tous les `*.sha256` de `tests/e2e/` restent identiques. Renommer ne change pas un pixel. Les références qui contiennent des chemins ou des noms (`line_coverage_reference.txt`, listes d'exceptions de `tools/`) sont mises à jour dans le même commit que ce qui les invalide, jamais régénérées pour masquer une différence de contenu.

**Deux gardes permanentes**, ajoutées à `make test` à l'étape 5 :

1. **Anti-cart.** Aucun ancien nom du TSV n'apparaît dans le code, ni comme identifiant, ni entre backticks, ni dans une chaîne. Les noms trop courts pour une recherche texte (`p`, `t`, `r`) sont vérifiés via l'AST clang.
2. **Portée, à la Go.** Depuis l'AST clang : un identifiant de 1 ou 2 caractères est refusé s'il nomme un type, une fonction, un champ, une constante ou une globale ; il n'est permis qu'à une locale ou un paramètre dont la portée tient en ≤ 40 lignes, hors liste d'exceptions (§4).

## 6. Étapes

Chacune est une PRD et une PR, et respecte l'invariant du §5.

0. **Vocabulaire de base — point d'arrêt humain.** Le dictionnaire des ~50 concepts récurrents du domaine (`jo`, `ga`, `jc`, `g_`, `pj`, `hp`, `lq`…), les champs de l'en-tête commun (`pico8_struct_header_t`) et les membres de la vtable des classes (`kj`, `ev`, `hd`, `hk`, `hb`…), présenté **rempli** à partir du glossaire et de la lecture du code. L'humain corrige ; aucun code n'est touché. Le vocabulaire validé est l'entrée des étapes 1 à 3.
1. **Déplacements seuls.** `git mv` vers l'arborescence du §3 (noms de fichiers anglais du vocabulaire), includes en `"dir/fichier.h"`, `-Isrc`, `Makefile`, chemins dans `tools/`. Aucun symbole ne change. Commits de `git mv` purs, séparés des retouches de contenu, pour que `git log --follow` suive.
2. **Dictionnaire complet — point d'arrêt humain.** `docs/renommage.tsv` : fichiers, types, fonctions, champs, constantes, statiques, globales, noms de tests et de fixtures, dérivés du vocabulaire et des règles du §4. Extraction exhaustive par l'AST clang (`-Xclang -ast-dump=json`), pas à la main : chaque déclaration de 1–2 caractères ou issue d'un digramme y figure. L'humain relit un échantillon ; un nom corrigé dans l'échantillon fait revoir ceux qui en dérivent.
3. **Application.** Renommage par token à partir des positions exactes données par l'AST clang ; pas de `sed` naïf (un champ `p` ou `t` n'est pas cherchable en texte). Le compilateur arbitre : une occurrence oubliée casse le build. Un commit par lot cohérent : d'abord l'en-tête commun et la vtable, puis module par module. Les tests, fixtures et données de `tools/` suivent dans le lot de leur module.
4. **Commentaires, asserts, locales.** Un agent par module : commentaires et messages d'`assert` réécrits en anglais sans citer le cart (§4), mots français restants retirés, locales cryptiques à longue portée renommées. Le skill `reprendre-les-commentaires` cadre le travail.
5. **Clôture.**
   - `TROU-N` → `GAP-N` dans le code et comme clé dans `docs/trous-de-portage.md`, qui reste en français mais décrit chaque écart avec les nouveaux noms C ; le nom Lua n'apparaît qu'en tête de chaque GAP.
   - Les deux gardes du §5 dans `make test`, vertes.
   - `CLAUDE.md` : retrait du portage littéral et de la branche « nom minifié » des commentaires, nouvelle arborescence, renvoi `TROU-7` → `GAP-7`.
   - `docs/glossaire-noms.md` supprimé ; `README`, les autres docs et les skills qui citent des symboles ou des chemins (dont `reprendre-les-commentaires`) mis à jour vers les nouveaux noms.

Chaque étape se termine en ajoutant sous sa propre entrée une courte note « Découvert » : les noms qui ont résisté aux règles du §4, les fichiers rangés autrement que dans le §3, les exceptions de garde proposées. Si un constat contredit le §3 ou le §4, l'étape corrige le plan dans le même commit.

## 7. Hors périmètre

- Changer le comportement du jeu, découper ou fusionner des modules au-delà des déplacements du §3. Un renommage qui révèle un module mal découpé est noté dans « Découvert », pas traité.
- Traduire en anglais les docs, le `CLAUDE.md`, les skills, les messages de commit.
- Réécrire l'historique git.
