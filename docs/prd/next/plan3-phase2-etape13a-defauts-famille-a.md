---
title: "Plan 3, étapes 2.13a1 et 2.13a2 : la famille a lit son archétype (`bg`, `ui`, `js`, `fq`, `nf`, `mh`, `iv`, `hp`, `hy`, puis `qb`, `ie`, `pj`, `pj_cr`) ; `défauts` 669 → 603 → 536, pied 1073 / 85 → 1007 / 78 → 940 / 74 — deux commits, jeu identique"
description: "Chaque instance de la famille a reçoit un `const pico8_X_def_t *def` posé par son `_new`. Les champs A quittent l'instance et se lisent par `self->def->X`, les champs I sont copiés depuis `def`, et les `pico8_X_start` de la famille disparaissent. Le test ponctuel garde, par classe, les seules comparaisons `me`/`t`, dont la chaîne (`pico8_ke_start`) vit jusqu'en 2.13e2. Deux valeurs n'avaient que lui pour filet (`ui.hw`, `iv.fa`) : elles reçoivent un TU durable. Essayé sur copie : vert, 26 e2e, mutations au parent et à l'enfant sans ÉCHEC."
status: needs-triage
---

## Problem Statement

**Écrit sur `8e2f685`** (branche `claude/plan-pilot-3-7zdft0`), pendant que 2.12e2 s'implémente dans l'arbre de travail. Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier. Les essais ont été faits sur une seule copie jetable (scratchpad `prd213a/`, supprimée depuis). Sur cette copie, `8e2f685` a reçu les deux diffs de 2.12e (PRD `docs/prd/triage/plan3-phase2-etape12e-archetypes-famille-e.md`, blocs `diff` des sections 2.12e1 et 2.12e2, `git apply` sans erreur), commités `3e14654` (e1) et `762f2f2` (e2). Notation : `p` = `762f2f2`, l'arbre d'après 2.12e2 ; `a1` = `366d450`, `a2` = `20b4078`, les deux commits de la copie qui portent le texte de « Ce qui change ».

Vu du mainteneur du port : 2.12 a créé les 26 archétypes et un test ponctuel, `tests/test_pico8_archetype_chain.c`, qui prouve, champ par champ, que chaque archétype vaut ce que la chaîne d'init laisse dans une instance fraîche. 2.13 bascule les lectures. Chaque instance pointe son archétype, et les `pico8_X_start` disparaissent. 2.13a1 et 2.13a2 le font pour la famille a : treize racines simples, sans masquage.

### Texte du plan, recopié (`docs/plan-3.md` à `8e2f685`, ligne 2.13)

> - 2.13 — l'init lit l'archétype, les `pico8_X_start(_t)` et la chaîne d'init disparaissent. Chaque instance reçoit `def`, posé par son `_new`. L'init copie la part I (étude, §2), et les lectures des champs A passent par `self->def`. Les champs A quittent l'instance. Les objets statiques `g_jo_kw_shared`, `g_ga_by_shared`, `g_shared_le`, `g_nf_ra`, `g_mz_jj_wrapper` restent posés par l'init. La fonction d'une famille dans le test ponctuel part dans le commit qui retire sa chaîne, après être passée à 2.12, et le TU disparaît en 2.13e2. […] Les TU `jo_spec_defaults`, `spec_defaults` et `entity_kind` sont réécrits pour comparer l'instance à son `def`. Les écritures de TU sur un champ A sont réécrites. […] Les comptes d'arrivée sont attribués par les noms `pico8_X_start` de chaque ligne du `--check`. Preuves : chemin touché (`gcov` des inits, mutations : défaut d'archétype changé → e2e ou TU rouge) ; règle 1 : un champ A qui quitte l'instance a sa preuve `const`, rejouée sur la copie du commit ; `game_data.h` mesuré au commit. `class-coverage-net` : oui. `sanitize-net` : oui. Budget ≈ 7 × 50 min.
>   - 2.13a1 — `bg`, `ui`, `js`, `fq`, `nf`, `mh`, `iv`, `hp`, `hy`. Compte : `défauts` → **603**.
>   - 2.13a2 — `qb`, `ie`, `pj`, `pj_cr` (les trois A dans l'initialiseur de `g_pj_cr`). Compte : → **536**.

Étude 2.9 (`docs/etude-2-9-archetypes.md`) : §2 « Chemin de lecture » (un `const pico8_X_def_t *def` dans l'instance, posé par l'init, sans cast ni aiguillage) ; §2 « Part copiée à l'init » (famille a : `js.fa`, `qb.ma`/`p`/`ra`/`v`, `mh.rh`, `iv.l`, `ie.ra`, `pj.fa`, `hy.nz`/`ew`/`lo`/`yr` ; `ui.le` et `nf.ra` restent posés par l'init) ; §3 (test ponctuel) ; §4 (comptes `défauts` par famille : a1 66, a2 67).

Règles communes : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte (`*_HASH`, `FROZEN_*`) changée.

### Réserves des vérificateurs, et ce que ce PRD en fait

1. **Filet unique.** Une valeur que seul le test ponctuel garde doit recevoir un TU durable avant que sa comparaison parte. Sinon, la mutation « défaut changé » est rouge au parent (test ponctuel) et verte à l'enfant : ÉCHEC par la règle du parent. Pour la famille a, les 37 jumelles de chaîne ont été rejouées au parent (preuve 3). **Deux** valeurs n'ont que le test ponctuel : `ui.hw` (P6) et `iv.fa` (P13). 2.13a1 leur donne chacune un `assert` à littéral dans leur TU de classe, prouvé à l'enfant (A6, A13). Les trois valeurs connues des autres familles sont traitées dans le « Contrat pour 2.13b à 2.13e2 ».
2. **Empreintes.** Aucune constante ne change. Trois TU plient une valeur de la famille a qui passait par un `_start` ou un membre A retiré. Le pli garde ses octets s'il lit la même valeur par `def` : `test_pico8_ie.c` (`fold_fixed(h, ie->def->fa)`, `fold_vec(h, &ie->def->eh)`, `fold_kq` sur `&…->def->kq`), `test_pico8_pj.c` (`fold_ref_member(h, ju->def->gs)`, `ju->def->i_`, sous `BUILT_HASH`) et `test_pico8_hp.c` (`DEFAULTS_HASH` plie la rangée `fk`, prise sur `b->def->fk`). Mesuré : vert aux deux commits.
3. **`pico8_hy_def`** reste dans `economy.[ch]`. Son type monte au-dessus de `pico8_hy_t`, qui le pointe. `economy.h` n'inclut toujours pas `game_data.h`.
4. **Pas de cast ni d'aiguillage par classe.** Mesuré : casts `\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)` dans `src/` + `tests/` : 532 à `p`, `a1` et `a2`. `->cls|header.cls|_struct_class()` : 707 aux trois. Les seules lignes « cast » du diff sont des accesseurs existants (`((const pico8_bg_t *)self)->fa` → `…->def->fa`).

### Question ouverte chez l'humain (Q-2.10, liens du noyau)

**2.13a n'en dépend pas.** `def` est un membre de l'instance, pas un lien du noyau. Aucun accesseur Q6 ne change de forme : `bg_get_fa`, `ui_get_ql`… lisent `self->def->X` au lieu de `self->X`. Les liens du noyau (2.14/2.15) viseront `&def->X` quelle que soit la réponse, ou n'existeront pas.

### Travail en parallèle

| Fichier | 2.13a1 | 2.13a2 | Recouvrement |
|---|---|---|---|
| `src/pico8_game_data.[ch]`, `tests/test_pico8_archetype_chain.c`, `tests/test_pico8_spec_defaults.c` | oui | oui | a2 part de a1 : **a1 d'abord** ; 2.12e2 écrit les deux premiers : **partir de son commit** |
| `docs/plan-3.md` | ligne 2.13a1 | ligne 2.13a2 | `git add -p` (seul le bloc de la sous-étape), jamais `git commit -a` |
| `tmp/` | `make test`, mutations | idem | **jamais deux `make` dans le même arbre en même temps** |

### État de départ attendu (le commit 2.12e2, ou un descendant qui ne touche que `docs/`)

Mesuré sur `p` :

| Commande | Résultat |
|---|---|
| `make composition-residue` | `parente 10 3`, `lien-parent 111 27`, `accesseurs-q6 237 33`, `défauts 669 52`, `void-ptr 8 4`, `inclusion-mutuelle 17 16` ; **`résidu 2 : 1073 ligne(s), 85 fichier(s)`** |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 282 ; 0 écriture : 15 ; 0 hors constructeur : 20` |
| `--check inclusion-mutuelle \| tail -1` | `résidu gardé (inclusion-mutuelle) : 43 ligne(s)` |
| `wc -l < src/pico8_game_data.h` | 751 |
| `grep -c 'same(' tests/test_pico8_archetype_chain.c` ; total asserté | `g_compared == 438` |
| `make test` ; `make e2e` | vert ; 26 conformes |
| attribution des 669 lignes `défauts` par nom `pico8_X_start` (script, preuve 1) | a1 66, a2 67, b 29, c 104, d 179, e1 127, e2 97 |

**2.12e2 est arrivée pendant l'écriture** : `5922ab8` (2.12e1 `2418a39`). Contrôlé par index temporaire : `src/` et `tests/` de `5922ab8` sont identiques à `p` (`git diff --stat` vide), donc les diffs ci-dessous s'appliquent tels quels sur `5922ab8` ou son descendant `05daecc`.

Un départ autre : arrêt, rapporté. **Si 2.12e2 n'a pas été commitée à l'identique de son PRD** (`git diff 762f2f2 <commit 2.12e2> -- src tests` non vide dans une copie qui rejoue le PRD de 2.12e), les diffs ci-dessous s'appliquent par ancres de texte, pas par `git apply` : les seuls textes de 2.12e qu'ils touchent sont `extern const pico8_jo_def_t …` (contexte) et la ligne du total du test ponctuel.

## Solution

Deux commits.

**2.13a1** (`bg`, `ui`, `js`, `fq`, `nf`, `mh`, `iv`, `hp`, `hy`) :
- neuf membres `const pico8_X_def_t *def`, chacun posé une seule fois, dans le `_new` de sa classe, à `&pico8_X_def` ;
- les douze champs A de la famille quittent l'instance et se lisent par `self->def->X` : `bg.fa`/`hd`/`pl`, `ui.ql`/`fa`/`hw`, `fq.fa`/`hd`, `mh.fa`, `iv.fa`/`v`, `hp.fk`. Un vecteur ou une boîte A se lit `&self->def->X` là où un pointeur était attendu ;
- les I sont copiés depuis `def` : `js.fa`, `mh.rh`, `iv.l`, `hy.nz`/`ew`/`lo`/`yr`. `ui.le` (`&g_shared_le`) et `nf.ra` (`&g_nf_ra`) restent posés par l'init. `g_nf_ra` est remis à zéro par `pico8_nf_init`, au lieu de recopier `pico8_nf_start.ra` (`{0, 0}`) ;
- les huit `pico8_X_start_t`, leurs valeurs et leurs `extern` sortent (`hy` n'en a pas) ;
- les TU qui lisaient un champ A ou un `_start` de ces classes sont réécrits ; `spec_defaults` compare l'instance à son `def` ; `ui.hw` et `iv.fa` reçoivent un littéral (réserve 1) ;
- test ponctuel : les fonctions des neuf classes perdent tout sauf `me`/`t`. `test_hp` et `test_hy` (sans `me`/`t`) partent. Total **438 → 419**.

**2.13a2** (`qb`, `ie`, `pj`, `pj_cr`) :
- trois `def` (`qb`, `ie`, `pj`) ; six A quittent l'instance (`qb.pc`, `ie.kq`/`fa`/`eh`, `pj.gs`/`i_`) ; I copiés : `qb.v`/`p`/`ra` (pointés dans l'archétype : `self->v = &self->def->v`), `qb.ma`, `ie.ra` (`&self->def->ra`), `pj.fa` ;
- `pj_cr` : ses trois A (`hi`, `gq`, `ca`) entrent dans l'initialiseur statique de `g_pj_cr`. Les deux vecteurs sont des littéraux composés de portée fichier (durée statique, `&(const pico8_vec_t){0x0, 0xb0000}`), et `pico8_score_panel_init` n'écrit plus `ca` ;
- ce que `pico8_ie_start`/`pico8_pj_start` portaient hors archétype : `ie.id` (0) devient le littéral de `pico8_research_icon_init` ; `pj.ou`/`pj.ea` se lisent directement sur `pico8_pj_ou_row`/`pico8_pj_ea_row` ; `pj.ed` (1), que `src/` ne lit jamais, ne survit que dans `ed_fixed` du TU `pj` ;
- les quatre `pico8_X_start_t` (`qb`, `ie`, `pj_cr`, `pj`), leurs valeurs et leurs `extern` sortent ;
- test ponctuel : `test_qb`, `test_ie`, `test_pj` ne gardent que `me`/`t`. Total **419 → 407**.

Rien d'autre : ni `tools/`, ni `Makefile`, ni `glossaire-noms.md` (aucun nom minifié neuf ; `def` n'est pas minifié), ni `trous-de-portage.md`, ni `tests/e2e/`, ni `*.sha256`.

## User Stories

1. En tant que mainteneur, je veux que chaque instance de la famille a lise ses défauts sur son archétype, afin que les `pico8_X_start` de la famille disparaissent sans changer une valeur du jeu.
2. En tant que gardien de la règle 1, je veux que les champs A quittent l'instance et que `def` soit `const` et posé une seule fois, afin qu'aucune écriture hors construction ne soit possible.
3. En tant que gardien de la règle 3, je veux que le test ponctuel ne garde que ce dont la chaîne existe encore (`me`/`t`, par `pico8_ke_start`), afin qu'il ne compare pas un archétype à lui-même.
4. En tant que gardien de la règle du parent, je veux qu'aucune valeur ne perde son dernier filet quand sa comparaison part, afin qu'une mutation d'archétype rouge au parent le reste à l'enfant.
5. En tant que gardien des empreintes, je veux qu'aucune constante `*_HASH` ne change, afin que les plis réécrits prouvent la même lecture.
6. En tant que gardien du compteur, je veux `défauts` 603 puis 536, sans nouvelle paire d'inclusion, afin que 2.13b parte des bons comptes.
7. En tant qu'auteur de 2.13b à 2.13e2, je veux trouver ici ce que la famille a laisse au test ponctuel et les filets uniques connus des autres familles, afin de ne pas les perdre.

## Implementation Decisions

### Modules

2.13a1 : `game_data`, `bg`, `ui`, `ui_button` (`js`, `fq`), `mouse_reader` (`nf`), `msgbox` (`mh`, `iv`), `research` (`hp`), `economy` (`hy`) ; TU `archetype_chain`, `bg`, `fq`, `g_`, `hp`, `iv`, `mh`, `spec_defaults`, `ui` ; `docs/plan-3.md`.
2.13a2 : `game_data`, `scroll` (`qb`), `research_icon` (`ie`), `score_panel` (`pj`, `pj_cr`) ; TU `archetype_chain`, `ie`, `pj`, `qb`, `spec_defaults` ; `docs/plan-3.md`.

### Inclusions

`def` exige le type de l'archétype dans l'en-tête de la classe. Quatre en-têtes gagnent `#include "pico8_game_data.h"` : `ui.h`, `ui_button.h`, `mouse_reader.h` (a1), `research_icon.h` (a2). `game_data.h` n'inclut que `fixed`, `geom`, `map_tables`, `vec` : aucune paire possible. Mesuré : `inclusion-mutuelle` **17 / 16, 43 lignes** à `a1` et `a2`.

### Ce qui change

Les deux diffs ci-dessous sont le texte exact (`git apply` sur `p`, puis sur `a1`, passe). Lecture, en plus de la Solution :

