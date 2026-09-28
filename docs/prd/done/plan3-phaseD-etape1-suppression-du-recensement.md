---
title: "Plan 3, étape D.1 : suppression du recensement — `pico8_member_census.c/.h`, les 69 crochets `SEE`/`READ`/`WRITE`/`WALK`/`CHILD_EXIT`/`STOP` et l'inclusion de `src/` et `tests/`, `tools/member_census.*`, la cible `member-census` et ses entrées de listes de sources, le mode `--c-table` de `tools/entity_value_members.sh` ; les 108 messages d'`assert` « census row » / « plan-3 section 5 » réécrits pour nommer le contrat violé — un seul commit, aucun comportement changé"
description: "La phase 1b est sortie : aucune struct d'entité ne garde de membre `pico8_value_t`, et le recensement qui mesurait ces membres n'a plus d'objet. Il reste pourtant dans l'arbre : un module de 664 lignes, 69 crochets qui ne font rien sans `-DPICO8_MEMBER_CENSUS`, deux outils, une cible `make` de 70 lignes, un mode de script, et 108 messages d'`assert` qui renvoient à une « census row » et à « plan-3 section 5 », deux choses qu'un lecteur du code ne peut plus consulter. D.1 retire tout cela, en un commit, sans changer le comportement du jeu ni des tests."
status: needs-triage
---

## Problem Statement

Puce de l'étape, recopiée de `docs/plan-3.md` (§5, « Phase D — Données ») :

> - **D.1 — suppression du recensement**, premier groupe après D.0 : `pico8_member_census.c/.h`, les crochets `SEE`/`READ`/`WRITE`/`CHILD_EXIT`/`STOP` de `src/` et `tests/`, `tools/member_census.*`, la cible `member-census` et ses entrées de listes de sources, le mode `--c-table` de `tools/entity_value_members.sh`. Les messages d'`assert` qui renvoient à « census row » / « plan-3 section 5 » sont réécrits pour nommer le contrat violé. Aucun `.sha256` ne change ; `make class-coverage-net` identique.

Note « Découvert à D.0 », recopiée de la fin de la puce D.0 :

> Découvert à D.0 : `make data-residue` (`tools/data_residue.sh`, modes `--detail` pour la liste de D et `--check`), neuf catégories figées, lignes qui se chevauchent, commentaires compris — `valeur`, `ob` (avec `pico8_fi`), `stash`, `type-tag` (tout `pico8_type_tag.h`, dont le `tag` de `pico8_struct_header_t`), `ref-audit`, `par-nom` (`*field_truthy` seulement), `dispatch` (`pico8_value_function(_t)`, `pico8_mv` ; pas `methods[]`), `overrides`, `spec` (hors six `_default` à argument omis). Comptes initiaux après la fusion de master (lignes/fichiers) : 6001/133, 2280/128, 56/14, 450/72, 70/17, 1070/81, 180/57, 421/57, 942/68, **9711 lignes distinctes dans 145 fichiers** (100 de `src/`, 45 de `tests/`). Sortie vide = la seule ligne `résidu D : 0 ligne(s), 0 fichier(s)`. `pico8_list_t` sans catégorie (§6 ne le retire pas), mais ses éléments `pico8_value_t` comptent dans `valeur` : la liste de D doit en changer la représentation. « Plus aucun `malloc` pour des données » lu comme les allocations de `pico8_ob.c`/`pico8_stash.c`/`pico8_value.c`, éteintes avec leurs catégories ; une lecture plus large ajoute une catégorie dans son propre commit. Garde : `make data-guard` dans `make test` (`entity_value_members.sh --check`, `grep -rn offsetof src/`, `DATA_RESIDUE_GUARD`, vide à D.0) ; une étape qui éteint une catégorie l'ajoute à `DATA_RESIDUE_GUARD`. Motifs figés : une étape ne change un motif qu'avec la preuve que les lignes retirées ne touchent plus aucune catégorie transversale.

Fin de la note de sortie de 1b (§5, « Phase 1b ») :

> […] Restent sans objet : le recensement (`pico8_member_census`, ses crochets, `tools/member_census.*`, `make member-census`), supprimé par D.1 ; `make value-members` reste la garde de cette sortie, dans `make test` à partir de D.0.

Règles du §7, recopiées :

> - **Filets depuis la fusion de master (e2e rapide)** : `FRAMES_EVENTS` n'existe plus ; `make e2e` rejoue les 26 e2e de `tests/e2e/` depuis l'écran titre, `make sanitize-net` et `make class-coverage-net` rejouent ce même jeu, `tools/class_coverage_net_expected.txt` y a été reposé à la fusion ; aucun TU ne rejoue de fichier d'événements : un TU construit son état directement, un rejeu est un e2e de `tests/e2e/` (moins de 180 frames, et il doit ajouter de la couverture de lignes). Les mentions de `FRAMES_EVENTS`, `frames-check` et `float-audit` dans les étapes livrées sont historiques.
> - **En D et en 2** (ajouté à la revue de sortie de 1b) : `make class-coverage-net` aux étapes dont le diff touche un constructeur, un appel `pico8_class_coverage_record` ou `KNOWN[]`, et à la sortie de phase ; `make sanitize-net` aux étapes qui changent la durée de vie ou le propriétaire d'une donnée (table ob → locale ou statique, `malloc`/`free` retiré), et à la sortie de phase.

Lecture de ces règles pour D.1 :

- Le diff **touche 29 constructeurs** : 29 des 40 crochets `SEE` sont des `SEE(self, …)` en fin de constructeur, peu après `pico8_class_coverage_record`. Les 11 autres portent sur une statique en fin d'init de module (9), ou sur un objet local (`view` d'`ie_ke`, `point` de `jc_mouse_point`). Le diff réécrit aussi les `assert` de clés de construction. **`make class-coverage-net` est donc exigé**, et il doit rendre une sortie identique.
- D.1 ne change la durée de vie ni le propriétaire d'**aucune** donnée. Sans le drapeau, le module de recensement est une unité de compilation vide, et ses crochets ne font rien. La règle **n'exige donc pas `make sanitize-net`**. Le pilote le demande quand même : il coûte peu et il retire un objet du lien ASan. Il est donc dans les critères.

### État de départ

