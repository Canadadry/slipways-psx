---
title: "Plan 3, étape 1a.30 : l'enveloppe de `pz()` sur `pj.cr`, instance sans classe, devient la struct statique qui porte `hi`, `gq`, `ca`, `ep` et remplace la table `cr` dans l'état partagé de `pj`"
description: "`pico8_cr` passe à `pz` une enveloppe sur la pile (tag `INSTANCE`, `cls` NULL, `fields` = la table `cr` partagée de `pj`). Comme `mz.jj` en 1a.11, la table et l'enveloppe fusionnent : un seul objet statique à en-tête, sans parent, avec quatre membres, un lecteur et aucun écrivain. C'est lui que rendent `pico8_pj_cr()` et le lecteur de `pj` pour `cr`, et `cr()` y écrit `ep` directement."
status: needs-triage
---

## Problem Statement

Ligne de l'étape, recopiée de `docs/plan-3.md` (§5, « Étapes de la phase 1a ») :

> 1a.30 — enveloppe de `pz()` sur `pj.cr` : sans classe (`menu.c:243`).

Le numéro de ligne a bougé. L'enveloppe est maintenant à `menu.c:202,231-234`, dans `pico8_cr` (`menu.c:137`).

Règles du §5 qui s'appliquent :

> **Instances sans classe** : converties comme des classes à part entière, sans parent.
>
> **Défauts mutés sur la classe** : […] deviennent un état statique partagé de la classe, jamais un champ d'instance. Même traitement pour tout autre défaut […] **dont la table pointée est modifiée en place** (connus : `pj.ou`, `pj.ea`, `pj.cr`, …).
>
> **Coexistence** : une instance convertie porte un nouveau tag (`PICO8_TYPE_TAG_STRUCT`) et un pointeur vers sa table de méthodes, dans le même premier emplacement qu'aujourd'hui.

§6 : l'étape ne nomme aucun test de régression. Tous passent avec un diff vide. §7 : `make class-coverage-net` rend une sortie identique à `tools/class_coverage_net_expected.txt`. L'enveloppe n'a pas de nom dans `KNOWN[]` et n'enregistre rien au crochet. La preuve des tests passe par `gcov` et 2 ou 3 mutations. La note « Découvert à 1a.30 » suit la forme courte (§7, à partir de 1a.27).

### Ce que le cart fait

`pico8/lua.lua`, `cr()` (`:955-964`) : `pj.cr.ep="level "..tl.." (…)" ; pz(pj.cr,bs)`. `pj` déclare `cr=o(hi=v(0,11),gq=v(0,11),ca=10)`. `pz(p,bs)` (`:280-288`) lit `p.ep`, `p.eo`, `p.hi` (sinon `p.ra`), `p.ca`, `p.gq`, `p.ff`, et n'écrit jamais sur `p`.

En Lua, **l'enveloppe est `pj.cr` elle-même.** Le C a besoin d'une enveloppe parce que `pz` prend une instance et que `cr` est une table.

### État hérité de 1a.29 (2092090)

- `cr` est un défaut modifié en place : il n'a pas de membre, il vit dans l'état statique de `pj` (`g_pj_defaults[PJ_CR]`, `score_panel.c:99-112`). C'est la même REF que `g_pj_class->defaults`, prise à l'init et remise à NULL au shutdown.
- `pico8_pj_cr()` (`score_panel.c:134`) rend cette table (`pico8_ob_table_t *`), ou NULL hors de init..shutdown.
- Le lecteur de `pj` (`score_panel.c:353-372`) rend `cr` depuis l'état statique.
- `pico8_cr` lit la table par l'accesseur, y écrit `ep` avec `pico8_ob_table_set_name`, puis construit sur la pile un `pico8_instance_t` (`tag INSTANCE`, `fields` = table, `cls` NULL) et appelle `pz`.
- Le `ep` est une chaîne allouée à chaque appel, qui n'est jamais libérée (Plan 2 §8). La nouvelle chaîne remplace l'ancienne dans la table partagée : un `pj` suivant, et même une partie suivante, voient le dernier `ep`.
- `pz` est à double forme (lecture par `pico8_entity_get_field`, `pico8_msgbox_field_vec`). Il reçoit déjà une struct : le `fv` de 1a.28, par `help.c:79`.

