---
title: "Plan 2, Phase E1 : sortir from_double/to_double du cœur, retirer le build d'observation"
description: "Tant que pico8_fixed.h déclare pico8_fixed_from_double/to_double et que pico8_fixed.c les définit, le cœur contient du flottant que la garde voit (pico8_fixed.c en échec) et un angle mort qu'elle ne voit pas (un nouvel appel from_double(<littéral>) compilerait sans erreur sous -mgeneral-regs-only)."
status: needs-triage
---

## Problem Statement

Texte de l'étape, recopié de `docs/plan-2.md` (§7, Phase E) :

> ### Phase E — Fermeture
> Seule porte de E : les quatre points du §6. Décisions humaines prises en fin de Phase D :
> - **§6.1 porte sur le cœur seul**, conformément au §4 : `CORE_SRCS` et les en-têtes qu'ils incluent par défaut (sans les définitions `PICO8_FLOAT_*`). Le reste de `src/` (backend raylib, visualiseur, builds d'audit) est exclu par une liste versionnée, une ligne de justification par fichier.
> - **§6.4 se lit « toute divergence justifiée par une ligne du relevé »** : C2b (bloc A) et D1 (bloc B) sont admis.
> - **Le build d'observation est retiré** (`make float-observe`, `src/pico8_float_observe.*`) : plus aucun cast à observer, l'audit avant/après le remplace.
> - **TROU-1 est restreint, pas fermé** : il ne décrit plus que `sin`/`cos` hors des 64 points du relevé.
>
> Étapes :
> - **E1 — Sortir `from_double`/`to_double` du cœur.** Déclarations **et** définitions dans un module d'aides de test/hôte, hors `ENGINE_SRCS`/`CORE_SRCS`, lié par les tests et l'audit. Sans prototype dans l'en-tête du cœur, tout nouvel appel flottant dans le cœur devient une erreur de compilation (`-Werror`, déclaration implicite) : c'est ce qui referme l'angle mort x86_64 de la garde. Retrait du build d'observation. Neutre : `frames-check` identique sans `frames-baseline`.

**Où en est-on (« État de E0 »).** Sur l'arbre d'E0, aucune ligne de `src/` n'a changé depuis D4 ; les quatre points du §6 ont chacun une cible :

| § | Cible | Code | Résultat sur l'arbre d'E0 |
|---|---|---|---|
| 6.1 | `make float-grep` | 1 | `float-grep coeur 119 exclus 30 lignes 13` : 13 lignes dans 8 fichiers |
| 6.2 | `make float-link` | 0 | `float-link objets 62 non-resolus 0` |
| 6.3 | `make float-guard` | 1 | `garde fichiers 58 en-echec 1` : `src/pico8_fixed.c` |
| 6.4 | `make frames-ledger` | 0 | `frames-ledger commits 6 poses 4 divergences 2 erreurs 0` |

Les 13 lignes du 6.1 : `pico8_fixed.c` 9, 10, 15, 16 (définitions de `from_double`/`to_double`) et 128 (« no host float ») ; `pico8_fixed.h` 29, 32 (prototypes) et 31 (« representable as a double ») ; `pico8_galaxy.c` 133, `pico8_ob.c` 347 ; `pico8_geom.h` 9, `pico8_research.h` 21, `pico8_type_tag.h` 15. L'inventaire compte 10 jetons « autre » : les 6 des définitions de `pico8_fixed.c`, et 4 mots de commentaire (`pico8_fixed.c` 128, `pico8_galaxy.c` 133, `pico8_ob.c` 201 et 347). E0 note que le compte « les 3 d'en-têtes » d'E2 ne tient **que si le commentaire de `pico8_fixed.h` 31 sort avec les prototypes en E1**, et que seul E1 ferme l'angle mort de la mention `pico8_fixed_from_double` de `pico8_ob.c` 201 (invisible au grep, le `_` étant un caractère de mot).

Le problème, du point de vue de celui qui ferme le chantier :

