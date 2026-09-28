---
title: "Plan 3, étape 1b.28 : `mx`, le synthétiseur. `ht` en `pico8_ob_table_t *` (pointeur de spec, puis table neuve de `mx:kf`), écrivain de 1b.13/1b.15 ; `g_mx_defaults` supprimée"
description: "Après 1b.27, `pico8_mx_t` ne garde qu'un membre `pico8_value_t` : `ht`, la table `o(28,20,21,22)` de la spec que `mx:kf` remplace par une table vide neuve. 1b.28 le type en `pico8_ob_table_t *` selon R1–R9 de 1b.1, sans indicateur, avec la branche d'écrivain de 1a gardée sous la forme 1b.13/1b.15 et son test d'arrêt. `g_mx_defaults` se vide et est supprimée ; le défaut est déballé en `static` (R7 de 1b.14a). `make value-members` passe de 110 à 109."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.28 — `mx` (`planet_family_trade.h:49`), `ht`. Empreinte 2 (1, 1). Compte : 109.

Cadre : puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », §3 (`make test` vert, aucun `*.sha256` modifié), §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Modèle : le PRD de 1b.27 (`docs/prd/triage/plan3-phase1b-etape27-pm-protoetoile.md`, petite fille de planète). Notes **citées, pas redérivées** :

- **1b.1, puis 1b.3 à 1b.26** : patron R1–R9. En particulier **1b.11** (R2 des TU lents), **1b.21** (pas d'indicateur sans lecture nil par nom), **1b.23a2** (un membre `l` que l'écrivain de 1a accepte garde sa branche ; il sort des `written[]`, son écriture par nom est figée par la caractérisation), **1b.24/1b.25** (tables vidées supprimées ; preuve de typedef par `awk`).
- **Patron des défauts `static` (R7 de 1b.14a, repris en 1b.23–1b.27)** : déballage une fois à l'init dans `static g_<X>_<m>` au même pointeur, remise à nil dans `defaults_reset`, pose typée à la place de l'ancienne boucle, avant les clés et tout `pq`.
- **Forme écrivain de 1b.13/1b.15** (reprise sur `ref:liste` par `hj.qy` en 1b.26, `family_take_list`) : aide `static` par genre et par fichier, état nil posé d'abord, sortie sur `REF NULL` (genre **et** pointeur), sinon `assert` « champ sans règle » nommant `struct.membre` et le genre ; test d'arrêt `must_abort` rouge sur l'ancien code.
- **1b.23d** : un tableau `written[]` vidé est supprimé avec sa boucle, les autres variables restent.

### État de départ

**1b.27 est en cours d'implémentation (l'arbre porte déjà `PM_MEMBERS_HASH` non commité). Précondition : 1b.27 est commitée avant que 1b.28 commence (H1).** Alors : `make value-members` rend **110** ; `pico8_mx_t` = `{ pico8_hj_t hj; pico8_value_t ht; }` ; `g_mx_defaults` ne liste que `ht` ; les 18 autres défauts de `mx` sont déjà en `static g_mx_*`. Numéros de ligne indicatifs (arbre 1b.27). **Rien n'a été compilé ni lancé pour ce PRD** : tout fait d'exécution est une hypothèse (H1–H13).

### Ligne de recensement (`tools/member_census.txt:130`)

| membre | accès | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|---|
| `ht` | offset | `ref:table` | observé | stat | l | `pico8_ob_table_t *` | non (pointeur) |

Compte : 110 − 1 = **109**. Aucune ligne `pico8_mx_t` ne reste. Le `stat` est le test `self->ht.as.ref != NULL` de `pico8_mx_kf` (port, pas cart).

### Le cart (`pico8/lua.lua`)

- `mx=hj:jr[[…]]` (`:1036-1049`) : `ht=o(28,20,21,22)`.
- `mx:kf` (`:1049-1055`) : pour chaque `i` de `sw:eb(self).jy` avec `i.gc==0 and gu(self.ht,i.qe)`, `self.kw.qe,self.ht=i.qe,{}`. Seules lecture et écriture de `ht`. Jamais nil dans le cart.

### Le code (`src/pico8_planet_family_trade.[ch]`)

