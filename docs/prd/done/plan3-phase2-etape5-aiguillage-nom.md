---
title: "Plan 3, étape 2.5 : chaque position d'un projet de recherche écrit son membre en dur, `o`/`dm` passant par `r` ; `mr`, `gp`, leurs tables et `pico8_ho_gp_has` retirés (`aiguillage-nom` 30 → 0, gardé ; `défauts` 698 → 694 ; pied 1320/91 → 1291/89) — un commit, jeu identique"
description: "La construction d'un `hp` cherchait, pour chaque position d'un enregistrement de `fd`, le nom du membre dans une table (`mr`), puis ce nom dans une seconde table (`gp`) pour savoir s'il passe par `r`. Les deux tables sont des constantes de classe : l'aiguillage devient une affectation directe position → membre, les tables, leurs membres et leur aide partent, et `aiguillage-nom` entre dans la garde. En passant, `fold_pk` du TU `hj` lit la classe sans cast."
status: needs-triage
---

## Problem Statement

**Écrit sur `ac0d09a`** (branche `claude/plan-pilot-3-7zdft0`, après 2.4 vérifiée). Aucune ligne du dépôt n'a changé pour écrire ce PRD. Les comptes et les preuves viennent d'une seule copie jetable de `ac0d09a` (parent et enfant par `git stash`), supprimée depuis. Tous les numéros de ligne ci-dessous sont ceux de `ac0d09a`.

Vu du mainteneur du port : dans le cart, `hp:pq` appelle `ho(gb, hp.mr, hp.gp)(self)` (`lua.lua:1403`). `ho` (`lua.lua:17-23`) parcourt `mr = o("px","lr","fj","ja","i","o","il","dm")` et, pour chaque position `i` de nom `p`, écrit `ri[p] = gu(gp,p) and r[ri[i]] or ri[i]`, avec `gp = o("o","dm")` (`lua.lua:1400-1401`). Le port recopie cet aiguillage **par nom** :

- `hp_ho` (`research.c:104-129`) lit `self->mr->items[p]`, demande à `pico8_ho_gp_has(self->gp, name)` (`collect.c:111-120`) si le nom passe par `r`, puis
- `hp_pose` (`research.c:55-101`) aiguille la **position** `p` vers son membre par un `switch`.

Le nom lu dans `mr` ne sert donc qu'à interroger `gp` : la position suffit déjà à `hp_pose`. `mr` et `gp` sont des constantes de classe (`pico8_hp_start`, `game_data.c:461-465`), jamais écrites ailleurs (`--champs` : `pico8_hp_t mr 2 1`, `gp 2 1`, les deux écritures du constructeur). Ce sont les 30 lignes `aiguillage-nom` du compteur, et deux champs qu'on écrirait sinon dans l'archétype de `hp` (2.12) pour les voir mourir aussitôt (plan, « Champs qui meurent avant les archétypes »).

**Pourquoi rien ne change.** Les deux tables sont fixes : position 1..8 = `px lr fj ja i o il dm`, et seules les positions 6 (`o`) et 8 (`dm`) passent par `r`. L'affectation directe écrit les mêmes membres, avec les mêmes valeurs, dans les mêmes conditions (`npos`, `il_false`, chaîne absente). Les deux empreintes du TU `hp` (`GB_HASH`, `DEFAULTS_HASH`) restent identiques, mesuré.

### Texte du plan, recopié (`docs/plan-3.md` à `ac0d09a`, ligne 2.5)

> - 2.5 — `aiguillage-nom` : `hp_ho`/`hp_pose` (`research.c:61-131`) remplacés par l'affectation directe position → membre (`px`, `lr`, `fj`, `ja`, `i`, `o`, `il`, `dm`), les deux positions que `gp` nomme (`o`, `dm`, `game_data.c:273`) passant par `pico8_r_lookup` ; `mr`, `gp`, `pico8_hp_mr_names`, `pico8_hp_gp_names` et `pico8_ho_gp_has` (`collect.c:111`, seul appelant `research.c:118`) retirés. Les `assert` de genre par position restent, messages compris. Modules : `research`, `collect`, `game_data`, TU `hp`, `spec_defaults`. Compte : `aiguillage-nom` 30 → **0**, gardé ; `défauts` 698 → **694** ; pied → **1291 / 89**. Preuves : chemin touché (`gcov` de la construction de `gb` ; mutations : `o` sans passage par `r`, `dm` et `o` échangés, `npos` ignoré). `class-coverage-net` : oui. `sanitize-net` : non. Budget ≈ 50 min.

Les numéros du plan ont glissé depuis son écriture : `hp_pose`/`hp_ho` sont à `research.c:55-129`, l'appel de `pico8_ho_gp_has` à `:116`. Rien d'autre ne diffère.

