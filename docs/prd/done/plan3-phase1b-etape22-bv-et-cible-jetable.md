---
title: "Plan 3, étape 1b.22 : `bv_et` (la cible jetable de `bv`) — `fo` devient un `pico8_struct_header_t *`, premier membre REF à plusieurs cibles, et l'écrivain R5 vérifie un ensemble de classes"
description: "`pico8_bv_et_t` a un membre `pico8_value_t`, `fo` (`ref:` vers 8 typedefs, nil observé, lu et écrit par nom). Le typer selon le patron R1–R9 de 1b.1 fait passer `make value-members` de 170 à 169. C'est le premier membre REF dont la ligne a plusieurs cibles. Ce PRD tranche le type (`pico8_struct_header_t *`), l'ensemble admis (les 11 classes dont `hq` lance `bv`, soit les 8 de la ligne plus `fv`, `nl`, `pm`, prouvées par le code et par une caractérisation dirigée) et la façon dont l'aide R5 le vérifie (prédicat `static` sur le descripteur : `methods[PICO8_METHOD_HQ] == pico8_bv_dispatch`, ou `mz`)."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.22 — `bv_et` (`probe.c:52`), accès par nom seul, `fo` écrit par nom, cible de `qh.et`/`mz.et`. Empreinte 5 (2, 3). Compte : 169.

