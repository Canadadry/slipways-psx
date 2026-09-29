---
title: "Plan 3, étape D.15b : `valeur`, mécanique — `pico8_ij` et `pico8_jc_ce` en pointeurs d'en-tête, `pico8_d_` en entier, `pico8_jc_hg_add/has` en `const char *`, `g_ie_id` en nombre + drapeau, `pf/mz_value_vec` retirés ; les TU suivent dans le même commit ; aucune constante ne change"
description: "Six API et un état de module, qui ne prennent chacun qu'un seul genre non nil (une entité ou rien, vrai ou rien, une chaîne, un nombre ou rien), passent encore par `pico8_value_t` ; D.15b leur donne leur type C (règle (a) de l'étude D.14), retire les enveloppes et les deux copies `*_truthy` qui n'avaient plus que `d_` pour argument, fait suivre 9 TU sans changer une constante : `valeur` baisse de 69 lignes (70 retirées dont un include devenu inutile, 1 gagnée et déclarée)."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.15a était implémentée** (`plan-pilot`, étape 2b), sur l'arbre commis `a702ce4`, puis **révisé après sa graduation** : D.15a gradée (`48d165e`, VERIFIED ; « docs: D.15a gradée (ov en nombres, cb en entier) » **`77c916a`**). Les hypothèses `H-D15a-n` sont confrontées plus bas à `77c916a` ; les comptes et **tous les numéros de ligne de ce PRD sont ceux de `77c916a`**, remesurés par contenu sur `git archive 77c916a` (seuls `slipway.c` ≥ 270 et `wormhole.c` ≥ 76 ont bougé parmi les fichiers de l'étape). Seuls `grep`, `sed`, `awk`, `git diff a702ce4 77c916a` et `sh tools/data_residue.sh` ont tourné. **Rien n'a été compilé ni lancé.** Si le HEAD de départ n'est pas `77c916a` (ou un descendant qui ne touche que `docs/`), l'implémenteur remesure par contenu et explique l'écart.

Le problème, vu du mainteneur du port : la Sortie de la phase D exige que `pico8_value.c/.h` disparaissent (D.19c). L'étude D.14 (`docs/prd/done/plan3-phaseD-etape14-etude-genres-meles.md`) a classé 560 lignes `valeur` comme **(a) mécaniques** : une valeur qui ne prend qu'un seul genre non nil et dont aucun lecteur ne distingue nil de `false` devient son type C, avec `NULL` ou un drapeau `has_x` pour nil. D.15b en prend le deuxième lot, celui des pointeurs :

- `pico8_ij` rend le plus proche voisin par un `pico8_value_t *out_nearest` qui ne vaut jamais que `REF NULL` ou `REF` d'une entité — D.12a l'avait gardé tel quel en attendant Q12 (« `out_nearest` reste `pico8_value_t *`, Q12 ») ;
- `pico8_jc_ce` rend `pico8_value_ref(w)`, `pico8_value_ref(&g_jc_too_far)` ou `REF NULL` — toujours une référence (D.12b : « `jc:ce` rend `pico8_value_ref(w)` (Q12) ») ; son seul appelant la redéballe par `pico8_jc_value_instance` ;
- `pico8_d_` rend `pico8_value_bool(1)` ou `REF NULL`, que ses deux appelants de `src/` relisent par une copie privée de `*_truthy` qui n'existe que pour cela ;
- `pico8_jc_hg_add/has` prennent un `pico8_value_t id` alors que `src/` ne leur passe que des chaînes (ou, pour `has`, une chaîne NULL) ;
- `g_ie_id` (`ie.id`) est un `pico8_value_t` statique qui ne vaut que nil ou un nombre ;
- `pico8_pf_value_vec`/`pico8_mz_value_vec` enveloppent un `pico8_vec_t *` dans une valeur pour le relire aussitôt.

Ligne de l'étape, recopiée de `docs/plan-3.md` à `a702ce4`, identique à `77c916a` (§5, « Phase D — Données », l. 284) :

> - D.15b — `valeur`, mécanique : `ij`/`jc_ce` en pointeurs, `d_` en entier, `jc_hg` en `const char *`, `g_ie_id`. Compte : ≈ 910.

Ligne de l'étude D.14 pour cette étape (§3, table des étapes), recopiée :

> | **D.15b** | pointeurs : `pico8_ij` (sortie `void **`), `pico8_jc_ce` (pointeur, `jc_value_instance` retiré), `pico8_d_` (`int`), `pico8_jc_hg_add/has` (`const char *`), `pf/mz_value_vec` retirés, `g_ie_id` en drapeau + nombre ; TU `jc`, `g_`, et les plis de `d_` (`fr`, `fv`, `hj`, `mx`, `nl`, `nr`, `os`, `qh`) | ≈ 20 fichiers | ≈ 910 |

Règles communes de l'étude (§3), recopiées :

> - **Un TU qui nomme un symbole changé le suit dans le même commit.** Un TU peut être converti plus tôt que son symbole ; jamais plus tard.
> - **Aucune empreinte figée ne change** (G1). Pendant D.15–D.18, un TU qui plie une valeur désormais typée reproduit les octets de l'ancien pli (octet de genre + contenu) à partir du type C.
> - Aucun `*.sha256` ni e2e ne change.
> - `class-coverage-net` si l'étape touche un constructeur ; `sanitize-net` si elle change une durée de vie ou un propriétaire.

Lignes (a) de l'étude qui fondent ces règles (§1.2), recopiées :

