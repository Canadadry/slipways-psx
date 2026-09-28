---
title: "Plan 3, étape 1b.30 : `nr`, la porte de glisse. `hz` en `pico8_ob_table_t *` et `rr` en `const char *`, pointeurs de spec ; `g_nr_defaults` supprimée, `g_fr_defaults` réduite à `mt`"
description: "Après 1b.29, `pico8_nr_t` garde 2 membres `pico8_value_t` : `hz` (table de spec lue par `nr:hb`) et `rr` (chaîne de spec). 1b.30 les type selon R1–R9 de 1b.1, sans indicateur ni écrivain ; `g_nr_defaults` se vide et est supprimée ; la fille `fr` perd ses entrées `hz`/`rr` (R7 de 1b.14a) et déballe les siennes en `static`. `make value-members` passe de 98 à 96."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.30 — `nr` (`planet_family_trade.h:27`). Empreinte 4 (1, 3). Compte : 96.

Cadre : puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », §3 (`make test` vert, aucun `*.sha256` modifié), §6, §7. Modèle : le PRD de 1b.28 (`docs/prd/done/plan3-phase1b-etape28-mx-synthetiseur.md`, même fichier). Notes **citées, pas redérivées** :

- **1b.1, puis 1b.3 à 1b.28** : patron R1–R9. En particulier **1b.14a** (R7 : table de fille qui garde des entrées), **1b.21** (pas d'indicateur sans lecture nil par nom), **1b.24/1b.25** (tables vidées supprimées, preuve de typedef par `awk`, caractérisation dans le TU rapide de la famille), **1b.27** (R5 vacante sans écrivain : pas de test d'arrêt).
- **Patron des défauts `static`** (R7 de 1b.14a, repris en 1b.23–1b.28) : déballage une fois à l'init dans `static g_<X>_<m>` au même pointeur, remise à nil dans `defaults_reset`, pose typée à la place de l'ancienne boucle, avant les clés et tout `pq`.
- **Forme écrivain de 1b.13/1b.15** : sans objet ici (pas d'écrivain, voir R5).
- **1b.23d** : un `written[]` vidé est supprimé avec sa boucle — sans objet ici (aucun `written[]` ne liste `hz`/`rr`).
- **1b.11 — R2 des TU lents** (`test_pico8_nr` est lent) : revue plus bas.

### État de départ

HEAD `0db111f` (PRD de 1b.29). **Précondition : 1b.29 est commitée avant que 1b.30 commence (H1).** `make value-members` rend alors **98**. Numéros de ligne indicatifs (arbre `0db111f` ; 1b.29 ne touche pas `planet_family_trade.[ch]`, H2). **Rien n'a été compilé ni lancé pour ce PRD** : tout fait d'exécution est une hypothèse (H1–H12).

### Lignes de recensement (`tools/member_census.txt:127-128`)

| membre | accès | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|---|
| `hz` | offset | `ref:table` | observé | stat | l | `pico8_ob_table_t *` (spec de `nr` ou de `fr`) | non (pointeur) |
| `rr` | offset | `chaine` | observé | stat? | l | `const char *` (spec de `nr` ou de `fr`) | non (pointeur) |

Compte : 98 − 2 = **96**. Aucune ligne `pico8_nr_t` ne reste. Le `stat` de `hz` est la garde `self->hz.as.ref != NULL` de `pico8_nr_hb` (port).

### Le cart (`pico8/lua.lua`)

- `nr=qh:jr` (`:1101-1113`) : `hz=o(170,170,170,168)`, `rr="slipgates"` ; `fr=nr:jr` (`:1123-1127`) : `rr="infraspace"`, `hz=o(78,78,78,76)`.
- `nr:hb` (`:1114`) : `self.s=self.hz[#self.oe+1]`. Seule lecture de `hz`. `rr` n'est lu que par nom (`ie:hb` via `self.ke.rr`, `:926`). Jamais écrits.

### Le code (`src/pico8_planet_family_trade.[ch]`)

- **Typedef** `pico8_nr_t` (`.h:27-31`) ; ses commentaires et celui de `pico8_fr_t` restent vrais.
- **Table** `g_nr_defaults = {hz, rr}` (`.c:~235`) et ses 3 clients : boucle de `pico8_nr_defaults_reset`, boucle de recopie de `pico8_nr_struct_init` (après les deux `= pico8_value_ref(NULL)`), boucle de lecture + `assert` « nr: every default … » de `pico8_planet_family_trade_init`.
- **Fille** `g_fr_defaults = {mt, rr → nr.rr, hz → nr.hz}` (`.c:~397`), recopiée dans `pico8_fr_struct_new` après `pico8_nr_struct_init`.
- **Lecteur** `pico8_nr_field` : branches `hz`/`rr` (pointeur `member`, REF NULL → 0), repli sur `qh`. `pico8_fr_field` y délègue.
- **Écrivain** `pico8_nr_set_field` : `s` seul, puis `qh` (20 noms). **Aucune branche `hz`/`rr`** : leurs écritures avortent déjà (`write_hz`/`write_rr` dans `test_pico8_nr` et `test_pico8_fr`).
- **Accès direct (R6)** : `pico8_nr_hb` seul, lecture `self->hz.kind == REF && self->hz.as.ref != NULL` puis `get_index`.
- **Aides de défaut** existantes : `family_trade_default_table`, `family_trade_default_string` (message « mx.rr »).

### Revue des TU lents (R2 de 1b.11)

`test_pico8_nr` (lent), `test_nr_is_a_struct` : `bare->hz = pico8_value_ref(NULL); bare->rr = pico8_value_ref(NULL);` (**accès direct**, `.c:739-740`) puis `!cls->field(bare, "hz"/"rr")` : nil lu par nom, mais sur des pointeurs, NULL suffit : **aucun indicateur**. Ailleurs dans ce TU et dans `test_pico8_fr` : lectures par nom seules (`gate_kind`, `fold_slipgate`, `fold_gate`, `inherited[]`, `must_abort(write_hz/rr)`), inchangées.

## Solution

1. **Caractérisation figée sur l'ancien code** (arbre 1b.29 commité), dans le TU rapide `test_pico8_fr`, lisant `hz`/`rr` par nom seul ; verte avant tout typage.
2. **Commit 1b.30** : `hz`/`rr` typés ; `g_nr_defaults` supprimée avec ses 3 boucles ; `g_nr_hz`/`g_nr_rr` et `g_fr_hz`/`g_fr_rr` déballés en `static` ; `g_fr_defaults` réduite à `mt` ; `test_pico8_nr` réécrit ses deux affectations directes ; `make value-members` rend **96**.
3. Pas de test d'arrêt : **aucun écrivain** (R5 vacante, précédent 1b.27).
4. Aucun pixel ne bouge. Note « Découvert à 1b.30 » sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `planet_family_trade.h`, je veux `pico8_nr_t` sans `pico8_value_t`.
2. En tant que lecteur par nom, je veux que `hz` d'une porte neuve rende la REF de la table de la spec de **sa** classe (`nr` : 170,170,170,168 ; `fr` : 78,78,78,76), même pointeur qu'avant.
3. En tant que lecteur par nom, je veux que `rr` rende la chaîne de la spec de sa classe (« slipgates », « infraspace »), même pointeur.
4. En tant que `nr:hb`, je veux `s = hz[#oe+1]` inchangé pour `nr` et `fr`.
5. En tant que gardien du §5, je veux qu'une écriture par nom de `hz`/`rr` avorte toujours.
6. En tant que mainteneur, je veux `g_nr_defaults` supprimée et aucun `offsetof(pico8_nr_t` ni `offsetof(pico8_fr_t, nr.` dans `src/`.
7. En tant qu'auteur de 1b.31, je veux `g_fr_defaults` = `{mt}` avec sa boucle et sa remise à nil intactes.
8. En tant que gardien du §3, je veux `make test` vert et aucun `*.sha256` touché.
9. En tant que `plan-pilot`, je veux `make value-members` à 96 sans ligne `pico8_nr_t`.
10. En tant que vérificateur, je veux caractérisation verte sur l'ancien code, mutations et trous justifiés, rapport `gcov`, tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle : les validations que demande `to-prd` (modules, tests) sont tranchées comme le ferait un agent `guideline` — hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables — et marquées (G).

### R1–R9 appliquées

| règle | sur `nr` |
|---|---|
| R1 | `pico8_qh_t qh; pico8_ob_table_t *hz; const char *rr;` — empruntés à la spec, jamais libérés (TROU-7 inchangé). |
| R2 | Aucun indicateur (pointeurs ; nil lu seulement par le TU lent, NULL suffit). |
| R3 | `pico8_nr_struct_init` : `self->hz = g_nr_hz; self->rr = g_nr_rr;` remplacent les deux `REF NULL` et la boucle ; `size_t i` retiré (`-Werror`). |
| R4 | `hz` : NULL → 0, sinon `pico8_value_ref(hz)` ; `rr` : NULL → 0, sinon `pico8_value_string(rr)` (forme `hj.rr`/`nl.rr`). `*out_value` intact sur 0. |
| R5 | **Vacante** : aucune branche d'écrivain, aucune clé de construction. Pas de test d'arrêt (G). |
| R6 | `pico8_nr_hb` : `if (self->hz != NULL)` puis `get_index(self->hz, …)`. `test_pico8_nr.c:739-740` : `bare->hz = NULL; bare->rr = NULL;`. |
| R7 | `g_nr_defaults` supprimée avec ses 3 boucles et son `assert` ; `size_t i` de `pico8_nr_defaults_reset` retiré, celui de l'init reste (boucle `fr`). **Fille `fr`, R7 de 1b.14a** : entrées `rr`/`hz` retirées de `g_fr_defaults`, qui garde `mt`, sa boucle et sa remise à nil. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_nr_t)` gardé ; `tools/member_census.txt` non édité. |
| R9 | 98 → 96. |

### Défauts déballés (patron `static`) (G)

- `static pico8_ob_table_t *g_nr_hz`, `static const char *g_nr_rr`, déballés dans l'init après `g_nr_dw` par `pico8_nr_default` et `family_trade_default_table` / `family_trade_default_string` ; remis à NULL dans `pico8_nr_defaults_reset`.
- `static pico8_ob_table_t *g_fr_hz`, `static const char *g_fr_rr`, déballés après la boucle `g_fr_defaults` par `pico8_fr_default` ; remis à NULL dans `pico8_fr_defaults_reset` ; posés dans `pico8_fr_struct_new` après la boucle (`self->nr.hz = g_fr_hz; self->nr.rr = g_fr_rr;`), avant `od`/`s`, les clés et `pq`.
- Messages des deux aides élargis (`nr/fr.hz`, `nr/fr.rr`) ; aucune aide nouvelle.

### Revue des tests de régression, mécanique

**Liste** : les tests nommés au §6 (les 17 de régression) et dans les lignes 1a : `tests/test_pico8_<nom>.c` pour `built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`, `bg`, `entity_kind`, `hj`, `jc`, `jo`, `jo_spec_defaults`, `js`, `mo`, `mz`, `nl`, `os`, `pm`, `qh`, plus `tests/test_class_coverage.c` (`class_chain` et `entity_smoke` n'existent plus).

**Greps** (faits pour ce PRD) : accès direct `grep -cE '(->|\.)(hz|rr)\b|offsetof\([^)]*\b(hz|rr)\)'` → **0 partout**. Par nom `"(hz|rr)"` : `entity_kind` (lecteurs de genre, spec, pas le membre), `jo_spec_defaults` (`NAMES[]` ∋ `hz`, `rr`, lecture seule sur `nr`/`fr` neufs), `hj`/`nl`/`os`/`pm` (autres structs). **Aucune preuve rouge due**, aucun diff (H6).

### Preuves (sortie attendue sur l'arbre final)

- `awk '/^typedef struct \{/{b=""} {b=b $0 "\n"} /\} pico8_nr_t;/{printf "%s", b}' src/pico8_planet_family_trade.h | grep -c pico8_value_t` : **0** ;
- `make value-members | grep -c pico8_nr_t` : **0** ;
- `grep -rnw g_nr_defaults src/`, `grep -rn 'offsetof(pico8_nr_t' src/`, `grep -n 'nr\.\(hz\|rr\)' src/pico8_planet_family_trade.c | grep offsetof` : **vides** ;
- `grep -n '"mt"' src/pico8_planet_family_trade.c` : l'entrée `g_fr_defaults` et le lecteur, inchangés ;
- `git diff --stat HEAD -- src/ ':!src/pico8_planet_family_trade.h' ':!src/pico8_planet_family_trade.c' tools/ Makefile` : **vide** ;
- dans `tests/`, seuls `test_pico8_fr.c` (ajouts seuls) et `test_pico8_nr.c` (les deux affectations seules) changent.

### Empreinte et découpage

Ligne : 4 (1, 3). Estimée : 4 (2, 2) — `.h`, `.c`, `test_pico8_nr`, `test_pico8_fr` (H10). **Aucun découpage (G)**, un commit.

### Hypothèses (rien n'a été lancé)

- **H1** : 1b.29 commitée ; `make value-members` rend 98 au départ, 96 à la fin, seule la ligne `pico8_nr_t` disparaît.
- **H2** : 1b.29 ne modifie ni `planet_family_trade.[ch]` ni `test_pico8_fr`/`test_pico8_nr`.
- **H3** : sur toute porte neuve, `hz`/`rr` sont les pointeurs de la spec de sa classe (valeurs figées = valeurs **observées**).
- **H4** : `test_pico8_fr` reste sous 30 s avec une fonction de plus.
- **H5** : aucune valeur figée ne bouge : hash de `test_pico8_nr` (`SESSION_HASH`, `HUB_HASH`…), de `test_pico8_fr` (`BUILT_HASH`, `LIFE_HASH`), de `jo_spec_defaults`, `test_pico8_ui`, 15 empreintes du filet.
- **H6** : aucun test de régression ne dépend de la représentation de `hz`/`rr`.
- **H7** : `make sanitize-net` propre (aucune allocation nouvelle).
- **H8** : le build `-DPICO8_MEMBER_CENSUS` compile.
- **H9** : `-Werror` muet (deux `size_t i` retirés, aides toujours appelées).
- **H10** : empreinte réelle comptée au commit.
- **H11** : `test_pico8_nr` tourne en moins de 5 min seul (mutation 1).
- **H12** : `hb` avec `oe` de taille 0 à 3 posé par nom (`list_of`, forme `test_life`) donne 170,170,170,168 (`nr`) et 78,78,78,76 (`fr`).

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom, ou l'écran.** La caractérisation n'accède jamais à `hz`/`rr` directement : elle compile sur les deux formes.
- **(G) Où : `test_pico8_fr`**, rapide, dans `CENSUS_UNIT_TESTS`, TU de la fille qui construit déjà des `nr` (`build(pico8_nr_kind())`) et a `list_of`, `place`, `build_gate`, `class_ref`, `in_fresh_child` ; il juge aussi la pose R7 de `fr`. `test_pico8_nr` est lent (1b.11) ; `test_pico8_ui` est écarté (1b.29/1b.31 en évitent les collisions).
- **Caractérisation `test_nr_members_by_name`**, par `in_fresh_child` après `test_fr_is_a_struct`, hash propre **`NR_MEMBERS_HASH`**. Lecture par le lecteur brut (`cls->field`, témoin `0x1234`) **et** par `pico8_entity_get_field`, qui doivent concorder ; adresses jugées par `assert`, jamais pliées.
  1. **Neufs** : `pico8_nr_new(NULL)`, un `nr` par le genre, `pico8_fr_new(NULL)`. `hz` = REF au pointeur de `pico8_<classe>_default("hz")`, identique d'une instance à l'autre de même classe, **distinct** entre `nr` et `fr` ; 4 entrées pliées. `rr` = chaîne au pointeur de `pico8_<classe>_default("rr")`, octets pliés.
  2. **`hb`** : pour `oe` de taille 0 à 3 (par nom), `s` lu et plié, sur un `nr` et un `fr` ; `oe` d'origine remis.
  3. **Écriture par nom** : `hz`/`rr` refusés (déjà `must_abort`, pas repris).
  4. **Intrus** : `""`, `"h"`, `"hz "`, `"HZ"`, `"hz_"`, `"r"`, `"rr "`, `"RR"` rendent 0 ; `qh` brut (`pico8_qh_struct_class()->field`) ne voit ni `hz` ni `rr` sur `nr`.
  5. Tout plié dans `NR_MEMBERS_HASH`, relevé au premier passage vert sur l'arbre de départ, figé ensuite.
- **Pas de test d'arrêt** : pas d'écrivain (G, 1b.27).
- **Mutations (§7)**, dans une copie jetable du scratchpad, par `make tmp/test_pico8_fr && ./tmp/test_pico8_fr` sauf mention, notées dans le commit :
  1. **(§7-1) indicateur forcé vrai** : sans objet dans sa lettre (pas de `has_`). **Forme voisine** : la branche `hz` du lecteur ne teste plus NULL → rouge : `test_pico8_nr` (`test_nr_is_a_struct`, `!cls->field(bare, "hz")`), lancé **une fois** (H11) ; sinon noté non lancé.
  2. **(§7-2) « présent 0 »** : sans objet (aucun fixe typé). **Forme voisine** : `rr` rendu en `pico8_value_ref(rr)` → rouge : caractérisation 1 (genre).
  3. **(§7-3) init qui saute un membre** : la lettre s'applique à `fr` — (a) pose `self->nr.hz = g_fr_hz` retirée (la porte garde le `hz` de `nr`) → rouge : caractérisation 1 et 2 ; (b) idem pour `rr` → rouge : caractérisation 1. Pour `nr`, plus de boucle ; forme « pose une autre valeur » : (c) `self->hz = g_fr_hz` en R3 → rouge : caractérisation 1.
  4. **R6** : `hb` lit `g_nr_hz` au lieu de `self->hz` → rouge : caractérisation 2 (`fr`).
  5. Remise à NULL retirée de `defaults_reset` : **mutant équivalent, noté** (l'init redéballe avant toute lecture).
- **`gcov`** : `test_pico8_fr` en `--coverage -O0`, piège `__gcov_dump` des fils, sur `planet_family_trade.c`. Lignes attendues exécutées : déballages et remises à NULL, poses R3 et `fr`, branches non-NULL de R4, R6. **Trous attendus** : branches NULL de R4 (seul le TU lent y passe — mutation 1) et échecs d'`assert`. Rapport dans le commit.
- **Prior art** : `test_mx_members_by_name` (1b.28), `test_pm_members_by_name` (1b.27) de `test_pico8_ui` ; `test_life`, `test_fr_is_a_struct` de `test_pico8_fr`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque.

- `./tmp/test_pico8_fr` : ≤ 30 s (H4) ; ~9 lancements (départ, final, mutations 2, 3a, 3b, 3c, 4, 5, `gcov`) ≈ 4 min.
- `./tmp/test_pico8_nr` (mutation 1) : une fois, ≤ 5 min (H11).
- `make test` une fois ; `make e2e` une fois, **dans la même invocation** que `make class-coverage-net` ; `make sanitize-net` une fois, jamais en même temps que `e2e` ; `make value-members`.
- TU lents : compiler tôt sans lancer (`make -j tmp/test_pico8_nr tmp/test_pico8_mx tmp/test_pico8_qh`).

| # | Commande exacte | État de l'arbre (départ / final / mutation n) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, les **8** TU de `UNIT_TESTS_SLOW` verts ;
3. `timeout 15m make sanitize-net` : **15 propre(s)** ;
4. `make value-members` : **96**, aucune ligne `pico8_nr_t` ;
5. `timeout 15m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` — requis, `pico8_nr_struct_init` et `pico8_fr_struct_new` changent.

On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

- `FAILED:` si 1b.29 n'est pas commitée au départ, ou si `make value-members` ne rend pas 98 au départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, ou si le diff touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements.
- `FAILED:` si `hz` n'est pas `pico8_ob_table_t *` ou `rr` pas `const char *`, si un `has_` est ajouté, ou si une porte neuve n'a pas les **pointeurs de la spec de sa classe**.
- `FAILED:` si `g_nr_defaults` ou un `offsetof` vers `hz`/`rr` existe encore, si `g_fr_defaults` ne garde pas `mt` avec sa boucle, ou si un `static` nouveau n'est pas remis à NULL dans son `defaults_reset`.
- `FAILED:` si un écrivain ou une clé de construction de `nr` accepte `hz`/`rr`.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, lit `hz`/`rr` directement, ou si `NR_MEMBERS_HASH` change après.
- `FAILED:` si un autre fichier de `src/` que `planet_family_trade.[ch]`, `tools/` ou le `Makefile` change ; si `test_pico8_nr.c` change autrement que par ses deux affectations ; si un test de régression a un diff ; si une valeur figée existante change (H5).
- `FAILED:` si le commit ne porte pas le rapport `gcov` et les mutations 1 à 5, chacune avec son test ou la justification de son trou.
- `FAILED:` si une preuve ne rend pas l'attendu, si `make sanitize-net` n'est pas `15 propre(s)`, ou s'il manque la ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` si le travail n'est pas en un commit vert, si le budget est dépassé sans note de ce qui n'a pas été lancé, ou s'il manque le tableau « Runs » ou la note « Découvert à 1b.30 ».

## Out of Scope

- `fr.mt` (1b.31) ; les membres de `qh`/`jo` portés par `nr` (déjà typés) ; `mx`.
- `hb` avec `#oe ≥ 4` (le cart rend nil, le port 0) : non caractérisé, non touché.
- Libérer les specs (TROU-7) ; régénérer `tools/member_census.txt` ; déplacer un TU entre listes.
- Fichiers d'événements, `FRAMES_EVENTS`, rendu, audio, appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur (G)** : modules `planet_family_trade.h` (typedef) et `.c` (lecteur, R3, R6, R7 sur `fr`, 4 `static`, messages d'aides) ; types pointeurs sans indicateur ; pas d'écrivain, pas de test d'arrêt ; tests : `test_pico8_fr` (caractérisation), `test_pico8_nr` (deux affectations) ; pas de découpage.
- **Note « Découvert à 1b.30 »** (à écrire sur la ligne) : patron 1b.1 appliqué ; `hz` → `pico8_ob_table_t *`, `rr` → `const char *`, pointeurs de spec, aucun indicateur ; R5 vacante ; R7 de 1b.14a sur la fille `fr` (`g_fr_defaults` = `{mt}`), **`g_nr_defaults` supprimée** ; §6 : aucune correspondance ; caractérisation dans `test_pico8_fr` ; empreinte réelle mesurée au commit.
- Le message de commit porte : compte 98 → 96, résumé « Runs », `gcov`, mutations et trous, `NR_MEMBERS_HASH`, réponses à H1–H12.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
