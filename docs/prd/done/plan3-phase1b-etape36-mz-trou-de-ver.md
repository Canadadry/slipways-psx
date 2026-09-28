---
title: "Plan 3, étape 1b.36 : `mz`, le trou de ver. Ses 17 membres typés ; `jj` en `pico8_mz_jj_t *` (1b.3), `et` en pointeur d'en-tête plus `has_et` (false = présent et NULL, lecture de 1b.24), `ij` en pointeur d'en-tête sans aide (lecture de 1b.35), `p_` avec indicateur ; écrivain à 4 branches (`ra`, `p`, `et`, `p_`) ; la table `members[]` et tout `offsetof` de `wormhole.c` supprimés"
description: "Après 1b.35, `pico8_mz_t` garde 17 membres `pico8_value_t` : 9 défauts de spec posés en littéraux (`nu`, `dw`, `dq`, `spawn_snd`, `ik`, `oi`, `od`, `fp`, `fa`) plus `kk` et `jj`, ce que `pq` alloue (`ox`), ce que `bv` écrit par nom (`ra`, `p`, `et`), ce que `mz:hq`/`kz`/`ey`/`pr` écrivent directement (`ij`, `et`, `oi`, `p_`) et `p_` écrit par `d_`. 1b.36 les type selon R1–R9 de 1b.1. `et` ({false, jc, bv_et, mz_jj}) prend la lecture de 1b.24 ; son ensemble est exactement la ligne (planètes exclues par `mz.dw` = 0, preuve de 1b.23d), prédicat par trois accesseurs publics. `ij` n'est écrit que par `mz:hq` depuis `pico8_ij` : pas d'aide (lecture de 1b.35). Aucun test existant ne change ; caractérisation et test d'arrêt ajoutés à `test_pico8_mz`. Un seul commit. `make value-members` passe de 27 à 10."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.36 — `mz` (`wormhole.h:27`), `jj` → 1b.3, genres mêlés `et`, REF multiple `ij`, 4 membres écrits par nom. Empreinte 36 (13, 23). Compte : 10.

Ligne de 1b.3, qui fixe la cible de `jj` :

> 1b.3 — `mz_jj` (`wormhole.h:19`), `qs`, cible de `mz.jj`. Empreinte 4 (2, 2). Compte : 263. Découvert à 1b.3 : patron 1b.1 appliqué sans écart ; sur une instance statique sans constructeur, R3 se lit sur l'initialiseur agrégé ; ni écrivain ni table d'offsets, R5 et R7 restent à fixer par la première étape qui en a.

Cadre : puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », §3 (`make test` vert, aucun `*.sha256` modifié), §6, §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Modèles : le PRD de 1b.35 (`docs/prd/triage/plan3-phase1b-etape35-jc-voie-de-glisse.md`, forme la plus récente, REF à plusieurs cibles écrites par des méthodes), celui de 1b.34 (`plan3-phase1b-etape34-js-bouton.md`, dernier gradé) et celui de 1b.32 (`plan3-phase1b-etape32-g-sonde.md` : `g_`, l'autre porteur de `bv`, dont `ra`/`p`/`et`/`p_` sont les homonymes exacts). Notes « Découvert à » **citées, pas redérivées** ; elles lient ce PRD :

- **1b.1** : patron R1–R9 ; s'en écarter est une lecture nouvelle, notée.
- **1b.3** : `pico8_mz_jj_t` typé (`qs` → `const char *`), instance statique `g_mz_jj_wrapper`, sans écrivain, accesseurs `pico8_mz_jj_wrapper()` et `pico8_mz_jj_struct_class()` ; **cible de `mz.jj`** → `pico8_mz_jj_t *jj` (R1 `ref:pico8_X_t`).
- **1b.4b** : aucune écriture de test hors ligne ni boucle sentinelle ne vise `mz` ; `bv_et.fo` et `jc.fo` portent `ref:pico8_mz_t`.
- **1b.5** : emballage public unique pour un membre à genres mêlés, appelé par le lecteur **et par toute lecture directe qui garde la valeur de 1a** ; preuve `grep 'pico8_value_t \*' … | grep -v out_value` vide.
- **1b.7** (homonymes) : un nil `stat?` venu d'un homonyme ne donne pas d'indicateur (ici `od`) ; lecture {`false` seul, pointeur} → pointeur + `has_`.
- **1b.9/1b.20/1b.31/1b.33** : R7 forme `offsetof` — une table vidée est supprimée avec son seul client (ici la `members[]` locale du lecteur), branches R4 `strcmp` dans l'ordre du typedef ; `<stddef.h>` retiré quand plus aucun `offsetof`.
- **1b.10** : `ref:autre` = REF non NULL de tag `UNKNOWN` (vecteurs `ra`, `p`).
- **1b.11** : R2 des TU lents — relus plus bas.
- **1b.13** (forme écrivain de R5) et **1b.15** (aide à indicateur) : une branche par membre `e`, ordre du typedef ; aide `static` par genre, forme par valeur ; l'aide pose **d'abord** l'état nil, puis sort sur `REF NULL` (genre **et** pointeur testés) ; sinon `assert` « champ sans règle » qui nomme `struct.membre`, le genre de la ligne et l'arrêt ; nom sans branche → `return 0` ; **test d'arrêt rouge d'abord**, avec des cas non-REF à union nulle (`memset` puis `kind`, 1b.14b) ; pour `ref:pico8_X_t`, `cls` comparé à l'**accesseur public**.
- **1b.14b** : un fixe sans table de défauts de fille garde le littéral de l'init ; une mutation qui retire l'unique appel d'une aide `static` se plante sous `if (0)` (`-Werror`).
- **1b.15/1b.19** : `pico8_X_value_vec(pico8_value_ref(self->p))` pour relire un vecteur typé par une aide qui prend une valeur ; branches dans l'ordre du typedef.
- **1b.20/1b.34** : un `assert` de tag fait **à la lecture** d'un membre passe à l'écriture ; il tombe quand l'écriture est déjà gardée.
- **1b.22** : **REF à plusieurs cibles → `pico8_struct_header_t *`** ; **ensemble admis = ligne ∪ classes productrices prouvées par le code et exercées par la caractérisation sur l'ancien code** ; **prédicat par liste d'accesseurs publics quand tous sont liés partout où le fichier l'est**.
- **1b.23a2** : un membre typé sort des `written[]` des TU (vacant ici) ; un test qui écrit un membre directement change dans la seule forme de son écriture (vacant ici : aucun).
- **1b.23d** : **`mz` exclue de `jo.fo` par preuve de code** (`mz.dw` = 0, `nq < dw` jamais vrai, confirmé par les 15 rejeux) — la même preuve exclut les planètes de `mz.et`.
- **1b.24** : **{`false` seul, `ref:` à plusieurs cibles} → `pico8_struct_header_t *<m>` + `int has_<m>`, NULL présent = `false`, `true` avorte** ; « `g_.et` et `mz.et` la relisent sur leur preuve » ; le crochet `STOP` d'une boucle supprimée est gardé seul ; preuve de typedef par `awk`.
- **1b.29/1b.32/1b.34** : un test de régression **sur le chemin** du membre typé, sans diff, est prouvé rouge.
- **1b.30/1b.31/1b.33/1b.34** : caractérisation dans le TU rapide de la classe ; un nil écrit directement par un test n'est pas une preuve.
- **1b.32** : `g_.et` → `pico8_struct_header_t *` + `has_et`, emballage public `pico8_g__et`, aides `g__take_vec`/`g__take_target`/`g__take_bool` : **précédent de forme exact** pour `mz.ra`/`p`/`et`/`p_` (même `bv`, autre ensemble).
- **1b.35** (supposée commitée, H1–H2) : lecture proposée — **un membre à plusieurs cibles écrit seulement par des méthodes de sa struct n'a ni aide ni prédicat, son ensemble n'est gardé que par le type** (`jc.re`, `jc.et`) ; ici `mz.ij`. `jc_is_fo_holder_class` admet `mz` par `pico8_mz_struct_class()` (interaction plus bas).

### État de départ

HEAD `a01ff5a` (PRD de 1b.35) au moment de l'écriture ; 1b.35 est implémentée **en parallèle** (l'arbre de travail modifie `src/pico8_slipway.h` et `tests/test_pico8_jc.c`, lus sans être touchés). **Précondition : 1b.35 (`jc`) est commitée avant que 1b.36 commence (H1)** ; `make value-members` rend alors **27** (17 lignes `pico8_mz_t`, 10 `pico8_ui_t`). 1b.34 est commitée (`d771b51`). Numéros de ligne indicatifs (arbre `a01ff5a`) ; chaque site est aussi désigné par son contenu. **Rien n'a été compilé ni lancé pour ce PRD** : les faits viennent de `grep` et de la lecture ; tout fait d'exécution est une hypothèse (H1–H26).

### Lignes de recensement (`tools/member_census.txt:258-274`)

