---
title: "Plan 3, étape 1a.9 : le `mo` provisoire, instance sans classe, devient une struct sans parent et sans champ"
description: "Entre `pico8_entity_init` et le `pico8_entity_set_mo` de `jl()`, le slot `mo` tient encore une instance de la couche objet construite à la main (tag `INSTANCE`, table vide, aucune classe), allouée à chaque `jl()` et perdue sans être libérée. Elle devient une struct à en-tête seul, en stockage statique, dont le lecteur ne trouve jamais rien et qui n'a pas d'écrivain. La libération « seulement si tagué `INSTANCE` » de `pico8_entity_shutdown` ne vise alors plus rien et disparaît."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.9 — `mo` : sans classe (`entity.c:129`), provisoire, remplacée par un `qb` (`bootstrap.c:48`). Tests : `planet_kj_dispatch`.

Règles du §5 qui s'appliquent à ce commit :

> **Instances sans classe** : converties comme des classes à part entière, sans parent.
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui. `hv`/`cv`/`cg`, `dc`, `mv` traitent les deux formes pendant la migration ; l'ordre d'enregistrement est préservé.

§6 : `planet_kj_dispatch` est nommé par l'étape (voir plus bas ce qu'il voit réellement du `mo` provisoire). Tout test de régression garde son intention et reste rouge si son bug revient. §7 : le commit passe `make class-coverage-net` avec une sortie identique à `tools/class_coverage_net_expected.txt`. Un site découvert en cours d'étape et couvert par une règle existante du §5 est ajouté à la liste des étapes dans le commit, avec « découvert à 1a.9 » dans le message.

### Ce que 1a.7 et 1a.8 ont posé (commits ec01e86, 470accc), réutilisé tel quel

- **Modèle exact : le `je` provisoire (1a.7).** Struct à en-tête seul (`pico8_struct_header_t`), sans `pico8_ke_t`. Descripteur `static const` sans méthode, lecteur de champ qui rend 0 pour tout nom, écrivain NULL. Un unique objet statique initialisé à la compilation. `pico8_ui_init` range son adresse dans `g_je` par un cast vers le type du slot. La branche de libération de `pico8_ui_shutdown` a été supprimée. Accesseur public du descripteur : `pico8_je_placeholder_struct_class()`. Nouveau test `test_pico8_je` : caractérisation figée sur l'ancien code, puis forme ajoutée après la conversion.
- **1a.8 sur le slot `mo`.** Tout lecteur de `mo` passe par `pico8_entity_get_field` : `gd` et `ma()` (`entity_geom.c`), `pico8_ui_field_vec` (`ui:lj`), `pico8_msgbox_field_vec` (`iv:pq`), `pico8_mv(g_mo,"md",…)` (`dc("kj")`). Tout écrivain de `mo` passe par `pico8_entity_set_field` : `ga:pq` (`mo.ma=3`, `planet_family.c:173`) et `bg:hq` (`mo.v=…`, `bg.c:225`). Les deux gardent `mo != NULL`. `pico8_entity_shutdown` ne libère `g_mo` que s'il est tagué `INSTANCE` ; un `qb` struct est oublié (TROU-7). L'enum de méthodes vaut `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD`. `pico8_entity_set_field` asserte sur une struct sans écrivain (« a struct a generic writer reaches needs a field writer »). `test_pico8_qb` fige déjà `test_placeholder_mo` : après `pico8_entity_init()` seul, `mv(mo,"md")` rend 0, la caméra ne bouge pas, `ra`/`p`/`ma` sont absents.

### Ce que fait le cart

`pico8/lua.lua:153-155` : `function jl()hv,cv,cg={},{},{} mo,je,ok=qb(),nf(),ui()end`, et `:1178` : `function _init()cu()jl()…`. **Dans le cart, `mo` n'existe pas avant `jl()`** : c'est un global nil, et rien ne le lit ni ne l'écrit avant. Une écriture `mo.ma=3` ou `mo.v=…` sur ce nil serait une erreur Lua (« attempt to index a nil value »). Le `mo` provisoire est un artefact du portage. Il existe pour que `dc("kj")`, `gd` et `ui:lj` aient un objet à lire sur un chemin qui les atteindrait avant `pico8_entity_set_mo`. En pratique, seuls des tests passent par là.

### Emprise réelle du `mo` provisoire (mesurée sur 470accc)

