---
title: "Plan 3, étape 2.16 : membres `hd` et `mm` de la table de classe typés `pico8_struct_header_t *self` avec leurs sept cibles ; `void-ptr` 3 → 0, gardé — un commit, jeu identique, objets identiques sous -O1/-O2"
description: "Deux membres de `struct pico8_struct_class` (`hd`, `mm`), leurs cibles `pico8_hd_true`, `pico8_jo_hd_dispatch`, `pico8_ie_hd_dispatch`, `pico8_js_hd_dispatch`, `pico8_fv_hd_dispatch`, `pico8_g__mm`, `pico8_ga_mm` et une cible de TU (`giver_hd`) prennent `pico8_struct_header_t *self` ; 5 casts devenus inutiles partent, aucun n'entre ; 6 appels de TU passent `&x->header` ; `void-ptr` entre dans `COMPOSITION_RESIDUE_GUARD`. Diff de 233 lignes (16 fichiers), reproduit par `run216.py`. Essayé sur copie (départ = 2.15d rejouée par `run215d.sh`, hachages vérifiés) : vert, 26 e2e (aussi sous perturbation), 74 objets de `src/` identiques sous -O1/-O2, un seul écart à -O0 (`pico8_g__mm`, une copie de paramètre en moins), 10 jumelles sans ÉCHEC, valeurs observables identiques, L 12 → 11/11 lignes neuves couvertes. Aucune question à l'humain."
status: needs-triage
---

## Problem Statement

**Écrit sur `5275038`** (branche `claude/plan-pilot-3-7zdft0`). Départ réel attendu : **le commit de 2.15d**, dont l'implémentation était en cours dans l'arbre de travail du dépôt pendant l'écriture (fichiers non commités, non lus). L'arbre des essais est `5275038` + `run215d.sh` (annexe A du PRD 2.15d, `docs/prd/triage/plan3-phase2-etape15d-planete-unique.md`), rejoué sur une copie : il rend exactement les hachages que ce PRD annonce (`src` `f520dbf0…`, `tests` `b15bb8ec…`, `Makefile` `e9226b4f…`) ; les sorties de passes sont celles du PRD (`726` erreurs / `726` éditions, `81` casts enveloppés, `11` identités retirées, `0` erreur finale). Notation : `d` = le commit de la copie qui porte 2.15d (arbres ci-dessus) ; `x` = le commit de la copie qui porte 2.16. Numéros de ligne : ceux de `d`, sauf mention. Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier ; aucun `make` lancé dans le dépôt. Essais sur une seule copie jetable (scratchpad `prd216/`, clone, arbres de travail `wt_p` = `d`, `wt_x` = `x`, `wt_r` = rejeu), supprimée depuis.

Vu du mainteneur du port : après 2.15d, le compteur de composition ne garde plus que `void-ptr` (3 lignes) et `inclusion-mutuelle`. Les trois lignes sont les prototypes publics des cibles de deux membres de la table de classe qui reçoivent encore l'entité par `void *self` : `pico8_hd_true` (`entity.h:47`, le survol « toujours vrai » de `bg` et `fq`), `pico8_ga_mm` (`planet_family.h:139`) et `pico8_g__mm` (`probe.h:158`), les deux générateurs de particule. Or tout ce qui appelle `cls->hd` et `cls->mm` dans `src/` tient déjà un `pico8_struct_header_t *` (`ui.c:274`, `particle.c:55`) : le `void *` n'est plus qu'une porte ouverte à un pointeur qui n'est pas une entité. Une cible doit avoir exactement le type du membre (étude 2.10, §2) : typer une cible, c'est typer le membre et toutes ses autres cibles.

### Texte du plan, recopié (`docs/plan-3.md` à `5275038`)

> - 2.16 — `void-ptr`, selon l'étude 2.10 (§2) : les membres `hd` et `mm` typés `pico8_struct_header_t *self` avec toutes leurs cibles (`pico8_hd_true`, `entity.h:47`, et les quatre `*_hd_dispatch` ; `pico8_g__mm`, `probe.h:177` ; `pico8_ga_mm`, `planet_family.h:212`). Essai sur copie : 18 erreurs pour `hd` (13 entrées de table, 5 dans 2 TU), 2 pour `mm` (`test_pico8_g_.c:552-553`). Le reste de la table garde `void *self` : la Sortie ne compte pas les membres, et les 16 `*_dispatch` ne sont pas renommés. Compte : `void-ptr` 3 → **0**, gardé. Preuves : preuve compilateur négative (un `pico8_vec_t *` passé à `cls->hd` ne compile pas) ; chemin touché. `class-coverage-net` : oui. `sanitize-net` : non. Budget ≈ 50 min.