> | `pico8_ij(…, out_nearest)` — `entity_geom.c:96-118`, `.h:10,29,31` ; `galaxy.c:53`, `wormhole.c:74` | nil ou une entité | `void **out_nearest` (NULL : rien) |
> | `pico8_jc_ce` — `slipway.c:266-339`, `.h:119` ; `pico8_jc_value_instance` `slipway.c:59-60` | nil, la vue « -too far- » ou une entité (lua.lua:641-652) : toujours une référence | `pico8_struct_header_t *` ; `jc_value_instance` disparaît |
> | `pico8_d_` — `probe.c:328-364`, `.h:175` | `true` ou nil (lua.lua:569-575), lu en vérité seule | `int` |
> | `pico8_jc_hg_add/has(hg, id)` — `slipway.c:94-108`, `.h:28,31` ; `economy_tiers.c:134`, `slipway_display.c:31` | `src/` ne passe que des chaînes (`planet->id`, `t_id`) | `const char *id` ; les cas nombre et nil de `test_pico8_jc` sont des genres que `src/` ne produit pas (voir G4) |
> | `pico8_pf_value_vec`, `pico8_mz_value_vec` — `planet_family.c:27-28,373-374`, `wormhole.c:40-41,84,173` | enveloppent un `pico8_vec_t *` pour le relire | lecture directe du pointeur (NULL : vecteur nul) |
> | `g_ie_id` — `research_icon.c:24,127-129,267,297-300` | nil ou nombre (l'`assert` de `:300` le dit déjà) | `pico8_fixed_t` + `has_ie_id` |

Lignes directrices de l'étude qui s'appliquent ici (§4), recopiées : **G1** (empreintes figées : « Un pli typé écrit le même octet de genre (`0`/`1`/`2`/`3` […] ou les octets propres au TU : `0xB0`, `0xF1`, `0x57`) et le même contenu. Une empreinte qui bouge est une régression, pas une mise à jour. ») ; **G4** (« Un cas de TU qui passe un genre qu'aucun appelant de `src/` n'écrit (`pico8_oj` d'un nombre, `pico8_jc_hg_has` d'un nombre ou de nil, `pico8_jo_ex` d'un `false`) est supprimé avec le type qui le permettait ; le message de commit le nomme. »).

Notes antérieures qui visent ces sites : D.11e (« **Pour D.12** : `pico8_ij` lit `p` par `get_p` et garde `pico8_value_t *out_nearest` ») ; D.12a (« `pico8_ij` converti en place (`out_nearest` reste `pico8_value_t *`, Q12) ») ; D.12b2 (« `jc:ce` rend `pico8_value_ref(w)` (Q12) ») ; remise des comptes de D.12d (« Restes nommés : […] `pico8_jc_ce` […] »). Q12 est tranchée oui (D.14) : ces restes sont l'objet de cette étape.

### Hypothèses `H-D15a-n` (écrites pendant D.15a ; verdicts plus bas)

Source : la ligne D.15a du plan (« `valeur`, mécanique : `ov` prend des nombres, `cb` un entier. Compte : `valeur` ≈ 979. ») et la ligne D.15a de l'étude (« `pico8_ov` en nombres, `pico8_cb(int)` ; appelants `economy`, `planet`, `slipway`, `wormhole`, `menu` ; TU `fm`, `js` ; ≈ 8 fichiers »). Le PRD de D.15a n'est pas commis à `a702ce4` : rien de lui n'a été lu.

| Hypothèse | Énoncé | Ce qui en dépend ici | Vérification à la graduation |
|---|---|---|---|
| H-D15a-1 | Après D.15a, `valeur` ≈ **979** lignes (1014 − ≈ 35), `type-tag` **243 / 58**, `DATA_RESIDUE_GUARD` inchangé (`spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit`), résidu D ≈ 1220 | « État de départ », « Effet sur `make data-residue` », critère `FAILED:` de départ | `make -s data-residue` ; note « Découvert à D.15a » |
| H-D15a-2 | Dans `pico8_slipway.c`, D.15a ne touche que le bloc `ov` de `pico8_jc_ce` (`:270` `pico8_value_t ov_result;`, `:282-284`) : la locale `ov_result` disparaît, `threshold` reçoit directement `pico8_ov("space folding", …)` (une ou deux lignes) ; `threshold` reste une `pico8_fixed_t` locale ; rien d'autre du fichier ne change | numéros de `slipway.c` ≥ 270 (Inventaire, mutations M3–M6), comptes par fichier de `slipway.c` | `git diff a702ce4 <HEAD gradé> -- src/pico8_slipway.c` |
| H-D15a-3 | Dans `pico8_wormhole.c`, D.15a ne touche que `threshold_val` de `pico8_mz_hq` (`:76`, `:88-90`) et de `pico8_mz_kz` (`:103`, `:115-117`) ; `pico8_mz_truthy`, `pico8_mz_value_vec`, `nearest` (`:74`), `:84-86`, `:148`, `:173`, `pico8_mz_et` inchangés | numéros de `wormhole.c` ≥ 76, comptes de `wormhole.c`, M1, M10, M17, M18 | `git diff a702ce4 <HEAD gradé> -- src/pico8_wormhole.c` |
| H-D15a-4 | D.15a ne touche ni `entity_geom.c/.h`, ni `probe.c/.h`, ni `galaxy.c`, ni `research_icon.c`, ni `planet_family.c`, ni `economy_tiers.c`, ni `slipway_display.c`, ni `slipway.h`, ni les TU `jc`, `g_`, `fr`, `mx`, `os`, `nr`, `hj`, `nl`, `qh`, `mz`, `ie`, `pm` | numéros non marqués de l'Inventaire (tous ceux de ces fichiers) | `git diff --stat a702ce4 <HEAD gradé> -- <ces chemins>` vide |
| H-D15a-5 | D.15a ne retire ni n'ajoute aucun `#include "pico8_value.h"` (lecture de l'étude : les 76 includes vont à D.19c) ; s'il en retire, c'est dans des fichiers que D.15b ne touche pas | « Includes », grep 7 de la Sortie | `git diff a702ce4 <HEAD gradé> \| grep -E '^[-+]#include.*pico8_value\.h'` |
| H-D15a-6 | `pico8_ov` a la signature `pico8_fixed_t pico8_ov(const char *px, pico8_fixed_t lk, pico8_fixed_t mc)` et n'a pas d'autre appelant dans les fonctions que D.15b réécrit que ceux de `jc_ce` et `mz_hq` | « `jc_ce` », M3 | lecture de `tech_gate.h` |
| H-D15a-7 | D.15a ne retire aucune copie `*_truthy` (celles de `slipway.c` et `wormhole.c` ont encore `d_` pour seul appelant, voir « Surprises ») et ne touche pas `pico8_rn` (B5) | « Copies `truthy` », comptes de `slipway.c`/`wormhole.c` | lecture des deux fichiers |
| H-D15a-8 | `Makefile`, `tools/` inchangés ; aucun fichier ni TU ajouté ou retiré ; `make e2e` 26 conformes ; `tools/class_coverage_net_expected.txt` inchangé | filets, « `Makefile` et liens » | note de D.15a ; `git diff --stat` |
| H-D15a-9 | La note « Découvert à D.15a » ne modifie pas la ligne D.15b du plan et ne déplace aucun site de D.15b vers D.15a (ni l'inverse) | tout le PRD | lecture du plan gradé |
| H-D15a-10 | D.15a est entière (pas de repli laissé à moitié) à la graduation | tout | note de D.15a |

### Sections à revérifier quand D.15a sera gradée (liste d'origine, gardée pour trace ; faite, voir « Sections revérifiées »)

- **Toujours** : « État de départ » (comptes réels, H-D15a-1) ; critères `FAILED:` de départ (HEAD, `valeur` de départ) ; « Effet sur `make data-residue` » (colonne « départ » et « après », par fichier pour `slipway.c` et `wormhole.c`) ; tous les numéros de `slipway.c` ≥ 270 et de `wormhole.c` ≥ 76 dans l'Inventaire et les mutations (remesure **par contenu**) ; Annexe A (ligne D.15a et sa note, à recopier ; ligne D.15b si la note l'a modifiée).
- **Si H-D15a-2 ou -3 tombe** (D.15a a touché d'autres lignes de `jc_ce`, `mz_hq`, `mz_ey`, `mz_pr`, ou une copie `truthy`) : Inventaire de ces deux fichiers, « Copies `truthy` », M3–M6, M10, M17, M18.
- **Si H-D15a-4 tombe** : Inventaire du fichier touché, « Les TU suivent », empreinte.
- **Si H-D15a-5 tombe** : « Includes » (la règle reste : aucun include touché par D.15b), grep 7.
- **Si H-D15a-6 tombe** : « `jc_ce` » (forme de `threshold`).
- **Si H-D15a-7 tombe** (une copie `truthy` déjà retirée, ou `rn` typé) : « Copies `truthy` », comptes de `slipway.c`/`wormhole.c`, total de l'étape.
- **Si H-D15a-8 tombe** : filets, « `Makefile` et liens ».
- **Si H-D15a-9 ou -10 tombe** : ne pas mettre ce PRD en file ; le reprendre.

**Verdicts à `77c916a`** (`git archive`, `git diff a702ce4 77c916a`, `sh tools/data_residue.sh`, note « Découvert à D.15a » recopiée en Annexe A) :

| Hypothèse | Verdict |
|---|---|
| H-D15a-1 | **fausse en valeur, sans effet sur le plan** : `valeur` **980 / 103 ; 1105 occ.** (`src/` 488 / 65 / 563 occ., `tests/` 492 / 38 / 542 occ.) au lieu de ≈ 979 : D.15a garde un pont déclaré, `tech_gate.c:45` `if (pico8_oj(pico8_value_string(px))) {`, jusqu'à D.16b (`oj`) ; `type-tag` 243 / 58 ; résidu D **1221 / 111** ; `DATA_RESIDUE_GUARD` inchangé |
| H-D15a-2 | **tenue** : dans `slipway.c`, seuls `:270` (`ov_result`, retirée) et `:282-284` (trois lignes → une, `:281` `threshold = pico8_ov("space folding", …);`) ont changé ; `threshold` reste une locale `pico8_fixed_t`. Numéros de `slipway.c` ≥ 270 : **− 3** (`:274` → `:273`, `:287` → `:284`, `:310` → `:307`, `:322` → `:319`, `:338` → `:335`) ; `< 270` inchangés |
| H-D15a-3 | **tenue** : dans `wormhole.c`, seuls `threshold_val` de `mz_hq` (`:76` retirée, `:88-90` → `:87`) et de `mz_kz` (`:103` retirée, `:115-117` → `:111`). Numéros : `:74` inchangé ; `:84` → `:83`, `:85` → `:84`, `:86` → `:85` ; `:148` → `:142` ; `:173` → `:167` ; `pico8_mz_et` `:222-229` → `:216-223` |
| H-D15a-4 | **tenue** : D.15a n'a touché que `economy.c`, `menu.c`, `planet.c`, `slipway.c`, `tech_gate.c/.h`, `wormhole.c`, TU `fm`, `js` (et `docs/`) ; aucun autre fichier de l'étape, numéros inchangés |
| H-D15a-5 | **tenue à la lettre de sa clause, mais elle change une règle d'ici** : D.15a a retiré un include, `test_pico8_fm.c:35`, fichier que D.15b ne touche pas, parce que le TU ne nommait plus rien du module (son PRD : « les includes inutiles partent au fil des étapes, comme le prévoit l'étude »). Ce précédent **remplace** la lecture « aucun include touché » de la première version : voir « Includes » |
| H-D15a-6 | **tenue** : `pico8_fixed_t pico8_ov(const char *px, pico8_fixed_t lk, pico8_fixed_t mc)` (`tech_gate.h:12`) ; `jc_ce` et `mz_hq` l'appellent sur une ligne |
| H-D15a-7 | **tenue** : `pico8_jc_truthy` (`slipway.c:48`) et `pico8_mz_truthy` (`wormhole.c:30`) intactes, toujours avec `d_` pour seul appelant ; `rn` non touché. (`pico8_tech_gate_truthy` n'a plus que `oj` : hors étape) |
| H-D15a-8 | **tenue** : `Makefile`, `tools/` inchangés (`git diff --stat a702ce4 77c916a`) ; aucun fichier ni TU ajouté ; VERIFIED |
| H-D15a-9 | **tenue** : ligne D.15b du plan identique ; aucun site déplacé |
| H-D15a-10 | **tenue** : D.15a entière, un commit |

Conséquences : aucune décision de fond ni le découpage ne changent. Changent : les comptes de départ (980 / 103 / 1105) ; les numéros de `slipway.c` et `wormhole.c` ; la règle des includes (un include de plus retiré : `slipway_display.c:11`), d'où **− 69 lignes, − 75 occ.** au lieu de − 68 / − 74 ; prototypes publics qui nomment `pico8_value_t` : 25 à `77c916a` (compte du coordinateur), **20** après D.15b (les cinq de la table « API »).

### Sections revérifiées sur `77c916a`

Refaites : en-tête, verdicts, « Lecture » (includes, compte), « État de départ », Inventaire (numéros de `slipway.c`, `wormhole.c` ; ligne d'include ajoutée), « Includes », « Effet sur `make data-residue` », Sortie (greps 1 et 7), anti-tassement, mutations et cibles, critères `FAILED:`, Out of Scope, Annexe A (ligne D.15a et sa note recopiées). Inchangées et vérifiées : décisions de type, TU, empreinte (22 chemins), découpage (un commit), budget.

### Lecture de ces textes pour D.15b

- **« `ij`/`jc_ce` en pointeurs »** : l'étude propose `void **out_nearest` pour `ij` et `pico8_struct_header_t *` pour `jc_ce`. **Tranché (G)** : les deux en `pico8_struct_header_t` (`pico8_struct_header_t **out_nearest`, `pico8_struct_header_t *pico8_jc_ce`). Raison : `ij` asserte déjà que chaque élément est une struct et le lie à `pico8_struct_header_t *o` (`entity_geom.c:108-111`) ; ses trois lecteurs rangent le résultat dans un `pico8_struct_header_t *` (`mz.ij`, `wormhole.h:40`) ou le passent comme cible `et` (`probe.c:181`, `probe.h:26`). `void **` obligerait `wormhole.c:86` à convertir un `void *` en en-tête sans test. Rejeté : `void **` (écart à l'étude, déclaré).
- **« `d_` en entier »** : `int`, 1 pour le `true` du cart (`lua.lua:572`), 0 pour nil (`lua.lua:574`, et `self` NULL). Les deux lecteurs de `src/` (`slipway.c:187`, `wormhole.c:148`) lisent la vérité C directement.
- **« `jc_hg` en `const char *` »** : `pico8_jc_hg_add(pico8_jc_hg_t *, const char *id)`, `pico8_jc_hg_has(const pico8_jc_hg_t *, const char *id)`. **G4 lu en deux moitiés (G)** : le cas **nombre** meurt avec le type (`test_pico8_jc.c:909`, `:928`) ; le cas **nil ne meurt pas** : un `const char *` peut être NULL, et `src/` en produit un — `slipway_display.c:29-31` passe `t_id` NULL quand `t` est le point souris de `jc:hq` (« The mouse point jc:hq hangs on `re` has no `id` »). `has` garde donc `id == NULL → 0` (lecture Lua de `hg[nil]`) et son cas de TU (`:929`, réécrit sur `NULL`) ; `add` garde son refus d'un `id` NULL (moitié `id.as.string != NULL` de l'`assert` actuel, `lua.lua` : `t[nil]=true` lève une erreur), et le `must_abort` qui refusait un nombre refuse désormais NULL (`hg_add_fixed` → `hg_add_nil`, même garde, l'autre moitié de l'`assert`). Écart à la lettre de G4, déclaré.
- **« `g_ie_id` »** : la ligne ne donne pas la règle ; l'étude dit « `pico8_fixed_t` + `has_ie_id` ». **Tranché (G)** : `static pico8_fixed_t g_ie_id` et `static int g_ie_has_id` (préfixe `g_ie_` des statiques du module, nom hors motif), nil = `g_ie_has_id == 0` ; `pico8_ie_id(&out)` garde son contrat (0 pour nil) ; l'`assert` « only ever nil or a number » (`:300`) part avec le genre qu'il gardait.
- **`pf/mz_value_vec`** : absents de la ligne du plan, présents dans la ligne de l'étude pour D.15b, et comptés dans son « ≈ 910 ». **Tranché (G)** : dans l'étape (la ligne du plan abrège celle de l'étude, qu'elle dit reprendre : « Étapes ci-dessous, reprises de l'étude »), déclaré. Lecture au site : `p != NULL ? *p : vecteur nul`, exactement ce que faisait l'enveloppe.
- **« Compte : ≈ 910 »** : estimation de l'étude. Mesurée à `77c916a` : **70 lignes retirées** (`src/` 53 dont l'include de `slipway_display.c`, `tests/` 17), **1 gagnée** (`probe.c:181`, voir « `ij` »), soit **− 69** : `valeur` **980 → 911** (1105 → 1030 occ.). « ≈ » n'est pas un critère ; « − 69 exact, écart expliqué » l'est.
- **Empreinte** : l'étude dit ≈ 20 fichiers, dont le TU `fv`. Mesuré : **12 fichiers de `src/`, 9 TU**, plus `docs/plan-3.md` (note) — **pas `fv`** (`test_pico8_fv.c:592` est `(void)pico8_d_(e);`, qui compile tel quel avec un `int`), **pas `ie`, `mz`, `pm`** (ils lisent `pico8_ie_id`, `mz->ij`, `pm->ng`, dont le type ne change pas : ils prouvent, ils ne suivent pas).
- **Drapeaux** : la ligne ne dit rien de `class-coverage-net` ni de `sanitize-net`, l'étude non plus pour D.15b. Vérifié : aucun constructeur, aucun appel `pico8_class_coverage_record`, aucun `KNOWN[]` touché (`jc:hq`, `pm:pq`, `mz:hq`, `ie:ldown`, `bw` ne sont pas des constructeurs de classe au sens du §7 ; `pm:pq` construit un `ga` par `pico8_ga_new`, inchangé) ; aucune durée de vie ni aucun propriétaire ne change (les pointeurs rendus sont empruntés comme les REF qu'ils remplacent ; `g_ie_id` reste statique). **Tranché (G)** : ni l'un ni l'autre exigé (§7, « En D et en 2 ») ; le balayage ASan+UBSan des TU est exigé (consigne de l'utilisateur).
- **§6** : aucun des trois tests de régression (`layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `ga_export_panel_by`) ne nomme un symbole de l'étape ; ils sont recompilés et relancés (deux se lient sur `DATA_ENGINE_SRCS`), rien de plus.
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json` ; rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; interdits : jonction de lignes, alias, renommage pour échapper aux greps, cast non vérifié, cale valeur ↔ typé ; mutations restaurées par `cp` simple puis `touch` ; binaires lancés exactement comme le `Makefile` (`ASAN_OPTIONS=detect_leaks=0` pour un binaire lié sous ASan : `test_pico8_planet_gh_row_lq`, `Makefile:550`, `:655-656`) ; jamais l'appli raylib.

### État de départ

Mesuré à `a702ce4` (D.15a non commencée), puis à **`77c916a`** (D.15a gradée), sans build :

| Mesure | À `a702ce4` | Départ réel de D.15b (`77c916a`) |
|---|---|---|
| `valeur` (lignes / fichiers ; occ.) | 1014 / 105 ; 1151 occ. (`src/` 518 / 66 / 605 occ., `tests/` 496 / 39 / 546 occ.) | **980 / 103 ; 1105 occ.** (`src/` 488 / 65 / 563 occ., `tests/` 492 / 38 / 542 occ.) |
| `type-tag` | 243 / 58 (`src/` 72 / 24, `tests/` 171 / 34) | **243 / 58** |
| résidu D | 1255 / 111 | **1221 / 111** |
| prototypes publics qui nomment `pico8_value_t` | — | 25 |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit` | inchangé |

Recette des occurrences (celle de D.12, motif `valeur` de `tools/data_residue.sh`) :

```sh
P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
printf 'valeur\t%s lignes\t%s fichiers\t%s occ.\n' "$(cat $F | grep -cE "$P")" \
  "$(grep -lE "$P" $F | wc -l)" "$(cat $F | grep -oE "$P" | wc -l)"
for f in $F; do n=$(grep -cE "$P" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$P" $f | wc -l)"; done
```

### Inventaire (à `77c916a`, ligne à ligne ; mesure d'origine à `a702ce4`)

Classes du message de commit : **(a1)** même site, type C à la place de la valeur ; **(a2)** retirée sans remplaçant (enveloppe, copie `truthy`, `assert` de genre, cas de TU d'un genre mort) ; **(b)** signature dont le type change (table « API ») ; **(c)** même donnée, autre nom : **attendu 0**. `occ.` : occurrences du motif sur la ligne.

#### API dont le type change (classe (b))

| API | Aujourd'hui | Après D.15b | Appelants (`src/`) | TU |
|---|---|---|---|---|
| `pico8_ij` (`entity_geom.c:96`, `.h:31`) | `pico8_value_t *out_nearest` | `pico8_struct_header_t **out_nearest` (NULL : rien) | `galaxy.c:67` (`bw`), `wormhole.c:84` (`mz:hq`), **`probe.c:176` (`bv`)** | aucun direct (`mz`, `g_` par `hq`) |
| `pico8_jc_ce` (`slipway.c:265`, `.h:119`) | rend `pico8_value_t` | rend `pico8_struct_header_t *` (NULL : rien) | `slipway.c:167` (`jc:hq`) seul | aucun direct (`jc` par `hq`) |
| `pico8_d_` (`probe.c:328`, `.h:175`) | rend `pico8_value_t` (`bool(1)` ou `REF NULL`) | rend `int` (1 / 0) | `slipway.c:187` (`jc:ey`), `wormhole.c:142` (`mz:ey`), `probe.c:369` (`d__dispatch`, résultat ignoré) | `g_` (4 sites), plis `fr`, `mx`, `os`, `nr`, `hj`, `nl`, `qh` (×2) ; `fv:592` inchangé |
| `pico8_jc_hg_add` (`slipway.c:93`, `.h:28`) | `pico8_value_t id` | `const char *id` (NULL refusé, `assert`) | `economy_tiers.c:133` (`bd`) | `jc` |
| `pico8_jc_hg_has` (`slipway.c:104`, `.h:31`) | `pico8_value_t id` | `const char *id` (NULL : 0) | `slipway_display.c:31` (`jc:ch`), `slipway.c:95` (`add`) | `jc` |

Aucune autre signature publique ne change. Statiques retirées : `pico8_jc_value_instance`, `pico8_jc_truthy` (`slipway.c`), `pico8_mz_truthy`, `pico8_mz_value_vec` (`wormhole.c`), `pico8_pf_value_vec` (`planet_family.c`).

#### Sites de `src/` (53 lignes, 56 occ. retirées ; 1 ligne, 1 occ. gagnée)

| Fichier | Ligne(s) `valeur` (occ.) | Remplacement | Classe |
|---|---|---|---|
| `entity_geom.c` | `:96` (1) | signature `pico8_struct_header_t **out_nearest` | (b) |
| | `:101` (1), `:118` (1) | `*out_nearest = NULL;`, `*out_nearest = o;` | (a1) |
| `entity_geom.h` | `:31` (1) | prototype | (b) |
| | `:29` (1) | contrat du prototype (`:28-30`) réécrit : « `*out_nearest` is NULL -- Lua's nil -- when … » | (a1) |
| | `:10` (1) | commentaire de tête (`:9-12`) devenu faux depuis la phase 1b (« is a PICO8_VALUE_REF ») : « is a pointer to a heap vector the field borrows and never owns » ; l'étude le nomme avec son objet (§1.2, « commentaires ») | (a1) |
| `galaxy.c` | `:53` (1) | `pico8_struct_header_t *nearest;` (`:67` inchangé) | (a1) |
| `wormhole.c` | `:30`, `:31`, `:34` (3) | `pico8_mz_truthy` retirée (seul appelant `:142`, voir « Copies `truthy` ») | (a2) |
| | `:40`, `:41` (2) | `pico8_mz_value_vec` retirée | (a2) |
| | `:74` (2) | `pico8_struct_header_t *nearest = NULL;` | (a1) |
| | `:83` (1), `:167` (1) | `p_vec = self->p != NULL ? *self->p : <vecteur nul>;` (idem `self_p`) | (a1) |
| | `:85`, `:142` (0) | `self->ij = nearest;` ; `result = !pico8_d_(…);` — lignes modifiées hors motif | — |
| `probe.c` | `:157` (1), `:175` (1) | `pico8_struct_header_t *c;`, `c = NULL;` | (a1) |
| | `:328` (1) | signature `int pico8_d_(…)` | (b) |
| | `:335` (1), `:364` (1), `:359` (1) | `return 0;`, `return 0;`, `return 1;` | (a1) |
| | `:180`, `:181` (0 → **1**) | `c != NULL` au lieu de `pico8_probe_truthy(c)` ; **`et = pico8_value_ref(c);`** : ligne qui **gagne** `valeur`, `et` restant une valeur jusqu'à D.16b (B3) | gagnée, déclarée |
| `probe.h` | `:175` (1) | prototype ; contrat `:173-174` réécrit (« 1 on the success branch (Lua true), 0 on the other (nil) ») | (b) |
| `slipway.c` | `:48`, `:49`, `:52` (3) | `pico8_jc_truthy` retirée (seul appelant `:187`) | (a2) |
| | `:58`, `:59` (2) | `pico8_jc_value_instance` retirée ; `:167` devient `self->probe->et = pico8_jc_ce(self);` (hors motif) | (a2) |
| | `:93` (1), `:104` (1) | signatures `const char *id` | (b) |
| | `:94` (1) | `assert(id != NULL && "jc.hg: …")` (message gardé, reformulé sur nil) | (a1) |
| | `:107` (1) | `if (hg == NULL \|\| id == NULL)` ; `:99`, `:110` (`id.as.string` → `id`) modifiées hors motif | (a1) |
| | `:265` (1) | signature `pico8_struct_header_t *pico8_jc_ce(…)` | (b) |
| | `:273` (1), `:284` (1), `:307` (1) | `return NULL;`, `return &g_jc_too_far.header;`, `return w;` | (a1) |
| | `:319` (2), `:335` (1) | `pico8_struct_header_t *result = NULL;`, `result = ds.items[0];` | (a1) |
| | `:187` (0) | `should_fail = (re == NULL) \|\| !pico8_d_(…);` (hors motif) | — |
| `slipway.h` | `:28`, `:31`, `:119` (3) | prototypes ; contrats `:26-27`, `:30`, `:117-118` réécrits (« NULL » au lieu de « nothing », « `id` must not be NULL ») | (b) |
| `research_icon.c` | `:24` (2) | `static pico8_fixed_t g_ie_id = 0;` + ligne nouvelle `static int g_ie_has_id = 0;` | (a1) |
| | `:127` (1), `:129` (1) | `g_ie_has_id ? g_ie_id : 0` ; `g_ie_id = new_id;` + ligne nouvelle `g_ie_has_id = 1;` | (a1) |
| | `:267` (1) | `g_ie_id = pico8_ie_start.id;` + ligne nouvelle `g_ie_has_id = 1;` | (a1) |
| | `:297` (1) | `if (!g_ie_has_id) {` | (a1) |
| | `:300` (1) | `assert` de genre retiré (le type le rend vrai) | (a2) |
| `planet_family.c` | `:26`, `:27` (2) | `pico8_pf_value_vec` retirée | (a2) |
| | `:372`, `:373` (2) | lecture directe de `self->hj.qh.jo.ra` et `self->ng`, NULL → vecteur nul | (a1) |
| `economy_tiers.c` | `:133` (1) | `pico8_jc_hg_add(w->hg, planet->id);` | (a1) |
| `slipway_display.c` | `:31` (1) | `pico8_jc_hg_has(self->hg, t_id)` (`t_id` peut être NULL, garde de `has`) | (a1) |
| | `:11` (1) | `#include "pico8_value.h"` retiré : le fichier ne nomme plus rien du module (précédent de D.15a, `test_pico8_fm.c:35` ; voir « Includes ») | (a2) |

Totaux `src/` : entity_geom.c 3, entity_geom.h 3, galaxy.c 1, wormhole.c 8 (9 occ.), probe.c 6, probe.h 1, slipway.c 15 (16 occ.), slipway.h 3, research_icon.c 6 (7 occ.), planet_family.c 4, economy_tiers.c 1, slipway_display.c 2 : **53 lignes, 56 occ.** ; gagnée : `probe.c:181` (1, 1).

#### TU (17 lignes, 20 occ. retirées ; 8 lignes modifiées hors motif)

| TU | Ligne(s) `valeur` (occ.) | Remplacement | Classe |
|---|---|---|---|
| `jc` | `:909` (1) | `hg_add_fixed` → `hg_add_nil` : `pico8_jc_hg_add(hg, NULL);` (garde de l'`assert` nil, voir « Lecture ») ; `:930` suit | (a1) |
| | `:913` (1), `:923`–`:925` (3), `:935` (1) | chaînes passées directement | (a1) |
| | `:927` (2), `:937` (1) | `pico8_jc_hg_has(&hg, "p2")`… | (a1) |
| | `:928` (1) | cas nombre supprimé (G4, nommé au message) | (a2) |
| | `:929` (2) | `!pico8_jc_hg_has(&hg, NULL) && !pico8_jc_hg_has(NULL, "p1")` | (a1) |
| | `:915-916` (0) | commentaire du test : « refuses a nil `id` » au lieu de « a non-string `id` » | — |
| `g_` | `:585` (1) | `assert(pico8_d_(&a->ke.header) == 0);` | (a1) |
| | `:598`, `:618`, `:641` (3) | `int out;` | (a1) |
| | `:605` (1), `:654` (1), `:624` (2) | `assert(out == 0);`, `assert(out == 0);`, `assert(out == 1);` | (a1) |
| `fr` `:487`, `mx` `:521`, `os` `:521`, `nr` `:473`, `hj` `:531`, `nl` `:457`, `qh` `:451`, `:463` | 0 (`fold_value(h, pico8_d_(…))`, hors motif) | pli typé aux mêmes octets, au site : `h = pico8_d_(…) ? fnv(fnv(h, 0xB0u), 1u) : fold_ref(h, NULL);` — c'est exactement ce que `fold_value` pliait pour `bool(1)` (`0xB0`, 1) et pour `REF NULL` (`fold_ref` → `0xDEAD`) dans ces sept TU (vérifié : mêmes corps à `a702ce4`) ; `pico8_d_` appelé une fois | — |

Totaux `tests/` : `jc` 10 lignes (12 occ.), `g_` 7 (8 occ.) : **17 lignes, 20 occ.** Aucune ligne de TU ne gagne `valeur`.

**Pourquoi pas `fold_value(h, pico8_value_bool(1))`** : 8 lignes gagneraient `valeur` pour les rendre à D.19a ; le pli direct écrit les mêmes octets sans passer par la valeur (G1 : « un pli typé écrit le même octet de genre »). Rejeté aussi : une aide `fold_d_` par TU (8 aides pour 9 appels).

#### Copies `truthy` (surprise, déclarée)

`pico8_jc_truthy` (`slipway.c:48-55`) et `pico8_mz_truthy` (`wormhole.c:30-37`) n'ont qu'un appelant chacune, le résultat de `pico8_d_` (`slipway.c:187`, `wormhole.c:148`). Avec `d_` en `int`, elles deviennent des `static` inutilisées, fatales sous `-Werror` : elles partent ici (6 lignes classées B12 par l'étude, prévues en D.16). `pico8_probe_truthy` **reste** : `probe.c:196` la garde pour `first` (B3, D.16b) ; seul `:180` la quitte (`c != NULL`).

#### Sites nommés par l'étude et laissés

- `entity_geom.h:6`, `slipway.h:10`, `probe.h:11`, `galaxy.c:12`, `slipway.c:21`, `research_icon.h:10`, `wormhole.h:12` (includes) : **gardés** (`slipway_display.c:11` part) (voir « Includes »).
- `geom.h:37-38` (le `qz` rangé « as a PICO8_VALUE_REF ») : hors étape (objet : `qz`, pas un site D.15b).
- `probe.c:186-203`, `:208-214` (`et`, `first`), `wormhole.c:216-223` (`pico8_mz_et`) : B3, D.16b. `probe.c:357`, `:362`, `slipway.c:125`, `:194`, `wormhole.c:64` (`rn`) : B5, D.16b. `galaxy.c:79`, `probe.c:273`, `research_icon.c:136` (`tostr`) : D.15c. `research_icon.c:91` (`oj`) : B4, D.16b. `planet_family.c:37-44` (`qh_qh`) : B1, D.16a. `slipway.c:281`, `wormhole.c:87`, `:111` (`ov`, déjà typés par D.15a, hors motif) ; `tech_gate.c:45` (pont `oj` de D.15a) : D.16b.

#### `Makefile` et liens

- Aucun fichier nouveau ; **aucune ligne du `Makefile` ne change**.
- Règle de lien : aucun fichier de `DATA_ENGINE_SRCS` (`Makefile:229-279`) ne nomme un symbole défini hors de cette liste. D.15b n'ajoute aucun symbole ; `pico8_ij`, `pico8_jc_ce`, `pico8_jc_hg_*`, `pico8_d_` restent définis dans `entity_geom.c`, `slipway.c`, `probe.c` (dans la liste) ; `galaxy.c` et `research_icon.c` (hors liste) appellent la liste, ce qui est permis. Les binaires liés sur `DATA_ENGINE_SRCS` seule (`data_init`, `map_tables`, `jb_ol`, `planet_kj_dispatch`, `bg_kj`), sous ASan (`planet_gh_row_lq`, `Makefile:550`) ou avec `planet_family.c` en plus (`ga_export_panel_by`, `:553`) doivent se lier (grep 5).

## Solution

**Un commit** (≈ 22 chemins : 12 de `src/`, 9 TU, `docs/plan-3.md`), vert seul, vérifié et poussé :

1. `pico8_ij` rend son voisin dans un `pico8_struct_header_t **` ; `bw`, `mz:hq` et `bv` passent l'adresse d'un `pico8_struct_header_t *` ; `bv` teste `c != NULL` et range `c` dans `et` enveloppé (seule ligne gagnée, rendue par D.16b).
2. `pico8_jc_ce` rend un `pico8_struct_header_t *` ; `jc:hq` le range directement dans `et` ; `pico8_jc_value_instance` disparaît.
3. `pico8_d_` rend `int` ; `jc:ey` et `mz:ey` lisent la vérité C ; `pico8_jc_truthy` et `pico8_mz_truthy` disparaissent.
4. `pico8_jc_hg_add/has` prennent `const char *` ; nil refusé par `add`, lu absent par `has`.
5. `ie.id` devient un nombre + un drapeau statiques.
6. `pm:pq`, `mz:hq`, `mz:pr` lisent leurs `pico8_vec_t *` directement ; les deux enveloppes disparaissent.
7. Les TU `jc`, `g_` et les sept plis de `d_` suivent ; aucune constante ne change.

**Sortie de l'étape**, constatée par commande sur le commit :

- `make test` vert (dont `make data-guard`, inchangé) ; `make e2e` : **26 conformes** ; balayage ASan+UBSan des TU propre ;
- `make data-residue` : `valeur` **980 → 911** lignes (**1105 → 1030** occ.), fichiers **103 → 101** (`entity_geom.c` sort de `valeur`, pas du résidu D ; `slipway_display.c` sort des deux) ; `type-tag` 243 / 58 inchangé ; résidu D **1221 / 111 → 1152 / 110** ;
- aucun `*.sha256`, aucun fichier de `tests/e2e/`, rien dans `tools/` (`tools/float_guard.txt` réécrit à l'identique par `make test`), rien dans le `Makefile` ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
  # 1. entity_geom : plus rien hors l'include gardé de l'en-tête (une ligne : src/pico8_entity_geom.h:6)
  grep -nE "$P" src/pico8_entity_geom.c src/pico8_entity_geom.h
  # 2. les prototypes typés : 5 lignes, aucune ne nomme pico8_value_t
  grep -nE 'pico8_ij\(|pico8_jc_ce\(|pico8_d_\(|pico8_jc_hg_(add|has)\(' src/*.h
  # 3. enveloppes et copies retirées, nulle part : vide
  grep -rnE 'pico8_(jc|mz)_truthy|pico8_jc_value_instance|pico8_(pf|mz)_value_vec' src tests
  # 4. g_ie_id sans valeur : vide ; le drapeau existe
  grep -nE 'g_ie_id.*(pico8_value|PICO8_VALUE)' src/pico8_research_icon.c
  grep -c 'g_ie_has_id' src/pico8_research_icon.c        # >= 4
  # 5. règle de lien
  make tmp/test_pico8_data_init tmp/test_pico8_map_tables tmp/test_pico8_jb_ol \
       tmp/test_pico8_planet_kj_dispatch tmp/test_pico8_bg_kj tmp/test_pico8_planet_gh_row_lq \
       tmp/test_pico8_ga_export_panel_by
  # 6. aucune constante de TU changée : vide
  git diff <parent> -- tests | grep -E '^[-+].*(_HASH|identity\[)' | grep -vE '^[-+]\s*(/\*|\*)'
  # 7. un seul include de pico8_value.h touché : exactement la ligne '-#include "pico8_value.h"' de src/pico8_slipway_display.c
  git diff <parent> | grep -E '^[-+]\s*#include "(\.\./src/)?pico8_value\.h"'
  grep -c 'pico8_value' src/pico8_slipway_display.c                      # 0
  # 8. plus de pli de d_ par la valeur : vide
  grep -nE 'fold_value\(h, pico8_d_' tests/*.c
  # 9. comptes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^valeur'
  ```

## User Stories

1. En tant que mainteneur du port, je veux que `pico8_ij` rende son plus proche voisin dans un pointeur d'en-tête, NULL pour rien, afin que ses trois appelants n'aient plus à déballer une référence.
2. En tant que mainteneur du port, je veux que `pico8_jc_ce` rende un pointeur d'en-tête (le slipway à reprendre, l'obstacle, la vue « -too far- » ou NULL), afin que `jc:hq` range sa cible sans `pico8_jc_value_instance`.
3. En tant que mainteneur du port, je veux que `pico8_d_` rende 1 ou 0, afin que `jc:ey` et `mz:ey` lisent une vérité C et que les deux copies `truthy` qui n'existaient que pour lui disparaissent.
4. En tant que mainteneur du port, je veux que l'ensemble `hg` d'un slipway prenne et cherche des chaînes, afin que son type dise ce que `src/` y range.
5. En tant que mainteneur du port, je veux que `hg` refuse toujours un `id` nil et lise toujours un `id` nil comme absent, afin que le point souris sans `id` de `jc:ch` reste sans effet et que `bd` ne range jamais de NULL.
6. En tant que mainteneur du port, je veux que le compteur `ie.id` soit un nombre avec un drapeau de présence, afin que « nil avant la première initialisation » reste lisible sans valeur.
7. En tant que mainteneur du port, je veux que `pm:pq`, `mz:hq` et `mz:pr` lisent leurs vecteurs directement, NULL donnant le vecteur nul, afin qu'aucune enveloppe ne les emballe pour les relire.
8. En tant que mainteneur du port, je veux que la seule ligne qui gagne `valeur` (`et` de `bv`) soit nommée avec l'étape qui la retire (D.16b), afin qu'aucune dette ne soit cachée.
9. En tant que gardien des tests, je veux que les TU `jc` et `g_` lisent les nouveaux types dans le commit qui les introduit, afin qu'ils testent le vrai état.
10. En tant que gardien des tests, je veux que les sept TU qui plient le résultat de `d_` le plient aux mêmes octets sans passer par la valeur, afin qu'aucune constante ne change et qu'aucune ligne ne gagne `valeur`.
11. En tant que gardien des tests, je veux que le cas « nombre » de `hg` soit supprimé avec le type qui le permettait, et que le cas « nil » soit gardé parce que le type le permet encore et que `src/` le produit, afin que G4 soit appliqué sur sa raison et non sur sa lettre.
12. En tant que gardien des filets, je veux `make test`, `make e2e` et le balayage ASan+UBSan des TU verts, afin de prouver qu'aucun ordre, aucun pixel, aucune cible `et` ni aucune fuite ne change.
13. En tant que gardien des tests, je veux `gcov` avant/après et des mutations ciblées restaurées par `cp`, afin que chaque ligne modifiée soit exécutée et qu'aucune ligne survivante ne cesse de l'être.
14. En tant que gardien du compteur, je veux au message du commit les comptes par catégorie et par fichier en lignes et en occurrences, la classification (a1)/(a2)/(b)/(c), les lignes qui gagnent une catégorie (« aucune » compris) et chaque Δlignes > Δocc justifié, avec les numéros du parent, afin qu'aucune baisse ne soit un tassement.
15. En tant que gardien du compteur, je veux qu'aucune ligne ne soit jointe, qu'aucun alias, renommage pour les greps, cast non vérifié ni cale valeur ↔ typé ne soit ajouté, et qu'aucun include ne soit touché hors celui du seul `.c` que l'étape vide, afin que « − 69 » veuille dire « retiré ».
16. En tant que `plan-pilot`, je veux une étape d'environ 45 minutes, un repli écrit, et une note « Découvert à D.15b » qui dise les comptes réels, les lectures de la ligne et ce que D.15c et D.16 trouvent, afin que la suite parte d'un état exact.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)** ; les rejets sont notés à chaque décision. Q12 (oui) et Q13 (A) sont des décisions humaines ; l'étude D.14 (G1–G7) fait foi sauf écart déclaré ici.

### Modules

- **Géométrie des entités** : `pico8_entity_geom.c/.h` (`ij`).
- **Sonde** : `pico8_probe.c/.h` (`d_`, `bv` : locale `c`).
- **Slipway** : `pico8_slipway.c/.h` (`ce`, `hg`, `hq`, `ey`), `pico8_slipway_display.c` (`ch`), `pico8_economy_tiers.c` (`bd`).
- **Trou de ver** : `pico8_wormhole.c` (`mz:hq`, `mz:ey`, `mz:pr`).
- **Galaxie** : `pico8_galaxy.c` (`bw`). **Protoétoile** : `pico8_planet_family.c` (`pm:pq`). **Icône de recherche** : `pico8_research_icon.c` (`ie.id`).
- **TU** : `jc`, `g_`, `fr`, `mx`, `os`, `nr`, `hj`, `nl`, `qh` (9). **Docs** : `plan-3.md` (note).
- Inchangés : `Makefile`, `tools/`, `pico8_value.c/.h`, `pico8_type_tag.h`, tout en-tête hors `entity_geom.h`, `probe.h`, `slipway.h` ; les TU `fv`, `ie`, `mz`, `pm` et les trois §6 (recompilés, relancés).

### `ij` (G)

- Signature : `pico8_fixed_t pico8_ij(pico8_vec_t p, const pico8_ptrs_t *j_, pico8_struct_header_t **out_nearest)`. Corps inchangé hors les deux affectations (`NULL`, `o`) ; l'`assert` d'étiquette (`:110-111`, `type-tag`) reste mot pour mot (D.20).
- `bw` (`galaxy.c:53`) : `pico8_struct_header_t *nearest;`, jamais lu, comme aujourd'hui.
- `mz:hq` (`wormhole.c:74-86`) : `pico8_struct_header_t *nearest = NULL;` … `self->ij = nearest;` (même type, aucune conversion).
- `bv` (`probe.c:157`, `:175-181`) : `pico8_struct_header_t *c;`, `c = NULL;` (gardé, comme le `local nq,c` du cart ; `ij` l'écrit de toute façon), `if (nq < probe->dw && c != NULL)`, `et = pico8_value_ref(c);`. **Surprise** : l'étude ne liste pas `probe.c:176` parmi les appelants d'`ij` (elle range `c` dans B3). Le changement de sortie l'impose ici ; `et`, `first` et `pico8_probe_truthy` restent à D.16b. Rejeté : avancer B3 (`et` pointeur, `has_et` retiré) dans D.15b (hors étape, constructeur `jc` et TU `hj`/`os`/`qh`/`nl`/`g_` qui plient `has_et`).

### `jc_ce` (G)

- Rend `pico8_struct_header_t *` : `NULL` (`self` NULL ; rien sur le chemin), `&g_jc_too_far.header` (premier membre, pas de cast), `w` (déjà un en-tête), `ds.items[0]` (élément de `cg("np")`/`cg("jc")`, conversion implicite de `void *` vers l'en-tête générique, **comme aujourd'hui** par `pico8_jc_value_instance` ; son test reste l'`assert` d'étiquette de `jc:hq`, `slipway.c:169-170`, inchangé).
- `jc:hq` : `self->probe->et = pico8_jc_ce(self);` ; `has_et` et l'`assert` suivants inchangés.
- `threshold` : forme laissée par D.15a (H-D15a-2/-6), non retouchée.

### `d_` (G)

- `int pico8_d_(pico8_struct_header_t *self)` : `return 0;` (`self` NULL, branche `rn(57)`), `return 1;` (branche `ks`). Le contrat de `probe.h:173-174` est réécrit (« 1 on the success branch (Lua true), 0 on the other (nil) ; `jc:ey` and `mz:ey` consume it »).
- `jc:ey` : `should_fail = (re == NULL) || !pico8_d_(…);` — le commentaire de court-circuit (`:185-186`) reste vrai. `mz:ey` : `result = !pico8_d_(…);` — son commentaire (`:143-144`) reste vrai.
- `pico8_d__dispatch` inchangé (`(void)`).

### `hg` (G)

- `void pico8_jc_hg_add(pico8_jc_hg_t *hg, const char *id)` : `assert(id != NULL && "jc.hg: a nil id does not fit the member's type -- stop, do not widen the member")` ; dédoublonnage par `has`, `assert` de capacité et rangement (`hg->ids[hg->count] = id`) inchangés.
- `int pico8_jc_hg_has(const pico8_jc_hg_t *hg, const char *id)` : `hg == NULL || id == NULL` → 0 ; `strcmp` sur `id`.
- Contrats de `slipway.h:26-31` réécrits (« `id` must not be NULL » ; « 1 for an `id` already added, 0 otherwise, NULL included »).
- Appelants : `economy_tiers.c:133` passe `planet->id` ; `slipway_display.c:31` passe `t_id` (NULL pour le point souris, lu absent comme aujourd'hui).

### `ie.id` (G)

- `static pico8_fixed_t g_ie_id = 0;` et `static int g_ie_has_id = 0;` (0 : nil, l'état avant la première `pico8_research_icon_init()`, comme la `REF NULL` d'aujourd'hui). `_init` pose les deux ; `_shutdown` ne touche ni l'un ni l'autre (comme aujourd'hui). `ldown` : `pico8_fixed_add(g_ie_has_id ? g_ie_id : 0, 1)`, puis les deux affectations. `pico8_ie_id` : `if (!g_ie_has_id) return 0; *out = g_ie_id; return 1;`.
- Le commentaire de tête du module (`:19-22`) reste vrai. Glossaire (`id (sur ie)`, « lu par `pico8_ie_id(&out)` (0 = nil) ») : reste vrai, inchangé.

### Lectures de vecteurs (G)

- `pm:pq` (`planet_family.c:372-373`) : `pico8_vec_add(ra != NULL ? *ra : <nul>, ng != NULL ? *ng : <nul>)` avec les deux membres lus une fois (`self->hj.qh.jo.ra`, `self->ng`) ; le `(void *)` de qualificatif de `:373` disparaît avec l'enveloppe (aucun cast nouveau).
- `mz:hq`, `mz:pr` : `self->p != NULL ? *self->p : <nul>`. `<nul>` = `pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0))`, l'expression même des enveloppes.
- Rejeté : une aide partagée « pointeur ou nul » (elle existe en statique dans `entity_geom.c` et `slipway.c` ; en publier une serait une API nouvelle hors étape) ; renommer les enveloppes en gardant leur corps (renommage, interdit).

### Includes (G)

**Règle (G, révisée après D.15a)** : un fichier **`.c`** qui ne nomme plus rien du module après l'étape perd son `#include "pico8_value.h"` dans ce commit — précédent de D.15a (`test_pico8_fm.c:35`, « les includes inutiles partent au fil des étapes, comme le prévoit l'étude »). Un **en-tête** le garde, même vidé, jusqu'à D.19c : il passe le type à des dépendants dont une vingtaine de `.c` nomment `pico8_value_*` sans l'inclure directement (`probe.c`, `wormhole.c`, `research_icon.c`, `planet_family.c`, `economy_tiers.c`… ; mesuré par grep à `77c916a`), casse invisible sans build. Appliqué : **`slipway_display.c:11` retiré** (seul `.c` que l'étape vide : ses lignes `:11` et `:31` étaient les seules) ; **gardés** : `entity_geom.h:6`, `probe.h:11`, `slipway.h:10` (vidés, pour D.19c), et tous les autres. Aucun include ajouté ; si la compilation en exige un (dépendant de `slipway_display.c`, aucun attendu : c'est un `.c`), il est ajouté et déclaré avec l'erreur du compilateur. `slipway_display.c` sort alors de `valeur` **et du résidu D** (aucune ligne `type-tag`). Rejeté : garder l'include (contredit le précédent de D.15a) ; vider aussi les en-têtes (risque de transitivité).

### Les TU suivent (G, règle commune de l'étude)

- **Tranché** : chaque TU qui nomme un symbole changé suit dans ce commit ; aucun pont.
- `test_pico8_jc` : les dix lignes de l'Inventaire ; `hg_add_fixed` devient `hg_add_nil` (même `must_abort`, l'`assert` nil) ; `:928` supprimée (G4) ; commentaire `:915-916` réécrit.
- `test_pico8_g_` : les sept lignes ; `out` en `int` ; mêmes cas, mêmes attentes (nil → 0, vrai → 1).
- Plis de `d_` (`fr`, `mx`, `os`, `nr`, `hj`, `nl`, `qh` ×2) : pli direct aux mêmes octets (Inventaire). `fold_value` reste (ses autres appels) ; aucune aide nouvelle.
- **Constantes** : aucune `*_HASH`/`identity[]` ne change. Une constante qui rougit est un bug de conversion.
- **Assertion nouvelle** : admise seulement pour couvrir une ligne modifiée non exécutée ou pour faire rougir une mutation qui ne rougit rien (trou), dans un test du module, listée au message avec la ligne qu'elle couvre ; rien de lent, aucun fichier d'événements.

### Anti-tassement et message de commit (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer la valeur est un échec.

- **Interdit** : joindre sur une ligne ce qui était sur plusieurs ; renommer un identifiant, un paramètre ou un commentaire compté dont la donnée existe toujours pour échapper au motif ; alias (macro ou `typedef` qui renomme `pico8_value_t` ou une de ses fonctions, enveloppe qui renomme une aide) ; cast d'un `void *` vers une struct précise sans son test ; cast de pointeur de fonction ; cast nouveau autre que les conversions implicites déjà présentes aux lignes touchées ; **cale valeur ↔ typé** (fonction qui convertit une valeur en pointeur ou en `int` et l'inverse, double tenu en parallèle, `pico8_d_value` ou `pico8_jc_ce_value` gardé « pour les TU ») ; déplacer du code compté hors de `src`/`tests` ou hors `.c`/`.h` ; tout changement d'une constante de hachage ; toute modification de `tests/e2e/`, d'un `*.sha256`, de `tools/`, du `Makefile`, d'un `#include "pico8_value.h"` autre que `slipway_display.c:11`.
- **Classification exigée** au message, pour chaque ligne `valeur` retirée ou modifiée (recette : `git diff -U0 <parent>` par fichier, lignes `-` qui portent le motif, en regard des lignes `+`) : **(a1)**, **(a2)**, **(b)** (les cinq API de la table, et elles seules), **(c)** = **0**. Attendu : (a1) 45, (a2) 15, (b) 10, (c) 0 (à `77c916a` ; recompté au parent).
- **Mesure exigée**, avant et après, par catégorie (`valeur`, `type-tag` ; les huit gardées à 0 constatées) et **par fichier** du diff : lignes (`--detail`) **et** occurrences (recette).
- **Lignes qui gagnent une catégorie**, par fichier et par catégorie, « aucune » compris. Attendu : `valeur` : `probe.c` (`et = pico8_value_ref(c);`, rendue par D.16b) ; `type-tag` : aucune ; toute autre catégorie : aucune. Toute autre est à justifier.
- **Lignes comptées modifiées, non retirées** (fichier:ligne) : attendu **aucune** hors la ligne gagnée (les lignes `valeur` de l'Inventaire quittent toutes le motif).
- **Δlignes > Δoccurrences** : chaque couple justifié (attendu : aucun ; Δocc ≥ Δlignes dans chaque fichier).
- **Numéros de ligne** : ceux du **parent du commit**, jamais ceux de ce PRD sans remesure.

### Empreinte et découpage (G)

| Commit | `src/` | `tests/` | autres | Chemins | Coût estimé |
|---|---|---|---|---|---|
| **D.15b** | `entity_geom.c`, `entity_geom.h`, `galaxy.c`, `wormhole.c`, `probe.c`, `probe.h`, `slipway.c`, `slipway.h`, `research_icon.c`, `planet_family.c`, `economy_tiers.c`, `slipway_display.c` (12) | `jc`, `g_`, `fr`, `mx`, `os`, `nr`, `hj`, `nl`, `qh` (9) | `plan-3.md` | 22 | ≈ 45 min |

- **Pourquoi un seul commit** : 22 chemins (< ~40), changements mécaniques ; rythme mesuré : D.12d, 27 chemins, estimé ≈ 40 min, plafond 65 min ; D.11d1..d4 ≈ 21 à 27 min pour 9 à 12 TU retypés. Estimation : éditions ≈ 12 min ; `gcov` départ/arrivée ≈ 8 min ; 20 mutations ≈ 10 min ; filets et balayage ≈ 8 min ; mesures, classification, message, note ≈ 8 min. **Plafond : 70 min.**
- **Repli écrit** (G) : si le plafond est atteint sans commit, rien n'est commis et `plan-pilot` relance en deux commits, même PRD : **b1** — `ij`, `jc_ce`, `d_` (et les deux copies `truthy`) ; TU `g_` et les sept plis ; **b2** — `hg`, `ie.id`, `pf/mz_value_vec` ; TU `jc`. Coût : `slipway.c`, `wormhole.c` édités deux fois, sur des lignes disjointes. Rejeté comme plan principal : deux coûts fixes pour 22 chemins.

### Effet sur `make data-residue`

| Catégorie | À `a702ce4` | Départ (après D.15a) | Après D.15b | Pourquoi |
|---|---|---|---|---|
| `valeur` | 1014 / 105 ; 1151 occ. | **980 / 103 ; 1105 occ.** | **911 / 101 ; 1030 occ.** (− 69, − 75) | Inventaire : `src/` − 53 + 1, `tests/` − 17 |
| `valeur` `src/` | 518 / 66 ; 605 occ. | 488 / 65 ; 563 occ. | **436 / 63 ; 508 occ.** | — |
| `valeur` `tests/` | 496 / 39 ; 546 occ. | 492 / 38 ; 542 occ. | **475 / 38 ; 522 occ.** | — |
| `type-tag` | 243 / 58 | 243 / 58 | **inchangé** | aucune ligne `type-tag` touchée |
| résidu D | 1255 / 111 | 1221 / 111 | **1152 / 110** | `entity_geom.c` garde ses lignes `type-tag` ; `slipway_display.c` n'en a pas |
| `DATA_RESIDUE_GUARD` | 8 catégories | inchangé | inchangé | `valeur` ≠ 0 |

Par fichier (lignes / occ., à `77c916a`, avant → après D.15b) : `entity_geom.c` 3/3 → 0/0 ; `entity_geom.h` 4/4 → 1/1 ; `galaxy.c` 3/4 → 2/3 ; `wormhole.c` 13/14 → 5/5 ; `probe.c` 21/23 → 16/18 ; `probe.h` 2/2 → 1/1 ; `slipway.c` 18/19 → 3/3 ; `slipway.h` 4/4 → 1/1 ; `research_icon.c` 8/11 → 2/4 ; `planet_family.c` 8/8 → 4/4 ; `economy_tiers.c` 7/9 → 6/8 ; `slipway_display.c` 2/2 → 0/0 ; `test_pico8_jc` 36/38 → 26/26 ; `test_pico8_g_` 23/24 → 16/16 ; les sept TU de plis : inchangés.

« − 69 », « − 75 occ. », « 911 / 1030 », « inchangé » sont des critères (écart = explication chiffrée au message).

### Documentation (G)

- `docs/plan-3.md` : note « Découvert à D.15b » en fin de ligne D.15b, rien d'autre. Elle dit : le commit ; les comptes réels ; les lectures de la ligne (en-tête pour `ij`/`jc_ce` au lieu de `void **` ; G4 lu en deux moitiés, nil gardé ; `pf/mz_value_vec` pris ; `fv` hors empreinte) ; la ligne gagnée `probe.c` (`et`) et l'étape qui la retire ; les deux copies `truthy` parties ici ; l'include retiré (`slipway_display.c`) et les en-têtes vidés gardés pour D.19c (`entity_geom.h`, `probe.h`, `slipway.h`) ; **pour D.15c** : les sites `tostr` des fichiers touchés, numéros au commit ; **pour D.16** : les copies `truthy` restantes (11 au lieu de 13), `probe_truthy` n'a plus que `first`, `bv` a `c` en pointeur.
- `docs/glossaire-noms.md` : aucune entrée ne nomme un type changé (`ij`, `hg`, `jc:ce`, `id` sur `ie` restent vrais) ; inchangé sauf si l'implémenteur trouve une phrase devenue fausse (listée).
- `docs/trous-de-portage.md` : inchangé ; TROU-15 (`bd` sur un `hg` absent) reste vrai.
- Commentaires : quatre sortes de `CLAUDE.md` ; contrats réécrits : `entity_geom.h:9-12`, `:28-30`, `probe.h:173-174`, `slipway.h:26-31`, `:117-118`, commentaire du test `jc:915-916` ; aucun renvoi à D.15b ni au plan dans `src/` ou `tests/`.

### Ce qui ne change pas

- `tests/e2e/`, tout `*.sha256`, `tools/`, le `Makefile`, tout `#include "pico8_value.h"` hors `slipway_display.c:11` ; aucun e2e ni TU ajouté ou supprimé.
- Toute constante `*_HASH`/`identity[]` de TU.
- Le comportement : mêmes voisins, mêmes cibles `et` (planète, slipway, « -too far- », vue statique, faux), même `ie.id`, mêmes `hg`, mêmes vecteurs, mêmes durées de vie (tout reste emprunté ; TROU-7 inchangé), `make e2e` conforme au bit près.
- `et` de la sonde, `pico8_mz_et`, `pico8_probe_truthy`, `rn`, `tostr`, `oj`, `qh_qh` (D.15c, D.16).
- Les `assert` d'étiquette (`type-tag`) des fonctions touchées, mot pour mot.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : `-Wall -Wextra -Werror` rend fatales `pico8_jc_truthy`, `pico8_mz_truthy` une fois `d_` typé, et `pico8_jc_value_instance`, `pico8_pf_value_vec`, `pico8_mz_value_vec` une fois leurs appels retirés : d'où leur retrait dans le même commit.
- **H2** : aucun autre appelant de `pico8_ij`, `pico8_jc_ce`, `pico8_d_`, `pico8_jc_hg_*` n'existe hors ceux de l'Inventaire (grep à `a702ce4` et `77c916a`, `src/` et `tests/`) ; un oublié se révèle à la compilation et suit (listé).
- **H3** : les sept `fold_value` touchés plient `BOOL` en `fnv(fnv(h, 0xB0u), b)` et `REF NULL` en `fnv(h, 0xDEADu)` (vérifié par lecture à `a702ce4` ; TU non touchés par D.15a) ; le pli direct ne change donc aucune constante.
- **H4** : `ie.id` n'est jamais lu à nil par un TU ou un e2e après la première `_init` : la branche nil de `pico8_ie_id` (`:298`) n'est exécutée par personne, avant comme après (justifiée au rapport `gcov`, pas dans `L`).
- **H5** : `jc_ce` à `self` NULL (`:273`) n'est jamais atteint (`jc:hq` garde `self`) : ligne non exécutée avant comme après.

## Implementation Chunks

Un seul commit. Ordre de travail :

1. Mesure de départ (`77c916a` : `valeur` 980 / 103 / 1105 occ., `type-tag` 243 / 58) ; `gcov` de départ.
2. `entity_geom.c/.h` (`ij`), puis `galaxy.c`, `wormhole.c` (`mz:hq`), `probe.c` (`bv`).
3. `probe.c/.h` (`d_`), `slipway.c` (`jc:ey`, `pico8_jc_truthy`), `wormhole.c` (`mz:ey`, `pico8_mz_truthy`) ; TU `g_` et les sept plis.
4. `slipway.c/.h` (`jc_ce`, `pico8_jc_value_instance`, `jc:hq`).
5. `slipway.c/.h` (`hg`), `economy_tiers.c`, `slipway_display.c` (appel, include) ; TU `jc`.
6. `research_icon.c` (`ie.id`) ; `planet_family.c`, `wormhole.c` (`pf/mz_value_vec`).
7. `make test`, `make e2e`, balayage ASan+UBSan ; `gcov` d'arrivée ; mutations M1–M20 ; mesure, classification ; note « Découvert à D.15b » ; **commit**.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (voisin trouvé, cible `et`, vérité de `d_`, contenu de `hg`, valeur d'`ie.id`, position calculée), pas la présence d'une ligne. D.15b change sept représentations sans changer un résultat. La preuve : les TU existants, qui lisent ces états et plient leurs effets dans des constantes inchangées ; les 26 e2e au bit près ; `gcov` et mutations ; le balayage ASan+UBSan pour les pointeurs.

### TU touchés (G)

1. `jc` (`hg`), `g_` (`d_`), `fr`, `mx`, `os`, `nr`, `hj`, `nl`, `qh` (pli de `d_`).
2. Prouvent sans suivre : `mz` (`mz->ij`, `:455-456` ; `mz:ey`), `ie` (`pico8_ie_id`, `:98-105`, `:419-428`), `pm` (`ra + ng`, `:292-302`), `jc` (cibles de `jc_ce` : `ET_QS` `:572`, `:695`, `ET_JC` `:579`, `:709`), `g_` (`et_kind`, `:423-441`).
3. **Constantes** : aucune `*_HASH`/`identity[]` ne change.
4. **Prior art** : TU suiveurs dans le même commit (D.10b, D.11c, D.12a–D.12c) ; plis typés aux mêmes octets (`fold_value(h, pico8_value_bool(gv(...).stop))` de D.10–D.11, constantes figées d'étiquette de D.11d3/D.12b) ; `must_abort` de `test_pico8_jc`.

### Preuve des tests (§7 : `gcov` et mutations)

**`gcov`** : dans une copie jetable du scratchpad, tous les TU compilés en `--coverage` (flags du `Makefile` + `GCOV_CFLAGS`), lancés comme le `Makefile` (`ASAN_OPTIONS=detect_leaks=0` pour `test_pico8_planet_gh_row_lq`) ; union des lignes de `src/` exécutées, départ et arrivée.

- Chaque ligne **ajoutée ou modifiée** de `src/` est exécutée par l'union des TU, sinon par les e2e (`make line-coverage EVENTS="tests/e2e/*.json"` → `tmp/line_coverage.txt`), sinon couverte par une assertion ajoutée, sinon justifiée au message (déclarations ; H4 ; H5). Attendus par TU : `ij` (tous les TU qui démarrent une partie : `bw`), `mz:hq`/`mz:pr`/`mz:ey` (`mz`), `bv`/`d_` (`g_`, plis), `jc_ce`/`jc:hq`/`jc:ey`/`hg` (`jc`), `ie.id` (`ie`), `pm:pq` (`pm`) ; `economy_tiers.c:133` (`bd`) et `slipway_display.c:31` (`jc:ch`) : par les TU qui lancent `bd`/`kj`, sinon les e2e.
- **`L`** = lignes **survivantes** de `src/` exécutées au départ et plus à l'arrivée (appariées par contenu et fonction) : **attendu vide**. Partantes, hors `L` : corps de `pico8_jc_truthy`, `pico8_mz_truthy`, `pico8_jc_value_instance`, `pico8_pf_value_vec`, `pico8_mz_value_vec`, l'`assert` `research_icon.c:300`. Une ligne de `L` est un comportement perdu : arrêt, rapporté.

**Mutations**, une à la fois, dans l'arbre de travail : fichier sauvegardé par `cp` avant, restauré par `cp` simple puis `touch` après (jamais `git checkout`/`stash`) ; chaque TU lancé exactement comme le `Makefile` le lance ; chacune doit rougir au moins le TU nommé, sinon c'est un trou, couvert par une assertion ou justifié (mutation équivalente, démontrée). **Astuces** : retirer un appel peut échouer sous `-Werror` (fonction `static` ou variable inutilisée) : neutraliser par `0 ? f(…) : NULL`, `if (0)` ou une condition toujours fausse plutôt que supprimer ; si un `assert` de `src/` attrape la mutation avant le TU (abandon), relancer une seconde fois avec cet `assert` relâché dans la copie mutée, pour montrer que le TU l'attrape aussi, et rapporter les deux.

- **M1** `ij` n'écrit jamais son voisin (`if (0)` sur `*out_nearest = o`) → `test_pico8_mz` (`mz->ij != NULL`, `:455`) / `test_pico8_g_` (`et_kind` planète) ;
- **M2** `ij` ne retient plus le plus proche (`mk = d` neutralisé) → `test_pico8_mz` (rang de `ij`), constantes des TU qui démarrent une partie (`bw`) ;
- **M3** `jc_ce` rend NULL au lieu de « -too far- » → `test_pico8_jc` (`test_too_far`, `ET_QS`) ;
- **M4** `jc_ce` ne reprend plus le slipway existant (`return w` neutralisé) → `test_pico8_jc` (`ET_JC`, `:579`) ;
- **M5** `jc_ce` ignore l'obstacle (`result = ds.items[0]` neutralisé) → `test_pico8_jc`, sinon `make e2e`, sinon assertion ajoutée ;
- **M6** `jc:hq` range NULL (`0 ? pico8_jc_ce(self) : NULL`) → `test_pico8_jc` ;
- **M7** `d_` rend 0 sur la branche de succès → `test_pico8_g_` (`test_ey_off`, `out == 1`), `test_pico8_jc` (`jc:ey`), constantes des plis dont `d_` réussit ;
- **M8** `d_` rend 1 sur la branche d'échec → `test_pico8_g_` (`test_ey_on`, `test_ey_void`, `:585`), `test_pico8_mz` (`mz:ey`) ;
- **M9** `jc:ey` lit `d_` à l'envers → `test_pico8_jc` ;
- **M10** `mz:ey` lit `d_` à l'envers → `test_pico8_mz` ;
- **M11** `hg_has` sans sa garde `id == NULL` → `test_pico8_jc` (`!has(&hg, NULL)`, plantage) ;
- **M12** `hg_add` sans son `assert` nil → `test_pico8_jc` (`must_abort(hg_add_nil)` rouge) ;
- **M13** `hg_add` ne dédoublonne plus (appel à `has` neutralisé) → `test_pico8_jc` (`count == 2`) ;
- **M14** `_init` ne pose pas `g_ie_has_id` → `test_pico8_ie` (`shared_id_fixed`, `:105`, `:419-428`) ;
- **M15** `ldown` ne pose pas le nouvel `id` (`g_ie_id = new_id` neutralisé) → `test_pico8_ie` (`:423`, `:428`) ;
- **M16** `pm:pq` ignore `ng` → `test_pico8_pm` (`:298`) ;
- **M17** `mz:hq` lit `p` comme le vecteur nul → `test_pico8_mz` ;
- **M18** `mz:pr` lit `p` comme le vecteur nul → `test_pico8_mz`, sinon `make e2e`, sinon assertion ;
- **M19** `bv` ignore le voisin (`c != NULL` → `0`) → `test_pico8_g_` (`et_kind` planète), constantes `hj`/`os`/`qh`/`nl` (`fold_et`) ;
- **M20** (preuve du pli typé, dans le TU) `test_pico8_fr` plie `0u` au lieu de `1u` pour le vrai de `d_` → `test_pico8_fr` rouge (sa constante plie bien ce pli), restauré, vert.

Cibles (numéros à `77c916a`) : `entity_geom.c:96-122`, `slipway.c:93-115`, `:160-200`, `:265-339`, `probe.c:150-220`, `:328-365`, `wormhole.c:72-93`, `:127-175`, `research_icon.c:118-130`, `:260-302`, `planet_family.c:360-380`. Une mutation dont la cible n'existe plus sous cette forme est remplacée par une mutation équivalente sur le même comportement, déclarée.

### Filets

- `make test`, `make e2e` (26 conformes) au commit. Aucun e2e ajouté.
- **Balayage ASan+UBSan des TU** : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`, comme à D.12a–D.12d.
- `make sanitize-net`, `make class-coverage-net` : non exigés (« Lecture ») ; s'ils sont lancés, résultat rapporté.
- Aucun TU ne rejoue un fichier d'événements.

### Vérification, budget et tableau « Runs »

**Budget : environ 45 min de temps réel**, mesures, `gcov`, mutations et filets compris ; **plafond 70 min** (au-delà : repli b1/b2). La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make tmp/test_pico8_<nom>` + lancement, par TU | quelques secondes |
| `make test` | de l'ordre de la minute (`slipway.h`, `probe.h`, `entity_geom.h` changent : recompilation large) |
| `make e2e` | ~1 s |
| `make line-coverage EVENTS="tests/e2e/*.json"` | moins d'une minute |
| `gcov` de tous les TU (départ et arrivée) | quelques minutes |
| mutations (20 ; un `make` ciblé chacune) | < 10 min |
| balayage ASan+UBSan des TU | quelques minutes |
| `make data-residue`, recette, classification, greps | < 2 min |

Au-delà du plafond, l'implémenteur s'arrête et note ce qui n'a pas été fait ; rien d'inachevé n'est commis. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + étape) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | `77c916a` (D.15a gradée), avant D.15b | 0 | `valeur 980 103` | … |
| 2 | recette `valeur` | idem | 0 | `valeur 980 lignes 103 fichiers 1105 occ.` | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` ne liste que les 22 chemins de « Empreinte » (plus un TU suiveur listé au message). Ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni `Makefile`.
2. **Greps** de la « Sortie », exactement.
3. **Formes** : `ij` sort un `pico8_struct_header_t *` ; `jc_ce` rend un `pico8_struct_header_t *` ; `d_` rend `int` ; `hg` prend `const char *` ; `ie.id` en nombre + drapeau ; aucune enveloppe ni copie `truthy` de `slipway.c`/`wormhole.c`.
4. **Casts** : aucun cast nouveau ; aucune conversion d'un `void *` vers une struct précise sans son test (`ds.items[0]` vers l'en-tête générique, comme avant, gardé par l'`assert` de `jc:hq`) ; aucun cast de pointeur de fonction.
5. **API** : les cinq signatures de la table « classe (b) », et elles seules, ont changé.
6. **Liens** : grep 5.
7. **Constantes** : grep 6 vide ; plis de `d_` aux mêmes octets (grep 8 vide) ; M20 rapporté.
8. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste, (c) = 0, (b) limitée à la table ; ligne gagnée listée (`probe.c`) ; un seul include retiré, `slipway_display.c:11` (grep 7) ; aucun alias, renommage, cale.
9. **Écarts déclarés** : en-tête au lieu de `void **` ; G4 en deux moitiés ; `pf/mz_value_vec` pris ; deux copies `truthy` prises ; `probe.c:176` ajouté ; `fv` hors empreinte ; include de `slipway_display.c` retiré, en-têtes vidés gardés.
10. **Preuve** : `gcov` (lignes modifiées exécutées ou justifiées ; `L` vide), mutations restaurées par `cp`, balayage ASan+UBSan rapportés.
11. **Commentaires** : quatre sortes de `CLAUDE.md` ; contrats devenus faux réécrits ; aucun renvoi à D.15b ni au plan dans `src/` ou `tests/`.
12. **Docs** : note « Découvert à D.15b » en fin de ligne D.15b ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `77c916a` (ou un descendant qui ne touche que `docs/`), propre ; ou si `valeur` n'y vaut pas 980 / 103 / 1105 occ., `type-tag` 243 / 58, résidu D 1221 / 111, sans explication.
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur le commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change.
- `FAILED:` si un site ne tombe sous aucune règle de ce PRD ni de l'étude (arrêt « champ sans règle »), ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.
- `FAILED:` si le balayage ASan+UBSan des TU n'est pas propre.

**Périmètre et forme**

- `FAILED:` si `tools/`, le `Makefile` ou un `#include "pico8_value.h"` autre que `slipway_display.c:11` change, ou si celui-ci reste ; si le commit touche un chemin hors « Empreinte » sans le lister comme TU suiveur.
- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit.
- `FAILED:` si une signature hors de la table « classe (b) » change, si `et`, `has_et`, `pico8_mz_et`, `pico8_probe_truthy`, `rn`, `tostr`, `oj` ou `qh_qh` changent de type (D.15c, D.16), ou si une durée de vie change.
- `FAILED:` si un cast nouveau est ajouté, si une conversion vers une struct précise n'a pas son test, ou si un pointeur de fonction est casté.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` change.
- `FAILED:` si `valeur` ne passe pas de 980 / 1105 occ. à **911 lignes et 1030 occurrences exactement** (− 69, − 75), sans explication chiffrée ligne à ligne ; si `type-tag` change.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes, alias, renommage d'un identifiant vivant pour échapper aux greps, commentaire compté réécrit hors de son sujet, cale valeur ↔ typé, (c) non nulle, (b) hors de la table.
- `FAILED:` si le **message du commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(b)/(c) avec numéros du parent ; la liste par fichier des lignes qui gagnent une catégorie (« aucune » compris) ; les lignes comptées modifiées non retirées ; la justification de chaque Δlignes > Δoccurrences ; l'include retiré et les en-têtes vidés gardés ; les cas de TU supprimés (G4) et le cas nil gardé ; le rapport `gcov` ; les mutations ; le tableau « Runs » (ou son renvoi au journal). Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` diffère après `make test`.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si une ligne modifiée n'est exécutée ni par un TU ni par les e2e sans justification ; si `L` n'est pas vide.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification.
- `FAILED:` si le plafond est dépassé sans note (et sans repli b1/b2), ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.15b », ou si elle omet : les comptes réels, les lectures de la ligne, la ligne gagnée et son étape, les copies `truthy` prises, l'include retiré et les en-têtes vidés gardés, ce que D.15c et D.16 trouvent.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- `pico8_ov`, `pico8_cb` : faits par D.15a ; son pont `tech_gate.c:45` (`pico8_oj(pico8_value_string(px))`) : D.16b avec `oj`.
- `tostr` (`galaxy.c:79`, `probe.c:273`, `research_icon.c:136`…) : D.15c.
- `et` de la sonde (`probe.c:186-214`, `has_et`, `pico8_mz_et`, `pico8_probe_truthy`) : D.16b (B3) ; `rn` : D.16b (B5) ; `oj`/`il` : D.16b (B4) ; `hf`/`qh_qh`/`jo_gg` : D.16a (B1) ; les autres copies `truthy` : D.16.
- Les includes de `pico8_value.h`, `geom.h:37-38`, `type_tag.h:10`, `collect.c:90` : D.19c (ou avec leur objet).
- Les aides d'empreinte des TU (`fold_value`, `vec_ref`, `mouse_at`…) : D.19a/b.
- Les `assert` d'étiquette et `pico8_type_tag.h` : D.20.
- Tout e2e ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) :
  - **Types** : `pico8_struct_header_t **`/`*` pour `ij`/`jc_ce` (écart à `void **` de l'étude) ; `int` pour `d_` ; `const char *` pour `hg` ; `pico8_fixed_t` + `int` pour `ie.id`.
  - **Portée** : `pf/mz_value_vec` inclus (ligne de l'étude) ; `probe.c:176` inclus (imposé par `ij`) ; deux copies `truthy` incluses (imposé par `-Werror`) ; include de `slipway_display.c` retiré (précédent D.15a), en-têtes vidés gardés pour D.19c.
  - **G4** : nombre supprimé, nil gardé (le type le permet, `src/` le produit).
  - **TU** : suivent dans le même commit ; plis de `d_` directs aux mêmes octets, sans gagner `valeur` ; `fv`, `ie`, `mz`, `pm` ne suivent pas.
  - **Drapeaux** : ni `class-coverage-net` ni `sanitize-net` ; balayage ASan+UBSan des TU.
  - **Découpage** : un commit (≈ 45 min, plafond 70), repli b1/b2 écrit.
- **Surprises de mesure, pour `plan-pilot`** : (1) l'étude omet un appelant d'`ij`, `probe.c:176` (`bv`), dont la locale `c` est rangée en B3 : le changement de sortie la retype ici et fait gagner une ligne `valeur` (`et = pico8_value_ref(c)`), rendue par D.16b ; (2) `pico8_jc_truthy` et `pico8_mz_truthy` n'ont que `d_` pour argument : elles partent ici sous `-Werror` (6 lignes B12 de moins pour D.16) ; (3) G4 dit « `pico8_jc_hg_has` […] de nil » genre non produit, mais `slipway_display.c:29-31` passe un `t_id` NULL (point souris) : le cas nil est gardé ; (4) la ligne du plan omet `pf/mz_value_vec`, que la ligne de l'étude et son « ≈ 910 » comptent ; (5) le TU `fv` que l'étude range parmi les plis de `d_` ne plie rien (`(void)pico8_d_(e)`) : hors empreinte ; (6) `pico8_jc_ce` n'a aucun appelant de TU : sa preuve passe par `jc:hq` et `et_kind` de `test_pico8_jc` ; (7) les numéros de l'étude (`galaxy.c:53`, `wormhole.c:74`) sont ceux des déclarations, les appels sont `:67`, `:85` ; (8) mesuré à `77c916a`, l'étape fait − 69 lignes (70 retirées dont un include, 1 gagnée) : 980 → 911, contre ≈ 910 au plan ; (9) après D.15b, `entity_geom.h`, `probe.h`, `slipway.h` n'incluent plus `pico8_value.h` que pour leurs dépendants : liste pour D.19c ; (10) révision après D.15a : ses hypothèses tiennent sauf H-D15a-1 (980 au lieu de ≈ 979, pont `tech_gate.c:45`), sans effet ; le précédent d'include de D.15a fait retirer `slipway_display.c:11`.
- **Pour D.15c** : `galaxy.c:79`, `probe.c:273`, `research_icon.c:136` (tostr) intacts, numéros au commit.
- **Pour D.16** : prototypes publics qui nomment `pico8_value_t` : 25 → 20 ; `probe.c` : `et = pico8_value_ref(c)` à retirer avec B3 ; `pico8_probe_truthy` n'a plus que `first` (`:196`) ; copies `truthy` restantes : 11 (`economy_score`, `economy_tiers`, `layout`, `planet`, `planet_display`, `probe`, `research`, `research_display`, `tech_gate`, `ui_button`, `wormhole_display`).

## Annexe A — textes recopiés (`docs/plan-3.md` à `77c916a`, par extraction de lignes ; depuis `a702ce4`, seules la ligne D.15a (note) et la ligne « `valeur` (980) » du reste de D ont changé)

### Ligne D.14 (l. 282)

> - D.14 — étude D.9b, lecture seule (Q12 tranchée oui par l'humain, 2026-09-29 : « fais le tri d'abord », le but étant de supprimer la couche Lua) : pour chaque champ, retour ou paramètre encore en `pico8_value_t` dont le genre varie (nil, faux, nombre, chaîne, référence selon le cas), la règle C qui le remplace (drapeau `has_x` + type, pointeur NULL, petite union locale, ou autre), prouvée par les écritures du cart et du port ; puis la liste ordonnée des étapes `valeur`/`type-tag` qui videront ces catégories, sur le modèle de D.9. Les questions à l'humain sont posées en français simple, sans noms minifiés ni numéros de TROU, 100 mots au plus chacune. Découvert à D.14 : étude écrite (`docs/prd/done/plan3-phaseD-etape14-etude-genres-meles.md`) ; 560 lignes mécaniques, 376 à genres mêlés en 12 groupes, chacun avec sa règle C ; décisions G1–G7 prises par l'étude. Q13 (nombre de contrôle en tête des objets du jeu) tranchée par l'humain (2026-09-29) : **A, retiré** ; les `assert` d'étiquette deviennent des contrôles de classe. Étapes ci-dessous, reprises de l'étude (§ étapes), chacune avec sa PRD.

### Lignes D.15a à D.15c (l. 283-285)

> - D.15a — `valeur`, mécanique : `ov` prend des nombres, `cb` un entier. Compte : `valeur` ≈ 979. Découvert à D.15a : `valeur` 980 / 103 (1105 occ.), et non ≈ 979 : `ov` passe `pico8_value_string(px)` à `oj`, ligne qui part à D.16b avec `oj` ; `pico8_tech_gate_truthy` n'a plus que `oj` pour appelant (B12, D.16b) ; `pico8_economy.c` et `test_pico8_fm` sortent de `valeur` (restent au résidu D par `type-tag`) ; `nil()` de `test_pico8_js` retirée ; aucun include gagné. Résidu D 1221 / 111.
> - D.15b — `valeur`, mécanique : `ij`/`jc_ce` en pointeurs, `d_` en entier, `jc_hg` en `const char *`, `g_ie_id`. Compte : ≈ 910.
> - D.15c — `valeur` : `tostr` déplacé dans `pico8_str.c` avec ses tests. `sanitize-net` : oui. Compte : ≈ 880.

### §7, extrait (l. 335, « En D et en 2 »)

> - **En D et en 2** (ajouté à la revue de sortie de 1b) : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée (table ob → locale ou statique, `malloc`/`free` retiré), et à la sortie de phase.

Ligne D.15b identique à `a702ce4` (H-D15a-9 tenue).

## Annexe B — cart (`pico8/lua.lua`), écritures et lectures des genres

- `ij` (l. 351-356) : `local mk,nd=32767` … `nd,mk=o,d` : `nd` vaut nil ou un élément de `j_` ; lu par `bv` (l. 553 `local nq,c=ij(…)` puis `nq<self.dw and c or …`), `bw` (l. 748, distance seule), `mz:hq` (l. 768 `nq,self.ij=ij(…)`).
- `jc:ce` (l. 641-653) : `return {qs="-too far-"}`, `return w`, `return ds[1]` (nil si vide) : nil ou une table ; lu par `jc:hq` (l. 626 `self.et=self:ce()if self.et then …`).
- `d_` (l. 569-576) : `return true` ou fin sans `return` (nil) ; lu par `jc:ey` (l. 634 `or not d_(self)`) et `mz:ey` (l. 775 `not d_(self)`) : vérité seule.
- `hg` (l. 620 `self.hg={}`, l. 657 `self.hg[t.id]`, l. 1271 `w.hg[e.lp[i+1].id]=true`) : clés `id` de planètes (chaînes `"p"..id`, l. 749 ; `"s"..ie.id`, l. 934).
- `ie.id` (l. 933 `ie.id+=1`) : nombre ; nil avant la première écriture.
