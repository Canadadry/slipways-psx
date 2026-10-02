---
title: "Plan 5, morceau 4c1 — commentaires, chaînes et locales du socle mémoire (allocateur, zones, file de sortie)"
description: "Ramener à zéro, sur les neuf modules du morceau 4c1, le français résiduel, les noms du cart (backtick et commentaire), les citations du cart et les locales cryptiques, en traduisant les messages fatals et d'assert de zone et de file de sortie à tous leurs sites lecteurs, sans changer un pixel."
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
>    - *Registre des décisions humaines.* `docs/decisions-plan-5.md` : une entrée numérotée par question (origine, question, options, nombre de sites, proposition prête à appliquer), ouverte par 4a à partir des notes de 3a–3l, complétée par chaque 4x, close par 4l. Une décision de nommage précise qu'elle s'applique aussi aux mentions entre backticks et en commentaire, avec un contrôle sur le nom remplacé.
>
> 3c. **Socle.** `engine/`, sauf l'en-tête de 3a : `allocator`, `zone`, `zone_sizes`, `frame_zone`, `game_zone`, `pointer_list`, `entity_list`, `exit_queue`, `collect`, `vec`, `geom`, `entity`, `entity_registry`, `entity_geom`, `dispatch_context`, `bootstrap`. Tests : `heap_allocator.[ch]`, `test_allocator`, `test_arena`, `test_zone`, `test_frame_zone`, `test_game_zone`, `test_pointer_list`, `test_entity_list`, `test_exit_queue`, `test_vec`, `test_geom`, `test_draw_layers_camera` ; `tools/scope_exceptions.txt` suit (`pico8_vec_t`). Résidu ≤ 314.
>
> 4c. **Commentaires — socle.** Modules et tests de 3c. Coupée par 4a (charge 1 612) :
>    - 4c1 (864) : `engine/allocator`, `engine/zone`, `engine/zone_sizes`, `engine/exit_queue`, `tests/heap_allocator`, `tests/test_allocator`, `tests/test_arena`, `tests/test_zone`, `tests/test_exit_queue`.

Et, sous l'entrée 4l : « Un morceau (4c1…) suit la PRD de sa sous-étape, avec ses commits et `make test-all` vert ; ses compteurs se lisent par `MODULES="<sa liste>"`, et `SUBSTEP=3x` à zéro clôt la sous-étape après son dernier morceau. » 4c1 ne clôt donc pas 4c : `SUBSTEP=3c` reste non nul jusqu'à 4c2.

Phase 3 du pilotage (tronçon 3 : étapes 3 et 4). Morceau 4c1 = l'entrée 4 appliquée à la première moitié de 3c, telle que la note « Découvert » de 4a l'a coupée. Liste `MODULES` de cette PRD (neuf modules) :

```
engine/allocator engine/zone engine/zone_sizes engine/exit_queue tests/heap_allocator tests/test_allocator tests/test_arena tests/test_zone tests/test_exit_queue
```

Fichiers : `src/engine/allocator.[ch]`, `src/engine/zone.[ch]`, `src/engine/zone_sizes.h`, `src/engine/exit_queue.[ch]`, `tests/heap_allocator.[ch]`, `tests/test_allocator.c`, `tests/test_arena.c`, `tests/test_zone.c`, `tests/test_exit_queue.c`. Aucune donnée de `tools/` (`tools/scope_exceptions` est à 4c2).

Entrées lues : `docs/plan-5.md` §1, §2, §4, §5, l'entrée 4 et sa méthode, la note de 3c, les notes « Découvert » de 4a et de 4b ; `docs/consigne-etape-4.md` ; `docs/decisions-plan-5.md` (D-1 à D-36) ; le skill `reprendre-les-commentaires` ; `git log --format='%h %s' -5` et `git show --stat` des trois commits de code de 4b (`98b7c6a` chaînes, `f9014c5` locales, `68e543d` commentaires et note), et la PRD de 4b (`c24d103`), modèle de forme de celle-ci.

Aucun humain n'était disponible : l'agent rédacteur a chargé lui-même `guideline` aux points « vérifier avec l'utilisateur » de `to-prd`. Les décisions reposent sur le plan et sur des mesures en lecture seule (écritures dans `tmp/` seulement : `make rename-extract` frais et compteurs, sorties dans `tmp/4c1_baseline/`).

**Condition d'entrée** : `origin/plan-5-noms-anglais` à `1449507` ou un descendant, arbre propre, `make test-all` vert ; `make rename-extract` frais sans `warning:` (les 4 lignes `partial` de `class_coverage` attendues) ; `rename-rekey-check SUBSTEP=3l` vert (0 introuvable) ; la mesure de départ ci-dessous retrouvée à l'identique.

## Problem Statement

