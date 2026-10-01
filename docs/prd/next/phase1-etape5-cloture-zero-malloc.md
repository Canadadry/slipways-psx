---
title: "Plan 4, étape 5 — Clôture : liste du verrou vide, zone transitoire supprimée, TROU-7 refermé"
description: "Une fois 4h finie, le cœur n'alloue plus que par pico8_zone_transitoire.c (3 appels), que 18 fichiers de TU utilisent encore comme zone adossée à la libc, et TROU-7 est cité par des marqueurs devenus faux ; l'étape 5 donne aux TU leur propre zone de tas, supprime la transitoire et son symbole du verrou, vide la liste d'exceptions et retire TROU-7 de la doc, des sources, du Makefile et du README."
status: needs-triage
---

## Problem Statement

Le plan 4 (`docs/plan-4.md`) veut zéro `malloc`, `calloc`, `realloc` ni `free` dans le cœur (`CORE_SRCS`). Cette PRD traite sa dernière étape, **5, Clôture**.

**Elle est rédigée pendant 4h.** À la commande, 4h n'était pas finie : sa PRD est en file (`docs/prd/triage/phase1-etape4h-singletons-et-objets-du-joueur.md`). Tout ce que cette PRD suppose de l'issue de 4h est écrit comme hypothèse `H-4h-n` (section « Hypothèses sur l'étape 4h »). Ce qu'il faut relire est listé dans « À revérifier une fois 4h finie ».

Le contenu lu est toujours du contenu commité, jamais l'arbre de travail :
- **a61134c** (`HEAD` à la rédaction : 4g close à 5c3ea7e, PRD 4h en file), lu par `git archive` ;
- le **prototype de 4h** (copie `git archive` de 5c3ea7e, mêmes sources que a61134c, hors dépôt), sur lequel l'étape 5 a été prototypée à son tour (voir « Further Notes »).

**Le texte de l'étape**, recopié du plan à a61134c :

> 5. **Clôture.** La liste d'exceptions du verrou est vide (0a : aucune exception permanente) et `pico8_zone_transitoire.c` est supprimé ; `make alloc-guard` le montre. `TROU-7` est retiré de `docs/trous-de-portage.md` et des sources qui le citent, et l'entrée `malloc` du `README` est retirée.

Et ce que le plan en dit ailleurs :

> - (0b) Une zone **transitoire**, adossée à la libc, reproduit le comportement actuel (l'appelant libère encore) : c'est elle que passe tout appelant pas encore converti. Ses allocations sont comptées par le cliquet comme les autres ; les étapes 1 à 4 en retirent leurs appelants, l'étape 5 la supprime.
> - (0b, *Découvert*) Le verrou compte chaque référence à `pico8_zone_transitoire` […].
> - (0a, *Découvert*) `pico8_coverage.c` sort de `CORE_SRCS` : aucune exception permanente au verrou.

**Ce qui se passe aujourd'hui** (a61134c, et prototype de 4h) :

- **Le verrou.** `make alloc-guard` répond à a61134c « verrou objets 65 allouent 8 appels 20 », et sur le prototype de 4h « verrou objets 65 allouent 1 appels 3 ». La seule ligne restante est `src/pico8_zone_transitoire.c 3` (son `malloc`, son `realloc`, son `free`). `tools/alloc_guard.sh` compte, en plus des quatre noms de la libc, toute relocation vers le symbole `pico8_zone_transitoire` ; l'en-tête de la liste d'exceptions et l'aide du `Makefile` le disent aussi.
- **La transitoire dans le cœur.** À a61134c, seul `pico8_ui.c` la cite encore (inclusion, et deux `pico8_ents_init` des 16 calques `ic`), et 4h l'en retire. `pico8_zone_transitoire.c` est dans `ENGINE_SRCS` (donc dans `CORE_SRCS`) et dans `DATA_ENGINE_SRCS`.
- **La transitoire dans les TU.** 19 fichiers de `tests/` la nomment :
  - `test_alloc_guard.c` fabrique des objets qui référencent le symbole, pour tester qu'il est compté (trois cas) ;
  - 18 autres l'utilisent comme **zone adossée à la libc** (117 occurrences) : `test_pico8_ents`, `_ptrs`, `_str`, `_lq`, `_jb_ol`, `_js`, `_ie`, `_hy`, `_fr`, `_mx`, `_nr`, `_os`, `_trade`, `_struct_header`, `_layout_nil_item_draws_nothing`, `_zone_frame`, `_zone_arene`, `_zone_choisie`. Ils y font grandir des listes `ents`/`ptrs`, des textes (`sub`, `tostr_fixed`, `ne`), des panneaux `lq`/`jb`, et y prennent des arènes (`pico8_zone_arene_prendre(&pico8_zone_transitoire, …)`). `test_pico8_zone_choisie` a trois cas qui testent la transitoire elle-même (`transient_round_trip`, `ents_transient_round_trip`, `ne_transient_round_trip`), et son cas `arena_dead_after_release` attend « heap-use-after-free » parce que le parent de l'arène est la libc.
- **La transitoire dans le `Makefile`.** 11 occurrences : `ENGINE_SRCS`, `DATA_ENGINE_SRCS`, les deux lignes de `main_raylib`, et les lignes de lien de sept TU (`geom`, `ptrs`, `zone_arene`, `ents`, `str`, `zone_choisie`, `layout_nil_item_draws_nothing`). `test_pico8_geom` la lie sans s'en servir. Les autres TU qui l'utilisent l'ont par `$(ENGINE_SRCS)` ou `$(DATA_ENGINE_SRCS)`.
- **Ailleurs.** `tools/composition_r2_sites.txt` cite encore, dans sa colonne de justification, `pico8_jp_ents(&pico8_zone_transitoire, pico8_cg("jo"), …)` de `score_panel.c:113` ; le code y passe `pico8_zone_frame()` depuis l'étape 1. `tools/float_guard.txt` (réécrit par `make float-guard`) compte 65 fichiers.
- **TROU-7.** L'entrée « Fuites mémoire assumées » de `docs/trous-de-portage.md` dit que les constructeurs de rangées et les réaffectations de champ fuient, et renvoie au `README`. À a61134c, 24 sites la citent dans `src/` et `tests/`, plus le `Makefile`. 4h retire ceux de ses propres sites (H-4h-3). Restent alors :
  - `pico8_planet.h` (devant `pico8_jo_lz`, et devant `pico8_jo_ldown`, avec « nothing frees it ») ;
  - `pico8_planet_display.h` (devant `pico8_jo_fe`) ;
  - `pico8_planet_family_trade.h` (devant `pico8_nr_ldown`, avec « nothing ever frees the instance ») ;
  - `pico8_planet_trade.h` (devant `pico8_jo_cn`) ;
  - `pico8_planet_trade.c` (devant le `break` de `pico8_jo_ib`, qui abandonnait `lw.t`) ;
  - `tests/test_pico8_planet_gh_row_lq.c` (marqueur, et « The target runs with detect_leaks=0 ») ;
  - le `Makefile` : « detect_leaks=0 : les entités ne sont jamais libérées (TROU-7). » devant `pico8_replay_sanitize`, dont la règle `sanitize-run-%` lance `ASAN_OPTIONS=detect_leaks=0` ; et, sans le numéro, « detect_leaks=0 au lancement, ce port fuit par conception » devant `test_pico8_planet_gh_row_lq`, que `UNIT_RUNS_SPECIAL` lance sous `detect_leaks=0`.
- **Le `README`.** Sa liste « À faire » porte « Remplacer malloc par des zones mémoire de taille fixe, en suivant `docs/plan-4.md`. »

**Le problème.** Tant que la transitoire existe, le cœur garde une porte vers la libc : une zone choisie qu'on peut passer à n'importe quelle fonction partagée, comptée par un symbole spécial au lieu des quatre noms de la libc. Les TU qui s'en servent empêchent de la supprimer. Et TROU-7 annonce une fuite que plus rien ne fait : ses marqueurs restants décrivent des sites déjà convertis par les étapes 1 à 4, et `detect_leaks=0` cache, sur `sanitize-net`, la preuve qu'il n'y a plus de fuite.

## Solution

- **Les TU ont leur propre zone de tas.** Un petit module de test, hors de `src/`, fournit une zone choisie adossée à `malloc`/`realloc`/`free`. Les 18 fichiers de TU qui passaient la transitoire passent cette zone, et la lient. Les trois cas de `test_pico8_zone_choisie` qui testaient la transitoire elle-même disparaissent.
- **La transitoire est supprimée** : son `.c`, son `.h`, ses mentions dans les listes de sources et de lien du `Makefile`.
- **Le verrou ne compte plus que la libc.** `tools/alloc_guard.sh` oublie le symbole `pico8_zone_transitoire` et garde `malloc`, `calloc`, `realloc`, `free`. La liste d'exceptions n'a plus que son en-tête. `make alloc-guard` répond **« verrou objets 64 allouent 0 appels 0 »** (mesuré sur le prototype).
- **TROU-7 est refermé.** L'entrée sort de `docs/trous-de-portage.md`, chaque marqueur sort des sources et des TU, et le contrat faux qui l'accompagne (« nothing frees it ») est réécrit. `sanitize-net` et `test_pico8_planet_gh_row_lq` tournent sous LSan (sans `detect_leaks=0`) : c'est la preuve mécanique qu'aucun e2e ne fuit plus. Le prototype : « sanitize: 26 propre(s) » avec LSan actif.
- **Le `README` perd son entrée `malloc`.**
- **Le dernier vocabulaire de la libc** sort des commentaires du cœur (`malloc`, « heap », « leaked » comme mécanisme actuel), et `<stdlib.h>` sort des fichiers du cœur qui n'en utilisent plus rien.

Aucun pixel ne change. Le prototype garde les 26 `*.sha256`, `make test-all` est vert, `make sanitize-net` répond « 26 propre(s) » sous LSan, et `make raylib` compile et lie (sans être lancé).

## User Stories

1. En tant que mainteneur du plan 4, je veux que `make alloc-guard` réponde « allouent 0 appels 0 » avec une liste d'exceptions vide, pour constater l'objectif du plan : zéro `malloc`, `calloc`, `realloc`, `free` dans le cœur.
2. En tant que mainteneur du plan 4, je veux que `pico8_zone_transitoire.{h,c}` n'existe plus, pour que plus aucun appelant du cœur ne puisse retrouver la libc par une zone choisie.
3. En tant que mainteneur du plan 4, je veux que `pico8_zone_transitoire.c` sorte de `ENGINE_SRCS`, donc de `CORE_SRCS`, de `DATA_ENGINE_SRCS` et du lien de `main_raylib`, pour que le cœur compile et se lie sans elle.
4. En tant que mainteneur, je veux que le verrou ne compte plus que les quatre noms de la libc, pour que la règle soit celle du §1 du plan et rien de plus.
5. En tant que mainteneur, je veux que le verrou accepte une liste d'exceptions faite de son seul en-tête, et échoue encore si un fichier du cœur se met à allouer, pour que le cliquet reste armé une fois vide.
6. En tant que mainteneur, je veux un TU du verrou qui montre qu'une référence à un symbole qui n'est pas l'un des quatre noms (l'ancienne poignée comprise) ne compte pas, pour fixer ce que le verrou compte désormais.
7. En tant qu'auteur de TU, je veux une zone de tas propre aux tests, adossée à la libc, pour continuer à tester les listes, textes, panneaux et arènes avec les diagnostics de tas d'ASan (débordement d'un bloc à sa taille exacte, usage après `free`).
8. En tant qu'auteur de TU, je veux que cette zone vive dans `tests/` et ne soit liée que par les TU, pour qu'un objet du cœur qui la nommerait ne se lie pas.
9. En tant qu'auteur de TU, je veux que les cas d'arène gardent leur diagnostic « heap-use-after-free » après le rendu de l'arène, pour que `test_pico8_zone_choisie` ne change pas d'oracle.
10. En tant qu'auteur de TU, je veux que les trois cas qui testaient la transitoire elle-même disparaissent, pour ne pas tester une zone de test.
11. En tant que mainteneur, je veux que `test_pico8_geom` ne lie plus un module dont il ne se sert pas.
12. En tant que mainteneur, je veux que `grep -rn zone_transitoire` hors de `docs/plan-4.md` et `docs/prd/` ne trouve plus rien, colonne de justification de `tools/composition_r2_sites.txt` comprise, pour qu'aucun texte ne renvoie à un module disparu.
13. En tant que lecteur des trous de portage, je veux que l'entrée TROU-7 soit supprimée, selon la convention du fichier (« une entrée supprimée est une entrée refermée »), sans renuméroter les autres.
14. En tant que lecteur du code, je veux qu'aucun `/* TROU-7 */` ne reste dans `src/` ni `tests/`, pour qu'aucun marqueur ne renvoie à une entrée absente.
15. En tant que lecteur du code, je veux que les contrats « nothing frees it » de `pico8_jo_ldown` et `pico8_nr_ldown` disent ce qui est vrai depuis 4e (le `jc` est pris dans l'arène de prise, qu'il rend s'il est refusé), pour que le commentaire reste vrai (règle du `CLAUDE.md`).
16. En tant que gardien de l'invariant, je veux que `make sanitize-net` tourne sous LSan, pour qu'une fuite d'un e2e le fasse échouer au lieu d'être tue.
17. En tant que gardien de l'invariant, je veux que `test_pico8_planet_gh_row_lq` tourne lui aussi sous LSan, puisque sa seule raison de `detect_leaks=0` était TROU-7.
18. En tant que mainteneur, je veux que les commentaires du `Makefile` qui justifiaient `detect_leaks=0` par TROU-7 ou par « ce port fuit par conception » disparaissent avec l'option qu'ils justifiaient, ou soient réécrits s'ils couvrent encore une autre ligne.
19. En tant que lecteur du `README`, je veux que l'entrée « Remplacer malloc… » quitte la liste « À faire », pour que la liste ne porte que ce qui reste à faire.
20. En tant que lecteur du code, je veux que les commentaires du cœur qui décrivent un `malloc`, un vecteur ou une liste « heap », ou un objet « leaked » comme mécanisme actuel soient réécrits, pour que le code ne dise plus le contraire du verrou.
21. En tant que mainteneur, je veux que `<stdlib.h>` sorte des fichiers du cœur qui n'en utilisent plus rien, vérifié par la compilation `-Werror`, pour ne pas laisser une porte d'entrée évidente à `malloc`.
22. En tant que gardien de l'invariant, je veux que les 26 `*.sha256` restent identiques à chaque commit, pour prouver que la clôture ne change pas un pixel.
23. En tant que mainteneur, je veux que `make raylib` compile et lie encore (sans être lancé), pour que le binaire interactif ne cherche plus la transitoire.
24. En tant que mainteneur du plan 4, je veux une note « Découvert » sous l'entrée 5, avec un récapitulatif final des zones (taille, maximum relevé, somme sur 1,5 Mo), pour que le plan se ferme sur une mesure.
25. En tant que mainteneur du plan 4, je veux que la ligne « *Reçu de l'étape 4* » que 4h écrira sous l'entrée 5 soit traitée ici ou que la PRD dise pourquoi non.
26. En tant que mainteneur du plan 4, je veux que tout site qui fuirait encore après 4h soit nommé et bloque la clôture, plutôt que d'être caché par le retrait de TROU-7.
27. En tant qu'agent sans humain, je veux que toute hypothèse sur l'issue de 4h soit numérotée et revérifiée avant l'implémentation, pour ne pas bâtir sur une conception que 4h aurait changée.
28. En tant que mainteneur, je veux que les points hors périmètre du §7 du plan restent intacts (table `pico8_quarter_wave`, chaînes d'`assert`, assets JSON, budget de slipways, `fopen`/`snprintf`/`__divdi3` du cœur), pour que la clôture ne déborde pas sur le chantier PSX.

## Implementation Decisions

### Ce qui remplace la transitoire dans les TU

**Décision : une zone de tas propre aux TU**, un module de test (`tests/zone_tas.{h,c}` dans le prototype, symbole `zone_tas`), adossé à `malloc`/`realloc`/`free`, avec la forme exacte de l'ancienne transitoire. Les 18 fichiers remplacent `&pico8_zone_transitoire` par `&zone_tas` et l'inclusion de l'en-tête ; chacun ajoute le module à sa ligne de lien.

Forme retenue par le prototype (en-tête entier) :

```c
/* Zone des TU adossee a malloc, realloc et free : un bloc qu'elle alloue se
 * libere aussi par free. */
extern const pico8_zone_choisie_t zone_tas;
```

Pourquoi elle, et pas une autre, cas par cas :

| Usage dans les TU | Fichiers | Pourquoi la zone de tas |
|---|---|---|
| listes `ents`/`ptrs` qui grandissent, `ents_cat`, `jp_ents`, `ptrs_init_reserve` | `ents`, `ptrs`, `fr`, `hy`, `mx`, `nr`, `os`, `trade`, `lq` | `realloc` donne un bloc à sa taille exacte : ASan attrape un débordement d'un élément, qu'une zone montante (blocs voisins dans le même stockage) laisserait passer. `pico8_ents_free` y libère vraiment |
| textes `sub`, `tostr_fixed`, `ne` | `str` | même raison ; les cas comparent des chaînes, rien ne dépend de la durée de vie |
| panneaux `lq`/`jb` et leurs copies | `lq`, `jb_ol`, `js`, `ie`, `struct_header`, `layout_nil_item_draws_nothing`, `zone_frame` | un panneau garde sa zone pour `lq_add` ; aucun de ces cas ne remet de zone à zéro. `zone_frame` y construit une rangée qui doit **survivre** à la remise à zéro frame : il lui faut une zone qui dure |
| parent d'une arène | `zone_arene`, `zone_choisie` (9 cas), `ents` (3 cas) | le cas `arena_dead_after_release` attend « heap-use-after-free » : le parent doit être le tas. `arena_vider_*`, `arena_taken_*`, `reserved_list` ne dépendent que de l'arène |

Écartés :
- **la zone frame ou la zone partie du jeu** : il faudrait les initialiser et les remettre à zéro dans chaque TU, l'oracle ASan change (« use-after-poison »), et une zone montante ne voit pas un débordement d'un bloc sur le suivant ;
- **une arène par TU** : même cécité au débordement, et une taille à choisir par cas ;
- **une zone locale recopiée dans chaque fichier** : 18 copies du même code.

**Ce n'est pas une transitoire rebaptisée.** Elle est hors de `src/`, hors de `ENGINE_SRCS` : un fichier du cœur qui la nommerait ne se lierait ni dans `pico8_replay` ni dans `main_raylib`. Les TU sont hors de `CORE_SRCS` : la libc y est permise (§1 du plan).

Dans `test_pico8_zone_choisie`, les cas `transient_round_trip`, `ents_transient_round_trip` et `ne_transient_round_trip` sont **supprimés** (ils testaient la zone elle-même). Les autres cas, et leurs attentes, sont inchangés.

### Suppression de la transitoire

- `src/pico8_zone_transitoire.{h,c}` supprimés.
- `Makefile` : retirée de `ENGINE_SRCS` (dernière ligne de la liste, la ligne d'avant perd son `\`), de `DATA_ENGINE_SRCS`, des deux lignes de `main_raylib`, et des sept lignes de lien qui la nomment. Les six TU qui s'en servaient y prennent la zone de tas à sa place ; `test_pico8_geom` la perd sans remplacement. Les douze TU qui l'avaient par `$(ENGINE_SRCS)`/`$(DATA_ENGINE_SRCS)` ajoutent la zone de tas à leur ligne.
- `tools/float_guard.txt` est réécrit par `make float-guard` : « garde fichiers 64 en-echec 0 ».
- `tools/composition_r2_sites.txt` : la justification de `pico8_eg_gg_pred` cite `pico8_jp_ents(pico8_zone_frame(), …)`, l'appel réel.

### Verrou

- `tools/alloc_guard.sh` : le test du symbole perd `|| sym == "pico8_zone_transitoire"` ; son commentaire d'en-tête ne parle plus de la poignée. Les quatre noms (et leur forme `_malloc` de Darwin) restent.
- `tools/alloc_guard_exceptions.txt` : plus aucune ligne de données. L'en-tête dit ce qu'est la liste et qu'elle est vide ; il ne parle plus de la poignée.
- **Une liste réduite à son en-tête est acceptée** : le script retire les commentaires (`sed 's/#.*//'`) puis ne lit que les lignes à deux champs. Avec des objets propres, le rapport est vide et le script rend 0 ; un objet qui alloue sort « hors liste ; ligne a ajouter ». Vérifié par un TU du prototype.
- L'aide en tête du `Makefile` (`make alloc-guard`) ne parle plus des « références à pico8_zone_transitoire ».
- **Sortie attendue** : « verrou objets 64 allouent 0 appels 0 » (65 objets moins la transitoire), mesurée sur le prototype.

### Fermeture de TROU-7

**Chaque site restant est-il encore une fuite ?** Lus un par un sur le prototype (état après 4h) :

| Site | Ce que TROU-7 y disait | Aujourd'hui | Fuite ? |
|---|---|---|---|
| `pico8_jo_lz` | `jy`, `ep` refaits sans libérer l'ancien | fait dans la zone de construction (prise ou génération), une fois par planète : par `jo:pq` pour un `qh`, ou par `jo:hu` au passage à « ly », qui n'a lieu qu'une fois | non : une fois par objet de partie, rendu avec la génération |
| `pico8_jo_ldown`, `pico8_nr_ldown` | « nothing frees » le `jc` rendu | le `jc` prend l'arène de prise (4e) et la rend s'il est refusé | non ; **le contrat est faux** et se réécrit |
| `pico8_jo_fe` | rangées `bs`/`rows` jamais libérées | zone frame (étape 1) ; les boutons de `pz` recopient dans leur place (étape 2) | non |
| `pico8_jo_cn` | `jy` refait | génération, une fois par planète (gardé par `has_jh`) | non |
| boucle de `pico8_jo_ib` (`break`) | `lw.t` abandonnée | route candidate en zone frame (4e) | non |
| `test_pico8_planet_gh_row_lq` | `detect_leaks=0` | le TU passe sous LSan (prototype) | non |
| `Makefile`, `sanitize-net` | `detect_leaks=0` | les 26 e2e passent sous LSan (prototype) | non |

**Conclusion : aucun site ne fuit encore** sur l'état prototypé de 4h. Rien ne bloque la clôture. Si la revérification trouve une fuite (H-4h-4), **l'étape s'arrête** : le site est nommé dans « Découvert », TROU-7 reste, et on le signale au lieu de le cacher.

**Ce qui change :**
- `docs/trous-de-portage.md` : la section « TROU-7 — Fuites mémoire assumées » est supprimée ; TROU-8 et les suivants gardent leur numéro.
- Les marqueurs `/* TROU-7 */` des sites ci-dessus sont supprimés. Les contrats de `pico8_jo_ldown` et `pico8_nr_ldown` sont réécrits : le `jc` rendu est pris dans l'arène de prise et la rend s'il est refusé.
- `tests/test_pico8_planet_gh_row_lq.c` perd son marqueur et sa phrase « The target runs with detect_leaks=0 ».
- `Makefile` :
  - `sanitize-run-%` lance `pico8_replay_sanitize` **sans** `ASAN_OPTIONS=detect_leaks=0`, et le commentaire « detect_leaks=0 : … (TROU-7) » disparaît ;
  - `test_pico8_planet_gh_row_lq` sort de `UNIT_RUNS_SPECIAL` et perd sa règle `detect_leaks=0` ; son commentaire de construction perd « detect_leaks=0 au lancement, ce port fuit par conception » et garde la raison ASan, qui reste vraie.
  - Les onze autres TU de `UNIT_RUNS_SPECIAL` **gardent** `detect_leaks=0` : ce sont leurs propres allocations de test qui fuient (le prototype en voit dans `test_pico8_zone_frame` et `test_pico8_iv`, toutes faites par le TU), pas le cœur.
- **Pourquoi LSan sur `sanitize-net`, et pas seulement un relevé.** Le verrou empêche déjà le cœur d'appeler la libc ; LSan garde le reste du rejeu (hôte). Le retrait de `detect_leaks=0` n'ajoute rien : il retire une option dont la raison écrite (TROU-7) disparaît. La garder demanderait une nouvelle raison, et il n'y en a pas : les 26 e2e sont propres.
- **Hors des sources** : `docs/plan-4.md` et les PRD de `docs/prd/` citent TROU-7 dans leur historique ; ils ne sont pas réécrits. Le `CLAUDE.md` et le skill `reprendre-les-commentaires` prennent `/* TROU-7 */` comme **exemple de format** de marqueur ; un agent ne modifie pas le `CLAUDE.md` (voir « Further Notes »).

### README

L'entrée « Remplacer malloc par des zones mémoire de taille fixe, en suivant `docs/plan-4.md`. » quitte la liste « À faire ». Rien d'autre ne change dans le `README`.

### Dernier vocabulaire de la libc dans le cœur

Le `grep -n -i -E "malloc|calloc|realloc|\bheap|leak"` sur les fichiers de `CORE_SRCS` et leurs en-têtes, à a61134c, hors appels, trouve :

| Site | Texte | Sort |
|---|---|---|
| `pico8_struct.h:116` | « its `malloc` leaves the header indeterminate » | réécrit : le bloc de zone est indéterminé |
| `pico8_economy_tiers.h:11` | « `cc` is a heap list … the one a previous call installed is freed » | réécrit : `cc` est vidée sur place à chaque `bd` (4d) |
| `pico8_entity_geom.h:9` | « a heap vector the field borrows » | réécrit : « vector » sans « heap » |
| `pico8_cart.c:188` | « `eh`'s heap vector » | idem |
| `pico8_entity.c:281` | « deliberately leaked rather than freed » | 4h (`entity`) ; sinon ici |
| `pico8_mouse_reader.h:43`, `pico8_scroll.h:42` | « fresh heap vector(s) » | 4h ; sinon ici |
| `pico8_zone_choisie.c:10` | « pas de realloc : … pas dans le tas » | **gardé** : vrai, il dit pourquoi on copie |

Après l'étape, ce `grep` ne trouve que `pico8_zone_choisie.c:10`.

`<stdlib.h>` sort de `pico8_economy_score.c`, `pico8_menu.c`, `pico8_pi.c` (et de `pico8_probe.c` si 4h l'y a laissé, H-4h-7) : le prototype les compile en `-Werror` sans elle. `pico8_gfx.c` la garde (`abs`).

Ne sont **pas** réécrits ici les contrats « never freed », « does not free », « free grabs » : ils restent vrais. Trois contrats « frees » qui semblent faux depuis les étapes précédentes sont signalés dans « Further Notes », pas corrigés.

### Hors périmètre du §7, vérifié intact

Le diff de l'étape ne touche ni `src/pico8_quarter_wave.h`, ni les chaînes d'`assert`, ni le chargement des assets, ni le budget de slipways et de relais. Les appels libc du cœur autres que les quatre noms (`fopen`, `snprintf`, `__divdi3`, …) restent : le verrou ne les compte pas.

### Note « Découvert » et clôture du plan

À la clôture, sous l'entrée 5 de `docs/plan-4.md`, une note « *Découvert (5) :* » de quelques phrases :
- la zone de tas des TU (où, pourquoi pas une zone du jeu), les trois cas supprimés ;
- le verrou : symbole retiré, liste vide acceptée, « verrou objets 64 allouent 0 appels 0 » ;
- TROU-7 : les sites relus, aucun ne fuyait encore ; `sanitize-net` et `gh_row_lq` sous LSan ; les contrats réécrits ;
- le traitement de la ligne « *Reçu de l'étape 4* » ;
- **le récapitulatif final des zones**, mesuré sur les 26 e2e (valeurs du prototype, à refaire) :

| Zone | Taille | Maximum relevé (e2e) |
|---|---|---|
| frame | 256 Ko | 37 408 o (`trade_rich_mine`), relevé à la remise à zéro par une sonde jetable : le rejeu n'écrit pas la zone frame |
| interface | 335 Ko | places vivantes : js 8 / 24, fq 1 / 4, mh 1 / 6, iv 40 / 128, rangée 8 / 100, le 1 / 16 |
| particules | 128 Ko | particule 215 / 2085, qy 3 / 24, qy_fv 1 / 1, ga 1 / 8 (584 o sur 640), ts 13 / 680 |
| partie | 768 Ko | génération 157 704 o (`pause_restart`) sur 704 Ko ; chargement 22 752 o sur 64 Ko |
| **Somme** | **1487 Ko sur 1536** | — |

Aucune limite n'est relevée par l'étape 5.

La PRD est retirée de `docs/prd/` dans le commit de clôture. `docs/plan-4.md` n'est **pas** supprimé ici : `plan-pilot` le lit encore pour la revue de fin de phase (`warn-review`, `plan-review`). Sa suppression, prévue par le `CLAUDE.md` pour un plan fini, vient après.

## Implementation Chunks

Chaque morceau est un commit qui respecte l'invariant du §3 :
- `make test-all` et `make sanitize-net` passent ;
- les 26 `*.sha256` sont identiques ;
- `alloc-guard` est à égalité stricte.

**Aucun ne commence avant la revérification de « À revérifier une fois 4h finie ».**

1. **Les TU quittent la transitoire.**
   - Le module de zone de tas dans `tests/`. Les 18 fichiers passent à lui. Les lignes de lien l'ajoutent. Les trois cas de `test_pico8_zone_choisie` qui testaient la transitoire sont supprimés.
   - La transitoire existe encore dans le cœur : la liste reste `src/pico8_zone_transitoire.c 3`.
   - Vérification : `grep -l pico8_zone_transitoire tests/*.c` ne rend plus que `test_alloc_guard.c`.
2. **La transitoire supprimée, le verrou vide.**
   - Le test d'abord : dans `test_alloc_guard`, les trois cas de la poignée sont remplacés par « une référence au symbole `pico8_zone_transitoire` ne compte pas » (rouge aujourd'hui : elle compte) et « liste réduite à son en-tête, objets propres : 0 et "allouent 0 appels 0" ; un objet qui alloue : ligne à ajouter » (vert aujourd'hui : c'est la garde du cliquet vide).
   - Suppression des deux fichiers, des 11 mentions du `Makefile`, du symbole du script, des lignes de la liste ; aide du `Makefile`, `float_guard.txt`, `composition_r2_sites.txt`.
   - Le verrou : **1 / 3 → 0 / 0**, « verrou objets 64 allouent 0 appels 0 ».
   - `make raylib` compile et lie (sans lancement).
3. **TROU-7 refermé, README, dernier vocabulaire.**
   - Le test d'abord : `make sanitize-net` sans `detect_leaks=0` (vert sur le prototype ; rouge à a61134c, où `pause_restart` fuit 1 792 o en 80 blocs) ; `test_pico8_planet_gh_row_lq` sous LSan.
   - Entrée TROU-7 supprimée ; marqueurs et contrats faux ; `Makefile` ; `README` ; commentaires « malloc/heap/leaked » du cœur ; `<stdlib.h>` inutiles.
   - Vérification : `grep -rnw "TROU-7" src tests tools Makefile README.md docs/trous-de-portage.md` vide.
4. **Mesure et clôture du plan.**
   - Le récapitulatif des zones sur les 26 e2e (`PICO8_REMPLISSAGE`, et la zone frame par une sonde jetable hors dépôt).
   - La note « Découvert (5) », le traitement du « Reçu de l'étape 4 », le retrait de cette PRD.

Ordre : 1 avant 2 (la transitoire ne peut partir tant qu'un TU la lie) ; 3 après 2 (le verrou vide d'abord, la preuve LSan ensuite) ; 4 en dernier. 3 pourrait précéder 2 sans risque, mais l'ordre du texte de l'étape est gardé.

## Testing Decisions

- **Ce qu'est un bon test ici.** Il observe ce qui se voit de l'extérieur : la sortie et le code de retour de `alloc_guard.sh` ; le rapport ASan/LSan d'un rejeu ou d'un TU ; l'empreinte d'un e2e ; un `grep` sur le dépôt. Il ne teste pas la zone de tas des TU : c'est un outil de test.
- **Verrou** (`tests/test_alloc_guard.c`, sur des objets fabriqués comme ses cas actuels) :
  - un objet qui référence `pico8_zone_transitoire` (code et données) sans appeler la libc compte 0 : rouge avant le morceau 2 ;
  - une liste faite de commentaires seuls passe sur des objets propres, avec « allouent 0 appels 0 », et échoue avec « ligne a ajouter » dès qu'un objet alloue ;
  - les cas existants des quatre noms, de Darwin et de `pico8_free` (non compté) restent.
- **Arènes et listes** : `test_pico8_zone_choisie`, `test_pico8_zone_arene`, `test_pico8_ents` gardent leurs attentes, « heap-use-after-free » compris. Le prototype les a vus verts.
- **Fuites** : `make sanitize-net` sous LSan sur les 26 e2e, et `test_pico8_planet_gh_row_lq` sous LSan. Le prototype : 26 propres, TU propre. Pour mémoire, LSan est actif dans ce conteneur (un programme qui perd 77 o est signalé).
- **Invariant** : `make e2e` (26 `*.sha256` inchangés), `make test-all`, `make sanitize-net`, `make raylib` (compile et lie, jamais lancé).
- **Contrôles par `grep`**, écrits dans le message du commit de clôture :
  - `pico8_zone_transitoire` hors `docs/plan-4.md` et `docs/prd/` : vide ;
  - `TROU-7` (mot entier) dans `src`, `tests`, `tools`, `Makefile`, `README.md`, `docs/trous-de-portage.md` : vide ;
  - le `grep` du vocabulaire libc du cœur : seulement `pico8_zone_choisie.c:10`.
- **Exemples dans le dépôt** : `tests/test_alloc_guard.c` (objets fabriqués, faux `objdump`) ; `tests/test_pico8_zone_choisie.c` (cas en processus enfant, `expect_freed_crash`).

## Out of Scope

- Tout ce que fait 4h : boîtes par tick, singletons en génération, sonde dans l'arène de prise, et les marqueurs TROU-7 et contrats de ses propres sites (`scroll`, `mouse_reader`, `bg`, `ui`, `probe`, `probe_comp`, `score_panel`, `entity`, `cart`). L'étape 5 ne reprend que ce que 4h aurait laissé (H-4h-3).
- Le §7 du plan : table `pico8_quarter_wave`, chaînes d'`assert`, assets en binaire, budget de slipways et de relais, appels libc autres que les quatre noms.
- Réduire une zone sous sa taille actuelle (la génération mesure ~154 Ko sur 704) : c'est une économie PSX (§7).
- Exposer le remplissage de la zone frame dans le rejeu : le récapitulatif la mesure par une sonde jetable, hors dépôt.
- Les `detect_leaks=0` des onze autres TU de `UNIT_RUNS_SPECIAL` : leurs fuites sont celles du TU.
- Les contrats « frees » douteux de `pico8_cart.h`, `pico8_data_init.h` et `pico8_help.h` (voir « Further Notes »).
- Modifier le `CLAUDE.md` ou les skills qui prennent `/* TROU-7 */` comme exemple de format.
- Supprimer `docs/plan-4.md`.
- L'audio, lancer l'application raylib.

## Hypothèses sur l'étape 4h

Chacune est ce que cette PRD suppose de 4h, lue dans sa PRD (`docs/prd/triage/phase1-etape4h-singletons-et-objets-du-joueur.md` à a61134c) et vue sur son prototype. Pour chacune : ce qui change ici si elle tombe.

- **H-4h-1 (verrou).** Après 4h, la liste ne contient que `src/pico8_zone_transitoire.c 3`, et `make alloc-guard` répond « verrou objets 65 allouent 1 appels 3 ». Aucun fichier de `src/` ne nomme plus la transitoire (`pico8_ui.c` ne l'inclut plus ; ses calques `ic` sont en zone frame). *Si elle tombe* : chaque fichier encore listé est un site non converti ; **l'étape 5 ne commence pas**, on le signale (il revient à l'étape qui le possède).
- **H-4h-2 (périmètre de 4h).** 4h ne supprime pas la transitoire, ne touche ni `alloc_guard.sh`, ni l'en-tête de la liste, ni les usages de la transitoire dans les TU, ni ses mentions du `Makefile`. *Si elle tombe* : l'étape 5 part de ce qui reste ; un morceau peut devenir vide.
- **H-4h-3 (TROU-7 de 4h).** 4h retire les marqueurs et réécrit les contrats de ses sites : `pico8_scroll.h` (2), `pico8_mouse_reader.h`, `pico8_bg.h`, `pico8_ui.c`, `pico8_ui.h`, `pico8_probe.h`, `pico8_probe_comp.h`, `pico8_score_panel.h` (2), `pico8_entity.c`, `pico8_entity.h`, `pico8_cart.c` (devant `pj_new`). Restent les six sites des autres familles, le TU `gh_row_lq` et le `Makefile`. *Si elle tombe* : l'étape 5 retire aussi ceux-là, avec la même réécriture de contrat (le prototype de 4h ne les avait pas retirés : sur lui, 18 marqueurs restaient dans `src/`).
- **H-4h-4 (plus de fuite du cœur).** Après 4h, aucun e2e ne fuit sous LSan, et `test_pico8_planet_gh_row_lq` non plus. Constaté sur le prototype (26 e2e propres, un par un ; la PRD 4h note la même chose sur ses rejeux d'affilée, hors outil de mesure). *Si elle tombe* : la fuite est nommée (site, octets, blocs) ; **TROU-7 reste et l'étape s'arrête** après son morceau 2, en le signalant.
- **H-4h-5 (zones).** 4h ne change aucune taille de zone : somme 1487 Ko ; génération au plus 157 704 o (`pause_restart`), chargement 22 752 o ; `ga` 584 o dans des places de 640. *Si elle tombe* : le récapitulatif reprend les valeurs de la note « Découvert (4h) » ; une somme au-delà de 1536 Ko est déjà un arrêt de 4h, pas de l'étape 5.
- **H-4h-6 (« Reçu de l'étape 4 »).** 4h écrit sous l'entrée 5 : la liste ne contient plus que la transitoire, et aucun fichier du cœur ne la référence ; les `/* TROU-7 */` qui restent sont ceux des autres familles, à relire un par un. Cette PRD les relit (tableau « Fermeture de TROU-7 »). *Si elle tombe* : le nouveau « Reçu » est traité, ou la PRD dit pourquoi non.
- **H-4h-7 (`<stdlib.h>`).** 4h retire `<stdlib.h>` de `scroll`, `mouse_reader`, `ui`, `bg`, `help`, `score_panel` ; `probe` la garde peut-être (son prototype ne l'en a pas retirée). *Si elle tombe* : la liste des `<stdlib.h>` à retirer se refait par compilation, fichier par fichier.
- **H-4h-8 (TU de 4h).** Les TU ajoutés ou réécrits par 4h n'utilisent pas la transitoire : ils prennent les zones du jeu. Les 18 fichiers listés ici sont ceux de a61134c (même liste sur le prototype de 4h). *Si elle tombe* : chaque nouveau fichier passe à la zone de tas des TU.
- **H-4h-9 (commentaires « heap » de 4h).** 4h réécrit « fresh heap vector(s) » de `pico8_mouse_reader.h` et `pico8_scroll.h`, et « deliberately leaked » de `pico8_entity.c`. *Si elle tombe* : l'étape 5 les réécrit.

## À revérifier une fois 4h finie

À `HEAD` au moment d'implémenter, sur une copie `git archive`, sans toucher git ni les sources :

- la note « *Découvert (4h)* », la fin de l'étape 4 et la ligne « *Reçu de l'étape 4* » sous l'entrée 5 de `docs/plan-4.md` (H-4h-5, H-4h-6) ;
- `make alloc-guard` : « allouent 1 appels 3 », et `grep -rn zone_transitoire src` vide hors de la transitoire elle-même (H-4h-1) ;
- `git diff a61134c..HEAD --stat -- tools/alloc_guard.sh tools/alloc_guard_exceptions.txt Makefile tests` : ce que 4h a touché de ce que l'étape 5 change (H-4h-2, H-4h-8) ;
- `grep -rnw TROU-7 src tests Makefile` : la liste exacte des marqueurs restants (H-4h-3) ;
- le rejeu des 26 e2e sous LSan, et `test_pico8_planet_gh_row_lq` sous LSan (H-4h-4) ;
- `grep -n "#include <stdlib.h>"` sur `CORE_SRCS`, et le `grep` du vocabulaire libc (H-4h-7, H-4h-9) ;
- `grep -l pico8_zone_transitoire tests/*.c` (H-4h-8) ;
- **refaire le prototype** si l'un de ces points montre un écart : `make test-all`, `sanitize-net` sous LSan, `alloc-guard`, `make raylib`, et le récapitulatif des zones.

## Further Notes

- **Pas d'humain disponible, et pas d'outil `Agent` dans cette session** pour lancer un agent `guideline` neuf. J'ai donc appliqué moi-même le skill `guideline` (`.claude/skills/guideline/SKILL.md`) aux points que `to-prd` fait valider par un humain (modules, tests, découpage) :
  - **Hypothèses explicites** : celles sur 4h sont numérotées (H-4h-1 à H-4h-9), confrontées au prototype de 4h et listées à revérifier.
  - **Simplicité** : aucun mécanisme neuf dans le cœur. Dans les TU, un seul module de test remplace la transitoire, à l'identique, plutôt que 18 copies ou des zones du jeu à remettre à zéro.
  - **Changements chirurgicaux** : l'étape ne réécrit que ce qui cite TROU-7, la transitoire ou le vocabulaire de la libc. Les contrats « frees » douteux, hors de ce critère, sont signalés ci-dessous et non corrigés.
  - **Critères vérifiables** : la sortie d'`alloc-guard`, `sanitize-net` sous LSan, trois `grep` vides, les 26 empreintes.
- **Le prototype de l'étape 5** a été fait sur le prototype de 4h (copie `git archive` de 5c3ea7e avec les changements de 4h, hors dépôt) : zone de tas des TU, suppression de la transitoire, verrou, TROU-7, `README`, `<stdlib.h>`. Résultats :
  - `make test-all` vert, 26 `*.sha256` conformes ;
  - « verrou objets 64 allouent 0 appels 0 » ;
  - `make sanitize-net` sous LSan : « 26 propre(s) » ; à a61134c, le même rejeu de `pause_restart` sous LSan fuit 1 792 o en 80 blocs (sites de 4h) ;
  - `make raylib` compile et lie ;
  - diff : 50 fichiers, +168 / −282 lignes, dont `src/` +5 / −59 (sans la réécriture des contrats).

  Rien de ce prototype n'est dans le dépôt.
- **Contrats « frees » qui semblent faux, signalés et non corrigés** (hors du critère de l'étape) :
  - `pico8_cart.h` : « A second call … frees what the first built before building it again » ;
  - `pico8_data_init.h` : « it frees a previous build first » ;
  - `pico8_help.h` : « `pico8_help_shutdown()` frees it » (le prototype montre une fonction qui ne libère rien).

  Les zones remettent à zéro au lieu de libérer. Une reprise des commentaires (skill `reprendre-les-commentaires`) ou une revue de fin de phase peut les traiter.
- **Pour un humain** : le `CLAUDE.md` et le skill `reprendre-les-commentaires` prennent `/* TROU-7 */` comme exemple de marqueur nu. Une fois l'entrée supprimée, l'exemple désigne un numéro refermé. Le format reste juste ; changer l'exemple de numéro est une décision humaine, qu'un agent ne prend pas.
