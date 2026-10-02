---
title: "Plan 5, sous-étape 3f — sonde, trou de ver, particules"
description: "La sonde, le ciblage, le trou de ver et les particules (6 modules de game/ et 5 tests) passent aux noms anglais du TSV. Il faut d'abord corriger un faux conflit de l'outil entre constantes d'enum de deux .c, corriger une entrée du TSV contraire au §4 et traduire l'exemption `pico8_emitter_t.qy`."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 3f. **Sonde, trou de ver, particules.** `game/` : `probe`, `targeting`, `wormhole`, `wormhole_display`, `particle`, `particle_zone`. Tests : `test_probe`, `test_probe_spawn_particle_order`, `test_wormhole`, `test_particle_zone`, `test_live_slots`. Résidu ≤ 211.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4), sixième étape.

Entrées : `docs/plan-5.md` §4, §5, l'entrée 3 avec « Sous-étapes » et « Méthode des sous-étapes », et les notes « Découvert » de **3a à 3e**. Cette PRD y renvoie sans les recopier : ordre des commandes (3a), règle d'appartenance des chaînes, angles morts des littéraux composés et recollés, prévision des hachages et des tampons en worktree jetable, commentaires couverts par une exemption (3d), recâblage après chaque édition de texte, porte `rename-rekey-check`, défauts de l'outil corrigés avec leur test, corrections du TSV pour la seule conformité au §4. Exemple à suivre : `3355848` (3e, une seule tranche).

Aucun humain n'était disponible : l'agent rédacteur a chargé lui-même la consigne `guideline` aux points « vérifier avec l'utilisateur » de `to-prd`. Modules, tests et découpage sont tranchés sur le texte du plan, sans interview.

**Condition d'entrée** :
- `origin/plan-5-noms-anglais` au HEAD de départ (`0f1f5d0` ou un descendant), arbre propre, `make test-all` vert ;
- `make rename-extract` frais, sans ligne `warning:` (les 4 lignes `partial` de `class_coverage` sont attendues) ;
- `make rename-rekey-check SUBSTEP=3f` vert. Mesuré : 0 identité introuvable.

## Problem Statement

Après 3e, la sonde, son ciblage, le trou de ver et les particules gardent les noms du cart : `pico8_g__t`, `pico8_probe_t` (le composant de ciblage), `pico8_bv`, `pico8_dl`, `pico8_d_`, `pico8_mz_t`, `pico8_emitter_t`, `pico8_qy_t`, `pico8_zone_particules*`, et les champs `et`, `fo`, `jj`, `ox`, `qy`… La sous-étape compte **599 identités**, soit **4 244 occurrences AST dans 65 fichiers**. Une répétition à blanc dans un worktree jetable (application, gardes, chaînes, puis `make test` et `make e2e`) a trouvé :

- **L'outil refuse l'application.** `ET_JO -> TARGET_PLANET: collides with enum_constant c:@Ea@TARGET_ABSENT@TARGET_PLANET (tests/test_slipway.c:197)`. Ce sont deux constantes d'enum anonymes de deux `.c` différents (`test_wormhole.c`, `test_slipway.c`), chacune invisible hors de son fichier. Dans `_visible_together` (`tools/ast_rename/plan.py`), la dernière règle ne rend locales à leur fichier que les paires dont **les deux** liaisons valent `static`. Une constante d'enum, un type, un `typedef` ont la liaison `-` : deux d'entre eux se rencontrent toujours, d'un `.c` à l'autre. `rename_check`, qui a validé le TSV, applique `meet` : partagé (`.h` ou `extern`) ou même fichier. Défaut de l'outil, comme ceux de 3a–3e.
- **La seule application rougit `composition-guard`, sur 1 ligne** : `registres: src/game/particle.h:44: pointer_list_t *particles;`. L'exemption `pico8_emitter_t.qy` de `REGISTRES_NON_ENTITE` ne mord plus.
- **Une entrée du TSV contredit le §4.** `pico8_ga_place_prendre` → `burst_slot_take` est une fonction exportée de `game/particle` sans type receveur dans son module (`burst` est le type de `planet_family`) : ni `module_verbe`, ni méthode.
- **Des littéraux composés portent des noms du cart, invisibles au compteur et à `rename-strings-verify`** : les noms de zone de `particle_zone.c` (`"particules ga"`, `"place qy"`, `"place qy_fv"`, `"particules ts"`…), lus par les `expect_fatal` de `test_particle_zone.c`. `ga` est une clé de 3a, manquée par 3a.

