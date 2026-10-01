---
title: "Plan 3, étape 2.12a : les archétypes de la famille a (`bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `hp`, `ie`, `pj`, `hy`) et le test ponctuel `test_pico8_archetype_chain.c` (51 comparaisons ; `défauts` 669, pied inchangé) — un commit, jeu identique"
description: "Douze objets constants naissent à côté des `pico8_X_start` : chacun porte les défauts de sa classe résolus le long de la chaîne d'init, `me`/`t` de `ke` en tête. Personne ne les lit encore. Un TU neuf compare, champ par champ, chaque archétype à ce que la chaîne laisse dans un `pico8_X_new` frais, et compte ses 51 comparaisons."
status: needs-triage
---

## Problem Statement

**Écrit sur `0acfad8`** (branche `claude/plan-pilot-3-7zdft0`), pendant que 2.11r s'implémente en parallèle dans l'arbre de travail. Aucune ligne de code du dépôt n'a changé pour écrire ce PRD. Tous les essais ont été faits sur une seule copie jetable de `0acfad8` (scratchpad `prd212a/`, supprimée depuis) : le texte exact de la section « Ce qui change » y a été appliqué, compilé et joué.

**2.12a part de l'arbre d'après 2.11r-c.** Recouvrements avec 2.11r, tous vérifiés comme sans effet sur 2.12a :

- **Constructeurs.** 2.11r-c retype les `static void *pico8_X_struct_new` en `static pico8_X_t *…` (`bg.c:66`, `economy.c:22`, `mouse_reader.c:43`, `msgbox.c:106`, `:254`, `research.c:131`, `research_icon.c:236`, `score_panel.c:288`, `scroll.c:44`, `ui_button.c:107`, `:352`, `ui.c:93`). 2.12a ne touche aucun de ces corps. Le TU neuf n'appelle que les `pico8_X_new` publics, dont 2.11r ne change pas la signature.
- **`economy.c`.** 2.11r-b2 et -b3 réécrivent `pico8_hy_hk` et `pico8_hy_lj` (`:115`, `:192`, `:252`). 2.12a insère sept lignes après `static const pico8_struct_class_t g_hy_struct_class = {0};` (ancre de texte, `:20` à `0acfad8`). Les numéros bougent, les deux textes ne se touchent pas.
- **`ui.h`.** 2.11r-b2 retire `#include "pico8_ptrs.h"` de `ui.h`. Le TU neuf n'emploie aucun nom de `pico8_ptrs.h`.
- **Compteur.** 2.11r-a ajoute la 13ᵉ catégorie, `registres`. Le script en cours d'écriture dans l'arbre de travail (`tools/composition_residue.sh`, non commis, copié en lecture) a été joué sur la copie, au parent et à l'enfant de 2.12a : `registres 11 6`, pied `1084 / 87` aux deux, `--detail` identique hors la ligne `info` de taille de `game_data.h`. Aucune ligne de 2.12a n'est donc comptée par `registres`, et 2.12a ne change rien après 2.11r-c non plus.
- `game_data.h`, `game_data.c`, `economy.h` : 2.11r ne les touche pas (aucune mention dans son PRD). `Makefile` : 2.11r-c change la ligne `COMPOSITION_RESIDUE_GUARD`, 2.12a ajoute une règle et une entrée de `UNIT_TESTS` ailleurs.

Les numéros de ligne ci-dessous sont ceux de `0acfad8`. L'implémenteur cherche chaque site par le texte cité.

Vu du mainteneur du port : chaque classe fixe aujourd'hui ses défauts par une chaîne d'écritures (`pico8_ke_struct_init`, puis le `_new` de la classe qui lit son `pico8_X_start`). La phase 2 remplace ces défauts par un objet constant par classe, l'archétype, que l'instance lira par un pointeur `def` (étude 2.9, §2). Avant que les inits ne lisent l'archétype (2.13), la règle 3 exige un test ponctuel : l'archétype doit donner, champ par champ, la valeur que la chaîne donne encore. 2.12a fait ce travail pour la famille a, la plus simple : douze racines sans masquage.

### Texte du plan, recopié (`docs/plan-3.md` à `0acfad8`, ligne 2.12)

