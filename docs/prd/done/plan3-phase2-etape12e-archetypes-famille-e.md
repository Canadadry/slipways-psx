---
title: "Plan 3, étapes 2.12e1 et 2.12e2 : `kw_own` sort de l'archétype planète, puis les valeurs de `hj`, `pm`, `mx`, `ga`, `nr`, `fr` (TU 231 → 350 → 438 ; `défauts` 669, pied inchangé, `--champs` 19 → 20) — deux commits, jeu identique"
description: "2.12e1 retire `kw_own` de `pico8_jo_def_t`, des quatre valeurs de 2.12d et du test ponctuel (constat du vérificateur de 2.12d : c'est un E, pas un I), puis ajoute `pico8_hj_def`, `pico8_pm_def`, `pico8_mx_def` et 120 comparaisons (total 350). 2.12e2 ajoute `pico8_ga_def`, `pico8_nr_def`, `pico8_fr_def` et 88 comparaisons (total 438). Les 26 archétypes de l'étude existent alors. Essayé sur copie : vert, 26 e2e, mutations et jumelles au parent sans ÉCHEC."
status: ready
---

## Problem Statement

**Écrit sur `f9c0297`** (branche `claude/plan-pilot-3-7zdft0`, 2.12d `558b87f` commitée et notée). Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier. Les essais ont été faits sur une copie jetable (scratchpad `prd212e/`, supprimée depuis). Elle a d'abord été prise à `a7c0819`, avec 2.12d appliquée d'après son PRD, puis remise sur `f9c0297` quand 2.12d y est arrivée. Le texte de 2.12d appliqué est identique à `558b87f` (`git diff` vide sur `src/` et `tests/`). Notation : `p` = `f9c0297` ; `e1`, `e2` = le texte de « Ce qui change » appliqué et commité dans la copie (`673517b`, `0c12433` dans la copie).

Vu du mainteneur du port : 2.12d a créé le type planète unique `pico8_jo_def_t` et les valeurs de `jo`, `qh`, `nl`, `os`. 2.12e finit 2.12 : il n'ajoute que des valeurs (étude 2.9 §4), celles des six dernières sortes. Il corrige aussi une erreur de classement de l'étude, trouvée par le vérificateur de 2.12d : **`kw_own` n'est un champ d'archétype pour aucune sorte**.

### Texte du plan, recopié (`docs/plan-3.md` à `f9c0297`)

> - 2.12 — archétypes et **test ponctuel de la règle 3**, selon l'étude 2.9 […]. Il compare champ par champ les champs A et I d'un `pico8_X_new(NULL)` frais à `pico8_X_def`, groupes de Q7 compris, avec les trois exceptions de l'étude, §3. Aucune lecture ne bascule encore, et aucun `def` n'entre dans les instances. Compte : `défauts` **669** à chaque sous-étape. Preuves : preuve compilateur `const` de chaque champ d'archétype […] ; test ponctuel vert, rouge sous les deux mutations de la famille […]. `class-coverage-net` : non. `sanitize-net` : non. […]
>   - 2.12e1 — `hj`, `pm`, `mx` : 123 comparaisons, groupes émetteur et sonde.
>   - 2.12e2 — `ga`, `nr`, `fr` : 91 comparaisons, groupes émetteur (`ga`) et sonde.

Note « Découvert à 2.12d » (même fichier) : « Report, essayé sur copie […] : **2.12e1 : 120** […], **2.12e2 : 89** […] ; `ga` n'a pas de `hp` et garde son `kw_own` comparé. Pour 2.13d : `pico8_jo_struct_init` copie `def->kw_own` […] ». Plan, 2.13 : « Les objets statiques `g_jo_kw_shared`, `g_ga_by_shared` […] restent posés par l'init. »

Règles communes : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte (`*_HASH`, `FROZEN_*`) changée.

### `kw_own` : E, pas I (constat du vérificateur de 2.12d, décision du pilote)

`kw_own` est le stockage C de la table que `pico8_jo_lz` crée (`planet.c:325-328`). Le jeu ne le lit que par `kw`, après que `lz` a écrit ses trois membres. Sa valeur initiale n'est jamais lue par le jeu, même pour `jo` et `ga` : sans `hp`, `lz` ne tourne pas, et `kw` pointe `g_jo_kw_shared`. Grep sur `e2` : `kw_own` n'est lu hors de `planet.c` que par le TU `archetype_chain` (`assert_lz_ran`) et par `test_pico8_jo_spec_defaults.c:74` et `:230` (défaut recopié de `pico8_jo_start.kw`, que 2.13 réécrit). Mesure : `lz` tourne sur `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `nr`, `fr`, et pas sur `jo` ni `ga`.

Conséquences, que ce PRD place **dans 2.12e1** (deux commits, comme demandé) :
- `kw_own` sort de `pico8_jo_def_t` (33 → **32** membres) et des quatre valeurs de 2.12d ; `same_kw` et la comparaison `jo.kw_own` sortent du TU : **231 → 230** ;
- aucune des six sortes de la famille e ne compare `kw_own`. Les totaux deviennent **2.12e1 : +120** (inchangé : `hj`, `pm`, `mx` ne le comparaient pas) et **2.12e2 : +88** (`ga` 29 au lieu de 30) ; TU **350** puis **438** ;
- `--champs` « 0 hors constructeur » **19 → 20** : `pico8_jo_t.kw_own` repasse de 5 / 4 à 1 / 0 écritures ;
- l'étude 2.9 est corrigée (annexe A : `jo.kw_own` E) et la note du plan le dit.

### Ce que 2.12e place

| Élément | Sous-étape | Pourquoi |
|---|---|---|
| `kw_own` retiré du type, des valeurs `jo`/`qh`/`nl`/`os`, du TU ; étude corrigée | **2.12e1** | constat du vérificateur de 2.12d ; il faut le faire avant d'ajouter six valeurs qui le porteraient |
| valeurs `pico8_hj_def`, `pico8_pm_def`, `pico8_mx_def` ; `same_emitter_part`, `test_hj`, `test_pm`, `test_mx` | **2.12e1** | plan |
| valeurs `pico8_ga_def`, `pico8_nr_def`, `pico8_fr_def` ; `test_ga`, `test_nr`, `test_fr` | **2.12e2** | plan |
| `jo.kw` (`&g_jo_kw_shared`), `jo.by` de `ga` (`&g_ga_by_shared`) | **jamais** dans l'archétype | objets `static` de module (étude §2) ; posés par l'init jusqu'au bout (plan 2.13) ; comparés en `assert` nus |
| membre `def` de `pico8_jo_t`, lecture de l'archétype | 2.13d | interdit en 2.12 |

### Question ouverte chez l'humain (Q-2.10, liens du noyau)

**2.12e n'en dépend pas** : aucun lien du noyau, aucun membre d'instance.

### Travail en parallèle

| Fichier | 2.12e1 | 2.12e2 | Recouvrement |
|---|---|---|---|
| `src/pico8_game_data.[ch]`, `tests/test_pico8_archetype_chain.c` | oui | oui | e2 s'ancre sur ce que e1 pose : **e1 d'abord** |
| `docs/plan-3.md` | ligne 2.12e1 | ligne 2.12e2 | `git add -p` (seul le bloc de la sous-étape), jamais `git commit -a`. Le pilote écrit peut-être en même temps une note sur la ligne 2.12d : **ne pas toucher la ligne 2.12d** (la note de 2.12e1 porte la correction) |
| `docs/etude-2-9-archetypes.md` | oui (correction `kw_own`) | non | aucun autre PRD connu ne l'écrit |
| `tools/composition_residue.sh` | lu | lu | mêmes mesures au parent et au commit |
| `tmp/` | `make test`, mutations | idem | **jamais deux `make` dans le même arbre en même temps** |

### État de départ attendu (`f9c0297` ou un descendant qui ne touche que `docs/`)

| Commande | Résultat mesuré à `f9c0297` |
|---|---|
| `make composition-residue` | `défauts 669 52`, `inclusion-mutuelle 17 16` ; pied **1073 / 85** |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 282 ; 0 écriture : 15 ; 0 hors constructeur : 19` |
| `sh tools/composition_residue.sh --check inclusion-mutuelle \| tail -1` | `résidu gardé (inclusion-mutuelle) : 43 ligne(s)` |
| `wc -l src/pico8_game_data.h` | 745 |
| `grep -c 'g_compared == 231' tests/test_pico8_archetype_chain.c` | 1 |
| `grep -c 'kw_own' src/pico8_game_data.c` | 4 |
| `grep -rnwE 'pico8_(hj\|pm\|mx\|ga\|nr\|fr)_def' src tests` | vide |
| `make test` ; `make e2e` | vert ; 26 conformes |

Un départ autre : arrêt, rapporté.

## Solution

Deux commits :

- **2.12e1** : `kw_own` sort de `pico8_jo_def_t` (32 membres), des quatre valeurs et du TU (`same_kw`, `jo.kw_own` : 231 → 230), commentaires ajustés ; trois valeurs `const` non `static`, `pico8_hj_def`, `pico8_pm_def`, `pico8_mx_def`, déclarées `extern` ; TU : une inclusion (`planet_family_trade.h`), `same_emitter_part` (11 comparaisons), `test_hj` (39), `test_pm` (41), `test_mx` (40), total **350** ; `docs/etude-2-9-archetypes.md` corrigée ; plan, ligne 2.12e1.
- **2.12e2** : `pico8_ga_def`, `pico8_nr_def`, `pico8_fr_def` ; `test_ga` (29), `test_nr` (29), `test_fr` (30), total **438** ; plan, ligne 2.12e2.

Aucun membre neuf dans le type (seul `kw_own` en sort). Aucun `def` dans une instance. Aucun `_new`, init ou `_start` ne change. Aucune lecture ne bascule. Pas de `Makefile`.

## User Stories

