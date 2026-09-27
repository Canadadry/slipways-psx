---
title: "Plan 3, étape 1b.14a : `ke.me` (l'état courant de toute entité) — `me` devient un `const char *`, et son défaut quitte les tables de `jo`, `qh` et `pm`"
description: "`pico8_ke_t.me` est un `pico8_value_t` (`chaine`, nil `observé` par `stat?` seulement, lu par nom) que 24 genres d'entité enchâssent. Le typer `const char *` selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 209 à 208. C'est la première étape qui type un membre d'une racine à filles : l'entrée `me` sort de trois tables de défauts (nom/valeur de `jo`, `offsetof` de `qh` et de `pm`). Ce PRD fixe la lecture de R7 pour une table qui garde des entrées et a des filles."
status: needs-triage
---

## Problem Statement

Lignes de l'étape, recopiées de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.14 — `ke` (`ke.h:12`), empreinte 42 (19, 23), découpée :
>
> 1b.14a — `ke.me`, défaut réécrit dans les tables de toutes les filles. Empreinte 34 (14, 20). Compte : 208.

1b.14a ne type que `ke.me`. `ke.t` est l'étape 1b.14b.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité ici, pas redécidé. Les notes « Découvert à » de `docs/plan-3.md` qui suivent s'appliquent aussi. Elles sont citées, pas redérivées :

- **1b.1** : R1 à R9 ; `chaine` → `const char *`, pointeur sans indicateur (NULL = nil) ; R3 « un défaut REF garde la même REF » ;
- **1b.3** : R3 se lit sur ce qui initialise l'objet ;
- **1b.4** : pointeur emprunté, même pointeur ;
- **1b.4b** : la table de recensement est corrigée. La ligne `ke.me` n'y change pas de genre ;
- **1b.5** : un emballage public unique par membre seulement pour les genres mêlés, donc **pas ici** ; les appelants d'un accesseur retiré prennent une branche explicite ;
- **1b.6** : un lecteur qui copie n'est pas un `pico8_X_member` ;
- **1b.7** : un nil observé seulement par `stat?`, venu d'un homonyme, ne décide rien quand toutes les écritures sont prouvées ;
- **1b.8** : R7 s'applique à un sélecteur `pico8_value_t *` local au lecteur ;
- **1b.9 et 1b.10**, pour **R7 forme `offsetof`** et les **aides de boucle de clés** : l'entrée du membre typé est retirée de chaque table ; la valeur est posée par affectation typée ; une aide `static` par genre pose d'abord nil, puis sort sur nil, sinon lève un `assert` « champ sans règle » qui nomme le membre et le genre de sa ligne. 1b.9 laisse ouvert le cas « une table qui garde des entrées ou a des filles (`jo`, `g_`) ». **1b.14a est ce cas** (voir « R7 : lecture nouvelle ») ;
- **1b.11**, pour **R2** : un TU de `UNIT_TESTS_SLOW` n'est pas recensé, donc chaque PRD relit ceux qui touchent sa struct pour y chercher des lectures nil par nom (revue plus bas) ;
- **1b.12** : `ref:autre` reçu **en argument de méthode** et déballé par la lecture R5 de 1b.10. C'est le précédent de `ks(me)` ;
- **1b.13** (PRD `docs/prd/triage/plan3-phase1b-etape13-jc-too-far-ecrivain.md`, **précondition : commitée**) : **forme écrivain de R5**. Une aide `static` par genre prend la valeur (`<X>_take_<genre>(value, &<m>)`), pose d'abord nil, sort sur `REF NULL`, sinon lève un `assert` « champ sans règle » qui nomme `struct.membre` et le genre de la ligne. Un nom sans branche rend 0, et `pico8_entity_set_field` avorte. Un `must_abort` écrit rouge d'abord fige l'arrêt.

### État de départ

HEAD 40a9bfd. 1b.13 est en cours dans l'arbre de travail (`src/pico8_slipway.c`, `tests/test_pico8_jc.c`, `docs/plan-3.md` modifiés). **Précondition** : 1b.13 est commitée avant que 1b.14a commence. Elle touche `src/pico8_slipway.c`, où cette étape change une ligne (`self->ke.me = pico8_value_string("kz")` de `pico8_jc_struct_new`), mais pas cette ligne-là : chaque site est désigné par son contenu, pas seulement par son numéro (H9). `make value-members` rend alors 209 (H1).

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H12).

### Ligne de recensement (`tools/member_census.txt:41`)

| struct | membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|---|
| `pico8_ke_t` | `me` | offset | `chaine` | observé | `stat?` | l |

Un membre : 209 − 1 = **208**. Ligne voisine non typée ici : `pico8_ke_t t` (`fixe`, `jamais`, `l`), c'est 1b.14b.

- **`chaine` seul**, sur 15 rejeux et 47 TU recensés. Le recensement rattache les membres d'une parente enchâssée à la parente (`parent[]` de `tools/member_census.sh`) : la ligne couvre les 24 genres qui enchâssent `ke`.
- **Nil `observé` par `stat?` seulement.** `stat?` veut dire qu'un test de vérité ou un `= nil` a été trouvé par grep sur le nom `me`, et que plusieurs typedefs portent ce nom. Les deux sources statiques sont des homonymes : `pico8_jo_truthy(me_val)` sur le `me` de la **table** `nk` (`planet.c`, `pico8_jo_hh`), et `ie_ke.me`, déjà typé à 1b.11. Aucun lecteur n'a lu `ke.me` nil (pas de `lu`), et aucun écrivain par nom ne l'a écrit nil (pas d'`écrit`).
- **`l` seul, pas de `e`** : aucun `set_field` ne connaît `"me"`. Le seul écrivain hors initialisation est la méthode `ke:ks(me)`, en C `pico8_ke_ks(self, pico8_value_t me)`.

**Revue des TU lents (règle R2 de 1b.11).** Relus : `test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`. Tous lisent `me` par nom, et toujours présent :

- `fold_field(…, "me")` dans les listes `scalars` de `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr` ;
- `strcmp(get(placed, "me").as.string, "ly")` dans `qh`, `os`, `hj`, `pm`, `mx`, `nr` ;
- `is_string(icon, "me", "ph")` dans `ie` (`:345`, `:362`, `:389`) ;
- `pico8_value_equal(own, through_hj)` sur `inherited` dans `pm` (`:687-714`), qui compare aussi les **pointeurs** de chaîne ;
- `pico8_ke_ks(a, pico8_value_string("ll"))` dans `jo` (`test_ll`, `:590`).

Les `"me"` de `test_pico8_ie.c:880/890` visent la vue `ie_ke` (1b.11), un homonyme. **Aucune lecture nil de `ke.me` par nom** (H4). De toute façon, R2 ne donne pas d'indicateur à un pointeur.

