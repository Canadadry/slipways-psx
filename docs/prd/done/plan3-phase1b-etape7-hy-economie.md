---
title: "Plan 3, étape 1b.7 : `hy` (l'économie) — six fixes simples, `invented` en `pico8_list_t *`, `po`/`fu` en `pico8_ob_table_t *`, `happiness` avec `has_happiness`, et `hl` ({bool, chaine}) en `const char *` avec `has_hl`"
description: "`hy` a onze membres `pico8_value_t` lus par nom. Les typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 253 à 242. L'accesseur `hy_member` (forme `pico8_X_member`, 1b.5) disparaît. `hl` est à genres mêlés {bool, chaine}, le premier mélange scalaire/pointeur : cette étape en fixe la lecture de R1. Ni écrivain par nom ni clé de construction : les deux moitiés de R5 restent vacantes."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.7 — `hy` (`economy.h:17`), genres mêlés `hl`. Empreinte 23 (7, 16). Compte : 242.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié), le §6 (tests de régression) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de la note « Découvert à 1b.1 ». Il est cité, pas redécidé. S'appliquent aussi :

- « Découvert à 1b.3 » et « à 1b.4 » : patron appliqué sans écart, R5 et R7 vacantes tant qu'aucune étape n'en a ;
- « à 1b.4b » : la table corrigée (fils forkés) fait foi. Ses lignes `hy` sont celles citées ci-dessous ;
- « à 1b.5 » : **forme R7 d'un accesseur `pico8_X_member`**, avec sa preuve mécanique `grep -n 'pico8_value_t \*' … | grep -v 'pico8_value_t \*out_value)'`, et **lecture de R1 pour {fixe, bool}** (`<m>_is_bool`, un emballage public unique par membre).

### État de départ

HEAD 6a58b62. **Précondition** : 1b.6 (`pj_cr`) est commitée avant que 1b.7 commence. Elle est en cours dans l'arbre de travail : `menu.c`, `score_panel.[ch]` et `test_pico8_pj.c` y sont modifiés. `make value-members` rend alors 253. Les numéros de ligne de `menu.c` et `test_pico8_pj.c` cités ici sont ceux de HEAD : ils glisseront après 1b.6. Chaque site est désigné aussi par son contenu.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H12).

### Lignes de recensement (`tools/member_census.txt:20-30`, régénéré par 1b.4b)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `nz` | offset | `fixe` | observé | stat? | l |
| `ge` | offset | `fixe` | jamais | - | l |
| `jk` | offset | `fixe` | observé | stat? | l |
| `ew` | offset | `fixe` | jamais | - | l |
| `invented` | offset | `ref:liste` | jamais | - | l |
| `lo` | offset | `fixe` | jamais | - | l |
| `yr` | offset | `fixe` | jamais | - | l |
| `po` | offset | `ref:table` | observé | lu,stat | l |
| `fu` | offset | `ref:table` | observé | lu,stat | l |
| `happiness` | offset | `fixe` | observé | lu | l |
| `hl` | offset | **`bool,chaine`** | observé | lu | l |

Onze membres, donc 253 − 11 = **242**, comme le dit la ligne. Aucun n'a la colonne `e` : `hy` n'est écrit par nom nulle part (voir R5).

### Ce que fait le cart (`pico8/lua.lua`)

- `hy:pq()` (`:1198-1203`) pose `nz=100, ge=0, jk=0, ew=1, invented=o(), lo=1, yr=3401`.
- `hy:lj()` (`:1226-1237`) pose `po, fu = bx(), ck()`, puis `ge`, `happiness = bf()` et :

  ```lua
  self.hl = self.yr>=3426 and "final score" or self.ge<=0 and self.nz<3 and "bankrupt"
  ```

  Cette expression vaut `"final score"`, `"bankrupt"` ou **`false`**, **jamais `true`** : chaque `and` qui échoue rend le `false` de sa comparaison.
- `hl` est lu par `pj:ph` (`fm.hl and not self.gz`, `:870`) et par `pj:kj` (`fm.hl or "current score"`, `:885`). Ce sont deux tests de vérité, où `false` et nil se valent **à l'écran**.

### Le code aujourd'hui

- `src/pico8_economy.h:15-28` : `{ header; pico8_value_t nz, ge, jk, ew, invented, lo, yr, po, fu, happiness, hl; }`.
- `src/pico8_economy.c` :
  - `hy_member` (`:21-56`) : accesseur `pico8_X_member` à `strcmp` (11 branches) qui rend `pico8_value_t *`, sans `offsetof` ;
  - lecteur `pico8_hy_field` (`:58-66`) : il passe par `hy_member`, puis rend 0 sur `REF NULL` ;
  - descripteur (`:68-72`) : `set_field` NULL, aucune méthode ;
  - `member_fixed` (`:74-76`) : il rend 0 sur un genre non fixe ;
  - constructeur `pico8_hy_struct_new` (`:78-115`). Il pose d'abord les 11 membres à `REF NULL`, puis passe l'`assert` « aucune clé » (`:95-97`), puis les valeurs de `pq` (`:99-108`, `invented` alloué, `/* TROU-7 */`). Suivent `pico8_class_coverage_record("hy")` et `PICO8_MEMBER_CENSUS_SEE`. **Il n'y a pas de boucle de clés.**
  - fonctions : `nx` (`:149-172`, `lo`), `hk` (`:174-231`, `nz`/`ge`/`jk`/`po`/`yr`), `lu` (`:233-266`, `nz`), `lj` (`:268-353`, `po`/`fu` libérés puis remplacés, `ge`, `happiness`, `yr`/`nz` lus, `hl`), `kt` (`:355-390`, `jk`, `invented` par `PICO8_REF_AS_LIST`, `ew`).
