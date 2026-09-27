---
title: "Plan 3, étape 1b.23b : `jo`, position — `ra`, `p`, `gq`, `ca` typés, sortis des tables de `jo` et de la table de clés de `ga`, écrivain de `qh` et quatre boucles de clés sous la forme 1b.13/1b.15"
description: "`pico8_jo_t` garde 19 membres `pico8_value_t` après 1b.23a2. 1b.23b en type 4 (`gq`, `ca`, `p`, `ra`) selon le patron R1–R9 de 1b.1 : trois vecteurs `pico8_vec_t *` (`ref:autre`, lecture 1b.17/1b.19), un fixe ; `gq`/`ca` par la lecture R7 de 1b.14a (défaut = pointeur de la spec), `p`/`ra` par la forme écrivain de 1b.13/1b.15 dans l'écrivain de `qh` et les boucles de clés de `jo`, `pm`, `ga`, `nr`. `make value-members` passe de 157 à 153. Un seul commit (14 fichiers, sous les 16 de 1b.23a2)."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.23b — position : `ra`, `p` (écrits par nom), `gq`, `ca`. Empreinte 26 (13, 13). Compte : 153.

1b.23b ne type que ces 4 membres. `id`, `oe`, `ep`, `jy`, `fx`, `lq`, `jh`, `p_` sont 1b.23c ; `kr`, `cc`, `by`, `mi`, `m_`, `fo`, `ma` sont 1b.23d.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité, pas redécidé. Les notes « Découvert à » de `docs/plan-3.md` s'appliquent, citées et non redérivées :

- **1b.1** : R1 à R9 ; `ref:pico8_X_t` → pointeur, `fixe` → `pico8_fixed_t`, pointeurs non `const`, sans indicateur (NULL = nil) ; R3 « un défaut REF garde la même REF » ; R5 « même règle dans l'écrivain `set_field` et dans la boucle des clés de construction ».
- **1b.3 à 1b.10, 1b.12, 1b.14b, 1b.16, 1b.18, 1b.20, 1b.21** : patron appliqué, lectures déjà reprises par 1b.23a (R3 lu sur ce qui initialise ; boucles sentinelles arrêtées au recensement, 1b.4b ; lecture 1b.10 de `ref:autre` = REF non NULL de tag `UNKNOWN` ; cas « union nulle » par `memset` puis `kind`, mutation d'un appel unique sous `if (0)`, 1b.14b ; aide partagée entre écrivain et écriture directe, 1b.18 ; site C1 de `float-audit` réécrit seul, 1b.21).
- **1b.11 — R2, TU lents** : `CENSUS_UNIT_TESTS` exclut `UNIT_TESTS_SLOW` ; ce PRD relit les TU lents de l'arbre `jo` (revue plus bas).
- **1b.13 et 1b.15 — forme écrivain de R5** : branche `strcmp` par membre à `e` dans l'ordre du typedef ; aide `static` par genre, jamais partagée entre fichiers, partagée avec la boucle de clés du même fichier ; pose **d'abord** nil, sort sur `REF NULL` (genre **et** pointeur), sinon `assert` « champ sans règle » qui nomme `struct.membre` et le genre de la ligne ; un non-REF à union nulle (faux, fixe 0, chaîne NULL, fonction NULL posés par `memset`) avorte ; un `must_abort` rouge sur l'ancien code fige l'arrêt.
- **1b.14a — R7 sur une table de défauts qui garde des entrées et a des filles** : l'entrée sort de toutes les tables qui la listent ; le module déballe une fois, à son init, par son lecteur de spec, dans un `static` même pointeur que la spec ; remise à nil dans `defaults_reset` ; pose typée après la boucle, avant clés et `pq` ; filles sans défaut propre : rien.
- **1b.17 et 1b.19 — membre vecteur** : `ref:autre` vers un `v(…)` → `pico8_vec_t *` ; aide `qb_take_vec` (REF non NULL, tag `UNKNOWN`, nil → NULL) ; défauts **module** `&g_nf_ra`, `&g_qb_p`, `&g_qb_ra` quand le défaut n'a pas à être le pointeur de la spec ; lecture relue par `pico8_X_value_vec(pico8_value_ref(self-><m>))` (1b.15, `bg.mq`).
- **1b.22 — REF à plusieurs cibles** (`pico8_struct_header_t *`, prédicat sur `cls`) : **sans objet** ici, chaque membre de l'étape n'a qu'un genre (voir le recensement). Citée pour l'écarter.
- **1b.23a1** (note « Découvert à », arbre de travail) : `hf` 1b.5 sans indicateur ; R7 de 1b.14a sur `jo` et 8 filles ; **lecture de forme** : une aide `static` de lecture de spec par genre et par fichier (`jo_default_*`, `family_default_*`, `family_trade_default_*`), message aux noms de ses membres ; **H11 démentie en partie** : les filles de `qh` passent par `jo:lz` dans leur `pq`, la caractérisation fige les valeurs **observées**.
- **1b.23a2** (PRD de 1b.23a) : aides d'écriture `jo_take_table`, `jo_take_hp` dans `planet.c` ; **boucles sentinelles** : un membre typé sort des tableaux `written[]` des TU, son écriture par nom est figée par la caractérisation de l'étape — « 1b.23b–d recopient cette forme ».

### État de départ

HEAD `a0c0cb3` (PRD de 1b.23a). 1b.23a1 est dans l'arbre de travail, en vérification ; 1b.23a2 suit. **Précondition : 1b.23a1 et 1b.23a2 sont commités avant que 1b.23b commence** (H8). `make value-members` rend alors **157** (H1). Les lignes citées sont celles de l'arbre de travail a1 et glisseront avec a2 : chaque site est aussi désigné par son contenu.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H18).

### Lignes de recensement (`tools/member_census.txt:67-72`)

| membre | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|
| `gq` | `ref:autre` | jamais | - | l | `pico8_vec_t *` | non |
| `ca` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `p` | `ref:autre` | observé | lu | **le** | `pico8_vec_t *` | non |
| `ra` | `ref:autre` | observé | lu,stat? | **le** | `pico8_vec_t *` | non |

4 membres : 157 − 4 = **153**. Les 15 autres lignes `pico8_jo_t` (`ma id oe ep jy fx lq fo jh p_ kr cc by mi m_`) ne bougent pas.

