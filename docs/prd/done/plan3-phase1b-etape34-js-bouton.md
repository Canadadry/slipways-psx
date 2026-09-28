---
title: "Plan 3, étape 1b.34 : `js`, le bouton. Ses 19 membres typés, `h_` en `pico8_lq_t *` plus `has_h_` et `my` en fonction plus `has_my` (false = présent et NULL), 5 autres indicateurs ; les clés `lq`/`h_`/`jb` passent par des locales que `pq` reconstruit ; `g_js_members` et le dernier `offsetof` de `ui_button.c` supprimés"
description: "Après 1b.33, `pico8_js_t` garde 19 membres `pico8_value_t` : des clés de construction recopiées (`fa`, `ra`, `qf`, `my`, `hb`, `fg`, `px`, `trade`, `en`, `_cr_t`, `_jo_fe_*`), ce que `js:pq` construit (`lq`, `mb`, `kq`, `h_`, `jb`) et `m_`, écrit par `gw`. 1b.34 les type selon R1–R9 de 1b.1. `h_` ({false, lq}) et `my` ({false, fonction}) reprennent la lecture de 1b.7/1b.24 (NULL présent = `false`) ; les clés que `pq` reconstruit dans un autre genre ne sont plus stockées dans le membre mais dans des locales du constructeur (lecture nouvelle de R5) ; l'écrivain (`lq`, `m_`) et la boucle de clés prennent la forme de 1b.13/1b.15 avec des aides par fichier (1b.23b). Un seul commit. `make value-members` passe de 62 à 43."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.34 — `js` (`ui_button.h:20`), genres mêlés `h_`, `my` ; REF vers `lq` (1b.10), `jb` (1b.2), `jo` (1b.23), `hp` (1b.9). Empreinte 38 (15, 23). Compte : 43.

Cadre : puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », §3 (`make test` vert, aucun `*.sha256` modifié), §6, §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Modèles : le PRD de 1b.32 (`plan3-phase1b-etape32-g-sonde.md`, `et` {false, REF} et écrivain à plusieurs genres) et celui de 1b.33 (`plan3-phase1b-etape33-fv-aide.md`, forme la plus récente, table vidée d'un fichier). Notes « Découvert à » **citées, pas redérivées** ; elles lient ce PRD :

- **1b.1** : patron R1–R9 ; s'en écarter est une lecture nouvelle, notée.
- **1b.4b** : **R2 exige désormais `has_m_` pour `js.m_` (et `h_`, `bool,ref:pico8_lq_t`)** — le `l` vient de lectures par nom faites dans les fils ; aucune boucle sentinelle `written[]` ne vise `js`.
- **1b.5** : R7 forme `pico8_X_member` (accesseur `pico8_value_t *` à supprimer, preuve `grep 'pico8_value_t \*' … | grep -v out_value` vide) ; emballage public unique `pico8_X_<m>` pour un membre à genres mêlés.
- **1b.7** : {bool, X} dont le seul booléen écrit est `false` → pointeur + `has_<m>`, NULL présent = `false`, emballage public unique ; **« la forme quand `true` peut être écrit (…, `js.my`…) reste à fixer »** ; R2 : un nil `stat?` venu d'un homonyme ne donne pas d'indicateur à lui seul.
- **1b.9/1b.10** : R7 forme `offsetof` — table vidée supprimée avec son seul client, branches R4 `strcmp` dans l'ordre du typedef ; boucle de clés directe, une branche par clé, une aide `static` par genre, clé inconnue dans l'`else` final au message inchangé ; lecture `ref:autre` (REF non NULL, tag `UNKNOWN`).
- **1b.11** : R2 des TU lents — relus plus bas.
- **1b.13** (forme écrivain de R5) et **1b.15** (aide à indicateur) : une branche par membre `e`, ordre du typedef ; l'aide pose **d'abord** l'état nil, puis sort sur `REF NULL` (genre **et** pointeur testés), sinon `assert` « champ sans règle » qui nomme `struct.membre`, le genre de la ligne et l'arrêt ; `ref:pico8_X_t` : genre REF, tag `STRUCT`, `cls == pico8_X_struct_class()` ; un non-pointeur sans indicateur n'a pas d'état nil, nil y avorte ; nom sans branche → `return 0` ; **test d'arrêt rouge d'abord** avec les cas non-REF à union nulle (`memset` puis `kind`, 1b.14b).
- **1b.14b** : un fixe sans table de défauts de fille garde le littéral de l'init (`fa` = 11) ; une mutation qui retire l'unique appel d'une aide `static` se plante sous `if (0)` (`-Werror`).
- **1b.18** : genre `fonction` en clé — une `FUNCTION` à pointeur NULL avorte ; aide bool de l'écrivain partagée avec l'écriture directe (`fq_take_bool`).
- **1b.19** : fixe sans indicateur reçu par nom → nil avorte ; branches remises dans l'ordre du typedef.
- **1b.20** : `ref:autre` vers une boîte → `pico8_qz_box_t *` ; **`pico8_jb_struct_class()` ajouté à `layout` « sert à 1b.34 »** ; l'`assert` de lecture d'une méthode sur un membre typé passe à l'écriture (aussi 1b.23c).
- **1b.21** : chaîne NULL refusée en clé.
- **1b.22** : REF à plusieurs cibles → `pico8_struct_header_t *` ; ensemble admis = ligne ∪ producteurs prouvés par le code **et** exercés par la caractérisation sur l'ancien code. Ici chaque REF a **une seule cible** (plus bas).
- **1b.23a2** : un membre typé sort des `written[]` des TU (vacant ici) ; `test_pico8_planet_gh_row_lq` est prouvé rouge par le cast aveugle de la liste remis dans `pico8_js_lq_arg_table`.
- **1b.23b** : **une aide d'écriture par genre et par fichier, partagée entre l'écrivain et les boucles de clés de toutes les structs du fichier** ; le grep §6 couvre `(cast)x->m`.
- **1b.23d** : trois tests liés par nom ou par chemin prouvés rouges ; « pointeur différent » n'est pas `assert`able après une libération.
- **1b.24** : {`false` seul, REF} → pointeur + `has_<m>`, NULL présent = `false`, `true` avorte, « `g_.et` et `mz.et` la relisent sur leur preuve » ; preuve de typedef par `awk`.
- **1b.29/1b.32** : un test de régression **sur le chemin** du membre typé, sans diff, est prouvé rouge ; H « clé nommée à `REF NULL` gardée par `pico8_ob_table_set_name` ».
- **1b.30/1b.31/1b.33** : caractérisation dans le TU rapide de la classe ; un nil écrit directement par un test n'est pas une preuve.
- **1b.4 (précédent de lieu)** : `test_ha_pz_members_by_name` vit déjà dans `test_pico8_js`.

### État de départ

HEAD `931f555` (PRD de 1b.33) au moment de l'écriture ; 1b.33 est implémentée **en parallèle** (l'arbre de travail modifie `src/pico8_help.[ch]` et `tests/test_pico8_fv.c`, lus sans être touchés). **Précondition : 1b.33 (`fv`) est commitée avant que 1b.34 commence (H1)** ; `make value-members` rend alors **62** (19 lignes `pico8_js_t`, 16 `pico8_jc_t`, 17 `pico8_mz_t`, 10 `pico8_ui_t`). Numéros de ligne indicatifs (arbre `931f555`), chaque site est aussi désigné par son contenu. **Rien n'a été compilé ni lancé pour ce PRD** : les faits viennent de `grep` et de la lecture ; tout fait d'exécution est une hypothèse (H1–H25).

### Lignes de recensement (`tools/member_census.txt:234-252`)

| membre | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|
| `fa` | `fixe` | jamais | - | l | `pico8_fixed_t` (littéral 11, puis clé) | non |
| `lq` | `ref:pico8_lq_t` | observé | stat? | **le** | `pico8_lq_t *` (construit par `pq`, ou écrit par `ix[2].hb`) | non (pointeur) |
| `mb` | `fixe` | jamais | - | l | `pico8_fixed_t` (`pq`) | non |
| `kq` | `ref:autre` | jamais | - | l | `pico8_qz_box_t *` (`pico8_qz` de `pq`, TROU-7) | non (pointeur) |
| `h_` | `bool`, `ref:pico8_lq_t` | observé | lu,stat | l | `pico8_lq_t *` | **`int has_h_`** (1b.4b ; lecture 1b.7/1b.24) |
| `jb` | `ref:pico8_jb_t` | observé | lu,stat? | l | `pico8_jb_t *` (construit par `pq`) | non (pointeur) |
| `ra` | `ref:autre` | observé | lu,stat? | l | `pico8_vec_t *` (emprunté ; tas de `ph`, TROU-7) | non (pointeur) |
| `qf` | `ref:autre` | observé | lu,stat | l | `pico8_vec_t *` (tas de `pz`) | non (pointeur) |
| `my` | `bool`, `fonction` | observé | lu,stat? | l | `pico8_value_function_t` | **`int has_my`** (lecture 1b.7) |
| `m_` | `bool` | observé | lu,stat? | **le** | `int` (0/1) | **`int has_m_`** (1b.4b) |
| `hb` | `fonction` | observé | lu | l | `pico8_value_function_t` | non (pointeur) |
| `fg` | `fixe` | observé | lu | l | `pico8_fixed_t` | **`int has_fg`** |
| `px` | `chaine` | observé | lu | l | `const char *` (chaîne de la ligne `bo`, empruntée) | non (pointeur) |
| `trade` | `fixe` | observé | lu | l | `pico8_fixed_t` | **`int has_trade`** |
| `en` | `fixe` | observé | lu | l | `pico8_fixed_t` | **`int has_en`** |
| `_cr_t` | `ref:table` | observé | lu | l | `pico8_ob_table_t *` (entrée de `nb`, empruntée) | non (pointeur) |
| `_jo_fe_pt` | `ref:pico8_jo_t` | observé | lu | l | `pico8_jo_t *` (emprunté) | non (pointeur) |
| `_jo_fe_b` | `ref:pico8_hp_t` | observé | lu | l | `pico8_hp_t *` (emprunté) | non (pointeur) |
| `_jo_fe_od` | `fixe` | observé | lu | l | `pico8_fixed_t` | **`int has__jo_fe_od`** |

Compte : 62 − 19 = **43**. Aucune ligne `pico8_js_t` ne reste ; les lignes `jc`, `mz`, `ui` (1b.35–1b.37) ne bougent pas. Indicateurs : 7 (`has_h_`, `has_my`, `has_m_`, `has_fg`, `has_trade`, `has_en`, `has__jo_fe_od`), chacun juste après son membre, ne comptent pas (R9).

### Le cart (`pico8/lua.lua`)

