---
title: "Plan 3, étape 2.12c : l'archétype sonde `pico8_g__def_t` (valeurs de `g_` et `fv`), le sous-archétype émetteur `pico8_emitter_def_t`, et 55 comparaisons de plus au test ponctuel (total 131 ; `défauts` 669, pied inchangé) — un commit, jeu identique"
description: "Un seul type sonde pour `g_` et `fv` (décision A de l'étude 2.9), deux valeurs, et le type du sous-archétype émetteur, enchâssé avec le sous-archétype sonde. Personne ne les lit. Le test ponctuel gagne `test_g_` (23) et `test_fv` (32), total asserté 76 → 131. Au passage, les deux réserves mineures du vérificateur de 2.12a/b. Le `def` du composant émetteur reste à 2.13c."
status: needs-triage
---

## Problem Statement

**Écrit sur `232da49`** (branche `claude/plan-pilot-3-7zdft0`). Aucune ligne du dépôt n'a changé pour écrire ce PRD. Tous les essais ont été faits sur une seule copie jetable de `232da49` (scratchpad `prd212c/`, supprimée depuis) : `p` = `232da49`, `c` = le texte de « Ce qui change » appliqué et commité dans la copie.

Vu du mainteneur du port : 2.12a a créé les archétypes des racines simples et le test ponctuel, 2.12b ceux du trou de ver et de la voie avec le sous-archétype sonde. 2.12c fait de même pour la sonde (`g_`) et sa variante d'aide (`fv`) : **un seul type d'archétype pour les deux** (décision A de l'humain, étude 2.9 §2 : « un type d'archétype par futur type aplati »), deux valeurs, et le **type du sous-archétype émetteur**, que 2.12e1/e2 (planètes `hj`, `pm`, `mx`, `ga`) enchâsseront tel quel.

### Texte du plan, recopié (`docs/plan-3.md` à `232da49`)

> - 2.12 — archétypes et **test ponctuel de la règle 3**, selon l'étude 2.9 […]. Il compare champ par champ les champs A et I d'un `pico8_X_new(NULL)` frais à `pico8_X_def`, groupes de Q7 compris, avec les trois exceptions de l'étude, §3. Aucune lecture ne bascule encore, et aucun `def` n'entre dans les instances. Compte : `défauts` **669** à chaque sous-étape. […]
>   - 2.12c — `g_`, `fv`, type sonde `pico8_g__def_t` et `pico8_emitter_def_t` : 55 comparaisons, groupes émetteur et sonde.

Note « Découvert à 2.10 » : « leurs champs A se lisent par un `def` dans le composant, posé par l'init du porteur à `&def->probe`/`&def->emitter` ; ce `def` naît à 2.12b/2.13b (sonde) et **2.12c/2.13c (émetteur)** ». Note « Découvert à 2.12b » : « `--champs` "0 hors constructeur" 27 → 23, attendu […] ; 2.12c à 2.12e2 le feront bouger de même (`probe`, `emitter`) ».

Règles communes : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée.

### Ce que 2.12c place, et ce qu'il laisse à 2.13c

Même lecture que G1 de 2.12b (plan : « aucun `def` n'entre dans les instances » ; étude 2.10 §2 : « **2.12c**/**2.13c** pour l'émetteur ») :

| Élément | Sous-étape | Pourquoi |
|---|---|---|
| type `pico8_emitter_def_t`, enchâssé en membre `emitter` de `pico8_g__def_t` | **2.12c** (ce PRD) | le test ponctuel compare le groupe émetteur de Q7 de `g_` et `fv` : il lui faut l'archétype |
| type `pico8_g__def_t`, valeurs `pico8_g__def` et `pico8_fv_def` | **2.12c** | idem, groupes émetteur et sonde, et les champs propres |
| membre `const pico8_emitter_def_t *def` dans `pico8_emitter_t`, posé à `&def->emitter` par l'init du porteur | **2.13c** | membre de composant posé par une init : interdit en 2.12 |
| un sous-archétype émetteur nul | **jamais** | aucune instance d'émetteur hors d'un porteur : `grep -rn 'pico8_emitter_t [a-z_]* *[=;]' src tests` ne rend que les `emitter_own` des trois types porteurs (`probe.h:50`, `planet_family.h:75`, `:107`) ; la sonde locale de `research_icon.c:180` n'a pas d'émetteur |

**Contrat laissé à 2.13c** (à recopier dans son PRD) : `particle.h` inclut déjà `game_data.h` (`particle.h:5`), donc le membre `def` de `pico8_emitter_t` n'ajoute aucune inclusion. Entre 2.13c et 2.13e1, les autres porteurs d'émetteur (`hj_struct_init`, `ga_struct_new`) font `malloc` sans mise à zéro : leur `emitter->def` doit être posé (NULL ou archétype), jamais indéterminé, et aucun lecteur générique (`particle.c:149`, `fc`) ne bascule sur `emitter->def` avant 2.13e2.

### Question ouverte chez l'humain (Q-2.10, liens du noyau)

**2.12c n'en dépend pas.** Elle n'ajoute aucun lien au noyau ; le lien `emitter` du noyau est 2.15, et le `def` du composant (2.13c) est dans `pico8_emitter_t`, pas dans le noyau. Une réponse « non » change 2.14/2.15, pas les archétypes de 2.12c.

### Travail en parallèle : 2.11r-d (déclaration des recouvrements)

Un implémenteur applique 2.11r-d en même temps (`tools/composition_residue.sh`, `tests/test_composition_residue.c`, `tests/test_pico8_jc.c`, une ligne de `docs/plan-3.md`).

