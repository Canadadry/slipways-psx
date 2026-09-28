---
title: "Plan 3, étape 1b.26 : `hj`, l'ascension. 13 membres typés, `pk` en fixe plus `pk_is_bool` (lecture de 1b.23a1 sans écart), `qy` en `pico8_list_t *` écrit par nom ; `g_hj_members` et `g_hj_defaults` supprimées"
description: "`pico8_hj_t` garde 13 membres `pico8_value_t` : 12 défauts de spec (`rr`, 9 fixes, `pk` {bool, fixe}, `k_`) et `qy`, la liste de particules que `f_` écrit par nom. 1b.26 les type selon le patron R1–R9 de 1b.1 : `pk` suit `qh.qh` (1b.24) et `jo.hf` (1b.23a1), `qy` prend la forme écrivain de 1b.13/1b.15 (`family_take_list`). Les deux tables de `hj` se vident et sont supprimées ; 11 entrées sortent de la table de `pm`, 12 de celle de `mx`. Les défauts sont déballés en `static` (R7 de 1b.14a, 35 `static` et 3 `_is_bool`). `make value-members` passe de 125 à 112. Un commit, 7 fichiers plus le plan."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.26 — `hj` (`planet_family.h:74`), genres mêlés `pk`, `qy` écrit par nom. Empreinte 9 (5, 4). Compte : 112.

Cadre : les puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », le §3 (`make test` vert, aucun `*.sha256` modifié) et le §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Modèles : le PRD de 1b.24 (`docs/prd/done/plan3-phase1b-etape24-qh-laboratoire.md`), `qh` étant le parent de `hj`, et celui de 1b.25 (`docs/prd/triage/plan3-phase1b-etape25-nl-comptoir.md`). Les notes « Découvert à » sont **citées, pas redérivées** :

- **1b.1, puis 1b.3 à 1b.24** : patron R1–R9 appliqué. En particulier :
  - **1b.4b** : les fils forkés vident le recensement, et les boucles sentinelles sont arrêtées par `PICO8_MEMBER_CENSUS_STOP()` ;
  - **1b.9/1b.10** : R7 forme `offsetof`. Une table vidée part avec ses boucles et son seul client ;
  - **1b.14b** : les cas à union nulle se construisent par `memset` puis `kind`. Une mutation qui retire l'unique appel d'une aide `static` se plante sous `if (0)` (`-Werror`) ;
  - **1b.19** : un fixe sans indicateur a une aide sans état nil, et nil y avorte ;
  - **1b.21** : R2, pas d'indicateur sans lecture nil par nom ;
  - **1b.22** : un pointeur typé garde son `assert` à tag ;
  - **1b.23a1** : {bool, fixe} à nil `jamais` (`hf`) → `pico8_fixed_t <m>` + `int <m>_is_bool`, sans indicateur, emballage public unique. Aides de lecture de spec par genre et par fichier (`family_default_*`, `family_trade_default_*`), message aux noms de leurs membres. Valeurs figées = valeurs **observées** ;
  - **1b.23a2** : un membre `l` seul que l'écrivain de 1a accepte garde sa branche. **Boucles sentinelles** : un membre typé sort des `written[]` ;
  - **1b.23b/1b.23c** : aides d'écriture par genre et par fichier (`family_take_vec`, `family_trade_take_vec`, `family_take_string`) ;
  - **1b.23d** : **un tableau `written[]` vidé est supprimé avec sa boucle**, les autres variables restent ;
  - **1b.24** : `qh.qh` {bool, fixe} lu comme 1b.23a1, **sans écart** (`qh_is_bool`, `pico8_qh_qh`). Tables de `qh` et d'`os` supprimées ; `family_default_string` ajoutée ; le crochet `STOP` d'une boucle supprimée est gardé seul. La preuve `sed -n '/^typedef struct {/,/} pico8_X_t;/p'` rouvre sa plage : relue par `awk`.
- **1b.14a — R7 sur une table qui a des filles, patron des défauts `static`** (celui de `jo` et `qh`) :
  - l'entrée sort de **toutes** les tables qui la listent, tables des filles comprises ;
  - chaque défaut est déballé une fois à l'init dans un `static g_<X>_<m>`, au même pointeur pour une REF ou une chaîne ;
  - le `static` est remis à nil dans `defaults_reset` ;
  - la pose typée se fait à l'endroit de la boucle, avant les clés et tout `pq`.

  **`hj` a 2 filles qui listent ses membres : `pm` (11 entrées) et `mx` (12 entrées).**
- **1b.13 et 1b.15 — forme écrivain de R5** :
  - une branche `strcmp` par membre à `e`, dans l'ordre du typedef ; un membre `l` seul n'a pas de branche ;
  - une aide `static` par genre, jamais partagée entre fichiers ni générique ;
  - l'aide pose **d'abord** l'état nil (pointeur NULL), puis sort sur `REF NULL` (genre **et** pointeur testés) ; sinon un `assert` « champ sans règle » qui nomme `struct.membre`, le genre de sa ligne et l'arrêt ;
  - un nom sans branche rend `return 0`, et l'arrêt reste l'`assert(written && …)` de `pico8_entity_set_field` ;
  - **cas d'arrêt `memset`** : le test d'arrêt porte les non-REF hors genre à union entièrement nulle (fixe 0, chaîne NULL, fonction NULL, `false`), construits par `memset` puis `kind` ;
  - un `must_abort` nouveau, rouge sur l'ancien code, fige l'arrêt.
- **1b.11 — R2 des TU lents** : une lecture par nom faite seulement par un TU de `UNIT_TESTS_SLOW` compte comme `l`, et le nil qu'elle lit comme `lu`. **`test_pico8_hj`, `test_pico8_pm` et `test_pico8_mx` sont lents** : `make test` ne les lance pas. Revue plus bas.
- **Lectures de R1 pour les genres mêlés, 1b.5, 1b.7, 1b.16, 1b.24** :
  - 1b.5 : {fixe, bool} → `pico8_fixed_t` + `has_` + `<m>_is_bool` ;
  - 1b.7 : {bool, chaîne} dont le seul booléen écrit est `false` → pointeur + `has_`, NULL présent = `false` ;
  - 1b.16 : {fixe, `ref:pico8_X_t`} dont le seul fixe est le défaut de la spec → pointeur, NULL = ce fixe ;
  - 1b.24 : {bool, fixe} à nil `jamais` → forme 1b.23a1 sans écart (`qh.qh`) ; {`false` seul, REF à plusieurs cibles} → pointeur d'en-tête + `has_`.

  Ce PRD tranche `pk` sur ces lectures (voir « Décisions »).

### État de départ

HEAD `31d3b2d` (PRD de 1b.25). **1b.25 est en cours d'implémentation. Précondition : elle est commitée avant que 1b.26 commence (H1).** Après elle, `make value-members` rend **125**, `pico8_nl_t` n'a plus de `pico8_value_t`, et `family_take_fixed` existe dans `planet_family.c`. 1b.25 ne touche ni `hj`, ni `pm`, ni `mx`, ni `test_pico8_ui` (ses tests vont dans `test_pico8_nl`). Les numéros de ligne cités sont ceux de l'arbre 1b.24 et glisseront : chaque site est aussi désigné par son contenu.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H17).

### Lignes de recensement (`tools/member_census.txt:101-113`)

| membre | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|
| `rr` | `chaine` | observé | stat? | l | `const char *` (pointeur de spec) | non (pointeur ; R2 plus bas) |
| `py` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `ki` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `pn` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `pk` | `bool,fixe` | jamais | - | l | `pico8_fixed_t` + **`int pk_is_bool`** | non (1b.23a1) |
| `fc` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `k_` | `ref:table` | jamais | - | l | `pico8_ob_table_t *` (pointeur de spec) | non (pointeur) |
| `du` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `qo` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `ro` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `qg` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `q_` | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `qy` | `ref:liste` | observé | stat? | **le** | `pico8_list_t *` | non (pointeur) |