Numéros du plan datés : à `d`, `pico8_hd_true` est à `entity.h:47` (inchangé), `pico8_g__mm` à `probe.h:158` (plan : `:177`), `pico8_ga_mm` à `planet_family.h:139` (plan : `:212` ; 2.15d a renuméroté l'en-tête), les deux appels `mm` de `test_pico8_g_.c` à `:553-554` (plan : `:552-553`).

Étude 2.10 (`docs/etude-2-10-composants.md`, §2, « Cibles de table en `void *` et typage des membres ») : essai `hd` **18 erreurs** (13 entrées de table dans 6 fichiers de `src/` — `planet_family.c` 6, `planet_family_trade.c` 3, `help.c`, `planet.c`, `research_icon.c`, `ui_button.c` —, 5 dans 2 TU — `test_pico8_fv.c` 4, `test_pico8_ui.c` 1), « elles tombent en retypant les quatre autres cibles de `hd` » ; essai `mm` **2 erreurs** (`test_pico8_g_.c`) ; « **Retenu : typer seulement `hd` et `mm`**. Typer toute la table n'est pas exigé par la Sortie, qui compte les prototypes de premier niveau et pas les membres » (≈ 418 erreurs, le cast descendant resterait dans chaque cible, aucune catégorie ne bougerait) ; « les 16 prototypes `*_dispatch` en `void *self` restent hors du motif ; aucun n'est renommé pour en sortir ». Preuves : « preuve compilateur négative (un `pico8_vec_t *` passé à `cls->hd` ne compile pas, essai sur la copie du commit), chemin touché » ; `class-coverage-net` **oui**, `sanitize-net` **non** (signatures seules).

Décisions de l'humain (même fichier) : Q1 (toutes les catégories ajoutées à la revue de D, dont `void-ptr`, sont des conditions de sortie, à zéro) — c'est ce que fait 2.16. Q2 à Q4, **planètes A**, Q-2.10, Q-2.14d : sans objet ici (aucun champ, aucun lien, aucun type d'entité ne bouge). Note « Découvert à 2.15c » et PRD 2.15d (contrat) : « plus aucun type de sorte ; `pico8_ga_mm(void *self_ptr)` caste vers `const pico8_jo_t *` ; les dispatchers de sorte castent `void *` → `pico8_jo_t *` » ; « typer `mm` et `hd` en `pico8_struct_header_t *self` : les cibles de `planet_family(_trade)` ne connaissent plus que `pico8_jo_t` » ; « numéros de ligne : `planet_display.c` après `:18` reculent de 1 ; `planet_family.h` et `planet_family_trade.h` renumérotés […] ; `planet.c`, `planet.h`, `planet_family.c`, `planet_family_trade.c` gardent leurs numéros » — 2.16 ne cite aucune ligne de `planet_display.c` ; les numéros de `planet_family.h` ci-dessus sont ceux d'après 2.15d.

Règles communes du plan : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; aucune constante d'empreinte de TU (`*_HASH`, `FROZEN_*`) changée ; `class-coverage-net` oui, `sanitize-net` non (ligne 2.16 ; lancé quand même, il est vert). Ce PRD les tient toutes.

### Contrats reçus et ce que ce PRD en fait

| Point (source) | Ce que fait 2.16 |
|---|---|
| « `hd` et `mm` typés `pico8_struct_header_t *self` avec toutes leurs cibles » (plan, étude 2.10 §2) | tenu : 2 membres ; 7 cibles de `src/` (`pico8_hd_true`, 4 `*_hd_dispatch`, `pico8_g__mm`, `pico8_ga_mm`) et 1 de TU (`giver_hd`, `test_pico8_ui.c:733`) |
| essai `hd` 18 erreurs, `mm` 2 (étude) | **revérifié à `d`** : le premier temps (membres, `pico8_hd_true`, `pico8_g__mm`, `pico8_ga_mm` et leurs prototypes, seulement) rend **20 erreurs** : les 13 entrées de table, `test_pico8_fv.c:422-425` (4), `test_pico8_ui.c:741` (1), `test_pico8_g_.c:553-554` (2) — exactement le relevé de l'étude, numéros d'après 2.15d |
| « le reste de la table garde `void *self` ; les 16 `*_dispatch` ne sont pas renommés » (étude) | tenu : aucun autre membre ne change, aucun nom ne change ; les quatre `*_hd_dispatch` changent de type, pas de nom |
| « `pico8_ga_mm(void *self_ptr)` caste vers `const pico8_jo_t *` » (contrat 2.15d) | le cast reste (en-tête → planète, descendant) ; seul le type du paramètre change |
| `void-ptr` 3 → **0**, gardé (plan) | tenu : `void-ptr` entre en fin de `COMPOSITION_RESIDUE_GUARD` ; garde prouvée mordante |
| « preuve compilateur négative » (plan) | `neg216.c` (annexe A), six cas : cinq refusés à `x` et acceptés à `d`, un témoin accepté aux deux |

### État de départ attendu (le commit de 2.15d)

Mesuré sur `d` ; identique aux comptes du PRD 2.15d :

| Commande | Résultat |
|---|---|
| `git rev-parse HEAD:src HEAD:tests` ; `git ls-files -s Makefile` | `f520dbf0021c6e78e35a234db53acd4dd33cda41`, `b15bb8ecf8054c20edf4db61bad706b53fd26ada` ; `e9226b4f1b5a9648628f9189757f6e652b717347` |
| `make composition-residue` | `void-ptr 3 3`, `inclusion-mutuelle 15 15` ; **`résidu 2 : 42 ligne(s), 26 fichier(s)`** |
| `sh tools/composition_residue.sh --check void-ptr src tests` | 3 lignes : `entity.h:47`, `planet_family.h:139`, `probe.h:158` |
| `--champs \| tail -1` | `champs : 238 ; 0 écriture : 14 ; 0 hors constructeur : 19` |
| `--check inclusion-mutuelle src tests \| tail -1` | `résidu gardé (inclusion-mutuelle) : 39 ligne(s)` |
| `COMPOSITION_RESIDUE_GUARD` (`Makefile:874`) | `…,accesseurs-q6,lien-parent,parente` (11 catégories) |
| casts / tests de classe (greps de 2.15d) | 469 / 647 |
| graphe d'usage (`usegraph.py`, annexe A du PRD 2.11) | `paires 18` |
| `sizeof` : noyau, `g_`, `pico8_jo_t` ; `pico8_probe_t`, `pico8_emitter_t` | 128, 256, 424 ; 24, 56 |
| `make test` ; `make e2e` ; `MALLOC_PERTURB_=165 make e2e` | vert ; 26 conformes ; 26 conformes |

Autre départ (2.15d commitée autrement que par le script, ou suivie d'un commit qui touche `src/`, `tests/` ou le `Makefile`) : `run216.py` tourne quand même tant que chaque texte cherché est présent (il s'arrête sinon, en nommant le texte) ; les hachages d'arrivée ne valent plus ; comparer alors tous les comptes et rapporter l'écart (repli 2).

## Solution

Un commit, vert, jeu identique (26 e2e conformes, aussi sous `MALLOC_PERTURB_=165`), aucune constante d'empreinte changée. `void-ptr` 3 → **0**, gardé. Le commit est le résultat de `run216.py` (annexe A, texte exact ; 18 remplacements, 21 occurrences), plus la note du plan et le message :

1. **Membres** (`struct.h:147`, `:159`) : `int (*hd)(pico8_struct_header_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra);` et `struct pico8_qy *(*mm)(pico8_struct_header_t *self);`. `pico8_struct_header_t` est déclaré plus haut dans le même en-tête (`:97-114`) : aucun include, aucune déclaration anticipée.
2. **Cibles `hd`** : `pico8_hd_true` (`entity.h:47`, `entity.c:125`), `pico8_jo_hd_dispatch` (`planet.h:104`, `planet.c:35`), `pico8_ie_hd_dispatch` (`research_icon.c:201`), `pico8_js_hd_dispatch` (`ui_button.c:37`), `pico8_fv_hd_dispatch` (`help.c:34`) : `void *self` → `pico8_struct_header_t *self`. Les quatre dispatchers appelaient `pico8_gk((const pico8_struct_header_t *)self, qa, ra)` : le cast ne fait plus qu'ajouter `const`, conversion implicite en C ; il part (`pico8_gk(self, qa, ra)`). Les 13 entrées `.hd = …` des tables ne changent pas de texte.
3. **Cibles `mm`** : `pico8_g__mm` (`probe.h:158`, `probe.c:328`) prend `pico8_struct_header_t *self` ; sa locale `pico8_struct_header_t *self = (pico8_struct_header_t *)self_ptr;` (`:329`), devenue une copie du paramètre, part avec son cast. `pico8_ga_mm` (`planet_family.h:139`, `planet_family.c:350`) prend `pico8_struct_header_t *self_ptr` ; sa locale `const pico8_jo_t *self = (const pico8_jo_t *)self_ptr;` **reste** (descente en-tête → planète ; la table de `ga` n'est posée que sur des planètes `ga`).
4. **TU** : `test_pico8_fv.c:422-425` appellent `cls_of(fv)->hd(&fv->header, …)` (`fv` est un `pico8_g__t *`) ; `test_pico8_g_.c:553-554` appellent `cls_of(a)->mm(&a->header)`, `cls_of(b)->mm(&b->header)` ; `giver_hd` (`test_pico8_ui.c:733`) prend `pico8_struct_header_t *self`. `&x->header`, pas un cast : même adresse (`header` est le premier membre), aucun cast neuf ; les objets de ces trois TU sont identiques octet pour octet (preuve 4). Les autres appelants de TU (`test_pico8_ga.c`, `jo`, `ie`, `mx`, `pm`, `hj`, `fq`) passent un `void *` : conversion implicite, inchangés.
5. **`Makefile`** : `void-ptr` ajouté en fin de `COMPOSITION_RESIDUE_GUARD` (`:874`). La catégorie reste dans `CATEGORIES`/`SORTIE` du script (à 0).

Rien d'autre : ni `tools/` (`composition_r2_sites.txt` compris : ses 11 lignes décrivent l'amont `void *` des sites R2, aucun ne passe par `hd`/`mm`), ni `docs/glossaire-noms.md` (il dit ce que sont `hd`, `gk`, pas leur type), ni `docs/trous-de-portage.md`, ni `tests/e2e/`, ni `*.sha256`, ni une ligne `#include`, ni un nom de fonction, ni un autre membre de la table, ni un commentaire.

## User Stories

1. En tant que mainteneur, je veux que `cls->hd` et `cls->mm` refusent à la compilation un pointeur qui n'est pas une entité, afin qu'un `pico8_vec_t *` passé par erreur ne compile pas.
2. En tant que gardien du compteur, je veux `void-ptr` à 0 et gardé, afin qu'un prototype public qui reprend une entité par `void *` rougisse `make test`.
3. En tant qu'auteur de 2.17, je veux savoir quels numéros de ligne ont bougé (`probe.c` après `:329`, −1) et que le graphe d'usage et les paires d'inclusion n'ont pas changé.
4. En tant que gardien de la règle du parent, je veux que chaque mutation rouge au parent le soit à l'enfant sur les chemins que le diff touche (dispatchers, tables, générateurs de particule).

## Implementation Decisions

### Modules

`struct.h` ; `entity.h`/`.c` ; `planet.h`/`.c` ; `planet_family.h`/`.c` ; `probe.h`/`.c` ; `help.c` ; `research_icon.c` ; `ui_button.c` ; `Makefile` (une ligne) ; TU `fv`, `g_`, `ui` ; `docs/plan-3.md` (note « Découvert à 2.16 »). (L'étude listait aussi `planet_family_trade` et `planet` pour leurs entrées de table : elles ne changent pas de texte, seul le type de la cible change.)

### Inclusions

Aucune ligne `#include` ne bouge. `inclusion-mutuelle` **15 / 15, 39 lignes** aux deux bouts ; graphe d'usage **18**, sortie de `usegraph.py` identique octet pour octet (`diff` vide) ; `--detail` : seules sortent les trois lignes `void-ptr`.

### Ce qui change

Le texte exact est le résultat du script : `python3 run216.py <arbre>` sur un arbre propre au commit de 2.15d (arbres de la table « État de départ ») rend, mesuré, `git rev-parse` des arbres (après `git add -A`) : **`src` `148603f32a11ad8a77bcb8384deb2512a2a9935b`**, **`tests` `cac3259733163ed40d59198d02463eb9618e13a2`**, **`Makefile` `422546ca868cfbdd089c8348b8eb4806763b9010`** ; `git diff d x | sha256sum` → `efb84dbc…` (233 lignes, 16 fichiers, +25 / −26). Rejoué sur un arbre de travail neuf à `d` : mêmes hachages, même empreinte de diff (Runs 17). Le diff complet est en annexe B, texte exact.

**Casts** (grep de la réserve 4 de 2.13a, `\(\s*(const\s+)?[a-z0-9_]+\s*\*+\s*\)` sur `src tests`) : **469 → 464**, cinq retirés, **aucun ajouté**, chacun justifié :

| # | Site (`d`) | Cast retiré | Pourquoi il ne sert plus |
|---|---|---|---|
| C1 | `planet.c:36` (`pico8_jo_hd_dispatch`) | `(const pico8_struct_header_t *)self` | `self` est un `pico8_struct_header_t *` ; `pico8_gk` prend `const pico8_struct_header_t *` : ajout de qualificatif, implicite |
| C2 | `research_icon.c:202` (`pico8_ie_hd_dispatch`) | idem | idem |
| C3 | `ui_button.c:38` (`pico8_js_hd_dispatch`) | idem | idem |
| C4 | `help.c:35` (`pico8_fv_hd_dispatch`) | idem | idem |
| C5 | `probe.c:329` (`pico8_g__mm`) | `(pico8_struct_header_t *)self_ptr`, avec sa locale | identité : le paramètre a déjà ce type ; la locale n'était qu'une copie, le paramètre prend son nom `self` |

Cast gardé : `(const pico8_jo_t *)self_ptr` (`planet_family.c:351`, `pico8_ga_mm`), descente de l'en-tête vers la planète, seule forme possible sans enchâssement (2.15d). Tests de classe (`->cls`, `header.cls`, `_struct_class()`) : **647**, inchangés.

**Lecture** : les `*_hd_dispatch` restent des fonctions à part (`pico8_gk` prend `const pico8_struct_header_t *`, le membre un `pico8_struct_header_t *` : types de pointeur de fonction incompatibles, et le plan interdit de renommer ou de fondre un dispatcher). `pico8_hd_true`, `pico8_g__mm` et `pico8_ga_mm` gardent leur nom et leur place ; seuls leurs prototypes quittent `void-ptr` (ils ne prennent plus `void *`).

### Comptes, mesurés sur la copie

| Mesure | `d` | `x` |
|---|---|---|
| `void-ptr` (lignes / fichiers) | 3 / 3 | **0 / 0**, gardé |
| `parente`, `lien-parent`, `accesseurs-q6`, `défauts`, `registres` et les six autres gardées | 0, gardées | 0, gardées |
| `inclusion-mutuelle` (paires / modules, lignes du `--check`) | 15 / 15, 39 | 15 / 15, 39 |
| graphe d'usage (`usegraph.py`) | 18 | 18 (sortie identique) |
| pied `résidu 2` | 42 / 26 | **39 / 24** |
| `--champs` | 238 ; 14 ; 19 | 238 ; 14 ; 19 |
| casts | 469 | **464** (C1 à C5) |
| tests de classe | 647 | 647 |
| `sizeof(pico8_struct_header_t)`, `g_`, `pico8_jo_t` | 128, 256, 424 | 128, 256, 424 |
| `COMPOSITION_RESIDUE_GUARD` | 11 catégories | **12** (+ `void-ptr`) |
| `make composition-residue` | `void-ptr 3 3`, `inclusion-mutuelle 15 15` | `inclusion-mutuelle 15 15` seule |
| constantes `*_HASH`/`FROZEN_*` changées | — | 0 |
| lignes du diff : `src/*.c` ; `src/*.h` ; TU ; `Makefile` | — | +11 / −12 ; +6 / −6 ; +7 / −7 ; +1 / −1 |

**Pied** : 42 − 3 = **39** lignes ; fichiers : sortent `entity.h` et `probe.h`, qui n'y comptaient que par `void-ptr` ; `planet_family.h` reste (`:8`, paire `planet` ↔ `planet_family`) : 26 − 2 = **24**. Après 2.16, le pied ne compte plus que les lignes d'`inclusion-mutuelle` (39), comme le plan l'annonce pour 2.17.

**`--champs`** inchangé : il relève les membres des types d'entité, pas la table de classe.

### Contrat pour 2.17 et la suite (à recopier dans leurs PRD)

| Point | Pour | Ce qu'il faut savoir |
|---|---|---|
| pied = `inclusion-mutuelle` seule : **39 lignes / 24 fichiers**, 15 paires, usage 18 | 2.17 | toutes les autres catégories sont à 0 et gardées (12 dans `COMPOSITION_RESIDUE_GUARD`) |
| numéros de ligne : `probe.c` après `:329` reculent de 1 ; tous les autres fichiers gardent leurs numéros (changements sur place) | 2.17 | les PRD qui citent `probe.c` d'avant 2.16 au-delà de `:329` les décalent |
| `hd` et `mm` prennent `pico8_struct_header_t *` ; les 13 autres membres de la table gardent `void *self` ; les 16 `*_dispatch` gardent leur nom | toutes | typer le reste de la table n'est pas exigé (étude 2.10, §2) ; ne pas renommer un dispatcher pour sortir du motif |
| `void-ptr-r2` (`--detail`, `tools/composition_r2_sites.txt`) : 11 lignes d'information, inchangées | 2.18 | elles ne comptent pas au pied ; aucune ne passe par `hd`/`mm` |

### Anti-tassement

**Interdit** :
- renommer une cible en `*_dispatch` ou en `static` pour la sortir du motif sans la typer ; ajouter un alias de type (`typedef void pico8_self_t;`) ou une macro qui tient lieu de `void *` ;
- typer d'autres membres de la table, fondre ou renommer un dispatcher, retirer `pico8_hd_true` ;
- ajouter un cast (`(pico8_struct_header_t *)fv`) là où `&x->header` suffit, ou garder un des cinq casts C1 à C5 ;
- toucher un `*.sha256`, `tests/e2e/`, `tools/`, une constante `*_HASH`/`FROZEN_*`, un commentaire, une ligne `#include`, `docs/glossaire-noms.md`, `docs/trous-de-portage.md` ; toucher le `Makefile` ailleurs qu'à `COMPOSITION_RESIDUE_GUARD` ;
- réécrire à la main ce que le script fait, sauf repli.

## Implementation Chunks

Un commit, **≈ 30 min** preuves comprises, sous le budget de 50 min (mesuré sur la copie : script < 1 s ; `make test` + `make e2e` ≈ 1 min par arbre ; jumelles 138 s pour les deux côtés en parallèle ; couverture ≈ 1 min ; objets ≈ 2 min) : copier `run216.py` hors du dépôt, le lancer, comparer les hachages, `make test`, `make e2e`, `MALLOC_PERTURB_=165 make e2e` ; comptes (`base.sh` du PRD 2.15d, `-DFLAT`) ; `usegraph.py` ; objets (`objs.sh` ×5 configurations, `fncmp.py`) ; jumelles (`mut216.py`, 10 × 2) ; `obs216.c` ; `line-coverage` des 26 e2e aux deux bouts + `lset.py` ; `neg216.c` ; garde ; `class-coverage-net`, `sanitize-net` ; note « Découvert à 2.16 », message.

**Pas de découpe** : un membre et ses cibles changent de type ensemble (sinon l'initialiseur de table ne compile pas) ; `hd` et `mm` pourraient être deux commits, mais le second ferait 6 lignes et ne mettrait `void-ptr` à 0 qu'avec lui ; un seul commit, comme la ligne du plan.

**Replis écrits**, dans l'ordre :
1. si `run216.py` s'arrête sur un `AssertionError` (texte introuvable ou compté autrement) : lire le site, faire le même remplacement à la main **selon la même règle** (type du paramètre ; cast devenu inutile retiré ; `&x->header` dans un TU), noter chaque site dans le message ; si le site n'existe plus (2.15d a changé autrement), le noter et continuer ;
2. si le départ n'est pas l'arbre de 2.15d annoncé (hachages différents) : lancer le script quand même, ne pas comparer les hachages d'arrivée, comparer tous les comptes de la table et rapporter l'écart dans la note ;
3. si une erreur de compilation reste après le script (`syn.sh` du PRD 2.15d non vide) : c'est une cible ou un appelant que ce PRD n'a pas vus (neuf depuis `d`) ; le typer selon la même règle, jamais par un cast, et le noter ;
4. si le commit déborde de 50 min avant d'être fait : garder `hd` et `mm` dans le même commit, arrêter, rapporter ce qui manque dans la note « Découvert à ».

## Testing Decisions

- **Pas de TU neuf ; trois TU touchés au texte seulement** (`fv`, `g_` : l'argument devient `&x->header` ; `ui` : type du paramètre de `giver_hd`). Aucun rejeu d'`events.json` dans un TU. Aucun `*_HASH` ne bouge : les objets de ces trois TU sont identiques (preuve 4).
- **Bon test ici** : un changement de type de paramètre se prouve par le compilateur (ce qu'il refuse désormais, preuve 3) et par l'identité des objets (le code machine ne dépend pas du type d'un pointeur) ; le seul changement de code (C5, une copie de paramètre en moins dans `pico8_g__mm`) est un chemin touché : couverture, jumelles et valeurs observables.
- **Prior art** : PRD 2.15d (`syn.sh`, `objs.sh`, `mut.py`, `base.sh`, règle des jumelles, garde) ; PRD 2.15c (`lset.py`, ensemble `L`, `obs.c`) ; étude 2.11, annexe A (`usegraph.py`) ; règle du parent (D.21).

### Preuves (parent de `x` = `d`)

1. **Communes** : `make test` vert ; `make e2e` **26 conformes** ; `MALLOC_PERTURB_=165 make e2e` **26 conformes**, aux deux bouts ; `git diff --stat d -- tests/e2e tools '*.sha256'` vide ; `git diff d | grep -E '^[-+].*(_HASH|FROZEN_)'` vide (Runs 6, 16).

2. **Reproductibilité** : `run216.py` sur un arbre de travail neuf à `d` → `2.16 : 18 remplacements, 21 occurrences` ; arbres `148603f3…`, `cac32597…`, `422546ca…` ; `git diff d | sha256sum` `efb84dbc…`, identique à `x` (Runs 17).

3. **Preuve compilateur négative** (`neg216.c`, annexe A, `cc -std=c99 -Wall -Wextra -Werror -Isrc -DN=… -fsyntax-only`) :

| N | Ce que le cas fait | `d` | `x` |
|---|---|---|---|
| 1 | `c->hd(v, v, v)` avec `pico8_vec_t *v` (plan) | compile | **refusé** : `passing argument 1 of 'c->hd' from incompatible pointer type` |
| 2 | `c->mm(v)` avec `pico8_vec_t *v` | compile | **refusé** : `… of 'c->mm' …` |
| 3 | table `{.hd = t}` avec `int t(void *s, …)` (cible au mauvais type) | compile | **refusé** : `initialization of 'int (*)(pico8_struct_header_t *, …)' from incompatible pointer type 'int (*)(void *, …)'` |
| 4 | table `{.mm = m}` avec `struct pico8_qy *m(void *s)` | compile | **refusé** : `initialization of 'struct pico8_qy * (*)(pico8_struct_header_t *)' …` |
| 5 | `pico8_hd_true(v, …)`, `pico8_g__mm(v)`, `pico8_ga_mm(v)` avec `pico8_vec_t *v` | compile | **refusé** (3 erreurs) |
| 6 | témoin : `{.hd = pico8_hd_true, .mm = pico8_g__mm}`, `{.mm = pico8_ga_mm}`, `c->hd(&g->header, …)`, `c->mm(&g->header)` | compile | compile |

   (Runs 15.)

4. **Objets sans `-g`** (`objs.sh` du PRD 2.15d, les 74 `src/*.c` hors raylib ; `fncmp.py`, annexe A, compare fonction par fonction sans les adresses) :
   - `-O1` et `-O2`, `-DNDEBUG -DPICO8_CLASS_COVERAGE` : **74 / 74 identiques** octet pour octet ;
   - `-O0 -DNDEBUG` (avec ou sans `-DPICO8_CLASS_COVERAGE`) : 73 identiques ; `probe.o` diffère dans **une seule fonction sur 31, `pico8_g__mm`** (C5 : la copie du paramètre dans la locale disparaît, les emplacements de pile sont renumérotés ; les symboles qui suivent sont décalés de 8 octets dans `nm`, `.rodata` identique) ;
   - `-O0` avec les `assert` : en plus, `fm_free`, `pico8_fm_hy`, `pico8_g__struct_new` de `probe.o` ne diffèrent que par l'immédiat `__LINE__` de leur `assert` (−1 : `0x1c9` → `0x1c8`, `0x1ec` → `0x1eb`, `0x1a2` → `0x1a1`) ;
   - TU touchés (`test_pico8_fv`, `test_pico8_g_`, `test_pico8_ui`), `-O0`, `-O1`, `-O2` `-DNDEBUG` : **identiques** octet pour octet.
   Lecture : les 13 entrées de table, les quatre dispatchers (C1 à C4 compris), `pico8_hd_true`, `pico8_ga_mm` et les TU ne changent pas de code ; seul `pico8_g__mm` à `-O0` change, sans effet aux niveaux optimisés (Runs 8, 9).

5. **Mutations, règle du parent** (`mut216.py` = `mut.py` du PRD 2.15d avec `S` et le nom du JSON changés ; `muts216.json`, annexe A). Même mutation au parent `d` et à l'enfant `x`, sur les chemins que le diff touche : les quatre dispatchers (arguments `qa`/`ra` échangés), `pico8_hd_true`, deux entrées de table, les deux générateurs de particule. `make -k -j4 unit` puis `make -k e2e` ; restaurer par `git checkout -f`. **ÉCHEC si un TU rouge au parent n'est pas rouge à l'enfant, ou si l'e2e est < 26 au parent et 26 à l'enfant.**

| # | Cible | Mutation (des deux côtés) | Rouges au parent | Rouges à l'enfant | e2e d / x | Verdict |
|---|---|---|---|---|---|---|
| HD-jo | `pico8_jo_hd_dispatch` (C1) | `pico8_gk(self, ra, qa)` | `ui` | `ui` | 10 / 10 | conforme |
| HD-js | `pico8_js_hd_dispatch` (C3) | idem | `hp`, `lq`, `replay_from_title`, `ui` | les mêmes 4 | 0 / 0 | conforme |
| HD-ie | `pico8_ie_hd_dispatch` (C2) | idem | `ie` | `ie` | 17 / 17 | conforme |
| HD-fv | `pico8_fv_hd_dispatch` (C4) | idem | `fv` | `fv` | 26 / 26 | conforme |
| HDT | `pico8_hd_true` | `return 0;` | `bg`, `fq`, `qb`, `ui` | les mêmes 4 | 4 / 4 | conforme |
| HD-bg | entrée `.hd = pico8_hd_true` de `bg` | ligne retirée | `bg`, `fr`, `g_`, `ga`, `hj`, `jo`, `mx`, `nl`, `nr`, `os`, `pm`, `qb`, `qh`, `ui` | les mêmes 14 | 7 / 7 | conforme |
| HD-ga | entrée `.hd = pico8_jo_hd_dispatch` de `ga` | ligne retirée | `ga`, `jo`, `pm` | les mêmes 3 | 26 / 26 | conforme |
| MM-g_-t | `pico8_g__mm` (C5), lecture de `self` | `t = 0;` au lieu de `t = self->t;` | `hj`, `mx`, `pm`, `probe_mm_rnd_order` | les mêmes 4 | 9 / 9 | conforme |
| MM-g_-self | `pico8_g__mm` (C5), garde `self != NULL` | `if (self != NULL && 0)` | `fv`, `g_`, `hj`, `mx`, `pm`, `probe_mm_rnd_order` | les mêmes 6 | 7 / 7 | conforme |
| MM-ga-ms | `pico8_ga_mm`, lecture par le cast gardé | `ms = self->iu;` | `ga`, `hp`, `pm` | les mêmes 3 | 24 / 24 | conforme |

   Lecture : **aucun ÉCHEC** (10 jumelles, 20 lancements, aucune erreur de compilation, rouges et e2e identiques des deux côtés, chaque mutation rouge). HD-fv et HD-ga ne rougissent que des TU (26 e2e) : leurs survols ne changent pas l'image des 26 e2e ; déclaré, sans conséquence (le parent est identique). **Lignes K** (enfant seul) : aucune ; l'enfant n'a qu'un chemin neuf, C5, couvert par MM-g_-t et MM-g_-self (Runs 10).

6. **Chemin touché, ensemble `L`** : `make line-coverage EVENTS="<les 26 e2e>"` à `d` et `x` : **6032** et **6031** lignes (la ligne retirée `probe.c:329`). `lset.py` (PRD 2.15c, annexe A) : **0 perdue, 0 gagnée** sur les lignes inchangées ; `L` (couvertes à `d`, réécrites ou retirées) **12** — `entity.c:125`, `help.c:34-35`, `planet.c:35-36`, `planet_family.c:350`, `probe.c:328-329`, `research_icon.c:201-202`, `ui_button.c:37-38` ; lignes neuves couvertes **11 / 11 exécutables** (les mêmes, sans `probe.c:329`) ; les six neuves non couvertes sont des déclarations (`entity.h:47`, `planet.h:104`, `planet_family.h:139`, `probe.h:158`, `struct.h:147`, `:159`) (Runs 12, 13).

7. **Valeurs observables** (`obs216.c`, annexe A, même source aux deux bouts : elle appelle `h->cls->hd(h, …)` et `h->cls->mm(h)` sur un `pico8_struct_header_t *h`, qui convertit en `void *` à `d`) : cart démarré, dix planètes fraîches (par leur `kind`, `ra` posé), `g_`, `fv`, `bg` ; pour chacune, `hd` sur 6 × 6 couples de points (36 bits) et trois particules de `mm` (`p`, `v`, `has_a`, `a`, `l`). **13 lignes identiques** à `d` et `x`, normal et sous `MALLOC_PERTURB_=165` (`a48927a8…` ×4) (Runs 14).

8. **Garde mordante** : à `x`, le prototype `int pico8_hd_true(void *self, …)` remis dans `entity.h` → `make composition-guard` code 2, `void-ptr: src/pico8_entity.h:47:…`, `résidu gardé (…,void-ptr) : 1 ligne(s)` ; remis → 0 (Runs 16).

9. `class-coverage-net` : **oui**, `couverture: OK -- chaque classe non exemptée est instanciée (26 rejeu(x))` à `x`. `sanitize-net` : **non** exigé (signatures seules, aucune durée de vie ne change) ; lancé : `sanitize: 26 propre(s)` (Runs 16).

10. **Graphe d'usage et inclusions** : `usegraph.py` `paires 18` aux deux bouts, sorties identiques ; `--detail` : seules sortent les trois lignes `void-ptr` ; `--check inclusion-mutuelle` 39 → 39 (Runs 7).

### Greps de Sortie

À `x` :
1. `sh tools/composition_residue.sh --check void-ptr src tests | tail -1` → `résidu gardé (void-ptr) : 0 ligne(s)`.
2. `grep -nE '\(\*(hd|mm)\)\(' src/pico8_struct.h` → 2 lignes, toutes deux `(pico8_struct_header_t *self` ; `grep -cE '\(\*[a-z_]+\)\(void \*self' src/pico8_struct.h` → **12** (les autres membres, inchangés ; `has_hb` prend `const void *`).
3. `grep -rnE '^(static )?(int|pico8_qy_t \*)pico8_[a-z_]+(hd_true|hd_dispatch|_mm)\(void' src` → **0**.
4. `grep -rn 'pico8_gk((const pico8_struct_header_t \*)self' src` → **0** ; `grep -c 'self_ptr' src/pico8_probe.c` → **0** ; `grep -c '(const pico8_jo_t \*)self_ptr' src/pico8_planet_family.c` → **1**.
5. `grep -rnE 'cls_of\((fv|a|b)\)->(hd|mm)\(&(fv|a|b)->header' tests` → **6** ; `git diff d -- tests | grep -cE '^\+.*\((const )?pico8_struct_header_t \*\)'` → **0**.
6. `grep -n '^COMPOSITION_RESIDUE_GUARD' Makefile` → finit par `,parente,void-ptr`.
7. `git diff d -- src | grep -E '^[-+]#include'` → vide ; `git diff d | grep -E '^[-+].*(_HASH|FROZEN_)'` → vide.
8. `make composition-residue`, `--champs`, `--check inclusion-mutuelle`, les greps de casts et de classe → la table « Comptes ».
9. `git diff --stat d` : les fichiers de la section « Modules », plus `docs/plan-3.md`, rien d'autre.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Voici celui de ce PRD, sur la copie jetable (`prd216/copie`, arbres de travail `wt_p` = `d`, `wt_x`, `wt_r`).

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git clone --no-hardlinks` local, `checkout 5275038` ; `git rev-parse HEAD:src HEAD:tests HEAD:Makefile` ; scripts de l'annexe A du PRD 2.15d extraits | — | 0 | `3dc0b58f…`, `e839bb08…`, `30351cca…` (= `b35ca48`) ; 13 scripts | 3 s |
| 2 | `run215d.sh copie` ; `git add -A` ; hachages ; commit `d` | `d` | 0 | passes `726/726`, `0` ; casts `81`, `11`, `11` ; `f520dbf0…`, `b15bb8ec…`, `e9226b4f…` (ceux du PRD 2.15d) | 10 s |
| 3 | greps des cibles et appelants de `hd`/`mm` ; `--check void-ptr` | `d` | 0 | 3 lignes ; appelants `ui.c:274`, `particle.c:55` en `pico8_struct_header_t *` | < 1 s |
| 4 | premier temps (membres, `pico8_hd_true`, `pico8_g__mm`, `pico8_ga_mm` et prototypes, par `sed`) ; `syn.sh` | `wt_x` | 1 | **20 erreurs** : 13 entrées de table, `fv` 4, `ui` 1, `g_` 2 (relevé de l'étude) | 3 s |
| 5 | cibles `hd` et TU retypés (script Python, futur `run216.py`) ; `syn.sh` ; `Makefile` ; longueur des lignes (max 108 hors `Makefile`) ; commit `x` | `wt_x` | 0 | 0 erreur ; diff 233 lignes, `efb84dbc…` | 5 s |
| 6 | `make -j4 test` ; `make e2e` dans `wt_p` et `wt_x` en parallèle | `d`, `x` | 0 | vert ; 26 ; 26 | non chronométré (≈ 1 min) |
| 7 | `base.sh -DFLAT` aux deux bouts ; `usegraph.py` premier jet extrait avec sa ligne d'ouverture Markdown (`SyntaxError`, même piège que 2.15d Runs 3), corrigé ; `--detail`, fichiers du pied | `d`, `x` | 0 | table « Comptes » ; `paires 18` ×2, sortie identique ; 42/26 → 39/24 | 5 s |
| 8 | `objs.sh` ×5 (`-O0 -DNDEBUG`, `-O0` avec `assert`, `-O0`/`-O1`/`-O2` `-DNDEBUG -DPICO8_CLASS_COVERAGE`) aux deux bouts ; `cmp` | `d`, `x` | 0 | `-O1`, `-O2` 74/74 ; `-O0` 73/74 (`probe`) | ≈ 2 min |
| 9 | `fncmp.py` sur `probe` ; immédiats des trois fonctions à `assert` ; TU `fv`, `g_`, `ui` à `-O0/-O1/-O2` | `d`, `x` | 0 | 1/31 fonction (`pico8_g__mm`) ; `__LINE__` −1 ; TU identiques | 20 s |
| 10 | `mut216.py d` et `mut216.py x` en parallèle (10 jumelles) | `wt_p`, `wt_x` | 0 | tableau des jumelles, aucun ÉCHEC, aucune erreur de compilation | 138 s |
| 11 | `git status` des deux arbres après les jumelles | `wt_p`, `wt_x` | 0 | propres | < 1 s |
| 12 | `make line-coverage` des 26 e2e aux deux bouts, en parallèle | `d`, `x` | 0 | 6032 ; 6031 | non chronométré (≈ 1 min) |
| 13 | `lset.py copie d x` | — | 0 | `L` 12, neuves 11/11, 0 perdue, 0 gagnée | < 1 s |
| 14 | `obs216.c` aux deux bouts, normal et `MALLOC_PERTURB_=165` | `d`, `x` | 0 | 13 lignes, `a48927a8…` ×4 | < 5 s |
| 15 | `neg216.c` `-DN=1` à `6` aux deux bouts | `d`, `x` | `d` : 0 ×6 ; `x` : 1 ×5, 0 | tableau de la preuve 3 | < 2 s |
| 16 | garde (`pico8_hd_true(void *self` remis, `make -s composition-guard`, remis) ; `make class-coverage-net` ; `make -j2 sanitize-net` ; `MALLOC_PERTURB_=165 make e2e` aux deux bouts | `x` (et `d` pour l'e2e) | 2 ; 0 ; 0 ; 0 ; 0 | 1 ligne `void-ptr` ; `couverture: OK` ; `26 propre(s)` ; 26 ; 26 | non chronométré (≈ 1 min) |
| 17 | `run216.py` sur `wt_r` neuf à `d` ; `git diff x216` ; hachages | `wt_r` | 0 | diff vide ; `148603f3…`, `cac32597…`, `422546ca…` ; `efb84dbc…` | < 1 s |
| 18 | `rm -rf` de la copie, des arbres de travail et des fichiers d'essai | — | 0 | supprimés | < 1 s |

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas le commit de 2.15d (arbres `f520dbf0…`, `b15bb8ec…`, `e9226b4f…`, ou un descendant qui ne touche que `docs/`) sans que l'écart soit rapporté, ou si l'état de départ diffère de la table sans explication.
- `FAILED:` si 2.16 n'est pas un commit, vert (`make test`, `make e2e` 26 conformes, et 26 sous `MALLOC_PERTURB_=165`), ou si un `*.sha256`, `tests/e2e/`, `tools/`, une constante `*_HASH`/`FROZEN_*` change, ou le `Makefile` ailleurs qu'à `COMPOSITION_RESIDUE_GUARD`.
- `FAILED:` si, au départ annoncé, l'arbre commité n'a pas les hachages `src` `148603f3…`, `tests` `cac32597…`, `Makefile` `422546ca…`, sans repli rapporté.
- `FAILED:` s'il reste une ligne `void-ptr` ; si `void-ptr` n'est pas dans `COMPOSITION_RESIDUE_GUARD` ; si la garde n'a pas été prouvée mordante ; si une cible de `hd` ou `mm` garde `void *self` ou a été renommée, rendue `static` ou remplacée par un alias ou une macro pour sortir du motif.
- `FAILED:` si un autre membre de la table change de type, si un nom de fonction change, si une ligne `#include` ou un commentaire change ; si un cast est ajouté, ou si l'un des casts C1 à C5 reste.
- `FAILED:` si `neg216.c` ne rend pas le tableau de la preuve 3 (N = 1 à 5 refusés à `x`, N = 6 accepté).
- `FAILED:` si un objet de `src/` diffère sous `-O1` ou `-O2 -DNDEBUG` ; si, sous `-O0 -DNDEBUG`, une autre fonction que `pico8_g__mm` diffère ; si un objet des TU `fv`, `g_`, `ui` diffère.
- `FAILED:` si une jumelle a un TU rouge au parent qui ne l'est pas à l'enfant, ou un e2e < 26 au parent et 26 à l'enfant ; si la sortie d'`obs216.c` (aussi sous perturbation) diffère entre `d` et `x`.
- `FAILED:` si les comptes ne sont pas ceux de la table « Comptes » (`void-ptr` 0 ; pied 39 / 24 ; `inclusion-mutuelle` 15 / 15, 39 lignes ; usage 18 ; `--champs` 238 ; casts 464 ; tests de classe 647), ou si une catégorie gardée n'est plus à 0.
- `FAILED:` si `class-coverage-net` est rouge ; si une ligne couverte par les 26 e2e à `d` ne l'est plus à `x` hors `probe.c:329` (retirée), ou l'inverse ; si une ligne exécutable neuve n'est pas couverte.
- `FAILED:` si une grep de Sortie ne rend pas ce qui est écrit.
- `FAILED:` si le message de commit omet : les deux membres et les sept cibles, les cinq casts retirés et le cast gardé, les six appels de TU, la garde, la preuve négative, la preuve des objets (dont l'écart `-O0` de `pico8_g__mm`), la table des jumelles avec leur résultat au parent, les comptes, `L`, le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à 2.16 » au bout de sa ligne du plan, ou si le plan change ailleurs.
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- Les 13 autres membres de la table en `void *self` et les 16 `*_dispatch` (étude 2.10, §2 : non exigés par la Sortie) ; les casts descendants qu'ils portent, dont celui de `pico8_ga_mm`.
- Les paires d'inclusion (2.17) ; les 11 lignes d'information `void-ptr-r2` et `tools/composition_r2_sites.txt`.
- La taille du code C, l'appli raylib, l'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Les casts devenus inutiles partent (C1 à C5).** Même règle que 2.15d (« garder un cast identité » interdit) : C5 est une identité ; C1 à C4 n'ajoutent que `const`, ce que C fait implicitement. Les garder laisserait croire à une conversion qui n'existe plus.
- **G2. `pico8_g__mm` perd sa locale, `pico8_ga_mm` garde la sienne.** La première n'était qu'une copie du paramètre ; la seconde est la descente vers `pico8_jo_t`. Prix déclaré : `probe.o` diffère à `-O0` dans cette seule fonction ; `-O1`/`-O2` identiques. L'autre forme (`pico8_struct_header_t *self = self_ptr;`) garderait des objets identiques à `-O0` mais un paramètre mal nommé et une copie sans objet.
- **G3. `&x->header` dans les TU, pas un cast.** L'argument est un `pico8_g__t *` ; `&fv->header` dit ce qu'on passe, ne cache aucune conversion et donne le même objet.
- **G4. Un commit pour `hd` et `mm`.** Le plan le dit (≈ 50 min, une ligne) ; couper ne mettrait `void-ptr` à 0 qu'au second.
- **G5. `sanitize-net` lancé bien que non exigé** (6 s) : il ne coûte rien et ferme la question.

### Surprises de mesure, pour `plan-pilot`

1. **Le relevé de l'étude tient à l'identique après 2.15d** : 20 erreurs, mêmes sites (numéros décalés dans `test_pico8_g_.c`, `planet_family.h`, `probe.h`).
2. **Aucun appelant de `src/` ne change** : `ui.c:274` et `particle.c:55` tiennent déjà un `pico8_struct_header_t *` depuis 2.11r ; seuls trois TU passaient un type d'entité concret.
3. **Le pied tombe à 39 / 24** : après 2.16, il ne compte plus que `inclusion-mutuelle` ; `entity.h` et `probe.h` en sortent.
4. **Casts 469 → 464**, cinq retirés, aucun ajouté.

### Questions pour l'humain

Aucune.

### Annexe A — scripts (texte exact)

Tous hors dépôt, depuis un dossier d'essais `S` (à adapter) qui contient la copie et ses arbres de travail `wt_p` (`d`) et `wt_x` (`x`), après `make test` dans chacun. `syn.sh`, `objs.sh`, `base.sh`, `sizes.c`, `mut.py` : annexe A du PRD 2.15d, inchangés (`S` adapté ; `mut216.py` = `mut.py` avec `muts216.json` et le côté `x`) ; `lset.py` : annexe A du PRD 2.15c ; `usegraph.py` : annexe A du PRD 2.11 (sans sa première ligne, la clôture Markdown d'ouverture).

`run216.py` :

```python
# run216.py ARBRE : 2.16 sur un arbre propre au commit 2.15d. Chaque remplacement
# doit trouver exactement son texte, sinon arret (AssertionError).
import re, sys, os
T = sys.argv[1]
def sub(f, old, new, cnt=1):
    p = os.path.join(T, f); s = open(p).read()
    assert s.count(old) == cnt, (f, old, s.count(old))
    open(p, 'w').write(s.replace(old, new))
V, H = 'void *self', 'pico8_struct_header_t *self'
Q = ', const pico8_vec_t *qa, const pico8_vec_t *ra)'
sub('src/pico8_struct.h', f'    int (*hd)({V}{Q};', f'    int (*hd)({H}{Q};')
sub('src/pico8_struct.h', f'    struct pico8_qy *(*mm)({V});', f'    struct pico8_qy *(*mm)({H});')
sub('src/pico8_entity.h', f'int pico8_hd_true({V}{Q};', f'int pico8_hd_true({H}{Q};')
sub('src/pico8_entity.c', f'int pico8_hd_true({V}{Q} {{', f'int pico8_hd_true({H}{Q} {{')
sub('src/pico8_planet.h', f'int pico8_jo_hd_dispatch({V}{Q};', f'int pico8_jo_hd_dispatch({H}{Q};')
G = '    return pico8_gk((const pico8_struct_header_t *)self, qa, ra);\n'
for f, d in (('src/pico8_planet.c', 'int pico8_jo_hd_dispatch'), ('src/pico8_research_icon.c', 'static int pico8_ie_hd_dispatch'),
             ('src/pico8_ui_button.c', 'static int pico8_js_hd_dispatch'), ('src/pico8_help.c', 'static int pico8_fv_hd_dispatch')):
    sub(f, f'{d}({V}{Q} {{\n{G}', f'{d}({H}{Q} {{\n    return pico8_gk(self, qa, ra);\n')
sub('src/pico8_probe.h', f'pico8_qy_t *pico8_g__mm({V});', f'pico8_qy_t *pico8_g__mm({H});')
sub('src/pico8_probe.c', 'pico8_qy_t *pico8_g__mm(void *self_ptr) {\n    pico8_struct_header_t *self = (pico8_struct_header_t *)self_ptr;\n',
    f'pico8_qy_t *pico8_g__mm({H}) {{\n')
sub('src/pico8_planet_family.h', f'pico8_qy_t *pico8_ga_mm({V});', f'pico8_qy_t *pico8_ga_mm({H});')
sub('src/pico8_planet_family.c', 'pico8_qy_t *pico8_ga_mm(void *self_ptr) {', 'pico8_qy_t *pico8_ga_mm(pico8_struct_header_t *self_ptr) {')
sub('tests/test_pico8_fv.c', 'cls_of(fv)->hd(fv, ', 'cls_of(fv)->hd(&fv->header, ', 4)
sub('tests/test_pico8_g_.c', 'cls_of(a)->mm(a)', 'cls_of(a)->mm(&a->header)')
sub('tests/test_pico8_g_.c', 'cls_of(b)->mm(b)', 'cls_of(b)->mm(&b->header)')
sub('tests/test_pico8_ui.c', f'static int giver_hd({V}{Q} {{', f'static int giver_hd({H}{Q} {{')
sub('Makefile', ',lien-parent,parente\n', ',lien-parent,parente,void-ptr\n')
print('2.16 : 18 remplacements, 21 occurrences')
```

`neg216.c` :

```c
#include "pico8_struct.h"
#include "pico8_entity.h"
#include "pico8_probe.h"
#include "pico8_planet_family.h"
#if N == 1
int f(const pico8_struct_class_t *c, pico8_vec_t *v) { return c->hd(v, v, v); }
#elif N == 2
struct pico8_qy *f(const pico8_struct_class_t *c, pico8_vec_t *v) { return c->mm(v); }
#elif N == 3
static int t(void *s, const pico8_vec_t *a, const pico8_vec_t *b) { (void)s; (void)a; (void)b; return 1; }
const pico8_struct_class_t k = {.hd = t};
#elif N == 4
static struct pico8_qy *m(void *s) { (void)s; return 0; }
const pico8_struct_class_t k = {.mm = m};
#elif N == 5
int f(pico8_vec_t *v) { return pico8_hd_true(v, v, v) + (pico8_g__mm(v) != 0) + (pico8_ga_mm(v) != 0); }
#elif N == 6
const pico8_struct_class_t k = {.hd = pico8_hd_true, .mm = pico8_g__mm};
const pico8_struct_class_t k2 = {.mm = pico8_ga_mm};
int f(const pico8_struct_class_t *c, pico8_g__t *g) { return c->hd(&g->header, 0, 0) + (c->mm(&g->header) != 0); }
#endif
```

`fncmp.py` :

```python
# fncmp.py A.dis B.dis : fonctions dont les instructions (adresses et cibles de saut locales retirees) different
import sys, re
def fns(p):
    d = {}; cur = None
    for l in open(p):
        m = re.match(r'^[0-9a-f]+ <(.+)>:$', l.strip())
        if m: cur = m.group(1); d[cur] = []; continue
        if cur and '\t' in l:
            parts = l.rstrip('\n').split('\t')
            if len(parts) >= 3:
                ins = re.sub(r'\b[0-9a-f]+ <([^>+]+)(\+0x[0-9a-f]+)?>', r'<\1\2>', parts[2])
                d[cur].append(ins)
    return d
a, b = fns(sys.argv[1]), fns(sys.argv[2])
diff = sorted(k for k in set(a) | set(b) if a.get(k) != b.get(k))
print(len(a), 'fonctions ;', len(diff), 'differentes :', ' '.join(diff))
```

`obs216.c` (construit comme `obs.c` du PRD 2.15c : `cc -std=c99 -Wall -Wextra -Werror -Isrc -c obs216.c && cc -no-pie -o obs216 obs216.o $(ls tmp/obj/src/pico8_*.o | grep -v fixed_test_double) -lm`, lancé depuis la racine de l'arbre) :

```c
/* obs216.c : valeurs rendues par cls->hd et cls->mm sur des instances fraiches ; meme source aux deux bouts. */
#include <stdio.h>
#include "pico8_assets.h"
#include "pico8_bg.h"
#include "pico8_bootstrap.h"
#include "pico8_cart.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_gfx.h"
#include "pico8_help.h"
#include "pico8_input.h"
#include "pico8_mem.h"
#include "pico8_particle.h"
#include "pico8_planet.h"
#include "pico8_planet_family.h"
#include "pico8_planet_family_trade.h"
#include "pico8_probe.h"
#include "pico8_ui_button.h"
static pico8_mem_t mem; static pico8_gfx_t gfx; static pico8_input_t input;
static pico8_vec_t V(int x, int y) { return pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y)); }
static void show(const char *name, pico8_struct_header_t *h) {
    static const int pt[6][2] = {{0, 0}, {64, 64}, {64, 10}, {24, 106}, {70, 70}, {120, 120}};
    int i, j;
    printf("%-3s hd=%s", name, h->cls->hd ? "" : "NULL");
    if (h->cls->hd) for (i = 0; i < 6; i++) for (j = 0; j < 6; j++) {
        pico8_vec_t a = V(pt[i][0], pt[i][1]), b = V(pt[j][0], pt[j][1]);
        printf("%d", h->cls->hd(h, &a, &b));
    }
    printf(" mm=%s", h->cls->mm ? "" : "NULL");
    if (h->cls->mm) for (i = 0; i < 3; i++) {
        pico8_qy_t *q = h->cls->mm(h);
        printf(" {%d,%d %d,%d %d %d,%d %d}", (int)q->p.x, (int)q->p.y, (int)q->v.x, (int)q->v.y, q->has_a, (int)q->a.x, (int)q->a.y, (int)q->l);
    }
    printf("\n");
}
static pico8_struct_header_t *placed(pico8_struct_header_t *h) {
    static pico8_vec_t ras[16]; static int n;
    ras[n] = V(64, 64); ((pico8_jo_t *)h)->ra = &ras[n++]; return h;
}
int main(void) {
    pico8_jo_keys_t keys = {"s99", NULL, NULL};
    pico8_mem_init(&mem); pico8_gfx_init(&gfx);
    if (!pico8_load_gfx_json(&mem, "pico8/gfx.json") || !pico8_load_map_json(&mem, "pico8/map.json")) return 1;
    pico8_input_init(&input); pico8_dispatch_set_gfx(&gfx); pico8_dispatch_set_mem(&mem); pico8_dispatch_set_input(&input);
    pico8_cart_load(); pico8__init(); pico8_eu(NULL);
    show("jo", placed(&pico8_jo_new(&keys)->header));
    show("qh", placed(pico8_qh_kind()->new_entity(&keys)));
    show("nl", placed(pico8_nl_kind()->new_entity(&keys)));
    show("os", placed(pico8_os_kind()->new_entity(&keys)));
    show("hj", placed(pico8_hj_kind()->new_entity(&keys)));
    show("pm", placed(pico8_pm_kind()->new_entity(&keys)));
    show("mx", placed(pico8_mx_kind()->new_entity(&keys)));
    show("nr", placed(pico8_nr_kind()->new_entity(&keys)));
    show("fr", placed(pico8_fr_kind()->new_entity(&keys)));
    show("ga", placed(pico8_ga_kind()->new_entity(&keys)));
    show("g_", &pico8_g__new(NULL)->header);
    show("fv", &pico8_fv_new()->header);
    show("bg", &pico8_bg_new()->header);
    return 0;
}
```

`muts216.json` (`[fichier, ancre ou null, ancien, neuf]`, l'ancien cherché après l'ancre, unique sans ancre ; côtés `p` = `d`, `x`) :

```json
{
 "HD-jo": {
  "p": [
   [
    "src/pico8_planet.c",
    "pico8_jo_hd_dispatch(",
    "    return pico8_gk((const pico8_struct_header_t *)self, qa, ra);\n",
    "    return pico8_gk((const pico8_struct_header_t *)self, ra, qa);\n"
   ]
  ],
  "x": [
   [
    "src/pico8_planet.c",
    "pico8_jo_hd_dispatch(",
    "    return pico8_gk(self, qa, ra);\n",
    "    return pico8_gk(self, ra, qa);\n"
   ]
  ]
 },
 "HD-js": {
  "p": [
   [
    "src/pico8_ui_button.c",
    "pico8_js_hd_dispatch(",
    "    return pico8_gk((const pico8_struct_header_t *)self, qa, ra);\n",
    "    return pico8_gk((const pico8_struct_header_t *)self, ra, qa);\n"
   ]
  ],
  "x": [
   [
    "src/pico8_ui_button.c",
    "pico8_js_hd_dispatch(",
    "    return pico8_gk(self, qa, ra);\n",
    "    return pico8_gk(self, ra, qa);\n"
   ]
  ]
 },
 "HD-ie": {
  "p": [
   [
    "src/pico8_research_icon.c",
    "pico8_ie_hd_dispatch(",
    "    return pico8_gk((const pico8_struct_header_t *)self, qa, ra);\n",
    "    return pico8_gk((const pico8_struct_header_t *)self, ra, qa);\n"
   ]
  ],
  "x": [
   [
    "src/pico8_research_icon.c",
    "pico8_ie_hd_dispatch(",
    "    return pico8_gk(self, qa, ra);\n",
    "    return pico8_gk(self, ra, qa);\n"
   ]
  ]
 },
 "HD-fv": {
  "p": [
   [
    "src/pico8_help.c",
    "pico8_fv_hd_dispatch(",
    "    return pico8_gk((const pico8_struct_header_t *)self, qa, ra);\n",
    "    return pico8_gk((const pico8_struct_header_t *)self, ra, qa);\n"
   ]
  ],
  "x": [
   [
    "src/pico8_help.c",
    "pico8_fv_hd_dispatch(",
    "    return pico8_gk(self, qa, ra);\n",
    "    return pico8_gk(self, ra, qa);\n"
   ]
  ]
 },
 "HDT": {
  "p": [
   [
    "src/pico8_entity.c",
    "int pico8_hd_true(",
    "    return 1;\n",
    "    return 0;\n"
   ]
  ],
  "x": [
   [
    "src/pico8_entity.c",
    "int pico8_hd_true(",
    "    return 1;\n",
    "    return 0;\n"
   ]
  ]
 },
 "HD-bg": {
  "p": [
   [
    "src/pico8_bg.c",
    null,
    "    .hd = pico8_hd_true,\n",
    ""
   ]
  ],
  "x": [
   [
    "src/pico8_bg.c",
    null,
    "    .hd = pico8_hd_true,\n",
    ""
   ]
  ]
 },
 "HD-ga": {
  "p": [
   [
    "src/pico8_planet_family.c",
    "g_ga_struct_class = {",
    "    .hd = pico8_jo_hd_dispatch,\n",
    ""
   ]
  ],
  "x": [
   [
    "src/pico8_planet_family.c",
    "g_ga_struct_class = {",
    "    .hd = pico8_jo_hd_dispatch,\n",
    ""
   ]
  ]
 },
 "MM-g_-t": {
  "p": [
   [
    "src/pico8_probe.c",
    "pico8_g__mm(",
    "        t = self->t;\n",
    "        t = 0;\n"
   ]
  ],
  "x": [
   [
    "src/pico8_probe.c",
    "pico8_g__mm(",
    "        t = self->t;\n",
    "        t = 0;\n"
   ]
  ]
 },
 "MM-g_-self": {
  "p": [
   [
    "src/pico8_probe.c",
    "pico8_g__mm(",
    "    if (self != NULL) {\n",
    "    if (self != NULL && 0) {\n"
   ]
  ],
  "x": [
   [
    "src/pico8_probe.c",
    "pico8_g__mm(",
    "    if (self != NULL) {\n",
    "    if (self != NULL && 0) {\n"
   ]
  ]
 },
 "MM-ga-ms": {
  "p": [
   [
    "src/pico8_planet_family.c",
    "pico8_ga_mm(",
    "    ms = self->ms;\n",
    "    ms = self->iu;\n"
   ]
  ],
  "x": [
   [
    "src/pico8_planet_family.c",
    "pico8_ga_mm(",
    "    ms = self->ms;\n",
    "    ms = self->iu;\n"
   ]
  ]
 }
}
```

### Annexe B — diff (`git diff d x`, texte exact, 233 lignes)

```diff
diff --git a/Makefile b/Makefile
index e9226b4..422546c 100644
--- a/Makefile
+++ b/Makefile
@@ -871,7 +871,7 @@ composition-residue: tools/composition_residue.sh
 	@sh tools/composition_residue.sh
 
 # Catégories du compteur de composition tombées à 0, séparées par des virgules.
-COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6,lien-parent,parente
+COMPOSITION_RESIDUE_GUARD := noyau-inclut,un-ou-faux,aiguillage-nom,atteinte-à-prouver,nu,ptrs-entité,registres,défauts,accesseurs-q6,lien-parent,parente,void-ptr
 
 composition-guard: tools/composition_residue.sh
 	@sh tools/composition_residue.sh --check $(COMPOSITION_RESIDUE_GUARD) src tests
diff --git a/src/pico8_entity.c b/src/pico8_entity.c
index eed82e7..efcc6ec 100644
--- a/src/pico8_entity.c
+++ b/src/pico8_entity.c
@@ -122,7 +122,7 @@ static int pico8_bm_has(const pico8_struct_header_t *e, int i) {
     }
 }
 
-int pico8_hd_true(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
+int pico8_hd_true(pico8_struct_header_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
     (void)self;
     (void)qa;
     (void)ra;
diff --git a/src/pico8_entity.h b/src/pico8_entity.h
index 869b0bd..f6d9cc8 100644
--- a/src/pico8_entity.h
+++ b/src/pico8_entity.h
@@ -44,7 +44,7 @@ void pico8_he(pico8_struct_header_t *e);
 void pico8_dc(const char *nt);
 
 /* `hd=1` of `bg` and `fq`: the hover test that every point passes. */
-int pico8_hd_true(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra);
+int pico8_hd_true(pico8_struct_header_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra);
 
 /* Until pico8_jl() runs, the placeholder struct, with no member and no
  * `md`: dc() calls `mo:md(ci[o])` on a `qb` only; afterwards the
diff --git a/src/pico8_help.c b/src/pico8_help.c
index 5746283..d663021 100644
--- a/src/pico8_help.c
+++ b/src/pico8_help.c
@@ -31,8 +31,8 @@ void pico8_help_set_eu(pico8_js_fn fn) {
     g_eu_dispatch = (fn != NULL) ? fn : pico8_fv_eu_placeholder;
 }
 
-static int pico8_fv_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
-    return pico8_gk((const pico8_struct_header_t *)self, qa, ra);
+static int pico8_fv_hd_dispatch(pico8_struct_header_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
+    return pico8_gk(self, qa, ra);
 }
 
 /* The most items a row of `bo` holds. */
diff --git a/src/pico8_planet.c b/src/pico8_planet.c
index 92d93c3..59245a7 100644
--- a/src/pico8_planet.c
+++ b/src/pico8_planet.c
@@ -32,8 +32,8 @@ static pico8_jf_state_t *g_jo_ir = NULL;
  * `lz` gives the planet its own. */
 static pico8_kw_t g_jo_kw_shared;
 
-int pico8_jo_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
-    return pico8_gk((const pico8_struct_header_t *)self, qa, ra);
+int pico8_jo_hd_dispatch(pico8_struct_header_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
+    return pico8_gk(self, qa, ra);
 }
 
 void pico8_jo_hb_dispatch(void *self) {
diff --git a/src/pico8_planet.h b/src/pico8_planet.h
index 4fa9109..3aa5d07 100644
--- a/src/pico8_planet.h
+++ b/src/pico8_planet.h
@@ -101,7 +101,7 @@ void pico8_jo_pq(pico8_jo_t *self);
 
 /* The dispatchers jo's descriptor holds, for a daughter's `static const`
  * descriptor to name. */
-int pico8_jo_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra);
+int pico8_jo_hd_dispatch(pico8_struct_header_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra);
 void pico8_jo_hb_dispatch(void *self);
 pico8_down_t pico8_jo_gv_dispatch(void *self, const pico8_struct_header_t *s);
 void pico8_jo_hk_dispatch(void *self, pico8_fixed_t yr);
diff --git a/src/pico8_planet_family.c b/src/pico8_planet_family.c
index c4209c1..d43c7e2 100644
--- a/src/pico8_planet_family.c
+++ b/src/pico8_planet_family.c
@@ -347,7 +347,7 @@ void pico8_ga_pq(pico8_jo_t *self) {
     }
 }
 
-pico8_qy_t *pico8_ga_mm(void *self_ptr) {
+pico8_qy_t *pico8_ga_mm(pico8_struct_header_t *self_ptr) {
     const pico8_jo_t *self = (const pico8_jo_t *)self_ptr;
     pico8_fixed_t qd;
     pico8_fixed_t ms;
diff --git a/src/pico8_planet_family.h b/src/pico8_planet_family.h
index 3b69ffb..800271e 100644
--- a/src/pico8_planet_family.h
+++ b/src/pico8_planet_family.h
@@ -136,7 +136,7 @@ void pico8_ga_pq(pico8_jo_t *self);
 /* TROU-7 */
 /* Returns a freshly allocated `pico8_qy_t` (`has_a` 1), one allocation,
  * which `ez` stores into `self.qy`. */
-pico8_qy_t *pico8_ga_mm(void *self);
+pico8_qy_t *pico8_ga_mm(pico8_struct_header_t *self);
 
 void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec_t p);
 
diff --git a/src/pico8_probe.c b/src/pico8_probe.c
index 1634dc9..495453c 100644
--- a/src/pico8_probe.c
+++ b/src/pico8_probe.c
@@ -325,8 +325,7 @@ void pico8_d__dispatch(void *self, pico8_struct_header_t *kv) {
     (void)pico8_d_((pico8_struct_header_t *)self);
 }
 
-pico8_qy_t *pico8_g__mm(void *self_ptr) {
-    pico8_struct_header_t *self = (pico8_struct_header_t *)self_ptr;
+pico8_qy_t *pico8_g__mm(pico8_struct_header_t *self) {
     const pico8_emitter_t *emitter = NULL;
     pico8_fixed_t pn = 0, t = 0, du = 0, py = 0, ki = 0;
     int pk = 0;
diff --git a/src/pico8_probe.h b/src/pico8_probe.h
index 304fcd7..7414b59 100644
--- a/src/pico8_probe.h
+++ b/src/pico8_probe.h
@@ -155,6 +155,6 @@ void pico8_d__dispatch(void *self, pico8_struct_header_t *kv);
 /* TROU-7 */
 /* Returns a freshly allocated `pico8_qy_t` (`has_a` 0), one allocation;
  * `ez` appends it to `qy` and nothing ever frees it. */
-pico8_qy_t *pico8_g__mm(void *self);
+pico8_qy_t *pico8_g__mm(pico8_struct_header_t *self);
 
 #endif
diff --git a/src/pico8_research_icon.c b/src/pico8_research_icon.c
index f433e8c..25fb1ef 100644
--- a/src/pico8_research_icon.c
+++ b/src/pico8_research_icon.c
@@ -198,8 +198,8 @@ static void pico8_ie_kj_dispatch(void *self, const pico8_vec_t *ra) {
 }
 
 /* `ie.hd=gk`: a field assignment in the cart, not a method definition. */
-static int pico8_ie_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
-    return pico8_gk((const pico8_struct_header_t *)self, qa, ra);
+static int pico8_ie_hd_dispatch(pico8_struct_header_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
+    return pico8_gk(self, qa, ra);
 }
 
 static const pico8_struct_class_t g_ie_struct_class = {
diff --git a/src/pico8_struct.h b/src/pico8_struct.h
index 06f6ea4..82718c4 100644
--- a/src/pico8_struct.h
+++ b/src/pico8_struct.h
@@ -144,7 +144,7 @@ struct pico8_kind_rec;
 struct pico8_struct_class {
     void (*kj)(void *self, const pico8_vec_t *ra);
     void (*ev)(void *self, const pico8_vec_t *ra);
-    int (*hd)(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra);
+    int (*hd)(pico8_struct_header_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra);
     void (*hk)(void *self, pico8_fixed_t yr);
     void (*hb)(void *self);
     int (*has_hb)(const void *self);
@@ -156,7 +156,7 @@ struct pico8_struct_class {
     void (*kf)(void *self, pico8_struct_header_t *sw);
     void (*cn)(void *self, struct pico8_jo_trade_proposal *lw);
     void (*ol)(void *self, pico8_vec_t p, pico8_fixed_t a);
-    struct pico8_qy *(*mm)(void *self);
+    struct pico8_qy *(*mm)(pico8_struct_header_t *self);
     pico8_state_fn states[PICO8_STATE_COUNT];
     /* `PICO8_TAG(t)` set: lt() registers every instance under tag t. */
     unsigned tags;
diff --git a/src/pico8_ui_button.c b/src/pico8_ui_button.c
index 1883ea3..f0fd942 100644
--- a/src/pico8_ui_button.c
+++ b/src/pico8_ui_button.c
@@ -34,8 +34,8 @@ static void pico8_js_kj_dispatch(void *self, const pico8_vec_t *ra) {
 }
 
 /* `js.hd=gk`: a field assignment in the cart, not a method definition. */
-static int pico8_js_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
-    return pico8_gk((const pico8_struct_header_t *)self, qa, ra);
+static int pico8_js_hd_dispatch(pico8_struct_header_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
+    return pico8_gk(self, qa, ra);
 }
 
 static pico8_down_t pico8_js_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
diff --git a/tests/test_pico8_fv.c b/tests/test_pico8_fv.c
index c457b55..f67303a 100644
--- a/tests/test_pico8_fv.c
+++ b/tests/test_pico8_fv.c
@@ -419,10 +419,10 @@ static void test_life(void) {
         h = fold_string_member(h, fv->header.me);
         assert(ents_listed(pico8_hv(), fv));
     }
-    assert(cls_of(fv)->hd(fv, vec_ref(64, 10), vec_ref(64, 112)) == 1);
-    assert(cls_of(fv)->hd(fv, vec_ref(64, 112), vec_ref(64, 10)) == 0);
-    assert(cls_of(fv)->hd(fv, vec_ref(24, 106), vec_ref(24, 106)) == 1);
-    assert(cls_of(fv)->hd(fv, vec_ref(23, 106), vec_ref(23, 106)) == 0);
+    assert(cls_of(fv)->hd(&fv->header, vec_ref(64, 10), vec_ref(64, 112)) == 1);
+    assert(cls_of(fv)->hd(&fv->header, vec_ref(64, 112), vec_ref(64, 10)) == 0);
+    assert(cls_of(fv)->hd(&fv->header, vec_ref(24, 106), vec_ref(24, 106)) == 1);
+    assert(cls_of(fv)->hd(&fv->header, vec_ref(23, 106), vec_ref(23, 106)) == 0);
     h = fnv(h, (uint32_t)size_of(fv->emitter->qy));
     check("life", h, LIFE_HASH);
 }
diff --git a/tests/test_pico8_g_.c b/tests/test_pico8_g_.c
index 10c1436..6bd518b 100644
--- a/tests/test_pico8_g_.c
+++ b/tests/test_pico8_g_.c
@@ -550,8 +550,8 @@ static void test_life(void) {
     a = ldown();
     b = pico8_g__new(NULL);
     jc = a_slipway(&middle);
-    h = fold_particle(h, cls_of(a)->mm(a));
-    h = fold_particle(h, cls_of(b)->mm(b));
+    h = fold_particle(h, cls_of(a)->mm(&a->header));
+    h = fold_particle(h, cls_of(b)->mm(&b->header));
 
     h = fold_hq(h, a, planet_p(), 64, 2);
     h = fold_hq(h, a, middle, 64, 3);
diff --git a/tests/test_pico8_ui.c b/tests/test_pico8_ui.c
index 51fff57..65c73f0 100644
--- a/tests/test_pico8_ui.c
+++ b/tests/test_pico8_ui.c
@@ -730,7 +730,7 @@ static pico8_down_t giver_down(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
     return out;
 }
 
-static int giver_hd(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
+static int giver_hd(pico8_struct_header_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
     (void)self;
     (void)qa;
     (void)ra;
```
