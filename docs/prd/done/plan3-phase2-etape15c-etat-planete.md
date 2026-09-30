---
title: "Plan 3, étape 2.15c : l'état des sortes monte dans `pico8_jo_t`, `pico8_qh_of`/`pico8_nl_of` remplacés par les bits `QH`/`NL`, membre `parent` retiré ; `lien-parent` 42 → 0, gardé — un commit, jeu identique, aucune empreinte changée"
description: "Les neuf champs d'état des sortes (`s`, `qw`, `iu`, `ms`, `ht`, `probe`, `probe_own`, `emitter`, `emitter_own`) deviennent des membres de `pico8_jo_t` ; les neuf structs de sorte restent, réduites à leur ascendante enchâssée (2.15d les retire). Les huit lectures de `src/` qui descendaient par `pico8_qh_of`/`pico8_nl_of` lisent le bit de la table et le champ sur la planète ; les deux descentes, leurs marches, les neuf `.parent` et le membre `parent` de la table partent ; `lien-parent` entre dans `COMPOSITION_RESIDUE_GUARD`. `pico8_jo_struct_init` pose `probe`/`emitter` à NULL. Essayé sur copie : vert, 26 e2e (aussi sous perturbation), 14 jumelles sans ÉCHEC (dont les trois lignes K de 2.15b sur `QH`/`NL`, devenues de vraies jumelles) et 3 lignes K rouges, masque égal aux descentes sur les 40 tables, valeurs observables identiques. Aucune question à l'humain."
status: needs-triage
---

## Problem Statement

**Écrit sur `4947c55`** (branche `claude/plan-pilot-3-7zdft0`). `4947c55` ne diffère de `cecbcdb` que dans `docs/` (`git diff --stat cecbcdb 4947c55 -- src tests Makefile tools` vide). 2.15a et 2.15b n'étaient pas commités au départ : l'arbre des essais est `cecbcdb` + les deux diffs du PRD `docs/prd/triage/plan3-phase2-etape15ab-liens-sonde-lignee.md`, appliqués tels quels. Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier. Essais sur une seule copie jetable (scratchpad `prd215c/copie`, ses deux arbres de travail `wt_b`, `wt_c` et un export `tree_b`, supprimés depuis). Notation : `b` = `f09dfd2`, le commit de la copie qui porte `cecbcdb` + diff 2.15a + diff 2.15b (l'état d'après 2.15b) ; `c` = `64b3c9e`, le commit de la copie qui porte le diff de « Ce qui change ». Numéros de ligne : ceux de `b`, sauf mention. Pendant l'écriture, 2.15a et 2.15b ont été commités sur la branche (`f726cd8`, `c239f47`) : `git diff --stat f09dfd2 c239f47 -- src tests Makefile tools` est vide, donc `b` est `c239f47` pour tout ce qui suit, et le diff ci-dessous s'applique sur `c239f47`.

Vu du mainteneur du port : après 2.15b, trois choses tiennent encore la chaîne `parent` des tables de classe. (1) Les champs d'état propres aux sortes (`s` et la sonde de la lignée `qh`, `qw` de `nl`, l'émetteur de `hj`, `iu`/`ms`/l'émetteur de `ga`, `ht` de `mx`) vivent dans les structs de sorte : un lecteur générique qui tient un `pico8_jo_t *` doit d'abord « descendre » vers la sorte. (2) Cette descente passe par `pico8_qh_of`/`pico8_nl_of`, qui remontent la chaîne `cls->parent()`. (3) La chaîne existe pour elles seules : neuf `.parent` dans les tables de sorte et le membre `parent` de `pico8_struct_class_t`. 2.15c monte les champs dans la planète, lit les bits `QH`/`NL` que 2.15b a posés et épinglés, et retire la chaîne. 2.15d retirera ensuite les neuf structs de sorte, qui ne portent plus rien d'autre que leur ascendante.

### Texte du plan, recopié (`docs/plan-3.md` à `4947c55`)

