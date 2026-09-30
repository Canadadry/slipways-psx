---
title: "Plan 3, étapes 2.1 et 2.2 : trois includes morts retirés (`inclusion-mutuelle` 21/52 → 18/45), `struct.h` n'inclut plus `game_data.h` (`noyau-inclut` 1 → 0) et naissance de `COMPOSITION_RESIDUE_GUARD` dans `make test` — deux commits, jeu identique"
description: "Les deux premières lignes de la phase 2 sont sans lien avec la couche objet : trois includes que personne n'utilise créent trois paires d'inclusion mutuelle, et l'en-tête du noyau tire toutes les données de jeu. On les retire, on prouve que rien n'en dépendait, et la première catégorie vidée entre dans une garde de `make test`."
status: needs-triage
---

## Problem Statement

**Écrit sur `91197eb`** (branche `claude/plan-pilot-3-7zdft0`). Aucune ligne du dépôt n'a changé pour écrire ce PRD. Les comptes et les preuves viennent d'essais sur une copie jetable de l'arbre (`git archive HEAD`), supprimée depuis. L'implémenteur les remesure sur le vrai arbre. Un écart s'explique dans le message de commit.

Vu du mainteneur du port : la mesure 2.0 est faite et la liste de la phase 2 est écrite (`docs/plan-3.md`, « Étapes de la phase 2 »). Ses deux premières lignes ne touchent pas la couche objet :

- **2.1** : trois `#include` que leur fichier n'utilise pas réalisent trois paires d'inclusion mutuelle. Tant qu'ils restent, l'étude C (2.11) doit examiner trois paires qui n'en sont pas.
- **2.2** : l'en-tête du noyau, `pico8_struct.h`, inclut `pico8_game_data.h` (574 lignes, tous les `pico8_X_start_t`). Tout archétype de 2.12 qui nommerait un type du noyau depuis `game_data.h` créerait la paire `game_data` ↔ `struct`. Et aucune catégorie de 2.0 n'est encore gardée : une régression passerait sans bruit.

### Texte du plan, recopié (`docs/plan-3.md` à `91197eb`, l. 384-385)

> - 2.1 — `inclusion-mutuelle`, includes morts : `research.c:9` (`data_init.h`) et `research.c:10` (`research_display.h`), dont `research.c` n'utilise aucun symbole propre, et `slipway.c:22` (`wormhole.h`), dont `slipway.c` n'utilise aucun symbole propre (`wormhole.c:15` ne lit rien de `slipway.h` non plus ; le retirer aussi est libre). Modules : `research`, `slipway` (± `wormhole`). Compte : `inclusion-mutuelle` 21 paires / 52 lignes → **18 / 45** (paires `data_init` ↔ `research`, `research` ↔ `research_display`, `slipway` ↔ `wormhole`) ; pied → **1363 / 93** (mesuré sur copie ; `-fsyntax-only` passe pour les trois `.c`). Preuves : refactor pur (objets sans `-g` identiques). `class-coverage-net` : non. `sanitize-net` : non. Budget ≈ 30 min.
> - 2.2 — `noyau-inclut` : `struct.h:7` n'inclut plus `game_data.h`. `struct.h` n'y prend que `pico8_str_seq_t` (`get_nu`, `struct.h:97`), `pico8_kind_rec_t` (`pico8_entity_kind_t.rec`, `:127`) et, par transitivité, `pico8_qz_box_t` (`geom.h:33`, pour `get_kq`) : les deux premiers reçoivent une étiquette de struct (`game_data.h:32`, `:45`) et une déclaration anticipée dans `struct.h`, comme `pico8_qy`/`pico8_emitter`/`pico8_probe` (`struct.h:58-61`) ; `struct.h` inclut `pico8_geom.h`. Essai sur copie : tous les `.c` de `src/` et `tests/` passent `-fsyntax-only`, hors les quatre qui échouaient déjà sans `-DPICO8_CLASS_COVERAGE` ou `raylib.h`. Crée `COMPOSITION_RESIDUE_GUARD := noyau-inclut`. Modules : `struct`, `game_data`, `Makefile`. Compte : `noyau-inclut` 1 → **0**, gardé ; pied → **1362 / 93** (mesuré sur copie). Preuves : refactor pur (objets sans `-g` identiques) ; garde mordante (`#include "pico8_game_data.h"` remis dans une copie de `struct.h` → `make test` rouge). `class-coverage-net` : non. `sanitize-net` : non. Budget ≈ 30 min.

Règles communes rappelées par le plan (l. 367-373) : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte (`*_HASH`, `FROZEN_*`) changée. Une catégorie tombée à 0 entre dans `COMPOSITION_RESIDUE_GUARD` (créée à 2.2, lue par `make test` via `composition_residue.sh --check`, sur le modèle de `DATA_RESIDUE_GUARD`, `Makefile:840`). La garde doit être prouvée mordante par une ligne réinjectée dans une copie.

