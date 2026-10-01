---
title: "Plan 3, étape 2.19 : inclusions inutiles — 169 lignes `#include` retirées (64 dans `src/`, 105 dans `tests/`), aucune ajoutée ; 10 relais gardés ; deux commits, objets identiques, compteur inchangé"
description: "Chaque ligne `#include \"pico8_X.h\"` dont le fichier n'emploie aucun nom de X (deux mesures d'accord : `deadinc` et `misc-include-cleaner`) et dont le retrait, cumulé aux précédents, ne change le diagnostic d'aucune unité qui ouvre ce fichier, est supprimée. Le majorant de 72 du relevé de 2.18 (src/ seul) se résout en 64 retraits dans `src/`, 2 faux morts et 6 relais ; `tests/`, hors relevé, ajoute 105 retraits. Les relais restent : rien n'est ajouté. 2.19a (`src/`) puis 2.19b (`tests/`)."
status: needs-triage
---

## Problem Statement

**Écrit sur `5a9a5ee`** (branche `claude/plan-pilot-3-7zdft0` ; arbres `src` `452f895c…`, `tests` `8407eb57…`, `Makefile` `d4e26ba7…`, `tools` `4b5abf24…`). Aucune ligne du dépôt n'a changé pour écrire ce PRD, hors ce fichier. Aucun `make` lancé dans le dépôt. Les mesures viennent d'une copie jetable (`git archive HEAD`, scratchpad), supprimée depuis. L'implémenteur les refait sur le vrai arbre ; un écart s'explique dans le message de commit. Notation : `x` = `5a9a5ee` (départ, ou un descendant qui ne touche que `docs/`) ; `a` = le commit de 2.19a ; `b` = le commit de 2.19b.

Vu du mainteneur du port : la phase 2 est close (2.18), mais son relevé des restes compte 72 lignes `#include` dont le fichier n'emploie aucun nom propre à l'en-tête. L'humain a décidé, le 2026-10-01, de les retirer maintenant. Chacune ment au lecteur, qui croit à une dépendance qui n'existe pas. Elle cache aussi un détour : un fichier qui compile grâce à elle sans le savoir. Et elle grossit le graphe d'inclusion que 2.21 doit casser. Le relevé dit lui-même que 72 est un **majorant** : un en-tête peut relayer des noms à ceux qui l'incluent. Ce PRD fixe la preuve ligne par ligne, mesure la vraie liste et la découpe.

### Texte du plan, recopié (`docs/plan-3.md` à `5a9a5ee`)

> **Décisions de l'humain (2026-10-01, questions de 2.18)** : […] Inclusions inutiles (reste 3) : retirées maintenant, 2.19.