Le compte passe à 125 − 13 = **112**. `pk_is_bool` ne compte pas (R9). **Aucune ligne `pico8_hj_t` ne reste.** Les lignes `pico8_pm_t` (`ng`, `fz`) et `pico8_mx_t` (`ht`) restent : 1b.27 et 1b.28.

- **R2 pour `rr`** (lecture de 1b.7, recopiée de 1b.25 pour `nl.rr`). Son seul `stat?` est un homonyme : `oj(self.ke.rr)` (`ie:hb`, `lua.lua:926`) et `oj(b.rr)` (`:1392`, les `hp`). Sur une struct de l'arbre `hj`, `rr` vaut toujours la chaîne de la spec (`"ascension"` pour `hj` et `pm`, qui n'en définit pas, `"void synthesis"` pour `mx`) : jamais écrit (l'écrivain le refuse, `must_abort(write_rr)` dans les trois TU lents), et la vue `ie_ke` lit la **spec** (`kind->field`), pas l'instance. Donc pointeur de spec, sans indicateur.
- **R2 pour `qy`**. C'est un pointeur : NULL dit nil, sans indicateur par construction. Son `stat?` vient des homonymes `ga.qy` (`ga:kj`, `if pt then`) et `g_.qy`. Sur `hj`, `qy` est nil de l'init jusqu'à `f_` (appelé par le constructeur, avant `jo:pq`), puis toujours une liste. Aucune lecture par nom ne le voit nil (revue plus bas).
- **`qy` seul a `e`** : son seul écrivain est `pico8_f_` (`particle.c`, `pico8_entity_set_field(self, "qy", pico8_value_ref(qy))`), par nom, et les boucles `written[]` des TU lents.

### Revue des TU lents (R2 de 1b.11)

Relus pour les 13 noms : `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx`, puis `test_pico8_qh`, `os`, `nr`, `jo` et `ie`. Constats :

- **`hj`** : `hj_defaults[]` (les 12 défauts) plié par `has` ; `scalars[]` plié par `fold_field` (présence, genre, valeur) ; `hj_tables[]` ∋ `k_` : `get_ref(e, "k_") == class_ref(pico8_hj_default, "k_")`, d'où le **pointeur de spec** ; `list_size(e, "qy")` (−1 si absent) vaut 0 ou ≥ 2, **jamais −1** ; `get_ref(a, "qy") != get_ref(b, "qy")` (une liste par instance) ; `bool_state(pm, "pk") == 1` et `get(pm, "rr")`/`get(mx, "rr")` comparés par `strcmp` ; `must_abort(write_rr/py/k_)` (l'écrivain refuse ces noms), `must_abort(write_qy, qh neuf)`.
- **`pm`** et **`mx`** : `hj_members[]` (les 13 noms) ; `scalars[]` ; `pm_tables[]`/`mx_tables[]` ∋ `k_` (pointeur de la spec de la fille, `!= class_ref(pico8_hj_default, "k_")`) ; `inherited[]` comparé par `pico8_value_equal` entre le lecteur de la fille et celui de `hj` ; `list_size(…, "qy")` jamais −1.
- **Boucles sentinelles `written[]`** : `hj` `{qy}`, `pm` `{qy}`, `mx` `{qy, ht}`. Chaque boucle écrit `pico8_value_fixed(i + 1)` dans `qy` et relit `FIXED`. Une fois typé, `qy` avorte sur un fixe. Aucune de ces boucles n'a de crochet `PICO8_MEMBER_CENSUS_STOP()` (les TU lents ne sont pas dans `CENSUS_UNIT_TESTS`).
- `test_pico8_qh`, `os`, `nr`, `jo` : aucun des 13 noms. `test_pico8_ie` : `rr` sur la vue `ie_ke` (spec), homonyme de 1b.11.

**R2 ne donne aucun indicateur.** Trois TU lents sont modifiés, seulement dans leur `written[]` (voir « Boucles sentinelles »).

### Ce que fait le cart (`pico8/lua.lua`)

- **Défauts** (`hj=qh:jr[[…]]` `:998-1010`, `pm=hj:jr` `:1017-1030`, `mx=hj:jr` `:1036-1049`) :

| membre | `hj` | `pm` | `mx` |
|---|---|---|---|
| `rr` | `"ascension"` | (hérité de `hj`) | `"void synthesis"` |
| `py`, `ki` | 0.12, 9 | 0.06, 9 | 0.12, 9 |
| `pn`, `pk` | 5, **1** | 8, **`f`** | 5, **1** |
| `fc` | 0.5 | 1 | 0.3 |
| `k_` | `o(1,13,12,…)` | `o(1,2,4,9,10,7)` | `o(1,1,5,5,…)` |
| `du` | 7.5 | 2 | 5 |
| `qo`, `ro`, `qg`, `q_` | 0.98, −0.07, 0.07, 0.98 | 1.02, −0.0525, 0.0525, 1.02 | 0.98, −0.07, 0.07, 0.98 |

- **`pk`** : un seul booléen tenu, le `f` de `pm` ; un seul fixe, `1` (`hj`, `mx`). Jamais écrit, jamais nil. Lu par `g_:mm` (`:729`, `self.pk and v:pa() or v`), **en vérité seulement**, par `hj.mm=g_.mm`.
- **`qy`** : `f_` (`:685`, `self.qy={}`), le `pq` de `hj` (hérité par `pm` après son propre `pq`, et par `mx`). `ez` (`:687`) y ajoute `self:mm()` ; `ei` (`:692`) le parcourt (`pairs(self.qy)`) et en retire (`del`). `hj:kj` n'appelle `ei` que si `self.mi and self.mi >= 2`.
- **Les 11 autres** sont lus par `g_:mm` (`pn`, `du`, `py`, `ki`) et `ei` (`qo`, `qg`, `ro`, `q_`, `k_`, `fc`) ; `rr` par la vue de spec de `ie`.

### Le code (arbre 1b.25 supposé commité)

- **Typedef** (`planet_family.h`) : `{ pico8_qh_t qh; rr; py; ki; pn; pk; fc; k_; du; qo; ro; qg; q_; qy; }`, tous en `pico8_value_t`. `pm` l'enchâsse (`pico8_hj_t hj; ng; fz;`), `mx` aussi (`pico8_hj_t hj; ht;`, `planet_family_trade.h`).
- **Tables d'offsets** : **48 entrées** sur ces 13 noms.

| fichier | table | entrées sur les 13 noms | reste après |
|---|---|---|---|
| `planet_family.c` | `g_hj_members` | les 13 | **supprimée** |
| `planet_family.c` | `g_hj_defaults` | 12 (tous sauf `qy`) | **supprimée** |
| `planet_family.c` | `g_pm_defaults` | 11 (`py`, `ki`, `pn`, `pk`, `du`, `fc`, `k_`, `qo`, `ro`, `qg`, `q_`) | `fz`, `ng` |
| `planet_family_trade.c` | `g_mx_defaults` | 12 (`rr` et les 11 de `pm`) | `ht` |

  `g_pm_members` (`ng`, `fz`) ne liste aucun des 13 noms : inchangée.