## Solution

Une seule tranche, dans l'ordre de commandes de la note de 3a, sur `SUBSTEP=3f` :
1. correctif de l'outil et ses deux tests ;
2. correction de l'entrée `burst_slot_take` ;
3. application des 599 identités ;
4. traduction de l'exemption, de la première colonne de `composition_residue_sites.txt` et de la fixture de vivacité ;
5. recâblage ;
6. les 58 sites de chaînes et les littéraux composés de 3f ;
7. nouveau recâblage, relevés, note « Découvert ».

La répétition à blanc l'a établi : avec ces gestes, `make test` et `make e2e` sont verts (26 e2e conformes), `make raylib` et `make screencast-viewer` compilent, aucune constante de hachage ne bouge, aucun tampon ne déborde.

À la fin :
- aucun nom de 3f ne reste en `identifier` ni en `string` ;
- les fichiers de 3f ne gardent que les 211 mentions `identifier` et les 10 mentions `string` de sous-étapes suivantes ;
- `data-residue` et `composition-residue` à 0 ;
- `scope-count SUBSTEP=3f` passe de 88 à 2 ;
- pas un pixel ne change.

## User Stories

1. En tant qu'agent de 3f, je veux que `rename-apply` accepte deux constantes d'enum de deux `.c` renommées du même nom, et refuse toujours une constante d'un `.c` contre une constante d'un `.h` qu'il inclut, afin que l'outil juge comme `rename_check`.
2. En tant qu'agent de 3f, je veux appliquer les 599 identités de `rename-slice SUBSTEP=3f` en une commande, afin que le compilateur arbitre tout oubli dans les 65 fichiers touchés.
3. En tant que lecteur du jeu, je veux `probe_t` pour la sonde et `targeting_t` pour le composant de ciblage, avec `targeting_track`, `targeting_tooltip`, `targeting_confirm`, `target`, `targeted_by`, afin que la décision de vocabulaire de l'étape 0 (`g_` → `probe`, composant → `targeting`) se lise dans le code.
4. En tant que lecteur, je veux `wormhole_t`, `wormhole_too_far_target_t`, `emitter_t`, `particle_t` et `particle_zone_*`, afin que les effets et le trou de ver se lisent sans glossaire.
5. En tant que lecteur, je veux que la fonction qui rend l'arène des éclats s'appelle `particle_burst_slot_take`, afin qu'elle porte le nom de son module comme le §4 l'exige.
6. En tant que mainteneur, je veux `REGISTRES_NON_ENTITE` traduit pour `emitter_t.particles`, avec sa preuve de vivacité, afin que `composition-guard` reste vert sans passer à vide.
7. En tant que mainteneur, je veux `composition_residue.sh --detail` identique avant et après, aux noms traduits des lignes 3–9 de `composition_residue_sites.txt` près, et `data_residue.sh --detail` identique, afin de prouver qu'aucune garde ne change de verdict.
8. En tant qu'agent de 3f, je veux renommer les noms de chaîne de 3f à leurs 58 sites, afin que `rename-strings-verify SUBSTEP=3f` soit vert ; le site `unseen` est vérifié à la main.
9. En tant que relecteur humain, je veux le classement des quatre clés signalées `je`, `ju`, `mo`, `ok` avant leur renommage, afin de savoir si elles vivent.
10. En tant qu'agent de 3f, je veux renommer aussi les noms du cart des littéraux composés lus et écrits dans des fichiers de 3f, afin de ne pas laisser des noms que ni le compteur ni le contrôle ne voient.
11. En tant qu'agent de 3f, je veux savoir d'avance qu'aucune constante de hachage ne change, afin qu'un rouge imprévu arrête la tranche.
12. En tant que mainteneur, je veux que la garde de portée tombe de 88 à 2 dans 3f, les deux restes étant des statiques locales déjà proposées par 3a.
13. En tant que relecteur humain, je veux la liste des noms qui résistent au vocabulaire (`probe_own` sur un `targeting_t`, `probe_link_target` déclaré dans `targeting.h`, noms qui bégaient), afin de les trancher au point d'arrêt.
14. En tant qu'agent de 3g, je veux trouver un TSV recâblé, `rename-rekey-check` vert de 3f à 3l.
15. En tant que joueur, je ne veux voir aucun pixel changer.

