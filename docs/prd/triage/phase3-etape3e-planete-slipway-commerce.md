---
title: "Plan 5, sous-étape 3e — planète, slipway, commerce"
description: "La planète, la slipway et les routes commerciales (7 modules de game/ et 5 tests) passent aux noms anglais du TSV. Il faut d'abord corriger un faux conflit de l'outil entre locales sœurs, traduire trois exemptions de composition_residue.sh et étendre le suffixe de constructeur de --champs."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 3e. **Planète, slipway, commerce.** `game/` : `planet`, `planet_display`, `planet_trade`, `planet_family_trade`, `trade`, `slipway`, `slipway_display`. Tests : `test_planet`, `test_planet_spec_defaults`, `test_planet_exports_row`, `test_slipway`, `test_trade`. Résidu ≤ 372.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4), cinquième étape.

Entrées : `docs/plan-5.md` §4, §5, l'entrée 3 avec « Sous-étapes » et « Méthode des sous-étapes », et les notes « Découvert » de **3a à 3d**. Cette PRD y renvoie sans les recopier. Elles fixent :
- l'ordre des commandes (note de 3a) ;
- la règle d'appartenance des chaînes ;
- les angles morts des littéraux composés et des littéraux adjacents recollés ;
- la prévision des hachages dans un worktree jetable ;
- les tampons de nom à taille fixe (3d) ;
- les commentaires qui citent un nom couvert par une exemption (3d) ;
- le recâblage après chaque édition de texte ;
- la porte `rename-rekey-check`, et non `rename-table-check` ;
- les défauts de l'outil, corrigés avec leur test ;
- les corrections du TSV, permises pour la seule conformité au §4.

Exemple à suivre : `e224ec7` (3d, une seule tranche).

Aucun humain n'était disponible : l'agent rédacteur a chargé lui-même la consigne `guideline` aux points « vérifier avec l'utilisateur » de `to-prd`. Les modules, les tests et le découpage sont tranchés sur le texte du plan, sans interview.

**Condition d'entrée** :
- `origin/plan-5-noms-anglais` est au HEAD de départ (`4e806d2` ou un descendant), l'arbre est propre et `make test-all` est vert ;
- `make rename-extract` frais, sans ligne `warning:` (les 4 lignes `partial` de `class_coverage` sont attendues) ;
- `make rename-rekey-check SUBSTEP=3e` vert. Mesuré : 0 identité introuvable.

## Problem Statement

Après 3d, les entités centrales du jeu gardent les noms du cart : `pico8_jo_t`, `pico8_jc_t`, `pico8_trade_t`, `pico8_qe_t`, `pico8_jo_gh`, `pico8_jc_hq`, `g_jo_*`, et les champs `jy`, `oe`, `by`, `cc`, `fo`, `ts`… La sous-étape compte **563 identités**, soit **5 600 occurrences AST dans 64 fichiers**. Une répétition à blanc dans un worktree jetable (application, gardes, chaînes, puis `make test` et `make e2e`) a trouvé trois obstacles :

- **L'outil refuse l'application.** `rename-apply` rejette deux lignes : `mu_str` → `amount_str` « collides with local ». Ce sont deux locales sœurs de `pico8_jo_gh` (`planet_display.c`, blocs `if`/`else` disjoints, lignes 257 et 264), renommées toutes deux en `amount_str`. `tools/ast_rename/plan.py` fait se rencontrer toutes les locales d'une même fonction, alors que `rename_check` ne les fait se rencontrer que si leurs portées se chevauchent. C'est un défaut de l'outil, comme ceux de 3a–3c. C'est aussi le seul cas du TSV, toutes sous-étapes confondues : aucune autre paire de locales renommées d'une même fonction ne reçoit le même nom neuf.
- **La seule application rougit `composition-guard`, sur 3 lignes**, parce que les exemptions nommées en noms du cart ne mordent plus :
  - `ptrs-entité` : `planet.h:39 needs`, `slipway.h:57 transit_dots` ;
  - `registres` : `planet_display.c:88 unmet_needs`.
