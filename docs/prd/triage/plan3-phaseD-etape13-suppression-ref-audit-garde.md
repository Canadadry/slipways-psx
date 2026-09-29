---
title: "Plan 3, étape D.13 : `pico8_ref_audit.h` supprimé avec ses 12 includes, l'arbre `obj-refaudit`, la cible `make ref-audit` et `tools/ref_audit_report.sh` ; lignes de fichiers absents retirées de `tools/line_coverage_reference.txt` ; `ref-audit` = 0 et gardée"
description: "Après D.12d, `pico8_ref_audit.h` n'a plus aucun appelant : en build ordinaire il n'apporte que deux includes déjà présents dans chacun de ses 12 includers, et `make ref-audit` n'enregistre plus aucun site. D.13 supprime l'en-tête, ses includes, tout l'outillage `ref-audit` du `Makefile` et son script, nettoie la référence de couverture, met `ref-audit` dans `DATA_RESIDUE_GUARD` et prouve qu'aucun comportement ne change."
status: needs-triage
---

## Problem Statement

**Écrit sans humain** (`plan-pilot`, `to-prd`), sur l'arbre `b7f6c02` (« docs: Q12 tranchée (oui), étape D.14 = étude D.9b… »), descendant de `655c5c5` qui ajoute la ligne D.13 et de `0a74e49` qui gradue D.12d. Rien n'était en cours. Seuls `git`, `grep`, `sed`, `awk`, une fermeture d'includes par script shell (lecture seule), `sh tools/data_residue.sh` et une simulation des suppressions sur une copie de `src/` et `tests/` dans le scratchpad ont tourné. **Rien n'a été compilé ni lancé.**

Le problème, vu du mainteneur du port : la Sortie de la phase D exige que `pico8_ref_audit.h` disparaisse. Depuis D.12d, il n'en reste qu'un en-tête mort :

