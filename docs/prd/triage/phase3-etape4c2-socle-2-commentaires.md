---
title: "Plan 5, morceau 4c2 — commentaires, chaînes et locales du socle objet (zones frame et partie, listes, registre, géométrie, amorçage)"
description: "Ramener à zéro, sur les vingt modules du morceau 4c2, le français résiduel, les noms du cart (backtick et commentaire), les citations du cart et les locales cryptiques, en traduisant les noms de zone « partie » et « chargement » à leur seul lecteur, sans changer un pixel, et clore la sous-étape 4c."
status: needs-triage
---

## Texte de l'étape (copié de `docs/plan-5.md`, §6)

> 4. **Commentaires, asserts, locales** — une PRD par sous-étape, sur la même liste qu'à l'étape 3. La note « Découvert » de la première sous-étape corrige la consigne des agents avant les suivantes. Un agent par module : commentaires et messages d'`assert` réécrits en anglais sans citer le cart (§4), mots français restants retirés, locales cryptiques à longue portée renommées. Le skill `reprendre-les-commentaires` cadre le travail. Un troisième compteur, de **français résiduel** (commentaires, chaînes, asserts), est branché au début de l'étape. Fin d'une sous-étape : les trois compteurs à zéro sur ses modules, toutes catégories, mentions entre backticks comprises.
>
>    **Sous-étapes** : les modules, tests et données de `tools/` de la sous-étape 3 de même lettre ; l'étape 3 finie, aucun résidu d'une sous-étape suivante ne reste.
>
>    **Méthode de l'étape 4** (consultation `plan-review` de fin d'étape 3) :
>    - *Trois sortes de commit, trois preuves.* Commentaires seuls : sortie de `gcc -fpreprocessed -dD -E -P` identique avant et après, nombre de lignes de code inchangé, ni essai à blanc ni TSV. Chaînes et messages d'`assert` : la rigueur de l'étape 3 (essai à blanc, test rouge avant la correction, tampons vérifiés). Locales renommées : par `rename-slice`/`rename-apply` seulement, ajoutées au TSV et aux surcharges.
>    - *Propriété des chaînes.* Une chaîne française ou qui cite le cart, lue par un test ou par la sortie d'un binaire (`expect_fatal`, `must_abort_saying`, noms de zone, sortie `REPLAY_FILL`), est traduite par la sous-étape qui tient son site écrivain, à tous ses sites d'un coup ; le commit de chaînes liste d'abord ses sites lecteurs hors de ses modules.
>    - *Extraction fraîche* (corrigée par 4a). Chaque 4x finit par `rename-extract` frais sans `warning:` (les 4 lignes `partial` de `class_coverage` attendues), `rename-rekey-check SUBSTEP=3l` vert, puis les compteurs. `rename-rekey-check` sur les autres sous-étapes est rouge par construction depuis l'étape 3 (chaque ligne appliquée a perdu son ancien nom) : il n'est pas une porte. `rename-rekey` ne sert qu'à une 4x qui ajoute des lignes de locale, écrites depuis une extraction de l'arbre courant et appliquées dans le même commit, puisque les éditions de commentaires déplacent les lignes et que `rename-rekey` apparie par ligne. Les outils reçoivent la sous-étape 4x sous le nom `SUBSTEP=3x` ; un morceau (4c1…) sous `MODULES=` et sa liste.
>    - *Locales cryptiques.* 4a définit une liste candidate mesurable tirée de `declarations.tsv` (locale ou paramètre de portée > 40 lignes, nom de trois caractères ou moins ou non découpé en mots) ; chaque sous-étape la relève avant et après. Pour les 12 statiques locales proposées en exception, l'étape 4 propose aussi un nom dans « Découvert », pour que l'un ou l'autre verdict s'applique mécaniquement à l'étape 5.
>    - *Compteur de français.* Construit par 4a, d'abord, test d'abord : liste de mots-outils français choisis plus détection des accents (le code est en ASCII sans accents) ; fichier d'exceptions au format de `scope_exceptions.txt` ; mêmes colonnes et même scission fait / suivantes que `cart-names-count` ; jamais bloquant ; fixtures de détection, d'homographes (`place`, `file`, `limite`) et d'exception. Sa mesure de départ par sous-étape va dans la note de 4a ; une sous-étape hors norme est coupée, comme 3l.
>    - *Consigne des agents.* La PRD de 4a porte une version étape 4 de `consigne-agent.md` de `reprendre-les-commentaires`, qui lève point par point ses contradictions avec l'étape (code non modifié, assertions intouchables, chaîne humaine signalée sans correction, sortie du préprocesseur identique, destination glossaire) ; la note de 4a l'amende ensuite.
>    - *Citations du cart.* Relevé non bloquant par sous-étape, avant et après : commentaires qui citent `cart` (hors module `game/cart`), `Lua`, `lua.lua`, et noms du TSV d'une ou deux lettres employés nus.
>
> 3c. **Socle.** `engine/`, sauf l'en-tête de 3a : `allocator`, `zone`, `zone_sizes`, `frame_zone`, `game_zone`, `pointer_list`, `entity_list`, `exit_queue`, `collect`, `vec`, `geom`, `entity`, `entity_registry`, `entity_geom`, `dispatch_context`, `bootstrap`. Tests : `heap_allocator.[ch]`, `test_allocator`, `test_arena`, `test_zone`, `test_frame_zone`, `test_game_zone`, `test_pointer_list`, `test_entity_list`, `test_exit_queue`, `test_vec`, `test_geom`, `test_draw_layers_camera` ; `tools/scope_exceptions.txt` suit (`pico8_vec_t`). Résidu ≤ 314.
>
> 4c. **Commentaires — socle.** Modules et tests de 3c. Coupée par 4a (charge 1 612) :
>    - 4c2 (748) : `engine/frame_zone`, `engine/game_zone`, `engine/pointer_list`, `engine/entity_list`, `engine/collect`, `engine/vec`, `engine/geom`, `engine/entity`, `engine/entity_registry`, `engine/entity_geom`, `engine/dispatch_context`, `engine/bootstrap`, `tests/test_frame_zone`, `tests/test_game_zone`, `tests/test_pointer_list`, `tests/test_entity_list`, `tests/test_vec`, `tests/test_geom`, `tests/test_draw_layers_camera`, `tools/scope_exceptions`.

Et, sous l'entrée 4l : « Un morceau (4c1…) suit la PRD de sa sous-étape, avec ses commits et `make test-all` vert ; ses compteurs se lisent par `MODULES="<sa liste>"`, et `SUBSTEP=3x` à zéro clôt la sous-étape après son dernier morceau. » 4c2 est le dernier morceau de 4c : il se lit par `MODULES=<sa liste>`, puis clôt 4c par `SUBSTEP=3c`.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4). Morceau 4c2 = l'entrée 4 appliquée à la seconde moitié de 3c, telle que la note « Découvert » de 4a l'a coupée. Liste `MODULES` de cette PRD (vingt modules) :

