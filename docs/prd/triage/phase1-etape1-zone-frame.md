---
title: "Plan 4, étape 1 — Zone frame : textes et petites listes de la frame"
description: "Les textes, listes filtrées et tableaux de travail qui ne vivent qu'une frame sont encore pris à la libc puis libérés à la main : l'étape 1 instancie la zone frame, la remet à zéro au début de chaque frame, y range ces sites et retire leurs free."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. Les étapes 0a et 0b ont posé le socle :

- 0a (finie) : `pico8_fatal`, la zone à pointeur montant et la zone à places avec leur empoisonnement ASan, le plafond de 1,5 Mo, le verrou à cliquet (`alloc-guard`, égalité stricte) ;
- 0b (en cours) : la zone devient un paramètre choisi par l'appelant. Tous les appelants passent pour l'instant la **zone transitoire**, adossée à la libc, et chaque référence à cette zone compte dans le cliquet.

Aucune zone du jeu n'existe encore. Or une bonne part des allocations du cœur ne vit qu'**une frame**, voire seulement le temps d'un appel :

- des textes intermédiaires : un `tostr` concaténé puis libéré, les morceaux `"level " .. tl .. " (" ..` du menu de recherche ;
- des listes filtrées : `jp`, la concaténation d'entités, les calques de `dc` ;
- des tableaux de travail : les `bs`/`rows` passés à `pz`, la rangée `l` de `jo.ex`.

Chacun est pris à la libc puis rendu par un `free` écrit à la main. Ces `free` sont autant d'occasions d'erreur, et le compte du cliquet ne baisse pas.

## Solution

L'étape 1 instancie la **zone frame** et y range tous les sites dont l'objet meurt avant la fin de la frame. Leurs `free` disparaissent.

- **Instance.** Une zone à pointeur montant de `PICO8_ZONE_FRAME_BYTES` (256 Ko), nommée `frame`. Un petit module du cœur la possède.
  - Il expose la **poignée désignée** de 0b sur cette zone. Les appelants convertis la passent à la place de la transitoire.
  - Il expose aussi la **remise à zéro**.
  - La zone s'initialise d'elle-même au premier usage. Un test unitaire qui appelle une fonction du jeu sans `pico8_cart_load()` la trouve donc prête.
- **Remise à zéro au début de chaque frame**, en tête de `pico8__update60()`.
  - C'est le seul point par lequel passent tous les pilotes :
    - le rejeu (`main.c`) ;
    - le binaire raylib (`main_raylib.c`) ;
    - la couverture de classes (`main_class_coverage.c`) ;
    - les tests qui bouclent `update60`/`draw`.
  - Aucun pilote ne change.
  - Sous ASan, la remise à zéro empoisonne la moitié quittée (0a). Un objet de frame gardé jusqu'à la frame suivante fait planter `make sanitize-net`. C'est l'oracle du §3.
- **Conversion des sites.** Chaque site de l'inventaire ci-dessous passe la poignée frame au lieu de la transitoire, ou alloue dans la zone frame au lieu de la libc. Le `free`, le `pico8_ents_free` ou le `pico8_ptrs_free` correspondant est supprimé.
- **Pas de changement visible.** Les `*.sha256` de `tests/e2e/` restent identiques, et `make test-all` et `make sanitize-net` passent à chaque commit.
- **Cliquet.** Le compte de chaque fichier concerné baisse exactement du nombre de références à la transitoire et d'appels libc que l'étape retire. `src/pico8_pi.c` et `src/pico8_economy_score.c` sortent de la liste.
- **Correction du §2 du plan.** Le tableau range les « petites boîtes (`vec`) » en zone frame. La lecture montre que les boîtes `vec` du cœur sont presque toutes des champs d'objets durables (`ra`, `p`, `v`, `qf`, `qa`), remplacés à chaque tick et relus à la frame suivante. Ce ne sont donc pas des objets de frame (règle du §4). L'étape corrige le tableau dans son dernier commit, et sa note « Découvert » le dit.

## User Stories

