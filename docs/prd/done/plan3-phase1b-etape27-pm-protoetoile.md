---
title: "Plan 3, étape 1b.27 : `pm`, la protoétoile. `ng` en `pico8_vec_t *`, `fz` en `const char *`, tous deux pointeurs de spec ; `g_pm_members` et `g_pm_defaults` supprimées"
description: "Après 1b.26, `pico8_pm_t` ne garde que ses 2 membres propres en `pico8_value_t` : `ng` (le vecteur `v(1,0)` que `pm:pq` ajoute à `ra`) et `fz` (`\"pm\"`, la clé de `kv`). Ce sont deux défauts de spec, jamais nil et jamais écrits. 1b.27 les type selon le patron R1–R9 de 1b.1, sans indicateur ni écrivain. Les deux tables de `pm` se vident et sont supprimées, et les défauts sont déballés en `static` (R7 de 1b.14a). `make value-members` passe de 112 à 110. Un commit, 3 fichiers plus le plan."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.27 — `pm` (`planet_family.h:97`). Empreinte 1 (1, 0). Compte : 110.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Modèles : le PRD de 1b.25 (`docs/prd/done/plan3-phase1b-etape25-nl-comptoir.md`, petite fille de planète) et celui de 1b.26 (`docs/prd/triage/plan3-phase1b-etape26-hj-ascension.md`), `hj` étant le parent de `pm`. Les notes « Découvert à » sont **citées, pas redérivées** :

- **1b.1, puis 1b.3 à 1b.25** : patron R1–R9 appliqué. En particulier :
  - **1b.4** : `ref:autre` vers un vecteur de `v(…)` → `pico8_vec_t *` emprunté, même pointeur ;
  - **1b.9/1b.10** : R7 forme `offsetof`, et une table vidée part avec ses boucles ;
  - **1b.14b** : une mutation qui retire l'unique appel d'une aide `static` se plante sous `if (0)` ;
  - **1b.21** : R2, pas d'indicateur sans lecture nil par nom ;
  - **1b.23a1** : aides de lecture de spec par genre et par fichier (`family_default_*`), message aux noms de leurs membres ; les valeurs figées sont les valeurs **observées** ;
  - **1b.23b** : un défaut vecteur est le **pointeur de la spec** (`g_jo_gq` par `jo_default_vec`) ;
  - **1b.24/1b.25** : les tables d'un genre qui se vident sont supprimées. La preuve de typedef se fait par `awk`, pas par `sed`.
- **Patron des défauts `static` de `jo`/`qh`/`hj` (1b.14a, repris en 1b.23, 1b.24 et 1b.26)** :
  - chaque défaut est déballé une fois à l'init dans un `static g_<X>_<m>`, au même pointeur pour une REF ou une chaîne ;
  - le `static` est remis à nil dans `defaults_reset` ;
  - la pose typée se fait à l'endroit de l'ancienne boucle, avant les clés et tout `pq`.

  `pm` n'a **pas de fille** : aucune autre table ne liste `ng` ni `fz`.
- **1b.11, R2 des TU lents** : `test_pico8_pm` est lent. Revue plus bas.

### État de départ

HEAD `554cfa7` (PRD de 1b.26). **1b.26 est en cours d'implémentation. Précondition : elle est commitée avant que 1b.27 commence (H1).** Après elle :

- `make value-members` rend **112** ;
- `g_pm_defaults` ne liste que `fz` et `ng`, et `g_pm_members` que `ng` et `fz` ;
- les 11 défauts de `hj` que `pm` redéfinit sont en `static g_pm_*` ;
- `family_default_string` porte `hj.rr` ;
- le tableau `written[]` de `test_pico8_pm` est supprimé ;
- `test_pico8_ui` contient `test_hj_members_by_name`.

Les numéros de ligne cités sont ceux de l'arbre 1b.25 et glisseront : chaque site est aussi désigné par son contenu. **Rien n'a été compilé ni lancé pour ce PRD** : les faits viennent de `grep` et de la lecture du code, et tout fait d'exécution est une hypothèse (H1 à H11).

### Lignes de recensement (`tools/member_census.txt:114-115`)

