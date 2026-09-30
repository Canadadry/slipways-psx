---
title: "Plan 3, étape 2.11r : la catégorie `registres` du compteur (V1 de l'étude 2.10, élargie aux formes cachées que le vérificateur a injectées), puis `hv`, `cv`, `ui.ic`, les couches de `dc`, `cg` et `new_entity` typés (`registres` 0 → 11 → 9 → 6 → 1 → 0, gardé ; pied 1073/85 → 1084/87 → 1073/85) — cinq commits, jeu identique"
description: "Les registres `hv`, `cv`, `cg`, `ui.ic.layers` et le `new_entity` des sortes tiennent des entités par `void *` hors de toute catégorie (perte acceptée de D.20b, 2, restée partielle après 2.8). 2.11r-a ajoute au compteur la catégorie `registres` (points de déclaration hors types d'entité), élargie pour voir les onze formes que le vérificateur de 2.10 a injectées sans qu'elle les voie ; 2.11r-b1 à -c typent un registre par commit sur `pico8_ents_t`, les aides de TU `listed`/`rank_in` passant par des jumelles `ents_` ; 2.11r-c garde la catégorie. Tout est essayé sur une copie : cinq commits verts, 26 e2e conformes à chacun."
status: needs-triage
---

## Problem Statement

**Écrit sur `9ca66f4`** (branche `claude/plan-pilot-3-7zdft0`, pied `résidu 2 : 1073 ligne(s), 85 fichier(s)`, garde terminée par `ptrs-entité`). Aucune ligne du dépôt n'a changé pour écrire ce PRD. Les essais ont été faits sur une seule copie jetable de `9ca66f4` (un commit par sous-étape, rejoués séparément), supprimée depuis ; journaux dans le scratchpad `prd211r/`. **Tous les numéros de ligne sont ceux de `9ca66f4`**. Vérifié en fin d'écriture : l'étude 2.11 a été commise entre-temps (`1cb19cc`, `3a94816`) et ne touche que `docs/etude-2-11-inclusions.md` et `docs/plan-3.md` ; la ligne 2.11r du plan est inchangée (citation ci-dessous, comparée mot pour mot) et aucun fichier de code n'a bougé. Recouvrement avec 2.17 (écrite par 2.11) : 2.11r décale des numéros de ligne dans une vingtaine de modules et retire trois includes de `pico8_ptrs.h` (b3, `entity.h`, `entity.c`, `entity_geom.h`) ; 2.17 retrouve ses sites par leur texte, et un include transitif qui lui manquerait est une erreur de compilation, jamais un changement de sens. `inclusion-mutuelle` reste 17 / 16 à chacun des cinq commits.

Vu du mainteneur du port : `lt()` range chaque entité dans `hv`, dans les seaux `cv[bm]` et `cg[tag]` ; `gw` range les survolées dans `ui.ic.layers[fa]` ; `dc` les trie en couches locales ; `jo:hu` fabrique une planète par `kv[fz].new_entity`. Tous ces conteneurs sont des `pico8_ptrs_t` (`void *`) ou un pointeur de fonction qui rend `void *`. 2.8 a typé les sept listes **membres d'un type d'entité** et a laissé ceux-ci (sa « Surprise 5 ») ; l'étude 2.10 (§3) les a inventoriés et a choisi une catégorie V1 qui compte leurs **points de déclaration** : 11 lignes dans 6 fichiers au départ.

### Texte du plan, recopié (`docs/plan-3.md` à `9ca66f4`, ligne 2.11r)

> - 2.11r — `registres`, selon l'étude 2.10 (§3) : une catégorie neuve du compteur pour les conteneurs qui tiennent des entités par `void *` hors des types d'entité, puis `hv`, `cv`, `cg`, `ui.ic` et `new_entity` typés. La catégorie (V1) lit, hors `tests/` et `pico8_ptrs.[ch]`, les membres `pico8_ptrs_t` (tableaux compris) des structs qui ne sont pas des types d'entité, les objets `pico8_ptrs_t` de portée fichier, les prototypes de `.h` qui prennent ou rendent un `pico8_ptrs_t`, et les membres de struct qui rendent `void *` ; exemptions par nom, chacune prouvée par son écrivain (texte à l'annexe E de l'étude). Elle entre dans `CATEGORIES` et `SORTIE` (Q1). Le typage ne dépend que de 2.3 et 2.8 : il passe avant 2.12, et 2.14 trouve `hv` et `cv` typés. Preuves : 2.11r-a, fixture rouge puis verte et deux mutations du script ; 2.11r-b1 à -c, refactor pur (objets sans `-g` identiques quand seul un type change), sinon chemin touché ; garde prouvée mordante à 2.11r-c. `class-coverage-net` : oui à 2.11r-c seulement. `sanitize-net` : non. Budget ≈ 5 × 50 min.
>   - 2.11r-a — la catégorie, sa fixture, l'attendu changé de `test_void_ptr_reads_joined_public_prototypes`. Compte : `registres` 0 → **11 / 6** ; pied 1073 / 85 → **1084 / 87**.
>   - 2.11r-b1 — `hv` (`entity.c:32`, `entity.h:24`) en `pico8_ents_t`, lecteurs `bootstrap.c:52`, `wormhole.c:31-42`, TU (dont les aides `listed`/`rank_in`), R2 `pico8_bm_has`. Compte : → **9**.
>   - 2.11r-b2 — `cv` (`entity.c:18`, `entity.h:30`), `ui.ic.layers` (`ui.h:24`), couches locales de `dc`, lecteurs `economy.c:115`, `:252`, `entity.c:184-216`, `ui.c:268`, `:298`, `:411`, TU, R2 `pico8_dc`. Compte : → **6**.
>   - 2.11r-b3 — `cg` (`entity.c:27`, `entity.h:36`), jumelles `pico8_jp_ents`/`pico8_la_ents` et prédicats typés, `pico8_ij` retiré (`entity_geom.h:31`), `pico8_jp_ptrs`/`pico8_la_ptrs` exemptés (restent sur `jy`), TU, R2 `pico8_eg_gg_pred`, `pico8_ke_ks`. Compte : → **1**.
>   - 2.11r-c — `new_entity` (`struct.h:170`) rend `pico8_struct_header_t *`, avec ses neuf cibles ; les 21 autres `static void *pico8_X_struct_new` rendent leur type. Compte : `registres` → **0**, gardé.

### Réserve du vérificateur de 2.10, traitée en 2.11r-a

La Sortie de phase exige que **tout** registre d'entités soit compté. Le vérificateur a injecté onze formes de registre que le texte de l'annexe E ne voit pas ; aucune n'existe aujourd'hui dans `src/` (grep, run 3). Mesuré sur la copie, une forme à la fois dans `wormhole.c`/`wormhole.h` (`forms.sh`, run 4) :

| # | Forme injectée | Annexe E | Catégorie élargie (ce PRD) |
|---|---|---|---|
| F1 | `static pico8_ptrs_t reg;` dans une fonction | +0 | **+1** |
| F2 | `typedef pico8_ptrs_t reg_t;` (`.h`) puis `static reg_t g;` (`.c`) | +0 | **+1** (la ligne `static reg_t g;`) |
| F3a | `static struct {` / `pico8_ptrs_t list;` / `} g;` (anonyme) | +0 | **+1** |
| F3b | idem, `static struct reg_s {` (nommée) | +0 | **+1** |
| F4 | membre `pico8_ptrs_t *a, *b;` | +0 | **+1** |
| F5 | membre `pico8_ptrs_t **pp;` | +0 | **+1** |
| F6 | membre `const pico8_ptrs_t *const x;` | +0 | **+1** |
| F7 | accesseur `static inline const pico8_ptrs_t *reg_get(void) { return 0; }` dans un `.h` | +0 sur une ligne ; +1 sur trois lignes, **par accident** (`index(acc, ";")` vaut 0 quand le `;` manque) | **+1** sous les deux formes |
| F8 | `static void *g_void[64];` | +0 | **+1** |
| F9 | membre `void **items;` | +0 | **+1** |
| F10 | prototype `void **pico8_reg_items(void);` | +0 | **+1** |

Six formes voisines, vues aussi par la catégorie élargie (+1 chacune, `forms2.sh`, run 5) : membre pointeur de fonction qui nomme `pico8_ptrs_t` (`pico8_ptrs_t (*get)(void);`), membre `void *slots[8];`, `extern pico8_ptrs_t g_reg;` dans un `.h`, objet struct sur une ligne (`static struct { pico8_ptrs_t l; } g;`), objet à déclarateurs multiples (`static pico8_ptrs_t g_a, g_b;`), objet non statique (`pico8_ptrs_t g_pub;`), et un `static struct {…} g;` **déclaré dans une fonction**. L'annexe E voyait les trois dernières des six, pas les trois premières.

**Limites restantes**, mesurées à +0 et écrites dans la fixture comme lignes que la catégorie ne doit pas compter (une limite qui se mettrait à compter change l'attendu) : alias d'alias (`typedef reg_t reg2_t;`), `void *` derrière un `typedef` (`typedef void *handle_t; static handle_t g[64];`), membre d'une struct imbriquée, membre `void *` simple (`void *owner;` : `gen.h:39` `r_ctx` et `map_tables.h:46` `ref` en sont, qui ne tiennent pas d'entités), locale non statique, déclaration produite par une macro, définition de fonction d'un `.c`, et tout ce qui est sous `tests/` ou dans `pico8_ptrs.[ch]` (hors champ par construction, comme V1).

**Départ inchangé : `registres` 11 / 6, pied 1084 / 87**, les mêmes onze lignes que l'étude (run 6). Les douze autres catégories, en `--check`, sont identiques octet pour octet au script de `9ca66f4` ; `--detail` ne diffère que par les lignes `registres` et le pied ; `--champs` identique (`282 ; 15 ; 27`). Aucun écart à expliquer.

### État de départ attendu

