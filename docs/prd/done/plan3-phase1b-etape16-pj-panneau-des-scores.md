---
title: "Plan 3, étape 1b.16 : `pj` (le panneau des scores) — six membres typés, `ed` ({fixe, ref:pico8_mh_t}) en `pico8_mh_t *` dont NULL lit le fixe 1 de la spec (lecture nouvelle de R1), `c_`/`gz` avec `has_`, et les lignes `fa`/`ed` sorties de `g_pj_defaults[]`"
description: "`pico8_pj_t` a six membres `pico8_value_t`, tous lus par nom et jamais écrits par nom (pas de `set_field`). Les typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 202 à 196. `ed` mêle un fixe et une REF vers une `mh` : c'est un mélange nouveau, tranché ici comme une lecture nouvelle de R1. Le seul fixe que `ed` tient est le défaut 1 de la spec, et nil n'y est jamais écrit : `ed` devient donc un `pico8_mh_t *` sans indicateur, dont NULL se lit « fixe 1 ». La forme écrivain de R5 (1b.13) reste vacante, puisque `pj` n'a pas d'écrivain par nom."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.16 — `pj` (`score_panel.h:18`), genres mêlés `ed` (fixe ou REF vers `mh`, 1b.1). Empreinte 17 (7, 10). Compte : 196.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié), le §6 (tests de régression) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité ici, pas redécidé. Les notes « Découvert à » suivantes de `docs/plan-3.md` s'appliquent aussi, sans être redérivées :

- **1b.3** à **1b.8** : patron appliqué sans écart ; forme de la branche R4 à `strcmp` (1b.5) ; `fixe` à nil `jamais` sans indicateur, même quand un homonyme en a un (1b.6) ;
- **1b.4b** : la table corrigée fait foi. **`pj.gz` exige `has_gz`** (point (3) de la note : non-pointeur, lu par nom, nil observé). `c_` exigeait déjà `has_c_` (`bool`, nil observé `lu,stat`) ;
- **1b.5** : lecture de R1 pour {fixe, bool} (`pico8_fixed_t` + `has_` + `<m>_is_bool`, un emballage public unique `pico8_X_<m>`, appelé par le lecteur) ;
- **1b.7** : lecture de R1 pour {bool, chaine}, dont le seul booléen écrit est `false`. On y trouve `const char *` + `has_`, avec un écart assumé avec R2 (un NULL présent y veut dire `false`, pas nil), et le même emballage public unique. **C'est le modèle de la lecture nouvelle de `ed` ci-dessous** ;
- **1b.9** : **R7 forme `offsetof`** (`members[]`, table entière). Chaque entrée d'un membre typé est retirée et une table vidée est supprimée avec sa boucle. Ses usages deviennent des branches R4 `strcmp` dans l'ordre du typedef. La preuve mécanique est `grep -n 'pico8_value_t \*' … | grep -v 'pico8_value_t \*out_value)' | grep -v 'pico8_value_t \*out)'` ;
- **1b.10** : lecture {fixe, bool} de 1b.5 appliquée sans écart ;
- **1b.11** : **règle R2 des TU lents**. Chaque PRD relit les TU de `UNIT_TESTS_SLOW` qui touchent sa struct pour y chercher des lectures nil par nom (voir plus bas) ;
- **1b.12** : patron appliqué sans écart ;
- **1b.13** : **la forme écrivain de R5**, y compris sa règle « chaque test d'arrêt porte un cas non-REF à union entièrement nulle, construit par `memset` ». **Vacante ici** : voir « R5 » ;
- **1b.14a** : **R7 pour une table de défauts qui garde des entrées**. L'entrée du membre typé sort de toute table qui la liste, quel que soit son format, et la table garde ses autres entrées ;
- **1b.14b** : R7 de 1b.14a sans `static` déballé pour un fixe (« le littéral de l'init suffit pour un fixe ») ; un cas « union nulle » se construit par `memset` puis `kind` ; une mutation qui retire l'unique appel d'une fonction `static` est plantée sous une forme qui garde l'appel (`-Werror`).

### État de départ

HEAD `d7144a5` (PRD de 1b.15). 1b.15 (`bg`) est **en cours d'implémentation**. **Précondition** : 1b.15 est commitée avant que 1b.16 commence. Elle ne touche que `src/pico8_bg.[ch]`, `tests/test_pico8_bg.c` et `docs/plan-3.md`, et rien de ce que 1b.16 touche. `make value-members` rend alors 202 (H1). Les numéros de ligne cités sont ceux de HEAD et peuvent glisser. Chaque site est aussi désigné par son contenu (H9).

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code et du cart. Tout fait d'exécution est une hypothèse (H1 à H13).

### Lignes de recensement (`tools/member_census.txt:186-191`)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `fa` | offset | `fixe` | jamais | - | l |
| `gs` | offset | `ref:table` | jamais | - | l |
| `i_` | offset | `ref:table` | jamais | - | l |
| `ed` | offset | **`fixe,ref:pico8_mh_t`** | jamais | - | l |
| `c_` | offset | `bool` | observé | lu,stat | l |
| `gz` | offset | `fixe` | observé | lu,stat | l |

Six membres : 202 − 6 = **196**. Aucun membre n'a `e` : **`pj` n'a pas d'écrivain par nom** (`g_pj_struct_class.set_field` vaut NULL). `test_pj_is_a_struct` fige `cls->set_field == NULL` et `must_abort(write_c_, ju)`. `pico8_pj_cr_t` (1b.6) ne porte plus de `pico8_value_t`.

**Revue des TU lents (règle R2 de 1b.11).** Relus : `test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`. Aucun ne nomme `pico8_ju`, `pico8_pj_*`, `"ed"`, `"gz"`, `"c_"`, `"gs"` ni `"i_"`. Leurs lectures par nom sur des entités de `hv` visent d'autres noms (`fz`, `ra`, `ng`…), et le `ju` que `eu()` enregistre n'est lu par aucun d'eux par un nom de `pj`. **Aucune lecture nil par nom d'un membre de `pj`** (H4). Sans effet sur R2 : `c_` et `gz` ont déjà leur `has_`, `fa` et `ed` ne sont jamais nil, `gs` et `i_` sont des pointeurs.

### Ce que fait le cart (`pico8/lua.lua:815-903`, `:984`)

- Spec de `pj` : `fa=10`, `ea=o(…)`, `gs=o(…)`, `cr=o(…)`, `i_=o(…)`, `ou=o(…)`, **`ed=1`**. `pj()` est construit une seule fois, par `eu()` (`:1182`, `fm,ju=hy(),pj()`).
- **`ed`** : `pj:ph()` (`:882`) `if self.c_ and kc~=self.ed then … self.ed=mh(iw) end`. **C'est la seule écriture de `ed` du cart** : `self.ed` vaut le `1` de la classe jusqu'au premier score affiché, puis la dernière `mh` ouverte par `ph`. `1` n'est jamais `kc` : le bloc s'exécute au premier tick éligible. Le cart n'écrit jamais nil sur `ed`, ni un autre nombre.
- **`c_`** : `self.c_,self.gz,self.fa = true,0,13` (`ph`), `ju.c_ = not ju.c_` (`de.my`, `:901`), `ju.c_=true` (`ix[3].my`, `:984`). On lit `self.c_` dans `ph` et dans le `iw.lh` de `ph`. Nil au départ, puis toujours un booléen.
- **`gz`** : écrit `0` par `ph`, puis `self.gz+=0.2` par `pj:kj` (`:853`). Nil au départ, testé par `if self.gz` (`kj`) et `not self.gz` (`ph`).
- **`fa`** : `10` (classe), `13` (`ph`). **`gs`, `i_`** : la classe les lit en `pj.gs[…]` (`hk`) et `pj.i_` (`kj`). L'instance ne les lit jamais comme `self.gs`/`self.i_`, et seules les lectures par nom du port les voient.

### Le code aujourd'hui

`src/pico8_score_panel.h` :

- Typedef (`:17-25`) : `{ pico8_ke_t ke; pico8_value_t fa, gs, i_, ed, c_, gz; } pico8_pj_t`. Le contrat au-dessus dit : « `gs`, `i_` are the tables of `pj`'s class, borrowed and never modified. » L'en-tête n'inclut pas `pico8_msgbox.h` (pas de cycle : `msgbox.h` n'inclut pas `score_panel.h`).
- API publique : `pico8_pj_gs()`, `pico8_pj_i_()`, `pico8_pj_ou()`, `pico8_pj_ea()` et `pico8_pj_cr()` lisent l'état de classe. `pico8_pj_default`, `pico8_pj_new` et `pico8_pj_kj(gfx, mem, self)` complètent l'API.