Règles communes de la phase (plan, « Preuves communes à chaque ligne ») : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` ni `tests/e2e/` modifié ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée. Une catégorie tombée à 0 entre dans `COMPOSITION_RESIDUE_GUARD`, avec une preuve de garde mordante.

### Réserve héritée de 2.4 (vérificateur)

`tests/test_pico8_hj.c:186` (`fold_pk`) lit la classe par `((const pico8_struct_header_t *)hj)->cls`. Depuis 2.3c, l'en-tête est atteignable sans cast : `hj->qh.jo.header.cls`. C'est **équivalent** : `header` est le premier membre de `pico8_jo_t` (`planet.h:26`), `jo` celui de `pico8_qh_t` (`planet_family.h:32`), `qh` celui de `pico8_hj_t` (`planet_family.h:72`), et un pointeur vers une struct désigne son premier membre (C99 6.7.2.1 §13). Mesuré : l'objet `test_pico8_hj.o` compilé `-std=c99 -O2 -Wall -Wextra -Isrc` avant et après est **identique octet à octet** (`cmp`). Le chemin nommé est celui que le TU utilise déjà ailleurs (`:456`, `:559`). La correction entre dans ce commit (même TU que rien d'autre ici, mais trop petite pour un commit seul).

### État de départ attendu (`ac0d09a`, mesuré)

| Commande | Résultat |
|---|---|
| `make composition-residue` | `parente 10 3`, `lien-parent 111 27`, `accesseurs-q6 262 33`, `défauts 698 52`, `nu 208 37`, **`aiguillage-nom 30 8`**, `void-ptr 11 7`, `ptrs-entité 7 4`, `atteinte-à-prouver 1 1`, `inclusion-mutuelle 17 16` ; **`résidu 2 : 1320 ligne(s), 91 fichier(s)`** |
| `sh tools/composition_residue.sh --check aiguillage-nom` | 30 lignes (liste ci-dessous), code 1 |
| `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` | `864:COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux` |
| `--champs` | `champs : 289 ; 0 écriture : 15 ; 0 hors constructeur : 27` |
| `make line-coverage` sur les 26 e2e | 6334 lignes |
| `make test` ; `make e2e` | vert ; 26 conformes |

## Solution

Un seul commit : le code, les TU, le `Makefile` et le plan ne compilent ou ne se vérifient qu'ensemble.

- `hp_pose` et `hp_ho` sont remplacés par **`hp_positions`**, qui écrit chaque position 1..`npos` dans son membre en dur. Deux petites aides portent les `assert` de genre, messages inchangés : `hp_string` (positions `px`, `lr`, `ja`, `i`) et `hp_r` (positions `o`, `dm` : chaîne présente, puis `r[valeur]` par `pico8_r_lookup`).
- `mr` et `gp` quittent `pico8_hp_t` et `pico8_hp_start_t` ; `pico8_hp_mr_names`, `pico8_hp_gp_names` et `pico8_ho_gp_has` disparaissent.
- Le TU `hp` plie désormais les deux tables du cart comme constantes locales : `DEFAULTS_HASH` ne bouge pas (même principe qu'à 2.4 : le TU connaît la constante de sa classe).
- `aiguillage-nom` entre dans la garde : `COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom`.
- `fold_pk` (TU `hj`) lit `hj->qh.jo.header.cls`.

## User Stories

1. En tant qu'auteur des archétypes (2.12), je veux que `mr` et `gp` n'existent plus sur `hp`, afin de ne pas leur écrire un archétype qui mourrait aussitôt.
2. En tant que lecteur de la construction d'un projet, je veux lire « position 6 → `o`, par `r` » en une ligne, afin de ne plus suivre deux tables de noms pour savoir où va une valeur.
3. En tant que gardien des `assert` de genre, je veux que chaque message (`hp.px/lr/ja/i: …`, `hp.fj/o/dm: …`, `hp: a value that gp maps through r is missing from r …`) reste mot pour mot, afin qu'une donnée mal typée s'arrête au même endroit avec le même texte.
4. En tant que gardien du jeu, je veux `make e2e` à 26 conformes, aucun `*.sha256` touché, afin que le jeu reste le même image par image.
5. En tant que gardien des empreintes, je veux `GB_HASH` et `DEFAULTS_HASH` inchangés, afin d'être sûr que chaque membre de chaque projet reçoit la même valeur qu'avant.
6. En tant que vérificateur, je veux le `gcov` de la construction de `gb`, l'ensemble `L` et des mutations rejouées au parent, afin de savoir qu'aucun chemin ne perd son test.
7. En tant que gardien du compteur, je veux `aiguillage-nom` dans la garde, et la preuve qu'une ligne réinjectée rougit `make test`.
8. En tant que lecteur du glossaire, je veux que les entrées `ho`, `mr`/`gp` disent ce qui les remplace, afin de ne pas chercher `hp_ho` ni `pico8_ho_gp_has`.
9. En tant que vérificateur de 2.4, je veux `fold_pk` sans cast, afin que le TU `hj` lise l'en-tête par son chemin nommé comme partout ailleurs.
10. En tant que lecteur du plan, je veux une note « Découvert à 2.5 » courte, afin que 2.6 parte des vrais comptes.

## Implementation Decisions

### Modules

`research`, `collect`, `game_data` ; TU `hp`, `spec_defaults`, `hj` (réserve de 2.4) ; `Makefile` ; `docs/glossaire-noms.md` ; `docs/plan-3.md`. Le plan ne nomme ni le TU `hj` (réserve ajoutée) ni le glossaire (deux entrées nomment des fonctions qui disparaissent : règle 1 de `CLAUDE.md`).

### Ce qui change, fichier par fichier (numéros de `ac0d09a`)

**`src/pico8_research.c`**
- `:8` `#include "pico8_collect.h"` : retiré. `pico8_ho_gp_has` en était le seul usage (mesuré : aucun autre symbole de `collect.h` dans `research.c` ; `pico8_r_lookup` vient de `map_tables.h`, inclus par `research.h:6`). Compile sous `-Werror`.
- `:55-129` (commentaire de `hp_pose`, `hp_pose`, commentaire de `hp_ho`, `hp_ho`) remplacés par, texte exact essayé sur copie :

