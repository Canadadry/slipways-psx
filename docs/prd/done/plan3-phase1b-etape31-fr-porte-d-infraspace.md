---
title: "Plan 3, étape 1b.31 : `fr`, la porte d'infraspace. `mt` en `pico8_fixed_t` sans indicateur ; `g_fr_defaults` supprimée, plus aucune table d'offsets dans `planet_family_trade.c`"
description: "Après 1b.30, `pico8_fr_t` garde un seul membre `pico8_value_t`, `mt` (le fixe 1 de la spec de `fr`). 1b.31 le type selon R1–R9 de 1b.1, sans indicateur ni écrivain ; `g_fr_defaults` se vide et est supprimée ; `make value-members` passe de 96 à 95."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.31 — `fr` (`planet_family_trade.h:38`), `mt`. Empreinte 4 (2, 2). Compte : 95.

Cadre : puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », §3 (`make test` vert, aucun `*.sha256` modifié), §6, §7. Modèle : le PRD de 1b.30 (`plan3-phase1b-etape30-nr-porte-de-glisse.md`, la parente, même fichier). Notes **citées, pas redérivées** :

- **1b.1, puis 1b.3 à 1b.29** : patron R1–R9. En particulier **1b.7** (R2 : un nil `stat?` venu d'un homonyme ne donne pas d'indicateur quand toute écriture sur le typedef est non nil), **1b.21** (pas d'indicateur sans lecture nil par nom), **1b.24/1b.25** (tables vidées supprimées, preuve de typedef par `awk`, caractérisation dans le TU rapide de la famille), **1b.27** (R5 vacante sans écrivain : pas de test d'arrêt).
- **Patron des défauts `static`** (R7 de 1b.14a, repris en 1b.23–1b.30) : déballage une fois à l'init dans `static g_fr_mt`, remise à 0 dans `defaults_reset`, pose typée à la place de l'ancienne boucle, avant les clés et tout `pq`.
- **Forme écrivain de 1b.13/1b.15** : sans objet (pas d'écrivain, voir R5).
- **1b.23d** : un `written[]` vidé est supprimé avec sa boucle — sans objet (aucun `written[]` ne liste `mt` ; celui de `test_fr_is_a_struct` a déjà disparu).
- **1b.11 — R2 des TU lents** : revue plus bas.

### État de départ

**Précondition : 1b.30 est commitée avant que 1b.31 commence (H1)** ; `make value-members` rend alors **96**. Numéros de ligne indicatifs (arbre de travail de 1b.30). **Rien n'a été compilé ni lancé pour ce PRD** : tout fait d'exécution est une hypothèse (H1–H12).

### Ligne de recensement (`tools/member_census.txt:129`)

| membre | accès | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|---|
| `mt` | offset | `fixe` | observé | stat? | l | `pico8_fixed_t` | non (R2 de 1b.7) |

Compte : 96 − 1 = **95**. Aucune ligne `pico8_fr_t` ne reste. Le `stat?` vient de l'homonyme `jc.mt` (cart `:624-672`, `self.mt and 2 or 4`, `sw.mt==self.mt`) ; sur un `fr`, `mt` n'est jamais écrit.

### Le cart (`pico8/lua.lua`)

- `fr=nr:jr` (`:1123-1127`) : `mt=1`. Seule écriture. Lu seulement par nom, via `self.qu.mt or self.re.mt` (`:624-625`) du `jc` qui part de la porte.

### Le code (`src/pico8_planet_family_trade.[ch]`, après 1b.30)

- **Typedef** `pico8_fr_t` (`.h:38-41`) : `nr`, puis `pico8_value_t mt` ; son commentaire reste vrai.
- **Table** `g_fr_defaults = {mt}` et ses 3 clients : boucle de `pico8_fr_defaults_reset`, `self->mt = REF NULL` + boucle de recopie dans `pico8_fr_struct_new`, boucle de lecture + `assert` « fr: every default … » de `pico8_planet_family_trade_init`. C'est la **dernière** table à `offsetof` du fichier.
- **Lecteur** `pico8_fr_field` : branche `mt` (REF NULL → 0), sinon `pico8_nr_field`.
- **Écrivain** : `fr` prend `pico8_nr_set_field`, sans branche `mt` ; `write_mt` avorte déjà (`test_pico8_fr`, `test_pico8_nr`).
- **Accès direct (R6)** : aucun dans `src/` hors R3/R4. `slipway.c` lit `mt` par nom (`pico8_jc_entity_value(qu, "mt")`), inchangé.
- **Aide** existante : `family_trade_default_fixed`.

### Revue des TU lents (R2 de 1b.11)

`test_pico8_nr` : `not_nr[]` ∋ `mt` et `write_mt` portent sur des `nr` (pas le membre de `fr`) ; les `fr` qu'il construit ne lisent que `s`. `test_pico8_jo`/`test_pico8_qh`/`test_pico8_ie` : aucun `"mt"`. **Aucune lecture nil par nom de `fr.mt` : aucun indicateur.**

Seule lecture nil connue : `test_fr_is_a_struct` (`test_pico8_fr.c:~602`, après `PICO8_MEMBER_CENSUS_STOP()`) écrit `bare->mt = pico8_value_ref(NULL)` **directement** puis lit `!cls->field(bare, "mt")`. C'est un nil fabriqué par le test, que le cart ne produit jamais. Ce n'est pas une preuve (G) : ces deux lignes sont réécrites en « présent 0 » (voir Testing).

## Solution

1. **Caractérisation figée sur l'ancien code** (arbre 1b.30 commité), dans le TU rapide `test_pico8_fr`, qui lit `mt` par nom seul ; verte avant tout typage.
2. **Commit 1b.31** : `mt` typé ; `g_fr_defaults` supprimée avec ses 3 boucles et son `assert` ; `g_fr_mt` déballé en `static` ; les deux lignes `bare` réécrites ; `make value-members` rend **95**.
3. Pas de test d'arrêt : **aucun écrivain** (R5 vacante, 1b.27).
4. Aucun pixel ne bouge. Note « Découvert à 1b.31 » sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `planet_family_trade.h`, je veux `pico8_fr_t` sans `pico8_value_t`.
2. En tant que lecteur par nom, je veux que `mt` d'une porte neuve rende le fixe 1 de la spec de `fr`, et qu'un slipgate `nr` n'ait toujours pas de `mt`.
3. En tant que `jc` tiré d'une porte, je veux `mt` (puis `fa`) inchangés.
4. En tant que gardien du §5, je veux qu'une écriture par nom de `mt` avorte toujours.
5. En tant que mainteneur, je veux `g_fr_defaults` supprimée, et plus aucun `offsetof` dans `planet_family_trade.c`.
6. En tant que gardien du §3, je veux `make test` vert et aucun `*.sha256` touché.
7. En tant que `plan-pilot`, je veux `make value-members` à 95 sans ligne `pico8_fr_t`.
8. En tant que vérificateur, je veux une caractérisation verte sur l'ancien code, les mutations et leurs trous justifiés, un rapport `gcov` et un tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle : les validations que demande `to-prd` (modules, tests) sont tranchées comme le ferait un agent `guideline` (hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables) et marquées (G).

### R1–R9 appliquées

| règle | sur `fr` |
|---|---|
| R1 | `pico8_nr_t nr; pico8_fixed_t mt;` — place et nom gardés. |
| R2 | Aucun indicateur : `stat?` d'homonyme, seule écriture le défaut 1 (1b.7) ; aucune lecture nil par nom hors le nil fabriqué du test (G). |
| R3 | `pico8_fr_struct_new` : `self->mt = g_fr_mt;` remplace `REF NULL` et la boucle, avant `nr.hz`/`nr.rr`/`od`/`s`, les clés et `pq` ; `size_t i` retiré. |
| R4 | Branche `mt` : `*out_value = pico8_value_fixed(mt); return 1;` toujours présent. |
| R5 | **Vacante** : pas de branche d'écrivain, clé `id` seule. Pas de test d'arrêt (G). |
| R6 | Aucun site `src/`. `test_pico8_fr` : les deux lignes `bare` (voir Testing). |
| R7 | `g_fr_defaults` supprimée avec ses 3 boucles et son `assert` ; `size_t i` retiré de `pico8_fr_defaults_reset` et de `pico8_planet_family_trade_init` (plus aucune boucle `i` dans l'init) ; `#include <stddef.h>` retiré si plus aucun `offsetof` (précédent 1b.20, H9). |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_fr_t)` gardé ; `tools/member_census.txt` non édité. |
| R9 | 96 → 95. |

### Défaut déballé (patron `static`) (G)

`static pico8_fixed_t g_fr_mt = 0`, déballé dans l'init après `g_fr_rr` par `pico8_fr_default("mt")` et `family_trade_default_fixed` (message élargi : `nr/fr/mx.od` → ajouter `fr.mt`) ; remis à 0 dans `pico8_fr_defaults_reset`. Aucune aide nouvelle.

### Revue des tests de régression, mécanique

**Liste** : les tests nommés au §6 (les 17 de régression) et dans les lignes 1a : `tests/test_pico8_<nom>.c` pour `built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`, `bg`, `entity_kind`, `hj`, `jc`, `jo`, `jo_spec_defaults`, `js`, `mo`, `mz`, `nl`, `os`, `pm`, `qh`, plus `tests/test_class_coverage.c` (`class_chain` et `entity_smoke` n'existent plus).

**Greps** (faits pour ce PRD) : accès direct `grep -cE '(->|\.)mt\b|offsetof\([^)]*\bmt\)'` → **0 partout**. Par nom `"mt"` : `jc` (5, sur des `jc`, autre typedef), `jo_spec_defaults` (`NAMES[]`, lecture seule sur un `fr` neuf, FIXED 1 inchangé). **Aucune preuve rouge due**, aucun diff (H6).

### Preuves (sortie attendue sur l'arbre final)

- `awk '/^typedef struct \{/{b=""} {b=b $0 "\n"} /\} pico8_fr_t;/{printf "%s", b}' src/pico8_planet_family_trade.h | grep -c pico8_value_t` : **0** ;
- `make value-members | grep -c pico8_fr_t` : **0** ;
- `grep -rnw g_fr_defaults src/` et `grep -c offsetof src/pico8_planet_family_trade.c` : **vide / 0** ;
- `git diff --stat HEAD -- src/ ':!src/pico8_planet_family_trade.h' ':!src/pico8_planet_family_trade.c' tools/ Makefile` : **vide** ;
- dans `tests/`, seul `test_pico8_fr.c` change (caractérisation ajoutée, deux lignes `bare` réécrites).

### Empreinte et découpage

Ligne : 4 (2, 2). Estimée : 3 (2, 1) — `.h`, `.c`, `test_pico8_fr` (H10). **Aucun découpage (G)**, un commit.

### Hypothèses (rien n'a été lancé)

- **H1** : 1b.30 commitée ; `make value-members` rend 96 au départ, 95 à la fin, seule la ligne `pico8_fr_t` disparaît.
- **H2** : l'arbre 1b.30 a la forme décrite plus haut (`g_fr_defaults = {mt}`, `g_fr_hz`/`g_fr_rr` déballés, `test_nr_members_by_name` et `read_both` présents dans `test_pico8_fr`).
- **H3** : sur toute porte, `mt` vaut FIXED 1 (jamais nil, jamais autre chose), égal à `class_value(pico8_fr_default, "mt")`.
- **H4** : `test_pico8_fr` reste sous 30 s avec une fonction de plus.
- **H5** : aucune valeur figée ne bouge : `BUILT_HASH`, `LIFE_HASH`, `NR_MEMBERS_HASH`, hash de `jo_spec_defaults`, `jc`, `test_pico8_nr`, les 15 empreintes du filet.
- **H6** : aucun test de régression ne dépend de la représentation de `mt`.
- **H7** : `make sanitize-net` propre (aucune allocation nouvelle ; `mt` était déjà posé avant tout usage).
- **H8** : le build `-DPICO8_MEMBER_CENSUS` compile.
- **H9** : `-Werror` muet après retrait des trois `size_t i` et de `<stddef.h>` (`size_t`/`NULL` viennent d'autres en-têtes) ; sinon `<stddef.h>` reste.
- **H10** : empreinte réelle comptée au commit.
- **H11** : `slipway.c` lit toujours FIXED 1 par nom sur un `fr` (le `jc` tiré garde `mt`, `fa` = 2).
- **H12** : le `nr` brut ne voit pas `mt` sur un `fr` (`test_pico8_fr.c:~581`), inchangé.

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom, ou l'écran.** La caractérisation n'accède jamais à `mt` directement : elle compile sur les deux formes.
- **(G) Où : `test_pico8_fr`**, rapide, dans `CENSUS_UNIT_TESTS`, TU de la classe ; il a `read_both`, `class_value`, `build_gate`, `build`, `fold_value`, `in_fresh_child`.
- **Caractérisation `test_fr_mt_by_name`**, par `in_fresh_child` après `test_nr_members_by_name`, hash propre **`FR_MT_HASH`** :
  1. **Neufs** : `pico8_fr_new(NULL)` et `build_gate()` : `read_both(…, "mt")` rend FIXED égal à `class_value(pico8_fr_default, "mt")`, plié. `pico8_nr_new(NULL)` : `read_both(…, "mt")` rend 0.
  2. **Après la vie** : sur la porte posée (`place`), `hb` puis `ldown` : `mt` inchangé, plié.
  3. **Intrus** sur un `fr` : `""`, `"m"`, `"mt "`, `"MT"`, `"mt_"`, `"t"` : présence pliée (0 attendu, sauf `t` hérité de `jo`).
  4. Tout plié dans `FR_MT_HASH`, relevé au premier passage vert sur l'arbre de départ, figé ensuite.
- **Réécriture de `test_fr_is_a_struct` (G)** : `bare->mt = pico8_value_ref(NULL); assert(!cls->field(bare, "mt", &v));` devient `bare->mt = 0;` puis `assert(read_both(bare, "mt", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0)`. En Lua, 0 n'est pas nil : le test fige « pas de sentinelle ». Il n'est pas figé sur l'ancien code (il ne compile que sur le nouveau), comme les deux affectations de `test_pico8_nr` en 1b.30.
- **Pas de test d'arrêt** : pas d'écrivain (G, 1b.27).
- **Mutations (§7)**, dans une copie jetable du scratchpad, par `make tmp/test_pico8_fr && ./tmp/test_pico8_fr`, notées dans le commit :
  1. **(§7-1) indicateur forcé vrai** : sans objet dans sa lettre (pas de `has_`). **Forme voisine**, la sentinelle : la branche `mt` rend 0 quand `mt == 0` → rouge : `test_fr_is_a_struct` (ligne `bare` réécrite).
  2. **(§7-2) « présent 0 » là où absent** : sur un `fr`, `mt` n'est jamais absent. **Forme voisine** : la branche compare par préfixe (`strncmp(name, "mt", 2)`) → `"mt "`/`"mt_"` deviennent présents → rouge : caractérisation 3.
  3. **(§7-3) init qui saute le membre** : (a) `self->mt = 0` au lieu de `g_fr_mt` → rouge : caractérisation 1 et 2, `test_life` (`mt` = 1) ; (b) déballage de `g_fr_mt` sauté à l'init → même rouge.
  4. **R4 sur l'instance** : la branche lit `g_fr_mt` au lieu de `self->mt` → rouge : `test_fr_is_a_struct` (`bare` à 0).
  5. Remise à 0 retirée de `defaults_reset` : **mutant équivalent, noté** (l'init redéballe avant toute lecture).
- **`gcov`** : `test_pico8_fr` en `--coverage -O0`, piège `__gcov_dump` des fils, sur `planet_family_trade.c`. Lignes attendues exécutées : déballage et remise à 0, pose R3, branche R4. **Trous attendus** : échecs d'`assert`. Rapport dans le commit.
- **Prior art** : `test_nr_members_by_name` (1b.30) de `test_pico8_fr`, `test_mx_members_by_name` (1b.28), `test_life`, `test_fr_is_a_struct`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque.

- `./tmp/test_pico8_fr` : ≤ 30 s (H4) ; ~9 lancements (départ, final, mutations 1, 2, 3a, 3b, 4, 5, `gcov`) ≈ 4 min.
- `make test` une fois ; `make e2e` une fois, **dans la même invocation** que `make class-coverage-net` ; `make sanitize-net` une fois, jamais en même temps que `e2e` ; `make value-members`.
- Aucun TU lent n'est lancé seul (aucune mutation n'en dépend).

| # | Commande exacte | État de l'arbre (départ / final / mutation n) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, les **8** TU de `UNIT_TESTS_SLOW` verts ;
3. `timeout 15m make sanitize-net` : **15 propre(s)** ;
4. `make value-members` : **95**, aucune ligne `pico8_fr_t` ;
5. `timeout 15m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` — requis, `pico8_fr_struct_new` change.

On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

- `FAILED:` si 1b.30 n'est pas commitée au départ, ou si `make value-members` ne rend pas 96 au départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, ou si le diff touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements.
- `FAILED:` si `mt` n'est pas `pico8_fixed_t`, si un `has_mt` ou une sentinelle est ajouté, ou si une porte neuve n'a pas `mt` = 1.
- `FAILED:` si `g_fr_defaults` ou un `offsetof` existe encore dans `planet_family_trade.c`, ou si `g_fr_mt` n'est pas remis à 0 dans `pico8_fr_defaults_reset`.
- `FAILED:` si un écrivain ou une clé de construction de `fr` accepte `mt`.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, lit `mt` directement, ou si `FR_MT_HASH` change après.
- `FAILED:` si un autre fichier de `src/` que `planet_family_trade.[ch]`, `tools/` ou le `Makefile` change ; si un autre test que `test_pico8_fr.c` change ; si un test de régression a un diff ; si une valeur figée existante change (H5).
- `FAILED:` si le commit ne porte pas le rapport `gcov` et les mutations 1 à 5, chacune avec son test ou la justification de son trou.
- `FAILED:` si une preuve ne rend pas l'attendu, si `make sanitize-net` n'est pas `15 propre(s)`, ou s'il manque la ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` si le travail n'est pas en un commit vert, si le budget est dépassé sans note de ce qui n'a pas été lancé, ou s'il manque le tableau « Runs » ou la note « Découvert à 1b.31 ».

## Out of Scope

- `jc.mt` (1b.35) ; les membres de `nr`/`qh`/`jo` portés par `fr` (déjà typés) ; `mx`.
- Libérer les specs (TROU-7) ; régénérer `tools/member_census.txt` ; déplacer un TU entre listes.
- Fichiers d'événements, `FRAMES_EVENTS`, rendu, audio, appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur (G)** : modules `planet_family_trade.h` (typedef) et `.c` (lecteur, R3, R7, un `static`, message d'aide, `<stddef.h>`) ; fixe sans indicateur (R2 de 1b.7 : `stat?` d'homonyme, nil du `bare` fabriqué par le test, pas une preuve) ; pas d'écrivain, pas de test d'arrêt ; tests : `test_pico8_fr` seul (caractérisation, deux lignes `bare` réécrites en « présent 0 ») ; pas de découpage.
- **Note « Découvert à 1b.31 »** (à écrire sur la ligne) : patron 1b.1 appliqué ; `mt` → `pico8_fixed_t`, aucun indicateur (R2 de 1b.7 ; un nil écrit directement par un test après `STOP` n'est pas une preuve, la lecture devient « présent 0 ») ; R5 vacante ; **`g_fr_defaults` supprimée, `planet_family_trade.c` sans table d'offsets** ; §6 : aucune correspondance ; empreinte réelle mesurée au commit.
- Le message de commit porte : compte 96 → 95, résumé « Runs », `gcov`, mutations et trous, `FR_MT_HASH`, réponses à H1–H12.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
