---
title: "Plan 3, étape 1b.25 : `nl`, le comptoir. `rr` en `const char *`, `nl` et `qw` en `pico8_fixed_t`, sans indicateur ; `g_nl_members` et `g_nl_defaults` supprimées"
description: "`pico8_nl_t` garde 3 membres `pico8_value_t` (`rr`, `nl`, `qw`), tous des défauts de spec, dont `qw` est aussi réécrit par `nl:ly` et accepté par nom. 1b.25 les type selon le patron R1–R9 de 1b.1, sans genre mêlé ni indicateur. Les deux tables de `nl` se vident et sont supprimées. Les défauts sont déballés en `static` (R7 de 1b.14a). `make value-members` passe de 128 à 125. Un commit, 3 fichiers plus le plan."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.25 — `nl` (`planet_family.h:53`). Empreinte 5 (2, 3). Compte : 125.

Cadre : puces **Nil**, **REF**, **Coexistence**, **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Modèle : le PRD de 1b.24 (`plan3-phase1b-etape24-qh-laboratoire.md`), `qh` étant le parent de `nl`. Les notes « Découvert à » sont **citées, pas redérivées** :

- **1b.1, puis 1b.3 à 1b.23d** : patron R1–R9 appliqué. En particulier :
  - **1b.4b** : fils forkés, `PICO8_MEMBER_CENSUS_STOP()` sur les boucles sentinelles ;
  - **1b.9/1b.10** : R7 forme `offsetof` ; une table vidée part avec ses boucles ;
  - **1b.14b** : cas à union nulle par `memset` puis `kind` ; une mutation qui retire l'unique appel d'une aide `static` se plante sous `if (0)` ;
  - **1b.19** : un fixe sans indicateur écrit par nom a une aide sans état nil, et **nil y avorte** (`qb_take_fixed`) ;
  - **1b.21** : R2, pas d'indicateur sans lecture nil par nom ;
  - **1b.23a1** : aides de lecture de spec par genre et par fichier (`family_default_*`), message aux noms de leurs membres ; valeurs figées = valeurs observées ;
  - **1b.23a2** : un membre `l` seul que l'écrivain de 1a accepte **garde sa branche** ; boucles sentinelles : un membre typé sort des `written[]` ;
  - **1b.23b/1b.23c** : aides d'écriture par genre et par fichier ; `id` en clé par `family_take_string` ;
  - **1b.23d** : un tableau `written[]` vidé est supprimé avec sa boucle ;
  - **1b.24** (supposée commitée, H1) : tables de `qh` et `os` supprimées ; `family_default_string` ajoutée ; **le crochet `STOP` d'une boucle supprimée est gardé seul**.
- **1b.14a — R7 sur tables de filles, patron des défauts `static`** (celui de `jo` et `qh`) : l'entrée sort de toutes les tables qui la listent ; chaque défaut est déballé une fois à l'init dans un `static g_<X>_<m>`, au même pointeur pour une REF/chaîne, remis à nil dans `defaults_reset` ; la pose typée se fait à l'endroit de la boucle, avant les clés et tout `pq`. **`nl` n'a pas de fille** : seule sa propre table est en jeu.
- **1b.13 et 1b.15 — forme écrivain de R5** : une branche `strcmp` par membre à `e` dans l'ordre du typedef ; une aide `static` par genre, jamais partagée entre fichiers ; un non-pointeur sans indicateur n'a pas d'état nil et nil y avorte ; message « champ sans règle » qui nomme `struct.membre`, le genre de sa ligne et l'arrêt ; un nom sans branche rend 0 ; cas d'arrêt `memset` ; un `must_abort` nouveau, rouge sur l'ancien code.
- **1b.11 — R2 des TU lents** : une lecture par nom faite seulement par un TU de `UNIT_TESTS_SLOW` compte `l`, son nil compte `lu`. Revue plus bas.

### État de départ

HEAD `954465a` (PRD de 1b.24). **1b.24 est en cours d'implémentation ; précondition : elle est commitée avant 1b.25 (H1).** Après elle, `make value-members` rend 128, et les deux tables de `nl` ne listent plus que ses 3 membres propres. Numéros de ligne de l'arbre pré-1b.24 : ils glisseront, chaque site est aussi désigné par son contenu.

