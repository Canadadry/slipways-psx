---
title: "Plan 3, étape 1b.19 : `qb` (la caméra) — cinq membres typés, deux écrivains par nom de genres différents (`v` : `ref:autre`, `ma` : `fixe` sans indicateur), forme 1b.13/1b.15"
description: "`pico8_qb_t` a cinq membres `pico8_value_t` (`v` : `ref:autre`, jamais nil, lu et écrit par nom ; `ma` : `fixe`, jamais nil, lu et écrit par nom ; `p` : `ref:autre`, jamais nil ; `ra` : `ref:autre`, nil `stat?` d'homonyme ; `pc` : `ref:table`, nil `stat`). Les typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 190 à 185. L'écrivain par nom recopie la forme 1b.13/1b.15 avec deux aides `static` : `qb_take_vec` (pointeur, nil accepté → NULL) et `qb_take_fixed` (non-pointeur sans indicateur : nil avorte). Aucun genre mêlé."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.19 — `qb` (`scroll.h:21`), `v`/`ma` écrits par nom. Empreinte 26 (12, 14). Compte : 185.

Cadre : les puces **Nil**, **REF**, **Coexistence** (« Écriture : l'écrivain déballe ; nil éteint la présence ; un genre hors de la ligne de recensement du membre est un `assert` ») et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité, pas redécidé. S'appliquent aussi, sans être redérivées, les notes « Découvert à » de `docs/plan-3.md` :

- **1b.1**, puis **1b.3** à **1b.17** : patron appliqué ; `ref:autre` vers un vecteur → `pico8_vec_t *` emprunté, même pointeur (1b.4, puis `bg.mq` en 1b.15, `nf.ra` en 1b.17) ; `fixe` à nil `jamais` sans indicateur même quand un homonyme en a un (1b.6) ; nil `stat?` d'homonyme ne décide rien (1b.7) ; lecteur qui **copie** : R7 vacante (1b.6, 1b.11, 1b.17) ; R3 garde un **défaut module** partagé par toutes les instances, même REF (1b.17, `&g_nf_ra`) ; lecture de R5 pour `ref:autre` : REF non NULL de tag `UNKNOWN` (1b.10, `bg_take_vec` en 1b.15) ;
- **1b.9/1b.10, R7 et aides de boucle de clés** : aide `static` par genre qui prend la **valeur**, une branche par clé, `else` final gardé ; ici **vacantes dans leur moitié boucle** : `qb` n'a pas de clé de construction (`assert` « no construction table was measured », gardé tel quel), ni table `offsetof` ;
- **1b.13/1b.15, forme écrivain de R5**, recopiée point par point : une branche `strcmp` par membre `e`, **dans l'ordre du typedef** ; aide `static` par genre au nom de la struct ; **nil d'abord** pour un pointeur (pose `*out = NULL`, sort sur `REF NULL` — genre **et** pointeur testés) ; **un non-pointeur sans indicateur n'a pas d'état nil : son aide commence par l'`assert` et nil y avorte** (1b.13 point 3, `g__take_fixed` de 1b.14b) ; `assert` « champ sans règle » qui nomme `struct.membre` et le genre ; nom sans branche → `return 0` sans rien toucher ; **chaque test d'arrêt porte les cas non-REF à union entièrement nulle, construits par `memset` puis `kind`** (1b.13 `write_fo_zero_false`, 1b.14b, 1b.15 `zero_fixed`/`zero_string`/`zero_false`, 1b.18 `zero_function`) ; un fixe 0 à union nulle est le témoin accepté (1b.14b) ;
- **1b.14b** : une mutation qui retire l'unique appel d'une aide `static` est arrêtée par `-Werror` ; on la plante sous une forme qui garde l'appel (`if (0)`, nom renommé) ;
- **1b.11, règle R2 des TU lents** : relire les TU de `UNIT_TESTS_SLOW` qui touchent la struct pour y chercher des lectures nil par nom (fait plus bas) ;
- **1b.5 et 1b.7, lectures de R1 pour genres mêlés** ({fixe, bool}, {bool, chaine}) et 1b.16 ({fixe, `ref:pico8_X_t`}) : **sans objet** — chaque ligne de `qb` n'a qu'un genre (voir « Genres mêlés : aucun »).

### État de départ

HEAD `f001a2b` (PRD de 1b.18). **1b.18 (`fq`) est en cours dans l'arbre de travail** (`src/pico8_ui_button.[ch]`, `tests/test_pico8_fq.c`, `docs/plan-3.md`). **Précondition : elle est commitée avant que 1b.19 commence.** Aucun de ses fichiers de code n'est touché ici ; dans `docs/plan-3.md`, 1b.18 écrit sa ligne, 1b.19 la sienne. `make value-members` rend alors 190 (H1). Les numéros de ligne cités sont ceux de HEAD et peuvent glisser : chaque site est aussi désigné par son contenu.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H15).

### Lignes de recensement (`tools/member_census.txt:196-200`)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `v` | offset | `ref:autre` | jamais | - | **le** |
| `ma` | offset | `fixe` | jamais | - | **le** |
| `p` | offset | `ref:autre` | jamais | - | l |
| `ra` | offset | `ref:autre` | observé | stat? | l |
| `pc` | offset | `ref:table` | observé | stat | l |

Cinq membres : 190 − 5 = **185**.

- `v` a `le` : écrit par nom par `bg:hq` seulement (`mo.v = self.mq - rp`, un vecteur neuf du tas), écrit **directement** par `qb:ph` (R6), lu par nom par les tests (`fold_mo`, `check_fresh_mo`, `test_pico8_bg`). Pointeur → pas d'indicateur (NULL = nil).
- `ma` a `le` : écrit par nom par `pico8_ma` (`entity_geom.c`, sur `self` de `qb:ph`, fixe `max(0, ma-0.1)`), par `ga:pq` (`planet_family.c`, `mo.ma = 3`) et par les tests (`test_pico8_qb`, `test_pico8_mo` sur le voile-témoin) ; lu par nom par `pico8_ma` et les tests. Non-pointeur, nil `jamais` → **pas d'indicateur** (R2, 1b.6).
- `p` n'a que `l` : écrit directement par `ph` (R6), lu par nom par `ij` (`entity_geom.c`, sur les entités d'une liste, pas sur `mo` — homonyme) et par les tests (`fold_mo`, `must_vec(mo, "p")`). Pointeur.
- `ra` n'a que `l` : écrit directement par `ph` (R6, `pico8_ma(p, self)` dans un vecteur neuf), lu par nom sur `mo` par `gd` (`entity_geom.c`), `ui:lj` (`ui.c`), `pz` (`msgbox.c`) et par une dizaine de TU ; lu directement par `md`. Le nil `stat?` vient des homonymes (`jo.ra`, `nf.ra`…). Pointeur.
- `pc` n'a que `l` : posé par le constructeur (`g_qb_pc`, la table que le module possède, **distincte** du `pc` de la spec), lu directement par `ph` (`self->pc.kind == PICO8_VALUE_REF && self->pc.as.ref != NULL`, la preuve `stat`), lu par nom par `check_fresh_mo` et `test_second_jl_keeps_specs` (spec seulement). Pointeur.
- Lignes voisines, **non typées ici** : les homonymes `ra`/`p`/`ma`/`v` de `jo` (1b.23b, 1b.23d), `mz`, `ui`, `msgbox` ; `mo_placeholder` (pas de membre, `test_pico8_mo`).

