---
title: "Plan 4, étape 1 — Zone frame : textes et petites listes de la frame"
description: "Les textes, listes filtrées et tableaux de travail qui ne vivent qu'une frame passent encore par la zone transitoire ou la libc, avec un free écrit à la main : l'étape 1 instancie la zone frame, la remet à zéro au début de chaque frame, y range ces sites et retire leurs free."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur. Les étapes 0a et 0b ont posé le socle.

- **0a** a livré :
  - `pico8_fatal` ;
  - la zone à pointeur montant et la zone à places, avec leur empoisonnement ASan ;
  - le plafond de 1,5 Mo ;
  - le verrou à cliquet `alloc-guard`, en égalité stricte.
- **0b** a livré :
  - la poignée `pico8_zone_choisie_t` (alloc, grow, release), avec `pico8_zone_choisie_bump` sur une zone montante ;
  - la poignée `pico8_zone_transitoire`, adossée à `malloc`/`realloc`/`free`.

  Les fonctions partagées prennent leur zone **en premier argument** (`const pico8_zone_choisie_t *`) :
  - les textes : `pico8_tostr_fixed`, `pico8_sub`, `pico8_concat2`, `pico8_ne` ;
  - la boîte : `pico8_qz` ;
  - les listes fabriquées : `pico8_ents_cat`, `pico8_jp_ents`, `pico8_jp_ptrs`, `pico8_la_ptrs`, `pico8_nv_ptrs` ;
  - les panneaux : `pico8_lq_new`, `pico8_lq_copy`, `pico8_jb_new`.

  Les conteneurs `ents`/`ptrs` reçoivent leur zone à l'init et la gardent. Un panneau garde la zone de son créateur. Tous les appelants passent encore la transitoire, et chaque référence à `pico8_zone_transitoire` compte au cliquet. La liste d'exceptions compte 34 fichiers et 228 appels, mesurés sur objets. **Plus aucun compte ne peut monter.**

Aucune zone du jeu n'est encore instanciée. Or une bonne part des allocations du cœur ne vit qu'**une frame**, voire seulement le temps d'un appel :

- des textes intermédiaires : un `tostr` concaténé puis libéré, les morceaux `"level " .. tl .. " (" ..` du menu de recherche ;
- des listes filtrées : `jp`, la concaténation d'entités, les calques de `dc` ;
- des tableaux de travail : les `bs`/`rows` passés à `pz`, la rangée `l` de `jo.ex`.

Chacun passe par la transitoire ou la libc, puis est rendu par un `free` écrit à la main. Ces `free` sont autant d'occasions d'erreur, et le compte du cliquet ne baisse pas.

## Solution

L'étape 1 instancie la **zone frame** et y range tous les sites dont l'objet meurt avant la fin de la frame. Leurs `free` disparaissent.

- **Instance.** Un petit module du cœur possède une zone à pointeur montant de `PICO8_ZONE_FRAME_BYTES` (256 Ko), nommée `frame`.
  - Il expose la poignée `pico8_zone_choisie_t` sur cette zone, construite par `pico8_zone_choisie_bump`. Les appelants convertis la passent au lieu de `&pico8_zone_transitoire`.
  - Il expose aussi la remise à zéro.
  - La zone s'initialise d'elle-même au premier usage. Un test unitaire qui appelle une fonction du jeu sans `pico8_cart_load()` la trouve donc prête.
- **Remise à zéro au début de chaque frame**, en tête de `pico8__update60()`.
  - C'est le seul point par lequel passent tous les pilotes :
    - le rejeu (`main.c`) ;
    - le binaire raylib (`main_raylib.c`) ;
    - la couverture de classes (`main_class_coverage.c`) ;
    - les tests qui bouclent `update60`/`draw`.
  - Aucun pilote ne change.
  - Sous ASan, la remise à zéro empoisonne la moitié quittée (0a). Un objet de frame gardé jusqu'à la frame suivante fait planter `make sanitize-net`, qui est l'oracle du §3.
- **Conversion des sites.** Chaque site de l'inventaire ci-dessous change de zone :
  - il passe la poignée frame au lieu de `&pico8_zone_transitoire` ;
  - ou, pour un fabricant local, il alloue par `pico8_zone_alloc` sur la poignée frame au lieu de `malloc`/`calloc`.

  Le `free`, `pico8_ents_free` ou `pico8_ptrs_free` correspondant est supprimé.
- **Pas de changement visible.** Les 26 `*.sha256` de `tests/e2e/` restent identiques, et `make test-all` et `make sanitize-net` passent à chaque commit.
- **Cliquet.** Le compte de chaque fichier concerné baisse exactement du nombre de références à la transitoire et d'appels libc retirés : 81 au total. Aucun compte ne monte. `src/pico8_pi.c` et `src/pico8_economy_score.c` sortent de la liste.
- **Correction du §2 du plan.** Le tableau range les « petites boîtes (`vec`) » en zone frame. La lecture montre que les boîtes `vec` du cœur sont presque toutes des champs d'objets durables (`ra`, `p`, `v`, `qf`, `qa`), remplacés à chaque tick et relus à la frame suivante. Ce ne sont pas des objets de frame (§4). L'étape corrige le tableau dans son dernier commit, et sa note « Découvert » le dit.

## User Stories

