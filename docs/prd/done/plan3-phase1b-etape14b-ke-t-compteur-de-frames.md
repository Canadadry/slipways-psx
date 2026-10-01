---
title: "Plan 3, étape 1b.14b : `ke.t` (le compteur de frames de l'état courant) — `t` devient un `pico8_fixed_t` sans indicateur, et sa clé de construction quitte la table `keys[]` de `g_`"
description: "`pico8_ke_t.t` est le dernier `pico8_value_t` de `ke` (`fixe`, nil `jamais`, lu par nom) que 24 genres d'entité enchâssent. Le typer `pico8_fixed_t` selon le patron R1–R9 de 1b.1 vide `pico8_ke_t` et fait passer `make value-members` de 208 à 207. Aucune table de défauts de fille ne liste `t`. Sa seule entrée d'offsets est la clé de construction `{\"t\", offsetof(pico8_g__t, ke.t)}` de `g_`, qui devient une branche R5 de boucle de clés, avec une aide de forme 1b.13."
status: needs-triage
---

## Problem Statement

Lignes de l'étape, recopiées de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.14 — `ke` (`ke.h:12`), empreinte 42 (19, 23), découpée :
>
> 1b.14b — `ke.t`. Empreinte 32 (13, 19). Compte : 207.

1b.14b ne type que `ke.t`. `ke.me` est l'étape 1b.14a.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité ici, pas redécidé. Les notes « Découvert à » de `docs/plan-3.md` qui suivent s'appliquent aussi. Elles sont citées, pas redérivées :