| membre | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|
| `ng` | `ref:autre` | jamais | - | l | `pico8_vec_t *` (pointeur de spec, 1b.4/1b.23b) | non (pointeur) |
| `fz` | `chaine` | jamais | - | l | `const char *` (pointeur de spec) | non (pointeur) |

Le compte passe à 112 − 2 = **110**. **Aucune ligne `pico8_pm_t` ne reste.** Aucun `e` : aucun écrivain.

### Ce que fait le cart (`pico8/lua.lua`)

- `pm=hj:jr[[…]]` (`:1017-1030`) : `fz="pm"`, `ng=v(1,0)`.
- `pm:pq` (`:1031-1035`) : `ga(ob([[…]],{ra=self.ra+self.ng}))`. C'est la seule lecture de `ng` sur l'instance.
- `fz` n'est lu que **sur la spec** : `kv:jr` (`:86`, `kv[rj.fz or ""]`), dont le port est `jo_kv`/`g_kv` (`planet.c`). Il n'est jamais lu sur l'instance.
- Ni l'un ni l'autre n'est jamais écrit.

### Le code (arbre 1b.26 supposé commité)

- **Typedef** : `{ pico8_hj_t hj; pico8_value_t ng; pico8_value_t fz; }`. Le commentaire de tête dit que `ng` tient « the very tables of pm's spec ».
- **Tables** (`planet_family.c`) :
  - `g_pm_defaults` : `fz`, `ng` ;
  - `g_pm_members` : `ng`, `fz`.
- **Clients des tables** :
  - dans `pico8_pm_struct_new`, la boucle nil sur `g_pm_members` et la boucle de recopie de `g_pm_defaults` ;
  - dans `pico8_planet_family_init`, la boucle de lecture de spec et son `assert` « pm: every default … » ;
  - la boucle de `pico8_pm_defaults_reset` ;
  - le lecteur `pico8_pm_field`, qui boucle sur `g_pm_members` puis se replie sur `pico8_hj_field`.
- **Écriture directe (R6)** : aucune. **Lecture directe** : une seule, dans `pico8_pm_pq`, `pico8_pf_value_vec(self->ng)`.
- **Écrivain** : celui de `hj` (21 noms), qui refuse `ng` et `fz`. Le commentaire le dit : « No site writes `ng` or `fz` ». **Inchangé.**
- **Sites par nom hors `planet_family.c`** : aucun dans `src/`.

### Revue des TU lents (R2 de 1b.11)

`test_pico8_pm` lit :

- `has(e, "ng"/"fz")` (`pm_own[]`) ;
- `get_ref(e, "ng")` dans `pm_tables[]`, comparé à `class_ref(pico8_pm_default, "ng")`, **d'où le pointeur de spec** ;
- `fold_vec(h, e, "ng")` ;
- `fold_spawned`, qui vérifie `ra` du `ga` = `ra + ng` ;
- `must_abort(write_ng/write_fz)`.

Il ne lit jamais `ng` ni `fz` nil sur un `pm`. Les TU `test_pico8_hj` (`not_hj ∋ ng`), `test_pico8_mx` (`not_mx ∋ ng, fz`, `write_fixed(mx, "ng"/"fz")`) et `test_pico8_ga` (`fz` de `ga`, homonyme de 1b.29) lisent d'autres structs. **R2 ne donne aucun indicateur. Aucun TU lent n'est modifié.** Aucun tableau `written[]` ne contient `ng` ni `fz` : il n'y a pas de boucle sentinelle.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (arbre 1b.26 commité), dans un TU rapide. Elle lit `ng` et `fz` par nom sur des `pm` neufs, vérifie où `pm:pq` place le `ga`, et doit être verte avant tout typage.
2. **Pas de test d'arrêt** : il n'y a pas d'écrivain (décision plus bas).
3. **Commit 1b.27** :
   - `ng` et `fz` sont typés ;
   - `g_pm_members` et `g_pm_defaults` sont supprimées avec leurs boucles ;
   - `g_pm_ng` et `g_pm_fz` sont déballés en `static` ;
   - `family_default_vec` est ajoutée ;
   - `make value-members` rend **110**.