- Écritures directes hors du module :
  - `src/pico8_cheat.c:36-37` : `hy->jk`, `hy->nz` (branche `hy` de `apply`) ;
  - `src/pico8_menu.c:319` : `fm->lo = pico8_value_fixed(…)` (`pico8_ix1_my_dispatch`).
- Lecteurs par nom (**inchangés**) :
  - `score_panel.c:260-266` (`ge`, `nz`, `jk`, `happiness`, `lo`, `yr`, `po`) ;
  - `score_panel_display.c:208,255` (`hl`) ;
  - `tech_gate.c:49,80,124,131` (`invented`, `fu`, `jk`, `ew`) ;
  - `menu.c:201,204` (`ew`, `invented`, dans `cr()`) ;
  - `economy_score.c:263-267,308` (`happiness`, `invented`, `po`, par ses aides `field_*`) ;
  - les tests.
- `src/pico8_probe.c` nomme `pico8_hy_t` (dans `d_`, `fm_free`, `pico8_fm_hy`) mais ne touche aucun membre. Ses `fm->jk`/`fm->nz` (`:29-32`, `:579-580`) sont ceux de **`fm_placeholder`** (1b.8), qui ne change pas.

### Nil, membre par membre

- `ge`, `ew`, `lo`, `yr` : nil `jamais`. Pas d'indicateur (R2).
- `nz`, `jk` : nil « observé », mais **par `stat?` seulement**. La moitié statique rattache le nom à plusieurs typedefs. Les preuves viennent de l'homonyme `fm_placeholder` : `g_fm_placeholder.jk/nz = REF NULL` (`probe.c:579-580`), `test_replacement` (`test_pico8_fm.c`, `!has(pico8_fm(),"jk")` sur le provisoire) et `test_cheat_on_provisional` (`test_pico8_hy`). **Preuve du PRD (exception de R2)** : sur un `hy`, toute écriture de `nz`/`jk` est un fixe. Ce sont le constructeur (avant `PICO8_MEMBER_CENSUS_SEE`), `hk`, `lu`, `kt`, `apply` de la triche, et les préparations de test (`set_credits`, `fm_set`, `cheat_code`). Aucune n'est un nil (H3). **(G) Pas de `has_nz` ni de `has_jk`.**
- `invented` : nil `jamais`. C'est un pointeur (NULL = nil).
- `po`, `fu` : nil observé (`lu` : `test_construction` lit `!has(bare,"po")` sur un `hy` neuf, avant le premier `lj`). Ce sont des pointeurs, NULL = nil.
- `happiness` : nil observé (`lu`, même lecture sur `bare`). C'est un non-pointeur lu par nom : **`has_happiness`** (R2).
- `hl` : nil observé (`lu`, même lecture). Genres mêlés : voir la décision R1 ci-dessous.

### Empreinte remesurée

Le grep de la liste rend **23 (7, 16)**, la même valeur que la ligne :

- src : `cheat.c`, `economy.c`, `menu.c`, `probe.c`, `score_panel.c`, `score_panel_display.c`, `tech_gate.c` ;
- tests : `empty_planet_cleared`, `fm`, `g_`, `game_end`, `geoharvest_burst`, `hj`, `hy`, `ie`, `jc`, `js`, `nl`, `os`, `playability`, `qh`, `slipway_drag`, `tech_screen`.

Ce grep a deux défauts :

- il **sur-compte** : `probe.c` n'y entre que par `fm_placeholder`, et 13 des 16 tests ne font que lire par nom (diff vide) ;
- il **sous-compte** les fichiers qui écrivent un membre par `pico8_fm_hy()->m` sans nommer le typedef (`fr`, `mx`, `nr`, `mz`, `score_panel_stars`, `pj`, `cheat_code`), ainsi que `economy_score.c` (aides `field_*`, qui ne contiennent pas `_field`).

**Fichiers modifiés : 16**, plus `docs/plan-3.md`.

- src (4) : `src/pico8_economy.h`, `src/pico8_economy.c`, `src/pico8_cheat.c` (2 lignes), `src/pico8_menu.c` (1 ligne).
- tests (12) : tous changent **seulement la forme d'une écriture directe** de préparation, que le compilateur désigne. Seul `test_pico8_hy` reçoit aussi la caractérisation.
  - `test_pico8_hy.c` : corps de `fm_set` (`:71-81`), plus la caractérisation ;
  - `set_credits` et assimilés (`pico8_fm_hy()->nz = pico8_value_fixed(…)`) : `test_pico8_fr.c:360`, `mx.c:445`, `os.c:404`, `nr.c:412`, `mz.c:522`, `hj.c:450`, `nl.c:392`, `qh.c:402` ;
  - `test_pico8_pj.c:414,467` (`hl = pico8_value_string("final score")`) ;
  - **régression (§6)** : `test_pico8_cheat_code.c:85-86` (`jk`, `nz`) et `test_pico8_score_panel_stars.c:114,116` (`invented`, `happiness`).

