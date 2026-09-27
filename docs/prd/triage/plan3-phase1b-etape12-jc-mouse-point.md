---
title: "Plan 3, étape 1b.12 : `jc_mouse_point` (le `{ra,p}` de `jc:hq`) — `ra` et `p` deviennent des `pico8_vec_t *`, `ra` emprunté à `mp`, toujours alloué par appel"
description: "`pico8_jc_mouse_point_t` a deux membres `pico8_value_t` lus par nom seul (`ra` : `ref:autre`, nil observé `lu` ; `p` : `ref:autre`, nil jamais). Les typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 213 à 211. Deux pointeurs, donc aucun `has_`. Pas d'écrivain par nom : la moitié écrivain de R5 reste vacante. R7 prend la forme de 1b.8 (sélecteur local retiré)."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.12 — `jc_mouse_point` (`slipway.c:77`), accès par nom seul, cible de `jc.re`. Empreinte 21 (11, 10). Compte : 211.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité ici, pas redécidé. S'appliquent aussi, sans être redérivées :

- « à 1b.3 » : R3 se lit sur ce qui initialise l'objet, ici la branche « rien de survolable » de `pico8_jc_hq` ;
- « à 1b.4 » : `ref:autre` vers un vecteur → `pico8_vec_t *` emprunté, même pointeur ;
- « à 1b.6 » : un lecteur qui copie n'est pas un `pico8_X_member` ;
- « à 1b.8 » : R7 s'applique à un sélecteur `pico8_value_t *` local au lecteur (retiré, branches R4 explicites) ; première étape à écrivain par nom : 1b.13 ;
- « à 1b.10 » : lecture de R5 pour `ref:autre` (REF non NULL dont `pico8_type_tag_of` rend `UNKNOWN`, sinon `assert`) ;
- « à 1b.11 » (PRD en triage) : une lecture par nom faite seulement par un TU de `UNIT_TESTS_SLOW` compte pour R2.

### État de départ

HEAD 919efc4. **Précondition** : 1b.11 (`ie_ke`) est commitée avant que 1b.12 commence. Elle ne touche que `research_icon.[ch]` et `test_pico8_entity_kind.c`, aucun fichier de cette étape. `make value-members` rend alors 213 (H1).

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H9).

### Lignes de recensement (`tools/member_census.txt:203-204`)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `ra` | nom | `ref:autre` | observé | lu,stat? | l |
| `p` | nom | `ref:autre` | jamais | - | l |

Deux membres : 213 − 2 = **211**. Aucun `e` : pas d'écrivain. La ligne `jc.re` (`:215`) cite `ref:pico8_jc_mouse_point_t` : c'est le membre de `jc` (1b.35), pas l'objet de l'étape.

**Revue des TU lents (lecture de R2 de 1b.11).** Aucun des huit TU de `UNIT_TESTS_SLOW` (`ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`) ne nomme la struct. Aucun ne lit `ra` ou `p` d'un `{ra,p}` par nom : leurs `call(…, "hq", …)` visent une planète, et `mx` ne fait que `jc:ey` vers une planète. Le seul nil lu par nom, `ra` pour un `mp` nil (`test_mouse_point_is_a_struct`), est dans `test_pico8_jc`, qui est dans `CENSUS_UNIT_TESTS` : la ligne le voit déjà (`lu`). Les deux membres sont des pointeurs, donc **R2 ne donne aucun indicateur**, avec ou sans cette lecture.

### Ce que fait le cart

`function jc:hq(mp)self.re=ok.o_.gv and ok.o_ or {ra=mp,p=mp/128}` (`pico8/lua.lua:622`). `{ra,p}` n'a ni méthode ni écrivain.

### Le code aujourd'hui (`src/pico8_slipway.c`)

- `:75-79` : le contrat « a fresh one per call, never freed, nor its `p`; `ra` is the caller's `mp` itself, not a copy », puis `{ header; pico8_value_t ra; pico8_value_t p; }`.
- Lecteur `pico8_jc_mouse_point_field` (`:81-97`) : un sélecteur local `const pico8_value_t *member`, puis `REF NULL` → 0, sinon copie.
- Descripteur (`:99-103`) : aucune méthode, `set_field` **NULL**.
- Construction, dans `pico8_jc_hq` (`:187-197`) : `malloc` de la struct et d'un `p_heap` ; `point->ra = mp` ; `*p_heap = mp/128` par `pico8_jc_value_vec` (zéro si `mp` n'est pas une REF) ; `point->p = REF(p_heap)` ; `PICO8_MEMBER_CENSUS_SEE` ; `re_val = REF(point)`.
- Seul appelant réel : `ui:bh` (`pico8_ui.c:466`) passe `ui.qa`, un vecteur neuf par frame, ou `REF NULL` avant le premier `ui:lj` (`pico8_ui.c:182`). Les tests passent `vec_ref(…)` ou `REF NULL`.
- Lecteurs par nom, **inchangés** : `gd` (`entity_geom.c`), `lg` et `ch` (`slipway_display.c`), `dj` (`economy_formulas.c:165`), `ce` et son prédicat (`slipway.c`), le prédicat `jc` de `bv` (`probe.c:161-162`).
- Tests : `test_pico8_jc` seul (`is_throwaway` par le descripteur, `test_hq`, `test_too_far`, `test_hq_mouse_point`, `test_mouse_point_is_a_struct` avec `must_abort(write_ra/write_p/write_fo)`).