- **Clients des tables de `hj`** : le lecteur `pico8_hj_field` (boucle sur `g_hj_members`, repli sur celui de `qh`) ; dans `pico8_hj_struct_init`, la boucle nil sur `g_hj_members` et la boucle de recopie de `g_hj_defaults` ; dans `pico8_planet_family_init`, la boucle de lecture de spec et son `assert` « hj: every default … » ; la boucle de `pico8_hj_defaults_reset`.
- **Clients des entrées de `pm` et `mx`** : les boucles de recopie de `pico8_pm_struct_new` et `pico8_mx_struct_new`, de lecture de spec (init de chaque fichier) et de remise à nil. Elles restent, sur `fz`/`ng` et `ht`.
- **Lecteurs des filles** : `pico8_pm_field` (boucle sur `g_pm_members`, puis `pico8_hj_field`, même fichier) ; `pico8_mx_field` (`ht`, puis `pico8_hj_struct_class()->field`).
- **Écrivain** `pico8_hj_set_field` : `qy` (`->qy = value`, tout genre), puis les 20 noms de `qh`. Il refuse les 12 autres. `pm` a le même écrivain. Celui de `mx` ajoute `ht` avant de déléguer. **21 noms pour `hj`/`pm`, 22 pour `mx`, inchangés.**
- **Écritures directes (R6)** : aucune. `qy` n'est écrit que par nom (`f_`).
- **Sites par nom, inchangés** (Coexistence) :

| fichier | noms lus ou écrits par nom |
|---|---|
| `particle.c` | `f_` (`qy` écrit), `ez` (`qy`), `ei` (`qo`, `qg`, `ro`, `q_`, `k_`, `qy`, `fc`) |
| `probe.c` | `g__mm` (`pn`, `du`, `py`, `ki` en fixe ; `pk` en vérité, `pico8_probe_field_truthy`) |

- **Homonymes hors étape** : `g_.*` (1b.32, `probe.c`, et `fv` qui l'enchâsse, `help.c`), `ga.*` (1b.29, `fc`, `qo`, `ro`, `qg`, `q_`, `k_`, `py`, `qy`), `jc.k_` (1b.35), `nl.rr` (1b.25), `nr.rr` (1b.30), `hp.rr` (1b.9, fait), `ie_ke.rr` (vue).

## Solution

1. **Caractérisation d'abord, figée sur l'ancien code** (arbre 1b.25 commité), dans un TU rapide. Elle lit par nom les 13 membres sur `hj`, `pm` et `mx` neufs, placés et vivants (`mm`, `ez`, `kj`→`ei`), et fait l'aller-retour par nom de `qy`. Elle doit être verte avant tout typage.
2. **Test d'arrêt, rouge sur l'ancien code** : l'écriture hors genre de `qy` (non-listes, unions nulles `memset`) sur `hj`, `pm` et `mx`.
3. **Commit 1b.26** :
   - les 13 membres sont typés, avec `pk_is_bool` et l'emballage `pico8_hj_pk` ;
   - `g_hj_members` et `g_hj_defaults` sont supprimées avec leurs boucles ; 11 entrées sortent de `g_pm_defaults`, 12 de `g_mx_defaults` ;
   - les défauts sont déballés en `static` dans les deux fichiers ;
   - l'aide `family_take_list` est ajoutée, ainsi que `family_trade_default_string` et `family_trade_default_hf` ;
   - `qy` sort de 3 tableaux `written[]` ; ceux de `hj` et `pm`, vidés, sont supprimés avec leur boucle.

   `make value-members` rend **112**, dont **0 ligne `pico8_hj_t`**.
4. Aucun pixel ne bouge. La note « Découvert à 1b.26 » va sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `planet_family.h`, je veux `pico8_hj_t` sans aucun `pico8_value_t`, pour lire une ascension sur ses types.
2. En tant que lecteur, je veux `rr` en `const char *`, `k_` en `pico8_ob_table_t *`, `qy` en `pico8_list_t *`, `pk` en `pico8_fixed_t` suivi de `pk_is_bool`, et les 9 autres en `pico8_fixed_t`.
3. En tant que lecteur par nom (`particle.c`, `probe.c`, les TU), je veux que chacun des 13 noms rende exactement ce que rendait l'ancien code sur `hj`, `pm` et `mx` : présence, genre, valeur, identité du pointeur.
4. En tant que `hj`, `pm` ou `mx` neuf, je veux les défauts de ma spec ou de celle de `hj`, avec `rr` et `k_` au **même pointeur** que la spec qui les définit, et `qy` une liste vide à moi.
5. En tant que `pm`, je veux `pk` lu `BOOL` 0 présent ; en tant que `hj` ou `mx`, `FIXED` 1.
6. En tant que `g_:mm` appelé sur une ascension, je veux `self.pk` vrai pour `hj`/`mx` et faux pour `pm`, donc la même particule qu'avant.
7. En tant que `f_`, je veux écrire `qy` = une liste neuve par nom et la relire au même pointeur.
8. En tant que `ez` et `ei`, je veux lire `qy` par nom et l'étendre ou le réduire en place, comme avant.
9. En tant qu'écrivain de `qy`, je veux que nil l'éteigne (lecture suivante absente, témoin intact).
10. En tant que gardien de `qy`, je veux que tout non-nil qui n'est pas une liste avorte, fixe 0, chaîne NULL, fonction NULL et `false` à union nulle compris, avec un message qui nomme `hj.qy` et `ref:liste`.
11. En tant que mainteneur, je veux que les écrivains acceptent toujours exactement 21 noms (`hj`, `pm`) et 22 (`mx`), et que les 12 défauts restent refusés.
12. En tant que mainteneur, je veux `g_hj_members`/`g_hj_defaults` supprimées, et plus aucun `offsetof(pico8_hj_t` ni `offsetof(pico8_(pm|mx)_t, hj.` dans `src/`.
13. En tant que TU lent à boucle sentinelle, je veux que ma boucle ne réécrive plus `qy` en fixe.
14. En tant qu'auteur de 1b.27 (`pm`) et 1b.28 (`mx`), je veux leurs tables débarrassées des membres de `hj`, et leurs `static` de défaut à leur nom.
15. En tant qu'auteur de 1b.29 (`ga`) et 1b.32 (`g_`), je veux une aide `family_take_list` que `ga.qy` pourra étendre dans le même fichier, et `particle.c`/`probe.c` inchangés.
16. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
17. En tant que gardien de la mémoire et de la couverture, je veux `make sanitize-net` à 15 propres et `make class-coverage-net` identique à sa référence.
18. En tant que `plan-pilot`, je veux `make value-members` à 112 et `pico8_hj_t` absent de sa sortie.
19. En tant que vérificateur, je veux la caractérisation verte sur l'ancien code, le test d'arrêt rouge puis vert, les mutations et leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules et les tests. Ces choix sont tranchés comme le ferait un agent `guideline` : hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables. Ils sont marqués (G).

### `pk` : `pico8_fixed_t pk` + `int pk_is_bool`, lecture de 1b.23a1 sans écart (G)

**États à représenter** : `FIXED` 1 (`hj`, `mx`) et `BOOL` 0 présent (`pm`). Nil `jamais`, jamais écrit. Lu par nom en vérité (`g__mm`), et en genre et valeur par les TU (`bool_state(pm, "pk") == 1`, `fold_field`, `STRUCT_HASH` de `jo_spec_defaults`, `inherited[]` avec `pico8_value_equal`).

| option | verdict |
|---|---|
| **1b.5** : `pk` + `has_pk` + `pk_is_bool` | **rejetée** : nil `jamais`, aucune lecture nil par nom ; `has_pk` vaudrait toujours 1 (R2, 1b.21). |
| **1b.7** : pointeur + `has_`, NULL présent = `false` | **sans objet** : aucun genre pointeur dans la ligne. |
| **1b.16** : pointeur, NULL = le fixe de la spec | **sans objet** : aucune REF. |
| `int pk` (vérité seule) | **rejetée** : la lecture par nom doit rendre le genre de 1a (Coexistence) ; `FIXED` 1 et `BOOL` 1 se confondraient, et les plis des TU changeraient. |
| **1b.23a1, relue par 1b.24 (`qh.qh`)** : `pico8_fixed_t pk; int pk_is_bool;` | **retenue, sans écart** : même ligne `bool,fixe`, même nil `jamais`, même absence d'écriture. |

`pk_is_bool` est déclaré juste après `pk` (1 : `pk` tient un booléen 0/1 ; 0 : un fixe). **Emballage public unique** `pico8_value_t pico8_hj_pk(const pico8_hj_t *self)`, forme `pico8_qh_qh` : `REF NULL` si `self` est NULL, `pico8_value_bool(pk != 0)` si `pk_is_bool`, `pico8_value_fixed(pk)` sinon. Appelé par le lecteur. Aucune lecture directe de `pk` n'existe dans `src/`.

### `qy` : `pico8_list_t *`, écrit par nom, forme écrivain de 1b.13/1b.15 (G)

- **Type** : `pico8_list_t *qy`, NULL = nil. Possédé par l'instance, jamais libéré (TROU-7, comme la liste que `f_` alloue aujourd'hui) ; le typedef le dit dans son commentaire.
- **Aide R5** `static void family_take_list(pico8_value_t value, pico8_list_t **out)`, dans `planet_family.c`, forme `family_take_vec`/`jc_too_far_take_jc` :
  1. `*out = NULL;`
  2. `if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) return;` : genre **et** pointeur ;
  3. `assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_LIST && "hj.qy: a write neither nil nor a list is off its census row (ref:liste) -- a field under no rule of plan-3 section 5: stop, do not widen the member");`
  4. `*out = value.as.ref;`