`test_pico8_os`, `qh`, `hj`, `mx` et `nr` sont dans `UNIT_TESTS_SLOW` : `make test` ne les compile pas.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (voir Testing Decisions).
2. **Typage** :

   ```c
   pico8_fixed_t nz, ge, jk, ew;      /* un membre par ligne, ordre gardé */
   pico8_list_t *invented;
   pico8_fixed_t lo, yr;
   pico8_ob_table_t *po, *fu;
   pico8_fixed_t happiness; int has_happiness;
   const char *hl; int has_hl;
   ```

   L'accesseur `hy_member` et l'aide `member_fixed` disparaissent. Un emballage public, `pico8_hy_hl`, est ajouté.
3. Effet mesurable : `make value-members` passe de 253 à **242** et ne liste plus aucune ligne `pico8_hy_t`. Aucun pixel ne bouge.

**Pas de découpage en 1b.7a/b** (G). Le diff est mécanique. Côté `src/`, il tient en deux fichiers réécrits (11 branches R4 et un constructeur) et trois lignes ailleurs. Côté tests, il se réduit à une ligne d'écriture par fichier et à une caractérisation. `hy_member` ne peut disparaître qu'une fois ses 11 branches retirées, et deux commits laisseraient un accesseur à moitié vide pour rien. Seul `hl` porte une décision nouvelle, et il ne se typerait pas plus sûrement seul. **Découpage de repli**, à ne prendre que si le premier passage dépasse le budget avant le typage de `hl` :

- 1b.7a : dix membres, sans `hl`, compte 243, `hy_member` réduit à la seule branche `hl` ;
- 1b.7b : `hl`, compte 242, `hy_member` supprimé.

L'implémenteur l'écrit alors dans la liste du plan, dans le commit de 1b.7a.

## User Stories

1. En tant que lecteur de `economy.c`, je veux les onze membres de `hy` typés, afin que `nx`, `hk`, `lu`, `lj` et `kt` ne déballent plus de `pico8_value_t`.
2. En tant que `pj:ph`, `pj:kj`, `ky`, `tech_gate`, `cr()` et `economy_score`, je veux lire chaque membre par nom avec le même genre, la même valeur et **le même pointeur** qu'en 1a.
3. En tant que `pj:ph`, je veux `hl` lu `BOOL` false (présent), et non absent, après un `lj` qui ne conclut pas la partie, comme en 1a, même si l'écran ne les distingue pas.
4. En tant que `test_construction`, je veux un `hy` neuf qui ait exactement `nz`, `ge`, `jk`, `ew`, `invented`, `lo`, `yr`, et pas `po`, `fu`, `happiness`, `hl`.
5. En tant que `test_hy_is_a_struct`, je veux que l'écriture par nom, une clé de construction et `lu` sur une autre struct avortent toujours.
6. En tant que la triche (`apply`), je veux écrire `jk = 99` et `nz = 200` sur un `hy` par affectation typée.
7. En tant que `ix[1].my`, je veux remettre `lo` à 1 par affectation typée.
8. En tant que `cheat_code` et `score_panel_stars` (tests de régression, §6), je veux ne changer que la forme de mes écritures de préparation, et rester rouges avec mon bug remis.
9. En tant que les huit tests qui fixent les crédits (`set_credits`), je veux une écriture typée de `nz`, avec la même valeur.
10. En tant que `test_pico8_pj`, je veux poser `hl = "final score"` par affectation typée, présence comprise.
11. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
12. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux, sans nouvelle libération (TROU-7 préservé pour `invented`, `po` et `fu`).
13. En tant que `plan-pilot`, je veux `make value-members` à 242 et `make class-coverage-net` identique à la référence.
14. En tant que `make float-audit`, je veux que les sites C1 `pico8_hy_hk | k_val.as.fixed` et `pico8_hy_kt | ew` gardent leur extrait mot pour mot.
15. En tant qu'auteur des étapes suivantes à {bool, pointeur} (`hp.il` 1b.9, `js.h_`/`my` 1b.34, `qh.et` 1b.24, `g_.et` 1b.32, `mz.et` 1b.36), je veux la lecture de R1 retenue pour {bool, chaine}, et savoir ce qu'elle ne couvre pas.
16. En tant qu'auteur de 1b.8 (`fm_placeholder.jk`/`nz`), je veux savoir que les preuves `stat?` de `hy.nz`/`jk` venaient de lui.
17. En tant que vérificateur, je veux une caractérisation par nom lancée sur l'ancien code, les mutations du §7 déclinées sur `hy`, et la preuve rouge des deux tests de régression touchés.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules et tests par l'utilisateur. Ces choix sont tranchés ici comme le ferait un agent `guideline`, et marqués (G).

### Décision R1 pour `hl` : {bool, chaine} (lecture nouvelle de R1)

Ce n'est pas le mélange {fixe, bool} de 1b.5 : un pointeur ne peut pas porter un booléen comme un fixe le porte. **(G) Forme retenue :**

```c
const char *hl;
int has_hl;   /* 0: `hl` is nil; 1 with `hl` NULL: `hl` is false */
```

| état 1a | `has_hl` | `hl` | lu par nom |
|---|---|---|---|
| nil (`REF NULL`) | 0 | NULL (non lu) | absent, `*out_value` intact |
| `BOOL` 0 | 1 | NULL | `BOOL` 0 |
| `STRING` s | 1 | s, même pointeur | `STRING` s |

Justification :