**Rien n'a été compilé ni lancé pour ce PRD.** Faits d'exécution : H1 à H12.

### Lignes de recensement (`tools/member_census.txt:98-100`)

| membre | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|
| `rr` | `chaine` | observé | stat? | l | `const char *` (pointeur de spec) | non (pointeur) |
| `nl` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `qw` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |

128 − 3 = **125**. Aucune ligne `pico8_nl_t` ne reste.

- **R2 pour `rr`** (lecture de 1b.7/1b.24 sur `od`/`s`) : son seul `stat?` est un homonyme — `oj(self.ke.rr)` (`ie:hb`, `lua.lua:926`) et `oj(b.rr)` (`:1392`, les `hp`). Sur un `nl`, `rr` vaut toujours `"trade league"` : jamais écrit (l'écrivain le refuse, `must_abort(write_rr)`), et la vue `ie_ke` lit la **spec** (`kind->field` = `pico8_nl_default`), pas l'instance. La revue des TU lents ne trouve aucune lecture nil par nom sur un `nl`. Donc pas d'indicateur.
- **Aucun genre mêlé.**

### Ce que fait le cart (`pico8/lua.lua`)

- Spec de `nl` : `rr="trade league"`, `nl=1`, `qw=15` (plus des défauts de `qh`/`jo`, hors étape).
- `nl:ly` (`:1064`) : `self.qw=8+t%42/6`, seule écriture. `jo:kj` (`:546`) lit `dv(self.qw)`. `bd` (`:1302`) lit `if pt.nl then`.

### Le code (arbre 1b.24 supposé commité)

- **Typedef** (`planet_family.h`) : `{ pico8_qh_t qh; pico8_value_t rr; nl; qw; }`.
- **Tables** (`planet_family.c`) : `g_nl_members` = {`rr`, `nl`, `qw`} (lecteur, boucle nil du constructeur) ; `g_nl_defaults` = {`rr`, `nl`, `qw`} après 1b.24 (boucle de recopie du constructeur, boucle de lecture de spec et son `assert` dans `pico8_planet_family_init`, boucle de `pico8_nl_defaults_reset`). `g_nl_nu` reste.
- **Écrivain** `pico8_nl_set_field` : `qw` (`->qw = value`, tout genre), puis les 20 noms de `qh`. Il refuse `rr`, `nl`.
- **Écriture directe (R6)** : `pico8_nl_ly`, `self->qw = pico8_value_fixed(…)`. Aucune autre.
- **Sites par nom, inchangés** (Coexistence) : `planet_display.c` (`field_fixed(self, "qw")`, `jo:kj`) ; `economy_tiers.c` (`field_truthy(pt, "nl")`, `bd`).
- **Écritures par nom dans les TU** : `test_pico8_qh` `fold_descendant` (lent) relit puis **réécrit `qw` par nom** (la valeur relue, un fixe, sous `had_qw`) ; `test_pico8_nl` boucle sentinelle `written[]` = {`qw`} après 1b.24.
- **Homonymes hors étape** : `hj.rr` (1b.26), `nr.rr` (1b.30), `hp.rr` (1b.9, fait), `ie_ke.rr` (vue) ; `qw` de `layout.c` (variable locale).

### Revue des TU lents (R2 de 1b.11)

`test_pico8_qh` : `fold_descendant` lit `qw` par nom (présent, `FIXED`) et le réécrit. `test_pico8_os/hj/pm/mx/nr` : `not_<X>[]` ∋ `rr`/`nl`/`qw` et `write_fixed(e, "rr"|"nl"|"qw")` visent **leur** classe, pas un `nl`. `test_pico8_jo`, `test_pico8_ie` : aucun des 3 noms sur un `nl` (`ie` lit la vue de spec). **R2 ne donne aucun indicateur.** Aucun TU lent n'est modifié.

## Solution

1. **Caractérisation d'abord**, verte sur l'arbre 1b.24 commité, lisant par nom les 3 membres sur des `nl` neufs et vivants, l'aller-retour par nom de `qw`, et `nl:ly`.
2. **Test d'arrêt, rouge sur l'ancien code** : écriture hors genre de `qw` (nil, non-fixes, unions nulles).
3. **Commit 1b.25** : 3 membres typés ; `g_nl_members` et `g_nl_defaults` supprimées avec leurs boucles ; 3 `static` déballés ; aide `family_take_fixed` ; `nl:ly` réécrit ; `qw` sort de `written[]` de `test_pico8_nl`, tableau vidé supprimé avec sa boucle, crochet `STOP` gardé seul. `make value-members` rend **125**.
4. Aucun pixel ne bouge. La note « Découvert à 1b.25 » va sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `planet_family.h`, je veux `pico8_nl_t` sans aucun `pico8_value_t`.
2. En tant que lecteur, je veux `rr` en `const char *`, `nl` et `qw` en `pico8_fixed_t`.
3. En tant que lecteur par nom (`jo:kj`, `bd`, les TU), je veux que `rr`, `nl`, `qw` rendent exactement ce que rendait l'ancien code : présence, genre, valeur, pointeur de chaîne.
4. En tant que `nl` neuf, je veux `rr` au **même pointeur** que la chaîne de ma spec, `nl` = 1, `qw` = 15.
5. En tant que `nl:ly`, je veux poser `qw` = 8 + t%42/6 quand `mi ≥ 2`, et rien sinon.
6. En tant qu'écrivain par nom de `qw` (`test_pico8_qh`), je veux écrire un fixe et le relire tel quel.
7. En tant que gardien de `qw`, je veux que nil et tout non-fixe avortent (forme 1b.19), message nommant `nl.qw`.
8. En tant que mainteneur, je veux que l'écrivain de `nl` accepte toujours exactement 21 noms (`qw` + les 20 de `qh`) et refuse `rr`, `nl`.
9. En tant que mainteneur, je veux `g_nl_members` et `g_nl_defaults` supprimées, et plus aucun `offsetof(pico8_nl_t` dans `src/`.
10. En tant que gardien du §3, je veux `make test` vert, 15 « conforme », aucun `*.sha256` touché.
11. En tant que `plan-pilot`, je veux `make value-members` à 125, `pico8_nl_t` absent de sa sortie.
12. En tant que vérificateur, je veux la caractérisation verte sur l'ancien code, l'arrêt rouge puis vert, les mutations et leurs trous, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Les validations que demande `to-prd` (modules, tests) sont tranchées comme le ferait un agent `guideline` : hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables. Marquées (G).

### R1–R9 appliquées

| règle | sur `nl` |
|---|---|
| R1 | Ordre gardé : `const char *rr; pico8_fixed_t nl; pico8_fixed_t qw;`. `rr` emprunté à la spec, jamais libéré. |
| R2 | Aucun indicateur (plus haut). |
| R3 | `pico8_nl_struct_new`, après `pico8_qh_struct_init` et à la place des deux boucles : `rr = g_nl_rr; nl = g_nl_nl; qw = g_nl_qw;`, avant `nu`, les clés et `pq`. Le `size_t i` devenu inutile est retiré (`-Werror`). |
| R4 | `pico8_nl_field` : 3 branches `strcmp` dans l'ordre du typedef (`rr` : `return 0` si NULL, sinon `STRING` au même pointeur ; `nl`, `qw` : toujours `FIXED`), puis `pico8_qh_struct_class()->field`. |
| R5 | Branche `qw` gardée (1b.23a2, point 1) : `family_take_fixed(value, &nl->qw)`, forme `qb_take_fixed`/`jo_take_ma` : pas d'état nil, `assert(value.kind == PICO8_VALUE_FIXED && "nl.qw: a write that is not a fixed is off its census row (fixe, never nil) -- a field under no rule of plan-3 section 5: stop, do not widen the member")`, puis `*out = value.as.fixed`. `static` dans `planet_family.c`, distincte de `family_trade_take_fixed` (1b.13 point 2). |
| R6 | `pico8_nl_ly` : `self->qw = pico8_fixed_add(…)`, sans `pico8_value_fixed`. |
| R7 | `g_nl_members` et `g_nl_defaults` vidées → supprimées avec toutes leurs boucles (forme 1b.9/1b.24). |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_nl_t)` gardé (précédent `jo`, `qh`). `tools/member_census.txt` non édité. |
| R9 | 128 → 125. |

### Défauts déballés (R7 de 1b.14a) (G)

- `static const char *g_nl_rr`, `static pico8_fixed_t g_nl_nl`, `g_nl_qw`, déballés dans `pico8_planet_family_init` juste après `g_nl_spec`, à la place de la boucle de lecture de spec :
  - `rr` par `family_default_string` (ajoutée par 1b.24), message étendu à `nl.rr` ;
  - `nl`, `qw` par `family_default_fixed`, message étendu à `nl.nl/qw`.
- Remis à NULL/0 dans `pico8_nl_defaults_reset` (qui garde `g_nl_nu = NULL`, perd sa boucle et son `size_t i`).
- Si 1b.24 a nommé autrement son aide de chaîne, on étend celle-là (G) ; si elle n'existe pas, on l'ajoute sous ce nom et on le note au commit.
- Écarté : des littéraux en R3 (les TU comparent `rr` au pointeur de la spec).

### Boucle sentinelle (1b.23a2, 1b.23d, 1b.24)

`test_pico8_nl`, `test_nl_is_a_struct` : `written[]` = {`qw`} se vide → tableau et boucle supprimés, **`PICO8_MEMBER_CENSUS_STOP()` gardé seul à sa place** (lecture de 1b.24, H9). Les variables devenues inutiles sont retirées. Les `must_abort(write_rr|write_nl|write_hp|write_id|write_qw on qh)` restent tels quels.

### Revue des tests de régression (§6), mécanique

**Liste** : union des tests du §6 et des lignes « Tests : » des étapes 1a, soit les 17 fichiers `tests/test_pico8_<nom>.c` (même liste que 1b.24) : `built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`.

**Greps** sur ces 17 fichiers :

- accès direct : `grep -nE '(->|\.)(rr|nl|qw)\b|offsetof\([^)]*\b(rr|nl|qw)\)'` → **aucun** ;
- par nom : `grep -nE '"(rr|nl|qw)"'` → **aucun** ;
- chemin : `grep -nE 'pico8_nl_|"nl"|trading hub|trading_hub|nl_ly|"ly"'` → 3 fichiers, tous homonymes :

| # | test | ligne | nature | touché ? | plan de preuve rouge |
|---|---|---|---|---|---|
| R-a | `built_planet_panel_nil_row` | `:204` `"ly"` | état `me` d'une planète, pas `nl:ly` | non | Aucun obligatoire. Dans le budget seulement : bug de référence de 1a.34 remis, rouge, restauré, vert ; sinon « non lancé, homonyme hors chemin ». |
| R-b | `empty_planet_cleared` | `:42` `QH_PLACED_STATE "ly"` | idem | non | Idem, avec le bug 529f355 de 1b.24 (R-a de 1b.24). |
| R-c | `playability` | `:20-25` commentaires `"ly"` | idem | non | Idem, bug de référence de 1a.17. |

**Aucun accès direct ni par nom** : aucun test de régression n'est modifié, aucune preuve rouge n'est obligatoire. Qu'un de leurs rejeux dessine un `nl` (lecture `qw` par `jo:kj`) n'est pas connu sans lancer (H6) ; s'il le fait, `make test` vert suffit, le diff du test étant vide.

### Preuves (sortie attendue sur l'arbre final)

- `sed -n '/pico8_qh_t qh;/,/} pico8_nl_t;/p' src/pico8_planet_family.h | grep -c 'pico8_value_t'` : **0** ;
- `make value-members | grep -c 'pico8_nl_t'` : **0** ;
- `grep -rnw 'g_nl_members\|g_nl_defaults' src/` et `grep -rn 'offsetof(pico8_nl_t' src/` : **vides** ;
- `grep -n 'qw = ' src/pico8_planet_family.c` : la pose R3 et `nl:ly` seulement (plus l'aide) ;
- `grep -c 'off its census row' src/pico8_planet_family.c` : +1 par rapport au départ ;
- `git diff --stat HEAD -- src/ ':!src/pico8_planet_family.h' ':!src/pico8_planet_family.c'` : vide ; `git diff HEAD -- tools/ Makefile` : vide.

### Découpage (G)

Aucun : 3 fichiers (`pico8_planet_family.h`, `pico8_planet_family.c`, `tests/test_pico8_nl.c`) plus `docs/plan-3.md`.

### Hypothèses (rien n'a été lancé)

- **H1** : 1b.24 est commitée ; `make value-members` rend 128 au départ ; `g_nl_defaults` = {`rr`, `nl`, `qw`} ; `family_default_string` existe.
- **H2** : dans les 15 rejeux et tous les TU hors fils de `must_abort`, `qw` ne reçoit que des fixes (`nl:ly`, `fold_descendant` sous `had_qw`).
- **H3** : sur un `nl` neuf, `rr` est `"trade league"` au pointeur de la spec, `nl` = 1, `qw` = 15 (valeurs **observées** figées).
- **H4** : chaque cas du test d'arrêt se termine sans `SIGABRT` sur l'ancien code (l'écrivain recopie tout). Un cas qui avorte déjà est retiré du « rouge d'abord » et noté.
- **H5** : `BUILT_HASH`, `LIFE_HASH`, `HUB_HASH` de `test_pico8_nl`, les hash de `test_pico8_qh` (`fold_descendant`), `STRUCT_HASH`/`SPEC_HASH` de `jo_spec_defaults` et les 15 empreintes du filet ne bougent pas.
- **H6** : on ne sait pas lequel des 17 rejeux de régression construit un `nl` ; sans effet sur le diff.
- **H7** : `make sanitize-net` reste propre (aucune allocation nouvelle).
- **H8** : `test_pico8_nl` reste sous 30 s avec deux fonctions de plus (sinon §7 : `UNIT_TESTS_SLOW`, ce qui est un changement de `Makefile` interdit ici → arrêt et note).
- **H9** : le crochet `STOP` gardé seul compile et laisse la table régénérée identique (non lancé).
- **H10** : rien ne dépend de `sizeof(pico8_nl_t)`.
- **H11** : le build `-DPICO8_MEMBER_CENSUS` compile avec un `pico8_nl_t` sans `pico8_value_t` (non lancé).
- **H12** : `family_take_fixed`, appelée une fois, ne déclenche aucun avertissement (`-Werror`).

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Les nouveaux tests n'accèdent à aucun des 3 membres directement : ils compilent sur les deux formes.
- **(G) Où : `test_pico8_nl`.** Il est rapide (hors `UNIT_TESTS_SLOW`), dans `CENSUS_UNIT_TESTS`, a `start_game`, `play_frame`, `in_fresh_child`, `must_abort`, `fold_field`, `class_value`, et dépend déjà des rejeux `empty_planet_cleared` et `trading_hub_session`. 1b.24 allait à `test_pico8_ui` parce que `test_pico8_qh` est lent ; ce n'est pas le cas ici.
- **Caractérisation, figée sur l'ancien code** : `test_nl_members_by_name`, par `in_fresh_child`, fonction distincte, hash propre **`NL_MEMBERS_HASH`**. Lecture par le lecteur brut du descripteur (`cls->field`, témoin `0x1234`) **et** par `pico8_entity_get_field`, qui doivent concorder.
  1. **Neufs** : `pico8_nl_new(NULL)` et un `nl` bâti par `ie:ldown` (`build_plate`) : plier `rr`, `nl`, `qw` ; `assert` `rr` au même pointeur que `class_value(pico8_nl_default, "rr").as.string`.
  2. **Vivants** : rejeu `trading_hub_session` (lecture seule), plier les 3 noms sur chaque `nl` de `hv` à chaque frame.
  3. **`nl:ly`** : `mi` = 1 puis 2 puis 3 par nom, `call(e, "ly", t)` pour 3 valeurs de `t` → `qw` plié (inchangé à `mi` < 2).
  4. **Écrivain par nom de `qw`** : fixe 9 → `FIXED` 9 ; **fixe 0 à union nulle** → `FIXED` 0 présent ; `cls->set_field` rend 1 pour `qw`, 0 pour `rr`, `nl` (brut, rien touché).
  5. **Intrus** : `""`, `"q"`, `"qw "`, `"QW"`, `"rr_"`, `"nl_"` → 0 au lecteur de `nl`, témoin intact.
- **Test d'arrêt, rouge d'abord** : `test_nl_writer_stops_off_row`, par `in_fresh_child`. Ajoute `must_abort_value` (forme `test_pico8_bg`/`g_`) si absente. Sur un `nl` neuf, `qw` ← **nil (`REF NULL`)**, `true`, `false`, **`false` à union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table`. Exclu : le fixe 0 à union nulle (dans le genre). Rouge sur l'ancien code (H4), vert après : une ligne « Runs » pour chacun.
- **Mutations (§7)**, dans une copie jetable du scratchpad, `make tmp/test_pico8_nl && ./tmp/test_pico8_nl` :
  1. **(§7-1) indicateur forcé vrai** : **sans objet dans sa lettre** (aucun `has_`). Forme voisine : la branche `rr` du lecteur ne teste plus NULL. **Trou justifié** : `rr` n'est jamais NULL sur un `nl` (pointeur de spec, jamais écrit) — mutant équivalent sur tout état atteignable.
  2. **(§7-2) « présent 0 » là où c'était absent** : **sans objet dans sa lettre** (`nl`, `qw` jamais absents). Forme voisine : la branche `nl` rend `FIXED` 0 → rouge : caractérisation 1.
  3. **(§7-3) init qui saute un membre** : aucune boucle ne reste ; l'omission laisserait de la mémoire `malloc` non déterministe. Forme « pose une autre valeur » (précédents 1b.14a, 1b.23d, 1b.24), **trou justifié de la lettre** :
     - (a) `qw = g_nl_nl` → rouge : caractérisation 1, `LIFE_HASH` (`qw == 15`) ;
     - (b) `rr` = une autre chaîne `"trade league"` (littéral distinct) → rouge : caractérisation 1 (`assert` de pointeur).
  4. *`nl:ly` muet* (R6 retirée) → rouge : caractérisation 3, `test_life` (`qw != 15`).
  5. *Écrivain de `qw` muet* (`if (0) family_take_fixed(…)`, `return 1`) → rouge : caractérisation 4 ; `test_pico8_qh` (lent) noté sans relance.
  6. *`qw` accepte nil* (pose 0 sur `REF NULL`) → rouge : test d'arrêt (cas nil).
  7. *`qw` accepte un booléen* (`kind == FIXED` élargi à `BOOL`) → rouge : test d'arrêt.
