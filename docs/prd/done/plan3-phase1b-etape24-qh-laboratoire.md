---
title: "Plan 3, étape 1b.24 : `qh`, le laboratoire. Ses 10 membres typés, `et` en pointeur d'en-tête plus `has_et` (false = présent et NULL), `qh` en fixe plus `qh_is_bool`, les tables de `qh` et d'`os` supprimées"
description: "`pico8_qh_t` a 10 membres `pico8_value_t`. Tous sont des défauts de spec lus par nom, sauf `et`, la cible du glissé que `bv` écrit. 1b.24 les type selon le patron R1–R9 de 1b.1. Deux genres mêlés sont tranchés ici. `et` ({bool, REF vers 11 classes}, où seul `false` est jamais écrit) devient un `pico8_struct_header_t *` plus `has_et`, lecture de 1b.7 posée sur la forme à plusieurs cibles de 1b.22. `qh` ({bool, fixe}, jamais nil) devient un `pico8_fixed_t` plus `qh_is_bool`, lecture de 1b.23a1 (`hf`). Les tables `g_qh_members`/`g_qh_defaults` et la table `g_os_defaults` se vident et sont supprimées. Les défauts des 8 genres sont déballés en `static` (R7 de 1b.14a). `make value-members` passe de 138 à 128. Un seul commit (12 fichiers)."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.24 — `qh` (`planet_family.h:34`), genres mêlés `qh`, `et` (bool ou REF vers 8 typedefs, dont `bv_et`, 1b.22). Empreinte 21 (8, 13). Compte : 128.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité, pas redécidé. Les notes « Découvert à » de `docs/plan-3.md` s'appliquent. Elles sont citées ici, pas redérivées :

- **1b.1, puis 1b.3 à 1b.23c** : patron appliqué, dont :
  - **1b.4b** : les fils forkés vident le recensement, et les boucles sentinelles sont arrêtées par `PICO8_MEMBER_CENSUS_STOP()` ;
  - **1b.9 et 1b.10** : R7 forme `offsetof`. Une table vidée est supprimée avec son seul client, l'accesseur `pico8_value_t *`. Preuves `grep -rnw 'X_member\|g_X_members' src/` et `offsetof(pico8_X_t` vides ;
  - **1b.14b** : les cas à union nulle se construisent par `memset` puis `kind`. Une mutation qui retire l'unique appel d'une aide `static` se plante sous `if (0)` (`-Werror`) ;
  - **1b.19** : un fixe sans indicateur écrit par nom a une aide sans état nil, et nil y avorte (`qb_take_fixed`) ;
  - **1b.20** : un `assert` de lecture passe à l'écriture ;
  - **1b.23a1** : pour un genre mêlé {bool, fixe} à nil `jamais` (`hf`), on garde `pico8_fixed_t <m>` plus `int <m>_is_bool`, sans indicateur, avec un emballage public unique. Les aides de lecture de spec vont par genre et par fichier (`family_default_*`, `family_trade_default_*`), et leur message nomme leurs membres. H11 démentie : les valeurs figées sont celles **observées** ;
  - **1b.23a2** : un membre `l` seul que l'écrivain de 1a accepte garde sa branche. **Boucles sentinelles** : un membre typé sort des tableaux `written[]` des TU. H16 démentie : le grep §6 couvre `planet->m` ;
  - **1b.23b** : les aides d'écriture vont par genre et par fichier (`family_take_vec`, `family_trade_take_vec`). §6 : le grep couvre tous les TU ;
  - **1b.23c** : la clé `id` passe dans les dix constructeurs (`family_take_string`) ;
  - **1b.23d** (PRD en triage, supposée commitée, H10) : `jo` est entièrement typé, et `g_jo_members`/`g_jo_defaults` sont supprimées. **Un tableau `written[]` vidé est supprimé avec sa boucle.** `g_qh_mi` est déballé, `pico8_pf_truthy` est retirée.
- **1b.22 — REF à plusieurs cibles**, recopiée pour la partie REF d'`et` :
  - **pointeur d'en-tête plus `assert` à prédicat** : `ref:` à plusieurs cibles → `pico8_struct_header_t *<m>` (ni union, ni `void *`, ni base `ke`). L'ensemble est porté par l'`assert` de l'aide R5 : genre REF, tag `STRUCT`, puis prédicat `static` sur `cls` ;
  - **lecture Coexistence « ligne ∪ producteurs prouvés »** : l'ensemble admis est la ligne ∪ les classes productrices prouvées par le code **et** exercées par la caractérisation dirigée sur l'ancien code ;
  - **prédicat** par liste d'accesseurs publics quand tous sont liés partout où le fichier l'est ; sinon par une propriété du descripteur que l'ensemble partage exactement.
- **1b.13 et 1b.15 — forme écrivain de R5** :
  - une branche `strcmp` par membre à `e`, dans l'ordre du typedef ;
  - une aide `static` par genre, jamais partagée entre fichiers ;
  - l'aide pose **d'abord** l'état nil, puis sort sur `REF NULL` (genre **et** pointeur testés). Sinon, un `assert` « champ sans règle » qui nomme `struct.membre`, le genre de sa ligne et l'arrêt ;
  - un non-pointeur sans indicateur n'a pas d'état nil, et nil y avorte ;
  - aide à indicateur (`bg_take_bool`) : nil éteint `has_` quel que soit l'état d'avant, un `false` à union nulle reste un `false` présent, et `has_ = 1` est posé en dernier ;
  - un nom sans branche donne `return 0`, et l'arrêt reste l'`assert(written && …)` de `pico8_entity_set_field` ;
  - **cas d'arrêt `memset`** : chaque test d'arrêt porte les non-REF hors genre à union entièrement nulle (fixe 0, chaîne NULL, fonction NULL, et `false` quand `false` est hors genre), construits par `memset` puis `kind` ;
  - un `must_abort` nouveau, rouge sur l'ancien code, fige l'arrêt.
- **1b.11 — règle R2 des TU lents** : une lecture par nom faite seulement par un TU de `UNIT_TESTS_SLOW` compte comme `l`, et le nil qu'elle lit compte comme `lu`. Le PRD relit donc les TU lents de l'arbre `qh` (revue plus bas). **`test_pico8_qh` est lent** : il est dans `UNIT_TESTS_SLOW`, et `make test` ne le lance pas.
- **1b.5, 1b.7, 1b.16 — lectures de R1 pour les genres mêlés** :
  - 1b.5 : {fixe, bool} → `pico8_fixed_t` + `has_` + `<m>_is_bool` ;
  - 1b.7 : {bool, chaîne} dont le seul booléen écrit est `false` → pointeur + `has_`, où NULL présent veut dire `false`, sans `<m>_is_bool`. **1b.7 laisse ouverte « la forme quand `true` peut être écrit (`qh.et`, `g_.et`, `mz.et`…) »** ;
  - 1b.16 : {fixe, `ref:pico8_X_t`} dont le seul fixe est le défaut de la spec → pointeur, NULL = ce fixe.

  Ce PRD tranche `qh.et` et `qh.qh` sur ces trois lectures (voir « Décisions »).
- **1b.14a — R7 sur une table qui garde des entrées et a des filles** :
  - l'entrée sort de **toutes** les tables qui la listent, tables des filles comprises ;
  - chaque module déballe une fois, à son init, le défaut de sa spec dans un `static g_<X>_<m>`. Un défaut REF garde le même pointeur ;
  - le `static` est remis à nil dans `defaults_reset` ;
  - la pose typée se fait après la boucle de la table, avant les clés et tout `pq`.

  **`qh` a 7 filles qui listent ses membres : `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`.**

### État de départ

HEAD `3cafeb7` (PRD de 1b.23d). **1b.23d est en cours d'implémentation dans l'arbre de travail.** Précondition : elle est commitée avant que 1b.24 commence (H10). Après elle, `pico8_jo_t` ne contient plus de `pico8_value_t`, et `make value-members` rend **138** (H1). Les numéros de ligne cités sont ceux de l'arbre 1b.23d et glisseront. Chaque site est aussi désigné par son contenu.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H20).

### Lignes de recensement (`tools/member_census.txt:88-97`)

| membre | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|
| `hp` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `px` | `ref:table` | jamais | - | l | `pico8_ob_table_t *` | non (pointeur) |
| `dq` | `chaine` | jamais | - | l | `const char *` | non (pointeur) |
| `qh` | `bool,fixe` | jamais | - | l | `pico8_fixed_t` + **`int qh_is_bool`** | non (1b.23a1) |
| `od` | `fixe` | observé | stat? | l | `pico8_fixed_t` | **non** (voir R2) |
| `fp` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `dw` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `s` | `fixe` | observé | stat? | l | `pico8_fixed_t` | **non** (voir R2) |
| `ba` | `ref:table` | jamais | - | l | `pico8_ob_table_t *` | non (pointeur) |
| `et` | `bool`, `ref:` `bv_et`, `fr`, `hj`, `jc`, `jo`, `mx`, `pm`, `qh` | observé | lu,stat? | **le** | `pico8_struct_header_t *` + **`int has_et`** | **oui** (voir « `et` ») |

Le compte passe à 138 − 10 = **128**. Les `has_`/`_is_bool` ne comptent pas (R9). **Aucune ligne `pico8_qh_t` ne reste.**

- **R2 pour `od` et `s`** (lecture de 1b.7). Leur seule preuve de nil est `stat?` : c'est un test de vérité sur un nom que portent plusieurs typedefs (`dl` : `elseif self.od` ; `jo:kj` : `self.s or 43`, qui vise un `jo` nu, sans `s`). Sur un `qh`, les deux sont toujours tenus :
  - chaque spec de la chaîne les définit (`qh` 15/174, `nl` 20/68, `os` 15/100, `hj` 60/166, `pm` −/98, `nr` 10/170, `fr` 25/78, `mx` 40/66) ;
  - `od` n'est jamais écrit ;
  - `s` ne l'est que par `nr:hb`, en fixe, et par l'écrivain de `nr` dans les TU, en fixe (H2).
  
  La revue des TU lents ne trouve aucune lecture nil par nom (plus bas). Donc **pas d'indicateur** : le cas de 1b.7 où un `stat?` ne suffit pas.
