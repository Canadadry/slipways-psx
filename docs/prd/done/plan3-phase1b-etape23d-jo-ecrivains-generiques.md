---
title: "Plan 3, étape 1b.23d : `jo`, membres des écrivains génériques — `kr`, `cc`, `by`, `mi`, `m_`, `fo`, `ma` typés, `fo` à dix cibles par pointeur d'en-tête, `g_jo_members` et `g_jo_defaults` supprimées"
description: "`pico8_jo_t` garde 7 membres `pico8_value_t` après 1b.23c, tous écrits par un écrivain générique (`hy:lj`, `bd`, `bb`, `gw`, `ma()`, `bv`, `jc:hq`). 1b.23d les type selon le patron R1–R9 de 1b.1 : deux listes (`cc`, `by`), deux fixes à indicateur (`kr`, `mi`), un booléen à indicateur (`m_`), un fixe jamais nil (`ma`), et `fo`, REF à plusieurs cibles, en `pico8_struct_header_t *` sous la forme de 1b.22 avec un prédicat propre (dix classes : `hq` = `bv`, plus `jc` ; `mz` exclue). Dernière étape de `jo` : ses deux tables d'offsets disparaissent, `pico8_jo_t` ne contient plus aucun `pico8_value_t`. `make value-members` passe de 145 à 138. Un seul commit (17 fichiers)."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.23d — membres des écrivains génériques : `kr`, `cc`, `by`, `mi`, `m_`, `fo` (REF vers 9 typedefs), `ma`. Empreinte 34 (12, 22). Compte : 138.

1b.23d type ces 7 membres, les derniers `pico8_value_t` de `pico8_jo_t`. C'est la **dernière étape de `jo`** : après elle, `pico8_jo_t` ne contient plus de `pico8_value_t`, et `g_jo_members`/`g_jo_defaults` n'ont plus d'entrée.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité, pas redécidé. Les notes « Découvert à » de `docs/plan-3.md` s'appliquent. Elles sont citées ici, pas redérivées :

- **1b.1** : R1 à R9 ; `ref:liste` → `pico8_list_t *`, `fixe` → `pico8_fixed_t`, `bool` → `int` (0/1) ; R2 : `int has_<m>` juste après `<m>` pour un non-pointeur lu par nom à nil `observé`, aucun pour un pointeur (NULL = nil) ; R3 « un défaut REF garde la même REF » ; R5 « même règle dans l'écrivain `set_field` et dans la boucle des clés de construction ».
- **1b.5, 1b.7, 1b.16 — lectures de R1 pour les genres mêlés** : **sans objet**. Chaque ligne de l'étape n'a qu'un **genre scalaire** : `kr`, `mi`, `ma` `fixe`, `m_` `bool`, `cc`, `by` `ref:liste`. `fo` a un seul genre de fond, `ref:` vers des structs, mais plusieurs cibles : c'est 1b.22, pas une lecture de genres mêlés. Ces notes sont citées pour être écartées : aucun `<m>_is_bool`, aucun NULL qui porterait un défaut.
- **1b.9 et 1b.10 — R7** : forme `offsetof` (`members[]`/`defaults[]`), où une table vidée est supprimée avec son seul client, l'accesseur `pico8_value_t *`. Les preuves `grep -rnw 'X_member\|g_X_members' src/` et `offsetof(pico8_X_t` doivent être vides, et `grep -n 'pico8_value_t \*' … | grep -v 'pico8_value_t \*out_value)' | grep -v 'pico8_value_t \*out)'` doit être relue. Messages d'aide qui nomment leurs membres et le genre de leur ligne. **« Reste à fixer : une table qui garde des entrées ou a des filles (`jo`, `g_`) »** : cette étape ferme le cas `jo` (voir « Sortie de `jo` »).
- **1b.11 — R2, TU lents** : `CENSUS_UNIT_TESTS` exclut `UNIT_TESTS_SLOW`. Ce PRD relit donc les TU lents de l'arbre `jo` (revue plus bas).
- **1b.13 et 1b.15 — forme écrivain de R5** :
  - une branche `strcmp` par membre, dans l'ordre du typedef ;
  - une aide `static` par genre, jamais partagée entre fichiers ;
  - l'aide pose **d'abord** l'état nil, sort sur `REF NULL` (genre **et** pointeur), sinon un `assert` « champ sans règle » qui nomme `struct.membre` et le genre ;
  - un non-pointeur **sans** indicateur n'a pas d'état nil : son aide commence par l'`assert` et nil y avorte (point 3 ; précédent `qb_take_fixed`, 1b.19) ;
  - aide à indicateur (1b.15, `bg_take_bool`) : nil éteint `has_` quel que soit l'état d'avant, un `false` à union nulle reste un `false` présent, et `has_ = 1` est posé en dernier ;
  - les cas à union nulle sont construits par `memset`, puis `kind` ;
  - un `must_abort` rouge sur l'ancien code fige l'arrêt.
- **1b.14a — R7 sur une table qui garde des entrées et a des filles** : l'entrée sort de toutes les tables qui la listent, tables des filles comprises. Le module déballe le défaut une fois, à son init, dans un `static` qui garde le même pointeur que la spec. Il le remet à nil dans `defaults_reset`. La pose typée se fait après la boucle, avant les clés et `pq`.
- **1b.22 — REF à plusieurs cibles** : la note est recopiée pour `fo` (voir « `fo` »). Elle fixe trois règles :
  - `ref:` à plusieurs cibles → `pico8_struct_header_t *<m>` (ni union, ni `void *`, ni base `ke`) ; l'ensemble est porté par l'`assert` de l'aide R5 (genre REF, tag `STRUCT`, puis prédicat `static` sur `cls`) ;
  - **Coexistence : l'ensemble admis est la ligne ∪ les classes productrices prouvées par le code et exercées par la caractérisation dirigée sur l'ancien code** ;
  - **prédicat** par liste d'accesseurs publics quand tous sont liés partout où le fichier l'est ; sinon, par une propriété du descripteur que l'ensemble partage exactement.