| Fichier | 2.11r-d | 2.12c | Recouvrement |
|---|---|---|---|
| `src/pico8_game_data.[ch]`, `tests/test_pico8_archetype_chain.c` | — | oui | aucun |
| `docs/plan-3.md` | ligne 2.11r-d | ligne 2.12c (note « Découvert à ») | **même fichier, lignes distinctes** : commiter avec `git add -p docs/plan-3.md` (seul le bloc 2.12c), jamais `git commit -a` ; si 2.11r-d a déjà commité, rebaser d'abord |
| `tools/composition_residue.sh` | oui | lu (`--champs`, `--check`) | les comptes de 2.12c se mesurent **au script du parent et de l'enfant, le même** ; si 2.11r-d change le script entre-temps, remesurer le parent avant de comparer |
| `tests/test_pico8_jc.c` | oui | lu par la preuve `const` (§ Preuves, 6) | le site `probe.dw` de `test_bv_takes_the_first_slipway_of_cg` (`:893` à `232da49`) vient de 2.11r-d (`63df46c`) ; s'il bouge ou si un autre TU de `jc` écrit `dw`, le compte de `dw` suit, sans ÉCHEC tant que le site est un TU (réécrit par 2.13b) |
| `tmp/` (objets, binaires) | `make test` | `make test`, mutations | **ne jamais lancer deux `make` dans le même arbre en même temps** : les mutations de 2.12c réécrivent des `.o` partagés ; travailler sur une copie ou attendre la fin de l'autre |

### État de départ attendu (`232da49` ou un descendant qui ne touche que les fichiers de 2.11r-d ou `docs/`)

| Commande | Résultat mesuré à `232da49` |
|---|---|
| `make composition-residue` | `défauts 669 52`, `inclusion-mutuelle 17 16`, pas de `registres` ; pied **1073 / 85** |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 282 ; 0 écriture : 15 ; 0 hors constructeur : 23` |
| `sh tools/composition_residue.sh --check inclusion-mutuelle \| tail -1` | `résidu gardé (inclusion-mutuelle) : 43 ligne(s)` |
| `wc -l src/pico8_game_data.h` | 666 |
| `grep -c 'g_compared == 76' tests/test_pico8_archetype_chain.c` | 1 |
| `grep -rnwE 'pico8_emitter_def_t\|pico8_g__def_t\|pico8_g__def\|pico8_fv_def' src tests` | vide |
| `make test` ; `make e2e` | vert ; 26 conformes |

Un départ autre : arrêt, rapporté.

## Solution

Un seul commit :

- **Deux types** dans `pico8_game_data.h`, après `pico8_jc_def_t` : `pico8_emitter_def_t` (les onze champs A et I de l'émetteur, dans l'ordre de `pico8_emitter_t`, sans `qy`) et `pico8_g__def_t` (`me`/`t`, les champs de `g_` dans l'ordre de l'instance, `has_ra`, les neuf de `fv` dans l'ordre de l'instance, puis `emitter` et `probe`, dans l'ordre des membres de `pico8_g__t`).
- **Deux valeurs** `const` non `static` dans `pico8_game_data.c`, `pico8_g__def` et `pico8_fv_def`, du même type, déclarées `extern`. Résolues, en littéraux, sans nommer aucun `_start`.
- **Réserves du vérificateur de 2.12a/b** : le commentaire « A probe's own defaults… » (paraphrase du type) part ; l'en-tête du TU perd son récit de chantier.
- **TU** `tests/test_pico8_archetype_chain.c` : `test_g_` (23), `test_fv` (32), une aide `same_vec_or_nil`, deux inclusions, total asserté **131**.
- `docs/plan-3.md` : note « Découvert à 2.12c ».

Rien d'autre. Aucun `def` dans une instance ni dans un composant. Aucun `_new`, init ou `_start` ne change. Aucune lecture ne bascule. Pas de `Makefile`.

## User Stories

1. En tant qu'auteur de 2.13c, je veux les archétypes de `g_` et `fv` prouvés égaux à la chaîne, émetteur et sonde compris, afin que leur init lise l'archétype sans changer une valeur.
2. En tant qu'auteur de 2.12e1/e2, je veux un type `pico8_emitter_def_t` déjà là, dans l'ordre de `pico8_emitter_t`, afin de l'enchâsser dans l'archétype planète sans le redéfinir.
3. En tant que gardien de la décision A, je veux **un seul** type d'archétype pour `g_` et `fv`, afin que 2.15 aplatisse la sonde sur un seul `def`.
4. En tant que gardien de la règle 3, je veux les deux groupes de Q7 de `g_` et `fv` au test ponctuel, `qy` et `et` à part (E, vérifiés hors compte).
5. En tant que gardien du test ponctuel, je veux le total asserté à 131.
6. En tant que gardien du compteur, je veux `défauts` 669, `inclusion-mutuelle` 17 / 16, le pied inchangé, et `--champs` expliqué.
7. En tant que gardien de la règle 1, je veux la preuve `const` des 23 membres neufs (et des sept de la sonde, rejoués), sur la copie du commit.
8. En tant que gardien du jeu, je veux 26 e2e conformes et aucun objet de `src/` changé hors `game_data`.
9. En tant que vérificateur, je veux des mutations d'archétype et des jumelles de chaîne rejouées au parent.
10. En tant que lecteur des commentaires, je veux les deux réserves de 2.12a/b corrigées selon `CLAUDE.md`.

## Implementation Decisions

### Modules

`game_data` ; TU `archetype_chain` ; `docs/plan-3.md`. Aucun autre.

### Ce qui change, fichier par fichier (ancres de texte ; numéros de `232da49`)

**`src/pico8_game_data.h`**

1. Retirer la ligne `:348` (réserve du vérificateur ; le nom du type dit ce qu'elle disait) :

   ```c
   /* A probe's own defaults, in the archetype of each class that carries one. */
   ```

2. Après `} pico8_jc_def_t;` (`:376`), avant la ligne vide qui précède « The jo tree… », insérer :

   ```c

   typedef struct {
       pico8_fixed_t pn;
       pico8_fixed_t du;
       pico8_fixed_t py;
       pico8_fixed_t ki;
       pico8_fixed_t pk;
       pico8_fixed_t fc;
       const pico8_int_seq_t *k_;
       pico8_fixed_t qo;
       pico8_fixed_t ro;
       pico8_fixed_t qg;
       pico8_fixed_t q_;
   } pico8_emitter_def_t;

   /* g_ and fv share this type; fv's own members are zero in g_'s. */
   typedef struct {
       const char *me;
       pico8_fixed_t t;
       pico8_fixed_t fx;
       pico8_fixed_t fa;
       pico8_vec_t ra;
       int has_ra; /* 0: the instance's `ra` is NULL */
       pico8_vec_t hi;
       pico8_fixed_t ca;
       pico8_vec_t gq;
       pico8_fixed_t eo;
       pico8_fixed_t ff;
       pico8_qz_box_t kq;
       pico8_fixed_t ql;
       pico8_fixed_t fg;
       const pico8_jb_row_t *rb;
       pico8_emitter_def_t emitter;
       pico8_probe_def_t probe;
   } pico8_g__def_t;
   ```

3. Après `extern const pico8_jc_def_t pico8_jc_def;` (`:636`) :

   ```c
   extern const pico8_g__def_t pico8_g__def;
   extern const pico8_g__def_t pico8_fv_def;
   ```

`game_data.h` : 666 → **702** lignes.

**`src/pico8_game_data.c`** — après la valeur `pico8_jc_def` (`:525-532`), avant `static const char jo_me_text[]` :

```c