- **`gcov`** : `test_pico8_nl` en `--coverage -O0` (piège `__gcov_dump` des fils de 1a.23) sur `planet_family.c` : déballages et remises à nil, pose R3, 3 branches R4, aide R5, `nl:ly`. Trous attendus : l'échec des `assert` (fils de `must_abort`, prouvés par le test d'arrêt et les mutations 6–7) et celui des aides de lecture de spec (spec littérale). Le rapport va dans le commit.
- **Prior art** : `test_qh_members_by_name`/`test_qh_writer_stops_off_row` (1b.24), `qb_take_fixed` (1b.19), `jo_take_ma` (1b.23d), `test_life` de `test_pico8_nl`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer.

- `./tmp/test_pico8_nl` : 10 à 30 s (H8). Environ 12 lancements (départ, arrêt rouge, final, 8 mutations, `gcov`), ≈ 5 min. Si un lancement dépasse 30 s : mutations 2, 3a, 3b, 4, 6 d'abord, les autres notées non lancées ;
- `make test` : une fois ;
- `make e2e` : 4 à 6 min, une fois, **dans la même invocation** que `make class-coverage-net` (arbres distincts, 1b.0) ;
- `make sanitize-net` : environ 4 min, **jamais** en même temps que `e2e`, une fois sur l'arbre final ;
- `make value-members` : quelques secondes ;
- R-a/R-b/R-c (§6) : seulement s'il reste du budget.