1. En tant que mainteneur du plan, je veux une zone frame de taille fixe instanciée dans le cœur, pour que les objets d'une frame ne demandent plus rien à la libc ni à la transitoire.
2. En tant que mainteneur, je veux que la zone frame soit remise à zéro en tête de `pico8__update60()`, pour que tous les pilotes (rejeu, raylib, couverture, tests) la remettent à zéro au même instant sans être modifiés.
3. En tant que développeur qui chasse un accès hors durée de vie, je veux qu'un pointeur pris dans la zone frame puis lu après le `pico8__update60()` suivant plante sous ASan, pour qu'un site mal classé se révèle dans `make sanitize-net` au lieu de corrompre la mémoire.
4. En tant qu'auteur de tests unitaires, je veux que la zone frame s'initialise d'elle-même au premier usage, pour qu'un TU qui appelle `pico8_cr` ou `pico8_jo_gh` sans charger le cart n'ait rien à préparer.
5. En tant qu'auteur de tests unitaires, je veux pouvoir remettre la zone frame à zéro à la main, pour qu'un test qui boucle longtemps sans `update60` ne la remplisse pas et pour prouver qu'un panneau survit à la remise à zéro.
6. En tant que développeur, je veux que la zone pleine sorte par `pico8_fatal` avec « zone frame pleine, limite 262144 octets », pour qu'une limite trop basse se lise et se relève en un chiffre.
7. En tant que mainteneur du budget, je veux que relever la limite de la zone frame s'arrête et le signale si la somme dépasse 1,5 Mo, pour que le plafond PSX ne bouge pas en silence.
8. En tant que développeur du menu de recherche (`pico8_cr`), je veux que les `tostr` et les `pico8_concat2` intermédiaires (`level`, `(`, `/`, `n`) soient faits en zone frame, pour supprimer les dix `free` à la main de cette fonction.
9. En tant que développeur, je veux que les tableaux `bs`/`rows` passés à `pz` par `pico8_cr` et `pico8_jo_fe` soient pris dans la zone frame, pour supprimer leurs `calloc`/`free`, puisque `pz` en recopie tout avant de rendre la main.
10. En tant que développeur, je veux que la rangée de travail `l` de `jo.ex` soit prise dans la zone frame, pour supprimer son `malloc`/`free`, puisque `pico8_lq_new` recopie les cellules dans la zone du panneau.
11. En tant que développeur, je veux que les listes filtrées jetées dans la même fonction (`pico8_cz`, `pico8_eg`, `pico8_bv`, `pico8_jc_ce`, `pico8_jo_gh`) soient faites en zone frame, pour supprimer leurs `pico8_ents_free`/`pico8_ptrs_free`.
12. En tant que développeur, je veux que les quinze calques locaux de `pico8_dc` vivent en zone frame, pour que le dessin de chaque frame ne passe plus par la transitoire.
13. En tant que développeur, je veux que les listes de travail de `pico8_trade_kg` (`empty_lp`, `empty_rl`, `np_list`, `jc_list`) et le `prefix` de `pico8_jo_cn` soient faits en zone frame, pendant que les listes produites par `pico8_ents_cat`/`pico8_la_ptrs` restent des données de partie.
14. En tant que développeur, je veux que le texte numérique d'un identifiant (`tostr(id)` dans `pico8_bw` et `pico8_ie_ldown`) soit fait en zone frame, alors que l'identifiant `pico8_concat2("p", …)` / `("s", …)` reste hors de cette étape, pour qu'un identifiant de partie ne meure jamais à la frame suivante.
15. En tant que développeur, je veux que le texte du `print` d'un nombre (`pico8_pi_io_draw`) soit fait en zone frame, pour que `pico8_pi.c` sorte de la liste d'exceptions.
16. En tant que développeur, je veux que les textes de `pico8_dl_show` (prix d'une sonde) soient faits en zone frame, pour supprimer ses deux `free`.
17. En tant que développeur, je veux que les textes passés à `jo_dollar` (`ne`, `tostr`) soient faits en zone frame, pour que `jo_dollar` ne libère plus son argument, alors que le texte qu'il rend reste lié au panneau.
18. En tant que développeur, je veux que les `tostr` intermédiaires de `pico8_hp_jb_join` et de `pico8_hp_jb`, et le bloc `jb` de `pico8_hp_jb`, soient faits en zone frame, pour supprimer leurs `free`.
19. En tant que développeur, je veux que les `tostr` intermédiaires de `pj:hb` (`nz`, `+po`, `happiness`) et de `pj:hk` (`year`), et les morceaux `piece1`/`piece2` de `pj:ph_map`, soient faits en zone frame, pour supprimer leurs `free`.
20. En tant que développeur, je veux que `pico8_ky` fasse son texte `happiness` intermédiaire et son résultat `%` en zone frame, puisque son seul appelant le recopie aussitôt par `pico8_concat2`, pour que `pico8_economy_score.c` sorte de la liste.
21. En tant que développeur, je veux que les tableaux de cellules de `jo:gh` et la rangée « n invented » du menu soient faits en zone frame, puisque `pico8_lq_new` et `pico8_jb_new` les recopient dans la zone du panneau, pour qu'une fuite TROU-7 de frame disparaisse au lieu d'être portée dans une zone plus longue.
22. En tant que mainteneur, je veux qu'un site dont la lecture ne prouve pas que l'objet meurt dans la frame reste à la transitoire et passe à la zone la plus longue qui le couvre (§4), pour ne jamais raccourcir une durée de vie au jugé.
23. En tant que mainteneur, je veux qu'un site dont la conversion fait planter `make sanitize-net` ne soit pas converti, reste à la transitoire et soit noté dans « Découvert » comme classé autrement par crash ASan, pour que l'oracle rétrograde sans qu'on relève une limite ni qu'un compte remonte.
24. En tant que gardien du cliquet, je veux que chaque commit de l'étape remette `alloc-guard` à égalité stricte sur les comptes mesurés, sans jamais en faire monter un, et que son message liste les baisses mesurées et leur écart à la prévision, pour que l'égalité stricte reste lisible.
25. En tant que gardien du cliquet, je veux qu'à la fin de l'étape `pico8_pi.c` et `pico8_economy_score.c` aient quitté la liste d'exceptions, pour que le verrou interdise toute nouvelle allocation libc ou transitoire dans ces deux fichiers.
26. En tant que gardien de l'invariant, je veux que les `*.sha256` de `tests/e2e/` restent identiques à chaque commit, pour prouver que changer de zone ne change pas un pixel.
27. En tant que lecteur du plan, je veux que l'étape corrige la ligne « Frame » du tableau du §2 (les boîtes `vec` remplacées à chaque tick n'y sont pas), pour que les étapes 2 et 4 ne les y cherchent pas.
28. En tant que développeur des étapes 2 à 4, je veux que la note « Découvert » liste les sites écartés de la zone frame et pourquoi, pour savoir lesquels me reviennent.
29. En tant que mainteneur des commentaires, je veux que les `/* TROU-7 */` des sites dont la fuite disparaît soient retirés, et les contrats « l'appelant libère » réécrits pour les fonctions converties, pour que les commentaires restent vrais.
30. En tant que futur porteur PSX, je veux que la zone frame n'ajoute aucune dépendance à un pilote, pour qu'un pilote PSX n'ait qu'à appeler `pico8__update60()`.