### Empreinte remesurée

La ligne dit 21 (11, 10). `grep -rlE '"(ra|p)"'` rend 25 fichiers de `src` et 32 de `tests` : presque tout vient d'homonymes (`jo.ra`, `g_.p`, `je.ra`…). **Empreinte réelle** (fichiers qui lisent ou écrivent un membre d'un `pico8_jc_mouse_point_t`) : **6 (5, 1)**. En `src`, `slipway.c` y accède directement et `entity_geom.c`, `slipway_display.c`, `economy_formulas.c`, `probe.c` par nom. En tests, `test_pico8_jc.c`. `grep -rlE 'mouse_point' src tests` rend `slipway.[ch]` et `test_pico8_jc.c`.

**Fichiers modifiés : 2**, plus `docs/plan-3.md` : `src/pico8_slipway.c` et `tests/test_pico8_jc.c` (pur ajout). **Aucun TU de `UNIT_TESTS_SLOW` n'est touché.** Ils ne l'atteignent que par les rejeux, et `make e2e` les lance.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (voir Testing Decisions).
2. **Typage** : `pico8_vec_t *ra; pico8_vec_t *p;` (en-tête, noms et ordre gardés). `ra` est **emprunté** (le vecteur de `mp`, même pointeur, jamais libéré ici). `p` est le `p_heap` de l'appel (TROU-7, inchangé). Toujours une struct neuve par appel : l'identité de `jc.re` ne change pas.
3. Effet mesurable : `make value-members` passe de 213 à **211** et ne liste plus aucune ligne `pico8_jc_mouse_point_t`. Aucun pixel ne bouge.

## User Stories

1. En tant que lecteur de `slipway.c`, je veux `ra` et `p` typés `pico8_vec_t *`, afin que leur genre se lise sur le type.
2. En tant que `gd`, `lg`, `ch`, `dj`, `ce` et le prédicat de `bv`, je veux lire `ra` et `p` par nom avec le même genre (`REF`) et le **même pointeur** qu'en 1a.
3. En tant que `jc.re`, je veux un `{ra,p}` neuf et distinct à chaque `hq` au-dessus du vide, afin que `TOO_FAR_HASH` et `MOUSE_POINT_HASH` restent vrais.
4. En tant que `test_hq_mouse_point`, je veux `ra` égal par adresse au `mp` reçu, et `p` à son propre vecteur, gardé après un `hq` suivant.
5. En tant que `test_mouse_point_is_a_struct`, je veux `ra` absent pour un `mp` nil, `p` égal à (0, 0), et toute écriture par nom qui avorte.
6. En tant que lecteur par nom, je veux 0 avec `*out_value` intact pour un membre NULL et pour tout autre nom.
7. En tant que mainteneur, je veux qu'un `mp` qui n'est ni nil ni une REF de vecteur arrête l'exécution par un `assert` « champ sans règle ».
8. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
9. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
10. En tant que `plan-pilot`, je veux `make value-members` à 211.
11. En tant qu'auteur de 1b.13 et de 1b.35, je veux savoir que la moitié écrivain de R5 reste vacante et que `jc.re` n'a pas changé.
12. En tant que vérificateur, je veux une caractérisation par nom lancée sur l'ancien code, les mutations du §7 avec leurs trous justifiés, et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés comme le ferait un agent `guideline`, et marqués (G).

### R1–R9 appliquées à `jc_mouse_point`

