---
title: "Plan 5, sous-étape 5h — corrections de clôture : les quatorze verdicts `proposition` de « Clôture (5) » qui éditent, un commit chacun (5h-1 à 5h-14), gardes vertes à chaque commit, puis l'entrée 5h du plan et sa note"
description: "Le point d'arrêt final (5g) a donné « ok » aux 38 points de « Clôture (5) ». Quinze ont le verdict proposé `proposition` ; quatorze éditent le dépôt : des noms du cart que personne ne cherche (C-3) et trois motifs qui ne peuvent plus rien trouver (C-4) ; aucun contrôle sur `TROU-` ni sur un `GAP-N` sans entrée (C-8) ; des lignes d'exception mortes qui ne se signalent pas (C-9) et des exceptions de portée qu'aucun test n'exerce (C-10) ; douze `static const` qui gardent `g_` et trois locales à digramme (C-13, C-16) ; une étiquette `void-ptr-r2` d'une catégorie retirée (C-24) ; quatre entrées fausses ou périmées du registre des écarts (C-28 à C-31) ; une phrase fausse du skill et un hook disparu dans une doc (C-35, C-36). Les appliquer dans un ordre où chaque commit garde `make test-all` vert, outils test d'abord, renommages par la procédure N, puis inscrire 5h au plan avec sa note « Découvert »."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6, et de `docs/decisions-plan-5.md`, à `1c1bfa9`)

> 5g. **Point d'arrêt final.** Condition d'entrée : une liste unique, section « Clôture (5) » du registre, qui reprend une ligne par point avec son origine : chaque question ouverte des notes « Découvert » de 5a à 5f (décisions prises sans humain, « reste à trancher », entrées GAP signalées par 5d, angles morts des gardes), chacune des alertes de la `warn-review` de fin d'étape 5 (une alerte qui recoupe une entrée déjà tranchée renvoie à son verdict au lieu d'être rejugée), et la relecture humaine des skills de 5f. Chaque ligne reçoit un verdict de l'ensemble fermé du registre ; une ligne `proposition` qui édite devient une sous-étape 5h-k, avec PRD et sortie, ou un ticket hors plan. Les renvois « au prochain point d'arrêt » des notes de 5a à 5f désignent ce point.
>
>    **Sortie** : chaque ligne de « Clôture (5) » porte son verdict, écrit dans le dépôt par l'humain.
>
> 5i. **Sortie du chantier**, après les verdicts de 5g et les sous-étapes qu'ils ouvrent. […]

Registre, « Clôture (5) », « Mode d'emploi » :

> - Un point accepté en `proposition` qui édite devient une sous-étape (5h, 5j…) avec sa PRD, ou un ticket hors plan.

et « Bilan » :

