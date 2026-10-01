---
title: "Plan 3, étape 2.8 : les sept listes membres d'entités passent sur une liste typée `pico8_ents_t` (éléments `pico8_struct_header_t *`) ; `pico8_trade_to` et `pico8_ha_pz` typés, `pico8_gu_ptrs` retiré (`ptrs-entité` 7 → 0, gardé ; `void-ptr` 11 → 8 ; pied 1083/88 → 1073/85 après 2.7c) — trois commits, jeu identique"
description: "`trade.lp`/`rl`, `jo.oe`/`cc`/`by`, `ui.le` et `mz.ox` rangeaient des entités dans un `pico8_ptrs_t` (`void *`) : un non-entité y entrait sans bruit, et chaque lecteur le relisait par `void *` (perte acceptée de D.20b, 2). Un module `pico8_ents` (add/del/has/cat/free/itérateur sur `pico8_struct_header_t *`) les remplace ; les lecteurs reçoivent un en-tête typé et descendent par un cast explicite après l'assert de classe existant ; `ptrs-entité` entre dans la garde. Aucune liste ne se révèle non-entité."
status: needs-triage
---

## Problem Statement

**Écrit sur `8aea055`** (branche `claude/plan-pilot-3-7zdft0`, 2.7 en triage, **pas encore appliquée**). Aucune ligne du dépôt n'a changé pour écrire ce PRD. Les essais ont été faits sur une seule copie jetable de `8aea055` (arbre identique, historique local : `p` = parent, puis une branche par sous-étape), supprimée depuis. **Tous les numéros de ligne ci-dessous sont ceux de `8aea055`, avant 2.7** : 2.7 réécrit des lignes voisines dans plusieurs fichiers de cette étape (voir « Recouvrement avec 2.7 ») ; l'implémenteur retrouve chaque site par son texte, cité ici.

Vu du mainteneur du port : dans le cart, `add(jo.oe, sw)`, `add(ib, a)`, `add(cc, e)`… rangent des tables d'objets dans des tables Lua. Le port les range dans `pico8_ptrs_t` (`ptrs.h:7-11`), un tableau de `void *` sans type. Depuis D.20b, l'en-tête d'entité n'a plus d'étiquette : un pointeur vers autre chose qu'une entité, entré dans une de ces listes, n'est vu ni par un contrôle d'étiquette ni par ASan (`cls` à l'offset 0 est lu **dans** l'objet). Le plan l'a acceptée en nommant le remède (`plan-3.md`, « Perte acceptée à D.20b, 2 ») : « types d'entité aux signatures et ajout typé (`pico8_struct_header_t *`) sur les listes d'entités ». Le compteur la mesure en deux catégories :

- `ptrs-entité` (**7 / 4**) : les membres `pico8_ptrs_t` des types d'entité, hors `PTRS_NON_ENTITE` (`composition_residue.sh:85`) : `trade.h:12` `lp`, `:13` `rl` ; `planet.h:43` `oe`, `:58` `cc`, `:59` `by` ; `ui.h:35` `le` ; `wormhole.h:36` `ox`.
- `void-ptr` (**11 / 7**), dont trois tiennent aux listes : `trade.h:63` (`pico8_trade_to(…, void **out)`), `collect.h:19` (`pico8_gu_ptrs(…, const void *kb)`, appartenance par identité, dont les deux seuls appelants lisent `rl` et `le`), `menu.h:25` (`const void *pico8_ha_pz(void)`, qui rend une entité — `g_ha_pz` a un en-tête, `menu.c:40-52`).

Deux des sites R2 de D.20b tiennent aussi aux listes (`tools/composition_r2_sites.txt`, lus par `--detail`) : `pico8_jc_hq` n'a de `void *` en amont que le `void *w_val` que `pico8_jc_ce` lit dans `jo.oe` (`slipway.c:273`) ; `pico8_bm_has` a pour amont le mapper `pico8_ui_cj_he_mapper(void *e)` (`ui.c:450`) qui parcourt `ui.le`, et les registres `hv`/`cv`.

**Pourquoi rien ne change.** Le type neuf garde la disposition (`items`, `count`, `capacity`), les mêmes sémantiques qu'`add`/`del`/`all` de `pico8_ptrs` (suppression du premier identique, itérateur qui tolère la suppression de l'élément rendu), et les mêmes appels dans le même ordre. Seuls les types des locaux changent ; la descente vers le type concret, que C faisait en silence depuis `void *`, devient un cast écrit, **après** l'`assert` de classe qui la précédait déjà. Mesuré sur copie : `make test` vert, `make e2e` **26 conformes**, aucune constante d'empreinte changée, `sanitize-net` 26 propres, `class-coverage-net` OK.

### Texte du plan, recopié (`docs/plan-3.md` à `8aea055`, ligne 2.8)

> - 2.8 — `ptrs-entité` et `void-ptr` de listes : une liste typée d'entités (éléments `pico8_struct_header_t *`, le noyau de 2.3), avec ses `add`/`del`/`free`/itérateur, remède nommé par la perte acceptée de D.20b, 2. Une liste qui se révélerait non-entité rejoint `PTRS_NON_ENTITE` du compteur avec son écrivain, comme les quatre de `composition_residue.sh`, au lieu d'être typée. `class-coverage-net` : oui si un constructeur est touché. `sanitize-net` : oui (propriétaire du stockage des listes).
>   - 2.8a — le type, `trade.lp`/`trade.rl` (`trade.h:12-13`), `pico8_trade_to` typé (`trade.h:63`), une appartenance typée à côté de `pico8_gu_ptrs` pour l'appelant `planet_family_trade.c:138`. Compte : `ptrs-entité` 7 → **5** ; `void-ptr` 11 → **10** ; pied → **≈ 1080 / 88**. Preuves : chemin touché (`gcov`, mutations sur `add`/`del` et l'ordre) ; preuve compilateur négative : un non-entité ajouté dans une copie ne compile pas. Budget ≈ 50 min.
>   - 2.8b — `jo.oe`, `jo.cc`, `jo.by` (`planet.h:43`, `:58`, `:59`) et leurs lecteurs, dont `pico8_jc_ce` (`slipway.c:273`). Compte : `ptrs-entité` → **2** ; pied → **≈ 1077 / 88**. Preuves : idem. Budget ≈ 50 min.
>   - 2.8c — `ui.le` (`ui.h:35`), `mz.ox` (`wormhole.h:36`), `pico8_gu_ptrs` retiré (`collect.h:19`, dernier appelant `ui.c:220`), `pico8_ha_pz` typé (`menu.h:25`). Compte : `ptrs-entité` → **0**, gardé ; `void-ptr` 10 → **8** ; pied → **1073 / 85**. Preuves : idem. Budget ≈ 50 min.

Un seul écart de compte avec le plan, mesuré : `trade.h` n'a que des lignes `ptrs-entité` et `void-ptr`, il **quitte le pied dès 2.8a** ; le pied de 2.8a et 2.8b compte donc **87** fichiers, pas 88. Les lignes et l'arrivée (1073 / 85) sont celles du plan.

### Inventaire des sept listes (lu dans `src/`, prouvé par le compilateur sur copie)

Chaque écrivain de `src/` passe désormais un `pico8_struct_header_t *` typé à `pico8_ents_add` (liste en « Greps de Sortie ») : **aucune liste n'est non-entité**, aucune ne rejoint `PTRS_NON_ENTITE`, et `tools/composition_residue.sh` n'est pas touché.