> - 2.19 — inclusions inutiles (décision de l'humain du 2026-10-01) : chaque ligne `#include` qui n'apporte aucun nom au fichier qui l'écrit, ni à un fichier qui compte sur elle, est retirée ; le relevé de 2.18 en compte 72, un majorant. Aucun compte du compteur ne bouge, aucune paire n'entre.

> - 2.20 — objets passés sans leur type […] À écrire après 2.19.
> - 2.21 — boucles de dépendances entre modules […] À écrire après 2.20.

Règles communes (§7 et phase 2) : `make test` vert ; `make e2e` **26 conformes** ; aucun `*.sha256` modifié ; rien sous `tests/e2e/` ; aucune constante d'empreinte changée ; une note « Découvert à 2.19x » au bout de la ligne du plan, dans le commit.

### D'où vient 72 (relevé de 2.18, reste 3)

PRD 2.18 (`docs/prd/done/plan3-phase2-etape18-sortie.md`, l. 100, Runs 12, G6). Le relevé est `deadinc.py` (annexe A du PRD 2.17a–b, `docs/prd/done/plan3-phase2-etape17ab-affichage.md`, bloc après la l. 494). Il s'appuie sur les motifs de déclaration de `usegraph.py` (annexe A du PRD 2.11, `docs/prd/done/plan3-phase2-etape11-etude-inclusions.md`, bloc après la l. 289). Il a été lancé sur les 148 `.c`/`.h` de `src/` hors raylib. Pour chaque `#include "pico8_X.h"`, il compte les noms déclarés par `X.h` que le fichier emploie ; **0 = `MORT`**.

Trois angles morts, tous mesurés ici :

1. Il ne lit que `#include "pico8_X.h"`. Il ne voit donc ni `tests/` (`"../src/pico8_X.h"`) ni les en-têtes système.
2. Ses motifs ratent des noms : les tables `static const` d'un en-tête, certaines macros. Cela donne de **faux morts**.
3. Il ne voit pas le **relais** : un en-tête qui n'emploie rien de X mais que ses inclueurs utilisent pour obtenir X.

Remesuré à `x` : **72** `MORT` dans `src/`, le même compte (Runs 2).

### Contrats reçus et ce que ce PRD en fait

| Point (source) | Ce que fait 2.19 |
|---|---|
| plan 2.19 : « n'apporte aucun nom au fichier qui l'écrit, ni à un fichier qui compte sur elle » | deux preuves par ligne : **nom** (le fichier n'emploie rien de X) et **compilation** (personne n'en dépend, relais compris) — voir Solution |
| plan 2.19 : « aucun compte du compteur ne bouge, aucune paire n'entre » | aucun include ajouté, donc aucune arête neuve. Constaté quand même : pied, `--detail` octet pour octet, `usegraph.py` → `paires 0` |
| PRD 2.18, reste 3, méthode suggérée : « include direct ajouté là où un détour cassait (règle de 2.17d) » | **non retenue** pour 2.19 (G1) : un relais reste, rien n'est ajouté ; les 10 relais sont listés pour 2.21 |
| PRD 2.1–2.2, surprise 2 : « les objets ne sont pas identiques à la lettre (`__LINE__` des `assert`) ; la variante "ligne vidée" est la preuve à reprendre » | reprise telle quelle (Testing Decisions) |
| règle D.13 / 2.2 : « un fichier qui perd un chemin gagne l'include direct de ce qu'il emploie » | sans objet : un retrait qui ferait perdre un nom employé échoue à la preuve de compilation, et la ligne reste |

## Solution

### Méthode de preuve, par ligne (tranchée)

Une ligne `L` = `#include "…pico8_X.h"` du fichier `F` (`src/` ou `tests/`) est **retirée** si et seulement si les trois preuves passent. Sinon elle **reste**.

1. **Nom (le fichier qui l'écrit).** `F` n'emploie aucun nom de `X.h`, et deux mesures indépendantes doivent l'établir :
   - `deadline.py`, qui est `deadinc.py` avec les numéros de ligne et `tests/` compris, rend `MORT` ;
   - `misc-include-cleaner` (clang-tidy 18) dit « not used directly », **sans et avec** `-DPICO8_CLASS_COVERAGE` (`dead.sh`, annexe A).

   Si les deux mesures ne sont pas d'accord, la ligne reste (8 lignes, ci-dessous).

2. **Compilation (les fichiers qui comptent sur elle).** On vide la ligne : une ligne blanche, les numéros sont gardés. On le fait **cumulativement**, dans l'ordre (fichier, ligne). On compile alors en `-fsyntax-only` avec `-std=c99 -Wall -Wextra -Werror`, sans puis avec `-DPICO8_CLASS_COVERAGE`, chaque unité qui ouvre `F` (fermeture `-MM` à `x`) :
   - chaque `.c` ;
   - chaque `.h` pris seul (`-x c`) ;
   - les trois unités raylib, avec un `raylib.h` vide.

   Aucune unité ne doit gagner un diagnostic (`greedy.py`). Un diagnostic neuf signifie qu'un fichier compte sur `L` (relais) : la ligne est remise et **reste**. L'ordre fixé rend le résultat reproductible quand deux lignes relaient le même nom (G2).
3. **Objets.** Les `.o` sans `-g` de `src/` (hors raylib) et de `tests/`, soit 155 objets, sont comparés au parent :
   - **identiques octet pour octet** sous `-O0 -DNDEBUG`, `-O0 -DNDEBUG -DPICO8_CLASS_COVERAGE` et `-O2 -DNDEBUG` ;
   - sous `-O0`, `assert` actifs, un objet ne peut différer que par les immédiats `mov $<ligne>,%edx` (le `__LINE__` passé à `__assert_fail`), avec `nm` identique.

**Un relais reste, il n'est pas remplacé par l'inclusion directe** (G1). 2.19 ne fait que supprimer des lignes.

### Ce que donnent les preuves à `x`

| Étape | `src/` | `tests/` | Total |
|---|---|---|---|
| lignes `#include "…pico8_X.h"` lues (hors le `.c` de `X` lui-même) | — | — | 1532 |
| `MORT` (`deadline.py`) | **72** (= relevé de 2.18) | 115 | 187 |
| faux morts : `misc-include-cleaner` voit un emploi | 2 | 6 | 8 |
| morts par le nom (les deux mesures d'accord) | 70 | 109 | 179 |
| relais, la ligne seule casse une unité | 4 | 4 | 8 |
| relais de groupe, cassent une unité dans le cumul | 2 | 0 | 2 |
| **retirées** | **64** (41 fichiers) | **105** (38 fichiers) | **169** (79 fichiers) |

Le rapprochement avec le majorant donne **72 = 64 retirées + 2 faux morts + 6 relais**.

**Faux morts (8, restent)** :
- `src/pico8_economy_tiers.c:8` (`game_data.h`) ;
- `src/pico8_fixed.c:2` (`quarter_wave.h`, table `static const`) ;
- `tests/test_pico8_fr.c:38`, `test_pico8_hj.c:37`, `test_pico8_mx.c:39`, `test_pico8_nr.c:35`, `test_pico8_pm.c:35` (`scroll.h`) ;
- `tests/test_pico8_ui.c:40` (`particle.h`).

**Relais (10, restent ; numéros de `x`)** :

| Ligne | En-tête | Qui compte sur elle (premier diagnostic neuf) |
|---|---|---|
| `src/pico8_struct.h:6` | `coverage.h` | 19 `.c` de `src/`, sous `-DPICO8_CLASS_COVERAGE` (`pico8_class_coverage_record`) |
| `src/pico8_mouse_reader.h:6` | `ke.h` | `mouse_reader.c` (`pico8_ke_struct_init`) |
| `src/pico8_scroll.h:8` | `ke.h` | `scroll.c` (`pico8_ke_struct_init`) |
| `src/pico8_research_icon.c:14` | `probe.h` | lui-même : `pico8_probe_t` vient de `probe_comp.h` par `probe.h` |
| `src/pico8_collect.h:5` | `geom.h` | groupe : `collect.h` emploie `pico8_fixed_t`, relayé par `game_data.h:4` **ou** `geom.h:5` ; `:4` part, `:5` reste |
| `src/pico8_slipway.h:6` | `ke.h` | groupe : `planet_family_trade.c` (`pico8_ke_struct_pq`) n'a plus que ce chemin vers `ke.h` une fois les lignes précédentes parties |
| `tests/test_pico8_g_.c:40` | `ui.h` | lui-même (`pico8_bg_t`, relayé par `ui.h`) |
| `tests/test_pico8_ga.c:33`, `test_pico8_nl.c:32`, `test_pico8_os.c:32` | `planet_family_trade.h` | eux-mêmes (`pico8_jo_ib`, relayé) |

**Par en-tête retiré** : `help.h` 23, `ui_button.h` 19, `entity.h` 18, `probe.h` 14, `data_init.h` 14, `ke.h` 10, `planet_family.h` 8, `struct.h` 6, `collect.h` 6, `ptrs.h` 5, `geom.h` 5, `fixed.h` 4, autres 37. La liste exacte, `fichier:ligne` et en-tête, est le texte de `run219.py` (annexe A). Elle est numérotée à `x` pour les deux sous-étapes : 2.19a ne touche pas `tests/`.

**Mise en page** : `src/pico8_bootstrap.h` perd son seul include (l. 4). La ligne blanche qui le suivait part aussi, sinon deux lignes blanches se suivraient sous la garde. `run219.py` le fait. C'est la seule ligne non-`#include` du diff.

### Découpage : deux commits

- **2.19a — `src/`** : 64 lignes, 41 fichiers (20 `.h`, 21 `.c`), plus la ligne blanche de `bootstrap.h`. Arbre `src` attendu **`a63929a4…`**, `tests` inchangé (`8407eb57…`).
- **2.19b — `tests/`** : 105 lignes, 38 fichiers `tests/*.c`. Arbre `tests` attendu **`968e7267…`**, `src` inchangé (`a63929a4…`).

Le budget est d'environ 40 min chacun, preuves comprises : le diff est produit par script, les preuves sont scriptées, et `make test-all` prend environ 15 s. La seule part longue est `make raylib-vendor-setup`, réseau, une seule fois, en 2.19a. Pourquoi deux commits plutôt qu'un : 2.19a touche le binaire du jeu et ses en-têtes, 2.19b seulement les binaires de test. On relit chacun seul, et la preuve d'objets de 2.19a ne mélange pas les 57 objets de test qui diffèrent par `__LINE__`.

## User Stories

1. En tant qu'humain qui a décidé « retirées maintenant », je veux que chaque ligne retirée soit nommée par `fichier:ligne`, afin de relire le diff sans le deviner.
2. En tant que relecteur, je veux deux mesures du « 0 nom » qui se recoupent, afin qu'un nom que les motifs regex ratent (table `static const`, macro) ne fasse pas partir une ligne employée.
3. En tant que relecteur, je veux la preuve que personne ne compte sur une ligne retirée, en-têtes pris seuls et variante couverture comprises, afin d'écarter le relais.
4. En tant que gardien du jeu, je veux des objets identiques octet pour octet sous `-DNDEBUG` et `nm` identique sans, afin de savoir que rien n'a changé dans le binaire.
5. En tant que gardien du compteur, je veux le pied et `--detail` inchangés et `paires 0`, afin que la phase 2 reste close.
6. En tant qu'auteur de 2.21, je veux la liste des 10 relais restants et de qui en dépend, afin de les traiter avec les cycles.
7. En tant que vérificateur, je veux des arbres attendus (hachages) et un relevé final qui rend exactement les 10 relais, afin de juger sans refaire la recherche.

## Implementation Decisions

### Modules

- **2.19a** : les 41 fichiers de `src/` de la liste A. Aucun fichier raylib n'en fait partie : `main_raylib.c`, `pico8_raylib_backend.[ch]` et `main_screencast_viewer.c` n'ont aucune ligne `MORT`. `main_raylib.c` ouvre quatre en-têtes touchés (`cart.h`, `gen.h`, `planet.h`, `ui_button.h`).
- **2.19b** : les 38 fichiers de `tests/` de la liste B, tous `test_pico8_*.c`, dont les trois tests de régression (`gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`). Pour eux seule une ligne d'include part, l'intention ne change pas.
- **Les deux** : `docs/plan-3.md`, une note « Découvert à 2.19a » puis « Découvert à 2.19b » au bout de la ligne 2.19.

### Ce qui change

- Seules des lignes `#include "…"` sont supprimées (plus une ligne blanche dans `bootstrap.h`). **Aucune ligne n'est ajoutée ni modifiée** dans `src/` ou `tests/`. `git diff --numstat x a -- src` doit rendre 0 ajout.
- Aucun commentaire n'est touché : aucune ligne retirée ne porte de commentaire, et aucune n'est précédée d'un commentaire qui la justifie (vérifié).
- `Makefile`, `tools/` et `*.sha256` ne changent pas. Aucune garde neuve (question 2).

### Anti-tassement

- On ne retire pas un relais pour ajouter l'include direct ailleurs (G1). On ne touche pas aux en-têtes système (`<…>`). On ne remplace pas un include par une déclaration anticipée. On ne réordonne pas les blocs d'include.
- On ne retire pas un des 8 faux morts, même si `-fsyntax-only` passe sans lui.
- On ne prend pas un autre ordre que (fichier, ligne) pour départager les relais de groupe.
- On ne retire pas plus de 169 lignes : une ligne qui semblerait morte en cours de route sans figurer dans la liste se signale dans le message de commit, elle n'est pas retirée.

## Implementation Chunks

1. **2.19a** (un commit) : `run219.py <arbre> a` ; preuves P1–P6 (Testing Decisions) entre `x` et `a` ; `make raylib-vendor-setup` (une fois) puis `make raylib` ; note du plan ; commit ; push.
2. **2.19b** (un commit, sur `a`) : `run219.py <arbre> b` ; preuves P1–P6 entre `a` et `b` ; `make raylib` (les en-têtes ne changent pas, mais le contrôle coûte peu) ; note du plan ; commit ; push.

Message de chaque commit : la liste retirée (ou un renvoi au PRD et le hachage d'arbre obtenu), le relevé final (`dead.sh`), le rapport d'objets (nombre d'objets identiques, différents sous `-O0`, `nm`), les comptes et le tableau « Runs ».

## Testing Decisions

- **Aucun TU nouveau ni modifié** dans son intention. C'est un refactor pur : on prouve l'identité du binaire, on ne teste pas un comportement.
- **Prior art** : PRD 2.1–2.2 (objets « ligne vidée », `__LINE__` des `assert`), PRD 2.17a–b (`deadinc.py`, `objs2.sh`), PRD 2.11 (`usegraph.py`), PRD 2.18 (forme des constats).
- **Filets** : `class-coverage-net` et `sanitize-net` sont exigés à chaque commit. Ce n'est pas obligatoire au sens du §7 (aucun constructeur ni durée de vie ne change), mais ils coûtent 6 s et ferment la question.

### Preuves (P1–P6, parent → enfant ; `O` = dossier des scripts de l'annexe A, hors dépôt)

| # | Commande | Attendu à `a` | Attendu à `b` |
|---|---|---|---|
| P1 | `git rev-parse HEAD:src HEAD:tests` | `a63929a4…`, `8407eb57…` | `a63929a4…`, `968e7267…` |
| P2 | `sh O/dead.sh O` (racine de l'arbre) | **115** lignes : les 6 relais de `src/` (`collect.h:4` [ex-`:5`], `mouse_reader.h:6`, `research_icon.c:14`, `scroll.h:8`, `slipway.h:6`, `struct.h:6`) + 109 de `tests/` | **10** lignes, exactement les relais : les 6 ci-contre + `test_pico8_g_.c:37`, `test_pico8_ga.c:31`, `test_pico8_nl.c:31`, `test_pico8_os.c:31` |
| P3 | `sh O/diagall.sh <parent>` et `<enfant>`, puis `diff` | identiques (182 lignes : 10 unités, raylib avec `raylib.h` vide, et `main_class_coverage.c`/`test_class_coverage.c` sans `-D`) | identiques |
| P4 | `sh O/objs.sh` ×4 (`-O0 -DNDEBUG`, `-O0 -DNDEBUG -DPICO8_CLASS_COVERAGE`, `-O2 -DNDEBUG`, `-O0`), parent et enfant, `cmp` | 155 objets ; 0 différent ×3 ; sous `-O0`, les seuls différents sont des `.c` de la liste A, `nm` identique, `objdump -d` ne diffère que par `mov $0x…,%edx` | idem, des `.c` de la liste B ; sur les deux commits, 57 objets différents sous `-O0` au total, 0 autre |
| P5 | `make composition-residue` ; `sh tools/composition_residue.sh --detail src tests \| md5sum` ; `python3 O/usegraph.py .` | `résidu 2 : 0 ligne(s), 0 fichier(s)`, seule ligne ; md5 **égal à celui du parent** (`e15e93c9…` à `x`, 13 lignes) ; `paires 0` | idem |
| P6 | `make test-all` ; `make sanitize-net` ; `make class-coverage-net` ; `make raylib` (après `make raylib-vendor-setup`, jamais lancé) | `test` code 0, 72 « all tests passed », `résidu gardé (…14 catégories…) : 0 ligne(s)`, `float-grep coeur 120 exclus 33 lignes 0`, `garde fichiers 58 en-echec 0` ; `e2e` 26 conformes ; `sanitize: 26 propre(s)` ; `couverture: OK` ; `tmp/main_raylib` lié | idem |

P4 se fait aussi en variante « ligne vidée » : `greedy.py` laisse l'arbre avec des lignes blanches à la place des includes. Les 155 objets y sont identiques **octet pour octet sous les quatre drapeaux, `-O0` compris** (Runs 10). C'est la preuve que rien d'autre que les numéros de ligne ne bouge.

### Greps de Sortie

1. `git diff --numstat x b -- src tests | awk '{a+=$1; d+=$2} END {print a, d}'` → `0 170` (169 includes + la ligne blanche de `bootstrap.h`).
2. `git diff x b -- src tests | grep '^-' | grep -v '^---' | grep -vc '^-#include "'` → `1` (la ligne blanche est la seule ligne retirée qui n'est pas un include).
3. `grep -rhcE '^\s*#\s*include\s*"' src --include='*.[ch]' | paste -sd+ | bc` : `713` à `x`, `649` à `a`. Même chose sur `tests` : `892` à `x` et `a`, `787` à `b`.
4. `grep -c '^#include' src/pico8_bootstrap.h` → `0` à `a`.
5. `git diff --stat x a` → `src/` (41 fichiers) et `docs/plan-3.md` ; `git diff --stat a b` → `tests/` (38 fichiers) et `docs/plan-3.md`. Aucun autre chemin, en particulier ni `Makefile`, ni `tools/`, ni `tests/e2e/`, ni `*.sha256`.

### Tableau « Runs »

L'implémenteur tient le sien, une ligne par lancement. Celui des essais de ce PRD clôt ce fichier (« Runs », en fin).

### Critères d'échec (`FAILED:` pour le vérificateur)

- `FAILED:` si le départ n'est pas `5a9a5ee` ou un descendant qui ne touche que `docs/`, ou si `run219.py` s'arrête sur une assertion. Si les numéros ont bougé, on ne corrige pas la liste à la main : on arrête et on rapporte.
- `FAILED:` si une ligne ajoutée ou modifiée apparaît dans `src/` ou `tests/` (grep 1) ; si une ligne retirée n'est ni de la liste ni la ligne blanche de `bootstrap.h` (grep 2) ; si un commit touche un chemin hors de la grep 5.
- `FAILED:` si P1 ne rend pas les hachages attendus, ou si P2 ne rend pas 115 lignes à `a` et exactement les 10 relais à `b`.
- `FAILED:` si un diagnostic diffère (P3) ; si un objet diffère sous un drapeau `-DNDEBUG`, ou sous `-O0` autrement que par des immédiats `mov $…,%edx` ; si `nm` diffère (P4).
- `FAILED:` si le pied n'est pas `résidu 2 : 0 ligne(s), 0 fichier(s)`, si le md5 de `--detail` diffère du parent, ou si `usegraph.py` rend une paire (P5).
- `FAILED:` si `make test`, `make e2e` (26 conformes), `sanitize-net` ou `class-coverage-net` restent rouges après 3 tentatives ; si `float-grep` ne rend pas `coeur 120 exclus 33` ; si un `*.sha256`, `tests/e2e/`, `tools/` ou le `Makefile` change ; si une constante d'empreinte change.
- `FAILED:` si `make raylib` échoue après un vendoring réussi. Si le vendoring est impossible (réseau), ce n'est pas un échec : le message le dit, et P3 (unités raylib avec `raylib.h` vide) tient lieu de preuve.
- `FAILED:` si un relais est retiré, si un include est ajouté (G1), ou en cas de tassement.
- `FAILED:` s'il manque la note « Découvert à 2.19a/b », si le plan change ailleurs, ou si le message omet le relevé `dead.sh`, le rapport d'objets ou le tableau « Runs ».
- `FAILED:` si l'appli raylib est lancée.

## Out of Scope

- **2.20** : typer les membres de la table de classe, `pico8_state_fn`, les `*_dispatch`, les rappels et la locale `draw`.
- **2.21** : casser les deux composantes du graphe d'usage (4 et 19 modules). C'est là qu'iront les **10 relais** : remplacer un relais par l'include direct ajoute une arête, ce qui relève du démêlage.
- Les en-têtes système (`<…>`) : le relevé de 2.18 ne les compte pas, et ils ne sont pas mesurés ici.
- Les fichiers raylib, qui n'ont aucune ligne candidate.
- Les 8 faux morts.
- Les détours préexistants : un fichier qui emploie un nom qu'il reçoit par un autre de ses includes (PRD 2.2, question 2 : « attendre »).
- Une garde de `make test` contre le retour d'un include mort (question 2). Les noms provisoires (reste 4 : ils restent). L'audio, PSX.

## Further Notes

### Décisions de l'agent à la place de l'utilisateur (G)

- **G1. Un relais reste ; il n'est pas remplacé par l'inclusion directe.**
  - Le texte du plan exclut ces lignes : elles apportent un nom « à un fichier qui compte sur elle ».
  - Sans ajout, « aucune paire n'entre » est vrai par construction, et la preuve d'objets est complète.
  - Le remplacement coûterait au moins 28 lignes ajoutées pour 10 retirées, dont 19 `#include "pico8_coverage.h"` pour la seule ligne `struct.h:6`. Ces arêtes neuves tombent dans le graphe que 2.21 doit casser.
  - La règle de 2.17d (« include direct là où un détour casse ») s'appliquait à une coupe voulue par le plan. Ici, rien n'oblige à casser un détour.
- **G2. Relais de groupe départagés par l'ordre (fichier, ligne).** Deux lignes peuvent apporter le même nom au même fichier, chacune retirable seule mais pas les deux. Le cumul ordonné retire la première et garde la suivante (`collect.h` : `game_data.h:4` part, `geom.h:5` reste ; `planet_family_trade.c` : `slipway.h:6` reste). C'est reproductible par script, et le choix est bon pour `collect.h`, qui ne tire plus les 429 lignes de `game_data.h`.
- **G3. `tests/` est dans le périmètre**, alors que le relevé de 72 ne le lisait pas. Le plan dit « chaque ligne `#include` ». La commande de l'étape nomme `src/` et `tests/`. Et la méthode est la même.
- **G4. Deux mesures de nom plutôt qu'une.** Les motifs de `usegraph.py` ratent les tables `static const` et des macros (8 faux morts). `misc-include-cleaner` ne voit pas l'emploi des fichiers raylib sans `raylib.h`, mais ceux-ci n'ont aucun candidat. L'intersection des deux est la mesure retenue.
- **G5. En-têtes pris seuls dans la preuve de compilation.** Sans eux, un `.h` qui n'emploie que ce que ses inclueurs ont déjà inclus passerait. Le relevé de base a 10 unités avec diagnostic, comparées en ensemble (pas en « vert/rouge »).
- **G6. `bootstrap.h`** : sa ligne blanche orpheline part avec son seul include (une ligne non-include, déclarée).

### Surprises de mesure, pour `plan-pilot`

1. **Le majorant tient presque** : 64 / 72 lignes de `src/` partent. 2 sont de faux morts, que les motifs regex ne voient pas, et 6 sont des relais.
2. **`tests/` en a plus que `src/`** : 105 lignes, surtout `help.h` (23) et `ui_button.h` (19), copiées d'un test à l'autre.
3. **Les retraits ne s'additionnent pas** : 171 lignes sont sûres une par une, mais 6 diagnostics apparaissent quand on les retire ensemble. La preuve doit être cumulative.
4. **`pico8_ke.h`**, inclus par 12 en-têtes qui ne l'emploient pas, part de 10 d'entre eux. `scroll.h`, `mouse_reader.h` et `slipway.h` le relaient à des `.c` qui appellent `pico8_ke_struct_*` sans l'inclure : ce sont des détours à traiter en 2.21.

### Questions pour l'humain

1. Dix inclusions restent alors qu'elles n'apportent rien au fichier qui les écrit, parce que d'autres fichiers obtiennent leurs noms par elles. Les remplacer par l'inclusion directe coûterait près de trente lignes ajoutées, surtout pour la couverture des classes. Je les laisse à l'étape des boucles (2.21). D'accord ?
2. Rien n'empêche une inclusion inutile de revenir. Une garde demanderait `clang-tidy` dans la boucle de test, ce qui n'est pas acquis sur toutes les machines. Recommandation : pas de garde.

### Annexe A — scripts (texte exact)

Hors dépôt, dans un dossier `O` qui contient aussi :
- `usegraph.py`, extrait de l'annexe A du PRD 2.11 (`docs/prd/done/plan3-phase2-etape11-etude-inclusions.md`, bloc après la l. 289) ;
- `deadinc.py`, extrait de l'annexe A du PRD 2.17a–b (`docs/prd/done/plan3-phase2-etape17ab-affichage.md`, bloc après la l. 494) ;
- `stub/raylib.h`, un fichier vide.

Extraction : `awk -v start=N 'NR>start && /^```/ {c++; if(c==2) exit; next} NR>start && c==1' FICHIER`. Les scripts se lancent depuis la racine de l'arbre, ou prennent l'arbre en argument.

`deadline.py` :

```python
# deadline.py ARBRE FICHIER... : une ligne par #include "pico8_X.h" (ou
# "../src/pico8_X.h") du fichier : fichier:ligne, X.h, noms de X.h employes
# (decl de usegraph.py) ; MORT si 0. Le fichier X.c / X.h lui-meme est saute.
import re, sys, os
T = sys.argv[1]
exec(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'usegraph.py')).read().split('owner={}')[0].replace('os.chdir(sys.argv[1])', 'os.chdir(T)'))
for f in sys.argv[2:]:
    raw = open(f).read(); toks = set(re.findall(r'\b[A-Za-z_]\w*\b', strip(raw)))
    for i, l in enumerate(raw.split('\n'), 1):
        m = re.match(r'\s*#\s*include\s*"(?:\.\./src/)?(pico8_\w+)\.h"', l)
        if not m: continue
        h = m.group(1)
        if os.path.basename(f)[:-2] == h: continue
        n = decl.get(h, set()) & toks
        print(f'{f}:{i}\t{h}.h\t{len(n)}' + ('\tMORT' if not n else ''))
```

`tidy.sh` :

```sh
#!/bin/sh
# tidy.sh FICHIER [DRAPEAU...] : depuis la racine de l'arbre, une ligne
# "fichier:ligne<TAB>en-tete" par include pico8_ que misc-include-cleaner
# (clang-tidy 18) dit "not used directly"
f=$1; shift
clang-tidy --quiet --checks='-*,misc-include-cleaner' "$f" -- -std=c99 -x c "$@" 2>/dev/null \
 | sed -n "s|^$(pwd)/\([^:]*\):\([0-9]*\):1: warning: included header \(pico8_[a-z0-9_]*\.h\) is not used directly.*|\1:\2\t\3|p"
```

`dead.sh` :

```sh
#!/bin/sh
# dead.sh OUTILS : depuis la racine de l'arbre, les lignes #include a 0 nom :
# MORT pour deadline.py ET "not used directly" pour tidy.sh, sans et avec
# -DPICO8_CLASS_COVERAGE ; hors fichiers raylib (raylib.h absent sans vendoring)
O=$1; w=$(mktemp -d)
ls src/*.[ch] tests/*.[ch] | grep -vE 'main_raylib|raylib_backend\.[ch]$|main_screencast_viewer' > $w/f
xargs -P4 -n1 sh $O/tidy.sh < $w/f | sort -u > $w/t0
xargs -P4 -I{} sh $O/tidy.sh {} -DPICO8_CLASS_COVERAGE < $w/f | sort -u > $w/t1
python3 $O/deadline.py . src/*.[ch] tests/*.[ch] | grep MORT | cut -f1,2 | sort > $w/m
comm -12 $w/t0 $w/t1 | comm -12 $w/m -
rm -rf "$w"
```

`proof.py` :

```python
# proof.py ARBRE CANDIDATS : pour chaque "fichier:ligne<TAB>en-tete" de
# CANDIDATS, vide la ligne (numeros gardes), compile -fsyntax-only chaque
# unite (.c et .h pris seul, -x c) qui ouvre le fichier, sans et avec
# -DPICO8_CLASS_COVERAGE, raylib.h vide (O/stub) ; une ligne par candidat :
# SUR si aucune unite n'a de diagnostic neuf, sinon RELAIS et les unites
# (avec le premier diagnostic neuf). Restaure le fichier.
import sys, os, re, subprocess, concurrent.futures as cf
T, C = sys.argv[1], sys.argv[2]; os.chdir(T)
STUB = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'stub')
TUS = sorted([os.path.join(d, f) for d in ('src', 'tests') for f in os.listdir(d) if f.endswith(('.c', '.h'))])
VARS = [[], ['-DPICO8_CLASS_COVERAGE']]
BASE = ['cc', '-std=c99', '-Wall', '-Wextra', '-Werror', '-I', STUB, '-x', 'c']
def diag(tu, v):
    r = subprocess.run(BASE + v + ['-fsyntax-only', tu], capture_output=True, text=True)
    return frozenset(l for l in r.stderr.splitlines() if re.search(r': (error|warning|fatal error):', l))
def deps(tu, v):
    r = subprocess.run(BASE + v + ['-MM', tu], capture_output=True, text=True)
    return set(os.path.normpath(p) for p in r.stdout.replace('\\\n', ' ').split()[1:])
jobs = [(t, i) for t in TUS for i in range(2)]
with cf.ThreadPoolExecutor(8) as ex:
    base = dict(zip(jobs, ex.map(lambda j: diag(j[0], VARS[j[1]]), jobs)))
    dep = dict(zip(jobs, ex.map(lambda j: deps(j[0], VARS[j[1]]), jobs)))
    print('# base: %d unites x 2, %d avec diagnostic' % (len(TUS), sum(1 for j in jobs if base[j])), file=sys.stderr)
    for line in open(C):
        loc, h = line.rstrip('\n').split('\t')[:2]; f, n = loc.rsplit(':', 1); n = int(n)
        src = open(f).read(); L = src.split('\n')
        assert '#include' in L[n-1] and h in L[n-1], loc
        aff = [j for j in jobs if j[0] == f or f in dep[j]]
        L[n-1] = ''; open(f, 'w').write('\n'.join(L))
        try:
            res = list(zip(aff, ex.map(lambda j: diag(j[0], VARS[j[1]]) - base[j], aff)))
        finally:
            open(f, 'w').write(src)
        bad = [(j, d) for j, d in res if d]
        tus = sorted(set(j[0] for j, _ in bad))
        if not bad: print(f'{loc}\t{h}\tSUR\t{len(set(j[0] for j in aff))}')
        else: print(f'{loc}\t{h}\tRELAIS\t{len(tus)}\t{" ".join(tus)}\t{sorted(bad[0][1])[0]}')
        sys.stdout.flush()
```

`greedy.py` :

```python
# greedy.py ARBRE SUR : retire, dans l'ordre (fichier, ligne), chaque ligne de
# SUR (lignes vides a la place, numeros gardes) tant que toutes les unites qui
# ouvrent son fichier (deps de l'arbre de depart) gardent leurs diagnostics
# (sans/avec -DPICO8_CLASS_COVERAGE) ; sinon la remet. Une ligne par candidat :
# RETIRE, ou GARDE et la premiere unite en echec avec son diagnostic neuf.
# Laisse l'arbre avec les lignes RETIRE videes.
import sys, os, re, subprocess, concurrent.futures as cf
T, C = sys.argv[1], sys.argv[2]; os.chdir(T)
STUB = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'stub')
TUS = sorted([os.path.join(d, f) for d in ('src', 'tests') for f in os.listdir(d) if f.endswith(('.c', '.h'))])
VARS = [[], ['-DPICO8_CLASS_COVERAGE']]
BASE = ['cc', '-std=c99', '-Wall', '-Wextra', '-Werror', '-I', STUB, '-x', 'c']
def diag(tu, v):
    r = subprocess.run(BASE + v + ['-fsyntax-only', tu], capture_output=True, text=True)
    return frozenset(l for l in r.stderr.splitlines() if re.search(r': (error|warning|fatal error):', l))
def deps(tu, v):
    r = subprocess.run(BASE + v + ['-MM', tu], capture_output=True, text=True)
    return set(os.path.normpath(p) for p in r.stdout.replace('\\\n', ' ').split()[1:])
jobs = [(t, i) for t in TUS for i in range(2)]
cands = []
for line in open(C):
    loc, h = line.rstrip('\n').split('\t')[:2]; f, n = loc.rsplit(':', 1); cands.append((f, int(n), h))
cands.sort()
with cf.ThreadPoolExecutor(8) as ex:
    base = dict(zip(jobs, ex.map(lambda j: diag(j[0], VARS[j[1]]), jobs)))
    dep = dict(zip(jobs, ex.map(lambda j: deps(j[0], VARS[j[1]]), jobs)))
    for f, n, h in cands:
        src = open(f).read(); L = src.split('\n')
        assert L[n-1].lstrip().startswith('#include') and h in L[n-1], (f, n)
        aff = [j for j in jobs if j[0] == f or f in dep[j]]
        L[n-1] = ''; open(f, 'w').write('\n'.join(L))
        bad = [(j, d) for j, d in zip(aff, ex.map(lambda j: diag(j[0], VARS[j[1]]) - base[j], aff)) if d]
        if bad:
            open(f, 'w').write(src)
            print(f'{f}:{n}\t{h}\tGARDE\t{bad[0][0][0]}\t{sorted(bad[0][1])[0]}')
        else:
            print(f'{f}:{n}\t{h}\tRETIRE')
        sys.stdout.flush()
```

`diagall.sh` :

```sh
#!/bin/sh
# diagall.sh ARBRE : diagnostics -fsyntax-only (sans/avec -DPICO8_CLASS_COVERAGE,
# raylib.h vide) de chaque .c et .h de src/ et tests/, numeros de ligne et
# colonne retires, tries
O=$(dirname $(readlink -f $0)); cd "$1" || exit 2
for v in "" "-DPICO8_CLASS_COVERAGE"; do for f in src/*.[ch] tests/*.[ch]; do
  cc -std=c99 -Wall -Wextra -Werror -I $O/stub -x c $v -fsyntax-only $f 2>&1 | grep -E ': (error|warning|fatal error):' | sed -E "s/:[0-9]+:[0-9]+:/:/; s|^|[$v] $f -> |"
done; done | sort
```

`objs.sh` :

```sh
#!/bin/sh
# objs.sh ARBRE OUT DRAPEAUX : objets sans -g de src/*.c (hors raylib) et
# tests/*.c -> OUT/<dossier>_<nom>.o ; "ECHEC <fichier>" si un .c ne compile pas
cd "$1" || exit 2; mkdir -p "$2"
for f in $(ls src/*.c tests/*.c | grep -vE 'main_raylib|main_screencast_viewer|raylib_backend\.c'); do
  cc -std=c99 $3 -c -o "$2/$(dirname $f)_$(basename $f .c).o" $f 2>/dev/null || echo "ECHEC $f"
done
```

`apply.py` :

```python
# apply.py ARBRE LISTE : supprime chaque "fichier:ligne" de LISTE (ligne
# #include verifiee), de bas en haut dans chaque fichier.
import sys, os, collections
os.chdir(sys.argv[1]); by = collections.defaultdict(list)
for l in open(sys.argv[2]):
    loc, h = l.rstrip('\n').split('\t')[:2]; f, n = loc.rsplit(':', 1); by[f].append((int(n), h))
for f, ns in by.items():
    L = open(f).read().split('\n')
    for n, h in sorted(ns, reverse=True):
        assert L[n-1].lstrip().startswith('#include') and h in L[n-1], (f, n); del L[n-1]
    open(f, 'w').write('\n'.join(L))
print(sum(len(v) for v in by.values()), 'lignes,', len(by), 'fichiers')
```

`run219.py` :

```python
# run219.py ARBRE a|b : 2.19a (src/, 64 lignes) sur un arbre propre au commit
# 5a9a5ee, ou 2.19b (tests/, 105 lignes) sur un arbre propre au commit de 2.19a.
# Numeros de ligne du depart de l'etape (5a9a5ee pour les deux listes : 2.19a
# ne touche pas tests/). Chaque ligne doit etre un #include de l'en-tete nomme,
# sinon arret (AssertionError) ; suppression de bas en haut dans chaque fichier.
import sys, os, collections
A = """
src/main_class_coverage.c:15\tpico8_help.h
src/main_class_coverage.c:16\tpico8_ui_button.h
src/pico8_bg.h:6\tpico8_ke.h
src/pico8_bootstrap.h:4\tpico8_struct.h
src/pico8_cart.c:12\tpico8_collect.h
src/pico8_cart.c:18\tpico8_entity_geom.h
src/pico8_cart.c:35\tpico8_research.h
src/pico8_cart.c:43\tpico8_vec.h
src/pico8_cart.h:4\tpico8_struct.h
src/pico8_collect.h:4\tpico8_game_data.h
src/pico8_economy_score.c:13\tpico8_planet_family.h
src/pico8_economy_tiers.c:10\tpico8_planet_family.h
src/pico8_gen.h:4\tpico8_struct.h
src/pico8_help.c:7\tpico8_data_init.h
src/pico8_help.c:9\tpico8_entity.h
src/pico8_help.h:4\tpico8_fixed.h
src/pico8_help.h:5\tpico8_geom.h
src/pico8_ke.c:9\tpico8_game_data.h
src/pico8_menu.c:10\tpico8_collect.h
src/pico8_menu.c:13\tpico8_entity.h
src/pico8_mouse_reader.h:7\tpico8_mem.h
src/pico8_msgbox.c:9\tpico8_collect.h
src/pico8_msgbox.c:16\tpico8_ptrs.h
src/pico8_msgbox.h:7\tpico8_ke.h
src/pico8_particle.c:8\tpico8_entity.h
src/pico8_pi.c:7\tpico8_geom.h
src/pico8_planet.c:11\tpico8_entity.h
src/pico8_planet.h:8\tpico8_geom.h
src/pico8_planet.h:9\tpico8_ke.h
src/pico8_planet_display.c:9\tpico8_data_init.h
src/pico8_planet_display.c:12\tpico8_entity.h
src/pico8_planet_family.c:9\tpico8_collect.h
src/pico8_planet_family.c:18\tpico8_slipway.h
src/pico8_planet_family.h:9\tpico8_probe.h
src/pico8_planet_family_trade.c:8\tpico8_entity.h
src/pico8_planet_family_trade.h:4\tpico8_fixed.h
src/pico8_planet_family_trade.h:5\tpico8_planet_family.h
src/pico8_planet_trade.c:8\tpico8_entity.h
src/pico8_planet_trade.c:11\tpico8_planet_family.h
src/pico8_planet_trade.h:7\tpico8_ptrs.h
src/pico8_probe.h:5\tpico8_ke.h
src/pico8_research_display.c:8\tpico8_collect.h
src/pico8_research_display.c:9\tpico8_data_init.h
src/pico8_research_display.c:10\tpico8_entity.h
src/pico8_research_icon.h:5\tpico8_geom.h
src/pico8_research_icon.h:7\tpico8_ke.h
src/pico8_score_panel.c:9\tpico8_data_init.h
src/pico8_score_panel.c:18\tpico8_msgbox.h
src/pico8_score_panel.h:7\tpico8_ke.h
src/pico8_score_panel_display.c:7\tpico8_collect.h
src/pico8_score_panel_display.c:8\tpico8_data_init.h
src/pico8_score_panel_display.c:10\tpico8_entity.h
src/pico8_score_panel_display.h:4\tpico8_struct.h
src/pico8_slipway_display.c:8\tpico8_entity.h
src/pico8_tech_gate.c:9\tpico8_entity.h
src/pico8_ui.h:9\tpico8_ke.h
src/pico8_ui_button.h:5\tpico8_geom.h
src/pico8_ui_button.h:7\tpico8_ke.h
src/pico8_wormhole.h:7\tpico8_gfx.h
src/pico8_wormhole.h:8\tpico8_ke.h
src/pico8_wormhole.h:10\tpico8_ptrs.h
src/pico8_wormhole.h:11\tpico8_mem.h
src/pico8_wormhole_display.c:8\tpico8_entity.h
src/pico8_wormhole_display.h:4\tpico8_struct.h
"""
B = """
tests/test_pico8_bg.c:25\tpico8_help.h
tests/test_pico8_bg.c:31\tpico8_probe.h
tests/test_pico8_data_init.c:9\tpico8_fixed.h
tests/test_pico8_entity_kind.c:15\tpico8_data_init.h
tests/test_pico8_entity_kind.c:17\tpico8_entity.h
tests/test_pico8_entity_kind.c:21\tpico8_layout.h
tests/test_pico8_entity_kind.c:26\tpico8_research_icon.h
tests/test_pico8_entity_kind.c:28\tpico8_ui.h
tests/test_pico8_entity_kind.c:29\tpico8_vec.h
tests/test_pico8_fm.c:18\tpico8_bootstrap.h
tests/test_pico8_fm.c:23\tpico8_entity.h
tests/test_pico8_fm.c:27\tpico8_help.h
tests/test_pico8_fm.c:34\tpico8_ui_button.h
tests/test_pico8_fq.c:18\tpico8_help.h
tests/test_pico8_fq.c:20\tpico8_ke.h
tests/test_pico8_fr.c:29\tpico8_help.h
tests/test_pico8_fr.c:33\tpico8_planet_family.h
tests/test_pico8_fr.c:35\tpico8_probe.h
tests/test_pico8_fr.c:41\tpico8_ui_button.h
tests/test_pico8_fv.c:21\tpico8_data_init.h
tests/test_pico8_g_.c:34\tpico8_planet_family.h
tests/test_pico8_g_.c:35\tpico8_planet_family_trade.h
tests/test_pico8_g_.c:37\tpico8_research.h
tests/test_pico8_g_.c:41\tpico8_ui_button.h
tests/test_pico8_g_.c:43\tpico8_wormhole.h
tests/test_pico8_ga.c:20\tpico8_bootstrap.h
tests/test_pico8_ga.c:26\tpico8_help.h
tests/test_pico8_ga.c:34\tpico8_probe.h
tests/test_pico8_ga.c:37\tpico8_ui_button.h
tests/test_pico8_ga_export_panel_by.c:13\tpico8_entity.h
tests/test_pico8_headless_capture.c:19\tpico8_gfx.h
tests/test_pico8_hj.c:27\tpico8_help.h
tests/test_pico8_hj.c:39\tpico8_ui_button.h
tests/test_pico8_hp.c:21\tpico8_data_init.h
tests/test_pico8_hp.c:28\tpico8_help.h
tests/test_pico8_hp.c:33\tpico8_planet_display.h
tests/test_pico8_hp.c:35\tpico8_probe.h
tests/test_pico8_hp.c:42\tpico8_ui_button.h
tests/test_pico8_hy.c:22\tpico8_data_init.h
tests/test_pico8_hy.c:29\tpico8_help.h
tests/test_pico8_hy.c:40\tpico8_ui_button.h
tests/test_pico8_ie.c:19\tpico8_bootstrap.h
tests/test_pico8_ie.c:21\tpico8_data_init.h
tests/test_pico8_ie.c:28\tpico8_help.h
tests/test_pico8_ie.c:34\tpico8_probe.h
tests/test_pico8_jb_ol.c:13\tpico8_entity.h
tests/test_pico8_jc.c:17\tpico8_bg.h
tests/test_pico8_jc.c:26\tpico8_help.h
tests/test_pico8_jc.c:37\tpico8_ui_button.h
tests/test_pico8_jo.c:25\tpico8_help.h
tests/test_pico8_jo.c:36\tpico8_ui_button.h
tests/test_pico8_jo_spec_defaults.c:13\tpico8_data_init.h
tests/test_pico8_jo_spec_defaults.c:23\tpico8_probe.h
tests/test_pico8_js.c:21\tpico8_economy.h
tests/test_pico8_js.c:36\tpico8_planet_family.h
tests/test_pico8_js.c:37\tpico8_probe.h
tests/test_pico8_layout_nil_item_draws_nothing.c:28\tpico8_struct.h
tests/test_pico8_lq.c:21\tpico8_data_init.h
tests/test_pico8_lq.c:26\tpico8_help.h
tests/test_pico8_map_tables.c:10\tpico8_planet.h
tests/test_pico8_map_tables.c:11\tpico8_planet_family.h
tests/test_pico8_map_tables.c:12\tpico8_planet_family_trade.h
tests/test_pico8_mh.c:23\tpico8_help.h
tests/test_pico8_mh.c:29\tpico8_ui_button.h
tests/test_pico8_mx.c:28\tpico8_help.h
tests/test_pico8_mx.c:35\tpico8_probe.h
tests/test_pico8_mx.c:41\tpico8_ui_button.h
tests/test_pico8_mz.c:26\tpico8_help.h
tests/test_pico8_mz.c:38\tpico8_ui_button.h
tests/test_pico8_nf.c:16\tpico8_fixed.h
tests/test_pico8_nl.c:27\tpico8_help.h
tests/test_pico8_nl.c:33\tpico8_probe.h
tests/test_pico8_nl.c:37\tpico8_ui_button.h
tests/test_pico8_nr.c:26\tpico8_help.h
tests/test_pico8_nr.c:32\tpico8_probe.h
tests/test_pico8_nr.c:39\tpico8_ui_button.h
tests/test_pico8_os.c:27\tpico8_help.h
tests/test_pico8_os.c:33\tpico8_probe.h
tests/test_pico8_pj.c:18\tpico8_bootstrap.h
tests/test_pico8_pj.c:20\tpico8_data_init.h
tests/test_pico8_pj.c:22\tpico8_economy.h
tests/test_pico8_pj.c:29\tpico8_help.h
tests/test_pico8_pj.c:35\tpico8_probe.h
tests/test_pico8_planet_gh_row_lq.c:25\tpico8_entity.h
tests/test_pico8_pm.c:26\tpico8_help.h
tests/test_pico8_pm.c:33\tpico8_probe.h
tests/test_pico8_pm.c:37\tpico8_ui_button.h
tests/test_pico8_qb.c:23\tpico8_help.h
tests/test_pico8_qb.c:29\tpico8_ui_button.h
tests/test_pico8_qh.c:40\tpico8_ui_button.h
tests/test_pico8_spec_defaults.c:16\tpico8_entity.h
tests/test_pico8_spec_defaults.c:26\tpico8_msgbox.h
tests/test_pico8_spec_defaults.c:28\tpico8_ptrs.h
tests/test_pico8_spec_defaults.c:33\tpico8_slipway.h
tests/test_pico8_trade.c:25\tpico8_help.h
tests/test_pico8_trade.c:33\tpico8_menu.h
tests/test_pico8_trade.c:37\tpico8_ui_button.h
tests/test_pico8_ui.c:24\tpico8_data_init.h
tests/test_pico8_ui.c:26\tpico8_economy.h
tests/test_pico8_ui.c:27\tpico8_economy_tiers.h
tests/test_pico8_ui.c:29\tpico8_entity_geom.h
tests/test_pico8_ui.c:34\tpico8_layout.h
tests/test_pico8_ui.c:42\tpico8_planet_display.h
tests/test_pico8_ui.c:45\tpico8_planet_trade.h
tests/test_pico8_ui.c:47\tpico8_ptrs.h
"""
os.chdir(sys.argv[1]); by = collections.defaultdict(list)
for l in (A if sys.argv[2] == 'a' else B).strip().split('\n'):
    loc, h = l.split('\t'); f, n = loc.rsplit(':', 1); by[f].append((int(n), h))
for f, ns in by.items():
    L = open(f).read().split('\n')
    for n, h in sorted(ns, reverse=True):
        assert L[n-1] in ('#include "%s"' % h, '#include "../src/%s"' % h), (f, n, L[n-1])
        del L[n-1]
    s = '\n'.join(L)
    if f == 'src/pico8_bootstrap.h':
        # son seul include parti : une ligne blanche de trop sous la garde
        assert s.count('#define PICO8_BOOTSTRAP_H\n\n\n') == 1
        s = s.replace('#define PICO8_BOOTSTRAP_H\n\n\n', '#define PICO8_BOOTSTRAP_H\n\n')
    open(f, 'w').write(s)
print('2.19%s : %d lignes, %d fichiers' % (sys.argv[2], sum(len(v) for v in by.values()), len(by)))
```

## Runs

Essais de ce PRD, sur la copie jetable de `x` (supprimée depuis) :

| # | Commande | Arbre | Code | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `git archive HEAD \| tar -x` | `x` | 0 | copie ; arbres `src` `452f895c`, `tests` `8407eb57` (reproduits par `git write-tree`) | 23 s |
| 2 | `deadinc.py` sur les 148 `.c`/`.h` de `src/` hors raylib | `x` | 0 | **72** `MORT` (= 2.18) | < 1 s |
| 3 | `deadline.py` sur `src/` et `tests/` | `x` | 0 | 1532 lignes, 187 `MORT` (72 + 115) | < 1 s |
| 4 | `tidy.sh` ×226 fichiers, sans/avec `-DPICO8_CLASS_COVERAGE` | `x` | 0 | 182 / 179 « not used directly », intersection 179 ; 179 ⊂ `MORT` ; 8 `MORT` non confirmés | 5 s |
| 5 | `proof.py` : chaque ligne seule vidée, unités qui l'ouvrent | `x` | 0 | 171 sûres, **8 relais** | 21 s |
| 6 | `diagall.sh` | `x` | 0 | 182 lignes de diagnostic, 10 unités (raylib vide, couverture sans `-D`) | ~20 s |
| 7 | `objs.sh` ×4 drapeaux | `x` | 0 | 155 objets par drapeau | 36 s |
| 8 | les 171 supprimées d'un coup, `diagall.sh` | — | 1 | 6 diagnostics neufs : `collect.h` (`pico8_fixed_t`), `planet_family_trade.c` (`pico8_ke_struct_pq`) → relais de groupe | ~20 s |
| 9 | `greedy.py` (cumul ordonné) | `x` | 0 | **169 retirées**, 2 gardées (`collect.h:5`, `slipway.h:6`) | ~70 s |
| 10 | `diagall.sh` + `objs.sh` ×4, lignes vidées | `x`+169 vidées | 0 | diagnostics identiques ; 0 objet différent ×4 (`-O0` compris) | 56 s |
| 11 | `apply.py` (suppression réelle), `objs.sh` ×4, `nm`, `objdump -d` | `b` | 0 | 0 / 0 / 0 différents ; `-O0` : 57 différents, `nm` 0 différent, seuls des `mov $…,%edx` (2483 lignes) | 40 s |
| 12 | `composition_residue.sh`, `--detail \| md5sum`, `usegraph.py` | `x`, `b` | 0 | pied `0/0` ; `e15e93c9…` aux deux ; `paires 0` | < 1 s |
| 13 | `make -s test` | `b` | 0 | 72 « all tests passed », garde 14 catégories à 0, `float-grep coeur 120 exclus 33 lignes 0` (idem à `x`), `float_guard.txt` inchangé | 13 s |
| 14 | `make -s e2e` | `b` | 0 | 26 conformes | 1,1 s |
| 15 | `make sanitize-net` ; `make class-coverage-net` | `b` | 0 | `26 propre(s)` ; `couverture: OK` | 3,5 s ; 2,3 s |
| 16 | `diagall.sh` ; `dead.sh` | `b` | 0 | identiques à `x` ; **10** lignes = les relais | 20 s ; 6 s |
| 17 | `apply.py` liste A seule ; `diagall.sh`, `objs.sh -O0 -DNDEBUG`, md5 `--detail`, `make test`, `make e2e` | `a` | 0 | identiques ; 0 différent ; `e15e93c9…` ; 72 ; 26 | ~50 s |
| 18 | `dead.sh` | `a` | 0 | 115 lignes (6 relais `src/` + 109 `tests/`) | 6 s |
| 19 | `run219.py a` puis `b` sur une copie neuve ; `git write-tree` ; relance de `b` | — | 0, 0, 1 | `src` `a63929a4`, puis `tests` `968e7267` ; la relance s'arrête (`AssertionError`) | < 1 s |
| 20 | `diagall.sh` + `objs.sh -O0 -DNDEBUG` sur l'arbre de `run219.py` (avec la ligne blanche de `bootstrap.h`) | `b` | 0 | identiques ; 0 / 155 différent | 30 s |
| 21 | `clang-tidy` sur les 8 faux morts, ligne vidée | `x` | 0 | chacun perd des noms employés (`PICO8_LINEAGE`, `int32_t`/table, `pico8_ents_t`…) | 10 s |
| — | `make raylib` | — | — | **non lancé** : pas de `third_party/` dans l'environnement (vendoring réseau) ; substitut : unités raylib avec `raylib.h` vide, diagnostics identiques (Runs 10, 16) | — |