## Implementation Decisions

### Périmètre

- Modules : la ligne 3f de `tools/rename_substeps.tsv`. `rename-slice SUBSTEP=3f` rend **599 identités** :
  - par module : `test_live_slots` 197, `probe` 76, `particle_zone` 74, `test_probe` 50, `particle` 48, `test_wormhole` 47, `wormhole` 41, `test_particle_zone` 40, `targeting` 11, `wormhole_display` 9, `test_probe_spawn_particle_order` 6 ;
  - par sorte : 260 fonctions, 129 locales, 59 champs, 41 statiques, 39 macros, 37 paramètres, 16 types, 15 constantes d'enum, 3 tags.
- Occurrences : `ref` 2 345, `macro_arg` 840, `decl` 608, `type_ref` 307, `macro_use` 89, `macro_def` 39, `macro_body` 9, `macro_cond` 6, `designator` 1. 217 identités `homonym`, 89 `short`, aucun `multidef`, aucun refus `##` en répétition.
- Laissés en place : les 211 mentions `identifier` de sous-étapes suivantes (surtout `test_live_slots` 130, `test_probe` 41, `test_wormhole` 19, `probe` 18) ; les 10 `string` de 3l (`"qy"`, `"qy_fv"`, `"ts"`, `"le"`… de `test_live_slots.c`, écrits par `cmd/replay`) ; les fixtures génériques `"pico8_" "g__mm"`, `"pico8_" "ei_dispatch"` (`void-ptr`) et `emitter" "_of`, `g__probe" "_of` (`lien-parent`) de `test_composition_residue.c`.
- Pour l'étape 4 : 177 `backtick` et 25 `comment` des fichiers de 3f ; les messages d'`assert` qui citent `pico8_probe_init()`, `pico8_wormhole_init()`, `f_`, `qy`, `g_`… ; les mots français des littéraux (`particules`, `place`, `pleine`, `particule`, `rangee`).

### Défaut de l'outil : constantes d'enum de deux `.c`

Dans `_visible_together`, la dernière règle devient celle de `meet` dans `tools/rename_check.py` : deux déclarations de portée fichier se rencontrent si l'une est partagée (`_shared` : `.h` ou `extern`) ou si elles sont dans le même fichier. Les paires `static`/`static` gardent leur verdict. Rien d'autre ne change.

Deux tests dans `tests/test_ast_rename.py`, sur le modèle des refus « collides » existants :
- deux constantes d'enum de deux `.c`, l'une renommée du nom de l'autre : acceptée (rouge avant le correctif) ;
- une constante d'un `.c` renommée du nom d'une constante d'un `.h` qu'il inclut : refusée, « collides » (garde contre un correctif trop large ; peut passer avant).

Mesuré en répétition : avec ce correctif, l'application passe sans refus, et `rename-tool-test` (35 tests) reste vert.