> - 2.12 — archétypes et **test ponctuel de la règle 3**, selon l'étude 2.9 (`docs/etude-2-9-archetypes.md`, §2 à §4). Chaque archétype naît dans `pico8_game_data.h` (type) et `pico8_game_data.c` (valeur résolue, sans nommer aucun `_start`), sauf celui de `hy`, dans `economy.h/.c`. Le test ponctuel est un TU neuf, `tests/test_pico8_archetype_chain.c`. Il compare champ par champ les champs A et I d'un `pico8_X_new(NULL)` frais à `pico8_X_def`, groupes de Q7 compris, avec les trois exceptions de l'étude, §3. Aucune lecture ne bascule encore, et aucun `def` n'entre dans les instances. Compte : `défauts` **669** à chaque sous-étape. Preuves : preuve compilateur `const` de chaque champ d'archétype (Q3, relevé de l'étude rejoué sur la copie du commit) ; test ponctuel vert, rouge sous les deux mutations de la famille (étude, §3 : un défaut propre changé, un défaut hérité masqué rendu à la valeur de l'ascendante). `class-coverage-net` : non. `sanitize-net` : non. Budget ≈ 6 × 50 min.
>   - 2.12a — `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `hp`, `ie`, `pj`, `hy` : 12 types, 51 comparaisons, et le TU neuf. `pj_cr` n'a pas d'archétype (instance statique unique).

Règles communes de la phase (plan, « Preuves communes à chaque ligne ») : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée.

### Réserves de vérification intégrées

1. **`hy`** : la valeur est un objet `const` **non `static`**, `pico8_hy_def`, déclaré `extern` dans `economy.h`. Le type `pico8_hy_def_t` est dans `economy.h`, la valeur dans `economy.c`. Le test ponctuel la lit ainsi. `economy.h` n'inclut pas `game_data.h`, et 2.12a ne l'y ajoute pas. L'étude (§2) disait « valeur `static const` » et nommait l'essai `g_hy_def` ; sa propre phrase sur la place (« une liaison interne le rendrait illisible pour le test ponctuel ») tranche pour `extern`.
2. **Les cinq champs qui pointent un objet statique de module** (`g_jo_kw_shared`, `g_ga_by_shared`, `g_shared_le`, `g_nf_ra`, `g_mz_jj_wrapper`) ne sont pas membres des archétypes et restent posés par l'init. Deux sont dans la famille a : `ui.le` et `nf.ra`. Ils ne comptent pas dans les 51 comparaisons. Le TU les vérifie à part : `ui->le == pico8_ui_le_shared()`, et deux `nf` frais partagent un même `ra` non NULL (`g_nf_ra` n'a pas d'accesseur, et 2.12a n'en ajoute pas).
3. **Aucune nouvelle paire d'inclusion** : `inclusion-mutuelle 17 16`, **43 lignes** au `--check`, mesuré sur la copie modifiée. **Aucun cast, aucun aiguillage par classe** : grep du diff vide (Greps de Sortie 5).

### État de départ attendu (après 2.11r-c)

| Commande | Résultat attendu |
|---|---|
| `make composition-residue` | `parente 10 3`, `lien-parent 111 27`, `accesseurs-q6 237 33`, `défauts 669 52`, `void-ptr 8 4`, `inclusion-mutuelle 17 16` ; pas de ligne `registres` (0, gardée) ; **`résidu 2 : 1073 ligne(s), 85 fichier(s)`** |
| `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` | `…,nu,ptrs-entité,registres` |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 282 ; 0 écriture : 15 ; 0 hors constructeur : 27` |
| `sh tools/composition_residue.sh --check inclusion-mutuelle \| tail -1` | `résidu gardé (inclusion-mutuelle) : 43 ligne(s)` |
| `make test` ; `make e2e` | vert ; 26 conformes |
| `grep -rn '_def\b\|_def_t\b' src tests` | vide |

Un départ autre : arrêt, rapporté. Si 2.11r-c arrive à un autre pied, l'arrivée de 2.12a est ce même pied (2.12a ne change aucun compte).

## Solution

Un seul commit :

- **Onze types** `pico8_X_def_t` dans `pico8_game_data.h` et **onze valeurs** `const pico8_X_def_t pico8_X_def` dans `pico8_game_data.c`, déclarées `extern`, pour `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `mh`, `iv`, `hp`, `ie`, `pj`.
- **`pico8_hy_def_t`** et `extern const pico8_hy_def_t pico8_hy_def;` dans `economy.h`, la valeur dans `economy.c`.
- Chaque valeur est **résolue** et écrite en littéraux. Aucune ne nomme un `pico8_X_start`. `me`/`t` viennent en tête pour les dix classes qui descendent de `ke` (pas `hp` ni `hy`). Les vecteurs et les boîtes sont **par valeur**. Les séquences et les rangées pointent les mêmes objets de données que les `_start`.
- **TU neuf** `tests/test_pico8_archetype_chain.c` : une fonction par classe, 51 comparaisons comptées, et le total asserté.
- `Makefile` : la règle du TU et son entrée dans `UNIT_TESTS`.
- `docs/plan-3.md` : note « Découvert à 2.12a ».

Rien d'autre. Aucun `def` n'entre dans une instance. Aucun `_new`, aucune init et aucun `_start` ne changent. Aucune lecture ne bascule.

## User Stories

1. En tant qu'auteur de 2.13a1/a2, je veux que l'archétype de chaque classe de la famille a existe déjà et ait été prouvé égal à la chaîne, afin que l'init puisse le lire sans changer une valeur.
2. En tant que gardien de la règle 3, je veux un test qui compare chaque champ A et I de la famille a d'une instance fraîche à l'archétype, afin qu'un écart entre les deux soit rouge avant que la chaîne ne disparaisse.
3. En tant que gardien du test ponctuel, je veux qu'il compte ses comparaisons et asserte le total (51), afin qu'une comparaison oubliée ou supprimée soit rouge.
4. En tant que lecteur de `economy.h`, je veux que l'archétype de `hy` y soit, lisible par `extern`, afin que ni `economy.h` n'inclue `game_data.h`, ni le test ne dépende d'un accesseur neuf.
5. En tant que gardien du compteur, je veux que `défauts` reste à 669, `inclusion-mutuelle` à 17 / 16 et le pied inchangé, afin que l'archétype ne soit pas un `_start` déguisé ni une paire neuve.
6. En tant que gardien de la règle 1, je veux la preuve `const` de chaque champ d'archétype de la famille a, rejouée sur la copie du commit, afin de savoir que les champs A n'ont d'écriture qu'à la construction.
7. En tant que gardien du jeu, je veux `make e2e` à 26 conformes et aucun objet de `src/` changé hors `game_data` et `economy`, afin d'être sûr que personne ne lit encore l'archétype.
8. En tant que vérificateur, je veux des mutations de l'archétype et de la chaîne, rejouées au parent quand c'est possible, afin de prouver que le test mord et qu'il ajoute un filet.
9. En tant que lecteur du plan, je veux une note « Découvert à 2.12a » courte, afin que 2.12b parte des vrais comptes.

## Implementation Decisions

### Modules

`game_data`, `economy` ; TU neuf `archetype_chain` ; `Makefile` ; `docs/plan-3.md`. Aucun autre module, aucun autre TU.

### Ce qui change, fichier par fichier (numéros de `0acfad8`)

**`src/pico8_game_data.h`** — après `} pico8_pj_start_t;` (`:263`), avant le commentaire « The jo tree… », insérer :

```c

/* A class's archetype: its defaults resolved along its init chain, ke's
 * `me`/`t` first. A default that is the address of a module's static object
 * (ui.le, nf.ra) is not here: the module's init still sets it. */