- **Trois états, et le plus petit codage sans sentinelle.** nil est porté par l'indicateur, comme R2 le veut. `false` est porté par le seul état qu'une chaîne présente ne prend jamais. Les chaînes écrites sont des littéraux, jamais NULL (H4). Un troisième champ `hl_is_bool`, calqué sur 1b.5, serait redondant : l'invariant `hl_is_bool ⇔ hl == NULL` le rendrait égal à `hl == NULL`.
- **Écart assumé avec R2** : « un pointeur n'a pas d'indicateur (NULL = nil) » ne tient plus, car NULL doit dire `false`. Un membre {bool, pointeur} a donc **toujours** `has_<m>`, même s'il est pointeur.
- **Condition d'application : le seul booléen écrit est `false`.** Il faut le prouver par le cart (idiome `a and b or c and d`, `:1235-1236`) et par le C (`lj` : le seul `pico8_value_bool` écrit dans `hl` est `pico8_value_bool(0)`, `economy.c:334`). Les candidats suivants peuvent tenir `true` : `qh.et`, `g_.et` et `mz.et` (bool ou REF), `js.my` (bool ou fonction). La forme pour `true` n'est **pas** fixée ici. Le premier PRD qui en a une la tranche.
- **Emballage** (lecture de 1b.5 appliquée telle quelle) : `pico8_value_t pico8_hy_hl(const pico8_hy_t *self)`, déclaré dans `economy.h`, rend `REF NULL` si `self` est NULL ou si `has_hl` vaut 0, `pico8_value_bool(0)` si `hl` est NULL, `pico8_value_string(hl)` sinon. Son seul appelant est le lecteur, car aucune lecture directe de `hl` n'existe. Il est public quand même, pour garder la lecture de 1b.5 au mot près. Son contrat va dans `economy.h`.
- La chaîne n'est jamais possédée ni libérée (littéraux statiques : `lj`, `test_pico8_pj`).

### R1–R9 appliquées à `hy` (patron de 1b.1, cité)

| règle | sur `hy` |
|---|---|
| R1 | `nz`, `ge`, `jk`, `ew`, `lo`, `yr`, `happiness` → `pico8_fixed_t`. `invented` → `pico8_list_t *` (`ref:liste`). `po`, `fu` → `pico8_ob_table_t *` (`ref:table`). `hl` : ci-dessus. Noms et ordre gardés, un membre par ligne. |
| R2 | `has_happiness` juste après `happiness` ; `has_hl` juste après `hl` (lecture ci-dessus). Pas de `has_nz`/`has_jk` : preuve du PRD, H3. Pas d'indicateur pour `ge`, `ew`, `lo`, `yr` (nil `jamais`), ni pour `invented`, `po`, `fu` (pointeurs, NULL = nil). |
| R3 | Le bloc d'ouverture (`:84-94`) garde sa place, et chaque affectation `REF NULL` devient une affectation typée : fixe à 0, pointeur à NULL, et `has_happiness = 0`, `has_hl = 0` posés à côté. L'`assert` « aucune clé » reste tel quel. Le bloc `pq` (`:99-108`) reste à sa place, typé : `nz = pico8_fixed_from_int(100)`, …, `invented = <liste allouée>` (`/* TROU-7 */` gardé). `pico8_class_coverage_record` et `PICO8_MEMBER_CENSUS_SEE` restent à leur place. |
| R4 | Le lecteur a une branche `strcmp` par membre, dans l'ordre du typedef, avant le `return 0` final. Fixes sans indicateur → toujours `pico8_value_fixed(m)`. `happiness` : `has_happiness == 0` → `return 0`, sinon `pico8_value_fixed`. `invented`, `po`, `fu` : NULL → `return 0`, sinon `pico8_value_ref(ptr)`, même pointeur. `hl` : `v = pico8_hy_hl(self)`, puis `return 0` si `v` est `REF NULL`. Tout autre nom rend 0. Dans tous les `return 0`, `*out_value` reste intact. |
| R5 | **Vacante, les deux moitiés.** (1) **Écrivain** : `set_field` reste NULL, `must_abort(write_nz)` est figé, et aucun écrivain générique n'atteint `fm` (constat de 1a.35, H2). Ce n'est **pas** l'étape qui exerce la moitié « écrivain » de R5 : sa forme reste à fixer par la première struct qui a un `set_field`. (2) **Boucle de clés** : il n'y en a pas. Le constructeur refuse toute clé (`assert` gardé, message inchangé, `must_abort(build_with_key)`). |
| R6 | Le compilateur désigne les sites. `member_fixed(self->m)` → `self->m`, et l'aide `member_fixed` est supprimée (plus d'appelant). `self->m = pico8_value_fixed(x)` → `self->m = x`. `hk` : `if (self->po != NULL && r != NULL)`, puis `pico8_ob_table_get_*(self->po, …)` sans cast. `lj` : `if (self->po != NULL) pico8_ob_table_free(self->po);` (idem `fu`), puis `self->po = new_po;`, `self->fu = new_fu;`, `self->happiness = happiness; self->has_happiness = 1;`. Ensuite `hl` : les trois branches posent `self->hl = "final score"` / `"bankrupt"` / `NULL`, puis `self->has_hl = 1`. La variable locale `hl` disparaît. `kt` : `if (self->invented != NULL)`, `pico8_list_add(self->invented, px_val)`, `count = self->invented->count`. **Les locales `ew` de `kt` et `k_val` de `hk` gardent leur nom** (H9). `cheat.c:36-37` : `hy->jk = pico8_fixed_from_int(99); hy->nz = pico8_fixed_from_int(200);`. `menu.c:319` : `fm->lo = pico8_fixed_from_int(1);`. |
| R7 | **Forme `pico8_X_member` de 1b.5, appliquée** : les 11 branches sont retirées de `hy_member`, qui, vide, est supprimé. Seul le lecteur l'appelait : il prend les branches R4 ci-dessus. Il n'y a pas de boucle de clés à brancher. Preuves, vides après : `grep -rnw 'hy_member' src/` ; `grep -n 'pico8_value_t \*' src/pico8_economy.c \| grep -v 'pico8_value_t \*out_value)'` (restent `pico8_hy_field` et `pico8_hy_default`, tous deux en `out_value`) ; `grep -nE '(->\|\.)(nz\|ge\|jk\|ew\|invented\|lo\|yr\|po\|fu\|happiness\|hl)\.(kind\|as)' src/pico8_economy.c src/pico8_economy.h src/pico8_cheat.c src/pico8_menu.c`, restreint aux fichiers qui atteignent un `hy` parce que `fm_placeholder.jk`/`nz` restent `pico8_value_t` jusqu'à 1b.8 (aujourd'hui : `economy.c:294-298`) ; `grep -rn 'offsetof(pico8_hy_t' src/` (déjà vide). La forme `offsetof` (`members[]`/`defaults[]`/`keys[]`) reste à fixer. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_hy_t)` est gardé. `tools/member_census.txt` n'est pas édité. |
| R9 | 253 → 242, aucune ligne `pico8_hy_t`. `has_happiness` et `has_hl` ne comptent pas. |

- Aucun site par nom ne change, et aucun n'est remplacé par un accès direct (`score_panel*.c`, `tech_gate.c`, `menu.c` `cr()`, `economy_score.c`, lectures des tests).
- `#include "pico8_ref_audit.h"` est retiré de `economy.c` si `PICO8_REF_AS_LIST` de `kt` en était le seul usage.
- **Commentaires** (règle de `CLAUDE.md`) :
  - le contrat de `has_happiness` s'écrit au format de `trade.h` (`/* 0: \`happiness\` is nil, and is then never read */`), celui de `has_hl` comme ci-dessus, et celui de `pico8_hy_hl` en une phrase ;
  - la seconde moitié du commentaire de tête de `economy.h` (« an absent or wrong-kind field reads as its zero/false default ») devient fausse avec `member_fixed` : elle est retirée, et la moitié « NULL `self` is a safe no-op » reste ;
  - le commentaire du typedef (TROU-7, « `hl` is absent before the first pico8_hy_lj() ») et celui de `pico8_hy_lj` (« `hl` is BOOL(0), not a string… ») restent exacts ;
  - le `/* TROU-7 */` du constructeur est gardé ;
  - aucun autre commentaire.