1. **Le seul fichier du cœur encore en échec sous la garde est `pico8_fixed.c`**, et il n'y est que pour deux fonctions qu'aucun fichier du cœur n'appelle plus : depuis C2b les littéraux passent par `PICO8_FIXED_LIT`, depuis D1–D5 plus aucun cast ni calcul `double` ne passe par `to_double`. Mesuré en préparant ce PRD : dans `src/`, les seuls appels réels sont dans `pico8_float_audit.c` et `pico8_float_observe.c` ; dans le cœur, il ne reste que la mention en commentaire de `pico8_ob.c` 201.
2. **L'angle mort x86_64 de la garde est ouvert tant que le prototype est dans l'en-tête du cœur.** Un `pico8_fixed_from_double(0.3)` réintroduit demain dans n'importe quel fichier du cœur compilerait sous `-mgeneral-regs-only` (argument `double` passé sur la pile sans erreur, « État de B2 »), et seul l'inventaire le verrait, après coup.
3. **Le build d'observation est mort**, et il pèse : depuis D1, `make float-observe` rend le code 2 (« aucun cast »), `tools/float_observe.txt` est une photographie figée de D0b, et le build garde un arbre d'objets, une règle de motif, un `-include`, un test d'outil dans `make unit`, deux fichiers dans `src/` exclus du 6.1, et une inclusion conditionnelle à la fin de `pico8_fixed.h`.

## Solution

Un **module d'aides de test** nommé comme tel (`pico8_fixed_test_double`, en-tête et source) reçoit **les deux déclarations et les deux définitions**, à l'identique, noms de fonctions inchangés. Il n'est dans ni `ENGINE_SRCS` ni `CORE_SRCS`, aucun fichier du cœur ne l'inclut, et `tools/float_grep_exclusions.txt` l'exclut du 6.1 avec une ligne de justification. Il est lié par ceux qui en ont besoin, et seulement eux : les huit tests qui appellent ces fonctions, le build d'audit, et le programme engendré par le comparateur de constantes.

`pico8_fixed.h` perd les deux prototypes et leurs commentaires (dont « representable as a double »), et l'inclusion conditionnelle de `pico8_float_observe.h` ; `pico8_fixed.c` perd les deux définitions. À partir de là, un appel flottant de ces fonctions dans le cœur est une **erreur de compilation** (`-std=c99 -Wall -Werror` : déclaration implicite), ce qui ferme l'angle mort de la garde.

Le build d'observation est **retiré en entier** : `src/pico8_float_observe.*`, son arbre d'objets, sa règle, sa cible, son `-include`, sa ligne de légende, `tools/float_observe.sh`, `tools/float_observe.txt`, `tests/test_float_observe.c` et son entrée dans `make unit`, et ses deux lignes d'exclusion.

L'étape est **neutre** : `make frames-check` identique sur les 12 fichiers de `FRAMES_EVENTS` sans `frames-baseline`, `make float-audit` vert, `make test` vert. Les rapports versionnés qui dépendent de l'arbre sont régénérés (inventaire, garde, couverture), et l'« État de E1 » est écrit dans `docs/plan-2.md`.

Résultat attendu, à mesurer et consigner (pas à supposer) :

- **6.3** `make float-guard` → `garde fichiers 58 en-echec 0`, code 0, `tools/float_guard.txt` sans aucune ligne de fichier.
- **Inventaire** → `compte autre 4`, `compte total 4` : les 4 mots de commentaire d'E2 (`pico8_fixed.c` « float », `pico8_galaxy.c`, `pico8_ob.c` ×2), les 6 jetons des définitions ont quitté le cœur.
- **6.1** `make float-grep` → toujours en échec (code 1), `coeur 119 exclus 30 lignes 6`, dans 6 fichiers : `pico8_fixed.c` (« no host float », numéro de ligne décalé), `pico8_galaxy.c` 133, `pico8_ob.c` 347, `pico8_geom.h` 9, `pico8_research.h` 21, `pico8_type_tag.h` 15 — plus aucune ligne de `pico8_fixed.h`. Exclusions : 30 − 2 (observation) + 2 (aide de test) = 30. C'est exactement le reliquat d'E2.
- **6.2** et **6.4** inchangés (`float-link` 0, `frames-ledger` 0 : aucun `*.frames.hash` touché).

