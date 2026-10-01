---
title: "Plan 3, étape D.21c : cible `value-members` retirée — `tools/entity_value_members.sh` supprimé, l'appel `--check` sorti de `data-guard`, légendes du `Makefile` réécrites ; `data-guard` garde `offsetof` et `DATA_RESIDUE_GUARD`, et mord encore"
description: "Le type `pico8_value_t` n'existe plus depuis D.19c : la garde `value-members` imprime « membres pico8_value_t : 0 » à chaque `make test` et ne peut plus échouer, et la catégorie `valeur` du compteur attrape déjà le nom partout dans `src/` et `tests/`. D.21c retire la cible, le script, l'appel dans `data-guard` et les légendes, puis prouve par injection que `data-guard` rougit encore sur un `offsetof` et sur un `pico8_value_t`."
status: needs-triage
---

## Problem Statement

**Écrit sans humain** (`plan-pilot`, forme des PRD de `docs/prd/done/`), sur l'arbre `f5a8c48` (« refactor(D.21b2): … »), propre. Seuls `git`, `grep`, `sed`, `make test` (vert, 7,7 s) et `make -s data-guard` ont tourné sur l'arbre ; les injections plus bas ont été jouées sur une copie de `src/`, `tests/` et `tools/` dans le scratchpad. **Rien n'a été modifié dans le dépôt hors ce PRD.**

Le problème, vu du mainteneur : à chaque `make test`, `data-guard` imprime d'abord

```
membres pico8_value_t : 0
```

Cette ligne vient de `tools/entity_value_members.sh --check`, écrit à 1b.0 (commit `d64baa4`) pour mesurer la sortie de la phase 1b : il liste les membres `pico8_value_t` des typedefs d'entité de `src/`, c'est-à-dire des `typedef struct { … } pico8_X_t;` dont le premier membre est un `pico8_struct_header_t`, ou un typedef d'entité embarqué. Depuis D.19c, `pico8_value_t` n'existe plus : `pico8_value.h` et `pico8_value.c` sont supprimés, et la catégorie `valeur` est dans `DATA_RESIDUE_GUARD`. La garde ne peut donc plus échouer sans que `valeur` échoue aussi. Elle laisse croire que la couche existe encore. C'est l'avertissement **M4** de la revue de sortie de D (warn-review `0a74e49..39c4e23`) :

> **M4. `value-members` garde un type qui n'existe plus.** […] La garde imprime « membres pico8_value_t : 0 » à chaque `make test`. Elle ne peut plus échouer, et elle est incluse dans `valeur`, dont le motif `\bpico8_value_[A-Za-z0-9_]+` attrape `pico8_value_t` partout. […] **Remède (maintenant).** Retirer la cible, le script et l'appel `--check` de `data-guard`, puis réécrire la légende de `data-guard`.

La ligne du plan (`docs/plan-3.md:301`) :

> D.21c — cible `value-members` (≈ 30 min) : la cible et son `.PHONY`, `tools/entity_value_members.sh`, l'appel `--check` de `data-guard` et les deux légendes du `Makefile` (`value-members` retirée, `data-guard` réécrite : « aucun offsetof dans src/, catégories de DATA_RESIDUE_GUARD à 0 »). Compte : `grep -rn 'value-members\|entity_value_members' Makefile tools/ docs/plan-3.md` = les seules mentions historiques du plan ; `make test` n'imprime plus « membres pico8_value_t ». Preuves : `data-guard` mord encore après retrait de sa première ligne — un `offsetof` ajouté dans une copie d'un `.c` de `src/` → rouge, une ligne `pico8_value_t` ajoutée → rouge (catégorie `valeur`) ; `gcov` sans objet.

La règle commune de D.21 s'applique (`docs/plan-3.md:297`) : **aucune empreinte ne bouge**, et une mutation verte est rejouée au parent (règle du parent de D.20a).

### Pourquoi le retrait ne perd rien

| Ce que contrôlait le script | Qui le couvre après |
|---|---|
| un membre `pico8_value_t` (non pointeur, tableau compris) dans un typedef d'entité de `src/*.c`, `src/*.h` | la catégorie `valeur` (`tools/data_residue.sh:29`, motif `\bpico8_value_[A-Za-z0-9_]+`), gardée par `data-guard` : **toute** ligne qui nomme `pico8_value_t`, entité ou non, pointeur ou non, dans `src/` **et** `tests/`, sous-dossiers compris (`find`) |
| rien d'autre | — |