| Aspect | Mesure |
|---|---|
| Construction | 1 site : `pico8_entity_init` (`entity.c:130-136`). Il fait un `malloc`, pose le tag `INSTANCE` et `fields` = `pico8_ob_table_new()`, laisse `cls` = NULL. Pas de `pico8_class_t`, pas de `pico8_class_instantiate`, pas de `lt` : le provisoire n'est **jamais dans `hv`/`cv`/`cg`**, et jamais atteint par `dg`, `he`, la boucle par entité de `dc`, `gw` ou `jz`. `pico8_entity_init` n'est appelé dans `src/` que par `pico8_jl` (`bootstrap.c:30`). |
| Champs | aucun. La table reste vide pendant toute sa vie (voir « Écrivains »). |
| Méthodes | aucune. `mv(mo,"md")` le cherche comme champ (entrée `MD` NULL, puis lecteur) et rend 0. |
| `->defaults` | aucun. |
| Lecteurs | `dc("kj")` → `pico8_mv(g_mo,"md",…)` (`entity.c:316`) ; `gd` → `pico8_entity_geom_field_vec(pico8_mo(),"ra")` (`entity_geom.c:54`) ; `ui:lj` → `pico8_ui_field_vec(pico8_mo(),"ra")` (`ui.c:206`) ; `iv:pq` → `pico8_msgbox_field_vec(pico8_mo(),"ra")` (`msgbox.c:239`). Tous passent par le lecteur à double forme et lisent l'absence comme un vecteur nul ou comme « pas de méthode ». |
| Écrivains | `ga:pq` (`ma`) et `bg:hq` (`v`), par `pico8_entity_set_field`. **Aucun n'atteint le provisoire**, mesuré ainsi. Dans les trois binaires (`main.c`, `main_raylib.c`, `main_class_coverage.c`, avec ou sans `--from-title`), `pico8_cart_load` puis `pico8__init` → `pico8_cu` → `pico8_jl` s'exécutent avant la première frame, et un `ga` ou un clic droit `bg` n'arrive qu'en partie. `pico8_jl` ne construit ni `ga` ni `bg` entre `pico8_entity_init` et `pico8_entity_set_mo` : il construit seulement `qb`/`nf`/`ui`, dont les constructeurs se limitent à `lt`. Côté tests, chacun de ceux qui atteignent `ga:pq` ou `bg:hq` passe d'abord par `pico8__init()` : `test_pico8_geoharvest_burst`, `test_pico8_bg_kj`, `test_pico8_ga_export_panel_by`, `test_pico8_qb` (`test_geoharvest_shakes`, `test_replay_from_title`). `ma()` écrit sur `self` (un `qb` ou un `jo`), jamais sur le provisoire. |
| Vie | De `pico8_entity_init` au `pico8_entity_set_mo` de `pico8_jl`, ou jusqu'au `pico8_entity_shutdown` suivant si `jl()` n'est pas appelé. |
| Libération | `pico8_entity_shutdown` libère `g_mo` par `pico8_instance_free` s'il est tagué `INSTANCE`, donc seulement le provisoire. Ce chemin n'est pris que par un test qui fait `pico8_entity_init` sans `jl()`. **Fuite actuelle** : à chaque `pico8_jl()`, le provisoire alloué par `pico8_entity_init` est remplacé par `pico8_entity_set_mo`, qui ne libère rien, puis le `pico8_entity_shutdown` suivant voit un `qb` et ne libère rien. On perd ainsi un provisoire et sa table à chaque `jl()` (`_init`, chaque `eu()`). |
| Couverture | **aucun nom**. Il est construit à la main, jamais par `pico8_class_instantiate`, donc jamais compté. Absent de `KNOWN[]` et de `tools/class_coverage_net_expected.txt`. |
| `pico8_ref_audit.h` | aucune macro ne le reçoit. |
| `pico8_entity_set_mo` | appelé seulement par `pico8_jl`, avec un `qb`. Aucun test ne range une instance dans le slot. **Après ce commit, `g_mo` vaut donc le provisoire statique, un `qb` struct ou NULL, jamais une `INSTANCE`.** |
| Tests qui l'atteignent | `test_pico8_qb` (`test_placeholder_mo`, figé en 1a.8) ; `test_pico8_je` (`test_lj_on_placeholder` : `pico8_entity_init` puis `ui:lj`, `qa` = `je.ra + mo.ra` figé) ; `test_pico8_probe_mm_rnd_order` et `test_pico8_entity_smoke` (`pico8_entity_init`/`_shutdown` sans `jl()` : construction et libération, aucune lecture de `mo`). Tous les tests qui appellent `pico8_jl()` le construisent puis le remplacent aussitôt. |

