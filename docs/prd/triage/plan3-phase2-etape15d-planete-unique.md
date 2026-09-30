---
title: "Plan 3, étape 2.15d : les neuf structs de sorte disparaissent, toutes les planètes sont des `pico8_jo_t` ; `parente` 9 → 0, gardé — un commit produit par un script de réécriture guidé par le compilateur, jeu identique, objets identiques"
description: "`pico8_qh_t`, `pico8_nl_t`, `pico8_os_t`, `pico8_hj_t`, `pico8_pm_t`, `pico8_ga_t`, `pico8_nr_t`, `pico8_fr_t`, `pico8_mx_t` sont retirés de `planet_family.h` et `planet_family_trade.h` ; chaque nom de type devient `pico8_jo_t` (205 occurrences), chaque chemin enchâssé (`self->hj.qh.jo.emitter`, `&self->qh`) est raccourci d'après les 726 erreurs du compilateur, les 11 casts devenus identité (`pico8_jo_pq((pico8_jo_t *)self)`) sont retirés, `planet_display.c:18` part, `parente` entre dans `COMPOSITION_RESIDUE_GUARD`. Le script (`run215d.sh`, 9 s) rend l'arbre à l'identique (hachages d'arbre donnés). Essayé sur copie : vert, 26 e2e (aussi sous perturbation), 74 objets de `src/` identiques octet pour octet sous `-O1`/`-O2 -DNDEBUG`, 13 jumelles sans ÉCHEC, les 19 anciens casts de sorte contrôlés à l'exécution sans confusion, valeurs observables identiques, couverture identique ligne pour ligne. Aucune question à l'humain."
status: needs-triage
---

## Problem Statement

**Écrit sur `03a4bef`** (branche `claude/plan-pilot-3-7zdft0`), départ réel **`b35ca48`**. Au début de l'écriture, 2.15c n'était pas commitée : l'arbre des essais est `03a4bef` + le diff du PRD 2.15c (`docs/prd/done/plan3-phase2-etape15c-etat-planete.md`, lignes 109-1968), appliqué tel quel. Pendant l'écriture, 2.15c a été commitée (`4124a2c`) puis vérifiée (`b35ca48`) : `git rev-parse 4124a2c:src 4124a2c:tests 4124a2c:Makefile 4124a2c:tools` rend exactement les arbres de la copie (`3dc0b58f…`, `e839bb08…`, `30351cca…`, `e6db806c…`), donc tout ce qui suit vaut pour `b35ca48`. Notation : `p` = le commit de la copie qui porte 2.15c (arbres ci-dessus, identiques à `b35ca48`) ; `d` = le commit de la copie qui porte 2.15d. Numéros de ligne : ceux de `p`, sauf mention. Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier ; aucun `make` lancé dans le dépôt. Essais sur une seule copie jetable (scratchpad `prd215d/`, clone, deux arbres de travail `wt_p`, `wt_d`), supprimée depuis.

Vu du mainteneur du port : après 2.15c, les neuf structs de sorte ne portent plus que leur ascendante (`pico8_qh_t {pico8_jo_t jo;}`, `pico8_pm_t {pico8_hj_t hj;}`…) et font toutes 424 octets. Elles ne servent plus qu'à allonger les chemins (`self->hj.qh.jo.emitter`) et à imposer des casts entre sortes, alors que la décision A de l'humain veut « un seul type planète pour les dix sortes, la sorte n'étant plus que sa table de méthodes et son archétype ». 2.15d retire les neuf types d'un coup (un changement de type ne se coupe pas module par module sans forme transitoire), ce qui met `parente` à 0 et fait tomber la paire d'inclusion `planet_display` ↔ `planet_family`.

### Texte du plan, recopié (`docs/plan-3.md` à `b35ca48`)

> - 2.15 — `parente` et `lien-parent`, selon l'étude 2.10 (§2). […] Preuves : chemin touché (mutations : lien `probe` NULL sur une sorte de la lignée `qh`, bit `QH` ôté de `nr`, bit `JO` ôté de `ga` → rouge) ; TU d'équivalence réécrit sur le masque ; 2.15d, objets sans `-g` identiques pour les modules dont seul un chemin change, sinon chemin touché. `class-coverage-net` : oui (comptes égaux à `tools/class_coverage_net_expected.txt`). `sanitize-net` : oui. Budget ≈ 4,5 × 50 min.
>   - 2.15d — les neuf structs de sorte disparaissent (`planet_family.h:32`, `:51`, `:62`, `:72`, `:88`, `:102`, `planet_family_trade.h:23`, `:34`, `:45`) : chemins enchâssés et casts réécrits, ≈ 37 lignes de `src/` et ≈ 165 de TU estimées après 2.13. Changement de type indivisible : budget ≈ 1,5 × 50 min, déclaré. Compte : `parente` → **0**, gardé. (2.11 : retire aussi `planet_display.c:18`, `#include "pico8_planet_family.h"`, dont plus aucun nom n'est employé après 2.15c/2.15d ; fait tomber `planet_display` ↔ `planet_family` : 16 / 40 → **15 / 38**, usage 19 → 18. `planet` → `planet_family` ne garde que le registre `kv`, coupé à 2.17e.)

Numéros du plan datés : à `p`, les neuf corps sont à `planet_family.h:28`, `:34`, `:40`, `:47`, `:53`, `:63` et `planet_family_trade.h:19`, `:25`, `:32` (les lignes que `--check parente` imprime : `:29`, `:35`, `:41`, `:48`, `:54`, `:64`, `trade.h:20`, `:26`, `:33`).

Décisions de l'humain (même fichier) : **planètes, A** (2026-09-30) — « un seul type "planète" pour les dix sortes, qui porte tous leurs champs, la sorte n'étant plus que sa table de méthodes et son archétype » : 2.15c a monté tout l'état dans `pico8_jo_t`, 2.15d retire les types de sorte ; aucun alias de type ne les remplace. **Q-2.10, oui** et **Q-2.14d, oui** : sans objet ici (aucun lien, aucun `has_X` ne bouge). Notes « Découvert à 2.10 » (étude 2.10, §2, « Ce que l'aplatissement trouve ») : « ≈ 37 dans `src/` et ≈ 165 dans les TU pour 2.15d ; 2.3b/2.3c ont réécrit de tels volumes en ≈ 40 min par passe guidée par le compilateur » ; « 2.15d est un changement de type : objets sans `-g` identiques pour les modules dont seul un chemin change (`objdump -d`, `nm`), sinon chemin touché ». Notes « Découvert à 2.11 » : « deux tombent d'elles-mêmes : […] `planet_display` ↔ `planet_family` à 2.15d (chacune avec le retrait de son include) » ; étude 2.11, fiche 6 (`planet` → `planet_family` : `pico8_qh_t` part à 2.15d, `pico8_pm_kind`, `pico8_ga_kind` restent) et fiche 10 (`planet_display` → `planet_family` : « Technique : T5, à 2.15d, à condition que 2.15d retire aussi `planet_display.c:18` dans le même commit »). Note « Découvert à 2.15c » : `inclusion-mutuelle` 16 / 15 (**41** lignes), graphe d'usage **18**, pied **53 / 27**, `--champs` **238 ; 14 ; 19**, casts / tests de classe **480 / 647**, planètes **424** octets.

Règles communes du plan : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée ; `class-coverage-net` et `sanitize-net` : oui, oui (ligne 2.15). Ce PRD les tient toutes.

### Contrats reçus et ce que ce PRD en fait

| Point (source) | Ce que fait 2.15d |
|---|---|
| « structs de sorte réduites […] toutes à 424 octets ; seuls des chemins (`.qh.jo.`, `.hj.qh.jo.`, `.nr.qh.jo.`) et des casts vers une sorte restent ; la réécriture de 2.15c (236 sites) donne l'ordre de grandeur, à guider par le compilateur » (PRD 2.15c, contrat) | tenu : 205 noms de type renommés, **726** chemins raccourcis d'après 726 erreurs du compilateur, en une passe ; aucun champ ne bouge, `sizeof(pico8_jo_t)` reste 424 |
| « `inclusion-mutuelle` : 16 / 41 → **15 / 39** (et non 16 / 40 → 15 / 38) ; graphe d'usage : reste **18** » (idem) | vérifié : **15 paires / 15 modules, 39 lignes** (sortent `planet_display.c:18` et `planet_family.c:17`) ; graphe d'usage **18**, sortie de `usegraph.py` identique octet pour octet à `p` |
| « `planet.h:11` inclut `pico8_probe.h` […] 2.17d doit la faire viser l'en-tête neuf » (idem) | non touché ; la ligne compte toujours dans les 39 |
| « `pico8_jo_struct_init` pose `probe`, `emitter` à NULL ; les liens du noyau restent posés par les porteurs seuls » (idem) | inchangé : mêmes poses, chemins raccourcis (`self->header.probe = &self->probe_own`) |
| « `pico8_qh_struct_class`, `pico8_hj_struct_class`, `pico8_nr_struct_class` ne sont plus appelés par le jeu ; ne pas les retirer » (idem) | non touchés |
| « lecture d'une sorte : bit de `lineage`, puis le champ sur la planète ; aucune aide `*_of` » (idem) | inchangé ; aucune aide neuve |
| « `economy_score.c:12`, `economy_tiers.c:10`, `planet_trade.c:11` incluent `planet_family.h` sans usage : 2.17, hygiène » (idem) | non touchés (2.17) |
| Plan : « objets sans `-g` identiques pour les modules dont seul un chemin change, sinon chemin touché » | les **74** objets de `src/` sont identiques (désassemblage, `nm`, `.rodata`, octets) sous `-O1` et `-O2 -DNDEBUG` ; sous `-O0 -DNDEBUG`, 72 identiques et deux (`planet_family`, `planet_family_trade`) ne diffèrent que par l'ordre de trois chargements de registres dans trois fonctions (preuve 4). Chemin touché fait aussi : couverture des 26 e2e identique ligne pour ligne (preuve 7) |
| Plan : « `planet` → `planet_family` ne garde que le registre `kv` » | vérifié : `usegraph.py` rend `planet` → `planet_family` = `pico8_ga_kind pico8_pm_kind` (le registre `kv`), à `p` comme à `d` |

### État de départ attendu (`b35ca48`, ou un descendant qui ne touche que `docs/`)

Mesuré sur `p` (= `b35ca48` pour `src`, `tests`, `Makefile`, `tools`) ; identique à la note « Découvert à 2.15c » :

| Commande | Résultat |
|---|---|
| `make composition-residue` | `parente 9 2`, `void-ptr 3 3`, `inclusion-mutuelle 16 15` ; **`résidu 2 : 53 ligne(s), 27 fichier(s)`** |
| `sh tools/composition_residue.sh --check parente src tests` | 9 lignes : `planet_family.h:29`, `:35`, `:41`, `:48`, `:54`, `:64`, `planet_family_trade.h:20`, `:26`, `:33` |
| `sh tools/composition_residue.sh --champs \| tail -1` | `champs : 238 ; 0 écriture : 14 ; 0 hors constructeur : 19` |
| `--check inclusion-mutuelle src tests \| tail -1` | `résidu gardé (inclusion-mutuelle) : 41 ligne(s)` |
| `COMPOSITION_RESIDUE_GUARD` (`Makefile:874`) | `…,défauts,accesseurs-q6,lien-parent` (10 catégories) |
| casts / tests de classe : `grep -rEo '\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)' src tests \| wc -l` ; `grep -rEo '(->cls\|header\.cls\|_struct_class\(\))' src tests \| wc -l` | 480 / 647 |
| `sizeof` (x86-64) : noyau, `g_`, `jo`, les neuf sortes ; `pico8_probe_t`, `pico8_emitter_t` | 128, 256, 424, 424 chacune ; 24, 56 |
| graphe d'usage (`usegraph.py`, annexe A du PRD 2.11) | `paires 18` |
| `make test` ; `make e2e` ; `MALLOC_PERTURB_=165 make e2e` | vert ; 26 conformes ; 26 conformes |

Contrôle rapide du départ : `git rev-parse HEAD:src HEAD:tests` → `3dc0b58f8e00e430c2860a0c2d25aac80ee176fa`, `e839bb08c09ea091e486fd58567a6f7cfd6ae86f`. Autre départ : le script tourne quand même (il est guidé par le compilateur), mais les hachages d'arrivée ne valent plus ; comparer alors les comptes et rapporter l'écart.

## Solution

Un commit, vert, jeu identique (26 e2e conformes, aussi sous `MALLOC_PERTURB_=165`), aucune constante d'empreinte changée. `parente` 9 → **0**, gardé. Le commit est le résultat de `run215d.sh` (annexe A, texte exact), plus la note du plan et le message :

1. **`rw.py`** (texte) : retire les neuf blocs `commentaire + typedef struct { pico8_X_t x; } pico8_Y_t;` ; le commentaire de chaque sorte, sans sa phrase « Nothing frees the struct: TROU-7. » (dite une fois sur `pico8_jo_t`) et avec `jo.by` → `by`, remonte au-dessus de son `int pico8_Y_ready(void);` ; renomme les neuf noms de type en `pico8_jo_t` dans `src/` et `tests/` (**205** occurrences : prototypes, `_new`, `_struct_init`, locales, aides de TU) ; réécrit le commentaire de `pico8_jo_t` (« A planet of any of the ten kinds, which differ only by class table and archetype… » à la place de « A bare planet […] Every planet struct begins with one ») ; retire `planet_display.c:18`.
2. **`fix.py`**, relancé jusqu'à 0 édition sur les erreurs de `syn.sh` : pour chaque `'pico8_jo_t' has no member named 'jo'|'qh'|'hj'|'nr'`, retire la chaîne enchâssée : `X->hj.qh.jo.m` → `X->m` ; `&X->qh` → `X` (`pico8_qh_struct_init(&self->qh, …)` → `(self, …)`, `&as_hj(placed)->qh` → `as_hj(placed)`) ; `&v.qh` → `&v`. Mesuré : **726** erreurs, **726** éditions, une passe ; la seconde rend 0 erreur.
3. **`idcast.py` + `uncast.py`** : sur une copie, chaque cast `(pico8_jo_t *)`/`(const pico8_jo_t *)` (81) est enveloppé d'un contrôle `__builtin_types_compatible_p` qui n'échoue à la compilation que si l'opérande est déjà un `pico8_jo_t *` : **11** casts devenus identité, tous `(pico8_jo_t *)self` dans `planet_family.c:77`, `:138`, `:180`, `:192`, `:255`, `:329`, `:495` et `planet_family_trade.c:209`, `:252`, `:299`, `:346` (les appels `pico8_jo_pq`, `pico8_jo_kj`, `pico8_jo_kf` qui remontaient d'une sorte vers `jo`). Ils sont retirés. Les 70 autres partent d'un `void *` ou d'un en-tête et restent.
4. **`Makefile`** : `parente` ajouté en fin de `COMPOSITION_RESIDUE_GUARD` (`:874`). La catégorie reste dans `CATEGORIES`/`SORTIE` du script (à 0).

Rien d'autre : ni `docs/glossaire-noms.md`, ni `docs/trous-de-portage.md` (aucun des deux ne nomme un type de sorte ni un chemin enchâssé), ni `tests/e2e/`, ni `*.sha256`, ni `tools/`, ni une autre ligne `#include`, ni les fonctions (`pico8_qh_new`, `pico8_nr_gv`… gardent leur nom et rendent ou prennent un `pico8_jo_t *`), ni les tables de classe, ni les masques `lineage`, ni `pico8_ga_keys_t` (son membre `jo` est un `pico8_jo_keys_t` : les 9 lignes `keys->jo.p`… qui restent au grep `.jo.` sont les clés).

## User Stories

1. En tant que mainteneur, je veux qu'une planète soit un `pico8_jo_t` quelle que soit sa sorte, afin de lire `self->emitter` et non `self->hj.qh.jo.emitter`.
2. En tant que mainteneur, je veux que la sorte ne soit plus que la table de classe et l'archétype (décision A), afin qu'aucun cast ne passe d'une sorte à une autre.
3. En tant que gardien du compteur, je veux `parente` à 0 et gardé, afin qu'une struct qui en enchâsse une autre rougisse `make test`.
4. En tant qu'auteur de 2.17, je veux `planet_display` ↔ `planet_family` hors des paires, afin de partir de 15 / 39.
5. En tant que gardien de la règle du parent, je veux que chaque mutation rouge au parent le soit à l'enfant, et une preuve que les casts de sorte retirés ne cachaient aucune confusion.

## Implementation Decisions

### Modules

`planet.h` (commentaire de `pico8_jo_t`) ; `planet_family.h`/`.c` ; `planet_family_trade.h`/`.c` ; `planet_display.c` (une ligne `#include`) ; `Makefile` (une ligne) ; TU `fr`, `ga`, `ga_export_panel_by`, `hj`, `hy`, `jc`, `jo_spec_defaults`, `lq`, `mx`, `mz`, `nl`, `nr`, `os`, `pm`, `qh`, `struct_header`, `trade`, `ui` ; `docs/plan-3.md` (note « Découvert à 2.15d »).

### Inclusions

