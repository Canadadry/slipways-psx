---
title: "Plan 3, étapes 2.17e et 2.17f : fin du démêlage (T6, T7) puis garde de l'usage ; `inclusion-mutuelle` 2 → 0 gardée, catégorie neuve `usage-mutuel` 0 gardée — deux commits (e, f)"
description: "2.17e : `pico8_na` passe dans `planet`, `pico8_gd` lit la caméra par `pico8_entity_geom_ra(pico8_mo())`, le registre `kv` passe dans `planet_family.c` sous le nom `pico8_jo_kv` (déclaré dans `planet.h`) ; retire `entity_geom.c:9`, `planet.c:18` et, devenu mort, `planet_family.c:12` ; `inclusion-mutuelle` entre dans la garde. 2.17f : catégorie `usage-mutuel` (portage `awk` de `usegraph.py`, arêtes orientées identiques à neuf commits), fixture à détour, deux mutations du script, entrée à 0 dans `CATEGORIES`, `SORTIE` et la garde. Mesuré sur l'arbre d'après 2.17d2 (rejoué par les scripts du PRD 2.17c–d, hachages vérifiés) : `inclusion-mutuelle` 2 / 3 (5 lignes, 4 fichiers) → 0, graphe d'usage 3 → 0, pied 5 / 4 → 0 / 0, aucune paire ni arête entrante ; objets : `pico8_na`, `pico8_jo_kv` identiques, seules `pico8_gd` et `pico8_jo_hu` (cible d'appel) changent ; 6 jumelles sans ÉCHEC ; garde `usage-mutuel` mordante sur un détour réel (`economy.c` → `pico8_jo_is_family`). Aucune question à l'humain."
status: needs-triage
---

## Problem Statement

**Écrit sur `4dcba1e`** (branche `claude/plan-pilot-3-7zdft0`). Départ réel attendu : **le commit de 2.17d2**, dont l'implémentation était en cours dans l'arbre de travail du dépôt pendant l'écriture (fichiers non lus). L'arbre des essais est `4dcba1e` (arbres de 2.17b : `src` `3c5385b8…`, `tests` `cac32597…`, `Makefile` `422546ca…`) + `run217c.py`, `run217d.py d1`, `run217d.py d2` (annexe A du PRD 2.17c–d, `docs/prd/triage/plan3-phase2-etape17cd-economie-declarations.md`), rejoués sur une copie : **`src` `9af24592…`, `ad162508…`, `b5a5cb59…`, `tests` `ea2eb691…`**, diffs `5821adac…`, `0954e753…`, cumul `777e6a21…` — exactement ceux que ce PRD annonce. Notation : `d2` = le commit de la copie qui porte 2.17d2 (départ) ; `e`, `f` = les deux commits de ce PRD. Numéros de ligne : ceux de `d2` pour 2.17e, de `e` pour 2.17f, sauf mention. Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier ; aucun `make` lancé dans le dépôt. Essais sur une seule copie jetable (scratchpad `prd217ef/`, clone, arbres de travail `wt_d2`, `wt_e`, `wt_f`, `c_d2`, `c_e` propres, `wt_r` pour les rejeux), supprimée depuis.

Vu du mainteneur du port : après 2.17d2, le compteur de composition ne compte plus que **2 paires d'inclusion mutuelle (5 lignes `#include`, 4 fichiers)** et le graphe d'usage 3 paires ; chacune ne tient qu'à un nom mal rangé :

