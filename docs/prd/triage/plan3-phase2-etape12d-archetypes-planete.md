---
title: "Plan 3, étape 2.12d : le type planète `pico8_jo_def_t` complet (champs des dix sortes), les valeurs de `jo`, `qh`, `nl`, `os`, et 100 comparaisons de plus au test ponctuel (total 231, pas 234 ; `défauts` 669, pied inchangé) — un commit, jeu identique"
description: "Un seul type d'archétype pour les dix sortes de planète (décision A), 33 membres, dont les sous-archétypes sonde et émetteur enchâssés. Quatre valeurs. Le test ponctuel gagne `test_jo` (16), `test_qh` (27), `test_nl` (30), `test_os` (27) : 100, et non les 103 du plan, parce que `lz` réécrit `kw_own` sur les sortes de l'arbre `qh` (exception 2 de l'étude 2.9, mesurée). Le type couvre `hj` à `fr` sans champ manquant (essai sur copie : 440 comparaisons vertes)."
status: needs-triage
---

## Problem Statement

**Écrit sur `7d7f194`** (branche `claude/plan-pilot-3-7zdft0`, après 2.12c `168208e` et 2.11r-e `1f5e33d`). Aucune ligne du dépôt n'a changé pour écrire ce PRD. Les essais ont été faits sur une copie jetable (scratchpad `prd212d/`, supprimée depuis). Elle a d'abord été prise à `e288dbf`, avec 2.12c appliquée d'après son PRD, puis rejouée à `7d7f194` quand 2.12c y est arrivée. Le texte de 2.12c appliqué est identique octet pour octet aux trois fichiers de `168208e`. Notation : `p` = parent (2.12c), `c` = le texte de « Ce qui change » appliqué et commité dans la copie.

Vu du mainteneur du port : 2.12a à 2.12c ont créé les archétypes des racines, du trou de ver, de la voie et de la sonde. 2.12d crée **le type planète**, un seul type pour les dix sortes (décision A de l'humain, étude 2.9 §2 : « d crée `pico8_jo_def_t` complet, avec les champs des dix sortes, et e n'ajoute que des valeurs »). Il crée aussi les quatre premières valeurs : `jo`, `qh`, `nl`, `os`.

### Texte du plan, recopié (`docs/plan-3.md` à `7d7f194`)

> - 2.12 — archétypes et **test ponctuel de la règle 3**, selon l'étude 2.9 […]. Il compare champ par champ les champs A et I d'un `pico8_X_new(NULL)` frais à `pico8_X_def`, groupes de Q7 compris, avec les trois exceptions de l'étude, §3. Aucune lecture ne bascule encore, et aucun `def` n'entre dans les instances. Compte : `défauts` **669** à chaque sous-étape. […]
>   - 2.12d — `jo`, `qh`, `nl`, `os` et le type planète `pico8_jo_def_t` complet (champs des dix sortes) : 103 comparaisons, groupe sonde.

Plan, 2.13 : « Les objets statiques `g_jo_kw_shared`, `g_ga_by_shared` […] restent posés par l'init. » Étude 2.9 §3, exception 2 : « `jo.kw`/`kw_own` des neuf descendants de `qh` : `pico8_jo_pq` appelle `pico8_jo_lz` quand la planète a un `hp`, et les réécrit. Ils se comparent sur `jo`, que `lz` ne touche pas au `_new`. »

Règles communes : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée.

### 103 ou 100 : l'étude se contredit, la mesure tranche

Les 103 de l'étude (tableau « Masquages » : `jo` 10 + 6, `qh` 19 + 9, `nl` 21 + 10, `os` 19 + 9) comptent `kw_own` sur les quatre sortes. Son exception 2 le retire des sortes de l'arbre `qh`. Mesuré sur la copie, premier essai avec `kw_own` comparé partout : **rouge**, `archetype differs from its chain: qh.kw_own`. Diagnostic : après `pico8_qh_new(NULL)`, `pico8_nl_new(NULL)` et `pico8_os_new(NULL)`, on a `hn` non NULL, `kw == &kw_own` et `kw_own.has_qe = 1`. `kw_own.qe` vaut `0x180000` pour `qh`, `0xc0000` pour `nl` et `0x1c0000` pour `os`, et `mu` vaut 0. `pico8_cart_load` remplit `gb` : `pico8_jo_pq` trouve la recherche d'indice `flr(hp)` et `lz` réécrit `kw_own` (`planet.c:325-328`).

Donc, selon l'exception 2 : **`kw_own` n'est comparé que sur `jo`**, soit `jo` 16, `qh` 27, `nl` 30, `os` 27, **100** comparaisons (A 69, I 31), et **231** au total. La même règle vaut en famille e (essai plus bas) : 2.12e1 **120** (`hj` 39, `pm` 41, `mx` 40) au lieu de 123, et 2.12e2 **89** (`ga` 30, `nr` 29, `fr` 30) au lieu de 91. `ga` n'a pas de `hp` : `lz` ne tourne pas, et `ga` garde son `kw_own` comparé. Ce n'est pas un repli, c'est une erreur de compte de l'étude. La note « Découvert à 2.12d » la reporte sur les lignes 2.12e1/e2.

### Ce que 2.12d place, et ce qu'il laisse

| Élément | Sous-étape | Pourquoi |
|---|---|---|
| type `pico8_jo_def_t`, **complet** : les 33 membres des dix sortes, `probe` et `emitter` enchâssés | **2.12d** (ce PRD) | étude §4 : « d crée `pico8_jo_def_t` complet » ; 2.12e n'ajoute que des valeurs |
| valeurs `pico8_jo_def`, `pico8_qh_def`, `pico8_nl_def`, `pico8_os_def` | **2.12d** | test ponctuel, famille d |
| valeurs de `hj`, `pm`, `mx`, `ga`, `nr`, `fr` | 2.12e1, 2.12e2 | plan |
| `jo.kw` (`&g_jo_kw_shared`), `jo.by` de `ga` (`&g_ga_by_shared`) | **jamais** dans l'archétype | objets `static` de module (étude §2) ; posés par l'init jusqu'au bout (plan 2.13) |
| membre `def` de `pico8_jo_t`, lecture de l'archétype | 2.13d | interdit en 2.12 |

### Question ouverte chez l'humain (Q-2.10, liens du noyau)

**2.12d n'en dépend pas** : aucun lien du noyau, aucun membre d'instance.

### Travail en parallèle

Aucun PRD en cours d'implémentation connu à `7d7f194` (2.11r-e et 2.12c sont commités et notés). Si un autre implémenteur travaille en même temps :

| Fichier | 2.12d | Recouvrement |
|---|---|---|
| `src/pico8_game_data.[ch]`, `tests/test_pico8_archetype_chain.c` | oui | un autre PRD qui les touche (2.12e) passe **après** : il s'ancre sur ce que 2.12d pose |
| `docs/plan-3.md` | ligne 2.12d (note) | commiter avec `git add -p docs/plan-3.md` (seul le bloc 2.12d), jamais `git commit -a` |
| `tools/composition_residue.sh` | lu | les comptes se mesurent au même script au parent et au commit |
| `tmp/` | `make test`, mutations | **jamais deux `make` dans le même arbre en même temps** |

### État de départ attendu (`7d7f194` ou un descendant qui ne touche que `docs/`)