Le script ne contrôlait **pas** `offsetof`. Ce contrôle est un `grep -rn offsetof src/` écrit en dur dans la recette de `data-guard` (`Makefile:850-853`). Il reste tel quel. Aucune partie du contrôle retiré n'est donc orpheline : `valeur` est un sur-ensemble strict. Constaté sur copie : un typedef `{ pico8_value_t v; }` qui n'est pas une entité, ajouté à un `.h` de `src/`, laisse le script à `0` mais rougit `valeur` (voir « Preuves » plus bas). Le script ne reconnaissait d'ailleurs que les typedefs sans étiquette (`^typedef struct[ \t]*\{`) ; les 13 typedefs de `src/*.h` dont le premier membre est l’en-tête d’entité le sont, mais une entité écrite `typedef struct pico8_x {` lui aurait échappé, pas à `valeur`.

### Inventaire (numéros de `f5a8c48`)

`git grep -n 'value-members\|entity_value_members' -- . ':!docs/prd/done' ':!docs/plan-3.md'` rend **10 lignes**, toutes dans `Makefile` et dans le script. Hors de ces deux fichiers, seuls deux historiques citent la cible ou le script : `docs/plan-3.md` (**10 lignes** : 1b.0, 1b.1, 1b.23d, 1b.37, la Sortie de 1b, D.0, D.1, D.12d, D.21c) et `docs/prd/done/` (**68 PRD**). Ils restent. Aucun test, aucun e2e, aucune skill de `.claude/`, aucun autre document ni script de `tools/` ne cite la cible, le script ou la chaîne « membres pico8_value_t ».

#### `Makefile`

| Ligne(s) | Aujourd'hui | Après |
|---|---|---|
| `:59-62` | légende `# make value-members -> liste les membres pico8_value_t des structs d'entité` … `#                       en fait la garde` (4 lignes) | **supprimées** |
| `:66-67` | `# make data-guard    -> make value-members vide, aucun offsetof dans src/,` / `#                       catégories de DATA_RESIDUE_GUARD à 0, dans make test` | `# make data-guard    -> aucun offsetof dans src/, catégories de` / `#                       DATA_RESIDUE_GUARD à 0, dans make test` (sens imposé, coupure libre) |
| `:836-840` | `value-members: tools/entity_value_members.sh`, `	@sh tools/entity_value_members.sh`, ligne vide, `.PHONY: value-members`, ligne vide | **supprimées** (5 lignes) ; une seule ligne vide reste entre `.PHONY: minimize e2e-cleanup e2e-judge` et `# Catégories du compteur tombées à 0, …` |
| `:848` | `data-guard: tools/entity_value_members.sh tools/data_residue.sh` | `data-guard: tools/data_residue.sh` |
| `:849` | `	@sh tools/entity_value_members.sh --check` | **supprimée** |