> - 2.15 — `parente` et `lien-parent`, selon l'étude 2.10 (§2). Sonde et émetteur par deux liens du noyau ; porteur de sorte : un masque `lineage` dans la table de classe (bits `JO`, `QH`, `NL`, `G_`), équivalent aux descentes sur les douze classes (étude, contrôle 4) ; `pico8_jo_is_family` garde son nom et lit le bit `JO`. Forme sous A : un type planète de 33 membres (432 octets), un type sonde de 8 (264 octets), structs complètes dans l'étude. (2.9 : les archétypes ont déjà la forme aplatie. L'aplatissement ne les réécrit pas.) Preuves : chemin touché (mutations : lien `probe` NULL sur une sorte de la lignée `qh`, bit `QH` ôté de `nr`, bit `JO` ôté de `ga` → rouge) ; TU d'équivalence réécrit sur le masque ; 2.15d, objets sans `-g` identiques pour les modules dont seul un chemin change, sinon chemin touché. `class-coverage-net` : oui (comptes égaux à `tools/class_coverage_net_expected.txt`). `sanitize-net` : oui. Budget ≈ 4,5 × 50 min.
>   - 2.15c — les champs d'état des sortes (`s`, `qw`, `iu`, `ms`, `ht`, `probe`, `probe_own`, `emitter`, `emitter_own`) montent dans `pico8_jo_t` ; `pico8_qh_of`/`pico8_nl_of` remplacés par les bits `QH`/`NL` ; membre `parent`, ses neuf écrivains et ses marches retirés. Compte : `lien-parent` → **0**, gardé.
>   - 2.15d — les neuf structs de sorte disparaissent (`planet_family.h:32`, `:51`, `:62`, `:72`, `:88`, `:102`, `planet_family_trade.h:23`, `:34`, `:45`) : chemins enchâssés et casts réécrits, ≈ 37 lignes de `src/` et ≈ 165 de TU estimées après 2.13. Changement de type indivisible : budget ≈ 1,5 × 50 min, déclaré. Compte : `parente` → **0**, gardé. (2.11 : retire aussi `planet_display.c:18`, `#include "pico8_planet_family.h"`, dont plus aucun nom n'est employé après 2.15c/2.15d ; fait tomber `planet_display` ↔ `planet_family` : 16 / 40 → **15 / 38**, usage 19 → 18. `planet` → `planet_family` ne garde que le registre `kv`, coupé à 2.17e.)

Décisions de l'humain (même fichier) : **planètes, A** (2026-09-30) — « un seul type "planète" pour les dix sortes, qui porte tous leurs champs, la sorte n'étant plus que sa table de méthodes et son archétype » : 2.15c fait porter à `pico8_jo_t` tout l'état des sortes, 2.15d retire les structs de sorte. **Q-2.10, oui** — « chaque objet reçoit les liens de composant du noyau » : aucun lien neuf ici ; `probe`/`emitter` du noyau restent posés par le porteur seul. **Q-2.14d, oui** — « faux » et « absent » sont le même 0 : sans objet ici (aucun `has_X` ne bouge). Notes « Découvert à 2.10 » : « porteur de sorte : un masque `lineage` […], équivalent aux quatre descentes sur les douze classes » ; « planète aplatie : 56 noms → 20 à l'archétype, 3 retirés par 2.14, 33 membres (432 octets) » ; étude 2.10, §2 : les 33 membres sont « les 24 autres de `jo`, `s`, `qw`, `iu`, `ms`, `ht`, `probe`, `probe_own`, `emitter`, `emitter_own` », avec la struct complète (`s` « sortes de la lignée qh », `qw` « nl », `iu, ms` « ga », `ht` « mx », `probe` « NULL hors de la lignée qh », `emitter` « NULL hors de hj, pm, mx, ga »). Notes « Découvert à 2.11 » : `planet_display` ↔ `planet_family` tombe à 2.15d ; « trois autres attendent 2.14c ou 2.15c/d pour leur dernier symbole et se coupent à 2.17e » (dont `planet` → `planet_family`, sites `planet.c:269`, `:280`, à `9ca66f4`).

Étude 2.10, §2 (ce que ce PRD suit) : table « Découpage des 111 lignes » (2.15c − 42 : `pico8_qh_of`, `pico8_nl_of`, `.parent` des neuf sortes, marches, membre `parent`) ; « `pico8_qh_of(e)`, `pico8_nl_of(e)` : les lecteurs tiennent déjà la planète […] ou un en-tête qu'ils testent par `pico8_jo_is_family` ; ils lisent le bit `QH` ou `NL` et les champs sur la planète » ; « `test_pico8_qh.c:705-724` fait déjà ce contrôle pour `qh_of`/`nl_of` : 2.15c le réécrit sur le masque » ; « d'abord les champs d'état des sortes montent dans `pico8_jo_t` et le masque remplace les descentes (le lien parent tombe à 0), puis les neuf structs de sorte disparaissent ». Étude 2.11 : fiche 6 (`planet` → `planet_family` : `pico8_qh_of` part à 2.15c, `pico8_qh_t` à 2.15d) ; fiche 10 (`planet_display` → `planet_family` : `pico8_qh_of`, `pico8_nl_of` à 2.15c, `pico8_qh_t`, `pico8_nl_t` à 2.15d ; paire coupée à 2.15d avec `planet_display.c:18`).

Règles communes du plan : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée ; `class-coverage-net` et `sanitize-net` : oui, oui (ligne 2.15). Ce PRD les tient toutes.

### Contrats reçus et ce que ce PRD en fait

| Point (source) | Ce que fait 2.15c |
|---|---|
| « `lien-parent` 42 / 17 à `b` » : 30 lignes `pico8_qh_of`/`pico8_nl_of` (`src/` 11, TU 19), neuf `.parent`, deux marches (`planet_family.c:565`, `:574`), membre `parent` (`struct.h:166`) (PRD 2.15a–b, contrat pour 2.15c) | tenu à la ligne près : les 42 sortent (attribution ci-dessous), `lien-parent` **0**, entré dans `COMPOSITION_RESIDUE_GUARD` |
| « bits `QH`, `NL` posés dans les dix tables de planète et épinglés par `test_qh_of_follows_parent` contre les descentes » ; « retirer le côté descente du test » ; « mutations "bit `QH` ôté de `nr`", "bit `NL` ôté de `nl`" […] lignes K » (idem) | le test devient `test_lineage_of_each_class` : le masque seul, dix sortes, `ga`, `jo`, `g_`, `fv`, `qb`, `bg`, `mz`, `jc`. **Les lignes K de 2.15b deviennent de vraies jumelles** : `K-QH-nr` → QH-nr, `K-NL-nl` → NL-nl, `K-QH-jo` → N-jo-QH (preuve 3) — les bits ont maintenant des lecteurs dans `src/`, et la mutation équivalente au parent (descente faussée) existe |
| « liens mutables `emitter`, `probe` visent toujours `&self->X_own`, posés à côté du membre ; 2.15c les pose dans `pico8_jo_struct_init` si les membres montent dans `pico8_jo_t` (ils restent à poser pour les seules sortes porteuses) » (idem) | les **liens** restent posés par les seuls porteurs (`pico8_qh_struct_init`, `pico8_hj_struct_init`, `_new` de `ga`), vers `&self->jo.X_own` ; `pico8_jo_struct_init` pose les **membres** `probe`/`emitter` à NULL (non-porteur : « lien = membre » = NULL). Le TU `struct_header` le vérifie sur `jo`, `ga` et les huit sortes |
| « le test de sonde est `e->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_G_)` suivi du cast » (idem) | même forme pour `QH`/`NL` ; dans `src/`, les lecteurs tiennent déjà un `pico8_jo_t *` et n'ont plus de cast à faire, sauf `pico8_da` (un en-tête, un cast) |
| « TU `struct_header` : `planet_links(jo)`, `assert_qh_links(qh, emitter)` […] les chemins `&fr->nr.qh.probe_own`, `&pm->hj.qh`… deviennent des chemins aplatis à 2.15d » (idem) | chemins transitoires `&fr->nr.qh.jo.probe_own`, `&pm->hj.qh.jo.emitter_own` ; `assert_qh_links` compare aussi `emitter` au membre |
| « tailles à `b` : noyau 128, sonde 256, planètes 304 à 416 » ; « la forme A de l'étude (planète 432) se mesure à 2.15d » (idem) | mesurée dès `c` : les dix types planète font **424** octets (un `pico8_probe_t` fait 24 octets depuis 2.13b, pas 48 comme au temps de l'étude) ; 2.15d ne change plus que les chemins |
| `particle.c:12` (`pico8_planet.h`) sans usage : pour 2.17 (idem) | non touché ; c'est pourquoi `planet.h` n'inclut pas `particle.h` (G1) |
| Étude 2.11, fiche 10 : `planet_display` ↔ `planet_family` coupée à 2.15d avec `planet_display.c:18` | la paire d'**usage** tombe dès `c` (plus aucun nom de `planet_family.h` employé par `planet_display`) ; la ligne `#include` reste pour 2.15d, comme le plan le dit (G4) ; essai : la retirer à `c` compile |

### État de départ attendu (`c239f47`, ou un descendant qui ne touche que `docs/`)

Mesuré sur `b` (identique à la table « Comptes » du PRD 2.15a–b, colonne `b`) :

| Commande | Résultat |
|---|---|
| `make composition-residue` | `parente 9 2`, `lien-parent 42 17`, `void-ptr 3 3`, `inclusion-mutuelle 16 15` ; **`résidu 2 : 94 ligne(s), 38 fichier(s)`** |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 240 ; 0 écriture : 15 ; 0 hors constructeur : 20` |
| `--check inclusion-mutuelle src tests \| tail -1` | `résidu gardé (inclusion-mutuelle) : 40 ligne(s)` |
| `COMPOSITION_RESIDUE_GUARD` (`Makefile:874`) | `noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6` |
| casts / tests de classe : `grep -rEo '\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)' src tests \| wc -l` ; `grep -rEo '(->cls\|header\.cls\|_struct_class\(\))' src tests \| wc -l` | 484 / 636 |
| `sizeof` (x86-64) : noyau, `g_`, `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `ga`, `nr`, `fr` ; `pico8_probe_t`, `pico8_emitter_t` | 128, 256, 304, 344, 352, 344, 408, 408, 416, 376, 344, 344 ; 24, 56 |
| graphe d'usage (`usegraph.py`, annexe A du PRD 2.11) | `paires 19` |
| `make test` ; `make e2e` | vert ; 26 conformes |

Contrôle rapide du départ : `git diff --stat c239f47 <départ> -- src tests Makefile tools` vide ; `grep -c 'pico8_qh_of\|pico8_nl_of' src/*.[ch]` total 11 ; `grep -rn '\.parent = ' src | wc -l` → 9 ; `grep -rn '\.lineage = ' src | wc -l` → 12. Un départ autre : arrêt, rapporté (le diff est alors à rejouer, `git apply --check` le dit).

## Solution

Un commit, vert, jeu identique (26 e2e conformes, aussi sous `MALLOC_PERTURB_=165`), aucune constante d'empreinte changée. `lien-parent` 42 → **0**, gardé.

- **`pico8_jo_t`** (`planet.h`) gagne, après `kw_own` et dans l'ordre de la struct de l'étude, `s`, `qw`, `iu`, `ms`, `ht`, `probe`, `emitter`, `probe_own`, `emitter_own`, chacun avec la sorte qui le lit en commentaire (contrat). `planet.h` inclut `pico8_probe.h` (le type `pico8_probe_t` ; `pico8_emitter_t` vient par `probe.h` → `particle.h`, G1). Le commentaire de la struct dit qu'un champ de sorte est indéterminé sur une planète d'une autre sorte et que `probe`/`emitter` y sont NULL.
- **Structs de sorte** (`planet_family.h`, `planet_family_trade.h`) : `pico8_qh_t {pico8_jo_t jo;}`, `pico8_nl_t {pico8_qh_t qh;}`, `pico8_hj_t {pico8_qh_t qh;}`, `pico8_ga_t {pico8_jo_t jo;}`, `pico8_mx_t {pico8_hj_t hj;}` ; `os`, `pm`, `nr`, `fr` inchangées. Commentaires réécrits sans « `probe` points at `probe_own`: never copy the struct » (dit une fois, sur `pico8_jo_t`). Les neuf restent (2.15d).
- **Écrivains et lecteurs dans les modules de sorte** : chaque `self->s`, `self->qh.s`, `self->emitter`, `self->ht`, `&self->probe_own`… devient le chemin vers le membre de `jo` (`self->jo.s`, `self->qh.jo.s`, `self->qh.jo.emitter`, `self->hj.qh.jo.ht`, `&self->jo.probe_own`…). Réécriture guidée par le compilateur (236 sites dans `src/` et les TU, un préfixe inséré devant le membre selon le type de l'erreur). Les liens du noyau sont posés au même endroit qu'à `b`, vers `&self->jo.probe_own`/`&self->jo.emitter_own`.
- **`pico8_jo_struct_init`** pose `self->probe = NULL` et `self->emitter = NULL` (dernières lignes). Rien d'autre : `s`, `qw`, `iu`, `ms`, `ht` restent indéterminés hors de leur sorte, qui les pose (G2).
- **Descentes remplacées** (8 lectures de `src/`, toutes sur un en-tête ou une planète déjà en main) :
  - `pico8_jo_pq` (`planet.c:234`) : `if (self->header.cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH))`, puis `self->def->hp` ;
  - `pico8_jo_gh` (`planet_display.c:154`, `:205`) : bit `QH`, puis `self->def->px`, `self->def->qh` (la lecture de `:205` réutilisait la variable `qh` de `:154` : elle n'était pas comptée par `lien-parent`) ;
  - `pico8_jo_kj` (`planet_display.c:429`, `:436`) : bit `NL` pour `self->qw`, bit `QH` pour `self->s` ;
  - `pico8_jo_cn` (`planet_trade.c:151`) : bit `QH`, puis `self->def->ba` ;
  - `pico8_da` (`economy_score.c:47`) : `pt != NULL && (pt->cls->lineage & …QH) && ((const pico8_jo_t *)pt)->def->qh` — garde NULL conservée (`pico8_qh_of(NULL)` rendait NULL, et `call_gg_truthy` accepte NULL) ;
  - `pico8_dh` (`economy_tiers.c:261`) : `pt->cls->lineage & …NL` — `pt` est non NULL et planète (`:212-216`).
- **Retirés** : `pico8_qh_of`, `pico8_nl_of` (définitions `planet_family.c:561-577`, prototypes et commentaires `planet_family.h:107-113`), les neuf `.parent = pico8_X_struct_class,` (`planet_family.c:46`, `:117`, `:161`, `:220`, `:307`, `:451` ; `planet_family_trade.c:178`, `:232`, `:326`), le membre `parent` de `pico8_struct_class_t` (`struct.h:166`) et sa phrase du commentaire de table (`:143-144`).
- **Garde** : `lien-parent` ajouté en fin de `COMPOSITION_RESIDUE_GUARD` (`Makefile:874`). La catégorie reste dans `CATEGORIES`/`SORTIE` du script (à 0).
- **TU** : chemins transitoires (`qh->jo.s`, `hj->qh.jo.emitter`, `mx->hj.qh.jo.ht`…) dans les TU de l'arbre `jo` ; les huit descentes des TU deviennent un test de bit (`as_qh` ×5, `is_of`, `is_processor`, `is_mx`, `is_pm`, `is_plate`, et `as_nl`, aide neuve de `test_pico8_nl.c` pour `fold_plate`/`fold_dg`) ; `test_qh_of_follows_parent` → `test_lineage_of_each_class`, masque seul ; `test_pico8_struct_header.c` : `jo->probe == NULL && jo->emitter == NULL`, `assert_qh_links` compare aussi `emitter` au membre, `ga->jo.probe == NULL`.
- **Glossaire** : la ligne `iq` (`cls->parent`, qui n'existe plus) dit `— (cls->lineage)`.

Rien d'autre : ni `trous-de-portage.md`, ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni `planet_display.c:18` (2.15d), ni `particle.c:12` (2.17).

## User Stories

1. En tant que mainteneur, je veux lire `s`, `qw`, `iu`, `ms`, `ht`, la sonde et l'émetteur d'une planète sur la planète elle-même, afin qu'aucun lecteur générique ne descende vers une struct de sorte.
2. En tant que mainteneur, je veux que « est-ce un laboratoire ? une plaque ? » soit un bit de la table de classe, afin que plus rien ne remonte une chaîne `parent`.
3. En tant qu'auteur de 2.15d, je veux que les structs de sorte ne portent plus que leur ascendante, afin de n'avoir que des chemins à réécrire.
4. En tant que gardien du compteur, je veux `lien-parent` à 0 et gardé, afin qu'une descente ou un `.parent` réintroduits rougissent `make test`.
5. En tant que gardien de la règle du parent, je veux que chaque mutation rouge au parent (descente faussée) le soit à l'enfant (bit faussé), et que les trois lignes K de 2.15b deviennent des jumelles.
6. En tant que gardien de la mémoire, je veux qu'une planète non porteuse ait `probe`/`emitter` à NULL, vérifié sous perturbation.

## Implementation Decisions

### Modules

`struct.h` ; `planet.h`, `planet.c` ; `planet_family.h`/`.c` ; `planet_family_trade.h`/`.c` ; `planet_display.c` ; `planet_trade.c` ; `economy_score.c` ; `economy_tiers.c` ; `Makefile` (une ligne) ; `docs/glossaire-noms.md` (une ligne) ; TU `fr`, `ga`, `hj`, `jo_spec_defaults`, `mx`, `nl`, `nr`, `os`, `pm`, `qh`, `struct_header`.

### Inclusions

Une ligne ajoutée dans `src/` : `planet.h:11`, `#include "pico8_probe.h"` (type de `probe_own`). `planet` ↔ `probe` est déjà une paire (`planet.c:21`, `probe.c:18`) : **16 paires / 15 modules inchangés**, le `--check` passe de 40 à **41** lignes. Inclure aussi `particle.h` créerait la paire `planet` ↔ `particle` (`particle.c:12` inclut `planet.h`) : exclu, `pico8_emitter_t` vient par `probe.h` (G1). Aucune ligne retirée. Graphe d'usage **19 → 18** : `planet_display` ↔ `planet_family` sort (`planet_display` n'emploie plus `pico8_qh_of`, `pico8_nl_of`, `pico8_qh_t`, `pico8_nl_t` ; il garde son include jusqu'à 2.15d) ; `planet` → `planet_family` perd `pico8_qh_of`, `pico8_qh_t` (reste `pico8_ga_kind`, `pico8_pm_kind`, le registre `kv`), `planet_family` → `planet` perd `pico8_jo_struct_class` (les `.parent` de `qh`, `ga`) ; `planet` → `probe` gagne `pico8_probe_t`. Aucune paire n'entre.

### Ce qui change

Le diff ci-dessous est le texte exact (`git diff b c`) ; `git apply --check` puis `git apply` sur `b` rendent `c` à l'identique (Runs 23). Lecture, en plus de la Solution :

- **Un champ, une place** : chaque champ de sorte a un seul stockage, dans `pico8_jo_t` ; la sorte le pose dans son init comme à `b` (même valeur, même ordre). Le programme de la preuve 5 lit, sur dix planètes et six non-planètes, chaque champ, les liens, « lien = membre » et ce qu'en tirent `pico8_da`, `pico8_dh`, `pico8_jo_kj`, `pico8_jo_gh` : identique à `b` et `c`, aussi sous perturbation.
- **Bit, pas descente** : les deux descentes rendaient la même instance castée vers la sorte ; le bit dit la même chose (preuve 4, 40 tables) et le champ est déjà sur la planète : aucun cast neuf dans `src/` hors `pico8_da`, et deux de moins (ceux des descentes).
- **Écrivains inchangés** : preuve `const` (preuve 2) — les champs montés, déclarés `const` dans une copie, ne sont écrits qu'aux mêmes 13 sites de `src/` et 1 de TU à `b` et à `c`, plus, à `c`, les deux poses à NULL de `pico8_jo_struct_init`.
- **Chemins transitoires** : `self->hj.qh.jo.emitter` est long ; il ne vit que jusqu'à 2.15d, qui retire les structs de sorte. Les réécrire deux fois est le prix du découpage de l'étude (la chaîne tombe avant le changement de type indivisible).
- **Commentaires** (règle de `CLAUDE.md`) : celui de `pico8_jo_t` porte le contrat de disposition (indéterminé hors de la sorte, NULL, « never copy the struct ») ; les sept premiers membres neufs nomment la sorte qui les lit (contrat) ; ceux des structs de sorte perdent la phrase « points at `*_own` » ; celui de la table perd la phrase `parent`.

#### Diff de 2.15c (`git diff b c`)

```diff
diff --git a/Makefile b/Makefile
index 7ca5c70..30351cc 100644
--- a/Makefile
+++ b/Makefile
@@ -871,7 +871,7 @@ composition-residue: tools/composition_residue.sh
 	@sh tools/composition_residue.sh
 
 # Catégories du compteur de composition tombées à 0, séparées par des virgules.
-COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6
+COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6,lien-parent
 
 composition-guard: tools/composition_residue.sh
 	@sh tools/composition_residue.sh --check $(COMPOSITION_RESIDUE_GUARD) src tests
diff --git a/docs/glossaire-noms.md b/docs/glossaire-noms.md
index a0ba6ca..62a3b49 100644
--- a/docs/glossaire-noms.md
+++ b/docs/glossaire-noms.md
@@ -271,7 +271,7 @@ Conventions de lecture : « classe » = classe `ke`-descendante du système
 | Nom | Symbole C | Sens |
 |---|---|---|
 | `fz` | `cls->name` | nom de la classe ; les classes sans `fz` se marchent dessus dans le registre, comme dans l'original |
-| `iq` | `cls->parent` | classe parente ; absente pour une racine |
+| `iq` | — (`cls->lineage`) | classe parente ; absente pour une racine. Sans lien parent ici : le masque `lineage` de la table dit si la classe est `jo`, `qh`, `nl`, `g_` ou en descend |
 | `ns` | — | métatable d'instance de l'original ; sans équivalent ici : chaque `pico8_X_struct_new` recopie les défauts de classe (`pico8_X_start`) dans les membres du struct, qu'on lit ensuite directement |
 | `pq` | `cls->pq` | constructeur supplémentaire ; chaque `pq` distinct de la chaîne est appelé une fois |
 | `cw` | champ | nom de classe lu à l'exécution, pour le re-classement dynamique de `jo:hu` |
diff --git a/src/pico8_economy_score.c b/src/pico8_economy_score.c
index 32db7e6..fab8376 100644
--- a/src/pico8_economy_score.c
+++ b/src/pico8_economy_score.c
@@ -44,9 +44,7 @@ static int call_gg_truthy(pico8_struct_header_t *pt) {
 }
 
 pico8_fixed_t pico8_da(pico8_struct_header_t *pt) {
-    const pico8_qh_t *qh = pico8_qh_of(pt);
-
-    if (qh != NULL && qh->jo.def->qh) {
+    if (pt != NULL && (pt->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)) && ((const pico8_jo_t *)pt)->def->qh) {
         const pico8_ents_t *qh_list = pico8_cg("qh");
         return pico8_fixed_from_int(qh_list != NULL ? qh_list->count : 0);
     }
diff --git a/src/pico8_economy_tiers.c b/src/pico8_economy_tiers.c
index 9c8fc96..0689eb9 100644
--- a/src/pico8_economy_tiers.c
+++ b/src/pico8_economy_tiers.c
@@ -258,7 +258,7 @@ pico8_fixed_t pico8_dh(const pico8_struct_header_t *pt) {
         return pico8_fixed_from_int(1);
     }
 
-    if (pico8_nl_of(pt) != NULL) {
+    if (pt->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_NL)) {
         int count = 0;
         if (jy != NULL) {
             for (i = 0; i < jy->count; i++) {
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index f3fbb9a..92d93c3 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -127,6 +127,8 @@ void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, con
     self->header.m_ = &self->m_;
     self->m_ = 0;
     self->kw_own = (pico8_kw_t){0};
+    self->probe = NULL;
+    self->emitter = NULL;
 }
 
 static pico8_jo_t *pico8_jo_struct_new(const pico8_jo_keys_t *keys) {
@@ -220,7 +222,6 @@ pico8_jo_t *pico8_jo_new(const pico8_jo_keys_t *keys) {
 }
 
 void pico8_jo_pq(pico8_jo_t *self) {
-    const pico8_qh_t *qh;
     pico8_ents_t *oe;
 
     if (self == NULL) {
@@ -231,9 +232,8 @@ void pico8_jo_pq(pico8_jo_t *self) {
     pico8_ents_init(oe);
     self->oe = oe;
 
-    qh = pico8_qh_of(&self->header);
-    if (qh != NULL) {
-        pico8_hp_t *target = pico8_gb_at(pico8_fixed_flr_int(qh->jo.def->hp));
+    if (self->header.cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)) {
+        pico8_hp_t *target = pico8_gb_at(pico8_fixed_flr_int(self->def->hp));
 
         if (target != NULL) {
             pico8_jo_lz(self, target);
diff --git a/src/pico8_planet.h b/src/pico8_planet.h
index 395f64a..44c4a5a 100644
--- a/src/pico8_planet.h
+++ b/src/pico8_planet.h
@@ -8,6 +8,7 @@
 #include "pico8_geom.h"
 #include "pico8_ke.h"
 #include "pico8_layout.h"
+#include "pico8_probe.h"
 #include "pico8_ptrs.h"
 #include "pico8_research.h"
 #include "pico8_struct.h"
@@ -19,8 +20,11 @@
 /* A bare planet: its kind's archetype (`kw` points at the one kw every bare
  * planet shares, written in place by `bb` and `mx:kf`, until `lz` points it at
  * the planet's own `kw_own`), its construction keys `id`/`p`/`ra`, then every
- * field jo's functions write on it. Every planet struct begins with one, so
- * jo's functions take it for `self`. Nothing frees the struct: TROU-7. */
+ * field jo's functions write on it, then its kinds' fields. Every planet
+ * struct begins with one, so jo's functions take it for `self`. A kind's
+ * field is indeterminate on a planet of another kind; `probe`, `emitter` are
+ * NULL there, else they point at `probe_own`, `emitter_own`: never copy the
+ * struct. Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_struct_header_t header;
     const pico8_jo_def_t *def;
@@ -48,6 +52,14 @@ typedef struct {
     int has_mi;
     int m_;
     pico8_kw_t kw_own;
+    pico8_fixed_t s;           /* qh's lineage */
+    pico8_fixed_t qw;          /* nl */
+    pico8_fixed_t iu, ms;      /* ga */
+    const pico8_int_seq_t *ht; /* mx */
+    pico8_probe_t *probe;      /* qh's lineage */
+    pico8_emitter_t *emitter;  /* hj, pm, mx, ga */
+    pico8_probe_t probe_own;
+    pico8_emitter_t emitter_own;
 } pico8_jo_t;
 
 /* TROU-8 */
diff --git a/src/pico8_planet_display.c b/src/pico8_planet_display.c
index 96f1851..94b3503 100644
--- a/src/pico8_planet_display.c
+++ b/src/pico8_planet_display.c
@@ -134,7 +134,6 @@ void pico8_jo_gh(const pico8_jo_t *self) {
     pico8_lq_keys_t h_keys;
     pico8_lq_cell_t h_item;
     int n = 0;
-    const pico8_qh_t *qh;
     const pico8_jb_row_t *icon_row = NULL;
     pico8_lq_keys_t row_keys[PICO8_JO_GH_ROWS];
     pico8_lq_cell_t *row_items[PICO8_JO_GH_ROWS];
@@ -151,9 +150,8 @@ void pico8_jo_gh(const pico8_jo_t *self) {
 
     memset(bs, 0, sizeof bs);
     memset(row_keys, 0, sizeof row_keys);
-    qh = pico8_qh_of(&self->header);
-    if (qh != NULL && qh->jo.def->px != NULL) {
-        icon_row = qh->jo.def->px;
+    if ((self->header.cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)) && self->def->px != NULL) {
+        icon_row = self->def->px;
     } else if (self->has_mi) {
         int mi = pico8_fixed_flr_int(self->mi);
 
@@ -202,7 +200,7 @@ void pico8_jo_gh(const pico8_jo_t *self) {
         qe_is_r_w = kw != NULL && kw->has_qe && pico8_r_lookup("w", &r_w) && kw->qe == pico8_fixed_from_int(r_w);
 
         if (mi_fixed < pico8_fixed_from_int(4) && jy != NULL && jy->count > 0 && !qe_is_r_w) {
-            int last_only = qh != NULL && qh->jo.def->qh;
+            int last_only = (self->header.cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)) && self->def->qh;
 
             row_cap[n] = 1 + (last_only ? 1 : jy->count) + 1;
             row_items[n] = jo_gh_row_items(row_cap[n]);
@@ -401,8 +399,6 @@ void pico8_jo_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec
     int px;
     int py;
     int spr_index = 43;
-    const pico8_qh_t *qh;
-    const pico8_nl_t *nl;
 
     if (self == NULL || !pico8_gd((const pico8_struct_header_t *)self)) {
         return;
@@ -426,18 +422,16 @@ void pico8_jo_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec
     pico8_circfill(gfx, mem, px, py, 7, 0);
 
     /* An absent `qw` reads as 0, i.e. the default palette block. */
-    nl = pico8_nl_of(&self->header);
-    pico8_dv(mem, nl != NULL ? nl->qw : pico8_fixed_from_int(0));
+    pico8_dv(mem, (self->header.cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_NL)) ? self->qw : pico8_fixed_from_int(0));
     pico8_palt(mem, 3, 0);
     pico8_palt(mem, 0, 1);
 
     /* `A and B or C`: `nk.s` is never absent, so only a nil `nk` falls
      * through to `self.s`, then to the literal 43. */
-    qh = pico8_qh_of(&self->header);
     if (self->nk != NULL) {
         spr_index = self->nk->s;
-    } else if (qh != NULL) {
-        spr_index = pico8_fixed_flr_int(qh->s);
+    } else if (self->header.cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)) {
+        spr_index = pico8_fixed_flr_int(self->s);
     }
     pico8_spr(gfx, mem, spr_index, px - 4, py - 4, 2, 2);
     pico8_dv_default(mem);
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index cfe7f24..f432f85 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -43,20 +43,19 @@ static const pico8_struct_class_t g_qh_struct_class = {
             [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
             [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
         },
-    .parent = pico8_jo_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
 void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
     pico8_jo_struct_init(&self->jo, cls, def);
-    self->jo.header.probe = &self->probe_own;
-    self->probe = &self->probe_own;
-    self->probe->def = &self->jo.def->probe;
-    self->probe->od = self->jo.def->probe.od;
-    self->probe->has_od = self->jo.def->probe.has_od;
-    self->s = self->jo.def->s;
-    self->probe->et = NULL;
+    self->jo.header.probe = &self->jo.probe_own;
+    self->jo.probe = &self->jo.probe_own;
+    self->jo.probe->def = &self->jo.def->probe;
+    self->jo.probe->od = self->jo.def->probe.od;
+    self->jo.probe->has_od = self->jo.def->probe.has_od;
+    self->jo.s = self->jo.def->s;
+    self->jo.probe->et = NULL;
 }
 
 static pico8_qh_t *pico8_qh_struct_new(const pico8_jo_keys_t *keys) {
@@ -88,8 +87,9 @@ void pico8_nl_ly(pico8_nl_t *self, pico8_fixed_t t) {
 
     assert(self->qh.jo.has_mi && "nl:ly reads self.mi unguarded, unlike hj:kj");
     if (self->qh.jo.has_mi && self->qh.jo.mi >= pico8_fixed_from_int(2)) {
-        self->qw = pico8_fixed_add(pico8_fixed_from_int(8),
-                                   pico8_fixed_div(pico8_fixed_mod(t, pico8_fixed_from_int(42)), pico8_fixed_from_int(6)));
+        self->qh.jo.qw =
+            pico8_fixed_add(pico8_fixed_from_int(8),
+                            pico8_fixed_div(pico8_fixed_mod(t, pico8_fixed_from_int(42)), pico8_fixed_from_int(6)));
     }
 }
 
@@ -114,7 +114,6 @@ static const pico8_struct_class_t g_nl_struct_class = {
             [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
             [PICO8_STATE_LY] = pico8_nl_ly_dispatch,
         },
-    .parent = pico8_qh_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH) | PICO8_LINEAGE(PICO8_LINEAGE_NL),
 };
@@ -124,7 +123,7 @@ static pico8_nl_t *pico8_nl_struct_new(const pico8_jo_keys_t *keys) {
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
     pico8_qh_struct_init(&self->qh, &g_nl_struct_class, &pico8_nl_def);
-    self->qw = self->qh.jo.def->qw;
+    self->qh.jo.qw = self->qh.jo.def->qw;
     if (keys != NULL) {
         assert(keys->p == NULL && keys->ra == NULL &&
                "nl: the only construction key is id (ie:ldown); another key has no member "
@@ -158,7 +157,6 @@ static const pico8_struct_class_t g_os_struct_class = {
             [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
             [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
         },
-    .parent = pico8_qh_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
@@ -217,25 +215,24 @@ static const pico8_struct_class_t g_hj_struct_class = {
             [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
             [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
         },
-    .parent = pico8_qh_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
 void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
     pico8_qh_struct_init(&self->qh, cls, def);
-    self->qh.jo.header.emitter = &self->emitter_own;
-    self->emitter = &self->emitter_own;
-    self->emitter->def = &self->qh.jo.def->emitter;
-    self->emitter->py = self->qh.jo.def->emitter.py;
-    self->emitter->pn = self->qh.jo.def->emitter.pn;
-    self->emitter->k_ = self->qh.jo.def->emitter.k_;
-    self->emitter->du = self->qh.jo.def->emitter.du;
-    self->emitter->qo = self->qh.jo.def->emitter.qo;
-    self->emitter->ro = self->qh.jo.def->emitter.ro;
-    self->emitter->qg = self->qh.jo.def->emitter.qg;
-    self->emitter->q_ = self->qh.jo.def->emitter.q_;
-    self->emitter->qy = NULL;
+    self->qh.jo.header.emitter = &self->qh.jo.emitter_own;
+    self->qh.jo.emitter = &self->qh.jo.emitter_own;
+    self->qh.jo.emitter->def = &self->qh.jo.def->emitter;
+    self->qh.jo.emitter->py = self->qh.jo.def->emitter.py;
+    self->qh.jo.emitter->pn = self->qh.jo.def->emitter.pn;
+    self->qh.jo.emitter->k_ = self->qh.jo.def->emitter.k_;
+    self->qh.jo.emitter->du = self->qh.jo.def->emitter.du;
+    self->qh.jo.emitter->qo = self->qh.jo.def->emitter.qo;
+    self->qh.jo.emitter->ro = self->qh.jo.def->emitter.ro;
+    self->qh.jo.emitter->qg = self->qh.jo.def->emitter.qg;
+    self->qh.jo.emitter->q_ = self->qh.jo.def->emitter.q_;
+    self->qh.jo.emitter->qy = NULL;
 }
 
 static pico8_hj_t *pico8_hj_struct_new(const pico8_jo_keys_t *keys) {
@@ -304,7 +301,6 @@ static const pico8_struct_class_t g_pm_struct_class = {
             [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
             [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
         },
-    .parent = pico8_hj_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
@@ -369,9 +365,9 @@ pico8_qy_t *pico8_ga_mm(void *self_ptr) {
     }
     p = pico8_vec_qm(qd, pico8_fixed_rnd_default());
 
-    ms = self->ms;
-    iu = self->iu;
-    py = self->emitter->py;
+    ms = self->jo.ms;
+    iu = self->jo.iu;
+    py = self->jo.emitter->py;
 
     v = pico8_vec_mul(p, ms);
     a = pico8_vec_mul(p, iu);
@@ -397,7 +393,7 @@ void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ga_t *self, pico8_vec
         return;
     }
 
-    qy = self->emitter->qy;
+    qy = self->jo.emitter->qy;
 
     px = pico8_fixed_flr_int(p.x);
     py = pico8_fixed_flr_int(p.y);
@@ -448,7 +444,6 @@ static const pico8_struct_class_t g_ga_struct_class = {
     .cn = pico8_jo_cn_dispatch,
     .mm = pico8_ga_mm,
     .states = {[PICO8_STATE_LL] = pico8_jo_ll_dispatch},
-    .parent = pico8_jo_struct_class,
     /* ga's cart list of tags is empty and replaces jo's: never jo's. */
     .tags = 0,
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),
@@ -459,18 +454,18 @@ static pico8_ga_t *pico8_ga_struct_new(const pico8_ga_keys_t *keys) {
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
     pico8_jo_struct_init(&self->jo, &g_ga_struct_class, &pico8_ga_def);
-    self->jo.header.emitter = &self->emitter_own;
-    self->emitter = &self->emitter_own;
-    self->emitter->def = &self->jo.def->emitter;
-    self->emitter->qo = self->jo.def->emitter.qo;
-    self->emitter->ro = self->jo.def->emitter.ro;
-    self->emitter->qg = self->jo.def->emitter.qg;
-    self->emitter->q_ = self->jo.def->emitter.q_;
-    self->emitter->k_ = self->jo.def->emitter.k_;
-    self->iu = self->jo.def->iu;
-    self->emitter->py = self->jo.def->emitter.py;
-    self->ms = self->jo.def->ms;
-    self->emitter->qy = NULL;
+    self->jo.header.emitter = &self->jo.emitter_own;
+    self->jo.emitter = &self->jo.emitter_own;
+    self->jo.emitter->def = &self->jo.def->emitter;
+    self->jo.emitter->qo = self->jo.def->emitter.qo;
+    self->jo.emitter->ro = self->jo.def->emitter.ro;
+    self->jo.emitter->qg = self->jo.def->emitter.qg;
+    self->jo.emitter->q_ = self->jo.def->emitter.q_;
+    self->jo.emitter->k_ = self->jo.def->emitter.k_;
+    self->jo.iu = self->jo.def->iu;
+    self->jo.emitter->py = self->jo.def->emitter.py;
+    self->jo.ms = self->jo.def->ms;
+    self->jo.emitter->qy = NULL;
     self->jo.by = &g_ga_by_shared;
     if (keys != NULL) {
         if (keys->jo.p != NULL) {
@@ -483,14 +478,14 @@ static pico8_ga_t *pico8_ga_struct_new(const pico8_ga_keys_t *keys) {
             self->jo.id = keys->jo.id;
         }
         if (keys->pq != NULL) {
-            self->emitter->qo = keys->pq->qo;
-            self->emitter->ro = keys->pq->ro;
-            self->emitter->qg = keys->pq->qg;
-            self->emitter->q_ = keys->pq->q_;
-            self->emitter->k_ = keys->pq->k_;
-            self->iu = keys->pq->iu;
-            self->emitter->py = keys->pq->py;
-            self->ms = keys->pq->ms;
+            self->jo.emitter->qo = keys->pq->qo;
+            self->jo.emitter->ro = keys->pq->ro;
+            self->jo.emitter->qg = keys->pq->qg;
+            self->jo.emitter->q_ = keys->pq->q_;
+            self->jo.emitter->k_ = keys->pq->k_;
+            self->jo.iu = keys->pq->iu;
+            self->jo.emitter->py = keys->pq->py;
+            self->jo.ms = keys->pq->ms;
         }
     }
 #ifdef PICO8_CLASS_COVERAGE
@@ -558,24 +553,6 @@ pico8_nl_t *pico8_nl_new(const pico8_jo_keys_t *keys) {
     return pico8_nl_struct_new(keys);
 }
 
-const pico8_qh_t *pico8_qh_of(const pico8_struct_header_t *e) {
-    const pico8_struct_class_t *cls = e != NULL ? e->cls : NULL;
-
-    while (cls != NULL && cls != &g_qh_struct_class) {
-        cls = cls->parent != NULL ? cls->parent() : NULL;
-    }
-    return cls != NULL ? (const pico8_qh_t *)e : NULL;
-}
-
-const pico8_nl_t *pico8_nl_of(const pico8_struct_header_t *e) {
-    const pico8_struct_class_t *cls = e != NULL ? e->cls : NULL;
-
-    while (cls != NULL && cls != &g_nl_struct_class) {
-        cls = cls->parent != NULL ? cls->parent() : NULL;
-    }
-    return cls != NULL ? (const pico8_nl_t *)e : NULL;
-}
-
 int pico8_os_ready(void) {
     return g_family_up;
 }
diff --git a/src/pico8_planet_family.h b/src/pico8_planet_family.h
index a943ee6..e998bac 100644
--- a/src/pico8_planet_family.h
+++ b/src/pico8_planet_family.h
@@ -22,22 +22,17 @@ void pico8_planet_family_init(void);
  * before the first `pico8_planet_family_init()`. */
 void pico8_planet_family_shutdown(void);
 
-/* A laboratory: a bare planet, then qh's members copied from its kind's
+/* A laboratory: a planet whose `s` and probe are copied from its kind's
  * archetype, the probe's with `et`, which its own `hq` (`bv`) writes.
- * `probe` points at `probe_own`: never copy the struct. Nothing frees the
- * struct: TROU-7. */
+ * Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_jo_t jo;
-    pico8_fixed_t s;
-    pico8_probe_t *probe;
-    pico8_probe_t probe_own;
 } pico8_qh_t;
 
-/* A trading hub: a laboratory, then `qw`, copied from its archetype and
+/* A trading hub: a laboratory whose `qw` is copied from its archetype and
  * written by its own `ly`. Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_qh_t qh;
-    pico8_fixed_t qw;
 } pico8_nl_t;
 
 /* A processor: a laboratory and nothing of its own; its defaults are its
@@ -46,14 +41,11 @@ typedef struct {
     pico8_qh_t qh;
 } pico8_os_t;
 
-/* An ascension: a laboratory, then an emitter copied from its kind's
- * archetype, with `qy`, which its own `pq` (`f_`) writes. `emitter` points
- * at `emitter_own`: never copy the struct. Nothing frees the struct:
- * TROU-7. */
+/* An ascension: a laboratory whose emitter is copied from its kind's
+ * archetype, with `qy`, which its own `pq` (`f_`) writes. Nothing frees the
+ * struct: TROU-7. */
 typedef struct {
     pico8_qh_t qh;
-    pico8_emitter_t *emitter;
-    pico8_emitter_t emitter_own;
 } pico8_hj_t;
 
 /* A protostar: an ascension and nothing of its own; its defaults are its
@@ -62,19 +54,14 @@ typedef struct {
     pico8_hj_t hj;
 } pico8_pm_t;
 
-/* A particle burst: a bare planet, then `iu`, `ms` and an emitter copied from
+/* A particle burst: a planet whose `iu`, `ms` and emitter are copied from
  * its archetype, with `qy`, which its own `pq` (`f_`) writes; the emitter's
  * `pn`, `du` are left unwritten, read by `g_:mm` only. `jo.by` is the one list
  * every burst shares, which `ib` extends in place and each init empties. A
  * burst spawned by `pm:pq` takes `iu`, `ms`, `py`, `k_`, `qo`, `ro`, `qg`, `q_`
- * from its keys instead. `emitter` points at `emitter_own`: never copy the
- * struct. Nothing frees the struct: TROU-7. */
+ * from its keys instead. Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_jo_t jo;
-    pico8_fixed_t iu;
-    pico8_fixed_t ms;
-    pico8_emitter_t *emitter;
-    pico8_emitter_t emitter_own;
 } pico8_ga_t;
 
 /* Every `_ready` below is 1 from `pico8_planet_family_init()` to
@@ -104,14 +91,6 @@ const pico8_struct_class_t *pico8_nl_struct_class(void);
  * nothing ever frees it. Requires `pico8_planet_family_init()`. */
 pico8_nl_t *pico8_nl_new(const pico8_jo_keys_t *keys);
 
-/* `e` itself when its class is qh's or has qh's up its `parent` chain; NULL
- * otherwise, and for a NULL `e`. */
-const pico8_qh_t *pico8_qh_of(const pico8_struct_header_t *e);
-
-/* `e` itself when its class is nl's or has nl's up its `parent` chain; NULL
- * otherwise, and for a NULL `e`. */
-const pico8_nl_t *pico8_nl_of(const pico8_struct_header_t *e);
-
 int pico8_os_ready(void);
 const pico8_entity_kind_t *pico8_os_kind(void);
 const pico8_struct_class_t *pico8_os_struct_class(void);
diff --git a/src/pico8_planet_family_trade.c b/src/pico8_planet_family_trade.c
index d422b63..ce5ce11 100644
--- a/src/pico8_planet_family_trade.c
+++ b/src/pico8_planet_family_trade.c
@@ -97,7 +97,7 @@ void pico8_nr_hb(pico8_nr_t *self) {
         }
     }
 
-    self->qh.s = s_val;
+    self->qh.jo.s = s_val;
 }
 
 static void pico8_nr_hb_dispatch(void *self) {
@@ -175,7 +175,6 @@ static const pico8_struct_class_t g_nr_struct_class = {
             [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
             [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
         },
-    .parent = pico8_qh_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
@@ -229,7 +228,6 @@ static const pico8_struct_class_t g_fr_struct_class = {
             [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
             [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
         },
-    .parent = pico8_nr_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
@@ -287,13 +285,13 @@ void pico8_mx_kf(pico8_mx_t *self, pico8_struct_header_t *sw) {
                 continue;
             }
 
-            if (i->row.has_qe && self->ht != NULL && pico8_int_seq_has_fixed(self->ht, i->row.qe)) {
+            if (i->row.has_qe && self->hj.qh.jo.ht != NULL && pico8_int_seq_has_fixed(self->hj.qh.jo.ht, i->row.qe)) {
                 pico8_kw_t *kw = self->hj.qh.jo.kw;
                 if (kw != NULL) {
                     kw->has_qe = 1;
                     kw->qe = i->row.qe;
                 }
-                self->ht = &g_mx_ht_spent;
+                self->hj.qh.jo.ht = &g_mx_ht_spent;
             }
         }
     }
@@ -323,7 +321,6 @@ static const pico8_struct_class_t g_mx_struct_class = {
             [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
             [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
         },
-    .parent = pico8_hj_struct_class,
     .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
@@ -333,7 +330,7 @@ static pico8_mx_t *pico8_mx_struct_new(const pico8_jo_keys_t *keys) {
 
     assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
     pico8_hj_struct_init(&self->hj, &g_mx_struct_class, &pico8_mx_def);
-    self->ht = self->hj.qh.jo.def->ht;
+    self->hj.qh.jo.ht = self->hj.qh.jo.def->ht;
     if (keys != NULL) {
         assert(keys->p == NULL && keys->ra == NULL &&
                "mx: the only construction key is id (ie:ldown); another key has no member "
diff --git a/src/pico8_planet_family_trade.h b/src/pico8_planet_family_trade.h
index 130f769..ced2f16 100644
--- a/src/pico8_planet_family_trade.h
+++ b/src/pico8_planet_family_trade.h
@@ -26,12 +26,11 @@ typedef struct {
     pico8_nr_t nr;
 } pico8_fr_t;
 
-/* A synthesizer: an ascension, then `ht`, copied from its archetype;
+/* A synthesizer: an ascension whose `ht` is copied from its archetype;
  * `mx:kf` points it at an empty sequence once it accepts a resource.
  * Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_hj_t hj;
-    const pico8_int_seq_t *ht;
 } pico8_mx_t;
 
 /* As in pico8_planet_family.h: `_ready` from
diff --git a/src/pico8_planet_trade.c b/src/pico8_planet_trade.c
index 3333b1c..183de4a 100644
--- a/src/pico8_planet_trade.c
+++ b/src/pico8_planet_trade.c
@@ -148,12 +148,10 @@ void pico8_jo_cn(pico8_jo_t *self, pico8_jo_trade_proposal_t *lw) {
         qe_val = pico8_jy_qe(i);
 
         if (have_r_q && pico8_qe_equal(qe_val, r_q)) {
-            const pico8_qh_t *qh = pico8_qh_of(&self->header);
             int accepted = 0;
 
-            if (qh != NULL && qh->jo.def->ba != NULL) {
-                accepted = t_qe.kind == PICO8_QE_FIXED &&
-                           pico8_int_seq_has_fixed(qh->jo.def->ba, t_qe.n);
+            if ((self->header.cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)) && self->def->ba != NULL) {
+                accepted = t_qe.kind == PICO8_QE_FIXED && pico8_int_seq_has_fixed(self->def->ba, t_qe.n);
             }
             if (accepted) {
                 assert(t_qe.kind == PICO8_QE_FIXED && "a want's qe is a resource of r, never a boolean");
diff --git a/src/pico8_struct.h b/src/pico8_struct.h
index 162fa76..06f6ea4 100644
--- a/src/pico8_struct.h
+++ b/src/pico8_struct.h
@@ -140,8 +140,7 @@ struct pico8_kind_rec;
  * such method. `kj`, `ev` and `hd` read the vectors they get and never keep
  * them; `ldown`, `rdown` and `hq` may keep theirs. NULL `ra`: nil. `has_hb`
  * is NULL when a non-NULL `hb` is enough for `e.hb` to exist; otherwise the
- * instance decides. `mm` returns a fresh particle the caller owns. `parent` returns the class
- * whose struct this one embeds first; the member is NULL for a root. */
+ * instance decides. `mm` returns a fresh particle the caller owns. */
 struct pico8_struct_class {
     void (*kj)(void *self, const pico8_vec_t *ra);
     void (*ev)(void *self, const pico8_vec_t *ra);
@@ -163,7 +162,6 @@ struct pico8_struct_class {
     unsigned tags;
     /* `PICO8_LINEAGE(l)` set: the class is l's or descends from it. */
     unsigned lineage;
-    const pico8_struct_class_t *(*parent)(void);
 };
 
 /* The construction keys of the jo tree (the cart's `mw(ob)` in `kv:jr`).
diff --git a/tests/test_pico8_fr.c b/tests/test_pico8_fr.c
index bb0e184..e10b10f 100644
--- a/tests/test_pico8_fr.c
+++ b/tests/test_pico8_fr.c
@@ -145,7 +145,9 @@ static int ents_list_size(const pico8_ents_t *l) {
 
 /* `e` as a planet of qh's lineage. */
 static pico8_qh_t *as_qh(void *e) {
-    assert(pico8_qh_of(e) == e);
+    const pico8_struct_header_t *h = e;
+
+    assert(h != NULL && (h->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)));
     return e;
 }
 
@@ -258,11 +260,11 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     h = fold_string_member(h, nr->qh.jo.def->rr);
     h = fold_fixed_member(h, fr->nr.qh.jo.def->mt);
     h = fold_fixed_member(h, qh->jo.def->hp);
-    h = fold_fixed_member(h, qh->s);
-    h = fold_fixed_member(h, qh->probe->od);
-    h = fold_fixed_member(h, qh->probe->def->fp);
-    h = fold_fixed_member(h, qh->probe->def->dw);
-    h = fold_string_member(h, qh->probe->def->dq);
+    h = fold_fixed_member(h, qh->jo.s);
+    h = fold_fixed_member(h, qh->jo.probe->od);
+    h = fold_fixed_member(h, qh->jo.probe->def->fp);
+    h = fold_fixed_member(h, qh->jo.probe->def->dw);
+    h = fold_string_member(h, qh->jo.probe->def->dq);
     h = fold_qh(h, qh);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
@@ -280,14 +282,14 @@ static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
     h = fnv(h, (uint32_t)(nr->qh.jo.def->rr != NULL));
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
-    h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
+    h = fnv(h, qh->jo.probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -506,17 +508,17 @@ static void test_life(void) {
     for (i = 0; i <= 3; i++) {
         as_qh(placed)->jo.oe = list_of(i, idle);
         cls_of(placed)->hb(placed);
-        h = fold_fixed_member(h, as_qh(placed)->s);
+        h = fold_fixed_member(h, as_qh(placed)->jo.s);
         h = fold_bool(h, cls_of(placed)->gv(placed, NULL).stop);
     }
-    assert(as_qh(placed)->s == pico8_fixed_from_int(76));
+    assert(as_qh(placed)->jo.s == pico8_fixed_from_int(76));
     jc = cls_of(placed)->ldown(placed, NULL, NULL);
     assert(jc.held == NULL);
     h = fold_bool(h, jc.stop);
     as_qh(placed)->jo.oe = oe;
     cls_of(placed)->hb(placed);
-    h = fold_fixed_member(h, as_qh(placed)->s);
-    assert(as_qh(placed)->s == pico8_fixed_from_int(78));
+    h = fold_fixed_member(h, as_qh(placed)->jo.s);
+    assert(as_qh(placed)->jo.s == pico8_fixed_from_int(78));
     h = fold_economy(h, placed);
 
     h = fold_route(h, placed);
diff --git a/tests/test_pico8_ga.c b/tests/test_pico8_ga.c
index 8a56c96..09b3210 100644
--- a/tests/test_pico8_ga.c
+++ b/tests/test_pico8_ga.c
@@ -292,7 +292,7 @@ static void *spawn(int x, int y, const char *id, void **protostar) {
  * and `jo`, where `lt` registered it -- in `hv` alone -- and `mo.ma`. */
 static uint32_t fold_burst(uint32_t h, void *e, int from_pm) {
     const pico8_ga_t *g = as_ga(e);
-    const pico8_emitter_t *em = g->emitter;
+    const pico8_emitter_t *em = g->jo.emitter;
     const pico8_jo_t *jo = &g->jo;
     int j;
 
@@ -304,9 +304,9 @@ static uint32_t fold_burst(uint32_t h, void *e, int from_pm) {
     h = fold_fixed_member(h, em->ro);
     h = fold_fixed_member(h, em->qg);
     h = fold_fixed_member(h, em->q_);
-    h = fold_fixed_member(h, g->iu);
+    h = fold_fixed_member(h, g->jo.iu);
     h = fold_fixed_member(h, em->py);
-    h = fold_fixed_member(h, g->ms);
+    h = fold_fixed_member(h, g->jo.ms);
     h = fold_hf(h, jo);
     h = fold_string_member(h, jo->header.me);
     h = fold_fixed_member(h, jo->header.t);
@@ -423,13 +423,14 @@ static void test_built(void) {
     c = pico8_ga_new(NULL);
     h = fold_burst(h, c, 0);
     assert(a != b && b != c && a != c);
-    assert(as_ga(a)->emitter->qy != as_ga(b)->emitter->qy && as_ga(b)->emitter->qy != as_ga(c)->emitter->qy);
+    assert(as_ga(a)->jo.emitter->qy != as_ga(b)->jo.emitter->qy &&
+           as_ga(b)->jo.emitter->qy != as_ga(c)->jo.emitter->qy);
     assert(as_ga(a)->jo.oe != as_ga(b)->jo.oe);
     assert(as_ga(a)->jo.by == as_ga(b)->jo.by && as_ga(b)->jo.by == as_ga(c)->jo.by);
     assert(as_ga(c)->jo.ra == NULL && as_ga(c)->jo.p == NULL && as_ga(c)->jo.id == NULL);
-    assert(as_ga(a)->jo.def->fa == pico8_fixed_from_int(3) && as_ga(a)->ms == pico8_fixed_from_int(3));
+    assert(as_ga(a)->jo.def->fa == pico8_fixed_from_int(3) && as_ga(a)->jo.ms == pico8_fixed_from_int(3));
     assert(as_ga(a)->jo.def->hf == 0);
-    assert(as_ga(b)->ms == PICO8_FIXED_LIT(0x00018000, 1.5));
+    assert(as_ga(b)->jo.ms == PICO8_FIXED_LIT(0x00018000, 1.5));
     check("built", h, BUILT_HASH);
 }
 
@@ -523,7 +524,7 @@ static uint32_t fold_lq(uint32_t h, void *e) {
 /* `kj` on `e` with its `qy` cut in place to `n` particles: `p_` after. */
 static uint32_t fold_cut(uint32_t h, void *e, int n) {
     pico8_ga_t *g = as_ga(e);
-    pico8_ptrs_t *qy = g->emitter->qy;
+    pico8_ptrs_t *qy = g->jo.emitter->qy;
 
     assert(qy != NULL && qy->count >= n);
     qy->count = n;
@@ -562,10 +563,10 @@ static void test_life(void) {
         h = fold_p_(h, &as_ga(a)->jo);
         h = fold_p_(h, &as_ga(b)->jo);
         h = fold_p_(h, &as_ga(c)->jo);
-        h = fnv(h, (uint32_t)ptrs_size(as_ga(b)->emitter->qy));
+        h = fnv(h, (uint32_t)ptrs_size(as_ga(b)->jo.emitter->qy));
     }
 
-    as_ga(b)->emitter->qy->count = 3;
+    as_ga(b)->jo.emitter->qy->count = 3;
     for (i = 0; i < 3; i++) {
         pico8_dc("kj");
         h = fnv(h, screen_hash());
@@ -647,7 +648,7 @@ static void test_ei_moves_a_particle_in_place(void) {
 
     start_game();
     e = spawn(84, 60, "s73", NULL);
-    em = as_ga(e)->emitter;
+    em = as_ga(e)->jo.emitter;
     assert(em->qg != 0);
     particle.p = p0;
     particle.v = v0;
diff --git a/tests/test_pico8_hj.c b/tests/test_pico8_hj.c
index c76a384..bea9884 100644
--- a/tests/test_pico8_hj.c
+++ b/tests/test_pico8_hj.c
@@ -166,7 +166,7 @@ static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
 
 /* 1 no target, 3 a target: the bytes the frozen hashes fold. */
 static uint32_t et_state(const pico8_qh_t *qh) {
-    return qh->probe->et == NULL ? 1u : 3u;
+    return qh->jo.probe->et == NULL ? 1u : 3u;
 }
 
 /* `et` as qh's reader gave it: false or the target. */
@@ -175,7 +175,7 @@ static uint32_t fold_et(uint32_t h, const pico8_qh_t *qh) {
     case 1u:
         return fold_bool_member(h, 0);
     default:
-        return fold_ref(fnv(h, 1u), qh->probe->et);
+        return fold_ref(fnv(h, 1u), qh->jo.probe->et);
     }
 }
 
@@ -187,8 +187,8 @@ static pico8_hj_t *as_hj(void *e) {
 }
 
 static uint32_t fold_pk(uint32_t h, const pico8_hj_t *hj) {
-    return hj->qh.jo.header.cls == pico8_pm_struct_class() ? fold_bool_member(h, hj->emitter->def->pk != 0)
-                                                           : fold_fixed_member(h, hj->emitter->def->pk);
+    return hj->qh.jo.header.cls == pico8_pm_struct_class() ? fold_bool_member(h, hj->qh.jo.emitter->def->pk != 0)
+                                                           : fold_fixed_member(h, hj->qh.jo.emitter->def->pk);
 }
 
 /* A particle record as `g_.mm` returns it: `p`, `v`, `l`. */
@@ -291,8 +291,8 @@ static void *build_ascension(void) {
 /* `pm` and `mx` each set their own `px`: an entity whose `px` is the very
  * row `px` is one of that kind. */
 static int is_of(const void *e, const pico8_jb_row_t *px) {
-    const pico8_qh_t *qh = pico8_qh_of(e);
-    return qh != NULL && qh->jo.def->px == px;
+    const pico8_jo_t *jo = e != NULL && (cls_of(e)->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)) ? e : NULL;
+    return jo != NULL && jo->def->px == px;
 }
 
 /* An ascension as `ie:ldown`, `f_` and `jo:pq` left it: its scalars (the 12
@@ -302,7 +302,7 @@ static int is_of(const void *e, const pico8_jb_row_t *px) {
  * and where `lt` registered it -- in `cg.hj`, never in `cg.qh`. */
 static uint32_t fold_ascension(uint32_t h, void *e) {
     const pico8_hj_t *hj = as_hj(e);
-    const pico8_emitter_t *em = hj->emitter;
+    const pico8_emitter_t *em = hj->qh.jo.emitter;
     const pico8_qh_t *qh = &hj->qh;
     const pico8_jo_t *jo = &qh->jo;
     size_t i;
@@ -313,8 +313,8 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = fold_string_member(h, hj->qh.jo.def->rr);
     h = fold_qh(h, qh);
     h = fold_fixed_member(h, qh->jo.def->hp);
-    h = fold_fixed_member(h, qh->s);
-    h = fold_fixed_member(h, qh->probe->od);
+    h = fold_fixed_member(h, qh->jo.s);
+    h = fold_fixed_member(h, qh->jo.probe->od);
     h = fold_fixed_member(h, em->py);
     h = fold_fixed_member(h, em->def->ki);
     h = fold_fixed_member(h, em->pn);
@@ -325,9 +325,9 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = fold_fixed_member(h, em->ro);
     h = fold_fixed_member(h, em->qg);
     h = fold_fixed_member(h, em->q_);
-    h = fold_string_member(h, qh->probe->def->dq);
-    h = fold_fixed_member(h, qh->probe->def->fp);
-    h = fold_fixed_member(h, qh->probe->def->dw);
+    h = fold_string_member(h, qh->jo.probe->def->dq);
+    h = fold_fixed_member(h, qh->jo.probe->def->fp);
+    h = fold_fixed_member(h, qh->jo.probe->def->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
@@ -344,7 +344,7 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(hj->emitter->k_ != NULL));
+    h = fnv(h, (uint32_t)(hj->qh.jo.emitter->k_ != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -352,14 +352,14 @@ static uint32_t fold_ascension(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
-    h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
+    h = fnv(h, qh->jo.probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -431,7 +431,7 @@ static void test_built(void) {
     a = build_ascension();
     b = build_ascension();
     assert(a != b && is_of(a, &pico8_hj_px_row) && is_of(b, &pico8_hj_px_row));
-    assert(as_hj(a)->emitter->qy != NULL && as_hj(a)->emitter->qy != as_hj(b)->emitter->qy);
+    assert(as_hj(a)->qh.jo.emitter->qy != NULL && as_hj(a)->qh.jo.emitter->qy != as_hj(b)->qh.jo.emitter->qy);
     h = fold_ascension(h, a);
     h = fold_ascension(h, b);
     check("built", h, BUILT_HASH);
@@ -485,7 +485,7 @@ static uint32_t fold_economy(uint32_t h, void *e) {
 static uint32_t fold_draw(uint32_t h, void *e) {
     pico8_dc("kj");
     h = fnv(h, screen_hash());
-    return fnv(h, (uint32_t)ptrs_size(as_hj(e)->emitter->qy));
+    return fnv(h, (uint32_t)ptrs_size(as_hj(e)->qh.jo.emitter->qy));
 }
 
 /* `hq` (`bv`) on screen then `ey` (`d_`) with the credits; `bb`/`ib`/`gg`/
@@ -527,8 +527,8 @@ static void test_life(void) {
 
     h = fold_particle(h, cls_of(placed)->mm(placed));
     pico8_ez(placed, PICO8_FIXED_LIT(0x00028000, 2.5));
-    h = fnv(h, (uint32_t)ptrs_size(as_hj(placed)->emitter->qy));
-    assert(ptrs_size(as_hj(placed)->emitter->qy) >= 2);
+    h = fnv(h, (uint32_t)ptrs_size(as_hj(placed)->qh.jo.emitter->qy));
+    assert(ptrs_size(as_hj(placed)->qh.jo.emitter->qy) >= 2);
 
     h = fold_draw(h, placed);
     h = fold_draw(h, placed);
@@ -557,19 +557,19 @@ static uint32_t fold_heir(uint32_t h, void *e, const pico8_struct_class_t *cls)
     assert(is_of(e, cls == pico8_pm_struct_class() ? &pico8_pm_px_row : &pico8_mx_px_row));
     assert(cls_of(e)->kj == pico8_hj_struct_class()->kj);
     assert(cls_of(e)->mm == pico8_hj_struct_class()->mm);
-    assert(ptrs_size(hj->emitter->qy) == 0);
+    assert(ptrs_size(hj->qh.jo.emitter->qy) == 0);
     assert(hj->qh.jo.def->qh == 0);
     h = fold_string_member(h, hj->qh.jo.id);
     h = fold_string_member(h, hj->qh.jo.header.me);
     h = fold_qh(h, &hj->qh);
     h = fold_string_member(h, hj->qh.jo.def->rr);
-    h = fold_fixed_member(h, hj->qh.probe->od);
+    h = fold_fixed_member(h, hj->qh.jo.probe->od);
     h = fold_pk(h, hj);
     h = fold_fixed_member(h, hj->qh.jo.def->hp);
-    h = fold_fixed_member(h, hj->qh.s);
-    h = fold_fixed_member(h, hj->emitter->py);
-    h = fold_fixed_member(h, hj->emitter->pn);
-    h = fold_fixed_member(h, hj->emitter->du);
+    h = fold_fixed_member(h, hj->qh.jo.s);
+    h = fold_fixed_member(h, hj->qh.jo.emitter->py);
+    h = fold_fixed_member(h, hj->qh.jo.emitter->pn);
+    h = fold_fixed_member(h, hj->qh.jo.emitter->du);
     h = fnv(h, (uint32_t)ents_rank_in(pico8_cg("hj"), e));
     return fold_particle(h, cls_of(e)->mm(e));
 }
@@ -587,8 +587,8 @@ static void test_heirs(void) {
     keys.id = "s77";
     pm = pico8_pm_new(&keys);
 
-    assert(strcmp(as_hj(pm)->qh.jo.def->rr, "ascension") == 0 && as_hj(pm)->emitter->def->pk == 0);
-    assert(as_hj(pm)->qh.probe->od == pico8_fixed_from_int(60));
+    assert(strcmp(as_hj(pm)->qh.jo.def->rr, "ascension") == 0 && as_hj(pm)->qh.jo.emitter->def->pk == 0);
+    assert(as_hj(pm)->qh.jo.probe->od == pico8_fixed_from_int(60));
     assert(strcmp(as_hj(mx)->qh.jo.def->rr, "void synthesis") == 0);
     h = fold_heir(h, pm, pico8_pm_struct_class());
     h = fold_heir(h, mx, pico8_mx_struct_class());
@@ -650,7 +650,7 @@ static void test_hj_is_a_struct(void) {
     {
         pico8_hj_t *bare = pico8_hj_new(NULL);
         assert(is_an_hj_struct(bare) && bare->qh.jo.id == NULL && bare->qh.jo.oe != NULL &&
-               ptrs_size(bare->emitter->qy) == 0 && ents_listed(pico8_cg("hj"), bare));
+               ptrs_size(bare->qh.jo.emitter->qy) == 0 && ents_listed(pico8_cg("hj"), bare));
     }
 }
 
diff --git a/tests/test_pico8_jo_spec_defaults.c b/tests/test_pico8_jo_spec_defaults.c
index 1cf20e2..2ac39a8 100644
--- a/tests/test_pico8_jo_spec_defaults.c
+++ b/tests/test_pico8_jo_spec_defaults.c
@@ -53,9 +53,9 @@ static void assert_jo_box(const pico8_qz_box_t *kq) {
 /* The members of qh every class below it inherits, its probe's archetype, and
  * the probe's init literals. */
 static void assert_qh_tail(const pico8_qh_t *qh, const pico8_jo_def_t *def) {
-    assert(qh->probe->def == &def->probe);
+    assert(qh->jo.probe->def == &def->probe);
     assert(qh->jo.mi == def->mi && qh->jo.has_mi == 1);
-    assert(qh->probe->has_od == 1 && qh->probe->def->ik == 0 && qh->probe->def->has_ik == 0);
+    assert(qh->jo.probe->has_od == 1 && qh->jo.probe->def->ik == 0 && qh->jo.probe->def->has_ik == 0);
 }
 
 /* The emitter of an ascension, pm or mx, and its archetype. */
@@ -85,8 +85,8 @@ static void test_qh_defaults(void) {
 
     assert(pico8_qh_ready());
     assert_same_string(qh->jo.header.me, pico8_qh_def.me);
-    assert(qh->probe->od == pico8_qh_def.probe.od);
-    assert(qh->s == pico8_qh_def.s);
+    assert(qh->jo.probe->od == pico8_qh_def.probe.od);
+    assert(qh->jo.s == pico8_qh_def.s);
     assert_qh_tail(qh, &pico8_qh_def);
     assert_jo_box(&qh->jo.def->kq);
     assert_jo_tail(&qh->jo, &pico8_qh_def);
@@ -96,9 +96,9 @@ static void test_nl_defaults(void) {
     pico8_nl_t *nl = pico8_nl_new(NULL);
 
     assert(pico8_nl_ready());
-    assert(nl->qh.s == pico8_nl_def.s);
-    assert(nl->qh.probe->od == pico8_nl_def.probe.od);
-    assert(nl->qw == pico8_nl_def.qw);
+    assert(nl->qh.jo.s == pico8_nl_def.s);
+    assert(nl->qh.jo.probe->od == pico8_nl_def.probe.od);
+    assert(nl->qh.jo.qw == pico8_nl_def.qw);
     assert_same_string(nl->qh.jo.header.me, pico8_nl_def.me);
     assert_qh_tail(&nl->qh, &pico8_nl_def);
     assert_jo_box(&nl->qh.jo.def->kq);
@@ -109,8 +109,8 @@ static void test_os_defaults(void) {
     pico8_os_t *os = pico8_os_new(NULL);
 
     assert(pico8_os_ready());
-    assert(os->qh.s == pico8_os_def.s);
-    assert(os->qh.probe->od == pico8_os_def.probe.od);
+    assert(os->qh.jo.s == pico8_os_def.s);
+    assert(os->qh.jo.probe->od == pico8_os_def.probe.od);
     assert_same_string(os->qh.jo.header.me, pico8_os_def.me);
     assert_qh_tail(&os->qh, &pico8_os_def);
     assert_jo_box(&os->qh.jo.def->kq);
@@ -121,9 +121,9 @@ static void test_hj_defaults(void) {
     pico8_hj_t *hj = pico8_hj_new(NULL);
 
     assert(pico8_hj_ready());
-    assert(hj->qh.probe->od == pico8_hj_def.probe.od);
-    assert(hj->qh.s == pico8_hj_def.s);
-    assert_emitter(hj->emitter, &pico8_hj_def);
+    assert(hj->qh.jo.probe->od == pico8_hj_def.probe.od);
+    assert(hj->qh.jo.s == pico8_hj_def.s);
+    assert_emitter(hj->qh.jo.emitter, &pico8_hj_def);
     assert_same_string(hj->qh.jo.header.me, pico8_hj_def.me);
     assert_qh_tail(&hj->qh, &pico8_hj_def);
     assert_jo_box(&hj->qh.jo.def->kq);
@@ -135,9 +135,9 @@ static void test_pm_defaults(void) {
 
     assert(pico8_pm_ready());
     assert_same_string(pm->hj.qh.jo.header.me, pico8_pm_def.me);
-    assert(pm->hj.qh.s == pico8_pm_def.s);
-    assert_emitter(pm->hj.emitter, &pico8_pm_def);
-    assert(pm->hj.qh.probe->od == pico8_pm_def.probe.od);
+    assert(pm->hj.qh.jo.s == pico8_pm_def.s);
+    assert_emitter(pm->hj.qh.jo.emitter, &pico8_pm_def);
+    assert(pm->hj.qh.jo.probe->od == pico8_pm_def.probe.od);
     assert_qh_tail(&pm->hj.qh, &pico8_pm_def);
     assert_jo_box(&pm->hj.qh.jo.def->kq);
     assert_jo_tail(&pm->hj.qh.jo, &pico8_pm_def);
@@ -149,15 +149,15 @@ static void test_ga_defaults(void) {
 
     assert(pico8_ga_ready());
     assert(ga->jo.by != NULL && ga->jo.by == other->jo.by);
-    assert(ga->emitter->def == &pico8_ga_def.emitter);
-    assert(ga->emitter->qo == pico8_ga_def.emitter.qo);
-    assert(ga->emitter->ro == pico8_ga_def.emitter.ro);
-    assert(ga->emitter->qg == pico8_ga_def.emitter.qg);
-    assert(ga->emitter->q_ == pico8_ga_def.emitter.q_);
-    assert(ga->emitter->k_ == pico8_ga_def.emitter.k_);
-    assert(ga->iu == pico8_ga_def.iu);
-    assert(ga->emitter->py == pico8_ga_def.emitter.py);
-    assert(ga->ms == pico8_ga_def.ms);
+    assert(ga->jo.emitter->def == &pico8_ga_def.emitter);
+    assert(ga->jo.emitter->qo == pico8_ga_def.emitter.qo);
+    assert(ga->jo.emitter->ro == pico8_ga_def.emitter.ro);
+    assert(ga->jo.emitter->qg == pico8_ga_def.emitter.qg);
+    assert(ga->jo.emitter->q_ == pico8_ga_def.emitter.q_);
+    assert(ga->jo.emitter->k_ == pico8_ga_def.emitter.k_);
+    assert(ga->jo.iu == pico8_ga_def.iu);
+    assert(ga->jo.emitter->py == pico8_ga_def.emitter.py);
+    assert(ga->jo.ms == pico8_ga_def.ms);
     assert_box(&ga->jo.def->kq, 0, 0, -1, -1);
     assert_same_string(ga->jo.header.me, ga->jo.def->me);
     assert_jo_tail(&ga->jo, &pico8_ga_def);
@@ -170,8 +170,8 @@ static void test_ga_defaults(void) {
 
 /* What nr and fr inherit from qh and jo, and nr's box, which fr keeps. */
 static void assert_nr_inherited(const pico8_nr_t *nr, const pico8_jo_def_t *def) {
-    assert(nr->qh.probe->od == def->probe.od);
-    assert(nr->qh.s == def->s);
+    assert(nr->qh.jo.probe->od == def->probe.od);
+    assert(nr->qh.jo.s == def->s);
     assert_box(&nr->qh.jo.def->kq, -4, -4, 5, 5);
     assert_same_string(nr->qh.jo.header.me, def->me);
     assert_qh_tail(&nr->qh, def);
@@ -196,10 +196,10 @@ static void test_mx_defaults(void) {
     pico8_mx_t *mx = pico8_mx_new(NULL);
 
     assert(pico8_mx_ready());
-    assert(mx->hj.qh.s == pico8_mx_def.s);
-    assert(mx->hj.qh.probe->od == pico8_mx_def.probe.od);
-    assert(mx->ht == pico8_mx_def.ht);
-    assert_emitter(mx->hj.emitter, &pico8_mx_def);
+    assert(mx->hj.qh.jo.s == pico8_mx_def.s);
+    assert(mx->hj.qh.jo.probe->od == pico8_mx_def.probe.od);
+    assert(mx->hj.qh.jo.ht == pico8_mx_def.ht);
+    assert_emitter(mx->hj.qh.jo.emitter, &pico8_mx_def);
     assert_same_string(mx->hj.qh.jo.header.me, pico8_mx_def.me);
     assert_qh_tail(&mx->hj.qh, &pico8_mx_def);
     assert_jo_box(&mx->hj.qh.jo.def->kq);
diff --git a/tests/test_pico8_mx.c b/tests/test_pico8_mx.c
index a51eda2..cfc7991 100644
--- a/tests/test_pico8_mx.c
+++ b/tests/test_pico8_mx.c
@@ -161,7 +161,7 @@ static pico8_hj_t *as_hj(void *e) {
 }
 
 static uint32_t fold_pk(uint32_t h, const pico8_hj_t *hj) {
-    return fold_fixed_member(h, hj->emitter->def->pk);
+    return fold_fixed_member(h, hj->qh.jo.emitter->def->pk);
 }
 
 /* A particle record as `g_.mm` returns it: `p`, `v`, `l`. */
@@ -266,13 +266,13 @@ static void *build_synthesizer(void) {
 /* `mx` sets its own `px`: an entity whose `px` is the very table of mx's
  * class is a synthesizer. */
 static int is_mx(const void *e) {
-    const pico8_qh_t *qh = pico8_qh_of(e);
-    return qh != NULL && qh->jo.def->px == &pico8_mx_px_row;
+    const pico8_jo_t *jo = e != NULL && (cls_of(e)->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)) ? e : NULL;
+    return jo != NULL && jo->def->px == &pico8_mx_px_row;
 }
 
 /* `ht` of an `mx`; NULL on any other struct, which has no `ht`. */
 static const pico8_int_seq_t *ht_of(const void *e) {
-    return cls_of(e) == pico8_mx_struct_class() ? ((const pico8_mx_t *)e)->ht : NULL;
+    return cls_of(e) == pico8_mx_struct_class() ? ((const pico8_mx_t *)e)->hj.qh.jo.ht : NULL;
 }
 
 /* The entry count of the `ht` table, -1 when absent. */
@@ -293,7 +293,7 @@ static int ht_is_the_class_one(const void *e) {
 static uint32_t fold_synthesizer(uint32_t h, void *e) {
     const pico8_struct_class_t *hj_cls = pico8_hj_struct_class();
     const pico8_hj_t *hj = as_hj(e);
-    const pico8_emitter_t *em = hj->emitter;
+    const pico8_emitter_t *em = hj->qh.jo.emitter;
     const pico8_qh_t *qh = &hj->qh;
     const pico8_jo_t *jo = &qh->jo;
     size_t i;
@@ -304,8 +304,8 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = fold_string_member(h, hj->qh.jo.def->rr);
     h = fold_qh(h, qh);
     h = fold_fixed_member(h, qh->jo.def->hp);
-    h = fold_fixed_member(h, qh->s);
-    h = fold_fixed_member(h, qh->probe->od);
+    h = fold_fixed_member(h, qh->jo.s);
+    h = fold_fixed_member(h, qh->jo.probe->od);
     h = fold_fixed_member(h, em->py);
     h = fold_fixed_member(h, em->def->ki);
     h = fold_fixed_member(h, em->pn);
@@ -316,9 +316,9 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = fold_fixed_member(h, em->ro);
     h = fold_fixed_member(h, em->qg);
     h = fold_fixed_member(h, em->q_);
-    h = fold_string_member(h, qh->probe->def->dq);
-    h = fold_fixed_member(h, qh->probe->def->fp);
-    h = fold_fixed_member(h, qh->probe->def->dw);
+    h = fold_string_member(h, qh->jo.probe->def->dq);
+    h = fold_fixed_member(h, qh->jo.probe->def->fp);
+    h = fold_fixed_member(h, qh->jo.probe->def->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
@@ -337,7 +337,7 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(hj->emitter->k_ != NULL));
+    h = fnv(h, (uint32_t)(hj->qh.jo.emitter->k_ != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -346,14 +346,14 @@ static uint32_t fold_synthesizer(uint32_t h, void *e) {
     h = fnv(h, (uint32_t)(em->qy != NULL));
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
-    h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
+    h = fnv(h, qh->jo.probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -428,7 +428,7 @@ static void test_built(void) {
     a = build_synthesizer();
     b = build_synthesizer();
     assert(a != b && is_mx(a) && is_mx(b));
-    assert(as_hj(a)->emitter->qy != NULL && as_hj(a)->emitter->qy != as_hj(b)->emitter->qy);
+    assert(as_hj(a)->qh.jo.emitter->qy != NULL && as_hj(a)->qh.jo.emitter->qy != as_hj(b)->qh.jo.emitter->qy);
     h = fold_synthesizer(h, a);
     h = fold_synthesizer(h, b);
     check("built", h, BUILT_HASH);
@@ -476,7 +476,7 @@ static uint32_t fold_economy(uint32_t h, void *e) {
 static uint32_t fold_draw(uint32_t h, void *e) {
     pico8_dc("kj");
     h = fnv(h, screen_hash());
-    return fnv(h, (uint32_t)ptrs_size(as_hj(e)->emitter->qy));
+    return fnv(h, (uint32_t)ptrs_size(as_hj(e)->qh.jo.emitter->qy));
 }
 
 /* Placed on screen by `hq` (`bv`) and `ey` (`d_`); `dg` on one still in
@@ -516,8 +516,8 @@ static void test_life(void) {
 
     h = fold_particle(h, cls_of(placed)->mm(placed));
     pico8_ez(placed, PICO8_FIXED_LIT(0x00028000, 2.5));
-    h = fnv(h, (uint32_t)ptrs_size(as_hj(placed)->emitter->qy));
-    assert(ptrs_size(as_hj(placed)->emitter->qy) >= 2);
+    h = fnv(h, (uint32_t)ptrs_size(as_hj(placed)->qh.jo.emitter->qy));
+    assert(ptrs_size(as_hj(placed)->qh.jo.emitter->qy) >= 2);
 
     h = fold_draw(h, placed);
     h = fold_draw(h, placed);
@@ -670,7 +670,8 @@ static void test_mx_is_a_struct(void) {
     {
         pico8_mx_t *bare = pico8_mx_new(NULL);
         assert(is_an_mx_struct(bare) && bare->hj.qh.jo.id == NULL && bare->hj.qh.jo.oe != NULL &&
-               bare->hj.qh.jo.kw != NULL && ptrs_size(bare->hj.emitter->qy) == 0 && ents_listed(pico8_cg("jo"), bare));
+               bare->hj.qh.jo.kw != NULL && ptrs_size(bare->hj.qh.jo.emitter->qy) == 0 &&
+               ents_listed(pico8_cg("jo"), bare));
     }
 }
 
diff --git a/tests/test_pico8_nl.c b/tests/test_pico8_nl.c
index cc93a21..b4ed4cd 100644
--- a/tests/test_pico8_nl.c
+++ b/tests/test_pico8_nl.c
@@ -146,7 +146,9 @@ static uint32_t fold_kw(uint32_t h, const pico8_jo_t *jo) {
 
 /* `e` as a planet of qh's lineage. */
 static pico8_qh_t *as_qh(void *e) {
-    assert(pico8_qh_of(e) == e);
+    const pico8_struct_header_t *h = e;
+
+    assert(h != NULL && (h->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)));
     return e;
 }
 
@@ -156,7 +158,7 @@ static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
 
 /* 1 no target, 3 a target: the bytes the frozen hashes fold. */
 static uint32_t et_state(const pico8_qh_t *qh) {
-    return qh->probe->et == NULL ? 1u : 3u;
+    return qh->jo.probe->et == NULL ? 1u : 3u;
 }
 
 /* `et` as qh's reader gave it: false or the target. */
@@ -165,7 +167,7 @@ static uint32_t fold_et(uint32_t h, const pico8_qh_t *qh) {
     case 1u:
         return fold_bool_member(h, 0);
     default:
-        return fold_ref(fnv(h, 1u), qh->probe->et);
+        return fold_ref(fnv(h, 1u), qh->jo.probe->et);
     }
 }
 
@@ -198,6 +200,12 @@ static const pico8_struct_class_t *cls_of(const void *e) {
     return ((const pico8_struct_header_t *)e)->cls;
 }
 
+/* `e` as a planet of nl's lineage. */
+static const pico8_nl_t *as_nl(const void *e) {
+    assert(e != NULL && (cls_of(e)->lineage & PICO8_LINEAGE(PICO8_LINEAGE_NL)));
+    return e;
+}
+
 /* The members a planet of qh's tree holds: `kj`, `ev`, `hd`, `hk`, `hb`,
  * `ldown`, `gv`, `hq`, `ey`, `kf`, `cn` and the states `ll`, `ly`; `mm`
  * aside, no other. */
@@ -246,8 +254,8 @@ static void *build_plate(void) {
 
 /* A plate is the only entity whose chain holds `nl`, and its `nl` is 1. */
 static int is_plate(const void *e) {
-    const pico8_nl_t *nl = pico8_nl_of(e);
-    return nl != NULL && nl->qh.jo.def->nl == pico8_fixed_from_int(1);
+    const pico8_jo_t *jo = e != NULL && (cls_of(e)->lineage & PICO8_LINEAGE(PICO8_LINEAGE_NL)) ? e : NULL;
+    return jo != NULL && jo->def->nl == pico8_fixed_from_int(1);
 }
 
 /* A plate as `ie:ldown` and `jo:pq` left it: its scalars, which of nl's,
@@ -255,12 +263,11 @@ static int is_plate(const void *e) {
  * functions, but its own `ly`), which default tables it shares with `nl`
  * and `jo`, and where `lt` registered it -- never in `cg.qh`. */
 static uint32_t fold_plate(uint32_t h, const void *e) {
-    const pico8_nl_t *nl = pico8_nl_of(e);
+    const pico8_nl_t *nl = as_nl(e);
     const pico8_qh_t *qh;
     const pico8_jo_t *jo;
     size_t i;
 
-    assert(nl != NULL);
     qh = &nl->qh;
     jo = &qh->jo;
     h = fold_string_member(h, jo->id);
@@ -270,12 +277,12 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     h = fold_fixed_member(h, nl->qh.jo.def->nl);
     h = fold_qh(h, qh);
     h = fold_fixed_member(h, qh->jo.def->hp);
-    h = fold_fixed_member(h, qh->s);
-    h = fold_fixed_member(h, qh->probe->od);
-    h = fold_fixed_member(h, nl->qw);
-    h = fold_string_member(h, qh->probe->def->dq);
-    h = fold_fixed_member(h, qh->probe->def->fp);
-    h = fold_fixed_member(h, qh->probe->def->dw);
+    h = fold_fixed_member(h, qh->jo.s);
+    h = fold_fixed_member(h, qh->jo.probe->od);
+    h = fold_fixed_member(h, nl->qh.jo.qw);
+    h = fold_string_member(h, qh->jo.probe->def->dq);
+    h = fold_fixed_member(h, qh->jo.probe->def->fp);
+    h = fold_fixed_member(h, qh->jo.probe->def->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
@@ -291,14 +298,14 @@ static uint32_t fold_plate(uint32_t h, const void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
-    h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
+    h = fnv(h, qh->jo.probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -391,14 +398,13 @@ static uint32_t fold_ra_y(uint32_t h, const pico8_nl_t *nl) {
 
 /* `dg` once, then what `nl:ly` left on the plate. */
 static uint32_t fold_dg(uint32_t h, const void *e) {
-    const pico8_nl_t *nl = pico8_nl_of(e);
+    const pico8_nl_t *nl = as_nl(e);
 
-    assert(nl != NULL);
     h = fold_ra_y(h, nl);
     pico8_dg();
     h = fold_fixed_member(h, nl->qh.jo.header.t);
     h = fold_ra_y(h, nl);
-    return fold_fixed_member(h, nl->qw);
+    return fold_fixed_member(h, nl->qh.jo.qw);
 }
 
 /* `jo:ib`'s container, folded as fold_ref folded the tagged list it was:
@@ -453,7 +459,7 @@ static void test_life(void) {
     for (i = 0; i < 3; i++) {
         h = fold_dg(h, placed);
     }
-    assert(placed->qw == pico8_fixed_from_int(15));
+    assert(placed->qh.jo.qw == pico8_fixed_from_int(15));
     placed->qh.jo.mi = pico8_fixed_from_int(2);
     placed->qh.jo.has_mi = 1;
     h = fold_dg(h, placed);
@@ -461,13 +467,13 @@ static void test_life(void) {
     placed->qh.jo.mi = pico8_fixed_from_int(3);
     placed->qh.jo.has_mi = 1;
     h = fold_dg(h, placed);
-    assert(placed->qw != pico8_fixed_from_int(15));
+    assert(placed->qh.jo.qw != pico8_fixed_from_int(15));
     h = fold_economy(h, placed);
 
     idle = build_plate();
     h = fold_string_member(h, idle->qh.jo.header.me);
     h = fold_dg(h, idle);
-    assert(idle->qw == pico8_fixed_from_int(15));
+    assert(idle->qh.jo.qw == pico8_fixed_from_int(15));
     check("life", h, LIFE_HASH);
 }
 
diff --git a/tests/test_pico8_nr.c b/tests/test_pico8_nr.c
index 843f454..858cda7 100644
--- a/tests/test_pico8_nr.c
+++ b/tests/test_pico8_nr.c
@@ -143,7 +143,9 @@ static int ents_list_size(const pico8_ents_t *l) {
 
 /* `e` as a planet of qh's lineage. */
 static pico8_qh_t *as_qh(void *e) {
-    assert(pico8_qh_of(e) == e);
+    const pico8_struct_header_t *h = e;
+
+    assert(h != NULL && (h->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)));
     return e;
 }
 
@@ -274,11 +276,11 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     h = fold_fixed_member(h, jo->header.t);
     h = fold_string_member(h, nr->qh.jo.def->rr);
     h = fold_fixed_member(h, qh->jo.def->hp);
-    h = fold_fixed_member(h, qh->s);
-    h = fold_fixed_member(h, qh->probe->od);
-    h = fold_fixed_member(h, qh->probe->def->fp);
-    h = fold_fixed_member(h, qh->probe->def->dw);
-    h = fold_string_member(h, qh->probe->def->dq);
+    h = fold_fixed_member(h, qh->jo.s);
+    h = fold_fixed_member(h, qh->jo.probe->od);
+    h = fold_fixed_member(h, qh->jo.probe->def->fp);
+    h = fold_fixed_member(h, qh->jo.probe->def->dw);
+    h = fold_string_member(h, qh->jo.probe->def->dq);
     h = fold_qh(h, qh);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
@@ -295,14 +297,14 @@ static uint32_t fold_slipgate(uint32_t h, const void *e) {
     h = fnv(h, (uint32_t)(nr->qh.jo.def->rr != NULL));
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
-    h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
+    h = fnv(h, qh->jo.probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -492,24 +494,24 @@ static void test_life(void) {
     for (i = 0; i <= 3; i++) {
         as_qh(placed)->jo.oe = list_of(i, idle);
         cls_of(placed)->hb(placed);
-        h = fold_fixed_member(h, as_qh(placed)->s);
+        h = fold_fixed_member(h, as_qh(placed)->jo.s);
         h = fold_bool(h, cls_of(placed)->gv(placed, NULL).stop);
     }
-    assert(as_qh(placed)->s == pico8_fixed_from_int(168));
+    assert(as_qh(placed)->jo.s == pico8_fixed_from_int(168));
     jc = cls_of(placed)->ldown(placed, NULL, NULL);
     assert(jc.held == NULL);
     h = fold_bool(h, jc.stop);
     as_qh(placed)->jo.oe = oe;
     cls_of(placed)->hb(placed);
-    h = fold_fixed_member(h, as_qh(placed)->s);
+    h = fold_fixed_member(h, as_qh(placed)->jo.s);
     h = fold_economy(h, placed);
 
     fr = build(pico8_fr_kind());
     assert(gate_kind(fr) == FR);
-    h = fold_fixed_member(h, as_qh(fr)->s);
+    h = fold_fixed_member(h, as_qh(fr)->jo.s);
     cls_of(fr)->hb(fr);
-    h = fold_fixed_member(h, as_qh(fr)->s);
-    assert(as_qh(fr)->s == pico8_fixed_from_int(78));
+    h = fold_fixed_member(h, as_qh(fr)->jo.s);
+    assert(as_qh(fr)->jo.s == pico8_fixed_from_int(78));
 
     bare = pico8_nr_new(NULL);
     assert(gate_kind(bare) == NR && as_qh(bare)->jo.id == NULL && as_qh(bare)->jo.oe != NULL &&
@@ -564,7 +566,7 @@ static void test_nr_is_a_struct(void) {
     assert(fr != NULL &&
            ((const pico8_struct_header_t *)fr)->cls == pico8_fr_struct_class());
     cls_of(fr)->hb(fr);
-    assert(as_qh(fr)->s == pico8_fixed_from_int(78));
+    assert(as_qh(fr)->jo.s == pico8_fixed_from_int(78));
 }
 
 /* Counts the offers `cn` makes, taking each as `jo:ib`'s closure does. */
diff --git a/tests/test_pico8_os.c b/tests/test_pico8_os.c
index 4475994..569296b 100644
--- a/tests/test_pico8_os.c
+++ b/tests/test_pico8_os.c
@@ -160,7 +160,9 @@ static uint32_t fold_kw(uint32_t h, const pico8_jo_t *jo) {
 
 /* `e` as a planet of qh's lineage. */
 static pico8_qh_t *as_qh(void *e) {
-    assert(pico8_qh_of(e) == e);
+    const pico8_struct_header_t *h = e;
+
+    assert(h != NULL && (h->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)));
     return e;
 }
 
@@ -170,7 +172,7 @@ static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
 
 /* 1 no target, 3 a target: the bytes the frozen hashes fold. */
 static uint32_t et_state(const pico8_qh_t *qh) {
-    return qh->probe->et == NULL ? 1u : 3u;
+    return qh->jo.probe->et == NULL ? 1u : 3u;
 }
 
 /* `et` as qh's reader gave it: false or the target. */
@@ -179,7 +181,7 @@ static uint32_t fold_et(uint32_t h, const pico8_qh_t *qh) {
     case 1u:
         return fold_bool_member(h, 0);
     default:
-        return fold_ref(fnv(h, 1u), qh->probe->et);
+        return fold_ref(fnv(h, 1u), qh->jo.probe->et);
     }
 }
 
@@ -282,8 +284,8 @@ static void *build_processor(void) {
 
 /* A processor is the only entity whose `px` is os's own table. */
 static int is_processor(const void *e) {
-    const pico8_qh_t *qh = pico8_qh_of(e);
-    return qh != NULL && qh->jo.def->px == &pico8_os_px_row;
+    const pico8_jo_t *jo = e != NULL && (cls_of(e)->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)) ? e : NULL;
+    return jo != NULL && jo->def->px == &pico8_os_px_row;
 }
 
 /* A processor as `ie:ldown` and `jo:pq` left it: its scalars (`hs` is jo's
@@ -301,12 +303,12 @@ static uint32_t fold_processor(uint32_t h, void *e) {
     h = fold_fixed_member(h, jo->header.t);
     h = fold_qh(h, qh);
     h = fold_fixed_member(h, qh->jo.def->hp);
-    h = fold_fixed_member(h, qh->s);
-    h = fold_fixed_member(h, qh->probe->od);
+    h = fold_fixed_member(h, qh->jo.s);
+    h = fold_fixed_member(h, qh->jo.probe->od);
     h = fold_fixed_member(h, jo->def->hs);
-    h = fold_string_member(h, qh->probe->def->dq);
-    h = fold_fixed_member(h, qh->probe->def->fp);
-    h = fold_fixed_member(h, qh->probe->def->dw);
+    h = fold_string_member(h, qh->jo.probe->def->dq);
+    h = fold_fixed_member(h, qh->jo.probe->def->fp);
+    h = fold_fixed_member(h, qh->jo.probe->def->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->fa);
@@ -318,14 +320,14 @@ static uint32_t fold_processor(uint32_t h, void *e) {
      * still proves the typed reads. */
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
-    h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
+    h = fnv(h, qh->jo.probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
diff --git a/tests/test_pico8_pm.c b/tests/test_pico8_pm.c
index e3942bf..c52ef42 100644
--- a/tests/test_pico8_pm.c
+++ b/tests/test_pico8_pm.c
@@ -143,7 +143,7 @@ static pico8_hj_t *as_hj(void *e) {
 }
 
 static uint32_t fold_pk(uint32_t h, const pico8_hj_t *hj) {
-    return fold_bool_member(h, hj->emitter->def->pk != 0);
+    return fold_bool_member(h, hj->qh.jo.emitter->def->pk != 0);
 }
 
 /* A particle record as `g_.mm` returns it: `p`, `v`, `l`. */
@@ -239,8 +239,8 @@ static void *build_protostar(const pico8_vec_t *p, const pico8_vec_t *ra, const
 /* `pm` sets its own `px`: an entity whose `px` is the very table of pm's
  * class is a protostar. */
 static int is_pm(const void *e) {
-    const pico8_qh_t *qh = pico8_qh_of(e);
-    return qh != NULL && qh->jo.def->px == &pico8_pm_px_row;
+    const pico8_jo_t *jo = e != NULL && (cls_of(e)->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)) ? e : NULL;
+    return jo != NULL && jo->def->px == &pico8_pm_px_row;
 }
 
 static int is_ga(const void *e) {
@@ -274,7 +274,7 @@ static uint32_t fold_spawned(uint32_t h, void *e) {
     }
     assert(gra != ra && gra != ng);
     h = fold_vec(h, g->jo.ra);
-    h = fnv(h, (uint32_t)ptrs_size(g->emitter->qy));
+    h = fnv(h, (uint32_t)ptrs_size(g->jo.emitter->qy));
     assert(pico8_mo_qb() != NULL);
     return fold_fixed_member(h, pico8_mo_qb()->ma);
 }
@@ -287,7 +287,7 @@ static uint32_t fold_spawned(uint32_t h, void *e) {
 static uint32_t fold_protostar(uint32_t h, void *e) {
     const pico8_hj_t *hj = as_hj(e);
     const pico8_pm_t *pm;
-    const pico8_emitter_t *em = hj->emitter;
+    const pico8_emitter_t *em = hj->qh.jo.emitter;
     const pico8_qh_t *qh = &hj->qh;
     const pico8_jo_t *jo = &qh->jo;
     size_t i;
@@ -301,8 +301,8 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fold_string_member(h, hj->qh.jo.def->rr);
     h = fold_qh(h, qh);
     h = fold_fixed_member(h, qh->jo.def->hp);
-    h = fold_fixed_member(h, qh->s);
-    h = fold_fixed_member(h, qh->probe->od);
+    h = fold_fixed_member(h, qh->jo.s);
+    h = fold_fixed_member(h, qh->jo.probe->od);
     h = fold_fixed_member(h, em->py);
     h = fold_fixed_member(h, em->def->ki);
     h = fold_fixed_member(h, em->pn);
@@ -313,9 +313,9 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fold_fixed_member(h, em->ro);
     h = fold_fixed_member(h, em->qg);
     h = fold_fixed_member(h, em->q_);
-    h = fold_string_member(h, qh->probe->def->dq);
-    h = fold_fixed_member(h, qh->probe->def->fp);
-    h = fold_fixed_member(h, qh->probe->def->dw);
+    h = fold_string_member(h, qh->jo.probe->def->dq);
+    h = fold_fixed_member(h, qh->jo.probe->def->fp);
+    h = fold_fixed_member(h, qh->jo.probe->def->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
@@ -337,7 +337,7 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
-    h = fnv(h, (uint32_t)(hj->emitter->k_ != NULL));
+    h = fnv(h, (uint32_t)(hj->qh.jo.emitter->k_ != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -346,14 +346,14 @@ static uint32_t fold_protostar(uint32_t h, void *e) {
     h = fnv(h, (uint32_t)(em->qy != NULL));
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
-    h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
+    h = fnv(h, qh->jo.probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -433,7 +433,7 @@ static void test_built(void) {
     a = build_protostar(&pa, &ra_a, "s77");
     b = build_protostar(&pb, &ra_b, "s78");
     assert(a != b && is_pm(a) && is_pm(b));
-    assert(as_hj(a)->emitter->qy != NULL && as_hj(a)->emitter->qy != as_hj(b)->emitter->qy);
+    assert(as_hj(a)->qh.jo.emitter->qy != NULL && as_hj(a)->qh.jo.emitter->qy != as_hj(b)->qh.jo.emitter->qy);
     h = fold_protostar(h, a);
     h = fold_protostar(h, b);
     check("built", h, BUILT_HASH);
@@ -477,7 +477,7 @@ static uint32_t fold_economy(uint32_t h, void *e) {
 static uint32_t fold_draw(uint32_t h, void *e) {
     pico8_dc("kj");
     h = fnv(h, screen_hash());
-    return fnv(h, (uint32_t)ptrs_size(as_hj(e)->emitter->qy));
+    return fnv(h, (uint32_t)ptrs_size(as_hj(e)->qh.jo.emitter->qy));
 }
 
 /* `dg` in "ly" (qh's `ly`, `na`); `mm` (`g_.mm`, `pk` false: no rotation)
@@ -509,8 +509,8 @@ static void test_life(void) {
 
     h = fold_particle(h, cls_of(placed)->mm(placed));
     pico8_ez(placed, PICO8_FIXED_LIT(0x00028000, 2.5));
-    h = fnv(h, (uint32_t)ptrs_size(as_hj(placed)->emitter->qy));
-    assert(ptrs_size(as_hj(placed)->emitter->qy) >= 2);
+    h = fnv(h, (uint32_t)ptrs_size(as_hj(placed)->qh.jo.emitter->qy));
+    assert(ptrs_size(as_hj(placed)->qh.jo.emitter->qy) >= 2);
 
     h = fold_draw(h, placed);
     h = fold_draw(h, placed);
@@ -553,7 +553,8 @@ static void test_pm_is_a_struct(void) {
     {
         pico8_pm_t *bare = pico8_pm_new(NULL);
         assert(is_a_pm_struct(bare) && bare->hj.qh.jo.id == NULL && bare->hj.qh.jo.ra == NULL &&
-               bare->hj.qh.jo.oe != NULL && ptrs_size(bare->hj.emitter->qy) == 0 && ents_listed(pico8_cg("jo"), bare));
+               bare->hj.qh.jo.oe != NULL && ptrs_size(bare->hj.qh.jo.emitter->qy) == 0 &&
+               ents_listed(pico8_cg("jo"), bare));
     }
 }
 
diff --git a/tests/test_pico8_qh.c b/tests/test_pico8_qh.c
index 4e8915a..c3c9430 100644
--- a/tests/test_pico8_qh.c
+++ b/tests/test_pico8_qh.c
@@ -88,7 +88,9 @@ static int ents_listed(const pico8_ents_t *list, const void *e) {
 
 /* `e` as a planet of qh's lineage. */
 static pico8_qh_t *as_qh(void *e) {
-    assert(pico8_qh_of(e) == e);
+    const pico8_struct_header_t *h = e;
+
+    assert(h != NULL && (h->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)));
     return e;
 }
 
@@ -124,7 +126,7 @@ static uint32_t p__state(const pico8_jo_t *jo) {
 
 /* 1 no target, 3 a target: the bytes the frozen hashes fold. */
 static uint32_t et_state(const pico8_qh_t *qh) {
-    return qh->probe->et == NULL ? 1u : 3u;
+    return qh->jo.probe->et == NULL ? 1u : 3u;
 }
 
 static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
@@ -158,7 +160,7 @@ static uint32_t fold_et(uint32_t h, const pico8_qh_t *qh) {
     case 1u:
         return fold_bool_member(h, 0);
     default:
-        return fold_ref(fnv(h, 1u), qh->probe->et);
+        return fold_ref(fnv(h, 1u), qh->jo.probe->et);
     }
 }
 
@@ -270,13 +272,13 @@ static uint32_t fold_lab(uint32_t h, void *e) {
     h = fold_string_member(h, jo->header.me);
     h = fold_fixed_member(h, jo->header.t);
     h = fold_fixed_member(h, qh->jo.def->hp);
-    h = fold_string_member(h, qh->probe->def->dq);
+    h = fold_string_member(h, qh->jo.probe->def->dq);
     h = fold_qh(h, qh);
-    h = fold_fixed_member(h, qh->probe->od);
-    h = fold_fixed_member(h, qh->probe->def->fp);
-    h = fold_fixed_member(h, qh->probe->def->dw);
+    h = fold_fixed_member(h, qh->jo.probe->od);
+    h = fold_fixed_member(h, qh->jo.probe->def->fp);
+    h = fold_fixed_member(h, qh->jo.probe->def->dw);
     h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
-    h = fold_fixed_member(h, qh->s);
+    h = fold_fixed_member(h, qh->jo.s);
     h = fold_hf(h, jo);
     h = fold_fixed_member(h, jo->def->hs);
     h = fold_fixed_member(h, jo->def->fa);
@@ -288,14 +290,14 @@ static uint32_t fold_lab(uint32_t h, void *e) {
      * still proves the typed reads. */
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
-    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
+    h = fnv(h, (uint32_t)(qh->jo.probe->def->dq != NULL));
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
-    h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
+    h = fnv(h, qh->jo.probe->et != NULL ? 1u : 0u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
     h = fnv(h, 1u);
@@ -415,7 +417,7 @@ static void test_life(void) {
     mp = vec_ref(vec_xy(7680, 7680));
     cls_of(out_lab)->hq(out_lab, mp, vec_ref(vec_xy(0, 0)));
     h = fold_held(h, out_lab, mp);
-    assert(et_state(as_qh(out_lab)) == 3 && !ents_listed(pico8_hv(), as_qh(out_lab)->probe->et));
+    assert(et_state(as_qh(out_lab)) == 3 && !ents_listed(pico8_hv(), as_qh(out_lab)->jo.probe->et));
 
     planet = pico8_cg("jo")->items[0];
     assert(pico8_jo_is_family(planet));
@@ -424,7 +426,7 @@ static void test_life(void) {
     mp = vec_ref(pico8_vec_add(pico8_vec_mul(*planet_p, pico8_fixed_from_int(128)), vec_xy(6, 0)));
     cls_of(out_lab)->hq(out_lab, mp, vec_ref(vec_xy(64, 64)));
     h = fold_held(h, out_lab, mp);
-    assert(as_qh(out_lab)->probe->et == planet &&
+    assert(as_qh(out_lab)->jo.probe->et == planet &&
            ((const pico8_jo_t *)planet)->fo == out_lab);
 
     poor_lab = build_lab();
@@ -508,11 +510,11 @@ static int descendant_of(const void *e) {
 static uint32_t fold_descendant(uint32_t h, void *d) {
     pico8_qh_t *qh = as_qh(d);
     pico8_nl_t *nl = cls_of(d) == pico8_nl_struct_class() ? (pico8_nl_t *)d : NULL;
-    pico8_fixed_t qw = nl != NULL ? nl->qw : 0;
+    pico8_fixed_t qw = nl != NULL ? nl->qh.jo.qw : 0;
 
-    h = fold_string_member(h, qh->probe->def->dq);
-    h = fold_fixed_member(h, qh->probe->def->fp);
-    h = fold_fixed_member(h, qh->probe->def->dw);
+    h = fold_string_member(h, qh->jo.probe->def->dq);
+    h = fold_fixed_member(h, qh->jo.probe->def->fp);
+    h = fold_fixed_member(h, qh->jo.probe->def->dw);
     h = fold_hf(h, &qh->jo);
     h = qh->jo.has_mi ? fold_fixed_member(h, qh->jo.mi) : fold_absent(h);
     h = fnv(h, (uint32_t)(qh->jo.header.cls->tags == (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER))));
@@ -531,9 +533,9 @@ static uint32_t fold_descendant(uint32_t h, void *d) {
         cls_of(d)->states[PICO8_STATE_LY](d, pico8_fixed_from_int(50));
         h = fnv(h, 0u);
     }
-    h = nl != NULL ? fold_fixed_member(h, nl->qw) : fold_absent(h);
+    h = nl != NULL ? fold_fixed_member(h, nl->qh.jo.qw) : fold_absent(h);
     if (nl != NULL) {
-        nl->qw = qw;
+        nl->qh.jo.qw = qw;
     }
     return h;
 }
@@ -709,11 +711,10 @@ static unsigned lineage_of(const pico8_struct_header_t *e) {
     return e->cls->lineage;
 }
 
-/* qh's lineage answers pico8_qh_of() with itself, through one to three
- * `parent` links; jo, ga and NULL answer NULL. Only nl answers pico8_nl_of().
- * Each class's `lineage` says the same; g_ and fv are of g_'s alone, the
- * other classes of none. */
-static void test_qh_of_follows_parent(void) {
+/* The eight kinds of qh's lineage are of jo's and qh's, nl alone of nl's
+ * too; ga and jo of jo's alone, g_ and fv of g_'s alone, the other classes
+ * of none. */
+static void test_lineage_of_each_class(void) {
     const unsigned jo = PICO8_LINEAGE(PICO8_LINEAGE_JO);
     const unsigned qh = jo | PICO8_LINEAGE(PICO8_LINEAGE_QH);
     const pico8_entity_kind_t *const lineage[] = {pico8_qh_kind(), pico8_nl_kind(), pico8_os_kind(), pico8_hj_kind(),
@@ -725,15 +726,10 @@ static void test_qh_of_follows_parent(void) {
     start_game();
     for (i = 0; i < sizeof lineage / sizeof lineage[0]; i++) {
         e = lineage[i]->new_entity(&keys);
-        assert(pico8_qh_of(e) == (const void *)e);
-        assert(pico8_nl_of(e) == (lineage[i] == pico8_nl_kind() ? (const void *)e : NULL));
         assert(lineage_of(e) == (lineage[i] == pico8_nl_kind() ? (qh | PICO8_LINEAGE(PICO8_LINEAGE_NL)) : qh));
     }
-    e = pico8_ga_kind()->new_entity(&keys);
-    assert(pico8_qh_of(e) == NULL && pico8_nl_of(e) == NULL && lineage_of(e) == jo);
-    e = (const pico8_struct_header_t *)pico8_jo_new(&keys);
-    assert(pico8_qh_of(e) == NULL && pico8_nl_of(e) == NULL && lineage_of(e) == jo);
-    assert(pico8_qh_of(NULL) == NULL && pico8_nl_of(NULL) == NULL);
+    assert(lineage_of(pico8_ga_kind()->new_entity(&keys)) == jo);
+    assert(lineage_of(&pico8_jo_new(&keys)->header) == jo);
     assert(lineage_of(&pico8_g__new(NULL)->header) == PICO8_LINEAGE(PICO8_LINEAGE_G_));
     assert(lineage_of(&pico8_fv_new()->header) == PICO8_LINEAGE(PICO8_LINEAGE_G_));
     assert(lineage_of(&pico8_qb_new()->header) == 0 && lineage_of(&pico8_bg_new()->header) == 0);
@@ -781,7 +777,7 @@ int main(void) {
     in_fresh_child(test_life);
     in_fresh_child(test_descendants);
     in_fresh_child(test_qh_is_a_struct);
-    in_fresh_child(test_qh_of_follows_parent);
+    in_fresh_child(test_lineage_of_each_class);
 
     printf("test_pico8_qh: all tests passed\n");
     return 0;
diff --git a/tests/test_pico8_struct_header.c b/tests/test_pico8_struct_header.c
index 674a348..80117ca 100644
--- a/tests/test_pico8_struct_header.c
+++ b/tests/test_pico8_struct_header.c
@@ -160,6 +160,7 @@ static void test_carriers_link_their_storage(void) {
                                                       .fx = &jo->fx,
                                                       .p_ = &jo->p_,
                                                       .m_ = &jo->m_});
+    assert(jo->probe == NULL && jo->emitter == NULL);
     assert_links(&fr_jo->header, (pico8_struct_header_t){.fa = &fr_jo->def->fa,
                                                          .ra = (const pico8_vec_t *const *)&fr_jo->ra,
                                                          .p = (const pico8_vec_t *const *)&fr_jo->p,
@@ -169,7 +170,7 @@ static void test_carriers_link_their_storage(void) {
                                                          .fx = &fr_jo->fx,
                                                          .p_ = &fr_jo->p_,
                                                          .m_ = &fr_jo->m_,
-                                                         .probe = &fr->nr.qh.probe_own});
+                                                         .probe = &fr->nr.qh.jo.probe_own});
 }
 
 /* A planet's links, but for its emitter and probe. */
@@ -188,9 +189,9 @@ static void assert_qh_links(pico8_qh_t *qh, pico8_emitter_t *emitter) {
     pico8_struct_header_t want = planet_links(&qh->jo);
 
     want.emitter = emitter;
-    want.probe = &qh->probe_own;
+    want.probe = &qh->jo.probe_own;
     assert_links(&qh->jo.header, want);
-    assert(qh->probe == qh->jo.header.probe);
+    assert(qh->jo.probe == qh->jo.header.probe && qh->jo.emitter == qh->jo.header.emitter);
 }
 
 /* Emitter and probe carriers: each link is the component the instance
@@ -220,14 +221,13 @@ static void test_component_carriers_link_their_own(void) {
     assert_qh_links(pico8_qh_new(NULL), NULL);
     assert_qh_links(&nl->qh, NULL);
     assert_qh_links(&os->qh, NULL);
-    assert_qh_links(&hj->qh, &hj->emitter_own);
-    assert(hj->emitter == hj->qh.jo.header.emitter);
-    assert_qh_links(&pm->hj.qh, &pm->hj.emitter_own);
-    assert_qh_links(&mx->hj.qh, &mx->hj.emitter_own);
+    assert_qh_links(&hj->qh, &hj->qh.jo.emitter_own);
+    assert_qh_links(&pm->hj.qh, &pm->hj.qh.jo.emitter_own);
+    assert_qh_links(&mx->hj.qh, &mx->hj.qh.jo.emitter_own);
     assert_qh_links(&nr->qh, NULL);
-    want.emitter = &ga->emitter_own;
+    want.emitter = &ga->jo.emitter_own;
     assert_links(&ga->jo.header, want);
-    assert(ga->emitter == ga->jo.header.emitter);
+    assert(ga->jo.emitter == ga->jo.header.emitter && ga->jo.probe == NULL);
     assert_links(&mz->header, (pico8_struct_header_t){.fa = &mz->def->fa,
                                                       .ra = &mz->ra,
                                                       .p = &mz->p,
```

### Comptes, mesurés sur la copie

| Mesure | `b` | `c` |
|---|---|---|
| `lien-parent` (lignes / fichiers) | 42 / 17 | **0 / 0**, gardé |
| `parente` | 9 / 2 | 9 / 2 |
| `void-ptr` | 3 / 3 | 3 / 3 |
| `accesseurs-q6`, `défauts` et les sept autres gardées | 0, gardées | 0, gardées |
| `inclusion-mutuelle` (paires / modules, lignes du `--check`) | 16 / 15, 40 | 16 / 15, **41** |
| graphe d'usage (`usegraph.py`) | 19 | **18** |
| pied `résidu 2` | 94 / 38 | **53 / 27** |
| `--champs` | 240 ; 15 ; 20 | **238 ; 14 ; 19** |
| casts (grep de la réserve 4 de 2.13a) | 484 | **480** |
| tests de classe (`->cls`, `header.cls`, `_struct_class()`) | 636 | **647** |
| `sizeof(pico8_struct_header_t)`, `g_` | 128, 256 | 128, 256 |
| `sizeof` `jo`, `qh`, `nl`, `os`, `hj`, `pm`, `mx`, `ga`, `nr`, `fr` | 304, 344, 352, 344, 408, 408, 416, 376, 344, 344 | **424** chacun |
| `COMPOSITION_RESIDUE_GUARD` | 9 catégories | **10** (+ `lien-parent`) |
| constantes `*_HASH`/`FROZEN_*` changées | — | 0 |

**Attribution de `lien-parent`** (étude 2.10, annexe D, à la ligne près ; numéros de `b`). `src/`, **23** : les sept lectures (`economy_score.c:47`, `economy_tiers.c:261`, `planet.c:234`, `planet_display.c:154`, `:429`, `:436`, `planet_trade.c:151`), les deux définitions et leurs deux marches (`planet_family.c:561`, `:565`, `:570`, `:574`), les deux prototypes (`planet_family.h:109`, `:113`), les neuf `.parent` (`planet_family.c:46`, `:117`, `:161`, `:220`, `:307`, `:451`, `planet_family_trade.c:178`, `:232`, `:326`), le membre (`struct.h:166`). TU, **19** : `fr:148`, `hj:294`, `mx:269`, `nl:149`, `:249`, `:258`, `:394`, `nr:146`, `os:163`, `:285`, `pm:242`, `qh:91`, `:712`, `:713`, `:728`, `:729`, `:733`, `:735`, `:736`. 23 + 19 = 42 : la ligne 2.15c de la table de l'étude.

**Pied** : 94 − 42 + 1 (`planet.h:11`, `inclusion-mutuelle`) = **53** lignes ; fichiers : sortent `economy_score.c`, `economy_tiers.c`, `planet_family_trade.c`, `struct.h` et les huit TU `fr`, `hj`, `mx`, `nl`, `nr`, `os`, `pm`, `qh` (12), entre `planet.h` : 38 − 12 + 1 = **27**. `planet.c`, `planet_display.c`, `planet_family.c`/`.h`, `planet_trade.c` restent par leurs lignes `#include` de paires, `planet_family(_trade).h` par `parente`.

**`--champs`** : sortent `ga.{emitter, emitter_own, iu, ms}`, `hj.{emitter, emitter_own}`, `mx.ht`, `nl.qw`, `qh.{probe, probe_own, s}` (11), entrent les neuf de `jo` : 240 → 238 ; les trois `*_own` sans écriture (`ga`, `hj`, `qh`) deviennent deux (`jo`) : 15 → 14 et 20 → 19.

**Casts et tests de classe** (motif appliqué aux lignes `+`/`-` du diff) : casts, `src/` − 2 (les deux descentes) + 1 (`pico8_da`) = **− 1** ; TU − 3 (`test_pico8_qh.c` : deux `(const void *)e` et un `(const pico8_struct_header_t *)pico8_jo_new(…)`) ; la ligne `ht_of` de `test_pico8_mx.c` change de chemin, pas de compte. Tests de classe : `src/` + 8 − 2 = **+ 6** (huit tests de bit, deux `e->cls` des descentes) ; TU + 8 − 3 = **+ 5** (cinq `as_qh`, deux lignes réécrites de `hj`, `mx` ; trois retirées). **Aucun cast neuf net dans `src/`.**

### Contrat pour 2.15d et la suite (à recopier dans leurs PRD)

| Point | Pour | Ce qu'il faut savoir |
|---|---|---|
| structs de sorte réduites : `qh {jo}`, `nl {qh}`, `os {qh}`, `hj {qh}`, `pm {hj}`, `ga {jo}`, `nr {qh}`, `fr {nr}`, `mx {hj}` ; toutes à 424 octets | 2.15d | seuls des chemins (`.qh.jo.`, `.hj.qh.jo.`, `.nr.qh.jo.`) et des casts vers une sorte restent ; la réécriture de 2.15c (236 sites) donne l'ordre de grandeur, à guider par le compilateur de la même façon |
| `planet_display.c:18` n'est plus employé (essai à `c` : retirée, `planet_display.c` compile) ; la paire d'**usage** est déjà sortie | 2.15d | `inclusion-mutuelle` : **16 / 41 → 15 / 39** (et non 16 / 40 → 15 / 38) ; graphe d'usage : reste **18** (et non 19 → 18) |
| `planet.h:11` inclut `pico8_probe.h` (type de `probe_own`), une ligne de plus dans la paire `planet` ↔ `probe` | 2.17 | tous les comptes de lignes de 2.17 prennent + 1 tant que la ligne existe ; 2.17d (`pico8_probe_comp.h`, « le composant sonde ») doit la faire viser l'en-tête neuf, sinon `planet` ↔ `probe` reste une paire d'inclusion |
| `economy_score.c:12`, `economy_tiers.c:10`, `planet_trade.c:11` incluent `planet_family.h` sans plus en employer de nom (essai : retirées, compilent) | 2.17 | aucune n'est une paire ; hygiène seulement |
| `pico8_jo_struct_init` pose `probe`, `emitter` à NULL ; les liens du noyau restent posés par les porteurs seuls | 2.15d, toutes | « lien = membre » vaut pour les dix planètes ; `s`, `qw`, `iu`, `ms`, `ht` sont indéterminés hors de leur sorte (lus seulement sous leur bit ou sur leur sorte) |
| `pico8_qh_struct_class`, `pico8_hj_struct_class`, `pico8_nr_struct_class` ne sont plus appelés par le jeu (seul `parent()` les appelait) ; les TU les appellent | 2.17, 2.18 | 6 lignes perdent la couverture des e2e (preuve 6) ; ne pas les retirer : 8, 6, 5 TU les emploient |
| lecture d'une sorte : `self->header.cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)` (ou `NL`), puis le champ sur la planète | toutes | aucun `pico8_X_of` ni aide nommée `*_of` (le motif de `lien-parent`, gardé, la compterait) |

### Anti-tassement

**Interdit** :
- garder `pico8_qh_of`/`pico8_nl_of` sous un autre nom, ou une aide `pico8_*_of` (motif gardé) ; déduire la lignée des étiquettes ; toucher un masque `lineage` ;
- garder le membre `parent` « pour plus tard », ou un `.parent` dans une table ;
- laisser un champ monté aussi dans une struct de sorte (deux stockages) ; retirer une struct de sorte (2.15d) ; réécrire un chemin en cast vers `pico8_jo_t` là où le chemin enchâssé suffit ;
- poser un lien `emitter`/`probe` du noyau ailleurs que chez le porteur, ou dans `pico8_jo_struct_init` ; initialiser `s`, `qw`, `iu`, `ms`, `ht` hors de leur sorte ;
- inclure `particle.h` dans `planet.h` ; ajouter ou retirer une autre ligne `#include` dans `src/` (`planet_display.c:18` : 2.15d) ;
- changer un `*.sha256`, `tests/e2e/`, `tools/`, une constante `*_HASH`/`FROZEN_*` ; toucher le `Makefile` ailleurs qu'à `COMPOSITION_RESIDUE_GUARD`.

## Implementation Chunks

Un commit, **≈ 50 min** preuves comprises (mesuré sur la copie : l'essai a pris ≈ 20 min de réécriture guidée et ≈ 25 min de preuves) : ≈ 5 min pour appliquer le diff et lancer `make test`, `make e2e`, `MALLOC_PERTURB_=165 make e2e` (≈ 3 min) ; ≈ 1 min compteur, `--champs`, tailles, graphe d'usage ; jumelles et lignes K en arrière-plan sur deux arbres (14 × 2 + 3, ≈ 15 s chacune, ≈ 5 min) ; `line-coverage` des 26 e2e à `b` et `c` + `lset.py`, `class-coverage-net`, `sanitize-net` (≈ 8 min) ; `lin.c`, `obs.c`, preuve `const`, `neg.c`, garde (≈ 5 min) ; note « Découvert à 2.15c », message.

Pas de découpe : le diff est mécanique à 90 % (chemins) et chaque moitié seule laisserait une forme transitoire de plus. **Repli écrit** (si le commit déborde de 1,5 fois son budget avant d'être fait), deux commits verts :
- **c1** — les champs montent (`planet.h`, structs de sorte, chemins de `src/` et des TU, `pico8_jo_struct_init`, TU `struct_header`), les descentes restent et rendent la même instance (leurs lecteurs lisent `qh->jo.s`…) : `lien-parent` **42** ; `inclusion-mutuelle` 41 lignes ; pied 95 / 39 (calculé, non mesuré) ;
- **c2** — lecteurs sur les bits, descentes, `.parent`, membre retirés, garde, TU `qh`, glossaire : `lien-parent` → **0**, gardé ; comptes de la table.
Le repli, s'il sert, est écrit dans la note « Découvert à ».

## Testing Decisions

- **Pas de TU neuf** : `test_pico8_qh.c` réécrit son test de lignée sur le masque seul (`test_lineage_of_each_class`) ; `test_pico8_struct_header.c` ajoute trois contrôles « lien = membre » (NULL sur `jo`, `emitter` sur les huit sortes, `probe` NULL sur `ga`). Le reste est réécriture : chemins, descentes des TU en tests de bit. Aucun rejeu d'`events.json` dans un TU.
- **Bon test ici** : un masque se compare à 0 ou à la combinaison attendue sur une instance fraîche de chaque classe ; un membre de non-porteur se compare à NULL sous `MALLOC_PERTURB_` (le TU `struct_header` se relance lui-même avec `MALLOC_PERTURB_=90` et le tcache coupé : un NULL oublié n'y est pas NULL).
- **Prior art** : PRD 2.15a–b (jumelles, lignes K, `lin.c`, `obs.c`, preuve `const`, `lset.py`) ; étude 2.10, §2 et annexe D ; règle du parent (D.21) ; ensemble `L` (2.4 à 2.8).

### Preuves (parent de `c` = `b`)

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `MALLOC_PERTURB_=165 make e2e` **26 conformes** ; `git diff --stat b -- tests/e2e tools '*.sha256'` vide ; `git diff b | grep -E '^[-+].*(_HASH|FROZEN_)'` vide (Runs 7, 10).

2. **Preuve `const`** (copie de `c`, puis de `b`) : à `c`, les cinq champs montés déclarés `const`, `probe` et `emitter` déclarés `* const` dans `pico8_jo_t`, tout l'arbre compilé en `cc -std=c99 -Wall -Wextra -Werror -Isrc -fsyntax-only` (`syn.sh` de l'annexe A de l'étude 2.11) → **16** erreurs `assignment of read-only member` : `planet.c:130-131` (les deux NULL de `pico8_jo_struct_init`), `planet_family.c:53` (`probe`, `qh`), `:57` (`s`), `:90`, `:126` (`qw` : `nl:ly`, `_new` de `nl`), `:225` (`emitter`, `hj`), `:458`, `:465`, `:467`, `:486`, `:488` (`emitter`, `iu`, `ms` de `ga`, archétype puis clés), `planet_family_trade.c:100` (`s`, `nr:hb`), `:294`, `:333` (`ht` : `mx:kf`, `_new` de `mx`), et `tests/test_pico8_qh.c:538` (le TU remet `qw`). À `b`, les mêmes membres `const` dans les structs de sorte → **14** : les mêmes sites, sans les deux de `pico8_jo_struct_init`. **Aucun écrivain neuf, aucun perdu.** Preuve négative (`neg.c`, à `c`) : `e->cls->parent`, `{.parent = 0}` → `has no member named 'parent'` ; `pico8_qh_of(e)` → `implicit declaration` ; `qh->s`, `&hj->emitter_own` → `has no member` ; `mx->hj.qh.jo.ht = NULL`, `ga->jo.emitter->qy = NULL`, `nl->qh.jo.qw + mx->hj.qh.jo.s + ga->jo.iu` compilent (Runs 16, 17, 19).

3. **Mutations, règle du parent.** Une jumelle par bit et par classe (« bit ôté », « bit posé sur une non-porteuse ») et par lien. Au parent `b`, la mutation équivalente fausse la **descente** (`pico8_qh_of`/`pico8_nl_of` rend NULL, ou l'instance, pour la classe ; ou le `.parent` qui la relie est retiré) ; à l'enfant `c`, le **bit** de la table. Pour les liens, la même pose retirée des deux côtés. `make -k -j4 unit` puis `make -k e2e` ; restaurer par `git checkout -f`. **ÉCHEC si un TU rouge au parent n'est pas rouge à l'enfant, ou si l'e2e est < 26 au parent et 26 à l'enfant.** Lignes K : enfant seul, `make e2e` normal et sous `MALLOC_PERTURB_=165`. Script : `mut.py` + `muts.json` (hors dépôt, texte des mutations en annexe B).

**Jumelles** (parent `b`, enfant `c`) :

| # | Cible | Parent : mutation | Rouges au parent | Enfant : mutation | Rouges à l'enfant | e2e p / e | Verdict |
|---|---|---|---|---|---|---|---|
| QH-nr | bit `QH`, `nr` (plan ; **ligne K de 2.15b**) | `pico8_qh_of` rend NULL pour la table `nr` | `fr`, `nr`, `qh` | `QH` ôté du masque de `nr` | `fr`, `nr`, `qh` | 24 / 24 | conforme |
| QH-fr | bit `QH`, `fr` | `.parent` de `fr` retiré | `fr`, `nr`, `qh` | `QH` ôté du masque de `fr` | `fr`, `nr`, `qh` | 25 / 25 | conforme |
| QH-pm | bit `QH`, `pm` (deux marches) | `.parent` de `pm` retiré | `ga`, `hj`, `hp`, `pm`, `qh` | `QH` ôté du masque de `pm` | `ga`, `hj`, `hp`, `pm`, `qh` | 0 / 0 | conforme |
| QH-qh | bit `QH`, `qh` | `pico8_qh_of` rend NULL pour la table `qh` | `lq`, `qh`, `trade` | `QH` ôté du masque de `qh` | `lq`, `qh`, `trade` | 0 / 0 | conforme |
| NL-nl | bit `NL`, `nl` (**ligne K de 2.15b**) | `pico8_nl_of` rend toujours NULL | `nl`, `qh` | `NL` ôté du masque de `nl` | `nl`, `qh` | 26 / 26 | conforme |
| N-jo-QH | `jo` reçoit `QH` (**ligne K de 2.15b**) | `pico8_qh_of` rend l'instance pour la table `jo` | `fr`, `g_`, `ga`, `hj`, `hp`, `jc`, `lq`, `mx`, `nr`, `pm`, `qh` | `QH` posé sur le masque de `jo` | les mêmes 11 | 0 / 0 | conforme |
| N-ga-QH | `ga` reçoit `QH` | `pico8_qh_of` rend l'instance pour la table `ga` | `qh` | `QH` posé sur le masque de `ga` | `qh` | 26 / 26 | conforme |
| N-hj-NL | `hj` reçoit `NL` | `pico8_nl_of` rend l'instance pour la table `hj` | `qh` | `NL` posé sur le masque de `hj` | `qh` | 26 / 26 | conforme |
| JO-ga | bit `JO`, `ga` (plan) | `JO` ôté du masque de `ga` (même mutation) | `ga`, `hp`, `jo`, `qh` | idem | `ga`, `hp`, `jo`, `qh` | 26 / 26 | conforme |
| PR-qh | lien `probe`, lignée `qh` (plan) | pose du lien retirée de `pico8_qh_struct_init` | `fr`, `hj`, `jo`, `mx`, `nl`, `nr`, `os`, `qh`, `struct_header`, `ui` | idem (`&self->jo.probe_own`) | les mêmes 10 | 0 / 0 | conforme |
| PR-nl | lien `probe`, `nl` | lien mis à NULL après l'init, dans le `_new` de `nl` | `jo`, `nl`, `qh`, `struct_header`, `ui` | idem | les mêmes 5 | 0 / 0 | conforme |
| EM-hj | lien `emitter`, `hj`, `pm`, `mx` | pose retirée de `pico8_hj_struct_init` | `entity_kind`, `ga`, `hj`, `hp`, `ie`, `jo`, `jo_spec_defaults`, `mx`, `pm`, `qh`, `struct_header`, `ui` | idem (`&self->qh.jo.emitter_own`) | les mêmes 12 | 0 / 0 | conforme |
| EM-ga | lien `emitter`, `ga` | pose retirée du `_new` de `ga` | `entity_kind`, `ga`, `ga_export_panel_by`, `hj`, `hp`, `jc`, `jo`, `jo_spec_defaults`, `pm`, `qb`, `qh`, `struct_header`, `ui` | idem (`&self->jo.emitter_own`) | les mêmes 13 | 0 / 0 | conforme |
| N-jo-probe | `jo` et `ga` reçoivent un lien `probe` | `pico8_jo_struct_init` pose le lien vers une sonde statique (`def` = `pico8_probe_def_none`) | `struct_header`, `ui` | `pico8_jo_struct_init` pose le lien vers `&self->probe_own` (même `def`) | `struct_header`, `ui` | 26 / 26 | conforme |

**Lignes K** (enfant seul ; e2e normal, puis sous `MALLOC_PERTURB_=165`) :

| # | Mutation | Rouges | e2e | e2e perturbé |
|---|---|---|---|---|
| K-init-c | `pico8_jo_struct_init` ne pose plus `probe`, `emitter` à NULL | `struct_header` | 26/26 | 26/26 |
| K-membre-jo | `pico8_jo_struct_init` pose `probe = &self->probe_own` (membre sur un non-porteur) | `struct_header` | 26/26 | 26/26 |
| K-membre-jo-em | idem pour `emitter` | `struct_header` | 26/26 | 26/26 |

   Lecture : **aucun ÉCHEC** (14 jumelles, 28 lancements, rouges identiques des deux côtés ; 3 lignes K, toutes rouges). **Les trois lignes K de 2.15b sur les bits** (`K-QH-nr`, `K-NL-nl`, `K-QH-jo`) sont ici de **vraies jumelles** (QH-nr, NL-nl, N-jo-QH) : à `b`, un bit `QH`/`NL` n'avait de lecteur que le TU `qh` et la descente du parent ne le lisait pas ; à `c`, `src/` lit les bits (`pico8_jo_pq`, `jo:gh`, `jo:kj`, `jo:cn`, `da`, `dh`), et la mutation équivalente au parent fausse la descente. Elles sont rouges hors du seul `qh` (`fr`, `nr` ; `nl` ; onze TU pour N-jo-QH) aux deux bouts. N-ga-QH et N-hj-NL ne rougissent que `qh` (le masque épinglé) : aucun autre TU ne demande si un `ga` est un laboratoire ou un `hj` une plaque, ni au parent ni à l'enfant. Le premier jet de QH-nr au parent nommait `pico8_nr_struct_class()` depuis `planet_family.c`, que six TU ne lient pas (`COMPILE`) : relancé avec une déclaration faible, même résultat que l'enfant (Runs 13). K-init-c et les deux K-membre ne sont vues que du TU `struct_header` (sous perturbation) : le jeu ne lit `probe`/`emitter` que sur un porteur (26 e2e, aussi perturbés).

4. **Masque et descentes, table par table** (`lin.c`, annexe A) : lié aux objets de `src/` (sans `main`, `-no-pie`), il lit l'adresse des 40 tables `g_*_struct_class` dans `nm` de son propre binaire et imprime pour chacune `QH`, `NL`, `G_`, `JO` : à `b` (`-DV=0`) `pico8_qh_of`, `pico8_nl_of` (sur un en-tête `{.cls = table}`), le bit `G_`, `pico8_jo_is_family`, et sur sa sortie d'erreur les bits `QH`, `NL` ; à `c` (`-DV=1`) les quatre bits. Résultat : **40 lignes, sorties identiques** (même `md5`, `7e4ab61e…`, celui de la preuve 4 du PRD 2.15a–b) ; à `b`, les 40 × 2 colonnes de contrôle égales aux descentes. Les douze lignes non nulles sont la table du contrôle 4 de l'étude (`jo`, `ga` : `JO` ; `qh`, `os`, `hj`, `pm`, `mx`, `nr`, `fr` : `JO|QH` ; `nl` : `JO|QH|NL` ; `g_`, `fv` : `G_`) ; les 28 autres, 0 (Runs 14).

5. **Valeurs observables, parent contre enfant** (`obs.c`, annexe A) : lié aux objets de `src/`, il boote le cart (`pico8_cart_load`, `pico8__init`, `pico8_eu`), construit une instance fraîche des dix planètes (par leur `kind`, clé `id` seule) et de `g_`, `fv`, `mz`, `jc`, `qb`, `bg`, et imprime pour chacune : `QH`/`NL` (descentes à `b`, bits à `c`), `JO`, `G_`, présence des liens `emitter`/`probe` ; pour la lignée `qh` : `s`, « lien = membre », `od`/`has_od`, `def->dw`, `fp`, `dq`, `et` ; pour `nl` : `qw` ; pour `ga` : `iu`, `ms`, l'émetteur (`py`, `k_` = archétype, `qo`, `ro`, `qg`, `q_`, taille de `qy`) ; pour `hj`, `pm`, `mx` : l'émetteur, `pn`, `du` ; pour `mx` : `ht` = archétype et sa taille ; pour chaque planète : `oe`, `kw` = `kw_own` (effet de `pico8_jo_pq`), `pico8_da`, `pico8_dh`, puis l'empreinte de la mémoire avant et après `pico8_jo_kj` (placée à l'écran) et `pico8_jo_gh`. Compilé `-DV=0` sur `b`, `-DV=1` sur `c` : **16 lignes, sorties identiques** (`9da94ae5…`), et identiques sous `MALLOC_PERTURB_=165` aux deux arbres (Runs 15, 19).

6. **Chemin touché, ensemble `L`** : `make line-coverage EVENTS="<les 26 e2e>"` à `b` et `c` (**6052**, **6032** lignes), appariement `difflib` des sources (`lset.py`, annexe A). `L` (couvertes à `b`, retirées ou réécrites) : **78** — `economy_score.c:47`, `:49`, `economy_tiers.c:261`, `planet.c:234-236`, `planet_display.c:154-156`, `:205`, `:429-430`, `:436`, `:439-440`, `planet_family.c:53-59`, `:91`, `:127`, `:227-238`, `:372-374`, `:400`, `:462-473`, `:486-493`, `:561-576` (10 lignes exécutables des deux descentes), `planet_family_trade.c:100`, `:290`, `:296`, `:336`, `planet_trade.c:151`, `:154-156`. Lignes exécutables neuves : **64 sur 64** couvertes (`planet.c:130-131`, les lectures sur bit, les chemins réécrits) ; les non couvertes sont l'`#include`, des commentaires, des déclarations de membres et la ligne de continuation de `nl:ly` (`planet_family.c:92`, que `gcov` compte sur `:90`). **Lignes inchangées : 0 gagnée, 6 perdues**, toutes attendues : `planet_family.c:531-532` (`pico8_qh_struct_class`), `:615-616` (`pico8_hj_struct_class`), `planet_family_trade.c:380-381` (`pico8_nr_struct_class`) — le jeu ne les appelait que par `cls->parent()`, dans les marches retirées ; les TU les appellent encore (8, 6, 5 TU) (Runs 18, 20).

7. `class-coverage-net` : **oui**, `couverture: OK -- chaque classe non exemptée est instanciée (26 rejeu(x))` à `c`. `sanitize-net` : **oui** (disposition des dix types planète changée), `sanitize: 26 propre(s)` à `c` (Runs 16).

8. **Garde mordante** : à `c`, le membre `const pico8_struct_class_t *(*parent)(void);` remis dans `pico8_struct_class_t` → `make composition-guard` en échec (code 2), `lien-parent: src/pico8_struct.h:165:…`, `résidu gardé (…,lien-parent) : 1 ligne(s)` ; retiré → 0 (Runs 17).

9. **Graphe d'usage et inclusions** : `usegraph.py` 19 → 18, seule sortante `planet_display` ↔ `planet_family`, aucune entrante (`diff` des sorties) ; `inclusion-mutuelle` 16 / 15 aux deux arbres, `--check` 40 → 41 (seule ligne neuve `planet.h:11`) (Runs 20).

### Greps de Sortie

À `c` (numéros de `c`) :
1. `grep -rnE '\bpico8_(qh|nl)_of\b|(->|\.|\(\*)parent\b' src tests | wc -l` → **0** ; `sh tools/composition_residue.sh --check lien-parent src tests | tail -1` → `résidu gardé (lien-parent) : 0 ligne(s)`.
2. `grep -rn 'lineage & PICO8_LINEAGE(PICO8_LINEAGE_\(QH\|NL\))' src` → **8** lignes : `economy_score.c:47`, `economy_tiers.c:261`, `planet.c:235`, `planet_display.c:153`, `:203`, `:425`, `:433`, `planet_trade.c:153`.
3. `grep -rnE 'header\.(emitter|probe) = ' src` → **7** lignes, inchangées en nombre depuis `b` : `planet_family.c:52` (`&self->jo.probe_own`), `:224` (`&self->qh.jo.emitter_own`), `:457` (`&self->jo.emitter_own`), `probe.c:392-393`, `slipway.c:356`, `wormhole.c:217` ; `grep -n 'self->\(probe\|emitter\) = NULL' src/pico8_planet.c` → `:130`, `:131`.
4. `grep -nE '^\s+(pico8_fixed_t|const pico8_int_seq_t|pico8_probe_t|pico8_emitter_t)' src/pico8_planet_family.h src/pico8_planet_family_trade.h` → vide (plus aucun champ dans une struct de sorte) ; `grep -c '^typedef struct {' src/pico8_planet_family.h src/pico8_planet_family_trade.h` inchangé (neuf structs de sorte, plus `pico8_ga_keys_t`).
5. `grep -rn '\.lineage = ' src | wc -l` → **12** (inchangé).
6. `git diff b -- src | grep -E '^[-+]#include'` → exactement `+#include "pico8_probe.h"` (`planet.h`) ; `grep -rn 'pico8_struct_header_init(' src | wc -l` → 9.
7. `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` → finit par `,accesseurs-q6,lien-parent`.
8. `make composition-residue`, `--champs`, les greps de casts et de classe → la table « Comptes ».
9. `git diff --stat b` : les fichiers de la section « Modules » et `docs/plan-3.md`, rien d'autre.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable (`prd215c/copie`, arbres de travail `wt_b`, `wt_c`, export `tree_b`).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout 4947c55` ; `git diff --stat cecbcdb 4947c55 -- src tests Makefile tools` ; diffs 2.15a et 2.15b extraits du PRD 2.15a–b, `git apply --check`/`git apply`, deux commits | `b` | 0 | diff de départ vide ; deux `git apply` sans reste | 3 s |
| 2 | `make -j4 test` ; `make e2e` | `b` | 0 | vert ; 26 conformes | ≈ 3 min |
| 3 | `base.sh` (compteur, `--champs`, `--check inclusion-mutuelle`, casts, tests de classe, `usegraph.py`, `sizeof`, garde) | `b` | 0 | table « État de départ » (= colonne `b` du PRD 2.15a–b) | 5 s |
| 4 | en-têtes (`planet.h`, structs de sorte, `struct.h`), neuf `.parent` et deux descentes retirés, sept lecteurs de `src/` réécrits à la main | — | — | — | — |
| 5 | `fix.py` (préfixe inséré devant le membre, d'après `has no member named` et le type de l'erreur), premier jet : la colonne de GCC vise l'opérateur `->`, pas le membre ; assertion du script, rien écrit | — | 1 | corrigé (saut de `->`/`.`) | < 5 s |
| 6 | `fix.py` ×5 sur `syn.sh` ; `planet_display.c:203` (variable `qh` réutilisée) et les huit descentes des TU réécrites à la main ; `syn.sh -Werror` | — | 0 | 236 sites, puis 0 erreur | < 1 min |
| 7 | `make -j4 test` ; `make e2e` ; `MALLOC_PERTURB_=165 make e2e` | premier `c` | 0 | vert ; 26 ; 26 | ≈ 4 min |
| 8 | `base.sh` | premier `c` | 0 | casts 485 (cinq `as_qh` par cast) | 5 s |
| 9 | `as_qh` sans cast (`const pico8_struct_header_t *h = e`), trois lignes de TU > 120 colonnes coupées, `nl:ly` recoupée, TU `struct_header` (trois contrôles), glossaire, garde ; commit `c` | `c` | 0 | casts 480 | — |
| 10 | `make -j4 test` ; `make e2e` ; `MALLOC_PERTURB_=165 make e2e` ; `base.sh` | `c` | 0 | vert ; 26 ; 26 ; table « Comptes » | ≈ 4 min |
| 11 | `mut.py` premier jet (parent et enfant en parallèle) : le lecteur de sortie ne reconnaissait que `] Error`, pas `] Aborted` : « aucun » rouge partout ; arrêté (PID), corrigé | `wt_b`, `wt_c` | — | — | 2 min |
| 12 | `mut.py` : 14 jumelles ×2, 3 lignes K | `wt_b`, `wt_c` | 0 | table des jumelles ; QH-nr au parent : `COMPILE` (six TU ne lient pas `pico8_nr_struct_class`) | ≈ 5 min |
| 13 | QH-nr au parent relancé, déclaration `__attribute__((weak))` | `wt_b` | 0 | `fr`, `nr`, `qh` ; e2e 24 ; aucune erreur de lien | 6 s |
| 14 | `lin.c` à `c` (`-DV=1`), puis à `b` (`-DV=0`) ; premier lancement à `c` : sortie écrite dans la copie (chemin relatif), relancé en absolu | `c`, `b` | 0 | 40 lignes, `7e4ab61e…` aux deux ; contrôles de `b` égaux | < 5 s |
| 15 | `obs.c` : premier jet, `jo:kj` ne dessinait rien (`ra` NULL, `pico8_gd` faux) puis segfault (prototype de `pico8_mo_qb` manquant) ; `ra` posé à `mo.ra` + (64, 64), `pico8_entity.h` inclus | `c` | 139 puis 0 | 16 lignes | < 5 s |
| 16 | `make line-coverage` des 26 e2e ; `make class-coverage-net` ; `make -j2 sanitize-net` ; preuve `const` (`syn.sh`) | `c` | 0 | 6032 lignes ; `couverture: OK` ; `26 propre(s)` ; 16 erreurs | ≈ 8 min |
| 17 | `neg.c` (six variantes) ; garde : membre `parent` remis, `make composition-guard` | `c` | 1 ×5, 0 ; 2 | erreurs attendues ; 1 ligne `lien-parent`, échec | < 10 s |
| 18 | `checkout --detach b` dans la copie ; `make line-coverage` ; `make -j2 unit` | `b` | 0 | 6052 lignes | ≈ 4 min |
| 19 | `lin.c` (`-DV=0`), `obs.c` (`-DV=0`) normal et perturbé ; preuve `const` à `b` | `b` | 0 | `7e4ab61e…` ; `9da94ae5…` ×4 (b, b perturbé, c, c perturbé) ; 14 erreurs | < 1 min |
| 20 | `lset.py` ; `usegraph.py` à `c` ; `--check` des quatre catégories sur `tree_b` et `c` (fichiers du pied) | `b`, `c` | 0 | `L` 78, neuves 64/64, 6 perdues ; `paires 18` ; 38 → 27 fichiers | < 5 s |
| 21 | essai : `planet_display.c:18` retirée à `c`, `-fsyntax-only` ; de même `economy_score.c`, `economy_tiers.c`, `planet_trade.c` (copie de sauvegarde écrite par erreur dans `/tmp`, effacée aussitôt) | `c` | 0 | les quatre compilent | < 5 s |
| 22 | greps de Sortie | `c` | 0 | section « Greps de Sortie » | < 1 s |
| 23 | le diff extrait de ce PRD, `git apply --check`/`git apply` sur `b` (`wt_b`), `git diff c` | — | 0 | `git apply` sans reste ; diff vide | 2 s |
| 24 | `git fetch` de la branche dans la copie (2.15a–b commités entre-temps) ; `git diff --stat f09dfd2 FETCH_HEAD -- src tests Makefile tools` | — | 0 | vide : `b` = `c239f47` | 1 s |
| 25 | `rm -rf` de la copie, des arbres de travail, de l'export et des fichiers d'essai | — | 0 | supprimés | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas `c239f47` (ou un descendant qui ne touche que `docs/`), ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si 2.15c n'est pas un commit (hors repli écrit), vert (`make test`, `make e2e` 26 conformes, et 26 sous `MALLOC_PERTURB_=165`), ou si un `*.sha256`, `tests/e2e/`, `tools/`, une constante `*_HASH`/`FROZEN_*` change, ou le `Makefile` ailleurs qu'à `COMPOSITION_RESIDUE_GUARD`.
- `FAILED:` si une jumelle a un TU rouge au parent qui ne l'est pas à l'enfant, ou un e2e < 26 au parent et 26 à l'enfant ; si une ligne K est verte ; si QH-nr, NL-nl ou N-jo-QH ne sont pas menées en jumelles.
- `FAILED:` s'il reste `pico8_qh_of`, `pico8_nl_of`, un `.parent`, le membre `parent`, ou une ligne de `lien-parent` ; si `lien-parent` n'est pas dans `COMPOSITION_RESIDUE_GUARD` ; si la garde n'a pas été prouvée mordante.
- `FAILED:` si un des neuf champs reste dans une struct de sorte, ou si une struct de sorte est retirée ; si un lien `emitter`/`probe` du noyau est posé ailleurs que chez le porteur ; si `pico8_jo_struct_init` ne pose pas `probe`/`emitter` à NULL, ou pose un autre champ de sorte.
- `FAILED:` si la sortie de `lin.c` diffère entre `b` et `c`, ou celle d'`obs.c` (aussi sous perturbation) ; si un masque change.
- `FAILED:` si `test_lineage_of_each_class` ne compare pas le masque des dix sortes, de `ga`, `jo`, `g_`, `fv`, `qb`, `bg`, `mz`, `jc`, ou si le TU `struct_header` ne vérifie pas `probe`/`emitter` NULL sur `jo` et « lien = membre » pour `emitter` sur les huit sortes.
- `FAILED:` si les comptes ne sont pas ceux de la table « Comptes » (`lien-parent` 0 ; `parente` 9 ; `void-ptr` 3 ; pied 53 / 27 ; `--champs` 238 ; `inclusion-mutuelle` 16 / 15, 41 lignes ; usage 18 ; casts 480 ; tests de classe 647 ; planètes 424 octets), si une catégorie gardée n'est plus à 0, si une ligne `#include` de `src/` autre que `planet.h:11` est ajoutée ou retirée, ou si un cast neuf net apparaît dans `src/`.
- `FAILED:` si `class-coverage-net` ou `sanitize-net` est rouge ; si une ligne inchangée perd la couverture des e2e hors des six nommées (preuve 6), en gagne, ou si une ligne exécutable neuve n'est pas couverte ; si l'écrivain d'un champ monté change (preuve `const`).
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si le message de commit omet : les champs montés, les lecteurs sur bit, ce qui est retiré, la garde, la table des jumelles avec leur résultat au parent (et les trois anciennes lignes K), les lignes K, la preuve `const`, la table de la preuve 4, les comptes, `L` et les six lignes perdues, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.15c » au bout de sa ligne du plan, ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Les neuf structs de sorte, leurs chemins et casts, `planet_display.c:18` (2.15d) ; les membres `hd`/`mm` typés (2.16).
- Les paires d'inclusion (2.17), dont `planet.h:11` et les includes sans usage d'`economy_score.c`, `economy_tiers.c`, `planet_trade.c` ; `particle.c:12`.
- La taille des planètes (+ 16 à + 120 octets selon la sorte : A et Q-2.10 l'acceptent) ; la taille du code C ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. `planet.h` inclut `pico8_probe.h`, pas `pico8_particle.h`.** `probe_own`/`emitter_own` demandent des types complets. `probe.h` inclut déjà `particle.h` et ne remonte jamais à `planet.h` (pas de cycle d'inclusion) ; `planet` ↔ `probe` est déjà une paire, la ligne n'en crée pas. `particle.h` créerait `planet` ↔ `particle` (`particle.c:12`). La ligne est la seule ajoutée ; 2.17d la reprend.
- **G2. `pico8_jo_struct_init` pose les membres `probe`/`emitter` à NULL, et rien d'autre.** C'est la struct de l'étude (« NULL hors de la lignée qh », « NULL hors de hj, pm, mx, ga ») et l'invariant « lien = membre » sur toute planète ; les scalaires restent indéterminés hors de leur sorte, comme `m_` sur `g_` à 2.15b (G6 de ce PRD-là) : aucun lecteur ne les lit sans le bit ou la sorte.
- **G3. Les lecteurs testent le bit en place**, sans aide : une aide `pico8_X_of` serait comptée par le motif gardé, et une aide `is_qh` n'éviterait qu'une expression. `pico8_da` garde sa garde NULL (`qh_of(NULL)` était NULL) ; `pico8_dh` n'en a pas besoin (`:212-216`).
- **G4. `planet_display.c:18` reste pour 2.15d**, comme le plan le dit, bien qu'elle ne serve plus dès `c` (essai) : le changement d'inclusion est groupé avec la sous-étape qui la nomme, et le compte de 2.15d est corrigé dans le contrat (16 / 41 → 15 / 39).
- **G5. Les TU suivent sans cast** : `as_qh` passe par `const pico8_struct_header_t *h = e` ; `test_pico8_nl.c` gagne `as_nl` (deux appels) ; le test de lignée perd son nom de descente (`test_qh_of_follows_parent` → `test_lineage_of_each_class`).
- **G6. Trois contrôles de plus dans `test_pico8_struct_header.c`** (NULL sur `jo`, `emitter` = lien sur les huit sortes, `probe` NULL sur `ga`) : sans eux, K-init-c et les K-membre ne rougissaient aucun TU (le jeu ne lit ces membres que sur un porteur).
- **G7. Le glossaire change d'une ligne** (`iq`) : le symbole qu'il nommait, `cls->parent`, n'existe plus.
- **G8. Un commit, pas deux** : chaque moitié seule laisse une forme de plus (descentes qui rendent une instance dont les champs sont ailleurs) ; repli écrit.

### Surprises de mesure, pour `plan-pilot`

1. **La paire d'usage `planet_display` ↔ `planet_family` tombe à 2.15c**, pas à 2.15d : une fois les champs sur la planète, `planet_display` n'emploie plus `pico8_qh_t`/`pico8_nl_t`. La ligne `#include` reste (2.15d). Usage 19 → **18** ici ; 2.15d le laisse à 18.
2. **`inclusion-mutuelle` passe de 40 à 41 lignes** (`planet.h:11`), paires inchangées : 2.15d arrive à 15 / 39, et 2.17 compte une ligne de plus jusqu'à 2.17d.
3. **Six lignes inchangées perdent la couverture des e2e** : trois accesseurs de classe que seul `parent()` appelait en jeu.
4. **Une quatrième lecture de sorte dans `planet_display.c`** (`:205`, `last_only`), que `lien-parent` ne comptait pas : elle réutilisait la variable de `:154`.
5. **Les dix types planète font 424 octets dès 2.15c**, pas 432 à 2.15d : `pico8_probe_t` fait 24 octets depuis 2.13b (l'étude le mesurait à 48). Une planète `jo` passe de 304 à 424 octets, une `mx` de 416 à 424.
6. **Les trois lignes K de 2.15b sur `QH`/`NL` deviennent des jumelles** et rougissent plus de TU qu'à `b` (`fr`, `nr` ; onze TU pour `QH` posé sur `jo`).
7. **La réécriture des chemins est guidée par le compilateur** (236 sites, `fix.py`, annexe A) ; 2.15d peut réutiliser la même méthode en sens inverse.

### Questions pour l'humain

Aucune.

### Annexe A — programmes d'essai (texte exact)

Tous hors dépôt. `lin.c` et `obs.c` se construisent après `make test` dans l'arbre : `cc -std=c99 -DV=<0|1> -Isrc -c X.c && cc -no-pie -o X X.o $(ls tmp/obj/src/pico8_*.o | grep -v fixed_test_double) -lm` ; `lin` se lance par `nm lin | awk '$3 ~ /^g_[a-z0-9_]+_struct_class$/ {print $1, $3}' | sort -k2 | ./lin` ; `obs` depuis la racine de l'arbre (il lit `pico8/gfx.json`, `pico8/map.json`). `V=0` : parent `b` ; `V=1` : enfant `c`.

`lin.c` :

```c
/* Lignee de chaque table de classe : descentes qh_of/nl_of (V=0, parent) contre bits QH/NL (V=1, enfant). */
#include <stdio.h>
#include <string.h>
#include "pico8_struct.h"
#include "pico8_planet.h"
#include "pico8_planet_family.h"
static unsigned long addrs[64]; static char names[64][96]; static int n;
#define BIT(c, l) (((c)->lineage & PICO8_LINEAGE(l)) != 0)
int main(void) {
    unsigned long a; char nm[96]; int i;
    while (scanf("%lx %95s", &a, nm) == 2) { addrs[n] = a; strcpy(names[n], nm); n++; }
    for (i = 0; i < n; i++) {
        const pico8_struct_class_t *c = (const pico8_struct_class_t *)addrs[i];
        pico8_struct_header_t h = {.cls = c};
#if V == 0
        printf("%-34s QH=%d NL=%d G_=%d JO=%d\n", names[i], pico8_qh_of(&h) != NULL, pico8_nl_of(&h) != NULL,
               BIT(c, PICO8_LINEAGE_G_), pico8_jo_is_family(&h));
        fprintf(stderr, "%-34s bits QH=%d NL=%d\n", names[i], BIT(c, PICO8_LINEAGE_QH), BIT(c, PICO8_LINEAGE_NL));
#else
        printf("%-34s QH=%d NL=%d G_=%d JO=%d\n", names[i], BIT(c, PICO8_LINEAGE_QH), BIT(c, PICO8_LINEAGE_NL),
               BIT(c, PICO8_LINEAGE_G_), pico8_jo_is_family(&h));
#endif
    }
    return 0;
}
```

`obs.c` :

```c
/* Valeurs observables des planetes et de leur lignee : V=0 (parent, sortes et descentes), V=1 (enfant, pico8_jo_t et bits). */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico8_assets.h"
#include "pico8_bg.h"
#include "pico8_bootstrap.h"
#include "pico8_cart.h"
#include "pico8_dispatch_context.h"
#include "pico8_economy_score.h"
#include "pico8_economy_tiers.h"
#include "pico8_entity.h"
#include "pico8_gfx.h"
#include "pico8_help.h"
#include "pico8_input.h"
#include "pico8_mem.h"
#include "pico8_planet.h"
#include "pico8_planet_display.h"
#include "pico8_planet_family.h"
#include "pico8_planet_family_trade.h"
#include "pico8_probe.h"
#include "pico8_scroll.h"
#include "pico8_slipway.h"
#include "pico8_wormhole.h"
static pico8_mem_t mem; static pico8_gfx_t gfx; static pico8_input_t input;
#define BIT(h, l) (((h)->cls->lineage & PICO8_LINEAGE(l)) != 0)
#if V == 0
#define ISQH(h) (pico8_qh_of(h) != NULL)
#define ISNL(h) (pico8_nl_of(h) != NULL)
#define S_OF(e) (((const pico8_qh_t *)(e))->s)
#define PROBE_OF(e) (((const pico8_qh_t *)(e))->probe)
#define QW_OF(e) (((const pico8_nl_t *)(e))->qw)
#define IU_OF(e) (((const pico8_ga_t *)(e))->iu)
#define MS_OF(e) (((const pico8_ga_t *)(e))->ms)
#define GA_EM(e) (((const pico8_ga_t *)(e))->emitter)
#define HJ_EM(e) (((const pico8_hj_t *)(e))->emitter)
#define HT_OF(e) (((const pico8_mx_t *)(e))->ht)
#else
#define ISQH(h) BIT(h, PICO8_LINEAGE_QH)
#define ISNL(h) BIT(h, PICO8_LINEAGE_NL)
#define S_OF(e) (((const pico8_jo_t *)(e))->s)
#define PROBE_OF(e) (((const pico8_jo_t *)(e))->probe)
#define QW_OF(e) (((const pico8_jo_t *)(e))->qw)
#define IU_OF(e) (((const pico8_jo_t *)(e))->iu)
#define MS_OF(e) (((const pico8_jo_t *)(e))->ms)
#define GA_EM(e) (((const pico8_jo_t *)(e))->emitter)
#define HJ_EM(e) (((const pico8_jo_t *)(e))->emitter)
#define HT_OF(e) (((const pico8_jo_t *)(e))->ht)
#endif
static uint32_t fnv(const void *p, size_t n) { const unsigned char *b = p; uint32_t h = 2166136261u; while (n--) { h ^= *b++; h *= 16777619u; } return h; }
static void em(const pico8_emitter_t *m, int hj) {
    printf(" em{py=%d k_=%d qo=%d ro=%d qg=%d q_=%d qy=%d", (int)m->py, m->k_ == m->def->k_, (int)m->qo, (int)m->ro, (int)m->qg, (int)m->q_, m->qy == NULL ? -1 : m->qy->count);
    if (hj) printf(" pn=%d du=%d", (int)m->pn, (int)m->du);
    printf("}");
}
static void show(const char *name, pico8_struct_header_t *h, int planet) {
    printf("%-4s QH=%d NL=%d JO=%d G_=%d em=%d pr=%d", name, ISQH(h), ISNL(h), BIT(h, PICO8_LINEAGE_JO), BIT(h, PICO8_LINEAGE_G_), h->emitter != NULL, h->probe != NULL);
    if (ISQH(h)) {
        const pico8_probe_t *p = PROBE_OF(h);
        printf(" s=%d lien=membre:%d od=%d/%d dw=%d fp=%d dq=%s et=%d", (int)S_OF(h), p == h->probe, (int)p->od, p->has_od, (int)p->def->dw, (int)p->def->fp, p->def->dq ? p->def->dq : "-", p->et != NULL);
    }
    if (ISNL(h)) printf(" qw=%d", (int)QW_OF(h));
    if (h->cls == pico8_ga_struct_class()) { printf(" iu=%d ms=%d lien=membre:%d", (int)IU_OF(h), (int)MS_OF(h), GA_EM(h) == h->emitter); em(h->emitter, 0); }
    if (h->cls == pico8_hj_struct_class() || h->cls == pico8_pm_struct_class() || h->cls == pico8_mx_struct_class()) { printf(" lien=membre:%d", HJ_EM(h) == h->emitter); em(h->emitter, 1); }
    if (h->cls == pico8_mx_struct_class()) printf(" ht=def:%d n=%d", HT_OF(h) == ((const pico8_jo_t *)h)->def->ht, HT_OF(h) ? HT_OF(h)->count : -1);
    if (planet) {
        pico8_jo_t *jo = (pico8_jo_t *)h;
        pico8_vec_t at = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
        uint32_t before, after;
        printf(" oe=%d kw_own=%d da=%d dh=%d", jo->oe ? jo->oe->count : -1, jo->kw == &jo->kw_own, (int)pico8_da(h), (int)pico8_dh(h));
        memset(mem.data + 0x6000, 0, 0x2000);
        before = fnv(mem.data, sizeof mem.data);
        jo->header.me = NULL;
        { static pico8_vec_t ras[16]; static int nra; const pico8_qb_t *mo = pico8_mo_qb();
          ras[nra] = mo != NULL && mo->ra != NULL ? pico8_vec_add(*mo->ra, at) : at; jo->ra = &ras[nra++]; }
        pico8_jo_kj(&gfx, &mem, jo, at);
        pico8_jo_gh(jo);
        after = fnv(mem.data, sizeof mem.data);
        printf(" kj=%08x/%08x", (unsigned)before, (unsigned)after);
    }
    printf("\n");
}
int main(void) {
    pico8_jo_keys_t keys = {"s99", NULL, NULL};
    pico8_mem_init(&mem); pico8_gfx_init(&gfx);
    if (!pico8_load_gfx_json(&mem, "pico8/gfx.json") || !pico8_load_map_json(&mem, "pico8/map.json")) return 1;
    pico8_input_init(&input); pico8_dispatch_set_gfx(&gfx); pico8_dispatch_set_mem(&mem); pico8_dispatch_set_input(&input);
    pico8_cart_load(); pico8__init(); pico8_eu(NULL);
    show("jo", &pico8_jo_new(&keys)->header, 1);
    show("qh", pico8_qh_kind()->new_entity(&keys), 1);
    show("nl", pico8_nl_kind()->new_entity(&keys), 1);
    show("os", pico8_os_kind()->new_entity(&keys), 1);
    show("hj", pico8_hj_kind()->new_entity(&keys), 1);
    show("pm", pico8_pm_kind()->new_entity(&keys), 1);
    show("mx", pico8_mx_kind()->new_entity(&keys), 1);
    show("nr", pico8_nr_kind()->new_entity(&keys), 1);
    show("fr", pico8_fr_kind()->new_entity(&keys), 1);
    show("ga", pico8_ga_kind()->new_entity(&keys), 1);
    show("g_", &pico8_g__new(NULL)->header, 0);
    show("fv", &pico8_fv_new()->header, 0);
    show("mz", &pico8_mz_new()->header, 0);
    show("jc", &pico8_jc_new(NULL)->header, 0);
    show("qb", &pico8_qb_new()->header, 0);
    show("bg", &pico8_bg_new()->header, 0);
    return 0;
}
```

`lset.py` (`python3 lset.py <copie> <b> <c> <couverture de b> <couverture de c>`) :

```python
import sys,subprocess,difflib,collections
T,B,C,lb,lc=sys.argv[1:6]
def cov(f):
    d=collections.defaultdict(set)
    for l in open(f):
        p,n=l.strip().rsplit(':',1); d[p].add(int(n))
    return d
Cb,Cc=cov(lb),cov(lc)
files=subprocess.run(['git','-C',T,'diff','--name-only',B,C,'--','src'],capture_output=True,text=True).stdout.split()
lost=[];gained=[];L=[];new=[];newcov=[]
for f in sorted(set(files)|set(Cb)|set(Cc)):
    def src(r):
        x=subprocess.run(['git','-C',T,'show',f'{r}:{f}'],capture_output=True,text=True); return x.stdout.split('\n') if x.returncode==0 else []
    a,b=src(B),src(C)
    sm=difflib.SequenceMatcher(None,a,b,autojunk=False)
    for tag,i1,i2,j1,j2 in sm.get_opcodes():
        if tag=='equal':
            for k in range(i2-i1):
                x,y=i1+k+1,j1+k+1
                if x in Cb[f] and y not in Cc[f]: lost.append(f'{f}:{x}')
                if x not in Cb[f] and y in Cc[f]: gained.append(f'{f}:{y}')
        else:
            for x in range(i1+1,i2+1):
                if x in Cb[f]: L.append(f'{f}:{x}')
            for y in range(j1+1,j2+1):
                new.append((f,y,b[y-1]))
                if y in Cc[f]: newcov.append(f'{f}:{y}')
print('perdues',len(lost),lost[:10]); print('gagnees',len(gained),gained[:10])
print('L',len(L)); print(' '.join(L))
print('neuves couvertes',len(newcov)); print(' '.join(newcov))
print('neuves non couvertes:')
for f,y,t in new:
    if f'{f}:{y}' not in newcov: print(f'  {f}:{y}\t{t}')
```

`fix.py` (lit sur son entrée les erreurs de `syn.sh` sous `LANG=C`, insère devant le membre le chemin vers `jo` selon le type de l'erreur ; relancé jusqu'à 0 édition) :

```python
import re,sys,subprocess,collections
PRE={'qh':'jo.','ga':'jo.','nl':'qh.jo.','os':'qh.jo.','hj':'qh.jo.','nr':'qh.jo.','pm':'hj.qh.jo.','mx':'hj.qh.jo.','fr':'nr.qh.jo.'}
FIELDS={'s','qw','iu','ms','ht','probe','probe_own','emitter','emitter_own'}
err=sys.stdin.read()
edits=collections.defaultdict(set)
for m in re.finditer(r"^([^:\s]+):(\d+):(\d+): error: '(?:const )?pico8_(\w+?)_t'(?: \{aka [^}]*\})? has no member named '(\w+)'",err,re.M):
    f,l,c,t,fld=m.group(1),int(m.group(2)),int(m.group(3)),m.group(4),m.group(5)
    if t in PRE and fld in FIELDS: edits[f].add((l,c,PRE[t],fld))
n=0
for f,es in edits.items():
    L=open(f).read().split('\n')
    for l,c,p,fld in sorted(es,reverse=True):
        line=L[l-1]
        if line[c-1:c+1]=='->': c+=2
        elif line[c-1]=='.': c+=1
        while line[c-1]==' ': c+=1
        assert line[c-1:c-1+len(fld)]==fld,(f,l,c,line)
        L[l-1]=line[:c-1]+p+line[c-1:]; n+=1
    open(f,'w').write('\n'.join(L))
print('edits',n)
```

### Annexe B — mutations (`muts.json`, texte exact ; `p` : parent `b`, `c` : enfant `c` ; `[fichier, ancre ou null, ancien, neuf]`, l'ancien cherché après l'ancre, unique sans ancre)

```json
{
 "QH-nr": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "    while (cls != NULL && cls != &g_qh_struct_class) {",
    "    {\n        extern const pico8_struct_class_t *pico8_nr_struct_class(void) __attribute__((weak));\n        if (pico8_nr_struct_class != NULL && cls == pico8_nr_struct_class()) {\n            return NULL;\n        }\n    }\n    while (cls != NULL && cls != &g_qh_struct_class) {"
   ]
  ],
  "c": [
   [
    "src/pico8_planet_family_trade.c",
    "g_nr_struct_class = {",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),"
   ]
  ]
 },
 "QH-fr": {
  "p": [
   [
    "src/pico8_planet_family_trade.c",
    null,
    "    .parent = pico8_nr_struct_class,\n",
    ""
   ]
  ],
  "c": [
   [
    "src/pico8_planet_family_trade.c",
    "g_fr_struct_class = {",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),"
   ]
  ]
 },
 "QH-pm": {
  "p": [
   [
    "src/pico8_planet_family.c",
    "g_pm_struct_class = {",
    "    .parent = pico8_hj_struct_class,\n",
    ""
   ]
  ],
  "c": [
   [
    "src/pico8_planet_family.c",
    "g_pm_struct_class = {",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),"
   ]
  ]
 },
 "QH-qh": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "    while (cls != NULL && cls != &g_qh_struct_class) {",
    "    if (cls == &g_qh_struct_class) {\n        return NULL;\n    }\n    while (cls != NULL && cls != &g_qh_struct_class) {"
   ]
  ],
  "c": [
   [
    "src/pico8_planet_family.c",
    "g_qh_struct_class = {",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),"
   ]
  ]
 },
 "NL-nl": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "    return cls != NULL ? (const pico8_nl_t *)e : NULL;",
    "    return cls != NULL && 0 ? (const pico8_nl_t *)e : NULL;"
   ]
  ],
  "c": [
   [
    "src/pico8_planet_family.c",
    "g_nl_struct_class = {",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH) | PICO8_LINEAGE(PICO8_LINEAGE_NL),",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),"
   ]
  ]
 },
 "N-jo-QH": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "    while (cls != NULL && cls != &g_qh_struct_class) {",
    "    if (cls == pico8_jo_struct_class()) {\n        return (const pico8_qh_t *)e;\n    }\n    while (cls != NULL && cls != &g_qh_struct_class) {"
   ]
  ],
  "c": [
   [
    "src/pico8_planet.c",
    "g_jo_struct_class = {",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),"
   ]
  ]
 },
 "N-ga-QH": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "    while (cls != NULL && cls != &g_qh_struct_class) {",
    "    if (cls == &g_ga_struct_class) {\n        return (const pico8_qh_t *)e;\n    }\n    while (cls != NULL && cls != &g_qh_struct_class) {"
   ]
  ],
  "c": [
   [
    "src/pico8_planet_family.c",
    "g_ga_struct_class = {",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),"
   ]
  ]
 },
 "N-hj-NL": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "    while (cls != NULL && cls != &g_nl_struct_class) {",
    "    if (cls == &g_hj_struct_class) {\n        return (const pico8_nl_t *)e;\n    }\n    while (cls != NULL && cls != &g_nl_struct_class) {"
   ]
  ],
  "c": [
   [
    "src/pico8_planet_family.c",
    "g_hj_struct_class = {",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH) | PICO8_LINEAGE(PICO8_LINEAGE_NL),"
   ]
  ]
 },
 "JO-ga": {
  "p": [
   [
    "src/pico8_planet_family.c",
    "g_ga_struct_class = {",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),",
    "    .lineage = 0,"
   ]
  ],
  "c": [
   [
    "src/pico8_planet_family.c",
    "g_ga_struct_class = {",
    "    .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),",
    "    .lineage = 0,"
   ]
  ]
 },
 "PR-qh": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "    self->jo.header.probe = &self->probe_own;\n",
    ""
   ]
  ],
  "c": [
   [
    "src/pico8_planet_family.c",
    null,
    "    self->jo.header.probe = &self->jo.probe_own;\n",
    ""
   ]
  ]
 },
 "PR-nl": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "    pico8_qh_struct_init(&self->qh, &g_nl_struct_class, &pico8_nl_def);\n",
    "    pico8_qh_struct_init(&self->qh, &g_nl_struct_class, &pico8_nl_def);\n    self->qh.jo.header.probe = NULL;\n"
   ]
  ],
  "c": [
   [
    "src/pico8_planet_family.c",
    null,
    "    pico8_qh_struct_init(&self->qh, &g_nl_struct_class, &pico8_nl_def);\n",
    "    pico8_qh_struct_init(&self->qh, &g_nl_struct_class, &pico8_nl_def);\n    self->qh.jo.header.probe = NULL;\n"
   ]
  ]
 },
 "EM-hj": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "    self->qh.jo.header.emitter = &self->emitter_own;\n",
    ""
   ]
  ],
  "c": [
   [
    "src/pico8_planet_family.c",
    null,
    "    self->qh.jo.header.emitter = &self->qh.jo.emitter_own;\n",
    ""
   ]
  ]
 },
 "EM-ga": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "    self->jo.header.emitter = &self->emitter_own;\n",
    ""
   ]
  ],
  "c": [
   [
    "src/pico8_planet_family.c",
    null,
    "    self->jo.header.emitter = &self->jo.emitter_own;\n",
    ""
   ]
  ]
 },
 "N-jo-probe": {
  "p": [
   [
    "src/pico8_planet.c",
    null,
    "    self->kw_own = (pico8_kw_t){0};\n",
    "    self->kw_own = (pico8_kw_t){0};\n    {\n        static pico8_probe_t stray;\n        stray = (pico8_probe_t){.def = &pico8_probe_def_none};\n        self->header.probe = &stray;\n    }\n"
   ]
  ],
  "c": [
   [
    "src/pico8_planet.c",
    null,
    "    self->emitter = NULL;\n",
    "    self->emitter = NULL;\n    self->probe_own = (pico8_probe_t){.def = &pico8_probe_def_none};\n    self->header.probe = &self->probe_own;\n"
   ]
  ]
 },
 "K-init-c": {
  "c": [
   [
    "src/pico8_planet.c",
    null,
    "    self->probe = NULL;\n    self->emitter = NULL;\n",
    ""
   ]
  ]
 },
 "K-membre-jo": {
  "c": [
   [
    "src/pico8_planet.c",
    null,
    "    self->probe = NULL;\n",
    "    self->probe = &self->probe_own;\n"
   ]
  ]
 },
 "K-membre-jo-em": {
  "c": [
   [
    "src/pico8_planet.c",
    null,
    "    self->emitter = NULL;\n",
    "    self->emitter = &self->emitter_own;\n"
   ]
  ]
 }
}
```