| Commande | Résultat sur `9ca66f4` |
|---|---|
| `make composition-residue` | `parente 10 3`, `lien-parent 111 27`, `accesseurs-q6 237 33`, `défauts 669 52`, `void-ptr 8 4`, `inclusion-mutuelle 17 16` ; **`résidu 2 : 1073 ligne(s), 85 fichier(s)`** |
| `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` | `870:…:= noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité` |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 282 ; 0 écriture : 15 ; 0 hors constructeur : 27` |
| `make test` ; `make e2e` | vert ; 26 conformes |

Un départ autre : arrêt, rapporté. Si 2.11 (étude) ou un autre commit a touché un fichier de code entre-temps, chercher chaque site par son texte, cité ici.

## Solution

Cinq commits, dans l'ordre du plan, chacun vert (`make test`, `make e2e` 26 conformes) — **les cinq ont été écrits, compilés et joués séparément sur la copie** :

- **2.11r-a** : la catégorie `registres` dans `tools/composition_residue.sh` (texte exact en annexe A : V1 de l'étude, plus l'élargissement), `CATEGORIES` et `SORTIE` ; deux tests dans `tests/test_composition_residue.c` (celui de l'annexe E de l'étude, et un test neuf des formes cachées et des limites, annexe B) ; l'attendu de `test_void_ptr_reads_joined_public_prototypes` passe à `registres 2 1`, pied `3 ligne(s), 1 fichier(s)`. Aucun code du jeu.
- **2.11r-b1** : `g_hv` et `pico8_hv()` en `pico8_ents_t` ; ses deux lecteurs de `src/` ; 29 TU.
- **2.11r-b2** : les seaux `cv`, `ui.ic.layers`, les couches locales de `pico8_dc` en `pico8_ents_t` ; lecteurs ; 26 TU.
- **2.11r-b3** : les seaux `cg` en `pico8_ents_t` ; `pico8_jp_ents` (dans le module `ents`) et ses cinq prédicats typés ; `pico8_ents_cat` pour le `la` de `jc:ce` ; `pico8_ij` retiré (sa jumelle `pico8_ij_ents` sert `cg`) ; `pico8_jp_ptrs`/`pico8_la_ptrs` exemptés ; includes devenus morts retirés ; 24 TU d'entités, plus `composition_residue` et `ents`.
- **2.11r-c** : `new_entity` rend `pico8_struct_header_t *` ; les 29 `static void *pico8_X_struct_new` rendent leur type ; les huit sortes de planète reçoivent une enveloppe `pico8_X_kind_new` qui rend l'en-tête, comme `pico8_ga_kind_new` ; `registres` entre dans la garde.

**Pourquoi rien ne change.** `pico8_ents_t` a la disposition de `pico8_ptrs_t` et les mêmes sémantiques (2.8) ; chaque appel garde son ordre ; seuls les types des locaux changent, et une descente que C faisait en silence depuis `void *` devient un cast écrit (après l'`assert` de classe quand il existe). Mesuré sur copie, à chacun des cinq commits : `make test` vert, 26 e2e conformes, aucune constante `*_HASH`/`FROZEN_*` changée, aucun `*.sha256` ni `tests/e2e/` touché ; à 2.11r-c, `class-coverage-net` OK et `sanitize-net` 26 propres.

## User Stories

1. En tant que gardien de la Sortie de phase, je veux que tout registre d'entités déclaré n'importe où dans `src/` soit compté, y compris sous les formes que le vérificateur a injectées, afin que « résidu 2 : 0 » veuille dire qu'il n'en reste aucun.
2. En tant que lecteur du compteur, je veux la liste des formes que `registres` ne voit pas, écrite dans sa fixture, afin de savoir ce qu'un zéro ne prouve pas.
3. En tant que gardien de la perte acceptée de D.20b, je veux qu'un non-entité ajouté à `hv`, `cv`, `cg` ou `ui.ic` soit une erreur de compilation.
4. En tant qu'auteur de 2.14 et 2.15, je veux trouver `hv`, `cv` et `cg` typés, afin que leurs boucles ne soient pas réécrites deux fois et que chaque descente soit un cast visible.
5. En tant que gardien du jeu, je veux `make e2e` à 26 conformes et aucun `*.sha256` touché, à chaque commit.
6. En tant que gardien des empreintes, je veux chaque `*_HASH`/`FROZEN_*` de TU inchangé : les jumelles de TU plient la même valeur.
7. En tant que vérificateur, je veux `L`, les mutations rejouées au parent, les objets identiques quand seul un type change, et la garde mordante.
8. En tant que lecteur de `--detail`, je veux que les lignes R2 de `pico8_bm_has`, `pico8_dc`, `pico8_eg_gg_pred` et `pico8_ke_ks` disent leur amont réel après chaque sous-étape.
9. En tant que lecteur du glossaire, je veux savoir quels registres sont typés, et que `ij` ne renvoie plus à une fonction supprimée.
10. En tant que lecteur du plan, je veux une note « Découvert à 2.11r-x » courte par sous-étape.

## Implementation Decisions

### Modules

`tools/composition_residue.sh` ; `entity`, `bootstrap`, `wormhole`, `economy`, `ui`, `ents`, `bg`, `economy_formulas`, `economy_score`, `economy_tiers`, `entity_geom`, `galaxy`, `probe`, `score_panel`, `slipway`, `struct`, et les 21 modules qui ont un `pico8_X_struct_new` ; TU `composition_residue`, `ents` et les TU d'entités listés par sous-étape ; `Makefile` (garde) ; `tools/composition_r2_sites.txt` ; `docs/glossaire-noms.md` ; `docs/plan-3.md` (notes « Découvert à »).

### 2.11r-a — la catégorie

`tools/composition_residue.sh`, texte exact en **annexe A** (diff contre `9ca66f4`, 210 lignes). Ce qu'il ajoute à l'annexe E de l'étude :

1. **`REG_TYPES`** : `pico8_ptrs_t` et ses alias `typedef pico8_ptrs_t X;` lus dans les fichiers hors `tests/` et `pico8_ptrs.[ch]` (`$work/regfiles`, calculé une fois après `find`, avant tout lecteur). Les deux lecteurs cherchent le **jeton** `REG_TYPES`, pas un motif de déclarateur : tableau, `*`, `**`, `const`, `*const`, plusieurs noms, pointeur de fonction y tombent tous.
2. **Lecteur de structs** (`read_structs`) : un corps commence aussi à `static struct … {`, `extern struct … {` et, dans une fonction, `static struct … {` (profondeur de base `base`, membres à `base + 1`) ; un corps ouvert et fermé sur la même ligne est clos aussitôt et compté s'il nomme `REG_TYPES`. Ces objets struct (`sobj`) n'entrent **ni** dans la fermeture des types d'entité, **ni** dans `parente`, `ptrs-entité` ou `--champs` : les autres catégories ne changent pas. Membres comptés hors types d'entité : toute ligne qui nomme `REG_TYPES` ; `void **` et `void *nom[N]` ; pointeur de fonction qui rend `void *` (ou `void **`). Exemption par `type.membre`, déclarateur par déclarateur (une ligne à plusieurs noms n'est exemptée que si tous le sont).
3. **Lecteur de déclarations** (`read_registres`) : une déclaration commence à la profondeur 0 (comme V1) **ou** à toute ligne `static` d'un corps de fonction. Comptés : objet (fichier, statique de fonction, `extern` de `.h`) qui nomme `REG_TYPES` ou de type `void *` ; prototype **ou définition inline** de `.h` qui nomme `REG_TYPES` ou rend `void **`. Une définition de fonction d'un `.c` ne compte pas (pas un point public), une `typedef` non plus.
4. `sort -u` à la fusion des deux sorties : un objet struct sur une ligne est vu par les deux lecteurs.
5. `REGISTRES_NON_ENTITE` : les cinq noms de l'étude (`pico8_emitter_t.qy`, `pico8_jo_gh_ctx_t.ji`, `pico8_nv_ptrs`, `pico8_gu_content_ptrs`, `pico8_jo_ex`), chacun avec son écrivain en commentaire.

`tests/test_composition_residue.c` : `test_registres_reads_declaration_points_outside_entity_types` (texte de l'annexe E de l'étude, inchangé) et `test_registres_sees_hidden_forms_and_not_its_named_limits` (**annexe B**), appelés dans `main` après `test_ptrs_members_count_only_on_entity_types` ; `test_void_ptr_reads_joined_public_prototypes` : attendu `"accesseurs-q6\t1\t1\n" "registres\t2\t1\n" "résidu 2 : 3 ligne(s), 1 fichier(s)\n"`.

Compte : `registres` **11 / 6** (les onze lignes de l'étude : `collect.h:12`, `:16`, `entity.c:18`, `:27`, `:32`, `entity.h:24`, `:30`, `:36`, `entity_geom.h:31`, `struct.h:170`, `ui.h:24`) ; pied **1084 / 87**. `registres` n'entre **pas** encore dans la garde.

### 2.11r-b1 — `hv`

- `src/pico8_entity.h` : `#include "pico8_ents.h"` (avant `pico8_ptrs.h`) ; `:24` `const pico8_ents_t *pico8_hv(void);`.
- `src/pico8_entity.c` : `:32` `static pico8_ents_t g_hv;` ; `:55` `pico8_ents_free(&g_hv)`, `:78` `pico8_ents_init`, `:90` `const pico8_ents_t *pico8_hv(void)`, `:141` `pico8_ents_add(&g_hv, e)`, `:166` `pico8_ents_del(&g_hv, e)`.
- `src/pico8_bootstrap.c:52` : `const pico8_ents_t *hv = pico8_hv();` (la ligne `:57` `pico8_struct_header_t *qc = hv->items[i];` devient typée sans changer).
- `src/pico8_wormhole.c` (`pico8_mz_pq`) : `:30-31` `pico8_ents_iter_t it; pico8_struct_header_t *p;` ; `:42-44` `it = pico8_ents_all(pico8_hv()); while (pico8_ents_iter_next(&it, &p)) {` (le local `void *e` et la copie `p = e` partent ; la condition `get_fx` est inchangée).
- TU (29) : `bg fq fr fv g_ ga hj hp hy ie iv jc je jo js mh mo mx mz nf nl nr os pj pm qb qh trade ui` — voir « Aides de TU ». Casts de descente écrits : `pj` (`de_button`, `(pico8_js_t *)hv->items[i]`), `pm` (`fold_spawned`, `(const pico8_ga_t *)hv->items[at - 1]`), `fv` (`(const pico8_js_t *)hv->items[rank - 4 + k]`) ; locaux `const pico8_ptrs_t *hv;` → `pico8_ents_t` dans `fv`, `g_` (×2), `ui`.
- `tools/composition_r2_sites.txt`, ligne `pico8_bm_has`, colonne amont → `aucun : hv est typé (2.11r-b1)`.
- Compte : `registres` **9 / 6** ; pied **1082 / 87**.

### 2.11r-b2 — `cv`, `ui.ic`, couches de `dc`

- `src/pico8_entity.h:30` : `const pico8_ents_t *pico8_cv(const char *name);`.
- `src/pico8_entity.c` : `:18` `pico8_ents_t list;` ; `:58`, `:82`, `:94`, `:145`, `:170` en `pico8_ents_*` ; `pico8_dc` : `:184` `const pico8_ents_t *bucket_src;`, `:185` `pico8_ents_t layers[15];`, `:192`, `:211`, `:239` en `pico8_ents_*`, `:216` `pico8_ents_t *bucket`.
- `src/pico8_economy.c` : `pico8_hy_hk` `:115-117` `const pico8_ents_t *hk_list; pico8_ents_iter_t it; pico8_struct_header_t *qc;`, boucle `:144-151` sur `pico8_ents_all`/`iter_next(&it, &qc)` (la copie `qc = e` part) ; `pico8_hy_lj` `:252-258` de même (`hb_list`, `it2`, `qc`).
- `src/pico8_ui.h` : `:24` `pico8_ents_t layers[PICO8_UI_IC_LAYERS];` ; `:11` `#include "pico8_ptrs.h"` retiré (plus aucun usage ; `pico8_ents.h` y est déjà).
- `src/pico8_ui.c` : `:108`, `:277-278` `pico8_ents_init`/`free` ; `:268` `const pico8_ents_t *hd_bucket;` ; `:298` `pico8_ents_add(&self->ic->layers[fa], e);` ; `:411` `const pico8_ents_t *bucket`.
- TU (26) : `bg fq fr fv g_ ga hj ie iv jc je jo js mh mx mz nf nl nr os pj planet_kj_dispatch pm qb qh ui`. Couches d'`ic` : `ents_listed(&ic->layers[…])` (`bg`, `fq`, `fv`, `ui`) ; `planet_kj_dispatch:75` `bucket->items[i] == (pico8_struct_header_t *)planet` ; `os:230` `pico8_js_ldown((pico8_js_t *)hd->items[i])` ; locaux `hd` en `pico8_ents_t` (`js` ×4, `os`).
- `tools/composition_r2_sites.txt`, ligne `pico8_dc` → `aucun : cv est typé (2.11r-b2)`.
- Compte : `registres` **6 / 5** ; pied **1079 / 87**.

### 2.11r-b3 — `cg`, `jp`/`la`, `ij`

- `src/pico8_entity.h:36`, `src/pico8_entity.c:27`, `:63`, `:133`, `:152`, `:178` : `cg` en `pico8_ents_t`.
- `src/pico8_ents.h` / `.c` : après `pico8_ents_cat`, `typedef int (*pico8_jp_ents_predicate_fn)(pico8_struct_header_t *e, void *ctx);` et `pico8_ents_t pico8_jp_ents(const pico8_ents_t *qk, pico8_jp_ents_predicate_fn ow, void *ctx);` (commentaire : « As pico8_jp_ptrs: a fresh list of the elements `ow` keeps, in order; the caller frees its `items`. A NULL `qk` gives an empty list. ») ; corps : celui de `pico8_jp_ptrs` (`collect.c:9-23`) recopié sur `pico8_ents_*`. **Pas dans `collect`** : essayé, `collect.c` y gagne une dépendance à `ents.c` et le lien de `test_pico8_layout_nil_item_draws_nothing` (`Makefile:564`, liste réduite) casse.
- `tests/test_pico8_ents.c` : `test_jp_keeps_what_the_predicate_keeps_in_order` (cinq cellules, prédicat « rang impair » par arithmétique sur le tableau passé en `ctx`, attendu `cells[1]`, `cells[3]`, source intacte ; `qk` NULL → vide), appelé dans `main`. Mesuré : vert, et rouge quand `pico8_jp_ents` garde tout.
- Lecteurs `cg` : `bg.c:175` `const pico8_ents_t *g_tag;` ; `economy.c:192-194` (`pico8_hy_lj`) `const pico8_ents_t *jo_list; pico8_ents_iter_t it; pico8_struct_header_t *p;`, boucle `:216-220` sans copie `p = e` ; `for_each_cg` identique dans `economy_formulas.c:49-61`, `economy_score.c:24-36`, `economy_tiers.c:22-34` (`const pico8_ents_t *list; pico8_ents_iter_t it; pico8_struct_header_t *e;`) ; `economy_formulas.c:144`, `economy_score.c:51` en `const pico8_ents_t *` ; `wormhole.c:136` `const pico8_ents_t *jo_list;`, `:150` `pico8_struct_header_t *pt_val`, `:157` `pt = (pico8_jo_t *)pt_val;` (après l'`assert` « mz:pr: cg.jo holds planets only »).
- Listes filtrées : `economy_score.c` `cz_predicate(pico8_struct_header_t *e, void *ctx)`, `:74-78` `pico8_ents_t filtered = pico8_jp_ents(jo_list, cz_predicate, NULL)`, `pico8_ents_free` ; `probe.c` `pico8_bv_jc_predicate(pico8_struct_header_t *e, …)` avec `assert(e->cls == pico8_jc_struct_class() && "cg.jc holds slipways only"); sw = (const pico8_jc_t *)e;`, `:173-183` `pico8_ents_t filtered`, `pico8_jp_ents`, `pico8_ents_free` ; `score_panel.c` `pico8_eg_gg_pred(pico8_struct_header_t *e, …)` `return pico8_jo_gg((const pico8_jo_t *)e);`, `:121-127` idem ; `slipway.c` `pico8_jc_ce_np_predicate(pico8_struct_header_t *e, …)` (le local `n` part, `pico8_jc_end_p(e)`), `pico8_jc_ce_jc_predicate` comme `bv`, `:289-310` `pico8_ents_t part1, part2, ds`, `pico8_jp_ents` ×2, **`ds = pico8_ents_cat(&part1, &part2);`**, `pico8_ents_free` ×3.
- `pico8_ij` : `entity_geom.c:101-115` retiré (`ij_step` reste pour `pico8_ij_ents`) ; `entity_geom.h:31-32` (prototype et commentaire « As pico8_ij, on an entity list. ») retirés ; `galaxy.c:67`, `probe.c:165` → `pico8_ij_ents`.
- Includes devenus morts, retirés (mesuré : 0 erreur, `inclusion-mutuelle` 17 / 16 inchangée) : `pico8_ptrs.h` de `entity.h:4`, `entity.c:9`, `entity_geom.h:6` ; `pico8_collect.h` de `economy_score.c`, `probe.c`, `slipway.c`. `score_panel.c` garde le sien (2 erreurs sans).
- `tools/composition_residue.sh` : `REGISTRES_NON_ENTITE` += `pico8_jp_ptrs pico8_la_ptrs`, commentaire « pico8_jp_ptrs et pico8_la_ptrs sur des enregistrements jy (planet_display.c:186, planet_trade.c:128), les listes d'entites passant par pico8_jp_ents et pico8_ents_cat. » (ce sont leurs deux seuls appelants, grep de Sortie 3).
- `tests/test_composition_residue.c` : l'attendu de `test_void_ptr_reads_joined_public_prototypes` passe à `"registres\t1\t1\n"`, `"résidu 2 : 2 ligne(s), 1 fichier(s)\n"` (son `pico8_jp_ptrs` est désormais exempté).
- TU (24 d'entités) : `bg fr fv g_ ga hj hp hy jc je jo js lq mh mx mz nl nr os pj pm qh trade ui`, plus `composition_residue` et `ents` ; casts : `bg:411`, `hp:931`, `:935` `(pico8_jo_t *)pico8_cg("jo")->items[0]`, `ui:379` `return (pico8_jo_t *)jo->items[i];` ; locaux `const pico8_ptrs_t *X;` assignés d'un `pico8_cg(…)` en `pico8_ents_t` (`hp` ×2, `hy`, `jc` ×2, `jo` ×2, `js` ×2, `lq`, `mz` ×5, `trade`, `ui`).
- `tools/composition_r2_sites.txt` : ligne `pico8_eg_gg_pred` → amont `aucun : prédicat typé de pico8_jp_ents sur cg (2.11r-b3)` ; ligne `pico8_ke_ks` : retirer « `void *pt_val = jo_list->items[i]` (wormhole.c:153) » de l'amont (typé), garder `pico8_d__dispatch(void *self)`.
- Compte : `registres` **1 / 1** (`struct.h:170`) ; pied **1074 / 85** (`collect.h` et `entity_geom.h` quittent le pied).

### 2.11r-c — `new_entity` et les constructeurs

- `src/pico8_struct.h:170` : `pico8_struct_header_t *(*new_entity)(const pico8_jo_keys_t *keys);`.
- Les 29 `static void *pico8_X_struct_new` rendent leur type (`static pico8_bg_t *pico8_bg_struct_new(void)`, …) : `bg.c:66`, `economy.c:22`, `help.c:146`, `layout.c:331`, `:418`, `mouse_reader.c:43`, `msgbox.c:106`, `:254`, `planet.c:178`, `planet_family.c:84`, `:151`, `:210`, `:310`, `:391`, `:565`, `planet_family_trade.c:204`, `:263`, `:369`, `probe.c:471`, `research.c:131`, `research_icon.c:236`, `score_panel.c:288`, `scroll.c:44`, `slipway.c:395`, `trade.c:26`, `ui.c:93`, `ui_button.c:106`, `:352`, `wormhole.c:239` (le type est celui du `self` que chacun alloue ; l'étude comptait 9 + 21 = 30 en y mettant `pico8_ga_kind_new`, qui n'est pas un `_struct_new`).
- Huit enveloppes, chacune juste avant la table de sa sorte (`planet_family.c:636`, `:654`, `:690`, `:708`, `:726`, `planet_family_trade.c:424`, `:442`, `:460`) : `static pico8_struct_header_t *pico8_X_kind_new(const pico8_jo_keys_t *keys) { return &pico8_X_struct_new(keys)->CHEMIN; }`, `CHEMIN` = `jo.header` (`qh`), `qh.jo.header` (`nl`, `os`, `hj`, `nr`), `hj.qh.jo.header` (`pm`, `mx`), `nr.qh.jo.header` (`fr`) ; la table pointe l'enveloppe. `pico8_ga_kind_new` (`planet_family.c:745`) rend `pico8_struct_header_t *` et `return &pico8_ga_struct_new(…)->jo.header;` (deux `return`). Aucun `_struct_new` de ces sortes ne rend NULL (lu).
- Les lecteurs `planet.c:401` (cast `(pico8_jo_t *)` gardé) et `research_icon.c:147` (`out.held`, déjà `pico8_struct_header_t *`) ne changent pas.
- `Makefile:870` : `COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres`.
- `docs/glossaire-noms.md` (voir plus bas).
- Compte : `registres` **0**, disparue de la sortie, gardée ; `void-ptr` 8 / 4 inchangé ; pied **1073 / 85** ; `--champs` 282 / 15 / 27 inchangé.

### Aides de TU : `listed`, `rank_in` et les autres

Les TU plient les registres par des aides locales `static int listed(const pico8_ptrs_t *list, const void *e)`, `rank_in(…)` (46 définitions), plus `list_count`, `count_of`. À 2.11r-b1, **111 des 128 erreurs** de compilation viennent de `listed` (62) et `rank_in` (49) ; l'étude en comptait 292 sur 375 pour les trois registres ensemble. Comme en 2.8 (G6), chaque TU reçoit des **jumelles `ents_<aide>`** au corps recopié mot pour mot, et chaque appel sur le registre typé passe à la jumelle ; une aide `pico8_ptrs_t` qui n'a plus d'appelant est **retirée** (sinon `-Werror=unused-function`, que `-fsyntax-only` ne voit pas : premier `make test` rouge sur `test_pico8_mo.c`, run 12). Mesuré : b1, 37 jumelles (`ents_listed` 20, `ents_rank_in` 17), 3 aides retirées ; b2, 1 jumelle (`ents_list_count`), 6 retirées ; b3, 5 jumelles (`ents_count_of` 2, `ents_list_count`, `ents_listed` 2), 41 retirées. Fin : 283 lignes d'appel `ents_listed`/`ents_rank_in` ; les aides `pico8_ptrs_t` qui restent servent `jy`, `qy` et autres listes non-entités.

Le passage est mécanique : `twins.py <hv|cv|cg>` (annexe C) renomme les appels `aide(pico8_REG(…)` et les locaux `const pico8_ptrs_t *x = pico8_REG(…)`, insère les jumelles et retire les aides mortes ; il reste ensuite 7 (b1), 10 (b2), 23 (b3) erreurs, toutes listées ci-dessus (locaux assignés plus loin, casts de descente, couches d'`ic`). L'implémenteur peut le rejouer ou éditer à la main ; le résultat attendu est celui décrit.

### Glossaire (`docs/glossaire-noms.md`, numéros de `9ca66f4`)

- `:658` (« Le conteneur de pointeurs ») : retirer des registres qu'il tient `hv` (b1), `cv`, `ui.ic`, calques de `dc` (b2), `cg` (b3) ; à la fin il ne tient plus que `emitter->qy`, `jc.ts`, `hy.invented`, `jo.jy`, `hp.i`.
- `:659` (« Le conteneur d'entités ») : ajouter, à la même sous-étape, `hv`, `cv`, `ui.ic`, les calques de `dc`, `cg` ; à b3, `pico8_jp_ents` dans la liste des opérations (`add`/`del`/`has`/`cat`/`jp`/`all`).
- `:109` (`ij`) : « `pico8_ij_ents` (sur `cg` et `mz.ox`) ; plus de version `pico8_ptrs_t` » (b3).
- `:287` (`jp`) : « `pico8_jp_ptrs` (enregistrements `jy`) ; `pico8_jp_ents` sur les listes d'entités ; écrit au site sur une table statique (`cb`, `bj`) » (b3).
- `:16` (`kv`) : « … le `new_entity` d'une sorte prend un `pico8_jo_keys_t` et rend l'en-tête (`pico8_X_kind_new`) » (c).

### Anti-tassement

- **Interdit** :
  - faire échapper un registre à la catégorie sans typer ses éléments : `typedef` de `pico8_ptrs_t`, `void *` derrière un `typedef`, struct imbriquée, macro, déplacement dans `tests/` ou `pico8_ptrs.[ch]`, ou toute autre limite de l'annexe B ;
  - ajouter à `REGISTRES_NON_ENTITE` autre chose que `pico8_jp_ptrs` et `pico8_la_ptrs` (b3) ;
  - changer l'attendu des lignes « limites » de la fixture neuve pour la faire passer (une limite qui compte est une amélioration, à justifier dans le message de commit, jamais une régression silencieuse) ;
  - un cast `(pico8_ents_t *)` d'un `pico8_ptrs_t`, ou un argument `(void *)…` passé à `pico8_ents_add`/`del`/`has` dans `src/` ;
  - changer un message d'`assert` (« cg.jc holds slipways only », « hy:lj: cg.jo holds planets only », « mz:pr: cg.jo holds planets only », « gg: pt is a planet of cg.jo », « ui.ic: a hovered entity's fa outside 0..15… », « dc: only kj and ev are drawn through dc ») ;
  - toucher `tests/e2e/`, un `*.sha256` ou une constante `*_HASH`/`FROZEN_*` ;
  - retirer `registres` de la garde, ou l'y mettre avant 2.11r-c ;
  - garder `pico8_ij`, ou créer une `pico8_la_ents` à côté de `pico8_ents_cat` (même sémantique, 2.8).
- Les jumelles `ents_<aide>` ne sont pas du tassement (le corps est recopié, la valeur pliée est la même).

## Implementation Chunks

Cinq commits, ≈ 50 min chacun preuves comprises, dans l'ordre : **a** (script, deux tests, attendu, mutations du script, note) ; **b1** ; **b2** ; **b3** (y compris le TU `ents`) ; **c** (garde, `class-coverage-net`, `sanitize-net` recommandé). L'essai sans PRD a pris ≈ 2 h pour les cinq ; les textes ci-dessus et l'annexe C en retirent la recherche.

**Repli écrit (si une sous-étape déborde à 1,5 fois son budget avant le commit)** :
- **a** en deux : **a1** la V1 de l'annexe E seule avec son test (11 / 6, pied 1084 / 87) ; **a2** l'élargissement et le test des formes cachées (comptes inchangés). a2 doit précéder b1.
- **b1**, **b2** : pas de découpe utile (le stockage et ses lecteurs compilent ensemble) ; au-delà du budget, rapporter.
- **b3** en deux : **b3a** `pico8_jp_ents` et son TU seuls (aucun compte ne bouge) ; **b3b** le reste (comptes de b3).
- **c** en deux : **c1** les 21 constructeurs hors sortes de planète (aucun compte) ; **c2** `new_entity`, enveloppes, garde.

Un repli est écrit dans la note « Découvert à ». Les comptes d'arrivée ne changent pas.

## Testing Decisions

- **Un test neuf de primitive** (`test_pico8_ents.c`, b3) ; deux tests neufs du compteur (a). Aucun rejeu d'`events.json` dans un TU ; les e2e partent de l'écran titre (`make e2e`). Les empreintes des TU d'entités plient déjà rangs et appartenances aux registres : elles prouvent l'identité de comportement, et les mutations montrent qu'elles la voient.
- **Prior art** : 2.8 (jumelles, `L`, règle du parent, garde mordante), annexe E de l'étude 2.10.

### Preuves, à chaque commit sauf mention

1. `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. Mesuré aux cinq.
2. **2.11r-a** : fixture **rouge** avec le script du parent (`test_registres_reads_declaration_points_outside_entity_types`, code 2 « categorie inconnue », run 8) et avec le script de l'annexe E (`test_registres_sees_hidden_forms_and_not_its_named_limits`), **verte** avec le script neuf. Les douze autres catégories identiques au parent en `--check` (octet pour octet), `--detail` identique hors `registres` et pied, `--champs` identique. Mutations du script (le parent n'a pas la catégorie : pas de rejeu au parent), chacune rougit la fixture ; mesuré :

| # | Mutation du script | Premier test rouge | `registres` sur `src/` |
|---|---|---|---|
| S1 | `pico8_emitter_t.qy` retiré de `REGISTRES_NON_ENTITE` | `…reads_declaration_points…` | 12 (`particle.h:43` compté) |
| S2 | membre : `c !~ /\[/` ajouté (tableaux ignorés) | `…reads_declaration_points…` | 10 (`ui.h:24` sort) |
| S3 | pas de début de déclaration sur `static` dans une fonction | `…hidden_forms…` | 11 |
| S4 | début de corps sans `static`/`extern` (objet struct) | `…hidden_forms…` | 11 |
| S5 | `REG_TYPES` sans alias | `…hidden_forms…` | 11 |
| S6 | une ligne à plusieurs déclarateurs jamais comptée | `…hidden_forms…` | 11 |
| S7 | prototype qui rend `void **` non compté | `…hidden_forms…` | 11 |
| S8 | définition inline de `.h` non comptée | `…hidden_forms…` | 11 |
| S9 | jeton `REG_TYPES` ancré en tête (pas de `const`) | `…hidden_forms…` | 11 |

3. **2.11r-b1 à -c, objets** : `.c` de `src/` dont seul un type change, compilés sans `-g` au parent et à l'enfant (`objdump -d`, `nm`) — **identiques** mesurés : b1 `bootstrap.c` ; b3 `bg.c`, `wormhole.c` ; c `bg.c`, `economy.c`, `help.c`, `layout.c`, `mouse_reader.c`, `msgbox.c`, `planet.c`, `probe.c`, `research.c`, `research_icon.c`, `score_panel.c`, `scroll.c`, `slipway.c`, `trade.c`, `ui.c`, `ui_button.c`, `wormhole.c`. Les autres changent d'appel (`pico8_ptrs_*` → `pico8_ents_*`, enveloppes) : chemin touché.
4. **Chemin touché, `gcov` des e2e** (`make line-coverage EVENTS="$(ls tests/e2e/*.json | tr '\n' ' ')"`, dans une copie, parent puis enfant, appariement `difflib` des sources). Mesuré : **6311 → 6319 → 6316 → 6314 → 6330** (a → b1 → b2 → b3 → c). **Aucune ligne inchangée perdue**, aux quatre. `L` (couvertes au parent, retirées ou réécrites) : b1 **9** (`bootstrap.c` 1, `entity.c` 5, `wormhole.c` 3) ; b2 **23** (`economy.c` 9, `entity.c` 9, `ui.c` 5) ; b3 **57** (`economy.c` 4, `economy_formulas.c` 4, `economy_score.c` 8, `economy_tiers.c` 3, `entity.c` 4, `entity_geom.c` 9, `galaxy.c` 1, `probe.c` 6, `score_panel.c` 4, `slipway.c` 12, `wormhole.c` 2) ; c **31** (les 29 signatures et les deux `return` de `pico8_ga_kind_new`, `planet_family.c` 8, `planet_family_trade.c` 3, un par autre module). Lignes neuves non couvertes par les e2e : des déclarations seulement, plus **une** ligne exécutable, `return &pico8_ga_struct_new(NULL)->jo.header;` (`pico8_ga_kind_new` sans clés), déjà non couverte au parent. Les huit enveloppes `pico8_X_kind_new` sont couvertes. Toute autre différence : arrêt, rapporté.
5. **Mutations**, une à la fois, à l'enfant puis la même au parent sur l'ancienne forme (`make test` puis `make e2e`). **ÉCHEC si verte à l'enfant et rouge au parent.** Mesuré sur la copie, même résultat aux deux arbres à chaque ligne :

| # | Sous-étape | Mutation (enfant ; parent sur `pico8_ptrs_*`) | Premier TU rouge | e2e conformes enfant / parent |
|---|---|---|---|---|
| M1 | b1 | `lt` n'ajoute plus à `hv` | `iv` | 0 / 0 |
| M2 | b1 | `he` ne retire plus de `hv` | `iv` | 11 / 11 |
| M3 | b1 | `mz:pq` sans filtre `get_fx` (`if (p != NULL)`) | `iv` (`test_mz_scan_skips_a_live_toast`) | 8 / 8 |
| M4 | b1 | `dg` saute la dernière entité (`i < hv->count - 1`) | `iv` | 1 / 1 |
| M5 | b2 | `dc` range en `layers[15 - layer]` | `planet_kj_dispatch` ; parent `bg` (ordre de `make -j`) | 0 / 0 |
| M6 | b2 | `gw` ne range plus le survolé dans `ic` | `fq` (`test_hover_loop_on_the_veil`) | 0 / 0 |
| M7 | b2 | `hy:lj` n'appelle plus `hk` | `hy` (`test_hk`) | 15 / 15 |
| M8 | b2 | `he` ne retire plus de `cv` | `iv` | 4 / 4 |
| M9 | b3 | `pico8_jp_ents` garde tout ; parent `pico8_jp_ptrs` | `mz` (`test_hq`) | 0 / 0 |
| M10 | b3 | `jc:ce` : `cat(&part2, &part1)` | **aucun, vert aux deux** | 26 / 26 |
| M10b | b3 | `jc:ce` : `cat(&part2, &part2)` (sans `np`) | aucun TU ; e2e rouge | 25 / 25 |
| M11 | b3 | `ij_ents` ne visite rien (`if (0) ij_step(…)`) ; parent dans `pico8_ij` | `mz` | 0 / 0 |
| M12 | b3 | `he` ne retire plus de `cg` | `mh` | 24 / 24 |
| M13 | b3 | `cz_predicate` rend toujours 1 | `hj` | 0 / 0 |
| M14 | c | l'enveloppe de `nl` construit un `qh` ; parent `g_nl_kind = {pico8_qh_struct_new, …}` | `jo` (`test_daughters`) | 25 / 25 |
| M15 | c | `pico8_ga_kind_new` ignore les clés (`(pico8_jo_keys_t){0}`) | `jo` (`fold_daughter`) | 25 / 25 |

   M10 est un trou **déclaré**, le même au parent : l'ordre de `la` dans `jc:ce` n'est observé ni par un TU ni par un e2e (seul `ds.items[0]` est lu, et aucun cas ne rend à la fois un `np` et un `jc` croisés). Pièges mesurés : une mutation qui laisse un paramètre ou une fonction inutilisés (`(void)o;` dans `ij_ents`, table de `nl` sur `pico8_qh_kind_new`) casse la compilation sous `-Werror` et rend 0 conforme ; ce n'est pas un rouge de test (refaites en `if (0) …` et dans le corps de l'enveloppe).
6. **Preuve compilateur négative** (b1, sur copie, recommandée) : `pico8_ents_add` d'un pointeur non-entité ne compile pas (déjà mesuré en 2.8a) ; un `pico8_ptrs_t *` passé à `ents_listed` non plus (les 128, 113, 149 erreurs des runs 11, 13, 15 en sont la preuve).
7. **Garde mordante** (c, sur copie) : `static struct {` / `pico8_ptrs_t reg;` / `} g_mordant;` ajouté à `wormhole.c` → `make composition-guard` : `registres: src/pico8_wormhole.c:301:    pico8_ptrs_t reg;`, `résidu gardé (…,registres) : 1 ligne(s)`, `Error 1` ; idem avec `new_entity` remis en `void *(*…)` (`struct.h:170`). `test:` dépend de `composition-guard` : `make test` rougit. Mesuré.
8. `make class-coverage-net` (c, exigé) : **OK**, « 26 rejeu(x) », 1,8 s. `make sanitize-net` (c, non exigé par le plan, recommandé) : **26 propre(s)**, 3,3 s.
9. `--champs` : **282 / 15 / 27** aux cinq commits ; `inclusion-mutuelle` **17 / 16** aux cinq ; `void-ptr` **8 / 4** aux cinq.

### Comptes par sous-étape (mesurés)

| Commit | `registres` | pied |
|---|---|---|
| parent `9ca66f4` | — | 1073 / 85 |
| 2.11r-a | **11 / 6** | **1084 / 87** |
| 2.11r-b1 | **9 / 6** | 1082 / 87 |
| 2.11r-b2 | **6 / 5** | 1079 / 87 |
| 2.11r-b3 | **1 / 1** | 1074 / 85 |
| 2.11r-c | **0**, gardée | **1073 / 85** |

### Greps de Sortie (sur l'arbre de 2.11r-c)

1. `grep -rnE '\bpico8_ptrs_t\b' src/pico8_entity.[ch] src/pico8_ui.h src/pico8_entity_geom.h` → **vide**.
2. `grep -nE '^static void \*pico8_[a-z0-9_]+_(struct|kind)_new' src/*.c` → **vide** ; `grep -c 'kind_new' src/pico8_planet_family.c src/pico8_planet_family_trade.c` → `12`, `6`.
3. `grep -rn 'pico8_jp_ptrs(\|pico8_la_ptrs(' src | grep -v 'src/pico8_collect\.'` → **deux lignes**, `planet_display.c:186`, `planet_trade.c:128`.
4. `grep -rnw 'pico8_ij' src tests docs/glossaire-noms.md` → **vide** (la ligne `:109` du glossaire ne nomme plus que `pico8_ij_ents`).
5. `grep -rnE '(^|[^_a-z])(listed|rank_in|list_count|count_of)\(pico8_(hv|cv|cg)\(' tests` → **vide** (seules les jumelles `ents_` lisent les registres).
6. `grep -rn '(pico8_ents_t \*)' src` → **vide** ; `grep -rnE 'pico8_ents_(add|has|del)\([^,]*, \(void' src tests` → **vide**.
7. `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` → `…,ptrs-entité,registres`.
8. `sh tools/composition_residue.sh --check registres` → `résidu gardé (registres) : 0 ligne(s)`, code 0 ; `make composition-residue` → pas de ligne `registres`, `void-ptr 8 4`, `inclusion-mutuelle 17 16`, pied **1073 / 85**.

### Budget et tableau « Runs »

Budget : **5 × ≈ 50 min**. Au-delà de 1,5 fois, repli, sans rien commettre d'incomplet. Jamais l'appli raylib ; `make raylib` n'est pas exigé (aucune liste de sources ne change).

L'implémenteur tient un tableau « Runs », une ligne par lancement. Voici celui des essais de ce PRD, sur une copie de `9ca66f4` (commits `a`, `b1`, `b2`, `b3`, `c` de la copie) :

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone` local, `git checkout 9ca66f4` ; `sh tools/composition_residue.sh` ; garde | parent | 0 | pied 1073 / 85 ; `Makefile:870` finit par `ptrs-entité` | 0,8 s |
| 2 | `essai_registres.py` (annexe E de l'étude) ; compteur ; `--check registres` | parent + E | 0 ; 1 | `registres 11 6` ; pied 1084 / 87 ; les onze lignes de l'étude | < 1 s |
| 3 | greps des onze formes dans `src/` (`void **`, objets `void *`, `typedef pico8_ptrs_t`, statiques de fonction, `static struct`, `static inline`) | parent | 0 | aucune forme présente ; un `static struct {` (`research.c:19`) sans `pico8_ptrs_t` | < 1 s |
| 4 | `forms.sh` : onze formes injectées une à une, script de l'annexe E | parent + E | 0 | +0 ×10 ; F7 +1 sur trois lignes seulement (`index` à 0), +0 sur une ligne | ≈ 10 s |
| 5 | élargissement (annexe A) ; `forms.sh`, `forms2.sh` | `a` | 0 | onze formes +1 ; six voisines +1 ; limites +0 | ≈ 15 s |
| 6 | compteur ; `--check` des douze autres catégories, `--detail`, `--champs` comparés au script du parent | `a` | 0 | `registres 11 6`, 1084 / 87 ; `cmp` identiques ; `--detail` : pied seul | 2 s |
| 7 | fixture neuve (annexe B) ; `make tmp/test_composition_residue` ; TU | `a` | 0 | `all tests passed` | 3 s |
| 8 | TU avec le script du parent, puis avec celui de l'annexe E | `a` | 134 ; 134 | `…reads_declaration_points…` ; `…hidden_forms…` | < 1 s |
| 9 | mutations S1 à S9 du script | `a` | 134 ×9 | tableau S | ≈ 10 s |
| 10 | `timeout 900 make test` | `a` | 0 | vert | 8,4 s |
| 11 | b1 : stockage `hv` ; compilation de tout l'arbre | `b1` en cours | 1 | 128 erreurs : 2 `src/`, 126 dans 29 TU (111 `listed`/`rank_in`) | 2 s |
| 12 | `twins.py hv` ; corrections ; `make test` (premier essai) | `b1` | 2 | `test_pico8_mo.c:36` : `listed` défini non utilisé (`-fsyntax-only` ne le voyait pas) ; aides mortes retirées | 6 s |
| 13 | `make test` ; `make e2e` ; compteur | `b1` | 0 ; 0 | vert ; 26 ; `registres 9 6` | 6 s ; 1 s |
| 14 | b2 : stockage ; compilation ; `twins.py cv` ; corrections ; `make test` ; `make e2e` | `b2` | 1 puis 0 | 113 → 10 → 1 → 0 erreurs ; vert ; 26 ; `registres 6 5` | 8 s |
| 15 | b3 : stockage, lecteurs, `jp_ents` dans `collect` ; `twins.py cg` ; `make test` | `b3` en cours | 2 | 149 → 23 → 0 erreurs ; lien de `test_pico8_layout_nil_item_draws_nothing` cassé (`collect.o` → `ents`) | 4 s |
| 16 | `pico8_jp_ents` déplacé dans `ents` ; `make test` | `b3` | 2 | `test_void_ptr_reads_joined_public_prototypes` : attendu `registres 2` | 9 s |
| 17 | attendu corrigé ; includes morts (`collect.h` ×4 essayés, 3 retirés) ; `make test` ; `make e2e` | `b3` | 0 ; 0 | vert ; 26 ; `registres 1 1`, 1074 / 85 | 7 s |
| 18 | c : `_struct_new` retypés ; compilation | `c` en cours | 1 | 18 erreurs (8 sortes + publics) → enveloppes → 0 | 2 s |
| 19 | garde ; `make test` ; `make e2e` ; `class-coverage-net` ; `sanitize-net` | `c` | 0 ×5 | vert ; 26 ; `couverture: OK` ; `26 propre(s)` | 8,5 s ; 1 s ; 1,8 s ; 3,3 s |
| 20 | `g_mordant` puis `new_entity` en `void *` réinjectés ; `make composition-guard` | `c` | 2 ; 2 | `registres … 1 ligne(s)`, `Error 1` | < 1 s |
| 21 | compteur et `--champs` à chaque commit | tous | 0 | table des comptes ; 282 / 15 / 27 ; 17 / 16 | 5 s |
| 22 | `objcmp.sh` sur les quatre paires | tous | 0 | table des objets identiques | 10 s |
| 23 | `make line-coverage` sur les 26 e2e, cinq arbres ; `lset.py` | tous | 0 | 6311 → 6319 → 6316 → 6314 → 6330 ; `L` 9 / 23 / 57 / 31 ; 0 perdue | ≈ 5 s × 5 |
| 24 | mutations M1 à M15 à l'enfant et au parent (`mut.sh`) | tous | 2 sauf M10 | tableau M ; M11 et M14 d'abord cassés à la compilation, refaits | ≈ 15 s × 34 |
| 25 | TU `ents` : `test_jp_keeps…` ; puis `pico8_jp_ents` qui garde tout | `b3` | 0 ; 134 | `test_pico8_ents: ok` ; assertion rouge | 3 s |
| 26 | `ptrs.h` retiré de `entity.h`, `entity.c`, `entity_geom.h` ; compilation ; compteur | `b3` | 0 | 0 erreur ; `inclusion-mutuelle 17 16` | 2 s |
| 27 | `git diff 9ca66f4` des essais : `_HASH`/`FROZEN_`, `tests/e2e`, `*.sha256`, `float_guard.txt` | `c` | 0 | vides | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas pied 1073 / 85 avec la garde terminée par `ptrs-entité`, sans explication.
- `FAILED:` si, à l'un des cinq commits, `make test` ou `make e2e` (26 conformes) est rouge, si un `*.sha256` ou `tests/e2e/` change, ou si une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si, à 2.11r-a, une des onze formes F1 à F10 (F3 en deux) n'est pas comptée par la fixture, si une des douze autres catégories ou `--champs` change, ou si la fixture n'est pas rouge avec le script du parent.
- `FAILED:` si une mutation S1 à S9 laisse la fixture verte.
- `FAILED:` si une mutation M1 à M15 est **verte à l'enfant et rouge au parent** (règle du parent) ; M10, verte aux deux, est déclarée.
- `FAILED:` si une ligne inchangée entre dans `L`, ou si une ligne exécutable neuve non couverte par les e2e n'est pas le `return` de `pico8_ga_kind_new` sans clés.
- `FAILED:` si un `.c` listé « identique » au point 3 ne l'est pas, sans explication.
- `FAILED:` si les comptes d'arrivée ne sont pas ceux de la table (11 / 6, 9, 6, 1, 0 ; pied 1084 / 87 → 1073 / 85), sans explication.
- `FAILED:` si `registres` n'est pas dans la garde à 2.11r-c, ou y entre avant ; si la garde réinjectée ne rougit pas `make test` ; si `class-coverage-net` n'est pas OK à 2.11r-c.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` en cas de tassement (voir « Anti-tassement »).
- `FAILED:` si un message de commit omet : les lignes `--check registres` du parent retirées par ce commit, `L`, les mutations avec leur résultat au parent, les objets identiques, la garde mordante (c), le tableau « Runs ».
- `FAILED:` s'il manque une note « Découvert à 2.11r-a/b1/b2/b3/c » dans `docs/plan-3.md`, ou si le plan change ailleurs que sur la ligne 2.11r.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Les **locales** qui tiennent des entités hors des registres typés : elles suivent le type de ce qu'elles lisent (V1).
- Les listes non-entités restées `pico8_ptrs_t` (`jy`, `ji`, `jx`, `is`, `i`, `qy`, `ts`, `invented`) et leurs aides (`pico8_nv_ptrs`, `pico8_gu_content_ptrs`, `pico8_jo_ex`, `pico8_jp_ptrs`, `pico8_la_ptrs`).
- Les `void *` des méthodes de table (`void-ptr`, 8 / 4) : 2.14 à 2.16. `pico8_d__dispatch(void *self)`, amont de `pico8_ke_ks`, reste.
- Les limites de l'annexe B : les fermer demanderait de suivre les `typedef` en chaîne ou d'étendre les macros ; aucune n'existe dans `src/`.
- Renommer les jumelles `ents_listed`… en `listed` quand l'aide `pico8_ptrs_t` a disparu : non exigé.
- L'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Catégorie élargie par jeton, pas par déclarateur.** Chercher `pico8_ptrs_t` (ou un alias) comme mot dans une ligne de membre ou de déclaration voit d'un coup tableaux, `**`, `const`, déclarateurs multiples et pointeurs de fonction ; le motif de déclarateur de l'annexe E en manquait quatre. Même compte au départ (11 / 6).
- **G2. `void *` compté comme objet et `void **` comme membre ou type de retour, pas `void *` simple comme membre.** Deux membres `void *` de `src/` (`gen.h:39`, `map_tables.h:46`) ne tiennent pas d'entités ; les compter demanderait deux exemptions sans gain.
- **G3. `pico8_jp_ents` dans le module `ents`**, pas dans `collect` (lien cassé, run 15) ; **`pico8_ents_cat`** pour le `la` de `jc:ce`, pas une `pico8_la_ents` neuve (même sémantique, 2.8). Le plan nommait `pico8_jp_ents`/`pico8_la_ents` : écart de nom, pas de fond.
- **G4. Enveloppes `pico8_X_kind_new` typées pour les huit sortes de planète**, sur le modèle de `pico8_ga_kind_new` : tous les `_struct_new` rendent leur type, aucun cast de l'en-tête vers le type dans les `pico8_X_new` publics.
- **G5. Jumelles de TU plutôt qu'un retypage des aides** : `listed`/`rank_in` servent aussi `jy` et les autres listes non-entités, et chaque commit doit compiler avec des registres des deux types. Les aides `pico8_ptrs_t` mortes sont retirées au commit qui les rend mortes.
- **G6. Includes morts retirés** à b2 (`ui.h`) et b3 (`entity.h`, `entity.c`, `entity_geom.h`, `economy_score.c`, `probe.c`, `slipway.c`), comme G7 de 2.8.
- **G7. `sanitize-net` recommandé à c seulement** (le plan dit non : la propriété ne change nulle part) ; mesuré propre.

### Surprises de mesure, pour `plan-pilot`

1. **L'annexe E voit l'accesseur `static inline` sur trois lignes par accident** (`index(acc, ";")` vaut 0 quand le `;` manque, et 0 < position de `{`), pas sur une ligne. La catégorie élargie compte toute définition inline de `.h` qui nomme `pico8_ptrs_t`.
2. **Les onze formes du vérificateur n'étaient pas toutes des registres de `pico8_ptrs_t`** : trois sont des conteneurs `void *` (F8 à F10). Les voir a demandé G2.
3. **`-fsyntax-only` ne voit pas `-Wunused-function`** : une aide de TU devenue morte passe la vérification syntaxique et casse `make test`. Vérifier en compilant (`-c -o /dev/null`).
4. **29 `_struct_new` en `void *`, pas 30** : l'étude comptait `pico8_ga_kind_new` avec eux.
5. **M10 (ordre de `la` dans `jc:ce`) n'est vu par aucun test**, au parent comme à l'enfant.
6. **Le pied de b2 garde 87 fichiers** : `ui.h` quitte `registres` mais reste au pied par ses deux lignes `défauts`. Celui de b3 en perd deux (`collect.h`, `entity_geom.h`, qui n'avaient que leur ligne `registres`) ; `entity.h` reste par `void-ptr`.

### Questions pour l'humain

Aucune.

## Annexe A — `tools/composition_residue.sh`, diff de 2.11r-a contre `9ca66f4`

```diff
diff --git a/tools/composition_residue.sh b/tools/composition_residue.sh
index 63b4d73..d5b03d7 100644
--- a/tools/composition_residue.sh
+++ b/tools/composition_residue.sh
@@ -38,10 +38,10 @@
 
 set -u
 
-CATEGORIES="parente lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux void-ptr ptrs-entité noyau-inclut atteinte-à-prouver inclusion-mutuelle"
+CATEGORIES="parente lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux void-ptr ptrs-entité registres noyau-inclut atteinte-à-prouver inclusion-mutuelle"
 
 # Categories comptees dans le pied.
-SORTIE="parente lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux void-ptr ptrs-entité noyau-inclut atteinte-à-prouver inclusion-mutuelle"
+SORTIE="parente lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux void-ptr ptrs-entité registres noyau-inclut atteinte-à-prouver inclusion-mutuelle"
 
 fail() {
     echo "composition_residue: $1" >&2
@@ -84,6 +84,22 @@ VOID_EXCLUSION='s/void \*+(r_)?ctx\b//g'
 # enregistrements jy (research.c:233), jc.ts des particules (slipway_display.c:43).
 PTRS_NON_ENTITE="pico8_hy_t.invented pico8_jo_t.jy pico8_hp_t.i pico8_jc_t.ts"
 
+# registres : hors tests/ et pico8_ptrs.[ch], les points de declaration qui
+# tiennent des entites par void * hors des types d'entite (que ptrs-entite
+# lit). Membre d'une struct (ou d'un objet struct, meme dans une fonction) qui
+# nomme pico8_ptrs_t ou un alias, quel que soit son declarateur (tableau,
+# **, const, plusieurs noms, pointeur de fonction), ou de type void ** ou
+# void *[N] ; membre qui rend void * par pointeur de fonction ; objet de portee
+# fichier, statique de fonction, extern de .h, de ces types ou void * ;
+# prototype ou definition inline de .h qui prend ou rend un pico8_ptrs_t, ou
+# rend void **. Non vus : alias d'alias, void * derriere un typedef, struct
+# imbriquee, membre void * simple, locale, macro. Exemptes, chacun prouve par
+# son ecrivain : emitter.qy des particules (particle.c:65), le ji du contexte
+# de planet_display.c des enregistrements jy (:79, :186) ; pico8_nv_ptrs sur
+# hp.i (planet.c:319), pico8_gu_content_ptrs sur des chaines (tech_gate.c:26),
+# pico8_jo_ex sur des enregistrements jy (planet_display.c:54).
+REGISTRES_NON_ENTITE="pico8_emitter_t.qy pico8_jo_gh_ctx_t.ji pico8_nv_ptrs pico8_gu_content_ptrs pico8_jo_ex"
+
 # En-tetes du noyau, par nom de fichier : noyau-inclut ne lit qu'eux.
 NOYAU="pico8_struct.h pico8_ke.h pico8_entity.h"
 
@@ -140,6 +156,19 @@ while IFS= read -r file; do
     done
 done < "$work/files"
 
+# registres lit hors tests/ et pico8_ptrs.[ch] ; REG_TYPES est pico8_ptrs_t
+# et ses alias "typedef pico8_ptrs_t X;" de ces fichiers (un alias d'alias
+# n'est pas suivi).
+grep -v '/tests/' "$work/files" | grep -v '^tests/' | grep -v '/pico8_ptrs\.[ch]$' | grep -v '^pico8_ptrs\.[ch]$' > "$work/regfiles" || true
+REG_TYPES=pico8_ptrs_t
+if [ -s "$work/regfiles" ]; then
+    grep -hE '^[[:space:]]*typedef[[:space:]]+(const[[:space:]]+)?pico8_ptrs_t[[:space:]*]+[A-Za-z_][A-Za-z0-9_]*[[:space:]]*;' $(cat "$work/regfiles") > "$work/reg.alias"
+    [ $? -le 1 ] || fail "echec de grep"
+    for a in $(sed -E 's/^.*[[:space:]*]([A-Za-z_][A-Za-z0-9_]*)[[:space:]]*;.*$/\1/' "$work/reg.alias"); do
+        REG_TYPES="$REG_TYPES|$a"
+    done
+fi
+
 # Retire commentaires, chaines et caracteres d'une ligne, pour compter les
 # accolades ; un commentaire ouvert passe a la ligne suivante (incomment).
 CLEAN_AWK='
@@ -181,23 +210,35 @@ function braces(s,    t, n) {
 # $work/members les "<type>\t<membre>" que lit --champs.
 read_structs() {
     awk -v non_entite=" $PTRS_NON_ENTITE " -v out_parente="$work/parente" \
+        -v reg_non_entite=" $REGISTRES_NON_ENTITE " -v out_reg="$work/registres.structs" -v reg_types="$REG_TYPES" \
         -v out_ptrs="$work/ptrs-entité" -v out_members="$work/members" "$CLEAN_AWK"'
-    FNR == 1 { depth = 0; incomment = 0; inbody = 0 }
+    FNR == 1 { depth = 0; incomment = 0; inbody = 0; base = 0 }
     {
         c = clean($0)
         if (!inbody && depth == 0 && c ~ /^typedef[ \t]+struct[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]*;/) {
             split(c, w, /[ \t;]+/)
             alias_n++; alias_a[alias_n] = w[3]; alias_b[alias_n] = w[4]
         }
-        if (!inbody && depth == 0 && c ~ /^(typedef[ \t]+)?struct([ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*\{/) {
-            n++; inbody = 1; seen = 0; names[n] = " "
-            sfile[n] = FILENAME
-            t = c; sub(/^(typedef[ \t]+)?struct[ \t]*/, "", t); sub(/[ \t]*\{.*/, "", t)
+        if (!inbody && (depth == 0 && c ~ /^((static|extern)[ \t]+)?(typedef[ \t]+)?(const[ \t]+)?struct([ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*\{/ ||
+                        depth > 0 && c ~ /^[ \t]*static[ \t]+(const[ \t]+)?struct([ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*\{/)) {
+            n++; inbody = 1; seen = 0; names[n] = " "; base = depth
+            sfile[n] = FILENAME; sobj[n] = c ~ /^[ \t]*(static|extern)[ \t]/
+            t = c; sub(/^[ \t]*((static|extern)[ \t]+)?(typedef[ \t]+)?(const[ \t]+)?struct[ \t]*/, "", t); sub(/[ \t]*\{.*/, "", t)
             if (t != "") names[n] = names[n] t " "
             braces(c)
+            if (depth == base) {
+                inbody = 0
+                if (match(c, /\}[ \t]*[A-Za-z_][A-Za-z0-9_]*/)) {
+                    t = substr(c, RSTART + 1, RLENGTH - 1); sub(/^[ \t]*/, "", t)
+                    names[n] = names[n] t " "
+                }
+                if (c ~ ("(^|[^A-Za-z0-9_])(" reg_types ")([^A-Za-z0-9_]|$)")) {
+                    nr[n]++; rmem[n, nr[n]] = ""; rline[n, nr[n]] = FNR; rtext[n, nr[n]] = $0
+                }
+            }
             next
         }
-        if (inbody && depth == 1 && c !~ /^[ \t]*$/ && c !~ /^[ \t]*\}/) {
+        if (inbody && depth == base + 1 && c !~ /^[ \t]*$/ && c !~ /^[ \t]*\}/) {
             if (c !~ /[({]/ && match(c, /;/)) {
                 t = substr(c, 1, RSTART - 1)
                 nd = split(t, parts, ",")
@@ -219,9 +260,23 @@ read_structs() {
                 t = $0; sub(/;.*/, "", t); sub(/.*[ \t*]/, "", t)
                 np[n]++; pmem[n, np[n]] = t; pline[n, np[n]] = FNR; ptext[n, np[n]] = $0
             }
+            if (c ~ /^[ \t]*(const[ \t]+)?void[ \t]*(\*[ \t]*)+\([ \t]*\*/) {
+                t = c; sub(/\).*/, "", t); sub(/.*[ \t*(]/, "", t)
+                nr[n]++; rmem[n, nr[n]] = t; rline[n, nr[n]] = FNR; rtext[n, nr[n]] = $0
+            } else if (c ~ /;/ &&
+                       (c ~ ("(^|[^A-Za-z0-9_])(" reg_types ")([^A-Za-z0-9_]|$)") ||
+                        c !~ /\(/ && c ~ /^[ \t]*(const[ \t]+)?void[ \t]*\*[ \t]*(const[ \t]+)?(\*|[A-Za-z_][A-Za-z0-9_]*[ \t]*\[)/)) {
+                t = c; sub(/;.*/, "", t); sub(/^[ \t]*(const[ \t]+)?[A-Za-z_][A-Za-z0-9_]*/, "", t)
+                nd = split(t, parts, ","); t = ""
+                for (i = 1; i <= nd; i++) {
+                    d = parts[i]; sub(/\[.*/, "", d); sub(/[ \t]+$/, "", d)
+                    if (match(d, /[A-Za-z_][A-Za-z0-9_]*$/)) t = t substr(d, RSTART, RLENGTH) " "
+                }
+                nr[n]++; rmem[n, nr[n]] = t; rline[n, nr[n]] = FNR; rtext[n, nr[n]] = $0
+            }
         }
         braces(c)
-        if (inbody && depth == 0) {
+        if (inbody && depth == base) {
             inbody = 0
             if (match(c, /\}[ \t]*[A-Za-z_][A-Za-z0-9_]*/)) {
                 t = substr(c, RSTART + 1, RLENGTH - 1); sub(/^[ \t]*/, "", t)
@@ -239,7 +294,7 @@ read_structs() {
         do {
             grown = 0
             for (k = 1; k <= n; k++) {
-                if (!(first[k] in e) || in_e(k)) continue
+                if (sobj[k] || !(first[k] in e) || in_e(k)) continue
                 m = split(names[k], nm, " ")
                 for (i = 1; i <= m; i++) e[nm[i]] = 1
                 grown = 1
@@ -253,11 +308,21 @@ read_structs() {
         printf "" > out_parente
         printf "" > out_ptrs
         printf "" > out_members
+        printf "" > out_reg
         for (i = 1; i <= alias_n; i++) alias_of[alias_a[i]] = alias_b[i]
         for (k = 1; k <= n; k++) {
-            if (first[k] in e && first[k] != "pico8_struct_header_t")
+            if (!sobj[k] && first[k] in e && first[k] != "pico8_struct_header_t")
                 print sfile[k] ":" firstline[k] ":" firsttext[k] > out_parente
-            if (!in_e(k)) continue
+            if (!in_e(k) && sfile[k] !~ /(^|\/)tests\// && sfile[k] !~ /(^|\/)pico8_ptrs\.[ch]$/) {
+                m = split(names[k], nm, " ")
+                for (j = 1; j <= nr[k]; j++) {
+                    q = split(rmem[k, j], mm, " "); left = q
+                    for (h = 1; h <= q; h++)
+                        for (i = 1; i <= m; i++) if (index(reg_non_entite, " " nm[i] "." mm[h] " ")) { left--; break }
+                    if (left > 0 || q == 0) print sfile[k] ":" rline[k, j] ":" rtext[k, j] > out_reg
+                }
+            }
+            if (!in_e(k) || sobj[k]) continue
             if (sfile[k] ~ /\.h$/ && sfile[k] !~ /(^|\/)tests\//) {
                 m = split(names[k], nm, " "); type = nm[m]
                 if (type in alias_of) type = alias_of[type]
@@ -312,6 +377,41 @@ read_prototypes() {
     }' > "$work/void-ptr" || fail "echec de awk"
 }
 
+# registres, suite : objets pico8_ptrs_t de portee fichier des .c et
+# prototypes des .h (hors pico8_ptrs.h), hors tests/, joints comme ceux de
+# void-ptr ; un nom de REGISTRES_NON_ENTITE est exempte.
+read_registres() {
+    [ -f "$work/parente" ] || read_structs
+    : > "$work/registres.decl"
+    if [ -s "$work/regfiles" ]; then
+        awk -v non=" $REGISTRES_NON_ENTITE " -v reg_types="$REG_TYPES" "$CLEAN_AWK"'
+        FNR == 1 { depth = 0; incomment = 0; acc = "" }
+        {
+            c = clean($0)
+            if (acc == "" && ((depth == 0 && $0 ~ /^[a-z]/) || (depth > 0 && c ~ /^[ \t]*static[ \t]/))) {
+                start = FNR; text = $0; acc = " "; local = depth > 0
+            }
+            if (acc != "") {
+                acc = acc " " c
+                if (acc ~ /[;{=]/) {
+                    fn = !local && match(acc, /[A-Za-z_][A-Za-z0-9_]*[ \t]*\(/)
+                    if (fn) { name = substr(acc, RSTART, RLENGTH); sub(/[ \t]*\($/, "", name); ret = substr(acc, 1, RSTART - 1) }
+                    else { t = acc; sub(/[ \t]*(\[[^]]*\])?[ \t]*[;=].*/, "", t); sub(/.*[ \t*]/, "", t); name = t }
+                    ok = acc !~ /^[ \t]*typedef/ &&
+                         (acc ~ ("(^|[^A-Za-z0-9_])(" reg_types ")([^A-Za-z0-9_]|$)") ||
+                          (!fn && acc ~ /(^|[^A-Za-z0-9_])void[ \t]*\*/) ||
+                          (fn && ret ~ /(^|[^A-Za-z0-9_])void[ \t]*\*[ \t]*(const[ \t]+)?\*/))
+                    keep = FILENAME ~ /\.h$/ || !fn
+                    if (ok && keep && !index(non, " " name " ")) print FILENAME ":" start ":" text
+                    acc = ""
+                }
+            }
+            braces(c)
+        }' $(cat "$work/regfiles") > "$work/registres.decl" || fail "echec de awk"
+    fi
+    cat "$work/registres.structs" "$work/registres.decl" | sort -u -t: -k1,1 -k2,2n > "$work/registres"
+}
+
 # Paires {a, b} de modules hors tests/ ou chacun inclut l'en-tete de l'autre,
 # dans $work/inclusion-mutuelle.pairs ; dans $work/inclusion-mutuelle, les
 # lignes #include qui realisent ces paires, au format des autres categories.
@@ -356,6 +456,7 @@ collect() {
             sort -t: -k1,1 -k2,2n "$work/$1" > "$work/sorted" && mv "$work/sorted" "$work/$1"
             return 0 ;;
         void-ptr) read_prototypes; return 0 ;;
+        registres) read_registres; return 0 ;;
         inclusion-mutuelle) read_inclusions; return 0 ;;
         atteinte-à-prouver)
             status=0
```

À 2.11r-b3, `REGISTRES_NON_ENTITE` et son commentaire changent comme écrit plus haut.

## Annexe B — `test_registres_sees_hidden_forms_and_not_its_named_limits`

Lignes vues par la catégorie : `f.c` 2 (F1), 5 (statique struct de fonction), 9 (F2), 12 (F3a), 15 (F3b), 17, 18 (F8), 19, 20 ; `f.h` 4 (F4), 5 (F5), 6 (F6), 7 (F9), 8, 9, 15 (F7), 16 (F10), 17, 18. Lignes laissées, une par limite : `f.h` 1-2 (les alias eux-mêmes, et l'alias d'alias), 10 (membre `void *` simple), 12 (struct imbriquée) ; `f.c` 3 (locale), 10 (objet par alias d'alias), 21-22 (`void *` derrière un `typedef`), 23-24 (macro), 25 (définition d'un `.c`) ; `tests/t.c` (hors champ).

```c
/* Formes d'un registre que le lecteur voit (F) et limites qu'il ne voit pas
 * (L), chacune a sa ligne : une limite qui se met a compter change l'attendu. */
static void test_registres_sees_hidden_forms_and_not_its_named_limits(void) {
    setup();
    write_file(SRC "/f.h", "typedef pico8_" "ptrs_t reg_t;\n"
                           "typedef reg_t reg2_t;\n"
                           "typedef struct {\n"
                           "    pico8_" "ptrs_t *a, *b;\n"
                           "    pico8_" "ptrs_t **pp;\n"
                           "    const pico8_" "ptrs_t *const x;\n"
                           "    void **items;\n"
                           "    pico8_" "ptrs_t (*get)(void);\n"
                           "    void *slots[8];\n"
                           "    void *owner;\n"
                           "    struct {\n"
                           "        pico8_" "ptrs_t l;\n"
                           "    } inner;\n"
                           "} holder_t;\n"
                           "static inline const pico8_" "ptrs_t *reg_get(void) { return 0; }\n"
                           "void **pico8_" "reg_items(void);\n"
                           "extern pico8_" "ptrs_t g_reg;\n"
                           "typedef struct { pico8_" "ptrs_t l; } one_t;\n");
    write_file(SRC "/f.c", "static void reg_f(void) {\n"
                           "    static pico8_" "ptrs_t reg;\n"
                           "    pico8_" "ptrs_t l;\n"
                           "    static struct {\n"
                           "        pico8_" "ptrs_t l;\n"
                           "    } g;\n"
                           "    (void)reg;\n"
                           "}\n"
                           "static reg_t g_alias;\n"
                           "static reg2_t g_alias2;\n"
                           "static struct {\n"
                           "    pico8_" "ptrs_t list;\n"
                           "} g_anon;\n"
                           "static struct reg_s {\n"
                           "    pico8_" "ptrs_t list;\n"
                           "} g_named;\n"
                           "static struct { pico8_" "ptrs_t l; } g_one;\n"
                           "static void *g_void[64];\n"
                           "static pico8_" "ptrs_t g_a, g_b;\n"
                           "pico8_" "ptrs_t g_pub;\n"
                           "typedef void *handle_t;\n"
                           "static handle_t g_h[64];\n"
                           "#define REG(n) pico8_" "ptrs_t n\n"
                           "static REG(g_m);\n"
                           "int reg_n(const pico8_" "ptrs_t *p) {\n"
                           "    return p->count;\n"
                           "}\n");
    write_file(TST "/t.c", "static void *g_t[4];\n");
    assert(run_residue("--check registres " TREE) == 1);
    assert(strcmp(read_file(OUT), "registres: " SRC "/f.c:2:    static pico8_" "ptrs_t reg;\n"
                                  "registres: " SRC "/f.c:5:        pico8_" "ptrs_t l;\n"
                                  "registres: " SRC "/f.c:9:static reg_t g_alias;\n"
                                  "registres: " SRC "/f.c:12:    pico8_" "ptrs_t list;\n"
                                  "registres: " SRC "/f.c:15:    pico8_" "ptrs_t list;\n"
                                  "registres: " SRC "/f.c:17:static struct { pico8_" "ptrs_t l; } g_one;\n"
                                  "registres: " SRC "/f.c:18:static void *g_void[64];\n"
                                  "registres: " SRC "/f.c:19:static pico8_" "ptrs_t g_a, g_b;\n"
                                  "registres: " SRC "/f.c:20:pico8_" "ptrs_t g_pub;\n"
                                  "registres: " SRC "/f.h:4:    pico8_" "ptrs_t *a, *b;\n"
                                  "registres: " SRC "/f.h:5:    pico8_" "ptrs_t **pp;\n"
                                  "registres: " SRC "/f.h:6:    const pico8_" "ptrs_t *const x;\n"
                                  "registres: " SRC "/f.h:7:    void **items;\n"
                                  "registres: " SRC "/f.h:8:    pico8_" "ptrs_t (*get)(void);\n"
                                  "registres: " SRC "/f.h:9:    void *slots[8];\n"
                                  "registres: " SRC "/f.h:15:static inline const pico8_" "ptrs_t *reg_get(void) { return 0; }\n"
                                  "registres: " SRC "/f.h:16:void **pico8_" "reg_items(void);\n"
                                  "registres: " SRC "/f.h:17:extern pico8_" "ptrs_t g_reg;\n"
                                  "registres: " SRC "/f.h:18:typedef struct { pico8_" "ptrs_t l; } one_t;\n"
                                  "résidu gardé (registres) : 19 ligne(s)\n") == 0);
}
```

## Annexe C — `twins.py`, le passage mécanique des TU

Lancé depuis la racine, `python3 twins.py hv` (b1), `cv` (b2), `cg` (b3), puis corrections à la main listées dans chaque sous-étape.

```python
# usage: twins.py REG [files...] : appels d'aides de TU sur pico8_REG(...) -> ents_<aide>, locaux types,
# jumelles ents_<aide> inserees apres l'aide d'origine (corps recopie).
import re, sys, glob
reg = sys.argv[1]
files = sys.argv[2:] or sorted(glob.glob('tests/*.c'))
HELP_RE = re.compile(r'^static [a-z_0-9 ]+?\b([a-z_0-9]+)\(const pico8_ptrs_t \*[a-z_]+[,)]', re.M)
def block(s, start):
    i = s.index('{', start); d = 0
    for j in range(i, len(s)):
        if s[j] == '{': d += 1
        elif s[j] == '}':
            d -= 1
            if d == 0: return s[start:j+1]
for f in files:
    s = open(f).read(); o = s
    helpers = HELP_RE.findall(s)
    if not helpers: helpers_set = set()
    else: helpers_set = set(helpers)
    for h in helpers_set:
        s = re.sub(r'\b(?<!ents_)' + h + r'\((?=\(?(const )?(pico8_ptrs_t \*\))?pico8_' + reg + r'\()', 'ents_' + h + '(', s)
    s = re.sub(r'const pico8_ptrs_t \*([a-z_0-9]+) = pico8_' + reg + r'\(', r'const pico8_ents_t *\1 = pico8_' + reg + '(', s)
    # twins
    changed = True
    while changed:
        changed = False
        for m in HELP_RE.finditer(s):
            h = m.group(1)
            if ('ents_' + h + '(') in s and not re.search(r'^static [a-z_0-9 ]+?\bents_' + h + r'\(', s, re.M):
                b = block(s, m.start())
                t = b.replace('pico8_ptrs_t', 'pico8_ents_t')
                for h2 in helpers_set:
                    t = re.sub(r'\b(?<!ents_)' + h2 + r'\(', 'ents_' + h2 + '(', t)
                end = m.start() + len(b)
                s = s[:end] + '\n\n' + t + s[end:]
                changed = True
                break
    if s != o:
        open(f, 'w').write(s); print('edited', f)

# aides d'origine devenues inutilisees : retirees (sinon -Werror=unused-function)
for f in files:
    s = open(f).read(); o = s
    for m in list(HELP_RE.finditer(s))[::-1]:
        h = m.group(1)
        uses = len(re.findall(r'(?<![a-z_0-9])' + h + r'\(', s))
        if uses == 1:
            b = block(s, m.start())
            end = m.start() + len(b)
            s = s[:m.start()] + s[end:].lstrip('\n')
    if s != o:
        open(f, 'w').write(s); print('pruned', f)
```