| Commande | Résultat mesuré à `7d7f194` |
|---|---|
| `make composition-residue` | `défauts 669 52`, `inclusion-mutuelle 17 16` ; pied **1073 / 85** |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 282 ; 0 écriture : 15 ; 0 hors constructeur : 20` |
| `sh tools/composition_residue.sh --check inclusion-mutuelle \| tail -1` | `résidu gardé (inclusion-mutuelle) : 43 ligne(s)` |
| `wc -l src/pico8_game_data.h` | 702 |
| `grep -c 'g_compared == 131' tests/test_pico8_archetype_chain.c` | 1 |
| `grep -rnwE 'pico8_jo_def_t\|pico8_jo_def\|pico8_qh_def\|pico8_nl_def\|pico8_os_def' src tests` | vide |
| `make test` ; `make e2e` | vert ; 26 conformes |

Un départ autre : arrêt, rapporté.

## Solution

Un seul commit :

- **Un type** dans `pico8_game_data.h`, après `pico8_g__def_t` : `pico8_jo_def_t`, 33 membres. D'abord `me`/`t`, puis les champs de `jo` dans l'ordre de l'instance, sans `kw` ni `by`, avec `kw_own` par valeur. Viennent ensuite ceux de `qh`, puis les noms propres des sortes, un seul membre par nom (`rr` pour `nl`/`hj`/`nr`, `fz` pour `pm`/`ga`), puis `probe` et `emitter`.
- **Quatre valeurs** `const` non `static` dans `pico8_game_data.c`, `pico8_jo_def`, `pico8_qh_def`, `pico8_nl_def`, `pico8_os_def`, déclarées `extern`. Elles sont résolues, écrites en littéraux, et ne nomment aucun `_start`.
- **TU** `tests/test_pico8_archetype_chain.c` : deux inclusions, deux aides de comparaison (`same_in`, `same_kw`), deux aides par partie (`same_jo_part`, `same_qh_part`), `assert_lz_ran`, `test_jo` (16), `test_qh` (27), `test_nl` (30), `test_os` (27). Total asserté **231**.
- `docs/plan-3.md` : note « Découvert à 2.12d ».

Rien d'autre. Aucun `def` dans une instance. Aucun `_new`, init ou `_start` ne change. Aucune lecture ne bascule. Pas de `Makefile`.

## User Stories

1. En tant qu'auteur de 2.13d, je veux les archétypes de `jo`, `qh`, `nl`, `os` prouvés égaux à la chaîne, afin que `pico8_jo_struct_init` copie le `def` de la classe concrète sans changer une valeur.
2. En tant qu'auteur de 2.12e1/e2, je veux un type planète **complet**, déjà essayé sur les six autres sortes, afin de n'ajouter que des valeurs.
3. En tant que gardien de la décision A, je veux **un seul** type d'archétype pour les dix sortes.
4. En tant que gardien de la règle 3, je veux le groupe sonde de Q7 comparé pour `qh`, `nl`, `os`, avec les exceptions de l'étude hors compte : `kw`, `by`, `et`, et `kw_own` sur l'arbre `qh`.
5. En tant que gardien du test ponctuel, je veux le total asserté à 231, avec l'écart au plan (103 → 100) expliqué et reporté sur 2.12e.
6. En tant que gardien du compteur, je veux `défauts` 669, `inclusion-mutuelle` 17 / 16, le pied inchangé et `--champs` expliqué.
7. En tant que gardien de la règle 1, je veux la preuve `const` des 29 membres d'instance que l'archétype décrit, sur la copie du commit.
8. En tant que gardien du jeu, je veux 26 e2e conformes et aucun objet de `src/` changé hors `game_data`.
9. En tant que vérificateur, je veux des mutations côté archétype, et des jumelles côté chaîne rejouées au parent.

## Implementation Decisions

### Modules

`game_data` ; TU `archetype_chain` ; `docs/plan-3.md`. Aucun autre.

### Ce qui change, fichier par fichier (ancres de texte ; numéros de `7d7f194`)

**`src/pico8_game_data.h`**

1. Après `} pico8_g__def_t;` (`:410`), avant la ligne vide qui précède « The jo tree… » (`:412`), insérer :

   ```c

   /* The ten planet kinds share this type; a kind's value leaves at zero the
    * members of the kinds it does not descend from. `kw`, and ga's `by`, point
    * at objects their module owns: not here. */
   typedef struct {
       const char *me;
       pico8_fixed_t t;
       int hf;
       pico8_fixed_t hs;
       pico8_fixed_t fa;
       pico8_qz_box_t kq;
       pico8_fixed_t ma;
       const pico8_int_seq_t *ka;
       const pico8_jb_row_t *dz; /* PICO8_JO_DZ_COUNT rows */
       const pico8_int_seq_t *cq;
       pico8_vec_t gq;
       pico8_fixed_t ca;
       const pico8_jb_row_t *em;
       pico8_fixed_t mi;
       int has_mi;
       pico8_kw_t kw_own;
       pico8_fixed_t hp;
       const pico8_jb_row_t *px;
       int qh;
       pico8_fixed_t s;
       const pico8_int_seq_t *ba;
       const char *rr;
       pico8_fixed_t nl;
       pico8_fixed_t qw;
       pico8_vec_t ng;
       const char *fz;
       pico8_fixed_t iu;
       pico8_fixed_t ms;
       const pico8_int_seq_t *hz;
       pico8_fixed_t mt;
       const pico8_int_seq_t *ht;
       pico8_probe_def_t probe;
       pico8_emitter_def_t emitter;
   } pico8_jo_def_t;
   ```

2. Après `extern const pico8_g__def_t pico8_fv_def;` (`:672`) :

   ```c
   extern const pico8_jo_def_t pico8_jo_def;
   extern const pico8_jo_def_t pico8_qh_def;
   extern const pico8_jo_def_t pico8_nl_def;
   extern const pico8_jo_def_t pico8_os_def;
   ```

`game_data.h` : 702 → **745** lignes.

**`src/pico8_game_data.c`** — après la fin de la valeur `pico8_mx_start` (`    .q_ = 0xfae0,` puis `};`, `:715-733`), avant `const pico8_kind_rec_t pico8_qh_kind_rec` (`:735`). Les valeurs nomment `jo_me_text`, `qh_me_text`, `qh_dq_text` et `nl_rr_text`, des `static` déclarés plus haut (`:566-576`) : elles ne peuvent pas aller avant eux.

```c

const pico8_jo_def_t pico8_jo_def = {
    .me = jo_me_text,
    .t = 0x0,
    .hf = 1,
    .hs = 0x10000,
    .fa = 0x60000,
    .kq = {-0x70000, -0x70000, 0x70000, 0x70000},
    .ma = 0x0,
    .ka = &pico8_jo_ka_seq,
    .dz = pico8_jo_dz_rows,
    .cq = &pico8_jo_cq_seq,
    .gq = {0x0, 0xb0000},
    .ca = 0x90000,
    .em = &pico8_jo_em_row,
    .mi = 0x0,
    .has_mi = 0,
    .kw_own = {0, 0x0, 0x0},
};