- **Place de `def`** : premier membre après `header`, dans chaque struct d'instance (`struct pico8_js {`, `struct pico8_fq {`, `struct pico8_mh {` sont déclarées par étiquette ; même place).
- **Place de la pose** : là où le premier défaut `_start` était écrit dans le `_new`. `hp` : l'écriture `self->fk = NULL;` du début du `_new` part avec le membre ; `def` est posé à la place de `self->fk = pico8_hp_start.fk;`. `hy` : `def` est posé juste après `header.cls`, et la seconde série de littéraux de `pico8_hy_struct_new` (100, 1, 1, 3401) devient `self->X = self->def->X`. La première série (tout à 0) ne change pas.
- **Lecteurs qui prenaient un pointeur** : `pico8_ui_hw()` rend `&pico8_ui_def.hw` ; `pico8_ie_eh()` rend `&pico8_ie_def.eh` ; `ie_get_kq` rend `&…->def->kq` ; `pico8_iv_ph` lit `&self->def->v` ; `pico8_hp_fk()`, `pico8_pj_i_()`, `pico8_pj_gs()` lisent `pico8_X_def`.
- **Commentaires** : toute mention d'un `_start` retiré est réécrite ou retirée (elle compterait en `défauts`) : `bg.h`, `ui.h` (×2), `ui_button.h`, `mouse_reader.c`, `msgbox.h` (×2), `research.h`, `scroll.h`, `research_icon.h` (×3), `score_panel.h` (×2), et les en-têtes de TU `ui`, `pj`, `ie`, `iv`, `spec_defaults`. Les commentaires qui décrivaient où pointait un champ A retiré partent avec lui.
- **Littéraux à zéro** (`g_nf_ra`, `g_ie_id`) : valeurs 0 d'objets statiques de module, hors archétype (étude, §2) ; leur jumelle au parent (P10, P30) ne rougit pas le test ponctuel, qui ne les comparait pas.
- **Test ponctuel** : les lignes `same(…)` des champs A et I de la famille partent, les fonctions restent pour `me`/`t` (voir « Test ponctuel » plus bas).

#### 2.13a1 (`p` → `a1`)