```c
static const char *hp_string(const char *s) {
    assert(s != NULL && "hp.px/lr/ja/i: a position that is not a string does not fit the member's type "
                        "-- stop, do not widen the member");
    return s;
}

/* `r[s]`, for the two positions `gp` names: `o` and `dm`. */
static pico8_fixed_t hp_r(const char *s) {
    int qe = 0;
    int found;

    assert(s != NULL && "hp.fj/o/dm: a position that is not a number does not fit the member's type "
                        "-- stop, do not widen the member");
    found = pico8_r_lookup(s, &qe);
    assert(found && "hp: a value that gp maps through r is missing from r; it would stay a string in a "
                    "fixed member -- stop, do not widen the member");
    (void)found;
    return pico8_fixed_from_int(qe);
}

/* `ho(gb, mr, gp)` on one record of `fd`: positions 1..npos are `px`, `lr`,
 * `fj`, `ja`, `i`, `o`, `il`, `dm`. `i` gets the raw string that `hp:pq`
 * turns into its list afterwards. */
static void hp_positions(pico8_hp_t *self, const pico8_fd_entry_t *ri, const char **i_src) {
    int n = ri->npos;

    assert(ri->header == NULL && "hp: a header entry of fd is the rr of the records after it, not a project");
    assert(n <= 8 && "hp: fd has eight positions, px to dm");
    if (n >= 1) {
        self->px = hp_string(ri->px);
    }
    if (n >= 2) {
        self->lr = hp_string(ri->lr);
    }
    if (n >= 3) {
        self->fj = pico8_fixed_from_int(ri->fj);
        self->has_fj = 1;
    }
    if (n >= 4) {
        self->ja = hp_string(ri->ja);
    }
    if (n >= 5) {
        *i_src = hp_string(ri->i);
    }
    if (n >= 6) {
        self->o = hp_r(ri->o);
        self->has_o = 1;
    }
    if (n >= 7) {
        self->il = ri->il_false ? NULL : ri->il;
        self->has_il = 1;
    }
    if (n >= 8) {
        self->dm = hp_r(ri->dm);
        self->has_dm = 1;
    }
}
```

  Correspondance avec l'ancien code : une position `o`/`dm` absente (`s` NULL) échouait à `hp_pose` sur « `hp.fj/o/dm` » sans passer par `r` ; `hp_r` fait le même test **avant** la recherche, même message. Une valeur absente de `r` échoue sur le même message qu'avant. `fj` n'a pas de chaîne (`int` de `fd`), il n'a jamais eu d'`assert`. L'assert d'`il` (« `hp: \`il\` gets a number` ») part : voir G2.
- `:156-157` `self->mr = NULL;`, `self->gp = NULL;` et `:179-180` `self->mr = pico8_hp_start.mr;`, `self->gp = pico8_hp_start.gp;` : retirées.
- `:186` `hp_ho(self, ri, &i_src);` → `hp_positions(self, ri, &i_src);`.

