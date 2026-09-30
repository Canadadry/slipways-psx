---
title: "Plan 3, étapes 2.17a et 2.17b : dispatchers et clic rangés avec l'affichage (T1) ; six paires d'inclusion coupées par l'usage, `inclusion-mutuelle` 15 → 12 → 9 paires — deux commits, corps déplacés identiques"
description: "2.17a : `pico8_mz_kj_dispatch`, `pico8_jc_kj_dispatch`, `pico8_pj_hk_dispatch`, `pico8_pj_ph_dispatch` passent dans leur `*_display.c` (déclarés dans `wormhole.h`, `slipway.h`, `score_panel.h`), les prototypes d'init/shutdown de `score_panel_display` dans `score_panel.h` ; 2.17b : `pico8_jo_hb/gv/kj/ldown_dispatch` et `pico8_jo_ldown` dans `planet_display.c`, `pico8_jo_kf/cn_dispatch` dans `planet_trade.c`. Mesuré sur l'arbre d'après 2.16 (rejoué par `run216.py`, hachages vérifiés) : `inclusion-mutuelle` 15 / 15 (39 lignes, 24 fichiers) → 12 / 11 (30, 17) → 9 / 9 (22, 13) ; graphe d'usage 18 → 15 → 12 ; aucune paire entrante ; relevé d'usage vide dans chaque sens coupé. Objets : sous -O0 et -O1 `-DNDEBUG`, 11 corps déplacés identiques à l'adresse près, toutes les autres fonctions et relocations de données identiques ; 14 jumelles sans ÉCHEC ; L 13 et 31, lignes neuves toutes couvertes. Aucune question à l'humain."
status: needs-triage
---

## Problem Statement

**Écrit sur `1ab94d6`** (branche `claude/plan-pilot-3-7zdft0`). Départ réel attendu : **le commit de 2.16**, dont l'implémentation était en cours dans l'arbre de travail du dépôt pendant l'écriture (fichiers non commités, non lus). L'arbre des essais est `1ab94d6` + `run216.py` (annexe A du PRD 2.16, `docs/prd/triage/plan3-phase2-etape16-void-ptr.md`), rejoué sur une copie : `2.16 : 18 remplacements, 21 occurrences`, arbres **`src` `148603f3…`, `tests` `cac32597…`, `Makefile` `422546ca…`**, empreinte du diff `efb84dbc…` — exactement ceux que le PRD 2.16 annonce. Pendant l'écriture, 2.16 a été commitée (`9794a11`, notée à `f938bfb`) : ses arbres sont ces trois-là, le départ est donc `9794a11` ou un descendant qui ne touche que `docs/`. Notation : `x` = le commit de la copie qui porte 2.16 ; `a` = 2.17a ; `b` = 2.17b. Numéros de ligne : ceux de `x` pour 2.17a, de `a` pour 2.17b, sauf mention. Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier ; aucun `make` lancé dans le dépôt. Essais sur une seule copie jetable (scratchpad `prd217ab/`, clone, arbres de travail `wt_p` = `x`, `wt_a`, `wt_b`, `c_p`/`c_a`/`c_b` propres pour la couverture, `wt_r` pour les rejeux), supprimée depuis.

Vu du mainteneur du port : après 2.16, le pied du compteur de composition ne compte plus que `inclusion-mutuelle` — **15 paires de modules qui s'emploient l'un l'autre, 39 lignes `#include`, 24 fichiers**. Six de ces paires ne tiennent qu'à un détail de rangement : la table de classe d'un module pointe un dispatcher qui n'est qu'un appel à la fonction d'affichage (ou de commerce) de l'autre module, et c'est ce dispatcher, défini du mauvais côté, qui oblige le module de l'entité à inclure son module d'affichage. Pour la planète, s'y ajoute la méthode de clic `pico8_jo_ldown`, qui ouvre la fiche (`pico8_jo_fe`, affichage) ou lance une voie (`pico8_jc_new`, `slipway`). Ranger ces définitions avec ce qu'elles appellent coupe les six paires **par l'usage** : dans le sens coupé, plus aucun nom de l'autre en-tête n'est employé, et plus d'include.

### Texte du plan, recopié (`docs/plan-3.md` à `1ab94d6`)

> - 2.17 — `inclusion-mutuelle`, selon l'étude 2.11 (`docs/etude-2-11-inclusions.md`, §2 à §4) : les quinze paires qui restent après 2.14b et 2.15d, et les trois paires d'usage cachées, coupées **par l'usage** (dans un sens, plus aucun nom de l'autre en-tête employé, et plus d'include) ; chaque sous-étape relance au départ et au commit le graphe d'usage de l'étude (annexe A) : les paires attendues sortent, aucune n'entre ; une ligne retirée qui compile par un détour est un `FAILED:`. Tout 2.17 passe après 2.16 : `pico8_hy_t` et `pico8_probe_t` changent en 2.12/2.13, et les corps déplacés changeraient les numéros de ligne que 2.12 à 2.16 citent. Pied : les autres catégories étant à 0, il ne compte que les lignes d'`inclusion-mutuelle` (lignes / fichiers). (2.9 : les archétypes, dans `game_data.h/.c`, n'ajoutent aucune paire ; essai sur copie. 2.10 : table du sort des symboles, §4.) `class-coverage-net` : non à chaque sous-étape (aucune table ni constructeur ne bouge). `sanitize-net` : non. Budget ≈ 6 × 50 min.
>   - 2.17a — T1, trois modules d'affichage : `pico8_mz_kj_dispatch`, `pico8_jc_kj_dispatch`, `pico8_pj_hk_dispatch`/`pico8_pj_ph_dispatch` dans leur `*_display.c`, déclarés dans `wormhole.h`, `slipway.h`, `score_panel.h` ; prototypes de `pico8_score_panel_display_init`/`_shutdown` dans `score_panel.h` ; retire `wormhole.c:15`, `slipway.c:18`, `score_panel.c:21`. Compte : 15 / 38 → **12 / 29** (16 fichiers) ; usage 18 → 15. Preuves : corps déplacés identiques à l'adresse près, `nm` ne diffère que par la liaison des quatre dispatchers (`t` → `T`).
>   - 2.17b — T1, planète : `pico8_jo_hb_dispatch`, `_gv_dispatch`, `_kj_dispatch`, `_ldown_dispatch` et `pico8_jo_ldown` dans `planet_display.c`, `pico8_jo_kf_dispatch`, `_cn_dispatch` dans `planet_trade.c` (déjà déclarés dans `planet.h`) ; retire `planet.c:18`, `:20`, `:23`. Compte : → **9 / 21** (12) ; usage → 12. Preuves : corps identiques à l'adresse près ; `nm` : sept symboles changent de module.

**Comptes du plan remesurés.** L'étude comptait en « paires / lignes » depuis `9ca66f4` (16 / 40 → 15 / 38 → 12 / 29 → 9 / 21) ; 2.14 à 2.16 ont fait tomber les deux paires prévues (`particle` ↔ `probe` à 2.14b, `planet_display` ↔ `planet_family` à 2.15d), mais **2.15c a ajouté la ligne `planet.h:11` → `probe.h`** (la planète porte désormais `pico8_probe_t probe_own` : 40 → 41 lignes, puis 41 → 39 à 2.15d). Mesures réelles, comptées comme le compteur les rend (`inclusion-mutuelle <paires> <modules>` au `--detail`/pied, lignes par `--check`, fichiers au pied) :

| Après | Paires | Modules | Lignes `#include` | Fichiers (pied) | Graphe d'usage | Plan / étude |
|---|---|---|---|---|---|---|
| 2.16 (`x`, départ) | 15 | 15 | 39 | 24 | 18 | 15 / 38, usage 18 |
| 2.17a | **12** | **11** | **30** | **17** | **15** | 12 / 29 (16 fichiers), 15 |
| 2.17b | **9** | **9** | **22** | **13** | **12** | 9 / 21 (12 fichiers), 12 |

La ligne de plus à chaque bout est `planet.h:11` ; elle appartient à `planet` ↔ `probe`, que 2.17d coupe (voir « Contrat »). **Réserve du vérificateur de l'étude** (message de `0acfad8`, « 2.17a : relire la colonne usage, projetée ») : la colonne usage est ici **mesurée** (`usegraph.py`, `paires 18` → `15` → `12`), et chaque sens coupé a son relevé de noms, vide après la coupe (preuve 2).

Numéros du plan datés : à `x`, les includes retirés par 2.17a sont `wormhole.c:15`, `slipway.c:17` (plan : `:18`, une ligne au-dessus est partie depuis `9ca66f4`), `score_panel.c:21` ; à `a`, ceux de 2.17b sont `planet.c:18`, `:20`, `:23` (inchangés).

Étude 2.11 (fiches 5, 7, 9, 14, 15, 17 ; §4) : technique T1 (« la définition passe dans `X_display.c`, la déclaration dans `X.h` ; la table de classe reste où elle est et pointe le même nom ») ; fiche 5 : « `pico8_jo_ldown` avec son dispatcher passent dans `planet_display.c` […] déplacer `ldown` règle aussi la paire 9 » ; fiche 14 : « l'appel d'init reste dans `pico8_score_panel_init` : l'ordre d'init ne change pas » ; piège 5 : « pour un corps déplacé, la preuve est "corps identiques à l'adresse près, `nm` ne diffère que par la liaison du symbole déplacé" ». Essais seuls de l'étude (`t1_mz`, `t1_jc`, `t1_pj`, `t1_jo_ui`, `t1_jo_trade`, annexe A) : ce PRD en reprend la forme, rejouée sur `x`.

Décisions de l'humain (même fichier) : **Q2** — « les paires d'inclusion mutuelle sont démêlées pendant la phase 2 ; `inclusion-mutuelle` est une condition de sortie, à zéro » : 2.17a et 2.17b en retirent six. Q1, Q3, Q4, **planètes A**, Q-2.10, Q-2.14d : sans objet ici (aucun champ, lien ni type ne bouge ; les trois paires qui dépendent de Q-2.10 sont celles de 2.17e).

Règles communes du plan : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée ; `class-coverage-net` et `sanitize-net` « non » (lancés quand même, verts). Ce PRD les tient toutes.

### Contrats reçus et ce que ce PRD en fait

| Point (source) | Ce que font 2.17a et 2.17b |
|---|---|
| coupe **par l'usage**, plus d'include ; un détour est un `FAILED:` (plan, étude §0) | relevé `usedir.py` de chaque sens coupé : **0 nom** (seul déclarant), **0 nom partagé**, **0 include** au commit (preuve 2) ; `cross.py` ne rend plus aucune ligne du sens coupé |
| graphe d'usage relancé au départ et au commit, aucune paire n'entre (plan) | `usegraph.py` : 18 → 15 → 12, seules sortent les paires visées ; `--detail` idem ; arêtes orientées : seules entrent des arêtes vers `dispatch_context` (preuve 3) |
| « corps déplacés identiques à l'adresse près, `nm` : `t` → `T` » (2.17a) ; « sept symboles changent de module » (2.17b) | `fnbody.py` (annexe A) : 4 et 7 corps identiques sous `-O0`/`-O1 -DNDEBUG` ; liaisons `l` → `g` (2.17a), `g` → `g` (2.17b) ; toutes les autres fonctions et toutes les relocations de données identiques (preuve 4) |
| la table de classe reste où elle est et pointe le même nom (étude, T1) | aucune table ne bouge ; relocations des tables résolues : mêmes fonctions cibles (preuve 4) |
| init/shutdown de `score_panel_display` : l'appel reste dans `pico8_score_panel_init` (fiche 14) | tenu : seuls les deux prototypes changent d'en-tête |
| « déjà déclarés dans `planet.h` » (2.17b) | tenu : `planet.h` ne change pas ; aucun en-tête ne change en 2.17b |
| `class-coverage-net`, `sanitize-net` : non (plan) | lancés : `couverture: OK`, `sanitize: 26 propre(s)` à `a` et `b` |
| réserve de `0acfad8` : relire la colonne usage (2.17a) | mesurée, voir la table ci-dessus |

### État de départ attendu (le commit de 2.16)

Mesuré sur `x` ; identique aux comptes d'arrivée du PRD 2.16 :