| règle | sur `jc_mouse_point` |
|---|---|
| R1 | `ra`, `p` → `pico8_vec_t *` (lecture de 1b.4). |
| R2 | Aucun indicateur : deux pointeurs, NULL = nil. La revue des TU lents n'ajoute rien (voir plus haut). |
| R3 | La branche de `pico8_jc_hq` pose les deux membres par affectation typée, dans l'ordre d'aujourd'hui : `ra` par l'aide R5 ci-dessous, puis `p = p_heap`. La ligne `*p_heap = pico8_vec_div(pico8_jc_value_vec(mp), …)` ne change pas. |
| R4 | Le sélecteur local disparaît. Il y a une branche `strcmp` par membre, dans l'ordre du typedef : NULL → `return 0`, sinon `pico8_value_ref(<m>)`, même pointeur. Tout autre nom → `return 0`. Dans les deux cas, `*out_value` reste intact. |
| R5 | **Source de `ra` : l'argument `mp`**. C'est la lecture `ref:autre` de 1b.10, avec une source différente (un argument au lieu d'une clé), ce qui n'est pas une lecture nouvelle (G). Une aide `static` : `REF NULL` → NULL ; REF non NULL dont `pico8_type_tag_of` rend `UNKNOWN` → le pointeur ; tout autre cas → `assert` « champ sans règle », au format de `layout.c:460` : `"jc_mouse_point.ra: an mp neither nil nor a vector ref is off its census row (ref:autre) -- a field under no rule of plan-3 section 5: stop, do not widen the member"`. `p` ne vient d'aucune source par nom. **Moitié écrivain : vacante, confirmée** (voir plus bas). |
| R6 | Aucune lecture directe hors du lecteur. Seules les deux affectations de la construction changent. |
| R7 | **Forme de 1b.8** : le sélecteur `const pico8_value_t *member` est retiré. Il n'y a ni `offsetof` ni `pico8_X_member`. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(point, pico8_jc_mouse_point_t)` gardé ; `tools/member_census.txt` non édité. |
| R9 | 213 → 211, aucune ligne `pico8_jc_mouse_point_t`. |

### R5, moitié écrivain : vacante

- `set_field` est NULL (`slipway.c:99-103`), et aucune ligne n'a `e`.
- `test_mouse_point_is_a_struct` le fige par `must_abort(write_ra/write_p/write_fo)`.
- Aucun site de `src/` n'écrit un nom sur `re` : `jc:hq` et `bv` écrivent `fo` sur `et`, jamais sur `re`.
- (G) Aucun écrivain n'est ajouté. **Première candidate : 1b.13** (`jc_too_far.fo`, `pico8_jc_too_far_set_field` dans le même fichier).

### Identité des pointeurs

- `jc.re` : une REF vers une struct `malloc`ée à chaque appel, comme aujourd'hui.
- `ra` : `mp.as.ref`, par adresse ; il n'est jamais copié, dupliqué ni libéré.
- `p` : un vecteur par appel, distinct de `ra`.

Le contrat du typedef (`:75-76`) reste exact et n'est pas retouché. Aucun autre commentaire n'est ajouté ; l'aide porte son contrat par son `assert`.

### Preuves

Elles sont toutes vides après l'étape, sauf mention :

- `grep -nE 'point->(ra|p) = |member = &point' src/pico8_slipway.c` : 4 lignes avant l'étape, 2 après (les affectations typées : `ra` par l'aide, `p = p_heap`), et plus aucun `member` ;
- `grep -rnE '(point|mouse_point)[^;]*(ra|p)\.(kind|as)' src/ tests/` : vide ;
- `grep -rn 'offsetof(pico8_jc_mouse_point_t' src/` : vide avant comme après ;
- `grep -n 'pico8_value_t \*' src/pico8_slipway.c | grep -v 'pico8_value_t \*out_value)' | grep -v 'pico8_value_t \*out)'` : avant l'étape, `:83` (le sélecteur) et `:478` (lecteur de **`jc`**, 1b.35) ; après, **`:478` seule**.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 213 sur l'arbre de départ (1b.11 commitée), puis 211.
- **H2** : `mp` n'est jamais autre chose que `REF NULL` ou une REF de vecteur, que ce soit dans les 15 rejeux ou dans les TU. Sinon, l'aide R5 avorte, ce qui est l'arrêt « champ sans règle ».
- **H3** : `pico8_type_tag_of` rend `UNKNOWN` sur chaque vecteur passé en `mp` (comme en 1b.10 pour `nf.ra`, `jo.ra`).
- **H4** : aucun TU de `UNIT_TESTS_SLOW` ne lit par nom un `{ra,p}`, et encore moins nil (grep : aucun ne nomme la struct ; leurs `hq` visent des planètes).
- **H5** : `HQ_HASH`, `TOO_FAR_HASH`, `MOUSE_POINT_HASH` et les empreintes des rejeux de `test_pico8_jc` ne bougent pas, pas plus que les 15 empreintes du filet.
- **H6** : le build `-DPICO8_MEMBER_CENSUS` compile sur une struct sans `pico8_value_t` (comme après 1b.6). Il n'est pas lancé.
- **H7** : sous ASan+UBSan, rien de nouveau : les deux membres sont posés avant toute lecture, et il n'y a pas de nouvelle allocation.
- **H8** : `test_pico8_jc` reste sous 30 s avec la caractérisation (hors `UNIT_TESTS_SLOW`).
- **H9** : 1b.11, commitée, ne touche ni `slipway.c` ni `test_pico8_jc.c`, et les numéros de ligne cités restent bons. Chaque site est aussi désigné par son contenu.

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom ou à l'écran.** La caractérisation ne lit aucun membre directement et compile sur les deux formes.
- **Caractérisation, figée sur l'ancien code** : `test_mouse_point_members_by_name`, dans `test_pico8_jc` (rapide, dans `CENSUS_UNIT_TESTS`), lancée par `in_fresh_child` après `test_mouse_point_is_a_struct`. Elle lit par le **lecteur brut** (`pico8_jc_mouse_point_struct_class()->field`, témoin : fixe `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder.
  1. `world()`, `a = build_jc(origin)`, `hq_over(a, NULL, m1)` : `ra` → 1, `REF`, adresse `m1.as.ref` ; `p` → 1, `REF`, non NULL, ≠ `m1.as.ref`, valeur `m1/128`, même adresse à une seconde lecture. Les intrus `""`, `"x"`, `"mt"`, `"id"`, `"re"`, `"fo"`, `"header"`, `"has_ra"` → 0, avec le témoin intact.
  2. `hq_over(a, NULL, REF NULL)` : `ra` → 0, avec le témoin intact ; `p` → 1, (0, 0).
  3. Deux `hq` au-dessus du vide (sur `a`, puis sur `b`) : les structs et les `p` sont distincts ; l'ancien `{ra,p}` garde ses adresses et ses valeurs.
  4. On plie le retour, le genre et les valeurs de vecteur de chaque lecture dans **`MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ. Les adresses sont jugées par `assert`, pas pliées.

  Elle est lancée verte sur l'arbre de départ **avant** le typage (ligne « Runs »). Ses valeurs ne changent pas ensuite. Le diff de `test_pico8_jc.c` est un pur ajout (la fonction, son `#define`, son appel).