const pico8_jo_def_t pico8_qh_def = {
    .me = qh_me_text,
    .t = 0x0,
    .hf = 0,
    .hs = 0x10000,
    .fa = 0x60000,
    .kq = {-0x70000, -0x70000, 0x70000, 0x70000},
    .ma = 0x0,
    .ka = &pico8_jo_ka_seq,
    .dz = pico8_jo_dz_rows,
    .cq = &pico8_jo_cq_seq,
    .gq = {0x0, 0xb0000},
    .ca = 0x90000,
    .em = &pico8_jo_em_row,
    .mi = 0x10000,
    .has_mi = 1,
    .kw_own = {0, 0x0, 0x0},
    .hp = 0x10000,
    .px = &pico8_qh_px_row,
    .qh = 1,
    .s = 0xae0000,
    .ba = &pico8_qh_ba_seq,
    .probe = {.dw = 0x666, .od = 0xf0000, .has_od = 1, .dq = qh_dq_text, .fp = 0x20000, .ik = 0x0, .has_ik = 0},
};

const pico8_jo_def_t pico8_nl_def = {
    .me = qh_me_text,
    .t = 0x0,
    .hf = 0,
    .hs = 0x10000,
    .fa = 0x60000,
    .kq = {-0x70000, -0x70000, 0x70000, 0x70000},
    .ma = 0x0,
    .ka = &pico8_jo_ka_seq,
    .dz = pico8_jo_dz_rows,
    .cq = &pico8_jo_cq_seq,
    .gq = {0x0, 0xb0000},
    .ca = 0x90000,
    .em = &pico8_jo_em_row,
    .mi = 0x10000,
    .has_mi = 1,
    .kw_own = {0, 0x0, 0x0},
    .hp = 0x60000,
    .px = &pico8_nl_px_row,
    .qh = 0,
    .s = 0x440000,
    .ba = &pico8_nl_ba_seq,
    .rr = nl_rr_text,
    .nl = 0x10000,
    .qw = 0xf0000,
    .probe = {.dw = 0x666, .od = 0x140000, .has_od = 1, .dq = qh_dq_text, .fp = 0x20000, .ik = 0x0, .has_ik = 0},
};

const pico8_jo_def_t pico8_os_def = {
    .me = qh_me_text,
    .t = 0x0,
    .hf = 0,
    .hs = 0x8000,
    .fa = 0x60000,
    .kq = {-0x70000, -0x70000, 0x70000, 0x70000},
    .ma = 0x0,
    .ka = &pico8_jo_ka_seq,
    .dz = pico8_jo_dz_rows,
    .cq = &pico8_jo_cq_seq,
    .gq = {0x0, 0xb0000},
    .ca = 0x90000,
    .em = &pico8_jo_em_row,
    .mi = 0x10000,
    .has_mi = 1,
    .kw_own = {0, 0x0, 0x0},
    .hp = 0x40000,
    .px = &pico8_os_px_row,
    .qh = 0,
    .s = 0x640000,
    .ba = &pico8_qh_ba_seq,
    .probe = {.dw = 0x666, .od = 0xf0000, .has_od = 1, .dq = qh_dq_text, .fp = 0x20000, .ik = 0x0, .has_ik = 0},
};
```

Chaque valeur est celle de l'annexe B de l'étude 2.9, recopiée de `pico8_ke_start` (`t`), de `pico8_jo_start`, `pico8_qh_start`, `pico8_nl_start`, `pico8_os_start`, et des littéraux de `pico8_jo_struct_init` (`mi = 0`, `has_mi = 0`) et de `pico8_qh_struct_init` (`has_mi = 1`, `has_od = 1`, `ik = 0`, `has_ik = 0`). `kw_own` recopie les trois valeurs de `pico8_jo_kw_start`, **sans le nommer** : nommer un `_start` ferait passer `défauts` à 670 (étude §2, essai d'inclusion). Masquages portés (étude §3) : `qh` masque `hf`, `mi`, `has_mi` et `me` ; `nl` masque `qh`, `hp`, `s`, `px`, `ba` et `probe.od` ; `os` masque `qh`, `hp`, `s`, `px` et `hs`. Chaque membre comparé a son désignateur, même nul (`.ma = 0x0`, `.qh = 0`, `.ik = 0x0`), pour que la mutation ait une ligne à changer. Les membres des sortes absentes de la chaîne restent à zéro (désignateurs omis) : `rr`/`nl`/`qw` hors `nl`, `ng` … `ht` et `emitter` partout, `hp` … `ba` et `probe` pour `jo`. Les vecteurs et la boîte sont par valeur, en initialiseurs positionnels, comme en 2.12c. `kw_own` aussi : il suit `pico8_jo_kw_start`, positionnel (voir `--champs`).

**`tests/test_pico8_archetype_chain.c`**

1. Inclusions, dans l'ordre alphabétique, après `#include "../src/pico8_msgbox.h"` :

   ```c
   #include "../src/pico8_planet.h"
   #include "../src/pico8_planet_family.h"
   ```

2. Après `same` (avant `same_text`) :

   ```c

   static void same_in(int ok, const char *kind, const char *field) {
       char what[32];

       snprintf(what, sizeof what, "%s.%s", kind, field);
       same(ok, what);
   }

   static int same_kw(const pico8_kw_t *got, const pico8_kw_t *want) {
       return got->has_qe == want->has_qe && got->qe == want->qe && got->mu == want->mu;
   }
   ```

3. Avant `int main(void)`, après `test_fv` :

   ```c

   /* `kw_own`, `kw` and `by` are left to each kind's test. */
   static void same_jo_part(const pico8_jo_t *jo, const pico8_jo_def_t *def, const char *kind) {
       same_in(same_text(jo->header.me, def->me), kind, "me");
       same_in(jo->header.t == def->t, kind, "t");
       same_in(jo->hf == def->hf, kind, "hf");
       same_in(jo->hs == def->hs, kind, "hs");
       same_in(jo->fa == def->fa, kind, "fa");
       same_in(same_box(jo->kq, &def->kq), kind, "kq");
       same_in(jo->ma == def->ma, kind, "ma");
       same_in(jo->ka == def->ka, kind, "ka");
       same_in(jo->dz == def->dz, kind, "dz");
       same_in(jo->cq == def->cq, kind, "cq");
       same_in(same_vec(jo->gq, &def->gq), kind, "gq");
       same_in(jo->ca == def->ca, kind, "ca");
       same_in(jo->em == def->em, kind, "em");
       same_in(jo->mi == def->mi, kind, "mi");
       same_in(jo->has_mi == def->has_mi, kind, "has_mi");
   }

   static void same_qh_part(const pico8_qh_t *qh, const pico8_jo_def_t *def, const char *kind) {
       same_in(qh->hp == def->hp, kind, "hp");
       same_in(qh->px == def->px, kind, "px");
       same_in(qh->qh == def->qh, kind, "qh");
       same_in(qh->s == def->s, kind, "s");
       same_in(qh->ba == def->ba, kind, "ba");
       same_in(qh->probe->dw == def->probe.dw, kind, "probe.dw");
       same_in(qh->probe->od == def->probe.od, kind, "probe.od");
       same_in(qh->probe->has_od == def->probe.has_od, kind, "probe.has_od");
       same_in(same_text_or_nil(qh->probe->dq, def->probe.dq), kind, "probe.dq");
       same_in(qh->probe->fp == def->probe.fp, kind, "probe.fp");
       same_in(qh->probe->ik == def->probe.ik, kind, "probe.ik");
       same_in(qh->probe->has_ik == def->probe.has_ik, kind, "probe.has_ik");
       assert(qh->probe->et == NULL);
       assert(qh->jo.by == NULL);
   }

   /* A kind of the qh tree has an `hp`: its `_new` runs `lz`, which rewrites
    * `kw_own` from that research and points `kw` at it. Its `kw_own` is
    * compared on jo only. */
   static void assert_lz_ran(const pico8_jo_t *jo) {
       assert(jo->hn != NULL && jo->kw == &jo->kw_own);
   }

   /* `kw` is the module's shared kw, not the archetype's. */
   static void test_jo(void) {
       const pico8_jo_t *jo = pico8_jo_new(NULL);

       same_jo_part(jo, &pico8_jo_def, "jo");
       same_in(same_kw(&jo->kw_own, &pico8_jo_def.kw_own), "jo", "kw_own");
       assert(jo->kw == pico8_jo_shared_kw());
       assert(jo->by == NULL);
   }

   static void test_qh(void) {
       const pico8_qh_t *qh = pico8_qh_new(NULL);

       same_jo_part(&qh->jo, &pico8_qh_def, "qh");
       same_qh_part(qh, &pico8_qh_def, "qh");
       assert_lz_ran(&qh->jo);
   }

   static void test_nl(void) {
       const pico8_nl_t *nl = pico8_nl_new(NULL);

       same_jo_part(&nl->qh.jo, &pico8_nl_def, "nl");
       same_qh_part(&nl->qh, &pico8_nl_def, "nl");
       same_in(same_text(nl->rr, pico8_nl_def.rr), "nl", "rr");
       same_in(nl->nl == pico8_nl_def.nl, "nl", "nl");
       same_in(nl->qw == pico8_nl_def.qw, "nl", "qw");
       assert_lz_ran(&nl->qh.jo);
   }

   static void test_os(void) {
       const pico8_os_t *os = pico8_os_new(NULL);

       same_jo_part(&os->qh.jo, &pico8_os_def, "os");
       same_qh_part(&os->qh, &pico8_os_def, "os");
       assert_lz_ran(&os->qh.jo);
   }
   ```

