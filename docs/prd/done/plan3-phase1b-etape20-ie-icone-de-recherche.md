---
title: "Plan 3, étape 1b.20 : `ie` (l'icône de recherche) — huit membres typés, `ke` en `const pico8_entity_kind_t *`, `jb` en `pico8_jb_t *` (→ 1b.2), `kh`/`m_` avec `has_` sur preuve d'un TU lent, `m_` écrit par nom (forme 1b.15), table `offsetof` supprimée (1b.9)"
description: "`pico8_ie_t` a huit membres `pico8_value_t` (`kq` : `ref:autre`, une boîte `pico8_qz_box_t` ; `fa` : `fixe`, jamais nil ; `ra`, `eh` : `ref:autre`, des vecteurs ; `ke` : `ref:autre`, un descripteur de genre, pas une entité ; `jb` : `ref:pico8_jb_t` ; `kh` : `bool`, nil `stat` ; `m_` : `bool`, écrit par nom). Les typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 185 à 177. `kh` et `m_` reçoivent un `has_` parce que `test_pico8_ie` (lent) les lit nil par nom (règle R2 de 1b.11). L'écrivain de `m_` recopie la forme 1b.15 ; les clés `ke`/`jb` et la réaffectation de `ra` dans `hb` passent par des aides R5 `static` ; la table `members[]` du lecteur disparaît (R7 de 1b.9)."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.20 — `ie` (`research_icon.h:26`), `jb` → 1b.2, `m_` écrit par nom. Empreinte 26 (10, 16). Compte : 177.

Cadre : les puces **Nil**, **REF**, **Coexistence** (« Écriture : l'écrivain déballe ; nil éteint la présence ; un genre hors de la ligne de recensement du membre est un `assert` ») et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité, pas redécidé. S'appliquent aussi, sans être redérivées, les notes « Découvert à » de `docs/plan-3.md` :

- **1b.1**, puis **1b.3** à **1b.18** : patron appliqué ; `ref:autre` vers un vecteur → `pico8_vec_t *` emprunté, même pointeur (1b.4 ; `bg.mq` en 1b.15, `nf.ra` en 1b.17) ; `fixe` à nil `jamais` sans indicateur (1b.6) ; nil `stat?` d'homonyme ne décide rien (1b.7) ; R3 garde un **défaut module** partagé par toutes les instances, même REF (1b.17) ; **lecture de R5 pour `ref:autre`** : REF non NULL de tag `UNKNOWN`, `assert` sinon (1b.10, `bg_take_vec` en 1b.15) ; **R5 boucle de clés directe** (1b.10) : une branche par clé qui appelle une aide `static` par genre, la clé inconnue garde son `assert` et son message ;
- **1b.2** : `pico8_jb_t` est typée (un seul membre, `ls`) ; `ie.jb` en est la cible (`ref:pico8_jb_t`) ;
- **1b.9, R7 forme `offsetof`** : chaque entrée d'un membre typé sort de `members[]` ; la table vide est supprimée avec sa boucle ; ses appelants deviennent des branches R4 `strcmp` (ordre du typedef), les affectations R3 et les aides R5 ; preuves `grep -rn 'offsetof(pico8_X_t' src/` vide et la forme mécanique `… | grep -v 'pico8_value_t \*out_value)' | grep -v 'pico8_value_t \*out)'`. Le PRD de 1b.11 l'avait noté : la table `members[]` de `pico8_ie_field` et sa ligne de recopie `*(const pico8_value_t *)((const char *)ie + members[i].offset)` relèvent de 1b.20 ;
- **1b.13/1b.15, forme écrivain de R5**, recopiée point par point (1b.18 sans écart) : une branche `strcmp` par membre `e`, dans l'ordre du typedef ; aide `static` par genre au nom de la struct ; **nil d'abord** (pose sans condition `<m> = 0` puis `has_<m> = 0`, sort sur `REF NULL` — genre **et** pointeur testés) ; sinon `assert` « champ sans règle » qui nomme `struct.membre` et le genre, recopie, `has_<m> = 1` en dernière instruction ; pour `ref:pico8_X_t`, l'`assert` vérifie le genre REF, le tag `STRUCT` **et** `cls == pico8_X_struct_class()` (accesseur public) ; nom sans branche → `return 0` sans rien toucher ; **chaque test d'arrêt porte les cas non-REF à union entièrement nulle, construits par `memset` puis `kind`** (1b.13 `write_fo_zero_false`, 1b.14b, 1b.15 `zero_fixed`/`zero_string`/`zero_false`, 1b.18 `zero_function`) ; un `false` à union nulle est un `false` présent, pas nil ;
- **1b.14b** : une mutation qui retire l'unique appel d'une aide `static` est arrêtée par `-Werror` ; on la plante sous une forme qui garde l'appel (`if (0)`, clé renommée) ;
- **1b.18** : une écriture directe (R6) dont la valeur arrive en `pico8_value_t` passe par l'aide R5 du genre (`fq_take_bool` pour `p_` de `ldown`) ;
- **1b.11, règle R2 des TU lents** : une lecture par nom faite par un TU de `UNIT_TESTS_SLOW` compte comme `l`, et le nil qu'elle lit comme `lu`. **`test_pico8_ie` est lent : relu plus bas, il décide `has_kh` et `has_m_`.**
- **1b.5, 1b.7, 1b.16, lectures de R1 pour genres mêlés** : **sans objet** — chaque ligne d'`ie` n'a qu'un genre.

### État de départ

HEAD `5674692` (PRD de 1b.19). **1b.19 (`qb`) est en cours dans l'arbre de travail** (`src/pico8_scroll.[ch]`, `tests/test_pico8_qb.c`, `docs/plan-3.md`). **Précondition : elle est commitée avant que 1b.20 commence.** Aucun de ses fichiers de code n'est touché ici ; dans `docs/plan-3.md`, 1b.19 écrit sa ligne, 1b.20 la sienne. `make value-members` rend alors 185 (H1). Les numéros de ligne cités sont ceux de HEAD et peuvent glisser : chaque site est aussi désigné par son contenu.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H18).

### Lignes de recensement (`tools/member_census.txt:174-181`)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `kq` | offset | `ref:autre` | jamais | - | l |
| `fa` | offset | `fixe` | jamais | - | l |
| `ra` | offset | `ref:autre` | observé | stat? | l |
| `eh` | offset | `ref:autre` | jamais | - | l |
| `ke` | offset | `ref:autre` | observé | stat | l |
| `jb` | offset | `ref:pico8_jb_t` | observé | stat? | - |
| `kh` | offset | `bool` | observé | stat | **-** |
| `m_` | offset | `bool` | observé | stat? | **e** |

Huit membres : 185 − 8 = **177**, comme le dit la ligne.

**Ce que la table ne voit pas.** Elle a été réécrite pour la dernière fois à 1b.4b (`1a50801`). Elle ne passe pas `UNIT_TESTS_SLOW` (1b.11), et elle ne connaît pas `test_ie_ke_view_by_name`, ajouté à `test_pico8_entity_kind` par 1b.11 après sa dernière réécriture. Les lectures par nom qui lui manquent, relevées par `grep` :

- `test_pico8_ie` (lent) : `check_fresh_icon` fait `assert(!has(icon, …))` sur `absent[] = {"kh", "m_", …}` (lecture nil par nom de **`kh` et `m_`**, preuve `lu`) ; `fold_icon` plie `bool_state(icon, "kh")`, `fold_vec(icon, "ra")` et `bool_state(icon, "m_")` sur chaque icône à chaque frame des quatre rejeux (états 0 absent, 1 `false`, 2 `true`) ; `test_directed` prend `hidden = first_icon_with_kh(0)` et vérifie `bool_state(hidden, "kh") == 0` ; `test_draw` vérifie `bool_state(…, "m_") == 2` après écriture ; `check_fresh_icon` et `test_construction` lisent aussi `kq`, `fa`, `ra`, `eh`, `ke`, `jb` par nom (valeurs et identités) ; `is_icon` lit `ke` par nom ;
- `test_pico8_entity_kind` (rapide, dans `CENSUS_UNIT_TESTS`) : `ie_bool_state(icons[i], "kh")` après `pico8_ie_hb`, et deux écritures de `m_` par nom par icône révélée.

**Décision R2 qui en découle** : `kh` et `m_` sont des non-pointeurs lus par nom (`l`) et lus nil (`lu`) : **`has_kh` et `has_m_`**. Sans eux, un `kh` ou un `m_` nil serait lu « présent `false` » (état 1 au lieu de 0) : `CONSTRUCTION_HASH`, `START_HASH`, les quatre empreintes de rejeu et `DIRECTED_HASH` de `test_pico8_ie` changeraient, et `check_fresh_icon` avorterait.