### État de départ (`91197eb`, remesuré)

| Commande | Résultat |
|---|---|
| `make composition-residue` | `parente 26 17`, `lien-parent 111 27`, `accesseurs-q6 262 33`, `défauts 701 52`, `nu 208 37`, `aiguillage-nom 30 8`, `un-ou-faux 27 14`, `void-ptr 11 7`, `ptrs-entité 7 4`, `noyau-inclut 1 1`, `atteinte-à-prouver 1 1`, `inclusion-mutuelle 21 21` ; **`résidu 2 : 1370 ligne(s), 96 fichier(s)`** |
| `sh tools/composition_residue.sh --check inclusion-mutuelle` | **52** lignes `#include` |
| `sh tools/composition_residue.sh --check noyau-inclut` | 1 ligne : `src/pico8_struct.h:7:#include "pico8_game_data.h"` |
| `make test` | vert, 71 binaires « all tests passed » |
| `make e2e` | 26 conformes |

## Solution

Deux commits, dans cet ordre. Chacun est vérifié et poussé avant le suivant.

**2.1 : trois lignes retirées, rien d'ajouté.**

| Include retiré (numéros de `91197eb`) | Paire qui tombe | Autres lignes `#include` qui quittent le compte |
|---|---|---|
| `src/pico8_research.c:9` `#include "pico8_data_init.h"` | `data_init` ↔ `research` | `src/pico8_data_init.c:3` |
| `src/pico8_research.c:10` `#include "pico8_research_display.h"` | `research` ↔ `research_display` | `src/pico8_research_display.c:14`, `src/pico8_research_display.h:8` |
| `src/pico8_slipway.c:22` `#include "pico8_wormhole.h"` | `slipway` ↔ `wormhole` | `src/pico8_wormhole.c:15` |

Les quatre lignes de droite restent dans leurs fichiers. Elles ne réalisent plus de paire, donc le compteur ne les compte plus. Résultat : `inclusion-mutuelle` **18 paires / 18 modules / 45 lignes**. Le pied tombe à **1363 / 93** : `pico8_data_init.c`, `pico8_research_display.c` et `pico8_research_display.h` n'ont plus aucune ligne comptée.

**2.2 : `struct.h` cesse de tirer les données de jeu.**

- `pico8_game_data.h` : `typedef struct {` devient `typedef struct pico8_str_seq {` (l. 32) et `typedef struct pico8_kind_rec {` (l. 45). Les noms `pico8_str_seq_t` et `pico8_kind_rec_t` ne changent pas.
- `pico8_struct.h` :
  - la l. 7 `#include "pico8_game_data.h"` part ;
  - `#include "pico8_fixed.h"` et `#include "pico8_geom.h"` entrent, dans l'ordre alphabétique du bloc ;
  - `struct pico8_str_seq;` et `struct pico8_kind_rec;` rejoignent les déclarations anticipées des l. 58-61 ;
  - `get_nu` (l. 97) rend `const struct pico8_str_seq *` ;
  - `rec` (l. 127) devient `const struct pico8_kind_rec *`.
- `Makefile` : `COMPOSITION_RESIDUE_GUARD := noyau-inclut`, une cible `composition-guard` ajoutée aux prérequis de `test:`, et la légende d'en-tête (deux entrées).

Résultat : `noyau-inclut` **0**, gardé. Le pied tombe à **1362 / 93**. `pico8_struct.h` reste dans le résidu par ses lignes `nu`/`accesseurs-q6`/`lien-parent`.

## User Stories