### `planet_kj_dispatch` après 1a.8

Il **ne lit ni n'écrit le `mo` provisoire**. Il appelle `pico8_jl()`, qui construit le provisoire dans `pico8_entity_init` et le remplace aussitôt par le `qb`. Il prend `mo = pico8_mo()` après, donc un `qb` struct. Il pose son espion `md` sur le descripteur de ce `qb` et le rétablit avant `pico8_jo_shutdown`/`pico8_bootstrap_shutdown`. Le provisoire n'est donc visible pour lui que comme une allocation perdue dans `jl()`, qui disparaît avec ce commit. **Le test ne change pas** : diff vide, même intention. Le bug qu'il garde est le bloc `mo:md(ci[o])` de `pico8_dc` exécuté après la boucle par entité de la couche. Il est remis à l'épreuve dans ce commit (§6 « vérifié dans ce commit ») : bloc déplacé → rouge ; restauré → vert ; contre-épreuve bloc supprimé → rouge. La variante `_refaudit` compile et passe.

## Solution

1. **Struct du `mo` provisoire, sans parent** : un seul membre, l'en-tête `pico8_struct_header_t`. Pas de `pico8_ke_t` (il n'est pas une entité de `hv`). Pas de `ra`/`p`/`v`/`ma`, que l'instance n'avait pas et dont chaque lecteur tolère l'absence.
2. **Descripteur `static const`** : aucune méthode, en particulier pas `MD` ni `PH`. Le lecteur de champ rend 0 pour tout nom, comme une instance sans classe à table vide. **Écrivain NULL** : aucun écrivain n'atteint le provisoire. Si l'un l'atteignait un jour, l'assert de `pico8_entity_set_field` partirait, ce qui équivaut à l'erreur Lua du cart sur `mo` nil. C'est un arrêt §7, pas un membre improvisé.
3. **Stockage statique** : un unique objet de ce type, défini dans le module entité et initialisé à la compilation (tag `STRUCT`, descripteur). `pico8_entity_init` range son adresse dans `g_mo`, avec un cast vers le type du slot comme pour le `qb`, au lieu du `malloc`. Aucune allocation, aucune libération. La fuite d'un provisoire par `jl()` disparaît du même coup.
4. **`pico8_entity_shutdown`** : la branche « libère `g_mo` s'il est tagué `INSTANCE` » ne peut plus être prise, elle est supprimée. `g_mo` est seulement remis à NULL. Tout ce que ce changement rend inutilisé dans `entity.c` est retiré avec, et rien d'autre.
5. **Identification pour les tests** : accesseur public du descripteur, déclaré dans `entity.h`, par exemple `pico8_mo_placeholder_struct_class()`, calqué sur `pico8_je_placeholder_struct_class()`.
6. **Contrats d'en-tête corrigés**, seulement ceux que ce changement rend faux :
   - `pico8_entity_init()` : il ne « (re)construit » plus le provisoire, il le remet en place.
   - `pico8_entity_shutdown()` : « frees the placeholder mo if pico8_mo() still holds it » devient « `mo` n'est jamais libéré ».
   - `pico8_mo()` : « a classless instance with no fields » devient « struct provisoire sans champ ni méthode ».
   - `pico8_bootstrap_shutdown()` dans `bootstrap.h` : « Frees the `ok` instance, and a placeholder `mo` » devient « `ok` seulement ; `mo` et `je` ne sont jamais libérés ».
   - Le commentaire de construction dans `pico8_entity_init` (« A classless placeholder, not `mo=qb()` … ») devient faux et est remplacé par le contrat C de l'objet : statique, jamais libéré, aucun champ, aucun écrivain.