## Implementation Decisions

- **Module de la zone frame.** C'est un petit module du cœur, dans `CORE_SRCS`.
  - Il ne référence ni la libc ni `pico8_zone_transitoire`, et reste donc hors de la liste d'exceptions.
  - Il possède :
    - le stockage `PICO8_BUMP_ZONE_STORAGE` de `PICO8_ZONE_FRAME_BYTES` ;
    - le `pico8_bump_zone_t` nommé `"frame"` ;
    - une poignée `pico8_zone_choisie_t` statique, obtenue par `pico8_zone_choisie_bump`. Elle doit être statique parce que les conteneurs `ents`/`ptrs` gardent un pointeur vers leur zone (« `zone` must outlive `ents` »).
  - Il expose deux fonctions : l'accès à cette poignée (`const pico8_zone_choisie_t *`) et la remise à zéro.
  - **Initialisation paresseuse.** L'accès initialise la zone et la poignée au premier appel. Un initialiseur statique ne suffit pas : `pico8_bump_zone_init` empoisonne le stockage sous ASan.
  - La remise à zéro initialise d'abord la zone si besoin, puis appelle `pico8_bump_zone_reset`.
  - Le module ne touche pas `pico8_zone_sizes.h` : la taille existe déjà et compte déjà dans le plafond.
  - Il entre dans toutes les clôtures d'objets qui lient un module converti, aux côtés de `pico8_zone_choisie.c` et `pico8_zone.c` :
    - `ENGINE_SRCS`/`DATA_ENGINE_SRCS` ;
    - les listes de chaque TU ;
    - `raylib` ;
    - la couverture de classes.
- **Point de remise à zéro.** Il est en tête de `pico8__update60()`, avant `pico8_dg()`.
  - `pico8_cheat_step`, appelé juste avant par les pilotes, n'alloue pas.
  - `pico8__draw()` suit `update60` dans la même frame : un objet fait pendant `update` et lu pendant `draw` est valide.
  - Pas de remise à zéro dans `pico8_cart_load`, `pico8__init` ni `pico8_cart_shutdown`. Les sites convertis rendent leur objet avant le retour de la fonction qui l'a fait, ou le font recopier avant ce retour. Ils sont donc corrects même appelés hors de `update60`/`draw`.