- **Un seul appelant** : la branche `qy` de `pico8_hj_set_field` (`family_take_list(value, &hj->qy); return 1;`), puis la délégation à `qh`, ordre gardé. `pm` et `mx` y passent par délégation.
- **Distincte** de `family_default_list` (lecture de spec) et de toute aide de `planet_family_trade.c` (1b.13 point 2). `ga.qy` (1b.29), dans le même fichier, pourra l'étendre en élargissant son message ; ce n'est pas l'objet de 1b.26.
- **Écarté** : un indicateur `has_qy` (pointeur, R2) ; une pose de liste vide en R3 (le cart pose nil, et `f_` écrit la liste).

### R1–R9 appliquées

| règle | sur `hj` |
|---|---|
| R1 | Ordre et noms gardés : `const char *rr; pico8_fixed_t py, ki, pn, pk; int pk_is_bool; pico8_fixed_t fc; pico8_ob_table_t *k_; pico8_fixed_t du, qo, ro, qg, q_; pico8_list_t *qy;` (un membre par ligne, comme `qh`). `rr`, `k_` empruntés à la spec, jamais libérés. |
| R2 | Aucun indicateur (plus haut). |
| R3 | `pico8_hj_struct_init`, après `pico8_qh_struct_init` et à la place des deux boucles : `rr`, `py`, `ki`, `pn`, `pk`/`pk_is_bool`, `fc`, `k_`, `du`, `qo`, `ro`, `qg`, `q_` depuis leurs `static`, puis `qy = NULL` ; les poses de `qh` existantes suivent, inchangées. Le `size_t i` devenu inutile est retiré (`-Werror`). **`pm`** (`pico8_pm_struct_new`) repose ses 11 défauts juste après sa boucle de recopie (qui garde `fz`, `ng`), avant ses poses de `qh`/`jo`, les clés et tout `pq`. **`mx`** (`pico8_mx_struct_new`) repose ses 12 défauts juste après sa boucle (qui garde `ht`), avant ses poses de `qh`/`jo` et les clés. |
| R4 | `pico8_hj_field` : 13 branches `strcmp` dans l'ordre du typedef, puis `pico8_qh_struct_class()->field`. Fixes : toujours `FIXED`. `pk` : `pico8_hj_pk`. `rr` : `return 0` si NULL, sinon `STRING` au même pointeur. `k_`, `qy` : `return 0` si NULL, sinon REF au même pointeur. `*out_value` intact sur `return 0`. |
| R5 | Branche `qy` par `family_take_list` (plus haut). Aucune boucle de clés ne prend ces noms (clés mesurées : `id`, et `p`/`ra` pour `pm`). |
| R6 | Aucune écriture directe. |
| R7 | 48 entrées retirées. `g_hj_members` et `g_hj_defaults` supprimées avec toutes leurs boucles (forme 1b.9/1b.24). `g_pm_defaults` garde `fz`, `ng` ; `g_mx_defaults` garde `ht`. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_hj_t)` gardé sur un typedef sans `pico8_value_t` (précédent `jo`, `qh`). `tools/member_census.txt` n'est pas édité. |
| R9 | 125 → 112. |

### Défauts déballés (R7 de 1b.14a) (G)

- **`planet_family.c`** : 23 `static` et 2 `_is_bool`, déballés dans `pico8_planet_family_init` juste après la lecture de spec de chaque genre (à la place de la boucle de `hj` ; après celle de `pm`, qui reste), et remis à 0 ou NULL dans le `defaults_reset` du genre.

| genre | `static` |
|---|---|
| `hj` | `g_hj_rr`, `g_hj_py`, `g_hj_ki`, `g_hj_pn`, `g_hj_pk`/`g_hj_pk_is_bool`, `g_hj_fc`, `g_hj_k_`, `g_hj_du`, `g_hj_qo`, `g_hj_ro`, `g_hj_qg`, `g_hj_q_` |
| `pm` | `g_pm_py`, `g_pm_ki`, `g_pm_pn`, `g_pm_pk`/`g_pm_pk_is_bool`, `g_pm_du`, `g_pm_fc`, `g_pm_k_`, `g_pm_qo`, `g_pm_ro`, `g_pm_qg`, `g_pm_q_` |

  `pm` n'a pas de `rr` : il garde celui que pose `pico8_hj_struct_init` (`g_hj_rr`), et `pico8_pm_default("rr")` rend le même pointeur par le repli sur `hj` (H3). Aides, messages étendus (lecture de forme de 1b.23a1) :
  - `family_default_string` : `+ hj.rr` ;
  - `family_default_fixed` : `+ hj/pm.py/ki/pn/fc/du/qo/ro/qg/q_` ;
  - `family_default_hf` : `+ hj/pm.pk` (gardée sous son nom, 1b.24) ;
  - `family_default_table` : `+ hj/pm.k_`.
- **`planet_family_trade.c`** : 12 `static` et 1 `_is_bool`, déballés et remis à nil de même : `g_mx_rr`, `g_mx_py`, `g_mx_ki`, `g_mx_pn`, `g_mx_pk`/`g_mx_pk_is_bool`, `g_mx_fc`, `g_mx_k_`, `g_mx_du`, `g_mx_qo`, `g_mx_ro`, `g_mx_qg`, `g_mx_q_`. Aides :
  - `family_trade_default_table` : `+ mx.k_` ;
  - `family_trade_default_fixed` : `+ mx.py/ki/pn/fc/du/qo/ro/qg/q_` ;
  - **`family_trade_default_string`**, nouvelle : `"mx.rr: a default of the struct is a key of mx's spec and a string (chaine)"` (`nr.rr`, 1b.30, l'étendra) ;
  - **`family_trade_default_hf`**, nouvelle, forme `family_default_hf` : `"mx.pk: a default of the struct is a key of mx's spec and a fixed or a boolean (bool,fixe)"`.
- **Pourquoi `family_trade_default_hf` plutôt qu'un `pk_is_bool = 0` littéral pour `mx` (G)** : le genre vient de la spec, comme pour `hj` et `pm` ; un littéral serait une seconde source de vérité. Coût : 7 lignes.
- **Pointeur de spec** pour `rr` et `k_` (R3 « même REF »). L'identité est figée par `hj_tables[]` (`test_pico8_hj`), `pm_tables[]`, `mx_tables[]`.
- **Écarté (G)** : des littéraux en R3 (1b.14a s'applique, les filles listent ces membres, et les TU comparent aux specs) ; une aide partagée entre les deux fichiers (1b.13, point 2).

### Boucles sentinelles (1b.23a2, 1b.23d)

- `qy` sort des tableaux `written[]` de `test_pico8_hj` (`test_hj_is_a_struct`), `test_pico8_pm` (`test_pm_is_a_struct`) et `test_pico8_mx` (`test_mx_is_a_struct`).
- **Vidés, supprimés avec leur boucle** : `hj` et `pm`. Aucun crochet `STOP` n'y précède la boucle (TU lents, hors recensement) : rien à garder. Dans `pm`, le `pico8_value_t v` local à la boucle part avec elle ; `i` reste (boucles `methods`, `inherited`). Dans `hj`, `i` reste.
- **Gardé** : `mx` `{ht}` (1b.28). `ht` y reçoit désormais `pico8_value_fixed(1)` au lieu de 2 : la boucle ne relit que ce qu'elle écrit (H15).
- `must_abort(write_qy, pico8_qh_new(NULL))` de `test_pico8_hj` reste (l'écrivain de `qh` refuse `qy`). Rien d'autre ne change dans ces TU. L'intention des boucles passe dans la caractérisation (liste, nil) et dans le test d'arrêt (hors genre).

### Revue des tests de régression (§6), mécanique

**Liste** : l'union des tests nommés au §6 et dans les lignes « Tests : » des étapes 1a, soit les 17 fichiers `tests/test_pico8_<nom>.c` (même liste que 1b.24 et 1b.25) : `built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`. Les 17 existent.

**Greps sur ces 17 fichiers** :

- accès direct : `grep -nE '(->|\.)(rr|py|ki|pn|pk|pk_is_bool|fc|k_|du|qo|ro|qg|q_|qy)\b|offsetof\([^)]*\b(rr|py|ki|pn|pk|fc|k_|du|qo|ro|qg|q_|qy)\)'` → **aucun accès** (seuls deux commentaires de `probe_mm_rnd_order`, `self.pn`) ;
- par nom : `grep -nE '"(rr|py|ki|pn|pk|fc|k_|du|qo|ro|qg|q_|qy)"'` → 2 fichiers, homonymes ;
- chemin : `grep -nE 'pico8_(hj|pm|mx)_|"(hj|pm|mx)"|ascension|protostar|synthesi|pico8_(f_|ez|ei)\b|"mm"|g__mm'` → 1 fichier, homonyme.

**Aucun accès direct, aucun accès par nom sur une struct de l'arbre `hj`.** Toutes les correspondances, chacune avec son plan de preuve rouge :

| # | test | ligne | nature | touché par le diff ? | plan de preuve rouge |
|---|---|---|---|---|---|
| R-a | `geoharvest_burst` | `:95` `"qy"` | `ga.qy` (1b.29), rempli par `ga:pq` ; le rejeu ne construit aucune ascension connue | non (`particle.c` et le code de `ga` inchangés) | **Dans le budget seulement.** Bug remis localement : `ga:pq` sans son `ez(self,140)`. Attendu **rouge** (`qy` du `ga` vide, la gerbe ne vit pas jusqu'à la frame 289). On restaure : vert. Sinon le journal note « non lancé, homonyme hors chemin ». |
| R-b | `probe_mm_rnd_order` | `:1-2` (commentaires `self.pn`), `:32-35` (`pn`, `du`, `py` en clés de construction d'un `g_`), `:47` `pico8_ez`, `:49` `"qy"` | `g_.*` (1b.32). `g__mm` et `pico8_ez` sont aussi le `mm` et l'`ez` de `hj`, mais le test ne construit qu'une sonde et **ces deux fonctions ne sont pas modifiées** | non | **Dans le budget seulement.** Bug d'origine **3175ebd** remis localement : dans `g__mm`, les deux tirages refondus en arguments d'une même expression (`rnd(0.5)` tiré d'abord par gcc). Attendu **rouge**. On restaure : vert. Sinon noté comme R-a. |

Les 15 autres tests n'ont aucune correspondance et gardent un diff vide. Qu'un de leurs rejeux construise une ascension n'est pas connu sans lancer (H6) ; s'il le fait, `make test` vert suffit, le diff du test étant vide. **Aucune preuve rouge n'est obligatoire.**

### Preuves (sortie attendue sur l'arbre final)

**Sortie de `hj`** :

- `awk '/^typedef struct \{/{b=""} {b=b $0 "\n"} /\} pico8_hj_t;/{printf "%s", b}' src/pico8_planet_family.h | grep -c 'pico8_value_t'` : **0** (forme `awk`, leçon de 1b.24) ;
- `make value-members | grep -c 'pico8_hj_t'` : **0** ; `grep -c 'pico8_pm_t'` : 2 ; `grep -c 'pico8_mx_t'` : 1 ;
- `grep -rnw 'g_hj_members\|g_hj_defaults' src/` : **vide** ;
- `grep -rn 'offsetof(pico8_hj_t' src/` : **vide** ;
- `grep -rnE 'offsetof\(pico8_(pm|mx)_t, hj\.' src/` : **vide** (23 occurrences avant).

**Aides et écrivain** :

- `grep -nE '(\.|->)(rr|py|ki|pn|pk|fc|k_|du|qo|ro|qg|q_|qy)\.(kind|as)' src/pico8_planet_family*.c` : relu, seuls les homonymes de `ga` (1b.29) restent ;
- `grep -n 'family_take_list' src/pico8_planet_family.c` : la définition et un seul appel ;
- `grep -c 'off its census row' src/pico8_planet_family.c` : +1 par rapport au départ ; `src/pico8_planet_family_trade.c` : inchangé ;
- `grep -n 'pk_is_bool\|pico8_hj_pk' src/pico8_planet_family.h` : `pk_is_bool` juste après `pk`, plus l'emballage.

**Périmètre du diff** :

- `git diff --stat HEAD -- src/ ':!src/pico8_planet_family.h' ':!src/pico8_planet_family.c' ':!src/pico8_planet_family_trade.c'` : **vide** (`particle.c`, `probe.c`, `planet_family_trade.h` inchangés) ;
- `git diff HEAD -- tools/ Makefile` : vide ;
- 3 TU lents : le diff ne touche que `written[]`, et pour `hj`/`pm` le tableau et sa boucle ;
- `test_pico8_ui.c` : ajouts seulement.

### Empreinte remesurée ; découpage

La ligne dit 9 (5, 4). **Empreinte réelle** (fichiers qui lisent ou écrivent l'un des 13 membres d'une struct de l'arbre `hj`) : **9 (5, 4)** (H16).

- **src (5)** : `planet_family.h`, `planet_family.c`, `planet_family_trade.c` en direct ; `particle.c` et `probe.c` par nom.
- **tests (4)** : `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx` (lents) et `test_pico8_jo_spec_defaults` (rapide : `STRUCT_HASH` plie les 13 noms sur des `hj`/`pm`/`mx` neufs).

**Fichiers modifiés : 7**, plus `docs/plan-3.md` :

- **src (3)** : `pico8_planet_family.h`, `pico8_planet_family.c`, `pico8_planet_family_trade.c` ;
- **tests (4)** : `test_pico8_ui.c` (caractérisation, test d'arrêt) ; `written[]` de `test_pico8_hj`, `test_pico8_pm`, `test_pico8_mx` (**3 TU lents**).

**Découpage (G) : aucun, un seul commit.** L'étape tient en une passe : 7 fichiers, un seul genre nouveau (`qy`), une forme mêlée déjà tranchée (`pk`). Découpes écartées :

- **(a) `qy` seul, puis les 12 défauts** : les deux moitiés touchent le même lecteur, la même `struct_init` et `test_pico8_ui` ; `g_hj_members` ne tomberait qu'au second commit ; deux `make e2e` et deux `class-coverage-net`, hors budget.
- **(b) `hj` puis `pm`/`mx`** : impossible, un membre change de type pour toutes les tables à la fois (R7).

### Hypothèses (rien n'a été lancé)

- **H1** : 1b.25 est commitée ; `make value-members` rend 125 au départ, puis 112 ; seules les 13 lignes `pico8_hj_t` disparaissent. `family_default_string` existe (1b.24) ; 1b.25 n'a touché ni `hj`/`pm`/`mx` ni `test_pico8_ui`.
- **H2** : dans les 15 rejeux et dans tous les TU hors fils de `must_abort`, `qy` ne reçoit que des listes (`f_`) ou nil, une fois les `written[]` réduits.
- **H3** : sur un `hj`/`pm`/`mx` neuf, les valeurs lues sont celles du tableau du cart ; `rr` de `pm` est le pointeur de la spec de `hj` ; `pk` est `FIXED` 1 sur `hj`/`mx`, `BOOL` 0 présent sur `pm` ; `qy` est une liste vide propre à l'instance. Les valeurs figées sont celles **observées**.
- **H4** : sur l'ancien code, chaque cas du test d'arrêt se termine sans `SIGABRT` (l'écrivain recopie toute valeur). Un cas qui avorte déjà est retiré du « rouge d'abord » et noté.
- **H5** : aucune valeur figée ne bouge : `BUILT_HASH`, `LIFE_HASH`, `HEIRS_HASH`, `RECORDED_HASH` de `test_pico8_hj` ; ceux de `test_pico8_pm` (dont `SESSION_HASH`) et `test_pico8_mx` ; `STRUCT_HASH`/`SPEC_HASH` de `jo_spec_defaults` ; les hash de `test_pico8_ga`, `test_pico8_g_`, `test_pico8_ui` (dont `QH_MEMBERS_HASH`) ; les 15 empreintes du filet.
- **H6** : on ne sait pas lequel des 17 rejeux de régression construit une ascension ; sans effet sur le diff.
- **H7** : `make sanitize-net` reste propre : aucune allocation nouvelle (la liste de `qy` est toujours celle de `f_`).
- **H8** : `test_pico8_ui` reste sous 30 s avec deux fonctions de plus (sinon §7 : `UNIT_TESTS_SLOW`, changement de `Makefile` interdit ici → arrêt et note).
- **H9** : le build `-DPICO8_MEMBER_CENSUS` compile avec un `pico8_hj_t` sans `pico8_value_t` (non lancé).
- **H10** : rien ne dépend de `sizeof(pico8_hj_t)` ni de la disposition de `pm`/`mx`.
- **H11** : `pico8_hj_pk`, `family_take_list`, `family_trade_default_string` et `family_trade_default_hf`, chacune appelée au moins une fois, ne déclenchent aucun avertissement (`-Werror`).
- **H12** : une ascension placée comme dans `test_life` de `test_pico8_hj` (`hq`, `ey` avec 200 crédits, `mi` = 2 par nom) fait tirer `ei` au `kj` suivant (`fa >= 10` ou `gd`). Sinon la caractérisation garde `mm` et `ez` pour les lecteurs de `ei`, et l'implémenteur le note au commit ; ce n'est pas un arrêt.
- **H13** : dans un fils frais, l'état `rnd` rend les plis de `mm`/`ez`/`ei` reproductibles d'un lancement à l'autre.
- **H14** : `pico8_pm_new` avec `p`, `ra`, `id` (forme `build_protostar` de `test_pico8_pm`) se construit dans un `start_game(DRAG)` ou `EMPTY_CLEARED` de `test_pico8_ui`.
- **H15** : le passage de `ht` de 2 à 1 dans la boucle `written[]` de `mx` ne touche aucune valeur figée.
- **H16** : l'empreinte réelle est 9 (5, 4). Le compte exact est fait au commit.
- **H17** : les 17 tests de régression passent sans modification.

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Les nouveaux tests n'accèdent à aucun des 13 membres directement : ils compilent sur les deux formes.
- **(G) Où : `test_pico8_ui`.** Il est rapide (hors `UNIT_TESTS_SLOW`), dans `CENSUS_UNIT_TESTS`, inclut déjà `planet_family.h` et `planet_family_trade.h`, et a `start_game`, `play_frame`, `in_fresh_child`, `must_abort` avec sa forme à valeur (`g_off_row_value`/`write_off_row`), `qh_tree_new`/`qh_tree_reader` et le chemin `EMPTY_CLEARED`. Précédents : 1b.23a–d, 1b.24. **Pas `test_pico8_hj`, `pm` ni `mx`, qui sont lents** (1b.11) ; et aucun rejeu rapide ne construit d'ascension (les rejeux connus pour en construire, `events_recorded_*` et `session_events`, sont ceux des TU lents).
- **Caractérisation, figée sur l'ancien code** : `test_hj_members_by_name`, lancée par `in_fresh_child` après les tests de `qh`, fonction distincte, hash propre **`HJ_MEMBERS_HASH`**. Elle lit par le **lecteur brut du descripteur** (`cls->field`, témoin `0x1234`) **et** par `pico8_entity_get_field`, qui doivent concorder (retour, genre, valeur, pointeur). Les adresses sont jugées par `assert`, jamais pliées.
  1. **Neufs.** `pico8_hj_new(NULL)`, `pico8_pm_new(NULL)`, `pico8_mx_new(NULL)`, et un `pm` construit avec `p`, `ra`, `id` : on plie les 13 noms. `assert` : `rr` au même pointeur que `class_value(pico8_X_default, "rr").as.string` (pour `pm`, égal à celui de `pico8_hj_default`) ; `k_` au même pointeur que `class_ref(pico8_X_default, "k_")`, et celui de `pm`/`mx` différent de celui de `hj` ; `qy` présent, liste vide, différente d'une instance à l'autre ; `pk` `BOOL` 0 sur `pm`, `FIXED` 1 sur `hj`/`mx`.
  2. **Vivants.** Pour chacun des trois genres, dans un `start_game(EMPTY_CLEARED)` : `call(e, "mm")` → pli de la particule (`p`, `v`, `l` : `pn`, `du`, `py`, `ki`, et `pk` en vérité, `pm` sans `pa`) ; `pico8_ez(e, 2.5)` → taille de `qy` ; puis une ascension placée (H12) : `mi` = 2 par nom, 4 appels de `kj` → pli des enregistrements de `qy` (`ei` lit `qo`, `qg`, `ro`, `q_`, `k_`, `fc`) et de la taille. `qy` garde le même pointeur tout du long.
  3. **Écrivain par nom de `qy`**, sur `hj`, `pm` et `mx` (chaque chaîne de délégation) :

     | écriture | lecture attendue |
     |---|---|
     | une liste neuve (`pico8_list_init`) ; `pico8_hv()` | même pointeur, `REF` |
     | `REF NULL` | absent aux deux lecteurs, témoin intact ; `list_size` −1 |
     | la liste d'origine, de nouveau | même pointeur que celle de `f_` |

     `cls->set_field` rend 1 pour `qy` ; 0 pour les 12 autres noms, **par l'écrivain brut**, sans rien toucher (repli identique) : les ensembles de 21 et 22 noms sont inchangés.
  4. **Intrus** sur le lecteur de `hj` : `""`, `"q"`, `"qy "`, `"QY"`, `"pk_is_bool"`, `"k"`, `"k__"`, `"rr_"` → 0, témoin intact.
  5. Tout est plié dans **`HJ_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ, puis inchangé.
