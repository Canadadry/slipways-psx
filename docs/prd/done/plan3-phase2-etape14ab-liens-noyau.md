---
title: "Plan 3, étapes 2.14a et 2.14b : les liens de composant `fa`, `ra`, `p` au noyau, une init du noyau unique ; `accesseurs-q6` 237 → 193 → 123, `lien-parent` 111 → 110, `inclusion-mutuelle` 17 / 43 → 16 / 40 — deux commits, jeu identique"
description: "2.14a : `pico8_struct_header_t` reçoit `const pico8_fixed_t *fa` et `pico8_struct_header_init(self, cls)`, que chaque constructeur appelle (le `_new` fait `malloc` sans mise à zéro) ; lien `&def->fa` pour les neuf classes A, `&self->fa` pour `jc`, `js`, `pj` ; `get_fa` disparaît ; TU neuf `test_pico8_struct_header` qui se relance sous `MALLOC_PERTURB_`. 2.14b : liens `ra`, `p` de type `const pico8_vec_t *const *`, huit membres passés en `const`, trois casts de qualificatif (`nf.ra`, `jo.ra`, `jo.p`) ; `get_ra`/`get_p` disparaissent ; lectures croisées `pico8_jc_end_p`/`_ra` et `emitter_host_fa_ra` sur les liens ; `particle.c` n'inclut plus `pico8_probe.h`. Essayé sur copie : vert, 26 e2e (aussi sous perturbation) à chaque commit ; 17 + 17 jumelles et six lignes K, aucun ÉCHEC."
status: needs-triage
---

## Problem Statement

**Écrit sur `7a5c47c`** (branche `claude/plan-pilot-3-7zdft0`), pendant que 2.13e2c s'implémentait dans l'arbre de travail (fichiers non commités non lus). Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier. Essais sur une seule copie jetable (scratchpad `prd214ab/`, supprimée depuis). Sur cette copie, `7a5c47c` a reçu les quatre diffs du PRD 2.13e (`docs/prd/triage/plan3-phase2-etape13e-defauts-famille-e.md`, blocs `diff` ; `git apply --check` puis `git apply`, `git rm tests/test_pico8_archetype_chain.c` avant le quatrième), commités `348d510` (e1), `6d5d5a7` (e2a), `ea878f0` (e2b), `8195d5e` (e2c). La mesure de `8195d5e` est exactement la colonne `e2c` du PRD 2.13e (table ci-dessous). Notation : `p` = `8195d5e`, l'arbre d'après 2.13e ; `a` = `cd485a8`, `b` = `ca7260b`, les commits de la copie qui portent les deux diffs de « Ce qui change ». Les numéros de ligne cités sont ceux de `p`, sauf mention.

Vu du mainteneur du port : le code générique (la destruction en couches `dc`, le survol de `ui`, `pico8_gk`, `pico8_gd`, l'ancrage `ij` de `mz`, les bouts d'un `jc`, l'hôte d'un émetteur) lit `fa`, `ra` et `p` sur une entité dont il ignore le type, par dix accesseurs de table (`get_fa`, `get_ra`, `get_p`, 114 lignes du compteur) ou par un test de famille suivi d'un cast. La phase 2 remplace chaque accesseur par un **lien du noyau** vers le stockage du champ (Q-2.10, « oui »). 2.14a pose le premier lien, `fa`, avec l'init du noyau que tous les liens suivants réutilisent ; 2.14b pose `ra` et `p`, et fait tomber la paire `particle` ↔ `probe`.

### Texte du plan, recopié (`docs/plan-3.md` à `7a5c47c`)

> - 2.14 — `accesseurs-q6`, selon l'étude 2.10 (§1) : chaque emplacement Q6 remplacé par un lien typé du noyau vers le stockage que 2.9 fixe (`&def->X` pour un champ A sur ce type, `&self->X` sinon, NULL pour un non-porteur), posé une fois par l'init. `ql` est A sur ses deux porteurs (lecture d'archétype par le lien), les neuf autres groupes sont des composants. Les lecteurs génériques lisent `e->X` ; les accès des modules porteurs ne changent pas. Comptes calculés sur `--check accesseurs-q6`, une ligne sortant avec le dernier emplacement qu'elle nomme (étude, annexe A). Preuves : chemin touché (`gcov` des lecteurs du groupe ; mutations : lien laissé NULL sur un porteur, lien posé sur un non-porteur là où un lecteur observe l'absence, `dc`, `pico8_gk`, `jc_mt_of`) ; un lien de champ A vise `def` (grep de l'init) ; retrait d'un `has_X` prouvé par le grep de ses lectures et par une mutation. `class-coverage-net` : oui. `sanitize-net` : oui (le noyau grandit). Budget ≈ 5 × 50 min.
>   - 2.14a — `fa` ; une init du noyau unique, appelée par chaque constructeur, pose `cls` et met tous les liens à NULL (les `_new` font `malloc` sans mise à zéro) ; TU qui construit un non-porteur sur un bloc rempli de `0xa5`. Compte : `accesseurs-q6` 237 → **193** (mesuré sur copie).
>   - 2.14b — `ra`, `p` : liens `const pico8_vec_t *const *` ; membres passés en `const`, sauf `jo.ra`, `jo.p`, `nf.ra` (cast de qualificatif seul, trois) ; lectures croisées `slipway.c:55`, `:64`, `particle.c:19-30`. Compte : → **123** ; `lien-parent` 111 → **110** (`particle.c:19`). (2.11 : retire aussi `particle.c:13`, `#include "pico8_probe.h"`, dont plus aucun nom n'est employé ; fait tomber `particle` ↔ `probe` : `inclusion-mutuelle` 17 / 43 → **16 / 40**, graphe d'usage 20 → 19.)

Décision de l'humain (même fichier) : **Q-2.10** — « oui, chaque objet reçoit les liens de composant du noyau (environ 100 octets de plus par objet sur PC, 50 sur la PSX). La taille du code C est reprise plus tard, hors phase 2. » Notes « Découvert à 2.10 » : forme (R2), piège `malloc`, noyau 24 → 112 → 128 octets, essai `fa` 237 → 193 ; `ra`/`p` en `const` sauf `jo.ra`, `jo.p`, `nf.ra`. Notes « Découvert à 2.11 » : `particle` ↔ `probe` tombe à 2.14b avec le retrait de son include ; la paire dépend de Q-2.10.

Règles communes : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte (`*_HASH`, `FROZEN_*`) changée ; `défauts` reste à 0 et gardé.

### Contrats reçus et ce que ce PRD en fait

| Point (source) | Ce que font 2.14a/2.14b |
|---|---|
| forme (R2) : un pointeur typé par groupe dans le noyau, `&def->X` pour un champ A, `&self->X` sinon, NULL pour un non-porteur, posé une fois par l'init (étude 2.10, §1) | **a** : `fa` ; **b** : `ra`, `p`. Poses : grep de Sortie 1 et 5 |
| piège : les `_new` font `malloc(sizeof *self)` sans mise à zéro ; une init du noyau unique pose `cls` et met les liens à NULL ; plus aucune écriture de `cls` hors d'elle et des initialiseurs statiques (étude 2.10, §1) | **a** : `pico8_struct_header_init`, appelée par `pico8_ke_struct_init` et les sept constructeurs qui ne passent pas par `ke` (`hy`, `lq`, `jb`, `hp`, `trade`, le point souris de `jc:hq`, la vue `ie.ke`) ; grep de Sortie 2 |
| réserve du vérificateur de 2.10 : sous perturbation, `pico8_nf_new()` rendait `header.fa = 0xa5a5…` ; le TU doit rougir si l'init oublie un lien | **a** : `tests/test_pico8_struct_header.c`, qui se relance lui-même sous `MALLOC_PERTURB_=90` (glibc : chaque `malloc` rend un bloc de `0xa5`) ; mutations K rouges (preuve 3) |
| `ra`/`p` : `pico8_vec_t **` ne se convertit pas en `const pico8_vec_t *const *` ; membres en `const` (0 erreur) sauf `jo.ra` (9 erreurs), `jo.p` (3), `nf.ra` (3), liés par un cast de qualificatif seul avec son contrat (étude 2.10, §1) | **b** : huit membres en `const` (`lq.ra`, `iv.ra`, `js.ra`, `mz.ra`, `mz.p`, `g_.p`, point souris `ra`/`p`) ; trois casts ; relevé 3 / 3 / 9 rejoué sur `b` (preuve 2) |
| lectures croisées `slipway.c:55`, `:64` (`pico8_jc_end_p`/`_ra`), `particle.c:19-30` (`emitter_host_fa_ra`) (étude 2.10, §1) | **b** : lues par les liens ; `jc_end_point` et le test `pico8_jo_is_family` des bouts partent ; `pico8_g__of` de `particle.c:19` sort de `lien-parent` |
| `particle.c:13` retirée dans le même commit ; aucune autre ligne de `particle` vers `probe` (étude 2.11, fiche 4) | **b** : retirée ; `particle` n'emploie plus aucun nom de `probe.h` (graphe d'usage 20 → 19) |
| `def` des composants posé par l'init du porteur ; un lien de composant n'a rien à changer au `def` (PRD 2.13e, « Contrat pour 2.14 ») | tenu : aucun `def` ne bouge ; les liens `fa` de `g_`/`fv` et de l'arbre `jo` visent `&self->def->fa`, posés juste après `self->def = def` |
| `emitter.pn`/`du` de `ga` non écrits par son init (PRD 2.13e) | aucun lecteur neuf de l'émetteur : sans objet |
| `pico8_ke_struct_init(self, cls, me, t)` (PRD 2.13e) | signature inchangée ; son corps appelle l'init du noyau puis pose `me`/`t` |
| `défauts` gardé (PRD 2.13e) | aucun `_start` nommé ; garde verte aux deux commits |
| `test_pico8_pj.c`, `ed_fixed(ju) == 1` (réserve reportée par 2.13e) | fichier non touché par 2.14a/2.14b : reste reportée |

### Question Q-2.10 : tranchée « oui » (`259fabe`)

2.14a et 2.14b en dépendent entièrement ; la paire `particle` ↔ `probe` aussi. Mesuré : `sizeof(pico8_struct_header_t)` passe de 24 à **32** (a) puis **48** octets (b) sur x86-64.

### État de départ attendu (le commit 2.13e2c, ou un descendant qui ne touche que `docs/`)

Mesuré sur `p` :

