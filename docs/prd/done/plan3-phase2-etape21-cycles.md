---
title: "Plan 3, étape 2.21 : boucles de dépendances entre modules — les deux composantes du graphe d'usage ([4, 19], 70 arêtes internes) cassées par 8 coupes ; `usage-mutuel` étendu des paires aux cycles et gardé ; les 10 relais de 2.19 remplacés par l'inclusion directe ; cinq commits, objets identiques ou écarts expliqués"
description: "Rejoué sur une copie jetable depuis `c54f166` : 2.21a casse la composante de 4 (`entity` → `scroll`) par un membre `md` de la table de classe (`mo:md`, appel de méthode du cart) et `pico8_mo_qb` qui passe dans `scroll` ; 2.21b déplace deux groupes de fonctions chez leur seul appelant (`pico8_jo_hu` → `planet_display`, `bx`/`ck` → `economy`), corps identiques à l'adresse près ; 2.21c finit la composante de 19 par deux définitions déplacées (prototype gardé) et deux prototypes déplacés (`hy_nx` → `fm.h`, `pm_struct_class` → `planet.h`) ; 2.21d remplace les 10 relais par l'inclusion directe (+30 / −10) ; 2.21e étend `usage-mutuel` aux composantes fortement connexes (Tarjan en `awk`), fixture à trois modules, deux mutations du script, garde mordante sur un vrai cycle. Graphe : 625 → 616 arêtes, composantes [4, 19] → []. Chaque commit : `make test-all` vert (72 TU, 26 e2e), pied `résidu 2 : 0 ligne(s), 0 fichier(s)`, aucune paire. `make test` ne ralentit pas (mesuré). Une question à l'humain (graphe des liens)."
status: needs-triage
---

## Problem Statement

**Écrit sur `c54f166`** (2.20 vérifiée et notée). Arbres de départ : `src` `ded3f088…`, `tests` `14e4d629…`, `tools` `389b7136…`, `Makefile` `05b35f5c…`. Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier ; aucun `make` lancé dans le dépôt. Essais sur une seule copie jetable (scratchpad `prd221/`, clone `--no-hardlinks`, arbre de travail `wa`, `third_party/raylib` lié en symbolique pour `make raylib`), supprimée depuis. Notation : `x` = `c54f166` ; `a` … `e` = les commits de la copie qui portent 2.21a à 2.21e. Numéros de ligne : ceux de `x`, sauf mention. Préfixe `pico8_` omis dans les noms quand rien ne prête à confusion.

Vu du mainteneur du port : la phase 2 a démêlé toutes les **paires** de modules (`inclusion-mutuelle` et `usage-mutuel` à 0, gardées), mais le graphe d'usage garde deux **cycles longs** : une composante fortement connexe de 4 modules (le noyau des entités et la caméra) et une de 19 (l'économie, les planètes, l'interface). Dans une composante, aucun module ne se comprend, ne se teste ni ne se porte seul : toucher `economy_formulas` oblige à lire `planet`, qui lit `economy`, qui relit `economy_formulas`. La garde actuelle ne voit que les cycles de longueur 2 ; un cycle de 3 peut entrer sans bruit.

### Texte du plan, recopié (`docs/plan-3.md` à `c54f166`)

> **Décisions de l'humain (2026-10-01, questions de 2.18)** : […] Boucles de dépendances entre modules (reste 2) : une étape dédiée, 2.21.