`src/pico8_score_panel.c` :

- **Table de défauts** `g_pj_defaults[]` (`:99-114`) : `enum { PJ_FA, PJ_EA, PJ_GS, PJ_CR, PJ_I_, PJ_OU, PJ_ED }`, avec les colonnes `name`, `member`, `offset`, `value`. Quatre lignes sont des membres (`fa`, `gs`, `i_`, `ed`, avec leur `offsetof`), trois n'en sont pas (`ea`, `cr`, `ou`). L'init la remplit par `pico8_pj_default` (`:516-525`), puis `assert` que toute ligne sauf `fa`, `ed` et `cr` est une table (« its 4 table accessors cast these REFs to plain tables »). `pico8_pj_defaults_reset` la remet à nil. Les accesseurs publics lisent `ea`, `gs`, `i_` et `ou` par `pj_default_table(PJ_X)`. Le lecteur lit `ou`, `ea` et `cr`.
- **Lecteur** `pico8_pj_field` (`:402-435`) : branches `ou`, `ea`, `cr` sur la table, puis table locale `members[]` à `offsetof` (les six membres), lecture `*(const pico8_value_t *)`, puis repli `pico8_ke_field`. Un `REF NULL` final → `return 0`.
- **Constructeur** `pico8_pj_struct_new` (`:448-471`) : `malloc`, `pico8_ke_struct_init`, `c_` = `gz` = `REF NULL`, puis boucle `if (member) *(pico8_value_t *)(self + offset) = value`. Vient ensuite l'`assert` « the only measured road is pico8_pj_new(NULL) » (aucune clé de construction, donc la moitié boucle de R5 est vacante), puis `pico8_class_coverage_record("pj")`, `pico8_ke_struct_pq`, `PICO8_MEMBER_CENSUS_SEE`.
- **Lecture directe** `pico8_pj_kj` (`:358-363`) : `self->gz.kind == PICO8_VALUE_FIXED`, puis `self->gz.as.fixed + 0.2`, puis `self->gz = pico8_value_fixed(gz)`.

`src/pico8_score_panel_display.c` (R6) :

- `pico8_pj_de_my_dispatch` (`:150`) : `ju->c_ = pico8_value_bool(!pj_display_truthy(ju->c_))`.
- `pico8_pj_ph_lh_dispatch` (`:163`) : `pj_display_truthy(ju->c_)`.
- `pico8_pj_ph` (`:256-322`) : `has_gz = !(gz.kind == REF && gz.as.ref == NULL)`. Écrit `c_` = `bool(1)`, `gz` = `fixed(0)`, `fa` = `fixed(13)`, puis `pj_display_truthy(self->c_)`. **Lecture de `ed`** : `if (self->ed.kind == PICO8_VALUE_REF) ed_equals_kc = (pico8_kc() == self->ed.as.ref)`, sous le commentaire « `self.ed`'s class-level default is the number 1, never a REF … Comparing only a REF-valued `ed` silently disables it. » Puis `self->ed = pico8_value_ref(made)`, avec `made = pico8_mh_new(iw_table)`.

`src/pico8_menu.c` (R6) : `pico8_ix3_my_dispatch` (`:376`) fait `ju->c_ = pico8_value_bool(1)`.

**Sites par nom, inchangés** : lecteurs `src` génériques (`fa` des couches, par `ui:gw`/tri de `hv`) ; tests `test_pico8_pj` (tous les membres), `test_pico8_mh` (`ed`), `test_pico8_game_end` et `test_pico8_score_panel_close` (`c_`), `test_pico8_spec_defaults` (les sept clés de la spec, `STRUCT_HASH` : `same_value` compare `fa`/`ed` en fixe et `gs`/`i_` **par adresse** à la spec). `test_pico8_ui` construit un `pj` (`e[17]`) mais n'y lit que `me` et `t` (`ke`).

**Accès directs dans les tests** (ils ne compilent que sur l'ancienne forme) :

- `tests/test_pico8_pj.c:673` : `assert(ju->gs.as.ref == class_ref("gs") && ju->i_.as.ref == class_ref("i_"))` ;
- `tests/test_pico8_mh.c:255-257` (`set_c_` : `ju->c_ = pico8_value_bool(on)`) et `:281` (`ju->gz = pico8_value_fixed(pico8_fixed_from_int(0))`) : mise en place d'état, faute d'écrivain par nom.

### Empreinte remesurée

La ligne dit 17 (7, 10). C'est un majorant, à cause des noms courts partagés (`fa`, `ed`, `c_`, `gz`). **Empreinte réelle** (fichiers qui lisent ou écrivent un membre d'un `pico8_pj_t`) : **8 (3, 5)**. Côté `src` : `pico8_score_panel.c`, `pico8_score_panel_display.c`, `pico8_menu.c`. Côté `tests` : `test_pico8_pj.c`, `test_pico8_mh.c`, `test_pico8_game_end.c`, `test_pico8_score_panel_close.c`, `test_pico8_spec_defaults.c`.