- `invented`, `po` et `fu` restent jamais libérés par `fm_free` (TROU-7, préservé). `score_panel_stars` remplace `invented` par sa propre liste, et l'ancienne fuit comme avant.

### Tests : forme des écritures de préparation (G)

Les tests ne changent **que dans leur façon d'accéder à l'état**. Les lectures par nom ne bougent pas. Les écritures directes de préparation, que le compilateur désigne, passent à l'affectation typée **avec la même valeur** :

- `pico8_fm_hy()->nz = pico8_value_fixed(pico8_fixed_from_int(n))` → `pico8_fm_hy()->nz = pico8_fixed_from_int(n)` (8 fichiers `set_credits`, `cheat_code:85-86` pour `jk`/`nz`) ;
- `test_pico8_hy` `fm_set(name, v)` : signature et appelants gardés, le corps devient `assert(v.kind == PICO8_VALUE_FIXED); fm->nz = v.as.fixed;` (idem `jk`, `yr`). La caractérisation passe par `fm_set`, donc compile sur les deux formes ;
- `score_panel_stars:114` → `pico8_fm_hy()->invented = invented;` ; `:116` → `pico8_fm_hy()->happiness = pico8_fixed_from_int(happiness); pico8_fm_hy()->has_happiness = 1;` ;
- `test_pico8_pj` (les deux `pico8_fm_hy()->hl = pico8_value_string("final score")`) → `pico8_fm_hy()->hl = "final score"; pico8_fm_hy()->has_hl = 1;`.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 253 sur l'arbre de départ (1b.6 commitée), puis 242.
- **H2** : aucun code n'écrit un membre de `hy` par nom. `pico8_entity_set_field` sur `fm` n'existe que dans `must_abort` (`test_pico8_hy` `write_nz`, `test_pico8_fm` `write_jk` sur le provisoire).
- **H3** : aucune écriture de `nz`/`jk` sur un `hy` n'est nil, et les preuves `stat?` de la table viennent toutes de `fm_placeholder`. `test_construction` lit `nz` présent sur `bare`. Si un rejeu ou un TU lisait `nz`/`jk` absent sur un `hy`, un `assert(lookup…)` de test rougirait : c'est l'arrêt « champ sans règle ».
- **H4** : `hl` ne tient jamais `BOOL` 1, ni une chaîne NULL ou allouée, dans les 15 rejeux et les TU. Ses seules chaînes sont les littéraux de `lj` et de `test_pico8_pj`.
- **H5** : les 15 empreintes et les valeurs figées ne bougent pas : `CONSTRUCTION_HASH` … `GAME_END_REPLAY_HASH` de `test_pico8_hy`, les `*_HASH` des tests `set_credits`, et ceux de `test_pico8_pj`. `fold_field` lit par nom le même genre et la même valeur.
- **H6** : `test_pico8_hy` (dans `UNIT_TESTS`, hors `UNIT_TESTS_SLOW`) reste sous 30 s avec la caractérisation. Il rejoue déjà deux fixtures, et sa durée n'est pas mesurée ici.
- **H7** : `make class-coverage-net` rend `hy  19  15/15`, identique à la référence.
- **H8** : le build `-DPICO8_MEMBER_CENSUS` compile encore. Il n'est pas lancé.
- **H9** : `make float-audit` (dans `make e2e`) retrouve ses deux clés C1 `src/pico8_economy.c | pico8_hy_hk | k_val.as.fixed | 1` et `… | pico8_hy_kt | ew | 1`, parce que les extraits ne changent pas. Sinon, c'est « entrée sans site », et `e2e` rougit.
- **H10** : `menu.c:319` (`ix[1].my`) est exécuté par `test_pico8_game_end`, dont le rejeu clique `ix[1]` jusqu'à 3426, et `cheat.c:36-37` par `test_pico8_cheat_code`.
- **H11** : `pico8_ob_table_free` accepte le `pico8_ob_table_t *` tel quel, et les appels de `hk` compilent sans cast (`-Werror`).
- **H12** : la preuve rouge de 1a.35 tient encore pour `cheat_code` (branche `hy` d'`apply` vidée) et pour `score_panel_stars` (constructeur de `lq` qui remplit `items` dans l'ordre de stockage).

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom ou à l'écran.** La caractérisation ne fait aucun accès `->m`, sauf par `fm_set`, et compile sur les deux formes.
- **Caractérisation, figée sur l'ancien code** : `test_hy_members_by_name`, dans `test_pico8_hy`, lancée par `in_fresh_child` après `test_hy_is_a_struct`. Pas de nouveau TU. Le témoin est un fixe `0x1234` dans `out` avant chaque lecture attendue absente.
  1. **`hy` neuf** (`pico8_hy_new(NULL)` après `pico8__init()`) :
     - `nz` `FIXED 100`, `ge` `FIXED 0`, `jk` `FIXED 0`, `ew` `FIXED 1`, `lo` `FIXED 1`, `yr` `FIXED 3401` ;
     - `invented` `REF` d'étiquette `LIST`, `count` 0 ;
     - `po`, `fu`, `happiness`, `hl` : le lecteur (`cls->field`) rend 0, et `out` reste égal au témoin.
  2. **Après `new_game()`** (`eu()`, donc un `lj`) :
     - `po`/`fu` `REF` non NULL ;
     - `happiness` `FIXED` ;
     - **`hl` présent, `BOOL` 0**, et non absent ;
     - `invented` : deux lectures donnent le même pointeur ;
     - le lecteur sur `"x"`, `""`, `"hk"`, `"kr"`, `"jo"` rend 0, `out` intact.
  3. **Identité des pointeurs** : `po`/`fu` lus deux fois de suite → même pointeur. Après `pico8_hy_lj`, ce sont d'autres pointeurs, non NULL. Après `pico8_hy_kt` avec un `px`, `invented` garde le même pointeur, et son `count` a monté de 1.
  4. **`hl`, les trois états présents** :
     - `fm_set("yr", 3426)` + `lj` → `STRING` `"final score"` ;
     - puis `fm_set("yr", 3425)` + `lj` → **`BOOL` 0 présent** (retour de chaîne à faux) ;
     - `new_game()`, `fm_set("nz", 2)` + `lj` → `STRING` `"bankrupt"` ;
     - `fm_set("nz", 3)` + `lj` → `BOOL` 0 présent.
  5. **Triche sur un `hy`** : après `new_game()`, la séquence de `test_cheat_on_provisional` (même tableau `presses`, 16 frames, recopiée en aide locale ; `test_cheat_on_provisional` lui-même ne change pas) passée à `pico8_cheat_step` donne `jk` `FIXED 99` et `nz` `FIXED 200`. `test_cheat_on_provisional` ne couvre que le provisoire. Ce cas fige dans `test_pico8_hy` la branche `hy` d'`apply`, que seul `cheat_code` atteignait.

  Elle est lancée verte sur l'arbre de départ **avant** le typage. Ses valeurs ne changent pas ensuite. Le diff de `test_pico8_hy.c` est un pur ajout, plus le corps de `fm_set`.