- **Renommer la chaîne `pico8_jo_struct_init` rougit `test_composition_residue`.** Le nom neuf est `planet_init_instance`, et la fixture de `--champs` le porte. Or `--champs` ne reconnaît un constructeur qu'à son suffixe `(_new|_init)$`. `planet_init_instance` n'en est plus un, et l'écriture de `b` passe « hors constructeur ». L'inventaire de 3a n'avait pas relevé ce motif. Il dérive en silence depuis 3c sur le vrai arbre : `entity_init_instance`, `row_create` et `tooltip_create` changent 11 lignes de `--champs` à HEAD, avec le même pied.

## Solution

Une seule tranche, dans l'ordre de commandes de la note de 3a, sur `SUBSTEP=3e` :
1. correction de l'outil, avec ses tests ;
2. application des 563 identités ;
3. traduction des trois exemptions, de la première colonne de `composition_residue_sites.txt` et du suffixe de constructeur, avec des fixtures de vivacité ;
4. recâblage ;
5. les 39 sites de chaînes ;
6. nouveau recâblage, relevés, note « Découvert ».

La répétition à blanc l'a établi : avec ces gestes, `make test` et `make e2e` sont verts (26 e2e conformes), et aucune constante de hachage ne bouge.

À la fin :
- aucun nom de 3e ne reste en `identifier` ni en `string` ;
- les fichiers de 3e ne gardent que les 372 mentions de sous-étapes suivantes ;
- `data-residue` et `composition-residue` restent à 0 ;
- `--detail` ne change que par le nom traduit de la première colonne ;
- `scope-count SUBSTEP=3e` passe de 138 à 0 ;
- pas un pixel ne change.

## User Stories

1. En tant qu'agent de 3e, je veux que `rename-apply` accepte deux locales sœurs d'une même fonction renommées du même nom, et refuse toujours deux locales dont les portées se chevauchent, afin que l'outil juge comme `rename_check` et que la table validée s'applique sans surcharge.
2. En tant qu'agent de 3e, je veux appliquer les 563 identités de `rename-slice SUBSTEP=3e` en une commande, afin que le compilateur arbitre tout oubli dans les 64 fichiers touchés.
3. En tant que lecteur du jeu, je veux `planet_t`, `slipway_t`, `trade_route_t`, `resource_t`, `trade_offer_t`, `transit_dot_t` et leurs champs complets (`needs`, `slipways`, `outgoing_routes`, `incoming_routes`, `targeted_by`, `transit_dots`, `archetype`…), afin que le cœur de la partie se lise sans glossaire.
4. En tant que lecteur, je veux des méthodes nommées sur leur type receveur (`planet_draw`, `planet_offer_trade`, `slipway_drag`, `slipgate_accept_drop`, `synthesizer_attach_slipway`, `resource_equal`…), afin que le §4 se lise dans les fichiers `_display` et `_trade` comme dans le fichier du type.
5. En tant que mainteneur, je veux que la couverture de classes dise toujours `trade` pour un `trade_route_t` (nom de classe conservé, note de 2b), afin que `class_coverage_net_expected.txt` et `KNOWN[]` ne bougent pas.
6. En tant que mainteneur, je veux `PTRS_NON_ENTITE` et `REGISTRES_NON_ENTITE` traduits pour les éléments de 3e, chacun avec sa preuve de vivacité, afin que `composition-guard` reste vert sans passer à vide.
7. En tant que mainteneur, je veux que `--champs` reconnaisse les constructeurs sous leurs noms neufs (`_create`, `_init_instance`), avec un test qui rougit sans la traduction, afin que le relevé de la règle 1 ne classe pas les constructeurs comme du code ordinaire.
8. En tant que relecteur humain, je veux savoir quels noms le suffixe étendu classe encore mal, afin de trancher le motif générique au point d'arrêt. Ce sont `entity_init_header` (3a) et `allocator_init_slot_kinds` (3c), perdus, et `emitter_init` (3f), `weighted_picker_new` et `random_walk_new` (3d), gagnés.
9. En tant que mainteneur, je veux `composition_residue.sh --detail` identique avant et après, au nom de sa première colonne près (`pico8_jc_hq` → `slipway_drag`), et `data_residue.sh --detail` identique, afin de prouver qu'aucune garde ne change de verdict.
10. En tant qu'agent de 3e, je veux renommer les noms de chaîne de 3e à leurs 39 sites, afin que `rename-strings-verify SUBSTEP=3e` soit vert. Les 3 sites `unseen` sont vérifiés à la main et classés.
11. En tant qu'agent de 3e, je veux savoir d'avance qu'aucune constante de hachage ne change, afin qu'un rouge imprévu arrête la tranche au lieu d'être recopié.
12. En tant qu'agent de 3e, je veux chercher mes noms aussi dans les littéraux composés et recollés de tout le dépôt, afin de ne pas refaire les angles morts de 3a et 3b (`printf("[bv] …")`).
13. En tant que mainteneur, je veux que la garde de portée ne refuse plus rien dans 3e, afin que les 138 refus disparaissent par le seul TSV.
14. En tant qu'agent de 3f, je veux trouver un TSV recâblé après l'application et après les chaînes, avec `rename-rekey-check` vert de 3e à 3l, afin de partir sur une table saine.
15. En tant que relecteur humain, je veux une note « Découvert » sous 3e avec les relevés de fin, le défaut de l'outil, le motif `--champs` et les écarts, afin de les trancher au point d'arrêt.
16. En tant que joueur, je ne veux voir aucun pixel changer.