Au total, 10 lignes retirées et 3 modifiées. Le commentaire `:847` (« grep rend 1 quand rien ne correspond ; au-delà, c'est une erreur. ») porte sur le `grep offsetof`, qui devient la première ligne de la recette : il reste. `DATA_RESIDUE_GUARD` (`:842`), la cible `data-residue`, `.PHONY: data-residue data-guard` et la ligne `test:` (`:668`) ne changent pas.

#### `tools/`

| Chemin | Sort |
|---|---|
| `tools/entity_value_members.sh` (161 lignes) | **supprimé** |
| `tools/data_residue.sh` | inchangé |

#### `docs/`

| Chemin | Sort |
|---|---|
| `docs/plan-3.md` | **inchangé** par ce commit (historique ; la note « Découvert à D.21c » est le travail du pilote, dans son propre commit) |
| `docs/prd/done/*` (68 PRD) | **inchangés** (historique) |
| `CLAUDE.md`, `docs/glossaire-noms.md`, `docs/trous-de-portage.md`, autres `docs/*.md`, `.claude/skills/` | ne citent ni la cible ni le script : inchangés |

## Solution

Du point de vue du mainteneur : `make test` n'imprime plus que les gardes qui peuvent échouer. `data-guard` imprime deux lignes, `offsetof dans src/ : 0` puis `résidu gardé (…) : 0 ligne(s)`, et sa légende dit exactement ce qu'il contrôle.

**Une sous-étape, un commit** (D.21c), vert seul, vérifié et poussé :

1. `git rm tools/entity_value_members.sh` ;
2. `Makefile` : les éditions du tableau plus haut, et rien d'autre.

Message en français, préfixe `build(D.21c):` ou `refactor(D.21c):` au choix de l'implémenteur.

## User Stories

1. En tant que mainteneur, je veux que `make test` n'imprime aucune garde qui ne peut plus échouer, afin que chaque ligne de sa sortie soit une information.
2. En tant que mainteneur, je veux qu'aucune cible ni aucun script ne nomme `pico8_value_t` hors de l'historique, afin de ne pas croire que la couche valeur existe encore.
3. En tant que lecteur du `Makefile`, je veux que la légende de `data-guard` dise exactement ce qu'il contrôle (`offsetof` dans `src/`, catégories de `DATA_RESIDUE_GUARD` à 0), afin de ne pas chercher une troisième garde.
4. En tant que relecteur, je veux la preuve que `data-guard` rougit encore sur un `offsetof` injecté, afin d'être sûr que le retrait de sa première ligne n'a pas cassé la suivante.
5. En tant que relecteur, je veux la preuve qu'un `pico8_value_t` injecté, y compris comme membre d'un typedef d'entité, rougit `data-guard` par la catégorie `valeur`, afin d'accepter que le contrôle retiré est couvert.
6. En tant que relecteur, je veux savoir que le script ne contrôlait rien d'autre, afin de n'avoir aucune garde orpheline à replacer.
7. En tant que pilote du plan, je veux des greps de Sortie dont chaque « vide » exclut nommément l'historique gardé, afin qu'un vide soit vérifiable.
8. En tant que pilote du plan, je veux un diff limité à `Makefile` et au script supprimé, avec zéro empreinte touchée, afin de respecter la règle commune de D.21.

## Implementation Decisions

### Modules

Exactement deux chemins :

- `Makefile` — légende `value-members` retirée, légende `data-guard` réécrite, cible `value-members` et son `.PHONY` retirés, prérequis et première ligne de recette de `data-guard` retirés ;
- `tools/entity_value_members.sh` — supprimé.

Rien d'autre : ni `src/`, ni `tests/`, ni `tools/data_residue.sh`, ni `docs/`, ni `DATA_RESIDUE_GUARD`.

### Ce qui reste dans `data-guard`

La recette garde, dans l'ordre et mot pour mot, le bloc `grep -rn offsetof src/` (`:850-853`) et le bloc `DATA_RESIDUE_GUARD` (`:854-858`). Pas de nouvelle garde, pas d'extension du `grep offsetof` à `tests/` (voir « Out of Scope »).

### Commentaires (règle de `CLAUDE.md`)

Dans un `Makefile`, seule l'aide à la lecture d'une commande reste. La légende réécrite de `data-guard` en est une. Aucun commentaire ajouté ne cite D.21, D.19c, la revue ou `value-members` : cet historique va au message de commit.

### Ce qui ne change pas

Toute constante `*_HASH`, `FROZEN_*` ; tout test ; tout e2e et ses `*.sha256` ; `tools/data_residue.sh` et son TU ; la liste des cibles de `test:`.

## Testing Decisions

Aucune ligne de C ne change : `gcov` est sans objet, comme le dit la ligne du plan. Il faut prouver deux choses : **rien ne régresse** (filet), et **la garde mord encore** (injections).

### Filet

- `make test` vert ;
- `make e2e` : **26 conformes** ;
- `git diff HEAD^ HEAD -- src tests tools/data_residue.sh '*.sha256'` → vide.

### Preuves : `data-guard` mord encore (injections, une à la fois)

Sur l'arbre du commit, chaque injection est faite par `cp` de sauvegarde, puis ajout d'une ligne en fin de fichier, puis `make -s data-guard`, puis restauration par `cp`. Après les quatre injections, `git status --porcelain` doit être vide.

| # | Injection | Attendu à l'arrivée | Au parent (`f5a8c48`) |
|---|---|---|---|
| Mg1 | `static int d21c_probe = (int)offsetof(struct { int a; }, a);` à la fin de `src/main.c` | rouge, rc ≠ 0, ligne `data-guard : offsetof dans src/` | rouge (idem) |
| Mg2 | `typedef struct { pico8_value_t v; } d21c_probe_t;` à la fin d'un `.h` de `src/` (par exemple `src/pico8_assets.h`) | rouge, ligne `valeur: src/…:<n>:typedef struct { pico8_value_t v; } d21c_probe_t;` puis `résidu gardé (…) : 1 ligne(s)` | rouge par `valeur`, et **pas** par le script (`membres pico8_value_t : 0`, le typedef n'est pas une entité) |
| Mg3 | le cas exact que gardait le script : `    pico8_value_t d21c_probe;` inséré après `pico8_struct_header_t header;` dans le typedef d'entité de `src/pico8_economy.h:17-18` (`sed -i '18a\    pico8_value_t d21c_probe;'`) | rouge par `valeur`, ligne `valeur: src/pico8_economy.h:19:    pico8_value_t d21c_probe;` | rouge dès la première ligne de la recette (`src/pico8_economy.h:19: pico8_hy_t d21c_probe`, `membres pico8_value_t : 1`) |
| Mg4 | `pico8_value_t d21c_probe;` à la fin d'un `.c` de `tests/` | rouge par `valeur` | rouge par `valeur` (le script ne lisait pas `tests/`) |

