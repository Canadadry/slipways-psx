---
title: "Plan 3, étape 1b.17 : `nf` (le lecteur souris) — `ra` devient un `pico8_vec_t *`, défaut partagé `g_nf_ra` gardé, un vecteur neuf par `ph`"
description: "`pico8_nf_t` n'a qu'un membre `pico8_value_t`, `ra` (`ref:autre`, nil `observé` par `stat?` d'homonymes seulement, lu par nom, jamais écrit par nom). Le typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 196 à 195. Pas d'écrivain par nom : la forme écrivain de 1b.13 ne s'applique pas ; l'identité du pointeur, que `bg.mq` emprunte, est l'enjeu."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.17 — `nf` (`mouse_reader.h:19`), `ra`. Empreinte 20 (10, 10). Compte : 195.

Cadre : puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », §3 (`make test` vert, **aucun `*.sha256` modifié**), §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Le **patron R1–R9** est celui de « Découvert à 1b.1 », cité, pas redécidé. S'appliquent sans être redérivées les notes « Découvert à » :

- **1b.3 à 1b.15** : en particulier `ref:autre` vers un vecteur → `pico8_vec_t *` emprunté, même pointeur (1b.4) ; lecteur qui copie ≠ `pico8_X_member`, R7 vacante (1b.6) ; nil `stat?` d'homonyme ne décide rien quand toute écriture est prouvée non nil (1b.7) ; **lecture `ref:autre` de 1b.10/1b.12** (REF non NULL de tag `UNKNOWN`) ; `bg.mq` emprunte `rp` (1b.15) ;
- **1b.11, règle R2** : relire les TU de `UNIT_TESTS_SLOW` pour des lectures nil par nom (fait plus bas) ;
- **1b.13, forme écrivain de R5**, avec les cas d'arrêt non-REF à union entièrement nulle construits par `memset` (1b.14b, 1b.15) : **vacante ici**, voir plus bas.

### État de départ

HEAD `7bf981d` (PRD de 1b.16). **Précondition** : 1b.16 (`pj`) est commitée avant que 1b.17 commence ; elle ne touche que `score_panel*`, `menu.c`, `test_pico8_pj.c`, `test_pico8_mh.c` et le plan, aucun fichier de cette étape. `make value-members` rend alors 196 (H1). Numéros de ligne de HEAD, susceptibles de glisser (H9).

**Rien n'a été compilé ni lancé pour ce PRD.** Faits tirés de `grep` et de la lecture ; tout fait d'exécution est une hypothèse (H1 à H12).

### Ligne de recensement (`tools/member_census.txt:49`)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `ra` | offset | `ref:autre` | observé | `stat?` | l |

Un membre : 196 − 1 = **195**. Pas de `e`. Le nil ne vient que de `stat?` (homonymes `jo`, `g_`, `ie`, `lq`…) : `ra` est posé non NULL par le constructeur et par `ph`, et rien d'autre ne l'écrit (H2). Pointeur, donc **aucun `has_`** de toute façon (R2).

**Revue des TU lents (R2 de 1b.11).** Des huit TU de `UNIT_TESTS_SLOW`, seul `test_pico8_ie` lit `je.ra` (`:675`) : **présent**, REF non NULL, puis écrit *à travers* le pointeur (`*(pico8_vec_t *)mouse.as.ref = v2(64, 64)`). Les `"ra"` des sept autres visent planètes, icônes ou `mo`. Aucune lecture nil par nom (H4). Sans effet.

### Ce que fait le cart

`nf` : spec `l=o(),r=o(),ra=v(0,0)` ; `nf:ph()` fait `self.ra=v(stat(32),stat(33))` — **un vecteur neuf à chaque tick**, jamais modifié en place (`pico8/lua.lua:203`). Lecteurs : `ui:lj` (`self.qa=je.ra+mo.ra`, par valeur), `ui:ev` (`je.ra.x`), `ui:gw` (`mv(e,"hd",self.qa,je.ra)`), `ui:bh` (`jz(bt.."down",self.qa,je.ra)`, `mv(held,"hq",self.qa,je.ra)`), `dl` (`:561`), `msgbox` (`:945`).

### Le code aujourd'hui

