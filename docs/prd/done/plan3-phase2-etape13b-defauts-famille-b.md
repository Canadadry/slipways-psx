---
title: "Plan 3, étape 2.13b : la famille b lit son archétype (`mz`, `jc`) et le composant sonde reçoit son `def` ; `défauts` 536 → 507, pied 940 / 74 → 911 / 74 — un commit, jeu identique"
description: "`mz` et `jc` reçoivent un `const pico8_X_def_t *def` posé par leur `_new` ; leurs champs A (`mz.fa`/`kk`/`spawn_snd`, `jc.k_`) quittent l'instance, leurs I et toute leur sonde sont copiés depuis `def`, `pico8_mz_start` et `pico8_jc_start` disparaissent. `pico8_probe_t` gagne `def` (posé à `&def->probe` par `mz`/`jc`, à NULL par `g_` et l'arbre `qh`, à `&pico8_probe_def_none` pour la sonde locale de `research_icon.c`) ; aucun lecteur ne le lit avant 2.13e2. Trois filets uniques (`mz.has_ik`, `jc.has_ik`, `jc.od`) reçoivent un `assert` à littéral. Essayé sur copie : vert, 26 e2e, 23 jumelles sans ÉCHEC."
status: needs-triage
---

## Problem Statement

**Écrit sur `3c92c74`** (branche `claude/plan-pilot-3-7zdft0`), pendant que 2.13a1/a2 s'implémentaient dans l'arbre de travail. Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier. Les essais ont été faits sur une seule copie jetable (scratchpad `prd213b/`, supprimée depuis). Sur cette copie, `3c92c74` a reçu les deux diffs du PRD 2.13a (`docs/prd/triage/plan3-phase2-etape13a-defauts-famille-a.md`, blocs `diff` de 2.13a1 et 2.13a2, `git apply` sans erreur), commités `d5ce7ce` (a1) et `7d685d2` (a2). Notation : `p` = `7d685d2`, l'arbre d'après 2.13a2 ; `b` = `654a5cd`, le commit de la copie qui porte le texte de « Ce qui change ».

**2.13a1 et 2.13a2 sont arrivées pendant l'écriture** : `3cc4b9a` et `cf1c128`. Contrôlé : `git diff 7d685d2 cf1c128 -- src tests` est **vide** (seul `docs/plan-3.md` diffère), et le diff de 2.13b passe `git apply --check` sur `cf1c128`. Toutes les mesures « au parent » ci-dessous valent donc pour `cf1c128`.

Vu du mainteneur du port : 2.13a a basculé la famille a (treize racines simples). 2.13b fait de même pour les deux racines qui portent une sonde, le trou de ver (`mz`) et la voie (`jc`). C'est aussi l'étape où le **composant sonde** reçoit son `def` (étude 2.10, §2, réserves 1 et 4 ; contrat de 2.12b).

### Texte du plan, recopié (`docs/plan-3.md` à `cf1c128`, ligne 2.13)

