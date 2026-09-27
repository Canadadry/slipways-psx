---
title: "Plan 3, étape 1b.23a : `jo`, défauts et tables de classe — douze membres de la planète typés, sortis des tables de défauts de `jo` et de ses 9 filles (découpée en 1b.23a1/a2)"
description: "`pico8_jo_t` garde 31 membres `pico8_value_t`. 1b.23a en type 12 (`nu`, `hf`, `hs`, `kq`, `ka`, `dz`, `cq`, `kw`, `em`, `hn`, `nk`, `fa`) selon le patron R1–R9 de 1b.1 et la lecture R7 de 1b.14a ; les 19 autres restent `pico8_value_t` jusqu'à 1b.23b–d. `make value-members` passe de 169 à 157. Le travail est découpé en deux commits verts : 1b.23a1 (9 membres à défaut, R7 sur 8 tables de filles, `hf` à genres mêlés ; 169 → 160) puis 1b.23a2 (`kw`, `hn`, `nk`, l'écrivain par nom, les boucles sentinelles de 10 TU ; 160 → 157)."
status: needs-triage
---

## Problem Statement

Lignes de l'étape, recopiées de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.23 — `jo` (`planet.h:24`), 31 membres, empreinte 58 (26, 32), découpée ; chaque groupe retire ses membres des tables de défauts des 9 filles :
>
> 1b.23a — défauts et tables de classe : `nu`, `hf` (genres mêlés), `hs`, `kq`, `ka`, `dz`, `cq`, `kw`, `em`, `hn` (→ 1b.9), `nk`, `fa`. Empreinte 30 (14, 16). Compte : 157.

1b.23a ne type que ces 12 membres. `ra`, `p`, `gq`, `ca` sont 1b.23b ; `id`, `oe`, `ep`, `jy`, `fx`, `lq`, `jh`, `p_` sont 1b.23c ; `kr`, `cc`, `by`, `mi`, `m_`, `fo`, `ma` sont 1b.23d.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité, pas redécidé. Les notes « Découvert à » de `docs/plan-3.md` qui suivent s'appliquent aussi, citées et non redérivées :

- **1b.1** : R1 à R9 ; `ref:table` → `pico8_ob_table_t *`, `ref:pico8_X_t` → `pico8_X_t *`, `fixe` → `pico8_fixed_t`, pointeurs non `const` ; pointeur sans indicateur (NULL = nil) ; R3 « un défaut REF garde la même REF ».
- **1b.3, 1b.4, 1b.4b** : R3 lu sur ce qui initialise l'objet ; pointeur emprunté, même pointeur ; recensement corrigé (fils forkés ; boucles sentinelles `written[]` arrêtées par `PICO8_MEMBER_CENSUS_STOP()`).
- **1b.5, 1b.7, 1b.16 — lectures de R1 pour les genres mêlés**, relues pour `hf` : {fixe, bool} → `pico8_fixed_t <m>` + `int <m>_is_bool` (+ `has_<m>` si nil) et un emballage public unique `pico8_X_<m>` (1b.5, appliquée sans écart par 1b.10 à `lq.bg` « même si seul `false` est écrit ») ; {bool, chaine} à `false` seul → `const char *` + `has_` (1b.7) ; {fixe, REF} au seul fixe de la spec → pointeur, NULL = le fixe (1b.16). R7 forme `pico8_X_member` et preuves mécaniques `grep` (1b.5).
- **1b.6, 1b.8** : un lecteur qui copie n'est pas un `pico8_X_member` ; R7 sur un sélecteur local.
- **1b.9** : R7 forme `offsetof` ; `hp` typé, accesseur public `pico8_hp_struct_class()` et `pico8_hp_assert` (cible de `jo.hn`) ; messages d'aide qui nomment tous leurs membres et le genre de leur ligne.
- **1b.10** : R5 en boucle de clés, et **lecture de R5 pour `ref:autre`** : REF non NULL dont `pico8_type_tag_of` rend `UNKNOWN` (sert à `jo.kq`).
- **1b.11 — R2, TU lents** : `CENSUS_UNIT_TESTS` exclut `UNIT_TESTS_SLOW` ; chaque PRD relit ceux qui touchent sa struct (revue plus bas).
- **1b.12** : `ref:autre` reçu en argument, déballé par la lecture de 1b.10.
- **1b.13 et 1b.15 — forme écrivain de R5** : une branche `strcmp` par membre, aide `static` par genre et par struct, qui pose **d'abord** nil, sort sur `REF NULL` (genre **et** pointeur), sinon `assert` « champ sans règle » qui nomme `struct.membre` et le genre ; un non-REF à union nulle (faux, fixe 0 posés par `memset`) doit avorter ; `ref:pico8_X_t` vérifie tag `STRUCT` **et** descripteur par l'accesseur public ; un `must_abort` rouge sur l'ancien code fige l'arrêt.
- **1b.14a — R7, lecture pour une table de défauts qui garde des entrées et a des filles** : (1) l'entrée du membre typé sort de toutes les tables qui la listent, filles comprises, et chaque table garde ses autres entrées, sa boucle de recopie et sa remise à nil ; (2) chaque module qui avait l'entrée déballe **une fois, à son init**, juste après la boucle de recopie, le défaut de **sa** spec par son propre lecteur (`pico8_<X>_default`) dans un `static <type> g_<X>_<m>`, même pointeur que la spec ; (3) remise à nil dans `pico8_<X>_defaults_reset` ; (4) pose typée dans le `struct_init`/constructeur après la boucle de la table, avant clés et `pq` ; (5) les filles sans défaut propre héritent par la chaîne d'init. **1b.23a est l'application la plus large de cette lecture.**
- **1b.14b** : R7 vacante quand aucune table de fille ne liste le membre ; cas « union nulle » construits par `memset` puis `kind` ; mutation d'un appel unique d'aide `static` plantée sous `if (0)` (`-Werror`).
- **1b.17 à 1b.21** : patron sans écart ; aide par genre partagée entre écrivain et écriture directe d'une valeur (`fq_take_bool`, 1b.18) ; `ref:autre` vers une boîte → `pico8_qz_box_t *` (1b.20, `ie.kq`) ; un site C1 de `float-audit` dont l'extrait change se réécrit seul dans `tools/float_audit_c1.txt` (1b.21).

### État de départ

HEAD `172084d` (PRD de 1b.22). **1b.22 (`bv_et`) est en cours d'implémentation. Précondition : elle est commitée avant que 1b.23a commence.** Elle touche `src/pico8_probe.c` et `tests/test_pico8_g_.c`, aucun fichier de cette étape (H9). `make value-members` rend alors **169** (H1). Les numéros de ligne cités sont ceux de HEAD et peuvent glisser : chaque site est aussi désigné par son contenu.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H17).

### Lignes de recensement (`tools/member_census.txt:57-77`)

| membre | genres | nil | preuves-nil | par-nom | type retenu | indicateur | sous-étape |
|---|---|---|---|---|---|---|---|
| `nu` | `ref:table` | jamais | - | l | `pico8_ob_table_t *` | non | a1 |
| `hf` | `bool,fixe` | jamais | - | l | `pico8_fixed_t hf` + `int hf_is_bool` | non | a1 |
| `hs` | `fixe` | jamais | - | l | `pico8_fixed_t` | non | a1 |
| `fa` | `fixe` | jamais | - | l | `pico8_fixed_t` | non | a1 |
| `kq` | `ref:autre` | jamais | - | l | `pico8_qz_box_t *` | non | a1 |
| `ka` | `ref:table` | jamais | - | l | `pico8_ob_table_t *` | non | a1 |
| `dz` | `ref:table` | jamais | - | l | `pico8_ob_table_t *` | non | a1 |
| `cq` | `ref:table` | jamais | - | l | `pico8_ob_table_t *` | non | a1 |
| `em` | `ref:table` | jamais | - | l | `pico8_ob_table_t *` | non | a1 |
| `kw` | `ref:table` | observé | stat | **le** | `pico8_ob_table_t *` | non | a2 |
| `hn` | `ref:pico8_hp_t` | observé | lu | l | `pico8_hp_t *` | non | a2 |
| `nk` | `ref:table` | observé | lu | l | `pico8_ob_table_t *` | non | a2 |

12 membres : 169 − 12 = **157** (169 − 9 = 160 après a1). Les 19 autres lignes `pico8_jo_t` ne bougent pas.