1. En tant qu'auteur de 2.13d/e, je veux les dix archétypes planète prouvés égaux à la chaîne, afin que `pico8_jo_struct_init` copie le `def` de la classe concrète sans changer une valeur.
2. En tant que gardien de la règle 1, je veux que l'archétype ne porte que des champs A et I : `kw_own`, état que `lz` écrit, n'y est pas.
3. En tant que gardien de la décision A, je veux **un seul** type pour les dix sortes, que 2.12e ne fait que remplir.
4. En tant que gardien de la règle 3, je veux les groupes émetteur (`hj`, `pm`, `mx`, `ga`) et sonde (`hj`, `pm`, `mx`, `nr`, `fr`) de Q7 comparés, les exceptions hors compte : `kw`, `kw_own`, `by`, `et`, `qy`.
5. En tant que gardien du test ponctuel, je veux les totaux assertés 350 puis 438, avec l'écart au plan (123/91 → 120/88) expliqué.
6. En tant que gardien du compteur, je veux `défauts` 669, `inclusion-mutuelle` 17 / 16, le pied inchangé et `--champs` 19 → 20 expliqué.
7. En tant que gardien du jeu, je veux 26 e2e conformes et aucun objet de `src/` changé hors `game_data`.
8. En tant que vérificateur, je veux des mutations côté archétype, des jumelles côté chaîne rejouées au parent, et la liste des 26 archétypes contrôlée contre l'étude.
9. En tant qu'auteur de 2.13, je veux savoir quels littéraux n'ont que le test ponctuel pour filet.

## Implementation Decisions

### Modules

`game_data` ; TU `archetype_chain` ; `docs/etude-2-9-archetypes.md` (e1) ; `docs/plan-3.md`. Aucun autre.

### 2.12e1 : ce qui change (diff appliqué sur la copie, `p` → `e1`)

Le diff ci-dessous est le texte exact (`git apply` sur `f9c0297` passe). Lecture :
- `game_data.h` : commentaire de contrat du type étendu à `kw_own` (une ligne de plus), membre `kw_own` retiré, trois `extern` après `extern const pico8_jo_def_t pico8_os_def;`. **745 → 748** lignes.
- `game_data.c` : les quatre `.kw_own = {0, 0x0, 0x0},` retirés ; trois valeurs après la fin de `pico8_os_def`, avant `const pico8_kind_rec_t pico8_qh_kind_rec`. Elles nomment `qh_me_text`, `pm_me_text`, `qh_dq_text`, `hj_rr_text`, `pm_fz_text`, `mx_rr_text`, des `static` déclarés plus haut, et aucun `_start`.
- TU : inclusion de `planet_family_trade.h` (types `nr`, `fr`, `mx`) ; `same_kw` retiré (il deviendrait une fonction inutilisée, `-Werror`) ; commentaires de `same_jo_part` et `assert_lz_ran` ajustés ; `same_emitter_part` et trois tests après `test_os` ; total **350**.