1. En tant qu'auteur de l'étude C (2.11), je veux que les paires d'inclusion faites d'un include mort aient disparu, afin de n'étudier que les 18 paires qui ont une raison d'être.
2. En tant que mainteneur, je veux que chaque include retiré soit nommé par `fichier:ligne`, afin de relire le diff sans le deviner.
3. En tant que relecteur, je veux la preuve que le fichier compile sans cet include, afin d'écarter l'include « utile par accident ».
4. En tant que relecteur, je veux la preuve que le préprocesseur trié ne perd que des déclarations que rien n'emploie, afin d'écarter une macro ou une déclaration lue en silence.
5. En tant que gardien du jeu, je veux des objets identiques avant et après, `nm` compris, afin de savoir que le binaire n'a pas changé.
6. En tant que gardien du jeu, je veux `make e2e` à 26 conformes après chaque commit, afin que le jeu reste le même image par image.
7. En tant qu'auteur des archétypes (2.12), je veux que l'en-tête du noyau n'inclue plus `game_data.h`, afin qu'un archétype puisse nommer un type du noyau sans créer la paire `game_data` ↔ `struct`.
8. En tant que relecteur de 2.2, je veux savoir quels fichiers recevaient `game_data.h` par `struct.h`, et par quel chemin chacun l'obtient désormais, afin qu'aucun ne tienne à un fil que je ne vois pas.
9. En tant que gardien de la règle de D.13, je veux qu'un fichier qui perd un chemin gagne l'include direct de ce qu'il emploie, afin qu'aucun fichier ne compile par chance.
10. En tant que `plan-pilot`, je veux `COMPOSITION_RESIDUE_GUARD` dans `make test`, afin qu'une catégorie vidée ne revienne pas sans bruit.
11. En tant que `plan-pilot`, je veux la preuve que la garde mord, afin de ne pas me fier à une garde qui ne voit rien.
12. En tant qu'auteur des étapes suivantes, je veux que la garde soit une variable qu'on allonge d'un mot, afin que chaque catégorie vidée (2.4, 2.5, 2.6…) n'ajoute qu'un mot.
13. En tant que vérificateur, je veux des greps de Sortie dont chaque « vide » nomme ce qu'on garde, afin de ne pas confondre « vide » et « vide sauf ce qu'on a oublié ».
14. En tant que lecteur du plan, je veux une note « Découvert à 2.1 » et une note « Découvert à 2.2 », courtes, afin que 2.3 parte des vrais comptes.

## Implementation Decisions

### Modules

- **2.1** : `research`, `slipway`. `wormhole` n'est pas touché (voir « Décisions de l'agent »).
- **2.2** : `struct`, `game_data`, `Makefile`.
- **Les deux** : `docs/plan-3.md`, une note « Découvert à » au bout de chaque ligne, dans le commit de l'étape.

### 2.1 : pourquoi chaque include est mort (prouvé sur copie)

Méthode : on compare les en-têtes que le fichier inclut, avant et après (`cc -MM`). On liste les identifiants déclarés par les en-têtes perdus : sortie `-E -P` et `-E -dM` des seuls `#include` du fichier, avant moins après. On les croise avec les identifiants du corps du fichier, sans commentaires, sans chaînes et sans ses propres `#include` (`cc -fpreprocessed -dD -E -P`). Le témoin mord : retirer `slipway.c:16` (`planet.h`) fait sortir `pico8_jo_is_family`, `pico8_jo_struct_class`, `pico8_jo_t`, et retirer `research.c:8` (`collect.h`) fait sortir `pico8_ho_gp_has`.

| Include | En-têtes qui quittent la fermeture du fichier | Identifiants perdus | Employés par le fichier |
|---|---|---|---|
| `research.c:9` | `pico8_data_init.h` | 3 (`pico8_data_init`, `pico8_data_shutdown`, sa garde) | **aucun** |
| `research.c:10` | `pico8_research_display.h`, `pico8_layout.h`, `pico8_gfx.h`, `pico8_mem.h` | 148 | **aucun** |
| `research.c:9` + `:10` | les cinq ci-dessus | 151 | **aucun** |
| `slipway.c:22` | `pico8_wormhole.h` | 19 | **aucun** |