**`src/pico8_research.h`**
- `:27-28` « `mr`, `gp`, `fk` point at `pico8_hp_mr_names`, `pico8_hp_gp_names`, `pico8_hp_fk_row`; the other » → « `fk` points at `pico8_hp_fk_row`; the other » (une ligne).
- `:35-36` `const pico8_str_seq_t *mr;`, `*gp;` : retirées.

**`src/pico8_collect.h`**, **`src/pico8_collect.c`**
- `collect.h:37-40` (ligne vide, commentaire deux lignes, prototype de `pico8_ho_gp_has`) : retirées.
- `collect.c:110-120` (ligne vide et corps de `pico8_ho_gp_has`) : retirées. `<string.h>` reste (`strcmp` à `:79`).

**`src/pico8_game_data.h`**, **`src/pico8_game_data.c`**
- `game_data.h:242-243` (`mr`, `gp` de `pico8_hp_start_t`) et `:500-501` (`extern` des deux tables) : retirées. `pico8_hp_start_t` garde `fk` seul.
- `game_data.c:270-275` (`hp_mr_items`, `pico8_hp_mr_names`, ligne vide, `hp_gp_items`, `pico8_hp_gp_names`, ligne vide) et `:462-463` (`.mr = …`, `.gp = …`) : retirées.

**Tests** (aucun `*_HASH` ni `FROZEN_*` touché ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide, mesuré)
- `test_pico8_hp.c`, après `fold_names` (`:168`), ajouter les tables du cart en constantes du TU :

```c
/* The cart's `mr` and `gp` of `hp`, gone from the port since each position
 * writes its member directly; the frozen defaults fold still reads them. */
static const char *const cart_mr_items[] = {"px", "lr", "fj", "ja", "i", "o", "il", "dm"};
static const pico8_str_seq_t CART_MR = {8, cart_mr_items};
static const char *const cart_gp_items[] = {"o", "dm"};
static const pico8_str_seq_t CART_GP = {2, cart_gp_items};
```

  (Le commentaire ne doit **pas** écrire `hp.mr` : le motif `(->|\.)(mr|gp)\b` le compterait, et la garde rougit — piège rencontré sur copie.)
- `test_pico8_hp.c:416` commentaire « the class's `mr`/`gp`/`fk` shared by all » → « the class's `fk` shared by all ».
- `:421-422` `void *mr = NULL;`, `void *gp = NULL;` : retirées.
- `:475-478` → `fk = (void *)b->fk; assert(fk != NULL);` ; `:480` → `assert((void *)b->fk == fk);`.
- `:488-490` → `assert(fk == &pico8_hp_fk_row);`, `d = fold_names(d, &CART_MR);`, `d = fold_names(d, &CART_GP);`. `:493` `check("mr gp fk", d, DEFAULTS_HASH);` inchangé.
- `:934-938`, commentaire de `test_hp_is_a_struct` : « `mr`/`gp`/`fk` the class's » → « `fk` the class's » ; « more positions than `mr` » → « more than eight positions » ; « only `mr`/`gp`/`fk` » → « only `fk` ».
- `:960` retirée ; `:962` → `assert(bare->fk == pico8_hp_start.fk);` (reste en `défauts`).
- `:973` aiguille `"ri->npos <= self->mr->count"` → `"hp: fd has eight positions"`.
- `test_pico8_spec_defaults.c:201-202` (`hp->mr`, `hp->gp`) : retirées. `:203` (`fk`) reste.
- `test_pico8_hj.c:186-187` (réserve 2.4) : `return hj->qh.jo.header.cls == pico8_pm_struct_class() ? fold_bool_member(h, hj->emitter->pk != 0)` / `: fold_fixed_member(h, hj->emitter->pk);` (le `:` réaligné sous le `?`).

**`Makefile`**
- `:864` → `COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom`. Rien d'autre.

**`docs/glossaire-noms.md`**
- `:292` (`ho(rc,mr,gp)`) : colonne port → « `hp_positions` (`static`, `pico8_research.c`) : chaque position écrit son membre, `o`/`dm` par `hp_r` ; `pico8_ho_apply` supprimé (D.8) ».
- `:293` (`pico8_ho_gp_has`) : ligne retirée.
- `:631` (`mr`, `gp`) : garder le sens du cart, remplacer la parenthèse des tables par « plus de table au port : l'ordre est celui de `hp_positions`, et `o`/`dm` passent par `hp_r` ».

**`docs/plan-3.md`**
- Note « Découvert à 2.5 » au bout de la ligne 2.5 : comptes, valeur de la garde, et pour 2.12 : l'archétype de `hp` n'a plus que `fk`. Le reste va au message.

Rien d'autre : ni `trous-de-portage.md` (aucun écart avec le cart), ni `tools/composition_residue.sh`, ni `tests/test_composition_residue.c`, ni `tools/class_coverage_net_expected.txt`.