```diff
diff --git a/src/pico8_game_data.c b/src/pico8_game_data.c
index 482ca96..77ee5d5 100644
--- a/src/pico8_game_data.c
+++ b/src/pico8_game_data.c
@@ -748,7 +748,6 @@ const pico8_jo_def_t pico8_jo_def = {
     .em = &pico8_jo_em_row,
     .mi = 0x0,
     .has_mi = 0,
-    .kw_own = {0, 0x0, 0x0},
 };
 
 const pico8_jo_def_t pico8_qh_def = {
@@ -767,7 +766,6 @@ const pico8_jo_def_t pico8_qh_def = {
     .em = &pico8_jo_em_row,
     .mi = 0x10000,
     .has_mi = 1,
-    .kw_own = {0, 0x0, 0x0},
     .hp = 0x10000,
     .px = &pico8_qh_px_row,
     .qh = 1,
@@ -792,7 +790,6 @@ const pico8_jo_def_t pico8_nl_def = {
     .em = &pico8_jo_em_row,
     .mi = 0x10000,
     .has_mi = 1,
-    .kw_own = {0, 0x0, 0x0},
     .hp = 0x60000,
     .px = &pico8_nl_px_row,
     .qh = 0,
@@ -820,7 +817,6 @@ const pico8_jo_def_t pico8_os_def = {
     .em = &pico8_jo_em_row,
     .mi = 0x10000,
     .has_mi = 1,
-    .kw_own = {0, 0x0, 0x0},
     .hp = 0x40000,
     .px = &pico8_os_px_row,
     .qh = 0,
@@ -829,6 +825,90 @@ const pico8_jo_def_t pico8_os_def = {
     .probe = {.dw = 0x666, .od = 0xf0000, .has_od = 1, .dq = qh_dq_text, .fp = 0x20000, .ik = 0x0, .has_ik = 0},
 };
 
+const pico8_jo_def_t pico8_hj_def = {
+    .me = qh_me_text,
+    .t = 0x0,
+    .hf = 0,
+    .hs = 0x10000,
+    .fa = 0x60000,
+    .kq = {-0x70000, -0x70000, 0x70000, 0x70000},
+    .ma = 0x0,
+    .ka = &pico8_jo_ka_seq,
+    .dz = pico8_jo_dz_rows,
+    .cq = &pico8_jo_cq_seq,
+    .gq = {0x0, 0xb0000},
+    .ca = 0x90000,
+    .em = &pico8_jo_em_row,
+    .mi = 0x10000,
+    .has_mi = 1,
+    .hp = 0x20000,
+    .px = &pico8_hj_px_row,
+    .qh = 0,
+    .s = 0xa60000,
+    .ba = &pico8_qh_ba_seq,
+    .rr = hj_rr_text,
+    .probe = {.dw = 0x666, .od = 0x3c0000, .has_od = 1, .dq = qh_dq_text, .fp = 0x20000, .ik = 0x0, .has_ik = 0},
+    .emitter = {.pn = 0x50000, .du = 0x78000, .py = 0x1eb8, .ki = 0x90000, .pk = 0x10000, .fc = 0x8000,
+                .k_ = &pico8_hj_k__seq, .qo = 0xfae0, .ro = -0x11ec, .qg = 0x11ea, .q_ = 0xfae0},
+};
+
+const pico8_jo_def_t pico8_pm_def = {
+    .me = pm_me_text,
+    .t = 0x0,
+    .hf = 0,
+    .hs = 0x10000,
+    .fa = 0x60000,
+    .kq = {-0x70000, -0x70000, 0x70000, 0x70000},
+    .ma = 0x0,
+    .ka = &pico8_jo_ka_seq,
+    .dz = pico8_jo_dz_rows,
+    .cq = &pico8_jo_cq_seq,
+    .gq = {0x0, 0xb0000},
+    .ca = 0x90000,
+    .em = &pico8_jo_em_row,
+    .mi = 0x10000,
+    .has_mi = 1,
+    .hp = 0x30000,
+    .px = &pico8_pm_px_row,
+    .qh = 0,
+    .s = 0x620000,
+    .ba = &pico8_qh_ba_seq,
+    .rr = hj_rr_text,
+    .ng = {0x10000, 0x0},
+    .fz = pm_fz_text,
+    .probe = {.dw = 0x666, .od = 0x3c0000, .has_od = 1, .dq = qh_dq_text, .fp = 0x20000, .ik = 0x0, .has_ik = 0},
+    .emitter = {.pn = 0x80000, .du = 0x20000, .py = 0xf5c, .ki = 0x90000, .pk = 0x0, .fc = 0x10000,
+                .k_ = &pico8_pm_k__seq, .qo = 0x1051e, .ro = -0xd70, .qg = 0xd70, .q_ = 0x1051e},
+};
+
+const pico8_jo_def_t pico8_mx_def = {
+    .me = qh_me_text,
+    .t = 0x0,
+    .hf = 0,
+    .hs = 0x10000,
+    .fa = 0x60000,
+    .kq = {-0x70000, -0x70000, 0x70000, 0x70000},
+    .ma = 0x0,
+    .ka = &pico8_jo_ka_seq,
+    .dz = pico8_jo_dz_rows,
+    .cq = &pico8_jo_cq_seq,
+    .gq = {0x0, 0xb0000},
+    .ca = 0x90000,
+    .em = &pico8_jo_em_row,
+    .mi = 0x10000,
+    .has_mi = 1,
+    .hp = 0x50000,
+    .px = &pico8_mx_px_row,
+    .qh = 0,
+    .s = 0x420000,
+    .ba = &pico8_qh_ba_seq,
+    .rr = mx_rr_text,
+    .ht = &pico8_mx_ht_seq,
+    .probe = {.dw = 0x666, .od = 0x280000, .has_od = 1, .dq = qh_dq_text, .fp = 0x20000, .ik = 0x0, .has_ik = 0},
+    .emitter = {.pn = 0x50000, .du = 0x50000, .py = 0x1eb8, .ki = 0x90000, .pk = 0x10000, .fc = 0x4ccc,
+                .k_ = &pico8_mx_k__seq, .qo = 0xfae0, .ro = -0x11ec, .qg = 0x11ea, .q_ = 0xfae0},
+};
+
 const pico8_kind_rec_t pico8_qh_kind_rec = {NULL, 1, 0xae0000, qh_me_text, 1, 0xf0000};
 const pico8_kind_rec_t pico8_nl_kind_rec = {nl_rr_text, 1, 0x440000, qh_me_text, 1, 0x140000};
 const pico8_kind_rec_t pico8_os_kind_rec = {NULL, 1, 0x640000, qh_me_text, 1, 0xf0000};
diff --git a/src/pico8_game_data.h b/src/pico8_game_data.h
index 2b4bed1..104794b 100644
--- a/src/pico8_game_data.h
+++ b/src/pico8_game_data.h
@@ -411,7 +411,8 @@ typedef struct {
 
 /* The ten planet kinds share this type; a kind's value leaves at zero the
  * members of the kinds it does not descend from. `kw`, and ga's `by`, point
- * at objects their module owns: not here. */
+ * at objects their module owns, and `kw_own` only holds what `lz` writes:
+ * not here. */
 typedef struct {
     const char *me;
     pico8_fixed_t t;
@@ -428,7 +429,6 @@ typedef struct {
     const pico8_jb_row_t *em;
     pico8_fixed_t mi;
     int has_mi;
-    pico8_kw_t kw_own;
     pico8_fixed_t hp;
     const pico8_jb_row_t *px;
     int qh;
@@ -713,6 +713,9 @@ extern const pico8_jo_def_t pico8_jo_def;
 extern const pico8_jo_def_t pico8_qh_def;
 extern const pico8_jo_def_t pico8_nl_def;
 extern const pico8_jo_def_t pico8_os_def;
+extern const pico8_jo_def_t pico8_hj_def;
+extern const pico8_jo_def_t pico8_pm_def;
+extern const pico8_jo_def_t pico8_mx_def;
 extern const pico8_kind_rec_t pico8_qh_kind_rec;
 extern const pico8_kind_rec_t pico8_nl_kind_rec;
 extern const pico8_kind_rec_t pico8_os_kind_rec;
diff --git a/tests/test_pico8_archetype_chain.c b/tests/test_pico8_archetype_chain.c
index 2e1f664..cef839e 100644
--- a/tests/test_pico8_archetype_chain.c
+++ b/tests/test_pico8_archetype_chain.c
@@ -20,6 +20,7 @@
 #include "../src/pico8_msgbox.h"
 #include "../src/pico8_planet.h"
 #include "../src/pico8_planet_family.h"
+#include "../src/pico8_planet_family_trade.h"
 #include "../src/pico8_probe.h"
 #include "../src/pico8_research.h"
 #include "../src/pico8_research_icon.h"
@@ -51,10 +52,6 @@ static void same_in(int ok, const char *kind, const char *field) {
     same(ok, what);
 }
 
-static int same_kw(const pico8_kw_t *got, const pico8_kw_t *want) {
-    return got->has_qe == want->has_qe && got->qe == want->qe && got->mu == want->mu;
-}
-
 static int same_text(const char *got, const char *want) {
     return got != NULL && want != NULL && strcmp(got, want) == 0;
 }
@@ -305,7 +302,7 @@ static void test_fv(void) {
     assert(fv->g_.probe->et == NULL);
 }
 
-/* `kw_own`, `kw` and `by` are left to each kind's test. */
+/* `kw`, `kw_own` and `by` are not the archetype's: left to each kind's test. */
 static void same_jo_part(const pico8_jo_t *jo, const pico8_jo_def_t *def, const char *kind) {
     same_in(same_text(jo->header.me, def->me), kind, "me");
     same_in(jo->header.t == def->t, kind, "t");
@@ -341,9 +338,8 @@ static void same_qh_part(const pico8_qh_t *qh, const pico8_jo_def_t *def, const
     assert(qh->jo.by == NULL);
 }
 
-/* A kind of the qh tree has an `hp`: its `_new` runs `lz`, which rewrites
- * `kw_own` from that research and points `kw` at it. Its `kw_own` is
- * compared on jo only. */
+/* A kind of the qh tree has an `hp`: its `_new` runs `lz`, which fills
+ * `kw_own` from that research and points `kw` at it. */
 static void assert_lz_ran(const pico8_jo_t *jo) {
     assert(jo->hn != NULL && jo->kw == &jo->kw_own);
 }
@@ -353,7 +349,6 @@ static void test_jo(void) {
     const pico8_jo_t *jo = pico8_jo_new(NULL);
 
     same_jo_part(jo, &pico8_jo_def, "jo");
-    same_in(same_kw(&jo->kw_own, &pico8_jo_def.kw_own), "jo", "kw_own");
     assert(jo->kw == pico8_jo_shared_kw());
     assert(jo->by == NULL);
 }
@@ -385,6 +380,58 @@ static void test_os(void) {
     assert_lz_ran(&os->qh.jo);
 }
 
+/* `qy` is left to each kind's test. */
+static void same_emitter_part(const pico8_emitter_t *emitter, const pico8_jo_def_t *def, const char *kind) {
+    same_in(emitter->pn == def->emitter.pn, kind, "emitter.pn");
+    same_in(emitter->du == def->emitter.du, kind, "emitter.du");
+    same_in(emitter->py == def->emitter.py, kind, "emitter.py");
+    same_in(emitter->ki == def->emitter.ki, kind, "emitter.ki");
+    same_in(emitter->pk == def->emitter.pk, kind, "emitter.pk");
+    same_in(emitter->fc == def->emitter.fc, kind, "emitter.fc");
+    same_in(emitter->k_ == def->emitter.k_, kind, "emitter.k_");
+    same_in(emitter->qo == def->emitter.qo, kind, "emitter.qo");
+    same_in(emitter->ro == def->emitter.ro, kind, "emitter.ro");
+    same_in(emitter->qg == def->emitter.qg, kind, "emitter.qg");
+    same_in(emitter->q_ == def->emitter.q_, kind, "emitter.q_");
+}
+
+/* `qy` is the fresh particle list `f_` gives every emitter. */
+static void test_hj(void) {
+    const pico8_hj_t *hj = pico8_hj_new(NULL);
+
+    same_jo_part(&hj->qh.jo, &pico8_hj_def, "hj");
+    same_qh_part(&hj->qh, &pico8_hj_def, "hj");
+    same_in(same_text(hj->rr, pico8_hj_def.rr), "hj", "rr");
+    same_emitter_part(hj->emitter, &pico8_hj_def, "hj");
+    assert(hj->emitter->qy != NULL && hj->emitter->qy->count == 0);
+    assert_lz_ran(&hj->qh.jo);
+}
+
+static void test_pm(void) {
+    const pico8_pm_t *pm = pico8_pm_new(NULL);
+
+    same_jo_part(&pm->hj.qh.jo, &pico8_pm_def, "pm");
+    same_qh_part(&pm->hj.qh, &pico8_pm_def, "pm");
+    same_in(same_text(pm->hj.rr, pico8_pm_def.rr), "pm", "rr");
+    same_in(same_vec(pm->ng, &pico8_pm_def.ng), "pm", "ng");
+    same_in(same_text(pm->fz, pico8_pm_def.fz), "pm", "fz");
+    same_emitter_part(pm->hj.emitter, &pico8_pm_def, "pm");
+    assert(pm->hj.emitter->qy != NULL && pm->hj.emitter->qy->count == 0);
+    assert_lz_ran(&pm->hj.qh.jo);
+}
+
+static void test_mx(void) {
+    const pico8_mx_t *mx = pico8_mx_new(NULL);
+
+    same_jo_part(&mx->hj.qh.jo, &pico8_mx_def, "mx");
+    same_qh_part(&mx->hj.qh, &pico8_mx_def, "mx");
+    same_in(same_text(mx->hj.rr, pico8_mx_def.rr), "mx", "rr");
+    same_in(mx->ht == pico8_mx_def.ht, "mx", "ht");
+    same_emitter_part(mx->hj.emitter, &pico8_mx_def, "mx");
+    assert(mx->hj.emitter->qy != NULL && mx->hj.emitter->qy->count == 0);
+    assert_lz_ran(&mx->hj.qh.jo);
+}
+
 int main(void) {
     pico8_mem_init(&mem);
     pico8_gfx_init(&gfx);
@@ -417,7 +464,10 @@ int main(void) {
     test_qh();
     test_nl();
     test_os();
-    assert(g_compared == 231 && "families a to d compare 231 fields: one missed or one too many");
+    test_hj();
+    test_pm();
+    test_mx();
+    assert(g_compared == 350 && "families a to e1 compare 350 fields: one missed or one too many");
 
     printf("test_pico8_archetype_chain: all tests passed\n");
     return 0;
```