1. En tant que mainteneur du plan, je veux une zone frame de taille fixe instanciée dans le cœur, pour que les objets d'une frame ne demandent plus rien à la libc.
2. En tant que mainteneur, je veux que la zone frame soit remise à zéro en tête de `pico8__update60()`, pour que tous les pilotes (rejeu, raylib, couverture, tests) la remettent à zéro au même instant sans être modifiés.
3. En tant que développeur qui chasse un accès hors durée de vie, je veux qu'un pointeur pris dans la zone frame puis lu après le `pico8__update60()` suivant plante sous ASan, pour qu'un site mal classé se révèle dans `make sanitize-net` au lieu de corrompre la mémoire.
4. En tant qu'auteur de tests unitaires, je veux que la zone frame s'initialise d'elle-même au premier usage, pour qu'un TU qui appelle `pico8_cr` ou `pico8_jo_gh` sans charger le cart n'ait rien à préparer.
5. En tant qu'auteur de tests unitaires, je veux pouvoir remettre la zone frame à zéro à la main, pour qu'un test qui boucle longtemps sans `update60` ne la remplisse pas et pour prouver qu'un panneau survit à la remise à zéro.
6. En tant que développeur, je veux que la zone pleine sorte par `pico8_fatal` avec « zone frame pleine, limite 262144 octets », pour qu'une limite trop basse se lise et se relève en un chiffre.
7. En tant que mainteneur du budget, je veux que relever la limite de la zone frame s'arrête et le signale si la somme dépasse 1,5 Mo, pour que le plafond PSX ne bouge pas en silence.
8. En tant que développeur du menu de recherche (`pico8_cr`), je veux que les textes intermédiaires `level`, `(`, `/` et les `tostr` qui les composent soient faits en zone frame, pour supprimer les neuf `free` à la main de cette fonction.
9. En tant que développeur, je veux que les tableaux `bs`/`rows` passés à `pz` par `pico8_cr` et `pico8_jo_fe` soient pris dans la zone frame, pour supprimer leurs `calloc`/`free`, puisque `pz` en recopie tout avant de rendre la main.
10. En tant que développeur, je veux que la rangée de travail `l` de `jo.ex` soit prise dans la zone frame, pour supprimer son `malloc`/`free`, puisque `lq_new` en recopie les cellules.
11. En tant que développeur, je veux que les listes filtrées jetées dans la même fonction (`pico8_cz`, `pico8_eg`, `pico8_bv`, `pico8_jc_ce`, `pico8_jo_gh`) soient faites en zone frame, pour supprimer leurs `pico8_ents_free`/`pico8_ptrs_free`.
12. En tant que développeur, je veux que les quinze calques locaux de `pico8_dc` vivent en zone frame, pour que le dessin de chaque frame ne fasse plus de `realloc`.
13. En tant que développeur, je veux que les listes de travail de `pico8_trade_kg` (`empty_lp`, `empty_rl`, `np_list`, `jc_list`) et le `prefix` de `pico8_jo_cn` soient faits en zone frame, pendant que les listes qu'ils produisent restent des données de partie.
14. En tant que développeur, je veux que le texte numérique d'un identifiant (`tostr(id)` dans `pico8_bw` et `pico8_ie_ldown`) soit fait en zone frame, alors que l'identifiant concaténé `"p"..id` / `"s"..id` reste hors de cette étape, pour qu'un identifiant de partie ne meure jamais à la frame suivante.
15. En tant que développeur, je veux que le texte du `print` d'un nombre (`pico8_pi_io_draw`) soit fait en zone frame, pour que `pico8_pi.c` sorte de la liste d'exceptions.
16. En tant que développeur, je veux que les textes de `pico8_dl_show` (prix d'une sonde) soient faits en zone frame, pour supprimer ses trois `free`.
17. En tant que développeur, je veux que les textes passés à `jo_dollar` (`ne`, `tostr`) soient faits en zone frame, pour que `jo_dollar` ne libère plus son argument, alors que le texte qu'il rend reste lié au panneau.
18. En tant que développeur, je veux que les `tostr` intermédiaires de la jointure de `hp:jb` et de `hp:jb` lui-même soient faits en zone frame, pour supprimer leurs `free`.
19. En tant que développeur, je veux que les `tostr` intermédiaires de `pj:hb` (`nz`, `+po`, `happiness`) et de `pj:hk` (`year`), et les morceaux `piece1`/`piece2` de `pj:ph`, soient faits en zone frame, pour supprimer leurs `free`.
20. En tant que développeur, je veux que `pico8_ky` fasse son texte `happiness` intermédiaire et son résultat `%` en zone frame, puisque son seul appelant le recopie aussitôt, pour que `pico8_economy_score.c` sorte de la liste.
21. En tant que développeur, je veux que les tableaux de cellules de `jo:gh` et la rangée « n invented » du menu soient faits en zone frame, puisque `lq_new` et `jb_new` les recopient, pour qu'une fuite TROU-7 de frame disparaisse au lieu d'être portée dans une zone plus longue.
22. En tant que mainteneur, je veux qu'un site dont la lecture ne prouve pas que l'objet meurt dans la frame reste hors de l'étape, et passe à la zone la plus longue qui le couvre (§4), pour ne jamais raccourcir une durée de vie au jugé.
23. En tant que mainteneur, je veux qu'un site converti qui fait planter `make sanitize-net` revienne à la transitoire, et soit noté dans « Découvert » comme classé autrement par crash ASan, pour que l'oracle tranche sans qu'on relève une limite.
24. En tant que gardien du cliquet, je veux que chaque commit de l'étape baisse le compte de chaque fichier touché exactement du nombre mesuré par `alloc-guard`, et que son message liste ces baisses, pour que l'égalité stricte reste lisible.
25. En tant que gardien du cliquet, je veux qu'à la fin de l'étape `pico8_pi.c` et `pico8_economy_score.c` aient quitté la liste d'exceptions, pour que le verrou interdise toute nouvelle allocation libc dans ces deux fichiers.
26. En tant que gardien de l'invariant, je veux que les `*.sha256` de `tests/e2e/` restent identiques à chaque commit, pour prouver que changer de zone ne change pas un pixel.
27. En tant que lecteur du plan, je veux que l'étape corrige la ligne « Frame » du tableau du §2 (les boîtes `vec` remplacées à chaque tick n'y sont pas), pour que les étapes 2 et 4 ne les y cherchent pas.
28. En tant que développeur des étapes 2 à 4, je veux que la note « Découvert » liste les sites écartés de la zone frame et pourquoi, pour savoir lesquels me reviennent.
29. En tant que mainteneur des commentaires, je veux que les `/* TROU-7 */` des sites dont la fuite disparaît soient retirés, et les contrats « l'appelant libère » réécrits pour les fonctions converties, pour que les commentaires restent vrais.
30. En tant que futur porteur PSX, je veux que la zone frame n'ajoute aucune dépendance à un pilote, pour qu'un pilote PSX n'ait qu'à appeler `pico8__update60()`.