Le socle mémoire — allocateur et poignées, zones (bump, à places, bascule), tailles de zone, file de sortie — et ses tests sont encore écrits en français : commentaires d'en-tête, messages d'`assert`, et surtout les **messages fatals** que lisent les tests (`zone %s pleine, limite %lu %s` avec `octets`/`places`, `file de sortie pleine, limite %d`). Les commentaires renvoient à des noms disparus (`pico8_fatal`, `pico8_zone_choisie.h`, `PICO8_BUMP_ZONE_HALVES`, `pico8_bascule_zone_rendre`…) et à des digrammes du cart (`hv`, `eu`, `cj`, `mh`, `ok.le`). Les noms de cas des tests à processus enfant (`bascule_stale_haut_after_rendre`, `ents_grow_keeps_order`, `he_outside_hv_or_without_member`) et les noms de zone des tests (`essai-partie`, `place essai`) sont français ou citent le cart. Un lecteur anglophone ne peut pas lire le socle sur lequel tout le jeu alloue, et les compteurs de l'étape 4 ne sont pas à zéro sur 4c1.

Le compteur de français a de plus un angle mort ici : sa liste de 106 mots-outils ne voit ni `pleine`, ni `limite`, ni `octets`, ni `essai`, ni `bascule` (homographes et mots de contenu exclus exprès par 4a). Il compte 18 `string`, alors que 99 littéraux des fichiers de 4c1 portent un mot de contenu français.

## Solution