| Commande | Résultat |
|---|---|
| `git rev-parse HEAD:src HEAD:tests` ; `git ls-files -s Makefile` | `148603f32a11ad8a77bcb8384deb2512a2a9935b`, `cac3259733163ed40d59198d02463eb9618e13a2` ; `422546ca868cfbdd089c8348b8eb4806763b9010` |
| `make composition-residue` | `inclusion-mutuelle 15 15` seule ; **`résidu 2 : 39 ligne(s), 24 fichier(s)`** |
| `sh tools/composition_residue.sh --check inclusion-mutuelle src tests \| tail -1` | `résidu gardé (inclusion-mutuelle) : 39 ligne(s)` |
| `--champs \| tail -1` | `champs : 238 ; 0 écriture : 14 ; 0 hors constructeur : 19` |
| casts / tests de classe (greps de 2.15d) | 464 / 647 |
| graphe d'usage (`usegraph.py`, annexe A du PRD 2.11) | `paires 18` |
| `COMPOSITION_RESIDUE_GUARD` | 12 catégories, finit par `,parente,void-ptr` |
| `make test` ; `make e2e` ; `MALLOC_PERTURB_=165 make e2e` | vert ; 26 conformes ; 26 conformes |

Autre départ (2.16 commitée autrement que par son script, ou suivie d'un commit qui touche `src/`) : `run217a.py` tourne quand même tant que chaque texte cherché est présent (il s'arrête sinon, en nommant le texte) ; les hachages d'arrivée ne valent plus ; comparer alors tous les comptes et rapporter l'écart (repli 2).

## Solution

Deux commits, chacun vert, jeu identique (26 e2e conformes, aussi sous `MALLOC_PERTURB_=165`), aucune constante d'empreinte changée. Chaque commit est le résultat de son script (annexe A, texte exact), plus la note du plan et le message :

**2.17a** (`run217a.py` ; diff de 195 lignes, 10 fichiers, +32 / −27) :
1. `pico8_mz_kj_dispatch` quitte `wormhole.c:187-190`, est ajouté **en fin de `wormhole_display.c`** sans `static` ; prototype ajouté en fin de `wormhole.h` ; `wormhole_display.c` inclut `pico8_dispatch_context.h` (il appelle `pico8_dispatch_gfx`/`_mem`) ; `wormhole.c` perd `#include "pico8_wormhole_display.h"` (`:15`) **et** `#include "pico8_dispatch_context.h"` (`:8`, qu'il n'employait plus que dans ce dispatcher : G2).
2. `pico8_jc_kj_dispatch` quitte `slipway.c:332-335` pour la fin de `slipway_display.c`, sans `static` ; prototype en fin de `slipway.h` ; `slipway_display.c` inclut `pico8_dispatch_context.h` ; `slipway.c` perd `#include "pico8_slipway_display.h"` (`:17`) et garde `dispatch_context.h` (employé à `:151`, `:314`).
3. `pico8_pj_hk_dispatch`, `pico8_pj_ph_dispatch` quittent `score_panel.c:265-273` pour la fin de `score_panel_display.c`, sans `static` ; leurs prototypes et ceux de `pico8_score_panel_display_init`/`_shutdown` (retirés de `score_panel_display.h:18-19`) vont en fin de `score_panel.h` ; `score_panel.c` perd `#include "pico8_score_panel_display.h"` (`:21`). L'appel `pico8_score_panel_display_init()` reste dans `pico8_score_panel_init` (`score_panel.c:339`).

**2.17b** (`run217b.py` ; diff de 163 lignes, 3 fichiers, +51 / −55) :
4. `pico8_jo_hb_dispatch`, `pico8_jo_gv_dispatch`, `pico8_jo_kj_dispatch`, `pico8_jo_ldown_dispatch` (`planet.c:39-61`) et `pico8_jo_ldown` (`:437-459`, dernière fonction du fichier) sont ajoutés, dans cet ordre, **en fin de `planet_display.c`**, qui inclut déjà `dispatch_context.h`, `planet.h`, `slipway.h`, `<assert.h>`, `<string.h>`.
5. `pico8_jo_kf_dispatch`, `pico8_jo_cn_dispatch` (`planet.c:67-73`) passent en fin de `planet_trade.c` (qui inclut `planet.h`).
6. `planet.c` perd `#include "pico8_planet_display.h"` (`:18`), `"pico8_planet_trade.h"` (`:20`), `"pico8_slipway.h"` (`:23`) **et** `"pico8_dispatch_context.h"` (`:10`, employé seulement par `pico8_jo_kj_dispatch` : G2). Tous ces noms sont déjà déclarés dans `planet.h` (`:105-110`, `:115`, `:143`) : aucun en-tête ne change.

Rien d'autre : aucune table de classe, aucun nom, aucun corps, aucun commentaire, ni `tests/`, ni `tools/`, ni le `Makefile`, ni `*.sha256`, ni `docs/glossaire-noms.md`, ni `docs/trous-de-portage.md`. Les prototypes ajoutés n'ont pas de commentaire (règle des quatre destinations : ils ne portent rien que le nom ne dise).

## User Stories

1. En tant que mainteneur, je veux que `wormhole`, `slipway`, `score_panel` et `planet` n'emploient plus rien de leur module d'affichage (ni `planet` de `planet_trade` et `slipway`), afin que l'affichage dépende de l'entité et jamais l'inverse.
2. En tant que gardien du compteur, je veux `inclusion-mutuelle` à 12 puis 9 paires, chaque coupe prouvée par l'usage (relevé vide) et non par le seul retrait d'une ligne `#include`.
3. En tant que gardien de la règle du parent, je veux que chaque corps déplacé soit le même code machine qu'avant, et que chaque mutation rouge au parent le soit à l'enfant.
4. En tant qu'auteur de 2.17c à 2.17f, je veux les numéros de ligne et les comptes d'arrivée réels, dont la ligne `planet.h:11` que l'étude ne connaissait pas.

## Implementation Decisions

### Modules

2.17a : `wormhole.c/.h`, `wormhole_display.c` ; `slipway.c/.h`, `slipway_display.c` ; `score_panel.c/.h`, `score_panel_display.c/.h` ; `docs/plan-3.md` (note « Découvert à 2.17a »). 2.17b : `planet.c`, `planet_display.c`, `planet_trade.c` ; `docs/plan-3.md` (note « Découvert à 2.17b »).

### Inclusions

| Commit | Retirées | Ajoutées |
|---|---|---|
| 2.17a | `wormhole.c:8` (`dispatch_context.h`, G2), `wormhole.c:15` (`wormhole_display.h`), `slipway.c:17` (`slipway_display.h`), `score_panel.c:21` (`score_panel_display.h`) | `wormhole_display.c`, `slipway_display.c` : `pico8_dispatch_context.h` (avant `pico8_entity.h`, ordre alphabétique) |
| 2.17b | `planet.c:10` (`dispatch_context.h`, G2), `:18` (`planet_display.h`), `:20` (`planet_trade.h`), `:23` (`slipway.h`) | aucune |

**Aucun include mort créé** (`deadinc.py`, annexe A : pour chaque `#include "pico8_X.h"` des neuf `.c` touchés, noms de `X.h` employés) : la liste des includes à 0 nom à `a` est celle de `x` (13 lignes préexistantes, hors périmètre) ; à `b`, elle perd une ligne (`planet_display.c` → `dispatch_context.h`, désormais employé). Les deux `dispatch_context.h` retirés passeraient sinon à 0 nom.

### Ce qui change

Texte exact : résultat des scripts. `python3 run217a.py <arbre>` sur un arbre propre au commit de 2.16 (arbres de la table « État de départ ») rend, mesuré, après `git add -A` : **`src` `e8e7684016bdb3c24e21b08a352058f108f908b0`**, `tests` et `Makefile` inchangés ; `git diff x a | sha256sum` → `90747520…` (195 lignes). `python3 run217b.py <arbre>` sur `a` : **`src` `3c5385b892f44317b1a42cae2108b40bf727e5d9`** ; `git diff a b | sha256sum` → `548da3b7…` (163 lignes). Rejoués sur un arbre de travail neuf à `x` : mêmes hachages ; relancer `run217b.py` sur `b` s'arrête (`AssertionError`, texte absent), le script n'est pas rejouable deux fois (Runs 18). Les deux diffs complets sont en annexe B.

**Corps** : copiés au caractère près (le script coupe le texte exact de la définition et l'ajoute tel quel) ; seul `static ` part pour les quatre dispatchers de 2.17a. **Casts** 464 et **tests de classe** 647, inchangés aux trois arbres (mêmes lignes, déplacées).

### Comptes, mesurés sur la copie

| Mesure | `x` | `a` | `b` |
|---|---|---|---|
| `inclusion-mutuelle` (paires / modules) | 15 / 15 | **12 / 11** | **9 / 9** |
| lignes du `--check inclusion-mutuelle` | 39 | **30** | **22** |
| pied `résidu 2` (lignes / fichiers) | 39 / 24 | **30 / 17** | **22 / 13** |
| graphe d'usage (`usegraph.py`) | 18 | **15** | **12** |
| arêtes orientées (`usegraph_e.py`) ; composantes fortement connexes > 1 | 622 ; 26 + 2 | 620 ; 24 | 617 ; 24 |
| lignes `cross.py` (une par include de paire) | 39 | 30 | 22 |
| autres catégories | 0, 12 gardées | 0, 12 gardées | 0, 12 gardées |
| `--champs` ; casts ; tests de classe | 238 ; 14 ; 19 ; 464 ; 647 | idem | idem |
| `COMPOSITION_RESIDUE_GUARD` | 12 catégories | idem | idem |
| constantes `*_HASH`/`FROZEN_*` changées | — | 0 | 0 |

**Paires sortantes** — 2.17a : `score_panel` ↔ `score_panel_display`, `slipway` ↔ `slipway_display`, `wormhole` ↔ `wormhole_display` (compteur et usage). 2.17b : `planet` ↔ `planet_display`, `planet` ↔ `planet_trade`, `planet` ↔ `slipway` (compteur et usage). **Paires entrantes : aucune**, au compteur comme à l'usage. Lignes qui sortent du `--check` : 2.17a, les trois lignes de chaque paire (`wormhole.c:15`, `wormhole_display.c:9`, `wormhole_display.h:8` ; `slipway.c:17`, `slipway_display.c:10`, `slipway_display.h:8` ; `score_panel.c:21`, `score_panel_display.c:14`, `score_panel_display.h:6`) = 9 ; 2.17b : `planet.c:18`, `:20`, `:23`, `planet_display.c:17`, `planet_display.h:7`, `planet_trade.c:10`, `planet_trade.h:6`, `slipway.c:15` (`planet.h`, ligne gardée : `slipway` → `planet` n'est plus une paire) = 8. Fichiers : 2.17a, sortent les six fichiers des trois `*_display` et `score_panel.c` (24 − 7 = 17) ; 2.17b, sortent `planet_display.c/.h`, `planet_trade.c/.h` (17 − 4 = 13).

### Contrat pour 2.17c à 2.17f (à recopier dans leurs PRD)