- `src/pico8_mouse_reader.h:17-20` : `{ pico8_ke_t ke; pico8_value_t ra; } pico8_nf_t` ; contrat « `l` and `r` are not members … TROU-7 » au-dessus ; contrat de `pico8_nf_ph` (« Rebinds `self.ra` to a fresh heap vector ») exact après l'étape. `pico8_vec.h` n'est inclus que par le `.c`.
- `src/pico8_mouse_reader.c` :
  - `static pico8_vec_t g_nf_ra = {0, 0};` (`:16`) : **défaut partagé par tous les `nf` neufs**, distinct du `v(0,0)` de la spec ;
  - lecteur `pico8_nf_field` (`:28-46`) : copie dans une locale `value` (`ra`, puis `l`/`r` qui ne sont pas des membres), `REF NULL` → 0, repli `pico8_ke_field`. Lecteur qui copie : **R7 vacante** (1b.6) ;
  - descripteur (`:48-52`) : `set_field` **NULL** — `pico8_entity_set_field` avorte sur `assert(header->cls->set_field != NULL …)` pour tout nom ;
  - constructeur (`:61-76`) : `self->ra = pico8_value_ref(&g_nf_ra)` après `pico8_ke_struct_init`, avant l'`assert` « no construction table » (aucune clé : moitié boucle de R5 vacante), `pico8_class_coverage_record("nf")`, `pq`, `PICO8_MEMBER_CENSUS_SEE` ;
  - `pico8_nf_ph` (`:114-132`) : `malloc` d'un vecteur (TROU-7), `self->ra = pico8_value_ref(ra)`.
- **Écriture directe dans un test** : `tests/test_pico8_iv.c:108`, `((pico8_nf_t *)pico8_je())->ra = pico8_value_ref(mouse);` — ne compile plus après le typage.
- **Pas de `offsetof(pico8_nf_t`**, pas de fille.

### Identité du pointeur et ses consommateurs

- `pico8_ui.c` lit `je.ra` par `pico8_ui_field_raw` (`:387`, `:449`) et passe **la REF elle-même** comme `rp` à `hd` (`gw`), à `jz(…down)` et à `hq` (`bh`). Le seul qui la **garde** est `bg.mq` (`bg_take_vec(rp, &self->mq)`, 1b.15, `rdown`/`hq`) : après `ui:bh`, `bg.mq == je.ra` par adresse. Ailleurs, lecture par valeur (`ui:lj`, `ui:ev`, `dl`, `msgbox`).
- **Correction de l'hypothèse de départ** : `jc_mouse_point.ra` (1b.12) **n'emprunte pas** `nf.ra`. `jc:hq(mp)` reçoit `arg0` = `ui.qa` (`pico8_ui.c:466`, `pico8_slipway.c:448`), un vecteur neuf par frame calculé par valeur depuis `je.ra + mo.ra`. De même `g_.ra` (`bv`, `probe.c:193`) vient de `mp` = `qa`.
- `test_pico8_entity_kind:332` et `test_pico8_ie:675` écrivent **à travers** la REF de `je.ra` : juste après `jl()`, c'est `g_nf_ra`, la statique partagée. R3 doit donc garder `&g_nf_ra` (même REF), pas un vecteur par `nf`, ni celui de la spec. `test_struct_defaults` (`STRUCT_HASH`) juge l'égalité d'adresse struct/spec : aujourd'hui fausse pour `ra`, elle doit le rester.

### Empreinte remesurée