```
engine/frame_zone engine/game_zone engine/pointer_list engine/entity_list engine/collect engine/vec engine/geom engine/entity engine/entity_registry engine/entity_geom engine/dispatch_context engine/bootstrap tests/test_frame_zone tests/test_game_zone tests/test_pointer_list tests/test_entity_list tests/test_vec tests/test_geom tests/test_draw_layers_camera tools/scope_exceptions
```

Fichiers (32) : `src/engine/{frame_zone,game_zone,pointer_list,entity_list,collect,vec,geom,entity,entity_registry,entity_geom,dispatch_context,bootstrap}.[ch]`, `tests/test_frame_zone.c`, `tests/test_game_zone.c`, `tests/test_pointer_list.c`, `tests/test_entity_list.c`, `tests/test_vec.c`, `tests/test_geom.c`, `tests/test_draw_layers_camera.c`, `tools/scope_exceptions.txt`.

Entrées lues : `docs/plan-5.md` §1, §2, §4, §5, l'entrée 4 et sa méthode, l'entrée 3c et sa note, les notes « Découvert » de 4a, 4b et 4c1 ; `docs/consigne-etape-4.md` ; `docs/decisions-plan-5.md` (D-1 à D-38) ; le skill `reprendre-les-commentaires` ; la PRD de 4c1 (`b7623ba`, modèle de forme de celle-ci) et `git log -4 --stat` : 4c1 s'est commitée en deux commits de code, `8c1a768` (`refactor:`, chaînes, onze fichiers dont les lecteurs hors morceau) et `a33b88b` (`docs:`, commentaires et note), puis la PRD graduée (`b3c42d1`).

Aucun humain n'était disponible : l'agent rédacteur a chargé lui-même `guideline` aux points « vérifier avec l'utilisateur » de `to-prd`. Les décisions reposent sur le plan et sur des mesures en lecture seule à `b3c42d1` (écritures dans `tmp/` seulement : `make rename-extract` frais, `rename-rekey-check SUBSTEP=3l`, compteurs ; sorties dans `tmp/4c2_baseline/`).

**Condition d'entrée** : `origin/plan-5-noms-anglais` à `b3c42d1` ou un descendant, arbre propre, `make test-all` vert ; `make rename-extract` frais sans `warning:` (les 4 lignes `partial` de `class_coverage` attendues) ; `rename-rekey-check SUBSTEP=3l` vert (0 introuvable) ; la mesure de départ ci-dessous retrouvée à l'identique. Sorties de `class-coverage-net` et `e2e-gap` relevées au départ sous `tmp/4c2_base/`.

## Problem Statement

Le socle objet — zones frame et partie (avec sa sous-zone chargement), listes de pointeurs et d'entités, `collect`, vecteurs et géométrie, classe racine, registre d'entités et `draw_layers`, géométrie d'entité, contexte de dispatch, amorçage — et ses tests sont encore en partie en français et pleins de noms du cart. Les en-têtes de zone sont entièrement français (`Zone partie : une generation par partie, de `eu` a `eu``), les commentaires de `entity_registry.h` (39 lignes de commentaire pour 19 de code) parlent en digrammes (`cv`, `cg`, `hv`, `lt()`, `jl()`, `kj`, `ev`), et 69 renvois nomment des symboles disparus (`pico8_entity_init` ×6, `pico8_jl` ×5, `pico8_fixed_div` ×5, `pico8_zone_partie_reset`…). Les messages d'`assert` du socle citent le cart (« dc: only kj and ev are drawn through dc », « ma: e is a planet (jo:kj) or the camera (qb:ph) »), ceux des tests de zone sont en français.

Surtout, les deux **noms de zone** que `engine/game_zone` écrit, `"partie"` et `"chargement"`, composent le message fatal de zone traduit par 4c1 : le test lit aujourd'hui un message hybride voulu, « `zone chargement full, limit 65536 bytes` ». 4c1 a laissé ces noms à 4c2, leur écrivain. Les mêmes mots apparaissent ailleurs comme **étiquettes de la sortie `REPLAY_FILL`** (`partie <n>`, `max_octets chargement <n>`), mais ce sont d'autres chaînes, écrites par `cmd/replay.c` : il faut les distinguer pour ne traduire que ce que 4c2 possède.

Les compteurs de l'étape 4 ne sont pas à zéro sur 4c2, et la sous-étape 4c ne peut se clore (`SUBSTEP=3c` non nul) qu'à la fin de ce morceau.

## Solution

Appliquer à 4c2 la méthode prouvée par 4a, 4b et 4c1 : un seul morceau, trois états de code — chaînes (rouge d'abord, tous les lecteurs d'un coup), locales (quatre, par `rename-apply`), commentaires (préprocesseur identique, hauteur des blocs gardée dans les huit `.c` cités par `tools/line_coverage_reference.txt`) — puis la note « Découvert » de 4c2, qui clôt 4c. L'exécutant ne commite pas : il laisse deux patchs d'états intermédiaires et trois messages dans `tmp/`, et l'arbre de travail à l'état final (§ « Remise »). À la fin, sur `MODULES=<la liste>` puis sur `SUBSTEP=3c` : français, anti-cart (`backtick`, `comment`), citations à zéro, locales cryptiques à zéro hors D-37, portée à 3 hors D-13, et plus aucun mot de contenu français à la lecture.

## User Stories