### Ce que fait le cart (`pico8/lua.lua`)

- `ke=kv:jr([[ me="ph",t=0 ]])` (`:182`), et `function ke:ks(me)self.me,self.t=me,0` (`:185`).
- Défauts propres des filles : `jo` `me="unknown"`, `qh` `me="kz"`, `pm` `me="ly"`, `mz` `me="kz"`, `jc` `me="kz"`. Les autres héritent par la chaîne de classes.
- Écritures de `ks` : `self:ks(self.nk.me or "known")` (`:411`, `jo:hh`), `self:ks("ly")` (`:458`), `self:ks(self.dq or "ph")` (`:571`, `d_`). `dq` est `chaine` partout (`qh.dq`, `g_.dq`, `mz.dq` : `chaine`, `jamais`). Le seul `nk.me` des données est `o("ll",45,0,me="ll")` (`data_init.c:65`).
- `dg()` appelle chaque frame `qc[qc.me](qc,qc.t)` (glossaire : « `me` : nom de l'état/icône courant d'une entité »).

### « Défaut réécrit dans les tables de toutes les filles » : ce que c'est dans le code

**Qui pose `me` à la construction**, pour chacun des 24 genres (les 14 structs qui enchâssent `pico8_ke_t` en premier membre, plus leurs descendants) :

| genre | chaîne d'init | écrit en dernier par | valeur | origine du pointeur |
|---|---|---|---|---|
| `bg`, `nf`, `mh`, `iv`, `g_`, `fv`, `ie`, `pj`, `qb`, `ui`, `js`, `fq` | `pico8_ke_struct_init` | `ke.c`, `pico8_value_string("ph")` | `"ph"` | littéral de `ke.c` |
| `jc` | `ke`, puis `pico8_jc_struct_new` | `slipway.c`, `self->ke.me = pico8_value_string("kz")` | `"kz"` | littéral de `slipway.c` |
| `mz` | `ke`, puis `pico8_mz_struct_new` | `wormhole.c`, `self->ke.me = pico8_value_string("kz")` | `"kz"` | littéral de `wormhole.c` |
| `jo`, `ga` | `ke` → `jo` | `pico8_jo_struct_init`, entrée `"me"` de `g_jo_defaults` (**table nom/valeur**, branche spéciale `strcmp(…, "me")` → `self->ke.me = …`) | `"unknown"` | spec de `jo` |
| `qh`, `nl`, `os`, `hj`, `nr`, `fr`, `mx` | `ke` → `jo` → `qh` (→ …) | `pico8_qh_struct_init`, entrée `{"me", offsetof(pico8_qh_t, jo.ke.me)}` de `g_qh_defaults` | `"kz"` | spec de `qh` |
| `pm` | `ke` → `jo` → `qh` → `hj` → `pm` | `pico8_pm_struct_new`, entrée `{"me", offsetof(pico8_pm_t, hj.qh.jo.ke.me)}` de `g_pm_defaults` | `"ly"` | spec de `pm` |

L'expression de la ligne du plan désigne donc exactement **trois tables** : `g_jo_defaults` (`planet.c:34-41`, nom/valeur), `g_qh_defaults` (`planet_family.c:60-70`) et `g_pm_defaults` (`planet_family.c:578-591`), ces deux dernières en `offsetof`. Les 21 autres genres ont en plus leurs propres tables (`members[]`, `defaults[]`, `keys[]`), mais **aucune n'a d'entrée `me`** : `grep -rnE 'offsetof\([a-z0-9_]+, ([a-z0-9_]+\.)*me\)' src tests` ne rend que les deux lignes de `qh` et de `pm`. Ces 21 genres héritent de `me` par la chaîne d'init.

**La chaîne `pico8_ke_default` (1a.42b/c).** Chaque module expose `pico8_X_default(name)`, qui lit « sa spec, puis celle de sa parente » jusqu'à `pico8_ke_default` (spec de `ke`, `"\nme=\"ph\",t=0,\n"`). Ce sont les lecteurs des **specs** : lecteur de genre (`pico8_entity_kind_t`), recopie de défauts à l'init de module, tests `*_spec_defaults`. Ils ne lisent jamais une struct. **Ils ne changent pas** : les specs restent des tables `ob` (phase D). Le typage n'intervient qu'à la recopie spec → struct : `pico8_jo_init` et `pico8_planet_family_init` remplissent `g_*_defaults[i].value` par `pico8_X_default(name, …)`, puis `*_struct_init` recopie. C'est **cette** recopie qui perd son entrée `me` (R7).

**Identité du pointeur, déjà figée.** Comme `pico8_value_equal` compare les chaînes **par adresse**, deux TU rapides de `CENSUS_UNIT_TESTS` figent déjà, par nom, présence et identité de `me` face à la spec :