## Implementation Decisions

### Périmètre et appartenance

- Modules : la ligne 3e de `tools/rename_substeps.tsv`. `rename-slice SUBSTEP=3e` rend **563 identités** :
  - par module : `planet` 89, `test_slipway` 79, `slipway` 78, `planet_family_trade` 56, `planet_display` 53, `trade` 42, `test_planet` 40, `test_trade` 39, `planet_trade` 31, `slipway_display` 26, `test_planet_spec_defaults` 19, `test_planet_exports_row` 11 ;
  - par sorte : 232 fonctions, 126 locales, 82 champs, 30 statiques, 29 constantes d'enum, 27 paramètres, 18 types, 18 macros, 1 tag.
  Le TSV donne les noms neufs : on y renvoie.
- Occurrences par origine : `ref` 3 095, `macro_arg` 1 053, `type_ref` 740, `decl` 655, `macro_use` 20, `macro_def` 18, `designator` 12, `macro_cond` 7. Aucun `##`, aucun `multidef`, aucun `macro_body` : 205 identités `homonym`, 141 `short`.
- Les macros sont surtout :
  - les gardes d'inclusion `PICO8_X_H` → `GAME_X_H` ;
  - les `*_HASH` de `test_slipway`, renommés par leur nom seul (`EB_HASH` → `OTHER_END_HASH`…), valeurs inchangées ;
  - `PICO8_JO_PRISE_OCTETS` → `PLANET_GRAB_ARENA_BYTES`.
- Le tag `pico8_jo_trade_proposal` devient `trade_offer`, à côté de son `typedef` `trade_offer_t` : deux identités, comme en 2a.
- Classe `trade` : le type devient `trade_route_t`. Le nom de classe `"trade"` est conservé, avec `class_coverage_record("trade")` et `KNOWN[]`. `class-coverage-net` reste rouge à l'identique (`trade` 247/263).
- Laissés en place :
  - les 372 mentions `identifier` de sous-étapes suivantes dans les fichiers de 3e, surtout `planet_display` 68, `test_slipway` 67 et `test_planet_spec_defaults` 62 ;
  - les fixtures génériques `"pico8_" "jo_t"` de `test_composition_residue.c`, hors celles que la vivacité réécrit (ci-dessous) ;
  - `"pico8_" "jo_default"`, `"jo_" "default_fixed"` et `"pico8_" "jo_truthy"` de `test_data_residue.c`, violations génériques ou historiques plantées.
- Hors 3e, pour l'étape 4 :
  - 128 `backtick` et 28 `comment` des fichiers de 3e ;
  - les messages d'`assert` qui citent un ancien nom : `pico8_jo_init()` ×3, `pico8_jc_init()` ×2, `pico8_planet_family_trade_init()` ×3, `` `pico8_qe_t` ``, `` `pico8_trade_qe` ``, et les `bv` de `probe.c` ;
  - les commentaires de `composition_residue.sh` (`jo.jy`, `jc.ts`…) ;
  - le texte libre de la troisième colonne de `composition_residue_sites.txt` (`jo.oe`, `entities_by_tag("jo")`).
