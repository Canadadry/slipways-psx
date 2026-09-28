---
title: "Plan 3, étape 1b.32 : `g_`, la sonde. Ses 23 membres typés, `et` en pointeur d'en-tête plus `has_et` (false = présent et NULL), `p_` en booléen plus `has_p_` ; `g_g__members`, `g_g__defaults` et les `keys[]` de construction supprimées ; la fille `fv` perd ses 13 entrées de `g_`"
description: "Après 1b.31, `pico8_g__t` garde 23 membres `pico8_value_t` : 18 défauts de spec lus par nom et 5 membres écrits par nom (`qy` par `f_`, `ra`/`p`/`et` par `bv`, `p_` par `d_`). 1b.32 les type selon R1–R9 de 1b.1. `et` ({false seul, REF à plusieurs cibles}) reprend la lecture de 1b.24 ; l'écrivain prend la forme de 1b.13/1b.15 ; les clés `pn`/`du`/`py` passent par `g__take_fixed` (1b.14b) ; la fille `fv` déballe ses 13 défauts de membres de `g_` en `static` (R7 de 1b.14a). `make value-members` passe de 95 à 72."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.32 — `g_` (`probe.h:25`), `et` genres mêlés (bool ou REF vers `jo`), 5 membres écrits par nom. Empreinte 39 (14, 25). Compte : 72.

Cadre : puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », §3 (`make test` vert, aucun `*.sha256` modifié), §6, §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Modèles : le PRD de 1b.24 (`plan3-phase1b-etape24-qh-laboratoire.md`, même `et`, même `bv`) et celui de 1b.30 (`plan3-phase1b-etape30-nr-porte-de-glisse.md`, forme la plus récente). Notes « Découvert à » **citées, pas redérivées** ; elles lient ce PRD :

- **1b.1** : patron R1–R9 ; s'en écarter est une lecture nouvelle, notée.
- **1b.4b** : `g_.et` gagne `ref:pico8_bv_et_t` et `ref:pico8_jc_t` (`jc` est typé après, en 1b.35 : la cible est un pointeur d'en-tête, sans effet d'ordre) ; les boucles sentinelles `written[]` de `g_` et `fv` sont arrêtées par `PICO8_MEMBER_CENSUS_STOP()`.
- **1b.7** : R2 — un nil `stat?` venu d'un homonyme ne donne pas d'indicateur quand toute écriture sur le typedef est prouvée non nil (`od`, `qy`) ; {bool, X} dont le seul booléen écrit est `false` → pointeur + `has_`, NULL présent = `false`, emballage public unique `pico8_X_<m>`.
- **1b.9/1b.10** : R7 forme `offsetof` — une table vidée est supprimée avec son seul client (`g_g__members`, `g_g__defaults`) ; boucle de clés directe, une branche par clé, une aide `static` par genre.
- **1b.11** : R2 des TU lents — relus plus bas.
- **1b.13** (forme écrivain de R5, que 1b.15 recopie) : une branche par membre `e`, dans l'ordre du typedef ; aide `static` par genre au nom de la struct, forme par valeur, partagée écrivain/boucle de clés d'un même fichier, **jamais entre fichiers** ; l'aide pose **d'abord** l'état nil, puis sort sur `REF NULL` (genre **et** pointeur testés), sinon `assert` « champ sans règle » qui nomme `struct.membre`, le genre de la ligne et l'arrêt ; un non-pointeur sans indicateur n'a pas d'état nil, nil y avorte ; nom sans branche → `return 0`, l'arrêt reste l'`assert(written && …)` de `pico8_entity_set_field` ; **test d'arrêt** rouge d'abord, avec les cas non-REF à union nulle.
- **1b.14a** : R7 sur une table de défauts qui garde des entrées et a des filles — l'entrée du membre typé sort de toutes les tables (ici `g_fv_defaults`) ; le module de la fille déballe **une fois, à son init**, après la boucle de sa table, son défaut dans `static g_fv_<m>` (même pointeur pour un pointeur), remis à nil dans `pico8_fv_defaults_reset` ; pose typée dans le constructeur après la boucle, avant les clés et tout `pq` ; une fille sans défaut propre n'écrit rien.
- **1b.14b** : `g__take_fixed` (clé `t`, non-pointeur sans indicateur, nil avorte) **« que 1b.32 étend à `pn`/`du`/`py` »** ; cas « union nulle » construits par `memset` puis `kind` ; une mutation qui retire l'unique appel d'une aide `static` se plante sous `if (0)` (`-Werror`).
- **1b.15** : aide à indicateur (`bg_take_bool`) — nil éteint `has_` quel que soit l'état d'avant, `false` à union nulle = `false` présent, `has_ = 1` posé en dernier.
- **1b.19** : fixe sans indicateur écrit par nom → nil avorte.
- **1b.22** : REF à plusieurs cibles → `pico8_struct_header_t *` ; ensemble admis = **ligne ∪ producteurs prouvés par le code et exercés par la caractérisation dirigée sur l'ancien code** ; prédicat par accesseurs publics liés partout où le fichier l'est, ou par une propriété du descripteur que l'ensemble partage exactement.
- **1b.23a1** : quand un fichier déballe plus de trois défauts, une aide `static` de lecture de spec par genre et par fichier, message aux noms de ses membres ; valeurs figées = valeurs **observées**.
- **1b.23a2** : un membre typé sort des `written[]` des TU, son écriture par nom est figée par la caractérisation (fonction distincte, hash propre).
- **1b.23b** : le grep §6 couvre `(cast)x->m`, pas seulement `g_\.m` ; un test de régression qui change dans la seule forme de son écriture porte sa preuve rouge.
- **1b.23d** : un `written[]` vidé est supprimé avec sa boucle.
- **1b.24** : **{`false` seul, `ref:` à plusieurs cibles} → `pico8_struct_header_t *<m>` + `int has_<m>`, NULL présent = `false`, `true` avorte ; « `g_.et` et `mz.et` la relisent sur leur preuve »** ; prédicat `methods[LY] != NULL` ∪ `jo` ∪ `jc` ∪ `bv_et` ; `od` sans indicateur malgré `stat?` ; le crochet `STOP` d'une boucle supprimée est gardé seul ; preuve de typedef par `awk`.
- **1b.29** : `keys[]` vidée, supprimée : clés typées en branches, `assert(0 && …)` final au message inchangé ; une clé nommée à `REF NULL` est gardée par `pico8_ob_table_set_name` (H11 de 1b.29) ; `probe_mm_rnd_order` prouvé rouge par 3175ebd.
- **1b.30/1b.31** : caractérisation dans le TU rapide de la classe ; **1b.31** : un nil écrit **directement** par un test après `STOP`, que le cart ne produit jamais, n'est pas une preuve de nil ; la lecture devient « présent 0 ».

### État de départ

HEAD `2b366fc` (PRD de 1b.31). **Précondition : 1b.31 (`fr.mt`) est commitée avant que 1b.32 commence (H1)** ; `make value-members` rend alors **95**. 1b.31 ne touche que `src/pico8_planet_family_trade.[ch]`, `tests/test_pico8_fr.c` et le plan (H2) : aucun fichier de cette étape. Numéros de ligne indicatifs (arbre `2b366fc`), chaque site est aussi désigné par son contenu. **Rien n'a été compilé ni lancé pour ce PRD** : les faits viennent de `grep` et de la lecture ; tout fait d'exécution est une hypothèse (H1–H22).

### Lignes de recensement (`tools/member_census.txt:132-154`)

| membre | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|
| `nu` | `ref:table` | jamais | - | l | `pico8_ob_table_t *` (spec de `g_`) | non |
| `fx` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `dq` | `chaine` | jamais | - | l | `const char *` (spec) | non |
| `od` | `fixe` | observé | stat? | l | `pico8_fixed_t` | **non** (R2 de 1b.7/1b.24) |
| `fp` | `bool` | jamais | - | l | `int` (0/1) | non |
| `fa`, `dw`, `py`, `ki`, `pn`, `pk`, `fc` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `k_` | `ref:table` | jamais | - | l | `pico8_ob_table_t *` (spec) | non |
| `qo`, `ro`, `qg`, `q_`, `du` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `qy` | `ref:liste` | observé | stat? | **le** | `pico8_list_t *` | non (pointeur) |
| `ra` | `ref:autre` | observé | lu,stat? | **le** | `pico8_vec_t *` (emprunté) | non (pointeur) |
| `p` | `ref:autre` | observé | lu | **le** | `pico8_vec_t *` (tas de `bv`, TROU-7) | non (pointeur) |
| `et` | `bool`, `ref:pico8_bv_et_t`, `ref:pico8_jc_t`, `ref:pico8_jo_t` | observé | lu,stat? | **le** | `pico8_struct_header_t *` | **`int has_et`** |
| `p_` | `bool` | observé | lu | **le** | `int` (0/1) | **`int has_p_`** |

Compte : 95 − 23 = **72**. Aucune ligne `pico8_g__t` ne reste ; les 10 lignes `pico8_fv_t` (1b.33) restent.

### Le cart (`pico8/lua.lua`)