- **Typedef** `pico8_mx_t` (`.h:47-50`) ; son commentaire (`ht` = table de la spec, `mx:kf` la remplace par une table neuve jamais libérée) reste vrai.
- **Table** `g_mx_defaults = {ht}` (`.c:~558`), et ses 3 clients : boucle de recopie dans `pico8_mx_struct_new` (après `self->ht = pico8_value_ref(NULL)`), boucle de lecture + `assert` « mx: every default … » dans `pico8_planet_family_trade_init`, boucle de `pico8_mx_defaults_reset`.
- **Lecteur** `pico8_mx_field` : branche `ht` (REF NULL → 0), repli sur `hj`.
- **Écrivain** `pico8_mx_set_field` : branche `ht` qui recopie **toute** valeur, puis repli sur `hj` (21 noms).
- **Accès directs (R6)**, tous dans `pico8_mx_kf` : la lecture `self->ht.kind == REF && self->ht.as.ref != NULL` → `ht_table`, et l'écriture `self->ht = pico8_value_ref(pico8_ob_table_new())`.
- **Aide de défaut** déjà présente : `family_trade_default_table` (message « nr/mx.nu, mx.px, mx.k_ … (ref:table) »).
- Aide d'écrivain `ref:table` : **aucune** dans ce fichier (`family_trade_take_fixed/string/vec` seulement).

### Revue des TU lents (R2 de 1b.11)

`test_pico8_mx` (lent) : `mx_tables[]`/`get_ref(e, "ht")` comparé à `class_ref(pico8_mx_default, "ht")` (pointeur de spec), `has`/`ht_size` dans `fold_synthesizer`, `test_synthesis` (swap de `kf`), et dans `test_mx_is_a_struct` :

- `written[] = {"ht"}` : sa boucle écrit **un fixe** dans `ht` par nom et le relit. Hors ligne (`ref:table`) : il avortera après typage.
- `pico8_entity_set_field(bare, "ht", REF NULL)` puis `!cls->field(bare, "ht")` : **nil lu par nom** (`lu` au sens de 1b.11). `ht` étant un pointeur, NULL suffit : **R2 ne donne aucun indicateur**.

`test_pico8_hj`, `pm`, `nr` (lents), `fr`, `ga` : `not_*` ∋ `ht` et `write_fixed(e, "ht")` sur d'autres structs, inchangés. `test_pico8_jo_spec_defaults` (rapide) : `NAMES[]` ∋ `ht`, lecture par nom seule.

## Solution