- `"prise"`, le nom d'arène de `planet.c`, est un mot français dans une chaîne. Il ne figure dans aucune ligne `chaîne` du TSV : il relève de l'étape 4, compteur de français.

### Défaut de l'outil : locales sœurs

Dans `_visible_together` (`tools/ast_rename/plan.py`), deux locales de la même fonction et du même fichier ne se rencontrent que si leurs portées se chevauchent. Une portée va de la ligne de déclaration à cette ligne plus `scope_lines` ; une portée inconnue va jusqu'à la fin. C'est la règle de `overlap` dans `tools/rename_check.py`, qui a validé le TSV. Rien d'autre ne change.

Deux tests dans `tests/test_ast_rename.py` :
- deux locales de blocs disjoints renommées du même nom : acceptées, les deux réécrites ;
- une locale dont la portée englobe l'autre : refusée, « collides ».

Ils sont rouges avant le correctif, sur la règle actuelle. Répétition à blanc avec ce correctif : l'application passe sans refus, 5 600 occurrences dans 64 fichiers.

### Fonctions exportées sans préfixe de module (relevé de 3c : 40 en 3e)

`planet_display` en compte 13, `planet_trade` 5, `planet_family_trade` 17, `slipway_display` 4 et `trade` 1. Ce sont des méthodes de `planet_*`, `slipway_*`, `infraspace_gate_*`, `synthesizer_*`, `slipgate_*`, et `resource_equal` sur `resource_t`, déclaré dans `trade.h`. Toutes portent le nom de leur type receveur : conformes au §4, **aucune correction du TSV**. La liste va dans la note. Un nom qui bégaie mais respecte le §4, comme la macro de test `LL_DG_CALLS` → `UPDATE_VANISHING_UPDATE_ENTITIES_CALLS`, est appliqué tel quel et signalé.

### Gardes à motifs (inventaire de 3a, éléments de 3e, plus un oubli)

**`PTRS_NON_ENTITE`.** `pico8_jo_t.jy` devient `planet_t.needs`, et `pico8_jc_t.ts` devient `slipway_t.transit_dots`. `pico8_hy_t.invented` et `pico8_hp_t.i` (3g) restent.

**`REGISTRES_NON_ENTITE`.** `pico8_jo_gh_ctx_t.ji` devient `planet_inspection_panel_ctx_t.unmet_needs`. `pico8_emitter_t.qy` (3f) et `pico8_jo_ex` (3g) restent.

**Vivacité sur le vrai arbre** (mesurée à HEAD) : chaque exemption retirée seule fait sortir `--check ptrs-entité,registres` en 1, avec une ligne (`planet.h:39`, `slipway.h:57`, `planet_display.c:88`). Après la traduction, la même mesure en noms neufs doit donner la même ligne.

**Fixtures de `test_composition_residue.c`.**
- `test_ptrs_members_count_only_on_entity_types` exempte `jy` de `"pico8_" "jo_t"`. Mesuré : la garde traduite avec la fixture ancienne fait échouer l'assertion de la ligne 237. On réécrit `*needs` et `"planet_" "t"`, et le test redevient vert : c'est le test de vivacité.
- `test_ptrs_entite_reads_entity_members_like_registres` (lignes 361–381) et `test_readers_see_qualifiers…` (416–446) sont réécrits de même, `needs, *other_needs` et `needs; … *hidden`, avec leurs sorties attendues. Le cas teste toujours que le nom exempté l'est seul sur sa ligne. Verdicts et comptes restent inchangés.