- **Test d'arrêt, rouge d'abord** : `test_hj_writer_stops_off_row`, par `in_fresh_child` après la caractérisation, forme `must_abort` à valeur existante de `test_pico8_ui`. Cibles : un `hj`, un `pm` et un `mx` neufs. Valeurs de `qy`, les cas « union nulle » construits par `memset` puis `kind` :
  - fixe 1, **fixe 0 à union nulle** ;
  - `true`, `false`, **`false` à union nulle** ;
  - chaîne `"x"`, **chaîne NULL** ;
  - **fonction NULL** ;
  - REF `ob_table`, REF struct (un `jo` neuf), REF vecteur (tag `UNKNOWN`).

  **Exclus**, parce qu'ils sont dans le genre : `REF NULL` (nil) et toute liste. Il est **rouge sur l'ancien code** (H4) et vert après : une ligne « Runs » pour chacun. Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Tests existants qui figent déjà beaucoup** : `STRUCT_HASH` de `jo_spec_defaults` (rapide, les 13 noms sur des neufs) ; dans les TU lents, `fold_ascension`, `test_life`, `hj_tables[]`, `inherited[]`, `bool_state(pm, "pk")`, les rejeux enregistrés ; les 15 rejeux du filet (`ei` dessine les particules).
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_ui && ./tmp/test_pico8_ui` sauf mention. Chacune est notée dans le commit avec le test qui la rougit, ou avec la justification de son trou. Un appel unique d'aide `static` se retire sous `if (0)` (1b.14b).
  1. **(§7-1) indicateur forcé vrai** : **sans objet dans sa lettre**, aucun `has_` dans l'étape. **Forme voisine** : la branche `qy` du lecteur ne teste plus NULL (rend `REF NULL` présent). Attendu rouge : caractérisation 3 (nil → absent).
  2. **(§7-2) « présent 0 » là où c'était absent** : **sans objet dans sa lettre**, aucun fixe typé ne peut être absent (nil `jamais`, sans indicateur). **Forme voisine** : `pico8_hj_pk` rend `pico8_value_fixed(0)` quand `pk_is_bool`. Attendu rouge : caractérisation 1 (`pm` : `BOOL` 0 attendu) et 2 (`FIXED` 0 est vrai : `pa` appliqué à la particule de `pm`) ; `jo_spec_defaults` (`STRUCT_HASH`).
  3. **(§7-3) init qui saute un membre typé** : aucune boucle d'init ne reste, et l'omission pure laisse de la mémoire de `malloc` non déterministe. **Trou justifié de la lettre**, forme « pose une autre valeur » (précédents 1b.14a, 1b.23d, 1b.24, 1b.25) :
     - (a) `pico8_hj_struct_init` pose `py = g_hj_ki` → rouge : caractérisation 1 et 2, `jo_spec_defaults` ;
     - (b) la pose `pk`/`pk_is_bool` de `pm` est mise sous `if (0)` (`pm` garde le `FIXED` 1 de `hj`) → rouge : caractérisation 1 et 2 ;
     - (c) la pose `k_ = g_mx_k_` de `mx` est mise sous `if (0)` → rouge : caractérisation 1 (`assert` de pointeur) ;
     - (d) `pico8_planet_family_init` pose dans `g_hj_k_` une copie neuve de la table → rouge : caractérisation 1 (`assert` de pointeur) ; `hj_tables[]` de `test_pico8_hj` (lent) noté ;
     - (e) `qy = NULL` retiré de `pico8_hj_struct_init` : **mutant équivalent, noté** : tout constructeur appelle `f_` avant qu'un lecteur ne touche `qy` (`pm:pq` ne le lit pas), et un `malloc` non initialisé n'est pas vu par ASan.
  4. *Écrivain de `qy` muet* (`if (0) family_take_list(…)`, `return 1`) → rouge : caractérisation 1 (`qy` absent après `f_`) et 3.
  5. *Tag non vérifié* (conjonction `LIST` retirée) → rouge : test d'arrêt (table, struct, vecteur).
  6. *Nil jugé sur le pointeur seul* (`value.kind == PICO8_VALUE_REF` retiré du test de sortie) → rouge : test d'arrêt (fixe 0, chaîne NULL, fonction NULL, `false` à union nulle).
  7. *Branche `k_` du lecteur qui rend `qh.px`* → rouge : caractérisation 1 (`assert` de pointeur) et 2 (couleurs de `ei`).
  8. *`family_trade_default_hf` rend `is_bool = 1`* (`mx` lit `BOOL` 1) → rouge : caractérisation 1.
  9. *Branche `rr` du lecteur sans test NULL* : **mutant équivalent, noté** : `rr` n'est jamais NULL sur l'arbre (pointeur de spec, jamais écrit).
- **`gcov`** : `test_pico8_ui` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23, sur `planet_family.c` et `planet_family_trade.c`. Chaque ligne modifiée doit être exécutée : déballages et remises à nil des 3 genres, poses R3 de `hj`, `pm`, `mx`, les 13 branches R4 (les deux états de `pk`, les deux de `qy`), `pico8_hj_pk`, `family_take_list` (sortie nil et recopie), les deux aides de lecture de spec nouvelles. Trous attendus : l'échec des `assert` (fils de `must_abort`, prouvés par le test d'arrêt et les mutations 5–6) et celui des aides de lecture de spec (inatteignable, la spec est un littéral). Le rapport va dans le commit.
- **Prior art** : `test_qh_members_by_name`/`test_qh_writer_stops_off_row` (1b.24, même TU) ; `test_jo_*` (1b.23a–d) ; `jo.hf`/`family_default_hf` (1b.23a1) ; `qh.qh`/`pico8_qh_qh` (1b.24) ; `family_take_vec` (1b.23b) ; `test_life` de `test_pico8_hj`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer. Ordres de grandeur :

- `./tmp/test_pico8_ui` : 10 à 30 s (H8). Environ 17 lancements (départ, arrêt rouge, final, 12 mutations 1, 2, 3a–3d, 4–8, `gcov`), soit environ 6 min. Si un lancement dépasse 30 s : mutations 1, 2, 3a, 3b, 3c, 4, 5, 6 d'abord, les autres notées non lancées ;
- `make test` : 20 s à 1 min, une fois ;
- `make e2e` : 4 à 6 min, une fois ;
- `make class-coverage-net` : 3 à 4 min, **dans la même invocation** que `make e2e` (arbres distincts, 1b.0) ;
- `make sanitize-net` : environ 4 min seul, **jamais** en même temps que `e2e`, une fois sur l'arbre final ;
- `make value-members` : quelques secondes ;
- R-a et R-b (§6) : seulement s'il reste du budget.

**TU lents touchés** : `test_pico8_hj` et `test_pico8_pm` (tableau `written[]` et boucle supprimés), `test_pico8_mx` (`written[]` réduit à `{ht}`). `test_pico8_ie`, `jo`, `qh`, `os`, `nr` ne sont pas modifiés, mais `make e2e` les lance tous. **Construire les huit tôt**, juste après le typage et avant toute mutation, en compilation seule : `make -j tmp/test_pico8_hj tmp/test_pico8_pm tmp/test_pico8_mx tmp/test_pico8_qh tmp/test_pico8_os tmp/test_pico8_nr tmp/test_pico8_jo tmp/test_pico8_ie`. Ils ne sont lancés que par `make e2e`.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre (départ / final / mutation n / §6 R-x bug remis / §6 R-x restauré) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` vert, et les **8** TU de `UNIT_TESTS_SLOW` passent (`test_pico8_ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 112`**, aucune ligne `pico8_hj_t` ;
5. `timeout 15m make class-coverage-net` : **identique à `tools/class_coverage_net_expected.txt`**. Elle s'applique : le diff touche `pico8_hj_struct_init` et les constructeurs de `hj`, `pm` et `mx`, sans toucher `pico8_class_coverage_record` ni `KNOWN[]`.

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib. On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements : le rejeu `empty_planet_cleared_events.json` est seulement **lu** par la caractérisation.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

**Départ et filet**

- `FAILED:` si 1b.25 n'est pas commitée au départ, ou si `make value-members` ne rend pas 125 sur l'arbre de départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`). Les changements non commités de l'utilisateur sur ces fichiers ne sont ni modifiés, ni indexés, ni commités.

**Types, tables et écrivains**

- `FAILED:` (« champ sans règle ») si `family_take_list` ou une aide de lecture de spec avorte dans un rejeu ou dans un TU hors des fils de `must_abort` (H2), ou si un type est élargi au lieu d'avorter (`pico8_value_t` gardé, `has_` ajouté, `qy` qui accepte un non-liste).
- `FAILED:` si l'un des 13 membres n'a pas le type retenu à sa place, si `pk_is_bool` manque ou n'est pas juste après `pk`, ou si `pico8_hj_pk` n'est pas l'emballage unique appelé par le lecteur.
- `FAILED:` si `pk` ne se lit plus `BOOL` 0 présent sur `pm` et `FIXED` 1 sur `hj`/`mx`, ou si `qy` nil ne se lit plus absent.
- `FAILED:` si `rr` ou `k_` n'est pas le **pointeur de la spec** qui le définit, ou si un `static` de défaut n'est pas remis à nil dans son `defaults_reset`.
- `FAILED:` si `pico8_hj_t` garde un `pico8_value_t`, si `g_hj_members` ou `g_hj_defaults` existe encore, ou si une table d'offsets liste encore l'un des 13 noms.
- `FAILED:` si une autre entrée de `g_pm_defaults`, `g_pm_members` ou `g_mx_defaults` est retirée ou modifiée.
- `FAILED:` si l'écrivain de `hj`/`pm` n'accepte plus exactement ses 21 noms ou celui de `mx` ses 22, ou si une boucle de clés change.
- `FAILED:` si `family_take_list` viole 1b.13/1b.15 : état nil pas posé d'abord ; genre **et** pointeur pas testés ; hors-genre à union nulle accepté ; message sans `hj.qy` et `ref:liste` ; aide partagée entre fichiers ou rendue publique.

**Diff et preuves**

- `FAILED:` si un site par nom change (`particle.c`, `probe.c`) ou est remplacé par un accès direct.
- `FAILED:` si `tools/`, le `Makefile`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` change, ou si un crochet du recensement change.
- `FAILED:` si une preuve de la section « Preuves » ne rend pas ce qui est attendu.
- `FAILED:` si le diff d'un test fait autre chose que : ajouter à `test_pico8_ui` la caractérisation, le test d'arrêt, `HJ_MEMBERS_HASH` et leurs appels ; retirer `qy` des 3 `written[]` ; supprimer ceux de `hj` et `pm` avec leur boucle et les variables devenues inutiles. Ou si une valeur figée existante change (H5).
- `FAILED:` si un test de régression (§6) a un diff. Pour R-a et R-b, soit leurs deux lignes « Runs » (rouge, vert), soit la mention « non lancé, homonyme hors chemin » au journal.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)` sur l'arbre final.
- `FAILED:` si `make value-members` ne rend pas 112, liste encore `pico8_hj_t`, ou si un autre typedef gagne ou perd une ligne.
- `FAILED:` s'il manque une ligne « Runs » de `make class-coverage-net` identique à la référence, ou si le diff touche `pico8_class_coverage_record` ou `KNOWN[]`.

**Tests de l'étape**

- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, si elle lit ou écrit l'un des 13 membres directement, ou si `HJ_MEMBERS_HASH` change après.
- `FAILED:` si la caractérisation ne couvre pas `hj`, `pm` et `mx` neufs et vivants, les 13 noms, les deux états de `pk`, et `qy` écrit, relu et éteint par nom.
- `FAILED:` si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'arbre de départ et une verte sur l'arbre final, ou s'il lui manque les cas à union nulle `memset`, `true`, ou les REF non-listes.
- `FAILED:` si le commit ne contient pas son rapport `gcov` et ses mutations (1, 2, 3a–3e, 4 à 9), chacune avec son test ou la justification de son trou.

**Commit et note**

- `FAILED:` si le travail n'est pas en un commit vert, ou si un membre propre d'une fille (`pm.ng`/`fz`, `mx.ht`) y est typé.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.
- `FAILED:` si le budget d'environ 20 minutes est dépassé sans que le journal note ce qui n'a pas été lancé.
- `FAILED:` s'il manque le tableau « Runs » ou la note « Découvert à 1b.26 ».

## Out of Scope

- Les membres de `qh` (1b.24), `jo` (1b.23) et `ke` que portent `hj`, `pm`, `mx`.
- Les membres propres des filles : `pm.ng`/`fz` (1b.27), `mx.ht` (1b.28), et leurs tables `g_pm_members`, `g_pm_defaults` (`fz`, `ng`), `g_mx_defaults` (`ht`).
- Les homonymes : `ga.*` (1b.29), `g_.*` et `fv` (1b.32), `jc.k_` (1b.35), `nl.rr` (1b.25), `nr.rr` (1b.30), `ie_ke.rr`.
- `f_`, `ez`, `ei`, `g__mm` eux-mêmes (sites par nom, partagés avec `ga` et `g_`).
- Étendre `family_take_list` à `ga.qy` ; renommer `family_default_hf` ; réunir les aides des deux fichiers.
- Régénérer `tools/member_census.txt` ; déplacer un TU entre `UNIT_TESTS` et `UNIT_TESTS_SLOW`.
- Les fichiers d'événements et `FRAMES_EVENTS`, tout changement de rendu, l'audio, lancer l'appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur** (G) :
  - **modules** :
    - `planet_family.h` : typedef, `pk_is_bool`, emballage `pico8_hj_pk`, commentaire du typedef ;
    - `planet_family.c` : lecteur, écrivain de `qy`, `family_take_list`, 23 `static` + 2 `_is_bool`, poses R3 de `hj` et `pm`, suppression de `g_hj_members`/`g_hj_defaults`, 11 entrées retirées de `g_pm_defaults`, messages étendus ;
    - `planet_family_trade.c` : 12 `static` + 1 `_is_bool`, pose R3 de `mx`, 12 entrées retirées de `g_mx_defaults`, `family_trade_default_string` et `family_trade_default_hf`, messages étendus ;
  - **types** : `pk` → `pico8_fixed_t` + `pk_is_bool` (1b.23a1/1b.24 sans écart) ; `qy` → `pico8_list_t *` ; `rr` → `const char *` ; `k_` → `pico8_ob_table_t *` ; les 9 autres → `pico8_fixed_t`, sans indicateur ;
  - **tests** : `test_pico8_ui` étendu (caractérisation, test d'arrêt rouge d'abord) ; 3 `written[]` réduits, dont 2 supprimés ; aucun test de régression modifié ; pas de nouveau TU ;
  - **pas de découpage** : 7 fichiers, une passe ;
  - **`class-coverage-net`** lancé une fois, sur l'arbre final ;
  - **§6** : aucune preuve obligatoire ; R-a et R-b (homonymes) dans le budget seulement.
- **Note « Découvert à 1b.26 »**, à écrire sur la ligne de l'étape. Le reste va dans le commit :

  > patron 1b.1 appliqué ; `py`, `ki`, `pn`, `fc`, `du`, `qo`, `ro`, `qg`, `q_` → `pico8_fixed_t`, `rr` → `const char *`, `k_` → `pico8_ob_table_t *` (pointeurs de spec) ; **`pk` {bool, fixe} → lecture de 1b.23a1 sans écart** (`pk_is_bool`, `pico8_hj_pk`) ; **`qy` → `pico8_list_t *`, forme écrivain de 1b.13/1b.15 sur `ref:liste`** (`family_take_list`, tag `LIST`) ; R2 : aucun indicateur (`stat?` de `rr`/`qy` d'homonymes) ; R7 de 1b.14a sur 2 filles (35 `static`), **`g_hj_members` et `g_hj_defaults` supprimées** ; boucles sentinelles, 2 tableaux vidés supprimés ; §6 : aucun accès, aucune preuve obligatoire ; empreinte réelle 9 (5, 4), 7 fichiers modifiés.
- Le message de commit porte : le périmètre et le compte 125 → 112 ; le tableau des poses R3 par genre ; le résumé du tableau « Runs », le rapport `gcov`, les mutations et leurs trous ; `HJ_MEMBERS_HASH` et les réponses à H1–H17.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