HEAD `6458511`, arbre propre (`git status --short` vide à l'écriture). Depuis D.0 (`3cac4b5`), on trouve :

- la fusion de master (`ab57bf5`) : `make e2e` = 26 e2e de `tests/e2e/` depuis l'écran titre, en ~1 s, et `FRAMES_EVENTS` n'existe plus ;
- la réécriture de 27 TU de struct (`7f552f4..baca9c2`). Aucun TU ne rejoue plus de fichier d'événements. `pico8_bo()`, `UNIT_TESTS_SLOW` et `make unit-slow` sont supprimés ;
- `6458511` : le §7 aligné.

Ces décisions de l'utilisateur engagent D.1 : **un TU ne rejoue jamais un `events.json`** (il construit son état), **rien de lent**, **pas de difficulté forcée**.

**Rien n'a été compilé ni lancé pour ce PRD.** Les comptes viennent de `grep` sur `6458511`. Le compte de `make data-residue` a été refait à la main, avec les neuf motifs de `tools/data_residue.sh` passés à `grep -nE` fichier par fichier, et avec l'exclusion `sed` de `spec`. Tout fait d'exécution est une hypothèse (H1 à H14).

### Inventaire (mesuré sur `6458511`)

**Le module** : `src/pico8_member_census.c` (620 lignes, tout le corps sous `#ifdef PICO8_MEMBER_CENSUS`) et `src/pico8_member_census.h` (44 lignes). L'en-tête n'inclut rien. Sans le drapeau, ses macros valent :

| Crochet | Sans `-DPICO8_MEMBER_CENSUS` |
|---|---|
| `PICO8_MEMBER_CENSUS_SEE(obj, td)` | `((void)0)` |
| `PICO8_MEMBER_CENSUS_READ(e, name, found, out)` | `(found)` |
| `PICO8_MEMBER_CENSUS_WRITE(e, name, value)` | `((void)0)` |
| `PICO8_MEMBER_CENSUS_WALK(hv)` | `((void)0)` (l'argument `pico8_hv()` n'est pas évalué) |
| `PICO8_MEMBER_CENSUS_CHILD_EXIT(code)` | `_exit(code)` |
| `PICO8_MEMBER_CENSUS_STOP()` | rien (pas même un `;`) |

**Crochets hors du module : 69 lignes**, recensées par `grep -rnE 'PICO8_MEMBER_CENSUS_|pico8_member_census\.h' src tests --include='*.[ch]'`, moins le module lui-même.

| Crochet | Lignes | Fichiers | Où |
|---|---|---|---|
| `SEE` | 40 | 21 | `src/` : `bg`, `economy`, `entity`, `help`, `layout` (2), `menu`, `mouse_reader`, `msgbox` (2), `planet`, `planet_family` (6), `planet_family_trade` (4), `probe` (3), `research`, `research_icon` (2), `score_panel` (2), `scroll`, `slipway` (3), `trade`, `ui` (2), `ui_button` (2), `wormhole` (2) |
| `READ` | 1 | 1 | `src/pico8_entity.c:187` |
| `WRITE` | 1 | 1 | `src/pico8_entity.c:199` |
| `WALK` | 2 | 1 | `src/pico8_cart.c:254`, `:264` (`_update60`, `_draw`) |
| `#include "pico8_member_census.h"` | 1 | 1 | `src/pico8_struct.h:7` |
| `CHILD_EXIT(0)` | 18 | 17 | `tests/test_pico8_` `bg`, `fm`, `fr`, `fv`, `g_`, `ga`, `hp`, `hy`, `jc`, `js` (2), `lq`, `mz`, `nl`, `pj`, `qb`, `trade`, `ui` |
| `STOP()` | 6 | 6 | `tests/test_pico8_` `fr:602`, `fv:572`, `g_:798`, `ga:702`, `jc:663`, `nl:513` |
| **Total** | **69** | **40** (23 de `src/`, 17 de `tests/`) | |

- `WALK` n'est pas nommé par la puce. C'est pourtant un crochet du même en-tête, couvert par « les crochets ». Voir la décision plus bas.
- Les 17 fichiers de `tests/` qui ont `CHILD_EXIT` incluent tous `<unistd.h>` et appellent déjà `_exit(0)` en direct ailleurs (vérifié par grep). Aucun test ne prévoit le cas `#ifdef PICO8_MEMBER_CENSUS`.
- `STOP()` est seul sur sa ligne dans cinq cas. À `test_pico8_jc.c:663`, il précède `for (i = 0; i < 3; i++) {` sur la même ligne. À `test_pico8_fr.c:602`, il est suivi d'un bloc `{ … }` nu, qui reste.

**Messages d'`assert` : 108 messages dans 21 fichiers de `src/`.** On compte une occurrence de `plan-3` par message. `grep -rnE 'plan-3' src tests` rend 108 lignes, toutes dans `src/`. Ils se rangent en trois familles :

| Famille | Messages | Forme actuelle (abrégée) |
|---|---|---|
| A. genre d'un membre typé | 76 | `"<cls.m>: <ce qui arrive> is off its census row (<genre>) -- a field under no rule of plan-3 section 5: stop, do not widen the member"` |
| B. clé de construction ou champ par nom | 30 | `"<cls>: <clés connues> ; a key would be a field under no rule of plan-3 section 5 -- stop, do not add a member"`, dont `entity.c:201` (écrivain générique) |
| C. clé de bouton de `je` | 2 | `"je[bt]: plan-3 section 5 enumerates bt as l or r only; another key is an unmeasured field -- stop"` (`ui.c:103`, `:488`) |

Répartition par fichier : `ui_button` 14, `planet_family` 12, `planet` 11, `layout` 7, `msgbox` 7, `planet_family_trade` 7, `probe` 7, `research_icon` 7, `slipway` 7, `ui` 5, `research` 4, `wormhole` 4, `bg` 3, `scroll` 3, `trade` 3, `help` 2, `economy` 1, `entity` 1, `ke` 1, `mouse_reader` 1, `score_panel` 1. Sur ces 21 fichiers, un seul n'a aucun crochet : `ke.c`. Deux fichiers ont des crochets sans message : `cart.c` et `menu.c`.

- **Aucun test ne lit le texte d'un message.** Les `must_abort` redirigent `stderr` vers `/dev/null` et ne jugent que `SIGABRT` (voir `tests/test_pico8_fv.c:470-484`).

**Mentions de « census » dans `tests/` hors crochets : 11 lignes dans 6 fichiers.** Ce sont des commentaires (« census row ») et une fonction :

- `test_pico8_js.c:1638` ;
- `test_pico8_lq.c:757` ;
- `test_pico8_mh.c:511` ;
- `test_pico8_ui.c:2000`, `:2298`, `:2399` ;
- `test_pico8_g_.c:899`, `:1469` ;
- `test_pico8_hp.c:743`, `:820` (la fonction `static int census_kind(…)` et son appel), et `:767` (commentaire).

**Outils** :

- `tools/member_census.sh` (435 lignes) et `tools/member_census.txt` (275 lignes). Aucun autre script, TU ou cible ne les lit : `grep -rni census tools .claude docs` ne rend que ces deux fichiers, `entity_value_members.sh`, `docs/plan-3.md` et des PRD livrés.
- `tools/entity_value_members.sh` (210 lignes) a trois modes : liste (défaut), `--check` (D.0, utilisé par `data-guard`) et `--c-table`, dont `make member-census` est le **seul appelant**. Ce qui tient au mode `--c-table` :
  - les trois lignes d'usage en tête ;
  - la branche `if [ "$1" = "--c-table" ]` ;
  - tout le bloc `END` après `exit (mode == "check" && total > 0)`, qui émet la table C (avec `offsetof(…)` dans une chaîne `awk`) ;
  - les écritures de `parent[t]` dans la boucle de fermeture, qui ne servent qu'à ce bloc.

**`Makefile`** :

| Site | Lignes (`6458511`) |
|---|---|
| Légende `make member-census` | 70-75 |
| `src/pico8_member_census.c` dans `ENGINE_SRCS` | 206 |
| `src/pico8_member_census.c` dans `DATA_ENGINE_SRCS` | 272 |
| Bloc du recensement : commentaire, `CENSUS_UNIT_TESTS` (41 TU), `CENSUS_TMP`/`_EVENTS`/`_UNIT`/`_TABLE`/`_CFLAGS`, `member-census`, `CENSUS_REPLAYS`, `census-runs`, règle `census-replay-%`, `.PHONY` | 922-992 |

- `CORE_SRCS` (float-guard, float-grep) dérive d'`ENGINE_SRCS` : le module en sort de lui-même.
- La liste de sources en dur de `$(TMP_DIR)/main_raylib` (`Makefile:1070-1071`) **ne contient pas** `pico8_member_census.c` : elle ne change pas.
- Aucune autre cible ne nomme `MEMBER_CENSUS`, `CENSUS_` ni `member_census`.

**Docs** : hors `docs/prd/`, seul `docs/plan-3.md` nomme le recensement. Ses mentions sont historiques (1b.0, 1b.4b, liste de 1b, note de sortie, puce D.1) et restent. `docs/glossaire-noms.md` et `docs/trous-de-portage.md` n'en disent rien.

**Résidu de données sur `6458511`** (recompte à la main, H5). Il diffère de la note D.0 (9711) parce que les 27 TU ont été réécrits depuis :

```
valeur	6006	133
ob	2245	128
stash	56	14
type-tag	451	72
ref-audit	70	17
par-nom	1083	81
dispatch	197	59
overrides	449	62
spec	939	68
résidu D : 9702 ligne(s), 145 fichier(s)
```

Contribution du module à ce compte : **24 lignes distinctes dans 2 fichiers**.

- `pico8_member_census.c` : `valeur` 17 lignes et `type-tag` 6 lignes (l'inclusion de `pico8_type_tag.h`, un `pico8_type_tag_of`, des `PICO8_TYPE_TAG_*`), soit 23 lignes distinctes.
- `pico8_member_census.h` : `par-nom`, 2 lignes (son commentaire nomme `pico8_entity_get_field` et `pico8_entity_set_field`).

Parmi les lignes retouchées hors du module, trois seulement tombent dans une catégorie. Aucune ne sort du compte :

- `entity.c:187` (`par-nom`, `->field(`) : après le déballage du crochet, la ligne garde `header->cls->field(`.
- `ui_button.c:271` (`valeur`) : `PICO8_VALUE_FIXED` est dans la condition, que la réécriture du message ne touche pas.
- `test_pico8_hp.c:743` (`valeur`, `pico8_value_kind_t`) : le renommage garde le type.

## Solution

**Un seul commit**, qui ne change aucun comportement :

1. **Le module disparaît** : `src/pico8_member_census.c`, `src/pico8_member_census.h`, et leur inclusion dans `pico8_struct.h`.
2. **Chaque crochet est remplacé par ce qu'il vaut sans le drapeau** :
   - `SEE`, `WRITE`, `WALK` et `STOP` sont retirés ;
   - `READ` est déballé en son expression ;
   - `CHILD_EXIT(0)` devient `_exit(0)`.
3. **L'outillage disparaît** :
   - `tools/member_census.sh`, `tools/member_census.txt` ;
   - la cible `member-census`, avec ses variables, ses règles et sa légende ;
   - l'entrée du module dans `ENGINE_SRCS` et `DATA_ENGINE_SRCS` ;
   - le mode `--c-table` de `tools/entity_value_members.sh`. Les modes liste et `--check` rendent exactement la même sortie qu'avant.
4. **Les 108 messages d'`assert` nomment le contrat violé**, sans renvoyer au recensement ni au plan. Ils gardent le piège : « ne pas élargir le membre », « ne pas ajouter de membre ». Les 11 mentions de « census » dans les commentaires et noms de `tests/` sont reformulées de même.

**Sortie de l'étape**, constatée par commande :

- `grep -rniE 'census' src tests tools Makefile` est vide ;
- `grep -rnE 'plan-3|section 5' src tests` est vide ;
- `make test` est vert ;
- `make e2e` rend 26 conformes ;
- `make class-coverage-net` est identique à la référence ;
- `make sanitize-net` rend 26 propres ;
- `make value-members` rend `membres pico8_value_t : 0` ;
- `make data-residue` rend 9678 lignes dans 143 fichiers.

## User Stories

1. En tant que lecteur de `src/`, je veux qu'aucune ligne ne porte un crochet qui ne fait rien dans les builds de la suite, afin de lire les constructeurs sans bruit d'instrumentation morte.
2. En tant que lecteur d'un constructeur, je veux que sa dernière instruction avant `return self` soit du jeu ou de la couverture de classes, et non un crochet de mesure d'une phase close, afin de voir ce que le constructeur fait.
3. En tant que lecteur de `pico8_entity_get_field`, je veux lire `return header->cls->field(e, name, out_value);` tel quel, afin de ne pas déballer une macro pour savoir ce que la fonction rend.
4. En tant que lecteur de `pico8__update60` et `pico8__draw`, je veux ne plus voir d'appel `pico8_hv()` passé à une macro qui ne l'évalue pas, afin de ne pas croire que la liste des entités est parcourue à chaque frame.
5. En tant qu'auteur d'un TU qui fork, je veux finir le fils par `_exit(0)` comme les autres fils du même fichier, afin qu'il n'y ait qu'une façon d'écrire la sortie d'un fils.
6. En tant que lecteur d'un TU, je veux que les marques `STOP()` disparaissent, afin que chaque ligne soit une instruction ou un commentaire utile.
7. En tant que développeur qui voit un `assert` échouer, je veux un message qui dit quel membre a reçu quoi et quel type il tient, afin de comprendre la faute sans ouvrir un fichier supprimé ni une section de plan.
8. En tant que développeur tenté d'élargir un membre typé pour faire passer un `assert`, je veux que le message me dise encore de ne pas l'élargir, afin que le piège qu'il garde survive à la suppression du recensement (règle CLAUDE.md : un `assert` porteur de message).
9. En tant que développeur tenté d'ajouter un membre pour une clé de construction inconnue, je veux que le message nomme les clés connues et dise de ne pas ajouter de membre, afin de reconnaître un champ sans règle.
10. En tant que mainteneur du `Makefile`, je veux que la cible `member-census`, ses sept variables, ses règles et sa légende disparaissent, afin que la légende ne liste que des cibles qui marchent.
11. En tant que mainteneur, je veux que le module sorte d'`ENGINE_SRCS` et de `DATA_ENGINE_SRCS`, afin qu'aucun binaire ne compile ni ne lie une unité vide.
12. En tant que gardien de la sortie de 1b, je veux que `make value-members` et `make data-guard` rendent exactement la même sortie qu'avant, afin que la garde de D.0 continue de mordre.
13. En tant que mainteneur de `entity_value_members.sh`, je veux que le script n'ait plus que les modes liste et `--check`, sans le code mort du mode `--c-table`, afin qu'il ne génère plus de C pour un module absent.
14. En tant que `plan-pilot`, je veux constater par `grep` qu'aucune mention du recensement ne reste dans `src/`, `tests/`, `tools/` et le `Makefile`, afin de clore D.1 par une commande et non par une relecture.
15. En tant que gardien de l'invariant §3, je veux que les 26 e2e restent conformes, sans aucun `*.sha256` ni fichier de `tests/e2e/` touché, afin de prouver qu'aucun pixel n'a bougé.
16. En tant que gardien du §7, je veux `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt`, afin de prouver qu'aucun constructeur n'a perdu ni gagné une instanciation.
17. En tant que gardien du §7, je veux `make sanitize-net` propre sur les 26 e2e, afin de prouver que le lien ASan sans le module ne révèle rien.
18. En tant qu'auteur de la liste de D, je veux le pied de `make data-residue` avant et après dans le message de commit, afin de partir du bon compte.
19. En tant que `make test`, je veux ne rien gagner en durée, afin que la boucle rapide reste rapide.
20. En tant que lecteur de l'historique, je veux que le message de commit dise ce que le recensement mesurait et pourquoi il part, afin que cette connaissance ne se perde pas avec les fichiers (règle CLAUDE.md : le reste va dans le message de commit).

## Implementation Decisions

Pas d'humain dans la boucle. Le skill `to-prd` demande de faire valider par l'utilisateur les modules, les tests et le découpage. Ces choix sont tranchés ici comme le ferait un agent `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables), et marqués **(G)**. Les rejets sont notés à chaque décision.

### Remplacement des crochets : ce que la macro vaut sans le drapeau (G)

La règle est unique : **chaque crochet devient exactement son expansion sans `-DPICO8_MEMBER_CENSUS`**. Le préprocesseur produit donc le même code avant et après, aux numéros de ligne près.

| Crochet | Remplacement |
|---|---|
| `PICO8_MEMBER_CENSUS_SEE(…);` (40) | ligne supprimée |
| `return PICO8_MEMBER_CENSUS_READ(e, name, header->cls->field(e, name, out_value), out_value);` | `return header->cls->field(e, name, out_value);` |
| `PICO8_MEMBER_CENSUS_WRITE(e, name, value);` | ligne supprimée ; `written` et `value` gardent leurs usages |
| `PICO8_MEMBER_CENSUS_WALK(pico8_hv());` (2) | ligne supprimée. `pico8_hv()` n'était pas évalué : le supprimer ne retire aucun appel |
| `PICO8_MEMBER_CENSUS_CHILD_EXIT(0);` (18) | `_exit(0);` |
| `PICO8_MEMBER_CENSUS_STOP()` (6) | le jeton est retiré. Seul sur sa ligne (5 cas), la ligne part. À `test_pico8_jc.c:663`, la ligne devient `    for (i = 0; i < 3; i++) {`. Le bloc nu qui suit à `test_pico8_fr.c:602` reste tel quel |
| `#include "pico8_member_census.h"` (`pico8_struct.h:7`) | ligne supprimée |

- **`WALK` est supprimé avec les autres (G).** La puce énumère `SEE`/`READ`/`WRITE`/`CHILD_EXIT`/`STOP`, mais elle vise « les crochets » de l'en-tête supprimé. `WALK` en est un, et il ne compilerait plus sans son en-tête. Ce n'est pas une extension du périmètre : c'est un oubli de l'énumération, noté dans le message de commit.
- **Pas de reformatage au-delà de la ligne touchée.** L'indentation de `for` à `jc.c:663` est celle des lignes voisines, et aucune autre ligne ne bouge.
- Rejeté : **garder l'en-tête avec des macros vides** « au cas où ». La puce le supprime. Un crochet sans outil pour le lire est du code mort (`guideline` : ne rien garder de spéculatif).
- Rejeté : **remplacer `CHILD_EXIT` par une fonction d'aide commune aux TU.** `_exit(0)` est déjà la forme des autres fils des mêmes fichiers.

### Suppressions de fichiers et `Makefile`

- Supprimés : `src/pico8_member_census.c`, `src/pico8_member_census.h`, `tools/member_census.sh`, `tools/member_census.txt`.
- **`Makefile`** :
  - retirer les deux lignes `src/pico8_member_census.c \` d'`ENGINE_SRCS` et de `DATA_ENGINE_SRCS` ;
  - retirer la légende `make member-census` (6 lignes) ;
  - retirer le bloc entier du commentaire « Les TU de UNIT_TESTS qui lient… » jusqu'au `.PHONY: member-census census-runs $(CENSUS_REPLAYS)` inclus, avec la ligne vide qui le suit, pour qu'une seule ligne vide sépare `data-guard` du bloc `sanitize`.
  - Rien d'autre ne change. La légende de `value-members` ne nomme pas le recensement.
- `tmp/census/` (non versionné, sous `tmp/`, ignoré) : l'implémenteur peut le supprimer, **sans que ce soit un critère**.

### `tools/entity_value_members.sh` sans `--c-table` (G)

- On retire :
  - les trois lignes d'usage de `--c-table` en tête ;
  - la branche `if [ "$1" = "--c-table" ]` (le `if` devient `if [ "$1" = "--check" ]`) ;
  - tout le code `END` qui suit l'`exit` des modes liste et `--check` ;
  - les deux affectations `parent[t] = …` de la boucle de fermeture, qui n'ont plus de lecteur. `is_entity[t] = 1` et `changed = 1` restent.
- La condition `if (mode == "list" || mode == "check")` peut devenir inconditionnelle, puisqu'aucun autre mode n'existe. L'implémenteur garde la forme la plus courte qui laisse la sortie **identique octet pour octet**.
- On garde :
  - le mode liste et le mode `--check`, avec leurs sorties et leurs codes ;
  - la ligne d'en-tête « Phase 1b of docs/plan-3.md ends when the list is empty » : c'est le contrat du compteur, pas le recensement.
- **Une option inconnue** (dont `--c-table`) continue de tomber dans le mode liste, comme aujourd'hui pour toute option autre que `--c-table` et `--check`.
  - Rejeté : un code 2 sur option inconnue. C'est un changement de contrat qu'aucune puce ne demande, et `data-guard` n'en dépend pas.
- **`make value-members` et `make data-guard` marchent après (H3)**. Le script lit `ls src/*.c src/*.h`. Le module retiré ne contient aucun typedef d'entité : ses deux typedefs ont pour premier membre `const char *name` et `int td`. La liste d'entités, donc la sortie, ne change pas : `membres pico8_value_t : 0`.

### Réécriture des 108 messages d'`assert` (G)

**Principe** (règle CLAUDE.md « un piège qu'on réintroduirait : un `assert` porteur de message ») :

- le message garde **quel site** (`cls.membre` ou `cls`), **ce qui est arrivé**, et **le piège** (ne pas élargir, ne pas ajouter) ;
- il perd le renvoi au recensement (`off its census row (<genre>)`) et au plan (`a field under no rule of plan-3 section 5`) ;
- **la condition de l'`assert` n'est jamais touchée**, seule la chaîne change ;
- les coupures de chaîne suivent la largeur des lignes voisines (≤ 120 colonnes).

**Famille A, 76 messages : genre d'un membre typé.** Le texte avant `is off its census row` est gardé tel quel. La suite `is off its census row (<genre>) -- a field under no rule of plan-3 section 5: stop, do not widen the member` devient `does not fit the member's type -- stop, do not widen the member`. Si le message porte une clause de contrat en plus, elle est gardée à sa place, par exemple `and rh has no presence flag` (`mh.rh`) ou `and at2 has no presence flag` (`iv.at2`). Exemples :

- `"jo.kw/nk: a write neither nil nor a table does not fit the member's type -- stop, do not widen the member"` ;
- `"mh.rh: a nil or non-fixed key does not fit the member's type and rh has no presence flag -- stop, do not widen the member"`.

Le jeton de genre entre parenthèses (`fixe`, `ref:liste`, `chaine`…) vient du vocabulaire du recensement. Il redit la condition : le message dit déjà « neither nil nor a table ». Il part.

- Rejeté : garder `(ref:table)`. C'est un nom de genre de `tools/member_census.txt`, qui disparaît.

**Famille B, 30 messages : clé de construction ou champ sans membre.**

- La phrase qui nomme les clés ou la route connues est gardée. On retire seulement `measured` (« the only measured construction keys are » → « the only construction keys are » ; « the only measured road is » → « the only road is » ; « every measured construction passes only » → « every construction passes only »). « no construction table was measured » devient « takes no construction table ».
- La suite `a key would be a field under no rule of plan-3 section 5 -- stop, do not add a member` (et ses variantes « another would be… ») devient `another key has no member -- stop, do not add one`.
- Exemples :
  - `"nf: takes no construction table; another key has no member -- stop, do not add one"` ;
  - `"entity: a generic write names a field this struct has no member for -- stop, do not add a member"`.
- Les variables locales `int measured` (`planet.c:659`, `planet_family.c:280`, `planet_family_trade.c:431`, `:647`) et le message `ui_button.c:43` (« unmeasured REF ») **ne sont pas touchés**. Hors de la puce, et le nom reste vrai : ce sont les clés observées.

**Famille C, 2 messages (`ui.c:103`, `:488`)** : `"je[bt]: plan-3 section 5 enumerates bt as l or r only; another key is an unmeasured field -- stop"` devient `"je[bt]: bt is l or r only; another button has no state table -- stop"`. `ui:bh` suit la même forme.

**Forme libre à l'intérieur de ces règles.** Si une coupure de chaîne rend la forme exacte maladroite, l'implémenteur peut s'en écarter. Il garde alors les trois éléments (site, ce qui arrive, piège) et le critère `grep` (aucun `census`, `plan-3`, `section 5`). Les exemples ci-dessus sont des patrons, pas un texte figé.

**Aucun jeton de catégorie du résidu** (`pico8_value_*`, `PICO8_VALUE_*`, `overrides`, `->field`, `.field`, `*_take_*(`, `g_*_spec`…) n'est **ajouté** dans une chaîne réécrite. Sinon, le compte de `make data-residue` bougerait (H5).

- Rejeté : **supprimer les messages** et garder des `assert(cond)` nus. La règle CLAUDE.md garde un `assert` porteur de message quand il porte un piège, et c'est le cas des 108.
- Rejeté : **un renvoi à une section de `docs/plan-3.md`** mise à jour. C'est ce que la puce retire : le code ne renvoie pas à un plan qui change.

### Commentaires et nom dans `tests/` (G)

- `census_kind` (`test_pico8_hp.c:743`, `:820`) devient `member_kind_ok`. Le type `pico8_value_kind_t` reste, la ligne reste dans `valeur`.
- Les 9 commentaires :
  - « census row » / « its census row » devient « the member's type » ou « the kinds the member holds », selon la phrase ;
  - « the census row, plus `fv`, `nl`, `pm` » (`g_:899`) devient « the classes of the typed member, plus `fv`, `nl`, `pm` ».
  - Le sens ne change pas. Ce sont des contrats de test (ce que le test attend du membre), gardés au titre de la règle CLAUDE.md.
- Les noms de fonctions de test `…_stops_off_row`, `…_rejects_off_row_keys` **ne changent pas** : ils ne disent pas « census », et les renommer serait du bruit hors puce.

### Ce qui ne change pas

- Aucun comportement : aucun octet de framebuffer, aucune instanciation, aucune allocation.
- Aucun `*.sha256`, aucun fichier de `tests/e2e/`, aucun fichier d'événements.
- Aucun appel ni aucune condition d'un TU.
- `tools/class_coverage_*`, `KNOWN[]`, `pico8_class_coverage_record`, `DATA_RESIDUE_GUARD`, `tools/data_residue.sh`, `tests/test_data_residue.c`.
- La cible `$(TMP_DIR)/main_raylib`.
- Les docs, sauf l'éventuelle note (voir Further Notes). Les mentions historiques du recensement dans `docs/plan-3.md` et `docs/prd/` restent.
- **TROU** : aucun écart port/cart n'est touché.
- **Fichiers touchés, et seulement eux** : 26 fichiers de `src/` (2 supprimés), 18 de `tests/`, 3 de `tools/` (2 supprimés), le `Makefile`. Soit **48 chemins**.

### Effet sur `make data-residue`

Seul le module supprimé contribue au compte. Toutes les autres lignes retouchées gardent leurs jetons (voir l'inventaire). Sortie attendue après D.1 (H5) :

```
valeur	5989	132
ob	2245	128
stash	56	14
type-tag	445	71
ref-audit	70	17
par-nom	1081	80
dispatch	197	59
overrides	449	62
spec	939	68
résidu D : 9678 ligne(s), 143 fichier(s)
```

- Écart avec le départ : −24 lignes distinctes, −2 fichiers, tous deux dans `src/` (100 → 98, `tests/` reste à 45).
- Aucune catégorie ne tombe à 0. **`DATA_RESIDUE_GUARD` reste vide.**
- Aucun motif du compteur ne change.

### Découpage (G) : un seul commit

Il y a 48 chemins, soit plus que le seuil de ~40 fichiers de la liste de D. Mais ce seuil vise les **conversions**, où chaque fichier demande un jugement. Ici, une vingtaine des 44 fichiers de `src/` et `tests/` ne changent que de une à quatre lignes mécaniques. Tous les autres changements sont des suppressions de lignes sans effet. Seuls les messages demandent de la rédaction, et ils suivent trois patrons. Découpes évaluées :

- (a) **D.1a** retire le module, les crochets et les outils ; **D.1b** réécrit les messages. Écartée :
  - entre les deux commits, 108 messages renverraient à une « census row » d'un fichier déjà supprimé ;
  - les deux commits touchent 20 des mêmes fichiers de `src/`, dont les mêmes constructeurs, et chacun devrait passer `class-coverage-net` (règle du §7) ;
  - la puce met les deux dans la même étape.
- (b) **`src/`** d'abord, **`tests/`** ensuite. Écartée : `tests/` ne compile plus dès que l'en-tête est supprimé (`CHILD_EXIT`, `STOP`). Ce n'est pas une découpe qui reste verte.
- (c) **Les outils et le `Makefile` à part**. Écartée : `make member-census` casserait à l'instant où le module part. Les outils vont avec.

Donc **un commit**. Il n'y a **pas de section « Implementation Chunks »**, que `prd-autopilot` traiterait comme autant de commits.

### Hypothèses (rien n'a été lancé)

- **H1** : `6458511` est vert : `make test`, `make e2e` (26 conformes), `make class-coverage-net` identique, `make sanitize-net` (26 propres), `make value-members` à 0. Les messages de `ab57bf5` et de la série `7f552f4..baca9c2` l'affirment. Démentie au départ : l'implémenteur s'arrête et note, D.1 ne répare rien.
- **H2** : sans `-DPICO8_MEMBER_CENSUS`, supprimer un crochet selon le tableau ne change pas le code produit, aux constantes `__LINE__` près. Seuls les `assert` et `PICO8_REF_AS_LIST` (`pico8_ref_audit.h`, sous `-DPICO8_REF_AUDIT` seulement) les lisent. Aucune valeur versionnée ne dépend d'un numéro de ligne de `src/` : `make ref-audit` est un diagnostic hors suite, sans référence versionnée.
- **H3** : `make value-members` rend exactement `membres pico8_value_t : 0` avant et après. `sh tools/entity_value_members.sh --check` sort 0. Donc `make data-guard` rend `membres pico8_value_t : 0`, `offsetof dans src/ : 0`, `catégories gardées : aucune`, code 0.
- **H4** : `-Werror` reste muet :
  - aucune variable ne devient inutilisée : `written` et `value` gardent leurs usages dans `pico8_entity_set_field`, et le `view` de `research_icon.c:137` est utilisé avant le `SEE` ;
  - l'inclusion de `pico8_entity.h` dans `cart.c` n'était pas seulement pour `WALK` (et une inclusion inutile ne donne pas d'avertissement) ;
  - aucun fichier n'obtenait une déclaration par `pico8_member_census.h`, qui n'inclut rien.
- **H5** : `make data-residue` rend exactement la sortie attendue ci-dessus. Le compte de départ, refait à la main, est 9702/145. Un écart vient d'un arbre changé, ou d'un message réécrit qui ajoute ou retire un jeton de catégorie. L'implémenteur l'explique dans le commit.
- **H6** : `make e2e` rend 26 conformes, sans nouvelle référence (H2 : aucun pixel ne bouge).
- **H7** : `make class-coverage-net` est identique à `tools/class_coverage_net_expected.txt`. `SEE` est posé après `pico8_class_coverage_record` et ne vaut rien sans le drapeau.
- **H8** : `make sanitize-net` rend 26 propres, et l'objet ASan du module disparaît du lien.
- **H9** : `make test` reste vert. Les 17 TU à `CHILD_EXIT` compilent avec `_exit(0)` : ils incluent `<unistd.h>` et l'utilisent déjà. Les 5 TU à `STOP()` seul et `jc` compilent sans le jeton.
- **H10** : aucun test ne juge le texte d'un message d'`assert` (les `must_abort` jettent `stderr`). La réécriture ne rougit aucun test.
- **H11** : aucune autre cible, script, skill ni doc active n'appelle `make member-census`, `tools/member_census.sh` ou `--c-table`. Vérifié par grep : seuls `docs/plan-3.md` (historique) et les PRD livrés les nomment.
- **H12** : `make raylib` (non exigé) n'est pas affecté : sa liste de sources ne nomme pas le module, et il ne compile plus l'en-tête à travers `pico8_struct.h`.
- **H13** : les durées sont courtes. `make test` prend quelques secondes (CLAUDE.md) et `make e2e` ~1 s. `make class-coverage-net` et `make sanitize-net` prennent moins de 2 min chacun, compilation comprise, sur 26 rejeux courts. Démentie : l'implémenteur note la durée, sans relancer.
- **H14** : l'arbre de départ est `6458511`, ou un commit de docs au-dessus (la graduation de D.0, la liste de D), sans changement non commité. La ligne « Runs » n° 1 le constate.

## Testing Decisions

Un bon test juge ce que le jeu fait (pixels, instanciations, sorties d'outil), pas la présence d'une ligne. D.1 est une suppression de code sans effet : **la preuve est que tout ce qui mesure le comportement rend la même chose**, et que les `grep` de périmètre sont vides.

### Pas de nouveau TU ni de nouvel e2e (G)

- Aucun crochet n'a de comportement sans le drapeau (tableau de l'inventaire). Il n'y a rien à rendre rouge puis vert. Un TU qui vérifierait « le module n'existe plus » serait un `grep` déguisé.
- Rejeté : **un e2e**. Le §7 n'en admet un que s'il ajoute de la couverture de lignes et dure moins de 180 frames. D.1 n'ajoute aucune ligne à couvrir.
- Rejeté : **comparer les objets `-O0` avant et après** (`cmp` des `.o`). Les constantes `__LINE__` des `assert` bougent dans chaque fichier dont on retire une ligne, et la comparaison serait rouge sans raison. L'égalité de comportement est prouvée par les filets.
- Rejeté : **`gcov` et mutations**. La « Preuve des tests » du §7 porte sur les lignes ajoutées ou modifiées par une conversion. D.1 ne modifie que des chaînes de messages, et supprime des lignes sans effet.

### Ce qui prouve l'étape

1. **Filet de comportement** :
   - `make test` vert ;
   - `make e2e` avec **26 « conforme »** ;
   - `make class-coverage-net` identique ;
   - `make sanitize-net` avec 26 propres.
2. **Gardes** :
   - `make value-members` rend la seule ligne `membres pico8_value_t : 0` ;
   - `make data-guard` code 0 ;
   - `make data-residue` = sortie attendue (H5).
3. **Périmètre, par `grep`** (tous vides) :
   - `grep -rniE 'census' src tests tools Makefile` ;
   - `grep -rnE 'plan-3|section 5' src tests` ;
   - `grep -n -- '--c-table\|ctable\|offsetof' tools/entity_value_members.sh` ;
   - `ls src/pico8_member_census.* tools/member_census.*` (aucun fichier) ;
   - `git grep -n 'MEMBER_CENSUS' -- . ':!docs'`.
4. **Messages** : relecture d'un échantillon d'au moins un message par famille et par fichier. Chacun a son site, ce qui arrive et son piège, et garde sa condition intacte (`git diff -U0` ne montre que des lignes de chaînes).

### Vérification, budget et tableau « Runs »

**Budget : environ 10 minutes de temps total de tests.** Toutes les cibles sont rapides depuis la fusion de master.

| Cible | Durée attendue |
|---|---|
| `make test` | quelques secondes (compilation incrémentale en plus : 44 fichiers touchés, dont `pico8_struct.h`, donc presque tout recompile) |
| `make e2e` | ~1 s après `make test` |
| `make class-coverage-net`, `make sanitize-net` | < 2 min chacun, compilation comprise (H13) |
| `make value-members`, `make data-guard`, `make data-residue` | < 1 s |

Ordre conseillé :

1. `make value-members` et `make data-residue` sur l'arbre de départ (H5, H14) ;
2. les suppressions et remplacements de crochets, puis `make test` ;
3. la réécriture des messages, puis `make test` ;
4. `make e2e`, `make class-coverage-net`, `make sanitize-net` une fois en fin ;
5. les gardes et les `grep`.

Au-delà du budget, l'implémenteur s'arrête et note ce qui manque, plutôt que de relancer. **Jamais l'appli raylib.** `make raylib` n'est pas exigé.

**L'implémenteur tient un tableau « Runs »** dans son journal de progression, une ligne par lancement :

| # | Commande exacte | État de l'arbre (HEAD + « crochets retirés » / « messages réécrits »…) | Code de sortie | Ligne clé | Durée |
|---|---|---|---|---|---|

Exemples de ligne clé :

- `résidu D : 9702 ligne(s), 145 fichier(s)` puis `résidu D : 9678 ligne(s), 143 fichier(s)` ;
- `membres pico8_value_t : 0` ;
- `26 conforme(s)` ;
- `sanitize: 26 propre(s)` ;
- la dernière ligne de `class-coverage-net`.

Le vérificateur réutilise ce tableau et ne relance que ce qui suit le dernier changement de l'arbre.

**Cibles finales**, toutes vertes sur l'arbre final :

1. `timeout 5m make test` ;
2. `timeout 5m make e2e` : **26 « conforme »** ;
3. `timeout 10m make class-coverage-net` : identique à `tools/class_coverage_net_expected.txt` ;
4. `timeout 10m make sanitize-net` : `sanitize: 26 propre(s)` ;
5. `make value-members` : `membres pico8_value_t : 0` ;
6. `make data-guard` : code 0 ;
7. `make data-residue` : sortie attendue, ou écart expliqué.

### Contrôles du vérificateur

1. **Périmètre du diff** : `git show --stat` liste les 48 chemins de « Ce qui ne change pas » et rien d'autre. Aucun `*.sha256`, rien sous `tests/e2e/`, aucun fichier d'événements, aucun fichier de `docs/` sauf l'éventuelle note du plan.
2. **Crochets** : chaque remplacement suit le tableau. `entity.c` rend `header->cls->field(e, name, out_value)` ; chaque ancien `CHILD_EXIT(0)` est un `_exit(0);` ; `jc.c` garde son `for`.
3. **Conditions intactes** : dans les hunks des 21 fichiers à messages, seules des lignes de chaînes littérales changent. Aucun opérande d'`assert` ne bouge.
4. **Messages** : aucun ne contient `census`, `plan-3`, `section 5` ni de jeton de genre du recensement (`ref:`, `fixe`, `chaine`). Chacun garde « do not widen » (famille A) ou « do not add » (famille B), ou « -- stop » (famille C).
5. **Script** : `tools/entity_value_members.sh` n'a plus `--c-table`, ni `ctable`, ni `offsetof`. Sa sortie en mode liste et en mode `--check` est identique à celle du départ, comparée par `diff`.
6. **`Makefile`** : plus de `CENSUS`, `census` ni `member_census` ; `ENGINE_SRCS` et `DATA_ENGINE_SRCS` ont chacun une ligne de moins ; la légende n'a plus `make member-census`.
7. **Commentaires** : ceux qui sont reformulés relèvent des quatre sortes de `CLAUDE.md`, et aucun n'est ajouté.

### Critères d'échec (traduits en `FAILED:` par le vérificateur de `prd-autopilot`)

**Départ et filet (§7)**

- `FAILED:` si l'arbre de départ n'est pas `6458511` (ou un commit de docs au-dessus) propre, ou si `make value-members` n'y rend pas `membres pico8_value_t : 0`.
- `FAILED:` si le filet (`make test`, `make e2e`, `make class-coverage-net`, `make sanitize-net`) reste rouge après **3 tentatives**.
- `FAILED:` si un `*.sha256` est modifié ou supprimé, si un fichier de `tests/e2e/` est touché, ou si un e2e est ajouté ou retiré.
- `FAILED:` si `make e2e` ne rend pas **26 « conforme »**, ou si `make class-coverage-net` diffère de `tools/class_coverage_net_expected.txt`, ou si `make sanitize-net` ne rend pas 26 propres.
- `FAILED:` si un test change dans son intention : condition, appel, valeur attendue. Seuls sont admis `_exit(0)` à la place de `CHILD_EXIT(0)`, le retrait de `STOP()`, le renommage de `census_kind` et la reformulation de commentaires.

**Périmètre**

- `FAILED:` si `grep -rniE 'census' src tests tools Makefile` ou `grep -rnE 'plan-3|section 5' src tests` n'est pas vide, ou si `src/pico8_member_census.*` ou `tools/member_census.*` existe encore.
- `FAILED:` si une condition d'`assert` change, si un `assert` perd son message, ou si un message réécrit perd son piège (« do not widen » / « do not add » / « stop »).
- `FAILED:` si le diff touche une conversion de D : table ob, dispatch, `overrides`, spec, `pico8_value_t`. Ou s'il écrit la liste ordonnée de D, ou une ligne D.2 ou suivante.
- `FAILED:` si le diff ajoute un TU, un e2e, une cible `make` ou un mode de script.
- `FAILED:` si `tools/data_residue.sh`, `tests/test_data_residue.c`, `DATA_RESIDUE_GUARD` ou un motif du compteur change.

**Gardes**

- `FAILED:` si `make value-members` ou `sh tools/entity_value_members.sh --check` change de sortie ou de code par rapport au départ, ou si `make data-guard` n'est pas vert.
- `FAILED:` si la sortie de `make data-residue` diffère de H5 sans explication dans le commit.

**Commit et note**

- `FAILED:` si le travail n'est pas en un seul commit vert.
- `FAILED:` si le budget est dépassé sans note de ce qui n'a pas été lancé, ou s'il manque le tableau « Runs », ou le pied de `make data-residue` avant et après dans le message de commit.
- `FAILED:` si un commentaire ajouté ou reformulé sort des quatre sortes de `CLAUDE.md`.

## Out of Scope

- **Toute conversion de la phase D** : tables ob, `unstash`, dispatch à `pico8_value_t`, `overrides`, spécifications de classe, `pico8_list_t`, `malloc`.
- **L'écriture de la liste ordonnée de D** (groupes, « Compte : N », rattachement des tests du §6). D.1 est déjà nommée par le plan et peut passer avant elle (voir PRD de D.0, Further Notes).
- **Renommer** les variables `measured`, le message « unmeasured REF » de `ui_button.c:43`, et les noms de tests `…_off_row`.
- **Nettoyer les mentions historiques** du recensement dans `docs/plan-3.md` et `docs/prd/`.
- Un code de sortie 2 sur option inconnue pour `entity_value_members.sh`, et une garde « census » dans `data-guard`. Ce sont des fonctions spéculatives, qu'aucune puce ne demande.
- `make ref-audit`, `make raylib`, l'appli raylib, l'audio, le rendu.
- Le `malloc` des entités (Plan 2 §8), TROU-7.

## Further Notes

- **Décisions de l'agent à la place de l'utilisateur** (consigne `to-prd` : vérifier modules, tests et découpage avec l'utilisateur), tranchées en `guideline` et marquées (G) plus haut :
  - **modules** : aucun module créé. Un module est supprimé (`pico8_member_census`), deux outils aussi, et un mode de script. Les 108 messages suivent trois patrons.
  - **`WALK`** : il est supprimé bien que la puce ne l'énumère pas. C'est un crochet du même en-tête, noté dans le commit.
  - **tests** : aucun TU ni e2e nouveau. La preuve vient des filets inchangés, des gardes et du `grep`, et tient dans un budget court.
  - **découpage** : un seul commit. Les trois découpes sont écartées, et il n'y a pas de section « Implementation Chunks ».
- **Note du plan (G)** : **pas de « Découvert à D.1 » par défaut.** Le §7 ne garde dans le plan que ce qui change une étape suivante. Les comptes (69 crochets, 108 messages, résidu 9702 → 9678) vont dans le message de commit. Une note n'est ajoutée à la puce D.1 que si l'implémentation découvre un tel fait, par exemple un appelant de `--c-table` non vu par grep.
- **Surprise pour le pilote** : le PRD de D.0 est encore dans `docs/prd/triage/`, alors que D.0 est implémentée (`3cac4b5`). La graduation de D.0 (déplacement vers `docs/prd/done/`) n'est pas faite. Ce n'est pas le travail de D.1, mais le pilote devrait la faire avant ou avec le PRD de D.1. Sinon, `prd-autopilot` reprendrait D.0.
- **Contrôle d'étape** : un sous-agent frais relit le diff contre ce PRD avant le commit (hors budget de tests).
- Le message de commit porte :
  - le périmètre, avec les comptes de l'inventaire : 69 crochets dans 40 fichiers, 108 messages dans 21 fichiers, 11 lignes de `tests/`, 48 chemins ;
  - ce que le recensement mesurait (genres des membres `pico8_value_t`, à la fin des constructeurs, dans les lectures et écritures par nom et sur `hv`) et pourquoi il part (sortie de 1b) ;
  - les décisions (G) et leurs rejets, dont `WALK` ;
  - les trois patrons de messages ;
  - le pied de `make data-residue` avant et après ;
  - le tableau « Runs » résumé ;
  - les réponses à H1 à H14.