4. Aucun pixel ne bouge. La note « Découvert à 1b.27 » va sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `planet_family.h`, je veux `pico8_pm_t` sans aucun `pico8_value_t`.
2. En tant que lecteur par nom, je veux que `ng` rende une REF vers le vecteur (1, 0) de la spec et que `fz` rende `STRING "pm"` au pointeur de la spec, comme avant, sur tout `pm`.
3. En tant que `pm:pq`, je veux placer le `ga` à `ra + ng`, comme avant.
4. En tant que mainteneur, je veux `g_pm_members`/`g_pm_defaults` supprimées et plus aucun `offsetof(pico8_pm_t` dans `src/`.
5. En tant que mainteneur, je veux que l'écrivain de `pm` accepte toujours exactement les 21 noms de `hj` et refuse toujours `ng` et `fz`.
6. En tant qu'auteur de 1b.28 (`mx`) et de 1b.29 (`ga.fz`), je veux `planet_family_trade.c` inchangé, et une aide `family_default_string` que `ga.fz` pourra étendre.
7. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
8. En tant que `plan-pilot`, je veux `make value-members` à 110 et `pico8_pm_t` absent de sa sortie.
9. En tant que vérificateur, je veux la caractérisation verte sur l'ancien code, les mutations avec leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules et les tests. Ces choix sont tranchés comme le ferait un agent `guideline` : hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables. Ils sont marqués (G).

### R1–R9 appliquées

