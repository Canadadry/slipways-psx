---
title: "Plan 3, étape 2.12b : les archétypes de la famille b (`mz`, `jc`), le sous-archétype sonde `pico8_probe_def_t`, et 25 comparaisons de plus au test ponctuel (total 76 ; `défauts` 669, pied inchangé) — un commit, jeu identique"
description: "Le type du sous-archétype sonde naît dans `game_data.h`, enchâssé dans les deux archétypes neufs de `mz` et `jc`. Personne ne les lit encore. Le test ponctuel gagne `test_mz` (13 comparaisons) et `test_jc` (12), et son total asserté passe de 51 à 76. Le `def` du composant sonde et le sous-archétype nul de la sonde locale restent à 2.13b."
status: needs-triage
---

## Problem Statement

**Écrit sur `81a29b2`** (branche `claude/plan-pilot-3-7zdft0`), pendant que 2.11r s'implémente dans l'arbre de travail et que 2.12a attend en triage. Aucune ligne de code du dépôt n'a changé pour écrire ce PRD. Tous les essais ont été faits sur une seule copie jetable de `81a29b2` (scratchpad `prd212b/`, supprimée depuis), où le texte de « Ce qui change » du PRD 2.12a a d'abord été appliqué et commité (`2.12a` de la copie, vert, 26 e2e conformes, 51 comparaisons), puis celui de 2.12b.

**2.12b part de l'arbre d'après 2.12a**, lui-même après 2.11r-c. Ce qui dépend de ce qui n'était pas là :

- **2.11r-b3 et 2.11r-c** n'étaient pas commis. Ils touchent `wormhole.c`, `slipway.c`, `probe.c` (constructeurs retypés, `cg`), jamais `game_data.[ch]` ni le TU. 2.12b ne touche que ces trois fichiers-là (plus `docs/plan-3.md`) : aucun recouvrement de texte. Les numéros de ligne de `wormhole.c` et `slipway.c` cités par la preuve `const` (§ Preuves, 6) bougeront : l'implémenteur les lit par fonction.
- **Compteur.** Sur la copie (2.11r-b2 + 2.12a), le pied est `1079 / 87` (`registres 6 5`) au parent comme à l'enfant, `--detail` identique hors `info game_data.h 634 → 666`. Après 2.11r-c, le PRD 2.12a attend `1073 / 85` ; 2.12b ne change aucun compte, son arrivée est donc le pied de départ.
- **2.12a.** Toutes les ancres de ce PRD sont du texte que 2.12a crée (`} pico8_pj_def_t;`, `extern const pico8_pj_def_t pico8_pj_def;`, la valeur `pico8_pj_def`, `test_hy();`, l'assert du total 51). Si 2.12a a été implémenté avec un repli (2.12a1/a2) ou un autre ordre, l'ancre reste la dernière ligne du bloc de 2.12a.

Vu du mainteneur du port : 2.12a a créé l'archétype des douze racines simples et le test ponctuel. 2.12b fait de même pour les deux racines qui portent une sonde, le trou de ver et la voie, et crée pour cela le **sous-archétype sonde**, que 2.12c (sonde `g_`/`fv`) et 2.12d/e (planètes) réemploieront tel quel.

### Texte du plan, recopié (`docs/plan-3.md` à `81a29b2`)

> - 2.12 — archétypes et **test ponctuel de la règle 3**, selon l'étude 2.9 (`docs/etude-2-9-archetypes.md`, §2 à §4). Chaque archétype naît dans `pico8_game_data.h` (type) et `pico8_game_data.c` (valeur résolue, sans nommer aucun `_start`), sauf celui de `hy`, dans `economy.h/.c`. Le test ponctuel est un TU neuf, `tests/test_pico8_archetype_chain.c`. Il compare champ par champ les champs A et I d'un `pico8_X_new(NULL)` frais à `pico8_X_def`, groupes de Q7 compris, avec les trois exceptions de l'étude, §3. Aucune lecture ne bascule encore, et aucun `def` n'entre dans les instances. Compte : `défauts` **669** à chaque sous-étape. […]
>   - 2.12b — `mz`, `jc` et `pico8_probe_def_t` : 25 comparaisons, groupe sonde de Q7.

Et, note « Découvert à 2.10 » : « sonde et émetteur (réserve du vérificateur de 2.9) : leurs champs A se lisent par un `def` dans le composant, posé par l'init du porteur à `&def->probe`/`&def->emitter` ; ce `def` naît à 2.12b/2.13b (sonde) […], avec un sous-archétype nul pour la sonde locale de `research_icon.c:180`. »

Règles communes de la phase : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée.

### Réserve du vérificateur de 2.10 : où naissent le `def` du composant et `pico8_probe_def_none`

La réserve (`9ca66f4`) demande que les PRD 2.12b/c et 2.13b/c traitent « `def` dans le composant sonde/émetteur et sous-archétype nul `pico8_probe_def_none` ». Le plan place à 2.12b le **type** `pico8_probe_def_t`, rien de plus. Tranché ici, d'après l'étude 2.10 §2 (« ce `def` naît avec le premier composant converti : **2.12b** (type) et **2.13b** (`mz`, `jc`) » ; réserve 4 : « Posé par 2.13b avec le `def` du composant ») :

| Élément | Sous-étape | Pourquoi |
|---|---|---|
| type `pico8_probe_def_t`, enchâssé en membre `probe` de `pico8_mz_def_t` et `pico8_jc_def_t` | **2.12b** (ce PRD) | le test ponctuel compare le groupe sonde de Q7 de `mz` et `jc` : il lui faut un archétype à comparer |
| membre `const pico8_probe_def_t *def` dans `pico8_probe_t`, posé à `&pico8_X_def.probe` par l'init du porteur | **2.13b** | c'est un membre d'instance (le composant vit dans `probe_own`) posé par une init. Le plan interdit l'un et l'autre en 2.12 (« aucun `def` n'entre dans les instances » ; aucun `_new` ni init ne change) |
| `pico8_probe_def_none` (sous-archétype nul) et `pb.def = &pico8_probe_def_none` dans `pico8_ie_kj` (`research_icon.c:180`) | **2.13b** | sa seule raison d'être est d'être la cible de `pb.def`, qui n'existe pas avant 2.13b. À 2.12b, ce serait une constante sans lecteur ni comparaison possible : la sonde locale `pb` n'est atteignable par aucun TU, et `pico8_dl_show` ne lit que `et`, `has_od`, `od` (`probe.c:245-256`), aucun champ A |