> - 2.21 — boucles de dépendances entre modules (décision de l'humain du 2026-10-01) : les deux composantes du graphe d'usage (4 et 19 modules) sont cassées. À écrire après 2.20.

Reste 2 de 2.18 (note « Découvert à 2.18 ») : « deux cycles d'usage, 4 et 19 modules (623 arêtes) ». Note « Découvert à 2.17e » : « arêtes 627 → 623 (4 sorties, 0 entrée), composante [25] → [4, 19] ».

Règles communes du plan : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256`, rien sous `tests/e2e/` ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée ; pied du compteur `résidu 2 : 0 ligne(s), 0 fichier(s)` ; aucune paire n'entre ; l'appli raylib n'est jamais lancée ; portage littéral, sans réécriture de logique.

### Le graphe mesuré, à `x`

Définition (étude 2.11, PRD 2.17f) : **a → b quand un `.c`/`.h` du module a, hors `tests/`, emploie un nom que `b.h` est seul à déclarer** ; module = nom du fichier sans extension. La mesure de ce PRD est le portage `awk` du compteur (`read_usages` de `tools/composition_residue.sh`), dont `edges.sh` (annexe A) imprime chaque emploi au lieu du seul premier.

| Mesure à `x` | Valeur |
|---|---|
| emplois lus (`edges.sh`) | 5576 |
| arêtes orientées ; modules | **625** ; 81 |
| composantes fortement connexes > 1 | **[4, 19]**, 70 arêtes internes (5 + 65) |
| paires (`usage-mutuel` du script de `x`) | 0 |

625 et non 623 : le portage `awk` lit, depuis la réserve `b23bdb6` de 2.18, une `static inline` d'en-tête comme un prototype, ce que `usegraph.py` ne fait pas ; les deux arêtes de plus ne touchent aucune composante. Les composantes sont les mêmes avec les deux lecteurs.

**Composante de 4** (`entity`, `entity_geom`, `ke`, `scroll`) — 5 arêtes, emplois entre parenthèses ; en gras, la coupe :
**`entity` → `scroll`** (7) : `qb_md`, `qb_struct_class`, `qb_t` ; `entity_geom` → `entity` (1) : `mo` ; `ke` → `entity` (1) : `lt` ; `scroll` → `entity_geom` (1) : `ma` ; `scroll` → `ke` (3) : `ke_ready`, `ke_struct_init`, `ke_struct_pq`.
Deux cycles : `entity` → `scroll` → `entity_geom` → `entity` et `entity` → `scroll` → `ke` → `entity`.

**Composante de 19** (`bg`, `economy`, `economy_formulas`, `economy_score`, `economy_tiers`, `fm`, `msgbox`, `planet`, `planet_display`, `planet_family`, `planet_trade`, `probe`, `research`, `research_display`, `slipway`, `tech_gate`, `ui`, `ui_button`, `wormhole`) — 65 arêtes ; en gras, les coupes :
**`bg` → `probe`** (1) : `g__new` ; `bg` → `wormhole` (2) : `mz_new`, `mz_ready` ; `economy` → `economy_formulas` (3) : `bx`, `ck`, `qe_map_get` ; `economy` → `economy_score` (3) : `bf`, `cz`, `kr` ; `economy` → `economy_tiers` (2) : `bb`, `bd` ; `economy` → `fm` (17) : `hy_def_t`, `hy_t` ; `economy` → `msgbox` (3) : `iv_keys_t`, `iv_new` ; `economy` → `tech_gate` (3) : `gi_nb`, `ov` ; **`economy_formulas` → `planet`** (5) : `jo_is_family`, `jo_t` ; **`economy_formulas` → `research`** (1) : `jy_t` ; `economy_score` → `economy_formulas` (5) : `qe_map_get`, `qe_map_t` ; `economy_score` → `fm` (4) : `fm_hy`, `hy_t` ; `economy_score` → `planet` (17) : `jo_gg`, `jo_is_family`, `jo_t` ; `economy_score` → `ui_button` (6) : `js_t` ; `economy_tiers` → `planet` (18) : `jo_is_family`, `jo_t` ; `economy_tiers` → `planet_trade` (1) : `jo_ib` ; `economy_tiers` → `research` (9) : `hp_po`, `hp_t`, `jy_new`, `jy_qe`, `jy_t` ; `economy_tiers` → `slipway` (3) : `jc_hg_add`, `jc_t` ; `fm` → `economy_formulas` (4) : `qe_map_t` ; `msgbox` → `ui` (4) : `je_nf`, `ok`, `ui_cj`, `ui_t` ; `msgbox` → `ui_button` (7) : `js_keys_t`, `js_new`, `js_t` ; **`planet` → `economy`** (1) : `hy_nx` ; `planet` → `fm` (1) : `fm_hy` ; **`planet` → `msgbox`** (2) : `iv_keys_t`, `iv_new` ; `planet` → `research` (11) : `gb_at`, `hp_assert`, `hp_t`, `jy_t` ; `planet` → `tech_gate` (1) : `ov` ; `planet_display` → `economy` (1) : `hy_lu` ; `planet_display` → `fm` (1) : `fm_hy` ; `planet_display` → `msgbox` (6) : `pz`, `pz_frame_t` ; `planet_display` → `planet` (30, 11 noms) ; `planet_display` → `research` (4) : `hp_od`, `hp_t`, `jy_t` ; `planet_display` → `research_display` (5) : `bj`, `bj_kept_t`, `hp_jb`, `hp_lq`, `jo_ex` ; `planet_display` → `slipway` (4) : `jc_keys_t`, `jc_new`, `jc_ready`, `jc_t` ; `planet_display` → `ui_button` (4) : `js_keys_t`, `js_t` ; `planet_family` → `planet` (107, 15 noms, surtout les `jo_*_dispatch` des tables) ; `planet_family` → `planet_display` (1) : `jo_kj` ; `planet_family` → `probe` (2) : `g__mm` ; `planet_trade` → `planet` (13) : `jo_cn_dispatch`, `jo_is_family`, `jo_kd`, `jo_kf_dispatch`, `jo_t` ; `planet_trade` → `research` (6) : `jy_new`, `jy_qe`, `jy_t` ; `planet_trade` → `slipway` (4) : `jc_eb`, `jc_t` ; `probe` → `economy` (4) : `hy_lu`, `hy_nx`, `hy_struct_class` ; `probe` → `fm` (12, 7 noms) ; `probe` → `planet` (6) : `jo_is_family`, `jo_struct_class`, `jo_t` ; `probe` → `slipway` (10) : `jc_end_p`, `jc_t`, `jc_too_far_struct_class`, `jc_too_far_t` ; `probe` → `ui` (2) : `je_nf`, `ui_hw` ; `probe` → `wormhole` (7) : `mz_jj_struct_class`, `mz_jj_t`, `mz_struct_class`, `mz_t` ; `research` → `tech_gate` (3) : `oj` ; `research_display` → `planet` (4) : `jo_em`, `jo_is_family`, `jo_t` ; `research_display` → `research` (20, 9 noms) ; `research_display` → `tech_gate` (1) : `oj` ; **`slipway` → `economy`** (1) : `hy_nx` ; `slipway` → `economy_formulas` (1) : `dj` ; `slipway` → `fm` (1) : `fm_hy` ; `slipway` → `planet` (3) : `jo_is_family`, `jo_struct_class`, `jo_t` ; `slipway` → `tech_gate` (1) : `ov` ; `slipway` → `ui` (2) : `ok`, `ui_t` ; `tech_gate` → `economy_formulas` (1) : `qe_map_get` ; `tech_gate` → `fm` (8) : `fm_hy`, `fm_placeholder`, `fm_placeholder_t`, `hy_t` ; `ui` → `bg` (3) : `bg_struct_class`, `bg_t` ; **`ui` → `planet_family`** (1) : `pm_struct_class` ; `ui` → `wormhole` (1) : `mz_struct_class` ; `ui_button` → `planet` (2) : `jo_t` ; `ui_button` → `research` (2) : `hp_t` ; `wormhole` → `planet` (4) : `jo_hh`, `jo_is_family`, `jo_t` ; `wormhole` → `tech_gate` (2) : `ov`.

### Ensemble minimal d'arêtes à couper (feedback arc set exact)

`fas.py` (annexe A) : programmation dynamique sur les sous-ensembles de chaque composante (2¹⁹ états), qui rend le minimum exact et **toutes** les solutions optimales.

| Composante | Minimum (poids 1 par arête) | Solutions | Minimum (poids = emplois) |
|---|---|---|---|
| 4 | **1** | 1 : `entity` → `scroll` | 2 (`ke` → `entity` et `entity_geom` ou `scroll` → `entity_geom`) |
| 19 | **7** | 12 (Runs 4) | 11 |

Les 12 solutions de la composante de 19 partagent `planet` → `economy`, `planet` → `msgbox`, `ui` → `planet_family`, et choisissent dans { `bg` → `probe` | `ui` → `bg` }, { `economy_formulas` → `planet` + `economy_formulas` → `research` | `economy_formulas` → `planet` + `research` → `tech_gate` | `fm` → `economy_formulas` + `tech_gate` → `economy_formulas` }, { `economy` → `economy_tiers` | `slipway` → `economy` }.

**Ensemble retenu : la solution n° 4**, celle dont chaque arête a une coupe sans réécriture (tableau ci-dessous) :
`entity` → `scroll` ; `bg` → `probe`, `economy_formulas` → `planet`, `economy_formulas` → `research`, `planet` → `economy`, `planet` → `msgbox`, `slipway` → `economy`, `ui` → `planet_family`. Huit arêtes, le minimum exact de chaque composante (1 + 7). Le déplacement de `jo_hu` emporte en plus `planet` → `fm`, `planet` → `tech_gate` et `planet` → `audio` (hors composante), sans coût.

### Contrats reçus et ce que ce PRD en fait

| Point (source) | Ce que fait 2.21 |
|---|---|
| plan 2.21 : « les deux composantes (4 et 19 modules) sont cassées » | tenu : [4, 19] → [] à `c` (mesuré, Runs 8–10) |
| demande : technique par arête, portage littéral, déplacement à corps identiques | tableau des coupes ; corps identiques sous `-O0 -DNDEBUG` pour les 9 fonctions déplacées (Runs 15) ; **une** réécriture déclarée (`pico8_mo_qb`, deux lignes), prouvée par jumelles |
| PRD 2.19, G1 et question 1 : les 10 relais renvoyés à 2.21 | 2.21d : les 10 remplacés par l'inclusion directe (+30 / −10, dont 19 `coverage.h`) ; `deadinc.py` passe de 18 à **3** lignes, les 3 faux morts de 2.19 (G5) |
| PRD 2.20, « Contrat pour 2.21 » : 15 catégories gardées ; aucun `#include` changé par 2.20 ; numéros de `composition_residue.sh` | constaté : `CATEGORIES` = `SORTIE` = 15 à `x` et à `e` (`usage-mutuel` change de sens, pas de nom) ; le membre neuf `md` prend `pico8_struct_header_t *self` (`void-self` reste à 0) ; le diff de 2.21e est écrit sur les numéros de `x` (annexe B) |
| PRD 2.17f, G5 : `usage-mutuel` rend le premier emploi de chaque sens d'une paire | étendu : le premier emploi de chaque **arête interne** d'une composante ; pour une paire, sortie identique octet pour octet (les fixtures de 2.17f et 2.18 passent sans changement) |
| demande : garde prouvée mordante par mutation | fixture rouge avec le script de `d`, verte avec celui de `e` ; M1, M2 rouges ; un vrai cycle de 4 modules réintroduit dans `src/` rougit la garde à `e`, la laisse verte à `d` (Runs 12–13) |
| demande : `make test` ne ralentit pas | mesuré : garde 1,10–1,17 s avant et après ; `make -j4 test` chaud 6,95 s → 6,82 s en moyenne de 4 (bruit) ; aucun `run_residue` neuf dans le TU (Runs 14) |

## Solution

Cinq commits, chacun vert et autonome. Les quatre premiers sont le résultat exact de `python3 run221.py <arbre> a|b|c|d` (annexe A) sur l'arbre du commit précédent ; le cinquième est `git apply patch221e.diff` (annexe B, texte exact). Aucun commit ne touche `Makefile`, `*.sha256`, `tests/e2e/`, une constante d'empreinte, `docs/glossaire-noms.md` ou `docs/trous-de-portage.md`.

### Les coupes, arête par arête

| Arête coupée | Sous-étape | Technique | Ce qui bouge | Coupe aussi le lien ? |
|---|---|---|---|---|
| `entity` → `scroll` | a | **inversion par la table de classe** + déplacement | `dc` appelle `mo:md(ci[o])` par un membre neuf `md` de `struct pico8_struct_class` (dernier membre, après `lineage`) ; cible `pico8_qb_md_dispatch` (`static`, `scroll.c`, comme les 56 autres `*_dispatch`) ; `pico8_mo_qb` passe de `entity` à `scroll` (définition et prototype) | oui : la composante de 4 disparaît aussi du graphe des liens |
| `planet` → `economy`, `fm`, `tech_gate` | b | **déplacement chez le seul appelant** | `pico8_jo_hu` (43 lignes) de `planet.c` à la fin de `planet_display.c`, son prototype et son commentaire de `planet.h` à `planet_display.h` ; son seul appelant est `pico8_jo_fe_my_dispatch` (`planet_display.c:320`) | oui |
| `economy_formulas` → `planet`, `research` | b | **déplacement chez le seul appelant** | `cg_visit_fn`, `for_each_cg`, `bx_visit`, `pico8_bx`, `ck_visit`, `pico8_ck` de `economy_formulas.c` à la fin de `economy.c` ; prototypes de `bx`/`ck` à `economy.h`, le commentaire commun scindé (`cm` reste) ; seul appelant `pico8_hy_lj` (`economy.c:216–217`) | oui |
| `planet` → `msgbox` | c | **définition déplacée, prototype gardé** | `pico8_jo_hk` (le texte flottant du survol, qui construit un `iv`) de `planet.c` à la fin de `planet_display.c` ; le prototype reste dans `planet.h`, comme ceux de `jo_kj_dispatch`, `jo_hb_dispatch`… que `planet_display.c` définit déjà | non (lien `planet.o` ↔ `planet_display.o` déjà là, 2.17) |
| `bg` → `probe` | c | **définition déplacée, prototype gardé** | `pico8_bg_ldown` (`(#cg.g_>0 and mz or g_)()` : il construit une sonde ou un trou de ver) de `bg.c` à la fin de `probe.c` ; le prototype reste dans `bg.h` | non |
| `slipway` → `economy` | c | **prototype déplacé** (en-tête scindé) | `void pico8_hy_nx(…)` de `economy.h` à `fm.h`, qui tient déjà `pico8_hy_t` et l'accès `pico8_fm_hy()` à l'unique `hy` ; définition inchangée dans `economy.c` | non |
| `ui` → `planet_family` | c | **prototype déplacé** | `pico8_pm_struct_class` de `planet_family.h` à `planet.h`, à côté de `pico8_jo_kv` (registre des sortes, défini dans `planet_family.c` : précédent de 2.17e) ; seul emploi hors `planet_family` : l'`assert` de `ui_is_held_class` | non |

**Seule réécriture** (2.21a, déclarée) : `pico8_mo_qb` lisait les statiques `g_mo` et `g_mo_placeholder` d'`entity.c` ; dans `scroll.c` il les lit par `pico8_mo()` et `pico8_mo_placeholder_struct_class()` :

```c
pico8_qb_t *pico8_mo_qb(void) {
    pico8_struct_header_t *mo = pico8_mo();

    if (mo == NULL || mo->cls == pico8_mo_placeholder_struct_class()) {
        return NULL;
    }
    assert(mo->cls == pico8_qb_struct_class() && "mo: only the placeholder or a qb is ever stored");
    return (pico8_qb_t *)mo;
}
```

Équivalence : le gabarit est « the only one » (commentaire d'`entity.c:39`, un seul objet de sa classe, jamais alloué) ; comparer sa classe ou son adresse revient au même. Le test `cls->md != NULL` de `dc` remplace `cls == pico8_qb_struct_class()` : seule la table de `qb` pose `.md` (grep : 1 initialiseur). Jumelles Ma1, Ma2 (Testing Decisions) : mêmes TU rouges et même nombre d'e2e au parent et à l'enfant.

**Pourquoi ces quatre techniques et pas d'autres** (G1–G4) : un déplacement chez le seul appelant coupe le lien comme l'usage et ne change aucun corps ; il est pris partout où il existe (b). Les quatre arêtes de c sont des dépendances réelles du cart (le survol d'une planète écrit un texte flottant ; un clic sur le fond lance une sonde ; poser une voie relance l'économie ; l'interface vérifie ce qu'elle tient) : aucun déplacement de fonction ne les ôte sans en créer une autre qui ferme le cycle (mesuré par `sim.py`, Runs 5–7). Le port a déjà deux usages établis pour ce cas : un `.c` qui définit des fonctions déclarées dans l'en-tête d'un module voisin (`planet_display.c` pour `planet.h` ; `planet_family.c` pour `pico8_jo_kv`, 2.17e) ; `fm.h` qui déclare le type de l'économie hors de `economy.h`. Les coupes de c suivent ces deux usages ; elles coupent le graphe d'usage, pas le graphe des liens (question 1).

### Graphe et compteur à chaque sous-étape (mesurés sur la copie)

| | `x` | `a` | `b` | `c` | `d` | `e` |
|---|---|---|---|---|---|---|
| arêtes (`edges.sh`) | 625 | 623 | 620 | **616** | 616 | 616 |
| composantes > 1 (`scc.py`) | [4, 19] | [19] | [15] | **[]** | [] | [] |
| script de `e` sur l'arbre : `usage-mutuel` (composantes, modules) ; pied | 2, 23 ; 67 l. / 23 f. | 1, 19 ; 62 / 19 | 1, 15 ; 39 / 15 | 0 ; **0 / 0** | 0 ; 0 / 0 | 0 ; 0 / 0 |
| script du commit (`make composition-residue`) | pied `0 / 0` | `0 / 0` | `0 / 0` | `0 / 0` | `0 / 0` | `0 / 0` |
| `--detail` md5 (script du commit) | `1c29d5d4…` | idem | idem | idem | idem | idem |
| paires (`usage-mutuel`, `inclusion-mutuelle`) | 0 | 0 | 0 | 0 | 0 | 0 |
| `deadinc.py` (lignes à 0 nom, `src/` + `tests/`) | 18 = 10 relais + 8 faux morts | 13 | 13 | 13 | **3** | 3 |
| `syn.sh` (gcc, `-Werror`, `.h` seuls, ± `-DPICO8_CLASS_COVERAGE`) | 190 lignes de base | = base | = base | = base | = base | = base |
| graphe des liens (`linkg.py`, objets de `src/`) | [2, 4, 17] | [2, 17] | [2, 17] | [2, 17] | [2, 17] | [2, 17] |
| `make test-all` ; e2e conformes | vert ; 26 | vert ; 26 | vert ; 26 | vert ; 26 | vert ; 26 | vert ; 26 |

À `a`, les 5 faux morts `scroll.h` de 2.19 (`test_pico8_fr`, `hj`, `mx`, `nr`, `pm`) deviennent de vrais emplois (`pico8_mo_qb` est désormais dans `scroll.h`) : 18 → 13. À `d`, il ne reste que les 3 autres faux morts de 2.19 (`economy_tiers.c:8`, `fixed.c:2`, `test_pico8_ui.c:35`), que `misc-include-cleaner` voit employés.

### Les 10 relais (2.21d)

| Relais (2.19, numéros de `x`) | Remplacé par |
|---|---|
| `struct.h:6` → `coverage.h` | `#include "pico8_coverage.h"` dans les **19** `.c` qui appellent `pico8_class_coverage_record` sans l'inclure (`bg`, `economy`, `help`, `layout`, `mouse_reader`, `msgbox`, `planet`, `planet_family`, `planet_family_trade`, `probe`, `research`, `research_icon`, `score_panel`, `scroll`, `slipway`, `trade`, `ui`, `ui_button`, `wormhole`) ; la ligne de `struct.h` part |
| `mouse_reader.h:6`, `scroll.h:8`, `slipway.h:6` → `ke.h` | la ligne part de l'en-tête ; `ke.h` entre dans `mouse_reader.c`, `scroll.c`, `planet_family_trade.c`, `tests/test_pico8_jc.c`, `tests/test_pico8_trade.c` |
| `collect.h:4` → `geom.h` | `fixed.h` à la même ligne (`collect.h` n'emploie que `pico8_fixed_t`) |
| `research_icon.c:14` → `probe.h` | `probe_comp.h` à la même ligne (`pico8_probe_t`) |
| `tests/test_pico8_g_.c:37` → `ui.h` | `bg.h` (`pico8_bg_t`) |
| `tests/test_pico8_ga.c:31`, `nl.c:31`, `os.c:31` → `planet_family_trade.h` | `planet_trade.h` (`pico8_jo_ib`) |

Total **+30 / −10**, 30 fichiers (24 de `src/`, 6 TU), objets identiques sous `-DNDEBUG`. Le coût de `struct.h` (19 lignes) est celui que 2.19 avait chiffré ; il est pris (G5).

### La garde : `usage-mutuel` étendu aux composantes (2.21e)

**Tranché : extension de `usage-mutuel`, pas de catégorie neuve.** Une paire est une composante fortement connexe de deux modules ; la catégorie garde son nom, sa place dans `CATEGORIES`, `SORTIE` et `COMPOSITION_RESIDUE_GUARD` (le `Makefile` ne change pas, 15 catégories). Le lecteur `read_usages` garde ses deux passes ; seul son `END` change : Tarjan récursif (`strong`, 12 lignes d'`awk` POSIX) sur les arêtes vues, puis le premier emploi de chaque arête **interne à une composante**, et une ligne par composante (modules triés) dans `usage-mutuel.pairs`. La branche de sortie commune aux catégories de paires lit ce fichier sans changement : `<composantes>\t<modules>` au compte, `usage-mutuel\t<a>\t<b>…` au `--detail`.

Pourquoi l'extension plutôt qu'une catégorie `cycle-usage` : une catégorie neuve recompterait chaque paire une seconde fois au pied (une paire est un cycle), ou devrait exclure les paires par une règle de plus ; l'extension rend, pour une paire, une sortie **identique octet pour octet** à celle de 2.17f (les 52 appels `run_residue` du TU passent sans changement), et elle ne relit pas les fichiers (même passe `awk`).

Fixture : le test existant `test_usage_pairs_see_a_detour_not_a_shared_name_nor_a_comment` reçoit un cycle `x` → `y` → `z` → `x` sans paire (six fichiers) ; ses quatre attendus changent (`usage-mutuel\t2\t5`, la ligne `usage-mutuel\tx\ty\tz`, trois lignes de `--check` de plus, `rm a.c z.c`). Aucun appel `run_residue` n'est ajouté : c'est ce qui garde le TU du compteur, le plus long de `make test`, au même temps (Runs 14). `g` → `f`, déjà dans la fixture, est l'arête hors composante qui ne doit pas sortir (M2).

### Coût en vitesse (mesuré)

| Mesure | `x` | `e` |
|---|---|---|
| `sh tools/composition_residue.sh --check <15 catégories> src tests` (3 essais) | 1,10 / 1,17 / 1,11 s | 1,16 / 1,10 / 1,10 s |
| `--check usage-mutuel` seul | 0,33 s (PRD 2.17f) | 0,30 s |
| `./tmp/test_composition_residue` (3 essais) | 5,6 / 6,2 / 6,3 s | 6,5 / 6,7 / 5,9 s (bruit ; 52 appels `run_residue` des deux côtés) |
| `make -j4 test` chaud (4 essais en alternance) | 7,34 / 6,68 / 6,77 / 7,18 s | 7,21 / 6,68 / 6,34 / 6,59 s |

Une première version de la fixture en test séparé (4 appels `run_residue`) coûtait ≈ +0,5 s à `make -j4 test` (7,22–7,31 s contre 6,55–6,92 s) : écartée (Runs 14).

### Anti-tassement

**Interdit** :
- couper une arête par une étiquette de struct, un `#define`, un `void *`, un cast de pointeur de fonction, une macro, ou en renommant un nom pour qu'il ne soit plus « seul déclaré » ;
- rendre `usage-mutuel` aveugle (exemption de module, filtre de chemin) ; ajouter une catégorie ; toucher le `Makefile` ;
- réécrire un corps déplacé (hors les deux lignes de `pico8_mo_qb`, déclarées) ; fondre, renommer ou rendre `static` une fonction déplacée ; placer `md` ailleurs qu'en dernier membre ;
- remplacer une coupe de c par un déplacement non mesuré : chaque variante passe par `sim.py`, puis par la copie ;
- ajouter un include qui n'apporte aucun nom ; retirer un des 3 faux morts ;
- toucher `*.sha256`, `tests/e2e/`, une constante d'empreinte, `docs/glossaire-noms.md`, `docs/trous-de-portage.md` ;
- réécrire à la main ce que le script ou le diff font, sauf repli (Implementation Chunks).

## User Stories

1. En tant que mainteneur, je veux qu'aucun module ne dépende, même par trois intermédiaires, d'un module qui dépend de lui, afin de lire, tester et porter chaque module en ne lisant que ce qu'il emploie.
2. En tant qu'humain qui a décidé « une étape dédiée », je veux l'ensemble minimal d'arêtes à couper, mesuré, et pour chacune le nom qui la porte et la technique, afin de juger chaque coupe sans relire le graphe.
3. En tant que gardien du portage littéral, je veux que chaque fonction déplacée garde son corps instruction pour instruction, et que la seule réécriture soit nommée et prouvée par jumelles.
4. En tant que gardien de `make test`, je veux qu'un cycle de longueur 3 ou plus qui entre rougisse la garde, sans que la boucle rapide ralentisse.
5. En tant qu'auteur de 2.19, je veux que les 10 relais soient remplacés par l'inclusion directe et que le relevé des includes à 0 nom ne rende plus que les 3 faux morts.
6. En tant que vérificateur, je veux les arbres attendus de chaque commit, le diff exact de la garde et les commandes des preuves, afin de juger sans refaire la recherche.

## Implementation Decisions

### Modules

(préfixe `pico8_` omis ; liste exacte : `git diff --stat` des commits de la copie, Runs 11)
- **2.21a** (12 fichiers, +30 / −17) : `struct.h` (membre `md`), `entity.c` (`dc` ; `mo_qb` part ; includes `dispatch_context.h`, `scroll.h` partis), `entity.h` (prototype de `mo_qb` et include `scroll.h` partis), `scroll.c` (`qb_md_dispatch`, `.md`, `mo_qb`, include `entity.h`), `scroll.h` (prototype de `mo_qb`), `scroll.h` inclus par `bg.c`, `msgbox.c`, `planet_family.c`, `ui.c` ; `msgbox.c` perd `entity.h` (mort) ; TU `ga`, `hp`, `jc` gagnent `scroll.h`.
- **2.21b** (16 fichiers, +151 / −135) : `planet.c`, `planet.h`, `planet_display.c`, `planet_display.h` (+ `research.h`), `economy.c`, `economy.h` (+ `economy_formulas.h`), `economy_formulas.c`, `economy_formulas.h` ; includes morts retirés de `planet.c` (`audio`, `economy`, `fm`, `tech_gate`) et d'`economy_formulas.c` (`planet`, `research`) ; includes directs ajoutés à `planet_display.c` (`audio`, `game_data`, `ke`, `tech_gate`) et à `economy.c` (`ents`, `planet`, `ptrs`, `research`) ; TU `ga`, `hp`, `hy`, `jc`, `jo` (+ `economy.h`), `qh`, `trade`, `ui` gagnent `planet_display.h`.
- **2.21c** (11 fichiers, +55 / −56) : `planet.c` (−`jo_hk`, −`msgbox.h`), `planet_display.c` (+`jo_hk`), `bg.c` (−`bg_ldown`, −`probe.h`, −`wormhole.h`), `probe.c` (+`bg_ldown`, +`bg.h`), `economy.h`, `fm.h`, `slipway.c` (−`economy.h`), `planet_family.h`, `planet.h`, `ui.c` (`planet_family.h` → `planet.h`), TU `entity_kind` (+`planet.h`).
- **2.21d** (30 fichiers, +30 / −10) : le tableau des relais.
- **2.21e** (2 fichiers, +59 / −21) : `tools/composition_residue.sh`, `tests/test_composition_residue.c`.
- Chaque sous-étape : `docs/plan-3.md`, note « Découvert à 2.21x » au bout de la ligne 2.21, que 2.21a découpe en cinq sous-lignes (forme de 2.20).

### Arbres attendus (copie, rejoués deux fois par `chain.sh`, identiques)

| Commit | `src` | `tests` | `tools` | `Makefile` |
|---|---|---|---|---|
| `x` | `ded3f088…` | `14e4d629…` | `389b7136…` | `05b35f5c…` |
| `a` | **`e75229959f9761dc9d19471e2cf899f06aca0e94`** | **`415b04c8030d45ff58c9e4a8e4d530599c5e3b99`** | `389b7136…` | `05b35f5c…` |
| `b` | **`9136e5eb00b792233ffeefffd3739dc066ccedce`** | **`3150383d5f2a88ce1afec333523f85a6f9ba8845`** | idem | idem |
| `c` | **`7707a5963f950b92988aa516216f275404ce2a3a`** | **`2e09c05b10ca1738e7f81278d2107c2b3aa466df`** | idem | idem |
| `d` | **`ad0015991a522b9e40759050c7f35439c1fe3b08`** | **`f20e15682080ae4898d03fb16a5da10d2ba9bd00`** | idem | idem |
| `e` | `ad001599…` (inchangé) | **`5196b7004a083e8afd0b6f6f96a4d2395024f2eb`** | **`40313d9aa539de2c84bd07d18276fd32ca758952`** | `05b35f5c…` (inchangé) |

### Contrat pour la suite (à recopier dans leurs PRD)

| Point | Pour | Ce qu'il faut savoir |
|---|---|---|
| `usage-mutuel` compte des composantes | toutes | une paire reste `1\t2` ; un cycle de n modules sort en une ligne `--detail` de n modules et en autant de lignes `--check` que d'arêtes internes (dédoublonnées par ligne) |
| membre `md` de la table de classe | phase 3, nettoyage | dernier membre, typé en-tête ; seule `qb` le pose |
| graphe des liens | question 1 | composante de 17 objets (`economy*`, `msgbox`, `planet*`, `probe`, `research*`, `slipway*`, `tech_gate`, `ui`, `wormhole*`) et paire `score_panel` ↔ `score_panel_display`, préexistantes, non gardées |
| includes à 0 nom | nettoyage | `deadinc.py` rend 3 faux morts ; aucune garde (question 2 de 2.19, recommandation inchangée) |

## Implementation Chunks

Cinq commits, **≈ 50 min chacun** preuves comprises (machine, mesurée sur la copie : script < 1 s ; `syn.sh` ≈ 8 s ; objets, trois configurations des deux côtés ≈ 40 s ; `make test-all` ≈ 15 s ; `sanitize-net` et `class-coverage-net` ≈ 3 s chacun sur un arbre construit ; jumelles de a, 4 × ≈ 11 s ; le reste est lecture, note et message). **Total ≈ 4 h 10.** Ordre imposé : a, b, c, d, e (e ne peut entrer qu'une fois les composantes à 0 ; d après c pour que `deadinc.py` mesure l'état final des includes).

Par sous-étape x ∈ {a, b, c, d} : copier `run221.py` hors du dépôt ; `python3 run221.py . x` ; `git add -A src tests` ; comparer les arbres au tableau ; preuves ; note ; commit. Pour e : `git apply patch221e.diff` (sha256 en annexe B) ; arbres ; preuves ; note ; commit.

**Repli** : si `run221.py` s'arrête sur une assertion (texte introuvable), on n'adapte pas à la main : arrêt, rapport. Si l'arbre d'arrivée diffère alors que le script passe, le diff contre la copie se lit dans le message ; tout écart qui touche un corps de fonction est un arrêt.

## Testing Decisions

- **Aucun TU neuf ni modifié dans son intention** en a–d : refactor pur ou chemin touché par jumelles. En e, un test existant du compteur reçoit le cycle de trois modules.
- **Prior art** : PRD 2.20 (`objs.sh`, `fncmp.py`, jumelles, `neg220.c`), PRD 2.19 (`deadline.py`, relais, faux morts), PRD 2.17e–f (`usegraph.py`, `usage-mutuel`, garde mordante sur un détour réel), étude 2.11 (`scc.py`, `syn.sh`), règle du parent (D.21).

### Preuves (parent → enfant ; `O` = dossier des scripts de l'annexe A, hors dépôt)

| # | Commande | Attendu |
|---|---|---|
| P1 | `git rev-parse HEAD:src HEAD:tests HEAD:tools HEAD:Makefile` | le tableau « Arbres attendus » |
| P2 | `sh O/syn.sh .` (numéros de ligne et colonne retirés, trié), parent puis enfant, `diff` | vide à chaque sous-étape (190 lignes de base : unités raylib avec `raylib.h` vide, `main_class_coverage.c` sans `-D`) |
| P3 | `sh O/edges.sh . > E && python3 O/scc.py E` ; `sh <script de e> src tests` | ligne « Graphe et compteur » du tableau, sous-étape par sous-étape |
| P4 | `sh O/objs.sh` ×3 (`-O2 -DNDEBUG`, `-O0 -DNDEBUG`, `-O0`), parent et enfant, `cmp` ; `fnbody.py` sur les objets différents ; symboles globaux définis (`nm`, union de tous les objets) | ci-dessous |
| P5 | `make composition-residue` ; `sh tools/composition_residue.sh --detail src tests \| md5sum` ; `make composition-guard` | pied seul `résidu 2 : 0 ligne(s), 0 fichier(s)` ; md5 `1c29d5d4…` (13 lignes) ; `résidu gardé (…15 catégories…) : 0 ligne(s)` |
| P6 | `make test-all` ; `make sanitize-net` ; `make class-coverage-net` ; `make raylib` (vendoring présent, **jamais lancé**) | 72 « all tests passed », `float-grep coeur 120 exclus 33 lignes 0`, `garde fichiers 58 en-echec 0` ; 26 conformes ; `sanitize: 26 propre(s)` ; `couverture: OK` ; `tmp/main_raylib` lié |
| P7 | `WITH_TESTS=1 sh O/edges.sh . > W && python3 O/deadinc.py . W` | 13 lignes à a, b, c (les 10 relais + `economy_tiers.c:8`, `fixed.c:2`, `test_pico8_ui.c:35`) ; **3** à d et e |

**P4, attendus mesurés** (153 objets) :

| Passage | `-O2 -DNDEBUG` | `-O0 -DNDEBUG` | `-O0` | Explication |
|---|---|---|---|---|
| `x` → `a` | 25 diffèrent | 25 | 28 | `struct pico8_struct_class` grandit de 8 octets : toute table de classe (`.rodata`) change de taille ; par fonction (`fnbody.py`), seules diffèrent `pico8_dc` (l'appel par `md`), `test_dc_kj_dispatches_mo_md_then_real_jo_kj` et, sous `-O2`, `spy_kj` (copie `g_spied_jo` d'une table, plus longue) et `pico8_qb_md` (deux instructions de remplissage avant la fonction neuve qui la suit) ; `pico8_mo_qb` diffère (réécriture déclarée) ; sous `-O0`, le reste ne diffère que par `mov $…,%edx` (`NORM_LINE=1`) |
| `a` → `b` | 4 (`economy`, `economy_formulas`, `planet`, `planet_display`) | 4 | 12 | `pico8_jo_hu`, `pico8_bx`, `pico8_ck`, `bx_visit`, `ck_visit`, `for_each_cg` **identiques** d'un objet à l'autre sous `-O0 -DNDEBUG`, et sous `-O0` à `NORM_LINE=1` ; sous `-O2`, l'inlining change (les `*_visit` statiques sont fondus dans leur appelant, `jo_hu` voisine son appelant) : écart attendu, pas un arrêt |
| `b` → `c` | 4 (`bg`, `planet`, `planet_display`, `probe`) | 4 | 6 | `pico8_jo_hk`, `pico8_bg_ldown` identiques sous `-O0 -DNDEBUG` et `-O0` (`NORM_LINE=1`) ; aucun autre corps ne change |
| `c` → `d` | **0** | **0** | 20 | les includes décalent les `__LINE__` des `assert` ; rien d'autre (`NORM_LINE=1`) |
| `d` → `e` | 1 (`test_composition_residue`) | 1 | 1 | le TU du compteur |
| symboles globaux définis | identiques d'une sous-étape à l'autre (`pico8_qb_md_dispatch` est `static`, les fonctions déplacées changent d'objet, pas de nom) | | | |

**Jumelles de 2.21a** (`mut221.sh`, règle du parent : chaque mutation a sa sœur au parent) :

| Nom | Enfant (`a`) | Parent (`x`) | Résultat mesuré, des deux côtés |
|---|---|---|---|
| Ma1 — `dc` n'appelle plus `md` | `g_mo->cls->md != NULL` → `0` | `g_mo->cls == pico8_qb_struct_class()) {` → `0) {` | rouges : `bg`, `fr`, `g_`, `ga`, `hj`, `hp`, `iv`, `lq`, `mx`, `nr`, `planet_kj_dispatch`, `pm` ; e2e conformes : 0 |
| Ma2 — `mo_qb` ne filtre plus le gabarit | `mo == NULL \|\| mo->cls == pico8_mo_placeholder_struct_class()` → `mo == NULL` | `g_mo == NULL \|\| g_mo == &g_mo_placeholder.header` → `g_mo == NULL` | rouges : `je`, `mo`, `qb` ; e2e conformes : 26 |

`gcov` du TU `mo` et de `planet_kj_dispatch` au parent et à l'enfant : l'implémenteur constate que les lignes neuves de `pico8_dc`, `pico8_qb_md_dispatch` et `pico8_mo_qb` sont exécutées (non fait sur la copie ; les jumelles l'impliquent).

**Preuves de 2.21e** :
1. le TU du compteur avec le script de `d` : rouge à la première assertion du test modifié (`:652` sur la copie) ; avec celui de `e` : vert ;
2. M1 (dans `strong`, `{ strong(w); if (slow[w] < slow[v]) slow[v] = slow[w] }` → `{ strong(w) }`) : rouge (`:652`) ; M2 (`if (comp[ab[1]] == comp[ab[2]]) print site[k]` → `print site[k]`) : rouge (`:225`, toute arête sort) ;
3. garde mordante sur l'arbre réel : à `e`, ajouter à `slipway.c` `#include "pico8_economy.h"` et une fonction `int pico8_jc_mut(void) { return pico8_hy_ready(); }` (compile, `-Werror`) ; `make composition-guard` **code 2**, 5 lignes `usage-mutuel`, `--detail` : `usage-mutuel pico8_economy pico8_economy_tiers pico8_planet_trade pico8_slipway`, aucune paire ; la même mutation avec le script de `d` : garde verte, 0 ligne — c'est l'extension qui mord ;
4. vitesse : `make -j4 test` chaud et `--check` des 15 catégories, parent puis enfant, en alternance (tableau « Coût en vitesse ») ; écart toléré ≤ 0,3 s en moyenne de 4.

### Greps de Sortie

1. `python3 O/scc.py <(sh O/edges.sh .)` à `e` → `composantes []`, 616 arêtes.
2. `grep -c '^    void (\*md)(pico8_struct_header_t \*self, pico8_fixed_t fy);$' src/pico8_struct.h` → 1 ; `grep -rn '\.md = ' src` → 1 ligne (`scroll.c`).
3. `grep -n 'pico8_mo_qb' src/pico8_entity.[ch]` → rien ; `grep -n 'pico8_jo_hu\b' src/pico8_planet.[ch]` → rien ; `grep -n 'pico8_bx\b\|pico8_ck\b' src/pico8_economy_formulas.[ch]` → rien.
4. `grep -c '#include "pico8_coverage.h"' src/pico8_struct.h` → 0 ; `grep -l 'pico8_class_coverage_record' src/*.c | xargs grep -L 'pico8_coverage.h'` → rien.
5. `git diff --stat x e -- Makefile tests/e2e '*.sha256' docs/glossaire-noms.md docs/trous-de-portage.md` → vide.
6. `grep -c 'usage-mutuel' tools/composition_residue.sh` → 13 à `x`, 14 à `e` (une ligne d'en-tête de plus ; aucune catégorie neuve) ; `grep -c 'strong' tools/composition_residue.sh` → 4.

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas `c54f166` ou un descendant qui ne touche que `docs/` ; si `run221.py` s'arrête sur une assertion et que le texte a été adapté à la main ; si `patch221e.diff` n'a pas le sha256 de l'annexe B.
- `FAILED:` si un arbre d'arrivée diffère du tableau sans explication écrite dans le message ; si un commit touche un chemin de la grep 5.
- `FAILED:` si une composante reste à `c`, `d` ou `e` ; si une paire entre à une sous-étape ; si le pied du compteur du commit n'est pas `résidu 2 : 0 ligne(s), 0 fichier(s)`.
- `FAILED:` si un corps déplacé diffère sous `-O0 -DNDEBUG` ; si un objet diffère sous `-DNDEBUG` en d ; si une fonction non nommée au tableau P4 diffère ; si `pico8_mo_qb` diffère du texte de la Solution.
- `FAILED:` si une jumelle de a ne rougit pas les mêmes TU et le même nombre d'e2e au parent et à l'enfant.
- `FAILED:` si la fixture n'est pas rouge avec le script de `d` ; si M1 ou M2 la laisse verte ; si la garde ne mord pas sur le cycle réel de la preuve 3, ou mord déjà avec le script de `d`.
- `FAILED:` si `make -j4 test` ralentit de plus de 0,3 s en moyenne de 4 mesures alternées ; si un appel `run_residue` est ajouté au TU.
- `FAILED:` si `make test`, `make e2e` (26 conformes), `sanitize-net` ou `class-coverage-net` restent rouges après 3 tentatives ; si `make raylib` échoue après un vendoring réussi ; si l'appli raylib est lancée.
- `FAILED:` si `deadinc.py` ne rend pas 3 lignes à `d` ; si un des 3 faux morts est retiré ; si un relais reste.
- `FAILED:` s'il manque une note « Découvert à 2.21x », ou si le message omet les arbres, le rapport d'objets, le graphe ou le tableau « Runs ».

## Out of Scope

- **Le renommage de phase 3** (`pico8_jo_kv`, `pico8_kr`, `pico8_fm.h`, `pico8_probe_comp.h`, `md`…) : les noms provisoires restent (décision de l'humain, reste 4).
- **Les 214 aides de TU** qui reçoivent un objet du jeu par `void *` sous un autre nom que `self` : question ouverte à l'humain depuis 2.20d, non tranchée ici.
- **Le graphe des liens** (objets) : ses composantes préexistantes de 17 et de 2 restent (question 1) ; seule celle de 4 tombe, par 2.21a.
- Les détours préexistants qui ne touchent ni une arête coupée ni un relais ; une garde contre les includes morts (2.19, question 2) ; la liste `void-ptr-r2` (nettoyage, contrat de 2.20) ; l'audio ; PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. La solution n° 4 parmi les 12 ensembles minimaux.** Les trois choix libres se tranchent par la technique disponible : `economy_formulas` → `planet`/`research` part par un déplacement chez le seul appelant (les autres variantes, `fm` → `economy_formulas` et `tech_gate` → `economy_formulas`, demanderaient de sortir `qe_map` dans un module neuf) ; `slipway` → `economy` plutôt qu'`economy` → `economy_tiers` (le second se ferait par un prototype de `bb`/`bd` dans `economy.h`, défini ailleurs : moins lisible) ; `bg` → `probe` plutôt qu'`ui` → `bg` (le second toucherait le type de `ui.rheld` et l'`assert` de classe de `ui_bh`).
- **G2. Composante de 4 : la table de classe plutôt que deux prototypes déplacés.** L'autre ensemble minimal pondéré (`ke` → `entity` par `pico8_lt` déclaré dans `ke.h` — le cart écrit bien `ke.pq=lt` — et `entity_geom` → `entity` par `pico8_gd` déplacé dans `entity`) ne coupe que l'usage ; la table coupe aussi le lien, et `mo:md(…)` est un appel de méthode dans le cart. Le membre va en dernier : aucun décalage des 15 autres, objets lisibles.
- **G3. Une réécriture de deux lignes (`pico8_mo_qb`) plutôt qu'un accesseur neuf** dans `entity` : ajouter `pico8_mo_is_placeholder()` créerait un nom sans équivalent dans le cart.
- **G4. Les coupes de c suivent les deux usages établis du port** (définition dans le `.c` voisin, prototype dans l'en-tête qui tient le type). Elles coupent le graphe d'usage, que la décision de l'humain et la garde nomment ; pas le graphe des liens (question 1).
- **G5. Les 10 relais à l'inclusion directe, `struct.h` compris** (19 lignes). Un relais « justifié » demanderait un commentaire ou une liste d'exemption ; la règle du port est « un fichier inclut ce qu'il emploie » (D.13, 2.2), et le coût est mécanique.
- **G6. Extension de `usage-mutuel` plutôt que catégorie neuve** (Solution) ; fixture fondue dans le test existant pour ne pas ajouter d'appel au TU le plus long de `make test`.
- **G7. Cinq commits.** a, b, c tiennent chacun une seule technique (inversion, déplacement, déplacement de déclaration) et se relisent seuls ; d ne touche que des includes ; e ne touche que la garde.

### Surprises de mesure, pour `plan-pilot`

1. **Le minimum exact est petit** : 1 + 7 arêtes pour 70 arêtes internes, et deux déplacements (b) vident déjà la composante de 19 de quatre modules (`fm`, `research`, `tech_gate`, `economy_formulas`) puis `planet`.
2. **Les 5 faux morts `scroll.h` de 2.19 deviennent vrais** dès 2.21a, parce que `pico8_mo_qb` change d'en-tête.
3. **Le TU du compteur domine `make test`** : chaque `run_residue` coûte ≈ 0,1 s sur le chemin critique ; une fixture séparée de 4 appels se voit au chronomètre.
4. **Le graphe des liens a d'autres cycles que le graphe d'usage** (17 et 2 objets), là depuis 2.17 au moins : les en-têtes partagés entre modules voisins les cachent au graphe d'usage.

### Questions pour l'humain

1. Quatre des huit coupes (2.21c) séparent les déclarations sans séparer le code : le compilateur voit toujours ces fichiers s'appeler en boucle, comme il le voit déjà entre les planètes et leur affichage depuis l'étape 2.17. Le graphe que vous avez demandé de casser n'a plus de boucle ; celui de l'éditeur de liens en garde deux, plus anciennes. Recommandation : les laisser, et en faire une étape à part si un jour on veut des modules compilables séparément.

### Annexe A — scripts (texte exact)

Tous hors dépôt, dans un dossier `O` ; lancés depuis la racine d'un arbre de travail. sha256 (16 premiers chiffres) : `run221.py` `7b9815f7f9dd452c`, `mkedges.sh` `920151ee648efee6`, `edges.sh` (sortie de `mkedges.sh`) `5416f04036fc7062`, `scc.py` `68b3c18d8a4b77df`, `fas.py` `e32c4560ef64423e`, `deadinc.py` `64be5e8423d8894a`, `syn.sh` `dcff85f69bbd91ce`, `objs.sh` `13541cd40f3673b1`, `fnbody.py` `623a661cb74c4af7`, `mut221.sh` `ea9642d114e05008`, `linkg.py` `d0d0fb5bfc65fe51`, `chain.sh` `a4a4adbd31da2504`. `sim.py` et `fnsites.py` (exploration des variantes, Runs 5–7) ne sont pas des preuves : omis.

`run221.py` :

```python
# run221.py ARBRE a|b|c|d|e : 2.21, une sous-etape sur un arbre propre au commit
# precedent (a : c54f166). Chaque texte cherche doit etre present exactement
# une fois (ou le nombre donne), sinon arret (AssertionError).
import sys, os
T, STEP = sys.argv[1], sys.argv[2]
def rd(f): return open(os.path.join(T, f)).read()
def wr(f, s): open(os.path.join(T, f), 'w').write(s)
def sub(f, old, new, n=1):
    s = rd(f); c = s.count(old)
    assert c == n, (f, c, old[:70])
    wr(f, s.replace(old, new))
def cut(f, start, end):
    # retire le texte de start (inclus) a end (exclu) ; rend le texte retire
    s = rd(f); i = s.index(start); j = s.index(end, i)
    assert s.count(start) == 1, (f, start[:60])
    wr(f, s[:i] + s[j:]); return s[i:j]
def body(f, sig):
    # la definition de premier niveau qui commence par sig, accolade fermante
    # en colonne 0 comprise, et la ligne blanche qui la suit
    s = rd(f); assert s.count(sig) == 1, (f, sig); i = s.index(sig)
    j = s.index('\n}\n', i) + 3
    if s[j:j + 1] == '\n': j += 1
    wr(f, s[:i] + s[j:]); return s[i:j]
def append_before_endif(f, text):
    s = rd(f); k = s.rindex('#endif'); wr(f, s[:k] + text + '\n' + s[k:])
def append(f, text):
    s = rd(f).rstrip('\n') + '\n\n' + text.rstrip('\n') + '\n'; wr(f, s)
def inc_add(f, h):
    # ajoute #include "h" au bloc des includes "pico8_*" du fichier, dans l'ordre
    if f.startswith('tests/'): h = '../src/' + h
    s = rd(f); line = '#include "%s"\n' % h
    assert line not in s, (f, h)
    lines = s.split('\n'); idx = [k for k, l in enumerate(lines) if l.startswith('#include "')]
    own = os.path.basename(f)[:-2] + '.h'
    cand = [k for k in idx if lines[k] != '#include "%s"' % own]
    pos = None
    for k in cand:
        if lines[k] > line.rstrip('\n'): pos = k; break
    if not cand:
        # seul l'en-tete propre : un bloc neuf apres les includes systeme
        last = max(k for k, l in enumerate(lines) if l.startswith('#include'))
        assert lines[last + 1] == '', f
        lines.insert(last + 2, line.rstrip('\n'))
        if lines[last + 3] != '': lines.insert(last + 3, '')
        wr(f, '\n'.join(lines)); return
    if pos is None: pos = cand[-1] + 1
    lines.insert(pos, line.rstrip('\n')); wr(f, '\n'.join(lines))
def inc_del(f, h):
    if f.startswith('tests/'): h = '../src/' + h
    sub(f, '#include "%s"\n' % h, '')

if STEP == 'a':
    # entity -> scroll : mo:md par la table de classe, mo_qb dans scroll
    sub('src/pico8_struct.h', '    unsigned lineage;\n};',
        '    unsigned lineage;\n    void (*md)(pico8_struct_header_t *self, pico8_fixed_t fy);\n};')
    sub('src/pico8_scroll.c', '''static const pico8_struct_class_t g_qb_struct_class = {
    .states = {[PICO8_STATE_PH] = pico8_qb_ph_dispatch},
};''', '''static void pico8_qb_md_dispatch(pico8_struct_header_t *self, pico8_fixed_t fy) {
    pico8_qb_md(pico8_dispatch_gfx(), (pico8_qb_t *)self, fy);
}

static const pico8_struct_class_t g_qb_struct_class = {
    .states = {[PICO8_STATE_PH] = pico8_qb_ph_dispatch},
    .md = pico8_qb_md_dispatch,
};''')
    sub('src/pico8_entity.c', '''g_mo != NULL &&
                g_mo->cls == pico8_qb_struct_class()) {
                pico8_qb_md(pico8_dispatch_gfx(), (pico8_qb_t *)g_mo, ci);''',
        '''g_mo != NULL &&
                g_mo->cls->md != NULL) {
                g_mo->cls->md(g_mo, ci);''')
    old = body('src/pico8_entity.c', 'pico8_qb_t *pico8_mo_qb(void) {')
    assert 'g_mo == &g_mo_placeholder.header' in old
    append('src/pico8_scroll.c', '''pico8_qb_t *pico8_mo_qb(void) {
    pico8_struct_header_t *mo = pico8_mo();

    if (mo == NULL || mo->cls == pico8_mo_placeholder_struct_class()) {
        return NULL;
    }
    assert(mo->cls == pico8_qb_struct_class() && "mo: only the placeholder or a qb is ever stored");
    return (pico8_qb_t *)mo;
}''')
    sub('src/pico8_entity.h', '''/* `mo` as its `qb`: NULL when there is no mo and on the placeholder. */
pico8_qb_t *pico8_mo_qb(void);

''', '')
    append_before_endif('src/pico8_scroll.h', '''/* `mo` (pico8_entity.h) as its `qb`: NULL when there is no mo and on the
 * placeholder. */
pico8_qb_t *pico8_mo_qb(void);
''')
    inc_del('src/pico8_entity.h', 'pico8_scroll.h')
    inc_del('src/pico8_entity.c', 'pico8_scroll.h')
    inc_del('src/pico8_entity.c', 'pico8_dispatch_context.h')
    inc_add('src/pico8_scroll.c', 'pico8_entity.h')
    for f in ('src/pico8_bg.c', 'src/pico8_msgbox.c', 'src/pico8_planet_family.c', 'src/pico8_ui.c',
              'tests/test_pico8_ga.c', 'tests/test_pico8_hp.c', 'tests/test_pico8_jc.c'):
        inc_add(f, 'pico8_scroll.h')
    inc_del('src/pico8_msgbox.c', 'pico8_entity.h')

if STEP == 'b':
    # planet -> economy, fm, tech_gate : jo:hu passe chez son seul appelant
    hu = body('src/pico8_planet.c', 'void pico8_jo_hu(pico8_jo_t *self, pico8_hp_t *hp_instance) {')
    append('src/pico8_planet_display.c', hu)
    decl = '''/* In the `hp.cw` branch `self` is marked for removal and the tail of the
 * work lands on a fresh planet the caller never sees -- or on nothing,
 * when `hp.cw` is neither "pm" nor "ga". */
void pico8_jo_hu(pico8_jo_t *self, pico8_hp_t *hp_instance);
'''
    sub('src/pico8_planet.h', decl + '\n', '')
    append_before_endif('src/pico8_planet_display.h', decl)
    for h in ('pico8_audio.h', 'pico8_economy.h', 'pico8_fm.h', 'pico8_tech_gate.h'):
        inc_del('src/pico8_planet.c', h)
    for h in ('pico8_audio.h', 'pico8_game_data.h', 'pico8_ke.h', 'pico8_tech_gate.h'):
        inc_add('src/pico8_planet_display.c', h)
    inc_add('src/pico8_planet_display.h', 'pico8_research.h')
    # economy_formulas -> planet, research : bx et ck passent chez leur seul appelant
    f = 'src/pico8_economy_formulas.c'
    walk = cut(f, 'typedef void (*cg_visit_fn)', 'void pico8_cm(')
    bxck = cut(f, 'static void bx_visit(', 'pico8_fixed_t pico8_dj(')
    append('src/pico8_economy.c', walk + bxck)
    sub('src/pico8_economy_formulas.h', '''/* Each empties `out` first. `cm`: every `qe` VALUE of `r` (not `r`'s keys)
 * mapped to `jm`; `bx`: `cm(0)` plus each producing planet's `kw.mu`;
 * `ck`: `cm(1)` less 0.15 per `qh` for the resource of its last `jy`. */
void pico8_cm(pico8_qe_map_t *out, pico8_fixed_t jm);
void pico8_bx(pico8_qe_map_t *out);
void pico8_ck(pico8_qe_map_t *out);
''', '''/* Empties `out` first: every `qe` VALUE of `r` (not `r`'s keys) mapped to
 * `jm`. */
void pico8_cm(pico8_qe_map_t *out, pico8_fixed_t jm);
''')
    append_before_endif('src/pico8_economy.h', '''/* Each empties `out` first. `bx`: `cm(0)` plus each producing planet's
 * `kw.mu`; `ck`: `cm(1)` less 0.15 per `qh` for the resource of its last
 * `jy`. */
void pico8_bx(pico8_qe_map_t *out);
void pico8_ck(pico8_qe_map_t *out);
''')
    inc_add('src/pico8_economy.h', 'pico8_economy_formulas.h')
    for h in ('pico8_planet.h', 'pico8_research.h'):
        inc_del(f, h)
    for h in ('pico8_ents.h', 'pico8_planet.h', 'pico8_ptrs.h', 'pico8_research.h'):
        inc_add('src/pico8_economy.c', h)
    for t in ('ga', 'hp', 'hy', 'jc', 'jo', 'qh', 'trade', 'ui'):
        inc_add('tests/test_pico8_%s.c' % t, 'pico8_planet_display.h')
    if '#include "../src/pico8_economy.h"' not in rd('tests/test_pico8_jo.c'):
        inc_add('tests/test_pico8_jo.c', 'pico8_economy.h')

if STEP == 'c':
    # planet -> msgbox : la definition de jo:hk rejoint les autres methodes
    # d'affichage de jo, dans planet_display.c ; le prototype reste dans planet.h
    hk = body('src/pico8_planet.c', 'void pico8_jo_hk(const pico8_jo_t *self) {')
    append('src/pico8_planet_display.c', hk)
    inc_del('src/pico8_planet.c', 'pico8_msgbox.h')
    # bg -> probe : la definition de bg:ldown, qui construit g_ ou mz, passe dans
    # probe.c ; le prototype reste dans bg.h
    ld = body('src/pico8_bg.c', 'pico8_down_t pico8_bg_ldown(pico8_bg_t *self) {')
    append('src/pico8_probe.c', ld)
    inc_del('src/pico8_bg.c', 'pico8_probe.h')
    inc_del('src/pico8_bg.c', 'pico8_wormhole.h')
    inc_add('src/pico8_probe.c', 'pico8_bg.h')
    # slipway -> economy : le prototype de hy:nx passe dans fm.h, avec hy_t
    sub('src/pico8_economy.h', 'void pico8_hy_nx(pico8_hy_t *self, pico8_fixed_t hx);\n\n', '')
    append_before_endif('src/pico8_fm.h', 'void pico8_hy_nx(pico8_hy_t *self, pico8_fixed_t hx);\n')
    inc_del('src/pico8_slipway.c', 'pico8_economy.h')
    # ui -> planet_family : le prototype de la classe de pm passe dans planet.h,
    # avec le registre des sortes (pico8_jo_kv)
    sub('src/pico8_planet_family.h', 'const pico8_struct_class_t *pico8_pm_struct_class(void);\n', '')
    sub('src/pico8_planet.h', '''/* NULL when no kind has `fz`. */
const pico8_entity_kind_t *pico8_jo_kv(const char *fz);
''', '''/* NULL when no kind has `fz`. */
const pico8_entity_kind_t *pico8_jo_kv(const char *fz);

const pico8_struct_class_t *pico8_pm_struct_class(void);
''')
    inc_del('src/pico8_ui.c', 'pico8_planet_family.h')
    inc_add('src/pico8_ui.c', 'pico8_planet.h')
    inc_add('tests/test_pico8_entity_kind.c', 'pico8_planet.h')

COVERAGE_USERS = ['src/pico8_bg.c', 'src/pico8_economy.c', 'src/pico8_help.c', 'src/pico8_layout.c', 'src/pico8_mouse_reader.c', 'src/pico8_msgbox.c', 'src/pico8_planet.c', 'src/pico8_planet_family.c', 'src/pico8_planet_family_trade.c', 'src/pico8_probe.c', 'src/pico8_research.c', 'src/pico8_research_icon.c', 'src/pico8_score_panel.c', 'src/pico8_scroll.c', 'src/pico8_slipway.c', 'src/pico8_trade.c', 'src/pico8_ui.c', 'src/pico8_ui_button.c', 'src/pico8_wormhole.c']
EXTRA_D = [('tests/test_pico8_jc.c', 'pico8_ke.h'), ('tests/test_pico8_trade.c', 'pico8_ke.h')]

if STEP == 'd':
    # les 10 relais de 2.19 : chaque fichier inclut ce qu'il emploie
    sub('src/pico8_collect.h', '#include "pico8_geom.h"\n', '#include "pico8_fixed.h"\n')
    sub('src/pico8_research_icon.c', '#include "pico8_probe.h"\n', '#include "pico8_probe_comp.h"\n')
    for h, f, users in (('pico8_ke.h', 'src/pico8_mouse_reader.h', ['src/pico8_mouse_reader.c']),
                        ('pico8_ke.h', 'src/pico8_scroll.h', ['src/pico8_scroll.c']),
                        ('pico8_ke.h', 'src/pico8_slipway.h', ['src/pico8_planet_family_trade.c']),
                        ('pico8_coverage.h', 'src/pico8_struct.h', COVERAGE_USERS)):
        inc_del(f, h)
        for u in users: inc_add(u, h)
    inc_del('tests/test_pico8_g_.c', 'pico8_ui.h'); inc_add('tests/test_pico8_g_.c', 'pico8_bg.h')
    for t in ('ga', 'nl', 'os'):
        inc_del('tests/test_pico8_%s.c' % t, 'pico8_planet_family_trade.h')
        inc_add('tests/test_pico8_%s.c' % t, 'pico8_planet_trade.h')
    for f, h in EXTRA_D:
        inc_add(f, h)

if STEP == 'e':
    # la garde est le texte de patch221e.diff (git apply), rien ici
    pass
```

`chain.sh` (rejoue a…e depuis `c54f166` sur un clone jetable ; `O/patch221e.diff` appliqué à l'étape e) :

```sh
#!/bin/sh
# chain.sh ARBRE ETAPE... : depuis c54f166, applique run221.py etape par etape,
# un commit par etape ; imprime les arbres src, tests, tools, Makefile
O=$(cd "$(dirname "$0")" && pwd)
cd "$1" && git checkout -q -f c54f166 && git clean -qfd src tests tools || exit 1
for s in "$@"; do
  [ "$s" = "$1" ] && continue
  python3 "$O/run221.py" . "$s" || exit 1
  [ -f "$O/patch221$s.diff" ] && { git apply "$O/patch221$s.diff" || exit 1; }
  git add -A src tests tools Makefile && git -c user.email=x@x -c user.name=x commit -qm "$s" || exit 1
  echo "$s src $(git rev-parse --short=8 HEAD:src) tests $(git rev-parse --short=8 HEAD:tests) tools $(git rev-parse --short=8 HEAD:tools) Makefile $(git rev-parse --short=8 HEAD:Makefile)"
done
```

`mkedges.sh` (tire `edges.sh` du compteur de `c54f166` : même nettoyage, mêmes motifs de déclaration, mêmes règles — nom déclaré par un seul en-tête, `static inline` lue — mais un emploi par ligne de sortie) :

```sh
#!/bin/sh
# mkedges.sh RESIDU > edges.sh : tire edges.sh de tools/composition_residue.sh
# de c54f166 (CLEAN_AWK l. 200-233, lecteur de read_usages l. 516-580)
R=$1
cat <<'H'
#!/bin/sh
# edges.sh ARBRE : aretes du graphe d usage (portage awk de usage-mutuel),
# une ligne par emploi : a<TAB>b<TAB>nom<TAB>fichier:ligne
cd "$1" || exit 1
find src ${WITH_TESTS:+tests} \( -name "*.c" -o -name "*.h" \) -type f | sort > /tmp/.e_files.$$
grep "\.h$" /tmp/.e_files.$$ > /tmp/.e_heads.$$
H
sed -n 200,233p "$R"
echo "awk \"\$CLEAN_AWK\"'"
sed -n 516,580p "$R"
cat <<'T'
            if (x ~ /^[0-9]/ || !(x in nown) || nown[x] != 1 || own[x] == a) continue
            print a "\t" own[x] "\t" x "\t" FILENAME ":" FNR
        }
    }
    ' pass=1 $(cat /tmp/.e_heads.$$) pass=2 $(cat /tmp/.e_files.$$)
rm -f /tmp/.e_files.$$ /tmp/.e_heads.$$
T
```

`scc.py` :

```python
# scc.py ARETES : composantes fortement connexes (> 1) du graphe a->b lu dans
# ARETES (sortie de edges.sh), puis, par composante, ses aretes internes avec
# le nombre d'emplois et les noms
import sys, collections
E = collections.defaultdict(lambda: collections.defaultdict(list))
for ln in open(sys.argv[1]):
    a, b, x, site = ln.rstrip('\n').split('\t')
    E[a][b].append((x, site))
nodes = set(E) | {b for a in E for b in E[a]}
idx, low, st, on, out, n = {}, {}, [], set(), [], [0]
sys.setrecursionlimit(10000)
def sc(v):
    idx[v] = low[v] = n[0]; n[0] += 1; st.append(v); on.add(v)
    for w in E[v]:
        if w not in idx: sc(w); low[v] = min(low[v], low[w])
        elif w in on: low[v] = min(low[v], idx[w])
    if low[v] == idx[v]:
        c = []
        while True:
            w = st.pop(); on.discard(w); c.append(w)
            if w == v: break
        out.append(sorted(c))
for v in sorted(nodes):
    if v not in idx: sc(v)
print('aretes', sum(len(E[a]) for a in E), 'modules', len(nodes))
big = sorted([c for c in out if len(c) > 1], key=len)
print('composantes', [len(c) for c in big])
for c in big:
    s = set(c); print('==', len(c), ' '.join(m.replace('pico8_', '') for m in c))
    for a in c:
        for b in sorted(E[a]):
            if b in s:
                names = sorted({x for x, _ in E[a][b]})
                print('  %s -> %s\t%d\t%s' % (a.replace('pico8_',''), b.replace('pico8_',''), len(E[a][b]), ' '.join(names[:12]) + (' …' if len(names) > 12 else '')))
```

`fas.py` :

```python
# fas.py ARETES : par composante > 1, l'ensemble minimal d'aretes a couper
# (feedback arc set exact, programmation dynamique sur les sous-ensembles) ;
# poids 1 par arete, puis poids = nombre d'emplois ; toutes les solutions
# optimales en poids 1
import sys, collections, itertools
E = collections.defaultdict(lambda: collections.Counter())
for ln in open(sys.argv[1]):
    a, b, x, site = ln.rstrip('\n').split('\t')
    E[a][b] += 1
def sccs():
    nodes = sorted(set(E) | {b for a in E for b in E[a]})
    idx, low, st, on, out, n = {}, {}, [], set(), [], [0]
    def sc(v):
        idx[v] = low[v] = n[0]; n[0] += 1; st.append(v); on.add(v)
        for w in E[v]:
            if w not in idx: sc(w); low[v] = min(low[v], low[w])
            elif w in on: low[v] = min(low[v], idx[w])
        if low[v] == idx[v]:
            c = []
            while True:
                w = st.pop(); on.discard(w); c.append(w)
                if w == v: break
            out.append(sorted(c))
    for v in nodes:
        if v not in idx: sc(v)
    return [c for c in out if len(c) > 1]
def fas(c, weight):
    k = len(c); ix = {m: i for i, m in enumerate(c)}
    w = [[0]*k for _ in range(k)]
    for a in c:
        for b, cnt in E[a].items():
            if b in ix: w[ix[a]][ix[b]] = (cnt if weight else 1)
    INF = 10**9; N = 1 << k; f = [INF]*N; f[0] = 0
    # ordre : v place apres S ; aretes arriere = v -> S
    for S in range(N):
        if f[S] == INF: continue
        for v in range(k):
            if S >> v & 1: continue
            cost = sum(w[v][u] for u in range(k) if S >> u & 1)
            T = S | 1 << v
            if f[S] + cost < f[T]: f[T] = f[S] + cost
    return f[N-1], f, w
def all_opt(c, f, w, best):
    # tous les ensembles d'aretes arriere optimaux (par retour sur f)
    k = len(c); res = set()
    def rec(S, acc):
        if S == 0: res.add(frozenset(acc)); return
        for v in range(k):
            if not S >> v & 1: continue
            P = S & ~(1 << v)
            back = [(v, u) for u in range(k) if P >> u & 1 and w[v][u]]
            if f[P] + sum(w[v][u] for u in range(k) if P >> u & 1) == f[S]:
                rec(P, acc + back)
    rec((1 << k) - 1, [])
    return [sorted((c[a], c[b]) for a, b in s) for s in res]
for c in sccs():
    short = lambda m: m.replace('pico8_', '')
    for weight in (False, True):
        best, f, w = fas(c, weight)
        sols = all_opt(c, f, w, best) if len(c) <= 19 else []
        print('== %d modules, poids %s : %d ; %d solution(s)' % (len(c), 'emplois' if weight else '1', best, len(sols)))
        for s in sorted(sols)[:40]:
            print('   ', ' ; '.join('%s->%s(%d)' % (short(a), short(b), E[a][b]) for a, b in s))
```

`deadinc.py` :

```python
# deadinc.py ARBRE ARETES : lignes #include "pico8_X.h" (src/, tests/) dont le
# fichier n'emploie aucun nom que X.h est seul a declarer (ARETES : edges.sh
# lance avec WITH_TESTS=1) ; le X.c de X.h est saute
import sys, os, re, collections
T, A = sys.argv[1:3]
used = collections.defaultdict(set)
for l in open(A):
    a, b, x, s = l.rstrip('\n').split('\t'); used[s.rsplit(':', 1)[0]].add(b)
for d in ('src', 'tests'):
    for f in sorted(os.listdir(os.path.join(T, d))):
        if not f.endswith(('.c', '.h')) or 'raylib' in f: continue
        p = d + '/' + f
        for n, l in enumerate(open(os.path.join(T, p), errors='replace'), 1):
            m = re.match(r'\s*#\s*include\s*"(?:\.\./src/)?(pico8_\w+)\.h"', l)
            if not m or m.group(1) == f[:-2]: continue
            if m.group(1) not in used[p]: print('%s:%d\t%s.h' % (p, n, m.group(1)))
```

`syn.sh` (`O/stub/raylib.h` : fichier vide) :

```sh
#!/bin/sh
# syn.sh ARBRE : -fsyntax-only -std=c99 -Wall -Wextra -Werror de chaque .c et
# de chaque .h pris seul (-x c) de src/ et tests/, sans puis avec
# -DPICO8_CLASS_COVERAGE, raylib.h vide (O/stub) ; une ligne par diagnostic
O=$(cd "$(dirname "$0")" && pwd)
cd "$1" || exit 1
for d in "" -DPICO8_CLASS_COVERAGE; do
  for f in src/*.c src/*.h tests/*.c; do
    case $f in tests/e2e/*) continue;; esac
    x=; case $f in *.h) x="-x c";; esac
    cc -std=c99 -Wall -Wextra -Werror -fsyntax-only -I"$O/stub" $d $x "$f" 2>&1 | grep -E 'error' | sed "s|^|$d |"
  done
done
```

`objs.sh` :

```sh
#!/bin/sh
# objs.sh ARBRE OUT "DRAPEAUX" : objets sans -g ni -Werror de src/*.c (hors raylib) et
# tests/*.c -> OUT/<dossier>_<nom>.o (main_class_coverage.c et
# test_class_coverage.c avec -DPICO8_CLASS_COVERAGE en plus) ; "ECHEC f" sinon
cd "$1" || exit 1; mkdir -p "$2"
for f in src/*.c tests/*.c; do
  case $f in *raylib*|*screencast_viewer*) continue;; esac
  x=; case $f in *class_coverage*) x=-DPICO8_CLASS_COVERAGE;; esac
  cc -std=c99 $3 $x -c "$f" -o "$2/$(dirname $f)_$(basename $f .c).o" 2>/dev/null || echo "ECHEC $f"
done
```

`fnbody.py` :

```python
# fnbody.py A.o B.o [FONCTION...] : par fonction definie dans les deux objets
# (ou celles nommees, cherchees dans A puis B), "identique" ou "differe" ;
# instructions sans adresses, sans octets, cibles de saut et d'appel locales
# retirees, relocations gardees par nom de symbole ; NORM_LINE=1 : les
# immediats "mov $...,%edx" (le __LINE__ des assert) sont confondus
import sys, subprocess, re
def funcs(o):
    out = subprocess.run(['objdump', '-d', '-r', '--no-show-raw-insn', o], capture_output=True, text=True).stdout
    F, cur = {}, None
    for l in out.split('\n'):
        m = re.match(r'^[0-9a-f]+ <(.+)>:$', l)
        if m: cur = m.group(1); F[cur] = []; continue
        if cur is None or not l.strip(): continue
        l = re.sub(r'^\s*[0-9a-f]+:\s*', '', l)
        l = re.sub(r'R_X86_64_\S+\s+', 'R ', l)
        l = re.sub(r'\b[0-9a-f]+ <[^>]*>', '<j>', l)
        l = re.sub(r'\.text\+0x[0-9a-f]+|\.rodata(\.\S+)?\+0x[0-9a-f]+|\.rodata(\.\S+)?-0x[0-9a-f]+', 'SEC', l)
        F[cur].append(l.strip())
    return F
import os
if os.environ.get("NORM_LINE"):
    _f = funcs
    def funcs(o):
        return {k: [re.sub(r"mov\s+\$0x[0-9a-f]+,%edx", "mov $L,%edx", x) for x in v] for k, v in _f(o).items()}
A, B = funcs(sys.argv[1]), funcs(sys.argv[2])
names = sys.argv[3:] or sorted(set(A) & set(B))
for n in names:
    if n not in A or n not in B: print(n, 'absent', 'A' if n not in A else 'B'); continue
    print(n, 'identique' if A[n] == B[n] else 'differe')
```

Emploi, fonctions déplacées (a → b, b → c) : `python3 O/fnbody.py obj_a/src_pico8_planet.o obj_b/src_pico8_planet_display.o pico8_jo_hu`, `… obj_a/src_pico8_economy_formulas.o obj_b/src_pico8_economy.o pico8_bx pico8_ck bx_visit ck_visit for_each_cg`, `… obj_b/src_pico8_planet.o obj_c/src_pico8_planet_display.o pico8_jo_hk`, `… obj_b/src_pico8_bg.o obj_c/src_pico8_probe.o pico8_bg_ldown` (`-O0 -DNDEBUG` ; sous `-O0`, `NORM_LINE=1`).

`mut221.sh` :

```sh
#!/bin/sh
# mut221.sh ARBRE FICHIER TEXTE REMPLACEMENT : une mutation (texte unique),
# make -k -j4 unit puis make -k e2e ; imprime les TU rouges et le nombre
# d'e2e conformes ; restaure par git checkout -f
cd "$1" || exit 1
python3 - "$2" "$3" "$4" <<'PY' || exit 1
import sys
f, a, b = sys.argv[1:]; s = open(f).read(); assert s.count(a) == 1, (f, s.count(a)); open(f, 'w').write(s.replace(a, b))
PY
timeout 600 make -k -j4 unit > /tmp/.mut221.$$ 2>&1
red=$(grep -o 'test_[a-z0-9_]*: [^ ]*\.c:[0-9]*' /tmp/.mut221.$$ | cut -d: -f1 | sort -u | tr '\n' ' ')
timeout 600 make -k e2e > /tmp/.mut221e.$$ 2>&1
ok=$(grep -c ': conforme' /tmp/.mut221e.$$)
echo "rouges: ${red:-aucun} ; e2e conformes: $ok"
rm -f /tmp/.mut221.$$ /tmp/.mut221e.$$
git checkout -q -f -- src tests
```

`linkg.py` (objets de `src/` d'un dossier, `src_` retiré des noms) :

```python
# linkg.py OBJDIR : graphe des liens (a.o a un symbole indefini que b.o
# definit), composantes fortement connexes > 1 ; main*.o exclus
import sys, os, subprocess, collections
D = sys.argv[1]; defs = {}; und = collections.defaultdict(set)
for f in sorted(os.listdir(D)):
    if not f.endswith('.o') or f.startswith('main'): continue
    m = f[:-2]
    for l in subprocess.run(['nm', D + '/' + f], capture_output=True, text=True).stdout.split('\n'):
        p = l.split()
        if len(p) == 3 and p[1] in 'TDRB': defs[p[2]] = m
        elif len(p) == 2 and p[0] == 'U': und[m].add(p[1])
E = collections.defaultdict(set)
for m, us in und.items():
    for s in us:
        if s in defs and defs[s] != m: E[m].add(defs[s])
nodes = sorted(set(defs.values()))
idx, low, st, on, out, c = {}, {}, [], set(), [], [0]
sys.setrecursionlimit(10000)
def sc(v):
    idx[v] = low[v] = c[0]; c[0] += 1; st.append(v); on.add(v)
    for w in E[v]:
        if w not in idx: sc(w); low[v] = min(low[v], low[w])
        elif w in on: low[v] = min(low[v], idx[w])
    if low[v] == idx[v]:
        k = []
        while True:
            w = st.pop(); on.discard(w); k.append(w)
            if w == v: break
        out.append(sorted(k))
for v in nodes:
    if v not in idx: sc(v)
big = [k for k in out if len(k) > 1]
print('modules', len(nodes), 'aretes', sum(len(E[a]) for a in E), 'composantes', sorted(len(k) for k in big))
for k in big: print('  ', ' '.join(x.replace('pico8_', '') for x in k))
```

### Annexe B — `patch221e.diff` (2.21e, `git diff d e`, texte exact, 137 lignes, sha256 `c3355741c5574eb135b3450498df09dc8baad5ddba626e197a7e8007e2778788`)

```diff
diff --git a/tests/test_composition_residue.c b/tests/test_composition_residue.c
index a93383f..f92637f 100644
--- a/tests/test_composition_residue.c
+++ b/tests/test_composition_residue.c
@@ -621,9 +621,10 @@ static void test_mutual_inclusion_counts_its_include_lines_in_the_footer(void) {
 }
 
 /* a et b s'emploient, mais a.c ne recoit b.h que par c.h : inclusion-mutuelle
- * n'y voit rien. d et e emploient u, qui ne les emploie que par s_t, que d.h
- * et e.h declarent tous deux ; f n'emploie g que dans un commentaire et une
- * chaine : ni u-d, ni u-e, ni f-g n'est une paire. */
+ * n'y voit rien. x -> y -> z -> x, sans paire, est une composante. d et e
+ * emploient u, qui ne les emploie que par s_t, que d.h et e.h declarent tous
+ * deux ; f n'emploie g que dans un commentaire et une chaine : ni u-d, ni
+ * u-e, ni f-g n'est une paire ; g -> f n'est dans aucune composante. */
 static void test_usage_pairs_see_a_detour_not_a_shared_name_nor_a_comment(void) {
     setup();
     write_file(SRC "/a.h", "int a_f(void);\n");
@@ -641,17 +642,27 @@ static void test_usage_pairs_see_a_detour_not_a_shared_name_nor_a_comment(void)
     write_file(SRC "/g.h", "int g_h(void);\n");
     write_file(SRC "/f.c", "/* g_h */\nconst char *f_n = \"g_h\";\nint f_k(void) { return 0; }\n");
     write_file(SRC "/g.c", "#include \"f.h\"\nint g_h(void) { return f_k(); }\n");
+    write_file(SRC "/x.h", "int x_f(void);\n");
+    write_file(SRC "/y.h", "int y_f(void);\n");
+    write_file(SRC "/z.h", "int z_f(void);\n");
+    write_file(SRC "/x.c", "#include \"y.h\"\nint x_f(void) { return y_f(); }\n");
+    write_file(SRC "/y.c", "#include \"z.h\"\nint y_f(void) { return z_f(); }\n");
+    write_file(SRC "/z.c", "#include \"x.h\"\nint z_f(void) { return x_f(); }\n");
     assert(run_residue(TREE) == 0);
-    assert(strcmp(read_file(OUT), "usage-mutuel\t1\t2\n"
-                                  "résidu 2 : 2 ligne(s), 2 fichier(s)\n") == 0);
+    assert(strcmp(read_file(OUT), "usage-mutuel\t2\t5\n"
+                                  "résidu 2 : 5 ligne(s), 5 fichier(s)\n") == 0);
     assert(run_residue("--detail " TREE) == 0);
     assert(strcmp(read_file(OUT), "usage-mutuel\ta\tb\n"
-                                  "résidu 2 : 2 ligne(s), 2 fichier(s)\n") == 0);
+                                  "usage-mutuel\tx\ty\tz\n"
+                                  "résidu 2 : 5 ligne(s), 5 fichier(s)\n") == 0);
     assert(run_residue("--check inclusion-mutuelle,usage-mutuel " TREE) == 1);
     assert(strcmp(read_file(OUT), "usage-mutuel: " SRC "/a.c:4:    return b_g();\n"
                                   "usage-mutuel: " SRC "/b.c:3:    return a_f();\n"
-                                  "résidu gardé (inclusion-mutuelle,usage-mutuel) : 2 ligne(s)\n") == 0);
-    assert(system("rm " SRC "/a.c") == 0);
+                                  "usage-mutuel: " SRC "/x.c:2:int x_f(void) { return y_f(); }\n"
+                                  "usage-mutuel: " SRC "/y.c:2:int y_f(void) { return z_f(); }\n"
+                                  "usage-mutuel: " SRC "/z.c:2:int z_f(void) { return x_f(); }\n"
+                                  "résidu gardé (inclusion-mutuelle,usage-mutuel) : 5 ligne(s)\n") == 0);
+    assert(system("rm " SRC "/a.c " SRC "/z.c") == 0);
     assert(run_residue("--check usage-mutuel " TREE) == 0);
     assert(strcmp(read_file(OUT), "résidu gardé (usage-mutuel) : 0 ligne(s)\n") == 0);
 }
diff --git a/tools/composition_residue.sh b/tools/composition_residue.sh
index 8a96836..9c57a40 100644
--- a/tools/composition_residue.sh
+++ b/tools/composition_residue.sh
@@ -6,18 +6,20 @@
 # atteinte-à-prouver compte les lignes utiles de COMPOSITION_ATTEINTE ;
 # inclusion-mutuelle compte des paires de modules hors tests/ ; au pied et a
 # --check, ses lignes sont les #include qui realisent ces paires.
-# usage-mutuel compte les paires de modules hors tests/ dont chacun emploie
-# un nom que l'en-tete de l'autre est seul a declarer, include ou detour ;
-# ses lignes sont, par paire et par sens, le premier emploi.
+# usage-mutuel compte les composantes fortement connexes du graphe d'usage
+# des modules hors tests/ (a -> b : a emploie un nom que l'en-tete de b est
+# seul a declarer, include ou detour), paires et cycles plus longs ; ses
+# lignes sont, par arete interne a une composante, le premier emploi.
 #   sans option : "<cat>\t<lignes>\t<fichiers>" par categorie non nulle
-#                 ("<paires>\t<modules>" pour inclusion-mutuelle et
-#                 usage-mutuel), puis
+#                 ("<paires>\t<modules>" pour inclusion-mutuelle,
+#                 "<composantes>\t<modules>" pour usage-mutuel), puis
 #                 "résidu 2 : N ligne(s), M fichier(s)" (couples fichier:ligne
 #                 et fichiers distincts des categories de SORTIE) ; code 0
 #                 quel que soit le compte.
 #   --detail    : "<cat>\t<fichier>\t<lignes>" par couple non nul,
-#                 "<cat>\t<a>\t<b>" par paire d'inclusion-mutuelle et
-#                 d'usage-mutuel, sous void-ptr les
+#                 "<cat>\t<a>\t<b>" par paire d'inclusion-mutuelle,
+#                 "<cat>\t<a>\t<b>..." par composante d'usage-mutuel
+#                 (modules tries), sous void-ptr les
 #                 "void-ptr-r2\t<fonction>\t<amont>" de COMPOSITION_R2, la
 #                 ligne "info" de pico8_game_data.h s'il est lu, puis le meme
 #                 pied ; code 0.
@@ -504,8 +506,11 @@ read_inclusions() {
 # usegraph.py ne fait pas. Un nom que deux
 # en-tetes declarent n'est a aucun : les etiquettes, lues sans cette regle,
 # feraient six paires par les declarations anticipees de pico8_struct.h.
-# Rend dans $work/usage-mutuel.pairs les paires, dans $work/usage-mutuel le
-# premier emploi de chaque sens (ordre de $work/files, puis des lignes).
+# Les composantes sont celles de Tarjan sur les aretes vues (strong) ; une
+# paire est une composante de deux modules. Rend dans
+# $work/usage-mutuel.pairs une ligne par composante (modules tries), dans
+# $work/usage-mutuel le premier emploi de chaque arete interne (ordre de
+# $work/files, puis des lignes).
 read_usages() {
     grep -v '/tests/' "$work/files" | grep -v '^tests/' > "$work/modfiles" || true
     : > "$work/usage-mutuel.pairs"
@@ -582,13 +587,35 @@ read_usages() {
             if (!((a, own[x]) in site)) site[a, own[x]] = FILENAME ":" FNR ":" $0
         }
     }
+    function strong(v,    i, w) {
+        sidx[v] = slow[v] = ++scount; sstack[++ssp] = v; son[v] = 1
+        for (i = 1; i <= nadj[v]; i++) {
+            w = adj[v, i]
+            if (!(w in sidx)) { strong(w); if (slow[w] < slow[v]) slow[v] = slow[w] }
+            else if (son[w] && sidx[w] < slow[v]) slow[v] = sidx[w]
+        }
+        if (slow[v] == sidx[v]) {
+            ncomp++
+            do { w = sstack[ssp--]; son[w] = 0; comp[w] = ncomp; size[ncomp]++; mem[ncomp, size[ncomp]] = w } while (w != v)
+        }
+    }
     END {
         for (k in site) {
             split(k, ab, SUBSEP)
-            if (ab[1] < ab[2] && ((ab[2], ab[1]) in site)) {
-                print ab[1] "\t" ab[2] > pairs
-                print site[k]; print site[ab[2], ab[1]]
-            }
+            adj[ab[1], ++nadj[ab[1]]] = ab[2]; node[ab[1]]; node[ab[2]]
+        }
+        for (v in node) if (!(v in sidx)) strong(v)
+        for (k in site) {
+            split(k, ab, SUBSEP)
+            if (comp[ab[1]] == comp[ab[2]]) print site[k]
+        }
+        for (c = 1; c <= ncomp; c++) {
+            if (size[c] < 2) continue
+            for (i = 2; i <= size[c]; i++)
+                for (j = i; j > 1 && mem[c, j - 1] > mem[c, j]; j--) { w = mem[c, j]; mem[c, j] = mem[c, j - 1]; mem[c, j - 1] = w }
+            line = mem[c, 1]
+            for (i = 2; i <= size[c]; i++) line = line "\t" mem[c, i]
+            print line > pairs
         }
     }
     ' pass=1 $(cat "$work/modheaders") pass=2 $(cat "$work/modfiles") > "$work/usage-mutuel" || fail "echec de awk"
```

## Runs

Essais de ce PRD, sur la copie jetable (`prd221/`, clone `--no-hardlinks` de `c54f166` ; `wt` = départ, `wa` = arbre de travail de la chaîne a…e). Durées : horloge murale.

| # | Commande | Arbre | Code | Sortie (résumé) | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` ; `git rev-parse HEAD:src HEAD:tests HEAD:tools HEAD:Makefile` | `x` | 0 | `ded3f088`, `14e4d629`, `389b7136`, `05b35f5c` | 1 s |
| 2 | `edges.sh` (premier jet, puis `mkedges.sh`, sortie identique) ; `scc.py` | `x` | 0 | 5576 emplois, 625 arêtes, 81 modules, composantes [4, 19] | 0,26 s |
| 3 | `fnsites.py` (fonctions qui portent chaque arête interne) | `x` | 0 | 70 arêtes ; table de la Problem Statement | 1 s |
| 4 | `fas.py` | `x` | 0 | 4 : min 1, 1 solution (`entity`→`scroll`) ; pondéré 2. 19 : min 7, 12 solutions ; pondéré 11 | 20 s |
| 5 | `sim.py` : `jo_hu`→`planet_display`, `bx`/`ck`→`economy` | `x` | 0 | 625 → 623 ; [4, 15] ; aucune paire | 1 s |
| 6 | `sim.py` : + `jo_hk` (définition) ; variantes `lt`→`ke.h` + `gd`→`entity` ; `md` + `mo_qb`→`scroll` | `x` | 0 | [4, 10] ; les deux variantes de 4 cassent la composante (la seconde garde le cycle des liens) | 3 s |
| 7 | `sim.py` : + `hy_nx`→`fm.h`, `pm_struct_class`→`planet.h`, `bg_ldown`→`probe.c` ; FAS résiduel | `x` | 0 | [4, 8] puis [4] ; FAS du reste : 3 (`bg`→`probe`, `slipway`→`economy`, `ui`→`planet_family`) | 2 s |
| 8 | `linkg.py` (objets `-O0 -DNDEBUG` de `src/`) | `x` | 0 | 68 objets, 428 arêtes, [2, 4, 17] | 3 s |
| 9 | `syn.sh` de base | `x` | 0 | 190 lignes (raylib vide, couverture sans `-D`) | 8 s |
| 10 | `run221.py a` (premier jet) ; `syn.sh` | `a` | 0 | 7 fichiers en erreur (`pico8_mo_qb`, `pico8_qb_t`) : `scroll.h` ajouté à `bg`, `msgbox`, `planet_family`, `ui`, TU `ga`, `hp`, `jc` | 9 s |
| 11 | `run221.py a` ; `syn.sh` ; `make -j4 test-all` ; `deadinc.py` | `a` | 0 | `syn` = base ; 72 TU, 26 conformes, gardes à 0 ; `deadinc` : `msgbox.c:10 entity.h` mort → retiré dans le script ; 12 fichiers +30/−17 | 25 s |
| 12 | `run221.py b` (premier jet) ; `syn.sh` | `b` | 0 | 8 TU en erreur (`pico8_jo_hu`, `pico8_bx`) : `planet_display.h` (et `economy.h` pour `jo`) ajoutés | 9 s |
| 13 | `run221.py b` ; `syn.sh` ; `make -j4 test-all` | `b` | 0 | `syn` = base ; vert, 26 conformes ; [15], 620 arêtes ; 16 fichiers +151/−135 | 25 s |
| 14 | `run221.py c` ; `syn.sh` ; détours des noms déplacés ; `make -j4 test-all` | `c` | 0 | composantes [] ; 616 arêtes ; un détour (`test_pico8_entity_kind.c`, `pm_struct_class`) → `planet.h` ajouté ; vert, 26 conformes | 30 s |
| 15 | `run221.py d` (premier jet) | `d` | 1 | `IndexError` dans `inc_add` (`trade.c` n'a que son en-tête) → bloc neuf ; puis double ligne blanche → corrigé (le blanc existant est repris) | 1 s |
| 16 | `run221.py d` ; `syn.sh` | `d` | 0 | 2 TU en erreur (`pico8_ke_struct_pq`, `pico8_ke_ks`) → `ke.h` ajouté à `test_pico8_jc.c`, `test_pico8_trade.c` | 9 s |
| 17 | `run221.py d` ; `syn.sh` ; `deadinc.py` ; `make -j4 test-all` | `d` | 0 | `syn` = base ; 3 lignes (faux morts) ; vert, 26 conformes ; 30 fichiers +30/−10 | 25 s |
| 18 | prototype de l'extension sur `x` : `--detail`, `--check usage-mutuel` ; TU du compteur | `x` | 0 | `usage-mutuel 2 23`, pied 67 / 23 ; 67 lignes ; TU vert sans changement (paires identiques) | 1,1 s ; 0,30 s ; 5,5 s |
| 19 | garde 15 catégories, script de `x` contre script étendu, 3 fois | `x` | 1 | 1,10 / 1,17 / 1,11 s contre 1,16 / 1,10 / 1,10 s | 7 s |
| 20 | fixture en test séparé (4 `run_residue`) ; `make -j4 test` chaud ×3 | `e` (1er jet) | 0 | 7,22–7,31 s contre 6,55–6,92 s à `x` : écartée | 45 s |
| 21 | fixture fondue dans le test existant ; TU avec le script de `d` ; M1 ; M2 ; script de `e` | `e` | 0 | rouge `:652` ; M1 rouge `:652` ; M2 rouge `:225` ; vert | 25 s |
| 22 | garde mordante : `pico8_jc_mut` dans `slipway.c` (+`economy.h`), compile ; garde ; `--detail` ; script de `d` | `e` | 0 | 5 lignes `usage-mutuel`, composante `economy economy_tiers planet_trade slipway`, aucune paire ; script de `d` : 0 ligne | 3 s |
| 23 | `chain.sh wa a b c d e` (deux fois) | — | 0 | arbres du tableau, identiques aux deux rejeux | 4 s |
| 24 | `make -j4 test-all` ; `make raylib` (lien seul, jamais lancé) ; `make sanitize-net` ; `make class-coverage-net` | `e` | 0 | 72 TU, 26 conformes ; `tmp/main_raylib` lié ; `sanitize: 26 propre(s)` ; `couverture: OK` | 15 s ; 30 s ; 3 s ; 3 s (arbre déjà construit) |
| 25 | `make -j4 test` chaud, 4 alternances `x`/`e` ; TU du compteur 3 alternances | `x`, `e` | 0 | tableau « Coût en vitesse » | 1 min |
| 26 | `objs.sh` ×3 drapeaux sur `x`, `a`…`e` (premier jet avec `-Werror` : `ECHEC` sous `-DNDEBUG`, paramètre inutilisé d'`assert_hy` → `-Werror` retiré) | 6 | 0 | 153 objets par configuration | ≈ 7 s par configuration et par arbre |
| 27 | `cmp` des objets ; `fnbody.py` (± `NORM_LINE=1`) ; union `nm` | 6 | 0 | tableau P4 | 20 s |
| 28 | jumelles Ma1, Ma2 (`mut221.sh`) à `a` et à `x` | `a`, `x` | 0 | tableau des jumelles, identiques des deux côtés | ≈ 11 s chacune |
| 29 | `deadinc.py` à `x`, `a`…`d` ; `--detail \| md5sum` et pied, script du commit et script de `e` | 6 | 0 | 18 / 13 / 13 / 13 / 3 / 3 ; `1c29d5d4…` partout ; tableau « Graphe et compteur » | 10 s |
| 30 | `linkg.py` à `e` | `e` | 0 | 68 objets, 426 arêtes, [2, 17] | 3 s |
