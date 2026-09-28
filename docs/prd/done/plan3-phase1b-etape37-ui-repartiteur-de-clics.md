---
title: "Plan 3, étape 1b.37 : `ui`, le répartiteur de clics, et la sortie de la phase 1b. Ses 10 membres typés ; `o_`, `gj` en pointeurs d'en-tête sans aide (lecture de 1b.35, source déjà gardée), `lheld` en pointeur d'en-tête à aide et prédicat (1b.14a + 1b.22, la valeur vient d'un `ldown`), `rheld` en `pico8_bg_t *` (R1, cible unique) ; défauts `ql`/`fa`/`le`/`hw` en `static` déballés de la spec, `g_ui_defaults` et la `members[]` du lecteur supprimées, plus aucun `offsetof` dans `src/` ; `make value-members` vide, `class-coverage-net` identique, `sanitize-net` propre"
description: "Après 1b.36, `pico8_ui_t` garde les 10 derniers membres `pico8_value_t` du moteur : 4 défauts copiés de la spec par une table d'offsets (`ql`, `fa`, `le`, `hw`), ce que `lj`/`gw` allouent à chaque frame (`qa`, `ic`), ce que `gw`/`lj` rangent depuis `cv.hd` (`o_`, `gj`) et ce que `bh` range depuis le `ldown`/`rdown` d'une autre entité (`lheld`, `rheld`). 1b.37 les type selon R1–R9 de 1b.1, sans écrivain par nom ni clé (R5 vacante dans ses deux moitiés, sauf l'aide par valeur de 1b.14a dans `bh`). `msgbox.c` (`pz` écrit `ok.le`), `test_pico8_jc:444` (écrit `ok.o_`) et `test_pico8_ui:742-745` (lisent `le`/`hw`) changent dans la seule forme de leur accès. Un seul commit, qui constate la sortie de la phase 1b : `make value-members` passe de 10 à 0, liste vide."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.37 — `ui` (`ui.h:21`), `o_`/`gj`/`lheld`/`rheld` pointent vers presque toutes les structs précédentes. Empreinte 25 (6, 19). Compte : 0. Sortie de phase : `make value-members` vide, `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt`, `make sanitize-net` propre.

