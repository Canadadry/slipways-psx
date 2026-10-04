---
title: "Plan 12, étape 1 — Figer l'oracle"
description: "Avant de passer l'écran en 160×120, rien ne fixe ce que chaque rejeu doit atteindre ni ce que les e2e couvrent : sans cet oracle figé sur le code actuel, l'étape 6 n'aurait rien contre quoi vérifier les rejeux convertis."
status: needs-triage
---

## Problem Statement

Le plan 12 passe le framebuffer de 128×128 à 160×120 et reconvertit tous les rejeux (les 26 e2e de `tests/e2e/` et les 5 rejeux de `worst-case/`). Une fois l'écran changé, les `*.sha256` ne valent plus rien : chaque capture change, pixel par pixel. Il faut donc, **avant tout changement**, figer sur le code actuel ce qu'on vérifiera à l'étape 6 :

- où chaque rejeu aboutit dans le jeu (planètes, slipways, structures, horloge, économie), indépendamment de ce que montre l'écran ;
- quelles lignes du jeu l'ensemble des e2e exécute.

Aujourd'hui, `make state` écrit bien l'état de la dernière frame de chaque rejeu, mais dans `tmp/`, non versionné, et mêlé de champs qui dépendent du cadre (la ligne `camera`, les champs `screen` et `visible`) et qui bougeront forcément. Quant à la mesure de couverture de lignes, elle a été supprimée par `c38875e` (« retirer l'outillage de construction de la suite e2e ») : elle n'existe plus que dans l'historique.

## Solution

Sur le code actuel, sans toucher au jeu :

1. La mesure de couverture de lignes revient depuis `c38875e^` — le script, le pilote de rejeu instrumenté gcov et son arbre d'objets, et son test —, sous des cibles `make` à part, **hors de `make test-all`** : elle ne rallonge ni `make test` ni `make e2e`.
2. Une cible écrit, pour chacun des 31 rejeux, l'**état normalisé** de sa dernière frame — la sortie de `make state` pour ce rejeu, sans la ligne `camera` ni les champs `screen` et `visible` — dans un répertoire versionné le temps du plan.
3. La même cible écrit dans ce répertoire la **couverture de lignes de l'ensemble des e2e** : l'ensemble trié des `fichier:ligne` exécutés par l'union des 26 e2e.

Le répertoire et les outils sont temporaires : ils seront supprimés à l'étape 7 du plan.

## User Stories

1. En tant que développeur du portage, je veux un état figé de la dernière frame de chaque e2e, afin de vérifier à l'étape 6 qu'un rejeu converti à 160×120 aboutit au même état de jeu.
2. En tant que développeur du portage, je veux un état figé de la dernière frame de chacun des 5 rejeux de `worst-case/` (`full_game`, `full_game_new_game`, `full_game_pause_restart`, `full_game_pause_idle`, `title_idle`), afin qu'une partie complète convertie soit jugée elle aussi.
3. En tant que développeur du portage, je veux que l'état figé ne contienne pas la ligne `camera`, afin que le nouveau coin de départ de la caméra, (176,196) au lieu de (192,192), ne passe pas pour une régression.
4. En tant que développeur du portage, je veux que l'état figé ne contienne ni le champ `screen` ni le champ `visible` des planètes et des structures, afin que ce qui dépend du cadre — position écran, présence dans le cadre 160×120 — ne fasse pas échouer la comparaison.
5. En tant que développeur du portage, je veux que tout le reste de l'état soit gardé tel quel (horloge, positions `world`, états des planètes, slipways, structures, liens, compteurs, techs, lignes `zone` et `cap`), afin que l'étape 6 compare tout ce que le changement d'écran ne doit pas toucher.
6. En tant que développeur du portage, je veux que les lignes `zone` et `cap` restent dans l'état figé, afin que l'étape 6 puisse constater qu'elles bougent avec le cadre et vérifier qu'elles restent sous leur limite.
7. En tant que développeur du portage, je veux un fichier d'état figé par rejeu, nommé comme le rejeu, afin de retrouver d'un coup d'œil l'oracle d'un e2e donné.
8. En tant que développeur du portage, je veux que les 31 fichiers soient produits par une seule cible `make`, afin de ne pas les fabriquer à la main ni en oublier.
9. En tant que développeur du portage, je veux que la cible échoue si un rejeu s'arrête (`p8_fatal`, état absent), afin de ne jamais figer comme oracle un état d'arrêt ou un fichier manquant.
10. En tant que développeur du portage, je veux que le nombre de fichiers figés corresponde au nombre de rejeux (26 + 5), afin qu'un rejeu ajouté ou oublié se voie.
11. En tant que développeur du portage, je veux que l'oracle soit versionné dans le dépôt, afin qu'il survive aux `make clean`, aux changements de branche et aux sessions, jusqu'à la fin du plan.
12. En tant que développeur du portage, je veux que l'oracle soit produit deux fois de suite à l'identique, afin d'être sûr qu'il ne dépend ni de l'ordre d'exécution ni du parallélisme de `make`.
13. En tant que développeur du portage, je veux retrouver la mesure de couverture de lignes telle qu'elle existait avant `c38875e`, afin de ne pas réinventer un outil qui a déjà fait ses preuves.
14. En tant que développeur du portage, je veux que la couverture se mesure avec un pilote de rejeu instrumenté gcov, construit dans son propre arbre d'objets, afin de ne jamais mêler objets instrumentés et objets de `make test`.
15. En tant que développeur du portage, je veux une cible qui donne la couverture de lignes d'un ensemble de rejeux (`fichier:ligne`, une ligne par ligne exécutée, triée), afin de mesurer ce qu'un rejeu ou un groupe de rejeux exécute.
16. En tant que développeur du portage, je veux pouvoir calculer les lignes d'un ensemble absentes d'un autre, afin qu'à l'étape 6 on voie les lignes que les e2e convertis ne couvrent plus.
17. En tant que développeur du portage, je veux que le script revienne verbatim, avec ses sous-commandes `contrib` et `gap`, afin de restaurer un outil connu pour marcher sans réécrire ni lui ni son test.
18. En tant que développeur du portage, je veux que la mesure échoue sans rien écrire sur sa sortie quand un fichier d'événements manque ou qu'un rejeu échoue, afin qu'une couverture partielle ne soit jamais prise pour la vraie.
19. En tant que développeur du portage, je veux que la couverture ne garde que les sources du dépôt, sans les en-têtes système, afin que l'oracle ne dépende pas de la machine.
20. En tant que développeur du portage, je veux que la couverture de l'ensemble des 26 e2e soit écrite dans le répertoire de l'oracle, afin que l'étape 6 vérifie qu'elle ne baisse pas.
21. En tant que développeur du portage, je veux que le test de la mesure de couverture revienne avec elle, afin qu'elle soit vérifiée avant qu'on se fie à ses chiffres.
22. En tant que développeur du portage, je veux que ce test tourne sous sa propre cible, hors de `make test` et de `make test-all`, afin que la boucle rapide reste rapide (règle du projet : ne jamais allonger `make test` ni `make e2e`).
23. En tant que développeur du portage, je veux que `make test` et `make e2e` passent à l'identique avant et après cette étape, afin que figer l'oracle ne change rien au jeu ni à la suite.
24. En tant que développeur du portage, je veux que les cibles restaurées apparaissent dans la légende en tête du `Makefile`, avec la mention « outside the suite », afin qu'on sache qu'elles existent et qu'elles sont temporaires.
25. En tant que développeur du portage, je veux que la normalisation retire seulement ` screen X Y` et ` visible N` des lignes `planet` et `structure`, et seulement la ligne `camera`, afin qu'aucune autre information ne disparaisse par un motif trop large.
26. En tant que relecteur, je veux voir dans le diff de cette étape l'oracle complet (31 états + 1 couverture) et les outils, sans aucun changement dans `src/game/`, `src/engine/` ni `src/p8/`, afin de m'assurer que l'oracle a été figé sur le code d'avant le changement.
27. En tant que développeur des étapes suivantes, je veux que le répertoire de l'oracle soit un emplacement unique et nommé d'après le plan, afin de le supprimer d'un coup à l'étape 7 sans rien oublier.

## Implementation Decisions

- **Restauration de la couverture de lignes.** On reprend depuis `c38875e^`, en les adaptant au `Makefile` actuel :
  - le script de mesure, tel quel, avec ses quatre sous-commandes (`cover`, `diff`, `contrib`, `gap`) : la restauration est verbatim, on n'élague ni le script ni son test, et on n'y ajoute rien. Seules `cover` et `diff` servent au plan (étapes 1 et 6) ;
  - l'arbre d'objets gcov (`--coverage`, son propre répertoire sous `tmp/`, distinct des arbres `obj`, `obj-asan`, `obj-ubsan`, `obj-sanitize` — la règle « un arbre par jeu de drapeaux » du `Makefile` s'applique) et le pilote `replay_gcov`, lié depuis les mêmes sources que `tmp/replay` (`REPLAY_SRCS`) ;
  - le test de la mesure, ses neuf cas tels quels, qui n'entre **pas** dans `UNIT_TESTS` : il a sa cible propre, hors de `make test-all`.
  - Les autres outils supprimés par `c38875e` (minimize, class_coverage, les cibles `line-coverage-contrib`, `e2e-gap`, `e2e-judge`, `e2e-cleanup`, la référence de couverture et sa liste d'inatteignables, les sondes `CLASS_COVERAGE_BUILD`) ne reviennent **pas** : le test de la mesure ne dépend que du script, de l'arbre gcov et du pilote.
- **Contrat de la mesure** (inchangé par rapport à `c38875e^`) : `cover <pilote> <arbre gcov> <events>...` rejoue chaque fichier depuis l'écran titre, un processus par fichier, et écrit l'union triée des `fichier:ligne` exécutés, sources du dépôt seulement (les chemins absolus — en-têtes système — sont exclus). Code 1 et rien sur la sortie standard si un fichier manque ou qu'un rejeu échoue. `diff <a> <b>` : les lignes de `a` absentes de `b`. `contrib` et `gap` reviennent avec le même contrat, sans rôle dans le plan.
- **Cibles `make`** (noms indicatifs, à confirmer à l'implémentation, toutes « outside the suite » dans la légende) :
  - `line-coverage EVENTS="<events>..."` : la couverture de ces rejeux dans `tmp/` (comme avant `c38875e`) ;
  - `line-coverage-test` : construit et lance le test de la mesure ;
  - `oracle` (ou équivalent) : produit l'oracle versionné complet.
- **État normalisé.** C'est la sortie par rejeu de `make state` (`tmp/state/<nom>.txt`), dont on retire :
  - la ligne qui commence par `camera` ;
  - sur les lignes `planet` et `structure`, les champs ` screen X Y` et ` visible N` (le champ `world X Y` et tout ce qui suit `visible`, par exemple ` links N`, restent).
  Le reste est gardé octet pour octet, lignes `zone` et `cap` comprises. La normalisation n'est pas un module : c'est une ligne `sed`/`awk` dans la recette de la cible `oracle`, sans script ni C à part.
- **Production de l'oracle.** La cible `oracle` s'appuie sur `make state` (qui rejoue déjà `FULL_GAME_EVENTS` et `E2E_EVENTS` et juge chaque rejeu par `tools/state_peaks.sh`) : elle échoue si `make state` échoue (un rejeu arrêté ou sans état), puis normalise chacun des 31 fichiers d'état dans le répertoire de l'oracle, un fichier par rejeu, nommé d'après le rejeu (les noms des e2e et de `worst-case/` ne se recouvrent pas). Elle écrit ensuite dans le même répertoire la couverture de l'union des 26 e2e (`E2E_EVENTS`), par la sous-commande `cover`.
- **Répertoire de l'oracle.** Un seul répertoire versionné, hors de `tmp/` (ignoré par git), nommé d'après le plan pour être supprimé d'un bloc à l'étape 7. Il contient 31 états normalisés et 1 fichier de couverture, rien d'autre.
- **Aucun changement du jeu.** Cette étape ne touche ni `src/game/`, ni `src/engine/`, ni `src/p8/`, ni `src/platform/`, ni `src/devtools/state.c` : l'oracle est figé sur le code actuel, 128×128.
- **Commentaires.** Les fichiers restaurés suivent la règle des commentaires du `CLAUDE.md` (seuls un piège, un contrat C non dit par le nom, ou un écart avec le cart restent) ; les commentaires d'en-tête qui décrivent l'usage d'un script en sont la seule aide à la lecture.

## Implementation Chunks

1. **Restaurer la mesure de couverture** — un vrai cycle TDD. Le test restauré d'abord, sous `make line-coverage-test` : rouge (script et pilote absents). Puis l'arbre gcov, `replay_gcov`, le script et `make line-coverage` : vert. Terminé quand `make line-coverage-test` passe et que `make test` lance exactement les mêmes tests qu'avant.
2. **Produire l'oracle** — une étape de production, pas un cycle de test. La cible `oracle` écrit les 31 états normalisés et la couverture de l'union des 26 e2e dans le répertoire versionné. Dépend du chunk 1. Acceptée sur les vérifications listées dans Testing Decisions (« L'oracle lui-même »). États et couverture partagent une cible, une exécution et un répertoire : les séparer ajouterait une étape sans rien apporter.

## Testing Decisions

- **Un bon test** vérifie ce que l'outil rend, pas comment il le calcule : des ensembles de lignes sur des fichiers d'événements construits (un fichier qui ne fait rien couvre strictement moins qu'un fichier qui clique, l'union de deux fichiers, l'échec sur un fichier absent ou mal formé), et les opérations d'ensembles sur des ensembles écrits à la main.
- **Mesure de couverture** : le seul module testé. Le test restauré verbatim depuis `c38875e^` (`tests/test_line_coverage.c`, neuf cas), lancé par `make line-coverage-test`, jamais par `make test` ni `make test-all`.
- **Normalisation** : pas de test propre — c'est un filtre d'une ligne dont la sortie se lit une fois, au gel de l'oracle. Elle est vérifiée par les contrôles de l'oracle ci-dessous, puis gratuitement à l'étape 6 : un filtre faux ferait diverger les rejeux convertis de l'oracle.
- **L'oracle lui-même** : vérifié à la production — la cible échoue si un rejeu s'arrête ; 31 fichiers d'état ; aucune ligne `camera`, aucun champ ` screen ` ni ` visible ` dans le répertoire ; hors de ces champs, chaque fichier est identique à son état brut de `tmp/state/` ; deux exécutions successives donnent des fichiers identiques ; le fichier de couverture n'est pas vide et contient des lignes de `src/game/`.
- **Non-régression** : `make test` et `make e2e` passent, et la liste des tests de `make test` est la même qu'avant l'étape.
- **Art antérieur** : `tests/test_line_coverage.c` à `c38875e^` (fixtures d'événements écrites par le test, scripts lancés par `system`) ; `tests/test_state_peaks.c` (test C d'un script shell de `tools/`, qui lit les fichiers d'état de `make state`) ; `tests/test_state.c` pour le format des fichiers d'état.

## Out of Scope

- Tout changement de taille d'écran, de caméra, d'ancrage ou de rejeu (étapes 3 à 5).
- Les fichiers d'étiquettes et le devtool qui les produit (étape 2).
- La comparaison des rejeux convertis à l'oracle, et le jugement des écarts sur `zone` et `cap` (étape 6).
- La suppression des outils et de l'oracle, le `make e2e-baseline` final (étape 7).
- Le retour des autres outils supprimés par `c38875e` : minimize, class_coverage, e2e-gap, e2e-judge, e2e-cleanup, la référence de couverture et sa liste d'inatteignables.
- Toute modification du format de `make state` lui-même : la normalisation est un filtre posé après lui.
- L'ajout de quoi que ce soit à `make test`, `make e2e` ou `make test-all`.

## Further Notes

- Le plan est encore un brouillon « à passer en `plan-converge` » ; cette PRD ne couvre que l'étape 1, telle qu'elle est écrite.
- La couverture des `worst-case/` n'est pas demandée : seule celle des e2e entre dans l'oracle.
- Les lignes `zone` et `cap` de l'oracle sont figées à 128×128 ; l'étape 6 sait déjà qu'elles bougeront et ne les juge que contre leur limite.
- Les outils et le répertoire de l'oracle sont temporaires : leur légende dans le `Makefile` peut le dire, pour que l'étape 7 les retrouve tous.
- Sans humain disponible, les points que `to-prd` fait valider (modules, modules testés, découpage) ont été tranchés par un agent `guideline` : restauration verbatim plutôt qu'élaguée, normalisation dans la recette plutôt qu'en module testé, deux chunks plutôt que trois.