| membre | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|
| `nu` | `ref:table` | jamais | - | l | `pico8_ob_table_t *` (`g_mz_nu`, table du module) | non (pointeur) |
| `dw` | `fixe` | jamais | - | l | `pico8_fixed_t` (littéral 0) | non |
| `dq` | `chaine` | jamais | - | l | `const char *` (littéral `"pr"`) | non (pointeur) |
| `spawn_snd` | `fixe` | jamais | - | l | `pico8_fixed_t` (52) | non |
| `ik` | `fixe` | jamais | - | l | `pico8_fixed_t` (49) | non |
| `oi` | `fixe` | jamais | - | l | `pico8_fixed_t` (0, puis `kz`) | non |
| `od` | `fixe` | observé | stat? | l | `pico8_fixed_t` (3) | non (`stat?` d'homonyme, 1b.7) |
| `fp` | `fixe` | jamais | - | l | `pico8_fixed_t` (1 ; homonyme `jc.fp`/`g_.fp` booléen) | non |
| `fa` | `fixe` | jamais | - | l | `pico8_fixed_t` (5) | non |
| `kk` | `ref:table` | observé | stat | l | `pico8_ob_table_t *` (`g_mz_kk`) | non (pointeur) |
| `jj` | `ref:pico8_mz_jj_t` | jamais | - | l | `pico8_mz_jj_t *` (`&g_mz_jj_wrapper`) | non (pointeur) |
| `ox` | `ref:liste` | observé | stat | l | `pico8_list_t *` (alloué par `pq`) | non (pointeur) |
| `ra` | `ref:autre` | observé | lu,stat? | **le** | `pico8_vec_t *` (le `mp` de `hq`, aliasé) | non (pointeur) |
| `p` | `ref:autre` | observé | lu | **le** | `pico8_vec_t *` (alloué par `bv`, TROU-7) | non (pointeur) |
| `et` | `bool`, `ref:` bv_et, jc, mz_jj | observé | lu,stat? | **le** | `pico8_struct_header_t *` | **`int has_et`** (NULL présent = `false`) |
| `ij` | `ref:` fr, g_, jo, mx, nl, nr, pm | observé | lu,stat | l | `pico8_struct_header_t *` (écrit par `hq` seul) | non (pointeur) |
| `p_` | `bool` | observé | lu | **le** | `int` (0/1) | **`int has_p_`** |

Compte : 27 − 17 = **10**. Aucune ligne `pico8_mz_t` ne reste ; les 10 lignes `pico8_ui_t` (1b.37) ne bougent pas. Indicateurs : 2 (`has_et`, `has_p_`), chacun juste après son membre, ne comptent pas (R9). « 4 membres écrits par nom » de la ligne : `ra`, `p`, `et`, `p_`.

### Le cart (`pico8/lua.lua`)

- `mz=ke:jr([[nu=o("mz"),me="kz",dw=0,dq="pr",spawn_snd=52,ik=49,oi=0,od=3,fp=1,fa=5,kk=o(7,7,6,13,5,1,1,1,0),jj=o(qs="-too far-"),]])` (`:752-762`) ; `mz.hq=bv` redéfini par `function mz:hq` (`:767`), `mz.ev=dl` (`:763`).
- **`mz:pq`** (`:764-766`) : `rn(53)self.ox=jp(hv,function(p) return p.fx end)`.
- **`mz:hq(...)`** (`:767-771`) : `bv(self,...)` puis `nq,self.ij=ij(self.p,self.ox)` ; `if nq>ov("space folding",0.19,0.27) then self.et=mz.jj end`.
- **`bv(self,mp,rp)`** (`:552-558`) : `self.ra,self.p=mp,mp/128` ; `self.et = nq<self.dw and c or jp(cg.jc, …)[1] or not ui.hw(rp) and {}` — avec `self.dw` = 0, **`nq<0` est toujours faux** (`ij` rend une longueur au carré ou 32767) : `et` ∈ {un `jc`, `{}` (le `bv_et`), `false`} ; **jamais `true`, jamais nil après `bv`, jamais une planète** ; `if self.et then self.et.fo=self end`.
- `mz:kz(t)` (`:772-773`) : `self.oi=…` ; **`mz:ey()`** (`:774-775`) : `self.p_=self.t<15 or not d_(self)` (booléen) ; **`mz:pr(t)`** (`:776-782`) : lit `self.p`, `self.p_=t>25` ; `mz:kj(p)` (`:783-793`) : `if not self.et`, `self.oi`, `self.ij.ra`, `self.kk[…]` ; `mz:ej(t)` (`:794-796`) : `self.oi`.
- Lecteurs par nom hors `mz:*` : `dl` (`et`, `et.qs`, `od`), `d_` (`et`, `od`, `dq`, `fp`, `ik` ; écrit `p_=true`), `lt` (`nu`), `dg` (`p_`, `me`, `t`), `dc` (`ra`, argument de `kj`), `bv` (`dw`).

### Le code

**`src/pico8_wormhole.h`** : `pico8_mz_jj_t` (`:17-20`, 1b.3, **inchangé**) ; typedef `pico8_mz_t` (`:25-44`), `ke` puis 17 `pico8_value_t` ; commentaire de tête (« `nu` and `kk` point at the tables the module owns, `jj` at the one `pico8_mz_jj_t`; all shared by every `mz` and never modified in place. Nothing frees the struct: TROU-7 ») qui reste vrai. Inclusions : `fixed`, `gfx`, `ke`, `mem`, `struct`, `value`, `vec` — `pico8_ob_table_t`, `pico8_list_t`, `pico8_struct_header_t` y arrivent déjà par `ke.h` → `struct.h` → `ob.h` (H10). API publique inchangée (`pico8_mz_new`, `pico8_mz_pq`, `pico8_mz_hq`, `pico8_mz_kz`, `pico8_mz_ey`, `pico8_mz_pr`, `pico8_mz_ej`, accesseurs).

**`src/pico8_wormhole.c`** :

- `pico8_mz_jj_field`, `g_mz_jj_struct_class`, `g_mz_jj_wrapper` (`:24-48`, 1b.3), **inchangés**.
- Aides : `pico8_mz_truthy` (clients : `pq_ox_predicate`, `ey`), `pico8_mz_value_vec` (clients : `pico8_mz_field_vec`, `hq`, `pr`), `pico8_mz_field_vec`.
- `pico8_mz_pq` (`:89-102`) : `self->ox = pico8_value_ref(heap_ox)` (TROU-7).
- `pico8_mz_hq` (`:104-132`) : `pico8_bv` (écrit `ra`, `p`, `et` **par nom**, puis `fo` de la cible), `p_vec = pico8_mz_value_vec(self->p)`, `self->ox` par `PICO8_REF_AS_LIST_CONST`, `nq = pico8_ij(p_vec, ox_list, &nearest)`, **`self->ij = nearest`**, **`self->et = pico8_value_ref(pico8_mz_jj_wrapper())`** au-delà du seuil.
- `pico8_mz_kz` : `self->oi = pico8_value_fixed(result)` ; `pico8_mz_ey` : `self->p_ = pico8_value_bool(result)` après `pico8_d_` (qui peut écrire `p_` par nom) ; `pico8_mz_pr` : `self->p` lu, `self->p_ = pico8_value_bool(t > 25)` ; `pico8_mz_ej` : `self->oi.kind == FIXED`.
- **Lecteur `pico8_mz_field` (`:281-310`)** : `static` `members[]` locale de 17 `offsetof` (9 lignes) et sa boucle (`REF NULL` → 0), repli `pico8_ke_field`. Seule table d'offsets de `mz` : ni `defaults[]` (littéraux), ni `pico8_mz_member`, ni fille.
- **Écrivain `pico8_mz_set_field` (`:312-329`)** : `ra`, `p`, `et`, `p_` recopiés **quel que soit le genre** ; commentaire de contrat (« `ra`, `p`, `et` (bv) and `p_` (d_): the only names a generic writer ever writes on an `mz` »).
- **`pico8_mz_struct_new` (`:348-380`)** : `me = "kz"`, 17 poses (`nu` = `g_mz_nu`, 8 fixes, `dq` `"pr"`, `kk` = `g_mz_kk`, `jj` = `&g_mz_jj_wrapper`, 6 `REF NULL`), `assert` « pas de table de construction » (**aucune clé** : R5 boucle vacante), `pico8_class_coverage_record("mz")`, `pico8_mz_pq`, `pico8_ke_struct_pq`, `PICO8_MEMBER_CENSUS_SEE`.
- `offsetof` : 9 lignes, 17 occurrences, toutes dans `members[]` ; `<stddef.h>` inclus pour elles et le `size_t i` du lecteur ; `pico8_ref_audit.h` inclus pour le seul `PICO8_REF_AS_LIST_CONST` de `hq` (`pico8_type_tag_of` vient de `struct.h`). Inclusions : `probe.h` (donc `pico8_bv_et_struct_class`, `pico8_bv`) et `planet.h` déjà ; **pas `slipway.h`**.

**`src/pico8_wormhole_display.c`** (`pico8_mz_kj`) : `pico8_mzd_truthy(self->et)` (seul client de `pico8_mzd_truthy`), `pico8_mzd_fixed(self->oi)` deux fois (seul client de `pico8_mzd_fixed`), `pico8_mzd_ra_of_ij` (`self->ij.kind`, **`assert` de tag `STRUCT` à la lecture**), `has_kk = self->kk.kind == REF && …`, **`assert` de tag `OB_TABLE` à la lecture** de `kk`, `pico8_ob_table_get_index((const pico8_ob_table_t *)self->kk.as.ref, idx, &entry)` puis `entry.as.fixed` (**site C1 de `float-audit`**, `tools/float_audit_c1.txt:60`, extrait `entry.as.fixed` : inchangé). Commentaire TROU-12 (anneau noir) inchangé.

**Sites par nom, inchangés** (grep `"(M)"` sur `src/`, `M` les 17 noms) : `pico8_bv` (`probe.c`, écritures `ra`, `p`, `et`, lecture `dw`), `pico8_dl`/`pico8_d_` (`probe.c`, `et`, `od`, `dq`, `fp`, `ik`, écriture `p_`), `entity.c` (`nu` de `lt`/`he`, `ra` de `dc`, `p_` de `dg`), `pico8_mz_field_vec` (`p` d'un `jo` inconnu dans `pr`), `pico8_mzd_field_vec` (`ra` de la cible `ij`). Les autres lignes (`bootstrap.c`, `entity_geom.c`, `msgbox.c`, `particle.c`, `planet_display.c`, `slipway_display.c`, `ui.c`…) portent des homonymes.

**Tests, accès direct à un des 17 membres d'un `mz`** (grep `(->|\.)(M)\b` sur les fichiers qui nomment `pico8_mz_t` : `test_pico8_iv.c` seul, plus `test_pico8_mz.c` par `void *`) : **aucun**. `test_pico8_iv.c:189` déclare un `pico8_mz_t *` et ne lit `ox` que par nom. **Écriture de test par nom sur un `mz`** : `test_pico8_mz.c:556` (`paid_mz_on_unknown` : `"p"` = un vecteur alloué, dans la ligne), et les deux fils de `test_mz_is_a_struct` (`fo` sur le `jj`, `oi` sur un `mz` : avortent déjà, sans branche). Aucune boucle sentinelle `written[]` ne vise `mz` (vacant, 1b.23a2).

`tests/test_pico8_mz.c` (TU de la classe, rapide, ~3 s mesuré à 1b.3, dans `UNIT_TESTS` et `CENSUS_UNIT_TESTS`, hors `UNIT_TESTS_SLOW`, lié par `ENGINE_SRCS`) fige déjà beaucoup par nom : `check_fresh_mz` (défauts, `kk` entrée par entrée, `nu`, `jj` = `pico8_mz_jj_wrapper()`, absents `ra`/`p`/`et`/`ij`/`p_`, `ox` présent), `test_ox` (`OX_HASH`), `test_hq` (`HQ_HASH` : `et` `jc`/`jj`/`false`/jetable, `fo` de la cible = le `mz`, rang de `ij` dans `hv`), `test_kz_ticks`, `test_ey`, `test_pr_ticks`, `test_draw` (`DRAW_HASH`), deux rejeux (`PLAYTHROUGH_HASH`, `GEOHARVEST_HASH`), `test_gv`, `test_jj_wrapper`, `test_mz_is_a_struct`, `test_jj_qs_by_name` (1b.3).

### Revue des TU lents (R2 de 1b.11)

`grep -n 'mz\|wormhole\|bg_ldown'` sur `test_pico8_{ie,jo,qh,os,hj,pm,mx,nr}.c` : seul `test_pico8_ie.c:833` construit un `mz` (`pico8_mz_new(NULL)`) pour vérifier qu'il n'a pas de méthode `hb` (une méthode, pas un membre). **Aucune lecture nil par nom d'un membre de `mz`** dans un TU lent ; **aucun indicateur de plus que les 2 de la ligne** (H19).

## Solution

1. **Caractérisation figée sur l'ancien code** (arbre 1b.35 commité), dans le TU rapide `test_pico8_mz`, par nom seul ; verte avant tout typage.
2. **Test d'arrêt rouge d'abord** (écrivain `ra`, `p`, `et`, `p_`) dans le même TU.
3. **Commit 1b.36** : les 17 membres typés, 2 indicateurs ; `members[]` du lecteur supprimée ; lecteur à 17 branches ; écrivain à 4 branches ; 3 aides `static` (`mz_take_vec`, `mz_take_et`, `mz_take_bool`) et 1 prédicat `static` (`mz_is_et_class`) ; emballage public `pico8_mz_et` ; plus aucun `offsetof`, ni `<stddef.h>`, ni `pico8_ref_audit.h` dans `wormhole.c` ; `make value-members` rend **10**.
4. Aucun pixel ne bouge. Note « Découvert à 1b.36 » sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `wormhole.h`, je veux `pico8_mz_t` sans `pico8_value_t`, chaque membre au type de sa ligne, `has_et` et `has_p_` juste après leur membre.
2. En tant que lecteur par nom (`dl`, `d_`, `bv`, `lt`, `dg`, `dc`, `ui`, les TU), je veux que chacun des 17 noms rende sur tout `mz` exactement ce que rendait l'ancien code : présence, genre, valeur, identité du pointeur.
3. En tant que `bv` mené par `mz:hq`, je veux ranger `mp` dans `ra` (même pointeur), mon vecteur alloué dans `p`, et dans `et` un `jc`, la cible jetable ou `false` ; puis poser mon `fo` sur la cible.
4. En tant que `mz:hq`, je veux ranger la plus proche ancre de `ox` dans `ij` (ou rien) et le `jj` dans `et` au-delà du seuil.
5. En tant que `d_`/`mz:ey`/`mz:pr`, je veux écrire `p_` (vrai ou faux présent) et que `dg` le voie ; nil éteint `p_`.
6. En tant que `mz:kj`/`dl`, je veux dessiner exactement comme avant (anneau si `et` n'est pas vrai, ligne vers `ij.ra`, anneaux colorés par `kk`, étiquette « -too far- »).
7. En tant que gardien de `ra`, `p`, `et`, `p_`, je veux que tout genre hors ensemble avorte, union nulle comprise : `true` ou une planète dans `et`, une struct dans `ra`, un fixe dans `p_`.
8. En tant que `jc:hq`/`bv` de 1b.35, je veux que `jc.fo` continue d'accepter un `mz` et que `mz.et` continue d'accepter un `jc`.
9. En tant que mainteneur, je veux `members[]`, tout `offsetof`, `<stddef.h>` et `pico8_ref_audit.h` absents de `wormhole.c`.
10. En tant que tests de régression `playability`, `geoharvest_burst` (sans diff), je veux rester verts et rouges quand mon bug revient.
11. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
12. En tant que `plan-pilot`, je veux `make value-members` à 10 sans ligne `pico8_mz_t`.
13. En tant que vérificateur, je veux la caractérisation verte sur l'ancien code, le test d'arrêt rouge puis vert, les preuves rouges §6, les mutations et leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle : les validations que demande `to-prd` (modules, tests) sont tranchées comme le ferait un agent `guideline` — hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables — et marquées (G).

### `et` : `pico8_struct_header_t *et` + `int has_et` — lecture de 1b.24 sans écart (G)

États : nil (avant le premier `hq`, lu absent par `check_fresh_mz`) ; `false` (écrit par `bv` quand `rp` est dans `ui.hw`, lu `BOOL` 0 présent : `ET_FALSE` de `test_hq`/`test_ey`, étape 2 de `playability`) ; une REF (`jc`, `bv_et`, `jj`). **`true` n'est jamais écrit** : cart (`not ui.hw(rp) and {}`), C (seul `pico8_value_bool(0)` dans `pico8_bv`), TU (aucune écriture par nom d'`et` sur un `mz`) (H3). La preuve de 1b.24 vaut donc pour `mz`, comme 1b.24 le prévoyait : **pas une lecture nouvelle**.

- **Ensemble d'`et` = la ligne, exactement** : `jc` (branche `jp(cg.jc, …)` de `bv`), `bv_et` (`{}` hors panneau), `mz_jj` (`mz:hq`, écriture directe ; sur la ligne, donc admis aussi par nom). **Planètes exclues** par la preuve de 1b.23d (`mz.dw` = 0, `nq < 0` jamais vrai) — c'est le même raisonnement qui exclut `mz` de `jo.fo` ; le test d'arrêt porte un `jo` et un `qh`. Aucune classe productrice hors ligne : rien à élargir (1b.22).
- **Prédicat `mz_is_et_class(cls)` : `cls == pico8_jc_struct_class() || cls == pico8_bv_et_struct_class() || cls == pico8_mz_jj_struct_class()`** — liste d'accesseurs publics (1b.22 : tous liés partout où `wormhole.c` l'est : `slipway.c` et `probe.c` sont dans `ENGINE_SRCS`, `DATA_ENGINE_SRCS` et `main_raylib`, fait vérifié par grep du `Makefile`). **Inclusion ajoutée à `wormhole.c` : `pico8_slipway.h`** (`pico8_jc_struct_class`), rien d'autre ; `probe.h` y est déjà.
- **`mz_take_et(value, pico8_struct_header_t **out, int *has)`** : forme exacte de `g__take_target` (1b.32) : `*out = NULL; *has = 0;` ; sortie sur `REF NULL` ; `assert` (`BOOL` 0, **ou** REF de tag `STRUCT` **puis** `mz_is_et_class(cls)`) ; `*out = value.kind == REF ? value.as.ref : NULL; *has = 1;`. `true` avorte ; un `false` à union nulle est un `false` présent.
- **Emballage public `pico8_value_t pico8_mz_et(const pico8_mz_t *self)`**, déclaré dans `wormhole.h` avec son contrat (« REF NULL when `self` is NULL or `et` is absent, `false` when present and NULL, a REF to the target otherwise », forme `pico8_g__et`) ; appelé par le lecteur **et par `pico8_mz_kj`** (lecture directe qui garde la valeur de 1a, 1b.5) : `if (!pico8_mzd_truthy(pico8_mz_et(self)))` — `pico8_mzd_truthy` garde ainsi son client.
- **R6 dans `hq`** : `self->et = (pico8_struct_header_t *)pico8_mz_jj_wrapper(); self->has_et = 1;` (même adresse que l'ancien `pico8_value_ref(pico8_mz_jj_wrapper())`, H13).

### `ij` : `pico8_struct_header_t *`, sans aide ni prédicat — lecture de 1b.35 (G)

`ij` n'a pas de ligne `e` : seul `mz:hq` l'écrit, depuis le `nearest` de `pico8_ij` (la plus proche entité de `ox`, ou nil). Lecture de 1b.35 (« un membre à plusieurs cibles écrit seulement par des méthodes n'a ni aide ni prédicat ; son ensemble n'est gardé que par le type ») **étendue à une source `pico8_value_t` dont le genre est déjà gardé** : `pico8_ij` pose `*out_nearest = pico8_value_ref(NULL)` puis n'y range qu'un élément dont il a vérifié `kind == REF` et tag `STRUCT` (`entity_geom.c:102-104`, `assert` existant) (H4). R6 : `self->ij = nearest.as.ref;`, **aucun `assert` ajouté**. Noté dans « Découvert » comme lecture étendue. L'ensemble réel (entités de `hv` à `fx` vrai au `pq` : planètes de l'arbre `jo`, `g_`) n'est pas écrit dans le code.

- **Lecture dans `wormhole_display.c`** : `pico8_mzd_ra_of_ij` devient `if (self->ij == NULL) return (0,0); return pico8_mzd_field_vec(self->ij, "ra");` ; **l'`assert` de tag de lecture tombe** (1b.20/1b.34 : la garde est à l'écriture, ici celle de `pico8_ij`). Le repli (0,0) sur `ij` nil (le cart plante sur `self.ij.ra`) est le comportement actuel, **préservé** ; il n'a pas de TROU et ce PRD n'en ouvre pas (hors périmètre : écart préexistant, non découvert par l'étape — si l'implémenteur le juge écart port/cart, il le note au commit sans le corriger).

