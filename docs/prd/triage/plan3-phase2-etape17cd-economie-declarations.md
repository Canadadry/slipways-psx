---
title: "Plan 3, étapes 2.17c et 2.17d : économie (T3, T6) puis déclarations (T2, T4) ; sept paires d'inclusion et deux paires d'usage cachées coupées par l'usage, `inclusion-mutuelle` 9 → 7 → 2 paires — trois commits (c, d1, d2)"
description: "2.17c : `pico8_dj` prend deux points (`pico8_vec_t`), le pas `kr` de `hy:lj` devient `pico8_kr` dans `economy_score` ; 2.17d1 : en-tête feuille neuf `pico8_fm.h` (`pico8_hy_def_t`, `pico8_hy_t`, créneau `fm`) ; 2.17d2 : en-tête neuf `pico8_probe_comp.h` (le composant sonde, choisi contre `particle.h` par mesure), identités `pico8_jc_struct_class`/`pico8_g__struct_class` dans `struct.h`, `planet.h:11` coupée. Mesuré sur l'arbre d'après 2.17b (rejoué par `run217a.py`/`run217b.py`, hachages vérifiés) : `inclusion-mutuelle` 9 / 9 (22 lignes, 13 fichiers) → 7 / 7 (18, 11) → 7 / 7 (17, 11) → 2 / 3 (5, 4) ; graphe d'usage 12 → 10 → 8 → 3 ; aucune paire entrante ; relevé d'usage vide dans chaque sens coupé ; aucun include mort créé. Objets : d1, d2 identiques octet pour octet sous `-DNDEBUG` (74 / 74, trois niveaux) ; c : 4 fonctions touchées, 6 jumelles sans ÉCHEC, 0 ligne perdue. Aucune question à l'humain."
status: needs-triage
---

## Problem Statement

**Écrit sur `0f2db15`** (branche `claude/plan-pilot-3-7zdft0`). Départ réel attendu : **le commit de 2.17b**, dont l'implémentation était en cours dans l'arbre de travail du dépôt pendant l'écriture (fichiers non commités, non lus). L'arbre des essais est `0f2db15` (arbres de 2.16 : `src` `148603f3…`, `tests` `cac32597…`, `Makefile` `422546ca…`) + `run217a.py` + `run217b.py` (annexe A du PRD 2.17a–b, `docs/prd/triage/plan3-phase2-etape17ab-affichage.md`), rejoués sur une copie : **`src` `e8e76840…` puis `3c5385b8…`**, diffs `90747520…`, `548da3b7…`, cumul `944b21ab…` — exactement ceux que le PRD 2.17a–b annonce. Pendant l'écriture, 2.17a et 2.17b ont été commitées (`5729d73`, `6b03062`, notées à `c993ded`) : l'arbre `src` de `6b03062` est `3c5385b8…`, `tests` et `Makefile` inchangés — le départ est donc `6b03062` ou un descendant qui ne touche que `docs/`. Notation : `b` = le commit de la copie qui porte 2.17b (départ) ; `c` = 2.17c ; `d1`, `d2` = les deux commits de 2.17d. Numéros de ligne : ceux de `b` pour 2.17c, de `c` pour d1, de `d1` pour d2, sauf mention. Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier ; aucun `make` lancé dans le dépôt. Essais sur une seule copie jetable (scratchpad `prd217cd/`, clone, arbres de travail `wt_b`, `wt_c`, `wt_d1`, `wt_d2`, `c_b`, `c_c`, `c_d1`, `c_d2` propres pour la couverture et les filets, `wt_r` pour les rejeux), supprimée depuis.

Vu du mainteneur du port : après 2.17b, le compteur de composition ne compte plus que `inclusion-mutuelle` — **9 paires, 22 lignes `#include`, 13 fichiers** — et le graphe d'usage 12 paires (les 9, plus trois cachées : `economy` ↔ `economy_score`, `economy` ↔ `tech_gate`, `entity_geom` ↔ `scroll`). Sept de ces paires et deux des cachées ne tiennent qu'à une signature ou à un rangement de déclarations :

- `pico8_dj` (coût d'une voie, `economy_formulas`) reçoit deux entités et lit leur point par `pico8_jc_end_p` de `slipway` ; `pico8_hy_lj` (`economy`) écrit `kr` dans chaque planète (`pico8_jo_t`, test de famille) : deux paires pour deux lectures.
- `probe.h` mélange trois choses — le composant sonde, l'entité `g_`, le créneau `fm` (qui rend le `hy` d'`economy`) — et `economy.h` porte `pico8_hy_t` ; tout porteur de sonde (`slipway`, `wormhole`, `planet`) et tout lecteur de `fm` inclut donc `probe.h`, que `probe.c` retourne en incluant leurs en-têtes.
- `ui.c` n'inclut `probe.h` et `slipway.h` que pour deux identités de classe lues par un `assert`.

### Texte du plan, recopié (`docs/plan-3.md` à `0f2db15`)

