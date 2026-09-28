---
title: "Plan 3, étape 1b.35 : `jc`, la voie de glisse. Ses 16 membres typés ; `qu`, `re`, `et`, `fo` en pointeurs d'en-tête (1b.22), `od`, `mt`, `p_` avec indicateur ; écrivain `fo` à ensemble « porteurs de `bv` ∪ `mz` ∪ `jc` », clé `qu` à ensemble « `LY` ∪ `jo` » ; la table `members[]` et tout `offsetof` de `slipway.c` supprimés ; `test_draw` n'écrit plus un `jo` dans `jc.fo`"
description: "Après 1b.34, `pico8_jc_t` garde 16 membres `pico8_value_t` : 7 défauts de spec posés en littéraux (`nu`, `fa`, `fp`, `ik`, `dw`, `k_`, `nh`), la clé `qu`, ce que `pq` alloue (`ts`, `hg`), ce que `hq`/`ey` réécrivent (`re`, `od`, `mt`, `et`, `fa`, `nh`, `p_`) et ce qu'écrivent les autres (`fo`, `p_`). 1b.35 les type selon R1–R9 de 1b.1. Les quatre REF à plusieurs cibles prennent la lecture de 1b.22 (pointeur d'en-tête) ; seules `fo` (écrivain) et `qu` (clé) ont une aide R5, avec un prédicat de descripteur et un ensemble = ligne ∪ producteurs exercés sur l'ancien code (`fv`, `nl`, `os`, `pm`, `mx` pour `fo` ; `hj`, `nl` pour `qu`). La ligne `test_pico8_jc:695` (un `jo` dans `jc.fo`, arrêtée pour le recensement à 1b.4b) est réécrite, `test_pico8_g_:494` (écriture directe de `re`) aussi. Un seul commit. `make value-members` passe de 43 à 27."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.35 — `jc` (`slipway.h:19`), REF multiples `qu`, `re` (dont `jc_mouse_point`, 1b.12), `et`, `fo`. Empreinte 30 (14, 16). Compte : 27.

