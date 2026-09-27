---
title: "Plan 3, étape 1a.36 : le `fm` provisoire, instance sans classe, devient une struct sans parent aux deux membres `jk`/`nz` que la triche écrit sur l'écran titre"
description: "Entre `pico8_probe_init` et le `pico8_probe_set_fm` d'`eu()`, le slot `fm` tient encore une instance de la couche objet construite à la main (tag `INSTANCE`, table vide, aucune classe), que le code de triche peut écrire sur l'écran titre (`jk`=99, `nz`=200, figé par `test_pico8_hy`). Elle devient une struct sans parent, en stockage statique remis à nil par chaque `pico8_probe_init`, avec deux membres `jk`/`nz`, un lecteur, sans méthode ni écrivain. `apply` de `cheat.c` aiguille alors sur le descripteur, plus sur le tag, et la libération par tag de `pico8_probe_set_fm`/`shutdown` ne libère plus qu'un `hy`."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.36 — `fm` : sans classe (`probe.c:385`), provisoire, remplacée par un `hy` (`cart.c:207`). Tests : ceux de 1a.35.

Les numéros de ligne ont dérivé. À HEAD 296933e :
- le provisoire est construit à la main dans `pico8_probe_init` (`probe.c:527-530` : `malloc`, tag `INSTANCE`, `fields` = `pico8_ob_table_new()`, `cls` NULL) ;
- il est remplacé par `pico8_probe_set_fm((pico8_instance_t *)pico8_hy_new(NULL))` (`cart.c:204`), dans `eu()`.

« Ceux de 1a.35 » sont les huit tests de régression de la ligne 1a.35 : `cheat_code`, `empty_planet_cleared`, `playability`, `slipway_drag`, `game_end`, `geoharvest_burst`, `score_panel_stars`, `tech_screen`.

Règles du §5 qui s'appliquent à ce commit :

> Chaque classe devient un `struct` […] ; chaque champ de sa chaîne (défauts et champs écrits sur instance) devient un membre de type `pico8_value_t`, **nom minifié conservé**. Nil reste `pico8_value_ref(NULL)`.
>
> **Instances sans classe** : converties comme des classes à part entière, sans parent.
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui. […]

§6 : les tests de régression ne sont jamais supprimés. Un test qui doit changer ne change que sa façon de lire l'état, et il est prouvé rouge dans ce commit (bug remis localement, test rouge, correctif restauré). §7 :
- un seul commit ;
- `make class-coverage-net` identique à `tools/class_coverage_net_expected.txt` ;
- preuve des tests par `gcov` et 2 à 3 mutations (à partir de 1a.23) ;
- note « Découvert à 1a.36 », réduite à ce qui change une étape suivante ;
- un écart port/cart découvert est préservé et décrit par un `TROU-N`, jamais corrigé.

### Ce que 1a.7, 1a.9 et 1a.35 ont posé (commits 4b257ee, a1a9d2a, 2274c98), réutilisé tel quel

- **Modèle des provisoires `je` (1a.7) et `mo` (1a.9)** :
  - struct privée dont le premier membre est l'en-tête `pico8_struct_header_t`, sans `pico8_ke_t` ;
  - descripteur `static const` sans méthode, avec un lecteur. L'écrivain est NULL, donc une écriture générique avorte dans `pico8_entity_set_field` ;
  - un unique objet statique, initialisé à la compilation, rangé dans le slot par un cast vers le type du slot ;
  - la branche de libération « si tagué `INSTANCE` » est supprimée, et le slot n'est jamais libéré ;
  - un accesseur public du descripteur (`pico8_je_placeholder_struct_class()`, `pico8_mo_placeholder_struct_class()`) ;
  - un nouveau test de caractérisation figé sur l'ancien code, puis des vérifications de forme ajoutées après la conversion.
