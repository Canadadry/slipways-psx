---
title: "Plan 3, étape 1b.29 : `ga`, l'éclat de particules. 11 membres typés, `qy` en `pico8_list_t *` écrit par nom (forme 1b.13/1b.15, `family_take_list` étendue), clés de construction typées ; `g_ga_members`, `g_ga_defaults` et `keys[]` supprimées"
description: "Après 1b.28, `pico8_ga_t` garde 11 membres `pico8_value_t` : 10 défauts de spec (`fz`, 8 fixes, `k_`), dont 8 aussi clés de construction de `pm:pq`, et `qy`, la liste de particules que `f_` écrit par nom. 1b.29 les type selon R1–R9 de 1b.1, sans indicateur ; `qy` garde sa branche d'écrivain sous la forme 1b.13/1b.15 ; la table de clés `keys[]` se vide et part avec sa boucle ; `g_ga_members` et `g_ga_defaults` se vident et sont supprimées ; les défauts sont déballés en `static` (R7 de 1b.14a) ; TROU-21 est préservé. `make value-members` passe de 109 à 98."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1b ») :

> 1b.29 — `ga` (`planet_family.h:110`), `qy` écrit par nom. Empreinte 7 (4, 3). Compte : 98.

Cadre : puces **Nil**, **REF**, **Coexistence** et **Sortie** du §5 « Phase 1b », §3 (`make test` vert, aucun `*.sha256` modifié), §6 (tests de régression), §7 (arrêts, ASan+UBSan, `gcov`, mutations 1b, `class-coverage-net` quand un constructeur change). Modèles : le PRD de 1b.26 (`docs/prd/done/plan3-phase1b-etape26-hj-ascension.md`, écrivain `qy` sur `ref:liste`) et celui de 1b.28 (`docs/prd/triage/plan3-phase1b-etape28-mx-synthetiseur.md`). Notes **citées, pas redérivées** :

- **1b.1, puis 1b.3 à 1b.27** : patron R1–R9. En particulier :
  - **1b.4b** : les fils forkés vident le recensement ; boucles sentinelles arrêtées par `PICO8_MEMBER_CENSUS_STOP()` ;
  - **1b.9/1b.10** : une table d'offsets vidée part avec ses boucles et son seul client ;
  - **1b.14b** : cas à union nulle construits par `memset` puis `kind` ; une mutation qui retire l'unique appel d'une aide `static` se plante sous `if (0)` (`-Werror`) ;
  - **1b.19** : un fixe sans indicateur a une aide sans état nil, et nil y avorte ;
  - **1b.21** : pas d'indicateur sans lecture nil par nom ;
  - **1b.22** : un pointeur typé garde son `assert` à tag ;
  - **1b.23a1** : aides de lecture de spec par genre et par fichier, message aux noms de leurs membres ; valeurs figées = valeurs **observées** ;
  - **1b.23a2** : un membre `l` que l'écrivain de 1a accepte garde sa branche ; un membre typé sort des `written[]`, son écriture par nom est figée par la caractérisation ;
  - **1b.23b** : aides d'écriture par genre et par fichier, **partagées entre l'écrivain et les boucles de clés** (`family_take_vec`) ; **R7 sur la table de clés de fille `keys[]` de `ga`** (entrée retirée, table gardée, branche typée avant la recherche) ; le grep §6 couvre `planet->m`, pas seulement `jo\.m` ;
  - **1b.23d** : `by` de `ga` → `g_ga_by` (`family_default_list`), pointeur de spec partagé ; la vérification §6 de `ga_export_panel_by` par l'**échange de `by`** retiré (`SIGABRT` dans `family_default_list`, H16 de 1b.23d, confirmée) ;
  - **1b.24** : le crochet `STOP` d'une boucle supprimée est **gardé seul** ; preuve de typedef par `awk` (la plage `sed` se rouvre) ;
  - **1b.25** : caractérisation dans le TU de la classe quand il est rapide ; `family_take_fixed` (nil avorte) ;
  - **1b.26** : `hj.qy` → `pico8_list_t *`, `family_take_list` (tag `LIST`) ; `ga.qy`, « dans le même fichier, pourra l'étendre en élargissant son message » ;
  - **1b.27** : patron `static` sans fille, tables supprimées ; §6 sans correspondance ;
  - **1b.28** (supposée commitée, H1) : `mx.ht` ; ne touche ni `planet_family.[ch]` ni `test_pico8_ga`.
- **Patron des défauts `static` (R7 de 1b.14a, repris en 1b.23–1b.28)** : déballage une fois à l'init dans `static g_<X>_<m>` au même pointeur pour une REF ou une chaîne, remise à nil dans `defaults_reset`, pose typée à l'endroit de l'ancienne boucle, avant les clés et tout `pq`.
- **Forme écrivain de 1b.13/1b.15** : une branche `strcmp` par membre `e` ; aide `static` par genre et par fichier, jamais partagée entre fichiers ; l'aide pose **d'abord** l'état nil, sort sur `REF NULL` (genre **et** pointeur testés), sinon `assert` « champ sans règle » nommant `struct.membre` et le genre de sa ligne ; **cas d'arrêt `memset`** : fixe 0, chaîne NULL, fonction NULL, `false`, à union entièrement nulle ; un `must_abort` nouveau, **rouge sur l'ancien code**.
- **1b.23d — un tableau `written[]` vidé est supprimé avec sa boucle**, les autres variables restent (et **1b.24** : son crochet `STOP` reste seul).
- **1b.11 — R2 des TU lents** : une lecture par nom faite seulement par un TU de `UNIT_TESTS_SLOW` compte comme `l`, et le nil qu'elle lit comme `lu`. Revue plus bas.
- **Lectures de R1 pour les genres mêlés** (1b.5 {fixe, bool} ; 1b.7 {bool, chaîne}, `false` seul ; 1b.16 {fixe, `ref:X`} ; 1b.23a1/1b.24 {bool, fixe} à nil `jamais` ; 1b.24 {`false` seul, REF à plusieurs cibles}) : **aucune ne s'applique**. Les 11 lignes de `ga` ont chacune un seul genre. Rien à trancher sur ce point.

### État de départ

HEAD `40082dc` (PRD de 1b.28), avec 1b.28 en cours d'implémentation dans l'arbre (`planet_family_trade.[ch]`, `test_pico8_mx.c`, `test_pico8_ui.c` modifiés, non commités). **Précondition : 1b.28 est commitée avant que 1b.29 commence (H1).** `make value-members` rend alors **109**, et `pico8_ga_t` porte les 11 lignes ci-dessous. Numéros de ligne indicatifs (arbre 1b.27) ; chaque site est aussi désigné par son contenu.

**Rien n'a été compilé ni lancé pour ce PRD.** Les faits viennent de `grep` et de la lecture du code. Tout fait d'exécution est une hypothèse (H1 à H16).

### Lignes de recensement (`tools/member_census.txt:116-126`)