- **Tests existants qui figent déjà beaucoup**, inchangés : `test_construction` (présences sur `bare` et après `new_game`), `test_lj` (`hl_string` : `BOOL` 0 → NULL, les deux chaînes), `test_kt`, `test_hy_is_a_struct` (`set_field` NULL, `must_abort` ×3), `test_game_end` (`fm_hl`), `test_pico8_pj` (`ph` sur `hl`).
- **Tests de régression (§6)** touchés : `cheat_code` et `score_panel_stars`, **dans leur seule forme d'écriture**. Aucune assertion retirée ni affaiblie, mêmes valeurs, mêmes frames, même fixture. Preuve rouge de chacun sur le code final, dans une copie jetable du scratchpad, par `make tmp/test_pico8_<nom> && ./tmp/test_pico8_<nom>` (`score_panel_stars` avec `1` puis `2`). On reprend les bugs de référence de 1a.35 :

  | Test | Bug remis localement | Attendu |
  |---|---|---|
  | `cheat_code` | dans `apply`, la branche `hy` n'écrit rien | `assert_cheat_applied` faux à la frame 23 |
  | `score_panel_stars` | constructeur de `lq` qui remplit `items` dans l'ordre de stockage (mutation 3 de 1a.34) | rangée d'étoiles fausse pour 1 et 2 étoiles |

  Les six autres tests de régression de la ligne 1a.35 (`empty_planet_cleared`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `tech_screen`) lisent par nom : **diff vide**. **Si un bug remis ne rend pas son test rouge, c'est un arrêt (§7), pas un test durci.**
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune lancée par `make tmp/test_pico8_hy && ./tmp/test_pico8_hy` (et le binaire cité s'il y en a un autre). Chacune est notée dans le commit avec le test qui la rougit.
  1. (§7-1) *indicateur forcé vrai* : le lecteur ignore `has_happiness`. Attendu rouge : caractérisation 1 (`happiness` présent 0 sur `bare`), `test_construction` (`has(bare, names[n]) == (n < 7)`).
  2. (§7-2) *« présent 0 » là où c'était absent*. Pas d'écrivain ni de clé : la branche R5 `REF NULL` n'existe pas. Sa forme sur `hy` : le bloc d'ouverture du constructeur pose `has_hl = 1`, et `hl` d'un `hy` neuf se lit `BOOL` 0 (le « présent 0 » d'un {bool, chaine}). Attendu rouge : caractérisation 1, `test_construction`.
  3. (§7-3) *init qui saute un membre typé* : pas de boucle de clés. Sa forme : le bloc `pq` saute `yr = 3401` (reste 0). Attendu rouge : caractérisation 1, `test_construction` (`get_fixed(bare,"yr")`), `CONSTRUCTION_HASH`.
  4. Propre à l'étape, *faux devenu nil, invisible à l'écran* : `pico8_hy_hl` rend `REF NULL` quand `hl` est NULL. Les pixels ne bougent pas, car `fm.hl and …` et `fm.hl or …` ne distinguent pas `false` de nil. Attendu rouge : caractérisation 2 et 4, `test_lj` (`assert(lookup(…,"hl"))` de `hl_string`), `test_construction` (tous présents après `new_game`).
  5. Propre à l'étape, *présence oubliée dans R6* : la branche `false` de `lj` pose `hl = NULL` sans `has_hl = 1`. Attendu rouge : caractérisation 2 et 4, `test_construction`, `test_lj`.