- **`et` seul a `e`**. Ses seuls écrivains sont `pico8_bv`, par nom (`probe.c`, `pico8_entity_set_field(self, "et", et)`), et les TU. L'écrivain de `nr` accepte aussi `s`, en `l` seul : la branche reste (1b.23a2, point 1).

### Revue des TU lents (R2 de 1b.11)

Relus pour les 10 noms : `test_pico8_qh`, `os`, `hj`, `pm`, `mx`, `nr` (arbre `qh`), `test_pico8_jo` et `test_pico8_ie`. Constats :

- **Listes `own[]`/`qh_members[]`** (les 10 noms) et **`scalars[]`**, pliées par `fold_field` (présence, genre, valeur) sur des planètes neuves et posées. Les 9 défauts y sont toujours présents.
- **`et`, lectures nil par nom** : c'est la preuve `lu` de la ligne, et la raison de `has_et`.
  - `bool_state(e, "et")` (`qh:410,499`, `os:486,516`, `hj:505,591`, `nl:437,475`) rend 0 (absent), 1 (`false`) ou 3 (REF). `test_life` (`qh:442-456`) atteint les trois : 3 hors du panneau (la cible jetable), la planète visée, et 1 dans le panneau ;
  - `has(e, "et")` (`nr:552`) ;
  - `not_pm[]`, `not_mx[]`, `not_nr[]`, `not_fr[]` ∋ `et` : absent sur une fille neuve.
- **`qh`** : `bool_state(e, "qh") == 1` (`hj:552`), soit `false` présent.
- **Identité des tables** : `get_ref(e, "px") == class_ref(pico8_X_default, "px")` (`os:320`, `hj:354`, `pm:336`, `mx:343`) et `get_ref(e, "ba") == class_ref(pico8_qh_default, "ba")` (`os:361`, `hj:405`, `pm:422`, `mx:406`). D'où le **pointeur de spec** pour `px`/`ba`.
- **`inherited[]`**, avec `pico8_value_equal` par rapport à la spec : `qh:549` (`dq`, `fp`, `dw`, …), `nr:688` (`od`, `fp`, `s`, `dw`), `fr:557`, `pm:686`, `mx:743`.
- **`s` de `nr`/`fr`** : `fold_field(placed, "s")` et `get(fr, "s") == 78` (après `hb`).
- **Arrêts gardés** : `must_abort(write_hp/s/od, qh)`, car l'écrivain de `qh` refuse ces noms, et `must_abort(write_et, jo nu)`.
- **Boucles sentinelles `written[]`** (arbre 1b.23d) :
  - `qh` `{et}`, `os` `{et}`, `hj` `{et, qy}`, `pm` `{et, qy}`, `mx` `{et, qy, ht}` et `nr` `{et, s}` (6 lents) ;
  - `nl` `{et, qw}` et `fr` `{et, s}` (rapides).

  Chaque boucle écrit `pico8_value_fixed(i + 1)` et relit `FIXED`. Une fois typé, `et` avorte sur un fixe (voir « Boucles sentinelles »).
- `test_pico8_jo` et `test_pico8_ie` : aucun des 10 noms sur l'arbre `qh`. `ie` lit `s`/`od` sur sa vue de genre, homonymes de 1b.11.

**R2 ne donne aucun indicateur de plus que `has_et`.**

### Ce que fait le cart (`pico8/lua.lua`)

- **Défauts** (`qh=jo:jr[[…]]` `:905-915`, et les filles `:998-1110`) :
  - `hp`, `px`, `dq="ly"`, `qh=1`, `od`, `fp=2`, `dw=0.025`, `s` et `ba` pour `qh` ;
  - redéfinis par les filles comme suit :

| fille | redéfinit |
|---|---|
| `nl` | `qh=f`, `hp`, `s`, `px`, `od`, `ba` |
| `os` | `hp`, `px`, `qh=f`, `s`, `od` |
| `hj` | `hp`, `qh=f`, `px`, `od`, `s` |
| `pm` | `hp`, `px`, `s` |
| `nr` | `od`, `fp=1`, `s`, `dw=0.01` |
| `fr` | `od`, `s` |
| `mx` | `hp`, `s`, `px`, `od` |

- **`et`** : `bv` (`:552-558`) écrit `self.et = nq<self.dw and c or jp(cg.jc,…)[1] or not ui.hw(rp) and {}`, puis `if (self.et)self.et.fo=self`.
  - `c` est la planète la plus proche de `cg.np`. `ij` (`:351`) saute l'appelant par identité de `p`.
  - Le second terme est un `jc` de `cg.jc`.
  - Le dernier terme vaut `{}` hors du panneau, et **`false`** dedans (`not true and {}`).
  - **`true` n'est jamais écrit, et nil non plus** : `bv` pose toujours un de ces termes. Nil n'est tenu qu'avant le premier `hq`.
  - Lu par `dl` (`if self.et`) et `d_` (`if not self.et`), en vérité seulement.