| membre | accès | genres | nil | preuves-nil | par-nom | type retenu | indicateur |
|---|---|---|---|---|---|---|---|
| `fz` | offset | `chaine` | jamais | - | l | `const char *` (pointeur de spec) | non (pointeur) |
| `fc` | offset | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `qo` | offset | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `ro` | offset | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `qg` | offset | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `q_` | offset | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `k_` | offset | `ref:table` | jamais | - | l | `pico8_ob_table_t *` (spec, ou table de `pm:pq`) | non (pointeur) |
| `iu` | offset | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `py` | offset | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `ms` | offset | `fixe` | jamais | - | l | `pico8_fixed_t` | non |
| `qy` | offset | `ref:liste` | observé | stat? | **le** | `pico8_list_t *` | non (pointeur) |

Compte : 109 − 11 = **98**. **Aucune ligne `pico8_ga_t` ne reste.**

- **R2 pour `qy`** (lecture de 1b.26 pour `hj.qy`) : pointeur, NULL dit nil. Son `stat?` vient d'homonymes (`g_.qy`, `hj.qy`). Sur `ga`, `qy` est nil de l'init jusqu'à `f_` (appelé par `ga:pq` dans le constructeur), puis toujours une liste.
- **R2 pour `fz`** : pointeur. Le seul nil lu par nom est celui que `test_ga_is_a_struct` pose **directement** (`->fz = REF NULL`, après le crochet `STOP`) avant `!cls->field(roads[2], "fz")` ; NULL suffit.
- **`fc` vaut 0** sur un `ga` de spec (`fc=0`) : un fixe **présent** à 0 (en Lua `0` est vrai). Le lecteur doit le rendre `FIXED` 0 présent. C'est la cible de la mutation §7-2 voisine.

### Revue des TU lents (R2 de 1b.11)

Relus pour les 11 noms : `test_pico8_pm`, `jo`, `hj`, `mx`, `qh`, `os`, `nr`, `ie`.

- **`test_pico8_pm`** : sur le `ga` que `pm:pq` crée, `is_ga` lit `fz` par nom (`strcmp`), `fold_spawned` lit `ra` (de `jo`) et `list_size(g, "qy")`. Jamais nil. `pm_own[]`/`hj_members[]` et `write_fixed(e, "fz")` portent sur un `pm`, homonymes.
- **`test_pico8_jo`** : `daughter_names[]` ∋ `"ga"`, repère la classe par `pico8_ga_struct_class()` ; aucun des 11 noms lu sur un `ga`.
- **`test_pico8_hj`, `mx`** : `not_*` et `write_fixed(e, "fz")` sur leurs propres structs, homonymes. `qh`, `os`, `nr`, `ie` : aucun des 11 noms sur un `ga`.

**R2 ne donne aucun indicateur. Aucun TU lent n'est modifié.**

### Le cart (`pico8/lua.lua`)

- `ga=jo:jr[[…]]` (`:1067-1080`) : `fz="ga"`, `nu=o()`, `hf=f`, `by=o()`, `kq=b(0,0,-1,-1)`, `fa=3`, **`fc=0`**, `qo=0.9`, `ro=0`, `qg=0`, `q_=0.9`, `k_=o(1,2,4,9,9,10,10,7,7,7,7,7)`, `iu=-0.06`, `py=6`, `ms=3`.
- `ga:pq` (`:1080`) : `f_(self)` (écrit `qy={}`), `ez(self,140)`, `mo.ma=3`.
- `ga:mm` (`:1081-1086`) : lit `ms`, `iu`, `py`.
- `ga:kj` (`:1087-1093`) : lit `self.qy[i]` et `self.k_` ; `ei(self)` (lit `qo`, `qg`, `ro`, `q_`, `k_`, `qy`, `fc` par nom dans le port).
- `pm:pq` (`:1030-1035`) : `ga(ob([[qo=0.97,ro=-0.1255,qg=0.1255,q_=0.97,k_=o(1,2,8,14,…),iu=-0.009,py=9,ms=1.5]],{ra=…}))` : **8 clés de construction** sur les 11 membres.

### Le code (`src/pico8_planet_family.[ch]`, arbre 1b.28 supposé commité)

- **Typedef** `pico8_ga_t` (`.h:124-137`) : `{ pico8_jo_t jo; fz; fc; qo; ro; qg; q_; k_; iu; py; ms; qy; }`, tous `pico8_value_t`. Son commentaire (`nu`, `kq`, `by` et `k_` tiennent les tables de la spec, `pm:pq` donne son propre `k_`, TROU-7) reste vrai.
- **Tables d'offsets** : **29 entrées** sur ces 11 noms, les seules `offsetof(` restantes du fichier (13 lignes).