- **`gcov`** : `test_pico8_hy` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23, plus `test_pico8_cheat_code` et `test_pico8_game_end` pour `cheat.c` et `menu.c` (H10). Chaque ligne ajoutée ou modifiée doit être exécutée :
  - les 11 branches du lecteur, avec leurs `return 0` sur `happiness`, `hl`, `po`, `fu` absents ;
  - les trois branches `hl` de `lj` ;
  - `kt`, `hk`, `lu` et `nx`.

  Trous attendus et justifiés :
  - le `return 0` sur `invented == NULL` : inatteignable, le constructeur l'alloue toujours ;
  - la branche `self == NULL` de `pico8_hy_hl` : le lecteur ne passe jamais NULL, et la caractérisation, qui doit compiler sur l'ancien code, ne peut pas appeler l'emballage.

  Aucun autre trou n'est attendu. Le rapport va dans le commit.
- **Prior art** : `test_construction`, `test_lj` et `test_hy_is_a_struct` (`test_pico8_hy`), `test_trade_members_by_name` (1b.5), `test_mh_members_by_name` (1b.1), et le tableau de régression de 1a.35 (`docs/prd/done/plan3-phase1a-etape35-hy-economie.md`).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min ;
- `make sanitize-net` : ~4 min seul, le double sous charge ; on ne le lance **jamais** en même temps que `e2e` ;
- `make class-coverage-net` : 3 à 4 min ;
- `make value-members` : quelques secondes.

Mutations et preuves rouges passent par un binaire seul (`make tmp/test_pico8_<nom>`), jamais par `make test`.

**Compilation précoce des TU lents** : juste après le typage, `make tmp/test_pico8_os tmp/test_pico8_qh tmp/test_pico8_hj tmp/test_pico8_mx tmp/test_pico8_nr` (construction seule, sans lancement). `make test` ne les compile pas, et une erreur de `set_credits` ne se verrait sinon qu'à `make e2e`.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` sans erreur de liste C1, et les `UNIT_TESTS_SLOW` qui passent ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 242`**, sans aucune ligne `pico8_hy_t` ;
5. `timeout 15m make class-coverage-net` : **s'applique**. Le diff touche le corps du constructeur `pico8_hy_struct_new` (R3), même si `pico8_class_coverage_record("hy")` et `KNOWN[]` ne bougent pas. C'est la même lecture du §7 qu'en 1b.5. La sortie doit être identique à `tools/class_coverage_net_expected.txt` (`hy  19  15/15`).

`make member-census` ne s'applique pas (R8). Jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si :
  - un test lit `nz`/`jk` absent sur un `hy` (H3) ;
  - `hl` tient `BOOL` 1 ou une chaîne NULL (H4) ;
  - un membre est ajouté à `hy`, sauf `has_happiness` et `has_hl` ;
  - un écrivain, une méthode ou une clé de construction est ajouté ;
  - le lecteur accepte un autre nom.