Valeurs : annexe B de l'étude, recopiées de `pico8_ke_start` (`t`), `pico8_jo_start`, `pico8_qh_start`, `pico8_hj_start`, `pico8_pm_start`, `pico8_mx_start` et des littéraux de `pico8_jo_struct_init`/`pico8_qh_struct_init` (`has_mi = 1`, `has_od = 1`, `ik = 0`, `has_ik = 0`). Masquages portés (étude §3) : `hj` masque `hp`, `px`, `qh`, `s`, `probe.od` ; `pm` masque en plus `me` (`pm_me_text`) et dix membres de l'émetteur (`pk` 0 compris) ; `mx` masque `rr` (`mx_rr_text`), `du`, `fc`, `k_`, `probe.od`. `pm.ng` est par valeur (l'instance pointe `&pico8_pm_start.ng` et se compare par contenu) ; `hj.rr`, `pm.fz` par contenu ; `mx.ht`, `px`, `k_` par adresse. Chaque membre comparé a son désignateur, même nul (`.pk = 0x0`, `.qh = 0`). Les membres des sortes absentes de la chaîne restent à zéro (`nl`, `qw`, `iu`, `ms`, `hz`, `mt` ; `ng`/`fz` hors `pm`, `ht` hors `mx`).

Lecture du TU :
- Les parties se lisent par leurs membres (`&pm->hj.qh.jo`, `pm->hj.emitter`), jamais par une conversion : aucun cast, aucun `emitter_of`/`probe_of`, aucune comparaison de classe.
- `same_emitter_part` compare les onze membres de `pico8_emitter_def_t`, étiquettes `sorte.emitter.X`. `qy` (E) reste au test de chaque sorte : `hj`, `pm`, `mx` ont la liste neuve de `f_` (non NULL, vide, mesuré) ; `ga` a déjà les particules que `ez` crée dans `ga_pq` (140 mesurées : `assert` non NULL seulement, le nombre dépend de `ez`).
- Répartition : `hj` 39 = 15 + 12 + `rr` + 11 ; `pm` 41 = 15 + 12 + `rr`, `ng`, `fz` + 11 ; `mx` 40 = 15 + 12 + `rr`, `ht` + 11. **120**, et −1 (`jo.kw_own`) : **231 → 350**.
- Hors compte, en `assert` nus : `qy` ; `assert_lz_ran` (`lz` a tourné, `kw` pointe `kw_own`) sur les trois ; `et` NULL et `by` NULL dans `same_qh_part`.

**`docs/etude-2-9-archetypes.md`** (e1, numéros de `f9c0297`) — la correction, sans réécrire l'étude :
1. `:86`, ligne `jo` du tableau par type : `| 37 | 10 | 6 | 1 | 20 | 0 |` → `| 37 | 10 | 5 | 1 | 21 | 0 |`.
2. `:109` : « Les 42 champs **I** » → « Les 41 champs **I** », et `` `jo.by`/`has_mi`/`kw`/`kw_own`/`ma`/`mi` `` → `` `jo.by`/`has_mi`/`kw`/`ma`/`mi` ``.
3. `:268` : « , et `kw_own` : l'init fait `self->kw_own = def->kw_own` » → « . `kw_own` n'en est pas : c'est un E (corrigé à 2.12e1, annexe A) ».
4. `:272-281` : « `` `jo.kw_own`, `` » retiré des dix lignes du tableau « Part copiée ».
5. `:192` et `:220` (texte C du §2) : la ligne `pico8_kw_t kw_own;` et `` .kw_own = {0, 0x0, 0x0}, `` retirés.
6. `:308`, exception 2, remplacée par : « `jo.kw_own` n'est pas un champ d'archétype (E, corrigé à 2.12e1) : c'est le stockage de la table que `pico8_jo_lz` crée, lu seulement par `kw` après que `lz` a écrit ses trois membres. Le test ne le compare sur aucune sorte ; `assert_lz_ran` vérifie que `kw` pointe dessus sur les huit sortes où `lz` tourne (toutes sauf `jo` et `ga`). »
7. Tableau « Masquages », colonne « Comparés (A + I) » : I −1 sur les dix sortes planète (`jo` 10 + 5, `qh` 19 + 8, `nl` 21 + 9, `os` 19 + 8, `hj` 23 + 16, `pm` 25 + 16, `mx` 23 + 17, `ga` 14 + 15, `nr` 21 + 8, `fr` 22 + 8).
8. §4 : « (214 comparaisons) » → « (208 comparaisons) » ; tableau 2.12 : 2.12d `103` → `100 (99 après 2.12e1)`, 2.12e1 `123` → `120`, 2.12e2 `91` → `88`.
9. `:584`, annexe A, ligne `jo`/`kw_own` : `| I |` → `| E |`, verdict : « stockage de la table que `jo_lz` crée ; lu seulement par `kw`, après `lz` ; la valeur initiale n'est jamais lue (corrigé à 2.12e1) ».
10. En tête de l'annexe B, une ligne : « Correction 2.12e1 : les entrées `jo.kw_own` ci-dessous sont un E, hors archétype. »

Rien d'autre dans l'étude (les Runs de l'annexe C sont l'histoire : ils restent).

**`docs/plan-3.md`** (e1) — ligne 2.12e1 : `123 comparaisons` → `120 comparaisons`, puis la note « Découvert à 2.12e1 » : `kw_own` retiré (E, constat du vérificateur de 2.12d ; étude corrigée) ; total du TU 231 → 230 → **350** ; `défauts` 669, pied **1073 / 85**, `inclusion-mutuelle` 17 / 16 ; `--champs` **19 → 20** et pourquoi ; `game_data.h` **748** ; ce qui remplace le « pour 2.13d » de la note 2.12d sur `kw_own` (voir « Contrat pour 2.13 ») ; le filet unique de `probe.has_ik` (H1) ; le repli s'il a servi.

### 2.12e2 : ce qui change (diff appliqué sur la copie, `e1` → `e2`)

- `game_data.h` : trois `extern` après `extern const pico8_jo_def_t pico8_mx_def;`. **748 → 751** lignes.
- `game_data.c` : trois valeurs après la fin de `pico8_mx_def`, avant `const pico8_kind_rec_t pico8_qh_kind_rec`. Elles nomment `jo_me_text`, `qh_me_text`, `qh_dq_text`, `ga_fz_text`, `nr_rr_text`, `fr_rr_text`, et aucun `_start`. `pico8_ga_def` n'a ni `qh` ni `probe` (zéros).
- TU : `test_ga`, `test_nr`, `test_fr` après `test_mx` ; total **438**.

```diff
diff --git a/src/pico8_game_data.c b/src/pico8_game_data.c
index 77ee5d5..fa0dac4 100644
--- a/src/pico8_game_data.c
+++ b/src/pico8_game_data.c
@@ -909,6 +909,82 @@ const pico8_jo_def_t pico8_mx_def = {
                 .k_ = &pico8_mx_k__seq, .qo = 0xfae0, .ro = -0x11ec, .qg = 0x11ea, .q_ = 0xfae0},
 };
 
+const pico8_jo_def_t pico8_ga_def = {
+    .me = jo_me_text,
+    .t = 0x0,
+    .hf = 0,
+    .hs = 0x10000,
+    .fa = 0x30000,
+    .kq = {0x0, 0x0, -0x10000, -0x10000},
+    .ma = 0x0,
+    .ka = &pico8_jo_ka_seq,
+    .dz = pico8_jo_dz_rows,
+    .cq = &pico8_jo_cq_seq,
+    .gq = {0x0, 0xb0000},
+    .ca = 0x90000,
+    .em = &pico8_jo_em_row,
+    .mi = 0x0,
+    .has_mi = 0,
+    .fz = ga_fz_text,
+    .iu = -0xf5c,
+    .ms = 0x30000,
+    .emitter = {.pn = 0x0, .du = 0x0, .py = 0x60000, .ki = 0x0, .pk = 0x0, .fc = 0x0,
+                .k_ = &pico8_ga_k__seq, .qo = 0xe666, .ro = 0x0, .qg = 0x0, .q_ = 0xe666},
+};
+
+const pico8_jo_def_t pico8_nr_def = {
+    .me = qh_me_text,
+    .t = 0x0,
+    .hf = 0,
+    .hs = 0x10000,
+    .fa = 0x60000,
+    .kq = {-0x40000, -0x40000, 0x50000, 0x50000},
+    .ma = 0x0,
+    .ka = &pico8_jo_ka_seq,
+    .dz = pico8_jo_dz_rows,
+    .cq = &pico8_jo_cq_seq,
+    .gq = {0x0, 0xb0000},
+    .ca = 0x90000,
+    .em = &pico8_jo_em_row,
+    .mi = 0x10000,
+    .has_mi = 1,
+    .hp = 0x10000,
+    .px = &pico8_qh_px_row,
+    .qh = 1,
+    .s = 0xaa0000,
+    .ba = &pico8_qh_ba_seq,
+    .rr = nr_rr_text,
+    .hz = &pico8_nr_hz_seq,
+    .probe = {.dw = 0x28e, .od = 0xa0000, .has_od = 1, .dq = qh_dq_text, .fp = 0x10000, .ik = 0x0, .has_ik = 0},
+};
+
+const pico8_jo_def_t pico8_fr_def = {
+    .me = qh_me_text,
+    .t = 0x0,
+    .hf = 0,
+    .hs = 0x10000,
+    .fa = 0x60000,
+    .kq = {-0x40000, -0x40000, 0x50000, 0x50000},
+    .ma = 0x0,
+    .ka = &pico8_jo_ka_seq,
+    .dz = pico8_jo_dz_rows,
+    .cq = &pico8_jo_cq_seq,
+    .gq = {0x0, 0xb0000},
+    .ca = 0x90000,
+    .em = &pico8_jo_em_row,
+    .mi = 0x10000,
+    .has_mi = 1,
+    .hp = 0x10000,
+    .px = &pico8_qh_px_row,
+    .qh = 1,
+    .s = 0x4e0000,
+    .ba = &pico8_qh_ba_seq,
+    .rr = fr_rr_text,
+    .hz = &pico8_fr_hz_seq,
+    .mt = 0x10000,
+    .probe = {.dw = 0x28e, .od = 0x190000, .has_od = 1, .dq = qh_dq_text, .fp = 0x10000, .ik = 0x0, .has_ik = 0},
+};
+
 const pico8_kind_rec_t pico8_qh_kind_rec = {NULL, 1, 0xae0000, qh_me_text, 1, 0xf0000};
 const pico8_kind_rec_t pico8_nl_kind_rec = {nl_rr_text, 1, 0x440000, qh_me_text, 1, 0x140000};
 const pico8_kind_rec_t pico8_os_kind_rec = {NULL, 1, 0x640000, qh_me_text, 1, 0xf0000};
diff --git a/src/pico8_game_data.h b/src/pico8_game_data.h
index 104794b..393cb85 100644
--- a/src/pico8_game_data.h
+++ b/src/pico8_game_data.h
@@ -716,6 +716,9 @@ extern const pico8_jo_def_t pico8_os_def;
 extern const pico8_jo_def_t pico8_hj_def;
 extern const pico8_jo_def_t pico8_pm_def;
 extern const pico8_jo_def_t pico8_mx_def;
+extern const pico8_jo_def_t pico8_ga_def;
+extern const pico8_jo_def_t pico8_nr_def;
+extern const pico8_jo_def_t pico8_fr_def;
 extern const pico8_kind_rec_t pico8_qh_kind_rec;
 extern const pico8_kind_rec_t pico8_nl_kind_rec;
 extern const pico8_kind_rec_t pico8_os_kind_rec;
diff --git a/tests/test_pico8_archetype_chain.c b/tests/test_pico8_archetype_chain.c
index cef839e..8b1899b 100644
--- a/tests/test_pico8_archetype_chain.c
+++ b/tests/test_pico8_archetype_chain.c
@@ -432,6 +432,44 @@ static void test_mx(void) {
     assert_lz_ran(&mx->hj.qh.jo);
 }
 
+/* `kw` and `by` are the module's shared objects, not the archetype's; ga has
+ * no `hp`, so `lz` does not run. Its `qy` already holds what `ez` spawned in
+ * `ga_pq`. */
+static void test_ga(void) {
+    const pico8_ga_t *ga = pico8_ga_new(NULL);
+    const pico8_ga_t *other = pico8_ga_new(NULL);
+
+    same_jo_part(&ga->jo, &pico8_ga_def, "ga");
+    same_in(same_text(ga->fz, pico8_ga_def.fz), "ga", "fz");
+    same_in(ga->iu == pico8_ga_def.iu, "ga", "iu");
+    same_in(ga->ms == pico8_ga_def.ms, "ga", "ms");
+    same_emitter_part(ga->emitter, &pico8_ga_def, "ga");
+    assert(ga->emitter->qy != NULL);
+    assert(ga->jo.kw == pico8_jo_shared_kw());
+    assert(ga->jo.by != NULL && ga->jo.by == other->jo.by);
+}
+
+static void test_nr(void) {
+    const pico8_nr_t *nr = pico8_nr_new(NULL);
+
+    same_jo_part(&nr->qh.jo, &pico8_nr_def, "nr");
+    same_qh_part(&nr->qh, &pico8_nr_def, "nr");
+    same_in(nr->hz == pico8_nr_def.hz, "nr", "hz");
+    same_in(same_text(nr->rr, pico8_nr_def.rr), "nr", "rr");
+    assert_lz_ran(&nr->qh.jo);
+}
+
+static void test_fr(void) {
+    const pico8_fr_t *fr = pico8_fr_new(NULL);
+
+    same_jo_part(&fr->nr.qh.jo, &pico8_fr_def, "fr");
+    same_qh_part(&fr->nr.qh, &pico8_fr_def, "fr");
+    same_in(fr->nr.hz == pico8_fr_def.hz, "fr", "hz");
+    same_in(same_text(fr->nr.rr, pico8_fr_def.rr), "fr", "rr");
+    same_in(fr->mt == pico8_fr_def.mt, "fr", "mt");
+    assert_lz_ran(&fr->nr.qh.jo);
+}
+
 int main(void) {
     pico8_mem_init(&mem);
     pico8_gfx_init(&gfx);
@@ -467,7 +505,10 @@ int main(void) {
     test_hj();
     test_pm();
     test_mx();
-    assert(g_compared == 350 && "families a to e1 compare 350 fields: one missed or one too many");
+    test_ga();
+    test_nr();
+    test_fr();
+    assert(g_compared == 438 && "families a to e compare 438 fields: one missed or one too many");
 
     printf("test_pico8_archetype_chain: all tests passed\n");
     return 0;
```

Répartition : `ga` 29 = 15 + `fz`, `iu`, `ms` + 11 ; `nr` 29 = 15 + 12 + `hz`, `rr` ; `fr` 30 = 15 + 12 + `hz`, `rr`, `mt`. **88** : 350 → **438**. Masquages portés : `ga` masque `me` (`jo_me_text`, celui de `jo`), `hf`, `fa`, `kq` et tout l'émetteur ; `nr` masque `kq`, `s`, `probe.dw`, `probe.od`, `probe.fp` ; `fr` masque en plus `hz`, `rr`. Hors compte : `ga.jo.kw == pico8_jo_shared_kw()` et un `by` non NULL partagé par deux `ga` (exception 1 : `&g_ga_by_shared`, que le TU ne peut pas nommer) ; `assert_lz_ran` sur `nr`, `fr`. `ga` n'a pas de `hp` : `lz` ne tourne pas.

**`docs/plan-3.md`** (e2) — ligne 2.12e2 : `91 comparaisons` → `88 comparaisons`, puis la note « Découvert à 2.12e2 » : total **438** ; `ga` 29 et non 30 (`kw_own`) ; comptes (`défauts` 669, pied, `inclusion-mutuelle`, `--champs` 20) ; `game_data.h` **751** ; les 26 archétypes présents (contrôle ci-dessous) ; C4 (le `pn` de `ga`, jamais lu, sans autre filet que le test ponctuel).

### Fin de 2.12 : les 26 archétypes de l'étude existent (mesuré sur `e2`)

Étude §2, inventaire : 26 classes reçoivent un archétype, 14 tables n'en reçoivent pas (40 tables `g_X_struct_class`). Sur `e2` :
- `grep -c 'const pico8_[a-z_]*_def_t pico8_[a-z_]*_def = {' src/pico8_game_data.c` → **25** : `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `hp`, `ie`, `pj`, `mz`, `jc`, `g_`, `fv`, `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `ga`, `nr`, `fr` ; plus `pico8_hy_def` dans `economy.c` : **26**.
- `git grep -hoE 'static const pico8_struct_class_t g_[a-z0-9_]+_struct_class\b' -- src | sort -u` → **40** tables. Les 14 sans archétype : `bv_et`, `fm_placeholder`, `ha_pz`, `ie_ke_view`, `jb`, `jc_mouse_point`, `jc_too_far`, `je_placeholder`, `lq`, `mo_placeholder`, `mz_jj`, `nr_ldown_empty`, `pj_cr`, `trade` — exactement les « non » de l'inventaire. Les 26 autres tables ont chacune leur `pico8_X_def`. Aucun ne manque.

### Comptes, mesurés sur la copie

| Mesure | `p` | `e1` | `e2` |
|---|---|---|---|
| `make composition-residue` | 1073 / 85 | **1073 / 85** | **1073 / 85** |
| `défauts` (`--check`) | 669 / 52 | **669 / 52** | **669 / 52** |
| `inclusion-mutuelle` | 17 / 16, 43 lignes | **17 / 16, 43 lignes** | **17 / 16, 43 lignes** |
| `--champs` | 282 ; 15 ; 19 | **282 ; 15 ; 20** | **282 ; 15 ; 20** |
| comparaisons du test ponctuel | 231 | **350** (−1 + 120) | **438** (+88) |
| membres de `pico8_jo_def_t` | 33 | **32** | 32 |
| `game_data.h` | 745 | **748** | **751** |
| `grep -c kw_own src/pico8_game_data.c` | 4 | **0** | 0 |
| `git diff --stat` (sans `docs/`) | — | 3 fichiers, 149 insertions, 16 suppressions | 3 fichiers, 121 insertions, 1 suppression |

**`--champs`, anticipé puis mesuré.** Le relevé compte, par nom de membre, les lignes `.X =` d'un `.c` de `src/`, désignateurs imbriqués compris ; une ligne hors fonction est « hors constructeur ». e1 retire les quatre `.kw_own = {…}` : `pico8_jo_t kw_own` passe de 5 / 4 à **1 / 0** et rentre dans « 0 hors constructeur » (19 → **20**) ; c'est le seul couple (type, membre) qui change de groupe (jointure `p`/`e1` complète). Les trois valeurs font monter 63 lignes sans changer de groupe (`fa`, `me`, `probe`, `emitter`, `fz`… déjà hors du groupe). e2 : 66 lignes montent, **aucune** ne change de groupe (20). Aucun membre neuf (282), aucun ne quitte « 0 écriture » (15). Positionnels (`kq`, `gq`, `ng`) : seul le nom englobant compte.

### Contrat pour 2.13 (à recopier dans ses PRD)

- **2.13d** : `pico8_jo_struct_init` ne copie pas `kw_own` d'un `def` (il n'y est plus). Aujourd'hui `planet.c:174` fait `self->kw_own = *pico8_jo_start.kw;` ; quand `pico8_jo_kw_start` sort, `kw_own` est mis à zéro (ou laissé à `lz`, qui l'écrit avant toute lecture). `test_pico8_jo_spec_defaults.c:74` et `:230`, qui lisent ce défaut, sont réécrits. Ceci remplace la phrase « `pico8_jo_struct_init` copie `def->kw_own` » de la note 2.12d. La jumelle K1 (plus bas) le montre : après e1, `pico8_jo_kw_start` n'a plus que `hp` et `jo` pour filet.
- **2.13d/e** : `probe.has_ik` de l'arbre `qh` n'a **d'autre filet que le test ponctuel** (H1 : `has_ik = 0` → `1` dans `pico8_qh_struct_init`, rouge au seul `archetype_chain`, au parent comme au commit, 26 e2e). Un TU durable doit le garder avant que la fonction du test ponctuel parte.
- **2.13e2** : le `pn` (et `du`, `ki`, `pk`) de `ga` est 0 et n'est lu que par `g_:mm` (`particle.h:29`) : C4 n'a que le test ponctuel pour filet, et aucune lecture du jeu. À noter, pas à garder.
- `kw` reste `&g_jo_kw_shared` posé par l'init, `by` NULL ou `&g_ga_by_shared` ; aucun n'est membre de `pico8_jo_def_t`.