- `entity_geom` ↔ `planet` : `pico8_na` (le balancement d'une planète, `entity_geom.c:55-65`) teste la famille et lit `pico8_jo_t` — une fonction de planète logée dans la géométrie générique.
- `entity_geom` ↔ `scroll` (cachée : pas d'include, `entity.h:5` → `scroll.h`) : `pico8_gd` lit la caméra par `pico8_mo_qb()->ra` (`:46-47`) alors que le lien `ra` du noyau donne la même valeur.
- `planet` ↔ `planet_family` : le registre `kv` (`g_kv`, `jo_kv`, `planet.c:278-298`) ne nomme que deux sortes de `planet_family`.

Et la garde d'`inclusion-mutuelle` ne voit pas une paire d'usage réintroduite par un détour (un nom reçu par un include transitif) : il faut une garde du graphe d'usage lui-même.

### Texte du plan, recopié (`docs/plan-3.md` à `4dcba1e`)

> - 2.17 — `inclusion-mutuelle`, selon l'étude 2.11 (`docs/etude-2-11-inclusions.md`, §2 à §4) : les quinze paires qui restent après 2.14b et 2.15d, et les trois paires d'usage cachées, coupées **par l'usage** (dans un sens, plus aucun nom de l'autre en-tête employé, et plus d'include) ; chaque sous-étape relance au départ et au commit le graphe d'usage de l'étude (annexe A) : les paires attendues sortent, aucune n'entre ; une ligne retirée qui compile par un détour est un `FAILED:`. […] `class-coverage-net` : non à chaque sous-étape (aucune table ni constructeur ne bouge). `sanitize-net` : non. Budget ≈ 6 × 50 min.
>   - 2.17e — après 2.14c et 2.15d, T6 et T7 : `pico8_na` dans `planet`, `pico8_gd` sur `pico8_entity_geom_ra(pico8_mo())`, registre `kv` dans `planet_family.c` (`pico8_jo_kv`, déclaré dans `planet.h`) ; retire `entity_geom.c:9`, `planet.c:19`. Départ : la table des symboles rejouée (restent `pico8_na`, `pico8_gd`, `kv`, sinon arrêt). Compte : → **0 paire, 0 ligne**, usage **0** ; `inclusion-mutuelle` gardé, garde prouvée mordante. Preuves : corps identiques à l'adresse près (`na`, `kv`) ; `pico8_gd` chemin touché.
>   - 2.17f — garde de l'usage : catégorie neuve `usage-mutuel` (portage `awk` du graphe d'usage de l'étude), fixture à détour, deux mutations du script, entrée à 0 dans `CATEGORIES`, `SORTIE` et la garde. Compte : `usage-mutuel` **0**, gardé ; pied inchangé. Preuves : fixture rouge puis verte, garde mordante (usage réintroduit par un détour sur une copie).
> - 2.18 — sortie de phase : `make composition-residue` rend `résidu 2 : 0 ligne(s), 0 fichier(s)`, les treize catégories dans `COMPOSITION_RESIDUE_GUARD` (dont `registres`, 2.10) (2.11 : quatorze, avec `usage-mutuel`, 2.17f) ; […] Budget ≈ 50 min.

Étude 2.11 : fiches 3, 6, c3 ; §4 (2.17e, 2.17f) ; techniques T6 (fonction déplacée vers le module de ses seuls appelants ou de ses seules données), T7 (lecture réécrite sur une fonction générique déjà là) ; essais `na_planet`, `kv_family`, `gd_mo` (annexe A de l'étude). Ce PRD en reprend les coupes, rejouées sur `d2` : les numéros ont bougé (2.14c, 2.15d, 2.17b–d), d'où un script neuf (annexe A).

Décisions de l'humain : **Q2** — « les paires d'inclusion mutuelle sont démêlées pendant la phase 2 ; `inclusion-mutuelle` est une condition de sortie, à zéro » : 2.17e la met à 0 et la garde, 2.17f garde aussi l'usage. **Q-2.10** (« oui », étude 2.10) : les fiches 3 et c3 en dépendaient ; 2.14b–c l'ont réalisé (liens `ra`, `ma` du noyau), la table des symboles rejouée le confirme (« État de départ »). Q1, Q3, Q4, **planètes A**, Q-2.14d : sans objet (aucun champ, lien ni table ne bouge).

Règles communes du plan : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte (`*_HASH`, `FROZEN_*`) changée ; `class-coverage-net` et `sanitize-net` « non » (lancés quand même à `e`, verts). Ce PRD les tient toutes.

### Contrats reçus et ce que ce PRD en fait

| Point (source) | Ce que font 2.17e et 2.17f |
|---|---|
| départ **2 / 3, 5 lignes / 4 fichiers, usage 3** (PRD 2.17c–d, contrat) | mesuré à `d2` : identique (table « État de départ ») |
| numéros à `d2` : `entity_geom.c:9` → `planet.h`, `planet.c:12` → `entity_geom.h`, **`:18` → `planet_family.h`** (plan : `:19`) ; `pico8_gd` à `entity_geom.c:44`, `pico8_na` à `:55`, `g_kv`/`jo_kv` à `planet.c:283-297`, `jo_kv` à `:289` (PRD 2.17c–d) | relus : exacts ; le bloc `kv` commence à `:278` (commentaire) |
| détour résiduel : `planet.c` voit `probe.h` par `planet_family.h:9`, relevé `planet` > `probe` vide (PRD 2.17c–d) | tombe avec `planet.c:18` ; relevé vide à `d2` et à `e` (0 nom, 0 include) ; le détour de preuve de l'étude (`pico8_g__ready()` appelé dans `planet.c`) **ne compile plus à `e`** (mesuré : `implicit declaration of function 'pico8_g__ready'`) |
| réserve de `0acfad8` : prendre un autre détour pour la garde (`economy.c` → `planet.h`, `pico8_jo_is_family`) ; garder la règle du nom déclaré par deux en-têtes | détour tenu : un appel `(void)pico8_jo_is_family(p);` dans `pico8_hy_lj` **compile à `f`** (par `economy_score.h` → `ui_button.h` → `planet.h`), `inclusion-mutuelle` reste 0, `usage-mutuel` 1 (`economy` ↔ `planet`), `make composition-guard` code 2 ; au parent `e`, la même mutation laisse la garde verte (preuve 7). Règle gardée : un nom que deux en-têtes déclarent n'est à aucun (fixture, mutation M1) ; mesuré : si le portage lisait les étiquettes de struct sans cette règle, les déclarations anticipées de `struct.h` feraient **6 paires** à `e` (preuve 8) |
| `pico8_fm.h`, `pico8_probe_comp.h` sans `.c` : `usegraph.py` les attribue au module `pico8_fm`/`pico8_probe_comp` ; le portage doit rendre les mêmes 3 paires à `d2` (PRD 2.17c–d) | tenu, et plus : arêtes orientées du portage **identiques** à `usegraph_e.py` à neuf commits (`9ca66f4`, `ced58a6`, `1ab94d6`, `0f2db15`, `b`, `c`, `d1`, `d2`, `e` : 636 à 615 arêtes, 0 écart) ; paires du script commité identiques à `usegraph.py` à huit commits (20, 18, 12, 10, 8, 3, 0, 0) (preuve 6) |
| réserve de `7d7f194` : mutation X2 et ligne de 81 colonnes | **déjà traitées à 2.13e2c** (`4f81553`) : X2 tuée par `test_ptrs_entite_keeps_one_line_number_in_two_files` (aujourd'hui `:382`), le commentaire de `:335` recoupé ; vérifié à `4dcba1e` : il reste **une** ligne de commentaire à 81 caractères, `:261` (« (L), chacune a sa ligne… »), que 2.17f recoupe (seule ligne du fichier touchée hors la fixture) |
| plan 2.17e : « corps identiques à l'adresse près (`na`, `kv`) ; `pico8_gd` chemin touché » | `fnbody.py` : `pico8_na` et `jo_kv`→`pico8_jo_kv` **corps identiques** sous `-O0`, `-O0 -DNDEBUG`, `-O1 -DNDEBUG` ; `pico8_gd` : couverture et 3 jumelles (preuves 3, 4, 5) |
| plan 2.17f : « fixture rouge puis verte, deux mutations, garde mordante » | fixture rouge avec le script de `e`, verte avec celui de `f` ; M1, M2 rouges ; garde mordante sur `src/` (preuves 6, 7) |

### État de départ attendu (le commit de 2.17d2)

Mesuré sur `d2` ; identique aux comptes d'arrivée du PRD 2.17c–d :

| Commande | Résultat |
|---|---|
| `git rev-parse HEAD:src HEAD:tests` ; `git ls-files -s Makefile` | `b5a5cb597049fbf0ab6c7d4cc952bbc30450fca2`, `ea2eb6913cc25857b60c0adfae142b8ef44c4d73` ; `422546ca868cfbdd089c8348b8eb4806763b9010` |
| `make composition-residue` | `inclusion-mutuelle 2 3` seule ; **`résidu 2 : 5 ligne(s), 4 fichier(s)`** |
| `--check inclusion-mutuelle src tests` | 5 lignes : `entity_geom.c:9`, `planet.c:12`, `:18`, `planet_family.c:16`, `planet_family.h:8` |
| `usegraph.py` | `paires 3` : `entity_geom` ↔ `planet` (2 : `pico8_jo_is_family`, `pico8_jo_t` / 2 : `pico8_gd`, `pico8_gk`), `entity_geom` ↔ `scroll` (1 : `pico8_qb_t` / 1 : `pico8_ma`), `planet` ↔ `planet_family` (2 : `pico8_ga_kind`, `pico8_pm_kind` / 13) |
| table des symboles (`sites.py` de l'étude) | `entity_geom` → `planet` : `entity_geom.c:58-59` (`pico8_na`) seuls ; `entity_geom` → `scroll` : `:46` (`pico8_gd`) seul ; `planet` → `planet_family` : `planet.c:284-285` (`g_kv`) seuls — **restent `pico8_na`, `pico8_gd`, `kv`**, la condition de départ du plan est tenue |
| `--champs \| tail -1` ; casts / tests de classe | `champs : 238 ; 0 écriture : 14 ; 0 hors constructeur : 19` ; 464 / 647 |
| arêtes orientées ; composantes fortement connexes > 1 | 627 ; une de 25 |
| `COMPOSITION_RESIDUE_GUARD` | 12 catégories, finit par `,parente,void-ptr` |
| `make test` ; `make e2e` ; `MALLOC_PERTURB_=165 make e2e` | vert ; 26 conformes ; 26 conformes |

Autre départ (2.17d2 commitée autrement que par son script, ou suivie d'un commit qui touche `src/`, `tests/`, `tools/` ou le `Makefile`) : repli 2.

## Solution

Deux commits, chacun vert, jeu identique (26 e2e conformes, aussi sous `MALLOC_PERTURB_=165`), aucune constante d'empreinte changée. Chaque commit est le résultat de son script (annexe A) — ou, ce qui revient au même, de `git apply` du diff de l'annexe B —, plus la note du plan et le message.

**2.17e** (`python3 run217e.py <arbre>` ; diff de 210 lignes, 7 fichiers, +47 / −48) :
1. **T6, `entity_geom` → `planet`** (fiche 3) — la définition de `pico8_na` (`entity_geom.c:55-65`) est coupée et collée telle quelle **en fin de `planet.c`** ; sa déclaration et son commentaire (`entity_geom.h:20-23`) passent **en fin de `planet.h`**. Le commentaire de `pico8_gk` (`entity_geom.h:14-15`), qui parlait aussi de « `na`'s `self` », ne parle plus que de `gk` (« `gk`'s `e` is NULL or a struct (asserted); its NULL `mp`/`rp` reads as the zero vector. »). `entity_geom.c` perd `#include "pico8_planet.h"` (`:9`, la coupe) ; `planet_family.c` perd `#include "pico8_entity_geom.h"` (`:12`, mort : il n'en employait que `pico8_na`, G2) ; `tests/test_pico8_fv.c`, qui appelle `pico8_na` (`:581`), gagne `#include "../src/pico8_planet.h"` (G4).
2. **T7, `entity_geom` → `scroll`** (fiche c3) — dans `pico8_gd`, les deux lignes `const pico8_qb_t *mo = pico8_mo_qb();` / `pico8_vec_t mo_ra = mo != NULL && mo->ra != NULL ? *mo->ra : pico8_entity_geom_zero();` deviennent `pico8_vec_t mo_ra = pico8_entity_geom_ra(pico8_mo());`. Même valeur dans les trois cas : `mo` NULL → zéro ; le provisoire de `entity.c` (`{{.cls = …}}`, lien `ra` NULL) → zéro ; un `qb` → `header.ra == &self->ra` (`scroll.c:39`), donc `*self->ra`, zéro si NULL. L'`assert` de classe de `pico8_mo_qb` (`entity.c:253`) n'est plus traversé par ce chemin. Aucun include ne bouge (`entity_geom.c` garde `entity.h` pour `pico8_mo`).
3. **T6, `planet` → `planet_family`** (fiche 6) — le bloc `kv` (`planet.c:278-298` : commentaire, table `g_kv`, fonction `jo_kv`) est coupé et collé **en fin de `planet_family.c`** ; `static const pico8_entity_kind_t *jo_kv(` devient `const pico8_entity_kind_t *pico8_jo_kv(` ; son commentaire « NULL when no kind has `fz`. » passe sur la déclaration, **en fin de `planet.h`** ; l'appel `planet.c:316` devient `pico8_jo_kv(hp_instance->cw)`. `planet.c` perd `#include "pico8_planet_family.h"` (`:18`, la coupe). `planet_family.c` inclut déjà `<string.h>`, `<stddef.h>`, `planet.h`.
4. `inclusion-mutuelle` entre dans `COMPOSITION_RESIDUE_GUARD` (`Makefile:874`, en fin de liste).

**2.17f** (`python3 run217f.py <arbre>` ; diff de 265 lignes, 3 fichiers, +159 / −10) :
5. **`tools/composition_residue.sh`** : catégorie neuve **`usage-mutuel`**, en fin de `CATEGORIES` et de `SORTIE` ; lecteur `read_usages` (portage `awk` de `usegraph.py`, 78 lignes d'`awk` sous le `CLEAN_AWK` existant) ; `collect` l'appelle ; la sortie traite les deux catégories de paires par la même branche (`[ -f "$work/$cat.pairs" ]`) : `<paires>\t<modules>` au compte, `<cat>\t<a>\t<b>` par paire au `--detail`, et au `--check` et au pied, **par paire et par sens, la ligne du premier emploi** (ordre de `$work/files`, puis des lignes). En-tête du script complété (trois lignes, deux reformulées).
6. **`Makefile`** : `usage-mutuel` en fin de `COMPOSITION_RESIDUE_GUARD` (14 catégories).
7. **`tests/test_composition_residue.c`** : un témoin `usage-mutuel` dans `test_one_occurrence_per_category_in_table_order` (paire `k` ↔ `l`, pied 13 → 15) ; fixture neuve `test_usage_pairs_see_a_detour_not_a_shared_name_nor_a_comment` (détour vu ; nom déclaré par deux en-têtes, commentaire et chaîne non vus ; `--check` des deux catégories) ; commentaire de `:261` recoupé à 80 colonnes (réserve de `7d7f194`).

Rien d'autre : aucune table de classe, aucun corps hors `pico8_gd` et la cible d'appel de `pico8_jo_hu`, aucun nom hors `pico8_jo_kv` (était `jo_kv`, statique) ; ni `*.sha256`, ni `docs/glossaire-noms.md`, ni `docs/trous-de-portage.md`, ni `tests/e2e/`. `tests/` : une ligne `#include` (e) et `test_composition_residue.c` (f). `tools/` : `composition_residue.sh` seul (f).

### Le portage `awk` (2.17f)

Même définition que `usegraph.py` (annexe A du PRD 2.11) : **a → b quand un `.c`/`.h` du module a (hors `tests/`) emploie un nom que `b.h` est seul à déclarer** ; paire quand a → b et b → a. Module = nom du fichier sans `.c`/`.h`. Correspondance, motif par motif (le texte exact est dans l'annexe B) :

| `usegraph.py` | `read_usages` |
|---|---|
| `strip` : commentaires `/* */`, `//`, chaînes → `""` | `clean()` de `CLEAN_AWK` (déjà employé par trois lecteurs) : commentaires, chaînes et caractères, commentaire ouvert porté à la ligne suivante |
| `top(s)` : texte hors accolades, les deux accolades omises | `tops[h]` : ligne par ligne, texte à profondeur 0 ; la fin de ligne à profondeur 0 devient une espace (python garde `\n` ; tous les motifs lisent `\s` ou une classe niée, sans différence) |
| `#define\s+(\w+)` sur tout le texte | `#define[ \t]+NOM` ligne par ligne |
| prototype `(\w+)\s*\([^()]*(?:\([^()]*\)[^()]*)*\)\s*;` sur `top`, hors 11 mots-clés | `[A-Za-z0-9_]+[ \t]*\(([^()]\|\([^()]*\))*\)[ \t]*;`, même liste de mots-clés |
| `\(\s*\*\s*(\w+)\s*\)` sur `top` | idem |
| `\}\s*(\w+)\s*;` sur `top` | **omis** : `top` ne garde aucune accolade, le motif ne rend jamais rien (mesuré : 0 nom à `e`) |
| `typedef\s[^;{}]*?(\w+)\s*;`, `extern\s[^;(]*?(\w+)\s*(?:\[[^\]]*\])?\s*;` (paresseux) | motif gourmand jusqu'au `;`, puis le dernier nom (après retrait d'un `[…]` final pour `extern`) : même nom, la classe niée arrêtant au premier `;` |
| énumérateurs `enum\s*\w*\s*\{([^}]*)\}` puis `(\w+)\s*(?:=[^,]*)?(?:,\|$)` | idem (ERE : `$` dans un groupe, accepté par `mawk` 1.3.4) |
| filtre : `''`, `PICO8`, `PICO8_` seul, noms en `__` | idem |
| `\b[A-Za-z_]\w*\b` (jetons) | suites de `[A-Za-z0-9_]` qui ne commencent pas par un chiffre (`0x1f` ne rend pas `x1f`, comme `\b`) |
| `len(owner[x]) == 1` | `nown[x] == 1` (`seen[x, h]` compte chaque en-tête une fois) |

Choix de sortie (G7) : `inclusion-mutuelle` rend les lignes `#include` ; `usage-mutuel` n'a pas d'include à montrer (le détour est justement l'include absent) : il rend, pour chaque sens de chaque paire, **le premier emploi**, c'est-à-dire une ligne qui dit quel nom a franchi la frontière. Le compte reste en paires / modules, comme `inclusion-mutuelle`. À `f`, 0 et 0 : le pied est inchangé (`0 ligne(s), 0 fichier(s)`).

Coût (mesuré) : la garde de `make test` passe de 0,78 s (13 catégories, `e`) à 1,04 s (14, `f`) ; `--check usage-mutuel` seul : 0,33 s.

### Comptes, mesurés sur la copie

| Mesure | `d2` (départ) | `e` | `f` |
|---|---|---|---|
| `inclusion-mutuelle` (paires / modules) | 2 / 3 | **absente (0)** | 0 |
| `--check inclusion-mutuelle` ; `cross.py` | 5 ; 5 | **0 ; 0** | 0 ; 0 |
| `usage-mutuel` (paires / modules) | — (catégorie absente ; le script de `f` rend 3 / 5 sur `d2`) | — (le script de `f` rend 0) | **absente (0)** |
| pied `résidu 2` (lignes / fichiers) | 5 / 4 | **0 / 0** | **0 / 0** |
| graphe d'usage (`usegraph.py`) | 3 | **0** | 0 |
| arêtes orientées ; composantes fortement connexes > 1 | 627 ; [25] | **623 ; [4, 19]** | 623 ; [4, 19] |
| catégories gardées (`COMPOSITION_RESIDUE_GUARD`) | 12 | **13** | **14** |
| `--champs` ; casts ; tests de classe | 238 ; 14 ; 19 ; 464 ; 647 | idem | idem |
| includes morts dans les fichiers touchés (`deadinc.py`) | — | parent 5 / enfant 5 (listes égales) | — |
| constantes `*_HASH`/`FROZEN_*` changées | — | 0 | 0 |

**Paires sortantes** à `e` : `entity_geom` ↔ `planet`, `planet` ↔ `planet_family` (compteur et usage), `entity_geom` ↔ `scroll` (usage seul). **Arêtes sortantes** (4) : `entity_geom` → `planet`, `entity_geom` → `scroll`, `planet` → `planet_family`, `planet_family` → `entity_geom` (il n'employait que `pico8_na`). **Entrantes : aucune** (paire, arête, include mort). À `f`, `src/` ne change pas (arbre `452f895c…` aux deux commits).

**Lignes du `--check`** à `e` : sortent les 5 — `entity_geom.c:9`, `planet.c:18` (coupées) ; `planet.c:12` → `entity_geom.h`, `planet_family.c:16` et `planet_family.h:8` → `planet.h` (gardées : les sens naturels, la paire n'existe plus).

**Cycle** (hors périmètre, signalé) : la composante de 25 modules se scinde en 4 + 19 (23 modules en cycle) : couper `planet` → `planet_family` et `entity_geom` → `planet` ôte deux retours. Ce n'est pas une paire ; l'étude (§3) exclut les cycles longs de la Sortie.

**Repli e1** (mesuré, `run217e.py . e1` : `pico8_na` et `pico8_gd` seuls) : 0 erreur, `inclusion-mutuelle` **1 / 2, 3 lignes / 3 fichiers**, usage **1** (`planet` ↔ `planet_family`) ; e2 fait le reste.

### Ce que 2.18 aura à constater (contrat)

| Point | Attendu à `f` (mesuré) | Commande |
|---|---|---|
| pied vide | `résidu 2 : 0 ligne(s), 0 fichier(s)`, aucune ligne de catégorie | `make composition-residue` |
| **quatorze** catégories gardées | `noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6,lien-parent,parente,void-ptr,inclusion-mutuelle,usage-mutuel` ; `make composition-guard` « : 0 ligne(s) » ; `CATEGORIES` = `SORTIE` = ces 14 | `grep COMPOSITION_RESIDUE_GUARD Makefile` ; `grep -c` des deux listes |
| aucune paire d'usage | `usegraph.py` : `paires 0` ; `usage-mutuel` absent du `--detail` | — |
| test ponctuel | `tests/test_pico8_archetype_chain.c` absent (retiré à 2.13e2c) : vérifié à `4dcba1e` | `ls tests` |
| archétypes | 26 déclarations `extern const pico8_*_def_t pico8_*_def` dans `game_data.h` (dont `pico8_probe_def_none`) à `4dcba1e` ; 2.18 les recompte et relit les 14 tables dispensées (étude 2.9, §2) | — |
| filets | `class-coverage-net` identique à `tools/class_coverage_net_expected.txt` ; `sanitize-net` 26 propres (mesurés à `e` ; `src/` identique à `f`) | — |
| revue de sortie | `warn-review` (lancée par le pilote) | — |
| restes signalés, hors Sortie | cycle du graphe d'usage (composantes [4, 19]) ; includes préexistants à 0 nom (`deadinc.py` : `planet.c` → `entity.h`, `planet.h` → `geom.h`, `ke.h`, `planet_family.c` → `collect.h`, `slipway.h`) ; noms provisoires `pico8_kr`, `pico8_jo_kv`, `pico8_fm.h`, `pico8_probe_comp.h` (phase 3) | — |

### Anti-tassement

**Interdit** :
- retirer une ligne `#include` sans retirer l'usage (un détour, que le relevé `usedir.py` doit rendre vide), redéclarer `pico8_na` ou `pico8_jo_kv` localement (prototype, `extern`), garder `jo_kv` statique dans `planet.c` en recopie ;
- changer un corps hors `pico8_gd` (et l'appel de `pico8_jo_hu`), un commentaire déplacé (hors celui de `pico8_gk` et le déplacement de « NULL when no kind has `fz`. » vers la déclaration) ; toucher une table de classe ;
- dans le portage, lire les étiquettes de struct, compter un nom déclaré par deux en-têtes, lire `tests/`, ou compter un emploi en commentaire ou en chaîne ; rendre `usage-mutuel` en lignes `#include` ;
- toucher un `*.sha256`, `tests/e2e/`, une constante `*_HASH`/`FROZEN_*`, `docs/glossaire-noms.md`, `docs/trous-de-portage.md` ; toucher `tests/` hors la ligne de `test_pico8_fv.c` (e) et `test_composition_residue.c` (f) ; toucher `tools/` hors `composition_residue.sh` (f) ;
- fondre e et f ; réécrire à la main ce que les scripts font, sauf repli.

## User Stories

1. En tant que mainteneur, je veux que la géométrie générique (`entity_geom`) n'emploie plus ni la planète ni la caméra concrète, afin qu'elle ne dépende que du noyau.
2. En tant que mainteneur, je veux que `planet` n'emploie plus `planet_family`, afin que les sortes dépendent de la planète et pas l'inverse.
3. En tant que gardien du compteur, je veux `inclusion-mutuelle` à 0 et gardée, et une garde qui rougit quand une paire d'usage revient par un détour — sans rougir sur une déclaration anticipée partagée.
4. En tant que gardien de la règle du parent, je veux que chaque mutation rouge au parent le soit à l'enfant, et que les corps déplacés restent identiques.
5. En tant qu'auteur de 2.18, je veux la liste de ce qui doit être constaté à la sortie de phase, mesurée.

## Implementation Decisions

### Modules

2.17e : `entity_geom.c/.h`, `planet.c/.h`, `planet_family.c`, `tests/test_pico8_fv.c`, `Makefile` ; `docs/plan-3.md` (note « Découvert à 2.17e »). 2.17f : `tools/composition_residue.sh`, `tests/test_composition_residue.c`, `Makefile` ; `docs/plan-3.md` (note « Découvert à 2.17f », qui clôt la ligne 2.17).

### Inclusions

| Commit | Retirées | Ajoutées |
|---|---|---|
| 2.17e | `entity_geom.c:9` (`planet.h`), `planet.c:18` (`planet_family.h`) — les coupes ; `planet_family.c:12` (`entity_geom.h`, mort, G2) | `tests/test_pico8_fv.c` : `../src/pico8_planet.h` (G4) |
| 2.17f | — | — |

**Aucun include mort créé** (`deadinc.py` sur les 6 fichiers de `e`, au parent et à l'enfant) : 5 → 5, mêmes lignes (préexistantes : `planet.c` → `entity.h`, `planet.h` → `geom.h`, `ke.h`, `planet_family.c` → `collect.h`, `slipway.h`). Premier jet sans G2 : `planet_family.c` → `entity_geom.h` entrait (6), d'où le retrait.

### Ce qui change

Texte exact : résultat des scripts (annexe A), identique aux diffs de l'annexe B. Sur un arbre propre au commit de 2.17d2 (arbres de la table « État de départ »), après `git add -A` :

| Commit | Commande | `src` | `tests` | autres | diff (`sha256sum`, lignes) |
|---|---|---|---|---|---|
| 2.17e | `python3 run217e.py .` | **`452f895c9c1d411aaba7c5497412af7570bc892a`** | **`d78e7efb0ed5b8d3490c5e8eb1e9353ef89d9e80`** | `Makefile` **`2f29559961ed16624c310053846b305b0fca998b`** ; `tools` `e6db806c…` (inchangé) | `2a848d9c…`, 210 |
| 2.17f | `python3 run217f.py .` | `452f895c…` (inchangé) | **`9f85799194704e9ab37d4e9485962e7df2608d4e`** | `Makefile` **`0b08417435e21d9dbe6268dd5807a6d6b37920f5`** ; `tools` **`f5677a3859b82571accdae8b07176ba26a35cdc8`** | `40131301…`, 265 |

Cumul `git diff 4dcba1e f | sha256sum` → `78546d6a…` (50 fichiers, +402 / −208, avec 2.17c–d). Rejoués sur un arbre de travail neuf à `4dcba1e` (`run217c.py`, `run217d.py d1`, `d2`, `run217e.py`, `run217f.py`) : mêmes hachages ; relancer `run217e.py` ou `run217f.py` sur l'arrivée s'arrête (`AssertionError`) **avant toute écriture** (Runs 24).

## Implementation Chunks

Deux commits, **≈ 45 min** chacun preuves comprises (mesuré sur la copie : script < 1 s ; `make -j4 test` + `make e2e` + e2e perturbé 12 à 16 s par arbre ; objets 4 configurations × 2 arbres en parallèle 7 s ; `fnbody.py` quelques secondes ; couverture ≈ 6 s par arbre ; 6 jumelles × 2 côtés en parallèle ≈ 90 s ; `class-coverage-net` + `sanitize-net` 8 s ; `usecmp.sh` 9 commits ≈ 5 s ; TU du compteur ≈ 3 s par variante) :

- **2.17e** : copier `run217e.py` hors du dépôt, le lancer, comparer les hachages ; `tests.sh` ; comptes (`counts.sh`, `usedir.py`, `sites.py`, `deadinc.py`) ; objets (`objs2.sh` ×4 à `d2` et `e`, `cmp`, `fnbody.py`, `objcopy --redefine-sym`) ; couverture `d2`, `e` + `lset.py` ; jumelles (`muts217egen.py`, `mut217e.py d2` et `e`) ; garde mordante ; `nets.sh` ; note, message.
- **2.17f** : `run217f.py .` (ou `git apply` de l'annexe B) ; `tests.sh` ; fixture seule avec le script de `e` puis de `f` ; M1, M2 ; `usecmp.sh` ; script de `f` sur les huit commits ; garde mordante (détour `economy.c`) à `f` et au parent ; note qui clôt 2.17.

**Replis écrits**, dans l'ordre :
1. si un script s'arrête sur un `AssertionError` (texte introuvable) : lire le site ; faire la même modification à la main selon la même règle (bloc coupé et collé tel quel, include retiré seulement après relevé vide, texte du script de `f` recopié depuis l'annexe B) ; noter chaque site dans le message ;
2. si le départ n'est pas l'arbre de 2.17d2 annoncé : lancer les scripts quand même, ne pas comparer les hachages d'arrivée, comparer tous les comptes des tables et rapporter l'écart dans la note ;
3. si une erreur de compilation reste (`syn.sh` non vide) : ajouter **au fichier en erreur** l'include de l'en-tête qui déclare le nom (`pico8_planet.h`…), jamais remettre une ligne coupée ; relancer `usegraph.py` et le compteur : si une paire entre, arrêter ;
4. si le relevé d'un sens coupé n'est pas vide, si une paire ou une arête entre, ou si `deadinc.py` trouve un include mort de plus qu'au parent : ne pas commiter ; arrêter et rapporter ;
5. si 2.17e déborde de 1,5 × 50 min : la couper en **e1** (`run217e.py . e1` : `pico8_na`, `pico8_gd` ; mesuré 1 / 2, 3 / 3, usage 1) puis **e2** (le reste, garde comprise) ;
6. si le vérificateur refuse la ligne ajoutée à `test_pico8_fv.c` : la retirer (le TU compile sans, par `ui_button.h` → `planet.h` : mesuré) et le noter ;
7. si `awk` sur la machine de l'implémenteur n'est pas `mawk` et que `usecmp.sh` ou la fixture diffère : ne pas commiter f ; rapporter la version d'`awk` et l'écart (le portage n'emploie que de l'ERE POSIX, `match`, `sub`, `gsub`, `split`, et des affectations `pass=1` en argument).

## Testing Decisions

- **Pas de TU de jeu neuf.** 2.17e ne touche `tests/` que d'une ligne `#include` ; 2.17f n'écrit que dans le TU du compteur (fixtures fabriquées, aucun rejeu d'`events.json`). Aucun `*_HASH` ne bouge.
- **Bon test ici** : une fonction déplacée se prouve par le code machine (corps identique) ; une lecture réécrite (`pico8_gd`) par le chemin touché — couverture des lignes, jumelles à la règle du parent sur ce qui pourrait glisser (la caméra lue, son champ) ; un compteur neuf par une fixture qui rougit au parent et sous chaque mutation du lecteur, par l'équivalence avec l'outil qu'il porte sur des arbres réels, et par une garde qui mord sur un vrai détour de `src/`.
- **Prior art** : PRD 2.17c–d et 2.17a–b (forme, `usedir.py`, `fnbody.py`, `objs2.sh`, `deadinc.py`, `tests.sh`, `cov.sh`, `nets.sh`, `counts.sh`) ; PRD 2.15d (`syn.sh`, `mut.py`) ; PRD 2.15c (`lset.py`) ; PRD 2.11 (`usegraph.py`, `cross.py`) ; étude 2.11 (`sites.py`, `scc.py`, `na_planet`, `kv_family`, `gd_mo`) ; 2.13e2c (garde mordante d'une catégorie, X2) ; règle du parent (D.21).

### Preuves (parent de `e` = `d2`, de `f` = `e`)

1. **Communes** : `make -j4 test` vert ; `make e2e` **26 conformes** ; `MALLOC_PERTURB_=165 make e2e` **26 conformes**, à `d2`, `e`, `f` ; `git diff d2 f -- '*.sha256' tests/e2e` vide ; `git diff d2 f | grep -cE '^[-+].*(_HASH|FROZEN_)'` → 0 (Runs 5, 17).

2. **Coupe par l'usage, sens par sens** (`usedir.py` ; noms que l'en-tête B est seul à déclarer et que A emploie ; noms partagés : 0 partout) :

| Sens coupé | `d2` | `e` |
|---|---|---|
| `entity_geom` > `planet` | 2 : `pico8_jo_is_family`, `pico8_jo_t` ; include `entity_geom.c` | **0 ; aucun** |
| `entity_geom` > `scroll` | 1 : `pico8_qb_t` ; (aucun include : détour `entity.h` → `scroll.h`) | **0 ; aucun** |
| `planet` > `planet_family` | 2 : `pico8_ga_kind`, `pico8_pm_kind` ; include `planet.c` | **0 ; aucun** |
| `planet` > `probe` (détour résiduel, contrôle) | 0 ; aucun | 0 ; aucun |

   Sens gardés, à `e` : `planet` > `entity_geom` 2 (`pico8_gd`, `pico8_gk`) ; `scroll` > `entity_geom` 1 (`pico8_ma`) ; `planet_family` > `planet` 13 → **15** (`pico8_jo_kv`, `pico8_na` entrent) (Runs 4, 9).

3. **Graphe d'usage et compteur, aucune paire entrante** : `usegraph.py` 3 → 0 ; `--detail` 2 → 0 ; arêtes 627 → 623, seules des sorties (liste plus haut) ; `syn.sh` (gcc) et le même jeu sous `clang` : 0 erreur à `e` (Runs 7, 8).

4. **Objets** (`objs2.sh` : les 74 `src/*.c` hors raylib, sans `-g`, `-DPICO8_CLASS_COVERAGE` ; `cmp` ; `fnbody.py` ; `jo_kv` renommé `pico8_jo_kv` dans une copie des objets de `d2` par `objcopy --redefine-sym` pour comparer son corps) :

| Configuration | `d2` → `e` |
|---|---|
| `-O0 -DNDEBUG` (celle de `make test`/`e2e`) | **71 / 74** objets identiques (`entity_geom`, `planet`, `planet_family` diffèrent) ; `pico8_na` : `entity_geom` → `planet`, **corps identique** ; `jo_kv` (`t`) → `pico8_jo_kv` (`T`, `planet_family`), **corps identique** ; 2 fonctions différentes : `pico8_gd` (l'appel à `pico8_mo_qb` et ses tests remplacés par `pico8_mo` + `pico8_entity_geom_ra`), `pico8_jo_hu` (un `call <jo_kv>` local devient un appel relogé à `pico8_jo_kv`) ; 836 autres identiques ; 1247 relocations de données, 8 différentes : les 4 entrées de `g_kv` changent de module (`"pm"`, `"ga"`, `pico8_pm_kind`, `pico8_ga_kind`) |
| `-O0` avec `assert`, `NORM_ASSERT=1` | idem (les autres écarts bruts sont des `__LINE__` de `entity_geom.c`, `planet.c`, `planet_family.c`) |
| `-O1 -DNDEBUG` | 71 / 74 ; `pico8_na` corps identique ; `jo_kv` était intégré à `pico8_jo_hu` à `d2` (statique, un appelant) : `pico8_jo_hu` et `pico8_gd` diffèrent, `pico8_jo_kv` neuve ; 728 autres identiques |
| `-O2 -DNDEBUG` | 71 / 74 ; `pico8_na`, `pico8_gd`, `pico8_gk`, `pico8_jo_hu`, `pico8_jo_hk_dispatch`, `pico8_ga_new` diffèrent par intégration (`pico8_entity_geom_ra` n'est plus intégrée partout) ; 717 autres identiques ; données 1242 / 1242 identiques |
| `nm` | `pico8_na` `T` de `entity_geom.o` à `planet.o` ; `jo_kv` `t` (`planet.o`) → `pico8_jo_kv` `T` (`planet_family.o`) ; `entity_geom.o` perd `U pico8_fixed_add`, `U pico8_fixed_sin` (partis avec `pico8_na`) et `U pico8_mo_qb`, gagne `U pico8_mo` |

   Lecture : les deux corps déplacés sont identiques ; seules changent les deux fonctions que la coupe nomme (Runs 10, 11).

5. **Mutations, règle du parent** (`mut217e.py` = `mut.py` du PRD 2.15d avec `S` et le nom du JSON changés ; `muts217e.json` écrit par `muts217egen.py`, annexe A). Même effet au parent et à l'enfant, chacun sur son texte ; `make -k -j4 unit` puis `make -k e2e` ; restaurer par `git checkout -f`. **ÉCHEC si un TU rouge au parent n'est pas rouge à l'enfant, ou si l'e2e est < 26 au parent et 26 à l'enfant.**

| # | Cible (`d2` → `e`) | Mutation | Rouges au parent | Rouges à l'enfant | e2e | Verdict |
|---|---|---|---|---|---|---|
| GD-zero | caméra de `pico8_gd` | zéro rendu pour un `qb` (plan) | `fr`, `g_`, `ga`, `hj`, `hp`, `hy`, `jc`, `lq`, `mx`, `nr`, `planet_kj_dispatch`, `pm` | les mêmes 12 | 0 / 0 | conforme |
| GD-p | idem | `p` lu au lieu de `ra` (plan) | aucun | aucun | 26 / 26 | **survivante aux deux bouts**, déclarée : `ra = ma(p, self)` (`scroll.c:124`) ne s'écarte de `p` que pendant une secousse, et la marge de `gd` est de ±76 |
| GD-self | idem | le `ra` de l'entité lu à la place de la caméra | `g_`, `jc`, `mo` | les mêmes 3 | 26 / 26 | conforme (remplace GD-p comme témoin du champ lu) |
| NA-sub | `pico8_na` (`entity_geom.c` → `planet.c`) | `pico8_fixed_add` → `pico8_fixed_sub` | `fr`, `hj`, `mx`, `nl`, `nr`, `os`, `pm`, `qh` | les mêmes 8 | 17 / 17 | conforme |
| KV-swap | `g_kv` (`planet.c` → `planet_family.c`) | `{"pm", pico8_ga_kind}` | `hp`, `jo`, `qh` | les mêmes 3 | 25 / 25 | conforme |
| KV-miss | `jo_kv` → `pico8_jo_kv` | `== 0` → `!= 0` | `ga`, `hp`, `jo`, `qh` | les mêmes 4 | 24 / 24 | conforme |

   Lecture : **aucun ÉCHEC** (6 jumelles, 12 lancements ; GD-self ajoutée après la survivante GD-p) (Run 14).

6. **Chemin touché, ensemble `L`** (`make line-coverage EVENTS="<les 26 e2e>"`, `lset.py`) : `d2` **6034**, `e` **6033** lignes. **0 gagnée ; 1 « perdue »** = artefact d'alignement : `entity_geom.c:65` (l'accolade fermante de `pico8_na`, comptée par `gcov` pour une fonction `void`) est appariée par `difflib` à l'accolade de `pico8_gd` (après `return`, jamais comptée) ; l'accolade de `pico8_na` à `e`, `planet.c:393`, est couverte. `L` **13** (`entity_geom.c:46-47`, `:55`, `:58-61`, `:63`, `planet.c:289`, `:292-294`, `:316`) ; lignes neuves couvertes **13** (`entity_geom.c:45`, `planet.c:293`, `:383`, `:386-389`, `:391`, `:393`, `planet_family.c:661`, `:664-666`) ; non couvertes : déclarations, commentaires, lignes vides, accolades, la table `g_kv` (données) et `return NULL;` (`planet_family.c:669`, déjà non couvert à `d2`, `planet.c:296`) (Run 12).

7. **Gardes mordantes** :
   - `inclusion-mutuelle` à `e` : `#include "pico8_planet.h"` remis dans `entity_geom.c` → `make composition-guard` **code 2** (`inclusion-mutuelle: src/pico8_entity_geom.c:9:…`, `planet.c:12:…`, 2 lignes) ; retiré, vert (Run 15).
   - `usage-mutuel` à `f`, **détour réel** (réserve de `0acfad8`) : `(void)pico8_jo_is_family(p);` ajouté après `pico8_fixed_t kr = pico8_kr(p);` dans `pico8_hy_lj` (`economy.c:226`) ; compile (`-Wall -Wextra -Werror`), `economy.c` reçoit `planet.h` par `economy_score.h` → `ui_button.h` → `planet.h` sans l'inclure ; `--detail` : `usage-mutuel pico8_economy pico8_planet`, pas d'`inclusion-mutuelle` ; `make composition-guard` **code 2**, lignes `usage-mutuel: src/pico8_economy.c:228:                (void)pico8_jo_is_family(p);` et `src/pico8_planet.c:309:    pico8_hy_nx(pico8_fm_hy(), fp);` ; au parent `e`, la même mutation : garde **verte** (13 catégories, 0 ligne), `usegraph.py` 1 paire — c'est la garde neuve qui mord (Runs 20, 21).

8. **Fixture et mutations du script** (2.17f ; TU `test_composition_residue`, construit et lancé seul) :

| Variante | Résultat |
|---|---|
| script de `f` | vert (`all tests passed`) |
| script de `e` (parent), TU de `f` | **rouge** : `:101` (témoin de table) ; la fixture seule (`main` réduit à elle, `fixture_only.c`) rouge à `:618` avec le script de `e`, verte avec celui de `f` |
| **M1** « nom partagé compté » : `\|\| !(x in nown) \|\| nown[x] != 1 \|\| own[x] == a` → `\|\| !(x in nown) \|\| own[x] == a` | **rouge** `:618`, trois lancements sur trois (la fixture ne dépend pas de l'ordre de parcours de `awk` : `u` emploie `s_t`, que `d.h` et `e.h` déclarent, et `d`, `e` emploient tous deux `u`) |
| **M2** « commentaires non retirés » : `l = clean($0)` → `l = $0` (lecture des emplois) | **rouge** `:618` (`f` → `g` par le commentaire et la chaîne) |

   Sur l'arbre réel à `f` : M1 laisse 0 paire (aucun nom n'est déclaré par deux en-têtes quand les étiquettes ne sont pas lues) ; M2 fait **13 paires / 18 modules** (des noms dans les commentaires). Mesuré avec `usegraph.py` étendu aux étiquettes `struct\s+(\w+)` : **6 paires** sans la règle du partage (`game_data` ↔ `struct`, `particle` ↔ `struct`, `planet` ↔ `planet_trade`, `planet` ↔ `struct`, `planet_trade` ↔ `struct`, `probe_comp` ↔ `struct`, toutes par les déclarations anticipées `pico8_kind_rec`, `pico8_str_seq`, `pico8_emitter`, `pico8_qy`, `pico8_jo_trade_proposal`, `pico8_probe`), **0** avec : c'est la raison de la règle, écrite dans le commentaire du lecteur (Runs 18, 19, 22).

9. **Équivalence du portage** (`usecmp.sh`, annexe A : le programme `awk` seul, plus un `END` qui imprime les arêtes, contre `usegraph_e.py`) : arêtes orientées **identiques** (0 écart) à `9ca66f4` (636, 20 paires), `ced58a6` (636, 20), `1ab94d6` (622, 18), `0f2db15` (622, 18), `b` (617, 12), `c` (615, 10), `d1` (619, 8), `d2` (627, 3), `e` (623, 0). Le script commité de `f`, lancé sur les arbres de ces commits : paires de `--detail` identiques à `usegraph.py` (hachage de la liste triée) à `9ca66f4`, `0f2db15`, `b`, `c`, `d1`, `d2`, `e`, `f` (20, 18, 12, 10, 8, 3, 0, 0) (Runs 16, 23).

10. **Reproductibilité** : les cinq scripts sur un arbre de travail neuf à `4dcba1e` → `b5a5cb59…`, puis `452f895c…` / `d78e7efb…` / `2f295599…`, puis `9f857991…` / `f5677a38…` / `0b084174…`, cumul `78546d6a…` ; `run217e.py` et `run217f.py` relancés sur l'arrivée : `AssertionError`, arbre inchangé (Run 24).

11. `class-coverage-net` : **non** exigé ; lancé à `e` : `couverture: OK -- chaque classe non exemptée est instanciée (26 rejeu(x))`. `sanitize-net` : **non** exigé ; lancé à `e` : `sanitize: 26 propre(s)` ; `src/` de `f` = `src/` de `e` (Run 13).

### Greps de Sortie

À `e` :
1. `grep -n 'pico8_na(' src/*.c src/*.h` → 4 lignes : `planet.c:383` (définition), `planet.h:155`, `planet_family.c:25`, `:85` ; `grep -c 'pico8_na' src/pico8_entity_geom.[ch]` → 0 et 0.
2. `grep -n 'pico8_jo_kv\|jo_kv(' src/*.c src/*.h` → 3 lignes : `planet.c:293` (appel), `planet.h:158`, `planet_family.c:661` (définition) ; `grep -c 'g_kv' src/pico8_planet.c` → **0**.
3. `grep -n 'pico8_mo_qb\|pico8_qb_t\|pico8_jo_' src/pico8_entity_geom.c` → vide ; `grep -n 'pico8_entity_geom_ra(pico8_mo())' src/pico8_entity_geom.c` → `:45`.
4. `grep -c '#include "pico8_planet.h"' src/pico8_entity_geom.c` → 0 ; `grep -c '#include "pico8_planet_family.h"' src/pico8_planet.c` → 0 ; `grep -c '#include "pico8_entity_geom.h"' src/pico8_planet_family.c` → 0.
5. `make composition-residue` → `résidu 2 : 0 ligne(s), 0 fichier(s)` seul ; `grep -c 'void-ptr,inclusion-mutuelle$' Makefile` → 1 ; `python3 usegraph.py .` → `paires 0`.

À `f` :
6. `grep -c 'usage-mutuel' tools/composition_residue.sh` → **13** ; `grep -o 'inclusion-mutuelle,usage-mutuel$' Makefile` → 1 ligne ; `sh tools/composition_residue.sh --check usage-mutuel src tests` → `résidu gardé (usage-mutuel) : 0 ligne(s)`, code 0.
7. `grep -c 'test_usage_pairs_see_a_detour_not_a_shared_name_nor_a_comment' tests/test_composition_residue.c` → 2.
8. `make composition-residue` → `résidu 2 : 0 ligne(s), 0 fichier(s)` ; `make composition-guard` → `résidu gardé (…,inclusion-mutuelle,usage-mutuel) : 0 ligne(s)`.

Aux deux : 9. `git diff --stat <parent>` : les fichiers de la section « Modules », plus `docs/plan-3.md`, rien d'autre.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable (`prd217ef/copie`, arbres de travail `wt_d2`, `wt_e`, `wt_f`, `c_d2`, `c_e`, `wt_r`).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout 4dcba1e` ; `git rev-parse` des arbres | `b` | 0 | `3c5385b8…`, `cac32597…`, `422546ca…` (= 2.17b) | 3 s |
| 2 | `extract.py` : 20 scripts tirés des annexes des PRD 2.11, 2.15c, 2.15d, 2.17a–b, 2.17c–d et de l'étude ; `usedir.py`, `usegraph_e.py` montés comme au PRD 2.17a–b | — | 0 | `cross.py`, `usegraph.py`, `deadinc.py`, `objs2.sh`, `fnbody.py`, `tests.sh`, `cov.sh`, `nets.sh`, `run217c.py`, `run217d.py`, `counts.sh`, `syn.sh`, `scc.py`, `sites.py`, `mut.py`, `lset.py`… | < 1 s |
| 3 | `run217c.py`, `run217d.py d1`, `d2` ; hachages | `c`, `d1`, `d2` | 0 | `9af24592…`, `ad162508…`, `b5a5cb59…`, `ea2eb691…` ; `5821adac…`, `0954e753…`, cumul `777e6a21…` (ceux du PRD 2.17c–d) | < 1 s |
| 4 | compteur, `--check`, `usegraph.py`, `sites.py` | `d2` | 0 | `2 3`, `5 / 4`, 5 lignes, `paires 3` ; restent `pico8_na`, `pico8_gd`, `g_kv` seuls | 1 s |
| 5 | `run217e.py` premier jet (sans G2, sans garde) ; `syn.sh` ; compteur ; `usegraph.py` ; `usedir.py` | `e` | 0 ; 1 | premier appel : `AssertionError` (fin de `pico8_na` non unique, ancre corrigée) ; puis 0 erreur, `0 / 0`, `paires 0`, sens coupés vides | 2 s |
| 6 | `deadinc.py` parent / enfant (6 fichiers) | `d2`, `e` | 0 | **5 → 6** : `planet_family.c` → `entity_geom.h` devient mort → script complété (G2) ; puis 5 → 5 | < 1 s |
| 7 | `syn.sh` (gcc) ; même jeu sous `clang` ; `test_pico8_fv.c` sans la ligne ajoutée | `e` | 0 | 0 erreur ×2 ; le TU compile sans (G4, repli 6) | 5 s |
| 8 | + garde dans le `Makefile` ; script définitif ; hachages | `e` | 0 | `452f895c…`, `d78e7efb…`, `2f295599…` ; `2a848d9c…`, 210 lignes | < 1 s |
| 9 | `usedir.py` (7 sens) ; arêtes `d2`/`e` ; `scc.py` | `d2`, `e` | 0 | preuve 2 ; 4 arêtes sortent, 0 entre ; [25] → [4, 19] | 1 s |
| 10 | `objs2.sh` ×4 ×2 ; `cmp` ; `fnbody.py` (`NORM_ASSERT`, `FNDIFF`) | `d2`, `e` | 0 | 71 / 74 ×4 ; `pico8_na` corps identique ; `pico8_gd`, `pico8_jo_hu` | 7 s + 5 s |
| 11 | `objcopy --redefine-sym jo_kv=pico8_jo_kv` sur `planet.o` de `d2` ; `fnbody.py` ×4 | `d2`, `e` | 0 | `pico8_jo_kv` : `planet` (`l`) → `planet_family` (`g`), corps identique (`-O0`, `-O0 -DNDEBUG`) ; intégrée à `-O1`, `-O2` | 3 s |
| 12 | `cov.sh d2 e` ; `lset.py` | `c_d2`, `c_e` | 0 | 6034 / 6033 ; `L` 13 ; 13 neuves couvertes ; 1 « perdue » (artefact d'accolade) | 6 s ×2 |
| 13 | `nets.sh e` | `c_e` | 0 | `OK … (26 rejeu(x))` ; `26 propre(s)` | 8 s |
| 14 | jumelles `GD-zero`, `GD-p`, `NA-sub`, `KV-swap`, `KV-miss` ×2 ; puis `GD-self` ×2 | `wt_d2`, `wt_e` | 0 | preuve 5 ; **GD-p survit aux deux bouts** → `GD-self` ajoutée | 66 s + 25 s |
| 15 | garde mordante `inclusion-mutuelle` (include remis dans `entity_geom.c`) | `wt_e` | 2 ; 0 | 2 lignes ; restauré, vert | 2 s |
| 16 | `usage.awk` (portage seul) + `usecmp.sh` : premier jet | `e`, `d2` | 2 | `mawk: illegal reference to variable depth` (nom déjà pris par `CLEAN_AWK`) → `hdepth` | < 1 s |
| 17 | `tests.sh wt_d2 wt_e`, puis `wt_f` (deux fois) | 3 | 0 ×9 | vert ; 26 ; 26 | 16, 15, 12, 7 s |
| 18 | `run217f.py` premier jet ; TU seul ; M1 | `f` | 0 ; 0 | vert, mais **M1 survit** (fixture `d` ↔ `e` : l'en-tête gardé par `own[]` dépend de l'ordre de `for … in`) → fixture refaite (`u` employé par `d` et `e`) | 3 s |
| 19 | fixture refaite : premier essai | `f` | 134 | rouge à `:618` : `u.c` incluait `d.h` → paire d'**inclusion** `d` ↔ `u` → include retiré de la fixture | 3 s |
| 20 | TU : script de `f`, de `e`, M1 ×3, M2, restauré ; `fixture_only.c` avec les scripts de `e` et `f` | `wt_f` | 0 ; 134 ; 134 ×3 ; 134 ; 0 ; 134 / 0 | preuve 8 | 3 s chacun |
| 21 | garde mordante `usage-mutuel` (détour `economy.c`) à `f`, puis au parent `e` | `wt_f`, `wt_e` | 2 ; 0 | preuve 7 | 2 s ×2 |
| 22 | M2, M3 (étiquettes lues), M3 + M1 sur l'arbre réel ; `usegraph_sh.py`, `usegraph_tags.py` (`TAGS=1`) à `d2` et `e` | `wt_f`, `d2`, `e` | 0 | M2 13 / 18 ; M3 0 ; M3 + M1 0 (propriétaire unique, dépend de l'ordre) ; python, étiquettes sans la règle : 6 paires de plus à `e` comme à `d2` ; aucun nom partagé sans étiquettes | 3 s |
| 23 | `usecmp.sh` 9 commits ; script de `f` sur 8 arbres contre `usegraph.py` ; `counts.sh d2 e f` | 9 | 0 | preuve 9 ; table « Comptes » | 6 s + 8 s |
| 24 | les cinq scripts sur `wt_r` neuf à `4dcba1e` ; hachages ; `run217f.py`, `run217e.py` relancés ; `run217e.py . e1` sur `d2` ; détour de l'étude (`pico8_g__ready()` dans `planet.c`) à `e` | `wt_r` | 0 ; 1 ; 1 ; 0 ; 1 | hachages de « Ce qui change », cumul `78546d6a…` ; `AssertionError` ×2, arbre intact (après correction de l'ancre de tête de `run217f.py`, qui écrivait avant d'échouer) ; e1 : `1 2`, `3 / 3`, `paires 1` ; `implicit declaration of function 'pico8_g__ready'` | 3 s |
| 25 | `rm -rf` de la copie, des arbres de travail et des fichiers d'essai | — | 0 | supprimés | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas le commit de 2.17d2 (arbres `b5a5cb59…`, `ea2eb691…`, `422546ca…`, ou un descendant qui ne touche que `docs/`) sans que l'écart soit rapporté, ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si 2.17e et 2.17f ne sont pas deux commits, chacun vert (`make test`, `make e2e` 26 conformes, et 26 sous `MALLOC_PERTURB_=165`) ; si un `*.sha256`, `tests/e2e/`, une constante `*_HASH`/`FROZEN_*` change ; si `tests/` change hors la ligne de `test_pico8_fv.c` (e) et `test_composition_residue.c` (f), `tools/` hors `composition_residue.sh` (f).
- `FAILED:` si, au départ annoncé, les arbres commités ne sont pas ceux de « Ce qui change », sans repli rapporté.
- `FAILED:` si le relevé `usedir.py` d'un sens coupé n'est pas vide à `e` ; si une ligne `#include` est retirée alors qu'un nom de l'en-tête reste employé ; si `pico8_na` ou `pico8_jo_kv` est redéclaré localement ; si `entity_geom.c` emploie encore `pico8_mo_qb` ou `pico8_qb_t`.
- `FAILED:` si une paire ou une arête entre (compteur `--detail`, `usegraph.py`, arêtes orientées) ; si les comptes d'arrivée ne sont pas `0 ligne(s), 0 fichier(s)`, usage 0 ; si `inclusion-mutuelle` n'est pas dans `COMPOSITION_RESIDUE_GUARD` à `e`, `usage-mutuel` à `f` ; si une catégorie gardée n'est plus à 0 ; si `deadinc.py` trouve plus d'includes morts à l'enfant qu'au parent.
- `FAILED:` si, à `e`, les corps de `pico8_na` et de `jo_kv`/`pico8_jo_kv` ne sont pas identiques sous `-O0 -DNDEBUG` (`fnbody.py`, renommage par `objcopy`), ou si une autre fonction que `pico8_gd` et `pico8_jo_hu` diffère sous `-O0`/`-O1 -DNDEBUG`, ou une relocation de données hors les 4 entrées de `g_kv`.
- `FAILED:` si une jumelle a un TU rouge au parent qui ne l'est pas à l'enfant, ou un e2e < 26 au parent et 26 à l'enfant ; si une ligne couverte au parent ne l'est plus à l'enfant (hors l'artefact d'accolade décrit), ou si une ligne exécutable neuve de `pico8_gd` n'est pas couverte.
- `FAILED:` si la garde n'est pas prouvée mordante : `inclusion-mutuelle` à `e` (include remis) ; `usage-mutuel` à `f` sur un détour réel de `src/` qui compile, avec `inclusion-mutuelle` à 0 dans le même essai.
- `FAILED:` si la fixture de 2.17f n'est pas rouge avec le script de `e` et verte avec celui de `f`, ou si M1 ou M2 la laisse verte ; si le portage rend, sur un des arbres de la preuve 9, une autre liste de paires que `usegraph.py` ; si `usage-mutuel` lit les étiquettes de struct ou compte un nom déclaré par deux en-têtes.
- `FAILED:` si `class-coverage-net` ou `sanitize-net` est rouge ; si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si un message de commit omet : les fonctions déplacées et leurs fichiers, les includes retirés et ajoutés (dont G2 et G4), le relevé d'usage de chaque sens coupé, les comptes (paires / modules, lignes / fichiers, usage, catégories gardées), la preuve des objets par configuration, la table des jumelles avec la survivante GD-p, `L`, les gardes mordantes, pour 2.17f la fixture, M1, M2, l'équivalence du portage et le détour réel, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.17e » (resp. « 2.17f ») au bout de sa ligne du plan, ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- 2.18 (revue de sortie de phase, `warn-review`), sauf la table « Ce que 2.18 aura à constater ».
- Les includes préexistants à 0 nom (hors `planet_family.c:12`, que la coupe rend mort) ; le cycle du graphe d'usage ([4, 19]) ; les paires d'usage que verrait un lecteur plus fin (étiquettes de struct, macros d'un autre en-tête, prototypes à deux niveaux de parenthèses) : le portage garde les limites de `usegraph.py`, écrites dans son commentaire.
- `tools/line_coverage_reference.txt` ; les numéros de ligne cités par des documents hors `docs/plan-3.md`.
- Renommer `pico8_jo_kv` (phase 3 ; nom de l'étude, provisoire) ; la taille du code C, l'appli raylib (aucun de ses fichiers n'emploie `pico8_na`, `jo_kv` ni la catégorie : grep), l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Deux commits, e et f.** e change le code et ferme `inclusion-mutuelle` ; f change l'outil. Chacun a sa preuve (objets et jumelles ; fixture, équivalence, détour) et tient en ≈ 45 min. e n'est pas coupé en e1/e2 d'emblée : les trois coupes sont petites (+47 / −48) ; repli 5 mesuré.
- **G2. `planet_family.c:12` → `entity_geom.h` part avec `pico8_na`** (règle de 2.17a–d : un include que la coupe rend mort part dans le même commit) ; le garder laisserait un chemin de détour tout prêt.
- **G3. Les corps vont en fin de fichier** (`pico8_na` en fin de `planet.c`, le bloc `kv` en fin de `planet_family.c`, les déclarations en fin de `planet.h`), comme les essais de l'étude : aucun numéro de ligne plus haut ne bouge dans les fichiers d'arrivée.
- **G4. `test_pico8_fv.c` inclut `planet.h`** (règle unique de 2.17d : tout fichier qui emploie un nom inclut son en-tête), bien qu'il le reçoive par `ui_button.h` ; repli 6.
- **G5. `usage-mutuel` rend le premier emploi de chaque sens**, pas une ligne `#include` (qui n'existe pas dans un détour) ni tous les emplois (plus de cent lignes pour `planet_family` → `planet` à `d2`) : deux lignes par paire, qui nomment le nom qui franchit la frontière. Le plan (étude §4) disait « une ligne par paire » ; deux disent en plus quel sens est le détour.
- **G6. Le portage est fidèle, limites comprises** : ni étiquettes de struct, ni macros suivies. Plus fin, il ne serait plus « le graphe d'usage de l'étude » que le plan fait relancer à chaque sous-étape, et l'équivalence (preuve 9) ne se prouverait plus. La règle du nom partagé est gardée et justifiée dans le commentaire du lecteur par la mesure des 6 paires.
- **G7. Les deux catégories de paires partagent la branche de sortie** (`[ -f "$work/$cat.pairs" ]`) au lieu d'un second `if` nommé : une catégorie de paires de plus n'aurait qu'à écrire son `.pairs`.
- **G8. La ligne `:261` du TU est recoupée** (reste de la réserve de `7d7f194`, seule ligne de commentaire du fichier au-delà de 80 caractères) : le fichier est touché de toute façon.
- **G9. `class-coverage-net` et `sanitize-net` lancés bien que non exigés** (8 s).

### Surprises de mesure, pour `plan-pilot`

1. **La mutation « `p` lu au lieu de `ra` » du plan survit** aux deux bouts : `ra = ma(p, self)` ne s'écarte de `p` que pendant une secousse. Remplacée comme témoin par GD-self.
2. **`jo_kv` n'existe pas à `-O1`** : statique à un appelant, intégrée ; son identité se prouve à `-O0` par renommage (`objcopy`).
3. **Le motif `\}\s*(\w+)\s*;` de `usegraph.py` est mort** : `top()` ôte les deux accolades. Le portage l'omet.
4. **Un nom partagé attribué à un seul en-tête dépend de l'ordre de `for … in`** : la fixture de M1 a dû être construite pour rougir quel que soit l'en-tête retenu (Run 18).
5. **Le détour de preuve de l'étude ne compile plus à `e`** (réserve de `0acfad8` confirmée) ; celui d'`economy.c` compile.
6. **Le cycle se scinde** : 25 → 4 + 19 modules.
7. **`CLEAN_AWK` réserve le nom `depth`** : un lecteur neuf qui l'emploie comme tableau échoue sous `mawk`.

### Questions pour l'humain

Aucune.

### Annexe A — scripts (texte exact)

Tous hors dépôt, depuis un dossier d'essais `S` (à adapter) qui contient la copie et ses arbres de travail ; journaux dans `S/logs`. Repris tels quels : `usegraph.py`, `cross.py` (annexe A du PRD 2.11) ; `usedir.py`, `usegraph_e.py`, `deadinc.py`, `objs2.sh`, `fnbody.py`, `tests.sh`, `cov.sh`, `nets.sh` (annexe A du PRD 2.17a–b ; dans les `.sh`, `prd217ab` devient `prd217ef`) ; `run217c.py`, `run217d.py`, `counts.sh` (annexe A du PRD 2.17c–d ; `prd217cd` devient `prd217ef`) ; `syn.sh`, `scc.py`, `sites.py` (annexe A de l'étude 2.11 ; `sites.py` se lance `python3 sites.py <arbre> <sortie de usegraph.py>`) ; `lset.py` (annexe A du PRD 2.15c) ; `mut217e.py` = `mut.py` du PRD 2.15d où `prd215d` devient `prd217ef` et `muts.json` devient `muts217e.json`, lancé `python3 mut217e.py d2 <noms>` et `python3 mut217e.py e <noms>` (arbres `wt_d2`, `wt_e`). Sens de la preuve 2 : `DIRS="entity_geom>planet entity_geom>scroll planet>planet_family planet>entity_geom scroll>entity_geom planet_family>planet planet>probe" python3 usedir.py <arbre>`. Objets de `d2` renommés : `cp -r obj_d2_<cfg> ren_d2_<cfg> && objcopy --redefine-sym jo_kv=pico8_jo_kv ren_d2_<cfg>/pico8_planet.o`, puis `NORM_ASSERT=1 python3 fnbody.py ren_d2_<cfg> obj_e_<cfg>`. `clang` : `syn.sh` où `cc` devient `clang`.

`run217e.py` :

```python
# run217e.py ARBRE [e1] : 2.17e sur un arbre propre au commit 2.17d2. Chaque
# texte cherche doit etre present exactement une fois, sinon arret
# (AssertionError). e1 : pico8_na et pico8_gd seuls (repli).
import os, sys, re
T = sys.argv[1]
def p(f): return os.path.join(T, f if '/' in f else 'src/' + f)
def rd(f): return open(p(f)).read()
def wr(f, s): open(p(f), 'w').write(s)
def sub(f, old, new):
    s = rd(f); assert s.count(old) == 1, (f, old, s.count(old)); wr(f, s.replace(old, new))
def cut_span(f, start, end):
    s = rd(f); assert s.count(start) == 1 and s.count(end) == 1, (f, start, end)
    i = s.index(start); j = s.index(end, i) + len(end)
    while s[j:j+1] == '\n': j += 1
    wr(f, s[:i] + s[j:]); return s[i:j].rstrip('\n') + '\n'
def append(f, text): wr(f, rd(f).rstrip('\n') + '\n\n' + text.rstrip('\n') + '\n')
def before_endif(f, text): sub(f, '\n#endif\n', '\n' + text + '\n#endif\n')
def add_include(f, h, pre=''):
    s = rd(f); inc = '#include "%s%s"\n' % (pre, h); assert inc not in s, (f, h)
    m = list(re.finditer(r'#include "%spico8_[^"]*"\n' % re.escape(pre), s)); assert m, f
    own = m[0].end() if f.endswith('.c') and not pre else 0
    ins = next((x for x in m if x.group(0) > inc and x.start() >= own), None)
    pos = ins.start() if ins else m[-1].end()
    wr(f, s[:pos] + inc + s[pos:])

# e1a : entity_geom -> planet (fiche 3, T6) : pico8_na passe dans planet
na = cut_span('pico8_entity_geom.c', 'void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t) {',
              '        ra_vec->y = pico8_fixed_add(ra_vec->y, delta);\n    }\n}\n')
d = cut_span('pico8_entity_geom.h', '/* Mutates the pico8_vec_t `self.ra` points at',
             'void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t);')
sub('pico8_entity_geom.h', "/* `gk`'s `e` and `na`'s `self` are NULL or a struct (asserted); `gk`'s\n * NULL `mp`/`rp` reads as the zero vector. */",
    "/* `gk`'s `e` is NULL or a struct (asserted); its NULL `mp`/`rp` reads as\n * the zero vector. */")
append('pico8_planet.c', na)
before_endif('pico8_planet.h', d)
sub('pico8_entity_geom.c', '#include "pico8_planet.h"\n', '')
add_include('tests/test_pico8_fv.c', 'pico8_planet.h', '../src/')
# planet_family.c n'employait d'entity_geom.h que pico8_na (G2)
sub('pico8_planet_family.c', '#include "pico8_entity_geom.h"\n', '')
# e1b : entity_geom -> scroll (fiche c3, T7) : la camera lue par le lien ra du noyau
sub('pico8_entity_geom.c', '''    const pico8_qb_t *mo = pico8_mo_qb();
    pico8_vec_t mo_ra = mo != NULL && mo->ra != NULL ? *mo->ra : pico8_entity_geom_zero();''',
'''    pico8_vec_t mo_ra = pico8_entity_geom_ra(pico8_mo());''')
print('2.17e1 : pico8_na dans planet, pico8_gd sur pico8_mo(), 2 includes retires, 1 ajoute (TU)')
if len(sys.argv) > 2 and sys.argv[2] == 'e1': sys.exit(0)

# e2 : planet -> planet_family (fiche 6, T6) : le registre kv passe dans planet_family
kv = cut_span('pico8_planet.c', '/* `kv`, indexed by `fz`', '            return g_kv[i].kind();\n        }\n    }\n    return NULL;\n}\n')
kv = kv.replace('/* NULL when no kind has `fz`. */\n', '', 1)
assert kv.count('static const pico8_entity_kind_t *jo_kv(const char *fz) {') == 1
kv = kv.replace('static const pico8_entity_kind_t *jo_kv(', 'const pico8_entity_kind_t *pico8_jo_kv(', 1)
append('pico8_planet_family.c', kv)
sub('pico8_planet.c', 'target_kind = jo_kv(hp_instance->cw);', 'target_kind = pico8_jo_kv(hp_instance->cw);')
before_endif('pico8_planet.h', '/* NULL when no kind has `fz`. */\nconst pico8_entity_kind_t *pico8_jo_kv(const char *fz);\n')
sub('pico8_planet.c', '#include "pico8_planet_family.h"\n', '')
print('2.17e2 : registre kv -> pico8_jo_kv (planet_family), 1 include retire')
# inclusion-mutuelle a 0 : gardee
sub('./Makefile', ',parente,void-ptr\n', ',parente,void-ptr,inclusion-mutuelle\n')
print('2.17e : inclusion-mutuelle dans COMPOSITION_RESIDUE_GUARD')
```

`run217f.py` (lit `usage_body.txt` : le programme `awk` de `read_usages` tel qu'il figure dans le diff de l'annexe B, lignes `+` entre `awk -v pairs="$work/usage-mutuel.pairs" "$CLEAN_AWK"'` et `' pass=1 …`, sans le `+` ; ou `git apply` de ce diff, même résultat) :

```python
# run217f.py ARBRE : 2.17f sur un arbre propre au commit 2.17e. Chaque texte
# cherche doit etre present exactement une fois, sinon arret (AssertionError).
# Le programme awk de usage-mutuel est lu dans usage_body.txt (annexe A).
import os, sys
T = sys.argv[1]; HERE = os.path.dirname(os.path.abspath(__file__))
def p(f): return os.path.join(T, f)
def rd(f): return open(p(f)).read()
def wr(f, s): open(p(f), 'w').write(s)
def sub(f, old, new):
    s = rd(f); assert s.count(old) == 1, (f, old, s.count(old)); wr(f, s.replace(old, new))
SH, TU, MK = 'tools/composition_residue.sh', 'tests/test_composition_residue.c', 'Makefile'
AWK = open(os.path.join(HERE, 'usage_body.txt')).read()

# 1. en-tete du script
sub(SH, '''# --check, ses lignes sont les #include qui realisent ces paires.
#   sans option''', '''# --check, ses lignes sont les #include qui realisent ces paires.
# usage-mutuel compte les paires de modules hors tests/ dont chacun emploie
# un nom que l'en-tete de l'autre est seul a declarer, include ou detour ;
# ses lignes sont, par paire et par sens, le premier emploi.
#   sans option''')
sub(SH, '("<paires>\\t<modules>" pour inclusion-mutuelle), puis',
        '("<paires>\\t<modules>" pour inclusion-mutuelle et\n#                 usage-mutuel), puis')
sub(SH, '''#                 "inclusion-mutuelle\\t<a>\\t<b>" par paire, sous void-ptr les''',
        '''#                 "<cat>\\t<a>\\t<b>" par paire d'inclusion-mutuelle et
#                 d'usage-mutuel, sous void-ptr les''')
# 2. listes
sub(SH, 'registres noyau-inclut atteinte-à-prouver inclusion-mutuelle"\n\n# Categories comptees dans le pied.',
        'registres noyau-inclut atteinte-à-prouver inclusion-mutuelle usage-mutuel"\n\n# Categories comptees dans le pied.')
sub(SH, 'registres noyau-inclut atteinte-à-prouver inclusion-mutuelle"\n\nfail() {',
        'registres noyau-inclut atteinte-à-prouver inclusion-mutuelle usage-mutuel"\n\nfail() {')
# 3. lecteur
sub(SH, '''    sort -t: -k1,1 -k2,2n "$work/inclusion-mutuelle" > "$work/sorted" && mv "$work/sorted" "$work/inclusion-mutuelle"
}
''', '''    sort -t: -k1,1 -k2,2n "$work/inclusion-mutuelle" > "$work/sorted" && mv "$work/sorted" "$work/inclusion-mutuelle"
}

# Graphe d'usage de l'etude 2.11 (usegraph.py) : a -> b quand un .c/.h de a
# hors tests/ emploie un nom que b.h est seul a declarer, commentaires et
# chaines retires ; il voit la paire qu'un detour cache a inclusion-mutuelle.
# Noms lus par motif : #define, prototype (un niveau de parentheses
# internes), (*nom), dernier nom d'un typedef ou d'un extern hors des
# accolades, enumerateur ; pas les etiquettes de struct. Un nom que deux
# en-tetes declarent n'est a aucun : les etiquettes, lues sans cette regle,
# feraient six paires par les declarations anticipees de pico8_struct.h.
# Rend dans $work/usage-mutuel.pairs les paires, dans $work/usage-mutuel le
# premier emploi de chaque sens (ordre de $work/files, puis des lignes).
read_usages() {
    grep -v '/tests/' "$work/files" | grep -v '^tests/' > "$work/modfiles" || true
    : > "$work/usage-mutuel.pairs"
    : > "$work/usage-mutuel"
    [ -s "$work/modfiles" ] || return 0
    grep '\\.h$' "$work/modfiles" > "$work/modheaders" || true
    awk -v pairs="$work/usage-mutuel.pairs" "$CLEAN_AWK"'
''' + AWK + '''    ' pass=1 $(cat "$work/modheaders") pass=2 $(cat "$work/modfiles") > "$work/usage-mutuel" || fail "echec de awk"
    sort "$work/usage-mutuel.pairs" > "$work/sorted" && mv "$work/sorted" "$work/usage-mutuel.pairs"
    sort -u -t: -k1,1 -k2,2n "$work/usage-mutuel" > "$work/sorted" && mv "$work/sorted" "$work/usage-mutuel"
}
''')
sub(SH, '        inclusion-mutuelle) read_inclusions; return 0 ;;\n',
        '        inclusion-mutuelle) read_inclusions; return 0 ;;\n        usage-mutuel) read_usages; return 0 ;;\n')
# 4. sortie : les deux categories de paires
sub(SH, '''        if [ "$cat" = inclusion-mutuelle ] && [ "$mode" = detail ]; then
            sed 's/^/inclusion-mutuelle\\t/' "$work/$cat.pairs"
        elif [ "$cat" = inclusion-mutuelle ]; then''', '''        if [ -f "$work/$cat.pairs" ] && [ "$mode" = detail ]; then
            sed "s/^/$cat\\t/" "$work/$cat.pairs"
        elif [ -f "$work/$cat.pairs" ]; then''')
# 5. garde
sub(MK, ',parente,void-ptr,inclusion-mutuelle\n', ',parente,void-ptr,inclusion-mutuelle,usage-mutuel\n')

# 6. TU : un temoin dans la table, la fixture a detour, la reserve des 80 colonnes
sub(TU, '''    write_file(SRC "/b.h", "#include \\"a.h\\"\\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "parente\\t1\\t1\\n"''', '''    write_file(SRC "/b.h", "#include \\"a.h\\"\\n");
    write_file(SRC "/k.h", "int k_f(void);\\n");
    write_file(SRC "/k.c", "int k = l_g();\\n");
    write_file(SRC "/l.h", "int l_g(void);\\n");
    write_file(SRC "/l.c", "int l = k_f();\\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "parente\\t1\\t1\\n"''')
sub(TU, '''                                  "inclusion-mutuelle\\t1\\t2\\n"
                                  "résidu 2 : 13 ligne(s), 13 fichier(s)\\n") == 0);''',
        '''                                  "inclusion-mutuelle\\t1\\t2\\n"
                                  "usage-mutuel\\t1\\t2\\n"
                                  "résidu 2 : 15 ligne(s), 15 fichier(s)\\n") == 0);''')
sub(TU, '''/* Formes d'un registre que le lecteur voit (F) et limites qu'il ne voit pas
 * (L), chacune a sa ligne : une limite qui se met a compter change l'attendu. */''',
        '''/* Formes d'un registre que le lecteur voit (F) et limites qu'il ne voit pas
 * (L), chacune a sa ligne : une limite qui se met a compter change
 * l'attendu. */''')
sub(TU, '''static void test_detail_sorts_by_category_then_file(void) {''', '''/* a et b s'emploient, mais a.c ne recoit b.h que par c.h : inclusion-mutuelle
 * n'y voit rien. d et e emploient u, qui ne les emploie que par s_t, que d.h
 * et e.h declarent tous deux ; f n'emploie g que dans un commentaire et une
 * chaine : ni u-d, ni u-e, ni f-g n'est une paire. */
static void test_usage_pairs_see_a_detour_not_a_shared_name_nor_a_comment(void) {
    setup();
    write_file(SRC "/a.h", "int a_f(void);\\n");
    write_file(SRC "/b.h", "#include \\"a.h\\"\\nint b_g(void);\\n");
    write_file(SRC "/c.h", "#include \\"b.h\\"\\n");
    write_file(SRC "/a.c", "#include \\"a.h\\"\\n#include \\"c.h\\"\\nint a_f(void) {\\n    return b_g();\\n}\\n");
    write_file(SRC "/b.c", "#include \\"b.h\\"\\nint b_g(void) {\\n    return a_f();\\n}\\n");
    write_file(SRC "/d.h", "struct s_tag;\\ntypedef struct s_tag s_t;\\n");
    write_file(SRC "/e.h", "struct s_tag;\\ntypedef struct s_tag s_t;\\n");
    write_file(SRC "/u.h", "int u_f(void);\\n");
    write_file(SRC "/u.c", "int u_g(s_t *s) { return s != 0; }\\n");
    write_file(SRC "/d.c", "#include \\"u.h\\"\\nint d = u_f();\\n");
    write_file(SRC "/e.c", "#include \\"u.h\\"\\nint e = u_f();\\n");
    write_file(SRC "/f.h", "int f_k(void);\\n");
    write_file(SRC "/g.h", "int g_h(void);\\n");
    write_file(SRC "/f.c", "/* g_h */\\nconst char *f_n = \\"g_h\\";\\nint f_k(void) { return 0; }\\n");
    write_file(SRC "/g.c", "#include \\"f.h\\"\\nint g_h(void) { return f_k(); }\\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "usage-mutuel\\t1\\t2\\n"
                                  "résidu 2 : 2 ligne(s), 2 fichier(s)\\n") == 0);
    assert(run_residue("--detail " TREE) == 0);
    assert(strcmp(read_file(OUT), "usage-mutuel\\ta\\tb\\n"
                                  "résidu 2 : 2 ligne(s), 2 fichier(s)\\n") == 0);
    assert(run_residue("--check inclusion-mutuelle,usage-mutuel " TREE) == 1);
    assert(strcmp(read_file(OUT), "usage-mutuel: " SRC "/a.c:4:    return b_g();\\n"
                                  "usage-mutuel: " SRC "/b.c:3:    return a_f();\\n"
                                  "résidu gardé (inclusion-mutuelle,usage-mutuel) : 2 ligne(s)\\n") == 0);
    assert(system("rm " SRC "/a.c") == 0);
    assert(run_residue("--check usage-mutuel " TREE) == 0);
    assert(strcmp(read_file(OUT), "résidu gardé (usage-mutuel) : 0 ligne(s)\\n") == 0);
}

static void test_detail_sorts_by_category_then_file(void) {''')
sub(TU, '''    test_mutual_inclusion_counts_its_include_lines_in_the_footer();
''', '''    test_mutual_inclusion_counts_its_include_lines_in_the_footer();
    test_usage_pairs_see_a_detour_not_a_shared_name_nor_a_comment();
''')
print('2.17f : usage-mutuel (script, garde, TU)')
```

`muts217egen.py` :

```python
# muts217egen.py : ecrit muts217e.json (format de mut.py : [fichier, ancre, ancien, neuf]
# par cote) ; cotes d2 = 2.17d2, e = 2.17e ; meme effet des deux cotes.
import json
G, P, F = 'src/pico8_entity_geom.c', 'src/pico8_planet.c', 'src/pico8_planet_family.c'
GD = 'int pico8_gd('; NA = 'void pico8_na('
OLD = 'mo != NULL && mo->ra != NULL ? *mo->ra : pico8_entity_geom_zero()'
NEW = 'pico8_entity_geom_ra(pico8_mo())'
J = {
 'GD-zero': {'d2': [[G, GD, OLD, 'mo != NULL && mo->ra != NULL ? pico8_entity_geom_zero() : pico8_entity_geom_zero()']],
             'e':  [[G, GD, NEW, 'pico8_entity_geom_zero()']]},
 'GD-p':    {'d2': [[G, GD, OLD, 'mo != NULL && mo->p != NULL ? *mo->p : pico8_entity_geom_zero()']],
             'e':  [[G, GD, NEW, 'pico8_entity_geom_p(pico8_mo())']]},
 'GD-self': {'d2': [[G, GD, OLD, 'mo != NULL && mo->ra != NULL ? e_ra : e_ra']],
             'e':  [[G, GD, NEW, 'pico8_entity_geom_ra(e)']]},
 'NA-sub':  {'d2': [[G, NA, 'ra_vec->y = pico8_fixed_add(ra_vec->y, delta);', 'ra_vec->y = pico8_fixed_sub(ra_vec->y, delta);']],
             'e':  [[P, NA, 'ra_vec->y = pico8_fixed_add(ra_vec->y, delta);', 'ra_vec->y = pico8_fixed_sub(ra_vec->y, delta);']]},
 'KV-swap': {'d2': [[P, None, '{"pm", pico8_pm_kind},', '{"pm", pico8_ga_kind},']],
             'e':  [[F, None, '{"pm", pico8_pm_kind},', '{"pm", pico8_ga_kind},']]},
 'KV-miss': {'d2': [[P, None, 'if (strcmp(g_kv[i].fz, fz) == 0) {', 'if (strcmp(g_kv[i].fz, fz) != 0) {']],
             'e':  [[F, None, 'if (strcmp(g_kv[i].fz, fz) == 0) {', 'if (strcmp(g_kv[i].fz, fz) != 0) {']]},
}
json.dump(J, open('muts217e.json', 'w'), indent=1)
```

`mkawk.sh` (tire du script de `f` les deux programmes que lit `usecmp.sh`) :

```sh
#!/bin/sh
# mkawk.sh ARBRE_F : usage.awk (programme de read_usages, desindente) et clean.awk
# (CLEAN_AWK) tires du script de 2.17f, pour usecmp.sh.
S=/tmp/claude-0/-home-user-slipways-love/c83e28ad-64b1-5e5c-ac9f-9444621e8cef/scratchpad/prd217ef
sed -n '/^    awk -v pairs="$work\/usage-mutuel.pairs"/,/^    . pass=1/p' "$1/tools/composition_residue.sh" | sed '1d;$d' | sed 's/^    //' > $S/usage.awk
sed -n "/^CLEAN_AWK='/,/^'/p" "$1/tools/composition_residue.sh" | sed '1d;$d' > $S/clean.awk
```

`dump.awk` :

```awk
END { for (k in site) { split(k, ab, SUBSEP); print "E\t" ab[1] "\t" ab[2] > dump } }
```

`usecmp.sh` (`bash usecmp.sh <rév>...`, sur la copie) :

```sh
#!/bin/bash
# usecmp.sh REV... : aretes orientees du portage awk contre usegraph_e.py, paires comprises
S=/tmp/claude-0/-home-user-slipways-love/c83e28ad-64b1-5e5c-ac9f-9444621e8cef/scratchpad/prd217ef
cd $S/copie
for r in "$@"; do
  git checkout -q $r
  ls src/*.h | LC_ALL=C sort > $S/logs/uh.txt; find src -name '*.[ch]' | LC_ALL=C sort > $S/logs/uf.txt
  : > $S/logs/awk_pairs_$r.txt; : > $S/logs/awk_edges_raw.txt
  LC_ALL=C mawk -v pairs=$S/logs/awk_pairs_$r.txt -v dump=$S/logs/awk_edges_raw.txt -f $S/clean.awk -f $S/usage.awk -f $S/dump.awk pass=1 $(cat $S/logs/uh.txt) pass=2 $(cat $S/logs/uf.txt) > $S/logs/awk_sites_$r.txt
  sort $S/logs/awk_edges_raw.txt > $S/logs/awk_edges_$r.txt
  python3 $S/usegraph_e.py . 2>&1 >/dev/null | grep '^E' | cut -f1-3 | sort > $S/logs/py_edges_$r.txt
  python3 $S/usegraph.py . 2>/dev/null | cut -f1,2 | sort > $S/logs/py_pairs_$r.txt
  sort -o $S/logs/awk_pairs_$r.txt $S/logs/awk_pairs_$r.txt
  echo "$r aretes awk $(wc -l < $S/logs/awk_edges_$r.txt) py $(wc -l < $S/logs/py_edges_$r.txt) diff $(diff $S/logs/awk_edges_$r.txt $S/logs/py_edges_$r.txt | grep -c '^[<>]') | paires awk $(wc -l < $S/logs/awk_pairs_$r.txt) py $(wc -l < $S/logs/py_pairs_$r.txt) diff $(diff $S/logs/awk_pairs_$r.txt $S/logs/py_pairs_$r.txt | grep -c '^[<>]') | sites $(wc -l < $S/logs/awk_sites_$r.txt)"
done
git checkout -q work
```

Script de `f` sur les arbres anciens (preuve 9, second volet) : `git show f:tools/composition_residue.sh > cr_f.sh`, puis pour chaque révision `git checkout -q <rév> && sh cr_f.sh --detail | grep '^usage-mutuel' | cut -f2,3 | sort | sha256sum` contre `python3 usegraph.py . | cut -f1,2 | sort | sha256sum`.

Fixture seule (preuve 8) : `fixture_only.c` = `tests/test_composition_residue.c` de `f` dont `main` est remplacé par `int main(void) { test_usage_pairs_see_a_detour_not_a_shared_name_nor_a_comment(); return 0; }`, compilé `cc -std=c99 -w`, lancé depuis la racine de l'arbre avec `tools/composition_residue.sh` pris à `e` puis à `f`.

`usegraph_sh.py`, `usegraph_tags.py` (mesure de la règle du partage, preuve 8) :

```python
# usegraph_sh.py : usegraph.py, bloc insere avant `pairs=[(a,b)` (sortie d'erreur) ;
# usegraph_tags.py : le meme, avec `n|=set(re.findall(r'struct\s+(\w+)',s)) if
# os.environ.get('TAGS') else set()` insere avant `n-= {'','PICO8'}`.
sh={x:sorted(o) for x,o in owner.items() if len(o)>1}
for x in sorted(sh): print('SHARED',x,' '.join(sh[x]),file=sys.stderr)
use2=collections.defaultdict(lambda: collections.defaultdict(set))
for a,fs in files.items():
    toks=set()
    for f in fs: toks|=set(re.findall(r'\b[A-Za-z_]\w*\b',strip(open(f).read())))
    for x in toks:
        for b in owner.get(x,()):
            if b!=a: use2[a][b].add(x)
p2=[(a,b) for a in use2 for b in use2[a] if a<b and a in use2.get(b,{})]
print('PAIRES_PARTAGE',len(p2),file=sys.stderr)
```

### Annexe B — diffs (texte exact)

`git diff d2 e` (2.17e, 210 lignes, `2a848d9c…`) :

```diff
diff --git a/Makefile b/Makefile
index 422546c..2f29559 100644
--- a/Makefile
+++ b/Makefile
@@ -871,7 +871,7 @@ composition-residue: tools/composition_residue.sh
 	@sh tools/composition_residue.sh
 
 # Catégories du compteur de composition tombées à 0, séparées par des virgules.
-COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6,lien-parent,parente,void-ptr
+COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6,lien-parent,parente,void-ptr,inclusion-mutuelle
 
 composition-guard: tools/composition_residue.sh
 	@sh tools/composition_residue.sh --check $(COMPOSITION_RESIDUE_GUARD) src tests
diff --git a/src/pico8_entity_geom.c b/src/pico8_entity_geom.c
index d0d58c0..0ffb089 100644
--- a/src/pico8_entity_geom.c
+++ b/src/pico8_entity_geom.c
@@ -6,7 +6,6 @@
 
 #include "pico8_entity.h"
 #include "pico8_geom.h"
-#include "pico8_planet.h"
 
 static pico8_vec_t pico8_entity_geom_zero(void) {
     return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
@@ -43,8 +42,7 @@ int pico8_gk(const pico8_struct_header_t *e, const pico8_vec_t *mp, const pico8_
 
 int pico8_gd(const pico8_struct_header_t *e) {
     pico8_vec_t e_ra = pico8_entity_geom_ra(e);
-    const pico8_qb_t *mo = pico8_mo_qb();
-    pico8_vec_t mo_ra = mo != NULL && mo->ra != NULL ? *mo->ra : pico8_entity_geom_zero();
+    pico8_vec_t mo_ra = pico8_entity_geom_ra(pico8_mo());
     pico8_fixed_t sixty_four = pico8_fixed_from_int(64);
     pico8_fixed_t seventy_six = pico8_fixed_from_int(76);
 
@@ -52,18 +50,6 @@ int pico8_gd(const pico8_struct_header_t *e) {
            pico8_fixed_abs(pico8_fixed_sub(pico8_fixed_sub(e_ra.y, mo_ra.y), sixty_four)) <= seventy_six;
 }
 
-void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t) {
-    pico8_vec_t *ra_vec;
-
-    assert((self == NULL || pico8_jo_is_family(self)) && "na: self is a planet (qh:ly, nl:ly) -- stop");
-    ra_vec = self != NULL ? ((pico8_jo_t *)self)->ra : NULL;
-    if (ra_vec != NULL) {
-        pico8_fixed_t delta = pico8_fixed_mul(pico8_fixed_sin(pico8_fixed_mul(t, PICO8_FIXED_LIT(0x00000147, 0.005))),
-                                               PICO8_FIXED_LIT(0x00000f5c, 0.06));
-        ra_vec->y = pico8_fixed_add(ra_vec->y, delta);
-    }
-}
-
 pico8_vec_t pico8_ma(pico8_vec_t p, pico8_struct_header_t *e) {
     pico8_fixed_t *ma = NULL;
     pico8_fixed_t decremented;
diff --git a/src/pico8_entity_geom.h b/src/pico8_entity_geom.h
index b758301..abd3ddb 100644
--- a/src/pico8_entity_geom.h
+++ b/src/pico8_entity_geom.h
@@ -11,17 +11,12 @@
  * field reads as the zero vector, as 0, or as a false hit test. */
 /* TROU-15 */
 
-/* `gk`'s `e` and `na`'s `self` are NULL or a struct (asserted); `gk`'s
- * NULL `mp`/`rp` reads as the zero vector. */
+/* `gk`'s `e` is NULL or a struct (asserted); its NULL `mp`/`rp` reads as
+ * the zero vector. */
 int pico8_gk(const pico8_struct_header_t *e, const pico8_vec_t *mp, const pico8_vec_t *rp);
 
 int pico8_gd(const pico8_struct_header_t *e);
 
-/* Mutates the pico8_vec_t `self.ra` points at, in place: every other holder
- * of that vector -- the `iv` toast `jo:hk` builds from `self.ra` -- sees the
- * bob. Rebinding the field to a fresh vector, as `qb:kj` does, would not. */
-void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t);
-
 /* `e` is NULL or carries `ma` (asserted): a planet or the camera. Decays
  * `e.ma` in place. */
 pico8_vec_t pico8_ma(pico8_vec_t p, pico8_struct_header_t *e);
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index 0dbcf72..5fdd889 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -15,7 +15,6 @@
 #include "pico8_ke.h"
 #include "pico8_msgbox.h"
 #include "pico8_pi.h"
-#include "pico8_planet_family.h"
 #include "pico8_research.h"
 #include "pico8_tech_gate.h"
 
@@ -275,28 +274,6 @@ void pico8_jo_ll(pico8_jo_t *self, pico8_fixed_t t) {
     self->p_ = t > pico8_fixed_from_int(60);
 }
 
-/* `kv`, indexed by `fz`: `pm` and `ga` are the only kinds whose spec sets
- * one. */
-static const struct {
-    const char *fz;
-    const pico8_entity_kind_t *(*kind)(void);
-} g_kv[] = {
-    {"pm", pico8_pm_kind},
-    {"ga", pico8_ga_kind},
-};
-
-/* NULL when no kind has `fz`. */
-static const pico8_entity_kind_t *jo_kv(const char *fz) {
-    size_t i;
-
-    for (i = 0; i < sizeof g_kv / sizeof g_kv[0]; i++) {
-        if (strcmp(g_kv[i].fz, fz) == 0) {
-            return g_kv[i].kind();
-        }
-    }
-    return NULL;
-}
-
 void pico8_jo_hu(pico8_jo_t *self, pico8_hp_t *hp_instance) {
     pico8_jo_t *working_self;
     pico8_fixed_t fp = pico8_fixed_from_int(2);
@@ -313,7 +290,7 @@ void pico8_jo_hu(pico8_jo_t *self, pico8_hp_t *hp_instance) {
 
         self->p_ = 1;
 
-        target_kind = jo_kv(hp_instance->cw);
+        target_kind = pico8_jo_kv(hp_instance->cw);
 
         keys.p = self->p;
         keys.ra = self->ra;
@@ -402,3 +379,15 @@ void pico8_jo_hk(const pico8_jo_t *self) {
          * frame it stays on screen: not freed here. */
     }
 }
+
+void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t) {
+    pico8_vec_t *ra_vec;
+
+    assert((self == NULL || pico8_jo_is_family(self)) && "na: self is a planet (qh:ly, nl:ly) -- stop");
+    ra_vec = self != NULL ? ((pico8_jo_t *)self)->ra : NULL;
+    if (ra_vec != NULL) {
+        pico8_fixed_t delta = pico8_fixed_mul(pico8_fixed_sin(pico8_fixed_mul(t, PICO8_FIXED_LIT(0x00000147, 0.005))),
+                                               PICO8_FIXED_LIT(0x00000f5c, 0.06));
+        ra_vec->y = pico8_fixed_add(ra_vec->y, delta);
+    }
+}
diff --git a/src/pico8_planet.h b/src/pico8_planet.h
index cde5f77..3012358 100644
--- a/src/pico8_planet.h
+++ b/src/pico8_planet.h
@@ -149,4 +149,12 @@ pico8_down_t pico8_jo_ldown(pico8_jo_t *self);
 void pico8_jo_set_oc(pico8_gn_state_t *oc);
 void pico8_jo_set_ir(pico8_jf_state_t *ir);
 
+/* Mutates the pico8_vec_t `self.ra` points at, in place: every other holder
+ * of that vector -- the `iv` toast `jo:hk` builds from `self.ra` -- sees the
+ * bob. Rebinding the field to a fresh vector, as `qb:kj` does, would not. */
+void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t);
+
+/* NULL when no kind has `fz`. */
+const pico8_entity_kind_t *pico8_jo_kv(const char *fz);
+
 #endif
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index d43c7e2..09c1da4 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -9,7 +9,6 @@
 #include "pico8_collect.h"
 #include "pico8_dispatch_context.h"
 #include "pico8_entity.h"
-#include "pico8_entity_geom.h"
 #include "pico8_ke.h"
 #include "pico8_particle.h"
 #include "pico8_pi.h"
@@ -648,3 +647,24 @@ const pico8_struct_class_t *pico8_ga_struct_class(void) {
 pico8_jo_t *pico8_ga_new(const pico8_ga_keys_t *keys) {
     return pico8_ga_struct_new(keys);
 }
+
+/* `kv`, indexed by `fz`: `pm` and `ga` are the only kinds whose spec sets
+ * one. */
+static const struct {
+    const char *fz;
+    const pico8_entity_kind_t *(*kind)(void);
+} g_kv[] = {
+    {"pm", pico8_pm_kind},
+    {"ga", pico8_ga_kind},
+};
+
+const pico8_entity_kind_t *pico8_jo_kv(const char *fz) {
+    size_t i;
+
+    for (i = 0; i < sizeof g_kv / sizeof g_kv[0]; i++) {
+        if (strcmp(g_kv[i].fz, fz) == 0) {
+            return g_kv[i].kind();
+        }
+    }
+    return NULL;
+}
diff --git a/tests/test_pico8_fv.c b/tests/test_pico8_fv.c
index f67303a..7595f5d 100644
--- a/tests/test_pico8_fv.c
+++ b/tests/test_pico8_fv.c
@@ -29,6 +29,7 @@
 #include "../src/pico8_input.h"
 #include "../src/pico8_mem.h"
 #include "../src/pico8_particle.h"
+#include "../src/pico8_planet.h"
 #include "../src/pico8_probe.h"
 #include "../src/pico8_struct.h"
 #include "../src/pico8_ui.h"
```

`git diff e f` (2.17f, 265 lignes, `40131301…`) :

```diff
diff --git a/Makefile b/Makefile
index 2f29559..0b08417 100644
--- a/Makefile
+++ b/Makefile
@@ -871,7 +871,7 @@ composition-residue: tools/composition_residue.sh
 	@sh tools/composition_residue.sh
 
 # Catégories du compteur de composition tombées à 0, séparées par des virgules.
-COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6,lien-parent,parente,void-ptr,inclusion-mutuelle
+COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6,lien-parent,parente,void-ptr,inclusion-mutuelle,usage-mutuel
 
 composition-guard: tools/composition_residue.sh
 	@sh tools/composition_residue.sh --check $(COMPOSITION_RESIDUE_GUARD) src tests
diff --git a/tests/test_composition_residue.c b/tests/test_composition_residue.c
index 13e6704..69f3ad5 100644
--- a/tests/test_composition_residue.c
+++ b/tests/test_composition_residue.c
@@ -93,6 +93,10 @@ static void test_one_occurrence_per_category_in_table_order(void) {
     write_file(ATT, "q1\tx\n");
     write_file(SRC "/a.h", "#include \"b.h\"\n");
     write_file(SRC "/b.h", "#include \"a.h\"\n");
+    write_file(SRC "/k.h", "int k_f(void);\n");
+    write_file(SRC "/k.c", "int k = l_g();\n");
+    write_file(SRC "/l.h", "int l_g(void);\n");
+    write_file(SRC "/l.c", "int l = k_f();\n");
     assert(run_residue(TREE) == 0);
     assert(strcmp(read_file(OUT), "parente\t1\t1\n"
                                   "lien-parent\t1\t1\n"
@@ -106,7 +110,8 @@ static void test_one_occurrence_per_category_in_table_order(void) {
                                   "noyau-inclut\t1\t1\n"
                                   "atteinte-à-prouver\t1\t1\n"
                                   "inclusion-mutuelle\t1\t2\n"
-                                  "résidu 2 : 13 ligne(s), 13 fichier(s)\n") == 0);
+                                  "usage-mutuel\t1\t2\n"
+                                  "résidu 2 : 15 ligne(s), 15 fichier(s)\n") == 0);
 }
 
 static void test_other_witnesses_of_each_pattern_bite(void) {
@@ -258,7 +263,8 @@ static void test_registres_reads_declaration_points_outside_entity_types(void) {
 }
 
 /* Formes d'un registre que le lecteur voit (F) et limites qu'il ne voit pas
- * (L), chacune a sa ligne : une limite qui se met a compter change l'attendu. */
+ * (L), chacune a sa ligne : une limite qui se met a compter change
+ * l'attendu. */
 static void test_registres_sees_hidden_forms_and_not_its_named_limits(void) {
     setup();
     write_file(SRC "/f.h", "typedef pico8_" "ptrs_t reg_t;\n"
@@ -587,6 +593,42 @@ static void test_mutual_inclusion_counts_its_include_lines_in_the_footer(void) {
     assert(strcmp(read_file(OUT), "résidu gardé (inclusion-mutuelle) : 0 ligne(s)\n") == 0);
 }
 
+/* a et b s'emploient, mais a.c ne recoit b.h que par c.h : inclusion-mutuelle
+ * n'y voit rien. d et e emploient u, qui ne les emploie que par s_t, que d.h
+ * et e.h declarent tous deux ; f n'emploie g que dans un commentaire et une
+ * chaine : ni u-d, ni u-e, ni f-g n'est une paire. */
+static void test_usage_pairs_see_a_detour_not_a_shared_name_nor_a_comment(void) {
+    setup();
+    write_file(SRC "/a.h", "int a_f(void);\n");
+    write_file(SRC "/b.h", "#include \"a.h\"\nint b_g(void);\n");
+    write_file(SRC "/c.h", "#include \"b.h\"\n");
+    write_file(SRC "/a.c", "#include \"a.h\"\n#include \"c.h\"\nint a_f(void) {\n    return b_g();\n}\n");
+    write_file(SRC "/b.c", "#include \"b.h\"\nint b_g(void) {\n    return a_f();\n}\n");
+    write_file(SRC "/d.h", "struct s_tag;\ntypedef struct s_tag s_t;\n");
+    write_file(SRC "/e.h", "struct s_tag;\ntypedef struct s_tag s_t;\n");
+    write_file(SRC "/u.h", "int u_f(void);\n");
+    write_file(SRC "/u.c", "int u_g(s_t *s) { return s != 0; }\n");
+    write_file(SRC "/d.c", "#include \"u.h\"\nint d = u_f();\n");
+    write_file(SRC "/e.c", "#include \"u.h\"\nint e = u_f();\n");
+    write_file(SRC "/f.h", "int f_k(void);\n");
+    write_file(SRC "/g.h", "int g_h(void);\n");
+    write_file(SRC "/f.c", "/* g_h */\nconst char *f_n = \"g_h\";\nint f_k(void) { return 0; }\n");
+    write_file(SRC "/g.c", "#include \"f.h\"\nint g_h(void) { return f_k(); }\n");
+    assert(run_residue(TREE) == 0);
+    assert(strcmp(read_file(OUT), "usage-mutuel\t1\t2\n"
+                                  "résidu 2 : 2 ligne(s), 2 fichier(s)\n") == 0);
+    assert(run_residue("--detail " TREE) == 0);
+    assert(strcmp(read_file(OUT), "usage-mutuel\ta\tb\n"
+                                  "résidu 2 : 2 ligne(s), 2 fichier(s)\n") == 0);
+    assert(run_residue("--check inclusion-mutuelle,usage-mutuel " TREE) == 1);
+    assert(strcmp(read_file(OUT), "usage-mutuel: " SRC "/a.c:4:    return b_g();\n"
+                                  "usage-mutuel: " SRC "/b.c:3:    return a_f();\n"
+                                  "résidu gardé (inclusion-mutuelle,usage-mutuel) : 2 ligne(s)\n") == 0);
+    assert(system("rm " SRC "/a.c") == 0);
+    assert(run_residue("--check usage-mutuel " TREE) == 0);
+    assert(strcmp(read_file(OUT), "résidu gardé (usage-mutuel) : 0 ligne(s)\n") == 0);
+}
+
 static void test_detail_sorts_by_category_then_file(void) {
     setup();
     write_file(SRC "/b.c", "q = pico8_" "qh_of(b);\n");
@@ -685,6 +727,7 @@ int main(void) {
     test_detail_prints_r2_sites_under_void_ptr_and_the_info_line();
     test_attainment_list_counts_only_useful_lines();
     test_mutual_inclusion_counts_its_include_lines_in_the_footer();
+    test_usage_pairs_see_a_detour_not_a_shared_name_nor_a_comment();
     test_detail_sorts_by_category_then_file();
     test_check_fails_only_on_a_named_category_present();
     test_measure_on_nothing_is_an_error_not_an_empty_residue();
diff --git a/tools/composition_residue.sh b/tools/composition_residue.sh
index c0ecfb8..bc91993 100644
--- a/tools/composition_residue.sh
+++ b/tools/composition_residue.sh
@@ -6,13 +6,18 @@
 # atteinte-à-prouver compte les lignes utiles de COMPOSITION_ATTEINTE ;
 # inclusion-mutuelle compte des paires de modules hors tests/ ; au pied et a
 # --check, ses lignes sont les #include qui realisent ces paires.
+# usage-mutuel compte les paires de modules hors tests/ dont chacun emploie
+# un nom que l'en-tete de l'autre est seul a declarer, include ou detour ;
+# ses lignes sont, par paire et par sens, le premier emploi.
 #   sans option : "<cat>\t<lignes>\t<fichiers>" par categorie non nulle
-#                 ("<paires>\t<modules>" pour inclusion-mutuelle), puis
+#                 ("<paires>\t<modules>" pour inclusion-mutuelle et
+#                 usage-mutuel), puis
 #                 "résidu 2 : N ligne(s), M fichier(s)" (couples fichier:ligne
 #                 et fichiers distincts des categories de SORTIE) ; code 0
 #                 quel que soit le compte.
 #   --detail    : "<cat>\t<fichier>\t<lignes>" par couple non nul,
-#                 "inclusion-mutuelle\t<a>\t<b>" par paire, sous void-ptr les
+#                 "<cat>\t<a>\t<b>" par paire d'inclusion-mutuelle et
+#                 d'usage-mutuel, sous void-ptr les
 #                 "void-ptr-r2\t<fonction>\t<amont>" de COMPOSITION_R2, la
 #                 ligne "info" de pico8_game_data.h s'il est lu, puis le meme
 #                 pied ; code 0.
@@ -38,10 +43,10 @@
 
 set -u
 
-CATEGORIES="parente lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux void-ptr ptrs-entité registres noyau-inclut atteinte-à-prouver inclusion-mutuelle"
+CATEGORIES="parente lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux void-ptr ptrs-entité registres noyau-inclut atteinte-à-prouver inclusion-mutuelle usage-mutuel"
 
 # Categories comptees dans le pied.
-SORTIE="parente lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux void-ptr ptrs-entité registres noyau-inclut atteinte-à-prouver inclusion-mutuelle"
+SORTIE="parente lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux void-ptr ptrs-entité registres noyau-inclut atteinte-à-prouver inclusion-mutuelle usage-mutuel"
 
 fail() {
     echo "composition_residue: $1" >&2
@@ -483,6 +488,106 @@ read_inclusions() {
     sort -t: -k1,1 -k2,2n "$work/inclusion-mutuelle" > "$work/sorted" && mv "$work/sorted" "$work/inclusion-mutuelle"
 }
 
+# Graphe d'usage de l'etude 2.11 (usegraph.py) : a -> b quand un .c/.h de a
+# hors tests/ emploie un nom que b.h est seul a declarer, commentaires et
+# chaines retires ; il voit la paire qu'un detour cache a inclusion-mutuelle.
+# Noms lus par motif : #define, prototype (un niveau de parentheses
+# internes), (*nom), dernier nom d'un typedef ou d'un extern hors des
+# accolades, enumerateur ; pas les etiquettes de struct. Un nom que deux
+# en-tetes declarent n'est a aucun : les etiquettes, lues sans cette regle,
+# feraient six paires par les declarations anticipees de pico8_struct.h.
+# Rend dans $work/usage-mutuel.pairs les paires, dans $work/usage-mutuel le
+# premier emploi de chaque sens (ordre de $work/files, puis des lignes).
+read_usages() {
+    grep -v '/tests/' "$work/files" | grep -v '^tests/' > "$work/modfiles" || true
+    : > "$work/usage-mutuel.pairs"
+    : > "$work/usage-mutuel"
+    [ -s "$work/modfiles" ] || return 0
+    grep '\.h$' "$work/modfiles" > "$work/modheaders" || true
+    awk -v pairs="$work/usage-mutuel.pairs" "$CLEAN_AWK"'
+    function mod(path,    m) {
+        m = path; sub(/.*\//, "", m); sub(/\.[ch]$/, "", m)
+        return m
+    }
+    function declare(h, x) {
+        if (x == "" || x == "PICO8" || x == "PICO8_" || x ~ /^__/ || ((x, h) in seen)) return
+        seen[x, h] = 1; nown[x]++; own[x] = h
+    }
+    function declare_all(    h, t, m, x) {
+        for (h in full) {
+            t = tops[h]
+            while (match(t, /[A-Za-z0-9_]+[ \t]*\(([^()]|\([^()]*\))*\)[ \t]*;/)) {
+                x = substr(t, RSTART, RLENGTH); t = substr(t, RSTART + RLENGTH)
+                sub(/[^A-Za-z0-9_].*$/, "", x)
+                if (x !~ /^(if|for|while|return|sizeof|switch|void|int|char|const|unsigned)$/) declare(h, x)
+            }
+            t = tops[h]
+            while (match(t, /\([ \t]*\*[ \t]*[A-Za-z0-9_]+[ \t]*\)/)) {
+                x = substr(t, RSTART, RLENGTH); t = substr(t, RSTART + RLENGTH)
+                gsub(/[^A-Za-z0-9_]/, "", x); declare(h, x)
+            }
+            t = tops[h]
+            while (match(t, /typedef[ \t][^;{}]*[A-Za-z0-9_][ \t]*;/)) {
+                x = substr(t, RSTART, RLENGTH); t = substr(t, RSTART + RLENGTH)
+                sub(/[ \t]*;$/, "", x); match(x, /[A-Za-z0-9_]+$/); declare(h, substr(x, RSTART))
+            }
+            t = tops[h]
+            while (match(t, /extern[ \t][^;(]*[A-Za-z0-9_][ \t]*(\[[^]]*\])?[ \t]*;/)) {
+                x = substr(t, RSTART, RLENGTH); t = substr(t, RSTART + RLENGTH)
+                sub(/[ \t]*;$/, "", x); sub(/\[[^]]*\]$/, "", x); sub(/[ \t]*$/, "", x)
+                match(x, /[A-Za-z0-9_]+$/); declare(h, substr(x, RSTART))
+            }
+            t = full[h]
+            while (match(t, /enum[ \t]*[A-Za-z0-9_]*[ \t]*\{[^}]*\}/)) {
+                m = substr(t, RSTART, RLENGTH); t = substr(t, RSTART + RLENGTH)
+                sub(/^[^{]*\{[ \t]*/, "", m); sub(/[ \t]*\}$/, "", m)
+                while (match(m, /[A-Za-z0-9_]+[ \t]*(=[^,]*)?(,|$)/)) {
+                    x = substr(m, RSTART, RLENGTH); m = substr(m, RSTART + RLENGTH)
+                    sub(/[^A-Za-z0-9_].*$/, "", x); declare(h, x)
+                }
+            }
+        }
+    }
+    FNR == 1 { incomment = 0 }
+    pass == 1 {
+        h = mod(FILENAME); c = clean($0); full[h] = full[h] c " "
+        l = c
+        while (match(l, /#define[ \t]+[A-Za-z0-9_]+/)) {
+            x = substr(l, RSTART, RLENGTH); l = substr(l, RSTART + RLENGTH)
+            sub(/^#define[ \t]+/, "", x); declare(h, x)
+        }
+        l = c
+        while (match(l, /[{}]/)) {
+            if (hdepth[h] == 0) tops[h] = tops[h] substr(l, 1, RSTART - 1)
+            hdepth[h] += substr(l, RSTART, 1) == "{" ? 1 : -1
+            l = substr(l, RSTART + 1)
+        }
+        if (hdepth[h] == 0) tops[h] = tops[h] l " "
+        next
+    }
+    !done { declare_all(); done = 1 }
+    {
+        a = mod(FILENAME); l = clean($0)
+        while (match(l, /[A-Za-z0-9_]+/)) {
+            x = substr(l, RSTART, RLENGTH); l = substr(l, RSTART + RLENGTH)
+            if (x ~ /^[0-9]/ || !(x in nown) || nown[x] != 1 || own[x] == a) continue
+            if (!((a, own[x]) in site)) site[a, own[x]] = FILENAME ":" FNR ":" $0
+        }
+    }
+    END {
+        for (k in site) {
+            split(k, ab, SUBSEP)
+            if (ab[1] < ab[2] && ((ab[2], ab[1]) in site)) {
+                print ab[1] "\t" ab[2] > pairs
+                print site[k]; print site[ab[2], ab[1]]
+            }
+        }
+    }
+    ' pass=1 $(cat "$work/modheaders") pass=2 $(cat "$work/modfiles") > "$work/usage-mutuel" || fail "echec de awk"
+    sort "$work/usage-mutuel.pairs" > "$work/sorted" && mv "$work/sorted" "$work/usage-mutuel.pairs"
+    sort -u -t: -k1,1 -k2,2n "$work/usage-mutuel" > "$work/sorted" && mv "$work/sorted" "$work/usage-mutuel"
+}
+
 # Rend dans $work/<cat> les lignes "<fichier>:<ligne>:<texte>", dans l'ordre
 # des fichiers puis des lignes. grep rend 1 quand rien ne correspond.
 # L'exclusion est un sed sur le texte, jamais un filtre sur la ligne entiere :
@@ -496,6 +601,7 @@ collect() {
         void-ptr) read_prototypes; return 0 ;;
         registres) read_registres; return 0 ;;
         inclusion-mutuelle) read_inclusions; return 0 ;;
+        usage-mutuel) read_usages; return 0 ;;
         atteinte-à-prouver)
             status=0
             grep -nE '^[^#[:space:]]' "$ATTEINTE" > "$work/numbers" || status=$?
@@ -603,9 +709,9 @@ for cat in $CATEGORIES; do
         case " $SORTIE " in
             *" $cat "*) cut -d: -f1,2 "$work/$cat" >> "$work/all" ;;
         esac
-        if [ "$cat" = inclusion-mutuelle ] && [ "$mode" = detail ]; then
-            sed 's/^/inclusion-mutuelle\t/' "$work/$cat.pairs"
-        elif [ "$cat" = inclusion-mutuelle ]; then
+        if [ -f "$work/$cat.pairs" ] && [ "$mode" = detail ]; then
+            sed "s/^/$cat\t/" "$work/$cat.pairs"
+        elif [ -f "$work/$cat.pairs" ]; then
             pairs=$(wc -l < "$work/$cat.pairs" | tr -d ' ')
             modules=$(tr '\t' '\n' < "$work/$cat.pairs" | sort -u | wc -l | tr -d ' ')
             printf '%s\t%s\t%s\n' "$cat" "$pairs" "$modules"
```