Piège à noter : l'USR d'une constante d'enum anonyme est `c:@Ea@<première constante>@<nom>`. Les enums de `test_slipway.c` et `test_wormhole.c` commencent tous deux par `ET_ABSENT` : 3e a renommé par la même identité `TARGET_ABSENT`, `TARGET_FALSE`, `TARGET_SLIPWAY`, `TARGET_OTHER` dans les deux fichiers. Après 3f, `TARGET_PLANET` des deux fichiers a une seule USR : une identité de l'extraction, sans effet puisque les deux sont appliquées.

### Entrée du TSV corrigée

`pico8_ga_place_prendre` → **`particle_burst_slot_take`** (était `burst_slot_take`), dans `tools/rename_overrides.tsv` (ligne `name`) et dans la ligne de `docs/renommage.tsv`, avant `rename-slice`. Nom neuf absent du TSV : aucune collision. Les noms qui en dérivent : aucun.

### Fonctions exportées sans préfixe de module (relevé de 3c : 21 en 3f)

Classées ; toutes appliquées telles quelles sauf la précédente :
- `particle` : `emitter_update`, `emitter_update_method`, `emitter_spawn`, `emitter_init`, `emitter_init_in`, `emitter_release_particles` — méthodes de `emitter_t`, déclaré dans `particle.h`.
- `probe` : `targeting_track`, `targeting_track_method`, `targeting_track_target_class`, `targeting_confirm`, `targeting_confirm_method`, `targeting_tooltip`, `targeting_tooltip_method`, `targeting_tooltip_show` — méthodes du composant `targeting_t`, déclarées dans `targeting.h`, définies dans `probe.c`.
- `probe` : `main_economy`, `main_economy_typed`, `main_economy_placeholder`, `main_economy_placeholder_class` — déclarées dans `main_economy.h`, définies dans `probe.c` : nommées d'après leur en-tête, conformes.
- `wormhole_display` : `wormhole_draw`, `wormhole_draw_method` — méthodes de `wormhole_t`.
- `particle` : `burst_slot_take` — sans type receveur : corrigée.

### Sonde et ciblage : la décision de vocabulaire

Le vocabulaire (étape 0) a tranché : `g_` → `probe`, composant `pico8_probe_t` → `targeting_t`, membre `probe` → `targeting`. 3a a renommé le membre d'en-tête. 3f applique le reste tel que le TSV le dit : `pico8_g__t` → `probe_t`, `pico8_probe` (tag) → `targeting`, `pico8_probe_t` → `targeting_t`, champs `probe` de `probe_t` et `wormhole_t` → `targeting`, fonctions `pico8_probe_*` de `probe.c` → `probe_*`. Pour décision humaine, dans la note, sans correction (le §4 ne les interdit pas) :
- `probe_own`, conservé ×4 (`probe.h`, `wormhole.h`, `planet.h`, `slipway.h`), est le stockage d'un `targeting_t` : après 3f, `targeting_t *targeting;` pointe sur `targeting_t probe_own;`. `targeting_own` suivrait le vocabulaire.
- `probe_link_target` (ex-`pico8_probe_et_hang_fo`) est déclaré dans `targeting.h` mais défini dans `probe.c`.
- Noms qui bégaient : `targeting_track_target_targeted_by`, `TARGET_TOO_FAR_TARGET`, `emitter_init_help_screen_takes_a_particles_help_screen_list`.

### Gardes à motifs (inventaire de 3a, éléments de 3f)

**`REGISTRES_NON_ENTITE`.** `pico8_emitter_t.qy` → `emitter_t.particles` ; `pico8_jo_ex` (3g) et les noms de 3c–3e restent. Vivacité sur le vrai arbre, mesurée en répétition : sans l'exemption traduite, `--check registres` sort 1 sur `particle.h:44`.

**Fixture de `test_composition_residue.c`.** `test_registres_reads_declaration_points_outside_entity_types` exempte `qy` de `"pico8_" "emitter_t"`. Mesuré : l'exemption traduite avec la fixture ancienne fait échouer l'assertion de la ligne 259. On réécrit `*particles` et `"emitter_" "t"`, et le test redevient vert : c'est le test de vivacité. Le `*qy` de `other_t` (test `ptrs-entité`, ligne 234) est un membre neutre de fixture, laissé.