4. Dans `main`, après `test_fv();`, et l'assert du total remplacé :

   ```c
       test_jo();
       test_qh();
       test_nl();
       test_os();
       assert(g_compared == 231 && "families a to d compare 231 fields: one missed or one too many");
   ```

Lecture du TU :
- Chaque partie de l'instance se lit par ses membres (`&nl->qh.jo`, `&os->qh`), jamais par une conversion : aucun cast, aucun `pico8_qh_of`/`probe_of`, aucune comparaison de classe. La sonde se lit par le membre `probe` (K, pointe `probe_own`), comme en 2.12b/c.
- Les aides par partie servent aux dix sortes : 2.12e1/e2 les réutilisent (essai plus bas) et n'ajoutent que `same_emitter_part`. Les étiquettes restent `sorte.champ` (`nl.hf`, `os.probe.od`), comme les littéraux de 2.12a à 2.12c : `same_in` les compose.
- Comparaisons par contenu : `me`, `rr` et `dq` (textes), `kq` (boîte), `gq` (vecteur), `kw_own` (les trois membres). Par adresse : `ka`, `dz`, `cq`, `em`, `px`, `ba` (données partagées de `game_data.c`).
- Hors compte, en `assert` nus : `kw` (exception 1 : `&g_jo_kw_shared` sur `jo`, par `pico8_jo_shared_kw()`), `by` NULL (exception 1), `et` NULL (E, sonde), et `assert_lz_ran` sur `qh`/`nl`/`os` (exception 2 : `kw_own` y est réécrit par `lz`, `kw` pointe dessus).
- Répartition : `jo` 16 = 15 (`same_jo_part`) + `kw_own` (A 10, I 6) ; `qh` 27 = 15 + 12 (A 19, I 8) ; `nl` 30 = 15 + 12 + 3 (A 21, I 9) ; `os` 27 = 15 + 12 (A 19, I 8). **100 = A 69 + I 31.** L'étude (§3) comptait `kw_own` en I sur les trois dernières : 9, 10 et 9.

**`docs/plan-3.md`** — note « Découvert à 2.12d » au bout de la ligne 2.12d (`:447`). Elle porte : les comptes (`défauts` 669, pied inchangé, `--champs` 20 → 19 et pourquoi) ; `game_data.h` 745 lignes ; le total du TU **231**, soit **100** comparaisons et non 103, parce que `lz` réécrit `kw_own` sur l'arbre `qh` (exception 2) ; le report **2.12e1 : 120**, **2.12e2 : 89** (total 440), essayé sur copie ; et le « pour 2.13d » (voir plus bas). Le reste va au message de commit. Commit de ce fichier par `git add -p`.

Rien d'autre : ni `glossaire-noms.md` (aucun nom minifié neuf), ni `trous-de-portage.md`, ni `Makefile`, ni `tools/`, ni `tests/e2e/`, ni `*.sha256`, ni les lignes 2.12e1/e2 du plan (la note les annonce ; leur PRD les corrige).

### Comptes, mesurés sur la copie (`p` = `7d7f194`, `c` = 2.12d)

| Mesure | `p` | `c` |
|---|---|---|
| `make composition-residue` | 1073 / 85 | **1073 / 85**, `--detail` identique hors `info src/pico8_game_data.h 702 → 745 lignes` |
| `défauts` (`--check`) | 669 / 52 | **669 / 52** |
| `inclusion-mutuelle` | 17 / 16, 43 lignes | **17 / 16, 43 lignes** |
| `--champs` | 282 ; 15 ; 20 | **282 ; 15 ; 19** (voir ci-dessous) |
| comparaisons du test ponctuel | 131 | **231** (+100) |
| `game_data.h` | 702 | **745** |
| `git diff --stat` (sans `docs/`) | — | 3 fichiers, 238 insertions, 1 suppression |

**`--champs` 20 → 19, attendu.** Le relevé compte, par nom de membre, les lignes `.X =` d'un `.c` de `src/`. Une ligne hors fonction est « hors constructeur », y compris les désignateurs imbriqués d'un initialiseur (`.probe = {.dw = …, .od = …}` compte `probe`, `dw`, `od`, …). Les quatre `.kw_own = {…}` donnent à `pico8_jo_t.kw_own` 4 écritures hors constructeur (1/0 → 5/4) : c'est **le seul** couple (type, membre) qui quitte le groupe « 0 hors constructeur » (`diff` complet `p`/`c`). Tout le reste bouge sans changer de groupe : `fa`, `me`, `ca`, `gq`, `kq`, `hf`, `hs`, `ka`, `dz`, `cq`, `em`, `ma`, `mi`, `has_mi` +4 ; `probe`, `od`, `has_od`, `px`, `hp`, `qh`, `s`, `ba` +3 ; `rr`, `nl`, `qw` +1. Aucun membre neuf (282), aucun ne quitte « 0 écriture » (15). Les initialiseurs positionnels (`kq`, `gq`, `kw_own`) n'ajoutent que le nom du membre englobant ; en désignateurs, `.kw_own = {.has_qe = …}` ajouterait `has_qe`, `qe`, `mu` à tous les types qui ont ces noms. **2.12e1/e2** : les six valeurs de l'essai laissent `--champs` à **19** (le groupe ne bouge plus ; `emitter` en est sorti dès 2.12c).

