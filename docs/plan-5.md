# Plan 5 — Des noms clairs, en anglais

Ce document est **le plan** ; aucune implémentation n'est engagée par sa rédaction. Exécution prévue par `plan-pilot`, une PRD par étape ou sous-étape (§6), avec trois points d'arrêt humains (après les étapes 0, 2b et 4). `plan-pilot` ne s'arrête pas de lui-même : il est donc lancé **par tronçons** — étape 0 seule, puis étapes 1–2b, puis étapes 3–4, puis étape 5. Un tronçon ne reprend qu'après une validation humaine tracée dans le dépôt (le vocabulaire, puis le TSV, puis les exceptions, marqués validés), que l'étape suivante vérifie comme condition d'entrée. La PRD de la première étape d'un tronçon (1, 3a, 5) n'est écrite qu'après la validation qui l'ouvre, et renvoie à l'artefact validé sans en recopier le contenu ; une version écrite en parallèle avant est jetée. Pour `plan-pilot`, chaque tronçon est une phase, et chaque étape ou sous-étape numérotée du §6 une étape de cette phase. Tout le chantier se fait sur **une seule branche**, celle de ce plan : chaque étape y pousse ses commits, aucune PR n'est mergée en cours de route. La condition d'entrée d'une étape vérifie que les commits de la précédente sont poussés. La relecture humaine des commits se fait aux points d'arrêt, entre tronçons.

Une validation humaine ne suffit pas seule : la condition d'entrée qui suit un point d'arrêt (étape 1 après le vocabulaire, 3a après le TSV) propage d'abord les corrections humaines aux noms qui en dérivent, puis rejoue le contrôle mécanique (exhaustivité, collisions, noms cibles de fichiers uniques) sur la version validée.

## 1. Objectif

À la fin, chaque fichier, type, fonction, champ de struct, constante et globale du code porte un nom anglais clair. Plus aucun digramme du cart minifié (`jo`, `jc`, `g_`…), plus aucun mot français dans le code.

Seule tolérance, à la Go : une variable locale ou un paramètre d'une ou deux lettres (`i`, `n`, `e`, `p`, `dx`) quand sa portée tient en **40 lignes au plus**.

## 2. Décisions actées, à ne pas rouvrir

- **On coupe le cordon avec le cart.** Le jeu marche et les e2e le couvrent : le C devient la référence. La règle « portage littéral, fonction par fonction » sort du `CLAUDE.md`. `pico8/lua.lua` reste dans le dépôt, comme archive.
- **Pont inverse unique : `docs/renommage.tsv`** (`ancien`, `nouveau`, `fichier`), produit à l'étape 2b. C'est lui qui sert si on doit retourner fouiller le Lua. `docs/glossaire-noms.md` est supprimé.
- **Périmètre de l'anglais : le code seul.** `src/`, `tests/`, `tools/`, `Makefile`, fixtures, messages d'`assert`, commentaires. Les docs, le `CLAUDE.md`, les skills et les messages de commit restent en français.
- **Pas de préfixe pour le jeu.** `p8_` pour les seuls symboles du runtime PICO-8 émulé.
- **`main` gelé** pendant tout le chantier : rien n'est en vol ; toutes les étapes poussent leurs commits sur une même branche, sans merge intermédiaire.

## 3. Arborescence cible

```
src/
  p8/         runtime PICO-8 émulé — symboles en p8_, fichiers sans préfixe
              fixed, gfx, palette, mem, str, input, audio (stub), assets,
              quarter_wave, fatal
  engine/     socle objet et mémoire, sans planète ni slipway
              entity_class.h, entity (classe racine), entity_registry,
              entity_list, pointer_list, entity_geom, vec, geom, collect,
              dispatch_context, zones mémoire, exit_queue, bootstrap
  game/       le jeu
              planet*, slipway*, trade, research*, economy*, galaxy, gen,
              probe, targeting, wormhole*, ui, ui_button, ui_zone, menu,
              hud, message_box, layout, help, score_panel*, scroll,
              particle, particle_zone, background, mouse, tech_gate, cheat,
              game_data, map_tables, data_init, cart…
  platform/   raylib_backend, raylib_backend_coords, fatal_pc
  devtools/   vérification sans fenêtre : dump, gif, screencast*, replay_*,
              session_recorder, input_json, class_coverage, minimize,
              fixed_test_double
  cmd/        un fichier par binaire, à plat : game.c, replay.c, frames.c,
              gif.c, minimize.c, screencast_viewer.c, class_coverage.c
tests/        reste à plat ; fichiers renommés (test_planet.c…), pas déplacés
```

La liste des fichiers par répertoire est indicative : l'étape 1 tranche chaque fichier et le note dans « Découvert ».

Les includes s'écrivent depuis `src/` (`#include "game/planet.h"`), avec `-Isrc` seul.

## 4. Règles de nommage

| Élément | Règle | Exemple |
|---|---|---|
| Fichier | nom du module, sans préfixe, y compris dans `p8/` | `game/planet.c`, `p8/gfx.c` |
| Type | `snake_case_t` | `planet_t`, `trade_route_t`, `p8_fixed_t` |
| Fonction exportée | `module_verbe[_objet]`, module = nom du fichier | `planet_draw`, `slipway_find_endpoint` |
| Méthode de classe | même règle, sur le type receveur | `entity_reset_state` |
| Fonction `static` | pas de préfixe de module | `find_nearest` |
| Constante, enum, macro | `MODULE_UPPER_CASE` | `TAG_PLANET` |
| Champ de struct | nom complet, jamais abrégé | `e->map_pos`, pas `e->p` |
| Globale | nom complet ; `g_` seulement pour une statique mutable de fichier | `g_frame_count` |
| Locale, paramètre | 1–2 lettres permises si portée ≤ 40 lignes ; idiomes encouragés | `i`, `n`, `e`, `p`, `dx` |

Exceptions à la règle des deux lettres : `x` et `y` comme champs d'un vecteur. La liste d'exceptions de la garde (§5) ne s'allonge que par une décision humaine : les étapes 3 et 4 proposent leurs exceptions (portée, faux positifs du compteur de français) dans leur note « Découvert », l'humain les tranche toutes au point d'arrêt d'avant l'étape 5. Le critère de fin d'une sous-étape 3 ou 4 est donc « compteur à zéro, hors exceptions proposées en attente ».