> - 2.13 — l'init lit l'archétype, les `pico8_X_start(_t)` et la chaîne d'init disparaissent. Chaque instance reçoit `def`, posé par son `_new`. L'init copie la part I (étude, §2), et les lectures des champs A passent par `self->def`. Les champs A quittent l'instance. Les objets statiques `g_jo_kw_shared`, `g_ga_by_shared`, `g_shared_le`, `g_nf_ra`, `g_mz_jj_wrapper` restent posés par l'init. La fonction d'une famille dans le test ponctuel part dans le commit qui retire sa chaîne, après être passée à 2.12, et le TU disparaît en 2.13e2. […] Les écritures de TU sur un champ A sont réécrites. […] `g_.fa` et la sonde `dw` passent par un archétype de test pointé par `def`. Les comptes d'arrivée sont attribués par les noms `pico8_X_start` de chaque ligne du `--check`. Preuves : chemin touché (`gcov` des inits, mutations : défaut d'archétype changé → e2e ou TU rouge) ; règle 1 : un champ A qui quitte l'instance a sa preuve `const`, rejouée sur la copie du commit ; `game_data.h` mesuré au commit. `class-coverage-net` : oui. `sanitize-net` : oui. Budget ≈ 7 × 50 min.
>   - 2.13b — `mz`, `jc`. Compte : → **507**.

Note « Découvert à 2.12b » : « Pour 2.13b : le membre `def` de `pico8_probe_t` et `pico8_probe_def_none` naissent là, et les autres porteurs de sonde doivent poser leur `def` (à `&pico8_probe_def_none` ou NULL) tant que leur famille n'a pas d'archétype. »

Étude 2.9 (`docs/etude-2-9-archetypes.md`), classement de la famille b : `mz` A `fa`, `kk`, `spawn_snd` ; I `oi` ; S `jj` (`&g_mz_jj_wrapper`, reste posé par l'init). `jc` A `k_` ; I `fa`, `nh`. Sonde : A `dw`, `dq`, `fp`, `ik`, `has_ik` ; I `od`, `has_od` ; E `et`. En-tête `me`/`t` : I. Étude 2.10, §2 : `def` dans le composant, posé par l'init du porteur à `&def->probe` ; réserve 2 : les champs A de la sonde **restent membres de `pico8_probe_t` jusqu'en 2.13e2** (les familles c à e les écrivent encore) ; réserve 4 : `pb.def = &pico8_probe_def_none` pour la sonde locale de `research_icon.c:180`.

Règles communes : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte (`*_HASH`, `FROZEN_*`) changée.

### Contrat de 2.12b (recopié) et ce que ce PRD en fait

> `probe.h` inclut déjà `game_data.h`, donc le membre `def` n'ajoute aucune inclusion ; `pico8_probe_def_none` va dans `game_data.h/.c` à côté des autres ; entre 2.13b et 2.13e2, les autres porteurs de sonde (`g__struct_init`, `qh_struct_init`, sortes de `nr`) font `malloc` sans mise à zéro : 2.13b doit poser leur `def` (à `&pico8_probe_def_none` ou NULL, jamais indéterminé) tant que leur famille n'a pas d'archétype, et aucun lecteur générique (`probe.c:168`, `:333-337`) ne bascule sur `probe->def` avant que tous les porteurs l'aient (2.13e2, étude 2.10, réserve 2).

| Porteur de `pico8_probe_t` | Où le `def` est posé (2.13b) | Valeur | Remplacé par |
|---|---|---|---|
| `mz` | `pico8_mz_struct_new` (`wormhole.c:246`) | `&self->def->probe` (= `&pico8_mz_def.probe`) | — |
| `jc` | `pico8_jc_struct_new` (`slipway.c:403`) | `&self->def->probe` (= `&pico8_jc_def.probe`) | — |
| `g_`, `fv` | `pico8_g__struct_init` (`probe.c:446`), qui sert les deux | **NULL** | 2.13c : le `def` de la classe concrète (`&pico8_g__def.probe` ou `&pico8_fv_def.probe`) |
| `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr` | `pico8_qh_struct_init` (`planet_family.c:67`), qui sert les huit | **NULL** | 2.13d : le `def` de la sorte concrète (`&pico8_X_def.probe`), avec `pico8_jo_struct_init` |
| sonde locale `pb` de `pico8_ie_kj` | `research_icon.c:180`, initialiseur | `&pico8_probe_def_none` | — |

Numéros de `b`. Tout `pico8_probe_t` a donc un `def` déterminé dès 2.13b (preuve `const` : quatre poses par affectation, plus l'initialiseur de `pb`). **NULL plutôt que `&pico8_probe_def_none`** sur `g_` et l'arbre `qh` (G2) : une lecture prématurée par `probe->def` y plante au lieu de lire des zéros faux ; `none` est réservé à la sonde sans classe.

**Quand les lecteurs basculent sur `probe->def` : en 2.13e2, pas avant.** Les lecteurs génériques des champs A de la sonde sont `probe.c:168` (`dw`), `:333` (`dq`), `:335` (`fp`), `:337` (`has_ik`/`ik`) et `planet_display.c:469` (`dw`, par `probe_of`). Ils passent à `probe->def->X` dans le commit **2.13e2**, celui qui retire `pico8_nr_start`/`pico8_fr_start` : c'est la dernière famille dont un `_new` écrit un champ A de la sonde (la famille e réécrit encore ses `_start` par-dessus en 2.13d, plan), et c'est là que `dw`, `dq`, `fp`, `ik`, `has_ik` quittent `pico8_probe_t` (étude 2.10, réserve 2). Basculer plus tôt (fin de 2.13d, quand tous les `def` pointent déjà un archétype) rendrait morte l'écriture `_start` de la famille e : sa jumelle serait rouge au parent et verte à l'enfant en 2.13e1/e2, ÉCHEC par la règle du parent. `od`/`has_od` (I) restent membres : `probe.c:255`, `:327` ne basculent jamais. Mesuré ici : la mutation B27 (`def` de `g_` posé à `&pico8_mz_def.probe` au lieu de NULL) est verte partout, et `pico8_probe_def_none.dw` changé (B26) aussi : **personne ne lit `probe->def` à `b`**.

### `test_pico8_jc.c:853` et `:893` (écritures de `probe->dw`) : pas à 2.13b

Numéros du parent ; `:854`/`:894` à `b`, l'`assert` de pose de `check_fresh_jc` les décale d'une ligne. Les deux écrivent `probe->dw = 0` sur une sonde **`g_`** (`pico8_g__new`), pas sur celle de `jc` : `test_bv` (`:853`, relevé par l'étude 2.9) et `test_bv_takes_the_first_slipway_of_cg` (`:893`, ajouté par 2.11r-d, après l'étude). À `b`, `dw` reste membre de `pico8_probe_t` et `probe.c:168` le lit sur l'instance : un archétype de test pointé par `probe->def` ne changerait rien à ce que lit `bv`, et les deux tests perdraient leur prémisse (`dw` nul). **Ils restent tels quels à 2.13b** et sont réécrits en 2.13e2, dans le commit où `probe.c:168` lit `probe->def->dw` : un `static const pico8_probe_def_t` de test (la sonde de `pico8_g__def` avec `dw = 0`) pointé par `probe->def`, une fois pour les deux fonctions. Même chose pour `g_.fa` (`test_pico8_g_.c:751`), en 2.13c, où `g_.fa` quitte l'instance. Le plan (« `g_.fa` et la sonde `dw` passent par un archétype de test ») est tenu, à l'étape où le champ quitte l'instance.

### Question ouverte chez l'humain (Q-2.10, liens du noyau)

**2.13b n'en dépend pas.** Le `def` de `mz`/`jc` est un membre de l'instance ; celui du composant est un membre de `pico8_probe_t`, que l'instance stocke (`probe_own`). Aucun n'est un lien du noyau. `mz_get_fa` lit `self->def->fa` : l'emplacement Q6 garde sa forme. Le lien `probe` du noyau (2.15a) visera `&self->probe_own` quelle que soit la réponse, ou n'existera pas ; le `def` du composant est lu à travers lui dans les deux cas.

### État de départ attendu (le commit 2.13a2, `cf1c128`, ou un descendant qui ne touche que `docs/`)

Mesuré sur `p` (`src/`, `tests/` identiques à `cf1c128`) :

| Commande | Résultat |
|---|---|
| `make composition-residue` | `parente 10 3`, `lien-parent 111 27`, `accesseurs-q6 237 33`, `défauts 536 31`, `void-ptr 8 4`, `inclusion-mutuelle 17 16` ; **`résidu 2 : 940 ligne(s), 74 fichier(s)`** |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 276 ; 0 écriture : 15 ; 0 hors constructeur : 32` |
| `--check inclusion-mutuelle \| tail -1` | `résidu gardé (inclusion-mutuelle) : 43 ligne(s)` |
| `wc -l < src/pico8_game_data.h` | 666 |
| total asserté du test ponctuel | `g_compared == 407` |
| `make test` ; `make e2e` | vert ; 26 conformes |
| lignes `défauts` qui nomment `pico8_mz_start` ou `pico8_jc_start` | 31, dont **29** attribuées à b (les deux autres, `test_pico8_map_tables.c:156` et `test_pico8_spec_defaults.c:110`, nomment aussi `pico8_ke_start`, famille e2) |
| casts / tests de classe (grep de la réserve 4 de 2.13a) | 532 / 707 |

Un départ autre : arrêt, rapporté. Si 2.13a2 n'a pas été commitée à l'identique (`git diff 7d685d2 <commit 2.13a2> -- src tests` non vide), le diff s'applique par ancres de texte : il ne touche aucun texte de 2.13a hors la ligne du total du test ponctuel.

## Solution

Un commit.

- **`def` d'instance** : `const pico8_mz_def_t *def` dans `pico8_mz_t`, `const pico8_jc_def_t *def` dans `pico8_jc_t`, premier membre après `header`, posés une seule fois dans le `_new`, juste après `pico8_ke_struct_init`.
- **Champs A qui quittent l'instance** : `mz.fa`, `mz.kk`, `mz.spawn_snd`, `jc.k_`. Lecteurs : `mz_get_fa` et `pico8_mz_kj` (`kk`) lisent `self->def->X` ; `pico8_jc_kj` lit `self->def->k_`. `spawn_snd` n'a aucun lecteur dans `src/` (audio hors périmètre) : il ne vit plus que dans l'archétype.
- **Copiés depuis `def`** : `header.me` (les deux classes le réécrivent sur la chaîne de `ke`) ; `mz.oi` ; `jc.fa`, `jc.nh` ; et les **sept** champs de la sonde (`dw`, `od`, `has_od`, `dq`, `fp`, `ik`, `has_ik`), A compris, puisqu'ils restent membres de `pico8_probe_t` jusqu'en 2.13e2. Les littéraux d'init de la sonde (`has_ik = 1`, `has_od = 1` de `mz` ; `dq = NULL`, `od = 0`, `has_od = 0`, `has_ik = 1` de `jc`) deviennent aussi des copies : sinon leur mutation d'archétype serait verte à l'enfant et rouge au parent (P8, P12, P18 à P20, P23).
- `mz.jj` reste `&g_mz_jj_wrapper`, posé par l'init.
- **Composant sonde** : `const pico8_probe_def_t *def` en tête de `pico8_probe_t`, posé comme au tableau du contrat ; `pico8_probe_def_none` (tous les champs à 0) dans `game_data.h/.c`, après `pico8_jc_def`.
- `pico8_mz_start_t`, `pico8_jc_start_t`, leurs valeurs et leurs `extern` sortent.
- **TU** : `archetype_chain` perd 23 comparaisons (garde `mz.t`, `jc.t`) ; `spec_defaults` compare `mz` à son `def` ; `mz`, `jc`, `map_tables` lisent les A par `def` ; trois `assert` à littéral neufs (filets uniques, preuve 3).

Rien d'autre : ni `tools/`, ni `Makefile`, ni `glossaire-noms.md` (aucun nom minifié neuf), ni `trous-de-portage.md`, ni `tests/e2e/`, ni `*.sha256`, ni les écritures `probe->dw` de `test_pico8_jc.c:853`/`:893` (section dédiée).

## User Stories

1. En tant que mainteneur, je veux que `mz` et `jc` lisent leurs défauts sur leur archétype, afin que `pico8_mz_start` et `pico8_jc_start` disparaissent sans changer une valeur du jeu.
2. En tant que gardien de la règle 1, je veux que les champs A de `mz`/`jc` quittent l'instance et que chaque `def` soit `const` et posé une seule fois, afin qu'aucune écriture hors construction ne soit possible.
3. En tant qu'auteur de 2.13c à 2.13e2, je veux un `def` déjà dans `pico8_probe_t`, déterminé sur chaque porteur, et la date précise où les lecteurs basculent, afin de ne ni lire un pointeur indéterminé ni rendre morte une écriture `_start` encore gardée.
4. En tant que gardien de la règle du parent, je veux qu'aucune valeur de la famille b ne perde son dernier filet quand sa comparaison part du test ponctuel.
5. En tant que gardien du test ponctuel, je veux que seules les comparaisons dont la chaîne existe encore restent (`t`, par `pico8_ke_start`), afin qu'il ne compare pas un archétype à lui-même.
6. En tant que gardien du compteur, je veux `défauts` 507, pied 911 / 74, sans nouvelle paire d'inclusion, et une explication chiffrée de `--champs`.

## Implementation Decisions

### Modules

`game_data`, `wormhole` (+ `wormhole_display`), `slipway` (+ `slipway_display`), `probe`, `planet_family` (une ligne), `research_icon` (une ligne) ; TU `archetype_chain`, `jc`, `map_tables`, `mz`, `spec_defaults` ; `docs/plan-3.md`.

### Inclusions

`wormhole.h`, `slipway.h` et `probe.h` incluent déjà `pico8_game_data.h` : aucune inclusion neuve. Mesuré : `inclusion-mutuelle` **17 / 16, 43 lignes** à `b`.

### Ce qui change

Le diff ci-dessous est le texte exact (`git apply` sur `p`, et `--check` sur `cf1c128`, passent). Lecture, en plus de la Solution :

- **Ordre dans le `_new`** : `self->def` juste après `pico8_ke_struct_init`, puis `header.me`, puis `self->probe = &self->probe_own;` suivi immédiatement de `self->probe->def = &self->def->probe;`. Les copies de sonde lisent `self->def->probe.X` (pas `self->probe->def->X`) : c'est la même adresse, et la ligne ne dépend pas du composant.
- **`pb`** : `pico8_probe_t pb = {.def = &pico8_probe_def_none};` remplace `{0}` ; l'initialiseur désigné met à zéro les autres membres comme avant. Pas d'affectation `pb.def = …` (G3).
- **Commentaires** : ceux de `wormhole.h` et `slipway.h` qui disaient où pointaient `kk` et `k_` partent avec les membres ; le reste du texte est gardé. Deux commentaires neufs, chacun un contrat ou un piège : le membre `def` de `pico8_probe_t` (« NULL on g_ and the jo tree: no generic reader may use it » ; 2.13c et 2.13d le réécrivent, 2.13e2 le retire) et l'`extern` de `pico8_probe_def_none`.
- **Test ponctuel** : voir plus bas.

```diff
diff --git a/src/pico8_game_data.c b/src/pico8_game_data.c
index 64e820d..51014e1 100644
--- a/src/pico8_game_data.c
+++ b/src/pico8_game_data.c
@@ -292,29 +292,6 @@ const pico8_ke_start_t pico8_ke_start = {
     .t = 0x0,
 };
 
-const pico8_mz_start_t pico8_mz_start = {
-    .me = "kz",
-    .dw = 0x0,
-    .dq = "pr",
-    .spawn_snd = 0x340000,
-    .ik = 0x310000,
-    .oi = 0x0,
-    .od = 0x30000,
-    .fp = 0x10000,
-    .fa = 0x50000,
-    .kk = &pico8_mz_kk_seq,
-};
-
-const pico8_jc_start_t pico8_jc_start = {
-    .me = "kz",
-    .fa = 0x20000,
-    .fp = 0,
-    .ik = 0x300000,
-    .dw = 0x106,
-    .k_ = &pico8_jc_k__seq,
-    .nh = 0x140000,
-};
-
 const pico8_g__start_t pico8_g__start = {
     .fx = 0x10000,
     .dq = "ly",
@@ -459,6 +436,9 @@ const pico8_jc_def_t pico8_jc_def = {
     .probe = {.dw = 0x106, .od = 0x0, .has_od = 0, .dq = NULL, .fp = 0x0, .ik = 0x300000, .has_ik = 1},
 };
 
+const pico8_probe_def_t pico8_probe_def_none = {
+    .dw = 0x0, .od = 0x0, .has_od = 0, .dq = NULL, .fp = 0x0, .ik = 0x0, .has_ik = 0};
+
 const pico8_g__def_t pico8_g__def = {
     .me = "ph",
     .t = 0x0,
diff --git a/src/pico8_game_data.h b/src/pico8_game_data.h
index 0aebeed..9434509 100644
--- a/src/pico8_game_data.h
+++ b/src/pico8_game_data.h
@@ -119,30 +119,6 @@ typedef struct {
     pico8_fixed_t t;
 } pico8_ke_start_t;
 
-typedef struct {
-    const char *me;
-    pico8_fixed_t dw;
-    const char *dq;
-    pico8_fixed_t spawn_snd;
-    pico8_fixed_t ik;
-    pico8_fixed_t oi;
-    pico8_fixed_t od;
-    pico8_fixed_t fp;
-    pico8_fixed_t fa;
-    const pico8_int_seq_t *kk;
-} pico8_mz_start_t;
-
-/* fp is the spec's false. */
-typedef struct {
-    const char *me;
-    pico8_fixed_t fa;
-    int fp;
-    pico8_fixed_t ik;
-    pico8_fixed_t dw;
-    const pico8_int_seq_t *k_;
-    pico8_fixed_t nh;
-} pico8_jc_start_t;
-
 /* fp is the spec's false. */
 typedef struct {
     pico8_fixed_t fx;
@@ -594,8 +570,6 @@ extern const pico8_jb_row_t pico8_pj_ou_row;
 /* lq, jb, hy and trade have an empty spec, hence no object; ui.le, nf.l and
  * nf.r are empty tables, held by their module. */
 extern const pico8_ke_start_t pico8_ke_start;
-extern const pico8_mz_start_t pico8_mz_start;
-extern const pico8_jc_start_t pico8_jc_start;
 extern const pico8_g__start_t pico8_g__start;
 extern const pico8_fv_start_t pico8_fv_start;
 extern const pico8_jo_start_t pico8_jo_start;
@@ -622,6 +596,8 @@ extern const pico8_ie_def_t pico8_ie_def;
 extern const pico8_pj_def_t pico8_pj_def;
 extern const pico8_mz_def_t pico8_mz_def;
 extern const pico8_jc_def_t pico8_jc_def;
+/* The probe of research_icon's `pb`, which has no class: every field 0. */
+extern const pico8_probe_def_t pico8_probe_def_none;
 extern const pico8_g__def_t pico8_g__def;
 extern const pico8_g__def_t pico8_fv_def;
 extern const pico8_jo_def_t pico8_jo_def;
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index b3d4d59..834edcb 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -64,6 +64,7 @@ void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls) {
     self->hp = pico8_qh_start.hp;
     self->px = pico8_qh_start.px;
     self->probe = &self->probe_own;
+    self->probe->def = NULL;
     self->probe->dq = pico8_qh_start.dq;
     self->qh = pico8_qh_start.qh;
     self->probe->od = pico8_qh_start.od;
diff --git a/src/pico8_probe.c b/src/pico8_probe.c
index f6fc3f7..020aac6 100644
--- a/src/pico8_probe.c
+++ b/src/pico8_probe.c
@@ -443,6 +443,7 @@ void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls) {
     self->ra = NULL;
     self->p = NULL;
     self->probe = &self->probe_own;
+    self->probe->def = NULL;
     self->probe->et = NULL;
     self->p_ = 0;
     self->has_p_ = 0;
diff --git a/src/pico8_probe.h b/src/pico8_probe.h
index 7b9fc95..f6d5e88 100644
--- a/src/pico8_probe.h
+++ b/src/pico8_probe.h
@@ -22,6 +22,7 @@
  * nil. `fp` is what `d_` hands to `fm:nx`: the cart's false is held as 0,
  * which is what `self.fp or 0` hands. */
 typedef struct pico8_probe {
+    const pico8_probe_def_t *def; /* NULL on g_ and the jo tree: no generic reader may use it */
     pico8_struct_header_t *et;
     pico8_fixed_t dw;
     pico8_fixed_t od;
diff --git a/src/pico8_research_icon.c b/src/pico8_research_icon.c
index 14eb398..5acf841 100644
--- a/src/pico8_research_icon.c
+++ b/src/pico8_research_icon.c
@@ -177,7 +177,7 @@ void pico8_ie_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ie_t *self, pico8_vec
      * kind-level build cost. */
     if (self->has_m_ && self->m_) {
         if (ie_self_ke_kind(self) != NULL) {
-            pico8_probe_t pb = {0};
+            pico8_probe_t pb = {.def = &pico8_probe_def_none};
 
             pb.has_od = view.has_od;
             pb.od = view.od;
diff --git a/src/pico8_slipway.c b/src/pico8_slipway.c
index c0c2868..05c8d81 100644
--- a/src/pico8_slipway.c
+++ b/src/pico8_slipway.c
@@ -396,22 +396,23 @@ static pico8_jc_t *pico8_jc_struct_new(const pico8_jc_keys_t *keys) {
     pico8_jc_t *self = malloc(sizeof *self);
 
     pico8_ke_struct_init(&self->header, &g_jc_struct_class);
-    self->header.me = pico8_jc_start.me;
-    self->fa = pico8_jc_start.fa;
+    self->def = &pico8_jc_def;
+    self->header.me = self->def->me;
+    self->fa = self->def->fa;
     self->probe = &self->probe_own;
-    self->probe->dq = NULL;
-    self->probe->fp = pico8_jc_start.fp;
-    self->probe->ik = pico8_jc_start.ik;
-    self->probe->has_ik = 1;
-    self->probe->dw = pico8_jc_start.dw;
-    self->k_ = pico8_jc_start.k_;
-    self->nh = pico8_jc_start.nh;
+    self->probe->def = &self->def->probe;
+    self->probe->dq = self->def->probe.dq;
+    self->probe->fp = self->def->probe.fp;
+    self->probe->ik = self->def->probe.ik;
+    self->probe->has_ik = self->def->probe.has_ik;
+    self->probe->dw = self->def->probe.dw;
+    self->nh = self->def->nh;
     self->qu = NULL;
     self->ts = NULL;
     self->hg = NULL;
     self->re = NULL;
-    self->probe->od = 0;
-    self->probe->has_od = 0;
+    self->probe->od = self->def->probe.od;
+    self->probe->has_od = self->def->probe.has_od;
     self->mt = 0;
     self->has_mt = 0;
     self->probe->et = NULL;
diff --git a/src/pico8_slipway.h b/src/pico8_slipway.h
index 1e7dc07..93ff90e 100644
--- a/src/pico8_slipway.h
+++ b/src/pico8_slipway.h
@@ -29,15 +29,14 @@ void pico8_jc_hg_add(pico8_jc_hg_t *hg, const char *id);
 /* `hg[id]`: 1 for an `id` already added, 0 otherwise, NULL included. */
 int pico8_jc_hg_has(const pico8_jc_hg_t *hg, const char *id);
 
-/* `k_` points at `pico8_jc_k__seq`; `ts` is this
- * slipway's own, made by `pico8_jc_pq`, which also empties `hg_own` and
- * points `hg` at it (NULL before). `fp`, `ik`, `dw`, `od` and `et` (NULL is
- * nil) live in the probe; `probe` points at `probe_own`: never copy the
- * struct. Nothing frees the struct: TROU-7. */
+/* `ts` is this slipway's own, made by `pico8_jc_pq`, which also empties
+ * `hg_own` and points `hg` at it (NULL before). `fp`, `ik`, `dw`, `od` and
+ * `et` (NULL is nil) live in the probe; `probe` points at `probe_own`: never
+ * copy the struct. Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_struct_header_t header;
+    const pico8_jc_def_t *def;
     pico8_fixed_t fa;
-    const pico8_int_seq_t *k_;
     pico8_fixed_t nh;
     pico8_struct_header_t *qu; /* borrowed, never freed */
     pico8_ptrs_t *ts;
diff --git a/src/pico8_slipway_display.c b/src/pico8_slipway_display.c
index 3b61d85..f4f9f1c 100644
--- a/src/pico8_slipway_display.c
+++ b/src/pico8_slipway_display.c
@@ -106,9 +106,9 @@ void pico8_jc_kj(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jc_t *self) {
 
         assert(nh_i >= 1 && "k_ is 1-based: an nh of 0 would silently draw the slipway black");
 
-        if (self->k_ != NULL) {
+        if (self->def->k_ != NULL) {
             int entry;
-            if (pico8_int_seq_at(self->k_->items, self->k_->count, nh_i, &entry)) {
+            if (pico8_int_seq_at(self->def->k_->items, self->def->k_->count, nh_i, &entry)) {
                 jv = pico8_fixed_from_int(entry);
             }
         }
diff --git a/src/pico8_wormhole.c b/src/pico8_wormhole.c
index 13d98ba..17186ff 100644
--- a/src/pico8_wormhole.c
+++ b/src/pico8_wormhole.c
@@ -200,7 +200,7 @@ static pico8_probe_t *mz_probe_of(void *self) {
 }
 
 static int mz_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_mz_t *)self)->fa;
+    *out = ((const pico8_mz_t *)self)->def->fa;
     return 1;
 }
 
@@ -240,19 +240,18 @@ static pico8_mz_t *pico8_mz_struct_new(void) {
     pico8_mz_t *self = malloc(sizeof *self);
 
     pico8_ke_struct_init(&self->header, &g_mz_struct_class);
-    self->header.me = pico8_mz_start.me;
+    self->def = &pico8_mz_def;
+    self->header.me = self->def->me;
     self->probe = &self->probe_own;
-    self->probe->dw = pico8_mz_start.dw;
-    self->probe->dq = pico8_mz_start.dq;
-    self->spawn_snd = pico8_mz_start.spawn_snd;
-    self->probe->ik = pico8_mz_start.ik;
-    self->probe->has_ik = 1;
-    self->oi = pico8_mz_start.oi;
-    self->probe->od = pico8_mz_start.od;
-    self->probe->has_od = 1;
-    self->probe->fp = pico8_mz_start.fp;
-    self->fa = pico8_mz_start.fa;
-    self->kk = pico8_mz_start.kk;
+    self->probe->def = &self->def->probe;
+    self->probe->dw = self->def->probe.dw;
+    self->probe->dq = self->def->probe.dq;
+    self->probe->ik = self->def->probe.ik;
+    self->probe->has_ik = self->def->probe.has_ik;
+    self->oi = self->def->oi;
+    self->probe->od = self->def->probe.od;
+    self->probe->has_od = self->def->probe.has_od;
+    self->probe->fp = self->def->probe.fp;
     self->jj = &g_mz_jj_wrapper;
     self->ox = NULL;
     self->ra = NULL;
diff --git a/src/pico8_wormhole.h b/src/pico8_wormhole.h
index 2d8b31b..f942967 100644
--- a/src/pico8_wormhole.h
+++ b/src/pico8_wormhole.h
@@ -22,16 +22,14 @@ typedef struct {
     const char *qs;
 } pico8_mz_jj_t;
 
-/* `kk` points at `pico8_mz_kk_seq`, `jj` at the
- * one `pico8_mz_jj_t`; all shared by every `mz` and never modified in place.
- * `dw`, `dq`, `ik`, `od`, `fp` and `et` live in the probe; `probe` points at
- * `probe_own`: never copy the struct. Nothing frees the struct: TROU-7. */
+/* `jj` points at the one `pico8_mz_jj_t`, shared by every `mz` and never
+ * modified in place. `dw`, `dq`, `ik`, `od`, `fp` and `et` live in the probe;
+ * `probe` points at `probe_own`: never copy the struct. Nothing frees the
+ * struct: TROU-7. */
 typedef struct {
     pico8_struct_header_t header;
-    pico8_fixed_t spawn_snd;
+    const pico8_mz_def_t *def;
     pico8_fixed_t oi;
-    pico8_fixed_t fa;
-    const pico8_int_seq_t *kk;
     pico8_mz_jj_t *jj;
     pico8_ents_t *ox;
     pico8_vec_t *ra;
diff --git a/src/pico8_wormhole_display.c b/src/pico8_wormhole_display.c
index 859e316..decdaf0 100644
--- a/src/pico8_wormhole_display.c
+++ b/src/pico8_wormhole_display.c
@@ -48,7 +48,7 @@ void pico8_mz_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_mz_t *self, pico8_vec
     } else {
         pico8_fixed_t t = self->header.t;
         pico8_fixed_t oi = self->oi;
-        int has_kk = self->kk != NULL;
+        int has_kk = self->def->kk != NULL;
         int dt;
 
         for (dt = 0; dt <= 8; dt += 2) {
@@ -63,7 +63,7 @@ void pico8_mz_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_mz_t *self, pico8_vec
                     pico8_fixed_t ratio = pico8_fixed_mul(pico8_fixed_div(sr, oi), pico8_fixed_from_int(8));
                     int idx = pico8_fixed_flr_int(pico8_fixed_flr(ratio));
 
-                    pico8_int_seq_at(self->kk->items, self->kk->count, idx, &color);
+                    pico8_int_seq_at(self->def->kk->items, self->def->kk->count, idx, &color);
                 }
 
                 if (gfx != NULL && mem != NULL) {
diff --git a/tests/test_pico8_archetype_chain.c b/tests/test_pico8_archetype_chain.c
index 08719bc..a7d205f 100644
--- a/tests/test_pico8_archetype_chain.c
+++ b/tests/test_pico8_archetype_chain.c
@@ -154,19 +154,7 @@ static void test_mz(void) {
     const pico8_mz_t *mz = pico8_mz_new();
     const pico8_mz_t *other = pico8_mz_new();
 
-    same(same_text(mz->header.me, pico8_mz_def.me), "mz.me");
     same(mz->header.t == pico8_mz_def.t, "mz.t");
-    same(mz->spawn_snd == pico8_mz_def.spawn_snd, "mz.spawn_snd");
-    same(mz->oi == pico8_mz_def.oi, "mz.oi");
-    same(mz->fa == pico8_mz_def.fa, "mz.fa");
-    same(mz->kk == pico8_mz_def.kk, "mz.kk");
-    same(mz->probe->dw == pico8_mz_def.probe.dw, "mz.probe.dw");
-    same(mz->probe->od == pico8_mz_def.probe.od, "mz.probe.od");
-    same(mz->probe->has_od == pico8_mz_def.probe.has_od, "mz.probe.has_od");
-    same(same_text_or_nil(mz->probe->dq, pico8_mz_def.probe.dq), "mz.probe.dq");
-    same(mz->probe->fp == pico8_mz_def.probe.fp, "mz.probe.fp");
-    same(mz->probe->ik == pico8_mz_def.probe.ik, "mz.probe.ik");
-    same(mz->probe->has_ik == pico8_mz_def.probe.has_ik, "mz.probe.has_ik");
     assert(mz->jj != NULL && mz->jj == other->jj);
     assert(mz->probe->et == NULL);
 }
@@ -174,18 +162,7 @@ static void test_mz(void) {
 static void test_jc(void) {
     const pico8_jc_t *jc = pico8_jc_new(NULL);
 
-    same(same_text(jc->header.me, pico8_jc_def.me), "jc.me");
     same(jc->header.t == pico8_jc_def.t, "jc.t");
-    same(jc->fa == pico8_jc_def.fa, "jc.fa");
-    same(jc->k_ == pico8_jc_def.k_, "jc.k_");
-    same(jc->nh == pico8_jc_def.nh, "jc.nh");
-    same(jc->probe->dw == pico8_jc_def.probe.dw, "jc.probe.dw");
-    same(jc->probe->od == pico8_jc_def.probe.od, "jc.probe.od");
-    same(jc->probe->has_od == pico8_jc_def.probe.has_od, "jc.probe.has_od");
-    same(same_text_or_nil(jc->probe->dq, pico8_jc_def.probe.dq), "jc.probe.dq");
-    same(jc->probe->fp == pico8_jc_def.probe.fp, "jc.probe.fp");
-    same(jc->probe->ik == pico8_jc_def.probe.ik, "jc.probe.ik");
-    same(jc->probe->has_ik == pico8_jc_def.probe.has_ik, "jc.probe.has_ik");
     assert(jc->probe->et == NULL);
 }
 
@@ -463,7 +440,7 @@ int main(void) {
     test_ga();
     test_nr();
     test_fr();
-    assert(g_compared == 407 && "families a to e compare 407 fields: one missed or one too many");
+    assert(g_compared == 384 && "families a to e compare 384 fields: one missed or one too many");
 
     printf("test_pico8_archetype_chain: all tests passed\n");
     return 0;
diff --git a/tests/test_pico8_jc.c b/tests/test_pico8_jc.c
index 6b5c0d3..3f969e7 100644
--- a/tests/test_pico8_jc.c
+++ b/tests/test_pico8_jc.c
@@ -441,16 +441,17 @@ static void check_fresh_jc(const void *e, const void *qu) {
     const pico8_jc_hg_t *hg;
     size_t i;
 
+    assert(jc->def == &pico8_jc_def && jc->probe->def == &pico8_jc_def.probe);
     assert(jc->header.me != NULL && strcmp(jc->header.me, "kz") == 0);
     assert(jc->header.t == 0);
     assert(jc->fa == pico8_fixed_from_int(2));
     assert(jc->probe->fp == 0);
-    assert(jc->probe->ik == pico8_fixed_from_int(48));
+    assert(jc->probe->ik == pico8_fixed_from_int(48) && jc->probe->has_ik == 1);
     printf("[dw] 0x%08X\n", (unsigned)jc->probe->dw);
     assert(jc->probe->dw == (pico8_fixed_t)DW_BITS);
     assert(jc->nh == pico8_fixed_from_int(20));
 
-    k_ = jc->k_;
+    k_ = jc->def->k_;
     assert(k_ == &pico8_jc_k__seq && k_->count == 20);
     for (i = 0; i < 20; i++) {
         assert(k_->items[i] == want_k_[i]);
@@ -462,7 +463,7 @@ static void check_fresh_jc(const void *e, const void *qu) {
     hg = jc->hg;
     assert(hg == &jc->hg_own && hg->count == 0);
 
-    assert(jc->re == NULL && !jc->probe->has_od && !jc->has_mt && jc->probe->et == NULL);
+    assert(jc->re == NULL && !jc->probe->has_od && jc->probe->od == 0 && !jc->has_mt && jc->probe->et == NULL);
     assert(!cls_of(jc)->get_mt(jc, &mt) && mt == 0);
     with_mt.mt = pico8_fixed_from_int(3);
     with_mt.has_mt = 1;
@@ -494,7 +495,7 @@ static void test_construction(void) {
     by_lookup = build_jc(ly);
     check_fresh_jc(by_lookup, ly);
 
-    assert(as_jc(by_ldown)->k_ == as_jc(by_lookup)->k_);
+    assert(as_jc(by_ldown)->def == as_jc(by_lookup)->def);
     assert(as_jc(by_ldown)->header.cls->tags == as_jc(by_lookup)->header.cls->tags);
     assert(as_jc(by_ldown)->ts != as_jc(by_lookup)->ts);
     assert(as_jc(by_ldown)->hg != as_jc(by_lookup)->hg);
diff --git a/tests/test_pico8_map_tables.c b/tests/test_pico8_map_tables.c
index 6f84717..47cffc4 100644
--- a/tests/test_pico8_map_tables.c
+++ b/tests/test_pico8_map_tables.c
@@ -153,7 +153,7 @@ static void test_jo_tree_strings_are_distinct_objects(void) {
     const char *const tree[] = {pico8_jo_start.me, pico8_qh_start.me, pico8_qh_start.dq, pico8_nl_start.rr,
         pico8_hj_start.rr, pico8_pm_start.fz, pico8_pm_start.me, pico8_ga_start.fz, pico8_nr_start.rr,
         pico8_fr_start.rr, pico8_mx_start.rr};
-    const char *const others[] = {pico8_ke_start.me, pico8_mz_start.me, pico8_mz_start.dq, pico8_jc_start.me,
+    const char *const others[] = {pico8_ke_start.me, pico8_mz_def.me, pico8_mz_def.probe.dq, pico8_jc_def.me,
         pico8_g__start.dq, pico8_mz_jj_qs};
     int n = (int)(sizeof tree / sizeof tree[0]);
     int i;
diff --git a/tests/test_pico8_mz.c b/tests/test_pico8_mz.c
index d3a269e..f6e52b4 100644
--- a/tests/test_pico8_mz.c
+++ b/tests/test_pico8_mz.c
@@ -308,15 +308,15 @@ static void check_fresh_mz(void *e) {
     assert(mz->header.t == 0);
     assert(mz->probe->dw == 0);
     assert(mz->probe->dq != NULL && strcmp(mz->probe->dq, "pr") == 0);
-    assert(mz->spawn_snd == pico8_fixed_from_int(52));
-    assert(mz->probe->ik == pico8_fixed_from_int(49));
+    assert(mz->def->spawn_snd == pico8_fixed_from_int(52));
+    assert(mz->probe->ik == pico8_fixed_from_int(49) && mz->probe->has_ik == 1);
     assert(mz->oi == 0);
     assert(mz->probe->od == pico8_fixed_from_int(3));
     assert(mz->probe->fp == pico8_fixed_from_int(1));
-    assert(mz->fa == pico8_fixed_from_int(5));
-    assert(mz->header.cls->get_fa(mz, &fa) && fa == mz->fa);
+    assert(mz->def->fa == pico8_fixed_from_int(5));
+    assert(mz->header.cls->get_fa(mz, &fa) && fa == mz->def->fa);
 
-    kk = mz->kk;
+    kk = mz->def->kk;
     assert(kk == &pico8_mz_kk_seq && kk->count == 9);
     for (i = 0; i < 9; i++) {
         assert(kk->items[i] == want_kk[i]);
diff --git a/tests/test_pico8_spec_defaults.c b/tests/test_pico8_spec_defaults.c
index 1e5ba4e..2240c3d 100644
--- a/tests/test_pico8_spec_defaults.c
+++ b/tests/test_pico8_spec_defaults.c
@@ -107,13 +107,13 @@ static void test_qb_defaults_and_ready(void) {
 static void test_mz_defaults_and_ready(void) {
     pico8_mz_t *mz = pico8_mz_new();
 
-    assert(mz->header.me == pico8_mz_start.me && strcmp(mz->header.me, "kz") == 0 && mz->header.t == pico8_ke_start.t);
-    assert(mz->kk == pico8_mz_start.kk);
-    assert(mz->probe->dw == pico8_mz_start.dw && mz->probe->dq == pico8_mz_start.dq);
-    assert(mz->spawn_snd == pico8_mz_start.spawn_snd && mz->probe->ik == pico8_mz_start.ik);
-    assert(mz->oi == pico8_mz_start.oi);
-    assert(mz->probe->od == pico8_mz_start.od && mz->probe->od == pico8_fixed_from_int(3));
-    assert(mz->probe->fp == pico8_mz_start.fp && mz->fa == pico8_mz_start.fa);
+    assert(mz->def == &pico8_mz_def && mz->probe->def == &pico8_mz_def.probe);
+    assert(mz->header.me == mz->def->me && strcmp(mz->header.me, "kz") == 0 && mz->header.t == pico8_ke_start.t);
+    assert(mz->probe->dw == mz->def->probe.dw && mz->probe->dq == mz->def->probe.dq);
+    assert(mz->probe->ik == mz->def->probe.ik && mz->probe->has_ik == mz->def->probe.has_ik);
+    assert(mz->oi == mz->def->oi);
+    assert(mz->probe->od == mz->def->probe.od && mz->probe->od == pico8_fixed_from_int(3));
+    assert(mz->probe->has_od == mz->def->probe.has_od && mz->probe->fp == mz->def->probe.fp);
     assert(mz->jj == pico8_mz_jj_wrapper() && mz->jj->qs == pico8_mz_jj_qs);
     assert(pico8_mz_ready());
     pico8_wormhole_shutdown();
```

**`docs/plan-3.md`** — note « Découvert à 2.13b » au bout de la ligne 2.13b : comptes (`défauts` **507 / 29**, pied **911 / 74**, `--champs` **274 ; 15 ; 20** et pourquoi « 0 hors constructeur » baisse, `inclusion-mutuelle` 17 / 16), `game_data.h` **642** lignes, total du test ponctuel **384** (`t` gardé, `me` parti : `mz`/`jc` le réécrivent depuis `def`), les trois filets neufs, les poses du `def` de sonde (NULL sur `g_` et l'arbre `qh`, à remplacer en 2.13c et 2.13d), la bascule des lecteurs en 2.13e2, `test_pico8_jc.c:853`/`:893` reportés à 2.13e2, le repli s'il a servi. Le reste va au message de commit.

### Test ponctuel : ce qui part, ce qui reste

Même règle qu'en 2.13a : une comparaison part dans le commit qui retire sa chaîne, jamais réécrite vers `def`. Différence avec la famille a : `mz` et `jc` **réécrivent `header.me`** (`"kz"`) par-dessus la chaîne de `ke`. À `b`, `header.me = self->def->me` : la comparaison `me` serait vraie par construction, elle part. `t` vient encore de `pico8_ke_start` (`pico8_ke_struct_init`) : sa comparaison reste, et elle est le seul filet de `pico8_mz_def.t`/`pico8_jc_def.t` (B24, B25 rouges au seul `archetype_chain`).

| Fonction | Garde | Retire | Total asserté |
|---|---|---|---|
| départ (`p`) | — | — | 407 |
| `test_mz` | `mz.t` ; `assert` nus `jj` partagé, `et` NULL | 12 : `me`, `spawn_snd`, `oi`, `fa`, `kk`, sonde `dw`, `od`, `has_od`, `dq`, `fp`, `ik`, `has_ik` | |
| `test_jc` | `jc.t` ; `assert` nu `et` NULL | 11 : `me`, `fa`, `k_`, `nh`, sonde ×7 | **384** |

`same_text_or_nil` reste employé (`g_`, `fv`, arbre `qh`). Le message du total garde sa forme (`"families a to e compare 384 fields: …"`).

### Comptes, mesurés sur la copie

| Mesure | `p` (= `cf1c128`) | `b` |
|---|---|---|
| `défauts` (lignes / fichiers) | 536 / 31 | **507 / 29** |
| pied `résidu 2` | 940 / 74 | **911 / 74** |
| `parente`, `lien-parent`, `accesseurs-q6`, `void-ptr` | 10/3, 111/27, 237/33, 8/4 | identiques |
| `inclusion-mutuelle` (`--check`) | 17 / 16, 43 lignes | **17 / 16, 43** |
| `--champs` | 276 ; 15 ; 32 | **274 ; 15 ; 20** |
| `wc -l src/pico8_game_data.h` | 666 | **642** |
| total du test ponctuel | 407 | **384** |
| casts / tests de classe | 532 / 707 | 532 / 707 |

Attribution des 29 lignes (script du PRD 2.13a : `re.findall(r'\bpico8_([a-z0-9_]+?)_start(?:_t)?\b', ligne)`, famille = la plus tardive) : `mz` 19 (`game_data.h` type et `extern`, `game_data.c` valeur, `wormhole.c` 10, `spec_defaults.c:111-116` 6), `jc` 10 (type, `extern`, valeur, `slipway.c` 7). `map_tables.c:156` et `spec_defaults.c:110` sont réécrites mais restent comptées : elles nomment `pico8_ke_start` (e2). Fichiers : `wormhole.c` et `slipway.c` sortent de `défauts` (31 → 29) ; le pied garde 74 fichiers (les deux sont encore comptés par `lien-parent`/`accesseurs-q6`).

`--champs`, lu : quatre A retirés (`mz.fa`, `kk`, `spawn_snd`, `jc.k_`) et deux `def` ajoutés, 276 → **274** (`pico8_probe_t` n'est pas un type d'entité : son `def` n'y figure pas). « 0 hors constructeur » **32 → 20** : le relevé compte **par nom de membre** (2.12b l'a déjà noté pour `probe`/`emitter`). L'initialiseur `pb = {.def = …}` est dans `pico8_ie_kj`, qui ne finit ni par `_new` ni par `_init` : le nom `def` y compte une écriture hors constructeur, ce qui sort les **quatorze** couples `<type> def` du groupe (12 de 2.13a, plus `mz`, `jc`) ; 32 + 2 − 14 = 20. Les autres lignes qui bougent (`fa`, `oi`, `nh`, `me`, `has_od`, `od` : écritures `= self->def->X` qui ne sont plus des copies `pico8_X_start.X`, retirées par le relevé) ne changent aucun groupe. Attendu : la règle 1 est prouvée par la preuve `const` (preuve 2), pas par ce compte ; le relevé ne distingue pas l'initialisation d'une locale d'une écriture.

### Contrat pour 2.13c à 2.13e2 (à recopier dans leurs PRD ; complète celui de 2.13a)

| Point | Sous-étape | Ce qu'elle doit faire |
|---|---|---|
| `def` de la sonde de `g_`/`fv` | 2.13c | remplacer `self->probe->def = NULL;` de `pico8_g__struct_init` par le `def` de la classe concrète (`fv` passe par la même init : poser dans chaque `_new`, ou passer le `def` à l'init) ; copier la sonde depuis ce `def` comme ici ; ne **pas** basculer les lecteurs |
| `def` de la sonde de l'arbre `qh` | 2.13d | idem dans `pico8_qh_struct_init` pour les huit sortes, avec le `def` de planète de la sorte concrète ; les `_new` de la famille e réécrivent encore leurs `_start` de sonde (`nr`, `fr` masquent `dw`, `od`, `fp`) : ne pas basculer |
| bascule des lecteurs | **2.13e2** | `probe.c:168`, `:333`, `:335`, `:337`, `planet_display.c:469` lisent `probe->def->X` ; `dw`, `dq`, `fp`, `ik`, `has_ik` quittent `pico8_probe_t` ; les copies de ces cinq champs dans les inits (`mz`, `jc` ici, `g_`, arbre `qh`) partent ; le commentaire du membre `def` part |
| `test_pico8_jc.c:853`, `:893` | 2.13e2 | archétype de test à `dw` nul pointé par `probe->def` (section dédiée) |
| `pico8_probe_def_none` | — | aucun lecteur, même après 2.13e2 (`pico8_dl_show` ne lit que `et`, `has_od`, `od`) ; il reste la cible de `pb.def`, pour qu'aucun `pico8_probe_t` n'ait un `def` NULL après 2.13e2 |
| `t` de `mz`, `jc` | 2.13e2 | avec les `me`/`t` de la famille a : quand `pico8_ke_start` part, leurs comparaisons partent ; `check_fresh_mz`/`check_fresh_jc` ont déjà `header.t == 0` à littéral |
| filets uniques connus | 2.13c à 2.13e2 | table du PRD 2.13a, inchangée (`probe.has_od` de `g_`, `probe.has_ik` de l'arbre `qh`, `emitter.pn` de `ga`, `me`/`t` de la famille a, `nf.ra`/`ui.le`) |

### Anti-tassement

**Interdit** :
- réécrire une comparaison du test ponctuel vers `def` au lieu de la retirer ; retirer `mz.t` ou `jc.t` avant 2.13e2 ;
- basculer un lecteur (`probe.c`, `planet_display.c`) sur `probe->def` ; retirer un champ A de `pico8_probe_t` ;
- laisser un `def` de `pico8_probe_t` indéterminé sur un porteur, ou le poser ailleurs que dans l'init du porteur ;
- garder un littéral d'init de sonde de `mz`/`jc` au lieu de la copie depuis `def` ;
- un cast, un `void *`, un aiguillage par classe pour atteindre `def` ;
- laisser un champ A de `mz`/`jc` dans l'instance « pour les TU » ; mettre `g_mz_jj_wrapper` dans un archétype ; changer un type `pico8_X_def_t` ;
- nommer `pico8_mz_start` ou `pico8_jc_start`, même en commentaire ;
- toucher `test_pico8_jc.c:853`/`:893` ;
- changer une constante `*_HASH`/`FROZEN_*`, un `*.sha256`, `tests/e2e/`, `tools/`, le `Makefile`.

## Implementation Chunks

Un commit, **≈ 35 min** preuves comprises (essai : ≈ 10 min d'écriture, ≈ 25 min de preuves, dont ≈ 8 min de mutations). Budget 50 min.

1. appliquer le diff ; `make test`, `make e2e`, compteur, `--champs` ;
2. preuves 1 à 8 ; note du plan ; commit.

**Repli écrit** (si l'étape déborde de 1,5 fois son budget avant le commit), chacun un commit vert :
- **2.13b1** : `mz`, le membre `def` de `pico8_probe_t`, `pico8_probe_def_none`, les poses NULL de `g_` et `qh` (le `def` de sonde de `jc` posé NULL en attendant b2) ; `map_tables.c:156` réécrit pour `mz` seul → `défauts` **517**, total **395**, filet `mz.has_ik` ;
- **2.13b2** : `jc` → **507**, **384**, filets `jc.has_ik`, `jc.od`.
Comptes du repli calculés par l'attribution (19 + 10), pas mesurés. Le repli est écrit dans la note « Découvert à ».

## Testing Decisions

- **Aucun TU neuf.** Trois `assert` à littéral neufs et deux `assert` de pose dans des TU existants, le reste est réécriture. Aucun rejeu d'`events.json` dans un TU.
- **Bon test ici** : un TU durable compare une valeur à un **littéral du cart** (`mz->probe->has_ik == 1`, `jc->probe->od == 0`), jamais l'archétype à lui-même. `spec_defaults` compare l'instance à son `def` (`mz->def == &pico8_mz_def`, `mz->oi == mz->def->oi`) : cela prouve la pose et la copie, pas la valeur.
- **Prior art** : preuves 1 à 8 du PRD 2.13a ; mutations et jumelles de 2.12b.

### Preuves (parent = le commit 2.13a2)

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. Mesuré à `b`.

2. **Règle 1, preuve `const`** (copie du commit ; chaque membre `def` rendu `*const def`, un à la fois, puis `cc -std=c99 -fsyntax-only -Isrc` de chaque `.c` de `src/` et `tests/`, l'erreur `raylib.h` de base ôtée) : `pico8_mz_t.def` → **une** erreur, `wormhole.c:243` ; `pico8_jc_t.def` → **une**, `slipway.c:399` ; `pico8_probe_t.def` → **quatre**, les poses `wormhole.c:246`, `slipway.c:403`, `probe.c:446`, `planet_family.c:67` (l'initialiseur de `pb` n'est pas une affectation). Aucune écriture de `def` hors construction. Preuve négative : `m->def->fa = 0;` sur un `pico8_mz_t *` et `j->probe->def->dw = 0;` sur un `pico8_jc_t *` → `assignment of member … in read-only object`. Les A retirés n'ont plus de membre (grep de Sortie 1 et 3).

3. **Mutations, règle du parent.** « Défaut de classe changé » : au parent, sur `pico8_mz_start`/`pico8_jc_start` ou le littéral d'init de `wormhole.c`/`slipway.c` ; à l'enfant, sur `pico8_mz_def`/`pico8_jc_def`. `make -k -j4 unit` puis `make -k e2e` ; restaurer. **Vert à l'enfant et rouge au parent = ÉCHEC.** Au parent, chaque mutation rougit aussi `archetype_chain` ; la colonne l'omet. e2e = conformes sur 26.

   | # parent / enfant | Valeur, mutation | TU rouges au parent (hors `archetype_chain`) | TU rouges à l'enfant | e2e (p = b) |
   |---|---|---|---|---|
   | P1 / B1 | `mz.me` `"kz"` → `"ph"` | `bg`, `mz`, `spec_defaults`, `ui` | idem | 8 |
   | P2 / B2 | `mz.spawn_snd` 52 → 53 | `mz` | idem | 26 |
   | P3 / B3 | `mz.oi` 0 → 1 | `mz` | idem | 26 |
   | P4 / B4 | `mz.fa` 5 → 4 | `mz` | idem | 25 |
   | P5 / B5 | `mz.kk` → `&pico8_jc_k__seq` | `mz` | idem | 9 |
   | P6 / B6 | sonde `mz.dw` 0 → `0x100` | `mz` | idem | 26 |
   | P7 / B7 | sonde `mz.od` 3 → 4 | `mz`, `spec_defaults` | idem | 9 |
   | P8 / B8 | sonde `mz.has_od` 1 → 0 (parent : littéral d'init) | `mz` | idem | 9 |
   | P9 / B9 | sonde `mz.dq` `"pr"` → `"ly"` | `mz` | idem | 9 |
   | P10 / B10 | sonde `mz.fp` 1 → 2 | `mz` | idem | 9 |
   | P11 / B11 | sonde `mz.ik` 49 → 50 | `mz` | idem | 26 |
   | **P12 / B12** | sonde `mz.has_ik` 1 → 0 (parent : littéral) | **aucun** | **`mz`** (filet neuf) | 26 |
   | P13 / B13 | `jc.me` `"kz"` → `"ph"` | `jc`, `ui` | idem | 17 |
   | P14 / B14 | `jc.fa` 2 → 3 | `jc` | idem | 26 |
   | P15 / B15 | `jc.k_` → `&pico8_mz_kk_seq` | `jc` | idem | 17 |
   | P16 / B16 | `jc.nh` 20 → 21 | `jc` | idem | 17 |
   | P17 / B17 | sonde `jc.dw` `0x106` → `0x107` | `jc` | idem | 26 |
   | **P18 / B18** | sonde `jc.od` 0 → 1 (parent : littéral) | **aucun** | **`jc`** (filet neuf) | 26 |
   | P19 / B19 | sonde `jc.has_od` 0 → 1 (parent : littéral) | `jc` | idem | 26 |
   | P20 / B20 | sonde `jc.dq` NULL → `"pr"` (parent : littéral) | `jc` | idem | 26 |
   | P21 / B21 | sonde `jc.fp` 0 → 1 | `hp`, `hy`, `jc`, `lq` | idem | 17 |
   | P22 / B22 | sonde `jc.ik` 48 → 49 | `jc` | idem | 26 |
   | **P23 / B23** | sonde `jc.has_ik` 1 → 0 (parent : littéral) | **aucun** | **`jc`** (filet neuf) | 26 |
   | — / B24 | `pico8_mz_def.t` 0 → 1 | (au parent : `archetype_chain` seul) | `archetype_chain` seul | 26 |
   | — / B25 | `pico8_jc_def.t` 0 → 1 | (idem) | `archetype_chain` seul | 26 |
   | — / B26 | `pico8_probe_def_none.dw` 0 → `0x100` | — | aucun (sans lecteur, attendu) | 26 |
   | — / B27 | `def` de sonde de `g_`/`fv` : NULL → `&pico8_mz_def.probe` | — | aucun (sans lecteur, attendu) | 26 |

   Lecture : **aucun ÉCHEC**. Hors `archetype_chain`, chaque jumelle rougit à l'enfant exactement ce qu'elle rougit au parent. **Filets uniques trouvés** (rejeu des 23 jumelles de chaîne au parent) : P12 (`mz.has_ik`), P18 (`jc.od`, valeur morte : `od` n'est lu que si `has_od`, nul pour `jc`) et P23 (`jc.has_ik`). Chacun reçoit un `assert` à littéral dans le TU de sa classe **avant** que sa comparaison parte (dans le même commit) : `test_pico8_mz.c` `check_fresh_mz` (`mz->probe->has_ik == 1`), `test_pico8_jc.c` `check_fresh_jc` (`jc->probe->has_ik == 1`, `jc->probe->od == 0`) ; B12, B18, B23 rouges à l'enfant par eux. B24, B25 montrent que `t` doit rester. B26, B27 prouvent qu'aucun lecteur n'emploie `probe->def` à `b`. P1 à P23 mesurées sur `p` (identique à `cf1c128` pour `src/`, `tests/`), B1 à B27 sur `b`.

4. **Chemin touché** : `make line-coverage EVENTS="<les 26 e2e>"` sur `b` : les poses et copies (`wormhole.c:243-253`, `slipway.c:399-415`), les poses NULL (`probe.c:446`, `planet_family.c:67`), l'initialiseur de `pb` (`research_icon.c:180`) et les lectures par `def` (`wormhole.c:203`, `wormhole_display.c:51`, `:66`, `slipway_display.c:109`, `:111`) sont exécutés. Les valeurs sont prouvées par la preuve 3.

5. **Empreintes** : aucune constante ne change ; le pli `fold_fixed(h, 1, jc->fa)` de `test_pico8_jc.c:261` lit un I, toujours membre, inchangé. Vert.

6. **Inclusion, casts, aiguillage** : `inclusion-mutuelle` **17 / 16, 43 lignes** ; casts 532, tests de classe 707, comme au parent. La seule ligne « cast » du diff est l'accesseur existant `((const pico8_mz_t *)self)->fa` → `…->def->fa`.

7. `class-coverage-net` : **oui**, vert à `b` (`couverture: OK`, comptes égaux à `tools/class_coverage_net_expected.txt`). `sanitize-net` : **oui** (le composant grandit d'un pointeur), `sanitize: 26 propre(s)` à `b`.

8. `game_data.h` mesuré au commit : **642**.

### Greps de Sortie (arbre du commit)

1. `grep -rnE 'pico8_(mz|jc)_start' src tests` → vide.
2. `grep -rn 'const pico8_[a-z_]*_def_t \*def;' src | wc -l` → **15** (12 de 2.13a, `wormhole.h`, `slipway.h`, `probe.h`).
3. `grep -rnE 'self->def = &pico8_[a-z_]*_def;' src | wc -l` → **14**, chacune dans un `pico8_X_struct_new`.
4. `grep -rnE 'probe->def = |\.def = ' src` → exactement : `probe.c:446` (NULL), `planet_family.c:67` (NULL), `wormhole.c:246`, `slipway.c:403` (`&self->def->probe`), `research_icon.c:180` (`{.def = &pico8_probe_def_none}`).
5. `grep -rnE 'probe->def->|pb\.def|->def->(dw|dq|fp|ik|has_ik)' src` → vide (aucun lecteur du `def` de sonde).
6. Casts et tests de classe : `grep -rEo '\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)' src tests | wc -l` → 532 ; `grep -rEo '(->cls|header\.cls|_struct_class\(\))' src tests | wc -l` → 707.
7. `grep -n 'g_compared ==' tests/test_pico8_archetype_chain.c` → **384**.
8. `make composition-residue` → pied **911 / 74**, `défauts 507 29`, `inclusion-mutuelle 17 16` ; `--champs` → `274 ; 15 ; 20`.
9. `git diff --stat <parent>` : les fichiers de la section « Modules » et `docs/plan-3.md`, rien d'autre ; `tests/test_pico8_jc.c` ne change pas aux lignes `probe->dw = 0`.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable (`p` = `7d685d2`, `b` = `654a5cd`).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout 3c92c74`, `git apply` des deux diffs du PRD 2.13a, commits a1/a2 | `p` | 0 | `d5ce7ce`, `7d685d2` | 3 s |
| 2 | `timeout 900 make -j8 test` ; `timeout 600 make e2e` | `p` | 0 | vert ; 26 conformes | non chronométré (< 1 min) |
| 3 | compteur, `--champs`, `--check inclusion-mutuelle`, `wc -l`, `--check défauts` + attribution | `p` | 0 | 940 / 74 ; 536 ; 276 / 15 / 32 ; 43 ; 666 ; b 29 | < 5 s |
| 4 | P1 à P23 (`mut.py` : `make -k -j4 unit` + `make -k e2e` par mutation) | `p` | 0 | tableau, preuve 3 ; P12, P18, P23 : `archetype_chain` seul | 236 s |
| 5 | texte `b` appliqué (script d'ancres) ; `timeout 900 make -j4 test` (1er) | `b` | 0 | vert du premier coup | non chronométré (< 1 min) |
| 6 | `make e2e`, compteur, `--champs`, `wc -l`, total | `b` | 0 | 26 ; 911 / 74 ; 507 ; 274 / 15 / 20 ; 642 ; 384 | < 30 s |
| 7 | `--champs` à `p` et `b`, `diff` | `p`, `b` | 0 | `def` 12/0 → 19/1 pour les 14 types : initialiseur de `pb` hors constructeur | < 5 s |
| 8 | B1 à B27 | `b` | 0 | tableau, preuve 3 | 250 s |
| 9 | preuve `const` (3 membres, un à la fois) ; preuve négative | `b` | 0 | 1, 1, 4 erreurs aux poses ; `read-only object` ×2 | < 1 min |
| 10 | `make class-coverage-net` | `b` | 0 | `couverture: OK` | non chronométré |
| 11 | `make sanitize-net` | `b` | 0 | `sanitize: 26 propre(s)` | non chronométré |
| 12 | `make line-coverage EVENTS=<26 e2e>` | `b` | 0 | 6324 lignes ; les 15 lignes de la preuve 4 présentes | non chronométré |
| 13 | `assert` de pose `jc->def`/`jc->probe->def` ajouté à `check_fresh_jc` ; `make test`, `make e2e`, compteur ; commit amendé | `b` | 0 | vert ; 26 ; 911 / 74 ; 274 / 15 / 20 | < 1 min |
| 14 | `git fetch` de la branche : 2.13a1/a2 commitées (`3cc4b9a`, `cf1c128`) ; `git diff 7d685d2 cf1c128 -- src tests` ; `git apply --check` du diff sur `cf1c128` | — | 0 | vide ; passe | < 5 s |
| 15 | greps de casts et de classe à `p`, `b` | — | 0 | 532 / 707 aux deux | < 1 s |
| 16 | `rm -rf` de la copie | — | 0 | supprimée | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas le commit 2.13a2 (`cf1c128`, ou un descendant qui ne touche que `docs/`), ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) est rouge, si un `*.sha256`, `tests/e2e/` ou une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si une jumelle de la preuve 3 est verte à l'enfant et rouge au parent (hors `archetype_chain` au parent), notamment si `mz.has_ik`, `jc.has_ik` ou `jc.od` n'a pas d'`assert` à littéral dans un TU durable.
- `FAILED:` si `mz.t` ou `jc.t` quitte le test ponctuel, si une autre comparaison de `mz`/`jc` y reste (ou y est réécrite vers `def`), ou si le total n'est pas 384.
- `FAILED:` si un champ A de `mz`/`jc` reste membre, si un `def` (instance ou sonde) est écrit hors de l'init de son porteur (preuve 2), ou n'est pas `const pico8_X_def_t *`.
- `FAILED:` si un porteur de `pico8_probe_t` laisse son `def` indéterminé (grep de Sortie 4), si un lecteur lit `probe->def` (grep 5), ou si un champ A quitte `pico8_probe_t`.
- `FAILED:` si `test_pico8_jc.c:853`/`:893` change.
- `FAILED:` si un cast ou un test de classe est ajouté, si `inclusion-mutuelle` n'est pas 17 / 16 (43 lignes).
- `FAILED:` si `défauts` n'est pas 507 (ou 517 puis 507 au repli), si le pied n'est pas 911 / 74, si `--champs` n'est pas `274 ; 15 ; 20`.
- `FAILED:` si `class-coverage-net` ou `sanitize-net` est rouge.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si le message du commit omet : les classes, les champs A retirés et les I copiés, les poses du `def` de sonde, la table des jumelles avec leur résultat au parent, la preuve `const`, les comptes, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.13b », ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- `g_`, `fv` et la suite (2.13c à 2.13e2), sauf le contrat plus haut.
- La bascule des lecteurs de la sonde et le retrait de ses champs A (2.13e2) ; les archétypes de test de `test_pico8_jc.c:853`/`:893` (2.13e2) et de `test_pico8_g_.c:751` (2.13c).
- `pico8_ke_start` et la lecture de `t` sur `def` (2.13e2).
- Les liens du noyau (2.14, 2.15), la question Q-2.10.
- `tools/composition_residue.sh` (le relevé par nom de `--champs`), son TU ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Toute la sonde de `mz`/`jc` est copiée depuis `def`, A compris**, et les littéraux d'init aussi (`has_ik = 1`, `od = 0`…) : les A de la sonde restent membres jusqu'en 2.13e2 (étude 2.10, réserve 2), et une valeur comparée au parent qui ne serait pas lue depuis `def` à l'enfant ferait une jumelle verte à l'enfant (ÉCHEC).
- **G2. NULL, pas `&pico8_probe_def_none`, sur `g_`/`fv` et l'arbre `qh`** : ces porteurs ont déjà un archétype (2.12), mais leur init partagée ne connaît pas la classe concrète avant 2.13c/2.13d ; `none` y serait une valeur fausse et silencieuse, NULL plante à la première lecture prématurée. Le contrat de 2.12b permet les deux.
- **G3. `pb` par initialiseur désigné** (`{.def = &pico8_probe_def_none}`), pas par `pb.def = …` : une ligne, et `pb` n'a jamais un `def` NULL. Effet mesuré sur `--champs` (« 0 hors constructeur » 32 → 20), expliqué plus haut ; une affectation ferait le même effet.
- **G4. `test_pico8_jc.c:853`/`:893` restent à 2.13b** (section dédiée) ; la commande les donnait « à réécrire », ce n'est possible qu'avec la bascule de `probe.c:168`.
- **G5. `header.me` copié depuis `def`** et sa comparaison retirée, alors que la famille a gardait `me` : `mz` et `jc` réécrivent `me` sur la chaîne de `ke`, leur `me` ne vient plus de `pico8_ke_start`.
- **G6. Deux `assert` de pose** (`mz` dans `spec_defaults`, `jc` dans `check_fresh_jc`) : `def == &pico8_X_def` et `probe->def == &pico8_X_def.probe`, faute de lecteur du `def` de sonde.

### Surprises de mesure, pour `plan-pilot`

1. **Trois filets uniques** dans la famille b (`mz.has_ik`, `jc.has_ik`, `jc.od`) : ce sont des **littéraux d'init** au parent, pas des champs de `_start`, et aucun TU ne les comparait à un littéral. `jc.od` est une valeur morte (`has_od` nul), gardée quand même par la règle du parent.
2. **`--champs` « 0 hors constructeur » baisse de 12** au lieu de monter de 2 : le relevé par nom ne voit qu'un seul `def`, et l'initialiseur de `pb` le sort du groupe pour les quatorze types. Il n'y rentrera plus (`pb` garde son `def`).
3. **`pico8_probe_def_none` n'a pas de lecteur et n'en aura pas** : `pico8_dl_show` ne lit que `et`, `has_od`, `od`. Il reste la cible de `pb.def` (étude 2.10, réserve 4).
4. 2.13a1/a2 commitées pendant l'écriture ont `src/` et `tests/` identiques à la copie : aucune ancre à reprendre.

### Questions pour l'humain

Aucune.