**Revue des autres TU lents (règle R2 de 1b.11)** : `test_pico8_jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr` ne nomment ni `ie` ni `research_icon` (`grep -c` à 0). Leurs rejeux atteignent `ie` par les sites de `src/` (`dc`, `gw`), inchangés. **Aucune lecture nil par nom d'un autre membre** (H4).

- `kq` : la boîte `b(0,0,9,8)` de la spec du module (`pico8_qz_constructor` de `collect.c` rend `pico8_value_ref(pico8_qz(…))`, un `pico8_qz_box_t *` du tas, tag `UNKNOWN`). Lue par nom par `gk` (`entity_geom.c`, `ie.hd = gk`), qui la caste en `const pico8_qz_box_t *`. Jamais écrite hors constructeur.
- `fa` : fixe 12, posé par le constructeur, lu par nom par `dc` (`entity.c`, tri par couche), `gw` (`ui.c`), `gk`. Jamais nil.
- `ra` : le vecteur `v(999,999)` de la spec, puis, dans `hb`, **`self->ra = g_ie_gy`** (écriture directe, R6), c'est-à-dire la REF que tient le curseur partagé `ie.gy`. `g_ie_gy` vaut `REF NULL` entre `pico8_research_icon_init` et `_init` (qui le pose sur `eh` par `pico8_ie_set_gy(pico8_ie_eh())`) : `hb` appelé avant `_init` (c'est le cas de `test_ie_ke_view_by_name`) écrit donc `ra` **nil**. Lu par nom par `dc` (`qc.ra` passé à `kj`), `gk`, et les tests.
- `eh` : le vecteur `v(118,120)` de la spec. Lu par nom par les tests. `pico8_ie_eh()` lit la spec, pas le membre.
- `ke` : la clé de construction `ke` (`cart.c` : `pico8_value_ref((void *)gx[i])`, un des sept `static const pico8_entity_kind_t`), nil sans clé. Lu directement par `ie_self_ke_kind` (preuve `stat` : `self->ke.kind == REF && self->ke.as.ref != NULL`, puis `assert` tag `UNKNOWN`), lu par nom par les tests.
- `jb` : la clé de construction `jb` (`cart.c` : `pico8_jb_new(dr_entry)`), nil sans clé. Lu directement par `kj` (`self->jb.kind == REF && …`, puis `assert` tag `STRUCT` « ie: unlike self.ke, self.jb holds a built jb struct »).
- `kh` : nil à la construction, **`true`** posé par `hb` (`self->kh = pico8_value_bool(1)`, R6), jamais `false`, jamais écrit par nom. Lu directement par `hb` et `kj` (`ie_truthy`).
- `m_` : nil à la construction, écrit par nom par `gw` (`ui.c` : `false` sur l'ancien `gj`, `true` sur `o_`) et par les tests. Lu directement par `kj` (`ie_truthy`).

### Genres mêlés : aucun

Chaque ligne n'a qu'un genre, vérifié côté écrivains : `m_` ne reçoit que `pico8_value_bool(0/1)` (`gw`, tests) ; `kh` que `pico8_value_bool(1)` ; `ke` que des REF de genres ; `jb` que des REF de `pico8_jb_new` ; `ra` que `g_ie_gy` (REF NULL ou REF d'un vecteur : `eh`, ou le vecteur neuf du tas de `hb`). **Aucune lecture de R1 pour genres mêlés** n'est décidée ici.

### Ce que fait le cart (`pico8/lua.lua`)

- `ie=ke:jr([[kq=b(0,0,9,8),fa=12,id=0,ra=v(999,999),eh=v(118,120),]])`, `ie.hd=gk` : défauts de classe ; `ie.gy`, `ie.id` sont des champs **de la classe**, mutés sur elle (état partagé, pas des membres).
- `ie:hb` : `if not self.kh and oj(self.ke.rr) then self.ra,self.kh=ie.gy,true ie.gy-=v(10,0) end` — `-=` rebinde `ie.gy` sur un vecteur neuf ; `self.ra` garde l'ancien.
- `ie:kj` : `spr(self.ke.s,…)`, puis `if self.m_ then dl(self.ke) self.jb:ol(0,0) end`.
- `ie:ldown` : `ie.id+=1`, rend `self.ke({id="s"..ie.id})`.
- `gx={qh,os,nr,fr,nl,mx,hj}`, un `ie({ke=b,jb=jb(dr[i])})` par genre. `self.ke` **est** la classe Lua : c'est un descripteur de genre, jamais une entité (1a.42a ; vue de classe de 1b.11).

### Le code aujourd'hui

`src/pico8_research_icon.h` (`:21-34`) : `{ pico8_ke_t ke_base; pico8_value_t kq, fa, ra, eh, ke, jb, kh, m_; } pico8_ie_t` (un membre par ligne), sous le contrat « `kq`, `ra` and `eh` point at the objects the module owns, shared by every `ie` and never modified in place; `hb` rebinds `ra` to the cursor. `gy` and `id` are no members: see the module's shared state. Nothing frees the struct: TROU-7. » Commentaire de module (`:12-19`) : « a nil or wrong-kind member, or an absent field of `self.ke`, reads as its zero/false default; a wrong-kind field of `self.ke` stops the run. `self.ke` holds a `pico8_entity_kind_t`, not an entity… ». Inclut `pico8_vec.h`, `pico8_struct.h` ; **n'inclut ni `pico8_geom.h` (`pico8_qz_box_t`) ni `pico8_layout.h` (`pico8_jb_t`)**. API publique inchangée : `pico8_research_icon_init`/`_shutdown`, `pico8_ie_ready`, `_default`, `_struct_class`, `pico8_ie_new`, `pico8_ie_eh`, `pico8_ie_set_gy`, `pico8_ie_hb`, `pico8_ie_ldown`, `pico8_ie_ke_view`, `pico8_ie_ke_view_struct_class`, `pico8_ie_kj`.

`src/pico8_research_icon.c` (463 lignes, tout `ie`) :

- **État module** : `g_ie_spec` (possède `kq`, `ra`, `eh`, libérés au `shutdown`), `g_ie_chain_spec`, `g_ie_gy`, `g_ie_id` (`pico8_value_t`, **pas des membres** : non typés ici).
- **`ie_truthy`** (`:37-45`) : vérité Lua sur un `pico8_value_t` ; trois appelants, tous sur `kh` ou `m_` (`:178`, `:260`, `:276`).
- **`ie_self_ke_kind`** (`:60-67`) : lit `self->ke` (REF non NULL), `assert` tag `UNKNOWN` « ie.ke holds a kind, not an entity or a table », rend le genre ou NULL. Appelé par `pico8_ie_ke_view`, `ldown`, `kj`.
- **Vue `ie_ke`** (`:69-161`) : typée en 1b.11, **non touchée**.
- **`pico8_ie_hb`** (`:171-205`) : `ie_truthy(self->kh)` ; `rr` de la vue ; `pico8_oj` ; `self->ra = g_ie_gy;` ; `self->kh = pico8_value_bool(1);` ; rebinde `g_ie_gy` sur un vecteur neuf (`ie_value_vec(g_ie_gy)` − (10,0)).
- **`pico8_ie_kj`** (`:256-289`) : garde `ie_truthy(self->kh)` ; sprite `s` de la vue ; si `ie_truthy(self->m_)` : `dl` si le genre existe, puis `jb` REF non NULL → `assert` tag `STRUCT` → `pico8_jb_ol(…, (const pico8_jb_t *)self->jb.as.ref, 0, 0)`.
- **Lecteur** `pico8_ie_field` (`:311-344`) : `gy`, `id` (état module) ; sinon table `members[]` `offsetof` des huit membres (`:316-320`), recopie `*(const pico8_value_t *)((const char *)ie + members[i].offset)` (`:337`) ; nom hors table → `pico8_ke_field` ; `REF NULL` → 0.
- **Écrivain** `pico8_ie_set_field` (`:348-356`) : `m_` seul, `ie->m_ = value` (n'importe quel genre), `return 1` ; autre nom → 0.
- **Constructeur** `pico8_ie_struct_new` (`:379-411`) : `malloc`, `pico8_ke_struct_init`, `kq`/`ra`/`eh` = `pico8_ie_spec_ref(…)` (même REF que la spec), `fa` = fixe 12, `ke`/`jb`/`kh`/`m_` = `REF NULL` ; boucle des clés : `assert((is_ke || is_jb) && "ie: the only measured construction keys are ke and jb (eu); …")`, puis `self->ke = value` ou `self->jb = value` (n'importe quel genre) ; `pico8_class_coverage_record("ie")` ; `pico8_ke_struct_pq` ; `PICO8_MEMBER_CENSUS_SEE(self, pico8_ie_t)`.

`src/pico8_layout.[ch]` : `g_jb_struct_class` est `static`, **sans accesseur public** (`grep -rn 'jb_struct_class' src tests` ne rend que `layout.c`). La forme 1b.13 (point 4) demande `cls == pico8_jb_struct_class()`.

**Sites par nom, inchangés** : `dc` (`entity.c`, `fa`, `ra`), `gk` (`entity_geom.c`, `fa`, `ra`, `kq`), `gw` (`ui.c`, `fa` lu, `m_` écrit) ; les TU.

**Arrêt « nom inconnu »** : `pico8_entity_set_field` appelle l'écrivain, puis `PICO8_MEMBER_CENSUS_WRITE`, puis `assert(written && …)`. Inchangé ; déjà figé pour `kh`, `ra`, `gy`, `id` par `test_ie_is_a_struct` (lent).

### Empreinte remesurée

La ligne dit 26 (10, 16) : majorant des noms courts partagés (`fa`, `ra`, `kq`, `m_`, `ke`, `jb`…). **Empreinte réelle** (fichiers qui lisent ou écrivent un membre d'un `pico8_ie_t`, directement ou par nom sur un `ie`) : **8 (5, 3)** —

- `src/` (5) : `pico8_research_icon.h` (typedef), `pico8_research_icon.c` (direct), `pico8_entity.c` (`dc` : `fa`, `ra`), `pico8_entity_geom.c` (`gk` : `fa`, `ra`, `kq`), `pico8_ui.c` (`gw` : `fa`, `m_`) ;
- `tests/` (3) : `test_pico8_ie` (tous, lent), `test_pico8_entity_kind` (`kh`, `m_`, clé `ke`), `test_pico8_spec_defaults` (`STRUCT_HASH` : `fa`, `ra`, `kq`, `eh` dans `NAMES`).
- Hors compte : `pico8_cart.c` (construit les sept `ie` avec `ke`/`jb`), `test_pico8_ui` (construit un `ie`, n'en lit que `me`/`t`, repli `ke`).

**Fichiers modifiés : 5**, plus `docs/plan-3.md` : `src/pico8_research_icon.h`, `src/pico8_research_icon.c`, `src/pico8_layout.h` et `src/pico8_layout.c` (accesseur `pico8_jb_struct_class`, pur ajout), `tests/test_pico8_entity_kind.c` (pur ajout).

**TU de `UNIT_TESTS_SLOW` touché : `test_pico8_ie`**, par compilation et par exécution, **avec un diff vide**. Aucun autre TU lent ne nomme `ie`. **Aucun test de régression (§6)** ne nomme `ie`.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code**, dans le TU rapide `test_pico8_entity_kind` : lectures par nom des huit membres, clés `ke`/`jb` nil et non nil, aller-retour d'écriture par nom de `m_` (dont `false` à union nulle et nil), `kh`/`ra` par `hb` (dont `ie.gy` nil), `kj` avec `jb` et `m_`, écrivain brut sur les autres noms (voir Testing Decisions).
2. **Typage** (en-tête `ke_base` en tête, noms et ordre gardés, chaque `has_` juste après son membre) :

   ```c
   pico8_ke_t ke_base;
   pico8_qz_box_t *kq;
   pico8_fixed_t fa;
   pico8_vec_t *ra;
   pico8_vec_t *eh;
   const pico8_entity_kind_t *ke;
   pico8_jb_t *jb;
   int kh;
   int has_kh; /* 0: `kh` is nil, and is then never read */
   int m_;
   int has_m_; /* 0: `m_` is nil, and is then never read */
   ```

3. **Lecteur** : branches `gy`/`id` gardées, puis huit branches R4 `strcmp` dans l'ordre du typedef, puis le repli `pico8_ke_field` ; `members[]` et la recopie `offsetof` supprimées (R7 de 1b.9).
4. **Écrivain** : la branche `m_` appelle `ie_take_bool`, forme 1b.15 exacte.
5. **Clés `ke` et `jb`** : deux aides R5 `static`, `ie_take_kind` et `ie_take_jb` ; l'`assert` des clés reste tel quel.
6. **R6** : `hb` pose `kh = 1; has_kh = 1;` et rebinde `ra` par `ie_take_vec(g_ie_gy, &self->ra)` ; `kj` lit `has_kh && kh`, `has_m_ && m_`, `self->jb` ; `ie_self_ke_kind` rend `self->ke` ; `ie_truthy` disparaît (plus d'appelant).
7. **Accesseur** `const pico8_struct_class_t *pico8_jb_struct_class(void)` ajouté à `layout.[ch]`.
8. **Un test d'arrêt**, rouge sur l'ancien code (qui accepte tout), vert après, avec les cas non-REF à union nulle.
9. Effet mesurable : `make value-members` passe de 185 à **177** et ne liste plus aucune ligne `pico8_ie_t`. Aucun pixel ne bouge.
10. **Note « Découvert à 1b.20 »**.

**Pas de découpage** (G) : un seul `.c` porte le code (une centaine de lignes de diff), et un lecteur à deux formes (branches R4 et table `offsetof`) pour une seule struct serait pire que le tout.

## User Stories

1. En tant que lecteur de `research_icon.h`, je veux `kq` en `pico8_qz_box_t *`, `ra`/`eh` en `pico8_vec_t *`, `ke` en `const pico8_entity_kind_t *`, `jb` en `pico8_jb_t *`, `fa` en `pico8_fixed_t`, `kh`/`m_` en `int` suivis de `has_kh`/`has_m_`, afin que ce que tient l'icône se lise sur le type.
2. En tant que `gk`, je veux lire par nom `fa` = 12, `ra` et `kq` (REF, même pointeur que la spec pour `kq`) comme aujourd'hui.
3. En tant que `dc`, je veux lire `fa` et `ra` par nom, et `ra` absent quand `hb` a copié un `ie.gy` nil.
4. En tant que `gw`, je veux écrire `m_ = true` sur l'icône survolée et `m_ = false` sur l'ancienne, par nom, comme aujourd'hui.
5. En tant que lecteur par nom de `m_`, je veux 0 avec `*out_value` intact sur une icône neuve, `BOOL 1` après `true`, `BOOL 0` **présent** après `false`, et absent après nil.
6. En tant que mainteneur, je veux qu'un `false` à union nulle (`memset`) écrit sur `m_` soit lu `false` présent, jamais nil.
7. En tant que mainteneur, je veux qu'une écriture de `m_` ni nil ni booléenne (fixe, fixe 0 à union nulle, chaîne, chaîne à union nulle, fonction, fonction à union nulle, REF de vecteur, de table, de struct, de genre) avorte par l'`assert` qui nomme `ie.m_` et `bool`.
8. En tant que `test_pico8_ie` et `test_pico8_entity_kind`, je veux lire `kh` absent avant `hb` et `BOOL 1` après une révélation, jamais « présent `false` ».
9. En tant que `ie:hb`, je veux que `ra` prenne exactement la REF que tient `ie.gy` (même pointeur), nil si `ie.gy` est nil, puis que `ie.gy` soit rebindé sur un vecteur neuf.
10. En tant que mainteneur, je veux qu'un `ie.gy` ni nil ni vecteur (fixe, fixe 0 à union nulle, REF de table, de struct) arrête `hb` par l'`assert` qui nomme `ie.ra` et `ref:autre`.
11. En tant que `eu`, je veux construire une icône avec `ke` = un genre et `jb` = un `jb` construit, et les relire par nom, même pointeur.
12. En tant que mainteneur, je veux qu'une clé `ke` ni nil ni REF de tag `UNKNOWN` (fixe, `false`, chaîne, fonction à union nulle, REF de struct, REF de table, de liste) avorte par l'`assert` qui nomme `ie.ke`, **à la construction** et non plus au premier usage.
13. En tant que mainteneur, je veux qu'une clé `jb` ni nil ni REF d'un `pico8_jb_t` (fixe, `false`, chaîne, REF de genre, de vecteur, de table, REF d'un autre `ie`, d'un `lq`) avorte par l'`assert` qui nomme `ie.jb` et `ref:pico8_jb_t`.
14. En tant que `ie:kj`, je veux le même dessin qu'aujourd'hui : rien avant `hb`, le sprite après, `dl` et la bulle `jb` quand `m_` est vrai.
15. En tant que `ie:ldown`, je veux construire une instance du genre tenu par `ke`, ou rien sans `ke`, et `ie.id` incrémenté dans les deux cas.
16. En tant que vue `ie_ke` (1b.11), je veux `ie_self_ke_kind` qui rend le même genre qu'aujourd'hui, pour que `RAW_PRESENCE_HASH`, `ABSENT_COUNT` et `BEHAVIOR_HASH` ne bougent pas.
17. En tant que mainteneur, je veux que l'écrivain brut rende 0 sans rien toucher pour tout nom autre que `m_`, et qu'écrire `kq`, `fa`, `ra`, `eh`, `ke`, `jb`, `kh` par nom avorte toujours, par la couche entité.
18. En tant que lecteur par nom, je veux que `gy` et `id` restent l'état partagé du module, et que `me` et `t` restent servis par le repli `pico8_ke_field`.
19. En tant qu'auteur de 1b.34 (`js.jb`) et des étapes `jo` (`jo.kq`, `jo.ra`, `jo.m_`), je veux trouver `pico8_jb_struct_class()` public, une forme écrite de `ref:autre` vers une boîte (`kq`) et vers un descripteur de genre (`ke`).
20. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
21. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
22. En tant que gardien de la couverture, je veux `make class-coverage-net` identique à sa référence (`ie  133  15/15`, `jb  831  15/15`), puisque le constructeur change.
23. En tant que `make float-audit`, je veux le site C1 `research_icon.c | ie_to_int | x` inchangé.
24. En tant que `plan-pilot`, je veux `make value-members` à 177.
25. En tant que vérificateur, je veux la caractérisation lancée sur l'ancien code, le test d'arrêt rouge puis vert, les mutations du §7 avec leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés comme le ferait un agent `guideline` (hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables), et marqués (G).

### Deux types à trancher

- **`ke` → `const pico8_entity_kind_t *`** (G). La ligne dit `ref:autre` : R1 laisse le type au PRD. Ce que tient `ke` est un descripteur de genre (`struct.h` : « one `static const` per kind, never freed, whose first member is no tag »), pas une entité (1a.42a, vue 1b.11) : ni `pico8_struct_header_t *`, ni `void *`. **`const`**, en écart assumé avec le « pointeurs non `const` » de R1, qui vise les cibles qu'on modifie (tables, listes, structs) : les genres sont des objets `static const`, un pointeur non `const` inviterait une écriture indéfinie. Tous les consommateurs (`ie_self_ke_kind`, la vue, `ldown`, `kj`) prennent déjà un `const pico8_entity_kind_t *`. Le seul retrait de `const` est l'emballage R4 `pico8_value_ref((void *)self->ke)`, le même cast que `cart.c` et les TU font déjà pour poser la clé. Aucun `-Wcast-qual` dans `CFLAGS` (`-std=c99 -Wall -Wextra -Werror -g`).
- **`jb` → `pico8_jb_t *`** (R1, `ref:pico8_X_t`, cible typée en 1b.2), non `const`. `pico8_jb_ol` prend un `const pico8_jb_t *` : conversion implicite.
- `kq` → `pico8_qz_box_t *` (lecture `ref:autre` de 1b.4 appliquée à une boîte : le type est celui que construit `b(…)`, et celui que `gk` caste déjà) ; `ra`, `eh` → `pico8_vec_t *` (1b.4). Non `const`. Pas une lecture nouvelle.

### R1–R9 appliquées à `ie`

| règle | sur `ie` |
|---|---|
| R1 | Voir la Solution et ci-dessus. Noms et ordre gardés. Aucun genre mêlé. `research_icon.h` ajoute `#include "pico8_geom.h"` et `#include "pico8_layout.h"` (H12). |
| R2 | `has_kh` et `has_m_`, chacun juste après son membre (preuve `lu` du TU lent `test_pico8_ie`, règle de 1b.11 ; pour `m_`, la ligne `e` sans `l` est complétée de la même façon). Aucun autre : `fa` nil `jamais` (1b.6) ; `kq`, `ra`, `eh`, `ke`, `jb` pointeurs (NULL = nil). Contrats au format de `trade.h`. |
| R3 | Dans `pico8_ie_struct_new`, à la place des huit affectations, même position (après `pico8_ke_struct_init`, avant la boucle des clés, `pico8_class_coverage_record` et `pico8_ke_struct_pq`) : `kq = pico8_ie_spec_ref("kq").as.ref`, `fa = pico8_fixed_from_int(12)`, `ra = pico8_ie_spec_ref("ra").as.ref`, `eh = pico8_ie_spec_ref("eh").as.ref`, `ke = NULL`, `jb = NULL`, `kh = 0`, `has_kh = 0`, `m_ = 0`, `has_m_ = 0`. Tous écrits (bloc de `malloc`). **Défaut module, même REF** (1b.17) : `kq`, `ra`, `eh` sont les pointeurs de `g_ie_spec`, partagés par toutes les icônes ; `pico8_ie_spec_ref` et son `assert` restent. |
| R4 | Après `gy`/`id` (inchangés : locale `value` copiée de l'état module, `REF NULL` → 0), huit branches `strcmp` dans l'ordre du typedef, avant `pico8_ke_field`. `kq`, `ra`, `eh`, `jb` : NULL → `return 0`, sinon `pico8_value_ref(<m>)`, même pointeur. `ke` : NULL → 0, sinon `pico8_value_ref((void *)ke)`. `fa` : toujours présent, `pico8_value_fixed`. `kh`, `m_` : `has_<m>` = 0 → `return 0`, sinon `pico8_value_bool(<m>)`. Dans chaque 0, `*out_value` intact. |
| R5 | **Écrivain** : `ie_take_bool` (section suivante). **Boucle de clés** : l'`assert((is_ke \|\| is_jb) && …)` et son message restent tels quels, à leur place ; `self->ke = value` devient `ie_take_kind(value, &self->ke)`, `self->jb = value` devient `ie_take_jb(value, &self->jb)`. |
| R6 | `hb` : `ie_truthy(self->kh)` → `self->has_kh && self->kh` ; `self->ra = g_ie_gy;` → `ie_take_vec(g_ie_gy, &self->ra);` ; `self->kh = pico8_value_bool(1);` → `self->kh = 1; self->has_kh = 1;`. `kj` : garde `!(self->has_kh && self->kh)` ; `ie_truthy(self->m_)` → `self->has_m_ && self->m_` ; le bloc `jb` devient `if (self->jb != NULL) { pico8_jb_ol(gfx, mem, self->jb, 0, 0); }`. `ie_self_ke_kind` devient `return self != NULL ? self->ke : NULL;`. `ie_truthy` n'a plus d'appelant : supprimé avec son commentaire (sinon `-Werror`). Ordre des instructions de `hb`/`kj`/`ldown` inchangé ; `ie_value_vec(g_ie_gy)`, le `malloc` du curseur, `ie_to_int` et `g_ie_gy`/`g_ie_id` inchangés. |
| R7 | **Forme `offsetof` de 1b.9** : les huit entrées sortent, la table `members[]` est vide, supprimée avec sa boucle, son `break` et la recopie `*(const pico8_value_t *)(…)`. `#include <stddef.h>` reste si un autre usage l'exige, sinon il sort (le compilateur le dira ; `size_t` vient aussi de `<string.h>`). |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_ie_t)` gardé ; `PICO8_MEMBER_CENSUS_WRITE` inchangé ; `tools/member_census.txt` non édité. |
| R9 | 185 → 177, aucune ligne `pico8_ie_t`. `has_kh`, `has_m_` ne comptent pas. |

### R5 : quatre aides `static`, une par genre

Toutes par valeur (1b.10), au nom de la struct, placées avant leur premier appelant.

1. **Écrivain.** Une seule branche, `m_` (seul membre `e`) : `if (strcmp(name, "m_") == 0) { ie_take_bool(value, &ie->m_, &ie->has_m_); return 1; } return 0;`. Le commentaire « `m_` (the hover loop…): the only name a generic writer ever writes on an `ie` » reste.
2. **`static void ie_take_bool(pico8_value_t value, int *out, int *has)`**, copie exacte de `bg_take_bool` : `*out = 0; *has = 0;` sans condition ; `if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) return;` ; `assert(value.kind == PICO8_VALUE_BOOL && "ie.m_: a write neither nil nor a boolean is off its census row (bool) -- a field under no rule of plan-3 section 5: stop, do not widen the member");` ; `*out = value.as.boolean;` ; `*has = 1;` en dernière instruction. Seule pour `m_` : `kh` n'est écrit que par le littéral de `hb` (R6 direct, sans valeur à déballer).
3. **`static void ie_take_kind(pico8_value_t value, const pico8_entity_kind_t **out)`** : `*out = NULL;` ; sortie sur `REF NULL` (genre et pointeur) ; `assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN && "ie.ke: a key neither nil nor a kind is off its census row (ref:autre) -- a field under no rule of plan-3 section 5: stop, do not widen the member");` ; `*out = value.as.ref;`. C'est l'`assert` de `ie_self_ke_kind`, déplacé de la lecture à l'écriture (G) : le membre typé ne peut plus tenir un mauvais genre, et le message « ie.ke holds a kind, not an entity or a table » disparaît (aucun TU ne le cite, H17). Limite connue, déjà vraie : une REF de vecteur a aussi le tag `UNKNOWN` et passerait (H3).
4. **`static void ie_take_jb(pico8_value_t value, pico8_jb_t **out)`** : `*out = NULL;` ; sortie sur `REF NULL` ; `assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT && ((const pico8_struct_header_t *)value.as.ref)->cls == pico8_jb_struct_class() && "ie.jb: a key neither nil nor a jb is off its census row (ref:pico8_jb_t) -- a field under no rule of plan-3 section 5: stop, do not widen the member");` ; `*out = value.as.ref;`. Remplace l'`assert` de `kj` (« ie: unlike self.ke, self.jb holds a built jb struct »), qui disparaît (G, H17).
5. **`static void ie_take_vec(pico8_value_t value, pico8_vec_t **out)`**, copie de `bg_take_vec`, pour la R6 de `ra` dans `hb` (forme 1b.18 : une écriture directe dont la valeur arrive en `pico8_value_t` passe par l'aide du genre) : `*out = NULL;` ; sortie sur `REF NULL` ; `assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN && "ie.ra: a gy neither nil nor a vector ref is off its census row (ref:autre) -- a field under no rule of plan-3 section 5: stop, do not widen the member");` ; `*out = value.as.ref;`.
6. **Nom inconnu : inchangé.** Pas d'`assert` dans l'écrivain.

### Accesseur `pico8_jb_struct_class`

`const pico8_struct_class_t *pico8_jb_struct_class(void);` déclaré dans `layout.h` près de `pico8_lq_struct_class`, défini dans `layout.c` (`return &g_jb_struct_class;`), sans commentaire (le nom dit le contrat). Pur ajout : ni le constructeur de `jb`, ni `pico8_class_coverage_record("jb")`, ni `g_jb_struct_class` ne changent (G : c'est l'accesseur public qu'impose 1b.13 point 4, et 1b.34 `js.jb` s'en servira).

### Commentaires (règle de `CLAUDE.md`)

- `research_icon.h`, commentaire de module : « a nil or wrong-kind member, or an absent field of `self.ke`, reads as its zero/false default » devient « a nil member, or an absent field of `self.ke`, reads as its zero/false default; a wrong-kind write or construction key stops the run » (contrat C : le mauvais genre ne peut plus être tenu). La suite (« `self.ke` holds a `pico8_entity_kind_t`… ») reste.
- Contrat de `pico8_ie_new` : « may hold `ke` … and `jb` …, both borrowed » reste exact.
- Contrat de `ie_self_ke_kind` : « or NULL when the member is nil or holds no REF » devient « or NULL when the member is nil ».
- Contrats `has_kh`, `has_m_` au format de `trade.h`. Aucun autre commentaire ; les aides portent leur contrat par leur `assert`.

### Preuves

Toutes sur l'arbre final, sauf mention. `F` = `src/pico8_research_icon.c`.

- `make value-members | grep -c pico8_ie_t` : 8 avant ; **0** après ;
- `grep -rn 'offsetof(pico8_ie_t' src/ tests/` : 3 lignes avant (`members[]`) ; **vide** après ;
- `grep -n 'pico8_value_t \*' F | grep -v 'pico8_value_t \*out_value)' | grep -v 'pico8_value_t \*out)'` : 1 ligne avant (`:337`, recopie `offsetof`) ; **vide** après ;
- `grep -nE '(->|\.)(kq|fa|ra|eh|ke|jb|kh|m_)\.(kind|as)' F` : 6 lignes avant (`:61`, `:62`, `:64`, `:280`, `:281`, `:283`) ; **vide** après ;
- `grep -nE 'self->(kq|fa|ra|eh|ke|jb|kh|m_) = pico8_value_|self->(ra|ke|jb) = (g_ie_gy|overrides)' F` : 9 lignes avant (`hb` : `ra`, `kh` ; init : `fa`, `ke`, `jb`, `kh`, `m_` ; boucle : `ke`, `jb`) ; **vide** après ;
- `grep -n 'ie->m_ = ' F` : 1 avant ; **vide** après ;
- `grep -c 'ie_truthy' F` : 4 avant ; **0** après ;
- `grep -c 'off its census row' F` : 2 avant (vue `ie_ke`) ; **6** après (+ `ie.m_`, `ie.ke`, `ie.jb`, `ie.ra`) ;
- `grep -n 'has_kh\|has_m_' src/pico8_research_icon.h` : 0 avant ; **2** après, chacune juste après son membre ;
- `grep -rn 'pico8_jb_struct_class' src/` : vide avant ; `layout.h`, `layout.c`, `research_icon.c` après ;
- `git diff --stat` : seuls les cinq fichiers listés plus haut et `docs/plan-3.md` ; le diff de `layout.[ch]` est un pur ajout ; aucune ligne de la vue `ie_ke` (`pico8_ie_ke_view_field` à `pico8_ie_ke_view_struct_class`) ne change.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 185 sur l'arbre de départ (1b.19 commitée), puis 177.
- **H2** : dans les 15 rejeux et dans les TU hors fils de `must_abort*`, `m_` d'un `ie` ne reçoit par nom que des `BOOL` (`gw`, tests) ou, dans la caractérisation, `REF NULL` ; la clé `ke` n'est qu'une REF de genre ; la clé `jb` n'est qu'une REF d'un `pico8_jb_new` ; `g_ie_gy` n'est, à l'appel de `hb`, que `REF NULL` ou une REF de vecteur. Sinon une aide avorte : arrêt « champ sans règle ».
- **H3** : `pico8_type_tag_of` rend `UNKNOWN` sur un genre (premiers octets : le pointeur de fonction `new_entity`), sur une boîte `pico8_qz_box_t` et sur un vecteur du tas (déjà supposé par l'`assert` actuel de `ie_self_ke_kind`) ; une REF de vecteur passée en clé `ke` n'avorte donc pas (limite de `ref:autre`, trou justifié).
- **H4** : aucun TU lent autre que `test_pico8_ie` ne lit un membre d'`ie` par nom (revue ci-dessus).
- **H5** : ne bougent pas : dans `test_pico8_ie`, `CONSTRUCTION_HASH`, `START_HASH`, `RECORDED_HASH`, `HUB_HASH`, `LONG_HASH`, `PLAYTHROUGH_HASH`, `DIRECTED_HASH`, `DRAW_HASH`, `RESTART_HASH`, `KE_VIEW_HASH` et les comptes de frames ; dans `test_pico8_entity_kind`, `KIND_HASH`, `RAW_PRESENCE_HASH`, `ABSENT_COUNT`, `BEHAVIOR_HASH` ; `STRUCT_HASH`/`SPEC_HASH`/`READY_HASH` de `test_pico8_spec_defaults` ; les empreintes de `test_pico8_ui` ; les 15 du filet.
- **H6** : le build `-DPICO8_MEMBER_CENSUS` compile sur `pico8_ie_t` sans `pico8_value_t`. Il n'est pas lancé.
- **H7** : sous ASan+UBSan, rien de nouveau : aucune allocation ajoutée ni retirée (le `malloc` du curseur de `hb` reste, TROU-7).
- **H8** : `test_pico8_entity_kind` reste sous 10 s avec les deux fonctions ajoutées (une douzaine de fils `fork` courts), bien sous 30 s ; sinon l'étape le note, le TU ne change pas de liste.
- **H9** : 1b.19, commitée, ne touche ni `research_icon.[ch]`, ni `layout.[ch]`, ni `test_pico8_entity_kind.c`.
- **H10** : `make class-coverage-net` rend `ie  133  15/15` et `jb  831  15/15`, identiques à `tools/class_coverage_net_expected.txt`.
- **H11** : `pico8_ob_table_set_name(t, "ke", pico8_value_ref(NULL))` garde l'entrée (`pico8_ob_table_set` ne retire rien : même lecture qu'en 1b.18, H3), donc une clé nil atteint la boucle.
- **H12** : `pico8_geom.h` et `pico8_layout.h` n'incluent pas `research_icon.h` (lu : ils incluent `fixed`, `vec`, `gfx`, `list`, `mem`, `ob`, `struct`, `value`) ; aucun cycle ; tous les includeurs de `research_icon.h` (`cart.c`, quatre TU) compilent.
- **H13** : `make float-audit` retrouve `research_icon.c | ie_to_int | x` inchangé (`ie_to_int` et ses appels ne changent pas).
- **H14** : les fils du test d'arrêt, créés par `fork` dans `test_pico8_entity_kind` (TU de `CENSUS_UNIT_TESTS`), ne vident rien au recensement : ils meurent par `SIGABRT`, ou sur l'ancien code sortent par `_exit(0)` (1b.4b).
- **H15** : après `pico8_cart_load()` et `pico8_jl()` (état de `test_pico8_entity_kind` en fin de `main`), `pico8_ie_hb` révèle toujours une icône sans `ke` et une icône de genre `qh` (`rr` nil → `pico8_oj(nil)` vrai), sans dépendre de l'état de recherche.
- **H16** : `pico8_ie_kj` sur une icône révélée dont `jb` est construit sur une ligne de `pico8_dr()` dessine une bulle quand `m_` est vrai (empreinte d'écran ≠ sans `m_`). Sinon l'empreinte fige quand même le dessin, et l'étape le note.
- **H17** : aucun TU ni doc hors PRD ne cite les messages « ie.ke holds a kind, not an entity or a table » et « ie: unlike self.ke, self.jb holds a built jb struct » (`grep` vide hors `docs/prd/`).
- **H18** : `STRUCT_HASH` compare `kq`, `ra`, `eh` de l'`ie` neuf à la spec de chaîne `g_ie_chain_spec` par identité ou valeur ; comme R3 garde les pointeurs de `g_ie_spec`, le résultat est celui d'aujourd'hui.

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Aucun accès direct à `->kq`/`->m_`… dans les tests : ils compilent sur les deux formes.
- **(G) Où : `test_pico8_entity_kind`, pas `test_pico8_ie`.** `test_pico8_ie` est lent (`UNIT_TESTS_SLOW`, trois rejeux de 5 000 à 27 000 frames) : y caractériser coûterait un rejeu complet par lancement, ce que le budget ne permet pas (même choix qu'en 1b.11). `test_pico8_entity_kind` est rapide, lie `ENGINE_SRCS`, construit déjà des `ie` après `pico8_cart_load()`/`pico8_jl()`, et appartient à `CENSUS_UNIT_TESTS`. On y ajoute `#define _POSIX_C_SOURCE 200809L` en tête, `<signal.h>`, `<sys/types.h>`, `<sys/wait.h>`, `<unistd.h>`, `pico8_data_init.h` (`pico8_dr`), `pico8_layout.h`, `pico8_vec.h`. Rien au Makefile.
- **Caractérisation dirigée, figée sur l'ancien code** : `test_ie_members_by_name`, appelée en fin de `main`, après `test_ie_ke_view_by_name`. Elle lit par le **lecteur brut** (`pico8_ie_struct_class()->field`, témoin : fixe `WITNESS` = `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder ; elle écrit par l'**écrivain brut** (`->set_field`) et par `pico8_entity_set_field`. `J` = `pico8_jb_new(ligne 3 de pico8_dr())`.
  1. **Lectures par nom.** `a` = `pico8_ie_new({ke = qh, jb = J})` ; `b` = `pico8_ie_new(NULL)` ; `n` = `pico8_ie_new({ke = REF NULL, jb = REF NULL})` (H11). Pour chacun : `kq` → REF, pointeur **identique** entre `a`, `b`, `n` (défaut module), plié par sa réponse à `pico8_qz_call` sur la couronne de points de `fold_kq` de `test_pico8_ie` ; `fa` → `FIXED 12` ; `ra` → REF (999,999), `eh` → REF (118,120), mêmes pointeurs entre icônes, `ra` ≠ `eh`, `eh` = `pico8_ie_eh().as.ref` ; `ke` → `a` : REF = `qh` (adresse), `b`/`n` : absent ; `jb` → `a` : REF = `J`, `b`/`n` : absent ; `kh`, `m_` → absents ; `me` → `STRING "ph"`, `t` → `FIXED 0` (repli) ; `gy`, `id` → présence et valeur pliées. Intrus `""`, `"KQ"`, `"x"`, `"has_kh"`, `"has_m_"`, `"ke_base"`, `"header"`, `"members"`, `"m"` → 0, témoin intact.
  2. **Aller-retour d'écriture par nom de `m_`**, sur `a` : `pico8_entity_set_field(…, "m_", true)` → `BOOL 1` ; écrivain brut `false` rend 1 → `BOOL 0` **présent** ; `false` à union nulle (`memset`, `kind = BOOL`) → `BOOL 0` présent ; `REF NULL` → **absent** ; `REF NULL` encore → absent ; `true` → 1 ; `REF NULL` → absent. À chaque pas : `kh` absent, `fa`, `kq`/`ra`/`eh`/`ke`/`jb` inchangés (valeurs et pointeurs), `m_` de `b` absent.
  3. **`kh` et `ra` par `hb`** : `pico8_ie_set_gy(pico8_value_ref(NULL))` ; `pico8_ie_hb(b)` → `kh` = `BOOL 1`, `ra` **absent**, `gy` présent (−10,0) ; `hb(b)` encore → rien ne change (`gy` même pointeur) ; `c` = `pico8_ie_new(NULL)`, `hb(c)` → `kh` 1, `ra` = **le pointeur** `gy` d'avant, `gy` nouveau pointeur (−20,0) ; `hb(a)` (`qh`, H15) → `kh` 1, `ra` = `gy` d'avant ; `d` = `pico8_ie_new({ke = nr})`, `hb(d)` → état de `kh` plié tel quel (dépend de la recherche) et, si `kh` reste absent, `ra` reste le pointeur de la spec et `gy` ne bouge pas ; enfin `pico8_ie_set_gy(pico8_ie_eh())`, `e` neuf, `hb(e)` → `ra` = `eh` (même pointeur).
  4. **`kj`** (écran vidé, `je.ra` à (64,64) comme en 1b.11) : sur `b` neuf non révélé → écran vide ; sur `c` (révélé, sans `ke`, sans `jb`) sans puis avec `m_` ; sur `a` (révélé, `qh`, `J`) sans puis avec `m_`, puis `m_` = `false` ; sur `f` = `pico8_ie_new({jb = J})` révélé, avec `m_`. Chaque empreinte d'écran est pliée (H16).
  5. **`ldown`** : `pico8_ie_ldown(a, …)` → REF d'une entité dont `id` est `"s<n>"` ; `pico8_ie_ldown(b, …)` → `REF NULL` ; `id` lu sur `a` et `b` incrémenté de 1 à chaque appel.
  6. **Écrivain brut sur un nom qu'il n'écrit pas** : `(a, n, pico8_value_bool(1))` rend 0 pour `n` ∈ {`"kq"`, `"fa"`, `"ra"`, `"eh"`, `"ke"`, `"jb"`, `"kh"`, `"gy"`, `"id"`, `"me"`, `"t"`, `""`, `"m_ "`, `"has_m_"`} ; l'état lu ensuite est celui d'avant. Puis, par un utilitaire `must_abort(e, name)` ajouté au fichier (`fork` + `SIGABRT`, forme de `test_pico8_ie`) : `pico8_entity_set_field` de `"ke"`, `"jb"`, `"kq"`, `"fa"` avorte — déjà vrai sur l'ancien code (« nom inconnu »), d'où sa place ici (`kh`, `ra`, `gy`, `id` le sont déjà dans le TU lent).
  7. On plie retours, genres, fixes, booléens, vecteurs lus, identités comparées (0/1), empreintes d'écran et comptes dans **`IE_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ. Les pointeurs sont jugés par `assert`, jamais pliés.

  Lancée verte sur l'arbre de départ **avant** le typage (ligne « Runs »). Ses valeurs ne changent pas ensuite.
- **Test d'arrêt, rouge d'abord** (G) : `test_ie_writer_stops_off_row`, appelé après la caractérisation, avec un utilitaire `must_abort_value(void *e, void (*write)(void *, pico8_value_t), pico8_value_t v)` (forme de `test_pico8_bg`) : chaque cas est un fils qui doit mourir par `SIGABRT`. Valeurs à union nulle par `memset(&v, 0, sizeof v)` puis `v.kind = …` : `zero_fixed`, `zero_false`, `zero_string`, `zero_function`.
  - **`m_` par nom** : `pico8_value_fixed(1)` ; `zero_fixed` ; `zero_string` ; `zero_function` ; `pico8_value_string("x")` ; la fonction `hb` lue sur `a` ; REF d'un vecteur du tas ; REF d'une table (`pico8_ob_table_new()`) ; REF de `hv` (liste) ; REF d'un autre `ie` ; REF du genre `qh`.
  - **clé `ke`** (le fils construit un `ie` avec `{ke = v}`) : `pico8_value_fixed(1)` ; `zero_fixed` ; `zero_false` ; `zero_string` ; `zero_function` ; `pico8_value_string("x")` ; REF d'un `ie` ; REF d'une table ; REF de `hv`.
  - **clé `jb`** (`{jb = v}`) : `pico8_value_fixed(1)` ; `zero_fixed` ; `zero_false` ; `zero_string` ; `zero_function` ; `pico8_value_string("x")` ; REF du genre `qh` ; REF d'un vecteur ; REF d'une table ; REF d'un autre `ie` ; REF d'un `lq` (`pico8_lq_new(NULL)`).
  - **`ie.gy` copié dans `ra` par `hb`** (le fils pose `pico8_ie_set_gy(v)` puis `hb` sur un `ie` neuf sans `ke`) : `pico8_value_fixed(1)` ; `zero_fixed` ; `zero_false` ; REF d'une table ; REF d'un `ie`.

  **Rouge sur l'ancien code** (l'écrivain, la boucle et `hb` recopient tout, le fils sort par `_exit(0)`), vert après : une ligne « Runs » pour chacun des deux passages. Il n'est pas dans la caractérisation figée, qui ne contient que ce que l'ancien code fait déjà. Le `false` à union nulle sur `m_` est le témoin accepté : il est dans la caractérisation (2), pas ici.
- **Tests existants, inchangés, qui figent déjà beaucoup** : dans `test_pico8_ie` (lent) : `check_fresh_icon`/`test_construction` (défauts, identités, absences de `kh`/`m_`), `test_start`, les quatre rejeux (`fold_icon` : `kh`, `ra`, `m_` à chaque frame), `test_directed` (`hb`, `ldown`, `hd`), `test_draw` (`m_` et `jb` à l'écran), `test_restart`, `test_ke_view`, `test_ke_view_is_a_struct`, `test_ie_is_a_struct` (arrêts sur `kh`, `ra`, `gy`, `id`, clé `ra`) ; dans `test_pico8_entity_kind` : `test_ie_ke_view_by_name` ; `test_struct_defaults` (`STRUCT_HASH`). Aucun test de régression (§6) ne nomme `ie`.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_entity_kind && ./tmp/test_pico8_entity_kind`. Les rougeurs attendues dans `test_pico8_ie` sont notées, pas lancées (budget). Chacune est notée dans le commit avec le test qui la rougit, ou la justification de son trou.
  1. (§7-1) *indicateur forcé vrai* : (a) la branche R4 de `m_` ignore `has_m_` → rouge : caractérisation 1 et 2 (hors budget : `check_fresh_icon`) ; (b) celle de `kh` ignore `has_kh` → rouge : caractérisation 1 et 3, `BEHAVIOR_HASH`.
  2. (§7-2) *« présent 0 » là où c'était absent* : **sans objet dans sa lettre** — le seul fixe, `fa`, n'est jamais absent. Forme la plus proche, jouée : (a) R3 pose `has_m_ = 1` → rouge : caractérisation 1 ; (b) R3 pose `has_kh = 1` → rouge : caractérisation 1 (`kh` lu `BOOL 0` présent au lieu d'absent sur une icône neuve ; hors budget : `check_fresh_icon`, `first_icon_with_kh(0)` de `test_directed`).
  3. (§7-3) *init ou boucle qui saute un membre typé*, `malloc` → `calloc` dans la copie : (a) **boucle** : `if (0) ie_take_kind(…)` → rouge : caractérisation 1 (`ke` de `a`), `RAW_PRESENCE_HASH`/`ABSENT_COUNT` ; (b) `if (0) ie_take_jb(…)` → rouge : caractérisation 1 et 4 ; (c) `kq` omis → caractérisation 1 ; (d) `ra` omis → caractérisation 1 ; (e) `eh` omis → caractérisation 1 ; (f) `fa = 12` omis → caractérisation 1 (`FIXED 0`) ; (g) `ke = NULL`, `jb = NULL`, `kh`/`has_kh`/`m_`/`has_m_` = 0 omis : **trou justifié**, `calloc` pose déjà l'état nil, et sous `malloc` l'omission est une lecture non initialisée qu'aucun outil du filet ne signale ; les lignes sont prouvées exécutées par `gcov`.
  4. *nil jugé sur le pointeur seul* dans `ie_take_bool` → rouge : caractérisation 2 (`false` à union nulle lu absent), test d'arrêt (`zero_fixed`, `zero_string`, `zero_function`).
  5. *nil jugé sur le genre seul* (`kind == REF`) dans `ie_take_bool` → rouge : test d'arrêt (REF de vecteur, table, liste, `ie`, genre).
  6. *nil n'éteint pas la présence* : `ie_take_bool` teste nil avant `*out = 0; *has = 0;` → rouge : caractérisation 2.
  7. *`*has = 1` retiré* de `ie_take_bool` → rouge : caractérisation 2 et 4, `BEHAVIOR_HASH`.
  8. *tag non vérifié* dans `ie_take_kind` → rouge : test d'arrêt (clé `ke` = REF d'`ie`, table, liste).
  9. *classe non vérifiée* dans `ie_take_jb` (tag `STRUCT` seul) → rouge : test d'arrêt (clé `jb` = REF d'`ie`, de `lq`).
  10. *tag non vérifié* dans `ie_take_vec` → rouge : test d'arrêt (`gy` = REF de table, d'`ie`).
  11. *écrivain qui accepte un autre nom* : branche `kh` qui rend 1 → rouge : caractérisation 6.
  12. *R6 muet sur `kh`* : `hb` ne pose plus `has_kh = 1` → rouge : caractérisation 3 et 4, `BEHAVIOR_HASH`.
  13. *R6 muet sur `ra`* : `if (0) ie_take_vec(g_ie_gy, &self->ra)` → rouge : caractérisation 3 (`ra` ≠ l'ancien `gy`) ; hors budget : `START_HASH`, rejeux.
  - **Mutants équivalents, non retenus, notés dans le commit** : `kj` qui lit `self->m_` sans `has_m_`, ou `self->kh` sans `has_kh` (les aides et R3 tiennent `<m>` à 0 quand `has_<m>` vaut 0) ; `kj` sans le test `jb != NULL` (`pico8_jb_ol` ne dessine rien sur NULL).
  - **Trou justifié** : une REF de vecteur en clé `ke` passe `ie_take_kind` (même tag `UNKNOWN`, H3) ; l'ancien code avait la même limite à la lecture.
- **`gcov`** : `test_pico8_entity_kind` en `--coverage -O0` ; ses seuls fils sont ceux de `must_abort`/`must_abort_value`, qui avortent sans vider leur compteur (pas de piège `__gcov_dump` à poser). Chaque ligne modifiée est exécutée : les huit branches R4 présentes et absentes, les quatre aides en sortie nil et en recopie, R3, `hb` (`gy` nil et non nil, `kh` déjà vrai), `kj` (`m_`, `jb` NULL et non NULL), `ie_self_ke_kind` (NULL et non NULL), `pico8_jb_struct_class`. Trou attendu : la branche d'échec des quatre `assert` d'aide, exécutée seulement dans les fils (prouvée par le test d'arrêt et les mutations 4, 5, 8, 9, 10). Le rapport va dans le commit.
- **Prior art** : `test_bg_members_by_name` et `test_bg_writer_stops_off_row` (1b.15, `must_abort_value`, `zero_*`), les tests de 1b.18 (`test_pico8_fq`, `zero_function`) et de 1b.19 (`test_pico8_qb`), `test_ie_ke_view_by_name` (1b.11, même TU, témoin, `ie_bool_state`, empreinte d'écran), `fold_kq`/`check_fresh_icon`/`must_abort` de `test_pico8_ie`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_entity_kind` : quelques secondes (H8) ; environ 25 lancements (caractérisation et test d'arrêt sur l'ancien code, nouveau code, 13 mutations dont 1, 2 et 3 en plusieurs formes, `gcov`), 3 à 5 min. Si un lancement dépasse 20 s, l'implémenteur joue d'abord les mutations §7 (1 à 3) et 4, 7, 9, et note les autres comme non lancées ;
- `make tmp/test_pico8_ie` (**compilation seule**, tôt, pour voir le TU lent compiler sur le nouveau typedef) : quelques secondes. Il n'est **jamais** lancé seul : `make e2e` le lance une fois ;
- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min (c'est lui qui lance `test_pico8_ie`, et `float-audit`) ;
- `make class-coverage-net` : 3 à 4 min, dans la même invocation que `make e2e` (arbres distincts, comme en 1b.14a, 1b.15, 1b.18) ;
- `make sanitize-net` : ~4 min seul ; **jamais** lancé en même temps que `e2e` ;
- `make value-members` : quelques secondes.

**TU de `UNIT_TESTS_SLOW` touché : `test_pico8_ie`**, diff vide, compilé seul tôt, lancé une fois par `make e2e`. Aucun autre TU lent ne nomme `ie`.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` sans erreur de liste C1, et les `UNIT_TESTS_SLOW` qui passent, **`test_pico8_ie` compris** (`test_pico8_ie: all tests passed`) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 177`**, sans aucune ligne `pico8_ie_t` ;
5. `timeout 15m make class-coverage-net` : **s'applique**, parce que le diff touche le corps du constructeur `pico8_ie_struct_new` (R3, R5), sans toucher `pico8_class_coverage_record("ie")`, celui de `jb` ni `KNOWN[]`. Sortie identique à `tools/class_coverage_net_expected.txt` (`ie  133  15/15`, `jb  831  15/15`, H10).

`make member-census` ne s'applique pas (R8). On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements. On ne lance jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si une aide avorte dans un rejeu ou dans un TU hors des fils de `must_abort`/`must_abort_value` (H2), si un indicateur autre que `has_kh`/`has_m_` est ajouté, si l'écrivain accepte un autre nom que `m_`, si la boucle accepte une autre clé que `ke`/`jb`, ou si un type est élargi (union, `void *`, `ke` non `const` ou en pointeur d'en-tête) au lieu d'avorter.
- `FAILED:` si les types ne sont pas exactement, dans cet ordre après `pico8_ke_t ke_base` : `pico8_qz_box_t *kq`, `pico8_fixed_t fa`, `pico8_vec_t *ra`, `pico8_vec_t *eh`, `const pico8_entity_kind_t *ke`, `pico8_jb_t *jb`, `int kh`, `int has_kh`, `int m_`, `int has_m_`.
- `FAILED:` si un défaut change d'identité : `kq`, `ra`, `eh` d'un `ie` neuf ne sont pas les pointeurs de `g_ie_spec`, partagés.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur un membre nil ou un nom inconnu, rend `kh` ou `m_` « présent `false` » quand ils sont nil, ne rend pas le même pointeur, ou si `gy`/`id`/`me`/`t` changent de chemin.
- `FAILED:` si `ie_take_bool` ne pose pas `*out = 0; *has = 0;` **avant** le test de nil, si ce test ne vérifie pas le genre **et** le pointeur, si `*has = 1` n'est pas la dernière instruction, ou si le message ne nomme pas `ie.m_` et `bool`.
- `FAILED:` si `ie_take_kind`/`ie_take_vec` ne vérifient pas le tag `UNKNOWN`, si `ie_take_jb` ne vérifie pas le tag `STRUCT` **et** `cls == pico8_jb_struct_class()`, ou si leurs messages ne nomment pas `ie.ke`/`ie.ra` et `ref:autre`, `ie.jb` et `ref:pico8_jb_t`.
- `FAILED:` si l'`assert` des clés (« ie: the only measured construction keys are ke and jb (eu)… ») ou son message change, si un `assert` de `pico8_ie_spec_ref`/`hb`/`ldown` (« pico8_research_icon_init() must run before… ») change, ou si le message de l'`assert(written && …)` de `pico8_entity_set_field` change.
- `FAILED:` si `hb`, `kj` ou `ldown` changent autrement que par R6 (ordre des instructions, `malloc` du curseur, `ie_value_vec`, `ie_to_int`, TROU-7 gardés), si `g_ie_gy`/`g_ie_id` changent de type, ou si la signature d'une fonction publique d'`ie` change.
- `FAILED:` si la vue `ie_ke` (typedef, lecteur, aides, constructeur de vue) change, ou si `ie_self_ke_kind` rend autre chose que `self->ke` (NULL pour `self` NULL).
- `FAILED:` si `layout.[ch]` changent autrement que par l'ajout de `pico8_jb_struct_class`, ou si un site par nom change (`dc`, `gk`, `gw`, les tests), ou si `entity.c`, `entity_geom.c`, `ui.c`, `cart.c` changent.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si un extrait C1 de `tools/float_audit_c1.txt`, ou ce fichier, changent.
- `FAILED:` si `test_pico8_ie.c` a un diff ; si le diff de `test_pico8_entity_kind.c` fait autre chose qu'ajouter la caractérisation, le test d'arrêt, leurs utilitaires, `IE_MEMBERS_HASH`, les `#include`/`#define` nommés et leurs appels en fin de `main` ; si une valeur figée existante change (H5) ; ou si un test de régression (§6) change.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 177, liste encore `pico8_ie_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` s'il manque une ligne « Runs » de `make class-coverage-net` identique à la référence, ou si le diff touche `pico8_class_coverage_record` ou `KNOWN[]`.
- `FAILED:` s'il manque la ligne « Runs » de `make e2e` montrant `test_pico8_ie` vert.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit ou écrit un membre directement, ou si `IE_MEMBERS_HASH` change après le typage ; si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'ancien code et une verte après, ou s'il ne porte pas les cas à union nulle construits par `memset` (`zero_fixed`, `zero_string`, `zero_function` sur `m_` ; `zero_fixed`, `zero_false`, `zero_string`, `zero_function` sur les clés `ke` et `jb` ; `zero_fixed`, `zero_false` sur `gy`).
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 13 mutations, chacune avec son test ou la justification de son trou, plus les mutants équivalents et le trou `ref:autre` de `ke`.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.20 », ou le tableau « Runs ».
- `FAILED:` si le budget d'environ 20 minutes est dépassé sans que le journal note ce qui n'a pas été lancé.

## Out of Scope

- `ie.gy` et `ie.id` (`g_ie_gy`, `g_ie_id`) : état module, pas des membres ; `pico8_ie_set_gy`/`pico8_ie_eh` gardent leur signature `pico8_value_t`.
- La vue `ie_ke` (1b.11) et `pico8_dl`.
- Les homonymes `kq`, `fa`, `ra`, `m_`, `jb` de `jo`, `js`, `fv`, `g_`, `bg`… (étapes suivantes).
- Typer `pico8_entity_kind_t` ou distinguer un genre d'un vecteur par un tag.
- Remplacer une lecture ou écriture par nom (`dc`, `gk`, `gw`) par un accès direct.
- La durée de vie de `g_ie_spec` après `pico8_research_icon_shutdown` (une icône vivante garde un pointeur libéré, déjà vrai aujourd'hui, non atteint par le filet).
- Une aide de déballage partagée entre fichiers ; retoucher les aides de 1b.5 à 1b.19.
- Régénérer `tools/member_census.txt` ; ajouter `test_pico8_ie` au recensement.
- Les fichiers d'événements et `FRAMES_EVENTS` ; tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `research_icon.[ch]`, plus l'accesseur `pico8_jb_struct_class` dans `layout.[ch]` ; un commit, sans découpage ;
  - tests : caractérisation et test d'arrêt dans `test_pico8_entity_kind` (rapide, au recensement), pas dans `test_pico8_ie` (lent, diff vide) ; pas de nouveau TU, pas de changement du Makefile ;
  - `ke` → `const pico8_entity_kind_t *` (descripteur `static const`, pas une entité ; écart assumé avec « non `const` » de R1, dont `ref:autre` est exclu) ; `jb` → `pico8_jb_t *` (→ 1b.2) ; `kq` → `pico8_qz_box_t *` ;
  - `has_kh` et `has_m_` sur preuve `lu` du TU lent `test_pico8_ie` (règle R2 de 1b.11), contre la lettre de leurs lignes (`-`, `e`) ;
  - R5 écrivain de 1b.15 sans écart sur `m_` ; clés `ke`/`jb` par la boucle directe de 1b.10 avec l'`assert` des clés gardé à sa place ; R6 de `ra` par `ie_take_vec` (forme 1b.18) ; `kh` en R6 direct (littéral) ;
  - les deux `assert` de lecture sur `ke`/`jb` déplacés à l'écriture ; `ie_truthy` supprimé ;
  - R7 `offsetof` de 1b.9 sur la table entière ;
  - mutation §7-2 sans objet dans sa lettre, jouée sous sa forme la plus proche ; §7-3 rendue déterministe par `calloc` ; `class-coverage-net` lancé.
- **Note « Découvert à 1b.20 »** proposée, sur la ligne 1b.20 de `docs/plan-3.md` : « patron 1b.1 appliqué ; R7 forme `offsetof` de 1b.9 sur la table entière (`members[]` supprimée, `gy`/`id` gardés en tête du lecteur) ; **R1 `ref:autre` vers un descripteur de genre : `const pico8_entity_kind_t *`** (objets `static const`, seul cast à l'emballage R4), vers une boîte : `pico8_qz_box_t *` ; `jb` → `pico8_jb_t *`, accesseur public **`pico8_jb_struct_class()` ajouté** à `layout` (sert à 1b.34) ; **R2 de 1b.11 appliquée** : `has_kh`, `has_m_` sur lecture nil par nom de `test_pico8_ie` (lignes `-` et `e`) ; R5 écrivain 1b.15 sans écart (`ie_take_bool`) ; clés par aides (`ie_take_kind`, `ie_take_jb`) : les `assert` de lecture sur `ke`/`jb` passent à l'écriture ; R6 de `ra` par `ie_take_vec` depuis `ie.gy` (1b.18) ; limite `ref:autre` : un vecteur passe pour un genre (tag `UNKNOWN`) ; empreinte réelle 8 (5, 3), 5 fichiers modifiés. » Le reste va dans le commit.
- Le message de commit porte le périmètre, le compte 185 → 177, le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous, `IE_MEMBERS_HASH` et les réponses à H1 à H18.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