## Implementation Decisions

- **Module de la zone frame.** C'est un petit module du cœur, dans `CORE_SRCS` et hors de la liste d'exceptions.
  - Il possède le stockage `PICO8_BUMP_ZONE_STORAGE` de `PICO8_ZONE_FRAME_BYTES` et le `pico8_bump_zone_t` nommé `"frame"`.
  - Il expose deux fonctions : l'accès à la poignée désignée sur cette zone, et la remise à zéro.
  - **Initialisation paresseuse.** L'accès initialise la zone au premier appel. Un initialiseur statique ne suffit pas : `pico8_bump_zone_init` empoisonne le stockage sous ASan.
  - La remise à zéro initialise d'abord la zone si besoin, puis appelle `pico8_bump_zone_reset`.
  - Le module ne touche pas `pico8_zone_sizes.h` : la taille existe déjà et compte déjà dans le plafond.
  - Il entre dans toutes les clôtures d'objets qui lient un module converti : `DATA_ENGINE_SRCS`, les listes de chaque TU, `raylib`, et la couverture de classes.
- **Point de remise à zéro.** Il est en tête de `pico8__update60()`, avant `pico8_dg()`.
  - `pico8_cheat_step`, appelé juste avant par les pilotes, n'alloue pas.
  - `pico8__draw()` suit `update60` dans la même frame : un objet fait pendant `update` et lu pendant `draw` est valide.
  - Pas de remise à zéro dans `pico8_cart_load`, `pico8__init` ni `pico8_cart_shutdown`. Les sites convertis rendent leur objet avant le retour de la fonction qui l'a fait, ou le font recopier avant ce retour. Ils sont donc corrects même appelés hors de `update60`/`draw`.
- **Critère de classement.** Il précise le §4 pour cette étape.
  - **Sûr** : l'objet est libéré (`free`, `*_free`) dans la fonction qui l'a fait, ou dans celle qui le reçoit aussitôt (`jo_dollar`). Ces sites sont convertis.
  - **Prouvé par lecture** : l'objet n'est libéré nulle part aujourd'hui (fuite TROU-7), mais son unique consommateur le recopie avant de rendre la main. Le seul cas est un tableau de cellules ou une rangée `jb`, que `lq_new`, `lq_copy` et `jb_new` (lignes de `jb_line`) recopient. Ces sites sont convertis, chacun avec un test qui remet la zone à zéro après la construction puis exerce le panneau.
  - **Douteux** : l'objet est rangé dans un champ d'un objet durable, d'un panneau ou d'un état global, ou il est référencé par pointeur dans une cellule (`pico8_lq_cell_row`, `pico8_lq_cell_string`). Ces sites restent à la transitoire et reviennent aux étapes 2 à 4.
- **Les `free` disparaissent.** Pour un texte ou un tableau fait en zone frame, la ligne `free` est supprimée. Pour une liste `ents`/`ptrs` faite en zone frame, l'appel `pico8_ents_free`/`pico8_ptrs_free` est supprimé aussi : il ne ferait rien, et le garder laisserait croire qu'il rend quelque chose.
- **Fabricants locaux.** Deux fabricants locaux font leur résultat en zone frame : `concat_percent` (`pico8_ky`) et le bloc de `pico8_hp_jb`. Ils prennent la poignée frame directement. Ils ne gagnent pas de paramètre de zone : ils n'ont qu'un appelant et une seule durée de vie.
  - `jo_dollar` ne change pas de zone pour son résultat, qui reste lié au panneau. Il perd seulement son `free(s)`, puisque ses deux appelants passent la zone frame.
  - `jo_gh_row_items` et `pico8_menu_invented_lines` prennent aussi la zone frame.
- **Contrats C.** Les en-têtes des fonctions publiques dont le résultat passe en zone frame disent « valable jusqu'à la prochaine remise à zéro de la zone frame » au lieu de « never freed » / « caller frees ». Cela concerne `pico8_hp_jb` et `pico8_ky` (champ `happiness`).
- **Commentaires.** Ils suivent les quatre destinations de `CLAUDE.md`.
  - Les `/* TROU-7 */` qui marquent une fuite supprimée sont retirés : `jo_gh_row_items`, `pico8_menu_invented_lines`, et `result.happiness` de `pico8_ky`.
  - L'entrée TROU-7 de `docs/trous-de-portage.md` reste, jusqu'à l'étape 5.
  - Le pourquoi de la remise à zéro en tête de `update60` est un piège qu'on réintroduirait (la déplacer dans un pilote). Il tient en un commentaire court.