| Commande | Résultat |
|---|---|
| `make composition-residue` | `parente 10 3`, `lien-parent 111 27`, `accesseurs-q6 237 33`, `void-ptr 8 4`, `inclusion-mutuelle 17 16` (ligne `défauts` absente) ; **`résidu 2 : 404 ligne(s), 63 fichier(s)`** |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 242 ; 0 écriture : 15 ; 0 hors constructeur : 20` |
| `--check inclusion-mutuelle \| tail -1` | `résidu gardé (inclusion-mutuelle) : 43 ligne(s)` |
| `COMPOSITION_RESIDUE_GUARD` (`Makefile:870`) | `noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts` |
| casts / tests de classe (greps de la réserve 4 de 2.13a) | 530 / 708 |
| `sizeof(pico8_struct_header_t)` | 24 |
| graphe d'usage (`usegraph.py`, annexe A du PRD 2.11) | `paires 20` |
| `make test` ; `make e2e` | vert (≈ 10 s) ; 26 conformes |

Un départ autre : arrêt, rapporté. Contrôle rapide : `git diff 8195d5e <départ> -- src tests Makefile tools` sur une copie qui a reçu les quatre diffs de 2.13e doit être vide ; sinon les diffs ci-dessous peuvent ne plus s'appliquer (`git apply --check`), et le PRD est à rejouer.

## Solution

Deux commits, chacun vert, jeu identique.

**2.14a — `fa` et l'init du noyau** (`accesseurs-q6` 237 → **193**) :

- `pico8_struct_header_t` gagne `const pico8_fixed_t *fa` ; `pico8_struct.h` gagne `static inline void pico8_struct_header_init(pico8_struct_header_t *self, const pico8_struct_class_t *cls)`, de corps `*self = (pico8_struct_header_t){.cls = cls};` : `me` nil, `t` 0, **chaque lien, présent et à venir, NULL** (G1).
- `pico8_ke_struct_init` l'appelle avant de poser `me`/`t` ; les sept autres sites qui posaient `cls` à la main l'appellent (`economy.c:33`, `layout.c:305`, `:412`, `research.c:137`, `research_icon.c:55`, `slipway.c:130`, `trade.c:32`).
- Lien posé une fois, avec `def` : `self->header.fa = &self->def->fa;` pour les neuf classes A (`bg`, `fq`, `ie`, `iv`, `mh`, `mz`, `ui`, `g_` et `fv` dans `pico8_g__struct_init`, l'arbre `jo` dans `pico8_jo_struct_init`) ; `self->header.fa = &self->fa;` pour les trois I (`jc`, `js`, `pj`), après leur copie de `def->fa`.
- Lecteurs : `dc` (`entity.c:203-206`), `pico8_gk` (`entity_geom.c:35-36`), le survol de `ui` (`ui.c:291-292`) lisent `e->fa`.
- Partent : le membre `get_fa` de la table, les douze implémentations et leurs 22 entrées, les deux prototypes (`planet.h`, `probe.h`) ; dans les TU, `giver_get_fa` (le faux porteur de `test_pico8_ui.c` reçoit `.fa = &g_giver_fa` dans son initialiseur) et l'appel de `test_pico8_mz.c:317`, qui devient l'épingle `mz->header.fa == &mz->def->fa`.
- TU neuf `tests/test_pico8_struct_header.c` (règle et ligne `UNIT_TESTS` du `Makefile`) : il se relance avec `MALLOC_PERTURB_=90` (et `MallocScribble=1` pour macOS), vérifie qu'un `malloc` frais est bien rempli, puis construit les non-porteurs de chaque chemin de construction hors `ke` et un par `ke` (`nf`, `lq`, `jb`, `hp`, `hy`, `trade`, la vue `ie.ke`) et trois porteurs (`bg` A, `js` I, `ie` A), et compare chaque lien à l'attendu (G2).

**2.14b — `ra`, `p`** (`accesseurs-q6` 193 → **123**, `lien-parent` 111 → **110**, `inclusion-mutuelle` 17 / 16 → **16 / 15**, 43 → **40** lignes) :

- `pico8_struct_header_t` gagne `const pico8_vec_t *const *ra` et `const pico8_vec_t *const *p` ; les membres `get_ra`, `get_p` de la table partent, avec quinze implémentations, leurs entrées et quatre prototypes.
- Membres passés en `const pico8_vec_t *` : `lq.ra`, `iv.ra`, `js.ra`, `mz.ra`, `mz.p`, `g_.p`, `jc_mouse_point.ra`/`.p` (`g_.ra`, `ie.ra`, `qb.ra`/`.p` l'étaient). Liens `&self->ra`/`&self->p` ; `nf.ra`, `jo.ra`, `jo.p`, écrits à travers, restent mutables, liés par `(const pico8_vec_t *const *)&self->X` sous le commentaire de contrat « Read-only: … writes through its member » (trois casts, G3).
- Poses : `lq` (après l'init du noyau), `nf`, `iv`, `ie`, `qb`, `js`, `mz`, `g_`/`fv` (`pico8_g__struct_init`), l'arbre `jo` (`pico8_jo_struct_init`), le point souris (`jc:hq`, après l'init du noyau).
- Lecteurs : `dc` (`entity.c:228`), `pico8_entity_geom_ra`/`_p` (`entity_geom.c:17`, `:23`), `pico8_mzd_ra_of_ij` (`wormhole_display.c:14-15`) ; lectures croisées : `pico8_jc_end_p`/`_ra` lisent `end->p`/`end->ra` (`jc_end_point` et son `assert` partent ; commentaire de `slipway.h:80-82` rendu vrai) ; `emitter_host_fa_ra` lit `self->fa`/`self->ra` sous un `assert` de présence des deux liens (plus de `pico8_g__of`, de `pico8_jo_is_family` ni de cast), et `particle.c` n'inclut plus `pico8_probe.h`.
- TU : les sept comparaisons « accesseur == membre » deviennent des épingles de lien (`header.ra == &x->ra`, ou `*header.ra == x->ra` pour `nf` et le point souris) ; le faux type `ra_only` de `test_pico8_mo.c` pose son lien dans son initialiseur ; `test_pico8_struct_header.c` compare aussi `ra` et `p`.

Rien d'autre : ni `glossaire-noms.md` (aucun nom minifié neuf), ni `trous-de-portage.md`, ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni `test_pico8_pj.c`. Le `Makefile` en 2.14a seulement (le TU neuf).

## User Stories

1. En tant que mainteneur, je veux que le code générique lise `fa`, `ra` et `p` par un lien du noyau, afin que dix fonctions de table et leurs 114 lignes disparaissent sans changer une valeur du jeu.
2. En tant que gardien de la mémoire, je veux que chaque constructeur passe par une seule init du noyau, afin qu'aucun lien ne soit jamais un pointeur indéterminé, même quand `malloc` rend un bloc sale — et qu'un TU le prouve sous perturbation.
3. En tant que gardien de la règle 1, je veux qu'un lien de champ A vise l'archétype, un champ I l'instance, et que rien ne puisse écrire à travers un lien (`const` jusqu'au bout).
4. En tant que gardien de la règle du parent, je veux qu'aucune mutation rouge au parent ne devienne verte à l'enfant.
5. En tant qu'auteur de 2.17, je veux que `particle` ↔ `probe` tombe ici, avec son include.
6. En tant qu'auteur de 2.14c à 2.14d2, je veux une init qui met déjà à NULL les liens que j'ajouterai, et un TU où ajouter leur attendu.

## Implementation Decisions

### Modules

- **a** : `Makefile` ; `struct.h`, `ke.c` ; les porteurs `bg`, `msgbox` (`mh`, `iv`), `planet`, `probe` (`g_`), `research_icon`, `score_panel`, `slipway`, `ui`, `ui_button` (`js`, `fq`), `wormhole` ; les tables partagées `help` (`fv`), `planet_family`, `planet_family_trade` ; les constructeurs hors `ke` `economy`, `layout`, `research`, `research_icon` (vue), `slipway` (point souris), `trade` ; les lecteurs `entity`, `entity_geom`, `ui` ; en-têtes `planet.h`, `probe.h` ; TU `mz`, `ui`, `struct_header` (neuf).
- **b** : `struct.h` ; en-têtes `layout.h`, `msgbox.h`, `probe.h`, `planet.h`, `slipway.h`, `ui_button.h`, `wormhole.h` ; porteurs `layout`, `mouse_reader`, `msgbox`, `planet`, `probe`, `research_icon`, `scroll`, `slipway`, `ui_button`, `wormhole`, tables `help`, `planet_family`, `planet_family_trade` ; lecteurs `entity`, `entity_geom`, `wormhole_display`, `slipway` (bouts), `particle` (hôte) ; TU `g_`, `jc`, `lq`, `mo`, `mz`, `nf`, `qb`, `struct_header`.

### Inclusions

2.14a : aucune inclusion neuve (`pico8_struct_header_init` est dans `pico8_struct.h`, que les sept constructeurs hors `ke` voient déjà par l'en-tête de leur type ; G1) ; `inclusion-mutuelle` **17 / 16, 43 lignes**, graphe d'usage 20. 2.14b : une ligne retirée, `particle.c:13` ; `inclusion-mutuelle` **16 / 15, 40 lignes** (la paire `particle` ↔ `probe` sort avec ses trois lignes : `particle.c:13`, `probe.c:17` et `probe.h:9`, ces deux dernières toujours présentes mais sans paire), graphe d'usage **19**. `particle.c:12` (`pico8_planet.h`) n'a plus aucun nom employé après 2.14b ; `particle` ↔ `planet` n'est pas une paire, la ligne reste (G6).

### Ce qui change

Les deux diffs ci-dessous sont le texte exact (`git diff p a`, `git diff a b`), `git apply --check` puis `git apply` sur `p` rendant `b` à l'identique (Runs 19). Lecture, en plus de la Solution :

- **Une init, pas une liste** : le littéral composé met à zéro tout le noyau ; 2.14c à 2.14d2 n'ont qu'à déclarer leur membre. `self->cls = cls;` seul (le code d'avant) est la mutation K-init, que seul le TU neuf voit (preuve 3).
- **Le lien d'un champ A vise `def`**, qui est `const` : `&self->def->fa` a le type `const pico8_fixed_t *` sans cast ; un champ I vise l'instance, après la copie de `def->fa` (`jc`, `pj`) ou la clé (`js`, `keys->fa` écrase la copie : le lien voit la clé).
- **Le point souris** (`jc_mouse_point`) garde sa table vide (`{0}`) : son identité de classe reste lue par `test_pico8_jc.c:119` ; ses liens `ra`/`p` sont posés après l'init du noyau.
- **Bouts d'un `jc`** : `pico8_jc_end_p(end)` rend `*end->p` quand le lien est posé, le vecteur nul sinon. L'`assert` « un bout est une planète ou le point souris » part avec `jc_end_point` (étude 2.10 : « plus de test ni de `jc_end_point` ») ; aucun TU ne l'observait (preuve 3 : aucune jumelle ne dépend de lui) ; le commentaire de `slipway.h` ne dit plus « Asserts on any other end ».
- **Hôte d'un émetteur** : `emitter_host_fa_ra` perd l'aiguillage `g_`/`jo` ; son `assert` porte sur la présence des liens `fa` et `ra` (les six classes à émetteur, `g_`, `fv`, `hj`, `pm`, `mx`, `ga`, les portent).
- **Commentaires** : celui du noyau dit le contrat d'un lien (stockage de l'archétype pour un champ jamais écrit par la classe, NULL pour un non-porteur) ; celui de l'init dit le piège ; les trois casts disent qui écrit.

#### Diff de 2.14a (`git diff p a`)

```diff
diff --git a/Makefile b/Makefile
index 9ff7838..b20dcc7 100644
--- a/Makefile
+++ b/Makefile
@@ -475,6 +475,9 @@ $(TMP_DIR)/test_pico8_entity_kind: $(call objs,tests/test_pico8_entity_kind.c $(
 $(TMP_DIR)/test_pico8_spec_defaults: $(call objs,tests/test_pico8_spec_defaults.c $(ENGINE_SRCS)) | $(TMP_DIR)
 	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm
 
+$(TMP_DIR)/test_pico8_struct_header: $(call objs,tests/test_pico8_struct_header.c $(ENGINE_SRCS)) | $(TMP_DIR)
+	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm
+
 $(TMP_DIR)/test_pico8_jo_spec_defaults: $(call objs,tests/test_pico8_jo_spec_defaults.c $(ENGINE_SRCS)) | $(TMP_DIR)
 	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm
 
@@ -628,6 +631,7 @@ UNIT_TESTS := \
     test_pico8_nl \
     test_pico8_entity_kind \
     test_pico8_spec_defaults \
+    test_pico8_struct_header \
     test_pico8_jo_spec_defaults \
     test_pico8_os \
     test_pico8_hj \
diff --git a/src/pico8_bg.c b/src/pico8_bg.c
index 9be6afc..3d4aaf7 100644
--- a/src/pico8_bg.c
+++ b/src/pico8_bg.c
@@ -41,11 +41,6 @@ static void pico8_bg_hq_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
     pico8_bg_hq((pico8_bg_t *)self, ra);
 }
 
-static int bg_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_bg_t *)self)->def->fa;
-    return 1;
-}
-
 static void bg_set_m_(void *self, int m_) {
     pico8_bg_t *bg = self;
 
@@ -59,7 +54,6 @@ static const pico8_struct_class_t g_bg_struct_class = {
     .ldown = pico8_bg_ldown_dispatch,
     .rdown = pico8_bg_rdown_dispatch,
     .hq = pico8_bg_hq_dispatch,
-    .get_fa = bg_get_fa,
     .set_m_ = bg_set_m_,
 };
 
@@ -68,6 +62,7 @@ static pico8_bg_t *pico8_bg_struct_new(void) {
 
     pico8_ke_struct_init(&self->header, &g_bg_struct_class, pico8_bg_def.me, pico8_bg_def.t);
     self->def = &pico8_bg_def;
+    self->header.fa = &self->def->fa;
     self->mq = NULL;
     self->m_ = 0;
     self->has_m_ = 0;
diff --git a/src/pico8_economy.c b/src/pico8_economy.c
index 8c7b5f4..a2dafca 100644
--- a/src/pico8_economy.c
+++ b/src/pico8_economy.c
@@ -30,7 +30,7 @@ static pico8_hy_t *pico8_hy_struct_new(void) {
     pico8_hy_t *self = malloc(sizeof *self);
     pico8_ptrs_t *invented;
 
-    self->header.cls = &g_hy_struct_class;
+    pico8_struct_header_init(&self->header, &g_hy_struct_class);
     self->def = &pico8_hy_def;
     self->nz = pico8_fixed_from_int(0);
     self->ge = pico8_fixed_from_int(0);
diff --git a/src/pico8_entity.c b/src/pico8_entity.c
index 06ba009..b52ec0f 100644
--- a/src/pico8_entity.c
+++ b/src/pico8_entity.c
@@ -195,15 +195,14 @@ void pico8_dc(const char *nt) {
     if (bucket_src != NULL) {
         for (i = 0; i < bucket_src->count; i++) {
             pico8_struct_header_t *qc = bucket_src->items[i];
-            pico8_fixed_t fa;
             int layer;
 
             /* An entity whose `.fa` is absent or outside 1..15 is dropped
              * from every layer, where Lua would error on the nil key. */
-            if (qc->cls->get_fa == NULL || !qc->cls->get_fa(qc, &fa)) {
+            if (qc->fa == NULL) {
                 continue;
             }
-            layer = pico8_fixed_flr_int(fa);
+            layer = pico8_fixed_flr_int(*qc->fa);
             if (layer < 1 || layer > 15) {
                 continue;
             }
diff --git a/src/pico8_entity_geom.c b/src/pico8_entity_geom.c
index 08785be..26fa7ef 100644
--- a/src/pico8_entity_geom.c
+++ b/src/pico8_entity_geom.c
@@ -26,15 +26,12 @@ static pico8_vec_t pico8_entity_geom_p(const pico8_struct_header_t *e) {
 }
 
 int pico8_gk(const pico8_struct_header_t *e, const pico8_vec_t *mp, const pico8_vec_t *rp) {
-    pico8_fixed_t fa = pico8_fixed_from_int(0);
+    pico8_fixed_t fa = e != NULL && e->fa != NULL ? *e->fa : pico8_fixed_from_int(0);
     pico8_vec_t ra = pico8_entity_geom_ra(e);
     const pico8_vec_t *chosen;
     pico8_vec_t point;
     const pico8_qz_box_t *kq;
 
-    if (e != NULL && e->cls->get_fa != NULL) {
-        e->cls->get_fa(e, &fa);
-    }
     chosen = (fa >= pico8_fixed_from_int(10)) ? rp : mp;
     point = pico8_vec_sub(chosen != NULL ? *chosen : pico8_entity_geom_zero(), ra);
     kq = e != NULL && e->cls->get_kq != NULL ? e->cls->get_kq(e) : NULL;
diff --git a/src/pico8_help.c b/src/pico8_help.c
index 8a89308..0603c10 100644
--- a/src/pico8_help.c
+++ b/src/pico8_help.c
@@ -132,7 +132,6 @@ static const pico8_struct_class_t g_fv_struct_class = {
     .emitter_of = pico8_g__emitter_of,
     .probe_of = pico8_g__probe_of,
     .tags = PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fa = pico8_g__get_fa,
     .get_ra = pico8_g__get_ra,
     .get_p = pico8_g__get_p,
     .get_kq = fv_get_kq,
diff --git a/src/pico8_ke.c b/src/pico8_ke.c
index 74fdcb8..f1a57d7 100644
--- a/src/pico8_ke.c
+++ b/src/pico8_ke.c
@@ -33,7 +33,7 @@ void pico8_ke_ks(pico8_struct_header_t *self, const char *me) {
 
 void pico8_ke_struct_init(pico8_struct_header_t *self, const pico8_struct_class_t *cls, const char *me,
                           pico8_fixed_t t) {
-    self->cls = cls;
+    pico8_struct_header_init(self, cls);
     self->me = me;
     self->t = t;
 }
diff --git a/src/pico8_layout.c b/src/pico8_layout.c
index e9a38b9..7dff2f2 100644
--- a/src/pico8_layout.c
+++ b/src/pico8_layout.c
@@ -302,7 +302,7 @@ static const pico8_struct_class_t g_lq_struct_class = {
 static pico8_lq_t *pico8_lq_struct_alloc(int n) {
     pico8_lq_t *self = malloc(sizeof *self);
 
-    self->header.cls = &g_lq_struct_class;
+    pico8_struct_header_init(&self->header, &g_lq_struct_class);
     self->c = 0;
     self->has_c = 0;
     self->bg = 0;
@@ -409,7 +409,7 @@ static pico8_jb_t *pico8_jb_struct_alloc(void) {
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("jb");
 #endif
-    self->header.cls = &g_jb_struct_class;
+    pico8_struct_header_init(&self->header, &g_jb_struct_class);
     self->ls = NULL;
     self->ls_count = 0;
     return self;
diff --git a/src/pico8_msgbox.c b/src/pico8_msgbox.c
index 7f9f9e7..18f87e6 100644
--- a/src/pico8_msgbox.c
+++ b/src/pico8_msgbox.c
@@ -85,11 +85,6 @@ void pico8_mh_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_mh_t *self) {
     }
 }
 
-static int mh_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_mh_t *)self)->def->fa;
-    return 1;
-}
-
 static int mh_get_p_(const void *self) {
     const pico8_mh_t *mh = self;
 
@@ -99,7 +94,6 @@ static int mh_get_p_(const void *self) {
 static const pico8_struct_class_t g_mh_struct_class = {
     .ev = pico8_mh_ev_dispatch,
     .tags = PICO8_TAG(PICO8_TAG_MH),
-    .get_fa = mh_get_fa,
     .get_p_ = mh_get_p_,
 };
 
@@ -109,6 +103,7 @@ static pico8_mh_t *pico8_mh_struct_new(const pico8_mh_keys_t *keys) {
 
     pico8_ke_struct_init(&self->header, &g_mh_struct_class, pico8_mh_def.me, pico8_mh_def.t);
     self->def = &pico8_mh_def;
+    self->header.fa = &self->def->fa;
     self->rh = self->def->rh;
     self->lh = NULL;
     self->p_ = 0;
@@ -228,11 +223,6 @@ static void pico8_iv_kj_dispatch(void *self, const pico8_vec_t *ra) {
     pico8_iv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_iv_t *)self, p);
 }
 
-static int iv_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_iv_t *)self)->def->fa;
-    return 1;
-}
-
 static const pico8_vec_t *iv_get_ra(const void *self) {
     return ((const pico8_iv_t *)self)->ra;
 }
@@ -246,7 +236,6 @@ static int iv_get_p_(const void *self) {
 static const pico8_struct_class_t g_iv_struct_class = {
     .kj = pico8_iv_kj_dispatch,
     .states = {[PICO8_STATE_PH] = pico8_iv_ph_dispatch},
-    .get_fa = iv_get_fa,
     .get_ra = iv_get_ra,
     .get_p_ = iv_get_p_,
 };
@@ -256,6 +245,7 @@ static pico8_iv_t *pico8_iv_struct_new(const pico8_iv_keys_t *keys) {
 
     pico8_ke_struct_init(&self->header, &g_iv_struct_class, pico8_iv_def.me, pico8_iv_def.t);
     self->def = &pico8_iv_def;
+    self->header.fa = &self->def->fa;
     self->l = self->def->l;
     self->ra = NULL;
     self->p_ = 0;
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index 713fab1..0b073b5 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -76,11 +76,6 @@ void pico8_jo_ll_dispatch(void *self, pico8_fixed_t t) {
     pico8_jo_ll((pico8_jo_t *)self, t);
 }
 
-int pico8_jo_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_jo_t *)self)->def->fa;
-    return 1;
-}
-
 const pico8_vec_t *pico8_jo_get_ra(const void *self) {
     return ((const pico8_jo_t *)self)->ra;
 }
@@ -123,7 +118,6 @@ static const pico8_struct_class_t g_jo_struct_class = {
     .cn = pico8_jo_cn_dispatch,
     .states = {[PICO8_STATE_LL] = pico8_jo_ll_dispatch},
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fa = pico8_jo_get_fa,
     .get_ra = pico8_jo_get_ra,
     .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
@@ -136,6 +130,7 @@ void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, con
     assert(g_jo_up && "pico8_jo_init() must run first");
     pico8_ke_struct_init(&self->header, cls, def->me, def->t);
     self->def = def;
+    self->header.fa = &self->def->fa;
     self->ma = self->def->ma;
     self->kw = &g_jo_kw_shared;
     self->id = NULL;
diff --git a/src/pico8_planet.h b/src/pico8_planet.h
index d3193cd..0a3ed19 100644
--- a/src/pico8_planet.h
+++ b/src/pico8_planet.h
@@ -103,7 +103,6 @@ void pico8_jo_kf_dispatch(void *self, pico8_struct_header_t *sw);
 
 /* The field accessors jo's descriptor holds, for a daughter's `static const`
  * descriptor to name. */
-int pico8_jo_get_fa(const void *self, pico8_fixed_t *out);
 const pico8_vec_t *pico8_jo_get_ra(const void *self);
 const pico8_vec_t *pico8_jo_get_p(const void *self);
 const pico8_qz_box_t *pico8_jo_get_kq(const void *self);
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index 65a0ce4..bb6eef3 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -50,7 +50,6 @@ static const pico8_struct_class_t g_qh_struct_class = {
     .parent = pico8_jo_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fa = pico8_jo_get_fa,
     .get_ra = pico8_jo_get_ra,
     .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
@@ -127,7 +126,6 @@ static const pico8_struct_class_t g_nl_struct_class = {
     .parent = pico8_qh_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fa = pico8_jo_get_fa,
     .get_ra = pico8_jo_get_ra,
     .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
@@ -178,7 +176,6 @@ static const pico8_struct_class_t g_os_struct_class = {
     .parent = pico8_qh_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fa = pico8_jo_get_fa,
     .get_ra = pico8_jo_get_ra,
     .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
@@ -249,7 +246,6 @@ static const pico8_struct_class_t g_hj_struct_class = {
     .emitter_of = pico8_hj_emitter_of,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fa = pico8_jo_get_fa,
     .get_ra = pico8_jo_get_ra,
     .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
@@ -343,7 +339,6 @@ static const pico8_struct_class_t g_pm_struct_class = {
     .emitter_of = pico8_hj_emitter_of,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fa = pico8_jo_get_fa,
     .get_ra = pico8_jo_get_ra,
     .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
@@ -500,7 +495,6 @@ static const pico8_struct_class_t g_ga_struct_class = {
     .emitter_of = ga_emitter_of,
     /* ga's cart list of tags is empty and replaces jo's: never jo's. */
     .tags = 0,
-    .get_fa = pico8_jo_get_fa,
     .get_ra = pico8_jo_get_ra,
     .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
diff --git a/src/pico8_planet_family_trade.c b/src/pico8_planet_family_trade.c
index 757a908..42c10d6 100644
--- a/src/pico8_planet_family_trade.c
+++ b/src/pico8_planet_family_trade.c
@@ -178,7 +178,6 @@ static const pico8_struct_class_t g_nr_struct_class = {
     .parent = pico8_qh_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fa = pico8_jo_get_fa,
     .get_ra = pico8_jo_get_ra,
     .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
@@ -244,7 +243,6 @@ static const pico8_struct_class_t g_fr_struct_class = {
     .parent = pico8_nr_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fa = pico8_jo_get_fa,
     .get_ra = pico8_jo_get_ra,
     .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
@@ -346,7 +344,6 @@ static const pico8_struct_class_t g_mx_struct_class = {
     .emitter_of = pico8_hj_emitter_of,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fa = pico8_jo_get_fa,
     .get_ra = pico8_jo_get_ra,
     .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
diff --git a/src/pico8_probe.c b/src/pico8_probe.c
index 43c2dc0..28f747a 100644
--- a/src/pico8_probe.c
+++ b/src/pico8_probe.c
@@ -395,11 +395,6 @@ pico8_qy_t *pico8_g__mm(void *self_ptr) {
     return particle;
 }
 
-int pico8_g__get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_g__t *)self)->def->fa;
-    return 1;
-}
-
 const pico8_vec_t *pico8_g__get_ra(const void *self) {
     return ((const pico8_g__t *)self)->ra;
 }
@@ -429,7 +424,6 @@ static const pico8_struct_class_t g_g__struct_class = {
     .emitter_of = pico8_g__emitter_of,
     .probe_of = pico8_g__probe_of,
     .tags = PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_fa = pico8_g__get_fa,
     .get_ra = pico8_g__get_ra,
     .get_p = pico8_g__get_p,
     .get_fx = pico8_g__get_fx,
@@ -439,6 +433,7 @@ static const pico8_struct_class_t g_g__struct_class = {
 void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls, const pico8_g__def_t *def) {
     pico8_ke_struct_init(&self->header, cls, def->me, def->t);
     self->def = def;
+    self->header.fa = &self->def->fa;
     self->emitter = &self->emitter_own;
     self->emitter->def = &self->def->emitter;
     self->emitter->qy = NULL;
diff --git a/src/pico8_probe.h b/src/pico8_probe.h
index bb3e989..96dce07 100644
--- a/src/pico8_probe.h
+++ b/src/pico8_probe.h
@@ -54,7 +54,6 @@ pico8_g__t *pico8_g__of(pico8_struct_header_t *e);
  * `fv`'s to name. */
 pico8_emitter_t *pico8_g__emitter_of(void *self);
 pico8_probe_t *pico8_g__probe_of(void *self);
-int pico8_g__get_fa(const void *self, pico8_fixed_t *out);
 const pico8_vec_t *pico8_g__get_ra(const void *self);
 const pico8_vec_t *pico8_g__get_p(const void *self);
 int pico8_g__get_fx(const void *self);
diff --git a/src/pico8_research.c b/src/pico8_research.c
index f2b9e5a..6d2659b 100644
--- a/src/pico8_research.c
+++ b/src/pico8_research.c
@@ -134,7 +134,7 @@ static pico8_hp_t *pico8_hp_struct_new(const pico8_hp_keys_t *keys) {
     const char *i_src = "";
     size_t c;
 
-    self->header.cls = &g_hp_struct_class;
+    pico8_struct_header_init(&self->header, &g_hp_struct_class);
     self->px = NULL;
     self->lr = NULL;
     self->fj = 0;
diff --git a/src/pico8_research_icon.c b/src/pico8_research_icon.c
index 4eb4383..6912174 100644
--- a/src/pico8_research_icon.c
+++ b/src/pico8_research_icon.c
@@ -52,7 +52,7 @@ pico8_ie_ke_t pico8_ie_ke_view(const pico8_ie_t *self) {
     const pico8_entity_kind_t *kind = ie_self_ke_kind(self);
     pico8_ie_ke_t view;
 
-    view.header.cls = &g_ie_ke_view_struct_class;
+    pico8_struct_header_init(&view.header, &g_ie_ke_view_struct_class);
     view.rr = NULL;
     view.s = 0;
     view.has_s = 0;
@@ -202,11 +202,6 @@ static int pico8_ie_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_v
     return pico8_gk((const pico8_struct_header_t *)self, qa, ra);
 }
 
-static int ie_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_ie_t *)self)->def->fa;
-    return 1;
-}
-
 static const pico8_vec_t *ie_get_ra(const void *self) {
     return ((const pico8_ie_t *)self)->ra;
 }
@@ -227,7 +222,6 @@ static const pico8_struct_class_t g_ie_struct_class = {
     .hd = pico8_ie_hd_dispatch,
     .hb = pico8_ie_hb_dispatch,
     .ldown = pico8_ie_ldown_dispatch,
-    .get_fa = ie_get_fa,
     .get_ra = ie_get_ra,
     .get_kq = ie_get_kq,
     .set_m_ = ie_set_m_,
@@ -239,6 +233,7 @@ static pico8_ie_t *pico8_ie_struct_new(const pico8_ie_keys_t *keys) {
     assert(g_ie_up && "pico8_research_icon_init() must run before an ie is built");
     pico8_ke_struct_init(&self->header, &g_ie_struct_class, pico8_ie_def.me, pico8_ie_def.t);
     self->def = &pico8_ie_def;
+    self->header.fa = &self->def->fa;
     self->ra = &self->def->ra;
     self->ke = NULL;
     self->jb = NULL;
diff --git a/src/pico8_score_panel.c b/src/pico8_score_panel.c
index 8f0b86d..19fec17 100644
--- a/src/pico8_score_panel.c
+++ b/src/pico8_score_panel.c
@@ -272,17 +272,11 @@ static void pico8_pj_ph_dispatch(void *self, pico8_fixed_t t) {
     pico8_pj_ph((pico8_pj_t *)self);
 }
 
-static int pj_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_pj_t *)self)->fa;
-    return 1;
-}
-
 static const pico8_struct_class_t g_pj_struct_class = {
     .kj = pico8_pj_kj_dispatch,
     .hk = pico8_pj_hk_dispatch,
     .hb = pico8_pj_hb_dispatch,
     .states = {[PICO8_STATE_PH] = pico8_pj_ph_dispatch},
-    .get_fa = pj_get_fa,
 };
 
 static pico8_pj_t *pico8_pj_struct_new(void) {
@@ -293,6 +287,7 @@ static pico8_pj_t *pico8_pj_struct_new(void) {
     pico8_ke_struct_init(&self->header, &g_pj_struct_class, pico8_pj_def.me, pico8_pj_def.t);
     self->def = &pico8_pj_def;
     self->fa = self->def->fa;
+    self->header.fa = &self->fa;
     self->ed = NULL;
     self->c_ = 0;
     self->has_c_ = 0;
diff --git a/src/pico8_slipway.c b/src/pico8_slipway.c
index 7263225..eaac938 100644
--- a/src/pico8_slipway.c
+++ b/src/pico8_slipway.c
@@ -127,7 +127,7 @@ void pico8_jc_hq(pico8_jc_t *self, pico8_vec_t *mp) {
     } else {
         pico8_jc_mouse_point_t *point = malloc(sizeof *point);
         pico8_vec_t *p_heap = malloc(sizeof *p_heap);
-        point->header.cls = &g_jc_mouse_point_struct_class;
+        pico8_struct_header_init(&point->header, &g_jc_mouse_point_struct_class);
         point->ra = mp;
         *p_heap = pico8_vec_div(mp != NULL ? *mp : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0)),
                                 pico8_fixed_from_int(128));
@@ -359,11 +359,6 @@ static pico8_probe_t *jc_probe_of(void *self) {
     return ((pico8_jc_t *)self)->probe;
 }
 
-static int jc_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_jc_t *)self)->fa;
-    return 1;
-}
-
 static int jc_get_mt(const void *self, pico8_fixed_t *out) {
     const pico8_jc_t *jc = self;
 
@@ -387,7 +382,6 @@ static const pico8_struct_class_t g_jc_struct_class = {
     .ey = pico8_jc_ey_dispatch,
     .probe_of = jc_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JC),
-    .get_fa = jc_get_fa,
     .get_mt = jc_get_mt,
     .get_p_ = jc_get_p_,
 };
@@ -398,6 +392,7 @@ static pico8_jc_t *pico8_jc_struct_new(const pico8_jc_keys_t *keys) {
     pico8_ke_struct_init(&self->header, &g_jc_struct_class, pico8_jc_def.me, pico8_jc_def.t);
     self->def = &pico8_jc_def;
     self->fa = self->def->fa;
+    self->header.fa = &self->fa;
     self->probe = &self->probe_own;
     self->probe->def = &self->def->probe;
     self->nh = self->def->nh;
diff --git a/src/pico8_struct.h b/src/pico8_struct.h
index 2ad2466..016e5cb 100644
--- a/src/pico8_struct.h
+++ b/src/pico8_struct.h
@@ -76,15 +76,23 @@ static inline int pico8_tag_named(const char *name) {
 typedef struct pico8_struct_class pico8_struct_class_t;
 
 /* First member of every entity struct, and of the parent struct a child
- * embeds first. `me` and `t` are ke's (`me` NULL is nil):
- * pico8_ke_struct_init sets them; on an entity that never went through it
- * they are left unset and never read. */
+ * embeds first. `me` and `t` are ke's (`me` NULL is nil). A component link
+ * points at the field's storage, the archetype's for a field the class
+ * never writes; NULL: the class has no such field. */
 typedef struct {
     const pico8_struct_class_t *cls;
     const char *me;
     pico8_fixed_t t;
+    const pico8_fixed_t *fa;
 } pico8_struct_header_t;
 
+/* Every constructor calls it first: its `malloc` leaves the header
+ * indeterminate, and a reader would follow a link left unset. `me` is nil,
+ * `t` 0 and every link NULL until the constructor sets its own. */
+static inline void pico8_struct_header_init(pico8_struct_header_t *self, const pico8_struct_class_t *cls) {
+    *self = (pico8_struct_header_t){.cls = cls};
+}
+
 /* What `ldown`, `rdown` and `gv` return: `stop` is the Lua truth of the
  * cart's return value -- 0 for nil as well as for false: a `stop` that only
  * caught false swallowed every click near an unrevealed planet --, `held`
@@ -139,7 +147,6 @@ struct pico8_struct_class {
      * reader returns NULL, or 0 leaving *out untouched, when the field is
      * nil; `get_fx` and `get_p_` return its Lua truth. `set_m_` also marks
      * `m_` present. Each is only ever called as `e->cls->get_X(e, ...)`. */
-    int (*get_fa)(const void *self, pico8_fixed_t *out);
     const pico8_vec_t *(*get_ra)(const void *self);
     const pico8_vec_t *(*get_p)(const void *self);
     const pico8_qz_box_t *(*get_kq)(const void *self);
diff --git a/src/pico8_trade.c b/src/pico8_trade.c
index d8360e7..b43ad2c 100644
--- a/src/pico8_trade.c
+++ b/src/pico8_trade.c
@@ -29,7 +29,7 @@ static pico8_trade_t *pico8_trade_struct_new(const pico8_trade_keys_t *keys) {
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("trade");
 #endif
-    self->header.cls = &g_trade_struct_class;
+    pico8_struct_header_init(&self->header, &g_trade_struct_class);
     self->lp = NULL;
     self->rl = NULL;
     self->qe = 0;
diff --git a/src/pico8_ui.c b/src/pico8_ui.c
index 47f4175..c81694d 100644
--- a/src/pico8_ui.c
+++ b/src/pico8_ui.c
@@ -74,11 +74,6 @@ static void pico8_ui_ev_dispatch(void *self, const pico8_vec_t *ra) {
     pico8_ui_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_ui_t *)self);
 }
 
-static int ui_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_ui_t *)self)->def->fa;
-    return 1;
-}
-
 static int ui_get_ql(const void *self, pico8_fixed_t *out) {
     *out = ((const pico8_ui_t *)self)->def->ql;
     return 1;
@@ -86,7 +81,6 @@ static int ui_get_ql(const void *self, pico8_fixed_t *out) {
 
 static const pico8_struct_class_t g_ui_struct_class = {
     .ev = pico8_ui_ev_dispatch,
-    .get_fa = ui_get_fa,
     .get_ql = ui_get_ql,
 };
 
@@ -98,6 +92,7 @@ static pico8_ui_t *pico8_ui_struct_new(void) {
     self = malloc(sizeof *self);
     pico8_ke_struct_init(&self->header, &g_ui_struct_class, pico8_ui_def.me, pico8_ui_def.t);
     self->def = &pico8_ui_def;
+    self->header.fa = &self->def->fa;
     self->le = &g_shared_le;
     self->qa = NULL;
     self->ic = NULL;
@@ -287,9 +282,8 @@ void pico8_ui_gw(pico8_mem_t *mem, pico8_ui_t *self) {
             pico8_struct_header_t *e = hd_bucket->items[i];
 
             if (e->cls->hd != NULL && e->cls->hd(e, self->qa, je_ra)) {
-                pico8_fixed_t fa_val;
-                if (e->cls->get_fa != NULL && e->cls->get_fa(e, &fa_val)) {
-                    int fa = pico8_ui_to_int(fa_val);
+                if (e->fa != NULL) {
+                    int fa = pico8_ui_to_int(*e->fa);
 
                     assert(fa >= 0 && fa < PICO8_UI_IC_LAYERS &&
                            "ui.ic: a hovered entity's fa outside 0..15, the layers jz walks -- stop, do not widen");
diff --git a/src/pico8_ui_button.c b/src/pico8_ui_button.c
index 96b53af..d66794d 100644
--- a/src/pico8_ui_button.c
+++ b/src/pico8_ui_button.c
@@ -58,11 +58,6 @@ static void pico8_js_hb_dispatch(void *self) {
     }
 }
 
-static int js_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_js_t *)self)->fa;
-    return 1;
-}
-
 static const pico8_vec_t *js_get_ra(const void *self) {
     return ((const pico8_js_t *)self)->ra;
 }
@@ -96,7 +91,6 @@ static const pico8_struct_class_t g_js_struct_class = {
     .has_hb = pico8_js_has_hb,
     .ldown = pico8_js_ldown_dispatch,
     .states = {[PICO8_STATE_PH] = pico8_js_ph_dispatch},
-    .get_fa = js_get_fa,
     .get_ra = js_get_ra,
     .get_kq = js_get_kq,
     .get_fg = js_get_fg,
@@ -134,6 +128,7 @@ static pico8_js_t *pico8_js_struct_new(const pico8_js_keys_t *keys) {
     self->has__jo_fe_od = 0;
     self->def = &pico8_js_def;
     self->fa = self->def->fa;
+    self->header.fa = &self->fa;
     if (keys != NULL) {
         if (keys->has_fa) {
             self->fa = keys->fa;
@@ -324,11 +319,6 @@ static pico8_down_t pico8_fq_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_v
     return pico8_fq_ldown((pico8_fq_t *)self);
 }
 
-static int fq_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_fq_t *)self)->def->fa;
-    return 1;
-}
-
 static int fq_get_p_(const void *self) {
     const pico8_fq_t *fq = self;
 
@@ -345,7 +335,6 @@ static void fq_set_m_(void *self, int m_) {
 static const pico8_struct_class_t g_fq_struct_class = {
     .hd = pico8_hd_true,
     .ldown = pico8_fq_ldown_dispatch,
-    .get_fa = fq_get_fa,
     .get_p_ = fq_get_p_,
     .set_m_ = fq_set_m_,
 };
@@ -355,6 +344,7 @@ static pico8_fq_t *pico8_fq_struct_new(const pico8_fq_keys_t *keys) {
 
     pico8_ke_struct_init(&self->header, &g_fq_struct_class, pico8_fq_def.me, pico8_fq_def.t);
     self->def = &pico8_fq_def;
+    self->header.fa = &self->def->fa;
     self->my = NULL;
     self->p_ = 0;
     self->has_p_ = 0;
diff --git a/src/pico8_wormhole.c b/src/pico8_wormhole.c
index c044514..4bd428c 100644
--- a/src/pico8_wormhole.c
+++ b/src/pico8_wormhole.c
@@ -199,11 +199,6 @@ static pico8_probe_t *mz_probe_of(void *self) {
     return ((pico8_mz_t *)self)->probe;
 }
 
-static int mz_get_fa(const void *self, pico8_fixed_t *out) {
-    *out = ((const pico8_mz_t *)self)->def->fa;
-    return 1;
-}
-
 static const pico8_vec_t *mz_get_ra(const void *self) {
     return ((const pico8_mz_t *)self)->ra;
 }
@@ -230,7 +225,6 @@ static const pico8_struct_class_t g_mz_struct_class = {
         },
     .probe_of = mz_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_MZ),
-    .get_fa = mz_get_fa,
     .get_ra = mz_get_ra,
     .get_p = mz_get_p,
     .get_p_ = mz_get_p_,
@@ -241,6 +235,7 @@ static pico8_mz_t *pico8_mz_struct_new(void) {
 
     pico8_ke_struct_init(&self->header, &g_mz_struct_class, pico8_mz_def.me, pico8_mz_def.t);
     self->def = &pico8_mz_def;
+    self->header.fa = &self->def->fa;
     self->probe = &self->probe_own;
     self->probe->def = &self->def->probe;
     self->oi = self->def->oi;
diff --git a/tests/test_pico8_mz.c b/tests/test_pico8_mz.c
index 357a5a1..3c4302e 100644
--- a/tests/test_pico8_mz.c
+++ b/tests/test_pico8_mz.c
@@ -300,7 +300,6 @@ static void check_fresh_mz(void *e) {
     const pico8_mz_t *mz = as_mz(e);
     const pico8_int_seq_t *kk;
     const pico8_mz_jj_t *jj;
-    pico8_fixed_t fa;
     static const int want_kk[9] = {7, 7, 6, 13, 5, 1, 1, 1, 0};
     int i;
 
@@ -314,7 +313,7 @@ static void check_fresh_mz(void *e) {
     assert(mz->probe->od == pico8_fixed_from_int(3));
     assert(mz->probe->def->fp == pico8_fixed_from_int(1));
     assert(mz->def->fa == pico8_fixed_from_int(5));
-    assert(mz->header.cls->get_fa(mz, &fa) && fa == mz->def->fa);
+    assert(mz->header.fa == &mz->def->fa);
 
     kk = mz->def->kk;
     assert(kk == &pico8_mz_kk_seq && kk->count == 9);
diff --git a/tests/test_pico8_struct_header.c b/tests/test_pico8_struct_header.c
new file mode 100644
index 0000000..d657e45
--- /dev/null
+++ b/tests/test_pico8_struct_header.c
@@ -0,0 +1,110 @@
+/* The kernel's component links on entities whose constructor `malloc`s
+ * without zeroing. The binary runs itself again with every `malloc`
+ * filling its block (MALLOC_PERTURB_ on glibc, MallocScribble on macOS),
+ * so a link no init set is not NULL. */
+
+#define _POSIX_C_SOURCE 200809L
+
+#include <assert.h>
+#include <stdio.h>
+#include <stdlib.h>
+#include <unistd.h>
+
+#include "../src/pico8_assets.h"
+#include "../src/pico8_bg.h"
+#include "../src/pico8_bootstrap.h"
+#include "../src/pico8_cart.h"
+#include "../src/pico8_dispatch_context.h"
+#include "../src/pico8_economy.h"
+#include "../src/pico8_gfx.h"
+#include "../src/pico8_input.h"
+#include "../src/pico8_layout.h"
+#include "../src/pico8_mem.h"
+#include "../src/pico8_mouse_reader.h"
+#include "../src/pico8_research.h"
+#include "../src/pico8_research_icon.h"
+#include "../src/pico8_trade.h"
+#include "../src/pico8_ui_button.h"
+
+static pico8_mem_t mem;
+static pico8_gfx_t gfx;
+static pico8_input_t input;
+
+static void rerun_with_malloc_filling(char **argv) {
+    if (getenv("PICO8_MALLOC_FILLS") != NULL) {
+        return;
+    }
+    setenv("MALLOC_PERTURB_", "90", 1);
+    setenv("MallocScribble", "1", 1);
+    setenv("PICO8_MALLOC_FILLS", "1", 1);
+    execv(argv[0], argv);
+    perror("execv");
+    exit(1);
+}
+
+/* Reads a fresh block on purpose: through `volatile`, which the compiler
+ * does not flag as uninitialized. */
+static void assert_malloc_fills(void) {
+    void *block = malloc(64);
+    const volatile unsigned char *bytes = block;
+    int i;
+
+    assert(block != NULL);
+    for (i = 0; i < 64; i++) {
+        assert(bytes[i] != 0 && bytes[i] == bytes[0] && "malloc must fill its block, or this test proves nothing");
+    }
+    free(block);
+}
+
+/* `fa`: the link a class carries or NULL. */
+static void assert_links(const pico8_struct_header_t *h, const pico8_fixed_t *fa) {
+    assert(h->fa == fa);
+}
+
+/* Non-carriers, one per constructor that does not go through ke. */
+static void test_non_carriers_have_null_links(void) {
+    pico8_ie_t *ie = pico8_ie_new(NULL);
+    pico8_ie_ke_t view = pico8_ie_ke_view(ie);
+
+    assert_links(&pico8_nf_new()->header, NULL);
+    assert_links(&pico8_lq_new(NULL)->header, NULL);
+    assert_links(&pico8_jb_new(NULL)->header, NULL);
+    assert_links(&pico8_hp_new(NULL)->header, NULL);
+    assert_links(&pico8_hy_new()->header, NULL);
+    assert_links(&pico8_trade_new(NULL)->header, NULL);
+    assert_links(&view.header, NULL);
+}
+
+/* Carriers: the archetype's field for a class that never writes it, the
+ * instance's otherwise. */
+static void test_carriers_link_their_storage(void) {
+    pico8_bg_t *bg = pico8_bg_new();
+    pico8_js_t *js = pico8_js_new(NULL);
+    pico8_ie_t *ie = pico8_ie_new(NULL);
+
+    assert_links(&bg->header, &bg->def->fa);
+    assert_links(&js->header, &js->fa);
+    assert_links(&ie->header, &ie->def->fa);
+}
+
+int main(int argc, char **argv) {
+    (void)argc;
+    rerun_with_malloc_filling(argv);
+    pico8_mem_init(&mem);
+    pico8_gfx_init(&gfx);
+    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
+    assert(pico8_load_map_json(&mem, "pico8/map.json"));
+    pico8_input_init(&input);
+    pico8_dispatch_set_gfx(&gfx);
+    pico8_dispatch_set_mem(&mem);
+    pico8_dispatch_set_input(&input);
+    pico8_cart_load();
+    pico8_jl();
+
+    assert_malloc_fills();
+    test_non_carriers_have_null_links();
+    test_carriers_link_their_storage();
+
+    printf("test_pico8_struct_header: all tests passed\n");
+    return 0;
+}
diff --git a/tests/test_pico8_ui.c b/tests/test_pico8_ui.c
index ba0a348..6150e66 100644
--- a/tests/test_pico8_ui.c
+++ b/tests/test_pico8_ui.c
@@ -741,12 +741,6 @@ static int giver_hd(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
     return g_giver_hovered;
 }
 
-static int giver_get_fa(const void *self, pico8_fixed_t *out) {
-    (void)self;
-    *out = pico8_fixed_from_int(15);
-    return 1;
-}
-
 static void giver_set_m_(void *self, int m_) {
     (void)self;
     (void)m_;
@@ -756,11 +750,11 @@ static const pico8_struct_class_t g_giver_class = {
     .hd = giver_hd,
     .ldown = giver_down,
     .rdown = giver_down,
-    .get_fa = giver_get_fa,
     .set_m_ = giver_set_m_,
 };
 
-static giver_t g_giver = {{.cls = &g_giver_class}};
+static const pico8_fixed_t g_giver_fa = 0xf0000;
+static giver_t g_giver = {{.cls = &g_giver_class, .fa = &g_giver_fa}};
 
 static void press(void *ok, int buttons) {
     mouse_at(64, 100, 0);
```

#### Diff de 2.14b (`git diff a b`)

```diff
diff --git a/src/pico8_entity.c b/src/pico8_entity.c
index b52ec0f..eed82e7 100644
--- a/src/pico8_entity.c
+++ b/src/pico8_entity.c
@@ -225,7 +225,7 @@ void pico8_dc(const char *nt) {
         for (i = 0; i < bucket->count; i++) {
             pico8_struct_header_t *qc = bucket->items[i];
             void (*draw)(void *self, const pico8_vec_t *ra) = kj ? qc->cls->kj : qc->cls->ev;
-            const pico8_vec_t *ra = qc->cls->get_ra != NULL ? qc->cls->get_ra(qc) : NULL;
+            const pico8_vec_t *ra = qc->ra != NULL ? *qc->ra : NULL;
 
             if (draw != NULL) {
                 draw(qc, ra);
diff --git a/src/pico8_entity_geom.c b/src/pico8_entity_geom.c
index 26fa7ef..2f63197 100644
--- a/src/pico8_entity_geom.c
+++ b/src/pico8_entity_geom.c
@@ -14,13 +14,13 @@ static pico8_vec_t pico8_entity_geom_zero(void) {
 
 /* `e`'s `ra` (`p` for the second), or (0,0) when `e` is NULL or the field is nil. */
 static pico8_vec_t pico8_entity_geom_ra(const pico8_struct_header_t *e) {
-    const pico8_vec_t *ra = e != NULL && e->cls->get_ra != NULL ? e->cls->get_ra(e) : NULL;
+    const pico8_vec_t *ra = e != NULL && e->ra != NULL ? *e->ra : NULL;
 
     return ra != NULL ? *ra : pico8_entity_geom_zero();
 }
 
 static pico8_vec_t pico8_entity_geom_p(const pico8_struct_header_t *e) {
-    const pico8_vec_t *p = e != NULL && e->cls->get_p != NULL ? e->cls->get_p(e) : NULL;
+    const pico8_vec_t *p = e != NULL && e->p != NULL ? *e->p : NULL;
 
     return p != NULL ? *p : pico8_entity_geom_zero();
 }
diff --git a/src/pico8_help.c b/src/pico8_help.c
index 0603c10..ddbe946 100644
--- a/src/pico8_help.c
+++ b/src/pico8_help.c
@@ -132,8 +132,6 @@ static const pico8_struct_class_t g_fv_struct_class = {
     .emitter_of = pico8_g__emitter_of,
     .probe_of = pico8_g__probe_of,
     .tags = PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_ra = pico8_g__get_ra,
-    .get_p = pico8_g__get_p,
     .get_kq = fv_get_kq,
     .get_ql = fv_get_ql,
     .get_fg = fv_get_fg,
diff --git a/src/pico8_layout.c b/src/pico8_layout.c
index 7dff2f2..0709fe0 100644
--- a/src/pico8_layout.c
+++ b/src/pico8_layout.c
@@ -288,13 +288,8 @@ static void pico8_jb_ol_dispatch(void *self, pico8_vec_t p, pico8_fixed_t a) {
     pico8_jb_ol(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_jb_t *)self, pico8_fixed_from_int(0), a);
 }
 
-static const pico8_vec_t *lq_get_ra(const void *self) {
-    return ((const pico8_lq_t *)self)->ra;
-}
-
 static const pico8_struct_class_t g_lq_struct_class = {
     .ol = pico8_lq_ol_dispatch,
-    .get_ra = lq_get_ra,
 };
 
 /* TROU-7 */
@@ -303,6 +298,7 @@ static pico8_lq_t *pico8_lq_struct_alloc(int n) {
     pico8_lq_t *self = malloc(sizeof *self);
 
     pico8_struct_header_init(&self->header, &g_lq_struct_class);
+    self->header.ra = &self->ra;
     self->c = 0;
     self->has_c = 0;
     self->bg = 0;
diff --git a/src/pico8_layout.h b/src/pico8_layout.h
index 1c2cd66..817c276 100644
--- a/src/pico8_layout.h
+++ b/src/pico8_layout.h
@@ -64,7 +64,7 @@ typedef struct {
     int bg_is_bool; /* 1: `bg` holds a boolean (0/1), 0: a fixed; read only when `has_bg` */
     pico8_fixed_t score;
     int has_score; /* 0: `score` is nil, and `score` is then never read */
-    pico8_vec_t *ra;
+    const pico8_vec_t *ra;
     pico8_lq_item_t *fs; /* NULL: not laid out yet */
     int fs_count;
     pico8_fixed_t w;
diff --git a/src/pico8_mouse_reader.c b/src/pico8_mouse_reader.c
index 5d645bf..5f8a037 100644
--- a/src/pico8_mouse_reader.c
+++ b/src/pico8_mouse_reader.c
@@ -24,13 +24,8 @@ static void pico8_nf_ph_dispatch(void *self, pico8_fixed_t t) {
     pico8_nf_ph(pico8_dispatch_input(), (pico8_nf_t *)self);
 }
 
-static const pico8_vec_t *nf_get_ra(const void *self) {
-    return ((const pico8_nf_t *)self)->ra;
-}
-
 static const pico8_struct_class_t g_nf_struct_class = {
     .states = {[PICO8_STATE_PH] = pico8_nf_ph_dispatch},
-    .get_ra = nf_get_ra,
 };
 
 static void pico8_nf_pq(pico8_mem_t *mem) {
@@ -45,6 +40,8 @@ static pico8_nf_t *pico8_nf_struct_new(void) {
 
     pico8_ke_struct_init(&self->header, &g_nf_struct_class, pico8_nf_def.me, pico8_nf_def.t);
     self->def = &pico8_nf_def;
+    /* Read-only: this module writes through its member. */
+    self->header.ra = (const pico8_vec_t *const *)&self->ra;
     self->ra = &g_nf_ra;
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("nf");
diff --git a/src/pico8_msgbox.c b/src/pico8_msgbox.c
index 18f87e6..482f88f 100644
--- a/src/pico8_msgbox.c
+++ b/src/pico8_msgbox.c
@@ -223,10 +223,6 @@ static void pico8_iv_kj_dispatch(void *self, const pico8_vec_t *ra) {
     pico8_iv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_iv_t *)self, p);
 }
 
-static const pico8_vec_t *iv_get_ra(const void *self) {
-    return ((const pico8_iv_t *)self)->ra;
-}
-
 static int iv_get_p_(const void *self) {
     const pico8_iv_t *iv = self;
 
@@ -236,7 +232,6 @@ static int iv_get_p_(const void *self) {
 static const pico8_struct_class_t g_iv_struct_class = {
     .kj = pico8_iv_kj_dispatch,
     .states = {[PICO8_STATE_PH] = pico8_iv_ph_dispatch},
-    .get_ra = iv_get_ra,
     .get_p_ = iv_get_p_,
 };
 
@@ -246,6 +241,7 @@ static pico8_iv_t *pico8_iv_struct_new(const pico8_iv_keys_t *keys) {
     pico8_ke_struct_init(&self->header, &g_iv_struct_class, pico8_iv_def.me, pico8_iv_def.t);
     self->def = &pico8_iv_def;
     self->header.fa = &self->def->fa;
+    self->header.ra = &self->ra;
     self->l = self->def->l;
     self->ra = NULL;
     self->p_ = 0;
diff --git a/src/pico8_msgbox.h b/src/pico8_msgbox.h
index 2216de8..38e7e9e 100644
--- a/src/pico8_msgbox.h
+++ b/src/pico8_msgbox.h
@@ -82,7 +82,7 @@ typedef struct {
     pico8_struct_header_t header;
     const pico8_iv_def_t *def;
     pico8_fixed_t l;
-    pico8_vec_t *ra;
+    const pico8_vec_t *ra;
     int p_;
     int has_p_; /* 0: `p_` is nil, and `p_` is then never read */
     const char *at1;
diff --git a/src/pico8_particle.c b/src/pico8_particle.c
index 5b57568..0c82e0c 100644
--- a/src/pico8_particle.c
+++ b/src/pico8_particle.c
@@ -10,24 +10,14 @@
 #include "pico8_fixed.h"
 #include "pico8_game_data.h"
 #include "pico8_planet.h"
-#include "pico8_probe.h"
 #include "pico8_vec.h"
 
-/* `fa`, and `ra` into `*ra` (the zero vector when nil), which the host of
- * `ei` holds for itself: `g_`'s line, or jo's family (`hj`, `pm`, `mx`, `ga`). */
-static pico8_fixed_t emitter_host_fa_ra(pico8_struct_header_t *self, pico8_vec_t *ra) {
-    const pico8_g__t *g_ = pico8_g__of(self);
-    const pico8_jo_t *jo;
-
-    if (g_ != NULL) {
-        *ra = g_->ra != NULL ? *g_->ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
-        return g_->def->fa;
-    }
-    assert(pico8_jo_is_family(self) &&
-           "ei: a host neither of g_'s line nor of jo's family, none of whose kj runs ei -- stop");
-    jo = (const pico8_jo_t *)self;
-    *ra = jo->ra != NULL ? *jo->ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
-    return jo->def->fa;
+/* `fa`, and `ra` into `*ra` (the zero vector when nil), of the host of `ei`. */
+static pico8_fixed_t emitter_host_fa_ra(const pico8_struct_header_t *self, pico8_vec_t *ra) {
+    assert(self->fa != NULL && self->ra != NULL &&
+           "ei: a host that carries no fa or no ra, none of whose kj runs ei -- stop");
+    *ra = *self->ra != NULL ? **self->ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
+    return *self->fa;
 }
 
 static int pico8_particle_to_int(pico8_fixed_t x) {
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index 0b073b5..5f1730c 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -76,14 +76,6 @@ void pico8_jo_ll_dispatch(void *self, pico8_fixed_t t) {
     pico8_jo_ll((pico8_jo_t *)self, t);
 }
 
-const pico8_vec_t *pico8_jo_get_ra(const void *self) {
-    return ((const pico8_jo_t *)self)->ra;
-}
-
-const pico8_vec_t *pico8_jo_get_p(const void *self) {
-    return ((const pico8_jo_t *)self)->p;
-}
-
 const pico8_qz_box_t *pico8_jo_get_kq(const void *self) {
     return &((const pico8_jo_t *)self)->def->kq;
 }
@@ -118,8 +110,6 @@ static const pico8_struct_class_t g_jo_struct_class = {
     .cn = pico8_jo_cn_dispatch,
     .states = {[PICO8_STATE_LL] = pico8_jo_ll_dispatch},
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_ra = pico8_jo_get_ra,
-    .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
@@ -131,6 +121,9 @@ void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, con
     pico8_ke_struct_init(&self->header, cls, def->me, def->t);
     self->def = def;
     self->header.fa = &self->def->fa;
+    /* Read-only: the planet modules write through their members. */
+    self->header.ra = (const pico8_vec_t *const *)&self->ra;
+    self->header.p = (const pico8_vec_t *const *)&self->p;
     self->ma = self->def->ma;
     self->kw = &g_jo_kw_shared;
     self->id = NULL;
diff --git a/src/pico8_planet.h b/src/pico8_planet.h
index 0a3ed19..2118a00 100644
--- a/src/pico8_planet.h
+++ b/src/pico8_planet.h
@@ -103,8 +103,6 @@ void pico8_jo_kf_dispatch(void *self, pico8_struct_header_t *sw);
 
 /* The field accessors jo's descriptor holds, for a daughter's `static const`
  * descriptor to name. */
-const pico8_vec_t *pico8_jo_get_ra(const void *self);
-const pico8_vec_t *pico8_jo_get_p(const void *self);
 const pico8_qz_box_t *pico8_jo_get_kq(const void *self);
 int pico8_jo_get_fx(const void *self);
 int pico8_jo_get_p_(const void *self);
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index bb6eef3..581d882 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -50,8 +50,6 @@ static const pico8_struct_class_t g_qh_struct_class = {
     .parent = pico8_jo_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_ra = pico8_jo_get_ra,
-    .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
@@ -126,8 +124,6 @@ static const pico8_struct_class_t g_nl_struct_class = {
     .parent = pico8_qh_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_ra = pico8_jo_get_ra,
-    .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
@@ -176,8 +172,6 @@ static const pico8_struct_class_t g_os_struct_class = {
     .parent = pico8_qh_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_ra = pico8_jo_get_ra,
-    .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
@@ -246,8 +240,6 @@ static const pico8_struct_class_t g_hj_struct_class = {
     .emitter_of = pico8_hj_emitter_of,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_ra = pico8_jo_get_ra,
-    .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
@@ -339,8 +331,6 @@ static const pico8_struct_class_t g_pm_struct_class = {
     .emitter_of = pico8_hj_emitter_of,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_ra = pico8_jo_get_ra,
-    .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
@@ -495,8 +485,6 @@ static const pico8_struct_class_t g_ga_struct_class = {
     .emitter_of = ga_emitter_of,
     /* ga's cart list of tags is empty and replaces jo's: never jo's. */
     .tags = 0,
-    .get_ra = pico8_jo_get_ra,
-    .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
diff --git a/src/pico8_planet_family_trade.c b/src/pico8_planet_family_trade.c
index 42c10d6..6695c36 100644
--- a/src/pico8_planet_family_trade.c
+++ b/src/pico8_planet_family_trade.c
@@ -178,8 +178,6 @@ static const pico8_struct_class_t g_nr_struct_class = {
     .parent = pico8_qh_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_ra = pico8_jo_get_ra,
-    .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
@@ -243,8 +241,6 @@ static const pico8_struct_class_t g_fr_struct_class = {
     .parent = pico8_nr_struct_class,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_ra = pico8_jo_get_ra,
-    .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
@@ -344,8 +340,6 @@ static const pico8_struct_class_t g_mx_struct_class = {
     .emitter_of = pico8_hj_emitter_of,
     .probe_of = pico8_qh_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_ra = pico8_jo_get_ra,
-    .get_p = pico8_jo_get_p,
     .get_kq = pico8_jo_get_kq,
     .get_fx = pico8_jo_get_fx,
     .get_p_ = pico8_jo_get_p_,
diff --git a/src/pico8_probe.c b/src/pico8_probe.c
index 28f747a..ebc08a9 100644
--- a/src/pico8_probe.c
+++ b/src/pico8_probe.c
@@ -395,14 +395,6 @@ pico8_qy_t *pico8_g__mm(void *self_ptr) {
     return particle;
 }
 
-const pico8_vec_t *pico8_g__get_ra(const void *self) {
-    return ((const pico8_g__t *)self)->ra;
-}
-
-const pico8_vec_t *pico8_g__get_p(const void *self) {
-    return ((const pico8_g__t *)self)->p;
-}
-
 /* `fx` is a fixed here, and a fixed is always true in Lua, even 0. */
 int pico8_g__get_fx(const void *self) {
     (void)self;
@@ -424,8 +416,6 @@ static const pico8_struct_class_t g_g__struct_class = {
     .emitter_of = pico8_g__emitter_of,
     .probe_of = pico8_g__probe_of,
     .tags = PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER),
-    .get_ra = pico8_g__get_ra,
-    .get_p = pico8_g__get_p,
     .get_fx = pico8_g__get_fx,
     .get_p_ = pico8_g__get_p_,
 };
@@ -434,6 +424,8 @@ void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls, con
     pico8_ke_struct_init(&self->header, cls, def->me, def->t);
     self->def = def;
     self->header.fa = &self->def->fa;
+    self->header.ra = &self->ra;
+    self->header.p = &self->p;
     self->emitter = &self->emitter_own;
     self->emitter->def = &self->def->emitter;
     self->emitter->qy = NULL;
diff --git a/src/pico8_probe.h b/src/pico8_probe.h
index 96dce07..41aea93 100644
--- a/src/pico8_probe.h
+++ b/src/pico8_probe.h
@@ -39,7 +39,7 @@ typedef struct {
     pico8_emitter_t *emitter;
     pico8_probe_t *probe;
     const pico8_vec_t *ra;
-    pico8_vec_t *p;
+    const pico8_vec_t *p;
     int p_;
     int has_p_; /* 0: `p_` is nil, and is then never read */
     pico8_emitter_t emitter_own;
@@ -54,8 +54,6 @@ pico8_g__t *pico8_g__of(pico8_struct_header_t *e);
  * `fv`'s to name. */
 pico8_emitter_t *pico8_g__emitter_of(void *self);
 pico8_probe_t *pico8_g__probe_of(void *self);
-const pico8_vec_t *pico8_g__get_ra(const void *self);
-const pico8_vec_t *pico8_g__get_p(const void *self);
 int pico8_g__get_fx(const void *self);
 int pico8_g__get_p_(const void *self);
 
diff --git a/src/pico8_research_icon.c b/src/pico8_research_icon.c
index 6912174..503ad8b 100644
--- a/src/pico8_research_icon.c
+++ b/src/pico8_research_icon.c
@@ -202,10 +202,6 @@ static int pico8_ie_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_v
     return pico8_gk((const pico8_struct_header_t *)self, qa, ra);
 }
 
-static const pico8_vec_t *ie_get_ra(const void *self) {
-    return ((const pico8_ie_t *)self)->ra;
-}
-
 static const pico8_qz_box_t *ie_get_kq(const void *self) {
     return &((const pico8_ie_t *)self)->def->kq;
 }
@@ -222,7 +218,6 @@ static const pico8_struct_class_t g_ie_struct_class = {
     .hd = pico8_ie_hd_dispatch,
     .hb = pico8_ie_hb_dispatch,
     .ldown = pico8_ie_ldown_dispatch,
-    .get_ra = ie_get_ra,
     .get_kq = ie_get_kq,
     .set_m_ = ie_set_m_,
 };
@@ -234,6 +229,7 @@ static pico8_ie_t *pico8_ie_struct_new(const pico8_ie_keys_t *keys) {
     pico8_ke_struct_init(&self->header, &g_ie_struct_class, pico8_ie_def.me, pico8_ie_def.t);
     self->def = &pico8_ie_def;
     self->header.fa = &self->def->fa;
+    self->header.ra = &self->ra;
     self->ra = &self->def->ra;
     self->ke = NULL;
     self->jb = NULL;
diff --git a/src/pico8_scroll.c b/src/pico8_scroll.c
index f21b845..96f90dc 100644
--- a/src/pico8_scroll.c
+++ b/src/pico8_scroll.c
@@ -27,18 +27,8 @@ static void pico8_qb_ph_dispatch(void *self, pico8_fixed_t t) {
     pico8_qb_ph(pico8_dispatch_input(), (pico8_qb_t *)self);
 }
 
-static const pico8_vec_t *qb_get_ra(const void *self) {
-    return ((const pico8_qb_t *)self)->ra;
-}
-
-static const pico8_vec_t *qb_get_p(const void *self) {
-    return ((const pico8_qb_t *)self)->p;
-}
-
 static const pico8_struct_class_t g_qb_struct_class = {
     .states = {[PICO8_STATE_PH] = pico8_qb_ph_dispatch},
-    .get_ra = qb_get_ra,
-    .get_p = qb_get_p,
 };
 
 static pico8_qb_t *pico8_qb_struct_new(void) {
@@ -46,6 +36,8 @@ static pico8_qb_t *pico8_qb_struct_new(void) {
 
     pico8_ke_struct_init(&self->header, &g_qb_struct_class, pico8_qb_def.me, pico8_qb_def.t);
     self->def = &pico8_qb_def;
+    self->header.ra = &self->ra;
+    self->header.p = &self->p;
     self->v = &self->def->v;
     self->ma = self->def->ma;
     self->p = &self->def->p;
diff --git a/src/pico8_slipway.c b/src/pico8_slipway.c
index eaac938..e3bb528 100644
--- a/src/pico8_slipway.c
+++ b/src/pico8_slipway.c
@@ -28,40 +28,17 @@ static const pico8_struct_class_t g_jc_too_far_struct_class = {0};
  * overwrites it. */
 static pico8_jc_too_far_t g_jc_too_far = {{.cls = &g_jc_too_far_struct_class}, "-too far-", NULL};
 
-static const pico8_vec_t *jc_mouse_point_get_ra(const void *self) {
-    return ((const pico8_jc_mouse_point_t *)self)->ra;
-}
-
-static const pico8_vec_t *jc_mouse_point_get_p(const void *self) {
-    return ((const pico8_jc_mouse_point_t *)self)->p;
-}
-
-static const pico8_struct_class_t g_jc_mouse_point_struct_class = {
-    .get_ra = jc_mouse_point_get_ra,
-    .get_p = jc_mouse_point_get_p,
-};
-
-static const pico8_jc_mouse_point_t *jc_end_point(const pico8_struct_header_t *end) {
-    assert(end->cls == &g_jc_mouse_point_struct_class &&
-           "jc: an end of a slipway is a planet or the mouse point jc:hq hangs on re -- stop");
-    return (const pico8_jc_mouse_point_t *)end;
-}
+static const pico8_struct_class_t g_jc_mouse_point_struct_class = {0};
 
 pico8_vec_t pico8_jc_end_p(const pico8_struct_header_t *end) {
-    const pico8_vec_t *p = NULL;
+    const pico8_vec_t *p = end != NULL && end->p != NULL ? *end->p : NULL;
 
-    if (end != NULL) {
-        p = pico8_jo_is_family(end) ? ((const pico8_jo_t *)end)->p : jc_end_point(end)->p;
-    }
     return p != NULL ? *p : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
 }
 
 pico8_vec_t pico8_jc_end_ra(const pico8_struct_header_t *end) {
-    const pico8_vec_t *ra = NULL;
+    const pico8_vec_t *ra = end != NULL && end->ra != NULL ? *end->ra : NULL;
 
-    if (end != NULL) {
-        ra = pico8_jo_is_family(end) ? ((const pico8_jo_t *)end)->ra : jc_end_point(end)->ra;
-    }
     return ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
 }
 
@@ -128,6 +105,8 @@ void pico8_jc_hq(pico8_jc_t *self, pico8_vec_t *mp) {
         pico8_jc_mouse_point_t *point = malloc(sizeof *point);
         pico8_vec_t *p_heap = malloc(sizeof *p_heap);
         pico8_struct_header_init(&point->header, &g_jc_mouse_point_struct_class);
+        point->header.ra = &point->ra;
+        point->header.p = &point->p;
         point->ra = mp;
         *p_heap = pico8_vec_div(mp != NULL ? *mp : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0)),
                                 pico8_fixed_from_int(128));
diff --git a/src/pico8_slipway.h b/src/pico8_slipway.h
index cbda0c0..7ae00ae 100644
--- a/src/pico8_slipway.h
+++ b/src/pico8_slipway.h
@@ -63,8 +63,8 @@ typedef struct {
  * never freed, nor its `p`; `ra` is the caller's `mp` itself, not a copy. */
 typedef struct {
     pico8_struct_header_t header;
-    pico8_vec_t *ra;
-    pico8_vec_t *p;
+    const pico8_vec_t *ra;
+    const pico8_vec_t *p;
 } pico8_jc_mouse_point_t;
 
 void pico8_jc_init(void);
@@ -77,9 +77,9 @@ void pico8_jc_shutdown(void);
 int pico8_jc_ready(void);
 const pico8_struct_class_t *pico8_jc_struct_class(void);
 
-/* `p`/`ra` of an end of a slipway: a planet, or the mouse point `jc:hq`
- * hangs on `re` over nothing; NULL, or an absent vector, reads as the zero
- * vector. Asserts on any other end. */
+/* `p`/`ra` of an end of a slipway (a planet, or the mouse point `jc:hq`
+ * hangs on `re` over nothing), read through its link: NULL, an end without
+ * the link, or an absent vector reads as the zero vector. */
 pico8_vec_t pico8_jc_end_p(const pico8_struct_header_t *end);
 pico8_vec_t pico8_jc_end_ra(const pico8_struct_header_t *end);
 
diff --git a/src/pico8_struct.h b/src/pico8_struct.h
index 016e5cb..5332c38 100644
--- a/src/pico8_struct.h
+++ b/src/pico8_struct.h
@@ -84,6 +84,8 @@ typedef struct {
     const char *me;
     pico8_fixed_t t;
     const pico8_fixed_t *fa;
+    const pico8_vec_t *const *ra;
+    const pico8_vec_t *const *p;
 } pico8_struct_header_t;
 
 /* Every constructor calls it first: its `malloc` leaves the header
@@ -147,8 +149,6 @@ struct pico8_struct_class {
      * reader returns NULL, or 0 leaving *out untouched, when the field is
      * nil; `get_fx` and `get_p_` return its Lua truth. `set_m_` also marks
      * `m_` present. Each is only ever called as `e->cls->get_X(e, ...)`. */
-    const pico8_vec_t *(*get_ra)(const void *self);
-    const pico8_vec_t *(*get_p)(const void *self);
     const pico8_qz_box_t *(*get_kq)(const void *self);
     int (*get_ql)(const void *self, pico8_fixed_t *out);
     int (*get_fg)(const void *self, pico8_fixed_t *out);
diff --git a/src/pico8_ui_button.c b/src/pico8_ui_button.c
index d66794d..f009977 100644
--- a/src/pico8_ui_button.c
+++ b/src/pico8_ui_button.c
@@ -58,10 +58,6 @@ static void pico8_js_hb_dispatch(void *self) {
     }
 }
 
-static const pico8_vec_t *js_get_ra(const void *self) {
-    return ((const pico8_js_t *)self)->ra;
-}
-
 static const pico8_qz_box_t *js_get_kq(const void *self) {
     return ((const pico8_js_t *)self)->kq;
 }
@@ -91,7 +87,6 @@ static const pico8_struct_class_t g_js_struct_class = {
     .has_hb = pico8_js_has_hb,
     .ldown = pico8_js_ldown_dispatch,
     .states = {[PICO8_STATE_PH] = pico8_js_ph_dispatch},
-    .get_ra = js_get_ra,
     .get_kq = js_get_kq,
     .get_fg = js_get_fg,
     .set_m_ = js_set_m_,
@@ -129,6 +124,7 @@ static pico8_js_t *pico8_js_struct_new(const pico8_js_keys_t *keys) {
     self->def = &pico8_js_def;
     self->fa = self->def->fa;
     self->header.fa = &self->fa;
+    self->header.ra = &self->ra;
     if (keys != NULL) {
         if (keys->has_fa) {
             self->fa = keys->fa;
diff --git a/src/pico8_ui_button.h b/src/pico8_ui_button.h
index 5d0e809..df549bf 100644
--- a/src/pico8_ui_button.h
+++ b/src/pico8_ui_button.h
@@ -35,7 +35,7 @@ struct pico8_js {
     pico8_lq_t *h_;
     int has_h_; /* 0: nil; 1: present, and a NULL `h_` is `false` */
     pico8_jb_t *jb;
-    pico8_vec_t *ra;
+    const pico8_vec_t *ra;
     pico8_vec_t *qf;
     pico8_js_fn my;
     int has_my; /* 0: nil; 1: present, and a NULL `my` is `false` */
diff --git a/src/pico8_wormhole.c b/src/pico8_wormhole.c
index 4bd428c..95a10e9 100644
--- a/src/pico8_wormhole.c
+++ b/src/pico8_wormhole.c
@@ -199,14 +199,6 @@ static pico8_probe_t *mz_probe_of(void *self) {
     return ((pico8_mz_t *)self)->probe;
 }
 
-static const pico8_vec_t *mz_get_ra(const void *self) {
-    return ((const pico8_mz_t *)self)->ra;
-}
-
-static const pico8_vec_t *mz_get_p(const void *self) {
-    return ((const pico8_mz_t *)self)->p;
-}
-
 static int mz_get_p_(const void *self) {
     const pico8_mz_t *mz = self;
 
@@ -225,8 +217,6 @@ static const pico8_struct_class_t g_mz_struct_class = {
         },
     .probe_of = mz_probe_of,
     .tags = PICO8_TAG(PICO8_TAG_MZ),
-    .get_ra = mz_get_ra,
-    .get_p = mz_get_p,
     .get_p_ = mz_get_p_,
 };
 
@@ -236,6 +226,8 @@ static pico8_mz_t *pico8_mz_struct_new(void) {
     pico8_ke_struct_init(&self->header, &g_mz_struct_class, pico8_mz_def.me, pico8_mz_def.t);
     self->def = &pico8_mz_def;
     self->header.fa = &self->def->fa;
+    self->header.ra = &self->ra;
+    self->header.p = &self->p;
     self->probe = &self->probe_own;
     self->probe->def = &self->def->probe;
     self->oi = self->def->oi;
diff --git a/src/pico8_wormhole.h b/src/pico8_wormhole.h
index 9f0a06c..174e545 100644
--- a/src/pico8_wormhole.h
+++ b/src/pico8_wormhole.h
@@ -31,8 +31,8 @@ typedef struct {
     pico8_fixed_t oi;
     pico8_mz_jj_t *jj;
     pico8_ents_t *ox;
-    pico8_vec_t *ra;
-    pico8_vec_t *p;
+    const pico8_vec_t *ra;
+    const pico8_vec_t *p;
     pico8_struct_header_t *ij; /* borrowed, never freed */
     int p_;
     int has_p_; /* 0: `p_` is nil, and is then never read */
diff --git a/src/pico8_wormhole_display.c b/src/pico8_wormhole_display.c
index decdaf0..a3c54de 100644
--- a/src/pico8_wormhole_display.c
+++ b/src/pico8_wormhole_display.c
@@ -11,8 +11,8 @@
 static pico8_vec_t pico8_mzd_ra_of_ij(const pico8_mz_t *self) {
     const pico8_vec_t *ra = NULL;
 
-    if (self->ij != NULL && self->ij->cls->get_ra != NULL) {
-        ra = self->ij->cls->get_ra(self->ij);
+    if (self->ij != NULL && self->ij->ra != NULL) {
+        ra = *self->ij->ra;
     }
     return ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
 }
diff --git a/tests/test_pico8_g_.c b/tests/test_pico8_g_.c
index 04d68be..8947703 100644
--- a/tests/test_pico8_g_.c
+++ b/tests/test_pico8_g_.c
@@ -347,7 +347,7 @@ static uint32_t fold_probe(uint32_t h, pico8_g__t *e) {
     size_t i;
 
     assert(is_probe(&e->header));
-    assert(e->header.cls->get_p(e) == e->p);
+    assert(e->header.p == &e->p);
     h = fold_fixed_member(h, e->def->fx);
     h = fold_string_member(h, pr->def->dq);
     h = fold_fixed_member(h, pr->od);
diff --git a/tests/test_pico8_jc.c b/tests/test_pico8_jc.c
index 28e65e1..74875b8 100644
--- a/tests/test_pico8_jc.c
+++ b/tests/test_pico8_jc.c
@@ -767,7 +767,7 @@ static void test_hq_mouse_point(void) {
     p1 = vec_of(mouse_point_p(r1));
     assert(same_vec(p1, pico8_vec_div(vec_of(mouse_point_ra(r1)), pico8_fixed_from_int(128))));
     assert(mouse_point_p(r1) != m1);
-    assert(cls_of(r1)->get_p(r1) == mouse_point_p(r1));
+    assert(*((const pico8_struct_header_t *)r1)->p == mouse_point_p(r1));
     assert(!pico8_gd(r1));
     assert(!ents_listed(pico8_hv(), r1) && !ents_listed(pico8_cg("jc"), r1) && !ents_listed(pico8_cg("np"), r1));
     h = fnv(h, (uint32_t)p1.x);
diff --git a/tests/test_pico8_lq.c b/tests/test_pico8_lq.c
index 76a73f0..8ee8d1e 100644
--- a/tests/test_pico8_lq.c
+++ b/tests/test_pico8_lq.c
@@ -335,7 +335,7 @@ static uint32_t fold_built_from(uint32_t h, const char *label, const pico8_lq_ke
     const void *lq = pico8_lq_new(k);
     uint32_t one = fold_row_from(2166136261u, lq, k, nil_at);
 
-    assert(((const pico8_struct_header_t *)lq)->cls->get_ra(lq) == as_lq(lq)->ra);
+    assert(((const pico8_struct_header_t *)lq)->ra == &as_lq(lq)->ra);
 
     printf("  %s: w=%d fs=%d n=%d 0x%08X\n", label, pico8_fixed_flr_int(w_of(lq)), fs_count(lq), row_count(lq),
            (unsigned)one);
diff --git a/tests/test_pico8_mo.c b/tests/test_pico8_mo.c
index 9b6a155..fe461c0 100644
--- a/tests/test_pico8_mo.c
+++ b/tests/test_pico8_mo.c
@@ -80,20 +80,14 @@ typedef struct {
     const pico8_vec_t *ra;
 } ra_only_t;
 
-static const pico8_vec_t *ra_only_get_ra(const void *self) {
-    const ra_only_t *e = self;
-
-    return e->ra;
-}
-
-static const pico8_struct_class_t g_ra_only_class = {.get_ra = ra_only_get_ra};
+static const pico8_struct_class_t g_ra_only_class = {0};
 
 /* `gd` reads `mo.ra` as (0,0): 64 off is inside the 76 margin, 77 is not. */
 static void test_gd_on_placeholder(void) {
     pico8_vec_t near_ra = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
     pico8_vec_t far_ra = pico8_vec_v(pico8_fixed_from_int(64 + 77), pico8_fixed_from_int(64));
-    ra_only_t near_e = {{.cls = &g_ra_only_class}, NULL};
-    ra_only_t far_e = {{.cls = &g_ra_only_class}, NULL};
+    ra_only_t near_e = {{.cls = &g_ra_only_class, .ra = &near_e.ra}, NULL};
+    ra_only_t far_e = {{.cls = &g_ra_only_class, .ra = &far_e.ra}, NULL};
     int near_hit;
     int far_hit;
 
diff --git a/tests/test_pico8_mz.c b/tests/test_pico8_mz.c
index 3c4302e..c73ec3d 100644
--- a/tests/test_pico8_mz.c
+++ b/tests/test_pico8_mz.c
@@ -428,7 +428,7 @@ static uint32_t fold_hq(uint32_t h, const pico8_mz_t *mz, int want_kind) {
     printf("[hq] et kind %d\n", kind);
     fflush(stdout);
     assert(kind == want_kind);
-    assert(mz->header.cls->get_ra(mz) == mz->ra && mz->header.cls->get_p(mz) == mz->p);
+    assert(mz->header.ra == &mz->ra && mz->header.p == &mz->p);
     h = fold_vec(h, mz->ra);
     h = fold_vec(h, mz->p);
     h = fnv(h, (uint32_t)kind);
diff --git a/tests/test_pico8_nf.c b/tests/test_pico8_nf.c
index 6fde293..6665274 100644
--- a/tests/test_pico8_nf.c
+++ b/tests/test_pico8_nf.c
@@ -279,7 +279,7 @@ static void test_nf_is_a_struct_sharing_its_buttons(void) {
     assert(((const pico8_struct_header_t *)generic)->cls->kj == NULL);
     assert(pico8_nf_l() != NULL && pico8_nf_r() != NULL);
     assert(pico8_nf_l() != pico8_nf_r());
-    assert(((const pico8_struct_header_t *)built)->cls->get_ra(built) == ((const pico8_nf_t *)built)->ra);
+    assert(*((const pico8_struct_header_t *)built)->ra == ((const pico8_nf_t *)built)->ra);
 }
 
 int main(void) {
diff --git a/tests/test_pico8_qb.c b/tests/test_pico8_qb.c
index 529a9cc..2f3f336 100644
--- a/tests/test_pico8_qb.c
+++ b/tests/test_pico8_qb.c
@@ -364,8 +364,8 @@ static void test_qb_is_a_struct(void) {
     assert(is_a_qb_struct(pico8_mo()) && is_a_qb_struct(generic));
     assert(ents_listed(pico8_hv(), generic) && !in_any_cv(generic));
     check_fresh_mo(generic);
-    assert(generic->header.cls->get_ra(generic) == generic->ra);
-    assert(generic->header.cls->get_p(generic) == generic->p);
+    assert(generic->header.ra == &generic->ra);
+    assert(generic->header.p == &generic->p);
     pico8_qb_struct_class()->states[PICO8_STATE_PH](generic, pico8_fixed_from_int(0));
     pico8_qb_md(pico8_dispatch_gfx(), generic, pico8_fixed_from_int(1));
     assert(pico8_qb_struct_class()->kj == NULL);
diff --git a/tests/test_pico8_struct_header.c b/tests/test_pico8_struct_header.c
index d657e45..a64ada1 100644
--- a/tests/test_pico8_struct_header.c
+++ b/tests/test_pico8_struct_header.c
@@ -56,23 +56,28 @@ static void assert_malloc_fills(void) {
     free(block);
 }
 
-/* `fa`: the link a class carries or NULL. */
-static void assert_links(const pico8_struct_header_t *h, const pico8_fixed_t *fa) {
+/* Each link: the one a class carries, or NULL. */
+static void assert_links(const pico8_struct_header_t *h, const pico8_fixed_t *fa, const pico8_vec_t *const *ra,
+                         const pico8_vec_t *const *p) {
     assert(h->fa == fa);
+    assert(h->ra == ra);
+    assert(h->p == p);
 }
 
 /* Non-carriers, one per constructor that does not go through ke. */
 static void test_non_carriers_have_null_links(void) {
     pico8_ie_t *ie = pico8_ie_new(NULL);
     pico8_ie_ke_t view = pico8_ie_ke_view(ie);
+    pico8_nf_t *nf = pico8_nf_new();
+    pico8_lq_t *lq = pico8_lq_new(NULL);
 
-    assert_links(&pico8_nf_new()->header, NULL);
-    assert_links(&pico8_lq_new(NULL)->header, NULL);
-    assert_links(&pico8_jb_new(NULL)->header, NULL);
-    assert_links(&pico8_hp_new(NULL)->header, NULL);
-    assert_links(&pico8_hy_new()->header, NULL);
-    assert_links(&pico8_trade_new(NULL)->header, NULL);
-    assert_links(&view.header, NULL);
+    assert_links(&nf->header, NULL, (const pico8_vec_t *const *)&nf->ra, NULL);
+    assert_links(&lq->header, NULL, &lq->ra, NULL);
+    assert_links(&pico8_jb_new(NULL)->header, NULL, NULL, NULL);
+    assert_links(&pico8_hp_new(NULL)->header, NULL, NULL, NULL);
+    assert_links(&pico8_hy_new()->header, NULL, NULL, NULL);
+    assert_links(&pico8_trade_new(NULL)->header, NULL, NULL, NULL);
+    assert_links(&view.header, NULL, NULL, NULL);
 }
 
 /* Carriers: the archetype's field for a class that never writes it, the
@@ -82,9 +87,9 @@ static void test_carriers_link_their_storage(void) {
     pico8_js_t *js = pico8_js_new(NULL);
     pico8_ie_t *ie = pico8_ie_new(NULL);
 
-    assert_links(&bg->header, &bg->def->fa);
-    assert_links(&js->header, &js->fa);
-    assert_links(&ie->header, &ie->def->fa);
+    assert_links(&bg->header, &bg->def->fa, NULL, NULL);
+    assert_links(&js->header, &js->fa, &js->ra, NULL);
+    assert_links(&ie->header, &ie->def->fa, &ie->ra, NULL);
 }
 
 int main(int argc, char **argv) {
```

### Comptes, mesurés sur la copie

| Mesure | `p` | `a` | `b` |
|---|---|---|---|
| `accesseurs-q6` (lignes / fichiers) | 237 / 33 | **193 / 32** | **123 / 21** |
| `lien-parent` | 111 / 27 | 111 / 27 | **110 / 27** (`particle.c:19` sort) |
| `parente`, `void-ptr` | 10 / 3, 8 / 4 | identiques | identiques |
| `défauts` | 0, gardé | 0, gardé | 0, gardé |
| `inclusion-mutuelle` (paires / modules, lignes du `--check`) | 17 / 16, 43 | **17 / 16, 43** | **16 / 15, 40** |
| graphe d'usage (`usegraph.py`) | 20 | 20 | **19** (`particle` ↔ `probe` sort) |
| pied `résidu 2` | 404 / 63 | **360 / 63** | **286 / 54** |
| `--champs` | 242 ; 15 ; 20 | **243 ; 15 ; 20** (`fa` du noyau) | **245 ; 15 ; 20** (`ra`, `p` du noyau) |
| casts (grep de la réserve 4 de 2.13a) | 530 | **518** | **500**, plus 3 casts de qualificatif (`(const pico8_vec_t *const *)`, que le motif ne lit pas) dans `src/` et 1 dans `test_pico8_struct_header.c` |
| tests de classe (`->cls`, `header.cls`, `_struct_class()`) | 708 | **693** | **677** |
| `sizeof(pico8_struct_header_t)` (x86-64) | 24 | **32** | **48** |
| `COMPOSITION_RESIDUE_GUARD` | 8 catégories | 8 | 8 |

**Attribution d'`accesseurs-q6`** (annexe A de l'étude 2.10, même découpage ; seuls les numéros de ligne ont bougé depuis `d9a2b2d`) : chaque ligne sort avec le dernier emplacement qu'elle nomme. `a`, **44** : `planet_family.c` 6, `msgbox.c` 4, `ui_button.c` 4, `planet_family_trade.c` 3, `ui.c` 3, `T:ui.c` 2, et 1 ou 2 dans `bg.c`, `entity.c`, `entity_geom.c`, `help.c`, `planet.c`, `planet.h`, `probe.c`, `probe.h`, `research_icon.c`, `score_panel.c`, `slipway.c`, `struct.h`, `wormhole.c`, `T:mz.c` ; le fichier qui sort est `bg.c` (33 → 32). `b`, **70** : `planet_family.c` 12, `planet_family_trade.c` 6, 4 dans `planet.c`, `probe.c`, `scroll.c`, `slipway.c`, `wormhole.c`, 2 dans `entity_geom.c`, `help.c`, `layout.c`, `mouse_reader.c`, `msgbox.c`, `planet.h`, `probe.h`, `research_icon.c`, `struct.h`, `ui_button.c`, `wormhole_display.c`, `T:mo.c`, `T:qb.c`, 1 dans `entity.c`, `T:g_.c`, `T:jc.c`, `T:lq.c`, `T:mz.c`, `T:nf.c` ; onze fichiers sortent (32 → 21). `tests/test_pico8_mz.c:431` nomme `get_ra` et `get_p` : il sort en `b`, pas avant. Somme 44 + 70 = 114 ; 237 − 114 = **123**, les 123 lignes de `kq`, `ql`, `fg`, `mt`, `fx`, `p_`, `m_` (2.14c à 2.14d2).

**Pied** : `a` − 44 lignes, même 63 fichiers ; `b` − 70 (`accesseurs-q6`), − 1 (`lien-parent`), − 3 (`inclusion-mutuelle`) = − 74, et neuf fichiers que plus aucune catégorie ne compte (63 → 54).

**Casts et tests de classe** : `a` − 12 casts (les implémentations `((const pico8_X_t *)self)->…fa`), − 15 tests de classe (`qc->cls->get_fa` ×2, `e->cls->get_fa` ×4, `mz->header.cls->get_fa`, `self->cls = cls` de `ke.c`, les sept `…header.cls = &g_…` devenus l'init) ; `b` − 18 casts (quinze implémentations, `jc_end_point`, les deux casts `pico8_jo_t` des bouts, celui de l'hôte d'émetteur, moins les casts des TU gardés), − 16 tests de classe (lecteurs `->cls->get_ra`/`get_p`, `jc_end_point`). Les trois casts de qualificatif ajoutés sont ceux que l'étude 2.10 prévoit, un par membre écrit à travers.

### Contrat pour 2.14c et la suite (à recopier dans leurs PRD)

| Point | Pour | Ce qu'il faut savoir |
|---|---|---|
| `pico8_struct_header_init(self, cls)` | 2.14c, 2.14d1, 2.14d2, 2.15 | met **tout** le noyau à zéro (littéral composé) : un lien neuf n'a qu'à être déclaré dans `pico8_struct_header_t`, puis posé par l'init du porteur après elle ; ne jamais réécrire `cls` à la main (grep de Sortie 2) |
| `test_pico8_struct_header.c` | idem | chaque lien neuf entre dans `assert_links` (un argument de plus, l'attendu de chaque type construit) ; le TU se relance sous `MALLOC_PERTURB_=90` et refuse de tourner sans (`assert_malloc_fills`) |
| liens `ra`, `p` | 2.14c (`ma`, `kq`), 2.15 | lecture `e->ra != NULL ? *e->ra : NULL` (le lien, puis le vecteur, peuvent être nuls) ; écrire un vecteur par le lien ne compile pas ; `nf.ra`, `jo.ra`, `jo.p` restent mutables, écrits par leur module |
| bouts d'un `jc` | 2.14c (`mt`, `jc_mt_of`) | `pico8_jc_end_p`/`_ra` ne vérifient plus la sorte du bout ; `jc_mt_of` lit encore `get_mt` sur les mêmes bouts |
| `emitter_host_fa_ra` | 2.15 (émetteur au noyau) | ne lit plus que `self->fa`, `self->ra` ; `pico8_f_`, `pico8_ez`, `pico8_ei` lisent encore `cls->emitter_of` (`lien-parent`) |
| `particle.c:12` (`pico8_planet.h`) | 2.17 | inclusion sans usage depuis 2.14b ; pas une paire |
| point souris (`jc_mouse_point`) | toutes | construit par `jc:hq`, qu'aucun TU hors `test_pico8_jc` n'atteint : un chemin qui oublierait l'init du noyau n'est vu que par le grep (K-mp vert, preuve 3) |
| `test_pico8_pj.c`, `ed_fixed(ju) == 1` | la première sous-étape qui touche ce fichier | toujours là |

### Anti-tassement

**Interdit** :
- poser un lien ailleurs que dans l'init du porteur ou juste après l'init du noyau (point souris, `lq`) ; le reposer ailleurs ; l'écrire dans un TU, sauf dans l'initialiseur d'un faux type (`g_giver`, `ra_only`) ;
- un lien de champ A qui vise l'instance, un lien de champ I qui vise `def` (`jc`, `js`, `pj`) ;
- écrire `cls` hors de `pico8_struct_header_init` et des initialiseurs statiques (les deux réaffectations de `test_pico8_planet_kj_dispatch.c:91`, `:128`, qui changent la classe d'une planète déjà construite, restent) ;
- un cast autre que les trois casts de qualificatif ; un cast de type (`(pico8_vec_t **)`) pour écrire à travers un lien ; retirer `const` d'un lien ;
- garder un accesseur `get_fa`, `get_ra`, `get_p` « pour les TU » ; ajouter un accesseur ;
- toucher `kq`, `ql`, `fg`, `mt`, `ma`, `fx`, `p_`, `m_` (2.14c et après) ;
- retirer `particle.c:13` en 2.14a, ou le garder après 2.14b ; retirer une autre ligne `#include` ;
- changer une constante `*_HASH`/`FROZEN_*`, un `*.sha256`, `tests/e2e/`, `tools/` ; toucher le `Makefile` hors 2.14a.

## Implementation Chunks

Deux commits, dans l'ordre, chacun vert. **≈ 50 min chacun**, preuves comprises : ≈ 10 min pour appliquer le diff, compiler, `make test`, `make e2e`, compteur ; les jumelles en arrière-plan, ≈ 15 à 20 s l'une (a : 39 lancements, ≈ 12 min ; b : 35, ≈ 11 min) ; ≈ 15 min pour les preuves 2, 4 à 8 (`sanitize-net` ≈ 1 min, `line-coverage` des 26 e2e ≈ 2 min). Le plan donne ≈ 5 × 50 min pour 2.14 : a et b en prennent deux, comme prévu.

1. **2.14a** : appliquer le diff de a (le TU neuf compris) ; `make test`, `make e2e`, `MALLOC_PERTURB_=165 make e2e`, compteur, `--champs` ; preuves 1 à 8 (jumelles A, I, N-fa ; lignes K sauf K-init-b) ; note du plan ; commit.
2. **2.14b** : idem avec le diff de b (jumelles R, P, N-ra ; K-init-b) ; note du plan ; commit.

**Repli écrit** (si un commit déborde de 1,5 fois son budget avant d'être fait), chacun un commit vert :
- a en **a1** (init du noyau et TU, sans lien : le TU n'a que `cls` à vérifier, et le lien `fa` absent ; `accesseurs-q6` inchangé, 237) puis **a2** (`fa`, → 193) ;
- b en **b1** (`ra`, avec `emitter_host_fa_ra` et le retrait de `particle.c:13` : → 151, `lien-parent` → 110, `inclusion-mutuelle` → 16 / 40) puis **b2** (`p`, bouts d'un `jc`, `tests/test_pico8_mz.c:431` : → 123).
Comptes du repli calculés sur l'attribution ci-dessus, non mesurés ; le repli, s'il sert, est écrit dans la note « Découvert à ».

## Testing Decisions

- **Un TU neuf**, `tests/test_pico8_struct_header.c` (2.14a, complété en 2.14b) : c'est le seul filet de l'init du noyau (K-init, K-lq, K-hy, K-ke rouges par lui seul ; sans lui, `make e2e` reste à 26, même sous perturbation, parce qu'aucun lecteur ne lit `fa` sur un non-porteur). Le reste est réécriture : sept comparaisons « accesseur == membre » en épingles de lien, un faux type par TU qui pose son lien dans son initialiseur. Aucun rejeu d'`events.json` dans un TU.
- **Bon test ici** : une épingle compare le lien à l'adresse du stockage que 2.9 fixe (`h->fa == &bg->def->fa`, `h->fa == &js->fa`), ou à NULL pour un non-porteur ; jamais une valeur lue par le lien à la même valeur lue par le membre.
- **Prior art** : jumelles de la règle du parent (D.21, PRD 2.13a à 2.13e) ; essai `fa` de l'étude 2.10 (annexe B) ; relevé `const` de l'étude 2.10 (runs 11, 12).

### Preuves (parent de `a` = le commit 2.13e2c ; parent de `b` = le commit 2.14a)

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. `MALLOC_PERTURB_=165 make e2e` : **26 conformes** aux deux commits. Mesuré.

2. **Preuve `const`** (copie de `b`, `syn.sh` de l'étude 2.10, annexe B : chaque `.c` de `src/` et `tests/` en `cc -std=c99 -Wall -Wextra -Werror -Isrc -fsyntax-only`, les deux `class_coverage` sous `-DPICO8_CLASS_COVERAGE`, les trois fichiers raylib exclus) :
   - les huit membres passés en `const` compilent : **0 erreur** à `b` ;
   - un des trois membres gardés mutables passé en `const`, un à la fois : `nf.ra` (`mouse_reader.h:21`) → **3** erreurs (`ui.c:342`, `test_pico8_ie.c:546`, `test_pico8_ui.c:445`) ; `jo.p` (`planet.h:31`) → **3** (`planet.c:375`, `test_pico8_bg.c:421`, `test_pico8_mz.c:697`) ; `jo.ra` (`planet.h:32`) → **9** (`entity_geom.c:59`, `planet.c:376`, `:452`, `test_pico8_bg.c:422`, `test_pico8_lq.c:778`, `:816`, `:843`, `test_pico8_mz.c:698`, `test_pico8_qh.c:521`) — les comptes de l'étude (runs 11, 12), d'où les trois casts (numéros de ligne de `b`) ;
   - preuve négative : `*e->ra = NULL;`, `(*e->ra)->x = 0;`, `*e->fa = 0;`, `(*e->p)->y = 0;` sur un `pico8_struct_header_t *e` → `assignment of read-only location '*e->ra'`, `assignment of member 'x' in read-only object`, `assignment of read-only location '*e->fa'`, `assignment of member 'y' in read-only object` ;
   - un lien de champ A vise `def` : grep de Sortie 1 (neuf `&self->def->fa`, trois `&self->fa`).

3. **Mutations, règle du parent.** Une jumelle par porteur et par groupe (« lien laissé NULL sur un porteur »), par champ I (« lien d'un champ I qui vise `def` ») et par non-porteur essayé (« lien posé sur un non-porteur ») : au parent, la mutation équivalente sur la table (entrée `get_X` mise à `0 ? f : NULL`, qui garde la fonction nommée et évite `-Werror=unused-function` ; ou une entrée neuve) ; à l'enfant, sur la pose du lien. `make -k -j4 unit` puis `make -k e2e` ; restaurer par `git checkout -- .`. **Vert à l'enfant et rouge au parent = ÉCHEC** ; plus précisément, ÉCHEC si un TU rouge au parent n'est pas rouge à l'enfant, ou si l'e2e est < 26 au parent et 26 à l'enfant. Lignes K : l'init du noyau et ses appels, enfant seul (le parent n'a ni init ni lien), `make e2e` normal et sous `MALLOC_PERTURB_=165`.

**Jumelles de 2.14a** (parent `p`, enfant `a`) :

| # | Porteur / cible | Parent : mutation | Rouges au parent | Enfant : mutation | Rouges à l'enfant | e2e p / e | Verdict |
|---|---|---|---|---|---|---|---|
| A-bg | `bg` | `.get_fa = NULL` | `bg`, `fr`, `g_`, `ga`, `hj`, `hp`, `lq`, `mx`, `nr`, `pm`, `qb`, `ui` | lien `fa` retiré | `bg`, `fr`, `g_`, `ga`, `hj`, `hp`, `lq`, `mx`, `nr`, `pm`, `qb`, `struct_header`, `ui` | 0 / 0 | conforme (+`struct_header`) |
| A-fq | `fq` | idem | `fq` | idem | `fq` | 24 / 24 | conforme |
| A-ie | `ie` | idem | `fr`, `g_`, `ga`, `hj`, `hp`, `ie`, `lq`, `mx`, `nr`, `pm` | idem | `fr`, `g_`, `ga`, `hj`, `hp`, `ie`, `lq`, `mx`, `nr`, `pm`, `struct_header` | 0 / 0 | conforme (+`struct_header`) |
| A-iv | `iv` | idem | `iv` | idem | `iv` | 7 / 7 | conforme |
| A-mh | `mh` | idem | `mh`, `replay_from_title` | idem | `mh`, `replay_from_title` | 15 / 15 | conforme |
| A-mz | `mz` | idem | `mz` | idem | `mz` | 8 / 8 | conforme |
| A-ui | `ui` | idem | `fv`, `hp`, `lq`, `mh` | idem | `fv`, `hp`, `lq`, `mh` | 0 / 0 | conforme |
| A-jc | `jc` (I) | idem | aucun | idem | aucun | 17 / 17 | conforme |
| A-pj | `pj` (I) | idem | `fr`, `g_`, `ga`, `hj`, `hp`, `lq`, `mx`, `nr`, `pm` | idem | `fr`, `g_`, `ga`, `hj`, `hp`, `lq`, `mx`, `nr`, `pm` | 0 / 0 | conforme |
| A-js | `js` (I) | idem | `class_coverage`, `fr`, `fv`, `g_`, `ga`, `hj`, `hp`, `lq`, `mx`, `nr`, `pm`, `replay_from_title`, `ui` | idem | `class_coverage`, `fr`, `fv`, `g_`, `ga`, `hj`, `hp`, `lq`, `mx`, `nr`, `pm`, `replay_from_title`, `struct_header`, `ui` | 0 / 0 | conforme (+`struct_header`) |
| A-g_ | `g_` et `fv` | entrée NULL dans les deux tables | `fv`, `g_` | lien retiré de `pico8_g__struct_init` | `fv`, `g_` | 7 / 7 | conforme |
| A-jo | arbre `jo` (dix sortes) | entrée NULL dans les dix tables | `fr`, `g_`, `ga`, `hj`, `hp`, `lq`, `mx`, `nr`, `planet_kj_dispatch`, `pm`, `ui` | lien retiré de `pico8_jo_struct_init` | `fr`, `g_`, `ga`, `hj`, `hp`, `lq`, `mx`, `nr`, `planet_kj_dispatch`, `pm`, `ui` | 0 / 0 | conforme |
| I-jc | `jc` | `jc_get_fa` lit `def->fa` | aucun | lien `&self->def->fa` | aucun | 25 / 25 | conforme |
| I-pj | `pj` | `pj_get_fa` lit `def->fa` | aucun | lien `&self->def->fa` | aucun | 24 / 24 | conforme |
| I-js | `js` | `js_get_fa` lit `def->fa` | `lq` | lien `&self->def->fa` | `lq`, `struct_header` | 9 / 9 | conforme (+`struct_header`) |
| N-nf-fa | `nf` reçoit `fa` = 15 | entrée `get_fa` neuve | aucun | lien vers une constante 15 | `struct_header` | 26 / 26 | conforme (+`struct_header`) |
| N-qb-fa | `qb` reçoit `fa` = 15 | idem | aucun | idem | aucun | 26 / 26 | conforme |

**Jumelles de 2.14b** (parent `a`, enfant `b`) :

| # | Porteur / cible | Parent : mutation | Rouges au parent | Enfant : mutation | Rouges à l'enfant | e2e p / e | Verdict |
|---|---|---|---|---|---|---|---|
| R-ie | `ie` | `.get_ra = NULL` | `fr`, `g_`, `ga`, `hj`, `hp`, `ie`, `lq`, `mx`, `nr`, `pm` | lien `ra` retiré | `fr`, `g_`, `ga`, `hj`, `hp`, `ie`, `lq`, `mx`, `nr`, `pm`, `struct_header` | 0 / 0 | conforme (+`struct_header`) |
| R-nf | `nf` | idem | `nf` | idem | `nf`, `struct_header` | 26 / 26 | conforme (+`struct_header`) |
| R-qb | `qb` | idem | `qb` | idem | `qb` | 26 / 26 | conforme |
| R-iv | `iv` | idem | `iv` | idem | `iv` | 7 / 7 | conforme |
| R-js | `js` | idem | `fr`, `fv`, `g_`, `ga`, `hj`, `hp`, `js`, `lq`, `mx`, `nr`, `pm`, `replay_from_title`, `ui` | idem | `fr`, `fv`, `g_`, `ga`, `hj`, `hp`, `js`, `lq`, `mx`, `nr`, `pm`, `replay_from_title`, `struct_header`, `ui` | 0 / 0 | conforme (+`struct_header`) |
| R-mz | `mz` | idem | `mz` | idem | `mz` | 8 / 8 | conforme |
| R-lq | `lq` | idem | `lq` | idem | `lq`, `struct_header` | 26 / 26 | conforme (+`struct_header`) |
| R-mp | point souris de `jc:hq` | idem | `jc` | idem | `jc` | 26 / 17 | conforme |
| R-g_ | `g_` et `fv` | entrée NULL dans les deux tables | `fv`, `g_` | lien retiré de `pico8_g__struct_init` | `bg`, `class_coverage`, `fv`, `g_`, `headless_capture`, `line_coverage`, `replay_from_title` | 8 / 0 | conforme (+`bg`, `class_coverage`, `headless_capture`, `line_coverage`, `replay_from_title`) |
| R-jo | arbre `jo` | entrée NULL dans les dix tables | `fr`, `g_`, `ga`, `hj`, `hp`, `hy`, `jc`, `jo`, `lq`, `mx`, `mz`, `nr`, `planet_kj_dispatch`, `pm`, `ui` | lien retiré de `pico8_jo_struct_init` | `fr`, `g_`, `ga`, `hj`, `hp`, `hy`, `jc`, `jo`, `lq`, `mx`, `mz`, `nr`, `planet_kj_dispatch`, `pm`, `ui` | 0 / 0 | conforme |
| P-qb | `qb` | `.get_p = NULL` | `qb` | lien `p` retiré | `qb` | 26 / 26 | conforme |
| P-mz | `mz` | idem | `mz` | idem | `mz` | 26 / 26 | conforme |
| P-mp | point souris | idem | `jc` | idem | `jc` | 26 / 17 | conforme |
| P-g_ | `g_` et `fv` | entrée NULL dans les deux tables | `g_` | lien retiré | `g_` | 8 / 8 | conforme |
| P-jo | arbre `jo` | entrée NULL dans les dix tables | `bg`, `fr`, `g_`, `ga`, `hj`, `hp`, `jc`, `jo`, `js`, `lq`, `mx`, `mz`, `nr`, `pm`, `qh`, `trade`, `ui` | lien retiré | `bg`, `fr`, `g_`, `ga`, `hj`, `hp`, `jc`, `jo`, `js`, `lq`, `mx`, `mz`, `nr`, `pm`, `qh`, `trade`, `ui` | 0 / 0 | conforme |
| N-bg-ra | `bg` reçoit `ra` = (64,64) | entrée `get_ra` neuve | aucun | lien vers un vecteur constant | `struct_header` | 26 / 26 | conforme (+`struct_header`) |
| N-fq-ra | `fq` reçoit `ra` = (64,64) | idem | aucun | idem | aucun | 26 / 26 | conforme |

**Lignes K** (enfant seul ; e2e normal, puis sous `MALLOC_PERTURB_=165`) :

| # | Mutation | Commit | Rouges | e2e | e2e perturbé |
|---|---|---|---|---|---|
| K-init | corps de `pico8_struct_header_init` réduit à `self->cls = cls;` | `a` | `struct_header` | 26/26 | 26/26 |
| K-lq | `pico8_lq_struct_alloc` pose `cls` sans l'init | `a` | `struct_header` | 26/26 | 26/26 |
| K-hy | `pico8_hy_struct_new` pose `cls` sans l'init | `a` | `struct_header` | 26/26 | 26/26 |
| K-mp | le point souris pose `cls` sans l'init | `a` | aucun | 26/26 | 26/26 |
| K-ke | `pico8_ke_struct_init` pose `cls` sans l'init | `a` | `struct_header` | 26/26 | 26/26 |
| K-init-b | init réduite à `self->cls = cls; self->fa = NULL;` (oublie `ra`, `p`) | `b` | `bg`, `class_coverage`, `fv`, `ga`, `headless_capture`, `hp`, `js`, `line_coverage`, `lq`, `mh`, `replay_from_title`, `struct_header` | 0/26 | 0/26 |

   Lecture : **aucun ÉCHEC** (34 jumelles, 68 lancements ; 6 lignes K). Chaque jumelle a au parent un sous-ensemble des rouges de l'enfant ; `struct_header` s'ajoute à l'enfant pour les types qu'il construit. **R-g_**, **R-mp**, **P-mp** : l'enfant rougit plus que le parent (e2e 8 → 0, 26 → 17) parce que les lectures croisées (`emitter_host_fa_ra`, `pico8_jc_end_p`/`_ra`) lisaient le membre au parent et lisent le lien à l'enfant : un filet gagné, pas perdu. **A-jc**, **I-jc**, **I-pj** : aucun TU rouge, aux deux bouts ; seul l'e2e voit la valeur (17, 25, 24). **N-qb-fa**, **N-fq-ra** : verts aux deux bouts, e2e 26 : aucun lecteur n'observe l'absence de `fa` sur `qb` ni de `ra` sur `fq` ; `N-nf-fa`, `N-bg-ra` ne sont vus que par le TU neuf, qui construit `nf` et `bg`. **K-mp** : vert, même sous perturbation : le point souris n'est pas construit par le TU neuf, et personne ne lit `fa` sur lui ; son passage par l'init n'est prouvé que par le grep de Sortie 2. **K-init** à `a` : seul le TU neuf rougit, l'e2e reste à 26 **même sous perturbation** (aucun lecteur ne lit `fa` d'un non-porteur) ; **K-init-b** à `b` : douze TU et l'e2e à 0, **même sans perturbation** (un bloc réutilisé par `malloc` porte déjà des liens `ra`/`p` sales, que `dc` et `pico8_gd` suivent).

4. **Chemin touché** : `make line-coverage EVENTS="<les 26 e2e>"` sur `b` (6148 lignes) : l'init du noyau (`struct.h:94`) et ses appels (`ke.c:36`, `economy.c:33`, `layout.c:300`, `:408`, `research.c:137`, `research_icon.c:55`, `slipway.c:107`, `trade.c:32`) ; les poses `fa` (`bg.c:65`, `msgbox.c:106`, `:243`, `planet.c:123`, `probe.c:426`, `research_icon.c:231`, `score_panel.c:290`, `slipway.c:373`, `ui.c:95`, `ui_button.c:126`, `:343`, `wormhole.c:228`) et `ra`/`p` (`layout.c:301`, `mouse_reader.c:44`, `msgbox.c:244`, `planet.c:125`, `:126`, `probe.c:427`, `:428`, `research_icon.c:232`, `scroll.c:39`, `:40`, `slipway.c:108`, `:109`, `ui_button.c:127`, `wormhole.c:229`, `:230`) ; les lecteurs (`entity.c:202`, `:205`, `:228`, `entity_geom.c:17`, `:23`, `:29`, `ui.c:285`, `:286`, `wormhole_display.c:14`, `:15`, `slipway.c:34`, `:40`, `particle.c:17`, `:19`, `:20`) : **tous exécutés** (numéros de ligne de `b`).

5. **Valeurs observables, parent contre enfant** : un programme d'essai (`obs.c`, hors dépôt), lié aux objets de `src/` (sans `main`), boote le cart (`pico8_cart_load`, `pico8_jl`), crée une instance fraîche de 29 types (`bg`, `ui`, `js`, `fq`, `nf`, `qb`, `mh`, `iv`, `ie`, `pj`, `mz`, `g_`, `fv`, les dix sortes, `lq`, `jb`, `hp`, `hy`, `trade`, `jc`), puis le point souris par `pico8_jc_hq`, et imprime ce que lit le code générique : `fa` (présent ou non, valeur), `ra` et `p` (absent, nil ou le vecteur), et les quatre bouts d'un `jc` par `pico8_jc_end_p`/`_ra` — **32 lignes**. Compilé `-DV=0` (accesseurs) sur `p`, `-DV=1` (lien `fa`, accesseurs `ra`/`p`) sur `a`, `-DV=2` (trois liens) sur `b` : sorties **identiques**, et identiques sous `MALLOC_PERTURB_=165` à `a` et `b`. (`-DV=2` sur `a` ne compile pas : `ra`, `p` absents du noyau.)

6. **Le TU neuf mord** : à `a`, K-init/K-lq/K-hy/K-ke rouges ; lancé avec `PICO8_MALLOC_FILLS=1` (sans relance, donc sans perturbation), il s'arrête sur `assert_malloc_fills` (« malloc must fill its block, or this test proves nothing ») : il ne peut pas passer vert sans perturbation.

7. `class-coverage-net` : **oui**, vert aux deux commits (`couverture: OK`, sortie égale à `tools/class_coverage_net_expected.txt`). `sanitize-net` : **oui** (le noyau grandit : 32 puis 48 octets), `sanitize: 26 propre(s)` aux deux. Mesurés sur une première version des commits (`4765383`, `97f27a6`) dont `src/` ne diffère des commits finaux que par un commentaire de `slipway.h` (Runs 21).

8. **Graphe d'usage** (`usegraph.py`, annexe A du PRD 2.11, texte inchangé) : 20 à `p` et `a`, **19** à `b` ; seule la paire `pico8_particle` ↔ `pico8_probe` sort, aucune n'entre.

### Greps de Sortie

À chaque commit (`a`, puis `b`) :
1. `grep -rn 'header\.fa = ' src` → **12** lignes : 9 `&self->def->fa;` (`bg`, `fq`, `ie`, `iv`, `mh`, `mz`, `ui`, `probe.c` pour `g_`/`fv`, `planet.c` pour l'arbre `jo`), 3 `&self->fa;` (`jc`, `js`, `pj`).
2. `grep -rnE '(\.|->)cls *= [^=]' src tests | grep -v '{'` → exactement `tests/test_pico8_planet_kj_dispatch.c:91` et `:128` ; `grep -rn 'pico8_struct_header_init(' src | wc -l` → **9** (la définition, `ke.c`, les sept constructeurs hors `ke`).
3. `grep -rn 'get_fa' src tests` → vide ; à `b`, `grep -rnE '\bget_(ra|p)\b' src tests` → vide (le seul `get_p` restant du motif large, `test_composition_residue.c:156`, est `budget_p`, hors motif `\b`).
4. `grep -c '(const pico8_vec_t \*const \*)' src/*.c` → à `b`, exactement `mouse_reader.c` 1 et `planet.c` 2.
5. `grep -rnE 'header\.(ra|p) = ' src` → à `b`, **15** lignes (10 `ra`, 5 `p`) : `layout.c`, `mouse_reader.c`, `msgbox.c`, `research_icon.c`, `ui_button.c` (`ra`) ; `planet.c`, `probe.c`, `scroll.c`, `wormhole.c`, `slipway.c` (point souris) (`ra` et `p`).
6. À `b` : `grep -n 'pico8_probe.h\|pico8_g__of\|pico8_jo_is_family' src/pico8_particle.c` → vide ; `grep -c jc_end_point src/pico8_slipway.c` → 0.
7. `make composition-residue` → `accesseurs-q6 193 32`, pied **360 / 63** à `a` ; `accesseurs-q6 123 21`, `lien-parent 110 27`, `inclusion-mutuelle 16 15`, pied **286 / 54** à `b` ; `--check inclusion-mutuelle | tail -1` → 43 puis **40** ; `--champs` → `243 ; 15 ; 20`, puis `245 ; 15 ; 20`.
8. `git diff --stat <parent>` : les fichiers de la section « Modules » et `docs/plan-3.md`, rien d'autre ; le `Makefile` à `a` seulement ; ni `tools/`, ni `tests/e2e/`, ni `test_pico8_pj.c`.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable (`prd214ab/copie`).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout 7a5c47c` ; les quatre diffs du PRD 2.13e extraits (lignes 146-1374, 1380-2641, 2647-3628, 3634-4158), `git apply --check`/`git apply`, `git rm` du test ponctuel avant le quatrième, quatre commits | `p` | 0 | `348d510`, `6d5d5a7`, `ea878f0`, `8195d5e` | 3 s |
| 2 | `timeout 900 make -j4 test` ; `timeout 600 make e2e` ; compteur, `--champs`, `--check inclusion-mutuelle`, casts, tests de classe, garde | `p` | 0 | vert ; 26 ; 404 / 63 ; 242 ; 43 ; 530 / 708 ; 8 catégories | 10 s ; 1 s |
| 3 | `--check accesseurs-q6`, attribution par sous-étape (motif de l'annexe A de l'étude 2.10) | `p` | 0 | 44 / 70 / 43 / 48 / 32 = 237 | < 1 s |
| 4 | script `a` (lien `fa`, init du noyau, lecteurs, TU) ; `syn.sh` | `a` | 1 | une erreur, `test_pico8_mz.c:303` (variable `fa` inutile) ; corrigée, 0 | 1 s |
| 5 | TU neuf, première forme (`mallopt(M_PERTURB)`) ; `make -j4 test`, `make e2e`, `MALLOC_PERTURB_=165 make e2e`, compteur | `a` | 0 | vert ; 26 ; 26 ; 193 / 32 ; 243 ; 518 / 693 ; 32 octets | 10 s |
| 6 | `class-coverage-net` ; `sanitize-net` ; commit `4765383` | `a` | 0 | `couverture: OK` ; `26 propre(s)` | ≈ 1 min |
| 7 | script `b` ; `syn.sh` ; TU neuf complété | `b` | 0 | 0 erreur ; compteur 123 / 21, `lien-parent` 110, `inclusion-mutuelle` 16 / 15 | 1 s |
| 8 | `make -j4 test`, `make e2e`, `MALLOC_PERTURB_=165 make e2e`, `--check inclusion-mutuelle`, casts, taille | `b` | 0 | vert ; 26 ; 26 ; 40 ; 500 / 677 (+4 casts de qualificatif) ; 48 octets | 12 s |
| 9 | `particle.c:12` retirée pour voir (puis remise) | `b` | 0 | compile : aucun nom de `planet.h` employé (G6) | < 1 s |
| 10 | `usegraph.py` sur `p`, `a`, `b` | les trois | 0 | 20, 20, 19 | < 2 s |
| 11 | `class-coverage-net`, `sanitize-net` ; commit `97f27a6` | `b` | 0 | `couverture: OK` ; `26 propre(s)` | ≈ 1 min |
| 12 | `obs.c` sur `p` (`-DV=0`), `a` (`-DV=1`), `b` (`-DV=2`) ; `cmp` ; puis `a`, `b` sous `MALLOC_PERTURB_=165` | les trois | 0 | 32 lignes, identiques ; identiques sous perturbation ; `-DV=2` sur `a` refusé par le compilateur | < 2 min |
| 13 | jumelles et lignes K (`muts.py` + `mut.sh`, 74 lancements) | `p`, `a`, `b` | 0 | six lancements « parent » refusés par un défaut du script (clé `fa` au lieu de `get_fa` pour les tables `g_`/`jo`) | ≈ 25 min |
| 14 | les six refaits (A-g_, A-jo, R-g_, R-jo, P-g_, P-jo au parent) | `p`, `a` | 0 | tableaux ci-dessus | ≈ 2 min |
| 15 | TU neuf réécrit sans `<malloc.h>` (le `Makefile` compile aussi sur macOS : `mallopt` n'y existe pas) : relance par `execv` sous `MALLOC_PERTURB_=90`/`MallocScribble=1` ; lecture `volatile` (`-Werror=maybe-uninitialized` sur la lecture d'un bloc frais) ; mutation K-init ; lancement sans relance | `a` | 0 / 134 / 134 | vert ; rouge `assert_links` ; rouge `assert_malloc_fills` | 1 min |
| 16 | `a` refait (`cd485a8`), diff de `b` réappliqué (`97f27a6` → `c64e4f6`) ; `make test`, `make e2e`, `MALLOC_PERTURB_=165 make e2e`, compteur | `a`, `b` | 0 | vert ; 26 ; 26 ; 360 / 63, 286 / 54 | 1 min |
| 17 | lignes K et jumelles qui touchent le TU neuf, rejouées sur `cd485a8`/`c64e4f6` (16 lancements) | `a`, `b` | 0 | mêmes rouges qu'au run 13 | ≈ 6 min |
| 18 | preuve `const` : `nf.ra`, `jo.p`, `jo.ra` en `const`, un à la fois ; preuve négative (`neg.c`) | `b` | 1 | 3, 3, 9 erreurs ; `read-only` ×4 | < 1 min |
| 19 | `make line-coverage EVENTS=<26 e2e>` ; lignes de la preuve 4 | `b` | 0 | 6148 lignes ; toutes présentes | ≈ 2 min |
| 20 | commentaire de `slipway.h:80-82` rendu vrai (« Asserts on any other end » parti) ; `make test`, `make e2e` ; `commit --amend` → `ca7260b` | `b` | 0 | vert ; 26 | 12 s |
| 21 | greps de Sortie ; `git diff 8195d5e cd485a8`, `git diff cd485a8 ca7260b` ; `git apply --check`/`git apply` des deux sur `8195d5e`, `git diff --cached ca7260b` | les trois | 0 | ci-dessus ; diff vide | < 10 s |
| 22 | `rm -rf` de la copie | — | 0 | supprimée | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas le commit 2.13e2c (ou un descendant qui ne touche que `docs/`), ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si 2.14a et 2.14b ne sont pas deux commits (hors repli écrit), dans cet ordre, chacun vert (`make test`, `make e2e` 26 conformes, et 26 sous `MALLOC_PERTURB_=165`), ou si un `*.sha256`, `tests/e2e/`, `tools/` ou une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si une jumelle a un TU rouge au parent qui ne l'est pas à l'enfant, ou un e2e < 26 au parent et 26 à l'enfant ; si une ligne K (hors K-mp) est verte ; si le TU neuf passe sans perturbation (preuve 6).
- `FAILED:` si un constructeur écrit `cls` hors de `pico8_struct_header_init` (grep de Sortie 2), si un lien est posé hors de l'init de son porteur, si un lien de champ A ne vise pas `def` ou un lien de champ I vise `def`.
- `FAILED:` si un accesseur `get_fa`, `get_ra`, `get_p` reste, ou si un autre groupe Q6 (`kq`, `ql`, `fg`, `mt`, `fx`, `p_`, `m_`) ou le lien `ma` est touché.
- `FAILED:` si un cast autre que les trois casts de qualificatif est ajouté, si un lien n'est pas `const`, ou si l'un des huit membres prévus reste mutable.
- `FAILED:` si `accesseurs-q6` n'est pas 193 puis 123, `lien-parent` 111 puis 110, `inclusion-mutuelle` 17 / 16 (43) puis 16 / 15 (40), le pied 360 / 63 puis 286 / 54, `--champs` `243 ; 15 ; 20` puis `245 ; 15 ; 20` ; si `défauts` n'est plus à 0 et gardé ; si le graphe d'usage n'est pas 19 à `b`.
- `FAILED:` si `particle.c:13` n'est pas retirée à `b`, ou si une autre ligne `#include` l'est.
- `FAILED:` si `class-coverage-net` ou `sanitize-net` est rouge ; si les valeurs observables (preuve 5) diffèrent.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si un message de commit omet : les liens posés et leur cible (`def` ou instance), l'init du noyau et ses appelants, les accesseurs retirés, la table des jumelles avec leur résultat au parent, les lignes K, la preuve `const`, les comptes, le tableau « Runs ».
- `FAILED:` s'il manque une note « Découvert à 2.14a » ou « Découvert à 2.14b » au bout des lignes 2.14a/2.14b du plan, ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- `kq`, `ql`, `fg`, `mt`, le lien `ma` (2.14c) ; `fx`, `p_` (2.14d1) ; `m_` (2.14d2) ; `has_fx`, `has_p_`, `has_m_`.
- Les liens `emitter`/`probe` et le masque `lineage` (2.15) ; `pico8_g__of`, `pico8_jo_is_family` hors des deux lectures croisées nommées.
- `particle.c:12` et les autres paires d'inclusion (2.17).
- `test_pico8_pj.c` (réserve reportée) ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. L'init du noyau dans `pico8_struct.h`, `static inline`, par un littéral composé** : `pico8_struct.h` est le noyau et déjà visible des sept constructeurs hors `ke` (`ke.h` ne l'est pas de `economy.c`, `layout.c`, `research.c`, `trade.c`) ; le littéral met à zéro les liens que 2.14c à 2.15 ajouteront, sans qu'aucune sous-étape ne doive se souvenir de l'init. `me`/`t` à nil/0 pour les non-`ke` : ils n'étaient jamais lus, ils ne sont plus indéterminés.
- **G2. Le TU se relance lui-même sous `MALLOC_PERTURB_`** plutôt que `mallopt(M_PERTURB)` (glibc seule : `<malloc.h>` manque sur macOS, où le `Makefile` compile aussi `make test`) ou un crochet sur `malloc` (`--wrap` est propre à GNU ld ; une interposition demanderait `free`/`realloc`/`calloc`). `MallocScribble=1` est posé pour macOS (bloc rempli de `0xaa`) : **non essayé** sur macOS ; `assert_malloc_fills` y arrêterait le TU si la variable n'agissait pas, jamais un vert à vide. Les deux perturbations remplissent tout le run (boot compris), ce qui est plus fort que le seul bloc du non-porteur que demande le plan.
- **G3. Contrat des trois casts** en anglais, comme tous les commentaires du dépôt : « Read-only: this module writes through its member. » (`nf`), « Read-only: the planet modules write through their members. » (`jo`, deux casts sous un commentaire).
- **G4. `jc_end_point` et son `assert` partent** (étude 2.10 : « plus de test ni de `jc_end_point` ») : un bout sans lien lit le vecteur nul, comme un bout NULL ; le commentaire de `slipway.h` est rendu vrai. Aucune jumelle n'a perdu de rouge.
- **G5. `emitter_host_fa_ra` garde un `assert`**, sur la présence des liens (l'ancien portait sur la famille) : c'est ce que la fonction suit.
- **G6. `particle.c:12` (`pico8_planet.h`) reste** : sans usage depuis 2.14b (essai : la retirer compile), mais `particle` ↔ `planet` n'est pas une paire, le plan ne la nomme pas, et 2.17 relit les inclusions par l'usage.
- **G7. Jumelles « non-porteur »** : le plan demande « là où un lecteur observe l'absence » ; pour `fa`, `ra`, `p`, les quatre essais (`nf`, `qb` pour `fa` ; `bg`, `fq` pour `ra`) montrent qu'aucun lecteur de `src/` ne l'observe sur ces types (e2e 26 aux deux bouts). Le filet est le TU neuf, qui épingle NULL sur les non-porteurs qu'il construit.

### Surprises de mesure, pour `plan-pilot`

1. **Sans le TU neuf, rien ne voit l'init du noyau à 2.14a** : un `cls` seul (le code d'avant) laisse `make e2e` à 26 même sous perturbation, parce que seul un non-porteur aurait un `fa` indéterminé et que personne ne le lit. À 2.14b, le même oubli casse tout (e2e à 0 sans perturbation) : les liens `ra`/`p` des non-porteurs sont suivis par `dc` et `pico8_gd`.
2. **Le point souris** (`jc:hq`) n'est vu par aucun TU pour son passage par l'init (K-mp vert) : grep seul.
3. **`mallopt` aurait cassé `make test` sur macOS** : le `Makefile` a une branche `Darwin` ; d'où la relance par `execv`.
4. Le relevé `const` de l'étude (3 / 3 / 9) se retrouve à l'identique après 2.13 ; les numéros de ligne ont changé.

### Questions pour l'humain

Aucune.