**`--champs`, suffixe de constructeur.** C'est un oubli de l'inventaire de 3a : le motif est générique.
- Traduction : `(_new|_init)$` devient `(_new|_init|_create|_init_instance)$`, et le commentaire d'usage de l'en-tête du script suit.
- Vivacité : la fixture de `test_champs_counts_writes_and_writes_outside_constructors` porte les chaînes de 3e `pico8_jo_struct_init` → `planet_init_instance` et `pico8_jo_hk` → `planet_new_year`. Renommée avec l'ancien suffixe, elle fait échouer l'assertion de la ligne 751 : c'est le rouge. Le suffixe étendu la rend verte, avec la même sortie attendue.
- Sur le vrai arbre, le suffixe étendu change 11 lignes de `--champs` à HEAD. C'est la dérive de 3c et 3d, réparée en passant ; le pied reste à 42.
- Après 3e, le pied passe à 38. Ce n'est pas un effet de constructeur : `--champs` est un majorant **par nom de membre**, et les noms neufs (`pos`, `is_dead`, `targeting`…) cumulent les écritures de plusieurs types. `--champs` n'est pas une porte. On le note, on ne le maquille pas.
- Reste pour décision humaine, dans la note : `entity_init_header` et `allocator_init_slot_kinds` ne finissent plus par `_init` ; `emitter_init` (3f, ex-`pico8_f_`), `weighted_picker_new` et `random_walk_new` y entrent.

**`composition_residue_sites.txt`.**
- Première colonne de la ligne 11 : `pico8_jc_hq` devient `slipway_drag`. C'est la seule différence attendue de `--detail`, et la répétition à blanc l'a vérifié.
- Les symboles cités des colonnes 3–4 des lignes 3, 8, 9 et 11 sont des sites `chaîne` (ci-dessous).

**Vérifiés à vide.** `data_residue.sh` (aucun motif ni exemption de 3e, `--detail` identique en répétition), `scope_exceptions.txt`, `line_coverage_reference.txt` (chemins seuls), `class_coverage_*` (nom `trade` conservé).

**Commentaires couverts par une exemption** (piège de 3d) : aucune des trois exemptions n'agit sur le texte d'une ligne. Elles portent sur un `type.membre` ou sur un nom de déclaration lus par l'analyse. La répétition n'a rien trouvé. Si `--check` montre une ligne de commentaire, on la renomme dès 3e, comme en 3d, et on le note.

### Chaînes