- **Limite.** Elle reste à 256 Ko au départ. Un `pico8_fatal` « zone frame pleine » dans un e2e fait relever `PICO8_ZONE_FRAME_BYTES`, dans la limite de 1,5 Mo de somme (§3). Si la somme dépasse, l'étape s'arrête et le signale : zone, limite voulue, somme actuelle.
  - Un TU qui remplit la zone parce qu'il boucle sans `update60` appelle la remise à zéro entre ses itérations. On ne relève pas la limite pour lui.
- **Correction du §2.** Dans la ligne « Frame » du tableau, « petites boîtes (`vec`, listes enveloppées) » devient « listes filtrées et tableaux de travail ». Les boîtes `vec` rangées dans un champ y sont renvoyées à la zone de leur propriétaire. Cette correction se fait dans le commit qui ajoute la note « Découvert ».

### Inventaire des sites — convertis à l'étape 1

Les sites sont repérés par fonction, à `HEAD` 6731f01, avant 0b. « réf. T » désigne une référence à la poignée transitoire après 0b (hypothèse H-0b-3). « libc » désigne un appel `malloc`/`calloc`/`realloc`/`free` restant.

**Sûrs** (objet rendu dans la fonction, ou par son récepteur immédiat) :

| Fichier | Fonction | Objet | Baisse |
|---|---|---|---|
| `pico8_menu.c` | `pico8_cr` | `tostr` de `gr` + son `free` | 2 (1 réf. T + 1 libc) |
| | | `tl_str`, `diff1_str`, `diff2_str` (`tostr`) + leurs 3 `free` | 6 (3 + 3) |
| | | `tmp1`, `tmp2`, `tmp3`, `tmp1`, `tmp2` (concaténation partagée) + leurs 5 `free` | 10 (5 + 5) |
| | | `rows`, `bs` (`calloc`) + leurs 2 `free` | 4 libc |
| `pico8_planet_display.c` | `pico8_jo_gh` | `ne` passé à `jo_dollar` | 1 réf. T |
| | | `ctx.ji` (`ptrs_init`) + `jx` (`jp_ptrs`), et leurs `ptrs_free` | 2 réf. T |
| | | `count_str`, `mu_str` de « exports » (`tostr`) + leurs 2 `free` | 4 (2 + 2) |
| | `pico8_jo_fe_keys` | `tostr(od)` passé à `jo_dollar` | 1 réf. T |
| | `jo_dollar` | `free(s)` de son argument | 1 libc |
| | `pico8_jo_fe` | `bs`, `rows` (`calloc`) + leurs 2 `free` | 4 libc |
| `pico8_research_display.c` | `pico8_hp_jb_join` | `num_str` (`tostr`) + `free` | 2 (1 + 1) |
| | `pico8_hp_jb` | `o_str` (`tostr`) + `free` | 2 (1 + 1) |
| | `pico8_jo_ex` | `l` (`malloc`) + `free` | 2 libc |
| `pico8_score_panel.c` | `pico8_eg` | `filtered` (`jp_ents`), son `ents_free` | 1 réf. T |
| | `pico8_pj_hb` | `nz_str`, `plus_num`, `happiness_str` (`tostr`) + 3 `free` | 6 (3 + 3) |
| `pico8_score_panel_display.c` | `pico8_pj_hk` | `yr_str` (`tostr`) + `free` | 2 (1 + 1) |
| | `pico8_pj_ph_map` | `piece1`, `piece2` (concaténation) + 2 `free` | 4 (2 + 2) |
| `pico8_probe.c` | `pico8_bv` | `filtered` (`jp_ents`), son `ents_free` | 1 réf. T |
| | `pico8_dl_show` | `od_str` (`tostr`) + `free` ; `heap_display` (`malloc`) + `free` | 4 (1 réf. T + 3 libc) |
| `pico8_slipway.c` | `pico8_jc_ce` | `part1`, `part2` (`jp_ents`), `ds` (`ents_cat`), leurs 3 `ents_free` | 3 réf. T |
| `pico8_trade.c` | `pico8_trade_kg` | `empty_lp`, `empty_rl`, `np_list`, `jc_list` (`ents_init`), 2 `ents_free` | 4 réf. T |
| `pico8_planet_trade.c` | `pico8_jo_cn` | `prefix` (`ptrs_init`), son `ptrs_free` | 1 réf. T |
| `pico8_entity.c` | `pico8_dc` | `layers[15]` (`ents_init` en boucle), ses `ents_free` | 1 réf. T |
| `pico8_galaxy.c` | `pico8_bw` | `id_num_str` (`tostr`) + `free` | 2 (1 + 1) |
| `pico8_research_icon.c` | `pico8_ie_ldown` | `id_num_str` (`tostr`) + `free` | 2 (1 + 1) |
| `pico8_pi.c` | `pico8_pi_io_draw` | `owned` (`tostr`) + `free` | 2 (1 + 1) |
| `pico8_economy_score.c` | `pico8_cz` | `filtered` (`jp_ents`), son `ents_free` | 1 réf. T |
| | `pico8_ky` | `happiness_str` (`tostr`) + `free` | 2 (1 + 1) |