Cadre : puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b » (la **Sortie** : « aucune struct d'entité ne contient de `pico8_value_t` — constaté par une commande nommée […] ; sortie vide »), §3 (`make test` vert, aucun `*.sha256` modifié), §6, §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` « toujours à la sortie de phase (note de sortie : une passe identique à `tools/class_coverage_net_expected.txt`) »). Modèles : le PRD de 1b.35 (`docs/prd/done/plan3-phase1b-etape35-jc-voie-de-glisse.md`, implémenté par **9fdf1dc**, forme de référence) et celui de 1b.36 (`docs/prd/triage/plan3-phase1b-etape36-mz-trou-de-ver.md`). Notes « Découvert à » **citées, pas redérivées** ; elles lient ce PRD :

- **1b.1** : patron R1–R9 ; s'en écarter est une lecture nouvelle, notée. R1 : `ref:pico8_X_t` → `pico8_X_t *` ; R2 : un pointeur n'a pas d'indicateur (NULL = nil).
- **1b.5** : preuve `grep 'pico8_value_t \*' … | grep -v out_value` ; **1b.9** : la même preuve garde les paramètres de sortie d'un autre nom (ici `out_kn` de `pico8_ui_jz`).
- **1b.7** : un nil `stat?` d'homonyme ne donne pas d'indicateur.
- **1b.9/1b.20/1b.31/1b.33** : R7 forme `offsetof` — une table vidée est supprimée avec son seul client (la `members[]` locale du lecteur), branches R4 `strcmp` dans l'ordre du typedef ; `<stddef.h>` retiré quand plus aucun `offsetof`.
- **1b.10** : `ref:autre` = REF non NULL de tag `UNKNOWN` ; **1b.20** : `ref:autre` vers une boîte → `pico8_qz_box_t *`.
- **1b.11** : R2 des TU lents — relus plus bas.
- **1b.13** (forme de l'aide R5) et **1b.15** : aide `static` par genre, au nom de la struct, par valeur ; état nil **d'abord**, sortie sur `REF NULL` (genre **et** pointeur), sinon `assert` « champ sans règle » qui nomme `struct.membre`, le genre de la ligne et l'arrêt ; pour `ref:pico8_X_t`, tag `STRUCT` **puis** `cls == pico8_X_struct_class()` (accesseur public) ; **test d'arrêt rouge d'abord**.
- **1b.14a** : **R5 sur une méthode — une valeur obtenue par nom sur une autre entité puis rangée passe par l'aide par valeur de 1b.13** (`ke_take_string`) ; appliquée par 1b.35 à `jc.mt` (`jc_take_fixed`). **R7 d'une table de défauts** : chaque défaut déballé une fois à l'init dans un `static <type> g_<X>_<m>`, même pointeur que la spec, remis à nil par `pico8_<X>_defaults_reset`, posé par affectation typée dans le constructeur.
- **1b.14b** : une mutation qui retire l'unique appel d'une aide `static` se plante sous `if (0)` (`-Werror`) ; cas « union nulle » par `memset` puis `kind`.
- **1b.16** : R7 de 1b.14a sur une table **sans fille** ; l'état de classe que le cart lit garde son accesseur public (ici `pico8_ui_hw()`, lu par `bv`).
- **1b.22** : **REF à plusieurs cibles → `pico8_struct_header_t *`**, l'ensemble porté par l'`assert` de l'aide R5 ; **ensemble admis = ligne ∪ classes productrices prouvées par le code et exercées par la caractérisation sur l'ancien code** ; **prédicat par accesseurs publics quand tous sont liés partout où le fichier l'est, sinon par une propriété du descripteur que l'ensemble partage exactement** (`DATA_ENGINE_SRCS` lie `ui.c` sans `planet_family_trade.c`, `help.c` ni `research_icon.c`).
- **1b.23a1** : **quand un fichier déballe plus de trois défauts, une aide `static` de lecture de spec par genre et par fichier** remplace les `assert` en ligne (précédent exact : `help_default_fixed/vec/table/box`, 1b.33).
- **1b.23a2** : **un test qui accède directement à un membre typé change dans la seule forme de son accès et porte sa preuve rouge** (précédents `test_pico8_jc:874` à 1b.32, `test_pico8_g_:494` à 1b.35) ; boucles sentinelles : aucune ne vise `ui`.
- **1b.23b** : le grep des accès directs couvre `x->m` quelle que soit l'expression `x` (ici `pico8_ok()->o_`).
- **1b.24** : preuve de typedef par `awk` (pas `sed`).
- **1b.29/1b.32/1b.34/1b.35** : un test de régression **sur le chemin** du membre typé, sans diff, est prouvé rouge ; recettes de bugs de référence reprises telles quelles.
- **1b.30/1b.31/1b.33/1b.34** : caractérisation dans le TU rapide de la classe.
- **1b.35** : **lecture nouvelle de 1b.22 : un membre à plusieurs cibles écrit seulement par des méthodes, depuis des pointeurs typés, n'a ni aide ni prédicat — son ensemble n'est gardé que par le type**. Précédent de forme du commit : **9fdf1dc**.
- **1b.36** (supposée commitée, H1–H2) : **lecture étendue de 1b.35 à une source `pico8_value_t` dont le genre est déjà gardé** (`mz.ij`, l'`assert` de `pico8_ij`).

### État de départ

HEAD `46992b9` (1b.35 gradée) au moment de l'écriture ; 1b.36 est implémentée **en parallèle** (l'arbre de travail modifie `src/pico8_wormhole.[ch]`, `src/pico8_wormhole_display.c`, `tests/test_pico8_mz.c` et `docs/plan-3.md`, lus sans être touchés). **Précondition : 1b.36 (`mz`) est commitée avant que 1b.37 commence (H1)** ; `make value-members` rend alors **10**, toutes `pico8_ui_t`. Numéros de ligne indicatifs (arbre `46992b9`) ; chaque site est aussi désigné par son contenu. **Rien n'a été compilé ni lancé pour ce PRD** : les faits viennent de `grep` et de la lecture ; tout fait d'exécution est une hypothèse (H1–H26).

### Lignes de recensement (`tools/member_census.txt:224-233`)

| membre | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|
| `ql` | `fixe` | jamais | - | l | `pico8_fixed_t` (`g_ui_ql`, 58 de la spec) | non |
| `fa` | `fixe` | jamais | - | l | `pico8_fixed_t` (`g_ui_fa`, 15 de la spec) | non |
| `le` | `ref:liste` | observé | stat | l | `pico8_list_t *` (`g_ui_le`, la liste vide de la classe ; `cj`, `pz` la remplacent) | non (pointeur) |
| `hw` | `ref:autre` | jamais | - | l | `pico8_qz_box_t *` (`g_ui_hw`, la boîte de la spec) | non (pointeur) |
| `qa` | `ref:autre` | observé | lu | l | `pico8_vec_t *` (alloué par `lj`, TROU-7) | non (pointeur) |
| `ic` | `ref:table` | observé | lu,stat | l | `pico8_ob_table_t *` (alloué par `gw`, TROU-7) | non (pointeur) |
| `o_` | `ref:` bg, fq, fr, fv, hj, ie, jo, js, mx, nl, nr, os, pm, qh | observé | lu,stat | l | `pico8_struct_header_t *` (écrit par `gw` seul) | non (pointeur) |
| `gj` | `ref:` les mêmes sauf fv | observé | lu | l | `pico8_struct_header_t *` (`lj` : `gj = o_`) | non (pointeur) |
| `lheld` | `ref:` fr, g_, hj, jc, mx, mz, nl, nr, os, qh | observé | lu | l | `pico8_struct_header_t *` (écrit par `bh` seul, aide `ui_take_held`) | non (pointeur) |
| `rheld` | `ref:pico8_bg_t` | observé | lu,stat | l | `pico8_bg_t *` (écrit par `bh` seul, aide `ui_take_bg`) | non (pointeur) |

Compte : 10 − 10 = **0**. Aucun indicateur (aucun non-pointeur n'est nil ; `o_`, `gj`, `lheld`, `rheld` ne tiennent jamais `false`, H6). **Aucune ligne n'a `e`** : `ui` n'a pas d'écrivain par nom (`set_field` NULL, `must_abort(write_o_)` existant) ni de clé de construction (`assert` « the only measured road is `pico8_ui_new(NULL)` ») : R5 vacante dans ses deux moitiés, hors l'aide par valeur de 1b.14a dans `bh`.

### Le cart (`pico8/lua.lua`)

- `ui=ke:jr([[ql=58,fa=15,le=o(),hw=b(10,10,118,118),]])` (`:212-217`).
- **`ui:lj`** (`:217-224`) : `local le=self.le` ; `self.qa=je.ra+mo.ra` ; `self:gw()` ; `if #le==0 or gu(le,self.o_) then self:bh("l") end` ; `self:bh("r")` ; `if je.l.jn and self.le==le then self:cj() end` ; **`self.gj=self.o_`**.
- **`ui:ev`** (`:225`) : `spr(self.rheld and 9 or self.o_ and self.o_.ql or self.ql, je.ra.x, je.ra.y)`.
- **`ui:gw`** (`:226-236`) : **`self.ic,self.o_={}`** (donc `o_` = nil) ; pour `e` de `cv.hd`, `if mv(e,"hd",self.qa,je.ra)` → `ft(self.ic,e.fa,e)`, **`self.o_=e`** ; `(self.gj or {}).m_=false` ; `(self.o_ or {}).m_=true` ; `poke(0x5f80, …self.o_.fg…)`.
- **`ui:bh(bt)`** (`:237-247`) : `local b,ni=je[bt],bt.."held"` ; `if b.jn` → `pw=self:jz(bt.."down",self.qa,je.ra)` ; **`if pw and type(pw.kn)=="table" then self[ni]=pw.kn end`** ; `mv(held,"hq",self.qa,je.ra)` ; `if b.hm and held` → `mv(held,"ey",nw and nw.kv)`, **`self[ni]=nil`**.
- `ui:jz` (`:248-255`) : couches 15 → 0 de `self.ic`, première réponse vraie. `ui:cj` (`:256-257`) : `nv(self.le,he)`, **`self.le={}`**. `pz` (`:280-288`) : **`ok:cj()`, `ok.le=nv(bs,…)`**, `if (p.ff) ok.le={}`.
- **Producteurs de `lheld`** (un `ldown` qui rend une table) : `jo:ldown` (`:412-416`, `me=="ly"` → `jc({qu=self})`), `nr:ldown` (`:1112-1113`, `jc`), **`bg:ldown`** (`:376-377`, `(#cg.g_>0 and mz or g_)()`), **`ie:ldown`** (`:932-934`, `self.ke({id=…})`, où `ie.ke` ∈ `gx={qh,os,nr,fr,nl,mx,hj}`, `:1182-1185`). `js:ldown`, `fq:ldown`, `jo:ldown` de `known` rendent `true`, jamais rangé. **`pm` n'est dans aucun `gx`** (il naît de `kv[hp.cw]`) : la ligne `lheld` est exactement l'ensemble des producteurs. **Producteur de `rheld`** : `bg:rdown` (`:378-380`, `return self`), seul `rdown` du cart.
- `o_` : un élément de `cv.hd` survolé (`gk` ou `hd=1`) ; `ga` y est (hérite `jo.hd`) mais sa `kq` vide ne le laisse jamais survoler (H26 de 1b.35), d'où son absence de la ligne.
- Lecteurs par nom hors `ui:*` : `jc:hq` (`ok.o_.gv`, `:622`), `dc` (`fa` de `cv.ev`), `bv` (`ui.hw(rp)`, sur la classe).

### Le code

**`src/pico8_ui.h`** : typedef `pico8_ui_t` (`:19-31`), `ke` puis 10 `pico8_value_t` ; commentaire de tête (« TROU-7 » ; « `le` of a new `ui` is the class's own empty list, shared until `cj` or `pz` replaces it; `hw` is the class's box, borrowed ») qui reste vrai. Inclusions : `geom` (→ `vec`), `gfx`, `ke`, `mem`, `ob` (→ `list`), `struct`, `value` ; **pas `pico8_bg.h`**. API publique inchangée (`pico8_ui_new`, `pico8_ui_hw`, `pico8_ui_default`, `pico8_ui_lj`, `_ev`, `_gw`, `_bh`, `_jz`, `_cj`, `pico8_ok`, `pico8_je`…).

**`src/pico8_ui.c`** :

- **`g_ui_defaults[]`** (`:22-35`, enum `UI_QL`…`UI_HW`, 4 entrées `{name, offsetof, value}`), sa remise à nil (`pico8_ui_defaults_reset`, `:37-42`), **sa boucle de recopie dans le constructeur** (`:179-181`, `*(pico8_value_t *)((char *)self + offset) = value`), **sa boucle de déballage à l'init** (`:219-223`) et l'`assert` groupé qui suit (`:224-227`, `le` liste et `hw` non NULL). `pico8_ui_hw()` (`:44-46`) lit `g_ui_defaults[UI_HW].value.as.ref`.
- **Lecteur `pico8_ui_field`** (`:134-162`) : `static` `members[]` de 10 `offsetof` et sa boucle (`REF NULL` → 0), repli `pico8_ke_field`.
- **`pico8_ui_struct_new`** (`:172-197`) : `malloc`, `pico8_ke_struct_init`, recopie des 4 défauts, 6 `REF NULL`, `assert` « construction key » (texte gardé), couverture, `pico8_ke_struct_pq`, `PICO8_MEMBER_CENSUS_SEE`.
- `pico8_ui_lj` (`:294-333`) : `self->le` (tag et `PICO8_REF_AS_LIST_CONST` deux fois), `self->qa = pico8_value_ref(heap_qa)` (TROU-7), `pico8_gu(…, self->o_)`, identité `self->le.as.ref == le_ptr_before`, `self->gj = self->o_`.
- `pico8_ui_ev` (`:337-362`) : `pico8_ui_truthy(self->rheld)`, `self->o_` puis `ql` de `o_` par nom, repli `self->ql` (`kind` testé).
- `pico8_ui_gw` (`:367-424`) : `self->ic = pico8_value_ref(ic)` (TROU-7), `self->o_ = pico8_value_ref(NULL)`, `qa_val = self->qa`, **`self->o_ = pico8_value_ref(e)` pour `e = hd_bucket->items[i].as.ref`**, `gj_val`/`o_val` (écritures `m_` par nom sur eux).
- **`pico8_ui_bh`** (`:426-482`) : `pico8_value_t *held = &self->lheld | &self->rheld` ; **`*held = kn`** si `jz` rend une réponse vraie de genre REF ; `held_inst` pris de `held_val.as.ref` sous **`/* TROU-6 */`** (`:462`) ; `mv(held_inst, "hq", …)`, `ey`, `*held = pico8_value_ref(NULL)`.
- `pico8_ui_jz` (`:487-525`) : `self->ic` (tag, cast) ; le `PICO8_REF_AS_LIST_CONST` d'un **seau** de `ic` (`:510`) n'est pas un membre et reste.
- `pico8_ui_cj` (`:539-555`) : `self->le` (`PICO8_REF_AS_LIST_CONST`), `self->le = pico8_value_ref(heap_le)` (TROU-7).
- `offsetof` : 9 lignes (4 dans `g_ui_defaults`, 5 dans `members[]`), **les seules de tout `src/`** (grep) ; `<stddef.h>` pour elles et les `size_t i` ; `pico8_ref_audit.h` pour les 4 `PICO8_REF_AS_LIST_CONST`. Inclusions : ni `slipway.h`, ni `probe.h`, ni `wormhole.h`, ni `planet_family.h`, ni `bg.h`.

**`src/pico8_msgbox.c`** (`pico8_pz`, `:549-574`) : **`ok->le = pico8_value_ref(le_heap)`** et **`ok->le = pico8_value_ref(empty_heap)`** (accès direct, `ok = pico8_ok()`).

**Sites par nom, inchangés** : `pico8_jc_hq` (`slipway.c:220`, `pico8_jc_entity_instance(ok, "o_")`), `pico8_dc` (`entity.c`, `fa` de `cv.ev`), les lectures par nom de `ql`/`fg`/`fa`/`ra` **sur d'autres entités** dans `ui.c` (`o_`, `je`, `mo`, éléments de `cv.hd`), les écritures `m_` par nom sur `gj`/`o_`. `bootstrap.c` déclare un `pico8_ui_t *` sans toucher de membre.

**Tests, accès direct à un des 10 membres d'un `ui`** (grep `(->|\.)(M)\b` sur tous les `tests/*.c`, `x` quelconque, 1b.23b) : **`tests/test_pico8_jc.c:444`** (`hq_over` : **`pico8_ok()->o_ = pico8_value_ref(hovered);`**, écriture) et **`tests/test_pico8_ui.c:742, 743, 745`** (`test_ui_is_a_struct` : `ok->hw.as.ref`, `ok->le.as.ref`, `other->le.as.ref`, `other->hw.as.ref`, lectures). Le reste (`test_pico8_je/mo/nf`) déclare un `pico8_ui_t *` sans accès. Aucune boucle sentinelle ni `STOP` ne vise `ui`.

`tests/test_pico8_ui.c` (TU de la classe, rapide, ~1–4 s mesuré de 1b.23 à 1b.28, dans `UNIT_TESTS` et `CENSUS_UNIT_TESTS`, lié par `ENGINE_SRCS`) fige déjà beaucoup par nom : `test_built` (défauts 58/15, `le`/`hw` = ceux de la classe, `absent[]` ∋ `qa`, `ic`, `o_`, `gj`, `lheld`, `rheld`), `test_shared`, `test_fresh` (`qa`, `ic`, `gj == o_ == bg`), `test_hover`, `test_layers`, `test_right` (`rheld` = le `bg`, curseur 9), `test_left` (`lheld` = un `jc`), `test_cj`, `test_stale`, deux rejeux, `test_ui_is_a_struct` (descripteur, `must_abort(write_o_)`, clés, `bh_on_x`).

### Revue des TU lents (R2 de 1b.11)

`grep -nE '"(ql|le|hw|qa|ic|o_|gj|lheld|rheld)"|pico8_ok\(\)|pico8_ui_'` sur `test_pico8_{ie,jo,qh,os,hj,pm,mx,nr}.c` : seul **`test_pico8_ie`** lit par nom `o_` et `lheld` (`replay`, chaque placement que `ie:ldown` confie à `ok.lheld`) — deux pointeurs, NULL suffit : **aucun indicateur** (H19). Ce TU lent exerce, dans `make e2e`, la clause `LY` du prédicat de `lheld` par de vrais `ie:ldown`.

## Solution

1. **Réécritures de forme** (au commit, pas avant : elles ne compilent que sur le nouveau code) : `test_pico8_jc.c:444` → `pico8_ok()->o_ = hovered;` ; `test_pico8_ui.c:742, 743, 745` → comparaisons de pointeurs typés. Chacune avec sa preuve rouge (§6 ci-dessous).
2. **Caractérisation figée sur l'ancien code** (arbre 1b.36 commité), dans le TU rapide `test_pico8_ui`, par nom seul ; verte avant tout typage.
3. **Test d'arrêt rouge d'abord** (`lheld`, `rheld` hors ensemble, rangés par `bh`) dans le même TU.
4. **Commit 1b.37** : les 10 membres typés, sans indicateur ; `g_ui_defaults`, son enum et ses deux boucles supprimés, 4 `static` déballés par 3 aides de spec ; `members[]` du lecteur supprimée ; lecteur à 10 branches ; 2 aides `static` de rangement (`ui_take_held`, `ui_take_bg`) et 1 prédicat `static` (`ui_is_held_class`) ; `msgbox.c` écrit `ok->le` typé ; **plus aucun `offsetof` dans `src/`**, plus de `<stddef.h>` dans `ui.c` ; `make value-members` **vide**.
5. **Sortie de la phase 1b constatée** dans le même commit : les trois cibles de la ligne, plus `make frames-check`, et un paragraphe « Sortie de la phase 1b, constatée à 1b.37 » dans le plan (forme de celui de 1a.42e).
6. Aucun pixel ne bouge. Note « Découvert à 1b.37 » sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `ui.h`, je veux `pico8_ui_t` sans `pico8_value_t`, chaque membre au type de sa ligne, et la phase 1b close : plus aucune struct d'entité ne tient de `pico8_value_t`.
2. En tant que lecteur par nom (`jc:hq`, `dc`, les TU `ui`, `jc`, `bg`, `js`, `fv`, `fq`, `ie`, `mo`, `lq` et les tests de régression), je veux que chacun des 10 noms rende sur `ok` exactement ce que rendait l'ancien code : présence, genre, valeur, identité du pointeur.
3. En tant que `ui:lj`, je veux `qa` neuf à chaque frame, `gj` = `o_` en fin de frame, et `cj` seulement quand `le` n'a pas changé pendant la frame (identité de pointeur).
4. En tant que `ui:gw`, je veux `ic` neuf, `o_` éteint puis posé sur le dernier survolé de `cv.hd`, `m_` écrit sur l'ancien `gj` et le nouveau `o_`.
5. En tant que `ui:bh`, je veux ranger dans `lheld` le `jc`, le `g_`, le `mz` ou la planète que me tend un `ldown`, dans `rheld` le `bg` que me tend `rdown`, leur envoyer `hq` puis `ey`, et les éteindre au relâchement.
6. En tant que `ui:ev`, je veux le curseur 9 tant que `rheld` est tenu, sinon le `ql` de `o_`, sinon 58.
7. En tant que `pz` (`msgbox.c`), je veux remplacer `ok.le` par la liste de boutons que je viens de bâtir, puis par une liste vide si `p.ff`.
8. En tant que `bv`, je veux lire la boîte `ui.hw` de la classe, même pointeur que la spec.
9. En tant que gardien de `lheld` et `rheld`, je veux qu'une REF hors ensemble tendue par un `ldown`/`rdown` avorte (« champ sans règle ») : un `pm`, un `ga`, un `jo`, un `bg` dans `lheld` ; un `jc`, un `g_`, une planète dans `rheld`.
10. En tant que mainteneur, je veux `g_ui_defaults`, `members[]`, tout `offsetof` de `src/` et `<stddef.h>` de `ui.c` absents.
11. En tant que tests de régression `slipway_drag`, `playability`, `pause_menu_music`, `built_planet_panel_nil_row` (sans diff) et TU `jc`, `ui` (une ou trois lignes réécrites), je veux rester verts et rouges quand mon bug revient.
12. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
13. En tant que `plan-pilot`, je veux `make value-members` vide, `make class-coverage-net` identique, `make sanitize-net` propre, et la phase 1b déclarée close dans le plan, pour passer à la phase D.
14. En tant que vérificateur, je veux la caractérisation verte sur l'ancien code, le test d'arrêt rouge puis vert, les preuves rouges §6, les mutations et leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».
15. En tant que responsable des phases suivantes, je veux la liste de l'infrastructure que la sortie de 1b rend sans objet, sans qu'elle soit supprimée ici.

## Implementation Decisions

Pas d'humain dans la boucle : les validations que demande `to-prd` (modules, tests) sont tranchées comme le ferait un agent `guideline` — hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables — et marquées (G).

### Défauts de la spec : quatre `static`, `g_ui_defaults` supprimée (R7 de 1b.14a sans fille, forme 1b.33) (G)

`ql`, `fa`, `le`, `hw` sont aujourd'hui recopiés d'une table d'offsets remplie à l'init. Lecture de 1b.14a sur une table sans fille (1b.16, 1b.25–1b.33) : **`g_ui_defaults`, l'enum `UI_*`, la boucle de recopie du constructeur et la boucle de déballage de l'init sont supprimés** ; quatre `static` — `pico8_fixed_t g_ui_ql`, `g_ui_fa` ; `pico8_list_t *g_ui_le` ; `pico8_qz_box_t *g_ui_hw` — déballés **une fois, à l'init, après l'échange de `le`**, par `pico8_ui_default(name, &v)` puis une aide de spec par genre (1b.23a1 : quatre défauts, plus de trois) :

| aide | défauts | `assert` | message (préfixe) |
|---|---|---|---|
| `ui_default_fixed(found, v)` | `ql`, `fa` | trouvé, `FIXED` | `"ui.ql/fa: a default of the struct is a key of ui's spec and a fixed (fixe)"` |
| `ui_default_list(found, v)` | `le` | trouvé, REF non NULL, tag `LIST` | `"ui.le: a default of the struct is a key of ui's spec and the list that replaced o() (ref:liste)"` |
| `ui_default_box(found, v)` | `hw` | trouvé, REF non NULL, tag `UNKNOWN` | `"ui.hw: a default of the struct is a key of ui's spec and a box b() (ref:autre)"` |

Forme exacte de `help_default_*` (1b.33). L'`assert` groupé de l'init (« lj/cj read le as a list … ») tombe : ses deux gardes passent dans `ui_default_list`/`ui_default_box`. `pico8_ui_defaults_reset` remet les quatre `static` à 0/NULL. **`pico8_ui_hw()` rend `g_ui_hw`** (signature et contrat inchangés, 1b.16 : état de classe lu par le cart). R3 pose `self->le = g_ui_le` (même REF que la classe, partagée jusqu'au premier `cj`/`pz`) et `self->hw = g_ui_hw`.

### `o_`, `gj` : `pico8_struct_header_t *`, sans aide ni prédicat — lecture de 1b.35 (G)

Écrits seulement par des méthodes de `ui` : `gw` (`self->o_ = NULL`, puis `self->o_ = e` pour `e` pris dans `cv.hd`, déjà un `pico8_struct_header_t *` dont le genre est gardé à l'entrée par `pico8_lt`, qui lit chaque entité par `pico8_entity_get_field` et son `assert` de tag `STRUCT`) et `lj` (`self->gj = self->o_`, copie d'un pointeur typé). Lecture de 1b.35 (source typée) — et, au besoin, de 1b.36 (source déjà gardée) — **sans écart** : ni aide, ni prédicat, ni `assert` ; l'ensemble réel (les 14 classes de la ligne, `ga` jamais survolé) n'est gardé que par le type. Jamais `false` (cart : `self.ic,self.o_={}` pose nil ; `gj=o_`) : pas d'indicateur. Commentaire `/* borrowed, never freed */` (forme `g_.et`).

### `lheld` : `pico8_struct_header_t *` + `ui_take_held` + `ui_is_held_class` ; `rheld` : `pico8_bg_t *` + `ui_take_bg` (1b.14a, 1b.22, R1) (G)

`bh` range dans `lheld`/`rheld` le `kn` que rend `jz(bt.."down", …)`, c'est-à-dire **la valeur d'un `ldown`/`rdown` obtenu par nom (`pico8_mv`) sur une autre entité**, une `pico8_value_t` dont `bh` ne vérifie que le genre REF et la vérité — pas le tag, pas la classe (c'est exactement TROU-6). **Ni la lecture de 1b.35 (source typée) ni celle de 1b.36 (genre déjà gardé à la source) ne s'appliquent** ; c'est le cas de 1b.14a (valeur lue par nom sur une autre entité puis rangée, précédent `jc.mt`/`jc_take_fixed`) : **aide par valeur**, et, pour une REF à plusieurs cibles, l'ensemble porté par son `assert` (1b.22). `rheld` a une seule cible (`ref:pico8_bg_t`) : **R1 sans écart, `pico8_bg_t *`**, `assert` sur tag et descripteur par l'accesseur public (1b.13 point 4).

- **Ensemble de `lheld` = la ligne, exactement** : `jc` (`jo:ldown`, `nr:ldown`), `g_`, `mz` (`bg:ldown`), `qh`, `os`, `nr`, `fr`, `nl`, `mx`, `hj` (`ie:ldown`, `gx`). Aucune classe productrice hors ligne : rien à élargir. **`pm` exclue** (jamais dans `gx`, H5), `ga`, `jo`, `bg`, `fv`, `js`, `fq`, `ie` exclus (aucun `ldown` ne les tend).
- **Prédicat `ui_is_held_class(cls)`** : `cls == pico8_jc_struct_class() || cls == pico8_g__struct_class() || cls == pico8_mz_struct_class() || (cls->methods[PICO8_METHOD_LY] != NULL && cls != pico8_pm_struct_class())`. Propriété de descripteur pour les sept de `gx` parce que `DATA_ENGINE_SRCS` lie `ui.c` sans `planet_family_trade.c` (`nr`, `fr`, `mx`) ; `LY` = exactement l'arbre `qh` (1b.24), dont `pm` est retiré par son accesseur (`planet_family.c`, lié partout). `jc`, `g_`, `mz`, `pm` : accesseurs publics, `slipway.c`, `probe.c`, `wormhole.c`, `planet_family.c` étant dans `ENGINE_SRCS`, `DATA_ENGINE_SRCS` et `main_raylib` (grep du `Makefile`).
- **Ensemble de `rheld` = `bg`**, `cls == pico8_bg_struct_class()` (`bg.c` lié partout où `ui.c` l'est).

| aide | membre | état nil puis sortie | accepté (`assert`) | message (préfixe) |
|---|---|---|---|---|
| `ui_take_held(value, pico8_struct_header_t **out)` | `lheld` | `*out = NULL` | REF, tag `STRUCT` **puis** `ui_is_held_class(cls)` | `"ui.lheld: a value neither nil nor what an ldown hands over (a jc; a g_ or an mz from bg; a qh, os, nr, fr, nl, mx or hj from an ie) is off its census row (ref: to those) -- …"` |
| `ui_take_bg(value, pico8_bg_t **out)` | `rheld` | `*out = NULL` | REF, tag `STRUCT` **puis** `cls == pico8_bg_struct_class()` | `"ui.rheld: a value neither nil nor the bg that rdown hands over is off its census row (ref:pico8_bg_t) -- …"` |

Chaque message finit par `"a field under no rule of plan-3 section 5: stop, do not widen the member"`, la phrase `is off its census row` sur une ligne du source. Forme 1b.13/1b.15 complète (état nil d'abord, sortie sur `REF NULL` genre **et** pointeur) même si `bh` ne leur passe jamais qu'une REF vraie : la branche nil et la distinction d'union nulle sont inatteignables par `bh` (trous `gcov` et mutant équivalent justifiés, voir Tests).

**R6 dans `bh`** : plus de `pico8_value_t *held` ; `if (jn && pico8_ui_jz(self, down, 2, pico8_value_ref(self->qa), je_ra_val, &kv, &kn) && kn.kind == PICO8_VALUE_REF) { /* TROU-6 */ l ? ui_take_held(kn, &self->lheld) : ui_take_bg(kn, &self->rheld); }` ; `held_inst = l ? self->lheld : (self->rheld != NULL ? &self->rheld->ke.header : NULL)` ; `hq`, `jz("gv", 1, pico8_value_ref(held_inst), …)`, `ey` inchangés ; relâchement : `self->lheld = NULL` ou `self->rheld = NULL`. **Le marqueur `/* TROU-6 */` reste**, déplacé sur la ligne où `kn` devient un pointeur typé ; **l'entrée TROU-6 de `docs/trous-de-portage.md` n'est pas éditée** : l'`assert` de l'aide est l'arrêt « champ sans règle » que R5 prescrit, il ne change aucun comportement atteignable (H20) et le côté Lua de l'écart (« n'importe quelle table ») demeure ; le commit le dit.

**Écartés** : un pointeur d'en-tête sans aide pour `lheld` (la source n'est pas gardée, cf. ci-dessus) ; `pico8_struct_header_t *` pour `rheld` (R1 donne `pico8_bg_t *` pour une cible unique) ; un prédicat partagé avec un autre fichier (1b.13 point 2) ; un `assert` d'ensemble sur `o_`/`gj` (source typée, lecture de 1b.35).

### `qa`, `ic`, `le` : pointeurs sans indicateur (G)

- **`qa`** → `pico8_vec_t *` : `lj` : `self->qa = heap_qa;` (TROU-7 gardé) ; `gw`, `bh` : `qa_val = pico8_value_ref(self->qa)` (même REF passée à `hd`/`ldown`/`hq` ; `jc_mouse_point.ra` et `mz.ra` l'empruntent, 1b.17, 1b.36).
- **`ic`** → `pico8_ob_table_t *` : `gw` : `self->ic = ic;` ; `jz` : `if (self->ic == NULL) return 0; ic = self->ic;` (tag et cast retirés : la garde est à l'écriture, 1b.20). Le seau de `ic` garde son `PICO8_REF_AS_LIST_CONST` (valeur d'une table ob, phase D).
- **`le`** → `pico8_list_t *` : `lj` : `le_ptr_before = self->le; le_count = self->le != NULL ? self->le->count : 0;`, `pico8_gu(le_ptr_before, pico8_value_ref(self->o_))`, identité `self->le == le_ptr_before` ; `cj` : `if (self->le != NULL) { discarded = pico8_nv(self->le, …); … } self->le = heap_le;` ; **`msgbox.c`** : `ok->le = le_heap;` et `ok->le = empty_heap;`. Trois `PICO8_REF_AS_LIST_CONST` disparaissent (`:307`, `:320`, `:547`) ; `pico8_ref_audit.h` reste inclus pour celui de `jz` (H11).

### R1–R9 appliquées

| règle | sur `ui` |
|---|---|
| R1 | Tableau de recensement ; nom et place gardés ; aucun indicateur ; `/* borrowed, never freed */` sur `o_`, `gj`, `lheld`, `rheld`. **Inclusion ajoutée à `ui.h` : `pico8_bg.h`** (typedef de `pico8_bg_t`, anonyme, pas de déclaration anticipée ; `bg.h` n'inclut pas `ui.h`, pas de cycle, H12), rien d'autre. **Inclusions ajoutées à `ui.c`** : `pico8_slipway.h`, `pico8_probe.h`, `pico8_wormhole.h`, `pico8_planet_family.h` (accesseurs de `jc`, `g_`, `mz`, `pm`), rien d'autre ; `<stddef.h>` retiré. |
| R2 | Aucun `has_` : les deux fixes sont nil `jamais`, les 8 autres sont des pointeurs jamais `false`. |
| R3 | `pico8_ui_struct_new`, après `pico8_ke_struct_init`, dans l'ordre du typedef : `ql = g_ui_ql`, `fa = g_ui_fa`, `le = g_ui_le`, `hw = g_ui_hw`, `qa`, `ic`, `o_`, `gj`, `lheld`, `rheld` à NULL ; puis l'`assert` des clés (**texte inchangé**), la couverture, `pico8_ke_struct_pq`, `PICO8_MEMBER_CENSUS_SEE`. |
| R4 | `pico8_ui_field` : 10 branches `strcmp` dans l'ordre du typedef, puis `return pico8_ke_field(self, name, out_value);` ; petite aide `static` de lecture permise (forme `jc_ref_field`). `ql`, `fa` : toujours `FIXED` ; pointeurs : NULL → 0, sinon REF même pointeur (`rheld` : l'adresse du `bg`, en-tête premier membre). `*out_value` intact sur 0. |
| R5 | Écrivain vacant (`set_field` NULL, gardé) ; boucle de clés vacante (`assert` gardé) ; aides par valeur de 1b.14a dans `bh` (`ui_take_held`, `ui_take_bg`). |
| R6 | `lj`, `ev` (`self->rheld != NULL` ; `o_` puis `ql` de `o_` par nom, repli `pico8_ui_to_int(self->ql)`), `gw`, `bh`, `jz`, `cj` ci-dessus ; `msgbox.c` (deux lignes) ; tests : `test_pico8_jc.c:444`, `test_pico8_ui.c:742-745`. |
| R7 | Forme `offsetof` de 1b.9 : `members[]` supprimée avec sa boucle ; R7 de 1b.14a : `g_ui_defaults` supprimée avec ses deux boucles ; **plus aucun `offsetof` dans `src/`**. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_ui_t)` et celui du `je` provisoire gardés ; `tools/member_census.txt` non édité. |
| R9 | 10 → **0**. |

### Interaction avec 1b.35 (`jc`), 1b.36 (`mz`), 1b.32 (`g_`), 1b.15 (`bg`)

- **`jc:hq`** lit `ok.o_` **par nom** (`slipway.c:220`) : inchangé, passe désormais par la branche `o_` du lecteur ; `test_pico8_jc` pose `o_` directement (`:444`, réécrite).
- **`bh` → `hq`/`ey` du tenu** : l'argument `mp` reste `pico8_value_ref(self->qa)`, même REF : `jc_mouse_point.ra` (1b.12) et `mz.ra` (1b.36) l'aliasent comme avant.
- **Les producteurs** : `pico8_jc_struct_class()`, `pico8_g__struct_class()`, `pico8_mz_struct_class()` (1b.36 ne change ni l'accesseur ni l'adresse du descripteur, H3), `pico8_bg_struct_class()` et les descripteurs à `LY` ne changent pas.
- Aucune autre struct n'est touchée : les lectures par nom de `o_`/`lheld`/`le`… par les autres TU passent par le lecteur.

### Tests touchés (G)

- **`tests/test_pico8_ui.c`** : (a) **lignes `:742`, `:743`, `:745`** : `ok->hw.as.ref` → `ok->hw`, `ok->le.as.ref` → `ok->le`, `other->le.as.ref` → `other->le`, `other->hw.as.ref` → `other->hw` (comparés au `void *` de `class_ref`), rien d'autre sur ces lignes ; (b) **ajouts** : caractérisation, test d'arrêt, `UI_MEMBERS_HASH`, entité de test (descripteur `static` du TU), aides de test, deux appels `in_fresh_child` dans `main`. Aucune autre ligne existante, aucune valeur figée ne change.
- **`tests/test_pico8_jc.c:444`** : `pico8_ok()->o_ = pico8_value_ref(hovered);` → `pico8_ok()->o_ = hovered;`, rien d'autre.
- Aucun autre fichier de `tests/`.

### Revue des tests de régression, mécanique

**Liste** (reprise de 1b.30–1b.36) : les tests nommés au §6 (les 17 de régression) et dans les lignes 1a : `tests/test_pico8_<nom>.c` pour `built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`, `bg`, `entity_kind`, `hj`, `jc`, `jo`, `jo_spec_defaults`, `js`, `mo`, `mz`, `nl`, `os`, `pm`, `qh`, plus `tests/test_class_coverage.c`. Tous existent (vérifié par `ls`).

**Greps faits pour ce PRD** (`M = ql|fa|le|hw|qa|ic|o_|gj|lheld|rheld`) :

- accès direct : `grep -nE '(->|\.)(M)\b'` sur tous les `tests/*.c` → dans la liste, **`jc:444`** seul (hors liste : `test_pico8_ui:742-745`) ;
- par nom sur `ok` (`grep -nE '"(M)"|pico8_ok\(\)'`) : `slipway_drag` (`lheld`), `pause_menu_music` (`o_`, `le`), `built_planet_panel_nil_row`, `ga_export_panel_by`, `planet_gh_row_lq`, `rich_planet_exports`, `tech_screen` (`le`), `bg` (`ic`, `o_`, `lheld`/`rheld` par `ok_ref`, `jz` direct), `jc` (`lheld`), `js` (`le`, `o_`), `mo` (`qa`) ; `probe_mm_rnd_order` nomme `pico8_ok()` sans ces noms ; ailleurs rien ;
- chemin sans lecture : `playability` (étape 2 : `bg:ldown` → `mz` rangé dans `lheld` ; étape 4 : `jo:ldown` → `jc`), `geoharvest_burst` (étape 1 : un `mz` lancé par pression tenue).

| # | test | accès / chemin | diff ? | preuve rouge |
|---|---|---|---|---|
| R-a | `slipway_drag` | `lheld` par nom (le `jc` rangé par `ui_take_held`), `o_` lu par `jc:hq`, `bh` → `ey` : **sur le chemin** | non | **Obligatoire.** Bug d'origine **2d7021f** remis (nil à la place de l'entité de `jz("gv")` dans `pico8_ui_bh`, recette de 1b.32 R-d / 1b.35 R-a) → rouge (`oe_count(origin) == 1`) ; restauré → vert. |
| R-b | `playability` | étapes 2 et 4 : `mz` puis `jc` rangés dans `lheld` par l'aide et son prédicat : **sur le chemin** | non | **Obligatoire.** Bug de référence de 1a.17 (constructeur de `jo` appelant `pico8_ke_struct_pq` deux fois, recette de 1b.35 R-b) → rouge (`count <= PICO8_PLAYABILITY_MAX_PLANETS`) ; restauré → vert. |
| R-c | `pause_menu_music` | `o_` et `le` par nom ; `cj` sur identité de `le` : **sur le chemin** | non | **Obligatoire.** Bug de référence de 1a.16 (dans `pico8_ix4_my_dispatch`, `pico8_pz` appelé avant les quatre installations de `my`, recette de 1b.34 R-b) → rouge ; restauré → vert. |
| R-d | `built_planet_panel_nil_row` | `le` par nom après `pz` (écriture de `msgbox.c` réécrite) : **sur le chemin** | non | **Obligatoire.** Recette corrigée de 1b.34 (clause `REF NULL` de `pico8_msgbox_truthy` retirée **et** lecteur de `jo.ep` qui rend nil présent) → rouge (`built_rows == BUILT_PANEL_EXPECTED_ROWS`) ; restauré → vert. |
| R-e | `jc` (TU de 1a) | **`:444` réécrite** (écriture directe de `o_`) ; `lheld` par nom | **oui**, la seule forme de l'écriture | **Obligatoire** (1b.23a2). La ligne réécrite mise sous `if (0)` → `test_hq` rouge (`re` attendu = la planète survolée) ; restaurée → vert. |
| R-f | `ui` (TU de 1a, hors liste) | **`:742-745` réécrites** (lectures directes de `le`, `hw`) | **oui**, 3 lignes | **Obligatoire.** Mutation 3a (R3 : `le` = une liste neuve) → `test_ui_is_a_struct` rouge à `:743` (et `test_built`) ; mutation 3b (`hw = NULL`) → rouge à `:742` ; restaurées → vert. |
| R-g | `tech_screen`, `ga_export_panel_by`, `planet_gh_row_lq`, `rich_planet_exports` | `le` par nom après `pz` : même chemin que R-d | non | **Dans le budget seulement**, recettes de 1b.34 (R-g `_cr_t` sous `if (0)` ; R-d 9183fee ; R-a 8361468 ; R-e 50f91b4) ; sinon « non lancé » au journal. Mutation 14 (`msgbox.c` n'écrit plus `ok->le`) jouée sur eux si le budget le permet. |
| R-h | `geoharvest_burst` | un `mz` rangé dans `lheld` | non | **Dans le budget seulement** (recette de 1b.29 : `pico8_ga_pq` sans `pico8_ez(self, 140)`). |
| — | `bg`, `js`, `mo` (TU de 1a) | lectures par nom de `ic`, `o_`, `lheld`, `rheld`, `le`, `qa`, `jz` direct | non | aucune de plus ; verts sans modification (mutations 8–11 les rougissent peut-être aussi, noté). |
| — | `empty_planet_cleared`, `cheat_code`, `game_end`, `score_panel_stars`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_kj_dispatch`, `probe_mm_rnd_order`, `entity_kind`, `hj`, `jo`, `jo_spec_defaults`, `mz`, `nl`, `os`, `pm`, `qh`, `test_class_coverage` | homonymes seuls (`fa`…), ou `ok` traversé par le rejeu sans lecture ; tout rejeu passe par `lj` | non | aucune ; verts sans modification. |

Hors liste, sur le chemin et verts sans modification : `test_pico8_fv`, `test_pico8_fq`, `test_pico8_lq` (`ic`, `o_`, `le` par nom), `test_pico8_je`, `test_pico8_nf` (déclarent un `pico8_ui_t *`), `test_pico8_ie` (lent, `o_`, `lheld` par nom).

### Preuves (sortie attendue sur l'arbre final)

**Sortie de `ui` et de la phase** :

- `awk '/^typedef struct \{/{b=""} {b=b $0 "\n"} /\} pico8_ui_t;/{printf "%s", b}' src/pico8_ui.h | grep -c pico8_value_t` : **0** ;
- `make value-members` : **une seule ligne, `membres pico8_value_t : 0`**, aucune ligne `fichier:ligne:` avant elle (« vide ») ;
- `grep -rn 'offsetof' src/` : **vide** (9 lignes avant, toutes dans `ui.c`) ; `grep -n 'stddef' src/pico8_ui.c` : **vide** ;
- `grep -rn 'pico8_value_t \*)((' src/` : la seule ligne de `pico8_member_census.c` (lecteur de la table générée, H9) ; aucune dans `ui.c` (2 avant).

**Lecteur, aides, R6** :

- `grep -nE '(->|\.)(M)\.(kind|as)' src/pico8_ui.c src/pico8_msgbox.c` : **vide** (11 lignes avant dans `ui.c`) ; `grep -n 'ok->le = pico8_value_ref' src/pico8_msgbox.c` : **vide** (2 avant) ;
- `grep -n 'pico8_value_t \*' src/pico8_ui.c | grep -v 'pico8_value_t \*out_value)' | grep -v 'pico8_value_t \*out_kn)'` : **vide** (4 lignes avant : `member`, les deux casts, `held`) ;
- `grep -c 'off its census row' src/pico8_ui.c` : **2** (0 avant) ; `grep -n 'ui_take_\|ui_is_\|ui_default_' src/pico8_ui.c` : 2 aides de rangement, 1 prédicat, 3 aides de spec, chacune appelée ;
- `grep -rnw 'g_ui_defaults\|UI_QL\|UI_HW' src/` : **vide** ; `grep -c 'PICO8_REF_AS_LIST' src/pico8_ui.c` : **1** (4 avant) ;
- `grep -n 'TROU-6' src/pico8_ui.c` : une ligne, dans `pico8_ui_bh`, sur le rangement de `kn` ;
- `git diff -U0 src/pico8_ui.h | grep '^+#include'` : `pico8_bg.h` seul ; `git diff -U0 src/pico8_ui.c | grep '^+#include'` : `pico8_planet_family.h`, `pico8_probe.h`, `pico8_slipway.h`, `pico8_wormhole.h`, rien d'autre.

**Périmètre du diff** :

- `git diff --stat HEAD -- src/ ':!src/pico8_ui.h' ':!src/pico8_ui.c' ':!src/pico8_msgbox.c' tools/ Makefile` : **vide** ;
- `git diff -U0 src/pico8_msgbox.c | grep -c '^-[^-]'` : **2** ;
- dans `tests/`, seuls `test_pico8_ui.c` (ajouts, plus `:742`, `:743`, `:745`) et `test_pico8_jc.c` (`:444`) changent : `git diff -U0 tests/test_pico8_ui.c | grep -c '^-[^-]'` : **3** ; `git diff -U0 tests/test_pico8_jc.c | grep -c '^-[^-]'` : **1**.

### Sortie de la phase 1b

La ligne de l'étape fixe trois critères ; le §5 (puce **Sortie**) et le §7 (« toujours à la sortie de phase ») les confirment. Ils sont constatés **sur l'arbre final du commit de 1b.37**, dans le tableau « Runs » :

1. `make value-members` : vide au sens ci-dessus (compteur à 0, aucune ligne de membre) ;
2. `make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` (sortie 0 du script) ;
3. `make sanitize-net` : `sanitize: 15 propre(s)` ;
4. plus, par l'invariant §3 : `make frames-check` (dans `make e2e`) 15 « conforme ».

**Le plan reçoit, sous la liste 1b, un paragraphe « Sortie de la phase 1b, constatée à 1b.37 »** (forme exacte du paragraphe de 1a.42e) : les quatre constats ; `grep -rn offsetof src/` vide ; point d'entrée de la phase D : toute entité est une struct à membres C typés, `pico8_value_t` ne vit plus que dans les tables ob, les arguments et retours de dispatch et les lecteurs/écrivains par nom du descripteur (Coexistence, qui survit à 1b).

**Infrastructure que la sortie rend sans objet — listée, pas supprimée** (aucune ligne du plan ne programme sa suppression ; la supprimer serait hors périmètre, et R8 garde les crochets) :

- **Recensement** : `src/pico8_member_census.[ch]`, 42 `PICO8_MEMBER_CENSUS_SEE` dans `src/`, 6 `PICO8_MEMBER_CENSUS_STOP` dans `tests/`, les crochets `READ`/`CHILD_EXIT` (26 lignes), `tools/member_census.sh`, `make member-census`, `CENSUS_UNIT_TESTS`, le mode `--c-table` de `tools/entity_value_members.sh` (qui émet une ligne factice à 0 membre, H9) ; **`tools/member_census.txt` garde des lignes périmées** (10 `pico8_ui_t` et celles des étapes précédentes, jamais régénérées) : non édité (R8), sa régénération est hors suite.
- **`make value-members`** reste : le §5 en fait la commande de sortie ; elle devient une garde, sans suppression prévue.
- **`pico8_ref_audit.h`, `make ref-audit`, les `PICO8_REF_AS_LIST*` restants** (dont celui de `jz`) : leur suppression est la **Sortie de la phase D**, pas celle-ci.
- **`pico8_jb_struct_class()`** : **pas morte**, contrairement à ce que laisserait croire la note de 1b.34 (« sans client » vaut pour `ui_button.c` seul) — `research_icon.c:428` (`ie_take_jb`) et `test_pico8_js` l'appellent. Rien à retirer.
- Les emballages publics (`pico8_g__et`, `pico8_mz_et`, `pico8_js_h_`…) et les lecteurs/écrivains par nom : clients vivants (Coexistence, « les autres survivent à 1b »).
- `tools/class_coverage*` : le §7 ne le demande plus après 1b ; gardé, rien n'est programmé.
- La phrase « castée sans vérification » de TROU-6 ne vaut plus que côté Lua une fois `ui_take_held`/`ui_take_bg` en place ; l'entrée n'est pas éditée ici (Out of Scope), le commit le signale.

### Empreinte et découpage

Ligne : 25 (6, 19). **Remesurée pour ce PRD** avec la formule du plan, sur l'arbre `46992b9` (plus 1b.36 en cours) : **25 (6, 19)**, identique — `src/` : `entity.c`, `entity_geom.c`, `msgbox.c`, `particle.c`, `slipway.c`, `ui.c` ; `tests/` : `bg`, `built_planet_panel_nil_row`, `fq`, `fr`, `fv`, `g_`, `ga_export_panel_by`, `jc`, `je`, `mh`, `mo`, `nf`, `pause_menu_music`, `pj`, `planet_gh_row_lq`, `rich_planet_exports`, `slipway_drag`, `tech_screen`, `ui`. Majorant : trois fichiers accèdent directement (`ui.c`, `msgbox.c`, `test_pico8_ui.c`), `slipway.c` n'y entre que par les homonymes `jc->fa` ; la formule manque `test_pico8_jc:444` comme accès direct (le fichier ne nomme pas `pico8_ui_t`, il y entre par nom) et les lecteurs par `lookup`/`get_ref` (`js`, `lq`, `ie`), sans effet sur ce qui change. **Fichiers modifiés attendus : 5** — `src/pico8_ui.h`, `src/pico8_ui.c`, `src/pico8_msgbox.c`, `tests/test_pico8_ui.c`, `tests/test_pico8_jc.c` — plus `docs/plan-3.md`. Empreinte réelle comptée au commit (H22).

**Découpage (G) : aucun, un seul commit** (la sortie de phase comprise). Évalué sur trois découpes :

- (a) **1b.37a** « défauts » (`ql`, `fa`, `le`, `hw`, `g_ui_defaults` ; 10 → 6) puis **1b.37b** « état de frame » (`qa`, `ic`, `o_`, `gj`, `lheld`, `rheld` ; 6 → 0) ;
- (b) les deux membres à aide (`lheld`, `rheld`) d'abord, le reste ensuite ;
- (c) le typage en un commit, la note de sortie de phase dans un commit de documentation à part.

Écartées parce que : (1) (a) et (b) touchent les **mêmes** fonctions — la `members[]` du lecteur (qui ne tomberait qu'au second commit, forme mixte qu'aucun commit final ne garde), `pico8_ui_struct_new`, `pico8_ui_lj` (`le`, `qa`, `o_`, `gj`) et `pico8_ui_bh` (`qa`, `lheld`, `rheld`) ; (2) **le coût de test double** : chaque commit change le constructeur, donc chacun passe `make e2e`, `make sanitize-net` et `make class-coverage-net` (≈ 12–15 min à eux trois, 1b.35 : e2e + class-coverage ≈ 9 min 42 s, sanitize ≈ 5 min), et R-a à R-d seraient faits deux fois ; (3) (c) séparerait le constat de l'arbre qui le produit : le précédent 1a.42e constate la sortie de 1a dans le commit qui la fait, et les trois cibles de la ligne sont de toute façon lancées pour le typage ; (4) le travail propre est peu coûteux : `test_pico8_ui` tourne en ~1–4 s, un mutant ne recompile que `ui.c` ou `msgbox.c`, 5 fichiers (1b.35 : 16 membres, 5 fichiers, un commit) ; (5) la règle de découpage du plan (> ~40 fichiers) ne s'applique pas à 25. Si le budget est dépassé, l'implémenteur suit l'ordre de priorité de « Vérification » et note ce qui n'a pas été lancé.

### Hypothèses (rien n'a été lancé)

- **H1** : 1b.36 commitée avant le départ ; `make value-members` rend 10 au départ (10 lignes `pico8_ui_t`, rien d'autre), 0 à la fin.
- **H2** : 1b.36 ne touche que `wormhole.[ch]`, `wormhole_display.c`, `test_pico8_mz.c` et le plan ; sa note « Découvert à 1b.36 » (lue dans l'arbre de travail à l'écriture, non commitée) porte la lecture étendue de 1b.35 pour `mz.ij` (« source `pico8_value_t` dont le genre est déjà gardé ») et aucune règle qui contredise ce PRD. Si la note commitée diffère, elle prime et l'écart est noté (pas un arrêt). Le choix des aides de `lheld`/`rheld` repose sur 1b.14a et R1, que 1b.36 ne touche pas ; celui de `o_`/`gj` tient sous la lecture stricte (1b.35) comme sous l'étendue (1b.36).
- **H3** : `pico8_mz_struct_class()` et le descripteur de `mz` inchangés par 1b.36 ; `ui.c` inclut `pico8_wormhole.h` (qui n'inclut pas `ui.h`) sans cycle ni collision.
- **H4** : hors des fils de `must_abort`, aucun site (cart, C, TU, 15 rejeux) ne fait ranger par `bh` dans `lheld` autre chose qu'un `jc`, un `g_`, un `mz` ou une planète de `gx` (`qh`, `os`, `nr`, `fr`, `nl`, `mx`, `hj`), ni dans `rheld` autre chose que le `bg`. Démentie : arrêt « champ sans règle », sans élargir le membre.
- **H5** : `pm` n'arrive jamais dans `lheld` (`ie.ke` ∈ `gx`, `pm` né de `kv[hp.cw]`).
- **H6** : `o_`, `gj`, `lheld`, `rheld` ne tiennent jamais `false` ; `le`, `hw` jamais NULL après l'init ; `ql`, `fa` jamais nil.
- **H7** : l'**entité de test** atteint `bh` : un descripteur `static` du TU (`LDOWN`, `RDOWN` qui rendent une valeur globale ; lecteur qui rend `hd` = `FIXED` 1 et `fa` = `FIXED` 15 ; écrivain qui accepte `m_`), une instance inscrite par `pico8_lt` (elle entre dans `cv.hd`), puis `mouse_at(…, 1)` ou `(…, 2)` et `pico8_ui_lj(&mem, ok)` : `gw` la range en couche 15 (aucune entité de `cv.hd` n'y est : `fq` 14, `ie` 12, `js` 11, `fv` 10), `jz` la trouve la première. Sinon l'implémenteur lève `jn` par nom sur la table de bouton de `je` et appelle `pico8_ui_bh`, et le note.
- **H8** : sur l'ancien code, chaque cas **struct** du test d'arrêt se termine sans `SIGABRT` (`bh` range tout ; `hq` du tenu ne plante pas) ; les cas **non-struct** (table ob, liste, vecteur) avortent déjà (`pico8_mv` → `pico8_entity_get_field`, `assert` de tag) : ils sont retirés du « rouge d'abord », gardés (verts avant et après), et notés.
- **H9** : le build `-DPICO8_MEMBER_CENSUS` compile avec zéro membre (`--c-table` émet sa ligne factice) ; `-fsyntax-only -DPICO8_MEMBER_CENSUS` sur `ui.c`, `msgbox.c`, `test_pico8_ui.c`, `test_pico8_jc.c` suffit si le build complet n'est pas lancé.
- **H10** : `test_pico8_ui` reste sous 30 s avec deux fonctions de plus (~1–4 s mesuré). Démentie : on allège, on ne déplace pas le TU.
- **H11** : `-Werror` muet : `<stddef.h>`, les `size_t i`, `UI_*`, `g_ui_defaults` retirés ; `pico8_ref_audit.h` garde un usage (`jz`) ; `pico8_ui_truthy` (`gw`, `jz`) et `pico8_ui_field_raw` (`ev`, `gw`, `bh`) gardent des clients ; les 6 aides et le prédicat appelés.
- **H12** : `ui.h` + `pico8_bg.h` compile partout où `ui.h` est inclus (`probe.c`, `cart.c`, `bootstrap.c`, `msgbox.c`, `slipway.c`, les TU) ; `pico8_vec_t`, `pico8_qz_box_t`, `pico8_list_t`, `pico8_ob_table_t`, `pico8_struct_header_t` y sont déjà visibles (`geom.h` → `vec.h`, `ob.h`, `struct.h`) ; sinon l'inclusion manquante est ajoutée et notée.
- **H13** : tout binaire qui lie `ui.c` lie `bg.c`, `slipway.c`, `probe.c`, `wormhole.c`, `planet_family.c` (`ENGINE_SRCS`, `DATA_ENGINE_SRCS`, `main_raylib` : vérifié par grep ; aucune autre cible ne nomme `ui.c`).
- **H14** : aucune valeur figée ne bouge : les 22 hash de `test_pico8_ui` (`BUILT_HASH` … `MX_MEMBERS_HASH`), ceux de `test_pico8_jc`, `bg`, `js`, `fv`, `fq`, `lq`, `mo`, `ie`, et les 15 empreintes du filet. `le` et `hw` gardent l'adresse de la classe, `qa` et `ic` sont alloués au même moment.
- **H15** : `make sanitize-net` propre (aucune allocation nouvelle ; `qa`, `ic`, `le` alloués comme avant, TROU-7 inchangé).
- **H16** : `make class-coverage-net` identique (le compte de `ui` ne change pas).
- **H17** : les preuves §6 rougissent (R-a à R-f) ; R-g, R-h dans le budget.
- **H18** : `pico8_ob_table_free(g_ui_spec)` au `shutdown` ne rend pas `g_ui_le`/`g_ui_hw` pendants avant leur remise à NULL par `pico8_ui_defaults_reset`, appelé en premier (ordre actuel gardé).
- **H19** : revue des TU lents confirmée (grep ci-dessus) : aucune lecture nil par nom d'un non-pointeur de `ui`.
- **H20** : les `assert` de `ui_take_held`/`ui_take_bg` ne changent aucun comportement atteignable (H4) ; TROU-6 reste préservé côté comportement.
- **H21** : le crochet `PICO8_MEMBER_CENSUS_SEE` gardé, une table régénérée ne contiendrait plus aucune ligne (non lancé, R8).
- **H22** : empreinte réelle comptée au commit.
- **H23** : `float-audit`, `constants-check`, `float-grep`, `float-guard` inchangés : `ui.c` n'a aucun site dans `tools/float_audit_c1.txt`, le seul site de `msgbox.c` (`pico8_iv_kj`, `at2`) n'est pas touché.
- **H24** : `pico8_ui_new`, `pico8_ui_hw`, `pico8_ui_default`, `pico8_ui_lj`, `_ev`, `_gw`, `_bh`, `_jz`, `_cj`, `pico8_ok`, `pico8_ui_set_ok` gardent leurs signatures ; `pico8_ui_struct_class()` et son descripteur (`EV` seul, `set_field` NULL) inchangés.
- **H25** : sur une partie `start_game(DRAG)`, deux clics gauches sur le fond vide (relâchés entre eux) rangent un `g_` puis un `mz` dans `lheld` (`bg:ldown`, `#cg.g_ > 0` au second) ; sinon la caractérisation les prend par l'entité de test, et c'est noté.
- **H26** : `make value-members` écrit exactement `membres pico8_value_t : 0` sur la dernière ligne et rien d'autre.

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Les nouveaux tests n'accèdent à aucun des 10 membres directement : ils compilent sur les deux formes.
- **Commentaires des nouveaux tests et du code** : seulement les quatre sortes de `CLAUDE.md` — le sens d'un nom minifié, un écart connu avec le cart (`/* TROU-N */` nu), un piège qu'on réintroduirait (`assert` à message, sinon 40 mots), un contrat C que le nom ne dit pas (`/* borrowed, never freed */`). Pas de numéro d'étape, pas de récit ; le reste va dans le message de commit.
- **(G) Où : `test_pico8_ui`**, TU de la classe, rapide, dans `CENSUS_UNIT_TESTS`, lié par `ENGINE_SRCS` ; il a `start_game`, `start_input`, `play_frame`, `mouse_at`, `the_bg`, `lookup`, `has`, `get_ref`, `get_fixed`, `class_ref`, `bool_state`, `vec_of`, `rank_in`, `listed`, `is_of`, `must_abort`, `in_fresh_child`, `zero_union`, `g_off_row_*`, `cursor_is`, et inclut déjà les en-têtes de toutes les classes productrices. Précédents de lieu : `test_ui_is_a_struct`, `test_left`, `test_right`.
- **Caractérisation dirigée `test_ui_members_by_name`**, par `in_fresh_child` après `test_ui_is_a_struct`, hash **`UI_MEMBERS_HASH`**. Lecture par le **lecteur brut** (`pico8_ui_struct_class()->field`, témoin `pico8_value_fixed(0x1234)`) **et** par `pico8_entity_get_field`, qui doivent concorder (retour, genre, valeur, pointeur) ; adresses jugées par `assert`, jamais pliées (rang dans `hv` ou dans une liste locale).
  1. **Neufs** : l'`ok` de `start_game(DRAG)` et un `pico8_ui_new(NULL)`. Les 10 noms pliés : `ql` `FIXED` 58, `fa` `FIXED` 15 ; `le` = `class_ref("le")` (même pointeur, liste vide), `hw` = `class_ref("hw")` = `pico8_ui_hw()` ; `qa`, `ic`, `o_`, `gj`, `lheld`, `rheld` absents.
  2. **Une frame** (`mouse_at` puis `pico8_ui_lj`) : `qa` présent, vecteur = `je.ra` + `mo.ra`, pointeur différent d'une frame à l'autre ; `ic` présent (table) ; `o_` = le `bg`, `gj` = `o_` (même pointeur) ; `le` inchangé.
  3. **`lheld` par producteur** : le `jc` du drag (chemin de `test_left`, qu'on ne rejoue que si le coût reste sous H10 ; sinon repris de `test_left`) ; un `g_` puis un `mz` par deux clics sur le fond (H25) ; puis, par l'**entité de test** (H7), un `qh`, `os`, `nl`, `hj` (`pico8_X_new(NULL)`), un `nr`, `fr`, `mx` neufs, un `g_`, un `mz`, un `jc` bâti depuis une planète `ly` : chaque fois `lheld` = cette instance (même pointeur) ; relâché (`je.l.hm`) → absent.
  4. **`rheld`** : le `bg` par un clic droit tenu (même pointeur), puis par l'entité de test ; relâché → absent ; curseur 9 tant que tenu.
  5. **`le` par `pz`** : `call_row_my(pico8_ix(), 4)` → `le` présent, ni celui de la classe ni l'ancien, compte > 0 ; `pico8_fv_new(NULL)` (`ff`) → liste vide présente, autre pointeur ; `cj` → liste neuve.
  6. **Écriture par nom** : `must_abort` sur `pico8_entity_set_field(ok, <nom>, nil)` pour les 10 noms (l'écrivain NULL avorte déjà : fige qu'aucune écriture par nom n'apparaît).
  7. **Intrus** sur le lecteur : `""`, `"q"`, `"le "`, `"LE"`, `"held"`, `"l"`, `"r"`, `"has_le"`, `"header"`, `"ke"`, `"o"` → 0, témoin intact.
  8. Tout plié dans **`UI_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ (valeur imprimée par `check`), figé ensuite.
- **Test d'arrêt `test_ui_held_stops_off_row`**, rouge d'abord, par `in_fresh_child` après la caractérisation ; `must_abort` existant ; l'entité de test tend dans un fils la valeur hors ensemble placée dans une globale :

  | bouton | membre | REF tendue par `ldown` / `rdown` |
  |---|---|---|
  | gauche | `lheld` | **un `pm`** (a `LY`, hors ligne), **un `ga`**, **un `jo`** (de `cg.jo`), **le `bg`**, un `fv`, un `js`, un `ie`, le `jj` (`pico8_mz_jj_wrapper()`) ; table ob, liste (`pico8_hv()`), vecteur |
  | droit | `rheld` | **un `jc`**, **un `g_`**, **un `mz`**, un `qh`, un `jo`, un `fv` ; table ob, liste, vecteur |

  **Exclus, inatteignables par `bh`** : tout non-REF (`bh` ne range que `kind == REF`), nil et REF à union nulle (`jz` ne rend qu'une réponse vraie) — noté, pas de cas. Rouge sur l'ancien code pour les cas struct (H8), vert après : une ligne « Runs » pour chacun. Les cas non-struct avortent déjà sur l'ancien code (H8). Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_ui && ./tmp/test_pico8_ui` sauf mention, notées dans le commit avec leur test rouge ou la justification du trou ; un appel unique d'aide `static` se retire sous `if (0)` (1b.14b) :
  1. **(§7-1) indicateur forcé vrai — sans objet dans sa lettre** (aucun `has_`) ; forme la plus proche (précédent 1b.36) : la branche `o_` du lecteur rend 1 avec `REF NULL` quand `o_` est NULL → rouge : `test_built` (`absent[]`), caractérisation 1 ; **(1b)** idem pour `lheld` → rouge : `test_built`, `test_right` (`!has(ok, "lheld")`).
  2. **(§7-2) « présent 0 » là où c'était absent — sans objet dans sa lettre** (`ql`, `fa` jamais absents) ; forme la plus proche : la branche `rheld` rend `pico8_value_bool(0)` quand NULL → rouge : `test_built`, `test_right`.
  3. **(§7-3) init qui saute un membre typé** — la boucle de recopie de `g_ui_defaults` disparaît ; forme « pose une autre valeur » (précédents 1b.24, 1b.32–1b.36) : (a) R3 `le` = une liste neuve → rouge : `test_built`, `test_shared`, `test_ui_is_a_struct:743` (preuve R-f) ; (b) `hw = NULL` → rouge : `test_built`, `test_ui_is_a_struct:742` (R-f) ; (c) `ql = g_ui_fa` → rouge : `test_built` (58), `test_fresh` (`cursor_is(58)`) ; (d) `fa = g_ui_ql` → rouge : `test_built` ; (e) R3 `o_` posé à `self` → rouge : `test_built`.
  4. *`ui_take_held` muet* (`if (0)`) → rouge : `test_left` (`held != NULL`), caractérisation 3 ; `test_pico8_slipway_drag` noté.
  5. *`ui_take_bg` muet* → rouge : `test_right` (`get_ref(ok, "rheld") == bg`).
  6. *Prédicat* : (a) sans `jc` → rouge : `test_left` (fils qui avorte), `test_pico8_jc` noté ; (b) sans `g_` puis (c) sans `mz` → rouge : caractérisation 3 ; `test_pico8_playability` (R-b, étape 2) noté ; (d) sans la clause `LY` → rouge : caractérisation 3 ; `test_pico8_ie` (lent) noté ; (e) clause `LY` sans l'exclusion de `pm` → rouge : test d'arrêt ; (f) élargi à `methods[LDOWN] != NULL` (admet `jo`, `ga`, `bg`, `js`, `ie`) → rouge : test d'arrêt.
  7. *`ui_take_bg` sans le test de `cls`* (tag `STRUCT` seul) → rouge : test d'arrêt.
  8. *R6 `lj` : `self->gj = self->o_` retiré* → rouge : `test_fresh`, `test_hover`, `test_stale`.
  9. *R6 `gw` : `self->o_ = NULL` retiré* → rouge : `test_hover` (`o_ != planet`).
  10. *R6 `lj` : identité de `le` forcée vraie* (`cj` même quand `pz` a remplacé `le`) → rouge : `test_pico8_pause_menu_music` (R-c) ; **(10b)** forcée fausse → rouge : `test_stale` (`get_ref(old, "le") != le`).
  11. *R6 `ev` : `rheld` ignoré* → rouge : `test_right` (`cursor_is(9)`).
  12. *R6 `bh` : `self->lheld = NULL` du relâchement retiré* → rouge : `test_left` (`!has(ok, "lheld")`).
  13. *R6 `jz` : garde `self->ic == NULL` qui rend 0 toujours* → rouge : `test_left`, `test_right`, `test_layers`.
  14. *`msgbox.c` : `ok->le = le_heap` sous `if (0)`* → rouge : `test_shared`, `test_cj` (`count > 0`) ; R-g dans le budget.
  15. *R4 : la branche `hw` rend `g_ui_le`* → rouge : `test_built`, caractérisation 1.
  - **Mutants équivalents, notés** : dans `ui_take_held`/`ui_take_bg`, nil jugé sur le pointeur seul (`bh` ne passe qu'une REF vraie) ; `*out = NULL` initial omis (écrasé aussitôt) ; retrait du test de tag `STRUCT` avant `cls` (lecture de `->cls` d'une non-struct : comportement indéfini, pas un mutant discriminant) ; `le_count` calculé sans la garde `self->le != NULL` (`le` jamais NULL, H6).
- **`gcov`** : `test_pico8_ui` en `--coverage -O0`, piège `__gcov_dump` des fils (1a.23), sur `ui.c` et `msgbox.c` (fonction `pico8_pz`). Lignes attendues exécutées : les 3 aides de spec (hors échec d'`assert`), la remise à nil, les poses R3, les 10 branches R4 (NULL et non NULL de chaque pointeur), les 2 aides de rangement jusqu'à la recopie, le prédicat (toutes ses clauses : un `jc`, un `g_`, un `mz`, un `LY` ; le `pm` par le test d'arrêt), les lectures R6 de `lj`/`ev`/`gw`/`bh`/`jz`/`cj`, les deux écritures de `pz`. **Trous attendus** : la branche nil des aides de rangement (inatteignable, voir ci-dessus) ; les échecs d'`assert` (fils de `must_abort` ; ceux des aides de spec, inatteignables sur une spec fixe) ; les gardes `self == NULL` existantes ; la branche `ok == NULL` de `pz`. Rapport dans le commit.
- **Prior art** : `test_ui_is_a_struct`, `test_left`, `test_right` de `test_pico8_ui` ; `test_jc_members_by_name`/`test_jc_writer_stops_off_row` (1b.35) ; `test_mz_members_by_name` (1b.36) ; `test_fv_*` (1b.33, `help_default_*`) ; `jc_take_fixed` (1b.35, aide par valeur sur une méthode).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer.

- `./tmp/test_pico8_ui` : ~1–4 s (H10) ; environ 28 lancements (départ, arrêt rouge, final, 24 mutations, `gcov`) ≈ 6–8 min builds compris. Si un lancement dépasse 20 s, on joue d'abord 1, 3a, 3b, 4, 5, 6a, 6e, 6f, 7, 8, 12, et on note les autres non lancées.
- §6 R-a à R-d : deux lancements chacun (bug remis, restauré), un build de TU et quelques secondes de rejeu ; R-e (`./tmp/test_pico8_jc`, ~10 s) et R-f (mutations 3a, 3b) ; R-g, R-h dans le budget seulement.
- `make test` une fois ; `make e2e` une fois, **dans la même invocation** que `make class-coverage-net` ; `make sanitize-net` une fois, jamais en même temps que `e2e` ; `make value-members`.
- Ordre de priorité si le budget manque : cibles finales 1–6 ; départ, arrêt rouge, final ; R-a à R-f ; mutations 1, 3a, 3b, 4, 5, 6a, 6e, 6f, 7, 8, 12 ; le reste noté « non lancé ».
- Aucun TU lent n'est modifié ; `test_pico8_ie` exerce `lheld` par de vrais `ie:ldown` : `make e2e` le lance.

| # | Commande exacte | État de l'arbre (départ / final / mutation n / §6 R-x bug remis / §6 R-x restauré) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, vertes sur l'arbre final (les 2 à 5 sont les critères de sortie de la phase 1b) :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` vert, les **8** TU de `UNIT_TESTS_SLOW` verts ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **vide** — dernière et seule ligne `membres pico8_value_t : 0` ;
5. `timeout 15m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` — requis deux fois : `pico8_ui_struct_new` change, et c'est la sortie de phase (sans toucher `pico8_class_coverage_record` ni `KNOWN[]`) ;
6. `grep -rn offsetof src/` : vide.

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements (les fixtures de `test_pico8_ui` et des tests §6 sont seulement **lues**).

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet**

- `FAILED:` si 1b.36 n'est pas commitée au départ, ou si `make value-members` ne rend pas 10 (toutes `pico8_ui_t`) au départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, ou si le diff touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements.

**Types, tables, aides**

- `FAILED:` (« champ sans règle ») si une aide avorte dans un rejeu ou un TU hors des fils de `must_abort` (H4), ou si un type est élargi au lieu d'avorter : `pm`, `ga`, `jo`, `bg`, `fv`, `js`, `ie` admis dans `lheld` ; autre chose que le `bg` dans `rheld` ; un `has_` ajouté ; `pico8_value_t` gardé dans `pico8_ui_t`.
- `FAILED:` si l'un des 10 membres n'a pas le type retenu à sa place (`rheld` en `pico8_bg_t *`, `lheld`, `o_`, `gj` en `pico8_struct_header_t *`, `hw` en `pico8_qz_box_t *`…), ou si un indicateur est ajouté.
- `FAILED:` si `ui_is_held_class` n'admet pas exactement `jc`, `g_`, `mz` et les descripteurs à `LY` sauf `pm`, si `ui_take_bg` n'exige pas `pico8_bg_struct_class()`, si une aide ou le prédicat est rendu public ou partagé avec un autre fichier, ou si `o_`/`gj` reçoivent une aide, un prédicat ou un `assert`.
- `FAILED:` si une aide de rangement viole la forme de 1b.13/1b.15 (état nil d'abord ; genre **et** pointeur testés ; message qui nomme `ui.<m>` et le genre ; tag `STRUCT` vérifié avant `cls`), ou si une aide de spec n'a pas la forme de `help_default_*`.
- `FAILED:` si nil et la valeur de l'un des 10 noms ne se lisent plus par nom exactement comme en 1a (absent ; `FIXED` ; REF au même pointeur), en particulier si `le`/`hw` d'un `ui` neuf ne sont plus ceux de la classe ou si `pico8_ui_hw()` change de valeur.
- `FAILED:` si `g_ui_defaults`, la `members[]` du lecteur, un `offsetof` (dans tout `src/`), `<stddef.h>` dans `ui.c` ou une lecture/écriture `*(pico8_value_t *)` sur un membre de `ui` existe encore.
- `FAILED:` si `set_field` de `ui` n'est plus NULL, si l'`assert` des clés de construction change de texte, ou si une fonction publique de `ui.h` change de signature.
- `FAILED:` si `ui.h` gagne une autre inclusion que `pico8_bg.h` (hors H12 notée), ou `ui.c` une autre que `pico8_planet_family.h`, `pico8_probe.h`, `pico8_slipway.h`, `pico8_wormhole.h`.

**Diff et preuves**

- `FAILED:` si un site par nom change (`slipway.c:220`, `dc`, les lectures par nom sur `o_`/`je`/`mo`/`cv.hd` et les écritures `m_` de `ui.c`), ou si un site par nom est remplacé par un accès direct.
- `FAILED:` si un autre fichier de `src/` que `ui.h`, `ui.c`, `msgbox.c` change, si `msgbox.c` change ailleurs qu'aux deux écritures de `ok->le`, ou si `tools/`, le `Makefile`, `CENSUS_UNIT_TESTS`, `UNIT_TESTS_SLOW` changent.
- `FAILED:` si un autre test que `test_pico8_ui.c` et `test_pico8_jc.c` change ; si `test_pico8_ui.c` retire ou modifie une autre ligne existante que `:742`, `:743`, `:745` ; si `test_pico8_jc.c` change ailleurs qu'à `:444` ; si une valeur figée existante change (H14).
- `FAILED:` si un test de régression (§6) a un diff, ou s'il manque les deux lignes « Runs » (rouge bug remis, vert restauré) de R-a à R-f ; pour R-g, R-h, soit leurs lignes, soit « non lancé » au journal.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas l'attendu, ou si un crochet du recensement change, ou si `tools/member_census.txt` est édité.
- `FAILED:` si le marqueur `/* TROU-6 */` disparaît de `pico8_ui_bh`, si l'entrée TROU-6 ou une autre de `docs/trous-de-portage.md` est éditée, ou si un écart port/cart est corrigé au lieu d'être préservé (TROU-6, TROU-7, TROU-11 gardés).

**Sortie de phase**

- `FAILED:` si l'une des cibles finales 2 à 6 n'a pas sa ligne « Runs » verte sur l'arbre final : `make value-members` autre que la seule ligne `membres pico8_value_t : 0`, `class-coverage-net` différent de la référence, `sanitize-net` autre que `15 propre(s)`, `frames-check` autre que 15 « conforme ».
- `FAILED:` s'il manque au plan le paragraphe « Sortie de la phase 1b, constatée à 1b.37 », ou si ce commit supprime un élément de l'infrastructure listée plus haut (recensement, `value-members`, `ref-audit`, `class-coverage`, accesseurs) : aucune suppression n'est programmée par le plan.

**Tests de l'étape**

- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, lit ou écrit un des 10 membres directement, ne couvre pas les 10 noms neufs et après une frame, `lheld` pour chacune des 10 classes de la ligne, `rheld` = le `bg`, `le` après `pz`, `ff` et `cj`, les écritures par nom qui avortent et les intrus ; ou si `UI_MEMBERS_HASH` change après.
- `FAILED:` si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'arbre de départ (cas struct) et une verte sur l'arbre final, ou s'il n'a pas les cas `pm`, `ga`, `jo`, `bg` pour `lheld` et `jc`, `g_`, `mz` pour `rheld`.
- `FAILED:` si un commentaire ajouté dans `src/` ou `tests/` sort des quatre sortes de `CLAUDE.md`.
- `FAILED:` si le commit ne porte pas le rapport `gcov` et les mutations 1 à 15 (avec 1b, 3a–3e, 6a–6f, 10b), chacune avec son test ou la justification de son trou, plus les mutants équivalents.

**Commit et note**

- `FAILED:` si le travail n'est pas en un commit vert.
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, ou s'il manque le tableau « Runs » ou la note « Découvert à 1b.37 ».

## Out of Scope

- Supprimer l'infrastructure que la sortie rend sans objet (recensement et ses crochets, `tools/member_census.txt`, `make value-members`, `make ref-audit`, `pico8_ref_audit.h`, `tools/class_coverage*`) : aucune ligne du plan ne le programme ; `pico8_ref_audit.h` tombe à la sortie de la phase D.
- Régénérer `tools/member_census.txt` ; éditer `docs/trous-de-portage.md` (TROU-6 compris).
- Les homonymes lus par nom sur d'autres entités (`ql`, `fa`, `fg`, `ra`, `m_`…) ; `je` provisoire et `nf` (`je.l`/`je.r`, 1b.17) ; le seau de `ic` (valeur d'une table ob, phase D).
- Remplacer un site par nom par un accès direct (`slipway.c:220` compris) ; typer les arguments et retours de dispatch (`pico8_ui_jz`, `pico8_mv`).
- Corriger TROU-6, TROU-7 (`qa`, `ic`, `le` jamais libérés), TROU-11.
- Déplacer un TU entre listes ; `make member-census` ; `make ref-audit`.
- Fichiers d'événements, `FRAMES_EVENTS`, rendu, audio, appli raylib.
- La phase D elle-même.

## Further Notes

- **Décisions à la place de l'utilisateur (G)** : modules `ui.h` (typedef, `pico8_bg.h`), `ui.c` (4 `static` et 3 aides de spec à la place de `g_ui_defaults` ; lecteur à 10 branches ; `ui_take_held`, `ui_take_bg`, `ui_is_held_class` ; R6 ; 4 inclusions ; `<stddef.h>` retiré), `msgbox.c` (deux écritures) ; `ql`, `fa` → `pico8_fixed_t`, `le` → `pico8_list_t *`, `hw` → `pico8_qz_box_t *`, `qa` → `pico8_vec_t *`, `ic` → `pico8_ob_table_t *`, `o_`, `gj` → `pico8_struct_header_t *` sans aide, `lheld` → `pico8_struct_header_t *` à aide et prédicat, `rheld` → `pico8_bg_t *` à aide ; tests dans `test_pico8_ui` par ajouts, trois lignes de forme dans `test_pico8_ui`, une dans `test_pico8_jc` ; un seul commit, sortie de phase comprise ; §6 : R-a à R-f obligatoires, R-g, R-h dans le budget.
- **Note « Découvert à 1b.37 »** (à écrire sur la ligne ; le reste va dans le commit) :

  > patron 1b.1 appliqué ; `ql`, `fa` → `pico8_fixed_t`, `le` → `pico8_list_t *`, `hw` → `pico8_qz_box_t *` (défauts de spec en `static`, R7 de 1b.14a sans fille, `ui_default_fixed/list/box`, **`g_ui_defaults` supprimée**), `qa` → `pico8_vec_t *`, `ic` → `pico8_ob_table_t *`, aucun indicateur ; **`o_`, `gj` : lecture de 1b.35 sans écart** (écrits par `gw`/`lj` depuis `cv.hd`, source typée et gardée par `lt`), ni aide ni prédicat ; **`lheld`, `rheld` : la valeur d'un `ldown`/`rdown` obtenue par `mv` n'est pas une source gardée (TROU-6) — aide par valeur de 1b.14a** : `lheld` → `pico8_struct_header_t *`, ensemble = la ligne exactement, prédicat `jc` ∪ `g_` ∪ `mz` ∪ (`methods[LY]` sauf `pm`) ; `rheld` → `pico8_bg_t *` (R1, cible unique) ; marqueur TROU-6 gardé au rangement ; `msgbox.c` (`pz`) écrit `ok->le` typé ; `test_pico8_jc:444` et `test_pico8_ui:742-745` réécrits dans leur seule forme ; **`members[]` du lecteur supprimée, plus aucun `offsetof` dans `src/`** ; §6 : `slipway_drag`, `playability`, `pause_menu_music`, `built_planet_panel_nil_row` sur le chemin, prouvés rouges sans diff ; empreinte réelle mesurée au commit.

- **Paragraphe de sortie à ajouter au plan** (sous la liste 1b, forme de celui de 1a) :

  > **Sortie de la phase 1b, constatée à 1b.37** : `make value-members` vide (`membres pico8_value_t : 0`) ; `grep -rn offsetof src/` vide ; `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` ; `make sanitize-net` : 15 propres ; `make frames-check` : 15 conformes. Point d'entrée de la phase D : toute entité est une struct à membres C typés ; `pico8_value_t` ne vit plus que dans les tables ob, les arguments et retours de dispatch, et les lecteurs/écrivains par nom des descripteurs. Restent sans objet, non supprimés (aucune étape ne le programme) : le recensement (`pico8_member_census`, ses crochets, `tools/member_census.*`, `make member-census`) ; `make value-members` reste la garde de cette sortie.

- Le message de commit porte : compte 10 → 0, résumé « Runs » (dont les quatre constats de sortie), `gcov`, mutations et trous, `UI_MEMBERS_HASH`, réponses à H1–H26, la liste de l'infrastructure devenue sans objet, et la remarque sur la lettre de TROU-6.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