- **Critère de classement.** Il précise le §4 pour cette étape.
  - **Sûr** : l'objet est libéré (`free`, `*_free`) dans la fonction qui l'a fait, ou dans celle qui le reçoit aussitôt (`jo_dollar`). Ces sites sont convertis.
  - **Prouvé par lecture** : l'objet n'est libéré nulle part (fuite TROU-7), mais son unique consommateur le recopie avant de rendre la main. Le seul cas est un tableau de cellules ou une rangée `jb`, que `pico8_lq_new`/`pico8_jb_new` recopient dans la zone gardée par le panneau (0b : `items`, `fs`, `ls` et les lignes d'un `jb` vivent dans la zone du créateur du panneau). C'est cette lecture, et elle seule, qui les range en zone frame (§4). Chacun a en plus un test qui remet la zone à zéro après la construction puis exerce le panneau : ce test ne peut que rétrograder un site, pas le promouvoir.
  - **Douteux** : l'objet est rangé dans un champ d'un objet durable, d'un panneau ou d'un état global, ou il est référencé par pointeur dans une cellule (`pico8_lq_cell_row`, `pico8_lq_cell_string`). Ces sites restent à la transitoire et reviennent aux étapes 2 à 4. Un `sanitize-net` vert après un essai de conversion ne suffit jamais à les ranger en zone frame (§4 : ASan ne voit pas un pointeur gardé à travers deux remises à zéro).
- **Les `free` disparaissent.**
  - Pour un texte ou un tableau fait en zone frame, la ligne `free` est supprimée.
  - Pour une liste `ents`/`ptrs` faite en zone frame, l'appel `pico8_ents_free`/`pico8_ptrs_free` est supprimé aussi. Il rendrait par la poignée frame, où rendre ne fait rien, et le garder laisserait croire qu'il rend quelque chose.
  - **Jamais de `free` sur un bloc frame.** La conversion d'un site et la suppression de son `free` vont dans le même commit.
  - `jo_dollar` libère son argument par `free(s)`. Ses deux appelants (`pico8_jo_gh` pour `ne`, `pico8_jo_fe_keys` pour `tostr`) et ce `free(s)` changent donc ensemble.
- **Fabricants locaux.** Quatre fabricants locaux font leur résultat en zone frame, par `pico8_zone_alloc` sur la poignée frame :
  - `concat_percent` (`pico8_ky`) ;
  - le bloc de `pico8_hp_jb` ;
  - `jo_gh_row_items` ;
  - `pico8_menu_invented_lines`.

  Ils ne gagnent pas de paramètre de zone : chacun n'a qu'un appelant et une seule durée de vie. La mémoire de `pico8_zone_alloc` n'étant pas initialisée, les deux anciens `calloc` (`pico8_hp_jb`, `pico8_menu_invented_lines`) mettent leur bloc à zéro explicitement.
  - `jo_dollar` garde son `malloc` : son résultat reste lié au panneau.
- **Contrats C.** Les en-têtes des fonctions publiques dont le résultat passe en zone frame disent « valable jusqu'à la prochaine remise à zéro de la zone frame » au lieu de « never freed » / « caller frees ». Cela concerne `pico8_hp_jb` et `pico8_ky` (champ `happiness`).
- **Commentaires.** Ils suivent les quatre destinations de `CLAUDE.md`.
  - Les `/* TROU-7 */` qui marquent une fuite supprimée sont retirés : `jo_gh_row_items`, `pico8_menu_invented_lines`, et `result.happiness` de `pico8_ky`.
  - L'entrée TROU-7 de `docs/trous-de-portage.md` reste jusqu'à l'étape 5.
  - Le pourquoi de la remise à zéro en tête de `update60` est un piège qu'on réintroduirait (la déplacer dans un pilote). Il tient en un commentaire court.
- **Limite.** Elle reste à 256 Ko au départ.
  - Un `pico8_fatal` « zone frame pleine » dans un e2e fait relever `PICO8_ZONE_FRAME_BYTES`, dans la limite de 1,5 Mo de somme (§3). Si la somme dépasse, l'étape s'arrête et le signale : zone, limite voulue, somme actuelle.
  - Croître une liste en zone montante copie le bloc (0b). Les quinze calques de `pico8_dc`, rebâtis deux fois par frame, laissent donc environ deux fois leur taille finale en blocs morts jusqu'à la remise à zéro. On mesure avant de relever.
  - Un TU qui remplit la zone parce qu'il boucle sans `update60` appelle la remise à zéro entre ses itérations. On ne relève pas la limite pour lui.
- **Correction du §2.** Dans la ligne « Frame » du tableau, « petites boîtes (`vec`, listes enveloppées) » devient « listes filtrées et tableaux de travail ». Les boîtes `vec` rangées dans un champ y sont renvoyées à la zone de leur propriétaire. Cette correction se fait dans le commit qui ajoute la note « Découvert ».

### Inventaire des sites — convertis à l'étape 1

Relevé à `HEAD` 86c7f0d par `objdump -r` sur les objets d'`alloc-guard` (`-O0 -g`), chaque relocation rattachée à sa fonction et à sa ligne par `addr2line`. À ce niveau d'optimisation, un appel correspond à une relocation.

« T » désigne une référence à `pico8_zone_transitoire`. « libc » désigne un appel `malloc`/`calloc`/`realloc`/`free`.

**Sûrs** (objet rendu dans la fonction, ou par son récepteur immédiat) :

| Fichier | Fonction | Objet | Baisse |
|---|---|---|---|
| `pico8_menu.c` | `pico8_cr` | `gr_str` (`tostr`) + son `free` | 2 (1 T + 1 libc) |
| | | `tl_str`, `diff1_str`, `diff2_str` (`tostr`) + leurs 3 `free` | 6 (3 T + 3 libc) |
| | | `tmp1`, `tmp2`, `tmp3`, `tmp1`, `tmp2` (`pico8_concat2`) + leurs 5 `free` | 10 (5 T + 5 libc) |
| | | `rows`, `bs` (`calloc`) + leurs 2 `free` | 4 libc |
| `pico8_planet_display.c` | `pico8_jo_gh` | `pico8_ne` passé à `jo_dollar` | 1 T |
| | | `ctx.ji` (`ptrs_init`) et `jx` (`jp_ptrs`), et leurs `ptrs_free` | 2 T |
| | | `count_str`, `mu_str` de « exports » (`tostr`) + leurs 2 `free` | 4 (2 T + 2 libc) |
| | `pico8_jo_fe_keys` | `tostr(od)` passé à `jo_dollar` | 1 T |
| | `jo_dollar` | `free(s)` de son argument | 1 libc |
| | `pico8_jo_fe` | `bs`, `rows` (`calloc`) + leurs 2 `free` | 4 libc |
| `pico8_research_display.c` | `pico8_hp_jb_join` | `num_str` (`tostr`) + `free` | 2 (1 T + 1 libc) |
| | `pico8_hp_jb` | `o_str` (`tostr`) + `free` | 2 (1 T + 1 libc) |
| | `pico8_jo_ex` | `l` (`malloc`) + `free` | 2 libc |
| `pico8_score_panel.c` | `pico8_eg` | `filtered` (`jp_ents`), son `ents_free` | 1 T |
| | `pico8_pj_hb` | `nz_str`, `plus_num`, `happiness_str` (`tostr`) + 3 `free` | 6 (3 T + 3 libc) |
| `pico8_score_panel_display.c` | `pico8_pj_hk` | `yr_str` (`tostr`) + `free` | 2 (1 T + 1 libc) |
| | `pico8_pj_ph_map` | `piece1`, `piece2` (`pico8_concat2`) + 2 `free` | 4 (2 T + 2 libc) |
| `pico8_probe.c` | `pico8_bv` | `filtered` (`jp_ents`), son `ents_free` | 1 T |
| | `pico8_dl_show` | `od_str` (`tostr`) + `free` ; `heap_display` (`malloc`) + `free` | 4 (1 T + 3 libc) |
| `pico8_slipway.c` | `pico8_jc_ce` | `part1`, `part2` (`jp_ents`), `ds` (`ents_cat`), leurs 3 `ents_free` | 3 T |
| `pico8_trade.c` | `pico8_trade_kg` | `empty_lp`, `empty_rl`, `np_list`, `jc_list` (`ents_init`), 2 `ents_free` | 4 T |
| `pico8_planet_trade.c` | `pico8_jo_cn` | `prefix` (`ptrs_init`), son `ptrs_free` | 1 T |
| `pico8_entity.c` | `pico8_dc` | `layers[15]` (`ents_init` en boucle), ses `ents_free` | 1 T |
| `pico8_galaxy.c` | `pico8_bw` | `id_num_str` (`tostr`) + `free` | 2 (1 T + 1 libc) |
| `pico8_research_icon.c` | `pico8_ie_ldown` | `id_num_str` (`tostr`) + `free` | 2 (1 T + 1 libc) |
| `pico8_pi.c` | `pico8_pi_io_draw` | `owned` (`tostr`) + `free` | 2 (1 T + 1 libc) |
| `pico8_economy_score.c` | `pico8_cz` | `filtered` (`jp_ents`), son `ents_free` | 1 T |
| | `pico8_ky` | `happiness_str` (`tostr`) + `free` | 2 (1 T + 1 libc) |

**Prouvés par lecture** (fuite TROU-7 aujourd'hui, recopiés par leur unique consommateur avant le retour) :

| Fichier | Fonction | Objet | Consommateur qui recopie | Baisse |
|---|---|---|---|---|
| `pico8_planet_display.c` | `jo_gh_row_items` | tableaux de cellules des rangées 2..n de `jo:gh` | `pz` → `js_new` → `pico8_lq_new` recopie `keys->items` dans la zone du panneau | 1 libc |
| `pico8_menu.c` | `pico8_menu_invented_lines` | rangée `jb` « n invented » | `pz` → `js_new` → `pico8_jb_new` (`jb_line` recopie chaque ligne) | 1 libc |
| `pico8_research_display.c` | `pico8_hp_jb` | bloc `jb` (`calloc`) | `jo_fe` → `pz` → `pico8_jb_new`. Le texte de la jointure, lui, reste référencé par pointeur et hors étape | 1 libc |
| `pico8_economy_score.c` | `concat_percent` | `result.happiness` de `pico8_ky` | seul appelant, `pico8_pj_ph_s_str`, le recopie par `pico8_concat2(…, s->happiness, "")` | 1 libc |

### Baisse attendue du cliquet, par fichier

| Fichier | Compte à `HEAD` (mesuré) | Baisse sûrs | Baisse lecture | Reste |
|---|---|---|---|---|
| `pico8_menu.c` | 27 | 22 | 1 | 4 |
| `pico8_planet_display.c` | 18 | 13 | 1 | 4 |
| `pico8_research_display.c` | 12 | 6 | 1 | 5 |
| `pico8_score_panel.c` | 12 | 7 | — | 5 |
| `pico8_score_panel_display.c` | 19 | 6 | — | 13 |
| `pico8_probe.c` | 9 | 5 | — | 4 |
| `pico8_trade.c` | 9 | 4 | — | 5 |
| `pico8_slipway.c` | 8 | 3 | — | 5 |
| `pico8_economy_score.c` | 4 | 3 | 1 | 0 — sort |
| `pico8_galaxy.c` | 5 | 2 | — | 3 |
| `pico8_research_icon.c` | 5 | 2 | — | 3 |
| `pico8_pi.c` | 2 | 2 | — | 0 — sort |
| `pico8_planet_trade.c` | 9 | 1 | — | 8 |
| `pico8_entity.c` | 4 | 1 | — | 3 |
| **Total** | **143 sur 228** | **77** | **4** | **62** |

En fin d'étape, la liste compte 32 fichiers et 147 appels. Les 20 autres fichiers de la liste, dont `pico8_cart.c` (1, le `pico8_jb_new` des bulles de `pico8_eu`), ne bougent pas.

Ces baisses sont des **prévisions**. Elles sont mesurées sur les relocations à `-O0`, mais le verrou compte des relocations, qui dépendent du compilateur. Le critère de fin est celui du §5 du plan : chaque site de l'inventaire est converti, **vérifié sur la source** (la fonction ne référence plus `pico8_zone_transitoire` ni la libc pour ces objets) ; `alloc-guard` est à égalité stricte sur les comptes mesurés ; aucun compte n'a monté. Un écart entre prévision et mesure se note dans le message de commit et dans « Découvert ». Il ne bloque pas l'étape et n'est jamais compensé ailleurs.

Ce qui reste dans ces fichiers est hors étape :

- **menu (4)** : `gi_str` de `pico8_cr_step`, `label` et `ep` de `pico8_cr`, le `pico8_lq_new` de `pico8_ix2_hb_dispatch`.
- **planet_display (4)** : `jo_kw_tuple`, le résultat de `jo_dollar`, le `mu_str` de « makes », `combined`.
- **research_display (5)** : la jointure de `hp:jb` (`malloc` + `realloc`), les tuples de `jo.ex`, `jq_str`, le `pico8_lq_new` de `jo.ex`.
- **score_panel_display (13)** :
  - `year_label` ;
  - les sept textes de `pico8_pj_ph_s_str` ;
  - `label` ;
  - le `pico8_lq_new` et `wrapped` de `pj:ph_map` ;
  - les deux `spec*.ra` de `pj:ph`.

### Sites écartés de la zone frame (reviennent aux étapes 2 à 4)

Ces sites restent à la transitoire ou à la libc. Chacun est listé avec sa raison, pour la note « Découvert ».

- **Textes rangés dans un objet durable ou une cellule** :
  - `pico8_ne` dans `pico8_hy_lu` et `pico8_jo_hk` : le texte est porté par `iv` ;
  - `gi_str`, `label` et `ep` de `pico8_cr` ;
  - le résultat de `jo_dollar`, `combined` et le `mu_str` de « makes » dans `jo:gh` ;
  - la jointure de `hp:jb` et `jq_str` de `jo.ex` ;
  - `cf` et les `pico8_concat2` de `pj:hb`, rangés dans `ou_state` et relus aux frames suivantes ;
  - `year_label`, les textes de `pico8_pj_ph_s_str` et `label` de `pj:ph` ;
  - `jo_kw_tuple` et les tuples de `jo.ex`, référencés par pointeur dans une cellule.
- **Panneaux** : tous les `pico8_lq_new`/`pico8_lq_copy`/`pico8_jb_new`, dont ceux de `jo.ex`, de `pj:ph_map`, d'`ix2_hb` et de `pico8_eu`. Leurs tableaux vivent dans la zone de leur créateur : étape 2.
- **Identifiants de partie** : `pico8_concat2("p", …)` dans `pico8_bw` et `("s", …)` dans `pico8_ie_ldown`. Ils reviennent à l'étape 4.
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
  - les `qf`/`ra` de `pz` et les `spec*.ra` de `pj:ph`.
- **Boîte `pico8_qz` de `js`** : elle est rangée dans `header.kq` du bouton. Étape 2.
- **Listes rangées dans un objet** :
  - `le` de `pz` : le même site est rangé dans `ok->le` ou libéré selon la branche, donc on prend la zone la plus longue ;
  - les calques `ic` de `ui`, rebâtis par `gw` mais relus hors de la frame qui les bâtit, ce qui n'est pas prouvé ;
  - `ox` de `mz` ;
  - `cc`, `oe`, `jy`, `by`/`ib`, `lp`/`rl` ;
  - `hv`, `cv`, `cg` de `pico8_entity_init` ;
  - les listes produites par `pico8_ents_cat` dans `trade_kg` et par `pico8_la_ptrs` dans `jo_cn`.
- **État de générateur `oh` de `pico8_bg_pq` (`pico8_jf`)** : il est fait et rendu dans la même fonction, mais par `pico8_jf`, que 0b n'a pas paramétré (`pico8_gen.h` inchangé). Il revient à l'étape 4, qui donnera une zone aux états de `pico8_gen.c`.

## Implementation Chunks

Chaque morceau est un commit qui respecte l'invariant du §3 et remet la liste d'exceptions à égalité stricte. Son message dit les baisses par fichier. **Aucun morceau ne fait monter un compte** : le morceau 1 n'ajoute que des références à la poignée frame, que le verrou ne compte pas, et les morceaux 2 à 5 ne font que retirer des références à la transitoire et des appels libc.

1. **Zone frame et remise à zéro.**
   - Le module de la zone frame, la remise à zéro en tête de `pico8__update60()`, et l'ajout du module aux clôtures d'objets.
   - Le TU ASan de la zone frame (voir Testing Decisions).
   - Aucun site n'est converti. Comptes inchangés : 34 fichiers, 228 appels.
2. **Listes de frame.** Les sites `jp_ents`, `jp_ptrs`, `ents_cat` et `ents_init`/`ptrs_init` sûrs :
   - `pico8_cz`, `pico8_eg`, `pico8_bv`, `pico8_jc_ce`, `pico8_trade_kg`, `pico8_jo_cn` ;
   - `ctx.ji`/`jx` de `jo:gh` ;
   - `pico8_dc`.

   Leurs `*_free` disparaissent. Baisses :

   | Fichier | Avant | Après |
   |---|---|---|
   | `economy_score` | 4 | 3 |
   | `score_panel` | 12 | 11 |
   | `probe` | 9 | 8 |
   | `slipway` | 8 | 5 |
   | `trade` | 9 | 5 |
   | `planet_trade` | 9 | 8 |
   | `planet_display` | 18 | 16 |
   | `entity` | 4 | 3 |

   Le total passe à 215.
3. **Textes de frame.** Les `tostr`, `pico8_ne` et `pico8_concat2` sûrs, et leurs `free` :
   - le menu ;
   - `jo:gh`, `jo:fe_keys` et le `free(s)` de `jo_dollar`, dans ce même commit ;
   - `hp:jb_join` et `hp:jb`, `pj:hb`, `pj:hk`, `pj:ph_map` ;
   - `dl_show`, `bw`, `ie_ldown`, `pi_io_draw`, `ky`.

   `pico8_pi.c` sort de la liste. Baisses :

   | Fichier | Avant | Après |
   |---|---|---|
   | `menu` | 27 | 9 |
   | `planet_display` | 16 | 10 |
   | `research_display` | 12 | 8 |
   | `score_panel` | 11 | 5 |
   | `score_panel_display` | 19 | 13 |
   | `probe` | 8 | 4 |
   | `galaxy` | 5 | 3 |
   | `research_icon` | 5 | 3 |
   | `pi` | 2 | sort |
   | `economy_score` | 3 | 1 |

   Le total passe à 162.
4. **Tableaux de travail.** `rows`/`bs` de `pico8_cr` et de `pico8_jo_fe`, et `l` de `jo.ex`. Baisses :

   | Fichier | Avant | Après |
   |---|---|---|
   | `menu` | 9 | 5 |
   | `planet_display` | 10 | 6 |
   | `research_display` | 8 | 6 |

   Le total passe à 152.
5. **Sites prouvés par lecture.** `jo_gh_row_items`, `pico8_menu_invented_lines`, le bloc de `pico8_hp_jb` et `concat_percent`.
   - Chacun a son test de survie à la remise à zéro.
   - Leurs `/* TROU-7 */` sont retirés.
   - `pico8_economy_score.c` sort de la liste.

   Baisses :

   | Fichier | Avant | Après |
   |---|---|---|
   | `menu` | 5 | 4 |
   | `planet_display` | 6 | 4 |
   | `research_display` | 6 | 5 |
   | `economy_score` | 1 | sort |

   À la fin : 32 fichiers, 147 appels.
6. **Clôture de l'étape.**
   - La note « Découvert » sous l'entrée 1 du plan : sites classés autrement que dans le §2, sites renvoyés par crash ASan, limite frame relevée ou non, comptes laissés, écarts entre prévision et mesure.
   - La correction de la ligne « Frame » du §2.
   - Les lignes « *Reçu de l'étape 1 :* … » sous les entrées des étapes concernées (§5) :
     - **sous l'étape 2** : les boîtes `vec` remplacées à chaque tick dans un champ d'objet d'interface (`ra` de `iv`, de `js`, `qa` de `ui`, `qf`/`ra` de `pz`, `spec*.ra` de `pj:ph`) font une allocation par tick sans libération. Elles relèvent du point de libération (2a), pas de la zone frame ;
     - **sous l'étape 4** : de même pour celles des objets de partie (`p`/`v`/`ra` de `qb`, `ra` de `nf`, `mo->v`, `re`/`p` de `jc:hq`, `p` de `pico8_bv`, `ie.gy`). L'état `oh` de `pico8_bg_pq` est un objet de frame fait par `pico8_jf`, qui ne prend pas de zone : l'étape 4, qui donne une zone aux états de `pico8_gen.c`, le range en zone frame.

Les avant/après des tables par morceau sont des prévisions (voir « Baisse attendue »). Le commit écrit la liste mesurée, à égalité stricte. Un site qui fait planter `make sanitize-net` dans les morceaux 2 à 5 n'est pas converti dans ce commit : il reste à la transitoire, son compte ne baisse pas d'autant, et il est noté pour « Découvert ». Revenir à la transitoire après une conversion déjà commitée ferait monter un compte : c'est interdit. Un site ne se convertit donc qu'une fois `sanitize-net` vert sur le commit qui le convertit.

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur :
  - un texte rendu, le contenu d'une liste, le dessin d'un panneau ;
  - un code de sortie, un rapport ASan ;
  - la sortie d'`alloc-guard`, l'empreinte d'un e2e.

  Il ne regarde ni l'adresse d'un bloc ni le remplissage interne de la zone.
- **TU ASan de la zone frame.** Il est construit comme `tmp/test_pico8_zone_choisie` : règle `asan_objs`, entrée dans `UNIT_RUNS_SPECIAL`, `ASAN_OPTIONS=detect_leaks=0`. Cas obligatoires :
  - **Survie dans la frame.** Un bloc pris par la poignée frame reste lisible tant qu'il n'y a pas de remise à zéro.
  - **Mort à la frame suivante.** Un bloc pris par la poignée frame, puis lu après **une** remise à zéro et de nouvelles allocations, plante (`use-after-poison`) dans un processus enfant. Le test s'en tient à une remise à zéro : après deux, la moitié empoisonnée redevient valide (0a).
  - **Branchement sur `update60`.** Le même pointeur, gardé à travers un `pico8__update60()` réel, cart chargé comme dans les TU qui bouclent `update60`/`draw`, plante dans un processus enfant. C'est ce cas qui prouve que la remise à zéro est câblée, pas seulement possible.
  - **Liste en zone frame.** Un `pico8_jp_ents` sur la poignée frame, lu après une remise à zéro, plante. Le même, lu avant, rend les bons éléments.
  - **Initialisation paresseuse.** Le premier accès, sans `pico8_cart_load()`, rend une poignée utilisable.
  - **Zone pleine.** Un débordement sort avec le code 3, et `stderr` contient « zone frame pleine ».
- **Sites prouvés par lecture** : un cas par site, dans le TU ASan ci-dessus ou dans un TU existant passé en règle ASan.
  - On construit le panneau, ou l'on lit `pico8_ky`. Les panneaux exercés sont `jo:gh`, le menu `cr` avec des technologies inventées, et `jo:fe`, qui appelle `hp:jb`.
  - On remet la zone frame à zéro, puis on alloue de nouveau.
  - On exerce alors le résultat sans rapport ASan : dessiner le panneau, ou relire la copie du texte « % ».
  - Si l'un plante, le site va dans les écartés.
- **Tests existants.** `test_pico8_hp`, `test_pico8_lq`, `test_pico8_jb_ol`, les TU du menu, de `pj`, de la sonde et des échanges restent verts sans changement de leurs attentes. Seul ajout possible : une remise à zéro explicite dans un TU qui remplirait la zone en bouclant sans `update60`.
- **Verrou.** `make alloc-guard` (dans `make test`) à égalité stricte, après chaque morceau, avec les comptes des tables ci-dessus. Pas de nouveau cas dans `tests/test_alloc_guard.c` : le format et les symboles comptés ne changent pas.
- **Invariant.** `make e2e` avec les 26 `*.sha256` inchangés, puis `make sanitize-net` sous l'empoisonnement de la zone frame. C'est l'oracle qui tranche un site mal classé.
- **Exemples dans le dépôt** :
  - `tests/test_pico8_zone_choisie.c` et sa règle ASan (lecture périmée après une remise à zéro, zone pleine en 3) ;
  - `tests/test_pico8_zone.c` ;
  - `tests/test_pico8_replay_usage.c` (processus enfant, statut et `stderr`) ;
  - les TU qui bouclent `pico8__update60()`/`pico8__draw()` (`test_pico8_bg`, `test_pico8_ga`, `test_pico8_lq`), pour le cas « branchement sur `update60` ».

## Out of Scope

- Les sites écartés listés plus haut :
  - étape 2 : panneaux et leurs tableaux, messages, boutons, boîtes `vec` et `qz` rangées dans un champ ;
  - étape 3 : particules ;
  - étape 4 : objets et identifiants de partie, états de `pico8_gen.c`.
- Une poignée sur zone à places, la zone interface, particules ou partie.
- Toute modification de `pico8_zone_choisie`, de `pico8_zone_transitoire` ou du script du verrou.
- Donner un paramètre de zone à `pico8_jf`/`pico8_gn`, ou à un autre fabricant que 0b n'a pas paramétré.
- Retirer l'entrée TROU-7 de `docs/trous-de-portage.md` (étape 5).
- Les autres appels libc du cœur (§7 du plan).
- Lancer l'application raylib.

## Vérifié contre 0b

Relu contre `HEAD` 86c7f0d :

- la note « Découvert » de 0b dans `docs/plan-4.md` ;
- `pico8_zone_choisie.h` et `pico8_zone_transitoire.h` ;
- les en-têtes `pico8_str.h`, `pico8_collect.h`, `pico8_geom.h`, `pico8_ents.h`, `pico8_ptrs.h` et `pico8_layout.h` ;
- `tools/alloc_guard_exceptions.txt` et `make alloc-guard` (« 60 objets, 34 allouent, 228 appels ») ;
- `git log -6`.

Le plan est relu dans sa version 2df61ce (méthode révisée) : critère de fin du §5, règle du §4 « ASan ne fait que rétrograder », lignes « *Reçu de l'étape N* », et §3 « Croissance par frame ».

- **H-0b-1 (poignée) — tenue.** Les noms réels sont `pico8_zone_choisie_t` (alloc, grow, release, `bump`), `pico8_zone_choisie_bump(bump)` qui rend la poignée par valeur, et `pico8_zone_alloc`/`_grow`/`_release`. Rendre ne fait rien en zone montante, et grow y copie. Conséquence : la poignée frame est un `static` du module, puisque les listes gardent un pointeur vers elle.
- **H-0b-2 (transitoire et cliquet) — tenue.**
  - `pico8_zone_transitoire` est une poignée globale constante. `src/pico8_zone_transitoire.c` est listé à 3.
  - Seules les références à ce symbole comptent, en plus de la libc. Une référence à la poignée frame ne compte pas.
  - `ents_free`/`ptrs_free` rendent par la zone gardée et ne référencent pas la transitoire : aucune relocation chez les appelants.
- **H-0b-3 (fonctions qui prennent une zone) — tenue, avec une précision.** La zone est toujours le premier argument. Les panneaux gardent la zone de leur créateur, et `items`, `fs`, `ls` et les lignes d'un `jb` y vivent. C'est ce qui fonde la catégorie « prouvés par lecture » : la copie se fait dans la zone du panneau. Les fabricants locaux sont bien restés en `malloc`/`calloc`.
- **H-0b-4 (concaténation) — tenue, nom corrigé.** Le nom est `pico8_concat2(zone, a, b)`, dans `pico8_str`. `pj_concat2`, `cr_concat2`, `ie_concat2` et `pico8_bw_concat2` n'existent plus. La PRD les cite désormais sous ce nom, y compris la recopie de `s->happiness`.
- **H-0b-5 (libération d'un texte) — tenue.** L'appelant d'un texte fait en transitoire le libère toujours par `free` (message de 86eca6e). L'étape 1 supprime ce `free`.
- **H-0b-6 (comptes après 0b) — en échec sur cinq fichiers.** L'estimation oubliait les références de `pico8_lq_new`/`pico8_jb_new` et l'init de `g_cg` :

  | Fichier | Estimé | Mesuré |
  |---|---|---|
  | `menu` | 26 | 27 |
  | `research_display` | 11 | 12 |
  | `score_panel_display` | 18 | 19 |
  | `entity` | 3 | 4 |
  | `cart` | absent | 1 |

  Les autres fichiers de l'inventaire tombent juste. Les références ajoutées sont toutes des panneaux ou des listes de partie, hors étape. Les baisses de l'étape sont donc inchangées (77 + 4) ; seuls les restes de ces fichiers changent. Les tables ci-dessus sont refaites sur la mesure.
- **H-0b-7 (aucune zone de jeu instanciée) — tenue.** Aucun `PICO8_BUMP_ZONE_STORAGE` hors des tests, et `pico8_zone_sizes.h` est inchangé.
- **H-0b-8 (`ne` dans une zone montante) — tenue.** `pico8_ne` fait et rend son texte intermédiaire dans la zone passée, et `pico8_collect.c` est sorti de la liste.
- **H-0b-9 (générateurs) — tenue.** `pico8_jf`/`pico8_gn` ne prennent pas de zone. `pico8_gen.c` reste à 4.
- **H-0b-10 (règle de TU ASan) — tenue.** `tmp/test_pico8_zone_choisie` (`asan_objs`, `UNIT_RUNS_SPECIAL`) sert de modèle.
- **H-0b-11 (fin des hausses) — tenue.** C'est dit dans la note « Découvert » et dans 86388db (« Dernier commit où un compte peut monter »). Chaque morceau de cette PRD ne fait que baisser.

## Further Notes

- **Remplissage des zones (§3, « Croissance par frame »).** Le contrôle vise les zones qui ne sont **pas** remises à zéro à chaque frame. La zone frame n'y est pas soumise. L'étape 1 n'ajoute donc pas l'accesseur de remplissage au module de zone, ni son écriture par le rejeu. Ce sera à l'étape 2, première à instancier une zone qui n'est pas remise à zéro à chaque frame et à en avoir besoin pour son critère des places vivantes. Ajouter ici un accesseur que rien ne lit serait du code spéculatif.

- Aucun outil `Agent` n'était disponible dans cette session. Les points que `to-prd` fait valider par un humain ont donc été tranchés en appliquant moi-même les principes du skill `guideline` :
  - **Hypothèses explicites**, revérifiées ci-dessus.
  - **Simplicité** : un module de zone frame à deux fonctions, et pas de nouveau paramètre de zone pour les fabricants locaux. Le point de remise à zéro est unique, dans le cœur, plutôt que dans chaque pilote.
  - **Changements chirurgicaux** : les sites douteux restent à la transitoire, et aucune boîte `vec` n'est déplacée.
  - **Critères vérifiables** : les comptes par fichier et par morceau, les `*.sha256` inchangés, et `sanitize-net` sous empoisonnement par frame.
- La décision d'inclure les sites « prouvés par lecture » (morceau 5) découle de la règle de fin du plan : tous les sites de la zone. Laissés dehors, ces tableaux de travail fuiraient vers une zone à places ou de partie, où ils rempliraient la zone sans jamais être rendus. Le test de survie à la remise à zéro est le garde-fou qu'exige le §4.
- **Constat pour les étapes 2 et 4**, à reporter dans « Découvert » et en lignes « *Reçu de l'étape 1* » (morceau 6). Le contrôle « Croissance par frame » du §3 le rendra visible. Une dizaine de boîtes `vec` sont remplacées **à chaque tick** par un nouveau `malloc` (`qb:ph` en fait trois par frame). Dans une zone de durée plus longue sans libération, elles la rempliraient en quelques minutes de jeu. Les étapes 2 et 4 devront décider comment les rendre : place rendue, ou boîte tenue dans l'objet. Ce n'est pas une décision de l'étape 1.
- À la fin de l'étape, la note « Découvert » sous l'entrée 1 du plan porte au minimum :
  - les sites classés autrement que dans le §2, et pourquoi ;
  - les sites renvoyés à la transitoire par un crash ASan ;
  - la limite frame (inchangée ou relevée) ;
  - les comptes du cliquet laissés, fichier par fichier, et leur total.