### Disposition mémoire

`pico8_hp_t` perd deux pointeurs en tête (après `header`) ; `pico8_hp_start_t` n'a plus qu'un membre. Rien ne dépend de ces offsets : aucun `offsetof` dans `src/` ni `tests/`, seul `sizeof *self` du `malloc` (`research.c:150`), aucun `memcpy` d'un `hp` (`git grep` mesuré). Durée de vie et propriétaire inchangés : `sanitize-net` non exigé, comme le dit le plan.

### Comptes, mesurés sur copie de `ac0d09a`

| Catégorie | parent → 2.5 |
|---|---|
| `aiguillage-nom` (lignes / fichiers) | 30 / 8 → **0**, disparue de la sortie, gardée |
| `défauts` | 698 / 52 → **694 / 52** (`research.c` 4 → 2, `test_pico8_spec_defaults.c` 80 → 78) |
| les neuf autres | inchangées (`parente 10 3`, `inclusion-mutuelle 17 16`, …) |
| pied | 1320 / 91 → **1291 / 89** (−29 lignes, −2 fichiers : `collect.c` et `research.h`, qui n'avaient que des lignes `aiguillage-nom`) |
| `--champs` | 289 → **287** champs (`pico8_hp_t gp`, `mr` partent) ; `0 écriture : 15 ; 0 hors constructeur : 27` inchangés |

Pourquoi −29 et pas −30 : `test_pico8_hp.c:962` sort de `aiguillage-nom` mais reste en `défauts` (`bare->fk == pico8_hp_start.fk`). Les quatre lignes `défauts` qui partent sont `research.c:179-180` et `spec_defaults.c:201-202`, toutes aussi `aiguillage-nom`. Lignes qui gagnent une catégorie : aucune (`--detail` avant/après : seules `défauts` et `aiguillage-nom` bougent).

Les 30 lignes, classées pour le message :
- **Retirées (24)** : `collect.c:111` ; `collect.h:40` ; `game_data.c:271`, `:274`, `:462`, `:463` ; `game_data.h:500`, `:501` ; `research.c:57`, `:59`, `:104`, `:109`, `:111`, `:116`, `:127`, `:156`, `:157`, `:179`, `:180` ; `test_pico8_hp.c:475`, `:476`, `:960` ; `test_pico8_spec_defaults.c:201`, `:202`. (`research.c:109`, l'ancien `assert` de nombre de positions, est compté ici : sa moitié `self->mr->count` part et la ligne devient celle de `hp_positions`.)
- **Réécrites hors motif** : `research.c:186` (appel), `research.h:28` (commentaire), `test_pico8_hp.c:480`, `:488`, `:962`, `:973`.

24 retirées + 6 réécrites = 30. L'implémenteur recopie la liste de `--check aiguillage-nom` du parent dans le message.

### Anti-tassement

- **Interdit** :
  - garder une table de noms (`"px", "lr", …` ou `"o", "dm"`) dans `src/`, sous un autre nom, pour aiguiller par nom ; `hp_positions` n'a aucune chaîne de nom de membre ;
  - renommer `mr`/`gp`/`pico8_ho_gp_has` pour échapper au motif, ou garder un membre `mr`/`gp` jamais écrit (le motif ne voit pas une déclaration `*mr;`, d'où la grep de Sortie 1) ;
  - changer un message d'`assert` de genre, `tools/composition_residue.sh`, `tests/test_composition_residue.c`, `tests/e2e/`, un `*.sha256` ou une constante d'empreinte ;
  - retirer `aiguillage-nom` de la garde.
- Les tables `CART_MR`/`CART_GP` du TU `hp` ne sont **pas** un aiguillage déguisé : c'est le TU qui plie une constante figée, pas le jeu qui aiguille. Elles ne sont lues que par `fold_names`.

## Implementation Chunks

Un commit, ≈ 35 min preuves comprises sur copie (l'essai complet a pris moins), sous le budget de 50 min. Pas de découpage.

1. Réécrire `research.c` (bloc ci-dessus), retirer les membres, tables, aide et include.
2. Tests `hp`, `spec_defaults`, `hj`.
3. Garde du `Makefile`, glossaire.
4. Preuves, note du plan, commit, push.

**Repli écrit (si le budget déborde à 1,5 fois avant le commit).** Deux moitiés qui compilent seules :
- **2.5a** : `hp_positions` remplace `hp_ho`/`hp_pose`, `pico8_ho_gp_has` et l'include partent, `fold_pk` corrigé ; `mr`/`gp` restent des membres écrits mais plus lus. `aiguillage-nom` 30 → ≈ 20, pied ≈ 1310 / 90 (à remesurer), garde non touchée.
- **2.5b** : membres, tables, `extern`, tests `hp`/`spec_defaults`, garde, glossaire ; `aiguillage-nom` → 0, `défauts` → 694, pied → 1291 / 89.

Un repli est écrit dans la note « Découvert à ». Les comptes finaux ne changent pas.

## Testing Decisions

- **Aucun TU nouveau.** Le comportement ne change pas. `GB_HASH` plie déjà chaque membre de chaque projet de `gb` (`px`, `lr`, `fj`, `ja`, `i`, `o`, `il`, `dm`, et leurs `has_`) : c'est lui qui prouve l'affectation position → membre, et les mutations montrent qu'il la voit. Les six `build_with_*` (`test_pico8_hp.c:838-909`) prouvent les `assert` d'en-tête, de `r`, de nombre de positions et de genre. Les TU ne rejouent jamais un `events.json`. Les e2e partent de l'écran titre (`make e2e`).
- **Prior art** : ensemble `L` de D.20a, 2.3a, 2.4 ; règle du parent de D.21 ; garde mordante de 2.2 et 2.4.

### Preuves (chemin touché)

1. `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. Mesuré.
2. **`gcov` des e2e** (`make line-coverage EVENTS="$(ls tests/e2e/*.json | tr '\n' ' ')"`, dans une copie) au parent puis à l'enfant ; `L` par appariement `difflib`/`diff` des sources. Mesuré : **6334 → 6305**. **`L` = les lignes exécutables retirées ou réécrites** : `collect.c:111`, `:114-116`, `:119` ; `research.c:59-98` (lignes exécutables de `hp_pose`), `:104-127` (celles de `hp_ho`), `:156-157`, `:179-180`, `:186`. Plus un artefact d'appariement : `research.c:100` (`}` du `switch`) apparié au `}` de `hp_string`, que `gcov` ne compte pas ; à déclarer, pas un ÉCHEC. **Toutes** les lignes exécutables neuves de `hp_string`, `hp_r`, `hp_positions` et l'appel sont couvertes par les e2e, les huit branches `n >= k` comprises. Toute ligne **inchangée** dans `L` : arrêt, rapporté.
3. **Mutations**, une à la fois, à l'enfant puis la même au parent sur l'ancienne forme (`make -k unit` puis `make -k e2e`). **ÉCHEC** si verte à l'enfant et rouge au parent. Verte aux deux : déclarée « mutant équivalent » ou « chemin jamais atteint ». Restaurer par `cp` puis `touch`. Mesuré sur copie, identique au parent et à l'enfant :

| # | Mutation enfant ; parent | TU rouges | e2e conformes |
|---|---|---|---|
| M1 | `o` sans passage par `r` : `self->o = hp_r(ri->o);` → `self->o = pico8_fixed_from_int(hp_string(ri->o)[0]);` ; parent `:125` → `fixed = pico8_fixed_from_int(p == 5 ? strings[p][0] : qe);` | 12 : `hj hp hy jo js lq mx nl os pm qh trade` | 7 / 26 |
| M2 | `dm` et `o` échangés : bloc `n >= 6` écrit `dm`/`has_dm`, bloc `n >= 8` écrit `o`/`has_o` ; parent : `case 5` écrit `dm`, `default` écrit `o` | 11 : `hj hp hy js lq mx nl os pm qh trade` | 7 / 26 |
| M3 | `npos` ignoré : `int n = ri->npos;` → `int n = 8;` ; parent `:110` `p < ri->npos` → `p < 8` | 42 (dont `hp`, `spec_defaults`, `replay_from_title`) | 0 / 26 |
| M4 | `il_false` ignoré : `self->il = ri->il;` ; parent `:112` → `const char *s = strings[p];` | aucun | 26 / 26 — **mutant équivalent** : les 4 entrées `il_false = 1` de `map_tables.c` ont `il` NULL |
| M5 | `has_il` non posé (ligne retirée) ; parent : `case 6` sans `has_il = 1` | 1 : `hp` | 25 / 26 |

4. **Garde mordante** (dans une copie) : prototype `int pico8_ho_gp_has(const pico8_str_seq_t *gp, const char *p);` réinjecté avant `#endif` de `collect.h` → `make test` code 2, `aiguillage-nom: src/pico8_collect.h:38:…`, `résidu gardé (noyau-inclut,un-ou-faux,aiguillage-nom) : 1 ligne(s)`, `make: *** [Makefile:867: composition-guard] Error 1`. Mesuré. (Réinjecter `const pico8_str_seq_t *mr;` dans `research.h` ne mord **pas** : le motif ne voit que `->mr`/`.mr` ; c'est la grep de Sortie 1 qui le couvre.)
5. `make class-coverage-net` : **OK** (« couverture: OK … 26 rejeu(x) »), mesuré. `make sanitize-net` : non exigé.
6. `--champs` : 289 → 287, les deux lignes `pico8_hp_t gp`/`mr` partent, rien d'autre. Mesuré.
7. `test_pico8_hj.o` identique avant/après la correction de `fold_pk` (`cmp`, `-O2` sans `-g`). Mesuré.

### Greps de Sortie (sur l'arbre du commit)

1. `grep -rnE '\*(mr|gp);|->(mr|gp)\b|\.(mr|gp)\b' src tests | grep -v test_composition_residue.c` → **vide**.
2. `grep -rnE '\bhp_(pose|ho)\b|pico8_ho_gp_has|pico8_hp_(mr|gp)_names' src tests docs/glossaire-noms.md | grep -v test_composition_residue.c` → **vide**.
3. `sh tools/composition_residue.sh --check aiguillage-nom` → `résidu gardé (aiguillage-nom) : 0 ligne(s)`, code 0.
4. `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` → `COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom`.
5. `grep -n 'pico8_r_lookup' src/pico8_research.c` → 2 lignes (`hp_r` et la boucle de `i`), comme au parent.
6. `grep -n 'pico8_struct_header_t \*)hj' tests/test_pico8_hj.c` → vide.
7. `make composition-residue` → `défauts 694`, pas de ligne `aiguillage-nom`, `parente 10 3`, `inclusion-mutuelle 17 16`, pied **`1291 / 89`**.
8. `git diff --stat <parent>` : `src/` (`research.c`, `research.h`, `collect.c`, `collect.h`, `game_data.c`, `game_data.h`), TU `hp`, `spec_defaults`, `hj`, `Makefile`, `docs/glossaire-noms.md`, `docs/plan-3.md`. Sur copie, hors docs : 10 fichiers, 73 insertions, 125 suppressions.

### Budget et tableau « Runs »

Budget : **≈ 50 min**, preuves comprises. Au-delà de 1,5 fois, repli, sans rien commettre d'incomplet. Jamais l'appli raylib ; `make raylib` n'est pas exigé.

L'implémenteur tient un tableau « Runs », une ligne par lancement. Voici celui des essais de ce PRD, sur une copie de `ac0d09a` (`p` parent, `a` 2.5 appliquée, par `git stash`) :

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make composition-residue` ; `--check aiguillage-nom` ; `--detail` | `p` | 0 ; 1 ; 0 | pied 1320 / 91 ; 30 lignes | < 1 s |
| 2 | `make test` (premier essai) | `a` | 0 | vert, `GB_HASH`/`DEFAULTS_HASH` tenus | ≈ 13 s |
| 3 | `gcc -O2` de `test_pico8_hj.c` avant/après, `cmp` | `a` | 0 | objets identiques | < 1 s |
| 4 | `make test` avec la garde élargie | `a` | 2 | 1 ligne : le commentaire `hp.mr` du TU (réécrit) | ≈ 13 s |
| 5 | `make test` ; `make e2e` | `a` | 0 ; 0 | vert ; 26 conformes | ≈ 13 s ; ≈ 1 s |
| 6 | `make composition-residue` ; `--detail` comparé | `a` | 0 | `défauts 694 52`, `aiguillage-nom` absent, pied 1291 / 89 | < 1 s |
| 7 | `--champs` | `p` / `a` | 0 | 289 → 287 | < 1 s |
| 8 | `make line-coverage` sur les 26 e2e | `a` / `p` | 0 | 6305 / 6334 ; `L` ci-dessus | ≈ 1 min × 2 |
| 9 | M1 à M5, `make -k unit` + `make -k e2e` | `a` / `p` | 2 (M4 : 0) | tableau des mutations | ≈ 30 s × 10 |
| 10 | `make class-coverage-net` | `a` | 0 | `couverture: OK` | ≈ 10 s |
| 11 | `make test`, membre `*mr;` réinjecté dans `research.h` | `a` | 0 | la garde ne mord pas (motif) | ≈ 13 s |
| 12 | `make test`, prototype `pico8_ho_gp_has` réinjecté | `a` | 2 | `composition-guard` Error 1, 1 ligne | ≈ 13 s |
| 13 | `git grep` `offsetof`, `sizeof`/`memcpy` de `hp` | `ac0d09a` | 0 | seul `sizeof *self` du `malloc` | < 1 s |
| 14 | `git diff` hash/e2e/sha256 | `a` | 0 | vide (après réécriture du commentaire du TU) | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas `ac0d09a` (ou un descendant qui ne touche que `docs/`), ou si les comptes de départ diffèrent de 1320 / 91 et `aiguillage-nom 30 8` sans explication.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) reste rouge, si un `*.sha256` ou `tests/e2e/` change, ou si une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si un message d'`assert` de genre (`hp.px/lr/ja/i: …`, `hp.fj/o/dm: …`, `hp: a value that gp maps …`, `hp: a header entry …`) change d'un caractère.
- `FAILED:` si `L` contient une ligne inchangée (hors l'artefact `research.c:100` déclaré), ou si une ligne exécutable neuve de `hp_positions`/`hp_string`/`hp_r` n'est pas exécutée par les e2e.
- `FAILED:` si une mutation M1 à M5 est verte à l'enfant et rouge au parent.
- `FAILED:` si la garde ne vaut pas `noyau-inclut,un-ou-faux,aiguillage-nom`, ou si la ligne réinjectée ne rougit pas `make test`.
- `FAILED:` si `class-coverage-net` n'est pas OK.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit, ou si les comptes d'arrivée ne sont pas `aiguillage-nom` 0, `défauts` 694, pied 1291 / 89, sans explication.
- `FAILED:` en cas de tassement (voir « Anti-tassement »).
- `FAILED:` si le message du commit omet : les 30 sites `fichier:ligne` du parent classés, `L`, les mutations avec leur résultat au parent, la garde mordante, `class-coverage-net`, la correction `fold_pk` et sa preuve `cmp`, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.5 », ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Les autres appels de `ho` du cart (`lua.lua:1389`, `:1442`, tables `df` et `nb`) : ils sont déjà portés sans aiguillage par nom (D.8) et le compteur ne les voit pas.
- `fk` de `hp` : il reste, pour l'archétype de 2.12.
- Les membres `has_*` de `hp` et le genre `false` d'`il` : inchangés.
- Élargir le motif `aiguillage-nom` du compteur aux déclarations de membres : `tools/composition_residue.sh` n'est pas touché ; la grep de Sortie 1 couvre le cas pour ce commit.
- `sanitize-net` ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Une fonction `hp_positions` plutôt que l'affectation dans `pico8_hp_struct_new`.** Le constructeur fait déjà 90 lignes ; la fonction n'a ni table ni nom de membre en chaîne, et l'anti-tassement l'interdit explicitement. Son nom n'est pas `hp_ho` pour que la garde puisse interdire le retour de l'ancien.
- **G2. L'`assert` d'`il` (« `hp: \`il\` gets a number -- stop, do not add one` », `research.c:68`) part.** Il testait `!is_fixed` pour la position 7, qui ne pouvait jamais être vraie : `il` n'est pas dans `gp` et n'est pas la position de `fj`. Sans table, `il` ne peut plus recevoir de nombre par construction. Aucun TU ne le visait. Le plan dit « les `assert` de genre restent, messages compris » : les trois autres messages restent mot pour mot ; celui-ci n'a plus de condition à tester.
- **G3. L'`assert` de nombre de positions** perd sa moitié `self->mr->count` et reçoit un message (`"hp: fd has eight positions, px to dm"`), que l'aiguille du TU vise ; `must_abort_saying` en avait besoin pour ne pas dépendre d'un nom disparu.
- **G4. `DEFAULTS_HASH` tenu par des constantes du TU** (`CART_MR`, `CART_GP`), selon le principe de 2.4 (« chaque TU connaît le genre constant de sa classe »). Le changer est interdit par les règles de la phase ; ne plus plier les noms le changerait.
- **G5. L'include `pico8_collect.h` de `research.c` part** : devenu mort, et le laisser serait une arête gratuite pour la phase des inclusions.
- **G6. Le glossaire est mis à jour** (deux entrées nommaient des fonctions qui disparaissent), bien que le plan ne le liste pas.
- **G7. `fold_pk` corrigé dans ce commit** plutôt qu'à part : une ligne de TU, objet identique, aucun compte ne bouge.

### Surprises de mesure, pour `plan-pilot`

1. **Pied −29, fichiers −2**, exactement comme le plan : `test_pico8_hp.c:962` reste en `défauts` ; `collect.c` et `research.h` quittent le pied.
2. **Le motif `aiguillage-nom` ne voit pas une déclaration de membre** (`const pico8_str_seq_t *mr;`) : la garde ne mord que sur les accès, les noms de table et les fonctions. Grep de Sortie 1 en complément.
3. **Un commentaire peut rougir la garde** : écrire `hp.mr` dans un commentaire de TU est compté. Écrire `` `mr` `` sans point.
4. **M4 est un mutant équivalent** : les entrées `il_false` de `fd` ont toutes `il` NULL ; le test d'`il_false` ne change rien aux données actuelles. Vrai aussi au parent.
5. **`build_with_nine_positions`** dépendait du texte `self->mr->count` de l'assert : son aiguille change.

### Questions pour l'humain

Aucune.