- `g_=ke:jr` (`:708-721`) : `nu=o("g_","prober")`, `fx=1`, `dq="ly"`, `od=0`, `fp=f`, `fa=2`, `dw=0.025`, `py=0.06`, `ki=9`, `pn=3`, `pk=1`, `fc=1`, `k_=o(1,1,5,13,6)`, `qo=1.02`, `ro=-0.0525`, `qg=0.0525`, `q_=1.02`, `du=2` ; `g_.hq=bv`, `ey=d_`, `ev=dl`, `pq=f_`, `kj=ei`, `g_:mm` (`:722-731`).
- `f_` (`:685`) : `self.qy={}`. **`bv`** (`:552-558`) : `self.ra,self.p=mp,mp/128` ; `self.et = nq<self.dw and c or jp(cg.jc,…)[1] or not ui.hw(rp) and {}` ; `if (self.et) self.et.fo=self`. Le dernier terme vaut `{}` hors du panneau, **`false`** dedans ; **`true` n'est jamais écrit**, nil non plus (nil tenu avant le premier `hq`). `c` parcourt `cg.np` : le tag `np` est dans le `nu` de `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr` (`fr` hérite de `nr`, `ga` n'y est pas) — mêmes 9 classes que pour `qh.et` (1b.24).
- `dl` (`:559-566`) : `if self.et` … `elseif self.od`. `d_` (`:567-575`) : `if not self.et and fm:lu(self.od)` → `ks(self.dq or "ph")`, `fm:nx(self.fp or 0)` ; sinon `self.p_=true`. **`p_` : seul `true` est écrit.**
- `fv=g_:jr` (`:1143-1166`) : 22 défauts, dont **13 qui recouvrent des membres de `g_`** : `fa=10`, `ra=v(64,28)`, `fc=5`, `k_=o(0,1,…)`, `qo`, `ro`, `qg`, `q_`, `pn=5`, `du=15`, `py=0.0067`, `pk=1`, `ki=17`. `fv` hérite `nu`, `fx`, `dq`, `od`, `fp`, `dw` de `g_`.

### Le code