- en build ordinaire, il ne fournit que `#include "pico8_type_tag.h"` et `#include <assert.h>` ; tout le reste (l'état par unité de compilation, `pico8_ref_audit_dump`, `pico8_ref_audit_note`, `pico8_ref_audit_expect`) est sous `#ifdef PICO8_REF_AUDIT` et n'a aucun appelant ;
- 12 fichiers de `src/` l'incluent encore, sans rien lui emprunter ;
- le `Makefile` garde pour lui un septième arbre d'objets (`tmp/obj-refaudit`), sa règle de compilation, `REFAUDIT_CFLAGS`, cinq binaires `*_refaudit`, la cible `make ref-audit`, son entrée de légende et son `-include` de dépendances ;
- `tools/ref_audit_report.sh` joint un grep de `PICO8_REF_AS_LIST` (retiré à D.12d) à des tables que plus rien n'écrit : il ne rapporte plus rien ;
- `tools/line_coverage_reference.txt` nomme six lignes de l'en-tête (`:239` à `:246`, au-delà de sa fin actuelle, 171 lignes) et, déjà périmées, 388 lignes de quatre fichiers supprimés.

Tant que ces lignes existent, `ref-audit` (23 lignes, 13 fichiers) ne peut pas entrer dans `DATA_RESIDUE_GUARD`, et un en-tête d'audit pourrait revenir sans que `make test` le voie.

La ligne D.13, la ligne D.14 (contexte), le bloc « Reste de D et report vers la phase 2 » et la ligne D.12d avec sa note de clôture sont recopiés tels quels, par extraction de lignes de `b7f6c02`, en **Annexe A**. L'essentiel :

> - D.13 — `ref-audit`, suppression : `pico8_ref_audit.h`, ses 12 includes dans `src/`, `REFAUDIT_CFLAGS` et la cible `make ref-audit`, `tools/ref_audit_report.sh`, la mention périmée de `tools/line_coverage_reference.txt`. Décidé par l'humain (2026-09-29) avant la réponse à Q12 : pure suppression, sans lien avec les genres mêlés. Compte : `ref-audit` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net`, `sanitize-net` : non. Preuve : une ligne `ref-audit` plantée fait échouer `make data-guard`.

Extrait de la note de D.12d : « `ref-audit` (l'en-tête d'audit sans aucune macro ni appelant : état, `dump`, `note`, `expect` sous `#ifdef PICO8_REF_AUDIT`, 11 lignes ; ses 12 includes ; `make ref-audit` et `tools/ref_audit_report.sh`, qui n'enregistrent plus aucun site). […] `pico8_ref_audit.h` est désormais un en-tête sans appelant : sa suppression (l'en-tête, ses 12 includes, `make ref-audit`, le script) ne dépend d'aucune réponse à Q12. »

### État de départ (mesuré à `b7f6c02`)

| Mesure | Valeur |
|---|---|
| `ref-audit` | **23 lignes / 13 fichiers / 23 occ.**, tout dans `src/` : `pico8_ref_audit.h` 11 / 11, et une ligne (l'include) dans chacun des 12 includers |
| `type-tag` | 249 / 59 / 384 occ. (`src/` 78 / 25 / 103, `tests/` 171 / 34) ; dont **6 lignes / 6 occ. dans `pico8_ref_audit.h`** |
| `valeur` | 1014 / 105 (`src/` 518 / 66, `tests/` 496 / 39) ; aucune dans `pico8_ref_audit.h` |
| résidu D | 1284 lignes / 113 fichiers (`src/` 619 / 74, `tests/` 665 / 39) |
| `DATA_RESIDUE_GUARD` | `spec,overrides,stash,ob,dispatch,par-nom,liste` |
| `tools/float_guard.txt` | `garde fichiers 58 en-echec 0` |
| `tools/line_coverage_reference.txt` | 6362 lignes, 65 fichiers, dont 5 absents de l'arbre (tableau plus bas) |

Recette (motifs figés de `tools/data_residue.sh` ; `--detail` donne les lignes, la recette les occurrences) :

```sh
P='pico8_ref_audit\.h|\bPICO8_REF_[A-Z0-9_]+'
T='\bpico8_type_tag(_of|_t)?\b|\bPICO8_TYPE_TAG_[A-Z0-9_]+|pico8_type_tag\.h'
F=$(find src tests \( -name '*.c' -o -name '*.h' \) -type f | sort)
for c in "ref-audit:$P" "type-tag:$T"; do n=${c%%:*}; p=${c#*:}
  printf '%s\t%s lignes\t%s fichiers\t%s occ.\n' $n "$(cat $F | grep -cE "$p")" \
    "$(grep -lE "$p" $F | wc -l)" "$(cat $F | grep -oE "$p" | wc -l)"; done
```

### Inventaire (numéros de `b7f6c02`)

#### `src/` — 13 fichiers

| Fichier | Lignes retirées | Catégories |
|---|---|---|
| `pico8_ref_audit.h` | **fichier supprimé** (171 lignes) | `ref-audit` 11 (`:1`, `:2`, `:20`, `:25`, `:37`, `:49`, `:64`, `:105`, `:158`, `:169`, `:171`) ; `type-tag` 6 (`:4`, `:121`, `:132`, `:134`, `:136`, `:144`) |
| `pico8_economy_formulas.c` | `:12` `#include "pico8_ref_audit.h"` | `ref-audit` 1 ; **le fichier sort du résidu D** (aucune autre catégorie) |
| `pico8_economy_score.c` | `:16` | `ref-audit` 1 |
| `pico8_economy_tiers.c` | `:15` | `ref-audit` 1 |
| `pico8_menu.c` | `:26` | `ref-audit` 1 |
| `pico8_planet_display.c` | `:23` | `ref-audit` 1 |
| `pico8_planet_family.c` | `:19` | `ref-audit` 1 |
| `pico8_planet_family_trade.c` | `:19` | `ref-audit` 1 |
| `pico8_planet_trade.c` | `:13` | `ref-audit` 1 |
| `pico8_slipway.c` | `:24` | `ref-audit` 1 |
| `pico8_tech_gate.c` | `:11` | `ref-audit` 1 |
| `pico8_ui.c` | `:19` | `ref-audit` 1 |
| `pico8_ui_button.c` | `:14` | `ref-audit` 1 |

`tests/` : **rien**. Aucun TU n'inclut l'en-tête. `tests/test_data_residue.c` nomme la catégorie `ref-audit` dans ses fixtures, dont les chaînes sont coupées en deux (`"pico8_" "ref_audit.h"`) : aucune ligne n'est comptée, et le fichier ne change pas.

#### Ce que chaque includer tenait de l'en-tête (fermeture d'includes, lecture seule)

En build ordinaire, l'en-tête n'apporte que `<assert.h>` et `pico8_type_tag.h` (qui apporte `<string.h>`). Aucun autre en-tête de `src/` n'inclut `<assert.h>`. La fermeture a été calculée sur chaque includer, l'en-tête d'audit retiré :

| Includer | `assert(` | `<assert.h>` direct | `type-tag` employé | `pico8_type_tag.h` reçu par | fonctions de `<string.h>` | `<string.h>` reçu |
|---|---|---|---|---|---|---|
| `economy_formulas` | 3 | `:3` | non | `entity.h` → `struct.h` | 0 | `struct.h` |
| `economy_score` | 3 | `:3` | non | idem | 2 | direct `:5` |
| `economy_tiers` | 9 | `:3` | non | idem | 2 | direct `:5` |
| `menu` | 4 | `:3` | oui | `entity.h`, `struct.h` direct `:8` | 5 | direct `:5` |
| `planet_display` | 7 | `:3` | oui | direct `:24`, `entity.h` | 13 | direct `:6` |
| `planet_family` | 13 | `:3` | oui | direct `:21`, `struct.h` `:8` | 0 | direct `:6` |
| `planet_family_trade` | 10 | `:3` | oui | direct `:18`, `struct.h` `:7` | 0 | direct `:5` |
| `planet_trade` | 4 | `:3` | non | `entity.h` → `struct.h` | 0 | `struct.h` |
| `slipway` | 9 | `:3` | oui | `entity.h` → `struct.h` | 1 | direct `:5` |
| `tech_gate` | 2 | `:3` | non | idem | 1 | direct `:5` |
| `ui` | 11 | `:3` | oui | direct `:16` | 5 | direct `:5` |
| `ui_button` | 3 | `:3` | oui | `entity.h` → `struct.h` | 0 | direct `:5` |

Chaque includer inclut `<assert.h>` directement à la ligne 3, avant tout en-tête du projet, et reçoit `pico8_type_tag.h` au plus tard par `pico8_entity.h` → `pico8_struct.h`, inclus avant l'en-tête d'audit. **Aucune rupture de fermeture attendue** (hypothèse H1).

#### `Makefile` (numéros de `b7f6c02`)

| Ligne(s) | Aujourd'hui | Après |
|---|---|---|
| `:26-27` | légende `# make ref-audit -> diagnostic des casts REF…` / `hors suite` | supprimées |
| `:105` | `# sur ce code, sans rien dire. Ne pas fusionner les sept.` | `… Ne pas fusionner les six.` (seul mot changé) |
| `:109` | `OBJ_REFAUDIT_DIR := $(TMP_DIR)/obj-refaudit` | supprimée |
| `:120` | `REFAUDIT_CFLAGS := -DPICO8_REF_AUDIT` | supprimée |
| `:131` | `refaudit_objs = …` | supprimée |
| `:149-152` | ligne vide et règle `$(OBJ_REFAUDIT_DIR)/%.o: %.c` (3 lignes) | supprimées |
| `:768-800` | commentaire de tête, cinq binaires `*_refaudit` et leurs recettes, `REFAUDIT_BINS`, commentaire, cible `ref-audit`, `.PHONY: ref-audit` et la ligne vide qui suit | supprimées (33 lignes) ; une seule ligne vide reste entre `e2e-ref` et le commentaire de `pico8_class_coverage` |
| `:890` | `DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch,par-nom,liste` | `…,par-nom,liste,ref-audit` |
| `:1002` | `-include $(wildcard $(OBJ_REFAUDIT_DIR)/src/*.d …)` | supprimée |

Soit 43 lignes retirées et 2 modifiées. Les cinq binaires retirés : `pico8_replay_refaudit`, `test_pico8_planet_gh_row_lq_refaudit`, `test_pico8_data_init_refaudit`, `test_pico8_planet_kj_dispatch_refaudit` et `test_pico8_ga_export_panel_by_refaudit`. Deux de ces cibles compilent des tests du §6 (`gh_row_lq` et `ga_export_panel_by`) en mode audit. Les tests eux-mêmes et leurs cibles ordinaires ne changent pas. `DATA_ENGINE_SRCS`, `ENGINE_SRCS` et `E2E_EVENTS` restent, puisqu'ils servent ailleurs.

#### `tools/`, `docs/`

| Chemin | Sort |
|---|---|
| `tools/ref_audit_report.sh` (174 lignes) | **supprimé** |
| `tools/line_coverage_reference.txt` | lignes des fichiers absents retirées (décision plus bas) : 6362 → **5968** |
| `docs/audit-ref-cast-reverse.md` | `:5-7` (liens « Outillage » vers deux fichiers supprimés) réécrites sans lien ; section `## Rejouer l'audit` (`:274-286`) supprimée |
| `docs/plan-3.md` | note « Découvert à D.13 » ; bloc « Reste de D » (décision plus bas) |
| `docs/plan-2.md` (`:305`, `:575`), `docs/prd/done/*` | **inchangés** (historiques) |
| `docs/glossaire-noms.md`, `docs/trous-de-portage.md`, `CLAUDE.md`, `.claude/skills/` | ne nomment pas l'outillage : inchangés |

## Solution

**Une sous-étape, un commit** (D.13), vert seul, vérifié et poussé :

- `src/pico8_ref_audit.h` supprimé ; la ligne `#include "pico8_ref_audit.h"` retirée des 12 includers, et rien d'autre dans ces fichiers ;
- `Makefile` : tout l'outillage `ref-audit` retiré (table plus haut), « sept » arbres deviennent « six », et `ref-audit` entre dans `DATA_RESIDUE_GUARD` ;
- `tools/ref_audit_report.sh` supprimé ; `tools/line_coverage_reference.txt` perd les 394 lignes des cinq fichiers absents de l'arbre ;
- `docs/audit-ref-cast-reverse.md` : les deux passages devenus faux ; `docs/plan-3.md` : la note et le bloc « Reste de D ».

**Sortie de l'étape**, constatée par commande sur le commit :

- `make test` vert, **dont `make data-guard` avec `ref-audit` gardée** ; `make e2e` : **26 conformes** ;
- `make data-residue` : **plus de ligne `ref-audit`** ; `type-tag` **243 / 58** ; `valeur` 1014 / 105 ; résidu D **1255 / 111** ;
- la garde mord : une ligne `ref-audit` plantée fait échouer `make data-guard` (M1) ;
- aucune ligne exécutée perdue : `L` vide (e2e, `gcov`) ; sortie du préprocesseur des 12 includers identique au `__LINE__` près ;
- ces commandes rendent exactement ce qui est écrit (`<parent>` = le parent du commit) :

  ```sh
  # 1. plus aucune trace de l'outillage dans les fichiers suivis (hors le motif du compteur et les fixtures de son TU) : vide
  git grep -nE 'pico8_ref_audit|PICO8_REF_|REFAUDIT|refaudit|ref_audit' -- src tests tools Makefile ':!tools/data_residue.sh' ':!tests/test_data_residue.c'
  # 2. « ref-audit » dans le Makefile : une seule ligne, la garde
  git grep -n 'ref-audit' -- Makefile
  grep -n '^DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit$' Makefile
  # 3. fichiers supprimés : deux « No such file »
  ls src/pico8_ref_audit.h tools/ref_audit_report.sh
  # 4. six arbres d'objets, et le commentaire qui le dit
  grep -cE '^OBJ_([A-Z]+_)?DIR :=' Makefile          # 6
  grep -n 'Ne pas fusionner les six\.' Makefile      # une ligne
  # 5. src/ : aucune ligne ajoutée ; exactement 12 lignes retirées hors du fichier supprimé
  git diff <parent> -- src | grep -c '^+[^+]'        # 0
  git diff <parent> -- src ':!src/pico8_ref_audit.h' | grep '^-[^-]'   # 12 × -#include "pico8_ref_audit.h"
  # 6. référence de couverture : seulement des retraits, tous de fichiers absents ; plus aucun fichier absent
  git diff <parent> -- tools/line_coverage_reference.txt | grep -c '^+[^+]'   # 0
  git diff <parent> -- tools/line_coverage_reference.txt | grep '^-[^-]' | grep -vcE '^-src/pico8_(class|list|ob|stash)\.c:|^-src/pico8_ref_audit\.h:'   # 0
  wc -l < tools/line_coverage_reference.txt           # 5968
  cut -d: -f1 tools/line_coverage_reference.txt | sort -u | while read f; do [ -f "$f" ] || echo "$f"; done   # vide
  # 7. tools/ : exactement deux chemins
  git diff --stat <parent> -- tools                   # ref_audit_report.sh (supprimé), line_coverage_reference.txt
  # 8. comptes
  make -s data-residue; sh tools/data_residue.sh --detail | grep -E '^(type-tag|ref-audit)'
  ```

## User Stories

1. En tant que mainteneur du port, je veux que `pico8_ref_audit.h` disparaisse maintenant qu'aucune macro ni aucun appel ne s'en sert, afin que la Sortie de D ait un fichier de moins à supprimer.
2. En tant que mainteneur du port, je veux que les 12 includers perdent leur `#include "pico8_ref_audit.h"` sans rien gagner d'autre, afin que le diff de `src/` ne soit fait que de retraits.
3. En tant que mainteneur du port, je veux que chaque includer compile toujours sous `-Wall -Wextra -Werror`, afin que la suppression ne casse aucune dépendance d'include passée par l'en-tête.
4. En tant que mainteneur du port, je veux que l'arbre `tmp/obj-refaudit`, `REFAUDIT_CFLAGS`, les cinq binaires `*_refaudit`, `make ref-audit`, sa légende et son `-include` partent ensemble, afin qu'aucune cible du `Makefile` ne nomme un en-tête ou un script absent.
5. En tant que lecteur du `Makefile`, je veux que le commentaire sur les arbres d'objets en dise le vrai nombre, afin de ne pas chercher un septième arbre.
6. En tant que mainteneur du port, je veux que `tools/ref_audit_report.sh` parte avec sa cible, afin qu'aucun script ne greppe une macro qui n'existe plus.
7. En tant qu'utilisateur de `make e2e-gap`, je veux que `tools/line_coverage_reference.txt` ne nomme plus de fichiers absents de l'arbre, afin que l'écart ne compte pas des lignes impossibles à couvrir.
8. En tant que mainteneur du port, je veux que `ref-audit` entre dans `DATA_RESIDUE_GUARD`, afin qu'un en-tête d'audit ou une macro `PICO8_REF_*` qui revient fasse échouer `make test`.
9. En tant que vérificateur, je veux la preuve que la garde mord (une ligne plantée, la garde rouge ; restaurée, verte), afin de ne pas croire une garde vide.
10. En tant que vérificateur, je veux la preuve qu'aucune ligne exécutée n'est perdue (`gcov` sur les e2e, `L` vide), afin de constater que la suppression ne change aucun comportement.
11. En tant que vérificateur, je veux la preuve que la sortie du préprocesseur des 12 includers ne change qu'au `__LINE__` des `assert`, afin qu'« en-tête vide en build ordinaire » soit une mesure et pas une lecture.
12. En tant que mainteneur du port, je veux que `make e2e` reste à 26 conformes au bit près, sans `.sha256` ni fichier de `tests/e2e/` touché, afin que le filet juge et ne soit pas ajusté.
13. En tant que relecteur, je veux la classification de chaque ligne retirée ((a)/(b)/(c)), avec comptes par fichier en lignes et en occurrences, afin de vérifier qu'aucun compte ne baisse par tassement.
14. En tant que relecteur, je veux savoir que les six lignes `type-tag` de l'en-tête partent aussi (249 → 243), afin que la baisse de `type-tag` ne passe pas pour un travail de l'étape `type-tag`.
15. En tant que lecteur de `docs/audit-ref-cast-reverse.md`, je veux que le document ne renvoie plus vers des fichiers ni une commande qui n'existent plus, et que son contenu historique reste intact.
16. En tant que `plan-pilot`, je veux une note « Découvert à D.13 » courte et un bloc « Reste de D » sans les deux points que D.13 ferme, afin que D.14 parte des bons comptes.
17. En tant qu'humain, je veux qu'aucun test ne rejoue un `events.json`, que rien ne soit lent, et que l'appli raylib ne soit jamais lancée, afin que la vérification reste dans les règles du dépôt.

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables) et marqués **(G)**. Les rejets sont notés à chaque décision. La suppression elle-même est une décision humaine (2026-09-29) que ce PRD applique.

### Modules

- **Supprimés** : `src/pico8_ref_audit.h`, `tools/ref_audit_report.sh`.
- **Modifiés, `src/`** : les 12 includers, une ligne retirée chacun, rien d'autre.
- **Build** : `Makefile` (table « `Makefile` »).
- **Outils** : `tools/line_coverage_reference.txt` (retraits seuls).
- **Docs** : `docs/audit-ref-cast-reverse.md` (deux passages), `docs/plan-3.md` (note et bloc « Reste de D »).
- **Inchangés** : `tests/` en entier (dont `test_data_residue.c`), `tools/data_residue.sh` (la catégorie `ref-audit` et son motif restent, c'est la garde), `tools/float_guard.txt` (58 fichiers `.c`, aucun ne part), `pico8_type_tag.h`, `pico8_struct.h`, `docs/plan-2.md`, `docs/prd/done/`, le glossaire.

### Includes (G)

- Chaque `#include "pico8_ref_audit.h"` est **retiré, sans remplaçant** : la fermeture montre que `<assert.h>` (direct, `:3`) et `pico8_type_tag.h` (par `pico8_entity.h` → `pico8_struct.h`) restent reçus partout.
- Aucun include n'est ajouté. Si la compilation dément H1, le fichier gagne l'include **direct** de ce qu'il emploie. `pico8_type_tag.h` gagne alors une ligne `type-tag` et `<assert.h>` aucune. Chaque ajout est déclaré avec l'erreur du compilateur. Rejeté : réordonner ou regrouper les includes (hors sujet).

### `tools/line_coverage_reference.txt` (G)

Le fichier est une référence **« écrite une fois, sur l'ancien filet de longs enregistrements : on ne la recalcule pas, on la compare »** (commentaire du `Makefile`, `:819-820`). Aucune cible ne la régénère. `line-coverage-reference-check` a été supprimée à la fusion de l'e2e rapide (#32), avec le filet dont elle partait.

- **Rejeté : la régénérer.** Aucune cible n'existe pour cela. Le filet d'origine n'existe plus, et un recalcul sur les e2e courts rendrait `make e2e-gap` vide par construction.
- **Rejeté : ne retirer que « la mention de `pico8_list.c` »** (texte du bloc « Reste de D »). La ligne D.13 dit « la mention périmée », et le fichier contient, de même nature, 84 lignes de `pico8_class.c`, 207 de `pico8_ob.c` et 39 de `pico8_stash.c` (fichiers supprimés en 1a et à D.8), ainsi que 6 lignes de `pico8_ref_audit.h`, qui devient absent ici. Aucune autre étape du plan n'a ce fichier pour objet (le PRD de D.12d le renvoyait « à une étape qui en a l'objet »). En laisser 330 obligerait à rouvrir un point du « Reste de D » pour ces seules lignes.
- **Tranché** : retirer **toutes les lignes dont le fichier n'existe plus dans l'arbre après D.13**. Le critère se vérifie d'une commande (grep 6), il ne demande aucun build et c'est le seul sens de « périmé » qu'on peut corriger sans recalcul.

| Fichier absent | Lignes retirées |
|---|---|
| `src/pico8_class.c` | 84 |
| `src/pico8_list.c` | 58 |
| `src/pico8_ob.c` | 207 |
| `src/pico8_stash.c` | 39 |
| `src/pico8_ref_audit.h` | 6 (`:239-241`, `:244-246`) |
| **Total** | **394** (6362 → 5968 ; 65 → 60 fichiers) |

- **Non traité, déclaré** : les numéros de ligne des fichiers qui existent encore ont dérivé depuis #32 (chaque diff de `src/` les décale, D.13 compris : −1 après l'include dans les 12 includers). Les corriger demanderait un recalcul (rejeté). `make e2e-gap` reste hors suite et n'est pas lancé.
- Recette : `grep -vE '^src/pico8_(class|list|ob|stash)\.c:|^src/pico8_ref_audit\.h:'` sur le fichier. L'ordre des lignes restantes est conservé.

### `Makefile` (G)

- Diff exact : la table « `Makefile` ». « Ne pas fusionner les sept » devient « … les six », seul mot changé (piège qu'on réintroduirait, CLAUDE.md). Rejeté : « Ne pas les fusionner » (reformulation hors besoin).
- Les fichiers `.d` des arbres restants (`tmp/obj/src/*.d`, etc.) nomment encore `src/pico8_ref_audit.h` comme prérequis pour les 12 objets. Grâce à `-MP` (`DEPFLAGS`), chaque en-tête y a une cible factice, donc un en-tête supprimé ne casse pas `make`. De toute façon, les 12 `.c` changent et leurs objets sont recompilés (H2).
- `tmp/obj-refaudit/` et les binaires `tmp/*_refaudit` restent sur le disque (non suivis, `tmp/` ignoré). Ils ne sont pas nettoyés à la main.

### `docs/audit-ref-cast-reverse.md` (G)

Document historique (l'audit des casts REF de 1a), cité par `docs/plan-3.md` §1 : il n'est donc pas supprimé. Seuls changent les passages que D.13 rend faux au présent :

- `:5-7` : « Outillage : [liens vers `src/pico8_ref_audit.h` et `tools/ref_audit_report.sh`], cible `make ref-audit`. » devient une phrase sans lien, qui dit que cet outillage a été supprimé depuis et renvoie à l'historique git (sans numéro d'étape) ;
- `## Rejouer l'audit` (`:274-286`, fin du fichier) : supprimée. Elle prescrit une commande qui n'existe plus.
- Le reste (méthode, résultats, au passé ; `:4`, qui renvoie à un PRD déjà déplacé) : **non touché**. Rejeté : réécrire les descriptions de méthode, qui sont historiques.

### `docs/plan-3.md` (G)

- **Note « Découvert à D.13 »** en fin de ligne D.13, limitée à ce qui change une étape suivante (§7) : `type-tag` 249 → 243 (les six lignes de l'en-tête) ; résidu D 1255 / 111 ; `DATA_RESIDUE_GUARD` final ; `line_coverage_reference.txt` sans fichier absent (numéros encore décalés, hors suite) ; tout include direct gagné, s'il y en a. Le reste (chemins, classification, `gcov`) va dans le message de commit.
- **Bloc « Reste de D »** (« une ligne par point ouvert ») : les deux puces que D.13 ferme (`ref-audit (23)…` et `tools/line_coverage_reference.txt : mention périmée…`) sont **retirées**. Dans la puce `valeur`/`type-tag`, « `type-tag` (249) » devient « (243) ». Rien d'autre ne change : ni D.14, ni la Sortie de D, ni les notes des étapes livrées.

### Anti-tassement et message de commit (liant)

Le compteur compte des **lignes**. Une baisse obtenue sans retirer l'outillage est un échec.

- **Interdit** : garder un `pico8_ref_audit.h` vide ou renommé, déplacer son contenu dans un autre en-tête ou hors de `src`/`tests`, un `#define PICO8_REF_*` ou un commentaire qui le nomme « pour mémoire » ; changer le motif de `ref-audit` dans `tools/data_residue.sh` ; toute modification de `tests/`, de `tests/e2e/`, d'un `*.sha256`, de `tools/` hors les deux chemins, du `Makefile` hors sa table ; ajouter une ligne à `tools/line_coverage_reference.txt`.
- **Classification exigée**, au message, pour chaque ligne comptée retirée ou modifiée (recette : `git diff -U0 <parent>`, lignes `-` qui portent un motif, en regard des lignes `+`) :
  - **(a) supprimée sans remplaçant** (fichier supprimé, include inutile) : attendu **23 `ref-audit`** (11 de l'en-tête, 12 includes) **+ 6 `type-tag`** (en-tête : `:4`, `:121`, `:132`, `:134`, `:136`, `:144`) ;
  - **(b) signature ou type qui change** : attendu **0** ;
  - **(c) même donnée, autre nom** : attendu **0**.
- **Mesure exigée**, avant et après, par catégorie (`ref-audit`, `type-tag`, `valeur` ; les sept gardées à 0 constatées) et **par fichier** du diff, en lignes (`--detail`) **et** en occurrences (recette).

| Fichier | `ref-audit` avant → après (lignes / occ.) | `type-tag` avant → après |
|---|---|---|
| `pico8_ref_audit.h` | 11 / 11 → fichier supprimé | 6 / 6 → fichier supprimé |
| chacun des 12 includers | 1 / 1 → 0 / 0 | inchangé (`menu` 1, `planet_display` 1, `planet_family` 1, `planet_family_trade` 2, `slipway` 5, `ui` 2, `ui_button` 1 ; les autres 0) |
| **Total** | **23 / 13 fich. / 23 occ. → 0** | **249 / 59 / 384 → 243 / 58 / 378** (`src/` 78 / 25 / 103 → 72 / 24 / 97) |

- **Lignes qui gagnent une catégorie** : attendu **aucune** (le diff de `src/` n'a aucune ligne `+`). Un include direct ajouté (H1 démentie) est déclaré avec sa catégorie gagnée.
- **Lignes comptées modifiées, non retirées** : aucune attendue.
- **Δlignes > Δoccurrences** : aucun (chaque ligne retirée porte exactement une occurrence de sa catégorie : 23 / 23, 6 / 6).
- **Numéros de ligne** : ceux du **parent du commit**, remesurés, jamais ceux de ce PRD sans remesure.

### Effet sur `make data-residue` (simulé sur une copie de `src/` et `tests/`, sans build)

| Catégorie | Départ | Après D.13 | Pourquoi |
|---|---|---|---|
| `ref-audit` | 23 / 13 | **0, exact** | l'en-tête (11) et 12 includes |
| `type-tag` | 249 / 59 | **243 / 58, exact** | les 6 lignes de l'en-tête |
| `valeur` | 1014 / 105 | **1014 / 105, exact** | l'en-tête n'en porte aucune |
| résidu D | 1284 / 113 (`src/` 619 / 74) | **1255 / 111, exact** (`src/` 590 / 72 ; `tests/` 665 / 39 inchangé) | 29 lignes distinctes ; sortent l'en-tête et `pico8_economy_formulas.c` (plus aucune catégorie) |
| `DATA_RESIDUE_GUARD` | `…,par-nom,liste` | **`…,par-nom,liste,ref-audit`** | `ref-audit` = 0 |

Simulation : `--check spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit src tests` sur la copie rend `0 ligne(s)`, code 0. Avec une ligne `/* pico8_ref_audit.h */` ajoutée en fin de `tests/test_pico8_ptrs.c`, il rend `ref-audit: tests/test_pico8_ptrs.c:225:/* pico8_ref_audit.h */` et `résidu gardé (…,ref-audit) : 1 ligne(s)`, code 1. La garde mordra donc.

### Ce qui ne change pas

- `tests/` en entier ; `tests/e2e/` ; tout `*.sha256` ; aucun e2e ni TU ajouté ou supprimé.
- Le comportement : l'en-tête ne produit aucun code en build ordinaire, et la seule différence de préprocesseur attendue est le `__LINE__` des `assert` situés après l'include (−1), qui ne change que le texte d'un échec d'`assert`. Aucune empreinte e2e ne plie un numéro de ligne.
- `tools/float_guard.txt` : `make test` le réécrit identique (58 fichiers `.c`, un en-tête ne compte pas). Un diff sur ce fichier est un échec à expliquer.
- `pico8_type_tag.h`, l'étiquette `STRUCT`, `valeur` : D.14 et les étapes qui suivront.

### Hypothèses d'exécution (rien n'a été compilé ni lancé)

- **H1** : les 12 includers compilent sans l'en-tête sous les flags du `Makefile` (fermeture plus haut). Si c'est démenti, include direct déclaré (voir « Includes »).
- **H2** : `-MP` rend inoffensifs les `.d` existants qui nomment l'en-tête. Démenti (« No rule to make target `src/pico8_ref_audit.h` ») : supprimer les `.d` des 12 objets **dans `tmp/`** (non suivi), jamais toucher au `Makefile` au-delà de sa table ; le déclarer.
- **H3** : `make test` ne réécrit aucun fichier suivi autrement qu'à l'identique (`float_guard.txt`).
- **H4** : aucun fichier suivi hors de l'Empreinte ne nomme l'outillage. `.claude/worktrees/` (ignoré) contient des copies anciennes qui le nomment : les greps de contrôle passent par `git grep`, jamais par un `grep -r` sur tout l'arbre.

## Implementation Chunks

Une sous-étape, dans cet ordre, verte seule, vérifiée et poussée :

1. **D0 — départ** : HEAD = `b7f6c02` ou un descendant qui ne touche que `docs/`, arbre propre. Recette : `ref-audit` 23 / 13 / 23, `type-tag` 249 / 59 / 384, résidu 1284 / 113. Sinon remesurer et expliquer, avec arrêt si une ligne `ref-audit` existe hors des 13 fichiers. `git grep -n 'pico8_ref_audit\.h' -- src tests` ne rend que les 12 includes. **`make line-coverage EVENTS="$(ls tests/e2e/*.json)"`** : copie de `tmp/line_coverage.txt` dans le scratchpad (départ). **Préprocesseur** : pour chacun des 12 includers, `$(CC) $(CFLAGS) -E -P src/<f>.c` dans le scratchpad (départ).
2. **Édition** : les 12 lignes d'include ; suppression de l'en-tête ; `Makefile` (table) ; suppression du script ; filtre de `line_coverage_reference.txt` ; `audit-ref-cast-reverse.md`.
3. **Filets** : `make test`, puis `make e2e`.
4. **Preuves** : `line-coverage` et préprocesseur à l'arrivée, puis comparaison ; M1 ; greps 1 à 8 ; mesure, classification ; note et bloc « Reste de D » du plan ; **commit**.

## Testing Decisions

Un bon test juge ce que le code rend ou fait, pas la présence d'une ligne. D.13 ne retire que du code sans appelant et de l'outillage hors suite. La preuve est donc que rien de ce qui s'exécute ne change (préprocesseur, `gcov`, 26 e2e au bit près) et que la garde nouvelle mord (M1). Aucun TU n'est écrit ni touché.

### TU touchés

Aucun. §6 : aucun test de régression touché. Seules les variantes `_refaudit` de `gh_row_lq` et `ga_export_panel_by` disparaissent du `Makefile`, et leurs cibles ordinaires restent. Prior art : suppression d'outillage hors suite avec sa cible (le build d'observation de E1, `docs/plan-2.md:575` ; `member-census` à D.1) ; garde qui mord par ligne plantée (D.11e3, M6 de D.12d).

### Couverture perdue, et pourquoi elle est acceptable

| Retrait | Ce qui n'est plus exécuté | Pourquoi rien n'est perdu |
|---|---|---|
| `pico8_ref_audit.h` | rien en build ordinaire ; en mode audit, `note`/`expect`/`dump`, sans appelant | plus aucune macro ni aucun appel depuis D.12d |
| `make ref-audit`, script, binaires | un diagnostic hors suite, vide | il ne rapportait plus aucun site |
| 12 includes | rien | lignes sans code |

`L` (lignes de `src/` exécutées par les 26 e2e au départ et plus à l'arrivée) : **attendu vide**. Pour les 12 includers, chaque ligne de départ `n` située après l'include retiré (ligne `k`) est appariée à `n − 1`, et les autres fichiers se comparent tels quels. Le nombre total de lignes exécutées est **égal** au départ et à l'arrivée (l'en-tête n'en avait aucune). Une ligne de `L` est un comportement perdu : arrêt, rapporté.

### Preuve (§7)

- **`gcov`** : `make line-coverage EVENTS="$(ls tests/e2e/*.json)"` (pilote instrumenté `tmp/pico8_replay_gcov`, arbre `obj-gcov` du `Makefile`), au départ puis à l'arrivée ; appariement ci-dessus ; `L` et les deux totaux rapportés. Il n'y a ni ligne ajoutée ni ligne modifiée dans `src/`, donc rien d'autre à couvrir. Rejeté : l'union `gcov` de tous les TU, comme à D.12d (quelques minutes deux fois). Les seules unités de compilation qui changent sont les 12 includers, et le préprocesseur prouve directement qu'ils ne changent pas.
- **Préprocesseur** : pour chaque includer, `diff` des sorties `-E -P` du départ et de l'arrivée. Attendu : identiques, **ou** ne différant que par des entiers qui baissent de 1 dans des expansions d'`assert` (`__LINE__`). Recette de contrôle : remplacer chaque entier par `N` des deux côtés (`sed -E 's/[0-9]+/N/g'`), puis `diff` vide. Rapporter en plus le nombre de lignes qui diffèrent avant normalisation. Toute autre différence : arrêt, rapportée.
- **Mutation M1, la garde mord** : dans l'arbre de travail, `tests/test_pico8_ptrs.c` est sauvegardé par `cp`, puis une ligne `/* pico8_ref_audit.h */` y est ajoutée en fin de fichier. `make data-guard` doit échouer avec `ref-audit: tests/test_pico8_ptrs.c:<n>:/* pico8_ref_audit.h */` et `résidu gardé (spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit) : 1 ligne(s)`. Le fichier est ensuite restauré par `cp` (jamais `git checkout`/`stash`), et `make data-guard` redevient vert. Aucune autre mutation : la seule logique nouvelle est la garde, et §7 demande des mutations sur ce que `gcov` ne prouve pas, c'est-à-dire ici rien d'autre (déclaré).

### Filets

- `make test` (dont `data-guard` avec `ref-audit`, et `float-guard` qui doit réécrire `float_guard.txt` à l'identique), `make e2e` (26 conformes).
- `make sanitize-net`, `make class-coverage-net` : non, selon la ligne de l'étape (aucune durée de vie, aucun constructeur). `make raylib` : non exigé.
- Aucun TU ne rejoue un fichier d'événements ; `make e2e-gap` n'est pas lancé (hors suite, périmé par dérive, voir plus haut).

### Vérification, budget et tableau « Runs »

**Budget : environ 15 à 20 min de temps réel**, D0, filets et preuves compris ; **plafond 30 min**. Au-delà, rien n'est commis et l'implémenteur note ce qui manque. La relecture par un sous-agent frais est hors budget.

| Cible | Durée attendue |
|---|---|
| recette, `make -s data-residue`, greps | < 1 min |
| `make line-coverage EVENTS=…` (26 e2e), départ et arrivée | < 1 min chacun |
| `-E -P` des 12 includers, départ et arrivée | secondes |
| éditions (12 lignes, `Makefile`, filtre, doc) | ~5 min |
| `make test` | de l'ordre de la minute (12 objets recompilés, liens) |
| `make e2e` | ~1 s |
| M1 (`make data-guard` × 2) | secondes |
| mesure, classification, message, note | ~5 min |

**Jamais l'appli raylib.** Rien de lent : ni balayage ASan, ni union `gcov` des TU, ni `minimize`.

**L'implémenteur tient un tableau « Runs »** dans son journal, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + étape) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|
| 1 | `make -s data-residue` | `b7f6c02`, avant D.13 | 0 | `ref-audit 23 13` ; résidu 1284 / 113 | … |
| 2 | recette `ref-audit`/`type-tag` | idem | 0 | `ref-audit 23 lignes 13 fichiers 23 occ.` | … |
| 3 | `make line-coverage EVENTS="…26…"` | idem | 0 | `couverture de lignes: N ligne(s)` | … |
| 4 | `cc … -E -P` × 12 | idem | 0 | 12 fichiers écrits | … |
| … | … | … | … | … | … |

Le vérificateur le réutilise et ne relance que ce qui suit le dernier changement de l'arbre.

### Contrôles du vérificateur

1. **Périmètre** : `git show --stat` ne liste que les 18 chemins de l'Empreinte (`src/pico8_ref_audit.h` ✗, les 12 includers, `Makefile`, `tools/ref_audit_report.sh` ✗, `tools/line_coverage_reference.txt`, `docs/audit-ref-cast-reverse.md`, `docs/plan-3.md`). S'y ajoute seulement un include direct prouvé nécessaire, qui reste dans un des 12 fichiers. Ni `tests/`, ni `tests/e2e/`, ni `*.sha256`, ni `tools/float_guard.txt`.
2. **Greps** 1 à 8 de la « Sortie », exactement.
3. **`Makefile`** : diff = la table, rien d'autre ; six arbres d'objets ; `DATA_RESIDUE_GUARD` finit par `,liste,ref-audit`.
4. **Anti-tassement** : comptes avant/après en lignes ET occurrences, par catégorie et par fichier ; classification présente et juste ((a) 23 `ref-audit` + 6 `type-tag`, (b) = (c) = 0, ou écart expliqué) ; lignes gagnées « aucune » ; aucune cale ni aucun alias.
5. **Garde** : `make data-guard` vert ; M1 rapporté (rouge avec la ligne plantée, vert une fois restaurée).
6. **Preuve** : `L` vide et totaux égaux ; préprocesseur identique après normalisation ; 26 e2e conformes.
7. **Docs** : `audit-ref-cast-reverse.md` sans lien ni commande vers l'outillage supprimé, le reste inchangé ; note « Découvert à D.13 » courte ; bloc « Reste de D » sans les deux puces fermées, avec `type-tag (243)` ; rien d'autre dans le plan.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `b7f6c02` ou un descendant qui ne touche que `docs/`, propre ; si `ref-audit` n'y vaut pas 23 / 13 / 23 occ. ou `type-tag` 249 / 59 sans explication ; si une ligne `ref-audit` existe hors des 13 fichiers de l'inventaire.
- `FAILED:` si le filet (`make test`, `make e2e`) reste rouge après **3 tentatives** sur le commit.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, si un e2e est ajouté ou retiré, ou si un TU rejoue un fichier d'événements.
- `FAILED:` si un test de régression du §6 (`gh_row_lq`, `ga_export_panel_by`, `layout_nil_item_draws_nothing`) change.
- `FAILED:` si une ligne ne tombe sous aucune décision de ce PRD ni du §5 (arrêt « champ sans règle ») ; si un écart port/cart découvert n'est pas préservé et décrit par une entrée `TROU-N`.
- `FAILED:` si l'appli raylib est lancée.

**Périmètre et forme**

- `FAILED:` si `pico8_ref_audit.h` ou `tools/ref_audit_report.sh` survit, sous ce nom ou un autre, ou si leur contenu est déplacé ailleurs.
- `FAILED:` si un fichier de `src/` gagne une ligne autre qu'un include direct prouvé nécessaire par une erreur de compilation citée ; si un includer perd autre chose que sa ligne d'include.
- `FAILED:` si le `Makefile` change hors de sa table ; si `tools/` change hors des deux chemins ; si `tools/line_coverage_reference.txt` gagne une ligne, perd une ligne d'un fichier existant, ou garde une ligne d'un fichier absent ; si `tools/data_residue.sh` ou `tests/test_data_residue.c` change.
- `FAILED:` si une grep de la « Sortie » ne rend pas ce qui y est écrit.

**Gardes, comptes et anti-tassement**

- `FAILED:` si `make data-guard` n'est pas vert, si `DATA_RESIDUE_GUARD` ne vaut pas `spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit`, ou si M1 ne fait pas échouer la garde.
- `FAILED:` si `ref-audit` ne vaut pas **exactement 0** ; si `type-tag` ne vaut pas 243 / 58 ou `valeur` 1014 / 105 sans explication (un include direct gagné l'explique).
- `FAILED:` **si un compte baisse par tassement** (cale, alias, renommage, déplacement hors `src`/`tests` ou hors `.c`/`.h`) ; (b) ou (c) non nulle sans justification.
- `FAILED:` si le **message du commit** omet l'un de ces éléments : par catégorie et par fichier, les comptes avant/après **en lignes et en occurrences** ; la classification (a)/(b)/(c) avec numéros du parent ; les lignes qui gagnent une catégorie (« aucune » compris) ; les lignes comptées modifiées non retirées (« aucune ») ; Δlignes/Δoccurrences ; les includes retirés ou ajoutés ; le détail des 394 lignes retirées de la référence par fichier ; le rapport `gcov` (`L`, totaux) ; le rapport du préprocesseur ; M1. Un commit de rattrapage ne compense pas un message incomplet.
- `FAILED:` si `make constants-check`, `float-grep` ou `float-guard` rougit, ou si `tools/float_guard.txt` change.

**Preuve, commit et note**

- `FAILED:` si `L` n'est pas vide ou si les totaux diffèrent ; si le préprocesseur montre une différence autre qu'un entier ; si M1 manque ou ne se restaure pas par `cp`.
- `FAILED:` si le plafond est dépassé sans note, ou s'il manque le tableau « Runs ».
- `FAILED:` s'il manque la note « Découvert à D.13 », si le bloc « Reste de D » garde une puce fermée par D.13, ou si le plan change ailleurs.
- `FAILED:` si un commentaire est ajouté ou reformulé dans `src/` (aucun n'est attendu, sauf le mot « six » du `Makefile`), ou si un renvoi à D.13 ou au plan apparaît dans `src/`, `tests/` ou le `Makefile`.

## Out of Scope

- `valeur`, `type-tag`, `pico8_type_tag.h`, l'étiquette `STRUCT`, `pico8_value.c` et toute API à valeur : D.14 (étude D.9b) et les étapes qu'elle ordonnera.
- Le recalcul ou le recalage des numéros de `tools/line_coverage_reference.txt` pour les fichiers qui existent ; `make e2e-gap`.
- `docs/plan-2.md`, `docs/prd/done/`, le contenu historique de `docs/audit-ref-cast-reverse.md`, et son `:4` (lien vers un PRD déjà déplacé).
- Le nettoyage de `tmp/` et de `.claude/worktrees/` (non suivis).
- `make sanitize-net`, `make class-coverage-net`, `make raylib`, l'appli raylib, l'audio, le rendu, tout e2e.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd`), tranchées en `guideline` et marquées (G) :
  - **Includes** : retirés sans remplaçant, avec la fermeture comme preuve ; si la compilation la dément, include direct déclaré.
  - **`line_coverage_reference.txt`** : pas de régénération (aucune cible, filet d'origine disparu) ; retrait de toutes les lignes de fichiers absents (394), pas seulement celles de `pico8_list.c` ; dérive des numéros laissée, déclarée.
  - **`Makefile`** : outillage `ref-audit` retiré en entier, y compris la légende, le `-include`, la règle d'objets et les cinq binaires ; « sept » devient « six ».
  - **`audit-ref-cast-reverse.md`** : gardé (cité par le plan), purgé de ses deux passages au présent devenus faux.
  - **Plan** : note courte et bloc « Reste de D » mis à jour (deux puces retirées, `type-tag` 243).
  - **Preuve** : `gcov` sur les e2e et préprocesseur au lieu de l'union `gcov` des TU (budget 15 à 20 min) ; une seule mutation, la garde.
  - **Découpage** : un commit, 18 chemins, sans repli (l'étape est déjà la plus petite possible).
- **Surprises de mesure, pour `plan-pilot`** :
  1. **L'en-tête porte aussi 6 lignes `type-tag`** (son include de `pico8_type_tag.h`, et `pico8_type_tag_of`/`pico8_type_tag_t` dans `note`/`expect`) : `type-tag` passe de 249 à 243 sans aucun travail de l'étape `type-tag`, et le compte « `type-tag` (249) » du « Reste de D » doit suivre.
  2. **`pico8_economy_formulas.c` sort du résidu D** : son include d'audit était sa seule ligne comptée (résidu 113 → 111 fichiers).
  3. **`pico8_ref_audit.h` était le seul en-tête de `src/` à inclure `<assert.h>`**. C'est sans conséquence, puisque les 12 includers l'incluent eux-mêmes à la ligne 3, mais c'est le seul risque de rupture de fermeture, vérifié.
  4. **`tools/line_coverage_reference.txt` est périmé bien au-delà de `pico8_list.c`** : 394 lignes de 5 fichiers absents, et des numéros décalés partout depuis #32. Il n'existe aucune cible pour le régénérer (« on ne la recalcule pas »).
  5. **Le `Makefile` compte ses arbres d'objets en toutes lettres** (« Ne pas fusionner les sept ») : le retrait de `obj-refaudit` rend ce mot faux.
  6. **`docs/audit-ref-cast-reverse.md` renvoie vers l'outillage** (liens et section « Rejouer l'audit ») : ces deux passages deviennent faux ; le document reste, puisque le plan §1 le cite.
  7. `.claude/worktrees/agent-…` (ignoré par git) contient une copie ancienne de `src/` qui nomme l'en-tête : un `grep -r` naïf la ramasse, d'où `git grep` dans les contrôles.
  8. HEAD a avancé pendant la rédaction (`655c5c5` → `b7f6c02`, ligne D.14 ajoutée, docs seuls) : les comptes sont inchangés.
- **Pour la suite** : après D.13, la Sortie de D ne garde que `pico8_value.c` et `pico8_type_tag.h` ; `plan-pilot` passe à D.14.

## Annexe A — textes du plan recopiés (`docs/plan-3.md` à `b7f6c02`, par extraction de lignes)

### Ligne D.13 (l. 281)

- D.13 — `ref-audit`, suppression : `pico8_ref_audit.h`, ses 12 includes dans `src/`, `REFAUDIT_CFLAGS` et la cible `make ref-audit`, `tools/ref_audit_report.sh`, la mention périmée de `tools/line_coverage_reference.txt`. Décidé par l'humain (2026-09-29) avant la réponse à Q12 : pure suppression, sans lien avec les genres mêlés. Compte : `ref-audit` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net`, `sanitize-net` : non. Preuve : une ligne `ref-audit` plantée fait échouer `make data-guard`.

### Ligne D.14 (l. 282), contexte

- D.14 — étude D.9b, lecture seule (Q12 tranchée oui par l'humain, 2026-09-29 : « fais le tri d'abord », le but étant de supprimer la couche Lua) : pour chaque champ, retour ou paramètre encore en `pico8_value_t` dont le genre varie (nil, faux, nombre, chaîne, référence selon le cas), la règle C qui le remplace (drapeau `has_x` + type, pointeur NULL, petite union locale, ou autre), prouvée par les écritures du cart et du port ; puis la liste ordonnée des étapes `valeur`/`type-tag` qui videront ces catégories, sur le modèle de D.9. Les questions à l'humain sont posées en français simple, sans noms minifiés ni numéros de TROU, 100 mots au plus chacune.

### Bloc « Reste de D et report vers la phase 2 » (l. 284-290)

**Reste de D et report vers la phase 2** (revue du plan en fin de D ; une ligne par point ouvert, le détail est dans l'historique git) :

- `valeur` (1014), `type-tag` (249) : étapes après Q12. Elles emportent les 13 copies privées de `*_truthy(pico8_value_t)` et les includes inutiles de `pico8_value.h` (motif `valeur`). Restes nommés : `pico8_lq_keys_t.items`, `pico8_lq_bg`, `pico8_pj_ed`, `pico8_jc_ce`, `pico8_jy_qe`, `pico8_oj` ; 29 prototypes publics à `pico8_value_t` (96 au début de D ; l'étude disait 62, compte d'une autre méthode).
- `ref-audit` (23) : `pico8_ref_audit.h` sans appelant, ses 12 includes, `make ref-audit`, `tools/ref_audit_report.sh` ; suppression sans lien avec Q12, soumise à l'humain avec Q12 (D.13 si acceptée).
- `tools/line_coverage_reference.txt` : mention périmée de `pico8_list.c`.
- `hp_pose` (`research.c`) : aiguillage nom → membre à la construction, hors `par-nom` ; phase 2 ou étape `valeur`.
- Phase 2 reprend : le lien parent (Q5, `parent()` dans `pico8_struct_class_t`) et les `pico8_X_of` ; les composants émetteur et sonde (Q7, `emitter_of`/`probe_of`), faits sans le test ponctuel d'archétype ; les onze accesseurs Q6, premiers remplacés par le noyau et les composants ; les tags `nu` en masque de bits (reportés de D).

### Ligne D.12d avec sa note de clôture « Découvert à D.12d » (l. 280)

- D.12d — `liste`, TU (431 lignes, 38 fichiers ; `test_pico8_list` réécrit sur le nouveau conteneur, il ne perd pas son objet, §6), en deux chunks ; `pico8_list.c/.h` supprimés, `PICO8_REF_AS_LIST*` et `PICO8_TYPE_TAG_LIST` retirés. Empreinte 42 (4, 38). Compte : `liste` = 0, ajouté à `DATA_RESIDUE_GUARD`. §6 : aucun. `class-coverage-net`, `sanitize-net` : non. Découvert à D.12d (clôt D.12 et la liste de D) : un commit, une sous-étape (pas le repli d1/d2), 27 chemins : `src/pico8_list.c`, `src/pico8_list.h`, `tests/test_pico8_list.c` supprimés ; `pico8_ref_audit.h`, `pico8_type_tag.h`, `pico8_value.h` ; 17 TU (`bg`, `fq`, `fv`, `hp`, `hy`, `iv`, `jb_ol`, `je`, `js`, `lq`, `mh`, `mo`, `mz`, `nf`, `planet_gh_row_lq` §6 include seul, `qb`, `ui`) ; `Makefile`, `tools/float_guard.txt` (réécrit par `make test`, 59 → 58), `glossaire-noms.md`, ce plan. Lectures de la ligne : « réécrit sur le nouveau conteneur » = absorbé par `test_pico8_ptrs` (ses cas tiennent les sept de `test_pico8_list`, prouvé par sept mutations de `pico8_ptrs.c`, M3a–M3g, toutes rouges ; `del` par valeur retiré par Q9, `pico8_value_equal` reste testé par `test_pico8_value`) ; un commit au lieu de « deux chunks » (79 lignes de TU dans 18 fichiers, pas 431/38) ; `pico8_ref_audit.h` réduit de ses parties liste (`PICO8_REF_AS_LIST*`, `pico8_ref_as_list(_const)`, `PICO8_REF_AUDIT_EXPECT_LIST` sous ses deux formes, cas `LIST` de `note`, `#else` vidé), pas supprimé ; `PICO8_TYPE_TAG_LIST` retiré, `pico8_type_tag_of` ne reconnaît plus que `STRUCT` ; commentaires devenus faux réécrits (`value.h:6-9` dont `:7`, `:43-45`, `type_tag.h:6-8`, `:19-24`, tête de `ref_audit.h`, et deux de ses commentaires de fonction qui nommaient les macros et les primitives de liste ; `fold_value` de `hp`, `hy`, `lq`) ; constantes figées `FROZEN_LIST_TAG` (neuf TU) et `VIEW_LIST_TAG` (`hp`) intactes, 0x9E5C1157u ; `tools/line_coverage_reference.txt` et `tools/ref_audit_report.sh` nomment encore le module, périmés, non touchés. D0 : `gcov` par TU (47) : `pico8_list.c` exécuté par `test_pico8_list` (59 lignes) et, hors lui, seulement par `pico8_list_init` (6 lignes) appelé depuis les deux lignes (a1) de `fv:612` et `ui:826` elles-mêmes (aucun appelant de `src/`) ; ces deux lignes converties, 0 ligne ; e2e (`make line-coverage`, 26 rejeux) : 0 ligne ; blocs « liste » des `fold_value` : `hy`, `lq` jamais atteints, `hp:137` évalué 688 fois, branche jamais prise, corps à 0. `gcov` d'arrivée : `L` = 0 (6901 → 6842 lignes de `src/*.c`, les 59 partantes sont `pico8_list.c`) ; `pico8_type_tag.h:48` (ex-`:51`) exécutée. Mutations M1 (`ke:ks`), M2 (`f_`) : `ui`, `fv` rouges (l'enfant ne meurt plus par `SIGABRT` sur le `pico8_ptrs_t` vide) ; M4 (`if (1)`) : 11 TU rouges (`qh`, `nl`, `hj`, `mx`, `os`, `pm`, `hp`, `js`, `fv`, `g_`, `entity_kind`) ; M5 (`if (0)`) : 44 ; M6 : une ligne `/* pico8_list_t */` plantée dans `test_pico8_ptrs.c` fait échouer `make data-guard` (« 1 ligne(s) »), restaurée : 0. Balayage ASan+UBSan des 46 TU propre ; `sanitize-net` 26 propres, `class-coverage-net` identique. `liste` 129/23/157 occ. → **0** (retirées : (a1) 4, (a2) 125, (b) 0, (c) 0 ; aucune ligne gagnée) ; `DATA_RESIDUE_GUARD` = `spec,overrides,stash,ob,dispatch,par-nom,liste`. **Remise des comptes de la phase D** (`make data-residue`, lignes / fichiers, `src/` et `tests/`) : `valeur` **1014 / 105** (`src/` 518 / 66, `tests/` 496 / 39 ; 1054 / 108 au départ) ; `type-tag` **249 / 59** (`src/` 78 / 25, `tests/` 171 / 34 ; 262 / 61) ; `ref-audit` **23 / 13** (`src/` seul ; 40 / 14) ; `ob`, `stash`, `par-nom`, `dispatch`, `overrides`, `spec`, `liste` : **0**, gardées ; résidu D **1284 lignes / 113 fichiers** (1435 / 116) ; `make value-members` vide, `offsetof` absent de `src/`. Sans garde : `valeur` (le type `pico8_value_t`, ses constructeurs et `pico8_value_equal`, `pico8_value.c` et `test_pico8_value`, les clés de rangée `pico8_lq_keys_t`, les retours à genres mêlés dont `pico8_lq_bg`, `pico8_pj_ed`, `pico8_jc_ce`, `pico8_jy_qe` et l'argument de `pico8_oj`, inchangés ; 29 déclarations de `src/*.h` nomment `pico8_value_t`, dont 6 dans `pico8_value.h` — compte par grep des prototypes, l'étude en disait 62 à `2dec71f` par une autre méthode) ; `type-tag` (`pico8_type_tag.h` à une étiquette, `STRUCT`, l'en-tête `pico8_struct_header_t` et les `assert(pico8_type_tag_of(x) == PICO8_TYPE_TAG_STRUCT …)` de `src/` et `tests/` ; 52 fichiers `.c` nomment `PICO8_TYPE_TAG_STRUCT`) ; `ref-audit` (l'en-tête d'audit sans aucune macro ni appelant : état, `dump`, `note`, `expect` sous `#ifdef PICO8_REF_AUDIT`, 11 lignes ; ses 12 includes ; `make ref-audit` et `tools/ref_audit_report.sh`, qui n'enregistrent plus aucun site). Sortie de D : `pico8_ob.c`, `pico8_stash.c` supprimés (D.8), `pico8_list.c` supprimé ici (exigé par §6/Q9, hors Sortie) ; **`pico8_value.c`, `pico8_type_tag.h`, `pico8_ref_audit.h` toujours là** ; « plus aucun `malloc` pour des données » : les allocations de `ob`/`stash` sont parties, celle de `pico8_value.c` (`tostr`) reste. `pico8_ref_audit.h` est désormais un en-tête sans appelant : sa suppression (l'en-tête, ses 12 includes, `make ref-audit`, le script) ne dépend d'aucune réponse à Q12. `type-tag` se réduit à « garder l'étiquette d'entité ou non », liée aux REF opaques (`valeur`). **Arrêt** : la liste de D n'a plus d'étape ; la Sortie de D exige le travail sur `valeur`, `type-tag`, `ref-audit` ; `plan-pilot` s'arrête et pose Q12 à l'humain : « **Q12. Ouvrir une étude D.9b pour les valeurs à genres mêlés (§4, point 2 ; 62 prototypes publics) avant les étapes `valeur`/`type-tag`/`ref-audit` ?** Oui. Les trois points de D.9 ne suffisent pas à éteindre `valeur`. Sans règle écrite pour `hf`/`gg` (TROU-3), `et`, `qh_qh`, `js.my`/`h_` et les retours de `ldown`, la première étape `valeur` rencontrera l'arrêt « champ sans règle ». D.9b suivrait le même modèle. » (étude D.9) ; décisions de D.9 : « **Q12 reste ouverte** : l'étude D.9b des valeurs à genres mêlés (`hf`/`gg` TROU-3, `et`, `qh_qh`, `js.my`/`h_`, retours de `ldown`/`gv` ; 62 prototypes publics à `pico8_value_t`), avant toute étape `valeur`/`type-tag`/`ref-audit`, n'est pas tranchée ; `plan-pilot` la pose à l'humain à la fin de D.12. »