```diff
diff --git a/src/pico8_bg.c b/src/pico8_bg.c
index 055cae5..55de6d8 100644
--- a/src/pico8_bg.c
+++ b/src/pico8_bg.c
@@ -42,7 +42,7 @@ static void pico8_bg_hq_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
 }
 
 static int bg_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_bg_t *)self)->fa;
+    *out = ((const pico8_bg_t *)self)->def->fa;
     return 1;
 }
 
@@ -67,9 +67,7 @@ static pico8_bg_t *pico8_bg_struct_new(void) {
     pico8_bg_t *self = malloc(sizeof *self);
 
     pico8_ke_struct_init(&self->header, &g_bg_struct_class);
-    self->fa = pico8_bg_start.fa;
-    self->hd = pico8_bg_start.hd;
-    self->pl = pico8_bg_start.pl;
+    self->def = &pico8_bg_def;
     self->mq = NULL;
     self->m_ = 0;
     self->has_m_ = 0;
@@ -109,7 +107,7 @@ void pico8_bg_pq(pico8_mem_t *mem, pico8_bg_t *self) {
     int vs_set[32];
     pico8_fixed_t vh = pico8_fixed_from_int(0);
     pico8_jf_state_t *oh;
-    const pico8_int_seq_t *pl = self != NULL && self->pl != NULL ? self->pl : &pico8_bg_pl_seq;
+    const pico8_int_seq_t *pl = self != NULL && self->def->pl != NULL ? self->def->pl : &pico8_bg_pl_seq;
     int x, y;
 
     if (mem == NULL) {
diff --git a/src/pico8_bg.h b/src/pico8_bg.h
index dc68351..65455af 100644
--- a/src/pico8_bg.h
+++ b/src/pico8_bg.h
@@ -13,12 +13,10 @@
  * crashing. `hd=1` is flat data, not a method: `bg` always answers the hover
  * test, since nothing lies behind the background to miss. */
 
-/* `pl` points at `pico8_bg_pl_seq`. Nothing frees the struct: TROU-7. */
+/* Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_struct_header_t header;
-    pico8_fixed_t fa;
-    pico8_fixed_t hd;
-    const pico8_int_seq_t *pl;
+    const pico8_bg_def_t *def;
     pico8_vec_t *mq;
     int m_;
     int has_m_;
diff --git a/src/pico8_economy.c b/src/pico8_economy.c
index 993023f..8c7b5f4 100644
--- a/src/pico8_economy.c
+++ b/src/pico8_economy.c
@@ -31,6 +31,7 @@ static pico8_hy_t *pico8_hy_struct_new(void) {
     pico8_ptrs_t *invented;
 
     self->header.cls = &g_hy_struct_class;
+    self->def = &pico8_hy_def;
     self->nz = pico8_fixed_from_int(0);
     self->ge = pico8_fixed_from_int(0);
     self->jk = pico8_fixed_from_int(0);
@@ -47,16 +48,16 @@ static pico8_hy_t *pico8_hy_struct_new(void) {
     self->hl = NULL;
     self->has_hl = 0;
 
-    self->nz = pico8_fixed_from_int(100);
+    self->nz = self->def->nz;
     self->ge = pico8_fixed_from_int(0);
     self->jk = pico8_fixed_from_int(0);
-    self->ew = pico8_fixed_from_int(1);
+    self->ew = self->def->ew;
     /* TROU-7 */
     invented = malloc(sizeof *invented);
     pico8_ptrs_init(invented);
     self->invented = invented;
-    self->lo = pico8_fixed_from_int(1);
-    self->yr = pico8_fixed_from_int(3401);
+    self->lo = self->def->lo;
+    self->yr = self->def->yr;
 
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("hy");
diff --git a/src/pico8_economy.h b/src/pico8_economy.h
index efb5472..aa2ee5d 100644
--- a/src/pico8_economy.h
+++ b/src/pico8_economy.h
@@ -9,6 +9,15 @@
 
 /* Throughout this module a NULL `self` is a safe no-op. */
 
+/* hy's archetype, here and not with the others: economy.h does not include
+ * pico8_game_data.h. */
+typedef struct {
+    pico8_fixed_t nz;
+    pico8_fixed_t ew;
+    pico8_fixed_t lo;
+    pico8_fixed_t yr;
+} pico8_hy_def_t;
+
 /* The economy, one per game. Nothing here frees it: pico8_probe_set_fm()
  * and pico8_probe_shutdown() free the struct alone, never `invented`
  * (TROU-7). `po`/`fu` are NULL before the first pico8_hy_lj(), then point
@@ -16,6 +25,7 @@
  * absent before the first pico8_hy_lj(). */
 typedef struct {
     pico8_struct_header_t header;
+    const pico8_hy_def_t *def;
     pico8_fixed_t nz;
     pico8_fixed_t ge;
     pico8_fixed_t jk;
@@ -33,15 +43,6 @@ typedef struct {
     int has_hl; /* 0: `hl` is nil; 1 with `hl` NULL: `hl` is false */
 } pico8_hy_t;
 
-/* hy's archetype, here and not with the others: economy.h does not include
- * pico8_game_data.h. */
-typedef struct {
-    pico8_fixed_t nz;
-    pico8_fixed_t ew;
-    pico8_fixed_t lo;
-    pico8_fixed_t yr;
-} pico8_hy_def_t;
-
 extern const pico8_hy_def_t pico8_hy_def;
 
 /* Marks the module ready only. The singleton `fm=hy()` is built later, by
diff --git a/src/pico8_game_data.c b/src/pico8_game_data.c
index fa0dac4..0a557bd 100644
--- a/src/pico8_game_data.c
+++ b/src/pico8_game_data.c
@@ -298,27 +298,6 @@ const pico8_ke_start_t pico8_ke_start = {
     .t = 0x0,
 };
 
-const pico8_bg_start_t pico8_bg_start = {
-    .fa = 0x10000,
-    .hd = 0x10000,
-    .pl = &pico8_bg_pl_seq,
-};
-
-const pico8_ui_start_t pico8_ui_start = {
-    .ql = 0x3a0000,
-    .fa = 0xf0000,
-    .hw = {0xa0000, 0xa0000, 0x760000, 0x760000},
-};
-
-const pico8_js_start_t pico8_js_start = {
-    .fa = 0xb0000,
-};
-
-const pico8_fq_start_t pico8_fq_start = {
-    .fa = 0xe0000,
-    .hd = 0x10000,
-};
-
 const pico8_qb_start_t pico8_qb_start = {
     .v = {0x0, 0x0},
     .ma = 0x0,
@@ -327,21 +306,6 @@ const pico8_qb_start_t pico8_qb_start = {
     .pc = &pico8_qb_pc_vecs,
 };
 
-const pico8_nf_start_t pico8_nf_start = {
-    .ra = {0x0, 0x0},
-};
-
-const pico8_mh_start_t pico8_mh_start = {
-    .fa = 0xe0000,
-    .rh = 0x50000,
-};
-
-const pico8_iv_start_t pico8_iv_start = {
-    .fa = 0x90000,
-    .l = 0x0,
-    .v = {0x0, -0x570a},
-};
-
 const pico8_mz_start_t pico8_mz_start = {
     .me = "kz",
     .dw = 0x0,
@@ -410,10 +374,6 @@ const pico8_fv_start_t pico8_fv_start = {
     .rb = &pico8_fv_rb_row,
 };
 
-const pico8_hp_start_t pico8_hp_start = {
-    .fk = &pico8_hp_fk_row,
-};
-
 const pico8_ie_start_t pico8_ie_start = {
     .kq = {0x0, 0x0, 0x90000, 0x80000},
     .fa = 0xc0000,
diff --git a/src/pico8_game_data.h b/src/pico8_game_data.h
index 393cb85..8f924e0 100644
--- a/src/pico8_game_data.h
+++ b/src/pico8_game_data.h
@@ -119,27 +119,6 @@ typedef struct {
     pico8_fixed_t t;
 } pico8_ke_start_t;
 
-typedef struct {
-    pico8_fixed_t fa;
-    pico8_fixed_t hd;
-    const pico8_int_seq_t *pl;
-} pico8_bg_start_t;
-
-typedef struct {
-    pico8_fixed_t ql;
-    pico8_fixed_t fa;
-    pico8_qz_box_t hw;
-} pico8_ui_start_t;
-
-typedef struct {
-    pico8_fixed_t fa;
-} pico8_js_start_t;
-
-typedef struct {
-    pico8_fixed_t fa;
-    pico8_fixed_t hd;
-} pico8_fq_start_t;
-
 typedef struct {
     pico8_vec_t v;
     pico8_fixed_t ma;
@@ -148,21 +127,6 @@ typedef struct {
     const pico8_vec_seq_t *pc;
 } pico8_qb_start_t;
 
-typedef struct {
-    pico8_vec_t ra;
-} pico8_nf_start_t;
-
-typedef struct {
-    pico8_fixed_t fa;
-    pico8_fixed_t rh;
-} pico8_mh_start_t;
-
-typedef struct {
-    pico8_fixed_t fa;
-    pico8_fixed_t l;
-    pico8_vec_t v;
-} pico8_iv_start_t;
-
 typedef struct {
     const char *me;
     pico8_fixed_t dw;
@@ -234,10 +198,6 @@ typedef struct {
     const pico8_jb_row_t *rb;
 } pico8_fv_start_t;
 
-typedef struct {
-    const pico8_jb_row_t *fk;
-} pico8_hp_start_t;
-
 typedef struct {
     pico8_qz_box_t kq;
     pico8_fixed_t fa;
@@ -667,19 +627,11 @@ extern const pico8_pj_cr_start_t pico8_pj_cr_start;
 /* lq, jb, hy and trade have an empty spec, hence no object; ui.le, nf.l and
  * nf.r are empty tables, held by their module. */
 extern const pico8_ke_start_t pico8_ke_start;
-extern const pico8_bg_start_t pico8_bg_start;
-extern const pico8_ui_start_t pico8_ui_start;
-extern const pico8_js_start_t pico8_js_start;
-extern const pico8_fq_start_t pico8_fq_start;
 extern const pico8_qb_start_t pico8_qb_start;
-extern const pico8_nf_start_t pico8_nf_start;
-extern const pico8_mh_start_t pico8_mh_start;
-extern const pico8_iv_start_t pico8_iv_start;
 extern const pico8_mz_start_t pico8_mz_start;
 extern const pico8_jc_start_t pico8_jc_start;
 extern const pico8_g__start_t pico8_g__start;
 extern const pico8_fv_start_t pico8_fv_start;
-extern const pico8_hp_start_t pico8_hp_start;
 /* ie's class keys, and the kq, ra, eh every icon points at. */
 extern const pico8_ie_start_t pico8_ie_start;
 extern const pico8_pj_start_t pico8_pj_start;
diff --git a/src/pico8_mouse_reader.c b/src/pico8_mouse_reader.c
index f9a7192..52eb2ac 100644
--- a/src/pico8_mouse_reader.c
+++ b/src/pico8_mouse_reader.c
@@ -14,7 +14,7 @@
 static int g_nf_up = 0;
 static pico8_nf_button_t g_nf_l;
 static pico8_nf_button_t g_nf_r;
-/* Not `&pico8_nf_start.ra`: two TUs write `je.ra` in place. */
+/* Not in the archetype: two TUs write `je.ra` in place. */
 static pico8_vec_t g_nf_ra;
 
 /* dg() dispatches the per-tick state method as `qc[qc.me](qc,qc.t)`, so
@@ -44,6 +44,7 @@ static pico8_nf_t *pico8_nf_struct_new(void) {
     pico8_nf_t *self = malloc(sizeof *self);
 
     pico8_ke_struct_init(&self->header, &g_nf_struct_class);
+    self->def = &pico8_nf_def;
     self->ra = &g_nf_ra;
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("nf");
@@ -58,7 +59,7 @@ void pico8_nf_init(void) {
     pico8_pi_init();
     assert(pico8_ke_ready() && "nf descends from ke");
     g_nf_up = 1;
-    g_nf_ra = pico8_nf_start.ra;
+    g_nf_ra = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
     memset(&g_nf_l, 0, sizeof g_nf_l);
     memset(&g_nf_r, 0, sizeof g_nf_r);
 }
diff --git a/src/pico8_mouse_reader.h b/src/pico8_mouse_reader.h
index a87ba61..e70fcd3 100644
--- a/src/pico8_mouse_reader.h
+++ b/src/pico8_mouse_reader.h
@@ -1,6 +1,7 @@
 #ifndef PICO8_MOUSE_READER_H
 #define PICO8_MOUSE_READER_H
 
+#include "pico8_game_data.h"
 #include "pico8_input.h"
 #include "pico8_ke.h"
 #include "pico8_mem.h"
@@ -16,6 +17,7 @@
  * TROU-7. */
 typedef struct {
     pico8_struct_header_t header;
+    const pico8_nf_def_t *def;
     pico8_vec_t *ra;
 } pico8_nf_t;
 
diff --git a/src/pico8_msgbox.c b/src/pico8_msgbox.c
index adba8d7..fb95c6e 100644
--- a/src/pico8_msgbox.c
+++ b/src/pico8_msgbox.c
@@ -86,7 +86,7 @@ void pico8_mh_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_mh_t *self) {
 }
 
 static int mh_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_mh_t *)self)->fa;
+    *out = ((const pico8_mh_t *)self)->def->fa;
     return 1;
 }
 
@@ -108,8 +108,8 @@ static pico8_mh_t *pico8_mh_struct_new(const pico8_mh_keys_t *keys) {
     int i;
 
     pico8_ke_struct_init(&self->header, &g_mh_struct_class);
-    self->fa = pico8_mh_start.fa;
-    self->rh = pico8_mh_start.rh;
+    self->def = &pico8_mh_def;
+    self->rh = self->def->rh;
     self->lh = NULL;
     self->p_ = 0;
     self->has_p_ = 0;
@@ -186,7 +186,7 @@ void pico8_iv_ph(pico8_iv_t *self) {
         return;
     }
 
-    new_ra = pico8_vec_add(pico8_iv_vec_or_zero(self->ra), pico8_iv_vec_or_zero(self->v));
+    new_ra = pico8_vec_add(pico8_iv_vec_or_zero(self->ra), pico8_iv_vec_or_zero(&self->def->v));
     /* TROU-7 */
     heap_ra = malloc(sizeof *heap_ra);
     *heap_ra = new_ra;
@@ -229,7 +229,7 @@ static void pico8_iv_kj_dispatch(void *self, const pico8_vec_t *ra) {
 }
 
 static int iv_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_iv_t *)self)->fa;
+    *out = ((const pico8_iv_t *)self)->def->fa;
     return 1;
 }
 
@@ -255,9 +255,8 @@ static pico8_iv_t *pico8_iv_struct_new(const pico8_iv_keys_t *keys) {
     pico8_iv_t *self = malloc(sizeof *self);
 
     pico8_ke_struct_init(&self->header, &g_iv_struct_class);
-    self->fa = pico8_iv_start.fa;
-    self->l = pico8_iv_start.l;
-    self->v = &pico8_iv_start.v;
+    self->def = &pico8_iv_def;
+    self->l = self->def->l;
     self->ra = NULL;
     self->p_ = 0;
     self->has_p_ = 0;
diff --git a/src/pico8_msgbox.h b/src/pico8_msgbox.h
index 9ed281f..2216de8 100644
--- a/src/pico8_msgbox.h
+++ b/src/pico8_msgbox.h
@@ -26,7 +26,7 @@ enum { PICO8_MH_ITEMS_CAP = PICO8_BZ_COUNT };
  * article stays shared with the caller. Nothing frees the struct: TROU-7. */
 struct pico8_mh {
     pico8_struct_header_t header;
-    pico8_fixed_t fa;
+    const pico8_mh_def_t *def;
     pico8_fixed_t rh;
     pico8_mh_lh_fn lh; /* NULL: `lh` is nil */
     int p_;
@@ -56,7 +56,7 @@ int pico8_mh_ready(void);
 typedef struct {
     const pico8_jb_row_t *const *items;
     int count;
-    int has_rh; /* 0: `rh` keeps `pico8_mh_start.rh` */
+    int has_rh; /* 0: `rh` keeps the archetype's */
     pico8_fixed_t rh;
     pico8_mh_lh_fn lh;
 } pico8_mh_keys_t;
@@ -77,13 +77,11 @@ void pico8_iv_init(void);
 /* `at1`/`at2` are the cart's `self[1]` (the text, a string never freed)
  * and `self[2]` (the forced color). `at1` NULL is nil; a missing `[2]`
  * leaves `at2` at 0, the color `kj` gave nil. Never freed, nor are the
- * vectors `ra` points at: TROU-7. `v` points at `pico8_iv_start.v`, shared
- * by every iv. */
+ * vectors `ra` points at: TROU-7. */
 typedef struct {
     pico8_struct_header_t header;
-    pico8_fixed_t fa;
+    const pico8_iv_def_t *def;
     pico8_fixed_t l;
-    const pico8_vec_t *v;
     pico8_vec_t *ra;
     int p_;
     int has_p_; /* 0: `p_` is nil, and `p_` is then never read */
diff --git a/src/pico8_research.c b/src/pico8_research.c
index 43222f8..f2b9e5a 100644
--- a/src/pico8_research.c
+++ b/src/pico8_research.c
@@ -135,7 +135,6 @@ static pico8_hp_t *pico8_hp_struct_new(const pico8_hp_keys_t *keys) {
     size_t c;
 
     self->header.cls = &g_hp_struct_class;
-    self->fk = NULL;
     self->px = NULL;
     self->lr = NULL;
     self->fj = 0;
@@ -156,7 +155,7 @@ static pico8_hp_t *pico8_hp_struct_new(const pico8_hp_keys_t *keys) {
     self->rn = 0;
     self->has_rn = 0;
     self->lq = NULL;
-    self->fk = pico8_hp_start.fk;
+    self->def = &pico8_hp_def;
 
     if (keys != NULL && keys->ri != NULL) {
         const pico8_fd_entry_t *ri = keys->ri;
@@ -256,7 +255,7 @@ const pico8_struct_class_t *pico8_hp_struct_class(void) {
 }
 
 const pico8_jb_row_t *pico8_hp_fk(void) {
-    return pico8_hp_start.fk;
+    return pico8_hp_def.fk;
 }
 
 pico8_hp_t *pico8_hp_new(const pico8_hp_keys_t *keys) {
diff --git a/src/pico8_research.h b/src/pico8_research.h
index 813c732..bb7f492 100644
--- a/src/pico8_research.h
+++ b/src/pico8_research.h
@@ -24,14 +24,13 @@ pico8_qe_t pico8_jy_qe(const pico8_jy_t *i);
 pico8_jy_t *pico8_jy_new(const pico8_fixed_t *qe);
 
 /* A research project of `gb`. Freed only by pico8_research_shutdown(),
- * through `gb`'s ranks. `fk` points at `pico8_hp_fk_row`; the other
- * members are borrowed from `pico8_fd_table` and from `r`, except `i`, a
- * list built here and never freed (TROU-7), and `lq`, which points at
- * `lq_own` (its third item `lq_inner`) or is NULL. The struct is never
- * copied: `lq` points into it. */
+ * through `gb`'s ranks. The members are borrowed from `pico8_fd_table` and
+ * from `r`, except `i`, a list built here and never freed (TROU-7), and
+ * `lq`, which points at `lq_own` (its third item `lq_inner`) or is NULL.
+ * The struct is never copied: `lq` points into it. */
 typedef struct {
     pico8_struct_header_t header;
-    const pico8_jb_row_t *fk;
+    const pico8_hp_def_t *def;
     const char *px;
     const char *lr;
     pico8_fixed_t fj;
diff --git a/src/pico8_ui.c b/src/pico8_ui.c
index 1f0470e..c8e4c1e 100644
--- a/src/pico8_ui.c
+++ b/src/pico8_ui.c
@@ -21,7 +21,7 @@ static pico8_ui_t *g_ok = NULL;
 static pico8_ents_t g_shared_le;
 
 const pico8_qz_box_t *pico8_ui_hw(void) {
-    return g_ui_up ? &pico8_ui_start.hw : NULL;
+    return g_ui_up ? &pico8_ui_def.hw : NULL;
 }
 
 pico8_ents_t *pico8_ui_le_shared(void) {
@@ -75,12 +75,12 @@ static void pico8_ui_ev_dispatch(void *self, const pico8_vec_t *ra) {
 }
 
 static int ui_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_ui_t *)self)->fa;
+    *out = ((const pico8_ui_t *)self)->def->fa;
     return 1;
 }
 
 static int ui_get_ql(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_ui_t *)self)->ql;
+    *out = ((const pico8_ui_t *)self)->def->ql;
     return 1;
 }
 
@@ -97,10 +97,8 @@ static pico8_ui_t *pico8_ui_struct_new(void) {
     assert(g_ui_up && "pico8_ui_init() must run first");
     self = malloc(sizeof *self);
     pico8_ke_struct_init(&self->header, &g_ui_struct_class);
-    self->ql = pico8_ui_start.ql;
-    self->fa = pico8_ui_start.fa;
+    self->def = &pico8_ui_def;
     self->le = &g_shared_le;
-    self->hw = &pico8_ui_start.hw;
     self->qa = NULL;
     self->ic = NULL;
     self->ic_own.layer_count = PICO8_UI_IC_LAYERS;
@@ -244,14 +242,14 @@ void pico8_ui_ev(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_ui_t *sel
     if (self->rheld != NULL) {
         spr_id = 9;
     } else if (self->o_ != NULL) {
-        pico8_fixed_t ql = self->ql;
+        pico8_fixed_t ql = self->def->ql;
 
         if (self->o_->cls->get_ql != NULL) {
             self->o_->cls->get_ql(self->o_, &ql);
         }
         spr_id = pico8_ui_to_int(ql);
     } else {
-        spr_id = pico8_ui_to_int(self->ql);
+        spr_id = pico8_ui_to_int(self->def->ql);
     }
 
     je = pico8_je_nf();
diff --git a/src/pico8_ui.h b/src/pico8_ui.h
index 6298f0a..c4ce707 100644
--- a/src/pico8_ui.h
+++ b/src/pico8_ui.h
@@ -3,6 +3,7 @@
 
 #include "pico8_bg.h"
 #include "pico8_ents.h"
+#include "pico8_game_data.h"
 #include "pico8_geom.h"
 #include "pico8_gfx.h"
 #include "pico8_ke.h"
@@ -25,15 +26,12 @@ typedef struct {
 
 /* TROU-7 */
 /* `le` of a new `ui` is `pico8_ui_le_shared()`, until `cj` or `pz`
- * replaces it; `hw` is `&pico8_ui_start.hw`. `ic` is NULL until
- * the first `gw`, then points at `ic_own`, which every `gw` empties and
- * refills. */
+ * replaces it. `ic` is NULL until the first `gw`, then points at `ic_own`,
+ * which every `gw` empties and refills. */
 typedef struct {
     pico8_struct_header_t header;
-    pico8_fixed_t ql;
-    pico8_fixed_t fa;
+    const pico8_ui_def_t *def;
     pico8_ents_t *le;
-    const pico8_qz_box_t *hw;
     pico8_vec_t *qa;
     pico8_ui_ic_t *ic;
     pico8_ui_ic_t ic_own;
@@ -58,7 +56,7 @@ int pico8_ui_ready(void);
 
 const pico8_struct_class_t *pico8_ui_struct_class(void);
 
-/* The box of the class, read by `bv`: `&pico8_ui_start.hw`, NULL before
+/* The box of the class, read by `bv`: `&pico8_ui_def.hw`, NULL before
  * `pico8_ui_init()` and after `pico8_ui_shutdown()`. */
 const pico8_qz_box_t *pico8_ui_hw(void);
 
diff --git a/src/pico8_ui_button.c b/src/pico8_ui_button.c
index 65afb82..1ba9241 100644
--- a/src/pico8_ui_button.c
+++ b/src/pico8_ui_button.c
@@ -132,7 +132,8 @@ static pico8_js_t *pico8_js_struct_new(const pico8_js_keys_t *keys) {
     self->_jo_fe_b = NULL;
     self->_jo_fe_od = 0;
     self->has__jo_fe_od = 0;
-    self->fa = pico8_js_start.fa;
+    self->def = &pico8_js_def;
+    self->fa = self->def->fa;
     if (keys != NULL) {
         if (keys->has_fa) {
             self->fa = keys->fa;
@@ -324,7 +325,7 @@ static pico8_down_t pico8_fq_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_v
 }
 
 static int fq_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_fq_t *)self)->fa;
+    *out = ((const pico8_fq_t *)self)->def->fa;
     return 1;
 }
 
@@ -353,8 +354,7 @@ static pico8_fq_t *pico8_fq_struct_new(const pico8_fq_keys_t *keys) {
     pico8_fq_t *self = malloc(sizeof *self);
 
     pico8_ke_struct_init(&self->header, &g_fq_struct_class);
-    self->fa = pico8_fq_start.fa;
-    self->hd = pico8_fq_start.hd;
+    self->def = &pico8_fq_def;
     self->my = NULL;
     self->p_ = 0;
     self->has_p_ = 0;
diff --git a/src/pico8_ui_button.h b/src/pico8_ui_button.h
index 6c06e06..5d0e809 100644
--- a/src/pico8_ui_button.h
+++ b/src/pico8_ui_button.h
@@ -1,6 +1,7 @@
 #ifndef PICO8_UI_BUTTON_H
 #define PICO8_UI_BUTTON_H
 
+#include "pico8_game_data.h"
 #include "pico8_geom.h"
 #include "pico8_gfx.h"
 #include "pico8_ke.h"
@@ -26,6 +27,7 @@ typedef void (*pico8_js_fn)(pico8_js_t *self);
  * Nothing frees the struct: TROU-7. */
 struct pico8_js {
     pico8_struct_header_t header;
+    const pico8_js_def_t *def;
     pico8_fixed_t fa;
     pico8_lq_t *lq;
     pico8_fixed_t mb;
@@ -66,7 +68,7 @@ const pico8_struct_class_t *pico8_js_struct_class(void);
  * `lq` `lq_copy` are read during the construction, not kept. `js.lq`: the
  * row `lq_row`, else a copy of `lq_copy`, else an empty row. */
 typedef struct {
-    int has_fa; /* 0: `fa` keeps `pico8_js_start.fa` */
+    int has_fa; /* 0: `fa` keeps the archetype's */
     pico8_fixed_t fa;
     const pico8_lq_t *lq_copy;
     const pico8_lq_keys_t *lq_row;
@@ -120,8 +122,7 @@ typedef int (*pico8_fq_my_fn)(pico8_fq_t *self);
  * a field, not a method. Nothing frees the struct: TROU-7. */
 struct pico8_fq {
     pico8_struct_header_t header;
-    pico8_fixed_t fa;
-    pico8_fixed_t hd;
+    const pico8_fq_def_t *def;
     pico8_fq_my_fn my;
     int p_;
     int has_p_;
diff --git a/tests/test_pico8_archetype_chain.c b/tests/test_pico8_archetype_chain.c
index 8b1899b..6d285ca 100644
--- a/tests/test_pico8_archetype_chain.c
+++ b/tests/test_pico8_archetype_chain.c
@@ -77,9 +77,6 @@ static void test_bg(void) {
 
     same(same_text(bg->header.me, pico8_bg_def.me), "bg.me");
     same(bg->header.t == pico8_bg_def.t, "bg.t");
-    same(bg->fa == pico8_bg_def.fa, "bg.fa");
-    same(bg->hd == pico8_bg_def.hd, "bg.hd");
-    same(bg->pl == pico8_bg_def.pl, "bg.pl");
 }
 
 /* `le` is the module's shared list, not the archetype's. */
@@ -88,9 +85,6 @@ static void test_ui(void) {
 
     same(same_text(ui->header.me, pico8_ui_def.me), "ui.me");
     same(ui->header.t == pico8_ui_def.t, "ui.t");
-    same(ui->ql == pico8_ui_def.ql, "ui.ql");
-    same(ui->fa == pico8_ui_def.fa, "ui.fa");
-    same(same_box(ui->hw, &pico8_ui_def.hw), "ui.hw");
     assert(ui->le == pico8_ui_le_shared());
 }
 
@@ -99,7 +93,6 @@ static void test_js(void) {
 
     same(same_text(js->header.me, pico8_js_def.me), "js.me");
     same(js->header.t == pico8_js_def.t, "js.t");
-    same(js->fa == pico8_js_def.fa, "js.fa");
 }
 
 static void test_fq(void) {
@@ -107,8 +100,6 @@ static void test_fq(void) {
 
     same(same_text(fq->header.me, pico8_fq_def.me), "fq.me");
     same(fq->header.t == pico8_fq_def.t, "fq.t");
-    same(fq->fa == pico8_fq_def.fa, "fq.fa");
-    same(fq->hd == pico8_fq_def.hd, "fq.hd");
 }
 
 static void test_qb(void) {
@@ -139,8 +130,6 @@ static void test_mh(void) {
 
     same(same_text(mh->header.me, pico8_mh_def.me), "mh.me");
     same(mh->header.t == pico8_mh_def.t, "mh.t");
-    same(mh->fa == pico8_mh_def.fa, "mh.fa");
-    same(mh->rh == pico8_mh_def.rh, "mh.rh");
 }
 
 static void test_iv(void) {
@@ -148,16 +137,6 @@ static void test_iv(void) {
 
     same(same_text(iv->header.me, pico8_iv_def.me), "iv.me");
     same(iv->header.t == pico8_iv_def.t, "iv.t");
-    same(iv->fa == pico8_iv_def.fa, "iv.fa");
-    same(iv->l == pico8_iv_def.l, "iv.l");
-    same(same_vec(iv->v, &pico8_iv_def.v), "iv.v");
-}
-
-/* hp does not descend from ke: no `me`/`t`. */
-static void test_hp(void) {
-    const pico8_hp_t *hp = pico8_hp_new(NULL);
-
-    same(hp->fk == pico8_hp_def.fk, "hp.fk");
 }
 
 static void test_ie(void) {
@@ -181,16 +160,6 @@ static void test_pj(void) {
     same(pj->i_ == pico8_pj_def.i_, "pj.i_");
 }
 
-/* hy does not descend from ke: no `me`/`t`. */
-static void test_hy(void) {
-    const pico8_hy_t *hy = pico8_hy_new();
-
-    same(hy->nz == pico8_hy_def.nz, "hy.nz");
-    same(hy->ew == pico8_hy_def.ew, "hy.ew");
-    same(hy->lo == pico8_hy_def.lo, "hy.lo");
-    same(hy->yr == pico8_hy_def.yr, "hy.yr");
-}
-
 /* `jj` is the module's shared wrapper, not the archetype's: two fresh
  * wormholes point at the same one. */
 static void test_mz(void) {
@@ -490,10 +459,8 @@ int main(void) {
     test_nf();
     test_mh();
     test_iv();
-    test_hp();
     test_ie();
     test_pj();
-    test_hy();
     test_mz();
     test_jc();
     test_g_();
@@ -508,7 +475,7 @@ int main(void) {
     test_ga();
     test_nr();
     test_fr();
-    assert(g_compared == 438 && "families a to e compare 438 fields: one missed or one too many");
+    assert(g_compared == 419 && "families a to e compare 419 fields: one missed or one too many");
 
     printf("test_pico8_archetype_chain: all tests passed\n");
     return 0;
diff --git a/tests/test_pico8_bg.c b/tests/test_pico8_bg.c
index 2c8ab81..765a1d8 100644
--- a/tests/test_pico8_bg.c
+++ b/tests/test_pico8_bg.c
@@ -152,7 +152,7 @@ static pico8_bg_t *find_bg(int *count) {
             continue;
         }
         bg = (pico8_bg_t *)e;
-        if (bg->hd == pico8_fixed_from_int(1) && bg->fa == pico8_fixed_from_int(1)) {
+        if (bg->def->hd == pico8_fixed_from_int(1) && bg->def->fa == pico8_fixed_from_int(1)) {
             found = bg;
             (*count)++;
         }
@@ -186,9 +186,10 @@ static void check_fresh_bg(const pico8_bg_t *bg) {
 
     assert(is_me(&bg->header, "ph"));
     assert(bg->header.t == 0);
-    assert(bg->fa == pico8_fixed_from_int(1));
-    assert(bg->hd == pico8_fixed_from_int(1));
-    pl = bg->pl;
+    assert(bg->def == &pico8_bg_def);
+    assert(bg->def->fa == pico8_fixed_from_int(1));
+    assert(bg->def->hd == pico8_fixed_from_int(1));
+    pl = bg->def->pl;
     assert(pl == &pico8_bg_pl_seq && pl->count == 15);
     for (i = 0; i < 15; i++) {
         assert(pl->items[i] == want_pl[i]);
@@ -214,7 +215,7 @@ static void test_construction(void) {
     generic = pico8_bg_new();
     check_fresh_bg(generic);
     assert(pico8_cg("bg") == NULL);
-    assert(from_eu->pl == built->pl && built->pl == generic->pl);
+    assert(from_eu->def == built->def && built->def == generic->def);
 }
 
 static int in_pl(int tile) {
diff --git a/tests/test_pico8_fq.c b/tests/test_pico8_fq.c
index 9b2f542..dc56564 100644
--- a/tests/test_pico8_fq.c
+++ b/tests/test_pico8_fq.c
@@ -251,10 +251,9 @@ static void test_defaults_match_the_parsed_spec(void) {
 
     pico8_jl();
     veil = pico8_fq_new(NULL);
-    assert(veil->fa == pico8_fq_start.fa);
-    assert(veil->fa == pico8_fixed_from_int(14));
-    assert(veil->hd == pico8_fq_start.hd);
-    assert(veil->hd == pico8_fixed_from_int(1));
+    assert(veil->def == &pico8_fq_def);
+    assert(veil->def->fa == pico8_fixed_from_int(14));
+    assert(veil->def->hd == pico8_fixed_from_int(1));
     assert(veil->header.me != NULL && strcmp(veil->header.me, "ph") == 0);
     assert(pico8_fq_ready());
     pico8_fq_shutdown();
diff --git a/tests/test_pico8_g_.c b/tests/test_pico8_g_.c
index 9b31e99..f292cc0 100644
--- a/tests/test_pico8_g_.c
+++ b/tests/test_pico8_g_.c
@@ -292,7 +292,7 @@ static pico8_bg_t *the_bg(void) {
             continue;
         }
         bg = (pico8_bg_t *)e;
-        if (bg->hd == pico8_fixed_from_int(1) && bg->fa == pico8_fixed_from_int(1)) {
+        if (bg->def->hd == pico8_fixed_from_int(1) && bg->def->fa == pico8_fixed_from_int(1)) {
             found = bg;
             count++;
         }
diff --git a/tests/test_pico8_hp.c b/tests/test_pico8_hp.c
index 1160b33..8b663ae 100644
--- a/tests/test_pico8_hp.c
+++ b/tests/test_pico8_hp.c
@@ -477,10 +477,10 @@ static void test_gb(void) {
         }
 
         if (k == 1) {
-            fk = (void *)b->fk;
+            fk = (void *)b->def->fk;
             assert(fk != NULL);
         }
-        assert((void *)b->fk == fk);
+        assert((void *)b->def->fk == fk);
         if (b->px != NULL) {
             found = pico8_gb_find(b->px);
             assert(found != NULL);
@@ -956,12 +956,11 @@ static void test_hp_is_a_struct(void) {
         assert(cls->states[j] == NULL);
     }
     assert(pico8_hp_ready());
-    assert(b->fk != NULL && b->fk == pico8_hp_fk());
+    assert(b->def->fk != NULL && b->def->fk == pico8_hp_fk());
 
     bare = pico8_hp_new(NULL);
     assert(bare != NULL && bare->header.cls == cls);
-    assert(bare->fk != NULL && bare->fk == b->fk);
-    assert(bare->fk == pico8_hp_start.fk);
+    assert(bare->def == &pico8_hp_def && bare->def == b->def);
     assert(bare->i != NULL && bare->i->count == 0);
     assert(bare->lq == NULL);
     assert(bare->px == NULL && bare->lr == NULL && !bare->has_fj && bare->ja == NULL && !bare->has_o);
diff --git a/tests/test_pico8_iv.c b/tests/test_pico8_iv.c
index fac75c1..d19f4b4 100644
--- a/tests/test_pico8_iv.c
+++ b/tests/test_pico8_iv.c
@@ -218,20 +218,20 @@ static void test_dg_drives_a_ke_struct(void) {
     assert(f->header.t == pico8_fixed_from_int(3));
 }
 
-/* The struct's defaults are, bit for bit, the class's generated defaults;
- * its `v` is the generated vector itself. */
+/* The struct's defaults are its archetype's, whose `fa` and `v` are the
+ * spec's. */
 static void test_defaults_match_the_parsed_spec(void) {
     const pico8_iv_t *e;
 
     fresh_world();
     e = build(NULL, "x", 7);
-    assert(e->v == &pico8_iv_start.v);
-    assert(pico8_iv_start.v.x == 0 && pico8_iv_start.v.y == -0x570a);
-    assert(e->fa == pico8_iv_start.fa);
-    assert(e->l == pico8_iv_start.l);
+    assert(e->def == &pico8_iv_def);
+    assert(e->def->v.x == 0 && e->def->v.y == -0x570a);
+    assert(e->def->fa == pico8_fixed_from_int(9));
+    assert(e->l == e->def->l);
     assert(e->header.me != NULL && strcmp(e->header.me, "ph") == 0);
     assert(e->header.t == 0);
-    assert(build(NULL, "y", 7)->v == e->v);
+    assert(build(NULL, "y", 7)->def == e->def);
     assert(pico8_iv_ready());
     pico8_iv_shutdown();
     assert(!pico8_iv_ready());
diff --git a/tests/test_pico8_mh.c b/tests/test_pico8_mh.c
index 716b0c2..64303b2 100644
--- a/tests/test_pico8_mh.c
+++ b/tests/test_pico8_mh.c
@@ -295,9 +295,9 @@ static void test_defaults_match_the_parsed_spec(void) {
 
     pico8_jl();
     box = year_box(3400);
-    assert(box->fa == pico8_mh_start.fa);
-    assert(box->fa == pico8_fixed_from_int(14));
-    assert(box->rh == pico8_mh_start.rh);
+    assert(box->def == &pico8_mh_def);
+    assert(box->def->fa == pico8_fixed_from_int(14));
+    assert(box->rh == box->def->rh);
     assert(box->rh == pico8_fixed_from_int(5));
     assert(box->header.cls->tags == PICO8_TAG(PICO8_TAG_MH));
     assert(box->header.me != NULL && strcmp(box->header.me, "ph") == 0);
diff --git a/tests/test_pico8_spec_defaults.c b/tests/test_pico8_spec_defaults.c
index 8f7d04a..553d458 100644
--- a/tests/test_pico8_spec_defaults.c
+++ b/tests/test_pico8_spec_defaults.c
@@ -69,13 +69,12 @@ static void test_second_jl_keeps_specs(void) {
     nf_l->hm = hm;
 }
 
-/* A fresh bg starts on bg's generated defaults and ke's; ready until its
- * shutdown. */
+/* A fresh bg reads its archetype and starts on ke's defaults; ready until
+ * its shutdown. */
 static void test_bg_defaults_and_ready(void) {
     pico8_bg_t *bg = pico8_bg_new();
 
-    assert(bg->fa == pico8_bg_start.fa && bg->hd == pico8_bg_start.hd);
-    assert(bg->pl == pico8_bg_start.pl && bg->pl == &pico8_bg_pl_seq);
+    assert(bg->def == &pico8_bg_def && bg->def->pl == &pico8_bg_pl_seq);
     assert(bg->header.me == pico8_ke_start.me && bg->header.t == pico8_ke_start.t);
     assert(pico8_bg_ready());
     pico8_bg_shutdown();
@@ -85,7 +84,7 @@ static void test_bg_defaults_and_ready(void) {
 static void test_js_defaults_and_ready(void) {
     pico8_js_t *js = pico8_js_new(NULL);
 
-    assert(js->fa == pico8_js_start.fa);
+    assert(js->def == &pico8_js_def && js->fa == js->def->fa);
     assert(js->header.me == pico8_ke_start.me && js->header.t == pico8_ke_start.t);
     assert(pico8_js_ready());
     pico8_js_shutdown();
@@ -196,7 +195,7 @@ static void test_g__defaults_and_ready(void) {
 static void test_hp_defaults_and_ready(void) {
     pico8_hp_t *hp = pico8_hp_new(NULL);
 
-    assert(hp->fk == pico8_hp_start.fk);
+    assert(hp->def == &pico8_hp_def && hp->def->fk == &pico8_hp_fk_row);
     assert(pico8_hp_ready());
     pico8_research_shutdown();
     assert(!pico8_hp_ready());
diff --git a/tests/test_pico8_ui.c b/tests/test_pico8_ui.c
index d56fbe3..a16c2d4 100644
--- a/tests/test_pico8_ui.c
+++ b/tests/test_pico8_ui.c
@@ -1,7 +1,7 @@
 /* The click dispatcher `ui`, the singleton `ok`, built by its only road
  * (`jl()`, `pico8_ui_new()`, on a game started directly: no events
  * file), read only through the dual-form field API and compared with the
- * generated `pico8_ui_start` and `pico8_ui_le_shared()`, and
+ * archetype `pico8_ui_def` and `pico8_ui_le_shared()`, and
  * driven through the real `dg`/`lj`/`ev`/`jz`/`cj`/`pz`. */
 
 #define _POSIX_C_SOURCE 200809L
@@ -287,8 +287,8 @@ static void test_built(void) {
         assert(!ents_listed(pico8_cg(tags[i]), ok));
     }
 
-    assert(ok->ql == pico8_fixed_from_int(58));
-    assert(ok->fa == pico8_fixed_from_int(15));
+    assert(ok->def->ql == pico8_fixed_from_int(58));
+    assert(ok->def->fa == pico8_fixed_from_int(15));
     assert(ok->header.me != NULL && strcmp(ok->header.me, "ph") == 0);
     /* The presence word is constant by the struct's shape, kept so that
      * BUILT_HASH still proves the typed read. */
@@ -300,7 +300,9 @@ static void test_built(void) {
            pico8_ui_struct_class()->rdown == NULL && pico8_ui_struct_class()->hd == NULL);
 
     assert(ok->le == pico8_ui_le_shared());
-    assert(ok->hw == &pico8_ui_start.hw);
+    assert(ok->def == &pico8_ui_def);
+    assert(ok->def->hw.xl == pico8_fixed_from_int(10) && ok->def->hw.yt == pico8_fixed_from_int(10));
+    assert(ok->def->hw.xr == pico8_fixed_from_int(118) && ok->def->hw.yb == pico8_fixed_from_int(118));
     assert(le_of(ok)->count == 0);
     check("built", h, BUILT_HASH);
 }
@@ -319,7 +321,7 @@ static void test_shared(void) {
     pico8_fixed_t ql;
 
     assert(other != NULL && other != ok);
-    assert(other->header.cls->get_ql(other, &ql) && ql == other->ql);
+    assert(other->header.cls->get_ql(other, &ql) && ql == other->def->ql);
     assert(other->le == le && le == pico8_ui_le_shared());
     h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), other));
     h = fnv(h, (uint32_t)ents_rank_in(pico8_cv("ev"), other));
@@ -705,11 +707,11 @@ static void test_ui_is_a_struct(void) {
         assert(cls->states[j] == NULL);
     }
     assert(pico8_ui_ready());
-    assert(ok->ql == pico8_ui_start.ql && ok->fa == pico8_ui_start.fa);
-    assert(pico8_ui_hw() == &pico8_ui_start.hw && ok->hw == &pico8_ui_start.hw);
+    assert(ok->def == &pico8_ui_def);
+    assert(pico8_ui_hw() == &ok->def->hw);
     assert(ok->le == pico8_ui_le_shared() && ok->le->count == 0);
     other = pico8_ui_new();
-    assert(is_of(other, cls) && other->le == pico8_ui_le_shared() && other->hw == &pico8_ui_start.hw);
+    assert(is_of(other, cls) && other->le == pico8_ui_le_shared() && other->def == ok->def);
 
     must_abort(bh_on_x, ok);
 }
```