Cadre : les puces **Nil**, **REF**, **Coexistence** (« Écriture : l'écrivain déballe ; nil éteint la présence ; un genre hors de la ligne de recensement du membre est un `assert` ») et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` seulement si un constructeur change). Le **patron R1–R9** est celui de « Découvert à 1b.1 ». Il est cité, pas redécidé. S'appliquent aussi, sans être redérivées, les notes « Découvert à » de `docs/plan-3.md` :

- **1b.1, puis 1b.3 à 1b.20** : patron appliqué. En particulier :
  - **1b.3** : sur une instance statique sans constructeur, R3 se lit sur l'initialiseur agrégé. C'est ici `g_bv_et`, comme `g_mz_jj_wrapper` (1b.3) et `g_jc_too_far` (1b.13) ;
  - **1b.4** : typedef d'un `.c`, sans table d'offsets ;
  - **1b.4b** : table corrigée ; **`bv_et.fo` gagne `ref:pico8_g__t`** (`g_` est typé après, en 1b.32) ; ses autres cibles étaient déjà présentes ;
  - **1b.6/1b.8** : un lecteur à `strcmp` qui copie n'est pas un `pico8_X_member` ; R7 est vacante ;
  - **1b.7** : un nil `stat?` d'homonyme ne décide rien ;
  - **1b.10** : la lecture de R5 pour `ref:autre` (tag `UNKNOWN`) sert ici de cas hors genre (un vecteur) ;
  - **1b.11** : un TU lent (voir plus bas) ;
  - **1b.14b** : les cas « union nulle » se construisent par `memset` puis `kind`. Une mutation qui retire l'unique appel d'une aide `static` est arrêtée par `-Werror`, on la plante donc sous une forme qui garde l'appel (`if (0)`) ;
  - **1b.15/1b.18/1b.19** : l'écrivain 1b.13 est recopié sans écart ;
  - **1b.16** : la preuve `grep set_field` se lit au nom, pas au `NULL` positionnel ;
  - **1b.20** : `ref:pico8_X_t` vérifié par l'accesseur public `pico8_X_struct_class()`, et les mutations §7-3 sans objet sont justifiées.
- **1b.13, forme écrivain de R5**, recopiée point par point :
  1. une branche `strcmp` par membre `e`, dans l'ordre du typedef ;
  2. une aide `static` par genre, au nom de la struct, forme par valeur de 1b.10 ;
  3. **nil d'abord** : l'aide pose `*out = NULL`, puis sort sur `REF NULL`, en testant le genre **et** le pointeur ;
  4. sinon, un `assert` « champ sans règle » qui nomme `struct.membre`, le genre de la ligne et l'arrêt. Pour `ref:pico8_X_t`, il vérifie le genre REF, le tag `STRUCT` **et** le descripteur ;
  5. un nom sans branche donne `return 0` sans rien toucher, et l'arrêt reste l'`assert(written && …)` de `pico8_entity_set_field` ;
  6. **chaque test d'arrêt porte les cas non-REF à union entièrement nulle, construits par `memset` puis `kind`** (1b.13 `write_fo_zero_false`, 1b.14b, 1b.15 `zero_fixed`/`zero_string`/`zero_false`, 1b.18 `zero_function`) ;
  7. un `must_abort` nouveau, rouge sur l'ancien code, fige l'arrêt.
- **1b.11, règle R2 des TU lents** : une lecture par nom faite seulement par un TU de `UNIT_TESTS_SLOW` compte comme `l`, et le nil qu'elle lit compte comme `lu`. Le PRD relit donc les TU lents qui touchent sa struct (voir « Revue des TU lents »).
- **1b.13, point 2** : « Une cible à plusieurs descripteurs (`jc.fo`, `jo.fo`) est tranchée par son propre PRD. » **C'est le cas ici, et c'est le premier.**

### État de départ

HEAD `18d024e` (PRD de 1b.21). **1b.21 (`iv`) est en cours d'implémentation. Précondition : elle est commitée avant que 1b.22 commence.** Elle touche `msgbox.[ch]` et ses tests, pas `src/pico8_probe.c` ni `tests/test_pico8_g_.c` (H12). Dans `docs/plan-3.md`, 1b.21 écrit sa ligne et 1b.22 la sienne. `make value-members` rend alors 170 (H1). Les numéros de ligne cités sont ceux de HEAD et peuvent glisser. Chaque site est aussi désigné par son contenu.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H16).

### Ligne de recensement (`tools/member_census.txt:131`)

| membre | accès | genres | nil | preuves-nil | par-nom |
|---|---|---|---|---|---|
| `fo` | nom | `ref:pico8_fr_t`, `ref:pico8_g__t`, `ref:pico8_hj_t`, `ref:pico8_mx_t`, `ref:pico8_mz_t`, `ref:pico8_nr_t`, `ref:pico8_os_t`, `ref:pico8_qh_t` | observé | lu,stat?,écrit | **le** |

Un membre : 170 − 1 = **169**.

- **Huit genres, tous `ref:pico8_X_t`.** Aucun scalaire, aucun `bool` : ce ne sont pas des « genres mêlés » au sens de 1b.5/1b.7/1b.16. C'est un seul genre R1 (`ref:pico8_X_t`) à plusieurs X, ce que R1 ne tranche pas.
- **Nil** : `écrit` vient de `pico8_entity_set_field(et, "fo", REF NULL)` dans `test_bv_et_is_a_struct` (`test_pico8_g_.c:920`). `lu` vient du `!has(et, "fo")` qui suit. `fo` est un pointeur, donc R2 ne donne pas d'indicateur (NULL = nil).
- Lignes voisines, **non typées ici** : `pico8_qh_t.et` (`:97`), `pico8_g__t.et` (`:153`) et `pico8_mz_t.et` (`:272`) portent `ref:pico8_bv_et_t`. Ce sont les membres qui pointent sur l'objet (1b.24, 1b.32, `mz`).

**Revue des TU lents (règle R2 de 1b.11).** Relu : `test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`.
- `qh`, `os` et `hj` reconnaissent l'objet (`is_bv_target`, `:177`/`:165`/`:177`) seulement pour plier la constante `FROZEN_INSTANCE_TAG` dans `fold_value`. **Aucun ne lit `fo` sur lui.**
- Leurs `"fo"` visent une planète (`test_pico8_qh.c:450`, `get_ref(planet, "fo")`, et les listes `written[]`).
- `pm`, `mx`, `nr`, `ie` et `jo` ne nomment ni `bv_et` ni `is_bv_target`.

Conclusion : **aucune lecture par nom de `bv_et.fo` dans un TU lent**, nil ou non (H9). De toute façon, un pointeur n'a pas d'indicateur.

### Qui écrit `fo` sur cet objet

**Le cart** (`pico8/lua.lua`) :
- `bv` (`:552-559`) finit par `if (self.et)self.et.fo=self`, avec `self` celui dont `hq` est `bv` ;
- `g_.hq=bv` (`:722`) et `qh.hq=bv` (`:915`) ; `mz:hq(...)` (`:767`) appelle `bv(self,...)` en tête ;
- par héritage (`jr`), `fv=g_:jr` (`:1143`) ; `hj`, `nl`, `os`, `nr` de `qh:jr` (`:998`, `:1057`, `:1095`, `:1101`) ; `pm=hj:jr` (`:1017`) ; `fr` et `mx` sont des filles de `qh` (glossaire).
- `hq` n'est appelé que sur l'objet tenu (`mv(held,"hq",…)`, `:244`).

**Le code** (`src/`) :
- `grep -rn '\[PICO8_METHOD_HQ\] = pico8_bv_dispatch' src/` donne **10 descripteurs** : `g_` (`probe.c`), `fv` (`help.c`), `qh`, `nl`, `os`, `hj`, `pm` (`planet_family.c`), `nr`, `fr`, `mx` (`planet_family_trade.c`).
- `grep -rn 'pico8_bv(' src/` donne l'appel direct de `pico8_mz_hq` (`wormhole.c:112`, `self` un `mz`) et celui de `pico8_bv_dispatch`.
- `pico8_bv` écrit `pico8_entity_set_field(et.as.ref, "fo", pico8_value_ref(self))` avec `self` son `pico8_struct_header_t *`.
- Côté tests, seul `test_bv_et_is_a_struct` écrit `fo` sur l'objet, et seulement `REF NULL`. `test_pico8_fv.c:635` (`call_bv`) passe une table à `pico8_bv`, qui avorte avant (`assert` STRUCT), et c'est inchangé.

**Écart entre la ligne et le code : `fv`, `nl`, `pm`.** Ils ont `bv` pour `hq`, mais aucun rejeu recensé et aucun TU recensé ne les tient au-dessus du vide hors du panneau :
- `pm` n'est touché que par `test_pico8_pm`, lent, que le recensement ne passe pas ;
- `nl` (le trading hub) et `fv` n'y passent pas par cette branche.

Dans le jeu, tenir un trading hub au-dessus du vide y mène. Voir « Ensemble admis ».

### Le code aujourd'hui (`src/pico8_probe.c`)

- **Typedef** (`:54-57`) : `{ pico8_struct_header_t header; pico8_value_t fo; } pico8_bv_et_t`. Il est local au `.c`, sans fille ni `offsetof`.
- **Lecteur** `pico8_bv_et_field` (`:59-67`) : `strcmp(name, "fo") != 0` ou `fo` = `REF NULL` donne `return 0` ; sinon il copie `fo`.
- **Écrivain** `pico8_bv_et_set_field` (`:69-77`) : `strcmp(name, "fo") != 0` donne `return 0` ; sinon `et->fo = value` (n'importe quel genre), puis `return 1`.
- **Descripteur** `g_bv_et_struct_class` (`:79-83`) : aucune méthode, un lecteur et un écrivain.
- **Instance** `g_bv_et` (`:85-87`), avec son contrat (« never allocated, never freed; only `fo` is written, and the next `bv` overwrites it ») : `{{PICO8_TYPE_TAG_STRUCT, &g_bv_et_struct_class}, {PICO8_VALUE_REF, {0}}}`. Elle n'est jamais remise à nil (décision de 1a.38). Elle est signalée au recensement par `PICO8_MEMBER_CENSUS_SEE(&g_bv_et, pico8_bv_et_t)` dans `pico8_probe_init`.
- **Seule pose de l'objet** : `pico8_bv`, dernière branche (`et = pico8_value_ref(&g_bv_et)`).
- **Accesseur public** : `pico8_bv_et_struct_class()` (`probe.h:117`).
- **Lecteurs par nom de `fo` sur l'objet, inchangés** : `test_pico8_g_` (`fold_hq` `:511`, `test_bv_et_is_a_struct` `:915`/`:921`) et `test_pico8_mz` (`fold_hq` `:407-409`). `dl` lit `qs` sur `et` et le trouve absent. Ce n'est pas un membre.
- **Édition de liens** : `DATA_ENGINE_SRCS` (`Makefile:219`) lie `pico8_probe.c` **sans** `pico8_help.c`, `pico8_planet_family_trade.c`, `pico8_research_icon.c`…, mais avec `pico8_wormhole.c` et `pico8_planet_family.c`. C'est le cas de `test_pico8_data_init`, `jb_ol`, `planet_kj_dispatch`, `bg_kj` et de leurs variantes `refaudit`. **`probe.c` ne peut donc pas appeler `pico8_fv_struct_class()`, `pico8_nr_struct_class()`, `pico8_fr_struct_class()` ni `pico8_mx_struct_class()`** sans casser ces liens (fait statique, lu dans le `Makefile`).

### Empreinte remesurée

La ligne dit 5 (2, 3), ce qui compte les homonymes `"fo"` de `jc`, `jc_too_far` et `jo`. **Empreinte réelle** (fichiers qui lisent ou écrivent un membre d'un `pico8_bv_et_t`) : **3 (1, 2)**.
- En `src` : `pico8_probe.c` (direct, et par nom dans `bv`).
- En tests : `test_pico8_g_.c` et `test_pico8_mz.c`.

**Fichiers modifiés : 2**, plus `docs/plan-3.md` : `src/pico8_probe.c` et `tests/test_pico8_g_.c` (pur ajout). **Aucun TU de `UNIT_TESTS_SLOW` n'est touché.** `test_pico8_mz.c` a un diff vide.

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** : lecture par nom de `fo`, et aller-retour d'écriture par nom **avec chacune des 11 classes admises**, par `bv` et par écriture directe par nom (voir Testing Decisions).
2. **Typage** : `pico8_struct_header_t *fo;` (en-tête, nom et ordre gardés). L'initialiseur statique pose `NULL`.
3. **Écrivain** : il déballe `fo` par une aide `static` :
   - nil met `fo` à NULL ;
   - tout autre genre, une REF qui n'est pas une struct, ou une struct dont la classe n'est pas admise, est l'arrêt « champ sans règle » ;
   - un autre nom rend 0, comme avant, et la couche entité avorte comme avant.
4. **L'ensemble admis est vérifié par un prédicat `static` sur le descripteur**, sans nouvelle dépendance d'édition de liens.
5. **Un test d'arrêt de l'écrivain**, écrit rouge sur l'ancien code (qui accepte tout), vert après. Il porte les cas à union nulle et des classes non admises.
6. Effet mesurable : `make value-members` passe de 170 à **169** et ne liste plus aucune ligne `pico8_bv_et_t`. Aucun pixel ne bouge.
7. **Note « Découvert à 1b.22 »** : la forme d'un membre REF à plusieurs cibles, que `jo.fo` (1b.23), `qh.et` (1b.24), `g_.et` (1b.32), `jc.fo`/`jc.et` (1b.35) et `mz.et` recopient.

## User Stories

1. En tant que lecteur de `probe.c`, je veux `fo` typé comme un pointeur d'en-tête d'entité, afin de voir sur le type que la cible jetable pointe en retour vers celui qui la tient.
2. En tant que `pico8_bv`, je veux écrire `fo = self` par nom, pour chacune des 11 classes dont `hq` est `bv`, et relire le même objet, par adresse.
3. En tant que joueur qui tient un trading hub (`nl`), une protostar (`pm`) ou tout autre bâtiment au-dessus du vide hors du panneau, je veux que rien n'avorte, comme aujourd'hui.
4. En tant que joueur qui tient une sonde au-dessus du vide, je veux le sprite 31 au curseur, la pose refusée et le cercle de `mz` caché, comme aujourd'hui.
5. En tant que `test_pico8_g_` et `test_pico8_mz`, je veux lire `fo` par nom et trouver celui qui vient d'appeler `hq`, empreintes inchangées.
6. En tant que `test_bv_et_is_a_struct`, je veux qu'écrire nil sur `fo` le rende absent, et qu'écrire `qs` par nom avorte toujours.
7. En tant que lecteur par nom, je veux 0 avec `*out_value` intact pour `fo` NULL et pour tout autre nom.
8. En tant que mainteneur, je veux qu'une écriture de `fo` qui n'est ni nil ni une classe admise arrête l'exécution par un `assert` « champ sans règle » qui nomme `bv_et.fo`. Sont visés : un fixe, un booléen, une chaîne, une fonction, un vecteur, une table, une liste, une planète nue `jo`, un `jc`, l'objet lui-même, le `jj` de `mz`.
9. En tant que mainteneur, je veux que les valeurs non-REF à union nulle (`false`, fixe 0, chaîne NULL, fonction NULL) avortent au lieu de passer pour nil.
10. En tant que mainteneur, je veux que l'écrivain brut rende 0 sans rien toucher pour un nom qu'il n'écrit pas, et que l'arrêt « clé inconnue » reste celui de `pico8_entity_set_field`, avec son message.
11. En tant qu'auteur de 1b.23 (`jo.fo`), 1b.24 (`qh.et`), 1b.32 (`g_.et`), 1b.35 (`jc.fo`, `jc.et`) et de `mz.et`, je veux une forme écrite d'un membre REF à plusieurs cibles (type, ensemble admis, prédicat, message), afin de la recopier sans la redécider.
12. En tant qu'auteur de ces étapes, je veux savoir quoi faire quand une classe productrice prouvée par le code manque à la ligne de recensement.
13. En tant que gardien de l'édition de liens, je veux que `probe.c` ne dépende d'aucun nouveau fichier, afin que les binaires `DATA_ENGINE_SRCS` lient toujours.
14. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
15. En tant que gardien de la mémoire, je veux `make sanitize-net` propre sur les 15 rejeux.
16. En tant que `plan-pilot`, je veux `make value-members` à 169.
17. En tant que vérificateur, je veux :
    - une caractérisation par nom (lectures, et aller-retour d'écriture avec chaque classe admise) lancée sur l'ancien code ;
    - un test d'arrêt rouge d'abord ;
    - les mutations du §7 avec leurs trous justifiés ;
    - le rapport `gcov` ;
    - le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules et les tests par l'utilisateur. Ces choix sont tranchés comme le ferait un agent `guideline` (hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables), et marqués (G).

### Type du pointeur `fo` : `pico8_struct_header_t *` (G)

La ligne a **huit** cibles (onze admises, voir plus bas). R1 dit `ref:pico8_X_t` → `pico8_X_t *` pour **une** cible. Options :

| option | verdict |
|---|---|
| `pico8_X_t *` d'une des cibles | **rejetée** : faux pour les dix autres, et chaque lecture devrait mentir par un cast. |
| un pointeur par cible (`fo_g_`, `fo_qh`…) ou une union étiquetée | **rejetée** : onze membres ou un tag inventés pour un seul champ du cart, et le lecteur devrait choisir. Cela contredit « nom minifié et place gardés » (R1). |
| `void *` | **rejetée** : perd la garantie que la cible commence par un en-tête (tag, descripteur), que l'aide vient de vérifier. |
| `pico8_ke_t *` (base commune de fait : `g_`, `mz` et `jo` enchâssent `ke` en premier) | **rejetée** : `ke` ne restreint rien d'utile (`jo`, `jc` et `ga` l'ont aussi, et ils sont refusés), et aucun code ne lit `me`/`t` à travers `fo`. Il faudrait un cast à l'écriture, là où `pico8_bv` tient déjà `self` en `pico8_struct_header_t *`. |
| **`pico8_struct_header_t *`** | **retenue** : c'est le type sous lequel l'unique producteur tient l'objet (`pico8_bv(pico8_struct_header_t *self, …)`), celui des REF d'entité dans tout le port (`pico8_probe_field_instance`, `pico8_je`…), et le seul type commun que l'aide peut vérifier sans cast faux. La restriction aux classes admises est portée par l'`assert` de l'aide, pas par le type. |

Pointeur non `const` (R1), **emprunté** : jamais possédé ni libéré. Comme aujourd'hui, il peut pendre après la libération de celui qui le tenait. Aucun lecteur ne le lit sans qu'un `bv` vienne de l'écrire (1a.38), et ce n'est pas retouché (H7).

### Ensemble admis : les 11 classes dont `hq` lance `bv` (G)

**Décision** : la ligne de recensement (`fr`, `g_`, `hj`, `mx`, `mz`, `nr`, `os`, `qh`) **plus `fv`, `nl`, `pm`**. C'est exactement l'ensemble des `self` possibles de `pico8_bv`.

- **Pourquoi pas la ligne seule.** La puce Coexistence dit qu'« un genre hors de la ligne de recensement est un `assert` ». Mais la table est « une entrée de la liste, pas un oracle » (1b.0), et 1b.0 prévoit déjà qu'un membre sans genre observé reçoit une caractérisation dirigée avant d'être typé.
  - Ici, trois classes productrices sont **prouvées par le code** (descripteur à `hq` = `pico8_bv_dispatch`) et **par le cart** (`g_.hq=bv`, `qh.hq=bv`, héritage `jr`). Aucune trace recensée ne les a exercées dans cette branche.
  - Les refuser ferait avorter le jeu quand on tient un trading hub au-dessus du vide hors du panneau. C'est un changement de comportement (§8), et le filet ne le verrait pas.
- **Lecture nouvelle, notée dans « Découvert à 1b.22 ».** Une cible REF absente de la ligne, mais dont le producteur est prouvé par le code, est admise **si et seulement si** la caractérisation dirigée de l'étape l'exerce sur l'ancien code. Ici, `bv` est appelé sur un `fv`, un `nl` et un `pm` neufs, et chacun est relu dans `fo`. Toute autre cible reste l'arrêt « champ sans règle ». On n'élargit pas au-delà de ce que le code produit.
- **Si H4 tombe pour l'un des trois** (son `hq` ne peut pas être mené hors du panneau dans le fils sans autre état), l'aller-retour par nom de la caractérisation le couvre quand même. La preuve par le code (descripteur) reste, et le commit le note. Ce n'est pas un arrêt.
- **Refusés, et figés par le test d'arrêt** : `jo` nu (les `qu`/`re` de `a_slipway`), `jc`, `bv_et` lui-même, `mz_jj`, et tout non-struct (vecteur `UNKNOWN`, table, liste).

### Vérification de l'ensemble : prédicat sur le descripteur (G)

```c
/* Forme décidée par ce PRD (pas un prototype) : */
static int bv_et_is_holder_class(const pico8_struct_class_t *cls) {
    return cls->methods[PICO8_METHOD_HQ] == pico8_bv_dispatch || cls == pico8_mz_struct_class();
}
```

- **Pourquoi pas une liste d'accesseurs** (`cls == pico8_fr_struct_class() || …`), la forme de 1b.13/1b.20 : quatre des onze accesseurs (`fv`, `nr`, `fr`, `mx`) sont définis dans des fichiers que `DATA_ENGINE_SRCS` ne lie pas. Les appeler depuis `probe.c` casse l'édition de liens de quatre TU et de leurs variantes `refaudit`. Rejetées aussi :
  - ajouter ces fichiers à `DATA_ENGINE_SRCS`, ce qui ferait une cascade de dépendances et un diff au `Makefile` hors sujet ;
  - des `extern` faibles, trop malins.
- **Pourquoi ce prédicat est exact** : il dit ce que dit le cart. `fo` est écrit par `bv(self)`, et `self` est un objet dont `hq` **est** `bv` (10 descripteurs, preuve `grep` ci-dessous), ou un `mz`, dont `hq` appelle `bv` (`pico8_mz_struct_class()`, défini dans `wormhole.c`, que `DATA_ENGINE_SRCS` lie). `pico8_bv_dispatch` est défini dans `probe.c` même. Une classe future dont `hq` serait `bv` serait admise, et c'est la règle du cart.
- **Ensemble prouvé égal à 11** par deux moyens :
  - la preuve `grep` (10 lignes `[PICO8_METHOD_HQ] = pico8_bv_dispatch`, plus `mz`) ;
  - la caractérisation, qui écrit une REF de chacune des 11 classes et la relit. Le test compare aussi chaque descripteur par les accesseurs publics (`test_pico8_g_` lie `ENGINE_SRCS`).
- **`#include`** : `pico8_wormhole.h` seul est ajouté à `probe.c`. Aucun en-tête de `help`, `planet_family` ou `planet_family_trade`.

### R1–R9 appliquées à `bv_et`

| règle | sur `bv_et` |
|---|---|
| R1 | `fo` → `pico8_struct_header_t *` (lecture nouvelle pour `ref:` à plusieurs cibles, ci-dessus). Nom et ordre gardés. |
| R2 | Aucun indicateur : pointeur, NULL = nil. La revue des TU lents n'ajoute rien (H9). |
| R3 | Lecture de 1b.3 : l'initialiseur agrégé devient `{ {PICO8_TYPE_TAG_STRUCT, &g_bv_et_struct_class}, NULL }`. Pas de remise à nil de `fo` à l'init de module (1a.38, inchangé). |
| R4 | Une branche `strcmp` : `fo` NULL → `return 0`, `*out_value` intact ; sinon `pico8_value_ref(fo)`, même adresse. Tout autre nom → `return 0`, témoin intact. |
| R5 | **Écrivain, forme 1b.13** : une branche `if (strcmp(name, "fo") == 0) { bv_et_take_holder(value, &et->fo); return 1; }`, puis `return 0`. Pas de clé de construction (instance statique), donc la moitié boucle est vacante. Détail ci-dessous. |
| R6 | Vacante : aucune méthode, aucune écriture directe de `fo` hors du lecteur, de l'écrivain et de l'initialiseur. `pico8_bv` écrit par nom et ne change pas. |
| R7 | Vacante : lecteur à `strcmp`, sans `offsetof`, sans sélecteur, sans `pico8_X_member` (1b.6). |
| R8 | `PICO8_MEMBER_CENSUS_SEE(&g_bv_et, pico8_bv_et_t)` gardé ; `PICO8_MEMBER_CENSUS_WRITE` inchangé ; `tools/member_census.txt` non édité. |
| R9 | 170 → 169, aucune ligne `pico8_bv_et_t`. |

### L'aide R5 : `bv_et_take_holder` (G)

`static void bv_et_take_holder(pico8_value_t value, pico8_struct_header_t **out)`. Le nom suit « aide au nom de la struct, par genre ». Le genre « à plusieurs cibles » est nommé par son rôle, `holder` (glossaire : `fo`, « accrochée par »).

1. `*out = NULL;`
2. `if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) return;` : le genre **et** le pointeur sont testés.
3. `assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT && bv_et_is_holder_class(((const pico8_struct_header_t *)value.as.ref)->cls) && "…");`. Les trois conjonctions sont évaluées dans cet ordre : le tag est vérifié avant de lire `cls`.
4. `*out = value.as.ref;`

**Message**, au format de `layout.c`/`slipway.c`, qui nomme `bv_et.fo`, l'ensemble et l'arrêt :

`"bv_et.fo: a write neither nil nor an object whose hq runs bv (g_, fv, mz, qh, nl, os, hj, pm, nr, fr, mx) is off its census row (ref: to those holders) -- a field under no rule of plan-3 section 5: stop, do not widen the member"`

Aucun commentaire ajouté (CLAUDE.md) : l'`assert` porte le contrat. Le contrat au-dessus de `g_bv_et` reste exact et n'est pas retouché.

### Preuves

Toutes vides après l'étape, sauf mention :

- `grep -n 'pico8_value_t fo;' src/pico8_probe.c` : 1 ligne avant ; **vide** après.
- `grep -nE 'et->fo\.(kind|as)' src/pico8_probe.c` : 1 ligne avant (le lecteur) ; vide après.
- `grep -n 'et->fo = ' src/pico8_probe.c` : 1 ligne avant ; vide après (l'écriture passe par l'aide).
- `grep -rn 'offsetof(pico8_bv_et_t' src/` : vide avant comme après.
- `grep -c 'off its census row' src/pico8_probe.c` : 1 avant (`g__take_fixed`) ; **2** après.
- `grep -rn '\[PICO8_METHOD_HQ\] = pico8_bv_dispatch' src/` : **10 lignes**, avant comme après (`help.c` 1, `planet_family.c` 5, `planet_family_trade.c` 3, `probe.c` 1).
- `grep -rn 'pico8_bv(' src/ | grep -v 'void pico8_bv('` : `wormhole.c` (`pico8_mz_hq`) et `probe.c` (`pico8_bv_dispatch`), inchangé.
- `git diff -U0 src/pico8_probe.c | grep '^+#include'` : **une** ligne, `pico8_wormhole.h`.
- `grep -nE 'pico8_(fv|nr|fr|mx|qh|nl|os|hj|pm)_struct_class' src/pico8_probe.c` : **vide** (H11).
- `make value-members | grep -c pico8_bv_et_t` : 1 avant ; 0 après.

### Hypothèses (rien n'a été lancé)

- **H1** : `make value-members` rend 170 sur l'arbre de départ (1b.21 commitée), puis 169.
- **H2** : dans les 15 rejeux et dans tous les TU, `bv_et.fo` ne reçoit que `REF NULL` ou une REF vers une struct d'une des 11 classes. Sinon l'aide avorte : arrêt « champ sans règle ».
- **H3** : après `start_game(DRAG)`, dans le fils, `pico8_X_new(NULL)` construit une instance valide pour `qh`, `nl`, `os`, `hj`, `pm`, `nr`, `fr`, `mx`, `mz`, `fv`. Sinon l'implémenteur reprend les clés minimales du TU propre à la classe, et le note au commit.
- **H4** : `hq` appelé sur chacune au point `middle + (0, 3)` de `a_slipway`, avec `rp` = (2, 2), prend la dernière branche de `bv` :
  - aucune planète à moins de `dw` (`dw` ≤ 0.025 dans le cart, point à 3 unités) ;
  - aucun `jc` sous le point ;
  - hors du panneau.
  Donc `fo` de l'objet est l'appelant. Pour `mz`, `pico8_mz_hq` peut ensuite remplacer son `et` par le `jj`, mais `fo` de l'objet est déjà écrit.
- **H5** : aucune valeur figée ne bouge. Cela vaut pour `BUILT_HASH`, `LIFE_HASH`, `EY_*`, `DRAG_HASH` et `FV_HASH` de `test_pico8_g_`, pour les empreintes de `test_pico8_mz` et les plis de `qh`/`os`/`hj`, et pour les 15 empreintes du filet.
- **H6** : le build `-DPICO8_MEMBER_CENSUS` compile sur une struct sans `pico8_value_t` (comme après 1b.3 et 1b.13). Il n'est pas lancé.
- **H7** : sous ASan+UBSan, rien de nouveau. Aucune allocation, et `fo` qui pend n'est jamais lu.
- **H8** : `test_pico8_g_` reste sous 30 s avec deux fils de plus (hors `UNIT_TESTS_SLOW`).
- **H9** : aucun TU lent ne lit `bv_et.fo` par nom (revue ci-dessus).
- **H10** : chaque instance de `pico8_X_new` porte le descripteur de sa classe (`struct_init(…, &g_X_struct_class)`). `cls->methods[PICO8_METHOD_HQ]` y vaut l'adresse de `pico8_bv_dispatch` pour les 10 descripteurs, comparable par `==` (liaison statique, un seul symbole).
- **H11** : les binaires `DATA_ENGINE_SRCS` et `main_raylib` lient après l'étape (`probe.c` n'appelle que `pico8_mz_struct_class`, défini dans `wormhole.c`).
- **H12** : 1b.21, commitée, ne touche ni `pico8_probe.c` ni `test_pico8_g_.c`. Les numéros cités glissent peut-être.
- **H13** : `bv_et_is_holder_class` et `bv_et_take_holder`, chacune appelée une fois, ne déclenchent aucun avertissement (`-Werror`).
- **H14** : `test_bv_et_is_a_struct`, inchangé, passe : l'écriture de `REF NULL` passe par l'aide et met `fo` à NULL.
- **H15** : `pico8_bv_dispatch` (type `pico8_value_function_t`) se compare à `methods[…]` sans cast.
- **H16** : les appels à `hq` de la caractérisation, dans un fils `in_fresh_child`, ne modifient pas d'état vu par les autres cas (chaque cas a son fils).

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Aucun accès direct à `->fo` dans les tests : ils compilent sur les deux formes.
- **(G) Où : `test_pico8_g_`**. Il est rapide, dans `CENSUS_UNIT_TESTS` (la prochaine régénération de la table y verra `fv`, `nl`, `pm`), lie `ENGINE_SRCS` et contient déjà `test_bv_et_is_a_struct`, `is_bv_target`, `a_slipway`, `mouse_at`, `must_abort`. On ajoute les `#include` de `pico8_planet_family.h`, `pico8_planet_family_trade.h` et `pico8_wormhole.h`. Rien au Makefile.
- **Caractérisation dirigée, figée sur l'ancien code** : `test_bv_et_members_by_name`, lancée par `in_fresh_child` après `test_bv_et_is_a_struct`.
  - Elle lit par le **lecteur brut** (`pico8_bv_et_struct_class()->field`, témoin : fixe `0x1234`) et par `pico8_entity_get_field`, qui doivent concorder.
  - Elle écrit par l'**écrivain brut** (`->set_field`) et par `pico8_entity_set_field`.
  - Mise en place : `start_game(DRAG)` ; `a = ldown()` ; `a_slipway(&middle)` ; `P = middle + (0, 3)` ; `hq` de `a` en `mouse_at(P)`, `rp` (2, 2) ; `T = get_ref(a, "et")`, `is_bv_target(T)`.
  1. **Lectures.** `fo` → 1, `REF`, adresse `a`. Intrus `""`, `"x"`, `"FO"`, `"fo "`, `"qs"`, `"et"`, `"header"`, `"cls"` → 0, témoin intact.
  2. **`bv` avec chaque classe admise.** `H` = liste de 11 : `a` (`g_`), puis `pico8_fv_new`, `pico8_mz_new`, `pico8_qh_new`, `pico8_nl_new`, `pico8_os_new`, `pico8_hj_new`, `pico8_pm_new`, `pico8_nr_new`, `pico8_fr_new`, `pico8_mx_new`, avec `NULL` (H3).
     - Le test vérifie d'abord que chaque descripteur est l'accesseur public attendu (`pico8_X_struct_class()`).
     - Pour chaque `h` : `hq` de `h` en `mouse_at(P)`, `rp` (2, 2).
     - Attendu : `fo` de `T` = `h` par adresse, par les deux lecteurs. `get_ref(h, "et")` vaut `T`, sauf pour `mz`, dont l'état de `et` (objet, ou `jj`) est plié tel quel (H4).
  3. **Aller-retour d'écriture par nom, chaque classe admise.** Pour chaque `h` de `H`, en ordre inverse :
     - `pico8_entity_set_field(T, "fo", REF(h))`, puis `fo` lu = `h` par les deux lecteurs ;
     - écrivain brut `(T, "fo", REF(a))` rend 1, puis `fo` = `a`.
     Puis nil : `REF NULL` → absent (0, témoin intact). REF à union nulle (`memset`, `kind = REF`) → absent. `REF NULL` encore → absent. `REF(h₀)` → présent.
  4. **Écrivain brut sur un nom qu'il n'écrit pas** : `(T, n, REF(a))` rend 0 pour `n` ∈ {`"qs"`, `"et"`, `""`, `"fo "`, `"FO"`}. Ensuite `fo` est inchangé.
  5. **Partage** : `get_ref(a, "et") == T` après tous les `hq` (objet unique et statique, 1a.38).
  6. On plie retours, genres, présences, rang de chaque classe dans `H` pour le `fo` lu, et l'état de `et` de `mz` dans **`BV_ET_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ. Les adresses sont jugées par `assert`, jamais pliées.

  Lancée verte sur l'arbre de départ **avant** le typage (ligne « Runs »). Ses valeurs ne changent pas ensuite.
- **Test d'arrêt, rouge d'abord** (G) : `test_bv_et_writer_stops_off_row`, lancé par `in_fresh_child` après la caractérisation.
  - Il utilise un utilitaire `must_abort_value(void *e, void (*write)(void *, pico8_value_t), pico8_value_t v)` ajouté au fichier (forme de `test_pico8_bg`, `fork` + `SIGABRT`), avec `write` = `pico8_entity_set_field(e, "fo", v)`.
  - `T` est obtenu comme en 1.
  - Valeurs à union nulle par `memset(&v, 0, sizeof v)` puis `v.kind = …`.
  - Cas :
    - **non-REF** : `pico8_value_fixed(1)` ; **`zero_fixed`** ; `pico8_value_bool(1)` ; **`zero_false`** ; `pico8_value_string("x")` ; **`zero_string`** ; **`zero_function`** ;
    - **REF non-struct** : un vecteur du tas (tag `UNKNOWN`), une table (`pico8_ob_table_new()`), `hv` (liste) ;
    - **struct de classe non admise** : le `qu` de `a_slipway` (`jo` nu, `pico8_jo_new`), le `jc` de `a_slipway`, `T` lui-même (`bv_et`), `pico8_mz_jj_wrapper()` (`mz_jj`).
  - **Rouge sur l'ancien code** (l'écrivain accepte tout, le fils sort par `_exit(0)`), vert après : une ligne « Runs » pour chacun des deux passages.
  - Il n'est pas dans la caractérisation figée, qui ne contient que ce que l'ancien code fait déjà.
  - La REF à union nulle est nil. C'est le témoin accepté, dans la caractérisation (3), pas ici.
  - Les fils de `must_abort_value` ne vident rien au recensement (1b.4b).
- **Tests existants, inchangés, qui figent déjà beaucoup** :
  - `test_pico8_g_` : `fold_hq` (`fo == e` sur la cible jetable), `test_drag` (`DRAG_HASH`), `test_bv_et_is_a_struct` (absences, `must_abort(write_qs)`, nil → absent), `is_bv_target` et `fold_value` ;
  - `test_pico8_mz` : `fold_hq` (`ET_THROWAWAY`, `fo == mz`) ;
  - les plis `FROZEN_INSTANCE_TAG` de `qh`/`os`/`hj` (lents, lancés par `make e2e`).
  - Aucun test de régression (§6) ne lit l'objet.
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_g_ && ./tmp/test_pico8_g_`. Chacune est notée dans le commit avec le test qui la rougit, ou la justification de son trou.
  1. (§7-1) *indicateur forcé vrai* : pas d'indicateur. Forme la plus proche : la branche `fo` du lecteur ignore NULL et rend `REF NULL` présent. Attendu rouge : caractérisation 3 (nil → absent) et `test_bv_et_is_a_struct` (`!has(et, "fo")`).
  2. (§7-2) *« présent 0 » là où c'était absent* : **sans objet dans sa lettre**, aucun fixe. Forme voisine : l'aide sort sur nil **sans** avoir posé `*out = NULL` (nil n'éteint plus, l'ancien `fo` reste). Attendu rouge : caractérisation 3, `test_bv_et_is_a_struct`.
  3. (§7-3) *init qui saute un membre typé* : **trou justifié**. L'initialiseur agrégé met déjà à NULL un `fo` omis, donc la mutation est identique au code (1b.13, 3b). Il n'y a ni constructeur ni boucle de clés.
  4. *Prédicat qui accepte toute struct* (`return 1;`). Attendu rouge : test d'arrêt (`jo`, `jc`, `bv_et`, `mz_jj`) seul.
  5. *Prédicat sans `mz`* (branche `|| cls == pico8_mz_struct_class()` retirée). Attendu rouge : caractérisation 2 et 3 (le fils avorte au `hq` ou à l'écriture de `mz`) ; hors budget : `test_pico8_mz` `fold_hq`.
  6. *Prédicat sans la branche `hq`* (`mz` seul). Attendu rouge : `fold_hq` de `test_pico8_g_` dès la première sonde, caractérisation 2.
  7. *Tag non vérifié* (conjonction `STRUCT` retirée). Attendu rouge : test d'arrêt, cas vecteur/table/liste (le fils meurt d'un autre signal ou survit : `must_abort_value` exige `SIGABRT`).
  8. *Nil jugé sur le pointeur seul* (`value.as.ref == NULL`). Attendu rouge : test d'arrêt, cas `zero_fixed`, `zero_false`, `zero_string`, `zero_function`.
  9. *Nil jugé sur le genre seul* (`value.kind == PICO8_VALUE_REF`). Attendu rouge : caractérisation 2 et 3 (`fo` absent après une écriture), `fold_hq`.
  10. *Écrivain muet* : `if (0) bv_et_take_holder(…)` et `return 1` (forme 1b.14b). Attendu rouge : caractérisation 2 et 3, `fold_hq`.
  11. *Écrivain qui accepte un autre nom* : une branche `qs` qui rend 1. Attendu rouge : caractérisation 4, `test_bv_et_is_a_struct` (`must_abort(write_qs)`).
  - **Mutant équivalent, non retenu, noté dans le commit** : l'ordre `*out = value.as.ref` avant l'`assert`. Dans un fils qui avorte, l'état écrit n'est jamais relu.
- **`gcov`** : `test_pico8_g_` en `--coverage -O0`, avec le piège `__gcov_dump` des fils `in_fresh_child` de 1a.23. Chaque ligne modifiée est exécutée : lecteur (NULL, présent, autre nom), écrivain (`fo`, autre nom), aide (nil, recopie), les deux branches du prédicat (`hq` vrai, `mz`).
  - Trou attendu : la branche d'échec de l'`assert` de l'aide, exécutée seulement dans les fils de `must_abort_value`, qui avortent sans vider leur compteur. Elle est prouvée par le test d'arrêt et les mutations 4, 7, 8.
  - L'initialiseur statique n'est pas une ligne exécutable (mutation 3).
  - Le rapport va dans le commit.
- **Prior art** :
  - `test_too_far_members_by_name` et `test_too_far_writer_stops_off_row` (1b.13, le frère statique : lecteur et écrivain bruts, témoin, `must_abort`) ;
  - `test_bg_members_by_name` et `test_bg_writer_stops_off_row` (1b.15, `must_abort_value`, `zero_*`) ;
  - `test_bv_et_is_a_struct` (1a.38).

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests au total.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_g_` : quelques secondes à ~20 s (un `start_game(DRAG)` par fils, H8). Il y aura une quinzaine de lancements, soit 3 à 6 min :
  - caractérisation et test d'arrêt sur l'ancien code ;
  - nouveau code ;
  - 11 mutations ;
  - `gcov`.
  Si un lancement dépasse 30 s, l'implémenteur joue d'abord les mutations §7 (1 à 3) puis 4, 5, 8, et note les autres comme non lancées.
- `make test` : 20 s à 1 min.
- `make e2e` : 4 à 6 min (il lance `qh`/`os`/`hj`, qui plient la cible).
- `make sanitize-net` : ~4 min seul ; il n'est **jamais** lancé en même temps que `e2e`.
- `make value-members` : quelques secondes.

**TU de `UNIT_TESTS_SLOW` touché : aucun.** `qh`, `os` et `hj` reconnaissent l'objet par son descripteur, inchangé. Ils sont lancés une fois par `make e2e`.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` (compile aussi les binaires `DATA_ENGINE_SRCS`, H11) ;
2. `timeout 15m make e2e` : **15 « conforme »**, et les `UNIT_TESTS_SLOW` passent ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 169`**, sans aucune ligne `pico8_bv_et_t`.

**`make class-coverage-net` ne s'applique pas** : aucun constructeur ne change. `bv_et` n'a pas de constructeur (objet statique), et le diff ne touche ni `pico8_g__struct_new`, ni un appel `pico8_class_coverage_record`, ni `KNOWN[]`. S'il en touche un, la cible redevient obligatoire, avec une sortie identique à `tools/class_coverage_net_expected.txt`.

`make member-census` ne s'applique pas (R8). On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements. On ne lance jamais l'appli raylib.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives** sur ce commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.
- `FAILED:` (« champ sans règle ») dans l'un de ces cas :
  - l'aide avorte dans un rejeu ou dans un TU hors des fils de `must_abort`/`must_abort_value` (H2) ;
  - une classe est admise hors des 11 ;
  - un membre ou un indicateur est ajouté ;
  - l'écrivain accepte un autre nom que `fo`.
- `FAILED:` si `fo` n'est pas `pico8_struct_header_t *` (union, `void *`, `pico8_ke_t *`, pointeur d'une cible), ou si l'objet cesse d'être unique et statique.
- `FAILED:` si le prédicat n'est pas exactement « `hq` = `pico8_bv_dispatch`, ou `mz` », ou si `probe.c` appelle un accesseur `pico8_(fv|nr|fr|mx|qh|nl|os|hj|pm)_struct_class`, ou gagne un autre `#include` que `pico8_wormhole.h`.
- `FAILED:` si le `Makefile` change (`DATA_ENGINE_SRCS`, `ENGINE_SRCS`, cibles).
- `FAILED:` si le lecteur ne rend pas 0 avec `*out_value` intact sur `fo` NULL ou un nom inconnu, ou ne rend pas la même adresse.
- `FAILED:` dans l'un de ces cas :
  - l'aide ne pose pas `*out = NULL` **avant** le test de nil ;
  - ce test ne vérifie pas le genre **et** le pointeur ;
  - l'`assert` ne vérifie pas le genre REF, le tag `STRUCT` (avant de lire `cls`) **et** le prédicat ;
  - son message ne nomme pas `bv_et.fo` et l'arrêt.
- `FAILED:` si l'écrivain avorte lui-même sur un nom inconnu, ou si le message de l'`assert(written && …)` de `pico8_entity_set_field` change.
- `FAILED:` si un site par nom change (`pico8_bv`, `dl`, `d_`, `pico8_mz_hq`, `mz:kj`), si le contrat au-dessus de `g_bv_et` change, ou si `g_`, `fm_placeholder`, `jc_too_far`, `mz_jj` ou leurs lecteurs, écrivains ou tables changent.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` dans l'un de ces cas :
  - le diff de `test_pico8_g_.c` fait autre chose qu'ajouter la caractérisation, le test d'arrêt, `must_abort_value`, `BV_ET_MEMBERS_HASH`, les trois `#include` nommés et leurs appels ;
  - `test_pico8_mz.c` ou un TU lent a un diff ;
  - une valeur figée existante change (H5) ;
  - un test de régression (§6) change.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 169, liste encore `pico8_bv_et_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` si le diff touche un constructeur, `pico8_class_coverage_record` ou `KNOWN[]` sans une ligne « Runs » de `make class-coverage-net` identique à la référence.
- `FAILED:` si un crochet du recensement change, si `tools/member_census.txt` est édité à la main, ou si `tools/entity_value_members.sh`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` changent.
- `FAILED:` dans l'un de ces cas :
  - la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant le typage ;
  - elle n'écrit pas une REF de chacune des 11 classes ;
  - elle lit ou écrit un membre directement ;
  - `BV_ET_MEMBERS_HASH` change après le typage.
- `FAILED:` dans l'un de ces cas :
  - le test d'arrêt n'a pas une ligne « Runs » rouge sur l'ancien code et une verte après ;
  - il ne porte pas `zero_fixed`, `zero_false`, `zero_string`, `zero_function` construits par `memset` ;
  - il ne porte pas au moins une struct de classe non admise (`jo`, `jc`, `bv_et`, `mz_jj`).
- `FAILED:` si le commit ne contient pas le rapport `gcov` et les 11 mutations, chacune avec son test ou la justification de son trou, plus le mutant équivalent.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` s'il manque la note « Découvert à 1b.22 » (type à plusieurs cibles, ensemble admis, prédicat), ou le tableau « Runs ».
- `FAILED:` si le budget d'environ 20 minutes est dépassé sans que le journal note ce qui n'a pas été lancé.

## Out of Scope

- `qh.et` (1b.24), `g_.et` (1b.32), `mz.et`, `jo.fo` (1b.23), `jc.fo`/`jc.et` (1b.35) : les membres qui pointent sur l'objet, ou qui ont la même forme.
- `pico8_bv` lui-même : sa signature, sa lecture de `et`, le `p` alloué (TROU-7).
- Remettre `fo` à nil à l'init, ou revenir à un objet par appel.
- Ajouter des fichiers à `DATA_ENGINE_SRCS` ; une aide de déballage partagée entre fichiers ; retoucher les aides de 1b.5 à 1b.21.
- Régénérer `tools/member_census.txt` ; ajouter un TU lent au recensement.
- Remplacer une lecture ou écriture par nom par un accès direct.
- Les fichiers d'événements et `FRAMES_EVENTS` ; tout changement de rendu ; l'audio ; lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - module : `probe.c` seul, un commit, sans découpage ;
  - tests : `test_pico8_g_` étendu (caractérisation et test d'arrêt), sans nouveau TU ni changement du Makefile ;
  - `fo` → `pico8_struct_header_t *`, avec l'ensemble porté par l'`assert` ;
  - ensemble = ligne + `fv`, `nl`, `pm`, admis par la preuve du code et la caractérisation dirigée ;
  - prédicat sur le descripteur plutôt qu'une liste d'accesseurs, à cause de `DATA_ENGINE_SRCS` ;
  - écrivain 1b.13 sans autre écart ;
  - `class-coverage-net` non lancé.
- **Note « Découvert à 1b.22 »** proposée, sur la ligne 1b.22 de `docs/plan-3.md` :

  > patron 1b.1 appliqué ; **lecture nouvelle de R1 : `ref:` à plusieurs cibles → `pico8_struct_header_t *<m>`** (ni union, ni `void *`, ni base `ke`), l'ensemble porté par l'`assert` de l'aide R5 (genre REF, tag `STRUCT`, puis prédicat `static` sur `cls`) ; **ensemble admis = ligne ∪ classes productrices prouvées par le code et exercées par la caractérisation dirigée sur l'ancien code** (ici `fv`, `nl`, `pm`, absents de la ligne, admis ; toute autre cible reste l'arrêt) ; **prédicat** par liste d'accesseurs publics quand tous sont liés partout où le fichier l'est, sinon par une propriété du descripteur que l'ensemble partage exactement (ici `methods[PICO8_METHOD_HQ] == pico8_bv_dispatch`, plus `mz`), parce que `DATA_ENGINE_SRCS` lie `probe.c` sans `help.c` ni `planet_family_trade.c` ; R5 écrivain 1b.13 sans autre écart (`bv_et_take_holder`) ; R7 vacante ; revue des TU lents sans effet ; empreinte réelle 3 (1, 2).

  Le reste va dans le commit.
- Le message de commit porte le périmètre, le compte 170 → 169, le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous, `BV_ET_MEMBERS_HASH` et les réponses à H1 à H16.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