> - 2.17 — `inclusion-mutuelle`, selon l'étude 2.11 (`docs/etude-2-11-inclusions.md`, §2 à §4) : les quinze paires qui restent après 2.14b et 2.15d, et les trois paires d'usage cachées, coupées **par l'usage** (dans un sens, plus aucun nom de l'autre en-tête employé, et plus d'include) ; chaque sous-étape relance au départ et au commit le graphe d'usage de l'étude (annexe A) : les paires attendues sortent, aucune n'entre ; une ligne retirée qui compile par un détour est un `FAILED:`. […] `class-coverage-net` : non à chaque sous-étape (aucune table ni constructeur ne bouge). `sanitize-net` : non. Budget ≈ 6 × 50 min.
>   - 2.17c — T3 et T6, économie : `pico8_dj(pico8_vec_t, pico8_vec_t)` ; le pas `kr` de `hy:lj` (`economy.c:219-226`) devient une fonction d'`economy_score` ; retire `economy_formulas.c:10`, `economy.c:15`. Compte : → **7 / 17** (10) ; usage → 10. Preuves : chemin touché (`gcov` de `pico8_dj`, `pico8_jc_hq`, `pico8_hy_lj` ; mutations : `pico8_dj` sur deux fois la même extrémité, `has_kr` laissé à 0, `flr` retiré).
>   - 2.17d — T2 et T4, déclarations : en-tête neuf `pico8_fm.h` (`pico8_hy_t`, créneau `fm`), en-tête neuf `pico8_probe_comp.h` (le composant sonde), identités `pico8_jc_struct_class`, `pico8_g__struct_class` déclarées dans `struct.h` ; retire `planet.c:21`, `ui.c:13`, `:14`, remplace `slipway.c:17`, `slipway.h:7`, `wormhole.c:13`, `wormhole.h:11` ; 17 TU gagnent `#include "../src/pico8_fm.h"`. Compte : → **2 / 5** (4) ; usage → 3 (dont les deux paires cachées d'`economy`). Preuves : refactor pur, objets sans `-g` identiques sous `-DNDEBUG` (essai : 73 / 73). Repli : d1 (`pico8_fm.h`), d2 (le reste).

**Comptes du plan remesurés.** Le PRD 2.17a–b a annoncé une ligne de plus que l'étude à chaque bout tant que `planet.h:11` → `probe.h` reste (entrée à 2.15c, `pico8_probe_t probe_own` dans `pico8_jo_t`). Mesures réelles (compteur : `inclusion-mutuelle <paires> <modules>` au pied, lignes par `--check`, fichiers au pied ; usage : `usegraph.py`) :

| Après | Paires | Modules | Lignes `#include` | Fichiers (pied) | Graphe d'usage | Plan / étude |
|---|---|---|---|---|---|---|
| 2.17b (`b`, départ) | 9 | 9 | 22 | 13 | 12 | 9 / 21 (12), 12 |
| 2.17c | **7** | **7** | **18** | **11** | **10** | 7 / 17 (10), 10 |
| 2.17d1 | **7** | **7** | **17** | **11** | **8** | (repli d1 : non chiffré) |
| 2.17d2 | **2** | **3** | **5** | **4** | **3** | 2 / 5 (4), 3 |

À `c`, la ligne de plus est encore `planet.h:11` (et le fichier de plus `planet.h`) ; d2 la coupe (contrat du PRD 2.17a–b) et l'arrivée est **exactement** celle de l'étude. **Écart de texte du plan** : « usage → 3 (dont les deux paires cachées d'`economy`) » est inexact — les deux paires cachées d'`economy` **sortent** à d1 ; les 3 qui restent sont `entity_geom` ↔ `planet`, `entity_geom` ↔ `scroll`, `planet` ↔ `planet_family` (celles de 2.17e ; l'étude, §4, écrivait « usage → 3 (c1, c2 sortent) »).

Numéros du plan datés (à `b`) : le pas `kr` est `economy.c:226-233` (plan : `:219-226`) ; `economy.c:15` et `economy_formulas.c:10` inchangés ; `pico8_dj` est appelé à `slipway.c:120`. À `c` : `planet.c:18` → `probe.h` (plan : `:21`), `slipway.c:16` (plan : `:17`), `wormhole.c:12` (plan : `:13`), `slipway.h:7`, `wormhole.h:11`, `ui.c:13`, `:14` inchangés.

Étude 2.11 (fiches 1, 2, 8, 11, 12, 13, 16, c1, c2 ; §4) : techniques T2f, T2p, T3, T4, T6 ; essais `t3_dj`, `kr_score`, `t2_fm`, `probe_comp`, `t4_ui` (annexe A de l'étude). Ce PRD en reprend la forme, rejouée sur `b` : deux textes ont changé depuis l'étude (`pico8_hy_t` porte `def` depuis 2.12a/2.13a1, `pico8_probe_t` porte `def` depuis 2.12b/2.13b ; la boucle de `hy:lj` itère par `pico8_ents_iter_next`), d'où des scripts neufs (annexe A).

Décisions de l'humain : **Q2** — « les paires d'inclusion mutuelle sont démêlées pendant la phase 2 ; `inclusion-mutuelle` est une condition de sortie, à zéro » : 2.17c et 2.17d en retirent sept, plus deux paires d'usage. Q1, Q3, Q4, **planètes A**, Q-2.10, Q-2.14d : sans objet (aucun champ, lien ni table ne bouge).

Règles communes du plan : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte (`*_HASH`, `FROZEN_*`) changée ; `class-coverage-net` et `sanitize-net` « non » (lancés quand même, verts). Ce PRD les tient toutes.

### Contrats reçus et ce que ce PRD en fait

| Point (source) | Ce que font 2.17c et 2.17d |
|---|---|
| coupe **par l'usage**, plus d'include ; un détour est un `FAILED:` (plan, étude §0) | relevé `usedir.py` de chaque sens coupé : **0 nom**, **0 nom partagé**, **0 include** au commit (preuve 2) |
| graphe d'usage relancé au départ et au commit, aucune paire n'entre (plan) | 12 → 10 → 8 → 3 ; `--detail` 9 → 7 → 7 → 2 ; seules des sorties (preuve 3) |
| 2.17c : chemin touché, `gcov` de `pico8_dj`, `pico8_jc_hq`, `pico8_hy_lj` ; mutations (plan) | ensemble `L` et lignes neuves couvertes (preuve 6) ; 6 jumelles à la règle du parent, dont les trois du plan (preuve 5) |
| 2.17d : refactor pur, objets identiques sous `-DNDEBUG` (plan) | **74 / 74** objets identiques octet pour octet à `-O0`, `-O1`, `-O2 -DNDEBUG`, pour d1 comme pour d2 (preuve 4) |
| **2.17d doit aussi couper `planet.h:11` → `probe.h`** (PRD 2.17a–b, contrat) | tenu : `planet.h` inclut `pico8_probe_comp.h` et `pico8_particle.h` (il recevait `pico8_emitter_t` par `probe.h`) ; `particle.c:12` → `planet.h`, include mort, part (sinon `particle` ↔ `planet` entre au compteur, Runs 8-9) |
| réserve de `0acfad8` : `particle.h` (essayé : −3 paires, aucune entrante) ou `pico8_probe_comp.h`, avec justification | **`pico8_probe_comp.h`**, tranché par mesure (« Le choix du rangement du composant ») |
| réserve de `0acfad8` : « aucun en-tête existant » = « aucun qui ait un sens » pour `pico8_fm.h` | mesuré : 38 modules sur 79 pourraient le déclarer sans paire, aucun qui ait un sens ; les cinq qui en ont un créent chacun une paire nommée (« Le choix de `pico8_fm.h` ») |
| réserve de `0acfad8` : les 17 includes des TU sont facultatifs | facultatifs **tant que `probe.h` inclut `economy.h`** ; cet include devient mort à d1 et part (G2), 11 des 17 TU deviennent obligatoires (Runs 6, 25) ; les 17 suivent la même règle (G4) |
| réserve de `0acfad8` : `struct.h` déclarerait deux identités de famille | assumé, précédent et prix dits (G6) ; `struct.h` n'inclut rien de neuf, `noyau-inclut` 0 |
| numéros de ligne d'après 2.17b (PRD 2.17a–b, contrat) | relus à `b` (ci-dessus) |
| repli d1/d2 (plan) | pris d'emblée : trois commits c, d1, d2, chacun ≈ 35 à 40 min (G1) |

### État de départ attendu (le commit de 2.17b)

Mesuré sur `b` ; identique aux comptes d'arrivée du PRD 2.17a–b :

| Commande | Résultat |
|---|---|
| `git rev-parse HEAD:src HEAD:tests` ; `git ls-files -s Makefile` | `3c5385b892f44317b1a42cae2108b40bf727e5d9`, `cac3259733163ed40d59198d02463eb9618e13a2` ; `422546ca868cfbdd089c8348b8eb4806763b9010` |
| `make composition-residue` | `inclusion-mutuelle 9 9` seule ; **`résidu 2 : 22 ligne(s), 13 fichier(s)`** |
| `--check inclusion-mutuelle src tests` | 22 lignes |
| `--champs \| tail -1` ; casts / tests de classe (greps de 2.15d) | `champs : 238 ; 0 écriture : 14 ; 0 hors constructeur : 19` ; 464 / 647 |
| `usegraph.py` ; arêtes orientées ; composantes fortement connexes > 1 | `paires 12` ; 617 ; une de 24 |
| `COMPOSITION_RESIDUE_GUARD` | 12 catégories, finit par `,parente,void-ptr` |
| `make test` ; `make e2e` ; `MALLOC_PERTURB_=165 make e2e` | vert ; 26 conformes ; 26 conformes |

Autre départ (2.17b commitée autrement que par son script, ou suivie d'un commit qui touche `src/`) : repli 2.

## Solution

Trois commits, chacun vert, jeu identique (26 e2e conformes, aussi sous `MALLOC_PERTURB_=165`), aucune constante d'empreinte changée. Chaque commit est le résultat de son script (annexe A, texte exact), plus la note du plan et le message.

**2.17c** (`python3 run217c.py <arbre>` ; diff de 118 lignes, 6 fichiers, +20 / −13) :
1. **T3** — `pico8_dj(const pico8_struct_header_t *pb, const pico8_struct_header_t *to)` devient `pico8_dj(pico8_vec_t pb_p, pico8_vec_t to_p)` (`economy_formulas.h:38`, `.c:132-133`) ; son corps soustrait `to_p - pb_p` au lieu de `pico8_jc_end_p(to) - pico8_jc_end_p(pb)` ; l'appelant (`slipway.c:120`, `pico8_jc_hq`) passe `pico8_jc_end_p(self->qu)`, `pico8_jc_end_p(self->re)`. `pico8_jc_end_p` ne fait que lire (`slipway.c:32-36`) : l'ordre d'évaluation des deux arguments, non spécifié avant (arguments de `pico8_vec_sub`) comme après, est sans effet. `economy_formulas.h` inclut `pico8_vec.h` et perd `pico8_struct.h` (plus aucun nom employé, G2) ; `economy_formulas.c` perd `#include "pico8_slipway.h"` (`:10`). Aucun TU n'appelle `pico8_dj`.
2. **T6** — le pas de boucle de `pico8_hy_lj` (`economy.c:226-232` : `kr = flr(db(p) - da(p))`, `assert` de famille, cast, écriture de `kr` et `has_kr`) devient `pico8_fixed_t pico8_kr(pico8_struct_header_t *p)`, **ajoutée en fin d'`economy_score.c`** (qui définit `pico8_da`/`pico8_db` et inclut déjà `planet.h`, `<assert.h>`), déclarée en fin d'`economy_score.h` avec le contrat « Writes `kr` into planet `p` (asserted) and returns it. » ; `pico8_hy_lj` garde la boucle et la somme `dd` (`pico8_fixed_t kr = pico8_kr(p);`). `economy.c` perd `#include "pico8_planet.h"` (`:15`). `db(p)` et `da(p)` restent les deux arguments d'une même expression (ordre inchangé). Le texte de l'`assert` ne change pas (« hy:lj: cg.jo holds planets only »). Nom `pico8_kr` : celui de l'étude, provisoire (phase 3, `kr` = « revenu net de la planète », glossaire).

**2.17d1** (`python3 run217d.py <arbre> d1` ; diff de 563 lignes, 32 fichiers dont 1 neuf, +101 / −73) :
3. **T2f** — en-tête feuille neuf **`src/pico8_fm.h`** (sans `.c`) : `pico8_hy_def_t` et `pico8_hy_t` avec leurs commentaires (coupés de `economy.h:12-44` ; seul changement de texte : « economy.h does not include » → « pico8_fm.h does not include », le commentaire parle de l'en-tête qui le porte), puis le créneau `fm` avec ses commentaires (coupé de `probe.h:86-112` : `pico8_fm_placeholder_t`, `pico8_fm_placeholder_struct_class`, `pico8_fm`, `pico8_probe_set_fm`, `pico8_fm_hy`, `pico8_fm_placeholder`). Il inclut `economy_formulas.h` (`pico8_qe_map_t`), `fixed.h`, `ptrs.h`, `struct.h`. Les définitions ne bougent pas (`economy.c`, `probe.c`). Pas d'étiquette `struct pico8_hy` (l'étude la prévoyait pour une déclaration anticipée qu'aucune coupe n'emploie : G5).
4. `economy.h` inclut `pico8_fm.h` et perd `economy_formulas.h`, `ptrs.h` (plus aucun nom employé, G2) ; `probe.h` perd `economy.h` (G2).
5. Chaque fichier qui emploie un nom de `pico8_fm.h` l'inclut à sa place alphabétique : **12 `.c` de `src/`** (`cart`, `cheat`, `economy`, `economy_score`, `menu`, `planet`, `planet_display`, `probe`, `score_panel`, `score_panel_display`, `slipway`, `tech_gate`) et **17 TU** (`fm`, `fr`, `g_`, `hj`, `hp`, `hy`, `ie`, `jc`, `js`, `mx`, `mz`, `nl`, `nr`, `os`, `pj`, `qh`, `spec_defaults`), en `"../src/pico8_fm.h"`.
6. Retirés : `planet.c:18` → `probe.h` (la coupe de la fiche 8) et, devenus morts, `#include "pico8_probe.h"` de `cheat.c`, `economy_score.c`, `menu.c`, `planet_display.c`, `score_panel.c`, `score_panel_display.c`, `tech_gate.c` (ils n'employaient de `probe.h` que des noms de `fm`) — 11 lignes en tout avec les trois du point 4.

**2.17d2** (`python3 run217d.py <arbre> d2` ; diff de 276 lignes, 10 fichiers dont 1 neuf, +76 / −65) :
7. **T2p** — en-tête neuf **`src/pico8_probe_comp.h`** (sans `.c`) : `pico8_probe_t` (coupé de `probe.h:18-28`, étiquette `struct pico8_probe` déjà là) puis les onze déclarations du composant avec leurs commentaires (coupées de `probe.h:85-124`, depuis le `/* TROU-7 */` de `pico8_bv` : `pico8_bv`, `pico8_bv_dispatch`, `pico8_probe_et_hang_fo`, `pico8_bv_et_t`, `pico8_bv_et_struct_class`, `pico8_dl`, `pico8_dl_show`, `pico8_dl_dispatch`, `pico8_d_`, `pico8_d__dispatch`). Il inclut `fixed.h`, `game_data.h` (`pico8_probe_def_t`), `gfx.h`, `mem.h`, `struct.h`, `vec.h`. `probe.h` l'inclut (`pico8_g__t` enchâsse un `pico8_probe_t`) et perd `gfx.h`, `mem.h` (G2).
8. `slipway.h:7`, `wormhole.h:11`, **`planet.h:11`**, `slipway.c:17`, `wormhole.c:12` incluent `pico8_probe_comp.h` au lieu de `pico8_probe.h` ; `planet.h` inclut aussi `pico8_particle.h` (`pico8_emitter_t`, reçu jusque-là par `probe.h`) ; `particle.c:12` → `planet.h` (0 nom employé, préexistant) part, sans quoi il ferait avec cette ligne la paire `particle` ↔ `planet`.
9. **T4** — `pico8_jc_struct_class` et `pico8_g__struct_class` quittent `slipway.h:78` et `probe.h:60` pour la fin de `struct.h`, sous le commentaire « Here, not in pico8_slipway.h / pico8_probe.h: ui.c compares against them, and including either header from ui.c makes an include pair. » (un piège qu'on réintroduirait) ; définitions inchangées ; `ui.c` perd `:13` (`probe.h`) et `:14` (`slipway.h`). `struct.h` n'inclut rien de neuf.

Rien d'autre : aucune table de classe, aucun corps hors ceux de 2.17c, aucun nom hors `pico8_kr` (neuf) et les deux paramètres de `pico8_dj` ; ni `tools/`, ni le `Makefile` (dépendances d'en-têtes par `-MMD`), ni `*.sha256`, ni `docs/glossaire-noms.md`, ni `docs/trous-de-portage.md`. `tests/` : seulement les 17 lignes `#include` de d1.

### Le choix de `pico8_fm.h` (réserve de `0acfad8`)

« Aucun en-tête existant ne convient » (étude, fiche 8) veut dire **aucun qui ait un sens**, pas aucun qui compile sans paire. Mesure (`hosts.py`, annexe A, sur les arêtes orientées de `c`) : un en-tête M peut recevoir les noms de `fm` sans paire d'usage si M n'emploie aucun des 12 modules qui les emploient et si aucune des nouvelles dépendances de M (`economy_formulas`, `fixed`, `ptrs`, `struct`) n'emploie M. **38 modules sur 79 passent** (`audio`, `collect`, `dispatch_context`, `gen`, `gfx`, `hud`, `input`, `ke`, `layout`, `mem`, `mouse`, `msgbox`, `palette`, `particle`, `pi`, `scroll`, `str`, `trade`, `wormhole_display`…) : aucun n'a rapport à l'économie. Ceux qui en ont un échouent chacun sur une paire nommée :

| Hôte | Paire créée (arête existante qui revient) |
|---|---|
| `economy.h` | `economy` → `economy_score` (`pico8_bf`, `pico8_cz`, `pico8_kr`), `economy` → `tech_gate` (`pico8_gi_nb`, `pico8_ov`) : c1, c2 restent |
| `economy_formulas.h` | `economy_formulas` → `planet` (`pico8_jo_is_family`, `pico8_jo_t`) |
| `economy_score.h` | `economy_score` → `planet`, → `probe`, → `economy` |
| `tech_gate.h` | `tech_gate` → `economy`, → `probe` |
| `probe.h` (l'actuel) | `probe` → `planet`, `slipway`, `wormhole`, `economy` |
| `struct.h` | `economy_formulas` → `struct` (et le noyau inclurait l'économie) |

### Le choix du rangement du composant sonde (réserve de `0acfad8`)

Mesuré en rejouant d2 avec `COMP=particle` (le composant ajouté en fin de `particle.h`, les cinq lignes incluant `particle.h`) : **mêmes comptes** que `pico8_probe_comp.h` — 2 / 3, 5 / 4, usage 3, 0 erreur, aucune paire entrante (Runs 10, 25). Le compte ne départage donc pas. Ce qui départage, mesuré sur les arêtes orientées (`usegraph_e.py`, Runs 11) :

| | `pico8_probe_comp.h` (retenu) | `particle.h` |
|---|---|---|
| arêtes sortantes du module hôte | `fixed`, `game_data`, `gfx`, `mem`, `struct`, `vec` : **une feuille**, qu'aucun de ces six n'emploie ; aucune paire ne peut passer par elle | `particle` emploie aussi `dispatch_context`, `entity_geom`, `ptrs`… et est dans la composante fortement connexe de 25 modules (`particle` → `entity_geom` → `planet` → `particle`) |
| modules qui emploient l'hôte | ceux qui emploient le composant (8) | les mêmes, plus ceux de l'émetteur ; **`slipway`, `wormhole`, `research_icon` deviennent « employeurs de `particle` »** sans employer une particule |
| risque de paire future | nul | tout usage de `slipway`, `wormhole`, `research_icon` ou `help` par `particle.c` fait une paire au compteur (le cas s'est présenté : `particle.c:12` incluait `planet.h`) |
| arêtes du graphe | 627 | 616 |
| fichiers `.c` qui voient l'en-tête | 71 / 158 | 72 / 158 |
| sens | le composant a son en-tête ; la garde de 2.17f lira des arêtes vraies | `particle.h` déclarerait `bv`, `dl`, `d_` (cible, infobulle), définis dans `probe.c` : le graphe dirait « `slipway` emploie `particle` » |

Prix du retenu : un fichier de plus, une ligne de plus dans `probe.h` et `planet.h`. **Retenu : `pico8_probe_comp.h`.**

### Comptes, mesurés sur la copie

| Mesure | `b` | `c` | `d1` | `d2` |
|---|---|---|---|---|
| `inclusion-mutuelle` (paires / modules) | 9 / 9 | **7 / 7** | **7 / 7** | **2 / 3** |
| lignes du `--check inclusion-mutuelle` ; `cross.py` | 22 | **18** | **17** | **5** |
| pied `résidu 2` (lignes / fichiers) | 22 / 13 | **18 / 11** | **17 / 11** | **5 / 4** |
| graphe d'usage (`usegraph.py`) | 12 | **10** | **8** | **3** |
| arêtes orientées ; composantes fortement connexes > 1 | 617 ; 24 | 615 ; 24 | 619 ; **25** | 627 ; 25 |
| autres catégories | 0, 12 gardées | idem | idem | idem |
| `--champs` ; casts ; tests de classe | 238 ; 14 ; 19 ; 464 ; 647 | idem | idem | idem |
| includes morts dans les fichiers touchés (`deadinc.py`) | — | parent 1 / enfant 1 | 17 / 17 | 11 / **10** (`particle.c:12` part) |
| constantes `*_HASH`/`FROZEN_*` changées | — | 0 | 0 | 0 |

**Paires sortantes** — 2.17c : `economy` ↔ `planet`, `economy_formulas` ↔ `slipway` (compteur et usage). 2.17d1 : `economy` ↔ `economy_score`, `economy` ↔ `tech_gate` (usage seul : le compteur ne les voyait pas). 2.17d2 : `planet` ↔ `probe`, `probe` ↔ `slipway`, `probe` ↔ `ui`, `probe` ↔ `wormhole`, `slipway` ↔ `ui` (compteur et usage). **Paires entrantes : aucune**, à aucun des trois commits.

**Lignes du `--check`** — 2.17c, sortent 4 : `economy.c:15`, `economy_formulas.c:10` (coupées), `planet.c:10` → `economy.h`, `slipway.c:10` → `economy_formulas.h` (gardées : la paire n'existe plus). 2.17d1, sort 1 : `planet.c:18` → `probe.h` ; `planet` ↔ `probe` reste par `planet.h:11` jusqu'à d2 ; les autres lignes de `probe.c`, `slipway.c`, `planet.c` se décalent de +1 (include de `fm.h`). 2.17d2, sortent 12 : `planet.h:11`, `probe.c:19-22` (`planet.h`, `slipway.h`, `ui.h`, `wormhole.h`, gardées), `slipway.c:17`, `:19` (`ui.h`, gardée), `slipway.h:7`, `ui.c:13`, `:14`, `wormhole.c:12`, `wormhole.h:11`. Restent 5 : `entity_geom.c:9`, `planet.c:12`, `:18`, `planet_family.c:16`, `planet_family.h:8`.

**Cycle** (hors périmètre, signalé) : à d1, `fm` entre dans la composante fortement connexe (24 → 25 : `fm` → `economy_formulas` → `planet` → `fm`, par `pico8_qe_map_t` et `pico8_jo_is_family`). Ce n'est pas une paire ; l'étude (§3) exclut les cycles longs de la Sortie.

### Contrat pour 2.17e et 2.17f (à recopier dans leurs PRD)

| Point | Pour | Ce qu'il faut savoir |
|---|---|---|
| départ : `inclusion-mutuelle` **2 / 3, 5 lignes / 4 fichiers**, usage **3** (`entity_geom` ↔ `planet`, `entity_geom` ↔ `scroll`, `planet` ↔ `planet_family`) | 2.17e | les comptes de l'étude sont atteints : plus de ligne en trop |
| numéros à `d2` : `entity_geom.c:9` → `planet.h` ; `planet.c:12` → `entity_geom.h`, **`:18` → `planet_family.h`** (plan : `:19`) ; `pico8_gd` à `entity_geom.c:44`, `pico8_na` à `:55` ; `g_kv` à `planet.c:283-297`, `jo_kv` à `:289` | 2.17e | `planet.c` n'a pas bougé entre `b` et `d2` au-delà de ses includes (nombre de lignes égal) |
| détour résiduel : `planet.c` voit encore `probe.h` par `planet_family.h:9` ; relevé `planet` > `probe` vide (0 nom, 0 include) | 2.17e | tombe avec `planet.c:18` ; ne pas le compter comme usage |
| la mutation de preuve proposée par `0acfad8` (un appel à `pico8_jo_is_family` dans `economy.c`, sans include direct) **compile à `d2`** par `economy_score.h` → `ui_button.h` → `planet.h` : compteur inchangé (2), usage 3 → **4** (`economy` ↔ `planet` entre) (Runs 27) | 2.17f | détour utilisable pour la garde mordante |
| `pico8_fm.h` et `pico8_probe_comp.h` n'ont pas de `.c` : leurs noms sont définis dans `probe.c`/`economy.c`, mais `usegraph.py` les attribue au module `pico8_fm`/`pico8_probe_comp` | 2.17f | le portage `awk` doit rendre les mêmes 3 paires que `usegraph.py` à `d2` |
| `pico8_kr`, `pico8_fm.h`, `pico8_probe_comp.h` : noms provisoires | phase 3 | à reprendre avec le glossaire |

### Anti-tassement

**Interdit** :
- retirer une ligne `#include` sans retirer l'usage (un détour, que le relevé `usedir.py` doit rendre vide), redéclarer un nom dans le fichier qui coupe (prototype local, `extern`), ou remettre `probe.h` → `economy.h` pour se passer des includes de `pico8_fm.h` ;
- ranger le composant dans `particle.h`, un nom de `fm` dans un autre en-tête que `pico8_fm.h`, ou les identités ailleurs que dans `struct.h` ;
- changer un corps hors `pico8_dj`, `pico8_jc_hq`, `pico8_hy_lj`, `pico8_kr` ; renommer une fonction (hors les paramètres de `pico8_dj`) ; toucher une table de classe ; changer un commentaire déplacé (hors le mot « economy.h » → « pico8_fm.h » du commentaire de `pico8_hy_def_t`) ;
- toucher `tools/`, le `Makefile`, un `*.sha256`, une constante `*_HASH`/`FROZEN_*`, `docs/glossaire-noms.md`, `docs/trous-de-portage.md` ; toucher `tests/` hors les 17 lignes `#include` de d1 ;
- fondre c, d1, d2 ; réécrire à la main ce que les scripts font, sauf repli.

## User Stories

1. En tant que mainteneur, je veux que la formule de coût (`economy_formulas`) et le gestionnaire (`economy`) n'emploient plus `slipway` ni `planet`, afin que l'économie calcule sur des valeurs et que l'entité appelle le gestionnaire, pas l'inverse.
2. En tant que mainteneur, je veux que les porteurs de sonde (`slipway`, `wormhole`, `planet`) et les lecteurs de `fm` n'incluent plus `probe.h`, afin que le composant et le créneau aient chacun leur en-tête et que `probe` ne soit plus au centre de cinq paires.
3. En tant que gardien du compteur, je veux `inclusion-mutuelle` à 7 puis 2 paires, chaque coupe prouvée par l'usage, et aucun include mort créé en chemin.
4. En tant que gardien de la règle du parent, je veux que chaque mutation rouge au parent le soit à l'enfant, et que les déclarations déplacées laissent le code machine intact.
5. En tant qu'auteur de 2.17e et 2.17f, je veux les comptes et numéros d'arrivée réels, et un détour de preuve qui compile encore.

## Implementation Decisions

### Modules

2.17c : `economy.c`, `economy_formulas.c/.h`, `economy_score.c/.h`, `slipway.c` ; `docs/plan-3.md` (note « Découvert à 2.17c »). 2.17d1 : `pico8_fm.h` (neuf), `economy.h`, `probe.h`, les 12 `.c` du point 5, les 17 TU ; `docs/plan-3.md` (note « Découvert à 2.17d1 »). 2.17d2 : `pico8_probe_comp.h` (neuf), `probe.h`, `planet.h`, `slipway.c/.h`, `wormhole.c/.h`, `struct.h`, `ui.c`, `particle.c` ; `docs/plan-3.md` (note « Découvert à 2.17d2 », qui clôt la ligne 2.17d).

### Inclusions

| Commit | Retirées | Ajoutées |
|---|---|---|
| 2.17c | `economy.c:15` (`planet.h`), `economy_formulas.c:10` (`slipway.h`), `economy_formulas.h:4` (`struct.h`, G2) | `economy_formulas.h` : `pico8_vec.h` |
| 2.17d1 | `planet.c:18` (`probe.h`) ; `probe.h:4` (`economy.h`) ; `economy.h:4` (`economy_formulas.h`), `:7` (`ptrs.h`) ; `probe.h` de `cheat.c:4`, `economy_score.c:13`, `menu.c:19`, `planet_display.c:18`, `score_panel.c:20`, `score_panel_display.c:13`, `tech_gate.c:10` (G2) — 11 | `pico8_fm.h` : `economy.h` + 12 `.c` + 17 TU = 30 |
| 2.17d2 | `probe.h` de `planet.h:11`, `slipway.h:7`, `wormhole.h:11`, `slipway.c:17`, `wormhole.c:12` (remplacés) ; `probe.h:5` (`gfx.h`), `:7` (`mem.h`), G2 ; `ui.c:13`, `:14` ; `particle.c:12` (`planet.h`, mort) | `pico8_probe_comp.h` : `probe.h` + les 5 remplacés ; `planet.h` : `pico8_particle.h` |

**Aucun include mort créé** (`deadinc.py` sur les fichiers modifiés et neufs, au parent et à l'enfant, Runs 20, 22) : 1 → 1 (c), 17 → 17 (d1), 11 → 10 (d2 : `particle.c:12` part) ; les listes restantes sont préexistantes (hors périmètre, comme au PRD 2.17a–b).

### Ce qui change

Texte exact : résultat des scripts. Sur un arbre propre au commit de 2.17b (arbres de la table « État de départ »), après `git add -A` :

| Commit | Commande | `src` | `tests` | diff (`sha256sum`, lignes) |
|---|---|---|---|---|
| 2.17c | `python3 run217c.py .` | **`9af245927d51f58b15a2d7dd55ae6fd730cf52e6`** | `cac32597…` (inchangé) | `5821adac…`, 118 |
| 2.17d1 | `python3 run217d.py . d1` | **`ad162508623b2b34a887a5b7de9046d36bc0bd48`** | **`ea2eb6913cc25857b60c0adfae142b8ef44c4d73`** | `1ac5ea64…`, 563 |
| 2.17d2 | `python3 run217d.py . d2` | **`b5a5cb597049fbf0ab6c7d4cc952bbc30450fca2`** | `ea2eb691…` | `0954e753…`, 276 |

`Makefile` inchangé (`422546ca…`). Cumul `git diff b d2 | sha256sum` → `777e6a21…`. Rejoués sur un arbre de travail neuf à `0f2db15` (`run217a.py`, `run217b.py`, `run217c.py`, `run217d.py d1`, `d2`) : mêmes hachages ; relancer `run217c.py` ou `run217d.py d2` sur l'arbre d'arrivée s'arrête (`AssertionError`) (Runs 24). Diffs de 2.17c et 2.17d2 complets en annexe B ; celui de d1 (deux blocs déplacés tels quels, 30 lignes `#include` ajoutées, 11 retirées) est le résultat du script.

Casts **464** et tests de classe **647**, inchangés aux quatre arbres (le cast et l'`assert` de `kr` changent de fichier, pas de nombre).

## Implementation Chunks

Trois commits, **≈ 40, 35 et 35 min** preuves comprises, chacun sous 50 min (mesuré sur la copie : script < 1 s ; `make -j4 test` + `make e2e` + e2e perturbé 9 à 19 s par arbre ; objets 4 configurations × 4 arbres en parallèle 13 s ; `fnbody.py` quelques secondes ; couverture ≈ 5 s par arbre ; 6 jumelles × 2 côtés en parallèle 61 s ; `class-coverage-net` + `sanitize-net` ≈ 8 s) :

- **2.17c** : copier `run217c.py` hors du dépôt, le lancer, comparer les hachages ; `tests.sh` ; comptes (`counts.sh`, `usedir.py`, `deadinc.py`) ; objets (`objs2.sh` ×4 à `b` et `c`, `cmp`, `fnbody.py`, `nm`) ; couverture `b`, `c` + `lset.py` ; jumelles (`muts217cdgen.py`, `mut217cd.py b` et `c`) ; `nets.sh` ; note, message.
- **2.17d1** : `run217d.py . d1` ; mêmes contrôles sauf les jumelles (refactor pur : preuve par les objets) ; `clang -fsyntax-only` (piège 4).
- **2.17d2** : `run217d.py . d2` ; idem ; relevé des sens coupés ; note qui clôt 2.17d.

**Replis écrits**, dans l'ordre :
1. si un script s'arrête sur un `AssertionError` (texte introuvable) : lire le site ; faire la même modification à la main selon la même règle (bloc coupé et collé tel quel, include à sa place alphabétique, include retiré seulement après relevé vide) ; noter chaque site dans le message ;
2. si le départ n'est pas l'arbre de 2.17b annoncé (hachages différents) : lancer les scripts quand même, ne pas comparer les hachages d'arrivée, comparer tous les comptes des tables et rapporter l'écart dans la note ;
3. si une erreur de compilation reste (`syn.sh` non vide) : c'est un nom que le fichier ne reçoit plus par le détour d'hier ; ajouter **au fichier en erreur** l'include de l'en-tête qui déclare ce nom (`pico8_fm.h`, `pico8_probe_comp.h`, `pico8_particle.h`…), jamais remettre l'include coupé ni `probe.h` → `economy.h` ; relancer `usegraph.py` et le compteur : si une paire entre, arrêter (critère d'échec) ;
4. si le relevé d'un sens coupé n'est pas vide, si une paire entre, ou si `deadinc.py` trouve un include mort de plus qu'au parent : ne pas commiter ; arrêter et rapporter ;
5. si 2.17c déborde de 1,5 × 50 min : la couper en **c1** (`run217c.py . c1` : `pico8_dj` seul ; mesuré : `inclusion-mutuelle` **8 / 8, 20 lignes / 12 fichiers**, usage **11**, 0 erreur) puis **c2** (le reste du script ; → 7 / 7, 18 / 11, usage 10) ;
6. si le vérificateur refuse les 17 lignes `#include` de `tests/` : ne pas remettre `probe.h` → `economy.h` ; n'ajouter `pico8_fm.h` qu'aux 11 TU qui ne compilent plus sans (`fr`, `g_`, `hj`, `ie`, `jc`, `mx`, `mz`, `nl`, `nr`, `os`, `qh` ; mesuré `TU=0` : 42 erreurs dans ces 11, Runs 25), et le noter.

## Testing Decisions

- **Pas de TU neuf ; les TU ne gagnent qu'une ligne `#include` (d1).** Aucun rejeu d'`events.json` dans un TU. Aucun `*_HASH` ne bouge.
- **Bon test ici** : une déclaration déplacée se prouve par le code machine (objets identiques octet pour octet) et par le graphe d'usage ; une signature changée et un corps déplacé-réécrit (2.17c) se prouvent par le chemin touché : couverture des lignes touchées, et jumelles à la règle du parent sur chaque ligne dont la sémantique pourrait glisser (quelle extrémité, quel opérateur, quel champ écrit, quel arrondi).
- **Prior art** : PRD 2.17a–b (forme, `usedir.py`, `fnbody.py`, `objs2.sh`, `deadinc.py`, `tests.sh`, `cov.sh`, `nets.sh`) ; PRD 2.15d (`syn.sh`, `mut.py`) ; PRD 2.15c (`lset.py`, ensemble `L`) ; PRD 2.11 (`usegraph.py`, `cross.py`) ; étude 2.11 (`scc.py`, `cuts/*.py`) ; règle du parent (D.21).

### Preuves (parent de `c` = `b`, de `d1` = `c`, de `d2` = `d1`)

1. **Communes** : `make -j4 test` vert ; `make e2e` **26 conformes** ; `MALLOC_PERTURB_=165 make e2e` **26 conformes**, aux quatre arbres ; `git diff --stat b d2 -- tests tools Makefile '*.sha256'` → 17 fichiers de `tests/`, +17 lignes, toutes `#include "../src/pico8_fm.h"` ; `git diff b d2 | grep -cE '^[-+].*(_HASH|FROZEN_)'` → 0 (Runs 14, 22, 23).

2. **Coupe par l'usage, sens par sens** (`usedir.py` ; « noms » = noms que l'en-tête B est seul à déclarer et que A emploie ; « include » = lignes `#include "B.h"` de A.c/A.h ; noms partagés : 0 partout) :

| Sens coupé | `b` | `c` | `d1` | `d2` |
|---|---|---|---|---|
| `economy` > `planet` | 2 : `pico8_jo_is_family`, `pico8_jo_t` ; `economy.c` | **0 ; aucun** | 0 | 0 |
| `economy_formulas` > `slipway` | 1 : `pico8_jc_end_p` ; `economy_formulas.c` | **0 ; aucun** | 0 | 0 |
| `economy_score` > `economy` (c1) | 1 : `pico8_hy_t` ; (aucun include : détour) | 1 | **0 ; aucun** | 0 |
| `tech_gate` > `economy` (c2) | 1 : `pico8_hy_t` ; (détour) | 1 | **0 ; aucun** | 0 |
| `planet` > `probe` | 2 : `pico8_fm_hy`, `pico8_probe_t` ; `planet.c`, `planet.h` | 2 | 1 : `pico8_probe_t` ; `planet.h` | **0 ; aucun** |
| `slipway` > `probe` | 5 : `pico8_d_`, `pico8_dl`, `pico8_fm_hy`, `pico8_probe_et_hang_fo`, `pico8_probe_t` ; `slipway.h`, `.c` | 5 | 4 (sans `pico8_fm_hy`) | **0 ; aucun** |
| `wormhole` > `probe` | 4 : `pico8_bv`, `pico8_d_`, `pico8_dl_dispatch`, `pico8_probe_t` ; `wormhole.h`, `.c` | 4 | 4 | **0 ; aucun** |
| `ui` > `probe` | 1 : `pico8_g__struct_class` ; `ui.c` | 1 | 1 | **0 ; aucun** |
| `ui` > `slipway` | 1 : `pico8_jc_struct_class` ; `ui.c` | 1 | 1 | **0 ; aucun** |
| `particle` > `planet` (contrôle) | 0 ; `particle.c` (include mort) | 0 ; idem | 0 ; idem | **0 ; aucun** |

   Sens gardés, à `d2` : `planet` > `economy` 1 (`pico8_hy_nx`) ; `slipway` > `economy_formulas` 1 (`pico8_dj`) ; `economy` > `economy_score` 4 → 3 (`pico8_bf`, `pico8_cz`, `pico8_kr` ; `pico8_da`/`pico8_db` partent avec le pas) ; `economy` > `tech_gate` 2 ; `probe` > `planet` 3 ; `probe` > `slipway` 5 → 4 (`pico8_jc_struct_class` passe au noyau) ; `probe` > `wormhole` 4 ; `probe` > `ui` 2 ; `slipway` > `ui` 2 ; `planet` > `particle` 1 (`pico8_emitter_t`) (Runs 4, 13, 26).

3. **Graphe d'usage et compteur, aucune paire entrante** : `usegraph.py` 12 → 10 → 8 → 3 ; `--detail` 9 → 7 → 7 → 2 ; `comm` des listes : seules des sorties, aux trois commits. `syn.sh` (gcc) et le même jeu sous `clang -std=c99 -Wall -Wextra -Werror` : 0 erreur à `c`, `d1`, `d2` (piège 4 : aucun `typedef` répété) (Runs 13, 19, 20).

4. **Objets** (`objs2.sh` : les 74 `src/*.c` hors raylib, sans `-g`, `-DPICO8_CLASS_COVERAGE` ; `cmp` octet pour octet ; `fnbody.py` fonction par fonction) :

| Configuration | 2.17c (`b` → `c`) | 2.17d1 (`c` → `d1`) | 2.17d2 (`d1` → `d2`) |
|---|---|---|---|
| `-O0 -DNDEBUG` (celle de `make test`/`e2e`) | 70 / 74 identiques ; 4 fonctions : `pico8_dj`, `pico8_hy_lj`, `pico8_jc_hq`, `pico8_kr` (neuve) ; 836 autres identiques ; 1247 relocations de données identiques | **74 / 74** | **74 / 74** |
| `-O0` avec `assert` | 70 / 74 ; sous `NORM_ASSERT=1`, les mêmes 4 fonctions seules (les 6 autres écarts sont des `__LINE__` d'`economy.c`, `economy_formulas.c`, `economy_score.c`) | 70 / 74 (`cart`, `economy`, `probe`, `slipway` : `__LINE__`) ; `NORM_ASSERT=1` : **0 écart**, 840 fonctions | 72 / 74 (`particle`, `ui` : `__LINE__`) ; `NORM_ASSERT=1` : **0 écart** |
| `-O1 -DNDEBUG` | 70 / 74 ; les mêmes 4 fonctions ; 727 autres identiques | **74 / 74** | **74 / 74** |
| `-O2 -DNDEBUG` | 70 / 74 ; les 4, plus `pico8_jc_hq.part.0` (découpe de `jc_hq`) et `pico8_economy_set_dk` (un `nop` de bourrage) ; données identiques | **74 / 74** | **74 / 74** |
| `nm` | `economy.o` : `U pico8_da`, `U pico8_db` → `U pico8_kr` ; `economy_formulas.o` : `U pico8_jc_end_p` part ; `economy_score.o` : `T pico8_kr`, `U pico8_fixed_sub` entrent ; `slipway.o` inchangé | identiques | identiques |

   Lecture : d1 et d2 sont des refactors purs (le plan l'annonçait, 73 / 73 à l'étude, 74 / 74 ici avec `main_class_coverage`). 2.17c touche exactement les quatre fonctions de sa description ; tout le reste du code machine et des données est identique (Runs 15, 16, 22, 23).

5. **Mutations, règle du parent** (`mut217cd.py` = `mut.py` du PRD 2.15d avec `S` et le nom du JSON changés ; `muts217cd.json`, écrit par `muts217cdgen.py`, annexe A). Même effet au parent et à l'enfant, chacun sur son texte ; `make -k -j4 unit` puis `make -k e2e` ; restaurer par `git checkout -f`. **ÉCHEC si un TU rouge au parent n'est pas rouge à l'enfant, ou si l'e2e est < 26 au parent et 26 à l'enfant.**

| # | Cible (parent → enfant) | Mutation | Rouges au parent | Rouges à l'enfant | e2e | Verdict |
|---|---|---|---|---|---|---|
| DJ-qq | appel de `pico8_dj` (`slipway.c:120`) | deux fois l'extrémité `qu` (plan) | `jc` | `jc` | 20 / 20 | conforme |
| DJ-sw | idem | `qu` et `re` échangés | aucun | aucun | 26 / 26 | survivante aux deux bouts, déclarée : la distance est au carré, l'échange ne change rien (étude, §4) |
| DJ-add | corps de `pico8_dj` | `pico8_vec_sub` → `pico8_vec_add` | `hp`, `hy`, `jc`, `js`, `lq`, `mz`, `trade`, `ui` | les mêmes 8 | 17 / 17 | conforme |
| KR-has | `jo->has_kr = 1;` (`economy.c` → `economy_score.c`) | `= 0` (plan) | `hy`, `jo`, `js`, `lq` | les mêmes 4 | 19 / 19 | conforme |
| KR-flr | `pico8_fixed_flr(…)` du pas `kr` | `flr` retiré (plan) | aucun | aucun | 23 / 23 | conforme (rouge par l'e2e seul) |
| KR-kr | `jo->kr = kr;` | `jo->kr = pico8_fixed_from_int(0);` | `hy`, `js`, `lq` | les mêmes 3 | 19 / 19 | conforme |

   Lecture : **aucun ÉCHEC** (6 jumelles, 12 lancements, rouges et e2e identiques des deux côtés). Une première jumelle `DJ-to` (`pico8_vec_sub(to, to)`) ne compilait ni au parent ni à l'enfant (`-Werror`, paramètre `pb` inutilisé) : remplacée par `DJ-add` (Runs 17, 18). Lignes K : aucune (l'enfant n'a pas de chemin neuf hors `pico8_kr`, couvert).

6. **Chemin touché, ensemble `L`** (`make line-coverage EVENTS="<les 26 e2e>"`, `lset.py`) : `b` **6031**, `c` **6034**, `d1` 6034, `d2` 6034 lignes.
   - 2.17c : **0 perdue, 0 gagnée** ; `L` **8** (`economy.c:226`, `:229-232`, `economy_formulas.c:132-133`, `slipway.c:120`) ; lignes neuves couvertes **11 / 11 exécutables** (`economy.c:225`, `economy_formulas.c:131-132`, `economy_score.c:267-268`, `:271-275`, `slipway.c:120`) : `gcov` passe par `pico8_dj`, `pico8_jc_hq`, `pico8_hy_lj`, `pico8_kr`. Non couvertes : l'include, les deux prototypes, la déclaration `pico8_jo_t *jo;`, des lignes vides et l'accolade fermante après `return` (jamais comptée par `gcov`).
   - 2.17d1, 2.17d2 : **0 perdue, 0 gagnée, `L` 0**.
   (Runs 17, 22, 23.)

7. **Reproductibilité** : les quatre scripts sur un arbre de travail neuf à `0f2db15` → `3c5385b8…`, `9af24592…`, `ad162508…`, `b5a5cb59…` ; relancer `run217c.py` ou `run217d.py d2` sur l'arrivée : `AssertionError` (Runs 24).

8. `class-coverage-net` : **non** exigé ; lancé à `c`, `d1`, `d2` : `couverture: OK -- chaque classe non exemptée est instanciée (26 rejeu(x))`. `sanitize-net` : **non** exigé ; lancé : `sanitize: 26 propre(s)` aux trois (Runs 18, 22, 23).

### Greps de Sortie

À `c` :
1. `grep -n 'pico8_dj(' src/*.c src/*.h` → 3 lignes : `economy_formulas.c:131` (définition, `pico8_vec_t pb_p, pico8_vec_t to_p`), `economy_formulas.h:38`, `slipway.c:120` (`pico8_dj(pico8_jc_end_p(self->qu), pico8_jc_end_p(self->re))`).
2. `grep -n 'pico8_kr(' src/*.c src/*.h` → 3 lignes : `economy.c:225`, `economy_score.c:267`, `economy_score.h:41`.
3. `grep -c 'pico8_jo_\|jo->' src/pico8_economy.c` → **0** ; `grep -c 'pico8_jc_' src/pico8_economy_formulas.c` → **0**.

À `d1` :
4. `grep -rl '#include "\(../src/\)\?pico8_fm.h"' src tests | wc -l` → **30** (dont 17 dans `tests/`).
5. `grep -c '#include "pico8_probe.h"' src/pico8_{planet,cheat,economy_score,menu,planet_display,score_panel,score_panel_display,tech_gate}.c` → **0** ×8 ; `grep -c 'pico8_fm\|pico8_hy_t' src/pico8_probe.h` → **0**.

À `d2` :
6. `grep -l '#include "pico8_probe.h"' src/pico8_{slipway,wormhole,planet}.[ch] src/pico8_ui.c` → vide ; `grep -rl '#include "pico8_probe_comp.h"' src | wc -l` → **6** (`probe.h`, `planet.h`, `slipway.h`, `slipway.c`, `wormhole.h`, `wormhole.c`).
7. `grep -n 'pico8_\(jc\|g_\)_struct_class(void)' src/*.h` → 2 lignes, `struct.h:189-190`.
8. `grep -c '#include "pico8_planet.h"' src/pico8_particle.c` → **0**.

Aux trois :
9. `make composition-residue` → `inclusion-mutuelle 7 7`, `résidu 2 : 18 ligne(s), 11 fichier(s)` (`c`) ; `7 7`, `17 / 11` (`d1`) ; `2 3`, `5 / 4` (`d2`) ; `python3 usegraph.py .` → `paires 10`, `8`, `3`.
10. `git diff --stat <parent>` : les fichiers de la section « Modules », plus `docs/plan-3.md`, rien d'autre.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable (`prd217cd/copie`, arbres de travail `wt_b`, `wt_c`, `wt_d1`, `wt_d2`, `c_b`, `c_c`, `c_d1`, `c_d2`, `wt_r`).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout 0f2db15` ; `git rev-parse` des arbres | `x` | 0 | `148603f3…`, `cac32597…`, `422546ca…` (= 2.16) | 3 s |
| 2 | `extract.py` : 18 scripts tirés des annexes des PRD 2.17a–b, 2.15d, 2.15c, 2.11 et de l'étude ; `usedir.py`, `usegraph_e.py` montés comme au PRD 2.17a–b | — | 0 | `run217a.py`, `run217b.py`, `deadinc.py`, `objs2.sh`, `fnbody.py`, `tests.sh`, `cov.sh`, `nets.sh`, `cross.py`, `usegraph.py`, `syn.sh`, `mut.py`, `lset.py`, `scc.py`… | < 1 s |
| 3 | `run217a.py`, `run217b.py` ; hachages | `a`, `b` | 0 | `e8e76840…`, `3c5385b8…` ; `90747520…`, `548da3b7…`, `944b21ab…` (ceux du PRD 2.17a–b) | < 1 s |
| 4 | compteur, `usegraph.py`, `usedir.py` (18 sens) | `b` | 0 | `9 9`, `22 / 13`, `paires 12` ; relevés de la preuve 2 | 2 s |
| 5 | `run217c.py` (premier jet, sans le retrait de `struct.h`) ; `syn.sh` ; compteur ; `usegraph.py` ; `usedir.py` | `c` | 0 | 0 erreur ; `7 7`, `18 / 11`, `paires 10` ; sens coupés vides | 3 s |
| 6 | `run217d.py d1` premier jet (`probe.h` sans `economy.h`, TU non touchés) ; `syn.sh` | `d1` | 0 | **53 erreurs** dans 11 TU (`pico8_fm_hy`, `pico8_hy_t` reçus par `probe.h` → `economy.h`) | 3 s |
| 7 | variante `probe.h` gardant `economy.h`, TU non touchés : d1, puis d2 | `d1`, `d2` | 0 | d1 : 0 erreur ; d2 : **182 erreurs** (`planet.h` : `pico8_emitter_t`, reçu par `probe.h` → `particle.h`) | 5 s |
| 8 | + `planet.h` → `particle.h` ; `syn.sh` ; compteur ; `usegraph.py` | `d2` | 0 | 0 erreur ; compteur **3 / 4, 7 / 6** : **`particle` ↔ `planet` entre** (`particle.c:12` → `planet.h`, 0 nom) ; usage 3 | 3 s |
| 9 | + `particle.c:12` retirée ; `syn.sh` ; compteur | `d2` | 0 | 0 erreur ; **2 / 3, 5 / 4**, usage 3 ; l'include venait de D.11c (`8737740`) | 3 s |
| 10 | d1 (TU touchés) puis d2 en `COMP=comp` et `COMP=particle` ; compteur ; `usegraph.py` | `d2` ×2 | 0 | les deux : 0 erreur, `2 / 3`, `5 / 4`, `paires 3` ; arêtes 627 / 616 | 6 s |
| 11 | `usegraph_e.py` ; arêtes vers et depuis `particle`/`probe_comp` ; `scc.py` ; `cc -M` des 158 `.c` | `d2` ×2 | 0 | table du choix ; composantes `[25]` aux deux ; 71 / 72 fichiers | 20 s |
| 12 | `hosts.py` sur les arêtes de `c` (hôtes possibles de `fm`, du composant) | `c` | 0 | 38 / 79 et 47 / 79 ; raisons des hôtes qui ont un sens | < 1 s |
| 13 | `counts.sh b c d1 d2` ; `comm` des paires et des lignes | 4 | 0 | table « Comptes » ; seules des sorties | 8 s |
| 14 | `tests.sh wt_b wt_c wt_d1 wt_d2` (premiers jets) | 4 | 0 ×12 | vert ; 26 ; 26 | 14, 18, 19, 11 s |
| 15 | `objs2.sh` ×4 ×4 ; `cmp` ; `fnbody.py` ; `NORM_ASSERT=1` | 4 | 0 | 70 / 74 (c), 74 / 74 ×3 (d1, d2 sous `-DNDEBUG`) | 13 s + 10 s |
| 16 | `FNDIFF=1 fnbody.py` `-O2` ; `nm` des 4 objets de c | `b`, `c` | 0 | `nop` de `pico8_economy_set_dk` ; symboles de la preuve 4 | < 1 s |
| 17 | `cov.sh b c` ; `lset.py` ; jumelles (premier JSON) | `c_b`, `c_c`, `wt_b`, `wt_c` | 0 | 6031 / 6034, `L` 8, 11 / 11 ; **`DJ-to` ne compile pas aux deux bouts** (`pb` inutilisé) ; les 5 autres identiques | 10 s ; 61 s |
| 18 | jumelle `DJ-add` | `wt_b`, `wt_c` | 0 | 8 TU rouges des deux côtés, 17 / 17 | 20 s |
| 19 | `clang -fsyntax-only` sur `src/` et `tests/` | `c`, `d1`, `d2` | 0 | 0 erreur | non chronométré |
| 20 | `deadinc.py` parent / enfant sur les fichiers touchés (premiers jets) | 3 paires | 0 | **includes morts créés** : 1 (c : `economy_formulas.h` → `struct.h`), 9 (d1), 2 (d2 : `probe.h` → `gfx.h`, `mem.h`) → scripts complétés (G2) | < 1 s |
| 21 | scripts définitifs : `run217c.py`, `run217d.py d1`, `d2` ; hachages ; `counts.sh` | `c`, `d1`, `d2` | 0 | table « Ce qui change » ; comptes inchangés | 10 s |
| 22 | `deadinc.py` ; `tests.sh` ; `objs2.sh` ; `fnbody.py` ; `cov.sh c d1` ; `lset.py` ; jumelles (6 × 2) ; `nets.sh c d1` | `c`, `d1` | 0 | 1 → 1, 17 → 17 ; vert, 26, 26 ; preuve 4 ; `L` 8 puis 0 ; aucun ÉCHEC ; `OK`, `26 propre(s)` | 9 s, 9 s ; 61 s ; 8 s |
| 23 | commentaire de `struct.h` reformulé (piège, règle des commentaires) ; d2 refait ; `tests.sh` ; objets ; `counts.sh` ; `nets.sh` ; `cov.sh d2` ; `usedir.py` | `d2` | 0 | `b5a5cb59…` ; vert ; 74 / 74 ×3, `NORM_ASSERT` 0 ; `2 3`, `5 / 4`, `paires 3` ; `OK` ; `L` 0 ; sens coupés vides | 10 s, 13 s, 8 s |
| 24 | les quatre scripts sur `wt_r` neuf à `0f2db15` ; hachages ; `run217d.py d2` et `run217c.py` relancés | `wt_r` | 0 ; 1 ; 1 | `3c5385b8…`, `9af24592…`, `ad162508…`, `b5a5cb59…`, cumul `777e6a21…` ; `AssertionError` ×2 | < 1 s |
| 25 | replis : `run217c.py . c1` ; `COMP=particle` ; `TU=0` en d1 ; compteur, `usegraph.py` | `wt_r` | 0 | c1 : `8 8`, `20 / 12`, `paires 11` ; particle : `2 3`, `5 / 4`, `paires 3` ; `TU=0` : 42 erreurs dans 11 TU | 8 s |
| 26 | `usedir.py` des 10 sens coupés aux quatre arbres, puis des 10 sens gardés à `b` et `d2` | 4 | 0 | preuve 2 | 3 s |
| 27 | détour de 2.17f : appel `pico8_jo_is_family(p)` ajouté dans `economy.c` à `d2`, `syn.sh`, compteur, `usegraph.py` ; restauré | `wt_r` | 0 | 0 erreur ; `inclusion-mutuelle 2 3` ; `paires 4` (`economy` ↔ `planet` entre) | 3 s |
| 28 | `rm -rf` de la copie, des arbres de travail et des fichiers d'essai | — | 0 | supprimés | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas le commit de 2.17b (arbres `3c5385b8…`, `cac32597…`, `422546ca…`, ou un descendant qui ne touche que `docs/`) sans que l'écart soit rapporté, ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si 2.17c, 2.17d1, 2.17d2 ne sont pas trois commits, chacun vert (`make test`, `make e2e` 26 conformes, et 26 sous `MALLOC_PERTURB_=165`) ; si un `*.sha256`, `tools/`, le `Makefile`, une constante `*_HASH`/`FROZEN_*` change ; si `tests/` change hors les 17 lignes `#include "../src/pico8_fm.h"` de d1 (ou les 11 du repli 6).
- `FAILED:` si, au départ annoncé, l'arbre `src` commité n'est pas `9af24592…`, `ad162508…`, `b5a5cb59…` (et `tests` `ea2eb691…` à d1 et d2), sans repli rapporté.
- `FAILED:` si le relevé `usedir.py` d'un sens coupé n'est pas vide à son commit (nom propre, nom partagé ou include) ; si une ligne `#include` est retirée alors qu'un nom de l'en-tête reste employé (détour) ; si un nom est redéclaré localement ; si `probe.h` inclut encore `economy.h` à d1.
- `FAILED:` si une paire entre au compteur (`--detail`) ou au graphe d'usage à l'un des trois commits ; si les comptes ne sont pas ceux de la table « Comptes » (7 / 7, 18 / 11, usage 10 ; 7 / 7, 17 / 11, usage 8 ; 2 / 3, 5 / 4, usage 3) ; si une catégorie gardée n'est plus à 0 ; si `deadinc.py` trouve plus d'includes morts à l'enfant qu'au parent.
- `FAILED:` si, à d1 ou d2, un objet diffère octet pour octet sous `-O0`, `-O1` ou `-O2 -DNDEBUG`, ou si un écart reste sous `NORM_ASSERT=1` à `-O0` avec `assert` ; si, à 2.17c, une autre fonction que `pico8_dj`, `pico8_hy_lj`, `pico8_jc_hq`, `pico8_kr` diffère sous `-O0`/`-O1 -DNDEBUG`, ou une relocation de données.
- `FAILED:` si une jumelle a un TU rouge au parent qui ne l'est pas à l'enfant, ou un e2e < 26 au parent et 26 à l'enfant.
- `FAILED:` si une ligne couverte par les 26 e2e au parent ne l'est plus à l'enfant, ou si une ligne exécutable neuve de 2.17c n'est pas couverte.
- `FAILED:` si le composant est rangé ailleurs que dans `pico8_probe_comp.h`, `fm` ailleurs que dans `pico8_fm.h`, les identités ailleurs que dans `struct.h` ; si `struct.h` inclut un en-tête de plus ; si une table de classe, un corps hors les quatre fonctions de 2.17c, ou un commentaire déplacé change (hors le mot du commentaire de `pico8_hy_def_t`).
- `FAILED:` si `class-coverage-net` ou `sanitize-net` est rouge.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si un message de commit omet : les fonctions et déclarations déplacées et leurs fichiers, les includes retirés et ajoutés (dont ceux de G2 et `particle.c:12`), le relevé d'usage de chaque sens coupé, les comptes (paires / modules, lignes / fichiers, usage), la preuve des objets par configuration, pour 2.17c la table des jumelles avec leur résultat au parent (dont la survivante DJ-sw) et `L`, pour 2.17d2 le choix `pico8_probe_comp.h` contre `particle.h` avec sa mesure, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.17c » (resp. « 2.17d1 », « 2.17d2 ») au bout de sa ligne du plan, ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- 2.17e (`pico8_na`, `pico8_gd`, registre `kv` ; les paires `entity_geom` ↔ `planet`, `planet` ↔ `planet_family`, cachée `entity_geom` ↔ `scroll`) et 2.17f (garde `usage-mutuel`).
- Les includes préexistants à 0 nom (hors ceux que 2.17c/d rendraient morts, et `particle.c:12`, qui fermerait sinon une paire) ; le cycle de 25 modules du graphe d'usage.
- `tools/line_coverage_reference.txt` (hors suite, déjà décalé) ; les numéros de ligne cités par des documents hors `docs/plan-3.md`.
- Renommer `pico8_kr`, `pico8_fm.h`, `pico8_probe_comp.h` (phase 3) ; la taille du code C, l'appli raylib (aucun de ses fichiers n'emploie un nom déplacé : grep), l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Trois commits, le repli d1/d2 du plan pris d'emblée.** d1 touche 32 fichiers, d2 porte le choix du rangement ; chacun a sa preuve (objets identiques) et tient en ≈ 35 min, la paire en ≈ 70 min contre 50 au plan. 2.17c reste d'un seul commit (≈ 40 min), repli c1/c2 mesuré.
- **G2. Les includes que la coupe rend morts partent, dans le commit qui les rend morts** (règle de 2.17a–b) : `economy_formulas.h` → `struct.h` (c) ; `probe.h` → `economy.h`, `economy.h` → `economy_formulas.h`, `ptrs.h`, et `probe.h` de sept `.c` (d1) ; `probe.h` → `gfx.h`, `mem.h` (d2). Les garder laisserait des chemins de détour tout prêts. Prix : 17 TU et 12 `.c` doivent inclure `pico8_fm.h` eux-mêmes (G4).
- **G3. `particle.c:12` → `planet.h` part à d2.** Préexistant et mort (0 nom), il ferait sinon entrer `particle` ↔ `planet` au compteur dès que `planet.h` inclut `particle.h`, ce qu'impose la coupe de `planet.h:11` (l'autre voie, `pico8_probe_comp.h` incluant `particle.h`, rendrait l'émetteur visible à tout porteur de sonde et casserait la feuille).
- **G4. Les 17 TU incluent `pico8_fm.h`** (règle unique : tout fichier qui emploie un nom de `fm.h` l'inclut), bien que 6 d'entre eux le reçoivent aussi par `economy.h` ; repli 6 si le vérificateur préfère le minimum (11).
- **G5. Pas d'étiquette `struct pico8_hy`.** L'étude l'ajoutait pour une déclaration anticipée (piège 3) qu'aucune coupe n'emploie ; l'ajouter serait un changement sans usage.
- **G6. Identités au noyau.** `struct.h` déclare déjà des noms d'une famille (`pico8_jo_keys_t`, la déclaration anticipée `struct pico8_probe`, le lien `probe`) : deux prototypes de plus, sans include neuf, `noyau-inclut` 0. Les étiquettes ne sont pas équivalentes pour `g_` (portée aussi par `fv`, étude fiche 12). Le commentaire ajouté porte le piège (les remettre dans leur module refait deux paires), pas le récit.
- **G7. Le commentaire de module de `probe.h`** (« Throughout this module a NULL instance argument is a safe no-op… for `bv`, `dl`, `d_`… ») reste dans `probe.h` : il parle du module `probe`, qui définit toujours ces fonctions ; le recopier dans `pico8_probe_comp.h` ajouterait un commentaire, que la règle des quatre destinations ne demande pas.
- **G8. `class-coverage-net` et `sanitize-net` lancés bien que non exigés** (≈ 8 s par arbre).

### Surprises de mesure, pour `plan-pilot`

1. **Les scripts de l'étude ne s'appliquent plus** (`kr_score.py` cherche `pico8_struct_header_t *p = e;`, `t2_fm.py` et `probe_comp.py` ignorent les `def` de 2.12/2.13) : scripts neufs, mêmes coupes.
2. **`planet.h` recevait `pico8_emitter_t` par `probe.h`** : couper `planet.h:11` exige `planet.h` → `particle.h`, qui ferait entrer `particle` ↔ `planet` sans le retrait de l'include mort `particle.c:12`.
3. **Les 17 includes des TU** ne sont facultatifs que si `probe.h` garde un include mort d'`economy.h` ; sans lui, 11 sont obligatoires.
4. **`particle.h` et `pico8_probe_comp.h` donnent les mêmes comptes** ; seule la forme du graphe (feuille ou non) les départage.
5. **Le texte du plan pour 2.17d** (« usage → 3, dont les deux paires cachées d'`economy` ») est inexact : elles sortent à d1.
6. **`fm` entre dans le cycle** (composante 24 → 25), par `pico8_qe_map_t` : hors périmètre.

### Questions pour l'humain

Aucune.

### Annexe A — scripts (texte exact)

Tous hors dépôt, depuis un dossier d'essais `S` (à adapter) qui contient la copie et ses arbres de travail ; journaux dans `S/logs`. Repris tels quels : `usegraph.py`, `cross.py` (annexe A du PRD 2.11) ; `usedir.py`, `usegraph_e.py`, `deadinc.py`, `objs2.sh`, `fnbody.py`, `tests.sh`, `cov.sh`, `nets.sh` (annexe A du PRD 2.17a–b ; dans les `.sh`, `prd217ab` devient `prd217cd`) ; `syn.sh` (annexe A du PRD 2.15d) ; `lset.py` (annexe A du PRD 2.15c) ; `scc.py` (annexe A de l'étude 2.11). `mut217cd.py` = `mut.py` du PRD 2.15d où `prd215d` devient `prd217cd` et `muts.json` devient `muts217cd.json` ; il se lance `python3 mut217cd.py b <noms>` et `python3 mut217cd.py c <noms>` (arbres `wt_b`, `wt_c`). Sens de la preuve 2 : `DIRS="economy>planet economy_formulas>slipway economy_score>economy tech_gate>economy planet>probe slipway>probe wormhole>probe ui>probe ui>slipway particle>planet" python3 usedir.py <arbre>`.

`run217c.py` :

```python
# run217c.py ARBRE : 2.17c sur un arbre propre au commit 2.17b. Chaque texte
# cherche doit etre present exactement une fois, sinon arret (AssertionError).
import os, sys
T = sys.argv[1]
def p(f): return os.path.join(T, 'src', f)
def rd(f): return open(p(f)).read()
def wr(f, s): open(p(f), 'w').write(s)
def sub(f, old, new):
    s = rd(f); assert s.count(old) == 1, (f, old, s.count(old)); wr(f, s.replace(old, new))
def append(f, text): wr(f, rd(f).rstrip('\n') + '\n\n' + text + '\n')
def before_endif(f, text): sub(f, '\n#endif\n', '\n' + text + '\n#endif\n')

# c1 : economy_formulas -> slipway (fiche 2, T3)
sub('pico8_economy_formulas.h',
    'pico8_fixed_t pico8_dj(const pico8_struct_header_t *pb, const pico8_struct_header_t *to);',
    'pico8_fixed_t pico8_dj(pico8_vec_t pb_p, pico8_vec_t to_p);')
sub('pico8_economy_formulas.h', '#include "pico8_map_tables.h"\n', '#include "pico8_map_tables.h"\n#include "pico8_vec.h"\n')
sub('pico8_economy_formulas.c',
    'pico8_fixed_t pico8_dj(const pico8_struct_header_t *pb, const pico8_struct_header_t *to) {\n'
    '    pico8_vec_t diff = pico8_vec_sub(pico8_jc_end_p(to), pico8_jc_end_p(pb));',
    'pico8_fixed_t pico8_dj(pico8_vec_t pb_p, pico8_vec_t to_p) {\n'
    '    pico8_vec_t diff = pico8_vec_sub(to_p, pb_p);')
sub('pico8_economy_formulas.c', '#include "pico8_slipway.h"\n', '')
# economy_formulas.h n'emploie plus rien de struct.h (G2)
sub('pico8_economy_formulas.h', '#include "pico8_struct.h"\n', '')
sub('pico8_slipway.c', 'pico8_dj(self->qu, self->re)', 'pico8_dj(pico8_jc_end_p(self->qu), pico8_jc_end_p(self->re))')
print('2.17c1 : pico8_dj sur deux points, 2 includes retires, 1 ajoute')
if len(sys.argv) > 2 and sys.argv[2] == 'c1': sys.exit(0)

# c2 : economy -> planet (fiche 1, T6)
sub('pico8_economy.c', '''                pico8_fixed_t kr = pico8_fixed_flr(pico8_fixed_sub(pico8_db(p), pico8_da(p)));
                pico8_jo_t *jo;

                assert(pico8_jo_is_family(p) && "hy:lj: cg.jo holds planets only");
                jo = (pico8_jo_t *)p;
                jo->kr = kr;
                jo->has_kr = 1;
                dd = pico8_fixed_add(dd, kr);''', '''                pico8_fixed_t kr = pico8_kr(p);

                dd = pico8_fixed_add(dd, kr);''')
append('pico8_economy_score.c', '''pico8_fixed_t pico8_kr(pico8_struct_header_t *p) {
    pico8_fixed_t kr = pico8_fixed_flr(pico8_fixed_sub(pico8_db(p), pico8_da(p)));
    pico8_jo_t *jo;

    assert(pico8_jo_is_family(p) && "hy:lj: cg.jo holds planets only");
    jo = (pico8_jo_t *)p;
    jo->kr = kr;
    jo->has_kr = 1;
    return kr;
}''')
before_endif('pico8_economy_score.h', '/* Writes `kr` into planet `p` (asserted) and returns it. */\npico8_fixed_t pico8_kr(pico8_struct_header_t *p);\n')
sub('pico8_economy.c', '#include "pico8_planet.h"\n', '')
print('2.17c2 : pas kr de hy:lj -> pico8_kr (economy_score), 1 include retire')
```

`run217d.py` (défauts : `TU=1`, `COMP=comp` ; les options ne servent qu'aux replis et à la mesure du choix) :

```python
# run217d.py ARBRE d1|d2 [OPTIONS] : 2.17d1 sur un arbre propre au commit 2.17c,
# 2.17d2 sur un arbre propre au commit 2.17d1. Chaque texte cherche doit etre
# present exactement une fois, sinon arret (AssertionError).
# Options d'essai (le PRD retient les defauts) : TU=1 ajoute pico8_fm.h aux TU
# qui en emploient un nom ; COMP=particle range le composant dans particle.h.
import os, sys, re, glob
T, STEP = sys.argv[1], sys.argv[2]
TU = os.environ.get('TU', '1') == '1'; COMP = os.environ.get('COMP', 'comp')
def p(f): return os.path.join(T, f if '/' in f else 'src/' + f)
def rd(f): return open(p(f)).read()
def wr(f, s): open(p(f), 'w').write(s)
def sub(f, old, new):
    s = rd(f); assert s.count(old) == 1, (f, old, s.count(old)); wr(f, s.replace(old, new))
def cut_span(f, start, end):
    s = rd(f); assert s.count(start) == 1 and s.count(end) == 1, (f, start, end)
    i = s.index(start); j = s.index(end, i) + len(end)
    while s[j:j+1] == '\n': j += 1
    wr(f, s[:i] + s[j:]); return s[i:j].rstrip('\n') + '\n'
def before_endif(f, text): sub(f, '\n#endif\n', '\n' + text + '\n#endif\n')
def add_include(f, h, pre=''):
    # a sa place alphabetique parmi les #include "pico8_*.h" (apres l'en-tete propre d'un .c)
    s = rd(f); inc = '#include "%s%s"\n' % (pre, h); assert inc not in s, (f, h)
    m = list(re.finditer(r'#include "%spico8_[^"]*"\n' % re.escape(pre), s)); assert m, f
    own = m[0].end() if f.endswith('.c') and not pre else 0
    ins = next((x for x in m if x.group(0) > inc and x.start() >= own), None)
    pos = ins.start() if ins else m[-1].end()
    wr(f, s[:pos] + inc + s[pos:])

if STEP == 'd1':
    # pico8_hy_def_t et pico8_hy_t quittent economy.h, le creneau fm quitte probe.h
    hy = cut_span('pico8_economy.h', "/* hy's archetype, here and not with the others:", '} pico8_hy_t;')
    hy = hy.replace('economy.h does not include', 'pico8_fm.h does not include', 1)
    fm = cut_span('pico8_probe.h', '/* The provisional `fm`:', 'const pico8_fm_placeholder_t *pico8_fm_placeholder(void);')
    wr('src/pico8_fm.h', '#ifndef PICO8_FM_H\n#define PICO8_FM_H\n\n#include "pico8_economy_formulas.h"\n#include "pico8_fixed.h"\n'
       '#include "pico8_ptrs.h"\n#include "pico8_struct.h"\n\n' + hy + '\n' + fm + '\n#endif\n')
    add_include('pico8_economy.h', 'pico8_fm.h')
    # chaque .c de src/ qui emploie un nom de pico8_fm.h l'inclut directement
    names = re.compile(r'\b(pico8_hy_def_t|pico8_hy_t|pico8_fm|pico8_fm_hy|pico8_fm_placeholder|pico8_fm_placeholder_t|'
                       r'pico8_fm_placeholder_struct_class|pico8_probe_set_fm)\b')
    fs = sorted(glob.glob(p('src/*.c')) + glob.glob(p('src/*.h')))
    if TU: fs += sorted(glob.glob(p('tests/*.c')))
    n = 0
    for f in fs:
        b = os.path.basename(f)
        if b in ('pico8_fm.h', 'pico8_economy.h'): continue
        body = re.sub(r'/\*.*?\*/', '', open(f).read(), flags=re.S)
        if names.search(body):
            add_include(os.path.relpath(f, T), 'pico8_fm.h', '../src/' if '/tests/' in f else ''); n += 1
    # planet -> probe : planet.c n'employait de probe.h que pico8_fm_hy
    sub('pico8_planet.c', '#include "pico8_probe.h"\n', '')
    # includes que le deplacement rend morts (G2) : ces .c n'employaient de probe.h
    # que des noms du creneau fm ; economy.h n'emploie plus economy_formulas.h ni ptrs.h
    for f in ('pico8_cheat.c', 'pico8_economy_score.c', 'pico8_menu.c', 'pico8_planet_display.c',
              'pico8_score_panel.c', 'pico8_score_panel_display.c', 'pico8_tech_gate.c'):
        sub(f, '#include "pico8_probe.h"\n', '')
    sub('pico8_economy.h', '#include "pico8_economy_formulas.h"\n', '')
    sub('pico8_economy.h', '#include "pico8_ptrs.h"\n', '')
    # probe.h n'emploie plus rien d'economy.h
    sub('pico8_probe.h', '#include "pico8_economy.h"\n', '')
    print('2.17d1 : pico8_fm.h neuf, %d fichiers l\'incluent, 11 includes retires' % (n + 1))

elif STEP == 'd2':
    t = cut_span('pico8_probe.h', '/* What `bv`, `dl` and `d_` read and write', '} pico8_probe_t;')
    fns = cut_span('pico8_probe.h', '/* TROU-7 */\n/* `mp` is stored into `ra`', 'void pico8_d__dispatch(void *self, pico8_struct_header_t *kv);')
    if COMP == 'comp':
        H = 'pico8_probe_comp.h'
        wr('src/' + H, '#ifndef PICO8_PROBE_COMP_H\n#define PICO8_PROBE_COMP_H\n\n#include "pico8_fixed.h"\n#include "pico8_game_data.h"\n'
           '#include "pico8_gfx.h"\n#include "pico8_mem.h"\n#include "pico8_struct.h"\n#include "pico8_vec.h"\n\n' + t + '\n' + fns + '\n#endif\n')
        add_include('pico8_probe.h', H)
    else:
        H = 'pico8_particle.h'
        before_endif('pico8_particle.h', t + '\n' + fns)
    for f in ('pico8_slipway.h', 'pico8_wormhole.h', 'pico8_planet.h', 'pico8_slipway.c', 'pico8_wormhole.c'):
        sub(f, '#include "pico8_probe.h"\n', '')
        if '#include "%s"\n' % H not in rd(f): add_include(f, H)
    # probe.h n'emploie plus rien de gfx.h ni de mem.h (G2)
    sub('pico8_probe.h', '#include "pico8_gfx.h"\n', '')
    sub('pico8_probe.h', '#include "pico8_mem.h"\n', '')
    # planet.h recevait pico8_emitter_t (membres de pico8_jo_t) par probe.h -> particle.h
    if '#include "pico8_particle.h"\n' not in rd('pico8_planet.h'): add_include('pico8_planet.h', 'pico8_particle.h')
    # particle.c:12 -> planet.h, include mort (0 nom), ferait avec la ligne ci-dessus la paire particle <-> planet
    sub('pico8_particle.c', '#include "pico8_planet.h"\n', '')
    # identites de classe au noyau (T4)
    sub('pico8_probe.h', 'const pico8_struct_class_t *pico8_g__struct_class(void);\n', '')
    sub('pico8_slipway.h', 'const pico8_struct_class_t *pico8_jc_struct_class(void);\n', '')
    before_endif('pico8_struct.h', '/* Here, not in pico8_slipway.h / pico8_probe.h: ui.c compares against\n'
                 ' * them, and including either header from ui.c makes an include pair. */\n'
                 'const pico8_struct_class_t *pico8_jc_struct_class(void);\n'
                 'const pico8_struct_class_t *pico8_g__struct_class(void);\n')
    sub('pico8_ui.c', '#include "pico8_probe.h"\n', '')
    sub('pico8_ui.c', '#include "pico8_slipway.h"\n', '')
    print('2.17d2 : composant sonde dans %s, 5 includes remplaces, 2 identites au noyau, 5 includes retires, 1 ajoute' % H)
```

`muts217cdgen.py` :

```python
# muts217cdgen.py : ecrit muts217cd.json (format de mut.py : [fichier, ancre, ancien, neuf]
# par cote) ; cotes b = 2.17b, c = 2.17c ; meme effet des deux cotes.
import json
E, F, SC, SW = 'src/pico8_economy.c', 'src/pico8_economy_formulas.c', 'src/pico8_economy_score.c', 'src/pico8_slipway.c'
DJ = 'pico8_fixed_t pico8_dj('; KR = 'pico8_fixed_t pico8_kr('; LJ = 'void pico8_hy_lj('
J = {
 'DJ-qq': {'b': [[SW, None, 'pico8_dj(self->qu, self->re)', 'pico8_dj(self->qu, self->qu)']],
           'c': [[SW, None, 'pico8_dj(pico8_jc_end_p(self->qu), pico8_jc_end_p(self->re))', 'pico8_dj(pico8_jc_end_p(self->qu), pico8_jc_end_p(self->qu))']]},
 'DJ-sw': {'b': [[SW, None, 'pico8_dj(self->qu, self->re)', 'pico8_dj(self->re, self->qu)']],
           'c': [[SW, None, 'pico8_dj(pico8_jc_end_p(self->qu), pico8_jc_end_p(self->re))', 'pico8_dj(pico8_jc_end_p(self->re), pico8_jc_end_p(self->qu))']]},
 'DJ-add': {'b': [[F, DJ, 'pico8_vec_sub(pico8_jc_end_p(to), pico8_jc_end_p(pb))', 'pico8_vec_add(pico8_jc_end_p(to), pico8_jc_end_p(pb))']],
           'c': [[F, DJ, 'pico8_vec_sub(to_p, pb_p)', 'pico8_vec_add(to_p, pb_p)']]},
 'KR-has': {'b': [[E, LJ, 'jo->has_kr = 1;', 'jo->has_kr = 0;']], 'c': [[SC, KR, 'jo->has_kr = 1;', 'jo->has_kr = 0;']]},
 'KR-flr': {'b': [[E, LJ, 'pico8_fixed_flr(pico8_fixed_sub(pico8_db(p), pico8_da(p)))', 'pico8_fixed_sub(pico8_db(p), pico8_da(p))']],
            'c': [[SC, KR, 'pico8_fixed_flr(pico8_fixed_sub(pico8_db(p), pico8_da(p)))', 'pico8_fixed_sub(pico8_db(p), pico8_da(p))']]},
 'KR-kr':  {'b': [[E, LJ, 'jo->kr = kr;', 'jo->kr = pico8_fixed_from_int(0);']], 'c': [[SC, KR, 'jo->kr = kr;', 'jo->kr = pico8_fixed_from_int(0);']]},
}
json.dump(J, open('muts217cd.json', 'w'), indent=1)
```

`hosts.py` (`python3 hosts.py <arêtes E> <employeurs> <dépendances neuves>` ; arêtes de `usegraph_e.py`, une par ligne `E<TAB>A<TAB>B<TAB>noms`) :

```python
# hosts.py EDGES USERS DEPS : modules M (en-tete existant) qui pourraient declarer
# les noms deplaces sans creer de paire d'usage : aucun M -> U (U employeur) et
# aucun D -> M (D dependance neuve de M).
import sys,collections
E=collections.defaultdict(set); mods=set()
for l in open(sys.argv[1]):
    _,a,b,_=l.rstrip('\n').split('\t'); E[a].add(b); mods|={a,b}
U=set('pico8_'+x for x in sys.argv[2].split(',')); D=set('pico8_'+x for x in sys.argv[3].split(','))
ok=[]
for M in sorted(mods):
    bad=[u for u in U if u!=M and u in E[M]]+[d+'>M' for d in D if d!=M and M in E[d]]
    if not bad: ok.append(M)
print(len(ok),'sur',len(mods),':',' '.join(x[6:] for x in ok))
```

Appels de la mesure du choix : `python3 hosts.py logs/edges_c.txt cart,cheat,economy,economy_score,menu,planet,planet_display,probe,score_panel,score_panel_display,slipway,tech_gate economy_formulas,fixed,ptrs,struct` (hôtes de `fm`) ; `python3 hosts.py logs/edges_c.txt help,planet,planet_family,planet_family_trade,probe,research_icon,slipway,wormhole fixed,game_data,gfx,mem,struct,vec` (hôtes du composant).

`counts.sh` (`bash counts.sh <rév>...`, sur la copie) :

```sh
#!/bin/bash
# counts.sh REV... : comptes de la table, sur la copie (checkout detache)
S=/tmp/claude-0/-home-user-slipways-love/c83e28ad-64b1-5e5c-ac9f-9444621e8cef/scratchpad/prd217cd
cd $S/copie
for r in "$@"; do
  git checkout -q $r
  im=$(sh tools/composition_residue.sh | grep '^inclusion-mutuelle' | cut -f2,3 | tr '\t' /)
  other=$(sh tools/composition_residue.sh | grep -v '^inclusion-mutuelle' | grep -v résidu | awk -F'\t' '$2!=0' | wc -l)
  pied=$(sh tools/composition_residue.sh | tail -1 | sed 's/résidu 2 : //')
  chk=$(sh tools/composition_residue.sh --check inclusion-mutuelle src tests | grep -c '^inclusion-mutuelle:')
  use=$(python3 $S/usegraph.py . 2>&1 >/dev/null)
  edges=$(python3 $S/usegraph_e.py . 2>&1 >/dev/null | grep '^E' | tee $S/logs/edges_$r.txt | wc -l)
  scc=$(python3 $S/scc.py $S/logs/edges_$r.txt | sed 's/.*composantes >1 : //')
  champs=$(sh tools/composition_residue.sh --champs | tail -1)
  casts=$(grep -rEo '\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)' src tests | wc -l)
  cls=$(grep -rEo '(->cls|header\.cls|_struct_class\(\))' src tests | wc -l)
  cross=$(python3 $S/cross.py . | wc -l)
  echo "$r | im $im | autres!=0 $other | pied $pied | check $chk | $use | aretes $edges scc $scc | $champs | casts $casts cls $cls | cross $cross"
done
git checkout -q work
```

### Annexe B — diffs (texte exact)

`git diff b c` (2.17c, 118 lignes, `5821adac…`) :

```diff
diff --git a/src/pico8_economy.c b/src/pico8_economy.c
index a2dafca..31343c3 100644
--- a/src/pico8_economy.c
+++ b/src/pico8_economy.c
@@ -12,7 +12,6 @@
 #include "pico8_game_data.h"
 #include "pico8_map_tables.h"
 #include "pico8_msgbox.h"
-#include "pico8_planet.h"
 #include "pico8_tech_gate.h"
 
 static int g_hy_up = 0;
@@ -223,13 +222,8 @@ void pico8_hy_lj(pico8_hy_t *self) {
         it = pico8_ents_all(jo_list);
         while (pico8_ents_iter_next(&it, &p)) {
             if (p != NULL) {
-                pico8_fixed_t kr = pico8_fixed_flr(pico8_fixed_sub(pico8_db(p), pico8_da(p)));
-                pico8_jo_t *jo;
+                pico8_fixed_t kr = pico8_kr(p);
 
-                assert(pico8_jo_is_family(p) && "hy:lj: cg.jo holds planets only");
-                jo = (pico8_jo_t *)p;
-                jo->kr = kr;
-                jo->has_kr = 1;
                 dd = pico8_fixed_add(dd, kr);
             }
         }
diff --git a/src/pico8_economy_formulas.c b/src/pico8_economy_formulas.c
index be62868..87db632 100644
--- a/src/pico8_economy_formulas.c
+++ b/src/pico8_economy_formulas.c
@@ -7,7 +7,6 @@
 #include "pico8_map_tables.h"
 #include "pico8_planet.h"
 #include "pico8_research.h"
-#include "pico8_slipway.h"
 #include "pico8_vec.h"
 
 static pico8_fixed_t fixed_max(pico8_fixed_t a, pico8_fixed_t b) {
@@ -129,8 +128,8 @@ void pico8_ck(pico8_qe_map_t *out) {
     for_each_cg("qh", ck_visit, out);
 }
 
-pico8_fixed_t pico8_dj(const pico8_struct_header_t *pb, const pico8_struct_header_t *to) {
-    pico8_vec_t diff = pico8_vec_sub(pico8_jc_end_p(to), pico8_jc_end_p(pb));
+pico8_fixed_t pico8_dj(pico8_vec_t pb_p, pico8_vec_t to_p) {
+    pico8_vec_t diff = pico8_vec_sub(to_p, pb_p);
     pico8_fixed_t len_sq = pico8_vec_len_sq(diff);
     pico8_fixed_t x = pico8_fixed_div(len_sq, pico8_bq());
     pico8_fixed_t ec = pico8_fixed_pow075(x);
diff --git a/src/pico8_economy_formulas.h b/src/pico8_economy_formulas.h
index 8c21af1..f776cfd 100644
--- a/src/pico8_economy_formulas.h
+++ b/src/pico8_economy_formulas.h
@@ -1,9 +1,9 @@
 #ifndef PICO8_ECONOMY_FORMULAS_H
 #define PICO8_ECONOMY_FORMULAS_H
 
-#include "pico8_struct.h"
 #include "pico8_fixed.h"
 #include "pico8_map_tables.h"
+#include "pico8_vec.h"
 
 /* Throughout this module an absent or wrong-kind field reads as its
  * zero/false default rather than crashing, and a NULL instance is a safe
@@ -35,7 +35,7 @@ void pico8_bx(pico8_qe_map_t *out);
 void pico8_ck(pico8_qe_map_t *out);
 
 /* `#(to.p-pb.p)` is the SQUARED distance, not the Euclidean one. */
-pico8_fixed_t pico8_dj(const pico8_struct_header_t *pb, const pico8_struct_header_t *to);
+pico8_fixed_t pico8_dj(pico8_vec_t pb_p, pico8_vec_t to_p);
 
 /* An absent `cg.jc` counts as zero slipways. */
 pico8_fixed_t pico8_bk(void);
diff --git a/src/pico8_economy_score.c b/src/pico8_economy_score.c
index fab8376..a2d8165 100644
--- a/src/pico8_economy_score.c
+++ b/src/pico8_economy_score.c
@@ -263,3 +263,14 @@ const pico8_js_t *pico8_dk(void) {
 void pico8_economy_set_dk(const pico8_js_t *dk) {
     g_dk = dk;
 }
+
+pico8_fixed_t pico8_kr(pico8_struct_header_t *p) {
+    pico8_fixed_t kr = pico8_fixed_flr(pico8_fixed_sub(pico8_db(p), pico8_da(p)));
+    pico8_jo_t *jo;
+
+    assert(pico8_jo_is_family(p) && "hy:lj: cg.jo holds planets only");
+    jo = (pico8_jo_t *)p;
+    jo->kr = kr;
+    jo->has_kr = 1;
+    return kr;
+}
diff --git a/src/pico8_economy_score.h b/src/pico8_economy_score.h
index 8ac1a7f..f1d6ae8 100644
--- a/src/pico8_economy_score.h
+++ b/src/pico8_economy_score.h
@@ -37,4 +37,7 @@ pico8_fixed_t pico8_bf(void);
 const pico8_js_t *pico8_dk(void);
 void pico8_economy_set_dk(const pico8_js_t *dk);
 
+/* Writes `kr` into planet `p` (asserted) and returns it. */
+pico8_fixed_t pico8_kr(pico8_struct_header_t *p);
+
 #endif
diff --git a/src/pico8_slipway.c b/src/pico8_slipway.c
index 8c279f1..83dce63 100644
--- a/src/pico8_slipway.c
+++ b/src/pico8_slipway.c
@@ -117,7 +117,7 @@ void pico8_jc_hq(pico8_jc_t *self, pico8_vec_t *mp) {
         self->re = (pico8_struct_header_t *)point;
     }
 
-    self->probe->od = pico8_dj(self->qu, self->re);
+    self->probe->od = pico8_dj(pico8_jc_end_p(self->qu), pico8_jc_end_p(self->re));
     self->probe->has_od = 1;
 
     /* A fixed is always true in Lua: `qu.mt or re.mt` takes the first present. */
```

`git diff d1 d2` (2.17d2, 276 lignes, `0954e753…`) :

```diff
diff --git a/src/pico8_particle.c b/src/pico8_particle.c
index 2f9c995..1d8ffc4 100644
--- a/src/pico8_particle.c
+++ b/src/pico8_particle.c
@@ -9,7 +9,6 @@
 #include "pico8_entity_geom.h"
 #include "pico8_fixed.h"
 #include "pico8_game_data.h"
-#include "pico8_planet.h"
 #include "pico8_vec.h"
 
 /* `fa`, and `ra` into `*ra` (the zero vector when nil), of the host of `ei`. */
diff --git a/src/pico8_planet.h b/src/pico8_planet.h
index 3aa5d07..cde5f77 100644
--- a/src/pico8_planet.h
+++ b/src/pico8_planet.h
@@ -8,7 +8,8 @@
 #include "pico8_geom.h"
 #include "pico8_ke.h"
 #include "pico8_layout.h"
-#include "pico8_probe.h"
+#include "pico8_particle.h"
+#include "pico8_probe_comp.h"
 #include "pico8_ptrs.h"
 #include "pico8_research.h"
 #include "pico8_struct.h"
diff --git a/src/pico8_probe.h b/src/pico8_probe.h
index 3165860..daff532 100644
--- a/src/pico8_probe.h
+++ b/src/pico8_probe.h
@@ -2,10 +2,9 @@
 #define PICO8_PROBE_H
 
 #include "pico8_game_data.h"
-#include "pico8_gfx.h"
 #include "pico8_ke.h"
-#include "pico8_mem.h"
 #include "pico8_particle.h"
+#include "pico8_probe_comp.h"
 #include "pico8_struct.h"
 #include "pico8_vec.h"
 
@@ -15,18 +14,6 @@
  * probe for `bv`, `dl`, `d_`, an emitter for `g_:mm` -- and a NULL
  * `gfx`/`mem` skips the draw calls while the rest of the body still runs. */
 
-/* What `bv`, `dl` and `d_` read and write of their host, the probe's own
- * fields, stored in the host (`probe_own`) and never copied; the rest is
- * read on `def`, the host's archetype's probe (`pico8_probe_def_none` on
- * research_icon's `pb`). `et` is borrowed; NULL is nil or false, which no
- * reader tells apart. */
-typedef struct pico8_probe {
-    const pico8_probe_def_t *def;
-    pico8_struct_header_t *et;
-    pico8_fixed_t od;
-    int has_od; /* 0: `od` is nil, and is then never read */
-} pico8_probe_t;
-
 /* TROU-7 */
 /* The emitter's `k_` points at
  * `pico8_g__k__seq` (`pico8_fv_k__seq` for `fv`); `ra` is the very REF `bv`
@@ -57,7 +44,6 @@ void pico8_probe_shutdown(void);
 
 /* 1 from pico8_probe_init() to pico8_probe_shutdown(), 0 otherwise. */
 int pico8_g__ready(void);
-const pico8_struct_class_t *pico8_g__struct_class(void);
 
 /* Initializes an allocated pico8_g__t under `cls`: ke's part, g_'s 4
  * written members nil, `def` kept, `ra`, the emitter and the probe copied
@@ -82,47 +68,6 @@ typedef struct {
  * Asserts when called before `pico8_probe_init()`. */
 pico8_g__t *pico8_g__new(const pico8_g__keys_t *keys);
 
-/* TROU-7 */
-/* `mp` is stored into `ra` by REF identity, not copied: the caller's vector
- * stays aliased. `et` ends in one of three states -- a real target, a
- * static struct with no readable field but `fo`, or literal `false` -- of
- * which only `false` is falsy; collapsing the throwaway to nil would change
- * `d_`. */
-void pico8_bv(pico8_struct_header_t *self, pico8_vec_t *mp, pico8_vec_t *rp);
-
-void pico8_bv_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra);
-
-/* Points the `fo` of `et` back at `self`, which it borrows: `et` is a planet
- * of jo's family, a `jc`, the `jc_too_far` struct or `bv`'s throwaway target
- * (asserted, as is the class of `self` each accepts). */
-void pico8_probe_et_hang_fo(pico8_struct_header_t *et, pico8_struct_header_t *self);
-
-/* The `{}` `bv` puts in `et` over the void: one static struct, never
- * allocated nor freed; only `fo` is written, by pico8_probe_et_hang_fo(),
- * and the next `bv` overwrites it. */
-typedef struct {
-    pico8_struct_header_t header;
-    pico8_struct_header_t *fo;
-} pico8_bv_et_t;
-
-const pico8_struct_class_t *pico8_bv_et_struct_class(void);
-
-/* The tooltip of a probe host: `self` must carry a probe (asserted), and is
- * read, never retained. */
-void pico8_dl(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *self);
-
-/* The same, for a caller that is not a host (the research icon): `me` NULL
- * is nil, `pb` is read during the call only. */
-void pico8_dl_show(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *me, const pico8_probe_t *pb);
-
-void pico8_dl_dispatch(void *self, const pico8_vec_t *ra);
-
-/* Returns 1 on the success branch (Lua true), 0 on the other (nil); `jc:ey`
- * and `mz:ey` consume it. */
-int pico8_d_(pico8_struct_header_t *self);
-
-void pico8_d__dispatch(void *self, pico8_struct_header_t *kv);
-
 /* TROU-7 */
 /* Returns a freshly allocated `pico8_qy_t` (`has_a` 0), one allocation;
  * `ez` appends it to `qy` and nothing ever frees it. */
diff --git a/src/pico8_probe_comp.h b/src/pico8_probe_comp.h
new file mode 100644
index 0000000..16947c9
--- /dev/null
+++ b/src/pico8_probe_comp.h
@@ -0,0 +1,64 @@
+#ifndef PICO8_PROBE_COMP_H
+#define PICO8_PROBE_COMP_H
+
+#include "pico8_fixed.h"
+#include "pico8_game_data.h"
+#include "pico8_gfx.h"
+#include "pico8_mem.h"
+#include "pico8_struct.h"
+#include "pico8_vec.h"
+
+/* What `bv`, `dl` and `d_` read and write of their host, the probe's own
+ * fields, stored in the host (`probe_own`) and never copied; the rest is
+ * read on `def`, the host's archetype's probe (`pico8_probe_def_none` on
+ * research_icon's `pb`). `et` is borrowed; NULL is nil or false, which no
+ * reader tells apart. */
+typedef struct pico8_probe {
+    const pico8_probe_def_t *def;
+    pico8_struct_header_t *et;
+    pico8_fixed_t od;
+    int has_od; /* 0: `od` is nil, and is then never read */
+} pico8_probe_t;
+
+/* TROU-7 */
+/* `mp` is stored into `ra` by REF identity, not copied: the caller's vector
+ * stays aliased. `et` ends in one of three states -- a real target, a
+ * static struct with no readable field but `fo`, or literal `false` -- of
+ * which only `false` is falsy; collapsing the throwaway to nil would change
+ * `d_`. */
+void pico8_bv(pico8_struct_header_t *self, pico8_vec_t *mp, pico8_vec_t *rp);
+
+void pico8_bv_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra);
+
+/* Points the `fo` of `et` back at `self`, which it borrows: `et` is a planet
+ * of jo's family, a `jc`, the `jc_too_far` struct or `bv`'s throwaway target
+ * (asserted, as is the class of `self` each accepts). */
+void pico8_probe_et_hang_fo(pico8_struct_header_t *et, pico8_struct_header_t *self);
+
+/* The `{}` `bv` puts in `et` over the void: one static struct, never
+ * allocated nor freed; only `fo` is written, by pico8_probe_et_hang_fo(),
+ * and the next `bv` overwrites it. */
+typedef struct {
+    pico8_struct_header_t header;
+    pico8_struct_header_t *fo;
+} pico8_bv_et_t;
+
+const pico8_struct_class_t *pico8_bv_et_struct_class(void);
+
+/* The tooltip of a probe host: `self` must carry a probe (asserted), and is
+ * read, never retained. */
+void pico8_dl(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *self);
+
+/* The same, for a caller that is not a host (the research icon): `me` NULL
+ * is nil, `pb` is read during the call only. */
+void pico8_dl_show(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *me, const pico8_probe_t *pb);
+
+void pico8_dl_dispatch(void *self, const pico8_vec_t *ra);
+
+/* Returns 1 on the success branch (Lua true), 0 on the other (nil); `jc:ey`
+ * and `mz:ey` consume it. */
+int pico8_d_(pico8_struct_header_t *self);
+
+void pico8_d__dispatch(void *self, pico8_struct_header_t *kv);
+
+#endif
diff --git a/src/pico8_slipway.c b/src/pico8_slipway.c
index 3d677f9..4daf600 100644
--- a/src/pico8_slipway.c
+++ b/src/pico8_slipway.c
@@ -14,7 +14,7 @@
 #include "pico8_input.h"
 #include "pico8_ke.h"
 #include "pico8_planet.h"
-#include "pico8_probe.h"
+#include "pico8_probe_comp.h"
 #include "pico8_tech_gate.h"
 #include "pico8_ui.h"
 #include "pico8_vec.h"
diff --git a/src/pico8_slipway.h b/src/pico8_slipway.h
index 75d1969..f36e238 100644
--- a/src/pico8_slipway.h
+++ b/src/pico8_slipway.h
@@ -4,7 +4,7 @@
 #include "pico8_fixed.h"
 #include "pico8_game_data.h"
 #include "pico8_ke.h"
-#include "pico8_probe.h"
+#include "pico8_probe_comp.h"
 #include "pico8_ptrs.h"
 #include "pico8_struct.h"
 
@@ -75,7 +75,6 @@ void pico8_jc_shutdown(void);
 
 /* 1 from pico8_jc_init() to pico8_jc_shutdown(), 0 otherwise. */
 int pico8_jc_ready(void);
-const pico8_struct_class_t *pico8_jc_struct_class(void);
 
 /* `p`/`ra` of an end of a slipway (a planet, or the mouse point `jc:hq`
  * hangs on `re` over nothing), read through its link: NULL, an end without
diff --git a/src/pico8_struct.h b/src/pico8_struct.h
index 82718c4..ceff074 100644
--- a/src/pico8_struct.h
+++ b/src/pico8_struct.h
@@ -184,4 +184,9 @@ typedef struct {
     const struct pico8_kind_rec *rec;
 } pico8_entity_kind_t;
 
+/* Here, not in pico8_slipway.h / pico8_probe.h: ui.c compares against
+ * them, and including either header from ui.c makes an include pair. */
+const pico8_struct_class_t *pico8_jc_struct_class(void);
+const pico8_struct_class_t *pico8_g__struct_class(void);
+
 #endif
diff --git a/src/pico8_ui.c b/src/pico8_ui.c
index f936d13..0e82ff1 100644
--- a/src/pico8_ui.c
+++ b/src/pico8_ui.c
@@ -10,8 +10,6 @@
 #include "pico8_mouse.h"
 #include "pico8_pi.h"
 #include "pico8_planet_family.h"
-#include "pico8_probe.h"
-#include "pico8_slipway.h"
 #include "pico8_vec.h"
 #include "pico8_wormhole.h"
 
diff --git a/src/pico8_wormhole.c b/src/pico8_wormhole.c
index fd18dd8..860040d 100644
--- a/src/pico8_wormhole.c
+++ b/src/pico8_wormhole.c
@@ -9,7 +9,7 @@
 #include "pico8_entity_geom.h"
 #include "pico8_ke.h"
 #include "pico8_planet.h"
-#include "pico8_probe.h"
+#include "pico8_probe_comp.h"
 #include "pico8_tech_gate.h"
 
 static int g_mz_up = 0;
diff --git a/src/pico8_wormhole.h b/src/pico8_wormhole.h
index eddb744..f92cb2c 100644
--- a/src/pico8_wormhole.h
+++ b/src/pico8_wormhole.h
@@ -6,9 +6,9 @@
 #include "pico8_game_data.h"
 #include "pico8_gfx.h"
 #include "pico8_ke.h"
+#include "pico8_probe_comp.h"
 #include "pico8_ptrs.h"
 #include "pico8_mem.h"
-#include "pico8_probe.h"
 #include "pico8_struct.h"
 #include "pico8_vec.h"
 
```