- `FAILED:` si un type diffère de la Solution : un fixe qui n'est pas `pico8_fixed_t`, `invented` qui n'est pas `pico8_list_t *`, `po`/`fu` qui ne sont pas `pico8_ob_table_t *`, `hl` qui n'est pas `const char *`, un indicateur qui n'est pas `int` déclaré juste après son membre. Également si `has_nz`, `has_jk` ou `hl_is_bool` existent.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur un membre nil ou un nom inconnu, ne rend pas le même pointeur pour `invented`/`po`/`fu`/`hl`, ou lit absent un `hl` `false`.
- `FAILED:` si `hy_member` ou `member_fixed` survivent, si `pico8_hy_hl` manque à `economy.h`, ou si une preuve de R7 n'est pas vide.
- `FAILED:` si une libération est ajoutée pour `invented`, `po`, `fu` ou `hl` (TROU-7), ou si un autre écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` si le message d'un `assert` existant change (`hy: every measured construction passes no key…`, `hy: every measured call passes the hy of fm…`).
- `FAILED:` si un extrait C1 de `tools/float_audit_c1.txt` ou ce fichier changent.
- `FAILED:` si le diff d'un test fait autre chose que :
  - l'ajout de la caractérisation ;
  - le corps de `fm_set` ;
  - les écritures de préparation listées (même valeur, forme typée).

  Également si une lecture par nom change, si une valeur figée existante change (`*_HASH`, `*_REPLAY_HASH`), ou si un test de régression autre que `cheat_code` et `score_panel_stars` a un diff non vide.
- `FAILED:` si `cheat_code` ou `score_panel_stars` perdent ou affaiblissent une assertion, si leur preuve rouge manque au commit (bug remis, échec observé, correctif restauré, vert), ou si un bug remis ne les rend pas rouges.
- `FAILED:` si un site par nom de `src/` change.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 242, liste encore `pico8_hy_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` si `make class-coverage-net` n'a pas de ligne « Runs » identique à la référence, ou si cette référence, les exemptions ou `KNOWN[]` changent.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh` change.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit un membre directement, ou si ses valeurs changent après le typage.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 5 mutations, chacune avec son test, ou la justification de son trou.
- `FAILED:` s'il manque la note « Découvert à 1b.7 », ou si elle omet la lecture de R1 pour {bool, chaine} ou le constat « R5 écrivain toujours vacante ».
- `FAILED:` s'il manque le tableau « Runs ».

## Out of Scope

- `fm_placeholder.jk`/`nz` et leurs accès dans `probe.c` et `cheat.c`, branche provisoire (1b.8).
- Le type du slot `g_fm` (`pico8_struct_header_t *`) et `pico8_fm()`.
- Libérer `invented`, `po` ou `fu` (TROU-7).
- Remplacer une lecture par nom par un accès direct.
- Fixer la forme de l'écrivain de R5, celle de R7 `offsetof`, ou celle de {bool, pointeur} quand `true` est écrit.
- Régénérer `tools/member_census.txt` (R8).
- Les fichiers d'événements et `FRAMES_EVENTS`.
- Tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `economy.c/.h`, plus une ligne par site dans `cheat.c` et `menu.c` ;
  - tests : `test_pico8_hy` étendu, sans nouveau TU, et onze autres tests retouchés dans leur seule forme d'écriture ;
  - un commit, avec le découpage de repli écrit ;
  - `hl` en `const char *` + `has_hl`, où NULL présent vaut `false` ;
  - un emballage public `pico8_hy_hl` ;
  - pas de `has_nz`/`has_jk` (preuve `stat?` d'homonyme) ;
  - `has_happiness` ;
  - `member_fixed` supprimée ;
  - R5 vacante, les deux moitiés ;
  - `class-coverage-net` lancé.
- **Note « Découvert à 1b.7 »**, à écrire sur la ligne 1b.7 de `docs/plan-3.md`. Elle ne garde que ce qui change une étape suivante :
  - « **lecture nouvelle de R1** : genres mêlés {bool, chaine}, dont le seul booléen écrit est `false` (preuve du PRD, cart et C) → `const char *<m>` + `int has_<m>`, même pour un pointeur : `has_<m>` = 0 nil ; 1 avec `<m>` NULL `false` ; sinon la chaîne, même pointeur ; un emballage public unique, `pico8_X_<m>`, comme en 1b.5. La forme quand `true` peut être écrit (`qh.et`, `g_.et`, `mz.et`, `js.my`…) reste à fixer par le premier PRD qui en a un » ;
  - « R2 : un nil observé seulement par `stat?` venu d'un homonyme (ici `fm_placeholder`, 1b.8) ne donne pas d'indicateur quand toute écriture sur le typedef est prouvée non nil » ;
  - « R7 `pico8_X_member` appliquée sans écart (`hy_member` supprimé) ; R5 vacante, les deux moitiés : `hy` n'a ni `set_field` ni clé de construction, et l'écrivain de R5 reste à fixer ».

  Le reste va dans le commit.
- Le message de commit porte :
  - le périmètre ;
  - le compte 253 → 242 ;
  - le résumé du tableau « Runs » ;
  - le rapport `gcov` ;
  - les 5 mutations ;
  - les deux preuves rouges de régression ;
  - les réponses à H1 à H12.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