- `js=ke:jr([[fa=11,]])`, `js.hd=gk` (`:258-260`). **`js:pq`** (`:261-262`) : `self.lq=lq(mv(self,"lq"))`, `self.mb=self.lq.w+2`, `self.kq=qz(0,-1,self.mb,11)`, **`self.h_=self.h_ and lq(self.h_)`**, **`self.jb=self.jb and jb(self.jb)`**. `js:ph` (`:263-267`) : `if self.qf` … `self.ra+=…`. `js:ev` (`:268-270`) : `if self.jb and self.m_`. `js:kj` (`:271-276`) : `c=self.my`, `self.m_`, `self.lq:ol`, `local c=self.h_ … c.bg`. `js:ldown` (`:277-279`) : `rn(self.my and 60)mv(self,"my")`.
- `pz` (`:279-287`) : `b.ra,b.qf,b.fa=ra,bp,p.ca` puis `js(b)` ; `ra` est **le même vecteur** pour toutes les lignes.
- Producteurs de lignes : `fv.pq` (`d.my=eu`, `:1167`) sur `bo` (`:1190-1197` : `lq`, `px`, `trade`, `en`) ; `ix` (`:966-983` : `ra`, `lq`, `my`, `ix[2].hb`, qui écrit **`self.lq=lq({…})`**) ; `ha` (`:985-997` : `lq`, `fg=5` sur la 4e, `my`) ; `pj:ph` (`:870-876` : `fa=15`, `ra`, `lq`, `{my=_init}` et `de`) ; **`cr`** (`:955-962` : `lq`, `h_` table, `jb=t.oy`, **`my=t.e_ and function()…end`**, donc `false` ou une fonction ; la ligne `invented` : `jb=fm.invented`) ; **`jo:fe`** (`:418-421` : `lq=mv(b,"lq")`, **`h_=od>0 and {…}`**, donc `false` ou une table, `my=function()…end`, `jb=b:jb()`) ; **`jo:gh`** (`:422-428` : `bs[1].h_=self.kr and {…}`, nil ou une table).
- **`true` n'est jamais écrit dans `my` ni dans `h_`** ; nil non plus par nom. `m_` : `ui:gw` écrit `true`/`false`. Le cart ne relit `m_`, `my`, `h_`, `jb` que dans `js:*` et `mv`.

### Le code

**`src/pico8_ui_button.h`** : typedef `pico8_js_t` (`:18-38`), `ke` puis 19 `pico8_value_t` ; commentaire de tête (« `my` and `hb` are closures … `ra` may be the vector every button of one `pz` shares: never written through. Nothing frees the struct: TROU-7 ») qui reste vrai. `void pico8_js_pq(pico8_js_t *self);` (`:59`). Inclusions : `gfx`, `ke`, `mem`, `ob`, `struct`, `value`, `vec` ; ni `pico8_layout.h`, ni `pico8_geom.h`, ni `pico8_planet.h`, ni `pico8_research.h`. `pico8_fq_t` (`:72-82`, 1b.18) typé.

**`src/pico8_ui_button.c`** :

- `pico8_js_is_lq` (`:20-23`, tag `STRUCT` et `pico8_lq_struct_class()`), `pico8_js_lq_arg_table` (`:29-47`, table / liste / rangée ; `assert` « untagged or unmeasured REF reached js:pq's lq()/h_() argument »), `pico8_js_truthy` (`:53-55`).
- `g_js_members[]` (`:97-120`, 19 entrées `offsetof`) et `pico8_js_member` (`:122-131`, rend `pico8_value_t *`) ; trois clients : le lecteur `pico8_js_field` (`:133-144`, REF NULL → 0, repli `pico8_ke_field`), la boucle nil de `pico8_js_struct_new` et sa boucle de clés.
- Écrivain `pico8_js_set_field` (`:146-159`) : `m_` puis `lq` recopiés **quel que soit le genre**.
- `pico8_js_struct_new` (`:173-207`) : 19 `REF NULL`, `fa` = 11, boucle de clés (refus de `mb`, `kq`, `m_`, positionnels, inconnus par `assert(member != NULL && "js: the only measured construction keys are fa, lq, h_, jb, ra, qf, my, hb, fg, px, trade, en, _cr_t and _jo_fe_pt/_b/_od; …")`, **recopie de tout genre**, puis **TROU-23** : clé `h_` ou `jb` à nil → `pico8_value_bool(1)`), `pico8_js_pq`, `pico8_ke_struct_pq`, `PICO8_MEMBER_CENSUS_SEE`.
- `pico8_js_pq` (`:248-278`) : lit `self->lq`, `self->h_`, `self->jb` **dans leur genre de clé** (table, liste, rangée, `true`, `false`) et les **remplace** par ce qu'il construit (`pico8_lq_t *`, `pico8_jb_t *`) ; `false` reste `false`. C'est l'« angle mort » de 1b.0 (un genre écrit puis écrasé avant l'observation) : la ligne ne voit que le genre d'après `pq`.
- Lectures directes : `pico8_js_ph` (`qf`, `ra`, écriture de `ra`), `pico8_js_ev` (`jb`, `m_`, `assert` de tag `STRUCT` sur `jb`), `pico8_js_kj` (`mb`, `my`, `m_`, `lq`, `h_` et ses `w`/`bg`), `pico8_js_ldown` (`my`, puis `pico8_mv(self, "my", …)` par nom).
- Partie `fq` (1b.18) : `fq_take_bool` (`:418-429`, `m_` et `p_` de `fq`), `fq_take_function` (`:431-440`, clé `my` de `fq`).
- `offsetof` : 20 occurrences, toutes `pico8_js_t` ; `<stddef.h>` inclus.
- **`pico8_js_pq` n'a aucun appelant hors de `pico8_js_struct_new`** (fait, grep de `src/` et `tests/`) ; `pico8_js_kj` est appelé directement par `tests/test_pico8_lq.c:436,464`.

**Sites par nom, inchangés** (grep `"(M)"` sur `src/`, `M` les 19 noms) : `pico8_ui_gw` (`ui.c:408,413` écrit `m_` en `pico8_value_bool(0/1)` ; `:417` lit `fg`), `pico8_gk` (`entity_geom.c`, `kq`), `pico8_ix2_hb_dispatch` (`menu.c:361`, écrit `lq` = un `pico8_lq_t` neuf), `pico8_cr_row_my_dispatch` (`menu.c:115`, `_cr_t`), `pico8_jo_fe_my_dispatch` (`planet_display.c:355-362`, `_jo_fe_pt`/`_b`/`_od`), `pico8_js_ldown` (`mv(self,"my")`), `economy_score.c:175,216` (`trade` de `dk`), `galaxy.c:123` (`en` de `dk`), `score_panel_display.c:218` (`px` de `dk`), la lecture des noms de méthodes de `entity.c` (`hb`). Les autres lignes (`msgbox.c`, `particle.c`, `probe.c`, `slipway_display.c`, `wormhole_display.c`, `planet_display.c:153`…) portent des homonymes.

**Tests** : accès direct à un des 19 membres d'un `js` : **aucun** (grep `(->|\.)(M)\b` sur les fichiers qui nomment `pico8_js_t` : seuls des homonymes de `mh`, `iv`, `pj`, `g_`, `qb`) ; aucune boucle sentinelle `written[]` ne vise `js`. `tests/test_pico8_js.c` (TU de la classe, rapide, ~3 s mesuré à 1a.16/1a.30, dans `CENSUS_UNIT_TESTS`) fige déjà tout par nom : `fold_button` (19 noms), `test_draw` (`h_` `false` présent : `bool_state(free_, "h_") == 1`), `test_nil_keys` (TROU-23), `test_js_is_a_struct` (écrivain `m_`/`lq`, `must_abort(write_ra/my/mb)`, clés refusées, `fa` 11, `!has(bare,"my")`), `test_fe_my`, `test_cr_my`, `test_hb`, `test_dk`, cinq rejeux.

### Revue des TU lents (R2 de 1b.11)

`grep -nE 'pico8_js_|is_button|"(mb|h_|qf|my|hb|trade|en|_cr_t|_jo_fe_pt|_jo_fe_b|_jo_fe_od|px|fg)"'` sur `test_pico8_{ie,jo,qh,os,hj,pm,mx,nr}.c` : `pico8_eu(pico8_js_new(bo_row(2)))` au démarrage, `"px"` et `"hb"` lus sur leurs propres classes (planètes, icônes, `cv("hb")`), jamais un non-pointeur de `js` lu nil par nom. **Aucun indicateur de plus que les 7 de la ligne** (H16). `hb` lu nil par `mv` sur un `js` est un pointeur (NULL = nil).

## Solution

1. **Caractérisation figée sur l'ancien code** (arbre 1b.33 commité), dans le TU rapide `test_pico8_js`, par nom seul ; verte avant tout typage.
2. **Test d'arrêt rouge d'abord** (écrivain, clés, `jb` = `false`) dans le même TU.
3. **Commit 1b.34** : les 19 membres typés, 7 indicateurs, emballages publics `pico8_js_h_` et `pico8_js_my` ; `g_js_members` et `pico8_js_member` supprimés avec leurs boucles ; lecteur à 19 branches ; écrivain à 2 branches ; boucle de clés à branches typées, dont trois vers des **locales** que `pico8_js_pq` reçoit ; aides `static` au préfixe du fichier (`button_take_*`), les deux de `fq` renommées ; plus aucun `offsetof` dans `ui_button.c` ; `make value-members` rend **43**.
4. Aucun pixel ne bouge. Note « Découvert à 1b.34 » sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `ui_button.h`, je veux `pico8_js_t` sans `pico8_value_t`, chaque membre au type de sa ligne, chaque `has_` juste après son membre.
2. En tant que lecteur par nom (`gw`, `gk`, `mv`, `cr`, `jo:fe`, `economy`, `galaxy`, `score_panel`, les TU), je veux que chacun des 19 noms rende sur tout `js` exactement ce que rendait l'ancien code : présence, genre, valeur, identité du pointeur.
3. En tant que `pz`, je veux que tous les boutons d'un même appel partagent le **même** vecteur `ra`, et que chacun ait son `qf`, son `fa` = `p.ca` quand `p` en a un (TROU-9), 11 sinon.
4. En tant que ligne de `cr`, je veux `my` = `false` lu `BOOL` 0 présent pour une tech trop chère, la fonction sinon, et `_cr_t` au pointeur de l'entrée de `nb`.
5. En tant que ligne de `jo:fe`, je veux `_jo_fe_pt` = la planète, `_jo_fe_b` = le projet, `_jo_fe_od` le coût, `h_` = `false` présent pour un bâtiment gratuit, une rangée sinon, et `jb` construit même quand `b:jb()` est nil (TROU-23).
6. En tant que ligne de `jo:gh`, je veux `lq` construit depuis une **liste** (le cast aveugle de 8361468 ne revient pas) et `h_` construit même quand `kr` est nil (TROU-23).
7. En tant que `gw`, je veux écrire `m_` = `true`/`false` relu `BOOL` 1 / `BOOL` 0 présent, et nil qui éteint `m_`.
8. En tant que `ix[2].hb`, je veux écrire `lq` = une rangée neuve, relue au même pointeur.
9. En tant que `kj`/`ev`/`ldown`/`ph`, je veux dessiner et réagir exactement comme avant (couleur selon `my` et `m_`, pastille `h_`, infobulle `jb`, `ra` qui glisse vers `qf`).
10. En tant que gardien de chaque membre, je veux que tout genre hors ligne avorte, union nulle comprise : `true` et fonction NULL pour `my`, nil pour `fa`, chaîne NULL pour `px`, toute struct hors cible, et `false` pour la clé `jb`.
11. En tant que mainteneur, je veux `g_js_members`, `pico8_js_member` et tout `offsetof` absents de `ui_button.c`.
12. En tant que tests de régression `planet_gh_row_lq`, `pause_menu_music`, `built_planet_panel_nil_row`, `ga_export_panel_by`, `rich_planet_exports`, `playability`, `tech_screen`, je veux rester verts sans diff, et rouges quand mon bug revient.
13. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
14. En tant que `plan-pilot`, je veux `make value-members` à 43 sans ligne `pico8_js_t`.
15. En tant que vérificateur, je veux la caractérisation verte sur l'ancien code, le test d'arrêt rouge puis vert, les preuves rouges §6, les mutations et leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle : les validations que demande `to-prd` (modules, tests) sont tranchées comme le ferait un agent `guideline` — hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables — et marquées (G).