Une ligne retirée dans `src/` : `planet_display.c:18`, `#include "pico8_planet_family.h"` (essai 2.15c et ici : `planet_display.c` compile sans elle ; plus aucun nom de `planet_family.h` n'y est employé depuis 2.15c). **16 → 15 paires, 15 modules**, `--check` **41 → 39** lignes : `planet_display.c:18` et `planet_family.c:17` (`#include "pico8_planet_display.h"`, qui ne comptait qu'au titre de cette paire) sortent. `planet_family.c:17` reste dans le code (`pico8_hj_kj` appelle `pico8_jo_kj`) ; sens coupé : `planet_display` → `planet_family`, comme la fiche 10. Graphe d'usage **18 → 18**, sortie identique octet pour octet (la paire d'usage était déjà sortie à 2.15c). Aucune paire n'entre ; aucune autre ligne `#include` ne bouge.

### Ce qui change

Le texte exact est le résultat du script : `sh outils/run215d.sh <arbre>` sur un arbre propre à `b35ca48` rend, mesuré, `git rev-parse` des arbres : **`src` `f520dbf0021c6e78e35a234db53acd4dd33cda41`**, **`tests` `b15bb8ecf8054c20edf4db61bad706b53fd26ada`**, **`Makefile` `e9226b4f1b5a9648628f9189757f6e652b717347`** (après `git add -A src tests Makefile`) ; `git diff p d | sha256sum` → `b445f33c…` (4036 lignes), `git diff p d -- tests | sha256sum` → `d091c5e8…` (2876 lignes). Rejoué deux fois de `p` sur un arbre de travail neuf : diff vide avec `d` (Runs 22). Le diff de `src/` et du `Makefile` (1160 lignes) est en annexe B, texte exact ; celui des TU n'est pas recopié (mécanique : noms de type et chemins, rien d'autre).

Lecture :