| règle | sur `pm` |
|---|---|
| R1 | Ordre et noms gardés : `pico8_hj_t hj; pico8_vec_t *ng; const char *fz;`. Les deux sont empruntés à la spec et jamais libérés. Le commentaire du typedef est corrigé : `ng` est le vecteur de la spec, et non une table. |
| R2 | Aucun indicateur : ce sont des pointeurs, à nil `jamais`. |
| R3 | Dans `pico8_pm_struct_new`, `self->ng = g_pm_ng; self->fz = g_pm_fz;` remplacent les deux boucles, avant les poses de 1b.26, les clés et `pico8_pm_pq`. Le `size_t i` devenu inutile est retiré (`-Werror`). |
| R4 | `pico8_pm_field` : branche `ng` (`return 0` si NULL, sinon REF au même pointeur), puis branche `fz` (`return 0` si NULL, sinon `STRING` au même pointeur), puis `pico8_hj_field`. `*out_value` reste intact sur `return 0`. |
| R5 | Vacante : aucun écrivain ne prend `ng`/`fz`, et aucune clé de construction non plus (clés mesurées : `p`, `ra`, `id`). |
| R6 | `pico8_pm_pq` : `pico8_pf_value_vec(self->ng)` devient `pico8_pf_value_vec(pico8_value_ref(self->ng))`, **forme exacte de la lecture de `ra` sur la même ligne** (G). Le repli (0, 0) sur NULL est gardé à l'identique. |
| R7 | 4 entrées retirées ; `g_pm_members` et `g_pm_defaults` sont supprimées avec leurs 4 boucles et l'`assert` de lecture. Il n'y a pas de fille. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_pm_t)` est gardé. `tools/member_census.txt` n'est pas édité. |
| R9 | 112 → 110. |

### Défauts déballés (patron `static` de `jo`/`qh`/`hj`) (G)

- `static pico8_vec_t *g_pm_ng` et `static const char *g_pm_fz` sont déballés dans `pico8_planet_family_init`, à la place de la boucle sur `g_pm_defaults`, par `pico8_pm_default`. Ils sont remis à NULL dans `pico8_pm_defaults_reset`, dont la boucle est retirée avec son `size_t i`.
- **`family_default_vec`, nouvelle**, dans `planet_family.c`, forme `jo_default_vec` (`planet.c`) : `assert(found && REF && ref != NULL && tag UNKNOWN && "pm.ng: a default of the struct is a key of pm's spec and a vector v() (ref:autre)")`, puis rend le pointeur. Elle n'est pas partagée avec `planet.c` (1b.13, point 2).
- **`family_default_string`** : le message est étendu à `pm.fz`.
- **Écarté (G)** : des littéraux en R3 (`"pm"`, un vecteur module). Le pointeur de spec est figé par `pm_tables[]` du TU lent (`ng`), et la spec est la seule source de vérité.

### Pas de test d'arrêt (G)

La consigne est : « si écrivain, test d'arrêt rouge d'abord ». **Il n'y a pas d'écrivain** :

- `pm` n'a pas de `set_field` propre ;
- celui de `hj` rend 0 pour `ng`/`fz`, et l'`assert(written && …)` de `pico8_entity_set_field` avorte déjà. Ce refus est figé par `must_abort(write_ng/write_fz)` du TU lent `test_pico8_pm` ;
- aucune clé de construction ne prend ces noms.

Un test d'arrêt nouveau serait vert sur l'ancien code. Il ne pourrait donc pas être « rouge d'abord ». La caractérisation vérifie à la place, dans le TU rapide, que l'écrivain brut **rend 0 sans rien toucher** pour `ng` et `fz`.

### Revue des tests de régression (§6), mécanique

**Liste** : les tests nommés au §6 et dans les lignes « Tests : » des étapes 1a, soit 18 fichiers :

- 17 fichiers `tests/test_pico8_<nom>.c` : `built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order` ;
- `tests/test_class_coverage.c`.

**Greps sur ces 18 fichiers** :

- accès direct : `grep -cE '(->|\.)(ng|fz)\b|offsetof\([^)]*\b(ng|fz)\)'` → **0 partout** ;
- par nom : `grep -cE '"(ng|fz)"'` → **0 partout** ;
- chemin : `grep -cE 'pico8_pm_|"pm"|protostar'` → **0 partout**.

**Aucune correspondance : aucune preuve rouge n'est due**, et les 18 diffs restent vides. Qu'un rejeu construise un `pm` n'est pas connu sans lancer (H6) ; `make test` vert suffit.

### Preuves (sortie attendue sur l'arbre final)

- `awk '/^typedef struct \{/{b=""} {b=b $0 "\n"} /\} pico8_pm_t;/{printf "%s", b}' src/pico8_planet_family.h | grep -c 'pico8_value_t'` : **0** ;
- `make value-members | grep -c 'pico8_pm_t'` : **0** ;
- `grep -rnw 'g_pm_members\|g_pm_defaults' src/` : **vide** ;
- `grep -rn 'offsetof(pico8_pm_t' src/` : **vide** ;
- `grep -n 'family_default_vec' src/pico8_planet_family.c` : la définition et un seul appel ;
- `git diff --stat HEAD -- src/ ':!src/pico8_planet_family.h' ':!src/pico8_planet_family.c'` : **vide** ;
- `git diff HEAD -- tools/ Makefile` : vide ;
- dans `tests/`, seul `test_pico8_ui.c` change, et seulement par des ajouts.

### Empreinte et découpage

- La ligne dit 1 (1, 0). **Empreinte réelle estimée : 4 (2, 2)** (H10) :
  - src : `planet_family.h` et `planet_family.c` ;
  - tests : `test_pico8_pm` (lent) et `test_pico8_jo_spec_defaults` (rapide ; `NAMES[]` plie `ng` et `fz` sur un `pm` neuf).
- **Fichiers modifiés : 3**, plus `docs/plan-3.md` : `pico8_planet_family.h`, `pico8_planet_family.c` et `test_pico8_ui.c`.
- **Découpage (G) : aucun.** L'étape se fait en un seul commit.

### Hypothèses (rien n'a été lancé)

- **H1** : 1b.26 est commitée ; `make value-members` rend 112 au départ, puis 110, et seules les 2 lignes `pico8_pm_t` disparaissent. L'état de départ est celui décrit plus haut.
- **H2** : aucun site, rejeu ou TU n'écrit `ng` ni `fz` hors des fils de `must_abort`.
- **H3** : sur tout `pm`, `ng` est la REF du vecteur (1, 0) de la spec (même pointeur que `class_ref(pico8_pm_default, "ng")`), et `fz` est `"pm"` au pointeur de la spec. Les valeurs figées sont celles **observées**.
- **H4** : `test_pico8_ui` reste sous 30 s avec une fonction de plus.
- **H5** : aucune valeur figée ne bouge :
  - les hash de `test_pico8_pm` (dont `SESSION_HASH`) ;
  - `STRUCT_HASH`/`SPEC_HASH` de `jo_spec_defaults` ;
  - ceux de `test_pico8_ui`, dont `HJ_MEMBERS_HASH` ;
  - les 15 empreintes du filet.
- **H6** : on ne sait pas quel rejeu de régression construit un `pm` ; c'est sans effet, puisque les diffs sont vides.
- **H7** : `make sanitize-net` reste propre, puisque l'étape n'ajoute aucune allocation.
- **H8** : le build `-DPICO8_MEMBER_CENSUS` compile.
- **H9** : `-Werror` reste muet : `family_default_vec` est appelée une fois et les `size_t i` retirés sont inutilisés.
- **H10** : l'empreinte réelle est 4 (2, 2). Le compte exact est fait au commit.
- **H11** : `pico8_pm_new` avec `p`, `ra`, `id` dans un `start_game(EMPTY_CLEARED)` de `test_pico8_ui` enregistre le `ga` de `pm:pq` dans `hv`, juste avant le `pm` (forme `fold_spawned` du TU lent).

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom, ou l'écran.** La caractérisation n'accède ni à `ng` ni à `fz` directement : elle compile sur les deux formes.
- **(G) Où : `test_pico8_ui`.** Il est rapide, dans `CENSUS_UNIT_TESTS`, et sert déjà pour `hj` (1b.26) et `qh` (1b.24). `test_pico8_pm` est lent (1b.11). Le précédent de 1b.25, qui mettait la caractérisation dans le TU de la classe, ne s'applique donc pas.
- **Caractérisation, figée sur l'ancien code** : `test_pm_members_by_name`, lancée par `in_fresh_child` après `test_hj_members_by_name`, avec un hash propre **`PM_MEMBERS_HASH`**. Elle lit par le lecteur brut du descripteur (`cls->field`, témoin `0x1234`) **et** par `pico8_entity_get_field`, qui doivent concorder. Les adresses sont jugées par `assert`, jamais pliées.
  1. **Neufs.** Deux `pm` : `pico8_pm_new(NULL)` et un `pm` construit avec `p`, `ra`, `id`.
     - `ng` doit être une REF au même pointeur que `class_ref(pico8_pm_default, "ng")`, identique d'un `pm` à l'autre, qui vaut (1, 0) ;
     - `fz` doit être une `STRING` au même pointeur que la valeur de la spec, et vaut `"pm"` ;
     - les contenus sont pliés.
  2. **`pm:pq`.** Dans un `start_game(EMPTY_CLEARED)`, on construit un `pm` avec `ra` (H11). Le `ga` enregistré juste avant lui dans `hv` doit avoir `ra` = `ra` du `pm` + (1, 0), et un pointeur distinct de `ra` et de `ng`. Cette position est pliée.
  3. **Écrivain brut** : `cls->set_field(pm, "ng" | "fz", fixe 9)` rend 0, et une relecture montre les deux membres intacts.
  4. **Intrus** sur le lecteur de `pm` : `""`, `"n"`, `"ng "`, `"NG"`, `"fz_"`, `"f"` rendent 0, témoin intact. `ng` et `fz` sont absents sur `hj` et `mx` neufs.
  5. Tout est plié dans **`PM_MEMBERS_HASH`**. La valeur est relevée au premier passage vert sur l'arbre de départ, puis ne change plus.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_ui && ./tmp/test_pico8_ui`. Chacune est notée dans le commit avec le test qui la rougit, ou avec la justification de son trou.
  1. **(§7-1) indicateur forcé vrai** : **sans objet dans sa lettre**, puisque l'étape n'a aucun `has_`.
     - **Forme voisine** : la branche `ng` du lecteur rend 0. Attendu rouge : caractérisation 1.
     - Le retrait du test NULL de la même branche est un **mutant équivalent, noté** : `ng` n'est jamais NULL.
  2. **(§7-2) « présent 0 » là où c'était absent** : **sans objet dans sa lettre**, puisque l'étape n'a aucun fixe typé. **Forme voisine** : la branche `fz` rend un littéral `"pm"`, même texte mais autre pointeur. Attendu rouge : caractérisation 1 (`assert` de pointeur).
  3. **(§7-3) init qui saute un membre typé** : il ne reste aucune boucle d'init, et une omission pure laisserait de la mémoire `malloc` non déterministe. **Trou justifié de la lettre**, forme « pose une autre valeur » (précédents 1b.14a, 1b.23d, 1b.24–1b.26) :
     - (a) `self->ng = NULL` en R3 → rouge : caractérisation 1 (absent) et 2 (`ga` à `ra`) ;
     - (b) `g_pm_fz` déballé depuis `"rr"` (`"ascension"`) → rouge : caractérisation 1.
  4. *R6 inversée* : `pico8_pm_pq` lit `ra` à la place de `ng` → rouge : caractérisation 2.
  5. *`family_default_vec` rend une copie neuve du vecteur* → rouge : caractérisation 1 (`assert` de pointeur). `pm_tables[]` du TU lent est noté.
  6. *Remise à nil de `g_pm_ng` retirée de `defaults_reset`* : **mutant équivalent, noté**. L'init redéballe avant toute lecture ; seule la relecture du diff le juge.
