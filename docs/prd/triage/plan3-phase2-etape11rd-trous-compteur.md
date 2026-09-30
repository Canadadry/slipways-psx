---
title: "Plan 3, étape 2.11r-d : les trous du compteur que le vérificateur de 2.11r a trouvés (`ptrs-entité` lit les membres d'un type d'entité comme `registres` lit les autres ; `registres` voit l'accolade à la ligne, l'union, `_Alignas` et `__attribute__`) et un TU qui observe l'ordre du `jp` de `bv` — un commit, comptes inchangés"
description: "Dans un type d'entité, `void **`, `void *[N]`, un alias de `pico8_ptrs_t` (et aussi `pico8_ptrs_t` en tableau, `**`, déclarateurs multiples, `*const`, pointeur de fonction qui rend `void *`) n'étaient comptés par aucune catégorie. `ptrs-entité` reprend la lecture de membre de `registres` ; les deux lecteurs retirent `__attribute__((…))`/`_Alignas(…)`, ouvrent un corps sur `union` et sur une accolade à la ligne suivante. Un TU de `jc` observe que `bv` vise le premier `jc` de `cg(\"jc\")`. Aucune forme n'existe dans `src/` : tous les comptes restent (1073 / 85). Essayé sur copie."
status: needs-triage
---

## Problem Statement

**Écrit sur `317b127`**, re-mesuré sur `f3d1c50` (2.12a commis entre-temps, plus le travail non commis de 2.12b dans `game_data.[ch]` et `test_pico8_archetype_chain.c`) : même résultat. Aucune ligne du dépôt n'a changé pour écrire ce PRD. Les essais ont été faits sur une seule copie jetable de `317b127` (scratchpad `prd211rd/`, supprimée depuis). Les numéros de ligne sont ceux de `317b127` ; ceux des trois fichiers touchés ne bougent pas à `f3d1c50`.

Le vérificateur de 2.11r (commit `317b127`) a ouvert trois trous :

1. **Trou du compteur.** Dans un type d'entité (`pico8_mz_t`, …), un membre `void **items;`, `void *slots[8];` ou `reg17_t l;` (après `typedef pico8_ptrs_t reg17_t;`) n'est compté par **aucune** catégorie : `registres` saute les types d'entité (`in_e(k)`, `composition_residue.sh:326`), et `ptrs-entité` ne lit qu'un motif ancré `^[ \t]+(const)?pico8_ptrs_t[ \t]*\*?[ \t]*nom;` sur la ligne brute (`:262`). Mesuré sur la copie (`forms.sh`, run 3), le trou est **plus large** que les trois formes signalées : dans un type d'entité, `pico8_ptrs_t arr[4];`, `pico8_ptrs_t **pp;`, `pico8_ptrs_t *a, *b;`, `const pico8_ptrs_t *const x;` et `void *(*get)(void);` rendent aussi 0 aux deux catégories. La Sortie de phase exige que tout conteneur d'entités non typé soit compté.
2. **Limites non écrites de `registres`** : struct dont l'accolade ouvre la ligne suivante, `union`, `_Alignas(8)` en tête, `struct __attribute__((…)) {`. Mesuré (run 3) : toutes à +0, sauf trois cas déjà vus par le jeton (`_Alignas(8) pico8_ptrs_t l;` comme membre, `extern _Alignas(8) pico8_ptrs_t g;` dans un `.h`, `union { pico8_ptrs_t l; } u;` membre sur une ligne).
3. **Trou de test.** Dans `pico8_bv` (`src/pico8_probe.c:179`), `first = filtered.items[filtered.count - 1]` au lieu de `items[0]` laisse `make test` vert et **26 e2e conformes** (mesuré, run 9) : l'ordre du `jp(cg.jc, …)[1]` de `bv` (`lua.lua:552-556`) n'est observé nulle part.

### Texte du plan à porter (après la ligne 2.11r-c de `docs/plan-3.md`, ligne 437 à `317b127`)

L'implémenteur insère ce texte tel quel, puis ajoute sa note « Découvert à 2.11r-d » en fin de ligne :

> - 2.11r-d — trous du vérificateur de 2.11r. `ptrs-entité` lit les membres d'un type d'entité avec la lecture de membre de `registres` (jeton `pico8_ptrs_t` ou alias, `void **`, `void *[N]`, pointeur de fonction qui rend `void *` ; exemption `type.membre` déclarateur par déclarateur) ; les deux lecteurs retirent `__attribute__((…))` et `_Alignas(…)`, ouvrent un corps sur `union` et sur une accolade à la ligne suivante, et `registres` commence une déclaration de fichier sur `_Alignas`. Un TU de `jc` observe que `bv` vise le premier `jc` de `cg("jc")`. Preuves : fixture rouge au script du parent, verte au neuf ; onze mutations du script, chacune rouge ; `ptrs-entité` mordante dans la garde ; mutation `items[count - 1]` de `bv` rouge au TU neuf. Compte : aucun ne change (pied **1073 / 85**), aucune forme n'existe dans `src/`. `class-coverage-net` : non. `sanitize-net` : non. Budget ≈ 50 min.

### État de départ attendu