1. **Caractérisation figée sur l'ancien code** (arbre 1b.27 commité), dans le TU rapide `test_pico8_ui`, lisant et écrivant `ht` par nom seul ; verte avant tout typage.
2. **Test d'arrêt**, rouge sur l'ancien code (l'écrivain y recopie tout).
3. **Commit 1b.28** : `ht` typé ; écrivain sous forme 1b.13/1b.15 ; `g_mx_defaults` supprimée avec ses 3 boucles ; `g_mx_ht` déballé en `static` ; `written[]` de `test_pico8_mx` supprimé avec sa boucle ; `make value-members` rend **109**.
4. Aucun pixel ne bouge. Note « Découvert à 1b.28 » sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `planet_family_trade.h`, je veux `pico8_mx_t` sans `pico8_value_t`.
2. En tant que lecteur par nom, je veux que `ht` d'un `mx` neuf rende la REF de la table de la spec (même pointeur, 4 entrées 28, 20, 21, 22), comme avant.
3. En tant que `mx:kf`, je veux remplacer `ht` par une table vide neuve quand une ligne non fournie a un `qe` de `ht`, laisser la table de la spec intacte, et ne plus rien accepter ensuite.
4. En tant qu'écrivain par nom, je veux que nil éteigne `ht` (lecture suivante 0) et qu'une table soit reprise au même pointeur.
5. En tant que gardien du §5, je veux qu'une écriture de `ht` hors `ref:table` (fixe, booléen, chaîne, fonction, liste, struct, vecteur, unions nulles) avorte.
6. En tant que mainteneur, je veux `g_mx_defaults` supprimée et plus aucun `offsetof(pico8_mx_t` dans `src/`.
7. En tant que mainteneur, je veux l'écrivain de `mx` acceptant toujours exactement `ht` plus les 21 noms de `hj`.
8. En tant qu'auteur de 1b.29 et suivantes (`nr`, `fr`), je veux `g_nr_defaults`/`g_fr_defaults` et `planet_family.[ch]` intacts.
9. En tant que gardien du §3, je veux `make test` vert et aucun `*.sha256` touché.
10. En tant que `plan-pilot`, je veux `make value-members` à 109 sans ligne `pico8_mx_t`.
11. En tant que vérificateur, je veux caractérisation verte sur l'ancien code, test d'arrêt rouge puis vert, mutations et trous justifiés, rapport `gcov`, tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle : les validations que demande `to-prd` (modules, tests) sont tranchées comme le ferait un agent `guideline` — hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables — et marquées (G).

### R1–R9 appliquées

| règle | sur `mx` |
|---|---|
| R1 | `pico8_hj_t hj; pico8_ob_table_t *ht;` — emprunté (spec ou table neuve de `kf`), jamais libéré (TROU-7 inchangé). |
| R2 | Aucun indicateur (pointeur ; le nil lu par le TU lent est NULL). |
| R3 | `self->ht = g_mx_ht;` remplace `self->ht = pico8_value_ref(NULL)` et la boucle, avant les poses de `hj` existantes, les clés et tout `pq`. `size_t i` de `pico8_mx_struct_new` retiré (`-Werror`). |
| R4 | Branche `ht` : NULL → `return 0`, `*out_value` intact ; sinon `pico8_value_ref(self->ht)`. |
| R5 | Écrivain : `family_trade_take_table(value, &self->ht)` (nouvelle, forme `family_take_list` de 1b.26) : `*out = NULL` ; `return` sur `REF NULL` ; `assert(REF && ref != NULL && tag == PICO8_TYPE_TAG_OB_TABLE && "mx.ht: a write neither nil nor a table is off its census row (ref:table) -- a field under no rule of plan-3 section 5: stop, do not widen the member")` ; recopie. Branche gardée malgré `l` seul (lecture 1b.23a2). Aucune clé de construction (`id` seule mesurée). |
| R6 | `pico8_mx_kf` : lecture → `ht_table = self->ht;` (garde NULL conservée) ; écriture → `self->ht = pico8_ob_table_new();`. Commentaire « previous `ht` … never freed » gardé. |
| R7 | `g_mx_defaults` supprimée avec ses 3 boucles et l'`assert` de lecture ; `size_t i` de `defaults_reset` retiré ; celui de l'init reste (boucles `nr`/`fr`). Pas de fille. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_mx_t)` gardé ; `tools/member_census.txt` non édité. |
| R9 | 110 → 109. |

### Défaut déballé (patron `static`) (G)

- `static pico8_ob_table_t *g_mx_ht = NULL;`, déballé dans `pico8_planet_family_trade_init` par `pico8_mx_default("ht", &v)` et **`family_trade_default_table`** (existante, message étendu à `mx.ht`) ; remis à NULL dans `pico8_mx_defaults_reset`.
- Écarté : un littéral `o(28,20,21,22)` construit à part. La spec est la seule source, et le pointeur est figé par `mx_tables[]` du TU lent.

### Écrivain et test d'arrêt (G)

`ht` est écrit par nom (TU lent `test_pico8_mx` : fixe dans `written[]`, nil sur `bare`) et l'écrivain de 1a l'accepte : forme 1b.13/1b.15 appliquée, avec **`test_mx_writer_stops_off_row`** dans `test_pico8_ui`, forme `test_hj_writer_stops_off_row` (`g_off_row_name = "ht"`, `write_off_row`, `zero_union`) sur `pico8_mx_new(NULL)` : fixe 1, fixe à union nulle, `true`, `false`, booléen à union nulle, chaîne `"x"`, chaîne à union nulle, fonction à union nulle, **liste** neuve, `jo` neuf, vecteur. Rouge sur l'ancien code (aucun avortement), vert après.

### `written[]` de `test_pico8_mx`

`ht` sort de `written[]` (1b.23a2) ; le tableau vidé est **supprimé avec sa boucle** (1b.23d). `i` et `v` restent utilisés. L'écriture nil sur `bare` et son `assert` restent. Aucun crochet `STOP` n'y est (TU hors recensement). Seul diff d'un TU lent.

### Revue des tests de régression (§6), mécanique

**Liste** : tests nommés au §6 et dans les lignes « Tests : » des étapes 1a, soit 18 fichiers : `tests/test_pico8_<nom>.c` pour `built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`, plus `tests/test_class_coverage.c`.

**Greps** (faits pour ce PRD) : accès direct `grep -cE '(->|\.)ht\b|offsetof\([^)]*\bht\)'` → **0 partout** ; par nom `grep -cE '"ht"'` → **0 partout** ; chemin `grep -cE 'pico8_mx_|synthesi|mx_kind'` (et `"mx"` pour `test_class_coverage`) → **0 partout**. **Aucune preuve rouge due**, 18 diffs vides (H6).

### Preuves (sortie attendue sur l'arbre final)

- `awk '/^typedef struct \{/{b=""} {b=b $0 "\n"} /\} pico8_mx_t;/{printf "%s", b}' src/pico8_planet_family_trade.h | grep -c pico8_value_t` : **0** ;
- `make value-members | grep -c pico8_mx_t` : **0** ;
- `grep -rnw g_mx_defaults src/` et `grep -rn 'offsetof(pico8_mx_t' src/` : **vides** ;
- `grep -n 'self->ht = ' src/pico8_planet_family_trade.c` : exactement la pose R3 et l'écriture de `kf` ;
- `grep -n family_trade_take_table src/pico8_planet_family_trade.c` : la définition et un appel ;
- `git diff --stat HEAD -- src/ ':!src/pico8_planet_family_trade.h' ':!src/pico8_planet_family_trade.c' tools/ Makefile` : **vide** ;
- dans `tests/`, seuls `test_pico8_ui.c` (ajouts seuls) et `test_pico8_mx.c` (suppression du `written[]` et de sa boucle seule) changent.

### Empreinte et découpage

Ligne : 2 (1, 1). Estimée : 3 (2, 1) — `.h`, `.c`, `test_pico8_mx` (H10). Fichiers modifiés : 4 plus le plan. **Aucun découpage (G)**, un commit.

### Hypothèses (rien n'a été lancé)

- **H1** : 1b.27 commitée ; `make value-members` rend 110 au départ, 109 à la fin, seule la ligne `pico8_mx_t` disparaît ; `g_mx_defaults = {ht}`.
- **H2** : aucun site de `src/`, rejeu ou TU rapide n'écrit `ht` par nom ; seul `kf` l'écrit directement.
- **H3** : sur tout `mx` neuf, `ht` est la REF de la table de la spec (même pointeur que `pico8_mx_default("ht")`), 4 entrées positionnelles 28, 20, 21, 22. Valeurs figées = valeurs **observées**.
- **H4** : `test_pico8_ui` reste sous 30 s avec deux fonctions de plus.
- **H5** : aucune valeur figée ne bouge : hash de `test_pico8_mx` (dont `SYNTHESIS_HASH`), de `jo_spec_defaults`, de `test_pico8_ui` (dont `PM_MEMBERS_HASH`, `HJ_MEMBERS_HASH`), 15 empreintes du filet.
- **H6** : on ignore quel rejeu de régression construit un `mx` ; sans effet (diffs vides).
- **H7** : `make sanitize-net` propre (seule allocation : celle de `kf`, déjà là).
- **H8** : le build `-DPICO8_MEMBER_CENSUS` compile.
- **H9** : `-Werror` muet (deux `size_t i` retirés, `family_trade_take_table` appelée une fois).
- **H10** : empreinte réelle 3 (2, 1), comptée au commit.
- **H11** : dans `test_pico8_ui`, la forme `join`/`give_row` de `test_synthesis` (`pico8_jc_new` avec `qu` = un `hj`, `jy` écrit par nom, `ey` sur le `mx`, crédits posés) déclenche `mx:kf` hors rejeu, en quelques secondes.
- **H12** : `test_mx_is_a_struct` ne plie aucun hash ; supprimer la boucle `written[]` ne change aucune valeur figée.
- **H13** : `test_pico8_jo_spec_defaults` plie `ht` à l'identique (même REF).

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom, ou l'écran.** La caractérisation n'accède jamais à `ht` directement : elle compile sur les deux formes.
- **(G) Où : `test_pico8_ui`**, rapide, dans `CENSUS_UNIT_TESTS`, déjà hôte de `hj` (1b.26) et `pm` (1b.27) ; `test_pico8_mx` est lent (1b.11).
- **Caractérisation `test_mx_members_by_name`**, lancée par `in_fresh_child` après `test_pm_members_by_name`, hash propre **`MX_MEMBERS_HASH`**. Lecture par le lecteur brut (`cls->field`, témoin `0x1234`) **et** par `pico8_entity_get_field`, qui doivent concorder ; adresses jugées par `assert`, jamais pliées.
  1. **Neufs** : `pico8_mx_new(NULL)` et un `mx` avec `id`. `ht` = REF au pointeur de `pico8_mx_default("ht")`, identique d'un `mx` à l'autre, 4 entrées pliées.
  2. **`mx:kf`** (H11), forme `test_synthesis` : ligne `qe` = 7 → `ht` inchangé ; `qe` = 20 → `ht` table vide, autre pointeur que la spec, spec toujours à 4, `kw.qe` = 20 ; `qe` = 21 → même table vide, `kw.qe` toujours 20. Tailles et `kw.qe` pliés.
  3. **Écrivain par nom** (remplace `written[]`) : `set_field(ht, REF NULL)` rend 1 puis lecture 0, témoin intact ; `set_field(ht, table neuve)` puis lecture au même pointeur ; `mx:kf` sur un `ht` NULL ne fait que `jo:kf` (pas d'avortement).
  4. **Intrus** : `""`, `"h"`, `"ht "`, `"HT"`, `"ht_"`, `"hz"` rendent 0 sur `mx` ; `ht` absent de `hj`, `pm`, `nr`, `fr` neufs.
  5. Tout est plié dans `MX_MEMBERS_HASH`, relevé au premier passage vert sur l'arbre de départ, figé ensuite.
- **Test d'arrêt `test_mx_writer_stops_off_row`** (plus haut), rouge sur l'arbre de départ, vert à la fin.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_ui && ./tmp/test_pico8_ui`, notées dans le commit :
  1. **(§7-1) indicateur forcé vrai** : sans objet dans sa lettre (pas de `has_`). **Forme voisine** : le lecteur ne teste plus NULL (rend `REF NULL` présent) → rouge : caractérisation 3.
  2. **(§7-2) « présent 0 »** : sans objet (aucun fixe typé). **Forme voisine** : `family_trade_take_table` ne pose pas NULL sur nil (garde l'ancienne table) → rouge : caractérisation 3.
  3. **(§7-3) init qui saute un membre** : plus de boucle ; une omission laisserait de la mémoire `malloc` non déterministe. **Trou justifié de la lettre**, forme « pose une autre valeur » (1b.14a, 1b.24–1b.27) : (a) `self->ht = NULL` en R3 → rouge : caractérisation 1 ; (b) `g_mx_ht` déballé depuis `"k_"` → rouge : caractérisation 1.
  4. **R6** : (a) `kf` n'écrit plus la table neuve → rouge : caractérisation 2 ; (b) `kf` lit `g_mx_ht` au lieu de `self->ht` → rouge : caractérisation 2 (`qe` = 21 accepté).
  5. **Écrivain** : (a) `assert` retiré → rouge : test d'arrêt ; (b) test de tag retiré (liste acceptée) → rouge : test d'arrêt (cas liste, vecteur, `jo`).
  6. Remise à NULL de `g_mx_ht` retirée de `defaults_reset` : **mutant équivalent, noté** (l'init redéballe avant toute lecture).
- **`gcov`** : `test_pico8_ui` en `--coverage -O0`, piège `__gcov_dump` des fils, sur `planet_family_trade.c`. Lignes attendues exécutées : déballage et remise à NULL, pose R3, les deux branches R4, les deux lignes R6, les chemins nil et table de `family_trade_take_table`. Trous attendus : échecs d'`assert` (fils de `must_abort`, non vidés) et de lecture de spec (spec littérale). Rapport dans le commit.
- **Prior art** : `test_hj_members_by_name`/`test_hj_writer_stops_off_row` (1b.26), `test_pm_members_by_name` (1b.27) ; `test_synthesis`, `join`, `give_row` de `test_pico8_mx`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque.

- `./tmp/test_pico8_ui` : 10 à 30 s (H4) ; ~13 lancements (départ caractérisation, départ arrêt rouge, final, mutations 1, 2, 3a, 3b, 4a, 4b, 5a, 5b, `gcov`) ≈ 5 min. Si > 30 s : mutations 3a, 4a, 5a d'abord, le reste noté non lancé.
- `make test` : une fois (≈ 1 min).
- `make e2e` (4–6 min) une fois, **dans la même invocation** que `make class-coverage-net` (arbres distincts, 1b.0).
- `make sanitize-net` (≈ 4 min), jamais en même temps que `e2e`, une fois sur l'arbre final.
- `make value-members` : secondes.
- TU lents : compiler tôt, sans lancer (`make -j tmp/test_pico8_mx tmp/test_pico8_hj tmp/test_pico8_pm tmp/test_pico8_nr`) ; seul `make e2e` les lance.

| # | Commande exacte | État de l'arbre (départ / final / mutation n) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` vert, les **8** TU de `UNIT_TESTS_SLOW` verts (`ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 109`**, aucune ligne `pico8_mx_t` ;
5. `timeout 15m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` — requis, car le constructeur `pico8_mx_struct_new` change (`pico8_class_coverage_record` et `KNOWN[]` intacts).

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

- `FAILED:` si 1b.27 n'est pas commitée au départ, ou si `make value-members` ne rend pas 110 au départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, ou si le diff touche `FRAMES_EVENTS`, `RECORDINGS` ou `tests/fixtures/*events*.json`.
- `FAILED:` si `ht` n'est pas un `pico8_ob_table_t *` à sa place, si un `has_ht` est ajouté, ou si un `mx` neuf n'a pas le **pointeur de la spec**.
- `FAILED:` si `g_mx_defaults` ou un `offsetof(pico8_mx_t` existe encore, si `g_mx_ht` n'est pas remis à NULL dans `pico8_mx_defaults_reset`, ou si `g_nr_defaults`/`g_fr_defaults` changent.
- `FAILED:` si l'écrivain n'accepte plus exactement `ht` plus les 21 noms de `hj`, si `family_trade_take_table` n'a pas la forme 1b.13/1b.15 (nil d'abord, genre **et** pointeur testés, message nommant `mx.ht` et `ref:table`), ou si elle est partagée ou publique.
- `FAILED:` si le test d'arrêt n'a pas de ligne « Runs » rouge sur l'arbre de départ, ou si la caractérisation n'a pas de ligne verte sur l'arbre de départ avant tout typage, lit ou écrit `ht` directement, ou si `MX_MEMBERS_HASH` change après.
- `FAILED:` si un autre fichier de `src/` que `planet_family_trade.[ch]`, `tools/`, le `Makefile`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` change ; si `test_pico8_mx.c` change autrement que par la suppression de `written[]` et de sa boucle ; si un autre TU lent ou un test de régression (§6) a un diff ; si une valeur figée existante change (H5).
- `FAILED:` si le commit ne porte pas le rapport `gcov` et les mutations 1 à 6, chacune avec son test ou la justification de son trou.
- `FAILED:` si une preuve de « Preuves » ne rend pas l'attendu.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)`, si `make value-members` ne rend pas 109 (ou si un autre typedef change de compte), ou s'il manque la ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` si le travail n'est pas en un commit vert, si le budget est dépassé sans note de ce qui n'a pas été lancé, ou s'il manque le tableau « Runs » ou la note « Découvert à 1b.28 ».

## Out of Scope

- Les membres de `hj`, `qh`, `jo`, `ke` portés par `mx` (déjà typés) ; `nr`, `fr` et leurs tables ; `ga` (1b.29).
- Libérer la table de spec ou la table neuve de `kf` (TROU-7).
- Réunir `family_trade_take_table` avec une aide d'un autre fichier.
- Régénérer `tools/member_census.txt` ; déplacer un TU entre `UNIT_TESTS` et `UNIT_TESTS_SLOW`.
- Fichiers d'événements, `FRAMES_EVENTS`, rendu, audio, appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur (G)** : modules `planet_family_trade.h` (typedef) et `planet_family_trade.c` (lecteur, écrivain + `family_trade_take_table`, R3, R6, `g_mx_ht`, message de `family_trade_default_table`, suppression de `g_mx_defaults`) ; type `pico8_ob_table_t *` sans indicateur ; tests : `test_pico8_ui` (caractérisation + test d'arrêt), `test_pico8_mx` (suppression de `written[]` seule) ; pas de découpage ; `class-coverage-net` une fois.
- **Note « Découvert à 1b.28 »** (à écrire sur la ligne) :

  > patron 1b.1 appliqué ; `ht` → `pico8_ob_table_t *` (pointeur de spec, table neuve de `kf`), aucun indicateur (nil lu par TU lent, pointeur) ; forme écrivain 1b.13/1b.15 sur `ref:table` (`family_trade_take_table`, tag `OB_TABLE`), test d'arrêt ; R7 de 1b.14a sans fille, **`g_mx_defaults` supprimée** ; `written[]` de `test_pico8_mx` vidé, supprimé avec sa boucle ; §6 : aucune correspondance ; caractérisation dans `test_pico8_ui` ; empreinte réelle mesurée au commit.
- Le message de commit porte : périmètre et compte 110 → 109, résumé « Runs », `gcov`, mutations et trous, `MX_MEMBERS_HASH`, réponses à H1–H13.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