Les 39 sites de `rename-strings-verify SUBSTEP=3e` :
- **Clés `"eb"` → `"other_end"` et `"bv"` → `"targeting_track"**. Ce sont des libellés de `check()` dans `test_slipway.c`, imprimés et jamais hachés. Le littéral composé `printf("[bv] on the slipway at %d/8\n", …)` change avec eux (angle mort de 3a).
- **5 noms de test** : `test_pico8_{jo,jo_spec_defaults,planet_gh_row_lq,jc,trade}` deviennent `test_planet`, `test_planet_spec_defaults`, `test_planet_exports_row`, `test_slipway` et `test_trade`.
- **16 symboles cités, à 32 sites** :
  - les fonctions et dispatchs de `pico8_jo_*` et `pico8_jc_*` dans `composition_residue_sites.txt` et à leur déclaration ;
  - `pico8_jo_struct_init`, `pico8_jo_hk` et `qe_is_bool` dans les fixtures de `test_composition_residue.c`. `self->resource_is_bool` reste un faux positif de `un-ou-faux`, vide comme avant ;
  - les trois `type.champ` de `composition_residue.sh`.
- **3 `unseen`**. Ce sont les `type.champ` (`planet_t.needs`, `planet_inspection_panel_ctx_t.unmet_needs`, `slipway_t.transit_dots`) à leurs sites de déclaration, `planet.h`, `planet_display.c` et `slipway.h`. On vérifie à la main que le type et le champ sont déclarés sous leurs noms neufs et que l'ancien nom est absent.

Littéraux composés et recollés : la recherche sur tout le dépôt, pour les 378 anciens noms de 3e d'au moins 5 caractères, n'a rien trouvé d'autre. Seuls restent les messages d'`assert` de l'étape 4, les fixtures génériques et `qe_is_bool`, déjà site. On la refait au moment de l'édition.

**Hachages, prévision** (worktree jetable, application, gardes et chaînes) : **0 test, 0 constante**. `make test` est vert et `make e2e` vert, 26 e2e conformes. Aucun libellé haché ne change : les `fnv_str` de `test_planet` et `test_slipway` replient des identifiants de planète et des chaînes du jeu, pas les clés de 3e. Tout rouge de hachage arrête donc la tranche (`help-me-debug`).

**Tampons** : aucun ne reçoit une clé de 3e. `char id[8]` de `test_planet` et `ids[…][8]` de `test_slipway` tiennent des identifiants `"p950"`, `"p1"`. Aucune clé n'est copiée dans un tableau de taille fixe. Les messages d'`assert` sont jetés (`must_abort` redirige `stderr` vers `/dev/null`), et aucun test de 3e ne lit un texte d'`assert`.

### Application et recâblage

- Ordre des commandes : celui de la note de 3a, sur `SUBSTEP=3e`, le correctif de l'outil d'abord.
- Prévision du recâblage après l'application : **≈ 460**. Ce sont les identités à offset de 3f–3l situées, dans leur fichier, après un token de 3e réécrit à longueur changée. Les estimations de cette méthode ont tenu à 2 % près (3d : prévu 670, constaté 657).
- Prévision du recâblage après les chaînes : **≈ 0**. Les littéraux édités sont dans les tests de 3e, déjà appliqués, dans `test_composition_residue.c`, sans USR à offset renommée, et dans des fichiers non C.
- Risque `usage-mutuel` (précédent de 3c) : le TSV croise des noms neufs de 3e avec d'autres déclarations, `number` (fonction de `draw_functions` contre champs de `map_tables` et `trade`), `to_int` (statiques de `hud` et `slipway_display`) et `status_text` (fonction de `cmd/frames` contre champs). Mesuré en répétition : `usage-mutuel` reste muet. On le relit tout de même sur le vrai arbre.

### Garde de portée

`scope-count SUBSTEP=3e` vaut **138** au départ :

| module | champs | constantes d'enum | statiques | portée > 40 |
|---|---:|---:|---:|---:|
| `planet` | 25 | | | |
| `slipway` | 18 | | | 5 |
| `trade` | 7 | | | 2 |
| `planet_trade` | 4 | | | 10 |
| `planet_display` | 3 | | | 8 |
| `slipway_display` | 3 | | | 7 |
| `planet_family_trade` | | | | 7 |
| `test_planet` | 1 | 9 | 1 | 4 |
| `test_planet_exports_row` | | | 1 | 3 |
| `test_slipway` | | | | 14 |
| `test_trade` | | | | 6 |

Le TSV renomme **les 138** en nom long. Attendu en fin : **0**. Aucune exception n'est proposée.

### Fin de PRD

On reprend la liste du §5 et des notes de 3a–3d, sur `SUBSTEP=3e` :
- `make test-all` vert, `tests/e2e/*.sha256` inchangés ;
- `make raylib` et `make screencast-viewer` compilent et linkent (jamais lancés) ;
- `rename-tool-test`, avec les deux tests neufs, et les `rename-*-test` verts ;
- `class-coverage-net` et `e2e-gap` rouges à l'identique de la fin de 3d ;
- `data-residue` et `composition-residue` à 0, `--detail` comme ci-dessus ;
- après `rename-extract` frais sans `warning:` :
  - `cart-names-count SUBSTEP=3e` : `done` 0 en `identifier` et en `string`, `later identifier` ≤ 372 ;
  - `rename-strings-verify SUBSTEP=3e` : 0 site en défaut sur 39, 3 `unseen` classés ;
  - `rename-rekey-check` vert de 3e à 3l ;
  - `scope-count SUBSTEP=3e` à 0.

## Implementation Chunks

1. **Sous-étape 3e entière, note comprise.** Une seule tranche. L'application, les gardes et les chaînes sont liées : l'application seule rougit `composition-guard`, et les chaînes rougissent `test_composition_residue` sans le suffixe. Séparées, `make test` rougirait entre deux commits.
   - Un commit de code qui laisse `make test-all` vert. Il contient :
     - le correctif `plan.py` et ses deux tests, rouges d'abord ;
     - l'application ;
     - les trois exemptions et la première colonne du fichier de sites ;
     - le suffixe de `--champs` ;
     - les fixtures réécrites, rouges d'abord ;
     - les 39 sites de chaînes et `[bv]` ;
     - le TSV recâblé deux fois.
   - Puis la note « Découvert » sous l'entrée 3e, dans le même commit ou dans un commit de docs qui suit. Elle relève :
     - les identités et occurrences appliquées ;
     - le défaut de l'outil ;
     - les recâblages (≈ 460 prévu, constaté ; ≈ 0) ;
     - les exemptions et leur vivacité ;
     - le motif `--champs`, avec la dérive de 3c–3d, le pied 42 → 38 et les noms à trancher ;
     - les `--detail` comparés ;
     - les `unseen` classés ;
     - les littéraux composés ;
     - les hachages (0 prévu, constaté) ;
     - les 40 fonctions sans préfixe de module ;
     - le nom `trade` conservé ;
     - les relevés de fin.
   - Le plan est corrigé dans le même commit si un constat le contredit. La « Méthode des sous-étapes » gagne, en une phrase, l'oubli de l'inventaire : le suffixe de constructeur de `--champs`.

## Testing Decisions

- **Outil.** Deux cas neufs dans `tests/test_ast_rename.py`, sur le modèle des refus « collides » existants : des locales sœurs acceptées, des locales chevauchantes refusées. Ils jugent le comportement observable, table appliquée ou refusée, pas la fonction interne.
- **Gardes.** On ne crée pas de test neuf : on réécrit les fixtures existantes de `test_composition_residue.c`.
  - Pour `ptrs-entité` : la fixture en noms neufs avec l'exemption ancienne est rouge, l'exemption traduite la rend verte.
  - Pour `--champs` : la fixture renommée avec l'ancien suffixe est rouge, le suffixe étendu la rend verte.
  - Sur le vrai arbre, chaque exemption retirée fait monter `--check` d'une ligne.
- **Code de 3e.** Le filet est le compilateur, `make test` et les e2e à l'octet. On n'écrit aucun test neuf, aucun hachage n'est prévu.
- **Outils de méthode.** Ils restent inchangés hors `plan.py` : le schéma du TSV et ses lecteurs restent figés.

## Out of Scope

- Les noms de 3f–3l rencontrés dans les fichiers de 3e (résidu), et les éléments de garde de 3f–3j : `pico8_emitter_t.qy`, `pico8_jo_ex`, `pico8_hy_t.invented`, `pico8_hp_t.i`, les autres lignes de `composition_residue_sites.txt`.
- Commentaires, messages d'`assert`, mentions entre backticks ou nues, `"prise"` : étape 4.
- Trancher les motifs génériques, dont le suffixe de `--champs` au-delà de `_create|_init_instance`, retirer une exemption morte, accepter des exceptions de portée : point d'arrêt d'avant l'étape 5.
- Renommer par goût un nom conforme au §4.
- Changer `--champs` au-delà de son suffixe, par exemple le rendre exact par type.
- Lancer l'appli raylib.

## Further Notes

- Mesures de cette PRD (arbre `4e806d2`, `rename-extract` frais) :
  - `cart-names-count SUBSTEP=3e` : `done` 3 619 `identifier`, 128 `backtick`, 7 `string`, 28 `comment` ; `later` 372 `identifier` (borne 372), 244 `backtick`, 0 `string`, 6 `comment` ;
  - `rename-strings-verify SUBSTEP=3e` : 39 sites, 36 en défaut (état de départ), 3 `unseen` ;
  - `scope-count SUBSTEP=3e` : 138 ;
  - `rename-rekey-check SUBSTEP=3e` : 0 introuvable.

  `tmp/ast_rename/slice.tsv` et `cart_names_count.txt` ont été rendus à leur état d'avant la mesure.
- Répétition à blanc (worktree jetable, détruit) : correctif de l'outil, application, gardes, chaînes. Aucun refus après le correctif. `make test` et `make e2e` sont verts, `rename-strings-verify SUBSTEP=3e` 0/39, `data_residue --detail` identique. `composition_residue --detail` ne diffère que par la ligne 11.
- `rename-table-check` reste rouge par construction : la porte est `rename-rekey-check`.
- Durées : chaque `rename-extract` ou `rename-apply` prend environ 1 min 30, et la tranche en enchaîne quatre ou cinq.