## User Stories

1. En tant que mainteneur du cœur, je veux que `pico8_fixed.h` ne déclare plus `pico8_fixed_from_double` ni `pico8_fixed_to_double`, afin qu'un appel flottant réintroduit dans le cœur soit une erreur de compilation et non une ligne découverte plus tard dans l'inventaire.
2. En tant que mainteneur du cœur, je veux que `pico8_fixed.c` ne définisse plus ces deux fonctions, afin que `make float-guard` passe à 0 fichier en échec.
3. En tant qu'agent qui ferme la Phase E, je veux que ces deux fonctions vivent dans un module dont le nom dit « aide de test », afin que l'exception du §6.1 (« helpers de test explicitement isolés et nommés comme tels ») s'applique sans discussion.
4. En tant qu'agent, je veux que le module d'aide garde les noms et le comportement exacts des deux fonctions (arrondi au plus proche pour `from_double`, division exacte pour `to_double`), afin que les tests, l'audit et les outils qui les appellent ou qui lisent ces noms dans du texte fabriqué ne changent pas de sens.
5. En tant qu'auteur de tests, je veux inclure l'en-tête d'aide dans les huit tests qui appellent ces fonctions (`test_pico8_fixed`, `test_pico8_vec`, `test_pico8_geom`, `test_pico8_slipway_drag`, `test_pico8_playability`, `test_pico8_empty_planet_cleared`, `test_pico8_geoharvest_burst`, `test_pico8_tech_screen`), afin qu'ils compilent sans prototype dans l'en-tête du cœur.
6. En tant qu'auteur de tests, je veux que la source d'aide soit liée par ces huit binaires, et par la variante UBSan de `test_pico8_fixed`, afin que leurs liens passent.
7. En tant qu'agent, je veux que les variantes `-DPICO8_REF_AUDIT` de `test_pico8_playability`, `test_pico8_slipway_drag` et `test_pico8_empty_planet_cleared` lient aussi l'aide, afin que `make ref-audit`, hors suite, ne casse pas en silence.
8. En tant qu'agent, je veux que le build d'audit (`pico8_float_audit.c`, qui recalcule les anciennes expressions en `double`) inclue l'en-tête d'aide et que `pico8_replay_audit` lie la source d'aide, afin que `make float-audit` reste vert dans `make test`.
9. En tant qu'agent, je veux que `pico8_replay` (la cible de lien du cœur, §6.2) **ne lie pas** l'aide, afin que ni `from_double` ni `to_double` n'apparaissent dans ses symboles.
10. En tant qu'agent, je veux que le programme engendré par `tools/constants_compare.sh` inclue l'en-tête d'aide et lie la source d'aide trouvée à côté du `pico8_fixed.c` qu'il reçoit, afin que ses appels `pico8_fixed_from_double(<littéral>)` issus d'un inventaire compilent encore.
11. En tant qu'agent, je veux que `test_constants_check` passe sans changer, puisqu'il donne `src/pico8_fixed.c` et que l'aide est sa voisine.
12. En tant qu'agent, je veux que `test_constant_variants` copie aussi la source d'aide dans sa racine fabriquée, l'ajoute aux sources de son pilote, et que ce pilote inclue l'en-tête d'aide, afin que la variante compile et que le test continue de prouver ce qu'il prouve (la frame 3 ne change que si la constante vaut sa valeur tronquée).
13. En tant qu'agent, je veux que `tools/constant_variants.sh` ne change pas : il copie tous les `.c`/`.h` de la racine, l'aide comprise, et son remplacement exact de `pico8_fixed_from_double(<littéral>)` ne touche pas la définition de l'aide.
14. En tant qu'agent, je veux que les prérequis des cibles `constants-check`, `test_constants_check` et `test_constant_variants` nomment aussi les fichiers d'aide, afin qu'un changement de l'aide reconstruise ce qui en dépend.
15. En tant qu'agent, je veux que `tools/float_grep_exclusions.txt` gagne les deux fichiers d'aide, chacun avec une justification (« aide de test, hors cœur : `from_double`/`to_double`, liée par les tests, l'audit et le comparateur de constantes »), afin que la partition de `src/` reste complète et que `make float-grep` ne rende pas 2.
16. En tant qu'agent, je veux que `make float-grep` rende 2 si un jour un fichier du cœur inclut l'en-tête d'aide (il entrerait dans le cœur par `-MM` tout en étant exclu), afin que le verrou soit double : erreur de compilation sans l'inclusion, partition incohérente avec.
17. En tant qu'agent, je veux vérifier une fois, sur une copie jetable, qu'ajouter `pico8_fixed_from_double(0.5)` dans un fichier du cœur fait échouer sa compilation par défaut, et consigner le message dans l'« État de E1 », afin que la fermeture de l'angle mort soit mesurée et non affirmée.
18. En tant qu'agent, je veux que `src/pico8_float_observe.c` et `src/pico8_float_observe.h` disparaissent, avec l'inclusion conditionnelle `PICO8_FLOAT_OBSERVE` de `pico8_fixed.h`, afin que le build mort ne tienne plus de place dans l'en-tête du cœur.
19. En tant qu'agent, je veux retirer du `Makefile` l'arbre `obj-observe`, `OBSERVE_CFLAGS`, `observe_objs`, sa règle de motif, `pico8_replay_observe`, la cible `float-observe` et son `.PHONY`, sa ligne de légende, et son `-include` de fragments de dépendances, afin qu'il n'en reste aucune trace exécutable.
20. En tant qu'agent, je veux retirer `tools/float_observe.sh`, `tools/float_observe.txt`, `tests/test_float_observe.c`, sa règle et ses deux mentions dans `make unit` (prérequis et lancement), afin que la suite ne teste plus un outil sans objet.
21. En tant qu'agent, je veux retirer les deux lignes `pico8_float_observe.*` de la liste d'exclusion du 6.1, afin qu'elle ne nomme pas de fichier absent (sinon `float_grep.sh` rend 2).
22. En tant qu'agent, je veux corriger les commentaires que ce retrait rend faux — le « Ne pas fusionner les sept » des arbres d'objets, qui passent à six, et la mention des « en-tetes d'audit et d'observation » de `tools/float_grep.sh` —, et rien d'autre, afin de ne pas laisser de désordre créé par l'étape.
23. En tant qu'agent, je veux régénérer et versionner `tools/float_inventory.txt` dans le même commit que le déplacement, afin que `float-inventory-check` de `make test` reste vert.
24. En tant qu'agent, je veux régénérer `tools/float_guard.txt` (`make float-guard`, attendu code 0 et liste vide) et `tools/float_coverage.txt` (`make float-coverage`, dont les 6 lignes des définitions disparaissent), afin que les rapports versionnés décrivent l'arbre.
25. En tant qu'agent, je veux lancer `make frames-check` sans `frames-baseline` et constater les 12 fichiers identiques, afin de prouver la neutralité de l'étape.
26. En tant qu'agent, je veux relancer les quatre cibles du §6 et consigner leur ligne de synthèse et leur code dans l'« État de E1 », afin qu'E2 parte d'un reliquat mesuré.
27. En tant que lecteur du plan, je veux que la puce E1 porte « ✅ faite, voir « État de E1 » », et que la décision « Le build d'observation est retiré » soit marquée comme faite en E1, sans réécrire les « État » antérieurs qui citent `tools/float_observe.txt` (ils restent de l'historique).
28. En tant que lecteur du plan, je veux que l'« État de E1 » dise ce que devient le compte d'E2 : « les 4 de l'inventaire et les 3 d'en-têtes » confirmés, `pico8_fixed.h` sorti du grep.
29. En tant que mainteneur des commentaires (CLAUDE.md), je veux que l'en-tête d'aide ne porte qu'un contrat que les noms ne disent pas — `from_double` arrondit au plus proche, ce qui n'est **pas** la troncature de PICO-8 que porte `PICO8_FIXED_LIT` ; `to_double` est exacte —, sans récit de chantier.
30. En tant que mainteneur, je veux que le comportement actuel de `make constant-variants` (déjà en échec avant E1, voir Further Notes) ne soit ni corrigé ni aggravé par E1.