| # | Commande exacte | État de l'arbre (départ / final / mutation n / §6 R-x bug remis / restauré) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` vert, et les **8** TU de `UNIT_TESTS_SLOW` passent (`ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 125`**, aucune ligne `pico8_nl_t` ;
5. `timeout 15m make class-coverage-net` : **identique à `tools/class_coverage_net_expected.txt`** (le constructeur `pico8_nl_struct_new` change ; `pico8_class_coverage_record` et `KNOWN[]` non touchés).

On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements : les rejeux sont seulement **lus**.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

- `FAILED:` si 1b.24 n'est pas commitée au départ ou si `make value-members` ne rend pas 128 au départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, ou si le diff touche `FRAMES_EVENTS`, `RECORDINGS` ou `tests/fixtures/*events*.json`.
- `FAILED:` (« champ sans règle ») si `family_take_fixed` avorte dans un rejeu ou un TU hors des fils de `must_abort` (H2), ou si un type est élargi au lieu d'avorter (`pico8_value_t` gardé, `has_` ou `_is_bool` ajouté).
- `FAILED:` si l'un des 3 membres n'a pas le type retenu à sa place, ou si `rr` n'est pas le pointeur de la spec.
- `FAILED:` si `g_nl_members`, `g_nl_defaults` ou un `offsetof(pico8_nl_t` existe encore, ou si un `static` de défaut n'est pas remis à nil dans `pico8_nl_defaults_reset`.
- `FAILED:` si l'écrivain de `nl` n'accepte plus exactement ses 21 noms, ou si une boucle de clés change.
- `FAILED:` si l'aide viole 1b.13/1b.15/1b.19 : nil accepté, hors-genre à union nulle accepté, message sans `nl.qw` et son genre, aide partagée entre fichiers ou publique.
- `FAILED:` si un site par nom (`planet_display.c`, `economy_tiers.c`) change ou devient un accès direct.
- `FAILED:` si `tools/`, le `Makefile`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` change, ou si un crochet du recensement change.
- `FAILED:` si le diff d'un test fait autre chose qu'ajouter la caractérisation, le test d'arrêt, `NL_MEMBERS_HASH`, `must_abort_value` si absente, et supprimer `written[]` et sa boucle de `test_pico8_nl` (crochet `STOP` gardé) ; ou si une valeur figée existante change (H5) ; ou si un test de régression (§6) a un diff.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, lit ou écrit un membre directement, ou si `NL_MEMBERS_HASH` change après.
- `FAILED:` si le test d'arrêt n'a pas une ligne « Runs » rouge au départ et verte à la fin, ou s'il lui manque nil, `true`, ou les cas à union nulle `memset`.
- `FAILED:` si le commit ne porte pas le rapport `gcov` et les mutations 1 à 7 (3a, 3b), chacune avec son test ou la justification de son trou.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)`, si `make value-members` ne rend pas 125 (ou qu'un autre typedef gagne ou perd une ligne), ou s'il manque la ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` si le travail n'est pas en un commit vert, si le budget est dépassé sans que le journal note ce qui n'a pas été lancé, ou s'il manque le tableau « Runs » ou la note « Découvert à 1b.25 ».

## Out of Scope

- Les membres de `qh` (1b.24) et de `jo` (1b.23) portés par `nl` ; `g_nl_nu`.
- Les homonymes : `hj.rr` (1b.26), `nr.rr` (1b.30), `ie_ke.rr`, le `qw` local de `layout.c`.
- `jo:kj`, `bd` eux-mêmes (sites par nom).
- Régénérer `tools/member_census.txt` ; toucher un TU lent.
- Les fichiers d'événements et `FRAMES_EVENTS`, tout changement de rendu, l'audio, lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) : modules `planet_family.h` (typedef) et `planet_family.c` (lecteur, écrivain, `family_take_fixed`, 3 `static`, pose R3, `nl:ly`, tables supprimées, messages étendus) ; tests dans `test_pico8_nl` (rapide) ; aucun test de régression modifié, aucune preuve §6 obligatoire ; pas de découpage ; `class-coverage-net` une fois sur l'arbre final.
- **Note « Découvert à 1b.25 »**, sur la ligne de l'étape (le reste va dans le commit) :

  > patron 1b.1 appliqué ; `rr` → `const char *` (pointeur de spec), `nl`, `qw` → `pico8_fixed_t`, aucun indicateur (R2 : `stat?` de `rr` d'homonyme, jamais nil sur `nl`) ; `qw` : branche `l` gardée (1b.23a2), `family_take_fixed`, nil avorte (1b.19) ; R7 de 1b.14a sans fille, **`g_nl_members` et `g_nl_defaults` supprimées** ; boucle sentinelle vidée, crochet `STOP` gardé seul (1b.24) ; §6 : aucun accès, aucune preuve ; caractérisation dans le TU de la classe, rapide.
- Le message de commit porte le compte 128 → 125, le tableau « Runs » résumé, le rapport `gcov`, les mutations et leurs trous, `NL_MEMBERS_HASH` et les réponses à H1–H12.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