- **Différence de fond avec `je`/`mo`** : aucun écrivain ne les atteignait. Ils sont donc restés à en-tête seul. Le `fm` provisoire, lui, **est écrit** (voir l'emprise).
- **1a.35 sur le slot `fm`** :
  - `pico8_fm()` garde son type `pico8_instance_t *` ;
  - `pico8_fm_hy()` rend NULL si le slot est vide, le `hy`, ou avorte sur le provisoire (« a hy function on the provisional fm was never measured; 1a.36 converts it ») ;
  - `fm_free` (`probe.c:533-539`) libère un occupant `STRUCT` par `free`, et tout autre par `pico8_instance_free`. `pico8_probe_set_fm` et `pico8_probe_shutdown` l'appellent ;
  - `apply` de `cheat.c` est à double forme par tag : sur une `STRUCT`, il écrit `pico8_fm_hy()->jk`/`nz` ; sinon (le provisoire) il écrit `jk`/`nz` dans sa table, par `pico8_instance_fields` ;
  - tous les lecteurs de `fm` de `src/` passent par `pico8_entity_get_field` : `tech_gate.c`, `menu.c` (`cr`), `score_panel.c`, `score_panel_display.c`, `economy_score.c` ;
  - la ligne 1a.35 du plan nomme ce qui reste à double forme « jusqu'à 1a.36 » : `apply`, la libération par tag, les lecteurs par `pico8_entity_get_field`.

### Ce que fait le cart

`pico8/lua.lua:1182` : `eu(nj)` fait `jl()bg()bw(90)fm,ju=hy(),pj()…`, et `:1186` `fm:lj()`. **Dans le cart, `fm` vaut nil avant `eu()`.** Aucun lecteur ne le touche sur l'écran titre. Une écriture `fm.jk=99` sur ce nil y serait une erreur Lua.

Le code de triche n'existe pas dans le cart : c'est un ajout du port (étape 0.1, `docs/prd/done/code-triche-boutons-ox.md`), qui tourne à chaque frame dès l'écran titre dans les trois pilotes. Le `fm` provisoire est donc un artefact du port. La triche sur l'écran titre, elle, est un comportement du port **figé par une caractérisation** (voir plus bas). Ni l'un ni l'autre n'est un écart port/cart au sens du §7 : pas de `TROU` nouveau (1a.35 a tranché de même).

### Emprise réelle du `fm` provisoire (mesurée à 296933e)

**Méthode.**
- Grep de `src/` et `tests/`.
- **Sonde** sur une copie `git archive HEAD` jetable du scratchpad, supprimée après usage :
  - le provisoire est suivi de sa construction dans `pico8_probe_init` jusqu'à sa libération dans `fm_free` ;
  - `pico8_instance_get_field` et `pico8_instance_fields` journalisent tout accès au provisoire (nom lu, pile d'appel résolue par `addr2line`). Ce sont les deux seules portes vers sa table : aucun `->fields` hors de `pico8_class.c` ne le vise (grep), et `pico8_entity_get_field`/`set_field` passent par elles sur une instance ;
  - la branche « provisoire » de `apply` journalise chaque passage ;
  - journal par processus, fils forkés compris.
- **Rejoués** :
  - les 84 binaires de test de `make unit`, `score_panel_stars` avec 1 et 2, `planet_gh_row_lq` sous `ASAN_OPTIONS=detect_leaks=0`, et les 8 de `UNIT_TESTS_SLOW` : tous verts ;
  - `pico8_replay_e2e` sur les 20 `tests/fixtures/*events*.json` non malformés, **chacun avec et sans `--from-title`** : 40 rejeux, tous en sortie 0.

| Aspect | Mesure |
|---|---|
| Construction | 1 site : `pico8_probe_init`, appelé seulement par `pico8_cart_load` (`cart.c:73`) et directement par `test_pico8_probe_mm_rnd_order`. `pico8_probe_init` commence par `pico8_probe_shutdown`. Pas de classe, pas de `pico8_class_instantiate`, pas de `lt` : le provisoire n'est **jamais dans `hv`/`cv`/`cg`**, ni atteint par `mv`, `dc`, `jz`, `gw`. |
| Méthodes | aucune. `pico8_entity_get_field(fm,"hk")` rend « absent ». |
| `->defaults` | aucun. |
| **Lecteurs** | Deux sites dans tout le dépôt, les deux par `pico8_entity_get_field` : `test_pico8_jc.c:112` (`nz`, 114 lectures, repli avant `eu()` de `fold_field`, `:331`) et `test_pico8_hy.c:96` (`lookup` : `jk` et `nz`, 2 lectures chacun, dans `test_cheat_on_provisional`). **Aucune lecture dans `src/`** sur les 40 rejeux : l'écran titre ne lit pas `fm`. |
| **Écrivains** | Un seul : la branche provisoire de `apply` (`cheat.c:36-40`). Elle fait 3 appels à `pico8_instance_fields` par passage (garde NULL, `jk`, `nz`) et écrit `jk`=99 puis `nz`=200 en fixe. Atteinte **1 fois** en TU (`test_pico8_hy`, `test_cheat_on_provisional`) et **2 fois** sur les rejeux, dans une seule configuration hors filet : `cheat_code_events.json --from-title`. Ce fichier n'est ni dans `FRAMES_EVENTS` ni dans `RECORDINGS`. Sans `--from-title`, et sur tout rejeu du filet, zéro. Aucun écrivain générique (`pico8_entity_set_field`) ne l'atteint : tout accès `fields` vient de `apply`. |
| Pilotes | `main.c:94`, `main_class_coverage.c:69` et `main_raylib.c:100` appellent `pico8_cheat_step` à chaque frame, écran titre compris. Un humain qui tape la séquence sur l'écran titre de l'appli raylib atteint donc la branche provisoire. Cela a été lu dans le code, pas lancé (CLAUDE.md). |
| Fonctions de `hy` | Jamais appelées sur le provisoire (l'assert de `pico8_fm_hy()` le garde depuis 1a.35 ; aucun abandon sur les 40 rejeux). |
| Vie | De `pico8_probe_init` au `pico8_probe_set_fm` d'`eu()`, ou au `pico8_probe_shutdown` suivant. Sur les rejeux `--from-title` d'un fichier qui ne clique jamais la difficulté (`demo`, `game_end`, `tech_screen_invent`, …), il vit jusqu'à la sortie du processus sans être libéré. |
| Libération | par `fm_free` → `pico8_instance_free` (instance + table) : dans `pico8_probe_set_fm` au premier `eu()`, ou dans `pico8_probe_shutdown`. |
| Couverture | **aucun nom** : il n'est jamais construit par `pico8_class_instantiate`, donc jamais compté. Absent de `KNOWN[]` et de `tools/class_coverage_net_expected.txt`. |
| `pico8_probe_set_fm` | appelé seulement par `cart.c:204`, avec un `hy`. Aucun test ne range autre chose dans le slot. **Après ce commit, `g_fm` vaut donc le provisoire statique, un `hy` struct ou NULL, jamais une `INSTANCE`.** |

### Ce que les tests figent du provisoire

- **`test_pico8_hy`, `test_cheat_on_provisional` (`:530-561`)**, caractérisation figée sur l'ancien code à 1a.35 :
  - après `pico8__init()`, `pico8_fm()` est non NULL et n'a ni `jk` ni `nz` ;
  - la séquence complète tapée avant `eu()` fait `fired == 1` et laisse `pico8_fm()` **au même pointeur** ;
  - `jk` vaut 99 et `nz` vaut 200, lus sur le provisoire ;
  - après `eu()`, le `hy` a `jk`=0, `nz`=100.

  **Ces valeurs imposent la forme** : un provisoire à en-tête seul, ou une triche sans effet sur l'écran titre, rendrait `jk`/`nz` absents, ce qui change une valeur figée (arrêt §7).
- **`test_pico8_hy`, `test_hy_is_a_struct` (`:640-671`)**, vérification **de forme** ajoutée au morceau 2 de 1a.35, pas une valeur figée :
  - `:647` asserte que le provisoire est tagué `INSTANCE`. Cette ligne devient fausse par construction ;
  - `:648` : `pico8_fm_hy()` avorte sur le provisoire. Cela reste vrai.
- **`test_pico8_jc:331`** : `nz` absent sur le provisoire. Diff vide.
- `test_pico8_probe_mm_rnd_order` : `pico8_probe_init`/`shutdown` sans `eu()`, sans lecture de `fm`. Diff vide.

### Les huit tests de 1a.35 après 1a.35

Mesuré par la sonde : **aucun ne lit ni n'écrit le provisoire**. Chacun le voit construit par `pico8_cart_load`, puis libéré par le `pico8_probe_set_fm` d'`eu()` : 1 construction, 1 libération, 0 lecture, 0 passage dans `apply` côté provisoire. Toutes leurs lectures et écritures de `fm` ont lieu après `eu()`, sur le `hy`, par `pico8_entity_get_field` ou `pico8_fm_hy()` (réécrites à 1a.35).

| Test | Touche le provisoire ? | Chemin changé par 1a.36 qu'il traverse | Diff |
|---|---|---|---|
| `cheat_code` | non : `pico8__init`, puis `eu()`, puis la séquence ×2 sur le `hy` | `apply` (branche `hy`), `fm_free` du provisoire à `eu()` | vide |
| `empty_planet_cleared` | non | `fm_free` à `eu()` | vide |
| `playability` | non | `fm_free` à `eu()` | vide |
| `slipway_drag` | non | `fm_free` à `eu()` | vide |
| `game_end` | non | `fm_free` à `eu()` | vide |
| `geoharvest_burst` | non | `fm_free` à `eu()` | vide |
| `score_panel_stars` (1 et 2) | non | `fm_free` à `eu()` | vide |
| `tech_screen` | non | `fm_free` à `eu()` | vide |

Aucun ne doit changer, ni dans sa façon de lire ni dans son intention. Chacun est tout de même remis à l'épreuve dans ce commit (§6 « vérifié dans ce commit », précédent `planet_kj_dispatch` à 1a.9), avec les bugs de la table de 1a.35 (voir Testing Decisions).

## Solution

1. **Struct du `fm` provisoire, sans parent** : l'en-tête `pico8_struct_header_t`, puis `jk` et `nz` (`pico8_value_t`), dans l'ordre d'écriture de `apply`. Pas de `pico8_ke_t` (il n'est pas une entité de `hv`). Aucun autre membre du `hy` : le provisoire ne les a jamais eus, et chaque lecteur tolère leur absence.
2. **Descripteur `static const`** :
   - aucune méthode ;
   - un lecteur qui sert `jk` et `nz` (membre nil = absent) et rend 0 pour tout autre nom, sans assert, comme une instance à table vide ;
   - **écrivain NULL** : aucun écrivain générique ne l'atteint (mesuré). S'il l'atteignait un jour, l'assert de `pico8_entity_set_field` partirait. C'est un arrêt §7, pas un membre improvisé.
3. **Stockage statique, remis à nil à chaque `pico8_probe_init`** :
   - un unique objet, défini dans le module sonde (tag `STRUCT`, descripteur fixés à la compilation) ;
   - `pico8_probe_init` remet `jk`/`nz` à nil (`pico8_value_ref(NULL)`), puis range son adresse dans `g_fm` par un cast vers le type du slot, au lieu du `malloc` ;
   - la remise à nil reproduit la table vide neuve que chaque `pico8_probe_init` construit aujourd'hui.
4. **Libération** : `fm_free` ne libère plus qu'un `hy` (par `free`, TROU-7 comme aujourd'hui). Le provisoire n'est jamais libéré, et NULL ne fait rien. `pico8_instance_free` quitte `pico8_probe.c` s'il n'y sert plus. `pico8_probe_shutdown` remet toujours `g_fm` à NULL.
5. **Triche** : `apply` aiguille sur le **descripteur**, plus sur le tag :
   - slot vide → rien ;
   - descripteur du provisoire → il écrit ses membres `jk`=99 et `nz`=200 ;
   - sinon → `pico8_fm_hy()` (qui avorte sur tout autre chose qu'un `hy`), puis les membres `jk`/`nz` du `hy`, comme aujourd'hui.

   La garde `pico8_instance_fields(fm) == NULL` et les appels à `pico8_ob_table_set_name` disparaissent. Les includes que cela rend inutiles dans `cheat.c` sont retirés, et rien d'autre.
6. **Accès public** : le type du provisoire est déclaré dans `pico8_probe.h`, puisque `cheat.c` écrit ses membres. L'accesseur du descripteur s'appelle `pico8_fm_placeholder_struct_class()`, calqué sur `pico8_je_placeholder_struct_class()`/`pico8_mo_placeholder_struct_class()`.
7. **`pico8_fm_hy()`** : même contrat (NULL, le `hy`, assert sur le provisoire). Seul le message d'assert change : « 1a.36 converts it » devient faux. Nouveau message : un appel de fonction de `hy` sur le provisoire est un `fm` nil dans le cart.
8. **Contrats d'en-tête corrigés**, seulement ceux que ce changement rend faux :
   - `pico8_probe_init()` (`probe.h:52-55`) : « installs the `fm` placeholder » devient « remet en place le provisoire, `jk`/`nz` nil » ;
   - `pico8_probe_shutdown()` (`:57-59`) : « Frees … whichever instance `fm` currently holds » devient « libère le `hy` de `fm` s'il y en a un ; le provisoire n'est jamais libéré » ;
   - `pico8_fm()` (`:77-82`) : « the class-less placeholder instance » devient « le provisoire, une struct statique qui n'a que `jk`/`nz` (écrits par la triche) ». « frees the previous one by its tag: the placeholder with its table » devient « libère un `hy` précédent, struct seule (TROU-7) ; jamais le provisoire » ;
   - le commentaire de `apply` (`cheat.c:24-25`, « still the placeholder instance ») : le mot « instance » tombe, et la phrase dit que la triche écrit le provisoire sur l'écran titre.
9. **Rien d'autre** : `pico8_fm()` et `pico8_probe_set_fm` gardent leur type ; les lecteurs de `fm` de `src/` restent par `pico8_entity_get_field` (ils lisent « absent » sur le provisoire, qui n'a que `jk`/`nz`) ; `hy`, `cart.c`, `menu.c`, l'enum de méthodes, `KNOWN[]` ne bougent pas.
10. **Plan** : à la ligne 1a.36 de `docs/plan-3.md`, ajouter « Découvert à 1a.36 : » réduit à ce qui change une étape suivante (§7). A priori : le provisoire garde `jk`/`nz` (triche sur l'écran titre) ; les lecteurs de `fm` restent par `pico8_entity_get_field` et le type `pico8_instance_t *` du slot tombe à 1a.42 ; enum inchangée. Sites, comptes et mesures vont dans le message de commit.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que le `fm` provisoire soit la seule chose convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux l'emprise du provisoire mesurée (construction, lecteurs, écrivains, pilotes avec et sans `--from-title`, tests, libération, couverture), afin que chaque membre et chaque absence de membre soient justifiés par une mesure.
3. En tant que lecteur du plan, je veux que le provisoire devienne une struct sans parent, afin d'appliquer « converties comme des classes à part entière, sans parent » à la lettre.
4. En tant que mainteneur, je veux que la struct n'ait que `jk` et `nz`, les deux seuls champs écrits sur le provisoire, afin d'appliquer « champ écrit sur instance → membre » sans inventer les neuf autres champs du `hy`.
5. En tant que joueur qui tape A A B B A A B B sur l'écran titre, je veux que le code fasse exactement ce qu'il fait aujourd'hui (le provisoire reçoit 99 et 200, la partie démarre à 0 recherche et 100 crédits), afin que la conversion ne change aucun comportement.
6. En tant que joueur, je veux que la triche tapée en partie mette toujours 99 recherche et 200 crédits sur le `hy`, afin que `cheat_code` reste vrai.
7. En tant que gardien du filet, je veux que toute lecture de `fm` sur le provisoire rende « absent » pour tout nom sauf `jk`/`nz` écrits, afin que `tech_gate`, `cr`, `pj:hb`, `pj:ph` et `economy_score` lisent comme aujourd'hui s'ils l'atteignaient.
8. En tant que gardien du filet, je veux que le provisoire n'ait aucune méthode, afin que `pico8_entity_get_field(fm,"hk")` rende « absent » comme aujourd'hui.
9. En tant que mainteneur, je veux que le provisoire n'ait pas d'écrivain générique, afin qu'une écriture jamais mesurée arrête le programme au lieu d'être rangée en silence.
10. En tant que mainteneur, je veux que la triche écrive les membres par aiguillage sur le descripteur, afin que `apply` n'ait plus de branche « instance » et que la double forme notée à 1a.35 se ferme.
11. En tant que gardien des tests, je veux que chaque `pico8_probe_init` remette `jk`/`nz` à nil, afin qu'une triche d'une session précédente ne survive pas à un nouvel init, comme la table neuve d'aujourd'hui.
12. En tant que gardien de la mémoire, je veux que le provisoire ne soit ni alloué ni libéré, afin qu'aucun `pico8_probe_set_fm` ou `pico8_probe_shutdown` ne passe un pointeur statique à `free`.
13. En tant que gardien de la mémoire, je veux que `fm_free` libère encore un `hy` remplacé comme aujourd'hui (struct seule, TROU-7), afin que l'étape ne corrige ni n'aggrave une fuite connue.
14. En tant que mainteneur, je veux que `pico8_probe_shutdown` remette toujours `g_fm` à NULL, afin que `pico8_fm()` et `pico8_fm_hy()` rendent NULL après un arrêt, comme aujourd'hui.
15. En tant que mainteneur, je veux que `pico8_fm_hy()` avorte toujours sur le provisoire, avec un message qui dit vrai, afin qu'un appel de fonction de `hy` avant `eu()` (une erreur Lua dans le cart) reste détecté.
16. En tant que gardien du filet, je veux que `pico8_fm()` et `pico8_probe_set_fm` gardent leur type, afin que `cart.c`, les lecteurs et les tests ne soient pas touchés.
17. En tant que gardien de la couverture, je veux que le provisoire ne soit jamais compté et qu'aucun nom ne soit ajouté à `KNOWN[]`, afin que `make class-coverage-net` reste identique à la référence.
18. En tant que mainteneur, je veux un accesseur public du descripteur, afin qu'un test prouve la forme sans lire l'agencement mémoire.
19. En tant que gardien des tests de régression, je veux que les huit tests de 1a.35 passent avec un diff vide, puisque aucun ne lit ni n'écrit le provisoire (mesuré).
20. En tant que gardien des tests de régression, je veux que chacun des huit soit remis à l'épreuve dans ce commit (bug remis, rouge, restauré, vert), afin de prouver qu'il garde encore son bug après que `apply` et `fm_free` ont changé.
21. En tant que gardien des caractérisations, je veux que `test_cheat_on_provisional` et toutes les valeurs figées de `test_pico8_hy` passent sans changement, afin que la conversion ne se valide pas en réécrivant son témoin.
22. En tant que gardien des caractérisations, je veux que la seule ligne de `test_pico8_hy` qui change soit l'assertion de forme `:647` (tag `INSTANCE` du provisoire), réécrite en tag `STRUCT` au descripteur du provisoire, afin que la seule modification soit celle qu'impose la conversion.
23. En tant que vérificateur, je veux une caractérisation `test_pico8_fm` figée sur l'ancien code, afin de prouver ce que `test_pico8_hy` ne couvre pas : la remise à nil par un nouvel init, l'absence des autres noms, le cycle init/shutdown, la triche après shutdown.
24. En tant que vérificateur, je veux un rapport `gcov` qui montre chaque ligne ajoutée ou modifiée exécutée, plus 2 à 3 mutations ciblées, afin de juger la preuve sans monter mon propre banc.
25. En tant que gardien du filet, je veux qu'aucun `.sha256` ni aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
26. En tant qu'auteur des étapes suivantes, je veux que `hy`, `g_`, `ui`, l'enum de méthodes et les `->defaults` ne changent pas, afin que leurs étapes restent séparées.
27. En tant que lecteur du code, je veux que les contrats d'en-tête de `pico8_probe_init()`, `pico8_probe_shutdown()`, `pico8_fm()` et le commentaire de `apply` disent la vérité après ce commit (CLAUDE.md).
28. En tant que lecteur du code, je veux qu'aucun commentaire de récit de chantier ne soit ajouté, afin de respecter la règle des quatre destinations.
29. En tant qu'autopilote, je veux qu'un écart port/cart découvert soit préservé et décrit par un `TROU-N`, afin que la conversion ne corrige rien en passant (le `fm` nil du cart n'est pas « corrigé » en un slot vide).

## Implementation Decisions

Cette session n'a pas d'outil pour lancer un agent `guideline` frais. J'ai donc chargé et appliqué moi-même le skill `guideline` (hypothèses explicites, simplicité, changement chirurgical, critères vérifiables). Les décisions tranchées ainsi sont marquées (G).

- **Forme retenue : en-tête + `jk` + `nz`** (G). C'est le point qui diffère de 1a.7/1a.9.
  - Rejeté : l'en-tête seul, calqué sur `je`/`mo`. La triche sur l'écran titre ne pourrait plus rien ranger, et `test_cheat_on_provisional` perdrait `jk`=99/`nz`=200 : valeur figée changée, arrêt §7.
  - Rejeté : un écrivain qui accepte et jette. Même perte, et une écriture suivie d'une lecture mentirait.
  - Rejeté : une vraie instance `hy` construite dans `pico8_probe_init`. Elle compterait `hy` au crochet (`hy  19  15/15` bougerait), et tout lecteur de `fm` sur l'écran titre verrait `nz`=100 au lieu de « absent ».
  - Rejeté : `fm` = NULL avant `eu()`, bien que plus proche du cart. La ligne dit « provisoire, remplacée », pas « supprimée ». La triche n'aurait plus d'effet sur l'écran titre (valeur figée changée). `test_pico8_jc:331` suivrait une autre branche (`0xDEAD` au lieu de `fold_field`), donc son empreinte figée changerait aussi.
  - **Règle du §5 qui couvre `jk`/`nz`** : « champs écrits sur instance → membre ». L'écrivain est `apply`, mesuré seul. Aucun champ sans règle.
- **Écrivain NULL, et la triche écrit les membres** (G, précédent 1a.35 pour `hy` : « `cheat.c` et `menu.c` savent qu'ils écrivent `fm` : ils écrivent le membre »). Rejeté : un `set_field` pour la seule triche, qui ouvrirait à tout écrivain générique futur une porte que le §5 réserve aux écrivains génériques mesurés.
- **Aiguillage de `apply` par le descripteur** (G). Le tag ne distingue plus rien : les deux occupants sont des `STRUCT`. `cheat.c` compare le descripteur de l'en-tête à `pico8_fm_placeholder_struct_class()` et caste vers le type public. Tout autre occupant passe par `pico8_fm_hy()`, qui garde son assert. Rejeté : un second accesseur typé `pico8_fm_placeholder()`, une fonction de plus pour un seul appelant.
- **Lecteur qui rend 0 pour tout autre nom, sans assert** (G, précédent `je`/`mo`). Aucun lecteur de `src/` n'atteint le provisoire (mesuré). Les deux lecteurs de test demandent `jk`/`nz`. Les lecteurs de `src/` tolèrent l'absence de tout nom (garde `pico8_fm() != NULL && pico8_entity_get_field(...)`). Un assert ne garderait rien. Le lecteur ne peut pas être NULL : `pico8_entity_get_field` l'asserte.
- **Statique, remis à nil par `pico8_probe_init`** (G). L'état mutable se réduit à `jk`/`nz`, que chaque init remet à nil comme la table neuve d'aujourd'hui. Aucun code ne compare l'identité du provisoire d'un init à l'autre : `test_cheat_on_provisional` la compare **dans** une même vie. Rejeté : `malloc` à chaque init et libération « si descripteur provisoire ». Cela garderait une branche de libération pour un objet à deux valeurs.
- **`fm_free`** : NULL et le provisoire → rien ; sinon → `free` (un `hy`, seul autre occupant possible, puisque `pico8_probe_set_fm` n'est appelé qu'avec un `hy`). Un assert porteur que l'occupant libéré est un `hy` est permis (contrat C, CLAUDE.md), pas exigé.
- **Pas de constructeur, pas de couverture** : la construction se réduit à l'initialiseur statique et à la remise à nil. Le crochet n'est pas appelé : un nom compté hors `KNOWN[]` ferait échouer l'outil, et l'ajouter changerait la sortie attendue.
- **Emplacement** : objet et descripteur privés dans `pico8_probe.c`, qui détient `g_fm`. Le type et l'accesseur sont déclarés dans `pico8_probe.h`, qui inclut déjà `pico8_struct.h` et `pico8_value.h`. Le nom est rattaché à `fm`, « placeholder » comme `je`/`mo`. Il n'y a pas de nom minifié du cart à conserver : le cart n'a pas de valeur pour `fm` avant `eu()`. Les membres gardent les noms minifiés `jk`/`nz`.
- **Type du slot** : `g_fm`/`pico8_fm()`/`pico8_probe_set_fm` restent en `pico8_instance_t *` (décision de 1a.6, suivie en 1a.7, 1a.9 et 1a.35). Ce type disparaît à 1a.42.
- **Lecteurs de `fm` dans `src/`** : inchangés, par `pico8_entity_get_field`. Les passer aux membres du `hy` demanderait un aiguillage sur le provisoire à chaque site, pour rien : ce n'est pas l'objet de la ligne.
- **Écarts port/cart** : aucun nouveau. Le provisoire et la triche sur l'écran titre sont des artefacts du port déjà nommés (plan, 1a.35). Si l'implémentation en découvre un, il est préservé et décrit par un `TROU-N`.
- **Commentaires** (CLAUDE.md) :
  - à ajouter : le contrat C de l'objet (statique, jamais libéré, `jk`/`nz` remis à nil par `pico8_probe_init`, écrits seulement par la triche, pas d'écrivain générique) ;
  - à corriger : ceux de la Solution §7-8 ;
  - tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau laisse `make test` vert.

1. **Caractérisation `test_pico8_fm` sur l'ancien code** (voir Testing Decisions). Les valeurs sont figées **avant toute modification de `src/`**. Mesurer la durée : > 30 s → `UNIT_TESTS_SLOW`, sinon `UNIT_TESTS` (attendu : ~1 s, comme `test_pico8_hy`).
2. **Conversion** :
   - type, descripteur, lecteur, objet statique ;
   - `pico8_probe_init` (remise à nil + rangement), `fm_free`, message de `pico8_fm_hy()`, accesseur, contrats ;
   - `apply` par descripteur ;
   - `test_pico8_hy:647` réécrite (forme) ; vérifications de forme ajoutées à `test_pico8_fm`.

   `test_pico8_fm`, `test_pico8_hy`, `test_pico8_jc` passent **sans changer une valeur figée**.
3. **Preuves** : `make test`, `make e2e`, `make class-coverage-net`, rapport `gcov`, mutations, preuves rouges des huit tests.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rend `pico8_entity_get_field` sur `pico8_fm()`, ce que rend `pico8_cheat_step`, NULL ou non NULL de `pico8_fm()`/`pico8_fm_hy()`, l'identité de `pico8_fm()` dans une même vie, l'abandon d'un fils. Il ne lit jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_fm`**, dans `make unit`, lié et lancé comme `test_pico8_hy` : cart réel chargé par `pico8_cart_load`, chaque scénario dans un fils forké juste après, comme `in_fresh_child`. Il lit **uniquement** par l'API à double forme (`pico8_fm`, `pico8_fm_hy` hors du provisoire, `pico8_entity_get_field`, `pico8_cheat_step`, `pico8_probe_init`/`shutdown`, `pico8__init`, `pico8_eu`), pour que ce soit le même code avant et après. Il reprend le prédicat « nil = absent » de `test_pico8_hy` (`lookup`, TROU-9).
  - **Caractérisation, figée sur l'ancien code** :
    1. **Au repos** : après `pico8__init()`, `pico8_fm()` est non NULL. `jk`, `nz`, `ge`, `ew`, `lo`, `yr`, `invented`, `po`, `fu`, `happiness`, `hl`, `hk`, `md`, `x` sont absents.
    2. **Triche sur l'écran titre** : la séquence complète (frames de `test_cheat_on_provisional`) rend `fired == 1`. `pico8_fm()` garde le même pointeur. `jk` = 99 et `nz` = 200, en fixe. Les 12 autres noms restent absents. Une seconde séquence complète rend les mêmes valeurs.
    3. **Nouvel init** : après la triche, `pico8_probe_init()` donne un `pico8_fm()` non NULL où `jk`/`nz` sont absents.
    4. **Cycle** : `pico8_probe_shutdown()` → `pico8_fm()` et `pico8_fm_hy()` NULL. La séquence complète tapée alors rend `fired == 1`, sans crash ni écriture. Un second `pico8_probe_shutdown()` ne plante pas. `pico8_probe_init()` → lisible comme en 1. Deux `pico8_probe_init()` de suite → lisible comme en 1.
    5. **Remplacement** : triche sur l'écran titre, puis `eu()` (même `new_game` que `test_pico8_hy`) → `jk` = 0, `nz` = 100 (le `hy`). Puis `pico8_probe_init()` → `jk`/`nz` absents.
  - **Forme**, ajoutée au morceau 2 :
    - après `pico8__init()`, `pico8_fm()` est tagué `STRUCT` au descripteur `pico8_fm_placeholder_struct_class()` ;
    - toutes les entrées de méthode sont NULL, `field` est posé, `set_field` est NULL ;
    - après `pico8_probe_init()`, c'est le provisoire **à la même adresse** ;
    - après `eu()`, c'est le descripteur de `hy` ;
    - `pico8_entity_set_field(provisoire, "jk", …)` avorte, vérifié dans un fils (prior art : `must_abort` de `test_pico8_hy`).
- **`test_pico8_hy`** : **seule `:647` change**. `assert(pico8_type_tag_of(pico8_fm()) == PICO8_TYPE_TAG_INSTANCE)` devient « tagué `STRUCT` au descripteur du provisoire ». C'est une vérification de forme ajoutée au morceau 2 de 1a.35, pas une valeur figée. Toutes les empreintes (`*_HASH`), toutes les valeurs de `test_cheat_on_provisional` et `must_abort(fm_hy_on_provisional)` restent identiques.
- **`test_pico8_jc`, `test_pico8_probe_mm_rnd_order`, `test_pico8_je`, `test_pico8_nf`, `test_pico8_mo`, `test_pico8_g_`** : diff vide, verts.
- **Les huit tests de 1a.35 : diff vide.** Preuve rouge de chacun sur le code final, dans une copie jetable, par `make tmp/test_pico8_<nom> && ./tmp/test_pico8_<nom>` (`score_panel_stars` avec 1 et 2). On reprend les bugs de la table de 1a.35 :

  | Test | Remis localement | Attendu |
  |---|---|---|
  | `cheat_code` | dans `apply`, la branche `hy` n'écrit rien (la branche du provisoire reste) | `assert_cheat_applied` faux à la frame 23 |
  | `empty_planet_cleared` | retirer l'entrée `LL` du descripteur de `jo` | une assertion échoue ; noter laquelle |
  | `playability` | constructeur de `jo` qui appelle `pico8_ke_struct_pq` deux fois | `count <= PICO8_PLAYABILITY_MAX_PLANETS` faux |
  | `slipway_drag` | nil à la place de l'entité de `jz("gv")` dans `ui:bh` | `oe_count(origin) == 1` faux |
  | `game_end` | `yr > 3426` au lieu de `>=` dans `hy:lj` | `assert(end_frame >= 0)` faux |
  | `geoharvest_burst` | écriture de `nz` gardée par `jt > 0` dans `hy:lu` | `fm_fixed("nz") == nz_before + 20` faux à `HARVEST_FRAME` |
  | `score_panel_stars` | constructeur de `lq` qui remplit `items` dans l'ordre de stockage | rangée d'étoiles fausse pour 1 et 2 |
  | `tech_screen` | retirer l'ajout de `px` dans `hy:kt` | `invented_count() == 1` faux à la frame 46 |

  Le commit note, pour chacun, le bug remis, l'échec observé, la restauration et le vert. **Si un bug remis ne rend pas son test rouge, c'est un arrêt (§7), pas un test durci.**
- **Preuve des tests (§7)** :
  - **`gcov`** : dans une copie jetable du scratchpad (arbre du commit candidat, supprimée après usage), compiler en `--coverage -O0` `test_pico8_fm`, `test_pico8_hy`, `test_pico8_cheat_code` et `test_pico8_jc` avec les objets du moteur, les lancer, puis `gcov` sur `pico8_probe.c` et `pico8_cheat.c`. Chaque ligne ajoutée ou modifiée doit être exécutée : lecteur (`jk`, `nz`, autre nom), remise à nil, rangement, `fm_free` sur NULL, sur le provisoire et sur un `hy`, les trois branches de `apply`. Une ligne non exécutée est couverte par un test ajouté, ou justifiée dans le commit.
  - **Mutations**, l'implémenteur en remet au moins 3 localement et note lesquelles sont attrapées :
    - lecteur qui rend `jk` pour `nz` (attrapé par la caractérisation 2) ;
    - `pico8_probe_init` qui ne remet pas `jk`/`nz` à nil (caractérisation 3 et 5) ;
    - branche provisoire de `apply` sans effet (caractérisation 2, `test_cheat_on_provisional`) ;
    - `fm_free` qui passe le provisoire à `free` (abandon glibc à `eu()` ou au shutdown) ;
    - `pico8_probe_shutdown` qui ne remet pas `g_fm` à NULL (caractérisation 4) ;
    - écrivain qui accepte et jette (test de forme seulement).
- **Couverture** : `make class-coverage-net` rend 0, sortie identique à la référence.
- **Filet** : `make e2e` vert. Le filet ne voit jamais le provisoire autrement que construit puis libéré (mesuré sur les 15 rejeux du filet). Les `.sha256` sont inchangés par construction.
- **Prior art** : `test_pico8_mo` et `test_pico8_je` (caractérisation puis forme d'un provisoire), `test_pico8_hy` (`test_cheat_on_provisional`, `must_abort`, `in_fresh_child`), la table des preuves rouges de 1a.35.

## Critères du vérificateur

Le vérificateur de `prd-autopilot` traduit chacun de ces critères en `FAILED:` (§7 ; c'est le seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make e2e` (filet compris) est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le code final, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt`. Aussi si ce fichier, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié, ou si le provisoire appelle le crochet de couverture.
- `FAILED:` si un champ ne tombe sous aucune règle du §5, dans l'un de ces cas :
  - la struct du provisoire a un membre autre que `jk`/`nz` ;
  - un écrivain autre que `apply` est trouvé sur le provisoire ;
  - l'assert d'écrivain de `pico8_entity_set_field` part sur le provisoire dans un rejeu ou un test (hors test de forme) ;
  - un lecteur attend du provisoire une valeur pour un nom autre que `jk`/`nz`.

  Dans chacun de ces cas, on s'arrête, sans membre improvisé.
- `FAILED:` si le descripteur du provisoire a une méthode ou un écrivain, ou si la struct contient un `pico8_ke_t`.
- `FAILED:` si l'un des huit tests de 1a.35 a un diff non vide, ou si le commit ne rapporte pas sa preuve rouge (bug remis, rouge, restauré, vert). Aussi si un bug remis ne le rend pas rouge.
- `FAILED:` si un autre test de régression de bug (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une valeur figée change après la conversion, dans l'un de ces tests :
  - `test_pico8_fm` (valeurs prises sur l'ancien code) ;
  - `test_pico8_hy` (empreintes, `test_cheat_on_provisional`) ; dans ce fichier, une ligne autre que l'assertion de forme `:647` modifiée compte aussi ;
  - `test_pico8_jc`, `test_pico8_je`, `test_pico8_mo`, `test_pico8_nf`.
- `FAILED:` si une autre classe est convertie ou change de forme (en particulier `hy`, `g_`, `ui`, les provisoires `je`/`mo`), si l'enum de méthodes change, ou si un `->defaults` bouge.
- `FAILED:` si un écart port/cart est corrigé au lieu d'être préservé, en particulier `fm` rendu NULL avant `eu()` ou la triche rendue sans effet sur l'écran titre.

Critères de réussite (en plus) :
- grep : plus de `malloc` de `g_fm`, ni de `PICO8_TYPE_TAG_INSTANCE`, ni de `pico8_ob_table_new` pour `fm` dans `pico8_probe_init` ; plus de `pico8_instance_free` dans `fm_free` ; plus de `pico8_instance_fields` ni de `pico8_ob_table_set_name` dans `pico8_cheat.c` ;
- dans `src/`, le diff se limite à :
  - `pico8_probe.c` : type, objet, descripteur, lecteur, `pico8_probe_init`, `fm_free`, message de `pico8_fm_hy()`, accesseur ;
  - `pico8_probe.h` : type, accesseur, contrats ;
  - `pico8_cheat.c` : `apply` et son commentaire, includes ;
- dans `tests/` : `test_pico8_fm` nouveau et la ligne `:647` de `test_pico8_hy` ; `Makefile` : cible et entrée `UNIT_TESTS` de `test_pico8_fm` ; `docs/plan-3.md` : la note de la ligne 1a.36.

## Out of Scope

- `hy` (1a.35, fait), ses fonctions, `cart.c`, `menu.c` (`ix[1].my`), et les lecteurs de `fm` de `src/` (restent par `pico8_entity_get_field`).
- Le type du slot `g_fm`/`pico8_fm()`/`pico8_probe_set_fm` (`pico8_instance_t *`), qui disparaît à 1a.42.
- La fuite de `invented`/`po`/`fu` d'un `hy` remplacé (TROU-7, Plan 2 §8).
- L'instance `et={}` de `bv` (`probe.c:165-167`), qui est l'étape 1a.38.
- Rendre `fm` nil avant `eu()` comme dans le cart, ou changer la triche sur l'écran titre : c'est un changement de comportement hors de la ligne.
- Lancer l'appli raylib : jamais.

## Further Notes

- Le filet ne voit jamais le provisoire lu ou écrit. Sur les 15 rejeux du filet (avec `--from-title` pour les 4 enregistrements), il est seulement construit puis libéré par `eu()`. Toute la garde de l'étape repose donc sur `test_pico8_fm`, `test_cheat_on_provisional` et `test_pico8_jc:331`. La couverture est inchangée par construction.
- Configuration hors filet notée pour le commit : `cheat_code_events.json` rejoué avec `--from-title` tape la triche deux fois sur l'écran titre, donc deux passages dans la branche provisoire, et le rejeu ne quitte jamais l'écran titre. Ce fichier n'est ni dans `FRAMES_EVENTS` ni dans `RECORDINGS`, et le filet ne le rejoue jamais ainsi.
- Effet de bord voulu : le `malloc` du provisoire et de sa table à chaque `pico8_probe_init` disparaît. C'est observable seulement par un outil mémoire, pas par un test de comportement.
- Mesures faites à 296933e :
  - grep de `src/` et `tests/` ;
  - lecture de `pico8/lua.lua:1182-1186` (`eu`) ;
  - lecture de `pico8_probe.c`/`.h` (`pico8_probe_init`, `fm_free`, `pico8_probe_shutdown`, `pico8_fm`, `pico8_fm_hy`, `pico8_probe_set_fm`), `pico8_cheat.c`, `pico8_cart.c:73,204,250`, `main.c`, `main_class_coverage.c`, `main_raylib.c`, `pico8_entity.c` (`pico8_entity_get_field`/`set_field`, modèle `mo`), `pico8_class.c` (`pico8_instance_get_field`, `pico8_instance_fields`, `pico8_instance_free`) ;
  - lecture des tests `test_pico8_hy`, `test_pico8_cheat_code`, `test_pico8_jc`, et des sept autres tests de 1a.35.

  La sonde (copie jetable du scratchpad, supprimée) a journalisé chaque accès au provisoire sur tous les TU et 40 rejeux.