### Le type couvre `hj` … `fr` : essai sur la copie

Sur `c`, six valeurs de plus, `pico8_hj_def` … `pico8_fr_def`, du **même** type, avec les valeurs de l'annexe B. Le TU a reçu six fonctions : les aides de 2.12d, plus `same_emitter_part` (onze comparaisons), `assert_lz_ran` sur `hj`, `pm`, `mx`, `nr`, `fr`, et sur `ga`, `kw == pico8_jo_shared_kw()` et un `by` partagé par deux `ga`. Résultat : **compile, vert, 440 comparaisons** (231 + 120 + 89), sans aucun membre ajouté au type ni aucun champ manquant. `défauts` 669, `inclusion-mutuelle` 43 lignes, `--champs` 19, `game_data.h` 751 lignes (six `extern`). L'essai a été retiré de la copie avant les preuves. Il n'entre pas dans ce PRD, mais son texte donne à 2.12e le modèle des valeurs, par exemple `pico8_ga_def` : `.me = jo_me_text`, `.hf = 0`, `.fa = 0x30000`, `.kq = {0x0, 0x0, -0x10000, -0x10000}`, `.mi = 0x0`, `.has_mi = 0`, `.fz = ga_fz_text`, `.iu = -0xf5c`, `.ms = 0x30000`, `.emitter = {.pn = 0x0, .du = 0x0, .py = 0x60000, .ki = 0x0, .pk = 0x0, .fc = 0x0, .k_ = &pico8_ga_k__seq, .qo = 0xe666, .ro = 0x0, .qg = 0x0, .q_ = 0xe666}`, sans `.probe`.

### Contrat laissé à 2.13d (à recopier dans son PRD)

- `pico8_jo_struct_init` copie `def->kw_own`, puis `pico8_jo_pq` → `lz` le réécrit sur l'arbre `qh`, **comme aujourd'hui** : le `kw_own` des archétypes `qh`/`nl`/`os` (et `hj` … `fr`) ne se voit que si `gb` n'a pas la recherche d'indice `flr(hp)`. La mutation témoin X1 (plus bas) le montre : aucun TU ni e2e ne le voit aujourd'hui.
- `kw` reste `&g_jo_kw_shared` posé par l'init (plan 2.13), `by` reste NULL ou `&g_ga_by_shared`. Aucun des deux n'est membre de `pico8_jo_def_t`.
- `pico8_jo_kw_start` sort à 2.13d (plan) : l'archétype ne le nomme pas.

### Anti-tassement

- **Interdit** :
  - plus d'un type d'archétype pour les sortes de planète (`pico8_qh_def_t`, `pico8_nl_def_t`…) ; un type incomplet que 2.12e devrait agrandir ;
  - nommer un `pico8_X_start` dans une valeur (`défauts` → 670), dont `pico8_jo_kw_start` pour `kw_own` ;
  - mettre `kw`, `by` ou un pointeur vers un statique de module dans le type ou une valeur ;
  - comparer `kw_own` de `qh`/`nl`/`os` à l'archétype, ou faire passer le total à 234 par une comparaison ajoutée ailleurs ;
  - ajouter un membre `def` à `pico8_jo_t`, `pico8_qh_t` ou `pico8_probe_t`, ou toucher `planet.h`, `planet_family.h`, `probe.h` ;
  - faire lire l'archétype par un `_new`, une init ou un lecteur ;
  - un cast, un `void *`, un `pico8_qh_of`/`pico8_nl_of`/`probe_of`, une comparaison de classe dans le TU ou les archétypes ;
  - comparer à un `pico8_X_start` au lieu de `pico8_X_def` ;
  - toucher un `*.sha256`, `tests/e2e/`, une constante d'empreinte, `tools/composition_residue.sh` ou son TU.

## Implementation Chunks