- **`gcov`** : `test_pico8_ui` en `--coverage -O0`, avec le piège `__gcov_dump` des fils, sur `planet_family.c`. Chaque ligne modifiée doit être exécutée : déballage et remise à nil, les 2 poses R3, les 2 branches R4 (la sortie NULL exceptée, inatteignable), la ligne R6 et `family_default_vec`. Trou attendu : l'échec de l'`assert` de lecture de spec, inatteignable puisque la spec est un littéral. Le rapport va dans le commit.
- **Prior art** : `test_hj_members_by_name` (1b.26) et `test_qh_members_by_name` (1b.24), dans le même TU ; `fold_spawned` et `pm_tables[]` de `test_pico8_pm` ; `jo_default_vec` (1b.23b).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer.

- `./tmp/test_pico8_ui` : 10 à 30 s (H4). Environ 10 lancements (départ, final, mutations 1, 2, 3a, 3b, 4, 5, `gcov`), soit environ 4 min. Si un lancement dépasse 30 s : mutations 3a, 3b et 4 d'abord, les autres notées non lancées.
- `make test` : une fois.
- `make e2e` : 4 à 6 min, une fois, **dans la même invocation** que `make class-coverage-net` (arbres distincts, 1b.0).
- `make sanitize-net` : environ 4 min, **jamais** en même temps que `e2e`, une fois sur l'arbre final.
- `make value-members` : quelques secondes.
- **TU lents** : aucun n'est modifié, mais `test_pico8_pm`, `hj` et `mx` incluent `planet_family.h`. Il faut les compiler tôt, en compilation seule (`make -j tmp/test_pico8_pm tmp/test_pico8_hj tmp/test_pico8_mx`). Seul `make e2e` les lance.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre (départ / final / mutation n) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` vert, et les **8** TU de `UNIT_TESTS_SLOW` passent (`ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 110`**, aucune ligne `pico8_pm_t` ;
5. `timeout 15m make class-coverage-net` : **identique à `tools/class_coverage_net_expected.txt`**. Elle s'applique parce que le constructeur `pico8_pm_struct_new` change ; `pico8_class_coverage_record` et `KNOWN[]` ne sont pas touchés.

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements : `empty_planet_cleared_events.json` est seulement **lu**.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