**Commentaires.** Un commentaire ne cite plus jamais le cart : chaque nom Lua est remplacé par le nouveau nom C, et une phrase « the cart does X » devient « X » ou disparaît. La règle des quatre destinations du `CLAUDE.md` reste, moins sa première branche (le sens d'un nom minifié) : il n'y a plus de nom minifié. Les étiquettes `TROU-N` deviennent `GAP-N`.

## 5. Invariant et gardes

**Invariant de chaque commit** (à l'étape 1, de chaque paire déplacement + retouches, cf. §6) : `make test-all` passe et tous les `*.sha256` de `tests/e2e/` restent identiques. **En fin de chaque PRD**, s'y ajoutent les cibles hors suite qui compilent du code ou lisent des noms et des chemins : `make raylib` et `screencast-viewer` (après `raylib-vendor-setup`), `class-coverage-net`, `e2e-gap`. Renommer ne change pas un pixel. Les références qui contiennent des chemins ou des noms (`line_coverage_reference.txt`, listes d'exceptions de `tools/`) sont mises à jour dans le même commit que ce qui les invalide, jamais régénérées pour masquer une différence de contenu.

**Deux gardes permanentes**, branchées tôt en compteur non bloquant par module, puis rendues bloquantes dans `make test` à l'étape 5 :

1. **Anti-cart** (compteur dès l'étape 2b). Aucun ancien nom du TSV n'apparaît dans le code, ni comme identifiant, ni entre backticks, ni dans une chaîne. Les noms trop courts pour une recherche texte (`p`, `t`, `r`) sont vérifiés via l'AST clang.
2. **Portée, à la Go** (compteur dès l'étape 3). Depuis l'AST clang : un identifiant de 1 ou 2 caractères est refusé s'il nomme un type, une fonction, un champ, une constante ou une globale ; il n'est permis qu'à une locale ou un paramètre dont la portée tient en ≤ 40 lignes, hors liste d'exceptions (§4).

## 6. Étapes

Chacune est une PRD, poussée sur la branche du chantier, et respecte l'invariant du §5.

0. **Vocabulaire de base — point d'arrêt humain.** Le dictionnaire des ~50 concepts récurrents du domaine (`jo`, `ga`, `jc`, `g_`, `pj`, `hp`, `lq`…), les champs de l'en-tête commun (`pico8_struct_header_t`) et les membres de la vtable des classes (`kj`, `ev`, `hd`, `hk`, `hb`…), présenté **rempli** à partir du glossaire et de la lecture du code. L'humain corrige ; aucun code n'est touché. Y figure aussi l'arborescence résolue : chaque fichier actuel, son répertoire et son nom cible, que l'étape 1 applique sans rien inventer. Le vocabulaire et l'arborescence validés sont l'entrée des étapes 1 à 3. Dans la même étape, avant que le code ne bouge, les **consignes aux agents** changent : le `CLAUDE.md` et les skills qui l'encodent (au moins `fix-bug`) perdent le portage littéral jugé contre `lua.lua` et la branche « nom minifié → glossaire », pour qu'un agent des étapes 1 à 4 — ou un `fix-bug` lancé par `check-e2e` en cours de route — ne reçoive pas deux règles contraires. Le glossaire, lui, reste jusqu'à l'étape 5 : c'est une entrée des étapes 0 et 2.

   **Découvert** (détail dans `docs/vocabulaire.md`, colonne « doute ») :
   - 83 concepts plutôt que ~50. Le code corrige le glossaire : `ma` est une secousse (`shake`), pas un halo ; `hf` vaut `is_planet` ; `ra` est la position en pixels (`pos`) et `p` la position en cases (`map_pos`), d'où l'exemple du §4 corrigé ; `dc` ne route plus que le dessin (`draw_layers`) ; l'archétype s'appelle `pico8_X_def` (→ `X_archetype`), pas `pico8_X_start`.
   - Noms qui résistent au §4, à trancher : `g_` → `probe` force le composant `pico8_probe_t` et le membre `probe` à devenir `targeting` ; `cls` → `class` est un mot réservé C++ (`entity_class` sinon) ; `fg` → `gpio_signal` ne repose que sur deux porteurs.
   - Doubles sens : `kf` (vtable `attach_slipway`, `lw` `accept`), `hp`, `nk`, `cr`.
   - Hors liste du §3 (corrigé en conséquence) : `mouse_reader` → `game/mouse` et `ptrs` → `engine/pointer_list` quittent `p8/` ; `struct` → `engine/entity_class.h`, `entity` → `engine/entity_registry`, `ents` → `engine/entity_list` ; `msgbox` → `message_box`, `bg` → `background`, `coverage` → `devtools/class_coverage` ; en plus dans `engine/` : `entity_geom`, `vec`, `geom`, `zone_sizes.h` ; dans `game/` : `ui_zone`, `particle_zone`, `main_economy.h`, `mouse_button`, `draw_functions`.
   - `devtools/input_json.c` définit des fonctions déclarées dans `p8/input.h` : en-tête peut-être à scinder, noté, non traité (§7).
   - `tools/probe.p8` → `pico8_reference_probe.p8` : `probe` nomme désormais la sonde du jeu.
   - Résidu hors périmètre : le `README` énonce encore le portage littéral, et le `CLAUDE.md` y renvoie pour la cible ; il est mis à jour à l'étape 5.
1. **Déplacements seuls.** `git mv` vers l'arborescence du §3 (noms de fichiers anglais du vocabulaire), includes en `"dir/fichier.h"`, `-Isrc`, `Makefile`, chemins dans `tools/`. Aucun symbole ne change. Commits de `git mv` purs, séparés des retouches de contenu, pour que `git log --follow` suive : le commit de `git mv` seul ne compile pas, l'invariant du §5 se vérifie sur la paire qu'il forme avec son commit de retouches, sans commit intermédiaire.

   **Découvert** (PRD `docs/prd/triage/phase2-etape1-deplacements.md`) :
   - Rangement conforme au §3. Seul hors de sa liste : `game/structure_icon` (ex-`pico8_research_icon`, concept `ie`). Les doutes du vocabulaire (`trade`, scission de `p8/input.h`, `main_*`, `entity_registry`, `main_economy.h`) sont appliqués tels quels, non tranchés.
   - Arbre final : `src/` sans fichier à plat ; `.c`/`.h` par répertoire : `p8` 8/10, `engine` 15/17, `game` 43/45, `platform` 3/2, `devtools` 11/10, `cmd` 7/0, soit 87/84 ; `tests/` à plat, 91 `.c` et `heap_allocator.h` (ex-`zone_tas.h`) ; `tools/gen_quarter_wave.c` seul `.c` de `tools/`.
   - Includes : tous en `"dir/fichier.h"` depuis `src/`, tests compris (plus aucun `"../src/"` ni `"pico8_…"`) ; seules exceptions, `"heap_allocator.h"` local aux tests et `"raylib.h"`. `-Isrc` par `override CFLAGS += -Isrc` après `CFLAGS ?=`, seul `-I` du port ; `-I third_party/raylib/src` reste dans les deux recettes raylib ; `constants_compare.sh` passe `-I` sur la racine des sources. Pas de `compile_commands.json`. Noms des cibles `make` inchangés ; `.d` relus par `src/*/*.d`.
   - Cibles rouges avant l'étape, inchangées après : `class-coverage-net` (`trade` 247/263, `lq` 2742/2763, même sortie au décalage de ligne du `Makefile` près) et `e2e-gap` (même 2932 lignes, chemins réécrits). Références périmées depuis `346651f` et `536ddd4`, sans changement de comportement ; hors périmètre du plan 5, tâche séparée. `data-residue` et `composition-residue` : mêmes comptes.
   - `make raylib` et `make screencast-viewer` compilent et linkent (vendor installé), binaires jamais lancés.
   - `tools/frames_ref.sh` garde exprès son test de `src/main.c` : il vise l'arbre des commits anciens qu'il reconstruit.
   - Gardes passées à vide par les déplacements, réparées : la ligne `info` de `composition_residue.sh --detail` (tranche 3) ; `inclusion-mutuelle`, aveugle aux includes préfixés d'un répertoire (`2b66cac`, avec test).
   - Laissés pour les étapes 3–4 : noms de fichier nus dans les commentaires de `src/` et `tests/` (81 `pico8_*.[ch]`, 19 `main*.c`), 83 chaînes `"test_pico8_X: all tests passed"`, 15 chemins `tmp/test_pico8_*`, le symbole `zone_tas`.
   - Noms des binaires d'outil inchangés (`tmp/pico8_replay`, `pico8_class_coverage`, `pico8_minimize`, `main_raylib`…). Ni le §4 ni le §6 ne les nomme : question à trancher dans le TSV de 2b, sachant que `frames_ref.sh` lance `make tmp/pico8_replay` dans un worktree d'un commit ancien.
2a. **Outil de renommage AST.** Extraction et application, prouvées de bout en bout sur un petit module, invariant compris, puis code rendu à son état d'origine : s'il ne tient pas face aux macros (`##`, `#define`), on le sait avant de produire le TSV. Sa note « Découvert » (macros gérées ou non, forme réelle des positions) est l'entrée de 2b.

   **Découvert** (PRD `docs/prd/triage/phase2-etape2a-outil-renommage.md`) :
   - Moteur : libclang 18 par ses bindings Python (`tools/ast_rename`), installés par `make rename-tool-setup` dans `third_party/clang-python`, hors `make test` ; `rename-extract`, `rename-apply TABLE=…` et `rename-tool-test` hors suite. `-ast-dump=json` n'a ni usages de types, ni désignateurs, ni macros : écarté.
   - Positions : (fichier, offset en octets, longueur), à la *spelling location* du token ; jamais de colonne. Un nom écrit dans un corps de `#define` est rendu au point d'expansion : l'outil le retrouve dans le corps (origine `macro_body`).
   - Macros : noms de `#define` et leurs usages, noms d'`#ifdef`/`#ifndef`/`defined` (`macro_cond`), corps et arguments gérés ; deux définitions d'un même nom dans un fichier (`#ifdef`/`#else`) = une identité, marquée `multidef` (5 : `POISON`, `UNPOISON`, `PICO8_ZONE_ASAN`, `PICO8_BUMP_ZONE_HALVES`, `PICO8_NORETURN`). `##` et `#` : détectés, refusés si un opérande est renommé ; seul usage, `raw##u` dans `PICO8_FIXED_LIT` (`p8/fixed.h`), aucune stringification d'identifiant. Tout token qui s'écrit comme un ancien nom sans être une occurrence connue (branche inactive, unité non analysée) fait refuser l'apply.
   - Identité par USR, stable entre unités. Piège : une fonction `extern` de même nom dans deux binaires partage une USR — `main` est une seule identité de 98 occurrences : une entrée du TSV pour un tel nom les renomme tous ensemble. Un `struct pico8_X` nommé a une identité de tag (`c:@S@…`, 12) distincte de son `typedef` (`c:@T@…`) : deux entrées.
   - Unités partielles : `cmd/class_coverage.c` et `tests/test_class_coverage.c` ne s'analysent qu'en configuration couverture (fonctions déclarées sous `PICO8_CLASS_COVERAGE`) ; couvertes par cette configuration, l'extraction reste verte. Aucune unité non analysée (vendor raylib présent).
   - Volumes (`make rename-extract`, ≈1 min 30 pour 179 unités × 3 configurations) : 16 185 identités — locales 5 461, paramètres 4 501, fonctions 3 303, champs 933, statiques 926, macros 605, types 201, constantes d'enum 151, globales 92, tags 12 ; 5 585 `short`, 11 587 `homonym`. 100 234 occurrences : `ref` 54 059, `decl` 16 524, `macro_arg` 16 265, `type_ref` 7 846, `macro_use` 2 703, `designator` 2 060, `macro_def` 610, `macro_cond` 85, `macro_body` 82.
   - Preuve sur `game/cheat` : table de 24 lignes (13 symboles déclarés par le module, garde d'inclusion comprise, 6 locales et 5 paramètres `cheat` homonymes, levés par la colonne identité), 121 occurrences dans 8 fichiers (`ref` 66, `decl` 22, `type_ref` 13, `macro_use` 13, `macro_def` 4, `macro_body` 2, `macro_cond` 1), ≈1 min 30. `make test-all` vert, `tests/e2e` intact, `raylib` et `screencast-viewer` compilent, `class-coverage-net` et `e2e-gap` rouges à l'identique avant et après. En négatif : une occurrence retirée (appel, ou nom dans le corps de `#define`) casse la compilation ; entrée ambiguë, introuvable ou en collision refusée sans rien écrire. Code rendu à l'état d'origine.
2b. **Dictionnaire complet — point d'arrêt humain.** `docs/renommage.tsv` : fichiers, types, fonctions, champs, constantes, statiques, globales, noms de tests et de fixtures, dérivés du vocabulaire et des règles du §4. Extraction exhaustive par l'outil libclang de 2a (`make rename-extract`), pas à la main : chaque déclaration de 1–2 caractères ou issue d'un digramme y figure. Les noms portés par une **chaîne** (clés d'étiquette comme `pico8_cg("jo")`, noms enregistrés par la couverture de classes, fichiers attendus de `tools/`) forment une catégorie à part du TSV : le compilateur ne les voit pas. Avant la relecture, le TSV satisfait un critère mécanique : **exhaustif** (chaque déclaration extraite a une entrée ou le statut explicite « conservé », ce qui y fait entrer les noms français, que ni l'une ni l'autre garde ne repère) et **sans collision** (aucun nom neuf en double, aucun masquage dans une même portée). La garde anti-cart est branchée en compteur. L'étape inscrit dans ce plan, comme entrées numérotées à part entière (3a, 3b…, 4a, 4b…, chacune avec son texte et ses modules), les sous-étapes des étapes 3 et 4 : `game/`, environ deux tiers du code (64 % des lignes de `src/`, 68 % avec ses tests), y est découpé en familles de taille comparable. Sa note « Découvert » n'en garde que la justification par les volumes. L'humain relit un échantillon ; un nom corrigé dans l'échantillon fait revoir ceux qui en dérivent.

   **Découvert** (PRD `docs/prd/triage/phase2-etape2b-dictionnaire.md`) :
   - Compteur anti-cart : `make cart-names-count [MODULES="game/planet*"]` (`tools/rename_count.py`), jamais en échec. Il range chaque mention sous le module du **fichier** où elle est, pas du module qui déclare le nom : une sous-étape 3 n'atteint donc zéro que si les modules dont ses fichiers utilisent les noms sont déjà passés (ordre du bas vers le haut). Quatre colonnes : `identifier` (code C), `backtick` (commentaire ou message d'`assert`), `string` (littéral C, texte d'un script, d'une donnée de `tools/`, du `Makefile`), et `comment`, mention nue d'un nom dans un commentaire, en plus des trois du §5 — elle relève de l'étape 4. Nom distinctif (≥ 3 caractères, pas un mot en minuscules, ni conservé ni nom neuf ailleurs) : recherche en mot entier. Nom ambigu : identifiant par les occurrences AST de ses identités renommées (`make rename-extract` d'abord, sinon un avertissement) ; backtick seulement si `` `p` `` est exactement le nom ; chaîne seulement si le littéral est exactement le nom, et aux seuls sites que le TSV liste.
   - Recoupement : en C, recherche texte des noms distinctifs + AST des ambigus = 47 133, exactement le nombre d'occurrences AST des 4 737 identités renommées.
   - État de départ (HEAD de l'étape, 189 modules non nuls) :

     | Répertoire | Modules | identifier | backtick | string | comment | Total |
     |---|---:|---:|---:|---:|---:|---:|
     | `src/p8` | 10 | 733 | 7 | 0 | 25 | 765 |
     | `src/engine` | 17 | 1 800 | 124 | 22 | 73 | 2 019 |
     | `src/game` | 45 | 16 086 | 1 060 | 106 | 220 | 17 472 |
     | `src/platform` | 3 | 78 | 0 | 1 | 9 | 88 |
     | `src/devtools` | 11 | 350 | 1 | 0 | 37 | 388 |
     | `src/cmd` | 7 | 217 | 0 | 38 | 9 | 264 |
     | `tests` | 85 | 27 869 | 1 790 | 880 | 113 | 30 652 |
     | `tools` | 10 | 0 | 0 | 111 | 20 | 131 |
     | `Makefile` | 1 | 0 | 0 | 45 | 10 | 55 |
     | **Total** | 189 | 47 133 | 2 982 | 1 203 | 516 | 51 834 |

     Modules les plus chargés : `game/game_data` 1 917, `tests/test_button` 1 674, `tests/test_live_slots` 1 466, `tests/test_slipway` 1 203, `tests/test_burst` 1 143, `tests/test_ui` 1 140 ; en `src/game` ensuite `planet_family` 1 054, `map_tables` 1 018, `planet_display` 697.
   - Angles morts : une mention qui n'est dans aucune configuration analysée ni lisible en texte (nom ambigu dans une branche `#if` inactive) ; un nom ambigu dans une chaîne à un site que le TSV ne liste pas, ou dans un backtick qui n'est pas exactement lui (`` `e->p` ``) ; un nom ambigu nu dans un commentaire ; les docs, `tests/e2e/`, `third_party/` et les outils du dictionnaire (`tools/rename_*`, `tools/ast_rename`, leurs tests), exclus exprès comme par `make rename-table-check`. Un backtick `` `p` `` compte même si le `p` cité est une locale conservée.
   - Dérivation (`make rename-derive`) : 16 185 identités, 4 737 renommées, 11 448 conservées, reste vide ; aucune ligne `manuel` (le seul `##`, `raw##u`, ne touche aucun nom renommé). Surcharges : 1 247 lignes dans `tools/rename_overrides.tsv` (1 225 à la dérivation, 22 pour passer le contrôle), loin de la « petite table » prévue : le vocabulaire nomme les préfixes, pas les feuilles. 529 noms du cart hors des 83 concepts (champs, fonctions, tables), 351 locales et paramètres courts à portée > 40 lignes nommés un à un, 289 mots et noms de tests français, 35 concepts que le lecteur du vocabulaire n'apparie pas, 21 masquages levés, 20 mots anglais de deux lettres conservés, 2 binaires.
   - Compléments au vocabulaire, aucun nom contredit : méthodes d'état `X_update`, `X_update_settled`, `X_update_placing`, `X_update_unfolding`, `X_update_vanishing` (les états restent `ENTITY_STATE_*` et `"normal"`… ; cf. doute de `ph`) ; cycle de vie `pico8_X_new` → `X_new`, `pico8_X_struct_new` → `X_create`, `pico8_X_struct_init` → `X_init_instance` (le vocabulaire ne nomme que `pq` → `X_construct`).
   - Chaînes et fichiers (`make rename-strings`) : 278 noms portés par une chaîne, 811 sites (75 noms de test, 74 symboles cités, 69 clés de dump et de libellé, 45 clés d'étiquette, de message, d'état et noms de classe, 15 chemins `tmp/`) ; 10 noms de fichier, 31 sites (8 binaires, 2 sorties). 14 clés signalées dans `tmp/ast_rename/string_flags.tsv` : 13 sans écrivain trouvé (`dm`, `fj`, `go`, `hm`, `il`, `ja`, `je`, `jn`, `ju`, `lr`, `mo`, `ok`, `px`), 1 sans lecteur (`pz`).
   - Binaires : le nom de leur fichier `cmd/`, sauf `tmp/pico8_frames` → `tmp/frames_tool`, `tmp/frames` étant déjà le dossier des captures e2e (`FRAMES_TOOL` dans le `Makefile`). `tools/frames_ref.sh` : voie (a), il garde `tmp/pico8_replay` pour un commit sans la règle `tmp/replay`, comme il garde son test de `src/main.c` ; la voie (b) laissait un nom du cart au binaire le plus lancé.
   - Classe `trade` : le type devient `trade_route_t` (vocabulaire), mais le nom de classe `"trade"`, déjà anglais, est conservé (comme `"ui"`) : la couverture de classes dira `trade` pour un `trade_route_t`.
   - USR partagée : une fonction `extern` homonyme dans plusieurs binaires (`main`, les doubles de test) est une identité, donc une ligne ; le contrôle de collision le sait.
   - 5 avertissements `p8_` hors `src/p8/` : `pico8_input_load/save/append/free` définis dans `devtools/input_json.c`, `pico8_fatal` dans `platform/fatal_pc.c`, déclarés dans `p8/`. C'est la scission d'en-tête notée par le vocabulaire (§7), non traitée : ces lignes suivent leur module de définition (3k, 3l).
   - Libellés hachés : 14 sites de tests plient une clé chaîne dans un hash figé (`fnv_str(h, "lh")`) ; les renommer à l'étape 3 change ces constantes, à recalculer dans le même commit.
   - Découpage des étapes 3 et 4 (§6, 3a–3l et 4a–4l) : 12 sous-étapes, 93 modules de `src/`, chacun dans une seule. Lignes pondérées = lignes `src/` + lignes des tests qui suivent + lignes `renommé` du TSV des modules ; la cible ≤ ×2 porte sur les familles de `game/` (3d–3j) : 8 034 / 5 216 = ×1,54. Les mentions du compteur y font ×2,20 (3i : dix fichiers de tests de structures pour un module). `p8/` compte 2 059 lignes générées (`quarter_wave.h`) ; 3a, petite en lignes, réécrit 4 134 occurrences (champs de l'en-tête partout) plus toutes les clés chaîne.

     | Sous-étape | Modules | `src/` | tests | TSV | Pondérées | Mentions | Occurrences AST | Résidu |
     |---|---:|---:|---:|---:|---:|---:|---:|---:|
     | 3a en-tête, vtable | 1 | 196 | 545 | 148 | 889 | 1 029 | 4 134 | 434 |
     | 3b `p8/` | 10 | 3 474 | 2 413 | 198 | 6 085 | 1 969 | 5 326 | 100 |
     | 3c `engine/` | 16 | 2 210 | 3 011 | 509 | 5 730 | 3 686 | 5 778 | 314 |
     | 3d données, mise en page | 8 | 2 932 | 1 938 | 914 | 5 784 | 5 716 | 6 459 | 362 |
     | 3e planète, slipway, commerce | 7 | 3 053 | 3 018 | 627 | 6 698 | 6 510 | 5 600 | 372 |
     | 3f sonde, trou de ver, particules | 6 | 1 571 | 3 776 | 709 | 6 056 | 5 672 | 4 244 | 211 |
     | 3g économie, recherche | 9 | 2 637 | 2 779 | 492 | 5 908 | 4 839 | 2 970 | 106 |
     | 3h pointeur, boutons, fond, caméra | 7 | 1 910 | 5 430 | 694 | 8 034 | 6 376 | 4 335 | 126 |
     | 3i structures | 1 | 848 | 6 473 | 547 | 7 868 | 9 944 | 4 678 | 50 |
     | 3j boîtes, panneaux, menus, cart | 7 | 2 434 | 2 322 | 460 | 5 216 | 4 513 | 2 529 | 4 |
     | 3k `devtools/` | 11 | 1 571 | 2 534 | 105 | 4 210 | 966 | 967 | 0 |
     | 3l `platform/`, `cmd/`, outils | 10 | 1 069 | 2 622 | 168 | 3 859 | 614 | 113 | 0 |
     | **Total** | 93 | 23 905 | 36 861 | 5 571 | 66 337 | 51 834 | 47 133 | |

     Mentions : état de départ du compteur sur les fichiers de la sous-étape. Occurrences AST : ce que `rename-apply` réécrit pour ses identités, où qu'elles soient. `game/` fait 64 % des lignes de `src/`, 68 % avec ses tests.
   - Ordre : 34 modules d'`engine/` et de `game/` se citent en cycle (`engine/bootstrap` crée `ui`, la caméra et la souris ; `entity_class.h` nomme des types du jeu) : aucun ordre ne vide le compteur d'une sous-étape avant la dernière. L'ordre retenu minimise le résidu (colonne Résidu : occurrences, dans les fichiers de la sous-étape, de noms des sous-étapes suivantes) ; le critère de fin de 3a–3k en tient compte (§6, sous-étapes).

   **Échantillon à relire** (le TSV est vert au contrôle ; ne relire que le sens). Pour corriger, ne jamais toucher le TSV : corriger `docs/vocabulaire.md` (un concept, propagé à tous ses dérivés) ou `tools/rename_overrides.tsv` (un nom, un champ `parent.nom`, une identité), puis `make rename-derive rename-strings rename-table-check`. Un nom corrigé fait revoir ceux qui en dérivent : ses lignes ont son concept dans la colonne `concept` du TSV. Puis passer l'en-tête du TSV à `Statut : validé`.

   *Sens devinés et écarts forcés* — en tête, aucun ne vient du vocabulaire :

   | ancien | nouveau | où | à juger |
   |---|---|---|---|
   | `nh` | `fade_in_step` | champ, `game/game_data` | sens deviné |
   | `en`, `dk_en` | `type_bias`, `difficulty_type_bias` | `game/game_data`, `game/galaxy` | sens deviné |
   | `pico8_bn`, `pico8_bq` | `game_data_route_cost_factor`, `game_data_route_length_factor` | `game/game_data` | sens deviné |
   | `pico8_da` | `planet_upkeep` | `game/economy_score` | sens deviné |
   | `pico8_es/ct/co/b__table` | `upkeep_by_tier_table`, `partner_bonus_by_tier_table`, `happiness_by_tier_table`, `development_score_by_tier_table` | `game/game_data` | sens deviné |
   | `PICO8_TAG_NP`, `"np"` | `TAG_NODE`, `"node"` | `engine/entity_class` | hors vocabulaire |
   | `PICO8_TAG_PROBER`, `"prober"` | `TAG_TARGETING`, `"targeting"` | `engine/entity_class` | doute du vocabulaire tranché ainsi |
   | `pico8_jo_set_ir`, `g_jo_ir` | `planet_set_vanish_roll`, `g_planet_vanish_roll` | `game/planet` | sens deviné |
   | `oc` ; `pico8_jo_set_oc`, `g_jo_oc` | `tickets` ; `planet_set_type_picker`, `g_type_picker` | `game/gen` ; `game/planet` | un digramme, deux noms |
   | `pico8_ie_gy`, `pico8_ie_eh`, `pico8_ie_id` | `structure_icon_cursor`, `structure_icon_rest_cursor`, `structure_icon_sprite_counter` | `game/structure_icon` | les champs `gy`, `eh` deviennent `revealed_count`, `rest_count` |
   | `go`, `with_go` | `next_type`, `with_next_type` | `game/map_tables`, test | sens deviné ; clé `"go"` sans écrivain |
   | `ii`, `has_ii` | `icon_sprite`, `has_icon_sprite` | `game/map_tables` | sens deviné |
   | `dm`, `has_dm` | `byproduct`, `has_byproduct` | `game/map_tables`, `game/research` | sens deviné |
   | `cr` de `pj`, `pico8_pj_cr_t` | `layout_block`, `score_panel_layout_block_t` | `game/score_panel` | second sens de `cr` |
   | `pico8_mo_qb`, `pico8_fm_hy`, `pico8_je_nf` | `main_camera_typed`, `main_economy_typed`, `main_mouse_typed` | `game/scroll`, `game/probe`, `game/ui` | leurs doubles de test disent `typed_main_camera`… |
   | `pico8_cr` | `menu_show_tech_tree_screen` | `game/menu` | module `menu` imposé par le §4 |
   | `pico8_msgbox_set_kc` | `message_box_set_shown` | `game/message_box` | vocabulaire : `shown_message_box` |
   | `_cr_t` ; `h` de `pico8_cr_row_t` | `technology` ; `cost_keys` | `game/ui_button` ; `game/menu` | rattachés au concept `cr` |
   | `tmp/pico8_frames` | `tmp/frames_tool` | binaire | `tmp/frames` est pris |
   | `PICO8_STATE_COUNT` | `STATE_COUNT` | `engine/entity_class` | sans `ENTITY_`, à côté de `ENTITY_STATE_*` |
   | `pico8_fv_struct_new`, `pico8_lq_struct_new` | `screen_create`, `row_create` | `game/help`, `game/layout` | `static` : le nom du module tombe avec le préfixe |

   *Doutes du vocabulaire, appliqués tels quels* :

   | concept | nom appliqué (exemple) | doute |
   |---|---|---|
   | `ke` | `pico8_ke_init` → `entity_init` ; registres `pico8_entity_init` → `entity_registry_init`, mais `pico8_lt` → `entity_register` | deux préfixes pour le noyau |
   | `pq` | champ `pq` → `construct` ; `pico8_trade_struct_new` → `trade_create` | `X_init` déjà pris |
   | `pico8_entity_kind_t` | `entity_kind_t` | voisin de `slot_kind_t` |
   | `cv`, `cg`, `hv` | `entities_by_message`, `entities_by_tag`, `live_entities` | clés chaîne de la vtable |
   | `dc` | `draw_layers` | ne route que le dessin |
   | `_dispatch` | `pico8_g__rendre_dispatch` → `probe_release_method` ; `pico8_ha3_my_dispatch` → `pause_menu_row3_on_click_method` | suffixe `_method` |
   | `ph`, `ly` | `ENTITY_STATE_NORMAL`, `ENTITY_STATE_SETTLED` ; `pico8_jo_ll` → `planet_update_vanishing` | état contre méthode |
   | `sorte`, `place` | `slot_kind_t`, `slot` ; `pico8_zone_sorte_t` → `slot_kind_zone_t` | ordre des mots |
   | `jo`, `qh` | `planet_t` ; champ `qh` → `is_structure` | `body`, `laboratory` |
   | `fr` | `infraspace_gate_new` | « infraspace » nomme aussi la techno |
   | `px` | `name` partout | pas de `display_name` |
   | `hf`, `oe` | `is_planet`, `slipways` | « hub » du glossaire |
   | `ie` | `structure_icon_t` | |
   | `g_`, `pico8_probe_t`, membre `probe` | `probe_t` ; `targeting_t` ; `targeting` | conflit levé ainsi |
   | `mz` | `wormhole_t` | `space_fold` |
   | `ui` | `ui_t`, `"ui"` conservé | garde de portée |
   | `ok`, `mo`, `je`, `fm`, `ju` | `main_ui`… ; `pico8_mo_placeholder_t` → `main_camera_placeholder_t` | préfixe `main_` |
   | `pico8_gfx_t`, `pico8_mem_t` | `p8_graphics_t`, `p8_memory_t` | `p8_gfx_t` ? |
   | `pico8_vec_t` | `vec_t` ; `pico8_vec_qm` → `vec_from_polar` | jeu ou runtime |
   | `pico8_struct_header_t`, `cls` | `entity_t`, `class` | mot réservé C++ ; aucun C++ compilé |
   | `ra`, `p` | `pos`, `map_pos` | l'exemple du §4 |
   | `fg`, `mt`, `ma`, `fx`, `p_` | `gpio_signal`, `infraspace`, `shake`, `fold_anchor`, `is_dead` | sens déduits du code |

   *Doubles sens* :

   | nom | sens 1 | sens 2 |
   |---|---|---|
   | `hp` | classe `pico8_hp_t` → `research_project_t` ; champ de `research.c` → `project` | champ de `pico8_jo_def_t` → `project_index` |
   | `kf` | vtable → `attach_slipway` | `pico8_jo_trade_proposal` → `accept` |
   | `nk` | champ de planète → `planet_type` | entrée de `map_tables` → `icon` (`nk_key` → `icon_key`) |
   | `cr` | `pico8_cr_row_t` → `tech_tree_screen_row_t` | `pj.cr` → `layout_block` |
   | `p` | `pico8_jo_keys_t.p` → `map_pos` ; `pico8_qy.p` → `pos` | caméra `pico8_qb_def_t.p` → `pos_before_shake` ; `pico8_zone_cell_t.p` → `align_pointer` |
   | `px` | rangée affichée → `name` | clé de nom → `name` |

   *Masquages levés au contrôle (21 surcharges)* : locales `hv` → `live` (4 tests) ; `partie` → `game_zone_used_before` (3 tests) ; `fm` → `economy` (`menu.c`, `score_panel.c`, `score_panel_display.c`) ; `je` → `mouse` (`ui.c` ×2) ; `ok` → `ui` (`message_box.c`, `test_ui.c`) ; `mo` → `camera` ; `ju` → `score_panel` ; `lq_t` → `row_keys` ; `need_qe` → `need_amount` ; `b` → `built` ; `g` → `built_route` ; `places` → `neighbour_allocator`.

   *Noms français renommés (289 surcharges ; exemples)* : `pico8_cart_vider_file_sortie` → `cart_flush_exit_queue` ; `pico8_entity_abandonner_hv` → `entity_registry_abandon_live_entities` ; `pico8_entity_oublier_registres` → `entity_registry_forget_registries` ; `chargement` → `load_zone_storage` ; `octets_des_planetes` → `planets_bytes` ; `pause_restart_ne_touche_pas_au_chargement` → `pause_restart_leaves_load_zone_alone` ; `devoilees` → `revealed` ; `case_qy_rendre_releases_the_arena` → `case_particles_release_releases_the_arena` (répétition).

   *Chaînes et fichiers (toutes ; une ligne par famille)* :
   - Étiquettes et classes : `g_`→`probe`, `hj`→`ascension`, `jc`→`slipway`, `jo`→`planet`, `mh`→`message_box`, `mz`→`wormhole`, `nr`→`slipgate`, `qh`→`structure`, `np`→`node`, `prober`→`targeting` ; `bg`→`background`, `fq`→`modal_backdrop`, `fr`→`infraspace_gate`, `fv`→`help_screen`, `ga`→`burst`, `hp`→`research_project`, `hy`→`economy`, `ie`→`structure_icon`, `jb`→`tooltip`, `js`→`button`, `lq`→`layout_row`, `mx`→`synthesizer`, `nf`→`mouse`, `nl`→`trading_hub`, `os`→`processor`, `pj`→`score_panel`, `pm`→`protostar`, `qb`→`camera`, `iv`→`toast`, `ke`→`entity` ; `trade`, `ui` conservés.
   - Messages : `kj`→`draw`, `ev`→`draw_overlay`, `hd`→`hit_test`, `hk`→`new_year`, `hb`→`refresh`, `ey`→`drop`, `hq`→`drag`, `ldown`→`left_press`. États : `ph`→`normal`, `ly`→`settled`, `kz`→`placing`, `pr`→`unfolding`, `ll`→`vanishing`.
   - Clés de dump et de libellé (69) : `le`→`ephemerals`, `qy`→`particles`, `qy_fv`→`particles_help_screen`, `ts`→`transit_dots`, `ld`→`draw_row`, `hm`→`released`, `jn`→`pressed`, `ok`/`mo`/`je`/`ju`→`main_ui`/`main_camera`/`main_mouse`/`main_score_panel` ; libellés de tests `ca`→`back_color`, `ep`→`description`, `fe`→`projects_panel`, `gh`→`inspection_panel`, `fs`→`placed_items`, `gq`→`row_step`, `hi`→`fixed_pos`, `lh`→`keep_open`, `eb`→`other_end`, `kt`→`invent`, `lu`→`spend`, `nx`→`advance_clock`, `lj`→`recompute`, `ox`→`anchors`, `il`→`upgrade_tech`, `ja`→`production_key`, `lr`→`planet_codes`, `fj`→`base_cost`, `pz`→`open_box` ; les autres sont des noms de cas composés des mêmes mots (`dk_outlives_its_button` → `difficulty_outlives_its_button`).
   - Noms de test (75) : `test_pico8_X` → `test_` + nom du fichier de test (`test_pico8_g_` → `test_probe`, `test_pico8_bg_kj` → `test_background_draw`, `test_pico8_ga_export_panel_by` → `test_burst_exports_row`). Chemins `tmp/` (15) : même règle (`tmp/test_pico8_zone_choisie_stderr.txt` → `tmp/test_allocator_stderr.txt`). Symboles cités (74) : le nouveau nom de l'identifiant cité.
   - Binaires : `tmp/pico8_replay` → `tmp/replay`, `pico8_replay_gcov` → `replay_gcov`, `pico8_replay_sanitize` → `replay_sanitize`, `pico8_gif` → `gif`, `pico8_minimize` → `minimize`, `pico8_class_coverage` → `class_coverage`, `main_raylib` → `game`, `pico8_frames` → `frames_tool` ; sorties `tmp/pico8_replay_dump.txt` → `tmp/replay_dump.txt`, `tmp/pico8_demo_dump.txt` → `tmp/demo_dump.txt`.

   *Tirage* (Python `random.Random(2026).sample`, 2 lignes `identifiant renommé` par sous-étape, dans l'ordre du TSV) :

   | sous-étape | ancien | nouveau | sorte | module |
   |---|---|---|---|---|
   | 3a | `kj` | `draw` | champ | `engine/entity_class` |
   | 3a | `PICO8_STATE_COUNT` | `STATE_COUNT` | constante | `engine/entity_class` |
   | 3b | `PICO8_FONT_ROWS_PLUS` | `FONT_ROWS_PLUS` | statique | `p8/gfx` |
   | 3b | `PICO8_FONT_ROWS_R` | `FONT_ROWS_R` | statique | `p8/gfx` |
   | 3c | `g_camera_y_at_kj` | `g_camera_y_at_draw` | statique | `tests/test_draw_layers_camera` |
   | 3c | `bascule` | `flip_zone` | statique | `tests/test_zone` |
   | 3d | `k_` | `trail_colors` | champ | `game/game_data` |
   | 3d | `pico8_bz_table` | `score_rows_table` | globale | `game/game_data` |
   | 3e | `test_fr_defaults` | `test_infraspace_gate_defaults` | fonction | `tests/test_planet_spec_defaults` |
   | 3e | `has_qe` | `has_resource` | paramètre | `tests/test_trade` |
   | 3f | `et_fo` | `target_targeted_by` | fonction | `tests/test_probe` |
   | 3f | `loin` | `far` | statique | `tests/test_live_slots` |
   | 3g | `pt` | `entity` | paramètre | `game/economy_tiers` |
   | 3g | `bx_visit` | `total_production_visit` | fonction | `game/economy` |
   | 3h | `g_qb_struct_class` | `g_camera_class` | statique | `game/scroll` |
   | 3h | `pico8_ui_le_new` | `ui_ephemerals_new` | fonction | `game/ui` |
   | 3i | `c` | `third` | locale | `tests/test_burst` |
   | 3i | `jo` | `planet` | locale | `tests/test_trading_hub` |
   | 3j | `pico8_ha3_my_dispatch` | `pause_menu_row3_on_click_method` | fonction | `game/menu` |
   | 3j | `fm` | `economy` | locale | `game/score_panel` |
   | 3k | `pico8_class_coverage_first_unknown` | `class_coverage_first_unknown` | fonction | `devtools/class_coverage` |
   | 3k | `PICO8_SCREENCAST_FRAME_BYTES` | `SCREENCAST_FRAME_BYTES` | macro | `devtools/screencast` |
   | 3l | `test_ptrs_entite_reads_entity_members_like_registres` | `test_pointer_list_entity_reads_entity_members_like_registries` | fonction | `tests/test_composition_residue` |
   | 3l | `PICO8_RAYLIB_BACKEND_H` | `PLATFORM_RAYLIB_BACKEND_H` | macro | `platform/raylib_backend` |
3. **Application** — une PRD par sous-étape : 3a en-tête commun et vtable, puis les sous-étapes inscrites à l'étape 2b. Renommage par token à partir des positions exactes données par l'AST clang ; pas de `sed` naïf (un champ `p` ou `t` n'est pas cherchable en texte). Le compilateur arbitre : une occurrence oubliée casse le build. Les noms portés par une chaîne n'ont pas ce filet : leur filet est les tests et les e2e, et chaque sous-étape les renomme des deux côtés à la fois. Les tests, fixtures et données de `tools/` suivent dans la sous-étape de leur module. Fin d'une sous-étape : compteur anti-cart à zéro pour ses modules, catégories identifiants et chaînes (les mentions entre backticks des commentaires relèvent de l'étape 4) ; la garde de portée est branchée en compteur dès 3a. Chaque sous-étape a sa note « Découvert » ; ce qu'elle constate (défaut de l'outil, entrée du TSV corrigée) est reporté dans l'outil ou le TSV dans le même commit, avant la sous-étape suivante, et une entrée corrigée fait revoir les noms qui en dérivent : le TSV reste ce qui a vraiment été appliqué.

   **Sous-étapes** (inscrites par 2b ; volumes dans sa note « Découvert »). Chacune applique `make rename-slice MODULES="<ses modules et ses tests>"` puis `make rename-apply TABLE=tmp/ast_rename/slice.tsv` : l'outil réécrit les occurrences de leurs identités dans tout le dépôt. Chaînes : 3a renomme toutes les clés d'étiquette, de message et d'état et tous les noms de classe, à tous leurs sites ; toute autre ligne `chaîne` (symbole cité, nom de test, chemin `tmp/`, clé) est renommée à tous ses sites par la sous-étape qui tient le module de son site `écrivain` (la première s'il y en a plusieurs, celle du lecteur s'il n'y en a pas) ; les lignes `fichier` par 3l. Un test suit le module qu'il teste (listes ci-dessous) ; aucune fixture de `tests/fixtures/` ne porte d'ancien nom ; une mention d'un script ou d'une donnée de `tools/` suit le nom qu'elle cite. Des modules d'`engine/` et de `game/` se citent en cycle : à la fin de 3a–3k, `make cart-names-count MODULES="<ses modules et ses tests>"` ne garde en `identifier` que des noms de sous-étapes suivantes, au plus le résidu indiqué (mesuré à la fin de 2b, à remesurer si le TSV validé change), et en `string` que des clés de sous-étapes suivantes ; à la fin de 3l, compteur à zéro partout en `identifier` et `string`. Les modules sont ceux de la colonne `module` du TSV (`src/` sans `src/`).
3a. **En-tête commun et vtable.** `engine/entity_class` : `entity_t`, `entity_class_t`, membres de la vtable, états, étiquettes, `entity_kind_t`. Toutes les clés chaîne d'étiquette, de message (`entities_by_message`) et d'état, et les noms de classe, à tous leurs sites, `tools/class_coverage_net_expected.txt` et `tools/class_coverage_exempt.txt` compris. Branche la garde de portée en compteur. Tests : `test_entity_header`, `test_entity_kind`. Résidu ≤ 434.
3b. **Runtime PICO-8.** `p8/` : `fixed`, `quarter_wave`, `mem`, `gfx`, `palette`, `str`, `input`, `audio`, `assets`, `fatal` ; `tools/gen_quarter_wave.c` et `tools/constants_compare.sh` suivent. `pico8_input_load/save/append/free` et `pico8_fatal`, définis hors de `p8/`, passent en 3k et 3l. Tests : `test_fixed`, `test_mem`, `test_gfx`, `test_demo`, `test_str`, `test_input`, `test_asset_loading`. Résidu ≤ 100.
3c. **Socle.** `engine/`, sauf l'en-tête de 3a : `allocator`, `zone`, `zone_sizes`, `frame_zone`, `game_zone`, `pointer_list`, `entity_list`, `exit_queue`, `collect`, `vec`, `geom`, `entity`, `entity_registry`, `entity_geom`, `dispatch_context`, `bootstrap`. Tests : `heap_allocator.[ch]`, `test_allocator`, `test_arena`, `test_zone`, `test_frame_zone`, `test_game_zone`, `test_pointer_list`, `test_entity_list`, `test_exit_queue`, `test_vec`, `test_geom`, `test_draw_layers_camera`. Résidu ≤ 314.
3d. **Données et mise en page.** `game/` : `game_data`, `map_tables`, `data_init`, `gen`, `galaxy`, `draw_functions`, `layout`, `hud`. Tests : `test_map_tables`, `test_data_init`, `test_spec_defaults`, `test_layout_row`, `test_layout_nil_item_draws_nothing`, `test_tooltip_draw_at`. Résidu ≤ 362.
3e. **Planète, slipway, commerce.** `game/` : `planet`, `planet_display`, `planet_trade`, `planet_family_trade`, `trade`, `slipway`, `slipway_display`. Tests : `test_planet`, `test_planet_spec_defaults`, `test_planet_exports_row`, `test_slipway`, `test_trade`. Résidu ≤ 372.
3f. **Sonde, trou de ver, particules.** `game/` : `probe`, `targeting`, `wormhole`, `wormhole_display`, `particle`, `particle_zone`. Tests : `test_probe`, `test_probe_spawn_particle_order`, `test_wormhole`, `test_particle_zone`, `test_live_slots`. Résidu ≤ 211.
3g. **Économie et recherche.** `game/` : `economy`, `economy_formulas`, `economy_score`, `economy_tiers`, `main_economy`, `research`, `research_display`, `tech_gate`, `structure_icon`. Tests : `test_economy`, `test_main_economy`, `test_research_project`, `test_structure_icon`. Résidu ≤ 106.
3h. **Pointeur, boutons, fond, caméra.** `game/` : `ui`, `ui_button`, `ui_zone`, `mouse`, `mouse_button`, `background`, `scroll`. Tests : `test_ui`, `test_button`, `test_modal_backdrop`, `test_ui_zone`, `test_mouse`, `test_main_mouse`, `test_background`, `test_background_draw`, `test_camera`, `test_main_camera`. Résidu ≤ 126.
3i. **Structures.** `game/planet_family`. Tests : `test_structure`, `test_processor`, `test_slipgate`, `test_infraspace_gate`, `test_trading_hub`, `test_synthesizer`, `test_ascension`, `test_protostar`, `test_burst`, `test_burst_exports_row`. Résidu ≤ 50.
3j. **Boîtes, panneaux, menus, cart.** `game/` : `message_box`, `score_panel`, `score_panel_display`, `menu`, `help`, `cart`, `cheat`. Tests : `test_message_box`, `test_toast`, `test_score_panel`, `test_help_screen`. Résidu ≤ 4.
3k. **Outils sans fenêtre.** `devtools/` : `dump`, `gif`, `screencast`, `screencast_player`, `replay_control`, `replay_source`, `session_recorder`, `input_json`, `class_coverage`, `minimize`, `fixed_test_double`. Tests : `test_dump`, `test_gif`, `test_screencast`, `test_headless_capture`, `test_screencast_player`, `test_replay_control`, `test_replay_source`, `test_marker_coherence`, `test_recording_continuity`, `test_class_coverage`, `test_minimize`. Résidu 0.
3l. **Plateforme, binaires, outils.** `platform/` : `raylib_backend`, `raylib_backend_coords`, `fatal_pc` ; `cmd/` : `game`, `replay`, `frames`, `gif`, `minimize`, `screencast_viewer`, `class_coverage`. Binaires et sorties de `tmp/` (lignes `fichier`) dans le `Makefile`, les tests et `tools/` (`frames_ref.sh` par la voie (a), `float_grep_exclusions.txt`) ; `tools/composition_residue.sh` et `composition_residue_sites.txt`, `tools/minimize.sh`, `tools/pico8_reference_probe.p8` y ferment leurs dernières mentions. Tests : `test_raylib_backend_coords`, `test_frames_tool`, `test_replay_from_title`, `test_replay_usage`, `test_line_coverage`, et ceux des gardes de `tools/` : `test_alloc_guard`, `test_composition_residue`, `test_constants_check`, `test_data_residue`, `test_e2e_cleanup`, `test_e2e_limits`, `test_float_grep`, `test_float_guard`, `test_float_link`, `test_frames_check`. Fin : compteur à zéro partout, `identifier` et `string`.
4. **Commentaires, asserts, locales** — une PRD par sous-étape, sur la même liste qu'à l'étape 3. La note « Découvert » de la première sous-étape corrige la consigne des agents avant les suivantes. Un agent par module : commentaires et messages d'`assert` réécrits en anglais sans citer le cart (§4), mots français restants retirés, locales cryptiques à longue portée renommées. Le skill `reprendre-les-commentaires` cadre le travail. Un troisième compteur, de **français résiduel** (commentaires, chaînes, asserts), est branché au début de l'étape. Fin d'une sous-étape : les trois compteurs à zéro sur ses modules, toutes catégories, mentions entre backticks comprises.

   **Sous-étapes** : les modules, tests et données de `tools/` de la sous-étape 3 de même lettre ; l'étape 3 finie, aucun résidu d'une sous-étape suivante ne reste.
4a. **Commentaires — en-tête commun et vtable.** Modules et tests de 3a.
4b. **Commentaires — runtime PICO-8.** Modules, tests et données de `tools/` de 3b.
4c. **Commentaires — socle.** Modules et tests de 3c.
4d. **Commentaires — données et mise en page.** Modules et tests de 3d.
4e. **Commentaires — planète, slipway, commerce.** Modules et tests de 3e.
4f. **Commentaires — sonde, trou de ver, particules.** Modules et tests de 3f.
4g. **Commentaires — économie et recherche.** Modules et tests de 3g.
4h. **Commentaires — pointeur, boutons, fond, caméra.** Modules et tests de 3h.
4i. **Commentaires — structures.** Module et tests de 3i.
4j. **Commentaires — boîtes, panneaux, menus, cart.** Modules et tests de 3j.
4k. **Commentaires — outils sans fenêtre.** Modules et tests de 3k.
4l. **Commentaires — plateforme, binaires, outils.** Modules, tests, scripts et données de `tools/` de 3l, et le `Makefile`.
5. **Clôture.**
   - `TROU-N` → `GAP-N` dans le code et comme clé dans `docs/trous-de-portage.md`, qui reste en français mais décrit chaque écart avec les nouveaux noms C ; le nom Lua n'apparaît qu'en tête de chaque GAP.
   - Les deux gardes du §5 dans `make test`, vertes ; l'étape tranche si le compteur de français devient une troisième garde permanente.
   - `CLAUDE.md` : nouvelle arborescence, renvoi `TROU-7` → `GAP-7`.
   - `docs/glossaire-noms.md` supprimé ; `README`, les autres docs et les skills qui citent des symboles ou des chemins (dont `reprendre-les-commentaires`) mis à jour vers les nouveaux noms.

Chaque étape se termine en ajoutant sous sa propre entrée une courte note « Découvert » : les noms qui ont résisté aux règles du §4, les fichiers rangés autrement que dans le §3, les exceptions de garde proposées. Si un constat contredit le §3 ou le §4, l'étape corrige le plan dans le même commit.

## 7. Hors périmètre

- Changer le comportement du jeu, découper ou fusionner des modules au-delà des déplacements du §3. Un renommage qui révèle un module mal découpé est noté dans « Découvert », pas traité.
- Traduire en anglais les docs, le `CLAUDE.md`, les skills, les messages de commit.
- Réécrire l'historique git.