### `ra`, `p` : `pico8_vec_t *` ; `p_` : `int` + `has_p_` (G)

- **`ra`, `p`** : forme de `g_.ra`/`g_.p` (1b.32) ; `mz_take_vec(value, pico8_vec_t **out)` = copie de `g__take_vec` (REF non NULL de tag `UNKNOWN`, lecture 1b.10), message aux deux noms. `ra` est le `mp` passé à `hq` (`ui.qa`, aliasé, jamais écrit à travers) ; `p` est alloué par `bv` (TROU-7, inchangé). Lectures R6 : `p_vec = pico8_mz_value_vec(pico8_value_ref(self->p));` dans `hq` et `pr` (forme 1b.15) — `pico8_mz_value_vec` garde ses clients.
- **`p_`** : nil au départ ; `mz:ey` (direct : `self->p_ = result; self->has_p_ = 1;`), `mz:pr` (direct : `self->p_ = t > pico8_fixed_from_int(25); self->has_p_ = 1;`), `d_` (écrivain, `true`). `mz_take_bool` = copie de `g__take_bool`. `false` présent (`ey` payé, `pr` avant 25).

### Défauts et membres internes : littéraux et pointeurs (G)

- `dw`, `spawn_snd`, `ik`, `oi`, `od`, `fp`, `fa` → `pico8_fixed_t`, **sans indicateur** (nil `jamais` ; `od` : `stat?` d'homonyme seulement, 1b.7) ; toujours `FIXED` présents à la lecture. `fp` est un **fixe** 1 ici (homonymes booléens `jc.fp`, `g_.fp`) : `d_` le lit `FIXED` 1 par nom, inchangé.
- `dq` → `const char *` = le littéral `"pr"` du même fichier (même pointeur qu'avant, H13).
- `nu`, `kk` → `pico8_ob_table_t *` = `g_mz_nu`, `g_mz_kk` (même REF, R3) ; `jj` → `pico8_mz_jj_t *` = `&g_mz_jj_wrapper` ; `ox` → `pico8_list_t *` (`pq` : `self->ox = heap_ox;`, `hq` : `pico8_ij(p_vec, self->ox, &nearest)`, **`PICO8_REF_AS_LIST_CONST` retiré**).
- `oi` : `kz` : `self->oi = result;` ; `ej` : `oi = self != NULL ? self->oi : 0` ; `kj` : locales `oi = self->oi` (les extraits `pico8_fixed_flr_int(oi)` et `entry.as.fixed` restent tels quels, H16) ; `pico8_mzd_fixed` supprimée (sans client, `-Werror`).
- `kk` dans `kj` : `has_kk = self->kk != NULL` ; **l'`assert` de tag `OB_TABLE` de lecture tombe** (1b.20 : la garde est à la pose, `pico8_mz_spec_table` asserte le tag de `g_mz_kk`) ; `pico8_ob_table_get_index(self->kk, idx, &entry)`.

### Aides R5 (forme 1b.13/1b.15), toutes `static` dans `wormhole.c` (G)

`wormhole.c` porte deux structs ; `mz_jj` n'a ni écrivain ni aide : aucune aide existante n'est partagée ni renommée. Nil = `value.kind == PICO8_VALUE_REF && value.as.ref == NULL` (genre **et** pointeur).

| aide | membres | état nil puis sortie | accepté (`assert`) | message (préfixe) |
|---|---|---|---|---|
| `mz_take_vec(value, pico8_vec_t **out)` | `ra`, `p` | `*out = NULL` | REF non NULL, tag `UNKNOWN` | `"mz.ra/p: a write neither nil nor a vector ref is off its census row (ref:autre) -- …"` |
| `mz_take_et(value, pico8_struct_header_t **out, int *has)` | `et` | `*out = NULL; *has = 0` | `BOOL` 0, ou REF, tag `STRUCT` **puis** `mz_is_et_class(cls)` ; `*has = 1` en dernier | `"mz.et: a write neither nil, false, nor what bv or mz:hq aims at (a jc, bv's throwaway target, the jj wrapper) is off its census row (bool,ref: to those targets) -- …"` |
| `mz_take_bool(value, int *out, int *has)` | `p_` | `*out = 0; *has = 0` | `BOOL` ; `*has = 1` en dernier | `"mz.p_: a write neither nil nor a boolean is off its census row (bool) -- …"` |

Chaque message finit par `"a field under no rule of plan-3 section 5: stop, do not widen the member"`, et la phrase `is off its census row` tient sur une ligne du source (preuve par `grep -c`). Un `false` à union nulle est un `false` présent pour `et` et `p_`.

**Écrivain `pico8_mz_set_field`** : 4 branches dans l'ordre du typedef (`ra`, `p`, `et`, `p_`), chacune `if (strcmp(name, "<m>") == 0) { mz_take_…(value, &mz-><m>[, &mz->has_<m>]); return 1; }`, puis `return 0`. Commentaire de contrat gardé tel quel.

**Boucle de clés** : vacante (l'`assert` « no construction table was measured » reste, **texte inchangé**).

### R1–R9 appliquées

| règle | sur `mz` |
|---|---|
| R1 | Tableau de recensement ; nom et place gardés ; `has_et` (« 0: nil; 1: present, and a NULL `et` is `false` », forme `probe.h`) et `has_p_` (« 0: `p_` is nil, and is then never read ») juste après leur membre ; `et`, `ij` avec `/* borrowed, never freed */`. **Aucune inclusion ajoutée à `wormhole.h`** (H10). **Inclusion ajoutée à `wormhole.c`** : `pico8_slipway.h` seule. |
| R2 | Les 2 indicateurs de la ligne ; pointeurs sans indicateur ; `od` sans (1b.7). |
| R3 | `pico8_mz_struct_new`, après `pico8_ke_struct_init` et `me`, dans l'ordre du typedef : `nu = g_mz_nu`, `dw = 0`, `dq = "pr"`, `spawn_snd = 52`, `ik = 49`, `oi = 0`, `od = 3`, `fp = 1`, `fa = 5` (`pico8_fixed_from_int`), `kk = g_mz_kk`, `jj = &g_mz_jj_wrapper`, `ox`, `ra`, `p`, `et` à NULL, `has_et = 0`, `ij = NULL`, `p_ = 0`, `has_p_ = 0` ; puis l'`assert`, la couverture, `pico8_mz_pq`, `pico8_ke_struct_pq`, `PICO8_MEMBER_CENSUS_SEE`. Littéraux, pas de table (1b.14b). |
| R4 | `pico8_mz_field` : 17 branches `strcmp` dans l'ordre du typedef, puis `return pico8_ke_field(self, name, out_value);` ; petites aides `static` de lecture permises (forme `g__ref_field`/`g__fixed_field`). Fixes : toujours `FIXED` ; `dq` : NULL → 0, sinon `STRING` même pointeur ; pointeurs (`nu`, `kk` table, `ox` liste, `jj`, `ra`, `p`, `ij`) : NULL → 0, sinon REF ; `et` : `pico8_mz_et`, `REF NULL` → 0 ; `p_` : `has_p_` = 0 → 0, sinon `pico8_value_bool(p_)`. `*out_value` intact sur 0. |
| R5 | Écrivain ci-dessus ; boucle de clés vacante. |
| R6 | `pq` (`ox`), `hq` (`p` relu, `ox`, `ij`, `et`/`has_et`), `kz` (`oi`), `ey` et `pr` (`p_`/`has_p_`, `p` relu), `ej` (`oi`) ; `wormhole_display.c` : `et` par `pico8_mz_et`, `oi`, `ij`, `kk` (deux `assert` de tag de lecture retirés, `pico8_mzd_fixed` supprimée). |
| R7 | Forme `offsetof` de 1b.9 : la `members[]` locale du lecteur supprimée avec sa boucle et son `size_t i` ; **plus aucun `offsetof` dans `wormhole.c`**, `#include <stddef.h>` retiré ; `#include "pico8_ref_audit.h"` retiré (dernier usage parti avec `ox`, H11). Pas de table de défauts ni de fille : R7 de 1b.14a vacante. `wormhole_display.c` garde ses inclusions. |
| R8 | Les deux `PICO8_MEMBER_CENSUS_SEE` gardés ; `tools/member_census.txt` non édité. |
| R9 | 27 → 10. |

### Interaction avec 1b.35 (`jc`), 1b.32 (`g_`), 1b.22 (`bv_et`), 1b.23d (`jo`), 1b.37 (`ui`)

- **Le prédicat `fo` de 1b.35 admet `mz`** (`jc_is_fo_holder_class` : `methods[HQ] == pico8_bv_dispatch || jc || cls == pico8_mz_struct_class()`, parce que `mz:hq` mène `bv`, qui pose `jc.fo = mz`). 1b.36 **ne change ni `pico8_mz_struct_class()` ni l'adresse du descripteur** : ce prédicat reste vrai pour tout `mz`. Ce qui change sur ce chemin : dans `pico8_bv`, les écritures `ra`, `p`, `et` sur le `mz` passent **avant** l'écriture de `fo` sur la cible ; elles atteignent désormais `mz_take_vec`/`mz_take_et`, et `mz_take_et` doit admettre le `jc` que `bv` vise — c'est le terme `pico8_jc_struct_class()` de `mz_is_et_class`. Les deux ensembles se répondent : `jc.fo` ∋ `mz` (1b.35) et `mz.et` ∋ `jc` (1b.36). Retirer l'un fait avorter le rejeu : la mutation 8a le prouve.
- **Caractérisation de 1b.35** (`test_jc_members_by_name`, étape 4 : un `mz` neuf mène `hq` au milieu d'un `jc` isolé → `jc.fo` = ce `mz`) : elle passe désormais par `mz_take_vec`/`mz_take_et`. **`JC_MEMBERS_HASH` ne doit pas bouger** (H6) : `test_pico8_jc` est sur le chemin (R-e). Le test d'arrêt de 1b.35 utilise `pico8_mz_new(NULL)` comme clé `qu` hors ligne : le constructeur de `mz` doit rester sans effet de bord nouveau.
- **Inclusions croisées** : `slipway.c` inclut `wormhole.h` (1b.35), `wormhole.c` inclut `slipway.h` (1b.36) ; aucun des deux en-têtes n'inclut l'autre, pas de cycle (H10).
- **`bv_et.fo`** (`bv_et_is_holder_class` ∋ `mz`, 1b.22) et **`jo.fo`** (`mz` exclue, 1b.23d) : inchangés. `g_.et` (1b.32) : homonyme, inchangé.
- **`ui`** (1b.37) : `lheld` pointe vers des `mz` et `hq` leur passe `ui.qa` en `mp` (`ra` aliasé) ; rien ne change côté `ui`.

### Tests touchés (G)

- **`tests/test_pico8_mz.c`** seul, **par ajouts** : caractérisation, test d'arrêt, `MZ_MEMBERS_HASH`, aides de test (`must_abort`, valeur hors ligne dans une globale, forme `g_off_row`), inclusion éventuelle de `pico8_planet_family.h` (`pico8_qh_new`) et `pico8_slipway.h` (lecture de la classe d'un `jc` si besoin), deux appels `in_fresh_child` dans `main`. Aucune fonction existante, aucune valeur figée, aucun commentaire existant ne change.
- Aucun autre fichier de `tests/` : aucun accès direct, aucune boucle sentinelle.

### Revue des tests de régression, mécanique

**Liste** (reprise de 1b.30–1b.35) : les tests nommés au §6 (les 17 de régression) et dans les lignes 1a : `tests/test_pico8_<nom>.c` pour `built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`, `bg`, `entity_kind`, `hj`, `jc`, `jo`, `jo_spec_defaults`, `js`, `mo`, `mz`, `nl`, `os`, `pm`, `qh`, plus `tests/test_class_coverage.c`. Tous existent.

**Greps faits pour ce PRD** (`M = nu|dw|dq|spawn_snd|ik|oi|od|fp|fa|kk|jj|ox|ra|p|et|ij|p_`) :

- accès direct : `grep -nE '(->|\.)(M)\b'` sur les fichiers qui nomment `pico8_mz_t` → **aucun** sur un `mz` ;
- construction d'un `mz` ou mention (`grep -c 'mz\|wormhole\|bg_ldown'`) : `mz` (173), `bg` (10 : `ldown` construit un `mz`, `lheld` compté par classe), `playability` (5 : étape 2), `jc` (4 : `others[2]`, `fo` d'un `mz`, caractérisation de 1b.35), `geoharvest_burst` (1 : étape 1) ; 0 ailleurs ;
- rejeux qui bâtissent un `mz` : `phase4_playthrough_events.json` (frame 54, étape 2 de `playability` ; `PLAYTHROUGH_HASH` de `test_pico8_mz` le voit) et son préfixe `built_planet_panel_events.json` (frames 0–223 recopiées), `geoharvest_burst_events.json` (`GEOHARVEST_HASH`). Tests qui les rejouent : `playability`, `built_planet_panel_nil_row`, `rich_planet_exports`, `geoharvest_burst`, `bg`, `jc`, `jo`, `js`, `mz`. Les autres rejeux ne sont pas connus pour bâtir un `mz` (H17).

| # | test | accès / chemin | diff ? | preuve rouge |
|---|---|---|---|---|
| R-a | `playability` | étape 2 (frame 54) : `bg:ldown` construit un `mz`, `hq` (écrivain `ra`/`p`/`et`, `ij`, `jj`) chaque frame, `ey` après 15 ticks : `d_` lit `et`/`od`/`dq`/`fp`/`ik` par nom et débite `od` = 3, `pr` révèle `p27` (`p`, `p_`, `oi`) : **sur le chemin** | non | **Obligatoire.** Bug de référence de 1a.17 (constructeur de `jo` appelant `pico8_ke_struct_pq` deux fois, recette de 1b.32 R-b / 1b.34 R-f / 1b.35 R-b) → rouge (`count <= PICO8_PLAYABILITY_MAX_PLANETS`) ; restauré → vert. |
| R-b | `geoharvest_burst` | étape 1 : « the press held on p27 … launches an `mz` probe from it, which reveals p27 » : **sur le chemin** | non | **Obligatoire.** Bug de 1b.29 : `pico8_ga_pq` sans `pico8_ez(self, 140)` (recette de 1b.32 R-c) → rouge à la frame de récolte ; restauré → vert. |
| R-c | `mz` (TU de la classe) | toute la classe | **ajouts seuls** | aucune de plus : la caractérisation est verte sur l'ancien code, le test d'arrêt y est rouge (lignes « Runs ») ; les mutations 1, 3, 9 rougissent des tests existants (`check_fresh_mz`, `HQ_HASH`) ; valeurs figées inchangées (H13). |
| R-d | `built_planet_panel_nil_row` | rejoue le préfixe du `playthrough` (frame 54 : un `mz`) : **sur le chemin** par le rejeu seul, rien de `mz` n'est lu par le test | non | **Dans le budget seulement.** Recette corrigée de 1b.34 (clause `REF NULL` de `pico8_msgbox_truthy` retirée **et** lecteur de `jo.ep` qui rend nil présent) → rouge ; restauré → vert. Sinon « non lancé ». |
| R-e | `jc` (TU de 1a) | caractérisation de 1b.35 (`mz` neuf mène `hq` → `jc.fo = mz`) et `test_jc_is_a_struct` (`others[2]`) : **sur le chemin** de `mz_take_et` | non | **Dans le budget seulement**, par la mutation 8a (`mz_is_et_class` sans `jc`) : `test_pico8_jc` rouge (fils qui avorte) ; sinon « non lancé » au journal. |
| — | `rich_planet_exports` | rejoue `built_planet_panel_events.json` (préfixe du `playthrough`) ; rien de `mz` lu | non | aucune ; vert sans modification (si le budget le permet, recette 50f91b4 de 1b.34 R-e, sinon non lancé). |
| — | `bg` | `ldown` construit un `mz` (`LDOWN_MZ_*`), `lheld` compté par classe dans le `playthrough` | non | aucune ; vert sans modification. |
| — | `empty_planet_cleared`, `cheat_code`, `slipway_drag`, `game_end`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`, `entity_kind`, `hj`, `jo`, `jo_spec_defaults`, `js`, `mo`, `nl`, `os`, `pm`, `qh`, `test_class_coverage` | homonymes seuls, ou aucun `mz` connu dans le scénario ; si un rejeu en bâtit un, l'implémenteur le note (précédent 1b.32 `empty_planet_cleared`) | non | aucune. |

Hors liste, sur le chemin et verts sans modification : `test_pico8_g_` (`test_bv_et_members_by_name` : `holders[2]` = un `mz` qui mène `hq`), `test_pico8_iv` (`ox` vide d'un `mz` neuf), `test_pico8_ui`, `test_pico8_ie` (construisent des `mz`), `test_pico8_spec_defaults` (`STRUCT_HASH` : chaque nom d'un `mz` neuf comparé à sa spec, identité des chaînes et REF comprise, H13).

### Preuves (sortie attendue sur l'arbre final)

**Sortie de `mz`** :

- `awk '/^typedef struct \{/{b=""} {b=b $0 "\n"} /\} pico8_mz_t;/{printf "%s", b}' src/pico8_wormhole.h | grep -c pico8_value_t` : **0** ;
- `make value-members` : **10**, `grep -c pico8_mz_t` : **0** ; seules les 17 lignes `pico8_mz_t` disparaissent ;
- `grep -c 'offsetof' src/pico8_wormhole.c` : **0** (9 lignes avant) ; `grep -rn 'offsetof(pico8_mz_t' src/` : **vide** ; `grep -n 'stddef\|ref_audit' src/pico8_wormhole.c` : **vide** (H11).

**Lecteur, écrivain, aides** :

- `grep -nE '(->|\.)(M)\.(kind|as)' src/pico8_wormhole.c src/pico8_wormhole_display.c` : **vide** (9 lignes avant) ;
- `grep -n 'pico8_value_t \*' src/pico8_wormhole.c | grep -v 'pico8_value_t \*out_value)'` : **vide** (1 ligne avant : la lecture `*(const pico8_value_t *)` du lecteur) ;
- `grep -c 'off its census row' src/pico8_wormhole.c` : **3** (0 avant) ;
- `grep -n 'mz_take_\|mz_is_' src/pico8_wormhole.c` : 3 aides et 1 prédicat définis, chacun appelé ; `mz_is_et_class` = `jc` ∪ `bv_et` ∪ `mz_jj`, rien d'autre ;
- `grep -n 'has_et\|has_p_' src/pico8_wormhole.h` : chacun juste après son membre ; `grep -n 'pico8_mz_et' src/` : déclaration, définition, lecteur, `pico8_mz_kj` ;
- `grep -n 'PICO8_REF_AS_LIST\|pico8_mzd_fixed' src/pico8_wormhole*.c` : **vide** ; `grep -c 'pico8_type_tag_of' src/pico8_wormhole_display.c` : **0** (2 avant) ;
- `git diff -U0 src/pico8_wormhole.h | grep '^+#include'` : **vide** ; `git diff -U0 src/pico8_wormhole.c | grep '^+#include'` : `pico8_slipway.h`, rien d'autre ;
- `tools/float_audit_c1.txt` inchangé ; `grep -n 'entry.as.fixed' src/pico8_wormhole_display.c` : la même ligne dans `pico8_mz_kj`.

**Périmètre du diff** :

- `git diff --stat HEAD -- src/ ':!src/pico8_wormhole.h' ':!src/pico8_wormhole.c' ':!src/pico8_wormhole_display.c' tools/ Makefile` : **vide** ;
- dans `tests/`, seul `test_pico8_mz.c` change, par ajouts : `git diff -U0 tests/test_pico8_mz.c | grep -c '^-[^-]'` : **0**.

### Empreinte et découpage

Ligne : 36 (13, 23). **Remesurée pour ce PRD** avec la formule du plan, sur l'arbre `a01ff5a` (plus 1b.35 en cours) : **37 (12, 25)** — `src/` : `bootstrap.c`, `entity.c`, `entity_geom.c`, `msgbox.c`, `particle.c`, `planet_display.c`, `probe.c`, `slipway_display.c`, `ui.c`, `wormhole.c`, `wormhole.h`, `wormhole_display.c` ; `tests/` : `bg`, `empty_planet_cleared`, `entity_kind`, `fq`, `fr`, `fv`, `g_`, `ga`, `hj`, `ie`, `iv`, `jc`, `je`, `jo`, `js`, `mh`, `mo`, `mz`, `nf`, `nl`, `os`, `pj`, `qb`, `qh`, `ui`. Majorant : deux fichiers accèdent directement (`wormhole.c`, `wormhole_display.c`), le reste par des noms courts partagés (`ra`, `p`, `et`, `p_`, `fa`, `od`…). **Fichiers modifiés attendus : 4** — `src/pico8_wormhole.h`, `src/pico8_wormhole.c`, `src/pico8_wormhole_display.c`, `tests/test_pico8_mz.c` — plus `docs/plan-3.md`. Empreinte réelle comptée au commit (H22).

**Découpage (G) : aucun, un seul commit.** Évalué sur deux découpes :

- (a) **1b.36a** « défauts et membres internes » (`nu`, `dw`, `dq`, `spawn_snd`, `ik`, `oi`, `od`, `fp`, `fa`, `kk`, `jj`, `ox` ; 27 → 15) puis **1b.36b** « ce que `bv` et `hq` écrivent » (`ra`, `p`, `et`, `ij`, `p_` ; 15 → 10) ;
- (b) l'écrivain seul d'abord (`ra`, `p`, `et`, `p_`), le reste ensuite.

Écartées parce que : (1) les deux moitiés touchent les **mêmes** fonctions — la `members[]` du lecteur (qui ne tomberait qu'au second commit, avec une forme mixte qu'aucun commit final ne garde), `pico8_mz_struct_new`, `pico8_mz_hq` (`p`, `ox`, `ij`, `et`) et `pico8_mz_kj` (`et`, `oi`, `ij`, `kk`) ; (2) **le coût de test double** : chaque commit change le constructeur, donc chacun passe `make e2e`, `make sanitize-net` et `make class-coverage-net` (≈ 12–15 min à eux trois), et les preuves §6 R-a, R-b seraient faites deux fois ; (3) le travail propre à l'étape est peu coûteux : `test_pico8_mz` tourne en ~3 s, un mutant ne recompile que `wormhole.c` ou `wormhole_display.c`, le diff touche 4 fichiers (1b.32 : 23 membres, 6 fichiers, un commit) ; (4) la règle de découpage du plan (> ~40 fichiers) ne s'applique pas à 36/37. Si le budget est dépassé, l'implémenteur suit l'ordre de priorité de « Vérification » et note ce qui n'a pas été lancé ; ce n'est pas une raison de découper après coup.

### Hypothèses (rien n'a été lancé)

- **H1** : 1b.35 commitée avant le départ ; `make value-members` rend 27 au départ (aucune ligne `pico8_jc_t`, 17 lignes `pico8_mz_t`), 10 à la fin.
- **H2** : 1b.35 ne touche que `slipway.[ch]`, `slipway_display.c`, `test_pico8_jc.c`, `test_pico8_g_.c` et le plan : aucun fichier de cette étape ; `slipway.c` y gagne `#include "pico8_wormhole.h"`. Si la note « Découvert à 1b.35 » diffère de la lecture proposée par son PRD (membre écrit seulement par des méthodes : ni aide ni prédicat) ou change une règle citée ici, elle prime et l'écart est noté (ce n'est pas un arrêt ; au pire `ij` reçoit une aide et un prédicat « entité de `hv` à `fx` », tranchés au commit sur la ligne ∪ producteurs exercés).
- **H3** : hors des fils de `must_abort`, aucun site (cart, C, TU, 15 rejeux) n'écrit par nom dans `mz.ra`/`p` autre chose que nil ou un vecteur (tag `UNKNOWN`), dans `mz.et` autre chose que nil, `false`, un `jc`, `&g_bv_et` ou le `jj`, dans `mz.p_` autre chose que nil ou un booléen. En particulier `bv` ne vise jamais une planète depuis un `mz` (`dw` = 0 ; `pico8_ij` rend une longueur au carré ≥ 0 ou 32767). Démentie : arrêt « champ sans règle », sans élargir le membre.
- **H4** : `pico8_ij` rend toujours dans `*out_nearest` un `REF` (NULL, ou une struct vérifiée par son `assert`) ; `self->ij = nearest.as.ref` est donc sûr sans `assert` ajouté.
- **H5** : `test_pico8_mz` reste sous 30 s avec deux fonctions de plus (~3 s à 1b.3 ; la caractérisation rejoue au plus `world()` une fois par fils).
- **H6** : `test_pico8_jc` (caractérisation de 1b.35, `JC_MEMBERS_HASH`) et `test_pico8_g_` (`BV_ET_MEMBERS_HASH`, `holders[2]` = un `mz`) restent verts, hash inchangés, avec le `mz` typé.
- **H7** : sur l'ancien code, chaque cas du test d'arrêt se termine sans `SIGABRT` (l'écrivain recopie tout) ; un cas qui avorte déjà est retiré du « rouge d'abord » et noté.
- **H8** : `pico8_mz_new` d'une table vide passe l'`assert` (`count == 0`) et donne un `mz` frais identique à `pico8_mz_new(NULL)`.
- **H9** : le build `-DPICO8_MEMBER_CENSUS` compile avec un `pico8_mz_t` sans `pico8_value_t` ; `-fsyntax-only -DPICO8_MEMBER_CENSUS` sur `wormhole.c`, `wormhole_display.c`, `test_pico8_mz.c` suffit si le build complet n'est pas lancé (précédents 1b.29–1b.35).
- **H10** : `wormhole.h` compile sans nouvelle inclusion ; `pico8_slipway.h` inclus par `wormhole.c` sans cycle ni collision de nom ; tout binaire qui lie `wormhole.c` lie `slipway.c` et `probe.c` (fait pour `ENGINE_SRCS`, `DATA_ENGINE_SRCS`, `main_raylib` ; aucune autre cible ne nomme `wormhole.c`).
- **H11** : `-Werror` muet : `<stddef.h>`, `pico8_ref_audit.h` et `size_t i` retirés de `wormhole.c` (`NULL` vient de `<stdlib.h>`/`<string.h>`, `pico8_type_tag_of` de `struct.h`) ; `pico8_mzd_fixed` supprimée ; `pico8_mz_value_vec`, `pico8_mz_truthy`, `pico8_mzd_truthy`, `pico8_mzd_value_vec`, `pico8_mzd_field_vec` gardent un client ; les 3 aides et le prédicat appelés. Sinon ce qui reste est noté.
- **H12** : retirer les `assert` de tag de lecture de `ij` et `kk` ne perd aucune garde atteignable (écriture de `ij` par `pico8_ij` seul, de `kk` par R3 depuis `pico8_mz_spec_table`).
- **H13** : aucune valeur figée ne bouge : `OX_HASH`, `HQ_HASH`, `KZ_HASH`, `EY_HASH`, `PR_HASH`, `PR_EXIT_TICK`, `DRAW_HASH`, `PLAYTHROUGH_HASH`, `GEOHARVEST_HASH` de `test_pico8_mz` ; `STRUCT_HASH`/`SPEC_HASH` de `test_pico8_spec_defaults` (le littéral `"pr"` du même fichier garde son adresse, `nu`/`kk`/`jj` leur REF) ; les hash de `test_pico8_bg`, `test_pico8_iv`, `test_pico8_ui`, `test_pico8_ie`, `test_pico8_jc`, `test_pico8_g_` ; les 15 empreintes du filet. L'adresse de `et` = `jj` est celle de `pico8_mz_jj_wrapper()` (en-tête premier membre).
- **H14** : `make sanitize-net` propre (aucune allocation nouvelle ; `ox`, `p` toujours alloués comme avant, TROU-7 inchangé).
- **H15** : `make class-coverage-net` identique à la référence (le compte d'instanciations de `mz` ne change pas).
- **H16** : `float-audit`, `constants-check`, `float-grep`, `float-guard` inchangés : le site C1 `entry.as.fixed` de `pico8_mz_kj` garde son extrait, les `PICO8_FIXED_LIT` de `hq`/`kz`/`pr` sont gardés à l'identique, les extraits de classe D gardent leurs locales (`oi`, `sr`).
- **H17** : seuls `phase4_playthrough`, son préfixe `built_planet_panel` et `geoharvest_burst` bâtissent un `mz` parmi les rejeux connus des tests §6 ; les 15 rejeux du filet en bâtissent peut-être d'autres (couverts par `make e2e`/`sanitize-net`).
- **H18** : les preuves §6 rougissent : double `pq` de `jo` (R-a), `ga:pq` sans `ez` (R-b) ; R-d, R-e dans le budget.
- **H19** : revue des TU lents confirmée par l'implémenteur (grep ci-dessus) : aucune lecture nil par nom d'un non-pointeur de `mz` hors `p_`.
- **H20** : le crochet `PICO8_MEMBER_CENSUS_SEE` gardé, la table régénérée perdrait exactement les 17 lignes `pico8_mz_t` (non lancé, R8).
- **H21** : `pico8_mz_new`, `pico8_mz_pq`, `pico8_mz_hq`, `pico8_mz_kz`, `pico8_mz_ey`, `pico8_mz_pr`, `pico8_mz_ej`, `pico8_mz_kj`, `pico8_mz_jj_wrapper`, les accesseurs de classe gardent leurs signatures ; seul `pico8_mz_et` s'ajoute à l'API.
- **H22** : empreinte réelle comptée au commit.
- **H23** : sur une partie neuve sans `world()` (`start_game` seul), `ox` d'un `mz` neuf est vide (précédent `test_pico8_iv`), donc `hq` y laisse `ij` absent : la caractérisation fige ce cas (NULL lu absent, `kj` trace depuis (0,0)).
- **H24** : la cible jetable (`bv_et`) s'obtient par `hq` hors panneau (`rp` = (2,2), `near_anchor`, forme `test_hq`) et se relit par nom (`et`) ; son adresse sert à l'aller-retour par nom.
- **H25** : `pico8_d_` sur un `mz` lit `fp` `FIXED` 1 (pas `BOOL`) avant comme après, et `pico8_hy_nx(fm, 1)` inchangé (`EY_HASH`).
- **H26** : `dl` sur un `mz` dont `et` = `jj` lit `qs` « -too far- » par nom sur la cible, inchangé (`HQ_HASH`).

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Les nouveaux tests n'accèdent à aucun des 17 membres directement et n'appellent pas `pico8_mz_et` (qui n'existe pas sur l'ancien code) : ils compilent sur les deux formes.
- **Commentaires des nouveaux tests** : seulement les quatre sortes de `CLAUDE.md` — le sens d'un nom minifié, un écart connu avec le cart (`/* TROU-N */` nu), un piège qu'on réintroduirait (`assert` à message, sinon 40 mots), un contrat C que le nom ne dit pas. Pas de numéro d'étape, pas de récit ; le reste va dans le message de commit.
- **(G) Où : `test_pico8_mz`**, TU de la classe, rapide, dans `CENSUS_UNIT_TESTS`, lié par `ENGINE_SRCS` ; il a `world`, `start_game`, `build_mz`, `vec_ref`, `vec_ref_fixed`, `hq`, `ey`, `until_t`, `vec_of`, `get`, `has`, `must_fixed`, `bool_state`, `et_kind`, `near_anchor`, `first_with_me`, `check_fresh_mz`, `rank_in`, `listed`, `in_fresh_child` ; `must_abort` est à ajouter (forme de `test_pico8_jc`), les fils inline de `test_mz_is_a_struct` restent tels quels. Précédent de lieu : `test_jj_qs_by_name` (1b.3).
- **Caractérisation dirigée `test_mz_members_by_name`**, par `in_fresh_child` après `test_jj_qs_by_name`, hash **`MZ_MEMBERS_HASH`**. Lecture par le **lecteur brut** (`pico8_mz_struct_class()->field`, témoin `pico8_value_fixed(0x1234)`) **et** par `pico8_entity_get_field`, qui doivent concorder (retour, genre, valeur, pointeur) ; adresses jugées par `assert`, jamais pliées (rang dans `hv` ou dans une liste locale).
  1. **Neufs** : `pico8_mz_new(NULL)`, `pico8_mz_new` d'une table vide (H8), le `mz` de `bg:ldown` (second appel). Les 17 noms pliés : `nu`, `kk`, `jj` au **même pointeur** d'un `mz` à l'autre, `jj` = `pico8_mz_jj_wrapper()` ; `dq` `STRING` `"pr"` au même pointeur d'un `mz` à l'autre ; `ox` présent, distinct d'un `mz` à l'autre ; `dw` 0, `spawn_snd` 52, `ik` 49, `oi` 0, `od` 3, `fp` **`FIXED` 1**, `fa` 5 ; `ra`, `p`, `et`, `ij`, `p_` absents.
  2. **`hq`** sur une partie `world()` : `ra` = le `mp` passé (**même pointeur**) ; `p` présent, vecteur égal à `mp/128` ; `et` dans ses quatre états (`jc` au milieu du `jc` du rejeu, **même pointeur** que `cg.jc[1]` ; `jj` loin ; `false` près de l'ancre, `rp` dans le panneau, `BOOL` 0 présent ; jetable, `rp` = (2,2), tag `STRUCT` et classe `pico8_bv_et_struct_class()`) ; `ij` présent, élément de `ox` ; sur une partie neuve sans `world()` : `ij` **absent** (H23).
  3. **`kz`, `ey`, `pr`** : `oi` `FIXED` après un `dg` ; `ey` tôt : `p_` `BOOL` 1 ; `ey` payé : `p_` `BOOL` 0 **présent** ; `pr` avant 25 : `p_` `BOOL` 0 présent (repris de `test_ey`/`test_pr_ticks` seulement s'il ne coûte pas de rejeu de plus).
  4. **Écrivain, aller-retour par nom** : `ra`, `p` : un vecteur alloué → même pointeur, `cls->set_field` rend 1 ; `REF NULL` → absent, témoin intact ; **REF à union nulle** (`memset`, `kind = REF`) → absent. `et` : `false` → `BOOL` 0 présent ; **`false` à union nulle** → `BOOL` 0 présent ; le `jc` du rejeu, la cible jetable (H24), le `jj` → même pointeur ; `REF NULL` après chacun → absent ; REF à union nulle → absent.
  5. **`p_`** : `true` → `BOOL` 1 ; `false` → `BOOL` 0 présent ; **`false` à union nulle** → `BOOL` 0 présent ; `REF NULL` après `true`, après `false`, sur nil → absent, témoin intact.
  6. **Noms sans branche** : `cls->set_field` rend 0, sans rien toucher (valeur relue identique), pour les 13 autres noms (écriture d'un fixe 9) et pour `""`, `"RA"`, `"p "`, `"et_"`, `"has_et"`, `"has_p_"`, `"fo"`, `"qu"`.
  7. **Intrus** sur le lecteur : `""`, `"q"`, `"et "`, `"ET"`, `"has_et"`, `"has_p_"`, `"header"`, `"ke"`, `"fo"`, `"qs"` → 0, témoin intact.
  8. Tout plié dans **`MZ_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ (valeur imprimée par `check`), figé ensuite.
- **Test d'arrêt `test_mz_writer_stops_off_row`**, rouge d'abord, par `in_fresh_child` après la caractérisation ; `must_abort` ajouté, valeur hors ligne dans une globale ; cas à union nulle par `memset` puis `kind` :

  | cible | nom | valeurs hors genre |
  |---|---|---|
  | un `mz` neuf, écrivain | `ra`, `p` | fixe 1, **fixe 0 union nulle**, `true`, **`false` union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table`, REF liste (`pico8_hv()`), REF d'un `jo`, REF d'un `mz` |
  | idem | `et` | **`true`**, fixe 1, **fixe 0 union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table`, REF liste, REF vecteur, **REF d'un `jo`**, **REF d'un `qh`** (planètes, exclues par `dw` = 0), REF d'un `g_`, REF d'un `mz`, REF d'un `bg` |
  | idem | `p_` | fixe 1, **fixe 0 union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table`, REF vecteur, REF d'un `mz` |

  **Exclus** (dans le genre) : nil pour les quatre ; `false`, `false` union nulle, `jc`, jetable, `jj` pour `et` ; `true`, `false`, `false` union nulle pour `p_`. Rouge sur l'ancien code (H7), vert après : une ligne « Runs » pour chacun. Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_mz && ./tmp/test_pico8_mz` sauf mention, notées dans le commit avec leur test rouge ou la justification du trou ; un appel unique d'aide `static` se retire sous `if (0)` (1b.14b) :
  1. **(§7-1) indicateur forcé vrai** : le lecteur ignore `has_p_` (nil lu `BOOL` 0 présent) → rouge : `check_fresh_mz` (`!has(mz, "p_")`), `test_kz_ticks` (`bool_state == 0`). **(1b)** ignore `has_et` (nil lu `false`) → rouge : `check_fresh_mz` (`!has(mz, "et")`), caractérisation 1.
  2. **(§7-2) « présent 0 » là où c'était absent** — **sans objet dans sa lettre** (aucun fixe à indicateur : les 7 fixes sont nil `jamais`) ; forme la plus proche : la branche `ij` rend 1 avec `REF NULL` quand `ij` est NULL → rouge : `check_fresh_mz` (`!has(mz, "ij")`), caractérisation 2 (partie neuve).
  3. **(§7-3) init qui saute un membre typé** — **sans objet dans sa lettre** (aucune boucle de recopie) ; forme « pose une autre valeur » (précédents 1b.24, 1b.32–1b.35) : (a) R3 `od = 2` → rouge : `check_fresh_mz`, `test_ey` (crédits) ; (b) `fp = 0` → rouge : `check_fresh_mz` ; (c) `jj = NULL` → rouge : `check_fresh_mz`, `test_jj_wrapper` ; (d) `nu = g_mz_kk` → rouge : `check_fresh_mz` (`nu` = `{"mz"}`) ; (e) `has_et = 1` → rouge : `check_fresh_mz` ; (f) `dq = "ph"` → rouge : `check_fresh_mz`, `test_ey` (`me` = `pr`) ; (g) `self->ox = heap_ox` de `pq` sous `if (0)` → rouge : `check_fresh_mz` (`has(mz, "ox")`), `test_ox`.
  4. *Écrivain `ra` muet* (`if (0) mz_take_vec(…, &mz->ra)`, `return 1` gardé) → rouge : `test_hq` (`fold_vec(ra)`), `PLAYTHROUGH_HASH`, caractérisation 2. *`p` muet* → rouge : `test_hq`, `paid_mz_on_unknown` (`PR_HASH`), caractérisation 4.
  5. *Écrivain `et` muet* → rouge : `test_hq` (`ET_JC` attendu), `test_jj_wrapper`, caractérisation 2 et 4.
  6. *Écrivain `p_` muet* → rouge : `test_ey` (`broke` : `bool_state == 2` par `d_`), caractérisation 5.
  7. *Nil jugé sur le pointeur seul* (`value.as.ref == NULL` sans le genre) dans `mz_take_bool`, puis `mz_take_et`, puis `mz_take_vec` → rouge : caractérisation 4/5 (`false` union nulle lu absent) ; test d'arrêt (fixe 0 union nulle accepté en nil dans `ra`).
  8. *Prédicat* : (a) `mz_is_et_class` sans `jc` → rouge : `test_hq`, `test_jj_wrapper` (fils qui avorte) ; `test_pico8_jc` (R-e) dans le budget ; (b) sans `bv_et` → rouge : `test_hq` (jetable), `test_jj_wrapper` ; (c) sans `mz_jj` → rouge : caractérisation 4 (aller-retour du `jj`) ; (d) élargi aux planètes (`methods[LY] != NULL || cls == pico8_jo_struct_class()`) → rouge : test d'arrêt ; (e) `true` admis (`BOOL` sans la clause `== 0`) → rouge : test d'arrêt.
  9. *R6 `hq` : `jj` jamais posé* (`if (0)` sur la pose) → rouge : `test_hq` (`ET_JJ`), `test_ey`, `test_draw`, `test_jj_wrapper`.
  10. *R6 `hq` : `ij` = le premier élément de `ox` au lieu du plus proche* → rouge : `HQ_HASH` (rang de `ij` dans `hv`), `DRAW_HASH`.
  11. *R6 `hq` : `ij` jamais posé* → rouge : `test_hq` (`fold_hq` : `ij` REF attendu), `DRAW_HASH` (ligne depuis (0,0)).
  12. *R6 `ey` : `has_p_` non posé* → rouge : `test_ey` (`bool_state(early) == 2`) ; *R6 `pr` : seuil inversé* → rouge : `test_pr_ticks` (`PR_EXIT_TICK`).
  13. *R6 `kz` : `oi` non écrit* → rouge : `KZ_HASH`, `DRAW_HASH`.
  14. *Affichage : anneau dessiné sans condition* (`!pico8_mzd_truthy(pico8_mz_et(self))` → `1`) → rouge : `DRAW_HASH` (le `far` à `jj`).
  15. *Affichage : `has_kk` forcé à 0* → rouge : `DRAW_HASH` (anneaux de `pr`).
  - **Mutants équivalents, notés** : dans `hq`, `has_et = 1` omis à la pose du `jj` (`bv` vient toujours de lever `has_et` par `mz_take_et`) ; dans `kj`, `self->et != NULL` à la place de `pico8_mzd_truthy(pico8_mz_et(self))` (nil et `false` sont tous deux NULL) ; `*has = 1` avant l'`assert` d'une aide (fils qui avorte, état jamais relu) ; retrait du test de tag `STRUCT` avant `cls` (lecture de `->cls` d'une non-struct : comportement indéfini, pas un mutant discriminant).
- **`gcov`** : `test_pico8_mz` en `--coverage -O0`, piège `__gcov_dump` des fils (1a.23), sur `wormhole.c` et `wormhole_display.c`. Lignes attendues exécutées : poses R3, les 17 branches R4 (NULL et non NULL de `ra`, `p`, `ij` ; trois états d'`et` dans `pico8_mz_et` ; `has_p_` = 0 et 1), les 4 branches d'écrivain et leur `return 0`, les 3 aides (hors échec d'`assert`), le prédicat (ses trois clauses : un `jc`, la cible jetable, le `jj`), les lectures R6 de `hq`/`kz`/`ey`/`pr`/`ej`/`kj`. **Trous attendus** : branches NULL de `nu`, `kk`, `jj`, `ox`, `dq` (jamais NULL après construction) ; les gardes `self == NULL` existantes ; les échecs d'`assert` (fils de `must_abort`, prouvés par le test d'arrêt et les mutations 7–8). Rapport dans le commit.
- **Prior art** : `test_jj_qs_by_name` de `test_pico8_mz` (1b.3) ; `test_bv_et_members_by_name` et le test d'arrêt de `test_pico8_g_` (1b.22/1b.32, `g__take_vec`/`g__take_target`/`g__take_bool`) ; les ajouts de `test_pico8_jc` (1b.13, 1b.35) ; `test_pico8_js` (1b.34, `g_off_row`).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer.

- `./tmp/test_pico8_mz` : ~3–5 s (H5) ; environ 28 lancements (départ, arrêt rouge, final, 24 mutations, `gcov`) ≈ 6–7 min builds compris. Si un lancement dépasse 20 s, on joue d'abord 1, 1b, 3e, 3g, 4, 5, 7, 8a, 8d, 9, 11, et on note les autres non lancées.
- §6 R-a, R-b : deux lancements chacun (bug remis, restauré), quelques secondes par rejeu plus un build ; R-d, R-e dans le budget seulement.
- `make test` une fois ; `make e2e` une fois, **dans la même invocation** que `make class-coverage-net` ; `make sanitize-net` une fois, jamais en même temps que `e2e` ; `make value-members`.
- Ordre de priorité si le budget manque : cibles finales 1–5 ; départ, arrêt rouge, final ; R-a, R-b ; mutations 1, 1b, 3e, 4, 5, 7, 8a, 8d, 9 ; le reste noté « non lancé ».
- Aucun TU lent n'est modifié ni ne lit `mz` au-delà d'un `hb` (`test_pico8_ie`) : `make e2e` les lance.

| # | Commande exacte | État de l'arbre (départ / final / mutation n / §6 R-x bug remis / §6 R-x restauré) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` vert, les **8** TU de `UNIT_TESTS_SLOW` verts ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **10**, aucune ligne `pico8_mz_t` ;
5. `timeout 15m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` — requis, `pico8_mz_struct_new` change (sans toucher `pico8_class_coverage_record` ni `KNOWN[]`).

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements (les fixtures de `test_pico8_mz` et des tests §6 sont seulement **lues**).

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet**

- `FAILED:` si 1b.35 n'est pas commitée au départ, ou si `make value-members` ne rend pas 27 au départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, ou si le diff touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements.

**Types, tables, écrivain**

- `FAILED:` (« champ sans règle ») si une aide avorte dans un rejeu ou un TU hors des fils de `must_abort` (H3), ou si un type est élargi au lieu d'avorter : `true` ou une planète admis dans `et` ; une struct, une table ou une liste admise dans `ra`/`p` ; un non-booléen dans `p_` ; un `has_` ajouté pour tenir autre chose ; `pico8_value_t` gardé dans `pico8_mz_t`.
- `FAILED:` si l'un des 17 membres n'a pas le type retenu à sa place, si `has_et` ou `has_p_` manque ou n'est pas juste après son membre, ou si un autre `has_` est ajouté (`has_od`, `has_ij`, `has_ra`…).
- `FAILED:` si `mz_is_et_class` n'admet pas exactement `jc`, `bv_et` et `mz_jj` (par leurs accesseurs publics), s'il est rendu public ou partagé avec un autre fichier ; si `ij` reçoit une aide, un prédicat ou un `assert` sans qu'une note « Découvert à 1b.35 » l'exige (H2) ; si `wormhole.c` gagne une autre inclusion que `pico8_slipway.h`, ou `wormhole.h` une inclusion.
- `FAILED:` si nil et la valeur de `ra`, `p`, `et` (`false` compris), `ij`, `p_` ne se lisent plus par nom exactement comme en 1a (absent ; REF au même pointeur ; `BOOL`), ou si un des 7 fixes, `dq`, `nu`, `kk`, `jj`, `ox` change de genre, de valeur ou d'identité (`fp` reste `FIXED` 1).
- `FAILED:` si `pico8_mz_et` manque, n'a pas le contrat de `pico8_g__et`, ou si le lecteur ou `pico8_mz_kj` lit `et` autrement.
- `FAILED:` si la `members[]` du lecteur, un `offsetof`, `<stddef.h>`, `pico8_ref_audit.h`, `PICO8_REF_AS_LIST_CONST` ou une lecture/écriture `*(pico8_value_t *)` existe encore dans `wormhole.c`.
- `FAILED:` si l'écrivain de `mz` n'accepte plus exactement `ra`, `p`, `et`, `p_`, ou si le message de l'`assert` du constructeur change.
- `FAILED:` si une aide viole la forme de 1b.13/1b.15 (état nil d'abord ; genre **et** pointeur testés ; union nulle hors genre qui avorte ; message qui nomme `mz.<m>` et le genre ; `has_` posé en dernier ; tag `STRUCT` vérifié avant `cls`), si elle est rendue publique ou partagée avec un autre fichier.
- `FAILED:` si `pico8_mz_jj_t`, `g_mz_jj_wrapper` ou `pico8_mz_jj_field` changent, ou si une fonction publique de `wormhole.h` change de signature.

**Diff et preuves**

- `FAILED:` si un site par nom change (`bv`, `dl`, `d_`, `entity.c`, `pico8_mz_field_vec`, `pico8_mzd_field_vec`…), ou si un site par nom est remplacé par un accès direct.
- `FAILED:` si un autre fichier de `src/` que `wormhole.h`, `wormhole.c`, `wormhole_display.c` change, ou `tools/`, le `Makefile`, `CENSUS_UNIT_TESTS`, `UNIT_TESTS_SLOW`.
- `FAILED:` si un autre test que `test_pico8_mz.c` change ; si `test_pico8_mz.c` retire ou modifie une ligne existante ; si une valeur figée existante change (H13).
- `FAILED:` si un test de régression (§6) a un diff, ou s'il manque les deux lignes « Runs » (rouge bug remis, vert restauré) de R-a et R-b ; pour R-d, R-e, soit leurs deux lignes, soit « non lancé » au journal.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas l'attendu, si `make sanitize-net` n'est pas `15 propre(s)`, si `make value-members` ne rend pas 10 ou si un autre typedef gagne ou perd une ligne, ou s'il manque la ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un crochet du recensement change, ou si `tools/member_census.txt` ou `tools/float_audit_c1.txt` est édité.

**Tests de l'étape**

- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, lit ou écrit un des 17 membres directement, appelle `pico8_mz_et`, ne couvre pas les 17 noms, les quatre états non nil d'`et` et son nil, `ij` présent et absent, `ra` au pointeur de `mp`, les états de `p_` (dont `false` à union nulle), la REF à union nulle de `ra`/`p`/`et`, les noms sans branche et les intrus ; ou si `MZ_MEMBERS_HASH` change après.
- `FAILED:` si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'arbre de départ et une verte sur l'arbre final, ou s'il n'a pas les cas union nulle `memset`, `true` et une planète dans `et`, fonction NULL, chaîne NULL.
- `FAILED:` si un commentaire ajouté dans `test_pico8_mz.c` ou `src/` sort des quatre sortes de `CLAUDE.md`.
- `FAILED:` si le commit ne porte pas le rapport `gcov` et les mutations 1 à 15 (avec 1b, 3a–3g, 8a–8e), chacune avec son test ou la justification de son trou, plus les mutants équivalents.

**Commit et note**

- `FAILED:` si le travail n'est pas en un commit vert.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé (TROU-3, TROU-7, TROU-12 gardés à l'identique ; le repli (0,0) de `ij` nil gardé).
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, ou s'il manque le tableau « Runs » ou la note « Découvert à 1b.36 ».

## Out of Scope

- `mz_jj` (1b.3), fait : ni son type, ni son lecteur, ni l'absence d'écrivain.
- Les homonymes : `g_.ra`/`p`/`et`/`p_`/`od`/`fp`/`fa`/`dw`/`ik`/`nu`/`dq` (1b.32), `jc.et`/`od`/`fp`/`fa`/`dw`/`ik`/`nu`/`p_` (1b.35), `qh.et`/`od`/`fp`/`dw` (1b.24), `jo.ra`/`p`/`p_`/`fa`/`nu` (1b.23), `ui.lheld`/`o_`/`qa` (1b.37, qui pointent vers des `mz` ou leur passent `mp`).
- Les prédicats de 1b.35 (`jc_is_fo_holder_class`), de 1b.22 (`bv_et_is_holder_class`), de 1b.23d (`jo_is_fo_holder_class`) : lus, pas touchés.
- `pico8_bv`, `pico8_dl`, `pico8_d_`, `pico8_ij` et les autres sites par nom.
- Corriger TROU-7 (`ox`, `p` jamais libérés), TROU-3 (`pr`), TROU-12 (anneau noir), ou le repli (0,0) de `ij` nil.
- Déplacer un TU entre listes ; régénérer `tools/member_census.txt` ; `make ref-audit`.
- Fichiers d'événements, `FRAMES_EVENTS`, rendu, audio, appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur (G)** : modules `wormhole.h` (typedef, 2 `has_`, `pico8_mz_et` déclaré, aucune inclusion), `wormhole.c` (lecteur, écrivain, 3 aides `mz_take_*`, prédicat `mz_is_et_class`, `pico8_mz_et`, R6, `pico8_slipway.h` ajoutée, `<stddef.h>` et `pico8_ref_audit.h` retirées), `wormhole_display.c` (R6, `pico8_mzd_fixed` supprimée, deux `assert` de tag de lecture retirés) ; `et` → `pico8_struct_header_t *` + `has_et`, ensemble = ligne exactement ; `ij` → `pico8_struct_header_t *` sans aide ; `ra`, `p` → `pico8_vec_t *` ; `p_` → `int` + `has_p_` ; `jj` → `pico8_mz_jj_t *` ; `nu`, `kk` → `pico8_ob_table_t *` ; `ox` → `pico8_list_t *` ; `dq` → `const char *` ; 7 fixes → `pico8_fixed_t` ; tests dans `test_pico8_mz` par ajouts ; pas de découpage ; §6 : R-a, R-b obligatoires, R-d, R-e dans le budget.
- **Note « Découvert à 1b.36 »** (à écrire sur la ligne ; le reste va dans le commit) :

  > patron 1b.1 appliqué ; `dw`, `spawn_snd`, `ik`, `oi`, `od`, `fp`, `fa` → `pico8_fixed_t` (littéraux, `od` sans indicateur : `stat?` d'homonyme), `dq` → `const char *`, `nu`, `kk` → `pico8_ob_table_t *`, `jj` → `pico8_mz_jj_t *` (1b.3), `ox` → `pico8_list_t *`, `ra`, `p` → `pico8_vec_t *`, `p_` → `int` + `has_p_` ; **`et` : lecture de 1b.24 sans écart** (`pico8_struct_header_t *` + `has_et`, `true` avorte, `pico8_mz_et` appelé aussi par `kj`), **ensemble = la ligne exactement (`jc`, `bv_et`, `mz_jj`), planètes exclues par la preuve de 1b.23d (`dw` = 0)**, prédicat `mz_is_et_class` par trois accesseurs publics ; **`ij` : lecture de 1b.35 étendue à une source `pico8_value_t` déjà gardée (`pico8_ij`), ni aide ni prédicat ni `assert`** ; R5 écrivain de 1b.13/1b.15 sur 4 membres (`mz_take_vec/et/bool`), boucle de clés vacante ; **`members[]` du lecteur supprimée, plus aucun `offsetof`, `<stddef.h>` ni `pico8_ref_audit.h` dans `wormhole.c`** ; `assert` de tag de lecture de `ij`/`kk` retirés (1b.20) ; interaction 1b.35 : `jc.fo` ∋ `mz` et `mz.et` ∋ `jc` se répondent ; §6 : `playability`, `geoharvest_burst` sur le chemin, prouvés rouges sans diff ; empreinte réelle mesurée au commit.

- Le message de commit porte : compte 27 → 10, résumé « Runs », `gcov`, mutations et trous, `MZ_MEMBERS_HASH`, réponses à H1–H26.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