typedef struct {
    const char *me;
    pico8_fixed_t t;
    pico8_fixed_t fa;
    pico8_fixed_t hd;
    const pico8_int_seq_t *pl;
} pico8_bg_def_t;

typedef struct {
    const char *me;
    pico8_fixed_t t;
    pico8_fixed_t ql;
    pico8_fixed_t fa;
    pico8_qz_box_t hw;
} pico8_ui_def_t;

typedef struct {
    const char *me;
    pico8_fixed_t t;
    pico8_fixed_t fa;
} pico8_js_def_t;

typedef struct {
    const char *me;
    pico8_fixed_t t;
    pico8_fixed_t fa;
    pico8_fixed_t hd;
} pico8_fq_def_t;

typedef struct {
    const char *me;
    pico8_fixed_t t;
    pico8_vec_t v;
    pico8_fixed_t ma;
    pico8_vec_t p;
    pico8_vec_t ra;
    const pico8_vec_seq_t *pc;
} pico8_qb_def_t;

typedef struct {
    const char *me;
    pico8_fixed_t t;
} pico8_nf_def_t;

typedef struct {
    const char *me;
    pico8_fixed_t t;
    pico8_fixed_t fa;
    pico8_fixed_t rh;
} pico8_mh_def_t;

typedef struct {
    const char *me;
    pico8_fixed_t t;
    pico8_fixed_t fa;
    pico8_fixed_t l;
    pico8_vec_t v;
} pico8_iv_def_t;

typedef struct {
    const pico8_jb_row_t *fk;
} pico8_hp_def_t;

typedef struct {
    const char *me;
    pico8_fixed_t t;
    pico8_qz_box_t kq;
    pico8_fixed_t fa;
    pico8_vec_t ra;
    pico8_vec_t eh;
} pico8_ie_def_t;

typedef struct {
    const char *me;
    pico8_fixed_t t;
    pico8_fixed_t fa;
    const pico8_jb_row_t *gs;
    const pico8_jb_row_t *i_;
} pico8_pj_def_t;
```

Et après `extern const pico8_mx_start_t pico8_mx_start;` (`:510`) :

```c
extern const pico8_bg_def_t pico8_bg_def;
extern const pico8_ui_def_t pico8_ui_def;
extern const pico8_js_def_t pico8_js_def;
extern const pico8_fq_def_t pico8_fq_def;
extern const pico8_qb_def_t pico8_qb_def;
extern const pico8_nf_def_t pico8_nf_def;
extern const pico8_mh_def_t pico8_mh_def;
extern const pico8_iv_def_t pico8_iv_def;
extern const pico8_hp_def_t pico8_hp_def;
extern const pico8_ie_def_t pico8_ie_def;
extern const pico8_pj_def_t pico8_pj_def;
```

`game_data.h` : 540 → **634** lignes.

**`src/pico8_game_data.c`** — après la valeur `pico8_pj_start` (`:425-432`, qui finit par `.ed = 0x10000,` / `};`), avant `static const char jo_me_text[]` :

```c

const pico8_bg_def_t pico8_bg_def = {
    .me = "ph",
    .t = 0x0,
    .fa = 0x10000,
    .hd = 0x10000,
    .pl = &pico8_bg_pl_seq,
};

const pico8_ui_def_t pico8_ui_def = {
    .me = "ph",
    .t = 0x0,
    .ql = 0x3a0000,
    .fa = 0xf0000,
    .hw = {0xa0000, 0xa0000, 0x760000, 0x760000},
};

const pico8_js_def_t pico8_js_def = {
    .me = "ph",
    .t = 0x0,
    .fa = 0xb0000,
};

const pico8_fq_def_t pico8_fq_def = {
    .me = "ph",
    .t = 0x0,
    .fa = 0xe0000,
    .hd = 0x10000,
};

const pico8_qb_def_t pico8_qb_def = {
    .me = "ph",
    .t = 0x0,
    .v = {0x0, 0x0},
    .ma = 0x0,
    .p = {0xc00000, 0xc00000},
    .ra = {0xc00000, 0xc00000},
    .pc = &pico8_qb_pc_vecs,
};

const pico8_nf_def_t pico8_nf_def = {
    .me = "ph",
    .t = 0x0,
};

const pico8_mh_def_t pico8_mh_def = {
    .me = "ph",
    .t = 0x0,
    .fa = 0xe0000,
    .rh = 0x50000,
};

const pico8_iv_def_t pico8_iv_def = {
    .me = "ph",
    .t = 0x0,
    .fa = 0x90000,
    .l = 0x0,
    .v = {0x0, -0x570a},
};

const pico8_hp_def_t pico8_hp_def = {
    .fk = &pico8_hp_fk_row,
};

const pico8_ie_def_t pico8_ie_def = {
    .me = "ph",
    .t = 0x0,
    .kq = {0x0, 0x0, 0x90000, 0x80000},
    .fa = 0xc0000,
    .ra = {0x3e70000, 0x3e70000},
    .eh = {0x760000, 0x780000},
};

const pico8_pj_def_t pico8_pj_def = {
    .me = "ph",
    .t = 0x0,
    .fa = 0xa0000,
    .gs = &pico8_pj_gs_row,
    .i_ = &pico8_pj_i__row,
};
```

Chaque valeur est celle de l'annexe B de l'étude, recopiée de `pico8_ke_start` (`"ph"`, `0x0`) et du `pico8_X_start` de la classe, qu'aucune clé de construction ne réécrit quand `keys` est NULL. Ce qui n'y entre pas, et pourquoi (annexe A de l'étude) : `ie.id` (lu une fois par `pico8_research_icon_init` vers `g_ie_id`, pas un membre), `pj.ea`/`ou`/`cr`/`ed` (lus par `pico8_score_panel_init`, et `ed` est E), `nf.ra` et `ui.le` (objets statiques de module).

**`src/pico8_economy.h`** — après `} pico8_hy_t;` (`:34`) :

```c