- **Trois vecteurs, un seul genre chacun.** Le `ref:autre` de chaque ligne est un `v(…)` : `gq=v(0,11)` dans la spec de `jo` ; `p`/`ra` posés par `bv` (`self.ra,self.p=mp,mp/128`), par le générateur de galaxie (`jo({id=…,p=p,ra=p*128})`), repassés par `jo:hu` et par `pm:pq` (`ra=self.ra+self.ng`). R1 de 1b.17 : `pico8_vec_t *`. Pas de pointeur d'en-tête (1b.22 sans objet).
- **Aucun indicateur.** Les trois vecteurs sont des pointeurs (R2 : NULL = nil ; le nil de `p`/`ra` est celui d'une planète nue, `!has(bare, "ra")`). `ca` est nil `jamais`, sans preuve, et la revue des TU lents ne lui trouve aucune lecture nil par nom (H4).
- **`p`, `ra` ont `e`** : écrits par nom par `bv` (`probe.c`, `pico8_entity_set_field(self, "ra", mp)` puis `"p"`), qui n'atteint que les planètes de la famille `qh` (`qh.hq=bv`) ; l'écrivain de `qh` a déjà leurs deux branches. Les écrivains de `jo` et de `ga` ne les prennent pas (`must_abort(write_ra)` de `test_pico8_jo` le fige).

### Revue des TU lents (R2 de 1b.11)

Relus pour ces 4 noms : `test_pico8_jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr` (arbre `jo`) ; `test_pico8_ie` est sans objet (son `ra` est celui d'`ie`, un homonyme). Constats :

- `fold_vec(h, e, "p"/"ra"/"gq")` : absent → `0xDEAD`, présent → `x`, `y` (lecture nil par nom de `p`/`ra`, sans effet : pointeurs) ;
- `scalars[]` qui contient `ca` (`fold_field`), toujours présent ; `fold_daughter` (`jo:464`) plie `ca` et `gq` sur les filles ;
- identité de pointeur face à la spec : `class_refs[] = {…, "gq", …}` de `test_pico8_jo` (`:85`, `:307`), `jo_tables[] = {…, "gq"}` de `qh`, `os`, `hj`, `pm`, `mx`, `nr` (`get_ref(e, "gq") == class_ref(pico8_jo_default, "gq")`) — **d'où le choix du pointeur de spec pour `gq`** (plus bas) ;
- identité des vecteurs de construction et de `bv` : `get_ref(placed, "ra") == mp.as.ref` (`qh:408`, `os:484`, `hj:503`, `mx:501`, `nr:477`), `get_ref(with, "p") == p.as.ref` (`nr:533`) ;
- `!has(bare, "ra")`, `!has(bare, "p")` (`jo:726`, `pm:534`, `pm:735`) : absents sur une planète nue ;
- **les boucles sentinelles `written[]`** de `qh:781`, `os:630`, `hj:736`, `pm:685`, `mx:742`, `nr:687` écrivent `pico8_value_fixed(i + 1)` sur `"ra"` et `"p"` et le relisent `FIXED`. Mêmes boucles dans deux TU rapides : `test_pico8_nl:589`, `test_pico8_fr:556`. Celles de `jo:681` et `ga:687` n'ont ni `ra` ni `p` (écrivains qui ne les prennent pas).

**Aucune lecture nil par nom de `ca`** (H4). R2 ne donne aucun indicateur. Les boucles sentinelles de 8 TU écrivent un fixe dans `ra` et `p` : typés, ces membres avortent (voir « Boucles sentinelles »).

### Ce que fait le cart (`pico8/lua.lua`)

- Spec de `jo` : `gq=v(0,11),ca=9`. Aucune fille de `jo` ne redéfinit `gq` ni `ca` (`qh`, `hj`, `pm`, `mx`, `nl`, `ga`, `os`, `nr`, `fr`). `gq`/`ca` ne sont jamais écrits ; lus par `pz` (`b.fa=p.ca`, `bp+=p.gq`, sur une copie `bp`, jamais en place) quand `jo:fe` appelle `pz(self, …)`.
- `p`, `ra` : pas de défaut. Posés par la clé de construction (`jo(…)` de la galaxie, `kv[hp.cw]({p=self.p,ra=self.ra,id=self.id})` de `jo:hu`, `ga(ob(…,{ra=self.ra+self.ng}))` de `pm:pq`), par `bv` (`qh.hq`), et `ra` modifié **en place** par `na` (`qh.ly=na`, `ra.y+=…`). Lus par `gk`, `gd`, `ij`, `jo:hk` (`iv({ra=self.ra,…})`), `jo:kj`, `pz`, `jc`, `mz`.

### Le code (arbre a2 supposé)

- **Typedef** (`planet.h`) : `gq`, `ca` après `kw` ; `p`, `ra` après `id` ; tous `pico8_value_t`.
- **`g_jo_defaults[]`** (`planet.c`, nom/valeur) : après a2, `ma gq ca`. Boucle de recopie `*pico8_jo_member(self, name) = value` dans `pico8_jo_struct_init`, remise à nil dans `pico8_jo_defaults_reset`. Aides de lecture de spec de a1 : `jo_default_table`, `jo_default_box`, `jo_default_fixed` (`"jo.hs/fa: …"`), `jo_default_hf`.
- **`g_jo_members[]`** : après a2, 19 entrées dont `gq`, `ca`, `p`, `ra`. Sert au lecteur `pico8_jo_field`, à l'écrivain (`written[]`, 14 noms après a2, sans `p`/`ra`), à la boucle nil de `pico8_jo_struct_init` et à la boucle de clés de `pico8_jo_struct_new`.
- **Boucles de clés de construction qui écrivent `p`/`ra`** (R5 boucle), **quatre** :

| constructeur | fichier | forme aujourd'hui | clés |
|---|---|---|---|
| `pico8_jo_struct_new` | `planet.c` | générique `*pico8_jo_member(self, key.name) = value` après `assert(measured …)` | `id`, `p`, `ra` |
| `pico8_pm_struct_new` | `planet_family.c` | directe `self->hj.qh.jo.p = …` / `.ra = …` | `p`, `ra`, `id` |
| `pico8_ga_struct_new` | `planet_family.c` | **table d'offsets `keys[]`** `{"p", offsetof(pico8_ga_t, jo.p)}`, `{"ra", offsetof(pico8_ga_t, jo.ra)}` + 9 membres de `ga` | `p`, `ra`, `id`, `qo`… |
| `pico8_nr_struct_new` | `planet_family_trade.c` | directe `self->qh.jo.p = value` / `.ra = value` | `id`, `p`, `ra` |

  `qh`, `nl`, `os`, `hj`, `mx`, `fr` n'acceptent que `id` (leur `assert(measured …)` avorte sur `p`/`ra`) : non touchés.
- **Écrivain par nom** : `pico8_qh_set_field` (`planet_family.c:127-143`) : `qh->jo.ra = value;`, `qh->jo.p = value;`. `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr` y délèguent. `pico8_jo_set_field` et `pico8_ga_set_field` n'ont ni `ra` ni `p`.
- **Tables d'offsets qui listent ces 4 membres** : `grep -rnE 'offsetof\(pico8_[a-z0-9_]+_t, ([a-z0-9_]+\.)*(ra|p|gq|ca)\)' src/ | grep -E 'pico8_(jo|qh|nl|os|hj|pm|ga|nr|fr|mx)_t'` rend **6 lignes** : 4 de `g_jo_members` et 2 de `keys[]` de `ga`. Aucune table de défauts de fille (`g_qh_defaults`…) ni de membres de fille ne les liste.
- **Lectures directes (R6)** : `pico8_jo_hu` (`pico8_jo_present(self->p)`, `…(self->ra)`, recopiés dans les `overrides`) ; `pico8_jo_hk` (`self->ra`, idem) ; `pico8_pm_pq` (`pico8_pf_value_vec(self->hj.qh.jo.ra)`).
- **Écritures directes (R6)** : les boucles de clés et l'écrivain de `qh` ci-dessus. Aucune autre dans `src/` (`grep -rnE 'jo\.(ra|p|gq|ca)\b' src/`).
- **Test de régression §6 touché** : `tests/test_pico8_ga_export_panel_by.c:115` `planet->jo.ra = pico8_value_ref(ra);` et `:118` `planet->jo.gq = pico8_value_ref(gq);` (écritures directes, que le compilateur désignera).
- **Sites par nom, inchangés** (puce Coexistence) : `probe.c` (`bv` écrit `ra`/`p` ; lit `p` des extrémités d'un `jc`), `galaxy.c` (clés `p`/`ra`), `entity.c` (`ra` des couches), `entity_geom.c` (`gk`, `gd`, `na` qui écrit `ra` en place par le pointeur, `ij`), `msgbox.c` (`pz` : `ca`, `gq`, `ra`), `economy_formulas.c` (`p`), `slipway.c`, `slipway_display.c`, `wormhole.c`, `wormhole_display.c`, `ui.c` (`je.ra`), et les TU.

### Coexistence : 4 membres de plus typés, 15 non typés

Même forme qu'en 1b.23a :

- **Lecteur** `pico8_jo_field` : 4 branches R4 de plus, dans l'ordre du typedef (`gq`, `ca` entre `kw` et `em` ; `p`, `ra` après `em`), avant `pico8_jo_member`. Les lecteurs des filles délèguent déjà à celui de `jo` pour ces noms.
- **Écrivain** de `qh` : ses deux branches `ra`, `p` passent par l'aide R5. `written[]` de `jo` ne change pas (il n'a ni `p` ni `ra`) : l'ensemble des noms qu'accepte chaque écrivain de l'arbre ne change pas.
- **Tables** : `g_jo_members[]` perd `gq ca p ra` (15 restent) ; `g_jo_defaults[]` perd `gq ca` et garde `ma` seul, avec sa boucle et sa remise à nil (R7 de 1b.14a, point 1) ; `keys[]` de `ga` perd `p`, `ra` et garde ses 9 autres entrées. **Piège** (même que a2) : un nom de clé ou de `written[]` sans entrée dans `g_jo_members[]` ferait déréférencer NULL dans la boucle générique de `pico8_jo_struct_new` : les branches typées `p`/`ra` passent **avant** l'appel à `pico8_jo_member`, qui ne reçoit plus que `id`.

### Empreinte remesurée ; découpage

La ligne dit 26 (13, 13). Par greps de noms (`"ra"`, `"p"`, `"gq"`, `"ca"`, `jo.<m>`), le majorant est ~60 (27, 33), gonflé par homonymie (`ra`/`p` sont portés par `nf`, `qb`, `ie`, `iv`, `g_`, `fv`, `mz`, `js`, `jc`, les particules ; `gq`/`ca` par `fv`, `pj_cr`, `ha`, le menu).

**Empreinte réelle** (fichiers qui lisent ou écrivent l'un des 4 membres d'une struct de l'arbre `jo`), par lecture : **≈ 35 (15, 20)**, sous le seuil de ~40.

- **src (15)** : `planet.h`, `planet.c`, `planet_family.c`, `planet_family_trade.c`, `probe.c`, `galaxy.c`, `entity.c`, `entity_geom.c`, `msgbox.c`, `economy_formulas.c`, `slipway.c`, `slipway_display.c`, `wormhole.c`, `wormhole_display.c` (H18), `ui.c`.
- **tests (≈ 20)** : `test_pico8_jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`, `nl`, `fr`, `ga`, `ga_export_panel_by`, `jo_spec_defaults`, `spec_defaults`, `empty_planet_cleared`, `hp`, `planet_kj_dispatch`, `jc`, `g_`, `bg`, `mz` (H18).

**Fichiers modifiés : 14**, plus `docs/plan-3.md` :

- **src (4)** : `pico8_planet.h`, `pico8_planet.c`, `pico8_planet_family.c`, `pico8_planet_family_trade.c` ;
- **tests (10)** : `test_pico8_ui.c` (ajouts : caractérisation, test d'arrêt) ; les tableaux `written[]` de `test_pico8_qh`, `os`, `hj`, `pm`, `mx`, `nr` (**6 TU lents**), `nl`, `fr` ; deux lignes de `test_pico8_ga_export_panel_by` (**test de régression §6**).

**Découpage (G) : aucun, un seul commit.** 14 fichiers, sous les 16 de 1b.23a2 que le PRD de 1b.23a a jugés tenables en une passe ; les 8 TU à boucle sentinelle ne changent que de deux jetons chacun, dans la forme posée par a2. Deux formes seulement, déjà écrites : R7 de 1b.14a sur `jo` seul (`gq`, `ca`, aucune fille) et R5 de 1b.13/1b.15 (`p`, `ra`). Écarté (G) : 1b.23b1 (`gq`, `ca`) puis 1b.23b2 (`p`, `ra`) — le test de régression `ga_export_panel_by` écrit `jo.gq` **et** `jo.ra` et aurait sa vérification §6 dans les deux commits ; la première moitié ferait 4 fichiers pour une forme déjà vue sans rien de nouveau ; deux `make e2e` et deux `class-coverage-net` ne tiennent pas dans 20 minutes.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (arbre a2 commité) : les 4 membres lus par nom sur `jo` et ses 9 filles, les boucles de clés, l'écrivain de `qh` par aller-retour, `bv`, `na`, `hu`, `pm:pq`, les planètes vivantes d'une partie (voir Testing Decisions). Verte avant tout typage.
2. **Test d'arrêt, rouge sur l'ancien code** : écrivain de `qh` et boucles de clés hors genre, avec les cas à union nulle.
3. **Commit 1b.23b** : `gq`, `ca`, `p`, `ra` typés. `gq`/`ca` sortent de `g_jo_defaults` et de `g_jo_members`, déballés une fois à l'init de `jo` dans `g_jo_gq` (pointeur de la spec) et `g_jo_ca`. `p`/`ra` sortent de `g_jo_members` et de `keys[]` de `ga` ; l'écrivain de `qh` et les quatre boucles de clés déballent par une aide par fichier ; les lectures directes sont réécrites ; `"ra"`, `"p"` sortent de 8 tableaux `written[]` ; deux lignes du test de régression. `make value-members` : **153**.
4. Aucun pixel ne bouge. La note « Découvert à 1b.23b » va sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `planet.h`, je veux `gq`, `p`, `ra` typés `pico8_vec_t *` et `ca` typé `pico8_fixed_t`, afin de lire la position d'une planète sur son type.
2. En tant que lecteur par nom (`entity.c`, `entity_geom.c`, `msgbox.c`, `probe.c`, `slipway*.c`, `wormhole*.c`, `ui.c`, `economy_formulas.c`, les TU), je veux que chacun des 4 noms rende exactement ce que rendait l'ancien code sur les 10 genres de l'arbre `jo` : présence, genre, valeur, identité du pointeur.
3. En tant que `jo` et chacune de ses 9 filles, je veux recevoir pour `gq` le **pointeur même** de la spec de `jo`, afin que `class_ref("gq")`, les `jo_tables[]` des TU lents et `STRUCT_HASH` ne bougent pas.
4. En tant que planète, je veux `ca` = 9 partout, toujours présent.
5. En tant que planète nue (`pico8_jo_new(NULL)`), je veux `p` et `ra` absents, comme en 1a.
6. En tant que galaxie, `jo:hu` ou `pm:pq`, je veux que le vecteur passé en clé `p`/`ra` soit le pointeur même que la planète garde (la galaxie le dit : « `p` aliases the instance's own field »).
7. En tant que `bv`, je veux que `ra` reçoive le pointeur `mp` même, et `p` le vecteur neuf `mp/128`.
8. En tant que `na`, je veux déplacer `ra.y` en place par le pointeur rendu par nom, comme en 1a.
9. En tant que `jo:hu`, `jo:hk`, `pm:pq`, je veux lire `p`/`ra` par le pointeur, absents quand NULL, avec la même table de vérité que `pico8_jo_present`.
10. En tant qu'écrivain par nom de la famille `qh`, je veux `ra`, `p` acceptés dans leur genre (vecteur) ou nil, même pointeur, et tout autre genre arrêté par un `assert` « champ sans règle » qui nomme `jo.ra/p` et `ref:autre`.
11. En tant que boucle de clés de `jo`, `pm`, `ga`, `nr`, je veux la même règle que l'écrivain (R5 de 1b.1), par la même aide dans le même fichier.
12. En tant que mainteneur, je veux que l'ensemble des noms qu'acceptent les écrivains et les boucles de clés de l'arbre ne change pas (`jo` et `ga` refusent toujours `ra`/`p` par nom ; `qh`, `nl`, `os`, `hj`, `mx`, `fr` refusent toujours la clé `p`/`ra`).
13. En tant que mainteneur, je veux qu'aucune table de défauts, d'offsets ou de clés ne liste plus ces 4 noms, et qu'aucune écriture `*(pico8_value_t *)` ne les atteigne.
14. En tant que TU à boucle sentinelle, je veux que ma boucle ne réécrive plus en fixe `ra` ni `p`, leur écriture par nom étant figée une fois par la caractérisation.
15. En tant que test de régression `ga_export_panel_by`, je veux écrire `ra` et `gq` par le pointeur et rester rouge si le bug `by=o()` revient.
16. En tant qu'auteur de 1b.23c–d, 1b.24, 1b.32–1b.35, je veux une forme écrite pour une table de clés de fille (`keys[]` de `ga`) qui garde des entrées, et pour une aide d'écriture par fichier partagée entre écrivain et boucles de clés de plusieurs structs.
17. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
18. En tant que gardien de la mémoire et de la couverture, je veux `make sanitize-net` à 15 propres et `make class-coverage-net` identique à sa référence.
19. En tant que `plan-pilot`, je veux `make value-members` à 153.
20. En tant que vérificateur, je veux la caractérisation verte sur l'ancien code, le test d'arrêt rouge puis vert, les mutations du §7 et leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés comme le ferait un agent `guideline` (hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables), et marqués (G).

### R1–R9 appliquées

| règle | sur les 4 membres |
|---|---|
| R1 | Tableau de recensement. Nom et place dans le typedef gardés. Tous empruntés, jamais possédés ni libérés : spec (`gq`), vecteurs de clés, de `bv`, de `pm:pq` (TROU-7 inchangé). |
| R2 | Aucun `has_` (tableau et revue des TU lents). |
| R3 | `pico8_jo_struct_init` : `gq = g_jo_gq`, `ca = g_jo_ca`, `p = NULL`, `ra = NULL`, à la place de la boucle nil et de la recopie, avant clés et `pq`. Défaut REF = **même pointeur** que la spec. |
| R4 | 4 branches dans `pico8_jo_field`, ordre du typedef, avant `pico8_jo_member`. Pointeur NULL → `return 0`, `*out_value` intact ; sinon `pico8_value_ref(<m>)`. `ca` → `pico8_value_fixed`, toujours 1. Une aide `jo_field_vec` à côté de `jo_field_table` (G). |
| R5 | Écrivain de `qh` et quatre boucles de clés : voir plus bas. |
| R6 | `pico8_jo_present(self->p)` → `self->p != NULL` ; `set_name(…, self->p)` → `set_name(…, pico8_value_ref(self->p))` (idem `ra`, dans `hu` et `hk`) ; `pico8_pf_value_vec(self->hj.qh.jo.ra)` → `pico8_pf_value_vec(pico8_value_ref(self->hj.qh.jo.ra))` (forme 1b.15 de `bg.mq`, nil → (0,0) gardé). |
| R7 | `gq`, `ca` : lecture de 1b.14a sur `jo` seul (aucune fille n'a d'entrée). `p`, `ra` : entrées de `g_jo_members` et de `keys[]` de `ga` retirées, remplacées par R5. |
| R8 | Crochets `PICO8_MEMBER_CENSUS_*` gardés. `tools/member_census.txt` non édité. |
| R9 | 157 → 153. Restent 15 lignes `pico8_jo_t` : `ma id oe ep jy fx lq fo jh p_ kr cc by mi m_`. |

### `gq` : pointeur de la spec, pas un défaut module (G)

1b.17 et 1b.19 posent des défauts **module** (`&g_nf_ra`, `&g_qb_p`) : des `static pico8_vec_t` du fichier, distincts de la spec. Ici, les TU lents figent l'identité de `gq` avec le pointeur de la spec (`class_refs[]`, `jo_tables[]`). R3 (« un défaut REF garde la même REF ») et R7 de 1b.14a s'appliquent donc : `static pico8_vec_t *g_jo_gq`, déballé dans `pico8_jo_init` par `pico8_jo_default("gq", &v)` et une aide `jo_default_vec` (REF non NULL, tag `UNKNOWN` : lecture 1b.10, même test que `jo_default_box`, type différent), remis à NULL dans `pico8_jo_defaults_reset`. Du patron 1b.17/1b.19 on garde le type (`pico8_vec_t *`) et l'aide d'écriture par valeur ; on écarte le défaut module. Rien n'écrit `gq` en place (`pz` additionne dans une copie `bp`).

### `ca` : fixe sans indicateur

`static pico8_fixed_t g_jo_ca`, déballé par `jo_default_fixed`, dont le message devient `"jo.hs/fa/ca: … (fixe)"` (forme des messages de 1b.9/1b.18 et de la lecture de forme de a1). Remis à 0 dans `defaults_reset`.

### R5 : écrivain de `qh` et boucles de clés (G)

- **Aide par fichier et par genre** (lecture de forme de 1b.23a1 étendue aux aides d'écriture ; 1b.13 point 2 : jamais partagée entre fichiers, partagée avec la boucle de clés du même fichier) :
  - `planet.c` : `static void jo_take_vec(pico8_value_t value, pico8_vec_t **out)`, pour la boucle de `pico8_jo_struct_new` ;
  - `planet_family.c` : `static void family_take_vec(…)`, pour l'écrivain de `qh` et les boucles de `pm` et de `ga` ;
  - `planet_family_trade.c` : `static void family_trade_take_vec(…)`, pour la boucle de `nr`.
  Forme de `qb_take_vec` (1b.19) : `*out = NULL` ; `return` si `value` est `REF NULL` (genre **et** pointeur) ; sinon `assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN && "jo.ra/p: a write neither nil nor a vector ref is off its census row (ref:autre) -- a field under no rule of plan-3 section 5: stop, do not widen the member")` ; `*out = value.as.ref`. Écarté : une aide publique unique dans `planet.c` (1b.13 point 2 l'interdit).
- **Écrivain de `qh`** : `if (strcmp(name, "ra") == 0) { family_take_vec(value, &qh->jo.ra); return 1; }`, puis `p`. L'ordre `et`, `ra`, `p` de la fonction est gardé (`et` est à `qh`, 1b.24 ; `ra` avant `p` n'est pas l'ordre du typedef de `jo`, mais les deux branches sont indépendantes ; les remettre dans l'ordre `p`, `ra` est permis, précédent 1b.19). Commentaire de tête inchangé.
- **Boucle de `jo`** : `if (nom == "p") jo_take_vec(value, &self->p); else if (nom == "ra") jo_take_vec(value, &self->ra); else *pico8_jo_member(self, key.name) = value;` après l'`assert(measured …)` inchangé (`id` reste générique jusqu'à 1b.23c).
- **Boucle de `pm`** : les branches `is_p`/`is_ra` appellent `family_take_vec` ; `id` inchangé.
- **Boucle de `ga`** : deux branches `p`, `ra` **avant** la recherche dans `keys[]`, qui perd ses deux entrées `jo.p`, `jo.ra` et garde les 9 autres (`id`, `qo`, `ro`, `qg`, `q_`, `k_`, `iu`, `py`, `ms`) ; chaque branche `continue`. L'`assert(found < …)` et son message (qui nomme les 11 clés mesurées) sont inchangés : l'ensemble des clés acceptées ne change pas. **Lecture nouvelle (G)** : R7 sur une table de **clés** de fille qui garde des entrées ; forme identique au point 1 de 1b.14a (entrée retirée, table et boucle gardées), la branche typée précède la boucle.
- **Boucle de `nr`** : branches `p`/`ra` → `family_trade_take_vec` ; `id` et l'`assert(0 …)` final inchangés.
- Nil accepté partout (pointeurs). Un non-REF à union nulle, une chaîne NULL, une fonction NULL, une liste, une table `ob`, une struct avortent.
- **Limite connue, non comblée** : un `pico8_qz_box_t *` a lui aussi le tag `UNKNOWN` et passerait l'`assert` — même limite que `qb_take_vec` (1b.19) et `bg_take_vec` (1b.15) ; aucun producteur ne l'écrit (H2). Pas de nouveau tag (hors périmètre).

### Boucles sentinelles (forme de 1b.23a2, recopiée)

`"ra"` et `"p"` sortent des 8 tableaux `written[]` (`qh`, `os`, `hj`, `pm`, `mx`, `nr`, `nl`, `fr`), et **rien d'autre ne change** dans ces fichiers. L'intention (« l'écrivain prend ces noms et les rend ») est reportée dans la caractérisation (aller-retour dans le genre, nil) et dans le test d'arrêt (hors genre). Les commentaires de ces boucles (« takes exactly N names ») restent vrais : les écrivains prennent toujours ces noms.

### Test de régression `ga_export_panel_by` (§6)

`planet->jo.ra = pico8_value_ref(ra);` → `planet->jo.ra = ra;` ; `planet->jo.gq = pico8_value_ref(gq);` → `planet->jo.gq = gq;`. Deux lignes, intention inchangée (le contrat « `by` est une vraie liste »). Vérification §6 dans le même commit : on retire localement le bloc d'échange de `by` de `pico8_planet_family_init`, le test doit être rouge ; on restaure et il repasse vert. Deux lignes « Runs ».

### Preuves (sortie attendue sur l'arbre final)

- `grep -rnE 'offsetof\(pico8_[a-z0-9_]+_t, ([a-z0-9_]+\.)*(ra|p|gq|ca)\)' src/ | grep -E 'pico8_(jo|qh|nl|os|hj|pm|ga|nr|fr|mx)_t'` : vide (6 lignes avant) ;
- `grep -rn 'offsetof(pico8_jo_t' src/` relu : 15 entrées, `ma id oe ep jy fx lq fo jh p_ kr cc by mi m_` (R7 de 1b.1, le typedef garde des `pico8_value_t`) ;
- `grep -nE '\{"(gq|ca)", \{0\}\}' src/pico8_planet.c` : vide ; `g_jo_defaults` ne liste que `ma` ;
- `grep -nE '(\.|->)(ra|p|gq|ca)\.(kind|as)|jo_present\(self->(p|ra)\)' src/pico8_planet*.c` : vide ;
- `grep -rnE 'jo\.(ra|p) = ' src/` : vide (toutes les écritures passent par une aide ou par R3 `self->p = NULL`) ;
- `grep -c 'off its census row' src/pico8_planet.c src/pico8_planet_family.c src/pico8_planet_family_trade.c` : 3, 1, 1 (a2 : 2, 0, 0) ;
- `make value-members | grep -c 'pico8_jo_t'` : 15 ;
- `git diff --stat HEAD -- src/ ':!src/pico8_planet*'` : vide (sites par nom intacts) ;
- `git diff` des 8 TU à boucle sentinelle : dans `written[]` seulement, `"ra", "p",` retirés.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 157 sur l'arbre de départ (a1 et a2 commités), 153 après. Seules 4 lignes `pico8_jo_t` disparaissent.
- **H2** : dans les 15 rejeux et dans tous les TU hors fils de `must_abort`, `ra`/`p` ne reçoivent (par nom ou par clé) que nil ou une REF vers un `pico8_vec_t` (tag `UNKNOWN`) : `bv` (`mp` vecteur, `p` tas), galaxie, `hu`, `pm:pq`, TU (`vec_ref`).
- **H3** : `v(0,11)` de la spec de `jo` est une REF non NULL de tag `UNKNOWN` vers un `pico8_vec_t`.
- **H4** : aucun TU lent ne lit `ca` d'une struct de l'arbre `jo` nil par nom, et aucun rejeu ne le voit absent.
- **H5** : aucune valeur figée ne bouge : `STRUCT_HASH`, `SPEC_HASH`, `SHARE_HASH`, `READY_HASH` de `test_pico8_jo_spec_defaults` ; `STRUCT_HASH` de `test_pico8_spec_defaults` ; les hash `fold_*` des TU de l'arbre `jo` ; `JO_MEMBERS_HASH` (1b.23a), `KE_ME_HASH`, `DRAG_REPLAY_HASH`, `TECH_REPLAY_HASH` ; les 15 empreintes du filet.
- **H6** : le build `-DPICO8_MEMBER_CENSUS` compile. Il n'est pas lancé.
- **H7** : `test_pico8_ui` reste sous 30 s avec deux fonctions de plus (hors `UNIT_TESTS_SLOW`).
- **H8** : 1b.23a1 et 1b.23a2 sont commités ; a2 ne touche ni les lignes `p`/`ra`/`gq`/`ca` du typedef, ni les boucles de clés, ni l'écrivain de `qh`, ni `hu`/`hk`/`pm_pq` sur ces membres ; les numéros de ligne glissent seulement.
- **H9** : rien ne dépend de `sizeof(pico8_jo_t)` ni de la disposition des filles (les membres rétrécissent ; tous les offsets restants sont recalculés par `offsetof`).
- **H10** : sur l'ancien code, chaque cas du test d'arrêt se termine sans `SIGABRT` (sortie `_exit(0)`, ou un autre signal) : le `must_abort` est rouge. Un cas qui avorterait déjà (par exemple dans un `pq` ou `lt` qui lirait `p`/`ra`) est retiré du « rouge d'abord » et noté.
- **H11** : sur un `pico8_<X>_new(NULL)` neuf, `p`/`ra` sont absents sauf si un `pq` les pose (`jo:lz` des filles de `qh`, leçon de a1) ; `pm_new(NULL)` construit un `ga` dont `ra` = (0,0) + `ng`. Les valeurs figées sont celles **observées** sur l'ancien code.
- **H12** : `pico8_bv` est appelable sur une planète `qh` neuve après `start_game(DRAG)` (`cg("np")`, `ui.hw` posés), et `pico8_na` aussi.
- **H13** : `pico8_jo_hu` avec un `hp` dont `cw` = `"pm"` (le premier de `pico8_gb()` qui en a un) construit une planète neuve et l'enregistre dans `hv`.
- **H14** : aucun site C1 de `tools/float_audit_c1.txt` ne cite ces membres. Si un extrait change quand même, seule sa ligne est réécrite (1b.21).
- **H15** : `test_pico8_ga_export_panel_by` est rouge quand l'échange de `by` est retiré.
- **H16** : les lignes modifiées de `hu`, `hk`, `pm_pq`, des boucles de clés de `pm`, `ga`, `nr` et de l'écrivain de `qh` sont exécutées par `test_pico8_ui` (caractérisation, rejeu `DRAG`) ; à défaut, par **une** passe `gcov` de `test_pico8_hp`, `test_pico8_ga` ou `test_pico8_nl`.
- **H17** : retirer `"ra"`, `"p"` des 8 boucles sentinelles ne change aucune autre assertion de ces TU (ces boucles n'ont que des `assert`, pas de hash).
- **H18** : `mz.ij` (`wormhole_display.c`) et les `mz` de `test_pico8_mz` atteignent des planètes (compte de l'empreinte seulement, sans effet sur le diff).

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom, ou l'écran.** Les nouveaux tests n'accèdent à aucun des 4 membres directement : ils compilent sur les deux formes. Les seules lignes directes touchées sont celles du test de régression, désignées par le compilateur.
- **Caractérisation dirigée, figée sur l'ancien code** : `test_jo_position_by_name`, dans `test_pico8_ui` (rapide, dans `CENSUS_UNIT_TESTS`, lie `ENGINE_SRCS`, a `start_game`, `must_abort` et `in_fresh_child`, précédent 1b.14a et 1b.23a). Lancée par `in_fresh_child` après les tests de `jo` de 1b.23a. Elle lit par le **lecteur brut du descripteur** (`cls->field`, témoin `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder (retour, genre, valeur, pointeur).
  1. **Les 10 genres, neufs.** `start_game(DRAG)`, puis `pico8_<X>_new(NULL)` pour `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`. Pour chacun : `gq` présent, REF, `x`/`y` (0, 11), **même pointeur** que `pico8_jo_default("gq")` (`assert`) ; `ca` `FIXED` 9 ; `p`, `ra` : présence, et `x`/`y` si présents — valeurs **observées** figées (H11).
  2. **Boucles de clés**, pour `jo`, `pm`, `ga`, `nr` : `overrides` = {`p` = P, `ra` = R} (vecteurs du tas, plus `id` pour `jo`, `pm`) → `p` == P, `ra` == R (`assert` de pointeur) ; puis {`p` = nil} → `p` absent.
  3. **Écrivain par nom dans le genre**, sur `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr` : `ra` ← V → rend 1, même pointeur V ; `ra` ← nil → absent ; idem `p`. Sur `jo` et `ga` : `cls->set_field(e, "ra"/"p", V)` rend 0 et la lecture ne change pas (ensemble des noms inchangé).
  4. **Transitions** : `pico8_bv(qh, mp, rp)` → `ra` == `mp.as.ref`, `p` présent = `mp/128` ; `pico8_na(qh, t)` → `ra` même pointeur, `y` changé ; `pico8_jo_hu(e, hp)` avec `hp.cw` = `"pm"` sur un `jo` à `p`/`ra` posés → la nouvelle planète (dernière de `hv`) a les mêmes pointeurs `p`, `ra` (H13) ; `pico8_pm_new` avec `ra` = R → le `ga` construit par `pm:pq` a `ra` = R + `ng` (valeur).
  5. **Intrus** sur le lecteur de `jo` : `""`, `"r"`, `"raa"`, `"p "`, `"P"`, `"gq_"`, `"has_ca"`, `"ca_is_bool"` → 0, témoin intact.
  6. **Planètes vivantes** : on parcourt `pico8_hv()` après `start_game(DRAG)`, **avant** les constructions de 1. Pour chaque entité de l'arbre `jo`, on plie présence et `x`/`y` de `p`, `ra`, `gq`, la classe d'identité de `gq` (1 = pointeur de la spec) et `ca`, plus le nombre d'entrées.
  7. Tout est plié dans **`JO_POSITION_HASH`**, relevé au premier passage vert sur l'arbre de départ, puis inchangé. Les adresses sont jugées par `assert`, jamais pliées.
- **Test d'arrêt, rouge d'abord** (1b.13/1b.15) : `test_jo_position_stops_off_row`, par `in_fresh_child` après la caractérisation.
  - **Écrivain** : `must_abort` sur `pico8_entity_set_field(e, n, v)`, avec `e` un `qh`, un `pm` (délégation `pm` → `hj` → `qh`) et un `nr` neufs, `n` ∈ {`ra`, `p`}, `v` ∈ {fixe 1, `pico8_value_bool(1)`, chaîne `"x"`, **fixe 0 à union nulle** (`memset`, `kind = FIXED`), **faux à union nulle** (`memset`, `kind = BOOL`), **chaîne NULL** (`memset`, `kind = STRING`), **fonction NULL** (`memset`, `kind = FUNCTION`), REF vers une `ob_table`, REF vers `pico8_hv()` (liste), REF vers `e` (struct)}.
  - **Boucles de clés** : `must_abort` sur `pico8_jo_new(b)`, `pico8_pm_new(b)`, `pico8_ga_new(b)`, `pico8_nr_new(b)`, `b` = {`p` = v} puis {`ra` = v}, v ∈ {fixe 1, faux à union nulle, REF `ob_table`, REF struct}.
  - **Rouge sur l'ancien code** (écrivain et boucles recopient tout, le fils sort par `_exit(0)`, H10), vert après : une ligne « Runs » pour chacun. Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Tests existants qui figent déjà beaucoup** : `test_pico8_jo_spec_defaults` (`STRUCT_HASH`), les TU lents (`fold_vec`, `class_refs[]`, `jo_tables[]`, identité `ra` == `mp`), `test_pico8_ga` (`:314`, clés de `hu`), `test_pico8_hp`, `test_pico8_empty_planet_cleared`, `test_pico8_planet_kj_dispatch`.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_ui && ./tmp/test_pico8_ui` (ou le TU cité). Chacune est notée dans le commit avec le test qui la rougit ou la justification de son trou. Un appel unique d'aide `static` se retire sous `if (0)` (1b.14b).
  1. (§7-1) *indicateur forcé vrai* : **sans objet dans sa lettre**, aucun indicateur. Forme voisine : la branche R4 de `ra` rend 1 avec `REF NULL` quand `ra` est NULL. Attendu rouge : caractérisation 1 (`jo` nu) et 3 (nil → absent) ; `test_pico8_jo:726` (lent) noté sans relance.
  2. (§7-2) *« présent 0 » là où c'était absent* : **sans objet dans sa lettre**, `ca` n'est jamais absent. Forme voisine : la branche de `p` rend `pico8_value_fixed(0)` quand `p` est NULL. Attendu rouge : caractérisation 1 et 2.
  3. (§7-3) *init ou boucle qui saute un membre typé*. Forme « pose une autre valeur », parce que l'omission pure de `p = NULL`/`ra = NULL` laisse de la mémoire de `malloc` non déterministe (**trou justifié** de la forme littérale, précédent 1b.14a) :
     - (a) `pico8_jo_struct_init` pose `gq = NULL` → rouge : caractérisation 1, `STRUCT_HASH` ;
     - (b) `pico8_jo_struct_init` pose `ca = 0` → rouge : caractérisation 1 et 6 ;
     - (c) `pico8_jo_struct_init` pose `p = g_jo_gq` → rouge : caractérisation 1 (`jo` nu) ;
     - (d) la branche `ra` de la boucle de `pm` sous `if (0)` → rouge : caractérisation 2 ;
     - (e) la branche `p` de `ga` saute l'écriture (`continue` seul) → rouge : caractérisation 2 et 4 (`hu`, `pm:pq`).
  4. Propre à l'étape, *identité perdue* : `pico8_jo_init` pose dans `g_jo_gq` une copie du tas du vecteur de la spec. Attendu rouge : caractérisation 1 ; `test_pico8_jo:307` (lent) noté sans relance.
  5. Propre à l'étape, *aide sans `assert`* : `family_take_vec` recopie tout genre. Attendu rouge : test d'arrêt.
  6. Propre à l'étape, *écrivain muet* : la branche `ra` de `qh` rend 1 sans écrire. Attendu rouge : caractérisation 3 et 4 (`bv`).
  7. Propre à l'étape, *`hu` perd `p`* : la condition `self->p != NULL` de `pico8_jo_hu` inversée. Attendu rouge : caractérisation 4.
- **`gcov`** : `test_pico8_ui` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23, sur les 3 fichiers `.c` modifiés. Chaque ligne modifiée doit être exécutée ; une ligne manquée (probablement `hk`, H16) est cherchée dans **une** passe `gcov` d'un TU rapide cité en H16, sinon justifiée dans le commit. Trous attendus : l'échec des `assert` des aides d'écriture (fils de `must_abort` qui avortent sans vider leur compteur, prouvés par le test d'arrêt et la mutation 5) ; l'échec de `jo_default_vec` (inatteignable, la spec est un littéral).
- **Prior art** : `test_jo_members_by_name` et le test d'arrêt de 1b.23a (même fichier) ; `test_ke_me_by_name`, `test_ke_ks_stops_off_row` (1b.14a) ; test d'arrêt de `qb` (1b.19, `qb_take_vec`, cas `zero_ref`) ; cas `memset` de 1b.14b/1b.15/1b.18.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_ui` : 10 à 30 s. Une quinzaine de lancements (départ, arrêt rouge, final, 9 mutations, `gcov`) : ~5 min ;
- `./tmp/test_pico8_ga_export_panel_by` : quelques secondes, deux lancements (§6) ;
- `make test` : 20 s à 1 min, une fois ;
- `make e2e` : 4 à 6 min, une fois ;
- `make class-coverage-net` : 3 à 4 min, **dans la même invocation** que le `make e2e` (arbres distincts, 1b.0) ;
- `make sanitize-net` : ~4 min seul, **jamais** en même temps que `e2e`, une fois sur l'arbre final ;
- `make value-members` : quelques secondes.

**TU lents touchés** : `test_pico8_qh`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`, `test_pico8_nr` (tableaux `written[]`). `test_pico8_jo` et `test_pico8_ie` ne sont pas modifiés, mais `test_pico8_jo` exerce le code changé (`class_refs[]`, `write_ra`, clés `p`/`ra`). **Construire les six tôt**, juste après le typage et avant toute mutation (`make -j tmp/test_pico8_qh tmp/test_pico8_os tmp/test_pico8_hj tmp/test_pico8_pm tmp/test_pico8_mx tmp/test_pico8_nr tmp/test_pico8_jo`, compilation seule) ; ils ne sont lancés que par `make e2e`.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre (départ / final / mutation n / §6 bug remis) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les **8** TU de `UNIT_TESTS_SLOW` passent (`test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 153`**, 15 lignes `pico8_jo_t` ;
5. `timeout 15m make class-coverage-net` : **sortie identique à `tools/class_coverage_net_expected.txt`**. Elle s'applique parce que le diff touche des constructeurs (`pico8_jo_struct_init`, `pico8_jo_struct_new`, `pico8_pm_struct_new`, `pico8_ga_struct_new`, `pico8_nr_struct_new`), sans toucher `pico8_class_coverage_record` ni `KNOWN[]`. Aucun compte ne change.

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si 1b.23a1 ou 1b.23a2 n'est pas commité au départ, ou si `make value-members` ne rend pas 157 sur l'arbre de départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si une aide d'écriture ou de lecture de spec avorte dans un rejeu ou dans un TU hors des fils de `must_abort` (H2, H3), si un indicateur `has_<m>` est ajouté à l'un des 4 membres, ou si un type est élargi (union, `pico8_struct_header_t *`, `void *`, `pico8_value_t` gardé à côté) au lieu d'avorter.
- `FAILED:` si l'un des 4 membres n'a pas le type du tableau de recensement à sa place dans `pico8_jo_t`, ou si l'un des 15 autres membres est typé ou touché.
- `FAILED:` si `g_jo_members`, `g_jo_defaults` ou `keys[]` de `ga` liste encore l'un des 4 noms, ou si une autre de leurs entrées est retirée ou modifiée ; si une table de fille gagne ou perd une entrée.
- `FAILED:` si le défaut `gq` n'est pas le **pointeur de la spec** de `jo` (ni une copie, ni un `static pico8_vec_t` module), si `g_jo_gq`/`g_jo_ca` ne sont pas remis à nil dans `pico8_jo_defaults_reset`, ou si une fille pose `gq`/`ca`.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact pour un pointeur NULL, ou ne rend pas le même pointeur.
- `FAILED:` si l'écrivain de `qh` ou une boucle de clés n'accepte plus exactement les mêmes noms (les écrivains de `jo`/`ga` qui prennent `ra`/`p`, ou les boucles de `qh nl os hj mx fr` qui prennent la clé `p`/`ra`, comptent comme un changement) ; si une aide ne pose pas nil d'abord, n'avorte pas sur un non-REF à union nulle, une chaîne NULL, une fonction NULL, une liste, une table `ob` ou une struct, ou si son message ne nomme pas `jo.ra/p` et `ref:autre` ; si une aide est partagée entre fichiers ou rendue publique.
- `FAILED:` si un site par nom de `src/` hors `pico8_planet*` change, ou si un site par nom est remplacé par un accès direct.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si le diff d'un test fait autre chose que : ajouter la caractérisation, le test d'arrêt, `JO_POSITION_HASH` et leurs appels à `test_pico8_ui` ; retirer `"ra"`, `"p"` des 8 tableaux `written[]` ; changer les deux lignes `jo.ra`, `jo.gq` de `test_pico8_ga_export_panel_by`. Idem si une valeur figée existante change (H5).
- `FAILED:` s'il manque les deux lignes « Runs » de la vérification §6 de `test_pico8_ga_export_panel_by` (rouge avec le bug remis, vert restauré).
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 153, ou si un autre typedef que `pico8_jo_t` gagne ou perd une ligne.
- `FAILED:` s'il manque une ligne « Runs » de `make class-coverage-net` identique à la référence, ou si le diff touche `pico8_class_coverage_record` ou `KNOWN[]`.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, si elle lit ou écrit l'un des 4 membres directement, si elle ne couvre pas les 10 genres, les 4 noms et les 4 boucles de clés, ou si `JO_POSITION_HASH` change après ; si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'arbre de départ et une verte sur l'arbre final, ou s'il n'a pas les cas à union nulle.
- `FAILED:` si le commit ne contient pas son rapport `gcov` et ses mutations (1, 2, 3a–3e, 4 à 7), chacune avec son test ou la justification de son trou.
- `FAILED:` si le travail n'est pas en un commit vert, ou si un membre de 1b.23c–d y est mêlé.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.23b », ou si elle ne porte pas les lectures nouvelles (`gq` pointeur de spec et non défaut module ; aides d'écriture par fichier partagées entre écrivain et boucles de clés de plusieurs structs ; R7 sur `keys[]` de `ga`), ou s'il manque le tableau « Runs ».

## Out of Scope

- Les 15 autres membres de `jo` (1b.23c–d), dont `ma` de `g_jo_defaults` et `id` des boucles de clés et de `keys[]` de `ga`.
- Les membres propres des filles (`qh.et`, `pm.ng`, `ga.qo`…) : 1b.24 et suivantes.
- Les homonymes `ra`/`p`/`gq`/`ca` d'autres structs (`nf`, `qb`, `ie`, `iv`, `g_`, `fv`, `mz`, `js`, `jc`, `pj_cr`) : leurs étapes.
- Les lecteurs de spec `pico8_X_default` et les specs `ob` (phase D) ; un tag pour `pico8_vec_t`.
- Remplacer un site par nom (`probe.c`, `entity*.c`, `msgbox.c`, `slipway*.c`, `wormhole*.c`, `ui.c`, `economy_formulas.c`) par un accès direct.
- Ajouter ou retirer un nom d'un écrivain ou d'une boucle de clés.
- Régénérer `tools/member_census.txt`.
- Les fichiers d'événements et `FRAMES_EVENTS`, tout changement de rendu, l'audio, lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `planet.[ch]` (type, lecteur, init, boucle de clés de `jo`, `hu`, `hk`, `jo_take_vec`, `jo_default_vec`), `planet_family.c` (écrivain de `qh`, boucles de `pm` et `ga`, `keys[]`, `pm_pq`, `family_take_vec`), `planet_family_trade.c` (boucle de `nr`, `family_trade_take_vec`) ;
  - tests : `test_pico8_ui` étendu (caractérisation des 4 membres sur les 10 genres et les 4 boucles, test d'arrêt rouge d'abord) ; 8 tableaux `written[]` réduits ; deux lignes du test de régression. Pas de nouveau TU ;
  - pas de découpage (14 fichiers, sous 1b.23a2) ;
  - `gq` → pointeur de spec (R7 de 1b.14a), pas défaut module (1b.17/1b.19) ; `ca` sans indicateur ; `p`, `ra` sans indicateur ;
  - une aide d'écriture par fichier et par genre, partagée entre l'écrivain et les boucles de clés du fichier ;
  - `class-coverage-net` lancé une fois, sur l'arbre final.
- **Note « Découvert à 1b.23b »**, à écrire sur la ligne de l'étape : « patron 1b.1 appliqué ; `gq`, `p`, `ra` → `pico8_vec_t *` (1b.17), `ca` → `pico8_fixed_t`, aucun indicateur ; **`gq` : défaut = pointeur de la spec** (R7 de 1b.14a, `g_jo_gq` déballé par `jo_default_vec`), et non défaut module de 1b.17/1b.19, parce que les TU lents figent l'identité avec la spec ; **lecture de forme étendue aux écrivains** : une aide d'écriture par genre et par fichier (`jo_take_vec`, `family_take_vec`, `family_trade_take_vec`), partagée entre l'écrivain et les boucles de clés de toutes les structs du fichier (`qh`, `pm`, `ga`) ; **R7 sur une table de clés de fille** (`keys[]` de `ga`) : entrée retirée, table gardée, branche typée avant la recherche ; boucles sentinelles de 1b.23a2 recopiées (8 TU, dont 6 lents) ; limite `ref:autre` vecteur/boîte (tag `UNKNOWN`) gardée ; `g_jo_defaults` ne garde que `ma` ; empreinte réelle ≈ 35 (15, 20), 14 fichiers modifiés. » Le reste va dans le commit.
- Le message de commit porte le périmètre, le compte, le tableau des poses et des boucles de clés (valeur et origine du pointeur), le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous, `JO_POSITION_HASH` et les réponses à H1–H18.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