Un commit, **≈ 35 min** preuves comprises (essai : 10 min d'écriture ; `make test` ≈ 15 s, `make e2e` ≈ 1 s ; une mutation ≈ 6 s, unités et e2e ; preuve `const` 20 s ; objets ≈ 1 min ; le reste est de la lecture). Budget **50 min**.

1. Coller le bloc de `game_data.h` et ses quatre `extern`, le bloc de `game_data.c`, les quatre retouches du TU.
2. `make test`, `make e2e`, compteur, `--champs`.
3. Preuves (ci-dessous), note du plan, commit, push.

**Repli écrit** (au-delà de 1,5 fois le budget avant le commit) : deux commits verts, chacun avec son total asserté.
- **2.12d1** : le type complet, `pico8_jo_def`, `pico8_qh_def`, les aides, `test_jo`, `test_qh` (total **174**).
- **2.12d2** : `pico8_nl_def`, `pico8_os_def`, `test_nl`, `test_os` (total **231**).

Les comptes ne changent à aucun des deux, sauf `--champs`, qui passe de 20 à 19 dès 2.12d1 (`.kw_own` de `pico8_jo_def`). `game_data.h` a 743 lignes à 2.12d1 et 745 à 2.12d2. Le repli est écrit dans la note « Découvert à ».

## Testing Decisions

- **Pas de TU neuf** : le test ponctuel gagne quatre fonctions et cinq aides. Il ne rejoue jamais un `events.json`.
- **Bon test ici** : la valeur que la chaîne laisse dans une instance fraîche, comparée au seul objet neuf.
- **Prior art** : PRD 2.12a à 2.12c (même TU, mêmes preuves) ; règle du parent de D.21 et 2.4 ; preuve `const` de l'étude 2.9, annexe A.

### Preuves

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. Mesuré : `make test` code 0 (`test_pico8_archetype_chain: all tests passed`), 26 conformes.

2. **Test ponctuel** : vert au commit = 231 égalités ; `g_compared == 231` = aucune oubliée, aucune de trop.

3. **Mutations**, une à la fois, `make -k -j4 unit` puis `make -k e2e`, restauration par `cp` puis `touch`. **Règle du parent** : une mutation verte au commit et rouge au parent est un **ÉCHEC** ; verte aux deux, « chemin jamais atteint ». L'étiquette rouge se lit dans `archetype differs from its chain: …`. Mesuré sur la copie :

   *Côté archétype (au parent, aucune des quatre valeurs n'existe : chacune doit être rouge au commit).*

   | # | Mutation | TU rouges (étiquette) | e2e |
   |---|---|---|---|
   | M1 | étude §3 d, défaut propre : `pico8_nl_def.qw` `0xf0000` → `0x100000` | `archetype_chain` seul (`nl.qw`) | 26 / 26 |
   | M2 | étude §3 d, masqué rendu à l'ascendante : `pico8_nl_def.hf` `0` → `1` (`jo`) | `archetype_chain` seul (`nl.hf`) | 26 / 26 |
   | M3 | masque propre à `os` rendu à `jo` : `pico8_os_def.hs` `0x8000` → `0x10000` | `archetype_chain` seul (`os.hs`) | 26 / 26 |
   | M4 | `kw_own` de `jo` : `{0, 0x0, 0x0}` → `{0, 0x0, 0x10000}` | `archetype_chain` seul (`jo.kw_own`) | 26 / 26 |
   | M5 | sonde I : `pico8_qh_def.probe.od` `0xf0000` → `0x140000` | `archetype_chain` seul (`qh.probe.od`) | 26 / 26 |
   | M6 | sonde, masque de `nl` rendu à `qh` : `pico8_nl_def.probe.od` `0x140000` → `0xf0000` | `archetype_chain` seul (`nl.probe.od`) | 26 / 26 |
   | M7 | adresse : `pico8_os_def.px` → `&pico8_qh_px_row` | `archetype_chain` seul (`os.px`) | 26 / 26 |
   | M8 | boîte par contenu : `pico8_qh_def.kq` `yb` `0x70000` → `0x80000` | `archetype_chain` seul (`qh.kq`) | 26 / 26 |
   | M9 | texte par contenu : `pico8_jo_def.me` → `qh_me_text` | `archetype_chain` seul (`jo.me`) | 26 / 26 |
   | M10 | comparaison oubliée : `same_in(nl->qw == …)` retirée du TU | `archetype_chain` seul (`g_compared == 231`) | 26 / 26 |
   | M11 | drapeau I : `pico8_jo_def.has_mi` `0` → `1` | `archetype_chain` seul (`jo.has_mi`) | 26 / 26 |
   | M12 | texte propre : `pico8_nl_def.rr` → `hj_rr_text` | `archetype_chain` seul (`nl.rr`) | 26 / 26 |
   | M13 | séquence héritée non masquée : `pico8_os_def.ba` → `&pico8_nl_ba_seq` | `archetype_chain` seul (`os.ba`) | 26 / 26 |
   | M14 | sonde A : `pico8_qh_def.probe.dq` → `"kz"` | `archetype_chain` seul (`qh.probe.dq`) | 26 / 26 |

   « `archetype_chain` seul » et 26 e2e : **personne ne lit l'archétype**.

   *Témoin d'exception, vert attendu (pas un ÉCHEC).* **X1** : `pico8_nl_def.kw_own` → `{0, 0x0, 0x10000}` donne **aucun TU rouge**, 26 / 26. C'est l'exception 2 : `kw_own` n'est pas comparé sur l'arbre `qh`. Voir le contrat pour 2.13d.

   *Côté chaîne, jumelles identiques au parent et au commit.*

   | # | Mutation | TU rouges au commit | TU rouges au parent | e2e (commit = parent) |
   |---|---|---|---|---|
   | C1 | `pico8_qh_start.s` `0xae0000` → `0xaf0000` | `archetype_chain`, `entity_kind`, `qh` | `entity_kind`, `qh` | 23 / 26 |
   | C2 | `pico8_nl_start.qw` `0xf0000` → `0x100000` | `archetype_chain`, `nl`, `qh` | `nl`, `qh` | 26 / 26 |
   | C3 | `pico8_os_start.hs` `0x8000` → `0x10000` | `archetype_chain`, `os` | `os` | 26 / 26 |
   | C4 | `pico8_jo_start.hf` `1` → `0` (masqué par `qh`) | `archetype_chain`, `hp`, `hy`, `jc`, `jo`, `js`, `lq`, `os` | les mêmes sans `archetype_chain` | 15 / 26 |
   | C5 | `pico8_qh_struct_init` : `self->jo.has_mi = 1;` → `0` | `archetype_chain`, `fr`, `hj`, `jo_spec_defaults`, `mx`, `nl`, `nr`, `os`, `pm`, `qh` | les mêmes sans `archetype_chain` | 26 / 26 |
   | C6 | `pico8_qh_struct_init` : `self->probe->has_od = 1;` → `0` | `archetype_chain`, `fr`, `hj`, `mx`, `nl`, `nr`, `os`, `qh` | les mêmes sans `archetype_chain` | 17 / 26 |
   | C7 | `pico8_jo_struct_init` : `self->has_mi = 0;` → `1` | `archetype_chain`, `ga`, `jo` | `ga`, `jo` | 26 / 26 |
   | C8 | `pico8_nl_struct_new` : `od = pico8_nl_start.od` → `pico8_qh_start.od` (masque perdu) | `archetype_chain`, `jo_spec_defaults`, `nl` | `jo_spec_defaults`, `nl` | 25 / 26 |
   | C9 | `pico8_jo_kw_start` `{0, 0x0, 0x0}` → `{0, 0x0, 0x10000}` | `archetype_chain`, `hp`, `jo` | `hp`, `jo` | 26 / 26 |
   | C10 | `pico8_qh_start.dw` `0x666` → `0x667` | `archetype_chain`, `hj`, `mx`, `nl`, `os`, `pm`, `qh` | les mêmes sans `archetype_chain` | 26 / 26 |
   | C11 | `pico8_jo_struct_init` : `ma = pico8_jo_start.ma` → `.ca` | `archetype_chain` + 15 TU | les 15 mêmes | 0 / 26 |
   | C12 | `pico8_os_struct_new` : `hs = pico8_os_start.hs` → `pico8_jo_start.hs` (masque perdu) | `archetype_chain`, `jo_spec_defaults`, `os` | `jo_spec_defaults`, `os` | 26 / 26 |

   Aucun ÉCHEC : au commit, chaque jumelle rougit ce que le parent rougit, plus le test ponctuel. Aucune n'est verte au parent : chaque littéral de chaîne de la famille d a déjà un filet. C'est le contraire de C3 en 2.12c.

4. **Couverture** : `gcov` du TU (`gcc --coverage -O0` avec les 60 `ENGINE_SRCS`, lancé depuis la racine de la copie). Mesuré : **309 / 310** lignes (99,68 %). La seule ligne non exécutée est le `fprintf` d'échec de `same`. Les quatre fonctions et les cinq aides sont exécutées.

5. **Objets** (sans `-g`, `gcc -std=c99 -Wall -Wextra -Werror -Isrc -c`, parent puis commit, 155 `.c` compilables sans raylib vendorisé, `-DPICO8_CLASS_COVERAGE` pour `main_class_coverage.c` et `test_class_coverage.c`) :
   - **153 identiques** octet à octet, dont tous ceux qui incluent `game_data.h`, `planet.h`, `planet_family.h`, `probe.h` ;
   - **2 diffèrent** : l'objet du TU (attendu) et `pico8_game_data.o`. Son `nm` ne gagne que `pico8_jo_def`, `pico8_qh_def`, `pico8_nl_def` et `pico8_os_def`. Son `objdump -d` ne diffère que par deux immédiats `__LINE__` (`0x2ea` → `0x34b`, `0x2f3` → `0x354`, +97). Sous `-DNDEBUG`, son `.text` est **identique** (405 octets).

6. **Règle 1, preuve `const`** (copie du commit ; chaque membre d'instance décrit par l'archétype rendu `const` dans sa struct, un à la fois — `*const` pour un pointeur ; `gcc -fsyntax-only` des 155 `.c` ; erreurs neuves listées par fonction). Mesuré : **29 membres, 127 sites**, exactement les sites de l'annexe A de l'étude 2.9, aux décalages de ligne près :
   - `jo` (14) : `hf` 3, `hs` 2, `fa` 2, `kq` 3, `ka`/`dz`/`cq`/`ca`/`em` 1 chacun (inits, A) ; `gq` 6 (init + 5 TU, que 2.13 réécrit) ; `ma` 3 (`+ entity_geom.c:79 pico8_ma`, fuite, et `jo_kd` : I) ; `mi` 12, `has_mi` 12 (inits, `bb_visit`, 9 TU : I ; `test_pico8_mx.c` décalé de −14, `:524-525` au lieu de `:538-539`) ; `kw_own` 5 (init + 4 dans `jo_lz` : I) ;
   - `qh` (5) : `hp` 6, `px` 6, `qh` 4, `ba` 2 (inits, A) ; `s` 9 (`+ nr_hb` : I) ;
   - `nl` (3) : `rr` 1, `nl` 1 (A) ; `qw` 3 (`+ nl_ly`, `T:qh.c:533`) ;
   - sonde, rejouée (7) : `dw` 7 (dont `test_pico8_jc.c:893`, déjà vu à 2.12c), `od` 12, `has_od` 6, `dq` 4, `fp` 5, `ik` 4, `has_ik` 4.
   Tout autre site est un arrêt « champ sans règle ». Pièges : `kq`, `ka`, `dz`, `cq`, `gq`, `em`, `px`, `ba`, `rr`, `dq` sont des pointeurs à rendre `*const` ; `kw_own` rendu `const` interdit aussi toute copie de struct `pico8_jo_t`, et il n'y en a aucune ; `pico8_probe_t` est `typedef struct pico8_probe {`. `me`/`t` de l'en-tête ne sont pas rejoués (écrits par toutes les inits, comme en 2.12c).

7. **Inclusion, casts, aiguillage** : `inclusion-mutuelle` **17 / 16**, **43 lignes** ; greps de Sortie 4 et 5. Le TU inclut `planet.h` et `planet_family.h` : `tests/` n'entre pas dans le compte, qui ne bouge pas.

8. `class-coverage-net` : **non**. `sanitize-net` : **non**.

### Greps de Sortie (sur l'arbre du commit)

1. `grep -c 'const pico8_[a-z_]*_def_t pico8_[a-z_]*_def = {' src/pico8_game_data.c` → **19** (15 + 4).
2. `grep -c 'pico8_jo_def_t' src/pico8_game_data.h` → **5** (le type, quatre `extern`) ; `grep -rlwE 'pico8_jo_def_t|pico8_jo_def|pico8_qh_def|pico8_nl_def|pico8_os_def' src tests` → `src/pico8_game_data.c`, `src/pico8_game_data.h`, `tests/test_pico8_archetype_chain.c`, rien d'autre.
3. `git diff <parent> -- src | grep -E '^\+.*_start'` → vide.
4. `grep -rnE '\bdef\b *;|->def\b|\.def\b' src` → vide.
5. `git diff <parent> | grep -E '^\+.*\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)'` → vide ; `git diff <parent> | grep -E '^\+.*(->cls|header\.cls|_struct_class\(\)|emitter_of|probe_of|pico8_g__of|pico8_qh_of|pico8_nl_of|_is_family)'` → vide.
6. `make composition-residue` → pied 1073 / 85, `défauts 669 52`, `inclusion-mutuelle 17 16` ; `--champs` → `282 ; 15 ; 19` ; `wc -l src/pico8_game_data.h` → 745.
7. `git diff --stat <parent>` : `src/pico8_game_data.h`, `src/pico8_game_data.c`, `tests/test_pico8_archetype_chain.c`, `docs/plan-3.md`, et rien d'autre (sur la copie, sans `docs/` : 3 fichiers, 238 insertions, 1 suppression).

### Tableau « Runs »

Celui de ce PRD, sur la copie jetable (`p` = 2.12c, d'abord appliquée d'après son PRD sur `e288dbf` (`d5aa77e` dans la copie), puis `7d7f194` ; `c` = 2.12d commitée dans la copie).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout e288dbf` ; texte de 2.12c appliqué (script, 8 ancres) et commité | — | 0 | 3 fichiers, 150 insertions, 4 suppressions ; `game_data.h` 702 | 3 s |
| 2 | `timeout 900 make -j4 test` ; `make e2e` ; compteur ; `--champs` ; `--check` | `p` | 0 | `all tests passed` ; 26 conformes ; 1073 / 85 ; 669 ; 43 ; 282 / 15 / 20 | 20 s |
| 3 | 2.12d, 1er jet : `kw_own` comparé sur les quatre sortes (234) ; `make test` | `c`₀ | 2 | `archetype differs from its chain: qh.kw_own` | 15 s |
| 4 | diagnostic (`fprintf` de `kw`, `kw_own`, `hn`) | `c`₀ | 0 | `qh`/`nl`/`os` : `kw == &kw_own`, `has_qe = 1`, `qe` `0x180000`/`0xc0000`/`0x1c0000`, `hn` non NULL : `lz` a tourné (exception 2) | 15 s |
| 5 | 2.12d, texte de ce PRD (`kw_own` sur `jo` seul, 231) ; `make test` ; `make e2e` ; compteur ; `--champs` ; `--detail` ; `diff` `p`/`c` | `c` | 0 | `all tests passed` ; 26 conformes ; 1073 / 85 ; 669 ; 43 ; **282 / 15 / 19** ; seul `pico8_jo_t kw_own` quitte le groupe ; `info game_data.h 702 → 745` | 25 s |
| 6 | essai e1/e2 : six valeurs, six fonctions de TU ; `make unit` ; compteur ; `--champs` ; puis `git checkout -- .` | `c` + essai | 0 | `e-check total 440` ; 669 ; 43 ; 19 ; `game_data.h` 751 | 30 s |
| 7 | M1 à M14 et X1, 1er passage (sans étiquettes) | `c` | — | `archetype_chain` seul ; 26 conformes ; X1 vert | 106 s |
| 8 | M1 à M14 et X1, script qui lit l'étiquette `differs from its chain: …` | `c` | 0 | étiquettes du tableau ; M10 : l'assert du total | ≈ 100 s |
| 9 | C1 à C12 au commit, puis `checkout p`, C1 à C12 | `c` / `p` | 0 | tableau des jumelles ; aucun ÉCHEC | ≈ 150 s |
| 10 | `constproof.py`, 29 membres | `c` | 0 | 127 sites, ceux de l'annexe A (décalés) | 19 s |
| 11 | objets sans `-g`, `p` puis `c` ; `cmp`, `nm`, `objdump -d` ; `-DNDEBUG` `.text` de `game_data` | `p` / `c` | 0 | 157 `.c`, 2 non compilables des deux côtés (raylib non vendorisé) ; 153 identiques ; `game_data.o` : 4 symboles, `__LINE__` +97 ; `.text` identique (405) | 60 s |
| 12 | `gcc --coverage` du TU + `ENGINE_SRCS`, depuis la racine ; `gcov -b` | `c` | 0 | 309 / 310 lignes | 30 s |
| 13 | greps de Sortie | `c` | 0 | conformes | 3 s |
| 14 | commentaires du TU resserrés (deux lignes), `make unit` ; `rm -rf` de la copie | `c` | 0 | vert ; copie supprimée | 20 s |
| 15 | la branche a avancé (2.12c `168208e`, 2.11r-e `1f5e33d`, `7d7f194`) : copie neuve ; le texte de 2.12c du PRD comparé aux trois fichiers de `168208e` | — | 0 | identiques octet pour octet | 5 s |
| 16 | à `7d7f194` : `make test`, `make e2e`, compteur, `--champs` ; 2.12d appliqué ; diff comparé à celui de l'essai ; `make test`, `make e2e`, compteur, `--champs` ; `rm -rf` | `p` / `c` | 0 | `p` : 26, 1073 / 85, 669, 43, 20, 702, `== 131` ; diff identique à l'essai (hors `index`/`@@`) ; `c` : vert, 26, 1073 / 85, 669, 43, **19**, 745 ; copie supprimée | 60 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas `7d7f194` ou un descendant qui ne touche que `docs/`, ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) est rouge, si un `*.sha256` ou `tests/e2e/` change, ou si une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si le TU ne compte pas 231 comparaisons (100 de plus : `jo` 16, `qh` 27, `nl` 30, `os` 27), si `kw_own` de `qh`/`nl`/`os` est comparé à l'archétype, ou si une comparaison lit un `pico8_X_start`.
- `FAILED:` s'il existe plus d'un type d'archétype pour les sortes de planète, ou si `pico8_jo_def_t` n'a pas ses 33 membres (dont `ng`, `fz`, `iu`, `ms`, `hz`, `mt`, `ht`, `emitter`).
- `FAILED:` si une mutation M1 à M14 est verte au commit, ou rougit un autre TU que `archetype_chain` ou un e2e.
- `FAILED:` si une jumelle C1 à C12 est verte au commit et rouge au parent.
- `FAILED:` si un objet autre que `game_data` et le TU diffère sans `-g`, ou si `game_data` diffère sous `-DNDEBUG` dans son `.text`.
- `FAILED:` si la preuve `const` trouve un site hors de l'annexe A (aux décalages près) autre que `test_pico8_jc.c` `test_bv_takes_the_first_slipway_of_cg`, ou une écriture hors construction d'un champ A.
- `FAILED:` si `défauts` n'est pas 669, `inclusion-mutuelle` 17 / 16 (43 lignes), le pied 1073 / 85, ou `--champs` autre que 282 / 15 / 19 sans explication par la même lecture.
- `FAILED:` si un `def` entre dans une instance ou un composant, si `planet.h`, `planet_family.h` ou `probe.h` change, ou en cas de tassement.
- `FAILED:` si un commentaire ajouté ne porte ni un contrat ni un piège (`CLAUDE.md`).
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si le commit embarque une ligne étrangère à 2.12d (`docs/plan-3.md` hors ligne 2.12d, `tools/`, un autre TU).
- `FAILED:` si le message du commit omet : le type planète unique et ses 33 membres, les quatre valeurs, la répartition des 100, l'écart 103 → 100 et sa mesure (exception 2), l'essai e1/e2 (440), les mutations M, le témoin X1 et les jumelles C avec leur résultat au parent, la couverture, les objets, la preuve `const` (29 membres, 127 sites), les comptes dont `--champs` 20 → 19 et sa raison, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.12d » (dont 2.12e1 : 120 et 2.12e2 : 89), ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Les valeurs de `hj`, `pm`, `mx`, `ga`, `nr`, `fr` et leurs fonctions de TU : 2.12e1 et 2.12e2 (essai ci-dessus, non commité).
- Le membre `def` de `pico8_jo_t`, toute lecture de l'archétype, la suppression de `pico8_jo_start` … `pico8_os_start` et de `pico8_jo_kw_start` : 2.13d.
- La correction des chiffres des lignes 2.12e1/e2 du plan : leurs PRD (la note de 2.12d les annonce).
- `tools/composition_residue.sh` (`--champs` se dit majorant) ; `class-coverage-net`, `sanitize-net` ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Un seul type `pico8_jo_def_t`, complet dès 2.12d** : décision A de l'humain et étude §4. Les membres propres des sortes e (`ng`, `fz`, `iu`, `ms`, `hz`, `mt`, `ht`, `emitter`) sont déjà là, à zéro dans les quatre valeurs, et rien ne les lit.
- **G2. 100 comparaisons, pas 103** : l'exception 2 de l'étude est une règle écrite, le compte de 103 l'oublie, et la mesure la confirme (étape 3 des Runs). Le plan chiffrait un total, pas une règle : on garde la règle. Comparer `kw_own` sur l'arbre `qh` exigerait soit une valeur d'archétype égale à la sortie de `lz` (ce n'est plus la chaîne d'init), soit des sortes construites avant que `gb` soit rempli (autre ordre du `main` du TU, que l'étude n'a pas prévu).
- **G3. Ordre des membres** : celui de l'arbre et de l'instance. D'abord `me`/`t`, puis `jo` (sans `kw` ni `by`), puis `qh`. Ensuite les noms propres dans l'ordre des sortes (`nl` : `rr`, `nl`, `qw` ; `pm` : `ng`, `fz` ; `ga` : `iu`, `ms` ; `nr` : `hz` ; `fr` : `mt` ; `mx` : `ht`). Enfin `probe` (porté dès `qh`), puis `emitter` (dès `hj`), comme le texte C de l'étude. Il n'est pas normatif, comme en 2.12b et 2.12c (G3).
- **G4. `kw_own` par valeur, positionnel** : étude §2, et `défauts` 670 si l'on nommait `pico8_jo_kw_start`. En positionnel, `--champs` ne voit que le nom `kw_own`.
- **G5. Aides par partie dans le TU** : 2.12a à 2.12c écrivaient chaque comparaison en ligne. Pour quatre, puis dix sortes du même type, les parties `jo` et `qh` se répètent à l'identique (15 et 12 lignes). Les écrire dix fois ferait environ 400 lignes de TU, que 2.13 retire. `same_in` garde les étiquettes `sorte.champ`, que M1 à M14 lisent. Les fonctions de 2.12a à 2.12c ne sont pas réécrites.
- **G6. Commentaires** : sur le type, un contrat (ce que valent les membres des autres sortes, ce qui n'y est pas) ; `dz` recopie le commentaire de contrat de `pico8_jo_t`. Dans le TU, un contrat (`same_jo_part` : ce qu'il ne compare pas), un piège (`assert_lz_ran` : pourquoi `kw_own` n'est pas comparé sur l'arbre `qh`) et une exception (`test_jo` : `kw`), comme `test_ui`/`test_mz`.
- **G7. Quatorze mutations, douze jumelles et un témoin** : M3, M6 et M13 couvrent un masque propre à `os`, un masque de sonde et une donnée héritée ; C5 à C8, C11 et C12 mutent un littéral ou une affectation de `_new`/init plutôt qu'un `_start` ; C9 mute la donnée que `kw_own` recopie ; X1 montre ce que l'exception 2 laisse sans filet.

### Surprises de mesure, pour `plan-pilot`

1. **103 → 100** (et 2.12e1 123 → 120, 2.12e2 91 → 89) : l'étude compte `kw_own` sur les neuf sortes de l'arbre `qh` alors que son exception 2 l'en retire. `lz` tourne dans le TU (`gb` rempli par `pico8_cart_load`).
2. **`--champs` 20 → 19** par les quatre `.kw_own = {…}` ; les six valeurs de la famille e ne le font plus bouger (essai).
3. **Le `kw_own` des archétypes de l'arbre `qh` n'a aucun filet** (X1 vert) : `lz` l'écrase à chaque `_new` quand `gb` a la recherche. À noter pour 2.13d.
4. Toutes les jumelles de chaîne de la famille d ont déjà un filet au parent : le test ponctuel n'est le seul filet d'aucun littéral.

### Questions pour l'humain

Aucune. L'écart 103 → 100 suit une règle déjà écrite dans l'étude (exception 2), et la mesure la confirme.