| table | entrées | clients | après |
|---|---|---|---|
| `g_ga_defaults` (`.c:~913`) | 10 (tous sauf `qy`) | boucle de recopie de `pico8_ga_struct_new` ; boucle de lecture + `assert` « ga: every default … » de `pico8_planet_family_init` (juste **après l'échange de `by`**) ; boucle de `pico8_ga_defaults_reset` ; **`pico8_ga_struct_default`** (seul appelant : le site TROU-21 de `pico8_ga_kj`) | **supprimée** |
| `g_ga_members` (`.c:~960`) | les 11 | lecteur `pico8_ga_field` ; boucle nil de `pico8_ga_struct_new` | **supprimée** |
| `keys[]` (`static` locale de `pico8_ga_struct_new`) | 8 (`qo`, `ro`, `qg`, `q_`, `k_`, `iu`, `py`, `ms`) | boucle de recherche des clés et écriture `*(pico8_value_t *)((char *)self + keys[found].offset)` | **supprimée** (vidée) |

- **L'échange de `by`** (`pico8_planet_family_init`, bloc après `g_ga_spec = pico8_ob(…)`) : remplace le `by=o()` parsé en `ob_table` par une liste neuve, puis libère la table brute. **Il ne change pas.** Le déballage des 10 `static` prend la place de la boucle de `g_ga_defaults`, donc reste après l'échange et avant les déballages existants (`g_ga_by`, `g_ga_nu`, `g_ga_hf`, `g_ga_kq`, `g_ga_fa`).
- **Lecteur** `pico8_ga_field` : boucle sur `g_ga_members` (REF NULL → 0), repli sur `pico8_jo_struct_class()->field`.
- **Écrivain** `pico8_ga_set_field` : branche `qy` qui recopie **toute** valeur, puis délégation à `jo` (17 noms). **18 noms, inchangés.**
- **Accès directs (R6)** :
  - `pico8_ga_mm` : `pico8_ga_fixed(self->ms/iu/py)` (0 si non fixe) ; `pico8_ga_fixed` n'a pas d'autre appelant ;
  - `pico8_ga_kj` : `self->qy` testé `REF && != NULL`, puis `PICO8_REF_AS_LIST_CONST` ; **`/* TROU-21 */` `k_val = pico8_ga_struct_default("k_")`** : la palette des cercles est celle de la **classe**, jamais celle de l'instance (`docs/trous-de-portage.md`, TROU-21).
- **Clés de construction** (`pico8_ga_struct_new`) : `p`, `ra` (`family_take_vec`), `id` (`family_take_string`), puis recherche dans `keys[]` et recopie brute ; toute autre clé tombe sur l'`assert` « ga: the only measured construction keys are p, ra, id (jo:hu) and ra, qo, ro, qg, q_, k_, iu, py, ms (pm:pq) … ».
- **Aides existantes** du fichier : `family_take_fixed` (message `nl.qw`, nil avorte), `family_take_list` (message `hj.qy`), `family_take_vec`, `family_take_string` ; `family_default_string` (`qh.dq, nl/hj.rr, pm.fz`), `family_default_fixed`, `family_default_table`. **Aucune `family_take_table`.**
- **Sites par nom, inchangés** (Coexistence) : `particle.c` (`f_` écrit `qy` ; `ez` lit `qy` ; `ei` lit `qo`, `qg`, `ro`, `q_`, `k_` **de l'instance**, `qy`, `fc`).
- **Homonymes hors étape** : `hj/pm/mx.*` (faits), `g_.*` et `fv` (1b.32), `pm.fz` (fait), `jc.k_` (1b.35).

## Solution

1. **Caractérisation figée sur l'ancien code** (arbre 1b.28 commité), dans le TU **rapide** `test_pico8_ga`, par nom seul, verte avant tout typage.
2. **Test d'arrêt**, rouge sur l'ancien code : écriture de `qy` hors genre, et clé de construction hors genre.
3. **Commit 1b.29** : 11 membres typés ; `g_ga_members`, `g_ga_defaults`, `keys[]`, `pico8_ga_struct_default` et `pico8_ga_fixed` supprimés ; 10 `static` déballés après l'échange de `by` ; `family_take_table` ajoutée ; messages de `family_take_list`, `family_take_fixed` et des aides de lecture étendus à `ga` ; TROU-21 préservé sur `g_ga_k_` ; `written[]` de `test_pico8_ga` supprimé avec sa boucle, crochet `STOP` gardé seul ; `make value-members` rend **98**.
4. Aucun pixel ne bouge. Note « Découvert à 1b.29 » sur la ligne de l'étape.

## User Stories

1. En tant que lecteur de `planet_family.h`, je veux `pico8_ga_t` sans aucun `pico8_value_t`.
2. En tant que lecteur, je veux `fz` en `const char *`, `k_` en `pico8_ob_table_t *`, `qy` en `pico8_list_t *` et les 8 autres en `pico8_fixed_t`.
3. En tant que lecteur par nom (`particle.c`, les TU), je veux que chacun des 11 noms rende sur un `ga` exactement ce que rendait l'ancien code : présence, genre, valeur, identité du pointeur.
4. En tant que `ga` neuf (`pico8_ga_new(NULL)` ou récolte `jo:hu`), je veux `fz` et `k_` au **même pointeur** que la spec de `ga`, et `qy` une liste à moi.
5. En tant que `ga` créé par `pm:pq`, je veux mes 8 clés (`qo`…`ms`, `k_`) telles que `pm:pq` les donne, `k_` étant sa table à 13 entrées, et pas celle de la spec.
6. En tant que `ga:kj`, je veux toujours dessiner les cercles dans la palette de la **classe** (TROU-21), même pour un éclat de protoétoile, et `ei` dans celle de l'instance.
7. En tant que `ga:mm`, je veux lire `ms`, `iu` et `py` typés et produire les mêmes particules.
8. En tant que `f_`, `ez` et `ei`, je veux écrire et lire `qy` par nom, comme avant.
9. En tant qu'écrivain de `qy`, je veux que nil l'éteigne (lecture suivante absente, témoin intact) et qu'une liste soit reprise au même pointeur.
10. En tant que gardien de `qy`, je veux que tout non-nil non-liste avorte, unions nulles `memset` comprises, avec un message qui nomme `ga.qy` et `ref:liste`.
11. En tant que gardien des clés de construction, je veux qu'une clé fixe hors genre (chaîne, booléen, nil) et une clé `k_` non-table avortent.
12. En tant que mainteneur, je veux l'écrivain de `ga` acceptant toujours exactement 18 noms, et la même liste de clés mesurées.
13. En tant que mainteneur, je veux `g_ga_members`, `g_ga_defaults`, `keys[]` supprimées et plus aucun `offsetof(` dans `planet_family.c`.
14. En tant que test de régression `ga_export_panel_by` ou `geoharvest_burst`, je veux rester vert sans diff, et rouge quand mon bug revient.
15. En tant qu'auteur de 1b.30 (`nr`), 1b.31 (`fr`) et 1b.32 (`g_`), je veux `planet_family_trade.[ch]`, `particle.c` et `probe.c` intacts.
16. En tant que gardien du §3, je veux `make test` vert, 15 « conforme » et aucun `*.sha256` touché.
17. En tant que `plan-pilot`, je veux `make value-members` à 98 sans ligne `pico8_ga_t`.
18. En tant que vérificateur, je veux la caractérisation verte sur l'ancien code, le test d'arrêt rouge puis vert, les mutations et leurs trous justifiés, le rapport `gcov` et le tableau « Runs ».

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules et les tests. Ces choix sont tranchés comme le ferait un agent `guideline` : hypothèses explicites, le plus simple qui tienne, changement chirurgical, critères vérifiables. Ils sont marqués (G).

### R1–R9 appliquées

| règle | sur `ga` |
|---|---|
| R1 | Ordre et noms gardés, un membre par ligne : `const char *fz; pico8_fixed_t fc, qo, ro, qg, q_; pico8_ob_table_t *k_; pico8_fixed_t iu, py, ms; pico8_list_t *qy;`. `fz`, `k_` empruntés (spec, ou table de `pm:pq`), jamais libérés (TROU-7). |
| R2 | Aucun indicateur (plus haut). |
| R3 | `pico8_ga_struct_new`, après `pico8_jo_struct_init` et à la place des deux boucles : `fz`, `fc`, `qo`, `ro`, `qg`, `q_`, `k_`, `iu`, `py`, `ms` depuis leurs `static`, puis `qy = NULL` ; les poses de `jo` existantes (`by`, `nu`, `hf`, `kq`, `fa`) suivent, inchangées, puis les clés, puis `pq`. `size_t i` retiré (`-Werror`) ; `int k` reste. |
| R4 | `pico8_ga_field` : 11 branches `strcmp` dans l'ordre du typedef, puis le lecteur de `jo`. Fixes : toujours `FIXED` (0 compris). `fz` : `return 0` si NULL, sinon `STRING` au même pointeur. `k_`, `qy` : `return 0` si NULL, sinon REF au même pointeur. `*out_value` intact sur `return 0`. |
| R5 | Branche `qy` : `family_take_list(value, &self->qy); return 1;`, puis délégation à `jo`, ordre gardé. **Clés** : branches typées (plus bas). |
| R6 | `pico8_ga_mm` lit `self->ms`, `self->iu`, `self->py` ; `pico8_ga_fixed` supprimée. `pico8_ga_kj` lit `qy = self->qy;` (le `PICO8_REF_AS_LIST_CONST` part) ; le site TROU-21 lit **`g_ga_k_`** (plus haut, `/* TROU-21 */` gardé) : `if (g_ga_k_ != NULL && pico8_ob_table_get_index(g_ga_k_, idx, &k_entry) && …)`. |
| R7 | 29 entrées retirées ; `g_ga_defaults` (et son client `pico8_ga_struct_default`), `g_ga_members` et `keys[]` supprimées avec toutes leurs boucles et l'`assert` « ga: every default … » ; les `size_t i` devenus inutiles de `defaults_reset`, du lecteur, de `struct_new` et de `pico8_planet_family_init` retirés. Pas de fille. |
| R8 | `PICO8_MEMBER_CENSUS_SEE(self, pico8_ga_t)` gardé ; `tools/member_census.txt` non édité. |
| R9 | 109 → 98. |

### Défauts déballés (patron `static`) (G)

- 10 `static` : `g_ga_fz` (`const char *`), `g_ga_fc`, `g_ga_qo`, `g_ga_ro`, `g_ga_qg`, `g_ga_q_`, `g_ga_iu`, `g_ga_py`, `g_ga_ms` (`pico8_fixed_t`), `g_ga_k_` (`pico8_ob_table_t *`), déclarés avec ceux de `ga` existants, remis à 0/NULL dans `pico8_ga_defaults_reset`.
- Déballés dans `pico8_planet_family_init` **à la place de la boucle de `g_ga_defaults`**, donc **après l'échange de `by`** et avant `g_ga_by`, par `pico8_ga_default(name, &v)` et les aides existantes, messages étendus (lecture de forme de 1b.23a1) :
  - `family_default_string` : `+ ga.fz` ;
  - `family_default_fixed` : `+ ga.fc/qo/ro/qg/q_/iu/py/ms` ;
  - `family_default_table` : `+ ga.k_`.
- **Pointeur de spec** pour `fz` et `k_` : identité figée par `fold_burst` (`get_ref(e, "k_") == class_ref(pico8_ga_default, "k_")`) et par la caractérisation.
- **Écarté** : des littéraux en R3 (la spec est la seule source) ; garder `pico8_ga_struct_default` avec une table réduite à `k_` (une table d'un seul client, que le `static` remplace).

### Clés de construction : `keys[]` vidée et supprimée (G)

Les 8 entrées de `keys[]` sont des membres typés : elles en sortent toutes (R7 de 1b.23b). La table vidée part avec sa boucle (1b.9/1b.10). La forme devient celle de `pico8_pm_struct_new` (1b.27) : une chaîne de branches nommées `p`, `ra`, `id`, `qo`, `ro`, `qg`, `q_`, `k_`, `iu`, `py`, `ms`, chacune déballée par l'aide de son genre, et **l'`assert` final garde son message mot pour mot** pour toute autre clé (nommée ou positionnelle).

- 7 clés fixes → `family_take_fixed` (existante, **nil avorte**, 1b.19), message étendu : `"nl.qw, ga.qo/ro/qg/q_/iu/py/ms: a write that is not a fixed is off its census row (fixe, never nil) -- …"`.
- `k_` → **`family_take_table`, nouvelle**, forme 1b.13/1b.15 : `*out = NULL;` ; `return` sur `REF NULL` (genre **et** pointeur) ; `assert(REF && ref != NULL && pico8_type_tag_of(ref) == PICO8_TYPE_TAG_OB_TABLE && "ga.k_: a write neither nil nor a table is off its census row (ref:table) -- a field under no rule of plan-3 section 5: stop, do not widen the member")` ; recopie. Un seul appelant.
- **Pourquoi l'état nil d'abord pour `k_`, alors que sa ligne dit nil `jamais` (G)** : c'est un pointeur (NULL représentable, lu absent comme en 1a, où la recopie brute d'un `REF NULL` rendait aussi 0) ; c'est la forme de `family_take_vec` que prennent déjà les clés `p`/`ra` du même constructeur, et celle de `family_trade_take_table` (1b.28). 1b.19 (nil avorte) vise les fixes sans indicateur, pas les pointeurs.
- **Écarté** : garder `keys[]` avec des offsets typés (une table de `size_t` sans genre ne dirait plus comment déballer) ; partager une aide avec `planet_family_trade.c` (1b.13, point 2).

### Écrivain de `qy` (G)

`qy` est écrit par nom (`f_`) et l'écrivain de 1a l'accepte : forme 1b.13/1b.15 par **`family_take_list`, existante**, dont le message s'étend : `"hj/ga.qy: a write neither nil nor a list is off its census row (ref:liste) -- …"` (annoncé par 1b.26). Deux appelants (`hj`, `ga`), même fichier. Écarté : une seconde aide `ga_take_list` (doublon de genre dans un fichier).

### TROU-21, préservé (G)

`ga:kj` du cart lit `self.k_` ; le port lit le `k_` de la **classe**. Le typage le garde à l'identique : `g_ga_k_` remplace `pico8_ga_struct_default("k_")`, jamais `self->k_`. `ei` continue de lire le `k_` de l'instance par nom. Le corriger est interdit (§7). La mutation 4b le prouve.

### `test_pico8_ga` : `written[]` et accès direct

- `written[] = {"qy"}` de `test_ga_is_a_struct` : sa boucle écrit un fixe dans `qy` ; vidé, **supprimé avec sa boucle** (1b.23d) ; le crochet `PICO8_MEMBER_CENSUS_STOP()` qui la précède est **gardé seul** (1b.24, forme `test_pico8_fr:597`) ; `i` et `v` restent utilisés.
- `((pico8_ga_t *)roads[2])->fz = pico8_value_ref(NULL);` → `((pico8_ga_t *)roads[2])->fz = NULL;` : **le seul accès direct** à un membre de `ga` dans `tests/`, désigné par le compilateur. L'`assert` qui suit (`!cls->field(roads[2], "fz", &v) && !is_ga(roads[2])`) ne change pas.
- Rien d'autre ne change dans ce TU hors ajouts ; `BUILT_HASH`, `LIFE_HASH`, `BURST_HASH`, `BURST_BIRTH`, `BURST_DEATH` restent (H5).

### Revue des tests de régression (§6), mécanique

**Liste** : les tests nommés au §6 et dans les lignes « Tests : » des étapes 1a, soit 18 fichiers : `tests/test_pico8_<nom>.c` pour `built_planet_panel_nil_row`, `empty_planet_cleared`, `cheat_code`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`, `ga_export_panel_by`, `score_panel_close`, `layout_nil_item_draws_nothing`, `planet_gh_row_lq`, `rich_planet_exports`, `planet_kj_dispatch`, `pause_menu_music`, `probe_mm_rnd_order`, plus `tests/test_class_coverage.c`. Les 18 existent.

**Greps** (faits pour ce PRD, sur les 18) :

- accès direct : `grep -nE '(->|\.)(fz|fc|qo|ro|qg|q_|k_|iu|py|ms|qy)\b|offsetof\([^)]*\b(fz|fc|qo|ro|qg|q_|k_|iu|py|ms|qy)\)'` → **aucun** ;
- par nom : `grep -nE '"(fz|fc|qo|ro|qg|q_|k_|iu|py|ms|qy)"'` → 3 lignes, 2 fichiers ;
- chemin : `grep -nE 'pico8_ga_|"ga"|pico8_ga_t|geoharvest|burst|pico8_(f_|ez|ei)\b|pico8_pm_pq'` → 3 fichiers.

Toutes les correspondances, chacune avec son plan de preuve rouge :

| # | test | ligne | nature | diff ? | plan de preuve rouge |
|---|---|---|---|---|---|
| R-a | `ga_export_panel_by` | `:80-103` `pico8_ga_t *planet`, `pico8_ga_new(NULL)` ; `:82` `"by"` par nom ; `:111-118` `planet->jo.kw/ra/gq` (membres de `jo`, déjà typés) | **sur le chemin** : le `ga` neuf passe par la pose R3 nouvelle et par l'init de `ga` modifiée juste après l'échange de `by` | **non** (aucun membre de `ga` touché ; la phrase « Without the `by` swap, ga's init aborts first » reste vraie) | **Obligatoire.** Bug remis localement : on retire le bloc d'échange de `by` de `pico8_planet_family_init`. Attendu **rouge** (`SIGABRT` dans `family_default_list`, tag `LIST` exigé ; H13). On restaure : vert. Deux lignes « Runs ». |
| R-b | `geoharvest_burst` | `:86` `pico8_ga_struct_class()` ; `:95` `"qy"` lu par nom (`particle_count`) ; `:158` `live_ga() == NULL` à la frame 289 | **sur le chemin** : `ga.qy` lu par nom (R4 nouvelle), `ga:kj` lit `qy` directement (R6) et règle `p_`, qui retire l'éclat | **non** | **Obligatoire.** Bug remis localement : `pico8_ga_pq` sans son `pico8_ez(self, 140)` (l'étape 1 de l'en-tête du test : `ga:pq` remplit `qy`). Attendu **rouge** : `particle_count(ga) >= GA_KJ_PARTICLE_SLOTS` échoue à la frame de récolte. On restaure : vert. Deux lignes « Runs ». Le bug d'origine exact n'est plus dans l'historique (squash), d'où ce choix (H14). |
| R-c | `probe_mm_rnd_order` | `:35` `"py"` (clé d'un `g_`), `:47` `pico8_ez`, `:49` `"qy"` sur une sonde | homonyme : `g_.*` (1b.32) ; `pico8_ez` est partagé mais **non modifié** | non | **Dans le budget seulement.** Bug d'origine **3175ebd** remis localement (dans `g__mm`, les deux tirages refondus en arguments d'une même expression). Attendu **rouge**. On restaure : vert. Sinon le journal note « non lancé, homonyme hors chemin ». |

Les 15 autres fichiers n'ont aucune correspondance et gardent un diff vide. Qu'un de leurs rejeux construise un `ga` n'est pas connu sans lancer (H6) ; `make test` vert suffit, leur diff étant vide.

### Preuves (sortie attendue sur l'arbre final)

- `awk '/^typedef struct \{/{b=""} {b=b $0 "\n"} /\} pico8_ga_t;/{printf "%s", b}' src/pico8_planet_family.h | grep -c pico8_value_t` : **0** ;
- `make value-members | grep -c pico8_ga_t` : **0** ; total **98** ;
- `grep -rnw 'g_ga_members\|g_ga_defaults\|pico8_ga_struct_default\|pico8_ga_fixed' src/` : **vide** ;
- `grep -rn 'offsetof(pico8_ga_t' src/` : **vide** ; `grep -c 'offsetof(' src/pico8_planet_family.c` : **0** (13 avant) ;
- `grep -n -A1 'TROU-21' src/pico8_planet_family.c` : le marqueur, suivi d'une lecture de `g_ga_k_` ; `grep -n 'self->k_' src/pico8_planet_family.c` : seulement la pose R3 et la clé ;
- `grep -n family_take_table src/pico8_planet_family.c` : la définition et **un** appel ; `grep -n family_take_list` : la définition et **deux** appels ; `grep -n family_take_fixed` : la définition et **huit** appels (`nl.qw` + 7 clés) ;
- `grep -c 'off its census row' src/pico8_planet_family.c` : **+1** par rapport au départ ;
- `grep -nE '(\.|->)(fz|fc|qo|ro|qg|q_|k_|iu|py|ms|qy)\.(kind|as)' src/pico8_planet_family.c` : relu, aucun sur un `ga` ;
- `git diff --stat HEAD -- src/ ':!src/pico8_planet_family.h' ':!src/pico8_planet_family.c' tools/ Makefile` : **vide** ;
- dans `tests/`, seul `test_pico8_ga.c` change : ajouts, suppression de `written[]` et de sa boucle (crochet gardé), la ligne `->fz = NULL`.

### Empreinte et découpage

Ligne : 7 (4, 3). **Fichiers modifiés : 3** (`pico8_planet_family.h`, `pico8_planet_family.c`, `test_pico8_ga.c`) plus le plan. Empreinte réelle (fichiers qui lisent ou écrivent l'un des 11 membres d'un `ga`) mesurée au commit (H15) ; attendus au moins `planet_family.h`, `planet_family.c`, `particle.c` côté `src/`, `test_pico8_ga`, `test_pico8_pm`, `test_pico8_geoharvest_burst`, `test_pico8_jo_spec_defaults` côté `tests/`.

**Aucun découpage (G)**, un commit : 3 fichiers, un seul genre nouveau (`family_take_table`), aucune forme mêlée. Écarté : (a) `qy` seul puis les 10 défauts — même lecteur, même constructeur, `g_ga_members` ne tomberait qu'au second commit, deux `make e2e` et deux `class-coverage-net` hors budget ; (b) défauts puis clés — `keys[]` pointe dans les mêmes membres, impossible séparément (R7).

### Hypothèses (rien n'a été lancé)

- **H1** : 1b.28 est commitée ; `make value-members` rend 109 au départ, 98 à la fin ; seules les 11 lignes `pico8_ga_t` disparaissent. 1b.28 n'a touché ni `planet_family.[ch]` ni `test_pico8_ga`.
- **H2** : dans les 15 rejeux et dans tous les TU hors fils de `must_abort`, `qy` ne reçoit que des listes (`f_`) ou nil (une fois `written[]` supprimé), et les clés `qo`…`ms` ne reçoivent que des fixes, `k_` qu'une table (`pm:pq`).
- **H3** : sur un `ga` de spec, les valeurs lues sont celles du cart (`fc` `FIXED` 0 présent) ; sur un `ga` de `pm:pq`, celles de ses clés, `k_` à 13 entrées. Valeurs figées = valeurs **observées**.
- **H4** : sur l'ancien code, chaque cas du test d'arrêt se termine sans `SIGABRT` (l'écrivain et la boucle de clés recopient tout ; rien ne lit `k_`, `qo`, `ms`… d'une façon qui avorte pendant la construction). Un cas qui avorte déjà est retiré du « rouge d'abord » et noté.
- **H5** : aucune valeur figée ne bouge : `BUILT_HASH`, `LIFE_HASH`, `BURST_HASH`, `BURST_BIRTH`/`BURST_DEATH` de `test_pico8_ga` ; `STRUCT_HASH`/`SPEC_HASH` de `jo_spec_defaults` ; ceux de `test_pico8_pm`, `test_pico8_ui` (dont `MX_MEMBERS_HASH`) ; les 15 empreintes du filet.
- **H6** : on ne sait pas lequel des 15 autres rejeux de régression construit un `ga` ; sans effet sur leur diff.
- **H7** : `make sanitize-net` reste propre (aucune allocation nouvelle ; la table `k_` de `pm:pq` survit à `pico8_ob_table_free(overrides)`, qui ne libère pas les tables imbriquées — lu dans `pico8_ob.c`).
- **H8** : `test_pico8_ga` reste sous 30 s avec deux fonctions de plus (sinon §7 : `UNIT_TESTS_SLOW`, changement de `Makefile` interdit ici → arrêt et note).
- **H9** : le build `-DPICO8_MEMBER_CENSUS` compile avec un `pico8_ga_t` sans `pico8_value_t` (non lancé).
- **H10** : `-Werror` muet : 5 `size_t i` retirés, `found` retiré de `struct_new`, `pico8_ga_fixed`/`pico8_ga_struct_default` supprimées, `family_take_table` appelée une fois.
- **H11** : `pico8_ob_table_set_name(b, "qo", pico8_value_ref(NULL))` garde une entrée nommée à valeur nil (sinon le cas « clé fixe nil » du test d'arrêt est retiré et noté).
- **H12** : rien ne dépend de `sizeof(pico8_ga_t)`.
- **H13** : sans l'échange de `by`, `family_default_list` avorte à l'init et `test_pico8_ga_export_panel_by` est rouge (`SIGABRT`) ; avec, il est vert (H16 de 1b.23d).
- **H14** : sans `pico8_ez(self, 140)` dans `ga:pq`, `test_pico8_geoharvest_burst` est rouge à la frame de récolte.
- **H15** : l'empreinte réelle est comptée au commit ; un écart avec 7 (4, 3) n'est pas un arrêt.
- **H16** : les 18 tests de régression passent sans modification.

## Testing Decisions

- **Un bon test juge ce qu'on lit et écrit par nom, ou l'écran.** Les nouveaux tests n'accèdent à aucun des 11 membres directement : ils compilent sur les deux formes.
- **(G) Où : `test_pico8_ga`.** Rapide (hors `UNIT_TESTS_SLOW`), dans `CENSUS_UNIT_TESTS`, TU de la classe (précédent 1b.25) ; il a déjà `harvest`, `spawn` (les deux routes de construction), `pico8_ga_new(NULL)`, `start_game(EMPTY)`, `in_fresh_child`, `must_abort`, `lookup`, `class_ref`, `fold_value`. Pas `test_pico8_ui` (1b.28 y travaille, et `ga` n'y est qu'invité). Il gagne une forme `must_abort` à valeur (`g_off_row_name`, `g_off_row_value`, `write_off_row`, `zero_union`, recopiée de `test_pico8_ui`) et un constructeur à clé hors genre.
- **Déjà figé** par ce TU, sur l'ancien code : les valeurs des scalaires et des tailles (`fold_burst`, `BUILT_HASH`), l'identité de `k_` par route, les particules de `mm`, les cercles de `kj` en palette de classe sur un éclat de protoétoile coupé à 3 (TROU-21) et les points d'`ei` (`LIFE_HASH`), la vie de l'éclat dans le rejeu (`BURST_HASH`, naissance 205, mort 289). La caractérisation ne refait pas ce travail.
- **Caractérisation `test_ga_members_by_name`**, lancée par `in_fresh_child` après `test_ga_is_a_struct`, hash propre **`GA_MEMBERS_HASH`**. Lecture par le lecteur brut (`cls->field`, témoin `0x1234`) **et** par `pico8_entity_get_field`, qui doivent concorder (retour, genre, valeur, pointeur). Adresses jugées par `assert`, jamais pliées.
  1. **Neufs, trois routes** (`harvest`, `spawn`, `pico8_ga_new(NULL)`) : les 11 noms pliés. `assert` : `fz` au même pointeur que `class_value(pico8_ga_default, "fz").as.string` ; `fc` présent, `FIXED` 0 ; `k_` au pointeur de la spec (récolte, neuf), différent de celui de la spec et à 13 entrées (protoétoile) ; `qy` présent, liste, différente d'une instance à l'autre.
  2. **Clés de construction** : `pico8_ga_new(b)` avec `qo`, `ro`, `qg`, `q_`, `iu`, `py`, `ms` fixes (valeurs distinctes des défauts, dont un 0) et `k_` une table neuve → relus au même fixe et au même pointeur ; les 3 autres noms gardent leurs défauts. Puis `k_` = `REF NULL` en clé → `k_` absent.
  3. **Écrivain par nom de `qy`** : une liste neuve → même pointeur ; `REF NULL` → absent aux deux lecteurs, témoin intact ; la liste d'origine → même pointeur. `cls->set_field` rend 1 pour `qy`, **0** pour les 10 autres noms propres (écrivain brut, rien touché) : l'ensemble de 18 noms est inchangé. `pico8_ez(e, 2)` puis taille de `qy` pliée.
  4. **Intrus** sur le lecteur : `""`, `"q"`, `"qy "`, `"QY"`, `"k"`, `"k__"`, `"fz_"`, `"m"` → 0, témoin intact ; aucun des 11 noms sur un `jo` neuf.
  5. Tout est plié dans **`GA_MEMBERS_HASH`**, relevé au premier passage vert sur l'arbre de départ, puis inchangé.
- **Test d'arrêt `test_ga_writer_stops_off_row`**, par `in_fresh_child` après la caractérisation :
  - **`qy` par nom**, sur un `ga` de chaque route : fixe 1, **fixe 0 à union nulle**, `true`, `false`, **`false` à union nulle**, chaîne `"x"`, **chaîne NULL**, **fonction NULL**, REF `ob_table` neuve, REF `jo` neuf, REF vecteur (tag `UNKNOWN`). Exclus (dans le genre) : `REF NULL` et toute liste.
  - **Clés** (`must_abort` sur un constructeur) : `qo` = `"x"`, `ms` = `true`, `iu` = **`false` à union nulle**, `py` = `REF NULL` (nil avorte, 1b.19, H11) ; `k_` = liste neuve, `k_` = fixe 1, `k_` = **chaîne NULL**. Exclus (dans le genre) : tout fixe, 0 compris, pour les 7 clés fixes ; `REF NULL` et toute table pour `k_`.
  - **Rouge sur l'ancien code** (H4), vert après : une ligne « Runs » pour chacun. Le fils de `must_abort` ne vide rien au recensement (1b.4b).
- **Mutations (§7)**, dans une copie jetable du scratchpad, chacune par `make tmp/test_pico8_ga && ./tmp/test_pico8_ga` sauf mention, notées dans le commit avec leur test rouge ou la justification du trou. Un appel unique d'aide `static` se retire sous `if (0)` (1b.14b).
  1. **(§7-1) indicateur forcé vrai** : **sans objet dans sa lettre** (aucun `has_`). **Forme voisine** : la branche `qy` du lecteur ne teste plus NULL → rouge : caractérisation 3 ; la branche `fz` idem → rouge : `test_ga_is_a_struct` (`!cls->field(roads[2], "fz")`).
  2. **(§7-2) « présent 0 » là où c'était absent** : **sans objet dans sa lettre** (fixes nil `jamais`, sans indicateur). **Forme voisine, le piège inverse** : la branche `fc` rend absent quand le fixe vaut 0 (`0` traité comme nil) → rouge : caractérisation 1, `BUILT_HASH`.
  3. **(§7-3) init qui saute un membre** : aucune boucle ne reste, et l'omission pure laisse de la mémoire `malloc` non déterministe. **Trou justifié de la lettre**, forme « pose une autre valeur » (1b.14a, 1b.24–1b.28) : (a) R3 pose `qo = g_ga_ro` → rouge : caractérisation 1, `BUILT_HASH`, `LIFE_HASH` ; (b) `g_ga_k_` déballé en copie neuve de la table → rouge : `fold_burst` (`assert` de pointeur) et caractérisation 1 ; (c) `g_ga_fz` pris sur `pico8_pm_default("fz")` → rouge : caractérisation 1 (pointeur) ; (d) `qy = NULL` retiré de R3 : **mutant équivalent, noté** (`ga:pq` appelle `f_` avant toute lecture).
  4. **R6** : (a) `mm` lit `self->py` à la place de `self->ms` → rouge : `LIFE_HASH` ; (b) **TROU-21 corrigé** (le site lit `self->k_`) → rouge : `LIFE_HASH` (éclat de protoétoile coupé à 3, cercles dans l'autre palette) ; (c) `kj` lit `qy = NULL` → rouge : `test_burst` (mort ≠ 289), `LIFE_HASH`.
  5. **Clés** : (a) la branche `ms` écrit dans `iu` → rouge : `test_built` (`get(b, "ms") == 1.5`), caractérisation 2 ; (b) la branche `k_` sous `if (0)` → rouge : `fold_burst` (`k_` de protoétoile ≠ spec), caractérisation 2 ; (c) conjonction de tag retirée de `family_take_table` → rouge : test d'arrêt (`k_` liste).
  6. **Écrivain** : (a) conjonction `LIST` retirée de `family_take_list` → rouge : test d'arrêt (table, `jo`, vecteur) ; (b) sortie nil jugée sur le pointeur seul → rouge : test d'arrêt (unions nulles) ; (c) `assert` de `family_take_fixed` retiré → rouge : test d'arrêt (clés `qo`, `ms`, `py`).
  7. Remise à 0/NULL des 10 `static` retirée de `pico8_ga_defaults_reset` : **mutant équivalent, noté** (l'init redéballe avant toute lecture).
- **`gcov`** : `test_pico8_ga` en `--coverage -O0`, avec le piège `__gcov_dump` des fils de 1a.23, sur `planet_family.c`. Lignes attendues exécutées : les 10 déballages et remises à nil, la pose R3, les 11 branches R4 (les deux états de `fz`, `k_`, `qy`), la branche `qy` de l'écrivain, les 11 branches de clés, `family_take_table` (sortie nil et recopie), les lectures R6 de `mm` et `kj`, le site TROU-21. Trous attendus : échecs d'`assert` (fils de `must_abort`, non vidés) et des aides de lecture de spec (spec littérale). Rapport dans le commit.
- **Prior art** : `test_hj_members_by_name`/`test_hj_writer_stops_off_row` (1b.26), `test_mx_*` (1b.28) de `test_pico8_ui` ; `test_nl_*` (1b.25, TU de la classe) ; `fold_burst`, `test_life`, `test_burst` de `test_pico8_ga`.

### Vérification, budget et tableau « Runs »

**Budget strict : environ 20 minutes de temps de tests.** Au-delà, l'implémenteur s'arrête et note ce qui manque au lieu de relancer.

- `./tmp/test_pico8_ga` : quelques secondes à 30 s (H8) ; environ 20 lancements (départ caractérisation, départ arrêt rouge, final, mutations 1 (×2), 2, 3a–3c, 4a–4c, 5a–5c, 6a–6c, `gcov`), soit ≤ 7 min. Si un lancement dépasse 20 s : 1, 2, 3a, 3b, 4b, 5b, 6a, 6b d'abord, les autres notées non lancées ;
- §6 R-a et R-b (obligatoires) : `./tmp/test_pico8_ga_export_panel_by` et `./tmp/test_pico8_geoharvest_burst`, deux lancements chacun, quelques secondes ; R-c seulement s'il reste du budget ;
- `make test` : une fois (≈ 1 min) ;
- `make e2e` (4–6 min) une fois, **dans la même invocation** que `make class-coverage-net` (arbres distincts, 1b.0) ;
- `make sanitize-net` (≈ 4 min), **jamais** en même temps que `e2e`, une fois sur l'arbre final ;
- `make value-members` : quelques secondes ;
- TU lents : aucun n'est modifié, mais `test_pico8_pm` lit un `ga` ; compiler tôt, sans lancer (`make -j tmp/test_pico8_pm tmp/test_pico8_jo tmp/test_pico8_hj tmp/test_pico8_mx`) ; seul `make e2e` les lance.

Le journal tient un tableau « Runs », une ligne par lancement. Le vérificateur ne relance que ce qui suit le dernier changement de l'arbre.

| # | Commande exacte | État de l'arbre (départ / final / mutation n / §6 R-x bug remis / §6 R-x restauré) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

**Cibles finales**, vertes sur l'arbre final :

1. `timeout 10m make test` ;
2. `timeout 15m make e2e` : **15 « conforme »**, `float-audit` vert, les **8** TU de `UNIT_TESTS_SLOW` verts (`ie`, `jo`, `qh`, `os`, `hj`, `pm`, `mx`, `nr`) ;
3. `timeout 15m make sanitize-net` : **`sanitize: 15 propre(s)`** ;
4. `make value-members` : **`membres pico8_value_t : 98`**, aucune ligne `pico8_ga_t` ;
5. `timeout 15m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` — **requis**, le constructeur `pico8_ga_struct_new` change (`pico8_class_coverage_record("ga")` et `KNOWN[]` intacts).

`make member-census` ne s'applique pas (R8). On ne lance jamais l'appli raylib. **On ne touche jamais `FRAMES_EVENTS`, `RECORDINGS` ni un fichier d'événements** : `empty_planet_cleared_events.json` et `geoharvest_burst_events.json` sont seulement **lus**.

### Critères d'échec (le vérificateur de `prd-autopilot` les traduit en `FAILED:`)

**Départ et filet**

- `FAILED:` si 1b.28 n'est pas commitée au départ, ou si `make value-members` ne rend pas 109 sur l'arbre de départ.
- `FAILED:` si le filet (`make test` ou `make e2e`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé.
- `FAILED:` si le diff ou le commit touche `FRAMES_EVENTS`, `RECORDINGS` ou un fichier d'événements (`tests/fixtures/*events*.json`).

**Types, tables, écrivains et clés**

- `FAILED:` (« champ sans règle ») si `family_take_list`, `family_take_fixed`, `family_take_table` ou une aide de lecture de spec avorte dans un rejeu ou un TU hors des fils de `must_abort` (H2), ou si un type est élargi au lieu d'avorter (`pico8_value_t` gardé, `has_` ajouté, `qy`/`k_` qui acceptent un autre genre).
- `FAILED:` si l'un des 11 membres n'a pas le type retenu à sa place, si `fc` à 0 ne se lit plus présent, ou si `qy` nil ne se lit plus absent.
- `FAILED:` si `fz` ou `k_` d'un `ga` de spec n'est pas le **pointeur de la spec**, si le `k_` d'un `ga` de `pm:pq` n'est plus la table de ses clés, ou si un `static` n'est pas remis à nil dans `pico8_ga_defaults_reset`.
- `FAILED:` si `g_ga_members`, `g_ga_defaults`, `keys[]`, `pico8_ga_struct_default` ou un `offsetof(` existe encore dans `planet_family.c`.
- `FAILED:` si l'échange de `by` change, ou si le déballage des 10 `static` a lieu avant lui.
- `FAILED:` si l'écrivain de `ga` n'accepte plus exactement ses 18 noms, si l'ensemble des clés de construction acceptées change, ou si le message de l'`assert` des clés change.
- `FAILED:` si `family_take_table` ou la branche `qy` viole 1b.13/1b.15 (état nil pas posé d'abord ; genre **et** pointeur pas testés ; hors-genre à union nulle accepté ; message sans `ga.k_`/`ref:table` ou `ga.qy`/`ref:liste`), ou si une aide est partagée entre fichiers ou rendue publique.
- `FAILED:` si TROU-21 est corrigé (le site de `kj` lit `self->k_`), ou si son marqueur disparaît. Plus généralement, si un écart port/cart est corrigé au lieu d'être préservé par un `TROU-N`.

**Diff et preuves**

- `FAILED:` si un autre fichier de `src/` que `planet_family.[ch]` change (dont `particle.c`, `probe.c`, `planet_family_trade.[ch]`), ou si `tools/`, le `Makefile`, `CENSUS_UNIT_TESTS` ou `UNIT_TESTS_SLOW` change.
- `FAILED:` si le diff d'un test fait autre chose que, dans `test_pico8_ga.c` : ajouter la caractérisation, le test d'arrêt, `GA_MEMBERS_HASH`, la forme `must_abort` à valeur et leurs appels ; supprimer `written[]` et sa boucle en gardant le crochet `STOP` ; réécrire la ligne `->fz = NULL`. Ou si une valeur figée existante change (H5).
- `FAILED:` si un test de régression (§6) a un diff, ou s'il manque les deux lignes « Runs » (rouge avec le bug remis, vert restauré) de **R-a** (`ga_export_panel_by`) et de **R-b** (`geoharvest_burst`). Pour R-c, soit ses deux lignes, soit « non lancé, homonyme hors chemin ».
- `FAILED:` si une preuve de la section « Preuves » ne rend pas l'attendu.
- `FAILED:` si `make sanitize-net` n'est pas `15 propre(s)`, si `make value-members` ne rend pas 98 (ou si un autre typedef gagne ou perd une ligne), ou s'il manque la ligne « Runs » de `make class-coverage-net` identique à la référence.

**Tests de l'étape**

- `FAILED:` si la caractérisation n'a pas de ligne « Runs » verte sur l'arbre de départ avant tout typage, si elle lit ou écrit l'un des 11 membres directement, si elle n'est pas dans un TU rapide, ou si `GA_MEMBERS_HASH` change après.
- `FAILED:` si la caractérisation ne couvre pas les trois routes, les 11 noms, les clés de construction et `qy` écrit, relu et éteint par nom.
- `FAILED:` si le test d'arrêt n'a pas une ligne « Runs » rouge sur l'arbre de départ et une verte sur l'arbre final, ou s'il lui manque les cas à union nulle `memset`, `true`, les REF non-listes, ou les cas de clés.
- `FAILED:` si le commit ne porte pas le rapport `gcov` et les mutations 1 à 7, chacune avec son test ou la justification de son trou.

**Commit et note**

- `FAILED:` si le travail n'est pas en un commit vert.
- `FAILED:` si le budget d'environ 20 minutes est dépassé sans que le journal note ce qui n'a pas été lancé.
- `FAILED:` s'il manque le tableau « Runs » ou la note « Découvert à 1b.29 ».

## Out of Scope

- Les membres de `jo` et `ke` que porte `ga` (`nu`, `hf`, `by`, `kq`, `fa`, `p`, `ra`, `id`, `p_`…), déjà typés ; l'échange de `by` lui-même.
- Corriger TROU-21 ; libérer les tables de spec ou la table `k_` de `pm:pq` (TROU-7).
- `f_`, `ez`, `ei` et `g__mm` (sites par nom partagés avec `hj`, `g_`) ; les homonymes `g_.*`, `fv` (1b.32), `jc.k_` (1b.35).
- `nr` (1b.30), `fr` (1b.31) et `planet_family_trade.[ch]`.
- Régénérer `tools/member_census.txt` ; déplacer un TU entre `UNIT_TESTS` et `UNIT_TESTS_SLOW`.
- Fichiers d'événements, `FRAMES_EVENTS`, rendu, audio, appli raylib.

## Further Notes

- **Décisions à la place de l'utilisateur (G)** :
  - **modules** : `planet_family.h` (typedef) ; `planet_family.c` (lecteur, branche `qy`, clés typées, `family_take_table`, 10 `static` et leur remise à nil, pose R3, R6 de `mm`/`kj`, site TROU-21 sur `g_ga_k_`, messages étendus, suppression de `g_ga_members`, `g_ga_defaults`, `keys[]`, `pico8_ga_struct_default`, `pico8_ga_fixed`) ;
  - **types** : `fz` → `const char *`, `k_` → `pico8_ob_table_t *`, `qy` → `pico8_list_t *`, 8 fixes → `pico8_fixed_t`, sans indicateur ;
  - **tests** : `test_pico8_ga` seul (caractérisation, test d'arrêt, `written[]` supprimé, une ligne `->fz`) ; aucun TU lent, aucun test de régression modifié ;
  - **§6** : R-a et R-b obligatoires (chemin), R-c dans le budget seulement ;
  - **pas de découpage** ; **`class-coverage-net`** une fois, sur l'arbre final.
- **Note « Découvert à 1b.29 »** (à écrire sur la ligne) :

  > patron 1b.1 appliqué ; `fc`, `qo`, `ro`, `qg`, `q_`, `iu`, `py`, `ms` → `pico8_fixed_t`, `fz` → `const char *`, `k_` → `pico8_ob_table_t *` (pointeurs de spec, `k_` de `pm:pq` gardé), `qy` → `pico8_list_t *`, aucun indicateur ; forme écrivain 1b.13/1b.15 par `family_take_list` étendue à `ga.qy` ; **`keys[]` de `ga` vidée, supprimée : clés typées en branches (forme `pm`), `family_take_fixed` (nil avorte) et `family_take_table` (nouvelle, nil d'abord)** ; R7 de 1b.14a sans fille, **`g_ga_members` et `g_ga_defaults` supprimées**, plus aucun `offsetof(` dans `planet_family.c` ; TROU-21 préservé sur `g_ga_k_` ; `written[]` de `test_pico8_ga` supprimé, crochet `STOP` gardé seul ; §6 : `ga_export_panel_by` et `geoharvest_burst` sur le chemin, prouvés rouges, sans diff ; caractérisation dans le TU de la classe, rapide ; empreinte réelle mesurée au commit.
- Le message de commit porte : périmètre et compte 109 → 98 ; résumé « Runs » ; `gcov` ; mutations et trous ; `GA_MEMBERS_HASH` ; réponses à H1–H16.
- Un sous-agent frais relit le diff contre ce PRD avant le commit.