### Emprise mesurée

| Aspect | Mesure |
|---|---|
| Construction de l'enveloppe | 1 site : `pico8_cr`, sur la pile, à chaque appel. Jamais dans `hv`/`cv`/`cg`. |
| Champs lus sur l'enveloppe (par `pz`) | `ep`, `eo`, `hi`, `ra` (seulement si `hi` est absent), `ca`, `gq`, `ff`. Présents : `hi`, `gq`, `ca`, `ep` (après `cr()`). Absents : `eo` (d'où `rn(62)`), `ra`, `ff` (donc `ok.le` n'est pas vidé). |
| Écrivains qui l'atteignent | aucun. `pz` et `pz_step` n'écrivent que sur les lignes `b` de `bs` et sur `ok`. Le seul écrivain de `cr` est `ep`, dans `pico8_cr` lui-même. |
| Méthodes | aucune. |
| Lecteurs de la table `cr` hors `pz` | `pico8_pj_cr()` (seul appelant `src/` : `menu.c:191`). Le lecteur de `pj` (`cr` lu sur un panneau). `test_pico8_pj` (`:57` tables, `:390-414` `test_life`, `:538` identité avec la classe). |
| `->defaults` | lu une fois à l'init de `pj` (1a.29). Aucun autre site. |
| Couverture | aucun nom, ligne `pj` inchangée. |
| Tests qui traversent `pz` sur `cr` | `test_pico8_js` `test_research` (`RESEARCH_HASH` : `fa`=`ca`, `ra`=`hi`, `qf` avancé de `gq`, ligne d'en-tête `ep`, nombre de lignes) et `:1032` (9 boutons struct). `test_pico8_pj` `test_life`. `test_pico8_tech_screen` (régression). Le rejeu `tech_screen_invent` du filet. |
| Clé dynamique | aucune. |

## Solution

1. **Struct de l'enveloppe**, publique dans le module du panneau de score, à côté de `pico8_pj_t`. Elle contient un `pico8_struct_header_t`, puis quatre membres `pico8_value_t` au nom minifié conservé : `hi`, `gq`, `ca`, `ep`. Elle n'a pas de `pico8_ke_t` : c'est une instance sans classe, sans parent, jamais une entité.
2. **Un seul objet statique, initialisé à la compilation** (modèle `mz.jj`, 1a.11) :
   - tag `STRUCT` et descripteur de l'enveloppe ;
   - `hi` et `gq` sont des REF vers deux vecteurs statiques `v(0,11)`. Il en faut deux distincts, comme les deux `v()` de la spec ;
   - `ca` = 10 ;
   - `ep` = nil.

   Cet objet **est** l'état partagé `pj.cr` : il remplace la table `cr` dans l'état statique de `pj`. Rien ne l'alloue et rien ne le libère.
3. **Descripteur `static const`** : aucune méthode, et un écrivain NULL, parce qu'aucun écrivain générique ne l'atteint. Le lecteur rend `hi`, `gq`, `ca`, et `ep` s'il n'est pas nil. Pour tout autre nom (`eo`, `ra`, `ff`, `fo`, `nu`…), il rend 0. Un accesseur public du descripteur sert aux tests.
4. **État de `pj`** : l'entrée `cr` de l'état statique désigne l'objet de l'enveloppe au lieu de la table de la classe.
   - À l'init, l'assert « chaque défaut est une clé de la spec » reste vrai pour `cr`, et la spec de la classe `pj` **ne change pas**. Sa table `cr` devient une copie en lecture que plus rien ne lit, comme la spec de `mz` en 1a.11. L'assert « table ob » ne vaut plus pour `cr`.
   - `ep` est remis à nil à l'init. Aujourd'hui, l'init re-parse la classe, donc `ep` est absent après une init. Il n'est **pas** remis à nil à la construction d'un `pj` : le partage entre parties de 1a.29 est gardé.
   - Au shutdown, l'entrée revient à NULL comme les autres.
5. **`pico8_pj_cr()`** rend un pointeur vers la struct de l'enveloppe, et plus une `pico8_ob_table_t *`. Le contrat reste le même : NULL hors de init..shutdown, objet partagé par tous les `pj` et modifié en place par `cr()`. Le lecteur de `pj` rend pour `cr` la REF de cet objet.
6. **`pico8_cr`** écrit `ep` directement dans le membre de l'objet, puis passe l'objet à `pz` avec un cast vers `const pico8_instance_t *`, comme `help.c:79`. Le `pico8_instance_t wrapper` sur la pile disparaît. La garde « accesseur non NULL » ne change pas.
7. **`pz` ne change pas.** Il reste à double forme jusqu'à 1a.41 (enveloppe `g_ha`, `menu.c:376-404`, qui reste une instance sans classe sur la pile). Son contrat d'en-tête (« `p` may be a classless field-bag … ») reste vrai pour `g_ha` et ne change pas.
8. **`test_pico8_pj`** : les lectures de la table `cr` passent à l'API à double forme sur l'objet (`ep` dans `test_life`). L'identité « `cr` == REF de la classe » devient « `cr` lu sur `ju` == `pico8_pj_cr()` == l'objet de l'enveloppe ». `cr` quitte la liste `tables[]` s'il y est traité comme une table ob. Aucune constante `*_HASH` ne bouge.
9. **Plan** : ajouter à la ligne 1a.30 la note « Découvert à 1a.30 : `pj.cr` et son enveloppe fusionnent en une struct statique, rendue par `pico8_pj_cr()` ; `pz` reste à double forme pour 1a.41 ; enum inchangée. » Si un site est découvert en cours d'étape et qu'il change une étape suivante, il s'ajoute à cette note.

## User Stories

1. En tant qu'autopilote de la phase 1a, je veux que l'enveloppe de `pz` sur `pj.cr` soit la seule chose convertie par ce commit, afin de respecter « une classe par commit ».
2. En tant que lecteur du code, je veux que l'enveloppe soit une struct à en-tête avec les membres `hi`, `gq`, `ca`, `ep`, afin que la traçabilité vers `cr=o(hi=v(0,11),gq=v(0,11),ca=10)` et `pj.cr.ep` tienne.
3. En tant que lecteur du code, je veux que la struct n'ait pas de `pico8_ke_t`, afin que « sans parent » se lise dans le type.
4. En tant que mainteneur, je veux que la table `cr` et l'enveloppe soient un seul objet, afin qu'il n'existe qu'une représentation de `pj.cr`, comme en Lua où `pz` reçoit `pj.cr` elle-même.
5. En tant que mainteneur, je veux que cet objet soit l'état statique partagé de `pj` pour `cr`, et jamais un membre, afin que la règle des défauts modifiés en place posée à 1a.29 tienne.
6. En tant que joueur qui relance une partie, je veux que le `ep` écrit par la partie précédente reste visible jusqu'au prochain `cr()`, afin que le partage entre parties ne change pas.
7. En tant que gardien des tests, je veux que `ep` soit absent juste après l'init, afin que les cycles init/shutdown se comportent comme quand la classe était re-parsée.
8. En tant que gardien du filet, je veux que l'écran technologie ait la même ligne d'en-tête, les mêmes positions (`hi`, puis `gq` par rang), le même `fa`=`ca`=10 et le même nombre de boutons, afin que `tech_screen_invent` et `RESEARCH_HASH` ne bougent pas.
9. En tant que gardien du filet, je veux que `eo`, `ra` et `ff` restent absents de l'enveloppe, afin que `pz` joue encore `rn(62)`, parte de `hi` et ne vide pas `ok.le`.
10. En tant que mainteneur, je veux que l'enveloppe n'ait pas d'écrivain, afin qu'une écriture jamais mesurée arrête l'exécution.
11. En tant que mainteneur, je veux que `pico8_cr` écrive `ep` dans le membre directement, afin qu'il n'y ait plus d'aller-retour par une table.
12. En tant que mainteneur, je veux que `pico8_pj_cr()` garde son contrat (NULL hors de init..shutdown, objet partagé et modifié en place), avec un type de retour qui le dit, afin que `pico8_cr` garde sa garde.
13. En tant qu'auteur de 1a.41, je veux que `pz` et l'enveloppe `g_ha` restent tels quels, afin que ma conversion reste une étape à part.
14. En tant que gardien de la couverture, je veux qu'aucun nom ne s'ajoute à `KNOWN[]`, afin que `make class-coverage-net` rende une sortie identique.
15. En tant que gardien de la mémoire, je veux que l'objet ne soit ni alloué ni libéré, et que les `ep` gardent leur fuite actuelle (Plan 2 §8), afin que ASan reste propre sans changer le comportement.
16. En tant que vérificateur, je veux une caractérisation figée sur l'ancien code de ce que `pz` produit sur `cr`, puis une vérification de forme ajoutée après la conversion, afin de prouver l'équivalence.
17. En tant que vérificateur, je veux un rapport `gcov` et 2 ou 3 mutations ciblées, afin de voir que chaque ligne ajoutée est exécutée et que les valeurs propres sont gardées.
18. En tant que lecteur du plan, je veux la note « Découvert à 1a.30 » sous forme courte, afin de savoir ce qui change pour 1a.41.
19. En tant que lecteur du code, je veux que les commentaires rendus faux soient corrigés et que les ajouts ne portent qu'un contrat C ou un piège (CLAUDE.md), afin de ne laisser aucun contrat mensonger.

## Implementation Decisions

Pas d'humain dans la boucle. J'ai appliqué le skill `guideline` moi-même : hypothèses explicites, simplicité, changement chirurgical, critères vérifiables. Les décisions tranchées ainsi sont marquées (G).

- **Modules touchés** : le panneau de score (struct, objet, descripteur, état de `pj`, accesseur, lecteur), le menu (`pico8_cr`), `test_pico8_pj`, et la ligne 1a.30 du plan. Rien d'autre dans `src/`. `msgbox` (`pz`), `help`, `ui`, `entity`, `pico8_struct.h` (enum), `cart` et `bootstrap` ne changent pas.
- **(G) Fusion de la table `cr` et de l'enveloppe**, modèle `mz.jj` (1a.11), parce que c'est la lecture littérale du cart. Deux solutions sont rejetées :
  - une enveloppe struct recopiée depuis la table à chaque appel : elle crée deux représentations, et `ep` devrait être écrit deux fois ;
  - une struct « mandataire » qui tiendrait une REF vers la table : ce ne serait pas une classe à part entière avec un membre par champ.
- **(G) Valeurs `hi`/`gq`/`ca` constantes à la compilation**, sans lecture dans `g_pj_class->defaults`. La règle « REF prise dans `defaults` » ne vise que les classes qui ont des descendants non convertis, et `pj` n'en a aucun. La spec de la classe reste en lecture, jamais lue pour `cr` (précédent `mz`).
- **(G) Garder NULL hors de init..shutdown** pour `pico8_pj_cr()`. On le fait en posant la REF de l'objet dans l'état de `pj` à l'init. `mz.jj` avait choisi « jamais NULL », mais ici `pico8_cr` teste cette nullité, et les cinq accesseurs de 1a.29 partagent ce contrat. C'est le changement le plus petit.
- **(G) `ep` remis à nil à l'init seulement.** C'est le comportement observable actuel : la classe est re-parsée à l'init, et la table est partagée entre les `pj`.
- **Règle du §5** : « Instances sans classe » donne la forme. « Défauts … dont la table pointée est modifiée en place » donne l'objet unique, dans l'état statique. `pj.cr` est déjà dans la liste connue, donc le §5 n'a rien à ajouter.
- **Aucun écrivain** (modèle `mo`/`mz.jj`). Si l'implémenteur trouvait une écriture générique sur l'enveloppe (l'assert de `pico8_entity_set_field` partirait), le nom écrit serait un champ de ce défaut partagé. On ajouterait alors un membre dans l'objet statique et un écrivain qui n'accepte que ce nom, et on justifierait l'écart dans le commit. Ce n'est pas un arrêt.
- **Commentaires** : le commentaire de l'état de `pj` (« `ea`, `cr`, `ou` have no member ») et le contrat des accesseurs sont corrigés si le changement les rend faux. Le nouvel objet porte au plus un contrat C : partagé, modifié en place par `cr()`, `ep` remis à nil à l'init.

## Testing Decisions

Un bon test lit ce que `pz` produit (les lignes de `ok.le`, leur `fa`/`ra`/`qf`/`lq`) et ce que l'API à double forme rend. Il ne regarde pas la représentation. Prior art : `test_pico8_mz` à 1a.11 (forme de l'enveloppe, identité enveloppe/membre), `test_pico8_js` `test_research`, `test_pico8_pj` à 1a.29.

1. **Caractérisation sur l'ancien code, avant de toucher `src/`**, dans `test_pico8_pj`. Elle ne passe que par `pico8_cr()` et par ce qu'il produit :
   - après `cr()`, `ok.le` commence par la ligne d'en-tête (`lq` = `{ep, c=13}`, texte figé) ;
   - toutes les lignes ont `fa` = 10 et partagent une même `ra` = `v(0,11)`. Le `qf` de la ligne *i* vaut `v(0,11)` + *i*×`v(0,11)`. `ok.le` n'est pas vidé ;
   - un second `cr()` rend le même en-tête ;
   - `cr` lu sur deux `pj` successifs désigne le même objet, et `ep` y est présent après `cr()`.

   Les valeurs sont figées dans un hash. Mesurer la durée : au-delà de 30 s, le test va dans `UNIT_TESTS_SLOW`, sinon il reste dans `UNIT_TESTS`.
2. **Après la conversion**, la même caractérisation passe à l'identique. On y ajoute un test de forme :
   - tag `STRUCT` et descripteur de l'enveloppe ;
   - aucune méthode, écrivain NULL ;
   - lecteur : `hi`/`gq`/`ca` présents, `ep` absent juste après l'init, `eo`/`ra`/`ff`/`fo` absents ;
   - `pico8_pj_cr()` == `cr` lu sur un `pj` ;
   - une écriture par `pico8_entity_set_field` fait avorter (`must_abort`) ;
   - `pico8_pj_cr()` est NULL après le shutdown.
3. **Tests existants** : `test_pico8_js` (`RESEARCH_HASH`, 9 boutons struct), `test_pico8_tech_screen` et tous les tests de régression passent avec un diff vide. Dans `test_pico8_pj`, seules les lectures de `cr` décrites au point 8 de la Solution changent.
4. **Preuve (§7)** :
   - `gcov` dans une copie jetable du scratchpad, supprimée après usage. Compiler en `--coverage -O0` `test_pico8_pj` et `test_pico8_js`, puis lancer `gcov` sur le panneau de score et sur le menu. Chaque ligne ajoutée ou modifiée doit être exécutée, et le rapport va dans le message de commit ;
   - 2 ou 3 mutations, chacune rouge sur la caractérisation : `ca` = 11 ; `gq` = `v(0,12)` (ou `hi` et `gq` sur un même vecteur muté) ; le lecteur qui rend `eo` présent, ou `ep` qui n'est pas écrit dans le membre. Le vérificateur relit le diff et le rapport, sans banc de mutations propre.
5. `make test`, `make frames-check` (aucun `.sha256` modifié) et `make class-coverage-net` (sortie identique).

Critères que le vérificateur traduit en `FAILED:` :

- l'enveloppe a un parent, un nom de couverture, une méthode ou un écrivain non justifié ;
- elle est allouée par appel ou par `pj` ;
- `cr` reçoit un membre dans `pico8_pj_t` ;
- deux `pj` ne voient pas le même objet ;
- `pz`, l'enveloppe `g_ha`, `help.c` ou un autre module de `src/` que le panneau de score et le menu change ;
- un test de régression change ;
- un `.sha256`, `FRAMES_EVENTS` ou la sortie de `class-coverage-net` bouge ;
- la note « Découvert à 1a.30 » manque ;
- le rapport `gcov` ou les mutations manquent.

## Out of Scope

- L'enveloppe `g_ha` de `pz` (1a.41), et le passage de `pz` à la forme struct seule.
- `ui` (1a.31), et les racines et instances sans classe suivantes.
- La fuite des chaînes `ep` et des vecteurs de `pz` (Plan 2 §8, TROU-7).
- La suppression de la `pico8_class_t` de `pj` et de sa spec (1a.42).

## Further Notes

- Précédents directs : 1a.11 (`mz.jj`, fusion table/enveloppe, objet statique) et 1a.29 (état partagé de `pj`, accesseurs).
- L'étape est petite : deux fichiers de `src/`, un test, une ligne du plan.