## Implementation Decisions

- **Emplacement du module d'aide : `src/`, pas `tests/`.** Tranché par la règle de simplicité (skill `guideline`), faute d'humain. Dans `src/`, l'aide est voisine de `pico8_fixed.c` : le comparateur de constantes la trouve sans nouvel argument (il reçoit déjà le chemin de `pico8_fixed.c` et compile avec `-I` sur son répertoire), `constant_variants.sh` la copie avec le reste de la racine sans changer, le build d'audit l'inclut comme un voisin, et les tests l'incluent comme ils incluent déjà `../src/…`. Dans `tests/`, il aurait fallu un nouvel argument au comparateur, à `constants_check.sh` et à `constant_variants.sh`, et un chemin hors racine pour les variantes. Le prix de `src/` est deux lignes d'exclusion justifiées, ce que la décision humaine sur le 6.1 prévoit déjà pour les builds d'audit.
- **Nom : `pico8_fixed_test_double`** (`.h` et `.c`). « test » dit l'exception du §6.1 ; le préfixe `pico8_fixed` le range à côté du module dont il est sorti. Les **noms de fonctions ne changent pas** : `tools/float_inventory.sh`, `tools/constant_variants.sh`, `tools/constants_compare.sh`, `tools/float_audit.sh` et les tests d'outils les lisent comme texte ; les renommer serait une réécriture d'outillage sans rapport avec l'objectif.
- **Contenu de l'aide** : l'en-tête inclut `pico8_fixed.h` (pour `pico8_fixed_t`) et déclare les deux fonctions ; la source les définit à l'identique de `pico8_fixed.c` aujourd'hui (même arrondi `±0.5` puis troncature `int64_t`, même division par 65536). Aucune autre fonction ne s'y ajoute.
- **Marqueur `/* TROU-1 */`** : il quitte `pico8_fixed.h` et `pico8_fixed.c` avec `from_double` et n'est pas reporté dans l'aide, qui n'est pas du code porté. Ceux de `pico8_fixed_cos`/`_sin` restent. La mise à jour du texte de TROU-1 (« `pico8_fixed_from_double` reste défini », « Fichiers : `src/pico8_fixed.c` ») relève d'E2, qui restreint TROU-1.
- **Aucun fichier du cœur n'inclut l'aide.** Vérifié en cadrage : hors `pico8_float_audit.c` et `pico8_float_observe.c`, aucun `.c` de `src/` n'appelle ces fonctions ; la seule mention du cœur est le commentaire de `pico8_ob.c` 201, laissé tel quel (E2 le retire comme périmé). Si un fichier du cœur ne compile plus, c'est un appel oublié : le corriger n'est pas dans l'étape, c'est un signal à consigner.
- **Liens** (tous dans le `Makefile`, par la variable d'objets de l'arbre concerné) :
  - arbre par défaut : `test_pico8_fixed`, `test_pico8_vec`, `test_pico8_geom`, `test_pico8_slipway_drag`, `test_pico8_playability`, `test_pico8_empty_planet_cleared`, `test_pico8_geoharvest_burst`, `test_pico8_tech_screen` ;
  - arbre UBSan : `test_pico8_fixed_ubsan` ;
  - arbre REF_AUDIT : les trois variantes `_refaudit` des tests ci-dessus qui en ont une ;
  - build d'audit : `pico8_replay_audit` prend la source d'aide depuis l'arbre par défaut, comme il y prend déjà `pico8_fixed.c` (l'aide n'a rien que les macros d'audit réécriraient, mais il n'y a aucune raison de la compiler sous `PICO8_FLOAT_AUDIT`) ;
  - **pas** `pico8_replay`, `REPLAY_SRCS`, `ENGINE_SRCS`, `CORE_SRCS`, `DATA_ENGINE_SRCS`, ni `main_raylib`.