- **`cg.np`** : le tag `np` est dans le `nu` de `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `mx` et `nr`. `fr` hérite celui de `nr`. `ga` a `nu=o()` et n'y est jamais.

### Le code (arbre 1b.23d supposé commité)

- **Typedef** (`planet_family.h`) : `{ pico8_jo_t jo; hp; px; dq; qh; od; fp; dw; s; ba; et; }`, tous en `pico8_value_t`. Les filles l'enchâssent : `nl`, `os`, `hj` en `qh` ; `pm`, `mx` en `hj.qh` ; `nr` en `qh` ; `fr` en `nr.qh`.
- **Tables d'offsets** : **48 entrées** `offsetof(pico8_<qh|nl|os|hj|pm|nr|fr|mx>_t, …<m>)` sur ces 10 noms.

| fichier | table | entrées sur les 10 noms |
|---|---|---|
| `planet_family.c` | `g_qh_members` | les 10 |
| `planet_family.c` | `g_qh_defaults` | 9 (`mi` sorti en 1b.23d) |
| `planet_family.c` | `g_nl_defaults` | `qh`, `hp`, `s`, `px`, `od`, `ba` |
| `planet_family.c` | `g_os_defaults` | `hp`, `px`, `qh`, `s`, `od` (toute la table) |
| `planet_family.c` | `g_hj_defaults` | `hp`, `qh`, `px`, `od`, `s` |
| `planet_family.c` | `g_pm_defaults` | `hp`, `px`, `s` |
| `planet_family_trade.c` | `g_nr_defaults` | `od`, `fp`, `s`, `dw` |
| `planet_family_trade.c` | `g_fr_defaults` | `od`, `s` |
| `planet_family_trade.c` | `g_mx_defaults` | `hp`, `s`, `px`, `od` |

- **Clients des tables de `qh`** :
  - `pico8_qh_member`, le lecteur `pico8_qh_field` et son repli sur celui de `jo` ;
  - dans `pico8_qh_struct_init`, la boucle nil sur `g_qh_members` et la boucle de recopie de `g_qh_defaults` ;
  - dans `pico8_planet_family_init`, la boucle de lecture de spec et son `assert` « every default … » ;
  - la boucle de remise à nil de `pico8_qh_defaults_reset`.

  **Clients de `g_os_defaults`** : la boucle de `pico8_os_struct_new`, la lecture de spec de l'init et la remise à nil.
- **Écrivain de `qh`** (`pico8_qh_set_field`) : `et` (`qh->et = value`, tout genre accepté), puis `ra`/`p` (`family_take_vec`), puis `jo`. `os` a le même lecteur et le même écrivain que `qh`. Les écrivains de `nl`, `hj`, `pm`, `mx` et `fr` délèguent à celui de `qh` après leurs propres noms. Celui de `nr` ajoute **`s`** (`((pico8_nr_t *)self)->qh.s = value`, tout genre), avant de déléguer.
- **Écritures directes (R6)** : `nr:hb` (`self->qh.s = pico8_value_fixed(s_val)`, `planet_family_trade.c`). Aucune autre. **Aucune lecture directe** des 10 membres dans `src/` ni dans `tests/` : `grep -rnE 'qh(\.|->)(hp|px|dq|qh|od|fp|dw|s|ba|et)\b'` ne rend que les deux écritures de `s` et celle d'`et`.
- **Sites par nom, inchangés** (puce Coexistence) :

| fichier | noms lus ou écrits par nom |
|---|---|
| `probe.c` | `bv` (`dw`, `et` écrit), `dl` (`et`, `od`), `d_` (`et`, `od`, `dq`, `fp`) |
| `planet.c` | `hp` |
| `planet_display.c` | `px`, `qh`, `s` |
| `planet_trade.c` | `ba` |
| `economy_score.c` | `qh` |
| les TU | voir la revue des TU lents |

- **Homonymes hors étape** :
  - `od`, `fp`, `dq`, `dw` de `g_`, `mz`, `jc` et de la vue `ie_ke` ;
  - `et` de `g_` (1b.32), `jc` (1b.35), `mz` (1b.36) ;
  - `hp` (`pico8_hp_t`, 1b.9) ;
  - `px` des projets et des lignes de menu ;
  - `s` de `ie_ke` ;
  - `qh`, le tag de `cg`.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (arbre 1b.23d commité). Elle lit par nom les 10 membres sur `qh` et ses 7 filles, neuves et vivantes. Elle mène `bv` vers chacune des 11 cibles admises, y compris `nl`, `os`, `nr` (absentes de la ligne), vers `false` et vers la cible jetable, puis fait l'aller-retour par nom d'`et`, de `s` (`nr`, `fr`) et de `nr:hb`. Elle doit être verte avant tout typage.
2. **Test d'arrêt, rouge sur l'ancien code** : l'écriture hors genre d'`et` (union nulle, `true`, non-structs, structs refusées) et de `s` (nil, union nulle).
3. **Commit 1b.24** :
   - les 10 membres sont typés, avec `has_et` et `qh_is_bool` ;
   - `g_qh_members`, `g_qh_defaults`, `pico8_qh_member` et `g_os_defaults` sont supprimés avec leurs boucles ;
   - 24 entrées sortent des tables de `nl`, `hj`, `pm`, `nr`, `fr` et `mx`, et les 5 d'`os` partent avec sa table ;
   - les défauts sont déballés en `static` dans les deux fichiers ;
   - on ajoute les emballages `pico8_qh_qh`/`pico8_qh_et`, les aides `family_take_target` (prédicat `family_is_target_class`) et `family_trade_take_fixed`, et la réécriture de `nr:hb` ;
   - `et` sort de 8 tableaux `written[]`, et `s` de 2. Ceux de `qh`, `os`, `nr` et `fr`, vidés, sont supprimés avec leur boucle.

   `make value-members` rend **128**, dont **0 ligne `pico8_qh_t`**.
4. Aucun pixel ne bouge. La note « Découvert à 1b.24 » va sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `planet_family.h`, je veux `pico8_qh_t` sans aucun `pico8_value_t`, pour lire un laboratoire sur ses types.
2. En tant que lecteur, je veux :
   - `hp`, `od`, `fp`, `dw`, `s` typés `pico8_fixed_t` ;
   - `px`, `ba` typés `pico8_ob_table_t *` ;
   - `dq` typé `const char *` ;
   - `qh` typé `pico8_fixed_t` avec `qh_is_bool` ;
   - `et` typé `pico8_struct_header_t *` avec `has_et`.
3. En tant que lecteur par nom (`probe.c`, `planet*.c`, `economy_score.c`, les TU), je veux que chacun des 10 noms rende exactement ce que rendait l'ancien code sur les 8 genres de l'arbre : présence, genre, valeur, identité du pointeur.
4. En tant que planète neuve de l'arbre `qh`, je veux les défauts de ma spec, ou de celle de mon ancêtre le plus proche, avec `px`/`ba` au **même pointeur** que la spec, et `et` absent.
5. En tant que `nl`, `os`, `hj`, `pm` ou `mx`, je veux `qh` lu `false` présent. En tant que `qh`, `nr` ou `fr`, je le veux lu `FIXED` 1.
6. En tant que `bv`, je veux écrire `et` = la planète visée (l'une des 9 classes de `cg.np`), le `jc` visé, la cible jetable, ou `false`, et le relire au même pointeur ou `BOOL` 0 présent.
7. En tant que `dl`/`d_`, je veux lire `et`, `od`, `dq`, `fp` par nom comme avant : le prix payé (`od`) et l'état suivant (`dq`) inchangés.
8. En tant que `nr:hb`, je veux poser `s` depuis `hz` ; en tant qu'écrivain de `nr`, accepter `s` fixe par nom.
9. En tant que gardien d'`et`, je veux refuser `true`, tout non-struct, et toute struct hors des 11 classes (`ga`, `g_`, `fv`, `mz`, `bg`, `ok`, un `hp`, `mz_jj`), avec un arrêt « champ sans règle » qui nomme `qh.et`.
10. En tant que gardien de `s`, je veux que nil et tout non-fixe avortent (fixe jamais nil, forme 1b.19).
11. En tant que mainteneur, je veux que l'ensemble des noms qu'acceptent les écrivains ne change pas : `qh` 20, `nr` 21.
12. En tant que mainteneur, je veux qu'aucune table d'offsets de l'arbre ne liste plus un membre de `qh`, et que les tables vidées disparaissent.
13. En tant que TU à boucle sentinelle, je veux que ma boucle ne réécrive plus `et`/`s` en fixe.
14. En tant que test de régression `empty_planet_cleared`, qui pose un laboratoire sur une planète vide (`dw`, `et` → `jo`, `od` payé), je veux rester vert, et rouge quand mon bug revient.
15. En tant qu'auteur de 1b.32 (`g_.et`) et 1b.36 (`mz.et`), je veux une forme écrite pour {bool, REF à plusieurs cibles} dont le seul booléen est `false`.
16. En tant qu'auteur des étapes des filles (1b.25–1b.31), je veux leurs tables débarrassées des membres de `qh`, et les `static` de défaut à leur nom.
17. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
18. En tant que gardien de la mémoire et de la couverture, je veux `make sanitize-net` à 15 propres et `make class-coverage-net` identique à sa référence.
19. En tant que `plan-pilot`, je veux `make value-members` à 128 et `pico8_qh_t` absent de sa sortie.
20. En tant que vérificateur, je veux :
    - la caractérisation verte sur l'ancien code ;
    - le test d'arrêt rouge puis vert ;
    - les preuves rouges §6 ;
    - les mutations et leurs trous justifiés ;
    - le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules et les tests. Ces choix sont tranchés comme le ferait un agent `guideline` : hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables. Ils sont marqués (G).

### `et` : `pico8_struct_header_t *et` + `int has_et`, `false` = présent et NULL (G)

**États à représenter**, tirés du cart, du C et des TU lents :

- nil, tenu avant le premier `hq` et lu par nom (preuve `lu`, `bool_state` = 0) ;
- `false`, écrit par `bv` dans le panneau, lu `BOOL` 0 **présent** (`bool_state` = 1) ;
- une REF vers l'une des 11 classes.

**`true` n'est jamais écrit**, ni par le cart (`not ui.hw(rp) and {}`), ni par le C (seul `pico8_value_bool(0)` dans `pico8_bv`), ni par un TU (seul `write_et` écrit `pico8_value_bool(0)`, sur un `jo` nu qui le refuse ; H3).

| option | verdict |
|---|---|
| **NULL = `false`, sans indicateur** | **rejetée**. Nil est tenu et lu par nom : la ligne dit `lu`, et `bool_state` = 0 dans six TU lents. Nil et `false` se confondraient : une lecture par nom qui rendait 0 rendrait `BOOL` 0 présent (ou l'inverse), contre la puce Coexistence (« absent exactement là où 1a rendait 0 »). Les plis des TU lents changeraient. |
| **`et` + `has_et` + `et_is_bool`** | **rejetée** : `et_is_bool` vaudrait toujours `has_et && et == NULL`, puisque `true` n'existe pas. C'est l'argument de 1b.7 (« pas de `<m>_is_bool`, il vaudrait `<m> == NULL` »). Un état `true` représentable élargirait le membre. |
| union étiquetée, `void *`, pointeur d'une cible | **rejetée** (1b.22). |
| **`pico8_struct_header_t *et; int has_et;`** | **retenue**. C'est la lecture de 1b.7 ({bool, X} dont le seul booléen écrit est `false` → pointeur + `has_`, NULL présent = `false`), posée sur la forme à plusieurs cibles de 1b.22. `has_et` = 0 : nil (`et` NULL, non lu). `has_et` = 1 et `et` NULL : `false`. Sinon, la cible, empruntée, jamais possédée ni libérée. `true` est l'arrêt « champ sans règle », comme dans `hp_take_il` (1b.9). |

C'est une **lecture nouvelle**, notée dans « Découvert à 1b.24 ». Elle répond à la question laissée ouverte par 1b.7 pour `qh.et` : ce n'est pas le cas « `true` écrit ». `g_.et` (1b.32) et `mz.et` (1b.36) la relisent sur leur propre preuve.

**Emballage public unique** (1b.7), `pico8_value_t pico8_qh_et(const pico8_qh_t *self)`, qui rend :

- `REF NULL` si `self` est NULL ou si `has_et` vaut 0 ;
- `pico8_value_bool(0)` si `et` est NULL ;
- `pico8_value_ref(et)` sinon.

Il est appelé par le lecteur. Aucune lecture directe d'`et` n'existe dans `src/`.

### Ensemble admis d'`et` : 11 classes (lecture Coexistence de 1b.22) (G)

- **Ligne** : `bv_et`, `fr`, `hj`, `jc`, `jo`, `mx`, `pm`, `qh` (8).
- **∪ producteurs prouvés par le code** : `nl`, `os`, `nr`. Ils sont dans `cg.np` (leur `nu`, ou celui de `nr` pour `fr`), donc `c` de `ij(self.p, cg.np)` peut les rendre. Aucun rejeu recensé n'a posé un laboratoire sur l'un d'eux. **Tous trois sont exercés par la caractérisation sur l'ancien code** : `hq` d'un `qh` neuf, visant une planète neuve de cette classe, puis aller-retour par nom.
- **Soit 11 classes** : `jo` et les 8 genres de l'arbre `qh` (les 9 classes de `cg.np`), plus `jc` et `bv_et`.
- **Refusés et figés par le test d'arrêt** : `ga` (pas de `np`), `g_`, `fv`, `mz`, `bg`, `ok`, un `hp`, `mz_jj`, et tout non-struct.

**Prédicat** (tiré de la règle de 1b.22, pas prototypé) :

```c
static int family_is_target_class(const pico8_struct_class_t *cls) {
    return cls->methods[PICO8_METHOD_LY] != NULL || cls == pico8_jo_struct_class() ||
           cls == pico8_jc_struct_class() || cls == pico8_bv_et_struct_class();
}
```

- **Pourquoi pas une liste d'accesseurs** : `planet_family.c` est lié par `DATA_ENGINE_SRCS`, qui n'a pas `planet_family_trade.c` (`pico8_nr/fr/mx_struct_class`). C'est le piège de 1b.22.
- **Propriété du descripteur partagée exactement** : `methods[PICO8_METHOD_LY] != NULL` désigne exactement les 8 descripteurs de l'arbre `qh`. `grep -rn '\[PICO8_METHOD_LY\] = pico8_' src/*.c` rend 8 lignes : `qh_ly` ×7, `nl_ly` ×1 ; `planet_family.c` 5, `planet_family_trade.c` 3. `ly` est la méthode de `qh` (1a.18), héritée par toutes ses filles. `jo`, `ga`, `g_`, `fv`, `mz` n'en ont pas (H6). Le prédicat recopie la règle du cart : `cg.np` = `jo` ∪ les descendants de `qh`.
- **Rejetée** : `methods[HQ] == pico8_bv_dispatch`. Elle admettrait `g_` et `fv` et refuserait `jo`.
- **Liens** : `pico8_jo_struct_class` (`planet.c`), `pico8_jc_struct_class` (`slipway.c`) et `pico8_bv_et_struct_class` (`probe.c`) sont liés partout où `planet_family.c` l'est (`ENGINE_SRCS`, `DATA_ENGINE_SRCS`, `main_raylib`, H7). **Un seul `#include` ajouté** : `pico8_slipway.h`.

### Aide R5 d'`et` : `family_take_target` (G)

`static void family_take_target(pico8_value_t value, pico8_struct_header_t **out, int *has)`. Elle est nommée par son rôle, comme en 1b.22 : glossaire, `et` = « cible courante du glissé ». Sa forme suit `hp_take_il` (1b.9) et `bv_et_take_holder` (1b.22) :

1. `*out = NULL; *has = 0;`
2. `if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) return;`. Genre **et** pointeur sont testés.
3. `assert(((value.kind == PICO8_VALUE_BOOL && value.as.boolean == 0) || (value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT && family_is_target_class(((const pico8_struct_header_t *)value.as.ref)->cls))) && "…")`. Le tag est vérifié avant de lire `cls`.
4. `*out = value.kind == PICO8_VALUE_REF ? value.as.ref : NULL;` puis `*has = 1;`, en dernier.

**Message** : `"qh.et: a write neither nil, false, nor what bv aims at (a planet jo, qh, nl, os, hj, pm, nr, fr, mx, a jc, or bv's throwaway target) is off its census row (bool,ref: to those targets) -- a field under no rule of plan-3 section 5: stop, do not widen the member"`.

Un `false` à union nulle (`memset`, `kind = BOOL`) est un `false` présent : c'est le témoin accepté. `true` avorte.

Un seul appelant : la branche `et` de `pico8_qh_set_field` (`family_take_target(value, &qh->et, &qh->has_et)`). L'ordre des branches est gardé : `et`, puis `ra`, `p`, puis `jo`.

### `qh` : `pico8_fixed_t qh` + `int qh_is_bool`, lecture de 1b.23a1 (`hf`) (G)

Genres {bool, fixe}. Le seul booléen tenu est le `f` des specs de `nl`, `os` et `hj`, hérité par `pm` et `mx`. Le seul fixe est le `1` de `qh`, hérité par `nr` et `fr`. Nil `jamais`, et jamais écrit.

C'est la forme de `jo.hf` (1b.23a1), sans écart : `pico8_fixed_t qh` + `int qh_is_bool`, lu toujours, sans indicateur. L'emballage public unique `pico8_value_t pico8_qh_qh(const pico8_qh_t *self)` rend :

- `REF NULL` si `self` est NULL ;
- `pico8_value_bool(qh != 0)` si `qh_is_bool` ;
- `pico8_value_fixed(qh)` sinon.

Il est appelé par le lecteur.

Rejetée : la forme 1b.16 (NULL = défaut), parce que ce n'est pas un pointeur. Rejetée aussi, la forme 1b.5 avec `has_qh`, parce que nil est `jamais` et qu'aucune lecture nil n'existe.

### R1–R9 appliquées

| règle | sur `qh` |
|---|---|
| R1 | Tableau de recensement, nom et place gardés ; `qh_is_bool` juste après `qh`, `has_et` juste après `et`. Pointeurs empruntés : `px`/`ba` à la spec, `dq` à la spec, `et` à sa cible. |
| R2 | `has_et` seul (plus haut). |
| R3 | `pico8_qh_struct_init`, après `pico8_jo_struct_init`, pose les 9 défauts typés de `qh` depuis ses `static` (`hp`, `px`, `dq`, `qh`/`qh_is_bool`, `od`, `fp`, `dw`, `s`, `ba`), puis `et = NULL`, `has_et = 0`. Ces poses remplacent la boucle nil et la recopie. Chaque fille repose ses propres défauts juste après l'appel de `pico8_qh_struct_init` (ou de `pico8_hj_struct_init`/`pico8_nr_struct_init`), à l'endroit de sa boucle de table, avant les clés et tout `pq` : `nl` (`qh`/`qh_is_bool`, `hp`, `s`, `px`, `od`, `ba`), `os` (`hp`, `px`, `qh`, `s`, `od`), `hj` (`hp`, `qh`, `px`, `od`, `s`), `pm` (`hp`, `px`, `s`), `nr` (`od`, `fp`, `s`, `dw`), `fr` (`od`, `s`), `mx` (`hp`, `s`, `px`, `od`). |
| R4 | `pico8_qh_field` : 10 branches `strcmp` dans l'ordre du typedef, puis `return pico8_jo_struct_class()->field(self, name, out_value);`. `hp`, `od`, `fp`, `dw`, `s` rendent toujours `FIXED`. `px`, `ba`, `dq` rendent `return 0` sur NULL, sinon REF/`STRING` au même pointeur. `qh` passe par `pico8_qh_qh`, `et` par `pico8_qh_et` (`REF NULL` → `return 0`, `*out_value` intact). |
| R5 | Écrivain de `qh` : `et` par `family_take_target`. Écrivain de `nr` : `s` par `family_trade_take_fixed` (`planet_family_trade.c`), forme `qb_take_fixed`/`jo_take_ma` : pas d'état nil, `assert(value.kind == PICO8_VALUE_FIXED && "qh.s: a write that is not a fixed is off its census row (fixe, never nil) -- …")`, puis `*out = value.as.fixed`. Aucune boucle de clés ne prend ces noms. |
| R6 | `nr:hb` → `self->qh.s = s_val;`. C'est la seule écriture directe. |
| R7 | 48 entrées retirées. `g_qh_members`, `pico8_qh_member`, `g_qh_defaults` et `g_os_defaults` sont supprimés avec leurs boucles (tables vidées, forme 1b.9/1b.23d). `nl`, `hj`, `pm`, `nr`, `fr` et `mx` gardent leurs tables avec leurs autres entrées. |
| R8 | Crochets gardés, dont `PICO8_MEMBER_CENSUS_SEE(self, pico8_qh_t)` sur un typedef sans `pico8_value_t` (précédent `jo`, H8). `tools/member_census.txt` n'est pas édité. |
| R9 | 138 → 128. |

### Défauts déballés (R7 de 1b.14a) (G)

- **`planet_family.c`** : 28 `static` et 4 `_is_bool`, déballés dans `pico8_planet_family_init` juste après la lecture de spec de chaque genre, et remis à 0 ou NULL dans le `defaults_reset` du genre.

| genre | `static` |
|---|---|
| `qh` | `g_qh_hp`, `g_qh_px`, `g_qh_dq`, `g_qh_qh`/`g_qh_qh_is_bool`, `g_qh_od`, `g_qh_fp`, `g_qh_dw`, `g_qh_s`, `g_qh_ba` |
| `nl` | `g_nl_qh`/`_is_bool`, `g_nl_hp`, `g_nl_s`, `g_nl_px`, `g_nl_od`, `g_nl_ba` |
| `os` | `g_os_hp`, `g_os_px`, `g_os_qh`/`_is_bool`, `g_os_s`, `g_os_od` |
| `hj` | `g_hj_hp`, `g_hj_qh`/`_is_bool`, `g_hj_px`, `g_hj_od`, `g_hj_s` |
| `pm` | `g_pm_hp`, `g_pm_px`, `g_pm_s` |

  Aides (lecture de forme de 1b.23a1) :
  - `family_default_table`, message étendu : `"qh/nl/os/hj/pm/ga.nu, qh/nl/os/hj/pm.px, qh/nl.ba: …"` ;
  - `family_default_fixed`, message étendu : `"os.hs, ga.fa, qh.mi, qh/nl/os/hj/pm.hp, qh/nl/os/hj.od, qh.fp/dw, qh/nl/os/hj/pm.s: …"` ;
  - `family_default_hf`, gardée sous son nom (renommer n'est pas l'objet de l'étape), message étendu : `"qh/ga.hf, qh/nl/os/hj.qh: …"` ;
  - **`family_default_string`**, nouvelle, pour `dq` : `"qh.dq: a default of the struct is a key of qh's spec and a string (chaine)"`. Les `assert` en ligne de `me` ne sont pas retouchés.
- **`planet_family_trade.c`** : 10 `static`, déballés et remis à nil de même : `g_nr_od`, `g_nr_fp`, `g_nr_s`, `g_nr_dw`, `g_fr_od`, `g_fr_s`, `g_mx_hp`, `g_mx_s`, `g_mx_px`, `g_mx_od`. Aides :
  - `family_trade_default_table`, message étendu avec `mx.px` ;
  - **`family_trade_default_fixed`**, nouvelle : `"nr/fr/mx.od, nr.fp/dw, nr/fr/mx.s, mx.hp: … (fixe)"`.
- **Pointeur de spec** pour `px`, `ba` et `dq` (R3 « même REF »). L'identité est figée par `os:320`, `hj:354/405`, `pm:336/422` et `mx:343/406`.
- **`os`** : sa table vidée est supprimée, avec sa boucle de construction, sa boucle de lecture de spec (remplacée par les déballages), sa boucle de remise à nil et les `size_t i` devenus inutiles (`-Werror`). `g_os_nu`/`g_os_hs` restent.
- **Écarté (G)** :
  - des littéraux en R3 à la place des `static` (lecture 1b.14b/1b.16), parce que les tables des filles listent ces membres (1b.14a s'applique) et que les TU comparent aux specs ;
  - une aide générique partagée entre les deux fichiers (1b.13, point 2).

### Boucles sentinelles (forme 1b.23a2, lecture 1b.23d pour un tableau vidé)

- `et` sort des tableaux `written[]` de `qh`, `os`, `hj`, `pm`, `mx`, `nr`, `nl` et `fr`, et `s` sort de ceux de `nr` et `fr`.
- **Tableaux vidés, supprimés avec leur boucle** : `test_pico8_qh`, `test_pico8_os`, `test_pico8_nr` et `test_pico8_fr`.
  - Dans `test_pico8_fr`, le crochet `PICO8_MEMBER_CENSUS_STOP()` qui précède la boucle est **gardé seul** à sa place (G). La sortie d'une future régénération du recensement reste ainsi identique (H18), et aucun crochet ne change.
  - Les variables encore utilisées (`i`, `v`) restent. Celles qui ne le sont plus sont retirées (`-Werror`).
- **Tableaux gardés** : `hj` `{qy}`, `pm` `{qy}`, `mx` `{qy, ht}` et `nl` `{qw}`.
- Rien d'autre ne change dans ces TU. L'intention des boucles passe dans la caractérisation (écriture dans le genre, nil) et dans le test d'arrêt (hors genre).

### Revue des tests de régression (§6), mécanique

**Liste** : l'union des tests nommés au §6 et dans les lignes « Tests : » des étapes 1a, soit les 17 fichiers `tests/test_pico8_<nom>.c`. `test_class_coverage` (1a.0) est l'outil de couverture ; `test_pico8_class_chain` est supprimé depuis 1a.42e.

`built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`.

**Greps sur ces 17 fichiers** :

- accès direct : `grep -nE '(->|\.)(hp|px|dq|qh|od|fp|dw|s|ba|et|has_et|qh_is_bool)\b|offsetof\([^)]*\b(hp|px|dq|qh|od|fp|dw|s|ba|et)\)'`. Il couvre `planet->qh.s` et `->hj.qh.od` (leçons de 1b.23a2/b) ;
- par nom : `grep -nE '"(hp|px|dq|qh|od|fp|dw|s|ba|et)"'` ;
- chemin : `grep -cE 'pico8_(qh|nl|os|hj|pm|nr|fr|mx)_|planet_family|"(qh|nl|os|hj|pm|nr|fr|mx)"|pico8_bv\b|"hq"|laborator'`.

**Aucun accès direct.** Toutes les correspondances, chacune avec son plan de preuve rouge :

| # | test | ligne | accès | touché par le diff ? | plan de preuve rouge |
|---|---|---|---|---|---|
| R-a | `empty_planet_cleared` | `:79`, `:214` `pico8_cg("qh")` (tag de `cg`, pas le membre) ; chemin : le rejeu construit 2 `qh`. Le laboratoire posé passe par `bv` (`dw` lu, `et` → la planète vide, **un `jo` nu**, `fo` posé), puis `d_` (`et`, `od` = 15 payé, `QH_PRICE`, `dq` → `"ly"`, `fp`) | **par chemin** : lecteur, écrivain et init de `qh` modifiés. Le fichier a un diff vide | Bug d'origine **529f355** remis localement (1a.17/1a.18) : retirer l'entrée `LL` du descripteur de `jo` (`planet.c`). Attendu **rouge**, l'assertion qui échoue étant notée (1a.18 : `qh->count == 1`, ou une planète vide qui reste). On restaure : vert. Deux lignes « Runs ». **Obligatoire.** |
| R-b | `playability` | `:9` commentaire « `self.et` false » | homonyme : `g_.et`, la sonde de l'étape 1. Le rejeu ne construit aucun `qh` (1a.17 : compte de `cg.jo` et `fm` seulement) | non | Bug de référence de 1a.17 : le constructeur de `jo` appelle `pico8_ke_struct_pq` deux fois. Attendu **rouge** (`count <= PICO8_PLAYABILITY_MAX_PLANETS` faux). On restaure : vert. **Dans le budget seulement** ; sinon le journal note « non lancé, homonyme hors chemin ». |
| R-c | `slipway_drag` | `:134` `field_number(held, "od")` | homonyme : `od` du `jc` tenu. Aucun `qh` sur le rejeu | non | Bug d'origine 2d7021f (1a.17) : remettre nil à la place de l'entité de `jz("gv")` dans `ui:bh`. Attendu **rouge** (`oe_count(origin) == 1` faux). On restaure : vert. **Dans le budget seulement**, sinon noté comme R-b. |

Les 14 autres tests n'ont aucune correspondance. `ga_export_panel_by` et `geoharvest_burst` n'incluent `pico8_planet_family.h` que pour `ga`. Tous restent verts dans `make test` sans modification (H17).

### Preuves (sortie attendue sur l'arbre final)

**Sortie de `qh`** :

- `sed -n '/^typedef struct {/,/} pico8_qh_t;/p' src/pico8_planet_family.h | grep -c 'pico8_value_t'` : **0** ;
- `make value-members | grep -c 'pico8_qh_t'` : **0** ;
- `grep -rnw 'g_qh_members\|g_qh_defaults\|pico8_qh_member\|g_os_defaults' src/` : **vide** ;
- `grep -rn 'offsetof(pico8_qh_t\|offsetof(pico8_os_t' src/` : **vide**.

**Tables et aides** :

- `grep -rnoE 'offsetof\(pico8_(qh|nl|os|hj|pm|nr|fr|mx)_t, ([a-z0-9_]+\.)*(hp|px|dq|qh|od|fp|dw|s|ba|et)\)' src/` : **vide** (48 occurrences avant : 38 dans `planet_family.c`, 10 dans `planet_family_trade.c`) ;
- `grep -nE '(\.|->)(hp|px|dq|qh|od|fp|dw|s|ba|et)\.(kind|as)' src/pico8_planet_family*.c` : vide ;
- `grep -nE 'qh(\.|->)(s|et) = ' src/pico8_planet_family*.c` : seulement `nr:hb` (`self->qh.s = s_val`) et les poses R3 ;
- `grep -n 'pico8_value_t \*' src/pico8_planet_family.c | grep -v 'pico8_value_t \*out_value)' | grep -v 'pico8_value_t \*out)'` : relu, aucun membre de `qh` ;
- `grep -rn '\[PICO8_METHOD_LY\] = pico8_' src/*.c` : **8 lignes**, avant comme après ;
- `grep -n 'family_is_target_class' src/pico8_planet_family.c` : la définition et un seul appel ;
- `git diff -U0 src/pico8_planet_family.c | grep '^+#include'` : une ligne, `pico8_slipway.h` ;
- `grep -c 'off its census row' src/pico8_planet_family.c` : 3 (2 avant) ; `src/pico8_planet_family_trade.c` : 3 (2 avant) ;
- `grep -n 'has_et\|qh_is_bool' src/pico8_planet_family.h` : chacun juste après son membre, plus les deux emballages.

**Périmètre du diff** :

- `git diff --stat HEAD -- src/ ':!src/pico8_planet_family*'` : vide ;
- `git diff HEAD -- tools/ Makefile` : vide ;
- 8 TU à boucle sentinelle : le diff ne touche que `written[]` ou le tableau vidé et sa boucle (sauf le `STOP` de `fr`, gardé) ;
- `test_pico8_ui.c` : ajouts seulement.

### Empreinte remesurée ; découpage

La ligne dit 21 (8, 13). **Empreinte réelle** (fichiers qui lisent ou écrivent l'un des 10 membres d'une struct de l'arbre `qh`) : **≈ 20 (8, 12)** (H19).

- **src (8)** : `planet_family.h`, `planet_family.c` et `planet_family_trade.c` en direct ; `probe.c`, `planet.c`, `planet_display.c`, `planet_trade.c` et `economy_score.c` par nom.
- **tests (≈ 12)** :
  - `test_pico8_qh`, `os`, `hj`, `pm`, `mx`, `nr`, `nl`, `fr` ;
  - `g_` (1b.22 lit `et` sur les porteurs) ;
  - `ui` (`nr.s`) ;
  - `jo_spec_defaults` ;
  - `empty_planet_cleared` (par chemin).

**Fichiers modifiés : 12**, plus `docs/plan-3.md` :

- **src (3)** : `pico8_planet_family.h`, `pico8_planet_family.c`, `pico8_planet_family_trade.c`. `probe.c` n'est pas touché : ses sites par nom restent.
- **tests (9)** :
  - `test_pico8_ui.c` (caractérisation, test d'arrêt) ;
  - `written[]` de `test_pico8_qh`, `os`, `hj`, `pm`, `mx`, `nr` (**6 TU lents**), `nl` et `fr`.

**Découpage (G) : aucun, un seul commit.** 12 fichiers, sous le seuil d'environ 18. Découpes écartées :

- **(a) `et` seul, puis les 9 défauts** : les deux moitiés touchent le même lecteur, la même `struct_init` et les mêmes tableaux `written[]` (qui ne contiennent qu'`et`), et `test_pico8_ui`. Cela ferait deux `make e2e`, deux `class-coverage-net` et deux passes sur 6 TU lents, hors budget. Surtout, `g_qh_members` ne tomberait qu'au second commit.
- **(b) `qh` puis les filles** : impossible, un membre change de type pour toutes les tables à la fois (R7).

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 138 sur l'arbre de départ, puis 128. Seules les 10 lignes `pico8_qh_t` disparaissent.
- **H2** : dans les 15 rejeux et dans tous les TU hors fils de `must_abort`, `et` ne reçoit que nil, `false` ou une REF vers l'une des 11 classes, et `s` que des fixes.
- **H3** : aucun site n'écrit `true` dans `qh.et` (cart, `pico8_bv`, TU). Démentie, c'est l'arrêt « champ sans règle », sans élargir le membre.
- **H4** : un `pico8_<X>_new(NULL)` neuf, pour les 9 classes de `cg.np`, s'inscrit dans `cg.np` par `lt`, et son `p` s'écrit par nom. Un `hq` d'un `qh` neuf à `mp = p·128 + (1, 0)`, cibles espacées de plus de √0,025 et loin de la galaxie, prend la branche `nq < dw` sur cette cible. Sinon, l'implémenteur vise une planète de la galaxie de ce genre, ou garde le seul aller-retour par nom pour ce genre, et le note au commit. La preuve par le code (`nu` ∋ `np`) reste, et ce n'est pas un arrêt.
- **H5** : aucune valeur figée ne bouge :
  - les hash de `test_pico8_qh`, `os`, `hj`, `pm`, `mx`, `nr`, `nl`, `fr` ;
  - `STRUCT_HASH`/`SPEC_HASH` de `jo_spec_defaults` et `spec_defaults` ;
  - `BV_ET_MEMBERS_HASH` de `test_pico8_g_` ;
  - les hash de `test_pico8_ui` (`JO_*`, `HOVER`, `DRAG_REPLAY`, `TECH_REPLAY`…) ;
  - les 15 empreintes du filet.
- **H6** : seuls les 8 descripteurs de l'arbre `qh` posent `PICO8_METHOD_LY`.
- **H7** : tout binaire qui lie `planet_family.c` lie aussi `planet.c`, `slipway.c` et `probe.c`.
- **H8** : le build `-DPICO8_MEMBER_CENSUS` compile avec un `pico8_qh_t` sans `pico8_value_t`. Il n'est pas lancé.
- **H9** : `test_pico8_ui` reste sous 30 s avec deux fonctions de plus, dont un rejeu `empty_planet_cleared` complet (229 frames).
- **H10** : 1b.23d est commitée. `g_qh_defaults` a 9 entrées, et les `written[]` des TU sont ceux cités.
- **H11** : sur l'ancien code, chaque cas du test d'arrêt se termine sans `SIGABRT` (`_exit(0)`). Un cas qui avorte déjà est retiré du « rouge d'abord » et noté.
- **H12** : sur une planète neuve, les valeurs sont celles des specs (tableau du cart). `qh` est `FIXED` 1 sur `qh`, `nr`, `fr`, et `BOOL` 0 présent sur `nl`, `os`, `hj`, `pm`, `mx`. `et` est absent. Les valeurs figées sont celles **observées**.
- **H13** : `start_game` sur `tests/fixtures/empty_planet_cleared_events.json` (lecture seule), puis toutes les frames, reproduit la pose du laboratoire sur la planète vide : `et` du laboratoire devient une REF `jo` à une frame observée.
- **H14** : le retrait de `LL` rend `empty_planet_cleared` rouge sur l'arbre final.
- **H15** : `make sanitize-net` reste propre. Aucune allocation nouvelle, et `et` qui pend n'est jamais lu sans un `bv` qui vient de l'écrire.
- **H16** : rien ne dépend de `sizeof(pico8_qh_t)` ni de la disposition des filles.
- **H17** : les 17 tests de régression passent sans modification.
- **H18** : le crochet `PICO8_MEMBER_CENSUS_STOP()` de `test_pico8_fr`, gardé seul, laisse la table régénérée identique. Ce n'est pas lancé.
- **H19** : l'empreinte réelle est ≈ 20. Le compte exact est fait au commit.
- **H20** : `pico8_value_t pico8_qh_et`/`pico8_qh_qh` et les aides nouvelles, chacune appelée au moins une fois, ne déclenchent aucun avertissement (`-Werror`).

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Les nouveaux tests n'accèdent à aucun des 10 membres directement : ils compilent sur les deux formes.
- **(G) Où : `test_pico8_ui`.** Il est rapide, dans `CENSUS_UNIT_TESTS`, lie `ENGINE_SRCS`, inclut déjà `planet_family(_trade).h`, `probe.h`, `slipway.h` et `help.h`, et a `start_game`, `play_frame`, `must_abort`, `in_fresh_child`. Précédents : 1b.14a et 1b.23a–d. **Pas `test_pico8_qh`, qui est lent** (1b.11).
- **Caractérisation dirigée, figée sur l'ancien code** : `test_qh_members_by_name`, lancée par `in_fresh_child` après les tests de `jo`. Elle lit par le **lecteur brut du descripteur** (`cls->field`, témoin `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder (retour, genre, valeur, pointeur). C'est une fonction distincte, avec un hash propre.
  1. **Les 8 genres, neufs.** `pico8_<X>_new(NULL)` pour `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx` : on plie les 10 noms (présence, genre, valeur). `assert` : `px`/`ba`/`dq` sont au **même pointeur** que `pico8_<X>_default` (ou celui de l'ancêtre qui le définit), et `et` est absent (H12).
  2. **Vivants.** `start_game` sur le rejeu `empty_planet_cleared` (lecture seule du fichier), puis on plie chaque entité de l'arbre `qh` de `hv`. On joue ensuite toutes les frames (`play_frame`) et on replie. Pour `et`, on plie la présence, `BOOL` 0, ou le rang de la classe de sa cible dans une liste fixe des 11 classes admises (accesseurs publics, `assert` qu'elle y est). On relève la frame où `et` devient une REF `jo` (H13).
  3. **`bv` vers chaque cible** (H4), dans une galaxie `start_game(DRAG)` :
     - un `qh` neuf mène `hq` vers une planète neuve de chacune des 9 classes de `cg.np` (`jo`, `qh`, `nl`, **`os`**, `hj`, `pm`, **`nr`**, `fr`, `mx`, placées par `p`, écrit par nom), puis vers un point vide hors du panneau (la cible jetable, `rp` (0,0)) et dans le panneau (`false`, `rp` (64,64)) ;
     - chaque genre de l'arbre (neuf) mène `hq` une fois vers une planète ;
     - attendu : `et` = la cible au même pointeur et `fo` de la cible = le porteur, ou `BOOL` 0 présent, ou la cible jetable.

     **`nl`, `os`, `nr` exercés ici prouvent sur l'ancien code qu'ils sont producteurs** (lecture 1b.22).
  4. **Écrivain par nom d'`et`**, sur `qh`, `nl`, `pm` et `fr` (chaque chaîne de délégation) :

     | écritures | lecture attendue |
     |---|---|
     | chacune des 11 classes admises : les 9 planètes neuves, un `jc` (`pico8_jc_new` sur `qu` = une planète), la cible jetable relue en 3 | même pointeur |
     | `false` ; **`false` à union nulle** | `BOOL` 0 présent |
     | **REF à union nulle** ; `REF NULL` | absent, témoin intact |

     `cls->set_field` rend 1. Sur `qh`, les noms `hp`, `s`, `od`, `id`, `qs` rendent 0 **par l'écrivain brut**, sans rien toucher (ensemble des 20 noms inchangé).
  5. **`s` de `nr` et `fr`** :
     - écrivain par nom, fixe 3 → `FIXED` 3, puis **fixe 0 à union nulle** → `FIXED` 0 présent. `cls->set_field(nr, "s")` rend 1, et `"od"` rend 0 ;
     - `mv(nr, "hb")` et `mv(fr, "hb")` → `s` = l'entrée de `hz` (170, 78, valeurs observées).
  6. **`d_` et `dl`** : sur un laboratoire à `et` `false` (3), avec des crédits suffisants puis insuffisants, `call(lab, "ey")` → retour, crédits, `me`, repliés (`od`, `dq`, `fp` lus par nom). `ev` (`dl`) → empreinte d'écran.
  7. **Intrus** sur le lecteur de `qh` : `""`, `"h"`, `"hp "`, `"HP"`, `"et_"`, `"has_et"`, `"qh_is_bool"`, `"e"`, `"ba_"` → 0, témoin intact.
  8. Tout est plié dans **`QH_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ, puis inchangé. Les adresses sont jugées par `assert`, jamais pliées.
- **Test d'arrêt, rouge d'abord** (1b.13/1b.15/1b.22) : `test_qh_writer_stops_off_row`, par `in_fresh_child` après la caractérisation. Il utilise la forme `must_abort` à valeur que 1b.23d a ajoutée à `test_pico8_ui` si elle existe, sinon il ajoute `must_abort_value` (forme `test_pico8_bg`/`g_`). Tous les cas « union nulle » sont construits par `memset` puis `kind`.

  | cible | nom | valeurs hors genre |
  |---|---|---|
  | un `qh`, un `pm` (délégation `pm` → `hj` → `qh`) et un `nr` neufs | `et`, non-structs | fixe 1, **fixe 0 à union nulle**, **`true`**, chaîne `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table`, REF `pico8_hv()` (liste), REF vecteur (tag `UNKNOWN`) |
  | idem | `et`, **structs refusées** | un `ga` neuf, un `g_` neuf, un `fv` neuf, un `mz` neuf, le `bg`, l'`ok`, un `hp` de `gb`, `pico8_mz_jj_wrapper()` |
  | un `nr` et un `fr` neufs | `s` | **nil (`REF NULL`)**, `true`, **`false` à union nulle**, `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table` |

  **Exclus**, parce qu'ils sont dans le genre : `false` et `false` à union nulle pour `et`, les 11 classes admises, et le fixe 0 à union nulle pour `s`.

  Il est **rouge sur l'ancien code**, qui recopie toute valeur (H11), et vert après : une ligne « Runs » pour chacun. Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Tests existants qui figent déjà beaucoup** :
  - dans les TU lents : `test_life` (`bool_state` 0/1/3), `fold_live`, `own[]`, `scalars[]`, `inherited[]`, les identités de `px`/`ba`, `must_abort(write_hp/s/od/et)` ;
  - `test_pico8_g_` (1b.22 : `get_ref(holders[k], "et") == t` pour les 8 porteurs de l'arbre) ;
  - `test_pico8_nl`, `test_pico8_fr` ;
  - `empty_planet_cleared` ;
  - les 15 rejeux (`session_events`, `long_session_events`, `trading_hub_session`… posent des laboratoires et des filles).
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_ui && ./tmp/test_pico8_ui` sauf mention. Chacune est notée dans le commit avec le test qui la rougit, ou avec la justification de son trou. Un appel unique d'aide `static` se retire sous `if (0)` (1b.14b).
  1. **(§7-1) indicateur forcé vrai, dans sa lettre** : `pico8_qh_et` ignore `has_et`, donc nil est lu comme `false` présent. Attendu rouge : caractérisation 1 et 4 (nil → absent). `bool_state` = 0 de `test_life` (lent) est noté sans relance.
  2. **(§7-2) « présent 0 » là où c'était absent** : **sans objet dans sa lettre**, puisqu'aucun fixe typé ici ne peut être absent (les 5 fixes sont nil `jamais`, sans indicateur). **Forme voisine** : `pico8_qh_et` rend `pico8_value_fixed(0)` quand `has_et` vaut 0. Attendu rouge : caractérisation 1 et 4.
  3. **(§7-3) init qui saute un membre typé.** Aucune boucle d'init de `qh` ne reste, et l'omission pure laisse de la mémoire de `malloc` non déterministe. On prend donc la forme « pose une autre valeur » (**trou justifié** de la forme littérale, précédents 1b.14a et 1b.23d) :
     - (a) `pico8_qh_struct_init` pose `has_et = 1` → rouge : caractérisation 1 ;
     - (b) `pico8_qh_struct_init` pose `od = g_qh_fp` → rouge : caractérisation 1 et 6 (prix), `empty_planet_cleared` (`QH_PRICE`) ;
     - (c) `nl` pose `qh.qh_is_bool = 0` (son `false` lu `FIXED` 0) → rouge : caractérisation 1 ;
     - (d) la pose `s = g_nr_s` de `nr` est mise sous `if (0)` (`s` reste celui de `qh`, 174) → rouge : caractérisation 1 et 5 ;
     - (e) `pico8_planet_family_init` pose dans `g_os_px` une copie neuve de la table → rouge : caractérisation 1 (`assert` de pointeur) ; `os:320` noté.
  4. Propre à l'étape, *prédicat sans `LY`* : les filles de l'arbre sont refusées. Attendu rouge : caractérisation 3 et 4, et `test_pico8_g_`, où le `bv` d'un porteur de l'arbre tombe sur la cible jetable, pas sur une planète : noté.
  5. *Prédicat sans `jo`* : attendu rouge : caractérisation 2 et 3 (la planète vide), et `empty_planet_cleared`.
  6. *Prédicat sans `jc`* : attendu rouge : caractérisation 4.
  7. *Prédicat sans `bv_et`* : attendu rouge : caractérisation 3 (hors du panneau), et `test_pico8_g_` (`BV_ET_MEMBERS_HASH`, porteurs de l'arbre).
  8. *Prédicat qui accepte toute struct* (`return 1`) : attendu rouge : test d'arrêt (structs refusées) seul.
  9. *Tag non vérifié* (conjonction `STRUCT` retirée) : attendu rouge : test d'arrêt (table, liste, vecteur ; `must_abort` exige `SIGABRT`).
  10. *`true` accepté* (`value.as.boolean == 0` retiré) : attendu rouge : test d'arrêt (cas `true`).
  11. *Nil jugé sur le pointeur seul* : attendu rouge : test d'arrêt (`zero_fixed`, `zero_string`, `zero_function`).
  12. *Écrivain d'`et` muet* : `if (0) family_take_target(…)` et `return 1`. Attendu rouge : caractérisation 3 et 4, et `test_pico8_g_`.
  13. *`s` accepte nil* : `family_trade_take_fixed` pose 0 sur `REF NULL`. Attendu rouge : test d'arrêt (cas nil).
  14. *`nr:hb` muet* : l'écriture R6 est retirée. Attendu rouge : caractérisation 5 ; `test_pico8_nr` (`fr` s = 78) noté.
  - **Mutant équivalent, non retenu, noté** : `*has = 1` posé avant l'`assert` de l'aide. Dans un fils qui avorte, l'état n'est jamais relu.
- **`gcov`** : `test_pico8_ui` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23, sur `planet_family.c` et `planet_family_trade.c`. Chaque ligne modifiée doit être exécutée : les déballages et les remises à nil, les poses R3 des 8 genres, les 10 branches R4, les deux emballages (trois états d'`et`, deux de `qh`), les deux aides d'écriture, le prédicat (ses 4 disjonctions) et `nr:hb`.
  - Trous attendus : l'échec des `assert` des aides d'écriture (fils de `must_abort`, prouvés par le test d'arrêt et les mutations 8–11 et 13), et celui des aides de lecture de spec (inatteignable, la spec est un littéral).
  - Le rapport va dans le commit.
- **Prior art** :
  - `test_jo_*` de `test_pico8_ui` (1b.23a–d) ;
  - `test_bv_et_members_by_name` et `test_bv_et_writer_stops_off_row` de `test_pico8_g_` (1b.22) ;
  - `hp_take_il` (1b.9, `false` seul) ;
  - `jo.hf`/`family_default_hf` (1b.23a1) ;
  - `qb_take_fixed` (1b.19) ;
  - `test_life` de `test_pico8_qh`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_ui` : 10 à 30 s. Environ 22 lancements (départ, arrêt rouge, final, 18 mutations, `gcov`), soit environ 8 min. Si un lancement dépasse 30 s, on joue d'abord les mutations §7 (1, 2, 3a–3e) puis 4, 5, 7, 8, 12, et on note les autres comme non lancées ;
- preuve §6 R-a : deux lancements de `test_pico8_empty_planet_cleared`, moins d'une minute. R-b et R-c seulement s'il reste du budget ;
- `make test` : 20 s à 1 min, une fois ;
- `make e2e` : 4 à 6 min, une fois ;
- `make class-coverage-net` : 3 à 4 min, **dans la même invocation** que `make e2e` (arbres distincts, 1b.0) ;
- `make sanitize-net` : environ 4 min seul, **jamais** en même temps que `e2e`, une fois sur l'arbre final ;
- `make value-members` : quelques secondes.

**TU lents touchés** : `test_pico8_qh` (tableau `written[]` et boucle supprimés), `test_pico8_os` (idem), `test_pico8_nr` (idem), `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx` (`written[]` réduits). `test_pico8_jo` et `test_pico8_ie` ne sont pas modifiés, mais `make e2e` les lance.

**Construire les huit tôt**, juste après le typage et avant toute mutation, en compilation seule : `make -j tmp/test_pico8_qh tmp/test_pico8_os tmp/test_pico8_hj tmp/test_pico8_pm tmp/test_pico8_mx tmp/test_pico8_nr tmp/test_pico8_jo tmp/test_pico8_ie`. Ils ne sont lancés que par `make e2e`.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre (départ / final / mutation n / §6 R-x bug remis / §6 R-x restauré) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` : il compile aussi les binaires `DATA_ENGINE_SRCS` (H7) ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` vert, et les **8** TU de `UNIT_TESTS_SLOW` passent (`test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 128`**, aucune ligne `pico8_qh_t` ;
5. `timeout 15m make class-coverage-net` : **sortie identique à `tools/class_coverage_net_expected.txt`**. Elle s'applique parce que le diff touche `pico8_qh_struct_init` et les constructeurs des 7 filles, sans toucher `pico8_class_coverage_record` ni `KNOWN[]`.

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements : le rejeu `empty_planet_cleared_events.json` est seulement **lu** par la caractérisation.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

**Départ et filet**

- `FAILED:` si 1b.23d n'est pas commitée au départ, ou si `make value-members` ne rend pas 138 sur l'arbre de départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.

**Types, tables et écrivains**

- `FAILED:` (« champ sans règle ») dans chacun de ces cas :
  - une aide d'écriture ou de lecture de spec avorte dans un rejeu ou dans un TU hors des fils de `must_abort` (H2, H3) ;
  - un type est élargi au lieu d'avorter : `true` accepté, `et_is_bool` ajouté, prédicat étendu hors des 11 classes, `pico8_value_t` gardé ;
  - un `has_` est ajouté à un autre membre qu'`et`.
- `FAILED:` si l'un des 10 membres n'a pas le type de la table de recensement à sa place, si `has_et` ou `qh_is_bool` manque ou n'est pas juste après son membre, ou si `et` n'est pas un `pico8_struct_header_t *`.
- `FAILED:` si nil et `false` d'`et` ne se lisent plus par nom exactement comme en 1a (absent ; `BOOL` 0 présent), ou si `qh` ne se lit plus `BOOL` 0 / `FIXED` 1 selon le genre.
- `FAILED:` si `pico8_qh_t` garde un `pico8_value_t`, si `g_qh_members`, `g_qh_defaults`, `pico8_qh_member` ou `g_os_defaults` existe encore, ou si une table d'offsets de l'arbre liste encore l'un des 10 noms.
- `FAILED:` si une autre entrée des tables de `nl`, `hj`, `pm`, `nr`, `fr`, `mx` est retirée ou modifiée.
- `FAILED:` si `px`, `ba` ou `dq` n'est pas le **pointeur de la spec**, ou si un `static` de défaut n'est pas remis à nil dans son `defaults_reset`.
- `FAILED:` si le prédicat n'admet pas exactement les 8 descripteurs à `LY`, `jo`, `jc` et `bv_et`, s'il est une liste d'accesseurs qui ne se lie pas partout où `planet_family.c` l'est, ou si `planet_family.c` gagne un autre `#include` que `pico8_slipway.h`.
- `FAILED:` si l'écrivain de `qh` n'accepte plus exactement ses 20 noms ou celui de `nr` ses 21, ou si une boucle de clés change.
- `FAILED:` si une aide viole la forme de 1b.13/1b.15 :
  - elle ne pose pas l'état nil d'abord (sauf `family_trade_take_fixed`, qui avorte sur nil) ;
  - elle ne teste pas genre **et** pointeur ;
  - elle n'avorte pas sur un hors-genre à union nulle ;
  - son message ne nomme pas `qh.et`/`qh.s` et le genre de la ligne ;
  - elle est partagée entre fichiers ou rendue publique.
- `FAILED:` si `family_take_target` pose `has_et = 1` ailleurs qu'en dernier, ou lit `cls` avant de vérifier le tag.

**Diff et preuves**

- `FAILED:` si un site par nom change (`probe.c`, `planet*.c`, `economy_score.c`), ou si un site par nom est remplacé par un accès direct.
- `FAILED:` si `tools/`, le `Makefile`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` change.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si le diff d'un test fait autre chose que ceci, ou si une valeur figée existante change (H5) :
  - ajouter la caractérisation, le test d'arrêt, `QH_MEMBERS_HASH`, la forme `must_abort` à valeur si elle manque, le chemin du rejeu (lecture seule) et leurs appels à `test_pico8_ui` ;
  - retirer `et`/`s` des 8 tableaux `written[]` ;
  - supprimer les tableaux vidés et leur boucle (`qh`, `os`, `nr`, `fr`), le crochet `STOP` de `fr` étant gardé seul.
- `FAILED:` si un test de régression (§6) a un diff, ou s'il manque les deux lignes « Runs » de R-a (rouge avec 529f355 remis, vert restauré). Pour R-b et R-c, soit leurs deux lignes, soit la mention « non lancé, homonyme hors chemin » au journal.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 128, liste encore `pico8_qh_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` s'il manque une ligne « Runs » de `make class-coverage-net` identique à la référence, ou si le diff touche `pico8_class_coverage_record` ou `KNOWN[]`.
- `FAILED:` si un crochet du recensement change, ou si `tools/member_census.txt` est édité à la main.

**Tests de l'étape**

- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, ou si elle lit ou écrit l'un des 10 membres directement.
- `FAILED:` si la caractérisation ne couvre pas les 8 genres neufs et vivants, les 10 noms, les 11 classes admises d'`et` (dont `nl`, `os`, `nr` par `hq`), `false`, nil, `s` par nom et par `nr:hb`, ou si `QH_MEMBERS_HASH` change après.
- `FAILED:` si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'arbre de départ et une verte sur l'arbre final, ou s'il n'a pas :
  - les cas à union nulle `memset` ;
  - `true` ;
  - les structs refusées (`ga`, `g_`, `fv`, `mz`, `bg`, `ok`, `hp`, `mz_jj`) ;
  - nil pour `s`.
- `FAILED:` si le commit ne contient pas son rapport `gcov` et ses mutations (1, 2, 3a–3e, 4 à 14), chacune avec son test ou la justification de son trou, plus le mutant équivalent.

**Commit et note**

- `FAILED:` si le travail n'est pas en un commit vert, ou si un membre propre d'une fille (`nl.qw`, `hj.qy`, `nr.hz`…) y est typé.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` si le budget d'environ 20 minutes est dépassé sans que le journal note ce qui n'a pas été lancé.
- `FAILED:` s'il manque le tableau « Runs » ou la note « Découvert à 1b.24 », ou si la note ne porte pas les lectures nouvelles :
  - `et` en {`false` seul, REF à plusieurs cibles} → pointeur d'en-tête + `has_et` ;
  - prédicat par `LY` ∪ trois accesseurs ;
  - `nl`, `os`, `nr` admis par preuve de code et caractérisation ;
  - `od`/`s` sans indicateur malgré `stat?` ;
  - tables de `qh` et d'`os` supprimées ;
  - crochet `STOP` gardé seul.

## Out of Scope

- Les membres propres des filles, qui ont leurs étapes :
  - `nl.rr`/`nl`/`qw` (1b.25) ;
  - `hj.*` (1b.26) ;
  - `pm.ng`/`fz` (1b.27) ;
  - `mx.ht` (1b.28) ;
  - `ga.*` (1b.29) ;
  - `nr.hz`/`rr` (1b.30) ;
  - `fr.mt` (1b.31).
- Les tables `g_nl_members`, `g_hj_members`, `g_pm_members` et les autres entrées des tables de défauts des filles.
- Les homonymes : `g_.et`/`od`/`fp`/`dq`/`dw` (1b.32), `jc.et`/`od` (1b.35), `mz.et`/`dw`/`dq`/`od`/`fp` (1b.36), `ie_ke.s`/`od`, les `px` des projets et des menus.
- `pico8_bv`, `dl` et `d_` eux-mêmes (sites par nom, TROU-7 de `p`).
- Renommer `family_default_hf` ; réunir les prédicats de porteur ou de cible (`bv_et`, `jo`, `qh`) en une aide commune, interdit par 1b.13 point 2 et avec des ensembles différents ; retoucher les `assert` en ligne de `me`.
- Régénérer `tools/member_census.txt` ; ajouter un TU lent au recensement.
- Les fichiers d'événements et `FRAMES_EVENTS`, tout changement de rendu, l'audio, lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - **modules** :
    - `planet_family.h` : typedef, `has_et`, `qh_is_bool`, emballages `pico8_qh_et`/`pico8_qh_qh` ;
    - `planet_family.c` : lecteur, écrivain d'`et`, `family_take_target`, `family_is_target_class`, `family_default_string`, messages étendus, 28 `static` + 4 `_is_bool`, poses R3 de `qh`, `nl`, `os`, `hj`, `pm`, suppression de `g_qh_members`/`g_qh_defaults`/`pico8_qh_member`/`g_os_defaults`, entrées retirées des tables de `nl`, `hj`, `pm` ;
    - `planet_family_trade.c` : `family_trade_take_fixed`, `family_trade_default_fixed`, 10 `static`, poses R3 de `nr`, `fr`, `mx`, entrées retirées, `nr:hb` ;
  - **types** :
    - `et` → `pico8_struct_header_t *` + `has_et`, `false` = présent et NULL, `true` avorte ;
    - `qh` → `pico8_fixed_t` + `qh_is_bool` ;
    - `hp`, `od`, `fp`, `dw`, `s` → `pico8_fixed_t` sans indicateur ;
    - `px`, `ba` → `pico8_ob_table_t *` ;
    - `dq` → `const char *` ;
  - **ensemble d'`et`** : ligne ∪ {`nl`, `os`, `nr`} = 11 classes. Prédicat `LY != NULL` ∪ `jo` ∪ `jc` ∪ `bv_et` ;
  - **tests** :
    - `test_pico8_ui` étendu (caractérisation, test d'arrêt rouge d'abord) ;
    - 8 tableaux `written[]` réduits, dont 4 supprimés ;
    - aucun test de régression modifié ;
    - pas de nouveau TU ;
  - **pas de découpage** : 12 fichiers ;
  - **`class-coverage-net`** lancé une fois, sur l'arbre final ;
  - **§6** : R-a obligatoire (chemin), R-b et R-c dans le budget seulement (homonymes hors chemin).
- **Note « Découvert à 1b.24 »**, à écrire sur la ligne de l'étape. Le reste va dans le commit :

  > patron 1b.1 appliqué ; `hp`, `od`, `fp`, `dw`, `s` → `pico8_fixed_t`, `px`, `ba` → `pico8_ob_table_t *` (pointeur de spec), `dq` → `const char *` ; **`qh` {bool, fixe} → lecture de 1b.23a1 sans écart** (`qh_is_bool`, `pico8_qh_qh`) ; **lecture nouvelle de R1 : {`false` seul, `ref:` à plusieurs cibles} → `pico8_struct_header_t *<m>` + `int has_<m>`, NULL présent = `false`, `true` avorte** (1b.7 posée sur 1b.22 ; ferme pour `qh.et` la question ouverte par 1b.7, puisque `true` n'est jamais écrit ; `g_.et` et `mz.et` la relisent sur leur preuve) ; **ensemble d'`et` = ligne ∪ `nl`, `os`, `nr`** (`cg.np`, exercés par `hq` sur l'ancien code), **prédicat `methods[LY] != NULL` ∪ `jo` ∪ `jc` ∪ `bv_et`** (`LY` = exactement l'arbre `qh`, `planet_family_trade.c` hors `DATA_ENGINE_SRCS`) ; R2 : `od`, `s` sans indicateur (`stat?` d'homonyme, toujours tenus) ; R7 de 1b.14a sur 7 filles (38 `static`), **`g_qh_members`, `g_qh_defaults`, `g_os_defaults` supprimées** ; `s` de `nr` : `l` seul gardé, nil avorte (1b.19) ; boucles sentinelles de 1b.23a2, 4 tableaux vidés supprimés, **le crochet `PICO8_MEMBER_CENSUS_STOP()` d'une boucle supprimée est gardé seul** ; §6 : aucun accès direct, `empty_planet_cleared` prouvé rouge par chemin ; empreinte réelle ≈ 20 (8, 12), 12 fichiers modifiés.
- Le message de commit porte :
  - le périmètre et le compte 138 → 128 ;
  - le tableau des poses R3 par genre ;
  - l'ensemble d'`et` et sa preuve ;
  - le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous ;
  - `QH_MEMBERS_HASH` et les réponses à H1–H20.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