- **1b.1** : R1 à R9. `fixe` → `pico8_fixed_t`. R2 : pas d'indicateur pour un non-pointeur dont le nil est `jamais`. R4 : un non-pointeur sans indicateur est toujours présent ;
- **1b.3** : R3 se lit sur ce qui initialise l'objet ;
- **1b.4b** : la table de recensement est corrigée (fils forkés). La ligne `ke.t` y reste `fixe`, `jamais` ;
- **1b.5** : un emballage public unique par membre n'existe que pour les genres mêlés, donc **pas ici**. La branche explicite d'une boucle de clés se place avant la recherche générique. L'`else` final garde son `assert` et son message **tels quels** ;
- **1b.6** : un lecteur qui copie n'est pas un `pico8_X_member` ;
- **1b.7** : un nil `stat?` venu d'un homonyme ne décide rien. Ici, sans objet : la ligne n'a pas de preuve de nil ;
- **1b.8** : R7 s'applique à un sélecteur `pico8_value_t *` local au lecteur. **C'est le sélecteur `member` de `pico8_ke_field`, que 1b.14a a laissé pour `t`** ;
- **1b.9 et 1b.10**, pour **R7 forme `offsetof`** et la **boucle de clés directe** : l'entrée du membre typé sort de la table `keys[]`. La clé devient une branche (forme 1b.5) qui appelle une aide `static` par genre. Un membre sans indicateur qui reçoit nil avorte (précédent : `lq.w`) ;
- **1b.11**, pour **R2** : un TU de `UNIT_TESTS_SLOW` n'est pas recensé. Chaque PRD relit donc ceux qui touchent sa struct, pour y chercher des lectures nil par nom (revue plus bas) ;
- **1b.12** : source différente, même déballage R5. Ce n'est pas une lecture nouvelle ;
- **1b.13** : **forme écrivain de R5**. L'aide est `static`, une par genre, au nom de la struct, dans le fichier de la boucle ou de l'écrivain, jamais générique ni partagée entre fichiers. Elle est partagée entre l'écrivain et la boucle de clés d'une même struct. **Un non-pointeur sans indicateur n'a pas d'état nil : son aide commence par l'`assert`, et nil y avorte.** Le test d'arrêt de **chaque** étape porte le cas **non-REF à union nulle** (ici `false`, et la chaîne NULL). Un `must_abort` écrit rouge d'abord fige l'arrêt ;
- **1b.14a** (PRD `docs/prd/triage/plan3-phase1b-etape14a-ke-me-etat-courant.md`, **précondition : commitée**) : **R7, lecture nouvelle pour une table de défauts qui garde des entrées et a des filles**. L'entrée du membre typé sort de toutes les tables qui la listent, quel que soit leur format, et la table reste avec ses autres entrées. Chaque module **fille** qui avait l'entrée déballe **une fois, à son init**, le défaut de sa spec dans un `static <type> g_<X>_<m>`, le remet à nil dans son `defaults_reset` et le pose par affectation typée dans son `struct_init`. Les filles sans défaut propre héritent par la chaîne d'init, et leurs tables ne sont pas touchées. La caractérisation et le test d'arrêt de `me` vivent dans `test_pico8_ui` (`test_ke_me_by_name`, `test_ke_ks_stops_off_row`, et l'aide de lecture double `me_of`, qui prend le nom).

### État de départ

HEAD `e4cf23b` (PRD de 1b.14a). 1b.14a est **en cours dans l'arbre de travail** : `src/pico8_ke.c`, `pico8_ke.h`, `pico8_planet.c`, `pico8_planet_display.c`, `pico8_slipway.c`, `pico8_wormhole.c`, `pico8_wormhole_display.c`, `tests/test_pico8_iv.c` et `test_pico8_ui.c` sont modifiés. **Précondition** : 1b.14a est commitée avant que 1b.14b commence. Elle touche cinq des fichiers de cette étape (`ke.h`, `ke.c`, `planet_display.c`, `wormhole.c`, `wormhole_display.c`) et `test_pico8_iv.c`, mais pas les lignes `t`, sauf le sélecteur de `pico8_ke_field`, qu'elle ne garde que pour `t`. Chaque site est désigné **par son contenu**. Les numéros cités sont ceux de HEAD et glisseront peut-être (H9). `make value-members` rend alors 208 (H1).

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code (HEAD et arbre de 1b.14a). Tout fait d'exécution est une hypothèse (H1 à H12).

### Ligne de recensement (`tools/member_census.txt:42`)

| struct | membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|---|
| `pico8_ke_t` | `t` | offset | `fixe` | jamais | - | l |

Un membre : 208 − 1 = **207**. Après l'étape, `pico8_ke_t` n'a plus aucun `pico8_value_t`.

- **`fixe` seul**, sur 15 rejeux et 47 TU recensés. La ligne couvre les 24 genres qui enchâssent `ke`, parce que le recensement rattache les membres d'une parente enchâssée à la parente.
- **Nil `jamais`, sans preuve.** Aucun lecteur par nom n'a lu `t` nil, aucun écrivain par nom ne l'a écrit nil, et la moitié statique ne trouve rien. Le cart le confirme : `dg` fait `qc.t+=1` sur chaque entité de `hv` (`lua.lua:179`), ce qui lèverait une erreur Lua sur un `t` nil. `ke` pose `t=0` (`:183`), et `ks` le remet à 0 (`:185`).
- **`l` seul, pas de `e`** : aucun `set_field` ne connaît `"t"` (`grep -rnE 'set_field\([^,]+, *"t"' src tests` est vide).

**Revue des TU lents (règle R2 de 1b.11).** Relus : `test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`. Tous lisent `t` par nom, et il y est toujours présent :

- `fold_field(…, "t")`, qui plie aussi la **présence**, dans les listes `scalars` de `jo` (`:287`, `:336`), `qh`, `os`, `hj`, `pm`, `mx` et `nr`, et sur les planètes posées (`placed`, `poor_lab`, `in_lab`) ;
- `fold_value(h, get(icon, "t"))` (`ie:242`) et `must_fixed(icon, "t") == 0` (`ie:346`).

**Aucune lecture nil de `t` par nom** (H4). R2 ne donne donc pas de `has_t`.

### Ce que fait le cart (`pico8/lua.lua`)

- `ke=kv:jr([[ me="ph",t=0 ]])` (`:182-183`), et `function ke:ks(me)self.me,self.t=me,0` (`:185`).
- `dg()` (`:173-180`) : `fn(qc,qc.t)`, puis `qc.t+=1` sur chaque entité de `hv`. C'est l'**écrivain générique** que la note 1a.16b range parmi les « écrivains génériques sur `cg.jo` » (« `t` (`bootstrap.c:101`, membre de `ke`) ») : il écrit sans filtre de classe, ce qui a mis `t` dans `ke`.
- Lectures : `ma()` (`:350`, `e.t*0.2`), `jo:kj` (`:543`, `self.me=="ll" and rnd(30)<self.t-30`), `mh:ev` (`:588-590`, lit **et écrit** `self.t=self.rh*10`), `jc:lg` (`:662`), `g_:mm` (`:727`), `mz:ey` (`:775`), `mz:kj` (`:789-790`).
- **Aucune spec de fille ne redéfinit `t`**. `grep -rnE '"[^"]*(\\n|,)t=' src/*.c` ne rend que la spec de `ke`.

### Tous les sites de `ke.t`

**Lecteurs par nom (inchangés, puce Coexistence)** :

- `bootstrap.c` `pico8_dg` : `get_field(qc, "t", &t_arg)` (nil si absent) pour l'argument de la méthode d'état, puis `get_field(qc, "t", &t_val) && t_val.kind == FIXED` avant l'écriture ;
- `probe.c` `pico8_g__mm` : `pico8_probe_field_fixed(self, "t")` ;
- `entity_geom.c` `ma()` : `pico8_entity_geom_field_fixed(e, "t")` ;
- les TU (voir « Empreinte »).

**Lecteur du descripteur** : `pico8_ke_field`. Après 1b.14a, il a une branche R4 pour `me`, puis le sélecteur local `const pico8_value_t *member` (`t` seulement) : `REF NULL` → 0, sinon copie. **C'est le seul lecteur par nom de `t` pour les 24 genres** : chaque lecteur de fille retombe sur lui, et aucune fille ne connaît `"t"`.

**Écritures directes (R6)** :

- `ke.c` `pico8_ke_struct_init` : `self->t = pico8_value_fixed(pico8_fixed_from_int(0))` ;
- `ke.c` `pico8_ke_ks` : `((pico8_ke_t *)self)->t = pico8_value_fixed(pico8_fixed_from_int(0))` ;
- `bootstrap.c` `pico8_dg` : `((pico8_ke_t *)qc)->t = pico8_value_fixed(pico8_fixed_add(t_val.as.fixed, pico8_fixed_from_int(1)))`. **C'est l'écrivain générique de 1a** : il lit par nom et écrit directement. Ce n'est pas un `set_field` ;
- `msgbox.c` `pico8_mh_ev` : `self->ke.t = pico8_value_fixed(pico8_fixed_mul(rh, pico8_fixed_from_int(10)))`.

**Lectures directes (R6, que le compilateur désigne)** :

- `msgbox.c` `pico8_mh_ev` : `self_t = pico8_mh_value_fixed(self->ke.t)`. `pico8_mh_value_fixed` (`static`) **n'a pas d'autre appelant** et tombe avec cette lecture (`-Werror` refuse une fonction `static` inutilisée) ;
- `planet_display.c` `pico8_jo_kj` : `fixed_of(self->ke.t)`. `fixed_of` garde deux appelants (`mi`) ;
- `wormhole.c` `pico8_mz_ey` : `if (self->ke.t.kind == PICO8_VALUE_FIXED) t = self->ke.t.as.fixed;` ;
- `wormhole_display.c` `pico8_mz_kj` : `pico8_mzd_fixed(self->ke.t)`. `pico8_mzd_fixed` garde l'appelant `oi` ;
- `slipway_display.c` `pico8_jc_lg` : `pico8_jc_display_value_fixed(self->ke.t)`. L'aide garde l'appelant `nh` ;
- `tests/test_pico8_iv.c` : `test_dg_drives_a_ke_struct` (`:246`, `:252`) et `test_defaults_match_the_parsed_spec` (`:272`).

**Tables d'offsets et de défauts qui listent `t`** :

- `g_g__defaults[]`, `g_g__members[]` (`probe.c`), `g_jo_defaults` (`planet.c`), `g_qh_defaults` et `g_pm_defaults` (`planet_family.c`), `g_fv_defaults` (`help.c`) et toutes les autres tables `members[]`/`defaults[]` des filles : **aucune n'a d'entrée `t`**. La preuve : `grep -rnE 'offsetof\([a-z0-9_]+, ([a-z0-9_]+\.)*t\)' src tests` et `grep -rnE '\{"t",' src tests` ne rendent qu'**une** ligne ;
- cette ligne est `probe.c`, `pico8_g__struct_new`, table `keys[]` des **clés de construction** de `g_` : `{"t", offsetof(pico8_g__t, ke.t)}`, recopiée par la boucle générique `*(pico8_value_t *)((char *)self + keys[found].offset) = overrides->entries[k].value`, avec l'`assert` « the only measured construction keys are pn, t, du, py ». Seuls des TU passent cette clé, toujours en fixe : `test_pico8_probe_mm_rnd_order` (`new_probe`), et `test_pico8_g_` (`keyed`, `t` = 3). Le jeu ne construit `g_` que par `pico8_g__new(NULL)` (`bg.c`), et `fv` refuse toute clé.

### Les trois décisions demandées

1. **Type C** : `fixe` seul → **`pico8_fixed_t t;`** (R1), à sa place, après `me`.
2. **`has_t` : non** (R2). C'est un non-pointeur lu par nom, mais son nil est `jamais`, sans preuve, et la revue des TU lents n'en trouve aucun. Le cart rend nil impossible (`qc.t+=1`). `t` sera **toujours présent** (R4).
3. **Écrivain par nom de `t` sur `ke` : aucun.** `ke` n'a pas de `set_field` (c'est une parente abstraite), aucun écrivain de fille ne connaît `"t"`, et la ligne n'a pas de `e`. Écrire `"t"` par nom reste l'arrêt « clé inconnue » de `pico8_entity_set_field`, sans changement. L'écrivain générique de 1a (`dg`) est une écriture **directe** (R6). **La forme 1b.13 s'applique à la seule moitié écrivain de R5 qui existe : la clé de construction `t` de `g_`.** Elle devient une branche de boucle de clés (1b.10) qui appelle une aide `static` de forme 1b.13, pour un non-pointeur sans indicateur : l'`assert` d'abord, et nil avorte.