Appliquer à 4c1 la méthode prouvée par 4a et 4b : un seul morceau, trois états de code — chaînes (rouge d'abord, tous les lecteurs d'un coup, y compris hors de 4c1), locales (aucune attendue, cf. D-37 ci-dessous), commentaires (préprocesseur identique) — puis la note « Découvert » de 4c1. L'exécutant ne commite pas : il laisse deux patchs d'états intermédiaires et trois messages dans `tmp/`, et l'arbre de travail à l'état final (§ « Remise »). À la fin, sur `MODULES=<la liste>` : français, anti-cart (`backtick`, `comment`), citations et locales cryptiques à zéro (hors proposition d'exception en attente), et plus aucun mot de contenu français à la lecture.

## User Stories

1. En tant que lecteur du socle, je veux les commentaires de `engine/allocator`, `engine/zone`, `engine/zone_sizes` et `engine/exit_queue` en anglais, afin de comprendre qui fournit le stockage, qui le libère et quand une zone s'arrête, sans traduire.
2. En tant que joueur ou testeur qui voit un arrêt fatal, je veux lire `p8_fatal: zone frame full, limit 262144 bytes` plutôt qu'un message français, afin de savoir quelle zone déborder sans dictionnaire.
3. En tant que mainteneur des tests de zone d'autres sous-étapes (`test_game_zone`, `test_ui_zone`, `test_particle_zone`), je veux que leurs chaînes attendues suivent le gabarit traduit dans le même commit, afin que `make test` reste vert après chaque commit.
4. En tant que relecteur, je veux voir chaque binaire lecteur rougir avec le seul écrivain traduit, puis verdir avec ses lecteurs, afin de savoir que chaque test lit vraiment le texte changé.
5. En tant que relecteur, je veux une table de traduction (`sed`) qui transforme chaque ligne lectrice d'avant en celle d'après, afin de vérifier que seules des étiquettes ont changé.
6. En tant que mainteneur des sous-étapes suivantes, je veux que le message de commit liste les sites lecteurs hors de 4c1 et les noms de zone laissés en français à leurs écrivains (`partie`, `chargement` à 4c2 ; `interface`, `place`, `rangee` à 4h ; `particules`, `particule` à 4f2), afin de savoir ce qui me revient.
7. En tant que lecteur d'un test à processus enfant, je veux que le nom de chaque cas soit le nom de sa fonction sans `case_`, afin de relier une ligne de sortie à son code par `grep`.
8. En tant que lecteur, je veux que les messages d'`assert` du socle disent le piège en anglais (« flush the exit queue between two game starts »), afin qu'un arrêt se comprenne sans le cart.
9. En tant que mainteneur, je veux que les tampons qui reçoivent ces textes soient vérifiés (`char message[160]` de `zone.c`, `char message[64]` de `exit_queue.c`, `char cmd[512]` des trois tests), afin qu'aucune troncature ne masque un message.
10. En tant que relecteur, je veux un état de commentaires dont la sortie de `gcc -fpreprocessed -dD -E -P` est identique à l'octet, fichier par fichier, afin de le relire sans relire le code.
11. En tant que lecteur, je veux que chaque renvoi nomme un symbole qui existe (`p8_fatal`, `engine/allocator.h`, `BUMP_ZONE_STORAGE`, `flip_zone_release`, `bump_zone_reset`), afin de le suivre par `grep`.
12. En tant que mainteneur, je veux que les pièges liés à ASan (deux moitiés de la zone bump, angle mort de la zone à bascule, arène liée sans ASan) restent dans le code, afin qu'on ne réintroduise pas un pointeur périmé qui lit sans planter.
13. En tant que mainteneur, je veux qu'aucune décision non tranchée du registre (D-14 ordre des macros de taille, D-29 paramètres `z`/`q`/`a`) ne soit appliquée, afin que l'humain garde la main.
14. En tant qu'humain du point d'arrêt, je veux que `argc`/`argv` de `main` (que la liste des locales cryptiques compte) arrivent au registre comme question prête à appliquer, afin de trancher l'idiome C une fois pour tout le dépôt.
15. En tant qu'humain du point d'arrêt, je veux que les statiques de fichier `mem`/`gfx` des tests, contraires à la ligne « Globale » du §4 mais conservées par le TSV, arrivent au registre, afin de trancher sans relire 4c1.
16. En tant qu'exécutant de 4c2, je veux la note « Découvert » de 4c1, afin d'hériter de ses sites lecteurs, de ses mots de contenu français et de ses pièges.
17. En tant qu'orchestrateur, je veux deux patchs d'états et trois messages dans `tmp/`, afin de commiter les trois sortes de commit séparément avec `make test-all` vert à chacun.

## Implementation Decisions

### Mesure de départ (`1449507`, extraction fraîche, `MODULES=<la liste>`, écrits dans `tmp/4c1_baseline/` seulement)

| Compteur | id | backtick | string | comment / détail | total |
|---|---|---|---|---|---|
| `french-count` | 0 | 0 | 18 | 694 | 712 |
| `cart-names-count` | 0 | 7 | 0 | 20 | 27 |
| `cart-citations` (cart/lua/short) | — | — | — | 0/0/123 | 123 |
| `cryptic-locals` | — | — | — | `tests/test_allocator` `argc`, `argv` (`main`, 43 lignes) | 2 |
| `scope-count` | — | — | — | — | 0 |

Charge 712 + 27 + 123 + 2 = 864, celle que 4a a écrite pour 4c1 ; 4c2 mesure 398 / 187 / 6-8-146 / 3, et les deux somment la ligne 3c de 4a. Avec `MODULES=`, les outils ne scindent pas `done`/`later` : le critère est zéro dans chaque colonne.

Détail :
- *Français* (`SITES=1`) : `comment` (messages d'`assert` compris) dans `allocator` 190, `zone` 138, `test_allocator` 117, `test_exit_queue` 98, `test_zone` 69, `exit_queue` 42, `zone_sizes` 24, `test_arena` 9, `heap_allocator` 7 ; les 18 `string` sont des diagnostics `fprintf` et `exit_queue.c:19` (`de`). **Angle mort** : les mots de contenu ne sont pas comptés. Relevé complémentaire de départ, sur les treize fichiers : `grep -owiE` des mots `pleine pleines limite octets place places essai attendu obtenu sortie inconnu propre remplie bascule rendre rendu rendue poignee arene designee vider vidage videe enfile moitie moities sonde agrandir agrandissement tete bloc cellule cellules partie chargement liste garde planter recouvert arrondi remise decompte reelles plafond entite entites` rend 335 mots (99 littéraux) — `allocator.h` 60, `test_allocator.c` 102, `test_zone.c` 43, `test_exit_queue.c` 32, `allocator.c` 29, `zone.c` 22, `zone.h` 15, `exit_queue.h` 13, `zone_sizes.h` 8, `test_arena.c` 5, `exit_queue.c` 5, `heap_allocator.h` 1. C'est une aide de lecture, pas un compteur : ses restes en fin de PRD sont des homographes anglais (`place`, `garde`…) listés un à un dans la note.
- *Anti-cart* : `comment` dans `allocator` 5, `zone` 10, `exit_queue` 1, `test_allocator`, `test_arena`, `test_zone`, `test_exit_queue` 1 chacun ; `backtick` dans `exit_queue.h` 2 (`` `rendre` ``, `` `oublier(e)` ``) et `test_exit_queue.c` 5, parmi `` `mh` ``, `` `kc` ``, `` `ju.ed` ``, `` `m_` ``, `` `ui:gw` ``, `` `gj` ``, `` `pz` ``, `` `ok.le` ``, `` `cj` ``, `` `eu()` ``, `` `dk` `` : toutes réécrites par le rôle, consigne point 2. Noms nus `pico8_*`/`PICO8_*` restants (19) : `pico8_fatal` ×6 (dont trois dans des `assert(0 && "… pico8_fatal")` de tests), `pico8_bump_zone_init`, `pico8_bump_zone_reset`, `pico8_zone_choisie[_t]`, `pico8_zone_arene_rendre`, `pico8_bascule_zone_rendre` ×2, `pico8_bascule_zone_basculer`, `PICO8_BUMP_ZONE_STORAGE`, `PICO8_SLOT_ZONE_STORAGE` ×2, `PICO8_BUMP_ZONE_HALVES` (`test_arena.c:2`). Le critère de fin y ajoute ce `grep` à 0, comme à 4b.
- *Citations* : 0 `cart`, 0 `lua` ; `short` 123 : mots français (`le` 49, `et` 30, `en` 10, `du` 9, `ou` 4, `il` 2, `lu` 1), noms de paramètres cités nus (`i` 6, `p` 3, `n` 3, `s` 1 ; `allocator.h` lignes 43, 63, 67, `test_allocator.c:32`) et digrammes du cart (`hv` 3, `eu` 1 dans le message d'`assert` de `zone.c:108`, `cj` 1).
- *Lignes de commentaire* (non vides moins préprocesseur ; lignes de code entre parenthèses) : `allocator.c` 4 (116), `allocator.h` 32 (42), `zone.c` 9 (169), `zone.h` 24 (78), `zone_sizes.h` 6 (17), `exit_queue.c` 0 (43), `exit_queue.h` 8 (10), `heap_allocator.c` 0 (21), `heap_allocator.h` 2 (5), `test_allocator.c` 17 (481), `test_arena.c` 2 (33), `test_zone.c` 11 (306), `test_exit_queue.c` 14 (288).
- *Étiquettes `TROU-N`* : aucune dans les fichiers de 4c1. Un écart nouveau se marque `/* TROU-? */` et se propose (consigne, point 4).
- *Hauteur des blocs* : aucun `.c` de 4c1 n'est cité par `tools/line_coverage_reference.txt` (`allocator.c`, `zone.c`, `exit_queue.c` absents) ; la règle de 4b ne contraint donc pas leurs blocs. `make e2e-gap` reste exigé égal à l'octet en fin de PRD.

### État 1 — chaînes (`refactor:`), rouge d'abord

**Règle de propriété.** 4c1 tient trois écrivains lus par des tests : le gabarit fatal de `zone.c` (`zone_full` : `"zone %s pleine, limite %lu %s"`, unités `"octets"` et `"places"`), le message fatal de `exit_queue.c` et les messages d'`assert` de `zone.c` et `allocator.c`. Il les traduit à **tous** leurs sites lecteurs. Il ne traduit **pas** les noms de zone écrits ailleurs : le message lu devient hybride jusqu'à leur sous-étape (« `zone chargement full, limit 65536 bytes` »), comme le veut la règle.

Traductions décidées ici :

| Écrivain | Avant | Après |
|---|---|---|
| `zone.c` `zone_full` | `zone %s pleine, limite %lu %s` | `zone %s full, limit %lu %s` |
| `zone.c`, unités | `octets`, `places` | `bytes`, `slots` (jetons `octets`→`bytes`, `places`→`slots` de `tools/rename_overrides.tsv`) |
| `exit_queue.c` | `file de sortie pleine, limite %d` | `exit queue full, limit %d` |
| `zone.c:108` `flip_zone_flip` | `bascule alors que l'ancienne generation attend son rendu : vider la file de sortie entre deux eu` | `flip while the old generation awaits its release: flush the exit queue between two game starts` (`eu` = `start_game`, digramme cité) |
| `allocator.c:30` | `zone a places : bloc plus grand qu'une place` | `slot zone: block larger than a slot` |
| `allocator.c:40` | `zone a places : une place ne grandit pas` | `slot zone: a slot does not grow` |

Les autres messages d'`assert` de `zone.c`, `allocator.c` et `exit_queue.c` (`poignee sur une zone absente`, `allocation/agrandissement/rendu sans zone designee`, `arene plus petite que sa tete`, `mesure/vidage/rendu d'une poignee qui n'est pas une arene`, `reset d'une zone a une moitie…`, `release d'un pointeur hors de la zone`, `… qui n'est pas un debut de place`, `release d'une place deja rendue`, `file de sortie : la classe n'a rien a rendre`) n'ont aucun lecteur (`grep` de chaque fragment) ; l'exécutant les traduit dans le même état, dans le vocabulaire du TSV (`handle`, `arena`, `slot`, `release`, `flush`, `flip`).

**Sites lecteurs hors de 4c1, en tête du message** (vérifiés par `grep` de `pleine`, `limite`, `octets`, `places`, `vider la file` sur `src tests tools Makefile` avant le rouge ; tout site trouvé en plus s'ajoute, comme la ligne 120 à 4b) :
- `tests/test_game_zone.c` (4c2) : ligne 410 (`expect_assert`, `"vider la file de sortie entre deux eu"` → `"flush the exit queue between two game starts"`), ligne 414 (`"zone chargement pleine, limite 65536 octets"` → `"zone chargement full, limit 65536 bytes"`) ;
- `tests/test_ui_zone.c` (4h) : lignes 157, 158, 159 (`… pleine, limite ` → `… full, limit `, `4 places` → `4 slots`) ;
- `tests/test_particle_zone.c` (4f2) : lignes 412, 413, 415, 417, 429 (idem ; `places` → `slots`).
Leurs propres textes (`attendu`, noms de cas `chargement_*`, `particule_zone_full`, messages d'`assert`) et leurs commentaires restent à leur sous-étape.

**Sites lecteurs dans 4c1** : `test_allocator.c` 542, 543, 545, 550, 558, 564 ; `test_zone.c` 349, 350-352 ; `test_exit_queue.c` 333.

**Chaînes des tests de 4c1** (écrivain et lecteur dans le même fichier) :
- *Noms de zone et d'arène des tests* : `essai-frame` → `test-frame`, `essai-places` → `test-slots`, `essai-partie` → `test-flip`, `essai-liste` → `test-list`, `place essai` → `test-arena`, `sonde` → `probe-arena` (`test_allocator.c:267`) ; `remplie` → `filled` (`test_zone.c:176` et son lecteur 352). Lecteurs : `test_allocator.c` 545, 550, 558, 564 ; `test_zone.c` 350-352.
- *Noms de cas* : règle mécanique, **le nom du cas est le nom de sa fonction sans `case_`** (précédent : 3f et 3h, noms de cas hors TSV). 21 noms, écrits dans `CASES[]` et lus par les `expect_*` de `main` (aucun lecteur hors du fichier : `grep` sur `Makefile`, `tools/`, `tests/`) :
  - `test_zone.c` (11) : `bascule_prise_follows`, `bascule_old_generation_lives`, `bascule_new_generation_after_rendre`, `bascule_stale_bas_after_rendre`, `bascule_stale_haut_after_rendre`, `bascule_twice_without_rendre`, `bascule_ends_meet`, `bascule_max_keeps_peak`, `bascule_reset_empties`, `bascule_stale_bas_after_reset`, `bascule_stale_haut_after_reset` → `flip_zone_grab_arena_follows`, … `flip_zone_stale_top_after_reset` ;
  - `test_allocator.c` (9) : `bascule_handle_round_trip`, `bascule_handle_stale_after_rendre`, `arena_vider_kills_old_block`, `ptrs_reserved_fills_without_growing`, `ptrs_reserved_overflow`, `ptrs_grow_keeps_order`, `ents_grow_keeps_order`, `ents_read_before_reset`, `ents_stale_after_reset` → `flip_zone_handle_*`, `arena_flush_kills_old_block`, `pointer_list_*`, `entity_list_*` ;
  - `test_exit_queue.c` (1) : `he_outside_hv_or_without_member` → `entity_unregister_outside_live_entities_or_without_member`.
  Contrôle : pour chaque ligne `{"x", case_y}` des trois `CASES[]`, `x == y`.
- *Diagnostics sans lecteur* : `attendu …`, `obtenu`, `sortie %d`, `cas inconnu %s`, `attendu propre` des `fprintf(stderr, …)` ; `test_arena: remettre une arene a zero doit s'arreter sur un assert` ; messages `assert(0 && "…")` des `expect_*` (dont les trois `… doit appeler pico8_fatal` → `p8_fatal`) ; messages d'`assert` des cas (`he enfile une entite de hv…`, `cj enfile la liste qu'il remplace`, `jl …`, `update60 vide la file`, `arrondi a la cellule`…), qui nomment désormais les fonctions C (`entity_unregister`, `live_entities`, `ui_clear_ephemerals`, `reset_world`, `cart_update60`). Préfixes de binaire (`test_zone:`, `test_allocator:`, `test_exit_queue:`, `test_arena:`) inchangés.

Preuves, dans le message :
- **rouge d'abord, par binaire** : les trois écrivains de `src/engine/` traduits seuls, chaque lecteur lancé seul (`make unit-run-test_X`) rougit au premier cas lu — attendu : `test_zone` à `bascule_twice_without_rendre` (349), `test_allocator` à `slot_handle_too_big` (542), `test_exit_queue` à `full` (333), `test_game_zone` à `double_start_game_without_flush` (410), `test_ui_zone` à `button_slot_full` (157), `test_particle_zone` à `burst_zone_full` (412) ; lecteurs traduits, verts. Puis, pour les noms de cas : `CASES[]` traduit seul, le premier `expect_*` d'un nom changé échoue (`cas inconnu`, sortie 2) ; lecteurs traduits, vert. Tout écart à cette prédiction est écrit dans le message.
- **table de traduction** : un `sed` (dans le message) qui, appliqué aux lignes `-` du patch des sites lecteurs, rend exactement leurs lignes `+`.
- **tampons** : `char message[160]` (`zone.c`) — le gabarit raccourcit (`full, limit` contre `pleine, limite`, `bytes`/`slots` contre `octets`/`places`) ; avec le plus long nom de zone du dépôt, `particules particles_help_screen` (32), et une limite de 20 chiffres, le message tient sous 80 octets ; `char message[64]` (`exit_queue.c`) — `exit queue full, limit 256` ; `char cmd[512]` des trois tests — le plus long nom de cas, `entity_unregister_outside_live_entities_or_without_member` (58), plus le chemin et `STDERR_LOG`, tient.
- `make test-all` vert, `tests/e2e/*.sha256` inchangés (aucun message fatal n'est dans un framebuffer).

### État 2 — locales : aucune

`cryptic-locals` ne relève que `argc` et `argv` de `main` de `tests/test_allocator.c` (portée 43 lignes). Ce sont l'idiome C de `main`, que la même liste relève aussi dans `cmd/class_coverage`, `cmd/game`, `cmd/replay` et `cmd/screencast_viewer` (10 déclarations dans le dépôt). Les renommer n'appartient pas à 4c1 seul : 4c1 ouvre **D-37** au registre (ci-dessous) et n'applique rien. Les autres locales et paramètres courts des fichiers de 4c1 (`z`, `q`, `a`, `n`, `hv`, `hd`, `mh`, `nj`) ont une portée ≤ 40 lignes et sont `conservé` (D-29 non tranchée) : non renommés, non cités entre backticks dans les commentaires réécrits. Aucun `rename-rekey`, aucune ligne de TSV ni de surcharge.

Conséquence pour la remise : `tmp/4c1_stage2.patch` est égal à l'octet à `tmp/4c1_stage1.patch` (`cmp`), et `tmp/4c1_2_msg.txt` n'est pas écrit — l'orchestrateur ne fait pas de commit 2, comme à 4a. Si un verdict humain sur D-37 arrivait avant l'exécution, l'état 2 l'appliquerait par la procédure N (extraction fraîche après l'état 1, `rename-apply`, TSV et surcharges dans le même état).

### État 3 — commentaires (`docs:`), consigne étape 4

« Fichier d'avant » : l'état 2 (= état 1), copié sous `tmp/4c1_before_comments/` avant tout agent (consigne, point 8). Un agent par module de 4c1 qui a des commentaires, en-tête et `.c` ensemble, chaque test avec son module (`allocator` + `test_allocator` + `test_arena` + `heap_allocator` ; `zone` + `zone_sizes` + `test_zone` ; `exit_queue` + `test_exit_queue`), consigne `docs/consigne-etape-4.md`, rapport au format amendé. Sans agents, l'exécutant applique la consigne lui-même et remplit le rapport.

Preuves : sortie de `gcc -fpreprocessed -dD -E -P` identique à l'octet, fichier par fichier (les branches `#ifdef ZONE_ASAN`/`#else` de `zone.c` et `zone.h` y figurent toutes deux, l'outil n'évaluant pas les `#if`) ; lignes de code inchangées ; lignes de commentaire avant/après dans le message.

Points connus, sans préjuger du rapport :
- *Contrats à garder* (destination 4) : le stockage fourni par le propriétaire de la zone (`BUMP_ZONE_STORAGE`, `SLOT_ZONE_STORAGE`, même capacité qu'à l'init) ; le nom de zone qui doit vivre autant qu'elle, puisqu'il compose le message de `p8_fatal` (`zone.h`, `allocator.h` l. 73-74) ; mémoire non initialisée valable jusqu'au prochain reset ; `zone_sizes.h` ne réserve aucune mémoire.
- *Pièges ASan à garder* (destination 3, 40 mots) : chaque reset d'une zone bump passe à l'autre moitié sous ASan (`zone.c` ~l. 60) ; la zone à bascule n'a pas de seconde moitié et son angle mort (pointeur gardé à travers deux bascules) ; `arena_take` n'appelle ni `bump_zone_init` (deux moitiés empoisonnées hors du bloc) ni `bump_zone_reset` ; `test_arena` lié sans ASan exprès, pour que la garde morde quand `BUMP_ZONE_HALVES` vaut 1 (`PICO8_BUMP_ZONE_HALVES` périmé) ; `handle` premier champ de `arena_t` (`allocator.c:103`).
- *Renvois périmés*, chacun vérifié par `grep` : `pico8_fatal` → `p8_fatal` ; `pico8_zone_choisie.h` → `engine/allocator.h` ; `pico8_zone_choisie[_t]` → `allocator[_t]` ; `pico8_bump_zone_init`/`_reset` → `bump_zone_init`/`_reset` ; `pico8_bascule_zone_rendre`/`_basculer` → `flip_zone_release`/`flip_zone_flip` ; `pico8_zone_arene_rendre` → `arena_release` ; `PICO8_BUMP_ZONE_STORAGE`, `PICO8_SLOT_ZONE_STORAGE`, `PICO8_BUMP_ZONE_HALVES` sans `PICO8_`.
- *Digrammes et noms du cart* : `hv` → `live_entities` ; `` `rendre` `` → la méthode `release` ; `` `oublier(e)` `` → `forget` ; « remet a nil » → « sets to NULL » ; `test_exit_queue.c` l. 47-48, 72, 98, 198 (`mh`, `kc`, `ju.ed`, `m_`, `ui:gw`, `gj`, `pz`, `ok.le`, `cj`, `eu()`, `dk`) réécrits par le rôle et les noms C (`message_box`, `is_hovered`, `previous_hovered`, `main_score_panel()->last_box`, `main_ui()->ephemerals`, `ui_clear_ephemerals`, `cart_start_game`, la difficulté ; `kc` et `gw`, hors TSV, par leur rôle) ; un nom encore porté par une locale conservée n'est pas mis entre backticks.
- *Noms de paramètres cités nus* (`i`, `s`, `p`, `n` d'`allocator.h` et `test_allocator.c:32`) : la phrase se réécrit (« the index », « the pointer », « the count »).
- *Macros de taille* (`zone_sizes.h`) : commentaires écrits avec les noms actuels (`ZONE_GAME_BYTES`…) ; D-14 non appliquée. « la PSX » reste (budget réel d'une cible).
- *Affirmations à revérifier en traduisant* (consigne, rubrique 7) : « la frame en cours la relit encore » et « même limite que les entités » (`exit_queue.h`) ; « une arène que plus rien ne désigne hors de la frame » ; l'ordre « `forget` puis `release` » de `exit_queue_flush` ; « rendre par la poignée ne fait rien » pour la zone à bascule.
- *Commentaires hors 4c1 qui citeront un message périmé* : `src/game/ui_zone.h:23-24` (4h), `src/game/particle_zone.h:30` (4f2), `tests/test_burst.c:784` (4i) citent « zone … pleine ». Laissés à leur sous-étape, listés dans la note.

### Ce qui reste hors de 4c1

- Noms de zone français écrits hors de 4c1 : `partie`, `chargement` (`engine/game_zone`, 4c2) ; `interface …`, `place …`, `rangee` (`game/ui_zone`, 4h) ; `particules …`, `particule` (`game/particle_zone`, 4f2) ; `rangee`, `particule` de `cmd/replay.c` (sortie `REPLAY_FILL`, 4l3, lus par `test_live_slots`, 4f1). `essai-panneau` (`test_layout_row`, 4d).
- `tools/french_count.py` et `tests/test_french_count.py` citent « zone … pleine, limite » comme exemple et fixture d'angle mort : outils de 4a, inchangés.
- Le formatage des lignes de prototype mal réindentées par l'étape 3 (`allocator.c`, `zone.c`) : du code, hors étape 4.
- D-14, D-29 : non tranchées, non appliquées.

### Registre (`docs/decisions-plan-5.md`), entrées ajoutées par 4c1

- **D-37 — `argc`/`argv` de `main` dans la liste des locales cryptiques.** Origine 4c1 (mesure). Options : (a) règle de `tools/scope_count.py --cryptic` : un paramètre `argc` ou `argv` d'une fonction `main` n'est pas cryptique (test d'abord dans `tests/test_scope_count.py`) ; (b) renommer (`arg_count`/`args`) par la procédure N ; (c) exceptions par module. Sites : 10 déclarations (5 `main` : `tests/test_allocator`, `cmd/class_coverage`, `cmd/game`, `cmd/replay`, `cmd/screencast_viewer`). Proposition : (a). Contrôle : `make cryptic-locals` sans `SUBSTEP` rend 10 candidats de moins, `make scope-count-test` vert. Ligne d'index ajoutée.
- **D-38 — statiques de fichier `mem`/`gfx` des tests.** Origine 4c1 (`tests/test_exit_queue.c` en déclare). Question : 80 déclarations `static p8_memory_t mem;` / `static p8_graphics_t gfx;` dans 40 tests, `conservé` au TSV ; le §4 veut pour une globale un nom complet, et `g_` pour une statique mutable de fichier ; 4b a renommé les locales homonymes de `test_demo.c` en `memory`/`graphics`. Options : (a) garder ; (b) `memory`/`graphics` ; (c) `g_memory`/`g_graphics` (§4 à la lettre). Sites : 80 déclarations plus leurs emplois, à mesurer par le `grep` du registre. Proposition : (b), procédure N, en un passage à l'étape 5 ; avec D-36 (b), les mentions entre backticks ne comptent pas hors de leur fichier. Ligne d'index ajoutée.
- Aucune exception de français ou de citation n'est prévue ; une ligne ne s'ajoute (format de D-33) que si un faux positif résiste à la réécriture.

### Fin de PRD

Ordre : `make rename-extract` frais sans `warning:` (4 `partial`) ; `rename-rekey-check SUBSTEP=3l` vert ; compteurs sur `MODULES=<la liste>`.

**Critère de fin** :
- `french-count` 0 dans les quatre colonnes ;
- `cart-names-count` 0 dans les quatre colonnes ;
- `cart-citations` 0/0/0 ;
- `cryptic-locals` 0 hors `argc`/`argv` de `main` (D-37 en attente), `scope-count` 0 ;
- `grep` `pico8_`/`PICO8_` des treize fichiers à 0 ;
- relevé complémentaire des mots de contenu : chaque reste est un homographe anglais, listé dans la note ;
- les trois `CASES[]` satisfont `nom == fonction sans case_`.

**Invariant (§5)** : `make test-all` vert à chacun des trois états, `tests/e2e/*.sha256` inchangés ; `make raylib` et `make screencast-viewer` compilent et linkent, jamais lancés ; `rename-tool-test` et les `rename-*-test` verts ; `class-coverage-net` et `e2e-gap` rouges, sorties égales à l'octet à celles de `1449507` (relevées au départ sous `tmp/4c1_base/`) ; `data-residue` et `composition-residue` à 0, `--detail` et `--champs` identiques ; `make sin-table` reproduit `quarter_wave.h` à l'octet.

### Note « Découvert » de 4c1 (sous l'entrée 4c de `docs/plan-5.md`, après la puce 4c2)

Elle porte : mesure de départ et de fin (dont le relevé complémentaire et ses homographes) ; sites lecteurs et preuves rouges de l'état 1, table de traduction ; l'absence de commit de locales et D-37 ; ratio et affirmations corrigées de l'état 3 ; ce qui est laissé à 4c2, 4d, 4f1, 4f2, 4h, 4i, 4l3 (noms de zone, commentaires qui citent « pleine ») ; les entrées D-37 et D-38 ; tout amendement de `docs/consigne-etape-4.md` tiré des rapports — au moins celui-ci : le compteur de français ne voit pas les mots de contenu (`pleine`, `limite`, `octets`, `essai`, `bascule`…), l'agent lit donc chaque chaîne et chaque message d'`assert` de ses fichiers et propose aussi ceux que le compteur ne marque pas ; un nom de cas de test à processus enfant suit le nom de sa fonction.

## Implementation Chunks

Un seul morceau (charge 864 ≤ 1 382). L'exécutant produit, dans cet ordre, `make test-all` vert à chaque état :
1. état 1, chaînes ; `git diff HEAD > tmp/4c1_stage1.patch`, message `tmp/4c1_1_msg.txt` (`refactor: plan 5 morceau 4c1 — messages de zone et de file de sortie en anglais, à tous leurs lecteurs`, lecteurs hors de 4c1 en tête) ;
2. état 2, locales (vide) ; `git diff HEAD > tmp/4c1_stage2.patch`, égal à l'état 1 ; pas de `tmp/4c1_2_msg.txt` ;
3. état 3, commentaires, note « Découvert », amendements de la consigne et du registre, laissés dans l'arbre de travail ; message `tmp/4c1_3_msg.txt` (`docs: plan 5 morceau 4c1 — commentaires du socle mémoire en anglais, note Découvert`).

## Remise

L'exécutant ne commite pas. Il laisse :
- `tmp/4c1_stage1.patch` : diff complet de `HEAD` à l'état 1 (s'applique par `git apply` sur `HEAD` propre) ;
- `tmp/4c1_stage2.patch` : diff complet de `HEAD` à l'état 2, cumulatif, ici égal à l'état 1 ;
- `tmp/4c1_1_msg.txt`, `tmp/4c1_3_msg.txt` : messages de commit en français, récit de chantier et preuves compris (destination « message de commit » du skill), terminés par les lignes d'attribution de la session ;
- l'arbre de travail à l'état 3 : commentaires, note, consigne et registre ;
- un rapport final qui dit en une ligne qu'il n'y a pas de commit 2.

## Testing Decisions

- Aucun test neuf, aucune assertion ajoutée ni modifiée dans sa condition (consigne, points 5 et 7). Le filet : les tests à processus enfant qui lisent les messages (`test_zone`, `test_allocator`, `test_exit_queue`, `test_game_zone`, `test_ui_zone`, `test_particle_zone`), prouvés lecteurs par le rouge d'abord ; le compilateur ; la sortie du préprocesseur pour les commentaires.
- Les binaires ASan (`test_zone`, `test_allocator`, `test_exit_queue`, `test_game_zone`, `test_ui_zone`, `test_particle_zone`) et le binaire sans ASan (`test_arena`) passent tous par `make test` : les deux valeurs de `BUMP_ZONE_HALVES` restent couvertes.
- Un bon contrôle juge une sortie observable : texte lu sur stderr, statut de sortie, `*.sha256` des e2e.
- Prior art : `98b7c6a` (lecteurs hors sous-étape listés d'abord, rouge d'abord, `sed` de traduction), `68e543d` (préprocesseur fichier par fichier), et la note de 3f (noms de cas et messages d'`expect_fatal` composés).

## Out of Scope

- Toute ligne de code hors des chaînes et messages de l'état 1.
- Les noms de zone, noms de cas, diagnostics et commentaires des autres sous-étapes, même quand ils lisent le gabarit de 4c1 (seule la partie écrite par 4c1 change à leurs sites).
- `TROU-N` → `GAP-N` (étape 5) ; toute entrée du registre sans verdict (D-14, D-29, D-36, D-37, D-38).
- Le formatage des prototypes, l'outil `french_count.py` et sa liste de mots.

## Further Notes

- `slots` pour l'unité de la zone à places suit le jeton de `tools/rename_overrides.tsv` et le nom du type (`slot_zone_t`) ; 4h et 4f2 traduiront leur préfixe `place …` en cohérence (proposition pour eux : `slot …`).
- Le message hybride (« `zone chargement full, limit …` ») est voulu : chaque moitié du texte a un écrivain et une sous-étape.