/* hy's archetype, here and not with the others: economy.h does not include
 * pico8_game_data.h. */
typedef struct {
    pico8_fixed_t nz;
    pico8_fixed_t ew;
    pico8_fixed_t lo;
    pico8_fixed_t yr;
} pico8_hy_def_t;

extern const pico8_hy_def_t pico8_hy_def;
```

**`src/pico8_economy.c`** — après `static const pico8_struct_class_t g_hy_struct_class = {0};` (`:20`) :

```c

const pico8_hy_def_t pico8_hy_def = {
    .nz = 0x640000,
    .ew = 0x10000,
    .lo = 0x10000,
    .yr = 0xd490000,
};
```

(`100`, `1`, `1`, `3401` en virgule fixe 16.16 : les quatre dernières écritures de `pico8_hy_struct_new`, `economy.c:43`, `:46`, `:51`, `:52`.)

**`tests/test_pico8_archetype_chain.c`** (neuf) — texte essayé sur la copie :

```c
/* Each archetype against the value its class's init chain still leaves in a
 * fresh instance, field by field. Removed with the chains it compares. */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bg.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy.h"
#include "../src/pico8_game_data.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_mouse_reader.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_score_panel.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static int g_compared;

static void same(int ok, const char *what) {
    if (!ok) {
        fprintf(stderr, "archetype differs from its chain: %s\n", what);
    }
    assert(ok);
    g_compared++;
}

static int same_text(const char *got, const char *want) {
    return got != NULL && want != NULL && strcmp(got, want) == 0;
}

static int same_vec(const pico8_vec_t *got, const pico8_vec_t *want) {
    return got != NULL && got->x == want->x && got->y == want->y;
}

static int same_box(const pico8_qz_box_t *got, const pico8_qz_box_t *want) {
    return got != NULL && got->xl == want->xl && got->yt == want->yt && got->xr == want->xr && got->yb == want->yb;
}

static void test_bg(void) {
    const pico8_bg_t *bg = pico8_bg_new();

    same(same_text(bg->header.me, pico8_bg_def.me), "bg.me");
    same(bg->header.t == pico8_bg_def.t, "bg.t");
    same(bg->fa == pico8_bg_def.fa, "bg.fa");
    same(bg->hd == pico8_bg_def.hd, "bg.hd");
    same(bg->pl == pico8_bg_def.pl, "bg.pl");
}

/* `le` is the module's shared list, not the archetype's. */
static void test_ui(void) {
    const pico8_ui_t *ui = pico8_ui_new();

    same(same_text(ui->header.me, pico8_ui_def.me), "ui.me");
    same(ui->header.t == pico8_ui_def.t, "ui.t");
    same(ui->ql == pico8_ui_def.ql, "ui.ql");
    same(ui->fa == pico8_ui_def.fa, "ui.fa");
    same(same_box(ui->hw, &pico8_ui_def.hw), "ui.hw");
    assert(ui->le == pico8_ui_le_shared());
}

static void test_js(void) {
    const pico8_js_t *js = pico8_js_new(NULL);

    same(same_text(js->header.me, pico8_js_def.me), "js.me");
    same(js->header.t == pico8_js_def.t, "js.t");
    same(js->fa == pico8_js_def.fa, "js.fa");
}

static void test_fq(void) {
    const pico8_fq_t *fq = pico8_fq_new(NULL);

    same(same_text(fq->header.me, pico8_fq_def.me), "fq.me");
    same(fq->header.t == pico8_fq_def.t, "fq.t");
    same(fq->fa == pico8_fq_def.fa, "fq.fa");
    same(fq->hd == pico8_fq_def.hd, "fq.hd");
}

static void test_qb(void) {
    const pico8_qb_t *qb = pico8_qb_new();

    same(same_text(qb->header.me, pico8_qb_def.me), "qb.me");
    same(qb->header.t == pico8_qb_def.t, "qb.t");
    same(same_vec(qb->v, &pico8_qb_def.v), "qb.v");
    same(qb->ma == pico8_qb_def.ma, "qb.ma");
    same(same_vec(qb->p, &pico8_qb_def.p), "qb.p");
    same(same_vec(qb->ra, &pico8_qb_def.ra), "qb.ra");
    same(qb->pc == pico8_qb_def.pc, "qb.pc");
}

/* `ra` is the module's shared vector, not the archetype's: two fresh
 * readers point at the same one. */
static void test_nf(void) {
    const pico8_nf_t *nf = pico8_nf_new();
    const pico8_nf_t *other = pico8_nf_new();

    same(same_text(nf->header.me, pico8_nf_def.me), "nf.me");
    same(nf->header.t == pico8_nf_def.t, "nf.t");
    assert(nf->ra != NULL && nf->ra == other->ra);
}

static void test_mh(void) {
    const pico8_mh_t *mh = pico8_mh_new(NULL);

    same(same_text(mh->header.me, pico8_mh_def.me), "mh.me");
    same(mh->header.t == pico8_mh_def.t, "mh.t");
    same(mh->fa == pico8_mh_def.fa, "mh.fa");
    same(mh->rh == pico8_mh_def.rh, "mh.rh");
}

static void test_iv(void) {
    const pico8_iv_t *iv = pico8_iv_new(NULL);

    same(same_text(iv->header.me, pico8_iv_def.me), "iv.me");
    same(iv->header.t == pico8_iv_def.t, "iv.t");
    same(iv->fa == pico8_iv_def.fa, "iv.fa");
    same(iv->l == pico8_iv_def.l, "iv.l");
    same(same_vec(iv->v, &pico8_iv_def.v), "iv.v");
}

/* hp does not descend from ke: no `me`/`t`. */
static void test_hp(void) {
    const pico8_hp_t *hp = pico8_hp_new(NULL);

    same(hp->fk == pico8_hp_def.fk, "hp.fk");
}