**Fichiers modifiés : 7**, plus `docs/plan-3.md` :

- `src/pico8_score_panel.h`, `src/pico8_score_panel.c`, `src/pico8_score_panel_display.c`, `src/pico8_menu.c` ;
- `tests/test_pico8_pj.c` (ajouts et la ligne `:673`) ;
- `tests/test_pico8_mh.c` (les deux écritures directes de mise en place).

**Aucun TU de `UNIT_TESTS_SLOW` n'est touché.**

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code**, par nom seulement : les six membres sur un panneau neuf, après `ph`, après `kj`, après `de.my` et `ix[3].my`. On y ajoute les écritures par nom qui doivent avorter (nil, et non-REF à union nulle), qui figent qu'aucun écrivain n'apparaît. La ligne `test_pico8_pj.c:673` passe à la lecture par nom dans le même temps.
2. **Typage** : `pico8_fixed_t fa; pico8_ob_table_t *gs; pico8_ob_table_t *i_; pico8_mh_t *ed; int c_; int has_c_; pico8_fixed_t gz; int has_gz;`. L'en-tête `ke` reste en tête, et les noms comme l'ordre sont gardés.
3. **`ed`, lecture nouvelle de R1** : un `pico8_mh_t *` sans indicateur. NULL se lit « le fixe 1 de la spec », une `mh` se lit comme sa REF. Un emballage public unique, `pico8_pj_ed`, sert au lecteur.
4. **R7** : `members[]` du lecteur est supprimée (forme 1b.9). Les lignes `fa` et `ed` sortent de `g_pj_defaults[]` (forme 1b.14a). Les lignes `gs` et `i_` restent, en tant qu'état de classe que le cart lit (`pj.gs`, `pj.i_`). Les colonnes `member`/`offset` disparaissent.
5. **R6** : `ph`, `kj`, `de.my`, `ph.lh` et `ix[3].my` lisent et écrivent les types. La comparaison `kc ~= self.ed` teste le pointeur **sous garde NULL**.
6. Effet mesurable : `make value-members` passe de 202 à **196** et ne liste plus aucune ligne `pico8_pj_t`. Aucun pixel ne bouge.
7. **Note « Découvert à 1b.16 »**.

## User Stories

1. En tant que lecteur de `score_panel.h`, je veux `fa` et `gz` en `pico8_fixed_t`, `gs`/`i_` en `pico8_ob_table_t *`, `ed` en `pico8_mh_t *`, et `c_` en `int` avec `has_c_`, `gz` avec `has_gz`, afin que ce que tient le panneau se lise sur le type.
2. En tant que lecteur par nom de `ed`, je veux `FIXED 1` sur tout panneau que `ph` n'a pas encore touché, puis la REF de la `mh` que `ph` a ouverte, même adresse que `kc`.
3. En tant que `pj:ph`, je veux que le bloc du score s'exécute au premier tick éligible même quand `kc` est NULL, parce qu'un `ed` qui vaut encore le fixe 1 n'est jamais `kc`.
4. En tant que `pj:ph` sur un tick suivant, je veux qu'un `ed` égal à `kc` empêche de reconstruire la boîte.
5. En tant que lecteur par nom de `c_`, je veux 0 avec `*out_value` intact sur un panneau neuf, `BOOL 1` après `ph` ou `ix[3].my`, et `BOOL 0` **présent** après un `de.my` qui l'éteint.
6. En tant que `de.my`, je veux qu'un `c_` nil bascule à `true` présent, et un `true` à `false` présent.
7. En tant que lecteur par nom de `gz`, je veux 0 avec témoin intact avant `ph`, `FIXED 0` juste après, puis `0.2` de plus par `kj` ouvert.
8. En tant que `pj:kj`, je veux que la barre ne soit tracée que si `gz` est présent, comme `if self.gz`.
9. En tant que lecteur par nom de `fa`, je veux 10 sur un panneau neuf et 13 après `ph`, toujours présent.
10. En tant que lecteur par nom de `gs` et `i_`, je veux la même adresse que `pico8_pj_gs()`/`pico8_pj_i_()` et que la table de la spec, pour tout panneau.
11. En tant que `test_pico8_spec_defaults`, je veux `STRUCT_HASH` inchangé : `fa`/`ed` égaux au fixe de la spec, `gs`/`i_` égaux à sa REF.
12. En tant que lecteur de `ke`, je veux que `me` et `t` d'un `pj` restent servis par le repli `pico8_ke_field`, et que `ou`, `ea` et `cr` restent lus sur l'état de classe.
13. En tant que `test_pj_is_a_struct`, je veux `set_field` toujours NULL, et toute écriture par nom d'un membre de `pj` (valeur, nil ou non-REF à union nulle) toujours avortée par la couche entité.
14. En tant qu'auteur d'une étape suivante à genres mêlés {fixe, REF}, je veux une lecture écrite de R1 pour ce mélange, afin de la recopier ou de s'en écarter explicitement.
15. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
16. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
17. En tant que gardien de la couverture, je veux `make class-coverage-net` identique à sa référence (`pj  19  15/15`), puisque le constructeur change.
18. En tant que `plan-pilot`, je veux `make value-members` à 196.
19. En tant que vérificateur, je veux la caractérisation lancée sur l'ancien code, les mutations du §7 avec leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Il n'y a pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés comme le ferait un agent `guideline` (hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables), et marqués (G).

### `ed` : représentation, et pourquoi (lecture nouvelle de R1)

**Faits.** `ed` ne tient que deux choses :

- le fixe `1` du défaut de la spec, posé par le constructeur ;
- une REF non NULL vers la `mh` que `ph` vient de construire (`pico8_mh_new`, qui ne rend jamais NULL, H2).