- `test_pico8_jo_spec_defaults` (`test_struct_defaults`, `STRUCT_HASH`) pour les 10 genres de l'arbre `jo` : `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr` et `mx` doivent rendre le **même pointeur** que leur spec ;
- `test_pico8_spec_defaults` (`STRUCT_HASH`) pour les 13 filles de `ke` hors de l'arbre `jo` qu'il construit (`pj`, `js`, `fq`, `nf`, `qb`, `mh`, `iv`, `ui`, `ie`, `bg`, `mz`, `g_`, `fv` ; `jc` n'y est pas construit) : leur `"ph"` ou `"kz"` littéral n'est **pas** le pointeur de la spec, et ce « pas le même » est figé aussi.

Le défaut propre de `jo`, `qh` et `pm` doit donc rester **le pointeur de la spec**. Les littéraux de `ke`, `jc` et `mz` doivent rester des littéraux.

### Le code aujourd'hui

- **Typedef** (`ke.h:10-14`) : `{ pico8_struct_header_t header; pico8_value_t me; pico8_value_t t; } pico8_ke_t`.
- **Init** `pico8_ke_struct_init` (`ke.c:42-47`) : tag, descripteur, `me = pico8_value_string("ph")`, `t = fixe 0`.
- **Lecteur** `pico8_ke_field` (`ke.c:49-65`) : c'est un sélecteur local `const pico8_value_t *member` (`me` ou `t`), `REF NULL` → 0, sinon copie. **C'est le seul lecteur par nom de `me` pour les 24 genres.** Chaque lecteur de fille retombe sur lui quand le nom n'est pas le sien (`bg.c:78`, `wormhole.c:311`, `slipway.c:498`, `probe.c:479`, `ui.c:154`, `msgbox.c:146/350`, `mouse_reader.c:39`, `scroll.c:63`, `ui_button.c:137/398`, `score_panel.c:426`, `research_icon.c:335`, `planet.c:203`, puis `qh` → `jo`, `pm` → `hj` → `qh` → `jo`, et ainsi de suite). Aucune fille ne connaît `"me"` dans ses propres tables.
- **Écrivain** `pico8_ke_ks` (`ke.c:32-40`) : `NULL` → rien ; `assert` tag `STRUCT` ; `me = <valeur reçue, tout genre>` ; `t = fixe 0`. Appelants : `planet.c` (`pico8_value_string("ly")` dans le chemin de pose sans projet, `ks_arg` de `pico8_jo_hh`, `nk.me` vrai ou `"known"`), `probe.c` (`dq_val` de `d_`, `dq` vrai ou `"ph"`), et les tests `test_pico8_ui` (`ks_on_a_table`, `ks(NULL, …)`), `test_pico8_g_` (`:542-543`, `"kz"`) et `test_pico8_jo` (`:590`, `"ll"`, TU lent).
- **Lectures directes** (compilateur, R6) : `planet.c` `pico8_jo_gg` (`me_val = self->ke.me; if (kind == STRING) … "ly"`) et `pico8_jo_ldown` (`self->ke.me.kind != STRING` → nil, puis `"known"`/`"ly"`/…) ; `planet_display.c` `pico8_jo_gv` (`kind != STRING || != "ly"` → nil) et `pico8_jo_kj` (`kind == STRING && == "ll"`) ; `wormhole_display.c` `pico8_mz_kj` (`kind == STRING && as.string != NULL && == "kz"`) ; `tests/test_pico8_iv.c:271` (`e->ke.me.kind == STRING && strcmp(e->ke.me.as.string, "ph") == 0`).
- **Écritures directes** (R6) : `wormhole.c` et `slipway.c`, `self->ke.me = pico8_value_string("kz")` juste après `pico8_ke_struct_init` ; `planet.c` `pico8_jo_struct_init`, `self->ke.me = g_jo_defaults[i].value`.
- **Lecteurs par nom, inchangés** : `bootstrap.c` `pico8_dg` (`me` → `pico8_mv(qc, me_val.as.string, …)`), `probe.c` `dl` (`"kz"`), `wormhole.c` (`"unknown"`), `economy_tiers.c` (`"ly"`), et les TU.

### Coexistence avec les filles pas encore typées

La puce **Coexistence** exige qu'un membre typé sorte de **toutes** les tables, celles des filles comprises. Les faits :

- **Lecteurs des filles** : aucun ne traite `me`. Tous délèguent à `pico8_ke_field`, qui prend la branche R4. Les sélecteurs et les tables `members[]` des filles gardent leurs membres `pico8_value_t` sans rien changer.
- **Tables d'offsets des filles** : les deux entrées `offsetof(…ke.me)` (`qh`, `pm`) sortent, ainsi que l'entrée nom/valeur `"me"` de `jo` et sa branche spéciale. Rien d'autre ne pointe sur `me`. La seule autre entrée `offsetof` vers `ke` est `{"t", offsetof(pico8_g__t, ke.t)}` (`probe.c`, clés de construction de `g_`), qui appartient à 1b.14b.
- **Boucles génériques** `*(pico8_value_t *)((char *)self + off) = …` des filles (`qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`, `g_`, `fv`…) : une fois l'entrée `me` retirée, aucune n'atteint plus `me`. Preuve par grep plus bas.
- **Écrivains par nom des filles** (`jo`, `qh`, `hj`, `g_`, `bg`, `jc`, `mz`…) : aucun ne connaît `"me"`. Écrire `"me"` par nom reste l'arrêt « clé inconnue » de `pico8_entity_set_field`, et ne change pas.

### Empreinte remesurée ; découpage

Les deux greps de la puce **Liste** redonnent **34 (14, 20)**, la valeur de la ligne. Mais ce sont des majorants :

- dans `src/`, `mouse_reader.c`, `scroll.c` et `planet.h` n'ont que le commentaire `qc[qc.me]`, et `research_icon.c` n'a que l'homonyme `ie_ke.me` ;
- `ke.h`, qui déclare le membre, n'y figure pas ;
- `test_pico8_js`, `bg`, `mz`, `ie`, `ui` et `pj` lisent `me` par des aides (`is_string`, `get`, `lookup`) que le grep ne voit pas.

**Empreinte réelle** (fichiers qui lisent ou écrivent `ke.me` d'une struct) : **37 (11, 26)**. En `src` : `ke.h`, `ke.c`, `planet.c`, `planet_family.c`, `planet_display.c`, `wormhole.c`, `wormhole_display.c`, `slipway.c`, `probe.c`, `bootstrap.c`, `economy_tiers.c`.

**Fichiers modifiés : 10**, plus `docs/plan-3.md` :

- **src (8)** : `pico8_ke.h`, `pico8_ke.c`, `pico8_planet.c`, `pico8_planet_family.c`, `pico8_planet_display.c`, `pico8_wormhole.c`, `pico8_wormhole_display.c`, `pico8_slipway.c` ;
- **tests (2)** : `test_pico8_ui.c` (pur ajout : la caractérisation et le test d'arrêt) et `test_pico8_iv.c` (une ligne de lecture directe, forcée par le compilateur).

Les 29 autres fichiers lisent par nom et restent intacts : c'est la puce Coexistence. **Pas de découpage en 1b.14a1/a2** (G). Dix fichiers, dont huit ne changent que de 1 à 15 lignes, c'est moins que 1b.9 (20 fichiers d'empreinte, 17 membres) ou 1b.10. Un seul membre, un seul genre, pas d'indicateur. Et une coupe laisserait un commit intermédiaire incompilable : le type du membre change pour tous ses sites à la fois.

**TU de `UNIT_TESTS_SLOW` : aucun modifié.** Tous les huit exercent le code changé (`me` lu par nom, et `ks("ll")` dans `test_pico8_jo`), et `make e2e` les passe. La signature de `pico8_ke_ks` est gardée **pour** ne pas toucher `test_pico8_jo` (voir R5).

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code.** On lit `me` par nom sur les 24 genres de filles, sur toutes les entités vivantes d'une partie, et après les écritures par `ks` (voir Testing Decisions).
2. **Test d'arrêt** de `ks` hors genre, écrit rouge sur l'ancien code (qui accepte tout), vert après.
3. **Typage** : `const char *me;` dans `pico8_ke_t`, à la même place et sous le même nom.
4. **Défauts** : `ke` pose `"ph"`, `jc` et `mz` posent `"kz"` par affectation typée. `jo`, `qh` et `pm` retirent `me` de leur table de défauts. Ils déballent **une fois**, à l'init de module, le pointeur de leur spec dans un `static const char *`, et le posent dans leur `struct_init`/constructeur.
5. **Lecteur** : une branche R4 pour `me` dans `pico8_ke_field`. **Écrivain** : `pico8_ke_ks` déballe par une aide `static` de forme 1b.13.
6. Effet mesurable : `make value-members` passe de 209 à **208**. La seule ligne `pico8_ke_t` restante est `t`. Aucun pixel ne bouge.
7. **Note « Découvert à 1b.14a »** : la lecture de R7 pour une table de défauts qui garde des entrées et a des filles, à recopier par 1b.14b, 1b.23 (`jo`, 9 filles) et 1b.32 (`g_`, fille `fv`).

## User Stories

1. En tant que lecteur de `ke.h`, je veux `me` typé `const char *`, afin que le nom de l'état d'une entité se lise sur le type.
2. En tant que `dg` (`bootstrap.c`), je veux lire `me` par nom sur toute entité de `hv`, présent, de genre chaîne, même pointeur, afin d'appeler la même méthode d'état que dans 1a.
3. En tant que lecteur par nom de n'importe laquelle des 24 filles, je veux que `me` passe par le lecteur de `ke` et rende exactement ce que rendait 1a : présence, genre et pointeur.
4. En tant que `jo`, `ga` et les fils de `qh` et `hj`, je veux recevoir le pointeur même de la spec qui fournit mon défaut (`"unknown"` de `jo`, `"kz"` de `qh`, `"ly"` de `pm`), afin que `STRUCT_HASH` de `test_pico8_jo_spec_defaults` et le `pico8_value_equal` de `test_pm_is_a_struct` ne bougent pas.
5. En tant que `bg`, `nf`, `mh`, `iv`, `g_`, `fv`, `ie`, `pj`, `qb`, `ui`, `js` et `fq`, je veux garder le `"ph"` littéral de `ke`, qui n'est pas le pointeur de la spec, afin que `STRUCT_HASH` de `test_pico8_spec_defaults` ne bouge pas.
6. En tant que `jc` et `mz`, je veux garder mon `"kz"` littéral posé après l'init de `ke`.
7. En tant que `d_` (`probe.c`) et `jo:hh`/`jo:ly` (`planet.c`), je veux appeler `ks` avec une valeur, comme aujourd'hui, et que `me` prenne la chaîne reçue, même pointeur, avec `t` remis à 0.
8. En tant que `test_ll` (`test_pico8_jo`, TU lent), je veux que `pico8_ke_ks(a, pico8_value_string("ll"))` compile et se comporte comme avant.
9. En tant que lecteur direct (`jo:gg`, `jo:ldown`, `jo:gv`, `jo:kj`, `mz:kj`), je veux tester `me != NULL && strcmp(me, …) == 0`, qui a la même table de vérité que le test de genre de 1a.
10. En tant que mainteneur, je veux qu'un `ks` qui reçoit autre chose que nil ou une chaîne non NULL (un fixe, un booléen, une REF, une chaîne NULL) arrête l'exécution par un `assert` « champ sans règle » qui nomme `ke.me` et `chaine`.
11. En tant que mainteneur, je veux que `ks(e, nil)` rende `me` absent, exactement comme en 1a, où `me` = `REF NULL` était lu absent.
12. En tant que mainteneur, je veux qu'aucune table de défauts ne liste plus `me`, ni par `offsetof` ni par nom, et qu'aucune écriture `*(pico8_value_t *)` n'atteigne `me`.
13. En tant qu'auteur de 1b.14b (`ke.t`), de 1b.23 (`jo`) et de 1b.32 (`g_`), je veux une forme écrite de R7 pour une table de défauts qui garde des entrées et a des filles, afin de la recopier sans la redécider.
14. En tant qu'auteur de 1b.23 et des étapes des filles de `jo`, je veux savoir que `g_jo_defaults` a perdu son entrée `me` et sa branche spéciale.
15. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
16. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
17. En tant que gardien de la couverture, je veux `make class-coverage-net` identique à sa référence, puisque des constructeurs changent.
18. En tant que `plan-pilot`, je veux `make value-members` à 208.
19. En tant que vérificateur, je veux une caractérisation par nom sur toutes les filles, lancée sur l'ancien code, les mutations du §7 avec leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés comme le ferait un agent `guideline` (hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables), et marqués (G).

### R1–R9 appliquées à `ke.me`

| règle | sur `ke.me` |
|---|---|
| R1 | `chaine` → `const char *me`, déclaré à la place de l'ancien membre, entre `header` et `t`. **Emprunté**, jamais possédé ni libéré : le pointeur vient d'un littéral, d'une spec ou d'une valeur reçue par `ks`, et vit aussi longtemps qu'aujourd'hui. |
| R2 | **Aucun indicateur** : c'est un pointeur (NULL = nil). Le nil `stat?` vient d'homonymes (lecture 1b.7), et la revue des TU lents n'ajoute rien. |
| R3 | Affectations typées, à la place des anciennes, avant les clés de construction et tout `pq` : `ke` pose `self->me = "ph"` ; `jc` et `mz` posent `self->ke.me = "kz"` ; `jo`, `qh` et `pm` posent `… .me = g_<X>_me` (voir R7). L'ordre reste le même : `ke`, puis la fille qui écrase. |
| R4 | Dans `pico8_ke_field`, une branche `strcmp(name, "me")` **avant** le sélecteur : NULL → `return 0`, `*out_value` intact ; sinon `*out_value = pico8_value_string(ke->me)`, même pointeur, `return 1`. Le sélecteur local ne garde que `t` (1b.14b le retire). |
| R5 | Moitié écrivain, forme de 1b.13, sur la méthode `ks` (précédent de l'argument de méthode : 1b.12). Voir la section suivante. Pas de `set_field` : `me` n'a pas de `e`. |
| R6 | Lectures directes : `x.kind == STRING && strcmp(x.as.string, s) == 0` devient `x != NULL && strcmp(x, s) == 0`, et `x.kind != STRING` devient `x == NULL`. Le `as.string != NULL` de `mz:kj` se fond dans `!= NULL`. Écritures directes : `= pico8_value_string("kz")` devient `= "kz"`. Les locales `pico8_value_t me_val` qui ne servent qu'à ça disparaissent, ou deviennent `const char *`. |
| R7 | Retrait de `me` des trois tables (voir « R7 : lecture nouvelle »). |
| R8 | Tous les `PICO8_MEMBER_CENSUS_SEE`, `READ` et `WRITE` gardés. `tools/member_census.txt` n'est pas édité. |
| R9 | 209 → 208. Plus de ligne `pico8_ke_t me`, et `pico8_ke_t t` reste. |

### R5 : `ks(me)` déballe par une aide de forme 1b.13 (G)

- **Signature gardée** : `void pico8_ke_ks(pico8_struct_header_t *self, pico8_value_t me)`. (G) La passer en `const char *` toucherait `planet.c`, `probe.c` et trois TU, dont `test_pico8_jo`, qui est lent. Le cart passe une **valeur** (`self.nk.me or "known"`, `self.dq or "ph"`), et 1b.12 a déjà fixé qu'un argument de méthode se déballe par R5. Les appelants ne changent pas.
- **Aide** `static void ke_take_string(pico8_value_t value, const char **out)`, dans `pico8_ke.c`, à la forme par valeur de 1b.13 :
  1. elle pose d'abord `*out = NULL` (nil) ;
  2. elle sort si `value` est `REF NULL` ;
  3. sinon `assert(value.kind == PICO8_VALUE_STRING && value.as.string != NULL && "ke.me: a write neither nil nor a string is off its census row (chaine) -- a field under no rule of plan-3 section 5: stop, do not widen the member")` ;
  4. enfin `*out = value.as.string`.
- **Chaîne NULL refusée** (G) : en 1a, `{STRING, NULL}` était lu **présent**. Typé, un `me` NULL serait lu absent. Pour garder présent/absent identiques, il faut refuser ce cas. Aucun site n'en produit : les chaînes viennent de littéraux, des specs et de `dq`/`nk.me` (H2).
- `pico8_ke_ks` garde son `NULL` → rien, son `assert` de tag, puis appelle `ke_take_string(me, &((pico8_ke_t *)self)->me)` et remet `t` à 0 **sans changement** (`t` est 1b.14b).
- Nil reste accepté (`me` → NULL, lu absent), comme en 1a. Aucun appelant du jeu ne passe nil (H2), mais la caractérisation le fige.

### R7 : lecture nouvelle, une table de défauts qui garde des entrées et a des filles (G)

1b.9 a fixé la forme `offsetof` quand une table se vide, et laissé ouvert le cas « la table garde des entrées ou a des filles ». Forme retenue :

1. **Retrait.** L'entrée du membre typé sort de chaque table qui le liste, filles comprises, quel que soit son format : ici `{"me", {0}}` de `g_jo_defaults` (nom/valeur), `{"me", offsetof(pico8_qh_t, jo.ke.me), {0}}` de `g_qh_defaults` et `{"me", offsetof(pico8_pm_t, hj.qh.jo.ke.me), {0}}` de `g_pm_defaults`. La table reste, avec ses autres entrées, sa boucle de recopie et sa remise à nil. La branche spéciale `strcmp(g_jo_defaults[i].name, "me")` de `pico8_jo_struct_init` disparaît avec l'entrée. La boucle n'a plus que la forme `*pico8_jo_member(…) = …`.
2. **Défaut déballé une fois, à l'init du module.** Chaque module qui avait l'entrée reçoit un `static const char *g_<X>_me = NULL` (`g_jo_me` dans `planet.c` ; `g_qh_me` et `g_pm_me` dans `planet_family.c`). Il est rempli **dans l'init du module, juste après la boucle de recopie de sa table**, par son propre lecteur de défauts : `found = pico8_<X>_default("me", &v)`, puis `assert(found && v.kind == PICO8_VALUE_STRING && v.as.string != NULL && "<X>: me, a default of the struct, is a key of <X>'s spec and a string (chaine)")`, puis `g_<X>_me = v.as.string`. C'est le **même pointeur** que la spec, donc R3 « même REF », lu pour une chaîne. (G) L'`assert` est écrit en ligne, sans aide : c'est une lecture de spec, pas un écrivain, et il y en a trois.
3. **Remise à nil avec la table.** `g_<X>_me = NULL` dans `pico8_<X>_defaults_reset()`, que le `shutdown` du module appelle déjà. Le pointeur ne survit pas à la spec qu'il désigne.
4. **Pose typée dans l'init de la struct**, après la boucle de la table et avant les clés de construction et tout `pq` : `self->ke.me = g_jo_me` dans `pico8_jo_struct_init`, `self->jo.ke.me = g_qh_me` dans `pico8_qh_struct_init`, `self->hj.qh.jo.ke.me = g_pm_me` dans `pico8_pm_struct_new`.
5. **Filles sans défaut propre** : aucune écriture. Elles héritent par la chaîne d'init (`ga` ← `jo` ; `nl`, `os`, `hj`, `nr`, `fr`, `mx` ← `qh` ; `fv` ← `g_` ← `ke`). Les tables de ces filles ne sont pas touchées.
6. **Preuves** : `grep -rnE 'offsetof\([a-z0-9_]+, ([a-z0-9_]+\.)*me\)' src/` vide (la forme de la puce Coexistence, pour `ke` et ses 24 filles en une seule expression) ; `grep -n '"me"' src/pico8_planet.c` ne garde que la lecture de la **table** `nk` de `pico8_jo_hh` ; `grep -rnE '\{"me",' src/` vide.

Écarté (G) : poser un littéral (`"unknown"`, `"kz"`, `"ly"`) dans `struct_init`, parce que ça change le pointeur et rougit les deux `STRUCT_HASH` et `test_pm_is_a_struct`. Écarté aussi : garder l'entrée dans la table en déballant `g_jo_defaults[i].value.as.string` à la recopie, parce qu'une table de défauts ne doit lister que des `pico8_value_t` (Coexistence), et que ça laisserait une lecture `.as` que les preuves interdisent.

### Identité et durée de vie

- `jo`, `qh`, `pm` : même pointeur que la spec, comme en 1a. La spec est libérée au `shutdown` du module, qui remet aussi `g_<X>_me` à NULL. Une struct construite avant garde un pointeur pendant, **exactement comme en 1a**, où la valeur tenait le même pointeur.
- `ke`, `jc`, `mz` : littéraux, sans durée de vie à gérer. (H11) Le compilateur peut fusionner ou non deux littéraux `"kz"` identiques. Rien ne compare le pointeur d'un littéral à celui d'un autre littéral. Les tests ne comparent qu'au pointeur des specs, qui vit sur le tas.
- `ks` : le pointeur reçu, emprunté, comme en 1a.

### Preuves

Toutes rendent la sortie indiquée après l'étape :

- `grep -n 'pico8_value_t me;' src/pico8_ke.h` : vide. `grep -n 'pico8_value_t t;' src/pico8_ke.h` : une ligne ;
- `grep -rnE 'offsetof\([a-z0-9_]+, ([a-z0-9_]+\.)*me\)' src/ tests/` : vide (2 lignes avant) ;
- `grep -rnE '\{"me",' src/` : vide (3 lignes avant) ;
- `grep -rnE '(\.|->)me\.(kind|as)' src/ tests/` : vide ;
- `grep -rnE 'me = pico8_value_string|->me = me;' src/` : vide ;
- `grep -rn 'me = g_jo_defaults' src/` : vide ;
- `grep -c 'off its census row' src/pico8_ke.c` : 1 ;
- `grep -n 'pico8_value_t \*' src/pico8_ke.c | grep -v 'pico8_value_t \*out_value)'` : seulement le sélecteur `const pico8_value_t *member;` de `pico8_ke_field`, gardé pour `t` (1b.14b) ;
- `grep -rn 'pico8_ke_ks(' src/ tests/` : **mêmes lignes d'appel qu'avant**. Seul le corps de `ke.c` change ;
- `grep -rn '"me"' src/ tests/ | grep -v '^src/pico8_\(planet\|planet_family\)\.c'` : mêmes lignes qu'avant (les sites par nom sont intacts).

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 209 sur l'arbre de départ (1b.13 commitée), puis 208, sans ligne `pico8_ke_t me`, et avec la ligne `pico8_ke_t t`.
- **H2** : dans les 15 rejeux et dans tous les TU hors fils de `must_abort`, `ks` ne reçoit que des chaînes non NULL : littéraux, `nk.me` (seulement `"ll"` dans les données), `dq` (`chaine`, `jamais`). Il ne reçoit jamais nil, un booléen, un fixe ou une REF. Sinon l'aide avorte : c'est l'arrêt « champ sans règle ».
- **H3** : les 24 constructeurs `pico8_<X>_new(NULL)` réussissent dans un fils de `test_pico8_ui` après `start_game(DRAG)`. Précédents : chacun a au moins un appel `(NULL)` dans `tests/`, dont `pico8_jc_new(NULL)`, `pico8_iv_new(NULL)` et `pico8_js_new(NULL)`, et `test_pico8_jo_spec_defaults` construit les 10 genres de l'arbre `jo` par `(NULL)`.
- **H4** : aucun TU de `UNIT_TESTS_SLOW` ne lit `ke.me` nil par nom, et `me` n'est absent dans aucun rejeu.
- **H5** : aucune valeur figée ne bouge : `STRUCT_HASH` de `test_pico8_jo_spec_defaults` et de `test_pico8_spec_defaults`, les hash `fold_field` des TU de filles, `LL_DG_CALLS`, `DRAG_REPLAY_HASH`, `TECH_REPLAY_HASH`, ni les 15 empreintes du filet.
- **H6** : le build `-DPICO8_MEMBER_CENSUS` compile, et la table générée perd d'elle-même la ligne `me`. Il n'est pas lancé.
- **H7** : sous ASan+UBSan, rien de nouveau : aucune allocation, aucun pointeur dont la durée de vie change.
- **H8** : `test_pico8_ui` reste sous 30 s avec deux fonctions de plus (hors `UNIT_TESTS_SLOW`).
- **H9** : 1b.13, une fois commitée, ne touche pas la ligne `self->ke.me = …` de `pico8_jc_struct_new`. Les numéros de ligne cités glissent peut-être : chaque site est désigné par son contenu.
- **H10** : aucun code ne dépend de `sizeof(pico8_ke_t)` ni de la disposition des filles (la struct rétrécit). Le grep `offsetof`/`sizeof` des tests est vide sur `ke`, et aucun binaire ne sérialise une entité.
- **H11** : deux littéraux identiques peuvent être fusionnés ou non. Aucun test ne compare deux littéraux par adresse.
- **H12** : les lignes modifiées de `planet.c` (`jo:gg`, `jo:ldown`), `planet_display.c` (`jo:gv`, `jo:kj`) et `wormhole_display.c` (`mz:kj`) sont exécutées par `test_pico8_ui` (rejeux `DRAG`/`TECH`) ou, à défaut, par `test_pico8_mz` et `test_pico8_js` (voir `gcov`).

## Testing Decisions

- **Un bon test juge ce qu'on lit par nom, ou l'écran.** Aucun accès direct à `->ke.me` dans le nouveau test : il compile sur les deux formes. La seule exception est la ligne existante de `test_pico8_iv.c:271`, que le compilateur désigne. Elle devient `strcmp(e->ke.me, "ph") == 0` (plus `e->ke.me != NULL`), sans changer ce qu'elle affirme. (G) Elle reste une lecture directe : c'est le test des défauts d'`iv` « bit pour bit », et le passer par nom changerait son objet.
- **Caractérisation dirigée, figée sur l'ancien code** : `test_ke_me_by_name`, dans `test_pico8_ui` (rapide, dans `CENSUS_UNIT_TESTS`, lie `ENGINE_SRCS`, a déjà `start_game` et `must_abort`). Elle est lancée par `in_fresh_child` juste après `test_ke_closed`. Elle lit par le **lecteur brut du descripteur** (`((const pico8_struct_header_t *)e)->cls->field`, témoin fixe `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder sur le retour, le genre et le pointeur.
  1. **Toutes les filles.** `start_game(DRAG)`, puis `pico8_<X>_new(NULL)` dans cet ordre : `bg`, `nf`, `mh`, `iv`, `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `ga`, `nr`, `fr`, `mx`, `g_`, `fv`, `ie`, `pj`, `qb`, `jc`, `ui`, `js`, `fq`, `mz`. Pour chacune, `me` est présent, de genre `STRING`, avec la chaîne attendue (`assert` explicite : `"ph"` ×12, `"unknown"` pour `jo` et `ga`, `"kz"` pour `qh`, `nl`, `os`, `hj`, `nr`, `fr`, `mx`, `jc`, `mz`, `"ly"` pour `pm`). On calcule aussi une **classe d'identité** : 1 si le pointeur est celui de `pico8_ke_default("me")`, 2 celui de `pico8_jo_default("me")`, 3 celui de `pico8_qh_default("me")`, 4 celui de `pico8_pm_default("me")`, 0 sinon. Attendu : 0 pour les 12 `"ph"`, `jc` et `mz` ; 2 pour `jo` et `ga` ; 3 pour les 7 fils de `qh` ; 4 pour `pm`. Ces valeurs sont celles que donne la lecture du code. Si un `pq` en change une sur l'ancien code, c'est la valeur **observée sur l'ancien code** qui est figée, et l'écart est noté dans le commit (H3).
  2. **Intrus** sur le lecteur de `ke` : `pico8_ke_field(bg, n, …)` pour `n` ∈ {`""`, `"m"`, `"mee"`, `"me "`, `"has_me"`, `"ke"`} → 0, témoin intact.
  3. **Entités vivantes** : on parcourt `pico8_hv()` après `start_game(DRAG)`, **avant** les constructions de 1. Pour chaque entrée, on plie la présence, le genre et la chaîne de `me` par nom (pas l'adresse), plus le nombre d'entrées.
  4. **Écritures par `ks`.** Sur le `bg` de 1 :
     - `ks(bg, pico8_value_string(s))` avec `s` un tableau local `"ll"` → même pointeur `s`, et `t` présent, fixe 0 ;
     - `ks(bg, REF NULL)` → `me` absent par les deux lecteurs (0, témoin intact), `t` présent 0 ;
     - `ks(bg, pico8_value_string("ph"))` → présent.

     Sur le `pm` de 1 (quatre niveaux d'enchâssement) : `ks(pm, pico8_value_string(s))` → même pointeur `s` par le lecteur de `pm`, et `pico8_value_equal` avec la lecture par le lecteur de `hj`.
  5. On plie les retours, les genres, les chaînes et les classes d'identité dans **`KE_ME_HASH`**, relevé au premier passage vert sur l'arbre de départ. Les adresses sont jugées par `assert`.

  Elle est lancée verte sur l'arbre de départ **avant** le typage (ligne « Runs »). Ses valeurs ne changent pas ensuite.
- **Test d'arrêt, rouge d'abord** (G, précédent 1b.13) : `test_ke_ks_stops_off_row`, lancé par `in_fresh_child` après la caractérisation. Il appelle `must_abort` (celui du fichier, avec un enrobage par valeur) sur `pico8_ke_ks(e, v)`, `e` un `bg` neuf, pour `v` ∈ {fixe 1, `pico8_value_bool(1)`, `pico8_value_bool(0)`, REF vers `e` (une struct), `pico8_value_string(NULL)`}. **Rouge sur l'ancien code** (`ks` accepte tout, le fils sort par `_exit(0)`), vert après : une ligne « Runs » pour chacun des deux passages. Sur l'ancien code, la caractérisation passe **avant** lui dans le même binaire et imprime son hash. Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Tests existants, inchangés, qui figent déjà beaucoup** :
  - `test_pico8_jo_spec_defaults` (`STRUCT_HASH` : présence et identité face à la spec pour 10 genres ; `test_second_jl_keeps_ke`) ;
  - `test_pico8_spec_defaults` (`STRUCT_HASH` : 13 filles de `ke` hors `jo`) ;
  - `test_pico8_iv` (`test_dg_drives_a_ke_struct`, qui appelle la méthode que `me` nomme) ;
  - `test_ke_closed` ;
  - `is_string(…, "me", …)` de `bg`, `mz`, `jc`, `ie`, `js`, `mh`, `fq`, `qb`, `pj`, `ui` et `g_` ;
  - les huit TU lents (`test_ll`, `LL_DG_CALLS`).

  Aucun test de régression (§6) ne lit `me` directement : `nil_row` et `empty_planet_cleared` le lisent par nom et ne changent pas.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_ui && ./tmp/test_pico8_ui`, plus le TU cité quand il est plus rapide à rougir. Chacune est notée dans le commit avec le test qui la rougit, ou la justification de son trou.
  1. (§7-1) *indicateur forcé vrai* : **sans objet dans sa lettre**, pas d'indicateur. Forme la plus proche : la branche `me` de `pico8_ke_field` ignore NULL et rend `{STRING, NULL}` présent. Attendu rouge : caractérisation 4 (`ks` nil → absent).
  2. (§7-2) *« présent 0 » là où c'était absent* : **sans objet dans sa lettre**, aucun fixe. Forme voisine : `ke_take_string` sort sur nil **sans** poser `*out = NULL`, donc l'ancien `me` reste. Attendu rouge : caractérisation 4.
  3. (§7-3) *init qui saute un membre typé*, une par pose :
     - (a) `pico8_qh_struct_init` omet `me = g_qh_me` → les 7 fils de `qh` gardent `"unknown"`. Attendu rouge : caractérisation 1, `test_pico8_jo_spec_defaults`.
     - (b) `pico8_pm_struct_new` omet `g_pm_me` → `pm` garde `"kz"`. Mêmes rouges, plus `test_pico8_pm`.
     - (c) `pico8_jo_struct_init` omet `g_jo_me` → `jo` et `ga` gardent `"ph"`. Mêmes rouges.
     - (d) `pico8_ke_struct_init` pose NULL au lieu de `"ph"`. Attendu rouge : caractérisation 1, `test_pico8_iv`, `test_pico8_spec_defaults`. On ne teste pas l'omission pure, qui laisserait de la mémoire de `malloc` non initialisée, non déterministe : c'est le **trou justifié** de cette forme.
  4. Propre à l'étape, *identité perdue* : `pico8_jo_init` pose un littéral `"unknown"` au lieu du pointeur de la spec. Attendu rouge : caractérisation 1 (classe d'identité) et `test_pico8_jo_spec_defaults` (`STRUCT_HASH`).
  5. Propre à l'étape, *`ks` sans `assert`* : l'aide recopie tout genre. Attendu rouge : `test_ke_ks_stops_off_row`.
  6. Propre à l'étape, *`ks` muet* : `pico8_ke_ks` ne touche plus `me`. Attendu rouge : caractérisation 4, `test_pico8_jo` (`test_ll`, TU lent, noté sans le relancer si la caractérisation suffit), et les TU de pose `"ly"`.
  7. Propre à l'étape, *`mz` sans son `"kz"`* : la ligne de `pico8_mz_struct_new` est retirée. Attendu rouge : caractérisation 1, `test_pico8_mz` (`is_string(mz, "me", "kz")`).
- **`gcov`** : `test_pico8_ui` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23, sur `pico8_ke.c`, `pico8_planet.c`, `pico8_planet_family.c`, `pico8_planet_display.c`, `pico8_wormhole.c`, `pico8_wormhole_display.c` et `pico8_slipway.c`. Chaque ligne modifiée doit être exécutée. Une ligne que `test_pico8_ui` n'atteint pas (probablement `mz:kj` et peut-être `jo:gv`, H12) est cherchée dans **une** passe `gcov` de `test_pico8_mz` et de `test_pico8_js` (rapides), sinon justifiée dans le commit. Deux trous sont attendus :
  - l'échec de l'`assert` de `ke_take_string`, exécuté seulement dans les fils de `must_abort` qui avortent sans vider leur compteur (prouvé par le test d'arrêt et la mutation 5) ;
  - l'échec des trois `assert` d'init de module, que la spec ne peut pas atteindre.

  Le rapport va dans le commit.
- **Prior art** : `test_struct_defaults` (`test_pico8_jo_spec_defaults`, identité face à la spec) ; `test_too_far_members_by_name` et `test_too_far_writer_stops_off_row` (1b.13) ; `test_ke_closed` et `must_abort` de `test_pico8_ui` ; `test_mouse_point_members_by_name` (1b.12, lecteur brut et témoin).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_ui` : 10 à 30 s (deux rejeux et des fils). Une douzaine de lancements (ancien code, nouveau code, 7 mutations, `gcov`) en 4 min environ ;
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
2. `timeout 15m make e2e` : **15 « conforme »**, et les 8 TU de `UNIT_TESTS_SLOW` passent. **Aucun n'est modifié**, mais tous exercent le code changé (`test_pico8_ie`, `test_pico8_jo`, `test_pico8_qh`, `test_pico8_os`, `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`, `test_pico8_nr`) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 208`**, sans ligne `pico8_ke_t me` ;
5. `timeout 15m make class-coverage-net` : **sortie identique à `tools/class_coverage_net_expected.txt`**. Elle s'applique parce que le diff touche des constructeurs (`pico8_ke_struct_init`, `pico8_jo_struct_init`, `pico8_qh_struct_init`, `pico8_pm_struct_new`, `pico8_jc_struct_new`, `pico8_mz_struct_new`), sans toucher ni `pico8_class_coverage_record` ni `KNOWN[]`. Aucun compte ne doit changer.

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si `ke_take_string` ou un `assert` d'init de module avorte dans un rejeu ou un TU hors des fils de `must_abort` (H2), si un indicateur `has_me` ou un membre est ajouté, ou si le type de `me` est élargi (union, `pico8_value_t` gardé à côté) au lieu d'avorter.
- `FAILED:` si `me` n'est pas `const char *` à sa place dans `pico8_ke_t`, ou si `t` est typé ou touché autrement que par le déplacement de ses voisins (1b.14b).
- `FAILED:` si une table de défauts, d'offsets ou de clés liste encore `me` (preuves `offsetof` et `{"me",`), si la branche spéciale `"me"` de `pico8_jo_struct_init` reste, ou si une autre entrée de ces tables est retirée ou modifiée.
- `FAILED:` si le défaut de `jo`, `qh` ou `pm` n'est pas le **pointeur de sa spec** (un littéral à sa place), si `g_<X>_me` n'est pas remis à NULL au `shutdown`, ou si `ke`, `jc` ou `mz` perdent leur littéral.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact quand `me` est NULL, ou ne rend pas le même pointeur.
- `FAILED:` si la signature de `pico8_ke_ks` change, si `ks` nil ne met pas `me` à NULL, si l'aide ne refuse pas une chaîne NULL, ou si son message ne nomme pas `ke.me` et `chaine`.
- `FAILED:` si un site par nom change (`dg`, `dl`, `d_`, `jo:hh`, `wormhole.c`, `economy_tiers.c`, lecteurs et écrivains des filles), ou si un site par nom est remplacé par un accès direct.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si le diff d'un test fait autre chose qu'ajouter la caractérisation, le test d'arrêt, `KE_ME_HASH` et leurs appels à `test_pico8_ui`, et changer la seule ligne `ke.me` de `test_pico8_iv`, ou si une valeur figée existante change (les deux `STRUCT_HASH`, `DRAG_REPLAY_HASH`, `TECH_REPLAY_HASH`, `LL_DG_CALLS`…), ou si un test de régression (§6) ou un TU de `UNIT_TESTS_SLOW` change.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 208, liste encore `pico8_ke_t me`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` s'il manque une ligne « Runs » de `make class-coverage-net` identique à la référence, ou si le diff touche `pico8_class_coverage_record` ou `KNOWN[]`.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit ou écrit `me` directement, si elle ne couvre pas les 24 genres, ou si `KE_ME_HASH` change après le typage ; si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'ancien code et une verte après.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 7 mutations, chacune avec son test ou la justification de son trou.
- `FAILED:` si le travail est découpé en plusieurs commits dont un seul est rouge, ou si 1b.14b (`ke.t`) y est mêlé.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.14a » ou si elle ne donne pas la lecture de R7 (points 1 à 5), ou s'il manque le tableau « Runs ».

## Out of Scope

- `ke.t` (1b.14b) : l'écriture directe `((pico8_ke_t *)qc)->t` de `dg`, la clé `{"t", offsetof(pico8_g__t, ke.t)}` de `g_`, la remise à 0 de `ks`, et le sélecteur local de `pico8_ke_field`.
- Les autres membres des tables `g_jo_defaults`, `g_qh_defaults` et `g_pm_defaults` (1b.23, 1b.24, 1b.27), et toute autre table des filles.
- `ie_ke.me` (1b.11, fait).
- Les lecteurs de spec `pico8_X_default` et les specs `ob` (phase D).
- Changer la signature de `ks`, ou remplacer une lecture par nom de `me` (`dg`, `dl`, `d_`…) par un accès direct.
- Régénérer `tools/member_census.txt`.
- Les fichiers d'événements et `FRAMES_EVENTS`, tout changement de rendu, l'audio, lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - modules : `ke` (type, lecteur, `ks`), et les trois modules à défaut propre (`planet.c`, `planet_family.c`), plus les poses de `jc`/`mz` et les lectures directes désignées par le compilateur. Un seul commit, **sans découpage** en 1b.14a1/a2 (10 fichiers modifiés, un membre ; une coupe laisserait un arbre incompilable) ;
  - tests : `test_pico8_ui` étendu (caractérisation sur les 24 genres, test d'arrêt rouge d'abord). Pas de nouveau TU, pas de nouveau TU lent. `test_pico8_iv` retouché d'une ligne ;
  - `me` → `const char *` emprunté, sans `has_me` ;
  - R7 : entrée retirée des trois tables, défaut déballé une fois à l'init de module dans un `static const char *`, même pointeur que la spec, remis à NULL au `shutdown`, posé après la boucle de la table ;
  - R5 : `ks` garde sa signature par valeur. Aide `ke_take_string` de forme 1b.13, nil accepté, chaîne NULL refusée ;
  - `class-coverage-net` lancé.
- **Note « Découvert à 1b.14a »**, sur la ligne 1b.14a de `docs/plan-3.md` : « `me` → `const char *` emprunté, sans indicateur ; `pico8_ke_field` est le seul lecteur par nom de `me` pour les 24 genres ; **R7, lecture nouvelle : une table de défauts qui garde des entrées et a des filles** : l'entrée du membre typé sort de toutes les tables qui la listent, quel que soit leur format (nom/valeur de `jo`, avec sa branche spéciale ; `offsetof` de `qh` et `pm`), et la table reste avec ses autres entrées ; chaque module qui avait l'entrée déballe **une fois, à son init**, le défaut de sa spec dans un `static <type> g_<X>_<m>` (même pointeur, `assert` du genre de la ligne), le remet à nil dans son `defaults_reset`, et le pose par affectation typée dans son `struct_init`, après la boucle de la table ; les filles sans défaut propre héritent par la chaîne d'init et leurs tables ne sont pas touchées. R5 sur une méthode de la parente (`ks`) : aide par valeur de 1b.13, signature gardée. Empreinte réelle 37 (11, 26), 10 fichiers modifiés. » Le reste va dans le commit. 1b.14b, 1b.23 (`jo`, 9 filles) et 1b.32 (`g_`, fille `fv`) recopient cette forme.
- Le message de commit porte le périmètre, le compte 209 → 208, le tableau des 24 genres (valeur et origine du pointeur), le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous, `KE_ME_HASH` et les réponses à H1 à H12.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