#### 2.13a2 (`a1` → `a2`)

```diff
diff --git a/src/pico8_game_data.c b/src/pico8_game_data.c
index 0a557bd..64e820d 100644
--- a/src/pico8_game_data.c
+++ b/src/pico8_game_data.c
@@ -285,12 +285,6 @@ static const pico8_jb_item_t pj_ou_items[] = {{.row = &pj_ou_1}, {.row = &pj_ou_
     {.row = &pj_ou_4}, {.row = &pj_ou_5}, {.row = &pj_ou_6}, {.row = &pj_ou_7}};
 const pico8_jb_row_t pico8_pj_ou_row = {.count = 7, .items = pj_ou_items};
 
-const pico8_pj_cr_start_t pico8_pj_cr_start = {
-    .hi = {0x0, 0xb0000},
-    .gq = {0x0, 0xb0000},
-    .ca = 0xa0000,
-};
-
 const char pico8_mz_jj_qs[] = "-too far-";
 
 const pico8_ke_start_t pico8_ke_start = {
@@ -298,14 +292,6 @@ const pico8_ke_start_t pico8_ke_start = {
     .t = 0x0,
 };
 
-const pico8_qb_start_t pico8_qb_start = {
-    .v = {0x0, 0x0},
-    .ma = 0x0,
-    .p = {0xc00000, 0xc00000},
-    .ra = {0xc00000, 0xc00000},
-    .pc = &pico8_qb_pc_vecs,
-};
-
 const pico8_mz_start_t pico8_mz_start = {
     .me = "kz",
     .dw = 0x0,
@@ -374,24 +360,6 @@ const pico8_fv_start_t pico8_fv_start = {
     .rb = &pico8_fv_rb_row,
 };
 
-const pico8_ie_start_t pico8_ie_start = {
-    .kq = {0x0, 0x0, 0x90000, 0x80000},
-    .fa = 0xc0000,
-    .id = 0x0,
-    .ra = {0x3e70000, 0x3e70000},
-    .eh = {0x760000, 0x780000},
-};
-
-const pico8_pj_start_t pico8_pj_start = {
-    .fa = 0xa0000,
-    .ea = &pico8_pj_ea_row,
-    .gs = &pico8_pj_gs_row,
-    .cr = &pico8_pj_cr_start,
-    .i_ = &pico8_pj_i__row,
-    .ou = &pico8_pj_ou_row,
-    .ed = 0x10000,
-};
-
 const pico8_bg_def_t pico8_bg_def = {
     .me = "ph",
     .t = 0x0,
diff --git a/src/pico8_game_data.h b/src/pico8_game_data.h
index 8f924e0..0aebeed 100644
--- a/src/pico8_game_data.h
+++ b/src/pico8_game_data.h
@@ -119,14 +119,6 @@ typedef struct {
     pico8_fixed_t t;
 } pico8_ke_start_t;
 
-typedef struct {
-    pico8_vec_t v;
-    pico8_fixed_t ma;
-    pico8_vec_t p;
-    pico8_vec_t ra;
-    const pico8_vec_seq_t *pc;
-} pico8_qb_start_t;
-
 typedef struct {
     const char *me;
     pico8_fixed_t dw;
@@ -198,30 +190,6 @@ typedef struct {
     const pico8_jb_row_t *rb;
 } pico8_fv_start_t;
 
-typedef struct {
-    pico8_qz_box_t kq;
-    pico8_fixed_t fa;
-    pico8_fixed_t id;
-    pico8_vec_t ra;
-    pico8_vec_t eh;
-} pico8_ie_start_t;
-
-typedef struct {
-    pico8_vec_t hi;
-    pico8_vec_t gq;
-    pico8_fixed_t ca;
-} pico8_pj_cr_start_t;
-
-typedef struct {
-    pico8_fixed_t fa;
-    const pico8_jb_row_t *ea;
-    const pico8_jb_row_t *gs;
-    const pico8_pj_cr_start_t *cr;
-    const pico8_jb_row_t *i_;
-    const pico8_jb_row_t *ou;
-    pico8_fixed_t ed;
-} pico8_pj_start_t;
-
 /* A class's archetype: its defaults resolved along its init chain, ke's
  * `me`/`t` first. A default that is the address of a module's static object
  * (ui.le, nf.ra, mz.jj) is not here: the module's init still sets it. */
@@ -622,19 +590,14 @@ extern const pico8_jb_row_t pico8_pj_i__row;
 /* pj's ea and ou: the state tables pico8_score_panel_init() builds from them. */
 extern const pico8_jb_row_t pico8_pj_ea_row;
 extern const pico8_jb_row_t pico8_pj_ou_row;
-extern const pico8_pj_cr_start_t pico8_pj_cr_start;
 
 /* lq, jb, hy and trade have an empty spec, hence no object; ui.le, nf.l and
  * nf.r are empty tables, held by their module. */
 extern const pico8_ke_start_t pico8_ke_start;
-extern const pico8_qb_start_t pico8_qb_start;
 extern const pico8_mz_start_t pico8_mz_start;
 extern const pico8_jc_start_t pico8_jc_start;
 extern const pico8_g__start_t pico8_g__start;
 extern const pico8_fv_start_t pico8_fv_start;
-/* ie's class keys, and the kq, ra, eh every icon points at. */
-extern const pico8_ie_start_t pico8_ie_start;
-extern const pico8_pj_start_t pico8_pj_start;
 extern const pico8_jo_start_t pico8_jo_start;
 extern const pico8_qh_start_t pico8_qh_start;
 extern const pico8_nl_start_t pico8_nl_start;
diff --git a/src/pico8_research_icon.c b/src/pico8_research_icon.c
index 87ce91d..14eb398 100644
--- a/src/pico8_research_icon.c
+++ b/src/pico8_research_icon.c
@@ -203,7 +203,7 @@ static int pico8_ie_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_v
 }
 
 static int ie_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_ie_t *)self)->fa;
+    *out = ((const pico8_ie_t *)self)->def->fa;
     return 1;
 }
 
@@ -212,7 +212,7 @@ static const pico8_vec_t *ie_get_ra(const void *self) {
 }
 
 static const pico8_qz_box_t *ie_get_kq(const void *self) {
-    return ((const pico8_ie_t *)self)->kq;
+    return &((const pico8_ie_t *)self)->def->kq;
 }
 
 static void ie_set_m_(void *self, int m_) {
@@ -238,10 +238,8 @@ static pico8_ie_t *pico8_ie_struct_new(const pico8_ie_keys_t *keys) {
 
     assert(g_ie_up && "pico8_research_icon_init() must run before an ie is built");
     pico8_ke_struct_init(&self->header, &g_ie_struct_class);
-    self->kq = &pico8_ie_start.kq;
-    self->fa = pico8_ie_start.fa;
-    self->ra = &pico8_ie_start.ra;
-    self->eh = &pico8_ie_start.eh;
+    self->def = &pico8_ie_def;
+    self->ra = &self->def->ra;
     self->ke = NULL;
     self->jb = NULL;
     self->kh = 0;
@@ -266,7 +264,7 @@ void pico8_research_icon_init(void) {
 
     g_ie_up = 1;
     g_ie_gy = NULL;
-    g_ie_id = pico8_ie_start.id;
+    g_ie_id = pico8_fixed_from_int(0);
     g_ie_has_id = 1;
 }
 
@@ -289,7 +287,7 @@ pico8_ie_t *pico8_ie_new(const pico8_ie_keys_t *keys) {
 }
 
 const pico8_vec_t *pico8_ie_eh(void) {
-    return g_ie_up ? &pico8_ie_start.eh : NULL;
+    return g_ie_up ? &pico8_ie_def.eh : NULL;
 }
 
 const pico8_vec_t *pico8_ie_gy(void) {
diff --git a/src/pico8_research_icon.h b/src/pico8_research_icon.h
index cee78f3..7b8caf3 100644
--- a/src/pico8_research_icon.h
+++ b/src/pico8_research_icon.h
@@ -1,6 +1,7 @@
 #ifndef PICO8_RESEARCH_ICON_H
 #define PICO8_RESEARCH_ICON_H
 
+#include "pico8_game_data.h"
 #include "pico8_geom.h"
 #include "pico8_gfx.h"
 #include "pico8_ke.h"
@@ -18,16 +19,14 @@
  * read off a `pico8_ie_ke_t`, never through a struct header on the kind
  * pointer. */
 
-/* `kq`, `ra` and `eh` point at `pico8_ie_start`'s, shared by every `ie`;
- * `hb` rebinds `ra` to the cursor. `gy` and `id` are no members: the
+/* `ra` starts on the archetype's, shared by every `ie`; `hb` rebinds it to
+ * the cursor. `gy` and `id` are no members: the
  * module's shared state, read by pico8_ie_gy()/pico8_ie_id(). Nothing frees
  * the struct: TROU-7. */
 typedef struct {
     pico8_struct_header_t header;
-    const pico8_qz_box_t *kq;
-    pico8_fixed_t fa;
+    const pico8_ie_def_t *def;
     const pico8_vec_t *ra;
-    const pico8_vec_t *eh;
     const pico8_entity_kind_t *ke;
     pico8_jb_t *jb;
     int kh;
@@ -49,7 +48,7 @@ typedef struct {
 } pico8_ie_ke_t;
 
 /* Calls pico8_research_icon_shutdown(), then pico8_pi_init(); puts `ie.id`
- * back to `pico8_ie_start.id`. */
+ * back to 0. */
 void pico8_research_icon_init(void);
 
 /* Clears `ie.gy`, frees nothing. Idempotent. */
@@ -70,7 +69,7 @@ typedef struct {
  * `lt`, and nothing frees it. */
 pico8_ie_t *pico8_ie_new(const pico8_ie_keys_t *keys);
 
-/* `ie.eh`, `pico8_ie_start.eh`, for `_init` to point `ie.gy` at: an alias,
+/* `ie.eh`, `pico8_ie_def.eh`, for `_init` to point `ie.gy` at: an alias,
  * not a copy. NULL before pico8_research_icon_init() and after the
  * shutdown. */
 const pico8_vec_t *pico8_ie_eh(void);
diff --git a/src/pico8_score_panel.c b/src/pico8_score_panel.c
index dbe21e9..1781501 100644
--- a/src/pico8_score_panel.c
+++ b/src/pico8_score_panel.c
@@ -64,9 +64,9 @@ static const pico8_struct_class_t g_pj_cr_struct_class = {0};
 
 static pico8_pj_cr_t g_pj_cr = {
     {.cls = &g_pj_cr_struct_class},
-    &pico8_pj_cr_start.hi,
-    &pico8_pj_cr_start.gq,
-    0,
+    &(const pico8_vec_t){0x0, 0xb0000},
+    &(const pico8_vec_t){0x0, 0xb0000},
+    0xa0000,
     NULL,
 };
 
@@ -87,11 +87,11 @@ const pico8_struct_class_t *pico8_pj_cr_struct_class(void) {
 }
 
 const pico8_jb_row_t *pico8_pj_i_(void) {
-    return pico8_pj_start.i_;
+    return pico8_pj_def.i_;
 }
 
 const pico8_jb_row_t *pico8_pj_gs(void) {
-    return pico8_pj_start.gs;
+    return pico8_pj_def.gs;
 }
 
 static void pico8_pj_init_placeholder(pico8_js_t *self) {
@@ -291,9 +291,8 @@ static pico8_pj_t *pico8_pj_struct_new(void) {
     assert(g_pj_up && "pico8_score_panel_init() must run first");
     self = malloc(sizeof *self);
     pico8_ke_struct_init(&self->header, &g_pj_struct_class);
-    self->fa = pico8_pj_start.fa;
-    self->gs = pico8_pj_start.gs;
-    self->i_ = pico8_pj_start.i_;
+    self->def = &pico8_pj_def;
+    self->fa = self->def->fa;
     self->ed = NULL;
     self->c_ = 0;
     self->has_c_ = 0;
@@ -328,16 +327,15 @@ void pico8_score_panel_init(void) {
     }
 
     g_pj_up = 1;
-    pj_row_copy(&g_pj_ou.row, g_pj_ou.items, 7, pico8_pj_start.ou);
+    pj_row_copy(&g_pj_ou.row, g_pj_ou.items, 7, &pico8_pj_ou_row);
     for (i = 0; i < g_pj_ou.row.count; i++) {
-        pj_row_copy(&g_pj_ou.subs[i], g_pj_ou.sub_items[i], 4, pico8_pj_start.ou->items[i].row);
+        pj_row_copy(&g_pj_ou.subs[i], g_pj_ou.sub_items[i], 4, pico8_pj_ou_row.items[i].row);
         g_pj_ou.items[i].row = &g_pj_ou.subs[i];
     }
-    pj_row_copy(&g_pj_ea.row, g_pj_ea.items, 3, pico8_pj_start.ea);
-    pj_row_copy(&g_pj_ea.second, g_pj_ea.second_items, 5, pico8_pj_start.ea->items[1].row);
+    pj_row_copy(&g_pj_ea.row, g_pj_ea.items, 3, &pico8_pj_ea_row);
+    pj_row_copy(&g_pj_ea.second, g_pj_ea.second_items, 5, pico8_pj_ea_row.items[1].row);
     g_pj_ea.items[1].row = &g_pj_ea.second;
     g_pj_ea.lh = NULL;
-    g_pj_cr.ca = pico8_pj_start.cr->ca;
     g_pj_cr.ep = NULL;
 
     pico8_score_panel_set_init(NULL);
diff --git a/src/pico8_score_panel.h b/src/pico8_score_panel.h
index 15209e6..0d61e03 100644
--- a/src/pico8_score_panel.h
+++ b/src/pico8_score_panel.h
@@ -14,13 +14,11 @@
 /* Throughout this module a NULL panel argument is a safe no-op. */
 
 /* TROU-7 */
-/* `gs` points at `pico8_pj_gs_row`, `i_` at `pico8_pj_i__row`. `ed` NULL
- * reads as `pico8_pj_start.ed`, never nil. */
+/* `ed` NULL reads as the class's `ed`, the number 1, never nil. */
 typedef struct {
     pico8_struct_header_t header;
+    const pico8_pj_def_t *def;
     pico8_fixed_t fa;
-    const pico8_jb_row_t *gs;
-    const pico8_jb_row_t *i_;
     pico8_mh_t *ed;
     int c_;
     int has_c_;
@@ -28,8 +26,7 @@ typedef struct {
     int has_gz;
 } pico8_pj_t;
 
-/* `pj.cr`: no parent, never an entity, and no writer. `hi`, `gq` point at
- * `pico8_pj_cr_start`'s. */
+/* `pj.cr`: no parent, never an entity, and no writer. */
 typedef struct {
     pico8_struct_header_t header;
     const pico8_vec_t *hi;
diff --git a/src/pico8_scroll.c b/src/pico8_scroll.c
index 94c4c72..128b29b 100644
--- a/src/pico8_scroll.c
+++ b/src/pico8_scroll.c
@@ -45,11 +45,11 @@ static pico8_qb_t *pico8_qb_struct_new(void) {
     pico8_qb_t *self = malloc(sizeof *self);
 
     pico8_ke_struct_init(&self->header, &g_qb_struct_class);
-    self->v = &pico8_qb_start.v;
-    self->ma = pico8_qb_start.ma;
-    self->p = &pico8_qb_start.p;
-    self->ra = &pico8_qb_start.ra;
-    self->pc = pico8_qb_start.pc;
+    self->def = &pico8_qb_def;
+    self->v = &self->def->v;
+    self->ma = self->def->ma;
+    self->p = &self->def->p;
+    self->ra = &self->def->ra;
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("qb");
 #endif
@@ -94,7 +94,7 @@ void pico8_qb_ph(const pico8_input_t *input, pico8_qb_t *self) {
         return;
     }
 
-    pc = self->pc;
+    pc = self->def->pc;
 
     for (b = 0; b <= 3; b++) {
         int btn_p1 = pico8_btn(input, b);
diff --git a/src/pico8_scroll.h b/src/pico8_scroll.h
index f82c73b..e7d1f5d 100644
--- a/src/pico8_scroll.h
+++ b/src/pico8_scroll.h
@@ -13,16 +13,16 @@
  * safe no-op, and a nil (NULL) vector member reads as the zero vector
  * rather than crashing. */
 
-/* `v`, `p` and `ra` start on the vectors of `pico8_qb_start`, which every qb
- * shares, `pc` on `pico8_qb_pc_vecs`: none is ever modified in place, only
- * rebound. Nothing frees the struct: TROU-7. */
+/* `v`, `p` and `ra` start on the vectors of the archetype, which every qb
+ * shares: none is ever modified in place, only rebound. Nothing frees the
+ * struct: TROU-7. */
 typedef struct {
     pico8_struct_header_t header;
+    const pico8_qb_def_t *def;
     const pico8_vec_t *v;
     pico8_fixed_t ma;
     const pico8_vec_t *p;
     const pico8_vec_t *ra;
-    const pico8_vec_seq_t *pc;
 } pico8_qb_t;
 
 /* Calls pico8_pi_init(), and requires pico8_ke_init() to have run --
diff --git a/tests/test_pico8_archetype_chain.c b/tests/test_pico8_archetype_chain.c
index 6d285ca..08719bc 100644
--- a/tests/test_pico8_archetype_chain.c
+++ b/tests/test_pico8_archetype_chain.c
@@ -107,11 +107,6 @@ static void test_qb(void) {
 
     same(same_text(qb->header.me, pico8_qb_def.me), "qb.me");
     same(qb->header.t == pico8_qb_def.t, "qb.t");
-    same(same_vec(qb->v, &pico8_qb_def.v), "qb.v");
-    same(qb->ma == pico8_qb_def.ma, "qb.ma");
-    same(same_vec(qb->p, &pico8_qb_def.p), "qb.p");
-    same(same_vec(qb->ra, &pico8_qb_def.ra), "qb.ra");
-    same(qb->pc == pico8_qb_def.pc, "qb.pc");
 }
 
 /* `ra` is the module's shared vector, not the archetype's: two fresh
@@ -144,10 +139,6 @@ static void test_ie(void) {
 
     same(same_text(ie->header.me, pico8_ie_def.me), "ie.me");
     same(ie->header.t == pico8_ie_def.t, "ie.t");
-    same(same_box(ie->kq, &pico8_ie_def.kq), "ie.kq");
-    same(ie->fa == pico8_ie_def.fa, "ie.fa");
-    same(same_vec(ie->ra, &pico8_ie_def.ra), "ie.ra");
-    same(same_vec(ie->eh, &pico8_ie_def.eh), "ie.eh");
 }
 
 static void test_pj(void) {
@@ -155,9 +146,6 @@ static void test_pj(void) {
 
     same(same_text(pj->header.me, pico8_pj_def.me), "pj.me");
     same(pj->header.t == pico8_pj_def.t, "pj.t");
-    same(pj->fa == pico8_pj_def.fa, "pj.fa");
-    same(pj->gs == pico8_pj_def.gs, "pj.gs");
-    same(pj->i_ == pico8_pj_def.i_, "pj.i_");
 }
 
 /* `jj` is the module's shared wrapper, not the archetype's: two fresh
@@ -475,7 +463,7 @@ int main(void) {
     test_ga();
     test_nr();
     test_fr();
-    assert(g_compared == 419 && "families a to e compare 419 fields: one missed or one too many");
+    assert(g_compared == 407 && "families a to e compare 407 fields: one missed or one too many");
 
     printf("test_pico8_archetype_chain: all tests passed\n");
     return 0;
diff --git a/tests/test_pico8_ie.c b/tests/test_pico8_ie.c
index 4870b6a..820ecab 100644
--- a/tests/test_pico8_ie.c
+++ b/tests/test_pico8_ie.c
@@ -259,7 +259,7 @@ static void *build_icon(const pico8_entity_kind_t *ke, pico8_jb_t *jb) {
 static uint32_t fold_kq(uint32_t h, void *icon) {
     static const int probe[][2] = {{0, 0},  {-1, 0}, {0, -1}, {8, 7},  {9, 8},  {10, 8}, {9, 9},
                                    {-1, -1}, {4, 4},  {8, 0},  {9, 0},  {0, 7},  {0, 8},  {0, 9}};
-    const pico8_qz_box_t *kq = as_ie(icon)->kq;
+    const pico8_qz_box_t *kq = &as_ie(icon)->def->kq;
     size_t i;
 
     assert(kq != NULL);
@@ -269,20 +269,18 @@ static uint32_t fold_kq(uint32_t h, void *icon) {
     return h;
 }
 
-/* A fresh icon: `kq`/`fa`/`ra`/`eh` those of `pico8_ie_start`, ke's `me`/`t`, the two
+/* A fresh icon: its archetype's `kq`/`fa`/`ra`/`eh`, ke's `me`/`t`, the two
  * construction keys, and nothing written by `hb` or the hover loop yet. */
 static uint32_t check_fresh_icon(uint32_t h, void *icon, const void *ke, const void *jb) {
     const pico8_ie_t *ie = as_ie(icon);
 
     assert(ie->header.me != NULL && strcmp(ie->header.me, "ph") == 0);
     assert(ie->header.t == 0);
-    assert(ie->fa == pico8_fixed_from_int(12));
-    assert(ie->fa == pico8_ie_start.fa);
-    assert(ie->kq == &pico8_ie_start.kq);
-    assert(ie->ra == &pico8_ie_start.ra);
-    assert(ie->eh == &pico8_ie_start.eh);
+    assert(ie->def == &pico8_ie_def);
+    assert(ie->def->fa == pico8_fixed_from_int(12));
+    assert(ie->ra == &ie->def->ra);
     assert(ie->ra->x == pico8_fixed_from_int(999) && ie->ra->y == pico8_fixed_from_int(999));
-    assert(ie->eh->x == pico8_fixed_from_int(118) && ie->eh->y == pico8_fixed_from_int(120));
+    assert(ie->def->eh.x == pico8_fixed_from_int(118) && ie->def->eh.y == pico8_fixed_from_int(120));
     assert((const void *)ie->ke == ke);
     assert((const void *)ie->jb == jb);
     assert(!ie->has_kh && !ie->has_m_);
@@ -295,10 +293,10 @@ static uint32_t check_fresh_icon(uint32_t h, void *icon, const void *ke, const v
     assert(!ents_listed(pico8_cv("ev"), icon) && !ents_listed(pico8_cv("hk"), icon));
 
     h = fold_string(h, ie->header.me);
-    h = fold_fixed(h, ie->fa);
+    h = fold_fixed(h, ie->def->fa);
     h = fold_kq(h, icon);
     h = fold_vec(h, ie->ra);
-    h = fold_vec(h, ie->eh);
+    h = fold_vec(h, &ie->def->eh);
     return fold_shared(h, icon);
 }
 
@@ -322,7 +320,7 @@ static void test_construction(void) {
         assert(ie->ke == classes[i]);
         assert(ie->jb != NULL);
         assert(ie->header.me != NULL && strcmp(ie->header.me, "ph") == 0);
-        assert(ie->fa == pico8_fixed_from_int(12));
+        assert(ie->def->fa == pico8_fixed_from_int(12));
         assert(cls_of(all[i])->hd != NULL);
         assert(ents_listed(pico8_hv(), all[i]) && ents_listed(pico8_cv("kj"), all[i]) && ents_listed(pico8_cv("hd"), all[i]));
         assert(!ents_listed(pico8_cv("ev"), all[i]) && !ents_listed(pico8_cv("hk"), all[i]));
@@ -336,9 +334,8 @@ static void test_construction(void) {
     h = check_fresh_icon(h, a, classes[0], jb_a);
     h = check_fresh_icon(h, b, classes[1], jb_b);
     assert(as_ie(a)->ra == as_ie(b)->ra);
-    assert(as_ie(a)->kq == as_ie(b)->kq);
-    assert(as_ie(a)->eh == as_ie(b)->eh);
-    assert(as_ie(a)->kq == as_ie(all[0])->kq);
+    assert(as_ie(a)->def == as_ie(b)->def);
+    assert(as_ie(a)->def == as_ie(all[0])->def);
     assert(icon_rank(a) == 7 && icon_rank(b) == 8);
     check("construction", h, CONSTRUCTION_HASH);
 }
@@ -362,7 +359,7 @@ static void test_start(void) {
     }
     printf("[start] first revealed icon %d\n", first);
     assert(first >= 0);
-    assert(as_ie(all[first])->ra == as_ie(all[first])->eh);
+    assert(as_ie(all[first])->ra == &as_ie(all[first])->def->eh);
     for (i = 0; i < n; i++) {
         if (kh_state(all[i]) != 2) {
             assert(as_ie(all[i])->ra != as_ie(all[first])->ra);
@@ -580,7 +577,7 @@ static void test_restart(void) {
     gy = shared_gy(as_ie(all[0]));
     id = shared_id_fixed(as_ie(all[0]));
     assert(id == pico8_fixed_from_int(1));
-    assert(gy != as_ie(all[0])->eh);
+    assert(gy != &as_ie(all[0])->def->eh);
 
     new_game();
     n = icons(all, 16);
@@ -589,14 +586,14 @@ static void test_restart(void) {
     assert(shared_gy(as_ie(all[0])) != gy);
     for (i = 0; i < n; i++) {
         if (kh_state(all[i]) == 2) {
-            assert(as_ie(all[i])->ra != as_ie(all[i])->eh);
+            assert(as_ie(all[i])->ra != &as_ie(all[i])->def->eh);
         }
         h = fold_icon(h, all[i]);
     }
     h = fold_shared(h, all[0]);
 
     pico8__init();
-    assert(shared_gy(as_ie(all[0])) == as_ie(all[0])->eh);
+    assert(shared_gy(as_ie(all[0])) == &as_ie(all[0])->def->eh);
     assert(shared_id_fixed(as_ie(all[0])) == id);
     h = fold_shared(h, all[0]);
     check("restart", h, RESTART_HASH);
diff --git a/tests/test_pico8_pj.c b/tests/test_pico8_pj.c
index 6669e7a..621faae 100644
--- a/tests/test_pico8_pj.c
+++ b/tests/test_pico8_pj.c
@@ -1,6 +1,6 @@
 /* The score panel `pj`, built by its only road (`eu()`, `pico8_pj_new()`,
  * on a game started directly: no events file), read on its members and on
- * the class's shared `ou`/`ea`/`cr`, its defaults compared to `pico8_pj_start`, and
+ * the class's shared `ou`/`ea`/`cr`, its defaults read on its archetype, and
  * driven through the real `mv`/`cr()`/`de.my`. */
 
 #define _POSIX_C_SOURCE 200809L
@@ -92,10 +92,10 @@ static uint32_t c__state(const pico8_pj_t *pj) {
     return pj->c_ ? 2 : 1;
 }
 
-/* `ed` NULL reads as `pico8_pj_start.ed`. */
+/* `ed` NULL reads as the class's `ed`, the number 1. */
 static pico8_fixed_t ed_fixed(const pico8_pj_t *pj) {
     assert(pj->ed == NULL);
-    return pico8_pj_start.ed;
+    return pico8_fixed_from_int(1);
 }
 
 static int is_lq(const void *e) {
@@ -361,7 +361,7 @@ static void test_built(void) {
     }
 
     assert(ju->fa == pico8_fixed_from_int(10));
-    assert(ju->fa == pico8_pj_start.fa);
+    assert(ju->def == &pico8_pj_def && ju->fa == ju->def->fa);
     assert(ed_fixed(ju) == pico8_fixed_from_int(1));
     assert(ju->header.me != NULL && strcmp(ju->header.me, "ph") == 0);
     h = fold_fixed_member(h, ju->header.t);
@@ -370,11 +370,11 @@ static void test_built(void) {
            cls_of(ju)->hk != NULL);
     assert(cls_of(ju)->hd == NULL && cls_of(ju)->ev == NULL);
     h = fold_ref_member(h, pico8_pj_ea());
-    assert(ju->gs == &pico8_pj_gs_row);
-    h = fold_ref_member(h, ju->gs);
+    assert(ju->def->gs == &pico8_pj_gs_row);
+    h = fold_ref_member(h, ju->def->gs);
     h = fold_cr(h, pico8_pj_cr());
-    assert(ju->i_ == &pico8_pj_i__row);
-    h = fold_ref_member(h, ju->i_);
+    assert(ju->def->i_ == &pico8_pj_i__row);
+    h = fold_ref_member(h, ju->def->i_);
     h = fold_ref_member(h, pico8_pj_ou());
     check("built", h, BUILT_HASH);
 }
@@ -389,7 +389,7 @@ static void test_shared(void) {
     const pico8_pj_ea_t *ea;
 
     assert(other != NULL && other != ju);
-    assert(other->gs == ju->gs && other->i_ == ju->i_);
+    assert(other->def == ju->def);
     h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), other));
 
     cls_of(ju)->hb(ju);
@@ -610,7 +610,6 @@ static void test_ou_ea_start_as_their_rows(void) {
     assert(&ou->row != &pico8_pj_ou_row && &pico8_pj_ea()->row != &pico8_pj_ea_row);
     assert(fold_row(0u, &ou->row) == fold_row(0u, &pico8_pj_ou_row));
     assert(fold_row(0u, &pico8_pj_ea()->row) == fold_row(0u, &pico8_pj_ea_row) && pico8_pj_ea()->lh == NULL);
-    assert(pico8_pj_start.ou == &pico8_pj_ou_row && pico8_pj_start.ea == &pico8_pj_ea_row);
     ou->sub_items[0][0] = (pico8_jb_item_t){.s = "written"};
     pico8_pj_ea()->second_items[0] = (pico8_jb_item_t){.s = "year 3425"};
     pico8_pj_ea()->items[2] = (pico8_jb_item_t){.row = &pico8_pj_gs_row};
@@ -646,8 +645,7 @@ static void test_pj_is_a_struct(void) {
     assert(pico8_pj_ready());
     assert(pico8_pj_ou() != NULL && pico8_pj_ea() != NULL && pico8_pj_cr() != NULL);
     assert(pico8_pj_i_() == &pico8_pj_i__row && pico8_pj_gs() == &pico8_pj_gs_row);
-    assert(pico8_pj_i_() == pico8_pj_start.i_ && pico8_pj_gs() == pico8_pj_start.gs);
-    assert(ju->gs == &pico8_pj_gs_row && ju->i_ == &pico8_pj_i__row);
+    assert(ju->def->gs == &pico8_pj_gs_row && ju->def->i_ == &pico8_pj_i__row);
 }
 
 /* `pj.cr` is one static struct without method, holding `hi`,
diff --git a/tests/test_pico8_qb.c b/tests/test_pico8_qb.c
index 7a8f1bf..529a9cc 100644
--- a/tests/test_pico8_qb.c
+++ b/tests/test_pico8_qb.c
@@ -148,7 +148,7 @@ static void check_fresh_mo(const pico8_qb_t *mo) {
     assert(mo->header.me != NULL && strcmp(mo->header.me, "ph") == 0);
     assert(mo->p != mo->ra);
 
-    pc = mo->pc;
+    pc = mo->def->pc;
     assert(pc != NULL);
     assert(pc == &pico8_qb_pc_vecs && pc->count == 4);
     assert(is_vec(pc->items[0], -1, 0));
diff --git a/tests/test_pico8_spec_defaults.c b/tests/test_pico8_spec_defaults.c
index 553d458..1e5ba4e 100644
--- a/tests/test_pico8_spec_defaults.c
+++ b/tests/test_pico8_spec_defaults.c
@@ -91,12 +91,12 @@ static void test_js_defaults_and_ready(void) {
     assert(!pico8_js_ready());
 }
 
-/* `v`, `p` and `ra` are the generated vectors themselves. */
+/* `v`, `p` and `ra` start on the archetype's vectors. */
 static void test_qb_defaults_and_ready(void) {
     pico8_qb_t *qb = pico8_qb_new();
 
-    assert(qb->v == &pico8_qb_start.v && qb->p == &pico8_qb_start.p && qb->ra == &pico8_qb_start.ra);
-    assert(qb->ma == pico8_qb_start.ma && qb->pc == pico8_qb_start.pc && qb->pc == &pico8_qb_pc_vecs);
+    assert(qb->def == &pico8_qb_def && qb->def->pc == &pico8_qb_pc_vecs);
+    assert(qb->v == &qb->def->v && qb->p == &qb->def->p && qb->ra == &qb->def->ra && qb->ma == qb->def->ma);
     assert(qb->header.me == pico8_ke_start.me && qb->header.t == pico8_ke_start.t);
     assert(pico8_qb_ready());
     pico8_qb_shutdown();
@@ -201,34 +201,27 @@ static void test_hp_defaults_and_ready(void) {
     assert(!pico8_hp_ready());
 }
 
-/* `kq`, `ra`, `eh` hold ie's generated box and vectors. */
+/* `ra` starts on the archetype's vector. */
 static void test_ie_defaults_and_ready(void) {
     pico8_ie_t *ie = pico8_ie_new(NULL);
 
-    assert(ie->kq->xl == pico8_ie_start.kq.xl && ie->kq->yt == pico8_ie_start.kq.yt);
-    assert(ie->kq->xr == pico8_ie_start.kq.xr && ie->kq->yb == pico8_ie_start.kq.yb);
-    assert(ie->fa == pico8_ie_start.fa);
-    assert(ie->ra->x == pico8_ie_start.ra.x && ie->ra->y == pico8_ie_start.ra.y);
-    assert(ie->eh->x == pico8_ie_start.eh.x && ie->eh->y == pico8_ie_start.eh.y);
+    assert(ie->def == &pico8_ie_def && ie->ra == &ie->def->ra);
     assert(ie->header.me == pico8_ke_start.me && ie->header.t == pico8_ke_start.t);
     assert(pico8_ie_ready());
     pico8_research_icon_shutdown();
     assert(!pico8_ie_ready());
 }
 
-/* `ed` absent reads as pj's generated `ed`; `cr` holds pj's generated
- * record. */
+/* `cr` holds the spec's record. */
 static void test_pj_defaults_and_ready(void) {
     pico8_pj_t *pj = pico8_pj_new();
     const pico8_pj_cr_t *cr = pico8_pj_cr();
 
-    assert(pj->fa == pico8_pj_start.fa);
-    assert(pj->gs == pico8_pj_start.gs);
-    assert(pj->i_ == pico8_pj_start.i_);
+    assert(pj->def == &pico8_pj_def && pj->fa == pj->def->fa);
     assert(pj->ed == NULL);
-    assert(cr->hi->x == pico8_pj_start.cr->hi.x && cr->hi->y == pico8_pj_start.cr->hi.y);
-    assert(cr->gq->x == pico8_pj_start.cr->gq.x && cr->gq->y == pico8_pj_start.cr->gq.y);
-    assert(cr->ca == pico8_pj_start.cr->ca);
+    assert(cr->hi->x == 0 && cr->hi->y == pico8_fixed_from_int(11));
+    assert(cr->gq->x == 0 && cr->gq->y == pico8_fixed_from_int(11));
+    assert(cr->ca == pico8_fixed_from_int(10));
     assert(pj->header.me == pico8_ke_start.me && pj->header.t == pico8_ke_start.t);
     assert(pico8_pj_ready());
     pico8_score_panel_shutdown();
```