| Point | Pour | Ce qu'il faut savoir |
|---|---|---|
| départ : `inclusion-mutuelle` **9 / 9, 22 lignes / 13 fichiers**, usage **12** | 2.17c | les comptes de l'étude (§3) sont à relire avec **une ligne de plus** tant que `planet.h:11` reste |
| **`planet.h:11` → `probe.h`** (ligne entrée à 2.15c, `pico8_probe_t probe_own` dans `pico8_jo_t`) ; `cross.py` : `planet.h` n'y emploie que `pico8_probe_t` ; `planet.c:18` → `probe.h` n'emploie que `pico8_fm_hy` | 2.17d | l'étude (fiche 8) ne coupait `planet` → `probe` que par `pico8_fm_hy` (T2f) ; la coupe exige aussi que `planet.h` inclue `pico8_probe_comp.h` au lieu de `pico8_probe.h` (même technique que `slipway.h:7`, `wormhole.h:11`) ; usage de `planet` → `probe` à `b` : `pico8_fm_hy`, `pico8_probe_t` |
| numéros à `b` : `planet.c:10` `economy.h`, `:12` `entity_geom.h`, `:17` `planet_family.h` (étude : `:19`), `:18` `probe.h` (étude : `:21`) ; `slipway.c:10` `economy_formulas.h`, `:16` `probe.h` (étude : `:17`), `:18` `ui.h` ; `wormhole.c:12` `probe.h` (étude : `:13`) ; `pico8_dj` appelé à `slipway.c:120` ; `g_kv`/`jo_kv` à `planet.c:283-297`, `pico8_hy_nx` à `:332` ; `economy.c`, `economy_formulas.c`, `entity_geom.c`, `probe.c`, `ui.c` inchangés | 2.17c–e | 2.17a/b ne touchent aucun de ces fichiers sauf `planet.c`, `slipway.c`, `wormhole.c` |
| `pico8_dispatch_context.h` n'est plus inclus par `wormhole.c` ni `planet.c` | 2.17f | le portage `awk` du graphe d'usage doit rendre les mêmes 12 paires que `usegraph.py` à `b` |
| les dispatchers déplacés gardent leur nom en `_dispatch` : `void-ptr` reste 0 | toutes | ne pas les renommer |

### Anti-tassement

**Interdit** :
- retirer une ligne `#include` sans retirer l'usage (un détour, que le relevé `usedir.py` doit rendre vide), ou redéclarer dans le `.c` de l'entité un nom de l'en-tête d'affichage (prototype local, `extern`) pour se passer de l'include ;
- déplacer une table de classe, renommer un dispatcher ou une fonction, fondre deux fonctions, changer un corps (hors `static ` des quatre dispatchers de 2.17a), déplacer `pico8_jo_hd_dispatch`, `_hk_dispatch`, `_ll_dispatch`, `pico8_jo_is_family` (hors T1 : ils n'appellent rien de l'affichage) ;
- ajouter un commentaire aux prototypes, en retirer un, toucher `planet.h` ;
- toucher `tests/`, `tools/`, le `Makefile`, un `*.sha256`, une constante `*_HASH`/`FROZEN_*`, `docs/glossaire-noms.md`, `docs/trous-de-portage.md` ;
- faire 2.17a et 2.17b dans un seul commit ; réécrire à la main ce que les scripts font, sauf repli.

## Implementation Chunks

Deux commits, **≈ 30 min chacun** preuves comprises, sous le budget de 50 min (mesuré sur la copie : script < 1 s ; `make -j4 test` + `make e2e` + e2e perturbé 12 à 18 s par arbre ; objets 4 configurations × 2 arbres ≈ 2 min ; jumelles 5 × 2 en 90 s et 9 × 2 en 110 s en parallèle ; couverture ≈ 10 s par arbre ; `class-coverage-net` + `sanitize-net` ≈ 15 s) :

- **2.17a** : copier `run217a.py` hors du dépôt, le lancer, comparer les hachages ; `make test`, `make e2e`, `MALLOC_PERTURB_=165 make e2e` ; comptes (`make composition-residue`, `--check`, `usegraph.py`, `usedir.py`, `cross.py`, `deadinc.py`) ; objets (`objs2.sh` ×4 à `x` et `a`, `fnbody.py`, `cmp`) ; jumelles (`mut217.py p` et `a`, 5 mutations) ; `line-coverage` des 26 e2e à `x` et `a` + `lset.py` ; `class-coverage-net`, `sanitize-net` ; note « Découvert à 2.17a », message.
- **2.17b** : idem avec `run217b.py`, parent `a`, 9 mutations.

**Replis écrits**, dans l'ordre :
1. si un script s'arrête sur un `AssertionError` (texte introuvable) : lire le site ; faire le même déplacement à la main **selon la même règle** (corps copié à l'identique, `static` retiré seulement pour les dispatchers de 2.17a, prototype en fin d'en-tête, include retiré seulement après relevé vide) ; noter chaque site dans le message ;
2. si le départ n'est pas l'arbre de 2.16 annoncé (hachages différents) : lancer les scripts quand même, ne pas comparer les hachages d'arrivée, comparer tous les comptes des tables et rapporter l'écart dans la note ;
3. si une erreur de compilation reste (`syn.sh` non vide) : c'est un nom que le corps déplacé emploie et que le fichier d'arrivée ne voit pas : ajouter l'include manquant **au fichier d'arrivée** (jamais remettre l'include retiré au fichier de départ), relancer `usegraph.py` : si une paire entre, arrêter (critère d'échec) ;
4. si le relevé d'un sens coupé n'est pas vide, ou si une paire entre : ne pas commiter ; arrêter et rapporter (pas de repli par un include de détour) ;
5. si 2.17b déborde de 1,5 × 50 min : la couper en **b1** (`planet_display` : quatre dispatchers, `pico8_jo_ldown`, includes `:10`, `:18`, `:23` ; mesuré : `inclusion-mutuelle` **10 / 10, 25 lignes / 15 fichiers**, usage **13**, 0 erreur) puis **b2** (`planet_trade` : deux dispatchers, include `:20` ; → 9 / 9, 22 / 13, usage 12). `run217b1.py` = `run217b.py` sans son dernier bloc (`# planet -> planet_trade`).

## Testing Decisions

- **Pas de TU neuf, aucun TU touché.** Aucun rejeu d'`events.json` dans un TU. Aucun `*_HASH` ne bouge.
- **Bon test ici** : un déplacement de définition se prouve par le code machine (même corps, mêmes cibles, mêmes tables) et par le graphe d'usage (le sens coupé n'emploie plus rien) ; le jeu, la couverture et les jumelles confirment que le chemin exécuté est le même.
- **Prior art** : PRD 2.16 (`fncmp.py`, jumelles, forme) ; PRD 2.15d (`syn.sh`, `objs.sh`, `mut.py`, `base.sh`) ; PRD 2.15c (`lset.py`, ensemble `L`) ; étude 2.11 (annexe A : `usegraph.py`, `cross.py`, `sites.py`, `cuts/t1_*.py`) ; règle du parent (D.21).

### Preuves (parent de `a` = `x`, parent de `b` = `a`)

1. **Communes** : `make -j4 test` vert ; `make e2e` **26 conformes** ; `MALLOC_PERTURB_=165 make e2e` **26 conformes**, aux trois arbres ; `git diff --stat x b -- tests tools Makefile '*.sha256'` vide (Runs 10).

2. **Coupe par l'usage, sens par sens** (`usedir.py`, annexe A : pour un sens A > B, noms que `B.h` est seul à déclarer et que `A.c`/`A.h` emploient, noms partagés avec un autre en-tête, lignes `#include "B.h"` de A) :

| Sens coupé | Au parent | Au commit |
|---|---|---|
| `wormhole` > `wormhole_display` | 1 : `pico8_mz_kj` ; include `wormhole.c` | **0 ; partagés 0 ; include aucun** |
| `slipway` > `slipway_display` | 1 : `pico8_jc_kj` ; include `slipway.c` | **0 ; 0 ; aucun** |
| `score_panel` > `score_panel_display` | 4 : `pico8_pj_hk`, `pico8_pj_ph`, `pico8_score_panel_display_init`, `_shutdown` ; include `score_panel.c` | **0 ; 0 ; aucun** |
| `planet` > `planet_display` | 4 : `pico8_jo_fe`, `pico8_jo_gv`, `pico8_jo_hb`, `pico8_jo_kj` ; include `planet.c` | **0 ; 0 ; aucun** |
| `planet` > `planet_trade` | 2 : `pico8_jo_cn`, `pico8_jo_kf` ; include `planet.c` | **0 ; 0 ; aucun** |
| `planet` > `slipway` | 3 : `pico8_jc_keys_t`, `pico8_jc_new`, `pico8_jc_ready` ; include `planet.c` | **0 ; 0 ; aucun** |

   Sens gardés (l'affichage emploie l'entité) : `wormhole_display` > `wormhole` 2 → 3 noms (+ `pico8_mz_kj_dispatch`), `slipway_display` > `slipway` 3 → 4, `score_panel_display` > `score_panel` 8 → 12, `planet_display` > `planet` 6 → 11, `planet_trade` > `planet` 3 → 5, `slipway` > `planet` 3 → 3. `pico8_jo_trade_proposal` (déclaré par `planet.h` et `planet_trade.h`, écarté par `usegraph.py`) : `cross.py` ne rend plus aucune ligne `planet.c` → `planet_trade.h` à `b` (Runs 6, 8, 16).

3. **Graphe d'usage et compteur, aucune paire entrante** : `usegraph.py` 18 → 15 → 12, `diff` des paires : seules des sorties ; `--detail` : seules des sorties ; arêtes orientées (`usegraph_e.py`) : à 2.17a sortent `score_panel` → `score_panel_display`, `slipway` → `slipway_display`, `wormhole` → `wormhole_display`, `wormhole` → `dispatch_context` et entrent `slipway_display` → `dispatch_context`, `wormhole_display` → `dispatch_context` ; à 2.17b sortent `planet` → `planet_display`, `planet_trade`, `slipway`, `dispatch_context` et entre `planet_display` → `dispatch_context` (`dispatch_context` n'emploie aucun de ces modules). `clang -std=c99 -Wall -Wextra -Werror -Isrc -fsyntax-only` sur `src/` et `tests/` : 0 erreur à `a` et `b` (Runs 6, 8, 17, 22).

