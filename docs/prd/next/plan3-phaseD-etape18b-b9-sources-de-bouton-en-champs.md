---
title: "Plan 3, étape D.18b : genres mêlés B9 — les sources `lq`, `h_`, `jb` d'un bouton `js` deviennent des champs séparés de `pico8_js_keys_t` (`lq_copy`, `h_is_false`, présence `has_h_`/`has_jb`, rangées `*_row`) ; `pico8_js_pq` lit les clés typées ; TROU-23 réécrit dans `js:pq` ; dernière copie `truthy` et `pico8_js_is_lq` retirées ; aucune constante ne change"
description: "Les trois sources de rangée d'un bouton passent encore par la valeur Lua (`pico8_value_t lq, h_, jb`), triée par le constructeur de `js` qui y applique TROU-23 ; D.18b les range en champs typés que les producteurs écrivent déjà presque tous (`lq_row`, `h_row`, `jb_row`, `has_h_`, `has_jb`), ajoute `lq_copy` (la copie d'un `lq` de `jo:fe`) et `h_is_false` (le `false` de `jo:fe`, que `js:pq` sépare de nil), fait lire les clés par `pico8_js_pq`, supprime les cas de TU d'un genre que `src/` n'écrit plus (G4) sans bouger une constante : `valeur` baisse de 66 lignes (76 occ.), ≈ 560 → ≈ 494."
status: needs-triage
---

## Problem Statement

**Écrit pendant que D.18a est implémentée** (`plan-pilot`, étape 2b), sur l'arbre commis **`d183e10`** (HEAD de `claude/plan-pilot-3-7zdft0` au moment de l'écriture : D.17 gradée, PRD de D.18a en file), extrait par `git archive d183e10` dans le scratchpad. Seuls `grep`, `sed`, `awk` et `sh tools/data_residue.sh` ont tourné ; **rien n'a été compilé ni lancé**, rien de non commis n'a été lu. Tout ce que ce PRD suppose de l'issue de D.18a est une hypothèse **`H-D18a-n`** (section dédiée) ; la liste des sections à revérifier à la graduation de D.18a est en fin de Problem Statement. **Les numéros de ligne sont ceux de `d183e10`** ; l'implémenteur les remesure par contenu au parent de son commit (le commit de D.18a, ou un descendant `docs/` seul) et explique tout écart.

Mesuré à `d183e10` (départ de D.18a) : `valeur` 693 / 85 ; 763 occ., `type-tag` 242 / 58, résidu D 933 / 102. **Départ attendu de D.18b** (H-D18a-1) : `valeur` 560 / 83 ; 623 occ. (`src/` 181 / 45 / 212, `tests/` 379 / 38 / 411), `type-tag` 242 / 58, résidu D 800 / 101.