**`composition_residue_sites.txt`.** Première colonne des lignes 4–9 : `pico8_f_`, `pico8_ez`, `pico8_ei`, `pico8_bv`, `pico8_dl`, `pico8_d_` → `emitter_init`, `emitter_spawn`, `emitter_update`, `targeting_track`, `targeting_tooltip`, `targeting_confirm`. Les symboles cités de la troisième colonne (lignes 3–9 : `pico8_d__dispatch`, `pico8_ei_dispatch`, `pico8_mz_pr`, `pico8_mz_ey`, `pico8_mz_hq`…) sont des sites `chaîne`. Mesuré : `--detail` ne diffère que par les lignes 3–9, ces noms et rien d'autre.

**`--champs`.** Rien à traduire : `emitter_init` entre dans le suffixe `_init` (annoncé par 3e). Mesuré : pied 38 → 46 après 3f, par cumul d'écritures sous des noms de membre neufs partagés (`archetype`, `pos`, `is_dead`…). `--champs` est un majorant, pas une porte : on le note.

**Vérifiés à vide.** `lien-parent` (`(emitter|probe)_of`) n'apparie rien dans le code, avant comme après ; `data_residue.sh` (`--detail` identique en répétition) ; `scope_exceptions.txt` ; `class_coverage_*` (noms de classe passés en 3a). Aucune ligne de commentaire couverte par l'exemption traduite.

### Chaînes

Les 58 sites de `rename-strings-verify SUBSTEP=3f` :
- **5 noms de test** dans les `printf("test_X: all tests passed")` : `test_pico8_{places_vivantes,zone_particules,g_,probe_mm_rnd_order,mz}` ;
- **12 cas de `test_particle_zone`** (`ga_zone_full` → `burst_zone_full`…), écrits et lus par `argv[1]` du même fichier, et son chemin `tmp/test_pico8_zone_particules_stderr.txt` ;
- **la clé `ox` → `anchors`** de `test_wormhole.c` ;
- **34 sites de symboles cités**, dont l'`unseen` ci-dessous : `composition_residue_sites.txt`, `composition_residue.sh` (`emitter_t.particles`), leurs déclarations dans `particle.[ch]`, `probe.c`, `wormhole.c`, et `pico8_qy_t` → `particle_t` dans la fixture `void-ptr` ;
- **les 4 clés signalées**, 5 sites ;
- l'**`unseen`** : `emitter_t.particles` à sa déclaration dans `particle.h`. Vérifié en répétition : le type et le champ sont déclarés sous leurs noms neufs, l'ancien nom n'y reste qu'en commentaire (étape 4).

**Clés signalées, classées avant renommage** (aucune n'a d'écrivain) :
- `mo`, `je`, `ok`, `ju` dans `NOMS_DES_SINGLETONS` de `test_live_slots.c` : libellés imprimés (`printf("  singleton %s :")`), jamais cherchés dans un registre. **Mortes comme clés**, vivantes comme libellés : renommées `main_camera`, `main_mouse`, `main_ui`, `main_score_panel`, comme leurs voisins déjà traduits `background`, `help_screen`.
- `je` dans `test_main_mouse.c` : étiquette lue par `entities_by_tag("je")`. Aucune classe n'écrit l'étiquette `je` (3a a traduit toutes les étiquettes ; la souris porte `mouse`, déjà dans le même tableau) : **morte**, la recherche rend toujours une liste vide. Renommée `main_mouse` selon le TSV ; pour décision humaine, la retirer du tableau.