### Lecture nouvelle de R5 : une clé que `pq` reconstruit n'est pas le membre (G)

`lq`, `h_`, `jb` arrivent en clé dans un genre (table, liste, rangée, `true` de TROU-23, `false`) que leur membre ne tient plus après `pq` (`pico8_lq_t *`, `pico8_jb_t *`). Aucune règle du §5 ne dit où ranger une valeur de clé qui n'est pas dans le genre du membre : ce n'est pas un « champ sans règle » (le **membre** a sa ligne et son type), c'est un argument de `js:pq`.

- Le constructeur tient trois **locales** `pico8_value_t lq_arg, h__arg, jb_arg`, toutes `pico8_value_ref(NULL)` au départ (clé absente). La boucle de clés y recopie la valeur de la clé, **sans aide ni `assert`** (la validation reste là où `pq` la faisait) ; **TROU-23** s'applique à la locale : clé `h_` ou `jb` à `REF NULL` → `pico8_value_bool(1)`, `/* TROU-23 */` gardé au site.
- **`void pico8_js_pq(pico8_js_t *self, pico8_value_t lq, pico8_value_t h_, pico8_value_t jb);`**, public, commentaire de contrat dans l'en-tête : « `lq`, `h_`, `jb` are the row's construction values `js:pq` builds the members from, TROU-23 already applied; borrowed, never kept ». Corps :
  - `self->lq = pico8_js_lq(lq); self->mb = self->lq->w + 2; self->kq = pico8_qz(0, -1, self->mb, 11);` (`pico8_js_lq`, `pico8_js_lq_arg_table` inchangées) ;
  - `h_` : `pico8_js_truthy(h_)` → `self->h_ = pico8_js_lq(h_); self->has_h_ = 1;` ; sinon `h_.kind == PICO8_VALUE_BOOL` (`false`) → `self->h_ = NULL; self->has_h_ = 1;` ; sinon (nil) l'état de R3 reste ;
  - `jb` : `pico8_js_truthy(jb)` → l'`assert` de tag `OB_TABLE` existant, texte inchangé, puis `self->jb = pico8_jb_new(…)` ; sinon `jb.kind == PICO8_VALUE_BOOL` (`false`) → `assert(0 && "js.jb: a key false is off its census row (ref:pico8_jb_t) -- a field under no rule of plan-3 section 5: stop, do not widen the member")` (le membre, pointeur seul, ne peut pas tenir `false` ; H6) ; sinon nil.