**`src/pico8_probe.h`** : typedef `pico8_g__t` (`:25-50`), `ke` puis 23 `pico8_value_t` ; commentaire TROU-7 au-dessus (« `nu` and `k_` are the tables of `g_`'s class … `ra` is the very REF `bv` was passed, aliased ») qui reste vrai.

**`src/pico8_probe.c`** :

- `g_g__defaults[]` (`:451-464`, 18 entrées `offsetof`) et ses 3 clients : `pico8_g__defaults_reset`, la boucle de recopie de `pico8_g__struct_init`, la boucle de lecture + `assert` « g_: every default … » de `pico8_probe_init`.
- `g_g__members[]` (`:473-485`, 23 entrées) et son seul client, le lecteur `pico8_g__field` (REF NULL → 0, repli `pico8_ke_field`).
- Écrivain `pico8_g__set_field` (`:504-521`) : `qy`, `ra`, `p`, `et`, `p_` recopiés **quel que soit le genre**.
- `pico8_g__struct_init` (`:535-547`) : 5 membres écrits à `REF NULL`, puis la recopie des 18 défauts.
- `pico8_g__struct_new` (`:556-597`) : `keys[] = {pn, du, py}` à `offsetof` (recopie de **tout** genre), branche `t` par `g__take_fixed`, `assert(found < …)` « g_: the only measured construction keys are pn, t, du, py … ».
- `g__take_fixed` (`:549-554`) : message « g_.ke.t: … ».
- Sites **par nom**, inchangés : `pico8_bv` (`ra`, `p`, `dw`, `et` ; écriture de `fo` sur la cible), `pico8_dl` (`me`, `et`, `od`), `pico8_d_` (`et`, `od`, `dq`, `fp`, `ik`, écriture de `p_`), `pico8_g__mm` (`pn`, `t`, `du`, `py`, `ki`, `pk` — `self` peut aussi être un `hj`, `hj.mm = g_.mm`). Aucune lecture directe d'un membre de `g_` hors du lecteur, de l'écrivain et de l'init.
- Inclusions : ni `pico8_planet.h` ni `pico8_slipway.h` aujourd'hui.

**`src/pico8_particle.c`** : `f_` écrit `qy` par nom (liste neuve, tag `LIST`) ; `ez`/`ei` lisent `qy`, `fa`, `ra`, `qo`, `qg`, `ro`, `q_`, `k_`, `fc` par nom. **Inchangé.**

**`src/pico8_help.c`** (la fille `fv`) :

- `g_fv_defaults[]` (`:117-132`, 22 entrées) : les 13 ci-dessus en `offsetof(pico8_fv_t, g_.<m>)`, puis 9 propres (`hi`, `ca`, `gq`, `eo`, `ff`, `kq`, `ql`, `fg`, `rb`) ; recopiée dans `pico8_fv_struct_new` après `pico8_g__struct_init` et `m_ = REF NULL` ; lue par la boucle de `pico8_help_init` (`assert` « fv: every default … ») ; remise à nil par `pico8_fv_defaults_reset`.
- `g_fv_members[]` (10 membres propres de `fv`), lecteur `pico8_fv_field` (repli sur celui de `g_`), écrivain `pico8_fv_set_field` (`m_`, puis délégation à celui de `g_`) : **non touchés** (1b.33).
- `offsetof(pico8_fv_t` : 32 occurrences, dont 13 `g_.` ; `offsetof(pico8_g__t` dans `probe.c` : 44 (18 + 23 + 3).

**Tests, accès directs aux membres de `g_`** (grep `(->|\.)(m)\b` sur les fichiers qui nomment `pico8_g__t`/`pico8_fv_t`) :

- `tests/test_pico8_g_.c:889` : `((pico8_g__t *)roads[1])->fa = pico8_value_ref(NULL);` puis `assert(!cls->field(roads[1], "fa", &v) && !is_probe(roads[1]));`, après `PICO8_MEMBER_CENSUS_STOP()` — un nil fabriqué par le test, sur un fixe `jamais` nil.
- `tests/test_pico8_jc.c:874` (`test_bv`) : `((pico8_g__t *)probe)->dw = pico8_value_fixed(0);`.
- `tests/test_pico8_probe_mm_rnd_order.c` : nomme `pico8_g__t *` sans accéder à un membre.

**Boucles sentinelles** : `test_pico8_g_.c:840,884` `written[] = {"qy","ra","p","et","p_"}` ; `test_pico8_fv.c:574,611` `written[] = {"m_","qy","ra","p","et","p_"}` — toutes deux écrivent `pico8_value_fixed(i + 1)` et relisent `FIXED`.

### Revue des TU lents (R2 de 1b.11)

`grep -nE 'pico8_g__|pico8_fv_|is_probe|is_help|cg\("g_"\)|cg\("prober"\)'` sur `test_pico8_{ie,jo,qh,os,hj,pm,mx,nr}.c` ne rend que `test_pico8_hj.c:396,758` (comparaison de méthode à `pico8_g__mm`). Leurs lectures par nom visent leurs propres classes, filtrées par descripteur (`daughter_of`) ou par un membre absent de `g_` (`fz` de `is_ga`). **Aucune lecture nil par nom d'un membre de `g_` sur un `g_` ou un `fv` : aucun indicateur de plus** que ceux que la ligne donne (`has_et`, `has_p_`) (H17).

La lecture nil de `fa` de `test_g__is_a_struct` est un nil écrit directement par le test après `STOP` : **pas une preuve** (lecture de 1b.31), réécrite en « présent 0 ».

## Solution

1. **Caractérisation figée sur l'ancien code** (arbre 1b.31 commité), dans le TU rapide `test_pico8_g_`, par nom seul ; verte avant tout typage.
2. **Test d'arrêt rouge d'abord** (écrivain et clés) dans le même TU.
3. **Commit 1b.32** : les 23 membres typés, avec `has_et` et `has_p_` ; emballage public `pico8_g__et` ; `g_g__members`, `g_g__defaults`, `keys[]` supprimées avec leurs boucles ; 18 `static g_g__<m>` ; écrivain à 5 branches et 5 aides ; prédicat `g__is_target_class` ; `g__take_fixed` étendu à `pn`/`du`/`py` ; R7 de 1b.14a sur `fv` (13 entrées retirées, 13 `static g_fv_<m>`) ; `written[]` de `test_pico8_g_` supprimé (crochet `STOP` gardé seul), celui de `test_pico8_fv` réduit à `{"m_"}` ; deux accès directs des TU réécrits ; `make value-members` rend **72**.
4. Aucun pixel ne bouge. Note « Découvert à 1b.32 » sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `probe.h`, je veux `pico8_g__t` sans `pico8_value_t`, chaque membre au type de sa ligne, `has_et` juste après `et`, `has_p_` juste après `p_`.
2. En tant que lecteur par nom (`probe.c`, `particle.c`, `ui.c`, les TU), je veux que chacun des 23 noms rende sur un `g_` et sur un `fv` exactement ce que rendait l'ancien code : présence, genre, valeur, identité du pointeur.
3. En tant que sonde neuve (trois routes : `bg:ldown`, `pico8_g__new(NULL)`, clés), je veux les 18 défauts de la spec de `g_`, `nu`/`k_`/`dq` au **même pointeur** que la spec, `qy` liste vide (après `f_`), `ra`/`p`/`et`/`p_` absents.
4. En tant que `fv` neuf, je veux mes 13 défauts de membres de `g_` pris dans **ma** spec (`fa` 10, `ra` = le vecteur `v(64,28)` de ma spec au même pointeur, `k_` = ma table au même pointeur…), et `nu`/`fx`/`dq`/`od`/`fp`/`dw` hérités de `g_`.
5. En tant que `bv`, je veux écrire `ra` = le pointeur de `mp`, `p` = mon vecteur alloué, `et` = la planète visée (l'une des 9 classes de `cg.np`), le `jc` visé, la cible jetable ou `false`, et les relire au même pointeur ou `BOOL` 0 présent.
6. En tant que `d_`/`dl`, je veux lire `et`, `od`, `dq`, `fp` par nom comme avant, et écrire `p_` = `true` relu `BOOL` 1.
7. En tant que `f_`/`ez`/`ei`/`g_:mm`, je veux `qy` écrit et relu au même pointeur, et `pn`/`t`/`du`/`py`/`ki`/`pk` lus comme avant.
8. En tant que constructeur à clés (`probe_mm_rnd_order`), je veux `pn`, `t`, `du`, `py` posés en fixe, et toute autre clé, toute clé positionnelle, et toute valeur non fixe (nil compris) en arrêt.
9. En tant que gardien d'`et`, je veux refuser `true`, tout non-struct et toute struct hors des 11 classes admises (`ga`, `g_`, `fv`, `mz`, `bg`, `ok`, un `hp`, `mz_jj`), avec un arrêt « champ sans règle » qui nomme `g_.et`.
10. En tant que gardien de `qy`, `ra`, `p`, `p_`, je veux que tout genre hors ligne avorte, union nulle comprise.
11. En tant que mainteneur, je veux qu'aucune table d'offsets ne liste plus un membre de `g_` (`probe.c` sans `offsetof`, `help.c` sans `offsetof(pico8_fv_t, g_.`).
12. En tant qu'auteur de 1b.33 (`fv`), je veux `g_fv_defaults` réduite à mes 9 entrées propres avec sa boucle et sa remise à nil, `g_fv_members` intacte, `written[]` de `test_pico8_fv` réduit à `{"m_"}`, et des aides de lecture de spec de `help.c` réutilisables.
13. En tant que test de régression `probe_mm_rnd_order`, `playability`, `geoharvest_burst`, `slipway_drag`, je veux rester vert sans diff, et rouge quand mon bug revient.
14. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
15. En tant que `plan-pilot`, je veux `make value-members` à 72 sans ligne `pico8_g__t`.
16. En tant que vérificateur, je veux la caractérisation verte sur l'ancien code, le test d'arrêt rouge puis vert, les preuves rouges §6, les mutations et leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle : les validations que demande `to-prd` (modules, tests) sont tranchées comme le ferait un agent `guideline` — hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables — et marquées (G).

### `et` : `pico8_struct_header_t *et` + `int has_et`, lecture de 1b.24 relue sur la preuve de `g_` (G)

États : nil (tenu avant le premier `hq`, lu par nom : `lu`, `not_yet[]` de `test_pico8_g_`, `absent[]` de `test_pico8_fv`) ; `false` (écrit par `bv` dans le panneau, lu `BOOL` 0 présent : `et_kind` = 1 de `test_pico8_g_`, étape 1 de `playability`) ; une REF. **`true` n'est jamais écrit** : cart (`not ui.hw(rp) and {}`), C (seul `pico8_value_bool(0)` dans `pico8_bv`), TU (la seule écriture par nom d'`et` sur un `g_`/`fv` est la boucle sentinelle, qui écrit des fixes et disparaît) (H3). La preuve de 1b.24 vaut donc pour `g_` : même `bv`, même `cg.np`. **Pas une lecture nouvelle** : lecture de 1b.24 appliquée sans écart.

- `has_et` = 0 : nil (`et` NULL, non lu) ; `has_et` = 1 et `et` NULL : `false` ; sinon la cible, empruntée, jamais possédée ni libérée.
- **Emballage public unique** `pico8_value_t pico8_g__et(const pico8_g__t *self)` déclaré dans `probe.h` (commentaire de contrat : REF NULL pour `self` NULL ou nil, `BOOL` 0 pour présent-NULL, REF sinon), forme exacte de `pico8_qh_et`, appelé par le lecteur. Aucune lecture directe d'`et` dans `src/`.
- Rejetées (1b.24) : NULL = `false` sans indicateur (nil et `false` se confondraient, lus par nom) ; `et_is_bool` (vaudrait `has_et && et == NULL`) ; union, `void *`.

### Ensemble admis d'`et` : 11 classes, prédicat `g__is_target_class` (G)

- **Ligne** : `jo`, `jc`, `bv_et` (et `false`).
- **∪ producteurs prouvés par le code** : `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx` — dans `cg.np` (leur `nu`, ou celui de `nr` pour `fr`), donc `c` de `ij(self.p, cg.np)` peut les rendre ; aucun rejeu recensé n'a posé de sonde dessus. **Tous les 8 exercés par la caractérisation sur l'ancien code** (`hq` d'une sonde neuve visant une planète neuve de chaque classe), lecture 1b.22.
- **Refusés et figés par le test d'arrêt** : `ga` (pas de `np`), `g_`, `fv`, `mz`, `bg`, `ok`, un `hp`, `mz_jj`, tout non-struct.
- **Prédicat**, copie de la règle de `family_is_target_class` (1b.24), `static` dans `probe.c` (1b.13 point 2 : jamais partagé entre fichiers) :

```c
static int g__is_target_class(const pico8_struct_class_t *cls) {
    return cls->methods[PICO8_METHOD_LY] != NULL || cls == pico8_jo_struct_class() ||
           cls == pico8_jc_struct_class() || cls == pico8_bv_et_struct_class();
}
```

- **Liens** (fait, grep du `Makefile`) : `probe.c` n'est lié que par `ENGINE_SRCS`, `DATA_ENGINE_SRCS` et la cible `main_raylib`, qui lient tous trois `planet.c`, `planet_family.c`, `slipway.c` (H7 pour la compilation). **Deux `#include` ajoutés à `probe.c`** : `pico8_planet.h` (`pico8_jo_struct_class`, `:82`) et `pico8_slipway.h` (`pico8_jc_struct_class`, `:49`).
- `grep -rn '\[PICO8_METHOD_LY\] = pico8_' src/*.c` : 8 lignes, avant comme après (H6).

### Aides R5 (forme 1b.13/1b.15), toutes `static` dans `probe.c`, au nom de la struct (G)

| aide | membres | état nil puis sortie | accepté (`assert`) | message (préfixe) |
|---|---|---|---|---|
| `g__take_list(value, pico8_list_t **out)` | `qy` | `*out = NULL` | REF non NULL, tag `LIST` (forme `family_take_list`) | `"g_.qy: a write neither nil nor a list is off its census row (ref:liste) -- …"` |
| `g__take_vec(value, pico8_vec_t **out)` | `ra`, `p` | `*out = NULL` | REF non NULL, tag `UNKNOWN` (lecture `ref:autre` de 1b.10, forme `family_take_vec`) | `"g_.ra/p: a write neither nil nor a vector ref is off its census row (ref:autre) -- …"` |
| `g__take_target(value, pico8_struct_header_t **out, int *has)` | `et` | `*out = NULL; *has = 0` | `BOOL` 0, ou REF tag `STRUCT` **puis** `g__is_target_class(cls)` ; `*out = REF ? ref : NULL`, `*has = 1` en dernier (forme `family_take_target`) | `"g_.et: a write neither nil, false, nor what bv aims at (a planet jo, qh, nl, os, hj, pm, nr, fr, mx, a jc, or bv's throwaway target) is off its census row (bool,ref: to those targets) -- …"` |
| `g__take_bool(value, int *out, int *has)` | `p_` | `*out = 0; *has = 0` | `BOOL` ; `*out = boolean != 0`, `*has = 1` en dernier (forme `jo_take_bool`/`bg_take_bool`) | `"g_.p_: a write neither nil nor a boolean is off its census row (bool) -- …"` |
| `g__take_fixed(value, pico8_fixed_t *out)` (existante) | clés `t`, `pn`, `du`, `py` | **aucun** (nil avorte, 1b.14b/1b.19) | `FIXED` | message élargi : `"g_.ke.t/pn/du/py: a construction key that is not a fixed is off its census row (fixe, never nil) -- …"` |

Chaque message finit par `"a field under no rule of plan-3 section 5: stop, do not widen the member"`. Un `false` à union nulle est un `false` présent pour `et` et `p_` (témoin accepté) ; `true` avorte pour `et`, pas pour `p_`.

**Écrivain `pico8_g__set_field`** : 5 branches dans l'ordre du typedef, `if (strcmp(name, "qy") == 0) { g__take_list(value, &g_->qy); return 1; }`, puis `ra`, `p` (`g__take_vec`), `et` (`g__take_target(value, &g_->et, &g_->has_et)`), `p_` (`g__take_bool(value, &g_->p_, &g_->has_p_)`), puis `return 0`. L'ensemble des 5 noms ne change pas ; `fv` y délègue toujours après `m_`.

**Boucle de clés de `pico8_g__struct_new`** (lecture 1b.29) : `keys[]` vidée → supprimée avec sa boucle interne et `size_t i` ; une branche par clé nommée (`t` → `&self->ke.t`, `pn`, `du`, `py` → `g__take_fixed`) ; clé inconnue ou positionnelle → `else { assert(0 && "g_: the only measured construction keys are pn, t, du, py (probe_mm_rnd_order); another would be a field under no rule of plan-3 section 5 -- stop, do not add a member"); }`, **texte inchangé**.

### R1–R9 appliquées

| règle | sur `g_` |
|---|---|
| R1 | Tableau de recensement ; nom et place gardés ; `has_et` juste après `et`, `has_p_` juste après `p_`, commentaires de contrat courts (forme `planet_family.h:45`). |
| R2 | `has_et`, `has_p_` seuls (non-pointeurs ou mêlés, lus par nom à nil `lu`). `od` sans indicateur (`stat?` d'homonyme, jamais écrit sur un `g_`, 1b.7/1b.24) ; `fp` bool `jamais` sans indicateur ; pointeurs sans indicateur. |
| R3 | `pico8_g__struct_init` : après `pico8_ke_struct_init`, `qy = ra = p = et = NULL`, `has_et = 0`, `p_ = 0`, `has_p_ = 0`, puis les 18 poses `self-><m> = g_g__<m>` dans l'ordre du typedef ; remplace les 5 `REF NULL` et la boucle ; `size_t i` retiré. |
| R4 | `pico8_g__field` : 23 branches `strcmp` dans l'ordre du typedef, puis `return pico8_ke_field(self, name, out_value);`. Fixes : toujours `FIXED`. `fp` : `pico8_value_bool(fp)`, toujours présent. `nu`, `k_` (REF), `dq` (`STRING`), `qy` (REF liste), `ra`, `p` (REF vecteur) : NULL → `return 0`. `et` par `pico8_g__et` (REF NULL → `return 0`). `p_` : `has_p_` = 0 → `return 0`, sinon `pico8_value_bool(p_)`. `*out_value` intact sur 0. |
| R5 | Écrivain et clés : ci-dessus. |
| R6 | Aucun accès direct dans `src/` hors R3/R4/R5 (sites par nom de `bv`, `dl`, `d_`, `mm`, `f_`, `ez`, `ei` inchangés). Tests : `test_pico8_jc.c:874` → `((pico8_g__t *)probe)->dw = 0;` ; `test_pico8_g_.c:889-890` → voir « Tests touchés ». |
| R7 | `g_g__members`, `g_g__defaults` et `keys[]` supprimées avec leurs boucles (plus aucun `offsetof` dans `probe.c`, `<stddef.h>` retiré s'il ne sert plus, précédent 1b.20) ; **fille `fv`, R7 de 1b.14a** : 13 entrées `g_.` retirées de `g_fv_defaults`, qui garde ses 9 entrées propres, sa boucle, son `assert` et sa remise à nil. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_g__t)` gardé sur un typedef sans `pico8_value_t` (précédents `jo`, `qh`) ; celui de `fv` gardé ; `tools/member_census.txt` non édité. |
| R9 | 95 → 72. |

### Défauts déballés (patron `static`, R7 de 1b.14a, lecture de forme de 1b.23a1) (G)

- **`probe.c`, 18 `static`** : `g_g__nu`, `g_g__k_` (`pico8_ob_table_t *`), `g_g__dq` (`const char *`), `g_g__fp` (`int`), `g_g__fx`, `g_g__od`, `g_g__fa`, `g_g__dw`, `g_g__py`, `g_g__ki`, `g_g__pn`, `g_g__pk`, `g_g__fc`, `g_g__qo`, `g_g__ro`, `g_g__qg`, `g_g__q_`, `g_g__du` (`pico8_fixed_t`). Déballés dans `pico8_probe_init` juste après le `pico8_ob(…)` de la spec, à la place de la boucle, par `found = pico8_g__default("<m>", &v); g_g__<m> = g__default_<genre>(found, v);`. Aides (plus de trois défauts → une par genre, forme `family_trade_default_*`, `static <T> g__default_<genre>(int found, pico8_value_t v)`) : `g__default_table` (`nu`, `k_`), `g__default_string` (`dq`), `g__default_bool` (`fp`), `g__default_fixed` (14 fixes) ; message aux noms, p. ex. `"g_.fx/od/fa/dw/py/ki/pn/pk/fc/qo/ro/qg/q_/du: a default of the struct is a key of g_'s spec and a fixed (fixe)"`. Remis à 0/NULL dans `pico8_g__defaults_reset` (gardée, appelée par `pico8_probe_shutdown`). **Pointeur de spec** pour `nu`, `k_`, `dq` (identité figée par `fold_probe` : `get_ref(e,"nu"/"k_") == class_ref(pico8_g__default, …)`).
- **`help.c`, 13 `static`** : `g_fv_fa`, `g_fv_fc`, `g_fv_qo`, `g_fv_ro`, `g_fv_qg`, `g_fv_q_`, `g_fv_pn`, `g_fv_du`, `g_fv_py`, `g_fv_pk`, `g_fv_ki` (`pico8_fixed_t`), `g_fv_ra` (`pico8_vec_t *`, le vecteur de la spec de `fv`), `g_fv_k_` (`pico8_ob_table_t *`, la table de la spec de `fv`). Déballés dans `pico8_help_init` **après** la boucle `g_fv_defaults` restante par `pico8_fv_default` et trois aides `help_default_fixed`, `help_default_vec`, `help_default_table` (message aux noms `fv.<m>`) ; remis à 0/NULL dans `pico8_fv_defaults_reset` ; posés dans `pico8_fv_struct_new` après la boucle, avant l'`assert` sur `overrides`, `pq`, `f_` et `lt` : `self->g_.fa = g_fv_fa; … self->g_.ra = g_fv_ra; self->g_.k_ = g_fv_k_; …`. Identité figée par `test_pico8_fv` (`tables[]` : `ra`, `k_` = `class_ref(pico8_fv_default, …)`) et `test_fv` de `test_pico8_g_` (`k_` ≠ celui de `g_`).
- Commentaire de `g_fv_defaults` (« fv's 22 spec defaults … the 13 that override g_'s land in g_'s members ») réécrit pour rester vrai (9 défauts propres ; les 13 autres en `static`).
- **Écarté (G)** : littéraux en R3 à la place des `static` (lecture 1b.14b) — la table de la fille liste ces membres, 1b.14a s'applique, et les TU comparent aux specs ; aide partagée entre `probe.c` et `help.c` (1b.13 point 2).

### Interaction avec la fille `fv` (1b.33)

- 1b.32 ne type **aucun** membre propre de `fv` : `hi`, `ca`, `gq`, `eo`, `ff`, `kq`, `ql`, `fg`, `rb`, `m_` restent `pico8_value_t` ; `g_fv_members[]`, `pico8_fv_field`, `pico8_fv_set_field` (branche `m_`) inchangés.
- Après 1b.32, `help.c` garde `offsetof(pico8_fv_t, <propre>)` seulement : 19 occurrences (9 + 10), aucune `g_.`.
- 1b.33 hérite : `g_fv_defaults` à 9 entrées (à vider et supprimer), les 13 `static g_fv_<membre de g_>` (qu'elle ne touche pas), les aides `help_default_*` (à étendre), `written[]` de `test_pico8_fv` = `{"m_"}` (à supprimer, crochet `STOP` gardé seul), la délégation de `pico8_fv_set_field` à l'écrivain typé de `g_`.
- La pose des 13 `static` de `fv` écrase dans le constructeur de `fv` les poses R3 de `g_` : ordre « parente, puis fille », comme les tables d'avant.

### Tests touchés (G)

- **`test_pico8_g_.c`** :
  - `written[]` de `test_g__is_a_struct` vidé → supprimé avec sa boucle ; `PICO8_MEMBER_CENSUS_STOP()` **gardé seul** à sa place (1b.24) ; variables encore utilisées gardées (`-Werror`) ; commentaire de tête de la fonction ajusté s'il ne dit plus vrai (« it takes exactly … » passe à la caractérisation).
  - `:889-890` (lecture de 1b.31) → `((pico8_g__t *)roads[1])->fa = 0;` puis `assert(cls->field(roads[1], "fa", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0 && !is_probe(roads[1]));` : « présent 0 », et la sonde n'est plus reconnue (`fa` ≠ 2) (H14).
  - ajouts : caractérisation, test d'arrêt, `G__MEMBERS_HASH`, aides de test.
- **`test_pico8_fv.c`** : `written[]` réduit à `{"m_"}`, rien d'autre (commentaire « it takes exactly `m_` and `g_`'s 5 written names » ajusté s'il ne dit plus vrai).
- **`test_pico8_jc.c:874`** : `->dw = pico8_value_fixed(0)` → `->dw = 0`, rien d'autre (preuve rouge §6, R-e).

### Revue des tests de régression, mécanique

**Liste** (reprise de 1b.30/1b.31) : les tests nommés au §6 (les 17 de régression) et dans les lignes 1a : `tests/test_pico8_<nom>.c` pour `built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`, `bg`, `entity_kind`, `hj`, `jc`, `jo`, `jo_spec_defaults`, `js`, `mo`, `mz`, `nl`, `os`, `pm`, `qh`, plus `tests/test_class_coverage.c`. Tous existent.

**Greps faits pour ce PRD** (`M = nu|fx|dq|od|fp|fa|dw|py|ki|pn|pk|fc|k_|qo|ro|qg|q_|du|qy|ra|p|et|p_`) :

- accès direct : `grep -nE 'pico8_(g_|fv)_t|->g_\.|(g_|fv)\)[^;]*->(M)\b'` → `probe_mm_rnd_order` (type seul, 4 lignes), **`jc:874`** ;
- par nom `"(M)"` : homonymes partout (`hj` 40, `jc` 86, `pm` 39, `mz` 33, `qh` 33…) ; sur une sonde : `probe_mm_rnd_order` (`pn`, `t`, `du`, `py` en clés, `qy`) ;
- chemin : `grep -cE 'pico8_g__|pico8_fv_|"g_"|"prober"|pico8_bv\b|probe|"hq"|help'` et en-têtes des rejeux.

| # | test | accès / chemin | diff ? | preuve rouge |
|---|---|---|---|---|
| R-a | `probe_mm_rnd_order` | clés `pn`/`t`/`du`/`py` (branches typées, `g__take_fixed` étendu), `ez` → `mm` lit `pn`/`t`/`du`/`py`/`ki`/`pk` par nom, `qy` par nom : **sur le chemin** | non | **Obligatoire.** Bug d'origine **3175ebd** remis (dans `pico8_g__mm`, les deux tirages refondus en arguments d'une même expression) → rouge ; restauré → vert. |
| R-b | `playability` | étape 1 : `bg:ldown` construit une sonde, `bv` pose `et` = `false`, `d_` la valide : **sur le chemin** | non | **Obligatoire.** Bug de référence de 1a.17 (constructeur de `jo` appelant `pico8_ke_struct_pq` deux fois) → rouge (`count <= PICO8_PLAYABILITY_MAX_PLANETS`) ; restauré → vert. |
| R-c | `geoharvest_burst` | étape 1 (frames 0-22) « places a `g_` » ; `"qy"` lu sur un `ga` (homonyme) : **sur le chemin** | non | **Obligatoire.** Bug de 1b.29 : `pico8_ga_pq` sans `pico8_ez(self, 140)` → rouge à la frame de récolte ; restauré → vert. |
| R-d | `slipway_drag` | le rejeu `slipway_drag_events.json` fait naître 2 sondes (`DRAG_PROBES` de `test_pico8_g_`) ; `"od"` du `jc` tenu (homonyme) : **sur le chemin** | non | **Obligatoire.** Bug d'origine 2d7021f (nil à la place de l'entité de `jz("gv")` dans `ui:bh`) → rouge (`oe_count(origin) == 1`) ; restauré → vert. |
| R-e | `jc` (TU de 1a.13/1a.40) | **`:874`, accès direct** `dw` de la sonde de `test_bv` | **oui**, la seule forme de l'écriture | **Obligatoire** (1b.23b). La ligne réécrite mise sous `if (0)` → `test_bv` rouge (une planète gagne : `get_ref(probe, "et") == old` faux, ou `BV_HASH`) ; restaurée → vert (H13). |
| R-f | `empty_planet_cleared` | `"ra"` lu sur des planètes (homonyme) ; le rejeu construit une sonde si son premier clic de fond passe par `bg:ldown` (H13) | non | **Dans le budget seulement.** Bug 529f355 (entrée `LL` retirée du descripteur de `jo`) → rouge ; restauré → vert. Sinon « non lancé, chemin non prouvé ». |
| — | `bg` | `bg:ldown` construit une sonde (`:397`, `:572`) ; `absent[]` ∋ `ra`, `p_`, `nu`, `fx` sur le `bg` (homonymes) | non | aucune (TU de classe, sans diff) ; vert dans `make test`. |
| — | `hj` | `pico8_g__mm` comparé comme méthode (`:396`, `:758`) | non | aucune. |
| — | les 11 autres rejeux du §6 | construisent le `fv` de l'écran titre (`pico8__init`) : pose R7 de `fv` sur le chemin, valeurs figées par `test_pico8_fv`/`test_fv` et les frames titre du filet | non | aucune ; verts sans modification (H16). |
| — | `entity_kind`, `jo`, `jo_spec_defaults`, `js`, `mo`, `mz`, `nl`, `os`, `pm`, `qh`, `planet_kj_dispatch` | homonymes seuls | non | aucune. |

### Preuves (sortie attendue sur l'arbre final)

**Sortie de `g_`** :

- `awk '/^typedef struct \{/{b=""} {b=b $0 "\n"} /\} pico8_g__t;/{printf "%s", b}' src/pico8_probe.h | grep -c pico8_value_t` : **0** ;
- `make value-members` : **72**, `grep -c pico8_g__t` : **0**, `grep -c pico8_fv_t` : **10** ;
- `grep -rnw 'g_g__members\|g_g__defaults' src/` : **vide** ; `grep -n 'offsetof' src/pico8_probe.c` : **vide** ;
- `grep -o 'offsetof(pico8_fv_t, g_\.' src/pico8_help.c | wc -l` : **0** (13 avant) ; `grep -o 'offsetof(pico8_fv_t, [a-z_.]*)' src/pico8_help.c | wc -l` : **19** (32 avant).

**Tables, aides, prédicat** :

- `grep -nE '(->|\.)(nu|fx|dq|od|fp|fa|dw|py|ki|pn|pk|fc|k_|qo|ro|qg|q_|du|qy|ra|p|et|p_)\.(kind|as)' src/pico8_probe.c src/pico8_help.c` : **vide** ;
- `grep -n 'pico8_value_t \*' src/pico8_probe.c | grep -v 'pico8_value_t \*out_value)' | grep -v 'pico8_value_t \*out)'` : **vide** (3 lignes avant, `:492`, `:545`, `:588`) ;
- `grep -c 'off its census row' src/pico8_probe.c` : **6** (2 avant : `bv_et_take_holder`, `g__take_fixed`) ;
- `grep -n 'g__is_target_class' src/pico8_probe.c` : la définition et un seul appel ; `grep -rn '\[PICO8_METHOD_LY\] = pico8_' src/*.c` : 8 lignes ;
- `git diff -U0 src/pico8_probe.c | grep '^+#include'` : `pico8_planet.h`, `pico8_slipway.h`, rien d'autre ;
- `grep -n 'has_et\|has_p_' src/pico8_probe.h` : chacun juste après son membre, plus `pico8_g__et`.

**Périmètre du diff** :

- `git diff --stat HEAD -- src/ ':!src/pico8_probe.h' ':!src/pico8_probe.c' ':!src/pico8_help.c' tools/ Makefile` : **vide** ;
- dans `tests/`, seuls `test_pico8_g_.c` (ajouts, `written[]` et sa boucle supprimés, lignes `:889-890`, commentaire), `test_pico8_fv.c` (`written[]`, commentaire) et `test_pico8_jc.c` (ligne `:874`) changent.

### Empreinte et découpage

Ligne : 39 (14, 25). **Remesurée pour ce PRD** avec la formule du plan (fichiers qui nomment `pico8_g__t`/`pico8_fv_t` et accèdent à un des 23 noms, ∪ lignes `_field`/`_truthy` portant `"m"`) : **40 (13, 27)**, majorant dû aux homonymes (`p`, `ra`, `fa`, `od`, `et`…). **Fichiers modifiés attendus : 6** — `src/pico8_probe.h`, `src/pico8_probe.c`, `src/pico8_help.c`, `tests/test_pico8_g_.c`, `tests/test_pico8_fv.c`, `tests/test_pico8_jc.c` — plus `docs/plan-3.md`. Empreinte réelle comptée au commit (H20).

**Découpage (G) : aucun, un seul commit.** Écartés : (a) défauts puis membres écrits — les deux moitiés partagent `g_g__members`, le lecteur et `pico8_g__struct_init`, et la table ne tomberait qu'au second commit ; deux `make e2e`, deux `class-coverage-net` hors budget ; (b) `g_` puis `fv` — impossible, un membre typé sort de toutes les tables à la fois (R7).

### Hypothèses (rien n'a été lancé)

- **H1** : 1b.31 commitée ; `make value-members` rend 95 au départ, 72 à la fin ; seules les 23 lignes `pico8_g__t` disparaissent.
- **H2** : 1b.31 ne modifie ni `probe.[ch]`, ni `help.c`, ni `test_pico8_g_`/`fv`/`jc`.
- **H3** : aucun site n'écrit `true` dans `g_.et` (cart, `pico8_bv`, TU). Démentie : arrêt « champ sans règle », sans élargir le membre.
- **H4** : dans les 15 rejeux et les TU hors fils de `must_abort`, `et` ne reçoit que nil, `false` ou une REF vers l'une des 11 classes ; `qy` que des listes ; `ra`/`p` que des REF de vecteur (tag `UNKNOWN`, dont `ui.qa`) ; `p_` que `true` ; les clés `pn`/`du`/`py` que des fixes.
- **H5** : un `pico8_<X>_new(NULL)` neuf, pour les 9 classes de `cg.np`, s'inscrit dans `cg.np` par `lt` et prend un `p` écrit par nom ; un `hq` d'une sonde neuve à `mp = p·128 + (1, 1)` (forme `mouse_at`), cibles espacées de plus de √0,025 et loin de la galaxie, prend la branche `nq < dw` sur cette cible. Sinon l'implémenteur vise une planète de ce genre dans la galaxie, ou garde l'aller-retour par nom pour ce genre, et le note au commit (précédent H4 de 1b.24) ; la preuve par le code reste, ce n'est pas un arrêt.
- **H6** : seuls les 8 descripteurs de l'arbre `qh` posent `PICO8_METHOD_LY`.
- **H7** : tout binaire qui lie `probe.c` compile et lie avec les deux `#include` ajoutés.
- **H8** : le build `-DPICO8_MEMBER_CENSUS` compile avec un `pico8_g__t` sans `pico8_value_t` ; `-fsyntax-only -DPICO8_MEMBER_CENSUS` sur les fichiers touchés suffit si le build complet n'est pas lancé (précédents 1b.29/1b.30).
- **H9** : `test_pico8_g_` reste sous 30 s avec deux fonctions de plus (il rejoue déjà le rejeu de glissé dans `test_drag`) ; la caractérisation ne rejoue aucun rejeu. Démentie, on allège la caractérisation, on ne déplace pas le TU : le `Makefile` ne change pas.
- **H10** : `pico8_ob_table_set_name(b, "pn", pico8_value_ref(NULL))` garde la clé (H11 de 1b.29), donc le cas « clé nil » du test d'arrêt atteint `g__take_fixed`.
- **H11** : sur l'ancien code, chaque cas du test d'arrêt se termine sans `SIGABRT` (l'écrivain et la boucle de clés recopient tout) ; un cas qui avorte déjà est retiré du « rouge d'abord » et noté.
- **H12** : valeurs **observées** sur une sonde neuve : les 18 de la spec, `fp` `BOOL` 0 présent, `od` `FIXED` 0 présent, `qy` liste vide, `ra`/`p`/`et`/`p_` absents ; sur un `fv` neuf : les 13 de sa spec, `ra` présent, `p`/`et`/`p_` absents.
- **H13** : les preuves rouges §6 rougissent : 3175ebd (R-a), double `pq` de `jo` (R-b), `ga:pq` sans `ez` (R-c, H14 de 1b.29 vraie), 2d7021f (R-d), `if (0)` sur `jc:874` (R-e) ; pour R-f, le rejeu construit une sonde.
- **H14** : la réécriture « présent 0 » de `test_pico8_g_.c:889-890` garde `!is_probe(roads[1])` vrai, et rien ne relit `roads[1]` avant `pico8__init()`.
- **H15** : `make sanitize-net` propre (aucune allocation nouvelle ; `et` pendant jamais lu sans un `bv` qui vient de l'écrire).
- **H16** : aucun test de régression ne dépend de la représentation des 23 membres ; rien ne dépend de `sizeof(pico8_g__t)` ni de la disposition de `pico8_fv_t` (`g_` reste son premier membre).
- **H17** : revue des TU lents confirmée par l'implémenteur (grep ci-dessus) : aucune lecture nil par nom d'un membre de `g_` sur un `g_`/`fv`.
- **H18** : le crochet `STOP` gardé seul dans `test_pico8_g_` laisse la table régénérée identique (non lancé).
- **H19** : `-Werror` muet : `size_t i` retirés (`pico8_g__defaults_reset`, `pico8_g__struct_init`, `pico8_g__struct_new`, `pico8_probe_init`) ; toutes les aides appelées ; `pico8_g__et` déclarée.
- **H20** : empreinte réelle comptée au commit.
- **H21** : aucune valeur figée ne bouge : `BUILT`/`LIFE`/`EY_ON`/`EY_OFF`/`DRAG`/`FV`/`BV_ET_MEMBERS_HASH` de `test_pico8_g_`, `BUILT`/`LIFE`/`HOVER`/`REPLAY_HASH` de `test_pico8_fv`, les hash de `test_pico8_jc` (`BV_HASH`…), de `test_pico8_ui` (`QH_MEMBERS_HASH`…), de `test_pico8_bg`, et les 15 empreintes du filet.
- **H22** : `fp` lu `BOOL` 0 garde `d_` identique (`fm:nx(self.fp or 0)` → 0).

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Les nouveaux tests n'accèdent à aucun des 23 membres directement : ils compilent sur les deux formes.
- **(G) Où : `test_pico8_g_`**, TU de la classe, rapide, dans `CENSUS_UNIT_TESTS`, lié par `ENGINE_SRCS`, qui inclut déjà `planet.h`, `planet_family(_trade).h`, `slipway.h`, `help.h`, `wormhole.h` et a `start_game`, `ldown`, `keyed`, `a_planet`, `a_slipway`, `mouse_at`, `et_kind`, `is_bv_target`, `must_abort_value`, `in_fresh_child` (précédents 1b.25, 1b.29 : caractérisation dans le TU rapide de la classe). Les poses de `fv` y sont jugées aussi (`fv` de l'écran titre et `pico8_fv_new(NULL)`).
- **Caractérisation dirigée `test_g__members_by_name`**, par `in_fresh_child` après `test_g__is_a_struct`, hash **`G__MEMBERS_HASH`**. Lecture par le **lecteur brut** (`cls->field`, témoin `0x1234`) **et** par `pico8_entity_get_field`, qui doivent concorder (retour, genre, valeur, pointeur) ; adresses jugées par `assert`, jamais pliées.
  1. **Neufs** : une sonde par route (`ldown`, `pico8_g__new(NULL)`, `keyed`), le `fv` du titre et un `pico8_fv_new(NULL)`. Les 23 noms pliés (présence, genre, valeur). `assert` : `nu`, `k_`, `dq` au pointeur de `pico8_g__default` sur les sondes ; sur les `fv`, les 13 recouverts égaux à `pico8_fv_default` (`ra`, `k_` au même pointeur), `nu`, `dq` au pointeur de `g_` ; `qy` liste de taille 0, distincte d'une instance à l'autre ; `ra`/`p`/`et`/`p_` absents sur les sondes, `p`/`et`/`p_` absents sur les `fv` (H12). Clés : `pn` 7, `t` 3, `du` 10, `py` 1 sur `keyed`.
  2. **`bv` vers chaque cible** (H5), `start_game(DRAG)` : une sonde neuve mène `hq` vers une planète neuve de chacune des 9 classes de `cg.np` (`jo`, **`qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`**, placées par `p` écrit par nom), vers un `jc` (`a_slipway`), hors du panneau (cible jetable, `rp` (2,2)) et dans le panneau (`false`, `rp` (64,64)) ; le `fv` mène `hq` une fois (délégation `fv` → `g_`). Attendu : `et` = la cible au même pointeur et son `fo` = le porteur, ou `BOOL` 0 présent, ou la cible jetable ; `ra` = le pointeur de `mp` ; `p` plié. **Les 8 classes hors ligne exercées ici prouvent sur l'ancien code qu'elles sont productrices** (lecture 1b.22).
  3. **Écrivain par nom**, sur une sonde et sur le `fv` :

     | nom | écritures | lecture attendue |
     |---|---|---|
     | `qy` | une liste neuve (forme de 1b.26/1b.29) ; `REF NULL` | même pointeur ; absent |
     | `ra`, `p` | une REF de vecteur du tas ; `REF NULL` | même pointeur ; absent, témoin intact |
     | `et` | les 11 classes admises ; `false` ; **`false` à union nulle** ; `REF NULL` | même pointeur ; `BOOL` 0 présent ; `BOOL` 0 présent ; absent |
     | `p_` | `true` ; `false` ; **`false` à union nulle** ; `REF NULL` après `true` | `BOOL` 1 ; `BOOL` 0 présent ; `BOOL` 0 présent ; absent |

     `cls->set_field` rend 1 pour les 5 noms ; il rend 0, sans rien toucher, pour les 18 défauts et `me`, `t`, `fo`, `m_` (sur la sonde), `""`, `"qy "`, `"P_"`, `"has_et"`, `"has_p_"`.
  4. **Clés** : `pico8_g__new` avec `pn`/`du`/`py` à **fixe 0 à union nulle** → `FIXED` 0 présent.
  5. **`d_`/`dl`** : une sonde à `et` `false` (in-panel), `call(a, "ey")` → retour, `me`, `nz` de `fm` pliés (`od`, `dq`, `fp` lus par nom) ; puis `p_` après un `ey` refusé ; `ev` → empreinte d'écran.
  6. **Intrus** sur le lecteur : `""`, `"n"`, `"nu "`, `"NU"`, `"et_"`, `"has_et"`, `"has_p_"`, `"p__"`, `"q"` → 0, témoin intact.
  7. Tout plié dans **`G__MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ, figé ensuite.
- **Test d'arrêt `test_g__writer_stops_off_row`**, rouge d'abord, par `in_fresh_child` après la caractérisation ; `must_abort_value` existant ; cas à union nulle par `memset` puis `kind` (forme `test_bv_et_writer_stops_off_row`) :

  | cible | nom | valeurs hors genre |
  |---|---|---|
  | une sonde neuve | `qy` | fixe 1, **fixe 0 union nulle**, `true`, **`false` union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table`, REF vecteur, REF d'un `jc` |
  | idem | `ra`, `p` | idem sans REF vecteur, plus REF `pico8_hv()` (liste) |
  | une sonde neuve et le `fv` (délégation) | `et`, non-structs | fixe 1, **fixe 0 union nulle**, **`true`**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table`, REF `pico8_hv()`, REF vecteur |
  | idem | `et`, **structs refusées** | un `ga` neuf, une autre sonde, un `fv`, un `mz` neuf, le `bg`, l'`ok`, un `hp` de `gb`, `pico8_mz_jj_wrapper()` |
  | une sonde neuve et le `fv` | `p_` | fixe 1, **fixe 0 union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table`, REF vecteur |
  | construction | clés `pn`, `du`, `py` | **nil (`REF NULL`, H10)**, `true`, **`false` union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table` |

  **Exclus** (dans le genre) : `false` et `false` union nulle pour `et` et `p_` ; `true` pour `p_` ; les 11 classes admises ; fixe 0 union nulle en clé. Rouge sur l'ancien code (H11), vert après : une ligne « Runs » pour chacun. Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Tests existants qui figent déjà beaucoup** : `test_built` (`fold_probe`, identités `nu`/`k_`, clés), `test_life` (`et_kind` 1 à 4, `fo`, `ra` = `mp`, `qy`, `p_`), `test_ey_on`/`off`/`void`, `test_drag` (rejeu, `p_`, `et`, `qy`, `ra` par frame), `test_fv` (18 défauts sur `fv`, `FV_HASH`) ; `test_pico8_fv` (`own[]`, `inherited[]`, `tables[]`, `absent[]`) ; `test_pico8_jc` `test_bv` ; `test_pico8_bg` (`ldown`) ; `probe_mm_rnd_order` ; les 15 rejeux.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_g_ && ./tmp/test_pico8_g_` sauf mention, notées dans le commit avec leur test rouge ou la justification du trou ; un appel unique d'aide `static` se retire sous `if (0)` (1b.14b) :
  1. **(§7-1) indicateur forcé vrai** : le lecteur ignore `has_p_` (nil lu `false` présent) → rouge : caractérisation 1 et 3, `test_ey_off` (`!has(a, "p_")`). **(1b)** `pico8_g__et` ignore `has_et` → rouge : caractérisation 1, `fold_probe` (`not_yet[]`).
  2. **(§7-2) « présent 0 » là où c'était absent** : sans objet dans sa lettre (aucun fixe typé ne peut être absent). **Forme voisine** : la branche `p_` rend `pico8_value_fixed(0)` quand `has_p_` = 0 → rouge : caractérisation 1, `fold_probe` ; `test_pico8_fv` (`absent[]`) noté.
  3. **(§7-3) init qui saute un membre typé** — **dans sa lettre sur `fv`** (la boucle de sa table reste) : (a) pose `self->g_.fa = g_fv_fa` sous `if (0)` (le `fv` garde 2) → rouge : `test_fv` (`fa` = 10, `is_help`), caractérisation 1 ; (b) pose `self->g_.ra = g_fv_ra` sous `if (0)` (`ra` nil) → rouge : caractérisation 1, `test_pico8_fv` (`tables[]`) noté ; (c) pose `k_` d'une copie neuve de la table → rouge : caractérisation 1 (pointeur). **Sur `g_`**, plus de boucle, forme « pose une autre valeur » (trou justifié de la lettre, précédents 1b.14a/1b.24) : (d) `has_et = 1` en R3 → rouge : caractérisation 1 ; (e) `od = g_g__fx` en R3 → rouge : caractérisation 1 et 5, `test_ey_off`/`EY_OFF_HASH` (crédits) ; (f) `g_g__nu` déballé dans une copie neuve → rouge : `fold_probe` (identité).
  4. *Clé `pn` ignorée* (branche `pn` sous `if (0)`) → rouge : `test_built` (`pn` = 7), caractérisation 1 ; `probe_mm_rnd_order` noté.
  5. *Prédicat sans `LY`* → rouge : caractérisation 2 et 3 (8 classes hors ligne) ; `test_pico8_ui` (1b.24, porteurs) non concerné, noté.
  6. *Prédicat sans `jo`* → rouge : `test_life` (`et_kind` 2), caractérisation 2.
  7. *Prédicat sans `jc`* → rouge : `test_life` (`et_kind` 3), caractérisation 2 ; `test_pico8_jc` `test_bv` noté.
  8. *Prédicat sans `bv_et`* → rouge : `test_life` (`et_kind` 4), `test_ey_void`.
  9. *Prédicat qui accepte toute struct* (`return 1`) → rouge : test d'arrêt (structs refusées) seul.
  10. *Tag non vérifié* (conjonction `STRUCT` de `g__take_target`, `LIST` de `g__take_list`, `UNKNOWN` de `g__take_vec` retirées une à une) → rouge : test d'arrêt.
  11. *`true` accepté pour `et`* → rouge : test d'arrêt (`true`).
  12. *Nil jugé sur le pointeur seul* (`value.as.ref == NULL` sans le genre) → rouge : test d'arrêt (union nulle) pour `qy`/`ra`/`p` ; pour `et`/`p_`, caractérisation 3 (`false` union nulle lu absent).
  13. *Écrivain d'`et` muet* (`if (0) g__take_target(…)`, `return 1` gardé) → rouge : `test_life`, caractérisation 2 et 3.
  14. *Clé `pn` accepte nil* (`g__take_fixed` pose 0 sur `REF NULL`) → rouge : test d'arrêt (clé nil).
  - **Mutants équivalents, notés** : `*has = 1` avant l'`assert` d'une aide (fils qui avorte, état jamais relu) ; remise à 0/NULL retirée d'un `defaults_reset` (l'init redéballe avant toute lecture).
- **`gcov`** : `test_pico8_g_` et `test_pico8_fv` en `--coverage -O0`, piège `__gcov_dump` des fils (1a.23), sur `probe.c` et `help.c`. Lignes attendues exécutées : déballages et remises à nil, poses R3 de `g_` et de `fv`, 23 branches R4 (trois états d'`et`, deux de `p_`, branches NULL des pointeurs), les 5 branches d'écrivain, les aides (hors échec d'`assert`), le prédicat (4 disjonctions), les 4 branches de clés et l'`else`. **Trous attendus** : échecs d'`assert` des aides d'écriture (fils de `must_abort`, prouvés par le test d'arrêt et les mutations 9–12, 14) et des aides de lecture de spec (inatteignables, spec littérale). Rapport dans le commit.
- **Prior art** : `test_qh_members_by_name`/`test_qh_writer_stops_off_row` de `test_pico8_ui` (1b.24) ; `test_bv_et_members_by_name`/`test_bv_et_writer_stops_off_row` de `test_pico8_g_` (1b.22) ; `family_take_target`, `family_take_list`, `family_take_vec`, `jo_take_bool`, `family_trade_default_*` ; `test_ga_*` de `test_pico8_ga` (1b.29, clés).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer.

- `./tmp/test_pico8_g_` : ≤ 30 s (H9) ; environ 24 lancements (départ, arrêt rouge, final, 20 mutations, `gcov`) ≈ 10 min. Si un lancement dépasse 20 s, on joue d'abord 1, 1b, 2, 3a, 3d, 3e, 4, 5, 8, 9, 10, 12, 13, et on note les autres non lancées.
- `./tmp/test_pico8_fv` : départ, final, `gcov`, mutation 3b ; `./tmp/test_pico8_jc` : R-e (deux lancements).
- §6 R-a à R-d : deux lancements chacun, quelques secondes ; R-f dans le budget seulement.
- `make test` une fois ; `make e2e` une fois, **dans la même invocation** que `make class-coverage-net` ; `make sanitize-net` une fois, jamais en même temps que `e2e` ; `make value-members`.
- Aucun TU lent n'est modifié ; compiler tôt sans lancer (`make -j tmp/test_pico8_hj tmp/test_pico8_qh tmp/test_pico8_jo`) : `make e2e` les lance.

| # | Commande exacte | État de l'arbre (départ / final / mutation n / §6 R-x bug remis / §6 R-x restauré) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` vert, les **8** TU de `UNIT_TESTS_SLOW` verts ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **72**, aucune ligne `pico8_g__t`, 10 lignes `pico8_fv_t` ;
5. `timeout 15m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` — requis, `pico8_g__struct_new`, `pico8_g__struct_init` et `pico8_fv_struct_new` changent (sans toucher `pico8_class_coverage_record` ni `KNOWN[]`).

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements (`slipway_drag_events.json` est seulement **lu**).

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet**

- `FAILED:` si 1b.31 n'est pas commitée au départ, ou si `make value-members` ne rend pas 95 au départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, ou si le diff touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements.

**Types, tables, écrivain**

- `FAILED:` (« champ sans règle ») si une aide d'écriture ou de lecture de spec avorte dans un rejeu ou un TU hors des fils de `must_abort` (H3, H4), ou si un type est élargi au lieu d'avorter : `true` accepté pour `et`, `et_is_bool` ajouté, prédicat étendu hors des 11 classes, clé nil acceptée, `pico8_value_t` gardé.
- `FAILED:` si l'un des 23 membres n'a pas le type retenu à sa place, si `has_et`/`has_p_` manque ou n'est pas juste après son membre, ou si un autre `has_` est ajouté.
- `FAILED:` si nil et `false` d'`et` ou de `p_` ne se lisent plus par nom exactement comme en 1a (absent ; `BOOL` 0 présent).
- `FAILED:` si `pico8_g__t` garde un `pico8_value_t`, si `g_g__members`, `g_g__defaults`, `keys[]` ou un `offsetof` existe encore dans `probe.c`, ou si `help.c` garde un `offsetof(pico8_fv_t, g_.`.
- `FAILED:` si une entrée propre de `fv` est retirée de `g_fv_defaults` ou `g_fv_members`, ou si un membre propre de `fv` est typé.
- `FAILED:` si `nu`, `k_`, `dq` (sonde) ou `ra`, `k_` (`fv`) ne sont pas les **pointeurs de la spec** de leur classe, ou si un `static` nouveau n'est pas remis à nil dans son `defaults_reset`.
- `FAILED:` si l'écrivain de `g_` n'accepte plus exactement ses 5 noms, si la boucle de clés n'accepte plus exactement `t`, `pn`, `du`, `py`, ou si le message de l'`assert(0 && …)` final change.
- `FAILED:` si une aide viole la forme de 1b.13/1b.15 (état nil d'abord, sauf `g__take_fixed` ; genre **et** pointeur testés ; union nulle hors genre qui avorte ; message qui nomme `g_.<m>` et le genre ; `has_` posé en dernier ; tag vérifié avant `cls`), ou si elle est partagée entre fichiers ou rendue publique.
- `FAILED:` si le prédicat n'admet pas exactement les 8 descripteurs à `LY`, `jo`, `jc`, `bv_et`, ou si `probe.c` gagne un autre `#include` que `pico8_planet.h` et `pico8_slipway.h`.

**Diff et preuves**

- `FAILED:` si un site par nom change (`bv`, `dl`, `d_`, `mm`, `f_`, `ez`, `ei`, `ui.c`…), ou si un site par nom est remplacé par un accès direct.
- `FAILED:` si un autre fichier de `src/` que `probe.h`, `probe.c`, `help.c` change, ou `tools/`, le `Makefile`, `CENSUS_UNIT_TESTS`, `UNIT_TESTS_SLOW`.
- `FAILED:` si le diff d'un test fait autre chose que ce que liste « Tests touchés », ou si une valeur figée existante change (H21).
- `FAILED:` si un test de régression (§6) a un diff, ou s'il manque les deux lignes « Runs » (rouge bug remis, vert restauré) de R-a, R-b, R-c, R-d, R-e ; pour R-f, soit ses deux lignes, soit « non lancé » au journal.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas l'attendu, si `make sanitize-net` n'est pas `15 propre(s)`, si `make value-members` ne rend pas 72 ou si un autre typedef gagne ou perd une ligne, ou s'il manque la ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un crochet du recensement change (le `STOP` de `test_g__is_a_struct` gardé seul), ou si `tools/member_census.txt` est édité à la main.

**Tests de l'étape**

- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, lit ou écrit un des 23 membres directement, ne couvre pas les trois routes, les deux `fv`, les 23 noms, les 11 classes d'`et` (dont les 8 hors ligne par `hq`), `false`, nil, `p_`, les clés ; ou si `G__MEMBERS_HASH` change après.
- `FAILED:` si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'arbre de départ et une verte sur l'arbre final, ou s'il n'a pas les cas union nulle `memset`, `true` pour `et`, les structs refusées, nil en clé, et la délégation `fv`.
- `FAILED:` si le commit ne porte pas le rapport `gcov` et les mutations 1 à 14 (avec 1b, 3a–3f), chacune avec son test ou la justification de son trou, plus les mutants équivalents.

**Commit et note**

- `FAILED:` si le travail n'est pas en un commit vert.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, ou s'il manque le tableau « Runs » ou la note « Découvert à 1b.32 ».

## Out of Scope

- Les 10 membres propres de `fv` (1b.33, dont `m_` sans genre) ; `g_fv_members`, les 9 entrées propres de `g_fv_defaults`, `pico8_fv_field`/`pico8_fv_set_field`.
- Les homonymes : `qh.et`/`od`/`fp`/`dq`/`dw` (1b.24, faits), `jc.et`/`od`/`fo` (1b.35), `mz.et`/`dw`/`dq`/`od`/`fp` (1b.36), `ui.qa`/`lheld` (1b.37), `ga.qy`, `hj.py`…
- `pico8_bv`, `dl`, `d_`, `mm`, `f_`, `ez`, `ei` eux-mêmes (sites par nom ; TROU-7 de `p`, de `qy`, des particules).
- Réunir `g__is_target_class` et `family_is_target_class` (1b.13 point 2).
- Déplacer un TU entre listes ; régénérer `tools/member_census.txt`.
- Fichiers d'événements, `FRAMES_EVENTS`, rendu, audio, appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur (G)** : modules `probe.h` (typedef, `has_et`, `has_p_`, `pico8_g__et`), `probe.c` (lecteur, écrivain, 5 aides d'écriture, 4 aides de spec, 18 `static`, prédicat, clés, deux `#include`), `help.c` (13 `static`, 3 aides de spec, poses, commentaire) ; `et` → `pico8_struct_header_t *` + `has_et`, `p_` → `int` + `has_p_`, `fp` → `int`, `qy` → `pico8_list_t *`, `ra`/`p` → `pico8_vec_t *`, `nu`/`k_` → `pico8_ob_table_t *`, `dq` → `const char *`, 14 fixes ; ensemble d'`et` = 11 classes ; tests dans `test_pico8_g_`, `written[]` supprimé/réduit, deux accès directs réécrits ; pas de découpage ; §6 : R-a à R-e obligatoires, R-f dans le budget.
- **Note « Découvert à 1b.32 »** (à écrire sur la ligne ; le reste va dans le commit) :

  > patron 1b.1 appliqué ; `fx`, `od`, `fa`, `dw`, `py`, `ki`, `pn`, `pk`, `fc`, `qo`, `ro`, `qg`, `q_`, `du` → `pico8_fixed_t`, `fp` → `int`, `nu`, `k_` → `pico8_ob_table_t *`, `dq` → `const char *` (pointeurs de spec), `qy` → `pico8_list_t *`, `ra`, `p` → `pico8_vec_t *` ; **`et` : lecture de 1b.24 sans écart** (`pico8_struct_header_t *` + `has_et`, `true` avorte, `pico8_g__et`), **ensemble = ligne ∪ `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`** (`cg.np`, exercés par `hq` sur l'ancien code), prédicat `g__is_target_class` copie de celui de `qh` ; `p_` → `int` + `has_p_` ; R2 : `od` sans indicateur (`stat?` d'homonyme) ; R5 écrivain de 1b.13/1b.15 sur 5 membres (`g__take_list/vec/target/bool`) ; **`keys[]` vidée, `g__take_fixed` étendu à `pn`/`du`/`py`** (1b.14b, 1b.29) ; **`g_g__members`, `g_g__defaults` supprimées, `probe.c` sans `offsetof`** ; R7 de 1b.14a sur `fv` (13 `static`, `g_fv_defaults` réduite à ses 9 entrées propres pour 1b.33) ; boucle sentinelle de `g_` supprimée (`STOP` gardé seul), celle de `fv` réduite à `m_` ; nil de `fa` écrit par un test → « présent 0 » (1b.31) ; §6 : `probe_mm_rnd_order`, `playability`, `geoharvest_burst`, `slipway_drag` sur le chemin, prouvés rouges sans diff, `test_pico8_jc:874` réécrit et prouvé rouge ; empreinte réelle mesurée au commit.
- Le message de commit porte : compte 95 → 72, résumé « Runs », `gcov`, mutations et trous, `G__MEMBERS_HASH`, réponses à H1–H22.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