**Contrat laissé à 2.13b** (à recopier dans son PRD) : `probe.h` inclut déjà `game_data.h`, donc le membre `def` n'ajoute aucune inclusion ; `pico8_probe_def_none` va dans `game_data.h/.c` à côté des autres (étude, « à côté des autres ») ; entre 2.13b et 2.13e2, les autres porteurs de sonde (`g__struct_init`, `qh_struct_init`, sortes de `nr`) font `malloc` sans mise à zéro : 2.13b doit poser leur `def` (à `&pico8_probe_def_none` ou NULL, jamais indéterminé) tant que leur famille n'a pas d'archétype, et aucun lecteur générique (`probe.c:168`, `:333-337`) ne bascule sur `probe->def` avant que tous les porteurs l'aient (2.13e2, étude 2.10, réserve 2).

### Question ouverte chez l'humain (Q-2.10, liens du noyau)

**2.12b n'en dépend pas.** Elle n'ajoute aucun lien au noyau, et le `def` du composant (2.13b) est dans `pico8_probe_t`, pas dans le noyau. Le lien `probe` du noyau est 2.15. Une réponse « non » change 2.14/2.15, pas le sous-archétype ni les archétypes de 2.12b.

### État de départ attendu (après 2.11r-c et 2.12a)

| Commande | Résultat attendu |
|---|---|
| `make composition-residue` | `défauts 669 52`, `inclusion-mutuelle 17 16`, pas de ligne `registres` ; pied **celui d'arrivée de 2.12a** (1073 / 85 attendu) |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 282 ; 0 écriture : 15 ; 0 hors constructeur : 27` |
| `sh tools/composition_residue.sh --check inclusion-mutuelle \| tail -1` | `résidu gardé (inclusion-mutuelle) : 43 ligne(s)` |
| `wc -l src/pico8_game_data.h` | 634 |
| `grep -c 'g_compared == 51' tests/test_pico8_archetype_chain.c` | 1 |
| `grep -rn 'probe_def\|pico8_mz_def\|pico8_jc_def' src tests` | vide |
| `make test` ; `make e2e` | vert ; 26 conformes |

Un départ autre : arrêt, rapporté.

## Solution

Un seul commit :