**Littéraux composés de 3f.** Hors TSV, invisibles au compteur, renommés dans 3f parce que leur écrivain et leur lecteur sont des fichiers de 3f ; seuls les noms du cart changent, le français reste à l'étape 4 :
- noms de zone de `SLOT_KINDS` (`particle_zone.c`) : `qy` → `particles`, `qy_fv` → `particles_help_screen`, `ga` → `burst`, `ts` → `transit_dots` ; et les 4 messages attendus de `expect_fatal` dans `test_particle_zone.c` (`"zone particules burst pleine"`, `"zone place particles pleine"`…). Mesuré en répétition : `make test` vert. Les noms de zone ne vont que dans un `%s`, aucun tampon ;
- libellés imprimés de `test_live_slots.c` (lignes ≈ 118, 129, 1430 : `qy=`, `qy_fv=`, `ga=`, `ts=`, `cg.jc`), sans lecteur.

Le tableau `{"particule", "qy", "qy_fv", "burst", "ts"}` de `test_live_slots.c` reste : ce sont des sites `chaîne` de 3l, appariés à `cmd/replay.c`. Recherche faite sur tout le dépôt pour les anciens noms de 3f d'au moins 5 caractères, littéraux adjacents recollés : hors mots communs, il ne reste que les `assert` de l'étape 4 et les fixtures génériques. On la refait au moment de l'édition, et aussi pour les clés courtes (`grep '"[^"]*\b<clé>\b'`).

**Hachages, prévision** (worktree jetable : application, gardes, chaînes, composés) : **0 test, 0 constante**. Les `fnv_str` de `test_probe` replient des identifiants du jeu, pas des clés de 3f. Tout rouge de hachage arrête la tranche (`help-me-debug`). **Tampons** : aucun ne reçoit une clé de 3f.

### Application et recâblage

- Ordre : celui de la note de 3a, sur `SUBSTEP=3f`, après le correctif de l'outil et la correction du TSV.
- Recâblage mesuré en répétition : **281 identités** après l'application (toutes dans des fichiers de 3g–3j : `test_button` 26, `menu` 18…) ; **0** après les chaînes (aucune identité à offset renommée après un littéral édité). La correction `particle_burst_slot_take` peut en ajouter quelques-unes dans `planet_family.c`.
- `usage-mutuel` : muet en répétition (`composition-guard` vert).

### Garde de portée

`scope-count SUBSTEP=3f` vaut **88** : 35 champs, 48 portées > 40, 3 statiques, 2 fonctions (`particle` 25, `probe` 18, `test_probe` 17, `wormhole` 8, `test_live_slots` 6, `test_probe_spawn_particle_order` 4, `test_wormhole` 4, `targeting` 3, `wormhole_display` 3). Mesuré après répétition : **2**, `CV` et `CG` de `test_live_slots`, statiques locales déjà proposées par 3a. Aucune nouvelle exception.

### Fin de PRD

La liste du §5 et des notes de 3a–3e, sur `SUBSTEP=3f` :
- `make test-all` vert, `tests/e2e/*.sha256` inchangés ;
- `make raylib` et `make screencast-viewer` compilent et linkent (jamais lancés) ;
- `rename-tool-test`, avec les deux tests neufs, et les `rename-*-test` verts ;
- `class-coverage-net` et `e2e-gap` rouges à l'identique de la fin de 3e ;
- `data-residue` et `composition-residue` à 0, `--detail` comme ci-dessus ;
- après `rename-extract` frais sans `warning:` : `cart-names-count SUBSTEP=3f` `done` 0 en `identifier` et `string`, `later identifier` ≤ 211 ; `rename-strings-verify SUBSTEP=3f` 0 site en défaut sur 58, 1 `unseen` classé ; `rename-rekey-check` vert de 3f à 3l ; `scope-count SUBSTEP=3f` à 2.

## Implementation Chunks