Le plan exige Mg1 et Mg2. Mg3 ferme la question « le cas précis du script est-il couvert ? ». Mg4 montre que la couverture après retrait est plus large que celle du script. Les quatre ont été joués sur une copie de `src/`, `tests/` et `tools/` avant ce PRD : tous rouges (grep rc 0 pour `offsetof`, `data_residue.sh --check` rc 1 pour `valeur`, et le script à `1` pour Mg3 seulement). Une injection verte à l'arrivée donne `FAILED:`. Règle du parent : si elle est aussi verte au parent, c'est un chemin jamais gardé, déclaré au commit ; si elle est rouge au parent et verte à l'arrivée, c'est `FAILED:`.

`data-guard` n'a que des scripts pour prérequis et ne compile rien : une injection qui casserait la compilation ne gêne pas la mesure.

### Greps de Sortie

Chaque « vide » exclut nommément ce qu'on garde : `docs/prd/done/` (68 PRD, historique), `docs/plan-3.md` (10 lignes d'historique, dont la ligne D.21c) et ce PRD, quel que soit le dossier de `docs/prd/` où il se trouve.

1. `git grep -n 'value-members\|entity_value_members' -- . ':!docs/prd/done' ':!docs/plan-3.md' ':!docs/prd/*/plan3-phaseD-etape21c-*'` → **vide** (10 lignes au départ).
2. `git grep -n 'membres pico8_value_t' -- . ':!docs/prd/done' ':!docs/plan-3.md' ':!docs/prd/*/plan3-phaseD-etape21c-*'` → **vide** (3 lignes au départ : `Makefile:59` et deux dans le script).
3. `grep -rn 'value-members\|entity_value_members' Makefile tools/ docs/plan-3.md | grep -v '^docs/plan-3.md:'` → **vide** (compte du plan) ; `grep -c 'value-members\|entity_value_members' docs/plan-3.md` → **10**, inchangé.
4. `make test 2>&1 | grep -c 'membres pico8_value_t'` → **0** ; `make test 2>&1 | grep -E '^offsetof dans src/ : 0$|^résidu gardé \('` → exactement deux lignes.
5. `ls tools/entity_value_members.sh` → « No such file or directory ».
6. `make -n value-members` → échec « No rule to make target 'value-members' ».
7. `grep -n '^data-guard: tools/data_residue.sh$' Makefile` → une ligne ; `grep -n '^# make data-guard    -> aucun offsetof dans src/' Makefile` → une ligne.
8. `grep -n '^DATA_RESIDUE_GUARD := spec,overrides,stash,ob,dispatch,par-nom,liste,ref-audit,valeur,type-tag$' Makefile` → une ligne, inchangée.
9. `git diff HEAD^ HEAD --numstat -- Makefile` → `3	13	Makefile` (10 lignes retirées, 3 modifiées) ; un autre compte, dû à la coupure de la légende, est rapporté avec son explication.
10. `git show --stat HEAD` → `Makefile` et `tools/entity_value_members.sh` seuls.