La ligne dit 20 (10, 10), majorant du nom court `ra`. **Empreinte réelle** (fichiers qui lisent ou écrivent le `ra` d'un `pico8_nf_t`) : **12 (4, 8)** — `src/pico8_mouse_reader.c` (direct), `pico8_ui.c`, `pico8_probe.c`, `pico8_msgbox.c` (par nom, via `pico8_je()`) ; `tests/test_pico8_iv.c` (direct), `test_pico8_nf.c`, `test_pico8_je.c`, `test_pico8_ui.c`, `test_pico8_jc.c`, `test_pico8_entity_kind.c`, `test_pico8_ie.c`, `test_pico8_spec_defaults.c` (par nom).

**Fichiers modifiés : 4**, plus `docs/plan-3.md` : `src/pico8_mouse_reader.h`, `src/pico8_mouse_reader.c`, `tests/test_pico8_iv.c` (la ligne `:108`), `tests/test_pico8_nf.c` (pur ajout). Aucun TU de `UNIT_TESTS_SLOW` touché.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code**, par nom seulement : valeur et **adresse** de `ra` sur un `nf` neuf, après `ph`, chez un consommateur (`bg.mq`), et les écritures par nom qui avortent.
2. **Typage** : `pico8_vec_t *ra;` après `pico8_ke_t ke`. `#include "pico8_vec.h"` dans le `.h`.
3. R3 `self->ra = &g_nf_ra;` ; R6 `self->ra = ra;` dans `ph` ; R4 branche `ra` (NULL → 0, sinon `pico8_value_ref(nf->ra)`, même pointeur) ; `test_pico8_iv.c:108` → `->ra = mouse;`.
4. `make value-members` : 196 → **195**, aucune ligne `pico8_nf_t`. Aucun pixel ne bouge.
5. Note « Découvert à 1b.17 ».

## User Stories

1. En tant que lecteur de `mouse_reader.h`, je veux `ra` typé `pico8_vec_t *`, afin que le genre se lise sur le type.
2. En tant que `ui:lj`, `ui:ev`, `dl` et `msgbox`, je veux lire `je.ra` par nom, même valeur qu'en 1a.
3. En tant que `ui:gw`/`ui:bh`, je veux recevoir de `pico8_entity_get_field` **le même pointeur** que tient `nf`, afin que `bg.mq` garde `je.ra` par adresse.
4. En tant que `nf` neuf, je veux pointer la statique `g_nf_ra` partagée, pas le vecteur de la spec ni un vecteur à moi.
5. En tant que `ph`, je veux rebrancher `ra` sur un vecteur neuf à chaque appel, l'ancien intact.
6. En tant que `test_pico8_entity_kind` et `test_pico8_ie`, je veux écrire à travers `je.ra` comme aujourd'hui.
7. En tant que mainteneur, je veux que toute écriture par nom sur un `nf` avorte comme aujourd'hui (nil, vecteur, `false` et fixe 0 à union nulle).
8. En tant que lecteur par nom, je veux 0 avec `*out_value` intact pour tout nom inconnu, et `l`, `r`, `me`, `t` inchangés.
9. En tant que `test_pico8_iv`, je veux poser la souris à (64, 64) sur `je` sans changer mes empreintes.
10. En tant que gardien du §3, je veux `make test` vert, 15 « conforme », aucun `*.sha256` touché.
11. En tant que gardien de la mémoire et de la couverture, je veux `sanitize-net` 15 propres et `class-coverage-net` identique à sa référence.
12. En tant que `plan-pilot`, je veux `make value-members` à 195.
13. En tant que vérificateur, je veux la caractérisation verte sur l'ancien code, les mutations du §7 et leurs trous justifiés, `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de valider modules et tests avec l'utilisateur ; tranché comme le ferait un agent `guideline` (hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables), marqué (G).

| règle | sur `nf` |
|---|---|
| R1 | `ra` → `pico8_vec_t *` (1b.4), emprunté, non `const`. |
| R2 | Aucun indicateur : pointeur ; nil `stat?` seul (1b.7) ; revue des TU lents sans effet. |
| R3 | `self->ra = &g_nf_ra;` à la place de l'affectation actuelle, même position. **Même REF** : ni `pico8_nf_default("ra")`, ni `malloc` par `nf`. |
| R4 | Branche `ra` : `nf->ra == NULL` → `return 0` (`*out_value` intact), sinon `*out_value = pico8_value_ref(nf->ra); return 1;`. Les branches `l`/`r` et le repli `ke` gardent leur forme (G : `l`/`r` ne sont pas des membres, hors étape). |
| R5 | **Vacante, les deux moitiés** : aucune clé de construction (l'`assert` reste), `set_field` NULL, aucune ligne `e`, aucune source par argument. Pas d'aide `take`, pas d'`assert` « champ sans règle ». La forme écrivain de 1b.13 ne s'applique pas ; (G) pas d'écrivain ajouté. |
| R6 | `ph` : `self->ra = pico8_value_ref(ra)` → `self->ra = ra`. Test : `tests/test_pico8_iv.c:108` → `((pico8_nf_t *)pico8_je())->ra = mouse;` (G, précédent 1b.16 pour `test_pico8_mh`) : même effet (vecteur neuf rebranché), pas un site par nom remplacé. |
| R7 | Vacante (lecteur qui copie, 1b.6 ; aucun `offsetof`). |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_nf_t)` gardé ; table non éditée. |
| R9 | 196 → 195, aucune ligne `pico8_nf_t`. |

Commentaires : les contrats existants restent exacts, aucun n'est ajouté (CLAUDE.md).

### Preuves

- `grep -nE '^\s*pico8_value_t [a-z_]+;' src/pico8_mouse_reader.h` : `ra` avant, **vide** après ;
- `grep -rn 'offsetof(pico8_nf_t' src/` : vide avant comme après ;
- `grep -rnE 'pico8_value_ref\((&g_nf_ra|ra|mouse)\)' src/pico8_mouse_reader.c tests/test_pico8_iv.c` : 3 lignes avant, **vide** après ;
- `grep -rnE '\->ra = ' src/pico8_mouse_reader.c tests/test_pico8_iv.c` : 3 lignes après (`&g_nf_ra`, `ra`, `mouse`) ;
- `grep -n 'pico8_value_t \*' src/pico8_mouse_reader.c | grep -v 'pico8_value_t \*out_value)'` : vide avant comme après ;
- `grep -c 'off its census row' src/pico8_mouse_reader.c` : 0 avant comme après.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 196 au départ (1b.16 commitée), 195 après.
- **H2** : `nf.ra` n'est jamais NULL dans les 15 rejeux et les TU : la branche NULL de R4 n'est pas exécutée (trou `gcov` attendu).
- **H3** : `pico8_type_tag_of` rend `UNKNOWN` sur `&g_nf_ra` et sur les vecteurs de `ph` (ligne `ref:autre`).
- **H4** : aucun TU de `UNIT_TESTS_SLOW` ne lit `je.ra` nil par nom.
- **H5** : `SCRIPTED_HASH`, `TITLE_HASH` (`test_pico8_nf`), `STRUCT_HASH`/`SPEC_HASH`, les empreintes de `test_pico8_iv`, `ui`, `jc`, `je`, `entity_kind`, `ie`, `bg` et les 15 du filet ne bougent pas.
- **H6** : le build `-DPICO8_MEMBER_CENSUS` compile (struct sans `pico8_value_t` propre, comme après 1b.14b).
- **H7** : sous ASan+UBSan, rien de nouveau (aucune allocation ajoutée).
- **H8** : `test_pico8_nf` reste sous 30 s avec les ajouts (hors `UNIT_TESTS_SLOW`) ; sinon l'étape s'arrête et le note.
- **H9** : 1b.16 ne touche aucun fichier de l'étape ; les numéros glissent peut-être.
- **H10** : `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` pour `nf` (le corps du constructeur change, pas le compte).
- **H11** : lecteur brut et `pico8_entity_get_field` rendent les mêmes `me`/`t` (repli `ke`) sur un `nf`.
- **H12** : dans `test_pico8_nf`, après `pico8_cart_load()` (qui appelle `pico8_bg_init`), `pico8_bg_new(NULL)` et `pico8_mv(bg, "rdown", …)` marchent ; sinon le cas « consommateur » (point 3 ci-dessous) va, en pur ajout, dans `test_pico8_bg`, et le commit le dit.

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom, ou l'écran.** Aucun accès direct à `->ra` dans les tests ajoutés.
- **Caractérisation figée sur l'ancien code** (G) : `test_nf_members_by_name` dans `test_pico8_nf` (rapide, dans `CENSUS_UNIT_TESTS`). Lecture par le **lecteur brut** (`pico8_nf_struct_class()->field`, témoin fixe `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder.
  1. **Neuf.** `pico8_jl()`, `je = pico8_je()`, `a = pico8_nf_new(NULL)`, `b = pico8_nf_new(NULL)` : `ra` présent, `REF`, tag `UNKNOWN`, (0, 0) ; **même adresse** pour `je`, `a`, `b` ; adresse **différente** de `pico8_nf_default("ra")`. `l`, `r` présents, partagés. `me`, `t` par le repli (H11). Intrus `""`, `"x"`, `"RA"`, `"p"`, `"v"`, `"ke"`, `"header"`, `"has_ra"` → 0, témoin intact.
  2. **`ph`.** Deux frames scriptées (x1, y1), (x2, y2) et `pico8_nf_ph(&input, a)` deux fois : après chaque appel, `ra` de `a` a la valeur de la frame et une **adresse neuve** (≠ défaut, ≠ précédente) ; le vecteur précédent garde son ancienne valeur ; `b` pointe toujours le défaut, (0, 0).
  3. **Consommateur.** `bg = pico8_bg_new(NULL)`, `r = get(je, "ra")` (REF brute, comme `pico8_ui_field_raw`) ; `pico8_mv(bg, "rdown", 2, vec_ref(0, 0), r, &out)` → `bg.mq` lu par nom **à la même adresse** que `je.ra` (H12).
  4. **Écritures par nom qui avortent**, chacune dans un fils (`fork`, `SIGABRT`, forme de `must_abort` de `test_pico8_bg`, utilitaire local ajouté) : `pico8_entity_set_field(je, "ra", v)` pour `v` ∈ {`REF NULL`, `vec_ref(1, 1)`, `false` à union nulle, fixe 0 à union nulle (`memset`, puis `kind`)}. Vert sur l'ancien code (`set_field` NULL) : c'est la forme de 1b.16, qui fige qu'aucun écrivain n'apparaît.
  5. Retours, genres et valeurs pliés dans **`NF_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ ; adresses jugées par `assert`.

  Ligne « Runs » verte sur l'arbre de départ **avant** le typage ; ses valeurs ne changent plus.
- **Pas de test d'arrêt rouge d'abord** : sans aide R5, aucun `assert` « champ sans règle » n'est ajouté ; les cas `memset` de 1b.13 vont au point 4, verts avant comme après (trou justifié : il n'y a rien à rendre rouge).
- **Existants inchangés qui figent déjà** : `test_jl_builds_a_mouse_reader`, `test_scripted_ticks` (`SCRIPTED_HASH`), `test_replay_from_title` (`TITLE_HASH`), `assert_an_nf` de `test_pico8_je`, `test_struct_defaults` (`STRUCT_HASH`), `test_drag` de `test_pico8_bg`. Aucun test de régression (§6) ne lit `nf.ra`.
- **Mutations (§7)**, copie jetable du scratchpad, `make tmp/test_pico8_nf && ./tmp/test_pico8_nf` (plus `test_pico8_spec_defaults` où dit) :
  1. (§7-1) *indicateur forcé vrai* : **sans objet**, aucun `has_`. Forme la plus proche, R4 qui rend `REF NULL` présent quand `ra` est NULL : **trou justifié**, `ra` n'est jamais NULL (H2), aucun chemin ne l'atteint.
  2. (§7-2) *« présent 0 » là où c'était absent* : **sans objet** (pas de fixe, jamais absent). Forme la plus proche, R4 qui rend `pico8_value_ref(&g_nf_ra)` au lieu de `nf->ra` : rouge `SCRIPTED_HASH`, caractérisation 2 et 3.
  3. (§7-3) *init qui saute le membre* (`malloc` → `calloc` dans la copie) : `ra` NULL → rouge `test_jl_builds_a_mouse_reader`, caractérisation 1, `STRUCT_HASH`.
  4. Propre : *R3 vers la spec* (`ra` = pointeur de `pico8_nf_default("ra")`) → rouge `STRUCT_HASH`, caractérisation 1.
  5. Propre : *R3 par `nf`* (un `malloc` (0, 0) par constructeur) → rouge caractérisation 1 (adresses partagées).
  6. Propre : *`ph` en place* (`*self->ra = …` sans rebrancher) → rouge caractérisation 2 (adresse inchangée, ancien vecteur écrasé, défaut partagé modifié).
  7. Propre : *R4 qui copie* (vecteur neuf à chaque lecture) → rouge caractérisation 1 et 3 (identité).
- **`gcov`** : `test_pico8_nf` en `--coverage -O0`, piège `__gcov_dump` des fils de 1a.23. Trou attendu : la branche NULL de R4 (H2). Rapport dans le commit.
- **Prior art** : `test_mouse_point_is_a_struct` (1b.12), `test_bg_members_by_name` et `must_abort` de `test_pico8_bg` (1b.15), la caractérisation de 1b.16 (écritures par nom qui avortent).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur : `./tmp/test_pico8_nf` ~10 lancements (départ, après, 7 mutations, `gcov`), 2 à 4 min ; `make test` < 1 min ; `make e2e` 4 à 6 min ; `make class-coverage-net` 3 à 4 min, même invocation que `e2e` (arbres distincts) ; `make sanitize-net` ~4 min, **jamais** en même temps que `e2e` ; `make value-members` quelques secondes.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Une ligne par lancement ; le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `UNIT_TESTS_SLOW` verts (aucun touché) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 195`**, aucune ligne `pico8_nf_t` ;
5. `timeout 15m make class-coverage-net` : **s'applique** — le diff change le corps du constructeur `pico8_nf_struct_new` (R3), sans toucher `pico8_class_coverage_record("nf")` ni `KNOWN[]` (lecture du §7 de 1b.10, 1b.14a, 1b.15). Sortie identique à la référence (H10).

`make member-census` ne s'applique pas (R8). On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements. On ne lance jamais l'appli raylib.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`) ; les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si `ra` doit tenir autre chose qu'un vecteur ou NULL, si un indicateur ou un membre est ajouté, si un écrivain par nom apparaît, ou si un type est élargi au lieu d'avorter.
- `FAILED:` si le typedef n'est pas exactement `pico8_ke_t ke; pico8_vec_t *ra;`.
- `FAILED:` si un `nf` neuf ne pointe pas `&g_nf_ra` (même adresse pour tous, différente de la spec), si `ph` modifie un vecteur en place au lieu de rebrancher, ou si le lecteur ne rend pas le même pointeur.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur un nom inconnu, ou si `l`, `r`, `me`, `t` changent.
- `FAILED:` si un site par nom change (`pico8_ui.c`, `probe.c`, `msgbox.c`, tests), si le diff d'un test fait autre chose qu'ajouter la caractérisation, ses utilitaires, `NF_MEMBERS_HASH` et leurs appels, plus la ligne `test_pico8_iv.c:108`, ou si une valeur figée existante change ; si un test de régression (§6) change.
- `FAILED:` si `bg`, `ui`, `jc` ou leurs lecteurs, écrivains ou aides changent.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)`, si `make value-members` ne rend pas 195 ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` s'il manque une ligne « Runs » de `make class-coverage-net` identique à la référence, ou si le diff touche `pico8_class_coverage_record` ou `KNOWN[]`.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, lit ou écrit `ra` directement, ou si `NF_MEMBERS_HASH` change après.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 7 mutations, chacune avec son test rouge ou la justification de son trou.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.17 » ou le tableau « Runs ».

## Out of Scope

- `l`/`r` (tables partagées du module, pas des membres) ; typer `ui.qa` (1b.36) ou `bg.mq` (fait) ; `jc_mouse_point` (fait).
- Un écrivain par nom pour `nf` ; une aide de déballage.
- Remplacer une lecture par nom par un accès direct ; régénérer `tools/member_census.txt`.
- Libérer les vecteurs de `ph` (TROU-7) ; remettre `g_nf_ra` à zéro dans `pico8_nf_init`.
- Les fichiers d'événements et `FRAMES_EVENTS` ; tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) : module `mouse_reader.[ch]` seul, un commit ; tests : `test_pico8_nf` étendu, sans nouveau TU ; caractérisation par nom gardée alors que la ligne n'est pas « sans genre », parce que l'enjeu est l'identité du pointeur ; `test_pico8_iv.c:108` réécrit en affectation typée (seul accès direct hors module) ; `class-coverage-net` lancé.
- **Note « Découvert à 1b.17 »**, sur la ligne 1b.17 : « patron 1b.1 appliqué sans écart ; `ra` → `pico8_vec_t *`, R3 garde le défaut **module** `&g_nf_ra` (même REF, partagé par tous les `nf`, distinct de la spec, écrit à travers par deux TU), `ph` rebranche (R6) ; R5 vacante (ni clé, ni `set_field`, ni argument), R7 vacante (lecteur qui copie) ; `jc_mouse_point.ra` emprunte `ui.qa`, pas `nf.ra` ; seul `bg.mq` garde `je.ra` par adresse ; revue des TU lents sans effet ; empreinte réelle 12 (4, 8). » Le reste va dans le commit.
- Le message de commit porte le périmètre, 196 → 195, le résumé « Runs », `gcov`, les mutations et leurs trous, `NF_MEMBERS_HASH` et les réponses à H1 à H12.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