- **Comparateur de constantes** : le programme qu'il engendre inclut l'en-tête d'aide en plus de `pico8_fixed.h`, et sa ligne de compilation ajoute la source d'aide prise dans le même répertoire que le `pico8_fixed.c` reçu. La signature de `constants_compare.sh`, `constants_check.sh` et `constant_variants.sh` ne change pas ; leur ligne d'usage peut dire que l'aide doit être voisine.
- **`test_constant_variants`** : la copie fabriquée prend aussi la source d'aide, `SOURCES` la liste, le pilote inclut l'en-tête d'aide. Le reste du test (valeurs, frames, assertions) ne bouge pas.
- **Retrait du build d'observation** : suppression pure, dans le même commit ou un commit dédié ; aucune fonctionnalité ne le remplace (la décision humaine dit que l'audit avant/après le remplace déjà).
- **Rapports versionnés** régénérés par leurs cibles, jamais édités à la main : `tools/float_inventory.txt`, `tools/float_guard.txt`, `tools/float_coverage.txt`. `tools/float_annotation.txt` n'a plus de cast et ne change pas.
- **Aucun `*.frames.hash` ne change** ; donc aucun rapport `tools/frames_reports/E1*.txt` ni ligne de registre.
- **Commentaires** (règle de CLAUDE.md) : on retire les commentaires qui décrivaient les prototypes sortis ; on corrige les deux commentaires que le retrait de l'observation rend faux (« sept » arbres, « audit et d'observation ») ; le commentaire d'en-tête de `tools/float_guard.sh` sur l'angle mort reste vrai de la garde elle-même et n'est pas touché. Le reste est pour le message de commit.

