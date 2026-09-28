---
title: "Plan 3, étape 1b.23c : `jo`, état lu — `id`, `oe`, `ep`, `jy`, `fx`, `lq`, `jh`, `p_` typés, premiers indicateurs de `jo`, écrivain à sept branches et clé `id` dans les dix constructeurs"
description: "`pico8_jo_t` garde 15 membres `pico8_value_t` après 1b.23b. 1b.23c en type 8 selon le patron R1–R9 de 1b.1 : deux chaînes (`id`, `ep`), deux listes (`oe`, `jy`), une rangée `pico8_lq_t *` (cible 1b.10), et les trois premiers non-pointeurs à indicateur de `jo` (`fx`, `p_` booléens, `jh` fixe, R2). Sept branches d'écrivain sous la forme 1b.13/1b.15, la clé `id` déballée dans les dix boucles de clés, les boucles sentinelles de 10 TU réduites. `make value-members` passe de 153 à 145. Un seul commit (17 fichiers)."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.23c — état lu : `id`, `oe`, `ep`, `jy`, `fx`, `lq` (→ 1b.10), `jh`, `p_`. Empreinte 36 (14, 22). Compte : 145.

1b.23c ne type que ces 8 membres. `kr`, `cc`, `by`, `mi`, `m_`, `fo`, `ma` sont 1b.23d.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité, pas redécidé. Les notes « Découvert à » de `docs/plan-3.md` s'appliquent, citées et non redérivées :