- **Un type, dix tables** : `pico8_qh_new` rend un `pico8_jo_t *` comme `pico8_jo_new` ; ce qui distingue un laboratoire d'une planète nue est `header.cls` (table, masque `lineage`) et `def` (archétype). Les dispatchers `static void pico8_nl_ly_dispatch(void *self, …)` castent vers `pico8_jo_t *` au lieu de `pico8_nl_t *`.
- **Casts** : les 19 casts vers une struct de sorte (9 dans `src/` : dispatchers de `nr`, `mx`, `nl`, `hj`, `ga` et `pico8_ga_mm` ; 10 dans les TU, gardés par un test de classe) deviennent des casts vers `pico8_jo_t` depuis `void *` ou un en-tête ; ils restent (le type source n'est pas une planète). Les 11 upcasts `(pico8_jo_t *)self` sont devenus des identités et sont retirés : **casts 480 → 469**. Aucun cast neuf. Tests de classe : 647, inchangés.
- **Commentaires** (règle de `CLAUDE.md`) : celui de chaque sorte (contrat : ce que la sorte copie de son archétype, qui écrit quoi) reste, au-dessus de son `_ready`, sans la phrase TROU-7 (sur `pico8_jo_t`) ; celui de `pico8_jo_t` dit « any of the ten kinds, which differ only by class table and archetype » et perd « Every planet struct begins with one, so jo's functions take it for `self` », devenu faux.
- **Alias de TU gardés** : dix lignes de TU deviennent `pico8_jo_t *X = Y;` sur un `Y` du même type (`nr = fr; qh = nr; jo = qh;`, `fr_jo = fr`). Les simplifier changerait des aides d'empreinte de TU ; hors périmètre (G4).
- **Longueurs** : les chemins raccourcis ne font qu'enlever des caractères ; aucune ligne neuve ne dépasse 120 colonnes. La ligne `self->qw =` de `nl:ly`, coupée par 2.15c, reste coupée (G5).

### Comptes, mesurés sur la copie

| Mesure | `p` | `d` |
|---|---|---|
| `parente` (lignes / fichiers) | 9 / 2 | **0 / 0**, gardé |
| `lien-parent` | 0, gardé | 0, gardé |
| `void-ptr` | 3 / 3 | 3 / 3 |
| `accesseurs-q6`, `défauts` et les sept autres gardées | 0, gardées | 0, gardées |
| `inclusion-mutuelle` (paires / modules, lignes du `--check`) | 16 / 15, 41 | **15 / 15, 39** |
| graphe d'usage (`usegraph.py`) | 18 | 18 (sortie identique) |
| pied `résidu 2` | 53 / 27 | **42 / 26** |
| `--champs` | 238 ; 14 ; 19 | 238 ; 14 ; 19 |
| casts (grep de la réserve 4 de 2.13a) | 480 | **469** |
| tests de classe (`->cls`, `header.cls`, `_struct_class()`) | 647 | 647 |
| `sizeof(pico8_struct_header_t)`, `g_`, `pico8_jo_t` | 128, 256, 424 | 128, 256, 424 |
| types planète distincts | 10 (`jo` + 9 sortes, 424 octets chacun) | **1** (`pico8_jo_t`) |
| `COMPOSITION_RESIDUE_GUARD` | 10 catégories | **11** (+ `parente`) |
| constantes `*_HASH`/`FROZEN_*` changées | — | 0 |
| lignes du diff : `src/*.c` ; `src/*.h` ; TU | — | + 172 / − 173 ; + 55 / − 91 ; + 625 / − 625 |

**Pied** : 53 − 9 (`parente`) − 2 (`inclusion-mutuelle`) = **42** lignes ; fichiers : sort `planet_family_trade.h` (il n'y comptait que par `parente`) : 27 − 1 = **26**. `planet_family.h` reste (`:8`, `#include "pico8_planet.h"`, paire `planet` ↔ `planet_family`), `planet_display.c` reste (`:17`), `planet_family.c` reste (`:16`).

**`--champs`** inchangé : le relevé ne lit pas un premier membre qui enchâsse un type d'entité (« ni parente enchâssée ») ; les neuf structs de sorte n'avaient pas d'autre membre.

**Estimation du plan** (≈ 37 lignes de `src/`, ≈ 165 de TU) : dépassée (173 et 625), parce qu'elle datait d'avant 2.15c, qui a ajouté 236 chemins transitoires (`self->qh.jo.s`) que 2.15d retire. Sans effet sur le budget : le script fait la réécriture en 9 s.

### Contrat pour 2.16, 2.17 et la suite (à recopier dans leurs PRD)

| Point | Pour | Ce qu'il faut savoir |
|---|---|---|
| plus aucun type de sorte ; `pico8_ga_mm(void *self_ptr)` caste vers `const pico8_jo_t *` ; les dispatchers de sorte castent `void *` → `pico8_jo_t *` | 2.16 | typer `mm` et `hd` en `pico8_struct_header_t *self` : les cibles de `planet_family(_trade)` ne connaissent plus que `pico8_jo_t` |
| `inclusion-mutuelle` **15 / 15, 39 lignes** au départ de 2.17 ; `planet_display` ↔ `planet_family` n'est plus une paire | 2.17 | la liste de 2.11 perd la fiche 10 ; `planet.h:11` (`probe.h`) compte toujours |
| numéros de ligne : `planet_display.c` après `:18` reculent de 1 ; `planet_family.h` et `planet_family_trade.h` renumérotés (les neuf structs retirées, commentaires de sorte au-dessus des `_ready` : `planet_family.h:31`, `:48`, `:59`, `:70`, `:95`, `:111`, `trade.h:22`, `:33`, `:44`) ; `planet.c`, `planet.h`, `planet_family.c`, `planet_family_trade.c` gardent leurs numéros | 2.16, 2.17 | les PRD qui citent des lignes de `planet_display.c` datées d'avant 2.15d les décalent |
| `planet` → `planet_family` : `pico8_ga_kind`, `pico8_pm_kind` seulement (registre `kv`) | 2.17e | inchangé depuis 2.15c ; `planet.c:19` se coupe à 2.17e comme prévu |
| aides de TU `as_qh`, `as_hj`, `as_ga`, `as_nl` et dix alias `pico8_jo_t *X = Y;` identiques au type près | toutes | ne pas les confondre avec un reste de parenté ; les simplifier est hors plan (toucherait des empreintes de TU) |

### Anti-tassement

**Interdit** :
- remplacer une struct de sorte par un alias (`typedef pico8_jo_t pico8_qh_t;`, `#define pico8_qh_t pico8_jo_t`) : `parente` ne le compterait pas (ce n'est pas un corps de struct), mais la décision A serait contournée ; une struct anonyme ou une union qui garde les chemins ;
- garder un cast identité `(pico8_jo_t *)` sur un `pico8_jo_t *`, ou en ajouter un ;
- renommer une fonction (`pico8_qh_new`, `pico8_nr_gv`…), toucher une table de classe, un masque `lineage`, un ordre d'initialisation, `pico8_ga_keys_t` ;
- réécrire à la main ce que le script fait, sauf si le script s'arrête (repli) ;
- ajouter ou retirer une ligne `#include` autre que `planet_display.c:18` ;
- changer un `*.sha256`, `tests/e2e/`, `tools/`, une constante `*_HASH`/`FROZEN_*`, `docs/glossaire-noms.md`, `docs/trous-de-portage.md` ; toucher le `Makefile` ailleurs qu'à `COMPOSITION_RESIDUE_GUARD`.

## Implementation Chunks

Un commit, **≈ 50 min** preuves comprises, sous le budget déclaré de 1,5 × 50 min (mesuré sur la copie : script 9 s ; preuves ≈ 30 min de machine, en grande partie en arrière-plan) : ≈ 2 min pour copier les scripts de l'annexe A hors du dépôt, lancer `run215d.sh`, comparer les hachages d'arbre, `make test`, `make e2e`, `MALLOC_PERTURB_=165 make e2e` ; ≈ 1 min comptes (`base.sh`) ; objets (`objs.sh` ×6, `perm.py`, ≈ 2 min) ; jumelles (13 × 2, en parallèle sur deux arbres, ≈ 8 min) ; `kck.py` au parent (≈ 2 min) ; `lin.c`, `obs.c` (< 1 min) ; `line-coverage` des 26 e2e aux deux bouts, `class-coverage-net`, `sanitize-net` (≈ 3 min) ; garde, `neg.c`, greps ; note « Découvert à 2.15d », message.

**Pas de découpe** : un type ne peut pas disparaître module par module sans forme transitoire (alias, ou deux types pour la même planète). Le budget est tenu par le script, comme 2.3b et 2.15c (réécriture guidée par le compilateur).

**Replis écrits**, dans l'ordre :
1. si `run215d.sh` s'arrête (« arret : 8 passes » ou « erreurs restantes ») ou imprime des lignes `MANUEL` : réécrire ces lignes à la main **selon la même règle** (chaîne enchâssée retirée ; `&X->qh` → `X`), relancer le script depuis la passe `syn`/`fix`, noter chaque ligne dans le message ;
2. si le départ n'est pas `b35ca48` (arbres différents) : lancer le script quand même, ne pas comparer les hachages, comparer tous les comptes de la table et rapporter l'écart dans la note ;
3. si le commit déborde de 1,5 × 50 min avant d'être fait : aucun second commit n'est possible (type indivisible) ; arrêter, garder l'arbre, rapporter ce qui manque dans la note « Découvert à ».

## Testing Decisions

- **Pas de TU neuf, aucun TU réécrit à la main** : les TU ne changent que par le script (noms de type, chemins). Aucun rejeu d'`events.json` dans un TU. Aucun `*_HASH` ne bouge : les empreintes de TU replient les mêmes valeurs lues par des chemins plus courts.
- **Bon test ici** : un changement de type pur se prouve par l'identité des objets compilés (le code machine ne dépend pas du nom du type ni de la longueur du chemin quand tout est à l'offset 0) ; ce qui reste de sémantique (les casts retirés) se prouve à l'exécution au parent : chaque cast de sorte, contrôlé, ne voit que sa sorte ou une descendante.
- **Prior art** : PRD 2.15a–b et 2.15c (jumelles, `lin.c`, `obs.c`, `lset.py`, `fix.py`) ; étude 2.11, annexe A (`syn.sh`, `objs.sh`) ; règle du parent (D.21) ; ensemble `L` (2.4 à 2.8).

### Preuves (parent de `d` = `p`)

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `MALLOC_PERTURB_=165 make e2e` **26 conformes**, aux deux bouts ; `git diff --stat p -- tests/e2e tools '*.sha256'` vide ; `git diff p | grep -E '^[-+].*(_HASH|FROZEN_)'` vide (Runs 2, 8, 13).

2. **Reproductibilité** : `run215d.sh` sur un arbre de travail neuf à `p` → `passe 1 : 726 erreur(s) ; edits 726`, `passe 2 : 0 erreur(s) ; edits 0`, `casts enveloppes 81`, `casts identite : 11`, `casts retires 11`, `erreurs finales : 0` ; `git diff d -- src tests Makefile` vide ; arbres `f520dbf0…`, `b15bb8ec…`, `e9226b4f…` (Runs 22).

3. **Preuve négative** (`neg.c`, annexe A, à `d`) : `pico8_qh_t *x;` → `unknown type name 'pico8_qh_t'` ; `e->jo.s`, `e->hj.qh.jo.emitter`, `&e->nr` → `'pico8_jo_t' has no member named 'jo'`, `'hj'`, `'nr'` ; les fonctions de sorte appelées avec un `pico8_jo_t *` (`pico8_nr_hb`, `pico8_mx_kf`, `pico8_ga_pq`, `pico8_nl_ly(pico8_nl_new(0), 0)`) compilent sans cast (Runs 20).

4. **Objets sans `-g`** (`objs.sh`, annexe A : chaque `src/*.c` hors raylib, 74 objets, `objdump -d`, `nm`, `objdump -s -j .rodata`, octets) :
   - `-O1` et `-O2`, `-DNDEBUG -DPICO8_CLASS_COVERAGE` : **74 / 74 identiques**, `planet_family.o` et `planet_family_trade.o` compris ;
   - `-O0 -DNDEBUG` (avec ou sans `-DPICO8_CLASS_COVERAGE`) : 72 identiques ; `planet_family.o`, `planet_family_trade.o` diffèrent par **trois blocs de trois instructions**, chacun une permutation des mêmes `mov -0x8/-0x10/-0x18(%rbp)` (`perm.py`) : l'ordre de chargement des arguments de `pico8_jo_struct_init(&self->jo, cls, def)` dans `pico8_qh_struct_init`, de `pico8_qh_struct_init(&self->qh, …)` dans `pico8_hj_struct_init` et `pico8_nr_struct_init` (à `-O0`, `&self->jo` et `self` ne sont pas évalués dans le même ordre ; même valeur, même appel) ;
   - `-O0` avec les `assert` : en plus, `planet_display.o` diffère par sept immédiats de `__LINE__` (−1, include retiré) et `planet_family.o` par le texte d'un `assert` (`self->qh.jo.has_mi` → `self->has_mi`) et les adresses `__PRETTY_FUNCTION__` qui suivent.
   Lecture : **aucun module ne change de code** ; la réécriture est un pur renommage de chemins et de types (Runs 11, 12).

5. **Mutations, règle du parent** (`mut.py` + `muts.json`, annexe A). Même mutation au parent `p` (chemin enchâssé) et à l'enfant `d` (chemin court), sur les sites que la réécriture touche : poses de liens, champs de sorte, masques, et les trois appels dont le cast identité est retiré. `make -k -j4 unit` puis `make -k e2e` ; restaurer par `git checkout -f`. **ÉCHEC si un TU rouge au parent n'est pas rouge à l'enfant, ou si l'e2e est < 26 au parent et 26 à l'enfant.**

| # | Cible | Mutation (des deux côtés) | Rouges au parent | Rouges à l'enfant | e2e p / d | Verdict |
|---|---|---|---|---|---|---|
| PR-qh | lien `probe`, lignée `qh` (plan) | pose de `header.probe` retirée de `pico8_qh_struct_init` | `fr`, `hj`, `jo`, `mx`, `nl`, `nr`, `os`, `qh`, `struct_header`, `ui` | les mêmes 10 | 0 / 0 | conforme |
| EM-hj | lien `emitter`, `hj`/`pm`/`mx` | pose retirée de `pico8_hj_struct_init` | `entity_kind`, `ga`, `hj`, `hp`, `ie`, `jo`, `jo_spec_defaults`, `mx`, `pm`, `qh`, `struct_header`, `ui` | les mêmes 12 | 0 / 0 | conforme |
| EM-ga | lien `emitter`, `ga` | pose retirée du `_new` de `ga` | `entity_kind`, `ga`, `ga_export_panel_by`, `hj`, `hp`, `jc`, `jo`, `jo_spec_defaults`, `pm`, `qb`, `qh`, `struct_header`, `ui` | les mêmes 13 | 0 / 0 | conforme |
| QH-nr | bit `QH`, `nr` (plan) | `QH` ôté du masque de `nr` | `fr`, `nr`, `qh` | les mêmes 3 | 24 / 24 | conforme |
| JO-ga | bit `JO`, `ga` (plan) | `JO` ôté du masque de `ga` | `ga`, `hp`, `jo`, `qh` | les mêmes 4 | 26 / 26 | conforme |
| QW-nl | `qw` de `nl` | `_new` de `nl` pose `qw` à 0 au lieu de l'archétype | `jo_spec_defaults`, `nl`, `qh` | les mêmes 3 | 26 / 26 | conforme |
| HT-mx | `ht` de `mx` | `_new` de `mx` pose `ht` à NULL | `jo_spec_defaults`, `mx` | les mêmes 2 | 25 / 25 | conforme |
| S-nr | `s` de `nr` | `nr:hb` n'écrit plus `s` (`(void)s_val;`) | `fr`, `nr` | les mêmes 2 | 26 / 26 | conforme |
| MT-fr | lien `mt`, `fr` | pose de `header.mt` retirée du `_new` de `fr` | `fr`, `jc`, `struct_header` | les mêmes 3 | 25 / 25 | conforme |
| IU-ga | `iu` de `ga` par les clés | la clé `iu` n'est plus appliquée | `ga`, `pm` | les mêmes 2 | 25 / 25 | conforme |
| PQ-mx | cast identité retiré (`planet_family_trade.c:346`) | appel `pico8_jo_pq` retiré du `_new` de `mx` | `jo`, `mx`, `qh` | les mêmes 3 | 0 / 0 | conforme |
| KF-mx | cast identité retiré (`:299`) | appel `pico8_jo_kf` retiré de `mx:kf` | `mx` | `mx` | 25 / 25 | conforme |
| KJ-hj | cast identité retiré (`planet_family.c:192`) | appel `pico8_jo_kj` retiré de `hj:kj` (`(void)p;`) | `ga`, `hj`, `mx`, `pm` | les mêmes 4 | 23 / 23 | conforme |

   Lecture : **aucun ÉCHEC** (13 jumelles, 26 lancements, rouges et e2e identiques des deux côtés). Premier jet : S-nr et KJ-hj laissaient une variable inutilisée (`-Werror`) ; `src/` ne compilait pas, aucun TU n'était lu rouge et l'e2e rendait 0 des deux côtés ; le lecteur de `mut.py` ne reconnaissait que les objets de TU. Corrigé (mutation neutre `(void)x;`, lecteur étendu à `tmp/obj/src/`), relancé (Runs 14). **Lignes K** (enfant seul) : aucune ; l'enfant n'a aucun chemin de code neuf (preuve 4), la garde est prouvée à part (preuve 9).

6. **Les casts retirés ne cachaient aucune confusion de sorte** (`kck.py`, annexe A, **au parent** `p`) : chacun des 19 casts vers une struct de sorte (`(const pico8_nr_t *)e`, `(pico8_hj_t *)self`…) est enveloppé d'un contrôle à l'exécution : la classe de l'objet (`header.cls`) doit être la sorte visée ou une sorte qui en descend (`qh` : les huit de la lignée ; `hj` : `hj`, `pm`, `mx` ; `nr` : `nr`, `fr` ; les autres : elles-mêmes), sinon `abort()` avec le site. `make -k -j4 unit` puis `make -k e2e` : **vert, 26 conformes, 0 confusion** ; **19 / 19 sites atteints** (`planet_family.c:97` ×20, `:197` ×94, `:351` ×7283, `:433` ×215 ; `planet_family_trade.c:43` ×22, `:80` ×8, `:104` ×39, `:158` ×15, `:303` ×5 ; TU `fr:223` ×11, `fr:525` ×1, `ga:213-214` ×188, `mx:275` ×24, `nr:244` ×4, `pm:247-248` ×4, `pm:267` ×4, `qh:512` ×1). Contrôle mordant : le site `:97` réglé sur `mx` au lieu de `nl` → `KCK confusion de sorte: cast vers mx, src/pico8_planet_family.c:97`, `nl` et `qh` rouges (Runs 15). Les 11 upcasts retirés (preuve 4, point 3 de la Solution) allaient d'une sorte vers `jo`, l'ascendante de toutes, au même offset 0 : ils ne pouvaient rien confondre. À `d`, les 19 casts vont vers `pico8_jo_t`, le type de toutes les planètes, et la table de chaque dispatcher n'est posée que dans ses sortes (inchangé).

7. **Chemin touché, ensemble `L`** : `make line-coverage EVENTS="<les 26 e2e>"` à `p` et `d` : **6032** lignes des deux côtés ; les deux listes sont **identiques ligne pour ligne** au décalage près de `planet_display.c` (−1 après `:18`). `lset.py` (PRD 2.15c, annexe A) : `L` (lignes couvertes à `p`, réécrites) **157**, lignes neuves couvertes **157** ; **0 perdue, 0 gagnée**. Les lignes neuves non couvertes sont des commentaires, des prototypes et des déclarations, et les chemins de fonctions que les e2e n'appellent pas non plus à `p` (`pico8_qh_new`, clés `p`/`ra` de `pm`…) (Runs 17).

8. **Valeurs observables et lignée** : `obs.c` et `lin.c` du PRD 2.15c (annexe A, texte exact, compilés `-DV=1` : ils ne nomment que `pico8_jo_t`, ils compilent donc aux deux bouts) : `obs` — dix planètes fraîches (par leur `kind`) et six non-planètes, bits, liens, `s`, `qw`, `iu`, `ms`, `ht`, émetteur, sonde, `oe`, `kw`, `pico8_da`, `pico8_dh`, empreinte mémoire de `jo:kj` et `jo:gh` — **16 lignes identiques** à `p` et `d`, normal et sous `MALLOC_PERTURB_=165` (`9da94ae5…` ×4, la même empreinte qu'à 2.15c) ; `lin` — **40 tables, identique** (`7e4ab61e…`) (Runs 16).

9. **Garde mordante** : à `d`, `typedef struct { pico8_jo_t jo; } pico8_qh_t;` remis dans `planet_family.h` → `make composition-guard` code 2, `parente: src/pico8_planet_family.h:26:    pico8_jo_t jo;`, `résidu gardé (…,parente) : 1 ligne(s)` ; retiré → 0 (Runs 19).

10. `class-coverage-net` : **oui**, `couverture: OK -- chaque classe non exemptée est instanciée (26 rejeu(x))` à `d`. `sanitize-net` : **oui** (neuf types retirés), `sanitize: 26 propre(s)` à `d` (Runs 18).

11. **Graphe d'usage et inclusions** : `usegraph.py` : `paires 18` aux deux bouts, sorties identiques (`diff` vide) ; `--detail` : seule paire sortante `planet_display` ↔ `planet_family`, aucune entrante ; `--check inclusion-mutuelle` : 41 → 39, lignes sortantes `planet_display.c:18`, `planet_family.c:17` (Runs 9).

### Greps de Sortie

À `d` :
1. `grep -rnE '\bpico8_(qh|nl|os|hj|pm|ga|nr|fr|mx)_t\b' src tests docs/glossaire-noms.md docs/trous-de-portage.md | wc -l` → **0** ; `grep -rnE 'typedef\s+pico8_jo_t|#define\s+pico8_\w+_t\b' src tests | wc -l` → **0**.
2. `sh tools/composition_residue.sh --check parente src tests | tail -1` → `résidu gardé (parente) : 0 ligne(s)`.
3. `grep -rnE '(\.|->)(jo|qh|hj|nr)\.' src tests | grep -vE 'keys(->|\.)jo\.' | wc -l` → **0** ; sans le filtre : **9** lignes, toutes `keys->jo.X`/`keys.jo.X` (`pico8_ga_keys_t`).
4. `grep -c 'pico8_planet_family.h' src/pico8_planet_display.c` → **0** ; `git diff p -- src | grep -E '^[-+]#include'` → exactement `-#include "pico8_planet_family.h"`.
5. `grep -rnE '\((const\s+)?pico8_jo_t\s*\*\)\s*self\b' src/pico8_planet_family*.c | wc -l` → **8** (dispatchers depuis `void *`, et `pico8_ga_mm`) ; aucune des 11 lignes de la Solution, point 3, ne porte encore `(pico8_jo_t *)self`.
6. `grep -cE '^typedef struct \{' src/pico8_planet_family.h src/pico8_planet_family_trade.h` → `1`, `0` (seul `pico8_ga_keys_t`).
7. `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` → finit par `,lien-parent,parente`.
8. `grep -rn '\.lineage = ' src | wc -l` → **12** (inchangé).
9. `make composition-residue`, `--champs`, `--check inclusion-mutuelle`, les greps de casts et de classe → la table « Comptes ».
10. `git diff --stat p` : les fichiers de la section « Modules », rien d'autre.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable (`prd215d/copie`, arbres de travail `wt_p`, `wt_d`).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout 03a4bef` ; diff 2.15c extrait du PRD (lignes 109-1968), `git apply --check`/`git apply`, commit `p` | `p` | 0 | `git apply` sans reste | 3 s |
| 2 | `make -j4 test` ; `make e2e` | `p` | 0 | vert ; 26 conformes | 11 s ; 1 s |
| 3 | `base.sh` ; `usegraph.py` premier jet extrait avec sa ligne de clôture Markdown (`SyntaxError`), corrigé | `p` | 0 | table « État de départ » ; `paires 18` | 5 s |
| 4 | `rw.py` premier jet : le commentaire de `qh` (et de `pm`, `nr`) collé au commentaire général qui le précède ; ligne blanche insérée, relancé sur arbre remis à zéro | — | 0 | 9 structs retirées, 205 noms renommés | < 1 s |
| 5 | `fix.py` sur `syn.sh`, premier jet : 726 erreurs, 714 éditions, 12 lignes `MANUEL` (base d'adresse = un appel, `&as_hj(placed)->qh`) ; motif de base étendu aux appels ; relancé | — | 0 | 12 éditions, puis 0 erreur | < 1 min |
| 6 | `idcast.py` sur une copie, compilation `-O0 -c` : premier jet lisait l'emplacement de l'erreur (la macro, `planet.h`) ; lu sur les notes `in expansion of macro 'IDCK'` ; `uncast.py` ; `syn.sh` | — | 0 | 11 casts identité retirés ; 0 erreur | < 1 min |
| 7 | `Makefile` (`,parente`) ; `make -s composition-guard` | — | 0 | 0 ligne | 1 s |
| 8 | `make -j4 test` ; `make e2e` ; `MALLOC_PERTURB_=165 make e2e` ; commit `d` | `d` | 0 | vert ; 26 ; 26 | ≈ 15 s |
| 9 | `base.sh` ; `usegraph.py` ; `--detail` et `--check inclusion-mutuelle` comparés à `p` | `d` | 0 | table « Comptes » ; usage identique ; 41 → 39 | 5 s |
| 10 | fichiers du pied à `p` et `d` ; premier jet : `checkout HEAD` après `checkout p` a mesuré `p` deux fois ; refait sur `d` | `p`, `d` | 0 | 27 → 26 (`planet_family_trade.h`) | 2 s |
| 11 | `git worktree add` `wt_p`, `wt_d` ; `objs.sh` `-O0 -DNDEBUG` et `-O0` avec `assert` ; `perm.py` ; `main_class_coverage.c` compilé sans `-DPICO8_CLASS_COVERAGE` (avertissements, sans effet sur la comparaison) | `p`, `d` | 0 | 72/74 ; 2 modules, 3 blocs permutés ; `assert` : + `planet_display` (`__LINE__`) | ≈ 1 min |
| 12 | `objs.sh` `-O1`, `-O2` et `-O0`, `-DNDEBUG -DPICO8_CLASS_COVERAGE` | `p`, `d` | 0 | 74/74 identiques (`-O1`, `-O2`) ; `-O0` : les 2 mêmes | ≈ 1 min |
| 13 | `make -j4 test` ; `make e2e` ; `MALLOC_PERTURB_=165 make e2e` dans `wt_p` et `wt_d` | `p`, `d` | 0 | vert ; 26 ; 26 aux deux | non chronométré (< 1 min) |
| 14 | `mut.py p` et `mut.py d` en parallèle (13 jumelles) ; S-nr et KJ-hj : `src/` ne compilait pas (variable inutilisée), non détecté ; corrigées, relancées | `wt_p`, `wt_d` | 0 | tableau des jumelles, aucun ÉCHEC | non chronométré (≈ 8 min estimé) |
| 15 | `kck.py` au parent : premier jet comptait par `atexit`, perdu par les enfants `_exit(0)` des TU ; journal par appel, puis par site ; contrôle mordant (`nl` → `mx`) | `wt_p` | 0 ; 2 (mordant) | 19/19 sites, 0 confusion ; mordant : `nl`, `qh` rouges | non chronométré (≈ 3 min estimé) |
| 16 | `obs.c`, `lin.c` (`-DV=1`) aux deux bouts, `obs` aussi sous `MALLOC_PERTURB_=165` | `p`, `d` | 0 | `9da94ae5…` ×4 ; `7e4ab61e…` ×2 | < 10 s |
| 17 | `make line-coverage` des 26 e2e aux deux bouts (en parallèle) ; `lset.py` ; comparaison directe au décalage près | `p`, `d` | 0 | 6032 / 6032 ; `L` 157, 157/157, 0 perdue, 0 gagnée | non chronométré (≈ 2 min estimé) |
| 18 | `make class-coverage-net` ; `make -j2 sanitize-net` | `d` | 0 | `couverture: OK` ; `26 propre(s)` | 2 s ; 6 s |
| 19 | garde : `pico8_qh_t` remise, `make -s composition-guard`, retirée | `wt_d` | 2 ; 0 | 1 ligne `parente` ; 0 | 1 s |
| 20 | `neg.c` (`-DN=1` à `5`) | `wt_d` | 1 ×4 ; 0 | erreurs attendues ; N=5 compile | < 2 s |
| 21 | statistiques du diff, hachages d'arbre, greps de Sortie ; `git rev-parse 4124a2c:src …` (2.15c commitée entre-temps) | `p`, `d` | 0 | arbres de `p` = `4124a2c` = `b35ca48` | 1 s |
| 22 | `run215d.sh` sur `wt_p` remis à `p`, deux fois ; `git diff d` | `wt_p` | 0 | diff vide ; 9 s | 9 s |
| 23 | `rm -rf` de la copie, des arbres de travail et des fichiers d'essai | — | 0 | supprimés | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas `b35ca48` (ou un descendant qui ne touche que `docs/`) sans que l'écart soit rapporté, ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si 2.15d n'est pas un commit, vert (`make test`, `make e2e` 26 conformes, et 26 sous `MALLOC_PERTURB_=165`), ou si un `*.sha256`, `tests/e2e/`, `tools/`, une constante `*_HASH`/`FROZEN_*` change, ou le `Makefile` ailleurs qu'à `COMPOSITION_RESIDUE_GUARD`.
- `FAILED:` si, au départ `b35ca48`, l'arbre commité n'a pas les hachages `src` `f520dbf0…`, `tests` `b15bb8ec…`, `Makefile` `e9226b4f…`, sans ligne `MANUEL` ni repli rapporté.
- `FAILED:` s'il reste un nom de type de sorte, un alias de type ou une macro qui en tient lieu, un chemin enchâssé hors `keys->jo`, un des 11 casts identité, une ligne de `parente` ; si `parente` n'est pas dans `COMPOSITION_RESIDUE_GUARD` ; si la garde n'a pas été prouvée mordante.
- `FAILED:` si une fonction est renommée, une table de classe, un masque ou un ordre d'initialisation change, ou si une ligne `#include` autre que `planet_display.c:18` est ajoutée ou retirée.
- `FAILED:` si un objet de `src/` diffère sous `-O1` ou `-O2 -DNDEBUG`, ou si, sous `-O0 -DNDEBUG`, un module autre que `planet_family`, `planet_family_trade` diffère, ou diffère autrement que par des permutations (`perm.py`).
- `FAILED:` si une jumelle a un TU rouge au parent qui ne l'est pas à l'enfant, ou un e2e < 26 au parent et 26 à l'enfant ; si `kck.py` voit une confusion ou atteint moins de 19 sites ; si la sortie d'`obs.c` (aussi sous perturbation) ou de `lin.c` diffère entre `p` et `d`.
- `FAILED:` si les comptes ne sont pas ceux de la table « Comptes » (`parente` 0 ; `lien-parent` 0 ; `void-ptr` 3 ; pied 42 / 26 ; `--champs` 238 ; `inclusion-mutuelle` 15 / 15, 39 lignes ; usage 18 ; casts 469 ; tests de classe 647 ; `pico8_jo_t` 424 octets), ou si une catégorie gardée n'est plus à 0.
- `FAILED:` si `class-coverage-net` ou `sanitize-net` est rouge ; si une ligne couverte par les 26 e2e à `p` ne l'est plus à `d` (au décalage de `planet_display.c` près), ou l'inverse.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si le message de commit omet : les neuf types retirés, le script et ses nombres (205, 726, 11), l'include retiré, la garde, la preuve des objets, la table des jumelles avec leur résultat au parent, `kck.py`, les comptes, `L`, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.15d » au bout de sa ligne du plan, ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Les membres `hd`/`mm` typés (2.16) ; les paires d'inclusion (2.17), dont `planet.h:11`, `planet.c:19` et les includes sans usage d'`economy_score.c`, `economy_tiers.c`, `planet_trade.c`.
- Les aides de TU devenues identiques (`as_qh`, `as_hj`…), les alias de TU ; les accesseurs `pico8_qh_struct_class` & co. que le jeu n'appelle plus.
- La taille des planètes (424 octets, inchangée) ; la taille du code C ; l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Le nom de type devient `pico8_jo_t` partout, sans alias.** La décision A dit « un seul type » ; un `typedef pico8_jo_t pico8_qh_t;` garderait neuf noms pour un type et échapperait au compteur. Les fonctions de sorte gardent leur nom : c'est la sorte (sa table, son archétype) qui les distingue, pas le type.
- **G2. Le commentaire de chaque sorte remonte au-dessus de son `_ready`.** Il porte un contrat (ce que la sorte copie de son archétype, qui écrit `qy`, `qw`, `ht`, le `by` partagé des `ga`) ; sans struct, le groupe `_ready`/`_kind`/`_struct_class` de la sorte est sa place. La phrase TROU-7 est dite une fois, sur `pico8_jo_t`.
- **G3. Les 11 upcasts devenus identités sont retirés ; les 19 casts de sorte restent en `pico8_jo_t`.** Les premiers ne convertissent plus rien ; les seconds partent d'un `void *` ou d'un en-tête. Détection par le compilateur (`idcast.py`), pas à l'œil.
- **G4. Les alias et aides de TU restent** (`nr = fr; qh = nr;`, `as_qh`) : les simplifier n'est pas un chemin enchâssé ni un cast, et toucherait des fonctions d'empreinte de TU.
- **G5. Pas de reformatage** : la ligne `self->qw =` coupée par 2.15c reste coupée ; le diff ne contient que ce que le script fait.
- **G6. Pas de ligne K à l'enfant** : les objets sont identiques (preuve 4) ; la seule chose neuve est la garde, prouvée mordante (preuve 9).
- **G7. Le diff des TU n'est pas recopié** : 2876 lignes mécaniques ; le script, son résultat haché et le diff de `src/` (annexe B) suffisent à le reproduire et à le relire.

### Surprises de mesure, pour `plan-pilot`

1. **Le volume est quatre fois l'estimation** (173 lignes de `.c` de `src/`, 625 de TU, contre ≈ 37 et ≈ 165) : l'estimation datait d'avant les 236 chemins transitoires de 2.15c. Le script le rend sans effet sur le budget (9 s).
2. **Aucun module ne change de code** : à `-O1`/`-O2`, les 74 objets sont identiques octet pour octet ; à `-O0`, seul l'ordre de chargement de trois arguments change dans trois fonctions.
3. **Onze casts deviennent des identités** (`pico8_jo_pq((pico8_jo_t *)self)`…) : le motif des casts passe de 480 à 469.
4. **Le graphe d'usage ne bouge pas** (18) : la paire d'usage `planet_display` ↔ `planet_family` était sortie à 2.15c ; 2.15d ne retire que la paire d'inclusion (15 / 39, comme le contrat de 2.15c le disait).
5. **`--champs` ne bouge pas** : les structs de sorte n'avaient que leur ascendante, que le relevé ne lit pas.

### Questions pour l'humain

Aucune.

### Annexe A — scripts (texte exact)

Tous hors dépôt. Les six premiers forment le dossier `outils/` de `run215d.sh` (un dossier à côté de l'arbre, jamais dedans) ; les autres se lancent depuis le dossier des essais, avec deux arbres de travail `wt_p` (parent) et `wt_d` (enfant) après `make test` dans chacun. `S` y vaut le dossier des essais (à adapter). `lin.c`, `obs.c`, `lset.py` : annexe A du PRD 2.15c (`docs/prd/done/plan3-phase2-etape15c-etat-planete.md`), inchangés, compilés `-DV=1` aux deux bouts ; `usegraph.py` : annexe A du PRD 2.11 (`docs/prd/done/plan3-phase2-etape11-etude-inclusions.md`).

`syn.sh` :

```sh
#!/bin/sh
# compile tout l'arbre en -fsyntax-only ; rend les lignes error:
cd "$1"
ls src/*.c tests/*.c | grep -vE 'main_raylib|main_screencast_viewer|raylib_backend|class_coverage' \
  | xargs -P4 -n8 cc -std=c99 -Wall -Wextra -Werror -Isrc -fsyntax-only 2>&1
for f in src/main_class_coverage.c tests/test_class_coverage.c; do
  cc -std=c99 -Wall -Wextra -Werror -Isrc -DPICO8_CLASS_COVERAGE -fsyntax-only "$f" 2>&1
done
```

`rw.py` :

```python
# rw.py ARBRE : 2.15d, partie texte. Retire les neuf structs de sorte (le
# commentaire de chacune, sans « Nothing frees the struct: TROU-7. », passe
# au-dessus de son `_ready`), renomme les neuf types en pico8_jo_t dans src/
# et tests/, reecrit le commentaire de pico8_jo_t, retire planet_display.c:18.
import re, sys, glob, os
T = sys.argv[1]
def rd(f): return open(os.path.join(T, f)).read()
def wr(f, s): open(os.path.join(T, f), 'w').write(s)
KINDS = 'qh nl os hj pm ga nr fr mx'.split()
BLOCK = re.compile(r'(/\*(?:[^*]|\*(?!/))*\*/)\ntypedef struct \{\n    pico8_(?:jo|qh|hj|nr)_t (?:jo|qh|hj|nr);\n\} pico8_(\w+?)_t;\n\n')
def reflow(c):
    body = ' '.join(l.strip().lstrip('/*').strip() for l in c.split('\n')).replace('*/', '').strip()
    body = body.replace(' Nothing frees the struct: TROU-7.', '').replace('`jo.by`', '`by`')
    words, lines, cur = body.split(), [], '/*'
    for w in words:
        if len(cur) + 1 + len(w) > 77: lines.append(cur); cur = ' *'
        cur += ' ' + w
    return '\n'.join(lines + [cur + ' */'])
comments = {}
for h in ('src/pico8_planet_family.h', 'src/pico8_planet_family_trade.h'):
    s = rd(h)
    for m in BLOCK.finditer(s): comments[m.group(2)] = reflow(m.group(1))
    s, n = BLOCK.subn('', s)
    for k in KINDS:
        r = f'int pico8_{k}_ready(void);'
        if k in comments and r in s:
            s = s.replace('*/\n' + r, '*/\n\n' + r).replace(r, f'{comments[k]}\n{r}')
    wr(h, s)
    print(h, 'structs retirees', n)
assert sorted(comments) == sorted(KINDS), comments.keys()
s = rd('src/pico8_planet.h')
old = """/* A bare planet: its kind's archetype (`kw` points at the one kw every bare
 * planet shares, written in place by `bb` and `mx:kf`, until `lz` points it at
 * the planet's own `kw_own`), its construction keys `id`/`p`/`ra`, then every
 * field jo's functions write on it, then its kinds' fields. Every planet
 * struct begins with one, so jo's functions take it for `self`. A kind's
 * field is indeterminate on a planet of another kind; `probe`, `emitter` are
 * NULL there, else they point at `probe_own`, `emitter_own`: never copy the
 * struct. Nothing frees the struct: TROU-7. */"""
new = """/* A planet of any of the ten kinds, which differ only by class table and
 * archetype: its archetype (`kw` points at the one kw every bare planet
 * shares, written in place by `bb` and `mx:kf`, until `lz` points it at the
 * planet's own `kw_own`), its construction keys `id`/`p`/`ra`, then every
 * field jo's functions write on it, then its kinds' fields. A kind's field is
 * indeterminate on a planet of another kind; `probe`, `emitter` are NULL
 * there, else they point at `probe_own`, `emitter_own`: never copy the
 * struct. Nothing frees the struct: TROU-7. */"""
assert s.count(old) == 1; wr('src/pico8_planet.h', s.replace(old, new))
s = rd('src/pico8_planet_display.c')
assert s.count('#include "pico8_planet_family.h"\n') == 1
wr('src/pico8_planet_display.c', s.replace('#include "pico8_planet_family.h"\n', ''))
TY = re.compile(r'\bpico8_(?:' + '|'.join(KINDS) + r')_t\b')
n = 0
for f in sorted(glob.glob(os.path.join(T, 'src/*.[ch]')) + glob.glob(os.path.join(T, 'tests/*.[ch]'))):
    s = open(f).read(); s2, k = TY.subn('pico8_jo_t', s)
    if k: open(f, 'w').write(s2); n += k
print('types renommes', n)
```

`fix.py` :

```python
# fix.py < erreurs de syn.sh (LANG=C), depuis la racine de l'arbre : pour chaque
# "'pico8_jo_t' has no member named 'jo|qh|hj|nr'", retire la chaine enchassee
# (`X->hj.qh.jo.m` -> `X->m` ; `&X->qh` -> `X` ; `&v.qh` -> `&v`). Relance
# jusqu'a 0 edition.
import re, sys, collections
CH = r'(?:jo|qh|hj|nr)(?:\.(?:jo|qh|hj|nr))*'
err = sys.stdin.read()
ed = collections.defaultdict(set)
for m in re.finditer(r"^([^:\s]+):(\d+):(\d+): error: '(?:const )?pico8_jo_t'(?: \{aka [^}]*\})? has no member named '(jo|qh|hj|nr)'", err, re.M):
    ed[m.group(1)].add((int(m.group(2)), int(m.group(3))))
n = 0; manual = []
for f, es in ed.items():
    L = open(f).read().split('\n')
    for l, c in sorted(es, reverse=True):
        line = L[l - 1]; i = c - 1
        if line[i:i + 2] == '->': i += 2
        elif line[i] == '.': i += 1
        while line[i] == ' ': i += 1
        m = re.compile(CH).match(line, i)
        assert m, (f, l, c, line)
        j = m.end()
        if j < len(line) and line[j] == '.' and re.match(r'[A-Za-z_]', line[j + 1:j + 2]):
            L[l - 1] = line[:i] + line[j + 1:]; n += 1; continue
        op = line[i - 2:i] if line[i - 2:i] == '->' else line[i - 1]
        pre = line[:i - len(op)]
        A = r'[A-Za-z_]\w*(?:\((?:[^()]|\([^()]*\))*\))?(?:\[[^\]]*\])?'
        b = re.search(r'&\s*((?:' + A + r'(?:->|\.))*' + A + r')$', pre)
        if b is None:
            manual.append(f'{f}:{l}: {line.strip()}'); continue
        if op == '->':
            L[l - 1] = pre[:b.start()] + b.group(1) + line[j:]
        else:
            L[l - 1] = pre + line[j:]
        n += 1
    open(f, 'w').write('\n'.join(L))
print('edits', n)
for x in manual: print('MANUEL', x)
```

`idcast.py` :

```python
# idcast.py ARBRE : dans une copie jetable, enveloppe chaque cast vers
# pico8_jo_t d'un controle qui echoue a la compilation (-O0 -c) si l'operande
# est deja un pico8_jo_t * : liste les casts identite.
import re, sys, glob, os
T = sys.argv[1]
R = re.compile(r'\(((?:const\s+)?pico8_jo_t\s*\*)\)\s*([A-Za-z_]\w*(?:(?:->|\.)\w+|\([^()]*\)|\[[^\]]*\])*)')
n = 0
for f in glob.glob(T + '/src/*.c') + glob.glob(T + '/tests/*.c'):
    s = open(f).read(); s2, k = R.subn(lambda m: f'(({m.group(1)})IDCK({m.group(2)}))', s)
    if k: open(f, 'w').write(s2); n += k
h = T + '/src/pico8_planet.h'; s = open(h).read()
s = s.replace('} pico8_jo_t;\n', '} pico8_jo_t;\nextern void *pico8_idck_fail(const void *) __attribute__((error("cast identite")));\n#define IDCK(x) __builtin_choose_expr(__builtin_types_compatible_p(__typeof__(x), pico8_jo_t *) || __builtin_types_compatible_p(__typeof__(x), const pico8_jo_t *), pico8_idck_fail(x), (x))\n', 1)
open(h, 'w').write(s)
print('casts enveloppes', n)
```

`uncast.py` :

```python
# uncast.py ARBRE LISTE : retire le cast identite `(pico8_jo_t *)self` aux lignes
# que idcast.py a rendues (fichier:ligne:...).
import sys, collections
T, lst = sys.argv[1], sys.argv[2]
by = collections.defaultdict(set)
for l in open(lst):
    f, n = l.split(':')[:2]; by[f].add(int(n))
k = 0
for f, ns in by.items():
    L = open(f'{T}/{f}').read().split('\n')
    for n in ns:
        assert L[n - 1].count('(pico8_jo_t *)self') == 1, (f, n, L[n - 1])
        L[n - 1] = L[n - 1].replace('(pico8_jo_t *)self', 'self'); k += 1
    open(f'{T}/{f}', 'w').write('\n'.join(L))
print('casts retires', k)
```

`run215d.sh` :

```sh
#!/bin/sh
# run215d.sh ARBRE : 2.15d de bout en bout sur un arbre propre au commit 2.15c.
# Les scripts sont dans le dossier de celui-ci ; rien n'est ecrit hors ARBRE et
# du dossier temporaire $W (supprime a la fin).
set -eu
O=$(cd "$(dirname "$0")" && pwd); T=$(cd "$1" && pwd); W=$(mktemp -d "$O/w.XXXXXX")
python3 "$O/rw.py" "$T"
k=1
while :; do
    (cd "$T" && LANG=C sh "$O/syn.sh" .) > "$W/syn$k.txt" 2>&1 || true
    r=$(cd "$T" && LANG=C python3 "$O/fix.py" < "$W/syn$k.txt")
    echo "passe $k : $(grep -c 'error' "$W/syn$k.txt") erreur(s) ; $r"
    case "$r" in "edits 0"*) break ;; esac
    k=$((k + 1)); [ $k -le 8 ] || { echo "arret : 8 passes"; exit 1; }
done
[ "$(grep -c 'error' "$W/syn$k.txt")" = 0 ] || { echo "arret : erreurs restantes"; exit 1; }
cp -r "$T/src" "$T/tests" "$W/" && mkdir -p "$W/chk" && mv "$W/src" "$W/tests" "$W/chk/"
python3 "$O/idcast.py" "$W/chk"
(cd "$W/chk" && ls src/*.c tests/*.c | grep -vE 'main_raylib|main_screencast_viewer|raylib_backend' | while read f; do
    cc -std=gnu99 -O0 -DPICO8_CLASS_COVERAGE -Isrc -c -o /dev/null "$f" 2>&1 || true; done) \
  | grep "in expansion of macro 'IDCK'" | grep -v 'pico8_planet\.h' | sort -u > "$W/idcast.txt"
echo "casts identite : $(wc -l < "$W/idcast.txt")"
python3 "$O/uncast.py" "$T" "$W/idcast.txt"
sed -i 's/^\(COMPOSITION_RESIDUE_GUARD := .*,lien-parent\)$/\1,parente/' "$T/Makefile"
(cd "$T" && LANG=C sh "$O/syn.sh" .) > "$W/final.txt" 2>&1 || true
echo "erreurs finales : $(grep -c 'error' "$W/final.txt" || true)"
rm -rf "$W"
```

`base.sh` :

```sh
#!/bin/sh
# base.sh ARBRE [FLAT] : comptes de la table
S=/tmp/claude-0/-home-user-slipways-love/c83e28ad-64b1-5e5c-ac9f-9444621e8cef/scratchpad/prd215d
cd "$1"
echo "== compteur"; sh tools/composition_residue.sh
echo "== champs"; sh tools/composition_residue.sh --champs | tail -1
echo "== incl"; sh tools/composition_residue.sh --check inclusion-mutuelle src tests | tail -1
echo "== parente"; sh tools/composition_residue.sh --check parente src tests
echo "== casts"; grep -rEo '\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)' src tests | wc -l
echo "== cls"; grep -rEo '(->cls|header\.cls|_struct_class\(\))' src tests | wc -l
echo "== usegraph"; python3 $S/usegraph.py . 2>&1 >/dev/null
echo "== guard"; grep '^COMPOSITION_RESIDUE_GUARD' Makefile; make -s composition-guard 2>&1 | tail -1
echo "== sizeof"; cc -std=c99 -Isrc $2 -o $S/sizes $S/sizes.c && $S/sizes
```

`sizes.c` :

```c
#include <stdio.h>
#include "pico8_struct.h"
#include "pico8_planet.h"
#include "pico8_planet_family.h"
#include "pico8_planet_family_trade.h"
#include "pico8_probe.h"
int main(void){
 printf("hdr %zu g_ %zu jo %zu probe %zu emitter %zu\n", sizeof(pico8_struct_header_t), sizeof(pico8_g__t), sizeof(pico8_jo_t), sizeof(pico8_probe_t), sizeof(pico8_emitter_t));
#ifndef FLAT
 printf("qh %zu nl %zu os %zu hj %zu pm %zu mx %zu ga %zu nr %zu fr %zu\n", sizeof(pico8_qh_t),sizeof(pico8_nl_t),sizeof(pico8_os_t),sizeof(pico8_hj_t),sizeof(pico8_pm_t),sizeof(pico8_mx_t),sizeof(pico8_ga_t),sizeof(pico8_nr_t),sizeof(pico8_fr_t));
#endif
 return 0;}
```

`objs.sh` :

```sh
#!/bin/bash
# objs.sh ARBRE OUT [DEFS] : objets sans -g de src/ (hors raylib), -O0, desassembles
S=/tmp/claude-0/-home-user-slipways-love/c83e28ad-64b1-5e5c-ac9f-9444621e8cef/scratchpad/prd215d
cd "$1"; mkdir -p $S/$2
for f in $(ls src/*.c | grep -vE 'main_raylib|main_screencast_viewer|raylib_backend'); do
  b=$(basename $f .c); cc -std=c99 ${O:--O0} $3 -Isrc -c -o $S/$2/$b.o $f && objdump -d $S/$2/$b.o | tail -n +3 > $S/$2/$b.dis && nm $S/$2/$b.o > $S/$2/$b.nm && objdump -s -j .rodata $S/$2/$b.o 2>/dev/null | tail -n +3 > $S/$2/$b.ro
done
```

`perm.py` :

```python
# perm.py A.dis B.dis : chaque bloc different est-il une permutation des memes instructions ?
import sys, difflib, re
a = open(sys.argv[1]).read().split('\n'); b = open(sys.argv[2]).read().split('\n')
ins = lambda l: re.sub(r'^\s*[0-9a-f]+:\s*', '', l)
fn = ''; out = []
sm = difflib.SequenceMatcher(None, a, b, autojunk=False)
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag == 'equal': continue
    f = [l for l in a[:i1] if l.endswith('>:')][-1]
    out.append((f, i2 - i1, sorted(map(ins, a[i1:i2])) == sorted(map(ins, b[j1:j2]))))
print(len(out), 'blocs ;', 'tous permutations' if all(x[2] for x in out) else 'PAS PERMUTATION', ';', ' '.join(f'{f[17:-2]}({n})' for f, n, _ in out))
```

`mut.py` :

```python
# mut.py p|d NOM... : applique la mutation dans wt_<cote>, make -k -j4 unit,
# puis make -k e2e ; imprime les TU rouges et le nombre d'e2e conformes ;
# restaure par git checkout -f.
import json, sys, subprocess, re, os
S = '/tmp/claude-0/-home-user-slipways-love/c83e28ad-64b1-5e5c-ac9f-9444621e8cef/scratchpad/prd215d'
side = sys.argv[1]; W = f'{S}/wt_{side}'; M = json.load(open(f'{S}/muts.json'))
for name in sys.argv[2:]:
    for f, anc, old, new in M[name][side]:
        p = f'{W}/{f}'; s = open(p).read()
        i = s.index(anc) if anc else 0
        if anc is None: assert s.count(old) == 1, (name, old)
        j = s.index(old, i); s = s[:j] + new + s[j + len(old):]; open(p, 'w').write(s)
    env = dict(os.environ); env.pop('MALLOC_PERTURB_', None)
    u = subprocess.run('timeout 900 make -k -j4 unit', shell=True, cwd=W, capture_output=True, text=True, env=env)
    out = u.stdout + u.stderr
    red = sorted(set(re.findall(r'\*\*\* \[\S+: unit-run-test_pico8_(\w+)\]', out)))
    comp = sorted(set(re.findall(r'\*\*\* \[\S+: tmp/(?:test_pico8_|obj/src/)(\w+?)(?:\.o)?\]', out)))
    e = subprocess.run('timeout 600 make -k e2e', shell=True, cwd=W, capture_output=True, text=True, env=env)
    ok = len(re.findall(r': conforme', e.stdout + e.stderr))
    print(f'{name}\t{side}\trouges={",".join(red) or "aucun"}\tcompile={",".join(comp) or "-"}\te2e={ok}', flush=True)
    subprocess.run('git checkout -qf -- . && git clean -qfd src tests', shell=True, cwd=W)
```

`muts.json` :

```json
{
 "PR-qh": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "    self->jo.header.probe = &self->jo.probe_own;\n",
    ""
   ]
  ],
  "d": [
   [
    "src/pico8_planet_family.c",
    null,
    "    self->header.probe = &self->probe_own;\n",
    ""
   ]
  ]
 },
 "EM-hj": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "    self->qh.jo.header.emitter = &self->qh.jo.emitter_own;\n",
    ""
   ]
  ],
  "d": [
   [
    "src/pico8_planet_family.c",
    "void pico8_hj_struct_init(",
    "    self->header.emitter = &self->emitter_own;\n",
    ""
   ]
  ]
 },
 "EM-ga": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "    self->jo.header.emitter = &self->jo.emitter_own;\n",
    ""
   ]
  ],
  "d": [
   [
    "src/pico8_planet_family.c",
    "pico8_ga_struct_new(const",
    "    self->header.emitter = &self->emitter_own;\n",
    ""
   ]
  ]
 },
 "QH-nr": {
  "p": [
   [
    "src/pico8_planet_family_trade.c",
    "g_nr_struct_class = {",
    ".lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),",
    ".lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),"
   ]
  ],
  "d": [
   [
    "src/pico8_planet_family_trade.c",
    "g_nr_struct_class = {",
    ".lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),",
    ".lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),"
   ]
  ]
 },
 "JO-ga": {
  "p": [
   [
    "src/pico8_planet_family.c",
    "g_ga_struct_class = {",
    ".lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),",
    ".lineage = 0,"
   ]
  ],
  "d": [
   [
    "src/pico8_planet_family.c",
    "g_ga_struct_class = {",
    ".lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),",
    ".lineage = 0,"
   ]
  ]
 },
 "QW-nl": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "    self->qh.jo.qw = self->qh.jo.def->qw;\n",
    "    self->qh.jo.qw = pico8_fixed_from_int(0);\n"
   ]
  ],
  "d": [
   [
    "src/pico8_planet_family.c",
    null,
    "    self->qw = self->def->qw;\n",
    "    self->qw = pico8_fixed_from_int(0);\n"
   ]
  ]
 },
 "HT-mx": {
  "p": [
   [
    "src/pico8_planet_family_trade.c",
    null,
    "    self->hj.qh.jo.ht = self->hj.qh.jo.def->ht;\n",
    "    self->hj.qh.jo.ht = NULL;\n"
   ]
  ],
  "d": [
   [
    "src/pico8_planet_family_trade.c",
    null,
    "    self->ht = self->def->ht;\n",
    "    self->ht = NULL;\n"
   ]
  ]
 },
 "S-nr": {
  "p": [
   [
    "src/pico8_planet_family_trade.c",
    null,
    "    self->qh.jo.s = s_val;\n",
    "    (void)s_val;\n"
   ]
  ],
  "d": [
   [
    "src/pico8_planet_family_trade.c",
    null,
    "    self->s = s_val;\n",
    "    (void)s_val;\n"
   ]
  ]
 },
 "MT-fr": {
  "p": [
   [
    "src/pico8_planet_family_trade.c",
    null,
    "    self->nr.qh.jo.header.mt = &self->nr.qh.jo.def->mt;\n",
    ""
   ]
  ],
  "d": [
   [
    "src/pico8_planet_family_trade.c",
    null,
    "    self->header.mt = &self->def->mt;\n",
    ""
   ]
  ]
 },
 "IU-ga": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "            self->jo.iu = keys->pq->iu;\n",
    ""
   ]
  ],
  "d": [
   [
    "src/pico8_planet_family.c",
    null,
    "            self->iu = keys->pq->iu;\n",
    ""
   ]
  ]
 },
 "PQ-mx": {
  "p": [
   [
    "src/pico8_planet_family_trade.c",
    "pico8_mx_struct_new(const",
    "    pico8_jo_pq((pico8_jo_t *)self);\n",
    ""
   ]
  ],
  "d": [
   [
    "src/pico8_planet_family_trade.c",
    "pico8_mx_struct_new(const",
    "    pico8_jo_pq(self);\n",
    ""
   ]
  ]
 },
 "KF-mx": {
  "p": [
   [
    "src/pico8_planet_family_trade.c",
    null,
    "    pico8_jo_kf((pico8_jo_t *)self, sw);\n",
    ""
   ]
  ],
  "d": [
   [
    "src/pico8_planet_family_trade.c",
    null,
    "    pico8_jo_kf(self, sw);\n",
    ""
   ]
  ]
 },
 "KJ-hj": {
  "p": [
   [
    "src/pico8_planet_family.c",
    null,
    "    pico8_jo_kj(gfx, mem, (pico8_jo_t *)self, p);\n",
    "    (void)p;\n"
   ]
  ],
  "d": [
   [
    "src/pico8_planet_family.c",
    null,
    "    pico8_jo_kj(gfx, mem, self, p);\n",
    "    (void)p;\n"
   ]
  ]
 }
}
```

`kck.py` :

```python
# kck.py ARBRE : au parent seulement, chaque cast vers une struct de sorte K
# verifie a l'execution que la classe de l'objet est K ou une sorte qui
# descend de K ; sinon abort. Compte les passages (fichier KCK_LOG).
import re, sys, glob
T = sys.argv[1]
R = re.compile(r'\(((?:const\s+)?)pico8_(qh|nl|os|hj|pm|ga|nr|fr|mx)_t\s*\*\)\s*([A-Za-z_]\w*(?:(?:->|\.)\w+|\([^()]*\)|\[[^\]]*\])*)')
n = 0
for f in glob.glob(T + '/src/*.c') + glob.glob(T + '/tests/*.c'):
    s = open(f).read(); s2, k = R.subn(lambda m: f'({m.group(1)}pico8_{m.group(2)}_t *)KCK({m.group(3)}, "{m.group(2)}")', s)
    if k: open(f, 'w').write(s2); n += k
h = T + '/src/pico8_planet_family.h'; s = open(h).read()
BLOCK = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern const pico8_struct_class_t *pico8_qh_struct_class(void) __attribute__((weak));
extern const pico8_struct_class_t *pico8_nl_struct_class(void) __attribute__((weak));
extern const pico8_struct_class_t *pico8_os_struct_class(void) __attribute__((weak));
extern const pico8_struct_class_t *pico8_hj_struct_class(void) __attribute__((weak));
extern const pico8_struct_class_t *pico8_pm_struct_class(void) __attribute__((weak));
extern const pico8_struct_class_t *pico8_ga_struct_class(void) __attribute__((weak));
extern const pico8_struct_class_t *pico8_nr_struct_class(void) __attribute__((weak));
extern const pico8_struct_class_t *pico8_fr_struct_class(void) __attribute__((weak));
extern const pico8_struct_class_t *pico8_mx_struct_class(void) __attribute__((weak));
static long kck_n[9];
static const char *const kck_k[9] = {"qh", "nl", "os", "hj", "pm", "ga", "nr", "fr", "mx"};
static void kck_dump(void) {
    const char *p = getenv("KCK_LOG"); FILE *o; int i;
    if (p == NULL || (o = fopen(p, "a")) == NULL) return;
    for (i = 0; i < 9; i++) if (kck_n[i]) fprintf(o, "%s %ld\n", kck_k[i], kck_n[i]);
    fclose(o);
}
static inline int kck_is(const pico8_struct_class_t *c, const pico8_struct_class_t *(*a)(void)) { return a != NULL && c == a(); }
static inline const void *pico8_kck(const void *x, const char *k, const char *f, int l) {
    static int reg; const pico8_struct_class_t *c; int ok, i;
    if (x == NULL) return x;
    c = ((const pico8_struct_header_t *)x)->cls;
    if (!strcmp(k, "qh")) ok = kck_is(c, pico8_qh_struct_class) || kck_is(c, pico8_nl_struct_class) || kck_is(c, pico8_os_struct_class) || kck_is(c, pico8_hj_struct_class) || kck_is(c, pico8_pm_struct_class) || kck_is(c, pico8_mx_struct_class) || kck_is(c, pico8_nr_struct_class) || kck_is(c, pico8_fr_struct_class);
    else if (!strcmp(k, "hj")) ok = kck_is(c, pico8_hj_struct_class) || kck_is(c, pico8_pm_struct_class) || kck_is(c, pico8_mx_struct_class);
    else if (!strcmp(k, "nr")) ok = kck_is(c, pico8_nr_struct_class) || kck_is(c, pico8_fr_struct_class);
    else if (!strcmp(k, "nl")) ok = kck_is(c, pico8_nl_struct_class);
    else if (!strcmp(k, "os")) ok = kck_is(c, pico8_os_struct_class);
    else if (!strcmp(k, "pm")) ok = kck_is(c, pico8_pm_struct_class);
    else if (!strcmp(k, "ga")) ok = kck_is(c, pico8_ga_struct_class);
    else if (!strcmp(k, "fr")) ok = kck_is(c, pico8_fr_struct_class);
    else ok = kck_is(c, pico8_mx_struct_class);
    if (!ok) { fprintf(stderr, "KCK confusion de sorte: cast vers %s, %s:%d\n", k, f, l); abort(); }
    (void)reg; (void)i; (void)kck_n; (void)kck_k; (void)kck_dump;
    { const char *p = getenv("KCK_LOG"); FILE *o = p != NULL ? fopen(p, "a") : NULL; if (o != NULL) { fprintf(o, "%s %s:%d\n", k, f, l); fclose(o); } }
    return x;
}
#define KCK(x, k) ((void)pico8_kck((const void *)(x), k, __FILE__, __LINE__), (x))
'''
anchor = '} pico8_ga_t;\n'
assert s.count(anchor) == 1
open(h, 'w').write(s.replace(anchor, anchor + BLOCK))
print('casts instrumentes', n)
```

`neg.c` :

```c
#include "pico8_planet_family.h"
#include "pico8_planet_family_trade.h"
#if N == 1
pico8_qh_t *x;
#elif N == 2
void f(pico8_jo_t *e) { e->jo.s = 0; }
#elif N == 3
void f(pico8_jo_t *e) { (void)e->hj.qh.jo.emitter; }
#elif N == 4
void f(pico8_jo_t *e) { (void)&e->nr; }
#elif N == 5
void f(void) { pico8_jo_t *a = pico8_fr_new(0), *b = pico8_mx_new(0), *c = pico8_ga_new(0); pico8_nr_hb(a); pico8_mx_kf(b, 0); pico8_ga_pq(c); pico8_nl_ly(pico8_nl_new(0), 0); }
#endif
```

### Annexe B — diff de `src/` et du `Makefile` (`git diff p d -- Makefile src`, texte exact, 1160 lignes)

```diff
diff --git a/Makefile b/Makefile
index 30351cc..e9226b4 100644
--- a/Makefile
+++ b/Makefile
@@ -871,7 +871,7 @@ composition-residue: tools/composition_residue.sh
 	@sh tools/composition_residue.sh
 
 # Catégories du compteur de composition tombées à 0, séparées par des virgules.
-COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6,lien-parent
+COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6,lien-parent,parente
 
 composition-guard: tools/composition_residue.sh
 	@sh tools/composition_residue.sh --check $(COMPOSITION_RESIDUE_GUARD) src tests
diff --git a/src/pico8_planet.h b/src/pico8_planet.h
index 44c4a5a..4fa9109 100644
--- a/src/pico8_planet.h
+++ b/src/pico8_planet.h
@@ -17,13 +17,13 @@
  * for the value-returning functions), and an absent or wrong-kind field
  * reads as absent rather than crashing. */
 
-/* A bare planet: its kind's archetype (`kw` points at the one kw every bare
- * planet shares, written in place by `bb` and `mx:kf`, until `lz` points it at
- * the planet's own `kw_own`), its construction keys `id`/`p`/`ra`, then every
- * field jo's functions write on it, then its kinds' fields. Every planet
- * struct begins with one, so jo's functions take it for `self`. A kind's
- * field is indeterminate on a planet of another kind; `probe`, `emitter` are
- * NULL there, else they point at `probe_own`, `emitter_own`: never copy the
+/* A planet of any of the ten kinds, which differ only by class table and
+ * archetype: its archetype (`kw` points at the one kw every bare planet
+ * shares, written in place by `bb` and `mx:kf`, until `lz` points it at the
+ * planet's own `kw_own`), its construction keys `id`/`p`/`ra`, then every
+ * field jo's functions write on it, then its kinds' fields. A kind's field is
+ * indeterminate on a planet of another kind; `probe`, `emitter` are NULL
+ * there, else they point at `probe_own`, `emitter_own`: never copy the
  * struct. Nothing frees the struct: TROU-7. */
 typedef struct {
     pico8_struct_header_t header;
diff --git a/src/pico8_planet_display.c b/src/pico8_planet_display.c
index 94b3503..ad47dca 100644
--- a/src/pico8_planet_display.c
+++ b/src/pico8_planet_display.c
@@ -15,7 +15,6 @@
 #include "pico8_map_tables.h"
 #include "pico8_msgbox.h"
 #include "pico8_planet.h"
-#include "pico8_planet_family.h"
 #include "pico8_probe.h"
 #include "pico8_research.h"
 #include "pico8_research_display.h"
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index f432f85..c4209c1 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -47,19 +47,19 @@ static const pico8_struct_class_t g_qh_struct_class = {
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
-void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
-    pico8_jo_struct_init(&self->jo, cls, def);
-    self->jo.header.probe = &self->jo.probe_own;
-    self->jo.probe = &self->jo.probe_own;
-    self->jo.probe->def = &self->jo.def->probe;
-    self->jo.probe->od = self->jo.def->probe.od;
-    self->jo.probe->has_od = self->jo.def->probe.has_od;
-    self->jo.s = self->jo.def->s;
-    self->jo.probe->et = NULL;
+void pico8_qh_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
+    pico8_jo_struct_init(self, cls, def);
+    self->header.probe = &self->probe_own;
+    self->probe = &self->probe_own;
+    self->probe->def = &self->def->probe;
+    self->probe->od = self->def->probe.od;
+    self->probe->has_od = self->def->probe.has_od;
+    self->s = self->def->s;
+    self->probe->et = NULL;
 }
 
-static pico8_qh_t *pico8_qh_struct_new(const pico8_jo_keys_t *keys) {
-    pico8_qh_t *self = malloc(sizeof *self);
+static pico8_jo_t *pico8_qh_struct_new(const pico8_jo_keys_t *keys) {
+    pico8_jo_t *self = malloc(sizeof *self);
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
     pico8_qh_struct_init(self, &g_qh_struct_class, &pico8_qh_def);
@@ -68,33 +68,33 @@ static pico8_qh_t *pico8_qh_struct_new(const pico8_jo_keys_t *keys) {
                "qh: the only construction key is id (ie:ldown); another key has no member "
                "-- stop, do not add one");
         if (keys->id != NULL) {
-            self->jo.id = keys->id;
+            self->id = keys->id;
         }
     }
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("qh");
 #endif
-    pico8_jo_pq((pico8_jo_t *)self);
-    pico8_ke_struct_pq(&self->jo.header);
+    pico8_jo_pq(self);
+    pico8_ke_struct_pq(&self->header);
     return self;
 }
 
-void pico8_nl_ly(pico8_nl_t *self, pico8_fixed_t t) {
+void pico8_nl_ly(pico8_jo_t *self, pico8_fixed_t t) {
     if (self == NULL) {
         return;
     }
     pico8_na((pico8_struct_header_t *)self, t);
 
-    assert(self->qh.jo.has_mi && "nl:ly reads self.mi unguarded, unlike hj:kj");
-    if (self->qh.jo.has_mi && self->qh.jo.mi >= pico8_fixed_from_int(2)) {
-        self->qh.jo.qw =
+    assert(self->has_mi && "nl:ly reads self.mi unguarded, unlike hj:kj");
+    if (self->has_mi && self->mi >= pico8_fixed_from_int(2)) {
+        self->qw =
             pico8_fixed_add(pico8_fixed_from_int(8),
                             pico8_fixed_div(pico8_fixed_mod(t, pico8_fixed_from_int(42)), pico8_fixed_from_int(6)));
     }
 }
 
 static void pico8_nl_ly_dispatch(void *self, pico8_fixed_t t) {
-    pico8_nl_ly((pico8_nl_t *)self, t);
+    pico8_nl_ly((pico8_jo_t *)self, t);
 }
 
 static const pico8_struct_class_t g_nl_struct_class = {
@@ -118,25 +118,25 @@ static const pico8_struct_class_t g_nl_struct_class = {
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH) | PICO8_LINEAGE(PICO8_LINEAGE_NL),
 };
 
-static pico8_nl_t *pico8_nl_struct_new(const pico8_jo_keys_t *keys) {
-    pico8_nl_t *self = malloc(sizeof *self);
+static pico8_jo_t *pico8_nl_struct_new(const pico8_jo_keys_t *keys) {
+    pico8_jo_t *self = malloc(sizeof *self);
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
-    pico8_qh_struct_init(&self->qh, &g_nl_struct_class, &pico8_nl_def);
-    self->qh.jo.qw = self->qh.jo.def->qw;
+    pico8_qh_struct_init(self, &g_nl_struct_class, &pico8_nl_def);
+    self->qw = self->def->qw;
     if (keys != NULL) {
         assert(keys->p == NULL && keys->ra == NULL &&
                "nl: the only construction key is id (ie:ldown); another key has no member "
                "-- stop, do not add one");
         if (keys->id != NULL) {
-            self->qh.jo.id = keys->id;
+            self->id = keys->id;
         }
     }
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("nl");
 #endif
-    pico8_jo_pq((pico8_jo_t *)self);
-    pico8_ke_struct_pq(&self->qh.jo.header);
+    pico8_jo_pq(self);
+    pico8_ke_struct_pq(&self->header);
     return self;
 }
 
@@ -161,40 +161,40 @@ static const pico8_struct_class_t g_os_struct_class = {
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
-static pico8_os_t *pico8_os_struct_new(const pico8_jo_keys_t *keys) {
-    pico8_os_t *self = malloc(sizeof *self);
+static pico8_jo_t *pico8_os_struct_new(const pico8_jo_keys_t *keys) {
+    pico8_jo_t *self = malloc(sizeof *self);
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
-    pico8_qh_struct_init(&self->qh, &g_os_struct_class, &pico8_os_def);
+    pico8_qh_struct_init(self, &g_os_struct_class, &pico8_os_def);
     if (keys != NULL) {
         assert(keys->p == NULL && keys->ra == NULL &&
                "os: the only construction key is id (ie:ldown); another key has no member "
                "-- stop, do not add one");
         if (keys->id != NULL) {
-            self->qh.jo.id = keys->id;
+            self->id = keys->id;
         }
     }
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("os");
 #endif
-    pico8_jo_pq((pico8_jo_t *)self);
-    pico8_ke_struct_pq(&self->qh.jo.header);
+    pico8_jo_pq(self);
+    pico8_ke_struct_pq(&self->header);
     return self;
 }
 
-void pico8_hj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_hj_t *self, pico8_vec_t p) {
+void pico8_hj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec_t p) {
     if (self == NULL) {
         return;
     }
-    if (self->qh.jo.has_mi && self->qh.jo.mi >= pico8_fixed_from_int(2)) {
+    if (self->has_mi && self->mi >= pico8_fixed_from_int(2)) {
         pico8_ei(gfx, mem, (pico8_struct_header_t *)self);
     }
-    pico8_jo_kj(gfx, mem, (pico8_jo_t *)self, p);
+    pico8_jo_kj(gfx, mem, self, p);
 }
 
 void pico8_hj_kj_dispatch(void *self, const pico8_vec_t *ra) {
     pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
-    pico8_hj_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_hj_t *)self, p);
+    pico8_hj_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jo_t *)self, p);
 }
 
 static const pico8_struct_class_t g_hj_struct_class = {
@@ -219,24 +219,24 @@ static const pico8_struct_class_t g_hj_struct_class = {
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
-void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
-    pico8_qh_struct_init(&self->qh, cls, def);
-    self->qh.jo.header.emitter = &self->qh.jo.emitter_own;
-    self->qh.jo.emitter = &self->qh.jo.emitter_own;
-    self->qh.jo.emitter->def = &self->qh.jo.def->emitter;
-    self->qh.jo.emitter->py = self->qh.jo.def->emitter.py;
-    self->qh.jo.emitter->pn = self->qh.jo.def->emitter.pn;
-    self->qh.jo.emitter->k_ = self->qh.jo.def->emitter.k_;
-    self->qh.jo.emitter->du = self->qh.jo.def->emitter.du;
-    self->qh.jo.emitter->qo = self->qh.jo.def->emitter.qo;
-    self->qh.jo.emitter->ro = self->qh.jo.def->emitter.ro;
-    self->qh.jo.emitter->qg = self->qh.jo.def->emitter.qg;
-    self->qh.jo.emitter->q_ = self->qh.jo.def->emitter.q_;
-    self->qh.jo.emitter->qy = NULL;
-}
-
-static pico8_hj_t *pico8_hj_struct_new(const pico8_jo_keys_t *keys) {
-    pico8_hj_t *self = malloc(sizeof *self);
+void pico8_hj_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
+    pico8_qh_struct_init(self, cls, def);
+    self->header.emitter = &self->emitter_own;
+    self->emitter = &self->emitter_own;
+    self->emitter->def = &self->def->emitter;
+    self->emitter->py = self->def->emitter.py;
+    self->emitter->pn = self->def->emitter.pn;
+    self->emitter->k_ = self->def->emitter.k_;
+    self->emitter->du = self->def->emitter.du;
+    self->emitter->qo = self->def->emitter.qo;
+    self->emitter->ro = self->def->emitter.ro;
+    self->emitter->qg = self->def->emitter.qg;
+    self->emitter->q_ = self->def->emitter.q_;
+    self->emitter->qy = NULL;
+}
+
+static pico8_jo_t *pico8_hj_struct_new(const pico8_jo_keys_t *keys) {
+    pico8_jo_t *self = malloc(sizeof *self);
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
     pico8_hj_struct_init(self, &g_hj_struct_class, &pico8_hj_def);
@@ -245,32 +245,32 @@ static pico8_hj_t *pico8_hj_struct_new(const pico8_jo_keys_t *keys) {
                "hj: the only construction key is id (ie:ldown); another key has no member "
                "-- stop, do not add one");
         if (keys->id != NULL) {
-            self->qh.jo.id = keys->id;
+            self->id = keys->id;
         }
     }
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("hj");
 #endif
     pico8_f_((pico8_struct_header_t *)self);
-    pico8_jo_pq((pico8_jo_t *)self);
-    pico8_ke_struct_pq(&self->qh.jo.header);
+    pico8_jo_pq(self);
+    pico8_ke_struct_pq(&self->header);
     return self;
 }
 
-void pico8_pm_pq(pico8_pm_t *self) {
+void pico8_pm_pq(pico8_jo_t *self) {
     pico8_vec_t spawn_ra;
     pico8_vec_t *spawn_ra_heap;
     pico8_ga_keys_t keys;
-    pico8_ga_t *spawned;
+    pico8_jo_t *spawned;
     const pico8_vec_t *ra;
 
     if (self == NULL) {
         return;
     }
 
-    ra = self->hj.qh.jo.ra;
+    ra = self->ra;
     spawn_ra = pico8_vec_add(ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0)),
-                             self->hj.qh.jo.def->ng);
+                             self->def->ng);
     spawn_ra_heap = malloc(sizeof *spawn_ra_heap);
     *spawn_ra_heap = spawn_ra;
 
@@ -305,20 +305,20 @@ static const pico8_struct_class_t g_pm_struct_class = {
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
-static pico8_pm_t *pico8_pm_struct_new(const pico8_jo_keys_t *keys) {
-    pico8_pm_t *self = malloc(sizeof *self);
+static pico8_jo_t *pico8_pm_struct_new(const pico8_jo_keys_t *keys) {
+    pico8_jo_t *self = malloc(sizeof *self);
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
-    pico8_hj_struct_init(&self->hj, &g_pm_struct_class, &pico8_pm_def);
+    pico8_hj_struct_init(self, &g_pm_struct_class, &pico8_pm_def);
     if (keys != NULL) {
         if (keys->p != NULL) {
-            self->hj.qh.jo.p = keys->p;
+            self->p = keys->p;
         }
         if (keys->ra != NULL) {
-            self->hj.qh.jo.ra = keys->ra;
+            self->ra = keys->ra;
         }
         if (keys->id != NULL) {
-            self->hj.qh.jo.id = keys->id;
+            self->id = keys->id;
         }
     }
 #ifdef PICO8_CLASS_COVERAGE
@@ -326,12 +326,12 @@ static pico8_pm_t *pico8_pm_struct_new(const pico8_jo_keys_t *keys) {
 #endif
     pico8_pm_pq(self);
     pico8_f_((pico8_struct_header_t *)self);
-    pico8_jo_pq((pico8_jo_t *)self);
-    pico8_ke_struct_pq(&self->hj.qh.jo.header);
+    pico8_jo_pq(self);
+    pico8_ke_struct_pq(&self->header);
     return self;
 }
 
-void pico8_ga_pq(pico8_ga_t *self) {
+void pico8_ga_pq(pico8_jo_t *self) {
     pico8_qb_t *mo;
 
     if (self == NULL) {
@@ -348,7 +348,7 @@ void pico8_ga_pq(pico8_ga_t *self) {
 }
 
 pico8_qy_t *pico8_ga_mm(void *self_ptr) {
-    const pico8_ga_t *self = (const pico8_ga_t *)self_ptr;
+    const pico8_jo_t *self = (const pico8_jo_t *)self_ptr;
     pico8_fixed_t qd;
     pico8_fixed_t ms;
     pico8_fixed_t iu;
@@ -365,9 +365,9 @@ pico8_qy_t *pico8_ga_mm(void *self_ptr) {
     }
     p = pico8_vec_qm(qd, pico8_fixed_rnd_default());
 
-    ms = self->jo.ms;
-    iu = self->jo.iu;
-    py = self->jo.emitter->py;
+    ms = self->ms;
+    iu = self->iu;
+    py = self->emitter->py;
 
     v = pico8_vec_mul(p, ms);
     a = pico8_vec_mul(p, iu);
@@ -382,7 +382,7 @@ pico8_qy_t *pico8_ga_mm(void *self_ptr) {
     return particle;
 }
 
-void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ga_t *self, pico8_vec_t p) {
+void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec_t p) {
     const pico8_ptrs_t *qy = NULL;
     const pico8_qy_t *last_pt = NULL;
     int i;
@@ -393,7 +393,7 @@ void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ga_t *self, pico8_vec
         return;
     }
 
-    qy = self->jo.emitter->qy;
+    qy = self->emitter->qy;
 
     px = pico8_fixed_flr_int(p.x);
     py = pico8_fixed_flr_int(p.y);
@@ -425,12 +425,12 @@ void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ga_t *self, pico8_vec
     }
 
     pico8_ei(gfx, mem, (pico8_struct_header_t *)self);
-    self->jo.p_ = last_pt == NULL;
+    self->p_ = last_pt == NULL;
 }
 
 static void pico8_ga_kj_dispatch(void *self, const pico8_vec_t *ra) {
     pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
-    pico8_ga_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_ga_t *)self, p);
+    pico8_ga_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jo_t *)self, p);
 }
 
 static const pico8_struct_class_t g_ga_struct_class = {
@@ -449,51 +449,51 @@ static const pico8_struct_class_t g_ga_struct_class = {
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO),
 };
 
-static pico8_ga_t *pico8_ga_struct_new(const pico8_ga_keys_t *keys) {
-    pico8_ga_t *self = malloc(sizeof *self);
+static pico8_jo_t *pico8_ga_struct_new(const pico8_ga_keys_t *keys) {
+    pico8_jo_t *self = malloc(sizeof *self);
 
     assert(g_family_up && "pico8_planet_family_init() must run first");
-    pico8_jo_struct_init(&self->jo, &g_ga_struct_class, &pico8_ga_def);
-    self->jo.header.emitter = &self->jo.emitter_own;
-    self->jo.emitter = &self->jo.emitter_own;
-    self->jo.emitter->def = &self->jo.def->emitter;
-    self->jo.emitter->qo = self->jo.def->emitter.qo;
-    self->jo.emitter->ro = self->jo.def->emitter.ro;
-    self->jo.emitter->qg = self->jo.def->emitter.qg;
-    self->jo.emitter->q_ = self->jo.def->emitter.q_;
-    self->jo.emitter->k_ = self->jo.def->emitter.k_;
-    self->jo.iu = self->jo.def->iu;
-    self->jo.emitter->py = self->jo.def->emitter.py;
-    self->jo.ms = self->jo.def->ms;
-    self->jo.emitter->qy = NULL;
-    self->jo.by = &g_ga_by_shared;
+    pico8_jo_struct_init(self, &g_ga_struct_class, &pico8_ga_def);
+    self->header.emitter = &self->emitter_own;
+    self->emitter = &self->emitter_own;
+    self->emitter->def = &self->def->emitter;
+    self->emitter->qo = self->def->emitter.qo;
+    self->emitter->ro = self->def->emitter.ro;
+    self->emitter->qg = self->def->emitter.qg;
+    self->emitter->q_ = self->def->emitter.q_;
+    self->emitter->k_ = self->def->emitter.k_;
+    self->iu = self->def->iu;
+    self->emitter->py = self->def->emitter.py;
+    self->ms = self->def->ms;
+    self->emitter->qy = NULL;
+    self->by = &g_ga_by_shared;
     if (keys != NULL) {
         if (keys->jo.p != NULL) {
-            self->jo.p = keys->jo.p;
+            self->p = keys->jo.p;
         }
         if (keys->jo.ra != NULL) {
-            self->jo.ra = keys->jo.ra;
+            self->ra = keys->jo.ra;
         }
         if (keys->jo.id != NULL) {
-            self->jo.id = keys->jo.id;
+            self->id = keys->jo.id;
         }
         if (keys->pq != NULL) {
-            self->jo.emitter->qo = keys->pq->qo;
-            self->jo.emitter->ro = keys->pq->ro;
-            self->jo.emitter->qg = keys->pq->qg;
-            self->jo.emitter->q_ = keys->pq->q_;
-            self->jo.emitter->k_ = keys->pq->k_;
-            self->jo.iu = keys->pq->iu;
-            self->jo.emitter->py = keys->pq->py;
-            self->jo.ms = keys->pq->ms;
+            self->emitter->qo = keys->pq->qo;
+            self->emitter->ro = keys->pq->ro;
+            self->emitter->qg = keys->pq->qg;
+            self->emitter->q_ = keys->pq->q_;
+            self->emitter->k_ = keys->pq->k_;
+            self->iu = keys->pq->iu;
+            self->emitter->py = keys->pq->py;
+            self->ms = keys->pq->ms;
         }
     }
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("ga");
 #endif
     pico8_ga_pq(self);
-    pico8_jo_pq((pico8_jo_t *)self);
-    pico8_ke_struct_pq(&self->jo.header);
+    pico8_jo_pq(self);
+    pico8_ke_struct_pq(&self->header);
     return self;
 }
 
@@ -514,7 +514,7 @@ int pico8_qh_ready(void) {
 }
 
 static pico8_struct_header_t *pico8_qh_kind_new(const pico8_jo_keys_t *keys) {
-    return &pico8_qh_struct_new(keys)->jo.header;
+    return &pico8_qh_struct_new(keys)->header;
 }
 
 static const pico8_entity_kind_t g_qh_kind = {pico8_qh_kind_new, &pico8_qh_kind_rec};
@@ -527,7 +527,7 @@ const pico8_struct_class_t *pico8_qh_struct_class(void) {
     return &g_qh_struct_class;
 }
 
-pico8_qh_t *pico8_qh_new(const pico8_jo_keys_t *keys) {
+pico8_jo_t *pico8_qh_new(const pico8_jo_keys_t *keys) {
     return pico8_qh_struct_new(keys);
 }
 
@@ -536,7 +536,7 @@ int pico8_nl_ready(void) {
 }
 
 static pico8_struct_header_t *pico8_nl_kind_new(const pico8_jo_keys_t *keys) {
-    return &pico8_nl_struct_new(keys)->qh.jo.header;
+    return &pico8_nl_struct_new(keys)->header;
 }
 
 static const pico8_entity_kind_t g_nl_kind = {pico8_nl_kind_new, &pico8_nl_kind_rec};
@@ -549,7 +549,7 @@ const pico8_struct_class_t *pico8_nl_struct_class(void) {
     return &g_nl_struct_class;
 }
 
-pico8_nl_t *pico8_nl_new(const pico8_jo_keys_t *keys) {
+pico8_jo_t *pico8_nl_new(const pico8_jo_keys_t *keys) {
     return pico8_nl_struct_new(keys);
 }
 
@@ -558,7 +558,7 @@ int pico8_os_ready(void) {
 }
 
 static pico8_struct_header_t *pico8_os_kind_new(const pico8_jo_keys_t *keys) {
-    return &pico8_os_struct_new(keys)->qh.jo.header;
+    return &pico8_os_struct_new(keys)->header;
 }
 
 static const pico8_entity_kind_t g_os_kind = {pico8_os_kind_new, &pico8_os_kind_rec};
@@ -571,7 +571,7 @@ const pico8_struct_class_t *pico8_os_struct_class(void) {
     return &g_os_struct_class;
 }
 
-pico8_os_t *pico8_os_new(const pico8_jo_keys_t *keys) {
+pico8_jo_t *pico8_os_new(const pico8_jo_keys_t *keys) {
     return pico8_os_struct_new(keys);
 }
 
@@ -580,7 +580,7 @@ int pico8_hj_ready(void) {
 }
 
 static pico8_struct_header_t *pico8_hj_kind_new(const pico8_jo_keys_t *keys) {
-    return &pico8_hj_struct_new(keys)->qh.jo.header;
+    return &pico8_hj_struct_new(keys)->header;
 }
 
 static const pico8_entity_kind_t g_hj_kind = {pico8_hj_kind_new, &pico8_hj_kind_rec};
@@ -593,7 +593,7 @@ const pico8_struct_class_t *pico8_hj_struct_class(void) {
     return &g_hj_struct_class;
 }
 
-pico8_hj_t *pico8_hj_new(const pico8_jo_keys_t *keys) {
+pico8_jo_t *pico8_hj_new(const pico8_jo_keys_t *keys) {
     return pico8_hj_struct_new(keys);
 }
 
@@ -602,7 +602,7 @@ int pico8_pm_ready(void) {
 }
 
 static pico8_struct_header_t *pico8_pm_kind_new(const pico8_jo_keys_t *keys) {
-    return &pico8_pm_struct_new(keys)->hj.qh.jo.header;
+    return &pico8_pm_struct_new(keys)->header;
 }
 
 static const pico8_entity_kind_t g_pm_kind = {pico8_pm_kind_new, &pico8_pm_kind_rec};
@@ -615,7 +615,7 @@ const pico8_struct_class_t *pico8_pm_struct_class(void) {
     return &g_pm_struct_class;
 }
 
-pico8_pm_t *pico8_pm_new(const pico8_jo_keys_t *keys) {
+pico8_jo_t *pico8_pm_new(const pico8_jo_keys_t *keys) {
     return pico8_pm_struct_new(keys);
 }
 
@@ -628,11 +628,11 @@ static pico8_struct_header_t *pico8_ga_kind_new(const pico8_jo_keys_t *keys) {
     pico8_ga_keys_t ga_keys;
 
     if (keys == NULL) {
-        return &pico8_ga_struct_new(NULL)->jo.header;
+        return &pico8_ga_struct_new(NULL)->header;
     }
     ga_keys.jo = *keys;
     ga_keys.pq = NULL;
-    return &pico8_ga_struct_new(&ga_keys)->jo.header;
+    return &pico8_ga_struct_new(&ga_keys)->header;
 }
 
 static const pico8_entity_kind_t g_ga_kind = {pico8_ga_kind_new, &pico8_ga_kind_rec};
@@ -645,6 +645,6 @@ const pico8_struct_class_t *pico8_ga_struct_class(void) {
     return &g_ga_struct_class;
 }
 
-pico8_ga_t *pico8_ga_new(const pico8_ga_keys_t *keys) {
+pico8_jo_t *pico8_ga_new(const pico8_ga_keys_t *keys) {
     return pico8_ga_struct_new(keys);
 }
diff --git a/src/pico8_planet_family.h b/src/pico8_planet_family.h
index e998bac..3b69ffb 100644
--- a/src/pico8_planet_family.h
+++ b/src/pico8_planet_family.h
@@ -22,66 +22,29 @@ void pico8_planet_family_init(void);
  * before the first `pico8_planet_family_init()`. */
 void pico8_planet_family_shutdown(void);
 
-/* A laboratory: a planet whose `s` and probe are copied from its kind's
- * archetype, the probe's with `et`, which its own `hq` (`bv`) writes.
- * Nothing frees the struct: TROU-7. */
-typedef struct {
-    pico8_jo_t jo;
-} pico8_qh_t;
-
-/* A trading hub: a laboratory whose `qw` is copied from its archetype and
- * written by its own `ly`. Nothing frees the struct: TROU-7. */
-typedef struct {
-    pico8_qh_t qh;
-} pico8_nl_t;
-
-/* A processor: a laboratory and nothing of its own; its defaults are its
- * archetype's. Nothing frees the struct: TROU-7. */
-typedef struct {
-    pico8_qh_t qh;
-} pico8_os_t;
-
-/* An ascension: a laboratory whose emitter is copied from its kind's
- * archetype, with `qy`, which its own `pq` (`f_`) writes. Nothing frees the
- * struct: TROU-7. */
-typedef struct {
-    pico8_qh_t qh;
-} pico8_hj_t;
-
-/* A protostar: an ascension and nothing of its own; its defaults are its
- * archetype's. Nothing frees the struct: TROU-7. */
-typedef struct {
-    pico8_hj_t hj;
-} pico8_pm_t;
-
-/* A particle burst: a planet whose `iu`, `ms` and emitter are copied from
- * its archetype, with `qy`, which its own `pq` (`f_`) writes; the emitter's
- * `pn`, `du` are left unwritten, read by `g_:mm` only. `jo.by` is the one list
- * every burst shares, which `ib` extends in place and each init empties. A
- * burst spawned by `pm:pq` takes `iu`, `ms`, `py`, `k_`, `qo`, `ro`, `qg`, `q_`
- * from its keys instead. Nothing frees the struct: TROU-7. */
-typedef struct {
-    pico8_jo_t jo;
-} pico8_ga_t;
-
 /* Every `_ready` below is 1 from `pico8_planet_family_init()` to
  * `pico8_planet_family_shutdown()`, 0 otherwise. A `_kind` accessor is never
  * NULL. */
+
+/* A laboratory: a planet whose `s` and probe are copied from its kind's
+ * archetype, the probe's with `et`, which its own `hq` (`bv`) writes. */
 int pico8_qh_ready(void);
 const pico8_entity_kind_t *pico8_qh_kind(void);
 const pico8_struct_class_t *pico8_qh_struct_class(void);
 
-/* Initializes an allocated pico8_qh_t under `cls` and `def`, its kind's
+/* Initializes an allocated pico8_jo_t under `cls` and `def`, its kind's
  * archetype: jo's part, then `s` and the probe copied from `def`. Reads no
  * key, runs neither `pq` nor `lt`, records no coverage. Requires
  * `pico8_planet_family_init()`. */
-void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def);
+void pico8_qh_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def);
 
 /* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
  * NULL. The new laboratory registers itself into `hv`/`cv`/`cg` through `lt`, and nothing
  * ever frees it. Requires `pico8_planet_family_init()`. */
-pico8_qh_t *pico8_qh_new(const pico8_jo_keys_t *keys);
+pico8_jo_t *pico8_qh_new(const pico8_jo_keys_t *keys);
 
+/* A trading hub: a laboratory whose `qw` is copied from its archetype and
+ * written by its own `ly`. */
 int pico8_nl_ready(void);
 const pico8_entity_kind_t *pico8_nl_kind(void);
 const pico8_struct_class_t *pico8_nl_struct_class(void);
@@ -89,8 +52,10 @@ const pico8_struct_class_t *pico8_nl_struct_class(void);
 /* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
  * NULL. The new trading hub registers itself into `hv`/`cv`/`cg` through `lt`, and
  * nothing ever frees it. Requires `pico8_planet_family_init()`. */
-pico8_nl_t *pico8_nl_new(const pico8_jo_keys_t *keys);
+pico8_jo_t *pico8_nl_new(const pico8_jo_keys_t *keys);
 
+/* A processor: a laboratory and nothing of its own; its defaults are its
+ * archetype's. */
 int pico8_os_ready(void);
 const pico8_entity_kind_t *pico8_os_kind(void);
 const pico8_struct_class_t *pico8_os_struct_class(void);
@@ -98,8 +63,10 @@ const pico8_struct_class_t *pico8_os_struct_class(void);
 /* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
  * NULL. The new processor registers itself into `hv`/`cv`/`cg` through `lt`, and nothing
  * ever frees it. Requires `pico8_planet_family_init()`. */
-pico8_os_t *pico8_os_new(const pico8_jo_keys_t *keys);
+pico8_jo_t *pico8_os_new(const pico8_jo_keys_t *keys);
 
+/* An ascension: a laboratory whose emitter is copied from its kind's
+ * archetype, with `qy`, which its own `pq` (`f_`) writes. */
 int pico8_hj_ready(void);
 const pico8_entity_kind_t *pico8_hj_kind(void);
 const pico8_struct_class_t *pico8_hj_struct_class(void);
@@ -107,13 +74,13 @@ const pico8_struct_class_t *pico8_hj_struct_class(void);
 /* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
  * NULL. The new ascension registers itself into `hv`/`cv`/`cg` through `lt`, and nothing
  * ever frees it. Requires `pico8_planet_family_init()`. */
-pico8_hj_t *pico8_hj_new(const pico8_jo_keys_t *keys);
+pico8_jo_t *pico8_hj_new(const pico8_jo_keys_t *keys);
 
-/* Initializes an allocated pico8_hj_t under `cls` and `def`, its kind's
+/* Initializes an allocated pico8_jo_t under `cls` and `def`, its kind's
  * archetype: qh's part, then the emitter copied from `def`, `qy` nil. Reads
  * no key, runs neither `pq` nor `lt`, records no coverage. Requires
  * `pico8_planet_family_init()`. */
-void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def);
+void pico8_hj_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def);
 
 /* The dispatchers hj's descriptor holds, for a daughter's `static const`
  * descriptor to name. */
@@ -122,6 +89,9 @@ void pico8_hj_kj_dispatch(void *self, const pico8_vec_t *ra);
 
 /* `pm` and `ga` are the only classes here whose spec sets `fz`: the two
  * kinds `jo:hu`'s `kv[hp.cw]` can resolve. */
+
+/* A protostar: an ascension and nothing of its own; its defaults are its
+ * archetype's. */
 int pico8_pm_ready(void);
 const pico8_entity_kind_t *pico8_pm_kind(void);
 const pico8_struct_class_t *pico8_pm_struct_class(void);
@@ -130,8 +100,14 @@ const pico8_struct_class_t *pico8_pm_struct_class(void);
  * spawns its `ga` at `ra + ng`, then registers itself into `hv`/`cv`/`cg`
  * through `lt`, after that `ga`; nothing ever frees it. Requires
  * `pico8_planet_family_init()`. */
-pico8_pm_t *pico8_pm_new(const pico8_jo_keys_t *keys);
+pico8_jo_t *pico8_pm_new(const pico8_jo_keys_t *keys);
 
+/* A particle burst: a planet whose `iu`, `ms` and emitter are copied from
+ * its archetype, with `qy`, which its own `pq` (`f_`) writes; the emitter's
+ * `pn`, `du` are left unwritten, read by `g_:mm` only. `by` is the one list
+ * every burst shares, which `ib` extends in place and each init empties. A
+ * burst spawned by `pm:pq` takes `iu`, `ms`, `py`, `k_`, `qo`, `ro`, `qg`,
+ * `q_` from its keys instead. */
 int pico8_ga_ready(void);
 const pico8_entity_kind_t *pico8_ga_kind(void);
 const pico8_struct_class_t *pico8_ga_struct_class(void);
@@ -146,22 +122,22 @@ typedef struct {
 /* `keys` (NULL means none) is read, not kept. The new burst registers
  * itself into `hv`/`cv`, in no `cg`, through `lt`, and nothing ever frees
  * it. Requires `pico8_planet_family_init()`. */
-pico8_ga_t *pico8_ga_new(const pico8_ga_keys_t *keys);
+pico8_jo_t *pico8_ga_new(const pico8_ga_keys_t *keys);
 
-void pico8_nl_ly(pico8_nl_t *self, pico8_fixed_t t);
+void pico8_nl_ly(pico8_jo_t *self, pico8_fixed_t t);
 
-void pico8_hj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_hj_t *self, pico8_vec_t p);
+void pico8_hj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec_t p);
 
 /* TROU-7 */
-void pico8_pm_pq(pico8_pm_t *self);
+void pico8_pm_pq(pico8_jo_t *self);
 
-void pico8_ga_pq(pico8_ga_t *self);
+void pico8_ga_pq(pico8_jo_t *self);
 
 /* TROU-7 */
 /* Returns a freshly allocated `pico8_qy_t` (`has_a` 1), one allocation,
  * which `ez` stores into `self.qy`. */
 pico8_qy_t *pico8_ga_mm(void *self);
 
-void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ga_t *self, pico8_vec_t p);
+void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec_t p);
 
 #endif
diff --git a/src/pico8_planet_family_trade.c b/src/pico8_planet_family_trade.c
index ce5ce11..2383658 100644
--- a/src/pico8_planet_family_trade.c
+++ b/src/pico8_planet_family_trade.c
@@ -20,7 +20,7 @@ static const pico8_int_seq_t g_mx_ht_spent = {0, NULL};
 
 static int g_family_trade_up = 0;
 
-pico8_down_t pico8_nr_gv(const pico8_nr_t *self, const pico8_struct_header_t *s) {
+pico8_down_t pico8_nr_gv(const pico8_jo_t *self, const pico8_struct_header_t *s) {
     int qu_not_self;
     int oe_count = 0;
     pico8_down_t out = {0, NULL};
@@ -31,16 +31,16 @@ pico8_down_t pico8_nr_gv(const pico8_nr_t *self, const pico8_struct_header_t *s)
 
     /* Only a held `jc` carries `qu`: a held `bg`, `mz` or planet reads it absent. */
     qu_not_self = !(s != NULL && s->cls == pico8_jc_struct_class() &&
-                    ((const pico8_jc_t *)s)->qu == &self->qh.jo.header);
+                    ((const pico8_jc_t *)s)->qu == &self->header);
 
-    oe_count = self->qh.jo.oe != NULL ? self->qh.jo.oe->count : 0;
+    oe_count = self->oe != NULL ? self->oe->count : 0;
 
     out.stop = qu_not_self && oe_count < 3;
     return out;
 }
 
 static pico8_down_t pico8_nr_gv_dispatch(void *self, const pico8_struct_header_t *s) {
-    return pico8_nr_gv((const pico8_nr_t *)self, s);
+    return pico8_nr_gv((const pico8_jo_t *)self, s);
 }
 
 static const pico8_struct_class_t g_nr_ldown_empty_struct_class = {0};
@@ -53,7 +53,7 @@ typedef struct {
  * never written. */
 static const pico8_nr_ldown_empty_t g_nr_ldown_empty = {{.cls = &g_nr_ldown_empty_struct_class}};
 
-pico8_down_t pico8_nr_ldown(pico8_nr_t *self) {
+pico8_down_t pico8_nr_ldown(pico8_jo_t *self) {
     pico8_down_t gv_result = {0, NULL};
     pico8_jc_keys_t keys = {0};
 
@@ -77,10 +77,10 @@ pico8_down_t pico8_nr_ldown(pico8_nr_t *self) {
 static pico8_down_t pico8_nr_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
     (void)qa;
     (void)ra;
-    return pico8_nr_ldown((pico8_nr_t *)self);
+    return pico8_nr_ldown((pico8_jo_t *)self);
 }
 
-void pico8_nr_hb(pico8_nr_t *self) {
+void pico8_nr_hb(pico8_jo_t *self) {
     int oe_count = 0;
     pico8_fixed_t s_val = pico8_fixed_from_int(0);
 
@@ -88,23 +88,23 @@ void pico8_nr_hb(pico8_nr_t *self) {
         return;
     }
 
-    oe_count = self->qh.jo.oe != NULL ? self->qh.jo.oe->count : 0;
+    oe_count = self->oe != NULL ? self->oe->count : 0;
 
-    if (self->qh.jo.def->hz != NULL) {
+    if (self->def->hz != NULL) {
         int entry;
-        if (pico8_int_seq_at(self->qh.jo.def->hz->items, self->qh.jo.def->hz->count, oe_count + 1, &entry)) {
+        if (pico8_int_seq_at(self->def->hz->items, self->def->hz->count, oe_count + 1, &entry)) {
             s_val = pico8_fixed_from_int(entry);
         }
     }
 
-    self->qh.jo.s = s_val;
+    self->s = s_val;
 }
 
 static void pico8_nr_hb_dispatch(void *self) {
-    pico8_nr_hb((pico8_nr_t *)self);
+    pico8_nr_hb((pico8_jo_t *)self);
 }
 
-void pico8_nr_cn(pico8_nr_t *self, pico8_jo_trade_proposal_t *lw) {
+void pico8_nr_cn(pico8_jo_t *self, pico8_jo_trade_proposal_t *lw) {
     pico8_trade_t *t;
     const pico8_ents_t *oe = NULL;
     pico8_ents_iter_t it;
@@ -115,7 +115,7 @@ void pico8_nr_cn(pico8_nr_t *self, pico8_jo_trade_proposal_t *lw) {
     }
     t = lw->t;
 
-    oe = self->qh.jo.oe;
+    oe = self->oe;
 
     it = pico8_ents_all(oe);
     while (pico8_ents_iter_next(&it, &w)) {
@@ -138,7 +138,7 @@ void pico8_nr_cn(pico8_nr_t *self, pico8_jo_trade_proposal_t *lw) {
 
         {
             pico8_jo_trade_proposal_t gf = *lw;
-            pico8_trade_t *extended = pico8_trade_kg(t, &self->qh.jo.header, w);
+            pico8_trade_t *extended = pico8_trade_kg(t, &self->header, w);
             pico8_struct_header_t *other;
 
             gf.t = extended;
@@ -155,7 +155,7 @@ void pico8_nr_cn(pico8_nr_t *self, pico8_jo_trade_proposal_t *lw) {
 }
 
 static void pico8_nr_cn_dispatch(void *self, struct pico8_jo_trade_proposal *lw) {
-    pico8_nr_cn((pico8_nr_t *)self, lw);
+    pico8_nr_cn((pico8_jo_t *)self, lw);
 }
 
 static const pico8_struct_class_t g_nr_struct_class = {
@@ -179,35 +179,35 @@ static const pico8_struct_class_t g_nr_struct_class = {
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
-/* Initializes an allocated pico8_nr_t under `cls` and `def`, its kind's
+/* Initializes an allocated pico8_jo_t under `cls` and `def`, its kind's
  * archetype: qh's part, nr having no member of its own. Reads no key, runs
  * neither `pq` nor `lt`, records no coverage. Requires
  * `pico8_planet_family_trade_init()`. */
-static void pico8_nr_struct_init(pico8_nr_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
-    pico8_qh_struct_init(&self->qh, cls, def);
+static void pico8_nr_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
+    pico8_qh_struct_init(self, cls, def);
 }
 
-static pico8_nr_t *pico8_nr_struct_new(const pico8_jo_keys_t *keys) {
-    pico8_nr_t *self = malloc(sizeof *self);
+static pico8_jo_t *pico8_nr_struct_new(const pico8_jo_keys_t *keys) {
+    pico8_jo_t *self = malloc(sizeof *self);
 
     assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
     pico8_nr_struct_init(self, &g_nr_struct_class, &pico8_nr_def);
     if (keys != NULL) {
         if (keys->id != NULL) {
-            self->qh.jo.id = keys->id;
+            self->id = keys->id;
         }
         if (keys->p != NULL) {
-            self->qh.jo.p = keys->p;
+            self->p = keys->p;
         }
         if (keys->ra != NULL) {
-            self->qh.jo.ra = keys->ra;
+            self->ra = keys->ra;
         }
     }
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("nr");
 #endif
-    pico8_jo_pq((pico8_jo_t *)self);
-    pico8_ke_struct_pq(&self->qh.jo.header);
+    pico8_jo_pq(self);
+    pico8_ke_struct_pq(&self->header);
     return self;
 }
 
@@ -232,29 +232,29 @@ static const pico8_struct_class_t g_fr_struct_class = {
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
-static pico8_fr_t *pico8_fr_struct_new(const pico8_jo_keys_t *keys) {
-    pico8_fr_t *self = malloc(sizeof *self);
+static pico8_jo_t *pico8_fr_struct_new(const pico8_jo_keys_t *keys) {
+    pico8_jo_t *self = malloc(sizeof *self);
 
     assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
-    pico8_nr_struct_init(&self->nr, &g_fr_struct_class, &pico8_fr_def);
-    self->nr.qh.jo.header.mt = &self->nr.qh.jo.def->mt;
+    pico8_nr_struct_init(self, &g_fr_struct_class, &pico8_fr_def);
+    self->header.mt = &self->def->mt;
     if (keys != NULL) {
         assert(keys->p == NULL && keys->ra == NULL &&
                "fr: the only construction key is id (ie:ldown); another key has no member "
                "-- stop, do not add one");
         if (keys->id != NULL) {
-            self->nr.qh.jo.id = keys->id;
+            self->id = keys->id;
         }
     }
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("fr");
 #endif
-    pico8_jo_pq((pico8_jo_t *)self);
-    pico8_ke_struct_pq(&self->nr.qh.jo.header);
+    pico8_jo_pq(self);
+    pico8_ke_struct_pq(&self->header);
     return self;
 }
 
-void pico8_mx_kf(pico8_mx_t *self, pico8_struct_header_t *sw) {
+void pico8_mx_kf(pico8_jo_t *self, pico8_struct_header_t *sw) {
     pico8_struct_header_t *cp = NULL;
 
     if (self != NULL && sw != NULL) {
@@ -285,22 +285,22 @@ void pico8_mx_kf(pico8_mx_t *self, pico8_struct_header_t *sw) {
                 continue;
             }
 
-            if (i->row.has_qe && self->hj.qh.jo.ht != NULL && pico8_int_seq_has_fixed(self->hj.qh.jo.ht, i->row.qe)) {
-                pico8_kw_t *kw = self->hj.qh.jo.kw;
+            if (i->row.has_qe && self->ht != NULL && pico8_int_seq_has_fixed(self->ht, i->row.qe)) {
+                pico8_kw_t *kw = self->kw;
                 if (kw != NULL) {
                     kw->has_qe = 1;
                     kw->qe = i->row.qe;
                 }
-                self->hj.qh.jo.ht = &g_mx_ht_spent;
+                self->ht = &g_mx_ht_spent;
             }
         }
     }
 
-    pico8_jo_kf((pico8_jo_t *)self, sw);
+    pico8_jo_kf(self, sw);
 }
 
 static void pico8_mx_kf_dispatch(void *self, pico8_struct_header_t *sw) {
-    pico8_mx_kf((pico8_mx_t *)self, sw);
+    pico8_mx_kf((pico8_jo_t *)self, sw);
 }
 
 static const pico8_struct_class_t g_mx_struct_class = {
@@ -325,26 +325,26 @@ static const pico8_struct_class_t g_mx_struct_class = {
     .lineage = PICO8_LINEAGE(PICO8_LINEAGE_JO) | PICO8_LINEAGE(PICO8_LINEAGE_QH),
 };
 
-static pico8_mx_t *pico8_mx_struct_new(const pico8_jo_keys_t *keys) {
-    pico8_mx_t *self = malloc(sizeof *self);
+static pico8_jo_t *pico8_mx_struct_new(const pico8_jo_keys_t *keys) {
+    pico8_jo_t *self = malloc(sizeof *self);
 
     assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
-    pico8_hj_struct_init(&self->hj, &g_mx_struct_class, &pico8_mx_def);
-    self->hj.qh.jo.ht = self->hj.qh.jo.def->ht;
+    pico8_hj_struct_init(self, &g_mx_struct_class, &pico8_mx_def);
+    self->ht = self->def->ht;
     if (keys != NULL) {
         assert(keys->p == NULL && keys->ra == NULL &&
                "mx: the only construction key is id (ie:ldown); another key has no member "
                "-- stop, do not add one");
         if (keys->id != NULL) {
-            self->hj.qh.jo.id = keys->id;
+            self->id = keys->id;
         }
     }
 #ifdef PICO8_CLASS_COVERAGE
     pico8_class_coverage_record("mx");
 #endif
     pico8_f_((pico8_struct_header_t *)self);
-    pico8_jo_pq((pico8_jo_t *)self);
-    pico8_ke_struct_pq(&self->hj.qh.jo.header);
+    pico8_jo_pq(self);
+    pico8_ke_struct_pq(&self->header);
     return self;
 }
 
@@ -365,7 +365,7 @@ int pico8_nr_ready(void) {
 }
 
 static pico8_struct_header_t *pico8_nr_kind_new(const pico8_jo_keys_t *keys) {
-    return &pico8_nr_struct_new(keys)->qh.jo.header;
+    return &pico8_nr_struct_new(keys)->header;
 }
 
 static const pico8_entity_kind_t g_nr_kind = {pico8_nr_kind_new, &pico8_nr_kind_rec};
@@ -378,7 +378,7 @@ const pico8_struct_class_t *pico8_nr_struct_class(void) {
     return &g_nr_struct_class;
 }
 
-pico8_nr_t *pico8_nr_new(const pico8_jo_keys_t *keys) {
+pico8_jo_t *pico8_nr_new(const pico8_jo_keys_t *keys) {
     return pico8_nr_struct_new(keys);
 }
 
@@ -387,7 +387,7 @@ int pico8_fr_ready(void) {
 }
 
 static pico8_struct_header_t *pico8_fr_kind_new(const pico8_jo_keys_t *keys) {
-    return &pico8_fr_struct_new(keys)->nr.qh.jo.header;
+    return &pico8_fr_struct_new(keys)->header;
 }
 
 static const pico8_entity_kind_t g_fr_kind = {pico8_fr_kind_new, &pico8_fr_kind_rec};
@@ -400,7 +400,7 @@ const pico8_struct_class_t *pico8_fr_struct_class(void) {
     return &g_fr_struct_class;
 }
 
-pico8_fr_t *pico8_fr_new(const pico8_jo_keys_t *keys) {
+pico8_jo_t *pico8_fr_new(const pico8_jo_keys_t *keys) {
     return pico8_fr_struct_new(keys);
 }
 
@@ -409,7 +409,7 @@ int pico8_mx_ready(void) {
 }
 
 static pico8_struct_header_t *pico8_mx_kind_new(const pico8_jo_keys_t *keys) {
-    return &pico8_mx_struct_new(keys)->hj.qh.jo.header;
+    return &pico8_mx_struct_new(keys)->header;
 }
 
 static const pico8_entity_kind_t g_mx_kind = {pico8_mx_kind_new, &pico8_mx_kind_rec};
@@ -422,6 +422,6 @@ const pico8_struct_class_t *pico8_mx_struct_class(void) {
     return &g_mx_struct_class;
 }
 
-pico8_mx_t *pico8_mx_new(const pico8_jo_keys_t *keys) {
+pico8_jo_t *pico8_mx_new(const pico8_jo_keys_t *keys) {
     return pico8_mx_struct_new(keys);
 }
diff --git a/src/pico8_planet_family_trade.h b/src/pico8_planet_family_trade.h
index ced2f16..be2cedf 100644
--- a/src/pico8_planet_family_trade.h
+++ b/src/pico8_planet_family_trade.h
@@ -14,27 +14,11 @@ void pico8_planet_family_trade_init(void);
  * `pico8_planet_family_trade_init()`. */
 void pico8_planet_family_trade_shutdown(void);
 
-/* A slipgate: a laboratory and nothing of its own; its defaults are its
- * archetype's. Nothing frees the struct: TROU-7. */
-typedef struct {
-    pico8_qh_t qh;
-} pico8_nr_t;
-
-/* An infraspace gate: a slipgate and nothing of its own; its defaults are
- * its archetype's. Nothing frees the struct: TROU-7. */
-typedef struct {
-    pico8_nr_t nr;
-} pico8_fr_t;
-
-/* A synthesizer: an ascension whose `ht` is copied from its archetype;
- * `mx:kf` points it at an empty sequence once it accepts a resource.
- * Nothing frees the struct: TROU-7. */
-typedef struct {
-    pico8_hj_t hj;
-} pico8_mx_t;
-
 /* As in pico8_planet_family.h: `_ready` from
  * `pico8_planet_family_trade_init()` to its shutdown. */
+
+/* A slipgate: a laboratory and nothing of its own; its defaults are its
+ * archetype's. */
 int pico8_nr_ready(void);
 const pico8_entity_kind_t *pico8_nr_kind(void);
 const pico8_struct_class_t *pico8_nr_struct_class(void);
@@ -42,8 +26,10 @@ const pico8_struct_class_t *pico8_nr_struct_class(void);
 /* `keys` (NULL means none) is read, not kept. The new slipgate registers
  * itself into `hv`/`cv`/`cg` through `lt`, and nothing ever frees it.
  * Requires `pico8_planet_family_trade_init()`. */
-pico8_nr_t *pico8_nr_new(const pico8_jo_keys_t *keys);
+pico8_jo_t *pico8_nr_new(const pico8_jo_keys_t *keys);
 
+/* An infraspace gate: a slipgate and nothing of its own; its defaults are
+ * its archetype's. */
 int pico8_fr_ready(void);
 const pico8_entity_kind_t *pico8_fr_kind(void);
 const pico8_struct_class_t *pico8_fr_struct_class(void);
@@ -51,8 +37,10 @@ const pico8_struct_class_t *pico8_fr_struct_class(void);
 /* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
  * NULL. The new infraspace gate registers itself into `hv`/`cv`/`cg` through
  * `lt`, and nothing ever frees it. Requires `pico8_planet_family_trade_init()`. */
-pico8_fr_t *pico8_fr_new(const pico8_jo_keys_t *keys);
+pico8_jo_t *pico8_fr_new(const pico8_jo_keys_t *keys);
 
+/* A synthesizer: an ascension whose `ht` is copied from its archetype;
+ * `mx:kf` points it at an empty sequence once it accepts a resource. */
 int pico8_mx_ready(void);
 const pico8_entity_kind_t *pico8_mx_kind(void);
 const pico8_struct_class_t *pico8_mx_struct_class(void);
@@ -60,23 +48,23 @@ const pico8_struct_class_t *pico8_mx_struct_class(void);
 /* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
  * NULL. The new synthesizer registers itself into `hv`/`cv`/`cg` through
  * `lt`, and nothing ever frees it. Requires `pico8_planet_family_trade_init()`. */
-pico8_mx_t *pico8_mx_new(const pico8_jo_keys_t *keys);
+pico8_jo_t *pico8_mx_new(const pico8_jo_keys_t *keys);
 
 /* Never stores `s`, which may be NULL: `nr:ldown` passes a static struct
  * with no member but its header. */
-pico8_down_t pico8_nr_gv(const pico8_nr_t *self, const pico8_struct_header_t *s);
+pico8_down_t pico8_nr_gv(const pico8_jo_t *self, const pico8_struct_header_t *s);
 
 /* TROU-7 */
 /* Returns a freshly built `jc` instance, or `gv`'s own false; nothing ever
  * frees the instance. */
-pico8_down_t pico8_nr_ldown(pico8_nr_t *self);
+pico8_down_t pico8_nr_ldown(pico8_jo_t *self);
 
-void pico8_nr_hb(pico8_nr_t *self);
+void pico8_nr_hb(pico8_jo_t *self);
 
 /* Writes the quantity left after forwarding back onto `*lw`, so the caller
  * sees what every hop downstream consumed. */
-void pico8_nr_cn(pico8_nr_t *self, pico8_jo_trade_proposal_t *lw);
+void pico8_nr_cn(pico8_jo_t *self, pico8_jo_trade_proposal_t *lw);
 
-void pico8_mx_kf(pico8_mx_t *self, pico8_struct_header_t *sw);
+void pico8_mx_kf(pico8_jo_t *self, pico8_struct_header_t *sw);
 
 #endif
```