**`docs/plan-3.md`** (a1) — note « Découvert à 2.13a1 » au bout de la ligne 2.13a1 : comptes (`défauts` **603**, pied **1007 / 78**, `--champs` **279 ; 15 ; 29**, `inclusion-mutuelle` 17 / 16), `game_data.h` **703** lignes, total du test ponctuel **419** et pourquoi (`me`/`t` gardés), les deux filets neufs (`ui.hw`, `iv.fa`), le repli s'il a servi. (a2) — note « Découvert à 2.13a2 » : `défauts` **536**, pied **940 / 74**, `--champs` **276 ; 15 ; 32**, `game_data.h` **666**, total **407**, `pj_cr` par littéraux composés, `ie.id`/`pj.ed`/`pj.ou`/`pj.ea` hors archétype. Le reste va au message de commit.

### Test ponctuel : comment il évolue quand une chaîne disparaît

Une comparaison du test ponctuel a un sens tant que la chaîne qu'elle compare existe. Une fois le `_start` retiré, l'instance lit l'archétype : `bg->def->hd == pico8_bg_def.hd` serait vrai par construction. Donc, dans le commit qui retire la chaîne d'une classe, **ses comparaisons de champs propres partent** (pas réécrites vers `def`).

Mais la famille a a deux chaînes. Ses champs propres viennent de `pico8_X_start`, qui part en 2.13a. `me`/`t` viennent de `pico8_ke_start`, par `pico8_ke_struct_init`, qui reste jusqu'en 2.13e2 (plan). Les comparaisons `header.me`/`header.t` contre `pico8_X_def.me`/`.t` comparent donc encore une chaîne vivante à l'archétype. Elles **restent**, et elles sont le seul filet de `pico8_X_def.me`/`.t` : A21 (`pico8_nf_def.me` → `"kz"`) et A22 (`pico8_bg_def.t` → 1) ne rougissent que `archetype_chain` à `a1`, comme au parent. Si elles partaient, ces jumelles seraient vertes à l'enfant et rouges au parent : ÉCHEC.