- Le membre ne reçoit que ce que `pq` construit : toujours dans le genre de sa ligne. Aucune écriture `*(pico8_value_t *)` ne reste.
- **Écartés** : garder des membres `pico8_value_t` de transit (contraire à la **Sortie**) ; `pq` qui relit la table de clés (TROU-23 quitterait la boucle, la recherche de clé serait faite deux fois) ; `pico8_js_pq` rendue `static` (changement d'interface aussi grand, et le nom du cart `js:pq` perd son point d'entrée public) ; une aide `button_take_*` sur ces trois clés (elle devrait accepter tout genre qu'accepte `pico8_js_lq_arg_table`, donc ne garderait rien).

### `h_` : `pico8_lq_t *h_` + `int has_h_` ; `my` : `pico8_value_function_t my` + `int has_my` — lecture de 1b.7/1b.24 (G)

- **`h_`** : nil (clé absente, lu par nom : `test_nil_keys`, `fold_button`) ; `false` (clé `false` de `jo:fe` à `od` ≤ 0, lu `BOOL` 0 présent : `test_draw`) ; une rangée. `true` n'existe que dans la locale, jamais dans le membre (H3). Cible unique `lq` : **pointeur typé** (R1 `ref:pico8_X_t`), pas le pointeur d'en-tête de 1b.24, qui ne sert qu'aux cibles multiples (1b.22).
- **`my`** : nil (lu par nom : `!has(bare, "my")`), `false` (`cr`, tech trop chère : `bool_state(…, "my")` de `test_research`/`test_cr_my`), une fonction non NULL (`bo`, `ix`, `ha`, `pj:ph`, `cr`, `jo:fe`). **`true` n'est jamais écrit** : cart (`t.e_ and function…`), C (seul `pico8_value_bool(0)` dans `pico8_cr_step`), TU (`counting_my`, `false` à `test_pico8_js:788`) (H3). Ferme pour `js.my` la question ouverte par 1b.7, comme 1b.24 l'a fait pour `qh.et`.
- États, pour les deux : `has_<m>` = 0 nil (pointeur NULL, non lu) ; `has_<m>` = 1 et NULL : `false` ; sinon la rangée / la fonction, empruntée.
- **Emballages publics uniques**, déclarés dans `ui_button.h` avec un commentaire de contrat (REF NULL pour `self` NULL ou nil, `BOOL` 0 pour présent-NULL, sinon REF / `FUNCTION`) : `pico8_value_t pico8_js_h_(const pico8_js_t *self)`, `pico8_value_t pico8_js_my(const pico8_js_t *self)`, forme de `pico8_g__et`/`pico8_hy_hl`, appelés par le lecteur.
- **Lectures directes** (R6) : `kj` et `ldown` testent `self->my != NULL` (NULL couvre nil et `false`, tous deux faux en Lua) ; `kj` teste `self->h_ != NULL` ; aucune n'a besoin de `has_`.
- Rejetées : `<m>_is_bool` (vaudrait `has_<m> && <m> == NULL`) ; NULL = `false` sans indicateur (nil et `false` se confondraient, lus par nom) ; `pico8_struct_header_t *` pour `h_` (une seule cible).

### REF à cible unique (lecture de 1b.22 sans producteur ajouté) (G)

- **`_jo_fe_pt` → `pico8_jo_t *`**, cible `jo` exactement. Preuve par le code : seul `pico8_jo_fe_step` l'écrit, avec `ctx->self` de `pico8_jo_fe`, appelé seulement par `pico8_jo_ldown` quand `me == "known"` ; `known` n'est posé que par `jo:hh` (`ks(self.nk.me or "known")`), que `mz:pr` n'appelle que sur `cg.jo` à `me == "unknown"` ; les filles de l'arbre ont `me` = `kz` (`qh` et descendance) ou `ly`, `ga` n'est pas dans `cg.jo` (`nu=o()`), `nr`/`fr` ont leur propre `ldown` ; une fille née de `jo:hu(cw)` est un objet neuf. Ensemble = ligne, aucun producteur à ajouter (H5).
- **`_jo_fe_b` → `pico8_hp_t *`** (`pico8_hp_struct_class()`), **`lq`, `h_` → `pico8_lq_t *`** (`pico8_js_is_lq`), **`jb` → `pico8_jb_t *`** (construit par `pq` seulement : ni écrivain ni clé typée, donc **`pico8_jb_struct_class()` reste sans client dans `ui_button.c`** — noté).

### Aides R5 (forme 1b.13/1b.15), toutes `static` dans `ui_button.c`, au préfixe du fichier (G)

**Lecture de 1b.23b** : une aide par genre **et par fichier**, partagée par toutes les structs du fichier. `ui_button.c` porte `js` et `fq` : les deux aides de `fq` (1b.18) sont **renommées** `button_take_bool` et `button_take_function`, **messages élargis** aux membres de `js` (précédent 1b.30/1b.31 : message élargi, comportement inchangé), et réutilisées par `js`. Les autres sont nouvelles.

| aide | membres | état nil puis sortie | accepté (`assert`) | message (préfixe) |
|---|---|---|---|---|
| `button_take_bool(value, int *out, int *has)` (ex-`fq_take_bool`) | `fq.m_`, `fq.p_`, **`js.m_`** | `*out = 0; *has = 0` | `BOOL` ; `*out = boolean`, `*has = 1` en dernier | `"fq.m_/p_, js.m_: a write neither nil nor a boolean is off its census row (bool) -- …"` |
| `button_take_function(value, pico8_value_function_t *out)` (ex-`fq_take_function`) | clé `fq.my`, **clé `js.hb`** | `*out = NULL` | `FUNCTION` non NULL | `"fq.my, js.hb: a key neither nil nor a non-NULL function is off its census row (fonction) -- …"` |
| `button_take_my(value, pico8_value_function_t *out, int *has)` | clé `js.my` | `*out = NULL; *has = 0` | `BOOL` 0 (`*out = NULL`), ou `FUNCTION` non NULL ; `*has = 1` en dernier ; **`true` avorte** | `"js.my: a key neither nil, false, nor a non-NULL function is off its census row (bool,fonction) -- …"` |
| `button_take_fixed(value, pico8_fixed_t *out, int *has)` | clés `fg`, `trade`, `en`, `_jo_fe_od` | `*out = 0; *has = 0` | `FIXED` ; `*has = 1` en dernier | `"js.fg/trade/en/_jo_fe_od: a key neither nil nor a fixed is off its census row (fixe) -- …"` |
| `button_take_fa(value, pico8_fixed_t *out)` | clé `fa` | **aucun** (nil avorte, 1b.19, forme `jo_take_ma`) | `FIXED` | `"js.fa: a key that is not a fixed is off its census row (fixe, never nil) -- …"` |
| `button_take_vec(value, pico8_vec_t **out)` | clés `ra`, `qf` | `*out = NULL` | REF non NULL, tag `UNKNOWN` (lecture `ref:autre` de 1b.10) | `"js.ra/qf: a key neither nil nor a vector ref is off its census row (ref:autre) -- …"` |
| `button_take_string(value, const char **out)` | clé `px` | `*out = NULL` | `STRING` non NULL (1b.21) | `"js.px: a key neither nil nor a string is off its census row (chaine) -- …"` |
| `button_take_table(value, pico8_ob_table_t **out)` | clé `_cr_t` | `*out = NULL` | REF non NULL, tag `OB_TABLE` | `"js._cr_t: a key neither nil nor a table is off its census row (ref:table) -- …"` |
| `button_take_jo(value, pico8_jo_t **out)` | clé `_jo_fe_pt` | `*out = NULL` | REF, tag `STRUCT`, `cls == pico8_jo_struct_class()` | `"js._jo_fe_pt: a key neither nil nor a jo is off its census row (ref:pico8_jo_t) -- …"` |
| `button_take_hp(value, pico8_hp_t **out)` | clé `_jo_fe_b` | `*out = NULL` | REF, tag `STRUCT`, `cls == pico8_hp_struct_class()` | `"js._jo_fe_b: a key neither nil nor an hp is off its census row (ref:pico8_hp_t) -- …"` |
| `button_take_lq(value, pico8_lq_t **out)` | écrivain `js.lq` | `*out = NULL` | REF non NULL et `pico8_js_is_lq` | `"js.lq: a write neither nil nor an lq row is off its census row (ref:pico8_lq_t) -- …"` |

Chaque message finit par `"a field under no rule of plan-3 section 5: stop, do not widen the member"`. Nil = `value.kind == PICO8_VALUE_REF && value.as.ref == NULL` (genre **et** pointeur). Un `false` à union nulle est un `false` présent pour `m_` et `my`.

**Écrivain `pico8_js_set_field`** : 2 branches dans l'ordre du typedef (**`lq` avant `m_`**, précédent 1b.19), `if (strcmp(name, "lq") == 0) { button_take_lq(value, &js->lq); return 1; }`, puis `m_` par `button_take_bool(value, &js->m_, &js->has_m_)`, puis `return 0`. Commentaire de contrat (« `m_` … and `lq` …: the only names a generic writer ever writes on a `js` ») gardé. L'ensemble des 2 noms ne change pas ; `mb` n'est pas un `l`/`e` écrit (`must_abort(write_mb)` reste).

**Boucle de clés de `pico8_js_struct_new`** (lecture 1b.10/1b.29) : une branche par clé nommée — `fa` (`button_take_fa`), `lq`/`h_`/`jb` (locales, TROU-23), `ra`/`qf` (`button_take_vec`), `my` (`button_take_my`), `hb` (`button_take_function`), `fg`/`trade`/`en`/`_jo_fe_od` (`button_take_fixed`), `px`, `_cr_t`, `_jo_fe_pt`, `_jo_fe_b` ; `mb`, `kq`, `m_`, clé inconnue ou positionnelle → `else { assert(0 && "js: the only measured construction keys are fa, lq, h_, jb, ra, qf, my, hb, fg, px, trade, en, _cr_t and _jo_fe_pt/_b/_od; another would be a field under no rule of plan-3 section 5 -- stop, do not add a member"); }`, **texte inchangé**.

### R1–R9 appliquées

| règle | sur `js` |
|---|---|
| R1 | Tableau de recensement ; nom et place gardés ; chaque `has_<m>` juste après `<m>` avec un commentaire de contrat court (forme `ui_button.h` de `fq`, `planet_family.h:45`). **Inclusions ajoutées à `ui_button.h`** : `pico8_geom.h` (`pico8_qz_box_t`), `pico8_layout.h` (`pico8_lq_t`, `pico8_jb_t`), `pico8_planet.h` (`pico8_jo_t`), `pico8_research.h` (`pico8_hp_t`) ; rien d'autre (H7). |
| R2 | Les 7 indicateurs de la ligne. `fa`, `mb` jamais nil ; pointeurs sans indicateur. |
| R3 | `pico8_js_struct_new` : après `pico8_ke_struct_init`, tous les pointeurs à NULL, `mb`, `fg`, `trade`, `en`, `_jo_fe_od`, `m_` à 0, les 7 `has_` à 0, puis `self->fa = pico8_fixed_from_int(11)` (littéral, 1b.14b) ; remplace la boucle `REF NULL` ; puis les clés, puis `pico8_js_pq(self, lq_arg, h__arg, jb_arg)`, `pico8_ke_struct_pq`, `PICO8_MEMBER_CENSUS_SEE`. `size_t i` retiré. |
| R4 | `pico8_js_field` : 19 branches `strcmp` dans l'ordre du typedef, puis `return pico8_ke_field(self, name, out_value);`. `fa`, `mb` : toujours `FIXED`. `fg`, `trade`, `en`, `_jo_fe_od` : `has_` = 0 → `return 0`. `m_` : `has_m_` = 0 → 0, sinon `pico8_value_bool(m_)`. `h_`, `my` par leur emballage (REF NULL → 0). Pointeurs (`lq`, `kq`, `jb`, `ra`, `qf`, `_cr_t`, `_jo_fe_pt`, `_jo_fe_b` en REF ; `hb` en `FUNCTION` ; `px` en `STRING`) : NULL → 0. `*out_value` intact sur 0. |
| R5 | Écrivain, clés, locales de `pq` : ci-dessus. |
| R6 | `pico8_js_ph` : `if (self->qf != NULL)`, `ra = self->ra != NULL ? *self->ra : (0,0)`, `self->ra = heap_ra` (TROU-7 gardé). `pico8_js_ev` : `if (self->jb != NULL && self->has_m_ && self->m_)`, **l'`assert` de tag de `jb` disparaît** (`jb` n'a d'autre source que `pico8_jb_new`, lecture 1b.20/1b.23c). `pico8_js_kj` : `mb = self->mb`, `has_my = self->my != NULL`, `m_flag = self->has_m_ && self->m_`, `self->lq != NULL`, `self->h_ != NULL` (`c = self->h_`). `pico8_js_ldown` : `has_my = self != NULL && self->my != NULL`, `pico8_mv(…, "my", …)` par nom inchangé. Aucun accès direct dans `tests/`. |
| R7 | Forme `offsetof` de 1b.9 sur la table entière : `g_js_members` et `pico8_js_member` supprimées avec leurs trois clients ; **plus aucun `offsetof` dans `ui_button.c`**, `#include <stddef.h>` retiré (précédents 1b.20, 1b.31, 1b.33 ; H19). Pas de table de défauts (spec `fa=11` seule, lue par `pico8_js_default`, inchangée) : R7 de 1b.14a vacante ; aucune fille. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_js_t)` gardé ; `tools/member_census.txt` non édité. |
| R9 | 62 → 43. |

### Interaction avec `fq` (1b.18) et les étapes suivantes

- `fq` ne change que par le renommage de ses deux aides et l'élargissement de leurs messages ; ses types, son lecteur, son écrivain, sa boucle de clés et `pico8_fq_ldown` gardent leur forme (H18).
- `ui.o_`/`ui.gj` (1b.37) listent `ref:pico8_js_t` : sans effet d'ordre, `js` y reste une cible d'en-tête.

### Tests touchés (G)

- **`tests/test_pico8_js.c`** seul, **par ajouts** : caractérisation, test d'arrêt, `JS_MEMBERS_HASH`, aides de test (valeur hors ligne dans une globale, forme `g_off_row` de `test_pico8_ui`/`test_pico8_fv`), deux appels `in_fresh_child` dans `main`. Aucune fonction existante, aucune valeur figée, aucun commentaire existant ne change.
- Aucun autre fichier de `tests/` (aucun accès direct, aucune boucle sentinelle ; `test_pico8_lq` appelle `pico8_js_kj` sur un `pico8_js_t *`, inchangé).

### Revue des tests de régression, mécanique

**Liste** (reprise de 1b.30–1b.33) : les tests nommés au §6 (les 17 de régression) et dans les lignes 1a : `tests/test_pico8_<nom>.c` pour `built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`, `bg`, `entity_kind`, `hj`, `jc`, `jo`, `jo_spec_defaults`, `js`, `mo`, `mz`, `nl`, `os`, `pm`, `qh`, plus `tests/test_class_coverage.c`. Tous existent.

**Greps faits pour ce PRD** (`M = fa|lq|mb|kq|h_|jb|ra|qf|my|m_|hb|fg|px|trade|en|_cr_t|_jo_fe_pt|_jo_fe_b|_jo_fe_od`) :

- accès direct : `grep -nE '(->|\.)(M)\b'` sur les fichiers qui nomment `pico8_js_t` → aucun sur un `js` (homonymes `mh`/`iv`/`pj`/`g_`/`qb`, commentaires) ;
- par nom `"(M)"` **sur un `js`** : `built_planet_panel_nil_row:95`, `ga_export_panel_by:48`, `planet_gh_row_lq:59`, `rich_planet_exports:104` (`lq` d'un bouton de `ok.le`) ; `pause_menu_music:130` (`my` de `ok.o_`) — les cinq lecteurs par nom de 1a.16 ; ailleurs homonymes (`empty_planet_cleared` : `ra` de planètes) ;
- chemin : tout test qui démarre une partie construit un `js` (`pico8_eu(pico8_js_new(bo[2]))`, `ix` par `eu`, boutons de `pz`) ; en plus, les scénarios qui lisent un membre de `js` par nom **dans du code du jeu** : `playability` (étape 3 : clic sur une ligne de `jo:fe` → `mv(self,"my")` → `_jo_fe_pt/_b/_od` par nom), `geoharvest_burst` (étape 4, même chemin), `tech_screen` (étape 4 : clic sur une ligne de `cr` → `my` → `_cr_t` par nom ; lignes à `my` = `false` dessinées), `slipway_drag` (clic de titre sur un bouton de difficulté → `my` = `eu`), `game_end` (`pj:ph` construit deux `js`, `px` de `dk` lu par nom pour l'étiquette).

| # | test | accès / chemin | diff ? | preuve rouge |
|---|---|---|---|---|
| R-a | `planet_gh_row_lq` | `"lq"` par nom (branche R4 typée) ; le `lq` de la ligne de `jo:gh` est une **liste** passée à `pico8_js_pq` par la **locale** `lq_arg` : **sur le chemin** de la lecture nouvelle | non | **Obligatoire** (1b.23a2). Bug d'origine **8361468** remis : le cas `PICO8_TYPE_TAG_LIST` de `pico8_js_lq_arg_table` remplacé par le cast aveugle `return (const pico8_ob_table_t *)v.as.ref;` → rouge (rapport ASan ou `assert` de champ) ; restauré → vert. Lancé par `make unit-run-test_pico8_planet_gh_row_lq`. |
| R-b | `pause_menu_music` | `"my"` par nom sur `ok.o_` (emballage `pico8_js_my`), attendu `FUNCTION` ; clé `my` des lignes `ha` par `button_take_my` ; `fg` de la 4e ligne | non | **Obligatoire.** Bug de référence de 1a.16 remis : dans `pico8_ix4_my_dispatch` (`menu.c`), `pico8_pz` appelé **avant** les quatre installations de `my` → rouge (`hovered_my` absent à la frame 20) ; restauré → vert. |
| R-c | `built_planet_panel_nil_row` | `"lq"` par nom sur les boutons du panneau (`jo:gh`, `h_` TROU-23) | non | **Obligatoire.** Bug d'origine **3b8ba2c** remis : clause `REF NULL` de `pico8_msgbox_truthy` (`msgbox.c:40`) retirée → rouge (`built_rows == BUILT_PANEL_EXPECTED_ROWS` faux) ; restauré → vert. |
| R-d | `ga_export_panel_by` | `"lq"` par nom sur les boutons du panneau d'un `ga` | non | **Obligatoire.** Bug d'origine **9183fee** remis (recette de 1b.29) : bloc d'échange de `by` retiré de `pico8_planet_family_init` → rouge (`SIGABRT` dans `family_default_list`) ; restauré → vert. |
| R-e | `rich_planet_exports` | `"lq"` par nom sur les boutons de `ok.le` (`promote_and_open_panel`) | non | **Obligatoire.** Bug d'origine **50f91b4** remis (recette de 1b.23a2) : dans `bb_visit` (`economy_tiers.c`), `ob_sequence_length(po)` redevient le compte de `po` → rouge (`mu4 == 3` faux) ; restauré → vert. |
| R-f | `playability` | étape 3 : ligne de `jo:fe` (`_jo_fe_*`, `my` fonction, `h_` `false` ou rangée, `jb` TROU-23), `mv(self,"my")` puis `_jo_fe_pt/_b/_od` par nom : **sur le chemin** | non | **Obligatoire.** Bug de référence de 1a.17 (constructeur de `jo` appelant `pico8_ke_struct_pq` deux fois, recette de 1b.32 R-b) → rouge (`count <= PICO8_PLAYABILITY_MAX_PLANETS`) ; restauré → vert. |
| R-g | `tech_screen` | étape 4 : ligne de `cr` (`my` `false` ou fonction, `_cr_t`, `h_`, `jb` = `t.oy`), clic → `my` → `_cr_t` par nom : **sur le chemin** | non | **Obligatoire.** Pas de correctif d'historique ; bug de référence nommé par 1a.16 (« sans `_cr_t`, le clic sur une tech ne ferait rien, en silence : seuls `tech_screen` et la caractérisation l'attraperaient ») : dans `pico8_cr_step` (`menu.c:171`), `pico8_ob_table_set_name(row, "_cr_t", …)` mis sous `if (0)` → rouge (aucune tech inventée) ; restauré → vert. |
| R-h | `slipway_drag` | clic de titre sur un bouton de difficulté (`my` = `eu`, `px`/`trade`/`en` en clés) : **sur le chemin** | non | **Dans le budget seulement.** Bug d'origine **2d7021f** remis (recette de 1b.32 R-d) → rouge ; restauré → vert. Sinon « non lancé » au journal. |
| — | `js` (TU de 1a.16) | TU de la classe | **ajouts seuls** (caractérisation, test d'arrêt) | aucune de plus : la caractérisation est verte sur l'ancien code, le test d'arrêt y est rouge (lignes « Runs ») ; valeurs figées existantes inchangées (H20). |
| — | `game_end`, `geoharvest_burst`, `score_panel_stars`, `score_panel_close`, `empty_planet_cleared`, `cheat_code` | construisent les `js` de `eu`/`ix`/`pz` ; `game_end`/`score_panel_stars` passent par `pj:ph` et l'étiquette `px` de `dk` ; `geoharvest_burst` par une ligne de `jo:fe` (même chemin que R-f) | non | aucune ; verts sans modification (valeurs figées par `test_pj`, `test_dk`, `test_fe_my` et le filet). |
| — | `bg`, `entity_kind`, `hj`, `jc`, `jo`, `jo_spec_defaults`, `mo`, `mz`, `nl`, `os`, `pm`, `qh`, `layout_nil_item_draws_nothing`, `planet_kj_dispatch`, `probe_mm_rnd_order`, `test_class_coverage` | homonymes seuls, ou `js` construit au démarrage sans scénario qui le lise | non | aucune. |

### Preuves (sortie attendue sur l'arbre final)

**Sortie de `js`** :

- `awk '/^typedef struct \{/{b=""} {b=b $0 "\n"} /\} pico8_js_t;/{printf "%s", b}' src/pico8_ui_button.h | grep -c pico8_value_t` : **0** ;
- `make value-members` : **43**, `grep -c pico8_js_t` : **0** ; seules les 19 lignes `pico8_js_t` disparaissent ;
- `grep -rnw 'g_js_members\|pico8_js_member' src/` : **vide** ; `grep -c 'offsetof' src/pico8_ui_button.c` : **0** (20 avant) ; `grep -rn 'offsetof(pico8_js_t' src/` : **vide** ; `grep -n 'stddef' src/pico8_ui_button.c` : **vide** (H19).

**Lecteur, écrivain, aides** :

- `grep -nE '(->|\.)(M)\.(kind|as)' src/pico8_ui_button.c` : **vide** ;
- `grep -n 'pico8_value_t \*' src/pico8_ui_button.c | grep -v 'pico8_value_t \*out_value)' | grep -v 'pico8_value_t \*out)'` : **vide** (4 lignes avant : `pico8_js_member`, `member` du lecteur, la boucle nil, `member` de la boucle de clés) ;
- `grep -c 'off its census row' src/pico8_ui_button.c` : **12** (2 avant : `fq_take_bool`, `fq_take_function`) — les 11 aides et l'`assert` de `jb` = `false` de `pico8_js_pq` ;
- `grep -nw 'fq_take_bool\|fq_take_function' src/` : **vide** ; `grep -n 'button_take_' src/pico8_ui_button.c` : 11 définitions, chacune appelée au moins une fois ;
- `grep -n 'has_h_\|has_my\|has_m_\|has_fg\|has_trade\|has_en\|has__jo_fe_od' src/pico8_ui_button.h` : chacun juste après son membre dans `pico8_js_t` (plus `has_m_` de `fq`), plus `pico8_js_h_` et `pico8_js_my` ;
- `git diff -U0 src/pico8_ui_button.h | grep '^+#include'` : `pico8_geom.h`, `pico8_layout.h`, `pico8_planet.h`, `pico8_research.h`, rien d'autre ; `git diff -U0 src/pico8_ui_button.c | grep '^+#include'` : **vide** ;
- `grep -rn 'pico8_js_pq' src/ tests/` : la déclaration, la définition et l'appel du constructeur ;
- `grep -c 'TROU-23' src/pico8_ui_button.c` : **1**, sur la boucle de clés.

**Périmètre du diff** :

- `git diff --stat HEAD -- src/ ':!src/pico8_ui_button.h' ':!src/pico8_ui_button.c' tools/ Makefile` : **vide** ;
- dans `tests/`, seul `test_pico8_js.c` change, **par ajouts** (`git diff -U0 tests/test_pico8_js.c | grep -c '^-[^-]'` : **0**).

### Empreinte et découpage

Ligne : 38 (15, 23). **Remesurée pour ce PRD** avec la formule du plan : **39 (14, 25)** — `src/` : `economy_score.c`, `entity.c`, `entity_geom.c`, `galaxy.c`, `menu.c`, `msgbox.c`, `particle.c`, `planet_display.c`, `probe.c`, `score_panel_display.c`, `slipway_display.c`, `ui.c`, `ui_button.c`, `wormhole_display.c` ; `tests/` : `bg`, `built_planet_panel_nil_row`, `empty_planet_cleared`, `entity_kind`, `fq`, `fr`, `fv`, `g_`, `ga`, `ga_export_panel_by`, `hp`, `ie`, `jc`, `je`, `jo`, `js`, `mh`, `mo`, `nf`, `pause_menu_music`, `pj`, `planet_gh_row_lq`, `qb`, `rich_planet_exports`, `ui`. Majorant : un seul fichier de `src/` accède directement (`ui_button.c` ; `msgbox.c`, `score_panel_display.c` n'y entrent que par les homonymes `fa`/`ra` de `mh`/`iv`/`pj`), le reste par des noms courts partagés. **Fichiers modifiés attendus : 3** — `src/pico8_ui_button.h`, `src/pico8_ui_button.c`, `tests/test_pico8_js.c` — plus `docs/plan-3.md`. Empreinte réelle comptée au commit (H22).

**Découpage (G) : aucun, un seul commit.** Évalué sur deux découpes :

- (a) **1b.34a** « ce que `pq` construit et ce qu'on écrit par nom » (`lq`, `mb`, `kq`, `h_`, `jb`, `m_` : locales, `pq`, écrivain ; 62 → 56) puis **1b.34b** « clés recopiées » (`fa`, `ra`, `qf`, `my`, `hb`, `fg`, `px`, `trade`, `en`, `_cr_t`, `_jo_fe_*` ; 56 → 43) ;
- (b) indicateurs d'abord (les 7 membres à `has_`), pointeurs ensuite.

Écartées parce que : (1) les deux moitiés touchent les **mêmes** trois fonctions (`g_js_members` et `pico8_js_member`, le lecteur, la boucle nil et la boucle de clés de `pico8_js_struct_new`) : la table ne tomberait qu'au second commit, et la boucle de clés passerait deux fois par une forme mixte (branches typées puis repli `pico8_js_member`) qu'aucun commit final ne garde ; (2) **le coût de test double au lieu de baisser** : chaque commit change le constructeur, donc chacun doit passer `make e2e`, `make sanitize-net` et `make class-coverage-net` (environ 12–15 min à eux trois), et les preuves §6 se répartissent mal — les lecteurs par nom de `lq` (R-a, R-c–R-e) tombent dans (a), celui de `my` (R-b) et les chemins `_jo_fe_*`/`_cr_t` (R-f, R-g) dans (b), mais `playability` et `tech_screen` passent par `h_`/`jb` **et** par `my`, donc seraient prouvés deux fois ; (3) le travail propre à l'étape est peu coûteux : `test_pico8_js` tourne en ~3 s, un mutant ne recompile que `ui_button.c`, et le diff touche 3 fichiers (1b.32 a typé 23 membres dans 6 fichiers en un commit, dans le même budget) ; (4) la règle de découpage du plan (« une ligne de plus de ~40 fichiers est découpée dans la liste elle-même ») ne s'applique pas à 38/39, et la découpe relève de la liste, pas du PRD, sauf proposition justifiée (1b.23a). Si le budget est dépassé, l'implémenteur suit l'ordre de priorité de « Vérification » et note ce qui n'a pas été lancé ; ce n'est pas une raison de découper après coup.

### Hypothèses (rien n'a été lancé)

- **H1** : 1b.33 commitée avant le départ ; `make value-members` rend 62 au départ (aucune ligne `pico8_fv_t`, 19 lignes `pico8_js_t`), 43 à la fin.
- **H2** : 1b.33 ne touche que `help.[ch]`, `test_pico8_fv.c` et le plan : aucun fichier de cette étape.
- **H3** : aucun site n'écrit `true` dans `my` ni dans `h_` d'un `js` construit (cart, C, TU) ; `true` n'existe que dans la locale `h__arg`/`jb_arg` de TROU-23. Démentie : arrêt « champ sans règle », sans élargir le membre.
- **H4** : dans les 15 rejeux et les TU hors fils de `must_abort`, les clés reçoivent : `fa` un fixe ; `ra`/`qf` une REF de vecteur (tag `UNKNOWN`) ; `my` une fonction non NULL ou `false` ; `hb` une fonction non NULL ; `fg`/`trade`/`en`/`_jo_fe_od` un fixe ; `px` une chaîne non NULL ; `_cr_t` une `ob_table` ; `_jo_fe_pt` un `jo` ; `_jo_fe_b` un `hp` ; et l'écrivain : `m_` un booléen ou nil, `lq` une rangée ou nil.
- **H5** : `pico8_jo_fe` n'est atteint que sur un `jo` exact (preuve ci-dessus) ; la caractérisation le constate sur les lignes de `jo:fe` du rejeu `BUILT`. Démentie (une fille atteint `fe`) : arrêt, pas d'élargissement silencieux.
- **H6** : aucune entrée de `pico8_nb()` n'a `oy` = `false` (absent ou `ob_table`), et `pico8_hp_jb` rend une `ob_table` ou NULL : la clé `jb` n'est jamais `false`. La caractérisation le vérifie sur **toutes** les entrées de `nb`, pas seulement celles d'un rejeu. Démentie : arrêt.
- **H7** : tout fichier qui inclut `ui_button.h` (`cart.h`, `economy_score.h`, `main.c`, `main_class_coverage.c`, `menu.c`, `msgbox.c`, `score_panel_display.c`) compile avec les quatre inclusions ajoutées (pas de cycle : `planet.h` n'inclut ni `ui_button.h` ni ceux qui l'incluent ; pas de collision de nom) ; tout binaire qui lie `ui_button.c` lie `planet.c` et `research.c` (fait : `ENGINE_SRCS`, `DATA_ENGINE_SRCS`, `main_raylib`).
- **H8** : le build `-DPICO8_MEMBER_CENSUS` compile avec un `pico8_js_t` sans `pico8_value_t` ; `-fsyntax-only -DPICO8_MEMBER_CENSUS` sur `ui_button.c` et `test_pico8_js.c` suffit si le build complet n'est pas lancé (précédents 1b.29–1b.33).
- **H9** : `test_pico8_js` reste sous 30 s avec deux fonctions de plus (il tourne en ~3 s ; la caractérisation rejoue au plus `BUILT` jusqu'à la frame 277, `TECH` jusqu'à 45 et `GAME_END`, comme `test_fe`/`test_gh`/`test_cr_my`/`test_pj`). Démentie : on allège, on ne déplace pas le TU, le `Makefile` ne change pas.
- **H10** : `pico8_ob_table_set_name(b, "<m>", pico8_value_ref(NULL))` garde la clé (H11 de 1b.29), donc chaque cas « clé nil » atteint son aide.
- **H11** : sur l'ancien code, chaque cas du test d'arrêt se termine sans `SIGABRT` (l'écrivain et la boucle de clés recopient tout ; `pq` garde `false` dans `jb`) ; un cas qui avorte déjà est retiré du « rouge d'abord » et noté.
- **H12** : valeurs **observées** sur un `pico8_js_new(NULL)` : `fa` 11, `mb` = `lq.w` + 2, `kq` une boîte, `lq` une rangée vide, les 15 autres absents ; sur le bouton de difficulté : `px` au pointeur de la chaîne de `bo[2]`, `trade` 6, `en` -0.6, `my` = `pico8_eu_field_value()`, `fa` = `ca` du `fv` ; sur les boutons d'un même `pz` : `ra` au même pointeur.
- **H13** : les preuves §6 rougissent : 8361468 (R-a), `pz` avant les `my` (R-b), 3b8ba2c (R-c), 9183fee (R-d), 50f91b4 (R-e), double `pq` de `jo` (R-f), `_cr_t` non posé (R-g) ; pour R-h, 2d7021f.
- **H14** : `make sanitize-net` propre (aucune allocation nouvelle ; les locales sont empruntées le temps de `pq`, comme les membres l'étaient).
- **H15** : aucun test de régression ne dépend de la représentation des 19 membres ni de `sizeof(pico8_js_t)`.
- **H16** : revue des TU lents confirmée par l'implémenteur (grep ci-dessus) : aucune lecture nil par nom d'un non-pointeur de `js` hors des 7 indicateurs.
- **H17** : `pico8_js_pq` n'a aucun appelant hors du constructeur (fait) : son changement de signature compile partout.
- **H18** : renommer `fq_take_bool`/`fq_take_function` et élargir leurs messages ne change aucun comportement de `fq` ; `test_pico8_fq` ne compare pas de message (`must_abort` sur le signal seul).
- **H19** : `-Werror` muet : `<stddef.h>` et les `size_t i` retirés (`size_t`, `NULL` viennent de `<stdlib.h>`/`<string.h>`) ; `pico8_js_truthy` et `pico8_js_is_lq` encore appelées ; les 11 aides appelées ; `pico8_js_h_`/`pico8_js_my` déclarées. Sinon `<stddef.h>` reste et c'est noté.
- **H20** : aucune valeur figée ne bouge : les 19 hash de `test_pico8_js` (`TITLE` … `DRAW_HASH`), ceux de `test_pico8_lq`, `test_pico8_ui`, `test_pico8_fv`, `test_pico8_pj`, `test_pico8_fq`, et les 15 empreintes du filet.
- **H21** : `make class-coverage-net` identique à la référence (le compte d'instanciations de `js` ne change pas).
- **H22** : empreinte réelle comptée au commit.
- **H23** : retirer l'`assert` de tag de `jb` de `pico8_js_ev` ne change rien (`jb` n'a d'autre source que `pico8_jb_new`).
- **H24** : `float-audit`, `constants-check`, `float-grep`, `float-guard` inchangés (`pico8_ui_to_int` est de classe D, son extrait `x` ne bouge pas ; aucun `PICO8_FIXED_LIT` touché).
- **H25** : le crochet `PICO8_MEMBER_CENSUS_SEE` gardé, la table régénérée perdrait exactement les 19 lignes `pico8_js_t` (non lancé, R8).

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Les nouveaux tests n'accèdent à aucun des 19 membres directement : ils compilent sur les deux formes.
- **(G) Où : `test_pico8_js`**, TU de la classe, rapide, dans `CENSUS_UNIT_TESTS`, lié par `ENGINE_SRCS` ; il a `start_from_title`, `start_game`, `run_to`, `bo_row`, `dr_row`, `ix_buttons`, `ok_le`, `build`, `lookup`, `has`, `get`, `get_ref`, `bool_state`, `is_lq`, `fold_value`, `fold_vec`, `fn_index`, `call`, `nil`, `check`, `must_abort`, `in_fresh_child`, et inclut déjà `layout.h`, `research.h`, `data_init.h`, `menu.h`, `ui.h` (précédent de lieu : `test_ha_pz_members_by_name`, 1b.4).
- **Caractérisation dirigée `test_js_members_by_name`**, par `in_fresh_child` après `test_js_is_a_struct`, hash **`JS_MEMBERS_HASH`**. Lecture par le **lecteur brut** (`pico8_js_struct_class()->field`, témoin `pico8_value_fixed(0x1234)`) **et** par `pico8_entity_get_field`, qui doivent concorder (retour, genre, valeur, pointeur) ; adresses jugées par `assert`, jamais pliées.
  1. **Neufs, par producteur** : `pico8_js_new(NULL)`, une table vide, `pico8_js_new(bo_row(2))` puis `eu`, les 4 boutons du titre (`pz` du `fv`), les 4 `ix`, les 4 lignes `ha` (après `ix[4].my`), les lignes de `cr` (`run_to(TECH, 45)`), de `jo:fe` (`run_to(BUILT, KNOWN_PANEL_FRAME)`), de `jo:gh` (`run_to(BUILT, BUILT_PANEL_FRAME)`), les deux boutons de `pj:ph` (`GAME_END`). Les 19 noms pliés (présence, genre, valeur ; rangées par `is_lq` et `w`). `assert` : `ra` d'un même `pz` au même pointeur, `qf` distincts ; `px` du bouton de difficulté au pointeur de la chaîne de `bo_row(2)` ; `my` = `pico8_eu_field_value().as.function` sur lui ; `_jo_fe_pt` = la planète cliquée, de classe `jo` exacte (H5) ; `_jo_fe_b` de classe `hp` ; `_cr_t` = une entrée de `pico8_nb()` au même pointeur ; au moins une ligne de `cr` à `my` = `BOOL` 0 présent et une à `FUNCTION` (sinon l'implémenteur prend un état de jeu où une tech est trop chère et le note) ; `h_` = `BOOL` 0 présent sur une ligne gratuite de `jo:fe`, rangée sinon ; `hb` présent seulement sur `ix[2]` (H12).
  2. **Toutes les entrées de `pico8_nb()`** : `oy` absent ou `ob_table` (H6).
  3. **Écrivain par nom**, sur un bouton `ix` et sur un `pico8_js_new(NULL)`, par `pico8_entity_set_field` :

     | nom | écritures | lecture attendue |
     |---|---|---|
     | `m_` | `true` ; `false` ; **`false` à union nulle** ; `REF NULL` après `true` ; après `false` ; sur nil | `BOOL` 1 ; `BOOL` 0 présent ; `BOOL` 0 présent ; absent, témoin intact |
     | `lq` | la rangée d'un autre bouton ; une rangée neuve (`pico8_lq_new`) ; `REF NULL` | même pointeur ; même pointeur ; absent, témoin intact (`mb`, `kq` inchangés) |

     `cls->set_field` rend 1 pour `lq`, `m_` ; il rend 0, sans rien toucher (valeur relue identique), pour les 17 autres noms (écriture d'un fixe 9) et pour `""`, `"M_"`, `"m_ "`, `"lq "`, `"has_m_"`, `"has_h_"`.
  4. **Clés** (par `build`/`pico8_js_new` sur une table) : chaque clé `fg`, `trade`, `en`, `_jo_fe_od`, `px`, `ra`, `qf`, `hb`, `my`, `_cr_t`, `_jo_fe_pt`, `_jo_fe_b` à `REF NULL` → absent (H10) ; `fg`/`trade`/`en`/`_jo_fe_od` à **fixe 0 à union nulle** → `FIXED` 0 présent ; `my` = `false` et **`false` à union nulle** → `BOOL` 0 présent ; `h_` = `false` → `BOOL` 0 présent ; `h_`, `jb`, `lq` à `REF NULL` → rangée / `jb` / rangée (TROU-23) ; `h_` = une rangée existante → rangée neuve distincte ; `lq` = une liste → rangée (chemin 8361468).
  5. **`kj`/`ev`/`ldown`/`ph`** : déjà figés par `test_draw`, `test_ldown`, `test_ph`, `test_hb`, `DRAW_HASH` et les rejeux ; la caractérisation ajoute seulement `kj` d'un bouton à `my` = `false` (couleur 5, comme sans `my`) comparé à un bouton sans `my`, par `draw_alone`.
  6. **Intrus** sur le lecteur : `""`, `"l"`, `"lq "`, `"LQ"`, `"h__"`, `"has_h_"`, `"has_my"`, `"_jo_fe"`, `"tradex"`, `"en_"` → 0, témoin intact.
  7. Tout plié dans **`JS_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ (valeur imprimée par `check`), figé ensuite.