Cadre : puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », §3 (`make test` vert, aucun `*.sha256` modifié), §6, §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Modèles : le PRD de 1b.34 (`plan3-phase1b-etape34-js-bouton.md`, forme la plus récente, table supprimée d'un fichier à trois structs) et celui de 1b.32 (`plan3-phase1b-etape32-g-sonde.md`, `et` à plusieurs cibles, porteurs de `bv` exercés par `hq`). Notes « Découvert à » **citées, pas redérivées** ; elles lient ce PRD :

- **1b.1** : patron R1–R9 ; s'en écarter est une lecture nouvelle, notée.
- **1b.4b** : **`jc.et` + `ref:pico8_jc_too_far_t`, `jc.fo` + `ref:pico8_g__t`** (« sans effet d'ordre ») ; `jc_too_far.fo` → `pico8_jc_t *` typé à 1b.13 (« `jc` après ») ; le `g_` de la table vise `jc.fo` (1b.35) ; **l'écriture de test `jc:692` (`test_draw` pose un `jo` dans `jc.fo`, ce que le cart ne fait jamais : seul `self.et.fo=self` d'un `jc` ou d'un `g_` l'écrit) est arrêtée par `PICO8_MEMBER_CENSUS_STOP()`** — la ligne `fo` ne porte donc pas `jo` ; aucune boucle sentinelle `written[]` ne vise `jc`.
- **1b.5** : R7 forme accesseur — preuve `grep 'pico8_value_t \*' … | grep -v out_value` vide ; emballage public seulement pour un membre à genres mêlés (aucun ici).
- **1b.7** (homonymes) : un nil `stat?` venu d'un homonyme ne donne pas d'indicateur à lui seul ; ici `od`, `mt`, `re`, `et`, `fo`, `p_` ont aussi `lu`, preuve directe.
- **1b.9/1b.20/1b.31/1b.33** : R7 forme `offsetof` — une table vidée est supprimée avec son seul client (ici le `members[]` local du lecteur), branches R4 `strcmp` dans l'ordre du typedef ; `<stddef.h>` retiré quand plus aucun `offsetof`.
- **1b.10/1b.29** : boucle de clés directe, une branche par clé, `else` final à l'`assert(0 && …)` au **message inchangé**.
- **1b.11** : R2 des TU lents — relus plus bas.
- **1b.12** : `jc_mouse_point` est une **cible de `jc.re`** ; son `ra` est déballé de `mp` par `pico8_jc_mouse_point_take_ra` (inchangé).
- **1b.13** (forme écrivain de R5) et **1b.15** (aide à indicateur) : une branche par membre `e`, ordre du typedef ; aide `static` par genre, forme par valeur, `<X>_take_<genre>` ; l'aide pose **d'abord** l'état nil, puis sort sur `REF NULL` (genre **et** pointeur testés) ; sinon `assert` « champ sans règle » qui nomme `struct.membre`, le genre de la ligne et l'arrêt ; nom sans branche → `return 0` ; **test d'arrêt rouge d'abord**, cas non-REF à union nulle (`memset` puis `kind`, 1b.14b) ; `jc_too_far_take_jc` existe déjà dans ce fichier.
- **1b.14a** : R5 sur une méthode (valeur lue par nom sur une autre entité puis rangée) : aide par valeur de 1b.13 (`ke_take_string`) — ici `mt` dans `hq`.
- **1b.14b** : un fixe sans table de défauts de fille garde le littéral de l'init ; une mutation qui retire l'unique appel d'une aide `static` se plante sous `if (0)` (`-Werror`).
- **1b.19** : branches remises dans l'ordre du typedef.
- **1b.22** : **REF à plusieurs cibles → `pico8_struct_header_t *`** ; **ensemble admis = ligne ∪ classes productrices prouvées par le code et exercées par la caractérisation sur l'ancien code** ; **prédicat** par accesseurs publics quand tous sont liés partout où le fichier l'est, sinon par une propriété du descripteur que l'ensemble partage exactement (`DATA_ENGINE_SRCS` lie `slipway.c` sans `planet_family_trade.c` ni `help.c`).
- **1b.23a2** : un membre typé sort des `written[]` des TU (vacant ici) ; **un test qui écrit un membre directement change dans la seule forme de son écriture et porte sa preuve rouge** (précédent `test_pico8_jc:874`, 1b.32).
- **1b.23d** : `jo.fo` → `pico8_struct_header_t *`, prédicat `jo_is_fo_holder_class` = `methods[HQ] == pico8_bv_dispatch` ∪ `jc` ; `mz` exclue de `jo.fo` par preuve (`mz.dw` = 0, `nq < dw` jamais vrai) — **mais `bv` d'un `mz` peut viser un `jc`** (branche `jp(cg.jc, …)`, sans seuil `dw`) : `mz` est sur la ligne de `jc.fo`.
- **1b.24** : le crochet `STOP` d'une boucle supprimée est gardé seul ; preuve de typedef par `awk` (pas `sed`).
- **1b.29/1b.32/1b.34** : un test de régression **sur le chemin** du membre typé, sans diff, est prouvé rouge ; H « clé nommée à `REF NULL` gardée par `pico8_ob_table_set_name` ».
- **1b.30/1b.31/1b.33/1b.34** : caractérisation dans le TU rapide de la classe ; un nil écrit directement par un test n'est pas une preuve.
- **1b.32** : `g_.et` → `pico8_struct_header_t *` + `has_et`, ensemble ∋ `jc` (`g__is_target_class`) ; `test_bv_et_members_by_name` (`test_pico8_g_`) mène `hq` de 11 porteurs neufs sur un point : **précédent de forme** pour les porteurs de `jc.fo`.

### État de départ

HEAD `7ffa242` (PRD de 1b.34) au moment de l'écriture ; 1b.34 est implémentée **en parallèle** (l'arbre de travail modifie `src/pico8_ui_button.[ch]` et `tests/test_pico8_js.c`, lus sans être touchés). **Précondition : 1b.34 (`js`) est commitée avant que 1b.35 commence (H1)** ; `make value-members` rend alors **43** (16 lignes `pico8_jc_t`, 17 `pico8_mz_t`, 10 `pico8_ui_t`). 1b.32 (`0638c21`) et 1b.33 (`98d993b`) sont commitées. Numéros de ligne indicatifs (arbre `7ffa242`) ; chaque site est aussi désigné par son contenu. **Rien n'a été compilé ni lancé pour ce PRD** : les faits viennent de `grep` et de la lecture ; tout fait d'exécution est une hypothèse (H1–H27).

### Lignes de recensement (`tools/member_census.txt:205-220`)

| membre | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|
| `nu` | `ref:table` | jamais | - | l | `pico8_ob_table_t *` (`g_jc_nu`, table du module) | non (pointeur) |
| `fa` | `fixe` | jamais | - | l | `pico8_fixed_t` (littéral 2, puis `hq` : 2 ou 4) | non |
| `fp` | `bool` | jamais | - | l | `int` (0/1, toujours 0) | non |
| `ik` | `fixe` | jamais | - | l | `pico8_fixed_t` (48) | non |
| `dw` | `fixe` | jamais | - | l | `pico8_fixed_t` (`PICO8_FIXED_LIT(0x00000106, 0.004)`) | non |
| `k_` | `ref:table` | jamais | - | l | `pico8_ob_table_t *` (`g_jc_k_`) | non (pointeur) |
| `nh` | `fixe` | jamais | - | l | `pico8_fixed_t` (20, `ey` : 1, `kj` : +1) | non |
| `qu` | `ref:` fr, jo, mx, nr, os, pm, qh | jamais | - | l | `pico8_struct_header_t *` (clé seule) | non (pointeur) |
| `ts` | `ref:liste` | jamais | - | l | `pico8_list_t *` (alloué par `pq`) | non (pointeur) |
| `hg` | `ref:table` | jamais | - | l | `pico8_ob_table_t *` (alloué par `pq`) | non (pointeur) |
| `re` | `ref:` fr, hj, jc_mouse_point, jo, mx, nl, nr, os, pm, qh | observé | lu | l | `pico8_struct_header_t *` (écrit par `hq`, `ey`) | non (pointeur) |
| `od` | `fixe` | observé | lu,stat? | l | `pico8_fixed_t` | **`int has_od`** |
| `mt` | `fixe` | observé | lu,stat? | l | `pico8_fixed_t` | **`int has_mt`** |
| `et` | `ref:` jc, jc_too_far, jo | observé | lu,stat? | l | `pico8_struct_header_t *` (écrit par `hq`) | non (pointeur) |
| `fo` | `ref:` fr, g_, hj, jc, mz, nr, qh | observé | lu,stat?,écrit… | **le** | `pico8_struct_header_t *` | non (pointeur) |
| `p_` | `bool` | observé | lu | **le** | `int` (0/1) | **`int has_p_`** |

Compte : 43 − 16 = **27**. Aucune ligne `pico8_jc_t` ne reste ; les lignes `mz`, `ui` (1b.36, 1b.37) ne bougent pas. Indicateurs : 3 (`has_od`, `has_mt`, `has_p_`), chacun juste après son membre, ne comptent pas (R9).

### Le cart (`pico8/lua.lua`)

- `jc=ke:jr([[nu=o("jc"),me="kz",fa=2,fp=f,ik=48,dw=0.004,k_=o(…),nh=20,]])` (`:612-619`). **`jc:pq`** (`:620`) : `rn(50)self.ts,self.hg={},{}`.
- **`jc:hq(mp)`** (`:622-632`) : `self.re=ok.o_.gv and ok.o_ or {ra=mp,p=mp/128}` ; `self.od=dj(self.qu,self.re)` ; `self.mt=self.qu.mt or self.re.mt` ; `self.et=self:ce()` ; `if self.et then self.et.fo=self end` ; `self.fa=self.mt and 2 or 4`.
- **`jc:ey(re)`** (`:633-640`) : `if not re or not d_(self)` → `self.p_=true` ; sinon `self.nh,self.re=1,re`, `kf` des deux bouts.
- **`jc:ce`** (`:641-653`) : `{qs="-too far-"}` au-delà de `ov("space folding",0.5,1.08)` ; un `w` de `self.qu.oe` qui joint déjà la paire ; sinon `ds[1]` de `jp(cg.np, be(…))` ∪ `jp(cg.jc, sw.mt==self.mt and cl(…))` — une planète de `cg.np`, un `jc`, ou nil ; **jamais `false`**.
- `jc:eb` (`:654-656`), `jc:ch` (`:657-660`, `self.hg[t.id]`, `add(self.ts,…)`), `jc:lg` (`:661-666`), **`jc:kj`** (`:667-681` : `self.k_[self.nh]`, `self.nh+=1`, `(self.et or self.fo) and 2 or self.mt and 0 or jv`, **`self.fo=nil`**), `jc:ev` (`:682-683`).
- Producteurs de `jc` : **`jo:ldown`** (`:412-416`, `self.me=="ly"` → `jc({qu=self})`), **`nr:ldown`** (`:1112-1113`). `me == "ly"` : `jo:hu` (`:458`) pour un `jo` de `cg.jo` ; `d_` (`:571`, `ks(self.dq or "ph")`, `dq="ly"` de `qh`, hérité par `hj`, `pm`, `mx`, `nl`, `os`, `nr`, `fr`) ; `pm` naît `me="ly"` (`:1021`). `ga` (`jo:jr`, `nu=o()`, hors `cg.jo`) n'y arrive jamais.
- Écrivains de `jc.fo` : `self.et.fo=self` de **`jc:hq`** (`et` = un autre `jc`) et de **`bv`** (`:552-558`, `et` = `jp(cg.jc, …)[1]`) ; `bv` est le `hq` de `g_` (`:722`), `qh` et sa descendance (`:915`), `fv` (fille de `g_`), et **`mz:hq`** l'appelle (`:767`). Écrivains de `jc.p_` : `jc:ey` (direct), `d_` (`:575`, par nom). `nil`/`false` jamais écrits dans `fo` ; `false` jamais dans `p_`.
- Lecteurs par nom hors `jc:*` : `dl` (`:559-568`, `self.et`, `self.et.qs`, `self.od`), `d_` (`self.et`, `self.od`, `self.dq`, `self.fp`, `self.ik`), `jo:kj` (`:548-549`, `sqrt(self.fo.dw)` : le `dw` d'un `jc` porté par une planète), `bv` (`sw.qu.p`, `sw.re.p`), `jc:ce` d'un autre `jc` (`sw.mt`, `sw.qu`, `sw.re`), `nr:gv` (`s.qu`), `jo:kf` (`sw:eb(self)`), `dg` (`p_`), `lt` (`nu`).

### Le code

**`src/pico8_slipway.h`** : typedef `pico8_jc_t` (`:17-34`), `ke` puis 16 `pico8_value_t` ; commentaire de tête (« `nu` and `k_` point at the tables the module owns … `ts` and `hg` are this slipway's own, made by `pico8_jc_pq`. Nothing frees the struct: TROU-7 ») qui reste vrai. Inclusions : `fixed`, `ke`, `struct`, `value` — `pico8_ob_table_t`, `pico8_list_t`, `pico8_struct_header_t` y arrivent déjà par `ke.h` → `struct.h` → `ob.h` → `list.h` (H18). API publique inchangée : `pico8_jc_ey(pico8_jc_t *, pico8_struct_header_t *re)`, `pico8_jc_ce`/`pico8_jc_eb` rendent `pico8_value_t`.

**`src/pico8_slipway.c`** :

- `pico8_jc_too_far_t` (`:30-87`, 1b.13) et `pico8_jc_mouse_point_t` (`:89-131`, 1b.12), typés, **inchangés**.
- Aides de lecture : `pico8_jc_truthy`, `pico8_jc_entity_value`, `pico8_jc_value_instance`, `pico8_jc_entity_instance`, `pico8_jc_value_vec`, `pico8_jc_entity_vec` (`:134-174`).
- `pico8_jc_pq` (`:176-192`) : `self->ts = pico8_value_ref(ts)`, `self->hg = pico8_value_ref(hg)`.
- `pico8_jc_hq` (`:194-247`) : `re_val` (entité survolée à `gv`, ou `jc_mouse_point` neuf), `self->re = re_val`, `self->od = pico8_value_fixed(od)`, `self->mt = mt_val` (**une `pico8_value_t` lue par nom sur `qu` puis `re`**), `self->et = et` puis `pico8_entity_set_field(et.as.ref, "fo", self)` sous l'`assert` de tag `STRUCT`, `self->fa` = 2 ou 4 selon `pico8_jc_truthy(mt_val)`.
- `pico8_jc_ey` (`:249-287`) : `self->p_ = pico8_value_bool(1)` ; `self->nh`, `self->re = pico8_value_ref(re)` ; `self->qu` pour `kf`.
- `pico8_jc_ce` (`:335-409`) : `self->qu`, `self->re`, **`jc_ctx.self_mt = self->mt`** comparé par `pico8_value_equal` au `mt` lu par nom des autres `jc`.
- `pico8_jc_eb` (`:411-428`), `pico8_jc_ev` (`:430-443`) : `self->qu`, `self->re`.
- **Lecteur `pico8_jc_field` (`:489-515`)** : `static` `members[]` locale de 16 `offsetof` et sa boucle (`REF NULL` → 0), repli `pico8_ke_field`. C'est la seule table d'offsets de `jc` : ni `defaults[]` (les défauts sont des littéraux du constructeur), ni `pico8_jc_member`.
- **Écrivain `pico8_jc_set_field` (`:517-530`)** : `fo` et `p_` recopiés **quel que soit le genre**.
- **`pico8_jc_struct_new` (`:544-581`)** : `me = "kz"`, 16 poses (`nu` = `g_jc_nu`, `fa` 2, `fp` `false`, `ik` 48, `dw`, `k_` = `g_jc_k_`, `nh` 20, 9 `REF NULL`), boucle de clés (`assert(key.kind == NAME && strcmp(key.name, "qu") == 0 && "jc: the only measured construction key is qu (jo:ldown, nr:ldown); another would be a field under no rule of plan-3 section 5 -- stop, do not add a member")`, recopie **de tout genre**), `pico8_jc_pq`, `pico8_ke_struct_pq`, `PICO8_MEMBER_CENSUS_SEE`.
- `offsetof` : 6 lignes, 16 occurrences, toutes dans `members[]` ; `<stddef.h>` inclus, `size_t` seulement dans le lecteur. `#include` : ni `pico8_planet.h`, ni `pico8_wormhole.h` ; `pico8_probe.h` (donc `pico8_bv_dispatch`) déjà.

**`src/pico8_slipway_display.c`** : `pico8_jc_ch` (`self->hg`, `self->ts`, `PICO8_REF_AS_LIST(ts_val.as.ref)`), `pico8_jc_lg` (`self->qu`, `self->re` par `pico8_jc_display_value_instance`), `pico8_jc_kj` (`self->qu`, `self->re`, `self->nh` par `pico8_jc_display_value_fixed` puis `self->nh = pico8_value_fixed(…)`, `self->k_`, `self->et`, `self->fo`, `self->mt` par `pico8_jc_display_truthy`, `self->ts` et `PICO8_REF_AS_LIST`, **`self->fo = pico8_value_ref(NULL)`**). Aides : `pico8_jc_display_value_instance` et `pico8_jc_display_value_fixed` n'ont **pas d'autre client** que ces lectures ; `pico8_jc_display_truthy` sert aussi à l'entrée de `hg`.

**Sites par nom, inchangés** (grep `"(M)"` sur `src/`, `M` les 16 noms) : `pico8_dl`/`pico8_d_` (`probe.c`, `et`, `od`, `fp`, `ik`, écriture `p_`), `pico8_bv` (`probe.c`, écriture `fo`), `pico8_bv_jc_predicate` (`qu`, `re`), `pico8_jc_ce_jc_predicate` (`mt`, `qu`, `re` d'un autre `jc`), `pico8_jc_hq` (écriture `fo` sur la cible), `pico8_nr_gv` (`qu`), `planet_display.c` (`dw` du `fo` d'une planète), `entity.c` (`nu` de `lt`, `p_` de `dg`). Les autres lignes (`bootstrap.c`, `economy_tiers.c`, `entity_geom.c`, `particle.c`, `ui.c`…) portent des homonymes.

**Tests, accès direct à un des 16 membres d'un `jc`** (grep `(->|\.)(M)\b` sur les fichiers qui nomment `pico8_jc_t`, plus `(pico8_jc_t *)`) : **un seul**, `tests/test_pico8_g_.c:494` (`a_slipway` : `((pico8_jc_t *)jc)->re = pico8_value_ref(re);`) ; `test_pico8_jc.c` ne lit et n'écrit que par nom (son `:874` écrit le `dw` d'une **sonde**, typé à 1b.32). **Écriture hors ligne** : `test_pico8_jc.c:695` (`test_draw`, sous `PICO8_MEMBER_CENSUS_STOP()`) : `pico8_entity_set_field(old, "fo", pico8_value_ref(origin))`, `origin` un `jo` — hors de l'ensemble de `fo` (1b.4b).

`tests/test_pico8_jc.c` (TU de la classe, rapide, 11 s mesuré à 1b.13, dans `CENSUS_UNIT_TESTS`) fige déjà beaucoup par nom : `check_fresh_jc` (défauts, `qu`, `ts`/`hg`, absents `re`/`od`/`mt`/`et`/`fo`/`p_`), `fold_jc` (11 des 16 noms, plus `me`, `t`), trois rejeux (`DRAG_HASH`, `HUB_HASH`, `PLAYTHROUGH_HASH`), `test_ey`, `test_hq` (`et` = `{qs}` ou `jc`, `fo` posé), `test_eb`, `test_draw`, `test_too_far`, `test_hq_mouse_point`, `test_bv` (`fo` = sonde), `test_jc_is_a_struct` (écrivain `fo`/`p_`, `must_abort(write_re/nh/fa, build_with_ra)`).

### Revue des TU lents (R2 de 1b.11)

`grep -nE 'pico8_jc_|cg\("jc"\)|"lheld"|slipway'` sur `test_pico8_{ie,jo,qh,os,hj,pm,mx,nr}.c` : `test_pico8_ie` (`pico8_jc_new(NULL)`, `mv(…, "hb")` : une méthode, pas un membre ; `lheld` lu sur `ok`) ; **`test_pico8_mx`** (`join` : `pico8_jc_new` avec **`qu` = un `hj`** — c'est pourquoi `hj` manque à la ligne `qu`, le recensement ne passe pas les TU lents — puis `ey` vers un `mx` : `d_` lit `od` nil par nom, déjà `has_od`) ; **`test_pico8_nr`** (`ldown` d'une porte → `jc` à `qu` = la porte, lu par nom ; `nr:gv(jc)` lit `qu`). Aucune lecture nil par nom d'un non-pointeur de `jc` hors `od`/`mt`/`p_`. **Aucun indicateur de plus que les 3 de la ligne** (H21). Ces deux TU lents exercent l'aide de `qu` (`hj`, `nr`) : `make e2e` les lance.

## Solution

1. **Réécritures de forme, sur l'ancien code d'abord** : `test_pico8_jc.c:695` écrit `old` (un `jc`, sur la ligne) au lieu de `origin` ; `DRAW_HASH` inchangé (H13). `test_pico8_g_.c:494` ne change qu'au commit (forme de l'écriture directe).
2. **Caractérisation figée sur l'ancien code** (arbre 1b.34 commité), dans le TU rapide `test_pico8_jc`, par nom seul ; verte avant tout typage.
3. **Test d'arrêt rouge d'abord** (écrivain `fo`/`p_`, clé `qu`) dans le même TU.
4. **Commit 1b.35** : les 16 membres typés, 3 indicateurs ; `members[]` du lecteur supprimée ; lecteur à 16 branches ; écrivain à 2 branches ; clé `qu` en branche typée ; 4 aides `static` (`jc_take_endpoint`, `jc_take_holder`, `jc_take_bool`, `jc_take_fixed`) et 2 prédicats `static` ; plus aucun `offsetof` ni `<stddef.h>` dans `slipway.c` ; `make value-members` rend **27**.
5. Aucun pixel ne bouge. Note « Découvert à 1b.35 » sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `slipway.h`, je veux `pico8_jc_t` sans `pico8_value_t`, chaque membre au type de sa ligne, chaque `has_` juste après son membre.
2. En tant que lecteur par nom (`dl`, `d_`, `bv`, `ce` d'un autre `jc`, `nr:gv`, `jo:kj`, `lt`, `dg`, `ui`, les TU), je veux que chacun des 16 noms rende sur tout `jc` exactement ce que rendait l'ancien code : présence, genre, valeur, identité du pointeur.
3. En tant que `jo:ldown`/`nr:ldown`, je veux construire un `jc` dont `qu` est moi, quelle que soit ma classe de planète (`jo` et les 8 descripteurs à `LY`), relu au même pointeur.
4. En tant que `jc:hq`, je veux ranger l'entité survolée ou un `{ra,p}` neuf dans `re`, `dj` dans `od`, le `mt` de `qu` ou de `re` (fixe ou nil), le résultat de `ce` dans `et`, et poser mon `fo` sur la cible.
5. En tant que `bv` (de `g_`, `fv`, `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`, `mz`) ou `jc:hq` d'un autre `jc`, je veux écrire mon pointeur dans `fo` du `jc` visé et le relire au même pointeur ; `jc:kj` veut l'éteindre.
6. En tant que `d_`/`jc:ey`, je veux lever `p_` et que `dg` le voie ; nil éteint `p_`.
7. En tant que `kj`/`lg`/`ch`/`ev`/`eb`/`ce`, je veux dessiner et répondre exactement comme avant.
8. En tant que gardien de `fo`, `p_`, `qu`, je veux que tout genre hors ensemble avorte, union nulle comprise : une planète dans `fo` (le cas de `test_draw`), un `ga` en clé `qu`, un fixe dans `p_`.
9. En tant que mainteneur, je veux `members[]`, tout `offsetof` et `<stddef.h>` absents de `slipway.c`.
10. En tant que tests de régression `slipway_drag`, `playability` (sans diff) et TU `jc`, `g_` (une ligne réécrite chacun), je veux rester verts et rouges quand mon bug revient.
11. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
12. En tant que `plan-pilot`, je veux `make value-members` à 27 sans ligne `pico8_jc_t`.
13. En tant que vérificateur, je veux la caractérisation verte sur l'ancien code, le test d'arrêt rouge puis vert, les preuves rouges §6, les mutations et leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle : les validations que demande `to-prd` (modules, tests) sont tranchées comme le ferait un agent `guideline` — hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables — et marquées (G).

### Quatre REF à plusieurs cibles : `pico8_struct_header_t *` (lecture de 1b.22 sans écart) (G)

`qu`, `re`, `et`, `fo` → `pico8_struct_header_t *`, commentaire `/* borrowed, never freed */` (forme `g_.et`), **sans indicateur** : aucun ne tient `false` (cart : `ce` rend nil, jamais `false` ; aucun `true`/`false` écrit ailleurs, H7), NULL = nil. Ni union, ni `void *`, ni base `ke` (1b.22). **Seuls `fo` (écrivain) et `qu` (clé) reçoivent une valeur par nom** ; `re` et `et` ne sont écrits que par `hq`/`ey` depuis des pointeurs déjà typés (R6), donc **sans aide ni prédicat** : leur ensemble n'est gardé que par le type (lecture de 1b.22 : l'ensemble est porté par l'`assert` de l'aide R5 ; pas d'aide, pas d'`assert`). Noté dans « Découvert ».

- **`qu`, ensemble = ligne ∪ `hj`, `nl`** = `jo` ∪ les 8 descripteurs à `LY` (`qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx` — « 8 descripteurs portent LY », 1b.32). Preuve par le code : `jo:ldown` (`qh`, `nl`, `os`, `hj`, `pm`, `mx` via `pico8_jo_ldown_dispatch`) et `nr:ldown` (`nr`, `fr`) sont les seuls producteurs ; `hj`, `nl` passent à `me == "ly"` par `d_` (`dq = "ly"` de `qh`). `hj` est exercé sur l'ancien code par `test_pico8_mx` (lent), `nl` par la caractérisation. **`ga` exclue** : `pico8_jo_ldown_dispatch` et `GV` sans `LY`, jamais `me == "ly"` (H4) ; le test d'arrêt porte un `ga`. **Prédicat `jc_is_endpoint_class(cls)` : `cls->methods[PICO8_METHOD_LY] != NULL || cls == pico8_jo_struct_class()`** (copie réduite de `family_is_target_class`) — propriété du descripteur parce que `DATA_ENGINE_SRCS` lie `slipway.c` sans `planet_family_trade.c` (`nr`, `fr`, `mx`) ; `pico8_jo_struct_class` vient de `planet.c`, lié partout.
- **`fo`, ensemble = ligne ∪ `fv`, `nl`, `os`, `pm`, `mx`** = porteurs de `pico8_bv_dispatch` (`g_`, `fv`, `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`, 1b.23d) ∪ `mz` (`mz:hq` appelle `bv`) ∪ `jc` (`jc:hq`). **`jo` exclue** (le cart ne l'écrit jamais ; `test_draw:695` réécrit), comme `bv_et`, `jc_too_far`, `jc_mouse_point`. Les cinq hors ligne sont exercés sur l'ancien code par la caractérisation (`hq` d'un porteur neuf au milieu d'un `jc` isolé, forme de `test_bv_et_members_by_name`). **Prédicat `jc_is_fo_holder_class(cls)` : `cls->methods[PICO8_METHOD_HQ] == pico8_bv_dispatch || cls == pico8_jc_struct_class() || cls == pico8_mz_struct_class()`** — union de `jo_is_fo_holder_class` (1b.23d) et de `bv_et_is_holder_class` (1b.22) ; `help.c`/`planet_family_trade.c` non liés par `DATA_ENGINE_SRCS`, `wormhole.c`/`probe.c` liés.
- **`re`** : ligne = `jo` ∪ `LY` ∪ `jc_mouse_point` (exactement les 10), sans ajout ; `ga` a `GV` mais un `kq` vide (`b(0,0,-1,-1)`), jamais survolé (H26, sans effet puisque `re` n'a pas d'`assert`).
- **`et`** : ligne = `jc`, `jc_too_far`, `jo` ; le cart peut aussi viser toute planète de `cg.np` (`qh` et descendance) : **accepté par le type**, rien à élargir (pas d'aide). Écrit par `hq` : `self->et = pico8_jc_value_instance(et);` puis `if (self->et != NULL) { assert(tag STRUCT, message inchangé); pico8_entity_set_field(self->et, "fo", pico8_value_ref(self)); }` (H7 : `ce` ne rend que REF).
- **Écartés** : un prédicat partagé entre fichiers (1b.13 point 2 : jamais entre fichiers) ; `pico8_jc_t *` pour `fo` (`g_`, `mz`, planètes y écrivent) ; un `assert` d'ensemble sur les écritures directes de `re`/`et` (aucune règle ne le demande, et `et` vise légitimement des planètes hors ligne).

### `od`, `mt` : `pico8_fixed_t` + `has_` ; `p_` : `int` + `has_p_` (G)

- **`od`** : nil jusqu'au premier `hq` (lu nil par `check_fresh_jc`, par `d_` sur le `jc` de `join` dans `test_pico8_mx`), puis `dj` : `self->od = od; self->has_od = 1;`.
- **`mt`** : `self.qu.mt or self.re.mt` — fixe (`fr.mt` = 1, le seul porteur) ou nil. `hq` garde `mt_val` (lu par nom, inchangé) puis **`jc_take_fixed(mt_val, &self->mt, &self->has_mt)`** (lecture de 1b.14a : valeur lue par nom sur une autre entité, aide par valeur) ; `self->fa = self->has_mt ? 2 : 4` (un fixe présent est vrai en Lua, 0 compris). `jc:ce` compare au `mt` lu par nom des autres `jc` : **`jc_ctx.self_mt = self->has_mt ? pico8_value_fixed(self->mt) : pico8_value_ref(NULL)`** (nil == nil en Lua ; pas d'emballage public : un seul genre, un seul site).
- **`p_`** : nil au départ, `true` par `ey` (direct : `self->p_ = 1; self->has_p_ = 1;`) ou par `d_` (écrivain) ; forme de `g_.p_`/`jo.p_`.
- R2 : `fa`, `fp`, `ik`, `dw`, `nh` jamais nil, sans indicateur (`stat?` d'homonymes sans `lu`, 1b.7). `fp` → `int` toujours 0, lu `BOOL` 0 **présent** (`d_` : `fp or 0`).

### Aides R5 (forme 1b.13/1b.15), toutes `static` dans `slipway.c` (G)

`slipway.c` porte trois structs ; leurs genres ne se recoupent pas (`jc_too_far.fo` : `ref:pico8_jc_t`, `jc_mouse_point.ra` : `ref:autre`), donc aucune aide existante n'est partagée ni renommée (lecture de 1b.23b sans objet). Nil = `value.kind == PICO8_VALUE_REF && value.as.ref == NULL` (genre **et** pointeur).

| aide | membres | état nil puis sortie | accepté (`assert`) | message (préfixe) |
|---|---|---|---|---|
| `jc_take_endpoint(value, pico8_struct_header_t **out)` | clé `qu` | `*out = NULL` | REF, tag `STRUCT` **puis** `jc_is_endpoint_class(cls)` | `"jc.qu: a key neither nil nor a planet a slipway starts from (jo, qh, nl, os, hj, pm, nr, fr, mx) is off its census row (ref: to those planets) -- …"` |
| `jc_take_holder(value, pico8_struct_header_t **out)` | écrivain `fo` | `*out = NULL` | REF, tag `STRUCT` **puis** `jc_is_fo_holder_class(cls)` | `"jc.fo: a write neither nil nor what hangs fo on a slipway (an object whose hq runs bv: g_, fv, qh, nl, os, hj, pm, nr, fr, mx; an mz; another jc) is off its census row (ref: to those holders) -- …"` |
| `jc_take_bool(value, int *out, int *has)` | écrivain `p_` | `*out = 0; *has = 0` | `BOOL` ; `*has = 1` en dernier | `"jc.p_: a write neither nil nor a boolean is off its census row (bool) -- …"` |
| `jc_take_fixed(value, pico8_fixed_t *out, int *has)` | `mt` dans `hq` | `*out = 0; *has = 0` | `FIXED` ; `*has = 1` en dernier | `"jc.mt: an mt read off qu or re neither nil nor a fixed is off its census row (fixe) -- …"` |

Chaque message finit par `"a field under no rule of plan-3 section 5: stop, do not widen the member"`, et la phrase `is off its census row` tient sur une ligne du source (preuve par `grep -c`). Un `false` à union nulle est un `false` présent pour `p_`.

**Écrivain `pico8_jc_set_field`** : 2 branches dans l'ordre du typedef (`fo` puis `p_`), `if (strcmp(name, "fo") == 0) { jc_take_holder(value, &jc->fo); return 1; }`, `p_` par `jc_take_bool(value, &jc->p_, &jc->has_p_)`, puis `return 0`. Commentaire de contrat (« `fo` (bv, and jc:hq of another jc) and `p_` (d_): the only names a generic writer ever writes on a `jc` ») gardé ; `mz` peut y être ajouté (« bv, of a probe, a planet or an mz, and jc:hq… »).

**Boucle de clés de `pico8_jc_struct_new`** (lecture 1b.10/1b.29) : `if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "qu") == 0) { jc_take_endpoint(overrides->entries[i].value, &self->qu); } else { assert(0 && "jc: the only measured construction key is qu (jo:ldown, nr:ldown); another would be a field under no rule of plan-3 section 5 -- stop, do not add a member"); }`, **texte inchangé**.

### R1–R9 appliquées

| règle | sur `jc` |
|---|---|
| R1 | Tableau de recensement ; nom et place gardés ; `has_od`, `has_mt`, `has_p_` juste après leur membre, commentaire court (« 0: `od` is nil, and is then never read », forme `probe.h`). **Aucune inclusion ajoutée à `slipway.h`** (H18). **Inclusions ajoutées à `slipway.c`** : `pico8_planet.h` (`pico8_jo_struct_class`), `pico8_wormhole.h` (`pico8_mz_struct_class`) ; rien d'autre. |
| R2 | Les 3 indicateurs de la ligne ; pointeurs sans indicateur. |
| R3 | `pico8_jc_struct_new` : après `pico8_ke_struct_init` et `me`, dans l'ordre du typedef : `nu = g_jc_nu` (même REF), `fa = pico8_fixed_from_int(2)`, `fp = 0`, `ik = pico8_fixed_from_int(48)`, `dw = PICO8_FIXED_LIT(0x00000106, 0.004)` (littéral gardé, H23), `k_ = g_jc_k_`, `nh = pico8_fixed_from_int(20)`, `qu`, `ts`, `hg`, `re` à NULL, `od` 0 / `has_od` 0, `mt` 0 / `has_mt` 0, `et`, `fo` à NULL, `p_` 0 / `has_p_` 0 ; puis les clés, `pico8_jc_pq`, `pico8_ke_struct_pq`, `PICO8_MEMBER_CENSUS_SEE`. Littéraux, pas de table (1b.14b). |
| R4 | `pico8_jc_field` : 16 branches `strcmp` dans l'ordre du typedef, puis `return pico8_ke_field(self, name, out_value);` ; petites aides `static` de lecture permises (forme `g__ref_field`/`g__fixed_field`). `fa`, `ik`, `dw`, `nh` : toujours `FIXED` ; `fp` : toujours `BOOL` ; `od`, `mt` : `has_` = 0 → `return 0`, sinon `FIXED` ; `p_` : `has_p_` = 0 → 0, sinon `pico8_value_bool(p_)` ; pointeurs (`nu`, `k_`, `hg` table, `ts` liste, `qu`, `re`, `et`, `fo` struct) : NULL → 0, sinon REF. `*out_value` intact sur 0. |
| R5 | Écrivain, clé, `mt` : ci-dessus. |
| R6 | `pq` : `self->ts = ts; self->hg = hg;`. `hq` : `self->re = hovered` ou `(pico8_struct_header_t *)point` (local `re_val` retiré ou gardé pour le seul `re`), `od` via `pico8_dj(self->qu, self->re)`, `mt` par `jc_take_fixed`, `et` et `fo` de la cible (plus haut), `fa`. `ey` : `self->p_ = 1; self->has_p_ = 1;`, `self->nh = pico8_fixed_from_int(1); self->re = re;`, `qu = self->qu`. `ce` : `qu = self->qu; re = self->re;`, `self_mt` (plus haut). `eb` : `if (jo_instance == self->qu && self->re != NULL) return pico8_value_ref(self->re); return pico8_value_ref(self->qu);` (le `A and B or C` de 1a garde son commentaire). `ev` : `self->qu != self->re`. **`slipway_display.c`** : `ch` (`self->hg`, `pico8_list_add(self->ts, …)` : **`PICO8_REF_AS_LIST` retiré**), `lg` (`self->qu`, `self->re`), `kj` (`nh = self->nh`, `self->nh = pico8_fixed_add(nh, 1)`, `self->k_`, `et_or_fo = self->et != NULL \|\| self->fo != NULL`, `mt_truthy = self->has_mt`, `ts = self->ts` sans `PICO8_REF_AS_LIST`, **`self->fo = NULL`**). `pico8_jc_display_value_instance` et `pico8_jc_display_value_fixed` supprimées (sans client, `-Werror`) ; `#include "pico8_ref_audit.h"` de `slipway_display.c` retiré s'il n'a plus d'usage (H25). Tests : `test_pico8_g_.c:494` → `((pico8_jc_t *)jc)->re = re;`. |
| R7 | Forme `offsetof` de 1b.9 : la `members[]` locale du lecteur supprimée avec sa boucle et son `size_t i` ; **plus aucun `offsetof` dans `slipway.c`**, `#include <stddef.h>` retiré (précédents 1b.31, 1b.33, 1b.34 ; H17). Pas de table de défauts ni de fille : R7 de 1b.14a vacante. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_jc_t)` gardé ; **le `PICO8_MEMBER_CENSUS_STOP()` de `test_draw` (`:694`) gardé tel quel**, même si l'écriture qu'il arrêtait est désormais sur la ligne ; `tools/member_census.txt` non édité. |
| R9 | 43 → 27. |

### Interaction avec `g_` (1b.32), `jo` (1b.23d), `jc_too_far` (1b.13), `mz`/`ui` (1b.36/1b.37)

- **`g_.et`** (`pico8_struct_header_t *` + `has_et`, `g__is_target_class` ∋ `pico8_jc_struct_class()`) : **inchangé** ; la cible `jc` reste un pointeur d'en-tête, l'accesseur public ne change pas. Ce que 1b.35 change sur ce chemin : l'écriture `pico8_entity_set_field(et, "fo", g_)` de `bv` atteint maintenant `jc_take_holder`, qui admet `g_` (et `fv`) par `methods[HQ] == pico8_bv_dispatch`. `dl`/`d_` lisent `et`/`od`/`fp`/`ik` d'un `jc` par nom : mêmes valeurs. `test_pico8_g_` construit ses `jc` par `a_slipway` (ligne `:494` réécrite) et y fige `fo` = la sonde (`test_life`) : sur le chemin, R-d.
- **`jo.fo`** (`jo_take_holder` ∋ `jc`) et **`jc_too_far.fo`** (`jc_too_far_take_jc`, `cls == pico8_jc_struct_class()`) : `jc:hq` continue d'y écrire `self` ; inchangés.
- **`mz`** : `mz:hq` → `bv` → `jc.fo = mz` (`test_pico8_mz:403-450`, ligne `fo`) ; `mz.et` (`ref:pico8_jc_t`) et `mz` elle-même restent à 1b.36. **`ui`** : `lheld`/`o_` pointent vers des `jc`, 1b.37.

### Tests touchés (G)

- **`tests/test_pico8_jc.c`** : (a) **ligne `:695`** : `pico8_value_ref(origin)` → `pico8_value_ref(old)` (un `jc`, sur la ligne ; `kj` ne dépend que de la vérité de `fo`, qu'il éteint ensuite : `DRAW_HASH` inchangé, H13), commentaire de `test_draw` inchangé sauf si sa lettre devient fausse ; (b) **ajouts** : caractérisation, test d'arrêt, `JC_MEMBERS_HASH`, aides de test (valeur hors ligne dans une globale, forme `g_planet`/`g_off_row`), inclusions `pico8_planet.h`, `pico8_planet_family.h`, `pico8_planet_family_trade.h` (constructeurs et accesseurs de classe), deux appels `in_fresh_child` dans `main`. Aucune autre ligne existante, aucune valeur figée ne change.
- **`tests/test_pico8_g_.c:494`** : `((pico8_jc_t *)jc)->re = pico8_value_ref(re);` → `((pico8_jc_t *)jc)->re = re;`, rien d'autre (preuve rouge §6, R-d).
- Aucun autre fichier de `tests/` (`test_pico8_ui.c:2315` appelle `pico8_jc_hq` sur un `pico8_jc_t *`, inchangé ; aucune boucle sentinelle).

### Revue des tests de régression, mécanique

**Liste** (reprise de 1b.30–1b.34) : les tests nommés au §6 (les 17 de régression) et dans les lignes 1a : `tests/test_pico8_<nom>.c` pour `built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`, `bg`, `entity_kind`, `hj`, `jc`, `jo`, `jo_spec_defaults`, `js`, `mo`, `mz`, `nl`, `os`, `pm`, `qh`, plus `tests/test_class_coverage.c`. Tous existent.

**Greps faits pour ce PRD** (`M = nu|fa|fp|ik|dw|k_|nh|qu|ts|hg|re|od|mt|et|fo|p_`) :

- accès direct : `grep -nE '(->|\.)(M)\b'` sur les fichiers qui nomment `pico8_jc_t` → aucun dans la liste (seul `test_pico8_g_:494`, hors liste §6 mais TU de 1a) ;
- par nom `"(M)"` ou `jc`/`lheld`/`slipway`/`"oe"` : `slipway_drag` (`qu`, `od` du `jc` tenu, `oe`), `playability` (étape 4 : `jo:ldown` → `jc({qu=self})`, `d_` débite `od`), `mz` (`qu`, `re` d'un `jc`, `fo` = `mz` après `hq`), `bg` (`lheld` du rejeu), `jc` ; ailleurs homonymes (`hj`, `jo`, `nl`, `os`, `pm`, `qh` : `oe`/`p_` de planètes) ;
- fixtures : `phase4_playthrough_events.json` (qui bâtit une voie à la frame 201) sert `playability` et `built_planet_panel_nil_row` ; `slipway_drag_events.json` sert `slipway_drag`. Les autres rejeux ne sont pas connus pour bâtir un `jc` (H15).

| # | test | accès / chemin | diff ? | preuve rouge |
|---|---|---|---|---|
| R-a | `slipway_drag` | le rejeu bâtit un `jc` : clé `qu` (`jc_take_endpoint`), `hq` à chaque frame (`re`, `od`, `mt`, `et`), `ey` (`re`, `nh`), `d_` lit `od`/`et` par nom ; `"qu"`, `"od"` lus par le test : **sur le chemin** | non | **Obligatoire.** Bug d'origine **2d7021f** remis (nil à la place de l'entité de `jz("gv")` dans `ui:bh`, recette de 1b.32 R-d) → rouge (`oe_count(origin) == 1`) ; restauré → vert. |
| R-b | `playability` | étape 4 (frame 201) : `jo:ldown` → `jc`, relâché sur une planète `ly`, `d_` débite `od` : **sur le chemin** | non | **Obligatoire.** Bug de référence de 1a.17 (constructeur de `jo` appelant `pico8_ke_struct_pq` deux fois, recette de 1b.32 R-b / 1b.34 R-f) → rouge ; restauré → vert. |
| R-c | `jc` (TU de la classe) | **`:695` réécrite** (`jo` → `jc` dans `fo`) ; toute la classe | **oui**, une ligne (plus les ajouts) | **Obligatoire.** (1) La ligne réécrite passe **sur l'ancien code** avec `DRAW_HASH` inchangé (ligne « Runs », H13). (2) Mutation `et_or_fo = self->et != NULL` (le `fo` ignoré par `kj`) → `test_draw` rouge (`DRAW_HASH`) ; (3) `self->fo = NULL` de `kj` retiré → rouge (`!has(old, "fo")`) ; restaurés → vert. |
| R-d | `g_` (TU de 1a.27) | **`:494` réécrite** (écriture directe de `re`) ; `test_life`, `test_bv_et_*` passent par `a_slipway` et par `jc_take_holder` (`fo` = la sonde) | **oui**, la seule forme de l'écriture | **Obligatoire** (1b.23a2). La ligne réécrite mise sous `if (0)` → `test_life` rouge (`fold_hq(…, middle, 64, 3)` ou `get_ref(jc, "fo") == a`) ; restaurée → vert. |
| R-e | `mz` (TU de 1a) | `mz:hq` → `bv` vise un `jc` → `jc.fo = mz` par `jc_take_holder` ; `qu`/`re` d'un `jc` lus par nom : **sur le chemin** du prédicat | non | **Dans le budget seulement**, par la mutation 7b (`mz` retirée du prédicat) : `test_pico8_mz` rouge (fils qui avorte) ; sinon « non lancé » au journal. |
| R-f | `built_planet_panel_nil_row` | rejoue `phase4_playthrough` : sur le chemin **s'il passe la frame 201** (H15) | non | **Dans le budget seulement.** Bug d'origine **3b8ba2c** remis (recette de 1b.34 R-c) → rouge ; restauré → vert. Sinon « non lancé ». |
| — | `bg` | rejeu `playthrough`, `lheld` compté par genre (`jc` en « autre ») | non | aucune ; vert sans modification. |
| — | `empty_planet_cleared`, `cheat_code`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`, `entity_kind`, `hj`, `jo`, `jo_spec_defaults`, `js`, `mo`, `nl`, `os`, `pm`, `qh`, `test_class_coverage` | homonymes seuls, ou aucun `jc` connu dans le scénario ; si un rejeu en bâtit un, l'implémenteur le note (précédent 1b.32 `empty_planet_cleared`) | non | aucune. |

### Preuves (sortie attendue sur l'arbre final)

**Sortie de `jc`** :

- `awk '/^typedef struct \{/{b=""} {b=b $0 "\n"} /\} pico8_jc_t;/{printf "%s", b}' src/pico8_slipway.h | grep -c pico8_value_t` : **0** ;
- `make value-members` : **27**, `grep -c pico8_jc_t` : **0** ; seules les 16 lignes `pico8_jc_t` disparaissent ;
- `grep -c 'offsetof' src/pico8_slipway.c` : **0** (6 lignes avant) ; `grep -rn 'offsetof(pico8_jc_t' src/` : **vide** ; `grep -n 'stddef' src/pico8_slipway.c` : **vide** (H17).

**Lecteur, écrivain, aides** :

- `grep -nE '(->|\.)(M)\.(kind|as)' src/pico8_slipway.c src/pico8_slipway_display.c` : **vide** ;
- `grep -n 'pico8_value_t \*' src/pico8_slipway.c | grep -v 'pico8_value_t \*out_value)'` : **vide** (1 ligne avant : la lecture `*(const pico8_value_t *)` du lecteur) ;
- `grep -c 'off its census row' src/pico8_slipway.c` : **6** (2 avant : `jc_too_far_take_jc`, `pico8_jc_mouse_point_take_ra`) ;
- `grep -n 'jc_take_\|jc_is_' src/pico8_slipway.c` : 4 aides et 2 prédicats définis, chacun appelé au moins une fois ; `jc_is_endpoint_class` = `LY` ∪ `jo`, `jc_is_fo_holder_class` = `bv_dispatch` ∪ `jc` ∪ `mz`, rien d'autre ;
- `grep -n 'has_od\|has_mt\|has_p_' src/pico8_slipway.h` : chacun juste après son membre ;
- `git diff -U0 src/pico8_slipway.h | grep '^+#include'` : **vide** ; `git diff -U0 src/pico8_slipway.c | grep '^+#include'` : `pico8_planet.h`, `pico8_wormhole.h`, rien d'autre ;
- `grep -n 'PICO8_REF_AS_LIST' src/pico8_slipway_display.c` : **vide** (2 avant) ; `grep -n 'pico8_jc_display_value_instance\|pico8_jc_display_value_fixed' src/` : **vide**.

**Périmètre du diff** :

- `git diff --stat HEAD -- src/ ':!src/pico8_slipway.h' ':!src/pico8_slipway.c' ':!src/pico8_slipway_display.c' tools/ Makefile` : **vide** ;
- dans `tests/`, seuls `test_pico8_jc.c` (ajouts, plus la ligne `:695`) et `test_pico8_g_.c` (ligne `:494`) changent : `git diff -U0 tests/test_pico8_jc.c | grep -c '^-[^-]'` : **1** ; `git diff -U0 tests/test_pico8_g_.c | grep -c '^-[^-]'` : **1**.

### Empreinte et découpage

Ligne : 30 (14, 16). **Remesurée pour ce PRD** avec la formule du plan : **31 (14, 17)** — `src/` : `bootstrap.c`, `economy_tiers.c`, `entity.c`, `entity_geom.c`, `particle.c`, `planet.c`, `planet_display.c`, `planet_family_trade.c`, `probe.c`, `slipway.c`, `slipway.h`, `slipway_display.c`, `slipway_display.h`, `ui.c` ; `tests/` : `bg`, `fq`, `fr`, `fv`, `g_`, `ga`, `hj`, `ie`, `jc`, `mh`, `mz`, `nf`, `nl`, `os`, `pj`, `qh`, `ui`. Majorant : trois fichiers accèdent directement (`slipway.c`, `slipway_display.c`, `test_pico8_g_.c`), les `.h` n'y entrent que par leurs commentaires, le reste par des noms courts partagés (`fa`, `od`, `et`, `fo`, `p_`, `re`…). **Fichiers modifiés attendus : 5** — `src/pico8_slipway.h`, `src/pico8_slipway.c`, `src/pico8_slipway_display.c`, `tests/test_pico8_jc.c`, `tests/test_pico8_g_.c` — plus `docs/plan-3.md`. Empreinte réelle comptée au commit (H22).

**Découpage (G) : aucun, un seul commit.** Évalué sur deux découpes :

- (a) **1b.35a** « défauts et ce que `pq`/`hq`/`ey` réécrivent » (`nu`, `fa`, `fp`, `ik`, `dw`, `k_`, `nh`, `ts`, `hg`, `od`, `mt` ; 43 → 32) puis **1b.35b** « les REF à plusieurs cibles et l'écrivain » (`qu`, `re`, `et`, `fo`, `p_` ; 32 → 27) ;
- (b) le seul écrivain d'abord (`fo`, `p_`, et la réécriture de `test_draw`), le reste ensuite.

Écartées parce que : (1) les deux moitiés touchent les **mêmes** fonctions — la `members[]` du lecteur (qui ne tomberait qu'au second commit, avec une forme mixte branches typées + repli `offsetof` qu'aucun commit final ne garde), `pico8_jc_struct_new`, `pico8_jc_hq` (qui écrit `re`, `od`, `mt`, `et`, `fa` et le `fo` d'autrui) et `pico8_jc_kj` (`nh`, `k_`, `et`, `fo`, `mt`, `ts`) ; (2) **le coût de test double au lieu de baisser** : chaque commit change le constructeur, donc chacun doit passer `make e2e`, `make sanitize-net` et `make class-coverage-net` (environ 12–15 min à eux trois, 1b.13 : 414 s + 317 s), et les preuves §6 (R-a, R-b sur le chemin de `qu`/`od`/`re` **et** de `fo`/`p_`) seraient faites deux fois ; (3) le travail propre à l'étape est peu coûteux : `test_pico8_jc` tourne en ~11 s, un mutant ne recompile que `slipway.c` ou `slipway_display.c`, le diff touche 5 fichiers (1b.32 : 23 membres, 6 fichiers, un commit) ; (4) la règle de découpage du plan (« une ligne de plus de ~40 fichiers est découpée dans la liste elle-même ») ne s'applique pas à 30/31. Si le budget est dépassé, l'implémenteur suit l'ordre de priorité de « Vérification » et note ce qui n'a pas été lancé ; ce n'est pas une raison de découper après coup.

### Hypothèses (rien n'a été lancé)

- **H1** : 1b.34 commitée avant le départ ; `make value-members` rend 43 au départ (aucune ligne `pico8_js_t`, 16 lignes `pico8_jc_t`), 27 à la fin.
- **H2** : 1b.34 ne touche que `ui_button.[ch]`, `test_pico8_js.c` et le plan : aucun fichier de cette étape. Si une note « Découvert à 1b.34 » change une règle citée ici, elle prime et l'écart est noté (ce n'est pas un arrêt).
- **H3** : hors des fils de `must_abort` et hors `test_draw:695` (réécrite), aucun site (cart, C, TU, 15 rejeux) n'écrit dans `jc.fo` autre chose que nil ou un porteur de `bv`/`mz`/`jc`, dans `p_` autre chose que nil ou un booléen, ni ne passe en clé `qu` autre chose que nil, un `jo` ou une planète à `LY`. Démentie : arrêt « champ sans règle », sans élargir le membre.
- **H4** : `ga` n'est jamais `qu` (jamais `me == "ly"` : hors `cg.jo`, pas de `hu`, pas de `bv`) ; `hj`, `nl` le sont (`d_` → `dq = "ly"`) ; `nr`/`fr` neufs acceptent `ldown` (`oe` vide, `nr:gv({})` vrai).
- **H5** : sur l'ancien code, `hq` d'un `g_`, `fv`, `mz`, `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx` neufs mené au milieu d'un `jc` isolé écrit ce porteur dans `jc.fo` (forme de `test_bv_et_members_by_name`, qui l'a fait pour la cible jetable) ; pour `mz`, par la branche `jp(cg.jc, …)` de `bv` (sans seuil `dw`).
- **H6** : un `jc` isolé se construit **sans accès direct** : deux `jo` nus (`p` par clé) à ½ unité l'un de l'autre, loin des autres planètes (forme `a_slipway` de `test_pico8_g_`), le `jc` de `qu` = le premier, `re` posé par `hq_over(jc, second, mp au second)` (`jo` a `gv`) ; len² = 0,25 sous le seuil 0,5 de `space folding` sans techno (pas de `{qs}`), aucune planète ni autre `jc` sur le segment (`et` nil) ; le milieu est plus loin que le `dw` de chaque porteur de toute planète. Sinon l'implémenteur déplace les points et le note.
- **H7** : `pico8_jc_ce` ne rend que `REF` (nil, `&g_jc_too_far`, un `jc` de `qu.oe`, une entrée de `cg.np`/`cg.jc`) ; `re` et `et` ne tiennent jamais `false` : aucun indicateur.
- **H8** : `mt` lu sur `qu` puis `re` est nil ou `FIXED` (`fr.mt` = 1, seul porteur ; `jc_mouse_point` et les autres planètes n'ont pas de `mt`).
- **H9** : le build `-DPICO8_MEMBER_CENSUS` compile avec un `pico8_jc_t` sans `pico8_value_t` ; `-fsyntax-only -DPICO8_MEMBER_CENSUS` sur `slipway.c`, `slipway_display.c`, `test_pico8_jc.c` et `test_pico8_g_.c` suffit si le build complet n'est pas lancé (précédents 1b.29–1b.34).
- **H10** : `test_pico8_jc` reste sous 30 s avec deux fonctions de plus (11 s à 1b.13 ; la caractérisation rejoue au plus le rejeu `DRAG`, comme `world()`). Démentie : on allège, on ne déplace pas le TU, le `Makefile` ne change pas.
- **H11** : `pico8_ob_table_set_name(b, "qu", pico8_value_ref(NULL))` garde la clé (H11 de 1b.29), donc le cas « clé nil » atteint son aide.
- **H12** : sur l'ancien code, chaque cas du test d'arrêt se termine sans `SIGABRT` (l'écrivain et la boucle de clés recopient tout ; `pico8_jc_new` ne relit pas `qu`) ; un cas qui avorte déjà est retiré du « rouge d'abord » et noté.
- **H13** : `test_draw` avec `old` au lieu de `origin` à `:695` rend le même `DRAW_HASH` sur l'ancien code (`kj` ne lit que la vérité de `fo`, `et` de `old` est nil, `fo` est éteint avant `fold_jc`).
- **H14** : `test_pico8_g_.c:494` réécrite compile sur l'arbre final et garde tous les hash de `test_pico8_g_`.
- **H15** : les preuves §6 rougissent : 2d7021f (R-a), double `pq` de `jo` (R-b), mutations de `kj` (R-c), `if (0)` sur `g_:494` (R-d) ; pour R-e, R-f dans le budget. `built_planet_panel_nil_row` et les rejeux hors `slipway_drag`/`playthrough` ne sont pas connus pour bâtir un `jc`.
- **H16** : `make sanitize-net` propre (aucune allocation nouvelle ; `ts`/`hg` toujours alloués par `pq`, TROU-7 inchangé).
- **H17** : `-Werror` muet : `<stddef.h>` et `size_t i` retirés de `slipway.c` (`size_t`, `NULL` viennent de `<stdlib.h>`/`<string.h>`) ; `pico8_jc_value_instance` garde des clients (`pico8_jc_entity_instance`, `et`) ; les 4 aides et 2 prédicats appelés ; les deux aides de `slipway_display.c` sans client supprimées. Sinon ce qui reste est noté.
- **H18** : `slipway.h` compile sans nouvelle inclusion (`ke.h` → `struct.h` → `ob.h` → `list.h`) ; `pico8_planet.h`, `pico8_wormhole.h` inclus par `slipway.c` sans cycle ni collision ; tout binaire qui lie `slipway.c` lie `planet.c`, `wormhole.c`, `probe.c` (`ENGINE_SRCS`, `DATA_ENGINE_SRCS`, `main_raylib`).
- **H19** : aucune valeur figée ne bouge : les 12 hash de `test_pico8_jc` (`DRAG_HASH` … `FAR_MEMBERS_HASH`), ceux de `test_pico8_g_` (`LIFE_HASH`, `BV_ET_MEMBERS_HASH`…), `test_pico8_mz`, `test_pico8_ui`, `test_pico8_bg`, `test_pico8_mx`, `test_pico8_nr`, et les 15 empreintes du filet.
- **H20** : `make class-coverage-net` identique à la référence (le compte d'instanciations de `jc` ne change pas).
- **H21** : revue des TU lents confirmée par l'implémenteur (grep ci-dessus) : aucune lecture nil par nom d'un non-pointeur de `jc` hors `od`, `mt`, `p_`.
- **H22** : empreinte réelle comptée au commit.
- **H23** : `float-audit`, `constants-check`, `float-grep`, `float-guard` inchangés (`PICO8_FIXED_LIT(0x00000106, 0.004)` et `(0x0000147a, 0.08)`, `(0x00018000, 1.5)`… gardés à l'identique ; `pico8_jc_display_to_int` intouché).
- **H24** : le crochet `PICO8_MEMBER_CENSUS_SEE` gardé, la table régénérée perdrait exactement les 16 lignes `pico8_jc_t` (non lancé, R8).
- **H25** : les deux sites `PICO8_REF_AS_LIST` de `slipway_display.c` disparaissent avec le typage de `ts` (§5 « les sites de `pico8_ref_audit.h` disparaissent au fil de l'eau ») ; `make ref-audit` (diagnostic hors suite) non lancé ; celui de `slipway.c:363` (`qu.oe`, lu par nom) reste.
- **H26** : `ga` (qui a `GV`) n'est jamais survolé (`kq` vide), donc jamais `re` ; sans effet sur le code (pas d'`assert` sur `re`).
- **H27** : `pico8_jc_ey_dispatch`, `pico8_jc_eb_dispatch`, `pico8_jc_hq` (appelé par `test_pico8_ui:2315`) gardent leurs signatures.

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Les nouveaux tests n'accèdent à aucun des 16 membres directement : ils compilent sur les deux formes.
- **(G) Où : `test_pico8_jc`**, TU de la classe, rapide, dans `CENSUS_UNIT_TESTS`, lié par `ENGINE_SRCS` ; il a `world`, `start_game`, `start_from_title`, `the_slipway`, `build_jc`, `hq_over`, `vec_ref`, `vec_ref_fixed`, `vec_of`, `call`, `lookup`, `has`, `get`, `get_ref`, `must_fixed`, `bool_state`, `re_kind`, `et_kind`, `holder_kind`, `fold_value`, `fold_jc`, `first_with_me`, `check_fresh_jc`, `must_abort`, `in_fresh_child`, et inclut déjà `probe.h`, `help.h`, `wormhole.h`, `bg.h`, `ui_button.h` (précédent de lieu : `test_too_far_members_by_name`, 1b.13).
- **Caractérisation dirigée `test_jc_members_by_name`**, par `in_fresh_child` après `test_too_far_writer_stops_off_row`, hash **`JC_MEMBERS_HASH`**. Lecture par le **lecteur brut** (`pico8_jc_struct_class()->field`, témoin `pico8_value_fixed(0x1234)`) **et** par `pico8_entity_get_field`, qui doivent concorder (retour, genre, valeur, pointeur) ; adresses jugées par `assert`, jamais pliées (rang dans `hv`, ou rang dans une liste locale de porteurs, forme `fold_bv_et_fo`).
  1. **Neufs** : `pico8_jc_new(NULL)`, `pico8_jc_new` d'une table vide, `build_jc(ly)` et `ldown` de `ly` (`world()`). Les 16 noms pliés : `nu`, `k_` au **même pointeur** d'un `jc` à l'autre ; `ts`, `hg` présents, distincts d'un `jc` à l'autre ; `fp` `BOOL` 0 **présent** ; `fa` 2, `ik` 48, `dw` `DW_BITS`, `nh` 20 ; `qu` absent sans clé ; `re`, `od`, `mt`, `et`, `fo`, `p_` absents.
  2. **`qu` par producteur** (H4) : `jo` (`first_with_me("jo", "ly")`), puis un `qh`, `nl`, `os`, `hj`, `pm`, `mx` neufs passés à `ly` par `pico8_ke_ks` (sauf `pm`, né `ly` : `assert`), et un `nr`, `fr` neufs ; pour chacun `mv(X, "ldown")` rend un `jc` dont `qu` = X (même pointeur, classe attendue) ; puis `build_jc(X)` pareil ; clé `qu` à `REF NULL` → absent (H11).
  3. **`hq`, `ey`** sur le `jc` isolé (H6) : `re` = la planète survolée (même pointeur), `od` `FIXED` présent (= `dj`), `mt` absent et `fa` 4 ; le même depuis un `fr` (clé `p`) : `mt` `FIXED` 1 présent, `fa` 2 ; `hq_over(…, NULL, mp lointain)` : `re` = un `jc_mouse_point` (`re_kind`), `et` = le `{qs}` et son `fo` = ce `jc` ; **`et` planète** : un troisième `jo` nu au milieu du segment → `et` = lui, son `fo` = ce `jc` ; **`et` `jc` par croisement** : un second `jc` isolé dont le segment croise le premier, `mt` nil des deux → `et` = le premier (branche `cg.jc` de `ce`, sous-tend la mutation 12) ; `et` `jc` par `qu.oe` : déjà `test_hq`. `ey(NULL)` : `p_` `BOOL` 1, `re` inchangé ; `ey` vers une planète : `nh` 1, `re` = elle (déjà `test_ey`, repris seulement s'il ne coûte pas un rejeu de plus).
  4. **`fo` par porteur** (H5) : `g_`, `fv`, `mz`, `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx` neufs mènent `hq` au milieu du `jc` isolé (`mv(X, "hq", 2, mp, rp (2,2))`) → `fo` = X ; un autre `jc` par `hq` (`twin` sur la paire) → `fo` = lui ; puis **aller-retour par nom** : `pico8_entity_set_field(jc, "fo", X)` pour les 12 → même pointeur, `cls->set_field` rend 1 ; `REF NULL` → absent, témoin intact ; **REF à union nulle** (`memset`, `kind = REF`) → absent ; `kj` → absent.
  5. **`p_`** par `pico8_entity_set_field` : `true` → `BOOL` 1 ; `false` → `BOOL` 0 présent ; **`false` à union nulle** → `BOOL` 0 présent ; `REF NULL` après `true`, après `false`, sur nil → absent, témoin intact.
  6. **Noms sans branche** : `cls->set_field` rend 0, sans rien toucher (valeur relue identique), pour les 14 autres noms (écriture d'un fixe 9) et pour `""`, `"FO"`, `"fo "`, `"p"`, `"has_p_"`, `"has_od"`, `"has_mt"`.
  7. **Intrus** sur le lecteur : `""`, `"q"`, `"qu "`, `"QU"`, `"has_od"`, `"has_mt"`, `"has_p_"`, `"header"`, `"ke"`, `"nu_"` → 0, témoin intact.
  8. Tout plié dans **`JC_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ (valeur imprimée par `check`), figé ensuite.
- **Test d'arrêt `test_jc_writer_stops_off_row`**, rouge d'abord, par `in_fresh_child` après la caractérisation ; `must_abort` existant, valeur hors ligne dans une globale ; cas à union nulle par `memset` puis `kind` :

  | cible | nom | valeurs hors genre |
  |---|---|---|
  | `the_slipway()`, écrivain | `fo` | fixe 1, **fixe 0 union nulle**, `true`, **`false` union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table`, REF liste (`pico8_hv()`), REF vecteur, **REF d'un `jo`** (le cas de l'ancien `test_draw`), REF d'un `ga`, REF d'un `bg`, REF du `{qs}`, REF d'un `jc_mouse_point` |
  | idem | `p_` | fixe 1, **fixe 0 union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table`, REF vecteur, REF d'un `jc` |
  | construction | `qu` | fixe 1, **fixe 0 union nulle**, `true`, **`false` union nulle**, `"x"`, REF `ob_table`, REF vecteur, **REF d'un `ga`**, REF d'un `g_`, REF d'un `mz`, REF d'un autre `jc`, REF d'un `jc_mouse_point` |

  **Exclus** (dans le genre) : nil pour les trois ; `true`, `false`, `false` union nulle pour `p_`. **`mt`** : aucun producteur ne rend autre chose que nil ou un fixe (H8), aucun cas atteignable par nom — échec de l'`assert` de `jc_take_fixed` en trou `gcov` justifié. Rouge sur l'ancien code (H12), vert après : une ligne « Runs » pour chacun. Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_jc && ./tmp/test_pico8_jc` sauf mention, notées dans le commit avec leur test rouge ou la justification du trou ; un appel unique d'aide `static` se retire sous `if (0)` (1b.14b) :
  1. **(§7-1) indicateur forcé vrai** : le lecteur ignore `has_od` (nil lu `FIXED` 0 présent) → rouge : `test_construction` (`check_fresh_jc`, `"od"` absent), caractérisation 1. **(1b)** ignore `has_mt` → idem. **(1c)** ignore `has_p_` → rouge : `check_fresh_jc`, `test_ey` (`bool_state(held, "p_") == 0`).
  2. **(§7-2) « présent 0 » là où c'était absent** : la branche `mt` rend `pico8_value_fixed(0)` quand `has_mt` = 0 → rouge : `check_fresh_jc`, caractérisation 3.
  3. **(§7-3) init qui saute un membre typé** — **sans objet dans sa lettre** (aucune boucle de recopie) ; forme « pose une autre valeur » (précédents 1b.24, 1b.32–1b.34) : (a) R3 `has_od = 1` → rouge : `check_fresh_jc` ; (b) R3 `nh` = 19 → rouge : `check_fresh_jc` ; (c) R3 `fp = 1` → rouge : `check_fresh_jc` (`fp` faux) ; (d) R3 `nu = g_jc_k_` → rouge : `check_fresh_jc` (`nu` = `{"jc"}`) ; (e) branche de clé `qu` sous `if (0)` → rouge : `check_fresh_jc` (`qu` = `ly`) ; `test_pico8_mx`/`test_pico8_nr` notés.
  4. *Écrivain de `fo` muet* (`if (0) jc_take_holder(…)`, `return 1` gardé) → rouge : `test_jc_is_a_struct`, `test_hq` (`get_ref(old, "fo") == twin`), `test_bv`, caractérisation 4.
  5. *Écrivain de `p_` muet* → rouge : `test_jc_is_a_struct` (`bool_state(jc, "p_") == 2`), caractérisation 5.
  6. *Nil jugé sur le pointeur seul* (`value.as.ref == NULL` sans le genre) dans `jc_take_bool`, puis dans `jc_take_holder`, puis dans `jc_take_endpoint` → rouge : caractérisation 5 (`false` union nulle lu absent), test d'arrêt (union nulle).
  7. *Prédicats* : (a) `jc_is_fo_holder_class` sans `jc` → rouge : `test_hq`, `test_jc_is_a_struct` ; (b) sans `mz` → rouge : caractérisation 4, **`test_pico8_mz`** (R-e, dans le budget) ; (c) sans `bv_dispatch` → rouge : `test_bv`, caractérisation 4 ; `test_pico8_g_` `test_life` noté ; (d) élargi à `jo` → rouge : test d'arrêt ; (e) `jc_is_endpoint_class` sans `jo` → rouge : `test_construction` ; (f) sans `LY` → rouge : caractérisation 2 ; `test_pico8_mx`, `test_pico8_nr` notés ; (g) élargi à `methods[LDOWN] != NULL` (admet `ga`) → rouge : test d'arrêt.
  8. *R6 `hq` : `et` jamais posé* (`self->et = NULL`) → rouge : `test_hq`, `test_too_far` (`ET_QS`), `HQ_HASH`.
  9. *R6 `kj` : `fo` ignoré* (`et_or_fo = self->et != NULL`) → rouge : `test_draw` (`DRAW_HASH`) — preuve R-c.
  10. *R6 `kj` : `fo` jamais éteint* → rouge : `test_draw` (`!has(old, "fo")`) — preuve R-c.
  11. *R6 `hq` : `fa` inversé* (`has_mt ? 4 : 2`) → rouge : `HQ_HASH`, caractérisation 3.
  12. *`ce` : `self_mt` sans `has_mt`* (`pico8_value_fixed(self->mt)`) → rouge : caractérisation 3 (croisement) ; `HUB_HASH` noté s'il rougit aussi.
  13. *`jc_take_fixed` : nil rangé en « présent 0 »* → rouge : caractérisation 3 (`mt` absent), `HQ_HASH`.
  14. *`jc_take_endpoint` sans sortie sur nil* (nil jusqu'à l'`assert`) → rouge : caractérisation 2 (clé `qu` nil, le fils avorte).
  - **Mutants équivalents, notés** : `mt_truthy = self->mt != 0` dans `kj` (un `mt` présent vaut toujours 1) ; `*has = 1` avant l'`assert` d'une aide (fils qui avorte, état jamais relu) ; retrait du test de tag `STRUCT` avant `cls` (lecture de `->cls` d'une non-struct : comportement indéfini, pas un mutant discriminant).
- **`gcov`** : `test_pico8_jc` en `--coverage -O0`, piège `__gcov_dump` des fils (1a.23), sur `slipway.c` et `slipway_display.c`. Lignes attendues exécutées : poses R3, les 16 branches R4 (`has_` = 0 et 1 de `od`, `mt`, `p_`, NULL et non NULL de `qu`, `re`, `et`, `fo`), les deux branches d'écrivain et leur `return 0`, la branche de clé et l'`else`, les 4 aides (hors échec d'`assert`), les 2 prédicats (toutes leurs clauses : un porteur `bv`, un `jc`, un `mz` ; un `jo`, un `LY`), les lectures R6. **Trous attendus** : branches NULL de `nu`, `k_`, `ts`, `hg` (jamais NULL après construction) ; les gardes `self == NULL` existantes ; les échecs d'`assert` (fils de `must_abort`, prouvés par le test d'arrêt et les mutations 6–7, 14), dont celui de `jc_take_fixed`, inatteignable (H8). Rapport dans le commit.
- **Prior art** : `test_too_far_members_by_name`/`test_too_far_writer_stops_off_row` de `test_pico8_jc` (1b.13) ; `test_bv_et_members_by_name` (`fold_bv_et_fo`, 11 porteurs neufs) et `a_slipway` de `test_pico8_g_` (1b.22/1b.32) ; `test_pico8_js` de 1b.34 (`g_off_row`) ; `jo_take_holder`, `bv_et_take_holder`, `family_take_target`, `g__take_bool`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer.

- `./tmp/test_pico8_jc` : ~11–15 s (H10) ; environ 24 lancements (départ avec `:695` réécrite, arrêt rouge, final, 20 mutations, `gcov`) ≈ 6–7 min builds compris. Si un lancement dépasse 20 s, on joue d'abord 1, 1c, 3e, 4, 6, 7a, 7d, 7g, 9, 10, 14, et on note les autres non lancées.
- §6 R-a, R-b : deux lancements chacun, quelques secondes par rejeu ; R-c par les mutations 9 et 10 ; R-d : deux lancements de `./tmp/test_pico8_g_` ; R-e (`./tmp/test_pico8_mz` sous mutation 7b), R-f dans le budget seulement.
- `make test` une fois ; `make e2e` une fois, **dans la même invocation** que `make class-coverage-net` ; `make sanitize-net` une fois, jamais en même temps que `e2e` ; `make value-members`.
- Ordre de priorité si le budget manque : cibles finales 1–5 ; départ, arrêt rouge, final ; R-a à R-d ; mutations 1, 3e, 4, 6, 7a, 7d, 7g, 9 ; le reste noté « non lancé ».
- Aucun TU lent n'est modifié ; compiler tôt sans lancer (`make -j tmp/test_pico8_mx tmp/test_pico8_nr tmp/test_pico8_ie`) : `make e2e` les lance, et `mx`/`nr` exercent la clé `qu` (`hj`, `nr`).

| # | Commande exacte | État de l'arbre (départ / final / mutation n / §6 R-x bug remis / §6 R-x restauré) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` vert, les **8** TU de `UNIT_TESTS_SLOW` verts ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **27**, aucune ligne `pico8_jc_t` ;
5. `timeout 15m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` — requis, `pico8_jc_struct_new` change (sans toucher `pico8_class_coverage_record` ni `KNOWN[]`).

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements (les fixtures de `test_pico8_jc`, `test_pico8_g_` et des tests §6 sont seulement **lues**).

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet**

- `FAILED:` si 1b.34 n'est pas commitée au départ, ou si `make value-members` ne rend pas 43 au départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, ou si le diff touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements.

**Types, tables, écrivain**

- `FAILED:` (« champ sans règle ») si une aide avorte dans un rejeu ou un TU hors des fils de `must_abort` (H3–H5, H8), ou si un type est élargi au lieu d'avorter : `jo`, `ga`, `bv_et`, `jc_too_far` ou `jc_mouse_point` admis dans `fo` ; `ga`, `g_`, `mz` ou `jc` admis en clé `qu` ; un non-booléen dans `p_` ; un `has_` ajouté pour tenir `false` ; `pico8_value_t` gardé dans `pico8_jc_t`.
- `FAILED:` si l'un des 16 membres n'a pas le type retenu à sa place, si l'un des 3 indicateurs manque ou n'est pas juste après son membre, ou si un autre `has_` est ajouté (`has_qu`, `has_re`, `has_et`, `has_fo`, `has_fa`…).
- `FAILED:` si un prédicat n'admet pas exactement son ensemble (`jc_is_endpoint_class` : les 8 descripteurs à `LY` et `jo` ; `jc_is_fo_holder_class` : les descripteurs à `pico8_bv_dispatch`, `jc`, `mz`), s'il est rendu public ou partagé avec un autre fichier, ou si `slipway.c` gagne une autre inclusion que `pico8_planet.h` et `pico8_wormhole.h` (ou `slipway.h` une inclusion).
- `FAILED:` si nil et la valeur de `od`, `mt`, `p_`, `re`, `et`, `fo` ne se lisent plus par nom exactement comme en 1a (absent ; `FIXED` / `BOOL` / REF au même pointeur), ou si `fp` n'est plus lu `BOOL` 0 présent.
- `FAILED:` si la `members[]` du lecteur, un `offsetof` ou une lecture/écriture `*(pico8_value_t *)` existe encore dans `slipway.c`.
- `FAILED:` si l'écrivain de `jc` n'accepte plus exactement `fo` et `p_`, si la boucle de clés n'accepte plus exactement `qu`, ou si le message de l'`assert(0 && …)` final change.
- `FAILED:` si une aide viole la forme de 1b.13/1b.15 (état nil d'abord ; genre **et** pointeur testés ; union nulle hors genre qui avorte ; message qui nomme `jc.<m>` et le genre ; `has_` posé en dernier ; tag `STRUCT` vérifié avant `cls`), si elle est rendue publique ou partagée avec un autre fichier.
- `FAILED:` si `jc_too_far`, `jc_mouse_point` ou leurs aides changent, ou si `pico8_jc_ey`, `pico8_jc_ce`, `pico8_jc_eb`, `pico8_jc_hq`, `pico8_jc_pq` changent de signature.

**Diff et preuves**

- `FAILED:` si un site par nom change (`dl`, `d_`, `bv`, `pico8_bv_jc_predicate`, `pico8_jc_ce_jc_predicate`, `nr:gv`, `planet_display.c`, `entity.c`…), ou si un site par nom est remplacé par un accès direct.
- `FAILED:` si un autre fichier de `src/` que `slipway.h`, `slipway.c`, `slipway_display.c` change, ou `tools/`, le `Makefile`, `CENSUS_UNIT_TESTS`, `UNIT_TESTS_SLOW`.
- `FAILED:` si un autre test que `test_pico8_jc.c` et `test_pico8_g_.c` change ; si `test_pico8_jc.c` retire ou modifie une autre ligne existante que `:695` ; si `test_pico8_g_.c` change autrement qu'à `:494` ; si une valeur figée existante change (H19), `DRAW_HASH` compris.
- `FAILED:` si un test de régression (§6) a un diff, ou s'il manque les deux lignes « Runs » (rouge bug remis, vert restauré) de R-a à R-d, ou la ligne « Runs » de la réécriture de `:695` verte sur l'ancien code ; pour R-e, R-f, soit leurs deux lignes, soit « non lancé » au journal.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas l'attendu, si `make sanitize-net` n'est pas `15 propre(s)`, si `make value-members` ne rend pas 27 ou si un autre typedef gagne ou perd une ligne, ou s'il manque la ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un crochet du recensement change (le `STOP` de `test_draw` compris), ou si `tools/member_census.txt` est édité à la main.

**Tests de l'étape**

- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, lit ou écrit un des 16 membres directement, ne couvre pas les 16 noms, les 9 classes de `qu` par `ldown`, les 12 porteurs de `fo` (dont `fv`, `nl`, `os`, `pm`, `mx`), `re` planète et `jc_mouse_point`, `et` `{qs}`/planète/`jc`, `mt` présent et absent, les états de `p_` (dont `false` à union nulle), la REF à union nulle de `fo`, les noms sans branche et les intrus ; ou si `JC_MEMBERS_HASH` change après.
- `FAILED:` si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'arbre de départ et une verte sur l'arbre final, ou s'il n'a pas les cas union nulle `memset`, un `jo` dans `fo`, un `ga` en clé `qu`, fonction NULL, chaîne NULL.
- `FAILED:` si le commit ne porte pas le rapport `gcov` et les mutations 1 à 14 (avec 1b, 1c, 3a–3e, 7a–7g), chacune avec son test ou la justification de son trou, plus les mutants équivalents.

**Commit et note**

- `FAILED:` si le travail n'est pas en un commit vert.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N` (TROU-7 gardé à l'identique).
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, ou s'il manque le tableau « Runs » ou la note « Découvert à 1b.35 ».

## Out of Scope

- `jc_too_far` (1b.13) et `jc_mouse_point` (1b.12), faits : ni leurs types, ni leurs aides.
- Les homonymes : `g_.et`/`fo`/`od`/`fa`/`dw`/`fp`/`ik`/`nu`/`k_`/`p_` (1b.32), `jo.fo`/`p_`/`nu`/`fa` (1b.23), `qh.et`/`od`/`fp`/`dw` (1b.24), `fr.mt` (1b.31), `bv_et.fo` (1b.22), `mz.et`/`dw`/`od`/`fp`/`fa`/`p_` (1b.36), `ui.lheld`/`o_` (1b.37, qui pointent vers des `jc`).
- `pico8_jc_ce`, `pico8_jc_eb` rendus en autre chose que `pico8_value_t` ; les aides de lecture par nom (`pico8_jc_entity_value`…) ; le site `PICO8_REF_AS_LIST_CONST` de `qu.oe`.
- Corriger TROU-7 (`ts`, `hg`, `jc_mouse_point` jamais libérés).
- Les sites par nom (`dl`, `d_`, `bv`, les prédicats de `ce` et de `bv`, `nr:gv`, `planet_display.c`).
- Déplacer un TU entre listes ; régénérer `tools/member_census.txt` ; `make ref-audit`.
- Fichiers d'événements, `FRAMES_EVENTS`, rendu, audio, appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur (G)** : modules `slipway.h` (typedef, 3 `has_`, aucune inclusion) et `slipway.c` (lecteur, écrivain, clé, 4 aides `jc_take_*`, 2 prédicats `jc_is_*`, R6, 2 inclusions, `<stddef.h>`), `slipway_display.c` (R6, 2 aides de lecture supprimées, `PICO8_REF_AS_LIST` retirés) ; `qu`, `re`, `et`, `fo` → `pico8_struct_header_t *` sans indicateur, `od`, `mt` → `pico8_fixed_t` + `has_`, `p_` → `int` + `has_p_`, `fp` → `int`, `fa`, `ik`, `dw`, `nh` → `pico8_fixed_t`, `nu`, `k_`, `hg` → `pico8_ob_table_t *`, `ts` → `pico8_list_t *` ; ensembles `qu` = `LY` ∪ `jo`, `fo` = `bv_dispatch` ∪ `jc` ∪ `mz` ; `test_draw:695` → `old` ; tests dans `test_pico8_jc` par ajouts ; pas de découpage ; §6 : R-a à R-d obligatoires, R-e, R-f dans le budget.
- **Note « Découvert à 1b.35 »** (à écrire sur la ligne ; le reste va dans le commit) :

  > patron 1b.1 appliqué ; `fa`, `ik`, `dw`, `nh` → `pico8_fixed_t` (littéraux), `fp` → `int`, `nu`, `k_`, `hg` → `pico8_ob_table_t *`, `ts` → `pico8_list_t *`, `od`, `mt` → `pico8_fixed_t` + `has_`, `p_` → `int` + `has_p_` ; **`qu`, `re`, `et`, `fo` : lecture de 1b.22, pointeur d'en-tête sans indicateur (jamais `false`)** ; **lecture nouvelle de 1b.22 : un membre à plusieurs cibles écrit seulement par des méthodes, depuis des pointeurs typés (`re`, `et`), n'a ni aide ni prédicat — son ensemble n'est gardé que par le type** (`et` vise légitimement toute planète de `cg.np`, hors ligne) ; **`qu` = ligne ∪ `hj`, `nl`, prédicat `methods[LY] != NULL` ∪ `jo`** (`ga` refusée) ; **`fo` = ligne ∪ `fv`, `nl`, `os`, `pm`, `mx`, prédicat `methods[HQ] == pico8_bv_dispatch` ∪ `jc` ∪ `mz`** (union de `jo.fo` et de `bv_et.fo`) ; `mt` lu par nom sur `qu`/`re` rangé par `jc_take_fixed` (1b.14a) ; **l'écriture de test arrêtée à 1b.4b (`test_draw`, un `jo` dans `jc.fo`) est réécrite sur la ligne (`old`), `DRAW_HASH` inchangé, `STOP` gardé** ; `test_pico8_g_:494` réécrite ; **`members[]` du lecteur supprimée, plus aucun `offsetof` ni `<stddef.h>` dans `slipway.c`** ; deux sites `PICO8_REF_AS_LIST` de `slipway_display.c` disparus avec `ts` ; §6 : `slipway_drag`, `playability` sur le chemin, prouvés rouges sans diff ; empreinte réelle mesurée au commit.
- Le message de commit porte : compte 43 → 27, résumé « Runs », `gcov`, mutations et trous, `JC_MEMBERS_HASH`, réponses à H1–H27.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