static void test_ie(void) {
    const pico8_ie_t *ie = pico8_ie_new(NULL);

    same(same_text(ie->header.me, pico8_ie_def.me), "ie.me");
    same(ie->header.t == pico8_ie_def.t, "ie.t");
    same(same_box(ie->kq, &pico8_ie_def.kq), "ie.kq");
    same(ie->fa == pico8_ie_def.fa, "ie.fa");
    same(same_vec(ie->ra, &pico8_ie_def.ra), "ie.ra");
    same(same_vec(ie->eh, &pico8_ie_def.eh), "ie.eh");
}

static void test_pj(void) {
    const pico8_pj_t *pj = pico8_pj_new();

    same(same_text(pj->header.me, pico8_pj_def.me), "pj.me");
    same(pj->header.t == pico8_pj_def.t, "pj.t");
    same(pj->fa == pico8_pj_def.fa, "pj.fa");
    same(pj->gs == pico8_pj_def.gs, "pj.gs");
    same(pj->i_ == pico8_pj_def.i_, "pj.i_");
}

/* hy does not descend from ke: no `me`/`t`. */
static void test_hy(void) {
    const pico8_hy_t *hy = pico8_hy_new();

    same(hy->nz == pico8_hy_def.nz, "hy.nz");
    same(hy->ew == pico8_hy_def.ew, "hy.ew");
    same(hy->lo == pico8_hy_def.lo, "hy.lo");
    same(hy->yr == pico8_hy_def.yr, "hy.yr");
}