| Sous-étape | Fonctions gardées (`me`/`t` seuls) | Fonctions retirées | Comparaisons retirées | Total asserté |
|---|---|---|---|---|
| départ (`p`) | — | — | — | 438 |
| 2.13a1 | `test_bg`, `test_ui`, `test_js`, `test_fq`, `test_nf`, `test_mh`, `test_iv` | `test_hp`, `test_hy` (pas de `me`/`t`) | 19 (`bg` 3, `ui` 3, `js` 1, `fq` 2, `mh` 2, `iv` 3, `hp` 1, `hy` 4) | **419** |
| 2.13a2 | `test_qb`, `test_ie`, `test_pj` | — | 12 (`qb` 5, `ie` 4, `pj` 3) | **407** |

Les `assert` nus hors compte restent : `ui->le == pico8_ui_le_shared()` (dans `test_ui`) et le `ra` partagé de deux `nf` (dans `test_nf`). Leur chaîne (l'init qui pose l'objet statique) ne part jamais : voir le contrat pour 2.13e2.

Le message du total garde sa forme (`"families a to e compare 419 fields: one missed or one too many"`) : c'est encore le compte des comparaisons de toutes les familles.

### Comptes, mesurés sur la copie

| Mesure | `p` | `a1` | `a2` |
|---|---|---|---|
| `défauts` (lignes / fichiers) | 669 / 52 | **603 / 39** | **536 / 31** |
| pied `résidu 2` | 1073 / 85 | **1007 / 78** | **940 / 74** |
| `parente`, `lien-parent`, `accesseurs-q6`, `void-ptr` | 10/3, 111/27, 237/33, 8/4 | identiques | identiques |
| `inclusion-mutuelle` (`--check`) | 17 / 16, 43 lignes | **17 / 16, 43** | **17 / 16, 43** |
| `--champs` | 282 ; 15 ; 20 | **279 ; 15 ; 29** | **276 ; 15 ; 32** |
| `wc -l src/pico8_game_data.h` | 751 | **703** | **666** |
| total du test ponctuel | 438 | **419** | **407** |
| casts / tests de classe (grep, réserve 4) | 532 / 707 | 532 / 707 | 532 / 707 |

