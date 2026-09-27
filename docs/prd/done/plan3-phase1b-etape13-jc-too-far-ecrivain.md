---
title: "Plan 3, étape 1b.13 : `jc_too_far` (le `{qs=\"-too far-\"}` de `jc:ce`) — `qs` devient un `const char *`, `fo` un `pico8_jc_t *`, et la moitié écrivain de R5 prend sa forme"
description: "`pico8_jc_too_far_t` a deux membres `pico8_value_t` (`qs` : `chaine`, jamais nil, lu ; `fo` : `ref:pico8_jc_t`, nil observé, lu et écrit par nom). Les typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 211 à 209. C'est la première struct à écrivain par nom (`pico8_jc_too_far_set_field`) : ce PRD fixe la forme de la moitié écrivain de R5, que 1b.15 (`bg.m_`) et les suivantes recopient."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.13 — `jc_too_far` (`slipway.c:32`), accès par nom seul, **`qs`, `fo` sans genre**. Empreinte 5 (2, 3). Compte : 209.

Cadre : les puces **Nil**, **REF**, **Coexistence** (en particulier « Écriture : l'écrivain déballe ; nil éteint la présence ; un genre hors de la ligne de recensement du membre est un `assert` ») et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité, pas redécidé. S'appliquent aussi, sans être redérivées, les notes « Découvert à » de `docs/plan-3.md` :

- **1b.3** : sur une instance statique sans constructeur, R3 se lit sur l'initialiseur agrégé (ici `g_jc_too_far`, le même cas que `g_mz_jj_wrapper`, le « frère » de cette struct) ;
- **1b.4** : un typedef d'un `.c` sans table d'offsets, patron appliqué sans écart ;
- **1b.4b** : correction du recensement ; le « sans genre » de 1b.13 est **levé** (`qs` - → `chaine`, `fo` - → `ref:pico8_jc_t`) ; nouvelle cible REF `jc_too_far.fo` → `jc`, `jc` typé après (1b.35) ;
- **1b.5** : forme de la branche R5 par nom (`strcmp`, déballage, `assert` « champ sans règle » hors genre) et `else` final qui garde tel quel l'arrêt de la clé inconnue ;
- **1b.6** : un lecteur qui copie n'est pas un `pico8_X_member` ;
- **1b.7** : un nil observé seulement par `stat?` venu d'un homonyme ne décide rien quand l'écriture est prouvée ;
- **1b.8** : « sans genre » levé par 1b.4b, caractérisation dirigée gardée ; R7 sur un sélecteur local ; **1b.13 est la première étape à écrivain par nom**, puis 1b.15 (`bg.m_`) ;
- **1b.9** : aides R5 `static` par genre, qui posent d'abord nil puis sortent sur nil, sinon `assert` dont le message nomme les membres et le genre de leur ligne ;
- **1b.10** : R5 en boucle de clés directe, une aide `static` par genre qui prend la **valeur** (`lq_take_vec(pico8_value_t, pico8_vec_t **)`) ;
- **1b.11** : R5 avec une autre source (lecteur de chaîne) ; **règle R2 des TU lents** : chaque PRD relit les TU de `UNIT_TESTS_SLOW` qui touchent sa struct (`ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`) pour y chercher des lectures nil par nom ;
- **1b.12** (PRD en triage, en cours d'implémentation) : R5 `ref:autre` reçu en argument ; « R5 écrivain toujours vacante, première candidate 1b.13 ».

### État de départ

HEAD 629286d. 1b.12 (`jc_mouse_point`) est en cours d'implémentation dans l'arbre de travail (`tests/test_pico8_jc.c` modifié, `pico8_jc_mouse_point_take_ra` apparu dans `src/pico8_slipway.c`). **Précondition** : 1b.12 est commitée avant que 1b.13 commence. Elle touche les deux mêmes fichiers (`src/pico8_slipway.c`, `tests/test_pico8_jc.c`), mais pas les lignes de `jc_too_far` : les numéros de ligne cités ici peuvent glisser, chaque site est aussi désigné par son contenu (H9). `make value-members` rend alors 211 (H1).

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H10).

### Lignes de recensement (`tools/member_census.txt:201-202`)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `qs` | nom | `chaine` | jamais | - | l |
| `fo` | nom | `ref:pico8_jc_t` | observé | lu,stat?,écrit | **le** |

Deux membres : 211 − 2 = **209**.

- **« Sans genre » levé** : la ligne du plan date de 1b.0 (« sans genre 5 : … `jc_too_far.qs`/`fo` »). 1b.4b a corrigé la table (fils forkés de `in_fresh_child`), et ses deux lignes portent désormais un genre, cité ci-dessus. On type donc sur ces lignes. **Une caractérisation dirigée est gardée quand même** (G), comme à 1b.8 : la ligne a été fausse une fois, et l'écriture `fo` = nil n'est vue que par un TU (`test_too_far_is_a_struct`), jamais par un rejeu.
- `fo` a `le` : **premier membre lu et écrit par nom** d'une étape 1b. La preuve de nil `écrit` est l'écriture `pico8_entity_set_field(et, "fo", REF NULL)` de `test_too_far_is_a_struct` ; `lu`, le `!has(et, "fo")` qui suit.
- `qs` n'a que `l` : aucun site ne l'écrit par nom, et `test_too_far_is_a_struct` fige que l'essayer avorte (`must_abort(write_qs, et)`).
- Ligne voisine, **non typée ici** : `pico8_jc_t.et` (`:218`, `ref:pico8_jc_t,ref:pico8_jc_too_far_t,ref:pico8_jo_t`), le membre de `jc` qui pointe sur l'objet (1b.35).

**Revue des TU lents (règle R2 de 1b.11).** Relu : `test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`. Aucun ne nomme `too_far`, `"qs"`, `ET_QS`, `pico8_jc_ce` ni `pico8_jc_hq`. Leurs `"fo"` visent une planète (`jo` et familles : listes `written[]`/`refs[]`, `test_pico8_qh.c:450` `get_ref(planet, "fo")`), leurs `"et"` un `qh` ou un `g_`. `test_pico8_ie.c:832` construit un `jc` sans `hq` ; `test_pico8_mx.c:534` (`join`) fait `jc:ey` vers une planète, sans `hq` au-delà de la portée. **Aucune lecture par nom de `jc_too_far`, nil ou non** (H4). Les deux membres sont des pointeurs : R2 ne donne aucun indicateur, avec ou sans cette lecture.

### Ce que fait le cart (`pico8/lua.lua`)

- `jc:ce` (`:641-643`) : `if #(sp-dp)>ov("space folding",0.5,1.08) then return {qs="-too far-"} end`.
- `jc:hq` (`:626-629`) : `self.et=self:ce()if self.et then self.et.fo=self end`. **C'est le seul `…fo=` qui atteint cette table** : `self` y est le `jc` dont `hq` est la méthode.
- `bv` (`:553-558`) écrit aussi `self.et.fo=self`, avec `self` un `g_` (ou un `mz`). Mais son `et` vaut `ij(self.p,cg.np)` (une planète), un `jc` de `jp(cg.jc,…)`, `{}` (l'objet de `bv`, `bv_et`, 1b.22) ou `false`. **Jamais le `{qs}` de `jc:ce`**, qui n'est rendu qu'à `jc:hq` et gardé seulement dans `jc.et`.
- `jc:kj` (`:680`) fait `self.fo=nil` sur **le `jc`**, et `jo:kj` (`:549`) sur **la planète** : aucun ne vise le `{qs}`. Dans le cart, le `{qs}` est neuf à chaque appel, donc son `fo` n'est jamais remis à nil.

La note 1b.4b dit que « seul `self.et.fo=self` d'un `jc` ou d'un `g_` l'écrit » : c'est vrai de **`jc.fo`** (le membre de `jc`, 1b.35, qui reçoit un `jc` par `jc:hq` d'un autre `jc` et un `g_` par `bv`). Pour **`jc_too_far.fo`**, seul le `jc` de `jc:hq` y arrive. Voir « Type du pointeur `fo` ».

### Le code aujourd'hui (`src/pico8_slipway.c`)

- Typedef (`:30-34`) : `{ pico8_struct_header_t header; pico8_value_t qs; pico8_value_t fo; } pico8_jc_too_far_t`.
- **Lecteur** `pico8_jc_too_far_field` (`:36-48`) : `strcmp(name, "qs")` → copie `qs`, `return 1` (sans test de NULL) ; puis `strcmp(name, "fo") != 0` ou `fo` = `REF NULL` → `return 0`, sinon copie. Ni `offsetof`, ni sélecteur local.
- **Écrivain** `pico8_jc_too_far_set_field` (`:50-58`) : `strcmp(name, "fo") != 0` → `return 0` ; sinon `too_far->fo = value` (n'importe quel genre), `return 1`.
- Descripteur (`:60-64`) : aucune méthode, lecteur, écrivain.
- **Instance statique** `g_jc_too_far` (`:66-71`), avec le contrat « never allocated, never freed; `qs` is fixed, only `fo` is written, and the next `hq` overwrites it » : `qs` = `{STRING, "-too far-"}`, `fo` = `{REF, NULL}`. Signalée au recensement par `PICO8_MEMBER_CENSUS_SEE(&g_jc_too_far, pico8_jc_too_far_t)` dans `pico8_jc_init`.
- `pico8_jc_ce` rend `pico8_value_ref(&g_jc_too_far)` sur la branche « trop loin ». Son seul appelant est `pico8_jc_hq`, qui fait `assert(… STRUCT …)` puis `pico8_entity_set_field(et.as.ref, "fo", pico8_value_ref(self))`, avec `self` un `pico8_jc_t *`.
- **Clé inconnue** : `pico8_entity_set_field` (`src/pico8_entity.c:190-203`) appelle l'écrivain, puis `PICO8_MEMBER_CENSUS_WRITE`, puis `assert(written && "entity: a generic write names a field this struct has no member for, a field under no rule of plan-3 section 5 -- stop, do not add a member")`. L'avortement sur nom inconnu vient donc de la couche entité, pas de l'écrivain.
- Accesseur public du descripteur : `pico8_jc_too_far_struct_class()` (`slipway.h:77`). Pas d'accesseur de l'objet : on ne l'atteint que par `jc.et` après un `hq` au-delà de la portée.
- `grep -rn 'offsetof(pico8_jc_too_far_t' src tests` : vide. Pas de fille.
- Lecteurs par nom, **inchangés** : `dl` (`pico8_probe.c`, `pico8_entity_get_field(et_obj, "qs", …)`) ; `test_pico8_jc.c` (`is_too_far`, `test_hq`, `test_too_far`, `test_too_far_is_a_struct`). Écrivains par nom, **inchangés** : `pico8_jc_hq` et `test_too_far_is_a_struct`.

### Empreinte remesurée

La ligne dit 5 (2, 3). `tests/test_pico8_mz.c` (`"qs"` de `mz.jj` et `"fo"` du `et` de `bv`) et les `"fo"` des tests de planètes sont des homonymes. **Empreinte réelle** (fichiers qui lisent ou écrivent un membre d'un `pico8_jc_too_far_t`) : **3 (2, 1)**. En `src`, `pico8_slipway.c` (direct, et par nom dans `hq`) et `pico8_probe.c` (`dl`, par nom). En tests, `test_pico8_jc.c`.

**Fichiers modifiés : 2**, plus `docs/plan-3.md` : `src/pico8_slipway.c` et `tests/test_pico8_jc.c` (pur ajout). **Aucun TU de `UNIT_TESTS_SLOW` n'est touché.**

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** : lecture par nom de `qs` et `fo`, et aller-retour d'écriture par nom de `fo` (voir Testing Decisions).
2. **Typage** : `const char *qs; pico8_jc_t *fo;` (en-tête, noms et ordre gardés). L'initialiseur statique pose `"-too far-"` et `NULL`.
3. **Écrivain** : il déballe `fo` par une aide `static` du genre `ref:pico8_jc_t` ; nil met `fo` à NULL ; tout autre genre, ou une REF qui n'est pas un `jc`, est l'arrêt « champ sans règle ». Un autre nom rend 0, comme avant, et la couche entité avorte comme avant.
4. **Un test d'arrêt de l'écrivain**, écrit rouge sur l'ancien code (qui accepte tout), vert après.
5. Effet mesurable : `make value-members` passe de 211 à **209** et ne liste plus aucune ligne `pico8_jc_too_far_t`. Aucun pixel ne bouge.
6. **Note « Découvert à 1b.13 »** : la forme de la moitié écrivain de R5, à recopier par 1b.15 et les suivantes.

## User Stories

1. En tant que lecteur de `slipway.c`, je veux `qs` typé `const char *` et `fo` typé `pico8_jc_t *`, afin que ce que tient l'objet « trop loin » se lise sur le type.
2. En tant que `dl` (`probe.c`), je veux lire `qs` par nom, présent, de genre chaîne, même pointeur, afin d'écrire « -too far- » au curseur comme avant.
3. En tant que `jc:kj` et `d_`, je veux que `jc.et` pointe toujours sur le même objet statique, afin que la vérité de `et` (trait couleur 2, pose refusée) ne change pas.
4. En tant que `pico8_jc_hq`, je veux écrire `fo = self` par nom et relire ce même `jc`, par adresse.
5. En tant que second `jc` hors de portée, je veux que mon `hq` écrase le `fo` du premier, comme aujourd'hui (objet partagé).
6. En tant que `test_too_far_is_a_struct`, je veux qu'écrire nil sur `fo` le rende absent, que `qs` reste « -too far- », et qu'écrire `qs` ou `ra` par nom avorte toujours.
7. En tant que lecteur par nom, je veux 0 avec `*out_value` intact pour `fo` NULL et pour tout autre nom.
8. En tant que mainteneur, je veux qu'une écriture de `fo` qui n'est ni nil ni un `jc` (un fixe, un booléen, une planète, un vecteur) arrête l'exécution par un `assert` « champ sans règle » qui nomme `jc_too_far.fo` et `ref:pico8_jc_t`.
9. En tant que mainteneur, je veux que l'écrivain brut rende 0 sans rien toucher pour un nom qu'il n'écrit pas, `qs` compris, et que l'arrêt « clé inconnue » reste celui de `pico8_entity_set_field`, avec son message.
10. En tant qu'auteur de 1b.15 (`bg.m_`), de 1b.18, 1b.19, 1b.20, 1b.22 et des autres étapes à membres `e`, je veux une forme écrite de l'écrivain R5 (branche par nom, aide par genre, nil, `has_`, `assert`, nom inconnu), afin de la recopier sans la redécider.
11. En tant qu'auteur de 1b.35 (`jc`), je veux savoir que `jc_too_far.fo` est un `pico8_jc_t *` et que `jc.fo`, lui, reçoit aussi un `g_`.
12. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
13. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
14. En tant que `plan-pilot`, je veux `make value-members` à 209.
15. En tant que vérificateur, je veux une caractérisation par nom (lecture et aller-retour d'écriture) lancée sur l'ancien code, les mutations du §7 avec leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés comme le ferait un agent `guideline` (hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables), et marqués (G).

### R1–R9 appliquées à `jc_too_far`

| règle | sur `jc_too_far` |
|---|---|
| R1 | `qs` → `const char *` (`chaine`, comme `mz_jj.qs` à 1b.3). `fo` → `pico8_jc_t *` (`ref:pico8_X_t`, pointeur non `const`). Noms et ordre gardés. |
| R2 | Aucun indicateur : deux pointeurs, NULL = nil. La revue des TU lents n'ajoute rien. |
| R3 | Lecture de 1b.3 : l'initialiseur agrégé devient `{ {PICO8_TYPE_TAG_STRUCT, &g_jc_too_far_struct_class}, "-too far-", NULL }`. Pas de remise à nil de `fo` à l'init de module (1a.39, inchangé). |
| R4 | Deux branches `strcmp`, dans l'ordre du typedef. `qs` : NULL → `return 0`, sinon `pico8_value_string(qs)`, même pointeur. `fo` : NULL → `return 0`, sinon `pico8_value_ref(fo)`, même adresse. Tout autre nom → `return 0`. Dans les trois cas de 0, `*out_value` reste intact. (G) La branche NULL de `qs` est ajoutée alors que `qs` n'est jamais NULL (H3) : c'est la lecture uniforme de R4 pour un pointeur (1b.3). |
| R5 | **Moitié écrivain, première forme** : voir la section suivante. Pas de clé de construction (instance statique) : la moitié boucle est vacante. |
| R6 | Vacante : aucune méthode, aucune écriture directe de `qs`/`fo` hors du lecteur, de l'écrivain et de l'initialiseur. `pico8_jc_hq` écrit par nom et ne change pas. |
| R7 | Vacante : lecteur à `strcmp`, sans `offsetof`, sans sélecteur local, sans `pico8_X_member` (1b.6). |
| R8 | `PICO8_MEMBER_CENSUS_SEE(&g_jc_too_far, pico8_jc_too_far_t)` gardé ; `PICO8_MEMBER_CENSUS_WRITE` de `pico8_entity_set_field` inchangé ; `tools/member_census.txt` non édité. |
| R9 | 211 → 209, aucune ligne `pico8_jc_too_far_t`. |

### R5, moitié écrivain : la forme (à recopier)

C'est la puce **Coexistence / Écriture** du §5 (« l'écrivain déballe ; nil éteint la présence ; un genre hors de la ligne est un `assert` »), rendue concrète.

1. **Noms écrits.** L'écrivain garde une branche `strcmp` **par membre dont la ligne a `e`**, dans l'ordre du typedef. Un membre typé seulement `l` (ici `qs`) n'a pas de branche : l'écrire par nom reste « nom inconnu ». Ici, une branche : `fo`.
2. **Déballage par genre de la ligne.** Chaque branche appelle une **aide `static` par genre**, au nom de la struct, dans la forme par valeur de 1b.10 : `<X>_take_<genre>(pico8_value_t value, <T> *out[, int *has])`, puis `return 1`. Par genre :
   - `fixe` : `*out = value.as.fixed` (+ `*has = 1` si le membre a un indicateur) ;
   - `bool` : `*out = value.as.boolean`, 0/1 (+ `*has = 1`) ;
   - `chaine` : `*out = value.as.string`, même pointeur, jamais possédé ;
   - `fonction` : `*out = value.as.function` ;
   - `ref:table` / `ref:liste` : `assert` sur `pico8_type_tag_of` (`TABLE` / `LIST`, 1b.5), puis `*out = value.as.ref` ;
   - `ref:autre` : REF non NULL dont `pico8_type_tag_of` rend `UNKNOWN` (1b.10), puis `*out = value.as.ref` ;
   - **`ref:pico8_X_t`** (ici) : `assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT && ((const pico8_struct_header_t *)value.as.ref)->cls == pico8_X_struct_class() && "…")`, puis `*out = value.as.ref`. Le descripteur est lu par l'accesseur public (précédent : le test de `w` dans `pico8_jc_ce`, `slipway.c`), pas par `&g_X_struct_class`, qui est défini plus bas dans le fichier. Une cible à plusieurs descripteurs (`jc.fo`, `jo.fo`) est tranchée par son propre PRD ;
   - genres mêlés (`{fixe, bool}` de 1b.5, `{bool, chaine}` de 1b.7) : l'aide pose aussi `<m>_is_bool`, ou lit `false` comme `NULL` avec `has_<m>` = 1, comme leur branche de boucle.
3. **Nil éteint la présence.** L'aide pose **d'abord** l'état nil (pointeur à NULL ; non-pointeur à 0 et `has_<m>` = 0 ; `<m>_is_bool` = 0), **puis** sort si `value` est `REF NULL` (forme de 1b.9/1b.10). Pour un pointeur, nil veut dire NULL. Un membre non pointeur **sans** indicateur (nil `jamais`) n'a pas d'état nil : son aide commence par l'`assert` (forme `lq_take_w`), et écrire nil y est l'arrêt.
4. **Hors genre : « champ sans règle ».** Tout autre genre, ou une REF d'une autre cible, tombe dans l'`assert` de l'aide. Son message nomme le membre, le genre de sa ligne et l'arrêt, au format de `layout.c` : `"jc_too_far.fo: a write neither nil nor a jc is off its census row (ref:pico8_jc_t) -- a field under no rule of plan-3 section 5: stop, do not widen the member"`.
5. **Nom inconnu : inchangé.** L'écrivain rend 0 **sans rien toucher** pour tout nom sans branche (ici tout sauf `fo`, `qs` compris) ; `pico8_entity_set_field` avorte ensuite par son propre `assert(written && …)`, message inchangé. (G) Pas d'`assert` dans l'écrivain lui-même : le contrat « rend 0 sur un nom qu'il n'écrit pas » est celui de tous les `set_field` (`bg`, `jc`, `bv_et`, `g_`…) et la couche entité est le seul point d'arrêt. C'est aussi ce qui rend l'écrivain brut testable par valeur de retour.
6. **Crochet** : `PICO8_MEMBER_CENSUS_WRITE` reste dans `pico8_entity_set_field`, après l'écrivain ; il observe la valeur reçue, pas le membre (R8).

**Relation aux aides de boucle de 1b.5/1b.9/1b.10/1b.11.** (G)

- **Même forme, pas les mêmes fonctions.** L'écrivain utilise la forme par valeur de 1b.10, que les aides de 1b.9 (table `ri`) et 1b.11 (lecteur `kind->field`) précèdent d'une lecture de leur source. Ces aides sont `static` dans leur fichier et nomment leur struct dans leur `assert`. On ne les partage pas entre fichiers, et on ne crée pas d'aide générique `pico8_take_*` : le message de l'arrêt doit nommer `struct.membre` et le genre de la ligne, et une aide commune le perdrait (ou prendrait une chaîne de plus pour rien).
- **Partagées dans une même struct.** Quand une struct a à la fois une boucle de clés et un écrivain (candidates : `bg`, `g_`, `mz`, `jo`), la branche de l'écrivain et celle de la boucle appellent **la même aide** pour un même membre : une seule règle de déballage par (struct, membre). Une aide de 1b.9/1b.11 à source (table, lecteur) garde sa forme ; la branche écrivain de sa struct appelle une aide par valeur, que l'aide à source peut appeler après sa lecture, au choix du PRD de cette struct.
- **Ici** : une seule aide, `jc_too_far_take_jc(pico8_value_t value, pico8_jc_t **out)`, appelée par la seule branche de l'écrivain. (G) On la garde même pour un seul appel : c'est la forme que recopient les suivantes, et elle porte la règle « nil d'abord » (point 3) et l'`assert` (point 4) séparés du `strcmp`.

### Type du pointeur `fo` : `pico8_jc_t *` (G)

- **Ligne** : `fo` a le seul genre `ref:pico8_jc_t`, sur les 15 rejeux et les 47 TU recensés.
- **Code** : `g_jc_too_far` n'est atteint que par `pico8_jc_ce`, dont `pico8_jc_hq` est le seul appelant (`grep -rn 'pico8_jc_ce\b' src tests` : la définition, l'appel de `hq`, la déclaration). `pico8_jc_hq` écrit `pico8_value_ref(self)` avec `self` un `pico8_jc_t *`. `bv` (`probe.c`) écrit `fo` sur son propre `et`, qui est une planète, un `jc` de `cg.jc`, `g_bv_et` ou `false`, jamais `g_jc_too_far`. Les tests n'y écrivent que `REF NULL`.
- **Cart** : `self.et.fo=self` de `jc:hq` avec `self` un `jc` ; le `{qs}` n'est l'`et` de personne d'autre.
- Donc **pas de pointeur d'en-tête** (`pico8_struct_header_t *`) : il ne servirait qu'à un `g_` qui n'arrive jamais ici. Le cas `g_` est celui de **`jc.fo`** (1b.35 : `ref:pico8_g__t` en plus), pas de cette struct. Si H2 tombe (un `g_` ou autre chose atteint `jc_too_far.fo`), l'aide avorte : c'est l'arrêt « champ sans règle », pas un élargissement du type.
- `pico8_jc_t` est déclaré dans `slipway.h`, déjà inclus en tête de `slipway.c` : aucun `#include` ni déclaration anticipée à ajouter. `jc` n'est pas encore typé (1b.35) : le pointeur typé n'en dépend pas.

### Identité et partage

L'objet reste **unique et statique** (1a.39) : deux `jc` hors de portée partagent `et`, et le dernier `hq` gagne `fo`. `fo` est **emprunté** : jamais possédé ni libéré. Le contrat au-dessus de l'instance (« `qs` is fixed, only `fo` is written, and the next `hq` overwrites it ») reste exact et n'est pas retouché. Aucun commentaire n'est ajouté : l'aide porte son contrat par son `assert` (CLAUDE.md).

### Preuves

Toutes vides après l'étape, sauf mention :

- `grep -nE 'pico8_value_t (qs|fo);' src/pico8_slipway.c` : avant, les deux membres de `jc_too_far` (le `fo` de `jc` est dans `slipway.h`, hors de ce grep) ; après, **vide** ;
- `grep -rnE 'too_far->(qs|fo)\.(kind|as)' src/` : 1 ligne avant (le test `fo.kind == REF` du lecteur) ; après, vide ;
- `grep -n 'too_far->fo = ' src/pico8_slipway.c` : 1 ligne avant ; après, vide (l'écriture passe par l'aide) ;
- `grep -rn 'offsetof(pico8_jc_too_far_t' src/` : vide avant comme après ;
- `grep -n 'pico8_value_t \*' src/pico8_slipway.c | grep -v 'pico8_value_t \*out_value)' | grep -v 'pico8_value_t \*out)'` : même sortie qu'après 1b.12 (le lecteur à offsets de **`jc`**, 1b.35), sans ligne nouvelle ;
- `grep -c 'off its census row' src/pico8_slipway.c` : celui de 1b.12 (`jc_mouse_point.ra`), plus **un** (`jc_too_far.fo`).

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 211 sur l'arbre de départ (1b.12 commitée), puis 209.
- **H2** : dans les 15 rejeux et dans tous les TU, `jc_too_far.fo` ne reçoit que `REF NULL` ou une REF vers une struct de descripteur `g_jc_struct_class`. Sinon l'aide avorte : arrêt « champ sans règle ».
- **H3** : `qs` n'est jamais NULL (seul l'initialiseur l'écrit, aucune branche d'écrivain) : la branche NULL de `qs` n'est jamais exécutée.
- **H4** : aucun TU de `UNIT_TESTS_SLOW` ne lit ni n'écrit `jc_too_far` par nom (grep ci-dessus : aucun ne nomme la struct, aucun `hq` de `jc` au-delà de la portée).
- **H5** : `HQ_HASH`, `TOO_FAR_HASH`, `DRAW_HASH`, `MEMBERS_HASH` (1b.12) et les empreintes des rejeux de `test_pico8_jc` ne bougent pas ; `test_pico8_mz` (`DRAW_HASH`, `HQ_HASH` de `dl`) non plus ; les 15 empreintes du filet non plus.
- **H6** : le build `-DPICO8_MEMBER_CENSUS` compile sur une struct sans `pico8_value_t` (comme après 1b.3). Il n'est pas lancé.
- **H7** : sous ASan+UBSan, rien de nouveau : aucune allocation, et `pico8_type_tag_of` sur une REF de `jc` lit un en-tête valide.
- **H8** : `test_pico8_jc` reste sous 30 s avec les deux fonctions ajoutées (hors `UNIT_TESTS_SLOW`).
- **H9** : 1b.12, commitée, ne touche pas les lignes de `jc_too_far` ni `test_too_far_is_a_struct` ; les numéros cités glissent peut-être, chaque site est désigné par son contenu.
- **H10** : `pico8_jc_struct_class()` rend `&g_jc_struct_class`, le descripteur de toute instance de `pico8_jc_new` (en-tête dans `ke`, premier membre) ; l'`assert` de l'aide passe donc pour chaque `self` de `pico8_jc_hq`.

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Aucun accès direct à `->qs`/`->fo` dans les tests : ils compilent sur les deux formes.
- **Caractérisation dirigée, figée sur l'ancien code** : `test_too_far_members_by_name`, dans `test_pico8_jc` (rapide, dans `CENSUS_UNIT_TESTS`), lancée par `in_fresh_child` après `test_mouse_point_members_by_name`. Elle lit par le **lecteur brut** (`pico8_jc_too_far_struct_class()->field`, témoin : fixe `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder, et écrit par l'**écrivain brut** (`->set_field`) et par `pico8_entity_set_field`.
  1. `world()`, `origin`, `a = build_jc(origin)`, `hq_over(a, NULL, vec_ref(-640, -640))`, `et = get_ref(a, "et")`, `is_too_far(et)`. `qs` → 1, `STRING`, `"-too far-"`, même `as.string` à une seconde lecture. `fo` → 1, `REF`, adresse `a`. Intrus `""`, `"x"`, `"ra"`, `"p"`, `"et"`, `"header"`, `"has_fo"` → 0, témoin intact.
  2. **Aller-retour d'écriture par nom.** `b = build_jc(origin)` (sans `hq`). `pico8_entity_set_field(et, "fo", REF(b))` → `fo` lu = `b` par adresse, par les deux lecteurs. Écrivain brut `(et, "fo", REF(a))` rend 1 → `fo` = `a`. `pico8_entity_set_field(et, "fo", REF NULL)` → `fo` absent (0, témoin intact) ; `qs` inchangé. Réécrire `REF(b)` → présent, `b`.
  3. **Écrivain brut sur un nom qu'il n'écrit pas** : `(et, n, fixe 1)` rend 0 pour `n` ∈ {`"qs"`, `"ra"`, `""`, `"fo "`}, et ensuite `qs` = `"-too far-"` (même pointeur), `fo` inchangé.
  4. **Partage** : `c = build_jc(origin)`, `hq_over(c, NULL, vec_ref(-640, -640))` → `get_ref(c, "et") == et`, `fo` = `c`.
  5. On plie les retours, genres, la chaîne `qs` et le rang dans `hv` des `fo` lus dans **`FAR_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ. Les adresses sont jugées par `assert`.

  Lancée verte sur l'arbre de départ **avant** le typage (ligne « Runs »). Ses valeurs ne changent pas ensuite.
- **Test d'arrêt de l'écrivain, rouge d'abord** (G) : `test_too_far_writer_stops_off_row`, lancé par `in_fresh_child` après la caractérisation. Sur l'objet obtenu comme en 1, `must_abort` (celui du fichier) sur `pico8_entity_set_field(et, "fo", v)` pour `v` ∈ {fixe 1, `pico8_value_bool(0)`, `pico8_value_string("x")`, REF vers `origin` (une planète : struct, autre descripteur), `vec_ref(1, 1)` (vecteur : tag `UNKNOWN`)}. **Rouge sur l'ancien code** (l'écrivain accepte tout, le fils sort par `_exit(0)`), vert après : ligne « Runs » pour chacun des deux passages. Il n'est pas dans la caractérisation figée, qui ne peut contenir que ce que l'ancien code fait déjà ; il fige le contrat que 1b.15 recopie. Le fils de `must_abort` ne vide rien au recensement (1b.4b) : aucun effet sur la table.
- **Tests existants, inchangés, qui figent déjà beaucoup** : `test_hq` (`et.fo == far`, `HQ_HASH`), `test_too_far` (`TOO_FAR_HASH`, écran « -too far- »), `test_too_far_is_a_struct` (partage, `must_abort(write_qs/write_ra)`, nil → absent), `is_too_far` et `fold_value` (constante `0x7A`). Aucun test de régression (§6) ne lit l'objet.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_jc && ./tmp/test_pico8_jc`. Chacune est notée dans le commit avec le test qui la rougit, ou la justification de son trou.
  1. (§7-1) *indicateur forcé vrai* : pas d'indicateur. Forme la plus proche : la branche `fo` du lecteur ignore NULL et rend `REF NULL` présent. Attendu rouge : caractérisation 2 et `test_too_far_is_a_struct` (`!has(et, "fo")`).
  2. (§7-2) *« présent 0 » là où c'était absent* : **sans objet dans sa lettre**, aucun fixe. Forme voisine, sur l'écrivain : l'aide sort sur nil **sans** poser `*out = NULL` (nil n'éteint plus la présence, l'ancien `jc` reste). Attendu rouge : caractérisation 2 et `test_too_far_is_a_struct`.
  3. (§7-3) *init qui saute un membre typé* : (a) l'initialiseur omet `qs` (NULL). Attendu rouge : `is_too_far` faux → `test_hq`, `test_too_far`, `test_too_far_is_a_struct`, caractérisation 1. (b) `fo` omis : **trou justifié**, un initialiseur agrégé met déjà à NULL un membre omis ; la mutation est identique au code.
  4. Propre à l'étape, *écrivain qui accepte `qs`* : une branche `qs` qui écrit et rend 1. Attendu rouge : caractérisation 3 et `test_too_far_is_a_struct` (`must_abort(write_qs)`).
  5. Propre à l'étape, *`assert` sans descripteur* : l'aide ne vérifie que le tag `STRUCT`. Attendu rouge : `test_too_far_writer_stops_off_row` (cas planète) seul ; les autres cas restent rouges par le tag.
  6. Propre à l'étape, *écrivain muet* : la branche `fo` rend 1 sans appeler l'aide. Attendu rouge : caractérisation 2, `test_hq`, `test_too_far_is_a_struct`.
- **`gcov`** : `test_pico8_jc` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23. Chaque ligne modifiée est exécutée, avec deux trous attendus : la branche `qs == NULL` du lecteur (H3) et l'échec de l'`assert` de l'aide, exécuté seulement dans les fils de `must_abort`, qui avortent sans vider leur compteur (prouvé par le test d'arrêt et la mutation 5). L'initialiseur statique n'est pas une ligne exécutable : prouvé par la mutation 3a. Le rapport va dans le commit.
- **Prior art** : `test_mouse_point_members_by_name` (1b.12, lecteur brut et témoin), `test_jj_qs_by_name` (1b.3, le frère statique), `test_too_far_is_a_struct` (1a.39), `must_abort` de `test_pico8_jc`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_jc` : quelques secondes ; une dizaine de lancements (caractérisation et test d'arrêt sur l'ancien code, nouveau code, 6 mutations, `gcov`) en 3 min environ ;
- `make test` : 20 s à 1 min ;
- `make e2e` : 4 à 6 min ;
- `make sanitize-net` : ~4 min seul ; il n'est **jamais** lancé en même temps que `e2e` ;
- `make value-members` : quelques secondes.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent (aucun n'est touché) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 209`**, sans aucune ligne `pico8_jc_too_far_t`.

`make class-coverage-net` **ne s'applique pas** : aucun constructeur ne change. `jc_too_far` n'a pas de constructeur (objet statique) ; le diff ne touche ni `pico8_jc_struct_new`/`pico8_jc_new`, ni un appel `pico8_class_coverage_record`, ni `KNOWN[]`. S'il en touche un, la cible redevient obligatoire, avec une sortie identique à `tools/class_coverage_net_expected.txt`. `make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») si l'aide de l'écrivain avorte dans un rejeu ou dans un TU hors des fils de `must_abort` (H2), si un membre ou un indicateur est ajouté, si l'écrivain accepte un autre nom que `fo`, ou si le type de `fo` est élargi (pointeur d'en-tête, union) au lieu d'avorter.
- `FAILED:` si `qs` n'est pas `const char *`, si `fo` n'est pas `pico8_jc_t *`, ou si l'objet cesse d'être unique et statique.
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur un membre NULL ou un nom inconnu, ou ne rend pas le même pointeur.
- `FAILED:` si écrire nil ne met pas `fo` à NULL, si l'`assert` de l'aide ne vérifie pas le tag **et** le descripteur, ou si son message ne nomme pas `jc_too_far.fo` et `ref:pico8_jc_t`.
- `FAILED:` si l'écrivain avorte lui-même sur un nom inconnu, ou si le message de l'`assert(written && …)` de `pico8_entity_set_field` change.
- `FAILED:` si un site par nom change (`pico8_jc_hq`, `dl`), si `pico8_jc_hq` ou `pico8_jc_ce` change, ou si `jc`, `jc_mouse_point`, `mz_jj`, `bv_et` ou leurs lecteurs, écrivains ou tables changent.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si le diff d'un test fait autre chose qu'ajouter la caractérisation, le test d'arrêt, `FAR_MEMBERS_HASH` et leurs appels, ou si une valeur figée existante change (`HQ_HASH`, `TOO_FAR_HASH`, `MEMBERS_HASH`, `DRAW_HASH`…), ou si un test de régression (§6) change.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 209, liste encore `pico8_jc_too_far_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` si le diff touche un constructeur, `pico8_class_coverage_record` ou `KNOWN[]` sans une ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.
- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage, si elle lit ou écrit un membre directement, ou si `FAR_MEMBERS_HASH` change après le typage ; si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'ancien code et une verte après.
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 6 mutations, chacune avec son test ou la justification de son trou.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.13 », si elle ne donne pas la forme de l'écrivain R5 (points 1 à 5), ou s'il manque le tableau « Runs ».

## Out of Scope

- `jc.et`, `jc.fo` et les autres membres de `jc` (1b.35), dont le cas `g_` de `jc.fo`.
- `bv_et.fo` (1b.22), `mz_jj` (1b.3, fait), `jc_mouse_point` (1b.12).
- Une aide de déballage partagée entre fichiers ; retoucher les aides de 1b.5 à 1b.12.
- Revenir à un objet par appel, ou remettre `fo` à nil à l'init.
- Remplacer une lecture ou écriture par nom par un accès direct ; régénérer `tools/member_census.txt`.
- Les fichiers d'événements et `FRAMES_EVENTS` ; tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - module : `slipway.c` seul, un commit, sans découpage ;
  - tests : `test_pico8_jc` étendu (caractérisation + test d'arrêt), sans nouveau TU ;
  - caractérisation dirigée gardée malgré le « sans genre » levé ;
  - `fo` → `pico8_jc_t *`, pas de pointeur d'en-tête ;
  - écrivain : aide par valeur (forme 1b.10), nil d'abord, `assert` tag + descripteur ; nom inconnu laissé à la couche entité ;
  - aides partagées dans une struct, jamais entre fichiers ;
  - un `must_abort` nouveau, rouge d'abord (premier écart à « pas de `must_abort` nouveau » de 1b.5 à 1b.12, parce que l'`assert` est ici atteignable par un appel public) ;
  - `class-coverage-net` non lancé.
- **Note « Découvert à 1b.13 »**, sur la ligne 1b.13 de `docs/plan-3.md` : « « sans genre » levé par 1b.4b (`qs` `chaine`, `fo` `ref:pico8_jc_t`), caractérisation dirigée gardée ; `fo` → `pico8_jc_t *` (seul `jc:hq` y écrit ; le `g_` de 1b.4b vise `jc.fo`, 1b.35) ; **R5, forme écrivain** : une branche `strcmp` par membre `e`, ordre du typedef ; chacune appelle une aide `static` par genre, forme par valeur de 1b.10 (`<X>_take_<genre>(value, &<m>[, &has_<m>])`), qui pose d'abord nil (NULL, ou 0 et `has_` = 0), sort sur `REF NULL`, sinon `assert` « champ sans règle » nommant `struct.membre` et le genre de la ligne (`ref:pico8_X_t` : tag `STRUCT` et descripteur par `pico8_X_struct_class()`), puis recopie ; un membre sans indicateur avorte sur nil ; un membre `l` seul n'a pas de branche ; nom sans branche → `return 0` sans rien toucher, l'arrêt reste celui de `pico8_entity_set_field` ; aide partagée avec la boucle de clés de la même struct, jamais entre fichiers ; un `must_abort` rouge d'abord fige l'arrêt. R7 vacante ; empreinte réelle 3 (2, 1). » Le reste va dans le commit.
- Le message de commit porte le périmètre, le compte 211 → 209, le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous, `FAR_MEMBERS_HASH` et les réponses à H1 à H10.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