int main(void) {
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);
    pico8_cart_load();
    pico8_jl();

    test_bg();
    test_ui();
    test_js();
    test_fq();
    test_qb();
    test_nf();
    test_mh();
    test_iv();
    test_hp();
    test_ie();
    test_pj();
    test_hy();
    assert(g_compared == 51 && "family a compares 51 fields: one missed or one too many");

    printf("test_pico8_archetype_chain: all tests passed\n");
    return 0;
}
```

Lecture du TU :
- La valeur « par la chaîne » est celle d'un `pico8_X_new` frais, clés NULL, après `pico8_cart_load()` et `pico8_jl()`, comme `test_pico8_spec_defaults.c`.
- Les chaînes (`me`) se comparent par contenu. Les vecteurs et boîtes aussi, puisque l'archétype porte les siens par valeur et que l'instance pointe encore ceux du `_start`. Les séquences et rangées (`pl`, `pc`, `fk`, `gs`, `i_`) se comparent par adresse : ce sont les mêmes objets de données.
- `same` compte. Le total 51 est asserté à la fin, avec un message qui dit le piège (règle 3 de `CLAUDE.md`). À 2.12b, l'implémenteur change ce total (51 + 25) et son message. C'est le seul point du TU qui bouge d'une sous-étape à l'autre, hors les fonctions ajoutées.
- Les vérifications à part de `ui.le` et `nf.ra` sont des `assert` nus, hors compte.
- Le TU n'emploie aucun cast, aucun `pico8_X_start`, et aucune comparaison de classe.

**`Makefile`**
- Après la règle de `$(TMP_DIR)/test_pico8_jo_spec_defaults` (`:477-478`) :

  ```make

  $(TMP_DIR)/test_pico8_archetype_chain: $(call objs,tests/test_pico8_archetype_chain.c $(ENGINE_SRCS)) | $(TMP_DIR)
  	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) -lm
  ```
- Dans `UNIT_TESTS`, après `    test_pico8_jo_spec_defaults \` (`:631`) : `    test_pico8_archetype_chain \`.

**`docs/plan-3.md`** — note « Découvert à 2.12a » au bout de la ligne 2.12a : comptes (inchangés), `game_data.h` 634 lignes, `pico8_hy_def` `extern` (réserve 1), total asserté du TU à changer à chaque sous-étape, et le « pour 2.12b ». Le reste va au message de commit.

Rien d'autre : ni `glossaire-noms.md` (aucun nom minifié neuf ; `def` n'est pas minifié), ni `trous-de-portage.md`, ni `tools/`, ni `tests/e2e/`, ni `*.sha256`.

### Comptes, mesurés sur la copie de `0acfad8`, valables après 2.11r-c

| Mesure | parent | 2.12a |
|---|---|---|
| `make composition-residue` (12 catégories, `0acfad8`) | pied 1073 / 85 | **1073 / 85**, `--detail` identique hors `info src/pico8_game_data.h 540 → 634 lignes` |
| même, compteur à 13 catégories de l'arbre de travail (2.11r-a en cours) | `registres 11 6`, pied 1084 / 87 | **identique** |
| attendu après 2.11r-c | pied 1073 / 85 | **1073 / 85** |
| `défauts` | 669 / 52 | **669 / 52** |
| `inclusion-mutuelle` | 17 / 16, 43 lignes | **17 / 16, 43 lignes** |
| `--champs` | 282 ; 15 ; 27 | **282 ; 15 ; 27** (aucun `def` dans les instances) |
| comparaisons du test ponctuel | — | **51** (A 18 + I 33, étude §3) |

Répartition des 51 : `bg` 5, `ui` 5, `js` 3, `fq` 4, `qb` 7, `nf` 2, `mh` 4, `iv` 5, `hp` 1, `ie` 6, `pj` 5, `hy` 4.

### Anti-tassement

- **Interdit** :
  - nommer un `pico8_X_start` (ou un objet au suffixe `_start`) dans une valeur d'archétype, même pour un vecteur : `défauts` passerait à 670 (piège mesuré par l'étude avec `kw_own`) ;
  - rendre `pico8_hy_def` `static` et ajouter un accesseur, ou inclure `game_data.h` dans `economy.h` ;
  - mettre `ui.le` ou `nf.ra` (ou un pointeur vers un objet statique de module) dans un archétype ;
  - ajouter un membre `def` à une instance, ou faire lire l'archétype par un `_new`, une init ou un lecteur (c'est 2.13) ;
  - comparer dans le TU l'instance à un `pico8_X_start`, au lieu de `pico8_X_def` ;
  - un cast, un `void *`, ou une comparaison de classe dans le TU ou dans les archétypes ;
  - toucher un `*.sha256`, `tests/e2e/`, une constante d'empreinte, `tools/composition_residue.sh` ou son TU.

## Implementation Chunks

Un commit, **≈ 35 min** preuves comprises (essai : 20 min d'écriture, ≈ 15 min de preuves). Budget 50 min.

1. Coller les quatre blocs de `src/` et le TU, les deux lignes du `Makefile`.
2. `make test`, `make e2e`, compteur.
3. Preuves (ci-dessous), note du plan, commit, push.

**Repli écrit** (si le budget déborde avant le commit, au-delà de 1,5 fois) : deux commits verts, chacun avec son TU et son total asserté :
- **2.12a1** : `bg`, `ui`, `js`, `fq`, `qb`, `nf`, `hy` et le TU (30 comparaisons) ;
- **2.12a2** : `mh`, `iv`, `hp`, `ie`, `pj` (21 de plus, total 51).
Les comptes ne changent à aucun des deux. Le repli est écrit dans la note « Découvert à ».

## Testing Decisions

- **Un TU neuf, et c'est le livrable.** Il ne rejoue jamais un `events.json`. Les e2e partent de l'écran titre (`make e2e`), comme toujours.
- **Bon test ici** : il prend la valeur que la chaîne laisse dans une instance fraîche (le comportement actuel), et la compare au seul objet neuf. Il ne relit pas le `_start` : sinon, un `_start` et un archétype faux de la même façon passeraient.
- **Prior art** : `tests/test_pico8_spec_defaults.c` (instance fraîche, inits de module) ; mutations avec la règle du parent de D.21 et de 2.4 ; preuve `const` de l'étude 2.9, annexe A.

### Preuves

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. Mesuré sur la copie : `make test` code 0 (`test_pico8_archetype_chain: all tests passed`, `garde fichiers 58 en-echec 0`), `make e2e` 26 conformes.

2. **Test ponctuel : chaque champ archétype comparé à l'ancienne valeur `_start` résolue.** La valeur par la chaîne, c'est le `_start` de la classe (plus `pico8_ke_start` pour `me`/`t`, et les littéraux de `pico8_hy_struct_new` pour `hy`), tel que le `_new` l'écrit. Le TU vert au commit prouve les 51 égalités. `g_compared == 51` prouve qu'aucune n'est oubliée.

3. **Mutations**, une à la fois, `make -k -j8 unit` puis `make -k e2e`. Restaurer par `cp` puis `touch` (piège de 2.3 : `mv` rend un `mtime` ancien). **Règle du parent** : une mutation verte au commit et rouge au parent est un **ÉCHEC**. Une mutation verte aux deux est déclarée « chemin jamais atteint ». Mesuré sur la copie :

   *Côté archétype (étude, §3 ; au parent, ni l'archétype ni le TU n'existent : la mutation n'a pas de jumelle au parent, et doit être rouge au commit, sinon ÉCHEC).*

   | # | Mutation | TU rouges | e2e |
   |---|---|---|---|
   | M1 | défaut propre : `pico8_bg_def.hd` `0x10000` → `0x20000` | `archetype_chain` seul (`bg.hd`) | 26 / 26 |
   | M2 | défaut hérité non masqué (la famille n'a aucun masquage, étude §3) : `pico8_ui_def.me` `"ph"` → `"kz"` | `archetype_chain` seul (`ui.me`) | 26 / 26 |
   | M3 | archétype hors `game_data` : `pico8_hy_def.yr` `0xd490000` → `0xd4a0000` | `archetype_chain` seul (`hy.yr`) | 26 / 26 |
   | M4 | vecteur par valeur : `pico8_qb_def.p` `{0xc00000, 0xc00000}` → `{0xc00000, 0xc10000}` | `archetype_chain` seul (`qb.p`) | 26 / 26 |
   | M5 | `t` hérité de `ke` : `pico8_nf_def.t` `0x0` → `0x10000` | `archetype_chain` seul (`nf.t`) | 26 / 26 |
   | M6 | comparaison oubliée : la ligne `same(pj->i_ == …)` retirée du TU | `archetype_chain` seul (`g_compared == 51`) | 26 / 26 |

   « `archetype_chain` seul » et 26 e2e conformes : c'est aussi la preuve que **personne ne lit encore l'archétype**.

   *Côté chaîne, jumelles rejouées au parent (règle du parent pleine).*

   | # | Mutation (identique au parent et au commit) | TU rouges au commit | TU rouges au parent | e2e (commit = parent) |
   |---|---|---|---|---|
   | C1 | `pico8_bg_start.hd` `0x10000` → `0x20000` | `archetype_chain`, `bg`, `g_` | `bg`, `g_` | 26 / 26 |
   | C2 | `pico8_hy_struct_new` : `pico8_fixed_from_int(3401)` → `3402` | `archetype_chain` + les 11 du parent | `fr`, `g_`, `ga`, `hj`, `hp`, `hy`, `lq`, `mx`, `nr`, `pj`, `pm` | rouge |
   | C3 | `pico8_qb_start.p` `{0xc00000, 0xc00000}` → `{0xc00000, 0xc10000}` | `archetype_chain` + les 11 du parent | `fr`, `hj`, `hp`, `iv`, `jc`, `lq`, `mx`, `nr`, `pm`, `qb`, `ui` | rouge |

   Lecture : au commit, chaque jumelle rougit ce que le parent rougit, plus le test ponctuel. Aucun ÉCHEC. C1 montre le filet ajouté : au parent, `test_pico8_spec_defaults` reste vert sous C1, parce qu'il compare l'instance au `_start` lui-même.

4. **Couverture.** `gcov` du TU (compilé `--coverage` avec `ENGINE_SRCS`, dans une copie) : **127 / 128** lignes exécutées (99,22 %), la seule non exécutée étant le `fprintf` d'échec de `same`. Les 12 fonctions de test sont exécutées, et le total 51 l'atteste. `make line-coverage` des e2e n'est pas exigé : aucune ligne de code de `src/` n'est ajoutée ni changée (données seulement, preuve 5).

5. **Objets** (sans `-g`, `gcc -std=c99 -Wall -Wextra -Werror -Isrc -c`, parent puis commit, 153 `.c` compilables sans flag ; `main_class_coverage.c` et `test_class_coverage.c` exigent `-DPICO8_CLASS_COVERAGE`, les trois raylib `raylib.h`) :
   - **151 objets identiques** octet à octet, dont tous ceux qui incluent `game_data.h` ou `economy.h` ;
   - **2 diffèrent**, `pico8_game_data.o` et `pico8_economy.o` : `nm` ne gagne que les nouveaux symboles (`pico8_bg_def` … `pico8_pj_def`, 11 ; `pico8_hy_def`, 1), et `objdump -d` ne diffère que par les immédiats `__LINE__` des `assert` (lignes décalées de 80 et de 7). Sous `-DNDEBUG`, le `.text` des deux est **identique** ;
   - l'objet du TU est neuf.

6. **Règle 1, preuve `const` rejouée** (dans la copie du commit ; chaque membre A ou I de la famille a rendu `const` dans la struct d'instance, un à la fois, puis `gcc -fsyntax-only` de tous les `.c` compilables de `src/` et `tests/`, erreurs listées). Mesuré : **31 membres, 69 sites**, plus `me`/`t` de l'en-tête (10 et 8 sites). Les sites sont ceux de l'annexe A de l'étude, décalés de +7 dans `economy.c` : les 18 champs A n'ont d'écriture que dans leur `_new` ; les écritures hors construction sont celles des champs I (`qb.v` `bg.c:226`, `scroll.c:128`, `T:qb.c:375` ; `qb.ma` `planet_family.c:446`, `T:qb.c:229` ; `qb.p` `scroll.c:127` ; `qb.ra` `scroll.c:132` ; `iv.l` `msgbox.c:196` ; `ie.ra` `research_icon.c:101` ; `pj.fa` `score_panel_display.c:283` ; `hy.nz` 11, `hy.ew` 1, `hy.lo` 3, `hy.yr` 3 ; `js.fa` et `mh.rh` par clé dans leur `_new`). Tout autre site est un arrêt « champ sans règle ». Le script tient en 50 lignes : un piège, `js` et `fq` sont déclarés `struct pico8_js {` / `struct pico8_fq {`, `mh` `struct pico8_mh {`, pas `typedef struct {…} pico8_X_t;`.

7. **Inclusion, casts, aiguillage** : `inclusion-mutuelle` **17 / 16**, **43 lignes** au `--check`, mesuré ; greps de Sortie 5 et 6.

8. `class-coverage-net` : **non** (aucune table, aucun constructeur ne bouge). `sanitize-net` : **non** (aucune durée de vie ne change).

### Greps de Sortie (sur l'arbre du commit)

1. `grep -c 'const pico8_[a-z_]*_def_t pico8_[a-z_]*_def = {' src/pico8_game_data.c` → **11** ; même grep sur `src/pico8_economy.c` → **1**.
2. `grep -n 'pico8_hy_def' src/pico8_economy.h` → le type et `extern const pico8_hy_def_t pico8_hy_def;` ; `grep -rn 'static const pico8_hy_def_t' src` → vide ; `grep -n 'game_data' src/pico8_economy.h` → vide.
3. `git diff <parent> -- src | grep -E '^\+.*_start'` → vide (aucune ligne ajoutée ne nomme un `_start`).
4. `grep -rnE '\bdef\b *;|->def\b|\.def\b' src` → vide (aucun `def` dans une instance, aucune lecture).
5. `git diff <parent> | grep -E '^\+.*\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)'` → vide (aucun cast) ; `git diff <parent> | grep -E '^\+.*(->cls|header\.cls|_struct_class\(\))'` → vide (aucun aiguillage).
6. `grep -rnE 'g_shared_le|g_nf_ra|g_jo_kw_shared|g_ga_by_shared|g_mz_jj_wrapper' src/pico8_game_data.[ch] src/pico8_economy.[ch]` → vide.
7. `make composition-residue` → pied **1073 / 85**, `défauts 669 52`, `inclusion-mutuelle 17 16` ; `--champs` → `282 ; 15 ; 27` ; `wc -l src/pico8_game_data.h` → 634.
8. `git diff --stat <parent>` : `src/pico8_game_data.h`, `src/pico8_game_data.c`, `src/pico8_economy.h`, `src/pico8_economy.c`, `tests/test_pico8_archetype_chain.c`, `Makefile`, `docs/plan-3.md`, et rien d'autre.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable de `0acfad8` (`p` parent, `a` 2.12a appliquée).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout 0acfad8` ; `composition_residue.sh` ; `--champs` | `p` | 0 | pied 1073 / 85 ; 282 / 15 / 27 | 2 s |
| 2 | texte de « Ce qui change » appliqué ; `timeout 600 make -j8 test` | `a` | 0 | `test_pico8_archetype_chain: all tests passed` ; `garde fichiers 58 en-echec 0` | 11 s |
| 3 | `timeout 300 make e2e` | `a` | 0 | 26 conformes | 1 s |
| 4 | compteur, `--check inclusion-mutuelle`, `--champs` | `a` | 0 | 1073 / 85 ; 43 lignes ; 282 / 15 / 27 | 1 s |
| 5 | `--detail` au parent (`git stash -u`) puis à l'enfant, `diff` | `p` / `a` | 1 | seule différence `info game_data.h 540 → 634` | 2 s |
| 6 | compteur à 13 catégories de l'arbre de travail (2.11r-a en cours), parent et enfant | `p` / `a` | 0 | `registres 11 6`, pied 1084 / 87 aux deux ; `--detail` idem 5 | 2 s |
| 7 | `constproof.py`, 1er essai | `a` | 1 | `IndexError` : `js`/`fq` déclarés `struct pico8_js {` ; corrigé | 3 s |
| 8 | `timeout 900 constproof.py`, 31 membres | `a` | 0 | 69 sites, conformes à l'annexe A (+7 dans `economy.c`) | 20 s |
| 9 | idem, `me`/`t` de l'en-tête | `a` | 0 | 10 et 8 sites, identiques à l'annexe A | 7 s |
| 10 | M1 à M6, `make -k -j8 unit` + `make -k e2e` | `a` | 2 | `archetype_chain` seul rouge ; 26 e2e conformes | ≈ 20 s × 6 |
| 11 | C1 à C3 au commit, puis au parent (`git stash -u`) | `a` / `p` | 2 | tableau des jumelles ; aucun ÉCHEC | ≈ 20 s × 6 |
| 12 | `gcc --coverage` du TU + `ENGINE_SRCS`, `gcov` | `a` | 0 | 127 / 128 lignes | 25 s |
| 13 | objets sans `-g` des `.c` compilables, parent et enfant ; `cmp`, `nm`, `objdump -d` ; `-DNDEBUG` sur les deux différents | `p` / `a` | 0 | 151 identiques ; 2 diffèrent par `__LINE__` et 12 symboles ; `.text` identique sous `-DNDEBUG` | 30 s |
| 14 | greps de cast et d'aiguillage sur le diff | `a` | 1 | vide | < 1 s |
| 15 | `rm -rf` de la copie | — | 0 | copie supprimée | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas le commit 2.11r-c (ou un descendant qui ne touche que `docs/`), ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) est rouge, si un `*.sha256` ou `tests/e2e/` change, ou si une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si le TU ne compte pas 51 comparaisons, ou si une comparaison lit un `pico8_X_start` au lieu de `pico8_X_def`.
- `FAILED:` si une mutation M1 à M6 est verte au commit, ou si elle rougit un autre TU que `archetype_chain` ou un e2e (quelqu'un lirait l'archétype).
- `FAILED:` si une jumelle C1 à C3 est verte au commit et rouge au parent.
- `FAILED:` si un objet autre que `game_data` et `economy` diffère sans `-g`, ou si l'un des deux diffère sous `-DNDEBUG` dans son `.text`.
- `FAILED:` si la preuve `const` trouve un site hors de l'annexe A (aux décalages de ligne près), ou une écriture hors construction d'un champ A.
- `FAILED:` si `défauts` n'est pas 669, `inclusion-mutuelle` 17 / 16 (43 lignes), le pied celui de 2.11r-c, ou `--champs` 282 / 15 / 27.
- `FAILED:` si `pico8_hy_def` est `static`, si `economy.h` inclut `game_data.h`, si un archétype contient un des cinq objets statiques, ou en cas de tassement (voir « Anti-tassement »).
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si le message du commit omet : les 12 archétypes et la répartition des 51, les mutations M et les jumelles C avec leur résultat au parent, la couverture du TU, les objets, la preuve `const` (comptes), les comptes, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.12a », ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- `pj_cr` : pas d'archétype (étude §2) ; ses trois A passent dans l'initialiseur de `g_pj_cr` à 2.13a2.
- Toute lecture de l'archétype, tout membre `def`, la suppression d'un `_start` : 2.13a1 et 2.13a2.
- `pico8_probe_def_t`, `pico8_emitter_def_t`, `mz`, `jc` et au-delà : 2.12b et suivantes.
- Un accesseur de `g_nf_ra` : non nécessaire, deux `nf` frais suffisent à montrer le partage.
- `tools/composition_residue.sh`, son TU ; `class-coverage-net`, `sanitize-net` ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. `pico8_hy_def` est `extern`, pas `static`** (réserve 1). L'étude écrivait « valeur `static const` » dans le §2, mais aussi, au même §2, qu'une liaison interne rendrait l'objet illisible pour le test ponctuel. La réserve du vérificateur tranche.
- **G2. `me` s'écrit `"ph"` en littéral** dans chaque archétype, comme `pico8_ke_start.me`. Nommer `pico8_ke_start` compterait en `défauts`, et aucun texte statique de `game_data.c` ne vaut `"ph"`. Le TU compare par contenu.
- **G3. Le TU compte et asserte son total.** L'étude voulait « champ par champ » : le compteur transforme « une comparaison oubliée » en rouge (M6). Le total change à chaque sous-étape de 2.12, dans le même commit que les fonctions ajoutées.
- **G4. Place des blocs** : les types juste après `pico8_pj_start_t`, les valeurs juste après `pico8_pj_start`, les `extern` juste après ceux des `_start`. À 2.13a, chaque `_start` qui part laisse son archétype à la même place.
- **G5. Jumelles côté chaîne (C1 à C3)** en plus des mutations d'archétype : l'étude interdit de muter la chaîne pour prouver que le test mord, mais seule une mutation qui existe au parent permet la règle du parent pleine. Les deux séries sont gardées, séparées.

### Surprises de mesure, pour `plan-pilot`

1. **Objets** : ajouter des données au milieu de `game_data.c` et d'`economy.c` décale les `__LINE__` des `assert` qui suivent. Les deux objets diffèrent donc sans `-g`, et sont identiques en `.text` sous `-DNDEBUG`. Les 151 autres, qui ne voient que des types et des `extern` en plus, sont identiques octet à octet.
2. **`test_pico8_spec_defaults` ne voit pas un `_start` faux** (C1 au parent) : il compare l'instance au `_start` lui-même. Le test ponctuel, lui, le voit.
3. **Trois structs d'instance de la famille a sont déclarées par étiquette** (`struct pico8_js {`, `struct pico8_fq {`, `struct pico8_mh {`) : un script de preuve `const` qui cherche `} pico8_X_t;` les manque.
4. **`hp` et `hy` n'écrivent jamais `me`/`t`** (ils ne passent pas par `pico8_ke_struct_init`) : leurs archétypes n'ont pas ces membres, et le TU ne les compare pas.

### Questions pour l'humain

Aucune.