1. **Sous-étape 3f entière, note comprise.** Une seule tranche : l'application seule rougit `composition-guard`, l'exemption traduite seule rougit `test_composition_residue`.
   - Un commit de code qui laisse `make test-all` vert : correctif `plan.py` et ses deux tests (rouge d'abord) ; correction `particle_burst_slot_take` ; application ; exemption, première colonne du fichier de sites, fixture réécrite (rouge d'abord) ; 58 sites de chaînes ; littéraux composés ; TSV recâblé deux fois.
   - La note « Découvert » sous l'entrée 3f, dans le même commit ou un commit de docs qui suit : identités et occurrences, défaut de l'outil et piège de l'USR d'enum anonyme, entrée corrigée, recâblages (≈ 281 prévu ; 0), exemption et vivacité, `--detail` comparés, `--champs` (pied 38 → 46), classement des 4 clés, `unseen`, littéraux composés, hachages (0), les 21 fonctions sans préfixe, les noms à trancher (`probe_own`, `probe_link_target`, bégaiements, étiquette morte de `test_main_mouse`), relevés de fin.
   - Le plan est corrigé dans le même commit si un constat le contredit. La « Méthode des sous-étapes » gagne, en une phrase, que les littéraux composés écrits et lus dans les fichiers de la sous-étape sont renommés par elle, même quand ils citent une clé d'une autre sous-étape.

## Testing Decisions

- **Outil.** Deux cas neufs dans `tests/test_ast_rename.py` : constantes d'enum de deux `.c` acceptées, constante de `.c` contre constante de `.h` refusée. Ils jugent la table appliquée ou refusée, pas la fonction interne.
- **Gardes.** Pas de test neuf : la fixture existante de `test_registres_reads_declaration_points_outside_entity_types` est réécrite ; avec l'exemption traduite et la fixture ancienne elle est rouge, réécrite elle est verte. Sur le vrai arbre, l'exemption retirée fait monter `--check registres` d'une ligne.
- **Code de 3f.** Le filet est le compilateur, `make test` (dont les `expect_fatal` de `test_particle_zone` pour les noms de zone) et les e2e à l'octet. Aucun test neuf, aucun hachage prévu.
- **Outils de méthode.** Inchangés hors `plan.py` : le schéma du TSV et ses lecteurs restent figés.

## Out of Scope

- Les noms de 3g–3l dans les fichiers de 3f (résidu), et les éléments de garde de 3g–3j (`pico8_jo_ex`, `pico8_hy_t.invented`, `pico8_hp_t.i`, autres lignes de `composition_residue_sites.txt`).
- Les clés `qy`, `qy_fv`, `ts`, `le` de `test_live_slots.c` ↔ `cmd/replay.c` : 3l.
- Commentaires, messages d'`assert`, mentions entre backticks ou nues, mots français des littéraux : étape 4.
- Trancher `probe_own`, `probe_link_target`, l'étiquette morte de `test_main_mouse`, les motifs génériques, les exceptions de portée : point d'arrêt d'avant l'étape 5.
- Renommer par goût un nom conforme au §4.
- Lancer l'appli raylib.

## Further Notes

- Mesures de cette PRD (arbre `0f1f5d0`, `rename-extract` frais, sans `warning:`) : `cart-names-count SUBSTEP=3f` `done` 3 312 `identifier`, 177 `backtick`, 36 `string`, 25 `comment` ; `later` 211 `identifier` (borne 211), 175 `backtick`, 10 `string`, 6 `comment` ; `rename-strings-verify SUBSTEP=3f` 58 sites, 57 en défaut, 1 `unseen` ; `scope-count SUBSTEP=3f` 88 ; `rename-rekey-check SUBSTEP=3f` 0 introuvable. `tmp/ast_rename/slice.tsv` et `cart_names_count.txt` rendus à leur état d'avant la mesure.
- Répétition à blanc (worktree jetable, détruit) : correctif de l'outil, application, gardes, chaînes, composés. Après extraction fraîche : `cart-names-count` `done` 0/0, `later` 211 ; `rename-strings-verify` 0/58 ; `rename-rekey-check` vert de 3f à 3l ; `scope-count` 2. La correction `particle_burst_slot_take` n'y figurait pas.
- `rename-table-check` reste rouge par construction : la porte est `rename-rekey-check`.
- Durées : chaque `rename-extract` ou `rename-apply` prend environ 1 min 30, et la tranche en enchaîne quatre ou cinq.