Attribution des lignes `défauts` (plan : « par les noms `pico8_X_start` de chaque ligne ») : une ligne sort quand le dernier `_start` qu'elle nomme disparaît. a1 : `bg` 8, `ui` 14, `js` 6, `fq` 7, `nf` 5, `mh` 8, `iv` 11, `hp` 7 = **66** (`hy` 0). a2 : `qb` 11, `ie` 22, `pj` 26 (dont la ligne `.cr = &pico8_pj_cr_start`), `pj_cr` 8 = **67**. Script : `re.findall(r'\bpico8_([a-z0-9_]+?)_start(?:_t)?\b', ligne)`, famille = la plus tardive des noms ; à `a1`, les 603 restantes se répartissent a2 67, b 29, c 104, d 179, e1 127, e2 97 ; à `a2`, b à e2 seules (536).

`--champs`, lu : a1 retire 12 membres A et ajoute 9 `def` (282 → 279) ; a2 retire 6 A et ajoute 3 `def` (→ 276). Chaque `def` n'est écrit que dans son `_new` et entre dans « 0 hors constructeur » (20 → 29 → 32). Aucun couple n'en sort. `pj_cr.hi`/`gq`/`ca` restent membres (instance statique unique) ; seul l'initialiseur les écrit désormais.

Pied, lu : a1 ôte 66 lignes et 7 fichiers (85 → 78), a2 67 lignes et 4 fichiers (→ 74) ; les autres catégories ne bougent pas.

### Contrat pour 2.13b à 2.13e2 (à recopier dans leurs PRD)

Filets uniques connus hors famille a (réserves des vérificateurs de 2.12c, 2.12d, 2.12e). Chacun reçoit un TU durable **dans le commit qui retire sa comparaison du test ponctuel**, avec sa jumelle rejouée au parent et à l'enfant :