const pico8_g__def_t pico8_g__def = {
    .me = "ph",
    .t = 0x0,
    .fx = 0x10000,
    .fa = 0x20000,
    .has_ra = 0,
    .emitter = {.pn = 0x30000, .du = 0x20000, .py = 0xf5c, .ki = 0x90000, .pk = 0x10000, .fc = 0x10000,
                .k_ = &pico8_g__k__seq, .qo = 0x1051e, .ro = -0xd70, .qg = 0xd70, .q_ = 0x1051e},
    .probe = {.dw = 0x666, .od = 0x0, .has_od = 1, .dq = "ly", .fp = 0x0, .ik = 0x0, .has_ik = 0},
};

const pico8_g__def_t pico8_fv_def = {
    .me = "ph",
    .t = 0x0,
    .fx = 0x10000,
    .fa = 0xa0000,
    .ra = {0x400000, 0x1c0000},
    .has_ra = 1,
    .hi = {0x220000, 0x380000},
    .ca = 0xb0000,
    .gq = {0x0, 0xb0000},
    .eo = -0x10000,
    .ff = 0x10000,
    .kq = {-0x280000, 0x4e0000, 0x280000, 0x620000},
    .ql = 0xb0000,
    .fg = 0x50000,
    .rb = &pico8_fv_rb_row,
    .emitter = {.pn = 0x50000, .du = 0xf0000, .py = 0x1b6, .ki = 0x110000, .pk = 0x10000, .fc = 0x50000,
                .k_ = &pico8_fv_k__seq, .qo = 0x1051e, .ro = -0x1ca, .qg = 0x1ca, .q_ = 0x1051e},
    .probe = {.dw = 0x666, .od = 0x0, .has_od = 1, .dq = "ly", .fp = 0x0, .ik = 0x0, .has_ik = 0},
};
```

Chaque valeur est celle de l'annexe B de l'étude 2.9 (`g_` 23, `fv` 32), recopiée de `pico8_ke_start` (`me`, `t`), de `pico8_g__start`, de `pico8_fv_start` et des littéraux de `pico8_g__struct_init` (`ra = NULL`, `has_od = 1`, `ik = 0`, `has_ik = 0`). `fp` est le faux de la spécification (`int` 0 dans `pico8_g__start_t`), rangé `0x0`. `fx` et la sonde de `fv` sont ceux de `g_` (non masqués) ; `fv` masque `fa`, `ra` et huit champs de l'émetteur, `pn`, `du`, `py`, `ki`, `fc`, `k_`, `ro`, `qg` (étude §3, « Masquages » ; `pk`, `qo`, `q_` y ont la même valeur que `g_`). Dans `pico8_g__def`, les membres propres de `fv` restent à zéro (désignateurs omis), et `ra` aussi, que `has_ra = 0` rend sans objet. `ra` est porté par valeur avec son drapeau (étude 2.9 §2 : « `ra` (vecteur, avec un drapeau : NULL pour `g_`) ») : la valeur de classe de `g_` est un pointeur nul, pas un vecteur.

**`tests/test_pico8_archetype_chain.c`**

1. En-tête (réserve du vérificateur : « Removed with the chains it compares » est un récit de chantier, le plan le dit déjà à 2.13 ; « still » aussi) :

   ```c
   /* Each archetype against the value its class's init chain leaves in a fresh
    * instance, field by field. */
   ```

2. Inclusions, dans l'ordre alphabétique : `#include "../src/pico8_help.h"` après `pico8_gfx.h`, `#include "../src/pico8_probe.h"` après `pico8_msgbox.h`.
3. Après `same_vec` :

   ```c

   static int same_vec_or_nil(const pico8_vec_t *got, const pico8_vec_t *want) {
       return (got == NULL && want == NULL) || (want != NULL && same_vec(got, want));
   }
   ```