- **1b.1** : R1 à R9 ; `chaine` → `const char *`, `bool` → `int` (0/1), `fixe` → `pico8_fixed_t`, `ref:liste` → `pico8_list_t *`, `ref:pico8_X_t` → `pico8_X_t *`, pointeurs non `const` sauf la chaîne ; R2 : `int has_<m>` juste après `<m>` pour un non-pointeur lu par nom (`l`) à nil `observé` ; un pointeur n'en a pas (NULL = nil) ; R5 « même règle dans l'écrivain `set_field` et dans la boucle des clés de construction ».
- **1b.3, 1b.4, 1b.4b** : R3 lu sur ce qui initialise ; pointeur emprunté, même pointeur ; boucles sentinelles `written[]` arrêtées au recensement (`PICO8_MEMBER_CENSUS_STOP()`).
- **1b.5, 1b.7, 1b.16 — lectures de R1 pour les genres mêlés** : **sans objet ici**, chaque ligne de l'étape n'a qu'un genre (voir le recensement). Citées pour les écarter : `fx` et `p_` sont `bool` seul, `ep` `chaine` seul (la forme 1b.7 {bool, chaine} ne s'applique pas) ; aucun `<m>_is_bool`.
- **1b.9** : R7 forme `offsetof` ; messages d'aide qui nomment tous leurs membres et le genre de leur ligne ; R5 par aide `static` par genre.
- **1b.10** : R5 en boucle de clés ; `lq` typé, **accesseur public `pico8_lq_struct_class()`**, cible de `jo.lq` ; lecture de R5 pour `ref:autre` (sans objet ici).
- **1b.11 — R2, TU lents** : `CENSUS_UNIT_TESTS` exclut `UNIT_TESTS_SLOW` ; ce PRD relit les TU lents de l'arbre `jo` (revue plus bas).
- **1b.12** : patron appliqué, rien de propre à l'étape.
- **1b.13 et 1b.15 — forme écrivain de R5** : branche `strcmp` par membre, ordre du typedef ; aide `static` par genre, partagée avec la boucle de clés du même fichier, jamais entre fichiers ; pose **d'abord** l'état nil (pointeur NULL ; non-pointeur 0 **et** `has_<m>` = 0), sort sur `REF NULL` (genre **et** pointeur), sinon `assert` « champ sans règle » qui nomme `struct.membre` et le genre ; `ref:pico8_X_t` vérifie tag `STRUCT` **et** descripteur par l'accesseur public ; 1b.15 : aide à indicateur (`bg_take_bool`) — nil éteint `has_` quel que soit l'état d'avant, un `false` à union nulle reste un `false` présent, `has_` = 1 posé en dernier ; un `must_abort` rouge sur l'ancien code fige l'arrêt.
- **1b.14a — R7 sur une table qui garde des entrées et a des filles** : l'entrée sort de toutes les tables qui la listent, chaque table garde ses autres entrées et sa boucle ; pose typée après la boucle, avant clés et `pq`. Ici aucun des 8 membres n'a de défaut de spec (`g_jo_defaults` garde `ma` seul) : seuls les points (1) et (4) s'appliquent ; aucun `static` déballé.
- **1b.14b** : cas « union nulle » par `memset` puis `kind` ; un fixe 0 à union nulle est le témoin accepté d'un fixe ; mutation d'un appel unique d'aide `static` plantée sous `if (0)` (`-Werror`).
- **1b.16** : lecture de R1 {fixe, REF} écartée (voir 1b.5 ci-dessus).
- **1b.17 à 1b.21** : aide partagée entre écrivain et écriture directe d'une valeur lue ailleurs (`fq_take_bool`, 1b.18) ; **1b.20** : les `assert` de lecture sur la cible passent à l'écriture (`ie_take_jb`) ; **1b.21** : chaîne NULL refusée en clé, site C1 de `float-audit` réécrit seul.
- **1b.22 — REF à plusieurs cibles** : sans objet (`lq` n'a qu'une cible). Citée pour l'écarter.
- **1b.23a1** (note « Découvert à ») : **patron `static` de `jo`** (déballage à l'init, remise à nil dans `defaults_reset`) — sans objet ici, aucun défaut ; **aides par fichier** (`jo_*`, `family_*`, `family_trade_*`), une par genre, message aux noms de ses membres ; **H11 démentie en partie** : les filles de `qh` passent par `jo:lz` dans leur `pq` — d'où le tableau `lz_at_pq[10] = {0, 1, 1, 1, 1, 1, 0, 1, 1, 1}` de `test_jo_members_by_name`, que la caractérisation de 1b.23c reprend pour `ep` et `jy` (que `lz` écrit).
- **1b.23a2** (note « Découvert à ») : un membre `l` seul que l'écrivain de 1a accepte déjà garde sa branche (lecture nouvelle du point 1 de 1b.13) — **s'applique à `ep`, `jy`, `fx`, `lq`, `jh`** ; aide d'écriture partagée avec les écritures directes du même fichier (`jo_take_table` de `hh`/`hu`) ; **boucles sentinelles** : un membre typé sort des tableaux `written[]` des TU, son écriture par nom est figée par la caractérisation de l'étape, fonction distincte et hash propre ; **H16 démentie** : `test_pico8_planet_gh_row_lq` (§6) écrivait `planet->kw` directement, que le grep `jo\.kw` ne voyait pas — ce PRD greppe donc `->m` et `.m` sur tous les TU, pas seulement `jo\.m` (revue plus bas).
- **1b.23b** (PRD, supposé commité) : aides d'écriture par fichier partagées entre écrivain et boucles de clés de plusieurs structs (`jo_take_vec`, `family_take_vec`, `family_trade_take_vec`) ; R7 sur une table de **clés** de fille (`keys[]` de `ga`) : entrée retirée, table gardée, branche typée avant la recherche.

### État de départ

HEAD `c790321` (PRD de 1b.23b). 1b.23b est dans l'arbre de travail, en cours d'implémentation. **Précondition : 1b.23b est commitée avant que 1b.23c commence** (H8). `make value-members` rend alors **153** (H1). Les lignes citées glissent : chaque site est aussi désigné par son contenu.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code (HEAD et arbre 1b.23b en cours). Tout fait d'exécution est une hypothèse (H1 à H21).

### Lignes de recensement (`tools/member_census.txt:70-82`)

| membre | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|
| `id` | `chaine` | observé | lu | l | `const char *` | non (pointeur) |
| `oe` | `ref:liste` | observé | stat | **le** | `pico8_list_t *` | non (pointeur) |
| `ep` | `chaine` | observé | lu,stat? | l | `const char *` | non (pointeur) |
| `jy` | `ref:liste` | observé | lu,stat | l | `pico8_list_t *` | non (pointeur) |
| `fx` | `bool` | observé | lu | l | `int` | **`has_fx`** |
| `lq` | `ref:pico8_lq_t` | observé | lu,stat? | l | `pico8_lq_t *` | non (pointeur) |
| `jh` | `fixe` | observé | lu,stat | l | `pico8_fixed_t` | **`has_jh`** |
| `p_` | `bool` | observé | lu | **le** | `int` | **`has_p_`** |

8 membres : 153 − 8 = **145** (les `has_` ne comptent pas, R9). Les 7 autres lignes `pico8_jo_t` (`ma fo kr cc by mi m_`) ne bougent pas.

- **Un seul genre par ligne.** Aucun genre mêlé, aucune REF à plusieurs cibles.
- **Trois indicateurs, les premiers de `jo`** : `fx`, `jh`, `p_` sont des non-pointeurs lus par nom à nil `observé` avec preuve `lu` : R2 donne `has_<m>` sans débat. La revue des TU lents le confirme (`bool_state(e, "p_")`, `fold_field(e, "fx")`, lectures de planète nue).
- **`p_`, `oe` ont `e`** : `p_` est écrit par nom par `d_` (`probe.c`, `pico8_entity_set_field(self, "p_", pico8_value_bool(1))`, atteint sur les planètes de la famille `qh` par `EY`) ; `oe` par les TU `test_pico8_nr` et `test_pico8_fr` (listes, dans le genre). `mx` écrit `jy` par nom (`test_pico8_mx`, `give_row`, une liste), sans que la ligne ait `e` (TU lent, 1b.11).

### Revue des TU lents (R2 de 1b.11)

Relus pour ces 8 noms : `test_pico8_jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr` (arbre `jo`) et `test_pico8_ie`. Constats :

- listes `members[]`/`jo_members[]` (les 31 noms) pliées par `fold_field` : présence, genre, identité ;
- `scalars[]` qui contient `id` (tous), `fx`, `ep` (`jo:336`) ; `lists[] = {"oe", "jy", …}`, `refs[] = {…, "lq", …}` (`jo:337-338`) ; `list_size(e, "oe"/"jy")` (tous) ;
- **lectures nil par nom** : `bool_state(e, "p_")` (tous, 0 = absent), `!has(bare, "id")` (tous), `has(bare, "oe")`, `fold_field(e, "fx")` (`mx:561`) ;
- **écritures par nom dans le genre** : `oe` ← liste (`nr:502`, `nr:509`), `jy` ← liste (`mx:553`) ; `id` ← chaîne sur un écrivain qui ne le prend pas, dans un `must_abort` (`write_id`, `jo:720`, `qh:814`, `hj:770`, `os`, `pm`) ;
- identité : `get_ref(a, "oe") != get_ref(b, "oe")` (`nr:405`) ;
- `test_pico8_ie` lit l'`id` **des planètes que construit `ie:ldown`** (`is_string(made.as.ref, "id", "s1")`, `fold_value`, `ie:553-559`) ; ses autres `id`/`p_`/`fx` sont ceux d'`ie`, homonymes ;
- **les boucles sentinelles `written[]`** de `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr` (7 lents) et de `nl`, `fr`, `ga` (3 rapides) écrivent `pico8_value_fixed(i + 1)` sur `ep`, `fx`, `jh`, `jy`, `lq`, `oe`, `p_` et le relisent `FIXED`. Typés, ces membres avortent (voir « Boucles sentinelles »).

R2 ne donne aucun indicateur de plus que le recensement : les 5 pointeurs n'en ont pas (NULL = nil), les 3 non-pointeurs en ont un.

### Revue des tests de régression (§6)

Le PRD de 1b.23a a manqué `test_pico8_planet_gh_row_lq` (`planet->kw`), et celui de 1b.23b `test_pico8_planet_gh_row_lq` et `test_pico8_planet_kj_dispatch` (`planet->ra`, `planet->gq`) : leurs greps ne cherchaient que `jo\.m`. Ici, sur **tous** les TU :

- `grep -nE '(->|\.)(id|oe|ep|jy|fx|lq|jh|p_)\b' tests/*.c` rend **3 lignes, toutes de `test_pico8_iv.c`** (`f->p_` d'un `fake_toast_t`, homonyme) ;
- `grep -rnE 'offsetof\(pico8_(jo|qh|nl|os|hj|pm|ga|nr|fr|mx)_t' tests/` : vide.

**Aucun test de régression n'accède directement à ces 8 membres.** Trois les lisent **par nom**, inchangés (puce Coexistence) : `test_pico8_built_planet_panel_nil_row` (`id`, `ep` de `p1`), `test_pico8_rich_planet_exports` (`id`, `jy`), `test_pico8_slipway_drag` (`oe`). Les autres lectures `"lq"` de `ga_export_panel_by`, `planet_gh_row_lq`, `nil_row` visent le `lq` d'un bouton `js` (homonyme, 1b.34). Aucune vérification §6 « bug remis » n'est donc due ; ces trois tests restent verts dans `make test` (H15).

### Ce que fait le cart (`pico8/lua.lua`)

- **Aucun défaut** dans la spec de `jo` ni des filles pour ces 8 noms.
- `id` : clé de construction seule — `bw` (`jo({id="p"..id,…})`, `galaxy.c`), `ie:ldown` (`self.ke({id="s"..ie.id})`, `research_icon.c`), `jo:hu` (`kv[hp.cw]({p=…,ra=…,id=self.id})`). Toujours une chaîne. Lu par `jc:ch` (`self.hg[t.id]`), `bd` (`w.hg[e.lp[i+1].id]`).
- `oe` : `jo:pq` (`self.oe={}`), `jo:kf` (`add(self.oe,sw)`), lu par `jc:ce` (`self.qu.oe`), `nr:gv`/`nr:hb`/`nr:cn` (`#self.oe`, boucle), `jo:ib` (`for w in all(self.oe)`).
- `ep` : `jo:lz` (`self.hn,self.ep=hp` : `ep` = nil), `jo:hh` (`self.ep=self.nk.d`, chaîne de `df` ou nil pour `ll`). Lu par `pz` (`if p.ep`).
- `jy` : `jo:lz` (`nv(hp.i,mw)`), `jo:cn` (`la({{qe=r.e}},self.jy)`, suppression en place), `bb` (`add(pt.jy,…)`), lu par `gh`, `hb`, `bx`, `ck`, `dh`, `mx:kf`.
- `fx` : `jo:hu` (`self.fx=true`), `jo:kf` (`if (sw:eb(self).fx) self.fx=true`). Seul `true` est écrit. Lu par `mz:pq` (`p.fx`) et `jo:kf`.
- `lq` : `jo:hb` (`self.lq=jo.ex(…)`), lu par `jo:kj` (`mv(self.lq,"ol",…)`).
- `jh` : `jo:cn` (`self.jy,self.jh=…,1`), lu par `jo:cn` (`not self.jh`) et `bb` (`pt.jh or 0`).
- `p_` : `jo:ll` (`t>60`, vrai **ou faux**), `jo:hu` (`true`), `ga:kj` (`not pt`), `d_` (`true`) ; lu par `dg` (`if qc.p_ then he(qc)`).

### Le code (arbre 1b.23b supposé commité)

- **Typedef** (`planet.h`) : `id` après `em` ; `oe` après `ra` ; `ep`, `jy` après `hn` ; `fx`, `lq` après `nk` ; `jh`, `p_` après `fo` ; tous `pico8_value_t`. `planet.h` n'inclut pas `pico8_layout.h` (`pico8_lq_t`) ni `pico8_list.h` directement.
- **`g_jo_members[]`** (`planet.c`) : après 1b.23b, 15 entrées dont nos 8. Sert au lecteur `pico8_jo_field` (repli), à l'écrivain (`written[]`), à la boucle nil de `pico8_jo_struct_init` et au `else` générique de la boucle de clés de `pico8_jo_struct_new` (qui n'y envoie plus que `id`).
- **Écrivain** `pico8_jo_set_field` : branches typées `kw`, `hn`, `nk`, puis `written[]` = `by cc ep fo fx jh jy kr lq m_ ma mi oe p_` (14) et `*pico8_jo_member(self, name) = value`. **7 de ces 14 noms sont de l'étape** : `ep`, `fx`, `jh`, `jy`, `lq`, `oe`, `p_`. `id` n'y est pas (arrêt `write_id` des TU lents). Les écrivains des 9 filles délèguent à celui de `jo` pour ces noms.
- **Boucles de clés qui écrivent `id`** (R5 boucle), **dix** :

| constructeur | fichier | forme aujourd'hui |
|---|---|---|
| `pico8_jo_struct_new` | `planet.c` | `else *pico8_jo_member(self, key.name) = value` (après les branches `p`/`ra` de 1b.23b) |
| `pico8_qh_struct_new`, `pico8_nl_struct_new`, `pico8_os_struct_new`, `pico8_hj_struct_new` | `planet_family.c` | `assert(measured …)` puis `self->…jo.id = value` |
| `pico8_pm_struct_new` | `planet_family.c` | branche `else` après `is_p`/`is_ra` : `self->hj.qh.jo.id = value` |
| `pico8_ga_struct_new` | `planet_family.c` | **`keys[]`** : `{"id", offsetof(pico8_ga_t, jo.id)}` + 8 membres de `ga` |
| `pico8_nr_struct_new` | `planet_family_trade.c` | branche `id` directe |
| `pico8_fr_struct_new`, `pico8_mx_struct_new` | `planet_family_trade.c` | `assert(measured …)` puis `self->…jo.id = value` |

- **Écritures directes (R6)** :
  - `planet.c` : `jo:pq` (`self->oe = pico8_value_ref(oe)`), `jo:lz` (`self->ep = pico8_value_ref(NULL)`, `self->jy = pico8_value_ref(jy_heap)`), `jo:ll` (`self->p_ = pico8_value_bool(t > 60)`), `jo:hu` (`self->p_ = pico8_value_bool(1)`, `self->fx = pico8_value_bool(1)`), `jo:hh` (`self->ep = d_val`, un `pico8_value_t` lu de `nk`) ;
  - `planet_display.c` : `jo:hb` (`self->lq = pico8_value_ref(lq_result)`) ;
  - `planet_trade.c` : `jo:kf` (`self->fx = pico8_value_bool(1)`), `jo:cn` (`self->jy = pico8_value_ref(merged_heap)`, `self->jh = pico8_value_fixed(1)`) ;
  - `planet_family.c` : `ga:kj` (`self->jo.p_ = pico8_value_bool(last_pt == NULL)`).
- **Lectures directes (R6)** :
  - `planet.c` : `jo:hu` (`pico8_jo_present(self->id)`, recopié en clé) ;
  - `planet_display.c` : `jo:hb` et `jo:gh` (`list_of(self->jy)`), `jo:kj` (`self->lq`, avec `assert` tag `STRUCT` + `cls == pico8_lq_struct_class()` « jo.lq must be the row jo.ex built ») ;
  - `planet_trade.c` : `jo:kf` et la boucle de `jo:ib` (`self->oe.kind/.as`), `jo:cn` (`self->jy.kind/.as`, `!pico8_jo_truthy(self->jh)`) ;
  - `planet_family_trade.c` : `nr:gv`, `nr:hb`, `nr:cn` (`self->qh.jo.oe.kind/.as`, `PICO8_REF_AS_LIST_CONST`).
- **Tables d'offsets qui listent ces 8 membres** : `grep -rnE 'offsetof\(pico8_[a-z0-9_]+_t, ([a-z0-9_]+\.)*(id|oe|ep|jy|fx|lq|jh|p_)\)' src/ | grep -E 'pico8_(jo|qh|nl|os|hj|pm|ga|nr|fr|mx)_t'` rend **9 entrées** : 8 de `g_jo_members`, 1 (`jo.id`) de `keys[]` de `ga`. Aucune table de défauts ni de membres de fille ne les liste.
- **Sites par nom, inchangés** (puce Coexistence) : `galaxy.c`, `research_icon.c` (clé `id`), `economy_tiers.c` (`id`, `jy`, `jh`), `economy_formulas.c` (`jy`), `slipway.c` (`oe`), `slipway_display.c` (`id`), `msgbox.c` (`ep`), `bootstrap.c` (`p_`), `probe.c` (`p_`, écrit), `wormhole.c` (`fx`), `planet_trade.c` (`fx` de `sw:eb(self)`), `planet_family_trade.c` (`jy` de `mx:kf`), et les TU.
- **Homonymes hors étape** : `pj_cr.ep` (`score_panel.c`), `mz.p_` (`wormhole.c`), `js.lq`/`js.p_`, `g_.fx`/`g_.p_`, `ie.id`, `mh.p_`, `iv.p_`, `hp.lq`, `jc.p_`.

### Coexistence : 8 membres de plus typés, 7 non typés

Même forme qu'en 1b.23a/b :

- **Lecteur** `pico8_jo_field` : 8 branches R4 de plus, dans l'ordre du typedef, avant `pico8_jo_member`. Pointeur NULL → `return 0`, `*out_value` intact ; `has_<m>` = 0 → idem. Les lecteurs des filles délèguent déjà.
- **Écrivain** `pico8_jo_set_field` : 7 branches R5 (`oe`, `ep`, `jy`, `fx`, `lq`, `jh`, `p_`) avant la boucle ; `written[]` garde `by cc fo kr m_ ma mi`. L'ensemble des 17 noms accepté ne change pas ; `id` reste refusé.
- **Tables** : `g_jo_members[]` perd les 8 entrées (7 restent) ; `keys[]` de `ga` perd `jo.id` et ne garde que les 8 membres propres de `ga`. **Piège** (même qu'en a2/b) : un nom de `written[]` ou de clé sans entrée dans `g_jo_members[]` ferait déréférencer NULL. Les branches typées passent **avant** l'appel à `pico8_jo_member`, et le `else` générique de la boucle de clés de `jo` disparaît (plus aucune clé ne l'atteint).

### Empreinte remesurée ; découpage

La ligne dit 36 (14, 22). Par greps de noms, le majorant dépasse 50, gonflé par homonymie (`id` d'`ie` ; `p_` de `mz`, `mh`, `iv`, `js`, `jc`, `g_`, `fv`, `fq` ; `lq` de `js`, `hp`, du menu ; `ep` de `pj_cr` ; `fx` de `g_`).

**Empreinte réelle** (fichiers qui lisent ou écrivent l'un des 8 membres d'une struct de l'arbre `jo`), par lecture : **≈ 37 (16, 21)**, sous le seuil de ~40.

- **src (16)** : `planet.h`, `planet.c`, `planet_display.c`, `planet_trade.c`, `planet_family.c`, `planet_family_trade.c`, `galaxy.c`, `research_icon.c`, `economy_tiers.c`, `economy_formulas.c`, `slipway.c`, `slipway_display.c`, `msgbox.c`, `bootstrap.c`, `probe.c`, `wormhole.c`.
- **tests (≈ 21)** : `test_pico8_jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`, `ie`, `nl`, `fr`, `ga`, `ui`, `hp`, `jc`, `entity_kind`, `built_planet_panel_nil_row`, `rich_planet_exports`, `slipway_drag`, `jo_spec_defaults`, `spec_defaults` (H21), `empty_planet_cleared` (par `hv`).

**Fichiers modifiés : 17**, plus `docs/plan-3.md` :

- **src (6)** : `pico8_planet.h`, `pico8_planet.c`, `pico8_planet_display.c`, `pico8_planet_trade.c`, `pico8_planet_family.c`, `pico8_planet_family_trade.c` ;
- **tests (11)** : `test_pico8_ui.c` (ajouts : caractérisation, test d'arrêt) ; les tableaux `written[]` de `test_pico8_jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr` (**7 TU lents**), `nl`, `fr`, `ga`. **Aucun test de régression** (revue §6 ci-dessus).

**Découpage (G) : aucun, un seul commit.** 17 fichiers, sous les 18 de 1b.23a2, qui a tenu en une passe ; le seuil de la consigne (« nettement plus de ~18 ») n'est pas franchi. Écarté (G) : 1b.23c1 (pointeurs `id`, `oe`, `ep`, `jy`, `lq`) puis 1b.23c2 (`fx`, `jh`, `p_` à indicateur) — les deux moitiés touchent les **mêmes** 6 fichiers `src/` (le lecteur, l'écrivain, `struct_init`, `hu`, `cn`) et les **mêmes** 10 tableaux `written[]` (chacun porte des noms des deux moitiés) : 17 fichiers deux fois, deux `make e2e`, deux `class-coverage-net`, deux passes sur 7 TU lents, hors du budget de 20 minutes, pour aucune forme isolable ; et un découpage par membre est impossible (un membre change de type pour tous ses sites à la fois).

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (arbre 1b.23b commité) : les 8 membres lus par nom sur `jo` et ses 9 filles, neuves et vivantes (début et fin du rejeu `DRAG`), les dix boucles de clés `id`, l'écrivain par aller-retour dans le genre, les transitions `pq`, `lz`, `hh`, `hu`, `ll`, `hb`, `ga:kj`, `nr:hb`. Verte avant tout typage.
2. **Test d'arrêt, rouge sur l'ancien code** : écrivain et boucles de clés hors genre, avec les cas à union nulle.
3. **Commit 1b.23c** : les 8 membres typés ; `has_fx`, `has_jh`, `has_p_` ; entrées sorties de `g_jo_members` et de `keys[]` de `ga` ; 7 branches d'écrivain et 10 boucles de clés par aides par fichier ; lectures et écritures directes réécrites ; 7 noms sortis de 10 tableaux `written[]`. `make value-members` : **145**.
4. Aucun pixel ne bouge. La note « Découvert à 1b.23c » va sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `planet.h`, je veux `id`, `ep` typés `const char *`, `oe`, `jy` `pico8_list_t *`, `lq` `pico8_lq_t *`, afin de lire l'état d'une planète sur son type.
2. En tant que lecteur de `planet.h`, je veux `fx`, `p_` typés `int` avec `has_fx`, `has_p_`, et `jh` `pico8_fixed_t` avec `has_jh`, afin que nil et `false` (ou 0) restent distincts.
3. En tant que lecteur par nom (`economy_*`, `slipway*.c`, `msgbox.c`, `bootstrap.c`, `probe.c`, `wormhole.c`, les TU), je veux que chacun des 8 noms rende exactement ce que rendait l'ancien code sur les 10 genres de l'arbre `jo` : présence, genre, valeur, identité du pointeur.
4. En tant que planète neuve, je veux `oe` présent (liste vide propre, posée par `pq`), `id`, `ep`, `fx`, `lq`, `jh`, `p_` absents, et `jy` présent exactement là où `lz` passe dans `pq` (`lz_at_pq`).
5. En tant que galaxie, `ie:ldown` ou `jo:hu`, je veux que la chaîne passée en clé `id` soit le pointeur même que la planète garde.
6. En tant que `jo:ll`, je veux poser `p_` à `false` **présent** quand `t ≤ 60`, comme en 1a (le cart écrit `t>60`, pas `true or nil`).
7. En tant que `dg` (`bootstrap.c`), je veux lire `p_` par nom et retirer la planète exactement quand il est vrai.
8. En tant que `d_` (`probe.c`), je veux écrire `p_` = vrai par nom sur une planète de la famille `qh`.
9. En tant que `jo:hh`, je veux que `ep` reçoive la chaîne `d` de `nk`, ou nil, par la même aide que l'écrivain.
10. En tant que `jo:cn`, je veux lire « `jh` absent » par `has_jh`, et poser `jy` puis `jh` = 1.
11. En tant que `jo:hb`/`jo:kj`, je veux poser et lire `lq` par le pointeur typé, la vérification de descripteur passant à l'écriture par nom (1b.20).
12. En tant que `nr:gv`, `nr:hb`, `nr:cn`, `jo:kf`, je veux compter et parcourir `oe` par le pointeur, 0 quand NULL.
13. En tant qu'écrivain par nom de l'arbre `jo`, je veux `oe`, `ep`, `jy`, `fx`, `lq`, `jh`, `p_` acceptés dans leur genre ou nil, même pointeur, et tout autre genre arrêté par un `assert` « champ sans règle » qui nomme `jo.<m>` et le genre de sa ligne.
14. En tant que boucle de clés de chacun des 10 constructeurs, je veux la même règle pour `id`, par l'aide de chaîne de mon fichier.
15. En tant que mainteneur, je veux que l'ensemble des noms qu'acceptent l'écrivain (17) et chaque boucle de clés ne change pas (`id` toujours refusé par l'écrivain).
16. En tant que mainteneur, je veux qu'aucune table de défauts, d'offsets ou de clés ne liste plus ces 8 noms, et qu'aucune écriture `*(pico8_value_t *)` ne les atteigne.
17. En tant que TU à boucle sentinelle, je veux que ma boucle ne réécrive plus en fixe ces 7 noms, leur écriture par nom étant figée une fois par la caractérisation.
18. En tant que test de régression qui lit ces membres par nom (`nil_row`, `rich_planet_exports`, `slipway_drag`), je veux rester vert sans modification.
19. En tant qu'auteur de 1b.23d, 1b.24, 1b.32–1b.35, je veux une forme écrite pour une aide à indicateur partagée entre écrivain et écritures directes (`jo_take_bool`, `jo_take_fixed`), et pour une clé déballée dans les dix constructeurs de l'arbre.
20. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
21. En tant que gardien de la mémoire et de la couverture, je veux `make sanitize-net` à 15 propres et `make class-coverage-net` identique à sa référence.
22. En tant que `plan-pilot`, je veux `make value-members` à 145.
23. En tant que vérificateur, je veux la caractérisation verte sur l'ancien code, le test d'arrêt rouge puis vert, les mutations du §7 et leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés comme le ferait un agent `guideline` (hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables), et marqués (G).

### R1–R9 appliquées

| règle | sur les 8 membres |
|---|---|
| R1 | Tableau de recensement. Nom et place dans le typedef gardés ; `has_<m>` juste après `<m>`. Tous empruntés, jamais possédés ni libérés : chaînes de `bw`/`ie:ldown` (`galaxy.c` le dit déjà : « the instance's "id" field keeps it »), de `df`, listes de `pq`/`lz`/`cn`, rangée de `jo.ex` (TROU-7 inchangé). `planet.h` inclut `pico8_layout.h` (G, H18). |
| R2 | `has_fx`, `has_jh`, `has_p_` (recensement, revue des TU lents). Aucun pour les 5 pointeurs. |
| R3 | `pico8_jo_struct_init`, à la place de la boucle nil pour ces noms, avant clés et `pq` : `id = NULL`, `oe = NULL`, `ep = NULL`, `jy = NULL`, `fx = 0`, `has_fx = 0`, `lq = NULL`, `jh = 0`, `has_jh = 0`, `p_ = 0`, `has_p_ = 0`. Aucun défaut de spec. |
| R4 | 8 branches dans `pico8_jo_field`, ordre du typedef, avant `pico8_jo_member`. Pointeur NULL ou `has_` = 0 → `return 0`, `*out_value` intact. Sinon `pico8_value_string(id/ep)`, `pico8_value_ref(oe/jy/lq)`, `pico8_value_bool(fx/p_)`, `pico8_value_fixed(jh)`. Aides `jo_field_string`, `jo_field_list` à côté de `jo_field_table`/`jo_field_vec` (G). |
| R5 | Écrivain et 10 boucles de clés : voir plus bas. |
| R6 | Voir « Lectures et écritures directes ». |
| R7 | Entrées de `g_jo_members` (8) et de `keys[]` de `ga` (1) retirées, remplacées par R3/R5. Aucune table de défauts touchée (1b.14a points (1) et (4) seuls). |
| R8 | Crochets `PICO8_MEMBER_CENSUS_*` gardés. `tools/member_census.txt` non édité. |
| R9 | 153 → 145. Restent 7 lignes `pico8_jo_t` : `ma fo kr cc by mi m_`. |

### R5 : aides par fichier et par genre (G)

Lecture de forme de 1b.23a1 étendue par 1b.23b aux aides d'écriture ; 1b.13 point 2 (jamais partagée entre fichiers, partagée avec la boucle de clés et les écritures directes du même fichier, 1b.18).

- **`planet.c`** (5 nouvelles, à côté de `jo_take_table`, `jo_take_hp`, `jo_take_vec`) :
  - `static void jo_take_string(pico8_value_t value, const char **out)` : `*out = NULL` ; `return` sur `REF NULL` ; sinon `assert(value.kind == PICO8_VALUE_STRING && value.as.string != NULL && "jo.id/ep: a write neither nil nor a string is off its census row (chaine) -- a field under no rule of plan-3 section 5: stop, do not widen the member")`. Appels : clé `id` de `pico8_jo_struct_new`, branche `ep` de l'écrivain, `ep` de `jo:hh` (`d_val`, 1b.18). Chaîne NULL refusée (1b.21).
  - `static void jo_take_list(pico8_value_t value, pico8_list_t **out)` : tag `PICO8_TYPE_TAG_LIST` (forme `ref:liste` de 1b.5), message `"jo.oe/jy: … (ref:liste) …"`. Appels : branches `oe`, `jy`.
  - `static void jo_take_bool(pico8_value_t value, int *out, int *has)` : forme `bg_take_bool` de 1b.15 (pose `*out = 0`, `*has = 0`, sort sur `REF NULL`, `assert` `BOOL`, recopie `as.boolean != 0`, `*has = 1` en dernier), message `"jo.fx/p_: … (bool) …"`. Appels : branches `fx`, `p_`.
  - `static void jo_take_fixed(pico8_value_t value, pico8_fixed_t *out, int *has)` : même forme, `assert` `FIXED`, message `"jo.jh: … (fixe) …"`. Appel : branche `jh`. **Premier écrivain par nom d'un fixe à indicateur de l'arbre `jo`** : nil éteint `has_jh` (l'ancien code l'acceptait, 1b.19 avortait parce que `qb.ma` n'avait pas d'indicateur).
  - `static void jo_take_lq(pico8_value_t value, pico8_lq_t **out)` : `assert` REF non NULL, tag `STRUCT`, `->cls == pico8_lq_struct_class()`, message `"jo.lq: a write neither nil nor a layout row lq is off its census row (ref:pico8_lq_t) -- …"`. Appel : branche `lq`.
- **`planet_family.c`** : `static void family_take_string(…)`, même forme et même message que `jo_take_string` au nom `"jo.id"`, pour les clés `id` de `qh`, `nl`, `os`, `hj`, `pm`, `ga`.
- **`planet_family_trade.c`** : `static void family_trade_take_string(…)`, pour `nr`, `fr`, `mx`.
- Écarté (G) : une aide publique unique dans `planet.c` (1b.13 point 2) ; une aide par membre (les messages nomment déjà les deux membres d'un même genre, forme de 1b.9).

### R5 : écrivain de `jo` (G)

- 7 branches `if (strcmp(name, "<m>") == 0) { jo_take_<genre>(value, &jo-><m>[, &jo->has_<m>]); return 1; }`, **dans l'ordre du typedef** et intercalées avec `kw`, `hn`, `nk` : `kw`, `oe`, `hn`, `ep`, `jy`, `nk`, `fx`, `lq`, `jh`, `p_`. Puis la boucle `written[]` réduite à `by cc fo kr m_ ma mi`.
- **Lecture de 1b.23a2 appliquée** : `ep`, `jy`, `fx`, `lq`, `jh` n'ont que `l` au recensement, mais l'écrivain de 1a les accepte : leur branche est gardée. `id` n'a pas de branche (l'écrivain le refuse déjà, `write_id` des TU lents).
- Commentaire de tête « The 17 names … » inchangé (toujours vrai).

### R5 : boucles de clés `id` (G)

- **`jo`** : le `else *pico8_jo_member(self, key.name) = …` de 1b.23b devient `else jo_take_string(value, &self->id);` (l'`assert(measured …)` n'admet que `id`, `p`, `ra`). `pico8_jo_member` n'a plus d'appelant dans la boucle.
- **`qh`, `nl`, `os`, `hj`, `fr`, `mx`** : `self->…jo.id = value` → `family(_trade)_take_string(value, &self->…jo.id)` après l'`assert(measured …)` inchangé.
- **`pm`** : la branche `else` (`is_id`) appelle `family_take_string`.
- **`ga`** : une branche `id` **avant** la recherche dans `keys[]`, qui `continue`, à côté des branches `p`/`ra` de 1b.23b ; `keys[]` perd `{"id", offsetof(pico8_ga_t, jo.id)}` et garde les 8 membres propres de `ga`. L'`assert(found < …)` et son message (11 clés mesurées) inchangés. Forme de 1b.23b (R7 sur une table de clés de fille), recopiée.
- **`nr`** : branche `id` → `family_trade_take_string` ; `p`, `ra` (1b.23b) et l'`assert(0 …)` final inchangés.
- Nil accepté (pointeur). Un non-chaîne, une chaîne NULL, un non-REF à union nulle avortent.

### Lectures et écritures directes (R6)

- `planet.c` : `jo:pq` → `self->oe = oe;` ; `jo:lz` → `self->ep = NULL;`, `self->jy = jy_heap;` ; `jo:ll` → `self->p_ = t > …; self->has_p_ = 1;` ; `jo:hu` → `self->p_ = 1; self->has_p_ = 1;`, `if (self->id != NULL) set_name(overrides, "id", pico8_value_string(self->id));`, `self->fx = 1; self->has_fx = 1;` ; `jo:hh` → `jo_take_string(d_val, &self->ep);` (1b.18). `pico8_jo_present` garde son appelant `kr` (1b.23d).
- `planet_display.c` : `jo:hb` → `self->lq = lq_result;`, `list_of(self->jy)` → `self->jy` (idem `jo:gh`) ; `list_of` garde ses appelants `by` (1b.23d). `jo:kj` : `if (self->lq != NULL)` puis `pico8_mv((pico8_struct_header_t *)self->lq, "ol", …)` ; **l'`assert` de lecture « jo.lq must be the row jo.ex built » est retiré** : le type le porte, et `jo_take_lq` le vérifie à l'écriture par nom (lecture de 1b.20, `ie_take_jb`) (G).
- `planet_trade.c` : `jo:kf` → `oe = self->oe;`, `self->fx = 1; self->has_fx = 1;` ; `jo:cn` → `jy = self->jy;`, `!pico8_jo_truthy(self->jh)` → `!self->has_jh` (un fixe présent est toujours vrai en Lua), `self->jy = merged_heap;`, `self->jh = 1; self->has_jh = 1;` ; boucle de `jo:ib` → `oe = self->oe;`. `pico8_jo_truthy` de ce fichier garde son appelant `fx_val`.
- `planet_family.c` : `ga:kj` → `self->jo.p_ = last_pt == NULL; self->jo.has_p_ = 1;`.
- `planet_family_trade.c` : `nr:gv`, `nr:hb` → `oe_count = self->qh.jo.oe != NULL ? self->qh.jo.oe->count : 0;` ; `nr:cn` → `oe = self->qh.jo.oe;`.
- Écarté (G) : faire passer les écritures directes de `p_`/`fx`/`jh` par les aides (les valeurs sont des littéraux C déjà dans le genre, R6 les écrit directement ; l'aide ne sert qu'aux `pico8_value_t` venus d'ailleurs, 1b.18).

### Boucles sentinelles (forme de 1b.23a2, recopiée)

`"ep"`, `"fx"`, `"jh"`, `"jy"`, `"lq"`, `"oe"`, `"p_"` sortent des 10 tableaux `written[]` (`jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`, `nl`, `fr`, `ga`) ; **rien d'autre ne change** dans ces fichiers (commentaires « takes exactly N names » compris : les écrivains prennent toujours ces noms). L'intention est reportée dans la caractérisation (aller-retour dans le genre, nil) et dans le test d'arrêt (hors genre). Les boucles de `g_` et `fv` (`p_` de `g_`, homonyme) ne sont pas touchées.

### Preuves (sortie attendue sur l'arbre final)

- `grep -rnE 'offsetof\(pico8_[a-z0-9_]+_t, ([a-z0-9_]+\.)*(id|oe|ep|jy|fx|lq|jh|p_)\)' src/ | grep -E 'pico8_(jo|qh|nl|os|hj|pm|ga|nr|fr|mx)_t'` : vide (9 lignes avant) ;
- `grep -rn 'offsetof(pico8_jo_t' src/` relu : 7 entrées, `ma fo kr cc by mi m_` (R7 de 1b.1, le typedef garde des `pico8_value_t`) ;
- `grep -nE '(\.|->)(id|oe|ep|jy|fx|lq|jh|p_)\.(kind|as)' src/pico8_planet*.c` : vide ;
- `grep -rnE 'jo\.id = |->id = ' src/pico8_planet*.c` : seulement l'affectation R3 `self->id = NULL` ;
- `grep -n 'list_of(self->jy)\|jo.lq must be the row' src/pico8_planet_display.c` : vide ;
- `grep -c 'off its census row' src/pico8_planet.c src/pico8_planet_family.c src/pico8_planet_family_trade.c` : 8, 2, 2 (1b.23b : 3, 1, 1) ;
- `grep -n 'has_fx\|has_jh\|has_p_' src/pico8_planet.h` : 3 lignes, chacune juste après son membre ;
- `make value-members | grep -c 'pico8_jo_t'` : 7 ;
- `git diff --stat HEAD -- src/ ':!src/pico8_planet*'` : vide (sites par nom intacts) ;
- `git diff` des 10 TU à boucle sentinelle : dans `written[]` seulement, les 7 noms retirés.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 153 sur l'arbre de départ (1b.23b commitée), 145 après. Seules 8 lignes `pico8_jo_t` disparaissent.
- **H2** : dans les 15 rejeux et dans tous les TU hors fils de `must_abort`, les aides ne voient que leur genre ou nil : `id` des chaînes (`bw`, `ie:ldown`, `hu`, TU) ; `ep` une chaîne `d` de `df` ou nil (`ll` n'a pas de `d`) ; `oe`/`jy` des listes (`pq`, `lz`, `cn`, TU `nr`/`fr`/`mx`) ; `fx`/`p_` des booléens (`d_`, TU) ; `lq` jamais écrit par nom hors TU ; `jh` jamais écrit par nom hors TU.
- **H3** : `pico8_jo_ex` rend un `pico8_lq_t *` de descripteur `pico8_lq_struct_class()` (l'`assert` de `jo:kj` le vérifie aujourd'hui sans jamais avorter).
- **H4** : les listes (`pico8_list_t`) portent le tag `PICO8_TYPE_TAG_LIST` ; aucune liste de planète n'est un `ob_table`.
- **H5** : aucune valeur figée ne bouge : `STRUCT_HASH`, `SPEC_HASH`, `SHARE_HASH`, `READY_HASH` de `test_pico8_jo_spec_defaults` ; `STRUCT_HASH` de `test_pico8_spec_defaults` ; les hash des TU de l'arbre `jo`, de `test_pico8_hp`, `test_pico8_jc`, `test_pico8_ie` ; `JO_MEMBERS_HASH`, `JO_WRITTEN_HASH`, le hash de 1b.23b, `KE_ME_HASH`, `KE_T_HASH`, `DRAG_REPLAY_HASH`, `TECH_REPLAY_HASH` ; les 15 empreintes du filet.
- **H6** : le build `-DPICO8_MEMBER_CENSUS` compile. Il n'est pas lancé.
- **H7** : `test_pico8_ui` reste sous 30 s avec deux fonctions de plus, dont un rejeu `DRAG` complet (hors `UNIT_TESTS_SLOW`).
- **H8** : 1b.23b est commitée et ne touche aucune ligne de ces 8 membres, sauf le `else` générique de la boucle de clés de `jo` (qui n'y envoie plus que `id`), la forme de `g_jo_members` et de `keys[]` de `ga` (`id` en tête), et les `written[]` (sans `ra`, `p`) ; les lignes glissent seulement.
- **H9** : rien ne dépend de `sizeof(pico8_jo_t)` ni de la disposition des filles.
- **H10** : sur l'ancien code, chaque cas du test d'arrêt se termine sans `SIGABRT` (`_exit(0)`) : le `must_abort` est rouge. Un cas qui avorterait déjà (par exemple une clé `id` non-chaîne lue plus loin dans `pq`/`lt`) est retiré du « rouge d'abord » et noté.
- **H11** : sur un `pico8_<X>_new(NULL)` neuf : `id`, `ep`, `fx`, `lq`, `jh`, `p_` absents ; `oe` présent, liste vide, distincte d'une planète à l'autre ; `jy` présent exactement pour `lz_at_pq[i]`. Les valeurs figées sont celles **observées**.
- **H12** : `pico8_jo_hb` est appelable sur une planète neuve après `start_game(DRAG)` et y pose `lq`.
- **H13** : `pico8_ga_kj` est appelable sur un `ga` neuf avec le `gfx`/`mem` de `test_pico8_ui` et y pose `p_` présent.
- **H14** : aucun site C1 de `tools/float_audit_c1.txt` ne cite ces membres (grep vide). Si un extrait change quand même, seule sa ligne est réécrite (1b.21).
- **H15** : aucun test de régression (§6) n'accède directement à ces 8 membres sur l'arbre 1b.23b (grep vide hors `iv`) ; `nil_row`, `rich_planet_exports`, `slipway_drag` les lisent par nom et restent verts sans modification.
- **H16** : les lignes modifiées de `jo:cn` (`planet_trade.c`), de `nr:cn` (`planet_family_trade.c`) et de la boucle de `jo:ib` ne sont peut-être pas atteintes par `test_pico8_ui` ; elles le sont par **une** passe `gcov` de `test_pico8_fr` ou de `test_pico8_ga` (TU rapides qui appellent `cn`, `kf`, `hb`, `gv`, `kj`).
- **H17** : retirer 7 noms des 10 boucles sentinelles ne change aucune autre assertion de ces TU (ces boucles n'ont que des `assert`, pas de hash).
- **H18** : `pico8_layout.h` s'inclut dans `planet.h` sans cycle ; tout binaire qui lie `planet.c` lie `layout.c` (`ENGINE_SRCS`, `DATA_ENGINE_SRCS` et `main_raylib` listent les deux), donc `pico8_lq_struct_class()` se lie partout où `jo_take_lq` est compilé (piège de 1b.22 écarté).
- **H19** : à la fin du rejeu `DRAG` joué dans la caractérisation, au moins une planète a `oe` à 1 entrée (le slipway construit), et `jo:kf` a été appelé ; `lq` est présent sur au moins une planète survolée. Valeurs observées figées, sans `assert` sur leur existence.
- **H20** : `jo:hh` sur une planète neuve après `start_game(DRAG)` pose `ep` à une chaîne, ou le laisse absent quand le tirage donne `ll` ; la valeur observée est figée.
- **H21** : `test_pico8_spec_defaults` plie `id`, `fx`, `lq` sur des structs de l'arbre `jo` (compte de l'empreinte seulement, sans effet sur le diff).

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom, ou l'écran.** Les nouveaux tests n'accèdent à aucun des 8 membres directement : ils compilent sur les deux formes. Aucune ligne directe de test n'est touchée (revue §6).
- **Caractérisation dirigée, figée sur l'ancien code** : `test_jo_state_by_name`, dans `test_pico8_ui` (rapide, dans `CENSUS_UNIT_TESTS`, lie `ENGINE_SRCS`, a `start_game`, `play_frame`, `must_abort`, `in_fresh_child`, précédents 1b.14a et 1b.23a/b). Lancée par `in_fresh_child` après les tests de `jo` de 1b.23a/b. Elle lit par le **lecteur brut du descripteur** (`cls->field`, témoin `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder (retour, genre, valeur, pointeur). Fonction distincte de celles de 1b.23a/b, hash propre.
  1. **Planètes vivantes au départ** : `start_game(DRAG)`, puis on parcourt `pico8_hv()`. Pour chaque entité de l'arbre `jo` (`jo_tree`), on plie : présence et octets de `id` et `ep` (FNV sur la chaîne) ; présence et compte de `oe` et `jy` ; présence et valeur de `fx`, `p_` (booléen), `jh` (fixe) ; présence de `lq` et son descripteur (`assert` = `pico8_lq_struct_class()`). Plus le nombre d'entrées.
  2. **Les 10 genres, neufs.** `pico8_<X>_new(NULL)` pour `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx` : `id`, `ep`, `fx`, `lq`, `jh`, `p_` absents (témoin intact) ; `oe` présent, liste vide, distincte entre les 10 (`assert`) ; `jy` présent ssi `lz_at_pq[i]` (tableau de 1b.23a1, repris). Valeurs **observées** figées (H11).
  3. **Boucles de clés `id`**, pour les 10 constructeurs : `overrides` = {`id` = S} (chaîne locale) → `id` rend le **même pointeur** S (`assert`) ; pour `jo`, `pm`, `ga`, `nr` aussi avec `p`/`ra` (1b.23b) pour garder les branches voisines.
  4. **Écrivain par nom dans le genre**, sur `jo`, `qh`, `pm`, `ga`, `nr`, `fr` (chaque chaîne de délégation) : `ep` ← S → même pointeur, ← nil → absent ; `oe`, `jy` ← liste L → même L, ← nil → absent ; `fx`, `p_` ← `true`, ← `false` (présent, `BOOL` 0), ← **faux à union nulle** (présent, `BOOL` 0), ← nil (absent) ; `jh` ← 3, ← **fixe 0 à union nulle** (présent, 0), ← nil (absent) ; `lq` ← la rangée R d'un `jo:hb` → même R, ← nil → absent. `cls->set_field` rend 1 à chaque écriture ; `id` rend 0 (ensemble des noms inchangé).
  5. **Transitions** : `jo:ll(e, 61)` → `p_` vrai ; `jo:ll(e, 60)` → `p_` **faux présent** ; `jo:lz(e, plain)` → `ep` absent, `jy` présent, liste neuve ; `jo:hh(e)` → `ep` tel qu'observé (H20) ; `jo:hu(e, plain)` (branche `lz`) → `fx` vrai ; `jo:hu(e, with_cw)` sur un `jo` construit avec `id` = S → `e` a `p_` vrai, et la nouvelle planète (dernière de `hv`) a `id` = S, même pointeur ; `jo:hb(e)` → `lq` présent, descripteur `lq` (H12) ; `pico8_ga_kj` sur un `ga` neuf → `p_` tel qu'observé (H13) ; `nr` neuf, `oe` ← liste de 2 par nom, `mv(nr, "hb")` → `s` observé, puis `mv(nr, "gv", nil)` → valeur observée ; `d_` par `mv(qh, "ey")` sur un `qh` que `lu` refuse → `p_` vrai si observé.
  6. **Planètes vivantes après le rejeu** : on joue toutes les frames du rejeu `DRAG` (`play_frame`, comme `replay()`), puis on replie le parcours de 1 (H19 : `oe`, `fx`, `lq` après `kf`, `hb`).
  7. **Intrus** sur le lecteur de `jo` : `""`, `"i"`, `"idd"`, `"p_ "`, `"has_p_"`, `"has_fx"`, `"has_jh"`, `"fx_"`, `"oe "` → 0, témoin intact.
  8. Tout est plié dans **`JO_STATE_HASH`**, relevé au premier passage vert sur l'arbre de départ, puis inchangé. Les adresses sont jugées par `assert`, jamais pliées.
- **Test d'arrêt, rouge d'abord** (1b.13/1b.15) : `test_jo_state_stops_off_row`, par `in_fresh_child` après la caractérisation.
  - **Écrivain** : `must_abort` sur `pico8_entity_set_field(e, n, v)`, `e` un `jo` puis un `pm` neufs (délégation `pm` → `hj` → `qh` → `jo`), pour chacun des 7 noms, `v` pris dans l'ensemble hors genre du nom parmi {fixe 1, `pico8_value_bool(1)`, chaîne `"x"`, **fixe 0 à union nulle**, **faux à union nulle**, **chaîne NULL**, **fonction NULL** (tous par `memset`, `kind`), REF `ob_table`, REF `pico8_hv()` (liste), REF `e` (struct), REF d'un `hp` de `gb` (struct d'un autre descripteur, pour `lq`)}. Sont **exclus** les cas dans le genre : `false` et faux à union nulle pour `fx`/`p_`, fixe 1 et fixe 0 à union nulle pour `jh`, liste pour `oe`/`jy`, `"x"` pour `ep`.
  - **Boucles de clés** : `must_abort` sur `pico8_<X>_new(b)`, `b` = {`id` = v}, pour les 10 constructeurs, v ∈ {fixe 1, chaîne NULL à union nulle, faux à union nulle, REF `ob_table`}.
  - **Rouge sur l'ancien code** (écrivain et boucles recopient tout, le fils sort par `_exit(0)`, H10), vert après : une ligne « Runs » pour chacun. Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Tests existants qui figent déjà beaucoup** : les TU lents (`fold_field` des 31 noms, `bool_state(e, "p_")`, `list_size(e, "oe"/"jy")`, `!has(bare, "id")`), `test_pico8_hp` (`fold_planet` : `ep`, `jy`, `fx`, `p_`), `test_pico8_ga` (`lq`, `p_` de `ga:kj`), `test_pico8_jc` (`oe` des extrémités), `test_pico8_ie` (`id` des planètes de `ie:ldown`), les trois tests de régression lus par nom, `DRAG_REPLAY_HASH`/`TECH_REPLAY_HASH` (écran).
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_ui && ./tmp/test_pico8_ui` (ou le TU cité). Chacune est notée dans le commit avec le test qui la rougit ou la justification de son trou. Un appel unique d'aide `static` se retire sous `if (0)` (1b.14b).
  1. (§7-1) *indicateur forcé vrai* — **s'applique dans sa lettre** (premiers indicateurs de `jo`) : la branche R4 de `p_` ignore `has_p_`. Attendu rouge : caractérisation 2 (`p_` présent `false` au lieu d'absent) ; `bool_state` des TU lents noté sans relance.
  2. (§7-2) *« présent 0 » là où c'était absent* — **s'applique dans sa lettre** : la branche de `jh` rend `pico8_value_fixed(0)` quand `has_jh` = 0. Attendu rouge : caractérisation 2 et 4. Note : `bb` lit `pt.jh or 0`, même valeur ; seul le lecteur par nom le voit.
  3. (§7-3) *init ou boucle qui saute un membre typé*. Forme « pose une autre valeur », parce que l'omission pure laisse de la mémoire de `malloc` non déterministe (**trou justifié** de la forme littérale, précédent 1b.14a) :
     - (a) `pico8_jo_struct_init` pose `has_fx = 1` → rouge : caractérisation 2 ;
     - (b) `pico8_jo_struct_init` pose `has_jh = 1` → rouge : caractérisation 2 ;
     - (c) `pico8_jo_struct_init` pose `ep = "x"` → rouge : caractérisation 2 (`jo`, `ga`, que `lz` ne remet pas à nil) ;
     - (d) la branche `id` de la boucle de `nr` sous `if (0)` → rouge : caractérisation 3 ;
     - (e) la branche `id` de `ga` saute l'écriture (`continue` seul) → rouge : caractérisation 3 et 5 (`hu` vers `ga`) ;
     - (f) `oe` : **trou justifié**, `jo:pq` repose `oe` dans tous les constructeurs avant toute lecture ; une init qui le saute ou le fausse est invisible par construction.
  4. Propre à l'étape, *identité perdue* : `jo_take_string` recopie la chaîne (`strdup`). Attendu rouge : caractérisation 3 et 4 (`assert` de pointeur).
  5. Propre à l'étape, *aide sans `assert`* : `jo_take_list` recopie tout REF. Attendu rouge : test d'arrêt.
  6. Propre à l'étape, *écrivain muet* : la branche `p_` rend 1 sans écrire. Attendu rouge : caractérisation 4.
  7. Propre à l'étape, *`ll` sans indicateur* : `jo:ll` omet `has_p_ = 1`. Attendu rouge : caractérisation 5 ; filet noté (`dg` ne retire plus rien).
  8. Propre à l'étape, *`hh` sans `ep`* : l'appel `jo_take_string(d_val, &self->ep)` de `jo:hh` sous `if (0)`. Attendu rouge : caractérisation 5 si `hh` y pose une chaîne (H20) ; sinon le trou est noté.
  9. Propre à l'étape, *`cn` à l'envers* : `!self->has_jh` → `self->has_jh`. Attendu rouge : la passe du TU rapide de H16 (`test_pico8_fr` ou `test_pico8_ga`) ; sinon le trou est justifié.
  10. Propre à l'étape, *`hb` muet* : `jo:hb` pose `lq = NULL`. Attendu rouge : caractérisation 5 et 6 ; `DRAG_REPLAY_HASH` si la rangée est dessinée.
- **`gcov`** : `test_pico8_ui` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23, sur les 5 fichiers `.c` modifiés. Chaque ligne modifiée doit être exécutée ; une ligne manquée (probablement `jo:cn`, `nr:cn`, la boucle de `jo:ib`, H16) est cherchée dans **une** passe `gcov` de `test_pico8_fr` ou de `test_pico8_ga`, sinon justifiée dans le commit. Trous attendus : l'échec des `assert` des aides d'écriture (fils de `must_abort` qui avortent sans vider leur compteur, prouvés par le test d'arrêt et la mutation 5).
- **Prior art** : `test_jo_members_by_name`, `test_jo_written_by_name`, `test_jo_writer_stops_off_row` (1b.23a, même fichier) et la caractérisation de 1b.23b ; `test_ke_me_by_name`, `test_ke_ks_stops_off_row` (1b.14a) ; `bg_take_bool` et son test d'arrêt (1b.15, indicateur) ; `fq_take_bool` (1b.18, aide partagée) ; cas `memset` de 1b.14b/1b.15/1b.18.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_ui` : 10 à 30 s. Une quinzaine de lancements (départ, arrêt rouge, final, 15 mutations, `gcov`) : ~7 min ;
- une passe `gcov` de `test_pico8_fr` ou `test_pico8_ga` : moins d'une minute ;
- `make test` : 20 s à 1 min, une fois ;
- `make e2e` : 4 à 6 min, une fois ;
- `make class-coverage-net` : 3 à 4 min, **dans la même invocation** que le `make e2e` (arbres distincts, 1b.0) ;
- `make sanitize-net` : ~4 min seul, **jamais** en même temps que `e2e`, une fois sur l'arbre final ;
- `make value-members` : quelques secondes.

**TU lents touchés** : `test_pico8_jo`, `test_pico8_qh`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`, `test_pico8_nr` (tableaux `written[]`). `test_pico8_ie` n'est pas modifié, mais il exerce le code changé (`id` des planètes de `ie:ldown`). **Construire les sept tôt**, juste après le typage et avant toute mutation (`make -j tmp/test_pico8_jo tmp/test_pico8_qh tmp/test_pico8_os tmp/test_pico8_hj tmp/test_pico8_pm tmp/test_pico8_mx tmp/test_pico8_nr tmp/test_pico8_ie`, compilation seule) ; ils ne sont lancés que par `make e2e`.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre (départ / final / mutation n) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les **8** TU de `UNIT_TESTS_SLOW` passent (`test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 145`**, 7 lignes `pico8_jo_t` ;
5. `timeout 15m make class-coverage-net` : **sortie identique à `tools/class_coverage_net_expected.txt`**. Elle s'applique parce que le diff touche des constructeurs (`pico8_jo_struct_init` et les dix `pico8_<X>_struct_new` de l'arbre), sans toucher `pico8_class_coverage_record` ni `KNOWN[]`. Aucun compte ne change.

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS` ni un fichier d'événements.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si 1b.23b n'est pas commitée au départ, ou si `make value-members` ne rend pas 153 sur l'arbre de départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si une aide d'écriture avorte dans un rejeu ou dans un TU hors des fils de `must_abort` (H2, H3, H4), ou si un type est élargi (union, `void *`, `pico8_struct_header_t *`, `pico8_value_t` gardé à côté) au lieu d'avorter.
- `FAILED:` si l'un des 8 membres n'a pas le type du tableau de recensement à sa place dans `pico8_jo_t`, si `has_fx`, `has_jh` ou `has_p_` manque ou n'est pas juste après son membre, si un indicateur est ajouté à un pointeur, ou si l'un des 7 autres membres est typé ou touché.
- `FAILED:` si `g_jo_members` ou `keys[]` de `ga` liste encore l'un des 8 noms, ou si une autre de leurs entrées est retirée ou modifiée ; si une table de défauts gagne ou perd une entrée.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact pour un pointeur NULL ou un `has_` à 0, ne rend pas le même pointeur, ou rend un autre genre que 1a (`p_` faux doit rester `BOOL` 0 présent).
- `FAILED:` si l'écrivain n'accepte plus exactement les 17 mêmes noms (dont `id` refusé), ou si une boucle de clés n'accepte plus exactement les mêmes clés ; si une aide ne pose pas l'état nil d'abord, n'avorte pas sur un hors-genre à union nulle (fixe 0 pour une chaîne, chaîne NULL, fonction NULL), accepte une chaîne NULL, ou si son message ne nomme pas `jo.<m>` et le genre de la ligne ; si une aide est partagée entre fichiers ou rendue publique.
- `FAILED:` si un site par nom de `src/` hors `pico8_planet*` change, ou si un site par nom est remplacé par un accès direct.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si le diff d'un test fait autre chose que : ajouter la caractérisation, le test d'arrêt, `JO_STATE_HASH` et leurs appels à `test_pico8_ui` ; retirer les 7 noms des 10 tableaux `written[]`. Idem si une valeur figée existante change (H5), ou si un test de régression est modifié.
- `FAILED:` si un test de régression (§6) se révèle accéder directement à l'un des 8 membres (le compilateur le désigne) et que son changement n'a pas ses deux lignes « Runs » (rouge avec le bug remis, vert restauré).
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 145, ou si un autre typedef que `pico8_jo_t` gagne ou perd une ligne.
- `FAILED:` s'il manque une ligne « Runs » de `make class-coverage-net` identique à la référence, ou si le diff touche `pico8_class_coverage_record` ou `KNOWN[]`.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, si elle lit ou écrit l'un des 8 membres directement, si elle ne couvre pas les 10 genres, les 8 noms, les 10 boucles de clés et les 7 branches d'écrivain, ou si `JO_STATE_HASH` change après ; si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'arbre de départ et une verte sur l'arbre final, ou s'il n'a pas les cas à union nulle.
- `FAILED:` si le commit ne contient pas son rapport `gcov` et ses mutations (1, 2, 3a–3f, 4 à 10), chacune avec son test ou la justification de son trou.
- `FAILED:` si le travail n'est pas en un commit vert, ou si un membre de 1b.23d y est mêlé.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.23c », ou si elle ne porte pas les lectures nouvelles (premiers indicateurs de `jo` et aides à indicateur partagées ; clé `id` dans les dix constructeurs par une aide de chaîne par fichier ; `assert` de lecture de `lq` passé à l'écriture) ; s'il manque le tableau « Runs ».

## Out of Scope

- Les 7 autres membres de `jo` (1b.23d) : `ma` (dernier de `g_jo_defaults`), `fo`, `kr`, `cc`, `by`, `mi`, `m_`, et `list_of`/`pico8_jo_present` qui les servent encore.
- Les membres propres des filles (`qh.et`, `ga.qy`, `nr.hz`…) : 1b.24 et suivantes.
- Les homonymes `id`/`p_`/`lq`/`ep`/`fx` d'autres structs (`ie`, `mz`, `mh`, `iv`, `js`, `jc`, `g_`, `fv`, `pj_cr`, `hp`) : leurs étapes.
- Remplacer un site par nom par un accès direct ; ajouter ou retirer un nom d'un écrivain ou d'une boucle de clés.
- Régénérer `tools/member_census.txt`.
- Tout test de régression (aucun n'est touché).
- Les fichiers d'événements et `FRAMES_EVENTS`, tout changement de rendu, l'audio, lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `planet.[ch]` (types, `has_`, lecteur, écrivain, init, boucle de clés de `jo`, `pq`, `lz`, `ll`, `hu`, `hh`, 5 aides d'écriture, 2 aides de lecture) ; `planet_display.c` (`hb`, `gh`, `kj`) ; `planet_trade.c` (`kf`, `cn`, `ib`) ; `planet_family.c` (6 boucles de clés, `keys[]` de `ga`, `ga:kj`, `family_take_string`) ; `planet_family_trade.c` (3 boucles de clés, `nr:gv/hb/cn`, `family_trade_take_string`) ;
  - types : `id`, `ep` → `const char *` ; `oe`, `jy` → `pico8_list_t *` ; `lq` → `pico8_lq_t *` ; `fx`, `p_` → `int` + `has_` ; `jh` → `pico8_fixed_t` + `has_` ;
  - tests : `test_pico8_ui` étendu (caractérisation des 8 membres sur les 10 genres, les 10 boucles de clés, les 7 branches et le rejeu `DRAG` joué ; test d'arrêt rouge d'abord) ; 10 tableaux `written[]` réduits ; aucun test de régression touché ; pas de nouveau TU ;
  - pas de découpage (17 fichiers, sous 1b.23a2 ; les deux moitiés naturelles partagent tous leurs fichiers) ;
  - une aide d'écriture par genre et par fichier, partagée entre l'écrivain, les boucles de clés et les écritures directes de valeurs venues d'ailleurs (`hh`) ; les littéraux C s'écrivent directement (R6) ;
  - l'`assert` de lecture de `jo:kj` sur `lq` passe à l'écriture (1b.20) ;
  - `class-coverage-net` lancé une fois, sur l'arbre final.
- **Note « Découvert à 1b.23c »**, à écrire sur la ligne de l'étape : « patron 1b.1 appliqué ; `id`, `ep` → `const char *`, `oe`, `jy` → `pico8_list_t *`, `lq` → `pico8_lq_t *` (1b.10), sans indicateur ; **premiers indicateurs de `jo`** : `fx`, `p_` → `int` + `has_`, `jh` → `pico8_fixed_t` + `has_` (R2, nil observé `lu`) ; `jo:ll` écrit `p_` faux **présent** ; écrivain : 7 branches (lecture de 1b.23a2 : `ep`, `jy`, `fx`, `lq`, `jh` n'ont que `l` et gardent leur branche), aides `jo_take_string/list/bool/fixed/lq`, la chaîne partagée avec `hh` (1b.18) ; **`jo_take_fixed`, premier écrivain par nom d'un fixe à indicateur : nil éteint `has_`** ; **clé `id` dans les dix constructeurs de l'arbre** par une aide de chaîne par fichier (`jo_`, `family_`, `family_trade_take_string`), `keys[]` de `ga` ne garde que les membres de `ga` ; `assert` de lecture de `jo:kj` sur `lq` passé à l'écriture (1b.20) ; boucles sentinelles de 1b.23a2 recopiées (10 TU, dont 7 lents) ; aucun test de régression n'accède directement à ces membres (grep sur `->m`/`.m` de tous les TU) ; `g_jo_members` garde 7 entrées ; empreinte réelle ≈ 37 (16, 21), 17 fichiers modifiés. » Le reste va dans le commit.
- Le message de commit porte le périmètre, le compte, le tableau des poses (R3), des branches d'écrivain et des boucles de clés, le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous, `JO_STATE_HASH` et les réponses à H1–H21.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
