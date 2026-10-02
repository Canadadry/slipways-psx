---
title: "Plan 5, sous-étape 3g — économie et recherche"
description: "L'économie, la recherche, la porte technologique et l'icône de structure (9 modules de game/ et 4 tests) passent aux noms anglais du TSV. Il faut d'abord corriger 5 entrées du TSV contraires au §4 et une clé qui n'en est pas une, puis traduire 3 exemptions de composition_residue.sh."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 3g. **Économie et recherche.** `game/` : `economy`, `economy_formulas`, `economy_score`, `economy_tiers`, `main_economy`, `research`, `research_display`, `tech_gate`, `structure_icon`. Tests : `test_economy`, `test_main_economy`, `test_research_project`, `test_structure_icon`. Résidu ≤ 106.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4), septième étape.

Entrées : `docs/plan-5.md` §4, §5, l'entrée 3 avec « Sous-étapes » et « Méthode des sous-étapes », et les notes « Découvert » de **3a à 3f**. Cette PRD y renvoie sans les recopier :
- l'ordre des commandes (3a) ;
- l'appartenance des chaînes et les littéraux composés (règle de 3f) ;
- la recherche des littéraux recollés ;
- la prévision des hachages en worktree jetable ;
- le recâblage après chaque édition de texte et la porte `rename-rekey-check` ;
- les corrections du TSV pour la seule conformité au §4.

Exemple à suivre : `8597fbd` (3f, une seule tranche).

Aucun humain n'était disponible. L'agent rédacteur a chargé lui-même la consigne `guideline` aux points « vérifier avec l'utilisateur » de `to-prd`. Les décisions ci-dessous sont prises sur le texte du plan et sur les mesures, sans interview.

**Condition d'entrée** :
- `origin/plan-5-noms-anglais` au HEAD de départ (`4ef9b52` ou un descendant), arbre propre, `make test-all` vert ;
- `make rename-extract` frais, sans ligne `warning:` (les 4 lignes `partial` de `class_coverage` sont attendues) ;
- `make rename-rekey-check SUBSTEP=3g` vert. Mesuré : 0 identité introuvable.

## Problem Statement

Après 3f, l'économie et la recherche gardent les noms du cart : `pico8_hy_t`, `pico8_hp_t`, `pico8_jy_t`, `pico8_qe_map_t`, `pico8_ky_t`, `pico8_ie_t`, les fonctions `pico8_bx`, `pico8_ck`, `pico8_da`, `pico8_db`, `pico8_ky`, `pico8_jo_ex`, et les champs `gb`, `gc`, `gy`, `id`… La sous-étape compte **447 identités**, soit **2 970 occurrences AST dans 67 fichiers**.

Une répétition à blanc a été faite dans un worktree jetable : application, gardes, chaînes, composés, puis `make test`, `make e2e`, extraction fraîche et compteurs. Elle a trouvé :
- **5 entrées du TSV contraires au §4.** Ce sont 5 des 12 fonctions exportées de 3g dont le nom neuf ne commence pas par leur module. Le relevé de 3c en avait compté 12, dont la collision `final_score`.
- **Une clé signalée qui n'est pas une clé.** `"pz"` est la valeur de `ri.inputs` : deux codes de ressource, `p` et `z`. Renommée `open_box`, elle fait rougir `test_wants` (`b->inputs->count == 2`, ligne 676).
- **L'application seule fait rougir `composition-guard` sur 3 lignes.** Ce sont `ptrs-entité` sur `main_economy.h:29` (`invented`) et `research.h:39` (`inputs`), et `registres` sur `research_display.h:26`. Les exemptions `pico8_hy_t.invented`, `pico8_hp_t.i` et `pico8_jo_ex` ne mordent plus.
- **Des constantes de hachage bougent : 5, dans 2 tests**, dont une dans un fichier de 3i (`test_burst`).
- **Des littéraux de 3g portent des noms du cart sans que le TSV les liste.** Ce sont des noms de vue hachés et des libellés imprimés.

## Solution