- **1b.23a1** (note « Découvert à ») : patron `static` de `jo` (déballage à l'init, remise à nil dans `defaults_reset`) ; aides de lecture de spec par fichier (`jo_default_*`, `family_default_*`), dont le message nomme ses membres ; **H11 démentie en partie** : les valeurs figées sont celles **observées**.
- **1b.23a2** (note « Découvert à ») :
  - un membre `l` seul que l'écrivain de 1a accepte déjà garde sa branche. Sans objet ici : les 7 lignes ont `e` ;
  - **boucles sentinelles** : un membre typé sort des tableaux `written[]` des TU, et son écriture par nom est figée par la caractérisation de l'étape ;
  - **H16 démentie** : les greps de §6 doivent couvrir `planet->m` et `->jo.m`, pas seulement `jo\.m`.
- **1b.23b** (note « Découvert à ») : aides d'écriture par fichier, partagées entre l'écrivain et les boucles de clés ; R7 sur une table de clés de fille ; **§6 : le PRD n'en voyait qu'un, il y en avait trois**.
- **1b.23c** (PRD, supposée commitée) :
  - premiers indicateurs de `jo` (`fx`, `jh`, `p_`) et aides `jo_take_list`, `jo_take_bool`, `jo_take_fixed` (à indicateur) dans `planet.c`, **reprises ici** en étendant leur message ;
  - `list_of` et `pico8_jo_present` gardés pour leurs derniers appelants `by`, `kr` : ils tombent ici ;
  - l'`assert` de lecture passé à l'écriture (1b.20).

### État de départ

HEAD `ce2fa89` (PRD de 1b.23c). 1b.23c est dans l'arbre de travail, en cours d'implémentation : 6 fichiers `src/` et 11 TU modifiés, `test_jo_state_by_name`/`JO_STATE_HASH` présents dans `test_pico8_ui.c`.

**Précondition : 1b.23c est commitée avant que 1b.23d commence** (H9). `make value-members` rend alors **145** (H1). Les lignes citées sont celles de l'arbre 1b.23c et glisseront. Chaque site est donc aussi désigné par son contenu.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code (HEAD et arbre 1b.23c en cours). Tout fait d'exécution est une hypothèse (H1 à H22).

### Lignes de recensement (`tools/member_census.txt:62,80,83-87`)

| membre | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|
| `ma` | `fixe` | jamais | - | **le** | `pico8_fixed_t` | non (nil avorte) |
| `fo` | `ref:` `fr`, `g_`, `hj`, `jc`, `mx`, `nl`, `nr`, `os`, `qh` (9 typedefs) | observé | lu,stat? | **le** | `pico8_struct_header_t *` | non (pointeur) |
| `kr` | `fixe` | observé | lu | **le** | `pico8_fixed_t` | **`has_kr`** |
| `cc` | `ref:liste` | observé | lu | **le** | `pico8_list_t *` | non (pointeur) |
| `by` | `ref:liste` | observé | lu,stat | **le** | `pico8_list_t *` | non (pointeur) |
| `mi` | `fixe` | observé | lu | **le** | `pico8_fixed_t` | **`has_mi`** |
| `m_` | `bool` | observé | lu,stat? | **le** | `int` | **`has_m_`** |

Le compte passe à 145 − 7 = **138** (les `has_` ne comptent pas, R9). **Aucune ligne `pico8_jo_t` ne reste.**

- **Les 7 lignes ont `e`.** Chacune est écrite par nom par un écrivain générique, sans filtre de classe (§5 1a, « Champ d'un écrivain générique ») :

| membre | écrivain | fichier | genre écrit |
|---|---|---|---|
| `kr` | `hy:lj` | `economy.c` | `pico8_value_fixed(kr)` sur chaque `cg.jo` |
| `cc` | `bd` (`bd_reset_visit`) | `economy_tiers.c` | liste neuve ; lit l'ancienne par nom et la libère |
| `by` | `bd` (`bd_route_visit`) | `economy_tiers.c` | résultat de `mv(pt, "ib")` (liste) ou nil |
| `mi` | `bb` (`bb_visit`) | `economy_tiers.c` | `pico8_value_fixed(jw)` |
| `m_` | boucle de survol `gw` | `ui.c` | `false` sur `gj`, `true` sur `o_` |
| `ma` | `ma()` | `entity_geom.c` | `pico8_value_fixed(clamped)`, le fixe décrémenté |
| `fo` | `bv` | `probe.c` | `self.et.fo=self` : REF vers le porteur (`self`) |
| `fo` | `jc:hq` | `slipway.c` | REF vers le `jc` |

- **Trois indicateurs.** `kr`, `mi`, `m_` sont des non-pointeurs lus par nom, à nil `observé` avec preuve `lu` : R2 donne `has_<m>` sans débat. La revue des TU lents le confirme : `has(e, "m_")`, `fold_field(e, "mi")`, `!has(b, "kr")`.
- **`ma` sans indicateur.** Il est nil `jamais` : défaut `ma=0` de la spec de `jo`, jamais écrit à nil, et aucune lecture nil par nom dans les TU lents (H4). La forme est celle de `qb.ma` (1b.19) : une aide sans état nil, où nil avorte.
- **`fo` : un seul genre (`ref:` de struct), dix cibles** (voir « `fo` »).

### Revue des TU lents (R2 de 1b.11)

Relus pour ces 7 noms : `test_pico8_jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr` (arbre `jo`) et `test_pico8_ie`. Constats :

- listes `members[]`/`jo_members[]` (les 31 noms) pliées par `fold_field` : présence, genre, valeur ;
- `fold_live` (`jo:335-338`) : `scalars[]` ∋ `ma`, `mi`, `kr` ; `lists[]` ∋ `by`, `cc` (`list_size`) ; `refs[] = {"fo", "nk", "lq", "m_"}` (`has`, présence seule) ;
- **lectures nil par nom** : `has(e, "m_")` dans les plis de `qh:506`, `os:525`, `hj:601`, `mx:629`, `nr:551` et `nl:485` (rapide) ; `fold_field(e, "mi")` (présence) ; `!has(b, "kr")` (`ga:561`, rapide) ;
- **écritures par nom dans le genre** : `mi` ← fixe (`hj:526`, `pm:527`, `mx:519`, `nl:452-455`, `os:458` recopié d'un autre `qh`), `by` ← liste (`os:446`) ;
- **identité** : `get_ref(e, "by") == class_ref(pico8_ga_default, "by")` (`ga:380`) et le même `by` partagé entre deux `ga` (`ga:550-551`), tous deux rapides — **d'où le pointeur de spec pour `by` de `ga`** (plus bas) ; `on_jo[] ∋ "by"` de `test_ga_is_a_struct` (`pico8_value_equal` avec le défaut de spec) ;
- `inherited[] ∋ "mi"` (`qh:549`) : `mi` des filles égal par valeur à celui de `qh` ;
- `test_pico8_ie` : ses `m_` sont ceux d'`ie`, des homonymes (1b.20). Sans objet ;
- **les boucles sentinelles `written[]`** de `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr` (7 lents) et de `nl`, `fr`, `ga` (3 rapides) écrivent `pico8_value_fixed(i + 1)` sur les 7 noms et le relisent `FIXED`. Typés, `cc`, `by`, `fo`, `m_` avortent sur un fixe (voir « Boucles sentinelles »).

R2 ne donne aucun indicateur de plus que le recensement : aucun TU lent ne lit `ma` absent (H4).

### Revue des tests de régression (§6), mécanique

**Liste** : l'union des tests nommés au §6 et dans les lignes « Tests : » des étapes 1a de `docs/plan-3.md`, soit 17 fichiers (`test_class_coverage` de 1a.0 est l'outil de couverture, pas un test de bug) :

- `built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `slipway_drag`, `game_end` ;
- `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close` ;
- `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`.

Tous sont dans `tests/test_pico8_<nom>.c`.

**Greps** sur ces 17 fichiers :

- accès direct : `grep -nE '(->|\.)(kr|cc|by|mi|m_|fo|ma)\b|offsetof\([^)]*\b(kr|cc|by|mi|m_|fo|ma)\)'` ;
- par nom : `grep -nE '"(kr|cc|by|mi|m_|fo|ma)"'`.

Le grep direct couvre `planet->jo.by` et `->qh.jo.mi`, la leçon de 1b.23a2/b.

**Aucun accès direct** : les deux seules correspondances directes sont des commentaires. **Toutes les correspondances**, chacune avec son plan de preuve rouge :

| # | test | ligne | accès | touché par le diff ? | plan de preuve rouge |
|---|---|---|---|---|---|
| R-a | `ga_export_panel_by` | `:82` `pico8_entity_get_field(planet, "by", &by)` ; `:1` commentaire | lecture par nom de `by` sur un `ga` neuf ; le test garde exactement la forme du défaut que cette étape déplace (`g_ga_defaults` → `g_ga_by`) | **oui**, une phrase de commentaire (`:136`, « Without the `by` swap, `jo:gh`'s REF check aborts first »), fausse après l'étape : `list_of` disparaît et c'est l'init de `ga` qui avorte d'abord. Elle devient « Without the `by` swap, ga's init aborts first. » (G). Rien d'autre ne change | Bug remis localement : on retire le bloc d'échange de `by` de `pico8_planet_family_init`. Attendu **rouge** (`SIGABRT` dans `family_default_list`, qui exige le tag `LIST`, H16). On restaure : vert. Deux lignes « Runs ». |
| R-b | `rich_planet_exports` | `:84` `pico8_entity_set_field(planet, "mi", fixe)` | écriture par nom de `mi`, dans le genre, par la nouvelle branche R5 | non | Bug remis localement : dans `bb_visit` (`economy_tiers.c`), `ob_sequence_length(po)` redevient le compte de `po`, qui inclut la clé nommée `mc`. Attendu **rouge**. On restaure : vert. Deux lignes « Runs ». Le chemin gardé (`bb` → `mi`) passe par le code modifié. |
| R-c | `planet_kj_dispatch` | `:132` commentaire « `self.ma` having decayed » | homonyme : `qb.ma` (1b.19). Le test mène toutefois `jo:kj` sur un vrai `jo` : `ma()` y écrit `jo.ma` par nom, et le bloc `fo` y est lu | non | Bug remis localement : l'ordre de `dc("kj")` inversé (`K` avant `M`). Attendu **rouge** (`g_order`). On restaure : vert. Deux lignes « Runs ». Le chemin `jo:kj` est touché. |
| R-d | `ga_export_panel_by` | `:133` commentaire « `self.by` already holds » | commentaire | non (commentaire exact) | couvert par R-a |

Les 14 autres tests n'ont aucune correspondance. Ceux qui atteignent ces membres par `src/` (économie, survol, sondes) restent verts dans `make test` sans modification (H17).

### Ce que fait le cart (`pico8/lua.lua`)

- **Défauts** :
  - `ma=0` dans la spec de `jo`, qu'aucune fille ne redéfinit ;
  - `mi=1` dans la spec de `qh`, hérité par ses 7 descendantes (`nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`), aucune ne le redéfinit ; `jo` et `ga` n'en ont pas ;
  - `by=o()` dans la spec de `ga`, **défaut muté** (§5 1a, « `ga.by` ») : une même liste, partagée par tous les `ga` et remplie en place par `jo:ib` (`add(ib,a)`).
- `kr` : `hy:lj` (`p.kr=flr(db(p)-da(p))`) ; lu par `jo:hk` (`if (self.kr or 0)~=0`) et `jo:gh` (`h_=self.kr and {…}`).
- `cc` : `bd` (`pt.cc={}`, puis `add(to.cc,a)`) ; lu par nom (`economy_*`).
- `by` : `bd` (`pt.by=pt:ib()`) ; lu par `jo:ib` (`self.by or {}`), `jo:hb`, `jo:gh` (`#self.by`), et par nom (`economy_*`).
- `mi` : `bb` (`pt.mi=jw`) ; lu par `jo:gh`, `jo:kj`, `nl:ly` (sans garde), `hj:kj`, et par nom (`economy_*`).
- `m_` : `gw` (`(self.gj or {}).m_=false`, `self.o_.m_=true`).
- `ma` : `jo:kd` (`self.ma=1.25`), `ma()` (`e.ma=max(e.ma-0.1,0)`).
- `fo` : `bv` et `jc:hq` (`et.fo=self`) ; lu et remis à nil par `jo:kj` (`if self.fo then … self.fo=nil end`).

### Le code (arbre 1b.23c supposé commité)

- **Typedef** (`planet.h`) : `ma` après `kq` ; `fo` après `lq` ; `kr`, `cc`, `by`, `mi`, `m_` en fin, après `has_p_`. Tous `pico8_value_t`.
- **`g_jo_defaults[]`** (`planet.c`) : **une seule entrée, `ma`**. Elle sert à quatre endroits :
  - la boucle de lecture de spec de `pico8_jo_init` (`assert(found && "jo: every default of the struct is a key of jo's spec")`) ;
  - la boucle de remise à nil de `pico8_jo_defaults_reset` ;
  - la boucle de recopie `*pico8_jo_member(self, …) = …` de `pico8_jo_struct_init` ;
  - la déclaration elle-même.
- **`g_jo_members[]`** : **7 entrées, exactement ces 7 membres**. Elle sert à quatre clients :
  - `pico8_jo_member` ;
  - le repli du lecteur `pico8_jo_field`, après ses 23 branches typées ;
  - la boucle `written[]` de l'écrivain (`{"by", "cc", "fo", "kr", "m_", "ma", "mi"}`) ;
  - la boucle nil de `pico8_jo_struct_init`.

  La boucle de clés de `pico8_jo_struct_new` ne l'appelle plus depuis 1b.23c.
- **Tables de défauts des filles** qui listent ces membres : `g_qh_defaults` `{"mi", offsetof(pico8_qh_t, jo.mi)}` (`planet_family.c`, recopiée par `pico8_qh_struct_init` pour les 8 structs qui l'appellent : `qh`, `nl`, `os`, `hj`, `pm`, `mx` et, dans `planet_family_trade.c`, `nr`, `fr`) ; `g_ga_defaults` `{"by", offsetof(pico8_ga_t, jo.by)}` (recopiée par `pico8_ga_struct_new`, remplie à l'init **après** l'échange `o()` → liste du tas).
- **Écrivain** : seul `pico8_jo_set_field` prend ces 7 noms. `qh`, `ga` et, par délégation, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx` s'en remettent à lui. Aucune boucle de clés ne les prend (clés mesurées : `id`, `p`, `ra` et les membres propres de `ga`).
- **Tables d'offsets** : `grep -rnE 'offsetof\(pico8_[a-z0-9_]+_t, ([a-z0-9_]+\.)*(kr|cc|by|mi|m_|fo|ma)\)' src/ | grep -E 'pico8_(jo|qh|nl|os|hj|pm|ga|nr|fr|mx)_t'` rend **9 lignes** : 7 de `g_jo_members`, 1 de `g_qh_defaults`, 1 de `g_ga_defaults`.
- **Lectures directes (R6)** :
  - `planet.c` : `jo:hk` (`kr_val = self->kr`, `pico8_jo_present`, `pico8_jo_truthy`, `kr_val.as.fixed`) ;
  - `planet_display.c` : `jo:hb` (`list_of(self->by)`) ; `jo:gh` (`self->mi` puis `mi_val.kind`/`.as.fixed`, `self->kr` et son vrai, `fixed_of(self->mi)`, `list_of(self->by)`) ; `jo:kj` (`fixed_of(self->mi)`, puis le bloc `fo` : `fo_val = self->fo`, vrai, REF non NULL, lecture par nom de `dw` sur le porteur) ;
  - `planet_trade.c` : `jo:ib` (`self->by.kind/.as`) ;
  - `planet_family.c` : `nl:ly` (`assert(self->qh.jo.mi.kind == PICO8_VALUE_FIXED && "nl:ly reads self.mi unguarded, unlike hj:kj")`, puis `mi >= 2`) ; `hj:kj` (`pico8_pf_truthy(mi_val) && kind == FIXED && >= 2`).
- **Écritures directes (R6)** : `planet.c` : `jo:kd` (`self->ma = pico8_value_fixed(1.25)`) ; `planet_display.c` : `jo:kj` (`self->fo = pico8_value_ref(NULL)`). Aucune autre (`kr`, `cc`, `by`, `mi`, `m_` ne sont écrits que par nom).
- **Sites C1 de `float-audit`** qui citent un de ces membres (`tools/float_audit_c1.txt:41,44`) : `pico8_jo_gh | mi_val.as.fixed | 1` et `pico8_jo_kj | fixed_of(self->mi) | 1`. Leur extrait change avec R6 (voir « `float-audit` »).
- **Aides statiques qui tombent avec leurs derniers appelants** (`-Werror` : une fonction `static` inutilisée ne compile pas) :
  - `list_of` et `fixed_of` de `planet_display.c` : derniers appelants `by`, `mi` ;
  - `pico8_jo_present` de `planet.c` : dernier appelant `kr` ;
  - `pico8_pf_truthy` de `planet_family.c` : seul appelant `hj:kj`.

  Les `pico8_jo_truthy` de `planet.c`, `planet_display.c` et `planet_trade.c` gardent d'autres appelants (`hp`, `hf`, `ll`, `me`, `px`, `gg`, `fx`).
- **Sites par nom, inchangés** (puce Coexistence) : `economy.c` (`kr`), `economy_tiers.c` (`cc`, `by`, `mi`), `economy_score.c` (`mi`, `by`), `entity_geom.c` (`ma`), `ui.c` (`m_`), `probe.c` (`fo`), `slipway.c` (`fo`), et les TU.
- **Homonymes hors étape** :
  - `m_` de `bg`, `fq`, `js`, `ie`, `fv` ;
  - `ma` de `qb` (dont `ga:pq` sur `mo`) ;
  - `fo` de `jc` (1b.35), de `bv_et` (1b.22), de `jc_too_far` (1b.13) ;
  - `mi` et `by` de la spec (`pico8_*_default`).

### `fo` : REF à dix cibles, forme 1b.22 (G)

**Qui écrit `jo.fo`, par le code :**

- `bv` (`probe.c`) écrit `et.fo = self`, où `et` est une planète quand `ij(p, cg("np"))` rend une distance `nq < self.dw`. Les porteurs de `bv` sont les descripteurs dont `methods[PICO8_METHOD_HQ] == pico8_bv_dispatch` : `g_`, `fv`, `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`. `mz` y passe aussi, par `mz:hq` qui appelle `pico8_bv`.
- `jc:hq` (`slipway.c`) écrit `et.fo = self` sur ce que rend `jc:ce`, qui peut être une planète : c'est `test_pico8_jc:707` (`origin` = `qu` du slipway).

**Qui atteint vraiment une planète :**

- `dw` vaut 0,025 pour `g_` (spec) et `fv` (qui l'hérite de `g_`), et 0,025 pour `qh` et ses 7 descendantes (spec de `qh` ; seul `fr` le porte à 0,01).
- **`mz.dw` vaut 0 et ne change jamais** (spec `dw=0`, pose `self->dw = pico8_value_fixed(0)`, aucun écrivain de `dw`). Or `nq ≥ 0`, donc `nq < 0` n'arrive jamais : **`mz` ne pose jamais `fo` sur une planète** (H3).

**Ensemble admis** (lecture Coexistence de 1b.22) :

- **ligne** : `fr`, `g_`, `hj`, `jc`, `mx`, `nl`, `nr`, `os`, `qh` (9) ;
- **∪ producteurs prouvés par le code** : `pm` (porteur de `bv` à `dw` hérité de `qh`) et `fv` (porteur de `bv` à `dw` hérité de `g_`). Tous deux sont exercés par la caractérisation sur l'ancien code (`hq` mené sur un `pico8_X_new(NULL)` neuf, visant une planète de la galaxie, puis aller-retour par nom) ;
- **soit 11 classes** : les 10 porteurs de `bv_dispatch`, plus `jc`. **`mz` est exclue.**

**Comparaison avec `bv_et.fo` (1b.22)** : les deux ensembles ne sont **pas** les mêmes.

| | porteurs de `bv_dispatch` (10) | `mz` | `jc` |
|---|---|---|---|
| `bv_et.fo` | admis | **admis** (`mz:hq` → `bv` atteint `g_bv_et`) | refusé (`jc:hq` ne vise jamais `g_bv_et`) |
| `jo.fo` | admis | **refusé** (`dw` = 0, H3) | **admis** (`jc:hq` vise une planète) |

`jc_too_far.fo` (1b.13) ne reçoit que des `jc`. C'est un autre membre, que seul `jc:hq` écrit. `jo.fo` en a une cible de plus, `jc`, et dix autres.

**Prédicat** :

```c
static int jo_is_fo_holder_class(const pico8_struct_class_t *cls) {
    return cls->methods[PICO8_METHOD_HQ] == pico8_bv_dispatch || cls == pico8_jc_struct_class();
}
```

Il est tiré de la règle de 1b.22, pas prototypé.

- **Pas de liste d'accesseurs** : `planet.c` est lié par `DATA_ENGINE_SRCS`, qui n'a ni `help.c` (`pico8_fv_struct_class`) ni `planet_family_trade.c` (`nr`, `fr`, `mx`). C'est le piège de 1b.22.
- **Propriété du descripteur partagée exactement** : `methods[HQ] == pico8_bv_dispatch` désigne exactement les 10 porteurs de `bv` admis (grep de `[PICO8_METHOD_HQ] = pico8_bv_dispatch` : `help.c`, `probe.c`, 5 dans `planet_family.c`, 3 dans `planet_family_trade.c`). `mz` (`pico8_mz_hq_dispatch`) et `bg` (`pico8_bg_hq_dispatch`) en sont exclues. `jc` est ajoutée par son accesseur public.
- **Liens** : `pico8_bv_dispatch` (`probe.c`) et `pico8_jc_struct_class` (`slipway.c`) sont liés partout où `planet.c` l'est (`ENGINE_SRCS`, `DATA_ENGINE_SRCS`, `main_raylib`). `planet.c` inclut déjà `pico8_probe.h` et `pico8_slipway.h`, et appelle déjà `pico8_jc_new` (H18).

**Aide** : `static void jo_take_holder(pico8_value_t value, pico8_struct_header_t **out)`, dans la forme de `bv_et_take_holder` :

- `*out = NULL` ; `return` sur `REF NULL` ;
- sinon `assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT && jo_is_fo_holder_class(((const pico8_struct_header_t *)value.as.ref)->cls) && "jo.fo: a write neither nil nor an object whose hq runs bv (g_, fv, qh, nl, os, hj, pm, nr, fr, mx) or a jc is off its census row (ref: to those holders) -- a field under no rule of plan-3 section 5: stop, do not widen the member")` ;
- `*out = value.as.ref`.

Un seul appelant : la branche `fo` de l'écrivain de `jo`.

**Lecture** : `pico8_jo_field` rend `pico8_value_ref(jo->fo)`, et 0 quand le pointeur est NULL. **`jo:kj`** : `if (self->fo != NULL) { … pico8_entity_get_field(self->fo, "dw", …) … self->fo = NULL; }`. Le test `pico8_jo_truthy(fo_val) && REF && ref != NULL` équivaut à `!= NULL` pour un pointeur.

### Coexistence : les 7 derniers membres typés, les tables de `jo` supprimées

- **Lecteur** `pico8_jo_field` : 7 branches R4 de plus, dans l'ordre du typedef (`ma` après `kq`, `fo` après `lq`, puis `kr`, `cc`, `by`, `mi`, `m_` en fin) :
  - `ma` rend toujours `pico8_value_fixed(ma)` et 1 ;
  - un pointeur NULL ou un `has_` à 0 rend 0, `*out_value` intact ;
  - sinon `pico8_value_ref(cc/by/fo)`, `pico8_value_fixed(kr/mi)`, `pico8_value_bool(m_)`.

  Le repli `member = pico8_jo_member(…)` disparaît : un nom sans branche va directement à `return pico8_ke_field(self, name, out_value);`. Les lecteurs des filles délèguent déjà.
- **Écrivain** `pico8_jo_set_field` : 7 branches R5, intercalées dans l'ordre du typedef avec celles de 1a2/1b.23c. On obtient 17 branches : `ma`, `kw`, `oe`, `hn`, `ep`, `jy`, `nk`, `fx`, `lq`, `fo`, `jh`, `p_`, `kr`, `cc`, `by`, `mi`, `m_`. Puis `return 0;`. La boucle `written[]`, son tableau et sa variable `i` disparaissent. L'ensemble des 17 noms accepté ne change pas, et `id` reste refusé. Le commentaire de tête « The 17 names … » reste vrai, inchangé.
- **Tables** (voir « Sortie de `jo` ») : `g_jo_members[]` et `g_jo_defaults[]` sont **supprimées**. `g_qh_defaults` perd `mi` et `g_ga_defaults` perd `by`. Chacune garde ses autres entrées et sa boucle (R7 de 1b.14a, point 1).

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (arbre 1b.23c commité). Elle couvre :
   - les 7 membres lus par nom sur `jo` et ses 9 filles, neuves et vivantes (début et fin du rejeu `DRAG`) ;
   - un aller-retour par chacun des 7 écrivains génériques (`hy:lj`, `bd`, `bb`, `gw`, `ma()`, `bv`, `jc:hq`) ;
   - l'écrivain par nom dans le genre sur chaque chaîne de délégation ;
   - `jo:kd`, `jo:hk`, `jo:kj`, `jo:ib` ;
   - les producteurs `pm` et `fv` de `fo`.

   Elle doit être verte avant tout typage.
2. **Test d'arrêt, rouge sur l'ancien code** : l'écrivain hors genre sur les 7 noms, avec les cas à union nulle, nil pour `ma`, et les classes de struct refusées pour `fo` (`mz`, un `jo` nu, un `ga`, un `hp`, le `bg`, l'`ok`).
3. **Commit 1b.23d** :
   - les 7 membres sont typés ; `has_kr`, `has_mi`, `has_m_` sont ajoutés ;
   - `g_jo_members`, `g_jo_defaults`, `pico8_jo_member` et leurs boucles sont supprimés ;
   - `mi` sort de `g_qh_defaults` et `by` de `g_ga_defaults`, déballés en `static` (`g_jo_ma`, `g_qh_mi`, `g_ga_by`) ;
   - 7 branches d'écrivain par des aides de `planet.c` ;
   - les lectures et écritures directes sont réécrites ; `list_of`, `fixed_of`, `pico8_jo_present` et `pico8_pf_truthy` sont retirés ;
   - deux extraits C1 de `float-audit` sont réécrits ;
   - les 7 noms sortent de 10 tableaux `written[]`. Celui de `test_pico8_jo`, vidé, est supprimé avec sa boucle ;
   - une phrase de commentaire de `ga_export_panel_by` change.

   `make value-members` rend **138**, dont **0 ligne `pico8_jo_t`**.
4. Aucun pixel ne bouge. La note « Découvert à 1b.23d » va sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `planet.h`, je veux `pico8_jo_t` sans aucun `pico8_value_t`, afin que toute la planète se lise sur ses types.
2. En tant que lecteur de `planet.h`, je veux `cc`, `by` typés `pico8_list_t *`, `fo` typé `pico8_struct_header_t *`, `ma` typé `pico8_fixed_t`, `kr`, `mi` typés `pico8_fixed_t` avec `has_kr`, `has_mi`, et `m_` typé `int` avec `has_m_`.
3. En tant que lecteur par nom (`economy*.c`, `entity_geom.c`, `ui.c`, `probe.c`, `slipway.c`, les TU), je veux que chacun des 7 noms rende exactement ce que rendait l'ancien code sur les 10 genres de l'arbre `jo` : présence, genre, valeur, identité du pointeur.
4. En tant que planète neuve, je veux `ma` présent à 0 ; `kr`, `cc`, `fo`, `m_` absents ; `mi` présent à 1 exactement sur `qh` et ses 7 descendantes, et absent sur `jo` et `ga` ; `by` présent sur `ga` seul, **pointeur même** de la spec de `ga`.
5. En tant que `ga`, je veux partager la liste `by` de ma spec avec tous les autres `ga`, comme en 1a (défaut muté), afin que `jo:ib` la remplisse en place.
6. En tant que `hy:lj`, je veux écrire `kr` par nom sur chaque planète, et que `jo:hk`/`jo:gh` lisent « `kr` absent » par `has_kr`.
7. En tant que `bd`, je veux lire l'ancien `cc` par nom, le libérer, puis écrire le nouveau ; et écrire `by` = la liste que rend `ib`, ou nil.
8. En tant que `bb`, je veux écrire `mi` par nom, et que `nl:ly`, `hj:kj`, `jo:gh`, `jo:kj` le lisent par le type.
9. En tant que boucle de survol `gw`, je veux poser `m_` faux **présent** sur `gj` et vrai sur `o_`.
10. En tant que `ma()`, je veux décrémenter `ma` par nom, et que `jo:kd` le pose à 1,25.
11. En tant que `bv` ou `jc:hq`, je veux poser `fo` = le porteur sur la planète visée, même pointeur ; en tant que `jo:kj`, je veux le lire puis le remettre à nil.
12. En tant qu'écrivain par nom de l'arbre `jo`, je veux ces 7 noms acceptés dans leur genre, ou nil (sauf `ma`), même pointeur, et tout autre genre arrêté par un `assert` « champ sans règle » qui nomme `jo.<m>` et le genre de sa ligne.
13. En tant que gardien de `fo`, je veux refuser `mz`, un `jo` nu, un `ga`, un `hp`, le `bg`, l'`ok` et tout non-struct, et admettre exactement les 10 porteurs de `bv` et `jc`.
14. En tant que mainteneur, je veux que l'ensemble des 17 noms qu'accepte l'écrivain ne change pas.
15. En tant que mainteneur, je veux `g_jo_members`, `g_jo_defaults` et `pico8_jo_member` supprimés, et qu'aucune table de défauts, d'offsets ou de clés ne liste plus un membre de `jo`.
16. En tant que TU à boucle sentinelle, je veux que ma boucle ne réécrive plus ces 7 noms en fixe. Celle de `test_pico8_jo`, vide, est supprimée.
17. En tant que test de régression `ga_export_panel_by`, `rich_planet_exports` ou `planet_kj_dispatch`, je veux rester vert, et rouge quand mon bug revient.
18. En tant que `float-audit`, je veux que les deux sites C1 de `mi` gardent leur valeur, avec leur extrait réécrit.
19. En tant qu'auteur de 1b.24–1b.36, je veux une forme écrite pour la suppression d'une table d'offsets qui a des filles, et un second prédicat de REF à plusieurs cibles dont l'ensemble diffère d'un homonyme.
20. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
21. En tant que gardien de la mémoire et de la couverture, je veux `make sanitize-net` à 15 propres et `make class-coverage-net` identique à sa référence.
22. En tant que `plan-pilot`, je veux `make value-members` à 138 et `pico8_jo_t` absent de sa sortie.
23. En tant que vérificateur, je veux la caractérisation verte sur l'ancien code, le test d'arrêt rouge puis vert, les preuves rouges des 3 tests de régression, les mutations du §7 et leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés comme le ferait un agent `guideline` : hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables. Ils sont marqués (G).

### R1–R9 appliquées

| règle | sur les 7 membres |
|---|---|
| R1 | Tableau de recensement. Le nom et la place dans le typedef sont gardés ; `has_<m>` juste après `<m>`. Tous empruntés, jamais possédés ni libérés par `jo` : `cc` est libéré par `bd` comme aujourd'hui, `by` appartient à la spec ou au tas (TROU-7), `fo` au porteur. |
| R2 | `has_kr`, `has_mi`, `has_m_` (recensement, revue des TU lents). Aucun pour `ma` (nil `jamais`, H4), aucun pour les 3 pointeurs. |
| R3 | `pico8_jo_struct_init` : `ma = g_jo_ma` ; `fo = NULL` ; `kr = 0`, `has_kr = 0` ; `cc = NULL` ; `by = NULL` ; `mi = 0`, `has_mi = 0` ; `m_ = 0`, `has_m_ = 0`. Ces poses remplacent la boucle nil et la recopie de `g_jo_defaults`. `pico8_qh_struct_init` : `jo.mi = g_qh_mi ; jo.has_mi = 1`, après sa boucle de défauts. `pico8_ga_struct_new` : `jo.by = g_ga_by`, après sa boucle de défauts, avant les clés. Le défaut REF garde le **même pointeur** que la spec. |
| R4 | 7 branches dans `pico8_jo_field`, ordre du typedef. Aides `jo_field_list` (1b.23c) pour `cc`/`by`. Une aide `jo_field_holder` à côté (G), ou le test en ligne, au choix de l'implémenteur. |
| R5 | Écrivain de `jo` seul : voir plus bas. Aucune boucle de clés. |
| R6 | Voir « Lectures et écritures directes ». |
| R7 | Les 7 entrées de `g_jo_members` sont retirées, puis la table ; l'entrée `ma` de `g_jo_defaults` est retirée, puis la table ; `mi` sort de `g_qh_defaults` et `by` de `g_ga_defaults` (1b.14a point 1, tables des filles). |
| R8 | Crochets `PICO8_MEMBER_CENSUS_*` gardés, y compris `PICO8_MEMBER_CENSUS_SEE(self, pico8_jo_t)` sur un typedef sans membre `pico8_value_t` (précédent `ke`, 1b.14b ; H7). `tools/member_census.txt` n'est pas édité. |
| R9 | 145 → 138. `pico8_jo_t` sort de la sortie de `make value-members`. |

### Défauts : `ma`, `mi`, `by` (R7 de 1b.14a) (G)

- **`ma`** (`planet.c`) : `static pico8_fixed_t g_jo_ma`, déballé dans `pico8_jo_init` par `jo_default_fixed`. Le message de l'aide devient `"jo.hs/fa/ca/ma: … (fixe)"`. Il est remis à 0 dans `pico8_jo_defaults_reset`.
- **`mi`** (`planet_family.c`) : `static pico8_fixed_t g_qh_mi`, déballé dans `pico8_planet_family_init` par `family_default_fixed` après la boucle de `qh`. Le message devient `"os.hs, ga.fa, qh.mi: … (fixe)"`. Il est remis à 0 dans `pico8_qh_defaults_reset`. Aucune autre table de fille ne liste `mi`.
- **`by`** (`planet_family.c`) : `static pico8_list_t *g_ga_by`, déballé **après l'échange** `o()` → liste du tas, par une aide nouvelle `family_default_list`. L'aide exige une REF non NULL de tag `LIST`, avec le message `"ga.by: a default of the struct is a key of ga's spec and a list (ref:liste)"`. `g_ga_by` est remis à NULL dans `pico8_ga_defaults_reset`. C'est le **pointeur de la spec**, parce que `ga:380`, `ga:551` et `on_jo[]` figent l'identité et le partage (défaut muté du §5 1a). Ce n'est ni une copie ni une liste module. `pico8_ga_struct_default` n'a qu'un appelant (`"k_"`), non touché.
- L'échange `o()` → liste reste tel quel. Sans lui, `family_default_list` avorte à l'init : c'est la preuve rouge R-a (H16).

### R5 : écrivain de `jo` (G)

Les aides vivent dans `planet.c`. 1b.13 point 2 les veut une par genre, jamais partagées entre fichiers ; l'écrivain de `jo` est le seul client.

- **`jo_take_list`** (1b.23c) : appelée aussi pour `cc` et `by`. Le message devient `"jo.oe/jy/cc/by: … (ref:liste) …"`.
- **`jo_take_fixed`** (1b.23c, à indicateur) : appelée aussi pour `kr` et `mi`. Le message devient `"jo.jh/kr/mi: … (fixe) …"`.
- **`jo_take_bool`** (1b.23c, à indicateur) : appelée aussi pour `m_`. Le message devient `"jo.fx/p_/m_: … (bool) …"`.
- **`jo_take_ma`** (nouvelle) : forme `qb_take_fixed` de 1b.19, sans état nil. `assert(value.kind == PICO8_VALUE_FIXED && "jo.ma: a write that is not a fixed is off its census row (fixe, never nil) -- a field under no rule of plan-3 section 5: stop, do not widen the member")`, puis `*out = value.as.fixed`. **Nil avorte** (1b.13 point 3). L'aide porte le nom du membre parce qu'elle diffère par sa règle de nil de `jo_take_fixed`, pas par son genre (précédents `hp_take_il`, `jc_too_far_take_jc`).
- **`jo_take_holder`** (nouvelle), avec son prédicat `jo_is_fo_holder_class` : voir « `fo` ».
- **Branches** `if (strcmp(name, "<m>") == 0) { jo_take_<…>(value, &jo-><m>[, &jo->has_<m>]); return 1; }`, dans l'ordre du typedef : `ma` (en tête), `fo` (après `lq`), `kr`, `cc`, `by`, `mi`, `m_` (en fin). Puis `return 0;`.
- Écarté (G) :
  - une aide par membre pour `kr`/`mi` : ils partagent le genre et la règle de nil de `jh` ;
  - l'élargissement de `jo_take_fixed` à `ma` : nil y serait accepté, contre la ligne `jamais` ;
  - le partage de `bv_et_take_holder` : c'est un autre fichier, et un autre ensemble.

### Lectures et écritures directes (R6)

- **`planet.c`** :
  - `jo:kd` → `self->ma = PICO8_FIXED_LIT(0x00014000, 1.25);` ;
  - `jo:hk` → `if (!self->has_kr || self->kr == pico8_fixed_from_int(0)) return;` (un fixe présent est vrai en Lua), puis `pico8_ne(self->kr, 10, "$", &color)` ;
  - `pico8_jo_present` est supprimée.
- **`planet_display.c`** :
  - `jo:hb` → `by_list = self->by;` ;
  - `jo:gh` :
    - `if (self->has_mi && pico8_jo_dz() != NULL) … pico8_fixed_flr_int(self->mi) …` ;
    - `if (self->has_kr)`, texte sur `self->kr`, et `bg` sur `self->kr >= 0` ;
    - `mi_fixed = self->has_mi ? self->mi : pico8_fixed_from_int(0);` ;
    - `const pico8_list_t *by = self->by;` ;
  - `jo:kj` :
    - `int mi = pico8_fixed_flr_int(self->has_mi ? self->mi : pico8_fixed_from_int(0));` ;
    - bloc `fo` : `if (self->fo != NULL) { … pico8_entity_get_field(self->fo, "dw", &dw_val) … self->fo = NULL; }` ;
  - `list_of` et `fixed_of` sont supprimées.
- **`planet_trade.c`** : `jo:ib` → `if (self->by != NULL) { ib = self->by; } else { ib = malloc(…); pico8_list_init(ib); }`.
- **`planet_family.c`** :
  - `nl:ly` → `assert(self->qh.jo.has_mi && "nl:ly reads self.mi unguarded, unlike hj:kj");`, puis `if (self->qh.jo.has_mi && self->qh.jo.mi >= pico8_fixed_from_int(2))`. Le message de l'`assert` est gardé ;
  - `hj:kj` → `if (self->qh.jo.has_mi && self->qh.jo.mi >= pico8_fixed_from_int(2))` ;
  - `pico8_pf_truthy` est supprimée.
- Écarté (G) : faire passer `jo:kd` ou le `fo = NULL` de `jo:kj` par une aide. Ce sont des littéraux C déjà dans le genre (R6, précédent 1b.23c).

### `float-audit` : deux extraits C1 réécrits (lecture de 1b.21)

`tools/float_audit_c1.txt` : seules les lignes `pico8_jo_gh | mi_val.as.fixed | 1` et `pico8_jo_kj | fixed_of(self->mi) | 1` changent. Leur extrait devient l'argument réécrit, tel qu'écrit (`self->mi` et `self->has_mi ? self->mi : pico8_fixed_from_int(0)`, ou ce que l'implémenteur a écrit à l'identique). L'occurrence (1) et toutes les autres lignes restent. `make float-audit` (dans `make e2e`) reste vert.

### Boucles sentinelles (forme de 1b.23a2, recopiée ; lecture nouvelle pour un tableau vidé)

- Les 7 noms sortent des tableaux `written[]` de `qh`, `os`, `hj`, `pm`, `mx`, `nr`, `nl`, `fr`, `ga`. Chacun garde ses noms propres (`et` ; `et`, `qy` ; `et`, `qy`, `ht` ; `et`, `s` ; `et`, `qw` ; `qy`) et sa boucle. Rien d'autre ne change.
- **`test_pico8_jo`** : son `written[]` ne contient que ces 7 noms, et le vider donnerait un tableau de taille nulle, illégal en C. **Lecture nouvelle (G)** : un tableau sentinelle vidé est supprimé avec sa boucle ; les autres variables restent (`i` sert encore à `methods[]`). Les `must_abort(write_ra)` et `must_abort(write_id)` qui suivent restent. Le commentaire de tête (« a generic write outside the 17 measured names … stops the run ») reste vrai.
- L'intention est reportée dans la caractérisation (aller-retour dans le genre, nil) et dans le test d'arrêt (hors genre). Les boucles de `g_` et `fv` (homonymes `m_`, `fo`) ne sont pas touchées.

### Sortie de `jo` : suppression des deux tables (forme 1b.9, étendue à une table à filles)

Une fois les 7 entrées retirées, `g_jo_members[]` et `g_jo_defaults[]` n'ont plus d'entrée. La lecture de R7 de 1b.9 s'applique : **une table vidée est supprimée avec ses clients**. Le « reste à fixer » de 1b.9 pour une table à filles se ferme ainsi, parce que les filles n'ont jamais lu ces tables (elles ont les leurs, qui gardent des entrées).

| supprimé | où | remplacé par |
|---|---|---|
| `g_jo_members[]` | `planet.c` | — |
| `pico8_jo_member` | `planet.c` | — (plus aucun appelant) |
| boucle nil de `pico8_jo_struct_init` | `planet.c` | poses R3 typées |
| repli `member = pico8_jo_member(…)` du lecteur | `planet.c` | `return pico8_ke_field(self, name, out_value);` |
| `written[]`, sa boucle et `i` dans l'écrivain | `planet.c` | 7 branches R5, puis `return 0;` |
| `g_jo_defaults[]` et son commentaire | `planet.c` | `static pico8_fixed_t g_jo_ma` |
| boucle de lecture de spec de `pico8_jo_init` et son `assert` « every default … » | `planet.c` | `found = pico8_jo_default("ma", &v); g_jo_ma = jo_default_fixed(found, v);` |
| boucle de remise à nil de `pico8_jo_defaults_reset` | `planet.c` | `g_jo_ma = 0;` |
| boucle de recopie de `pico8_jo_struct_init` | `planet.c` | `self->ma = g_jo_ma;` |
| variables `size_t i` devenues inutiles (`-Werror`) | `planet.c` | — |

`pico8_jo_default` (public, lu par les TU et par `pico8_jo_init`) reste.

### Preuves (sortie attendue sur l'arbre final)

**Sortie de `jo`** (critère de l'étape) :

- `sed -n '/^typedef struct {/,/} pico8_jo_t;/p' src/pico8_planet.h | grep -c 'pico8_value_t'` : **0** ;
- `make value-members | grep -c 'pico8_jo_t'` : **0** ;
- `grep -rnw 'g_jo_members\|g_jo_defaults\|pico8_jo_member' src/` : **vide** ;
- `grep -rn 'offsetof(pico8_jo_t' src/` : **vide**.

**Tables et aides** :

- `grep -rnE 'offsetof\(pico8_[a-z0-9_]+_t, ([a-z0-9_]+\.)*(kr|cc|by|mi|m_|fo|ma)\)' src/ | grep -E 'pico8_(jo|qh|nl|os|hj|pm|ga|nr|fr|mx)_t'` : vide (9 lignes avant) ;
- `grep -n 'pico8_jo_present\|list_of\|fixed_of\|pico8_pf_truthy' src/pico8_planet*.c` : vide ;
- `grep -n 'pico8_value_t \*' src/pico8_planet.c | grep -v 'pico8_value_t \*out_value)' | grep -v 'pico8_value_t \*out)'` : relu, aucun membre (paramètres de sortie seulement) ;
- `grep -nE '(\.|->)(kr|cc|by|mi|m_|fo|ma)\.(kind|as)' src/pico8_planet*.c` : vide ;
- `grep -rnE 'jo\.(kr|cc|by|mi|m_|fo|ma) = |->(kr|cc|by|mi|m_|fo|ma) = ' src/pico8_planet*.c` : seulement les poses R3 (`jo_struct_init`, `qh_struct_init`, `ga_struct_new`), `jo:kd` et `jo:kj` (`fo = NULL`) ;
- `grep -c 'off its census row' src/pico8_planet.c` : 10 (1b.23c : 8) ; `planet_family.c` et `planet_family_trade.c` inchangés ;
- `grep -n 'has_kr\|has_mi\|has_m_' src/pico8_planet.h` : 3 lignes, chacune juste après son membre ;
- `grep -n 'jo_is_fo_holder_class' src/pico8_planet.c` : définition et un seul appel (dans `jo_take_holder`).

**Périmètre du diff** :

- `git diff --stat HEAD -- src/ ':!src/pico8_planet*'` : vide (sites par nom intacts) ;
- `git diff HEAD -- tools/` : les deux lignes C1, rien d'autre ;
- `git diff` des 9 TU à boucle sentinelle : dans `written[]` seulement, les 7 noms retirés ; `test_pico8_jo` : le tableau et sa boucle retirés ;
- `git diff tests/test_pico8_ga_export_panel_by.c` : une ligne de commentaire.

### Empreinte remesurée ; découpage

La ligne dit 34 (12, 22). Par greps de noms, le majorant dépasse 60, gonflé par homonymie (`m_` de 5 structs, `ma` de `qb`, `fo` de `jc`/`bv_et`/`jc_too_far`, `mi`/`by` des specs).

**Empreinte réelle** (fichiers qui lisent ou écrivent l'un des 7 membres d'une struct de l'arbre `jo`), par lecture : **≈ 34 (12, 22)**, sous le seuil de ~40.

- **src (12)** : `planet.h`, `planet.c`, `planet_display.c`, `planet_trade.c`, `planet_family.c`, `economy.c`, `economy_tiers.c`, `economy_score.c`, `entity_geom.c`, `ui.c`, `probe.c`, `slipway.c`.
- **tests (≈ 22)** : `test_pico8_jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`, `nl`, `fr`, `ga`, `ui`, `jc`, `g_`, `hy`, `trade`, `hp`, `ga_export_panel_by`, `rich_planet_exports`, `planet_kj_dispatch` (par `jo:kj`), `jo_spec_defaults`, `spec_defaults`, `empty_planet_cleared` (H19).

**Fichiers modifiés : 17**, plus `docs/plan-3.md` :

- **src (5)** : `pico8_planet.h`, `pico8_planet.c`, `pico8_planet_display.c`, `pico8_planet_trade.c`, `pico8_planet_family.c`. `planet_family_trade.c` n'est pas touché : ni accès direct ni table sur ces membres.
- **tools (1)** : `float_audit_c1.txt` (deux lignes).
- **tests (11)** :
  - `test_pico8_ui.c` (ajouts : caractérisation, test d'arrêt) ;
  - les tableaux `written[]` de `test_pico8_jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr` (**7 TU lents**), `nl`, `fr`, `ga` ;
  - la phrase de `test_pico8_ga_export_panel_by` (**test de régression §6**, preuve rouge R-a).

**Découpage (G) : aucun, un seul commit.** 17 fichiers, sous les 18 de 1b.23a2, qui a tenu en une passe. Le seuil de la consigne (« nettement plus de ~18 ») n'est pas franchi. Trois découpes sont écartées (G) :

- **(a) 1b.23d1 (`kr`, `cc`, `by`, `mi`, `m_`, `ma`) puis 1b.23d2 (`fo`)** : les deux moitiés touchent les **mêmes** 10 tableaux `written[]`, le même lecteur, le même écrivain, la même `struct_init` et `test_pico8_ui`. Cela ferait deux `make e2e`, deux `class-coverage-net` et deux passes sur 7 TU lents, hors budget. Surtout, `g_jo_members` ne disparaîtrait qu'au second commit, avec un état intermédiaire sans valeur de forme.
- **(b) défauts (`ma`, `mi`, `by`) puis écrits seuls** : même objection. `by` et `mi` sont aussi écrits par nom.
- **(c) par membre** : impossible, un membre change de type pour tous ses sites à la fois.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 145 sur l'arbre de départ (1b.23c commitée), puis 138 après. Seules les 7 lignes `pico8_jo_t` disparaissent ; aucun autre typedef ne bouge.
- **H2** : dans les 15 rejeux et dans tous les TU hors fils de `must_abort`, les aides ne voient que leur genre ou nil :
  - `kr` : fixe (`hy:lj`, TU) ;
  - `cc` : liste neuve (`bd`) ;
  - `by` : liste ou nil (`bd`/`ib`, `os:446`) ;
  - `mi` : fixe (`bb`, TU, `os:458` recopié d'un `qh` qui l'a) ;
  - `m_` : booléen (`gw`) ;
  - `ma` : fixe, **jamais nil** (`ma()`) ;
  - `fo` : nil ou une struct des 11 classes admises (`bv`, `jc:hq`, `test_pico8_jc:707`, `test_pico8_g_`).
- **H3** : `mz` ne pose jamais `fo` sur une planète. `mz.dw` = 0 (spec, pose du constructeur, aucun écrivain), et `ij` rend une distance ≥ 0, donc `nq < dw` est toujours faux. Aucun rejeu ni TU ne voit `FO_MZ` sur une planète (`holder_kind` de `test_pico8_jc`). Démentie, c'est l'arrêt « champ sans règle », sans élargir le prédicat.
- **H4** : aucun TU lent ne lit `ma` d'une struct de l'arbre `jo` absent, et aucun rejeu ni TU n'y écrit nil.
- **H5** : `cc` (`bd_reset_visit`), la liste fraîche de `jo:ib` et le `by` échangé de `ga` portent le tag `PICO8_TYPE_TAG_LIST` (`pico8_list_init`).
- **H6** : aucune valeur figée ne bouge :
  - `STRUCT_HASH`, `SPEC_HASH`, `SHARE_HASH`, `READY_HASH` de `test_pico8_jo_spec_defaults` ; `STRUCT_HASH` de `test_pico8_spec_defaults` ;
  - les hash des TU de l'arbre `jo`, de `test_pico8_hy`, `test_pico8_trade`, `test_pico8_hp`, `test_pico8_jc` (`DRAW_HASH`), `test_pico8_g_` ;
  - dans `test_pico8_ui` : `HOVER_HASH`, `JO_MEMBERS_HASH`, `JO_WRITTEN_HASH`, `JO_POSITION_HASH`, `JO_STATE_HASH`, `KE_ME_HASH`, `KE_T_HASH`, `DRAG_REPLAY_HASH`, `TECH_REPLAY_HASH` ;
  - les 15 empreintes du filet.
- **H7** : le build `-DPICO8_MEMBER_CENSUS` compile avec un `pico8_jo_t` sans membre `pico8_value_t` (la table C générée par `tools/entity_value_members.sh --c-table` n'a plus de ligne `jo`). Ce build n'est pas lancé.
- **H8** : `test_pico8_ui` reste sous 30 s avec deux fonctions de plus, dont un rejeu `DRAG` complet (hors `UNIT_TESTS_SLOW`).
- **H9** : 1b.23c est commitée. Elle laisse ces 7 membres en `pico8_value_t`, `g_jo_members` à 7 entrées, `g_jo_defaults` à `ma` seul, `written[]` de l'écrivain à ces 7 noms, les 10 `written[]` des TU à ces 7 noms plus leurs noms propres, et `list_of`/`pico8_jo_present` à leurs derniers appelants `by`/`kr`. Les lignes glissent seulement.
- **H10** : rien ne dépend de `sizeof(pico8_jo_t)` ni de la disposition des filles.
- **H11** : sur l'ancien code, chaque cas du test d'arrêt se termine sans `SIGABRT` (`_exit(0)`) : le `must_abort` est rouge. Un cas qui avorterait déjà est retiré du « rouge d'abord » et noté. Exemple : `cc` hors genre relu plus loin par un `bd` du même fils.
- **H12** : sur un `pico8_<X>_new(NULL)` neuf :
  - `ma` est présent, `FIXED` 0 ;
  - `kr`, `cc`, `fo`, `m_` sont absents ;
  - `mi` est présent, `FIXED` 1, sur `qh nl os hj pm nr fr mx`, et absent sur `jo`, `ga` ;
  - `by` est présent sur `ga` (pointeur de spec), absent ailleurs.

  Aucun `pq` ne les touche. Les valeurs figées sont celles **observées**.
- **H13** : `pico8_hy_lj(pico8_fm_hy())`, `pico8_bd()`, `pico8_bb()`, `pico8_ui_gw(&mem, ok)` et `pico8_ma(p, e)` sont appelables après `start_game(DRAG)`, sur des planètes de `cg.jo` et sur les planètes neuves de la caractérisation.
- **H14** : `hq` d'un `pm` et d'un `fv` neufs, mené avec `mp` = le `ra` d'une planète de la galaxie, pose `fo` sur cette planète (ou sur la plus proche de `np`, éventuellement le `pm` lui-même, qui est dans l'ensemble admis). La valeur observée est figée ; l'aller-retour par nom avec `pm`/`fv` en valeur est, lui, `assert`é.
- **H15** : `pico8_jc_hq` sur un `jc` construit par `jo:ldown` d'une planète `ly`, souris sur une autre planète, pose `fo` sur la planète que rend `jc:ce`, ou sur un `jc`/`jc_too_far`. La valeur observée est figée.
- **H16** : sans l'échange de `by`, `family_default_list` avorte à `pico8_planet_family_init`, et `test_pico8_ga_export_panel_by` est rouge (`SIGABRT`). Avec l'échange, il est vert.
- **H17** : les 14 autres tests de régression, et `rich_planet_exports`/`planet_kj_dispatch`, passent sans modification.
- **H18** : `pico8_bv_dispatch` et `pico8_jc_struct_class` se lient dans tout binaire qui lie `planet.c` (`ENGINE_SRCS`, `DATA_ENGINE_SRCS`, `main_raylib` listent `probe.c` et `slipway.c`). Aucune cible de TU ne lie `planet.c` par une liste propre.
- **H19** : `test_pico8_jo_spec_defaults`, `test_pico8_spec_defaults` et `test_pico8_empty_planet_cleared` plient ou atteignent ces membres par `hv` ou par la spec (compte de l'empreinte seulement, sans effet sur le diff).
- **H20** : les lignes modifiées de `nl:ly` et de `hj:kj` (branche `mi ≥ 2`) ne sont peut-être pas atteintes par `test_pico8_ui`. Elles le sont par **une** passe `gcov` de `test_pico8_nl` (TU rapide, `ly` à `mi` = 2 puis 3, `nl:452-455`).
- **H21** : les deux extraits C1 réécrits donnent, par site, la même valeur ancienne = nouvelle dans `make float-audit`.
- **H22** : `jo:hk` sur une planète visible (`gd`) à `kr` ≠ 0 construit un `iv` de plus dans `hv`, et n'en construit aucun à `kr` absent ou nul.

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom, ou l'écran.** Les nouveaux tests n'accèdent à aucun des 7 membres directement : ils compilent sur les deux formes. Aucune ligne directe de test n'est touchée (revue §6).
- **Caractérisation dirigée, figée sur l'ancien code** : `test_jo_writers_by_name`, dans `test_pico8_ui`. Ce TU est rapide, dans `CENSUS_UNIT_TESTS`, lie `ENGINE_SRCS`, et a `start_game`, `play_frame`, `must_abort`, `in_fresh_child` ; précédents 1b.14a et 1b.23a–c. Elle est lancée par `in_fresh_child` après les tests de `jo` de 1b.23c. Elle lit par le **lecteur brut du descripteur** (`cls->field`, témoin `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder (retour, genre, valeur, pointeur). C'est une fonction distincte, avec un hash propre.
  1. **Planètes vivantes au départ** : `start_game(DRAG)`, puis parcours de `pico8_hv()`. Pour chaque entité de l'arbre `jo`, on plie :
     - présence et valeur de `ma`, `kr`, `mi` (fixe) et `m_` (booléen) ;
     - présence et compte de `cc` et `by` ;
     - présence de `fo` et la classe de son porteur (index dans une liste fixe des 11 classes admises, `assert` qu'il y est) ;
     - le nombre d'entrées.
  2. **Les 10 genres, neufs.** `pico8_<X>_new(NULL)` pour `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx` :
     - `ma` est `FIXED` ;
     - `kr`, `cc`, `fo`, `m_` sont absents (témoin intact) ;
     - `mi` et `by` présents ou absents, valeur observée (H12) ;
     - sur `ga`, `by` est le **même pointeur** que `pico8_ga_default("by")` (`assert`), et deux `ga` neufs partagent le même `by`.
  3. **Écrivain par nom dans le genre**, sur `jo`, `qh`, `pm`, `ga`, `nr`, `fr` (chaque chaîne de délégation) :

     | membre | écritures | lecture attendue |
     |---|---|---|
     | `ma` | 2 | `FIXED` 2 |
     | `kr`, `mi` | 3 ; **fixe 0 à union nulle** ; nil | présent 3 ; présent 0 ; absent |
     | `m_` | `true` ; `false` ; **faux à union nulle** ; nil | vrai ; `BOOL` 0 présent ; `BOOL` 0 présent ; absent |
     | `cc`, `by` | liste L ; nil | même L ; absent |
     | `fo` | chacun des 11 porteurs admis, un neuf de chaque classe (`g_`, `fv`, `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`, `jc`) ; nil | même pointeur ; absent |

     `cls->set_field` rend 1 à chaque écriture, et `id` rend 0 (ensemble des noms inchangé).
  4. **Les écrivains génériques, par aller-retour** :
     - `pico8_hy_lj(pico8_fm_hy())` → `kr` présent sur chaque `cg.jo`, valeur observée ;
     - `pico8_bd()` → `cc` présent, une **nouvelle** liste (pointeur différent d'avant, `assert`) ; `by` tel qu'observé ; sur un `ga`, `by` reste le pointeur de spec ;
     - `pico8_bb()` → `mi` observé ;
     - `pico8_ui_gw` avec la souris sur une planète, puis ailleurs → `m_` vrai puis faux **présent** ;
     - `pico8_jo_kd(e)` → `ma` = 1,25, puis `pico8_ma(p, e)` → `ma` = 1,15, et à 0 après assez d'appels (valeurs observées) ;
     - `pico8_bv(qh, mp = ra d'une planète P, rp)` → `P.fo` = `qh` si observé ;
     - `pico8_jc_hq` (H15) → `fo` de la cible de `jc:ce` = le `jc` si observé ;
     - puis `jo:kj` sur P → `fo` absent.
  5. **Producteurs hors ligne (1b.22)** : `mv(pm_neuf, "hq", ra de P, rp)` et `mv(fv_neuf, "hq", …)` → `fo` observé (H14) ; puis `set_field(P, "fo", pm)` et `set_field(P, "fo", fv)` → même pointeur (`assert`). C'est la preuve sur l'ancien code que `pm` et `fv` sont admis.
  6. **Transitions** :
     - `jo:hk` sur une planète visible à `kr` = 5 posé par nom → un `iv` de plus dans `hv` ; à `kr` nil ou 0 → aucun (H22) ;
     - `jo:ib` sur un `jo` à `by` nil → liste fraîche, et le membre reste absent ;
     - sur un `ga` → le pointeur de spec.
  7. **Planètes vivantes après le rejeu** : on joue toutes les frames du rejeu `DRAG` (`play_frame`), puis on replie le parcours de 1.
  8. **Intrus** sur le lecteur de `jo` : `""`, `"m"`, `"ma "`, `"M_"`, `"fo_"`, `"has_kr"`, `"has_mi"`, `"has_m_"`, `"kr_"`, `"cc "` → 0, témoin intact.
  9. Tout est plié dans **`JO_WRITERS_HASH`**, relevé au premier passage vert sur l'arbre de départ, puis inchangé. Les adresses sont jugées par `assert`, jamais pliées.
- **Test d'arrêt, rouge d'abord** (1b.13/1b.15/1b.22) : `test_jo_writers_stop_off_row`, par `in_fresh_child` après la caractérisation. `must_abort` sur `pico8_entity_set_field(e, n, v)`, `e` un `jo` puis un `pm` neufs (délégation `pm` → `hj` → `qh` → `jo`), puis un `ga` (écrivain propre qui délègue) :

  | nom | valeurs hors genre (toutes à union nulle par `memset` puis `kind`, sauf mention) |
  |---|---|
  | `ma` | **nil** (`REF NULL`), `true`, **faux à union nulle**, chaîne `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table` |
  | `kr`, `mi` | `true`, **faux à union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table`, REF `pico8_hv()` |
  | `m_` | fixe 1, **fixe 0 à union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table` |
  | `cc`, `by` | fixe 1, **fixe 0 à union nulle**, **faux à union nulle**, **chaîne NULL**, **fonction NULL**, REF `ob_table` (la forme même du bug de `ga_export_panel_by`), REF vecteur (tag `UNKNOWN`), REF `e` (struct) |
  | `fo`, non-structs | fixe 1, **fixe 0 à union nulle**, `true`, **faux à union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table`, REF `pico8_hv()` (liste), REF vecteur |
  | `fo`, **structs refusées** | **un `mz` neuf** (dans l'ensemble de `bv_et.fo`, pas dans celui-ci), **un `jo` nu**, **un `ga` neuf**, **un `hp` de `gb`**, **le `bg`** (`hq` = `bg_hq_dispatch`), **l'`ok`** (`ui`) |

  Sont **exclus** les cas dans le genre : `false` et faux à union nulle pour `m_`, fixe 0 à union nulle pour `kr`/`mi`/`ma`, les 11 porteurs admis pour `fo`.

  **Rouge sur l'ancien code** (la boucle `written[]` recopie tout, le fils sort par `_exit(0)`, H11), vert après : une ligne « Runs » pour chacun. Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Tests existants qui figent déjà beaucoup** :
  - les TU lents : `fold_live`, `has(e, "m_")`, `fold_field(e, "mi")`, `list_size(e, "by"/"cc")` ;
  - `test_pico8_ga` (`:380` identité de `by`, `:551` partage, `on_jo[]`, `!has(b, "kr")`) ;
  - `test_pico8_jc` (`fo` des planètes : `DRAW_HASH`, `holder_kind`, `:707`) ;
  - `test_pico8_g_` (`fo` posé par une sonde) ;
  - `test_pico8_hy` (`kr`), `test_pico8_trade` (`by`, `cc`), `test_pico8_hp` (`ma`) ;
  - `HOVER_HASH` de `test_pico8_ui` (`m_` d'une planète : `bool_state` 2 puis 1) ;
  - les trois tests de régression ; `DRAG_REPLAY_HASH` et `TECH_REPLAY_HASH` (écran).
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_ui && ./tmp/test_pico8_ui` (ou le TU cité). Chacune est notée dans le commit avec le test qui la rougit, ou avec la justification de son trou. Un appel unique d'aide `static` se retire sous `if (0)` (1b.14b).
  1. **(§7-1) *indicateur forcé vrai*, dans sa lettre** : la branche R4 de `m_` ignore `has_m_`. Attendu rouge : caractérisation 2 (`m_` présent faux au lieu d'absent). `has(e, "m_")` des TU lents est noté sans relance.
  2. **(§7-2) *« présent 0 » là où c'était absent*, dans sa lettre** : la branche de `kr` rend `pico8_value_fixed(0)` quand `has_kr` = 0. Attendu rouge : caractérisation 2 et 3 (nil → absent). `!has(b, "kr")` de `test_pico8_ga` est noté.
  3. **(§7-3) *init qui saute un membre typé***. Plus aucune boucle d'init de `jo` ne reste. On prend donc la forme « pose une autre valeur », parce que l'omission pure laisse de la mémoire de `malloc` non déterministe (**trou justifié** de la forme littérale, précédent 1b.14a) :
     - (a) `pico8_jo_struct_init` pose `has_mi = 1` → rouge : caractérisation 2 (`jo`, `ga`) ;
     - (b) la pose `has_mi = 1` de `pico8_qh_struct_init` sous `if (0)` → rouge : caractérisation 2 (8 genres) ; plis `mi` des TU lents notés ;
     - (c) `pico8_ga_struct_new` pose `by = NULL` → rouge : caractérisation 2, `test_pico8_ga`, `ga_export_panel_by` ;
     - (d) `pico8_jo_struct_init` pose `ma = pico8_fixed_from_int(1)` → rouge : caractérisation 1 et 2, `STRUCT_HASH` ;
     - (e) `pico8_jo_struct_init` pose `has_kr = 1` → rouge : caractérisation 2 ;
     - (f) `pico8_jo_struct_init` pose `fo = &self->ke.header` (une struct `jo`) → rouge : caractérisation 2.
  4. Propre à l'étape, *identité perdue* : `pico8_planet_family_init` pose dans `g_ga_by` une copie neuve de la liste de la spec. Attendu rouge : caractérisation 2 (`assert` de pointeur), `test_pico8_ga:380`.
  5. Propre à l'étape, *aide sans `assert`* : `jo_take_holder` recopie toute REF. Attendu rouge : test d'arrêt.
  6. Propre à l'étape, *prédicat de `bv_et`* : `jo_is_fo_holder_class` devient celui de `bv_et` (`|| cls == pico8_mz_struct_class()` à la place de `jc`). Attendu rouge : test d'arrêt (cas `mz`) **et** caractérisation 3 et 4 (le `jc` porteur avorte). Cette mutation prouve que les deux ensembles diffèrent dans les deux sens.
  7. Propre à l'étape, *`ma` accepte nil* : `jo_take_ma` pose 0 sur `REF NULL`. Attendu rouge : test d'arrêt (cas nil).
  8. Propre à l'étape, *écrivain muet* : la branche `m_` rend 1 sans écrire. Attendu rouge : caractérisation 3 et 4 (`gw`), `HOVER_HASH`.
  9. Propre à l'étape, *`kj` garde `fo`* : `jo:kj` omet `self->fo = NULL`. Attendu rouge : caractérisation 4 ; `test_pico8_jc` (`!has(origin, "fo")`) noté.
  10. Propre à l'étape, *`hk` à l'envers* : `!self->has_kr` → `self->has_kr`. Attendu rouge : caractérisation 6 (H22).
  11. Propre à l'étape, *`ib` perd `by`* : `jo:ib` prend toujours une liste fraîche. Attendu rouge : caractérisation 6 (`ga`) et `ga_export_panel_by` (« 2/2 »).
  12. Propre à l'étape, *`nl:ly` sans garde* : la condition `has_mi &&` retirée de `nl:ly`. **Trou justifié** : `mi` est toujours présent sur un `nl` (défaut de `qh`, jamais écrit à nil hors TU), donc la mutation est invisible par construction ; l'`assert` de tête garde la garde.
- **`gcov`** : `test_pico8_ui` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23, sur les 4 fichiers `.c` modifiés. Chaque ligne modifiée doit être exécutée. Une ligne manquée (probablement `nl:ly` et la branche `mi ≥ 2` de `hj:kj`, H20) est cherchée dans **une** passe `gcov` de `test_pico8_nl`, sinon justifiée dans le commit. Trous attendus : l'échec des `assert` des aides d'écriture (fils de `must_abort` qui avortent sans vider leur compteur, prouvés par le test d'arrêt et les mutations 5–7) et l'échec de `family_default_list`/`jo_default_fixed` (inatteignable, la spec est un littéral, sauf dans la preuve R-a).
- **Prior art** :
  - `test_jo_members_by_name`, `test_jo_written_by_name`, `test_jo_writer_stops_off_row`, `test_jo_position_*`, `test_jo_state_*` (même fichier) ;
  - le test d'arrêt de `bv_et` (1b.22, `bv_et_take_holder`, cas « liste » par `pico8_hv()`) ;
  - `qb_take_fixed` et son test (1b.19, nil avorte) ;
  - `bg_take_bool` (1b.15) ;
  - le `HOVER` de `test_pico8_ui`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_ui` : 10 à 30 s. Une vingtaine de lancements (départ, arrêt rouge, final, 17 mutations, `gcov`) : ~8 min ;
- une passe `gcov` de `test_pico8_nl` : moins d'une minute ;
- preuves §6 : `test_pico8_ga_export_panel_by`, `test_pico8_rich_planet_exports`, `test_pico8_planet_kj_dispatch`, deux lancements chacun : ~1 min ;
- `make test` : 20 s à 1 min, une fois ;
- `make e2e` : 4 à 6 min, une fois ;
- `make class-coverage-net` : 3 à 4 min, **dans la même invocation** que le `make e2e` (arbres distincts, 1b.0) ;
- `make sanitize-net` : ~4 min seul, **jamais** en même temps que `e2e`, une fois sur l'arbre final ;
- `make value-members` : quelques secondes.

**TU lents touchés** : `test_pico8_jo` (tableau `written[]` et sa boucle supprimés), `test_pico8_qh`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`, `test_pico8_nr` (tableaux `written[]` réduits). `test_pico8_ie` n'est pas modifié (ses `m_` sont homonymes), mais il est lancé par `make e2e`. **Construire les huit tôt**, juste après le typage et avant toute mutation (`make -j tmp/test_pico8_jo tmp/test_pico8_qh tmp/test_pico8_os tmp/test_pico8_hj tmp/test_pico8_pm tmp/test_pico8_mx tmp/test_pico8_nr tmp/test_pico8_ie`, compilation seule). Ils ne sont lancés que par `make e2e`.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre (départ / final / mutation n / §6 R-x bug remis / §6 R-x restauré) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` vert, et les **8** TU de `UNIT_TESTS_SLOW` passent (`test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 138`**, aucune ligne `pico8_jo_t` ;
5. `timeout 15m make class-coverage-net` : **sortie identique à `tools/class_coverage_net_expected.txt`**. Elle s'applique parce que le diff touche des constructeurs (`pico8_jo_struct_init`, `pico8_qh_struct_init`, `pico8_ga_struct_new`), sans toucher `pico8_class_coverage_record` ni `KNOWN[]`. Aucun compte ne change.

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS` ni un fichier d'événements.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

**Départ et filet**

- `FAILED:` si 1b.23c n'est pas commitée au départ, ou si `make value-members` ne rend pas 145 sur l'arbre de départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.

**Types, tables et écrivain**

- `FAILED:` (« champ sans règle ») dans chacun de ces cas :
  - une aide d'écriture ou de lecture de spec avorte dans un rejeu ou dans un TU hors des fils de `must_abort` (H2, H3, H4, H5) ;
  - un type est élargi (union, `void *`, `pico8_value_t` gardé à côté, prédicat étendu à `mz` ou à une autre classe) au lieu d'avorter ;
  - un `has_ma` est ajouté pour accepter nil.
- `FAILED:` si l'un des 7 membres n'a pas le type du tableau de recensement à sa place dans `pico8_jo_t`, ou si `has_kr`, `has_mi`, `has_m_` manque ou n'est pas juste après son membre.
- `FAILED:` si `pico8_jo_t` garde un membre `pico8_value_t`, si `g_jo_members`, `g_jo_defaults` ou `pico8_jo_member` existe encore, ou si `offsetof(pico8_jo_t` apparaît dans `src/`.
- `FAILED:` si `g_qh_defaults` liste encore `mi` ou `g_ga_defaults` `by`, ou si une autre de leurs entrées est retirée ou modifiée.
- `FAILED:` si le défaut `by` de `ga` n'est pas le **pointeur de la spec** (ni copie, ni liste module), si `ma`/`mi`/`by` ne sont pas remis à nil dans leur `defaults_reset`, ou si une autre fille pose `mi`/`by`/`ma`.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact pour un pointeur NULL ou un `has_` à 0, ne rend pas le même pointeur, ou rend un autre genre que 1a (`m_` faux doit rester `BOOL` 0 présent, `ma` toujours `FIXED`).
- `FAILED:` si l'écrivain n'accepte plus exactement les 17 mêmes noms (dont `id` refusé), ou si une boucle de clés change.
- `FAILED:` si une aide viole la forme de 1b.13/1b.15 :
  - elle ne pose pas l'état nil d'abord (sauf `jo_take_ma`, qui avorte sur nil) ;
  - elle n'avorte pas sur un hors-genre à union nulle ;
  - son message ne nomme pas `jo.<m>` et le genre de la ligne ;
  - elle est partagée entre fichiers ou rendue publique.
- `FAILED:` si le prédicat de `fo` n'admet pas exactement les 10 porteurs de `pico8_bv_dispatch` et `jc`, admet `mz`, ou est une liste d'accesseurs qui ne se lie pas partout où `planet.c` est lié.

**Diff et preuves**

- `FAILED:` si un site par nom de `src/` hors `pico8_planet*` change, ou si un site par nom est remplacé par un accès direct.
- `FAILED:` si `tools/float_audit_c1.txt` change ailleurs que sur les deux lignes C1 de `mi`, ou si `make float-audit` n'est pas vert.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si le diff d'un test fait autre chose que ceci, ou si une valeur figée existante change (H6) :
  - ajouter la caractérisation, le test d'arrêt, `JO_WRITERS_HASH` et leurs appels à `test_pico8_ui` ;
  - retirer les 7 noms des 9 tableaux `written[]` ;
  - supprimer le tableau `written[]` de `test_pico8_jo` avec sa boucle ;
  - changer la phrase de commentaire de `test_pico8_ga_export_panel_by`.
- `FAILED:` s'il manque, pour chacun de R-a, R-b, R-c, les deux lignes « Runs » (rouge avec le bug remis, vert restauré), ou si un test de régression change dans son intention.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 138, s'il liste encore `pico8_jo_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` s'il manque une ligne « Runs » de `make class-coverage-net` identique à la référence, ou si le diff touche `pico8_class_coverage_record` ou `KNOWN[]`.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.

**Tests de l'étape**

- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, ou si elle lit ou écrit l'un des 7 membres directement.
- `FAILED:` si elle ne couvre pas les 10 genres, les 7 noms, les 7 écrivains génériques, les 11 porteurs admis de `fo` (dont `pm` et `fv` par `hq`), ou si `JO_WRITERS_HASH` change après.
- `FAILED:` si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'arbre de départ et une verte sur l'arbre final, ou s'il n'a pas les cas à union nulle, nil pour `ma`, et les structs refusées pour `fo` (`mz`, `jo` nu, `ga`, `hp`, `bg`, `ok`).
- `FAILED:` si le commit ne contient pas son rapport `gcov` et ses mutations (1, 2, 3a–3f, 4 à 12), chacune avec son test ou la justification de son trou.

**Commit et note**

- `FAILED:` si le travail n'est pas en un commit vert, ou si un membre d'une fille (`qh.et`, `ga.qy`…) y est typé.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque le tableau « Runs », ou la note « Découvert à 1b.23d », ou si la note ne porte pas les lectures nouvelles :
  - `fo` : ensemble admis différent de `bv_et.fo`, `mz` exclue par preuve de code, `pm`/`fv` admis ;
  - suppression de `g_jo_members`/`g_jo_defaults`, forme 1b.9 fermée pour une table à filles ;
  - tableau sentinelle vidé supprimé ;
  - `jo_take_ma` sans état nil.

## Out of Scope

- Les membres propres des filles (`qh.et` et `qh`, `ga.qy`, `nr.hz`, `mx.ht`…) : 1b.24 et suivantes. Les tables `g_qh_members`, `g_ga_members` et les tables de défauts des filles, qui gardent leurs autres entrées.
- Les homonymes `m_` (`bg`, `fq`, `js`, `ie`, `fv`), `ma` (`qb`), `fo` (`jc`, `bv_et`, `jc_too_far`) : leurs étapes.
- Réunir les deux prédicats de porteur (`bv_et`, `jo`) en une aide commune : interdit par 1b.13 point 2, et ensembles différents.
- Remplacer un site par nom (`economy*.c`, `entity_geom.c`, `ui.c`, `probe.c`, `slipway.c`) par un accès direct ; ajouter ou retirer un nom d'un écrivain.
- Retirer l'échange `o()` → liste de `ga` (la spec `ob` est la phase D).
- Régénérer `tools/member_census.txt`.
- Les fichiers d'événements et `FRAMES_EVENTS`, tout changement de rendu, l'audio, lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - **modules** :
    - `planet.[ch]` : types, `has_`, lecteur, écrivain, init, `kd`, `hk`, aides `jo_take_ma`, `jo_take_holder`, `jo_is_fo_holder_class`, messages étendus, suppression des deux tables, de `pico8_jo_member` et de `pico8_jo_present` ;
    - `planet_display.c` : `hb`, `gh`, `kj`, suppression de `list_of` et `fixed_of` ;
    - `planet_trade.c` : `ib` ;
    - `planet_family.c` : `g_qh_mi`, `g_ga_by`, `family_default_list`, `nl:ly`, `hj:kj`, suppression de `pico8_pf_truthy` ;
    - `tools/float_audit_c1.txt` : deux extraits ;
  - **types** : `cc`, `by` → `pico8_list_t *` ; `fo` → `pico8_struct_header_t *` ; `ma` → `pico8_fixed_t` sans indicateur ; `kr`, `mi` → `pico8_fixed_t` + `has_` ; `m_` → `int` + `has_` ;
  - **`fo`** : ensemble = porteurs de `pico8_bv_dispatch` ∪ `jc`, prédicat par propriété de descripteur plus un accesseur, `mz` exclue ;
  - **tests** :
    - `test_pico8_ui` étendu (caractérisation des 7 membres sur les 10 genres, les 7 écrivains génériques et les 11 porteurs ; test d'arrêt rouge d'abord) ;
    - 9 tableaux `written[]` réduits, celui de `test_pico8_jo` supprimé ;
    - une phrase de `ga_export_panel_by` ;
    - pas de nouveau TU ;
  - **pas de découpage** : 17 fichiers, sous 1b.23a2. Les moitiés naturelles partagent tous leurs fichiers ;
  - **`class-coverage-net`** lancé une fois, sur l'arbre final.
- **Note « Découvert à 1b.23d »**, à écrire sur la ligne de l'étape. Le reste va dans le commit :

  > patron 1b.1 appliqué ; `cc`, `by` → `pico8_list_t *`, `kr`, `mi` → `pico8_fixed_t` + `has_`, `m_` → `int` + `has_`, `ma` → `pico8_fixed_t` sans indicateur (**`jo_take_ma`, nil avorte**, forme 1b.19) ; **`fo` → `pico8_struct_header_t *` (1b.22), ensemble admis = porteurs de `pico8_bv_dispatch` ∪ `jc`, différent de celui de `bv_et.fo` (`mz` exclue : `mz.dw` = 0, jamais une planète ; `jc` admis par `jc:hq`), `pm`/`fv` hors ligne admis par preuve de code et caractérisation** ; défauts `ma` (`g_jo_ma`), `mi` (`g_qh_mi`, table de `qh`), `by` (`g_ga_by`, pointeur de spec partagé) par R7 de 1b.14a ; **`g_jo_members`, `g_jo_defaults`, `pico8_jo_member` supprimés : `pico8_jo_t` sans `pico8_value_t`, forme 1b.9 fermée pour une table à filles** ; `list_of`, `fixed_of`, `pico8_jo_present`, `pico8_pf_truthy` supprimés (`-Werror`) ; deux extraits C1 réécrits (1b.21) ; boucles sentinelles de 1b.23a2 recopiées, **un tableau `written[]` vidé est supprimé avec sa boucle** (`test_pico8_jo`) ; §6 : aucun accès direct, trois tests liés par nom ou par chemin, prouvés rouges ; empreinte réelle ≈ 34 (12, 22), 17 fichiers modifiés.
- Le message de commit porte :
  - le périmètre et le compte ;
  - le tableau des poses (R3) et des branches d'écrivain ;
  - l'ensemble de `fo` avec sa preuve ;
  - le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous ;
  - `JO_WRITERS_HASH` et les réponses à H1–H22.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