| Commande | Résultat sur `317b127` (et sur `f3d1c50` + travail non commis de 2.12b) |
|---|---|
| `make composition-residue` | `parente 10 3`, `lien-parent 111 27`, `accesseurs-q6 237 33`, `défauts 669 52`, `void-ptr 8 4`, `inclusion-mutuelle 17 16` ; **`résidu 2 : 1073 ligne(s), 85 fichier(s)`** |
| `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` | `870:…:= noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres` (numéro possiblement décalé par 2.12, texte identique) |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 282 ; 0 écriture : 15 ; 0 hors constructeur : 27` à `317b127` ; **`… : 23`** à `f3d1c50` + 2.12b non commis (dû à 2.12, identique avec l'ancien et le nouveau script) |
| `make test` ; `make e2e` | vert ; 26 conformes |

Un départ autre : comparer l'ancien et le nouveau script **sur le même arbre** (point 3 des Preuves) ; seule leur égalité compte, pas les chiffres absolus.

## Solution

Un commit, trois fichiers :

- `tools/composition_residue.sh` (annexe A, diff exact contre `317b127`) :
  1. `ptrs-entité` ne lit plus le motif ancré : pour un type d'entité (pas un objet struct), il émet les lignes de membre que `read_structs` collecte déjà pour `registres` (`nr[k]`), avec l'exemption `PTRS_NON_ENTITE` appliquée déclarateur par déclarateur (une ligne à plusieurs noms n'est exemptée que si tous le sont), comme `registres`. Catégorie existante étendue, **pas de catégorie neuve** : `CATEGORIES`, `SORTIE` et la garde ne changent pas.
  2. `unattr()` (dans `CLEAN_AWK`, appelé seulement par `read_structs` et `read_registres`, **pas** par `read_prototypes` : `void-ptr` ne bouge pas) retire `__attribute__((…))` (un niveau de parenthèses imbriquées) et `_Alignas(…)`.
  3. `read_structs` : un corps s'ouvre sur `struct` **ou `union`** (premier niveau et `static` de fonction) ; une tête sans accolade (`typedef struct s`, `static struct`, `static union` dans une fonction) est gardée (`phead`) et jointe à la ligne suivante si celle-ci commence par `{`.
  4. `read_registres` : une déclaration de premier niveau commence aussi sur une ligne `_Alignas…` (le texte lu est `unattr(clean($0))`).
  5. Commentaires : celui de `PTRS_NON_ENTITE` dit ce que `ptrs-entité` lit ; celui de `REGISTRES_NON_ENTITE` écrit les nouvelles formes vues et la limite « struct ou union imbriquée sur plusieurs lignes ».
- `tests/test_composition_residue.c` (annexe B) : deux tests neufs, appelés dans `main` après `test_registres_sees_hidden_forms_and_not_its_named_limits`. Aucun attendu existant ne change.
- `tests/test_pico8_jc.c` (annexe C) : `test_bv_takes_the_first_slipway_of_cg`, appelé après `test_bv`. Aucune empreinte (`*_HASH`) neuve ni changée.

**Pas de TU neuf, pas de `Makefile`** : le test de `bv` entre dans `test_pico8_jc.c`, qui construit déjà le monde d'un `jc` (`world()`) et le `g_` sondeur de `test_bv`.

### Formes mesurées (run 3 : `forms.sh`, un mini-arbre par forme, `--check registres` / `--check ptrs-entité`)

| # | Forme (type d'entité : 1er membre `pico8_ke_t ke;`) | parent `317b127` | 2.11r-d |
|---|---|---|---|
| E1 | entité : `void **items;` | 0 / 0 | 0 / **1** |
| E2 | entité : `void *slots[8];` | 0 / 0 | 0 / **1** |
| E3 | entité : `reg17_t l;` (alias de `pico8_ptrs_t`) | 0 / 0 | 0 / **1** |
| E4, E5 | entité : `pico8_ptrs_t l;`, `pico8_ptrs_t *l;` | 0 / 1 | 0 / 1 |
| E6 | entité : `pico8_ptrs_t l[4];` | 0 / 0 | 0 / **1** |
| E7 | entité : `pico8_ptrs_t **l;` | 0 / 0 | 0 / **1** |
| E8 | entité : `pico8_ptrs_t *a, *b;` | 0 / 0 | 0 / **1** |
| E9 | entité : `const pico8_ptrs_t *const x;` | 0 / 0 | 0 / **1** |
| E10 | entité : `void *(*get)(void);` | 0 / 0 | 0 / **1** |
| E11 | entité : `pico8_jo_t.jy` (exempté) | 0 / 0 | 0 / 0 |
| E12 | entité : `void *owner;` (limite, G2 de 2.11r) | 0 / 0 | 0 / 0 |
| L1 | `typedef struct s` / `{` / `pico8_ptrs_t l;` | 0 / 0 | **1** / 0 |
| L1b | `static struct` / `{` … `} g;` | 0 / 0 | **1** / 0 |
| L2, L2b, L2c | `typedef union {…}` sur plusieurs lignes, sur une ligne, `static union {…} g;` | 0 / 0 | **1** / 0 chacune |
| L2d | membre `union { pico8_ptrs_t l; } u;` (une ligne) | 1 / 0 | 1 / 0 |
| L2e | union imbriquée sur plusieurs lignes | 0 / 0 | 0 / 0 (**limite**, comme la struct imbriquée) |
| L3, L3b | `_Alignas(8) static pico8_ptrs_t g;`, `_Alignas(8) pico8_ptrs_t g;` (`.c`) | 0 / 0 | **1** / 0 |
| L3c, L3g | membre `_Alignas(8) pico8_ptrs_t l;` ; `extern _Alignas(8) pico8_ptrs_t g;` (`.h`) | 1 / 0 | 1 / 0 |
| L3d, L3e, L3f | membre `_Alignas(8) void **items;` ; `_Alignas(8) static void *g[8];` ; `_Alignas(8) static struct {…} g;` | 0 / 0 | **1** / 0 chacune |
| L4, L4b, L4c | `typedef struct __attribute__((packed)) {…}` ; `static struct __attribute__((aligned(8))) {…} g;` ; `struct __attribute__((packed)) s {…};` | 0 / 0 | **1** / 0 chacune |
| L4d | membre `void **items __attribute__((aligned(8)));` | 0 / 0 | **1** / 0 |

Les quatre limites signalées sont donc **vues** (préférence du vérificateur) ; il reste écrit comme limite : union ou struct imbriquée sur plusieurs lignes (déjà limite de 2.11r pour la struct), plus les limites de 2.11r inchangées (alias d'alias, `void *` derrière un `typedef`, membre `void *` simple, locale, macro).

## User Stories

1. En tant que gardien de la Sortie de phase, je veux qu'un conteneur d'entités non typé **dans un type d'entité** soit compté sous toutes les formes que `registres` compte ailleurs, afin que « résidu 2 : 0 » ne cache aucun `void **` ni alias.
2. En tant que lecteur du compteur, je veux que l'accolade à la ligne, l'union, `_Alignas` et `__attribute__` ne fassent pas sortir une déclaration du compte, et que la seule limite neuve (union imbriquée) soit écrite dans la fixture.
3. En tant que gardien du jeu, je veux qu'un `bv` qui viserait le dernier `jc` croisé au lieu du premier rougisse un TU, sans rejeu d'`events.json`.
4. En tant que vérificateur, je veux chaque ajout du script prouvé par une mutation qui rougit la fixture, et tous les comptes du dépôt inchangés.

## Implementation Decisions

### Modules

`tools/composition_residue.sh` ; TU `composition_residue`, `jc` ; `docs/plan-3.md` (ligne 2.11r-d et sa note). **Pas** de code du jeu, **pas** de `Makefile`.

### Décisions (G)

- **G1. `ptrs-entité` étendue, pas de catégorie neuve.** Le lecteur de structs collecte déjà `nr[k]` pour **toute** struct ; `registres` n'en garde que les non-entités, `ptrs-entité` en garde maintenant les entités. Une seule définition de « membre conteneur » pour les deux catégories : une forme qu'on ajoutera à l'une vaudra pour l'autre. Le motif ancré de `ptrs-entité` disparaît (ses deux cas, `pico8_ptrs_t l;` et `*l;`, restent vus : E4, E5).
- **G2. Le pointeur de fonction qui rend `void *` compte aussi dans un type d'entité** (E10) : c'est la même lecture ; aucun n'existe dans `src/` (mesuré : l'exemption `PTRS_NON_ENTITE` vidée rend exactement les quatre listes exemptées, rien d'autre, run 5).
- **G3. `unattr` hors de `read_prototypes`** : `void-ptr` garde ses 8 / 4 sans raisonnement de plus ; aucun `__attribute__`/`_Alignas` n'existe dans `src/` ni `tests/` (grep, run 2).
- **G4. `union` ouvre un corps comme `struct`**, sans entrer dans les alias `typedef struct X Y;` ni dans les noms : un `union` de premier niveau n'est jamais un type d'entité (il n'a pas l'en-tête en premier membre stable). La seule `union` de `src/` est imbriquée (`layout.h:44`, dans `pico8_lq_cell_t`, sans conteneur) ; elle reste hors lecture.
- **G5. Le test de `bv` dans `test_pico8_jc.c`**, pas un TU neuf : il réutilise `world()`, `build_jc`, `pos_of`, et évite le `Makefile`, que 2.12 touche.
- **G6. Le jumeau du `jc` est posé par écriture de TU** (`as_jc(twin)->re = as_jc(old)->qu`) : un `jc` construit par `build_jc(re)` puis retourné couvre le même segment ; le test vérifie d'abord l'ordre de `cg("jc")` (`old` puis `twin`), puis vise le milieu du segment avec `dw = 0`, comme `test_bv`.

## Implementation Chunks

Un commit, ≈ 50 min preuves comprises : appliquer l'annexe A ; ajouter les annexes B et C ; runs 1 à 10 ci-dessous ; ligne du plan et note ; commit.

**Repli** (au-delà de 1,5 × le budget, rien d'incomplet commis) : **d1** = trou 1 (G1, test `test_ptrs_entite_reads_entity_members_like_registres` sans sa ligne 14 `_Alignas`, mutations S10, S20) et trou 3 (annexe C) ; **d2** = trou 2 (`unattr`, `union`, `phead`, `_Alignas` de `read_registres`, test des limites, ligne 14 rendue, mutations S11 à S19). Aucun compte ne change à l'un ni à l'autre. Le repli est écrit dans la note « Découvert à ».

## Testing Decisions

- Aucun rejeu d'`events.json` dans un TU ; `make e2e` part de l'écran titre. **Prior art** : tests de 2.11r-a (annexe B de son PRD), `test_bv` de `test_pico8_jc.c`.

### Preuves

1. `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. Mesuré (runs 7, 10).
2. **Fixture rouge au parent, verte au neuf** : avec le script de `<parent>`, `test_ptrs_entite_reads_entity_members_like_registres` abat (code 134, à `--check ptrs-entité` code 0 au lieu de 1) ; `test_registres_sees_split_braces_unions_and_attributes`, seul (l'autre retiré de `main`), abat aussi (code 134). Vert avec le script neuf. Mesuré (runs 4, 6).
3. **Comptes inchangés, mesurés sur la copie** : `--check` de chacune des treize catégories, `--detail`, `--champs` et le compteur sans option, ancien et nouveau script sur le même arbre : **identiques octet pour octet** (`cmp`), à `317b127` (avant et après ajout des tests) et à `f3d1c50` + 2.12b non commis. Pied **1073 / 85** ; `ptrs-entité` et `registres` à 0. Toute différence : arrêt, rapporté.
4. **Mutations du script**, une à la fois sur le script neuf ; chacune rougit la fixture (code 134), et le pied de `src/ tests/` reste 1073 / 85 sauf S20. Mesuré (run 6) :

| # | Ajout retiré | Premier test rouge |
|---|---|---|
| S10 | `ptrs-entité` ne garde de `nr` que les lignes de l'ancien motif ancré | `…ptrs_entite_reads_entity_members…` |
| S20 | exemption `PTRS_NON_ENTITE` ignorée dans la boucle neuve | `test_ptrs_members_count_only_on_entity_types` (et pied 1077 / 87 : les quatre listes exemptées) |
| S11 | `unattr` sans le `gsub` de `__attribute__` | `…split_braces_unions_and_attributes` |
| S12 | `unattr` sans le `gsub` de `_Alignas` | `…ptrs_entite_reads_entity_members…` (ligne 14) |
| S13 | `read_registres` ne commence plus sur `_Alignas` | `…split_braces…` |
| S14 | `union` retiré des deux têtes de premier niveau | `…split_braces…` |
| S15 | `union` retiré des deux têtes `static` de fonction | `…split_braces…` |
| S16 | `phead` jamais posé (accolade à la ligne non jointe) | `…ptrs_entite_reads_entity_members…` (ligne 21) |
| S17 | tête sans accolade seulement au premier niveau | `…split_braces…` (`f.c:19`) |
| S18 | `read_registres` lit `clean($0)` sans `unattr` | `…split_braces…` |
| S19 | `read_structs` lit `clean($0)` sans `unattr` | `…ptrs_entite_reads_entity_members…` |

   Les commandes de mutation sont des remplacements de texte exacts ; le script `mut.py` des essais les porte (annexe D).
5. **Garde mordante sur l'arbre réel** (copie) : `void **items;`, puis `void *slots[8];`, puis `typedef pico8_ptrs_t reg17_t;` en tête de `wormhole.h` et `reg17_t l;`, ajoutés en dernier membre de `pico8_mz_t` (`wormhole.h:44`) : `make composition-guard` **code 0 au parent**, **code 2 au neuf**, `ptrs-entité: src/pico8_wormhole.h:44:    void **items;` (resp. `:44`, `:45`), `résidu gardé (…) : 1 ligne(s)`. Mesuré (run 8).
6. **Trou de test** : `src/pico8_probe.c:179` muté en `first = filtered.items[filtered.count - 1];` : au parent (tests de `317b127`), `make test` **vert** et `make e2e` **26 conformes** ; au neuf, `make test` **rouge**, `tests/test_pico8_jc.c:898: test_bv_takes_the_first_slipway_of_cg: Assertion '…->probe->et == old' failed`. Mutation retirée : vert. Mesuré (run 9). **ÉCHEC si la mutation ne rougit pas au neuf.**
7. `--champs`, `void-ptr` (8 / 4), `inclusion-mutuelle` (17 / 16) : identiques au parent.

### Budget et tableau « Runs »

Budget **≈ 50 min**. Jamais l'appli raylib ; `make raylib` non exigé (aucune liste de sources ne change). L'implémenteur tient un tableau « Runs », une ligne par lancement. Celui des essais de ce PRD (copie de `317b127`) :

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone` local, `checkout 317b127` ; compteur, `--champs`, garde ; `snap.sh` (13 `--check`, `--detail`, `--champs`, compteur) | parent | 0 | 1073 / 85 ; 282 / 15 / 27 ; garde `…,ptrs-entité,registres` | 4 s |
| 2 | grep `union`, `__attribute__`, `_Alignas`, tête de struct sans accolade dans `src/`, `tests/` | parent | 0 | une `union` imbriquée (`layout.h:44`), rien d'autre | < 1 s |
| 3 | `forms.sh` : 30 formes, ancien puis nouveau script | parent ; neuf | 0 | table des formes | ≈ 6 s × 2 |
| 4 | annexes A, B ; `make tmp/test_composition_residue` ; TU | neuf | 0 | `all tests passed` | 3 s |
| 5 | `snap.sh` au neuf, `cmp` aux 16 sorties du parent ; `PTRS_NON_ENTITE` vidée, `--check ptrs-entité` | neuf | 0 | identiques ; 4 lignes (`economy.h:23`, `planet.h:46`, `research.h:40`, `slipway.h:43`), comme l'ancien script vidé | 4 s |
| 6 | TU avec le script du parent ; test des limites seul au parent ; mutations S10 à S20 | parent ; neuf | 134 ×13 | table S | ≈ 1 s × 13 |
| 7 | `timeout 900 make -j8 test` | neuf | 0 | vert | 17 s |
| 8 | trois formes injectées dans `pico8_mz_t` ; `make composition-guard`, ancien puis nouveau script | parent ; neuf | 0 ; 2 | point 5 | < 1 s × 6 |
| 9 | annexe C ; TU `jc` ; mutation `items[count - 1]` : TU `jc` au neuf ; `make test` + `make e2e` aux tests du parent ; `make test` au neuf | neuf ; parent | 0 ; 134 ; 0, 0 ; 2 | point 6 ; 26 conformes au parent muté | ≈ 20 s × 3 |
| 10 | mutation retirée ; `make test` ; `make e2e` ; `snap.sh` ; `cmp` | neuf | 0 | vert ; 26 conformes ; 16 sorties identiques au parent | ≈ 25 s |
| 11 | ancien et nouveau script sur le dépôt vivant (`f3d1c50` + 2.12b non commis), compteur, `--detail`, `--champs`, `--check ptrs-entité,registres` | vivant, lecture seule | 0 | identiques ; 1073 / 85 ; `champs : 282 ; 15 ; 23` ; 0 ligne | 4 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si une des treize catégories, `--detail`, `--champs` ou le pied diffère entre l'ancien et le nouveau script sur l'arbre du commit, sans explication.
- `FAILED:` si une ligne attendue des deux tests neufs (annexe B) n'est pas comptée, si une forme en gras de la table des formes n'est pas vue par le script du commit (`forms.sh`, annexe D, à rejouer), ou si la limite `f.h:19` (union imbriquée) se met à compter sans que l'attendu et le commentaire du script le disent.
- `FAILED:` si une mutation S10 à S20 laisse la fixture verte, ou si la fixture est verte avec le script du parent.
- `FAILED:` si la mutation `items[filtered.count - 1]` de `bv` ne rougit pas `make test` au commit.
- `FAILED:` si `make test` ou `make e2e` (26 conformes) est rouge, si `tests/e2e/` ou un `*.sha256` change, si une constante `*_HASH`/`FROZEN_*` change, si le `Makefile` ou un fichier de `src/` change.
- `FAILED:` si le message de commit omet : les formes vues et les limites, la table S avec les premiers tests rouges, la garde mordante, la mutation de `bv` (vert au parent, rouge au neuf), le tableau « Runs ».
- `FAILED:` s'il manque la ligne 2.11r-d et sa note « Découvert à 2.11r-d » dans `docs/plan-3.md`, ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

### Recouvrements avec 2.12 (en parallèle)

2.12a/2.12b touchent `src/pico8_game_data.[ch]`, `src/pico8_economy.[ch]`, `tests/test_pico8_archetype_chain.c`, le `Makefile` (`UNIT_TESTS`) et `docs/plan-3.md` (notes « Découvert à 2.12x »). 2.11r-d ne touche **ni le `Makefile` ni `src/`** ; seul `docs/plan-3.md` est commun (lignes distinctes : 2.11r-d s'insère après la ligne 2.11r-c ; `git pull --rebase` suffit si 2.12 commit entre-temps). Les archétypes de 2.12 (`pico8_X_def_t`) sont de nouvelles structs : le nouveau script les lit comme l'ancien (mesuré à `f3d1c50` + 2.12b non commis, run 11) ; si 2.12c à e y mettaient un `void **` ou un alias de `pico8_ptrs_t`, le nouveau `ptrs-entité` ou `registres` le compterait — c'est voulu.

## Out of Scope

- Union ou struct imbriquée sur plusieurs lignes, alias d'alias, `void *` derrière un `typedef`, membre `void *` simple, locale, macro : limites écrites (fixture et commentaire du script) ; aucune n'existe dans `src/`.
- L'ordre de `la` dans `jc:ce` (M10 de 2.11r) : trou déclaré, hors de cette étape.
- L'appli raylib, l'audio, PSX.

## Further Notes

### Surprises de mesure, pour `plan-pilot`

1. **Le trou 1 était plus large que signalé** : dans un type d'entité, `ptrs-entité` ne voyait que `pico8_ptrs_t l;` et `pico8_ptrs_t *l;` ; tableau, `**`, déclarateurs multiples, `*const` et pointeur de fonction lui échappaient aussi.
2. **Trois des formes « limites » étaient déjà vues** par le jeton (`_Alignas` en membre ou après `extern`, union membre sur une ligne) ; les vraies limites étaient celles où `_Alignas` ou `__attribute__` cassent un motif ancré, et l'accolade à la ligne.
3. **`--champs` passe à `0 hors constructeur : 23` avec 2.12a/b** (27 à `317b127`) : effet de 2.12, pas de ce script (ancien et nouveau identiques).

### Questions pour l'humain

Aucune.

## Annexe A — `tools/composition_residue.sh`, diff contre `317b127`

```diff
diff --git a/tools/composition_residue.sh b/tools/composition_residue.sh
index 8a0468b..e725205 100644
--- a/tools/composition_residue.sh
+++ b/tools/composition_residue.sh
@@ -78,7 +78,9 @@ exclusion() {
 # le sed GNU (\b), comme les grep du script.
 VOID_EXCLUSION='s/void \*+(r_)?ctx\b//g'
 
-# Listes pico8_ptrs_t d'un type d'entite qui ne portent pas d'entites
+# ptrs-entite : les membres d'un type d'entite que registres compterait hors
+# d'un type d'entite (REG_TYPES par jeton, void **, void *[N], pointeur de
+# fonction qui rend void *). Listes qui ne portent pas d'entites
 # (type.membre), chacune prouvee par son ecrivain : hy.invented des chaines
 # (economy.c:284), jo.jy des entrees (economy_tiers.c:167), hp.i des
 # enregistrements jy (research.c:233), jc.ts des particules (slipway_display.c:43).
@@ -92,11 +94,13 @@ PTRS_NON_ENTITE="pico8_hy_t.invented pico8_jo_t.jy pico8_hp_t.i pico8_jc_t.ts"
 # void *[N] ; membre qui rend void * par pointeur de fonction ; objet de portee
 # fichier, statique de fonction, extern de .h, de ces types ou void * ;
 # prototype ou definition inline de .h qui prend ou rend un pico8_ptrs_t, ou
-# rend void **. Non vus : alias d'alias, void * derriere un typedef, struct
-# imbriquee, membre void * simple, locale, macro. Exemptes, chacun prouve par
-# son ecrivain : emitter.qy des particules (particle.c:65), le ji du contexte
-# de planet_display.c des enregistrements jy (:79, :186) ; pico8_nv_ptrs sur
-# hp.i (planet.c:319), pico8_gu_content_ptrs sur des chaines (tech_gate.c:26),
+# rend void **. Une struct ou une union dont l'accolade ouvre la ligne suivante
+# est vue ; __attribute__((...)) et _Alignas(...) sont retires avant lecture.
+# Non vus : alias d'alias, void * derriere un typedef, struct ou union
+# imbriquee sur plusieurs lignes, membre void * simple, locale, macro.
+# Exemptes, chacun prouve par son ecrivain : emitter.qy des particules
+# (particle.c:65), le ji du contexte de planet_display.c des enregistrements
+# jy (:79, :186) ; pico8_nv_ptrs sur hp.i (planet.c:319), pico8_gu_content_ptrs sur des chaines (tech_gate.c:26),
 # pico8_jo_ex sur des enregistrements jy (planet_display.c:54) ;
 # pico8_jp_ptrs et pico8_la_ptrs sur des enregistrements jy (planet_display.c:186,
 # planet_trade.c:128), les listes d'entites passant par pico8_jp_ents et
@@ -198,6 +202,11 @@ function clean(s,    out, c, q) {
     }
     return out
 }
+function unattr(s) {
+    gsub(/__attribute__[ \t]*\(\([^()]*(\([^()]*\)[^()]*)*\)\)[ \t]*/, "", s)
+    gsub(/_Alignas[ \t]*\([^()]*\)[ \t]*/, "", s)
+    return s
+}
 function braces(s,    t, n) {
     t = s; n = gsub(/\{/, "", t); depth += n
     t = s; n = gsub(/\}/, "", t); depth -= n
@@ -215,15 +224,21 @@ read_structs() {
     awk -v non_entite=" $PTRS_NON_ENTITE " -v out_parente="$work/parente" \
         -v reg_non_entite=" $REGISTRES_NON_ENTITE " -v out_reg="$work/registres.structs" -v reg_types="$REG_TYPES" \
         -v out_ptrs="$work/ptrs-entité" -v out_members="$work/members" "$CLEAN_AWK"'
-    FNR == 1 { depth = 0; incomment = 0; inbody = 0; base = 0 }
+    FNR == 1 { depth = 0; incomment = 0; inbody = 0; base = 0; phead = "" }
     {
-        c = clean($0)
+        c = unattr(clean($0))
+        if (phead != "") { if (c ~ /^[ \t]*\{/) c = phead " " c; phead = "" }
+        if (!inbody && (depth == 0 && c ~ /^((static|extern)[ \t]+)?(typedef[ \t]+)?(const[ \t]+)?(struct|union)([ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*$/ ||
+                        depth > 0 && c ~ /^[ \t]*static[ \t]+(const[ \t]+)?(struct|union)([ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*$/)) {
+            phead = c
+            next
+        }
         if (!inbody && depth == 0 && c ~ /^typedef[ \t]+struct[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]*;/) {
             split(c, w, /[ \t;]+/)
             alias_n++; alias_a[alias_n] = w[3]; alias_b[alias_n] = w[4]
         }
-        if (!inbody && (depth == 0 && c ~ /^((static|extern)[ \t]+)?(typedef[ \t]+)?(const[ \t]+)?struct([ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*\{/ ||
-                        depth > 0 && c ~ /^[ \t]*static[ \t]+(const[ \t]+)?struct([ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*\{/)) {
+        if (!inbody && (depth == 0 && c ~ /^((static|extern)[ \t]+)?(typedef[ \t]+)?(const[ \t]+)?(struct|union)([ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*\{/ ||
+                        depth > 0 && c ~ /^[ \t]*static[ \t]+(const[ \t]+)?(struct|union)([ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*\{/)) {
             n++; inbody = 1; seen = 0; names[n] = " "; base = depth
             sfile[n] = FILENAME; sobj[n] = c ~ /^[ \t]*(static|extern)[ \t]/
             t = c; sub(/^[ \t]*((static|extern)[ \t]+)?(typedef[ \t]+)?(const[ \t]+)?struct[ \t]*/, "", t); sub(/[ \t]*\{.*/, "", t)
@@ -259,10 +274,6 @@ read_structs() {
                     first[n] = t; firstline[n] = FNR; firsttext[n] = $0
                 }
             }
-            if ($0 ~ /^[ \t]+(const[ \t]+)?pico8_ptrs_t[ \t]*\*?[ \t]*[a-z0-9_]+;/) {
-                t = $0; sub(/;.*/, "", t); sub(/.*[ \t*]/, "", t)
-                np[n]++; pmem[n, np[n]] = t; pline[n, np[n]] = FNR; ptext[n, np[n]] = $0
-            }
             if (c ~ /^[ \t]*(const[ \t]+)?void[ \t]*(\*[ \t]*)+\([ \t]*\*/) {
                 t = c; sub(/\).*/, "", t); sub(/.*[ \t*(]/, "", t)
                 nr[n]++; rmem[n, nr[n]] = t; rline[n, nr[n]] = FNR; rtext[n, nr[n]] = $0
@@ -335,10 +346,12 @@ read_structs() {
                     print type "\t" mname[k, j] > out_members
                 }
             }
-            for (j = 1; j <= np[k]; j++) {
-                m = split(names[k], nm, " "); skip = 0
-                for (i = 1; i <= m; i++) if (index(non_entite, " " nm[i] "." pmem[k, j] " ")) skip = 1
-                if (!skip) print sfile[k] ":" pline[k, j] ":" ptext[k, j] > out_ptrs
+            m = split(names[k], nm, " ")
+            for (j = 1; j <= nr[k]; j++) {
+                q = split(rmem[k, j], mm, " "); left = q
+                for (h = 1; h <= q; h++)
+                    for (i = 1; i <= m; i++) if (index(non_entite, " " nm[i] "." mm[h] " ")) { left--; break }
+                if (left > 0 || q == 0) print sfile[k] ":" rline[k, j] ":" rtext[k, j] > out_ptrs
             }
         }
     }' $(cat "$work/files") || fail "echec de awk"
@@ -390,8 +403,8 @@ read_registres() {
         awk -v non=" $REGISTRES_NON_ENTITE " -v reg_types="$REG_TYPES" "$CLEAN_AWK"'
         FNR == 1 { depth = 0; incomment = 0; acc = "" }
         {
-            c = clean($0)
-            if (acc == "" && ((depth == 0 && $0 ~ /^[a-z]/) || (depth > 0 && c ~ /^[ \t]*static[ \t]/))) {
+            c = unattr(clean($0))
+            if (acc == "" && ((depth == 0 && $0 ~ /^([a-z]|_Alignas)/) || (depth > 0 && c ~ /^[ \t]*static[ \t]/))) {
                 start = FNR; text = $0; acc = " "; local = depth > 0
             }
             if (acc != "") {
```

## Annexe B — `tests/test_composition_residue.c`

Insérés avant `test_void_ptr_reads_joined_public_prototypes` ; dans `main`, après `test_registres_sees_hidden_forms_and_not_its_named_limits();` :

```c
    test_ptrs_entite_reads_entity_members_like_registres();
    test_registres_sees_split_braces_unions_and_attributes();
```

```c
/* ptrs-entité lit les membres d'un type d'entité comme registres lit ceux des
 * autres structs : aucun conteneur non typé n'échappe aux deux. */
static void test_ptrs_entite_reads_entity_members_like_registres(void) {
    setup();
    write_file(SRC "/a.h", "typedef pico8_" "ptrs_t reg17_t;\n");
    write_file(SRC "/e.h", "typedef struct {\n    pico8_struct_" "header_t header;\n} pico8_" "ke_t;\n"
                           "typedef struct {\n"
                           "    pico8_" "ke_t ke;\n"
                           "    void **items;\n"
                           "    void *slots[8];\n"
                           "    reg17_t l;\n"
                           "    pico8_" "ptrs_t arr[4];\n"
                           "    pico8_" "ptrs_t **pp;\n"
                           "    pico8_" "ptrs_t *a, *b;\n"
                           "    const pico8_" "ptrs_t *const x;\n"
                           "    void *(*get)(void);\n"
                           "    _Alignas(8) void **al;\n"
                           "    void *owner;\n"
                           "    pico8_" "ptrs_t *jy;\n"
                           "} pico8_" "jo_t;\n"
                           "typedef struct pico8_" "nl_s\n"
                           "{\n"
                           "    pico8_" "jo_t jo;\n"
                           "    void **more;\n"
                           "} pico8_" "nl_t;\n");
    assert(run_residue("--check registres " TREE) == 0);
    assert(run_residue("--check ptrs-entité " TREE) == 1);
    assert(strcmp(read_file(OUT), "ptrs-entité: " SRC "/e.h:6:    void **items;\n"
                                  "ptrs-entité: " SRC "/e.h:7:    void *slots[8];\n"
                                  "ptrs-entité: " SRC "/e.h:8:    reg17_t l;\n"
                                  "ptrs-entité: " SRC "/e.h:9:    pico8_" "ptrs_t arr[4];\n"
                                  "ptrs-entité: " SRC "/e.h:10:    pico8_" "ptrs_t **pp;\n"
                                  "ptrs-entité: " SRC "/e.h:11:    pico8_" "ptrs_t *a, *b;\n"
                                  "ptrs-entité: " SRC "/e.h:12:    const pico8_" "ptrs_t *const x;\n"
                                  "ptrs-entité: " SRC "/e.h:13:    void *(*get)(void);\n"
                                  "ptrs-entité: " SRC "/e.h:14:    _Alignas(8) void **al;\n"
                                  "ptrs-entité: " SRC "/e.h:21:    void **more;\n"
                                  "résidu gardé (ptrs-entité) : 10 ligne(s)\n") == 0);
}

/* Accolade à la ligne, union, __attribute__ et _Alignas : vus. Une union
 * imbriquée sur plusieurs lignes (f.h:19) reste une limite, comme la struct
 * imbriquée. */
static void test_registres_sees_split_braces_unions_and_attributes(void) {
    setup();
    write_file(SRC "/f.h", "typedef struct reg_s\n"
                           "{\n"
                           "    pico8_" "ptrs_t l;\n"
                           "} reg_s_t;\n"
                           "typedef union {\n"
                           "    pico8_" "ptrs_t l;\n"
                           "    int x;\n"
                           "} reg_u_t;\n"
                           "typedef union { pico8_" "ptrs_t l; } reg_u1_t;\n"
                           "typedef struct __attribute__((packed)) {\n"
                           "    pico8_" "ptrs_t l;\n"
                           "} reg_p_t;\n"
                           "struct __attribute__((aligned(8))) reg_q {\n"
                           "    void **items __attribute__((aligned(8)));\n"
                           "};\n"
                           "typedef struct {\n"
                           "    _Alignas(8) void **items;\n"
                           "    union {\n"
                           "        pico8_" "ptrs_t l;\n"
                           "    } inner;\n"
                           "} reg_a_t;\n");
    write_file(SRC "/f.c", "_Alignas(8) static pico8_" "ptrs_t g_al;\n"
                           "_Alignas(8) static void *g_slots[8];\n"
                           "static union {\n"
                           "    pico8_" "ptrs_t l;\n"
                           "} g_u;\n"
                           "static struct\n"
                           "{\n"
                           "    pico8_" "ptrs_t l;\n"
                           "} g_s;\n"
                           "_Alignas(8) static struct {\n"
                           "    pico8_" "ptrs_t l;\n"
                           "} g_as;\n"
                           "static void reg_f(void) {\n"
                           "    static union {\n"
                           "        pico8_" "ptrs_t l;\n"
                           "    } u;\n"
                           "    static struct\n"
                           "    {\n"
                           "        pico8_" "ptrs_t l;\n"
                           "    } s;\n"
                           "    (void)u;\n"
                           "    (void)s;\n"
                           "}\n");
    assert(run_residue("--check registres " TREE) == 1);
    assert(strcmp(read_file(OUT), "registres: " SRC "/f.c:1:_Alignas(8) static pico8_" "ptrs_t g_al;\n"
                                  "registres: " SRC "/f.c:2:_Alignas(8) static void *g_slots[8];\n"
                                  "registres: " SRC "/f.c:4:    pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.c:8:    pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.c:11:    pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.c:15:        pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.c:19:        pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.h:3:    pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.h:6:    pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.h:9:typedef union { pico8_" "ptrs_t l; } reg_u1_t;\n"
                                  "registres: " SRC "/f.h:11:    pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.h:14:    void **items __attribute__((aligned(8)));\n"
                                  "registres: " SRC "/f.h:17:    _Alignas(8) void **items;\n"
                                  "résidu gardé (registres) : 13 ligne(s)\n") == 0);
}
```

## Annexe C — `tests/test_pico8_jc.c`, diff contre `317b127`

```diff
diff --git a/tests/test_pico8_jc.c b/tests/test_pico8_jc.c
index 4613c15..6b5c0d3 100644
--- a/tests/test_pico8_jc.c
+++ b/tests/test_pico8_jc.c
@@ -872,6 +872,33 @@ static void test_bv(void) {
     check("bv", h, BV_HASH);
 }
 
+/* `bv` aims at the first slipway of `cg("jc")` under the probe (`jp(...)[1]`),
+ * not the last: a second slipway laid over the first, registered after it,
+ * never wins. */
+static void test_bv_takes_the_first_slipway_of_cg(void) {
+    void *old;
+    void *twin;
+    void *probe;
+    pico8_vec_t qu;
+    pico8_vec_t re;
+    pico8_vec_t at;
+
+    world();
+    old = the_slipway();
+    twin = build_jc(as_jc(old)->re);
+    as_jc(twin)->re = as_jc(old)->qu;
+    assert(pico8_cg("jc")->count == 2 && pico8_cg("jc")->items[0] == old &&
+           pico8_cg("jc")->items[1] == twin);
+    probe = pico8_g__new(NULL);
+    ((pico8_g__t *)probe)->probe->dw = 0;
+    qu = pos_of(as_jc(old)->qu);
+    re = pos_of(as_jc(old)->re);
+    at = pico8_vec_div(pico8_vec_add(qu, re), pico8_fixed_from_int(2));
+    cls_of(probe)->hq(probe, vec_ref_fixed(pico8_vec_mul(at, pico8_fixed_from_int(128))), vec_ref(64, 64));
+    assert(((pico8_g__t *)probe)->probe->et == old);
+    assert(as_jc(old)->fo == probe && as_jc(twin)->fo != probe);
+}
+
 static int is_a_jc_struct(const void *e) {
     return e != NULL &&
            ((const pico8_struct_header_t *)e)->cls == pico8_jc_struct_class();
@@ -1061,6 +1088,7 @@ int main(void) {
     in_fresh_child(test_eb);
     in_fresh_child(test_draw);
     in_fresh_child(test_bv);
+    in_fresh_child(test_bv_takes_the_first_slipway_of_cg);
     in_fresh_child(test_too_far);
     in_fresh_child(test_hq_mouse_point);
     in_fresh_child(test_ce_crosses_a_slipway);
```

## Annexe D — outils des essais

`mut.py SRC DST Sxx` écrit dans `DST` le script `SRC` muté (remplacements exacts ; un `assert` échoue si le texte a changé) :

```python
import sys
src, dst, which = sys.argv[1], sys.argv[2], sys.argv[3]
s = open(src, encoding='utf-8').read()
def r(old, new, n=1):
    global s
    assert s.count(old) == n, (which, old, s.count(old))
    s = s.replace(old, new)
U_TOP1 = '(const[ \\t]+)?(struct|union)([ \\t]+[A-Za-z_][A-Za-z0-9_]*)?[ \\t]*$/ ||'
U_TOP2 = '(const[ \\t]+)?(struct|union)([ \\t]+[A-Za-z_][A-Za-z0-9_]*)?[ \\t]*\\{/ ||'
U_FN1 = 'static[ \\t]+(const[ \\t]+)?(struct|union)([ \\t]+[A-Za-z_][A-Za-z0-9_]*)?[ \\t]*$/)) {'
U_FN2 = 'static[ \\t]+(const[ \\t]+)?(struct|union)([ \\t]+[A-Za-z_][A-Za-z0-9_]*)?[ \\t]*\\{/)) {'
if which == 'S10':   # ptrs-entite ne lit que pico8_ptrs_t / *nom (ancien lecteur)
    r('            m = split(names[k], nm, " ")\n            for (j = 1; j <= nr[k]; j++) {\n                q = split(rmem[k, j], mm, " "); left = q\n                for (h = 1; h <= q; h++)\n                    for (i = 1; i <= m; i++) if (index(non_entite,',
      '            m = split(names[k], nm, " ")\n            for (j = 1; j <= nr[k]; j++) {\n                if (rtext[k, j] !~ /^[ \\t]+(const[ \\t]+)?pico8_ptrs_t[ \\t]*\\*?[ \\t]*[a-z0-9_]+;/) continue\n                q = split(rmem[k, j], mm, " "); left = q\n                for (h = 1; h <= q; h++)\n                    for (i = 1; i <= m; i++) if (index(non_entite,')
elif which == 'S11': r('    gsub(/__attribute__', '    if (0) gsub(/__attribute__')
elif which == 'S12': r('    gsub(/_Alignas', '    if (0) gsub(/_Alignas')
elif which == 'S13': r('$0 ~ /^([a-z]|_Alignas)/', '$0 ~ /^[a-z]/')
elif which == 'S14': r(U_TOP1, U_TOP1.replace('(struct|union)', 'struct')); r(U_TOP2, U_TOP2.replace('(struct|union)', 'struct'))
elif which == 'S15': r(U_FN1, U_FN1.replace('(struct|union)', 'struct')); r(U_FN2, U_FN2.replace('(struct|union)', 'struct'))
elif which == 'S16': r('            phead = c\n            next\n', '            phead = ""\n')
elif which == 'S17': r('                        depth > 0 && c ~ /^[ \\t]*' + U_FN1, '                        0 && c ~ /^[ \\t]*' + U_FN1)
elif which == 'S18': r('            c = unattr(clean($0))\n            if (acc', '            c = clean($0)\n            if (acc')
elif which == 'S19': r('        c = unattr(clean($0))\n        if (phead', '        c = clean($0)\n        if (phead')
elif which == 'S20': r('if (index(non_entite, " " nm[i] "." mm[h] " ")) { left--; break }', 'if (0) { left--; break }')
else: raise SystemExit('?')
open(dst, 'w', encoding='utf-8').write(s)
```

`forms.sh SCRIPT` (lancé depuis la racine d'une copie ; `$SCRATCH` est le dossier des essais) :

```sh
#!/bin/sh
# forms.sh SCRIPT : une forme par arbre, ligne "<forme>\t<registres>\t<ptrs-entité>"
S=$SCRATCH
scr=$1
ENT='typedef struct {\n    pico8_struct_header_t header;\n} pico8_ke_t;\n'
one() {
  name=$1; shift
  rm -rf $S/ft; mkdir -p $S/ft/src
  printf "$ENT" > $S/ft/src/e.h
  printf "$1" > $S/ft/src/$2
  r=$(COMPOSITION_ATTEINTE=$SCRATCH/empty.txt sh $scr --check registres $S/ft/src 2>&1 | tail -1 | sed 's/.*: //; s/ ligne(s)//')
  p=$(COMPOSITION_ATTEINTE=$SCRATCH/empty.txt sh $scr --check ptrs-entité $S/ft/src 2>&1 | tail -1 | sed 's/.*: //; s/ ligne(s)//')
  printf '%s\t%s\t%s\n' "$name" "$r" "$p"
}
# trou 1 : membres d'un type d'entite
one E1-voidpp 'typedef struct {\n    pico8_ke_t ke;\n    void **items;\n} pico8_mz_t;\n' m.h
one E2-voidarr 'typedef struct {\n    pico8_ke_t ke;\n    void *slots[8];\n} pico8_mz_t;\n' m.h
one E3-alias 'typedef pico8_ptrs_t reg17_t;\ntypedef struct {\n    pico8_ke_t ke;\n    reg17_t l;\n} pico8_mz_t;\n' m.h
one E4-ptrs 'typedef struct {\n    pico8_ke_t ke;\n    pico8_ptrs_t l;\n} pico8_mz_t;\n' m.h
one E5-ptrsptr 'typedef struct {\n    pico8_ke_t ke;\n    pico8_ptrs_t *l;\n} pico8_mz_t;\n' m.h
one E6-ptrsarr 'typedef struct {\n    pico8_ke_t ke;\n    pico8_ptrs_t l[4];\n} pico8_mz_t;\n' m.h
one E7-ptrspp 'typedef struct {\n    pico8_ke_t ke;\n    pico8_ptrs_t **l;\n} pico8_mz_t;\n' m.h
one E8-multi 'typedef struct {\n    pico8_ke_t ke;\n    pico8_ptrs_t *a, *b;\n} pico8_mz_t;\n' m.h
one E9-constc 'typedef struct {\n    pico8_ke_t ke;\n    const pico8_ptrs_t *const x;\n} pico8_mz_t;\n' m.h
one E10-fnret 'typedef struct {\n    pico8_ke_t ke;\n    void *(*get)(void);\n} pico8_mz_t;\n' m.h
one E11-exempt 'typedef struct {\n    pico8_ke_t ke;\n    pico8_ptrs_t *jy;\n} pico8_jo_t;\n' m.h
one E12-voidsimple 'typedef struct {\n    pico8_ke_t ke;\n    void *owner;\n} pico8_mz_t;\n' m.h
# trou 2 : limites de registres
one L1-brace-nl 'typedef struct s\n{\n    pico8_ptrs_t l;\n} s_t;\n' r.h
one L1b-brace-nl-anon 'static struct\n{\n    pico8_ptrs_t l;\n} g;\n' r.c
one L2-union-typedef 'typedef union {\n    pico8_ptrs_t l;\n    int x;\n} u_t;\n' r.h
one L2b-union-one 'typedef union { pico8_ptrs_t l; } u_t;\n' r.h
one L2c-union-static 'static union {\n    pico8_ptrs_t l;\n} g;\n' r.c
one L2d-union-nested1 'typedef struct {\n    int k;\n    union { pico8_ptrs_t l; } u;\n} s_t;\n' r.h
one L2e-union-nested 'typedef struct {\n    int k;\n    union {\n        pico8_ptrs_t l;\n    } u;\n} s_t;\n' r.h
one L3-alignas-obj '_Alignas(8) static pico8_ptrs_t g;\n' r.c
one L3b-alignas-obj2 '_Alignas(8) pico8_ptrs_t g;\n' r.c
one L3c-alignas-mem 'typedef struct {\n    _Alignas(8) pico8_ptrs_t l;\n} s_t;\n' r.h
one L3d-alignas-voidpp 'typedef struct {\n    _Alignas(8) void **items;\n} s_t;\n' r.h
one L3e-alignas-voidarr '_Alignas(8) static void *g[8];\n' r.c
one L3f-alignas-structobj '_Alignas(8) static struct {\n    pico8_ptrs_t l;\n} g;\n' r.c
one L3g-alignas-hdr 'extern _Alignas(8) pico8_ptrs_t g;\n' r.h
one L4-attr 'typedef struct __attribute__((packed)) {\n    pico8_ptrs_t l;\n} s_t;\n' r.h
one L4b-attr-static 'static struct __attribute__((aligned(8))) {\n    pico8_ptrs_t l;\n} g;\n' r.c
one L4c-attr-named 'struct __attribute__((packed)) s {\n    pico8_ptrs_t l;\n};\n' r.h
one L4d-attr-mem 'typedef struct {\n    void **items __attribute__((aligned(8)));\n} s_t;\n' r.h
```