- **Trois types** dans `pico8_game_data.h` : `pico8_probe_def_t` (les sept champs A et I de la sonde, dans l'ordre de `pico8_probe_t`, sans `et`), `pico8_mz_def_t`, `pico8_jc_def_t` (chacun `me`/`t` en tête, puis ses champs A et I dans l'ordre de l'instance, puis `probe`).
- **Deux valeurs** `const` non `static` dans `pico8_game_data.c`, `pico8_mz_def` et `pico8_jc_def`, déclarées `extern`. Résolues, en littéraux, sans nommer aucun `_start`.
- Le commentaire de tête des archétypes (2.12a) nomme aussi `mz.jj` parmi les défauts laissés à l'init.
- **TU** `tests/test_pico8_archetype_chain.c` : `test_mz` (13), `test_jc` (12), une aide `same_text_or_nil`, deux inclusions, total asserté **76**.
- `docs/plan-3.md` : note « Découvert à 2.12b ».

Rien d'autre. Aucun `def` dans une instance ni dans `pico8_probe_t`. Aucun `_new`, init ou `_start` ne change. Aucune lecture ne bascule. Pas de `Makefile` (le TU existe depuis 2.12a).

## User Stories

1. En tant qu'auteur de 2.13b, je veux les archétypes de `mz` et `jc` prouvés égaux à la chaîne, sonde comprise, afin que leur init lise l'archétype sans changer une valeur.
2. En tant qu'auteur de 2.12c à 2.12e2, je veux un type `pico8_probe_def_t` déjà là, dans l'ordre de `pico8_probe_t`, afin de l'enchâsser dans les archétypes sonde et planète sans le redéfinir.
3. En tant que gardien de la règle 3, je veux que le groupe sonde de Q7 (`et`, `dw`, `od`, `dq`, `fp`, `ik`) de `mz` et `jc` passe le test ponctuel, `et` à part (E, vérifié NULL hors compte), afin que le plan soit tenu pour ces deux classes.
4. En tant que gardien du test ponctuel, je veux le total asserté à 76, afin qu'une comparaison oubliée soit rouge.
5. En tant que gardien du compteur, je veux `défauts` à 669, `inclusion-mutuelle` à 17 / 16 et le pied inchangé, et une explication chiffrée de ce qui bouge dans `--champs`.
6. En tant que gardien de la règle 1, je veux la preuve `const` des 14 membres d'archétype de 2.12b, rejouée sur la copie du commit.
7. En tant que gardien du jeu, je veux 26 e2e conformes, et aucun objet de `src/` changé hors `game_data`, afin d'être sûr que personne ne lit l'archétype.
8. En tant que vérificateur, je veux des mutations d'archétype et des jumelles de chaîne rejouées au parent.
9. En tant qu'auteur de 2.13b, je veux lire dans ce PRD où naissent le `def` du composant et `pico8_probe_def_none`, et ce qu'ils exigent.

## Implementation Decisions

### Modules

`game_data` ; TU `archetype_chain` ; `docs/plan-3.md`. Aucun autre.

### Ce qui change, fichier par fichier (ancres de texte ; numéros de la copie `81a29b2` + 2.12a)

**`src/pico8_game_data.h`**

1. Dans le commentaire de tête des archétypes (2.12a, `:265-267`), remplacer ` * (ui.le, nf.ra) is not here: the module's init still sets it. */` par :

   ```c
    * (ui.le, nf.ra, mz.jj) is not here: the module's init still sets it. */
   ```

2. Après `} pico8_pj_def_t;` (`:346`), avant la ligne vide qui précède « The jo tree… », insérer :

   ```c

   /* A probe's own defaults, in the archetype of each class that carries one. */
   typedef struct {
       pico8_fixed_t dw;
       pico8_fixed_t od;
       int has_od;
       const char *dq;
       pico8_fixed_t fp;
       pico8_fixed_t ik;
       int has_ik;
   } pico8_probe_def_t;

   typedef struct {
       const char *me;
       pico8_fixed_t t;
       pico8_fixed_t spawn_snd;
       pico8_fixed_t oi;
       pico8_fixed_t fa;
       const pico8_int_seq_t *kk;
       pico8_probe_def_t probe;
   } pico8_mz_def_t;

   typedef struct {
       const char *me;
       pico8_fixed_t t;
       pico8_fixed_t fa;
       const pico8_int_seq_t *k_;
       pico8_fixed_t nh;
       pico8_probe_def_t probe;
   } pico8_jc_def_t;
   ```

3. Après `extern const pico8_pj_def_t pico8_pj_def;` (`:604`) :

   ```c
   extern const pico8_mz_def_t pico8_mz_def;
   extern const pico8_jc_def_t pico8_jc_def;
   ```

`game_data.h` : 634 → **666** lignes.

**`src/pico8_game_data.c`** — après la valeur `pico8_pj_def` (`:507-513`, qui finit par `.i_ = &pico8_pj_i__row,` / `};`), avant `static const char jo_me_text[]` :

```c

const pico8_mz_def_t pico8_mz_def = {
    .me = "kz",
    .t = 0x0,
    .spawn_snd = 0x340000,
    .oi = 0x0,
    .fa = 0x50000,
    .kk = &pico8_mz_kk_seq,
    .probe = {.dw = 0x0, .od = 0x30000, .has_od = 1, .dq = "pr", .fp = 0x10000, .ik = 0x310000, .has_ik = 1},
};

const pico8_jc_def_t pico8_jc_def = {
    .me = "kz",
    .t = 0x0,
    .fa = 0x20000,
    .k_ = &pico8_jc_k__seq,
    .nh = 0x140000,
    .probe = {.dw = 0x106, .od = 0x0, .has_od = 0, .dq = NULL, .fp = 0x0, .ik = 0x300000, .has_ik = 1},
};
```

Chaque valeur est celle de l'annexe B de l'étude 2.9 (`mz` 14 champs dont `jj` S hors archétype ; `jc` 12), recopiée de `pico8_ke_start` (`t`), de `pico8_mz_start`/`pico8_jc_start` et des littéraux de `pico8_mz_struct_new` (`has_ik = 1`, `has_od = 1`) et `pico8_jc_struct_new` (`dq = NULL`, `has_ik = 1`, `od = 0`, `has_od = 0`). `jc.fp` est le faux de la spécification (`int` 0 dans `pico8_jc_start_t`), rangé `0x0` dans le membre `pico8_fixed_t`. `me` vaut `"kz"` en littéral pour les deux (masque le `"ph"` de `ke`), comme G2 de 2.12a.

**`tests/test_pico8_archetype_chain.c`**

1. Inclusions, dans l'ordre alphabétique : `#include "../src/pico8_slipway.h"` après `pico8_scroll.h`, `#include "../src/pico8_wormhole.h"` après `pico8_ui_button.h`.
2. Après `same_text` :

   ```c

   static int same_text_or_nil(const char *got, const char *want) {
       return (got == NULL && want == NULL) || same_text(got, want);
   }
   ```

3. Avant `int main(void)`, après `test_hy` :

   ```c

   /* `jj` is the module's shared wrapper, not the archetype's: two fresh
    * wormholes point at the same one. */
   static void test_mz(void) {
       const pico8_mz_t *mz = pico8_mz_new();
       const pico8_mz_t *other = pico8_mz_new();

       same(same_text(mz->header.me, pico8_mz_def.me), "mz.me");
       same(mz->header.t == pico8_mz_def.t, "mz.t");
       same(mz->spawn_snd == pico8_mz_def.spawn_snd, "mz.spawn_snd");
       same(mz->oi == pico8_mz_def.oi, "mz.oi");
       same(mz->fa == pico8_mz_def.fa, "mz.fa");
       same(mz->kk == pico8_mz_def.kk, "mz.kk");
       same(mz->probe->dw == pico8_mz_def.probe.dw, "mz.probe.dw");
       same(mz->probe->od == pico8_mz_def.probe.od, "mz.probe.od");
       same(mz->probe->has_od == pico8_mz_def.probe.has_od, "mz.probe.has_od");
       same(same_text_or_nil(mz->probe->dq, pico8_mz_def.probe.dq), "mz.probe.dq");
       same(mz->probe->fp == pico8_mz_def.probe.fp, "mz.probe.fp");
       same(mz->probe->ik == pico8_mz_def.probe.ik, "mz.probe.ik");
       same(mz->probe->has_ik == pico8_mz_def.probe.has_ik, "mz.probe.has_ik");
       assert(mz->jj != NULL && mz->jj == other->jj);
       assert(mz->probe->et == NULL);
   }

   static void test_jc(void) {
       const pico8_jc_t *jc = pico8_jc_new(NULL);

       same(same_text(jc->header.me, pico8_jc_def.me), "jc.me");
       same(jc->header.t == pico8_jc_def.t, "jc.t");
       same(jc->fa == pico8_jc_def.fa, "jc.fa");
       same(jc->k_ == pico8_jc_def.k_, "jc.k_");
       same(jc->nh == pico8_jc_def.nh, "jc.nh");
       same(jc->probe->dw == pico8_jc_def.probe.dw, "jc.probe.dw");
       same(jc->probe->od == pico8_jc_def.probe.od, "jc.probe.od");
       same(jc->probe->has_od == pico8_jc_def.probe.has_od, "jc.probe.has_od");
       same(same_text_or_nil(jc->probe->dq, pico8_jc_def.probe.dq), "jc.probe.dq");
       same(jc->probe->fp == pico8_jc_def.probe.fp, "jc.probe.fp");
       same(jc->probe->ik == pico8_jc_def.probe.ik, "jc.probe.ik");
       same(jc->probe->has_ik == pico8_jc_def.probe.has_ik, "jc.probe.has_ik");
       assert(jc->probe->et == NULL);
   }
   ```

4. Dans `main`, après `test_hy();`, et l'assert du total remplacé :

   ```c
       test_mz();
       test_jc();
       assert(g_compared == 76 && "families a and b compare 76 fields: one missed or one too many");
   ```

Lecture du TU :
- La sonde se lit par le membre `probe` de l'instance (K, pointe `probe_own`), comme tous les lecteurs d'aujourd'hui ; aucun cast, aucun `probe_of`.
- `dq` peut être NULL (`jc`) : `same_text_or_nil` compare NULL à NULL, sinon par contenu. `same_text` de 2.12a reste strict (un `me` NULL est un défaut).
- Hors compte, en `assert` nus : `mz.jj` partagé et non NULL (exception 1 de l'étude, §3, comme `nf.ra`) ; `et` NULL pour les deux (E, groupe Q7 : exception 3).
- Pas de `test_probe` séparé : le sous-archétype n'a pas de chaîne propre, il est comparé à travers chaque porteur.

**`docs/plan-3.md`** — note « Découvert à 2.12b » au bout de la ligne 2.12b : comptes (`défauts` 669, pied inchangé, `--champs` 27 → 23 et pourquoi), `game_data.h` 666 lignes, total du TU 76, et le « pour 2.13b » (le `def` du composant et `pico8_probe_def_none` naissent là, avec la pose pour les autres porteurs). Le reste va au message de commit.

Rien d'autre : ni `glossaire-noms.md` (aucun nom minifié neuf), ni `trous-de-portage.md`, ni `Makefile`, ni `tools/`, ni `tests/e2e/`, ni `*.sha256`.

### Comptes, mesurés sur la copie (`81a29b2` + 2.12a, puis 2.12b)

| Mesure | parent (2.12a) | 2.12b |
|---|---|---|
| `make composition-residue` (copie, 2.11r-b3/c absents) | pied 1079 / 87, `registres 6 5` | **1079 / 87**, `--detail` identique hors `info src/pico8_game_data.h 634 → 666 lignes` |
| attendu après 2.11r-c | pied 1073 / 85 | **1073 / 85** |
| `défauts` (`--check`) | 669 / 52 | **669 / 52** |
| `inclusion-mutuelle` | 17 / 16, 43 lignes | **17 / 16, 43 lignes** |
| `--champs` | 282 ; 15 ; 27 | **282 ; 15 ; 23** (voir ci-dessous) |
| comparaisons du test ponctuel | 51 | **76** (+25 : `mz` 13 = A 8 + I 5, `jc` 12 = A 6 + I 6, étude §3) |

**`--champs` 27 → 23, attendu.** Le relevé (`composition_residue.sh`, fonction `champs`) compte par **nom** de membre toute écriture textuelle `.X =` ou `->X =` d'un `.c` de `src/`, et une ligne hors fonction (un initialiseur de fichier) compte « hors constructeur » (le script le dit : c'est un majorant). La ligne `.probe = {.dw = …}` des deux valeurs donne au nom `probe` 2 écritures hors constructeur ; or `probe` est membre de quatre types de `--champs` (`pico8_g__t`, `pico8_jc_t`, `pico8_mz_t`, `pico8_qh_t`), qui quittent tous les quatre le groupe « 0 hors constructeur ». Les autres comptes de la ligne bougent sans changer de groupe (`fa` 28 → 30 sur ses porteurs, `kk`, `oi`, `spawn_snd`, `k_`, `nh` +1, `od`/`has_od`/`me`/`t` +2). Aucun membre n'est ajouté (282) ; aucun membre ne quitte « 0 écriture » (15). Ce n'est pas une écriture d'instance : c'est la donnée de l'archétype. 2.12c à 2.12e2 verront le même effet (`probe`, `emitter`).

### Anti-tassement

- **Interdit** :
  - nommer un `pico8_X_start` dans une valeur d'archétype : `défauts` passerait à 670 ;
  - mettre `mz.jj` (ou `g_mz_jj_wrapper`) dans `pico8_mz_def_t` : ne compile pas (`'g_mz_jj_wrapper' undeclared`), et nommer son type depuis `game_data.h` ajoute une paire (étude 2.9, §2) ;
  - ajouter un membre `def` à `pico8_probe_t`, à `pico8_mz_t` ou à `pico8_jc_t`, créer `pico8_probe_def_none`, ou toucher `research_icon.c` : c'est 2.13b ;
  - faire lire l'archétype par un `_new`, une init ou un lecteur ;
  - mettre `et` dans `pico8_probe_def_t` (E) ;
  - un cast, un `void *`, un `probe_of`, une comparaison de classe dans le TU ou dans les archétypes ;
  - comparer à un `pico8_X_start` au lieu de `pico8_X_def` ;
  - toucher un `*.sha256`, `tests/e2e/`, une constante d'empreinte, `tools/composition_residue.sh` ou son TU (même pour rendre à `--champs` son 27).

## Implementation Chunks

Un commit, **≈ 30 min** preuves comprises (essai : 10 min d'écriture, ≈ 20 min de preuves, les jumelles au parent étant le plus long). Budget **50 min**.

1. Coller les trois blocs de `game_data.h`, le bloc de `game_data.c`, les quatre retouches du TU.
2. `make test`, `make e2e`, compteur, `--champs`.
3. Preuves (ci-dessous), note du plan, commit, push.

**Repli écrit** (au-delà de 1,5 fois le budget avant le commit) : deux commits verts, chacun avec son total asserté :
- **2.12b1** : `pico8_probe_def_t`, `pico8_mz_def_t`, `pico8_mz_def`, `same_text_or_nil`, `test_mz` (total **64**) ;
- **2.12b2** : `pico8_jc_def_t`, `pico8_jc_def`, `test_jc` (total **76**).
Les comptes ne changent à aucun des deux, sauf `--champs` : 27 → 23 dès 2.12b1 (le nom `probe` est déjà écrit hors fonction). Le repli est écrit dans la note « Découvert à ».

## Testing Decisions

- **Pas de TU neuf** : le test ponctuel de 2.12a gagne deux fonctions. Il ne rejoue jamais un `events.json` ; les e2e partent de l'écran titre.
- **Bon test ici** : la valeur que la chaîne laisse dans une instance fraîche, comparée au seul objet neuf, sans relire le `_start`.
- **Prior art** : PRD 2.12a (même TU, mêmes preuves) ; mutations avec la règle du parent de D.21 et 2.4 ; preuve `const` de l'étude 2.9, annexe A.

### Preuves

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. Mesuré : `make test` code 0 (`test_pico8_archetype_chain: all tests passed`), `make e2e` 26 conformes.

2. **Test ponctuel** : vert au commit = 76 égalités, `g_compared == 76` = aucune oubliée.

3. **Mutations**, une à la fois, `make -k -j8 unit` puis `make -k e2e`, restauration par `cp` puis `touch`. **Règle du parent** : une mutation verte au commit et rouge au parent est un **ÉCHEC** ; verte aux deux, « chemin jamais atteint ». Mesuré sur la copie :

   *Côté archétype (au parent, ni `pico8_mz_def` ni `pico8_jc_def` n'existent : pas de jumelle, doit être rouge au commit).*

   | # | Mutation | TU rouges | e2e |
   |---|---|---|---|
   | M1 | étude §3, défaut propre : `pico8_mz_def.probe.ik` `0x310000` → `0x320000` | `archetype_chain` seul (`mz.probe.ik`) | 26 / 26 |
   | M2 | étude §3, défaut hérité masqué rendu à l'ascendante : `pico8_jc_def.me` `"kz"` → `"ph"` (`ke`) | `archetype_chain` seul (`jc.me`) | 26 / 26 |
   | M3 | texte nil de la sonde : `pico8_jc_def.probe.dq` `NULL` → `"pr"` | `archetype_chain` seul (`jc.probe.dq`) | 26 / 26 |
   | M4 | drapeau de la sonde : `pico8_mz_def.probe.has_od` `1` → `0` | `archetype_chain` seul (`mz.probe.has_od`) | 26 / 26 |
   | M5 | champ A sonde de `jc` : `pico8_jc_def.probe.dw` `0x106` → `0x107` | `archetype_chain` seul (`jc.probe.dw`) | 26 / 26 |
   | M6 | comparaison oubliée : `same(jc->nh == …)` retirée du TU | `archetype_chain` seul (`g_compared == 76`) | 26 / 26 |
   | M7 | séquence par adresse : `pico8_mz_def.kk` → `&pico8_jc_k__seq` | `archetype_chain` seul (`mz.kk`) | 26 / 26 |

   « `archetype_chain` seul » et 26 e2e : **personne ne lit l'archétype**.

   *Côté chaîne, jumelles identiques au parent (2.12a) et au commit.*

   | # | Mutation | TU rouges au commit | TU rouges au parent | e2e (commit = parent) |
   |---|---|---|---|---|
   | C1 | `pico8_mz_start.ik` `0x310000` → `0x320000` | `archetype_chain`, `mz` | `mz` | 26 / 26 |
   | C2 | `pico8_jc_start.dw` `0x106` → `0x107` | `archetype_chain`, `jc` | `jc` | 26 / 26 |
   | C3 | `pico8_mz_struct_new` : `self->probe->has_od = 1;` → `0` (littéral de chaîne, pas de `_start`) | `archetype_chain`, `mz` | `mz` | 9 / 26 conformes |
   | C4 | `pico8_jc_struct_new` : `self->probe->dq = NULL;` → `"pr"` | `archetype_chain`, `jc` | `jc` | 26 / 26 |

   Aucun ÉCHEC : au commit, chaque jumelle rougit ce que le parent rougit, plus le test ponctuel. C1, C2, C4 : aucun e2e ne voit la mutation, le TU de classe et le test ponctuel si.

4. **Couverture** : `gcov` du TU (`gcc --coverage` avec les 60 `ENGINE_SRCS`, dans la copie) : **166 / 167** lignes (99,40 %), la seule non exécutée étant le `fprintf` d'échec de `same`. Les deux branches de `same_text_or_nil` sont prises (`jc` NULL, `mz` `"pr"`). `make line-coverage` des e2e n'est pas exigé (données seulement, preuve 5).

5. **Objets** (sans `-g`, `gcc -std=c99 -Wall -Wextra -Werror -Isrc -c`, parent puis commit, 155 `.c` compilables sans `raylib.h`, `-DPICO8_CLASS_COVERAGE` pour les deux qui l'exigent) :
   - **153 identiques** octet à octet, dont tous ceux qui incluent `game_data.h` ;
   - **2 diffèrent** : l'objet du TU (attendu) et `pico8_game_data.o`, dont `nm` ne gagne que `pico8_mz_def` et `pico8_jc_def` (les données suivantes glissent de 0xc0), et dont `objdump -d` ne diffère que par deux immédiats `__LINE__` (+19). Sous `-DNDEBUG`, son `.text` est **identique** (405 octets).

6. **Règle 1, preuve `const` rejouée** (copie du commit ; chaque membre rendu `const` dans sa struct, un à la fois, puis `gcc -fsyntax-only` des 155 `.c` ; erreurs neuves listées). Mesuré : **14 membres, 52 sites**, tous ceux de l'annexe A de l'étude 2.9 (décalés de −1 dans `wormhole.c`, de +14 pour `T:jc.c`) :
   - `mz` : `spawn_snd` 1, `fa` 1, `kk` 1 (`mz_struct_new` seul, A) ; `oi` 2 (`mz_struct_new`, `mz_kz` : I) ;
   - `jc` : `k_` 1 (`jc_struct_new`, A) ; `fa` 2 (`+ jc_hq`, I) ; `nh` 3 (`+ jc_ey`, `slipway_display.c` `jc_kj`, I) ;
   - sonde : `dw` 6 (cinq inits + `test_pico8_jc.c` `test_bv`, que 2.13 réécrit sur un archétype de test) ; `dq` 4, `fp` 5, `ik` 4, `has_ik` 4 (inits seules, A) ; `od` 12, `has_od` 6 (inits, `jc_hq`, et la sonde locale de `pico8_ie_kj`, I).
   Tout autre site est un arrêt « champ sans règle ». `me`/`t` de l'en-tête : relevé de 2.12a, inchangé (aucune ligne de code ne bouge). Piège : `pico8_probe_t` est `typedef struct pico8_probe {` ; un script qui cherche `typedef struct {` le manque.

7. **Inclusion, casts, aiguillage** : `inclusion-mutuelle` **17 / 16**, **43 lignes** ; greps de Sortie 4 et 5.

8. `class-coverage-net` : **non**. `sanitize-net` : **non**.

### Greps de Sortie (sur l'arbre du commit)

1. `grep -c 'const pico8_[a-z_]*_def_t pico8_[a-z_]*_def = {' src/pico8_game_data.c` → **13** (11 de 2.12a + 2).
2. `grep -c 'pico8_probe_def_t' src/pico8_game_data.h` → **3** (le type, deux membres) ; `grep -rn 'probe_def' src tests | grep -v pico8_game_data.h` → vide ; `grep -rn 'probe_def_none' src tests` → vide.
3. `git diff <parent> -- src | grep -E '^\+.*_start'` → vide.
4. `grep -rnE '\bdef\b *;|->def\b|\.def\b' src` → vide.
5. `git diff <parent> | grep -E '^\+.*\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)'` → vide ; `git diff <parent> | grep -E '^\+.*(->cls|header\.cls|_struct_class\(\)|probe_of)'` → vide.
6. `grep -nE 'g_mz_jj_wrapper|g_shared_le|g_nf_ra|g_jo_kw_shared|g_ga_by_shared' src/pico8_game_data.[ch]` → vide.
7. `make composition-residue` → pied de 2.12a, `défauts 669 52`, `inclusion-mutuelle 17 16` ; `--champs` → `282 ; 15 ; 23` ; `wc -l src/pico8_game_data.h` → 666.
8. `git diff --stat <parent>` : `src/pico8_game_data.h`, `src/pico8_game_data.c`, `tests/test_pico8_archetype_chain.c`, `docs/plan-3.md`, et rien d'autre.

### Tableau « Runs »

Celui de ce PRD, sur la copie jetable de `81a29b2` (`p` = 2.12a appliquée et commitée dans la copie, `b` = 2.12b).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout 81a29b2` ; texte de 2.12a appliqué depuis son PRD (script, 6 blocs) | — | 0 | `game_data.h` 634 lignes | 2 s |
| 2 | `timeout 900 make -j8 test` | `p` | 0 | `test_pico8_archetype_chain: all tests passed` | ≈ 60 s |
| 3 | `make e2e` ; compteur ; `--champs` ; `--check inclusion-mutuelle` ; commit de la copie | `p` | 0 | 26 conformes ; 1079 / 87 ; 282 / 15 / 27 ; 43 lignes | 5 s |
| 4 | texte de 2.12b appliqué ; `make -j8 test` | `b` | 2 | mon script a collé `same_text_or_nil` dans le corps de `same_text` (ancre) ; corrigé | 30 s |
| 5 | idem, réappliqué | `b` | 0 | `all tests passed` | 40 s |
| 6 | `make e2e` ; compteur ; `--champs` ; `--check inclusion-mutuelle` | `b` | 0 | 26 conformes ; 1079 / 87 ; **282 / 15 / 23** ; 43 lignes | 5 s |
| 7 | `--champs` complet, `p` puis `b`, `diff` ; lecture de `champs()` | `p` / `b` | 1 | `probe` 0 → 2 hors constructeur sur 4 types : initialiseurs `.probe = {` | 3 s |
| 8 | M1 à M7, `make -k -j8 unit` + `make -k e2e` | `b` | 2 | `archetype_chain` seul ; 26 conformes | ≈ 30 s × 7 |
| 9 | C1 à C4 au commit, puis au parent (`checkout` de `p`) | `b` / `p` | 2 | tableau des jumelles ; aucun ÉCHEC | ≈ 30 s × 8 |
| 10 | `constproof.py`, 14 membres | `b` | 0 | 52 sites, conformes à l'annexe A | ≈ 3 min |
| 11 | objets sans `-g`, `p` puis `b` ; `cmp`, `nm`, `objdump -d` ; `-DNDEBUG` `.text` de `game_data` | `p` / `b` | 0 | 153 identiques ; `game_data.o` : 2 symboles, `__LINE__` ; `.text` identique | 60 s |
| 12 | `gcc --coverage` du TU + `ENGINE_SRCS`, `gcov` | `b` | 0 | 166 / 167 lignes | 30 s |
| 13 | greps de Sortie ; `--detail` `p` / `b`, `diff` ; `--check défauts` | `b` | 0 | vides ; seule différence `info game_data.h 634 → 666` ; 669 | 3 s |
| 14 | `rm -rf` de la copie | — | 0 | copie supprimée | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas le commit 2.12a (après 2.11r-c), ou un descendant qui ne touche que `docs/`, ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) est rouge, si un `*.sha256` ou `tests/e2e/` change, ou si une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si le TU ne compte pas 76 comparaisons (25 de plus : `mz` 13, `jc` 12), ou si une comparaison lit un `pico8_X_start`.
- `FAILED:` si une mutation M1 à M7 est verte au commit, ou rougit un autre TU que `archetype_chain` ou un e2e.
- `FAILED:` si une jumelle C1 à C4 est verte au commit et rouge au parent.
- `FAILED:` si un objet autre que `game_data` et le TU diffère sans `-g`, ou si `game_data` diffère sous `-DNDEBUG` dans son `.text`.
- `FAILED:` si la preuve `const` trouve un site hors de l'annexe A (aux décalages près), ou une écriture hors construction d'un champ A.
- `FAILED:` si `défauts` n'est pas 669, `inclusion-mutuelle` 17 / 16 (43 lignes), le pied celui de 2.12a, ou `--champs` autre que 282 / 15 / 23 sans explication par la même lecture.
- `FAILED:` si un `def` entre dans une instance ou dans `pico8_probe_t`, si `pico8_probe_def_none` existe, si `mz.jj` est dans un archétype, ou en cas de tassement.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si le message du commit omet : les deux archétypes, le sous-archétype sonde, la répartition des 25, les mutations M et les jumelles C avec leur résultat au parent, la couverture, les objets, la preuve `const` (comptes), les comptes dont `--champs` 27 → 23 et sa raison, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.12b », ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Le membre `def` de `pico8_probe_t`, `pico8_probe_def_none`, la sonde locale de `research_icon.c`, toute lecture de l'archétype, la suppression de `pico8_mz_start`/`pico8_jc_start` : **2.13b** (voir le contrat plus haut).
- `pico8_emitter_def_t`, `g_`, `fv` et au-delà : 2.12c et suivantes.
- Un accesseur de `g_mz_jj_wrapper` : deux `mz` frais suffisent à montrer le partage.
- `tools/composition_residue.sh` (le compte `--champs` qui bouge par un initialiseur de données n'est pas un défaut du script : il se dit majorant) ; `class-coverage-net`, `sanitize-net` ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Le `def` du composant et `pico8_probe_def_none` vont à 2.13b**, pas à 2.12b (tableau de la réserve, plus haut) : le plan borne 2.12b au type, l'étude 2.10 place les deux à 2.13b, et l'objet nul n'a ni lecteur ni comparaison possible avant.
- **G2. Ordre des membres** : `pico8_probe_def_t` suit `pico8_probe_t` sans `et` ; `pico8_mz_def_t` et `pico8_jc_def_t` suivent l'ordre de leur instance, `probe` en dernier (comme `pico8_jo_def_t` dans l'étude). L'essai de l'étude 2.9 (troisième passe) écrivait `mz` avec `fa`, `kk`, `spawn_snd`, `oi` : ordre non normatif.
- **G3. `same_text_or_nil`** plutôt qu'élargir `same_text` : `same_text` doit rester rouge sur un `me` NULL (famille a).
- **G4. `et` vérifié NULL hors compte**, comme `ui.le`/`nf.ra` : E, groupe de Q7, exception 3 de l'étude.
- **G5. Quatre jumelles** au lieu de trois : C3 et C4 mutent un littéral de `_new`, pas un `_start`, et montrent que le test voit la chaîne entière, pas seulement les données.

### Surprises de mesure, pour `plan-pilot`

1. **`--champs` « 0 hors constructeur » 27 → 23** par le seul initialiseur `.probe = {…}` (relevé par nom de membre, initialiseurs de fichier comptés hors constructeur). 2.12c (`.probe`, `.emitter` de la sonde) et 2.12d/e (planète) le feront bouger de même : leurs PRD doivent l'attendre, pas le garder à une valeur.
2. **Au parent, chaque jumelle est déjà rouge dans le TU de classe** (`mz`, `jc`) : contrairement à `bg` en 2.12a, ces classes ont un TU qui compare à des littéraux. Le test ponctuel ajoute un filet au même endroit, pas un filet neuf.
3. **C3 rougit 17 e2e** (9 conformes sur 26, au parent comme au commit ; cause non analysée, hors sujet) : seule jumelle de 2.12b qui se voit en jeu.

### Questions pour l'humain

Aucune. 2.12b ne dépend pas de la question ouverte sur les pointeurs du noyau.