7. **Rien d'autre** : `pico8_jl`, `pico8_entity_set_mo`, les lecteurs et écrivains de `mo`, `qb`, `planet_kj_dispatch`, `KNOWN[]`, le plan (aucun site découvert attendu).

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que le `mo` provisoire soit la seule chose convertie par ce commit, afin de respecter « une classe par commit » (§7).
2. En tant qu'autopilote, je veux que le PRD mesure l'emprise du `mo` provisoire (construction, champs, méthodes, lecteurs, écrivains, vie, libération, couverture, tests), afin que chaque site touché soit vérifiable.
3. En tant que lecteur du plan, je veux que le `mo` provisoire devienne une struct sans parent (en-tête seul), afin d'appliquer « converties comme des classes à part entière, sans parent » à la lettre.
4. En tant que mainteneur, je veux que la struct n'ait ni `ra`, ni `p`, ni `v`, ni `ma`, afin de ne pas inventer des champs que l'instance n'a jamais eus.
5. En tant que gardien du filet, je veux que toute lecture de champ sur le `mo` provisoire rende « absent », afin que `gd`, `ui:lj` et `iv:pq` lisent `mo.ra` comme le vecteur nul, comme aujourd'hui.
6. En tant que gardien du filet, je veux que le `mo` provisoire n'ait aucune méthode, afin que `dc("kj")` ne pose pas la caméra avant `jl()` et que `mv(mo,"md")` rende 0.
7. En tant que gardien du filet, je veux que le `mo` provisoire ne soit jamais enregistré dans `hv`/`cv`/`cg`, afin que `dg`, `dc` et la boucle de survol ne le voient toujours pas.
8. En tant que mainteneur, je veux que le provisoire n'ait pas d'écrivain, afin qu'une écriture sur `mo` avant `jl()` (une erreur Lua dans le cart) arrête le programme au lieu d'être rangée en silence.
9. En tant que gardien de la mémoire, je veux que le `mo` provisoire ne soit ni alloué ni libéré, afin qu'aucun `pico8_entity_shutdown` ou `pico8_entity_init` répété ne passe un pointeur statique à `free`.
10. En tant que gardien de la mémoire, je veux que `jl()` cesse de perdre un provisoire et sa table à chaque appel, afin que la seule fuite de `mo` qui reste soit le `qb` de TROU-7.
11. En tant que mainteneur, je veux que la libération conditionnelle de `pico8_entity_shutdown` disparaisse, afin de ne pas laisser une branche morte que ce changement a créée.
12. En tant que mainteneur, je veux que `pico8_entity_shutdown` remette toujours `g_mo` à NULL, afin que `pico8_mo()` rende NULL après un arrêt, comme aujourd'hui.
13. En tant que gardien des tests, je veux que chaque `pico8_entity_init` remette le provisoire en place, y compris après un `jl()` où `g_mo` tenait un `qb`, afin que `test_placeholder_mo` et `test_lj_on_placeholder` passent sans changer une valeur figée.
14. En tant que gardien du filet, je veux que le slot `g_mo` et `pico8_mo()` gardent leur type, afin que `pico8_jl`, `pico8_entity_set_mo` et les lecteurs ne soient pas touchés.
15. En tant que gardien de la couverture, je veux que le `mo` provisoire ne soit jamais compté et qu'aucun nom ne soit ajouté à `KNOWN[]`, afin que la sortie de `make class-coverage-net` reste identique à la référence.
16. En tant que mainteneur, je veux un accesseur public du descripteur, afin qu'un test prouve la forme sans lire l'agencement mémoire.
17. En tant que gardien des tests de régression, je veux que `planet_kj_dispatch` passe avec un diff vide, afin que l'étape ne touche pas un test qui ne lit pas le provisoire.
18. En tant que gardien des tests de régression, je veux que `planet_kj_dispatch` échoue encore quand le bloc `mo:md` de `dc` est déplacé après la boucle par entité, puis repasse quand il est restauré, afin de prouver dans ce commit qu'il garde toujours son bug.
19. En tant que gardien des tests de régression, je veux que tous les autres tests de régression passent avec un diff vide, afin que la conversion ne se valide pas en affaiblissant un test.
20. En tant que gardien des tests, je veux que `test_pico8_qb`, `test_pico8_je`, `test_pico8_nf` et `test_pico8_iv` passent sans changer une valeur figée, afin que les caractérisations des étapes précédentes gardent leur objet.
21. En tant que gardien du filet, je veux qu'aucun `.sha256`, aucune entrée de `FRAMES_EVENTS`/`RECORDINGS` ne bouge, afin que l'étape ne se valide pas en déplaçant sa cible.
22. En tant qu'auteur des étapes suivantes, je veux que `qb`, `ga`, `bg`, `jo`, `ui` et l'enum de méthodes ne changent pas, afin que leurs conversions restent des étapes à part.
23. En tant que lecteur du code, je veux que les contrats d'en-tête de `pico8_entity_init()`, `pico8_entity_shutdown()`, `pico8_mo()` et `pico8_bootstrap_shutdown()` disent la vérité après ce commit, afin de ne pas laisser un contrat mensonger (CLAUDE.md).
24. En tant que lecteur du code, je veux qu'aucun commentaire de récit de chantier ne soit ajouté, afin de respecter la règle des quatre destinations.
25. En tant que vérificateur, je veux une caractérisation du provisoire figée sur l'ancien code, afin de prouver que la conversion ne change rien d'observable.