1. En tant que lecteur du socle, je veux les commentaires des zones frame et partie en anglais, afin de savoir combien de temps vit un bloc (jusqu'au prochain reset de frame, jusqu'à la release de sa génération, jusqu'au prochain `game_zone_reset`) sans traduire.
2. En tant que joueur ou testeur qui voit un arrêt fatal, je veux lire `p8_fatal: zone load full, limit 65536 bytes` plutôt que le message hybride `zone chargement full…`, afin que le message soit entièrement anglais.
3. En tant que mainteneur de `test_game_zone`, je veux que son `expect_fatal` lise le nom de zone traduit dans le même commit que l'écrivain, afin que `make test` reste vert après chaque commit.
4. En tant que relecteur, je veux voir `test_game_zone` rougir au premier lecteur prévu avec le seul écrivain traduit, et `test_live_slots` rester vert, afin de savoir que le premier lit vraiment le nom de zone et que le second ne le lit pas.
5. En tant que mainteneur de 4l3 et 4f1, je veux que la PRD dise que les étiquettes `partie`, `chargement`, `max_octets` de la sortie `REPLAY_FILL` sont des littéraux de `cmd/replay.c`, pas le nom de zone, afin de savoir qu'elles me reviennent, avec leur lecteur `test_live_slots`.
6. En tant que lecteur, je veux que les messages d'`assert` du socle nomment les fonctions C (`entity_registry_init`, `entities_by_message`, `draw_layers`, `pointer_list_init`, `entity_list_concat`…) plutôt que des digrammes, afin qu'un arrêt se comprenne sans le cart.
7. En tant que lecteur d'un test à processus enfant, je veux que chaque nom de cas soit le nom de sa fonction sans `case_` (`load_zone_full`, `final_score_happiness_survives_reset`), afin de relier une ligne de sortie à son code par `grep`.
8. En tant que relecteur, je veux une table de traduction (`sed`) qui transforme chaque ligne lectrice d'avant en celle d'après, afin de vérifier que seules des étiquettes ont changé.
9. En tant que mainteneur, je veux que les tampons qui reçoivent ces textes soient vérifiés (`char message[160]` de `zone.c`, `char cmd[512]` des deux tests de zone), afin qu'aucune troncature ne masque un message.
10. En tant que lecteur, je veux que les locales à longue portée `bucket_src`, `mem`, `gfx` et la locale française `mesure` portent un nom anglais clair, afin que plus aucun mot français ni nom cryptique ne reste dans le code du morceau.
11. En tant que mainteneur du TSV, je veux que chaque locale renommée passe `renommé` dans `docs/renommage.tsv` avec son identité courante et une ligne `identity` dans `tools/rename_overrides.tsv`, afin que le TSV reste ce qui a été appliqué.
12. En tant que relecteur, je veux un état de commentaires dont la sortie de `gcc -fpreprocessed -dD -E -P` est identique à l'octet, fichier par fichier, afin de le relire sans relire le code.
13. En tant que mainteneur de `e2e-gap`, je veux que chaque bloc de commentaire des huit `.c` cités par `tools/line_coverage_reference.txt` garde sa hauteur, afin que la référence de couverture par ligne ne décale pas.
14. En tant que lecteur, je veux que chaque renvoi nomme un symbole qui existe (`p8_fatal`, `game_zone_reset`, `reset_world`, `cart_update60`, `p8_fixed_div`, `entity_registry_init`), afin de le suivre par `grep`.
15. En tant que mainteneur, je veux que les pièges du socle restent (zone frame initialisée paresseusement parce que `bump_zone_init` empoisonne sous ASan ; handle statique parce qu'une liste garde un pointeur vers sa zone ; sous-zone chargement que ni la bascule ni la release ne touchent ; `TROU-3`, `TROU-5`, `TROU-15`), afin qu'on ne les réintroduise pas.
16. En tant que mainteneur, je veux qu'aucune décision non tranchée du registre (D-13 statiques `kw`/`CV`/`CG`, D-14 ordre des macros de taille, D-29 locales de digramme, D-36, D-37, D-38) ne soit appliquée, afin que l'humain garde la main.
17. En tant qu'exécutant des sous-étapes suivantes (4f1, 4g, 4j, 4l3), je veux la liste des mentions de « zone partie » et des renvois `pico8_zone_partie*` laissés hors de 4c2, afin de les réécrire avec les noms neufs `game`/`load`.
18. En tant qu'humain du point d'arrêt, je veux que 4c2 clôture 4c avec `SUBSTEP=3c` mesuré à zéro (hors D-13 et D-37), afin de savoir que le socle entier est fait.
19. En tant qu'orchestrateur, je veux deux patchs cumulatifs et trois messages dans `tmp/`, afin de commiter les trois sortes de commit séparément avec `make test-all` vert à chacun.

## Implementation Decisions

### Mesure de départ (`b3c42d1`, extraction fraîche, `MODULES=<la liste>`, écrits dans `tmp/4c2_baseline/` seulement)

`make rename-extract` : aucun `warning:`, 4 lignes `partial` (`cmd/class_coverage.c`, `tests/test_class_coverage.c` × 2 configurations) ; `rename-rekey-check SUBSTEP=3l` : 0 identité introuvable.

| Compteur | id | backtick | string | comment / détail | total |
|---|---|---|---|---|---|
| `french-count` | 0 | 0 | 8 | 387 | 395 |
| `cart-names-count` | 0 | 125 | 0 | 62 | 187 |
| `cart-citations` (cart/lua/short) | — | — | — | 6/8/146 | 160 |
| `cryptic-locals` | — | — | — | `bucket_src` (`draw_layers`, 53 lignes), `mem` (90), `gfx` (89) de `test_draw_layers_draw_dispatches_main_camera_apply_camera_then_real_planet_draw` | 3 |
| `scope-count` | — | — | — | `static` : `kw` (`test_frame_zone`), `CV`, `CG` (`test_game_zone`) — D-13 | 3 |

Charge 395 + 187 + 160 + 3 = 745, contre 748 écrits par 4a : les trois mots-outils de `"vider la file de sortie entre deux eu"` (`la`, `de`, `entre`), retirés de `tests/test_game_zone.c:410` par le commit de chaînes de 4c1. Avec `MODULES=`, les outils ne scindent pas `done`/`later` : le critère est zéro dans chaque colonne.

Détail :
- *Français* (`SITES=1`) : `comment` (messages d'`assert` compris) dans `test_game_zone` 190, `test_frame_zone` 90, `game_zone` 82, `frame_zone` 24, `collect` 1 ; `string` 8 : `tests/test_entity_list.c` 130, 156, 179 (`tu`, du nom d'arène `"tu ents"`), `tests/test_frame_zone.c:393` et `tests/test_game_zone.c` 175, 322, 391, 415 (diagnostics et nom d'arène). **Angle mort** (consigne, point 6, amendé par 4c1) : relevé complémentaire avec la liste de mots de contenu de 4c1 (`pleine pleines limite octets place places essai attendu obtenu sortie inconnu propre remplie bascule rendre rendu rendue poignee arene designee vider vidage videe enfile moitie moities sonde agrandir agrandissement tete bloc cellule cellules partie chargement liste garde planter recouvert arrondi remise decompte reelles plafond entite entites`, `grep -owiE`) : 87 mots — `test_game_zone.c` 36, `test_frame_zone.c` 19, `game_zone.h` 17, `game_zone.c` 5, `frame_zone.h` 4, `frame_zone.c` 2, `entity_registry.h` 2, `entity_geom.h` 1, `bootstrap.h` 1 ; et 50 lignes à littéral portant un mot français ou un nom du cart (`test_game_zone.c` 27, `test_frame_zone.c` 14, `entity_list.c`, `pointer_list.c`, `game_zone.c`, `test_entity_list.c` 2 chacun, `collect.c` 1). L'exécutant lit de toute façon chaque chaîne et chaque message d'`assert` des 32 fichiers. Ses restes en fin de PRD sont des homographes anglais (`place`, `garde`…), listés un à un dans la note.
- *Anti-cart* : `backtick` 125 — `engine/bootstrap` 26, `entity_registry` 20, `test_game_zone` 20, `entity_geom` 13, `test_draw_layers_camera` 12, `entity` 7, `entity_list` 7, `geom` 6, `pointer_list` 6, `collect` 4, `game_zone` 2, `test_frame_zone` 1, `test_pointer_list` 1 ; `comment` 62 — `entity_registry` 21, `entity_list` 6, `bootstrap` 5, `vec` 7, `test_vec` 5, `collect` 4, `entity` 3, `frame_zone`, `game_zone`, `geom`, `pointer_list` 2 chacun, `dispatch_context`, `test_frame_zone`, `test_game_zone` 1 chacun.
- *Noms nus `pico8_*`/`PICO8_*`* : 69 occurrences (`tmp/4c2_baseline/pico8_grep.txt`) — `pico8_entity_init` 6, `pico8_jl` 5, `pico8_fixed_div` 5, `pico8_ptrs_free` 3, `pico8_fatal` 3, `pico8_mo`, `pico8_ke_init`, `pico8_fixed_sin`, `pico8_file_sortie`, `pico8_ents_free`, `pico8_entity_shutdown`, `pico8_entity` 2 chacun, et 33 noms à une occurrence (`pico8_zone_partie[_reset]`, `pico8__update60`, `pico8_bump_zone_init`, `pico8_bascule_zone_init`, `pico8_ptrs_t`, `pico8_struct_class_t`, `pico8_qz_call`, `pico8_mo_qb`…). Cinq sont dans des chaînes (état 1 : `entity_registry.c:127`, `pointer_list.c:35`, `entity_list.c:43`, `test_frame_zone.c:395`, `test_game_zone.c:392`), le reste en commentaire (état 3). Le critère de fin y ajoute ce `grep` à 0, comme à 4b et 4c1.
- *Citations* : `cart` 6 (`bootstrap.h:5`, `geom.c:29`, `geom.h:36`, `vec.c:48`, `vec.h:32`, `vec.h:37`) ; `lua` 8 (`bootstrap.h:16`, `entity_geom.h:25`, `entity_registry.c:207`, `entity_registry.c:235`, `geom.c:15`, `geom.h:8`, `vec.h:8`, `test_vec.c:112`) ; `short` 146 : mots français (`le` 31, `et` 17, `du` 7, `en` 3, `ou` 2, `lu` 1), digrammes du cart (`mo` 9, `kj` 8, `eu` 7, `jo` 5, `hv` 5, `pq`, `ph`, `ev` 3 chacun, `pj`, `bm` 2 chacun, `ra`, `qb`, `oc`, `ma`, `jb`, `hp`, `hk`, `hd`, `hb`, `gh`, `ga`, `fe`, `cr`, `yt`, `yb`, `xr`, `xl` 1 chacun) et noms de paramètres cités nus (`v` 9 dans `test_vec.c`, `d` 5, `n` 3, `p`, `e` 1 chacun).
- *Lignes de commentaire* (non vides moins préprocesseur ; lignes de code entre parenthèses) : `frame_zone.c` 3 (20), `frame_zone.h` 3 (6), `game_zone.c` 6 (63), `game_zone.h` 11 (15), `pointer_list.c` 0 (89), `pointer_list.h` 15 (24), `entity_list.c` 0 (132), `entity_list.h` 16 (30), `collect.c` 3 (85), `collect.h` 15 (16), `vec.c` 3 (41), `vec.h` 12 (20), `geom.c` 5 (42), `geom.h` 14 (22), `entity.c` 0 (33), `entity.h` 11 (11), `entity_registry.c` 19 (209), `entity_registry.h` 39 (19), `entity_geom.c` 1 (78), `entity_geom.h` 12 (10), `dispatch_context.c` 0 (23), `dispatch_context.h` 7 (12), `bootstrap.c` 6 (57), `bootstrap.h` 13 (6), `test_frame_zone.c` 16 (362), `test_game_zone.c` 27 (351), `test_pointer_list.c` 2 (201), `test_entity_list.c` 0 (170), `test_vec.c` 23 (116), `test_geom.c` 0 (77), `test_draw_layers_camera.c` 26 (87) ; `tools/scope_exceptions.txt` 2 lignes `#` (2 hors commentaire), déjà en anglais, 0 à tous les compteurs.
- *Étiquettes `TROU-N`* : `TROU-3` (`collect.c:57`, `collect.h:27`), `TROU-5` (`entity_registry.c:196`, `bootstrap.c:56`), `TROU-15` (`entity_geom.h:12`) restent telles quelles (consigne, point 4). Un écart nouveau se marque `/* TROU-? */` et se propose.
- *Hauteur des blocs* (consigne, point 8, amendé par 4b) : huit `.c` du morceau sont cités par `tools/line_coverage_reference.txt` — `bootstrap.c`, `collect.c`, `dispatch_context.c`, `entity.c`, `entity_geom.c`, `entity_registry.c`, `geom.c`, `vec.c`. Chaque bloc de commentaire y garde son nombre de lignes, et chaque chaîne réécrite à l'état 1 y reste sur sa ligne (un message d'`assert` allongé ne se coupe pas en deux lignes). `frame_zone.c`, `game_zone.c`, `pointer_list.c`, `entity_list.c` n'y sont pas ; leurs blocs gardent quand même leur hauteur quand c'est possible, comme à 4c1. `make e2e-gap` est exigé égal à l'octet à chacun des trois états.

### État 1 — chaînes (`refactor:`), rouge d'abord

**Noms de zone : écrivain, lecteurs, non-lecteurs.** 4c2 tient deux écrivains lus : `src/engine/game_zone.c` ligne 20, `flip_zone_init(…, "partie", …)`, et ligne 22, `bump_zone_init(…, "chargement", …)`. Un nom de zone n'est lu que par le gabarit fatal de `engine/zone.c` (`zone %s full, limit %lu %s`, `zone->name`, lignes 54, 92, 181) ; `slot_name` d'`allocator.h` est un autre champ (nom d'arène), et aucun autre code ne lit `zone->name` (`grep` de `->name`, `.name` dans `src/`). Traductions, alignées sur les identifiants (`game_zone`, `load_zone`) et le nom `"frame"` déjà anglais :

| Écrivain | Avant | Après |
|---|---|---|
| `game_zone.c:20` | `"partie"` | `"game"` |
| `game_zone.c:22` | `"chargement"` | `"load"` |

Lecteurs (`grep -rn 'zone partie\|zone chargement\|"partie\|"chargement' src tests tools Makefile`, à refaire avant le rouge ; tout site trouvé en plus s'ajoute, comme `test_frame_zone.c:416` à 4c1) :
- `tests/test_game_zone.c:414` (4c2) : `expect_fatal("chargement_full", "zone chargement full, limit 65536 bytes")` → `expect_fatal("load_zone_full", "zone load full, limit 65536 bytes")` (le nom de cas change aussi, ci-dessous). C'est le **seul** lecteur : aucun test ne fait déborder la zone partie (`"zone partie"` n'apparaît que dans de la prose d'`assert`).

**Non-lecteurs, à ne pas toucher** (le prompt d'origine de cette PRD les supposait lecteurs ; la lecture du code dit le contraire) :
- `src/cmd/replay.c` `write_fill` (lignes 76-79) écrit dans le fichier `REPLAY_FILL` ses **propres littéraux** `"partie %lu\n"`, `"max_octets partie %lu\n"`, `"chargement %lu\n"`, `"max_octets chargement %lu\n"`, à côté de `"rangee"`, `"particule"`, `"max_vivantes"`, `"max_octets"` ; il ne lit pas le nom de zone. Écrivain : 4l3 (note de 4c1, « Laissés »).
- `tests/test_live_slots.c` 1377-1382 (attendu reconstruit : `"partie %lu\nmax_octets partie %lu\n"`, `"chargement %lu\n"`, `"max_octets chargement %lu\n"`) et `fill_value` (1397-1421) lisent ces étiquettes de `cmd/replay.c`, pas le nom de zone ; ses `printf` (`partie=%lu`, 129, 348, 620…) sont ses propres libellés. Lecteur rangé en 4f1 ; il suit 4l3.
- Les commentaires et messages d'`assert` qui disent « zone partie », « sous-zone chargement » hors de 4c2 (`src/game/cart.h:19`, `src/game/cart.c:129-130`, `src/game/economy.h:24` (`pico8_zone_partie()`), `src/cmd/replay.c:25-27`, `tests/test_live_slots.c` 333, 345, 494, 626-635…) : prose de leur sous-étape, listée dans la note pour qu'elle dise `game zone` / `load sub-zone`.

Rouge prévu, **écrivain seul** (`game_zone.c` traduit, rien d'autre) : `test_game_zone` rougit à `expect_fatal("chargement_full", …)`, ligne 414 (sortie 3, mais `strstr` ne trouve plus `zone chargement`) ; tous les `expect_*` d'avant restent verts. `test_live_slots`, `test_frame_zone` et les e2e restent verts (preuve de non-lecture, à écrire dans le message). Lecteur traduit, vert.

**Noms de cas** (règle de 4c1 : le nom du cas est le nom de sa fonction sans `case_`), écrits dans `CASES[]` et lus par les `expect_*` de `main` du même fichier (aucun lecteur ailleurs : `grep` sur `src tests tools Makefile` rend 0) :
- `tests/test_game_zone.c` (5) : `prise_and_max` → `grab_arena_and_max`, `chargement_dead_after_cart_load` → `load_zone_dead_after_cart_load`, `chargement_prise_and_max` → `load_zone_grab_arena_and_max`, `chargement_full` → `load_zone_full`, `listes_statiques_dans_chargement` → `static_lists_in_load_zone` ;
- `tests/test_frame_zone.c` (1) : `ky_happiness_survives_reset` → `final_score_happiness_survives_reset`.
Rouge prévu, `CASES[]` traduit seul : `test_game_zone` échoue au premier `expect_*` d'un nom changé, `expect_clean("prise_and_max")` (ligne 409 : `unknown case`/`cas inconnu`, sortie 2) ; `test_frame_zone` à `expect_clean("ky_happiness_survives_reset")` (ligne 420). Lecteurs traduits, vert. Contrôle : pour chaque `{"x", case_y}` des `CASES[]` de 4c2, `x == y`.

**Messages d'`assert` de `src/engine/` sans lecteur** (`grep -F` de chaque fragment sur `src tests tools Makefile` : seuls les écrivains ; aucun `must_abort_saying` ni `expect_assert` ne les lit, donc pas de rouge possible — à écrire dans le message) ; traduits dans le vocabulaire du TSV, chaque nom de fonction relu dans le code et non deviné :
- `entity_registry.c:127` `"pico8_entity_init() must run before cv is read"` → `"entity_registry_init() must run before entities_by_message is read"` ;
- `entity_registry.c:222` `"dc: only kj and ev are drawn through dc"` → `"draw_layers: only draw and draw_overlay go through draw_layers"` ;
- `pointer_list.c` 9, 24, 35, 43, 53, 69, 99 et `entity_list.c` 9, 27, 37, 43, 51, 61, 123 : `ptrs_init`/`ents_init` → `pointer_list_init`/`entity_list_init`, `ptrs primitive`/`ents primitive` → `pointer_list primitive`/`entity_list primitive`, `ptrs_add on a list pico8_ptrs_init never gave a zone` → `pointer_list_add on a list pointer_list_init never gave a zone` (idem `entity_list`) ;
- `entity_list.c:92` `"cat: … unlike has"` → `"entity_list_concat: … unlike entity_list_has"` ;
- `collect.c` 31, 50 (`nv`, `la`, `gu`, `jp`) : noms C des fonctions porteuses (`collect_map_pointer_list`, `collect_concat_pointer_list`) et de celles qui tolèrent NULL (relevées dans le TSV, colonne `ancien`, puis vérifiées dans le code) ;
- `geom.c:51` `"qz: …"` → préfixe `box_contains:` (la fonction qui porte l'`assert`) ;
- `entity_geom.c:60` `"ma: e is a planet (jo:kj) or the camera (qb:ph)"` → `"entity_geom_shake: the entity is a planet (its draw) or the camera (its update)"`, affirmation revérifiée sur les appelants de `entity_geom_shake` avant d'écrire.
Les quatre fichiers `collect.c`, `entity_registry.c`, `entity_geom.c`, `geom.c` sont dans `tools/line_coverage_reference.txt` : chaque message reste sur sa ligne.

**Chaînes des tests de 4c2** (écrivain et lecteur dans le même fichier, ou sans lecteur) :
- *Noms d'arène* : `"mesure des bulles"` (`test_game_zone.c:175`) → `"tooltip-measure"` ; `"tu ents"` (`test_entity_list.c` 130, 156, 179) → `"test-entity-list"`, sur le modèle `test-*` de 4c1. Aucun lecteur (un nom d'arène n'atteint `p8_fatal` qu'en débordement, qu'aucun de ces cas ne provoque).
- *Diagnostics* `fprintf(stderr, …)` de `test_frame_zone.c` (373, 383, 393, 404) et `test_game_zone.c` (361, 371, 381, 391, 401) : vocabulaire de 4c1 (`expected clean, exit %d`, `expected use-after-poison`, `expected an assert "%s"`, `expected exit 3 and "%s"`, `unknown case %s` ; `statut` → `status`).
- *Messages d'`assert`* : `test_frame_zone.c` 67, 168, 177, 188, 216, 219, 374, 384, 395 ; `test_game_zone.c` 73, 95, 153, 165, 166, 182, 188, 204, 219, 228, 244, 264, 268, 290, 294, 296, 297, 362, 372, 382, 392. Ceux qui citent le cart (`jo:gh`, `cr`, `jo:fe`, `hp:jb`, `pj:ph`, `ky`, `eu`, `dr`, `bg`, `ok.le`, `by`, `ga`, `P`) nomment les fonctions C ou le rôle ; `… doit appeler pico8_fatal` → `… must call p8_fatal` (texte de 4c1 : `"a full zone must call p8_fatal"`). « zone partie » → « game zone », « sous-zone chargement » → « load sub-zone ».
- Préfixes de binaire (`test_frame_zone:`, `test_game_zone:`), `STDERR_LOG`, `"use-after-poison"` inchangés. `test_pointer_list`, `test_vec`, `test_geom`, `test_draw_layers_camera` n'ont aucune chaîne française ni du cart (les `"kj"`, `"ra"` de `test_draw_layers_camera.c` sont dans des commentaires, état 3).

Preuves, dans le message :
- **lecteurs hors de 4c2** : aucun (en tête du message, avec la preuve de non-lecture de `cmd/replay.c` et `test_live_slots`) ;
- **rouge d'abord, par binaire**, chaque binaire lancé seul (`make unit-run-test_X` ou équivalent) : les deux rouges prédits ci-dessus, un pour l'écrivain de zone, un par fichier de `CASES[]`. Tout écart à la prédiction est écrit dans le message ;
- **table de traduction** : un `sed` (dans le message) qui, appliqué aux lignes `-` de la ligne lectrice `test_game_zone.c:414` et des lignes `expect_*` des deux `main`, rend exactement leurs lignes `+` ;
- **tampons** : `char message[160]` (`zone.c`) — `zone load full, limit 65536 bytes`, `zone game full, limit <capacité> bytes`, plus courts qu'avant ; `char cmd[512]` de `test_frame_zone.c` et `test_game_zone.c` — plus long nom de cas `research_project_tooltip_survives_reset` (39, inchangé) et `final_score_happiness_survives_reset` (36), plus le chemin et `STDERR_LOG`, tiennent ;
- `make test-all` vert, `tests/e2e/*.sha256` inchangés (aucun nom de zone dans un framebuffer) ; `make e2e-gap` égal à l'octet.

### État 2 — locales (`refactor:`), procédure de 4b

Quatre identités, renommées par `rename-slice`/`rename-apply` seulement (méthode, première puce), après une **extraction fraîche de l'état 1** (les identités portent un offset, qui bouge avec les chaînes) :

| Fichier | Fonction | Ancien | Nouveau | Pourquoi |
|---|---|---|---|---|
| `src/engine/entity_registry.c` | `draw_layers` | `bucket_src` | `message_bucket` | portée 53 lignes ; `src` n'est pas un mot du TSV ; le seau d'`entities_by_message(message)`, distinct du `bucket` de couche plus bas |
| `tests/test_draw_layers_camera.c` | `test_draw_layers_draw_dispatches_main_camera_apply_camera_then_real_planet_draw` | `mem` | `memory` | portée 90 lignes ; précédent 4b (`test_demo.c`) |
| idem | idem | `gfx` | `graphics` | portée 89 lignes ; précédent 4b |
| `tests/test_game_zone.c` | `icons_bytes` | `mesure` | `measure_arena` | mot français (§1 : « plus aucun mot français dans le code »), `conservé` au TSV ; portée 16 lignes, non cryptique, mais l'entrée 4 retire les mots français restants |

Ce ne sont **pas** des statiques de fichier : D-38 (statiques `mem`/`gfx`) ne les couvre pas, et `mem`/`gfx` locales suivent le précédent de 4b, qui n'avait pas ouvert d'entrée au registre. Les autres locales et paramètres courts des fichiers (`qc`, `ma`, `jo`, `fm`, `hv`, `mh`, `le`, `by`, `ra`, `qa`, `me`, `fy`, `kw`…) ont une portée ≤ 40 lignes ou relèvent de D-13/D-29, non tranchées : non renommés, et jamais cités entre backticks dans les commentaires réécrits.

Procédure (comme `f9014c5`) : identités relevées dans `tmp/ast_rename/declarations.tsv` ; table `ancien<TAB>nouveau<TAB>fichier<TAB>identité` dans `tmp/ast_rename/decision.tsv`, `make rename-apply TABLE=tmp/ast_rename/decision.tsv` ; dans le même état, les quatre lignes `conservé` de `docs/renommage.tsv` passent `renommé`, nom neuf, identité courante ; quatre lignes `identity` dans `tools/rename_overrides.tsv` ; `rename_derive.py` reproduit les quatre lignes à l'octet ; `rename-table-check` rouge par construction, même compte avant et après (à relever). Contrôle de masquage avant l'apply : aucun des noms neufs n'est déjà déclaré dans la fonction (`declarations.tsv`). Pas de `rename-rekey` : les lignes sont écrites depuis l'extraction de l'arbre courant et appliquées dans le même état (méthode, troisième puce).

Effets de bord à écrire dans le message : `mesure` devient un ancien nom ambigu (minuscule, `conservé` ailleurs) ; la locale homonyme de `tests/test_live_slots.c:566` (`icons_bytes`, 4f1) n'est pas comptée en `identifier` (autre identité) mais une mention exacte `` `mesure` `` le serait en `backtick` partout (cf. D-36) — à proposer à 4f1 dans la note. `entity_registry.c` étant dans `line_coverage_reference.txt`, l'apply ne change aucune ligne de place (renommage dans la ligne). `make test-all` vert, `e2e-gap` égal à l'octet.

### État 3 — commentaires (`docs:`), consigne étape 4

« Fichier d'avant » : l'état 2, copié sous `tmp/4c2_before_comments/` avant tout agent (consigne, point 8). Un agent par groupe, en-tête et `.c` ensemble, chaque test avec son module : `frame_zone` + `test_frame_zone` ; `game_zone` + `test_game_zone` ; `pointer_list` + `entity_list` + `collect` + `test_pointer_list` + `test_entity_list` ; `vec` + `geom` + `test_vec` + `test_geom` ; `entity` + `entity_registry` + `dispatch_context` + `test_draw_layers_camera` ; `entity_geom` + `bootstrap` ; `tools/scope_exceptions.txt` vérifié seul (rien à changer attendu). Consigne `docs/consigne-etape-4.md`, rapport au format amendé. Sans agents, l'exécutant applique la consigne lui-même et remplit le rapport, comme à 4a–4c1.

Preuves : sortie de `gcc -fpreprocessed -dD -E -P` identique à l'octet, fichier par fichier ; lignes de code inchangées ; dans les huit `.c` cités par `line_coverage_reference.txt`, lignes hors commentaire aux mêmes numéros (`diff` des numéros) ; lignes de commentaire avant/après dans le message.

Points connus, sans préjuger du rapport :
- *Contrats à garder* (destination 4) : bloc de la zone frame valable jusqu'au prochain reset, fait en tête de `cart_update60` (`pico8__update60` périmé) ; bloc de la zone partie valable jusqu'à la release de sa génération, faite par le flush de la file de sortie qui suit le flip ; bloc de la sous-zone chargement valable jusqu'au prochain `game_zone_reset` (`pico8_zone_partie_reset` périmé), que ni le flip ni la release ne touchent ; jamais de `free` sur ces blocs ; `game_zone_release` sans génération en attente ne fait rien ; les maxima de session que le reset garde ; qui libère dans `pointer_list`/`entity_list`/`collect` (zone fournie par l'appelant).
- *Pièges à garder* (destination 3, 40 mots) : init paresseuse de la zone frame (`bump_zone_init` empoisonne sous ASan, un initialiseur statique ne le ferait pas ; `pico8_bump_zone_init` périmé) ; handle statique parce qu'une liste garde un pointeur vers sa zone (« ents/ptrs » → `entity_list`/`pointer_list`) ; les `TROU-3`, `TROU-5`, `TROU-15` nus.
- *Renvois périmés*, chacun vérifié par `grep` puis remplacé ou supprimé : les 64 noms `pico8_*` de commentaire (`pico8_entity_init` → `entity_registry_init`, `pico8_jl` → `reset_world`, `pico8_fixed_*` → `p8_fixed_*`, `pico8_zone_partie[_reset]` → `game_zone[_reset]`, `pico8_zone_partie.h` → `engine/game_zone.h`, `pico8__update60` → `cart_update60`, `pico8_ptrs_*` → `pointer_list_*`, `pico8_ents_*` → `entity_list_*`, `pico8_mo` → `main_camera`, `pico8_bascule_zone_init` → `flip_zone_init`, `pico8_struct_class_t` → `entity_class_t`, `pico8_file_sortie` → `exit_queue`…), par le TSV (`ancien` → `nouveau`) et jamais de mémoire.
- *Digrammes et noms du cart* : `entity_registry.h` (`cv`, `cg`, `hv`, `lt()`, `kj`, `ev`), `bootstrap.h` (`hv`, `jl`, « the cart »), `test_draw_layers_camera.c` (`dc("kj")`, `mo:md(ci[o])`, `jo`, `ra`, `gd(self)`, `cv["kj"]`), `entity_geom.[ch]`, `geom.[ch]`, `vec.[ch]` (« Lua's … », « the cart ») : réécrits par les noms C et le rôle (consigne, point 2) ; « Lua would error on … » → ce que fait le C (« deleting from an empty list », « an absent layer is dropped ») ou disparaît ; un nom encore porté par une locale conservée (`qc`, `ma`, `ra`) n'est pas mis entre backticks.
- *Noms de paramètres cités nus* (`v` ×9 dans `test_vec.c`, `d`, `n`, `p`, `e`) : la phrase se réécrit (« the vector », « the divisor »…).
- *Macros de taille* (`FRAME_ZONE_BYTES`, `LOAD_ZONE_BYTES`, `ZONE_GENERATION_BYTES`) : commentaires écrits avec les noms actuels ; D-14 non appliquée.
- *Affirmations à revérifier en traduisant* (consigne, rubrique 7) : « une generation par partie, de `eu` a `eu` » (le flip est-il fait par `cart_start_game` seul ?) ; « au chargement ou a l'arret suivant » pour le reset de la sous-zone ; « faite en tete de update60 » pour la zone frame ; la liste des contrats de `entity_registry.h` sur `entities_by_tag` d'une étiquette jamais enregistrée ; les phrases de `vec.h`/`geom.h` sur la saturation de `p8_fixed_div` (`test_vec.c` : `test_normalize_zero_length_vector_saturates_via_fixed_div_by_zero`).

### Ce qui reste hors de 4c2

- Étiquettes `partie`, `chargement`, `max_octets`, `max_vivantes`, `max_elements`, `rangee`, `particule` de la sortie `REPLAY_FILL` (`cmd/replay.c`, 4l3) et leur lecteur `tests/test_live_slots.c` (4f1, à traduire avec 4l3 par la règle de propriété) ; proposition pour 4l3, alignée sur les noms de zone de 4c2 : `game`, `max_bytes game`, `load`, `max_bytes load`. Le commentaire d'en-tête de `cmd/replay.c` (`PICO8_REMPLISSAGE`, lignes 20-27) aussi.
- Mentions « zone partie », « sous-zone chargement », `pico8_zone_partie*` hors de 4c2 : `src/game/cart.h:19`, `src/game/cart.c:129-130` (4j), `src/game/economy.h:24` (4g), `src/cmd/replay.c:25-27` (4l3), `tests/test_live_slots.c` (4f1, prose et messages d'`assert`).
- La locale `mesure` de `tests/test_live_slots.c` (`icons_bytes`, 4f1) : même renommage proposé (`measure_arena`), et son nom d'arène `"mesure des bulles"` si elle en a un.
- D-13 (`kw`, `CV`, `CG`), D-14, D-29, D-36, D-37, D-38 : non tranchées, non appliquées.

### Registre (`docs/decisions-plan-5.md`)

Aucune entrée nouvelle n'est prévue : les locales renommées suivent le précédent de 4b, les noms de zone et de cas suivent la règle de propriété et celle de 4c1. Une entrée ne s'ajoute (format des conventions, ligne d'index comprise) que si un rapport d'agent soulève une question de nommage ou un faux positif qui résiste à la réécriture ; la note le dit dans les deux cas. D-13 se remesure à la fin (`scope-count SUBSTEP=3c` doit rendre exactement ses trois statiques de 4c2).

### Fin de PRD

Ordre : `make rename-extract` frais sans `warning:` (4 `partial`) ; `rename-rekey-check SUBSTEP=3l` vert ; compteurs sur `MODULES=<la liste>`, puis sur `SUBSTEP=3c` (clôture de 4c).

**Critère de fin** :
- `MODULES=<la liste>` : `french-count` 0 dans les quatre colonnes ; `cart-names-count` 0 dans les quatre colonnes ; `cart-citations` 0/0/0 ; `cryptic-locals` 0 ; `scope-count` 3 (`kw`, `CV`, `CG`, D-13 en attente), rien d'autre ;
- `SUBSTEP=3c` : `french-count` et `cart-names-count` 0 en `done` et `later` ; `cart-citations` 0/0/0 ; `cryptic-locals` 2 (`argc`/`argv` de `tests/test_allocator.c`, D-37) ; `scope-count` 3 (D-13) ;
- `grep` `pico8_`/`PICO8_` des 32 fichiers à 0 ;
- relevé complémentaire des mots de contenu : chaque reste est un homographe anglais, listé dans la note ;
- les deux `CASES[]` satisfont `nom == fonction sans case_` ;
- `grep -rnw 'partie\|chargement' src/engine tests/test_game_zone.c tests/test_frame_zone.c` à 0.

**Invariant (§5)** : `make test-all` vert à chacun des trois états, `tests/e2e/*.sha256` inchangés ; `make raylib` et `make screencast-viewer` compilent et linkent, jamais lancés ; `rename-tool-test` et les `rename-*-test` verts ; `class-coverage-net` et `e2e-gap` rouges, sorties égales à l'octet à celles de `b3c42d1` (`tmp/4c2_base/`) ; `data-residue` et `composition-residue` à 0, `--detail` et `--champs` identiques ; `make sin-table` reproduit `quarter_wave.h` à l'octet.

### Note « Découvert » de 4c2 (sous l'entrée 4c de `docs/plan-5.md`, après la note de 4c1)

Elle porte : mesure de départ et de fin (dont le relevé complémentaire et ses homographes) ; la distinction nom de zone / étiquette `REPLAY_FILL` et le seul lecteur trouvé, avec les rouges de l'état 1 et la table de traduction ; les quatre locales et leurs identités ; lignes de commentaire, hauteur des blocs des huit `.c`, affirmations corrigées de l'état 3 ; ce qui est laissé à 4f1, 4g, 4j, 4l3 ; la clôture de 4c (`SUBSTEP=3c`) ; tout amendement de `docs/consigne-etape-4.md` tiré des rapports — au moins celui-ci, si l'exécution le confirme : un même mot français peut être deux chaînes d'écrivains différents (nom de zone de `engine/game_zone`, étiquette de `cmd/replay.c`) ; la propriété se décide par l'écrivain de la chaîne lue, pas par le mot.

## Implementation Chunks

Un seul morceau (charge 745 ≤ 1 382). L'exécutant produit, dans cet ordre, `make test-all` vert à chaque état :
1. état 1, chaînes ; `git diff HEAD > tmp/4c2_stage1.patch`, message `tmp/4c2_1_msg.txt` (`refactor: plan 5 morceau 4c2 — noms de zone, cas et messages du socle objet en anglais`, absence de lecteur hors de 4c2 en tête) ;
2. état 2, locales ; `git diff HEAD > tmp/4c2_stage2.patch` (cumulatif : états 1 et 2), message `tmp/4c2_2_msg.txt` (`refactor: plan 5 morceau 4c2 — locales bucket_src, mem, gfx et mesure au nom complet`) ;
3. état 3, commentaires, note « Découvert », amendements de la consigne et, s'il y a lieu, du registre, laissés dans l'arbre de travail ; message `tmp/4c2_3_msg.txt` (`docs: plan 5 morceau 4c2 — commentaires du socle objet en anglais, note Découvert, 4c close`).

## Remise

L'exécutant ne commite pas. Il laisse :
- `tmp/4c2_stage1.patch` : diff complet de `HEAD` à l'état 1 (s'applique par `git apply` sur `HEAD` propre) ;
- `tmp/4c2_stage2.patch` : diff complet de `HEAD` à l'état 2, cumulatif ;
- `tmp/4c2_1_msg.txt`, `tmp/4c2_2_msg.txt`, `tmp/4c2_3_msg.txt` : messages de commit en français, récit de chantier et preuves compris (destination « message de commit » du skill), terminés par les lignes d'attribution de la session ;
- l'arbre de travail à l'état 3 : commentaires, note, consigne (et registre s'il a changé).

Si l'état 2 se révèle vide (par exemple si un verdict humain sur une entrée du registre arrive avant l'exécution et le rend caduc), `tmp/4c2_stage2.patch` est égal à l'octet à `tmp/4c2_stage1.patch`, `tmp/4c2_2_msg.txt` n'est pas écrit, et le rapport final le dit en une ligne, comme à 4c1.

## Testing Decisions

- Aucun test neuf, aucune assertion ajoutée ni modifiée dans sa condition (consigne, points 5 et 7). Le filet : les tests à processus enfant qui lisent les messages et les noms de cas (`test_game_zone`, `test_frame_zone`), prouvés lecteurs par le rouge d'abord ; `test_live_slots`, prouvé non-lecteur du nom de zone en restant vert avec l'écrivain seul ; le compilateur pour l'état 2 ; la sortie du préprocesseur et `e2e-gap` pour l'état 3.
- Les binaires ASan (`test_frame_zone`, `test_game_zone`) et les autres tests du morceau passent tous par `make test` ; les e2e par `make e2e`.
- Un bon contrôle juge une sortie observable : texte lu sur stderr, statut de sortie, `*.sha256` des e2e, sortie à l'octet des gardes hors suite.
- Prior art : `8c1a768` (lecteurs listés d'abord, rouge d'abord par binaire, `sed` de traduction, règle des noms de cas), `f9014c5` (locales par `rename-apply`, TSV et surcharges), `a33b88b` et `68e543d` (préprocesseur fichier par fichier, hauteur des blocs).

## Out of Scope

- Toute ligne de code hors des chaînes de l'état 1 et des quatre locales de l'état 2.
- Les étiquettes de `REPLAY_FILL` et leur lecteur, les mentions de la zone partie hors de 4c2, la locale `mesure` de `test_live_slots`.
- `TROU-N` → `GAP-N` (étape 5) ; toute entrée du registre sans verdict (D-13, D-14, D-29, D-36, D-37, D-38).
- Le formatage du code, `tools/french_count.py` et sa liste de mots.

## Further Notes

- `"game"` et `"load"` suivent les identifiants (`game_zone`, `load_zone`, `LOAD_ZONE_BYTES`) et le nom `"frame"` déjà anglais : le message fatal devient `zone game full…` / `zone load full…`, cohérent avec `zone frame full…`.
- La charge de 4c2 est la plus légère des morceaux restants de 4c (745), mais elle porte la plus forte densité de noms du cart en commentaire (`entity_registry.h`, `bootstrap.h`, `test_draw_layers_camera.c`) : le risque est la traduction mot pour mot d'une affirmation fausse (consigne, rubrique 7), pas le volume.