4. **Objets** (`objs2.sh`, annexe A : les 74 `src/*.c` hors raylib, sans `-g`, `-DPICO8_CLASS_COVERAGE` ; `fnbody.py` compare **fonction par fonction où qu'elle soit définie** : `objdump -dr` sans adresses ni déplacements `%rip`, relocations de section résolues en fonction, variable ou texte de chaîne ; relocations des sections de données résolues de même) :

| Configuration | 2.17a (`x` → `a`) | 2.17b (`a` → `b`) |
|---|---|---|
| `-O0 -DNDEBUG` (celle de `make test`/`e2e`, sans `assert`) | **4 corps déplacés identiques** ; 835 autres fonctions identiques ; 1247 relocations de données identiques | **7 corps déplacés identiques** ; 832 identiques ; 1247 identiques |
| `-O0` avec `assert` | 12 fonctions ne diffèrent que par l'immédiat `__LINE__` (lignes décalées dans `slipway.c`, `slipway_display.c`, `wormhole.c`, `score_panel.c`) : 0 écart sous `NORM_ASSERT=1` | `pico8_jo_ldown` diffère par `__LINE__` et `__FILE__` de son `assert` ; 9 fonctions de `planet_display.c`/`planet.c` par `__LINE__` et la numérotation des statiques (`__PRETTY_FUNCTION__.n`, `gap.n`) : 0 écart sous `NORM_ASSERT=1` |
| `-O1 -DNDEBUG` | **identiques** (4 déplacés, 726 en place, données) | **identiques** (7, 723) |
| `-O2 -DNDEBUG` | les 4 déplacés diffèrent : le dispatcher, désormais dans l'unité de son appelé, l'intègre ; `pico8_jc_kj` est découpée (`pico8_jc_kj.part.0` neuf), `pico8_mz_kj` et `pico8_de_my` ne diffèrent que par un `nop` de bourrage ; données identiques | les 7 déplacés (intégration ou appel terminal local de leur appelé) ; `pico8_jo_fe` découpée (`pico8_jo_fe.part.0` neuf), `pico8_jo_gv` recompilée (elle a désormais un appelant dans son unité) ; `pico8_jo_hk_dispatch`, `pico8_jo_ib`, `pico8_jo_kj` par le seul bourrage `nop` ; données identiques |
| objets non touchés, octet pour octet (4 configurations) | 68 / 74 | 71 / 74 |
| liaison (`nm`) | `pico8_mz_kj_dispatch`, `pico8_jc_kj_dispatch`, `pico8_pj_hk_dispatch`, `pico8_pj_ph_dispatch` : `t` → `T`, et changent d'objet | les 7 : `T` → `T`, changent d'objet (`planet` → `planet_display` ×5, `planet_trade` ×2) |
| tables de classe | `g_mz_struct_class+0`, `g_jc_struct_class+0`, `g_pj_struct_class+24`, `+120` pointent la même fonction (relocation par symbole au lieu de `.text+off`) | inchangées |

   Lecture : sous la configuration du jeu (`-O0`) et à `-O1`, le code machine des 11 fonctions est le même, à l'adresse près ; le reste des objets ne change pas. `-O2` n'est pas la configuration du jeu (sauf `sanitize-net`, vert, preuve 8) : l'intégration d'un appel dans la même unité est l'effet attendu de T1, déclaré (Runs 11 à 13).

5. **Mutations, règle du parent** (`mut217.py` = `mut.py` du PRD 2.15d avec `S` et le nom du JSON changés ; `muts217.json`, écrit par `muts217gen.py`, annexe A). Même mutation, même texte, au parent et à l'enfant, sur chaque corps déplacé et sur l'appel d'init dont le prototype a changé d'en-tête. `make -k -j4 unit` puis `make -k e2e` ; restaurer par `git checkout -f`. **ÉCHEC si un TU rouge au parent n'est pas rouge à l'enfant, ou si l'e2e est < 26 au parent et 26 à l'enfant.**

| # | Cible (parent → enfant) | Mutation | Rouges au parent | Rouges à l'enfant | e2e | Verdict |
|---|---|---|---|---|---|---|
| MZ-kj | `pico8_mz_kj_dispatch` (`wormhole.c` → `wormhole_display.c`) | `pico8_vec_v(p.y, p.x)` au lieu de `p` | aucun | aucun | 8 / 8 | conforme |
| JC-kj | `pico8_jc_kj_dispatch` | appel remplacé par `(void)self;` | `jc` | `jc` | 17 / 17 | conforme |
| PJ-hk | `pico8_pj_hk_dispatch` | `yr + 1` | `pj` | `pj` | 21 / 21 | conforme |
| PJ-ph | `pico8_pj_ph_dispatch` | appel remplacé par `(void)self;` | `js`, `pj`, `replay_from_title` | les mêmes 3 | 23 / 23 | conforme |
| PJ-init | appel `pico8_score_panel_display_init()` (`score_panel.c`, des deux côtés) | ligne retirée | aucun | aucun | 26 / 26 | survivante aux deux bouts, déclarée |
| JO-hb | `pico8_jo_hb_dispatch` (`planet.c` → `planet_display.c`) | appel remplacé par `(void)self;` | `ga`, `hj`, `hp`, `lq`, `mx` | les mêmes 5 | 12 / 12 | conforme |
| JO-gv | `pico8_jo_gv_dispatch` | rend `{0, NULL}` | `ui` | `ui` | 16 / 16 | conforme |
| JO-kj | `pico8_jo_kj_dispatch` | `pico8_vec_v(p.y, p.x)` | `fr`, `g_`, `ga`, `hj`, `hp`, `jc`, `lq`, `mx`, `nr`, `pm` | les mêmes 10 | 0 / 0 | conforme |
| JO-ld | `pico8_jo_ldown_dispatch` | rend `{0, NULL}` | `jc`, `ui` | les mêmes 2 | 10 / 10 | conforme |
| LD-known | `pico8_jo_ldown`, branche `"known"` | `pico8_jo_fe(self);` retiré | `js` | `js` | 10 / 10 | conforme |
| LD-me | `pico8_jo_ldown`, test de `me` | `"knowm"` | `js`, `ui` | les mêmes 2 | 10 / 10 | conforme |
| LD-ly | `pico8_jo_ldown`, branche `"ly"` | `out.stop = 0;` | `jc`, `ui` | les mêmes 2 | 16 / 16 | conforme |
| JO-kf | `pico8_jo_kf_dispatch` (`planet.c` → `planet_trade.c`) | appel remplacé par `(void)self; (void)sw;` | `hp`, `hy`, `jc`, `js`, `lq`, `mx`, `trade`, `ui` | les mêmes 8 | 17 / 17 | conforme |
| JO-cn | `pico8_jo_cn_dispatch` | appel remplacé par `(void)self; (void)lw;` | `hp`, `hy`, `js`, `lq`, `nr`, `trade` | les mêmes 6 | 18 / 18 | conforme |

   Lecture : **aucun ÉCHEC** (14 jumelles, 28 lancements, aucune erreur de compilation, rouges et e2e identiques des deux côtés). MZ-kj ne rougit que des e2e (aucun TU ne dessine un `mz` par sa table) ; PJ-init survit aux deux bouts (`pico8_score_panel_display_init` ne remet à 0 que `g_pj_hk_lh_t`, déjà 0 au premier init de chaque TU et de chaque rejeu) : sans conséquence pour la règle, le parent est identique ; la preuve de ce chemin est l'objet (`score_panel.o` identique hors dispatchers, preuve 4). **Lignes K** : aucune, l'enfant n'a aucun chemin neuf (Runs 14, 15).

6. **Chemin touché, ensemble `L`** : `make line-coverage EVENTS="<les 26 e2e>"` à `x`, `a`, `b` : **6031** lignes chacun. `lset.py` (PRD 2.15c) :
   - 2.17a : **0 perdue, 0 gagnée** ; `L` **13** (`score_panel.c:265`, `:267-268`, `:270`, `:272-273`, `slipway.c:332`, `:334-335`, `wormhole.c:187-190`) ; lignes neuves couvertes **13 / 13 exécutables** (`score_panel_display.c:338`, `:340-341`, `:343`, `:345-346`, `slipway_display.c:175`, `:177-178`, `wormhole_display.c:78-81`) ; les neuves non couvertes sont des prototypes, deux includes, des `(void)x;` et des lignes vides.
   - 2.17b : `L` **31** ; neuves couvertes **32 / 32** ; « 1 perdue » (`planet.c:73` à `a`) : artefact d'appariement — c'est l'accolade fermante de `pico8_jo_cn_dispatch`, que `difflib` apparie à l'accolade de `pico8_jo_is_family` (`b:42`, jamais comptée par `gcov`) ; l'accolade déplacée, `planet_trade.c:255`, est couverte (32 = 31 + 1). Aucune ligne inchangée ne perd ni ne gagne sa couverture.
   (Runs 19, 20.)

7. **Reproductibilité** : `run217a.py` puis `run217b.py` sur un arbre de travail neuf à `x` → `e8e76840…`, `3c5385b8…` ; `git diff x b | sha256sum` `944b21ab…` ; second `run217b.py` : `AssertionError` (Runs 18).

8. `class-coverage-net` : **non** exigé ; lancé à `a` et `b` : `couverture: OK -- chaque classe non exemptée est instanciée (26 rejeu(x))`. `sanitize-net` : **non** exigé ; lancé : `sanitize: 26 propre(s)` aux deux (seule configuration `-O2` du dépôt) (Runs 21).

### Greps de Sortie

À `a` :
1. `grep -n 'pico8_\(mz\|jc\)_kj_dispatch\|pico8_pj_\(hk\|ph\)_dispatch' src/*.c src/*.h` → 12 lignes : 4 définitions (`score_panel_display.c:338`, `:343`, `slipway_display.c:175`, `wormhole_display.c:78`), 4 entrées de table (`score_panel.c:266`, `:268`, `slipway.c:336`, `wormhole.c:190`), 4 prototypes (`score_panel.h:122-123`, `slipway.h:126`, `wormhole.h:78`).
2. `grep -rn 'static .*pico8_\(mz\|jc\)_kj_dispatch\|static .*pico8_pj_\(hk\|ph\)_dispatch' src | wc -l` → **0**.
3. `grep -c 'pico8_score_panel_display_\(init\|shutdown\)' src/pico8_score_panel_display.h src/pico8_score_panel.h` → **0**, **2**.
4. `grep -c '_display\.h"' src/pico8_wormhole.c src/pico8_slipway.c src/pico8_score_panel.c` → **0** ×3 ; `grep -c 'dispatch_context' src/pico8_wormhole.c` → **0**.

À `b` :
5. `grep -n '^[a-z].*pico8_jo_\(hb\|gv\|kj\|ldown\|kf\|cn\)_dispatch(\|^pico8_down_t pico8_jo_ldown(' src/*.c` → 7 lignes : `planet_display.c:478`, `:482`, `:486`, `:491`, `:497`, `planet_trade.c:249`, `:253`.
6. `grep -c '#include "pico8_\(planet_display\|planet_trade\|slipway\|dispatch_context\)\.h"' src/pico8_planet.c` → **0**.
7. `git diff a b -- src/*.h | wc -l` → **0**.

Aux deux :
8. `make composition-residue` → `inclusion-mutuelle 12 11`, `résidu 2 : 30 ligne(s), 17 fichier(s)` (`a`) ; `inclusion-mutuelle 9 9`, `résidu 2 : 22 ligne(s), 13 fichier(s)` (`b`) ; `python3 usegraph.py .` → `paires 15`, `paires 12`.
9. `git diff --stat <parent>` : les fichiers de la section « Modules », plus `docs/plan-3.md`, rien d'autre ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` → vide.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable (`prd217ab/copie`, arbres de travail `wt_p` = `x`, `wt_a`, `wt_b`, `c_p`, `c_a`, `c_b`, `wt_r`).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout 1ab94d6` ; `git rev-parse` des arbres | — | 0 | `f520dbf0…`, `b15bb8ec…`, `e9226b4f…` (= 2.15d) | 3 s |
| 2 | `extract.py` : 10 scripts tirés des annexes des PRD 2.16, 2.15d, 2.15c, 2.11 et de l'étude | — | 0 | `run216.py`, `fncmp.py`, `syn.sh`, `objs.sh`, `mut.py`, `base.sh`, `cross.py`, `usegraph.py`, `sites.py`, `lset.py` | < 1 s |
| 3 | `run216.py copie` ; `git add -A` ; hachages ; commit `x` | `x` | 0 | `18 remplacements, 21 occurrences` ; `148603f3…`, `cac32597…`, `422546ca…` ; `efb84dbc…` (ceux du PRD 2.16) | < 1 s |
| 4 | compteur, `--detail`, `--check inclusion-mutuelle`, `usegraph.py` | `x` | 0 | `15 15`, `39 / 24`, 39 lignes, `paires 18` | 2 s |
| 5 | `run217a.py wt_a` ; `syn.sh` ; `git diff --stat` | `a` | 0 | 0 erreur ; 10 fichiers, +32 / −27 | 3 s |
| 6 | compteur, `--check`, `usegraph.py` ; `usedir.py` (premier jet : `SyntaxError`, barre oblique inverse dans une f-chaîne sous Python 3.11 ; réécrit) | `x`, `a` | 0 | `12 11`, `30 / 17`, `paires 15` ; relevés du tableau de la preuve 2 | 2 s |
| 7 | commit `a` ; `run217b.py wt_b` ; `syn.sh` | `b` | 0 | 0 erreur ; 3 fichiers, +51 / −55 | 3 s |
| 8 | compteur, `--check`, `usegraph.py`, `usedir.py` | `b` | 0 | `9 9`, `22 / 13`, `paires 12` ; relevés vides | 2 s |
| 9 | `deadinc.py` sur les 9 `.c` touchés | `x`, `a`, `b` | 0 | 13 includes à 0 nom à `x` et `a` (préexistants), 12 à `b` | < 1 s |
| 10 | `tests.sh` : `make -j4 test`, `make e2e`, `MALLOC_PERTURB_=165 make e2e` | `x`, `a`, `b` | 0 ×9 | vert ; 26 ; 26, aux trois | 18 s, 12 s, 12 s |
| 11 | `objs2.sh` ×4 configurations ×3 arbres ; premier jet de `fnbody.py` (motif de `objdump -t` faux : liaisons `None`, tables non résolues) | `x`, `a`, `b` | 0 | 74 objets par configuration ; faux écarts de tables | non chronométré (≈ 2 min) |
| 12 | `fnbody.py` corrigé (déplacements `%rip`, cibles d'appel relogées, chaînes `.LC`, `NORM_ASSERT`) ; 3 versions, la dernière en annexe | `x`, `a`, `b` | 0 | table de la preuve 4 | 3 s par paire |
| 13 | `cmp` des objets ; `nm` des 11 symboles | `x`, `a`, `b` | 0 | 68 / 74, 71 / 74 ; `t` → `T` ×4 | < 1 s |
| 14 | `mut217.py p` et `a` (5), puis `a` et `b` (9), en parallèle ; premier JSON : MZ-kj et PJ-hk ne compilent pas (`-Werror` : variable `p`, paramètre `yr` inutilisés), aux deux bouts | `wt_p`, `wt_a`, `wt_b` | 0 | tableau des jumelles hors MZ-kj, PJ-hk | 162 s |
| 15 | `mut217.py p` et `a` : MZ-kj, PJ-hk corrigés (JSON = sortie de `muts217gen.py`, `cmp` identique) ; `git status` des arbres | `wt_p`, `wt_a` | 0 | lignes MZ-kj, PJ-hk ; arbres propres | 37 s |
| 16 | `cross.py` | `c_p`, `c_a`, `c_b` | 0 | 39, 30, 22 lignes ; plus aucune ligne des sens coupés | < 1 s |
| 17 | `clang -fsyntax-only` sur `src/` et `tests/` | `c_a`, `c_b` | 0 | 0 erreur | non chronométré |
| 18 | `run217a.py`, `run217b.py` sur `wt_r` neuf à `x` ; hachages ; `run217b.py` relancé | `wt_r` | 0 ; 1 | `e8e76840…`, `3c5385b8…` ; `AssertionError` | < 1 s |
| 19 | `cov.sh` : `make line-coverage` des 26 e2e | `c_p`, `c_a`, `c_b` | 0 | 6031 ×3 | 6 s, 9 s, 9 s |
| 20 | `lset.py` `x` → `a`, `a` → `b` ; `difflib` sur `planet.c` | — | 0 | `L` 13, 13/13 ; `L` 31, 32/32, 1 artefact | < 1 s |
| 21 | `nets.sh` : `make class-coverage-net`, `make -j2 sanitize-net` | `c_a`, `c_b` | 0 ×4 | `couverture: OK` ; `26 propre(s)` | 12 s, 14 s |
| 22 | `usegraph_e.py` ; composantes fortement connexes | `c_p`, `c_a`, `c_b` | 0 | 622 / 620 / 617 arêtes ; 26 + 2 → 24 → 24 | < 1 s |
| 23 | `run217b1.py` (repli 5) sur `wt_r` à `a` ; `syn.sh` ; compteur ; `usegraph.py` | `wt_r` | 0 | 0 erreur ; `10 10`, `25 / 15`, `paires 13` | 3 s |
| 24 | `rm -rf` de la copie, des arbres de travail et des fichiers d'essai | — | 0 | supprimés | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas le commit de 2.16 (arbres `148603f3…`, `cac32597…`, `422546ca…`, ou un descendant qui ne touche que `docs/`) sans que l'écart soit rapporté, ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si 2.17a et 2.17b ne sont pas deux commits, chacun vert (`make test`, `make e2e` 26 conformes, et 26 sous `MALLOC_PERTURB_=165`), ou si un `*.sha256`, `tests/`, `tools/`, le `Makefile`, une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si, au départ annoncé, l'arbre `src` commité n'est pas `e8e76840…` (2.17a) puis `3c5385b8…` (2.17b), sans repli rapporté.
- `FAILED:` si le relevé `usedir.py` d'un des six sens coupés n'est pas vide au commit (nom propre, nom partagé ou include) ; si une ligne `#include` retirée l'est alors qu'un nom de l'en-tête reste employé (détour) ; si un nom de l'en-tête d'affichage est redéclaré dans le `.c` de l'entité.
- `FAILED:` si une paire entre au compteur (`--detail`) ou au graphe d'usage, à l'un des deux commits ; si les comptes ne sont pas ceux de la table « Comptes » (12 / 11, 30 lignes / 17 fichiers, usage 15 ; 9 / 9, 22 / 13, usage 12) ; si une catégorie gardée n'est plus à 0.
- `FAILED:` si un corps déplacé diffère de son parent sous `-O0 -DNDEBUG` ou `-O1 -DNDEBUG` (`fnbody.py`), si une autre fonction ou une relocation de données diffère dans ces configurations, ou si un objet non touché diffère octet pour octet ; si, sous `-O0` avec `assert`, un écart reste sous `NORM_ASSERT=1`.
- `FAILED:` si une jumelle a un TU rouge au parent qui ne l'est pas à l'enfant, ou un e2e < 26 au parent et 26 à l'enfant.
- `FAILED:` si une ligne couverte par les 26 e2e au parent ne l'est plus à l'enfant (hors l'artefact `planet.c:73` décrit), ou si une ligne exécutable neuve n'est pas couverte.
- `FAILED:` si une table de classe, un nom, un corps (hors `static ` des quatre dispatchers de 2.17a), un commentaire ou `planet.h` change ; si un include mort est créé (`deadinc.py`).
- `FAILED:` si `class-coverage-net` ou `sanitize-net` est rouge.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si un message de commit omet : les fonctions déplacées et leurs fichiers, les includes retirés et ajoutés (dont les deux `dispatch_context.h`, G2), le relevé d'usage de chaque sens coupé, les comptes (paires / modules, lignes / fichiers, usage), la preuve des objets par configuration (dont l'écart `-O2` déclaré), la table des jumelles avec leur résultat au parent (dont la survivante PJ-init), `L`, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.17a » (resp. « 2.17b ») au bout de sa ligne du plan, ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- 2.17c à 2.17f (économie, déclarations `pico8_fm.h`/`pico8_probe_comp.h`, `pico8_na`/`pico8_gd`/`kv`, garde de l'usage) et les neuf paires qui restent : `economy` ↔ `planet`, `economy_formulas` ↔ `slipway`, `entity_geom` ↔ `planet`, `planet` ↔ `planet_family`, `planet` ↔ `probe`, `probe` ↔ `slipway`, `probe` ↔ `ui`, `probe` ↔ `wormhole`, `slipway` ↔ `ui` (plus les usages cachés `economy` ↔ `economy_score`, `economy` ↔ `tech_gate`, `entity_geom` ↔ `scroll`).
- Les 12 à 13 includes préexistants à 0 nom que `deadinc.py` signale dans les fichiers touchés (hors ceux que 2.17a/b rendraient morts) ; le cycle de 24 modules du graphe d'usage (étude, §3 : hors périmètre).
- `tools/line_coverage_reference.txt` (hors suite, déjà décalé par les étapes précédentes).
- La taille du code C, l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Corps ajoutés en fin de fichier d'arrivée, prototypes en fin d'en-tête.** Même règle que les essais de l'étude (`cuts/t1_*.py`) : pas de place « naturelle » à choisir, un diff lisible, et l'ordre des définitions n'a pas d'effet à `-O0`/`-O1` (preuve 4). Dans `planet_display.c`, l'ordre de `planet.c` est gardé (hb, gv, kj, ldown_dispatch, ldown).
- **G2. Les deux `#include "pico8_dispatch_context.h"` que le déplacement rend morts partent** (`wormhole.c:8`, `planet.c:10`). Le plan ne les nomme pas ; les garder laisserait un include sans usage de plus, exactement la forme qui cache un détour. Prix : deux lignes de diff, objets identiques (un en-tête de déclarations seules). `slipway.c` et `score_panel.c` gardent le leur (encore employé).
- **G3. Pas de commentaire aux prototypes déplacés ou ajoutés.** Ni les dispatchers ni init/shutdown n'ont de contrat que le nom ne dise ; `score_panel_display.h` n'en avait pas pour init/shutdown.
- **G4. Deux commits, comme le plan.** Repli écrit si 2.17b déborde (b1/b2, comptes mesurés).
- **G5. Preuve des objets par fonction, pas par objet.** Un corps qui change de fichier change d'objet ; `fncmp.py` du PRD 2.16 compare dans un même objet et ne résout pas les relocations (deux appels à des fonctions différentes y seraient égaux) : `fnbody.py` suit la fonction où qu'elle soit et résout chaque relocation.
- **G6. `class-coverage-net` et `sanitize-net` lancés bien que non exigés** (≈ 15 s) : `sanitize-net` est la seule configuration `-O2` du dépôt, celle où les corps diffèrent par intégration.

### Surprises de mesure, pour `plan-pilot`

1. **Une ligne de plus que l'étude à chaque bout** : `planet.h:11` → `probe.h`, entrée à 2.15c. Elle ne touche ni 2.17a ni 2.17b, mais **2.17d doit la couper** (`planet.h` inclut `pico8_probe_comp.h` au lieu de `pico8_probe.h`), sinon `planet` ↔ `probe` reste par `pico8_probe_t` et 2.17d n'arrive pas au compte de l'étude.
2. **Comptes en paires / modules et lignes / fichiers** : le compteur rend `paires modules` ; la ligne du plan écrivait « paires / lignes (fichiers) ». Réels : 12 / 11 (30 / 17) et 9 / 9 (22 / 13).
3. **La colonne usage de l'étude était juste** (18 → 15 → 12), réserve de `0acfad8` levée par mesure.
4. **`-O2` : les corps déplacés ne sont plus identiques** (intégration dans la même unité) ; `-O0` (le jeu) et `-O1` le sont.
5. **Une jumelle survit** (PJ-init) aux deux bouts : l'init de `score_panel_display` ne fait rien d'observable au premier appel.

### Questions pour l'humain

Aucune.

### Annexe A — scripts (texte exact)

Tous hors dépôt, depuis un dossier d'essais `S` (à adapter) qui contient la copie et ses arbres de travail `wt_p` (`x`), `wt_a`, `wt_b`, `c_p`, `c_a`, `c_b` ; journaux dans `S/logs`. `syn.sh`, `mut.py` : annexe A du PRD 2.15d (`mut217.py` = `mut.py` où `prd215d` devient `prd217ab` et `muts.json` devient `muts217.json`) ; `lset.py` : annexe A du PRD 2.15c ; `usegraph.py`, `cross.py` : annexe A du PRD 2.11 ; `usedir.py` et `usegraph_e.py` : `usegraph.py` avec le bloc ci-dessous inséré avant la ligne `pairs=[(a,b) …` (sortie d'erreur). Les sens de la preuve 2 : `DIRS="wormhole>wormhole_display slipway>slipway_display score_panel>score_panel_display planet>planet_display planet>planet_trade planet>slipway wormhole_display>wormhole slipway_display>slipway score_panel_display>score_panel planet_display>planet planet_trade>planet slipway>planet" python3 usedir.py <arbre>`.

Bloc de `usedir.py` :

```python
# usedir : pour chaque sens A>B de $DIRS, noms de B.h employes par A (seul
# declarant), noms partages avec un autre en-tete, lignes #include de B.h.
def _toks(f): return set(re.findall(r'\b[A-Za-z_]\w*\b',strip(open(f).read())))
for q in os.environ.get('DIRS','').split():
    a,b=['pico8_'+x for x in q.split('>')]
    ta=set().union(*[_toks(f) for f in files[a]])
    sh=sorted(x for x in owner if b in owner[x] and len(owner[x])>1 and x in ta)
    inc=[f for f in files[a] if '#include "'+b+'.h"' in open(f).read()]
    print('DIR\t%s\t%d:%s\tpartages:%s\tinclude:%s'%(q,len(use[a][b]),' '.join(sorted(use[a][b])) or '-',' '.join(sh) or '-',' '.join(inc) or '-'),file=sys.stderr)
```

Bloc de `usegraph_e.py` (une arête orientée par ligne, `E<TAB>A<TAB>B<TAB>noms`) :

```python
for a in sorted(use):
    for b in sorted(use[a]): print('E\t%s\t%s\t%s'%(a,b,' '.join(sorted(use[a][b]))),file=sys.stderr)
```

`run217a.py` :

```python
# run217a.py ARBRE : 2.17a sur un arbre propre au commit 2.16. Chaque texte
# cherche doit etre present exactement une fois, sinon arret (AssertionError).
import os, sys
T = sys.argv[1]
def p(f): return os.path.join(T, 'src', f)
def rd(f): return open(p(f)).read()
def wr(f, s): open(p(f), 'w').write(s)
def sub(f, old, new):
    s = rd(f); assert s.count(old) == 1, (f, old, s.count(old)); wr(f, s.replace(old, new))
def cut(f, text):
    sub(f, text + '\n', ''); return text
def append(f, text): wr(f, rd(f).rstrip('\n') + '\n\n' + text + '\n')
def before_endif(f, text): sub(f, '\n#endif\n', '\n' + text + '\n#endif\n')

MZ = '''static void pico8_mz_kj_dispatch(void *self, const pico8_vec_t *ra) {
    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_mz_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_mz_t *)self, p);
}
'''
JC = '''static void pico8_jc_kj_dispatch(void *self, const pico8_vec_t *ra) {
    (void)ra;
    pico8_jc_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jc_t *)self);
}
'''
HK = '''static void pico8_pj_hk_dispatch(void *self, pico8_fixed_t yr) {
    (void)self;
    pico8_pj_hk(yr);
}
'''
PH = '''static void pico8_pj_ph_dispatch(void *self, pico8_fixed_t t) {
    (void)t;
    pico8_pj_ph((pico8_pj_t *)self);
}
'''
# wormhole -> wormhole_display (fiche 17)
cut('pico8_wormhole.c', MZ)
append('pico8_wormhole_display.c', MZ[len('static '):].rstrip('\n'))
sub('pico8_wormhole_display.c', '#include "pico8_entity.h"\n', '#include "pico8_dispatch_context.h"\n#include "pico8_entity.h"\n')
before_endif('pico8_wormhole.h', 'void pico8_mz_kj_dispatch(void *self, const pico8_vec_t *ra);\n')
sub('pico8_wormhole.c', '#include "pico8_dispatch_context.h"\n', '')
sub('pico8_wormhole.c', '#include "pico8_wormhole_display.h"\n', '')
# slipway -> slipway_display (fiche 15)
cut('pico8_slipway.c', JC)
append('pico8_slipway_display.c', JC[len('static '):].rstrip('\n'))
sub('pico8_slipway_display.c', '#include "pico8_entity.h"\n', '#include "pico8_dispatch_context.h"\n#include "pico8_entity.h"\n')
before_endif('pico8_slipway.h', 'void pico8_jc_kj_dispatch(void *self, const pico8_vec_t *ra);\n')
sub('pico8_slipway.c', '#include "pico8_slipway_display.h"\n', '')
# score_panel -> score_panel_display (fiche 14)
cut('pico8_score_panel.c', HK)
cut('pico8_score_panel.c', PH)
append('pico8_score_panel_display.c', HK[len('static '):] + '\n' + PH[len('static '):].rstrip('\n'))
sub('pico8_score_panel_display.h', 'void pico8_score_panel_display_init(void);\nvoid pico8_score_panel_display_shutdown(void);\n\n', '')
before_endif('pico8_score_panel.h', 'void pico8_pj_hk_dispatch(void *self, pico8_fixed_t yr);\n'
             'void pico8_pj_ph_dispatch(void *self, pico8_fixed_t t);\n\n'
             'void pico8_score_panel_display_init(void);\nvoid pico8_score_panel_display_shutdown(void);\n')
sub('pico8_score_panel.c', '#include "pico8_score_panel_display.h"\n', '')
print('2.17a : 4 dispatchers deplaces, 2 prototypes deplaces, 4 includes retires, 2 ajoutes')
```

`run217b.py` :

```python
# run217b.py ARBRE : 2.17b sur un arbre propre au commit 2.17a. Chaque texte
# cherche doit etre present exactement une fois, sinon arret (AssertionError).
import os, sys
T = sys.argv[1]
def p(f): return os.path.join(T, 'src', f)
def rd(f): return open(p(f)).read()
def wr(f, s): open(p(f), 'w').write(s)
def sub(f, old, new):
    s = rd(f); assert s.count(old) == 1, (f, old, s.count(old)); wr(f, s.replace(old, new))
def cut(f, text):
    sub(f, text + '\n', ''); return text
def append(f, text): wr(f, rd(f).rstrip('\n') + '\n\n' + text + '\n')

HB = '''void pico8_jo_hb_dispatch(void *self) {
    pico8_jo_hb((pico8_jo_t *)self);
}
'''
GV = '''pico8_down_t pico8_jo_gv_dispatch(void *self, const pico8_struct_header_t *s) {
    return pico8_jo_gv((const pico8_jo_t *)self, s);
}
'''
KJ = '''void pico8_jo_kj_dispatch(void *self, const pico8_vec_t *ra) {
    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_jo_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jo_t *)self, p);
}
'''
LD = '''pico8_down_t pico8_jo_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    (void)qa;
    (void)ra;
    return pico8_jo_ldown((pico8_jo_t *)self);
}
'''
KF = '''void pico8_jo_kf_dispatch(void *self, pico8_struct_header_t *sw) {
    pico8_jo_kf((pico8_jo_t *)self, sw);
}
'''
CN = '''void pico8_jo_cn_dispatch(void *self, struct pico8_jo_trade_proposal *lw) {
    pico8_jo_cn((pico8_jo_t *)self, lw);
}
'''
LDOWN = '''pico8_down_t pico8_jo_ldown(pico8_jo_t *self) {
    const char *me_val;
    pico8_down_t out = {0, NULL};

    if (self == NULL || self->header.me == NULL) {
        return out;
    }
    me_val = self->header.me;
    if (strcmp(me_val, "known") == 0) {
        pico8_jo_fe(self);
        out.stop = 1;
        return out;
    }
    if (strcmp(me_val, "ly") == 0) {
        pico8_jc_keys_t keys = {0};

        assert(pico8_jc_ready() && "pico8_jc_init() must run before jo:ldown builds a jc");
        keys.qu = (pico8_struct_header_t *)self;
        out.held = (pico8_struct_header_t *)pico8_jc_new(&keys);
        out.stop = out.held != NULL;
    }
    return out;
}'''
# planet -> planet_display, planet -> slipway (fiches 5 et 9)
for f in (HB, GV, KJ, LD):
    cut('pico8_planet.c', f)
s = rd('pico8_planet.c'); assert s.endswith('\n\n' + LDOWN + '\n'); wr('pico8_planet.c', s[:-len('\n' + LDOWN + '\n')])
append('pico8_planet_display.c', HB + '\n' + GV + '\n' + KJ + '\n' + LD + '\n' + LDOWN)
sub('pico8_planet.c', '#include "pico8_dispatch_context.h"\n', '')
sub('pico8_planet.c', '#include "pico8_planet_display.h"\n', '')
sub('pico8_planet.c', '#include "pico8_slipway.h"\n', '')
# planet -> planet_trade (fiche 7)
for f in (KF, CN):
    cut('pico8_planet.c', f)
append('pico8_planet_trade.c', KF + '\n' + CN.rstrip('\n'))
sub('pico8_planet.c', '#include "pico8_planet_trade.h"\n', '')
print('2.17b : 7 fonctions deplacees, 4 includes retires')
```

`deadinc.py` :

```python
# deadinc.py ARBRE FICHIER... : pour chaque #include "pico8_X.h" du fichier, nombre
# de noms que X.h declare (motifs de usegraph.py, partages compris) employes
# par le fichier ; 0 = include mort.
import re, sys, os, glob
T = sys.argv[1]
exec(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'usegraph.py')).read().split('owner={}')[0].replace('os.chdir(sys.argv[1])', 'os.chdir(T)'))
for f in sys.argv[2:]:
    src = open(f).read(); toks = set(re.findall(r'\b[A-Za-z_]\w*\b', strip(src)))
    for h in re.findall(r'#include "(pico8_\w+)\.h"', src):
        n = decl.get(h, set()) & toks
        if f.endswith(h + '.c') or f.endswith(h + '.h'): continue
        print(f'{f}\t{h}.h\t{len(n)}' + ('\tMORT' if not n else ''))
```

`objs2.sh` (appelé `bash objs2.sh <arbre> <sortie> "<drapeaux> -DPICO8_CLASS_COVERAGE"`, drapeaux `-O0 -DNDEBUG`, `-O0`, `-O1 -DNDEBUG`, `-O2 -DNDEBUG`) :

```sh
#!/bin/bash
# objs2.sh ARBRE OUT DRAPEAUX : objets sans -g de src/ (hors raylib) -> OUT/*.o
S=/tmp/claude-0/-home-user-slipways-love/c83e28ad-64b1-5e5c-ac9f-9444621e8cef/scratchpad/prd217ab
cd "$S/$1"; mkdir -p $S/$2
for f in $(ls src/*.c | grep -vE 'main_raylib|main_screencast_viewer|raylib_backend'); do
  cc -std=c99 $3 -Isrc -c -o $S/$2/$(basename $f .c).o $f || echo "ECHEC $f"
done
```

`fnbody.py` (`python3 fnbody.py <objets du parent> <objets de l'enfant>`) :

```python
# fnbody.py DIR_A DIR_B : compare fonction par fonction les objets (*.o) de deux
# arbres, ou que chaque fonction soit definie. Corps = objdump -dr sans
# adresses ni deplacements %rip ; cible d'un appel reloge = sa relocation ;
# relocations de section resolues (fonction, variable, texte de chaine).
# NORM_ASSERT=1 : immediats `mov $..,%edx` (__LINE__), __FILE__ et numeros
# des statiques locales (__PRETTY_FUNCTION__.n, gap.n) neutralises.
# FNDIFF=1 : imprime le debut du diff de chaque corps different.
import sys, os, re, subprocess, glob, collections, difflib
NA = os.environ.get('NORM_ASSERT')
def sh(*a): return subprocess.run(a, capture_output=True, text=True).stdout
def na(r): return re.sub(r'"src/pico8_\w+\.c"', '"FICHIER"', re.sub(r'\b(__PRETTY_FUNCTION__|gap_items|gap)\.\d+', r'\1.N', r)) if NA else r
def load(d):
    F = collections.defaultdict(list); B = {}; D = {}; bind = {}
    for o in sorted(glob.glob(d + '/*.o')):
        mod = os.path.basename(o)[:-2]
        syms = collections.defaultdict(list); lc = {}
        for l in sh('objdump', '-t', o).split('\n'):
            m = re.match(r'^([0-9a-f]+) (.)(.{6}) (\S+)\t([0-9a-f]+) (\S+)$', l)
            if not m: continue
            off, b, _, sec, size, name = m.groups()
            if name.startswith('.L'): lc[name] = (sec, int(off, 16)); continue
            if name == sec or name.startswith('.'): continue
            syms[sec].append((int(off, 16), name))
            if sec.startswith('.text'): bind[(name, mod)] = b
        strs = {}
        for sec in set(re.findall(r'\s(\.rodata[\w.]*)\s', sh('objdump', '-h', o))):
            data = bytearray()
            for l in sh('objdump', '-s', '-j', sec, o).split('\n')[4:]:
                m = re.match(r'^ ([0-9a-f]+) ((?:[0-9a-f]{2,8} ?)+)', l)
                if m: data += bytes.fromhex(m.group(2).replace(' ', ''))
            strs[sec] = bytes(data)
        def res(target, rtype):
            m = re.match(r'^(\.[\w.]+)([+-]0x[0-9a-f]+)?$', target)
            if not m: return na(target)
            sec, add = m.group(1), int(m.group(2) or '0', 16)
            if 'PC32' in rtype or 'PLT32' in rtype: add += 4
            if sec in lc: sec, base = lc[sec]; add += base
            for off, name in sorted(syms.get(sec, []), reverse=True):
                if off <= add: return na(name if add == off else f'{name}+{add - off}')
            if sec in strs: return na('str:"%s"' % strs[sec][add:].split(bytes(1))[0].decode(errors='replace'))
            return f'{sec}+{add}'
        cur = None
        for l in sh('objdump', '-dr', '--no-show-raw-insn', o).split('\n'):
            m = re.match(r'^[0-9a-f]+ <(.+)>:$', l)
            if m: cur = m.group(1); F[cur].append(mod); B[(cur, mod)] = []; continue
            if cur is None or not l.strip(): continue
            L = B[(cur, mod)]
            m = re.match(r'^\s+[0-9a-f]+:\s+(R_\S+)\s+(\S+)$', l)
            if m:
                if L and not L[-1].startswith('RELOC'): L[-1] = re.sub(r'\s*<[^>]+>$', '', L[-1])
                L.append(f'RELOC {m.group(1)} {res(m.group(2), m.group(1))}'); continue
            m = re.match(r'^\s+[0-9a-f]+:\s+(.*)$', l)
            if not m: continue
            t = re.sub(r'\b[0-9a-f]+ <([^>]+)>', r'<\1>', m.group(1))
            t = re.sub(r'-?0x[0-9a-f]+\(%rip\)', '(%rip)', t)
            if NA: t = re.sub(r'mov\s+\$0x[0-9a-f]+,%edx', 'mov $LIGNE,%edx', t)
            L.append(t.strip())
        cur = None
        for l in sh('objdump', '-r', o).split('\n'):
            m = re.match(r'^RELOCATION RECORDS FOR \[(.+)\]:', l)
            if m: cur = None if m.group(1).startswith('.text') or 'eh_frame' in m.group(1) else m.group(1); continue
            m = re.match(r'^([0-9a-f]+) (\S+)\s+(\S+)$', l)
            if cur and m:
                off = int(m.group(1), 16)
                owner = next((f'{n}+{off - o2}' for o2, n in sorted(syms.get(cur, []), reverse=True) if o2 <= off), f'{cur}+{off}')
                D[(mod, na(owner))] = res(m.group(3), m.group(2))
    return F, B, D, bind
Fa, Ba, Da, bda = load(sys.argv[1]); Fb, Bb, Db, bdb = load(sys.argv[2])
moved = []; diff = []; same = 0
for fn in sorted(set(Fa) | set(Fb)):
    ma, mb = Fa.get(fn, []), Fb.get(fn, [])
    if len(ma) != 1 or len(mb) != 1:
        if ma != mb: diff.append(f'{fn}: modules {ma} -> {mb}')
        else:
            for m in ma:
                if Ba[(fn, m)] != Bb[(fn, m)]: diff.append(f'{fn}@{m} (copie locale)')
        continue
    a, b = Ba[(fn, ma[0])], Bb[(fn, mb[0])]
    if ma != mb or bda.get((fn, ma[0])) != bdb.get((fn, mb[0])):
        moved.append(f'{fn}: {ma[0]} ({bda.get((fn, ma[0]))}) -> {mb[0]} ({bdb.get((fn, mb[0]))}) corps {"identique" if a == b else "DIFFERENT"}')
    elif a != b: diff.append(f'{fn}@{ma[0]}')
    else: same += 1
dd = sorted(k for k in set(Da) | set(Db) if Da.get(k) != Db.get(k))
print(f'fonctions : {same} identiques en place ; {len(moved)} deplacees ou reliees ; {len(diff)} differentes')
for x in moved: print('  DEPL', x)
for x in diff: print('  DIFF', x)
print(f'relocations de donnees : {len(Da)} / {len(Db)} ; differentes : {len(dd)}')
for k in dd[:40]: print('  DATA', k, Da.get(k), '->', Db.get(k))
if os.environ.get('FNDIFF'):
    for x in diff + [m for m in moved if 'DIFFERENT' in m]:
        fn = re.split(r'[@:]', x)[0]
        ka = [k for k in Ba if k[0] == fn]; kb = [k for k in Bb if k[0] == fn]
        if len(ka) == 1 and len(kb) == 1:
            print('  ##', fn)
            for l in list(difflib.unified_diff(Ba[ka[0]], Bb[kb[0]], lineterm='', n=0))[2:14]: print('   ', l)
```

`muts217gen.py` :

```python
# muts217gen.py : ecrit muts217.json (format de mut.py : [fichier, ancre, ancien, neuf]
# par cote) ; cotes p = 2.16, a = 2.17a, b = 2.17b ; meme texte des deux cotes.
import json
def m(fa, fb, anc, old, new): return (fa, fb, anc, old, new)
A = {  # 2.17a : parent p, enfant a
 'MZ-kj':  m('src/pico8_wormhole.c', 'src/pico8_wormhole_display.c', 'pico8_mz_kj_dispatch(', '(pico8_mz_t *)self, p);', '(pico8_mz_t *)self, pico8_vec_v(p.y, p.x));'),
 'JC-kj':  m('src/pico8_slipway.c', 'src/pico8_slipway_display.c', 'pico8_jc_kj_dispatch(', '    pico8_jc_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jc_t *)self);\n', '    (void)self;\n'),
 'PJ-hk':  m('src/pico8_score_panel.c', 'src/pico8_score_panel_display.c', 'pico8_pj_hk_dispatch(', '    pico8_pj_hk(yr);\n', '    pico8_pj_hk(pico8_fixed_add(yr, pico8_fixed_from_int(1)));\n'),
 'PJ-ph':  m('src/pico8_score_panel.c', 'src/pico8_score_panel_display.c', 'pico8_pj_ph_dispatch(', '    pico8_pj_ph((pico8_pj_t *)self);\n', '    (void)self;\n'),
 'PJ-init': m('src/pico8_score_panel.c', 'src/pico8_score_panel.c', None, '    pico8_score_panel_display_init();\n', ''),
}
L = 'pico8_down_t pico8_jo_ldown(pico8_jo_t *self) {'
B = {  # 2.17b : parent a, enfant b
 'JO-hb':  m('src/pico8_planet.c', 'src/pico8_planet_display.c', 'pico8_jo_hb_dispatch(', '    pico8_jo_hb((pico8_jo_t *)self);\n', '    (void)self;\n'),
 'JO-gv':  m('src/pico8_planet.c', 'src/pico8_planet_display.c', 'pico8_jo_gv_dispatch(', '    return pico8_jo_gv((const pico8_jo_t *)self, s);\n', '    pico8_down_t o = {0, NULL};\n    (void)self;\n    (void)s;\n    return o;\n'),
 'JO-kj':  m('src/pico8_planet.c', 'src/pico8_planet_display.c', 'pico8_jo_kj_dispatch(', '(pico8_jo_t *)self, p);', '(pico8_jo_t *)self, pico8_vec_v(p.y, p.x));'),
 'JO-ld':  m('src/pico8_planet.c', 'src/pico8_planet_display.c', 'pico8_jo_ldown_dispatch(', '    return pico8_jo_ldown((pico8_jo_t *)self);\n', '    pico8_down_t o = {0, NULL};\n    (void)self;\n    return o;\n'),
 'LD-known': m('src/pico8_planet.c', 'src/pico8_planet_display.c', L, '        pico8_jo_fe(self);\n', ''),
 'LD-me':  m('src/pico8_planet.c', 'src/pico8_planet_display.c', L, 'strcmp(me_val, "known")', 'strcmp(me_val, "knowm")'),
 'LD-ly':  m('src/pico8_planet.c', 'src/pico8_planet_display.c', L, '        out.stop = out.held != NULL;\n', '        out.stop = 0;\n'),
 'JO-kf':  m('src/pico8_planet.c', 'src/pico8_planet_trade.c', 'pico8_jo_kf_dispatch(', '    pico8_jo_kf((pico8_jo_t *)self, sw);\n', '    (void)self;\n    (void)sw;\n'),
 'JO-cn':  m('src/pico8_planet.c', 'src/pico8_planet_trade.c', 'pico8_jo_cn_dispatch(', '    pico8_jo_cn((pico8_jo_t *)self, lw);\n', '    (void)self;\n    (void)lw;\n'),
}
J = {}
for k, (fa, fb, anc, old, new) in A.items(): J[k] = {'p': [[fa, anc, old, new]], 'a': [[fb, anc, old, new]]}
for k, (fa, fb, anc, old, new) in B.items(): J[k] = {'a': [[fa, anc, old, new]], 'b': [[fb, anc, old, new]]}
json.dump(J, open('muts217.json', 'w'), indent=1)
```

`tests.sh` :

```sh
#!/bin/bash
# tests.sh WT... : make test, make e2e, e2e perturbe dans chaque arbre, sequentiel
S=/tmp/claude-0/-home-user-slipways-love/c83e28ad-64b1-5e5c-ac9f-9444621e8cef/scratchpad/prd217ab
for w in "$@"; do
  cd $S/$w
  t0=$(date +%s); timeout 900 make -j4 test > $S/logs/$w.test 2>&1; c1=$?
  timeout 600 make e2e > $S/logs/$w.e2e 2>&1; c2=$?
  MALLOC_PERTURB_=165 timeout 600 make e2e > $S/logs/$w.e2ep 2>&1; c3=$?
  echo "$w test=$c1 e2e=$c2 ($(grep -c ': conforme' $S/logs/$w.e2e)) e2ep=$c3 ($(grep -c ': conforme' $S/logs/$w.e2ep)) $(( $(date +%s)-t0 ))s"
done
```

`cov.sh` :

```sh
#!/bin/bash
S=/tmp/claude-0/-home-user-slipways-love/c83e28ad-64b1-5e5c-ac9f-9444621e8cef/scratchpad/prd217ab
for w in "$@"; do
  cd $S/c_$w; t0=$(date +%s)
  E=$(ls tests/e2e/*.json | tr '\n' ' ')
  timeout 1200 make line-coverage EVENTS="$E" > $S/logs/cov_$w.log 2>&1; c=$?
  cp tmp/line_coverage.txt $S/logs/cov_$w.txt
  echo "$w code=$c lignes=$(wc -l < $S/logs/cov_$w.txt) $(( $(date +%s)-t0 ))s"
done
```

`nets.sh` :

```sh
#!/bin/bash
S=/tmp/claude-0/-home-user-slipways-love/c83e28ad-64b1-5e5c-ac9f-9444621e8cef/scratchpad/prd217ab
for w in "$@"; do
  cd $S/c_$w; t0=$(date +%s)
  timeout 1200 make class-coverage-net > $S/logs/ccn_$w.log 2>&1; c1=$?
  timeout 1800 make -j2 sanitize-net > $S/logs/san_$w.log 2>&1; c2=$?
  echo "$w ccn=$c1 $(tail -1 $S/logs/ccn_$w.log) | san=$c2 $(tail -1 $S/logs/san_$w.log) | $(( $(date +%s)-t0 ))s"
done
```

### Annexe B — diffs (texte exact)

`git diff x a` (2.17a, 195 lignes, `90747520…`) :

```diff
diff --git a/src/pico8_score_panel.c b/src/pico8_score_panel.c
index 19fec17..b18e1ed 100644
--- a/src/pico8_score_panel.c
+++ b/src/pico8_score_panel.c
@@ -18,7 +18,6 @@
 #include "pico8_pi.h"
 #include "pico8_planet.h"
 #include "pico8_probe.h"
-#include "pico8_score_panel_display.h"
 #include "pico8_str.h"
 #include "pico8_struct.h"
 #include "pico8_vec.h"
@@ -262,16 +261,6 @@ static void pico8_pj_kj_dispatch(void *self, const pico8_vec_t *ra) {
     pico8_pj_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_pj_t *)self);
 }
 
-static void pico8_pj_hk_dispatch(void *self, pico8_fixed_t yr) {
-    (void)self;
-    pico8_pj_hk(yr);
-}
-
-static void pico8_pj_ph_dispatch(void *self, pico8_fixed_t t) {
-    (void)t;
-    pico8_pj_ph((pico8_pj_t *)self);
-}
-
 static const pico8_struct_class_t g_pj_struct_class = {
     .kj = pico8_pj_kj_dispatch,
     .hk = pico8_pj_hk_dispatch,
diff --git a/src/pico8_score_panel.h b/src/pico8_score_panel.h
index 0d61e03..8c2a05a 100644
--- a/src/pico8_score_panel.h
+++ b/src/pico8_score_panel.h
@@ -119,4 +119,10 @@ void pico8_pj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_pj_t *self);
 
 void pico8_eg(void);
 
+void pico8_pj_hk_dispatch(void *self, pico8_fixed_t yr);
+void pico8_pj_ph_dispatch(void *self, pico8_fixed_t t);
+
+void pico8_score_panel_display_init(void);
+void pico8_score_panel_display_shutdown(void);
+
 #endif
diff --git a/src/pico8_score_panel_display.c b/src/pico8_score_panel_display.c
index d87ee53..58aebd9 100644
--- a/src/pico8_score_panel_display.c
+++ b/src/pico8_score_panel_display.c
@@ -334,3 +334,13 @@ void pico8_score_panel_display_shutdown(void) {
 pico8_fq_my_fn pico8_de_my(void) {
     return pico8_pj_de_my_dispatch;
 }
+
+void pico8_pj_hk_dispatch(void *self, pico8_fixed_t yr) {
+    (void)self;
+    pico8_pj_hk(yr);
+}
+
+void pico8_pj_ph_dispatch(void *self, pico8_fixed_t t) {
+    (void)t;
+    pico8_pj_ph((pico8_pj_t *)self);
+}
diff --git a/src/pico8_score_panel_display.h b/src/pico8_score_panel_display.h
index 4ff2b39..092a474 100644
--- a/src/pico8_score_panel_display.h
+++ b/src/pico8_score_panel_display.h
@@ -15,9 +15,6 @@ void pico8_pj_ph(pico8_pj_t *self);
  * deliberately left stale in that same case, as in the Lua. */
 void pico8_pj_hk(pico8_fixed_t yr);
 
-void pico8_score_panel_display_init(void);
-void pico8_score_panel_display_shutdown(void);
-
 /* The function `de.my` holds: `fq(de)` of `ix[3]` reads nothing else. */
 pico8_fq_my_fn pico8_de_my(void);
 
diff --git a/src/pico8_slipway.c b/src/pico8_slipway.c
index 94adb7b..8c279f1 100644
--- a/src/pico8_slipway.c
+++ b/src/pico8_slipway.c
@@ -14,7 +14,6 @@
 #include "pico8_ke.h"
 #include "pico8_planet.h"
 #include "pico8_probe.h"
-#include "pico8_slipway_display.h"
 #include "pico8_tech_gate.h"
 #include "pico8_ui.h"
 #include "pico8_vec.h"
@@ -329,11 +328,6 @@ static void pico8_jc_ev_dispatch(void *self, const pico8_vec_t *ra) {
     pico8_jc_ev((pico8_jc_t *)self);
 }
 
-static void pico8_jc_kj_dispatch(void *self, const pico8_vec_t *ra) {
-    (void)ra;
-    pico8_jc_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jc_t *)self);
-}
-
 static int jc_is_endpoint_class(const pico8_struct_class_t *cls) {
     return cls->states[PICO8_STATE_LY] != NULL || cls == pico8_jo_struct_class();
 }
diff --git a/src/pico8_slipway.h b/src/pico8_slipway.h
index ac4cac3..75d1969 100644
--- a/src/pico8_slipway.h
+++ b/src/pico8_slipway.h
@@ -123,4 +123,6 @@ pico8_struct_header_t *pico8_jc_eb(const pico8_jc_t *self, const pico8_struct_he
 
 void pico8_jc_ev(pico8_jc_t *self);
 
+void pico8_jc_kj_dispatch(void *self, const pico8_vec_t *ra);
+
 #endif
diff --git a/src/pico8_slipway_display.c b/src/pico8_slipway_display.c
index f4f9f1c..b5d219b 100644
--- a/src/pico8_slipway_display.c
+++ b/src/pico8_slipway_display.c
@@ -4,6 +4,7 @@
 #include <stddef.h>
 #include <stdlib.h>
 
+#include "pico8_dispatch_context.h"
 #include "pico8_entity.h"
 #include "pico8_entity_geom.h"
 #include "pico8_planet.h"
@@ -170,3 +171,8 @@ void pico8_jc_kj(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jc_t *self) {
 
     self->fo = NULL;
 }
+
+void pico8_jc_kj_dispatch(void *self, const pico8_vec_t *ra) {
+    (void)ra;
+    pico8_jc_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jc_t *)self);
+}
diff --git a/src/pico8_wormhole.c b/src/pico8_wormhole.c
index 8e1ed89..fd18dd8 100644
--- a/src/pico8_wormhole.c
+++ b/src/pico8_wormhole.c
@@ -5,14 +5,12 @@
 #include <string.h>
 
 #include "pico8_audio.h"
-#include "pico8_dispatch_context.h"
 #include "pico8_entity.h"
 #include "pico8_entity_geom.h"
 #include "pico8_ke.h"
 #include "pico8_planet.h"
 #include "pico8_probe.h"
 #include "pico8_tech_gate.h"
-#include "pico8_wormhole_display.h"
 
 static int g_mz_up = 0;
 
@@ -184,11 +182,6 @@ pico8_fixed_t pico8_mz_ej(pico8_mz_t *self, pico8_fixed_t t) {
     return pico8_fixed_mul(min_val, oi);
 }
 
-static void pico8_mz_kj_dispatch(void *self, const pico8_vec_t *ra) {
-    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
-    pico8_mz_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_mz_t *)self, p);
-}
-
 /* Lua's `mz.hq=bv` is rebound three statements later by `function mz:hq`,
  * so only that second definition is ever reachable. Putting
  * pico8_bv_dispatch in the HQ entry instead would look faithful and would
diff --git a/src/pico8_wormhole.h b/src/pico8_wormhole.h
index 409fdd4..eddb744 100644
--- a/src/pico8_wormhole.h
+++ b/src/pico8_wormhole.h
@@ -75,4 +75,6 @@ void pico8_mz_pr(pico8_mz_t *self, pico8_fixed_t t);
 /* Reads `self.oi` only; `self` is non-const for signature uniformity. */
 pico8_fixed_t pico8_mz_ej(pico8_mz_t *self, pico8_fixed_t t);
 
+void pico8_mz_kj_dispatch(void *self, const pico8_vec_t *ra);
+
 #endif
diff --git a/src/pico8_wormhole_display.c b/src/pico8_wormhole_display.c
index a3c54de..d893f1e 100644
--- a/src/pico8_wormhole_display.c
+++ b/src/pico8_wormhole_display.c
@@ -4,6 +4,7 @@
 #include <stddef.h>
 #include <string.h>
 
+#include "pico8_dispatch_context.h"
 #include "pico8_entity.h"
 #include "pico8_fixed.h"
 #include "pico8_wormhole.h"
@@ -73,3 +74,8 @@ void pico8_mz_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_mz_t *self, pico8_vec
         }
     }
 }
+
+void pico8_mz_kj_dispatch(void *self, const pico8_vec_t *ra) {
+    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
+    pico8_mz_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_mz_t *)self, p);
+}
```

`git diff a b` (2.17b, 163 lignes, `548da3b7…`) :

```diff
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index 59245a7..274f3d7 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -7,7 +7,6 @@
 
 #include "pico8_audio.h"
 #include "pico8_collect.h"
-#include "pico8_dispatch_context.h"
 #include "pico8_economy.h"
 #include "pico8_entity.h"
 #include "pico8_entity_geom.h"
@@ -15,12 +14,9 @@
 #include "pico8_ke.h"
 #include "pico8_msgbox.h"
 #include "pico8_pi.h"
-#include "pico8_planet_display.h"
 #include "pico8_planet_family.h"
-#include "pico8_planet_trade.h"
 #include "pico8_probe.h"
 #include "pico8_research.h"
-#include "pico8_slipway.h"
 #include "pico8_tech_gate.h"
 
 static int g_jo_up = 0;
@@ -36,42 +32,15 @@ int pico8_jo_hd_dispatch(pico8_struct_header_t *self, const pico8_vec_t *qa, con
     return pico8_gk(self, qa, ra);
 }
 
-void pico8_jo_hb_dispatch(void *self) {
-    pico8_jo_hb((pico8_jo_t *)self);
-}
-
-pico8_down_t pico8_jo_gv_dispatch(void *self, const pico8_struct_header_t *s) {
-    return pico8_jo_gv((const pico8_jo_t *)self, s);
-}
-
 void pico8_jo_hk_dispatch(void *self, pico8_fixed_t yr) {
     (void)yr;
     pico8_jo_hk((const pico8_jo_t *)self);
 }
 
-void pico8_jo_kj_dispatch(void *self, const pico8_vec_t *ra) {
-    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
-    pico8_jo_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jo_t *)self, p);
-}
-
-pico8_down_t pico8_jo_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
-    (void)qa;
-    (void)ra;
-    return pico8_jo_ldown((pico8_jo_t *)self);
-}
-
 int pico8_jo_is_family(const pico8_struct_header_t *e) {
     return (e->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_JO)) != 0;
 }
 
-void pico8_jo_kf_dispatch(void *self, pico8_struct_header_t *sw) {
-    pico8_jo_kf((pico8_jo_t *)self, sw);
-}
-
-void pico8_jo_cn_dispatch(void *self, struct pico8_jo_trade_proposal *lw) {
-    pico8_jo_cn((pico8_jo_t *)self, lw);
-}
-
 void pico8_jo_ll_dispatch(void *self, pico8_fixed_t t) {
     pico8_jo_ll((pico8_jo_t *)self, t);
 }
@@ -433,27 +402,3 @@ void pico8_jo_hk(const pico8_jo_t *self) {
          * frame it stays on screen: not freed here. */
     }
 }
-
-pico8_down_t pico8_jo_ldown(pico8_jo_t *self) {
-    const char *me_val;
-    pico8_down_t out = {0, NULL};
-
-    if (self == NULL || self->header.me == NULL) {
-        return out;
-    }
-    me_val = self->header.me;
-    if (strcmp(me_val, "known") == 0) {
-        pico8_jo_fe(self);
-        out.stop = 1;
-        return out;
-    }
-    if (strcmp(me_val, "ly") == 0) {
-        pico8_jc_keys_t keys = {0};
-
-        assert(pico8_jc_ready() && "pico8_jc_init() must run before jo:ldown builds a jc");
-        keys.qu = (pico8_struct_header_t *)self;
-        out.held = (pico8_struct_header_t *)pico8_jc_new(&keys);
-        out.stop = out.held != NULL;
-    }
-    return out;
-}
diff --git a/src/pico8_planet_display.c b/src/pico8_planet_display.c
index ad47dca..ee86ef3 100644
--- a/src/pico8_planet_display.c
+++ b/src/pico8_planet_display.c
@@ -474,3 +474,46 @@ void pico8_jo_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec
         }
     }
 }
+
+void pico8_jo_hb_dispatch(void *self) {
+    pico8_jo_hb((pico8_jo_t *)self);
+}
+
+pico8_down_t pico8_jo_gv_dispatch(void *self, const pico8_struct_header_t *s) {
+    return pico8_jo_gv((const pico8_jo_t *)self, s);
+}
+
+void pico8_jo_kj_dispatch(void *self, const pico8_vec_t *ra) {
+    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
+    pico8_jo_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jo_t *)self, p);
+}
+
+pico8_down_t pico8_jo_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
+    (void)qa;
+    (void)ra;
+    return pico8_jo_ldown((pico8_jo_t *)self);
+}
+
+pico8_down_t pico8_jo_ldown(pico8_jo_t *self) {
+    const char *me_val;
+    pico8_down_t out = {0, NULL};
+
+    if (self == NULL || self->header.me == NULL) {
+        return out;
+    }
+    me_val = self->header.me;
+    if (strcmp(me_val, "known") == 0) {
+        pico8_jo_fe(self);
+        out.stop = 1;
+        return out;
+    }
+    if (strcmp(me_val, "ly") == 0) {
+        pico8_jc_keys_t keys = {0};
+
+        assert(pico8_jc_ready() && "pico8_jc_init() must run before jo:ldown builds a jc");
+        keys.qu = (pico8_struct_header_t *)self;
+        out.held = (pico8_struct_header_t *)pico8_jc_new(&keys);
+        out.stop = out.held != NULL;
+    }
+    return out;
+}
diff --git a/src/pico8_planet_trade.c b/src/pico8_planet_trade.c
index 183de4a..ace6b12 100644
--- a/src/pico8_planet_trade.c
+++ b/src/pico8_planet_trade.c
@@ -245,3 +245,11 @@ pico8_ents_t *pico8_jo_ib(pico8_jo_t *self) {
 
     return ib;
 }
+
+void pico8_jo_kf_dispatch(void *self, pico8_struct_header_t *sw) {
+    pico8_jo_kf((pico8_jo_t *)self, sw);
+}
+
+void pico8_jo_cn_dispatch(void *self, struct pico8_jo_trade_proposal *lw) {
+    pico8_jo_cn((pico8_jo_t *)self, lw);
+}
```