> Verdicts proposés : `proposition` 15 (C-1, C-3, C-4, C-8, C-9, C-10, C-13, C-16, C-24, C-28, C-29, C-30, C-31, C-35, C-36), dont 14 qui éditent le dépôt (C-1 n'édite que ce registre) ; `garder tel quel` 22 ; `reporter hors plan 5` 1 (C-22).

Le plan n'a pas encore d'entrée 5h : 5g la nomme sans l'écrire. Cette PRD la rédige (§ « Further Notes ») et la fait inscrire, entre 5g et 5i, par son dernier commit, avec la note « Découvert ».

Phase 4 du pilotage (tronçon 4 : étape 5). 5h suit le point d'arrêt final (5g) et précède la sortie du chantier (5i). Elle ne rouvre aucun verdict : elle applique, point par point, ce que l'humain a accepté.

**Entrées lues**, toutes au commit `1c1bfa9` :
- `CLAUDE.md` ;
- `docs/plan-5.md` : §1 à §5 (dont le §4, règles de nommage, et le §5, invariant et gardes), l'entrée 5, les entrées 5a à 5i, et toutes les notes « Découvert » de 5a à 5f ; en particulier : 5a (*Lecture de « ok »*, *Frontière texte/AST*), 5b (*Vivacité*, *Ce qui reste écrit en noms du cart*, *La garde anti-cart ne voit pas les noms historiques*), 5c (*Décisions prises sans humain*, *Reste à trancher*, *Pièges laissés*), 5d (*Décisions prises sans humain*, *Registre des écarts, entrées signalées*, *Pour 5e*), 5e (*Plantes*, *Rouge joué par mutation*, *Écarts à la lettre*, *Ce que les gardes bloquantes ne voient toujours pas*), 5f (*Pour l'humain*) ;
- `docs/decisions-plan-5.md` : « Conventions » (procédure N, procédure G, verdicts), D-21, D-28, D-50, D-68, et toute la section « Clôture (5) » (C-1 à C-38, « Mode d'emploi », « Bilan ») ;
- les PRD de 5e (`git show f5c76ee:docs/prd/triage/phase4-etape5e-gardes-bloquantes.md`) et de 5f (`git show 1753eb3:docs/prd/triage/phase4-etape5f-docs-consignes.md`), pour la forme ;
- les commits de prior art : `e628de6` (D-21, tables `static const` sans `g_`), `9869441` (D-50, locales à digramme, ligne `identity`) ;
- le `Makefile` : légende, `test`, `test-all`, `data-guard`, `composition-guard`, `DATA_RESIDUE_GUARD`, `COMPOSITION_RESIDUE_GUARD`, `cart-names-count`, `old-names-guard`, `ast-guard`, `french-guard`, `text-guards-test`, `ast-guard-test`, `rename-apply`, `rename-table-check` ;
- `tools/rename_count.py` (`Names`, `is_ambiguous`, `distinctive`, `source_files`, `main`), `tools/french_count.py` (`main`), `tools/scope_count.py` (options), `tools/cart_citations.py` (options), `tools/rename_strings.py` (`SKIPPED_FILES`, `SKIPPED_PARTS`), `tools/rename_substeps.py` (`of_mention`, `violations`), `tools/data_residue.sh`, `tools/composition_residue.sh` (`pattern`, ligne 678), `tools/rename_overrides.tsv` (en-tête, ligne 656), `tools/rename_substeps.tsv` ;
- les quatre fichiers d'exceptions de `tools/` ;
- `tests/test_text_guards.py`, `tests/test_ast_guard.py`, `tests/test_data_residue.c`, `tests/test_composition_residue.c`, et les tests `--check` de `tests/test_rename_count.py`, `tests/test_french_count.py`, `tests/test_scope_count.py`, `tests/test_cart_citations.py` ;
- le code des sites : `src/game/economy_score.c` (`economy_score_trade_income`), `src/game/planet.c` (`planet_is_colonized`), `src/game/message_box.c:322`, `tests/test_button.c:1037-1060` et `:1670`, `src/game/menu.c:300-370`, `src/game/particle_zone.c:110-131`, `src/game/ui_zone.c:47`, `src/game/planet_family.c:418`, `src/game/score_panel.c:170`, `tests/test_structure_icon.c:393` et `:563-600` ;
- `docs/trous-de-portage.md` (en-tête, GAP-3, GAP-4, GAP-9, GAP-11, GAP-23), `docs/psx-hello-world.md`, `.claude/skills/reprendre-les-commentaires/SKILL.md` et `exemple-travaille.md`.

**Aucun humain n'était joignable** pendant la rédaction. `to-prd` demande de vérifier avec l'utilisateur les modules, les tests et le découpage : je l'ai fait seul. Chaque décision est écrite au § « Décisions prises sans humain ». Les verdicts sont tous donnés (5g close) : aucune hypothèse sur une étape non faite, aucune section « Hypothèses ».

**Mesures faites pour cette PRD** sur un clone jetable au commit `1c1bfa9`, dans le scratchpad de l'agent, supprimé ensuite. `third_party/clang-python` et `third_party/raylib` étaient des liens vers ceux du dépôt. Aucune commande git d'écriture n'a été lancée sur le dépôt. La machine a 4 cœurs.

- **Départ** :
  - `make test-all` vert en **150 s** (extraction comprise : 0 `warning:`, 4 `partial`) ;
  - sur cette extraction, sans `SUBSTEP` : `cart-names-count` 0/0/0/0, `french-count` 0/0/0/0, `cart-citations` 0/0/0, `cryptic-locals` 0, `scope-count` 0 ;
  - `class-coverage-net` et `e2e-gap` rouges, comme à chaque note depuis 5a (`tmp/line_coverage_gap.txt`, 2 932 lignes) ; `make raylib screencast-viewer` compile et linke ;
  - `rename-substeps-check` : 12 sous-étapes, 244 modules, 0 violation ;
  - tests : `text-guards-test` 12, `ast-guard-test` 5, `tests/test_rename_count.py` 27, `tests/test_french_count.py` 17, `tests/test_scope_count.py` 20, `tests/test_cart_citations.py` 9, `tests/test_data_residue.c` 11 fonctions, `tests/test_composition_residue.c` 28 ;
  - `--detail` : `tools/data_residue.sh`, la seule ligne de pied `residue D: 0 line(s), 0 file(s)` ; `tools/composition_residue.sh`, 13 lignes, dont 11 `void-ptr-r2`, la ligne `info … 438 lines` et le pied à 0.
- **C-3, C-4.** Les quinze noms historiques de C-3 n'ont aucune ligne dans `docs/renommage.tsv`, ni en `ancien`, ni en `nouveau`. Avec les quinze ajoutés aux noms distinctifs de `rename_count.Names` (émulation, `--part text`, même parcours que la garde), une seule mention dans tout le code : `nu`, en `string`, dans `tools/composition_residue.sh:64` (le motif `get_(nu|layer|…)` de `header-accessors`), celle que C-4 retire. Les fixtures ne les écrivent qu'en deux littéraux (`"pico8_" "mv(…"`, `"n" "u"`) : la garde ne les voit pas. Dans `tools/data_residue.sh`, `dispatch` (ligne 31) et les branches `\bpico8_entity_(get|set)_field\b` et `\bpico8_[a-z0-9_]+_(set_)?field(_fn)?\b` de `by-name` (lignes 29-30) n'apparient rien sur l'arbre : `--check dispatch` rend 0 ligne.
- **C-8.** `git grep TROU` rend 0 sur `src`, `tests`, `tools`, `Makefile`. Le code cite 19 numéros de `GAP-N` (1, 2, 3, 5, 8, 9, 10, 12, 13, 14, 15, 16, 17, 21, 22, 23, 24, 25, 26), en 60 mentions : 42 marqueurs nus `/* GAP-N */` et 18 lignes en prose (commentaires, message d'`assert` de `tests/test_str.c:91`). Chacun a son titre `## GAP-N` dans `docs/trous-de-portage.md`, qui en a 24 : 1 à 5, puis 8 à 26. GAP-4 et GAP-11 n'ont aucune mention dans le code.
- **C-9.** Les 28 lignes des quatre fichiers d'exceptions (`tools/french_exceptions.txt` 17, `tools/cart_names_exceptions.txt` 1, `tools/cart_citations_exceptions.txt` 8, `tools/scope_exceptions.txt` 2) retirent toutes quelque chose sur l'arbre réel : retirée seule, chacune change la sortie de son outil. Mesure par retrait en place (le fichier d'exceptions est sauté par son chemin ; un fichier d'exceptions copié ailleurs compterait son propre contenu). **Dans la copie réduite de `tests/test_text_guards.py`**, 14 lignes sont mortes : les 12 de `tests/test_cart_citations` et `tests/test_scope_count`, celle de `tools/pico8_reference_probe_out` (`french_exceptions.txt`), et celle de D-1 (`tools/frames_ref`). Copier en plus `tests/test_cart_citations.py`, `tests/test_scope_count.py`, `tools/pico8_reference_probe_out.txt` et `tools/frames_ref.sh` les rend toutes vivantes, et la copie reste verte : `old-names-guard` et `french-guard` code 0, `TOTAL` 0.
- **C-10.** Dans la copie AST, `typedef struct { int x; int y; } vec_t;` en tête de l'unité `src/game/planet.c` : `ast-guard` code 0, les deux tableaux à 0 (les déclarations sortent en `field` `x`, `y`, parent `vec_t`). Avec `tools/scope_exceptions.txt` réduit à ses deux lignes d'en-tête : code 2, portée `game/planet` `field` 2. Les unités propres de `tests/test_ast_guard.py` aujourd'hui ne déclarent aucun `vec_t` : sous C-9, les deux lignes `x`, `y` y seraient mortes.
- **C-13, C-16.** Les douze `static const g_…` sont exactement ceux du registre : `git grep` des `static const … g_` de `src/` et `tests/` en rend 18, dont six pointeurs mutables vers du `const` (`g_input`, `g_difficulty`, `g_planet_grab_arena`, `g_planet_grab_arenas_free`, `g_required_tech`, `g_cursor`), qui gardent `g_` (§4). `g_handles` existe deux fois : `src/game/particle_zone.c:110`, `static const` (C-13), et `src/game/ui_zone.c:47`, `static allocator_t` mutable, qui garde son nom. Contrôle des noms neufs (code, colonnes `ancien` et `nouveau` du TSV, `tmp/ast_rename/declarations.tsv`) :
  - libres : `message_names`, `outline_offsets` (sa seule mention est le `token er outline_offsets` de `tools/rename_overrides.tsv`), `galaxy_menu_refresh`, `slot_allocators`, `planet_name_grab_arena`, `synthesizer_synthesizable_resources_spent`, `slipgate_left_press_empty`, `without_birth`, `layer_ten`, `pos_nil`, `giver_layer`, `trail_color`, `resource_index` ;
  - **pris** : `galaxy_menu_on_click`, fonction exportée du même fichier (`src/game/menu.c:366`, `src/game/menu.h:23`, appelée par `tests/test_ui.c:304`) ; d'où `galaxy_menu_on_click_table` et, pour la paire, `galaxy_menu_refresh_table` (décision 9), tous deux libres ;
  - `cursor_before` existe déjà, locale de `test_directed` (`tests/test_structure_icon.c:393`) : c'est le nom que C-16 vise ; il est libre dans `test_restart`, la fonction de `gy` ;
  - `handles` est pris (deux lignes du TSV, deux déclarations) : c'est la raison de `slot_allocators`.

  **Essai** : la table des quinze identités (identités relues dans l'extraction fraîche), `make rename-apply TABLE=…` passe sans refus, 3 s : 42 occurrences dans 11 fichiers. Restent à la main deux mentions en commentaire de `g_message_names` (`src/engine/entity_registry.c:16` et `:137`), et la ligne 656 de `tools/rename_overrides.tsv`. Après la retouche des commentaires, `make test e2e` est vert, les 26 e2e « matches », `make raylib screencast-viewer` compile. Aucune ligne n'est ajoutée ni retirée.
- **C-24.** `r2-sites` est libre. `void-ptr-r2` vit dans `tools/composition_residue.sh` (ligne 22, en-tête ; ligne 678, littéral d'un `print` dans un programme `awk` entre apostrophes) et dans l'attendu de `tests/test_composition_residue.c:513`. Le test qui le fixe s'appelle déjà `test_detail_prints_r2_sites_after_header_accessors_and_the_info_line`.
- **C-28.** `economy_score_trade_income` (`src/game/economy_score.c:78`) ne compare aucune chaîne : elle compare l'indice de ressource de la production à celui du code `"w"` (`kw->resource == p8_fixed_from_int(w)`). `planet_is_colonized`, l'autre fonction citée par la même phrase, compare bien une chaîne (`strcmp(self->header.state, "settled")`). Dans le TSV, `pico8_db` → `economy_score_trade_income` : `db` et `db()` de la ligne « Lua : » de GAP-3 sont ses noms du cart.
- **C-29.** Le marqueur est `src/game/message_box.c:322` (`/* GAP-9 */`, au-dessus de `if (ctx->frame->has_back_color)`) ; le test, `test_open_box_layer_only_from_back_color` (`tests/test_button.c:1670-1683`). GAP-23 s'appuie sur GAP-9 (`docs/trous-de-portage.md:416`).
- **C-30.** GAP-4 et GAP-11 : aucune mention dans le code, ni dans `CLAUDE.md`, ni dans les autres docs hors plan, registre et `docs/prd/`. Seul `exemple-travaille.md:43` raconte « GAP-4 devenu atteignable » : c'est l'histoire du chantier de référence (décision 10 de 5f).
- **C-35, C-36.** `SKILL.md:23` est la puce « **la règle des quatre destinations est déjà dans le fichier d'instructions du dépôt** ». `docs/psx-hello-world.md:43-49` est la puce du hook ; `.claude/hooks/` n'existe pas ; la phrase d'annonce (lignes 32-33) dit « two hard constraints ».
- **`rename-table-check`** n'est pas un contrôle utilisable : 28 227 violations à `1c1bfa9`, le TSV décrivant une application faite. Je ne l'emploie pas.

## Condition d'entrée

- **5g close** : chaque ligne `Verdict` de « Clôture (5) » porte un verdict (38 « ok », `1c1bfa9`), lu comme le verdict proposé (« Mode d'emploi »).
- **L'arbre de départ tient l'invariant** : `make test-all` vert, `tests/e2e/*.sha256` inchangés, les cinq compteurs sans `SUBSTEP` à 0, les trois gardes à code 0. Une garde rouge à l'entrée arrête 5h avant le premier commit.
- **Mesure de départ égale à celle de cette PRD** (chunk 0). Un écart arrête 5h et se relit contre la section qui le mesure.
- **Lecture des verdicts.** Celle que 5a a consignée et que C-1 confirme : « ok » vaut le verdict proposé. Pour les quinze `proposition`, la proposition telle qu'écrite dans le registre ; là où elle laisse un choix (« après contrôle qu'il est libre », « noms à vérifier libres »), le choix est fait ici, mesuré, et écrit en décision.

## Problem Statement

Les gardes de 5e sont bloquantes, mais l'humain a vu, au point d'arrêt final, ce qu'elles laissent passer et ce que le chantier a laissé d'inexact ; il a accepté chaque correction proposée. Tant qu'elles ne sont pas faites :

- **Des noms du cart passent sans bruit.** Quinze noms historiques (`pico8_value_t`, `pico8_mv`, `hp_pose`…) n'ont pas de ligne au TSV : réintroduits par un agent qui recopie `pico8/lua.lua`, aucune garde ne les voit (C-3). Trois motifs des gardes à motifs ne cherchent que ces noms : verts par construction, ils donnent une fausse assurance (C-4).
- **Le registre des écarts n'est tenu par rien.** Un `TROU-` revenu d'une vieille branche, ou un `/* GAP-30 */` sans entrée, passe la suite (C-8).
- **Une exception peut survivre à son objet.** Si le code change, une ligne d'exception qui ne retire plus rien reste, et masquera le jour venu un vrai reste (C-9). Les deux exceptions de portée (`x`, `y` de `vec_t`) ne sont exercées par aucun test : 5e a vidé le fichier sans rien rougir (C-10).
- **Des noms contredisent le §4.** Douze tables `static const` portent `g_`, que le §4 réserve aux statiques mutables (C-13) ; trois locales gardent un digramme du cart (C-16) ; `--detail` imprime le nom d'une catégorie retirée (C-24).
- **Des docs disent faux.** GAP-3 dit qu'une fonction compare des chaînes ; GAP-9 dit « Sans site » et a un site ; GAP-4 et GAP-11 décrivent du code supprimé ; GAP-23 cite deux rejeux disparus (C-28 à C-31). Le skill `reprendre-les-commentaires` dit que ce dépôt a quatre destinations (C-35) ; `docs/psx-hello-world.md` décrit un hook qui n'existe plus (C-36).

Et 5i ne peut pas s'ouvrir : elle vient « après les verdicts de 5g et les sous-étapes qu'ils ouvrent ».

Deux contraintes pèsent sur l'ordre. Chaque commit doit garder `make test-all` vert, gardes bloquantes comprises : une garde neuve doit être verte sur l'arbre réel au commit qui la branche. Et les points se tiennent : la garde des noms historiques (C-3) rougirait sur le `nu` du motif que C-4 retire ; le contrôle des exceptions mortes (C-9) rougirait sur la copie AST de `ast-guard-test`, que C-10 seul fait déclarer un `vec_t`.

## Solution

**Une PRD, quatorze sous-étapes 5h-1 à 5h-14, un commit chacune**, plus une mesure de départ sans commit et un commit `docs:` final. Chaque commit garde `make test-all` vert et `tests/e2e/*.sha256` inchangés. L'ordre suit les dépendances :

| 5h-k | Point | Commit | Ce qui change |
|---|---|---|---|
| 5h-1 | C-4 | `refactor:` | `dispatch` et les deux branches `pico8_` de `by-name` sortent de `tools/data_residue.sh` (et `dispatch` de `DATA_RESIDUE_GUARD`), `nu` du motif `header-accessors` de `tools/composition_residue.sh` ; fixtures et lignes de vivacité suivent |
| 5h-2 | C-3 | `feat:` | `tools/historical_names.txt` (quinze noms), lu par `rename_count.py --historical` comme des noms distinctifs ; branché dans `old-names-guard` et `cart-names-count` ; un cas de plus dans `text-guards-test` |
| 5h-3 | C-24 | `refactor:` | l'étiquette `void-ptr-r2` devient `r2-sites` |
| 5h-4 | C-8 | `feat:` | `tools/gap_check.py` et `gap-check` dans `make test` : aucun `TROU-` dans le code, chaque `GAP-N` du code a son titre |
| 5h-5 | C-10 | `test:` | les unités propres de `ast-guard-test` déclarent un `vec_t` à `x`, `y` ; un cas rouge sans les exceptions de portée |
| 5h-6 | C-9 | `feat:` | sous `--check`, `french_count.py`, `rename_count.py` et `scope_count.py` échouent sur une ligne d'exception qui n'a rien retiré ; `cart_citations.py`, sans `--check`, l'imprime en `warning:` ; les copies des tests de vivacité suivent |
| 5h-7 | C-13 | `refactor:` | douze `static const` perdent `g_` (procédure N) |
| 5h-8 | C-16 | `refactor:` | `k_entry` → `trail_color`, `k_key` → `resource_index`, `gy` → `cursor_before` (procédure N) |
| 5h-9 | C-28 | `docs:` | GAP-3 ne cite plus `economy_score_trade_income` |
| 5h-10 | C-29 | `docs:` | GAP-9 nomme son site et son test |
| 5h-11 | C-30 | `docs:` | GAP-4 et GAP-11 supprimées |
| 5h-12 | C-31 | `docs:` | GAP-23 perd la parenthèse des deux rejeux |
| 5h-13 | C-35 | `docs:` | `SKILL.md:23` : « quatre, ou trois si le dépôt n'a plus de glossaire » |
| 5h-14 | C-36 | `docs:` | la puce du hook disparu sort de `docs/psx-hello-world.md` |
| — | — | `docs:` | l'entrée 5h au plan, entre 5g et 5i, et sa note « Découvert » |

**Ce qui n'édite rien** :
- **C-1** n'édite rien dans 5h (décision 3). Sa proposition demande à l'humain d'écrire « ok » sur les deux lignes `Verdict` restées vides de D-28 et D-68 ; c'est une transcription de verdict humain, comme celles de `8353be2` et `1c1bfa9`, que fait l'orchestrateur, pas un agent de sous-étape. La note dit si elles sont encore vides.
- **C-22** (`reporter hors plan 5`) : rien ; la question part dans un ticket hors plan, après 5i.
- **Les 22 `garder tel quel`** (C-2, C-5 à C-7, C-11, C-12, C-14, C-15, C-17 à C-21, C-23, C-25 à C-27, C-32 à C-34, C-37, C-38) : rien.

À la fin : quatorze points appliqués ; deux gardes de plus tiennent ce que la suite ne voyait pas (noms historiques, marqueurs `GAP`) ; les lignes d'exception mortes rougissent la garde qui les lit ; aucun `g_` sur une table constante ; le registre des écarts et les deux docs disent vrai ; le plan porte 5h et sa note.

## User Stories

1. En tant que mainteneur, je veux que `make test` échoue si `pico8_mv`, `hp_pose` ou un autre nom historique du cart réapparaît dans le code, pour que les noms sans ligne au TSV soient tenus comme les autres (C-3).
2. En tant que mainteneur, je veux que la liste des noms historiques vive dans un fichier lisible de `tools/`, une ligne par nom, pour l'allonger sans toucher à l'outil.
3. En tant que mainteneur, je veux que l'outil refuse un nom historique qui est aussi un nom neuf ou conservé du TSV, pour qu'une ligne ajoutée par erreur ne rougisse pas du code légitime.
4. En tant que mainteneur, je veux que le fichier des noms historiques ne soit pas compté comme du code par la garde qui le lit, pour qu'il puisse écrire les noms en clair.
5. En tant que relecteur des gardes à motifs, je veux que les motifs qui ne peuvent plus rien trouver soient retirés avec leurs fixtures et leurs lignes de vivacité, pour qu'aucune garde ne soit verte par construction (C-4, alerte 14 de D-68).
6. En tant que relecteur, je veux que `--check dispatch` rende « unknown category », comme les catégories retirées en 5b, pour qu'un `Makefile` qui la nommerait encore échoue franchement.
7. En tant que mainteneur, je veux savoir, après C-3 et C-4, ce qu'aucune garde ne voit plus (`get_nu`, `pico8_X_set_field` génériques), pour que le relais annoncé par le registre soit mesuré et pas supposé.
8. En tant que mainteneur du registre des écarts, je veux que `make test` échoue sur un `TROU-` dans le code, pour que le passage à `GAP` de 5d ne se défasse pas (C-8).
9. En tant que mainteneur du registre des écarts, je veux que `make test` échoue sur un `GAP-N` du code sans titre `## GAP-N` dans `docs/trous-de-portage.md`, pour qu'un marqueur ne renvoie jamais dans le vide (C-8).
10. En tant que mainteneur des exceptions, je veux qu'une garde échoue sur une ligne d'exception qui n'a rien retiré, en nommant le fichier, la ligne et son triplet, pour supprimer la ligne au commit qui l'a tuée (C-9).
11. En tant que mainteneur des exceptions, je veux qu'une ligne ne soit jugée que là où elle peut agir (sa moitié de la garde des anciens noms, l'arbre entier), pour qu'une exception de la moitié texte ne soit pas déclarée morte par la moitié AST.
12. En tant que lecteur du relevé `cart-citations`, je veux qu'une ligne morte de son fichier d'exceptions y soit signalée, sans le rendre bloquant, pour que le relevé reste un relevé (C-7) et que la ligne se voie.
13. En tant que relecteur, je veux que les exceptions de portée `x` et `y` soient exercées par un test qui rougit quand elles ne sont plus lues, pour que la mutation de 5e (`/dev/null`) ne passe plus (C-10).
14. En tant que relecteur, je veux que chaque test de vivacité prouve sa garde par la vraie cible du `Makefile`, sur une copie où chaque ligne d'exception est vivante, pour que C-9 ne rende pas les copies rouges par construction.
15. En tant que lecteur du code, je veux qu'un `g_` désigne toujours une statique mutable de fichier, pour que le préfixe dise quelque chose (C-13, §4).
16. En tant que lecteur du code, je veux que `galaxy_menu_on_click` reste le nom de la fonction exportée et que la table porte un autre nom, pour ne pas confondre la table et son accesseur.
17. En tant que lecteur du code, je veux que les locales `k_entry`, `k_key`, `gy` portent le nom de leur rôle, pour qu'aucun digramme du cart ne reste dans `src/` ni dans les tests (C-16).
18. En tant que mainteneur, je veux que chaque renommage passe par `make rename-apply` et mette à jour sa ligne du TSV et `tools/rename_overrides.tsv`, pour que le TSV reste ce qui a été appliqué et que la garde des anciens noms voie les noms abandonnés.
19. En tant que lecteur de `--detail`, je veux que les lignes R2 portent une étiquette qui ne nomme pas une catégorie retirée (C-24).
20. En tant que lecteur du registre des écarts, je veux que GAP-3, GAP-9 et GAP-23 disent ce que fait le code d'aujourd'hui, et que GAP-4 et GAP-11, refermées, disparaissent (C-28 à C-31).
21. En tant qu'agent qui suit le skill `reprendre-les-commentaires`, je veux que le skill ne m'affirme pas que ce dépôt a quatre destinations (C-35).
22. En tant que lecteur de `docs/psx-hello-world.md`, je veux qu'elle ne décrive pas un hook absent du dépôt (C-36).
23. En tant que relecteur, je veux que chaque outil changé ait été vu rouge par ses tests neufs avant l'implémentation, et que ce rouge soit consigné dans le message du commit.
24. En tant que mainteneur, je veux que chaque commit garde `make test-all` vert, les gardes bloquantes comprises, pour pouvoir bissecter la branche.
25. En tant qu'humain à la fin du chantier, je veux lire dans la note de 5h ce qui a été fait, ce qui a été tranché sans moi, et ce que les gardes ne voient toujours pas.
26. En tant qu'agent, je veux que l'appli raylib ne soit jamais lancée : `make raylib` et `make screencast-viewer` sont compilés et linkés, jamais exécutés.

## Implementation Decisions

### Inventaire par sous-étape (mesuré à `1c1bfa9`)

**5h-1 — C-4, gardes à motifs (procédure G).**
- `tools/data_residue.sh` : `dispatch` sort de `CATEGORIES` et de `pattern` ; `by-name` perd `\bpico8_entity_(get|set)_field\b` et `\bpico8_[a-z0-9_]+_(set_)?field(_fn)?\b` et garde `field_truthy(`, `->field`/`.set_field` et `_take_`. Le `Makefile` : `DATA_RESIDUE_GUARD := spec,overrides,by-name`.
- `tools/composition_residue.sh:64` : `header-accessors` perd l'alternative `nu`.
- `tests/test_data_residue.c` : la ligne `dispatch` du test d'ordre de table part (et la fixture `c7.c`), le pied devient 3 lignes, 3 fichiers ; `dispatch` entre dans `test_retired_categories_are_unknown` ; la ligne de vivacité `dispatch` part ; celle de `by-name` ne plante plus que `field_truthy` (1 ligne) ; le commentaire de la table de vivacité ne parle plus de jetons `pico8_`. Test neuf : les deux formes `pico8_` retirées (`entity_get_field`, `X_set_field`) ne sont plus du résidu (`by-name` 0).
- `tests/test_composition_residue.c` : test neuf, `x_get_nu(self)` n'est plus un accesseur (`header-accessors` 0). Les plantes s'écrivent en deux littéraux, comme les fixtures de 5b.
- Ce que C-4 laisse sans garde : `get_nu` et les `pico8_X_set_field` génériques ne sont vus ni par les motifs ni par C-3 (un jeton `planet_get_nu` n'est pas le mot `nu`). Accepté par le verdict ; dit dans la note.

**5h-2 — C-3, noms historiques.**
- `tools/historical_names.txt` : deux lignes d'en-tête `#` en anglais, puis les quinze noms, un par ligne, dans l'ordre du registre. L'en-tête ne contient ni `cart`, ni `Lua`, ni un mot français (le relevé `cart-citations` et `french-guard` lisent les commentaires de `tools/*.txt`).
- `tools/rename_count.py --historical <fichier>` : chaque nom du fichier entre dans les noms distinctifs (mot entier, dans les quatre colonnes, moitié texte), `nu` compris ; le fichier n'est pas compté comme code (même règle que `--exceptions`, par son chemin relatif à `--root`) ; un nom qui n'est pas un identifiant, ou qui est un nom neuf ou conservé du TSV, fait rendre 2 avec un message qui le nomme. Sans l'option, la sortie est égale à l'octet.
- `Makefile` : `old-names-guard` et `cart-names-count` passent `--historical tools/historical_names.txt` ; pas `ast-guard` (la moitié AST ne compte que les noms ambigus) ; légende des deux cibles.
- `tests/test_rename_count.py` : classe neuve (nom historique compté dans chaque colonne ; fichier non compté ; nom neuf du TSV refusé ; sans option, sortie inchangée).
- `tests/test_text_guards.py` : la copie réduite copie `tools/historical_names.txt` ; un cas : le premier nom historique de trois caractères ou plus du fichier, tiré à l'exécution (`fp_is_bool` à `1c1bfa9`), planté en identifiant dans `src/game/planet.c` → `game/planet` `identifier` 1.
- `tools/rename_substeps.tsv`, ligne 3a : `tools/historical_names`.

**5h-3 — C-24.** `tools/composition_residue.sh` : en-tête (ligne 22) et littéral du `print` de la ligne 678, rien d'autre dans le programme `awk` (piège 3 de D-65 : contrôle `sh -n` et sortie). `tests/test_composition_residue.c:513` : l'attendu. Contrôle : `--detail` après égal à `--detail` avant où `void-ptr-r2` est remplacé par `r2-sites` (11 lignes).

**5h-4 — C-8, `gap-check`.**
- `tools/gap_check.py --root <racine> --gaps <registre>` : parcourt tous les fichiers de `src/`, `tests/`, `tools/` et le `Makefile` (sans `__pycache__`), sans `git` (la copie de test n'a pas de dépôt) ; signale chaque `TROU-` et chaque `GAP-N` (en mot, marqueur ou prose) dont le numéro n'a pas de titre `## GAP-N` dans le registre, une ligne `fichier:ligne: …` chacun, puis un pied ; rend 1 s'il y en a, 2 si le registre manque ou n'a aucun titre `## GAP-` (une vérification sur rien passerait pour propre).
- `Makefile` : `gap-check` (`--root . --gaps docs/trous-de-portage.md`) et `gap-check-test` dans les prérequis de `test`, `.PHONY`, légende « in make test ».
- `tests/test_gap_check.py` : arbres jetables sous `tmp/` (prior art `tests/test_french_count.py`) et un cas par la vraie cible dans une copie réduite (`Makefile`, l'outil, un registre minimal), avec le `make` enfant sans `MAKEFLAGS`, `MFLAGS`, `MAKELEVEL` (décision 10 de 5e). Plantes tirées à l'exécution : ni l'outil ni son test n'écrivent `TROU-` ni un `GAP-` suivi de chiffres en clair, sinon `gap-check` rougit sur eux.
- `tools/rename_substeps.tsv`, 3a : `tools/gap_check`, `tests/test_gap_check`.

**5h-5 — C-10.** `tests/test_ast_guard.py` : l'unité propre `src/game/planet.c` déclare `typedef struct { int x; int y; } vec_t;` (les deux champs : sous C-9, une ligne sans déclaration serait morte) ; tous les cas partent de cette unité, celui de la globale de deux lettres compris. Cas neuf : la même copie avec `tools/scope_exceptions.txt` réduit à son en-tête → rouge, portée `game/planet` `field` 2, partie AST 0. Aucun outil ne change : le rouge joué est une mutation (recette d'`ast-guard` de la copie sans `--exceptions` → le cas vert rougit), consignée dans le message.

**5h-6 — C-9, lignes d'exception mortes.**
- **Règle** : une ligne d'exception est morte si, sur le comptage, elle n'a retiré aucune mention (ou, pour `scope_count.py`, aucune déclaration). Le jugement n'a lieu que sur un comptage de **tout l'arbre** : sans `--module` (`--substep` est déjà refusé avec `--check`).
- `french_count.py`, `rename_count.py`, `scope_count.py` : sous `--check`, chaque ligne morte est écrite sur stderr (`error: <fichier>:<ligne>: removed nothing: <triplet>`), le tableau reste imprimé, et l'outil rend 1. Le code 2 de `rename_count.py` et `scope_count.py` (extraction absente ou périmée) passe avant. Sans `--check`, la sortie ne change pas.
- `rename_count.py` juge par moitié : une ligne est de la moitié AST si sa colonne est `identifier` et son nom ambigu, de la moitié texte sinon ; sous `--part`, seules les lignes de cette moitié sont jugées. La ligne de D-1 (`string`) est donc jugée par `old-names-guard`, pas par `ast-guard`.
- `cart_citations.py` n'a pas de `--check` et reste un relevé (C-7, D-56, D-59) : sur un comptage de tout l'arbre, il imprime chaque ligne morte en `warning:` après son tableau, et rend toujours 0.
- Tests neufs, un par outil (ligne morte nommée, code 1 ; ligne vivante, code 0 ; sans `--check`, sortie inchangée ; pour `rename_count.py`, une ligne de l'autre moitié non jugée sous `--part`) ; `cart_citations.py` : le `warning:`, code 0.
- **Tests existants à ajuster** : `Check.test_check_fails_on_a_refused_declaration_and_passes_without_one` de `tests/test_scope_count.py` plante `x` sans `y` alors que ses exceptions ont les deux (le second cas rendrait 1) : il plante les deux. Les copies réduites de `tests/test_text_guards.py` copient en plus le fichier de chaque module nommé par une ligne d'exception (mesuré : `tests/test_cart_citations.py`, `tests/test_scope_count.py`, `tools/pico8_reference_probe_out.txt`, `tools/frames_ref.sh` ; le `Makefile` et `tools/*.py` y sont déjà) ; les deux cas « dans son module » **ajoutent** leur plante au fichier copié au lieu de le remplacer. Deux cas neufs par la vraie cible : le fichier du module de D-1 retiré de la copie → `old-names-guard` rouge en nommant sa ligne ; celui de `tools/pico8_reference_probe_out` retiré → `french-guard` rouge. `tests/test_ast_guard.py` : cas neuf, l'unité propre sans `vec_t` → `ast-guard` rouge en nommant les lignes `x` et `y`.
- Mesuré : les 28 lignes sont vivantes sur l'arbre réel ; aucune ligne n'est retirée par 5h.

**5h-7 — C-13, procédure N** (prior art `e628de6`, D-21) :

| Fichier | Ancien | Nouveau | Ligne du TSV (`ancien`) |
|---|---|---|---|
| `src/engine/entity_registry.c` | `g_message_names` | `message_names` | `g_bm_names`, renommé |
| `src/game/hud.c` | `g_outline_offsets` | `outline_offsets` | `g_er`, renommé |
| `src/game/menu.c` | `g_galaxy_menu_on_click` | `galaxy_menu_on_click_table` | `g_ix_my`, renommé |
| `src/game/menu.c` | `g_galaxy_menu_refresh` | `galaxy_menu_refresh_table` | `g_ix_hb`, renommé |
| `src/game/particle_zone.c` | `g_handles` | `slot_allocators` | `g_handles`, conservé → renommé |
| `src/game/planet.c` | `g_planet_name_grab_arena` | `planet_name_grab_arena` | `g_jo_nom_prise`, renommé |
| `src/game/planet_family_trade.c` | `g_synthesizer_synthesizable_resources_spent` | `synthesizer_synthesizable_resources_spent` | `g_mx_ht_spent`, renommé |
| `src/game/planet_family_trade.c` | `g_slipgate_left_press_empty` | `slipgate_left_press_empty` | `g_nr_ldown_empty`, renommé |
| `tests/test_particle_zone.c` | `g_without_birth` | `without_birth` | `g_sans_naissance`, renommé |
| `tests/test_particle_zone.c` | `g_layer_ten` | `layer_ten` | `g_fa_dix`, renommé |
| `tests/test_particle_zone.c` | `g_pos_nil` | `pos_nil` | `g_ra_nil`, renommé |
| `tests/test_ui.c` | `g_giver_layer` | `giver_layer` | `g_giver_fa`, renommé |

- `make rename-extract` frais ; identités relues dans `tmp/ast_rename/declarations.tsv` ; `make rename-apply TABLE=tmp/ast_rename/decision.tsv`.
- TSV : la colonne `nouveau` des onze lignes `renommé` ; la ligne `conservé` de `g_handles` de `src/game/particle_zone.c` passe `renommé`, nouveau `slot_allocators`. Celle de `src/game/ui_zone.c` ne bouge pas.
- `tools/rename_overrides.tsv` : une ligne `name <ancien du TSV> <nouveau>` par ligne renommée (la ligne 656 `name g_bm_names g_message_names` corrigée, pas doublée) ; pour `g_handles`, une ligne `identity c:particle_zone.c@g_handles slot_allocators`, parce qu'une ligne `name g_handles` renommerait aussi celle de `ui_zone.c`.
- À la main : `src/engine/entity_registry.c:16` et `:137`. Aucune ligne ajoutée ni retirée (`tools/line_coverage_reference.txt` et `tmp/line_coverage_gap.txt` comparent des numéros de ligne).
- Contrôle de l'entrée : le `grep` des sites des conventions rend 0 pour les onze anciens noms et pour `g_handles` dans `src/game/particle_zone.c` ; `g_handles` reste dans `src/game/ui_zone.c` (3 mentions).

**5h-8 — C-16, procédure N** (prior art `9869441`, D-50) : `k_entry` → `trail_color` (`burst_draw`, `src/game/planet_family.c:418`, 3 occurrences) ; `k_key` → `resource_index` (`score_panel_refresh`, `src/game/score_panel.c:170`, 3) ; `gy` → `cursor_before` (`test_restart`, `tests/test_structure_icon.c:568`, 4, dont 2 en argument de macro). Les trois lignes `conservé` du TSV passent `renommé`, avec l'identité relue dans l'extraction fraîche (les identités d'offset du TSV sont périmées : `@13607@`, `@4840@`, `@17357@` contre `@15022@`, `@5772@`, `@19679@` à `1c1bfa9`) ; une ligne `identity` par locale dans `tools/rename_overrides.tsv`. Effet sur les gardes : `k_entry` et `k_key` deviennent des anciens noms distinctifs, vus partout par `old-names-guard` ; `gy` reste ambigu.

**5h-9 à 5h-12 — registre des écarts** (`docs/trous-de-portage.md`) :
- **C-28, GAP-3** : la phrase « plusieurs modules ont leur propre comparaison par contenu (`planet_is_colonized`, `economy_score_trade_income`) » ne garde que `planet_is_colonized` (au singulier) ; la phrase « `economy_score_trade_income`, qui compare une chaîne, passe par le contenu. » part ; `db` et `db()` sortent de la ligne « Lua : » (décision 12).
- **C-29, GAP-9** : le paragraphe « Sans site : … » devient le site : le bouton d'une boîte ouverte (`message_box_open_box`) ne prend la couleur de fond du cadre que si le cadre en a une, au lieu d'écrire une couleur absente (en Lua, écrire nil effacerait le champ) ; fixé par `test_open_box_layer_only_from_back_color` de `tests/test_button.c` ; ligne « Fichier : `src/game/message_box.c` », comme les autres entrées. Le titre, le numéro et les deux premiers paragraphes restent.
- **C-30** : les sections GAP-4 et GAP-11 sont supprimées en entier ; les numéros 4 et 11 ne sont pas réutilisés (l'en-tête du registre le dit déjà : « Une entrée supprimée est une entrée refermée »).
- **C-31, GAP-23** : « dessinée au survol (events_recorded_1790198294, session_events). » devient « dessinée au survol. ».

**5h-13 — C-35** : la puce de `SKILL.md:23` devient « **la règle des destinations (quatre, ou trois si le dépôt n'a plus de glossaire) est déjà dans le fichier d'instructions du dépôt** (`CLAUDE.md` ou équivalent). » ; la suite de la puce est inchangée. Rien d'autre du skill.

**5h-14 — C-36** : la puce des lignes 43-49 de `docs/psx-hello-world.md` part ; « The session that wrote this had two hard constraints worth recording » devient « one hard constraint » ; le fichier reste en anglais (verdict).

### Effet des gardes sur 5h

- **Ce que chaque commit doit garder vert** : `old-names-guard`, `french-guard`, `text-guards-test` et, dès 5h-4, `gap-check` et `gap-check-test` dans `make test` ; `ast-guard` et `ast-guard-test` dans `make test-all`.
- **Les outils neufs et leurs tests sont lus par les gardes** (`source_files` parcourt `tools/` et `tests/`) : anglais seul, aucun ancien nom du TSV ni nom historique en clair, aucun `TROU-` ni `GAP-<chiffres>` en clair, aucun `cart`, `Lua`, `lua.lua` ni ancien nom de deux lettres nu dans un commentaire neuf (le relevé `cart-citations` doit rester à 0/0/0 ; cf. le cas `id` de D-47).
- **Une garde neuve est verte sur l'arbre réel à son commit** : `old-names-guard` avec `--historical` après 5h-1 (mesuré : seul le `nu` que 5h-1 retire) ; `gap-check` (mesuré : 0 `TROU`, 19 numéros, tous titrés) ; le contrôle des lignes mortes (mesuré : 28 lignes vivantes ; les copies de test rendues vivantes par 5h-5 et 5h-6 elles-mêmes).
- **Les renommages de 5h-7 et 5h-8 changent ce que voit la garde des anciens noms** : `k_entry`, `k_key` y entrent ; les onze anciens `g_…` intermédiaires ne sont l'`ancien` d'aucune ligne et n'y entrent pas (comme les noms intermédiaires de D-21).

### Décisions prises sans humain

1. **Une PRD pour les quatorze sous-étapes 5h-k**, et non une PRD par 5h-k comme la lettre de 5g pourrait le lire : chaque 5h-k a son commit, son contrôle et sa sortie dans cette PRD, comme les 33 entrées de 5c dans la sienne.
2. **Ordre** : C-4 avant C-3 (le `nu` du motif rougirait C-3) ; C-10 avant C-9 (sans `vec_t`, les lignes `x`, `y` seraient mortes dans la copie AST) ; les outils avant les renommages, les docs à la fin, comme la table de la Solution. C-24 et C-8 n'ont pas de dépendance ; ils suivent C-3.
3. **C-1 n'édite rien en 5h.** Sa proposition est une lecture (« ok » vaut le verdict proposé), déjà suivie de 5a à 5f. Écrire « ok » sur les lignes `Verdict` de D-28 et D-68 est une transcription de verdict humain : c'est l'orchestrateur qui transcrit (`8353be2`, `1c1bfa9`), et aucun agent de sous-étape n'a édité une ligne `Verdict` depuis 5a. La note de 5h dit si les deux lignes sont encore vides.
4. **C-4 en un commit**, les deux scripts ensemble : les retraits sont petits (une catégorie, deux branches, une alternative), là où 5b en faisait un par script pour 79 et 27 lignes.
5. **C-3 : une option `--historical`, un fichier à part**, pas des lignes ajoutées au TSV : le TSV est le pont des noms appliqués, et ces noms n'ont été appliqués par personne. `nu` entre comme les autres, en mot entier : deux lettres, mais aucun mot `nu` dans le code après C-4 ; le risque d'un faux positif est noté pour l'humain. Le fichier est passé à `old-names-guard` et `cart-names-count`, pas à `ast-guard`. Il est refusé pour un nom neuf ou conservé du TSV.
6. **C-8 : toute mention `GAP-N`, pas seulement le marqueur nu**, doit avoir son titre : plus strict que la lettre, vert aujourd'hui (19 numéros, tous titrés), et une mention en prose renvoie au registre autant qu'un marqueur. Parcours de fichiers, pas `git grep` : le test passe par la vraie cible dans une copie sans dépôt. Noms : `tools/gap_check.py`, `gap-check`, `gap-check-test` (les contrôles s'appellent `*-check` : `constants-check`, `frames-check`, `rename-substeps-check`).
7. **C-9 : jugé sous `--check`, sur tout l'arbre, par moitié pour `rename_count.py`** ; `cart_citations.py` imprime un `warning:` sans échouer, parce qu'il n'a pas de `--check` et que C-7 vient de confirmer qu'il reste un relevé — écart à la lettre (« sous `--check`, les lecteurs … échouent »), dit dans la note.
8. **C-9 : les copies de test copient le fichier de chaque module excepté**, plutôt que d'excepter les copies du contrôle : la copie reste la vraie recette sur des fichiers réels, et une ligne morte dans la copie est une vraie ligne morte.
9. **C-13 : `galaxy_menu_on_click_table` et `galaxy_menu_refresh_table`.** La proposition dit « les autres prennent leur nom sans `g_`, après contrôle qu'il est libre » ; `galaxy_menu_on_click` ne l'est pas (fonction exportée du même fichier). Le suffixe `_table` est celui des tables voisines (`galaxy_menu_table`, `pause_menu_table`) ; `galaxy_menu_refresh`, libre, le prend aussi pour que la paire se lise ensemble. `g_handles` de `src/game/ui_zone.c` (mutable) et les six pointeurs mutables vers du `const` gardent `g_` (§4).
10. **C-16 : comme D-50** — ligne du TSV `conservé` → `renommé`, identité relue, ligne `identity` dans `tools/rename_overrides.tsv`.
11. **C-24 : seul le littéral change** dans le programme `awk` ; aucun commentaire n'y entre (piège 3 de D-65).
12. **C-28 : `db` et `db()` quittent la ligne « Lua : »** de GAP-3 : elle rassemble les noms du cart que l'entrée cite (décision 6 de 5d), et l'entrée ne cite plus `economy_score_trade_income`. Le reste de la ligne ne change pas.
13. **C-29 : « Fichier : » plutôt qu'un numéro de ligne**, comme les autres entrées ; le test est nommé par sa fonction.
14. **C-36 : « two hard constraints » → « one hard constraint »**, sinon la phrase d'annonce ment ; la langue reste l'anglais (verdict).
15. **Modules neufs en 3a** de `tools/rename_substeps.tsv` (`tools/historical_names`, `tools/gap_check`, `tests/test_gap_check`), comme 5d et 5e : `rename-substeps-check` 244 → 247 modules, 0 violation.
16. **Un commit par point, l'invariant à chaque commit**, docs comprises, malgré la règle du skill `git-commit` (« si seuls des `.md` changent, pas de test ») : le plan est plus précis (décision 15 de 5f). Environ 2 min 30 par commit, plus deux extractions pour 5h-7 et 5h-8.
17. **Registre, `docs/vocabulaire.md`, `CLAUDE.md` et le plan hors de l'entrée 5h ne sont pas édités** ; aucune marque « appliqué » ; le TSV ne change que par la procédure N de 5h-7 et 5h-8.
18. **L'entrée 5h n'entre au plan qu'avec sa note**, au dernier commit, comme demandé : son texte est écrit ici (§ « Further Notes ») et ne change que d'un écart mesuré.

## Implementation Chunks

0. **Mesure de départ**, sans commit, dans `tmp/5h_baseline/` :
   - `make test-all` vert ; sur l'extraction qu'il laisse, les cinq compteurs sans `SUBSTEP` à 0, 0 `warning:` ;
   - `--detail` des deux scripts de résidu (13 et 1 lignes) ; `class-coverage-net` et `e2e-gap` (sorties gardées) ; `make raylib screencast-viewer` ;
   - l'émulation des noms historiques (seul `nu` de `tools/composition_residue.sh`), les 28 lignes d'exception vivantes, les 19 numéros `GAP` titrés, les noms neufs de 5h-7 et 5h-8 libres (et `galaxy_menu_on_click` pris).

   Un écart arrête 5h.
1. **5h-1, `refactor:`** — C-4. Tests d'abord : `dispatch` inconnue, formes `pico8_` et `get_nu` hors résidu, rouges ; puis les scripts et `DATA_RESIDUE_GUARD`. Contrôles : `--detail` égal à l'octet au chunk 0 ; chaque branche restante de `by-name` et de `header-accessors`, neutralisée seule dans une copie, rougit un test (mutation à la manière de `tmp/5b_mutation/`) — une branche qui ne rougit rien est notée, pas corrigée.
2. **5h-2, `feat:`** — C-3. Tests d'abord (rouges : option inconnue) ; puis l'outil, le fichier, le `Makefile`, `text-guards-test`. Rouge joué par mutation de la copie : `--historical` retiré de la recette → le cas neuf rougit. Contrôle : `old-names-guard` code 0 sur l'arbre réel ; une copie jetable avec un nom historique planté rend `make test` rouge par `old-names-guard` seule.
3. **5h-3, `refactor:`** — C-24. L'attendu du test d'abord (rouge), puis le script. Contrôles : `sh -n` ; `--detail` égal au chunk 0 à l'étiquette près.
4. **5h-4, `feat:`** — C-8. Tests d'abord (rouges : outil absent) ; puis l'outil et les cibles. Contrôle : `gap-check` code 0 sur l'arbre réel ; une copie jetable avec un `TROU-` puis un `GAP-99` plantés rend `make test` rouge par `gap-check` seule.
5. **5h-5, `test:`** — C-10. Rouge joué par mutation (ci-dessus). `ast-guard-test` 6 tests.
6. **5h-6, `feat:`** — C-9. Tests neufs d'abord, rouges ; l'ajustement de `tests/test_scope_count.py` et des copies dans le même commit. Contrôles : les trois gardes code 0 sur l'arbre réel ; `make cart-citations` sans `warning:` ; une ligne retirée de son module dans une copie jetable rougit sa garde en la nommant.
7. **5h-7, `refactor:`** — C-13, procédure N. Contrôles de l'entrée (ci-dessus) ; `make raylib screencast-viewer` compilés ; `tmp/line_coverage_gap.txt` égal à l'octet.
8. **5h-8, `refactor:`** — C-16, procédure N. Mêmes contrôles ; `old-names-guard` code 0 avec `k_entry` et `k_key` désormais cherchés.
9. **5h-9 à 5h-12, `docs:`** — C-28, C-29, C-30, C-31, un commit chacun. Contrôles : `gap-check` vert ; `git grep -nE 'GAP-(4|11)\b' docs/trous-de-portage.md` rend 0 après 5h-11 ; `git grep -n 'events_recorded_1790198294\|session_events' docs/trous-de-portage.md` rend 0 après 5h-12.
10. **5h-13, `docs:`** — C-35.
11. **5h-14, `docs:`** — C-36. Contrôle : `git grep -n deny-absolute-paths` rend 0 hors du plan, du registre et de `docs/prd/`.
12. **`docs:`** — l'entrée 5h au plan, entre 5g et 5i (texte au § « Further Notes »), et sa note « Découvert » dessous. La note contient :
    - les quinze commits, dans l'ordre, avec le rouge joué de chaque outil changé ;
    - C-1 sans édition (et l'état des lignes `Verdict` de D-28 et D-68), C-22 et les `garder tel quel` sans édition ;
    - les décisions 1 à 18 ;
    - ce que les gardes ne voient toujours pas (§ « Further Notes ») ;
    - tout écart à une mesure de cette PRD ;
    - les relevés de fin.

**Total : 15 commits** (14 sous-étapes et la note).

## Testing Decisions

- **Un bon test juge le comportement de l'outil par sa sortie et son code**, sur un arbre jetable sous `tmp/`, en boîte noire (prior art : `tests/test_rename_count.py`, `tests/test_french_count.py`, `tests/test_scope_count.py`). Une garde se prouve en plus **par sa vraie cible du `Makefile`**, dans une copie réduite (prior art : `tests/test_text_guards.py`, `tests/test_ast_guard.py`) : une recette qui perdrait son option se voit.
- **Test d'abord, rouge joué** pour chaque outil changé (5h-1, 5h-2, 5h-3, 5h-4, 5h-6) : le nombre de FAIL/ERROR avant l'implémentation est consigné dans le message, comme en 5d. Pour C-10, qui ne change aucun outil, le rouge est joué par mutation de la recette dans la copie (prior art `tmp/5e_mutation/run.py`, non commité).
- **Plantes jamais écrites en clair** dans un fichier que lit une garde : tirées du TSV, de `tools/historical_names.txt` ou des fichiers d'exceptions à l'exécution, ou construites (`"TRO" + "U-"`), comme en 5e (décision 3). Les fixtures C s'écrivent en deux littéraux, comme en 5b.
- **Modules testés** : `tools/rename_count.py` (C-3, C-9), `tools/french_count.py`, `tools/scope_count.py`, `tools/cart_citations.py` (C-9), `tools/gap_check.py` (C-8), `tools/data_residue.sh`, `tools/composition_residue.sh` (C-4, C-24) ; les cibles `old-names-guard`, `french-guard`, `ast-guard`, `gap-check` par leurs tests de vivacité.
- **Les renommages n'ont pas de test neuf** : le compilateur arbitre, les e2e et les tests de module tiennent le comportement (`tests/e2e/*.sha256` inchangés), la garde des anciens noms tient `k_entry` et `k_key`.
- **Les docs n'ont pas de test** : `gap-check` tient la cohérence code → registre ; le reste est relu au diff.

**Relevés de fin** (sortie de 5h et invariant du §5) :
- `make test-all` vert à chaque commit, `tests/e2e/*.sha256` inchangés ;
- `make raylib` et `make screencast-viewer` compilés et linkés aux commits qui changent `src/` (5h-7, 5h-8), jamais lancés ;
- `class-coverage-net` et `e2e-gap` : sorties égales au chunk 0 hors numéros de ligne du `Makefile` (`tmp/line_coverage_gap.txt` égal à l'octet) ;
- `--detail` de `tools/data_residue.sh` égal à l'octet ; celui de `tools/composition_residue.sh` égal à l'étiquette `r2-sites` près ;
- `rename-tool-test`, les `rename-*-test`, `scope-count-test`, `french-count-test`, `cart-citations-test`, `text-guards-test` (12 → 15), `ast-guard-test` (5 → 7), `gap-check-test` verts ; `rename-substeps-check` 247 modules, 0 violation ;
- sur l'extraction fraîche laissée par `make test-all` (0 `warning:`), sans `SUBSTEP` : `cart-names-count` 0/0/0/0 (avec les noms historiques), `french-count` 0/0/0/0, `cart-citations` 0/0/0 sans `warning:`, `cryptic-locals` 0, `scope-count` 0 ; `old-names-guard`, `ast-guard`, `french-guard`, `gap-check` code 0 ;
- le `grep` des sites des conventions rend 0 pour chaque ancien nom de 5h-7 et 5h-8 (sauf `g_handles` dans `src/game/ui_zone.c`) ;
- `git grep -n 'void-ptr-r2'` : 0 hors du plan, du registre et de `docs/prd/`.

## Out of Scope

- **Les 22 points `garder tel quel` et C-22** (`tools/` à plat : ticket hors plan, après 5i).
- **Les lignes `Verdict` du registre**, D-28 et D-68 comprises (décision 3), et toute marque « appliqué ».
- **Fermer les angles morts que les verdicts ont gardés** : ancienne identité réintroduite ailleurs (C-5), exception par module (C-6), relevés non bloquants (C-7), paramètres courts (C-14), `get_nu` et `pico8_X_set_field` génériques après C-4.
- **Une garde de plus sur le registre des écarts** : titre sans site, numéro réutilisé, cohérence des lignes « Lua : » ; C-8 ne demande que le sens code → registre.
- **Retoucher GAP-3 au-delà de `economy_score_trade_income`**, GAP-9 au-delà de sa fin, `exemple-travaille.md` (son « GAP-4 » est de l'histoire, C-33), ou traduire `docs/psx-hello-world.md`.
- **Le sort des outils `rename_*`, du plan, du registre, de `docs/vocabulaire.md`** : c'est 5i (C-38).
- Lancer `rename-derive`, `rename-strings`, `rename-tool-setup`, `e2e-baseline` ou l'appli raylib.

## Further Notes

- **Texte de l'entrée 5h**, à insérer au plan entre 5g et 5i par le dernier commit (la note « Découvert » vient dessous) :

  > 5h. **Corrections de clôture** (verdicts `proposition` de « Clôture (5) » qui éditent : C-3, C-4, C-8, C-9, C-10, C-13, C-16, C-24, C-28 à C-31, C-35, C-36). Une PRD, une sous-étape 5h-k par point, un commit chacune, dans l'ordre des dépendances : 5h-1 C-4, 5h-2 C-3, 5h-3 C-24, 5h-4 C-8, 5h-5 C-10, 5h-6 C-9, 5h-7 C-13, 5h-8 C-16, 5h-9 C-28, 5h-10 C-29, 5h-11 C-30, 5h-12 C-31, 5h-13 C-35, 5h-14 C-36. Outils test d'abord, rouge joué ; renommages par la procédure N. C-1 n'édite rien (la transcription des lignes `Verdict` de D-28 et D-68 revient à l'orchestrateur) ; C-22 part hors plan ; les `garder tel quel` n'éditent rien.
  >
  >    **Sortie** : à chaque commit, `make test-all` vert, gardes comprises, `tests/e2e/*.sha256` inchangés ; un nom historique, un `TROU-`, un `GAP-N` sans titre et une ligne d'exception morte, plantés dans une copie, rougissent leur garde par la vraie cible ; les douze `static const` n'ont plus `g_`, ni les trois locales de digramme ; `void-ptr-r2`, GAP-4, GAP-11, `events_recorded_1790198294` et `deny-absolute-paths` ne sont plus cités hors du plan, du registre et de `docs/prd/` ; sur une extraction fraîche, les cinq compteurs sans `SUBSTEP` à 0.

- **Ce que les gardes ne verront toujours pas après 5h** (pour la note et l'humain) : les formes génériques que C-4 retire sans relais (`get_nu`, `pico8_X_set_field`, tout `pico8_X` hors des quinze noms) ; un nom historique coupé en deux littéraux ; tout ce que C-5, C-6 et C-7 ont gardé. `nu` cherché en mot entier pourrait un jour rougir un mot légitime : la ligne se retire alors par décision humaine.
- **Coût** : environ 15 × 2 min 30 de `make test-all`, plus deux `rename-extract` (environ 2 min 15 chacun), soit environ 45 min de suite pour la PRD.
- **Pour 5i** : 5h ajoute deux outils permanents (`tools/gap_check.py`, l'option `--historical` de `tools/rename_count.py`) et un fichier de données (`tools/historical_names.txt`). Leurs en-têtes ne citent pas `docs/plan-5.md` : la sortie de 5i (`git grep -n 'plan-5' -- tools Makefile src tests` rend 0) n'en hérite rien. `tools/rename_count.py` porte la garde des anciens noms : 5i, en fixant le sort des outils `rename_*`, doit le savoir.