- **Aucun indicateur.** Les neuf pointeurs n'en ont pas (R2 : NULL = nil). Les trois non-pointeurs `hf`, `hs`, `fa` sont nil `jamais`, sans preuve, et la revue des TU lents (plus bas) ne leur trouve aucune lecture nil par nom.
- **`kw` a `e`** : écrit par nom par `test_pico8_ga` (`:546`, une table, dans le genre) ; les boucles sentinelles sont arrêtées au recensement (1b.4b) et n'y comptent pas.
- **`hn` et `nk` n'ont que `l`**, mais l'écrivain de 1a les accepte (liste `written[]` de `pico8_jo_set_field`, « the 17 names ») : voir « R5 : l'écrivain » (G).

### Revue des TU lents (R2 de 1b.11)

Relus pour ces 12 noms : `test_pico8_jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr` (arbre `jo`) et `test_pico8_ie` (sans objet : ses `kq`/`fa` sont ceux d'`ie`, un homonyme). Constats :

- listes `members[]`/`jo_members[]` (les 31 noms) pliées par `fold_field` : présence, genre, identité ; `scalars[]` qui contient `hf`, `hs`, `fa`, toujours présents ;
- `class_refs[] = {"nu", "kq", "kw", "gq", "em"}` et `class_ref("kw")` de `test_pico8_jo` (`:216`, `:612-623`), `pico8_jo_dz() == class_ref("dz")` etc. (`:708`) : identité de pointeur face à la spec ;
- `get_ref(e, "hn") == gb…` (`hj:411`, `fr:327`), `has(bare, "kw")` ;
- `bool_state(oz, "hf") == 3 || == 2` (`os:457`) ; `write_hs` (`os:595`) est un `must_abort` (`hs` n'est pas écrit par nom) ;
- **les boucles sentinelles `written[]`** (`jo:681`, `qh:781`, `os:630`, `hj:736`, `pm:685`, `mx:742`, `nr:687`) écrivent `pico8_value_fixed(i + 1)` sur chaque nom, dont `kw`, `hn`, `nk`, puis le relisent `FIXED`.

**Aucune lecture nil par nom de `hf`, `hs` ou `fa`** (H5). R2 ne donne donc aucun indicateur. En revanche les boucles sentinelles écrivent un fixe dans `kw`, `hn`, `nk` : typés, ces trois membres avortent (voir « Boucles sentinelles »). Les mêmes boucles existent dans trois TU rapides : `test_pico8_nl` (`:589`), `test_pico8_fr` (`:556`), `test_pico8_ga` (`:687`).

### Ce que fait le cart (`pico8/lua.lua`)

- Spec de `jo` : `nu=o("jo","np","prober")`, `me="unknown"`, `hf=1,hs=1`, `fa=6`, `kq=b(-7,-7,7,7)`, `ma=0`, `ka=o(2,3,9,12)`, `dz=o(o("unhappy",c=8),…)`, `cq=o(12,36,24,25)`, `kw=o(mu=0)`, `gq=v(0,11),ca=9`, `em=o(f,3,0,-1)`.
- Défauts propres des filles sur ces 12 membres : `qh` `nu`, `hf=f` ; `nl` `nu` ; `os` `nu`, `hs=0.5` ; `hj` `nu` ; `pm` `nu` ; `ga` `nu=o()`, `hf=f`, `kq=b(0,0,-1,-1)`, `fa=3` ; `nr` `nu`, `kq=b(-4,-4,5,5)` ; `mx` `nu`. `fr` n'en a aucun (il hérite de `nr`).
- `hf` : 1 (fixe) pour `jo`, `false` pour `qh` et ses descendants et pour `ga`. Aucune autre écriture. Lu comme test de vérité (`jo:gg`, `hy`).
- `kw` : défaut = la table même de la spec, partagée, écrite **en place** (`bb`, `mx:kf`) jusqu'à ce que `jo:lz` en pose une neuve (`o(qe=…,mu=0)`).
- `hn` : écrit par `jo:lz(hp)`. `nk` : écrit par `jo:hh` (`jo.ir()<0.45 and df.ll or jo.oc()`) et par `jo:hu` (`df[hp.go]`), nil possible.

### Le code aujourd'hui

- **Typedef** (`planet.h:24-56`) : 31 `pico8_value_t` après `pico8_ke_t ke`.
- **Table de défauts de `jo`** `g_jo_defaults[]` (`planet.c:34-41`, **nom/valeur**) : `nu hf hs fa kq ma ka dz cq kw gq ca em` (13). Remplie par `pico8_jo_default` dans `pico8_jo_init`, recopiée par `*pico8_jo_member(self, name) = value` dans `pico8_jo_struct_init`, remise à nil dans `pico8_jo_defaults_reset`. `pico8_jo_default_table(name)` la lit pour les accesseurs publics `pico8_jo_dz/cq/ka/em()`.
- **Table d'offsets** `g_jo_members[]` (`planet.c:173-187`, 31 entrées) et son accesseur `pico8_jo_member` : servent au lecteur `pico8_jo_field`, à l'écrivain `pico8_jo_set_field` (`written[]`, 17 noms), à la boucle nil de `pico8_jo_struct_init` et à la boucle de clés `id`/`p`/`ra` de `pico8_jo_struct_new`.
- **Tables des 9 filles**, entrées `offsetof(…jo.<m>)` sur ces 12 membres, **14 en tout** :

| fille | table | entrées 1b.23a | fonction qui recopie |
|---|---|---|---|
| `qh` | `g_qh_defaults` (`planet_family.c`) | `nu`, `hf` | `pico8_qh_struct_init` |
| `nl` | `g_nl_defaults` | `nu` | `pico8_nl_struct_new` |
| `os` | `g_os_defaults` | `nu`, `hs` | `pico8_os_struct_new` |
| `hj` | `g_hj_defaults` | `nu` | `pico8_hj_struct_init` |
| `pm` | `g_pm_defaults` | `nu` | `pico8_pm_struct_new` |
| `ga` | `g_ga_defaults` | `nu`, `hf`, `kq`, `fa` | `pico8_ga_struct_new` |
| `nr` | `g_nr_defaults` (`planet_family_trade.c`) | `nu`, `kq` | `pico8_nr_struct_init` |
| `mx` | `g_mx_defaults` | `nu` | `pico8_mx_struct_new` |
| `fr` | `g_fr_defaults` | — | (hérite de `nr`) |

  Aucune autre table n'a d'entrée sur ces 12 noms : `grep -rnE 'offsetof\(pico8_[a-z0-9_]+_t, ([a-z0-9_]+\.)*jo\.(nu|hf|hs|kq|ka|dz|cq|kw|em|hn|nk|fa)\)' src/` rend exactement ces 14 lignes. Les tables de membres des filles (`g_qh_members`, `g_nl_members`, …) et leurs clés de construction ne listent aucun membre de `jo`.
- **Lectures directes (R6)** : `planet.c` `pico8_jo_gg` (`hf`, par `pico8_jo_present` et `pico8_jo_truthy`) ; `planet_display.c` `:103-104` et `:226`, `:268` par `table_of(self->kw)` (`kw`), `:503` (`nk`) ; `planet_trade.c:238-239` (`kw`) ; `planet_family_trade.c:481` (`mx:kf`, `kw` écrit en place).
- **Écritures directes (R6)** : `pico8_jo_lz` (`self->hn = pico8_value_ref(hp_instance)`, `self->kw = pico8_value_ref(kw)`) ; `pico8_jo_hu` et `pico8_jo_hh` (`self->nk = nk_val`, un `pico8_value_t` lu d'une table).
- **Test de régression §6 touché** : `tests/test_pico8_ga_export_panel_by.c:111`, `planet->jo.kw = pico8_value_ref(kw);` (écriture directe, que le compilateur désignera).
- **Lecteurs par nom, inchangés** (puce Coexistence) : `entity.c` (`nu` pour `cg`, `fa` pour les couches), `entity_geom.c` (`fa`, `kq`), `ui.c` (`fa` au survol), `economy_score.c` (`hf`, `hs`, `kw`), `economy_tiers.c` (`hn`, `kw`, `hf`), `economy_formulas.c` (`kw`), `research_display.c` (`nk`), et les TU.

### Coexistence : 12 membres typés, 19 non typés, dans la même struct (G)

Même forme que `ke` à 1b.14a, où `me` typé côtoyait `t` non typé, étendue à un typedef dont la table d'offsets garde des entrées :

- **Lecteur** `pico8_jo_field` : 12 branches R4 `strcmp`, **avant** l'appel à `pico8_jo_member`, dans l'ordre du typedef. Le repli reste tel quel : `pico8_jo_member` sur les 19 entrées restantes de `g_jo_members[]`, puis `pico8_ke_field`. Les lecteurs des filles délèguent déjà à celui de `jo` pour ces noms (aucun ne les connaît) et ne changent pas.
- **Écrivain** `pico8_jo_set_field` : trois branches R5 (`kw`, `hn`, `nk`) **avant** la boucle `written[]`, qui garde les 14 autres noms et son `*pico8_jo_member(self, name) = value`. **Piège** : un nom de `written[]` sans entrée dans `g_jo_members[]` ferait déréférencer NULL. Les trois noms sortent de `written[]` dans le même commit que leurs entrées (a2), et a1 n'en retire aucun.
- **Tables** : `g_jo_members[]` perd les 12 entrées et garde les 19 autres. `g_jo_defaults[]` perd `nu hf hs fa kq ka dz cq em` (a1) puis `kw` (a2), et garde `ma`, `gq`, `ca` avec sa boucle de recopie et sa remise à nil (R7 de 1b.14a, point 1). Les 8 tables de filles perdent leurs 14 entrées et gardent toutes les autres.
- **Init** : `pico8_jo_struct_init` garde sa boucle nil sur les 19 entrées et sa boucle de défauts sur les entrées restantes, puis pose les 12 membres typés par affectation (R3) : 10 depuis les `static` déballés, `hn = NULL`, `nk = NULL`. Chaque fille à défaut propre pose les siens après sa propre boucle (R7 point 4).
- **Boucles génériques** `*(pico8_value_t *)((char *)self + off)` des filles : une fois les 14 entrées retirées, aucune n'atteint plus un membre typé (preuve `offsetof` plus bas).
- **Sites par nom** de `src/` et `tests/` : inchangés, sauf les tableaux `written[]` des boucles sentinelles (voir plus bas).

### Empreinte remesurée ; découpage

Les deux greps de la puce **Liste** redonnent **31 (15, 16)** sur HEAD (la ligne dit 30 (14, 16)). Ce sont des majorants : `fa` et `nu` sont des noms partagés (`mh`, `ie`, `pj`, `ui`, `js`, `bg`, `mz`, `jc`, `g_`), et `particle.c`, `score_panel.c`, `wormhole.c` n'y figurent que par homonymie.

**Empreinte réelle** (fichiers qui lisent ou écrivent l'un des 12 membres d'une struct de l'arbre `jo`) : **27 (13, 14)**.

- **src (13)** : `planet.h`, `planet.c`, `planet_family.c`, `planet_family_trade.c`, `planet_display.c`, `planet_trade.c`, `economy_score.c`, `economy_tiers.c`, `economy_formulas.c`, `entity.c`, `entity_geom.c`, `ui.c`, `research_display.c`.
- **tests (14)** : `test_pico8_jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`, `nl`, `fr`, `ga`, `hp`, `ga_export_panel_by`, `rich_planet_exports`, `jo_spec_defaults`.

**Fichiers modifiés : 18**, plus `docs/plan-3.md` :

- **src (6)** : `pico8_planet.h`, `pico8_planet.c`, `pico8_planet_family.c`, `pico8_planet_family_trade.c`, `pico8_planet_display.c`, `pico8_planet_trade.c` ;
- **tests (12)** : `test_pico8_ui.c` (ajouts : caractérisation, test d'arrêt) ; les tableaux `written[]` de `test_pico8_jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr` (**7 TU lents**), `nl`, `fr`, `ga` ; une ligne de `test_pico8_ga_export_panel_by` (**test de régression §6**).

**Découpage proposé (G) : 1b.23a1 puis 1b.23a2, deux commits verts, un seul PRD.** Aucun commit de 1b.1 à 1b.22 n'a dépassé 11 fichiers modifiés. Celui-ci en fait 18, dont 7 TU lents et un test de régression, avec deux formes distinctes : R7 sur huit tables de filles, et R5 écrivain avec les boucles sentinelles. Ces deux formes ne partagent aucun site : le découpage est naturel et chaque moitié compile seule, puisque les membres sont indépendants.

| sous-étape | membres | compte | fichiers modifiés | règles exercées | TU lents modifiés |
|---|---|---|---|---|---|
| **1b.23a1** — défauts | `nu`, `hf`, `hs`, `fa`, `kq`, `ka`, `dz`, `cq`, `em` (9) | 169 → **160** | 5 : `planet.h`, `planet.c`, `planet_family.c`, `planet_family_trade.c`, `test_pico8_ui.c` | R7 de 1b.14a sur `jo` et 8 filles (14 entrées) ; R1 genres mêlés (`hf`) ; R4 | aucun |
| **1b.23a2** — écrits | `kw`, `hn`, `nk` (3) | 160 → **157** | 16 : `planet.h`, `planet.c`, `planet_display.c`, `planet_trade.c`, `planet_family_trade.c`, `test_pico8_ui.c`, 10 TU à boucle sentinelle, `test_pico8_ga_export_panel_by.c` | R5 écrivain (1b.13/1b.15), R6 directes, R7 (`kw` sort de `g_jo_defaults`) | 7 |

La caractérisation couvre les **12** membres. Elle est écrite et figée **une fois**, sur l'arbre de départ, avant a1. C'est pour ça qu'on garde un seul PRD plutôt que deux : le même hash tient à travers les deux commits. Écarté (G) : un seul commit, trop large pour une passe et une vérification sous budget ; un découpage par fille, qui laisserait `hf`/`nu` à moitié typés, ce qui est impossible (un membre change de type pour tous ses sites à la fois).

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** : les 12 membres lus par nom sur `jo` et ses 9 filles, plus les planètes vivantes d'une partie, les transitions `lz`/`hh`/`hu` et les écritures par nom dans le genre (voir Testing Decisions). Verte sur l'arbre de départ, avant tout typage.
2. **Commit 1b.23a1** : `nu`, `hf`, `hs`, `fa`, `kq`, `ka`, `dz`, `cq`, `em` typés. Leurs entrées sortent de `g_jo_defaults`, de `g_jo_members` et des 14 lignes des tables de filles. Chaque module déballe une fois ses défauts dans des `static` et les pose (R7 de 1b.14a). Emballage `pico8_jo_hf`. Les accesseurs `pico8_jo_dz/cq/ka/em()` rendent les `static`. `make value-members` : 160.
3. **Test d'arrêt, rouge sur l'arbre a1** (l'écrivain accepte tout), vert après a2.
4. **Commit 1b.23a2** : `kw`, `hn`, `nk` typés. Écrivain R5 par deux aides, écritures et lectures directes réécrites, `kw` hors de `g_jo_defaults`, les trois noms hors des boucles sentinelles de 10 TU, ligne `kw` du test de régression. `make value-members` : **157**.
5. Aucun pixel ne bouge. Les notes « Découvert à 1b.23a1 » et « à 1b.23a2 » vont sur leurs lignes, écrites par le commit a1 à la place de la ligne 1b.23a (voir Further Notes).

## User Stories

1. En tant que lecteur de `planet.h`, je veux `nu`, `ka`, `dz`, `cq`, `kw`, `em`, `nk` typés `pico8_ob_table_t *`, `kq` `pico8_qz_box_t *`, `hn` `pico8_hp_t *`, et `hs`, `fa` `pico8_fixed_t`, afin de lire le genre sur le type.
2. En tant que lecteur de `planet.h`, je veux `hf` typé `pico8_fixed_t hf` avec `int hf_is_bool`, afin que « 1 » (le fixe de `jo`) et `false` (celui de `qh` et de `ga`) restent deux genres distincts.
3. En tant que lecteur par nom (`entity.c`, `entity_geom.c`, `ui.c`, `economy_*`, `research_display.c`, les TU), je veux que chacun des 12 noms rende exactement ce que rendait 1a sur les 10 genres de l'arbre `jo` : présence, genre, valeur, identité du pointeur.
4. En tant que `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `mx`, je veux recevoir le **pointeur même** de ma spec pour `nu`, `kq`, `ka`, `dz`, `cq`, `kw`, `em`, afin que `STRUCT_HASH` et les `class_ref` des TU ne bougent pas.
5. En tant que `fr`, je veux hériter de `nu` et `kq` de `nr` par la chaîne d'init, sans table touchée.
6. En tant que `os`, je veux `hs` = 0,5 ; en tant que `ga`, `fa` = 3, `hf` = `false`, `kq` ma boîte, `nu` ma table vide.
7. En tant que `jo:gg`, je veux lire `hf` par `pico8_jo_hf` et rendre `false` tel quel quand il l'est, sinon le booléen `me == "ly"`, comme en 1a.
8. En tant que `jo:gh`, `jo:cn`, `mx:kf`, `jo:kj`, je veux lire et écrire en place `kw` et `nk` par le pointeur, avec la même table de vérité que le test de genre de 1a.
9. En tant que `jo:lz`, je veux poser `hn` et un `kw` neuf par affectation typée.
10. En tant que `jo:hh` et `jo:hu`, je veux que `nk`, lu d'une table (`df`, `jo.oc()`), soit déballé par la même aide que l'écrivain, nil accepté, autre genre refusé.
11. En tant qu'écrivain par nom (`test_pico8_ga`, tout code futur), je veux `kw`, `hn`, `nk` acceptés dans leur genre (table, `hp`) ou nil, même pointeur, et tout autre genre arrêté par un `assert` « champ sans règle » qui nomme `jo.<m>` et le genre de sa ligne.
12. En tant que mainteneur, je veux que l'ensemble des noms qu'accepte l'écrivain de `jo` ne change pas (17 noms).
13. En tant que mainteneur, je veux qu'aucune table de défauts, d'offsets ou de clés ne liste plus ces 12 noms, et qu'aucune écriture `*(pico8_value_t *)` ne les atteigne.
14. En tant que `pico8_jo_dz/cq/ka/em()`, je veux rendre le même pointeur qu'avant, NULL hors de `init..shutdown`.
15. En tant que TU lent ou rapide à boucle sentinelle, je veux que ma boucle ne réécrive plus en fixe un membre typé, et que l'écriture de ces trois noms soit figée ailleurs, une fois.
16. En tant que test de régression `ga_export_panel_by`, je veux écrire `kw` par le pointeur et rester rouge si le bug `by=o()` revient.
17. En tant qu'auteur de 1b.23b–d, 1b.24–1b.35, je veux une forme écrite pour les boucles sentinelles et pour une branche d'écrivain `l` seul déjà présente en 1a.
18. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché, après **chacun** des deux commits.
19. En tant que gardien de la mémoire et de la couverture, je veux `make sanitize-net` à 15 propres et `make class-coverage-net` identique à sa référence.
20. En tant que `plan-pilot`, je veux `make value-members` à 160 puis 157.
21. En tant que vérificateur, je veux la caractérisation verte sur l'ancien code, le test d'arrêt rouge puis vert, les mutations du §7 et leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés comme le ferait un agent `guideline` (hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables), et marqués (G).

### R1–R9 appliquées

| règle | sur les 12 membres |
|---|---|
| R1 | Voir le tableau de recensement. Nom et place dans le typedef gardés. `hf_is_bool` est déclaré juste après `hf`. Tous empruntés, jamais possédés ni libérés : spec, `pico8_gb()`, `df`, table neuve de `lz` (qui la laisse vivre comme en 1a, TROU-7). |
| R2 | Aucun `has_` (tableau et revue des TU lents). |
| R3 | Affectations typées à la place des anciennes, avant clés et `pq`. `jo` : 10 défauts depuis ses `static`, `hn = NULL`, `nk = NULL`. Filles : voir R7. Défauts REF = **même pointeur** que la spec. |
| R4 | 12 branches dans `pico8_jo_field`, ordre du typedef, avant `pico8_jo_member`. Pointeur NULL → `return 0`, `*out_value` intact ; sinon `pico8_value_ref(<m>)`. `hs`, `fa` → `pico8_value_fixed`, toujours 1. `hf` → `pico8_jo_hf(self)`, toujours 1. |
| R5 | Écrivain : voir plus bas (a2). Pas de boucle de clés touchée (`id`/`p`/`ra` restent non typés). |
| R6 | Lectures et écritures directes du compilateur (liste « Le code aujourd'hui »). `x.kind == REF && x.as.ref != NULL` → `x != NULL` ; `(const pico8_ob_table_t *)x.as.ref` → `x`. `table_of` de `planet_display.c` n'a plus d'appelant : il est supprimé (a2). `pico8_jo_default_table` aussi (a1). |
| R7 | Lecture de 1b.14a, voir plus bas. |
| R8 | Crochets `PICO8_MEMBER_CENSUS_*` gardés. `tools/member_census.txt` non édité. |
| R9 | 169 → 160 (a1) → 157 (a2). Restent 19 lignes `pico8_jo_t` : `ma gq ca id p ra oe ep jy fx lq fo jh p_ kr cc by mi m_`. |

### `hf` : lecture de R1 de 1b.5, sans indicateur (G)

Genres `{bool, fixe}`, nil `jamais`. Valeurs tenues : fixe 1 (spec de `jo`) et `false` (specs de `qh` et de `ga`) ; aucun autre écrivain.

- **Retenu** : la forme de 1b.5, que 1b.10 a déjà appliquée à `lq.bg` quand seul `false` est écrit : `pico8_fixed_t hf; int hf_is_bool;` (1 : booléen 0/1 dans `hf` ; 0 : un fixe). **Sans `has_hf`**, puisque R2 n'en donne pas à un nil `jamais` sans lecture nil. `hf_is_bool` est donc lu toujours, et pas « seulement si `has_` ».
- **Emballage public unique** `pico8_value_t pico8_jo_hf(const pico8_jo_t *self)` (déclaré dans `planet.h`) : `self` NULL → `REF NULL` ; `hf_is_bool` → `pico8_value_bool(hf != 0)` ; sinon `pico8_value_fixed(hf)`. Il est appelé par le lecteur et par `pico8_jo_gg`.
- **`pico8_jo_gg`** : `if (self == NULL) return nil; hf_val = pico8_jo_hf(self); if (!pico8_jo_truthy(hf_val)) return hf_val;` puis le reste inchangé. Le test `pico8_jo_present(self->hf)` disparaît, parce que `hf` n'est jamais nil. C'est la même table de vérité.
- Écartés : la lecture 1b.7 (`false` porté par NULL), impossible pour un fixe (précédent 1b.10) ; la lecture 1b.16 (« NULL = le fixe de la spec »), qui ne s'applique qu'à un pointeur et serait un nouvel encodage sans nécessité.

### `hn` → `pico8_hp_t *` (G)

La cible est typée depuis 1b.9, la ligne a un seul genre et 1b.9 a fourni `pico8_hp_struct_class()` et `pico8_hp_assert`. `jo:lz` écrit déjà un `pico8_hp_t *` vérifié par `pico8_hp_assert` : son écriture devient `self->hn = hp_instance`. Pas d'indicateur (pointeur). `economy_tiers.c` lit par nom (`field_ref_instance`), inchangé.

### `kq` → `pico8_qz_box_t *` (G)

`ref:autre` vers `b(…)`, précédent 1b.20 (`ie.kq`). Défaut déballé selon la lecture 1b.10 de `ref:autre` (REF non NULL, `pico8_type_tag_of` = `UNKNOWN`). `entity_geom.c` lit par nom et caste, inchangé.

### R7 : lecture de 1b.14a sur `jo` et 8 filles (a1 ; `kw` en a2)

1. **Retrait** des entrées : 9 (a1) puis 1 (a2) de `g_jo_defaults` ; 12 de `g_jo_members` (9 en a1, 3 en a2) ; les 14 lignes du tableau des filles (toutes en a1, aucune fille n'a `kw`, `hn` ni `nk`). Les tables gardent leurs autres entrées, leurs boucles et leurs remises à nil.
2. **Déballage une fois, à l'init du module, juste après la boucle de recopie de la table**, par le lecteur de spec du module, dans un `static` typé :
   - `planet.c`, dans `pico8_jo_init` : `g_jo_nu`, `g_jo_hf` + `g_jo_hf_is_bool`, `g_jo_hs`, `g_jo_fa`, `g_jo_kq`, `g_jo_ka`, `g_jo_dz`, `g_jo_cq`, `g_jo_em` (a1), `g_jo_kw` (a2), à côté de `g_jo_me` ;
   - `planet_family.c`, dans `pico8_planet_family_init`, après la boucle de chaque table : `g_qh_nu`, `g_qh_hf` + `g_qh_hf_is_bool`, `g_nl_nu`, `g_os_nu`, `g_os_hs`, `g_hj_nu`, `g_pm_nu`, `g_ga_nu`, `g_ga_hf` + `g_ga_hf_is_bool`, `g_ga_kq`, `g_ga_fa` ;
   - `planet_family_trade.c`, dans son init : `g_nr_nu`, `g_nr_kq`, `g_mx_nu`.
3. **Aide de lecture de spec par genre et par fichier (G, écart de forme avec 1b.14a)**. 1b.14a écrivait l'`assert` en ligne parce qu'il y en avait trois. Ici il y en a 24, répartis sur trois fichiers. On prend donc une aide `static` par genre et par fichier, jamais partagée entre fichiers (1b.13 point 2), appelée `found = pico8_<X>_default("<m>", &v)` puis `g_<X>_<m> = <fichier>_default_<genre>(found, v)`. Chaque aide lève un `assert` dont le message nomme les membres qu'elle sert et le genre de leur ligne (forme des messages de 1b.9/1b.18). Par exemple `"jo.nu/ka/dz/cq/kw/em: a default of the struct is a key of the spec and a table (ref:table)"` : table = REF non NULL, tag `OB_TABLE` ; boîte = REF non NULL, tag `UNKNOWN` ; fixe = `FIXED` ; `hf` = `FIXED` ou `BOOL`, qui remplit `hf` et `hf_is_bool`. Aides attendues : `planet.c` table, box, fixed, hf ; `planet_family.c` table, fixed, box, hf ; `planet_family_trade.c` table, box. Si une aide n'a qu'un appelant, elle reste une aide : même forme dans les trois fichiers.
4. **Remise à nil** de chaque `static` (NULL, 0, `is_bool` 0) dans le `pico8_<X>_defaults_reset` du module, que le `shutdown` appelle déjà. `g_nl`, `g_os`, `g_hj`, `g_ga`, `g_nr`, `g_mx` reçoivent leur premier `static` : leur `defaults_reset` existe déjà.
5. **Pose typée** après la boucle de la table, avant clés et `pq` : `pico8_jo_struct_init` (10 défauts, puis `hn = NULL`, `nk = NULL`) ; `pico8_qh_struct_init` (`jo.nu`, `jo.hf`/`hf_is_bool`, à côté de `me`) ; `pico8_nl_struct_new` ; `pico8_os_struct_new` (`nu`, `hs`) ; `pico8_hj_struct_init` ; `pico8_pm_struct_new` (à côté de `me`) ; `pico8_ga_struct_new` (`nu`, `hf`, `kq`, `fa`, avant la boucle de clés) ; `pico8_nr_struct_init` (`nu`, `kq`) ; `pico8_mx_struct_new`. `fr` n'écrit rien.
6. **Accesseurs publics** `pico8_jo_dz/cq/ka/em()` : ils rendent `g_jo_dz` etc. Contrat inchangé (même pointeur, NULL hors de `init..shutdown`). `pico8_jo_default_table` est supprimée (plus d'appelant, `-Werror`).
7. **Commentaires** (règle de `CLAUDE.md`) : la phrase de `g_jo_defaults` sur « `kw` écrit en place » va à `g_jo_kw`. Aucun commentaire de récit de chantier n'est ajouté.

Écarté (G) : un littéral C à la place du pointeur de spec (identité perdue, rouge sur `STRUCT_HASH` et `class_ref`) ; garder les entrées et déballer à la recopie (une table de défauts ne liste que des `pico8_value_t`, puce Coexistence).

### R5 : l'écrivain (a2)

- **Branches** dans `pico8_jo_set_field`, ordre du typedef, avant la boucle : `kw` → `jo_take_table(value, &jo->kw)` ; `hn` → `jo_take_hp(value, &jo->hn)` ; `nk` → `jo_take_table(value, &jo->nk)` ; chacune `return 1`. `written[]` garde les 14 autres noms. Un nom inconnu rend toujours 0 (arrêt de `pico8_entity_set_field`, message inchangé).
- **Lecture nouvelle du point 1 de 1b.13 (G)** : `hn` et `nk` n'ont que `l`, mais l'écrivain de 1a les accepte. Leur branche est **gardée**, déballée par l'aide de leur genre. Le point 1 (« un membre `l` seul n'a pas de branche ») a été écrit pour `jc_too_far.qs`, qui n'en avait pas en 1a. Le retirer ici changerait l'ensemble des noms qu'accepte l'écrivain (arrêt au lieu d'écriture), ce que la puce Coexistence interdit. Aucun écrivain de `src/` n'écrit ces deux noms par nom (H2).
- **Aides** `static` de forme 1b.13/1b.15 dans `planet.c` :
  - `static void jo_take_table(pico8_value_t value, pico8_ob_table_t **out)` : `*out = NULL` ; `return` si `value` est `REF NULL` ; sinon `assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_OB_TABLE && "jo.kw/nk: a write neither nil nor a table is off its census row (ref:table) -- a field under no rule of plan-3 section 5: stop, do not widen the member")`, puis `*out = value.as.ref`. Elle est **partagée** avec les écritures directes de `nk` dans `pico8_jo_hh` et `pico8_jo_hu` (qui reçoivent un `pico8_value_t` lu d'une table, précédent 1b.18).
  - `static void jo_take_hp(pico8_value_t value, pico8_hp_t **out)` : même forme, `assert` tag `STRUCT` **et** `->cls == pico8_hp_struct_class()`, message `"jo.hn: … (ref:pico8_hp_t) …"`.
- **`kw` dans `jo:lz`** : `self->kw = kw` (table neuve, typée). **`hn` dans `jo:lz`** : `self->hn = hp_instance`.
- Nil accepté partout (pointeurs). Un non-REF à union nulle (`memset` puis `kind`), une chaîne NULL, une fonction NULL avortent.

### Boucles sentinelles : lecture nouvelle (G)

Les boucles `written[]` de 10 TU (`jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`, `nl`, `fr`, `ga`) écrivent un fixe sur chaque nom de l'écrivain et le relisent `FIXED`. Ce sont des écritures de test que le cart ne fait jamais (1b.4b). Elles ne sont pas des tests de régression (§6), n'ont pas de hash, et `PICO8_MEMBER_CENSUS_STOP()` les ignore déjà.

- **Retenu** : dans a2, `"kw"`, `"hn"` et `"nk"` sortent de chacun des 10 tableaux `written[]`, et **rien d'autre ne change** dans ces fichiers. L'intention (« l'écrivain prend ces noms et les rend ») est reportée, une fois pour les 10 genres, dans la caractérisation (écritures dans le genre, nil, relecture) et dans le test d'arrêt (hors genre). Le compte « takes exactly N names » des commentaires reste vrai : l'écrivain prend toujours ces noms.
- Écarté : écrire dans la boucle une valeur du genre de chaque membre typé. Il faudrait une table de genres par nom dans 10 fichiers, reprise à chaque étape de 1b.23b à 1b.35.
- **À recopier** par 1b.23b–d, 1b.24, 1b.32–1b.35 : un membre typé sort des boucles sentinelles `written[]`, son écriture par nom est figée par la caractérisation de l'étape.

### Test de régression `ga_export_panel_by` (§6, a2)

`planet->jo.kw = pico8_value_ref(kw);` devient `planet->jo.kw = kw;`. C'est la seule ligne changée, et l'intention ne change pas (le contrat « `by` est une vraie liste »). Vérification §6 dans le même commit : on retire localement le bloc d'échange de `by` de `pico8_planet_family_init`, le test doit être rouge ; on restaure et il repasse vert. Deux lignes « Runs ».

### Preuves (sortie attendue sur l'arbre final ; a1 entre parenthèses quand elle diffère)

- `grep -rnE 'offsetof\(pico8_[a-z0-9_]+_t, ([a-z0-9_]+\.)*(nu|hf|hs|fa|kq|ka|dz|cq|kw|em|hn|nk)\)' src/ | grep -E 'pico8_(jo|qh|nl|os|hj|pm|ga|nr|fr|mx)_t'` : vide (26 lignes avant : 12 de `g_jo_members`, 14 des filles ; a1 : les 3 de `kw`/`hn`/`nk` dans `g_jo_members`) ;
- `grep -nE '\{"(nu|hf|hs|fa|kq|ka|dz|cq|kw|em)", \{0\}\}' src/pico8_planet.c` : vide (a1 : `kw`) ;
- `grep -nE '(\.|->)(nu|hf|hs|fa|kq|ka|dz|cq|kw|em|hn|nk)\.(kind|as)' src/pico8_planet*.c` : vide ;
- `grep -rn 'pico8_jo_default_table\|table_of(' src/pico8_planet*.c` : vide (a1 : les 3 `table_of` de `planet_display.c`) ;
- `grep -c 'off its census row' src/pico8_planet.c` : 2 (a1 : 0) ;
- `grep -n 'pico8_value_t \*' src/pico8_planet.c | grep -v 'pico8_value_t \*out_value)'` : seulement `pico8_jo_member` et la déclaration du champ `value` de `g_jo_defaults` (tables gardées pour les 19 membres restants) ;
- `make value-members | grep -c 'pico8_jo_t'` : 19 (a1 : 22) ;
- `grep -rn '"me"\|"hn"\|"nk"\|"kw"' src/ | grep -v '^src/pico8_planet'` : mêmes lignes qu'avant (sites par nom intacts) ;
- `git diff --stat` des 10 TU à boucle sentinelle : au plus 2 lignes changées chacun, toutes dans `written[]`.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 169 sur l'arbre de départ (1b.22 commitée), 160 après a1, 157 après a2. Seules des lignes `pico8_jo_t` disparaissent.
- **H2** : dans les 15 rejeux et dans tous les TU hors fils de `must_abort`, les aides ne voient que leur genre ou nil : `nk` n'est écrit que d'une table de `df`, du générateur `oc` ou de nil ; `kw` que d'une table ; `hn` que d'un `hp`. Aucun code de `src/` n'écrit `kw`, `hn` ni `nk` par nom.
- **H3** : les tables de spec (`o(…)`), celles de `df` et les valeurs de `pico8_gn_next` portent le tag `PICO8_TYPE_TAG_OB_TABLE` ; `pico8_gn_next` peut rendre `REF NULL` (lu nil).
- **H4** : les boîtes `b(…)` ont le tag `UNKNOWN` et ne sont pas NULL.
- **H5** : aucun TU lent ne lit `hf`, `hs` ou `fa` d'une struct de l'arbre `jo` nil par nom (revue ci-dessus), et aucun rejeu ne les voit absents.
- **H6** : aucune valeur figée ne bouge : `STRUCT_HASH`, `SPEC_HASH`, `SHARE_HASH`, `READY_HASH` de `test_pico8_jo_spec_defaults` ; `STRUCT_HASH` de `test_pico8_spec_defaults` ; les hash `fold_field` des TU de l'arbre `jo` (qui plient genre et identité de `hf`, `nu`, `kq`, `kw`…) ; `DRAG_REPLAY_HASH`, `TECH_REPLAY_HASH`, `KE_ME_HASH`, `KE_T_HASH` ; les 15 empreintes du filet.
- **H7** : le build `-DPICO8_MEMBER_CENSUS` compile. Il n'est pas lancé.
- **H8** : `test_pico8_ui` reste sous 30 s avec trois fonctions de plus (hors `UNIT_TESTS_SLOW`).
- **H9** : 1b.22 ne touche ni `planet*.c`, ni `planet.h`, ni aucun TU de cette étape.
- **H10** : rien ne dépend de `sizeof(pico8_jo_t)` ni de la disposition des filles (`hf_is_bool` s'ajoute, les membres rétrécissent). Tous les offsets sont recalculés par `offsetof`, et aucun binaire ne sérialise une entité.
- **H11** : après `start_game(DRAG)`, `pico8_hv()` contient des planètes dont `nk` est posé par `hh` et `hn` absent. Les valeurs figées sont celles **observées** sur l'ancien code.
- **H12** : `pico8_jo_hh` et `pico8_jo_hu` sont appelables sur un `jo` neuf après `start_game(DRAG)` (`oc`/`ir` posés par `galaxy.c`, `df` et `gb` chargés).
- **H13** : aucun site C1 de `tools/float_audit_c1.txt` ne cite ces membres (grep vide). Si un extrait change quand même, seule sa ligne est réécrite (précédent 1b.21).
- **H14** : `test_pico8_ga_export_panel_by` est rouge quand l'échange de `by` est retiré.
- **H15** : les lignes modifiées de `planet_display.c`, `planet_trade.c` et `planet_family_trade.c` (`mx:kf`) sont exécutées par `test_pico8_ui` (rejeux `DRAG`/`TECH`) ou, à défaut, par une passe `gcov` de `test_pico8_hp`, `test_pico8_rich_planet_exports` ou `test_pico8_ga_export_panel_by`.
- **H16** : l'arbre a1 compile sans toucher un TU : aucun TU n'accède directement à `nu`, `hf`, `hs`, `fa`, `kq`, `ka`, `dz`, `cq` ou `em` (grep vide, seul `jo.kw` de `ga_export_panel_by` est direct).
- **H17** : retirer trois noms des boucles sentinelles ne change aucune autre assertion de ces TU (ces boucles n'ont que des `assert`, pas de hash).

## Implementation Chunks

1. **Caractérisation** (`test_jo_members_by_name` dans `test_pico8_ui`), verte sur l'arbre de départ, `JO_MEMBERS_HASH` relevé. Pas encore de commit.
2. **1b.23a1**, commit vert : typage des 9 membres à défaut (typedef, R4, R7 dans les trois fichiers `planet*.c`, `pico8_jo_hf`, `pico8_jo_gg`, accesseurs, suppression de `pico8_jo_default_table`), avec la caractérisation. La ligne 1b.23a de `docs/plan-3.md` devient l'en-tête d'un découpage, plus la ligne 1b.23a1 et sa note « Découvert à ». Cibles : `make test`, `make e2e`, `make value-members` (160).
3. **Test d'arrêt** (`test_jo_writer_stops_off_row`), rouge sur l'arbre a1.
4. **1b.23a2**, commit vert : typage de `kw`, `hn`, `nk` (typedef, R4, R5 et aides, R6 dans `planet_display.c`, `planet_trade.c`, `planet_family_trade.c`, `table_of` supprimé, `kw` hors de `g_jo_defaults`), les 10 tableaux `written[]`, la ligne `kw` de `ga_export_panel_by` et sa vérification §6, le test d'arrêt, la ligne 1b.23a2 et sa note. Cibles finales complètes (voir Vérification).

Chaque commit est vert à lui seul (§3). a2 ne commence qu'avec a1 commité.

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom, ou l'écran.** Le nouveau test n'accède à aucun des 12 membres directement : il compile sur les deux formes. La seule ligne directe touchée est celle du test de régression, désignée par le compilateur.
- **Caractérisation dirigée, figée sur l'ancien code** : `test_jo_members_by_name`, dans `test_pico8_ui` (rapide, dans `CENSUS_UNIT_TESTS`, lie `ENGINE_SRCS`, a `start_game`, `must_abort` et `in_fresh_child`, précédent 1b.14a). Lancée par `in_fresh_child` après les tests de `ke`. Elle lit par le **lecteur brut du descripteur** (`cls->field`, témoin `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder sur le retour, le genre, la valeur et le pointeur.
  1. **Les 10 genres.** `start_game(DRAG)`, puis `pico8_<X>_new(NULL)` pour `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`. Pour chacun et pour chacun des 12 noms, on relève la présence, le genre, la valeur (fixe ou booléen) et une **classe d'identité** du pointeur : 1 si c'est celui de `pico8_jo_default(<m>)`, 2 celui du lecteur de spec de la fille qui a un défaut propre (`pico8_qh/nl/os/hj/pm/ga/nr/mx_default`), 0 sinon. Valeurs attendues (lecture du code, `assert` explicites) : `hf` `FIXED` 1 pour `jo`, `BOOL` 0 pour les 9 autres ; `hs` 1, et 0,5 pour `os` ; `fa` 6, et 3 pour `ga` ; `nu` classe 2 partout sauf `jo` (1) et `fr` (celui de `nr`) ; `kq` classe 1 sauf `ga`, `nr`, `fr` (celui de `nr`) ; `ka`, `dz`, `cq`, `em`, `kw` classe 1 partout ; `hn` absent ; `nk` absent. Si un `pq` change une valeur sur l'ancien code, c'est la valeur **observée** qui est figée, et l'écart est noté (H11).
  2. **Accesseurs** : `pico8_jo_dz/cq/ka/em()` égaux au pointeur de `pico8_jo_default` du même nom.
  3. **Intrus** sur le lecteur de `jo` : `""`, `"n"`, `"nuu"`, `"hf "`, `"hf_is_bool"`, `"has_hf"`, `"jo"` → 0, témoin intact.
  4. **Transitions** sur un `jo` et un `pm` neufs : `pico8_jo_lz(e, <hp de pico8_gb()>)` → `hn` même pointeur, `kw` présent, table distincte de la spec, `mu` 0 ; `pico8_jo_hh(e)` → `nk` présent ou absent tel qu'observé, `ep` inchangé dans sa forme ; `pico8_jo_hu` avec un projet à `go` (le premier de `gb` qui en a un) → `nk` = l'entrée de `df`, même pointeur.
  5. **Écritures par nom dans le genre** (sur `jo`, `qh`, `pm`, `ga`, `fr`) : `kw` ← table locale T → même pointeur T ; `kw` ← nil → absent ; `hn` ← un `hp` de `gb` → même pointeur ; `hn` ← nil → absent ; `nk` ← table T → même T ; `nk` ← nil → absent. Chaque écrivain rend 1.
  6. **Planètes vivantes** : on parcourt `pico8_hv()` après `start_game(DRAG)`, **avant** les constructions de 1. Pour chaque entité de l'arbre `jo`, on plie présence, genre et classe d'identité des 12 noms, plus le nombre d'entrées.
  7. Tout est plié dans **`JO_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ. Les adresses sont jugées par `assert`. Le hash ne change plus ensuite, ni après a1, ni après a2.
- **Test d'arrêt, rouge d'abord** (précédent 1b.13/1b.15) : `test_jo_writer_stops_off_row`, par `in_fresh_child` après la caractérisation. `must_abort` sur `pico8_entity_set_field(e, n, v)`, avec `e` un `jo` neuf puis un `pm` neuf (l'écrivain passe par les délégations `pm` → `hj` → `qh` → `jo`), pour `n` ∈ {`kw`, `hn`, `nk`} et `v` ∈ {fixe 1, `pico8_value_bool(1)`, chaîne `"x"`, **fixe 0 à union nulle** (`memset`, `kind = FIXED`), **faux à union nulle** (`memset`, `kind = BOOL`), **chaîne NULL** (`memset`, `kind = STRING`), **fonction NULL** (`memset`, `kind = FUNCTION`), REF vers une `pico8_list_t`, REF vers `e` lui-même (une struct)}, plus pour `hn` une REF vers une `ob_table` et pour `kw`/`nk` une REF vers un `hp`. **Rouge sur l'arbre a1** (l'écrivain de 1a accepte tout, le fils sort par `_exit(0)`), vert après a2 : une ligne « Runs » pour chacun. Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Tests existants qui figent déjà beaucoup** : `test_pico8_jo_spec_defaults` (`STRUCT_HASH`, identité face à la spec pour 10 genres), les huit TU lents (`fold_field`, `class_ref`, `pico8_jo_dz() == class_ref("dz")`), `test_pico8_hp` (`hn`, `kw`, `nk` par nom après placement), `test_pico8_rich_planet_exports`, `test_pico8_planet_gh_row_lq`, `test_pico8_ga` (`:546`).
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_ui && ./tmp/test_pico8_ui` (ou le TU cité, plus rapide). Chacune est notée dans le commit qui la porte, avec le test qui la rougit ou la justification de son trou. Un appel unique d'aide `static` se retire sous `if (0)` (1b.14b).
  1. (§7-1) *indicateur forcé vrai* : **sans objet dans sa lettre**, aucun indicateur. Forme la plus proche (a2) : la branche R4 de `hn` rend 1 avec `REF NULL` quand `hn` est NULL. Attendu rouge : caractérisation 1 (`hn` absent).
  2. (§7-2) *« présent 0 » là où c'était absent* : **sans objet dans sa lettre**, puisque `hs`, `fa`, `hf` ne sont jamais absents. Forme voisine (a2) : la branche de `nk` rend `pico8_value_fixed(0)` quand `nk` est NULL. Attendu rouge : caractérisation 1 et 5.
  3. (§7-3) *init qui saute un membre typé*, une par module. Forme « pose 0/NULL à la place du `static` », parce que l'omission pure laisse de la mémoire de `malloc` non déterministe (**trou justifié** de la forme littérale, précédent 1b.14a) :
     - (a, a1) `pico8_jo_struct_init` pose `hs = 0` → rouge : caractérisation 1, `STRUCT_HASH` ;
     - (b, a1) `pico8_qh_struct_init` omet `hf`/`hf_is_bool` (il garde le fixe 1 de `jo`) → rouge : caractérisation 1 (9 genres), `STRUCT_HASH` ;
     - (c, a1) `pico8_os_struct_new` omet `hs` (il garde 1 au lieu de 0,5) → rouge : caractérisation 1 ;
     - (d, a1) `pico8_ga_struct_new` omet `kq` (il garde la boîte de `jo`) → rouge : caractérisation 1, `STRUCT_HASH` ;
     - (e, a1) `pico8_nr_struct_init` omet `nu` → rouge : caractérisation 1 (`nr`, `fr`) ;
     - (f, a2) `pico8_jo_struct_init` pose `kw = NULL` → rouge : caractérisation 1.
  4. Propre à l'étape, *identité perdue* (a1) : `pico8_jo_init` pose dans `g_jo_dz` une table neuve au lieu du pointeur de la spec. Attendu rouge : caractérisation 1 et 2, `STRUCT_HASH` ; `test_pico8_jo:708` (lent) noté sans relance.
  5. Propre à l'étape, *genre mêlé perdu* (a1) : la pose de `qh` met `hf_is_bool = 0`. Attendu rouge : caractérisation 1 (`FIXED` 0 au lieu de `BOOL` 0 : même vérité, genre différent).
  6. Propre à l'étape, *aide sans `assert`* (a2) : `jo_take_table` recopie tout genre. Attendu rouge : test d'arrêt.
  7. Propre à l'étape, *écrivain muet* (a2) : la branche `kw` rend 1 sans écrire. Attendu rouge : caractérisation 5.
  8. Propre à l'étape, *`hh` sans `nk`* (a2) : l'appel `jo_take_table(nk_val, &self->nk)` de `pico8_jo_hh` sous `if (0)`. Attendu rouge : caractérisation 4 ou 6 si `hh` y pose une table (H11) ; sinon le filet, et le trou est noté.
- **`gcov`** : `test_pico8_ui` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23, sur les six fichiers `src/` modifiés. Chaque ligne modifiée doit être exécutée. Une ligne que `test_pico8_ui` n'atteint pas (probablement `mx:kf`, `planet_trade.c`, H15) est cherchée dans **une** passe `gcov` d'un des TU rapides cités en H15, sinon justifiée dans le commit. Trous attendus : l'échec des `assert` des aides d'écriture (fils de `must_abort` qui avortent sans vider leur compteur, prouvés par le test d'arrêt et la mutation 6) ; l'échec des `assert` des aides de lecture de spec (inatteignable, la spec est un littéral). Un rapport par commit.
- **Prior art** : `test_ke_me_by_name` et `test_ke_ks_stops_off_row` (1b.14a, même fichier) ; `test_struct_defaults` (`test_pico8_jo_spec_defaults`) ; `test_too_far_members_by_name` et `test_too_far_writer_stops_off_row` (1b.13) ; cas `memset` de 1b.14b/1b.15/1b.18.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests pour les deux commits.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. S'il s'arrête après a1 sans a2, il lance `make sanitize-net` sur a1 avant de s'arrêter. Ordres de grandeur :

- `./tmp/test_pico8_ui` : 10 à 30 s. Une vingtaine de lancements (départ, a1, arrêt rouge, a2, mutations, `gcov`) en 6 min environ ;
- `make test` : 20 s à 1 min, une fois par commit ;
- `make e2e` : 4 à 6 min, une fois par commit ;
- `make class-coverage-net` : 3 à 4 min, **dans la même invocation** que le `make e2e` final (arbres distincts, 1b.0) ;
- `make sanitize-net` : ~4 min seul, **jamais** en même temps que `e2e`, une fois sur l'arbre final ;
- `make value-members` : quelques secondes, une fois par commit.

**TU lents touchés** : `test_pico8_jo`, `test_pico8_qh`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`, `test_pico8_nr` (tableaux `written[]`, a2) ; `test_pico8_ie` n'est pas touché. **Construire les sept tôt**, juste après le typage de `kw`/`hn`/`nk` et avant toute mutation (`make -j tmp/test_pico8_jo tmp/test_pico8_qh tmp/test_pico8_os tmp/test_pico8_hj tmp/test_pico8_pm tmp/test_pico8_mx tmp/test_pico8_nr`, compilation seule). Le compilateur ne désigne aucune de leurs lignes (accès par nom), mais un arbre qui ne se lie pas se voit tout de suite. Ils ne sont lancés que par `make e2e`. En a1, ils ne changent pas, mais ils exercent le code changé : le `make e2e` de a1 les passe.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre (départ / a1 / a2 / mutation n) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles du commit a1**, vertes sur l'arbre a1 : `timeout 10m make test` ; `timeout 15m make e2e` (15 « conforme », 8 TU lents verts) ; `make value-members` → `membres pico8_value_t : 160`.

**Cibles finales**, vertes sur l'arbre a2 :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les **8** TU de `UNIT_TESTS_SLOW` passent (`test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 157`**, 19 lignes `pico8_jo_t` ;
5. `timeout 15m make class-coverage-net` : **sortie identique à `tools/class_coverage_net_expected.txt`**. Elle s'applique parce que le diff touche des constructeurs (`pico8_jo_struct_init`, `pico8_qh_struct_init`, `pico8_hj_struct_init`, `pico8_nr_struct_init`, `pico8_nl/os/pm/ga/mx_struct_new`), sans toucher `pico8_class_coverage_record` ni `KNOWN[]`. Aucun compte ne change.

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur l'un des deux commits.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou un commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si une aide d'écriture ou de lecture de spec avorte dans un rejeu ou dans un TU hors des fils de `must_abort` (H2, H3, H4), si un indicateur `has_<m>` est ajouté à l'un des 12 membres, ou si un type est élargi (union, `pico8_value_t` gardé à côté) au lieu d'avorter.
- `FAILED:` si l'un des 12 membres n'a pas le type du tableau de recensement à sa place dans `pico8_jo_t`, si `hf_is_bool` manque ou n'est pas juste après `hf`, ou si l'un des 19 autres membres est typé ou touché.
- `FAILED:` si une table de défauts, d'offsets ou de clés liste encore l'un des 12 noms (preuves `offsetof` et `{"<m>", {0}}`), ou si une autre entrée de ces tables est retirée ou modifiée ; si `fr` gagne une pose.
- `FAILED:` si un défaut REF de `jo` ou d'une fille n'est pas le **pointeur de sa spec**, si un `static` déballé n'est pas remis à nil dans son `defaults_reset`, ou si `pico8_jo_dz/cq/ka/em()` changent de contrat.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact pour un pointeur NULL, ne rend pas le même pointeur, ou rend un autre genre pour `hf` que celui de 1a.
- `FAILED:` si l'écrivain n'accepte plus exactement les 17 mêmes noms, si une aide ne pose pas nil d'abord, n'avorte pas sur un non-REF à union nulle, une chaîne NULL ou une fonction NULL, ou si son message ne nomme pas `jo.<m>` et le genre de la ligne.
- `FAILED:` si un site par nom de `src/` change, ou si un site par nom est remplacé par un accès direct.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si le diff d'un test fait autre chose que : ajouter la caractérisation, le test d'arrêt, `JO_MEMBERS_HASH` et leurs appels à `test_pico8_ui` ; retirer `"kw"`, `"hn"`, `"nk"` des 10 tableaux `written[]` ; changer la seule ligne `jo.kw` de `test_pico8_ga_export_panel_by`. Idem si une valeur figée existante change (H6).
- `FAILED:` s'il manque les deux lignes « Runs » de la vérification §6 de `test_pico8_ga_export_panel_by` (rouge avec le bug remis, vert restauré).
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 160 après a1 et 157 après a2, ou si un autre typedef que `pico8_jo_t` gagne ou perd une ligne.
- `FAILED:` s'il manque une ligne « Runs » de `make class-coverage-net` identique à la référence, ou si le diff touche `pico8_class_coverage_record` ou `KNOWN[]`.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, si elle lit ou écrit l'un des 12 membres directement, si elle ne couvre pas les 10 genres et les 12 noms, ou si `JO_MEMBERS_HASH` change après ; si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'arbre a1 et une verte sur a2.
- `FAILED:` si un commit ne contient pas son rapport `gcov` et ses mutations (a1 : 3a–3e, 4, 5 ; a2 : 1, 2, 3f, 6, 7, 8), chacune avec son test ou la justification de son trou.
- `FAILED:` si le travail n'est pas en deux commits a1 puis a2, tous deux verts ; si un commit est rouge ; si a1 contient `kw`, `hn` ou `nk` typés ; ou si un membre de 1b.23b–d y est mêlé.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque les notes « Découvert à 1b.23a1 » et « Découvert à 1b.23a2 », ou si elles ne portent pas les lectures nouvelles (aides de lecture de spec par fichier ; `hf` 1b.5 sans `has_` ; branche `l` seul gardée ; boucles sentinelles), ou s'il manque le tableau « Runs ».

## Out of Scope

- Les 19 autres membres de `jo` (1b.23b–d), dont les entrées `ma`, `gq`, `ca` de `g_jo_defaults` et `by`, `mi`, `id`, `p`, `ra` des tables des filles.
- Les membres propres des filles (`qh.hp`, `qh.et`, `ga.fz`…) : 1b.24 et suivantes.
- Les lecteurs de spec `pico8_X_default` et les specs `ob` (phase D).
- Remplacer un site par nom (`entity.c`, `entity_geom.c`, `ui.c`, `economy_*`, `research_display.c`) par un accès direct.
- Retirer `hn` ou `nk` de l'écrivain.
- Régénérer `tools/member_census.txt`.
- Les fichiers d'événements et `FRAMES_EVENTS`, tout changement de rendu, l'audio, lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `planet.[ch]` (type, lecteur, écrivain, init, accesseurs, `pico8_jo_hf`), `planet_family.c` et `planet_family_trade.c` (R7 des 8 filles), `planet_display.c` et `planet_trade.c` (R6) ;
  - tests : `test_pico8_ui` étendu (caractérisation des 12 membres sur les 10 genres, test d'arrêt rouge d'abord) ; 10 tableaux `written[]` réduits ; une ligne du test de régression. Pas de nouveau TU ;
  - découpage en **1b.23a1** (9 membres à défaut, 169 → 160) et **1b.23a2** (`kw`, `hn`, `nk`, 160 → 157), deux commits verts, un seul PRD ;
  - `hf` : forme 1b.5 sans `has_hf`, emballage `pico8_jo_hf` ; `hn` → `pico8_hp_t *` ; `kq` → `pico8_qz_box_t *` ;
  - R7 : lecture de 1b.14a, avec des aides de lecture de spec par genre et par fichier à la place des `assert` en ligne ;
  - R5 : branches `kw`, `hn`, `nk` gardées, aides `jo_take_table` (partagée avec les écritures directes de `nk`) et `jo_take_hp` ;
  - `class-coverage-net` lancé une fois, sur l'arbre final.
- **Plan** : le commit a1 remplace la ligne 1b.23a par « 1b.23a — défauts et tables de classe, découpée : » suivie de deux lignes. D'abord « 1b.23a1 — `nu`, `hf` (genres mêlés), `hs`, `fa`, `kq`, `ka`, `dz`, `cq`, `em`, défauts réécrits dans les tables de 8 filles. Compte : 160. » avec sa note. Ensuite « 1b.23a2 — `kw` (écrit par nom), `hn` (→ 1b.9), `nk`. Compte : 157. », dont a2 écrit la note.
- **Note « Découvert à 1b.23a1 »** : « `hf` → lecture de R1 de 1b.5 sans indicateur (nil `jamais`) : `pico8_fixed_t hf` + `int hf_is_bool`, lu toujours, emballage `pico8_jo_hf` ; `kq` → `pico8_qz_box_t *` (1b.20) ; R7 de 1b.14a sur `jo` et 8 filles (14 entrées, `fr` sans pose) ; **lecture de forme** : quand un fichier déballe plus de trois défauts, une aide `static` de lecture de spec par genre et par fichier remplace les `assert` en ligne, message aux noms de ses membres ; accesseurs `pico8_jo_dz/cq/ka/em` sur les `static` ; empreinte réelle 27 (13, 14). » Le reste va dans le commit.
- **Note « Découvert à 1b.23a2 »** : « `hn` → `pico8_hp_t *` ; **lecture nouvelle du point 1 de 1b.13** : un membre `l` seul que l'écrivain de 1a accepte déjà (`hn`, `nk`) garde sa branche, déballée par l'aide de son genre, et l'ensemble des noms de l'écrivain ne change pas ; `jo_take_table` partagée entre écrivain et écritures directes de `nk` (1b.18) ; **lecture nouvelle, boucles sentinelles** : un membre typé sort des tableaux `written[]` des TU (10 ici, dont 7 lents), et son écriture par nom est figée par la caractérisation de l'étape ; 1b.23b–d, 1b.24, 1b.32–1b.35 recopient cette forme. » Le reste va dans le commit.
- Chaque message de commit porte le périmètre, le compte, le tableau des poses par module (valeur et origine du pointeur), le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous, `JO_MEMBERS_HASH` et les réponses aux hypothèses qui le concernent (a1 : H1, H3–H7, H9–H11, H16 ; a2 : H1–H3, H6, H8, H12–H15, H17).
- Un sous-agent frais relit le diff contre ce PRD avant chacun des deux commits.