- `FAILED:` si 1b.26 n'est pas commitée au départ, ou si `make value-members` ne rend pas 112 au départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, ou si le diff touche `FRAMES_EVENTS`, `RECORDINGS` ou `tests/fixtures/*events*.json`.
- `FAILED:` si `ng` n'est pas un `pico8_vec_t *` ou `fz` un `const char *` à sa place, si un `has_` est ajouté, ou si l'un des deux n'est pas le **pointeur de la spec**.
- `FAILED:` si `g_pm_members`, `g_pm_defaults` ou un `offsetof(pico8_pm_t` existe encore, ou si `g_pm_ng`/`g_pm_fz` ne sont pas remis à nil dans `pico8_pm_defaults_reset`.
- `FAILED:` si `pm` gagne un écrivain, si l'écrivain partagé n'accepte plus exactement les 21 noms de `hj`, ou si une boucle de clés change.
- `FAILED:` si `family_default_vec` est partagée entre fichiers ou publique, ou si son message ne nomme pas `pm.ng` et `ref:autre`.
- `FAILED:` si la ligne R6 de `pico8_pm_pq` change autrement que par l'emballage de `self->ng`.
- `FAILED:` si `planet_family_trade.c`, un autre fichier de `src/`, `tools/`, le `Makefile`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` change, ou si un crochet du recensement change.
- `FAILED:` si le diff d'un test fait autre chose qu'ajouter à `test_pico8_ui` la caractérisation, `PM_MEMBERS_HASH` et leur appel ; si un TU lent ou un test de régression (§6) a un diff ; ou si une valeur figée existante change (H5).
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, si elle lit ou écrit `ng`/`fz` directement, ou si `PM_MEMBERS_HASH` change après.
- `FAILED:` si le commit ne porte pas le rapport `gcov` et les mutations 1 à 6 (3a, 3b), chacune avec son test ou la justification de son trou.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)`, si `make value-members` ne rend pas 110 (ou si un autre typedef gagne ou perd une ligne), ou s'il manque la ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` si le travail n'est pas en un commit vert, si le budget est dépassé sans que le journal note ce qui n'a pas été lancé, ou s'il manque le tableau « Runs » ou la note « Découvert à 1b.27 ».