| Valeur | Sous-étape | Filet au parent | TU durable à ajouter |
|---|---|---|---|
| `probe.has_od` de `g_` (C3 de 2.12c : `has_od = 1` de `pico8_g__struct_init`) | 2.13c | test ponctuel seul | `test_pico8_spec_defaults.c`, `test_g__defaults_and_ready` : `assert(g_->probe->has_od == 1);` (I : lu sur l'instance) ; jumelle enfant : `pico8_g__def.probe.has_od` → 0, rouge dans `spec_defaults` |
| `probe.has_ik` de l'arbre `qh` (H1 de 2.12e1) | 2.13d (`qh`, `nl`, `os`), 2.13e1 (`hj`, `pm`, `mx`), 2.13e2 (`nr`, `fr`) | test ponctuel seul | `test_pico8_jo_spec_defaults.c`, pour chaque sorte, dans le commit de sa famille : `has_ik` (A) lu par le `def` de la sonde, comparé au littéral 0 ; jumelle : `has_ik` → 1 dans l'archétype de la sorte |
| `emitter.pn` de `ga` (et `du`, `ki`, `pk`, tous 0 ; C4 de 2.12e2) | 2.13e2 | test ponctuel seul ; le jeu ne le lit pas (`pn` n'est lu que par `g_:mm`) | la valeur est morte pour `ga`, mais la règle du parent s'applique quand même : un littéral dans `test_pico8_jo_spec_defaults.c` (`ga` : `emitter->pn == 0`, I) suffit. Le retirer de l'archétype de `ga` n'est pas possible (type planète partagé, décision A). Rien d'autre à garder |
| `me`/`t` des dix classes de la famille a (A21, A22 ici) | 2.13e2 | test ponctuel seul (tant que `pico8_ke_start` existe) | quand `pico8_ke_start` part, `me`/`t` sont lus sur `def` : leurs comparaisons partent avec lui. Les `assert` `me == "ph"` à littéral des TU `bg`, `ui`, `iv`, `fq`, `ie`, `pj` (déjà là) deviennent le filet ; 2.13e2 vérifie par jumelle qu'aucune des dix n'en manque (`js`, `nf`, `mh`, `qb` à voir) |
| `nf.ra` partagé, `ui.le` partagé (`assert` nus de `test_nf`, `test_ui`) | 2.13e2 (le TU disparaît) | `ui.le` : aussi `test_pico8_ui.c` (`test_shared`) ; `nf.ra` : jumelle P10/A10 rouge dans cinq TU | déplacer l'`assert` du `ra` partagé de deux `nf` dans `test_pico8_spec_defaults.c` avant la suppression du fichier |

Pour la famille a, les 37 jumelles de chaîne rejouées au parent (preuve 3) n'ont trouvé que `ui.hw` et `iv.fa`, traités ici. Aucune autre valeur de la famille n'a le test ponctuel pour seul filet.

### Anti-tassement

**Interdit** :
- réécrire une comparaison du test ponctuel vers `def` (`bg->def->hd == pico8_bg_def.hd`) au lieu de la retirer : elle serait vraie par construction ;
- retirer les comparaisons `me`/`t` de la famille a avant 2.13e2 (A21, A22) ;
- un cast, un `void *`, un aiguillage par classe (`header.cls`, `_struct_class()`) pour atteindre `def` ;
- laisser un champ A dans l'instance « pour les TU » ;
- mettre `g_shared_le`, `g_nf_ra` ou un pointeur vers un objet statique de module dans un archétype ; changer un type `pico8_X_def_t` ;
- nommer un `pico8_X_start` retiré, même en commentaire (compte en `défauts`) ;
- changer une constante `*_HASH`/`FROZEN_*`, un `*.sha256`, `tests/e2e/`, `tools/`, le `Makefile` ;
- rendre `pico8_hy_def` `static` ou inclure `game_data.h` dans `economy.h`.

## Implementation Chunks

Deux commits, chacun **≈ 40 min** preuves comprises (essai : a1 ≈ 25 min d'écriture et 15 min de preuves, a2 ≈ 15 et 10). Budget 50 min chacun.

1. a1 : appliquer le diff ; `make test`, `make e2e`, compteur ; preuves 1 à 8 ; note du plan ; commit.
2. a2 : idem.

**Replis écrits** (si une sous-étape déborde de 1,5 fois son budget avant le commit), chacun un commit vert avec son total asserté :
- **2.13a1a** : `bg`, `ui`, `js`, `fq`, `nf` → `défauts` **629**, total **429** ; **2.13a1b** : `mh`, `iv`, `hp`, `hy` → **603**, **419**. Les filets `ui.hw` (a1a) et `iv.fa` (a1b) vont avec leur classe.
- **2.13a2a** : `qb`, `ie` → **570**, total **410** ; **2.13a2b** : `pj`, `pj_cr` → **536**, **407**.
Le repli est écrit dans la note « Découvert à ».

## Testing Decisions

- **Aucun TU neuf.** Deux `assert` à littéral neufs dans des TU existants (`test_pico8_ui.c`, `test_pico8_iv.c`), le reste est réécriture. Aucun rejeu d'`events.json` dans un TU.
- **Bon test ici** : un TU durable compare une valeur à un **littéral du cart** (`ok->def->hw.xr == pico8_fixed_from_int(118)`), jamais l'archétype à lui-même. `spec_defaults` compare l'instance à son `def` (`bg->def == &pico8_bg_def`, `js->fa == js->def->fa`) : cela prouve la pose et la copie, pas la valeur. Les valeurs sont gardées par les TU de classe à littéraux, les empreintes et les e2e (preuve 3).
- **Prior art** : mutations et jumelles au parent de 2.12a à 2.12e ; preuve `const` de l'étude 2.9, annexe A ; `test_pico8_spec_defaults.c`.

### Preuves (chaque commit ; parent de a1 = le commit 2.12e2, parent de a2 = a1)

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. Mesuré aux deux commits de la copie.

2. **Règle 1, preuve `const` sur `def`** (copie du commit ; chaque membre `def` rendu `*const def`, un à la fois, puis `gcc -fsyntax-only` des `.c` compilables de `src/` et `tests/`) : **une seule erreur par classe**, la pose dans son `_new` : `bg.c:70`, `ui.c:100`, `ui_button.c:135` (`js`), `ui_button.c:357` (`fq`), `mouse_reader.c:47`, `msgbox.c:111` (`mh`), `msgbox.c:258` (`iv`), `research.c:158`, `economy.c:34`, `scroll.c:48`, `research_icon.c:241`, `score_panel.c:294` (numéros de `a2`). Preuve négative : `b->def->fa = 0;` et `q->def->v.x = 0;` sur un `pico8_bg_t *`/`pico8_qb_t *` → `assignment of member … in read-only object`. Les champs A retirés n'ont plus de membre : leurs écritures de construction ont disparu (grep de Sortie 3).

3. **Mutations, règle du parent.** Une mutation « défaut de classe changé » : au parent, sur `pico8_X_start` (ou le littéral de `pico8_hy_struct_new`, de `pico8_nf_init`, de `pico8_research_icon_init`, de l'initialiseur de `g_pj_cr`) ; à l'enfant, sur `pico8_X_def` (ou le littéral qui l'a remplacé). `make -k -j4 unit` puis `make -k e2e` ; restaurer par `git checkout -- <fichier>` puis `touch`. **Vert à l'enfant et rouge au parent = ÉCHEC.** Au parent, chaque mutation d'une valeur comparée rougit aussi `archetype_chain` ; la colonne « parent » l'omet. Mesuré (e2e = conformes sur 26) :

   | # parent / enfant | Valeur, mutation | TU rouges au parent (hors `archetype_chain`) | TU rouges à l'enfant | e2e (p = enfant) |
   |---|---|---|---|---|
   | P1 / A1 | `bg.fa` 1 → 2 | `bg`, `fr`, `g_`, `ga`, `hj`, `hp`, `lq`, `mx`, `nr`, `pm`, `ui` | idem | 0 |
   | P2 / A2 | `bg.hd` 1 → 0 | `bg`, `g_` | idem | 26 |
   | P3 / A3 | `bg.pl` → `&pico8_mz_kk_seq` | `bg`, `fr`, `g_`, `ga`, `hj`, `hp`, `lq`, `mx`, `nr`, `pm`, `spec_defaults` | idem | 0 |
   | P4 / A4 | `ui.ql` 58 → 59 | `fq`, `fv`, `hp`, `lq`, `mh`, `ui` | idem | 0 |
   | P5 / A5 | `ui.fa` 15 → 14 | `mh`, `ui` | idem | 22 |
   | **P6 / A6** | `ui.hw` `xr` 118 → 119 | **aucun** | **`ui`** (filet neuf) | 26 |
   | P7 / A7 | `js.fa` 11 → 12 | `js` | idem | 26 |
   | P8 / A8 | `fq.fa` 14 → 13 | `fq` | idem | 26 |
   | P9 / A9 | `fq.hd` 1 → 0 | `fq` | idem | 26 |
   | P10 / A10 | `nf.ra` (`g_nf_ra` initial) `x` 0 → 1 | `fq`, `fv`, `je`, `mh`, `nf` (pas `archetype_chain`) | idem | 0 |
   | P11 / A11 | `mh.fa` 14 → 13 | `mh` | idem | 26 |
   | P12 / A12 | `mh.rh` 5 → 6 | `mh` | idem | 15 |
   | **P13 / A13** | `iv.fa` 9 → 8 | **aucun** | **`iv`** (filet neuf) | 26 |
   | P14 / A14 | `iv.l` 0 → 1 | `iv` | idem | 16 |
   | P15 / A15 | `iv.v` `y` `-0x570a` → `-0x5700` | `iv` | idem | 23 |
   | P16 / A16 | `hp.fk` → `&pico8_pj_gs_row` | `hp`, `js`, `lq` | `hp`, `js`, `lq`, `spec_defaults` | 0 |
   | P17 / A17 | `hy.nz` 100 → 101 | `fm`, `g_`, `ga`, `hp`, `hy`, `jc`, `lq`, `mz`, `pj`, `pm` | idem | 0 |
   | P18 / A18 | `hy.ew` 1 → 2 | `hy`, `js`, `pj` | idem | 12 |
   | P19 / A19 | `hy.lo` 1 → 2 | `fr`, `g_`, `ga`, `hj`, `hp`, `hy`, `lq`, `mx`, `nr`, `pj`, `pm` | idem | 0 |
   | P20 / A20 | `hy.yr` 3401 → 3402 | idem P19 | idem | 0 |
   | — / A21 | `pico8_nf_def.me` `"ph"` → `"kz"` | (au parent : `archetype_chain` seul) | `archetype_chain` seul | 26 |
   | — / A22 | `pico8_bg_def.t` 0 → 1 | (idem) | `archetype_chain` seul | 26 |
   | P21 / A23 | `qb.v` `x` 0 → 1 | `fr`, `hj`, `hp`, `iv`, `jc`, `lq`, `mx`, `nr`, `pm`, `qb`, `ui` | idem | 0 |
   | P22 / A24 | `qb.ma` 0 → 1 | `fr`, `hj`, `iv`, `mx`, `nr`, `qb`, `ui` | idem | 0 |
   | P23 / A25 | `qb.p` `y` 192 → 193 | idem P21 | idem | 0 |
   | P24 / A26 | `qb.ra` `y` 192 → 193 | `g_`, `ga`, `hj`, `iv`, `mo`, `planet_kj_dispatch`, `pm`, `qb`, `qh` | idem | 0 |
   | P25 / A27 | `qb.pc` → NULL | `qb`, `spec_defaults` | idem | 25 |
   | P26 / A28 | `ie.kq` `xr` 9 → 10 | `ie` | idem | 26 |
   | P27 / A29 | `ie.fa` 12 → 11 | `ie` | idem | 26 |
   | P28 / A30 | `ie.ra` `y` 999 → 998 | `ie` | idem | 26 |
   | P29 / A31 | `ie.eh` `y` 120 → 119 | `fr`, `g_`, `ga`, `hj`, `hp`, `ie`, `lq`, `mx`, `nr`, `pm` | idem | 0 |
   | P30 / A32 | `ie.id` (`g_ie_id` initial) 0 → 1 | `fr`, `hj`, `ie`, `mx`, `nl`, `nr`, `os`, `qh` (pas `archetype_chain`) | idem | 26 |
   | P31 / A33 | `pj.fa` 10 → 11 | `pj` | idem | 26 |
   | P32 / A34 | `pj.gs` → `&pico8_pj_i__row` | `mh`, `pj` | idem | 24 |
   | P33 / A35 | `pj.i_` → `&pico8_pj_gs_row` | `bg`, `fr`, `g_`, `ga`, `hj`, `hp`, `js`, `lq`, `mx`, `nr`, `pj`, `pm`, `replay_from_title` | idem | 0 |
   | P34 / — | `pj.ed` 1 → 2 | `pj` (pas `archetype_chain`) | — (`src/` ne lit jamais ce défaut ; il ne survit que dans `ed_fixed` du TU) | 26 |
   | P35 / A36 | `pj_cr.hi` `y` 11 → 12 | `js`, `pj` | `js`, `pj`, `spec_defaults` | 14 |
   | P36 / A37 | `pj_cr.gq` `y` 11 → 12 | `js`, `pj` | `js`, `pj`, `spec_defaults` | 14 |
   | P37 / A38 | `pj_cr.ca` 10 → 11 | `js`, `pj` | `js`, `pj`, `spec_defaults` | 26 |
   | — / A39 | `pico8_qb_def.me` → `"kz"` | (`archetype_chain` seul) | `archetype_chain` seul | 26 |

   Lecture : **aucun ÉCHEC**. Hors `archetype_chain`, chaque jumelle rougit à l'enfant au moins ce qu'elle rougit au parent. P6 et P13 étaient les deux filets uniques ; A6 et A13 sont rouges à l'enfant par les `assert` neufs. `spec_defaults` s'ajoute là où il compare désormais à un littéral (`hp.fk`, `pj_cr`). A21, A22, A39 montrent que les comparaisons `me`/`t` gardées sont nécessaires. P1 à P20 sont mesurées sur `p`, A1 à A22 sur `a1` ; P21 à P37 sur `p`, A23 à A39 sur `a2`. Pour la règle du parent stricte, le vérificateur de a2 rejoue P21 à P37 sur `a1` (parent de a2), où les valeurs de a2 sont encore des `_start` : l'essai l'a fait sur `p`, dont la famille a2 est identique à `a1`.

4. **Chemin touché** : `make line-coverage EVENTS="<les 26 e2e>"` sur `a2` : les douze poses de `def` (preuve 2) sont exécutées. Les lectures sont prouvées par les mutations (preuve 3 : chaque valeur de `def` rougit un TU ou un e2e).

5. **Empreintes** (réserve 2) : `BUILT_HASH` (`ie`, `pj`, `ui`), `DEFAULTS_HASH` et `GB_HASH` (`hp`) inchangés, verts ; les plis réécrits lisent la même valeur par `def`.

6. **Inclusion, casts, aiguillage** : `inclusion-mutuelle` **17 / 16, 43 lignes** ; comptes de casts et de tests de classe identiques au parent (réserve 4).

7. `class-coverage-net` : **oui**, vert aux deux commits (`couverture: OK`, comptes égaux à `tools/class_coverage_net_expected.txt`). `sanitize-net` : **oui**, `sanitize: 26 propre(s)` aux deux commits.

8. `game_data.h` mesuré au commit : 703 (a1), 666 (a2).

### Greps de Sortie (arbre de chaque commit)

1. `grep -rnE 'pico8_(bg|ui|js|fq|nf|mh|iv|hp)_start' src tests` → vide (a1) ; plus `(qb|ie|pj|pj_cr)` → vide (a2).
2. `grep -rn 'const pico8_[a-z_]*_def_t \*def;' src` → 9 (a1 : `bg.h`, `ui.h`, `ui_button.h` ×2, `mouse_reader.h`, `msgbox.h` ×2, `research.h`, `economy.h`), 12 (a2 : + `scroll.h`, `research_icon.h`, `score_panel.h`).
3. `grep -rnE 'self->def = &pico8_[a-z_]*_def;' src` → 9 puis 12, chacune dans un `pico8_X_struct_new`.
4. `grep -n 'game_data' src/pico8_economy.h` → le seul commentaire ; `grep -rn 'static const pico8_hy_def_t' src` → vide.
5. Casts et tests de classe : `grep -rEo '\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)' src tests | wc -l` → 532 ; `grep -rEo '(->cls|header\.cls|_struct_class\(\))' src tests | wc -l` → 707 (valeurs de `p` ; égales au parent si 2.12e2 diffère).
6. `grep -rnE 'g_shared_le|g_nf_ra' src/pico8_game_data.[ch]` → vide.
7. `grep -c 'same(' tests/test_pico8_archetype_chain.c` et le total asserté : **419** (a1), **407** (a2).
8. `make composition-residue` → pied **1007 / 78** puis **940 / 74**, `défauts 603 39` puis `536 31`, `inclusion-mutuelle 17 16` ; `--champs` → `279 ; 15 ; 29` puis `276 ; 15 ; 32`.
9. `git diff --stat <parent>` : les fichiers de la section « Modules » et `docs/plan-3.md`, rien d'autre.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable (`p` = `762f2f2`, `a1` = `366d450`, `a2` = `20b4078`).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout 8e2f685`, `git apply` des deux diffs du PRD 2.12e, commits e1/e2 | `p` | 0 | `3e14654`, `762f2f2` | 3 s |
| 2 | `timeout 900 make -j4 test` | `p` | 0 | vert | 10 s |
| 3 | `timeout 600 make e2e` ; compteur ; `--champs` ; `wc -l game_data.h` | `p` | 0 | 26 conformes ; 1073 / 85 ; 282 / 15 / 20 ; 751 | 5 s |
| 4 | `--check défauts` + attribution par nom | `p` | 0 | a1 66, a2 67, b 29, c 104, d 179, e1 127, e2 97 | < 1 s |
| 5 | P1 à P37 (`mut.py`, `make -k -j4 unit` + `make -k e2e` par mutation) | `p` | 0 | tableau, preuve 3 ; P6 et P13 : `archetype_chain` seul | 265 s |
| 6 | texte a1 appliqué ; `gcc -fsyntax-only` de `src/` et `tests/` | `a1` | 1 | 63 erreurs (membres retirés), toutes réécrites | 20 s |
| 7 | `timeout 900 make -j4 test` (1er) | `a1` | 0 | vert, mais `ui.h` nommait encore `pico8_ui_start` (commentaire) ; corrigé | 10 s |
| 8 | `make test`, `make e2e`, compteur, `--champs`, `--check inclusion-mutuelle`, `wc -l` | `a1` | 0 | vert ; 26 ; 1007 / 78 ; 603 ; 279 / 15 / 29 ; 43 ; 703 | 20 s |
| 9 | A1 à A22 | `a1` | 0 | tableau, preuve 3 | 155 s |
| 10 | texte a2 appliqué ; `-fsyntax-only` | `a2` | 1 | 56 erreurs, toutes réécrites | 20 s |
| 11 | `make test`, `make e2e`, compteur, `--champs`, `wc -l` | `a2` | 0 | vert ; 26 ; 940 / 74 ; 536 ; 276 / 15 / 32 ; 666 | 20 s |
| 12 | A23 à A39 | `a2` | 0 | tableau, preuve 3 | 92 s |
| 13 | `constproof.py` (12 `def` en `*const`) ; preuve négative | `a2` | 0 | une erreur par classe, dans son `_new` ; `read-only object` ×2 | 60 s |
| 14 | `make class-coverage-net`, `make sanitize-net` | `a2`, `a1` | 0 | `couverture: OK` ; `sanitize: 26 propre(s)` aux deux | 15 s |
| 15 | `make line-coverage EVENTS=<26 e2e>` | `a2` | 0 | les 12 poses de `def` exécutées | 60 s |
| 16 | greps de cast et de classe à `p`, `a1`, `a2` | — | 0 | 532 / 707 aux trois | < 1 s |
| 17 | commentaires `bg.h`/`research.h` qui décrivaient un membre retiré, réécrits ; a1 amendé, a2 rejoué ; `make test`, `make e2e`, compteur aux deux | `a1`, `a2` | 0 | vert ; 26 ; 1007 / 78 ; 940 / 74 | 40 s |
| 18 | `rm -rf` de la copie | — | 0 | supprimée | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas le commit 2.12e2 (ou un descendant qui ne touche que `docs/`), ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) est rouge, si un `*.sha256`, `tests/e2e/` ou une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si une jumelle de la preuve 3 est verte à l'enfant et rouge au parent (hors `archetype_chain` au parent pour P10, P30, P34, sans jumelle de test ponctuel).
- `FAILED:` si une comparaison `me`/`t` de la famille a quitte le test ponctuel, ou si une comparaison de champ propre y reste (réécrite vers `def`), ou si le total n'est pas 419 puis 407.
- `FAILED:` si `ui.hw` ou `iv.fa` n'a pas d'`assert` à littéral dans un TU durable.
- `FAILED:` si un champ A de la famille reste membre de l'instance, si `def` est écrit hors du `_new` (preuve 2), ou n'est pas `const pico8_X_def_t *`.
- `FAILED:` si un cast ou un test de classe est ajouté (compte de la réserve 4 changé), si `inclusion-mutuelle` n'est pas 17 / 16 (43 lignes), si `economy.h` inclut `game_data.h`.
- `FAILED:` si `défauts` n'est pas 603 puis 536 (ou les comptes du repli), si le pied n'est pas 1007 / 78 puis 940 / 74, si `--champs` n'est pas `279 ; 15 ; 29` puis `276 ; 15 ; 32`.
- `FAILED:` si `class-coverage-net` ou `sanitize-net` est rouge.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si le message du commit omet : les classes, les champs A retirés et les I copiés, la table des jumelles avec leur résultat au parent, la preuve `const`, les comptes, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à », ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- `mz`, `jc` et la suite (2.13b à 2.13e2), sauf le contrat plus haut.
- `pico8_ke_start` et la lecture de `me`/`t` sur `def` : 2.13e2.
- Les liens du noyau (2.14), la question Q-2.10.
- `tools/composition_residue.sh`, son TU ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Les comparaisons `me`/`t` restent** dans le test ponctuel jusqu'en 2.13e2 : leur chaîne, `pico8_ke_start`, vit encore, et elles sont le seul filet de `pico8_X_def.me`/`.t` (A21, A22, A39). Le plan dit que la fonction d'une famille part avec sa chaîne ; la famille a en a deux.
- **G2. Les filets neufs vont dans les TU de classe** (`test_pico8_ui.c` `test_built`, `test_pico8_iv.c` `test_defaults_match_the_parsed_spec`), à côté des littéraux qui y sont déjà (`ok->def->ql == 58`, `e->def->v.y == -0x570a`), plutôt que dans `spec_defaults`, qui compare l'instance à son `def`.
- **G3. `g_nf_ra` et `g_ie_id` repartent de 0 par littéral.** Ce sont des objets statiques de module, hors archétype (étude, §2) ; la valeur de classe est 0. Ajouter un membre `ra` ou `id` aux archétypes changerait deux types figés par 2.12.
- **G4. `pj_cr` par littéraux composés de portée fichier** : `g_pj_cr.hi`/`gq` sont des `const pico8_vec_t *`, sans archétype (étude, §2). Un littéral composé a une durée statique en C99 ; son adresse est une constante d'adresse. Pas de `static const pico8_vec_t` nommé à côté : le nom ne dirait rien de plus.
- **G5. `pj.ou`, `pj.ea` lus sur leurs rangées** (`pico8_pj_ou_row`, `pico8_pj_ea_row`), que `pico8_pj_start` ne faisait que pointer ; `pj.ed` (1) disparaît de `src/`, qui ne l'a jamais lu.
- **G6. `hy` : le type monte au-dessus de `pico8_hy_t`** dans `economy.h`, pour que l'instance le pointe ; le commentaire de place reste.

### Surprises de mesure, pour `plan-pilot`

1. **Deux filets uniques dans la famille a**, que les PRD 2.12a n'avaient pas cherchés : `ui.hw` (lu seulement par `bv`, via `pico8_ui_hw()`, qu'aucun e2e ne pousse à un pixel près) et `iv.fa` (la couche `fa` des toasts, qu'aucun e2e ne distingue). Les TU les comparaient à l'adresse ou à la valeur de `pico8_X_start`, jamais à un littéral.
2. **Le test ponctuel garde 20 comparaisons de la famille a jusqu'en 2.13e2** (`me`/`t` de dix classes), pas zéro : la chaîne de `ke` passe par toutes.
3. `make test` vert au premier essai de a1 malgré deux commentaires de `ui.h` qui nommaient encore `pico8_ui_start` : seul le compteur `défauts` le voit. Le mesurer avant de commiter.

### Questions pour l'humain

Aucune.