Le problème, vu du mainteneur du port : la Sortie de la phase D exige que `pico8_value.c/.h` disparaissent (D.19c). Le groupe B9 de l'étude D.14 (`docs/prd/done/plan3-phaseD-etape14-etude-genres-meles.md`) est celui des **sources d'un bouton** : `pico8_js_keys_t` garde trois membres `pico8_value_t lq, h_, jb` à côté des rangées typées `lq_row`, `h_row`, `jb_row` ; le constructeur de `js` recopie ces valeurs dans trois locales, y applique TROU-23 en transformant un nil présent en `true`, puis `pico8_js_pq` les trie (`pico8_js_truthy`, `pico8_js_is_lq` qui lit l'étiquette de type, deux `assert` de genre). Presque tous les producteurs écrivent déjà la rangée typée et posent la valeur à nil pour rien (`.lq = pico8_value_ref(NULL)`, dix-neuf lignes de `src/` et de TU).

Ligne de l'étape, recopiée de `docs/plan-3.md` à `d183e10` (§5, l. 290) :

> - D.18b — B9 : sources des boutons en champs séparés. `class-coverage-net` : oui. Compte : ≈ 505.

Ligne de l'étude D.14 (§3), recopiée :

> | **D.18b** | B9 : sources de bouton en champs séparés (`lq_copy`, `h_is_false`), TROU-23 réécrit, `pico8_js_pq` typé, `pico8_js_is_lq` retiré ; TU `js`, `lq` | ≈ 10 fichiers | ≈ 505 |

Point B9 de l'étude (§1.3), recopié :

> **B9. Sources d'un bouton (`js` : `lq`, `h_`, `jb`) — 38 lignes (+ 12 de `tests/`).** `ui_button.h:73,76,79,107-108`, `ui_button.c:119-166` (constructeur, TROU-23), `:218-252` (`pico8_js_lq`, `pico8_js_pq`), écritures `.lq/.h_/.jb = pico8_value_ref(NULL)` de `menu.c` (7), `help.c:58`, `msgbox.c:356`, `planet_display.c:184,190,303,370-395`, `score_panel_display.c:278,293`.
> - Genres, preuve par les écritures de `src/` : `lq` vaut une rangée (`lq_row`), un `lq` existant (`pico8_hp_lq(b)`, `planet_display.c:372`) ou nil ; `h_` vaut une rangée, `false` (`h_=od>0 and {…}`, lua.lua:419 ; `planet_display.c:376`) ou nil — nil présent devient `true` (TROU-23) ; `jb` vaut une rangée ou nil (nil présent → `true`, TROU-23 ; `false` s'arrête sur un `assert`).
> - Lecteurs : `js:pq` (lua.lua:261-262) distingue `false` (pas de `h_`, `has_h_=1`) de nil (pas de `h_`, `has_h_=0`) et de `true` (rangée vide).
> - Règle (G) : **champs séparés** : `const pico8_lq_t *lq_copy` (NULL : aucun) à côté de `lq_row` ; `has_h_` + `h_is_false` à côté de `h_row` ; `has_jb` à côté de `jb_row`. TROU-23 s'écrit « présent, sans rangée, pas faux → rangée vide ». `pico8_js_pq` prend ces champs ; `pico8_js_is_lq` (qui lisait l'étiquette de type) disparaît. Raison : les quatre états sont déjà des drapeaux au site d'écriture ; une union ne ferait que les réemballer.

Lignes directrices de l'étude (§4) qui s'appliquent, en extrait :

> **G1.** Aucune constante figée ne change dans D.15–D.20. […] Une empreinte qui bouge est une régression, pas une mise à jour.
> **G2.** nil et `false` confondus seulement là où aucun lecteur du cart ne les sépare […]. Là où un lecteur les sépare, ils restent deux états : B2 (`qe`, TROU-24), B9 (`h_`, TROU-23).
> **G3.** TROU-13, TROU-23, TROU-24 et TROU-25 restent à l'identique ; leur texte est réécrit quand il nomme `pico8_value_*`.
> **G4.** Un cas de TU qui passe un genre qu'aucun appelant de `src/` n'écrit […] est supprimé avec le type qui le permettait ; le message de commit le nomme.

Règles communes de l'étude (§3) : un TU qui nomme un symbole changé le suit dans le même commit ; aucune empreinte figée ne change ; aucun `*.sha256` ni e2e ne change ; `class-coverage-net` si l'étape touche un constructeur (drapeau de l'étude pour D.18b : « constructeur `js` »).

Note de D.15c (PRD `docs/prd/done/plan3-phaseD-etape15c-tostr-nombre-dans-str.md`, l. 83, 132, 327, 484) : « `planet_display.c:377` garde `pico8_value_string` autour du formatage (`h_item` d'un bouton, B9, D.18b) ». **Relu ici** : cette ligne (`row->h_item = pico8_value_string(jo_dollar(pico8_tostr_fixed(od)));`, `planet_display.c:357` à `d183e10`) écrit la **case** de la rangée `h_` de `jo:fe`, pas la source du bouton : c'est une clé de `lq` (B8), et le PRD de D.18a la prend (son Inventaire, `planet_display.c:357`, (a1), `pico8_lq_cell_string`), avec sa jumelle `:175` (`h_item` de `jo:gh`). D.18b n'a rien à y faire (H-D18a-5) ; si D.18a la laissait, D.18b la prend, (a1), et le déclare.

### Hypothèses `H-D18a-n` (D.18a en cours ; à juger à sa graduation)

Source : le PRD de D.18a (`docs/prd/triage/plan3-phaseD-etape18a-b8-cles-de-rangee-en-cases.md` à `d183e10`), son Inventaire, ses « Pour D.18b » et ses totaux.

| Hypothèse | Énoncé | Si elle tombe |
|---|---|---|
| H-D18a-1 | Après D.18a : `valeur` **560 / 83 ; 623 occ.** (`src/` 181 / 45 / 212, `tests/` 379 / 38 / 411) ; `type-tag` 242 / 58 ; résidu D 800 / 101 | départ, « Effet », critères `FAILED:` recalculés (les deltas de D.18b restent − 66 / − 76) |
| H-D18a-2 | D.18a ne touche ni `pico8_ui_button.c` ni `pico8_ui_button.h` (sa liste « Inchangés ») | Inventaire de `ui_button` remesuré ligne à ligne |
| H-D18a-3 | Dans `menu.c`, `help.c`, `msgbox.c`, `planet_display.c`, D.18a change ses lignes **en place** : les lignes B9 gardent contenu et numéro (`menu.c:106,117,121,196,199,364,387`, `help.c:58`, `msgbox.c:356`, `planet_display.c:165,171,282,349,351,355,363,374`) ; dans `score_panel_display.c`, le retrait de `pico8_pj_ph_item_value` et de son commentaire (`:126-141`) fait remonter `:279`, `:294` d'environ 16 lignes | numéros remesurés par contenu |
| H-D18a-4 | Comptes par fichier après D.18a (lignes / occ.) : `ui_button.c` 16 / 21, `ui_button.h` 6 / 7, `menu.c` 8 / 8 (7 B9 + include `:24`), `help.c` 1 / 1, `msgbox.c` 1 / 1, `planet_display.c` 8 / 8, `score_panel_display.c` 13 / 13, `test_pico8_js` 39 / 44, `test_pico8_lq` 30 / 30 | tableau « Effet » recalculé |
| H-D18a-5 | `planet_display.c:175` et `:357` (cases `h_` de `jo:gh` et `jo:fe`, la « ligne pour D.18b » de D.15c) sont converties par D.18a | D.18b les prend, (a1), déclarées |
| H-D18a-6 | Copies `truthy` : **1** (`ui_button.c:27`) ; fonctions publiques qui nomment `pico8_value_t` : **2** (`pico8_pj_item_of`, `pico8_js_pq`) | Sortie grep 3 et « Effet » |
| H-D18a-7 | `pico8_lq_copy`, `pico8_lq_new(NULL)` (rangée vide), `pico8_jb_new(NULL)` (`jb` vide) inchangés ; D.18b n'emploie aucun constructeur `pico8_lq_cell_*` | lecture de `pico8_js_lq` revue |
| H-D18a-8 | Dans `test_pico8_js`, D.18a ne change que `build()` (`:780-781` : `h_item` écrit **dans** le `if (h_text != NULL)`, une ligne de plus : les lignes après `:789` descendent de 1) et `fold_keys` (`:1319`, `:1388-1389`, en place) ; aucune ligne B9 touchée | numéros de `js` remesurés |
| H-D18a-9 | Dans `test_pico8_lq`, D.18a change des lignes avant `:524` (plis `fold_cell`/`fold_positionals`, masque `nil_at`, `same_cell`) : `:524` et `:893` gardent leur contenu mais descendent d'un nombre inconnu ; `must_abort` garde deux appelants (`keys_of_row`, retypé en cases, et `ol_on_other`) | numéros remesurés ; si `must_abort` perdait ses appelants, voir « `-Werror` » |
| H-D18a-10 | `Makefile`, `tools/`, `DATA_RESIDUE_GUARD`, `tools/class_coverage_net_expected.txt`, les 26 e2e, `docs/trous-de-portage.md` inchangés par D.18a ; le glossaire change ses lignes `:434-435` en place (l'entrée `js` reste `:444`) | chemins et numéros de la « Documentation » revus |
| H-D18a-11 | D.18a en un commit, sans repli (sinon D.18a1/D.18a2, et D.18b part de D.18a2) ; durée réelle mesurée pour recalibrer le budget | « Budget » recalibré |
| H-D18a-12 | La ligne D.18b du plan est inchangée ; la note « Découvert à D.18a » dit « pour D.18b » ce que liste l'Inventaire ci-dessous (`menu.c` 7, `help.c` 1, `msgbox.c` 1, `planet_display.c` 8, `score_panel_display.c` 2, `ui_button.c/.h`, dernière copie `truthy` `ui_button`) | Inventaire relu contre la note |

### Sections à revérifier quand D.18a sera gradée

1. « État de départ » et les critères `FAILED:` de départ (H-D18a-1, -4).
2. Numéros de ligne de l'Inventaire : `score_panel_display.c` (H-D18a-3), `test_pico8_js.c` (H-D18a-8), `test_pico8_lq.c` (H-D18a-9) ; les autres par `git diff --quiet <D.18a> d183e10 -- <fichier>` ou remesure par contenu.
3. « Effet sur `make data-residue` » (départ réel moins − 66 / − 76 occ.).
4. Tableau « Runs », lignes 1 à 3 (valeurs attendues).
5. Cibles des mutations (numéros).
6. « Documentation » : numéro de l'entrée `js` du glossaire ; texte de TROU-23 (H-D18a-10).
7. « Budget » : recalibré sur le temps réel de D.18a (H-D18a-11).
8. La note « Découvert à D.18a » : legs pour D.18b (H-D18a-12), et tout écart qu'elle signale (ligne prise, `pico8_lq_cell_*` employé par un site B9).

### Preuve B9, par le cart

`js:pq` (lua.lua:261-262), recopiée :

```lua
function js:pq()self.lq=lq(mv(self,"lq"))self.mb=self.lq.w+2
self.kq=qz(0,-1,self.mb,11)self.h_=self.h_ and lq(self.h_)self.jb=self.jb and jb(self.jb)end
```

- `lq` : toujours construit (`lq(nil)` donne une rangée vide) ; `mv` rendrait l'appel d'une fonction, mais aucune construction de bouton ne passe `lq` en fonction (le seul `self.lq=` fonctionnel est l'écriture du **membre** par `ix[2].hb`, lua.lua:982, `menu.c:291`).
- `h_` : `false` et nil ne construisent rien ; tout autre vrai construit `lq(h_)`. Le cart n'a donc que deux issues, mais **le port en a trois** (TROU-23, plus bas) et **le membre les distingue** : `has_h_ = 1, h_ = NULL` est `false` (`js:kj` : `local c=self.h_ if c then …`, lecture en vérité seule dans le cart ; le port garde `false` à part parce que `JS_MEMBERS_HASH` et `h__state` le plient).
- `jb` : même règle, `jb(x)`.

Écritures du cart (grep `h_=\|jb=\|lq=`, lua.lua) : `jo:fe` (l. 419-421) `{lq=mv(b,"lq"), h_=od>0 and {od.."$",c=13}, …, jb=b:jb()}` — `lq` une table (le `lq` donnée d'une technologie) ou le résultat de `hp:lq()` ; `h_` **faux** ou une rangée ; `jb` nil (`hp:jb` quand `ii`) ou une rangée ; `jo:gh` (l. 424-445) `lq=` rangées, `bs[1].h_=self.kr and {…}` (nil ou une rangée) ; `pz` (l. 281) `{lq={p.ep,c=13}}` ; `cr` (l. 957-961) `lq={…}`, `h_={{24,5},tostr(gi(t))}`, `jb=t.oy` (nil ou une rangée), `jb=fm.invented` ; `ix`, `ha`, `bo`, `pj` (l. 874-990, 1190-1196) `lq=o(…)`.

### Preuve B9, par le port : écritures et lecteurs

Grep complet à `d183e10` : `grep -rnE '\b(lq_row|h_row|jb_row|has_h_|has_jb)\b|pico8_js_pq|pico8_js_new\(|pico8_js_keys_t|\.(lq|h_|jb) = ' src tests`.

| # | Site (`d183e10`) | `lq` | `h_` | `jb` | Genres écrits |
|---|---|---|---|---|---|
| S1 | `menu.c:94-128` `pico8_cr_step` (lignes de `cr`, `rows` en `calloc`) | `ref(NULL)` `:106` + `lq_row` | `has_h_ = 1`, `ref(NULL)` `:117` + `h_row` | `has_jb = 1`, `ref(NULL)` `:121` + `jb_row = t->oy` (NULL possible) | rangée ; `h_` rangée ; `jb` rangée ou nil présent (TROU-23) |
| S2 | `menu.c:185-200` rangée « n invented » | `ref(NULL)` `:196` + `lq_row` | — | `has_jb = 1`, `ref(NULL)` `:199` + `jb_row` | rangées |
| S3 | `menu.c:342-373` `ix4_my` (`ha`, `memset`) | `ref(NULL)` `:364` + `lq_row` | — | — | rangée |
| S4 | `menu.c:378-394` `pico8_ix_button` (`= {0}`) | `ref(NULL)` `:387` + `lq_row` | — | — | rangée |
| S5 | `help.c:43-66` `pico8_fv_pq` (`bo`, `memset`) | `ref(NULL)` `:58` + `lq_row` | — | — | rangée |
| S6 | `msgbox.c:333-358` `pico8_pz` (`ep_row = {0}`) | `ref(NULL)` `:356` + `lq_row` | — | — | rangée |
| S7 | `planet_display.c:131-287` `pico8_jo_gh` (`memset(bs)`) | `ref(NULL)` `:165` (+ `lq_row` si icône) ; `ref(NULL)` `:282` + `lq_row` | `has_h_ = 1`, `ref(NULL)` `:171` (+ `h_row` si `kr`) | — | rangée ou nil ; `h_` rangée ou **nil présent** (TROU-23) |
| S8 | `planet_display.c:342-376` `pico8_jo_fe_keys` (`memset(keys)`) | `ref(NULL)` `:349` + `lq_row` ; **`ref(pico8_hp_lq(b))` `:351`** (copie d'un `lq`, NULL si `hp` absent) | `has_h_ = 1`, **`bool(0)` `:355`** ; si `od > 0` : `ref(NULL)` `:363` + `h_row` | `has_jb = 1`, `ref(NULL)` `:374` + `jb_row = pico8_hp_jb(b)` (NULL possible) | `lq` rangée ou **copie** ; `h_` **faux** ou rangée ; `jb` rangée ou nil présent (TROU-23, le cas atteint par le filet) |
| S9 | `score_panel_display.c:256-300` `pico8_pj_ph` (`spec1`, `spec2 = {0}`) | `ref(NULL)` `:279`, `:294` + `lq_row` | — | — | rangées |
| S10 | `msgbox.c:310-331` `pico8_pz_step` | recopie `b = *row` | | | — |
| S11 | TU `ui`, `spec_defaults`, `js` (`fresh[0]`, `fresh[2]`) | `pico8_js_new(NULL)` | | | aucune clé (aucun site de `src/` n'appelle `pico8_js_new(NULL)`) |

**Genres réellement écrits dans `src/`** : `lq` — rangée par `lq_row`, copie d'un `lq` (S8 `:351` seul), nil ; `h_` — rangée par `h_row`, **faux** (S8 seul), nil présent (S7 sans `kr`, TROU-23), absent ; `jb` — rangée par `jb_row`, nil présent (S1 si `t->oy` NULL, S8 si `hp:jb` nil : TROU-23), absent. **Aucun site n'écrit** : un `h_` copie d'un `lq`, un `jb` faux, un `jb` référence, un `lq`/`h_` nombre ou chaîne, une référence qui n'est ni `lq` ni rangée. Toutes les structs de clés sont mises à zéro avant écriture (`calloc`, `memset`, `= {0}`) : **retirer une écriture `= pico8_value_ref(NULL)` laisse le champ à NULL/0**, c'est-à-dire nil.

**Lecteurs** (tous dans `ui_button.c`) : le constructeur `pico8_js_struct_new` (`:116-192` : trois locales `:118-120`, recopie `:153-159`, TROU-23 `:160-166`, appel `:188-189`) ; `pico8_js_lq` (`:215-226`, `assert(pico8_js_is_lq…)` `:224`) ; `pico8_js_pq` (`:228-254`) ; `pico8_js_truthy` (`:27-29`, dernière copie `truthy`) ; `pico8_js_is_lq` (`:18-21`, la seule ligne `type-tag` du fichier, `:19`). `pico8_pz_step` recopie la struct sans la lire. **Aucun TU n'appelle `pico8_js_pq`.**

**Table de vérité d'aujourd'hui** (après TROU-23 au constructeur), et celle qu'écrit D.18b :

| Source | Aujourd'hui | Après D.18b |
|---|---|---|
| `lq` | `lq_row` → `pico8_lq_new(lq_row)` ; sinon `REF` non NULL → `assert(is_lq)`, `pico8_lq_copy` ; sinon (nil, nombre, `{0}` = `FIXED` 0) → rangée vide | `lq_row` → `pico8_lq_new(lq_row)` ; sinon `lq_copy` → `pico8_lq_copy(lq_copy)` ; sinon rangée vide |
| `h_` | `h_row` → rangée ; sinon valeur vraie (`true` de TROU-23, nombre, chaîne → rangée vide ; `REF` → copie) → rangée ; sinon `BOOL` faux → `has_h_ = 1, h_ = NULL` ; sinon rien | `h_row` → rangée ; sinon `has_h_ && !h_is_false` → rangée vide (TROU-23) ; sinon `has_h_` → `has_h_ = 1, h_ = NULL` (faux) ; sinon rien |
| `jb` | `jb_row` → `pico8_jb_new(jb_row)` ; sinon vrai non `REF` (`true` de TROU-23, nombre, chaîne) → `jb` vide ; `REF` → `assert` ; `BOOL` faux → `assert` ; sinon rien | `jb_row` → `pico8_jb_new(jb_row)` ; sinon `has_jb` → `jb` vide (TROU-23) ; sinon rien |

Sur tous les genres que `src/` écrit, les deux colonnes rendent le même membre. Les lignes qui divergent ne sont atteintes que par des TU (G4, « Les TU suivent ») : `h_` copie d'un `lq` (`test_pico8_js:1383`), références étrangères et `jb` faux (`test_js_new_stops_off_row`, `test_pico8_lq` `button_on_other`).

### Lecture de la ligne pour D.18b

- **« Sources des boutons en champs séparés »** (G, règle de l'étude) : dans `pico8_js_keys_t`, `pico8_value_t lq` devient **`const pico8_lq_t *lq_copy`** (NULL : aucune copie) ; `pico8_value_t h_` devient **`int h_is_false`** (1 : la clé `h_` vaut `false` ; lue seulement avec `has_h_`, sans `h_row`) ; `pico8_value_t jb` **disparaît** (`has_jb` porte la présence, `jb_row` la rangée). `lq_row`, `has_h_`, `h_row`, `has_jb`, `jb_row` restent. Le nom `lq_copy` suit l'étude et le patron `lq_row` : il dit ce que `js:pq` en fait ; le changement de nom fait échouer à la compilation tout site oublié (`.lq =`), ce n'est pas un renommage d'identifiant vivant pour échapper au motif (le motif `valeur` ne lit pas les noms de champs).
- **`pico8_js_pq` typé** (G) : **`void pico8_js_pq(pico8_js_t *self, const pico8_js_keys_t *keys)`** (NULL : aucune clé). `js:pq` du cart lit les clés de construction recopiées sur `self` ; lire la struct de clés est la forme la plus proche, et le seul appelant (le constructeur) la tient déjà. **Rejeté** : la forme à huit paramètres (`lq_copy`, `lq_row`, `has_h_`, `h_is_false`, `h_row`, `has_jb`, `jb_row`), qui obligerait le constructeur à écrire six gardes `keys != NULL ? … : …` pour un appelant unique ; garder des locales dans le constructeur (elles ne porteraient plus rien, TROU-23 s'écrivant sur les drapeaux).
- **TROU-23 réécrit** (G3) : le constructeur ne remplace plus nil par `true` ; `pico8_js_pq` construit une rangée vide quand `has_h_` est posé sans `h_row` ni `h_is_false`, un `jb` vide quand `has_jb` est posé sans `jb_row`. Un seul marqueur nu `/* TROU-23 */` dans `ui_button.c`, au-dessus de ces deux tests (il remplace celui du constructeur, `:160`). Forme de décision, pas code à recopier :

  ```c
  /* `lq(x)` of the cart for `js.lq`: built from a row's keys, else a copy of an `lq`, else empty. */
  static pico8_lq_t *pico8_js_lq(const pico8_lq_t *copy, const pico8_lq_keys_t *row);

  void pico8_js_pq(pico8_js_t *self, const pico8_js_keys_t *keys) {
      /* keys NULL: every source absent */
      self->lq = pico8_js_lq(keys->lq_copy, keys->lq_row);
      …
      /* TROU-23 */
      if (keys->h_row != NULL || (keys->has_h_ && !keys->h_is_false)) { self->h_ = pico8_lq_new(keys->h_row); self->has_h_ = 1; }
      else if (keys->has_h_) { self->h_ = NULL; self->has_h_ = 1; }
      if (keys->jb_row != NULL || keys->has_jb) { self->jb = pico8_jb_new(keys->jb_row); }
  }
  ```

  `pico8_lq_new(NULL)` et `pico8_jb_new(NULL)` rendent déjà une rangée et un `jb` vides (H-D18a-7) : c'est ce que faisaient `pico8_js_lq(true, NULL)` et `pico8_jb_new(NULL)`. `pico8_js_lq` ne sert plus que `js.lq` (le `h_` n'a pas de copie).
- **`jo:fe`, le faux** (G) : `planet_display.c:355` `keys->h_ = pico8_value_bool(0);` devient **`keys->h_is_false = od <= pico8_fixed_from_int(0);`** (l'écriture du cart, `h_=od>0 and {…}`) et `:363` (`keys->h_ = pico8_value_ref(NULL);` sous `if (od > 0)`) part : une clé n'est jamais à la fois fausse et rangée. **Rejeté** : garder `h_is_false = 1` puis le remettre à 0 sous `if` (deux écritures du même champ ; la rangée l'emporterait de toute façon dans `pq`, mais les clés se contrediraient).
- **`jo:fe`, la copie** : `:351` `keys->lq = pico8_value_ref(pico8_hp_lq(b));` devient `keys->lq_copy = pico8_hp_lq(b);` (`pico8_lq_t *` vers `const pico8_lq_t *`, sans cast ; NULL si `hp:lq` ne rend rien : rangée vide, comme aujourd'hui).
- **Écritures de nil** : les seize lignes `.lq/.h_/.jb = pico8_value_ref(NULL)` de `src/` (S1–S9) **partent sans remplaçant** : la struct est à zéro (preuve par site ci-dessus).
- **`pico8_js_is_lq`** part (sans appel, `-Werror`) : le type de `lq_copy` dit ce qu'il pointe ; la vérification d'étiquette tombe (Q13 = A : « là où le pointeur est déjà typé, l'`assert` disparaît »). Effet : **`type-tag` − 1** (`ui_button.c:19`), `ui_button.c` sort de `type-tag`.
- **`pico8_js_truthy`** part (sans appel) : **copies `truthy` 1 → 0**.
- **Les trois `assert` de `pico8_js_lq`/`pico8_js_pq`** (`:224`, `:249`, `:252`) partent avec les genres qu'ils refusaient : aucun champ ne peut plus porter une référence étrangère, un `jb` référence ou un `jb` faux. `<assert.h>` de `ui_button.c`, devenu sans usage, part (guideline : un orphelin créé par le diff est retiré).
- **Include de `menu.c:24`** : après D.18a et D.18b, `menu.c` ne nomme plus rien du module ; règle de D.15b (un `.c` qui ne nomme plus rien du module perd son include) : **retiré** ((a2), compté). `help.c`, `msgbox.c`, `planet_display.c`, `ui_button.c` n'ont pas d'include propre (ils l'ont par leur en-tête). **En-têtes gardés** pour D.19c : `ui_button.h:13` (vidé : il ne garde que son include), `help.h`, `msgbox.h`, `planet_display.h`.
- **`class-coverage-net` : oui** (ligne ; le diff touche le constructeur `pico8_js_struct_new`) : sortie identique à l'attendu.
- **`sanitize-net`** : non demandé ; aucune durée de vie ni propriétaire ne change (les rangées de clés sont toujours empruntées pendant la construction, `pico8_lq_copy` alloue comme avant, TROU-7) ; **le balayage ASan+UBSan des TU est exigé** (consigne de l'utilisateur).
- **« Compte : ≈ 505 »** : mesuré ici **− 66 lignes / − 76 occ.** depuis ≈ 560 : **≈ 494**. L'écart à 505 : l'étude comptait B9 à 38 + 12 lignes sur `b7f6c02` ; s'y ajoutent la copie `truthy` de `ui_button` (2 lignes, rangée B12 par l'étude), l'include de `menu.c`, et le test d'arrêt `test_js_new_stops_off_row` supprimé (G4) avec ses deux aides (13 lignes). « ≈ » n'est pas un critère ; les deltas exacts le sont.
- **Découpage** : **(G) un commit** (`refactor(D.18b)`). Le type des clés change d'un coup pour tous les producteurs et le seul lecteur ; 66 lignes presque toutes des retraits. Repli écrit plus bas.
- **Consignes de l'utilisateur, liantes** : un TU ne rejoue jamais un `events.json` ; rien de lent ; aucun `*.sha256` ni fichier de `tests/e2e/` touché ; jamais l'appli raylib ; interdits : jonction de lignes comptées (sauf contrefactuel déclaré, plus bas), alias, renommage pour échapper aux greps, cast nouveau, cale valeur ↔ typé ou pont, aide de TU laissée sans appel (`-Werror`) ; mutations restaurées par `cp` simple puis `touch` ; binaires lancés exactement comme le `Makefile`.

### État de départ

| Mesure | À `d183e10` (D.18a non commencée) | Départ de D.18b attendu (D.18a gradée, H-D18a-1) |
|---|---|---|
| `valeur` (lignes / fichiers ; occ.) | 693 / 85 ; 763 occ. | **560 / 83 ; 623 occ.** (`src/` 181 / 45 / 212, `tests/` 379 / 38 / 411) |
| `type-tag` | 242 / 58 | **242 / 58** |
| résidu D | 933 / 102 | **800 / 101** |
| fonctions publiques qui nomment `pico8_value_t` | 4 | **2** (`pico8_pj_item_of`, `pico8_js_pq`) |
| copies `*_truthy(pico8_value_t)` | 2 | **1** (`ui_button.c:27`) |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit` | inchangé |

Recette des occurrences (celle de D.12–D.18a, motif `valeur` de `tools/data_residue.sh:29`) :

```sh
P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
printf 'valeur\t%s lignes\t%s fichiers\t%s occ.\n' "$(cat $F | grep -cE "$P")" \
  "$(grep -lE "$P" $F | wc -l)" "$(cat $F | grep -oE "$P" | wc -l)"
for f in $F; do n=$(grep -cE "$P" $f); [ $n -gt 0 ] && \
  printf '%s\t%s\t%s\n' $f $n "$(grep -oE "$P" $f | wc -l)"; done
```

### Inventaire

Classes du message de commit : **(a1)** même site, type C à la place de la valeur ; **(a2)** retirée sans remplaçant au site (écriture de nil dans une struct à zéro, fonction ou aide sans appel, cas de TU d'un genre que `src/` n'écrit pas (G4), include devenu inutile) ; **(b)** signature ou membre public dont le type change ou qui disparaît ; **(c)** même donnée, autre nom : **attendu 0**. `occ.` : occurrences du motif. Chaque ligne listée **sort** de `valeur` ; aucune n'en gagne. Numéros de `d183e10`.

#### API (`ui_button.h`, `ui_button.c`)

| Symbole | Aujourd'hui | Après D.18b | Classe |
|---|---|---|---|
| `pico8_js_keys_t.lq` (`.h:73`) | `pico8_value_t lq;` | **`const pico8_lq_t *lq_copy;`** | (b) |
| `pico8_js_keys_t.h_` (`.h:76`) | `pico8_value_t h_;` | **`int h_is_false;`** | (b) |
| `pico8_js_keys_t.jb` (`.h:79`) | `pico8_value_t jb;` | **retiré** | (b) |
| `pico8_js_pq` (`.h:107-108`, `.c:228-229`) | `(self, pico8_value_t lq, …lq_row, pico8_value_t h_, …h_row, pico8_value_t jb, …jb_row)` | **`(pico8_js_t *self, const pico8_js_keys_t *keys)`** | (b) ×4 |
| contrat des clés (`.h:65-69`) et de `pq` (`.h:104-106`), commentaires de `has_h_`/`has_jb` (`.h:75`, `:78`) | « sinon un `lq` row, true, false or nil » | réécrits : `lq_row` sinon `lq_copy` sinon rangée vide ; `has_h_` + `h_row` ou `h_is_false`, présent sans l'un ni l'autre : rangée vide (TROU-23) ; `has_jb` + `jb_row`, présent sans rangée : `jb` vide (TROU-23) ; `pq` : « `keys` read, not kept; NULL: none » | — |
| `pico8_js_truthy`, `pico8_js_is_lq` (`static`) | — | **retirées** (sans appel, `-Werror`) | (a2) |
| `pico8_js_lq` (`static`) | `(pico8_value_t v, const pico8_lq_keys_t *row)` | `(const pico8_lq_t *copy, const pico8_lq_keys_t *row)` | (a1) |

#### Sites de `src/` (41 lignes, 47 occ. retirées ; aucune gagnée)

| Fichier | Lignes `valeur` (occ.) | Remplacement | Classe |
|---|---|---|---|
| `ui_button.c` | `:27`, `:28` (1, 2) | `pico8_js_truthy` retirée entière | (a2) 2 |
| | `:118`, `:119`, `:120` (2 chacune) | locales `lq_arg`, `h__arg`, `jb_arg` retirées ; l'appel `:188-189` (0) devient `pico8_js_pq(self, keys);` | (a2) 3 |
| | `:161`, `:162`, `:164`, `:165` (1 chacune) | TROU-23 sur les locales retiré ; réécrit dans `pq` sur les drapeaux (lignes neuves, hors motif) ; marqueur `:160` déplacé | (a2) 4 |
| | `:217` (1) | `static pico8_lq_t *pico8_js_lq(const pico8_lq_t *copy, const pico8_lq_keys_t *row) {` ; contrat `:215-216` réécrit | (a1) 1 |
| | `:221` (1) | `if (copy == NULL) {` ; `:224` (`assert`, 0) part ; `:225` (0) `return pico8_lq_copy(copy);` (cast retiré) | (a1) 1 |
| | `:228`, `:229` (2, 1) | `void pico8_js_pq(pico8_js_t *self, const pico8_js_keys_t *keys) {` — une ligne, voir « Jonctions » | (b) 2 |
| | `:234` (0), `:238-240` (0) | `pico8_js_lq(keys->lq_copy, keys->lq_row)` ; condition `h_` de la table de vérité | — |
| | `:241` (1) | `} else if (keys->has_h_) {` | (a1) 1 |
| | `:249`, `:251` (1 chacune) | `assert` du `jb` référence et branche du `jb` faux retirés ; bloc `jb` réécrit en deux lignes (`jb_row || has_jb`) | (a2) 2 |
| | `:18-21` (0 ; `type-tag` `:19`), `:3` `<assert.h>` (0) | retirés | — |
| `ui_button.h` | `:73`, `:76`, `:79` (1 chacune), `:107`, `:108` (2, 1) | voir API ; include `:13` **gardé** (en-tête, D.19c) | (b) 5 |
| `menu.c` | `:24` (1) | `#include "pico8_value.h"` retiré (règle de D.15b) | (a2) 1 |
| | `:106`, `:117`, `:121`, `:196`, `:199`, `:364`, `:387` (1 chacune) | écritures de nil retirées (`calloc`, `memset`, `= {0}`) | (a2) 7 |
| `help.c` | `:58` (1) | retirée (`memset`) | (a2) 1 |
| `msgbox.c` | `:356` (1) | retirée (`= {0}`) | (a2) 1 |
| `planet_display.c` | `:165`, `:171`, `:282`, `:349` (1 chacune) | retirées (`memset`) ; `:170` `has_h_ = 1` reste (TROU-23 sans `kr`) | (a2) 4 |
| | `:351` (1) | `keys->lq_copy = pico8_hp_lq(b);` | (a1) 1 |
| | `:355` (1) | `keys->h_is_false = od <= pico8_fixed_from_int(0);` | (a1) 1 |
| | `:363` (1) | retirée (la clé n'est pas fausse quand `od > 0` : `:355` l'écrit) | (a2) 1 |
| | `:374` (1) | retirée ; `has_jb = 1`, `jb_row = pico8_hp_jb(b)` restent (TROU-23 quand `hp:jb` est nil) | (a2) 1 |
| `score_panel_display.c` | `:279`, `:294` (1 chacune ; ≈ `:263`, `:278` après D.18a, H-D18a-3) | retirées (`= {0}`) | (a2) 2 |

Totaux `src/` (lignes / occ.) : `ui_button.c` 16 / 21, `ui_button.h` 5 / 6, `menu.c` 8 / 8, `help.c` 1 / 1, `msgbox.c` 1 / 1, `planet_display.c` 8 / 8, `score_panel_display.c` 2 / 2 : **41 lignes, 47 occ.** Classes : (a1) 5, (a2) 29, (b) 7.

**Jonctions `ui_button.c:228-229` et `ui_button.h:107-108`, permises parce qu'elles ne changent pas le compte.** Aujourd'hui la signature de `pq` tient sur deux lignes, toutes deux comptées. La nouvelle (`void pico8_js_pq(pico8_js_t *self, const pico8_js_keys_t *keys)`) tient sur une ligne au format du dépôt ; forme contrefactuelle sur deux lignes (`… *self,` / `const pico8_js_keys_t *keys) {`) : aucune ne porte le motif. **Dans les deux formes, les lignes comptées retirées sont deux, gagnées zéro.** Déclarées au message avec ce contrefactuel. L'appel `:188-189` (non compté) devient une ligne, sans effet.

#### Les TU suivent (25 lignes, 29 occ. retirées)

Numéros de `d183e10` ; dans `test_pico8_js`, les lignes après `:789` descendent de 1 après D.18a (H-D18a-8) ; dans `test_pico8_lq`, `:524` et `:893` descendent d'un nombre à remesurer (H-D18a-9).

| TU | Lignes (occ.) | Nouveau texte | Classe |
|---|---|---|---|
| `js` | `:997` (1) | `free_keys.h_is_false = 1;` (`test_draw`, le bouton gratuit de `jo:fe`) | (a1) 1 |
| | `:1030`, `:1031`, `:1032` (1 chacune) | retirées (`b = {0}` : nil) ; `test_nil_keys` garde ses deux constructions et ses assertions | (a2) 3 |
| | `:1358` (1) | `k.h_is_false = 1;` (`fold_keys`, `h_` faux) | (a1) 1 |
| | `:1365`, `:1371`, `:1376` (1 chacune) | retirées : `has_h_ = 1` seul, `has_jb = 1` seul (TROU-23), `k = none` seul (nil) ; chaque cas garde son `built(k)` et son pli | (a2) 3 |
| | `:1383` (1) | **`h_` copie d'un `lq`** : genre qu'aucun producteur n'écrit (G4), mais le cas est plié dans `JS_MEMBERS_HASH` (G1). Le cas devient : `k = none; k.has_h_ = 1; b = built(k);` puis **`b->h_ = pico8_lq_copy(row);`** (le membre que construisait `lq(lq)`, écrit directement, prior art `test_draw:1000`) ; l'assertion `:1385` et le pli `:1386` inchangés : **mêmes octets** (même `has_h_`, même copie de `row`, `pico8_lq_new` n'enregistre rien de global hors `PICO8_CLASS_COVERAGE`) | (a1) 1 |
| | `:1105`, `:1106` (2, 1) ; `:1113`, `:1117` (1 chacune) | `zero_union`, `vec_ref` retirées : seul appelant, le test supprimé (`-Werror`) | (a2) 4 |
| | `:1624`, `:1649`, `:1661`, `:1662`, `:1668`, `:1670`, `:1671`, `:1676`, `:1677` (1, 1, 1, 1, 1, 2, 1, 3, 1) | **`test_js_new_stops_off_row` supprimé** (G4) avec `g_off_row`, `build_lq_off_row`, `build_h__off_row`, `build_jb_off_row`, `stops_on` (`:1624-1680`) et son appel `:1819` : ses cas sont une référence étrangère en `lq` ou `h_` (vecteur, `jb`, `js`, rangée générée), un `jb` faux, une union à zéro de genre booléen — aucun ne se range plus dans un champ ; `must_abort` (`:1054-1068`) perd son seul appelant et part (`-Werror`) ; `<signal.h>` (`SIGABRT`, seul usage) part | (a2) 9 |
| | `:1711` (1) | retirée (`row = {0}`, `test_pz_fa_only_from_ca`) | (a2) 1 |
| | commentaire `:1313-1315` (0) | « `h_` false, TROU-23, an `h_` copied from an `lq` (written on the member: no key builds it), a row's keys » | — |
| | **inchangés** : `fold_value` (`:207-215`), `fold_js_fn`, `fold_kind` et ses appels, `is_lq`, `is_of_class`, `build()` (D.18a), include `:46` | plis et aides de D.19b | — |
| `lq` | `:524` (1) | `spec.lq_copy = lq;` (`void *` vers `const pico8_lq_t *`, sans cast) | (a1) 1 |
| | `:893` (1) | **`button_on_other` supprimé** (`:891-895`) et son appel `:949` (G4 : une struct `ui` en source `lq` ; le champ typé ne l'admet plus, et sans `pico8_js_is_lq` elle ne s'arrêterait plus, elle serait lue comme un `lq`) ; `must_abort` garde ses appelants `keys_of_row`, `ol_on_other` (H-D18a-9) ; commentaire `:897-900` : « `ol` refuses anything that is not a row » (la comparaison « as `js:pq` refuses another struct » part) | (a2) 1 |

Totaux `tests/` (lignes / occ.) : `js` 23 / 27, `lq` 2 / 2 : **25 lignes, 29 occ.** Classes : (a1) 4, (a2) 21.

**Total D.18b : 66 lignes, 76 occ. ; (a1) 9, (a2) 50, (b) 7, (c) 0.**

**Tests supprimés (G4), nommés au message** : `test_js_new_stops_off_row` (`test_pico8_js`, un test, 7 arrêts) ; le cas `button_on_other` de `test_lq_is_a_struct` (`test_pico8_lq`, un arrêt). Ni l'un ni l'autre n'est un test de régression du §6.

**Aides de TU et `-Werror`** : partent avec leur dernier appel : `zero_union`, `vec_ref`, `must_abort`, `stops_on`, `build_*_off_row`, `g_off_row` (`js`), `button_on_other` (`lq`). Restent appelées : `is_of_class`, `ix_buttons`, `fold_value`, `fold_kind`, `built`, `must_abort` de `lq`. Aucune aide créée.

#### Includes

Retirés : `menu.c:24` `"pico8_value.h"` (compté), `ui_button.c:3` `<assert.h>`, `test_pico8_js.c:8` `<signal.h>` (orphelins du diff). Aucun ajouté ; aucun autre retiré ; en-têtes et includes `pico8_value.h` des TU gardés (D.19c).

#### `Makefile` et liens

- Aucun fichier nouveau ; **aucune ligne du `Makefile` ne change**.
- Règle de lien (aucun fichier de `DATA_ENGINE_SRCS`, `Makefile:229-279`, ne nomme un symbole défini hors de cette liste) : D.18b **n'ajoute aucun symbole** ; `ui_button.c`, `planet_display.c`, `msgbox.c` (dans la liste) n'appellent rien de neuf (`pico8_lq_copy`, `pico8_lq_new`, `pico8_jb_new` : `layout.c`, dans la liste ; `pico8_hp_lq` déjà appelé). Vérifié par la construction des binaires à liste courte (grep 6).

## Solution

**Un commit** `refactor(D.18b)` (12 chemins : 7 de `src/`, 2 TU, `docs/plan-3.md`, `docs/trous-de-portage.md`, `docs/glossaire-noms.md`), vert seul, vérifié, poussé :

1. Les sources d'un bouton sont des champs typés : copie d'un `lq`, rangées, présence, faux.
2. `js:pq` lit les clés et applique TROU-23 sur les drapeaux ; le constructeur ne fabrique plus de valeur.
3. `pico8_js_truthy` (dernière copie) et `pico8_js_is_lq` (dernière lecture d'étiquette de `ui_button`) disparaissent.
4. Les producteurs n'écrivent plus nil à la main ; `jo:fe` écrit son faux et sa copie dans leurs champs.
5. Les TU suivent aux mêmes octets ; les cas d'un genre que `src/` n'écrit plus sont supprimés (G4).

**Sortie**, constatée par commande :

- `make test` vert (dont `data-guard`, `constants-check`, `float-grep`, `float-guard`) ; `make e2e` **26 conformes** ; `make class-coverage-net` **identique** à l'attendu ; balayage ASan+UBSan des TU propre ;
- `make data-residue` : `valeur` **560 → 494** lignes (**623 → 547** occ.), fichiers **83 → 78** (sortent `ui_button.c`, `menu.c`, `help.c`, `msgbox.c`, `planet_display.c`) ; `type-tag` **242 / 58 → 241 / 57** (`ui_button.c:19`) ; résidu D **800 / 101 → 733 / 99** (sortent `ui_button.c`, `msgbox.c` ; `menu.c`, `help.c`, `planet_display.c` restent par `type-tag`) ; départs de H-D18a-1 (sinon : départ réel − 66 / − 76 occ., `type-tag` − 1) ;
- ces commandes rendent exactement ce qui est écrit :

  ```sh
  P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
  # 1. vidés : 0 pour chacun
  grep -cE "$P" src/pico8_ui_button.c src/pico8_menu.c src/pico8_help.c src/pico8_msgbox.c src/pico8_planet_display.c
  # 2. ui_button.h : une seule ligne, l'include
  grep -nE "$P" src/pico8_ui_button.h
  # 3. copies truthy : vide
  grep -lE 'static int [a-z0-9_]*truthy\(pico8_value_t' src/*.c
  # 4. plus de lecture d'étiquette ni de genre dans ui_button : vide
  grep -nE 'pico8_js_is_lq|pico8_js_truthy|pico8_type_tag|PICO8_TYPE_TAG' src/pico8_ui_button.c
  # 5. clés et pq : trois lignes, 'const pico8_lq_t *lq_copy;', 'int h_is_false;',
  #    'void pico8_js_pq(pico8_js_t *self, const pico8_js_keys_t *keys);'
  grep -nE 'lq_copy;|h_is_false;|void pico8_js_pq\(' src/pico8_ui_button.h
  # 6. règle de lien
  make tmp/test_pico8_data_init tmp/test_pico8_map_tables tmp/test_pico8_jb_ol \
       tmp/test_pico8_planet_kj_dispatch tmp/test_pico8_bg_kj tmp/test_pico8_planet_gh_row_lq \
       tmp/test_pico8_ga_export_panel_by tmp/test_pico8_layout_nil_item_draws_nothing
  # 7. aucune constante de TU changée : vide
  git diff <parent> -- tests | grep -E '^[-+].*(_HASH|identity\[)' | grep -vE '^[-+]\s*(/\*|\*)'
  # 8. includes : exactement trois lignes '-' (menu.c pico8_value.h, ui_button.c <assert.h>,
  #    test_pico8_js.c <signal.h>), aucune '+'
  git diff <parent> -- src tests | grep -E '^[-+]\s*#include'
  # 9. TROU-23 : un marqueur nu dans ui_button.c, texte de trous-de-portage sans pico8_js_new_table
  grep -c 'TROU-23 \*/' src/pico8_ui_button.c; grep -c 'pico8_js_new_table\|remplace alors la valeur nil' docs/trous-de-portage.md
  # 10. plus d'écriture de source par valeur : vide
  grep -rnE '\.(lq|h_|jb) = pico8_value_|->(lq|h_|jb) = pico8_value_' src tests
  # 11. comptes ; 12. filets
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^(valeur|type-tag)'
  make class-coverage-net
  ```

  (grep 9 : `1` puis `0`.)

## User Stories

1. En tant que mainteneur du port, je veux que les sources `lq`, `h_`, `jb` d'un bouton soient des champs typés, afin qu'aucune valeur Lua ne soit fabriquée pour dire nil, faux ou une copie.
2. En tant que mainteneur du port, je veux un champ `lq_copy` qui pointe un `lq` à recopier, afin que `jo:fe` passe le `lq` de `hp:lq` sans étiquette de type à vérifier.
3. En tant que mainteneur du port, je veux un drapeau `h_is_false` distinct de la présence `has_h_`, afin que le `false` d'un bâtiment gratuit de `jo:fe` reste séparé du nil présent de `jo:gh` (G2).
4. En tant que mainteneur du port, je veux que `jo:fe` écrive `h_is_false = od <= 0`, afin que la clé dise ce qu'écrit le cart (`h_=od>0 and {…}`) sans jamais être à la fois fausse et rangée.
5. En tant que mainteneur du port, je veux que TROU-23 s'écrive dans `js:pq` sur les drapeaux (présent sans rangée ni faux : rangée ou `jb` vides), afin que l'écart soit préservé à l'identique là où le membre est construit.
6. En tant que mainteneur du port, je veux que `pico8_js_pq` lise la struct de clés, afin que son unique appelant ne recopie pas six champs dans des paramètres.
7. En tant que mainteneur du port, je veux que `pico8_js_truthy` et `pico8_js_is_lq` disparaissent avec leur dernier appel, afin qu'il ne reste ni copie `truthy` ni lecture d'étiquette dans `ui_button`.
8. En tant que mainteneur du port, je veux que les producteurs n'écrivent plus `pico8_value_ref(NULL)` dans une struct déjà à zéro, afin que chaque ligne de clé dise quelque chose.
9. En tant que mainteneur du port, je veux que `menu.c` perde son include de `pico8_value.h`, afin qu'un fichier qui ne nomme plus le module ne l'inclue plus.
10. En tant que gardien des tests, je veux que `JS_MEMBERS_HASH`, `DRAW_HASH`, `HP_LQ_HASH` et toute constante restent égaux, afin de prouver que les membres construits n'ont pas changé.
11. En tant que gardien des tests, je veux que le cas « `h_` copie d'un `lq` » de `fold_keys` écrive son membre directement, afin que la constante garde ses octets sans qu'un champ existe pour un genre que `src/` n'écrit pas.
12. En tant que gardien des tests, je veux que les tests d'arrêt sur des genres devenus inexprimables soient supprimés et nommés au message, afin qu'aucun test ne passe par un trou de type.
13. En tant que gardien des tests, je veux que `test_nil_keys` garde ses assertions (nil présent : rangée et `jb` vides ; rien : aucun), afin que TROU-23 reste fixé par un TU.
14. En tant que gardien des tests, je veux des mutations exigées sur les deux branches de TROU-23 et sur la séparation faux / nil, afin qu'on ne puisse pas confondre les trois états sans rougir.
15. En tant que gardien des filets, je veux `make test`, `make e2e`, `make class-coverage-net` et le balayage ASan+UBSan verts, afin de prouver qu'aucun pixel, accès mémoire ou compte de constructeur ne change.
16. En tant que gardien des tests, je veux `gcov` avant/après et des mutations restaurées par `cp`, afin que chaque ligne modifiée soit exécutée et qu'aucune ligne survivante ne cesse de l'être.
17. En tant que gardien du compteur, je veux au message du commit les comptes par catégorie et par fichier, en lignes et occurrences, la classification (a1)/(a2)/(b)/(c), les lignes qui gagnent une catégorie (« aucune » compris), les jonctions de signature avec leur contrefactuel, avec les numéros du parent, afin qu'aucune baisse ne soit un tassement.
18. En tant que lecteur de `docs/trous-de-portage.md`, je veux que TROU-23 nomme les drapeaux qui le portent et le TU qui le fixe, afin qu'il cesse de nommer un pont disparu et une valeur `true`.
19. En tant que lecteur du glossaire, je veux que l'entrée `js` nomme `lq_copy`, `h_is_false`, `has_h_`, `has_jb` et les rangées, afin qu'elle reste vraie.
20. En tant que `plan-pilot`, je veux une étape d'environ vingt minutes, un plafond, un repli et une note « Découvert à D.18b », afin que D.18c parte d'un état exact.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider modules, tests et découpage ; ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, orphelins du diff retirés, critères vérifiables) et marqués **(G)** ; les rejets sont notés. Q12 (oui) et Q13 (A) sont des décisions humaines ; l'étude D.14 (B9, G1–G4) fait foi sauf écart déclaré ici (`pq` prend la struct de clés et non les champs un à un ; `jb` sans champ propre ; `h_` copie d'un `lq` retiré de l'API et gardé par le TU sur le membre ; `test_js_new_stops_off_row` et `button_on_other` supprimés ; include de `menu.c` retiré ; la ligne `planet_display.c:357` que D.15c envoyait à D.18b relève de B8/D.18a).

### Modules

- **Bouton** : `pico8_ui_button.c/.h` (clés, constructeur, `pico8_js_lq`, `pico8_js_pq`, retraits).
- **Producteurs** : `pico8_menu.c`, `pico8_help.c`, `pico8_msgbox.c`, `pico8_planet_display.c`, `pico8_score_panel_display.c`.
- **TU** : `js`, `lq` (suivent). **Docs** : `plan-3.md` (note), `trous-de-portage.md` (TROU-23), `glossaire-noms.md` (entrée `js`).
- Inchangés : `Makefile`, `tools/`, `pico8_value.c/.h`, `pico8_type_tag.h`, `pico8_js_t` et ses membres (`lq`, `h_`/`has_h_`, `jb`), `pico8_lq_t`, `pico8_lq_copy`, `pico8_jb_t`, `pico8_pz`, `pico8_pz_step`, `layout.c/.h`, `research_display.c`, `score_panel.c/.h`, `pi.c`, les TU hors `js`/`lq`, les §6 (`planet_gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`), `tests/e2e/`.

### Représentation (G)

- `pico8_js_keys_t` : `const pico8_lq_t *lq_copy` (à la place de `lq`), `int h_is_false` (à la place de `h_`), `jb` retiré ; `lq_row`, `has_h_`, `h_row`, `has_jb`, `jb_row` inchangés. Aucun nouveau type, aucune union.
- `pico8_js_pq(pico8_js_t *self, const pico8_js_keys_t *keys)`, `keys` NULL : aucune source (le membre `lq` reste une rangée vide, comme `pico8_js_new(NULL)` aujourd'hui).
- Table de vérité : celle de « Preuve B9, par le port », colonne « Après ».
- **Piège de motif** : aucun nom neuf ne correspond à un motif (`lq_copy`, `h_is_false` : ni `valeur`, ni `spec`, ni `par-nom` — pas de `_take_`, pas de `field` —, ni `liste`).
- **Piège de forme** : garder une `pico8_value_t` locale « pour TROU-23 », ou une aide de conversion drapeaux ↔ valeur, serait une cale ; interdit.

### Anti-tassement et message de commit (liant)

- **Interdit** : joindre des lignes comptées, sauf les deux signatures de `pq` (`.c:228-229`, `.h:107-108`), permises et déclarées (contrefactuel : même compte) ; renommer un identifiant vivant pour échapper au motif (`lq_copy` change de type et de sens : (b), déclaré) ; alias (`#define`, `typedef`) ; cast nouveau (les `(void *)`/`(const pico8_lq_t *)` retirés ne sont pas remplacés) ; cale valeur ↔ typé ou pont ; toucher une constante de hachage, `tests/e2e/`, un `*.sha256`, `tools/`, le `Makefile`, un include hors des trois listés.
- **Classification exigée** (recette : `git diff -U0 <parent>` par fichier, lignes `-` qui portent le motif en regard des `+`) : **(a1) 9, (a2) 50, (b) 7, (c) 0**, recomptée au parent.
- **Mesure exigée**, avant et après, par catégorie (`valeur`, `type-tag` ; les huit gardées à 0 constatées) et par fichier du diff, en lignes (`--detail`) **et** occurrences.
- **Lignes qui gagnent une catégorie**, par fichier et catégorie, « aucune » compris : attendu **aucune**.
- **Lignes comptées modifiées, non retirées** : attendu **aucune**.
- **Δlignes > Δoccurrences** : attendu aucun fichier.
- **Lignes prises à une autre étape** : les deux lignes de la copie `truthy` (`ui_button.c:27-28`, B12 de l'étude, rangées avec leur lecteur) ; l'include de `menu.c` (règle de D.15b). Déclarées.
- **Numéros de ligne** : ceux du parent.

### Empreinte et découpage (G)

| Commit | `src/` | `tests/` | docs | Chemins |
|---|---|---|---|---|
| **D.18b** | `ui_button.c`, `ui_button.h`, `menu.c`, `help.c`, `msgbox.c`, `planet_display.c`, `score_panel_display.c` (7) | `js`, `lq` (2) | `plan-3.md`, `trous-de-portage.md`, `glossaire-noms.md` | 12 |

(L'étude disait ≈ 10 fichiers et TU `js`, `lq` : conforme, plus les docs.)

- **Rythme mesuré** : D.15a–D.17, ≈ 7 à 11 min de la mise en file au commit pour 40 à 55 lignes comptées (D.17 : 52 lignes, ≈ 10 min, gradée à ≈ 16 min) ; D.18a estimée à ≈ 25 min pour 133 lignes (H-D18a-11, à recalibrer). **D.18b** : éditions ≈ 6 (12 chemins, 66 lignes dont 50 retraits) ; `gcov` départ/arrivée ≈ 3 ; mutations (≈ 12) ≈ 5 ; filets (`class-coverage-net`, balayage ASan+UBSan) ≈ 4 ; mesures, classification, message, note ≈ 3 : **budget ≈ 20 min jusqu'au commit**, **plafond 50 min**.
- **Repli écrit** (G) : au plafond sans commit, rien n'est commis et `plan-pilot` relance en **D.18b1** (`pico8_js_pq(self, keys)` lisant encore les trois valeurs des clés, TROU-23 déplacé du constructeur dans `pq`, locales retirées ; ≈ 10 lignes, `ui_button.c/.h` seuls) puis **D.18b2** (champs typés, producteurs, `jo:fe`, retraits, TU, docs ; ≈ 56 lignes). Sans pont. Comptes recomptés par commit. Rejeté comme plan principal : deux coûts fixes pour 66 lignes surtout mécaniques.

### Effet sur `make data-residue`

| Catégorie | Départ attendu (H-D18a-1) | Après D.18b |
|---|---|---|
| `valeur` | 560 / 83 ; 623 occ. | **494 / 78 ; 547 occ.** (− 66, − 76) |
| `valeur` `src/` | 181 / 45 ; 212 occ. | 140 / 40 ; 165 occ. |
| `valeur` `tests/` | 379 / 38 ; 411 occ. | 354 / 38 ; 382 occ. |
| `type-tag` | 242 / 58 | **241 / 57** |
| résidu D | 800 / 101 | **733 / 99** |
| `DATA_RESIDUE_GUARD` | inchangé | inchangé |

Par fichier (lignes / occ. ; départ attendu → après) : `ui_button.c` 16/21 → 0/0 ; `ui_button.h` 6/7 → 1/1 ; `menu.c` 8/8 → 0/0 ; `help.c` 1/1 → 0/0 ; `msgbox.c` 1/1 → 0/0 ; `planet_display.c` 8/8 → 0/0 ; `score_panel_display.c` 13/13 → 11/11 ; `test_pico8_js` 39/44 → 16/17 ; `test_pico8_lq` 30/30 → 28/28.

Fonctions publiques qui nomment `pico8_value_t` : 2 → **1** (`pico8_pj_item_of`, D.18c). Copies `truthy` : 1 → **0**.

« − 66 / − 76 », « `type-tag` − 1 » (depuis le départ réel) sont des critères (écart = explication chiffrée au message).

### Documentation (G)

- `docs/plan-3.md` : une note « Découvert à D.18b » en fin de ligne D.18b, rien d'autre. Elle dit : le commit ; les comptes réels et l'écart à ≈ 505 ; les lectures (`lq_copy`, `h_is_false`, `jb` sans champ, `pq` sur la struct de clés, TROU-23 dans `pq`, `h_is_false = od <= 0` à `jo:fe`, `pico8_js_is_lq` et la dernière copie `truthy` retirées, `type-tag` − 1) ; tests supprimés (G4) ; le cas `h_` copie gardé sur le membre ; **pour D.18c** : `score_panel_display.c` n'a plus que B11 (11 lignes) ; **pour D.19b** : `test_pico8_js` garde 16 lignes (`fold_value`, `fold_kind`, include), `test_pico8_lq` 28 ; **pour D.19c** : `ui_button.h:13` n'a plus que son include ; **pour D.20** : `is_lq`/`is_of_class` de `test_pico8_js` lisent encore l'étiquette.
- `docs/trous-de-portage.md`, TROU-23 (l. 374-391), dernier paragraphe réécrit (G3), le reste inchangé : « La struct `js`, où nil vaut absent, garde ce comportement : la présence d'une clé `h_` ou `jb` nil est portée par `has_h_`/`has_jb` de `pico8_js_keys_t`, sans `h_row` ni `h_is_false` (sans `jb_row`) ; `js:pq` construit alors une rangée ou un `jb` vides. Fixé par `test_nil_keys` de `test_pico8_js`. » (Le pont `pico8_js_new_table` et le `true` de remplacement disparaissent du texte.)
- `docs/glossaire-noms.md:444` (entrée `js`, H-D18a-10) : « ses clés : `pico8_js_keys_t` (`lq` : sa rangée par ses clés `lq_row`, sinon la copie d'un `lq` `lq_copy`, sinon vide ; `h_` : présence `has_h_`, rangée `h_row` ou faux `h_is_false` ; `jb` : présence `has_jb`, rangée générée `jb_row` ; présent sans rangée ni faux : rangée ou `jb` vides, TROU-23) ».
- Commentaires : quatre sortes de `CLAUDE.md` ; contrats réécrits (`ui_button.h:65-69`, `:75`, `:78`, `:104-106` ; `pico8_js_lq` `:215-216`) ; marqueur nu `/* TROU-23 */` déplacé dans `pq` ; commentaires de TU réécrits (`fold_keys` `:1313-1315`, `test_lq_is_a_struct` `:897-900`) ; commentaires retirés avec leur objet (`test_js_new_stops_off_row` `:1657-1658`) ; aucun renvoi à D.18b ni au plan dans `src/` ou `tests/`.

### Ce qui ne change pas

- `tests/e2e/`, tout `*.sha256`, `tools/`, le `Makefile` ; aucun e2e ajouté ou supprimé ; toute constante `*_HASH`/`identity[]` ; `tools/class_coverage_net_expected.txt`.
- Le comportement : mêmes membres `lq`, `h_`/`has_h_`, `jb` pour tout bouton que `src/` construit ; mêmes pixels ; mêmes durées de vie (TROU-7) ; TROU-9 et TROU-23 préservés ; `make e2e` conforme au bit près.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : sous `-Wall -Wextra -Werror`, les `static` retirées seraient fatales si gardées (retirées dans le même commit) ; `pico8_lq_t *` vers `const pico8_lq_t *` et `void *` vers `const pico8_lq_t *` s'affectent sans cast.
- **H2** : aucun autre écrivain ni lecteur de `pico8_js_keys_t.lq/.h_/.jb` n'existe hors de l'Inventaire (grep à `d183e10`, `src/`, `tests/`, `tools/`) ; un oublié se révèle à la compilation (champ renommé ou retiré) et suit, listé.
- **H3** : toutes les structs de clés sont à zéro avant écriture (preuve par site) ; si ASan/UBSan ou `gcov` montre une lecture de `lq_copy`/`h_is_false` non initialisée, H3 est fausse : arrêt, rapporté.
- **H4** : `pico8_lq_new(NULL)` et `pico8_jb_new(NULL)` rendent les mêmes rangée et `jb` vides que les branches « vrai » d'aujourd'hui (même appel) ; `JS_MEMBERS_HASH` le juge.
- **H5** : le cas réécrit de `fold_keys` (`b->h_ = pico8_lq_copy(row)`) plie les mêmes octets (même membre ; l'ancien chemin appelait `pico8_lq_copy(row)`) ; `JS_MEMBERS_HASH` le juge.

## Implementation Chunks

**D.18b** (un commit)

1. Mesure de départ (commit de D.18a ou un descendant `docs/` seul ; attendus H-D18a-1) ; remesure par contenu des numéros ; jugement des `H-D18a-n` rapporté au message ; `gcov` de départ.
2. `ui_button.h/.c` : clés, `pq`, `pico8_js_lq`, constructeur, retraits.
3. Producteurs : `menu.c` (dont l'include), `help.c`, `msgbox.c`, `planet_display.c`, `score_panel_display.c`.
4. TU `js` (`test_draw`, `test_nil_keys`, `fold_keys`, suppression G4 et aides, `test_pz_fa_only_from_ca`), `lq` (`test_hp_lq_and_copy`, `button_on_other`) ; `make tmp/test_pico8_js tmp/test_pico8_lq` et lancement de chacun.
5. Docs : TROU-23, glossaire.
6. `make test`, `make e2e`, `make class-coverage-net`, balayage ASan+UBSan ; `gcov` d'arrivée ; mutations ; mesures, classification ; note « Découvert à D.18b » ; **commit**, vérification, push.

## Testing Decisions

Un bon test juge ce que le code rend ou fait (membres construits, pixels, rangées de boutons), pas la présence d'une ligne. D.18b change le type des clés sans changer un membre ni un pixel pour les genres que `src/` écrit. La preuve : les TU existants, qui plient les membres dans des constantes inchangées (`JS_MEMBERS_HASH`, `DRAW_HASH`, `HP_LQ_HASH`…) et assertent les états de `h_`/`jb` (`test_nil_keys`, `test_draw`, `fold_keys`, `members_fe`, `members_research`) ; les 26 e2e au bit près ; `class-coverage-net` identique ; le balayage ASan+UBSan ; `gcov` et mutations.

### TU touchés (G)

1. Suivent : `js`, `lq`.
2. Prouvent sans suivre : `ui`, `spec_defaults` (`pico8_js_new(NULL)`), `fm`, `fv`, `mh`, `pj`, `ga`, `hp` (boutons de `cr`, `ix`, `ha`, `bo`, `pz`, `jo:gh`, `jo:fe`, `pj:ph`), §6 `planet_gh_row_lq`, `ga_export_panel_by` (panneaux à boutons).
3. **Constantes** : aucune `*_HASH`/`identity[]` ne change ; une constante qui rougit est un bug de conversion.
4. **Assertion nouvelle** : seulement pour couvrir une ligne modifiée non exécutée ou faire rougir une mutation qui ne rougit rien, dans un test du module (`test_pico8_js`), listée au message ; rien de lent, aucun fichier d'événements.
5. **Prior art** : membre écrit directement par un TU (`test_draw:1000`, `((pico8_js_t *)tooltip)->jb = …`) ; cas G4 supprimés et nommés (`pico8_oj` d'un nombre, D.16b2 ; `pico8_jc_hg_has` d'un nombre, D.15b) ; tests d'arrêt supprimés avec leur genre (D.11d3–d4) ; fonctions `static` retirées avec leur dernier appel (D.15b–D.18a) ; drapeau + pointeur à la place d'une valeur (`has_my` + `my`, D.10–D.12).

### Preuve des tests (§7 : `gcov` et mutations)

**`gcov`** : dans une copie jetable du scratchpad, tous les TU compilés en `--coverage` (flags du `Makefile` + `GCOV_CFLAGS`), **les TU qui forkent** (`in_fresh_child`/`in_fresh_part`/`must_abort`, dont `js`, `lq`, sortie par `_exit`) compilés en plus avec **`-D_exit=exit`** ; lancés comme le `Makefile` (depuis la racine, `ASAN_OPTIONS=detect_leaks=0` pour `test_pico8_planet_gh_row_lq`) ; union des lignes de `src/` exécutées, départ et arrivée.

- Chaque ligne **ajoutée ou modifiée** de `src/` est exécutée par l'union des TU, sinon par les e2e (`make line-coverage EVENTS="tests/e2e/*.json"` → `tmp/line_coverage.txt`), sinon couverte par une assertion ajoutée, sinon justifiée au message. Attendus : `pico8_js_lq`, branche rangée (partout), copie (`lq:test_hp_lq_and_copy`, `js:members_fe` si un projet sans `lq` donnée), vide (`pico8_js_new(NULL)`, `ui`, `spec_defaults`) ; `pq`, `keys` NULL (`js` `fresh[0]`, `ui`) ; `h_` rangée (`cr` : `js:members_research`, `lq` boutons à `h_`), vide TROU-23 (`test_nil_keys`, `fold_keys`), faux (`test_draw`, `fold_keys`, `members_fe`) ; `jb` rangée (`js:675-678`, `cr`, `jo:fe`), vide TROU-23 (`test_nil_keys`, `fold_keys`) ; `planet_display.c:351`, `:355` (`js:members_fe`, `lq`, `ga`).
- **`L`** = lignes **survivantes** de `src/` exécutées au départ et plus à l'arrivée (appariées par contenu et fonction) : **attendu vide**. Partantes, hors `L` : `pico8_js_truthy`, `pico8_js_is_lq`, les trois `assert` et branches de genre de `pico8_js_lq`/`pq`, TROU-23 du constructeur. **Surveillé** : `pico8_value_ref`, `pico8_value_bool` (`pico8_value.c`) perdent des appelants ; ils restent appelés par `pi.c`, `score_panel.c`, `score_panel_display.c` (D.18c) et `test_pico8_value` ; une ligne de `pico8_value.c` dans `L` est justifiée par ce seul appelant (D.19c). Toute autre ligne de `L` est un comportement perdu : arrêt, rapporté.

**Mutations**, une à la fois, dans l'arbre de travail : fichier sauvegardé par `cp` avant, restauré par `cp` simple puis `touch` après (jamais `git checkout`/`stash`) ; chaque TU lancé exactement comme le `Makefile` le lance ; chacune doit rougir au moins le TU nommé, sinon trou couvert par une assertion ou justifié (mutation équivalente démontrée). **Astuces** : neutraliser par `0 ? x : …`, `0 &&`, une condition toujours fausse, plutôt que supprimer (`-Werror`) ; si un `assert` de `src/` attrape la mutation avant le TU, relancer avec cet `assert` relâché dans la copie mutée, et rapporter les deux.

- **M1 (exigée) — TROU-23, `h_`** : dans `pq`, `(keys->has_h_ && !keys->h_is_false)` neutralisé (`0 &&`) → **`js` rouge** (`test_nil_keys` : `is_lq(h_)` ; `fold_keys` ; `JS_MEMBERS_HASH`) ; e2e possibles (`jo:gh` sans `kr`).
- **M2 (exigée) — TROU-23, `jb`** : `|| keys->has_jb` neutralisé → **`js` rouge** (`test_nil_keys` : `jb != NULL` ; `fold_keys`) ; e2e possible (infobulle vide de `jo:fe`).
- **M3 (exigée) — faux ≠ nil** : `!keys->h_is_false` neutralisé (`h_is_false` ignoré : le faux devient une rangée vide) → **`js` rouge** (`test_draw` : `h__state(free_) == 1` ; `fold_keys` : `h_ == NULL`) ; **M3b** : la branche `else if (keys->has_h_)` ne pose plus `has_h_` (faux lu comme absent) → `js` rouge (mêmes assertions, `JS_MEMBERS_HASH`). **Si M1, M2, M3 ou M3b ne rougit pas : `FAILED:`.**
- **M4** `jo:fe` : `h_is_false = od < pico8_fixed_from_int(0)` (le gratuit `od == 0` devient nil présent) → `js` (`members_fe` si le panneau connu a une ligne gratuite ; `JS_MEMBERS_HASH`), sinon e2e ; sinon assertion ajoutée dans `test_pico8_js` (une ligne de `jo:fe` à `od` nul a `h_` faux), ou justification (aucun bâtiment gratuit atteignable par un TU rapide).
- **M5** `pico8_js_lq` : branche copie neutralisée (`copy` ignoré : rangée vide) → `lq` (`test_hp_lq_and_copy` : `row_count(copy) == row_count(lq)`, `HP_LQ_HASH`).
- **M6** `jo:fe` : `keys->lq_copy = NULL` à la place de `pico8_hp_lq(b)` → `js` (`JS_MEMBERS_HASH` par `members_fe`) sinon `lq`/`ga`, sinon e2e (`project_tooltip_inputs`, `colony_panel`).
- **M7** `pq` : `h_row` ignoré (`pico8_lq_new(NULL)`) → `js` (`DRAW_HASH`, `members_research`), `lq` (`HP_LQ_HASH`).
- **M8** `pq` : `jb_row` ignoré (`pico8_jb_new(NULL)`) → `js` (`:675-678`, `jb->ls_count == 1` de la rangée « n invented »).
- **M9** `pico8_js_lq` : `lq_row` ignoré (la copie ou le vide seulement) → `js`, `lq`, `fm`, `fv`, `pj` (tous les boutons).
- **M10** constructeur : `pico8_js_pq(self, NULL)` à la place de `keys` → `js`, `lq` (tous les boutons perdent leurs rangées).
- **M11** `pq` : `keys` NULL traité en retour anticipé (membre `lq` laissé NULL) → `js` (`fresh[0]` : `lq != NULL`), `ui`, `spec_defaults`.
- **M12 (TU)** `fold_keys` : `b->h_ = pico8_lq_copy(row);` retirée → `js` rouge par `JS_MEMBERS_HASH` (l'`h_` reste la rangée vide de TROU-23 : l'assertion `:1385` passe, le pli change) : le cas plié est bien la copie.

Cibles (numéros de `d183e10`) : `ui_button.c:116-192`, `:215-254` ; `planet_display.c:342-376` ; `tests/test_pico8_js.c:1316-1397`. Une mutation dont la cible n'existe plus sous cette forme est remplacée par une mutation équivalente sur le même comportement, déclarée.

### Filets

- `make test`, `make e2e` (26 conformes). Aucun e2e ajouté.
- **`make class-coverage-net`** (exigé par la ligne) : sortie **identique** à `tools/class_coverage_net_expected.txt`.
- **Balayage ASan+UBSan des TU** : tous les TU compilés sous `-fsanitize=address,undefined` dans une copie jetable (`Makefile` inchangé), lancés avec `ASAN_OPTIONS=detect_leaks=0`.
- `make sanitize-net` : non exigé (aucune durée de vie ni propriétaire ne change) ; s'il est lancé, rapporté.
- Aucun TU ne rejoue un fichier d'événements.

### Vérification, budget et tableau « Runs »

**Budget : environ 20 min de temps réel jusqu'au commit**, mesures, `gcov`, mutations et filets compris ; **plafond 50 min** (au-delà : repli D.18b1/D.18b2). La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| `make tmp/test_pico8_js tmp/test_pico8_lq` + lancement | quelques secondes |
| `make test` | de l'ordre de la minute (`ui_button.h` change : recompilation large) |
| `make e2e` | ~1 s |
| `make class-coverage-net` | < 1 min |
| `make line-coverage EVENTS="tests/e2e/*.json"` | < 1 min |
| `gcov` de tous les TU (départ et arrivée) | quelques minutes |
| mutations (M1–M12, M3b) | < 6 min |
| balayage ASan+UBSan des TU | quelques minutes |
| `make data-residue`, recette, classification, greps | < 2 min |

Au-delà du plafond, l'implémenteur s'arrête et note ce qui n'a pas été fait ; rien d'inachevé n'est commis. **Jamais l'appli raylib** ; `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »**, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + état) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | commit de D.18a (gradée), avant D.18b | 0 | `valeur 560 83`, `type-tag 242 58` (H-D18a-1) | … |
| 2 | recette `valeur` | idem | 0 | `valeur 560 lignes 83 fichiers 623 occ.` | … |
| 3 | `grep -cE "$P" src/pico8_ui_button.c src/pico8_ui_button.h src/pico8_menu.c src/pico8_planet_display.c tests/test_pico8_js.c` | idem | 0 | `16`, `6`, `8`, `8`, `39` (H-D18a-4) | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` ne liste que les 12 chemins de « Empreinte » (plus un TU suiveur listé au message). Ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni `Makefile`, ni un §6.
2. **Greps** de la Sortie, exactement.
3. **Formes** : `lq_copy` en `const pico8_lq_t *`, `h_is_false` en `int`, pas de champ `jb` ; `pq(self, keys)` ; table de vérité « Après » lisible dans `pq` ; un marqueur nu TROU-23 ; `h_is_false = od <= 0` à `jo:fe` ; aucune écriture de nil restante.
4. **Casts** : aucun nouveau (`grep -nE '\(\s*(const\s+)?[a-z0-9_]+_t\s*\**\s*\)\s*[^{ ]'` et `grep -nE '\(\s*(const\s+)?void\s*\*\s*\)'` sur les lignes `+` du diff : vides).
5. **API** : seuls `pico8_js_keys_t` (`lq` → `lq_copy`, `h_` → `h_is_false`, `jb` retiré) et `pico8_js_pq` changent ; seules `pico8_js_truthy`, `pico8_js_is_lq` disparaissent ; aucun symbole nouveau.
6. **Liens** : grep 6.
7. **Constantes** : grep 7 vide ; M1, M2, M3, M3b, M12 rapportées.
8. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste ; aucune ligne gagnée ; jonctions des signatures déclarées avec leur contrefactuel ; includes (grep 8) ; lignes prises (B12, include) déclarées.
9. **Écarts déclarés** : `pq` sur la struct de clés ; `jb` sans champ ; `h_` copie d'un `lq` gardé sur le membre du TU ; tests G4 supprimés et nommés ; `planet_display.c:357` rangée B8 (D.18a) ; `type-tag` − 1 ; `<assert.h>`, `<signal.h>` retirés.
10. **Preuve** : `gcov` (lignes modifiées exécutées ou justifiées ; `L` vide ou justifié par `test_pico8_value` seul), mutations restaurées par `cp`, `class-coverage-net` et ASan+UBSan rapportés ; jugement des `H-D18a-n`.
11. **Commentaires** : quatre sortes de `CLAUDE.md` ; aucun renvoi à D.18b ni au plan dans `src/` ou `tests/`.
12. **Docs** : note « Découvert à D.18b » en fin de ligne D.18b, rien d'autre dans le plan ; TROU-23 (dernier paragraphe) et glossaire (entrée `js`) réécrits.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas le commit gradé de D.18a (ou un descendant qui ne touche que `docs/`), propre ; ou si `valeur` n'y vaut pas le départ réel annoncé au message (attendu 560 / 83 / 623 occ., `type-tag` 242 / 58, résidu D 800 / 101), sans explication.
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur le commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 change ; si un site ne tombe sous aucune règle de ce PRD ni de l'étude (arrêt « champ sans règle ») — en particulier une struct de clés non mise à zéro (H3) ou un genre de source écrit par `src/` hors de la table de vérité ; ou si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.
- `FAILED:` si le balayage ASan+UBSan des TU n'est pas propre, ou si `make class-coverage-net` diffère de l'attendu.

**Périmètre et forme**

- `FAILED:` si `tools/` ou le `Makefile` change, ou si un include change hors des trois listés ; si le commit touche un chemin hors de son « Empreinte » sans le lister comme TU suiveur.
- `FAILED:` si une grep de la Sortie ne rend pas ce qui y est écrit.
- `FAILED:` si une signature ou une fonction hors de la liste du contrôle 5 change ou disparaît ; si `pico8_js_t`, `pico8_lq_t`, `pico8_lq_copy`, `pico8_jb_t`, `pico8_pz`, les cases de `lq` (D.18a), `pico8_pj_item_of`/`ou[]`/`pi.c`/`s[i]` (D.18c) changent ; si une durée de vie change.
- `FAILED:` si TROU-23 n'est plus préservé à l'identique (nil présent : rangée ou `jb` vides ; faux : aucun ; absent : aucun), ou si faux et nil présent de `h_` se confondent.
- `FAILED:` si un cast nouveau est ajouté, si des lignes comptées sont jointes hors des deux signatures déclarées, ou si une cale valeur ↔ typé ou un pont apparaît (dans `src/` ou un TU).

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, ou si `DATA_RESIDUE_GUARD` change.
- `FAILED:` si `valeur` ne baisse pas de **66 lignes / 76 occ.** exactement depuis le départ réel, sans explication chiffrée ligne à ligne ; si `type-tag` ne baisse pas d'exactement **1** ligne.
- `FAILED:` **si un compte baisse par tassement** : lignes jointes (hors signatures déclarées), alias, renommage d'un identifiant vivant pour le motif, commentaire compté réécrit hors de son sujet, cale ou pont, (c) non nulle, (b) hors de `pico8_js_keys_t` et `pico8_js_pq`.
- `FAILED:` si le **message du commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a1)/(a2)/(b)/(c) avec numéros du parent ; les lignes qui gagnent une catégorie (« aucune » compris) ; les lignes prises (copie `truthy`, include) ; la justification de chaque Δlignes > Δoccurrences (attendu : aucune) ; les jonctions des signatures et leur contrefactuel ; les fonctions et aides retirées et leurs lecteurs (raison `-Werror`) ; les tests supprimés (G4) et pourquoi ; le cas `h_` copie gardé sur le membre ; la table de vérité avant/après ; le jugement des `H-D18a-n` ; le rapport `gcov` ; les mutations (M1, M2, M3, M3b en tête) ; `class-coverage-net`, ASan+UBSan ; le tableau « Runs » (ou son renvoi au journal). Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` diffère après `make test`.

**Preuve, commit et note**

- `FAILED:` si une constante `*_HASH` ou `identity[]` change.
- `FAILED:` si une ligne modifiée n'est exécutée ni par un TU ni par les e2e sans justification ; si `L` contient une ligne hors de `pico8_value.c` justifiée par `test_pico8_value`.
- `FAILED:` si une mutation manque, ne se restaure pas par `cp`, ou ne rougit rien sans justification ; **si M1, M2, M3, M3b ou M12 ne rougit pas**.
- `FAILED:` si le plafond est dépassé sans note (et sans repli), ou s'il manque un tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.18b », ou si elle omet : les comptes réels, les lectures, les tests supprimés, les legs pour D.18c, D.19b, D.19c, D.20.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`, ou si le marqueur TROU-23 disparaît de `src/pico8_ui_button.c`.

## Out of Scope

- B8 (cases de `lq`, `pico8_lq_cell_*`, dont les cases `h_item` de `jo:gh`/`jo:fe` que D.15c attribuait à D.18b) : D.18a.
- B10 (`pico8_pi_arg`), B11 (`s[i]`, `pico8_pj_ph_s_at`, `pico8_value_tostr` de `score_panel_display.c`), `ou[]` et `pico8_pj_item_of` : D.18c.
- Les plis `fold_value`, `fold_kind`, `fold_js_fn` de `test_pico8_js` et les restes de `test_pico8_lq` : D.19b.
- L'include de `ui_button.h` et des autres en-têtes vidés, ceux des TU ; le module lui-même : D.19c.
- Les lectures d'étiquette des TU (`is_lq`, `is_of_class`, `test_nil_keys:1040`) : D.20a.
- Corriger TROU-23 (interdit par le plan) ; la mise à jour des noms de rejeu périmés du paragraphe « Atteint par le filet » de TROU-23 (hors G3 : il ne nomme pas `pico8_value_*`).
- Tout e2e ; `make raylib`, l'appli raylib, l'audio, le rendu.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd`), tranchées en `guideline` et marquées (G) :
  - **Découpage** : un commit ; repli D.18b1 (`pq` sur la struct de clés, TROU-23 déplacé) / D.18b2 (champs typés, producteurs, TU, docs), sans pont.
  - **Types** : `lq_copy` (nom de l'étude), `h_is_false`, `jb` sans champ ; `pq(self, keys)` plutôt que huit paramètres.
  - **`jo:fe`** : `h_is_false = od <= 0`, écrit une fois ; la copie dans `lq_copy`.
  - **G4 contre G1** : les cas de TU d'un genre inexprimable sont supprimés quand ils ne plient rien (tests d'arrêt) ; le seul qui plie (`h_` copie, `fold_keys`) garde ses octets en écrivant le membre.
  - **Includes** : `menu.c` perd `pico8_value.h` (règle de D.15b) ; `<assert.h>` (`ui_button.c`) et `<signal.h>` (`test_pico8_js`), orphelins du diff, partent ; en-têtes gardés.
  - **Drapeaux** : `class-coverage-net` (ligne) ; balayage ASan+UBSan (consigne) ; `sanitize-net` non exigé.
- **Surprises de mesure, pour `plan-pilot`** : (1) **la « ligne pour D.18b » de D.15c** (`planet_display.c:377`, `:357` à `d183e10`) est une case de rangée (B8), prise par D.18a ; (2) **la table de vérité ne change pour aucun genre que `src/` écrit** : les seize écritures de nil de `src/` sont des lignes mortes dans des structs à zéro ; (3) **`{0}` met aujourd'hui `lq`/`h_`/`jb` au nombre 0, pas à nil** (`PICO8_VALUE_FIXED` vaut 0) : sans conséquence parce que `lq_row` ou l'absence de `has_h_`/`has_jb` masquent la valeur partout, mais c'est la raison pour laquelle le type ne pouvait pas rester ; (4) **le seul cas de TU qui plie un genre inexprimable** est `h_` copie d'un `lq` dans `fold_keys` ; (5) **`test_js_new_stops_off_row` disparaît entièrement** (sept arrêts, tous sur des genres que le type interdit) avec `must_abort`, `zero_union`, `vec_ref` : 13 lignes `valeur` de plus que l'étude ; (6) **`type-tag` baisse de 1** (`pico8_js_is_lq`) ; (7) **arrivée ≈ 494**, et non ≈ 505 (voir « Lecture de la ligne ») ; (8) TROU-23 nomme encore le pont `pico8_js_new_table` retiré depuis D.7, et des rejeux (`events_recorded_1790198294`, `session_events`) antérieurs à la fusion e2e.
- **Pour D.18c** : `score_panel_display.c` garde 11 lignes (B11) ; `pico8_value_tostr` y garde son appel ; `pico8_pj_item_of` est la dernière fonction publique qui nomme `pico8_value_t`.
- **Pour D.19b** : `test_pico8_js` garde 16 lignes / 17 occ. (`fold_value` `:207-215`, `fold_js_fn` `:1131`, `fold_kind` et neuf appels, include) ; `test_pico8_lq` 28.
- **Pour D.19c** : `ui_button.h:13` n'a plus que son include (avec `help.h`, `msgbox.h`, `planet_display.h`, `layout.h`).

## Annexe A — textes recopiés (`d183e10`)

### Lignes D.18a à D.18c (`docs/plan-3.md`, l. 289-291)

> - D.18a — B8 : `pico8_lq_keys_t.items` en cases de l'union de `layout` ; §6 `layout_nil_item_draws_nothing` et `planet_gh_row_lq` réécrits dans leur lecture, preuve rouge. `sanitize-net` : oui. Compte : ≈ 560.
> - D.18b — B9 : sources des boutons en champs séparés. `class-coverage-net` : oui. Compte : ≈ 505.
> - D.18c — B10, B11, `ou[]`. `sanitize-net` : oui. Compte : ≈ 435.

### TROU-23 (`docs/trous-de-portage.md:374-391`), dernier paragraphe, texte d'aujourd'hui

> La struct `js`, où nil vaut absent, garde ce comportement : la présence d'une clé `h_` ou `jb` nil est portée par `has_h_`/`has_jb` de `pico8_js_keys_t` (posés par `pico8_js_new_table`), et le constructeur de `js` remplace alors la valeur nil par `true`.

### Glossaire (`docs/glossaire-noms.md:444`), texte d'aujourd'hui

> | `js` | `pico8_js_t` | un bouton cliquable ; ses clés : `pico8_js_keys_t` (`lq`, `h_`, `jb` : une rangée par ses clés `lq_row`/`h_row` ou sa rangée générée `jb_row`, sinon un `lq` ou nil) |

### §7, extrait (« En D et en 2 »)

> - **En D et en 2** : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée […], et à la sortie de phase.

## Annexe B — reproduire

```sh
P='\bpico8_value_[A-Za-z0-9_]+|\bPICO8_VALUE_[A-Z0-9_]+|pico8_value\.h'
sed -n 258,286p pico8/lua.lua                                            # js, js:pq, pz
sed -n 412,446p pico8/lua.lua                                            # jo:fe, jo:gh
grep -rnE '\b(lq_row|h_row|jb_row|has_h_|has_jb)\b|pico8_js_pq|pico8_js_new\(|pico8_js_keys_t|\.(lq|h_|jb) = ' src tests
grep -nE "$P" src/pico8_ui_button.c src/pico8_ui_button.h src/pico8_menu.c src/pico8_help.c \
  src/pico8_msgbox.c src/pico8_planet_display.c src/pico8_score_panel_display.c
grep -nE "$P" tests/test_pico8_js.c tests/test_pico8_lq.c
grep -n 'zero_union\|vec_ref\|must_abort\|stops_on\|g_off_row\|button_on_other' tests/test_pico8_js.c tests/test_pico8_lq.c
grep -nE 'static int [a-z0-9_]*truthy\(pico8_value_t' src/*.c             # 1 attendu au départ (ui_button)
sh tools/data_residue.sh --detail | grep -E 'ui_button|menu|help|msgbox|planet_display|score_panel_display|test_pico8_(js|lq)\b'
```
