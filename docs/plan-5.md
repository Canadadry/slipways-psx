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
2b. **Dictionnaire complet — point d'arrêt humain.** `docs/renommage.tsv` : fichiers, types, fonctions, champs, constantes, statiques, globales, noms de tests et de fixtures, dérivés du vocabulaire et des règles du §4. Extraction exhaustive par l'AST clang (`-Xclang -ast-dump=json`), pas à la main : chaque déclaration de 1–2 caractères ou issue d'un digramme y figure. Les noms portés par une **chaîne** (clés d'étiquette comme `pico8_cg("jo")`, noms enregistrés par la couverture de classes, fichiers attendus de `tools/`) forment une catégorie à part du TSV : le compilateur ne les voit pas. Avant la relecture, le TSV satisfait un critère mécanique : **exhaustif** (chaque déclaration extraite a une entrée ou le statut explicite « conservé », ce qui y fait entrer les noms français, que ni l'une ni l'autre garde ne repère) et **sans collision** (aucun nom neuf en double, aucun masquage dans une même portée). La garde anti-cart est branchée en compteur. L'étape inscrit dans ce plan, comme entrées numérotées à part entière (3a, 3b…, 4a, 4b…, chacune avec son texte et ses modules), les sous-étapes des étapes 3 et 4 : `game/`, environ 70 % du code, y est découpé en familles de taille comparable. Sa note « Découvert » n'en garde que la justification par les volumes. L'humain relit un échantillon ; un nom corrigé dans l'échantillon fait revoir ceux qui en dérivent.
3. **Application** — une PRD par sous-étape : 3a en-tête commun et vtable, puis les sous-étapes inscrites à l'étape 2b. Renommage par token à partir des positions exactes données par l'AST clang ; pas de `sed` naïf (un champ `p` ou `t` n'est pas cherchable en texte). Le compilateur arbitre : une occurrence oubliée casse le build. Les noms portés par une chaîne n'ont pas ce filet : leur filet est les tests et les e2e, et chaque sous-étape les renomme des deux côtés à la fois. Les tests, fixtures et données de `tools/` suivent dans la sous-étape de leur module. Fin d'une sous-étape : compteur anti-cart à zéro pour ses modules, catégories identifiants et chaînes (les mentions entre backticks des commentaires relèvent de l'étape 4) ; la garde de portée est branchée en compteur dès 3a. Chaque sous-étape a sa note « Découvert » ; ce qu'elle constate (défaut de l'outil, entrée du TSV corrigée) est reporté dans l'outil ou le TSV dans le même commit, avant la sous-étape suivante, et une entrée corrigée fait revoir les noms qui en dérivent : le TSV reste ce qui a vraiment été appliqué.
4. **Commentaires, asserts, locales** — une PRD par sous-étape, sur la même liste qu'à l'étape 3. La note « Découvert » de la première sous-étape corrige la consigne des agents avant les suivantes. Un agent par module : commentaires et messages d'`assert` réécrits en anglais sans citer le cart (§4), mots français restants retirés, locales cryptiques à longue portée renommées. Le skill `reprendre-les-commentaires` cadre le travail. Un troisième compteur, de **français résiduel** (commentaires, chaînes, asserts), est branché au début de l'étape. Fin d'une sous-étape : les trois compteurs à zéro sur ses modules, toutes catégories, mentions entre backticks comprises.
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
