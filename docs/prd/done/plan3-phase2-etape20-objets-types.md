---
title: "Plan 3, étape 2.20 : les objets du jeu ne passent plus en `void *` — les 14 membres de la table de classe et `pico8_state_fn`, leurs 57 cibles de `src/` et 3 de `tests/`, la locale `draw` d'`entity.c` prennent `pico8_struct_header_t *self` ; garde neuve `void-self` ; quatre commits, jeu identique, objets identiques sous -O2"
description: "Quatre sous-étapes rejouées sur copie depuis `253f221` : 2.20a (`kj`, `ev`, locale `draw`), 2.20b (`ldown`, `rdown`, `gv`, `hq`, `has_hb`, `hb`, `hk`), 2.20c (`ey`, `kf`, `cn`, `ol`, `states`/`pico8_state_fn`), chacune par `run220.py` (membre et toutes ses cibles ensemble, appelants par `&x->header`, 5 casts identité retirés, 6 casts descendants ajoutés), puis 2.20d : catégorie neuve `void-self` du compteur (gardée), `void-ptr` perd son exemption `_dispatch`, trois aides de TU typées, 7 lignes de `composition_r2_sites.txt` réécrites sans numéro de ligne. Chaque commit : `make test-all` vert (26 e2e), `sanitize-net` 26 propres, `class-coverage-net` OK, `make raylib` lie, pied `résidu 2 : 0 ligne(s), 0 fichier(s)`, garde verte ; 153 objets identiques sous -O2 -DNDEBUG (hors le TU du compteur en 2.20d), écarts -O0 expliqués (ordre d'évaluation de `&x->header`) ; 14 jumelles conformes ; `neg220.c` refuse, membre par membre, ce que le parent acceptait. Les trois types de rappel du relevé de 2.18 ne portent pas d'objets du jeu : hors périmètre, justifié. Une question à l'humain (aides de TU)."
status: needs-triage
---

## Problem Statement

**Écrit sur `253f221`** (2.19 vérifiée et notée). Arbres de départ : `src` `a63929a4…`, `tests` `968e7267…`, `tools` `4b5abf24…`, `Makefile` `d4e26ba7…`. Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier ; aucun `make` lancé dans le dépôt. Essais sur une seule copie jetable (scratchpad `prd220/`, clone `--no-hardlinks`, `third_party/` lié en symbolique pour `make raylib`, arbres de travail `wt_p` = départ, `wt_a` … `wt_d`, `wt_r` = rejeu), supprimée depuis. Notation : `p` = `253f221` ; `a`, `b`, `c`, `d` = les commits de la copie qui portent 2.20a à 2.20d. Numéros de ligne : ceux de `p`, sauf mention.