## Implementation Chunks

1. **Aide de test et sortie du cœur** (un seul commit, sinon `float-inventory-check` casse `make test`) : créer le module d'aide ; retirer prototypes, commentaires et définitions de `pico8_fixed.{h,c}` ; inclure l'aide dans les huit tests et dans `pico8_float_audit.c` ; liens et prérequis du `Makefile` (tests, UBSan, REF_AUDIT, audit, constants-check) ; programme engendré de `constants_compare.sh` ; `test_constant_variants` ; deux lignes d'exclusion ; régénérer `tools/float_inventory.txt`. Vérification : `make test` vert ; `make frames-check` identique sans `frames-baseline` ; `make ref-audit` construit ses binaires ; `nm tmp/pico8_replay` ne contient ni `pico8_fixed_from_double` ni `pico8_fixed_to_double` ; une copie jetable d'un fichier du cœur avec `pico8_fixed_from_double(0.5)` ne compile pas avec les `CFLAGS` par défaut.
2. **Retrait du build d'observation** : sources `src/pico8_float_observe.*`, inclusion conditionnelle de `pico8_fixed.h`, tout le bloc du `Makefile` (arbre, flags, règle, cible, légende, `-include`, `test_float_observe` dans `make unit`), `tools/float_observe.{sh,txt}`, `tests/test_float_observe.c`, deux lignes d'exclusion, deux commentaires devenus faux. Vérification : `grep -rn -i observe Makefile src tools tests` ne rend plus que des mots sans rapport (le cas échéant) ; `make test` vert ; `make float-grep` ne rend pas 2.
3. **Mesure et État** : `make float-guard` et `make float-coverage` régénérés et versionnés ; les quatre cibles du §6 lancées, lignes et codes consignés ; « État de E1 » écrit dans `docs/plan-2.md`, puce E1 marquée faite, décision « build d'observation retiré » marquée faite en E1. Vérification : `make test` vert, `make frames-check` identique, durée de `make test` consignée.

1 et 2 sont indépendants (2 peut précéder 1) ; 3 dépend des deux.

