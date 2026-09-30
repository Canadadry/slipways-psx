---
title: "Plan 3, étape 2.11r-e : réserves du vérificateur de 2.11r-d — mutations V1, V4, V6 fixées par la fixture ; qualificatifs, deux déclarations sur une ligne, déclarateur à la ligne suivante et attribut à deux niveaux comptés — un commit, comptes inchangés"
description: "Le vérificateur de 2.11r-d (bab1416) a trouvé trois mutations du script qui survivent à la fixture et six formes ni comptées ni écrites comme limites. Toutes les formes sont comptées ; les limites restantes sont écrites dans le commentaire du script et dans la fixture. Aucune forme n'existe dans src/ : tous les comptes restent (1073 / 85)."
status: needs-triage
---

## Problem Statement

Mesuré sur `bab1416` avec `forms.sh` (scratchpad `impl211re/`, une ligne par forme dans un mini-arbre ; `e` membre d'un type d'entité `pico8_jo_t`, `r` membre d'une autre struct, `f` objet de `.c`, `h` prototype de `.h` ; résultat `registres/ptrs-entité`).

1. **Mutations qui survivent** (réserve du vérificateur) :
   - V1 — exemption par ligne dès qu'un déclarateur est exempté, au lieu de déclarateur par déclarateur. La fixture n'a aucune ligne qui mêle un nom exempté et un autre. `e: pico8_ptrs_t *jy, *other_jy;` compte 0/1 (bon) ; sous V1, 0/0.
   - V4 — retrait de `|| q == 0` (ligne sans nom de déclarateur lisible). `registres` a son témoin (`pico8_ptrs_t (*get)(void);`), pas `ptrs-entité`. `e: pico8_ptrs_t (*mk)(void);` compte 0/1 ; sous V4, 0/0.
   - V6 — `_Alignas[ \t]*\(` réduit à `_Alignas\(`. La fixture n'écrit que `_Alignas(8)`. `e: _Alignas (8) void **al;` compte 0/1 ; sous V6, 0/0.
2. **Formes ni comptées ni écrites comme limites**, toutes à 0/0 aujourd'hui :
   - Q1-Q3 : `void const **x;`, `volatile void **x;`, `void * volatile *x;` en membre (`e` et `r`) ; de même `static void const *g[4];` (`f`), `void const **f(void);` et `void *volatile *f(void);` (`h`). Seul `const` avant `void` ou juste après une `*` est lu.
   - D1-D2 : deux déclarations sur une ligne, `int k2; void **items;` (le motif est ancré au début de ligne) et `pico8_ptrs_t *jy; pico8_ptrs_t *hidden;` dans `pico8_jo_t` (seul le premier `;` est lu, `jy` exempté cache `hidden`).
   - N1-N3 : déclarateur à la ligne suivante (`void **` puis `items;`, `pico8_ptrs_t` puis `*hidden;`) : la première ligne n'a pas de `;`.
   - A1-A2 : attribut à deux niveaux, `void **items __attribute__((aligned(sizeof(int))));`, `_Alignas(sizeof(int)) void **items;` : `unattr` ne retire qu'un niveau de parenthèses internes (aucun pour `_Alignas`).
3. **Commentaires** : « aucun conteneur non typé n'échappe aux deux » (au-dessus de `test_ptrs_entite_reads_entity_members_like_registres`) est faux (membre `void *` simple, limites) ; une ligne du commentaire `REGISTRES_NON_ENTITE` dépasse 80 colonnes.

## Solution

Tout est compté, sans faux positif sur les formes voisines essayées :

- **Qualificatifs** : dans la lecture de membre, `(const|volatile)` répétables avant et après `void`, et après chaque `*` ; dans la lecture de fichier (`read_registres`), `void` suivi de qualificatifs avant `*` pour les objets, et qualificatifs entre les `*` du type rendu d'un prototype.
- **Plusieurs déclarations sur une ligne** : la lecture de membre découpe la ligne sur `;` et lit chaque déclaration ; une ligne compte une fois, et reste exemptée seulement si tous ses déclarateurs le sont.
- **Déclarateur à la ligne suivante** : une ligne de membre qui ne finit pas par `;` (hors `#`, `{`, `}`) est jointe aux suivantes jusqu'au `;` ; la déclaration compte à sa première ligne, comme `void-ptr`.
- **Attribut à deux niveaux** : `unattr` retire `__attribute__((…))` avec deux niveaux de parenthèses internes, et `_Alignas(…)` avec un niveau.

Limites écrites (commentaire du script, fixture à 0) : attribut à trois niveaux (`aligned(sizeof(sizeof(int)))`), deux déclarations de portée fichier sur une ligne (le nom lu pour l'exemption est le premier).

## Mutations

Chacune rougit la fixture neuve (code 134) : V1, V4, V6 ; et le retrait de chaque ajout : qualificatifs de membre, qualificatifs de fichier (objet), qualificatifs de fichier (type rendu), découpage sur `;`, jonction des lignes, second niveau de `__attribute__`, niveau de `_Alignas`.

## Critères

- 13 `--check`, `--detail`, `--champs` et pied identiques octet pour octet avant/après (`cmp`, 16 sorties) : **1073 / 85**, `champs : 282 ; 15 ; 23`.
- Faux positifs à 0/0 : `const char **names;`, `void *owner;`, `int *(*get)(void);`, `void (*cb)(void *, void **);`, `void *volatile_ptr;`, `void *constant;`, `char *const *names;`, `void (*cb)(void *a,` sur deux lignes, `int k2; void *owner;`, `static const char **g;`, `const char **f(void);`.
- `make test` vert ; `make e2e` 26 conformes. Ni `src/`, ni `Makefile`, ni `*.sha256`, ni `tests/e2e/`.
- Plan 3 : ligne 2.11r-e.