Vu du mainteneur du port : la phase 2 a donné un type à chaque objet du jeu (un noyau `pico8_struct_header_t` en premier membre, 26 types d'entité), mais la table de classe — le seul aiguillage polymorphe du port — reçoit encore l'objet par `void *self` dans 13 de ses 15 membres et dans `pico8_state_fn`. Le compilateur accepte donc qu'on passe à `cls->kj` un `pico8_vec_t *`, une chaîne, n'importe quoi ; chaque cible (`*_dispatch`) le reçoit en `void *` et le caste. `void-ptr` est à 0 parce qu'il ne lit que les prototypes de premier niveau des `.h` et exempte les `*_dispatch` (étude 2.10, §2).

### Texte du plan, recopié (`docs/plan-3.md` à `253f221`)

> - 2.20 — objets passés sans leur type (décision de l'humain du 2026-10-01) : les membres de la table de classe et `pico8_state_fn`, les `*_dispatch`, les types de rappel et la locale `draw` d'`entity.c` reçoivent le type de l'en-tête ; le relevé de 2.18 compte 231 erreurs de compilation dans 41 fichiers pour la table seule. À écrire après 2.19.

Décision de l'humain (2026-10-01, questions de 2.18) : « Objets passés sans leur type (reste 1) : une étape dédiée, 2.20. » Reste 1 de 2.18 (note « Découvert à 2.18 ») : « `void *` d'entité hors `void-ptr` : 14 emplacements de la table de classe (typer : 231 erreurs dans 41 fichiers), 56 `*_dispatch` (19 publics), 3 rappels, `entity.c:227`, 6 sites R2 sur 11 avec un `*_dispatch` en amont, 4 renvois datés dans `composition_r2_sites.txt` ».

Règles communes du plan : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256`, rien sous `tests/e2e/` ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée ; pied du compteur `résidu 2 : 0 ligne(s), 0 fichier(s)` ; aucune paire (`inclusion-mutuelle`, `usage-mutuel`) n'entre ; l'appli raylib n'est jamais lancée.

### Inventaire, remesuré à `p`

| Élément | Compte à `p` | Où | Ce que 2.20 en fait |
|---|---|---|---|
| membres de la table en `void *self` | **13** (`kj`, `ev`, `hk`, `hb`, `has_hb` en `const void *`, `ldown`, `rdown`, `gv`, `hq`, `ey`, `kf`, `cn`, `ol`) + `pico8_state_fn` (tableau `states`) = **14 emplacements** | `struct.h:132`, `:145-158` | `pico8_struct_header_t *self` (`const` pour `has_hb`) |
| essai du relevé de 2.18 (les 14 typés par `sed`, rien d'autre) | **231 erreurs, 41 fichiers** (18 de `src/`, 23 TU) : 160 initialisations (dont 18 `[PICO8_STATE_…] =`, 8 `.states =`, la locale `draw`), 37 arguments, 33 comparaisons `cls->m == cible`, 1 affectation | — | elles tombent toutes en typant les cibles et en passant `&x->header` |
| cibles de `src/` | **57** : 56 `*_dispatch` (19 publics dans 8 `.h`, 37 `static`) + `pico8_js_has_hb` (`ui_button.c:49`, `static`, sans suffixe) | 23 fichiers | type du membre ; nom inchangé |
| cibles de `tests/` | **3** : `spy_kj` (`test_pico8_planet_kj_dispatch.c:30`, affecté à `.kj` en `:90`), `giver_down` (`test_pico8_ui.c:714`, `ldown` et `rdown`), `fake_toast_ph` (`test_pico8_iv.c:179`, `states`) | 3 TU | type du membre |
| locale `draw` | 1 | `entity.c:227` | `void (*draw)(pico8_struct_header_t *self, const pico8_vec_t *ra)` |
| lignes `void *self` (avec `const`, `self_ptr`) | **91** dans 32 fichiers de `src/` (= 14 + 19 + 56 + 1 + 1) ; **16** dans 5 TU (10 de fixtures dans `test_composition_residue.c`, 3 cibles de TU, 3 aides : `lu_step`, `assert_own_emitter`, `assert_own_probe`) | — | **0** à `d`, gardé par `void-self` |
| sites R2 avec un `*_dispatch` en amont | **6 / 11** (`pico8_ke_ks`, `pico8_ez`, `pico8_ei`, `pico8_bv`, `pico8_dl`, `pico8_d_`) ; les six prennent déjà `pico8_struct_header_t *` | `tools/composition_r2_sites.txt` | l'amont typé : `aucun` (2.20d) |
| renvois datés de `composition_r2_sites.txt` | **4** lignes citent `probe.c:349` ou `particle.c:152` (aujourd'hui `:324`, `:140`) ; **7** lignes citent un `*_dispatch(void *self)` | idem | les 7 réécrites par noms de fonction, sans numéro de ligne |
| « 3 types de rappel » | `pico8_jp_ptrs_predicate_fn` (`collect.h:10`), `pico8_nv_ptrs_fn` (`:21`), `pico8_ptrs_foreach_fn` (`ptrs.h:35`) et leurs 3 cibles `static` | — | **hors périmètre, corrigé** : voir ci-dessous |

**Correction du relevé de 2.18 sur les rappels.** Les trois types de rappel servent `pico8_ptrs_t`, le conteneur générique ; depuis 2.11r, plus aucun registre d'entités n'est un `pico8_ptrs_t` (`registres` et `ptrs-entité` à 0, gardés ; les listes d'entités passent par `pico8_ents_t`, dont le prédicat `pico8_jp_ents_predicate_fn` est déjà typé, `ents.h:26`). Leurs trois seuls emplois portent des enregistrements `pico8_jy_t` (des « wants », pas des objets du jeu) : `pico8_jo_lz_mw_step` (`planet.c:212`, copie de `hp.i`), `pico8_jo_gh_jp_step` (`planet_display.c:62`, filtre de `jo.jy`), `pico8_la_ptrs_append` (`collect.c:42`, concaténation de deux listes de `jy`, `planet_trade.c:128`) ; `test_pico8_ptrs.c:202` y passe des entiers. Les typer « en-tête » serait faux. Ils restent `void *` (Out of Scope, avec les autres `void *` qui ne sont pas des objets du jeu).

### Contrats reçus et ce que ce PRD en fait

| Point (source) | Ce que fait 2.20 |
|---|---|
| « les membres de la table et `pico8_state_fn` reçoivent le type de l'en-tête » (plan) | tenu : 13 membres + le typedef, répartis en trois sous-étapes (a, b, c) |
| « les `*_dispatch` » (plan, reste 1 : 56, 19 publics) | tenu : les 56 et `pico8_js_has_hb` changent de type de paramètre, pas de nom ni de place |
| « les types de rappel » (plan, reste 1 : 3) | **non tenu, à dessein** : ils ne portent pas d'objets du jeu (correction ci-dessus) ; écrit dans la note « Découvert à 2.20d » |
| « la locale `draw` d'`entity.c` » | tenu (2.20a, elle prend `kj` ou `ev` : les deux membres vont ensemble) |
| 6 sites R2 avec un `*_dispatch` en amont ; 4 renvois datés | tenu en 2.20d : l'amont des six devient `aucun`, les 7 lignes qui citent un `*_dispatch(void *self)` sont réécrites par noms de fonction |
| « type de l'en-tête ou type exact quand il est connu » (demande) | membres et cibles : l'en-tête (une table sert plusieurs sortes) ; aides de TU : le type exact quand il existe (`lu_step` prend `pico8_hy_t *`) |
| règle 1 de Q3 (preuve par le compilateur acceptée) | `neg220.c` : 17 cas, refusés membre par membre au fil des sous-étapes |
| « une garde qui mord doit être prouvée par une mutation » (demande) | garde mordante prouvée en 2.20d (deux mutations du code, deux du script) |

## Solution

Quatre commits, chacun vert et autonome (le pied reste `résidu 2 : 0 ligne(s), 0 fichier(s)` à chacun ; aucune constante d'empreinte ne change ; aucun `#include`, aucun nom de fonction, aucun commentaire de `src/` ne change). Les trois premiers sont le résultat exact de `python3 run220.py <arbre> a|b|c` (annexe A) sur l'arbre du commit précédent ; le quatrième est `git apply patch220d.diff` (annexe B, texte exact).

**Règle de typage, la même pour chaque membre** (`run220.py`) :
1. le membre de `struct pico8_struct_class` (ou `pico8_state_fn`) prend `pico8_struct_header_t *self` (`const pico8_struct_header_t *self` pour `has_hb`) ;
2. chaque cible, **lue dans les initialiseurs de table de l'arbre** (`.m = f`, `[PICO8_STATE_X] = f`) plus `spy_kj`, prend le même type dans sa définition et son prototype ; aucune n'est renommée ;
3. dans le corps d'une cible, un cast `(pico8_struct_header_t *)self` ou `(const pico8_struct_header_t *)self` devenu identité (ou simple ajout de `const`) part ; **5** en tout (`pico8_ei_dispatch`, `pico8_dl_dispatch` en a ; `pico8_bv_dispatch` en b ; `pico8_d__dispatch`, `pico8_qh_ly_dispatch` en c) ;
4. un appelant qui passait un pointeur de type concret passe `&x->header` (même adresse : `header` est le premier membre ; aucun cast neuf côté appelant) ; **55** sites (5 en a, 20 en b, 12 en c, 18 en d) ;
5. une cible qui comptait sur la conversion implicite de `void *` vers son type concret reçoit le cast descendant explicite que portent déjà les autres dispatchers : **6** (5 dans `ui_button.c` — `pico8_js_ev_dispatch`, `pico8_js_kj_dispatch` en a, `pico8_js_ldown_dispatch`, `pico8_js_hb_dispatch` en b, `pico8_js_ph_dispatch` en c — et `fake_toast_ph` de `test_pico8_iv.c` en c).

**Pourquoi par familles de membres, sans signature transitoire ni macro** : un membre et toutes ses cibles doivent changer ensemble (sinon l'initialiseur de table ne compile pas), mais deux membres sont indépendants — aucune cible ne sert deux membres de groupes différents (`giver_down` sert `ldown` et `rdown`, tous deux en b ; `kj` et `ev` vont ensemble à cause de la locale `draw`). Typer par famille garde donc chaque commit compilable sans rien de transitoire : pas de typedef de passage, pas de macro `SELF_T`, pas de cast de pointeur de fonction dans les tables (tous trois interdits, Anti-tassement). Mesuré : après a, a+b et a+b+c, `syn.sh` rend **0 erreur** (Runs 6).

### Les quatre sous-étapes

| Sous-étape | Contenu | Cibles | Diff (fichiers, +/−) | Arbres d'arrivée |
|---|---|---|---|---|
| **2.20a** — dessin | `kj`, `ev`, locale `draw` (`entity.c:227`) ; `spy_kj` | 18 (12 `kj` dont `spy_kj`… et `pico8_ei_dispatch`, 5 `ev` dont `pico8_dl_dispatch`) | 26 (23 `src/`, 3 TU), +37 / −37 | `src` `f864c95f…`, `tests` `d6d3123f…` |
| **2.20b** — souris et survol | `ldown`, `rdown`, `gv`, `hq`, `has_hb`, `hb`, `hk` | 22 (dont `giver_down`, `pico8_bv_dispatch`, `pico8_js_has_hb`) | 28 (15 / 13), +58 / −58 | `src` `1ce65adc…`, `tests` `3005c9de…` |
| **2.20c** — liens, échanges, états | `ey`, `kf`, `cn`, `ol`, `states` et `pico8_state_fn` | 20 (dont `pico8_d__dispatch`, 11 états dont `fake_toast_ph`) | 28 (19 / 9), +47 / −47 | `src` `ded3f088…`, `tests` `c90aecca…` |
| **2.20d** — garde | catégorie `void-self` ; `void-ptr` sans exemption `_dispatch` ; `COMPOSITION_RESIDUE_GUARD` ; fixtures et test neuf de `test_composition_residue.c` ; aides `lu_step`, `assert_own_emitter`, `assert_own_probe` typées ; 7 lignes de `composition_r2_sites.txt` | — | 6 (`Makefile`, 2 `tools/`, 3 TU), +77 / −43 | `src` inchangé, `tests` `14e4d629…`, `tools` `389b7136…`, `Makefile` `05b35f5c…` |

`tools` et `Makefile` ne changent qu'en d ; `src` ne change pas en d.

### La garde : catégorie neuve `void-self`, et `void-ptr` resserré (2.20d)

**Tranché : une catégorie neuve, pas une extension de `void-ptr`.** `void-ptr` est un lecteur de prototypes de premier niveau des `.h` (`read_prototypes`) ; pour voir un membre de struct, un typedef, une définition `static` de `.c`, une locale ou un TU, il faudrait lui ajouter quatre lecteurs. Le reste à garder a une forme simple et uniforme — tout receveur d'objet du jeu s'appelle `self` (ou `self_ptr`) dans ce port, et aucun `void *self` ne reste à `d` —, donc une catégorie à motif suffit, du même genre qu'`un-ou-faux` (une ligne dans `pattern()`) :

```
void-self) printf '%s\n' '\bvoid[[:space:]]*\*[[:space:]]*(const[[:space:]]+)?self' ;;
```

Elle lit `src/` et `tests/`, `.c` et `.h`, commentaires compris (comme toutes les catégories à motif) ; elle entre dans `CATEGORIES` et `SORTIE` (après `void-ptr`) et en fin de `COMPOSITION_RESIDUE_GUARD`. Les dix fixtures de `test_composition_residue.c` qui écrivent `void *self` coupent le jeton (`"void *" "self"`), selon la règle déjà écrite en tête de ce TU. Trois attendus changent, un test neuf la fixe (`test_void_self_sees_every_untyped_receiver` : membre, typedef, définition `static`, locale, `void*self` collé, `void * const self_ptr`, TU ; témoins négatifs `void *e`, `void *ctx`, `pico8_struct_header_t *self`).

**En plus, `void-ptr` perd son exemption `_dispatch`** (`if (name ~ /_dispatch$/ || …) next` → `if (name ~ /_get_…/) next`) : elle n'existait que parce que les 16 puis 19 dispatchers publics prenaient `void *self` (étude 2.10, §2) ; à `c`, aucun ne le fait. Sans elle, un prototype public `pico8_x_dispatch(void *e)` — un `void *` d'un autre nom, que `void-self` ne lit pas — rougit la garde (preuve 6).

**Limite écrite** (dans le test neuf et le commentaire du script) : un `void *` d'un autre nom que `self` dans une définition de `.c`, un membre ou un TU n'est pas lu. C'est le cas des 214 aides de TU (question à l'humain) et des `void *` qui ne sont pas des objets du jeu (Out of Scope). Le compilateur couvre le reste de la table : un membre typé refuse une cible `void *` quel que soit son nom (`neg220.c` N = 15).

### Les aides de TU nommées `self` (2.20d)

`void-self` lit `tests/` : trois aides de TU qui ne sont pas des cibles de table y tomberaient. Elles prennent le type exact quand il existe :
- `lu_step(uint32_t h, void *self, …)` (`test_pico8_hy.c:401`) → `pico8_hy_t *self` (ses appelants passent `fm_self()`, un `pico8_hy_t *`, ou `NULL`) ;
- `assert_own_emitter`, `assert_own_probe` (`test_pico8_ui.c:831`, `:841`) → `(pico8_struct_header_t *self, pico8_struct_header_t *other, size_t size)` (la sorte varie : l'en-tête) ; leurs 18 appels passent `&pico8_X_new(…)->header` en second argument ; le premier, `e[i]`, est un élément de `void *e[24]` (aide de TU, hors périmètre : conversion implicite).

### `tools/composition_r2_sites.txt` (2.20d)

Les 7 lignes qui citent un `*_dispatch(void *self)` sont réécrites ; leurs colonnes 3 (amont) et 4 (preuve) ne nomment plus que des fonctions, sans numéro de ligne (relevé à `p` par `grep` des appelants et de leur fonction englobante, Runs 13). Après 2.20d, les 11 lignes de `--detail` disent `aucun` en amont ; `grep -c 'void \*self\|probe.c:349\|particle.c:152'` rend **0**. Les 4 autres lignes gardent leurs renvois (sans `void *`, hors de la demande ; reste écrit).

### Comptes, mesurés sur la copie

| Mesure | `p` | `a` | `b` | `c` | `d` |
|---|---|---|---|---|---|
| `syn.sh` (erreurs) | 0 | 0 | 0 | 0 | 0 |
| lignes `void *self` (motif de `void-self`), `src/` + `tests/` | 107 | 79 | 44 | 13 | **0** |
| `neg220.c` N = 1…17 refusés (1 = refusé) | `00000000000000000` | `11000000000000110` | `11111111100000110` | `11111111111111110` | idem `c` |
| `make composition-residue` | pied seul `0 / 0` | idem | idem | idem | idem |
| `make composition-guard` | 14 catégories, 0 | 14, 0 | 14, 0 | 14, 0 | **15**, 0 |
| casts retirés / ajoutés (cumul) | — | 2 / 2 | 3 / 4 | 5 / 6 | 5 / 6 |
| `&x->header` ajoutés (cumul) | — | 5 | 25 | 37 | 55 |
| `make test-all` ; e2e conformes | vert ; 26 | vert ; 26 | vert ; 26 | vert ; 26 | vert ; 26 |
| `sanitize-net` ; `class-coverage-net` ; `make raylib` | 26 propres ; OK ; lie | idem | idem | idem | idem |

Pied et paires : `inclusion-mutuelle` et `usage-mutuel` sont dans `SORTIE` ; un pied `0 / 0` dit qu'aucune paire n'est entrée (et 2.20 ne touche aucune ligne `#include`).

### Anti-tassement

**Interdit** :
- un alias (`typedef void pico8_self_t;`), une macro qui tient lieu de type, une signature transitoire, ou un cast de pointeur de fonction (`(void (*)(void *, …))f`) dans une table ;
- renommer un paramètre `self` (en `e`, `obj`, `host`…) pour sortir de `void-self` sans le typer ; renommer, fondre, rendre `static` ou déplacer une cible ;
- ajouter un cast côté appelant là où `&x->header` suffit ; garder un des cinq casts identité ; ajouter un cast descendant ailleurs que dans les six cibles nommées ;
- toucher un `*.sha256`, `tests/e2e/`, une constante `*_HASH`/`FROZEN_*`, une ligne `#include`, un commentaire de `src/`, `docs/glossaire-noms.md`, `docs/trous-de-portage.md` ; toucher `tools/` ou le `Makefile` avant 2.20d ;
- typer les trois rappels de `pico8_ptrs_t` en en-tête, ou toucher une aide de TU qui ne s'appelle pas `self` ;
- réécrire à la main ce que le script ou le diff font, sauf repli.

## User Stories

1. En tant que mainteneur, je veux que `cls->kj(v, …)` avec un `pico8_vec_t *v` ne compile pas, pour chacun des 14 emplacements de la table, afin que l'aiguillage refuse ce qui n'est pas un objet du jeu.
2. En tant que mainteneur, je veux qu'une cible de table écrite avec `void *self` ne puisse pas entrer dans une table, et qu'une fonction qui reprend un `void *self` rougisse `make test`.
3. En tant que relecteur, je veux des commits d'une centaine de lignes, chacun vert et autonome, dont les objets optimisés ne changent pas, afin de lire un changement de type sans craindre un changement de comportement.
4. En tant que gardien de la règle du parent, je veux que chaque mutation des chemins touchés rougisse les mêmes TU et le même nombre d'e2e au parent et à l'enfant.
5. En tant qu'auteur de 2.21, je veux savoir qu'aucune ligne `#include` n'a bougé, que le graphe d'usage n'a pas changé de forme et que `composition_r2_sites.txt` ne cite plus de `void *`.

## Implementation Decisions

### Modules

(préfixe `pico8_` omis)
- 2.20a, `src/` (23) : `bg.c entity.c help.c help.h msgbox.c particle.c particle.h planet.h planet_display.c planet_family.c planet_family.h probe.c probe_comp.h research_icon.c score_panel.c slipway.c slipway.h slipway_display.c struct.h ui.c ui_button.c wormhole.h wormhole_display.c` ; TU `g_`, `pj`, `planet_kj_dispatch`.
- 2.20b, `src/` (15) : `bg.c planet.c planet.h planet_display.c planet_family_trade.c probe.c probe_comp.h research_icon.c score_panel.c score_panel.h score_panel_display.c slipway.c struct.h ui_button.c wormhole.c` ; TU `bg`, `g_`, `hp`, `hy`, `ie`, `js`, `lq`, `mz`, `nl`, `pj`, `struct_header`, `trade`, `ui`.
- 2.20c, `src/` (19) : `layout.c mouse_reader.c msgbox.c planet.c planet.h planet_display.c planet_family.c planet_family.h planet_family_trade.c planet_trade.c probe.c probe_comp.h score_panel.h score_panel_display.c scroll.c slipway.c struct.h ui_button.c wormhole.c` ; TU `hp`, `hy`, `iv`, `js`, `lq`, `mz`, `pj`, `qb`, `trade`.
- 2.20d : `tools/composition_residue.sh`, `tools/composition_r2_sites.txt`, `Makefile` (une ligne), TU `composition_residue`, `hy`, `ui`.
- Chaque sous-étape : `docs/plan-3.md` (note « Découvert à 2.20x » au bout de la ligne 2.20, que 2.20a découpe en quatre sous-lignes).

La liste exacte est `git diff --stat` des commits de la copie (Runs 7).

### Contrat pour 2.21 et la suite (à recopier dans leurs PRD)

| Point | Pour | Ce qu'il faut savoir |
|---|---|---|
| 15 catégories gardées (`…,usage-mutuel,void-self`) ; `CATEGORIES` = `SORTIE` = 15 | 2.21, toutes | le constat C3 de 2.18 compte désormais 15 |
| numéros de ligne | 2.21 | aucun fichier ne change de nombre de lignes en a, b, c (remplacements sur place) ; en d, `composition_residue.sh` +1 ligne après `:63` et +5 après `:81`, `test_composition_residue.c` renuméroté après `:108` |
| graphe d'usage | 2.21 | 2.20 ne change que des types de paramètre et des `&x->header` ; aucun nom neuf n'est employé d'un module à l'autre (non remesuré par `usegraph.py` ; le pied à 0 dit qu'aucune paire n'entre) |
| `void-ptr-r2` | nettoyage | les 11 lignes disent `aucun` ; la liste n'a plus d'objet et peut partir (hors 2.20) |

## Implementation Chunks

Quatre commits, **≈ 45 min chacun** preuves comprises (machine, mesurée sur la copie : script < 1 s ; `proofs.sh` ≈ 25 s ; objets, trois configurations des deux côtés ≈ 30 s ; jumelles, 4 à 6 × 2 ≈ 1 à 2 min ; `neg220.c` 2 s ; le reste est lecture, note et message). **Total ≈ 3 h.** Ordre imposé : a, b, c, d (la garde ne peut entrer qu'une fois le dernier `void *self` parti ; a, b, c sont indépendants entre eux, mais les hachages d'arrivée sont donnés dans cet ordre).

Par sous-étape x ∈ {a, b, c} : copier `run220.py` hors du dépôt ; `python3 run220.py . x` (sortie attendue : `2.20a : 2 membre(s), 18 cible(s), 37 edition(s)` ; `2.20b : 7 membre(s), 22 cible(s), 58 edition(s)` ; `2.20c : 5 membre(s), 20 cible(s), 47 edition(s)`) ; `git add -A src tests` ; comparer les arbres ; preuves P1–P5 ; note, commit.
Pour d : `git apply patch220d.diff` ; arbres ; P1–P6 ; note « Découvert à 2.20d » (avec la correction sur les rappels et la question à l'humain), commit.

**Replis écrits**, dans l'ordre :
1. `run220.py` s'arrête sur un `AssertionError` (texte absent ou compté autrement, cible sans `void *self`) : lire le site, faire le même remplacement à la main selon la règle (type du paramètre ; cast identité retiré ; `&x->header` ; cast descendant seulement dans une cible qui en manque), noter chaque site ;
2. le départ n'est pas `253f221` (arbres différents) : lancer quand même, ne pas comparer les hachages d'arrivée, comparer tous les comptes de la table et rapporter l'écart ;
3. `syn.sh` non vide après le script : une cible ou un appelant neuf ; le typer selon la même règle, jamais par un cast côté appelant ;
4. `git apply` refuse le diff de d : appliquer à la main les cinq parties (script, liste R2, `Makefile`, `test_composition_residue.c`, aides de TU) selon l'annexe B, et noter ;
5. une sous-étape déborde de 50 min : la finir (un membre ne se coupe pas de ses cibles), et rapporter le temps.

## Testing Decisions

- **Pas de `events.json` dans un TU** ; un seul test neuf, `test_void_self_sees_every_untyped_receiver`, dans `test_composition_residue.c` (mini-arbre fabriqué, < 1 s). Aucun `*_HASH` ne bouge : les TU touchés changent d'objet seulement à -O0 (preuve 3).
- **Bon test ici** : un changement de type se prouve par le compilateur (ce qu'il refuse désormais, preuve 4) et par l'identité des objets (le code machine ne dépend pas du type d'un pointeur) ; les seuls changements de texte exécutable sont les casts retirés et `&x->header`, couverts par les jumelles.
- **Prior art** : PRD 2.16 (`neg216.c`, `fncmp.py`, jumelles, garde mordante) ; PRD 2.15d (`syn.sh`, `objs.sh`, `mut.py`) ; PRD 2.17f (catégorie neuve, fixture rouge puis verte, mutations du script) ; règle du parent (D.21).

### Preuves (parent → enfant : `p`→`a`, `a`→`b`, `b`→`c`, `c`→`d`)

1. **Filets** (`proofs.sh`, annexe A, à chaque bout) : `make -j4 test-all` code 0 et **26** `: conforme` ; `make -j4 sanitize-net` → `sanitize: 26 propre(s)` ; `make class-coverage-net` → `couverture: OK -- chaque classe non exemptée est instanciée (26 rejeu(x))` ; `make raylib` code 0 (lie `tmp/main_raylib`, **jamais lancé**) ; `make -s composition-residue` → `résidu 2 : 0 ligne(s), 0 fichier(s)` seul ; `make -s composition-guard` → `… : 0 ligne(s)`. Mesuré : tout vert aux cinq arbres (Runs 8).

2. **Reproductibilité** : `run220.py` a, b, c puis `git apply` sur un arbre de travail neuf à `p` → les arbres de la table « sous-étapes » (Runs 14).

3. **Objets sans `-g`** (`objs.sh`, annexe A : 153 objets, chaque `src/*.c` hors raylib et chaque `tests/*.c` ; `cmpobjs.sh`, `fncmp.py`) :
   - **`-O2 -DNDEBUG`** : 153 / 153 identiques octet pour octet à chaque pas, sauf `tests_test_composition_residue.o` en d (test neuf, attendus changés) ;
   - **`-O0 -DNDEBUG` et `-O0` avec `assert`** (mêmes résultats : aucun numéro de ligne ne bouge en a, b, c) : `p`→`a` 153 identiques ; `a`→`b` 10 TU diffèrent (`bg`, `g_`, `hp`, `hy`, `js`, `lq`, `mz`, `nl`, `struct_header`, `trade`), chacun dans **une seule fonction**, celle qui contient l'appel réécrit en `&x->header` (`test_drag`, `fold_hq`, `built_world` ×3, `built_planet_panel`, `world`, `test_life`, `jc_dragged_over_nothing`, `join`) ; `b`→`c` `planet_display.o` (`pico8_jo_kj` seule, l'appel `ol`) et 6 TU (mêmes fonctions) ; `c`→`d` `test_composition_residue.o` seul (`main` et le test neuf) — `test_pico8_hy.o` et `test_pico8_ui.o` identiques.
   - **Lecture des écarts -O0** : `fncmp.py` classe chaque fonction différente « permutation » — même nombre d'instructions, même multiensemble aux registres près : à -O0, `&x->header` change l'ordre dans lequel gcc charge les arguments (un `mov` de plus tôt, un registre échangé) ; aucun écart à -O2. Les cinq casts identité retirés ne changent aucun objet.

4. **Preuve compilateur négative** (`neg220.c`, annexe A, `cc -std=c99 -Wall -Wextra -Werror -Isrc -DN=… -fsyntax-only`) : N = 1…14 appellent chaque membre (et `states[0]`) avec un `pico8_vec_t *` ; N = 15 met une cible `void (*)(void *, …)` dans `.kj` ; N = 16 recopie la locale `draw` d'avant ; N = 17 est le témoin (tous les membres appelés avec un `pico8_struct_header_t *`). Attendu, et mesuré : la ligne « `neg220.c` » de la table « Comptes » — chaque sous-étape refuse exactement ses membres, le témoin compile partout (Runs 9).

5. **Jumelles, règle du parent** (`mut220.py`, `muts220.py`, annexe A ; une mutation des deux côtés, textes de parent et d'enfant donnés ; `make -k -j4 unit` puis `make -k e2e` ; restauration par `git checkout -f`). **ÉCHEC si un TU rouge au parent n'est pas rouge à l'enfant, ou si l'e2e est < 26 au parent et 26 à l'enfant.** Mesuré, toutes conformes (rouges et e2e identiques des deux côtés) :

| # | Pas | Cible | Mutation | Rouges (parent = enfant) | e2e |
|---|---|---|---|---|---|
| A-draw | a | locale `draw` | `draw(qc, NULL)` | `fr`, `fv`, `g_`, `ga`, `hj`, `hp`, `iv`, `lq`, `mx`, `nr`, `planet_kj_dispatch`, `pm` | 0 |
| A-ei | a | `pico8_ei_dispatch` (cast retiré) | appel retiré | `g_` | 9 |
| A-dl | a | `pico8_dl_dispatch` (cast retiré) | appel retiré | aucun | 6 |
| A-js-kj | a | `pico8_js_kj_dispatch` (cast ajouté) | appel retiré | `fr`, `fv`, `g_`, `ga`, `hj`, `hp`, `js`, `lq`, `mx`, `nr`, `pm` | 0 |
| A-js-ev | a | `pico8_js_ev_dispatch` (cast ajouté) | appel retiré | `js` | 22 |
| A-qh-ev | a | entrée `.ev` de `qh` | ligne retirée | `hj`, `nl`, `nr`, `os`, `pm`, `qh` | 23 |
| B-bv | b | `pico8_bv_dispatch` (cast retiré) | `qa`, `ra` échangés | `fr`, `g_`, `hj`, `jc`, `mx`, `nl`, `nr`, `os`, `qh` | 6 |
| B-js-ldown | b | `pico8_js_ldown_dispatch` (cast ajouté) | rend `{0, NULL}` | `test_class_coverage`, `js`, `replay_from_title`, `ui` | 0 |
| B-js-hb | b | `pico8_js_hb_dispatch` (cast ajouté) | `if (0)` | `js` | 10 |
| B-js-has-hb | b | `pico8_js_has_hb` | rend 1 | `fr`, `ga`, `hj`, `jo`, `js`, `mx`, `nl`, `nr`, `os`, `pm`, `qh` | 26 |
| C-d_ | c | `pico8_d__dispatch` (cast retiré) | appel retiré | aucun | 7 |
| C-qh-ly | c | `pico8_qh_ly_dispatch` (cast retiré) | appel retiré | `fr`, `hj`, `mx`, `nr`, `os`, `pm`, `qh` | 18 |
| C-ol | c | appel `ol` de `planet_display.c` (`&self->lq->header`) | appel retiré | `hj`, `hp`, `lq`, `mx` | 12 |
| C-js-ph | c | `pico8_js_ph_dispatch` (cast ajouté) | appel retiré | `js`, `lq`, `ui` | 0 |

   Déclaré : A-dl et C-d_ ne rougissent aucun TU mais font tomber l'e2e à 6 et 7, des deux côtés ; B-js-has-hb ne change aucun e2e (26) mais rougit 11 TU. Aucune jumelle ne rougit au parent seul. **Piège mesuré** (Runs 11) : un premier passage lancé en parallèle sur des arbres partagés (un arbre est l'enfant d'une paire et le parent de la suivante) a laissé un objet muté périmé et rendu de faux écarts sur A-draw et A-ei ; relancées seules, elles sont identiques. Lancer les jumelles l'une après l'autre (`runmuts.sh`).

6. **Garde mordante (2.20d)**, à `d`, chaque mutation puis `git checkout -f` :
   - `lu_step(uint32_t h, void *self, …)` remis dans `test_pico8_hy.c` (compile : conversion implicite) → `make -s composition-guard` code 2, `void-self: tests/test_pico8_hy.c:401:…`, `résidu gardé (…,void-self) : 1 ligne(s)` ;
   - `void pico8_x_dispatch(void *e);` ajouté à `probe_comp.h` (un `void *` d'un autre nom) → code 2, `void-ptr: src/pico8_probe_comp.h:63:…` (l'exemption retirée mord) ;
   - remis → code 0 ;
   - script, M1 : ligne `void-self)` de `pattern()` retirée → `test_composition_residue` rouge (`:77`, le script sort en code 2) ; M2 : exemption `_dispatch` remise → rouge (`:546`) ;
   - fixture rouge avec le script du parent (`git show c:tools/composition_residue.sh`) → rouge (`:101`), verte avec celui de `d`.
   (Runs 10.)

### Greps de Sortie

À `d` :
1. `grep -cE 'void[[:space:]]*\*[[:space:]]*(const[[:space:]]+)?self' src/*.c src/*.h tests/*.c | awk -F: '{s+=$2} END{print s}'` → **0**.
2. `sed -n '130,165p' src/pico8_struct.h | grep -c 'pico8_struct_header_t \*self'` → **16** (14 emplacements + `hd` + `mm`) ; `grep -c 'void \*' src/pico8_struct.h` → 0.
3. `grep -rnE '_dispatch\((const )?void \*' src` → vide.
4. `git diff p d -- src | grep -E '^[-+]#include'` → vide ; `git diff p d | grep -E '^[-+].*(_HASH|FROZEN_)'` → vide ; `git diff --stat p d -- tests/e2e '*.sha256'` → vide.
5. `git diff p d | grep -E '^\+' | grep -oE '\((const )?[a-z0-9_]+_t \*\)self\b' | sort | uniq -c` → `1 (fake_toast_t *)self`, `5 (pico8_js_t *)self`.
6. `grep '^COMPOSITION_RESIDUE_GUARD' Makefile` → finit par `,usage-mutuel,void-self` ; `CATEGORIES` et `SORTIE` du script ont 15 noms, `void-self` après `void-ptr`.
7. `grep -c 'void \*self\|probe.c:349\|particle.c:152' tools/composition_r2_sites.txt` → 0 ; `sh tools/composition_residue.sh --detail src tests | grep '^void-ptr-r2' | grep -vc aucun` → 0.
8. `git diff --stat p d -- tools Makefile` : seuls `composition_residue.sh`, `composition_r2_sites.txt`, `Makefile` ; et seulement dans le commit de d.

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si 2.20 n'est pas quatre commits a, b, c, d dans cet ordre, chacun vert (`make test-all`, 26 e2e conformes), ou si l'un touche un `*.sha256`, `tests/e2e/`, une constante `*_HASH`/`FROZEN_*`, une ligne `#include`, ou `tools/`/`Makefile` avant d.
- `FAILED:` si, au départ `253f221`, un commit n'a pas les arbres de la table « sous-étapes », sans repli rapporté.
- `FAILED:` si, à un commit, `sanitize-net` ou `class-coverage-net` est rouge, `make raylib` ne lie pas, le pied n'est pas `résidu 2 : 0 ligne(s), 0 fichier(s)` seul, ou la garde n'est pas à 0.
- `FAILED:` si l'appli raylib est lancée.
- `FAILED:` s'il reste une ligne `void-self` à `d` ; si `void-self` n'est pas dans `CATEGORIES`, `SORTIE` et `COMPOSITION_RESIDUE_GUARD` ; si `void-ptr` garde l'exemption `_dispatch` ; si la garde n'a pas été prouvée mordante par les deux mutations de la preuve 6, et le script par M1 et M2.
- `FAILED:` si une cible a été renommée, rendue `static`, déplacée, ou typée par un alias, une macro ou un cast de pointeur de fonction ; si un paramètre `self` a été renommé ; si un cast identité reste, ou si un cast est ajouté ailleurs que dans les six cibles nommées.
- `FAILED:` si `neg220.c` ne rend pas, à chaque commit, la ligne de la table « Comptes ».
- `FAILED:` si un objet de `src/` ou de `tests/` diffère sous `-O2 -DNDEBUG` (hors `test_composition_residue.o` en d) ; si, sous `-O0`, un objet diffère hors de la liste de la preuve 3, ou une fonction que `fncmp.py` ne classe pas « permutation ».
- `FAILED:` si une jumelle a un TU rouge au parent qui ne l'est pas à l'enfant, ou un e2e < 26 au parent et 26 à l'enfant.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si un message de commit omet : les membres et le nombre de cibles du pas, les casts retirés et ajoutés, les appels en `&x->header`, la preuve des objets (avec les écarts -O0 expliqués), la ligne `neg220.c`, la table des jumelles du pas, les filets, le tableau « Runs » ; pour d, en plus, la garde, ses mutations, la correction sur les rappels.
- `FAILED:` s'il manque une note « Découvert à 2.20x » au bout de la ligne 2.20 du plan, ou si le plan change ailleurs.

## Out of Scope

- **2.21** : les deux cycles du graphe d'usage (4 et 19 modules). 2.20 n'y touche pas.
- **Le renommage de phase 3** : aucun nom ne change (ni `*_dispatch`, ni `pico8_js_has_hb`, ni `self`).
- **Les 214 aides de TU** qui prennent un objet du jeu par `void *` sous un autre nom (`cls_of(const void *e)`, `as_X(void *e)`, `is_X(const void *e)`… dans 34 TU ; et les tableaux `void *e[24]` de `test_pico8_ui.c`) : la demande ne les nomme pas ; question à l'humain.
- **Les `void *` qui ne sont pas des objets du jeu**, laissés tels quels :

| `void *` | Où | Pourquoi ce n'est pas un objet du jeu |
|---|---|---|
| le conteneur générique `pico8_ptrs_t` (`void **items`, `add`, `del`, `iter_next`, `foreach` et `pico8_ptrs_foreach_fn`) | `ptrs.h`, `ptrs.c` | allocateur de liste générique, exclu de toutes les catégories ; il ne tient plus d'entités (`registres`, `ptrs-entité` à 0) |
| `pico8_jp_ptrs_predicate_fn`, `pico8_nv_ptrs_fn`, `pico8_jp_ptrs`, `pico8_nv_ptrs`, `pico8_la_ptrs`, `pico8_gu_content_ptrs` et leurs locales `void *e`/`v` | `collect.h`, `collect.c` | parcours de `pico8_ptrs_t` ; leurs emplois portent des `pico8_jy_t`, des chaînes (`hy.invented`) ou des entiers (TU) |
| les trois rappels `static` `pico8_jo_lz_mw_step`, `pico8_jo_gh_jp_step`, `pico8_la_ptrs_append` | `planet.c:212`, `planet_display.c:62`, `collect.c:42` | enregistrements `pico8_jy_t` |
| tous les `void *ctx` / `ctx_ptr` / `r_ctx` (prédicats de `pico8_jp_ents`, visiteurs `cg_visit_fn` des trois `economy_*.c`, `pico8_gn_random_fn`, `pico8_bw_gn_r`, puits de `assets.c`) | `ents.h`, `gen.h`, `economy_*.c`, `probe.c`, `slipway.c`, `score_panel.c`, `galaxy.c`, `assets.c` | contexte opaque d'un rappel (exclu de `void-ptr` par `VOID_EXCLUSION`) |
| locales `void *` lues dans une liste de `jy` ou de chaînes | `economy_tiers.c:224`, `:264`, `:280`, `economy_formulas.c:102`, `planet_family_trade.c:267`, `planet_trade.c:79`, `research_display.c:24`, `:175` ; `economy.c:284` (`(void *)t->px`) | éléments de `pico8_ptrs_t` qui ne sont pas des entités |
| `pico8_jb_item_t.ref` et `pico8_pi_ref` | `map_tables.h:46`, `pi.c:90` | cellule hétérogène d'une ligne de layout (valeur Lua : une entité `lq` **ou** un vecteur, `score_panel_display.c:221-222`) ; la typer demanderait une union étiquetée |
| `put(…, const void *bytes, …)` | `gif.c:14` | octets bruts |

- La taille du code C, l'audio, PSX ; les 4 autres lignes de `composition_r2_sites.txt` (leurs numéros datés n'ont pas de `void *`) et le retrait de `void-ptr-r2`, devenu sans objet.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Quatre sous-étapes par famille de membres**, pas une par membre (13 commits de quelques lignes, preuves répétées) ni une seule (284 lignes). Les familles suivent l'usage : dessin (`kj`, `ev`, liés par la locale `draw`), souris (`ldown`, `rdown` liés par `giver_down`, avec `gv`, `hq`, `hb`, `has_hb`, `hk`), le reste avec les états. Chaque pas fait ≈ 75 à 120 lignes de diff.
- **G2. Ni signature transitoire, ni macro, ni cast de pointeur de fonction** : par famille, chaque commit compile sans eux (mesuré).
- **G3. Une catégorie neuve `void-self`** plutôt qu'étendre `void-ptr` : un motif d'une ligne contre quatre lecteurs neufs. `void-ptr` perd quand même son exemption `_dispatch`, devenue sans objet, pour couvrir le `void *` d'un autre nom dans un prototype public.
- **G4. Les trois rappels de `pico8_ptrs_t` restent `void *`** : ils ne portent pas d'objets du jeu (mesuré). Le relevé de 2.18 les comptait par erreur.
- **G5. `&x->header` côté appelant, cast descendant seulement dans les six cibles** qui comptaient sur la conversion implicite — même forme que les autres dispatchers.
- **G6. Les aides de TU nommées `self` sont typées en d** (trois), parce que `void-self` lit `tests/` ; les 214 autres restent (question).
- **G7. `composition_r2_sites.txt` réécrit en d seulement**, pas au fil de a–c : la liste est d'information (`--detail`), et un seul pas la touche.

### Surprises de mesure, pour `plan-pilot`

1. **Le relevé de 2.18 tient** : 231 erreurs, 41 fichiers, à l'identique sur `253f221` (2.19 n'a pas touché les types).
2. **Les « 3 rappels » ne sont pas des objets du jeu** : ils servent `pico8_ptrs_t`, qui ne tient plus que des `jy`, des chaînes et des entiers.
3. **Six cibles comptaient sur la conversion implicite** de `void *` (cinq de `js`, un faux de TU) : elles reçoivent un cast descendant ; les cinq casts identité retirés compensent presque.
4. **À -O0, `&x->header` permute des chargements** (même code aux registres près) ; -O2 identique partout.
5. **Jumelles en parallèle sur arbres partagés : faux écarts.** Les lancer en série.
6. **Tous les filets tiennent en ≈ 25 s par arbre** ; `make raylib` lie en ≈ 3 s une fois le vendoring présent.

### Questions pour l'humain

1. **Les fichiers de test.** 214 petites fonctions d'aide, dans 34 fichiers de test, reçoivent encore des objets du jeu sans leur type (sous un autre nom que `self`). 2.20 ne les touche pas, et la garde neuve ne les voit pas. Faut-il une étape de plus pour elles, ou les laisser ?
2. **(Information, pas de choix à faire.)** Le relevé précédent comptait trois « rappels » parmi les objets sans type ; ils transportent en fait des fiches de besoins, des textes et des nombres, pas des objets du jeu. Ils restent tels quels.

### Annexe A — scripts (texte exact)

Tous hors dépôt, dans un dossier d'essais `S` qui contient la copie et ses arbres de travail `wt_p`, `wt_a` … `wt_d` (`git worktree add --detach`, `third_party` lié en symbolique depuis le dépôt pour `make raylib`).

`run220.py` :

```python
# run220.py ARBRE GROUPE : 2.20, un groupe de membres de la table de classe
# typés pico8_struct_header_t *self avec toutes leurs cibles. Les cibles sont
# lues dans les initialiseurs de table de l'arbre (src/ et tests/) et dans
# CIBLES_HORS_TABLE ; chaque remplacement doit trouver son texte, sinon arret.
import re, sys, glob, os
T, G = sys.argv[1], sys.argv[2]
GROUPES = {
    'a': ['kj', 'ev'],
    'b': ['ldown', 'rdown', 'gv', 'hq', 'has_hb', 'hb', 'hk'],
    'c': ['ey', 'kf', 'cn', 'ol', 'states'],
}
CIBLES_HORS_TABLE = {'kj': ['spy_kj']}   # test_pico8_planet_kj_dispatch.c:90, affectation
MEMBRES = GROUPES[G]
FILES = sorted(glob.glob(T + '/src/*.c') + glob.glob(T + '/src/*.h') + glob.glob(T + '/tests/*.c'))
def rd(f): return open(f).read()
def wr(f, s): open(f, 'w').write(s)
n_edit = 0
# 1. membres
f = T + '/src/pico8_struct.h'; s = rd(f)
for m in MEMBRES:
    if m == 'states':
        old = 'typedef void (*pico8_state_fn)(void *self, pico8_fixed_t t);'
        assert s.count(old) == 1, old
        s = s.replace(old, old.replace('(void *self', '(pico8_struct_header_t *self')); n_edit += 1
        continue
    pat = re.compile(r'(\(\*' + m + r'\)\()(const )?void \*self')
    assert len(pat.findall(s)) == 1, m
    s = pat.sub(lambda mo: mo.group(1) + (mo.group(2) or '') + 'pico8_struct_header_t *self', s); n_edit += 1
wr(f, s)
# 2. cibles, lues dans les initialiseurs
cibles = set()
for f in FILES:
    if not f.endswith('.c'): continue
    for body in re.findall(r'pico8_struct_class_t\s+\w+\s*=\s*\{(.*?)\n\};', rd(f), re.S):
        for m, fn in re.findall(r'\.(\w+)\s*=\s*(\w+)', body):
            if m in MEMBRES: cibles.add(fn)
        if 'states' in MEMBRES:
            cibles.update(re.findall(r'\[PICO8_STATE_\w+\]\s*=\s*(\w+)', body))
for m in MEMBRES: cibles.update(CIBLES_HORS_TABLE.get(m, []))
cibles.discard('NULL')
vus = {c: 0 for c in cibles}
for f in FILES:
    s = rd(f); o = s
    for c in cibles:
        pat = re.compile(r'(\b' + re.escape(c) + r'\()(const )?void \*self\b')
        k = len(pat.findall(s))
        if k == 0: continue
        vus[c] += k; n_edit += k
        s = pat.sub(lambda mo: mo.group(1) + (mo.group(2) or '') + 'pico8_struct_header_t *self', s)
        # 3. cast identite dans le corps de la definition
        d = re.search(r'\b' + re.escape(c) + r'\((const )?pico8_struct_header_t \*self[^)]*\)\s*\{', s)
        if d:
            end = s.index('\n}', d.end())
            body = s[d.end():end]
            nb = re.sub(r'\((const )?pico8_struct_header_t \*\)self\b', 'self', body)
            n_edit += (body != nb)
            s = s[:d.end()] + nb + s[end:]
    if s != o: wr(f, s)
# 4. appelants : l'objet passe par son en-tete (&x->header, meme adresse) ;
# une cible qui comptait sur la conversion implicite de void * descend par un
# cast explicite, comme les autres dispatchers.
APPELANTS = {
    'a': [
        ('src/pico8_entity.c', 'void (*draw)(void *self, const pico8_vec_t *ra)', 'void (*draw)(pico8_struct_header_t *self, const pico8_vec_t *ra)', 1),
        ('src/pico8_ui_button.c', 'pico8_js_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), self);', 'pico8_js_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_js_t *)self);', 1),
        ('src/pico8_ui_button.c', 'pico8_js_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), self, p);', 'pico8_js_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_js_t *)self, p);', 1),
        ('tests/test_pico8_g_.c', 'cls_of(a)->ev(a, NULL);', 'cls_of(a)->ev(&a->header, NULL);', 2),
        ('tests/test_pico8_g_.c', 'cls_of(b)->ev(b, NULL);', 'cls_of(b)->ev(&b->header, NULL);', 1),
        ('tests/test_pico8_pj.c', 'cls_of(ju)->kj(ju, NULL);', 'cls_of(ju)->kj(&ju->header, NULL);', 2),
    ],
    'b': [
        ('src/pico8_ui_button.c', 'return pico8_js_ldown(self);', 'return pico8_js_ldown((pico8_js_t *)self);', 1),
        ('src/pico8_ui_button.c', '    pico8_js_t *js = self;\n', '    pico8_js_t *js = (pico8_js_t *)self;\n', 1),
        ('tests/test_pico8_bg.c', 'pico8_bg_struct_class()->rdown(bg, ', 'pico8_bg_struct_class()->rdown(&bg->header, ', 1),
        ('tests/test_pico8_g_.c', 'cls_of(bg)->ldown(bg, ', 'cls_of(bg)->ldown(&bg->header, ', 1),
        ('tests/test_pico8_g_.c', 'cls_of(e)->hq(e, ', 'cls_of(e)->hq(&e->header, ', 1),
        ('tests/test_pico8_g_.c', 'cls_of(a)->hq(a, ', 'cls_of(a)->hq(&a->header, ', 4),
        ('tests/test_pico8_ie.c', 'cls_of(no_ke)->ldown(no_ke, ', 'cls_of(no_ke)->ldown(&no_ke->header, ', 1),
        ('tests/test_pico8_nl.c', 'cls_of(placed)->hq(placed, ', 'cls_of(placed)->hq(&placed->header, ', 1),
        ('tests/test_pico8_pj.c', 'cls_of(ju)->hb(ju);', 'cls_of(ju)->hb(&ju->header);', 2),
        ('tests/test_pico8_pj.c', 'cls_of(ju)->hk(ju, ', 'cls_of(ju)->hk(&ju->header, ', 2),
    ] + [('tests/test_pico8_%s.c' % t, 'pico8_jc_struct_class()->hq(jc, ', 'pico8_jc_struct_class()->hq(&jc->header, ', 1)
         for t in ['hp', 'hy', 'js', 'lq', 'mz', 'struct_header', 'trade']],
    'c': [
        ('src/pico8_planet_display.c', 'self->lq->header.cls->ol(self->lq, ', 'self->lq->header.cls->ol(&self->lq->header, ', 1),
        ('src/pico8_ui_button.c', '    pico8_js_ph(self);', '    pico8_js_ph((pico8_js_t *)self);', 1),
        ('tests/test_pico8_iv.c', '    fake_toast_t *f = self;', '    fake_toast_t *f = (fake_toast_t *)self;', 1),
        ('tests/test_pico8_qb.c', 'states[PICO8_STATE_PH](generic, ', 'states[PICO8_STATE_PH](&generic->header, ', 2),
        ('tests/test_pico8_pj.c', 'cls_of(ju)->states[PICO8_STATE_PH](ju, ', 'cls_of(ju)->states[PICO8_STATE_PH](&ju->header, ', 3, 4),
    ] + [('tests/test_pico8_%s.c' % t, 'pico8_jc_struct_class()->ey(jc, ', 'pico8_jc_struct_class()->ey(&jc->header, ', 1)
         for t in ['hp', 'hy', 'js', 'lq', 'mz', 'trade']],
}
# (fichier, texte, remplacement, k[, total]) : les k premieres des total
# occurrences (test_pico8_pj.c:749 tient un void *ju, sans en-tete a nommer).
for fn, old, new, k, *tot in APPELANTS[G]:
    f = os.path.join(T, fn); s = rd(f)
    assert s.count(old) == (tot[0] if tot else k), (fn, old, s.count(old))
    wr(f, s.replace(old, new, k)); n_edit += k
manque = [c for c, k in vus.items() if k == 0]
assert not manque, ('cible sans void *self', manque)
print(f'2.20{G} : {len(MEMBRES)} membre(s), {len(cibles)} cible(s), {n_edit} edition(s)')
```

`syn.sh` :

```sh
#!/bin/sh
# compile tout l'arbre en -fsyntax-only ; rend les lignes error:
cd "$1"
ls src/*.c tests/*.c | grep -vE 'main_raylib|main_screencast_viewer|raylib_backend|class_coverage' \
  | xargs -P4 -n8 cc -std=c99 -Wall -Wextra -Werror -Isrc -fsyntax-only 2>&1
for f in src/main_class_coverage.c tests/test_class_coverage.c; do
  cc -std=c99 -Wall -Wextra -Werror -Isrc -DPICO8_CLASS_COVERAGE -fsyntax-only "$f" 2>&1
done
```

`proofs.sh` :

```sh
#!/bin/sh
# proofs.sh ARBRE : filets d'une sous-etape, une ligne par preuve
cd "$1" || exit 2
make -j4 test-all > tmp_ta.log 2>&1; echo "test-all  : code $? ; $(grep -c ': conforme' tmp_ta.log) conformes"
make -j4 sanitize-net > tmp_sn.log 2>&1; echo "sanitize  : code $? ; $(tail -1 tmp_sn.log)"
make class-coverage-net > tmp_cc.log 2>&1; echo "couverture: code $? ; $(tail -1 tmp_cc.log)"
make raylib > tmp_rl.log 2>&1; echo "raylib    : code $? ; $(ls tmp/main_raylib 2>/dev/null)"
echo "pied      : $(make -s composition-residue | tr '\n' '|')"
echo "garde     : $(make -s composition-guard | sed 's/(.*) :/(…) :/')"
echo "void self : $(grep -cE 'void[[:space:]]*\*[[:space:]]*(const[[:space:]]+)?self' src/*.c src/*.h tests/*.c | awk -F: '{s+=$2} END{print s}') ligne(s) src+tests"
rm -f tmp_ta.log tmp_sn.log tmp_cc.log tmp_rl.log
```

`objs.sh` :

```sh
#!/bin/sh
# objs.sh ARBRE OUT "DRAPEAUX" : objets sans -g de src/*.c (hors raylib) et
# tests/*.c -> OUT/<dossier>_<nom>.o (main_class_coverage.c et
# test_class_coverage.c avec -DPICO8_CLASS_COVERAGE en plus)
cd "$1" || exit 2; mkdir -p "$2"
for f in $(ls src/*.c tests/*.c | grep -vE 'main_raylib|main_screencast_viewer|raylib_backend'); do
  x=; case $f in *class_coverage*) x=-DPICO8_CLASS_COVERAGE ;; esac
  d=$(dirname $f); b=$(basename $f .c)
  cc -std=c99 $3 $x -Isrc -c -o "$2/${d}_$b.o" $f || echo "ECHEC $f"
done
```

`cmpobjs.sh` :

```sh
#!/bin/sh
# cmpobjs.sh A B : objets de A et B compares octet pour octet ; les differents
for o in "$1"/*.o; do cmp -s "$o" "$2/$(basename $o)" || echo "DIFF $(basename $o)"; done
echo "$(ls "$1"/*.o | wc -l) objets"
```

`fncmp.py` :

```python
# fncmp.py A.o B.o : fonctions dont les instructions (adresses, octets et
# cibles de saut locales retires) different ; par fonction, "permutation" si
# les deux corps ont le meme multiensemble d'instructions modulo les registres
import sys, re, subprocess
def fns(p):
    out, cur = {}, None
    for l in subprocess.run(['objdump', '-d', '--no-show-raw-insn', p], capture_output=True, text=True).stdout.split('\n'):
        m = re.match(r'^[0-9a-f]+ <(.+)>:$', l)
        if m: cur = m.group(1); out[cur] = []; continue
        if cur and re.match(r'^\s+[0-9a-f]+:', l):
            ins = re.sub(r'^\s+[0-9a-f]+:\s*', '', l)
            ins = re.sub(r'\b[0-9a-f]+ <[^>]*>', 'L', ins)
            out[cur].append(ins.strip())
    return out
a, b = fns(sys.argv[1]), fns(sys.argv[2])
for f in sorted(set(a) | set(b)):
    if a.get(f) != b.get(f):
        reg = lambda l: sorted(re.sub(r'%r[a-z0-9]+|%e[a-z]+', 'R', x) for x in l)
        print(f, len(a.get(f, [])), '->', len(b.get(f, [])), 'permutation' if reg(a.get(f, [])) == reg(b.get(f, [])) else 'AUTRE')
```

`neg220.c` :

```c
/* neg220.c : compile -DN=1..17 ; N 1..15 passent un pico8_vec_t * ou une
 * cible void * a la table ; 16 la locale draw d'entity.c ; 17 temoin. */
#include "pico8_struct.h"
void neg(const pico8_struct_class_t *c, pico8_vec_t *v, pico8_struct_header_t *h);
void neg(const pico8_struct_class_t *c, pico8_vec_t *v, pico8_struct_header_t *h) {
#if N == 1
    c->kj(v, v);
#elif N == 2
    c->ev(v, v);
#elif N == 3
    c->hk(v, 0);
#elif N == 4
    c->hb(v);
#elif N == 5
    (void)c->has_hb(v);
#elif N == 6
    (void)c->ldown(v, v, v);
#elif N == 7
    (void)c->rdown(v, v, v);
#elif N == 8
    (void)c->gv(v, h);
#elif N == 9
    c->hq(v, v, v);
#elif N == 10
    c->ey(v, h);
#elif N == 11
    c->kf(v, h);
#elif N == 12
    c->cn(v, 0);
#elif N == 13
    c->ol(v, *v, 0);
#elif N == 14
    c->states[0](v, 0);
#elif N == 15
    extern void t15(void *self, const pico8_vec_t *ra);
    static const pico8_struct_class_t k = {.kj = t15};
    (void)k;
#elif N == 16
    void (*draw)(void *self, const pico8_vec_t *ra) = c->kj;
    (void)draw;
#elif N == 17
    c->kj(h, v); c->ev(h, v); c->hk(h, 0); c->hb(h); (void)c->has_hb(h);
    (void)c->ldown(h, v, v); (void)c->rdown(h, v, v); (void)c->gv(h, h);
    c->hq(h, v, v); c->ey(h, h); c->kf(h, h); c->cn(h, 0); c->ol(h, *v, 0);
    c->states[0](h, 0);
#endif
    (void)c; (void)v; (void)h;
}
```

`muts220.py` :

```python
# muts220.py : ecrit muts220.json. Une mutation = [fichier, ancre, texte, remplacement]
# par cote ; le texte est cherche apres l'ancre (None : il doit etre unique).
import json
H = 'pico8_struct_header_t *self'
def twin(f, anc, old_p, old_x, new):
    return {'p': [[f, anc, old_p, new]], 'x': [[f, anc, old_x, new]]}
M = {
 # 2.20a, parent p, enfant a
 'A-draw': twin('src/pico8_entity.c', None, '                draw(qc, ra);\n', '                draw(qc, ra);\n', '                (void)ra;\n                draw(qc, NULL);\n'),
 'A-ei': twin('src/pico8_particle.c', None,
    '    pico8_ei(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_struct_header_t *)self);\n',
    '    pico8_ei(pico8_dispatch_gfx(), pico8_dispatch_mem(), self);\n', '    (void)self;\n'),
 'A-dl': twin('src/pico8_probe.c', None,
    '    pico8_dl(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_struct_header_t *)self);\n',
    '    pico8_dl(pico8_dispatch_gfx(), pico8_dispatch_mem(), self);\n', '    (void)self;\n'),
 'A-js-kj': twin('src/pico8_ui_button.c', None,
    '    pico8_js_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), self, p);\n',
    '    pico8_js_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_js_t *)self, p);\n', '    (void)self;\n    (void)p;\n'),
 'A-js-ev': twin('src/pico8_ui_button.c', None,
    '    pico8_js_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), self);\n',
    '    pico8_js_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_js_t *)self);\n', '    (void)self;\n'),
 'A-qh-ev': twin('src/pico8_planet_family.c', 'g_qh_struct_class = {', '    .ev = pico8_dl_dispatch,\n', '    .ev = pico8_dl_dispatch,\n', ''),
 # 2.20b, parent a, enfant b
 'B-bv': twin('src/pico8_probe.c', None, '    pico8_bv((pico8_struct_header_t *)self, qa, ra);\n', '    pico8_bv(self, qa, ra);\n', '    pico8_bv(self, ra, qa);\n'),
 'B-js-ldown': twin('src/pico8_ui_button.c', None, '    return pico8_js_ldown(self);\n', '    return pico8_js_ldown((pico8_js_t *)self);\n', '    (void)self;\n    return (pico8_down_t){0, NULL};\n'),
 'B-js-hb': twin('src/pico8_ui_button.c', 'pico8_js_hb_dispatch(', '    if (js->hb != NULL) {\n', '    if (js->hb != NULL) {\n', '    if (0) {\n'),
 'B-js-has-hb': twin('src/pico8_ui_button.c', None, '    return ((const pico8_js_t *)self)->hb != NULL;\n', '    return ((const pico8_js_t *)self)->hb != NULL;\n', '    (void)self;\n    return 1;\n'),
 # 2.20c, parent b, enfant c
 'C-d_': twin('src/pico8_probe.c', None, '    (void)pico8_d_((pico8_struct_header_t *)self);\n', '    (void)pico8_d_(self);\n', '    (void)self;\n'),
 'C-qh-ly': twin('src/pico8_planet_family.c', 'pico8_qh_ly_dispatch(', '    pico8_na((pico8_struct_header_t *)self, t);\n', '    pico8_na(self, t);\n', '    (void)self;\n    (void)t;\n'),
 'C-ol': twin('src/pico8_planet_display.c', None,
    '            self->lq->header.cls->ol(self->lq, offset, PICO8_FIXED_LIT(0x00008000, 0.5));\n',
    '            self->lq->header.cls->ol(&self->lq->header, offset, PICO8_FIXED_LIT(0x00008000, 0.5));\n', '            (void)offset;\n'),
 'C-js-ph': twin('src/pico8_ui_button.c', None, '    pico8_js_ph(self);\n', '    pico8_js_ph((pico8_js_t *)self);\n', '    (void)self;\n'),
}
json.dump(M, open('muts220.json', 'w'), indent=1)
```

`mut220.py` :

```python
# mut220.py ARBRE p|x NOM... : applique la mutation (cote p = parent, x = enfant)
# dans ARBRE, make -k -j4 unit puis make -k e2e ; imprime les TU rouges et le
# nombre d'e2e conformes ; restaure par git checkout -f.
import json, sys, subprocess, re, os
W, side = sys.argv[1], sys.argv[2]
M = json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'muts220.json')))
for name in sys.argv[3:]:
    for f, anc, old, new in M[name][side]:
        p = f'{W}/{f}'; s = open(p).read()
        if anc is None: assert s.count(old) == 1, (name, old, s.count(old)); i = 0
        else: i = s.index(anc)
        j = s.index(old, i); s = s[:j] + new + s[j + len(old):]; open(p, 'w').write(s)
    env = dict(os.environ); env.pop('MALLOC_PERTURB_', None)
    u = subprocess.run('timeout 900 make -k -j4 unit', shell=True, cwd=W, capture_output=True, text=True, env=env)
    out = u.stdout + u.stderr
    red = sorted(set(re.findall(r'\*\*\* \[\S+: unit-run-(\w+)\]', out)))
    comp = sorted(set(re.findall(r'error:', out)))
    e = subprocess.run('timeout 600 make -k e2e', shell=True, cwd=W, capture_output=True, text=True, env=env)
    ok = len(re.findall(r': conforme', e.stdout + e.stderr))
    print(f'{name}\t{side}\trouges={",".join(r.replace("test_pico8_","") for r in red) or "aucun"}\terreurs={len(comp)}\te2e={ok}', flush=True)
    subprocess.run('git checkout -qf -- . && git clean -qfd src tests', shell=True, cwd=W)
```

`runmuts.sh` :

```sh
#!/bin/sh
# runmuts.sh : les jumelles de 2.20a-c, l'une apres l'autre (un arbre sert de
# parent a une paire et d'enfant a la precedente)
S=$(dirname "$0")
A="A-draw A-ei A-dl A-js-kj A-js-ev A-qh-ev"; B="B-bv B-js-ldown B-js-hb B-js-has-hb"; C="C-d_ C-qh-ly C-ol C-js-ph"
python3 $S/mut220.py $S/wt_p p $A; python3 $S/mut220.py $S/wt_a x $A
python3 $S/mut220.py $S/wt_a p $B; python3 $S/mut220.py $S/wt_b x $B
python3 $S/mut220.py $S/wt_b p $C; python3 $S/mut220.py $S/wt_c x $C
```

### Annexe B — `patch220d.diff` (2.20d, `git diff c d`, texte exact, 284 lignes, sha256 `95531dfc9a3f1821…`)

S'applique par `git apply patch220d.diff` sur l'arbre du commit de 2.20c (arbres `ded3f088…`, `c90aecca…`, `tools` `4b5abf24…`, `Makefile` `d4e26ba7…`).

```diff
diff --git a/Makefile b/Makefile
index d4e26ba..05b35f5 100644
--- a/Makefile
+++ b/Makefile
@@ -872,7 +872,7 @@ composition-residue: tools/composition_residue.sh
 	@sh tools/composition_residue.sh
 
 # Catégories du compteur de composition tombées à 0, séparées par des virgules.
-COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6,lien-parent,parente,void-ptr,inclusion-mutuelle,usage-mutuel
+COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6,lien-parent,parente,void-ptr,inclusion-mutuelle,usage-mutuel,void-self
 
 composition-guard: tools/composition_residue.sh
 	@sh tools/composition_residue.sh --check $(COMPOSITION_RESIDUE_GUARD) src tests
diff --git a/tests/test_composition_residue.c b/tests/test_composition_residue.c
index 07a4d42..a93383f 100644
--- a/tests/test_composition_residue.c
+++ b/tests/test_composition_residue.c
@@ -89,7 +89,7 @@ static void test_one_occurrence_per_category_in_table_order(void) {
     write_file(SRC "/e.h", "typedef struct {\n    pico8_struct_" "header_t header;\n    pico8_" "ptrs_t *oe;\n} pico8_"
                            "jo_t;\n");
     write_file(SRC "/p.h", "typedef struct {\n    pico8_" "jo_t jo;\n} pico8_" "qh_t;\n");
-    write_file(SRC "/v.h", "pico8_qy_t *pico8_" "g__mm(void *self);\n");
+    write_file(SRC "/v.h", "pico8_qy_t *pico8_" "g__mm(void *" "self);\n");
     write_file(ATT, "q1\tx\n");
     write_file(SRC "/a.h", "#include \"b.h\"\n");
     write_file(SRC "/b.h", "#include \"a.h\"\n");
@@ -106,6 +106,7 @@ static void test_one_occurrence_per_category_in_table_order(void) {
                                   "aiguillage-nom\t1\t1\n"
                                   "un-ou-faux\t1\t1\n"
                                   "void-ptr\t1\t1\n"
+                                  "void-self\t1\t1\n"
                                   "ptrs-entité\t1\t1\n"
                                   "noyau-inclut\t1\t1\n"
                                   "atteinte-à-prouver\t1\t1\n"
@@ -185,10 +186,11 @@ static void test_exclusions_edit_the_text_not_the_whole_line(void) {
 
 static void test_overlapping_categories_count_each_but_footer_counts_distinct_lines(void) {
     setup();
-    write_file(SRC "/o.h", "const pico8_str_seq_t *pico8_jo_get" "_n" "u(const void *self);\n");
+    write_file(SRC "/o.h", "const pico8_str_seq_t *pico8_jo_get" "_n" "u(const void *" "self);\n");
     assert(run_residue(TREE) == 0);
     assert(strcmp(read_file(OUT), "accesseurs-q6\t1\t1\n"
                                   "n" "u\t1\t1\n"
+                                  "void-self\t1\t1\n"
                                   "résidu 2 : 1 ligne(s), 1 fichier(s)\n") == 0);
 }
 
@@ -242,7 +244,7 @@ static void test_registres_reads_declaration_points_outside_entity_types(void) {
                            "ptrs_t *oe;\n} pico8_" "ke_t;\n"
                            "typedef struct {\n    pico8_" "ptrs_t layers[16];\n    pico8_" "ptrs_t *qy;\n} pico8_"
                            "emitter_t;\n"
-                           "typedef struct {\n    void *(*new_entity)(int k);\n    void (*kj)(void *self);\n} k_t;\n"
+                           "typedef struct {\n    void *(*new_entity)(int k);\n    void (*kj)(void *" "self);\n} k_t;\n"
                            "const pico8_" "ptrs_t *pico8_" "hv(void);\n"
                            "pico8_lq_t *pico8_" "jo_ex(const pico8_" "ptrs_t *is,\n    int o);\n"
                            "typedef int (*pico8_" "fn)(const pico8_" "ptrs_t *p);\n");
@@ -521,39 +523,64 @@ static void test_registres_sees_split_braces_unions_and_attributes(void) {
 
 static void test_void_ptr_reads_joined_public_prototypes(void) {
     setup();
-    write_file(SRC "/v.h", "pico8_qy_t *pico8_" "g__mm(void *self);\n"
+    write_file(SRC "/v.h", "pico8_qy_t *pico8_" "g__mm(void *" "self);\n"
                            "const void *pico8_" "ha_pz(void);\n"
                            "int pico8_" "two(int a,\n"
                            "    void *b);\n");
     assert(run_residue("--check void-ptr " TREE) == 1);
-    assert(strcmp(read_file(OUT), "void-ptr: " SRC "/v.h:1:pico8_qy_t *pico8_" "g__mm(void *self);\n"
+    assert(strcmp(read_file(OUT), "void-ptr: " SRC "/v.h:1:pico8_qy_t *pico8_" "g__mm(void *" "self);\n"
                                   "void-ptr: " SRC "/v.h:2:const void *pico8_" "ha_pz(void);\n"
                                   "void-ptr: " SRC "/v.h:3:int pico8_" "two(int a,\n"
                                   "résidu gardé (void-ptr) : 3 ligne(s)\n") == 0);
     setup();
-    write_file(SRC "/v.h", "void pico8_" "ei_dispatch(void *self, int x);\n"
-                           "int pico8_jo_get" "_fx(const void *self);\n"
+    write_file(SRC "/v.h", "void pico8_" "ei_dispatch(void *" "self, int x);\n"
+                           "int pico8_jo_get" "_fx(const void *" "self);\n"
                            "pico8_ptrs_t pico8_" "jp_ptrs(int a, void *ctx);\n"
                            "void pico8_" "gn(int a, void *r_ctx);\n"
                            "typedef int pico8_" "fn(void *e);\n"
-                           "typedef struct {\n    void (*kj)(void *self, int x);\n    void *(*new_entity)(void *k);\n} k_t;\n");
+                           "typedef struct {\n    void (*kj)(void *" "self, int x);\n    void *(*new_entity)(void *k);\n} k_t;\n");
     write_file(SRC "/pico8_ptrs.h", "void pico8_" "ptrs_add(pico8_ptrs_t *p, void *e);\n");
     write_file(SRC "/p.c", "void pico8_" "x(void *e);\n");
     write_file(SRC "/i.h", "static inline int f(int x) {\nreturn pico8_" "g((void *)0) + x;\n}\n");
     assert(run_residue(TREE) == 0);
     assert(strcmp(read_file(OUT), "accesseurs-q6\t1\t1\n"
+                                  "void-ptr\t1\t1\n"
+                                  "void-self\t3\t1\n"
                                   "registres\t1\t1\n"
-                                  "résidu 2 : 2 ligne(s), 1 fichier(s)\n") == 0);
+                                  "résidu 2 : 4 ligne(s), 1 fichier(s)\n") == 0);
+}
+
+/* Un receveur d'objet du jeu non type : membre, typedef, definition, locale,
+ * const autour de l'etoile, src/ comme tests/. Limite nommee : un void *
+ * d'un autre nom (e, ctx) n'est pas lu. */
+static void test_void_self_sees_every_untyped_receiver(void) {
+    setup();
+    write_file(SRC "/s.h", "typedef void (*pico8_" "state_fn)(void *" "self, int t);\n"
+                           "struct k {\n    int (*has_hb)(const void *" "self);\n};\n");
+    write_file(SRC "/s.c", "static void pico8_" "bg_kj_dispatch(void *" "self) {\n}\n"
+                           "void (*draw)(void*" "self) = 0;\n"
+                           "int pico8_" "ga_mm(void * const " "self_ptr);\n");
+    write_file(SRC "/n.c", "static int step(void *e, void *ctx);\n"
+                           "static void f(pico8_struct_" "header_t *self);\n");
+    write_file(TST "/t.c", "static void spy(void *" "self, int x) {\n}\n");
+    assert(run_residue("--check void-self " TREE) == 1);
+    assert(strcmp(read_file(OUT), "void-self: " SRC "/s.c:1:static void pico8_" "bg_kj_dispatch(void *" "self) {\n"
+                                  "void-self: " SRC "/s.c:3:void (*draw)(void*" "self) = 0;\n"
+                                  "void-self: " SRC "/s.c:4:int pico8_" "ga_mm(void * const " "self_ptr);\n"
+                                  "void-self: " SRC "/s.h:1:typedef void (*pico8_" "state_fn)(void *" "self, int t);\n"
+                                  "void-self: " SRC "/s.h:3:    int (*has_hb)(const void *" "self);\n"
+                                  "void-self: " TST "/t.c:1:static void spy(void *" "self, int x) {\n"
+                                  "résidu gardé (void-self) : 6 ligne(s)\n") == 0);
 }
 
 static void test_detail_prints_r2_sites_under_void_ptr_and_the_info_line(void) {
     setup();
-    write_file(R2, "pico8_" "f\tsrc/x.c\tamont(void *self)\tpreuve\n");
+    write_file(R2, "pico8_" "f\tsrc/x.c\tamont(void *" "self)\tpreuve\n");
     write_file(SRC "/pico8_game_data.h", "int a;\nint b;\n");
     write_file(SRC "/v.h", "void pico8_" "g(void *e);\n");
     assert(run_residue("--detail " TREE) == 0);
     assert(strcmp(read_file(OUT), "void-ptr\t" SRC "/v.h\t1\n"
-                                  "void-ptr-r2\tpico8_" "f\tamont(void *self)\n"
+                                  "void-ptr-r2\tpico8_" "f\tamont(void *" "self)\n"
                                   "info\t" SRC "/pico8_game_data.h\t2 lignes\n"
                                   "résidu 2 : 1 ligne(s), 1 fichier(s)\n") == 0);
 }
@@ -740,6 +767,7 @@ int main(void) {
     test_readers_see_qualifiers_split_declarations_and_nested_attributes();
     test_registres_sees_split_braces_unions_and_attributes();
     test_void_ptr_reads_joined_public_prototypes();
+    test_void_self_sees_every_untyped_receiver();
     test_detail_prints_r2_sites_under_void_ptr_and_the_info_line();
     test_attainment_list_counts_only_useful_lines();
     test_mutual_inclusion_counts_its_include_lines_in_the_footer();
diff --git a/tests/test_pico8_hy.c b/tests/test_pico8_hy.c
index 59d25b7..d4372fc 100644
--- a/tests/test_pico8_hy.c
+++ b/tests/test_pico8_hy.c
@@ -398,7 +398,7 @@ static void test_hk(void) {
     check("hk", h, HK_HASH);
 }
 
-static uint32_t lu_step(uint32_t h, void *self, int jt, int want, const pico8_struct_class_t *iv_cls) {
+static uint32_t lu_step(uint32_t h, pico8_hy_t *self, int jt, int want, const pico8_struct_class_t *iv_cls) {
     int from = pico8_hv()->count;
     int got = pico8_hy_lu(self, pico8_fixed_from_int(jt));
     assert(got == want);
diff --git a/tests/test_pico8_ui.c b/tests/test_pico8_ui.c
index 4da23e9..6f090d3 100644
--- a/tests/test_pico8_ui.c
+++ b/tests/test_pico8_ui.c
@@ -828,7 +828,7 @@ static uint32_t fold_ks_back(uint32_t h, void *e) {
 
 /* The component a host stores in itself: not shared with `other`, of the
  * same class, and inside the host's own `size` bytes. */
-static void assert_own_emitter(void *self, void *other, size_t size) {
+static void assert_own_emitter(pico8_struct_header_t *self, pico8_struct_header_t *other, size_t size) {
     pico8_struct_header_t *e = self;
     pico8_struct_header_t *o = other;
     const char *at = (const char *)e->emitter;
@@ -838,7 +838,7 @@ static void assert_own_emitter(void *self, void *other, size_t size) {
     assert(at >= (const char *)self && at < (const char *)self + size);
 }
 
-static void assert_own_probe(void *self, void *other, size_t size) {
+static void assert_own_probe(pico8_struct_header_t *self, pico8_struct_header_t *other, size_t size) {
     pico8_struct_header_t *e = self;
     pico8_struct_header_t *o = other;
     const char *at = (const char *)e->probe;
@@ -888,24 +888,24 @@ static void test_components_carried(void) {
         assert((header->probe != NULL) == probe[i]);
     }
 
-    assert_own_emitter(e[8], pico8_hj_new(NULL), sizeof(pico8_jo_t));
-    assert_own_emitter(e[9], pico8_pm_new(NULL), sizeof(pico8_jo_t));
-    assert_own_emitter(e[10], pico8_ga_new(NULL), sizeof(pico8_jo_t));
-    assert_own_emitter(e[13], pico8_mx_new(NULL), sizeof(pico8_jo_t));
-    assert_own_emitter(e[14], pico8_g__new(NULL), sizeof(pico8_g__t));
-    assert_own_emitter(e[15], pico8_fv_new(), sizeof(pico8_g__t));
-    assert_own_probe(e[5], pico8_qh_new(NULL), sizeof(pico8_jo_t));
-    assert_own_probe(e[6], pico8_nl_new(NULL), sizeof(pico8_jo_t));
-    assert_own_probe(e[7], pico8_os_new(NULL), sizeof(pico8_jo_t));
-    assert_own_probe(e[8], pico8_hj_new(NULL), sizeof(pico8_jo_t));
-    assert_own_probe(e[9], pico8_pm_new(NULL), sizeof(pico8_jo_t));
-    assert_own_probe(e[11], pico8_nr_new(NULL), sizeof(pico8_jo_t));
-    assert_own_probe(e[12], pico8_fr_new(NULL), sizeof(pico8_jo_t));
-    assert_own_probe(e[13], pico8_mx_new(NULL), sizeof(pico8_jo_t));
-    assert_own_probe(e[14], pico8_g__new(NULL), sizeof(pico8_g__t));
-    assert_own_probe(e[15], pico8_fv_new(), sizeof(pico8_g__t));
-    assert_own_probe(e[19], pico8_jc_new(NULL), sizeof(pico8_jc_t));
-    assert_own_probe(e[23], pico8_mz_new(), sizeof(pico8_mz_t));
+    assert_own_emitter(e[8], &pico8_hj_new(NULL)->header, sizeof(pico8_jo_t));
+    assert_own_emitter(e[9], &pico8_pm_new(NULL)->header, sizeof(pico8_jo_t));
+    assert_own_emitter(e[10], &pico8_ga_new(NULL)->header, sizeof(pico8_jo_t));
+    assert_own_emitter(e[13], &pico8_mx_new(NULL)->header, sizeof(pico8_jo_t));
+    assert_own_emitter(e[14], &pico8_g__new(NULL)->header, sizeof(pico8_g__t));
+    assert_own_emitter(e[15], &pico8_fv_new()->header, sizeof(pico8_g__t));
+    assert_own_probe(e[5], &pico8_qh_new(NULL)->header, sizeof(pico8_jo_t));
+    assert_own_probe(e[6], &pico8_nl_new(NULL)->header, sizeof(pico8_jo_t));
+    assert_own_probe(e[7], &pico8_os_new(NULL)->header, sizeof(pico8_jo_t));
+    assert_own_probe(e[8], &pico8_hj_new(NULL)->header, sizeof(pico8_jo_t));
+    assert_own_probe(e[9], &pico8_pm_new(NULL)->header, sizeof(pico8_jo_t));
+    assert_own_probe(e[11], &pico8_nr_new(NULL)->header, sizeof(pico8_jo_t));
+    assert_own_probe(e[12], &pico8_fr_new(NULL)->header, sizeof(pico8_jo_t));
+    assert_own_probe(e[13], &pico8_mx_new(NULL)->header, sizeof(pico8_jo_t));
+    assert_own_probe(e[14], &pico8_g__new(NULL)->header, sizeof(pico8_g__t));
+    assert_own_probe(e[15], &pico8_fv_new()->header, sizeof(pico8_g__t));
+    assert_own_probe(e[19], &pico8_jc_new(NULL)->header, sizeof(pico8_jc_t));
+    assert_own_probe(e[23], &pico8_mz_new()->header, sizeof(pico8_mz_t));
 }
 
 /* The kind words the frozen hashes fold before a value: any other value
diff --git a/tools/composition_r2_sites.txt b/tools/composition_r2_sites.txt
index 57de7df..0124d8b 100644
--- a/tools/composition_r2_sites.txt
+++ b/tools/composition_r2_sites.txt
@@ -1,11 +1,11 @@
 pico8_bm_has	src/pico8_entity.c	aucun : hv est typé (2.11r-b1)	pico8_he (entity.c:217) <- ui.c:458 (typee), bootstrap.c:72 ; pico8_lt (entity.c:188) <- pico8_ke_struct_pq (ke.c:40), typee
 pico8_dc	src/pico8_entity.c	aucun : cv est typé (2.11r-b2)	qc = bucket_src->items[i] ; entrees typees (const char *nt), appelants cart.c:208-209
-pico8_ke_ks	src/pico8_ke.c	pico8_d__dispatch(void *self) (probe.c:349), via pico8_d_	pico8_d_ (probe.c:334) <- probe.c:351 ; pico8_jo_hh <- pico8_mz_pr, typee ; pico8_jo_hu <- planet_display.c:325, typee
+pico8_ke_ks	src/pico8_ke.c	aucun : pico8_d__dispatch est typé (2.20c)	pico8_d_ <- pico8_d__dispatch ; pico8_jo_hh <- pico8_mz_pr, typee ; pico8_jo_hu <- pico8_jo_fe_my_dispatch, typee
 pico8_f_	src/pico8_particle.c	aucun	appelants : help.c:180, planet_family.c:332, :436, planet_family_trade.c:408, probe.c:502 (self local des *_struct_new) ; planet_family.c:448 dans pico8_ga_pq (ga_t *) <- pico8_ga_struct_new
-pico8_ez	src/pico8_particle.c	pico8_ei_dispatch(void *self) (particle.c:152), via pico8_ei	particle.c:149 dans pico8_ei ; planet_family.c:449 dans pico8_ga_pq, typee
-pico8_ei	src/pico8_particle.c	pico8_ei_dispatch(void *self) (particle.c:152)	particle.c:154 ; help.c:83 <- pico8_fv_kj_dispatch(void *self) (help.c:98) ; planet_family.c:246, :535 <- pico8_hj_kj, pico8_ga_kj <- dispatch (planet_family.c:253, :542)
-pico8_bv	src/pico8_probe.c	pico8_bv_dispatch(void *self) (probe.c:208)	probe.c:209 ; wormhole.c:61 dans pico8_mz_hq <- pico8_mz_hq_dispatch(void *self) (wormhole.c:77)
-pico8_dl	src/pico8_probe.c	pico8_dl_dispatch(void *self) (probe.c:275)	probe.c:277 ; slipway.c:337 dans pico8_jc_ev <- dispatch (slipway.c:352)
-pico8_d_	src/pico8_probe.c	pico8_d__dispatch(void *self) (probe.c:349)	probe.c:351 ; slipway.c:164 dans pico8_jc_ey <- dispatch (slipway.c:347) ; wormhole.c:124 dans pico8_mz_ey <- pico8_mz_ey_dispatch(void *self) (wormhole.c:131)
+pico8_ez	src/pico8_particle.c	aucun : pico8_ei_dispatch est typé (2.20a)	pico8_ei <- pico8_ei_dispatch ; pico8_ga_pq <- pico8_ga_struct_new, typee
+pico8_ei	src/pico8_particle.c	aucun : pico8_ei_dispatch est typé (2.20a)	pico8_ei_dispatch ; pico8_fv_kj, pico8_hj_kj, pico8_ga_kj <- leurs *_kj_dispatch, typés (2.20a)
+pico8_bv	src/pico8_probe.c	aucun : pico8_bv_dispatch est typé (2.20b)	pico8_bv_dispatch ; pico8_mz_hq <- pico8_mz_hq_dispatch, typé (2.20b)
+pico8_dl	src/pico8_probe.c	aucun : pico8_dl_dispatch est typé (2.20a)	pico8_dl_dispatch ; pico8_jc_ev <- pico8_jc_ev_dispatch, typé (2.20a)
+pico8_d_	src/pico8_probe.c	aucun : pico8_d__dispatch est typé (2.20c)	pico8_d__dispatch ; pico8_jc_ey <- pico8_jc_ey_dispatch, pico8_mz_ey <- pico8_mz_ey_dispatch, typés (2.20c)
 pico8_eg_gg_pred	src/pico8_score_panel.c	aucun : prédicat typé de pico8_jp_ents sur cg (2.11r-b3)	score_panel.c:125 : pico8_jp_ents(pico8_cg("jo"), pico8_eg_gg_pred, NULL)
-pico8_jc_hq	src/pico8_slipway.c	aucun : jo.oe est typee (2.8b)	self->probe->et = pico8_jc_ce(self) (slipway.c:147) ; self <- pico8_jc_hq_dispatch(void *self) (slipway.c:341)
+pico8_jc_hq	src/pico8_slipway.c	aucun : jo.oe est typee (2.8b)	self->probe->et = pico8_jc_ce(self) dans pico8_jc_hq <- pico8_jc_hq_dispatch, typé (2.20b)
diff --git a/tools/composition_residue.sh b/tools/composition_residue.sh
index 9f90b21..8a96836 100644
--- a/tools/composition_residue.sh
+++ b/tools/composition_residue.sh
@@ -43,10 +43,10 @@
 
 set -u
 
-CATEGORIES="parente lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux void-ptr ptrs-entité registres noyau-inclut atteinte-à-prouver inclusion-mutuelle usage-mutuel"
+CATEGORIES="parente lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux void-ptr void-self ptrs-entité registres noyau-inclut atteinte-à-prouver inclusion-mutuelle usage-mutuel"
 
 # Categories comptees dans le pied.
-SORTIE="parente lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux void-ptr ptrs-entité registres noyau-inclut atteinte-à-prouver inclusion-mutuelle usage-mutuel"
+SORTIE="parente lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux void-ptr void-self ptrs-entité registres noyau-inclut atteinte-à-prouver inclusion-mutuelle usage-mutuel"
 
 fail() {
     echo "composition_residue: $1" >&2
@@ -61,6 +61,7 @@ pattern() {
         nu) printf '%s\n' '\bnu\b|get_nu\b|_nu_(tags|items)\b' ;;
         aiguillage-nom) printf '%s\n' '\bhp_(pose|ho)\b|\bpico8_ho_gp_has\b|\bpico8_hp_(mr|gp)_names\b|(->|\.)(mr|gp)\b' ;;
         un-ou-faux) printf '%s\n' '\b(pk|fp)_is_bool\b' ;;
+        void-self) printf '%s\n' '\bvoid[[:space:]]*\*[[:space:]]*(const[[:space:]]+)?self' ;;
         noyau-inclut) printf '%s\n' '^#include "pico8_(game_data|map_tables|data_init)\.h"' ;;
         *) return 1 ;;
     esac
@@ -79,6 +80,11 @@ exclusion() {
     esac
 }
 
+# void-self : un objet du jeu recu en void * sous le nom self (self_ptr
+# compris), ou que la ligne le declare : membre, typedef, prototype,
+# definition, locale ; const lu autour de l'etoile. Un void * d'un autre nom
+# n'est pas lu.
+
 # Le void *ctx des parcours porte un contexte, pas une entite. Ce sed-ci veut
 # le sed GNU (\b), comme les grep du script.
 VOID_EXCLUSION='s/void \*+(r_)?ctx\b//g'
@@ -415,7 +421,7 @@ read_prototypes() {
         if (t ~ /^[ \t]*typedef/ || index(t, "void *") == 0) next
         if (!match(t, /(^|[^A-Za-z0-9_])pico8_[a-z0-9_]+\(/)) next
         name = substr(t, RSTART, RLENGTH - 1); sub(/^[^p]*/, "", name)
-        if (name ~ /_dispatch$/ || name ~ /_get_[a-z0-9_]+$/) next
+        if (name ~ /_get_[a-z0-9_]+$/) next
         print $1
     }' > "$work/void-ptr" || fail "echec de awk"
 }
```

## Runs

Sur la copie jetable (`prd220/copie`, arbres de travail `wt_p`, `wt_a` … `wt_d`, `wt_r`), dans l'ordre.

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local ; `git rev-parse HEAD:src HEAD:tests HEAD:tools` ; `ls-files -s Makefile` ; lien `third_party` | `p` | 0 | `a63929a4…`, `968e7267…`, `4b5abf24…`, `d4e26ba7…` | 2 s |
| 2 | greps de l'inventaire (`void *` des `.h` et des `.c`, rappels et leurs emplois, `jb_item.ref`, aides de TU) | `p` | 0 | 14 emplacements ; 19 publics + 37 `static` + `pico8_js_has_hb` ; 91 lignes / 32 fichiers de `src/`, 16 de TU ; rappels sur `jy` ; 214 aides / 34 TU | < 1 s |
| 3 | `syn.sh` ; `sed` des 14 emplacements (R6 de 2.18) ; `syn.sh` ; erreurs par fichier, par message, par membre | `p` | 0 | 0 → **231 erreurs, 41 fichiers** ; 160 init., 37 arguments, 33 comparaisons, 1 affectation | 2 s |
| 4 | `inv.py` (cibles par membre) | `p` | 0 | 13 membres + `states` ; 60 cibles (57 `src/`, 3 TU) | < 1 s |
| 5 | `run220.py` premier jet (membres et cibles seuls) a, b, c ; `syn.sh` | — | 0 | 8, 30, 44 erreurs restantes : appelants de TU, `ol` de `planet_display.c`, 6 cibles sans cast (`js`, `fake_toast`) | 2 s |
| 6 | `run220.py` avec les appelants ; `AssertionError` sur `test_pico8_pj.c` (4 occurrences, la 4e sur un `void *ju`) → champ « total » ; relancé a, b, c, `syn.sh` après chacun | — | 0 | 37, 58, 47 éditions ; **0 erreur** après a, a+b, a+b+c | 2 s |
| 7 | 2.20d à la main : catégorie, exemption, garde, coupe des 10 fixtures, `make composition-residue` ; `lu_step`, `assert_own_*` ; attendus et test neuf de `test_composition_residue.c` (3 attendus rouges l'un après l'autre) ; commentaire du script ; 7 lignes R2 ; `git diff` → `patch220d.diff` | — | 0 | pied `void-self 3 2` puis `0 / 0` ; `test_composition_residue: all tests passed` | ≈ 1 min |
| 8 | rejeu propre depuis `p` : a, b, c, `git apply` d, commits ; `proofs.sh` sur `wt_p` … `wt_d` | `p`…`d` | 0 | arbres de la table « sous-étapes » ; tout vert, 26 conformes, 26 propres, `couverture: OK`, raylib lié, pied `0 / 0`, `void self` 107 → 79 → 44 → 13 → 0 | ≈ 2 min |
| 9 | `neg220.c` `-DN=1..17` sur les cinq arbres | `p`…`d` | — | ligne « `neg220.c` » de la table « Comptes » | 3 s |
| 10 | garde à `d` : `lu_step` en `void *self` ; `pico8_x_dispatch(void *e)` ; remis ; M1, M2 et script du parent contre le TU de `d` | `d` | 2, 2, 0 ; rouge ×3 | `void-self: …hy.c:401` ; `void-ptr: …probe_comp.h:63` ; `:77`, `:546`, `:101` | 10 s |
| 11 | jumelles, premier passage en parallèle (deux sous-shells sur des arbres partagés), arrêté | `p`…`c` | — | faux écarts (objet muté périmé) | ≈ 2 min |
| 12 | `runmuts.sh` en série (14 jumelles × 2) ; A-draw et A-ei relancées seules des deux côtés | `p`…`c` | 0 | table de la preuve 5, aucun ÉCHEC | ≈ 5 min |
| 13 | appelants des six sites R2 et de leurs fonctions englobantes (`grep`, `head -n`) | `p` | 0 | les noms de fonction de la liste réécrite | < 1 s |
| 14 | `wt_r` neuf à `p` : `run220.py` a, b, c, `git apply`, `git write-tree` | `wt_r` | 0 | mêmes arbres qu'au Run 8 | < 1 s |
| 15 | `objs.sh` ×3 configurations (`-O0 -DNDEBUG`, `-O2 -DNDEBUG`, `-O0`) ×5 arbres ; `cmpobjs.sh` ; `fncmp.py` sur chaque objet différent | `p`…`d` | 0 | preuve 3 | 73 s |
| 16 | casts retirés / ajoutés, `&x->header`, fichiers par pas, longueur des lignes (`git diff`, `grep`) | `p`…`d` | 0 | 5 / 6 ; 55 ; une ligne neuve > 121 car. (le TU du compteur) | < 1 s |
| 17 | `rm -rf` de la copie, des arbres de travail et des fichiers d'essai | — | 0 | supprimés | < 1 s |