Une seule tranche, dans l'ordre de commandes de la note de 3a, sur `SUBSTEP=3g` :
1. corriger les 5 entrées et la clé `pz` dans le TSV, avant `rename-slice` ;
2. appliquer les 447 identités ;
3. traduire les 3 exemptions et la fixture de vivacité ;
4. recâbler ;
5. éditer les 24 sites de chaînes et les littéraux composés de 3g ;
6. recalculer les 5 constantes de hachage (rouge d'abord) ;
7. recâbler de nouveau, faire les relevés, écrire la note « Découvert ».

La répétition l'a établi : avec ces gestes, `make test` et `make e2e` sont verts (26 e2e conformes), et `make raylib` et `make screencast-viewer` compilent. `class-coverage-net` et `e2e-gap` sont rouges, avec des sorties égales à l'octet à celles de la fin de 3f.

À la fin :
- aucun nom de 3g ne reste en `identifier` ni en `string` ;
- les fichiers de 3g ne gardent que 106 mentions `identifier` et 1 mention `string` (`pico8_cart_load`, 3j) de sous-étapes suivantes ;
- `scope-count SUBSTEP=3g` passe de 116 à 2 ;
- pas un pixel ne change.

## User Stories

1. En tant qu'agent de 3g, je veux corriger dans le TSV les 5 fonctions exportées contraires au §4 avant `rename-slice`, afin que le TSV reste ce qui a été appliqué.
2. En tant que lecteur, je veux que les fonctions exportées d'`economy_score` portent toutes le préfixe `economy_score_`, comme leurs voisines `economy_score_difficulty`, `economy_score_happiness` et `economy_score_net_income`, afin qu'aucune ne prétende venir d'`economy` ou de `trade`.
3. En tant que lecteur, je veux que `final_score` (fonction) ne partage plus son nom avec le champ `final_score` de `score_panel_display`, afin de lever la collision signalée par 3c.
4. En tant qu'agent de 3g, je veux appliquer les 447 identités en une commande, afin que le compilateur arbitre tout oubli dans les 67 fichiers.
5. En tant que lecteur, je veux `economy_t`, `research_project_t`, `need_t`, `resource_map_t`, `final_score_t` et `structure_icon_t`, afin que l'économie et la recherche se lisent sans glossaire.
6. En tant que mainteneur, je veux `PTRS_NON_ENTITE` et `REGISTRES_NON_ENTITE` traduits, chacun avec sa preuve de vivacité, afin que `composition-guard` reste vert sans passer à vide.
7. En tant que mainteneur, je veux `composition_residue.sh --detail` et `data_residue.sh --detail` identiques avant et après, afin de prouver qu'aucune garde ne change de verdict.
8. En tant que relecteur humain, je veux le classement des 7 clés signalées de 3g (`dm`, `fj`, `il`, `ja`, `lr`, `px`, `pz`), afin de savoir lesquelles vivent.
9. En tant qu'agent de 3g, je veux que `"pz"` reste tel quel et que sa ligne `chaîne` passe à `conservé`, afin de ne pas casser une donnée qui n'est pas un nom.
10. En tant qu'agent de 3g, je veux renommer les 24 sites de chaînes, afin que `rename-strings-verify SUBSTEP=3g` soit vert. Les 2 sites `unseen` sont vérifiés à la main.
11. En tant qu'agent de 3g, je veux renommer les littéraux composés et les noms de vue hachés écrits et lus dans les fichiers de 3g, afin de ne pas laisser de noms du cart que ni le compteur ni le contrôle ne voient.
12. En tant qu'agent de 3g, je veux connaître d'avance les 5 constantes de hachage qui changent, afin qu'un rouge imprévu arrête la tranche.
13. En tant que mainteneur, je veux que la garde de portée tombe de 116 à 2 dans 3g, les deux restes étant des statiques locales déjà proposées par 3a.
14. En tant qu'agent de 3h, je veux trouver un TSV recâblé, avec `rename-rekey-check` vert de 3g à 3l.
15. En tant que joueur, je ne veux voir aucun pixel changer.

## Implementation Decisions

### Périmètre

`rename-slice SUBSTEP=3g` rend **447 identités** :
- par module : `research` 70, `test_research_project` 55, `structure_icon` 47, `research_display` 40, `economy_score` 39, `economy` 37, `test_structure_icon` 36, `test_economy` 35, `economy_tiers` 33, `main_economy` 25, `economy_formulas` 15, `tech_gate` 12, `test_main_economy` 3 ;
- par sorte : 170 fonctions, 110 locales, 72 champs, 30 paramètres, 26 statiques, 19 types, 18 macros, 1 globale, 1 constante.

Occurrences mesurées en répétition : `ref` 1 662, `decl` 499, `macro_arg` 453, `type_ref` 311, `macro_def` 18, `macro_use` 14, `macro_cond` 9, `designator` 4. Aucun refus, aucun `##`, aucun défaut de l'outil.

Restent en place :
- les 106 mentions `identifier` de sous-étapes suivantes : `test_economy` 28, `test_research_project` 21, `test_main_economy` 18, `test_structure_icon` 16, `economy_score` 13, `economy` 8, `research` 1, `structure_icon` 1 ;
- la mention `string` `pico8_cart_load` (3j) de `test_research_project.c`.

Pour l'étape 4 :
- 219 `backtick` et 45 `comment` dans les fichiers de 3g ;
- les messages d'`assert` qui citent `hy`, `hp`, `jo`, `gb`, `px`, `pico8_research_icon_init`… ;
- le commentaire de `tools/composition_residue.sh` (ligne 124) qui cite `pico8_jo_ex` ;
- le libellé français `"chargement apres pico8_cart_load"`.

### Entrées du TSV corrigées (relevé de 3c : 12 fonctions en 3g)

Corrigées dans `tools/rename_overrides.tsv` (lignes `name`, comme `particle_burst_slot_take` en 3f) et dans les lignes de `docs/renommage.tsv`, avant `rename-slice`. Les nouveaux noms sont absents du TSV et du code : aucune collision.

| ancien | nouveau TSV | corrigé | motif |
|---|---|---|---|
| `pico8_ky` | `final_score` | `economy_score_final` | aucun préfixe et aucun type receveur. Collision avec le champ `final_score` de `score_panel_display` (3j), relevée par 3c : le champ garde son nom, la fonction s'en écarte. `economy_score_final_score` bégaierait. |
| `pico8_db` | `trade_income` | `economy_score_trade_income` | préfixe d'un autre module (`trade`), sans type receveur `trade` |
| `pico8_economy_set_dk` | `economy_set_difficulty` | `economy_score_set_difficulty` | préfixe d'un autre module (`economy`), sans receveur ; le getter est `economy_score_difficulty` |
| `pico8_economy_dk_quitte` | `economy_difficulty_quits` | `economy_score_difficulty_quits` | idem |
| `pico8_jo_ex` | `planet_resource_row` | `research_display_planet_resource_row` | pas de receveur planète : fonction « statique de `jo` » dans le cart, sans planète en paramètre. Même cas que `burst_slot_take` en 3f. |

Pour `pico8_jo_ex`, il y a 1 ligne `identifiant` et 2 lignes `chaîne` (symbole cité, sites `research_display.c` et `composition_residue.sh`) : 7 lignes du TSV en tout.

Les noms qui en dérivent sont revus et gardés. Ce sont des locales et des fonctions statiques de tests nommées d'après le concept, pas des fonctions exportées : `fold_trade_income`, `processor_trade_income`, `bare_trade_income` (3i), `case_final_score_*`, `test_final_score_*`, `test_update_rows_show_final_score`.

Gardées telles quelles, conformes au §4 comme méthodes de leur type receveur (précédents de 3d et 3e) :
- `resource_map_get` et `resource_map_set` : `resource_map_t`, déclaré dans `economy_formulas.h` ;
- `need_new` et `need_resource` : `need_t`, déclaré dans `research.h` ;
- `research_project_tooltip` et `research_project_layout_row` : `research_project_t` ;
- `planet_upkeep` : il prend la planète, comme les `planet_*` de `planet_display`.

Pour décision humaine : `planet_upkeep` reste seul sans le préfixe `economy_score_`, alors que `economy_score_net_income` prend aussi une planète.

### Clés signalées, classées avant renommage

Aucune n'a d'écrivain dans le port.
- **`px`, `lr`, `fj`, `ja`, `il`, `dm`** : ce sont des éléments de `cart_mr_items` et `cart_gp_items` (`test_research_project.c`), copies figées des listes de positions `mr` et `gp` du cart, que seul `DEFAULTS_HASH` replie. Elles sont **mortes comme clés** et vivent comme libellés hachés. On les renomme selon le TSV : `name`, `planet_codes`, `base_cost`, `production_key`, `upgrade_tech`, `byproduct`. Dans la même liste, `"i"` et `"o"` (hors TSV, champs `inputs` et `output` de la même ligne de données) sont renommés `inputs` et `output` par la règle des littéraux composés. La liste se lit alors comme les champs de la fiche de projet, et `DEFAULTS_HASH` bouge de toute façon.
- **`pz`** : ce n'est **pas un nom**. `ri.inputs = "pz"` est une chaîne de deux codes de ressource, lue par `research_project_new`. Mesuré : renommée `open_box`, elle fait rougir `test_wants` (ligne 676). La ligne `chaîne` de `pz` passe à `conservé` (`pz` → `pz`), dans le TSV seulement. La ligne `token pz open_box` de `tools/rename_overrides.tsv` reste : elle nomme les identifiants `pico8_pz*` de `message_box` (3j). La régénération par `rename-strings` est figée pendant l'étape 3 ; on le note dans « Découvert » pour l'étape 5.

### Gardes à motifs (inventaire de 3a, éléments de 3g)

- `PTRS_NON_ENTITE` : `pico8_hy_t.invented` → `economy_t.invented`, `pico8_hp_t.i` → `research_project_t.inputs`.
- `REGISTRES_NON_ENTITE` : `pico8_jo_ex` → `research_display_planet_resource_row`.
- Vivacité sur le vrai arbre, mesurée : après l'application et sans traduction, `--check` sort 1 sur exactement ces 3 lignes (`main_economy.h:29`, `research.h:39`, `research_display.h:26`).
- Fixture de `test_registres_reads_declaration_points_outside_entity_types` (`test_composition_residue.c`, ligne 249) : avec l'exemption traduite et la fixture ancienne, l'assertion de la ligne 259 échoue (rouge d'abord, mesuré). On réécrit `"layout_row_t *research_display_" "planet_resource_row(…"`, et le test passe au vert.
- `PTRS_NON_ENTITE` n'a pas de fixture pour ces deux noms. La vivacité se prouve sur l'arbre.
- `--detail` de `composition_residue.sh` et de `data_residue.sh` : identiques avant et après (mesuré).
- `--champs` : pied 46 → 36 et « 0 écriture » 16 → 15. C'est un majorant, pas une porte : on le note.
- `composition_residue_sites.txt` : aucun nom de 3g.
- Les catégories historiques `aiguillage-nom` (`hp_pose`, `pico8_hp_(mr|gp)_names`) ne voient rien dans le code, ni avant ni après.
- Aucune ligne de commentaire n'est couverte par une exemption traduite.

### Chaînes

Les 25 sites de `rename-strings-verify SUBSTEP=3g`, dont 24 à renommer après la décision sur `pz` :
- **noms de test** : `test_pico8_hy`, `test_pico8_fm` (`printf("…: ok")`), `test_pico8_hp`, `test_pico8_ie` ;
- **libellés** de `check()` de `test_economy.c` : `nx`, `lu`, `lj`, `kt` → `advance_clock`, `spend`, `recompute`, `invent` (imprimés, non hachés) ; `gb` → `projects` ×2 ;
- **libellés hachés** : `ra` → `pos` et `fs` → `placed_items` dans `fold_view_name` (`test_research_project.c`) ; `fs` aussi dans `test_burst.c`, fichier de 3i, renommé par 3g parce qu'elle tient la ligne ;
- **les 6 clés mortes** de `cart_mr_items` et `cart_gp_items` ;
- **3 symboles cités** dans `composition_residue.sh`, couverts par la traduction des gardes, et leurs sites d'écriture ;
- les **2 `unseen`**, formes `type.champ` dans un `.h`, prévus par la note de 3a : `economy_t.invented` (`main_economy.h`) et `research_project_t.inputs` (`research.h`). Vérifiés en répétition : type et champ y sont déclarés sous leurs noms neufs, et l'ancien nom ne reste qu'en commentaire (étape 4).

**Littéraux composés et angles morts de 3g.** On a cherché dans les fichiers de 3g tous les anciens noms renommés du TSV, à l'intérieur des littéraux, littéraux adjacents recollés. Hors messages d'`assert` (étape 4), on renomme, puisque ces littéraux sont écrits et lus dans des fichiers de 3g :
- noms de vue hachés de `test_research_project.c` : `"gc"` → `"satisfied"` (champ de `need_t`, fait bouger `INSTALL_HASH`) et `"fn"` → `"draw_function"` (ne replie aucune donnée hachée aujourd'hui : aucun hachage ne bouge) ;
- libellés imprimés : `"mr gp fk"` → `"positions routed_positions row_tail"` (d'après `CART_POSITIONS`, `CART_ROUTED_POSITIONS`, `row_tail` du TSV) et `"gb named"` → `"projects named"` (`test_research_project.c`) ; `"ke view"`, `"[ke view]"`, `"ke view icons"`, `"ke view struct icons"` → `kind view…` (d'après `KIND_VIEW_HASH`), et `"eu icons"` → `"start_game icons"` (`test_structure_icon.c`).

Restent pour l'étape 4 les clés d'une lettre conservées (`"c"`, `"w"`) et les mots français.

**Hachages** (prévision mesurée en répétition) : **2 tests, 5 constantes** :
- `test_research_project` : `DEFAULTS_HASH` `0xC841FB8Au` → `0x4FE030B1u`, `FUNCTIONS_HASH` `0x538CF904u` → `0x6841C3AEu`, `FUNCTIONS_INVENTED_HASH` `0xA64C8C0Fu` → `0x418A4B5Eu`, `INSTALL_HASH` `0x81BCB6CDu` → `0x45B1D201u` ;
- `test_burst` : `LIFE_HASH` `0x196B693Au` → `0x7DAF9781u`.

On recalcule en laissant le test rougir d'abord, puis on reporte la valeur affichée. `PROJECTS_HASH`, les hachages de `test_economy` et `KIND_VIEW_HASH` ne bougent pas : leurs libellés sont imprimés, pas repliés. Tout autre rouge de hachage arrête la tranche (`help-me-debug`).

**Tampons** : aucun ne reçoit une clé de 3g, puisque seuls des fichiers de tests changent de littéraux.

### Application et recâblage

- Ordre : celui de la note de 3a, sur `SUBSTEP=3g`, après les corrections du TSV.
- Recâblage : la répétition a fait une seule passe après l'application et les chaînes, qui a recâblé **224 identités**. La tranche en fait deux, comme le veut l'ordre de 3a : la seconde ne touche que les identités à offset glissées par les littéraux allongés, tous dans des tests.
- `rename-rekey-check` vert de 3g à 3l (mesuré).
- `usage-mutuel` est muet en répétition (`composition-guard` vert).

### Garde de portée

`scope-count SUBSTEP=3g` vaut **116** : 50 champs, 61 portées > 40, 3 statiques, 2 fonctions. Après la répétition, il vaut **2** : `s` et `od` de `test_structure_icon`, statiques locales déjà proposées par 3a. Aucune nouvelle exception.

### Fin de PRD

La liste du §5 et des notes de 3a–3f, sur `SUBSTEP=3g` :
- `make test-all` vert, `tests/e2e/*.sha256` inchangés ;
- `make raylib` et `make screencast-viewer` compilent et linkent, sans jamais être lancés ;
- `rename-tool-test` (37 tests) et les `rename-*-test` verts ;
- `class-coverage-net` et `e2e-gap` rouges à l'identique de la fin de 3f (sorties égales à l'octet, mesuré) ;
- `data-residue` et `composition-residue` à 0 ;
- après `rename-extract` frais sans `warning:` :
  - `cart-names-count SUBSTEP=3g` : `done` 0 en `identifier` et en `string`, `later identifier` 106 ≤ 106, `later string` 1 ;
  - `rename-strings-verify SUBSTEP=3g` : 0 site en défaut sur 24, 2 `unseen` classés ;
  - `rename-rekey-check` vert de 3g à 3l ;
  - `scope-count SUBSTEP=3g` à 2.

## Implementation Chunks

1. **Sous-étape 3g entière, note comprise.** Une seule tranche : l'application seule fait rougir `composition-guard`, et l'exemption traduite seule fait rougir `test_composition_residue`.
   - Un commit de code qui laisse `make test-all` vert. Il contient :
     - les 5 corrections du TSV et des surcharges, et `pz` passé à `conservé` ;
     - l'application ;
     - les exemptions et la fixture (rouge d'abord) ;
     - les 24 sites de chaînes et les littéraux composés ;
     - les 5 constantes de hachage (rouge d'abord) ;
     - le TSV recâblé deux fois.
   - La note « Découvert » sous l'entrée 3g, dans le même commit ou dans un commit de docs qui suit. Elle relève :
     - les identités et les occurrences ;
     - les entrées corrigées et les 7 gardées ;
     - `pz` ;
     - les recâblages ;
     - les exemptions et leur vivacité, `--detail` et `--champs` ;
     - le classement des 7 clés ;
     - les `unseen` ;
     - les littéraux composés ;
     - les hachages ;
     - ce qui va à la décision humaine : `planet_upkeep`, et la ligne `chaîne` `pz` éditée hors de `rename-strings` ;
     - les relevés de fin.
   - Le plan est corrigé dans le même commit si un constat le contredit.

## Testing Decisions

- **Code de 3g.** Le filet est le compilateur, `make test` et les e2e à l'octet. Aucun test neuf. Les tests de hachage prouvent que seuls les libellés changent : chaque constante rougit d'abord, puis on reporte la valeur affichée.
- **Gardes.** Pas de test neuf. La fixture existante de `test_registres_reads_declaration_points_outside_entity_types` est réécrite : avec l'exemption traduite et la fixture ancienne elle est rouge, réécrite elle est verte. Sur le vrai arbre, chaque exemption retirée fait monter `--check` d'une ligne.
- **`pz`.** Le test existant `test_wants` est la preuve : il rougit si on renomme `"pz"`.
- **Outils de méthode.** Inchangés : le schéma du TSV et ses lecteurs restent figés.

## Out of Scope

- Les noms de 3h–3l dans les fichiers de 3g (le résidu), et `pico8_cart_load` (3j).
- Commentaires, messages d'`assert`, mentions entre backticks ou nues, mots français des littéraux : étape 4.
- Trancher `planet_upkeep`, les exceptions de portée et les motifs génériques : point d'arrêt d'avant l'étape 5.
- Renommer par goût un nom conforme au §4.
- Lancer l'appli raylib.

## Further Notes

- Mesures de cette PRD, sur l'arbre `4ef9b52` avec l'extraction de la fin de 3f, fraîche, sans `warning:` :
  - `cart-names-count SUBSTEP=3g` : `done` 2 511 `identifier`, 219 `backtick`, 20 `string`, 45 `comment` ; `later` 106 `identifier` (borne 106), 120 `backtick`, 1 `string`, 5 `comment` ;
  - `rename-strings-verify SUBSTEP=3g` : 25 sites, 23 en défaut, 2 `unseen` ;
  - `scope-count SUBSTEP=3g` : 116 ;
  - `rename-rekey-check SUBSTEP=3g` : 0 introuvable.

  `tmp/ast_rename/slice.tsv` et `cart_names_count.txt` ont été rendus à leur état d'avant la mesure.
- Répétition à blanc dans un worktree jetable, détruit depuis : corrections du TSV (sans les surcharges), application, gardes, chaînes, composés, hachages, puis `make test`, `make e2e`, `raylib`, `screencast-viewer`, `rename-tool-test`, `class-coverage-net`, `e2e-gap`, extraction fraîche et compteurs. Tous les chiffres ci-dessus en viennent.
- `rename-table-check` reste rouge par construction : la porte est `rename-rekey-check`.
- Durées : chaque `rename-extract` ou `rename-apply` prend environ 1 min 30, et la tranche en enchaîne quatre ou cinq.