## Testing Decisions

- **Un bon test ici est un effet observable sur l'arbre, pas une assertion sur la forme du code.** La preuve de l'étape est portée par les cibles déjà en place : `make test` vert (dont `float-inventory-check`, `constants-check`, `float-audit`, `frames-check`), `make float-guard` à 0 fichier, `make float-grep` réduit aux 6 lignes attendues, `make float-link` et `make frames-ledger` inchangés.
- **Aucun nouveau test automatisé.** Le verrou « tout appel flottant du cœur est une erreur de compilation » est le compilateur lui-même (`-Werror`, déclaration implicite, sur chaque objet du cœur construit par `make test`) ; le verrou « l'aide ne rentre pas dans le cœur » est la partition de `float_grep.sh`. Les faire entrer dans `make test` est l'objet d'E4. La preuve ponctuelle de l'angle mort fermé (story 17) est une mesure consignée, faite sur une copie jetable, pas un test versionné.
- **Tests existants touchés** : les huit tests appelants (une ligne d'inclusion chacun, aucune assertion changée), `test_constant_variants` (copie, sources, inclusion du pilote), `test_float_observe` (supprimé avec l'outil). `test_constants_check`, `test_float_inventory`, `test_float_inventory_check`, `test_float_coverage`, `test_float_grep`, `test_float_audit` ne changent pas : ils ne lisent les noms de fonctions que comme texte fabriqué.
- **Prior art** : même forme que C2b (déplacement mécanique neutre, prouvé par `frames-check` identique et régénération de l'inventaire dans le même commit) et qu'E0 (mesure des quatre cibles consignée dans un « État »).

## Out of Scope

- **E2** : les mots de commentaire restants (`pico8_fixed.c` « no host float », `pico8_galaxy.c`, `pico8_ob.c` 201 et 347, `pico8_geom.h`, `pico8_research.h`, `pico8_type_tag.h`) et la restriction de TROU-1 dans `docs/trous-de-portage.md`. `make float-grep` reste donc en échec après E1, et c'est attendu.
- **E3** : retrait de `-lm` de `pico8_replay`. **E4** : entrée des cibles 6.1, 6.3, 6.4 dans `make test`.
- Tout renommage de `pico8_fixed_from_double`/`to_double`, ou le remplacement de leurs usages dans les tests par `PICO8_FIXED_LIT`.
- La correction de `make constant-variants` (voir Further Notes).
- Le volet `malloc` (§8), `pico8_fixed_rnd`, la vérification `mipsel`.
- Lancer l'appli raylib : jamais (CLAUDE.md). `make raylib` n'est pas concerné, `main_raylib` ne lie pas l'aide.

## Further Notes

- **Constat de cadrage, pré-existant, non traité : `make constant-variants` échoue déjà sur l'arbre actuel.** Sa recette appelle `constants_compare.sh` sans sources ; l'inventaire n'ayant plus de constante depuis C2b, le comparateur s'arrête sur « aucune constante dans tools/float_inventory.txt ni dans les sources » (code 1, reproduit en préparant ce PRD). La cible est hors suite et son objet (attribuer les divergences de C2b) est clos ; E1 ne la corrige pas et ne doit pas la casser davantage. À consigner dans l'« État de E1 » comme constat, pour l'humain.
- **`tools/gen_quarter_wave.c`** dit reproduire « the same rounding as `pico8_fixed_from_double` » ; il ne l'appelle pas et reste vrai, puisque l'aide garde l'arrondi.
- **Historique préservé** : les « État de C0 … D1 » qui citent `tools/float_observe.txt` et `make float-observe` ne sont pas réécrits ; les PRD de `docs/prd/done/` non plus.
- **Risque faible, à surveiller** : si le lien d'un test `ENGINE_SRCS` échoue sur une référence non résolue à `pico8_fixed_to_double`, c'est un appelant que le grep de cadrage n'a pas vu ; la liste des huit a été faite en excluant les mentions dans des chaînes de caractères (tests d'outils) — la recompiler est la vérification.