### R7 selon 1b.14a : ce qu'elle donne ici

- **Tables de défauts des filles** : aucune ne liste `t`. La lecture 1b.14a (sortir l'entrée, puis déballer le défaut de la spec de chaque fille dans un `static`) est donc **vacante** : aucune fille n'a de défaut propre, et aucun `g_<X>_t` n'est créé. Les 24 genres héritent de `t` = 0 par la chaîne d'init (`pico8_ke_struct_init`).
- **Défaut de `ke` lui-même** : `ke` n'a pas de table de défauts. Son `struct_init` pose un **littéral** `0` sans lire sa spec, comme en 1a. Pour un fixe, l'identité ne compte pas : R3 donne `self->t = pico8_fixed_from_int(0)`. (G) On n'ajoute pas de `g_ke_t` déballé de la spec. Ce serait ajouter une lecture qui n'existe pas en 1a, sans rien protéger.
- **Table `keys[]` de `g_`** : l'entrée `t` sort (R7 couvre `keys[]`), et les entrées `pn`, `du`, `py` restent, puisqu'elles seront typées à 1b.32. La boucle reçoit une branche R5 (section suivante).
- **Sélecteur local de `pico8_ke_field`** (forme 1b.8) : retiré. Il est remplacé par une branche R4 `t`.

### Empreinte remesurée ; découpage

La ligne du plan dit 32 (13, 19). C'est un majorant : `t` est un nom d'une lettre, partagé avec des locales, avec `lw.t`/`gf.t` (proposition d'échange) et avec les listes d'absents de `mo`, `je`, `jc_too_far`, `jc_mouse_point` et `mz.jj`.

**Empreinte réelle** (fichiers qui lisent ou écrivent `ke.t` d'une struct) : **36 (10, 26)**.

- **src (10)** : `ke.h`, `ke.c`, `bootstrap.c`, `probe.c`, `entity_geom.c`, `planet_display.c`, `wormhole.c`, `wormhole_display.c`, `slipway_display.c`, `msgbox.c`.
- **tests (26)**, tous par nom sauf `iv` : `bg`, `fq`, `fr`, `fv`, `g_`, `ga`, `hj`, `ie`, `iv`, `jc`, `jo`, `jo_spec_defaults`, `js`, `mh`, `mx`, `mz`, `nl`, `nr`, `os`, `pj`, `pm`, `probe_mm_rnd_order`, `qb`, `qh`, `spec_defaults`, `ui`.

**Fichiers modifiés : 11**, plus `docs/plan-3.md` :

- **src (9)** : `pico8_ke.h`, `pico8_ke.c`, `pico8_bootstrap.c`, `pico8_probe.c`, `pico8_msgbox.c`, `pico8_planet_display.c`, `pico8_wormhole.c`, `pico8_wormhole_display.c`, `pico8_slipway_display.c` ;
- **tests (2)** : `test_pico8_ui.c` (ajout pur : caractérisation et test d'arrêt) et `test_pico8_iv.c` (trois lignes de lecture directe, que le compilateur impose).

`entity_geom.c` et les 25 autres TU lisent par nom et restent intacts.

**Pas de découpage** (G) : un membre, un genre, pas d'indicateur, et 9 fichiers `src` qui changent chacun de 1 à 15 lignes. C'est moins que 1b.14a. Une coupe laisserait de toute façon un arbre incompilable, puisque le type change pour tous les sites à la fois.

**TU de `UNIT_TESTS_SLOW` : aucun modifié.** Les huit (`test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`) exercent la nouvelle branche R4 par `fold_field(…, "t")` ou `get`/`must_fixed`. `test_pico8_jo` (`test_ll`) exerce en plus `ks` et la branche `"ll"` de `jo:kj`, qui lit `t` directement. `make e2e` les passe tous.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (l'arbre de 1b.14a commitée). On lit `t` par nom sur les 24 genres de filles, sur toutes les entités vivantes d'une partie avant et après un `dg`, et après les écritures (`g_` à clé, `ks`).
2. **Test d'arrêt** de la clé de construction `t` hors genre (nil, `false`, `true`, chaîne, chaîne NULL, REF), écrit rouge sur l'ancien code, qui recopie tout, et vert après.
3. **Typage** : `pico8_fixed_t t;` dans `pico8_ke_t`, à la même place et sous le même nom.
4. **Init et méthodes** (R3, R6) : `ke` pose `0`, `ks` pose `0`, `dg` pose `t_val.as.fixed + 1`, et `mh:ev` pose `rh*10`, tous par affectation typée. Les lectures directes deviennent `x->ke.t`.
5. **Lecteur** : une branche R4 `t` dans `pico8_ke_field`, et le sélecteur local disparaît. **Clé de construction** : une branche R5 dans la boucle de `g_`, avec l'aide `static` `g__take_fixed`.
6. Effet mesurable : `make value-members` passe de 208 à **207**, et il ne reste aucune ligne `pico8_ke_t`. Aucun pixel ne bouge.
7. **Note « Découvert à 1b.14b »** (voir Further Notes).

## User Stories

1. En tant que lecteur de `ke.h`, je veux `t` typé `pico8_fixed_t`, afin que le compteur de frames de l'état courant se lise sur le type, et que `pico8_ke_t` ne contienne plus de `pico8_value_t`.
2. En tant que `dg` (`bootstrap.c`), je veux lire `t` par nom sur toute entité de `hv` (présent, fixe, même valeur), afin de passer le même argument à la méthode d'état et d'écrire `t+1` comme en 1a.
3. En tant que lecteur par nom de n'importe laquelle des 24 filles, je veux que `t` passe par le lecteur de `ke` et rende exactement ce que rendait 1a : toujours présent, genre `FIXED`, même valeur.
4. En tant que `g_:mm` (`probe.c`) et `ma()` (`entity_geom.c`), je veux que mes lectures par nom de `t` ne changent pas.
5. En tant que `mh:ev`, `jo:kj`, `mz:ey`, `mz:kj` et `jc:lg`, je veux lire `self->ke.t` directement, avec la même valeur que l'ancienne conversion (`FIXED` → valeur, sinon 0 : jamais le « sinon »).
6. En tant que `mh:ev`, je veux écrire `t = rh*10` par affectation typée.
7. En tant que `ks`, je veux remettre `t` à 0 par affectation typée, avec ma signature et mon traitement de `me` (1b.14a) inchangés.
8. En tant que `test_pico8_probe_mm_rnd_order` et `test_pico8_g_` (`keyed`), je veux construire un `g_` avec la clé `t` fixe et relire la même valeur par nom.
9. En tant que mainteneur, je veux qu'une clé de construction `t` qui n'est pas un fixe (nil, `false`, `true`, une chaîne, une chaîne NULL, une REF) arrête l'exécution par un `assert` « champ sans règle » qui nomme `g_.ke.t` et `fixe`.
10. En tant que mainteneur, je veux qu'aucune table `keys[]`, `members[]` ou `defaults[]` ne liste plus `t`, et qu'aucune écriture `*(pico8_value_t *)` n'atteigne `t`.
11. En tant que mainteneur, je veux que le sélecteur `pico8_value_t *` de `pico8_ke_field` disparaisse, afin que `pico8_ke.c` ne manipule plus aucun membre emballé.
12. En tant que mainteneur, je veux qu'écrire `"t"` par nom reste l'arrêt « clé inconnue » de `pico8_entity_set_field`, sans branche d'écrivain nouvelle.
13. En tant qu'auteur de 1b.23 (`jo`) et de 1b.32 (`g_`), je veux savoir que la lecture R7 de 1b.14a est vacante quand aucune fille n'a de défaut propre, et que la table `keys[]` de `g_` a déjà une branche explicite et une aide `g__take_fixed` que 1b.32 étendra à `pn`, `du` et `py`.
14. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
15. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
16. En tant que gardien de la couverture, je veux `make class-coverage-net` identique à sa référence, puisque deux constructeurs changent (`pico8_ke_struct_init`, `pico8_g__struct_new`).
17. En tant que `plan-pilot`, je veux `make value-members` à 207, sans ligne `pico8_ke_t`.
18. En tant que vérificateur, je veux une caractérisation par nom sur toutes les filles, lancée sur l'ancien code, les mutations du §7 avec leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Il n'y a pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés comme le ferait un agent `guideline` (hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables), et marqués (G).

### R1–R9 appliquées à `ke.t`

| règle | sur `ke.t` |
|---|---|
| R1 | `fixe` → `pico8_fixed_t t`, déclaré à la place de l'ancien membre, après `me`. |
| R2 | **Aucun indicateur** : nil `jamais`, sans preuve, et les TU lents n'en lisent pas. Le cart l'exclut (`qc.t+=1`). |
| R3 | `pico8_ke_struct_init` : `self->t = pico8_fixed_from_int(0)`, à la place de l'ancienne affectation, dans le même ordre (après `me`). Aucune fille n'écrase `t` à l'init. Pas de défaut déballé de la spec (voir « R7 selon 1b.14a »). |
| R4 | Dans `pico8_ke_field`, une branche `strcmp(name, "t")` **après** celle de `me` : `*out_value = pico8_value_fixed(ke->t); return 1;`. Pas de cas absent : c'est un non-pointeur sans indicateur. Le nom inconnu donne `return 0`, `*out_value` intact. Le sélecteur `const pico8_value_t *member` et son test `REF NULL` disparaissent (1b.8). |
| R5 | Pas d'écrivain par nom. Une branche de boucle de clés dans `pico8_g__struct_new`, avec l'aide de forme 1b.13 (section suivante). |
| R6 | `ks` : `->t = pico8_fixed_from_int(0)`. `dg` : `->t = pico8_fixed_add(t_val.as.fixed, pico8_fixed_from_int(1))`, avec la lecture par nom et son test `kind == FIXED` **gardés**. `mh:ev` : `self->ke.t = pico8_fixed_mul(rh, …)`. Lectures : `self_t = self->ke.t` (`pico8_mh_value_fixed` supprimée, sans autre appelant), `t = self->ke.t` dans `jo:kj`, `mz:kj` et `jc:lg`. Dans `mz:ey`, la garde `kind == FIXED` et l'initialisation de la locale à 0 disparaissent : `t = self->ke.t`. `test_pico8_iv` : trois lectures directes (voir Testing Decisions). |
| R7 | Retrait de `{"t", offsetof(pico8_g__t, ke.t)}` de `keys[]`. Retrait du sélecteur du lecteur de `ke`. Aucune autre table ne liste `t`. |
| R8 | Tous les `PICO8_MEMBER_CENSUS_SEE`, `READ` et `WRITE` sont gardés. `tools/member_census.txt` n'est pas édité. |
| R9 | 208 → 207, et il ne reste aucune ligne `pico8_ke_t`. |

### R5 : la clé de construction `t` de `g_` (G)

- **Branche**, forme 1b.5/1b.10, placée **avant** la recherche générique dans `keys[]`, dans la boucle sur `overrides` : `if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "t") == 0) { g__take_fixed(overrides->entries[k].value, &self->ke.t); continue; }`. La recherche générique et son `assert` restent pour `pn`, `du` et `py`, avec leur **message inchangé** (qui nomme encore `t` parmi les clés mesurées, et c'est toujours vrai).
- **Aide** `static void g__take_fixed(pico8_value_t value, pico8_fixed_t *out)`, dans `pico8_probe.c`, forme par valeur de 1b.13, cas du **non-pointeur sans indicateur** :
  1. **pas d'état nil** : elle commence par l'`assert` ;
  2. `assert(value.kind == PICO8_VALUE_FIXED && "g_.ke.t: a construction key that is not a fixed is off its census row (fixe, never nil) -- a field under no rule of plan-3 section 5: stop, do not widen the member")`. Nil (`REF NULL`) avorte, tout comme `false` (non-REF à union nulle), `true`, une chaîne (NULL ou non) et une REF ;
  3. `*out = value.as.fixed`.
- **Nom** : `<X>_take_<genre>`, au nom de la struct de la boucle (`g_`), et non de `ke`, parce que l'aide vit dans le fichier de la boucle (1b.13, « jamais entre fichiers »). 1b.32 la réutilisera pour `pn`, `du` et `py` en élargissant la liste de membres de son message (précédent : `hp.fj/o/dm/ii/rn`, 1b.9).
- **Écart avec 1a, voulu par la règle** : en 1a, une clé `t` à nil était recopiée et donnait `t` absent. Aucun site ne le fait : seuls des TU passent `t`, toujours fixe (H2). C'est l'arrêt « champ sans règle » de 1b.13 pour un membre sans indicateur. Le test d'arrêt le fige.

### Ce qui ne change pas

- `pico8_ke_ks` garde sa signature `(pico8_struct_header_t *, pico8_value_t me)` et son aide `ke_take_string` (1b.14a). Seule la ligne `t` change.
- Les lecteurs par nom (`dg`, `g_:mm`, `ma()`) et les lecteurs des filles gardent leur forme. La branche « `t` absent → nil » de `dg` devient inatteignable pour un `ke`, mais elle est gardée : c'est un site par nom (Coexistence).
- `pico8_ke_default` et la spec de `ke` (`t=0`) : phase D.
- Les autres entrées de `keys[]`, `g_g__defaults[]` et `g_g__members[]` (1b.32).

### Preuves

Toutes rendent la sortie indiquée après l'étape :

- `grep -nE 'pico8_value_t [a-z_]+;' src/pico8_ke.h` : vide. `grep -n 'pico8_fixed_t t;' src/pico8_ke.h` : une ligne ;
- `grep -rnE 'offsetof\([a-z0-9_]+, ([a-z0-9_]+\.)*t\)' src/ tests/` : vide (1 ligne avant) ;
- `grep -rnE '\{"t",' src/ tests/` : vide (1 ligne avant) ;
- `grep -rnE '(ke|ke_base)(\.|->)t\.(kind|as)' src/ tests/` : vide ;
- `grep -rnE -- '->t = pico8_value_fixed|ke\.t = pico8_value_fixed|t = pico8_value_fixed\(pico8_fixed_from_int\(0\)\)' src/` : vide ;
- `grep -n 'pico8_value_t \*' src/pico8_ke.c | grep -v 'pico8_value_t \*out_value)'` : vide (le sélecteur `member` est parti) ;
- `grep -rnw 'pico8_mh_value_fixed' src/` : vide ;
- `grep -c 'off its census row' src/pico8_probe.c` : **un de plus** qu'avant (l'aide `g__take_fixed`) ;
- `grep -rn '"t"' src/ tests/ | grep -v '^src/pico8_\(ke\|probe\)\.c' | grep -v '^tests/test_pico8_ui\.c'` : **mêmes lignes qu'avant** (les sites par nom sont intacts) ;
- `grep -rn 'pico8_ke_ks(' src/ tests/` : mêmes lignes d'appel qu'avant.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 208 sur l'arbre de départ (1b.14a commitée), avec la seule ligne `pico8_ke_t t`, puis 207, sans ligne `pico8_ke_t`. Un typedef vidé de ses `pico8_value_t` (comme `mh`, `jb`) ne casse ni le script ni la reconnaissance des filles qui l'enchâssent.
- **H2** : dans les 15 rejeux et dans tous les TU hors fils de `must_abort`, la clé de construction `t` de `g_` n'est passée que par `test_pico8_probe_mm_rnd_order` et `test_pico8_g_`, toujours en fixe. Le jeu ne construit `g_` qu'avec `NULL` (`bg.c`). Sinon l'aide avorte : c'est l'arrêt « champ sans règle ».
- **H3** : les 24 constructeurs `pico8_<X>_new(NULL)` réussissent dans un fils de `test_pico8_ui` après `start_game(DRAG)` (précédent : la caractérisation de 1b.14a), et chacun rend `t` = 0 par nom.
- **H4** : aucun TU de `UNIT_TESTS_SLOW` ne lit `ke.t` nil par nom, et `t` n'est absent dans aucun rejeu.
- **H5** : aucune valeur figée ne bouge : `STRUCT_HASH` de `test_pico8_jo_spec_defaults` et de `test_pico8_spec_defaults`, les hash `fold_field` des TU de filles, `LL_DG_CALLS`, `DRAG_REPLAY_HASH`, `TECH_REPLAY_HASH`, `KE_ME_HASH` (1b.14a), ni les 15 empreintes du filet.
- **H6** : le build `-DPICO8_MEMBER_CENSUS` compile, et la table générée perd d'elle-même la ligne `t`. Il n'est pas lancé.
- **H7** : sous ASan+UBSan, rien de nouveau : aucune allocation, et `t` est toujours écrit avant d'être lu (`pico8_ke_struct_init` est appelée par les 14 structs qui enchâssent `ke`, et par le faux toast de `test_pico8_iv`).
- **H8** : `test_pico8_ui` reste sous 30 s avec deux fonctions de plus (hors `UNIT_TESTS_SLOW`).
- **H9** : 1b.14a, une fois commitée, ne touche aucune ligne `t` sauf le sélecteur de `pico8_ke_field`. Les numéros de ligne cités glissent peut-être : chaque site est désigné par son contenu.
- **H10** : aucun code ne dépend de `sizeof(pico8_ke_t)` ni de la disposition des filles (sur une cible 64 bits, `pico8_fixed_t` est plus petit que `pico8_value_t`). Aucun binaire ne sérialise une entité.
- **H11** : dans `dg`, un `pico8_dg()` de plus dans un fils, juste après `start_game(DRAG)`, est déterministe (c'est un `_update60` partiel) et ne libère aucune entité que la caractérisation relirait ensuite. Elle ne relit `hv` qu'après l'appel.
- **H12** : les lignes modifiées de `jc:lg` (`slipway_display.c`) et de `mh:ev` (`msgbox.c`) sont exécutées par `test_pico8_ui` (rejeux `DRAG`/`TECH`). `mz:ey` et `mz:kj` le sont par `test_pico8_mz`, et `mh:ev` au besoin par `test_pico8_mh`. La branche `"ll"` de `jo:kj` n'est peut-être atteinte que par `test_pico8_jo` (`test_ll`, lent). Voir `gcov`.

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom, ou l'écran.** Le nouveau test n'accède jamais directement à `->ke.t` : il compile sur les deux formes. Seule exception, les trois lignes existantes de `test_pico8_iv`, que le compilateur désigne. `f->ke.t.kind == FIXED && f->ke.t.as.fixed == X` devient `f->ke.t == X` (`:246`, `:252`, et `:272` pour `e->ke.t == 0`), sans changer ce qu'elles affirment. (G) Elles restent des lectures directes : c'est le test de `dg` sur un faux toast et le test des défauts « bit pour bit », et les faire passer par nom changerait leur objet.
- **Caractérisation dirigée, figée sur l'ancien code** : `test_ke_t_by_name`, dans `test_pico8_ui` (rapide, dans `CENSUS_UNIT_TESTS`, lie `ENGINE_SRCS`, a déjà `start_game`, `must_abort` et l'aide de lecture double de 1b.14a). Elle est lancée par `in_fresh_child` juste après `test_ke_ks_stops_off_row`. Chaque lecture passe par l'aide de 1b.14a (`me_of`, qui prend le nom : lecteur brut du descripteur et `pico8_entity_get_field`, témoin fixe `0x1234`, qui doivent concorder sur le retour, le genre et la valeur). Si l'aide a été renommée, c'est celle de la caractérisation de `me`.
  1. **Entités vivantes, et `dg`.** `start_game(DRAG)`, puis on parcourt `pico8_hv()` : pour chaque entrée, `t` est présent et `FIXED`, et on plie sa valeur, plus le nombre d'entrées. Puis **un** `pico8_dg()`, et on parcourt `hv` de nouveau, avec le même pli. Aucun pointeur du premier parcours n'est relu (H11).
  2. **Toutes les filles.** Ensuite, `pico8_<X>_new(NULL)` dans l'ordre de 1b.14a : `bg`, `nf`, `mh`, `iv`, `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`, `g_`, `fv`, `ie`, `pj`, `qb`, `jc`, `ui`, `js`, `fq`, `mz`. Pour chacune, `t` est présent, `FIXED`, et `assert` explicite `== 0`. Si un constructeur donne une autre valeur sur l'ancien code, c'est la valeur **observée sur l'ancien code** qui est figée, et l'écart est noté dans le commit (H3).
  3. **Intrus** sur le lecteur de `ke` : `pico8_ke_field(bg, n, …)` pour `n` ∈ {`""`, `"T"`, `"tt"`, `"t "`, `"has_t"`, `"ke"`} → 0, témoin intact.
  4. **Écritures.**
     - Un `g_` construit avec les clés `pn` = 7, `t` = 3, `du` = 10, `py` = 1 (la forme de `keyed`) : `t` lu 3, et `pn`, `du`, `py` inchangés par nom.
     - Un `g_` avec la seule clé `t` = 0 : `t` présent 0.
     - `pico8_ke_ks(g_, pico8_value_string("ll"))` sur le premier `g_` : `t` lu 0, `me` lu `"ll"`.
     - `pico8_ke_ks` sur le `pm` de 2 : `t` lu 0 par le lecteur de `pm`, et égal (`pico8_value_equal`) à la lecture par le lecteur de `hj`.
  5. On plie les retours, les genres et les valeurs dans **`KE_T_HASH`**, relevé au premier passage vert sur l'arbre de départ.

  Elle est lancée verte sur l'arbre de départ **avant** le typage (ligne « Runs »). Ses valeurs ne changent pas ensuite.
- **Test d'arrêt, rouge d'abord** (précédent 1b.13) : `test_g__t_key_stops_off_row`, lancé par `in_fresh_child` après la caractérisation. Il appelle `must_abort` (celui du fichier) sur une construction `pico8_g__new(b)` dont la seule clé est `t` = `v`, avec une enveloppe qui reçoit un `const pico8_value_t *`. Les valeurs de `v` sont :
  - `REF NULL` (nil : pas d'indicateur, donc arrêt) ;
  - **`pico8_value_bool(0)`**, le cas **non-REF à union nulle** exigé par 1b.13 ;
  - `pico8_value_bool(1)` ;
  - `pico8_value_string("3")` ;
  - **`pico8_value_string(NULL)`**, second cas à union nulle ;
  - une REF vers un `bg` neuf.

  Chaque cas est **rouge sur l'ancien code**, qui recopie tout (le fils sort par `_exit(0)`), et vert après. Il faut une ligne « Runs » pour chacun des deux passages. Sur l'ancien code, la caractérisation passe **avant** lui dans le même binaire et imprime son hash. Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Tests existants, inchangés, qui figent déjà beaucoup** :
  - `test_pico8_iv` (`test_dg_drives_a_ke_struct` : `t` passé à la méthode d'état et incrémenté par `dg`) ;
  - `test_pico8_jo_spec_defaults` et `test_pico8_spec_defaults` (`STRUCT_HASH` : `t` par nom sur 23 genres) ;
  - `test_pico8_g_` (`keyed` : `t` = 3 par clé) ;
  - `test_pico8_probe_mm_rnd_order` (régression §6 : `t` par clé, lu par `g_:mm`) ;
  - `must_fixed(…, "t")` de `bg`, `jc`, `ie`, `mz` (dont `mz:515`, après `ks`) et `qb` ;
  - les `fold_field(…, "t")` de `mh`, `fq`, `pj`, `fv`, `js`, `ga`, `nl` et des huit TU lents.

  Aucun test de régression (§6) ne lit `t` directement.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_ui && ./tmp/test_pico8_ui`, plus le TU cité quand il rougit plus vite. Chacune est notée dans le commit avec le test qui la rougit, ou avec la justification de son trou.
  1. (§7-1) *indicateur forcé vrai* : **sans objet dans sa lettre**, puisque `t` n'a pas d'indicateur et que son nil est `jamais`. Forme la plus proche, l'inverse d'un indicateur : la branche R4 rend 0 (absent) quand `t == 0`, soit une sentinelle. Attendu rouge : caractérisation 2 (24 genres présents à 0), `test_pico8_bg` (`must_fixed(bg, "t") == 0`), `test_pico8_spec_defaults`.
  2. (§7-2) *« présent 0 » là où c'était absent* : **sans objet dans sa lettre**, puisque `t` n'est jamais absent en 1a. Forme voisine : `g__take_fixed` accepte nil et pose 0 au lieu d'avorter. Attendu rouge : test d'arrêt (cas `REF NULL`).
  3. (§7-3) *init qui saute un membre typé* :
     - (a) la branche `t` de la boucle de `g_` est retirée, si bien que la clé tombe dans l'`else` et avorte. Attendu rouge : caractérisation 4, `test_pico8_g_`, `test_pico8_probe_mm_rnd_order` ;
     - (b) la branche `t` fait `continue` sans appeler l'aide, et `t` reste à 0. Attendu rouge : caractérisation 4 (`t` = 3), `test_pico8_g_` (`get(c, "t") == 3`) ;
     - (c) `pico8_ke_struct_init` pose 1 au lieu de 0. Attendu rouge : caractérisation 2, `test_pico8_iv`, `test_pico8_spec_defaults`. L'omission pure n'est pas testée, parce qu'elle laisserait de la mémoire `malloc` non initialisée, non déterministe : c'est le **trou justifié** de cette forme.
  4. Propre à l'étape, *`dg` muet* : l'affectation typée de `dg` est retirée. Attendu rouge : caractérisation 1 (second parcours) et `test_pico8_iv` (`test_dg_drives_a_ke_struct`).
  5. Propre à l'étape, *`ks` sans remise à 0* : la ligne `t` de `ks` est retirée. Attendu rouge : caractérisation 4 (`g_` à `t` = 3, puis `ks`), `test_pico8_mz`.
  6. Propre à l'étape, *aide sans `assert`* : `g__take_fixed` recopie `value.as.fixed` quel que soit le genre. Attendu rouge : test d'arrêt.
  7. Propre à l'étape, *`mh:ev` sans son écriture* : `self->ke.t = rh*10` est retirée. Attendu rouge : `test_pico8_mh`, ou `test_pico8_ui` (`DRAG`/`TECH`) si un toast y rejoue son `lh`. Sinon, trou justifié dans le commit, avec le nom du TU qui l'aurait vu.
- **`gcov`** : `test_pico8_ui` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23, sur `pico8_ke.c`, `pico8_bootstrap.c`, `pico8_probe.c`, `pico8_msgbox.c`, `pico8_planet_display.c`, `pico8_wormhole.c`, `pico8_wormhole_display.c` et `pico8_slipway_display.c`. Chaque ligne modifiée doit être exécutée. Une ligne que `test_pico8_ui` n'atteint pas (probablement `mz:ey`, `mz:kj` et la branche `"ll"` de `jo:kj`, H12) est cherchée dans **une** passe `gcov` de `test_pico8_mz` et de `test_pico8_mh` (rapides). La branche `"ll"`, si elle n'est atteinte que par `test_pico8_jo` (lent), est justifiée dans le commit : `make e2e` l'exécute. Deux trous sont attendus :
  - l'échec de l'`assert` de `g__take_fixed`, exécuté seulement dans les fils de `must_abort`, qui avortent sans vider leur compteur (prouvé par le test d'arrêt et la mutation 6) ;
  - la branche « `t` absent → nil » de `dg`, inchangée et devenue inatteignable pour un `ke`.

  Le rapport va dans le commit.
- **Prior art** : `test_ke_me_by_name` et `test_ke_ks_stops_off_row` (1b.14a, même fichier, même aide de lecture) ; `test_too_far_writer_stops_off_row` et `write_fo_zero_false` (1b.13, cas à union nulle) ; `keyed` de `test_pico8_g_` (clés de construction) ; `test_dg_drives_a_ke_struct` (`test_pico8_iv`).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_ui` : 10 à 30 s. Une douzaine de lancements (ancien code, nouveau code, 7 mutations, `gcov`) en 4 min environ ;
- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min ;
- `make class-coverage-net` : 3 à 4 min, **dans la même invocation** que `make e2e` (arbres distincts, 1b.0) ;
- `make sanitize-net` : ~4 min seul, **jamais** lancé en même temps que `e2e` ;
- `make value-members` : quelques secondes.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les 8 TU de `UNIT_TESTS_SLOW` passent. **Aucun n'est modifié**, mais tous exercent le code changé (`test_pico8_ie`, `test_pico8_jo`, `test_pico8_qh`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`, `test_pico8_nr` : `t` par nom ; `test_pico8_jo` aussi par `ks` et `jo:kj`) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 207`**, sans aucune ligne `pico8_ke_t` ;
5. `timeout 15m make class-coverage-net` : **sortie identique à `tools/class_coverage_net_expected.txt`**. Cette cible s'applique parce que le diff touche deux constructeurs (`pico8_ke_struct_init`, `pico8_g__struct_new`), sans toucher ni `pico8_class_coverage_record` ni `KNOWN[]`. Aucun compte ne doit changer.

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS` ni un fichier d'événements.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si 1b.14a n'est pas commitée quand l'étape commence, ou si le commit de 1b.14b mêle des changements de `me`.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si `g__take_fixed` avorte dans un rejeu ou dans un TU hors des fils de `must_abort` (H2), si un indicateur `has_t` ou un membre est ajouté, ou si le type de `t` est élargi (union, `pico8_value_t` gardé à côté) au lieu d'avorter.
- `FAILED:` si `t` n'est pas `pico8_fixed_t` à sa place dans `pico8_ke_t`, ou si `me` y change.
- `FAILED:` si une table `keys[]`, `members[]` ou `defaults[]` liste encore `t` (preuves `offsetof` et `{"t",`), si une autre entrée de `keys[]` de `g_` est retirée ou modifiée, si le message de l'`assert` de l'`else` change, ou si un `g_<X>_t` ou un `g_ke_t` est créé.
- `FAILED:` si le sélecteur `pico8_value_t *` de `pico8_ke_field` reste, si la branche `t` peut rendre 0, ou si elle ne rend pas `pico8_value_fixed(t)`.
- `FAILED:` si l'aide n'est pas `static` dans `pico8_probe.c`, si elle accepte nil, `false` ou une chaîne NULL, ou si son message ne nomme pas `g_.ke.t` et `fixe`.
- `FAILED:` si un écrivain par nom de `t` est ajouté (branche `"t"` dans un `set_field`), ou si écrire `"t"` par nom cesse d'être l'arrêt « clé inconnue ».
- `FAILED:` si la signature de `pico8_ke_ks` change, ou si `ks` ne remet pas `t` à 0.
- `FAILED:` si un site par nom change (`dg` et ses deux lectures, `g_:mm`, `ma()`, lecteurs des filles), ou si un site par nom est remplacé par un accès direct.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si le diff d'un test fait autre chose qu'ajouter la caractérisation, le test d'arrêt, `KE_T_HASH` et leurs appels dans `test_pico8_ui`, et changer les trois lignes `ke.t` de `test_pico8_iv` ; ou si une valeur figée existante change (`STRUCT_HASH` ×2, `DRAG_REPLAY_HASH`, `TECH_REPLAY_HASH`, `LL_DG_CALLS`, `KE_ME_HASH`…) ; ou si un test de régression (§6) ou un TU de `UNIT_TESTS_SLOW` change.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 207, liste encore une ligne `pico8_ke_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` s'il manque une ligne « Runs » de `make class-coverage-net` identique à la référence, ou si le diff touche `pico8_class_coverage_record` ou `KNOWN[]`.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit ou écrit `t` directement, si elle ne couvre pas les 24 genres, les entités vivantes avant et après `dg`, la clé de `g_` et `ks`, ou si `KE_T_HASH` change après le typage. Aussi si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'ancien code et une verte après, ou s'il ne porte pas le cas `pico8_value_bool(0)`.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 7 mutations, chacune avec son test ou la justification de son trou.
- `FAILED:` si le travail est découpé en plusieurs commits dont un seul est rouge.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.14b » ou le tableau « Runs ».

## Out of Scope

- `ke.me` (1b.14a, précondition).
- Les autres clés de construction de `g_` (`pn`, `du`, `py`), `g_g__defaults[]` et `g_g__members[]` (1b.32), ainsi que toute autre table des filles.
- Les lecteurs par nom de `t` (`dg`, `g_:mm`, `ma()`), la branche « absent → nil » de `dg` et les aides `fixed_of`, `pico8_mzd_fixed` et `pico8_jc_display_value_fixed`, qui gardent d'autres appelants.
- Le `t` homonyme des propositions d'échange (`lw.t`, `gf.t`) et des listes d'absents de `mo`, `je`, `jc_too_far`, `jc_mouse_point` et `mz.jj`.
- `pico8_ke_default` et la spec `ob` de `ke` (phase D).
- Régénérer `tools/member_census.txt`.
- Les fichiers d'événements et `FRAMES_EVENTS`, tout changement de rendu, l'audio, et lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `ke` (type, lecteur, `ks`, init), `g_` (branche de clé et aide), `dg` (écriture), et les lectures et écritures directes que le compilateur désigne (`mh:ev`, `jo:kj`, `mz:ey`, `mz:kj`, `jc:lg`). Un seul commit, **sans découpage** ;
  - tests : `test_pico8_ui` étendu (caractérisation sur les 24 genres, entités vivantes avant et après `dg`, clé de `g_`, `ks`, et test d'arrêt rouge d'abord avec le cas à union nulle). Pas de nouveau TU, pas de nouveau TU lent. `test_pico8_iv` est retouché de trois lignes ;
  - `t` → `pico8_fixed_t`, sans `has_t` ;
  - pas d'écrivain par nom. La forme 1b.13 s'applique à la clé de construction de `g_`, avec l'aide `g__take_fixed` qui avorte sur nil ;
  - R7 selon 1b.14a : vacante pour les tables de défauts, puisque aucune fille ne liste `t`. Pas de `static` déballé de la spec ;
  - `class-coverage-net` est lancé.
- **Note « Découvert à 1b.14b »**, sur la ligne 1b.14b de `docs/plan-3.md` : « `t` → `pico8_fixed_t`, sans indicateur (nil `jamais`, le cart fait `qc.t+=1`) ; `pico8_ke_t` n'a plus de `pico8_value_t` ; **R7 de 1b.14a vacante** quand aucune table de défauts de fille ne liste le membre : pas de `static` déballé, le littéral de l'init de la parente suffit pour un fixe ; la seule entrée d'offsets était la clé de construction `t` de `g_` : branche de boucle de clés (1b.10) et aide `g__take_fixed` (1b.13, non-pointeur sans indicateur, nil avorte), que 1b.32 étend à `pn`/`du`/`py` ; pas d'écrivain par nom, `dg` écrit directement (R6) après sa lecture par nom gardée ; `pico8_mh_value_fixed` supprimée. Empreinte réelle 36 (10, 26), 11 fichiers modifiés. » Le reste va dans le commit.
- Le message de commit porte le périmètre, le compte 208 → 207, la liste des sites (lecteur, 4 écritures directes, 5 lectures directes, la clé de `g_`), le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous, `KE_T_HASH` et les réponses à H1 à H12.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