4. Avant `int main(void)`, après `test_jc` :

   ```c

   /* `qy` is the fresh particle list `f_` gives every emitter. */
   static void test_g_(void) {
       const pico8_g__t *g_ = pico8_g__new(NULL);

       same(same_text(g_->header.me, pico8_g__def.me), "g_.me");
       same(g_->header.t == pico8_g__def.t, "g_.t");
       same(g_->fx == pico8_g__def.fx, "g_.fx");
       same(g_->fa == pico8_g__def.fa, "g_.fa");
       same(same_vec_or_nil(g_->ra, pico8_g__def.has_ra ? &pico8_g__def.ra : NULL), "g_.ra");
       same(g_->emitter->pn == pico8_g__def.emitter.pn, "g_.emitter.pn");
       same(g_->emitter->du == pico8_g__def.emitter.du, "g_.emitter.du");
       same(g_->emitter->py == pico8_g__def.emitter.py, "g_.emitter.py");
       same(g_->emitter->ki == pico8_g__def.emitter.ki, "g_.emitter.ki");
       same(g_->emitter->pk == pico8_g__def.emitter.pk, "g_.emitter.pk");
       same(g_->emitter->fc == pico8_g__def.emitter.fc, "g_.emitter.fc");
       same(g_->emitter->k_ == pico8_g__def.emitter.k_, "g_.emitter.k_");
       same(g_->emitter->qo == pico8_g__def.emitter.qo, "g_.emitter.qo");
       same(g_->emitter->ro == pico8_g__def.emitter.ro, "g_.emitter.ro");
       same(g_->emitter->qg == pico8_g__def.emitter.qg, "g_.emitter.qg");
       same(g_->emitter->q_ == pico8_g__def.emitter.q_, "g_.emitter.q_");
       same(g_->probe->dw == pico8_g__def.probe.dw, "g_.probe.dw");
       same(g_->probe->od == pico8_g__def.probe.od, "g_.probe.od");
       same(g_->probe->has_od == pico8_g__def.probe.has_od, "g_.probe.has_od");
       same(same_text_or_nil(g_->probe->dq, pico8_g__def.probe.dq), "g_.probe.dq");
       same(g_->probe->fp == pico8_g__def.probe.fp, "g_.probe.fp");
       same(g_->probe->ik == pico8_g__def.probe.ik, "g_.probe.ik");
       same(g_->probe->has_ik == pico8_g__def.probe.has_ik, "g_.probe.has_ik");
       assert(g_->emitter->qy != NULL && g_->emitter->qy->count == 0);
       assert(g_->probe->et == NULL);
   }

   static void test_fv(void) {
       const pico8_fv_t *fv = pico8_fv_new();

       same(same_text(fv->g_.header.me, pico8_fv_def.me), "fv.me");
       same(fv->g_.header.t == pico8_fv_def.t, "fv.t");
       same(fv->g_.fx == pico8_fv_def.fx, "fv.fx");
       same(fv->g_.fa == pico8_fv_def.fa, "fv.fa");
       same(same_vec_or_nil(fv->g_.ra, pico8_fv_def.has_ra ? &pico8_fv_def.ra : NULL), "fv.ra");
       same(same_vec(fv->hi, &pico8_fv_def.hi), "fv.hi");
       same(fv->ca == pico8_fv_def.ca, "fv.ca");
       same(same_vec(fv->gq, &pico8_fv_def.gq), "fv.gq");
       same(fv->eo == pico8_fv_def.eo, "fv.eo");
       same(fv->ff == pico8_fv_def.ff, "fv.ff");
       same(same_box(fv->kq, &pico8_fv_def.kq), "fv.kq");
       same(fv->ql == pico8_fv_def.ql, "fv.ql");
       same(fv->fg == pico8_fv_def.fg, "fv.fg");
       same(fv->rb == pico8_fv_def.rb, "fv.rb");
       same(fv->g_.emitter->pn == pico8_fv_def.emitter.pn, "fv.emitter.pn");
       same(fv->g_.emitter->du == pico8_fv_def.emitter.du, "fv.emitter.du");
       same(fv->g_.emitter->py == pico8_fv_def.emitter.py, "fv.emitter.py");
       same(fv->g_.emitter->ki == pico8_fv_def.emitter.ki, "fv.emitter.ki");
       same(fv->g_.emitter->pk == pico8_fv_def.emitter.pk, "fv.emitter.pk");
       same(fv->g_.emitter->fc == pico8_fv_def.emitter.fc, "fv.emitter.fc");
       same(fv->g_.emitter->k_ == pico8_fv_def.emitter.k_, "fv.emitter.k_");
       same(fv->g_.emitter->qo == pico8_fv_def.emitter.qo, "fv.emitter.qo");
       same(fv->g_.emitter->ro == pico8_fv_def.emitter.ro, "fv.emitter.ro");
       same(fv->g_.emitter->qg == pico8_fv_def.emitter.qg, "fv.emitter.qg");
       same(fv->g_.emitter->q_ == pico8_fv_def.emitter.q_, "fv.emitter.q_");
       same(fv->g_.probe->dw == pico8_fv_def.probe.dw, "fv.probe.dw");
       same(fv->g_.probe->od == pico8_fv_def.probe.od, "fv.probe.od");
       same(fv->g_.probe->has_od == pico8_fv_def.probe.has_od, "fv.probe.has_od");
       same(same_text_or_nil(fv->g_.probe->dq, pico8_fv_def.probe.dq), "fv.probe.dq");
       same(fv->g_.probe->fp == pico8_fv_def.probe.fp, "fv.probe.fp");
       same(fv->g_.probe->ik == pico8_fv_def.probe.ik, "fv.probe.ik");
       same(fv->g_.probe->has_ik == pico8_fv_def.probe.has_ik, "fv.probe.has_ik");
       assert(fv->g_.emitter->qy != NULL && fv->g_.emitter->qy->count == 0);
       assert(fv->g_.probe->et == NULL);
   }
   ```

5. Dans `main`, après `test_jc();`, et l'assert du total remplacé :

   ```c
       test_g_();
       test_fv();
       assert(g_compared == 131 && "families a to c compare 131 fields: one missed or one too many");
   ```