## Out of Scope

- Les membres de `hj`, `qh`, `jo` et `ke` que porte `pm` (1b.23 à 1b.26).
- `mx.ht` (1b.28) et `planet_family_trade.c`.
- Les homonymes : `ga.fz` (1b.29), les lectures de `fz` sur la spec (`jo_kv`/`g_kv`, `planet.c`).
- Réunir `family_default_vec` et `jo_default_vec`, ou étendre l'aide à un autre membre.
- Un test d'arrêt pour un écrivain qui n'existe pas.
- Régénérer `tools/member_census.txt` ; déplacer un TU entre `UNIT_TESTS` et `UNIT_TESTS_SLOW`.
- Les fichiers d'événements et `FRAMES_EVENTS`, tout changement de rendu, l'audio, lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - **modules** :
    - `planet_family.h` : le typedef et son commentaire ;
    - `planet_family.c` : lecteur, poses R3, ligne R6, `g_pm_ng`/`g_pm_fz`, `family_default_vec`, message de `family_default_string`, suppression des deux tables ;
  - **types** : `ng` → `pico8_vec_t *`, `fz` → `const char *`, pointeurs de spec, sans indicateur ;
  - **tests** : `test_pico8_ui` étendu par la caractérisation seule ; aucun test d'arrêt, faute d'écrivain ; aucun TU lent ni test de régression modifié ;
  - **pas de découpage**, et `class-coverage-net` lancé une fois.
- **Note « Découvert à 1b.27 »**, à écrire sur la ligne de l'étape :

  > patron 1b.1 appliqué ; `ng` → `pico8_vec_t *`, `fz` → `const char *`, pointeurs de spec (1b.4, 1b.23b), aucun indicateur ; patron `static` de `jo`/`qh`/`hj` sans fille, **`g_pm_members` et `g_pm_defaults` supprimées**, `family_default_vec` ajoutée ; R5 vacante, pas de test d'arrêt (aucun écrivain) ; §6 : aucune correspondance, aucune preuve ; caractérisation dans `test_pico8_ui` (TU de `pm` lent) ; empreinte réelle 4 (2, 2) mesurée au commit, 3 fichiers modifiés.
- Le message de commit porte :
  - le périmètre et le compte 112 → 110 ;
  - le résumé du tableau « Runs » ;
  - le rapport `gcov`, les mutations et leurs trous ;
  - `PM_MEMBERS_HASH` et les réponses à H1–H11.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