| Liste | Éléments | Écrivain(s) dans `src/` | Propriétaire du conteneur (le struct `pico8_ents_t` et son `items`) |
|---|---|---|---|
| `trade.lp` | planètes (`jo` et famille) | `pico8_jo_ib` (`planet_trade.c:220-222`, `{self}`) ; `pico8_trade_kg` (`trade.c:124-137`, `la(lp,{np})`) | jamais libéré (TROU-7, `trade.h:8`) ; `kg` libère ses deux listes d'un élément sur la pile |
| `trade.rl` | slipways (`jc`) | `pico8_jo_ib` (`planet_trade.c:223-224`, `{w}`) ; `pico8_trade_kg` (`la(rl,{jc})`) | idem |
| `jo.oe` | slipways | `pico8_jo_kf` (`planet_trade.c:54`) | alloué par `pico8_jo_pq` (`planet.c:281-283`), jamais libéré (TROU-7) |
| `jo.cc` | routes (`trade`) | `bd_route_visit` (`economy_tiers.c:132`) | `bd_reset_visit` (`economy_tiers.c:46-53`) libère l'ancien (`items` puis struct) et en alloue un neuf à chaque `bd` |
| `jo.by` | routes | `pico8_jo_ib_kf_closure` (`planet_trade.c:39`, via `lw.ib`) ; affecté par `bd_route_visit` (`economy_tiers.c:68-69`) | `pico8_jo_ib` rend `self.by` ou un conteneur neuf (`planet_trade.c:189-195`), jamais libéré ; les `ga` partagent `g_ga_by_shared` (`planet_family.c:23`, statique, vidé par `:632`) |
| `ui.le` | boutons `js` | `pico8_pz` (`msgbox.c:383-389`) | `pico8_pz` libère le sien s'il n'y a pas d'`ok` (`msgbox.c:394-395`), sinon jamais (TROU-7) ; `pico8_ui_cj` alloue un vide (`ui.c:471-474`) ; défaut : `g_shared_le` (`ui.c:22`, statique, vidé par `pico8_ui_init`, `:125`) |
| `mz.ox` | entités de `hv` dont `get_fx` est vrai | `pico8_mz_pq` (`wormhole.c:48-51`, filtre de `hv`) | jamais libéré (TROU-7) |

La propriété ne change pas : `pico8_ents_free` libère `items` seul, comme `pico8_ptrs_free` ; chaque `free` de struct reste où il est. D'où `sanitize-net` au dernier commit (le plan le demande pour chaque sous-étape ; mesuré sur l'état final, 26 propres).