**Prouvés par lecture** (fuite TROU-7 aujourd'hui, recopiés par leur unique consommateur avant le retour) :

| Fichier | Fonction | Objet | Consommateur qui recopie | Baisse |
|---|---|---|---|---|
| `pico8_planet_display.c` | `jo_gh_row_items` | tableaux de cellules des rangées 2..n de `jo:gh` | `pz` → `js_new` → `lq_new` recopie `keys->items` | 1 libc |
| `pico8_menu.c` | `pico8_menu_invented_lines` | rangée `jb` « n invented » | `pz` → `js_new` → `jb_new` (`jb_line` recopie chaque ligne) | 1 libc |
| `pico8_research_display.c` | `pico8_hp_jb` | bloc `jb` (`calloc`) | `jo_fe` → `pz` → `jb_new` ; le texte de la jointure, lui, reste référencé et hors étape | 1 libc |
| `pico8_economy_score.c` | `concat_percent` | `result.happiness` de `pico8_ky` | seul appelant, `pico8_pj_ph`, le recopie par `pj_concat2(s->happiness, "")` | 1 libc |

### Baisse attendue du cliquet, par fichier

Les comptes « après 0b » sont l'hypothèse H-0b-6, estimée sur les sources. Le compte réel se lit sur objets, avec `alloc-guard`, une fois 0b finie.

| Fichier | Avant 0b (`HEAD`) | Après 0b (H-0b-6) | Baisse étape 1 (sûrs + lecture) | Reste |
|---|---|---|---|---|
| `pico8_menu.c` | 15 | 26 | 22 + 1 | 3 |
| `pico8_planet_display.c` | 11 | 18 | 13 + 1 | 4 |
| `pico8_research_display.c` | 8 | 11 | 6 + 1 | 4 |
| `pico8_score_panel.c` | 5 | 12 | 7 | 5 |
| `pico8_score_panel_display.c` | 7 | 18 | 6 | 12 |
| `pico8_probe.c` | 7 | 9 | 5 | 4 |
| `pico8_trade.c` | 3 | 9 | 4 | 5 |
| `pico8_slipway.c` | 4 | 8 | 3 | 5 |
| `pico8_economy_score.c` | 2 | 4 | 3 + 1 | 0 — sort |
| `pico8_galaxy.c` | 4 | 5 | 2 | 3 |
| `pico8_research_icon.c` | 4 | 5 | 2 | 3 |
| `pico8_pi.c` | 1 | 2 | 2 | 0 — sort |
| `pico8_planet_trade.c` | 4 | 9 | 1 | 8 |
| `pico8_entity.c` | 0 (absent) | 3 | 1 | 2 |
| **Total** | | | **77 + 4 = 81** | |

La règle de fin est celle du plan : chaque site de l'inventaire est converti, et chaque compte a baissé d'autant. Si la mesure sur objets diffère des estimations, c'est elle qui fait foi. Par exemple, un compilateur peut charger une seule fois l'adresse de la poignée dans un registre pour plusieurs appels d'une même fonction. L'écart est écrit dans le message de commit et dans « Découvert ».

### Sites écartés de la zone frame (reviennent aux étapes 2 à 4)

Ces sites restent à la transitoire ou à la libc. Chacun est listé avec sa raison, pour la note « Découvert ».

- **Textes rangés dans un objet durable ou une cellule** :
  - `ne` dans `pico8_hy_lu` et `pico8_jo_hk` : le texte est porté par `iv` ;
  - `gi_str`, `label` et `ep` de `pico8_cr` ;
  - le résultat de `jo_dollar`, `combined` et le `mu_str` de « makes » dans `jo:gh` ;
  - la jointure de `hp:jb` et `jq_str` de `jo.ex` ;
  - `cf` et les concaténations de `pj:hb` : rangées dans `ou_state`, relues aux frames suivantes ;
  - `year_label`, `s_str` (tous les `tostr`/concaténations de `pico8_pj_ph_s_str`) et `label` de `pj:ph` ;
  - `jo_kw_tuple` et les tuples de `jo.ex` : référencés par pointeur dans une cellule.
- **Identifiants de partie** : `"p"..id` (`pico8_bw`) et `"s"..id` (`pico8_ie_ldown`). Ils reviennent à l'étape 4.
- **Boîtes `vec` remplacées à chaque tick et relues à la frame suivante** :
  - `mo->v` dans `pico8_bg` ;
  - `p`, `v` et `ra` de `qb` ;
  - `ra` de `nf` ;
  - `ra` de `iv` (`pq`, `ph`) ;
  - `ra` de `js:ph` ;
  - `qa` de `ui` ;
  - `re` et son `p` dans `jc:hq` ;
  - `p` dans `pico8_bv` ;
  - `ie.gy` ;
  - les `qf`/`ra` de `pz`, et `spec*.ra` de `pj:ph`.
- **Listes rangées dans un objet** :
  - `le` de `pz` : le même site est rangé dans `ok->le` ou libéré selon la branche, donc on prend la zone la plus longue ;
  - les calques `ic` de `ui`, rebâtis par `gw` mais relus hors de la frame qui les bâtit, ce qui n'est pas prouvé ;
  - `ox` de `mz` ;
  - `cc`, `oe`, `jy`, `by`/`ib`, `lp`/`rl` ;
  - les listes produites par `ents_cat` dans `trade_kg` et par `la_ptrs` dans `jo_cn`.
- **État de générateur `oh` de `pico8_bg_pq` (`pico8_jf`)** : il est fait et rendu dans la même fonction, mais par `pico8_jf`, que 0b ne paramètre pas (H-0b-9). Il revient à l'étape 4, qui donnera une zone aux états de `pico8_gen.c`.

## Implementation Chunks

Chaque morceau est un commit qui respecte l'invariant du §3 et remet la liste d'exceptions à égalité stricte. Son message dit les baisses par fichier.

1. **Zone frame et remise à zéro.**
   - Le module de la zone frame, la remise à zéro en tête de `pico8__update60()`, l'ajout du module aux clôtures d'objets.
   - Le TU ASan de la zone frame (voir Testing Decisions).
   - Aucun site n'est converti, et aucun compte ne bouge.
2. **Listes de frame.** Les sites `jp_ents`, `jp_ptrs`, `ents_cat` et `ents_init`/`ptrs_init` sûrs :
   - `pico8_cz`, `pico8_eg`, `pico8_bv`, `pico8_jc_ce`, `pico8_trade_kg`, `pico8_jo_cn` ;
   - `ctx.ji`/`jx` de `jo:gh` ;
   - `pico8_dc`.

   Leurs `*_free` disparaissent. `pico8_entity.c` descend.
3. **Textes de frame.** Les `tostr`, `ne` et concaténations sûrs, et leurs `free` : menu, `jo:gh`/`jo:fe`/`jo_dollar`, `hp:jb`, `pj:hb`, `pj:hk`, `pj:ph_map`, `dl_show`, `bw`, `ie_ldown`, `pi_io_draw`, `ky`. `pico8_pi.c` sort de la liste.
4. **Tableaux de travail.** `rows`/`bs` de `pico8_cr` et de `pico8_jo_fe`, et `l` de `jo.ex`.
5. **Sites prouvés par lecture.** `jo_gh_row_items`, `pico8_menu_invented_lines`, le bloc de `pico8_hp_jb` et `concat_percent`.
   - Chacun a son test de survie à la remise à zéro.
   - Leurs `/* TROU-7 */` sont retirés.
   - `pico8_economy_score.c` sort de la liste.
6. **Clôture de l'étape.**
   - La note « Découvert » sous l'entrée 1 du plan : sites classés autrement que dans le §2, limite relevée ou non, comptes laissés.
   - La correction de la ligne « Frame » du §2.

Un site qui fait planter `make sanitize-net` dans les morceaux 2 à 5 revient à la transitoire dans le même commit. Il est noté pour « Découvert ».

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur :
  - un texte rendu, le contenu d'une liste, le dessin d'un panneau ;
  - un code de sortie, un rapport ASan ;
  - la sortie d'`alloc-guard`, l'empreinte d'un e2e.

  Il ne regarde ni l'adresse d'un bloc ni le remplissage interne de la zone.
- **TU ASan de la zone frame**, construit comme `tmp/test_pico8_zone` : règle avec `asan_objs`, entrée dans `UNIT_RUNS_SPECIAL`, `ASAN_OPTIONS=detect_leaks=0`. Cas obligatoires :
  - **Survie dans la frame.** Un bloc pris par la poignée frame reste lisible tant qu'il n'y a pas de remise à zéro.
  - **Mort à la frame suivante.** Un bloc pris par la poignée frame, puis lu après **une** remise à zéro et de nouvelles allocations, plante (`use-after-poison`) dans un processus enfant. Le test s'en tient à une remise à zéro : 0a a établi qu'après deux, la moitié empoisonnée redevient valide.
  - **Branchement sur `update60`.** Le même pointeur, gardé à travers un `pico8__update60()` réel, cart chargé comme dans les TU qui bouclent `update60`/`draw`, plante dans un processus enfant. C'est ce cas qui prouve que la remise à zéro est câblée, pas seulement possible.
  - **Initialisation paresseuse.** Le premier accès, sans `pico8_cart_load()`, rend une poignée utilisable.
  - **Zone pleine.** Un débordement sort avec le code 3, et `stderr` contient « zone frame pleine ».
- **Sites prouvés par lecture** : un cas par site, dans le TU ASan ci-dessus ou dans un TU existant passé en règle ASan.
  - On construit le panneau (`jo:gh`, le menu `cr` avec des technologies inventées, `jo:fe` qui appelle `hp:jb`) ou l'on lit `pico8_ky`.
  - On remet la zone frame à zéro, puis on alloue de nouveau.
  - On exerce alors le résultat sans rapport ASan : dessiner le panneau, ou relire la copie du texte « % ».
  - Si l'un plante, le site va dans les écartés.
- **Tests existants.** `test_pico8_hp`, `test_pico8_lq`, `test_pico8_jb_ol`, les TU du menu, de `pj`, de la sonde et des échanges restent verts sans changement de leurs attentes. Seul ajout possible : une remise à zéro explicite dans un TU qui remplirait la zone en bouclant sans `update60`.
- **Verrou.** `make alloc-guard` (dans `make test`) à égalité stricte, après chaque morceau. Pas de nouveau cas dans `tests/test_alloc_guard.c` : le format et les symboles comptés ne changent pas.
- **Invariant.** `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net` sous l'empoisonnement de la zone frame. C'est l'oracle qui tranche un site mal classé.
- **Exemples dans le dépôt** :
  - `tests/test_pico8_zone.c` et sa règle ASan ;
  - `tests/test_pico8_replay_usage.c` (processus enfant, statut et `stderr`) ;
  - les TU qui bouclent `pico8__update60()`/`pico8__draw()` (`test_pico8_bg`, `test_pico8_ga`, `test_pico8_lq`) pour le cas « branchement sur `update60` ».

## Out of Scope

- Les sites écartés listés plus haut : objets de panneau, messages, boutons et boîtes `vec` rangées dans un champ (étape 2), particules (étape 3), objets et identifiants de partie et états de `pico8_gen.c` (étape 4).
- Une poignée sur zone à places, la zone interface, particules ou partie.
- Toute modification du module de zone désignée, de la zone transitoire ou du script du verrou. Ils sont livrés par 0b.
- Donner un paramètre de zone à `pico8_jf`/`pico8_gn` ou à un autre fabricant partagé que 0b n'a pas paramétré.
- Retirer l'entrée TROU-7 de `docs/trous-de-portage.md` (étape 5).
- Les autres appels libc du cœur (§7 du plan).
- Lancer l'application raylib.

## Hypothèses sur l'étape 0b

0b n'est pas finie au moment où cette PRD est écrite. Tout ce qui suit vient de sa PRD (`docs/prd/triage/phase1-etape0b-zones-choisies-par-l-appelant.md`), pas du code.

- **H-0b-1 (poignée).** 0b livre un type de poignée désignée (allouer, agrandir, rendre) et un moyen de construire une poignée sur un `pico8_bump_zone_t` déjà initialisé.
  - Sur cette poignée, rendre ne fait rien et agrandir recopie.
  - Les noms ne sont pas fixés (« le nom définitif est laissé à l'implémenteur »). Cette PRD écrit « poignée frame » et « poignée transitoire ».
- **H-0b-2 (transitoire et cliquet).** La transitoire est une poignée globale constante, dans un fichier du cœur listé avec ses 3 appels libc.
  - `alloc-guard` compte chaque relocation vers son symbole comme une allocation.
  - Une référence à la poignée frame ne compte pas.
  - `pico8_ents_free`/`pico8_ptrs_free` passent par la zone gardée dans le conteneur et ne référencent pas la transitoire.
- **H-0b-3 (fonctions qui prennent une zone).** Après 0b, prennent une zone :
  - `pico8_tostr_fixed`, `pico8_sub`, `pico8_ne` et la concaténation partagée ;
  - `pico8_jp_ptrs`, `pico8_la_ptrs`, `pico8_nv_ptrs`, `pico8_ents_cat`, `pico8_jp_ents` et `pico8_qz` ;
  - `pico8_ents_init`, `pico8_ptrs_init`, `lq_new`, `lq_copy` et `jb_new`.

  Les fabricants locaux (`jo_dollar`, `concat_percent`, la jointure de `hp`, le bloc de `hp:jb`, `jo_gh_row_items`, `pico8_menu_invented_lines`) restent des `malloc`/`calloc` au site.
- **H-0b-4 (concaténation).** Les cinq `concat2` statiques (`pico8_bw_concat2`, `cr_concat2`, `ie_concat2`, et les deux `pj_concat2`) sont remplacées par une seule concaténation partagée qui prend une zone. Les `free` de leurs résultats chez l'appelant sont inchangés.
- **H-0b-5 (libération d'un texte).** L'appelant d'un texte fait en transitoire le libère encore avec `free`. L'étape 1 supprime donc ce `free` en passant la poignée frame. Elle ne le remplace pas par **rendre**.
- **H-0b-6 (comptes après 0b).** Les comptes de la colonne « Après 0b » sont ceux de `HEAD`, plus ou moins les effets suivants :
  - les fichiers à `concat2` perdent 1 ;
  - chaque appel d'une fonction de H-0b-3 ajoute 1, compté sur les sources ;
  - `pico8_entity.c` entre dans la liste avec 3 (`g_hv`, `g_cv` en boucle, `layers` en boucle).

  `pico8_collect.c`, `pico8_str.c`, `pico8_ents.c`, `pico8_ptrs.c` et `pico8_geom.c` sortent de la liste.
- **H-0b-7 (aucune zone de jeu instanciée).** 0b n'instancie aucune zone frame, interface, particules ni partie, et ne change pas `pico8_zone_sizes.h`. C'est l'étape 1 qui crée la première instance.
- **H-0b-8 (`ne` dans une zone à pointeur montant).** `pico8_ne` fait son texte intermédiaire dans la zone passée et le rend par **rendre**. Avec la poignée frame, il ne reste donc aucun appel libc ni aucune référence à la transitoire dans `ne`.
- **H-0b-9 (générateurs).** 0b ne paramètre pas `pico8_jf`/`pico8_gn`. L'état `oh` de `pico8_bg_pq` reste hors étape 1.
- **H-0b-10 (règle de TU ASan).** Le TU ASan de 0b, construit sur le modèle de `tmp/test_pico8_zone`, existe et sert d'exemple. Le TU de l'étape 1 peut s'y ajouter ou avoir sa propre règle.
- **H-0b-11 (fin des hausses).** Après 0b, aucun compte ne monte plus. Toutes les variations de l'étape 1 sont des baisses.

## À revérifier une fois 0b finie

À relire contre le `HEAD` qui suit le dernier commit de 0b et sa note « Découvert », avant tout code :

- Le nom réel de la poignée, de son constructeur sur zone à pointeur montant et de la poignée transitoire (H-0b-1, H-0b-2).
- La liste réelle des fonctions qui prennent une zone, et leur ordre d'arguments (H-0b-3). En particulier, l'étape suppose que `jo_dollar`, `concat_percent`, `pico8_hp_jb` et `jo_gh_row_items` sont restés des fabricants locaux.
- La forme de la concaténation partagée, et la suppression effective des cinq copies (H-0b-4).
- Si le `free` d'un texte en transitoire est resté `free` chez l'appelant, ou s'il est devenu **rendre** (H-0b-5). Dans le second cas, ce que l'étape supprime est un appel à **rendre**, et la baisse ne compte que les références à la transitoire.
- **Les comptes réels de chaque fichier de l'inventaire dans `tools/alloc_guard_exceptions.txt`** (H-0b-6), et le compte d'objet de chaque site converti. Récrire alors la table « Baisse attendue » sur ces chiffres.
- Que `pico8_entity.c` est bien entré dans la liste, et avec quel compte.
- Que les `ents_free`/`ptrs_free` ne comptent pas, et que la poignée frame n'est pas comptée par `alloc-guard` (H-0b-2).
- Que `pico8_ne` ne laisse aucun appel libc quand on lui passe une poignée à pointeur montant (H-0b-8).
- Que 0b n'a pas paramétré `pico8_jf`/`pico8_gn` (H-0b-9). Sinon, l'état `oh` de `pico8_bg_pq` entre dans l'inventaire des sûrs.
- La règle ASan du TU de 0b, et si elle peut accueillir les cas de l'étape 1 (H-0b-10).
- Que la note « Découvert » de 0b ne reclasse aucun site de l'inventaire ci-dessus, par exemple un `bs`/`rows` que `pz` garderait finalement.

## Further Notes

- Aucun outil `Agent` n'était disponible dans cette session. Les points que `to-prd` fait valider par un humain ont donc été tranchés en appliquant moi-même les principes du skill `guideline` :
  - **Hypothèses explicites** : sections H-0b et « À revérifier ».
  - **Simplicité** : un module de zone frame à deux fonctions, et pas de nouveau paramètre de zone pour les fabricants locaux. Le point de remise à zéro est unique, dans le cœur, plutôt que dans chaque pilote.
  - **Changements chirurgicaux** : les sites douteux restent à la transitoire, et aucune boîte `vec` n'est déplacée.
  - **Critères vérifiables** : comptes du cliquet par fichier, `*.sha256` inchangés, et `sanitize-net` sous empoisonnement par frame.
  - Le découpage en six morceaux suit l'ordre où chaque morceau se vérifie seul.
- La décision d'inclure les sites « prouvés par lecture » (morceau 5) découle de la règle de fin du plan : tous les sites de la zone. Laissés dehors, ces tableaux de travail fuiraient vers une zone à places ou de partie, où ils rempliraient la zone sans jamais être rendus. Le test de survie à la remise à zéro est le garde-fou qu'exige le §4.
- **Constat pour les étapes 2 et 4**, à reporter dans « Découvert ». Une dizaine de boîtes `vec` sont remplacées **à chaque tick** par un nouveau `malloc` (`qb:ph` en fait trois par frame). Dans une zone de durée plus longue sans libération, elles la rempliraient en quelques minutes de jeu. Les étapes 2 et 4 devront décider comment les rendre : place rendue, ou boîte tenue dans l'objet. Ce n'est pas une décision de l'étape 1.
- À la fin de l'étape, la note « Découvert » sous l'entrée 1 du plan porte au minimum :
  - les sites classés autrement que dans le §2, et pourquoi ;
  - les sites renvoyés à la transitoire par un crash ASan ;
  - la limite frame (inchangée ou relevée) ;
  - les comptes du cliquet laissés, fichier par fichier ;
  - les écarts entre les estimations de cette PRD et la mesure sur objets.