### Genres mêlés : aucun

Chaque ligne n'a qu'un genre. Vérifié côté écrivains : `v` ne reçoit que des REF de vecteurs (`&g_qb_v`, le vecteur neuf de `ph`, celui de `bg:hq`) ; `ma` ne reçoit que des fixes (`pico8_value_fixed(0)` de l'init, `pico8_value_fixed(clamped)` de `pico8_ma`, `pico8_value_fixed(3)` de `ga:pq` et des tests). **Aucune lecture de R1 pour genres mêlés ne s'applique** (1b.5, 1b.7, 1b.16) et aucune n'est décidée ici.

### Qui écrit `v`/`ma` par nom (écrivains génériques)

Les notes de 1a (1a.16b) nomment des écrivains génériques sur `cg.jo` (`kr` d'`economy.c`, `cc`/`by`/`mi` d'`economy_tiers.c`, `t` de `bootstrap.c`) : **aucun n'écrit `v` ni `ma`**. `grep -rn '_field[^;]*"\(v\|ma\)"' src/` ne rend, en écriture, que :

| site | nom | cible | valeur |
|---|---|---|---|
| `pico8_bg_hq` (`bg.c`) | `v` | `pico8_mo()` | REF d'un `pico8_vec_t` neuf du tas |
| `pico8_ma` (`entity_geom.c`) | `ma` | `e` : `self` de `qb:ph`, ou un `jo` depuis `planet_display.c` (écrivain de `jo`, hors étape) | `pico8_value_fixed(clamped)` |
| `pico8_ga_pq` (`planet_family.c`) | `ma` | `pico8_mo()` | `pico8_value_fixed(3)` |

Le `"v"` de `probe.c`, `particle.c`, `slipway_display.c`, `planet_family.c:831` et le `"ma"` de `planet.c` sont des tables de particules ou la table de défauts de `jo` : hors `qb`. `pico8_mo()` est soit le `mo_placeholder` (sans membre, son écrivain rend 0 et la couche entité avorte : `test_placeholder_is_a_struct`), soit un `qb` (`jl`).

**Revue des TU lents (règle R2 de 1b.11).** `test_pico8_ie`, `jo`, `qh`, `os` ne nomment ni `mo` ni `qb`. `test_pico8_hj`, `mx`, `nr`, `pm` lisent `ra` de `mo` par nom (`get_ref`, pointeur) ; `test_pico8_pm` plie `ma` de `mo` (`fold_field`, présent/absent). `mo` y est un `qb` construit par `jl`, dont `ma` n'est jamais nil (aucun écrivain de nil). **Aucune lecture nil par nom qui change une décision** (H4).

### Ce que fait le cart (`pico8/lua.lua`)

- `qb=ke:jr([[v=v(0,0),ma=0,p=v(192,192),ra=v(192,192),pc=o(v(-1,0),v(1,0),v(0,-1),v(0,1)),]])` (`:1128-1134`) : défauts de classe, partagés par toutes les instances jusqu'à la première écriture.
- `qb:ph` (`:1134-1141`) : `self.p+=self.v`, `self.v+=(ip-self.v)*0.2`, bornes `mid(-64,320,…)`, `self.ra=ma(self.p,self)` — les `+=` sur des vecteurs **rebindent** (vecteurs neufs), ils ne mutent pas.
- `qb:md(fy)` (`:1142`) : `camera(self.ra*(fy or 1))`.
- `function ma(p,e) e.ma=max(0,e.ma-0.1) return p+qm(e.ma,e.t*0.2) end` (`:349`) ; `bg:hq` : `mo.v=self.mq-rp` (`:381`) ; `ga:pq` : `mo.ma=3` (`:1079`) ; `jl` : `mo=qb()` (`:155`).

### Le code aujourd'hui

`src/pico8_scroll.h` (`:19-26`) : `{ pico8_ke_t ke; pico8_value_t v, ma, p, ra, pc; } pico8_qb_t` (un membre par ligne), contrat au-dessus : « `v`, `p` and `ra` start on vectors every qb shares, `pc` on the direction table the module owns: none is ever modified in place, only rebound. Nothing frees the struct: TROU-7. » Contrat de module : « a nil or wrong-kind vector member reads as the zero vector ». N'inclut pas `pico8_vec.h` (ni directement, ni par `pico8_ke.h`, `pico8_struct.h`, `pico8_ob.h`, `pico8_value.h`) : le typedef typé l'ajoute, comme `bg.h` (1b.15) et `mouse_reader.h` (1b.17) — seul include ajouté (H12). API publique inchangée : `pico8_qb_init`, `_shutdown`, `_ready`, `_default`, `_struct_class`, `pico8_qb_new`, `pico8_qb_ph`, `pico8_qb_md`.

`src/pico8_scroll.c` (215 lignes, tout `qb`) :

- **Défauts module** (`:14-18`) : `g_qb_spec`, `g_qb_pc` (table possédée), `g_qb_v` (0,0), `g_qb_p` et `g_qb_ra` (192,192, posés par `pico8_qb_init`).
- **`pico8_qb_vec`** (`:21-26`, `static`) : `pico8_value_t` → vecteur, nil et mauvais genre → (0,0).
- **Lecteur** `pico8_qb_field` (`:48-70`) : copie locale `pico8_value_t value` choisie par cinq `strcmp`, `REF NULL` → `return 0`, sinon copie ; repli `pico8_ke_field`. Lecteur **qui copie** : pas de `pico8_value_t *`, pas de table `offsetof` (`grep -rn 'offsetof(pico8_qb_t' src tests` vide).
- **Écrivain** `pico8_qb_set_field` (`:74-85`) : `ma` puis `v`, `qb-><m> = value` (n'importe quel genre), `return 1` ; autre nom → `return 0`.
- **Constructeur** `pico8_qb_struct_new` (`:93-111`) : `malloc`, `pico8_ke_struct_init`, `v = REF &g_qb_v`, `ma = FIXED 0`, `p = REF &g_qb_p`, `ra = REF &g_qb_ra`, `pc = REF g_qb_pc` ; `assert` « qb: no construction table was measured… » (pas de boucle de clés) ; `pico8_class_coverage_record("qb")` ; `pico8_ke_struct_pq` ; `PICO8_MEMBER_CENSUS_SEE(self, pico8_qb_t)`.
- **`pico8_qb_ph`** (`:150-204`) : lit `pc` directement (test REF non NULL), `p`/`v` par `pico8_qb_vec`, écrit `self->p`, `self->v`, `self->ra` = REF de vecteurs neufs (R6) ; `pico8_ma(p, self)` lit et écrit `ma` **par nom** au passage.
- **`pico8_qb_md`** (`:206-215`) : `pico8_qb_vec(self->ra)`.

**Sites par nom, inchangés** : `pico8_bg_hq` (`"v"`), `pico8_ma` (`"ma"`, `"t"`), `pico8_ga_pq` (`"ma"`), `gd` (`"ra"` de `mo`), `ui:lj` (`"ra"` de `mo`), `pz` de `msgbox.c` (`"ra"` de `mo`) ; les TU.

**Arrêt « nom inconnu »** : `pico8_entity_set_field` (`src/pico8_entity.c`) appelle l'écrivain, puis `PICO8_MEMBER_CENSUS_WRITE`, puis `assert(written && …)`. Inchangé ; déjà figé pour `"p"` par `test_qb_is_a_struct`.

### Empreinte remesurée

La ligne dit 26 (12, 14) : majorant des noms courts partagés (`v`, `p`, `ra`, `ma`, `pc`). **Empreinte réelle** (fichiers qui lisent ou écrivent un membre d'un `pico8_qb_t`, directement ou par nom sur `mo`/un `qb`) : **19 (6, 13)** —

- `src/` (6) : `pico8_scroll.c` (direct), `pico8_entity_geom.c` (`ra` de `mo`, `ma` par `pico8_ma`), `pico8_ui.c` (`ra` de `mo`), `pico8_msgbox.c` (`ra` de `mo`), `pico8_bg.c` (`v` de `mo`), `pico8_planet_family.c` (`ma` de `mo`) ;
- `tests/` (13) : `test_pico8_qb` (tous), `test_pico8_mo` (`ra`, et `ma` sur le voile-témoin), `test_pico8_spec_defaults` (`STRUCT_HASH` : `v`, `ma`, `p`, `ra`, `pc` dans `NAMES`), `test_pico8_bg` (`v`), `test_pico8_ga`, `test_pico8_pm` (`ma`, `ra`), `test_pico8_ui`, `test_pico8_jc`, `test_pico8_fr`, `test_pico8_hp`, `test_pico8_hj`, `test_pico8_mx`, `test_pico8_nr` (`ra`).
- Hors compte : `pico8_bootstrap.c` (construit `mo`), `test_pico8_planet_kj_dispatch` (appelle `pico8_qb_md` sur un `qb` réel, ne nomme aucun membre).

**Fichiers modifiés : 3**, plus `docs/plan-3.md` : `src/pico8_scroll.h` (typedef, contrat de module), `src/pico8_scroll.c`, `tests/test_pico8_qb.c` (pur ajout). **Aucun TU de `UNIT_TESTS_SLOW` n'est touché.**

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** : lectures par nom des cinq membres (deux `qb`, défauts partagés, identité des REF), allers-retours d'écriture par nom de `v` (dont nil) et de `ma` (dont fixe 0 à union nulle), `ph` après un `v` nil, écrivain brut sur les noms lus seuls, `pc` absent d'un `qb` construit hors init (voir Testing Decisions).
2. **Typage** : `pico8_vec_t *v; pico8_fixed_t ma; pico8_vec_t *p; pico8_vec_t *ra; pico8_ob_table_t *pc;` (en-tête `ke` en tête, noms et ordre gardés, **aucun `has_`**).
3. **Lecteur** : cinq branches R4 `strcmp` dans l'ordre du typedef, copie locale supprimée.
4. **Écrivain** : forme 1b.13/1b.15, branches `v` puis `ma` (ordre du typedef), deux aides `static` : `qb_take_vec` (nil → NULL) et `qb_take_fixed` (nil avorte). Tout autre nom rend 0 ; la couche entité avorte comme avant.
5. **R6 dans `ph`/`md`** : affectations de pointeurs, `pico8_qb_vec` prend un `const pico8_vec_t *` (NULL → (0,0)).
6. **Un test d'arrêt**, rouge sur l'ancien code (qui accepte tout), vert après, avec les cas non-REF à union nulle et le nil de `ma`.
7. Effet mesurable : `make value-members` passe de 190 à **185** et ne liste plus aucune ligne `pico8_qb_t`. Aucun pixel ne bouge.
8. **Note « Découvert à 1b.19 »**.

## User Stories

1. En tant que lecteur de `scroll.h`, je veux `v`, `p`, `ra` en `pico8_vec_t *`, `ma` en `pico8_fixed_t` et `pc` en `pico8_ob_table_t *`, afin que ce que tient la caméra se lise sur le type.
2. En tant que `bg:hq`, je veux écrire `mo.v` par nom avec un vecteur neuf, comme aujourd'hui, et que `v` lu par nom rende ensuite ce même pointeur.
3. En tant que `pico8_ma` et `ga:pq`, je veux écrire `ma` par nom avec un fixe, et le relire `FIXED` présent, 0 compris.
4. En tant que lecteur par nom de `v`, je veux que nil écrit par nom rende `v` absent (0, `*out_value` intact), un nil de plus aussi, et qu'un vecteur écrit ensuite soit relu tel quel.
5. En tant que `qb:ph`, je veux qu'un `v` nil compte pour le vecteur nul, comme aujourd'hui, et que `ph` rebinde ensuite `v`, `p` et `ra` sur des vecteurs neufs.
6. En tant que mainteneur, je veux qu'écrire nil sur `ma` par nom avorte par l'`assert` « champ sans règle » qui nomme `qb.ma` et `fixe`, parce que `ma` n'a pas d'état nil.
7. En tant que mainteneur, je veux qu'une écriture de `ma` ni fixe (nil, `false` à union nulle, chaîne à union nulle, fonction à union nulle, booléen, chaîne, REF de vecteur, REF de struct) avorte.
8. En tant que mainteneur, je veux qu'un fixe 0 à union nulle écrit sur `ma` soit un `FIXED 0` présent, jamais une erreur.
9. En tant que mainteneur, je veux qu'une écriture de `v` ni nil ni REF de vecteur (fixe, fixe 0 à union nulle, `false` à union nulle, chaîne, chaîne à union nulle, fonction à union nulle, booléen, REF de table, de liste, de struct) avorte par l'`assert` qui nomme `qb.v` et `ref:autre`.
10. En tant que lecteur de `mo` (`gd`, `ui:lj`, `pz`, `md`), je veux `ra` présent, `REF` d'un vecteur, identique à aujourd'hui.
11. En tant que `jl`, je veux qu'un `qb` neuf serve `v` (0,0), `p` et `ra` (192,192), les trois partagés par tous les `qb` neufs, `p` ≠ `ra`, `ma` `FIXED 0`, `pc` la table du module (4 directions), distincte du `pc` de la spec.
12. En tant que mainteneur, je veux qu'un `qb` construit alors que la table du module n'existe pas (après `pico8_qb_shutdown`) lise `pc` absent, comme aujourd'hui.
13. En tant que mainteneur, je veux que l'écrivain brut rende 0 sans rien toucher pour tout nom autre que `v` et `ma`, et qu'écrire `p`, `ra` ou `pc` par nom avorte toujours, par la couche entité.
14. En tant que lecteur de `ke`, je veux que `me` et `t` d'un `qb` restent servis par le repli `pico8_ke_field`.
15. En tant qu'auteur de 1b.23b/1b.23d (`jo.ra`, `jo.p`, `jo.ma` écrits par nom), je veux une forme écrite de l'écrivain R5 sur un `ref:autre` et sur un fixe sans indicateur.
16. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
17. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
18. En tant que gardien de la couverture, je veux `make class-coverage-net` identique à sa référence (`qb  38  15/15`), puisque le constructeur change.
19. En tant que `plan-pilot`, je veux `make value-members` à 185.
20. En tant que vérificateur, je veux la caractérisation lancée sur l'ancien code, le test d'arrêt rouge puis vert, les mutations du §7 avec leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés comme le ferait un agent `guideline` (hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables), et marqués (G).

### R1–R9 appliquées à `qb`

| règle | sur `qb` |
|---|---|
| R1 | `v`, `p`, `ra` → `pico8_vec_t *` (lecture `ref:autre` de 1b.4) ; `ma` → `pico8_fixed_t` ; `pc` → `pico8_ob_table_t *`. Pointeurs non `const`. Noms et ordre gardés. Aucun genre mêlé. `#include "pico8_vec.h"` ajouté à `scroll.h`. |
| R2 | **Aucun indicateur.** `v`, `p`, `ra`, `pc` : pointeurs (NULL = nil). `ma` : non-pointeur, nil `jamais` (1b.6), aucune lecture par nom ne le voit nil, TU lents compris. |
| R3 | Dans `pico8_qb_struct_new`, à la place des cinq affectations actuelles, même position (après `pico8_ke_struct_init`, avant l'`assert` des clés, `pico8_class_coverage_record` et `pico8_ke_struct_pq`) : `v = &g_qb_v`, `ma = 0` (`pico8_fixed_from_int(0)`), `p = &g_qb_p`, `ra = &g_qb_ra`, `pc = g_qb_pc`. **Défauts module, même REF** (1b.17), `pc` reste la table du module, pas celle de la spec. |
| R4 | Cinq branches `strcmp` dans l'ordre du typedef, avant le repli `pico8_ke_field`. `v`, `p`, `ra`, `pc` : NULL → `return 0`, sinon `pico8_value_ref(<m>)`, même pointeur. `ma` : toujours présent, `pico8_value_fixed(ma)`. Dans chaque 0, `*out_value` intact. |
| R5 | **Moitié écrivain** : forme 1b.13/1b.15 (section suivante). **Moitié boucle : vacante**, pas de clé ; l'`assert` « qb: no construction table was measured… » reste tel quel. |
| R6 | `ph` : `pc_table = self->pc` ; `pico8_qb_vec(self->p)`, `pico8_qb_vec(self->v)` ; `self->p = p_new`, `self->v = v_new`, `self->ra = ra_new`. `md` : `pico8_qb_vec(self->ra)`. `pico8_qb_vec` devient `static pico8_vec_t pico8_qb_vec(const pico8_vec_t *v)` : NULL → (0,0), sinon `*v`. `pico8_ma(p, self)` reste l'unique écriture de `ma` dans `ph`, par nom. Ordre des instructions de `ph` inchangé. |
| R7 | **Vacante** : lecteur qui copie (1b.6, 1b.17), aucune table `offsetof`, aucun `pico8_qb_member`. La copie locale `pico8_value_t value` est retirée par R4. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_qb_t)` gardé ; `PICO8_MEMBER_CENSUS_WRITE` inchangé ; `tools/member_census.txt` non édité. |
| R9 | 190 → 185, aucune ligne `pico8_qb_t`. |

Contrat de module de `scroll.h` : « a nil or wrong-kind vector member reads as the zero vector » devient « a nil (NULL) vector member reads as the zero vector » (le mauvais genre ne peut plus être tenu). C'est un contrat C (règle des commentaires) ; le reste des commentaires est inchangé.

### R5, moitié écrivain : deux aides

1. **Noms écrits.** Deux branches, **dans l'ordre du typedef** (G : l'ancien écrivain testait `ma` avant `v`, 1b.13 point 1 fixe l'ordre du typedef) :
   `if (strcmp(name, "v") == 0) { qb_take_vec(value, &qb->v); return 1; }`
   `if (strcmp(name, "ma") == 0) { qb_take_fixed(value, &qb->ma); return 1; }`
   `return 0;`. `p`, `ra`, `pc` sont `l` seuls : pas de branche, leur écriture par nom reste « nom inconnu ».
2. **`static void qb_take_vec(pico8_value_t value, pico8_vec_t **out)`**, copie de `bg_take_vec` : `*out = NULL;` puis `if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) return;` — genre **et** pointeur ; puis `assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN && "qb.v: a write neither nil nor a vector ref is off its census row (ref:autre) -- a field under no rule of plan-3 section 5: stop, do not widen the member");` puis `*out = value.as.ref;`. Nil éteint donc `v` quel que soit l'état d'avant ; un non-REF à union nulle n'est pas nil (genre testé), il avorte.
3. **`static void qb_take_fixed(pico8_value_t value, pico8_fixed_t *out)`**, forme `g__take_fixed` (1b.14b) : **pas d'état nil**, l'aide commence par `assert(value.kind == PICO8_VALUE_FIXED && "qb.ma: a write that is not a fixed is off its census row (fixe, never nil) -- a field under no rule of plan-3 section 5: stop, do not widen the member");` puis `*out = value.as.fixed;`. Nil (`REF NULL`, qui est aussi le REF à union nulle) avorte.
4. **Pourquoi nil avorte sur `ma`** (G, pas une lecture nouvelle) : c'est la règle 1b.13 point 3 (« un non-pointeur sans indicateur n'a pas d'état nil, son aide commence par l'`assert` et nil y avorte »). L'ancien code acceptait nil et lisait ensuite `ma` absent ; aucun producteur n'écrit nil (H2), et R2 n'accorde pas d'indicateur à un fixe nil `jamais`. Donner un `has_ma` pour préserver un nil que personne n'écrit serait élargir le membre.
5. **Nom inconnu : inchangé.** Pas d'`assert` dans l'écrivain ; `pico8_entity_set_field` avorte par son propre `assert(written && …)`, message inchangé.

### Preuves

Toutes sur l'arbre final, sauf mention.

- `make value-members | grep -c pico8_qb_t` : 5 avant ; **0** après ;
- `grep -rn 'offsetof(pico8_qb_t' src/ tests/` : vide avant comme après ;
- `grep -nE '(->|\.)(v|ma|p|ra|pc)\.(kind|as)' src/pico8_scroll.c` : 2 lignes avant (`self->pc.kind`, `self->pc.as.ref` de `ph`) ; **vide** après ;
- `grep -nE 'self->(v|ma|p|ra|pc) = pico8_value_' src/pico8_scroll.c` : 8 lignes avant (5 de l'init, 3 de `ph`) ; **vide** après ;
- `grep -nE 'qb->(v|ma) = ' src/pico8_scroll.c` : 2 avant ; **vide** après (tout passe par les aides) ;
- `grep -n 'pico8_value_t \*' src/pico8_scroll.c | grep -v 'pico8_value_t \*out_value)'` : vide avant comme après ;
- `grep -n 'pico8_value_t value;' src/pico8_scroll.c` : 1 avant (copie du lecteur) ; **vide** après ;
- `grep -c 'off its census row' src/pico8_scroll.c` : 0 avant ; **2** après (`qb.v`, `qb.ma`) ;
- `grep -c 'has_' src/pico8_scroll.h` : 0 avant comme après ;
- `git diff --stat` : seuls `src/pico8_scroll.h`, `src/pico8_scroll.c`, `tests/test_pico8_qb.c`, `docs/plan-3.md`.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 190 sur l'arbre de départ (1b.18 commitée), puis 185.
- **H2** : dans les 15 rejeux et dans les TU hors fils de `must_abort*`, `v` d'un `qb` ne reçoit par nom que des REF de vecteurs du tas (`bg:hq`, tests) ou, dans la caractérisation, `REF NULL` ; `ma` d'un `qb` ne reçoit par nom que des `FIXED` (`pico8_ma`, `ga:pq`, tests). Sinon une aide avorte : arrêt « champ sans règle ».
- **H3** : aucun vecteur écrit sur `v` n'a une abscisse dont les 4 octets égalent un tag (`0x9E5C…`, soit environ −25 000 en 16.16) : `pico8_type_tag_of` le rend `UNKNOWN` (même hypothèse que `bg.mq`, 1b.15).
- **H4** : aucun TU de `UNIT_TESTS_SLOW` ne lit un membre de `qb` nil par nom (revue ci-dessus).
- **H5** : `SCRIPTED_HASH`, `SHAKE_HASH`, `TITLE_HASH`, `GEOHARVEST_HASH` (`test_pico8_qb`), `STRUCT_HASH`/`SPEC_HASH`/`READY_HASH` (`test_pico8_spec_defaults`), les empreintes de `test_pico8_bg` (`DRAG_HASH`…), `ga`, `pm`, `ui`, `mo`, `jc`, `fr`, `hp`, `hj`, `mx`, `nr`, `planet_kj_dispatch` et les 15 du filet ne bougent pas.
- **H6** : le build `-DPICO8_MEMBER_CENSUS` compile sur une struct sans `pico8_value_t` propre. Il n'est pas lancé.
- **H7** : sous ASan+UBSan, rien de nouveau : aucune allocation ajoutée ni retirée (les trois `malloc` de `ph` restent, TROU-7).
- **H8** : `test_pico8_qb` (deux rejeux en fils, dont 4 800 frames) dure moins de 20 s, et reste sous 30 s avec les deux fonctions ajoutées ; sinon l'étape le note, le TU ne change pas de liste.
- **H9** : 1b.18, commitée, ne touche ni `pico8_scroll.[ch]` ni `test_pico8_qb.c`.
- **H10** : `make class-coverage-net` rend `qb  38  15/15`, identique à `tools/class_coverage_net_expected.txt` (le corps du constructeur change, pas le nombre d'instanciations).
- **H11** : le lecteur brut `pico8_qb_struct_class()->field` rend `me` (`STRING "ph"`) et `t` (`FIXED 0`) par le repli `pico8_ke_field`, comme `pico8_entity_get_field`.
- **H12** : `#include "pico8_vec.h"` ajouté à `scroll.h` suffit au typedef, sans cycle d'includes (lu : `pico8_vec.h` n'inclut pas `scroll.h`) ; c'est le seul include ajouté, `scroll.c` l'a déjà.
- **H13** : `pico8_qb_new(NULL)` appelé après `pico8_qb_shutdown()` (ke prêt) construit un `qb` dont `pc` est `REF NULL` sur l'ancien code (lu absent) et NULL sur le nouveau, sans crash ; `pico8_jl()` ensuite (qui refait `pico8_qb_init`, `pico8_qb_ready()` valant 0) remet un état propre et vide `hv` des `qb` qui pointent l'ancienne table libérée.
- **H14** : les fils du test d'arrêt, créés par `fork` dans `test_pico8_qb` (TU de `CENSUS_UNIT_TESTS`), ne vident rien au recensement : ils meurent par `SIGABRT`, ou sur l'ancien code sortent par `_exit(0)` (1b.4b).
- **H15** : `pico8_mv(b, "ph", …)` sur un `qb` `b` hors `mo`, avec l'entrée de répartition au repos (`unscript()`), n'utilise pas `rnd` : les valeurs figées de la caractérisation ne dépendent pas de l'ordre des tests qui précèdent dans `main`.

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Aucun accès direct à `->v`/`->ma`… dans les tests : ils compilent sur les deux formes.
- **Caractérisation dirigée, figée sur l'ancien code** (G) : `test_qb_members_by_name`, dans `test_pico8_qb` (rapide, dans `CENSUS_UNIT_TESTS`), appelée en fin de `main`, après `test_qb_is_a_struct`. Elle lit par le **lecteur brut** (`pico8_qb_struct_class()->field`, témoin : fixe `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder ; elle écrit par l'**écrivain brut** (`->set_field`) et par `pico8_entity_set_field`.
  1. **Lectures par nom.** `pico8_jl()`, `a = pico8_mo()`, `b = pico8_qb_new(NULL)`. Pour chacun : `v` → REF, (0,0) ; `ma` → `FIXED 0` ; `p`, `ra` → REF, (192,192), `p` ≠ `ra` ; `pc` → REF, table de 4 directions ; `me` → `STRING "ph"`, `t` → `FIXED 0` (H11). Identités : `v`, `p`, `ra`, `pc` de `a` et `b` sont les **mêmes pointeurs** (défauts module) ; `pc` ≠ le `pc` de `pico8_qb_default`. Intrus `""`, `"V"`, `"x"`, `"q"`, `"has_v"`, `"has_ma"`, `"ke"`, `"header"`, `"pa"`, `"m"` → 0, témoin intact.
  2. **Aller-retour d'écriture par nom de `v`**, sur `b` : vecteur du tas `A` (3,4) par `pico8_entity_set_field` → `v` = `A` (même pointeur) ; écrivain brut `REF NULL` rend 1 → **absent** ; `REF NULL` encore → absent ; vecteur `B` (−2,5) → `B` ; à chaque pas, `ma`, `p`, `ra`, `pc` inchangés (valeurs et pointeurs), et `v` de `a` inchangé (partage non touché).
  3. **Aller-retour d'écriture par nom de `ma`**, sur `b` : `FIXED 3` → 3 ; écrivain brut fixe 0 **à union nulle** (`memset`, `kind = FIXED`) rend 1 → `FIXED 0` présent ; `FIXED −1` → −1 ; `FIXED 3` → 3 ; à chaque pas, `v`, `p`, `ra`, `pc` inchangés.
  4. **`ph` après un `v` nil** : `b` avec `ma` = 3 et `v` nil (pas 2), entrée au repos (`unscript()`), `pico8_mv(b, "ph", …)` → `v` présent, (0,0), pointeur neuf (≠ `&` partagé, ≠ `B`) ; `p` présent, (192,192), pointeur neuf ; `ra` présent, pointeur neuf, `p` ≠ `ra` ; `ma` = `max(0, 3 − 0,1)` ; `pc` inchangé. Un second `ph` : `v`, `p`, `ra` à nouveau neufs.
  5. **Écrivain brut sur un nom qu'il n'écrit pas** : `(b, n, pico8_value_fixed(1))` rend 0 pour `n` ∈ {`"p"`, `"ra"`, `"pc"`, `"me"`, `"t"`, `""`, `"V"`, `"ma "`, `"has_v"`} ; ensuite l'état lu est celui d'avant. Puis, par un utilitaire `must_abort(e, name)` ajouté au fichier (forme de `test_qb_is_a_struct`, `fork` + `SIGABRT`) : `pico8_entity_set_field` de `"ra"` et `"pc"` avorte — **déjà vrai sur l'ancien code** (« nom inconnu »), d'où sa place ici (`"p"` l'est déjà par `test_qb_is_a_struct`).
  6. **`pc` absent hors init** (H13) : `pico8_qb_shutdown()`, `c = pico8_qb_new(NULL)` → `pc` absent (0, témoin intact), `v`/`p`/`ra` présents ; puis `pico8_jl()`.
  7. On plie les retours, genres, fixes, vecteurs lus, identités comparées (0/1) et comptes dans **`QB_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ. Les pointeurs eux-mêmes sont jugés par `assert`, jamais pliés.

  Lancée verte sur l'arbre de départ **avant** le typage (ligne « Runs »). Ses valeurs ne changent pas ensuite. `bg:hq`, `ga:pq` et `pico8_ma` ne sont pas recopiés : `test_pico8_bg` (`DRAG_HASH`, `fold_vec(mo, "v")`), `test_shake_decays`, `test_geoharvest_shakes` et les rejeux figent déjà leurs écritures.
- **Test d'arrêt, rouge d'abord** (G) : `test_qb_writer_stops_off_row`, appelé après la caractérisation, avec un utilitaire `must_abort_value(void *e, void (*write)(void *, pico8_value_t), pico8_value_t v)` (forme de `test_pico8_bg`) : chaque cas est un fils qui doit mourir par `SIGABRT`. Valeurs à union nulle construites par `memset(&v, 0, sizeof v)` puis `v.kind = …` : `zero_fixed`, `zero_false`, `zero_string`, `zero_function`, et `zero_ref` (REF à union nulle, c'est-à-dire nil).
  - **`v` par nom** : `pico8_value_fixed(1)` ; `zero_fixed` ; `zero_false` ; `zero_string` ; `zero_function` ; `pico8_value_bool(1)` ; `pico8_value_string("x")` ; la fonction `ph` lue sur `b` ; REF de la table `pc` ; REF de la liste `pico8_hv()` ; REF d'un autre `qb` (struct).
  - **`ma` par nom** : **`zero_ref`** (nil) ; `pico8_value_ref(NULL)` ; `zero_false` ; `zero_string` ; `zero_function` ; `pico8_value_bool(1)` ; `pico8_value_string("x")` ; REF d'un vecteur du tas ; REF d'un autre `qb`.

  **Rouge sur l'ancien code** (l'écrivain recopie tout, le fils sort par `_exit(0)`), vert après : une ligne « Runs » pour chacun des deux passages. Il n'est pas dans la caractérisation figée, qui ne contient que ce que l'ancien code fait déjà. Le fixe 0 à union nulle sur `ma` est le témoin accepté (1b.14b) : il est dans la caractérisation (3), pas ici.
- **Tests existants, inchangés, qui figent déjà beaucoup** : `test_jl_builds_a_camera`/`check_fresh_mo` (défauts, `p` ≠ `ra`, `pc`), `test_md_sets_the_camera`, `test_scripted_ticks` (`SCRIPTED_HASH`), `test_shake_decays` (`ma` par nom, `SHAKE_HASH`), `test_second_jl`, `test_placeholder_mo`, `test_qb_is_a_struct` (arrêt sur `"p"`), les deux rejeux (`TITLE_HASH`, `GEOHARVEST_HASH`) ; `test_struct_defaults` (`STRUCT_HASH`) ; `test_pico8_bg` (`v` de `mo`) ; `test_pico8_mo` ; `test_pico8_planet_kj_dispatch` (caméra (192,192) par `md`). Aucun test de régression (§6) ne lit ces membres autrement que par nom.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_qb && ./tmp/test_pico8_qb` (et `test_pico8_spec_defaults` là où c'est dit). Chacune est notée dans le commit avec le test qui la rougit, ou la justification de son trou.
  1. (§7-1) *indicateur de présence forcé vrai sur un membre à nil observé* : **sans objet dans sa lettre** — les deux membres à nil observé (`ra`, `pc`) sont des pointeurs, `qb` n'a aucun `has_`. Forme la plus proche, jouée : (a) la branche R4 de `pc` rend 1 avec `REF NULL` quand `pc` est NULL → rouge : caractérisation 6 ; (b) celle de `v` fait de même → rouge : caractérisation 2 et 4. **Trou justifié** pour `ra` et `p` : aucun chemin public ne les rend NULL (défaut module non NULL, `ph` les rebinde sur des `malloc`).
  2. (§7-2) *lecture emballée d'un fixe typé qui rend « présent 0 » là où il était absent* : **sans objet dans sa lettre** — le seul fixe, `ma`, n'est jamais absent (nil `jamais`, nil avorte). Forme la plus proche, jouée : la branche `ma` rend `pico8_value_fixed(0)` au lieu de `ma` → rouge : `test_shake_decays` (`ma` = 3 attendu), caractérisation 3, `test_geoharvest_shakes`.
  3. (§7-3) *init qui saute un membre typé*, `malloc` remplacé par `calloc` dans la copie pour un résultat déterministe : (a) `v = &g_qb_v` omis → rouge : `check_fresh_mo` (`must_vec(v)`), caractérisation 1 ; (b) `p` omis → rouge : `check_fresh_mo` ; (c) `ra` omis → rouge : `check_fresh_mo`, `test_md_sets_the_camera` ; (d) `pc` omis → rouge : `check_fresh_mo` (`ref_of(pc)`) ; (e) `ma = 0` omis : **trou justifié**, `calloc` pose déjà 0, et sous `malloc` l'omission est une lecture non initialisée qu'aucun outil du filet ne signale (ASan ne voit pas les lectures non initialisées) ; la ligne est prouvée exécutée par `gcov`. **Boucle de clés : vacante** (pas de clé de construction, l'`assert` des clés n'est pas une boucle).
  4. Propre à l'étape, *nil jugé sur le pointeur seul* dans `qb_take_vec` → rouge : test d'arrêt (`zero_fixed`, `zero_false`, `zero_string`, `zero_function` sur `v` ne meurent plus).
  5. Propre à l'étape, *nil jugé sur le genre seul* (`kind == REF`) dans `qb_take_vec` → rouge : test d'arrêt (REF de table, de liste, de struct ne meurent plus).
  6. Propre à l'étape, *tag non vérifié* (toute REF acceptée) dans `qb_take_vec` → rouge : test d'arrêt (REF de table, de liste, de struct).
  7. Propre à l'étape, *nil accepté sur `ma`* : `qb_take_fixed` sort sur `REF NULL` avant l'`assert` → rouge : test d'arrêt (`zero_ref`, `REF NULL` sur `ma` ne meurent plus).
  8. Propre à l'étape, *nil n'éteint pas `v`* : `qb_take_vec` teste nil **avant** `*out = NULL` → rouge : caractérisation 2 (`v` présent `A` après nil).
  9. Propre à l'étape, *écrivain qui accepte un autre nom* : une branche `ra` qui rend 1 → rouge : caractérisation 5 (écrivain brut `"ra"` rend 1, `must_abort(ra)` ne meurt plus).
  10. Propre à l'étape, *écriture de `ma` muette* : `if (0) qb_take_fixed(…)` (1b.14b, sinon `-Werror`) → rouge : `test_shake_decays`, caractérisation 3.
  11. Propre à l'étape, *R6 muet* : `ph` ne rebinde plus `v` (`self->v = v_new` retiré) → rouge : `test_scripted_ticks` (`SCRIPTED_HASH`), caractérisation 4.
  12. Propre à l'étape, *`pico8_qb_vec` sans garde NULL* → rouge : caractérisation 4 (le TU meurt sur `v` NULL).
- **`gcov`** : `test_pico8_qb` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23. Chaque ligne modifiée est exécutée (la sortie nil de `qb_take_vec` par la caractérisation 2, la branche NULL de `pc` par la caractérisation 6, la garde NULL de `pico8_qb_vec` par la caractérisation 4) ; trou attendu : l'échec des deux `assert` d'aide, exécutés seulement dans les fils de `must_abort_value`, qui avortent sans vider leur compteur (prouvés par le test d'arrêt et les mutations 4 à 7). Le rapport va dans le commit.
- **Prior art** : `test_bg_members_by_name` et `test_bg_writer_stops_off_row` (1b.15, `must_abort_value`, `zero_*`), `test_too_far_writer_stops_off_row` (1b.13), les tests de 1b.18 dans `test_pico8_fq` (`zero_function`), `test_qb_is_a_struct` (arrêt par `fork`).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_qb` : moins de 20 s (H8) ; environ 20 lancements (caractérisation et test d'arrêt sur l'ancien code, nouveau code, 12 mutations dont 1 et 3 en plusieurs formes, `gcov`), 4 à 6 min. Si un lancement dépasse 20 s, l'implémenteur joue d'abord les mutations §7 (1 à 3) et 4, 7, 10, et note les autres comme non lancées ;
- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min ;
- `make class-coverage-net` : 3 à 4 min, dans la même invocation que `make e2e` (arbres distincts, comme en 1b.14a, 1b.15 et 1b.18) ;
- `make sanitize-net` : ~4 min seul ; il n'est **jamais** lancé en même temps que `e2e` ;
- `make value-members` : quelques secondes.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent (aucun n'est touché) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 185`**, sans aucune ligne `pico8_qb_t` ;
5. `timeout 15m make class-coverage-net` : **s'applique**, parce que le diff touche le corps du constructeur `pico8_qb_struct_new` (R3), sans toucher `pico8_class_coverage_record("qb")` ni `KNOWN[]`. Sortie identique à `tools/class_coverage_net_expected.txt` (`qb  38  15/15`, H10).

`make member-census` ne s'applique pas (R8). On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements. On ne lance jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si une aide avorte dans un rejeu ou dans un TU hors des fils de `must_abort`/`must_abort_value` (H2), si un indicateur `has_` est ajouté, si l'écrivain accepte un autre nom que `v` et `ma`, ou si un type est élargi (union, `ma` tenant nil, `v` tenant autre chose qu'un vecteur) au lieu d'avorter.
- `FAILED:` si les types ne sont pas exactement `pico8_vec_t *v`, `pico8_fixed_t ma`, `pico8_vec_t *p`, `pico8_vec_t *ra`, `pico8_ob_table_t *pc`, dans cet ordre, après `pico8_ke_t ke`.
- `FAILED:` si un défaut change d'identité : `v`, `p`, `ra` d'un `qb` neuf ne sont pas `&g_qb_v`, `&g_qb_p`, `&g_qb_ra` partagés, ou `pc` n'est pas `g_qb_pc`.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur un pointeur NULL ou un nom inconnu, ne rend pas le même pointeur, rend `ma` absent, ou si `me`/`t` ne passent plus par `pico8_ke_field`.
- `FAILED:` si `qb_take_vec` ne pose pas `*out = NULL` **avant** le test de nil, si ce test ne vérifie pas le genre **et** le pointeur, si le tag `UNKNOWN` n'est pas vérifié, ou si le message ne nomme pas `qb.v` et `ref:autre`.
- `FAILED:` si `qb_take_fixed` accepte nil, ne commence pas par l'`assert` `FIXED`, rejette un fixe 0 à union nulle, ou si son message ne nomme pas `qb.ma` et `fixe`.
- `FAILED:` si les branches de l'écrivain ne sont pas dans l'ordre du typedef (`v`, puis `ma`), si l'écrivain avorte lui-même sur un nom inconnu, ou si le message de l'`assert(written && …)` de `pico8_entity_set_field` change.
- `FAILED:` si `ph` ou `md` change autrement que par R6 (ordre des instructions, trois `malloc`, `pico8_ma(p, self)` par nom, TROU-2 et TROU-7 gardés), si `pico8_qb_vec` ne rend pas (0,0) sur NULL, ou si la signature d'une fonction publique de `qb` change.
- `FAILED:` si l'`assert` « qb: no construction table was measured… » ou son message change.
- `FAILED:` si un site par nom change (`pico8_bg_hq`, `pico8_ma`, `pico8_ga_pq`, `gd`, `ui:lj`, `pz`, les tests), ou si `bg.c`, `entity_geom.c`, `planet_family.c`, `ui.c`, `msgbox.c`, `bootstrap.c`, `entity.c` changent.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si le diff d'un test fait autre chose qu'ajouter la caractérisation, le test d'arrêt, leurs utilitaires, `QB_MEMBERS_HASH`, et leurs appels en fin de `main`, ou si une valeur figée existante change (`SCRIPTED_HASH`, `SHAKE_HASH`, `TITLE_HASH`, `GEOHARVEST_HASH`, `STRUCT_HASH`, `SPEC_HASH`, `DRAG_HASH`…), ou si un test de régression (§6) change.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 185, liste encore `pico8_qb_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` s'il manque une ligne « Runs » de `make class-coverage-net` identique à la référence, ou si le diff touche `pico8_class_coverage_record` ou `KNOWN[]`.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit ou écrit un membre directement, ou si `QB_MEMBERS_HASH` change après le typage ; si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'ancien code et une verte après, ou s'il ne porte pas les cas non-REF à union nulle construits par `memset` (`zero_fixed`, `zero_false`, `zero_string`, `zero_function` sur `v` ; `zero_false`, `zero_string`, `zero_function` et `zero_ref` sur `ma`).
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 12 mutations, chacune avec son test ou la justification de son trou.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.19 », ou le tableau « Runs ».
- `FAILED:` si le budget d'environ 20 minutes est dépassé sans que le journal note ce qui n'a pas été lancé.

## Out of Scope

- Les homonymes `ra`, `p`, `ma`, `v` de `jo` (1b.23b, 1b.23d), `mz`, `ui`, `msgbox`, `nf` ; l'écrivain de `jo` atteint par `pico8_ma` depuis `planet_display.c`.
- `mo_placeholder` et `pico8_entity_set_mo`.
- Remplacer une lecture ou écriture par nom (`pico8_ma`, `bg:hq`, `ga:pq`, `gd`, `ui:lj`, `pz`) par un accès direct.
- La table `pc` de la spec, distincte de celle du module ; la durée de vie de `g_qb_pc` après `pico8_qb_shutdown` (un `qb` vivant garde un pointeur libéré, déjà vrai aujourd'hui, non atteint par le filet).
- Une aide de déballage partagée entre fichiers ; retoucher les aides de 1b.5 à 1b.18.
- Régénérer `tools/member_census.txt`.
- Les fichiers d'événements et `FRAMES_EVENTS` ; tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - module : `scroll.[ch]` seul, un commit, sans découpage ;
  - tests : `test_pico8_qb` étendu (caractérisation + test d'arrêt, en fin de `main`), sans nouveau TU ;
  - caractérisation dirigée gardée alors que la ligne n'est pas « sans genre » : deux écrivains par nom de genres différents, et nil n'est écrit par nom dans aucun rejeu (H2) ;
  - aucun genre mêlé, aucune lecture de R1 décidée ; aucun indicateur (R2) ;
  - deux aides, une par genre : `qb_take_vec` (copie de `bg_take_vec`), `qb_take_fixed` (forme `g__take_fixed`, nil avorte) ;
  - branches de l'écrivain remises dans l'ordre du typedef (`v`, `ma`) ;
  - mutations §7-1 et §7-2 sans objet dans leur lettre, jouées sous leur forme la plus proche ; §7-3 rendue déterministe par `calloc` ; `class-coverage-net` lancé.
- **Note « Découvert à 1b.19 »** proposée, sur la ligne 1b.19 de `docs/plan-3.md` : « patron 1b.1 appliqué ; aucun genre mêlé, aucun indicateur ; R3 garde les défauts module (`&g_qb_v`, `&g_qb_p`, `&g_qb_ra`, `g_qb_pc`, 1b.17) ; R7 vacante (lecteur qui copie) ; R5 boucle vacante (pas de clé) ; **R5 écrivain de 1b.13/1b.15 sur deux genres** : `qb_take_vec` (`ref:autre`, copie de `bg_take_vec`, nil → NULL) et **`qb_take_fixed`, premier écrivain par nom sur un fixe sans indicateur : nil avorte** (1b.13 point 3 ; l'ancien code l'acceptait, aucun producteur ne l'écrit), cas `zero_ref` au test d'arrêt ; branches remises dans l'ordre du typedef ; mutations §7-1/§7-2 sans objet dans leur lettre (aucun `has_`, `ma` jamais absent) ; revue des TU lents sans effet ; empreinte réelle 19 (6, 13), 3 fichiers modifiés. » 1b.23b/1b.23d (`jo.ra`, `jo.p`, `jo.ma`) la recopient. Le reste va dans le commit.
- Le message de commit porte le périmètre, le compte 190 → 185, le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous, `QB_MEMBERS_HASH` et les réponses à H1 à H15.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