- **Compilation** : les deux `.c` passent `cc -std=c99 -Wall -Wextra -Werror -c` sans l'include.
- **Préprocesseur trié** (`-E -P | sort -u`) : il ne perd que les déclarations des en-têtes ci-dessus. Aucune n'est employée (tableau).
- **Objets** (sans `-g`, compilés depuis la racine de l'arbre sous le même chemin `src/…`) :
  - **ligne vidée** (l'include remplacé par une ligne blanche, numéros inchangés) : `.o` **identiques octet à octet** pour `research` et `slipway` ;
  - **ligne supprimée** (le commit) : `nm` **identique** ; `objdump -d` ne diffère que par l'immédiat `mov $<ligne>,%edx` des `assert` situés après la ligne retirée (`__LINE__` passé à `__assert_fail`), **11** dans `research`, **8** dans `slipway`, et aucune autre instruction ; sous `-DNDEBUG`, `objdump -d` identique pour les deux ;
  - les 148 autres objets de `src/` et `tests/` sont identiques octet à octet.
- **Règle de D.13** : `research.c` et `slipway.c` perdent chacun un chemin. Ce qu'ils emploient ne venait pas par ce chemin (colonne « employés » vide) : **aucun include direct gagné**.

### 2.2 : ce qui dépendait de `struct.h:7`, et par où chacun l'obtient désormais

**`struct.h` lui-même** (le seul fichier qui perd une ligne d'include) employait par cette ligne :

| Ce qu'il emploie | Avant | Après |
|---|---|---|
| `pico8_str_seq_t` (`get_nu`) | `game_data.h` | étiquette `struct pico8_str_seq`, déclarée d'avance, type incomplet suffisant (pointeur) |
| `pico8_kind_rec_t` (`rec`) | `game_data.h` | étiquette `struct pico8_kind_rec`, idem |
| `pico8_qz_box_t` (`get_kq`) | `game_data.h` → `geom.h` | **include direct `pico8_geom.h`** |
| `pico8_fixed_t` (`states`, `get_fa`…) | `game_data.h` → `fixed.h` (premier chemin) | **include direct `pico8_fixed.h`** |
| `pico8_vec_t` | `vec.h` (direct) | inchangé |

Règle de D.13 appliquée : `struct.h` gagne `pico8_geom.h` (le plan le prévoyait) et aussi `pico8_fixed.h`, qui venait d'abord par `game_data.h` (décision (G), ci-dessous).

**Fichiers dont la fermeture perd `game_data.h`** (14, mesurés par `cc -MM -MG -DPICO8_CLASS_COVERAGE`, avant/après). Aucun n'emploie un nom déclaré par ce qu'il perd. Les seuls identifiants communs sont des noms de membres ou de locales (`header`, `qe`, `count`, `score`, `qw`, `fa`, `kq`…), qui ne sont pas lus dans `game_data.h`.

| Fichier | Perd | Employé |
|---|---|---|
| `pico8_bootstrap.h`, `pico8_economy_tiers.h`, `pico8_entity_geom.h`, `pico8_ke.h`, `pico8_mouse_reader.h`, `pico8_trade.h` | `game_data.h`, `map_tables.h` | aucun |
| `pico8_economy.h`, `pico8_economy_formulas.h`, `pico8_research_icon.h`, `pico8_layout.h`, `pico8_layout.c`, `pico8_pi.c`, `tests/test_pico8_layout_nil_item_draws_nothing.c` | `game_data.h` (gardent `map_tables.h` par `layout.h` ou en direct) | aucun |
| `pico8_struct.h` | `game_data.h`, `map_tables.h` | voir le tableau précédent |

Aucun de ces 14 fichiers ne perd de ligne d'include, et aucun n'emploie ce qu'il perd : **aucun include direct gagné**.

**Fichiers qui emploient un nom propre à `game_data.h` sans l'inclure** (mesuré à 2.1, `struct.h` exclu). Il y en a 52. Tous le reçoivent encore, par un autre de leurs includes. Pour 38 d'entre eux, le premier chemin (`cc -H`) passait par `struct.h` ; il change ainsi :

| Nouveau premier chemin (après 2.2) | Fichiers |
|---|---|
| `pico8_cart.h > pico8_ui_button.h > pico8_planet.h > game_data.h` | `pico8_cart.c` ; TU `entity_kind`, `fq`, `fr`, `fv`, `g_`, `ga`, `hj`, `hp`, `iv`, `jo`, `jo_spec_defaults`, `js`, `lq`, `mh`, `mx`, `nl`, `nr`, `os`, `pm`, `qb`, `qh` |
| `pico8_entity.h > pico8_scroll.h > game_data.h` | TU `ga_export_panel_by`, `planet_gh_row_lq` |
| `… > pico8_particle.h > game_data.h` | `pico8_planet_family.c`, `.h`, `pico8_planet_family_trade.c`, `.h` |
| `pico8_probe.h > game_data.h` | `pico8_help.c`, `pico8_probe.c`, `pico8_research_icon.c` |
| `pico8_msgbox.h > game_data.h` | `pico8_msgbox.c` |
| `… > pico8_planet.h > game_data.h` | `pico8_planet_display.c`, `pico8_planet_trade.c`, `pico8_ui_button.c` |
| `… > pico8_score_panel.h > game_data.h` | `pico8_score_panel_display.c` |
| `… > pico8_slipway.h > game_data.h` | `pico8_slipway_display.c` |
| `… > pico8_wormhole.h > game_data.h` | `pico8_wormhole_display.c` |

Les 14 autres (`bg.c`, `collect.c`, `planet.c`, `research.c`, `scroll.c`, `slipway.c`, `ui.c`, `wormhole.c` ; TU `bg`, `ie`, `jc`, `mz`, `spec_defaults`, `ui`) gardent leur premier chemin, qui ne passait pas par `struct.h`.

Ces 38 fichiers ne perdent aucune ligne d'include, et `game_data.h` reste dans leur fermeture : ils ne perdent pas de chemin au sens de la règle. **Aucun include direct n'y est ajouté** (G, et question 2 pour l'humain). La liste complète, avec l'ancien et le nouveau chemin, va dans le message de commit.

**Preuves 2.2 (sur copie, après 2.1)**

- **Compilation** : sous `-std=c99 -Wall -Wextra -Werror`, les mêmes 5 fichiers échouent avant et après, et pour la même raison. Ce sont `main_raylib.c`, `main_screencast_viewer.c` et `pico8_raylib_backend.c` (il manque `raylib.h`), et `main_class_coverage.c`, `tests/test_class_coverage.c` (sans `-DPICO8_CLASS_COVERAGE` ; ils compilent avec). Le plan dit « quatre » : c'est cinq en comptant `tests/`. Tous les `.h` de `src/` passent `-x c -fsyntax-only` seuls, hors `pico8_raylib_backend.h` (avant comme après).
- **`main_raylib.c`** (seul includer de `struct.h` qu'on ne compile pas ici) : il garde `game_data.h` dans sa fermeture (`-MM -MG`), rien ne change pour lui.
- **Préprocesseur trié**, sur 150 `.c` (hors les fichiers raylib) :
  - 62 inchangés ;
  - tous les autres ne **gagnent** que ces 6 lignes : `struct pico8_str_seq;`, `struct pico8_kind_rec;`, `typedef struct pico8_str_seq {`, `typedef struct pico8_kind_rec {`, `const struct pico8_str_seq *(*get_nu)(const void *self);`, `const struct pico8_kind_rec *rec;` ;
  - ils ne **perdent** que les 2 lignes que celles-ci remplacent (`const pico8_str_seq_t *(*get_nu)…`, `const pico8_kind_rec_t *rec;`) ;
  - exception : `pico8_layout.c`, `pico8_pi.c` et `test_pico8_layout_nil_item_draws_nothing.c` perdent 265 lignes, le contenu de `game_data.h`, qu'aucun n'emploie (tableau des 14).
- **Objets** : les 150 objets sans `-g` (plus les deux sous `-DPICO8_CLASS_COVERAGE`) sont **identiques octet à octet** avant et après, et les 74 de `src/` compilés sous `-DPICO8_CLASS_COVERAGE` aussi. `nm` est donc identique partout.
- `make test` vert (71 binaires) ; la nouvelle ligne `résidu gardé (noyau-inclut) : 0 ligne(s)` y apparaît ; `make e2e` **26 conformes**.
- **Garde mordante** :
  - **M1** : `#include "pico8_game_data.h"` remis dans une copie de `src/pico8_struct.h` → `make test` sort en code 2, avec `noyau-inclut: src/pico8_struct.h:8:#include "pico8_game_data.h"`, `résidu gardé (noyau-inclut) : 1 ligne(s)` et `make: *** [Makefile:…: composition-guard] Error 1`.
  - **M2** (à faire) : `#include "pico8_map_tables.h"` ajouté à `src/pico8_entity.h` → rouge de même.
  - Chaque mutation est restaurée par `cp`, puis `make test` repasse au vert.

### `Makefile` (2.2), sur le modèle de `data-guard`

- `COMPOSITION_RESIDUE_GUARD := noyau-inclut`, juste après la cible `composition-residue`. Commentaire au-dessus : le même que `DATA_RESIDUE_GUARD` (« Catégories du compteur de composition tombées à 0, séparées par des virgules. »).
- La cible `composition-guard: tools/composition_residue.sh` a une seule recette, `@sh tools/composition_residue.sh --check $(COMPOSITION_RESIDUE_GUARD) src tests`. Elle entre dans `.PHONY` à côté de `composition-residue`.
- `test: unit constants-check float-grep float-guard data-guard composition-guard`.
- Légende d'en-tête : `make test -> … data-guard + composition-guard, la boucle rapide`, et une entrée `make composition-guard -> catégories de COMPOSITION_RESIDUE_GUARD à 0, dans make test` sous `make data-guard`.
- Pas de branche « garde vide » : dès 2.2, la variable n'est jamais vide (G).
- Rien d'autre ne change : ni `tools/composition_residue.sh`, ni `tests/test_composition_residue.c`. Le mode `--check` y est déjà testé (`test_check_fails_only_on_a_named_category_present`, `test_other_witnesses_of_each_pattern_bite` pour `noyau-inclut`).

### `docs/plan-3.md`

- **Note « Découvert à 2.1 »** au bout de la ligne 2.1, dans le commit 2.1. Elle donne les trois includes retirés, `inclusion-mutuelle` 18 / 45, le pied 1363 / 93, « aucun include direct gagné », `wormhole.c:15` laissé, et l'écart `__LINE__` des objets.
- **Note « Découvert à 2.2 »**, dans le commit 2.2. Elle donne `noyau-inclut` 0, la valeur de `COMPOSITION_RESIDUE_GUARD`, le pied 1362 / 93, les deux includes directs gagnés par `struct.h`, « cinq » fichiers non compilables au lieu de « quatre », et M1/M2.
- Rien d'autre du plan ne change.

### Commentaires

Aucun commentaire n'est ajouté ni reformulé dans `src/`. Les déclarations anticipées suivent celles des l. 58-61, qui n'en ont pas. Aucun renvoi à 2.1, 2.2 ou au plan dans `src/`, `tests/` ou le `Makefile`.

### Anti-tassement

- **Interdit** :
  - changer le motif de `noyau-inclut` ou la liste `NOYAU` de `tools/composition_residue.sh` ;
  - renommer ou déplacer un en-tête pour échapper au motif ;
  - déplacer le contenu de `game_data.h` vers un autre en-tête inclus par le noyau ;
  - inclure `game_data.h`, `map_tables.h` ou `data_init.h` depuis `ke.h` ou `entity.h` ;
  - un alias `typedef` de `pico8_str_seq_t` ou `pico8_kind_rec_t` dans `struct.h` (redéfinir un typedef, c'est du C11, pas du C99) ;
  - toucher `tests/`, `tests/e2e/`, un `*.sha256` ou `tools/`.
- **Classification au message**, pour chaque ligne comptée retirée ou modifiée (numéros du parent) :
  - **2.1** : (a) supprimée sans remplaçant, 3 lignes (`research.c:9`, `:10`, `slipway.c:22`) ; sortent aussi du compte sans être touchées : `data_init.c:3`, `research_display.c:14`, `research_display.h:8`, `wormhole.c:15` ;
  - **2.2** : (a) 1 (`struct.h:7`) ; lignes comptées modifiées non retirées : `struct.h:97` (`nu`, `accesseurs-q6`, reste comptée, même compte) ; lignes qui gagnent une catégorie : aucune.

## Implementation Chunks

1. **2.1** (un commit, ≈ 30 min) : trois lignes de `src/`, preuves, note du plan, `make test`, `make e2e`, push.
2. **2.2** (un commit, ≈ 30 min) : `game_data.h` (2 lignes), `struct.h` (1 retirée, 4 ajoutées, 2 modifiées), `Makefile`, preuves, M1/M2, note du plan, `make test`, `make e2e`, push.

2.2 part de l'arbre de 2.1 : ses preuves d'objets comparent 2.1 à 2.2.

## Testing Decisions

- **Aucun TU nouveau ni modifié.** Ce sont des refactors purs : on prouve l'identité du binaire, on ne teste pas un comportement. La garde s'appuie sur `--check`, déjà couvert par `test_composition_residue`.
- **Preuve par étape** :
  - **2.1** : `-fsyntax-only` des deux `.c` ; tableau des identifiants perdus et employés ; objets de la variante « ligne vidée » identiques octet à octet ; variante supprimée : `nm` identique et `objdump -d` différent seulement par les immédiats `__LINE__` (compte déclaré) ;
  - **2.2** : tous les objets identiques octet à octet ; préprocesseur trié (6 lignes gagnées, 2 perdues, 265 inutilisées dans 3 TU) ; M1, M2.
- **Prior art** : preuve d'objets identiques de D.19c1, préprocesseur de D.21a, garde mordante de D.13 et D.11e3 (`make data-guard`).
- **Filets** : `class-coverage-net` et `sanitize-net` ne sont **pas** exigés. Aucun constructeur n'est touché, aucune durée de vie ne change, les objets sont identiques.

### Recette des objets (à copier dans le journal)

```sh
# arbre A (parent) et B (après) extraits par git archive dans deux dossiers ;
# depuis la racine de chacun, pour chaque .c de src/ et tests/ :
cc -std=c99 -Wall -Wextra -Werror -c "$f" -o "$OUT/$(echo $f | tr / _).o"
# puis cmp A/x.o B/x.o ; pour 2.1 : nm, objdump -d | tail -n +4, et -DNDEBUG
```

Compiler depuis la racine de chaque arbre, sous le même chemin relatif (`src/…`). Sinon, `__FILE__` change `.rodata` et `nm` diffère pour une raison étrangère.

### Greps de Sortie (sur l'arbre final de chaque commit)

Après 2.1 :

1. `grep -nE '#include "pico8_(data_init|research_display)\.h"' src/pico8_research.c` → **vide**.
2. `grep -n '#include "pico8_wormhole.h"' src/pico8_slipway.c` → **vide**.
3. `grep -rn '#include "pico8_data_init.h"' src tests` → vide **hors** `cart.c:12`, `data_init.c:1`, `help.c:7`, `planet_display.c:9`, `research_display.c:9`, `score_panel.c:9`, `score_panel_display.c:8` (7 lignes).
4. `grep -rn '#include "pico8_research_display.h"' src tests` → vide **hors** `planet_display.c:21`, `research_display.c:1`.
5. `grep -rn '#include "pico8_wormhole.h"' src tests` → vide **hors** `bg.c:15`, `cart.c:43`, `probe.c:22`, `ui.c:17`, `wormhole.c:1`, `wormhole_display.c:9`, `wormhole_display.h:8`.
6. `sh tools/composition_residue.sh --detail | grep -E '^inclusion-mutuelle' | grep -E 'data_init.pico8_research$|research.pico8_research_display$|slipway.pico8_wormhole$'` → **vide** ; `--check inclusion-mutuelle` → `45 ligne(s)`.
7. `grep -n '#include "pico8_slipway.h"' src/pico8_wormhole.c` → exactement `15:` (gardé, voir décisions).

Après 2.2 :

8. `grep -nE '^#include "pico8_(game_data|map_tables|data_init)\.h"' src/pico8_struct.h src/pico8_ke.h src/pico8_entity.h` → **vide**.
9. `grep -n 'pico8_str_seq_t\|pico8_kind_rec_t' src/pico8_struct.h` → **vide**.
10. `grep -rnE 'struct pico8_(str_seq|kind_rec)\b' src tests` → vide **hors** 6 lignes : `game_data.h:32`, `:45` (étiquettes), `struct.h`, déclarations anticipées (2), `get_nu`, `rec`.
11. `grep -n '^#include "' src/pico8_struct.h` → exactement `coverage`, `fixed`, `geom`, `vec`.
12. `grep -n 'COMPOSITION_RESIDUE_GUARD\|composition-guard' Makefile` → exactement 7 lignes : légende de `test` et de `composition-guard`, `test:`, la variable, la cible, sa recette, `.PHONY`.
13. `make composition-residue` → aucune ligne `noyau-inclut` ; pied `résidu 2 : 1362 ligne(s), 93 fichier(s)` ; les dix autres catégories aux comptes de 2.1.
14. `git diff --stat <parent>` : 2.1 → `src/pico8_research.c`, `src/pico8_slipway.c`, `docs/plan-3.md` ; 2.2 → `src/pico8_struct.h`, `src/pico8_game_data.h`, `Makefile`, `docs/plan-3.md`. Aucun autre chemin.

### Budget et tableau « Runs »

Budget : **≈ 30 min par commit**, preuves comprises ; plafond 45 min. Au-delà, rien n'est commis et l'implémenteur note ce qui manque. Jamais l'appli raylib ; `make raylib` non exigé (pas de vendoring ici).

L'implémenteur tient un tableau « Runs », une ligne par lancement. Voici celui des essais de ce PRD, sur copie (à refaire sur le vrai arbre) :

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make composition-residue` | `91197eb` | 0 | `inclusion-mutuelle 21 21`, `noyau-inclut 1 1`, pied 1370 / 96 | < 1 s |
| 2 | `--check inclusion-mutuelle,noyau-inclut` | `91197eb` | 1 | 53 lignes (52 + 1) | < 1 s |
| 3 | fermeture et identifiants, 3 includes | `91197eb` | 0 | employés : aucun (témoins `planet.h`, `collect.h` mordent) | secondes |
| 4 | `cc -c`, ligne vidée, `research`/`slipway` | 2.1 | 0 | `.o` identiques octet à octet | secondes |
| 5 | build des 150 objets sans `-g`, `91197eb` / 2.1 | — | 0 | seuls `research.o`, `slipway.o` diffèrent ; `nm` identique ; 11 et 8 immédiats `__LINE__` | ~1 min |
| 6 | `make composition-residue` | 2.1 | 0 | `inclusion-mutuelle 18 18`, pied 1363 / 93 ; `--check` 45 | < 1 s |
| 7 | `make test` | 2.1 | 0 | 71 « all tests passed » | 8 s |
| 8 | `make e2e` | 2.1 | 0 | 26 conformes | 1,3 s |
| 9 | build des 152 objets, 2.1 / 2.2, et 74 sous `-DPICO8_CLASS_COVERAGE` | — | 0 | tous identiques octet à octet | ~1 min |
| 10 | `-MM -MG` avant/après, `-H`, préprocesseur trié | 2.2 | 0 | 14 fermetures perdent `game_data.h`, 0 emploi ; 6 lignes gagnées / 2 perdues | ~1 min |
| 11 | `make test` | 2.2 | 0 | `résidu gardé (noyau-inclut) : 0 ligne(s)` | 8 s |
| 12 | `make e2e` | 2.2 | 0 | 26 conformes | 1,3 s |
| 13 | M1 puis `make test` | 2.2 + M1 | 2 | `composition-guard` Error 1 | 8 s |
| 14 | `make composition-residue` | 2.2 | 0 | pas de `noyau-inclut`, pied 1362 / 93 | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas `91197eb` ou un descendant qui ne touche que `docs/`, ou si les comptes de départ diffèrent sans explication.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) reste rouge après 3 tentatives, si un `*.sha256` ou `tests/e2e/` change, ou si une constante d'empreinte change.
- `FAILED:` si un commit touche un chemin hors de la grep 14, ou si `src/` gagne une ligne autre que celles nommées ici.
- `FAILED:` si un objet sans `-g` diffère autrement que par les immédiats `__LINE__` déclarés (2.1), ou diffère du tout (2.2) ; si `nm` diffère.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si `inclusion-mutuelle` ne vaut pas 18 / 45 après 2.1, `noyau-inclut` 0 après 2.2, ou si le pied n'est pas 1363 / 93 puis 1362 / 93, sans explication.
- `FAILED:` si `COMPOSITION_RESIDUE_GUARD` ne vaut pas `noyau-inclut`, si `composition-guard` n'est pas un prérequis de `test`, ou si M1 ne rougit pas `make test`.
- `FAILED:` en cas de tassement (voir « Anti-tassement »), ou si un include direct est ajouté ailleurs que dans `struct.h`.
- `FAILED:` si le message d'un commit omet les includes retirés (fichier:ligne), le tableau des identifiants employés, le rapport objets/`nm`/préprocesseur, la liste des chemins de 2.2 (pour 2.2), M1/M2 (pour 2.2), ou le tableau « Runs ».
- `FAILED:` s'il manque une note « Découvert à », ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- `wormhole.c:15` (`#include "pico8_slipway.h"`), mort aussi : il n'est pas retiré ici (question 1).
- Les 18 paires restantes : l'étude C (2.11), puis 2.17. `entity` ↔ `ke` : 2.3a.
- Ajouter l'include direct de `game_data.h` aux 38 fichiers dont le premier chemin passait par `struct.h` (question 2).
- `pico8_coverage.h`, que `struct.h` inclut pour ses clients sans l'employer lui-même.
- Toute autre catégorie de 2.0, et toute modification de `tools/composition_residue.sh` ou de son TU.
- `make raylib`, l'appli raylib, `class-coverage-net`, `sanitize-net`, l'audio.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **Les trois includes, pas quatre.** Le plan laisse `wormhole.c:15` libre, et la commande demande les trois. Le compte est le même dans les deux cas : la ligne quitte le compte dès que la paire tombe. On le garde.
- **Sens de la règle de D.13.** « Perdre un chemin », pour un fichier, c'est :
  - perdre une de ses propres lignes d'include : `research.c`, `slipway.c`, `struct.h` ;
  - ou voir un en-tête quitter sa fermeture : les 14 fichiers.

  Un fichier qui garde l'en-tête par un autre de ses includes n'a pas perdu de chemin : ce sont les 38. `struct.h` gagne donc `geom.h` et `fixed.h` ; personne d'autre ne gagne rien.
- **`fixed.h` dans `struct.h`** : non prévu par le plan, mais c'est l'application de la règle. `pico8_fixed_t` venait d'abord par `game_data.h`. L'objet ne change pas (include gardé par `#ifndef`).
- **Étiquettes de struct plutôt que typedef répété** : C99 interdit de redéfinir un typedef. L'étiquette est la forme que `struct.h` emploie déjà (`pico8_qy`, `pico8_emitter`, `pico8_probe`).
- **Preuve d'objets de 2.1** : « identiques » ne peut pas valoir à la lettre, puisque les `assert` embarquent `__LINE__`. Deux preuves la remplacent : la variante « ligne vidée », identique octet à octet, et la variante supprimée, où seuls diffèrent les immédiats `__LINE__` (nombre déclaré) et où `nm` est identique.
- **Garde sans branche « vide »** : `data-guard` en a une, parce que sa variable a pu être vide ; celle-ci ne l'est jamais.

### Surprises de mesure, pour `plan-pilot`

1. **Cinq fichiers non compilables sans flag, pas quatre** : `tests/test_class_coverage.c` s'ajoute aux quatre de `src/`. Sans effet sur l'étape.
2. **Les objets de 2.1 ne sont pas identiques à la lettre** (`__LINE__` des `assert`). Toute étape « refactor pur » qui retire ou ajoute une ligne dans un `.c` aura le même écart ; la variante « ligne vidée » est la preuve à reprendre.
3. **`game_data.h` arrive par des chemins longs** dans 38 fichiers (souvent `cart.h > ui_button.h > planet.h`). 2.14 à 2.17 touchent ces en-têtes : une coupe peut faire perdre `game_data.h` à un TU, qui ne compilera plus. L'erreur sera visible, pas silencieuse.

### Questions pour l'humain

1. Un quatrième include est inutile : le fichier du trou de ver inclut celui des couloirs sans rien y lire. Le laisser ne change aucun compte. On le laisse pour l'étude des inclusions (2.11). Faut-il plutôt l'enlever dès 2.1 ?
2. Après 2.2, 38 fichiers reçoivent les données de jeu par un détour : un en-tête qui en inclut un autre, qui les inclut. Ils compilent, et rien ne change dans le binaire. Faut-il leur ajouter l'include direct maintenant, soit 38 lignes, ou attendre que les étapes suivantes cassent un détour, l'erreur de compilation le montrant alors ? Recommandation : attendre.