## Implementation Decisions

Cette session n'a pas d'outil d'agent pour lancer un agent `guideline` frais. J'ai donc chargé et appliqué moi-même le skill `guideline` : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Forme retenue : en-tête seul, sans parent, sans champ.** C'est l'équivalent exact de l'instance à table vide pour tous les lecteurs mesurés.
  - Rejeté : un vrai `qb` construit dans `pico8_entity_init`. Il passerait par `lt` dans `hv` (`dg` le ferait défiler), compterait `qb` au crochet (`qb  38` bougerait, FAILED), et `dc("kj")` poserait la caméra avant `jl()`.
  - Rejeté : `mo` = NULL avant `jl()`, bien que plus proche du cart. Les lecteurs le tolèrent (gardes NULL), mais la ligne de l'étape dit « provisoire, remplacée », pas « supprimée ». `test_placeholder_mo` de `test_pico8_qb`, figé en 1a.8, attend un `mo` non NULL à travers `mv` et `pico8_entity_get_field`. Et c'est un changement de comportement hors de la ligne.
- **(G) Écrivain NULL, c'est le point qui diffère de 1a.7.** Contrairement à `je`, des écrivains de `mo` existent : `ga:pq` et `bg:hq`. Mesuré : aucun n'atteint le provisoire, ni dans les binaires ni dans les tests (table d'emprise). Trois options :
  - (a) écrivain NULL, donc assert dans `pico8_entity_set_field` ;
  - (b) écrivain qui accepte et jette, ce qui ment : une écriture puis une lecture ne rendrait pas la valeur, comme elle le fait aujourd'hui ;
  - (c) membres `ma`/`v` pour les recevoir, c'est-à-dire des champs que l'instance n'a jamais eus, sur un chemin qu'aucun code n'emprunte.

  Retenu : (a). Dans le cart, écrire sur `mo` avant `jl()` est une erreur (`mo` est nil). L'assert est donc plus fidèle au cart que le rangement silencieux d'aujourd'hui, et c'est la règle §7 « champ sous aucune règle → arrêt ». Le seul comportement qui change est sur un chemin non atteint. La caractérisation n'en fige donc rien. Seul le test de forme, après conversion, prouve l'assert dans un processus enfant.
- **(G) Lecteur qui rend 0 pour tout nom, sans assert.** Même raisonnement qu'en 1a.7 : le provisoire n'est jamais dans `hv`, ses seuls lecteurs sont nommés ci-dessus (`md`, `ra`) et tolèrent l'absence. Un assert sur les autres noms ne garderait rien. Le lecteur ne peut pas être NULL, parce que `pico8_entity_get_field` l'asserte.
- **(G) Stockage statique plutôt que `malloc`.** L'objet n'a aucun état mutable : tag et descripteur sont fixés à la compilation, sans écrivain. Aucun code ne compare l'identité du provisoire à travers un `pico8_entity_init` : `test_second_jl` de `test_pico8_qb` compare deux `qb`, et `planet_kj_dispatch` compare au `qb` de `jl()`. Rejeté : `malloc` suivi d'une libération « si descripteur provisoire ». Cela garderait une branche et un `free` pour un objet sans état, et laisserait la fuite par `jl()`.
- **Pas de constructeur, pas de couverture.** « Classe à part entière » veut dire ici type struct + descripteur `static const`, et la construction se réduit à l'initialiseur statique. Le crochet de couverture n'est pas appelé : un nom compté hors `KNOWN[]` ferait échouer l'outil (règles 1a.0), et l'ajouter changerait la sortie attendue.
- **`pico8_entity_shutdown`** : après ce commit, `g_mo` ne peut plus tenir d'`INSTANCE` (`pico8_entity_set_mo` n'est appelé qu'avec un `qb`). La branche est donc supprimée. `pico8_instance_free` et le `malloc` du provisoire disparaissent de `entity.c` s'ils n'y servent plus. `pico8_type_tag_of` reste, car les lecteurs à double forme l'utilisent.
- **Type du slot** : `g_mo`/`pico8_mo()`/`pico8_entity_set_mo` restent en `pico8_instance_t *` (décision de 1a.6, suivie en 1a.7 et 1a.8). Ce type disparaît à la sortie de phase.
- **Emplacement** : type et objet privés dans `entity.c`, qui inclut déjà `pico8_struct.h`. L'accesseur est déclaré dans `entity.h`, qui doit alors inclure `pico8_struct.h`. Nom rattaché à `mo`, « placeholder » comme le disent déjà les en-têtes. Il n'y a pas de nom minifié du cart à conserver, puisque le cart n'a pas de valeur pour `mo` avant `jl()`.
- **`planet_kj_dispatch`** : non modifié (voir Problem Statement). La preuve rouge est refaite et rapportée dans le commit. Si le déplacement du bloc `mo:md` ne rend pas le test rouge, c'est un arrêt (§7), pas un test durci.
- **`test_pico8_entity_smoke`** : ses commentaires `:25` (« `dc("kj")` … dispatches "md" on `pico8_mo()`, which none of these tests ever sets up ») restent vrais. Le fichier est inchangé (supprimé en 1a.42).
- **Commentaires** (CLAUDE.md) : le contrat C de l'objet et les corrections d'en-tête listées en Solution §6. Tout le reste va dans le message de commit.

## Implementation Chunks

**Un seul commit** (§7). Chaque morceau est un point d'arrêt TDD qui laisse `make test` et `make frames-check` verts. `make class-coverage-net` est lancé au moins sur le code final.

1. **Caractérisation sur l'ancien code.** Nouveau test (voir Testing Decisions), valeurs figées **avant** toute modification de `entity.c`.
2. **Conversion.** Struct, descripteur, objet statique, `pico8_entity_init`, `pico8_entity_shutdown`, accesseur, contrats d'en-tête ; test de forme ajouté. La caractérisation passe **sans changer ses valeurs figées**. Puis preuve rouge de `planet_kj_dispatch` sur le code final.

## Testing Decisions

- **Un bon test observe le comportement** : ce que rend `pico8_entity_get_field` sur `pico8_mo()`, ce que rend `pico8_mv`, la caméra de `gfx`, ce que rend `pico8_gd`, l'appartenance à `hv`/`cv`, NULL ou non-NULL de `pico8_mo()`. Jamais l'agencement mémoire au-delà du tag et du descripteur.
- **Nouveau test `test_pico8_mo`**, dans `make unit`, lié et lancé comme `test_pico8_je`/`test_pico8_qb` sur le cart réel chargé. Il lit **uniquement** par l'API à double forme (`pico8_mo`, `pico8_entity_get_field`, `pico8_mv`, `pico8_hv`, `pico8_cv`, `pico8_gd`, `pico8_ui_lj`), pour être le même code avant et après.
  - **Caractérisation, figée sur l'ancien code** :
    1. Après `pico8_entity_init()` seul : `pico8_mo()` non NULL ; `pico8_entity_get_field` rend 0 pour `ra`, `p`, `v`, `ma`, `pc`, `me`, `t`, `nu`, `md`, `ph`, `kj` ; `pico8_mv(mo,"md",…)` et `pico8_mv(mo,"ph",…)` rendent 0, et la caméra posée à une valeur témoin ne bouge pas ; `hv` et chacun des cinq `cv` sont vides.
    2. `gd` sur le provisoire : une entité de test (instance à `ra` posé) en (64,64) est visible, une autre en (64+77,64) ne l'est pas. Cela montre que `mo.ra` est lu comme (0,0). Résultats figés.
    3. `ui:lj` sur un `ok` neuf après `pico8_entity_init()`, comme `test_lj_on_placeholder` : `qa` = (0,0), figé.
    4. Cycle : `pico8_entity_shutdown()` → `pico8_mo()` NULL ; `pico8_entity_init()` → lisible comme en 1 ; deux `pico8_entity_init()` de suite → lisible comme en 1, sans crash ; `pico8_entity_shutdown()` deux fois → sans crash.
    5. Remplacement : `pico8_jl()` → `pico8_mo()` rend `ra` = (192,192) (un `qb`) ; `pico8_entity_init()` ensuite → de nouveau le provisoire (lectures de 1) ; `pico8_jl()` puis `pico8_bootstrap_shutdown()` puis `pico8_jl()` → pas de crash, `ra` = (192,192).
  - **Forme**, ajoutée au morceau 2 :
    - après `pico8_entity_init()`, `pico8_mo()` est tagué `STRUCT` avec le descripteur du provisoire (accesseur) ;
    - après `pico8_jl()`, il porte celui de `qb` ;
    - après un nouveau `pico8_entity_init()`, c'est de nouveau le provisoire, **à la même adresse** ;
    - `pico8_entity_set_field(provisoire,"ma",…)` fait partir l'assert, vérifié dans un processus enfant (prior art : le fork de `test_qb_is_a_struct` dans `test_pico8_qb`).
  - **Mutations** : l'implémenteur remet localement au moins 6 erreurs plausibles et note dans le commit celles qui sont attrapées :
    - lecteur qui rend un vecteur non nul pour `ra` ;
    - `MD` posé dans la table du provisoire (la caméra bouge) ;
    - `PH` posé ;
    - `pico8_entity_init` qui ne range pas le provisoire (`g_mo` NULL) ;
    - `pico8_entity_shutdown` qui passe l'objet statique à `pico8_instance_free`/`free` (abandon glibc) ;
    - `pico8_entity_shutdown` qui ne remet pas `g_mo` à NULL ;
    - provisoire enregistré par `lt` dans `hv` ;
    - écrivain qui accepte et jette (attrapé par le test de forme seulement).

    **Mutant équivalent à justifier dans le commit** : `malloc` par `pico8_entity_init` sans libération. Il ne produit qu'une fuite, et seule l'assertion de même adresse du test de forme le voit.
- **`planet_kj_dispatch`** : diff vide. **Preuve rouge sur le code final** : déplacer localement le bloc `mo:md` de `pico8_dc` après la boucle par entité rend `tmp/test_pico8_planet_kj_dispatch` rouge (attendu : `g_order[0] == 'M'` faux) ; le restaurer le rend vert ; la contre-épreuve bloc supprimé le rend rouge (`g_order_n == 2` faux). La variante `_refaudit` compile et passe. Commande et modes d'échec notés dans le commit.
- **`test_pico8_qb`, `test_pico8_je`, `test_pico8_nf`, `test_pico8_iv`, `test_pico8_entity_smoke`, `test_pico8_probe_mm_rnd_order`** : inchangés. `test_placeholder_mo` et `test_lj_on_placeholder` passent avec leurs valeurs figées.
- **Autres tests de régression** : tous passent avec un diff vide.
- **Couverture** : `make class-coverage-net` rend 0 avec une sortie identique à la référence.
- **Prior art** : `test_pico8_je` et 1a.7 (conversion quasi identique), `test_pico8_qb` (`test_placeholder_mo`, fork pour l'assert), 1a.8 pour la preuve rouge de `planet_kj_dispatch`.

## Critères du vérificateur

Le vérificateur de `prd-autopilot` traduit chacun de ces critères en `FAILED:` (§7 ; c'est le seul arrêt que `plan-pilot` honore).

- `FAILED:` si `make test` ou `make frames-check` est encore rouge après **3 tentatives** de correction sur ce commit.
- `FAILED:` si le diff modifie ou supprime un `*.sha256` du filet, ou retire une entrée de `FRAMES_EVENTS` ou de `RECORDINGS`.
- `FAILED:` si `make class-coverage-net`, relancé sur le code final, ne rend pas 0 ou si sa sortie n'est pas identique à `tools/class_coverage_net_expected.txt`.
- `FAILED:` si `tools/class_coverage_net_expected.txt`, `tools/class_coverage_exempt.txt` ou `KNOWN[]` est modifié, ou si le `mo` provisoire appelle le crochet de couverture.
- `FAILED:` si un champ ne tombe sous aucune règle du §5. Cela couvre trois cas : l'assert d'écrivain de `pico8_entity_set_field` part sur le provisoire dans un rejeu ou un test (hors test de forme) ; un lecteur du provisoire demande un nom hors {`ra`, `md`} et en attend une valeur ; une écriture sur le provisoire est trouvée. Dans chacun, on s'arrête, sans membre improvisé.
- `FAILED:` si la struct du `mo` provisoire contient autre chose que l'en-tête (en particulier un `pico8_ke_t`, `ra`, `p`, `v` ou `ma`), si son descripteur a une méthode ou un écrivain.
- `FAILED:` si `planet_kj_dispatch` a un diff non vide qui change son intention, ou si le commit ne rapporte pas sa preuve rouge (bloc `mo:md` déplacé après la boucle, test rouge, restauré, vert).
- `FAILED:` si un autre test de régression de bug (§6) a un diff non vide, ou devrait changer d'intention pour passer.
- `FAILED:` si une autre classe est convertie ou change de forme (en particulier `qb`, `ga`, `bg`, `jo`, `ui`/`ok`, `nf`, le `je` provisoire, `ke`), si l'enum de méthodes change, ou si un `->defaults` bouge.
- `FAILED:` si les valeurs figées de la caractérisation du `mo` provisoire, de `test_pico8_qb`, `test_pico8_je`, `test_pico8_nf` ou `test_pico8_iv` ont été changées après la conversion.

Critères de réussite (en plus) :

- grep : plus aucun `malloc` de `g_mo`, plus aucun `PICO8_TYPE_TAG_INSTANCE` ni `pico8_ob_table_new` dans `pico8_entity_init`, plus aucun `pico8_instance_free(g_mo)` dans `src/pico8_entity.c` ;
- l'enum de méthodes vaut exactement `OL`, `PH`, `KJ`, `EV`, `LDOWN`, `MD` ;
- dans `src/`, le diff se limite à `entity.c` (type, objet et descripteur du provisoire, `pico8_entity_init`, `pico8_entity_shutdown`, accesseur), à `entity.h` (accesseur, include, contrats) et au commentaire de `bootstrap.h` ;
- les contrats d'en-tête de `pico8_entity_init()`, `pico8_entity_shutdown()`, `pico8_mo()` et `pico8_bootstrap_shutdown()` ne mentionnent plus de `mo` libéré ni d'instance sans classe.

## Out of Scope

- `qb` (1a.8, fait), `ga` (1a.26), `bg` (1a.12), `jo` (1a.17) et leurs lectures ou écritures de `mo`.
- Le type du slot `g_mo`/`pico8_mo()`/`pico8_entity_set_mo` (`pico8_instance_t *`), qui disparaît à la sortie de phase.
- Ajouter une garde, ou un défaut, pour une écriture sur `mo` avant `jl()` : aucun chemin ne l'atteint.
- Libérer les `qb` remplacés par `jl()` (TROU-7, Plan 2 §8).
- Réécrire `planet_kj_dispatch` ou les commentaires de `test_pico8_entity_smoke`.
- Lancer l'appli raylib : jamais.

## Further Notes

- Le filet ne voit jamais le `mo` provisoire : `jl()` le remplace dans `pico8__init`, avant la première frame de tout rejeu, avec ou sans `--from-title`. Toute la garde de l'étape repose donc sur la caractérisation, `test_placeholder_mo` et `test_lj_on_placeholder`. La couverture est inchangée par construction.
- Effet de bord voulu : la fuite d'un provisoire (struct + table `ob`) à chaque `jl()` disparaît. C'est observable seulement par un outil mémoire, pas par un test de comportement.
- Mesures faites par grep sur `src/` et `tests/` à 470accc, en lisant `pico8/lua.lua:153-155` (`jl`) et `:1178` (`_init`), `pico8_entity.c` (`pico8_entity_init`, `pico8_entity_shutdown`, `pico8_entity_get_field`/`_set_field`, `pico8_mv`, `pico8_dc`, `pico8_entity_set_mo`), `pico8_entity_geom.c`, `pico8_planet_family.c` (`pico8_ga_pq`), `pico8_bg.c` (`pico8_bg_hq`), `pico8_bootstrap.c`/`.h`, et les tests `test_pico8_planet_kj_dispatch`, `test_pico8_qb`, `test_pico8_je`, `test_pico8_probe_mm_rnd_order`, `test_pico8_entity_smoke`, `test_pico8_geoharvest_burst`, `test_pico8_bg_kj`, `test_pico8_ga_export_panel_by`.