Lecture du TU :
- Émetteur et sonde se lisent par les membres `emitter`/`probe` de l'instance (K, pointent `emitter_own`/`probe_own`), comme tous les lecteurs d'aujourd'hui : aucun cast, aucun `emitter_of`/`probe_of`/`pico8_g__of`. `fv` se lit par son membre `g_`, pas par une conversion.
- `ra` : NULL pour `g_`, vecteur pour `fv` ; `same_vec_or_nil` compare au vecteur de l'archétype seulement si `has_ra`. `hi`, `gq`, `kq` par contenu (`same_vec`, `same_box`, stricts : non NULL), `rb` et `k_` par adresse (objets de données partagés).
- Hors compte, en `assert` nus : `qy` non NULL et vide (E, groupe émetteur de Q7, exception 3 de l'étude : « tel que `pico8_f_` le laisse ») ; `et` NULL (E, sonde).
- Répartition : `g_` 23 = 5 (`me`, `t`, `fx`, `fa`, `ra`) + 11 émetteur + 7 sonde (A 10, I 13) ; `fv` 32 = 5 + 9 propres + 11 + 7 (A 19, I 13). Étude §3 : 10 + 13, 19 + 13.

**`docs/plan-3.md`** — note « Découvert à 2.12c » au bout de la ligne 2.12c (`:445`) : comptes (`défauts` 669, pied inchangé, `--champs` 23 → 20 et pourquoi), `game_data.h` 702 lignes, total du TU 131, jumelle C3 (le test ponctuel seul la voit), et le « pour 2.13c » (le `def` de `pico8_emitter_t`, sans inclusion neuve, posé aussi chez `hj` et `ga` ; pas de sous-archétype nul ; un TU durable pour le `has_od` de `g_`). Le reste va au message de commit. Commit de ce fichier par `git add -p` (voir recouvrements).

Rien d'autre : ni `glossaire-noms.md` (aucun nom minifié neuf ; `has_ra` suit `has_od`), ni `trous-de-portage.md`, ni `Makefile`, ni `tools/`, ni `tests/e2e/`, ni `*.sha256`.

### Comptes, mesurés sur la copie (`p` = `232da49`, `c` = 2.12c)

| Mesure | `p` | `c` |
|---|---|---|
| `make composition-residue` | 1073 / 85 | **1073 / 85**, `--detail` identique hors `info src/pico8_game_data.h 666 → 702 lignes` |
| `défauts` (`--check`) | 669 / 52 | **669 / 52** |
| `inclusion-mutuelle` | 17 / 16, 43 lignes | **17 / 16, 43 lignes** |
| `--champs` | 282 ; 15 ; 23 | **282 ; 15 ; 20** (voir ci-dessous) |
| comparaisons du test ponctuel | 76 | **131** (+55) |
| `game_data.h` | 666 | **702** |

**`--champs` 23 → 20, attendu** (même effet qu'à 2.12b). Le relevé compte par nom de membre les écritures textuelles `.X =` d'un `.c` de `src/`, une ligne hors fonction comptant « hors constructeur » (majorant, dit par le script). Les deux lignes `.emitter = {…` donnent au nom `emitter` 2 écritures hors constructeur ; `emitter` est membre de trois types de `--champs` (`pico8_g__t`, `pico8_ga_t`, `pico8_hj_t` : 3 → 5 écritures, 0 → 2 hors constructeur), qui quittent le groupe « 0 hors constructeur ». Le reste bouge sans changer de groupe (`diff` complet `p`/`c` : `fa` 30 → 32 sur ses porteurs, `fx` 4 → 6, `probe` 6 → 8, `ra` 51 → 52, les neuf de `fv` +1, `me`/`has_od`… +2). Aucun membre neuf (282), aucun ne quitte « 0 écriture » (15). 2.12d (`.probe`) et 2.12e (`.emitter`, `.probe`) le feront encore bouger : leurs PRD doivent l'attendre, pas le garder.

### Anti-tassement

- **Interdit** :
  - deux types d'archétype pour `g_` et `fv` (`pico8_fv_def_t`) : contraire à la décision A ;
  - nommer un `pico8_X_start` dans une valeur d'archétype (`défauts` → 670) ; pointer `ra`, `hi`, `gq`, `kq` sur les vecteurs de `pico8_fv_start` ;
  - ajouter un membre `def` à `pico8_emitter_t`, `pico8_probe_t`, `pico8_g__t` ou `pico8_fv_t`, toucher `particle.h`, `probe.h` ou `help.h`, créer un sous-archétype émetteur nul : c'est 2.13c (ou jamais) ;
  - faire lire l'archétype par un `_new`, une init ou un lecteur ;
  - mettre `qy` dans `pico8_emitter_def_t` ou `et` dans la sonde (E) ;
  - un cast, un `void *`, un `emitter_of`/`probe_of`/`pico8_g__of`, une comparaison de classe dans le TU ou les archétypes ;
  - comparer à un `pico8_X_start` au lieu de `pico8_X_def` ;
  - toucher un `*.sha256`, `tests/e2e/`, une constante d'empreinte, `tools/composition_residue.sh` ou son TU, `tests/test_pico8_jc.c`.

## Implementation Chunks

Un commit, **≈ 35 min** preuves comprises (essai : 10 min d'écriture, ≈ 25 min de preuves ; les mutations prennent ≈ 1 min 30 chacune, dont `make -k e2e`). Budget **50 min**.

1. Coller les deux retouches et le bloc de `game_data.h`, le bloc de `game_data.c`, les cinq retouches du TU.
2. `make test`, `make e2e`, compteur, `--champs`.
3. Preuves (ci-dessous), note du plan, commit, push.

**Repli écrit** (au-delà de 1,5 fois le budget avant le commit) : deux commits verts, chacun avec son total asserté :
- **2.12c1** : réserves (en-tête du TU, commentaire sonde), `pico8_emitter_def_t`, `pico8_g__def_t`, `pico8_g__def`, `same_vec_or_nil`, `test_g_` (total **99**) ;
- **2.12c2** : `pico8_fv_def`, `test_fv` (total **131**).
Les comptes ne changent à aucun des deux, sauf `--champs` : 23 → 20 dès 2.12c1 (`.emitter = {` de `pico8_g__def`) ; `game_data.h` 702 dès 2.12c1. Le repli est écrit dans la note « Découvert à ».

## Testing Decisions

- **Pas de TU neuf** : le test ponctuel gagne deux fonctions. Il ne rejoue jamais un `events.json`.
- **Bon test ici** : la valeur que la chaîne laisse dans une instance fraîche, comparée au seul objet neuf.
- **Prior art** : PRD 2.12a/2.12b (même TU, mêmes preuves) ; règle du parent de D.21 et 2.4 ; preuve `const` de l'étude 2.9, annexe A.

### Preuves

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. Mesuré : `make test` code 0 (`test_pico8_archetype_chain: all tests passed`), 26 conformes.

2. **Test ponctuel** : vert au commit = 131 égalités, `g_compared == 131` = aucune oubliée.

3. **Mutations**, une à la fois, `make -k -j8 unit` puis `make -k e2e`, restauration par `cp` puis `touch`. **Règle du parent** : une mutation verte au commit et rouge au parent est un **ÉCHEC** ; verte aux deux, « chemin jamais atteint ». Mesuré sur la copie :

   *Côté archétype (au parent, ni `pico8_g__def` ni `pico8_fv_def` n'existent : doit être rouge au commit).*

   | # | Mutation | TU rouges | e2e |
   |---|---|---|---|
   | M1 | étude §3 c, défaut propre : `pico8_fv_def.ql` `0xb0000` → `0xc0000` | `archetype_chain` seul (`fv.ql`) | 26 / 26 |
   | M2 | étude §3 c, défaut masqué rendu à l'ascendante : `pico8_fv_def.fa` `0xa0000` → `0x20000` (`g_`) | `archetype_chain` seul (`fv.fa`) | 26 / 26 |
   | M3 | émetteur I de `g_` : `pico8_g__def.emitter.ro` `-0xd70` → `-0xd71` | `archetype_chain` seul (`g_.emitter.ro`) | 26 / 26 |
   | M4 | drapeau : `pico8_g__def.has_ra` `0` → `1` | `archetype_chain` seul (`g_.ra`) | 26 / 26 |
   | M5 | vecteur par contenu : `pico8_fv_def.ra` `{0x400000, 0x1c0000}` → `{0x400000, 0x1d0000}` | `archetype_chain` seul (`fv.ra`) | 26 / 26 |
   | M6 | comparaison oubliée : `same(fv->rb == …)` retirée du TU | `archetype_chain` seul (`g_compared == 131`) | 26 / 26 |
   | M7 | séquence par adresse : `pico8_fv_def.emitter.k_` → `&pico8_g__k__seq` | `archetype_chain` seul (`fv.emitter.k_`) | 26 / 26 |
   | M8 | boîte par contenu : `pico8_fv_def.kq` `yb` `0x620000` → `0x630000` | `archetype_chain` seul (`fv.kq`) | 26 / 26 |
   | M9 | émetteur I de `g_` : `pico8_g__def.emitter.pn` `0x30000` → `0x30001` | `archetype_chain` seul (`g_.emitter.pn`) | 26 / 26 |
   | M10 | émetteur A de `fv` : `pico8_fv_def.emitter.pk` `0x10000` → `0x0` | `archetype_chain` seul (`fv.emitter.pk`) | 26 / 26 |

   « `archetype_chain` seul » et 26 e2e : **personne ne lit l'archétype**.

   *Côté chaîne, jumelles identiques au parent et au commit.*

   | # | Mutation | TU rouges au commit | TU rouges au parent | e2e (commit = parent) |
   |---|---|---|---|---|
   | C1 | `pico8_g__start.ki` `0x90000` → `0xa0000` | `archetype_chain`, `g_` | `g_` | 9 / 26 conformes |
   | C2 | `pico8_fv_start.ql` `0xb0000` → `0xc0000` | `archetype_chain`, `fv` | `fv` | 26 / 26 |
   | C3 | `pico8_g__struct_init` : `self->probe->has_od = 1;` → `0` (littéral de chaîne) | `archetype_chain` seul | **aucun** | 26 / 26 |
   | C4 | `pico8_fv_struct_new` : `self->g_.ra = &pico8_fv_start.ra;` → `&pico8_fv_start.hi` | `archetype_chain`, `fv`, `g_`, `spec_defaults` | `fv`, `g_`, `spec_defaults` | 26 / 26 |
   | C5 | `pico8_g__start.fx` `0x10000` → `0x20000` (hérité, non masqué par `fv`) | `archetype_chain`, `fv`, `g_` | `fv`, `g_` | 26 / 26 |
   | C6 | `pico8_fv_struct_new` : `emitter->ki = pico8_fv_start.ki` → `pico8_g__start.ki` (masque perdu) | `archetype_chain`, `fv`, `g_`, `spec_defaults` | `fv`, `g_`, `spec_defaults` | 24 / 26 |

   Aucun ÉCHEC : au commit, chaque jumelle rougit ce que le parent rougit, plus le test ponctuel. **C3 est verte au parent** (aucun TU, aucun e2e ne voit le `has_od` de la sonde `g_`) : le test ponctuel est le seul filet de ce littéral. Ce n'est pas un ÉCHEC (rouge au commit) ; à noter pour 2.13c, où le test ponctuel de `g_` part avec la chaîne : 2.13c doit garder un TU qui lise `has_od` de `g_` (réécriture de `spec_defaults` contre `def`, plan 2.13).

4. **Couverture** : `gcov` du TU (`gcc --coverage -O0` avec les 60 `ENGINE_SRCS`, lancé depuis la racine de la copie) : **235 / 236** lignes (99,58 %), la seule non exécutée étant le `fprintf` d'échec de `same`. Les deux côtés de `same_vec_or_nil` sont pris (`g_` : NULL/NULL ; `fv` : vecteur).

5. **Objets** (sans `-g`, `gcc -std=c99 -Wall -Wextra -Werror -Isrc -c`, parent puis commit, 155 `.c` sans `raylib.h`, `-DPICO8_CLASS_COVERAGE` pour `main_class_coverage.c` et `test_class_coverage.c`) :
   - **153 identiques** octet à octet, dont tous ceux qui incluent `game_data.h`, `probe.h`, `help.h`, `particle.h` ;
   - **2 diffèrent** : l'objet du TU (attendu) et `pico8_game_data.o`, dont `nm` ne gagne que `pico8_g__def` et `pico8_fv_def`, et dont `objdump -d` ne diffère que par deux immédiats `__LINE__` (`0x2ca` → `0x2ea`, `0x2d3` → `0x2f3`, +32). Sous `-DNDEBUG`, son `.text` est **identique** (405 octets).

6. **Règle 1, preuve `const` rejouée** (copie du commit ; chaque membre rendu `const` dans sa struct, un à la fois — `*const` pour un pointeur —, puis `gcc -fsyntax-only` des 153 `.c` compilables ; erreurs neuves listées par fonction). Mesuré : **30 membres, 133 sites**, tous ceux de l'annexe A de l'étude 2.9 (décalés de −1 dans `probe.c`), plus **un** site neuf :
   - émetteur (11) : `pn` 7, `du` 7, `py` 8 (`fv_struct_new`, `hj_struct_init`, `pm_struct_new`, `ga_struct_new` ×1 ou 2, `mx_struct_new`, `g__struct_init`, plus `g__struct_new` pour les trois clés : I) ; `ki` 6, `pk` 6, `fc` 6 (inits seules, A) ; `k_` 7, `qo` 7, `ro` 7, `qg` 7, `q_` 7 (dont deux dans `ga_struct_new` : I) ;
   - `g_` : `fx` 1 (`g__struct_init`, A) ; `fa` 3 (`fv_struct_new`, `g__struct_init`, `T:g_.c` `test_g__is_a_struct`, que 2.13 réécrit) ; `ra` 3 (`+ probe_host_set_ra_p` : I) ;
   - `fv` : `hi`, `ca`, `gq`, `eo`, `ff`, `kq`, `ql`, `fg`, `rb` 1 chacun (`fv_struct_new`, A) ;
   - sonde, rejouée (7) : `dw` **7**, `od` 12, `has_od` 6, `dq` 4, `fp` 5, `ik` 4, `has_ik` 4. Le septième site de `dw` est `test_pico8_jc.c:893 test_bv_takes_the_first_slipway_of_cg`, **ajouté par 2.11r-d** (`63df46c`) : écriture de TU sur un champ A, du même genre que `test_bv` (`:853`), que 2.13b réécrit sur un archétype de test. Attendu, pas un ÉCHEC.
   Tout autre site est un arrêt « champ sans règle ». Pièges : `pico8_emitter_t` et `pico8_probe_t` sont `typedef struct pico8_X {` ; `ra`, `hi`, `gq`, `kq`, `rb`, `k_` sont des pointeurs à rendre `*const`, pas `const T *`.

7. **Inclusion, casts, aiguillage** : `inclusion-mutuelle` **17 / 16**, **43 lignes** ; greps de Sortie 4 et 5.

8. `class-coverage-net` : **non**. `sanitize-net` : **non**.

### Greps de Sortie (sur l'arbre du commit)

1. `grep -c 'const pico8_[a-z_]*_def_t pico8_[a-z_]*_def = {' src/pico8_game_data.c` → **15** (13 + 2).
2. `grep -c 'pico8_emitter_def_t' src/pico8_game_data.h` → **2** (le type, un membre) ; `grep -c 'pico8_g__def_t' src/pico8_game_data.h` → **3** (le type, deux `extern`) ; `grep -rlwE 'pico8_emitter_def_t|pico8_g__def_t|pico8_g__def|pico8_fv_def' src tests` → `src/pico8_game_data.c`, `src/pico8_game_data.h`, `tests/test_pico8_archetype_chain.c`, rien d'autre ; `grep -c "A probe's own defaults" src/pico8_game_data.h` → 0 ; `grep -c 'Removed with' tests/test_pico8_archetype_chain.c` → 0.
3. `git diff <parent> -- src | grep -E '^\+.*_start'` → vide.
4. `grep -rnE '\bdef\b *;|->def\b|\.def\b' src` → vide.
5. `git diff <parent> | grep -E '^\+.*\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)'` → vide ; `git diff <parent> | grep -E '^\+.*(->cls|header\.cls|_struct_class\(\)|emitter_of|probe_of|pico8_g__of)'` → vide.
6. `make composition-residue` → pied 1073 / 85, `défauts 669 52`, `inclusion-mutuelle 17 16` ; `--champs` → `282 ; 15 ; 20` ; `wc -l src/pico8_game_data.h` → 702.
7. `git diff --stat <parent>` : `src/pico8_game_data.h`, `src/pico8_game_data.c`, `tests/test_pico8_archetype_chain.c`, `docs/plan-3.md`, et rien d'autre (sur la copie, sans `docs/` : 3 fichiers, 150 insertions, 4 suppressions).

### Tableau « Runs »

Celui de ce PRD, sur la copie jetable de `232da49` (`p` = `232da49`, `c` = 2.12c commitée dans la copie).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout 232da49` | — | 0 | copie `p` | 2 s |
| 2 | `timeout 900 make -j8 test` ; `make e2e` ; compteur ; `--champs` ; `--check` | `p` | 0 | 26 conformes ; 1073 / 85 ; 282 / 15 / 23 ; 43 ; 669 | ≈ 70 s |
| 3 | texte de 2.12c appliqué (script, 8 ancres) ; `make -j8 test` ; `make e2e` | `c` | 0 | `all tests passed` ; 26 conformes | ≈ 70 s |
| 4 | compteur ; `--champs` ; `--detail` ; `diff` `p`/`c` | `c` | 0 | 1073 / 85 ; **282 / 15 / 20** ; seule différence `info game_data.h 666 → 702` | 5 s |
| 5 | M1 à M10, premier passage | `c` | — | mon script ne lisait pas `make: *** [unit-run-…] Aborted` : TU rouges non extraits ; arrêté, copie rendue (`git checkout -- .`) | 6 min |
| 6 | M1 à M10, script corrigé | `c` | 2 | `archetype_chain` seul ; 26 conformes ; M8 : ancre double (`kq` aussi dans `pico8_fv_start`) | ≈ 15 min |
| 7 | M8 (ancre allongée), C1 à C6 au commit, puis `checkout p`, C1 à C6 | `c` / `p` | 2 | tableau des jumelles ; aucun ÉCHEC ; C3 verte au parent | ≈ 20 min |
| 8 | `constproof.py`, 30 membres | `c` | 0 | 133 sites, annexe A + `test_pico8_jc.c:893` | ≈ 6 min |
| 9 | objets sans `-g`, `p` puis `c` ; `cmp`, `nm`, `objdump -d` ; `-DNDEBUG` `.text` de `game_data` | `p` / `c` | 0 | 153 identiques ; `game_data.o` : 2 symboles, `__LINE__` ; `.text` identique (405) | 90 s |
| 10 | `gcc --coverage` du TU + `ENGINE_SRCS`, lancé hors racine puis depuis la racine ; `gcov -t -b` | `c` | 0 | 1er lancement : `pico8/gfx.json` introuvable (abort) ; 2e : 235 / 236 lignes | 60 s |
| 11 | greps de Sortie | `c` | 0 | conformes (grep 2 corrigé en `-w` : `test_fv_defaults_and_ready` de `spec_defaults` sortait sous le motif large) | 3 s |
| 12 | `rm -rf` de la copie | — | 0 | copie supprimée | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas `232da49` ou un descendant qui ne touche que les fichiers de 2.11r-d ou `docs/`, ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) est rouge, si un `*.sha256` ou `tests/e2e/` change, ou si une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si le TU ne compte pas 131 comparaisons (55 de plus : `g_` 23, `fv` 32), ou si une comparaison lit un `pico8_X_start`.
- `FAILED:` s'il existe deux types d'archétype pour `g_` et `fv`.
- `FAILED:` si une mutation M1 à M10 est verte au commit, ou rougit un autre TU que `archetype_chain` ou un e2e.
- `FAILED:` si une jumelle C1 à C6 est verte au commit et rouge au parent.
- `FAILED:` si un objet autre que `game_data` et le TU diffère sans `-g`, ou si `game_data` diffère sous `-DNDEBUG` dans son `.text`.
- `FAILED:` si la preuve `const` trouve un site hors de l'annexe A (aux décalages près) autre que `test_pico8_jc.c` `test_bv_takes_the_first_slipway_of_cg`, ou une écriture hors construction d'un champ A.
- `FAILED:` si `défauts` n'est pas 669, `inclusion-mutuelle` 17 / 16 (43 lignes), le pied 1073 / 85, ou `--champs` autre que 282 / 15 / 20 sans explication par la même lecture.
- `FAILED:` si un `def` entre dans une instance ou un composant, si `particle.h` change, ou en cas de tassement.
- `FAILED:` si l'une des deux réserves (en-tête du TU, commentaire sonde) reste, ou si un commentaire ajouté ne porte ni un contrat ni un piège (`CLAUDE.md`).
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si le commit embarque une ligne de 2.11r-d (`docs/plan-3.md` hors ligne 2.12c, `tools/`, `tests/test_pico8_jc.c`, `tests/test_composition_residue.c`).
- `FAILED:` si le message du commit omet : le type sonde unique et ses deux valeurs, le sous-archétype émetteur, la répartition des 55, les mutations M et les jumelles C avec leur résultat au parent (C3 comprise), la couverture, les objets, la preuve `const` (comptes, site de 2.11r-d), les comptes dont `--champs` 23 → 20 et sa raison, les deux réserves corrigées, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.12c », ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Le membre `def` de `pico8_emitter_t`, toute lecture de l'archétype, la suppression de `pico8_g__start`/`pico8_fv_start` : **2.13c** (contrat plus haut).
- Le type planète et ses valeurs (`jo` à `fr`, dont `hj`, `pm`, `mx`, `ga` qui enchâsseront `pico8_emitter_def_t`) : 2.12d à 2.12e2.
- Un TU qui garde le `has_od` de `g_` après 2.13c (C3) : 2.13c.
- `tools/composition_residue.sh` (`--champs` se dit majorant) ; `class-coverage-net`, `sanitize-net` ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Un seul type `pico8_g__def_t`**, deux valeurs : décision A de l'humain (étude 2.9 §2), rappelée par la consigne. Les membres propres de `fv` sont à zéro dans `pico8_g__def`, que rien ne lit (seul `fv` a ces champs).
- **G2. `ra` par valeur avec `has_ra`**, pas un pointeur : l'archétype porte ses vecteurs par valeur (étude §2) et la valeur de classe de `g_` est NULL. Même forme que `has_od`/`has_ik` de la sonde.
- **G3. Ordre des membres** : ordre de l'instance (`g_` : `fx`, `fa`, `ra` ; `fv` : ses neuf ; puis `emitter`, `probe` comme dans `pico8_g__t`). Le texte de l'étude (« `me`, `t`, `fa`, `fx`, `ra`… ») n'est pas normatif, comme G2 de 2.12b. `pico8_emitter_def_t` suit `pico8_emitter_t` sans `qy`, comme le texte C de l'étude.
- **G4. Pas de sous-archétype émetteur nul** : aucun émetteur hors d'un porteur (grep plus haut). Si 2.13c en trouve un, il le crée là.
- **G5. Réserves du vérificateur** : le commentaire sonde part sans remplaçant (le nom du type le dit) ; l'en-tête du TU garde sa phrase de contrat au présent, sans « still » ni « Removed with… » (le plan, ligne 2.13, porte le sort du TU). Pas de commentaire neuf sur `pico8_emitter_def_t`, pour la même raison. Les deux commentaires neufs sont des contrats : ce que valent les membres de `fv` dans l'archétype de `g_`, ce que veut dire `has_ra = 0`. Celui de `test_g_` (`qy`) dit pourquoi un champ du groupe Q7 est vérifié hors compte, comme `jj` dans `test_mz`.
- **G6. Six jumelles** : C3, C4 et C6 mutent un littéral ou une affectation de `_new`/init, pas un `_start` ; C5 mute un défaut hérité sans masque, C6 retire un masque côté chaîne.

### Surprises de mesure, pour `plan-pilot`

1. **`--champs` 23 → 20** par les deux `.emitter = {…}` (trois types porteurs d'`emitter`). 2.12d (`.probe` × 4 valeurs) et 2.12e (`.emitter`, `.probe`) le feront encore bouger ; le groupe « 0 hors constructeur » ne remonte pas avant 2.13.
2. **C3 : le `has_od = 1` de `pico8_g__struct_init` n'a aucun filet au parent** (aucun TU, 26 e2e conformes). Le test ponctuel est le premier. 2.13c doit le reprendre dans un TU durable.
3. **Un site `const` neuf** (`probe.dw`, `test_pico8_jc.c:893`), venu de 2.11r-d.
4. C1 (`ki` de `g_`) rougit 17 e2e, C6 (`ki` de `fv`) 2 : l'émetteur des sondes se voit en jeu.

### Questions pour l'humain

Aucune. 2.12c ne dépend pas de la question ouverte sur les pointeurs du noyau.