### Anti-tassement

- **Interdit** :
  - un second type d'archétype planète ; un membre ajouté à `pico8_jo_def_t` ; garder `kw_own` dans le type ou une valeur ;
  - comparer `kw_own` (à l'archétype ou à `pico8_jo_kw_start`) dans le test ponctuel ;
  - nommer un `pico8_X_start` dans une valeur (`défauts` → 670) ;
  - mettre `kw`, `by` ou un pointeur vers un statique de module dans le type ou une valeur ;
  - un membre `def` dans `pico8_jo_t`, `pico8_qh_t`, `pico8_hj_t`, `pico8_ga_t`, `pico8_emitter_t`, `pico8_probe_t` ; toucher `planet.h`, `planet_family.h`, `planet_family_trade.h`, `particle.h`, `probe.h` ;
  - faire lire l'archétype par un `_new`, une init ou un lecteur ;
  - un cast, un `void *`, un `emitter_of`/`probe_of`/`pico8_qh_of`, une comparaison de classe dans le TU ou les archétypes ;
  - comparer à un `pico8_X_start` au lieu de `pico8_X_def` ;
  - fondre e1 et e2 en un commit, ou mettre le retrait de `kw_own` dans e2 ;
  - toucher un `*.sha256`, `tests/e2e/`, une constante d'empreinte, `tools/composition_residue.sh` ou son TU, la ligne 2.12d du plan.

## Implementation Chunks

Deux commits, **≈ 45 min** (e1) et **≈ 35 min** (e2), preuves comprises. Mesures de l'essai : `make test` ≈ 10 s, `make e2e` ≈ 1 s, une mutation ≈ 6 s (unités et e2e), preuve `const` 13 s, objets ≈ 1 min par paire, couverture 30 s ; e1 porte en plus la correction de l'étude (≈ 10 min de texte). Budget **50 min** chacun.

1. **2.12e1** : appliquer le diff e1 ; `make test`, `make e2e`, compteur, `--champs` ; preuves (M, jumelles C au commit puis au parent `f9c0297`, K1, H1, objets, `const`, couverture, greps) ; étude corrigée ; note du plan (ligne 2.12e1) ; commit ; push.
2. **2.12e2** : appliquer le diff e2 ; mêmes mesures ; preuves (parent = le commit e1) ; contrôle des 26 archétypes ; note (ligne 2.12e2) ; commit ; push.

**Replis écrits** (au-delà de 1,5 fois le budget avant le commit), chacun vert avec son total asserté, écrits dans la note « Découvert à » :
- e1 → **2.12e1a** : le seul retrait de `kw_own` et la correction de l'étude (total **230**, `--champs` 20, `game_data.h` 745) ; **2.12e1b** : `hj`, `pm`, `mx` (total **350**, `game_data.h` 748).
- e2 → **2.12e2a** : `ga` (total **379**, `game_data.h` 749) ; **2.12e2b** : `nr`, `fr` (total **438**, 751).

## Testing Decisions

- **Pas de TU neuf** : le test ponctuel gagne six fonctions et une aide, et perd `same_kw`. Il ne rejoue jamais un `events.json`.
- **Bon test ici** : la valeur que la chaîne laisse dans une instance fraîche, comparée au seul objet neuf.
- **Prior art** : PRD 2.12a à 2.12d (même TU, mêmes preuves) ; règle du parent de D.21 et 2.4 ; preuve `const` de l'étude 2.9, annexe A ; découpage en parties d'un PRD commun : 2.3a–c.

### Preuves (chaque commit ; parent de e1 = `f9c0297`, parent de e2 = le commit e1)

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. Mesuré : code 0, `test_pico8_archetype_chain: all tests passed`, 26 conformes, aux deux commits.

2. **Test ponctuel** : vert = 350 (e1), 438 (e2) égalités ; l'assert du total = aucune oubliée, aucune de trop.

3. **Mutations**, une à la fois, `make -k -j4 unit` puis `make -k e2e`, restauration par `git checkout -- <fichier>` puis `touch`. **Règle du parent** : une mutation verte au commit et rouge au parent est un **ÉCHEC** ; verte aux deux, « chemin jamais atteint ». L'étiquette rouge se lit dans `archetype differs from its chain: …`. Mesuré sur la copie :

   *2.12e1, côté archétype (au parent, les trois valeurs n'existent pas : chacune doit être rouge au commit).* Toutes : `archetype_chain` seul rouge, **26 / 26** e2e.

   | # | Mutation | Étiquette |
   |---|---|---|
   | M1 | étude §3 e1, défaut propre : `pico8_pm_def.ng` `{0x10000, 0x0}` → `{0x20000, 0x0}` | `pm.ng` |
   | M2 | étude §3 e1, masqué rendu à l'ascendante : `pico8_mx_def.rr` → `hj_rr_text` | `mx.rr` |
   | M3 | en-tête masqué : `pico8_pm_def.me` → `qh_me_text` | `pm.me` |
   | M4 | émetteur A : `pico8_hj_def.emitter.fc` `0x8000` → `0x10000` | `hj.emitter.fc` |
   | M5 | émetteur, masque de `pm` rendu à `hj` : `.pk` `0x0` → `0x10000` | `pm.emitter.pk` |
   | M6 | émetteur, masque de `mx` rendu à `hj` : `.du` `0x50000` → `0x78000` | `mx.emitter.du` |
   | M7 | sonde, masque de `hj` rendu à `qh` : `pico8_hj_def.probe.od` `0x3c0000` → `0xf0000` | `hj.probe.od` |
   | M8 | adresse propre : `pico8_mx_def.ht` → `&pico8_nr_hz_seq` | `mx.ht` |
   | M9 | adresse masquée : `pico8_hj_def.px` → `&pico8_qh_px_row` | `hj.px` |
   | M10 | séquence d'émetteur : `pico8_pm_def.emitter.k_` → `&pico8_hj_k__seq` | `pm.emitter.k_` |
   | M11 | I masqué : `pico8_hj_def.s` `0xa60000` → `0xae0000` | `hj.s` |
   | M12 | comparaison oubliée : `same_in(mx->ht == …)` retirée du TU | `g_compared == 350` |
   | M13 | émetteur signé : `pico8_hj_def.emitter.ro` `-0x11ec` → `-0x11ea` | `hj.emitter.ro` |
   | M14 | texte propre : `pico8_pm_def.fz` → `ga_fz_text` | `pm.fz` |

   *2.12e2, côté archétype.* Toutes : `archetype_chain` seul rouge, **26 / 26** e2e.

   | # | Mutation | Étiquette |
   |---|---|---|
   | M1 | étude §3 e2, défaut propre : `pico8_fr_def.mt` `0x10000` → `0x20000` | `fr.mt` |
   | M2 | étude §3 e2, masqué rendu à l'ascendante : `pico8_ga_def.kq` → `{-0x70000, -0x70000, 0x70000, 0x70000}` | `ga.kq` |
   | M3 | masque de `ga` rendu à `jo` : `.fa` `0x30000` → `0x60000` | `ga.fa` |
   | M4 | émetteur I : `pico8_ga_def.emitter.qo` `0xe666` → `0x1051e` | `ga.emitter.qo` |
   | M5 | I propre : `pico8_ga_def.iu` `-0xf5c` → `-0x24e` | `ga.iu` |
   | M6 | émetteur I : `pico8_ga_def.emitter.py` `0x60000` → `0x90000` | `ga.emitter.py` |
   | M7 | sonde, masque de `nr` rendu à `qh` : `pico8_nr_def.probe.dw` `0x28e` → `0x666` | `nr.probe.dw` |
   | M8 | masque de `fr` rendu à `nr` : `pico8_fr_def.hz` → `&pico8_nr_hz_seq` | `fr.hz` |
   | M9 | texte masqué : `pico8_fr_def.rr` → `nr_rr_text` | `fr.rr` |
   | M10 | boîte masquée rendue à `jo` : `pico8_nr_def.kq` | `nr.kq` |
   | M11 | I masqué rendu à `nr` : `pico8_fr_def.s` `0x4e0000` → `0xaa0000` | `fr.s` |
   | M12 | drapeau I : `pico8_ga_def.has_mi` `0` → `1` | `ga.has_mi` |
   | M13 | comparaison oubliée : `same_in(ga->ms == …)` retirée du TU | `g_compared == 438` |
   | M14 | en-tête : `pico8_ga_def.me` → `qh_me_text` | `ga.me` |

   « `archetype_chain` seul » et 26 e2e : **personne ne lit l'archétype**.

   *2.12e1, côté chaîne, jumelles au commit et au parent `f9c0297`.* Les rouges au parent sont ceux du commit sans `archetype_chain`, sauf K1 (voulu, voir plus bas).

   | # | Mutation | TU rouges au commit | TU rouges au parent | e2e (= aux deux) |
   |---|---|---|---|---|
   | C1 | `pico8_hj_start.fc` `0x8000` → `0x10000` | `archetype_chain`, `hj` | `hj` | 26 |
   | C2 | `pico8_pm_start.ng` → `{0x20000, 0x0}` | `archetype_chain`, `ga`, `pm` | `ga`, `pm` | 25 |
   | C3 | `pico8_mx_start.rr` → `hj_rr_text` | `archetype_chain`, `entity_kind`, `hj`, `map_tables`, `mx` | les mêmes sans `archetype_chain` | 26 |
   | C4 | `pico8_mx_start.ht` → `&pico8_nr_hz_seq` | `archetype_chain`, `mx` | `mx` | 25 |
   | C5 | `pico8_pm_struct_new` : `header.me = pico8_pm_start.me;` retirée (masque perdu) | `archetype_chain`, `hj`, `hp`, `jo_spec_defaults`, `pm`, `qh`, `ui` | les mêmes sans `archetype_chain` | 25 |
   | C6 | `pico8_hj_struct_init` : `probe->od = pico8_hj_start.od;` retirée | `archetype_chain`, `hj`, `jo_spec_defaults`, `pm` | idem sans | 25 |
   | C7 | `pico8_mx_struct_new` : `emitter->du = pico8_mx_start.du;` retirée | `archetype_chain`, `hj`, `jo_spec_defaults`, `mx` | idem sans | 25 |
   | C8 | `pico8_hj_start.s` `0xa60000` → `0xa70000` | `archetype_chain`, `entity_kind`, `hj` | `entity_kind`, `hj` | 25 |
   | C9 | `pico8_pm_struct_new` : `emitter->pk = pico8_pm_start.pk;` retirée | `archetype_chain`, `hj`, `jo_spec_defaults`, `pm` | idem sans | 26 |
   | C10 | `pico8_hj_struct_init` : `qo = pico8_hj_start.qo` → `.ro` | `archetype_chain`, `hj`, `jo_spec_defaults` | idem sans | 26 |
   | C11 | `pico8_pm_start.fz` → `pm_fz_text + 1` | `archetype_chain`, `pm` | `pm` | 26 |
   | C12 | `pico8_hj_start.px` → `&pico8_qh_px_row` | `archetype_chain`, `hj`, `qh` | `hj`, `qh` | 26 |
   | K1 | `pico8_jo_kw_start` `{0, 0x0, 0x0}` → `{0, 0x0, 0x10000}` | `hp`, `jo` | `archetype_chain` (`jo.kw_own`), `hp`, `jo` | 26 |
   | H1 | `pico8_qh_struct_init` : `has_ik = 0` → `1` | `archetype_chain` (`qh.probe.has_ik`) | `archetype_chain` | 26 |

   Aucun ÉCHEC. K1 est la preuve attendue du retrait : le test ponctuel ne voit plus `kw_own`, `hp` et `jo` le gardent. H1 : seul filet, le test ponctuel (contrat pour 2.13). Piège de l'essai : C11 écrit d'abord `→ ga_fz_text` ne compile pas au parent (`pm_fz_text` inutilisé, `-Werror`) : garder `pm_fz_text + 1`.

   *2.12e2, côté chaîne, au commit et au parent (le commit e1).*

   | # | Mutation | TU rouges au commit | TU rouges au parent | e2e (= aux deux) |
   |---|---|---|---|---|
   | C1 | `pico8_ga_start.fa` `0x30000` → `0x60000` | `archetype_chain`, `ga` | `ga` | 24 |
   | C2 | `pico8_ga_struct_new` : `jo.kq = &pico8_ga_start.kq;` retirée | `archetype_chain`, `ga`, `jo_spec_defaults` | idem sans | 26 |
   | C3 | `pico8_ga_start.iu` `-0xf5c` → `-0xf5d` | `archetype_chain`, `ga` | `ga` | 25 |
   | C4 | `pico8_ga_struct_new` : `emitter->pn = 0;` → `0x10000` | `archetype_chain` (`ga.emitter.pn`) | **aucun** | 26 |
   | C5 | `pico8_nr_start.dw` `0x28e` → `0x666` | `archetype_chain`, `fr`, `nr`, `qh` | idem sans | 23 |
   | C6 | `pico8_nr_struct_init` : `jo.kq = &pico8_nr_start.kq;` retirée | `archetype_chain`, `fr`, `jo_spec_defaults`, `nr` | idem sans | 26 |
   | C7 | `pico8_fr_start.mt` `0x10000` → `0x20000` | `archetype_chain`, `fr` | `fr` | 26 |
   | C8 | `pico8_fr_struct_new` : `nr.hz = pico8_fr_start.hz;` retirée | `archetype_chain`, `fr`, `jo_spec_defaults`, `nr` | idem sans | 25 |
   | C9 | `pico8_fr_start.s` `0x4e0000` → `0x4f0000` | `archetype_chain`, `entity_kind`, `fr`, `nr` | idem sans | 25 |
   | C10 | `pico8_ga_start.ms` `0x30000` → `0x40000` | `archetype_chain`, `ga`, `hp` | `ga`, `hp` | 25 |
   | C11 | `pico8_ga_struct_new` : `jo.hf = pico8_ga_start.hf;` retirée | `archetype_chain`, `ga`, `jo_spec_defaults` | idem sans | 26 |
   | C12 | `pico8_nr_start.fp` `0x10000` → `0x20000` | `archetype_chain`, `fr`, `nr`, `qh` | idem sans | 23 |

   Aucun ÉCHEC. C4 est verte au parent : le `pn` de `ga` n'a que le test ponctuel pour filet ; le jeu ne le lit pas (`pn` n'est lu que par `g_:mm`), d'où 26 e2e.

4. **Couverture** : `gcov` du TU (`gcc --coverage -O0` avec les 60 `ENGINE_SRCS`, `gcov` lancé depuis la racine de la copie). Mesuré sur `e2` : **384 / 385** lignes (99,74 %). Seule ligne non exécutée : le `fprintf` d'échec de `same`. Les six fonctions et `same_emitter_part` sont exécutées.

5. **Objets** (sans `-g`, `gcc -std=c99 -Wall -Wextra -Werror -Isrc -c`, parent puis commit, 155 `.c` compilables sans raylib vendorisé — `main_raylib.c`, `main_screencast_viewer.c`, `pico8_raylib_backend.c` échouent des deux côtés —, `-DPICO8_CLASS_COVERAGE` pour `main_class_coverage.c` et `test_class_coverage.c`). Mesuré, à chaque commit :
   - **153 identiques** octet à octet, dont tous ceux qui incluent `game_data.h` (le membre `kw_own` retiré ne change aucun autre objet : seul `game_data.c` nomme `pico8_jo_def_t`) ;
   - **2 diffèrent** : l'objet du TU et `pico8_game_data.o`. `nm` ne gagne que `pico8_hj_def`, `pico8_mx_def`, `pico8_pm_def` (e1), puis `pico8_fr_def`, `pico8_ga_def`, `pico8_nr_def` (e2). `objdump -d` ne diffère que par deux immédiats `__LINE__` (`0x34b` → `0x39b`, `0x354` → `0x3a4` à e1 ; `→ 0x3e7`, `0x3f0` à e2). Sous `-DNDEBUG`, `.text` **identique** (405 octets).

6. **Règle 1, preuve `const`** (copie de `e2`, qui contient e1 ; chaque membre d'instance que les valeurs neuves décrivent rendu `const` dans sa struct, un à la fois — `*const` pour un pointeur ; `gcc -fsyntax-only` des 155 `.c` ; erreurs listées). Mesuré : **21 membres, 91 sites**, tous dans `src/`, aucun dans `tests/`, exactement les sites de l'annexe A (`probe.c` décalé de −1) :
   - propres (10 membres, 16 sites) : `hj.rr` 2 (`hj_struct_init`, `mx_struct_new`), `pm.ng` 1, `pm.fz` 1, `ga.fz` 1, `ga.iu` 2 et `ga.ms` 2 (défaut puis clé `pq`, I), `nr.hz` 2, `nr.rr` 2 (`nr_struct_init`, `fr_struct_new`), `fr.mt` 1, `mx.ht` 2 (`mx_struct_new`, et `planet_family_trade.c:326 mx_kf` : I) ;
   - émetteur, rejoué (11 membres, 75 sites) : `pn` 7, `du` 7, `py` 8, `ki` 6, `pk` 6, `fc` 6, `k_` 7, `qo` 7, `ro` 7, `qg` 7, `q_` 7 — `help.c` (`fv`), `planet_family.c` (`hj`, `pm`, `ga`, dont les clés `pq`), `planet_family_trade.c` (`mx`), `probe.c` (`g_`).
   Les membres `jo`, `qh` et sonde ont eu leur preuve à 2.12d (29 membres dont `kw_own` : ce dernier n'est plus un champ d'archétype, sa ligne ne se rejoue pas). Tout autre site est un arrêt « champ sans règle ». Piège : `hj.rr`, `pm.ng`, `pm.fz`, `ga.fz`, `nr.hz`, `nr.rr`, `mx.ht`, `k_` sont des pointeurs, à rendre `*const` ; `pico8_emitter_t` est `typedef struct pico8_emitter {` (`particle.h`).

7. **Inclusion, casts, aiguillage** : `inclusion-mutuelle` **17 / 16**, **43 lignes** aux deux commits ; greps de Sortie 4 et 5. Le TU inclut `planet_family_trade.h` : `tests/` n'entre pas dans le compte.

8. `class-coverage-net` : **non**. `sanitize-net` : **non**.

### Greps de Sortie (arbre de chaque commit)

1. `grep -c 'const pico8_[a-z_]*_def_t pico8_[a-z_]*_def = {' src/pico8_game_data.c` → **22** (e1), **25** (e2).
2. `grep -c 'pico8_jo_def_t' src/pico8_game_data.h` → **8** (e1), **11** (e2) ; `grep -rlwE 'pico8_jo_def_t|pico8_(jo|qh|nl|os|hj|pm|mx|ga|nr|fr)_def' src tests` → `src/pico8_game_data.c`, `src/pico8_game_data.h`, `tests/test_pico8_archetype_chain.c`, rien d'autre.
3. `grep -c kw_own src/pico8_game_data.c` → **0** ; `grep -n kw_own src/pico8_game_data.h` → la seule ligne du commentaire ; `git diff <parent> -- src | grep -E '^\+.*_start'` → vide.
4. `grep -rnE '\bdef\b *;|->def\b|\.def\b' src` → vide.
5. `git diff <parent> | grep -E '^\+.*\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)'` → vide ; `git diff <parent> | grep -E '^\+.*(->cls|header\.cls|_struct_class\(\)|emitter_of|probe_of|pico8_g__of|pico8_qh_of|pico8_nl_of|_is_family)'` → vide.
6. `make composition-residue` → pied 1073 / 85, `défauts 669 52`, `inclusion-mutuelle 17 16` ; `--champs` → `282 ; 15 ; 20` ; `wc -l src/pico8_game_data.h` → 748 (e1), 751 (e2).
7. `git diff --stat <parent>` : e1 = `src/pico8_game_data.h`, `src/pico8_game_data.c`, `tests/test_pico8_archetype_chain.c`, `docs/etude-2-9-archetypes.md`, `docs/plan-3.md` ; e2 = les trois premiers et `docs/plan-3.md`. Rien d'autre.
8. e2 : la liste des 26 archétypes et des 14 tables sans archétype (plus haut) se retrouve.

### Tableau « Runs »

Sur la copie jetable (`prd212e/tree`). `p` = 2.12d appliquée d'après son PRD sur `a7c0819` (`6436df0` dans la copie), puis `f9c0297` ; `e1`, `e2` = ce PRD commité dans la copie.

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout a7c0819` ; texte C de 2.12d extrait de son PRD et appliqué (script, 7 blocs) | — | 0 | 3 fichiers, 238 insertions, 1 suppression (après une ligne vide oubliée par le script) ; `game_data.h` 745 | 3 s |
| 2 | `timeout 900 make -j4 test` ; `make e2e` ; compteur ; `--champs` ; `--check` | `p` | 0 | `all tests passed` ; 26 ; 1073 / 85 ; 669 ; 43 ; 282 / 15 / 19 | 15 s |
| 3 | 1er jet e1 (avec `kw_own`) : sonde de `qy` (`fprintf` provisoire) | `e1`₀ | 0 | `hj`, `pm`, `mx` : `qy` non NULL, 0 élément ; 351 vertes | 15 s |
| 4 | 1er jet e1 : `make test`, `make e2e`, compteur, `--champs`, jointure `p`/`e1` | `e1`₀ | 0 | vert ; 26 ; 669 ; 43 ; 19, aucun changement de groupe | 20 s |
| 5 | 1er jet e2 : `make test` | `e2`₀ | 2 | `same_emitter_part: Assertion 'emitter->qy != NULL && emitter->qy->count == 0'` : `ga` a 140 particules (`ez` dans `ga_pq`) | 10 s |
| 6 | `qy` sorti de `same_emitter_part`, assert par sorte ; e1, e2 réappliqués ; `make test`, `make e2e` | `e1`₀/`e2`₀ | 0 | vert ; 26 ; 440 | 30 s |
| 7 | mutations et jumelles, 1er passage | `e1`₀/`e2`₀/`p` | 0 | regex des TU rouges fausse (`] Aborted`, pas `] Error`) : arrêté, relancé | 20 s |
| 8 | M, C, X (témoin `kw_own`) sur `e2`₀, `e1`₀, `p` | idem | 0 | aucun ÉCHEC ; C11 ne compile pas au parent (`pm_fz_text` inutilisé) ; C4 e2 vert au parent | ≈ 9 min |
| 9 | C11 → `pm_fz_text + 1`, rejouée au commit et au parent | `e1`₀/`p` | 0 | `archetype_chain`, `pm` / `pm` | 20 s |
| 10 | `constproof.py`, 21 membres | `e2`₀ | 0 | 91 sites, ceux de l'annexe A | 13 s |
| 11 | objets sans `-g` et `-DNDEBUG`, `p`/`e1`₀/`e2`₀ | — | 0 | 153 identiques ; `game_data.o` : 3 symboles, `__LINE__` ; `.text` 405 identique | 90 s |
| 12 | consigne du pilote (constat du vérificateur de 2.12d : `kw_own` est un E) ; copie remise sur `f9c0297` (`git fetch` local) ; `git diff 6436df0 558b87f -- src tests` | — | 0 | vide : 2.12d appliquée = 2.12d commitée | 3 s |
| 13 | `p` = `f9c0297` : `make test`, `make e2e`, `--champs`, `wc -l` | `p` | 0 | vert ; 26 ; 282 / 15 / 19 ; 745 | 12 s |
| 14 | retrait de `kw_own` + e1 (texte de ce PRD) ; `make test`, `make e2e`, compteur, `--champs`, jointure `p`/`e1` ; commit | `e1` | 0 | vert (350) ; 26 ; 1073 / 85 ; 669 ; 43 ; **282 / 15 / 20** ; seul `pico8_jo_t kw_own` change de groupe (5 / 4 → 1 / 0) ; 748 | 25 s |
| 15 | e2 (texte de ce PRD) ; mêmes mesures ; jointure `e1`/`e2` ; commit | `e2` | 0 | vert (438) ; 26 ; 1073 / 85 ; 669 ; 43 ; 20, aucun changement de groupe ; 751 | 25 s |
| 16 | M1-M14 e1 et e2 ; C1-C12, K1, H1 au commit e1 et à `p` ; C1-C12 e2 au commit e2 et à `e1` | `e1`/`e2`/`p` | 0 | tableaux ci-dessus ; aucun ÉCHEC | ≈ 8 min |
| 17 | objets sans `-g` et `-DNDEBUG`, `p`/`e1`/`e2` | — | 0 | 153 identiques à chaque paire ; `.text` 405 identique ; `__LINE__` +80, +76 | 90 s |
| 18 | `gcc --coverage` du TU + 60 `ENGINE_SRCS`, `gcov` depuis la racine | `e2` | 0 | 384 / 385 lignes | 30 s |
| 19 | `constproof.py` rejoué, 21 membres | `e2` | 0 | 91 sites, 0 dans `tests/` | 13 s |
| 20 | greps de Sortie ; liste des `_def` et des 40 tables | `e1`/`e2` | 0 | 22 / 25 valeurs + `hy` = 26 ; 14 tables sans archétype = l'inventaire | 3 s |
| 21 | `grep -rn kw_own src tests` | `e2` | 0 | lu hors `planet.c` par `archetype_chain` et `jo_spec_defaults:74`, `:230` seulement | 1 s |
| 22 | `rm -rf` de la copie | — | 0 | copie et journaux supprimés (tout `prd212e/`) | 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas `f9c0297` ou un descendant qui ne touche que `docs/`, ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) est rouge à l'un des deux commits, si un `*.sha256` ou `tests/e2e/` change, ou si une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si le TU ne compte pas **350** à e1 (`jo.kw_own` retiré, `hj` 39, `pm` 41, `mx` 40) et **438** à e2 (`ga` 29, `nr` 29, `fr` 30), si `kw_own` est comparé sur une sorte, ou si une comparaison lit un `pico8_X_start`.
- `FAILED:` si `kw_own` reste dans `pico8_jo_def_t` ou une valeur après e1, si un membre est ajouté au type, ou s'il existe plus d'un type d'archétype planète.
- `FAILED:` si une mutation M est verte au commit, ou rougit un autre TU que `archetype_chain` ou un e2e.
- `FAILED:` si une jumelle C est verte au commit et rouge au parent. (K1, rouge au parent par `archetype_chain` seulement et encore rouge au commit par `hp`/`jo`, est la preuve voulue du retrait.)
- `FAILED:` si un objet autre que `game_data` et le TU diffère sans `-g`, ou si `game_data` diffère sous `-DNDEBUG` dans son `.text`.
- `FAILED:` si la preuve `const` trouve, pour les 21 membres, un site hors de l'annexe A (aux décalages près).
- `FAILED:` si `défauts` n'est pas 669, `inclusion-mutuelle` 17 / 16 (43 lignes), le pied 1073 / 85, ou `--champs` autre que 282 / 15 / 20 sans explication par la même lecture.
- `FAILED:` si, à e2, un des 26 archétypes de l'étude manque, ou si une des 14 tables sans archétype en reçoit un.
- `FAILED:` si un `def` entre dans une instance ou un composant, si un en-tête de classe (`planet.h`, `planet_family.h`, `planet_family_trade.h`, `particle.h`, `probe.h`) change, ou en cas de tassement.
- `FAILED:` si un commentaire ajouté ne porte ni un contrat ni un piège (`CLAUDE.md`).
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si un commit embarque une ligne étrangère à sa sous-étape (`docs/plan-3.md` hors de sa ligne, la ligne 2.12d, `tools/`, un autre TU), ou si l'étude change à e2.
- `FAILED:` si le message d'un commit omet : ses valeurs, sa répartition des comparaisons et son total, l'écart au plan et sa raison, les mutations M, les jumelles C (et K1, H1 à e1) avec leur résultat au parent, la couverture (e2), les objets, la preuve `const`, les comptes dont `--champs` (19 → 20 à e1 et pourquoi), le tableau « Runs » ; à e1 en plus le retrait de `kw_own` et la correction de l'étude ; à e2 le contrôle des 26 archétypes.
- `FAILED:` s'il manque la note « Découvert à 2.12e1 » ou « Découvert à 2.12e2 », ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Le membre `def` des instances, toute lecture de l'archétype, la suppression des `pico8_X_start` et de `pico8_jo_kw_start` : 2.13.
- Le TU durable qui garde `probe.has_ik` de l'arbre `qh` : 2.13 (contrat ci-dessus).
- La note de la ligne 2.12d du plan : celle du pilote ; e1 écrit la correction sur sa propre ligne.
- `tools/composition_residue.sh` ; `class-coverage-net`, `sanitize-net` ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Le retrait de `kw_own` va dans e1**, pas dans un commit à part : la consigne demande deux commits et permet « avant e1, ou dans e1 ». Il passe avant les trois valeurs dans le même commit, pour qu'aucune valeur neuve ne naisse avec `kw_own`. Repli e1a/e1b s'il dépasse le budget.
- **G2. `qy` hors de `same_emitter_part`** : `hj`, `pm`, `mx` ont une liste vide, `ga` 140 particules (mesuré, run 5). Un `assert` par sorte, pas dans l'aide.
- **G3. `ga.by` comparé par deux instances** (non NULL et identique), pas par nom : `g_ga_by_shared` est `static` dans `planet_family.c` et n'a pas d'accesseur ; en ajouter un toucherait un en-tête de classe.
- **G4. Mutations choisies** pour couvrir les masques de chaque niveau (en-tête, `jo`, `qh`, `hj`, `nr`, émetteur, sonde), adresse, texte, boîte, vecteur par valeur, drapeau et comparaison oubliée ; M1/M2 sont celles de l'étude §3. Jumelles : un littéral `_start` et une affectation d'init par niveau, K1 pour le retrait de `kw_own`, H1 pour le filet de `has_ik`.
- **G5. La correction de l'étude** touche le classement, les comptes et les phrases qui disent le contraire ; pas les Runs (histoire). Une ligne en tête de l'annexe B plutôt que dix retouches.
- **G6. Commentaires** : type — un contrat (`kw_own` hors archétype) ; TU — `same_jo_part` et `same_emitter_part` (ce qu'elles ne comparent pas), `assert_lz_ran` (ce que `lz` fait), `test_hj` et `test_ga` (d'où vient `qy`, pourquoi `kw`/`by` sont hors archétype).

### Surprises de mesure, pour `plan-pilot`

1. **`kw_own` est un E** (vérificateur de 2.12d) : 2.12d avait 100 comparaisons dont une de trop ; TU 231 → 230 ; `--champs` revient à 20.
2. **`ga` sort de `_new` avec 140 particules** dans `qy` : `ga_pq` appelle `ez` ; « `qy` tel que `f_` le laisse » (étude §3) ne vaut que pour `hj`, `pm`, `mx`.
3. **Deux littéraux n'ont que le test ponctuel pour filet** : `probe.has_ik` de l'arbre `qh` (H1, à garder en 2.13) et le `pn` de `ga` (C4, jamais lu par le jeu).
4. Piège d'essai : une jumelle qui rend un texte `static` inutilisé ne compile pas au parent (`-Werror`) ; muter par `+ 1` sur le même texte.
5. Toutes les autres jumelles de la famille e ont un filet au parent.

### Questions pour l'humain

Aucune. Le retrait de `kw_own` suit un constat vérifié et la décision du pilote ; le reste suit l'étude.