- **Tests existants, inchangés, qui figent déjà beaucoup** : `test_hq`, `test_too_far`, `test_hq_mouse_point` et `test_mouse_point_is_a_struct`. Aucun test de régression (§6) ne lit le `{ra,p}`.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_jc && ./tmp/test_pico8_jc`. Chacune est notée dans le commit avec le test qui la rougit, ou la justification de son trou.
  1. (§7-1) *indicateur forcé vrai* : il n'y a pas d'indicateur. Forme la plus proche : la branche `ra` du lecteur ignore NULL et rend `REF NULL` présent. Attendu rouge : caractérisation 2 et `test_mouse_point_is_a_struct` (`!has(re, "ra")`).
  2. (§7-2) *« présent 0 » là où c'était absent* : **sans objet dans sa lettre**, puisqu'il n'y a aucun fixe. Forme voisine : l'aide rend un vecteur (0, 0) statique pour un `mp` nil. Attendu rouge : caractérisation 2 et `test_mouse_point_is_a_struct`.
  3. (§7-3) *init qui saute un membre typé* : (a) `p` laissé NULL. Attendu rouge : caractérisation 1, `test_hq` (`HQ_HASH`) et `test_too_far`. (b) `ra` laissé NULL. Attendu rouge : caractérisation 1, `test_hq_mouse_point` et `test_too_far` (extrémité du trait).
  4. Propre à l'étape, *`ra` copié au lieu d'emprunté* : `ra` reçoit un `malloc` de `*mp`. Les pixels ne bougent pas. Attendu rouge : caractérisation 1 (adresse) et `test_hq_mouse_point`.
  5. Propre à l'étape, *`p` partagé* : un vecteur statique au lieu de `p_heap`. Attendu rouge : caractérisation 3, `test_hq_mouse_point` et `TOO_FAR_HASH`.
  - **Trou justifié** : l'`assert` « champ sans règle » de l'aide n'est atteint par aucune entrée (H2). Un `must_abort` le visant avorterait sur le nouveau code, mais pas sur l'ancien : il ne peut donc pas faire partie de la caractérisation figée. (G) Pas de `must_abort` nouveau, comme de 1b.5 à 1b.11.
- **`gcov`** : `test_pico8_jc` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23. Chaque ligne modifiée est exécutée, avec deux trous attendus : la branche `p == NULL` du lecteur (p jamais NULL) et l'échec de l'`assert` de l'aide. Le rapport va dans le commit.
- **Prior art** : `test_cr_members_by_name` (1b.6, lecteur brut et témoin), `test_ha_pz_members_by_name` (1b.4, pointeurs de vecteurs), `test_hq_mouse_point` (1a.40).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_jc` : quelques secondes, soit huit lancements (ancien code, nouveau code, mutations, `gcov`) en 2 à 3 min ;
- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min ;
- `make sanitize-net` : ~4 min seul ; il n'est **jamais** lancé en même temps que `e2e` ;
- `make value-members` : quelques secondes.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent (aucun n'est touché) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 211`**, sans aucune ligne `pico8_jc_mouse_point_t`.

`make class-coverage-net` **ne s'applique pas**, car aucun constructeur ne change. Le `{ra,p}` est construit dans une méthode (`pico8_jc_hq`), sans `pico8_class_coverage_record` et hors de `KNOWN[]`. `pico8_jc_new` n'est pas touché. Si le diff le touche, lui ou `KNOWN[]`, la cible redevient obligatoire, avec une sortie identique à `tools/class_coverage_net_expected.txt`. `make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si l'aide R5 avorte dans un rejeu ou un TU, si un membre (indicateur compris) est ajouté, si un `set_field`, une méthode ou un écrivain est ajouté, ou si le lecteur accepte un autre nom que `ra` et `p`.
- `FAILED:` si `ra` ou `p` n'est pas un `pico8_vec_t *`, si `ra` n'est pas `mp.as.ref` par adresse, ou si le `{ra,p}` ou son `p` cesse d'être alloué par appel.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur un membre NULL ou un nom inconnu.
- `FAILED:` si un vecteur est libéré ou dupliqué, ou si un autre écart port/cart est corrigé au lieu d'être préservé par un `TROU-N` (TROU-7 reste).
- `FAILED:` si un site par nom de `src/` change (`gd`, `lg`, `ch`, `dj`, `ce`, `bv`), si `pico8_jc_hq` change hors de la branche du `{ra,p}`, ou si le lecteur de `jc`, `jc_too_far` ou leurs tables changent.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si le diff d'un test fait autre chose qu'ajouter la caractérisation, son `#define` et son appel, ou si une valeur figée existante change (`HQ_HASH`, `TOO_FAR_HASH`, `MOUSE_POINT_HASH`…), ou si un test de régression (§6) change.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 211, liste encore `pico8_jc_mouse_point_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` si le diff touche un constructeur, `pico8_class_coverage_record` ou `KNOWN[]` sans une ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit un membre directement, ou si `MEMBERS_HASH` change après le typage.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 5 mutations, chacune avec son test ou la justification de son trou.
- `FAILED:` s'il manque la note « Découvert à 1b.12 » ou le tableau « Runs ».

## Out of Scope

- `jc.re` et les autres membres de `jc` (1b.35) ; `jc_too_far` et son écrivain `fo` (1b.13).
- Donner une forme à la moitié écrivain de R5.
- Libérer le `{ra,p}` ou son `p` (TROU-7) ; embarquer `p` dans la struct.
- Remplacer une lecture par nom par un accès direct ; régénérer `tools/member_census.txt`.
- Les fichiers d'événements et `FRAMES_EVENTS` ; tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - module : `slipway.c` seul, avec un commit et sans découpage ;
  - tests : `test_pico8_jc` étendu, sans nouveau TU ;
  - `ra` emprunté à `mp`, par l'aide R5 `ref:autre` de 1b.10 ;
  - aucun `has_` ;
  - R7 dans la forme de 1b.8 ;
  - R5 écrivain vacante ;
  - pas de `must_abort` nouveau ;
  - `class-coverage-net` non lancé.
- **Note « Découvert à 1b.12 »**, sur la ligne 1b.12 de `docs/plan-3.md` : « patron 1b.1 appliqué sans écart ; `ref:autre` reçu en argument de méthode (`mp`) déballé par la lecture R5 de 1b.10, source différente, pas une lecture nouvelle ; R7 forme 1b.8 (sélecteur local retiré) ; revue des TU lents (1b.11) sans effet, deux pointeurs ; **R5 écrivain toujours vacante**, première candidate 1b.13 ; empreinte réelle 6 (5, 1), homonymes `ra`/`p`. » Le reste va dans le commit.
- Le message de commit porte le périmètre, le compte 213 → 211, le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous, `MEMBERS_HASH` et les réponses à H1 à H9.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