Hors de ces sept, et **non comptés** par le compteur (pas des membres d'un type d'entité), des conteneurs qui tiennent aussi des entités restent `pico8_ptrs_t` : les registres `hv`, `cv`, `cg` (`entity.c`, que 2.7 réécrit), `ui.ic.layers` (`ui.h:23`, membre d'un `pico8_ui_ic_t`), les calques locaux de `pico8_dc`. Voir « Out of Scope » et « Surprises ».

### Recouvrement avec 2.7

2.7 (en triage, appliquée en parallèle) touche `struct.h`, `entity.c` (`cg` en seaux fixes de `pico8_ptrs_t`), les membres `nu` et leurs lecteurs, dont `planet.c:79`, `msgbox.c:88`, `slipway.c:367`, `wormhole.c:205`, `planet_family_trade.c:204`, et les 17 TU qui plient le masque. 2.8 ne touche ni `struct.h`, ni `entity.c`, ni `cg`, ni un membre `nu`. Les fichiers communs (`planet.c`, `planet.h`, `msgbox.c`, `slipway.c`, `wormhole.c`, `wormhole.h`, `planet_family.c`, `planet_family_trade.c` et des TU de planète) sont touchés à des **lignes différentes** : conflit de texte au plus, jamais de sens. Ce qui dépend de 2.7, précisément :

1. **Le pied.** Mesuré sur `8aea055` (sans 2.7) : **1290 / 88 → 1287 / 87 → 1284 / 87 → 1280 / 85**. Les dix lignes qui partent (`ptrs-entité` 7, `void-ptr` 3) ne sont pas des lignes `nu`, et les trois fichiers qui sortent (`trade.h`, `collect.h`, `menu.h`) n'ont aucune ligne `nu` ; 2.7 n'en ajoute à aucun (son PRD : aucun des trois dans sa liste de fichiers). Donc, après 2.7c (**1083 / 88**, PRD de 2.7) : **1080 / 87 → 1077 / 87 → 1073 / 85**. Un départ autre que 1083 / 88 : arrêt, rapporté.
2. **La garde.** Après 2.7c : `COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu`. 2.8c y ajoute `,ptrs-entité` en fin.
3. **`--champs`.** Mesuré sur `8aea055` : 287 → 287 (2.8 ne crée ni ne retire aucun champ ; seuls des types changent). Après 2.7b : 282 → 282.
4. **`make line-coverage`.** Mesuré sans 2.7 : 6308 → 6353 (+45 : les lignes neuves de `pico8_ents.c` et des boucles écrites) ; avec 2.7 le départ diffère, seul le delta et `L` comptent.
5. **Les numéros de ligne** : ceux de `8aea055` ; après 2.7, chercher le texte.

### État de départ attendu (après 2.7c)

| Commande | Résultat |
|---|---|
| `make composition-residue` | `parente 10 3`, `lien-parent 111 27`, `accesseurs-q6 237 33`, `défauts 669 52`, **`void-ptr 11 7`**, **`ptrs-entité 7 4`**, `inclusion-mutuelle 17 16` ; **`résidu 2 : 1083 ligne(s), 88 fichier(s)`** |
| `sh tools/composition_residue.sh --check ptrs-entité,void-ptr` | les 18 lignes citées plus haut, code 1 |
| `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` | `…:= noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu` |
| `make test` ; `make e2e` | vert ; 26 conformes |

Vérifié en fin d'écriture sur `aa126e1` (2.7a-c commises et vérifiées entre-temps) : compteur et garde exactement comme ci-dessus (`Makefile:864`), `tools/float_guard.txt` à `garde fichiers 57`.

## Solution

Trois commits, dans l'ordre du plan, chacun vert (`make test`, `make e2e` 26 conformes) — **les trois découpages ont été compilés et joués séparément sur la copie**.

- **Le module `pico8_ents`** (`src/pico8_ents.{h,c}`, 2.8a) : `pico8_ents_t { pico8_struct_header_t **items; int count; int capacity; }`, `init`, `free`, `add`, `del`, `has` (appartenance par identité, `NULL` ne contient rien), `cat` (le `la` du cart), `all` + `iter_next` (même tolérance que `pico8_ptrs_all`). Aucun `void *` dans l'en-tête. Il n'inclut que `pico8_struct.h`. Son TU `tests/test_pico8_ents.c`.
- **Les sept membres** deviennent `pico8_ents_t *`, par sous-étape : 2.8a `trade.lp`/`rl` (et `pico8_trade_keys_t`), 2.8b `jo.oe`/`cc`/`by` (et l'`ib` d'une proposition, `pico8_jo_ib`), 2.8c `ui.le` (et `pico8_ui_le_shared`) et `mz.ox`.
- **Les lecteurs** : chaque local qui reçoit un élément d'une de ces listes est `pico8_struct_header_t *` ; le cast `(const pico8_struct_header_t *)x` des `assert` de classe disparaît ; la descente vers le type concret est un cast explicite après cet `assert` (`route = (const pico8_trade_t *)e;`).
- **Les trois prototypes `void-ptr`** : `pico8_trade_to(…, pico8_struct_header_t **out)` (2.8a) ; `pico8_gu_ptrs` retiré, ses deux appelants passent à `pico8_ents_has` (2.8a pour `rl`, 2.8c pour `le`) ; `const pico8_struct_header_t *pico8_ha_pz(void)` (2.8c).
- **Les parcours `collect` sur ces listes** deviennent des parcours typés : `la` → `pico8_ents_cat` (`trade.c`), `nv` du mapper de `cj` → boucle `pico8_ents_all` qui appelle `pico8_he` (`ui.c`), `jp` de `mz:pq` → boucle sur `hv` qui ajoute les entités dont `get_fx` est vrai (`wormhole.c`), `ij` sur `ox` → `pico8_ij_ents` (`entity_geom.c`, pas de calcul partagé avec `pico8_ij`, qui reste pour `cg`).
- `ptrs-entité` entre dans la garde (2.8c) ; `tools/composition_r2_sites.txt` suit (2.8b, 2.8c) ; glossaire.

## User Stories

1. En tant que gardien de la perte acceptée de D.20b, je veux qu'un pointeur typé vers un non-entité ajouté à l'une des sept listes soit une erreur de compilation, afin que l'erreur de représentation qui a fait quatre débordements de tas ne puisse plus entrer par une liste.
2. En tant que lecteur d'une route, je veux lire `lp`/`rl` comme des en-têtes d'entité, afin de voir à chaque lecture où le code descend vers `jo` ou `jc`.
3. En tant qu'auteur de l'aplatissement (2.15), je veux que chaque descente depuis une de ces listes soit un cast écrit, afin que le compilateur me les montre quand la forme des types changera.
4. En tant que gardien du jeu, je veux `make e2e` à 26 conformes et aucun `*.sha256` touché, à chaque commit.
5. En tant que gardien des empreintes, je veux chaque `*_HASH`/`FROZEN_*` de TU inchangé, afin d'être sûr que les listes rendent les mêmes éléments dans le même ordre.
6. En tant que gardien de la mémoire, je veux `sanitize-net` propre, afin que le nouveau conteneur ne change ni la durée de vie ni le propriétaire d'un stockage.
7. En tant que vérificateur, je veux `L`, les mutations rejouées au parent et la preuve compilateur négative, afin de savoir qu'aucun chemin ne perd son test.
8. En tant que gardien du compteur, je veux `ptrs-entité` dans la garde, et la preuve qu'un membre réinjecté rougit `make test`.
9. En tant que lecteur de `--detail`, je veux que les lignes R2 de `pico8_jc_hq` et `pico8_bm_has` disent leur amont réel après 2.8, afin de ne pas chercher un `void *` qui n'existe plus.
10. En tant que lecteur du glossaire, je veux savoir quelles listes sont typées et lesquelles restent `pico8_ptrs_t`, et que `gu` ne renvoie plus à une fonction supprimée.
11. En tant que lecteur du plan, je veux une note « Découvert à » courte par sous-étape, afin que les études 2.9 à 2.11 partent des vrais comptes.

## Implementation Decisions

### Modules

`ents` (neuf), `trade`, `planet`, `planet_trade`, `planet_family`, `planet_family_trade`, `planet_display`, `economy_tiers`, `economy_score`, `slipway`, `ui`, `msgbox`, `wormhole`, `entity_geom`, `menu`, `collect` ; TU `ents` (neuf), `trade`, `os`, `nr`, `fr`, `ga`, `ga_export_panel_by`, `hj`, `jc`, `jo`, `mx`, `nl`, `pm`, `qh`, `ui`, `js`, `lq`, `pj`, `planet_gh_row_lq`, `spec_defaults`, `iv`, `mz` ; `Makefile` ; `tools/float_guard.txt` (réécrit par `make test`) ; `tools/composition_r2_sites.txt` ; `docs/glossaire-noms.md` ; `docs/plan-3.md`.

### Le module `pico8_ents` (2.8a, texte exact essayé sur copie)

`src/pico8_ents.h` :

```c
#ifndef PICO8_ENTS_H
#define PICO8_ENTS_H

#include "pico8_struct.h"

/* pico8_ptrs_t for entities only. Owns only `items`; NULL is storable. */
typedef struct {
    pico8_struct_header_t **items;
    int count;
    int capacity;
} pico8_ents_t;

void pico8_ents_init(pico8_ents_t *ents);
/* Releases `items` only and leaves `ents` empty. */
void pico8_ents_free(pico8_ents_t *ents);
void pico8_ents_add(pico8_ents_t *ents, pico8_struct_header_t *e);
/* Removes the FIRST element identical to `e`; 1 if one was removed. */
int pico8_ents_del(pico8_ents_t *ents, const pico8_struct_header_t *e);
/* 1 if `e` is an element (pointer identity); `ents` NULL holds nothing. */
int pico8_ents_has(const pico8_ents_t *ents, const pico8_struct_header_t *e);
/* A fresh list: `a` then `b`; the caller frees its `items`. */
pico8_ents_t pico8_ents_cat(const pico8_ents_t *a, const pico8_ents_t *b);

/* As pico8_ptrs_iter_t. */
typedef struct {
    const pico8_ents_t *ents;
    int index;
    int last_count;
} pico8_ents_iter_t;
pico8_ents_iter_t pico8_ents_all(const pico8_ents_t *ents);
int pico8_ents_iter_next(pico8_ents_iter_t *iter, pico8_struct_header_t **out);

#endif
```

`src/pico8_ents.c` : le corps de `pico8_ptrs.c` recopié sur `pico8_struct_header_t **` (`init`, `free` = `free(items)` puis `init`, `add` qui double la capacité à partir de 4 et `abort()` si `realloc` rend NULL, `del` qui décale, `all`/`iter_next` avec la même règle `count < last_count ? index : index + 1`), plus `has` (boucle d'identité, `ents` NULL → 0) et `cat` (`init` puis `add` de chaque élément de `a`, puis de `b`). 88 lignes sur copie. Pas de `foreach` : aucun appelant.

`tests/test_pico8_ents.c` (83 lignes sur copie, quatre tests, éléments `pico8_struct_header_t` locaux) : ordre et croissance au-delà de 4, `free` qui vide ; `del` du premier identique seulement, `del` d'un absent sans effet, `has` (présent, absent, liste NULL, élément NULL stocké) ; `cat` qui copie `a` puis `b` sans toucher les sources ; itération qui supprime l'élément rendu à chaque pas et les voit tous.

`Makefile` : `src/pico8_ents.c` avant `src/pico8_ptrs.c` dans `ENGINE_SRCS` et `DATA_ENGINE_SRCS`, et dans les **deux** listes de la cible `$(TMP_DIR)/main_raylib` (prérequis et commande, `Makefile:947-948`) ; une règle `$(TMP_DIR)/test_pico8_ents: $(call objs,tests/test_pico8_ents.c src/pico8_ents.c)` calquée sur `test_pico8_ptrs` (`:408-409`) ; `test_pico8_ents` après `test_pico8_ptrs` dans la liste des TU (`:572`). `grep -c 'pico8_ents.c' Makefile` → **5**. `make test` réécrit `tools/float_guard.txt` : `garde fichiers 57 en-echec 0` → **`58`** ; la ligne entre dans le commit 2.8a.

### Ce qui change, fichier par fichier (numéros de `8aea055`)

**2.8a — le type, `trade.lp`/`rl`, `pico8_trade_to`**

- `src/pico8_trade.h` : `#include "pico8_ents.h"` ; `:12-13` et `:47-48` (`pico8_trade_t`, `pico8_trade_keys_t`) → `pico8_ents_t *lp;`, `pico8_ents_t *rl;` ; `:63` → `int pico8_trade_to(const pico8_trade_t *self, pico8_struct_header_t **out);` ; commentaire `:23-26` : « The generic membership helper (pico8_ents_has) therefore compares two trades by pointer identity… ».
- `src/pico8_trade.c` : `:8` include `collect.h` retiré (plus aucun symbole) ; `:80-81` `const pico8_ents_t *lp;` ; `:101-108` les huit locaux en `pico8_ents_t` ; `init`/`add`/`free` des listes `{np}`/`{jc}` en `pico8_ents_*` ; `:135`, `:137` `pico8_la_ptrs` → `pico8_ents_cat` (commentaire `:96` idem) ; `:148-150` `const pico8_ents_t *l_lp, *r_lp;`, `const pico8_struct_header_t *l_first, *r_first;`, `pico8_struct_header_t *l_last, *r_last;`.
- `src/pico8_economy_tiers.c` (route de `bd_route_visit`) : `:78-80` `const pico8_ents_t *rl; const pico8_ents_t *lp; pico8_struct_header_t *to_val;` ; `:94`, `:96` `w_val`/`planet_val` en `const pico8_struct_header_t *` ; `:110` `planet = (const pico8_jo_t *)planet_val;` ; `:114-116` `assert(w_val->cls == pico8_jc_struct_class() && "trade.rl holds slipways only"); w = (const pico8_jc_t *)w_val;` ; `:130` `dest = (pico8_jo_t *)to_val;`.
- `src/pico8_economy_score.c` : `:101` `pico8_struct_header_t *oz_val;` ; `:112` `oz = (const pico8_jo_t *)oz_val;`.
- `src/pico8_planet_trade.c` (`pico8_jo_ib`) : `:210-211` `pico8_ents_t *lp_list, *rl_list` ; `:220-224` `pico8_ents_init`, `pico8_ents_add(lp_list, &self->header)`, `pico8_ents_add(rl_list, w)`.
- `src/pico8_planet_family_trade.c` (`pico8_nr_cn`) : `:124` `const pico8_ents_t *t_rl = NULL;` ; `:138` `pico8_gu_ptrs(t_rl, w)` → `pico8_ents_has(t_rl, w)` ; `:8` include `collect.h` retiré (plus aucun symbole). `pico8_gu_ptrs` reste pour `ui.c:220` jusqu'à 2.8c.
- TU `trade` : `fold_ranks`, `list_of` (garde `void *const *items`), `route`, `list_is` (`pico8_struct_header_t *const *items`), locaux `lp`/`rl` de `test_construction` et `test_kg` en `pico8_ents_t` ; `want[16]` en `pico8_struct_header_t *` ; `fold_route` et `to_on_other` : `pico8_struct_header_t *to`/`out` ; `test_to` : `static pico8_struct_header_t sentinel_target;` et `out` typé (le témoin était un `static int`) ; `route((pico8_ents_t *)lp, …)` (cast de `const` seulement). TU `os` : `export_to`, la liste `lp` ; TU `nr` : `with_w`/`without_w` en `pico8_ents_t`, `pico8_ents_add(with_w, &w->header)`.
- Compte : `ptrs-entité` 7 / 4 → **5 / 3** ; `void-ptr` 11 / 7 → **10 / 6** ; pied → **1080 / 87** (mesuré 1287 / 87 sans 2.7).

**2.8b — `jo.oe`, `jo.cc`, `jo.by`**

- `src/pico8_planet.h` : include `pico8_ents.h` ; `:43`, `:58`, `:59` → `pico8_ents_t *`.
- `src/pico8_planet.c` (`pico8_jo_pq`) : `:275` `pico8_ents_t *oe;`, `:282` `pico8_ents_init(oe);`.
- `src/pico8_planet_trade.h` : `:27` (`ib` de la proposition) et `:38` (`pico8_jo_ib`) → `pico8_ents_t *`.
- `src/pico8_planet_trade.c` : `gu_trade` (`:18-31`) sur `const pico8_ents_t *list`, itérateur `pico8_ents_*`, `pico8_trade_eq((const pico8_trade_t *)e, v)` ; commentaire `:15` : « not pico8_ents_has's pointer identity » ; `:39` `pico8_ents_add(self->ib, &a->header);` ; `pico8_jo_kf` `:45`, `:54` ; `pico8_jo_ib` `:177-183` (`pico8_ents_t *ib`, `const pico8_ents_t *oe`, `pico8_ents_iter_t it`, `pico8_struct_header_t *w_val`), `:193` init, `:206-207` itérateur.
- `src/pico8_planet_family.c` : `:23` `static pico8_ents_t g_ga_by_shared;`, `:632` `pico8_ents_init`.
- `src/pico8_planet_family_trade.c` (`pico8_nr_cn`) : `:110-112` `const pico8_ents_t *oe`, `pico8_ents_iter_t it`, `pico8_struct_header_t *w` ; `:121-129` la boucle itère directement dans `w` (le local `w_val` et sa copie partent).
- `src/pico8_slipway.c` (`pico8_jc_ce`) : `:264` `const pico8_ents_t *oe` ; `:271-280` itérateur `pico8_ents_*` dans `pico8_struct_header_t *w`, `assert(w->cls == pico8_jc_struct_class() && "qu.oe must hold jc structs: only jo:kf adds to it, and only slipways");` (message inchangé), `w_val` retiré.
- `src/pico8_economy_tiers.c` : `bd_reset_visit` `:39`, `:47`, `:52` ; `bd_route_visit` `:58-60` (`pico8_ents_t *by`, `pico8_ents_iter_t`, `pico8_struct_header_t *e`), `:75-76` itérateur, `:86-88` `assert(e->cls == pico8_trade_struct_class() && "jo.by holds trades only"); route = (const pico8_trade_t *)e;`, `:132` `pico8_ents_add(dest->cc, e)` ; `pico8_dh` `:210` `const pico8_ents_t *cc`, `:240` `const pico8_struct_header_t *im_val`, `:245-247` `assert(im_val->cls == …, "jo.cc holds trades only"); im = (const pico8_trade_t *)im_val;` ; `:296` `const pico8_ents_t *by`.
- `src/pico8_economy_score.c` : `:86` `const pico8_ents_t *by` ; `:99-105` `pico8_struct_header_t *e = by->items[i];` (`e_val` retiré).
- `src/pico8_planet_display.c` : `:38`, `:250` `const pico8_ents_t *`.
- TU : dans `fr`, `ga`, `hj`, `jo`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`, `jc`, les aides de taille (`list_size`, `ptrs_size`, `ptrs_count`) et `listed` servent aussi à `jy`, `cg`, `hv`, `qy` : **une jumelle `ents_<aide>`** au corps identique, appelée sur `oe`/`cc`/`by` (même valeur pliée, empreintes inchangées) ; `mx` reçoit aussi `ents_rank_in`. `fold_ib` (`fr`, `hj`, `mx`, `nl`, `nr`, `os`, `pm`) et `pico8_ents_t *ib = pico8_jo_ib(e)`. `list_of` de `fr` et `nr` : `pico8_ents_t *list_of(int n, pico8_struct_header_t *item)`. `nr` : `take_offer` → `pico8_ents_add(self->ib, &a->header)`, `ib` local, `pico8_ents_add(self->qh.jo.oe, &w->header)`. `ga` : `by` local, `pico8_ents_add(by, &pico8_trade_new(NULL)->header)`. `os` : `by` d'`export_to`. `jc` : `pico8_jc_eb((const pico8_jc_t *)oe->items[j], p)`. `trade` : `game_route` (`by`), `test_dh_counts_numbered_routes_only` (`cc`). `ga_export_panel_by` : `planet_by_ref`, `by`. `ui` : `oe_count` (`:488`).
- `tools/composition_r2_sites.txt`, ligne `pico8_jc_hq` : colonne amont → `aucun : jo.oe est typée (2.8b)`.
- Compte : `ptrs-entité` → **2 / 2** ; `void-ptr` 10 / 6 ; pied → **1077 / 87** (mesuré 1284 / 87 sans 2.7).

**2.8c — `ui.le`, `mz.ox`, `pico8_gu_ptrs`, `pico8_ha_pz`, garde**

- `src/pico8_ui.h` : include `pico8_ents.h` ; `:35` `pico8_ents_t *le;` ; `:67` `pico8_ents_t *pico8_ui_le_shared(void);`.
- `src/pico8_ui.c` : `:7` include `collect.h` retiré ; `:22`, `:28`, `:125` (`g_shared_le`) ; `:194` `pico8_ents_t *le_ptr_before;` ; `:220` `pico8_ents_has(le_ptr_before, self->o_)` ; `:447-458` le commentaire et `pico8_ui_cj_he_mapper` retirés ; `pico8_ui_cj` `:460-474` :

```c
    if (self->le != NULL) {
        pico8_ents_iter_t it = pico8_ents_all(self->le);
        pico8_struct_header_t *e;
        while (pico8_ents_iter_next(&it, &e)) {
            if (e != NULL) {
                pico8_he(e);
            }
        }
    }
```

  puis `pico8_ents_t *heap_le` et `pico8_ents_init(heap_le)`. (`nv` parcourait par `all` et ne gardait rien : même ordre, mêmes appels.)
- `src/pico8_msgbox.c` (`pico8_pz`) : `:337` `pico8_ents_t *le_heap` ; `:384-389` `pico8_ents_init`, `pico8_ents_add(le_heap, &pico8_pz_step(&ctx, &ep_row)->header)` et `&…(&ctx, &bs[i])->header` ; `:394` `pico8_ents_free` ; `:402-403` `pico8_ents_t *empty_heap`, `pico8_ents_init`.
- `src/pico8_wormhole.h` : include `pico8_ents.h` ; `:36` `pico8_ents_t *ox;`.
- `src/pico8_wormhole.c` : `:8` include `collect.h` retiré ; `:29-37` `pico8_mz_pq_ox_predicate` retiré ; `pico8_mz_pq` :

```c
    pico8_ents_t *heap_ox;
    pico8_ptrs_iter_t it;
    void *e;
    …
    heap_ox = malloc(sizeof *heap_ox);
    pico8_ents_init(heap_ox);
    it = pico8_ptrs_all(pico8_hv());
    while (pico8_ptrs_iter_next(&it, &e)) {
        pico8_struct_header_t *p = e;
        if (p != NULL && p->cls->get_fx != NULL && p->cls->get_fx(p)) {
            pico8_ents_add(heap_ox, p);
        }
    }
    self->ox = heap_ox;
```

  (`hv` reste `pico8_ptrs_t` : la conversion `void *` → en-tête se fait ici, une fois, sur un registre que seul `pico8_lt` remplit avec un en-tête typé) ; `:66` `pico8_ij` → `pico8_ij_ents`.
- `src/pico8_entity_geom.h` : include `pico8_ents.h` ; après `:30`, `pico8_fixed_t pico8_ij_ents(pico8_vec_t p, const pico8_ents_t *j_, pico8_struct_header_t **out_nearest);`.
- `src/pico8_entity_geom.c` : le corps de boucle de `pico8_ij` (`:101-111`) devient `static void ij_step(pico8_vec_t p, pico8_struct_header_t *o, pico8_fixed_t *mk, pico8_struct_header_t **out_nearest)`, appelé par `pico8_ij` (inchangé pour ses appelants `cg`, `probe.c:165`, `galaxy.c:67`) et par `pico8_ij_ents` (même forme, itérateur `pico8_ents_*`, `j_` NULL → `mk`).
- `src/pico8_collect.h` `:18-19` et `src/pico8_collect.c` `:26-39` : `pico8_gu_ptrs` retiré (aucun appelant restant, aucun TU).
- `src/pico8_menu.h:25`, `src/pico8_menu.c:393-394` : `const pico8_struct_header_t *pico8_ha_pz(void)`, `return g_menu_up ? &g_ha_pz.header : NULL;`.
- TU : `ui` (`le_of`, locaux `le`, marque `static pico8_struct_header_t mark;` au lieu de `static int`), `spec_defaults` (`ui_le`, `mark_cell` en `pico8_struct_header_t`, `pico8_ents_add`/`del` — c'était un `static int`), `js` (`ok_le`, locaux `le`, jumelles `ents_rank_in`/`ents_listed` pour `ok_le()`), `lq`, `pj`, `planet_gh_row_lq`, `ga_export_panel_by` (`ok_le`, `le`), `iv` (jumelle `ents_listed` pour `mz->ox`), `mz` (`fold_ox`, jumelles `ents_rank_in`/`ents_listed`).
- `Makefile:864` (après 2.7) : `COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité`.
- `tools/composition_r2_sites.txt`, ligne `pico8_bm_has` : amont → `hv->items (void **) dans pico8_dg (bootstrap.c:57)` seul ; preuve → `pico8_he (entity.c:217) <- ui.c:<ligne du pico8_he de pico8_ui_cj> (typée), bootstrap.c:72 ; …` (le reste inchangé).
- `docs/glossaire-noms.md` (voir plus bas).
- Compte : `ptrs-entité` → **0**, disparue de la sortie, gardée ; `void-ptr` → **8 / 4** (les huit méthodes de table du plan) ; pied → **1073 / 85** (mesuré 1280 / 85 sans 2.7) ; `wormhole.h` reste au pied par sa ligne `inclusion-mutuelle` (`:10`).

### Glossaire (`docs/glossaire-noms.md`, numéros de `8aea055`)

- `:109` (`ij`) : « `pico8_ij` ; `pico8_ij_ents` sur `mz.ox` » (2.8c).
- `:285` (`gu`) : « `pico8_ents_has` (listes d'entités, identité de pointeur) ; plus de version `pico8_ptrs_t` » (2.8c ; en 2.8a, ajouter `pico8_ents_has` à côté de `pico8_gu_ptrs`).
- `:290` (`la`) : « `pico8_la_ptrs` ; `pico8_ents_cat` sur les listes d'entités » (2.8a).
- `:661` (« Le conteneur de pointeurs ») : retirer `ui.le`, `mz.ox`, `jo.oe`, `jo.cc`, `jo.by`, `trade.lp`, `trade.rl` et l'`ib` de la liste ; ajouter une ligne « Le conteneur d'entités | `pico8_ents_t` (`pico8_ents.h`) : `add`/`del`/`has`/`cat`/`all` sur des `pico8_struct_header_t *` ; tient `trade.lp`, `trade.rl`, `jo.oe`, `jo.cc`, `jo.by` (défaut partagé des `ga` : `g_ga_by_shared`), l'`ib` d'une proposition, `ui.le`, `mz.ox`. » Chaque sous-étape met à jour ce qu'elle déplace.

### Anti-tassement

- **Interdit** :
  - un `pico8_ents_t` dont `items` n'est pas `pico8_struct_header_t **` (un `void **` renommé), un `typedef` de `pico8_ptrs_t`, ou tout autre nom qui ferait échapper un membre au motif `ptrs-entité` sans typer ses éléments ;
  - un cast `(pico8_ents_t *)` d'un `pico8_ptrs_t`, ou un argument `(void *)…` passé à `pico8_ents_add`/`has`/`del` dans `src/` ;
  - ajouter une des sept listes à `PTRS_NON_ENTITE`, toucher `tools/composition_residue.sh`, `tests/test_composition_residue.c`, `tests/e2e/`, un `*.sha256` ou une constante `*_HASH`/`FROZEN_*` ;
  - changer un message d'`assert` (« jo.by holds trades only », « trade.rl holds slipways only », « trade.lp holds planets only », « jo.cc holds trades only », « qu.oe must hold jc structs… », « nr.oe holds slipways only… », « jo.oe holds slipways only… ») ;
  - retirer `ptrs-entité` de la garde, ou garder `pico8_gu_ptrs` ;
  - typer les registres `hv`/`cv`/`cg` ou `ui.ic` dans cette étape (voir « Out of Scope »).
- Les jumelles de TU (`ents_list_size`, `ents_listed`…) ne sont **pas** du tassement : le TU plie la même valeur sur deux types de liste ; le corps est recopié, pas modifié.

## Implementation Chunks

Trois commits, ≈ 50 min chacun preuves comprises (l'essai sur copie, sans PRD, a pris davantage ; les textes ci-dessus en retirent la recherche) :

1. **2.8a** : module, TU `ents`, `Makefile`, `float_guard.txt`, `trade`, les lecteurs de route, TU `trade`/`os`/`nr` (parties `lp`/`rl`), glossaire (`la`, `gu`, ligne du conteneur d'entités), note « Découvert à 2.8a ».
2. **2.8b** : `jo.oe`/`cc`/`by`, `ib`, lecteurs et 14 TU, R2 `pico8_jc_hq`, glossaire, note.
3. **2.8c** : `le`, `ox`, `pico8_ij_ents`, `pico8_gu_ptrs`, `pico8_ha_pz`, garde, R2 `pico8_bm_has`, `sanitize-net`, glossaire, note.

**Repli écrit (si une sous-étape déborde à 1,5 fois son budget avant le commit)** :
- 2.8a en deux : **2.8a1** le module, son TU, le `Makefile` et `float_guard.txt` seuls (aucun compte ne bouge ; pied 1083 / 88) ; **2.8a2** `trade` et ses lecteurs (comptes de 2.8a).
- 2.8b en deux : **2.8b1** `oe` (`planet.c`, `planet_trade.c` `kf`/`ib`, `planet_family_trade.c`, `slipway.c`, TU des aides de taille sur `oe`) ; **2.8b2** `cc`/`by`/`ib`. Chacune compile seule si l'`ib` d'une proposition suit `by` (même commit).
- 2.8c en deux : **2.8c1** `le` et `pico8_gu_ptrs` (`void-ptr` → 9) ; **2.8c2** `ox`, `pico8_ij_ents`, `pico8_ha_pz`, garde.

Un repli est écrit dans la note « Découvert à ». Les comptes d'arrivée ne changent pas.

## Testing Decisions

- **Un TU nouveau** : `tests/test_pico8_ents.c`, pour les primitives (le TU `ptrs` en est le modèle) ; aucun rejeu d'`events.json`. Les empreintes des TU de planète, de route, de `ui`, `js`, `mz` plient déjà les tailles, rangs et éléments de ces listes : elles prouvent l'identité de comportement, et les mutations montrent qu'elles la voient. Les e2e partent de l'écran titre (`make e2e`).
- **Prior art** : ensemble `L` de 2.4 à 2.6 ; règle du parent de D.21 ; garde mordante de 2.2 et 2.4 ; preuve compilateur de 2.7 (Q3).

### Preuves (chemin touché), à chaque commit sauf mention

1. `make test` vert ; `make e2e` **26 conformes** ; `git diff --stat <parent> -- tests/e2e '*.sha256'` vide ; `git diff <parent> | grep -E '^[-+].*(_HASH|FROZEN_)'` vide. Mesuré à chacune des trois sous-étapes.
2. **`gcov` des e2e** (`make line-coverage EVENTS="$(ls tests/e2e/*.json | tr '\n' ' ')"`, dans une copie) au parent puis à l'enfant, appariement `difflib` des sources. Mesuré sur l'ensemble 2.8 (sans 2.7) : **6308 → 6353**. **Aucune ligne inchangée perdue.** `L` (couvertes au parent, retirées ou réécrites) = **99 lignes**, toutes dans les hunks de cette étape : `collect.c:26-38` (`pico8_gu_ptrs`), `economy_score.c:99-112`, `economy_tiers.c:47-296` (sites cités), `entity_geom.c:103-110`, `menu.c:393-394`, `msgbox.c:384-403`, `planet.c:282`, `planet_display.c:250`, `planet_family.c:632`, `planet_family_trade.c:110-138`, `planet_trade.c:18-224`, `slipway.c:264-280`, `trade.c:81-140`, `ui.c:125`, `:220`, `:450-473`, `wormhole.c:29-66`. **Lignes exécutables neuves non exécutées par les e2e** (13, toutes déclarées) : `ents.c` `abort()` (échec de `realloc`) et le corps de `pico8_ents_del` (aucun appelant dans `src/` ; couvert par les TU `ents` et `spec_defaults`) ; `entity_geom.c` le `return mk;` de `pico8_ij_ents` sur `ox` NULL (un `mz` qui n'a pas fait `pq`) ; `msgbox.c:394` (`pz` sans `ok`, déjà non couvert au parent) ; `ui.c:28` (`pico8_ui_le_shared`, lu par les TU seuls, déjà non couvert au parent). Toute autre différence : arrêt, rapporté. Par sous-étape, `L` est la part de ces 99 que la sous-étape touche.
3. **Mutations**, une à la fois, à l'enfant puis la même au parent sur l'ancienne forme (`make test` puis `make e2e`). **ÉCHEC** si verte à l'enfant et rouge au parent. Mesuré sur copie (l'enfant = état final ; le parent = `8aea055`), identique aux deux arbres :

| # | Mutation enfant ; parent | Premier TU rouge | e2e conformes enfant / parent |
|---|---|---|---|
| M1 | ordre : `pico8_ents_cat` copie `b` puis `a` ; parent `pico8_la_ptrs` idem (et, à part, l'appel `la(np_list, lp)` de `trade.c:135`) | `ents` ; parent `js` | 19 / 19 |
| M2 | `pico8_ents_has` rend 0 sur un élément trouvé ; parent `pico8_gu_ptrs` idem | `nr` (`test_cn_skips_a_slipway_already_in_rl`) | 9 / 9 |
| M3 | itérateur sans tolérance (`index + 1` toujours) ; parent dans `pico8_ptrs_iter_next` | `ents` ; parent `ptrs` | 26 / 26 — aucune des sept listes n'est modifiée pendant un parcours en jeu ; déclaré |
| M4 | `ox` sans filtre `get_fx` (`if (p != NULL)`) ; parent : prédicat `return p != NULL;` | `iv` (`test_mz_scan_skips_a_live_toast`), `mz` | 8 / 8 |
| M5 | `cj` n'appelle plus `pico8_he` (`if (e == NULL)`) ; parent : même inversion dans le mapper | `js` | 9 / 9 |
| M6 | `bd` n'ajoute plus la route à `dest->cc` ; parent : même ligne retirée | `js` | 19 / 19 |

   Par sous-étape : M1, M3 à 2.8a ; M2 à 2.8a (`rl`) et 2.8c (`le`) ; M6 à 2.8b (`cc`) ; M4, M5 à 2.8c. Piège mesuré : une mutation qui laisse un local inutilisé (`p = e; return 1;`) casse la compilation sous `-Werror` et rend 0 conforme ; ce n'est pas un rouge de test.
4. **Preuve compilateur négative** (2.8a, sur copie) : `pico8_ents_add(l, want)` avec `pico8_jy_t *want` → `error: passing argument 2 of 'pico8_ents_add' from incompatible pointer type [-Werror=incompatible-pointer-types]`. Mesuré. Limite, déclarée : un `void *` se convertit sans bruit ; d'où la grep de Sortie 4 sur `src/`.
5. **Garde mordante** (2.8c, dans une copie) : `pico8_ptrs_t *ox;` remis dans `wormhole.h` → `make composition-guard` code 2, `ptrs-entité: src/pico8_wormhole.h:37:    pico8_ptrs_t *ox;`, `résidu gardé (…,ptrs-entité) : 1 ligne(s)` ; idem avec `pico8_ptrs_t *le;` dans `ui.h`. Mesuré.
6. `make sanitize-net` : **26 propre(s)** (mesuré sur l'état final, exigé à 2.8c ; recommandé à chaque sous-étape, 3,2 s mesurées). `make class-coverage-net` : **OK**, « 26 rejeu(x) » (mesuré ; aucun constructeur ne change de texte, mais `pico8_ui_struct_new` écrit `le` et `pico8_trade_struct_new` écrit `lp`/`rl` sous leur nouveau type).
7. `--champs` : inchangé (287 sans 2.7, 282 après 2.7b ; `0 écriture : 15 ; 0 hors constructeur : 27`). Mesuré sans 2.7.

### Greps de Sortie (sur l'arbre de 2.8c)

1. `grep -rnE 'pico8_ptrs_t \*(lp|rl|oe|cc|by|le|ox|ib);' src` → **vide**.
2. `grep -rn 'pico8_gu_ptrs\|pico8_ui_cj_he_mapper\|pico8_mz_pq_ox_predicate' src tests docs/glossaire-noms.md` → **vide**.
3. `grep -rnE 'void \*(w_val|e_val|oz_val|to_val|im_val|planet_val)\b' src` → **vide**.
4. `grep -rn 'pico8_ents_add\|pico8_ents_has\|pico8_ents_del' src | grep -v 'src/pico8_ents\.'` → 14 lignes : deux commentaires (`planet_trade.c:15`, `trade.h:24`) et 12 appels (`planet_trade.c` ×4, `trade.c` ×2, `planet_family_trade.c`, `ui.c`, `wormhole.c`, `economy_tiers.c`, `msgbox.c` ×2), chaque argument un `pico8_struct_header_t *` typé ou un `&x->header` ; `grep -rnE 'pico8_ents_(add|has|del)\([^,]*, \(void' src tests` → **vide**.
5. `grep -rn '(pico8_ents_t \*)' src tests` → une ligne, `tests/test_pico8_trade.c` (`route((pico8_ents_t *)lp, …)`, levée de `const`).
6. `grep -c 'pico8_ents.c' Makefile` → **5** ; `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` → `…,nu,ptrs-entité`.
7. `sh tools/composition_residue.sh --check ptrs-entité` → `résidu gardé (ptrs-entité) : 0 ligne(s)`, code 0 ; `--check void-ptr` → 8 lignes (`entity.h:47`, `planet.h` `pico8_jo_set_m_`, `planet_family.h` ×3, `probe.h` ×3).
8. `make composition-residue` → pas de ligne `ptrs-entité`, `void-ptr 8 4`, pied **`1073 / 85`**.
9. `grep -n 'collect.h' src/pico8_trade.c src/pico8_ui.c src/pico8_wormhole.c src/pico8_planet_family_trade.c` → **vide**.

### Budget et tableau « Runs »

Budget : **3 × ≈ 50 min**. Au-delà de 1,5 fois, repli, sans rien commettre d'incomplet. Jamais l'appli raylib ; `make raylib` n'est pas exigé (grep de Sortie 6 pour ses deux listes).

L'implémenteur tient un tableau « Runs », une ligne par lancement. Voici celui des essais de ce PRD, sur une copie de `8aea055` (`p` parent ; `f` état final ; `a`, `b` les arbres des sous-étapes rejoués séparément) :

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make test` | `p` | 0 | vert | non chronométré (froid) |
| 2 | `composition_residue.sh` ; `--detail` ; `--check ptrs-entité,void-ptr` | `p` | 0 ; 0 ; 1 | 1290 / 88 ; 18 lignes | < 1 s |
| 3 | `gcc -fsyntax-only` de chaque `.c` de `src/` et `tests/` après le seul changement des sept membres | `f` en cours | — | 13 `.c` de `src/` (42 erreurs) et 21 TU (101 erreurs) ; `main_screencast_viewer.c` échoue déjà sans `raylib.h` | ≈ 1 min |
| 4 | `make test` ; `make e2e` | `f` | 0 ; 0 | vert (`test_pico8_ents: ok`) ; 26 conformes | non chronométré ; ≈ 5 s une fois compilé |
| 5 | `make sanitize-net` | `f` | 0 | `sanitize: 26 propre(s)` | 3,2 s |
| 6 | `make class-coverage-net` ; `--champs` | `f` | 0 ; 0 | `couverture: OK` ; 287 / 15 / 27 | ≈ 10 s |
| 7 | `make test` garde élargie ; membre `ox`, puis `le`, réinjecté | `f` | 0 ; 2 ; 2 | `composition-guard` Error 1, 1 ligne | ≈ 5 s × 3 |
| 8 | `gcc` d'un `pico8_ents_add` d'un `pico8_jy_t *` | `f` | 1 | `incompatible-pointer-types` | < 1 s |
| 9 | M1 à M6 à l'enfant | `f` | 2 | tableau des mutations | ≈ 10 s × 6 |
| 10 | M1 (et M1b) à M6 au parent | `p` | 2 | idem ; un premier M4 cassait la compilation, refait | ≈ 10 s × 8 |
| 11 | `make line-coverage` sur les 26 e2e | `f` / `p` | 0 | 6353 / 6308 ; `L` 99, aucune perte inchangée | 4,8 s × 2 |
| 12 | `gcov -t` des 15 fichiers touchés | `f` | 0 | 157 lignes exécutables neuves, 13 non couvertes (déclarées) | < 1 s |
| 13 | includes `collect.h` retirés, `-fsyntax-only` ; `make test` ; `make e2e` | `f` | 0 | vert ; 26 conformes | ≈ 20 s |
| 14 | 2.8a seule : `-fsyntax-only` ; `make test` ; `make e2e` ; compteur | `a` | 0 | vert ; 26 ; `ptrs-entité 5 3`, `void-ptr 10 6`, 1287 / 87 | non chronométré |
| 15 | 2.8b seule : idem (un premier essai rouge : trois lignes `le` de TU laissées au type de 2.8c, rendues à `pico8_ptrs_t`) | `b` | 2 puis 0 | vert ; 26 ; `ptrs-entité 2 2`, 1284 / 87 | non chronométré |
| 16 | `git diff` hash / e2e / sha256 | `f` | 0 | vide ; `tools/float_guard.txt` 57 → 58 | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas 2.7c vérifiée (pied 1083 / 88, garde terminée par `nu`), sans explication.
- `FAILED:` si, à l'un des trois commits, `make test` ou `make e2e` (26 conformes) est rouge, si un `*.sha256` ou `tests/e2e/` change, ou si une constante `*_HASH`/`FROZEN_*` change.
- `FAILED:` si un message d'`assert` cité dans « Anti-tassement » change d'un caractère.
- `FAILED:` si une ligne inchangée entre dans `L`, ou si une ligne exécutable neuve non couverte par les e2e n'est pas l'une des 13 déclarées.
- `FAILED:` si une mutation M1 à M6 est verte à l'enfant et rouge au parent.
- `FAILED:` si la preuve compilateur négative n'est pas dans le message de 2.8a.
- `FAILED:` si la garde ne finit pas par `,ptrs-entité` à 2.8c, ou si le membre réinjecté ne rougit pas `make test`.
- `FAILED:` si `sanitize-net` n'est pas propre à 2.8c, ou si `class-coverage-net` n'est pas OK.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit, ou si les comptes d'arrivée ne sont pas `ptrs-entité` 0, `void-ptr` 8 / 4, pied 1073 / 85 (et 1080 / 87, 1077 / 87 en chemin), sans explication.
- `FAILED:` en cas de tassement (voir « Anti-tassement »).
- `FAILED:` si un message de commit omet : les sites `--check` du parent retirés par ce commit, `L`, les mutations avec leur résultat au parent, la garde mordante (2.8c), `sanitize-net` (2.8c), le tableau « Runs ».
- `FAILED:` s'il manque une note « Découvert à 2.8a/b/c », ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- **Les registres `hv`, `cv`, `cg`**, `ui.ic.layers` et les calques de `pico8_dc` : ils tiennent des entités mais ne sont pas des membres d'un type d'entité ; aucune catégorie ne les compte. `cg` est réécrit par 2.7. Leurs lecteurs restent l'amont `void *` de `pico8_bm_has` et `pico8_dc` dans `--detail` (lignes R2).
- Les quatre listes de `PTRS_NON_ENTITE` (`hy.invented`, `jo.jy`, `hp.i`, `jc.ts`) et `emitter.qy` (particules sans en-tête, membre d'un composant) : inchangées.
- Les huit `void-ptr` restants (méthodes de table) : 2.14 à 2.16.
- Libérer les listes que TROU-7 laisse fuir : hors étape, la propriété ne change pas.
- Les includes morts de `collect.h` déjà présents avant 2.8 (`cart.c`, `menu.c`, `msgbox.c`, `planet_family.c`, `research_display.c`, `score_panel_display.c`) : pas touchés.
- L'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Un module neuf `pico8_ents`, pas un type dans `ptrs.h` ni dans `entity.h`.** `ptrs.h` est la primitive PICO-8 sans en-tête d'entité ; `entity.h` tient les registres, que 2.7 réécrit. `ents.h` n'inclut que `struct.h` : aucune paire d'inclusion neuve (`inclusion-mutuelle` 17 / 16 inchangée, mesuré).
- **G2. Un tableau typé, pas une enveloppe de `pico8_ptrs_t`.** Une enveloppe garderait des `void *` à la lecture, ce que le plan veut ôter (« typées, les listes font des casts de l'aplatissement des erreurs de compilation ») ; une union de deux vues serait un accès par un type non compatible. D'où ≈ 85 lignes recopiées de `ptrs.c`.
- **G3. `has` et `cat` dans le module, pas de `foreach` ni de `jp`/`nv` typés.** Seuls ces deux parcours ont un appelant sur les sept listes ; les deux autres (`cj`, `mz:pq`) deviennent des boucles lisibles sur place.
- **G4. `pico8_ij_ents` à côté de `pico8_ij`, avec un pas commun `static`.** `pico8_ij` sert aussi `cg("np")` et `cg("jo")`, qui restent `pico8_ptrs_t`.
- **G5. Les marques non-entités des TU `ui` et `spec_defaults` deviennent des `pico8_struct_header_t` statiques** : elles ne servaient qu'à compter ; c'est précisément ce que le type refuse désormais. Le témoin `sentinel` de `test_to` (TU `trade`) aussi.
- **G6. Jumelles d'aides dans les TU** plutôt qu'une aide générique : le corps est recopié mot pour mot, les empreintes ne bougent pas.
- **G7. Includes `collect.h` devenus morts retirés** dans `trade.c`, `planet_family_trade.c` (2.8a), `ui.c`, `wormhole.c` (2.8c), comme G5 de 2.5.
- **G8. `sanitize-net` exigé à 2.8c seulement**, recommandé avant ; le plan le demande pour la ligne 2.8, et la propriété ne change dans aucune sous-étape.

### Surprises de mesure, pour `plan-pilot`

1. **Pied de 2.8a et 2.8b : 87 fichiers, pas 88.** `trade.h` n'a que des lignes `ptrs-entité` et `void-ptr` et sort dès 2.8a. Arrivée inchangée : 1073 / 85.
2. **Aucune liste non-entité.** Les sept tiennent des entités à l'écriture ; seules deux marques de TU (`static int`) et un témoin de TU n'en étaient pas.
3. **Deux TU rangeaient un `int` dans `ui.le`** (`test_pico8_ui.c:665`, `test_pico8_spec_defaults.c:60`) : exactement le cas que D.20b ne voyait plus ; le type le refuse maintenant.
4. **`make test` réécrit `tools/float_guard.txt`** (57 → 58 fichiers) dès qu'un module entre dans `CORE_SRCS` : la ligne va dans le commit 2.8a.
5. **Le remède de D.20b, 2 reste partiel** : `hv`/`cv`/`cg` sont encore des `void *` (amont R2 de `pico8_bm_has` et `pico8_dc`), sans catégorie qui les compte. À trancher par le pilote (ligne neuve après 2.7, ou rien : la Sortie de la phase ne l'exige pas).
6. **`L` de 99 lignes, aucune perte inchangée ; 13 lignes neuves non couvertes par les e2e**, toutes sans équivalent couvert au parent ou couvertes par un TU.

### Questions pour l'humain

Aucune.