Nil n'y est jamais écrit (ligne `jamais`, et `set_field` vaut NULL). Aucun autre fixe n'y est écrit, ni par le cart (`:899` est la seule écriture) ni par le C (`ph:321` est la seule écriture hors constructeur, et aucun test n'écrit `ed`). Une seule lecture directe existe (`ph`) : elle ne garde pas la valeur, elle compare l'adresse à `kc`.

**Options pesées** (G) :

| option | forme | verdict |
|---|---|---|
| A | `pico8_mh_t *ed;` seul ; NULL = « le fixe 1 de la spec » | **retenue** |
| B | `pico8_mh_t *ed_mh;` + `pico8_fixed_t ed;`, discriminant `ed_mh != NULL` | rejetée : un champ qui vaut toujours 1, et un nom (`ed_mh`) qu'aucune règle ne donne |
| C | petite paire étiquetée `{ int is_mh; pico8_fixed_t fixed; pico8_mh_t *mh; }` | rejetée : trois champs, avec une étiquette que R2 exclut (« pas de sentinelle, pas de masque ») |
| D | forme 1b.5 (`pico8_fixed_t` + `has_` + `ed_is_mh`) | impossible : un fixe ne porte pas un pointeur |

**Pourquoi A garde les valeurs par nom de 1a à l'identique.** Nil étant `jamais`, NULL est libre, exactement comme en 1b.7 où le seul booléen écrit (`false`) prenait le NULL présent. Le reste suit :

- un panneau neuf, ou tout panneau que `ph` n'a pas touché, se lit `FIXED 1`, comme sa copie du défaut de 1a ;
- après `ph`, il se lit `REF` vers la même `mh` (même adresse, tag `STRUCT`).

Le littéral `1` suit la lecture de 1b.14b (« le littéral de l'init suffit pour un fixe ») et la précédence de `mh` (`fa = 14`). Il ne dépend pas de l'état du module : après `pico8_score_panel_shutdown`, un panneau vivant se lit encore `FIXED 1`, comme sa copie de 1a. `STRUCT_HASH` et `test_built` le figent contre la spec.

**Forme écrite de la lecture**, qui sert de note : genres mêlés {fixe, `ref:pico8_X_t`} dont le seul fixe tenu est le défaut de la spec, à nil `jamais` → `pico8_X_t *<m>` sans indicateur. NULL veut dire « le fixe de la spec », jamais nil : c'est un écart assumé avec R2, comme en 1b.7. On ajoute un emballage public unique, `pico8_value_t pico8_X_<m>(const pico8_X_t *)`, qui rend :

- `REF NULL` pour `self` NULL ;
- `pico8_value_fixed(<défaut>)` pour `<m>` NULL ;
- `pico8_value_ref(<m>)` sinon.

Le lecteur l'appelle, et le membre est toujours présent. Une lecture directe qui ne garde que l'identité compare le pointeur **sous garde `<m> != NULL`** : sans cette garde, NULL égalerait une cible NULL (ici `kc` NULL). La forme pour un autre fixe écrit, ou pour nil écrit, reste à fixer par le premier PRD qui en a un. Jusque-là, c'est l'arrêt « champ sans règle », pas un élargissement.

En C :

- `score_panel.h` gagne `#include "pico8_msgbox.h"`, le champ `pico8_mh_t *ed;` et, au-dessus du typedef, le contrat « `ed` NULL reads as the spec's fixed 1, never nil » (un contrat C que le nom ne dit pas, règle des commentaires) ;
- on déclare `pico8_value_t pico8_pj_ed(const pico8_pj_t *self);` ;
- `ph` : `ed_equals_kc = self->ed != NULL && pico8_kc() == self->ed;` remplace le test de genre ; son commentaire reste exact au mot près et n'est pas retouché ;
- l'écriture devient `self->ed = made;`.

### R1–R9 appliquées à `pj`

| règle | sur `pj` |
|---|---|
| R1 | `fa`, `gz` → `pico8_fixed_t` ; `gs`, `i_` → `pico8_ob_table_t *` (non `const`) ; `c_` → `int` (0/1) ; `ed` → `pico8_mh_t *` (voir ci-dessus). Noms et ordre gardés. |
| R2 | `int has_c_;` juste après `c_`, `int has_gz;` juste après `gz` (note 1b.4b). Aucun autre : `fa` est nil `jamais` (lecture de 1b.6), `ed` aussi, et `gs`/`i_` sont des pointeurs. La revue des TU lents n'ajoute rien. |
| R3 | Dans `pico8_pj_struct_new`, à la place de `c_ = gz = REF NULL` et de la boucle de recopie, même position (après `pico8_ke_struct_init`, avant l'`assert` des clés et `pq`), dans l'ordre du typedef : `fa = pico8_fixed_from_int(10)` ; `gs = pj_default_table(PJ_GS)` ; `i_ = pj_default_table(PJ_I_)` (même REF qu'avant, celle de la spec, déjà vérifiée table par l'init) ; `ed = NULL` ; `c_ = 0`, `has_c_ = 0` ; `gz = 0`, `has_gz = 0`. Tout est écrit, puisque le bloc vient d'un `malloc`. |
| R4 | Les branches `ou`, `ea`, `cr` restent en tête, telles quelles. Suivent six branches `strcmp` dans l'ordre du typedef, avant le repli `pico8_ke_field` : `fa` → `pico8_value_fixed(fa)`, toujours présent ; `gs`, `i_` → NULL donne `return 0`, sinon `pico8_value_ref(…)` ; `ed` → `pico8_pj_ed(pj)`, toujours présent ; `c_` → `has_c_` = 0 donne `return 0`, sinon `pico8_value_bool(c_)` ; `gz` → `has_gz` = 0 donne `return 0`, sinon `pico8_value_fixed(gz)`. À chaque 0, `*out_value` reste intact. |
| R5 | **Les deux moitiés sont vacantes.** Il n'y a pas d'écrivain par nom (`set_field` NULL, aucun membre `e`), donc la forme écrivain de 1b.13 ne s'applique pas. Il n'y a pas de clé de construction : l'`assert` « the only measured road is pico8_pj_new(NULL) » reste tel quel. Il n'y a aucune aide `<X>_take_<genre>` à écrire. |
| R6 | `ph` : `self->c_ = 1; self->has_c_ = 1; self->gz = 0; self->has_gz = 1; self->fa = pico8_fixed_from_int(13);`, `has_gz` local = `self != NULL && self->has_gz`, `c_truthy` = `self != NULL && self->has_c_ && self->c_`, `ed` comme ci-dessus. `kj` : `if (self != NULL && self->has_gz)`, `gz = self->gz + 0.2`, `self->gz = gz`. `de.my` : `ju->c_ = !(ju->has_c_ && ju->c_); ju->has_c_ = 1;`. `ph.lh` : `ju != NULL && ju->has_c_ && ju->c_`. `ix[3].my` (`menu.c`) : `ju->c_ = 1; ju->has_c_ = 1;`. `pj_display_truthy` reste pour `hl` et `pp`. |
| R7 | Voir la section suivante. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_pj_t)` est gardé, et `tools/member_census.txt` n'est pas édité. |
| R9 | 202 → 196, aucune ligne `pico8_pj_t`. |

### R7 : deux tables, deux formes existantes

1. **`members[]` du lecteur** : forme `offsetof` de 1b.9, sur la table entière. Les six entrées sortent, et la table est supprimée avec sa boucle et sa lecture `*(const pico8_value_t *)`. Il n'y a pas d'accesseur `pico8_pj_member`.
2. **`g_pj_defaults[]`**, une table de défauts sans fille qui garde des entrées : forme 1b.14a, point (1). **Les lignes `fa` et `ed` sortent** (leur seul client est la recopie) :
   - leur défaut n'est pas déballé dans un `static` : c'est un fixe, et le littéral de R3 suffit (1b.14b) ;
   - l'`enum` perd `PJ_FA` et `PJ_ED` ;
   - l'`assert` de l'init perd l'exemption `i == PJ_FA || i == PJ_ED` et garde son message, qui reste exact (quatre tables : `ea`, `gs`, `i_`, `ou`).

   **Les lignes `gs` et `i_` restent** (G). Ce ne sont pas des copies de défaut : c'est l'état de classe `pj.gs`/`pj.i_` que le cart lit sur la classe, et que les accesseurs publics `pico8_pj_gs()`/`pico8_pj_i_()` rendent (`hk` lit `pico8_pj_gs()`, `kj` lit `pico8_pj_i_()`). Les sortir forcerait deux `static` qui doubleraient la table. Ce qui sort, c'est leur `offsetof`. La table n'ayant plus aucune ligne membre, **les colonnes `member` et `offset` disparaissent** : elle devient `{ const char *name; pico8_value_t value; }`. Le point (1) de 1b.14a est donc lu ainsi : la ligne sort quand elle ne sert qu'à la recopie, l'offset sort toujours. On le note dans « Découvert à 1b.16 ».

### Écriture par nom : vacante, et figée comme telle

`pj` n'a pas d'écrivain, donc aucune branche, aucune aide `static` et aucun `assert` « champ sans règle » à écrire. Le test d'arrêt de 1b.13, **rouge d'abord**, n'a pas d'objet : aucun nouvel `assert` n'est à atteindre. La règle des cas `memset` de 1b.13 est quand même honorée côté caractérisation (G). Les écritures par nom qu'une étape future pourrait être tentée d'accepter (nil sur `c_`, `false` à union nulle sur `ed`, fixe 0 à union nulle sur `gz`) sont figées **avortées** par `must_abort`, vertes sur l'ancien code (arrêt « a struct a generic writer reaches needs a field writer » de `pico8_entity_set_field`). Ajouter un `set_field` à `pj` dans cette étape est un `FAILED:`.

### Tests : lignes changées par le typage

- `test_pico8_pj.c:673` : `ju->gs.as.ref == class_ref("gs") && ju->i_.as.ref == class_ref("i_")` devient `get_ref(ju, "gs") == class_ref("gs") && get_ref(ju, "i_") == class_ref("i_")` (G). La forme par nom compile sur les deux formes, dit la même identité par le lecteur et se change **sur l'ancien code**, avec la caractérisation.
- `test_pico8_mh.c` : `set_c_` devient `ju->c_ = on; ju->has_c_ = 1;` et la ligne `:281` devient `ju->gz = pico8_fixed_from_int(0); ju->has_gz = 1;`. Ce sont des écritures directes de mise en place, sans écrivain par nom possible, sur le modèle de `pico8_fm_hy()->hl = …; ->has_hl = 1;` (1b.7) de `test_pico8_pj`. `set_c_(ju, 0)` pose un `false` **présent**, comme `pico8_value_bool(0)` avant. Elles changent dans le commit du typage. Aucune autre ligne de `test_pico8_mh.c` ne bouge, et aucune valeur figée non plus (H5).

### Preuves

Toutes vides après l'étape, sauf mention :

- `grep -nE '^\s*pico8_value_t [a-z_]+;' src/pico8_score_panel.h` : six lignes avant ; après, **vide** ;
- `grep -rn 'offsetof(pico8_pj_t' src/` : 6 lignes avant (4 de `g_pj_defaults`, 2 de `members[]`) ; après, vide ;
- `grep -rnE '(->|\.)(fa|gs|i_|ed|c_|gz)\.(kind|as)' src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_menu.c tests/test_pico8_pj.c tests/test_pico8_mh.c` : `kj`, `ph` et `test_pico8_pj.c:673` avant ; après, vide ;
- `grep -nE '(->)(fa|gs|i_|ed|c_|gz) = pico8_value_' src/pico8_score_panel.c src/pico8_score_panel_display.c src/pico8_menu.c tests/test_pico8_pj.c tests/test_pico8_mh.c` (fichiers nommés : ces noms courts sont partagés ailleurs) : 11 lignes avant (constructeur ×2, `kj`, `de.my`, `ph` ×4, `menu.c`, et 2 dans `test_pico8_mh.c`) ; après, vide ;
- `grep -n 'int member;' src/pico8_score_panel.c` : 1 avant ; après, vide ;
- `grep -n 'pico8_value_t \*' src/pico8_score_panel.c | grep -v 'pico8_value_t \*out_value)' | grep -v 'pico8_value_t \*out)'` : avant, les lectures/écritures par offset et le pointeur de l'init ; après, **une seule ligne**, `pico8_value_t *v = &g_pj_defaults[i].value;` (le remplissage de la table, pas un membre) ;
- `grep -n 'has_c_\|has_gz' src/pico8_score_panel.h` : deux lignes, chacune juste après son membre ;
- `grep -n 'self->ed != NULL' src/pico8_score_panel_display.c` : une ligne, celle de `ed_equals_kc` ;
- `grep -rn 'set_field' src/pico8_score_panel.c` : seule l'entrée `NULL` du descripteur, avant comme après.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 202 sur l'arbre de départ (1b.15 commitée), puis 196.
- **H2** : dans les 15 rejeux et dans tous les TU, `ed` d'un `pj` ne tient que `FIXED 1` ou une REF non NULL vers une `mh` (`pico8_mh_new` ne rend jamais NULL). Sinon la représentation A ne tient pas, et c'est l'arrêt « champ sans règle ».
- **H3** : `c_` ne tient que `BOOL` ou nil, et `gz` que `FIXED` ou nil. Donc le test `gz.kind == FIXED` de `kj` et le test « pas `REF NULL` » de `ph` coïncident, et les deux deviennent `has_gz`.
- **H4** : aucun TU de `UNIT_TESTS_SLOW` ne lit un membre de `pj` nil par nom (revue ci-dessus).
- **H5** : aucune de ces valeurs ne bouge : `BUILT_HASH`, `SHARED_HASH`, `LIFE_HASH`, `REPLAY_HASH`, `CR_HASH` (`test_pico8_pj`) ; `STRUCT_HASH`/`SPEC_HASH` (`test_pico8_spec_defaults`) ; les empreintes de `test_pico8_mh`, `test_pico8_game_end`, `test_pico8_score_panel_close`, `test_pico8_score_panel_stars`, `test_pico8_ui` ; les 15 du filet.
- **H6** : le build `-DPICO8_MEMBER_CENSUS` compile sur une struct sans `pico8_value_t` propre. Il n'est pas lancé.
- **H7** : sous ASan+UBSan, rien de nouveau : aucune allocation ajoutée.
- **H8** : `test_pico8_pj` reste sous 30 s avec la caractérisation (un fils de plus : une partie, `ph`, trois `kj`, deux `de.my`, `ix[3].my`). Sa durée actuelle n'est pas connue (`test_replay` rejoue `game_end_events.json`). Si elle dépasse, l'étape s'arrête et le note, et le TU ne change pas de liste.
- **H9** : 1b.15, une fois commitée, ne touche aucun fichier de cette étape. Les numéros cités glissent peut-être.
- **H10** : `make class-coverage-net` rend `pj  19  15/15`, identique à `tools/class_coverage_net_expected.txt`. Le diff change le corps du constructeur, pas le nombre d'instanciations.
- **H11** : le lecteur brut `pico8_pj_struct_class()->field` rend `me` (`STRING "ph"`) et `t` par le repli `pico8_ke_field`, comme `pico8_entity_get_field`.
- **H12** : `pico8_msgbox_set_kc(NULL)` avant le premier `ph`, puis `ph` avec `hl` posé, construit la boîte et en fait `kc` (ce qu'assert déjà `test_life`, sans maîtriser `kc` avant).
- **H13** : la rangée 3 de `pico8_ix()` porte `my` = `pico8_ix3_my_dispatch`, qu'on peut appeler hors menu ouvert (elle construit un `fq` sur `de`).

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom, ou l'écran.** Aucun accès direct à `->ed`, `->c_`… dans les tests nouveaux : ils compilent sur les deux formes. Les deux lignes de `test_pico8_mh.c` sont l'exception déclarée plus haut.
- **Caractérisation dirigée, figée sur l'ancien code** (G) : `test_pj_members_by_name`, dans `test_pico8_pj` (rapide, dans `CENSUS_UNIT_TESTS`), lancée par `in_fresh_child` après `test_cr_members_by_name`. Elle lit par le **lecteur brut** (`pico8_pj_struct_class()->field`, témoin : fixe `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder.
  1. **Panneau neuf.** `ju = start_game()`, `b = pico8_pj_new(NULL)`. Pour chacun :
     - `fa` → 1, `FIXED 10` ;
     - `gs` et `i_` → 1, `REF`, même adresse pour `ju`, pour `b`, pour `class_ref(…)` et pour `pico8_pj_gs()`/`pico8_pj_i_()`, tag `OB_TABLE` ;
     - **`ed` → 1, `FIXED 1`** (genre `FIXED`, pas `REF`) ;
     - `c_` et `gz` → 0, témoin intact ;
     - `ou`/`ea`/`cr` → mêmes adresses que les accesseurs ;
     - `me` → `STRING "ph"`, `t` par le repli (H11).

     Les intrus `""`, `"x"`, `"ED"`, `"e"`, `"ed "`, `"has_c_"`, `"has_gz"`, `"c"`, `"ke"`, `"header"`, `"member"` → 0, témoin intact.
  2. **`de.my` depuis nil**, sur le jeu de `ju` : `de_my()` → `c_` = `BOOL 1` présent ; `de_my()` → `c_` = **`BOOL 0` présent** ; `ix[3].my` (H13) → `BOOL 1`.
  3. **`ph` avec `kc` NULL** : `pico8_msgbox_set_kc(NULL)`, `hl` posé (comme `test_life`), `call(ju, "ph")`. On attend :
     - `c_` = `BOOL 1`, `gz` = `FIXED 0`, `fa` = `FIXED 13` ;
     - **`ed` = `REF`, même adresse que `pico8_kc()`, non NULL, tag `STRUCT`** ;
     - `b` reste inchangé (`fa` 10, `ed` `FIXED 1`, `c_`/`gz` absents).

     On appelle `ph` une seconde fois : le compte de `cg("mh")` est inchangé et `ed` garde la même adresse.
  4. **`kj` ouvert** ×3 → `gz` = `0x9999` (0.6), présent ; `fa`, `ed` et `c_` sont inchangés.
  5. **Écritures par nom avortées** (`must_abort`, nouvelles fonctions sur le modèle de `write_c_`) : `c_` ← `REF NULL` ; `ed` ← `false` **à union nulle** (`memset(&v, 0, sizeof v)`, `v.kind = PICO8_VALUE_BOOL`) ; `gz` ← fixe 0 **à union nulle** (`memset`, `kind = FIXED`) ; `fa` ← `FIXED 3`. Chacune doit mourir par `SIGABRT`. `cls->set_field == NULL` est réaffirmé.
  6. On plie les retours, genres, fixes, booléens, comptes et égalités d'adresses lus dans **`PJ_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ. Les adresses elles-mêmes sont jugées par `assert`, jamais pliées.

  La caractérisation est lancée verte sur l'arbre de départ **avant** le typage (ligne « Runs »), avec la ligne `:673` déjà passée par nom. Ses valeurs ne changent pas ensuite.
- **Pas de test d'arrêt rouge d'abord** : aucun `assert` nouveau à atteindre (R5 vacante). Les cas `memset` de 1b.13 sont au point 5, verts avant et après.
- **Tests existants, inchangés, qui figent déjà beaucoup** : `test_built` (`fa` 10, `ed` 1, `c_`/`gz` absents, tables par adresse) ; `test_shared` (le second panneau garde `fa`/`ed`) ; `test_life` (`ph`, `kj`, `de.my`, `ed == kc`) ; `test_replay` (`c_`, `gz`, `fa`, `ed == kc` par frame) ; `test_pj_is_a_struct` ; `test_scores_box_follows_c_` de `test_pico8_mh` (`ed` par nom) ; `test_pico8_game_end` et `test_pico8_score_panel_close` (`c_` par nom) ; `test_struct_defaults` de `test_pico8_spec_defaults`. Aucun test de régression (§6) ne lit ces membres.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_pj && ./tmp/test_pico8_pj` (et `test_pico8_mh` / `test_pico8_spec_defaults` là où c'est dit). Chacune est notée dans le commit avec le test qui la rougit, ou la justification de son trou.
  1. (§7-1) *Indicateur forcé vrai*, sur `c_` : la branche R4 ignore `has_c_`. Attendu rouge : `test_built` (`!has(ju, "c_")` dans `absent[]`), `test_shared`, caractérisation 1.
  2. (§7-2) *« Présent 0 » là où c'était absent*, sur `gz` (un fixe typé à indicateur, la lettre de la règle) : la branche R4 rend `pico8_value_fixed(gz)` sans lire `has_gz`. Attendu rouge : `test_built`, `test_shared` (`!has(other, "gz")`), `test_life` (`!has(ju, "gz")`), caractérisation 1.
  3. (§7-3) *Init qui saute un membre typé*. Pour qu'elle soit déterministe, la copie remplace aussi `malloc` par `calloc`.
     - (a) `gs = pj_default_table(PJ_GS)` omis → `gs` absent. Attendu rouge : `test_built` (`get_ref == class_ref`), `STRUCT_HASH`, caractérisation 1.
     - (b) `fa = 10` omis → `fa` lu 0. Attendu rouge : `test_built`, `test_shared`.
     - (c) `has_c_ = 0` / `has_gz = 0` / `ed = NULL` omis : **trou justifié**. `calloc` pose déjà la bonne valeur, et sous `malloc` une omission est une lecture non initialisée qu'aucun outil du filet ne signale (ASan ne détecte pas les lectures non initialisées). Les lignes sont prouvées exécutées par `gcov`.
  4. Propre à l'étape, *`ed` NULL lu comme nil* : `pico8_pj_ed` rend `REF NULL` pour `ed` NULL. Attendu rouge : `test_built`/`test_shared` (`get_fixed(…, "ed")`), `STRUCT_HASH`, caractérisation 1.
  5. Propre à l'étape, *mauvais fixe par défaut* : le littéral de `pico8_pj_ed` passe à 0. Attendu rouge : `test_built` (`ed` 1), `STRUCT_HASH` (valeur d'un défaut propre, §7).
  6. Propre à l'étape, *comparaison à `kc` sans garde* : `ed_equals_kc = pico8_kc() == self->ed`. Avec `kc` NULL et `ed` NULL, le bloc ne s'exécute plus. Attendu rouge : **caractérisation 3** (déterministe grâce à `pico8_msgbox_set_kc(NULL)`), et peut-être `test_life`/`test_replay` selon `kc` à ce moment. C'est la mutation qui justifie la garde.
  7. Propre à l'étape, *R6 qui oublie la présence* : `de.my` n'écrit pas `has_c_ = 1`. Attendu rouge : caractérisation 2 (`de.my` depuis nil lu absent). Planté sous une forme qui garde l'appel (1b.14b).
  8. Propre à l'étape, *`kj` sans indicateur* : `if (self != NULL)` au lieu de `self->has_gz`. La barre est tracée dès le premier `kj`. Attendu rouge : `test_life` (`kj` fermé ×2 avant `ph` : écrans et `LIFE_HASH`). La caractérisation ne lance `kj` qu'après `ph` et ne la rougit pas : trou de la caractérisation, couvert par `test_life`.
- **`gcov`** : `test_pico8_pj` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23, sur `pico8_score_panel.c`, `pico8_score_panel_display.c` et `pico8_menu.c`. Chaque ligne modifiée est exécutée, avec trois trous attendus :
  - la branche `self == NULL` de `pico8_pj_ed`, que le lecteur n'atteint jamais (précédent 1b.5/1b.7, gardée pour la forme de l'emballage) ;
  - les branches `gs == NULL`/`i_ == NULL` du lecteur (le constructeur `assert` que l'init a tourné) ;
  - les gardes `self != NULL` de `kj`/`ph`, qui existaient déjà.

  Le rapport va dans le commit.
- **Prior art** : `test_cr_members_by_name` (1b.6, lecteur brut et témoin), `test_too_far_members_by_name` et `write_fo_zero_false` (1b.13), `test_bg_members_by_name` (1b.15), `bool_state`, `must_abort` et `write_c_` de `test_pico8_pj`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_pj` : quelques secondes à quelques dizaines (H8). Environ 13 lancements (caractérisation sur l'ancien code, nouveau code, 10 mutations et sous-cas, `gcov`), soit 4 à 6 min ;
- `./tmp/test_pico8_mh` : un lancement après le typage ;
- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min ;
- `make class-coverage-net` : 3 à 4 min, dans la même invocation que `make e2e` (arbres distincts, comme en 1b.14a) ;
- `make sanitize-net` : environ 4 min seul, et **jamais** en même temps que `e2e` ;
- `make value-members` : quelques secondes.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent (aucun n'est touché) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 196`**, sans aucune ligne `pico8_pj_t` ;
5. `timeout 15m make class-coverage-net` : **s'applique**, parce que le diff change le corps du constructeur `pico8_pj_struct_new` (R3 remplace la boucle de recopie), sans toucher `pico8_class_coverage_record("pj")` ni `KNOWN[]`. Sortie identique à `tools/class_coverage_net_expected.txt` (`pj  19  15/15`, H10).

`make member-census` ne s'applique pas (R8). On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements. On ne lance jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

**Filet et fichiers protégés**

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.

**Représentation**

- `FAILED:` (« champ sans règle ») dans chacun de ces cas :
  - un rejeu ou un TU fait tenir à `ed` autre chose que le fixe 1 ou une REF de `mh` non NULL (H2) ;
  - `c_` ou `gz` tiennent un autre genre (H3) ;
  - un membre ou un indicateur autre que `has_c_`/`has_gz` est ajouté, `ed_mh`, `ed_fixed` et une étiquette compris ;
  - un type est élargi au lieu d'avorter.
- `FAILED:` si les types ne sont pas exactement `pico8_fixed_t fa`, `pico8_ob_table_t *gs`, `pico8_ob_table_t *i_`, `pico8_mh_t *ed`, `int c_` suivi de `int has_c_`, puis `pico8_fixed_t gz` suivi de `int has_gz`, dans cet ordre, après `pico8_ke_t ke`.
- `FAILED:` si le lecteur ne rend pas `FIXED 1` pour `ed` NULL, ne rend pas la même adresse pour `ed`, `gs` et `i_`, ne rend pas 0 avec `*out_value` intact pour `c_`/`gz` sans présence ou pour un nom inconnu, ou si `me`/`t`/`ou`/`ea`/`cr` changent de chemin.
- `FAILED:` si `pico8_pj_ed` n'est pas l'unique emballage de `ed` que le lecteur appelle, ou si la comparaison de `ph` à `kc` n'a pas la garde `self->ed != NULL`.

**R5, R7 et sites par nom**

- `FAILED:` si un `set_field` est ajouté à `pj`, si une écriture par nom d'un membre de `pj` cesse d'avorter, ou si le message de l'un des `assert` de `pico8_entity_set_field` change.
- `FAILED:` si les lignes `fa`/`ed` restent dans `g_pj_defaults[]`, si les lignes `gs`/`i_` en sortent, si `pico8_pj_gs()`/`pico8_pj_i_()` changent de signature ou de valeur, ou si l'`assert` de l'init perd la vérification de table des quatre lignes `ea`/`gs`/`i_`/`ou`.
- `FAILED:` si un site par nom change en `src` ou dans un test, sauf la ligne `test_pico8_pj.c:673`, passée par nom. `FAILED:` aussi si `mh`, `ke`, `hy`, `js` ou `fq`, leurs lecteurs ou leurs tables changent.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.

**Tests**

- `FAILED:` si le diff d'un test fait autre chose que :
  - ajouter la caractérisation, ses utilitaires, `PJ_MEMBERS_HASH` et son appel ;
  - passer `test_pico8_pj.c:673` par nom ;
  - typer les deux écritures de mise en place de `test_pico8_mh.c`.

  `FAILED:` aussi si une valeur figée existante change (`BUILT_HASH`, `SHARED_HASH`, `LIFE_HASH`, `REPLAY_HASH`, `CR_HASH`, `STRUCT_HASH`, les empreintes de `test_pico8_mh`…), ou si un test de régression (§6) change.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit ou écrit un membre directement, si elle ne porte pas les cas `memset` du point 5, ou si `PJ_MEMBERS_HASH` change après le typage.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les mutations 1 à 8 (avec les sous-cas de la 3), chacune avec son test ou la justification de son trou.

**Mesures de sortie**

- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 196, liste encore `pico8_pj_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` s'il manque une ligne « Runs » de `make class-coverage-net` identique à la référence, ou si le diff touche `pico8_class_coverage_record` ou `KNOWN[]`.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.

**Portage et journal**

- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.16 », ou le tableau « Runs ».

## Out of Scope

- `pj_cr` (1b.6, fait) ; `mh` (1b.1, fait) : `ed` pointe vers une `mh`, sans rien y changer.
- L'état de classe `ou`, `ea`, `cr`, `gs`, `i_` de `g_pj_defaults[]` : ses valeurs, ses accesseurs et son init ne changent pas, hors lignes `fa`/`ed` et colonnes `member`/`offset`.
- `pj_display_truthy` pour `hl` et `pp` ; les champs de `fm`/`hy` que `hb`/`ph` lisent par nom.
- Donner à `pj` un écrivain par nom, ou remplacer `must_abort(write_c_)`.
- Une forme de `ed` pour un autre fixe ou pour nil écrit (arrêt si ça arrive, pas d'élargissement).
- Remplacer une lecture ou écriture par nom par un accès direct ; régénérer `tools/member_census.txt`.
- Les fichiers d'événements et `FRAMES_EVENTS` ; tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `score_panel.[ch]`, `score_panel_display.c`, `menu.c`, dans un commit, sans découpage (empreinte réelle 8) ;
  - tests : `test_pico8_pj` étendu (caractérisation), deux lignes de `test_pico8_mh` typées, sans nouveau TU ;
  - `ed` → option A (`pico8_mh_t *`, NULL = fixe 1 de la spec), avec emballage public `pico8_pj_ed` et comparaison gardée ; B, C et D sont rejetées pour les raisons de la table ;
  - `c_` → `int` + `has_c_`, `gz` → `pico8_fixed_t` + `has_gz` (R1, R2), sans `<m>_is_bool` (un seul genre chacun) ;
  - R7 : `members[]` supprimée (1b.9) ; lignes `fa`/`ed` sorties de `g_pj_defaults[]` (1b.14a), lignes `gs`/`i_` gardées comme état de classe, colonnes `member`/`offset` supprimées ;
  - R5 vacante : pas de test d'arrêt rouge d'abord ; les cas `memset` de 1b.13 figés verts comme écritures avortées ;
  - `test_pico8_pj.c:673` passée par nom sur l'ancien code ;
  - mutation §7-3 rendue déterministe par `calloc` dans la copie ; `kc` rendu NULL dans la caractérisation pour rendre la mutation 6 déterministe ;
  - `class-coverage-net` lancé, puisque le constructeur change.
- **Note « Découvert à 1b.16 »**, sur la ligne 1b.16 de `docs/plan-3.md` :

  > patron 1b.1 appliqué ; **lecture nouvelle de R1** : genres mêlés {fixe, `ref:pico8_X_t`} dont le seul fixe tenu est le défaut de la spec, nil `jamais` → `pico8_X_t *<m>` sans indicateur, NULL = « le fixe de la spec » (écart assumé avec R2, comme 1b.7), un emballage public unique `pico8_X_<m>` (REF NULL pour `self` NULL, `FIXED` du défaut en littéral pour NULL, REF sinon) appelé par le lecteur ; une lecture directe qui compare l'identité garde `<m> != NULL` (sinon NULL égale une cible NULL, ici `kc`) ; un autre fixe ou nil écrit reste à fixer. **R7, lecture de 1b.14a sur une table sans fille** : la ligne d'un membre sort quand la recopie est son seul client (`fa`, `ed`, littéraux de R3), elle reste quand elle porte aussi l'état de classe que le cart lit (`pj.gs`, `pj.i_`, accesseurs publics) ; l'offset sort toujours, et la table sans ligne membre perd ses colonnes `member`/`offset`. `members[]` du lecteur supprimée (1b.9). R5 vacante, les deux moitiés (pas de `set_field`, pas de clé) ; les cas `memset` de 1b.13 figés comme écritures par nom avortées. Revue des TU lents sans effet ; empreinte réelle 8 (3, 5).

  Le reste va dans le commit.
- Le message de commit porte :
  - le périmètre et le compte 202 → 196 ;
  - le résumé du tableau « Runs » et le rapport `gcov` ;
  - les mutations et leurs trous ;
  - `PJ_MEMBERS_HASH` ;
  - les réponses à H1 à H13.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