- **Test d'arrêt `test_js_writer_stops_off_row`**, rouge d'abord, par `in_fresh_child` après la caractérisation ; `must_abort` existant, valeur hors ligne dans une globale ; cas à union nulle par `memset` puis `kind` :

  | cible | nom | valeurs hors genre |
  |---|---|---|
  | un bouton `ix`, écrivain | `m_` | fixe 1, **fixe 0 union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table`, REF vecteur, REF `pico8_hv()` (liste), REF d'un autre `js` |
  | idem | `lq` | fixe 1, **fixe 0 union nulle**, `true`, **`false` union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table`, REF liste, REF vecteur, REF d'un `jb` (`pico8_jb_new(NULL)`), REF d'un autre `js` |
  | construction | `fa` | **nil** (H10), `true`, **`false` union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table` |
  | construction | `ra`, `qf` | fixe 1, **fixe 0 union nulle**, `true`, **`false` union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table`, REF liste, REF d'un `js` |
  | construction | `my` | **`true`**, fixe 1, **fixe 0 union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table` |
  | construction | `hb` | `true`, `false`, **fixe 0 union nulle**, `"x"`, **fonction NULL**, REF `ob_table` |
  | construction | `fg`, `trade`, `en`, `_jo_fe_od` | `true`, **`false` union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table` |
  | construction | `px` | fixe 1, **fixe 0 union nulle**, `true`, **`false` union nulle**, **chaîne NULL**, **fonction NULL**, REF `ob_table` |
  | construction | `_cr_t` | fixe 1, **fixe 0 union nulle**, `true`, **`false` union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF liste, REF vecteur, REF d'un `js` |
  | construction | `_jo_fe_pt` | fixe 1, **`false` union nulle**, `"x"`, REF `ob_table`, REF vecteur, REF liste, **REF d'un `qh` neuf**, REF d'un `hp` de `gb`, REF d'un `js` |
  | construction | `_jo_fe_b` | fixe 1, **`false` union nulle**, `"x"`, REF `ob_table`, REF vecteur, REF d'un `jo` neuf, REF d'un `js` |
  | construction | `jb` | **`false`**, **`false` union nulle** |

  **Exclus** (dans le genre, ou validés par `pq` comme avant) : nil, `false`, `false` union nulle pour `m_` et `my` ; `true` pour `m_` ; fixe 0 union nulle en clé `fa` ; toute valeur des clés `lq` et `h_` (leur `assert` de `pico8_js_lq_arg_table` est celui de 1a, figé par `test_pico8_lq` `button_on_other`) ; clé `jb` autre que `false`. Rouge sur l'ancien code (H11), vert après : une ligne « Runs » pour chacun. Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_js && ./tmp/test_pico8_js` sauf mention, notées dans le commit avec leur test rouge ou la justification du trou ; un appel unique d'aide `static` se retire sous `if (0)` (1b.14b) :
  1. **(§7-1) indicateur forcé vrai** : le lecteur ignore `has_fg` (nil lu `FIXED` 0 présent) → rouge : caractérisation 1, `fold_button` (`TITLE_HASH`, `EU_HASH`…) ; `gw` y est insensible (nil et 0 pokent 0), noté. **(1b)** `pico8_js_h_` ignore `has_h_` → rouge : `test_nil_keys` (`!has(no_keys, "h_")`), caractérisation 1. **(1c)** `pico8_js_my` ignore `has_my` → rouge : `test_js_is_a_struct` (`!has(bare, "my")`), caractérisation 1.
  2. **(§7-2) « présent 0 » là où c'était absent** : la branche `trade` rend `pico8_value_fixed(0)` quand `has_trade` = 0 → rouge : caractérisation 1, `fold_button`.
  3. **(§7-3) init qui saute un membre typé** — **sans objet dans sa lettre** (plus aucune boucle de recopie) ; forme « pose une autre valeur » (précédents 1b.14a, 1b.24, 1b.32, 1b.33) : (a) R3 `has_m_ = 1` → rouge : caractérisation 1 ; (b) R3 `fa` = 12 → rouge : `test_js_is_a_struct` (`fa` 11), `EU_HASH` ; (c) branche de clé `px` sous `if (0)` → rouge : `test_dk` (`DK_HASH`), caractérisation 1 ; (d) ligne TROU-23 retirée (la locale garde nil) → rouge : `test_nil_keys` ; (e) branche `false` de `h_` dans `pq` sans `has_h_ = 1` → rouge : `test_draw` (`bool_state(free_, "h_") == 1`), `FE_HASH`.
  4. *Écrivain de `lq` muet* (`if (0) button_take_lq(…)`, `return 1` gardé) → rouge : `test_js_is_a_struct` (même pointeur), `test_hb` (`HB_HASH`).
  5. *Écrivain de `m_` muet* → rouge : `test_draw` (`px[2] != px[1]`), caractérisation 3.
  6. *Nil jugé sur le pointeur seul* (`value.as.ref == NULL` sans le genre) dans `button_take_bool`, puis dans `button_take_vec` → rouge : caractérisation 3 (`false` union nulle lu absent), test d'arrêt (union nulle).
  7. *Tag non vérifié* (`UNKNOWN` de `button_take_vec`, `OB_TABLE` de `button_take_table`, `pico8_js_is_lq` de `button_take_lq` retirés un à un) → rouge : test d'arrêt.
  8. *`true` accepté pour `my`* → rouge : test d'arrêt.
  9. *Fonction NULL acceptée* (`button_take_my`, puis `button_take_function`) → rouge : test d'arrêt.
  10. *`button_take_jo` sans test de `cls`* → rouge : test d'arrêt (`qh`, `hp`, `js`).
  11. *Clé `jb` = `false` acceptée* (l'`assert` de `pq` retiré, `jb` NULL) → rouge : test d'arrêt.
  12. *Clé `fa` nil acceptée* (`button_take_fa` pose 0 sur `REF NULL`) → rouge : test d'arrêt.
  13. *R6 : `kj` prend `has_my` pour `my != NULL`* (`false` présent dessiné cliquable) → rouge : caractérisation 5, `test_replay_tech` (`TECH_REPLAY_HASH`) noté.
  14. *Locale `lq_arg` jamais passée* (`pico8_js_pq(self, pico8_value_ref(NULL), …)`) → rouge : `fold_button` (rangées vides), `test_pause`, caractérisation 4 ; **`make unit-run-test_pico8_planet_gh_row_lq` noté** (ses `assert` de champ).
  - **Mutants équivalents, notés** : `m_flag = self->m_` sans `has_m_` dans `kj`/`ev` (`m_` vaut 0 quand nil) ; `self->h_ != NULL` remplacé par `has_h_ && h_ != NULL` dans `kj` ; `*has = 1` avant l'`assert` d'une aide (fils qui avorte, état jamais relu) ; `ldown` qui prend `has_my` (seul `rn(60)` contre `rn(false)` change : l'audio est hors périmètre, TROU-10).
- **`gcov`** : `test_pico8_js` en `--coverage -O0`, piège `__gcov_dump` des fils (1a.23), sur `ui_button.c`. Lignes attendues exécutées : poses R3, les 19 branches R4 (trois états de `h_` et de `my`, deux de `m_`, `has_` = 0 et 1 des quatre fixes, branches NULL des pointeurs sauf `kq`), les deux branches d'écrivain, les 16 branches de clés et l'`else`, les trois branches de `h_` et de `jb` de `pq`, les 11 aides (hors échec d'`assert`), `pico8_js_h_`, `pico8_js_my`, les lectures R6. **Trous attendus** : `return 0` de `kq` NULL (jamais NULL après `pq`) ; les gardes `self == NULL` existantes ; les échecs d'`assert` (fils de `must_abort`, prouvés par le test d'arrêt et les mutations 6–12). Rapport dans le commit.
- **Prior art** : `test_ha_pz_members_by_name` de `test_pico8_js` (1b.4) ; `test_g__members_by_name`/`test_g__writer_stops_off_row` de `test_pico8_g_` (1b.32, `et` {false, REF}) ; `test_fq_*` de `test_pico8_fq` (1b.18, fonction NULL en clé, `fq_take_bool`) ; `test_pico8_fv` de 1b.33 (`help_take_bool`, `g_off_row`) ; `family_take_target`, `jo_take_ma`, `bg_take_vec`, `ie_take_kind`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer.

- `./tmp/test_pico8_js` : ~3 s (H9) ; environ 24 lancements (départ, arrêt rouge, final, 20 mutations, `gcov`) ≈ 4–6 min builds compris. Si un lancement dépasse 20 s, on joue d'abord 1, 1b, 1c, 3d, 3e, 4, 5, 6, 8, 11, 14, et on note les autres non lancées.
- §6 R-a à R-g : deux lancements chacun, quelques secondes par rejeu (R-a par sa cible ASan) ; R-h dans le budget seulement.
- `make test` une fois ; `make e2e` une fois, **dans la même invocation** que `make class-coverage-net` ; `make sanitize-net` une fois, jamais en même temps que `e2e` ; `make value-members`.
- Ordre de priorité si le budget manque : cibles finales 1–5 ; départ, arrêt rouge, final ; R-a à R-g ; mutations 1, 3, 6, 8, 11, 14 ; le reste noté « non lancé ».
- Aucun TU lent n'est modifié ; compiler tôt sans lancer (`make -j tmp/test_pico8_hj tmp/test_pico8_qh tmp/test_pico8_jo`) : `make e2e` les lance.

| # | Commande exacte | État de l'arbre (départ / final / mutation n / §6 R-x bug remis / §6 R-x restauré) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` vert, les **8** TU de `UNIT_TESTS_SLOW` verts ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **43**, aucune ligne `pico8_js_t` ;
5. `timeout 15m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` — requis, `pico8_js_struct_new` change (sans toucher `pico8_class_coverage_record` ni `KNOWN[]`).

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements (les fixtures de `test_pico8_js` et des tests §6 sont seulement **lues**).

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet**

- `FAILED:` si 1b.33 n'est pas commitée au départ, ou si `make value-members` ne rend pas 62 au départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, ou si le diff touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements.

**Types, tables, écrivain**

- `FAILED:` (« champ sans règle ») si une aide ou l'`assert` de `jb` de `pq` avorte dans un rejeu ou un TU hors des fils de `must_abort` (H3–H6), ou si un type est élargi au lieu d'avorter : `true` accepté pour `my` ou `h_`, `<m>_is_bool` ajouté, `jb` = `false` gardé, cible étendue hors de `jo`/`hp`/`lq`, fonction NULL ou chaîne NULL acceptée, clé `fa` nil acceptée, `pico8_value_t` gardé dans `pico8_js_t`.
- `FAILED:` si l'un des 19 membres n'a pas le type retenu à sa place, si l'un des 7 indicateurs manque ou n'est pas juste après son membre, ou si un autre `has_` est ajouté (`has_lq`, `has_jb`, `has_fa`…).
- `FAILED:` si nil, `false` et la valeur de `h_`, `my` ou `m_` ne se lisent plus par nom exactement comme en 1a (absent ; `BOOL` 0 présent ; rangée / fonction / `BOOL` 1).
- `FAILED:` si `g_js_members`, `pico8_js_member`, un `offsetof` ou une écriture `*(pico8_value_t *)` existe encore dans `ui_button.c`.
- `FAILED:` si une clé `lq`, `h_` ou `jb` est rangée ailleurs que dans une locale passée à `pico8_js_pq`, si TROU-23 quitte la boucle de clés ou change de comportement, ou si `pico8_js_lq_arg_table`/`pico8_js_lq` change.
- `FAILED:` si l'écrivain de `js` n'accepte plus exactement `lq` et `m_`, si la boucle de clés n'accepte plus exactement `fa`, `lq`, `h_`, `jb`, `ra`, `qf`, `my`, `hb`, `fg`, `px`, `trade`, `en`, `_cr_t`, `_jo_fe_pt`, `_jo_fe_b`, `_jo_fe_od`, ou si le message de l'`assert(0 && …)` final change.
- `FAILED:` si une aide viole la forme de 1b.13/1b.15 (état nil d'abord, sauf `button_take_fa` ; genre **et** pointeur testés ; union nulle hors genre qui avorte ; message qui nomme `js.<m>` (et `fq.<m>` pour les deux aides partagées) et le genre ; `has_` posé en dernier ; tag vérifié avant `cls`), si elle est rendue publique ou partagée avec un autre fichier.
- `FAILED:` si `fq` change autrement que par le renommage de ses deux aides et l'élargissement de leurs messages.
- `FAILED:` si `ui_button.h` gagne une autre inclusion que `pico8_geom.h`, `pico8_layout.h`, `pico8_planet.h`, `pico8_research.h`, ou si `ui_button.c` en gagne une.

**Diff et preuves**

- `FAILED:` si un site par nom change (`gw`, `gk`, `ix2_hb`, `cr_row_my`, `jo_fe_my`, `ldown`/`mv`, `economy_score`, `galaxy`, `score_panel_display`…), ou si un site par nom est remplacé par un accès direct.
- `FAILED:` si un autre fichier de `src/` que `ui_button.h`, `ui_button.c` change, ou `tools/`, le `Makefile`, `CENSUS_UNIT_TESTS`, `UNIT_TESTS_SLOW`.
- `FAILED:` si un autre test que `test_pico8_js.c` change, si son diff retire ou modifie une ligne existante, ou si une valeur figée existante change (H20).
- `FAILED:` si un test de régression (§6) a un diff, ou s'il manque les deux lignes « Runs » (rouge bug remis, vert restauré) de R-a à R-g ; pour R-h, soit ses deux lignes, soit « non lancé » au journal.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas l'attendu, si `make sanitize-net` n'est pas `15 propre(s)`, si `make value-members` ne rend pas 43 ou si un autre typedef gagne ou perd une ligne, ou s'il manque la ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un crochet du recensement change, ou si `tools/member_census.txt` est édité à la main.

**Tests de l'étape**

- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, lit ou écrit un des 19 membres directement, ne couvre pas les producteurs listés, les 19 noms, les trois états de `h_` et de `my`, l'identité de `ra`/`px`/`_cr_t`/`_jo_fe_pt`, les clés à nil et à union nulle, TROU-23, les entrées de `nb`, l'écrivain (`m_` et `lq`), les noms sans branche et les intrus ; ou si `JS_MEMBERS_HASH` change après.
- `FAILED:` si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'arbre de départ et une verte sur l'arbre final, ou s'il n'a pas les cas union nulle `memset`, `true` pour `my`, fonction NULL, chaîne NULL, nil en clé `fa`, les structs refusées pour `_jo_fe_pt`/`_jo_fe_b`/`lq`, et `jb` = `false`.
- `FAILED:` si le commit ne porte pas le rapport `gcov` et les mutations 1 à 14 (avec 1b, 1c, 3a–3e), chacune avec son test ou la justification de son trou, plus les mutants équivalents.

**Commit et note**

- `FAILED:` si le travail n'est pas en un commit vert.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N` (TROU-23 et TROU-7 gardés à l'identique).
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, ou s'il manque le tableau « Runs » ou la note « Découvert à 1b.34 ».

## Out of Scope

- Les homonymes : `fq.fa`/`my`/`m_` (1b.18, faits, hors le renommage de deux aides), `ie.jb`/`m_`/`kq`/`ra` (1b.20), `lq.*` (1b.10), `jb.ls` (1b.2), `jo.lq`/`kq`/`ra`/`px`/`m_`/`fa` (1b.23), `ha_pz.*` (1b.4), `bg.m_` (1b.15), `fv.m_`/`kq`/`fg` (1b.33), `mh.fa`, `iv.ra`, `pj.fa`, `ui.o_`/`gj` (1b.37, qui pointent vers `js`).
- `pico8_js_lq_arg_table` et `pico8_js_lq` (validation des arguments de `pq`, inchangée) ; `pico8_js_default` et la spec `fa=11`.
- Corriger TROU-23 (clé nil qui construit une rangée ou un `jb` vide) ou TROU-7 (`ra`, `qf`, `kq` jamais libérés).
- Les sites par nom (`gw`, `gk`, `mv`, `ix2_hb`, `cr_row_my`, `jo_fe_my`, `economy_score`, `galaxy`, `score_panel_display`).
- Déplacer un TU entre listes ; régénérer `tools/member_census.txt`.
- Fichiers d'événements, `FRAMES_EVENTS`, rendu, audio, appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur (G)** : modules `ui_button.h` (typedef, 7 `has_`, `pico8_js_h_`, `pico8_js_my`, signature de `pico8_js_pq`, 4 inclusions) et `ui_button.c` (lecteur, écrivain, boucle de clés, locales de `pq`, 11 aides `button_take_*` dont 2 renommées, R6, `<stddef.h>`) ; `h_` → `pico8_lq_t *` + `has_h_`, `my` → `pico8_value_function_t` + `has_my` (NULL présent = `false`), `m_` → `int` + `has_m_`, `fg`/`trade`/`en`/`_jo_fe_od` → `pico8_fixed_t` + `has_`, `fa`/`mb` → `pico8_fixed_t`, `lq` → `pico8_lq_t *`, `jb` → `pico8_jb_t *`, `kq` → `pico8_qz_box_t *`, `ra`/`qf` → `pico8_vec_t *`, `hb` → `pico8_value_function_t`, `px` → `const char *`, `_cr_t` → `pico8_ob_table_t *`, `_jo_fe_pt` → `pico8_jo_t *`, `_jo_fe_b` → `pico8_hp_t *` ; tests dans `test_pico8_js` par ajouts ; pas de découpage ; §6 : R-a à R-g obligatoires, R-h dans le budget.
- **Note « Découvert à 1b.34 »** (à écrire sur la ligne ; le reste va dans le commit) :

  > patron 1b.1 appliqué ; `fa`, `mb` → `pico8_fixed_t`, `fg`, `trade`, `en`, `_jo_fe_od` → `pico8_fixed_t` + `has_`, `m_` → `int` + `has_m_` (1b.4b), `lq` → `pico8_lq_t *`, `jb` → `pico8_jb_t *`, `kq` → `pico8_qz_box_t *`, `ra`, `qf` → `pico8_vec_t *`, `hb` → `pico8_value_function_t`, `px` → `const char *`, `_cr_t` → `pico8_ob_table_t *`, `_jo_fe_pt` → `pico8_jo_t *` (seul un `jo` atteint `fe`), `_jo_fe_b` → `pico8_hp_t *` ; **`h_` {false, `ref:pico8_lq_t`} et `my` {false, fonction} : lecture de 1b.7/1b.24 sur une cible unique** (pointeur typé + `has_`, NULL présent = `false`, `true` avorte, `pico8_js_h_`, `pico8_js_my`) — ferme pour `js.my` la question de 1b.7 ; **lecture nouvelle de R5 : une clé que `pq` reconstruit dans un autre genre (`lq`, `h_`, `jb`) n'est pas le membre — locale `pico8_value_t` du constructeur, TROU-23 appliqué à la locale, passée à `pico8_js_pq(self, lq, h_, jb)`, validée là où `pq` la validait ; le membre ne reçoit que ce que `pq` construit ; clé `jb` = `false` avorte** ; lecture de 1b.23b sur un fichier à deux structs sans parenté : aides `button_take_*` au préfixe du fichier, `fq_take_bool`/`fq_take_function` renommées, messages élargis ; **`g_js_members` et `pico8_js_member` supprimées, plus aucun `offsetof` ni `<stddef.h>` dans `ui_button.c`** ; `pico8_jb_struct_class()` sans client (`jb` n'a ni écrivain ni clé typée) ; `assert` de tag de `jb` de `ev` retiré (1b.20) ; §6 : les cinq lecteurs par nom de 1a.16 (`planet_gh_row_lq`, `pause_menu_music`, `built_planet_panel_nil_row`, `ga_export_panel_by`, `rich_planet_exports`) et `playability`, `tech_screen` sur le chemin, prouvés rouges sans diff ; empreinte réelle mesurée au commit.
- Le message de commit porte : compte 62 → 43, résumé « Runs », `gcov`, mutations et trous, `JS_MEMBERS_HASH`, réponses à H1–H25.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