### Budget et tableau « Runs »

**≈ 30 min** jusqu'au commit, preuves comprises ; plafond 40 min.

| Cible | Durée attendue |
|---|---|
| `make test` | ~8 s (mesuré à `f5a8c48` : 7,7 s) |
| `make e2e` | ~1 s |
| 4 injections × (arrivée + parent) | < 2 min |
| greps | secondes |

L'implémenteur tient un tableau « Runs » : une ligne par lancement, avec le numéro, la commande exacte, l'état de l'arbre, le code de sortie, la ligne clé et la durée. Il le met au message de commit.

### Critères d'échec (`FAILED:` pour le vérificateur de `prd-autopilot`)

- `FAILED:` si l'arbre de départ n'est pas `f5a8c48` ou un descendant propre qui ne touche ni `Makefile:59-67`, `:836-853`, ni le script.
- `FAILED:` si un chemin hors des deux de « Modules » change, en particulier `src/`, `tests/`, `tools/data_residue.sh`, `docs/`, un `*.sha256` ou un fichier de `tests/e2e/`.
- `FAILED:` si une constante `*_HASH` ou `FROZEN_*` change.
- `FAILED:` si `make test` n'est pas vert ou si `make e2e` ne rend pas 26 conformes (3 tentatives au plus).
- `FAILED:` si une des injections Mg1 à Mg4 est verte à l'arrivée et rouge au parent ; si l'arbre n'est pas propre après les injections.
- `FAILED:` si `DATA_RESIDUE_GUARD` ou le bloc `offsetof` de la recette change.
- `FAILED:` si une grep de Sortie ne rend pas ce qui y est écrit, ou si une exclusion y est ajoutée sans être rapportée.
- `FAILED:` s'il manque au message : l'inventaire retiré (lignes du `Makefile`, script), la justification (type absent depuis D.19c, garde qui ne peut plus échouer, `valeur` sur-ensemble), le tableau des injections avec leurs sorties, les greps de Sortie avant/après, le tableau « Runs ».

## Out of Scope

- Étendre le `grep offsetof` de `data-guard` à `tests/`. Le plan ne le demande pas, et le script retiré ne le faisait pas.
- Toucher `tools/data_residue.sh`, ses catégories ou son TU.
- Réécrire l'historique : `docs/plan-3.md` (la note « Découvert à D.21c » est au pilote) et `docs/prd/done/`.
- Les autres lignes de D.21 (a, b1, b2, d/e/f), le mot « REF » restant (phase 3).
- `make raylib` exécuté, l'appli raylib, l'audio, la PSX.

## Further Notes

- **Aucune garde orpheline.** Le script ne contrôlait qu'une chose : un membre `pico8_value_t` dans un typedef d'entité de `src/`. La catégorie `valeur` contrôle un sur-ensemble strict de ce cas : toute mention, tout type, dans `src/` et `tests/`. Le contrôle `offsetof` n'a jamais été dans le script. Rien n'est donc à replacer.
- **Question ouverte (pour l'humain, non bloquante).** Le contrôle `offsetof` de `data-guard` ne regarde que `src/`, pas les tests. Les tests ont-ils le droit d'utiliser `offsetof`, par exemple pour sonder la disposition d'une struct ? Si non, il faudrait une ligne de plus dans une étape future. D.21c ne tranche pas.
- La mention « `make value-members` vide » restera dans la Sortie de 1b (`docs/plan-3.md:226`) et dans 68 PRD archivés. C'est voulu : l'historique renvoie à git (`d64baa4` pour la création, ce commit pour le retrait).

## Annexe — reproduire (lecture seule)

```sh
sed -n 56,68p Makefile; sed -n 834,860p Makefile        # légendes, cible, data-guard
sed -n 1,20p tools/entity_value_members.sh              # contrat du script
sed -n 20,30p tools/data_residue.sh                     # catégorie valeur
git grep -n 'value-members\|entity_value_members' -- . ':!docs/prd/done' ':!docs/plan-3.md'
git grep -n 'membres pico8_value_t' -- . ':!docs/prd/done' ':!docs/plan-3.md'
grep -c 'value-members\|entity_value_members' docs/plan-3.md    # 10
make -s data-guard                                      # trois lignes, rc 0
```
