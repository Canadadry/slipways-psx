---
title: "Plan 5, étape 0 — vocabulaire de base, arborescence résolue et consignes aux agents"
description: "Avant que le code ne soit renommé, il faut un vocabulaire anglais et une arborescence cible validés par l'humain, et des consignes d'agents qui ne prescrivent plus le portage littéral."
status: needs-triage
---

## Problem Statement

Le plan 5 (`docs/plan-5.md`) donne à tout le code des noms anglais clairs. Les étapes 1 à 3 déplacent puis renomment des centaines de symboles, mais elles ne peuvent rien inventer : il leur faut d'abord une entrée unique, relue par l'humain, qui fixe le sens et le nom anglais des concepts récurrents du domaine (`jo`, `ga`, `jc`, `g_`, `pj`, `hp`, `lq`…), des champs de l'en-tête commun et des membres de la vtable des classes, ainsi que la place et le nom cible de chaque fichier actuel.

Aujourd'hui, ce savoir est éparpillé : il est dans `docs/glossaire-noms.md` (670 lignes, rangées par symbole minifié et non par concept) et dans le code lui-même. Aucun document ne dit « ce concept s'appelle désormais ainsi », ni « ce fichier va là, sous ce nom ».

En même temps, les consignes que reçoivent les agents disent l'inverse du plan. Le `CLAUDE.md` et le skill `fix-bug` imposent un portage littéral jugé contre `pico8/lua.lua`, et le `CLAUDE.md` envoie le sens d'un nom minifié au glossaire. Un agent lancé pendant les étapes 1 à 4, y compris un `fix-bug` déclenché par `check-e2e`, recevrait donc deux règles contraires.

## Solution

L'étape produit trois choses, sans toucher au code :

1. **Un document de vocabulaire présenté rempli** (sous `docs/`). Il couvre les concepts récurrents, les champs de l'en-tête commun et les membres de la vtable. Pour chaque entrée, il donne le nom actuel, son sens, le nom anglais proposé et la source du sens. Il contient aussi l'**arborescence résolue** : chaque fichier actuel sous `src/`, `tests/` et `tools/`, avec son répertoire cible et son nom cible. En tête du document figure un marqueur visible **« Statut : à valider »**. L'humain le passe à « validé » après correction. C'est la trace que l'étape 1 vérifie comme condition d'entrée.
2. **Des consignes aux agents remises d'accord avec le plan.** Le `CLAUDE.md` et le skill `fix-bug` perdent le portage littéral jugé contre `lua.lua` et la branche « nom minifié → glossaire ».
3. **La note « Découvert »** sous l'entrée 0 de `docs/plan-5.md`.

L'étape se termine au point d'arrêt humain. `plan-pilot` s'arrête à la fin de ce tronçon, et l'étape 1 n'est écrite qu'après la validation.

## User Stories

1. En tant qu'humain relecteur, je veux trouver le vocabulaire déjà rempli plutôt qu'un gabarit vide, pour n'avoir qu'à corriger et pas à rédiger.
2. En tant qu'humain relecteur, je veux voir le sens de chaque nom minifié à côté du nom anglais proposé, pour juger le nom sans rouvrir le glossaire.
3. En tant qu'humain relecteur, je veux savoir d'où vient chaque sens (ligne du glossaire, site du code), pour vérifier une proposition qui me surprend.
4. En tant qu'humain relecteur, je veux que les propositions douteuses soient signalées comme telles, pour concentrer ma relecture là où l'agent a hésité.
5. En tant qu'humain relecteur, je veux que les ~50 concepts soient regroupés par thème (noyau objet, planètes, commerce, recherche, interface…), pour relire par familles cohérentes.
6. En tant qu'humain relecteur, je veux une table dédiée à chaque champ de l'en-tête commun, pour trancher d'un bloc ce que toute entité porte.
7. En tant qu'humain relecteur, je veux une table dédiée à chaque membre de la vtable des classes, pour trancher d'un bloc le nom des messages et des méthodes.
8. En tant qu'humain relecteur, je veux qu'un nom minifié qui porte deux sens selon le contexte apparaisse avec ses deux sens et deux propositions, pour qu'aucun renommage ne fusionne deux concepts.
9. En tant qu'humain relecteur, je veux que les noms proposés suivent les règles du §4 (snake_case, noms complets, pas de préfixe pour le jeu, `p8_` pour le runtime émulé), pour ne pas valider un nom que l'étape 2b devrait de toute façon refuser.
10. En tant qu'humain relecteur, je veux une arborescence qui liste chaque fichier actuel une seule fois, avec son répertoire et son nom cibles, pour valider le déplacement complet d'un coup.
11. En tant qu'humain relecteur, je veux voir les fichiers dont le classement s'écarte de la liste indicative du §3, pour trancher ces cas en connaissance de cause.
12. En tant qu'humain relecteur, je veux un marqueur de statut visible en tête du document, que je passe de « à valider » à « validé », pour que ma validation soit tracée dans le dépôt.
13. En tant qu'agent de l'étape 1, je veux lire dans l'arborescence validée le chemin cible exact de chaque fichier, pour faire mes `git mv` sans rien inventer.
14. En tant qu'agent de l'étape 1, je veux vérifier mécaniquement le statut « validé » du document, pour savoir si ma condition d'entrée est remplie.
15. En tant qu'agent de l'étape 1, je veux pouvoir rejouer le même contrôle mécanique (exhaustivité, noms cibles uniques) sur la version validée, pour détecter une correction humaine qui aurait cassé l'unicité.
16. En tant qu'agent des étapes 2b et 3, je veux dériver les noms des fonctions, des types et des champs des concepts validés, pour que le TSV reste cohérent avec le vocabulaire.
17. En tant qu'agent des étapes 1 à 4, je veux un `CLAUDE.md` qui ne m'impose plus le portage littéral contre `lua.lua`, pour ne pas recevoir deux règles contraires.
18. En tant qu'agent `fix-bug` lancé par `check-e2e` en cours de chantier, je veux juger un bug contre le comportement actuel du C couvert par les tests et les e2e, et non contre les lignes du cart, pour ne pas défaire un renommage au nom de la fidélité.
19. En tant qu'agent `fix-bug`, je veux ne plus être invité à ajouter des lignes au glossaire, pour ne pas alimenter un fichier destiné à disparaître à l'étape 5.
20. En tant qu'agent qui écrit un commentaire pendant le chantier, je veux une règle des destinations sans la branche « nom minifié → glossaire », pour ne pas réintroduire de digrammes dans le code.
21. En tant que mainteneur, je veux que `pico8/lua.lua` reste nommé comme archive consultable, pour pouvoir encore y fouiller au besoin.
22. En tant que mainteneur, je veux que le glossaire reste en place jusqu'à l'étape 5, pour que l'étape 2 puisse encore s'en servir.
23. En tant que mainteneur, je veux que `make test-all` reste vert et qu'aucun `*.sha256` de `tests/e2e/` ne change, pour que l'invariant du §5 tienne.
24. En tant que pilote du plan, je veux une note « Découvert » sous l'entrée 0 du plan. Elle liste les noms qui résistent au §4, les fichiers rangés hors du §3 et les résidus de consigne hors périmètre. Je pourrai ainsi corriger le plan avant l'étape 1.
25. En tant que pilote du plan, je veux qu'un constat qui contredit le §3 ou le §4 corrige le plan dans le même commit, pour que le plan reste vrai.
26. En tant que lecteur francophone, je veux que le vocabulaire, le `CLAUDE.md` et la note restent en français, conformément au §2.

## Implementation Decisions

### Le document de vocabulaire

- Un seul nouveau fichier, `docs/vocabulaire.md`, en français. Il rassemble le vocabulaire et l'arborescence pour que la validation ne porte que sur un artefact.
- **En-tête** : le titre, puis une ligne de statut seule sur sa ligne, de la forme exacte `Statut : à valider`. L'humain la remplace par `Statut : validé`. L'agent de cette étape ne la passe jamais à « validé ». Un court paragraphe dit que ce document est l'entrée des étapes 1 à 3 et qu'il cède la place au TSV de l'étape 2b pour le détail des symboles.
- **Section concepts** : environ 50 concepts récurrents, en tables par thème. Les colonnes sont : nom actuel (digramme du cart ou nom C actuel), sens (une phrase), nom anglais proposé, source (section du glossaire ou site du code), doute (vide ou une courte raison). Le choix des concepts s'appuie sur leur récurrence dans le code (classes et sortes `jo`, `ga`, `pm`, `jc`, `g_`, `pj`, `hp`, `lq`, `mz`, `ui`, `iv`, `fm`/`hy`, `ke`… ; structures de dispatch `cv`, `cg`, `hv`, `dc`, `lt`, `he`), et non sur l'exhaustivité du glossaire. L'exhaustivité est le travail de l'étape 2b.
- **Section en-tête commun** : une ligne par membre de `pico8_struct_header_t`, sans exception. Ce sont aujourd'hui `cls`, `me`, `t`, `fa`, `ra`, `p`, `kq`, `ql`, `fg`, `mt`, `ma`, `fx`, `p_`, `m_`, `emitter` et `probe`, plus le nom proposé pour le type lui-même. Un membre déjà anglais garde une ligne, avec le statut « conservé » quand il ne change pas.
- **Section vtable** : une ligne par membre de `pico8_struct_class` (`kj`, `ev`, `hd`, `hk`, `hb`, `has_hb`, `ldown`, `rdown`, `gv`, `hq`, `ey`, `kf`, `cn`, `ol`, `mm`, `states`, `tags`, `lineage`, `md`, `rendre`), plus le nom proposé pour le type de la classe. Les noms de message que route `dc` (`kj`, `ev`, `hd`, `hk`, `hb`) y sont nommés de façon cohérente avec la vtable.
- **Section arborescence** : une table qui donne, pour chaque fichier suivi par git sous `src/`, `tests/` et `tools/` (aujourd'hui 389 : 171, 186 et 32), le chemin actuel, le répertoire cible et le nom cible.
  - Fichiers de `src/` : leur répertoire est l'un de `p8/`, `engine/`, `game/`, `platform/`, `devtools/`, `cmd/` (§3). Leur nom suit le §4 : nom du module, sans préfixe `pico8_`, y compris dans `p8/` ; les `main_*.c` vont dans `cmd/` sous leur nom de binaire.
  - `tests/` reste à plat. Les fichiers y sont renommés à partir du vocabulaire (`test_pico8_jo.c` → `test_planet.c`…). Les e2e et les fixtures déjà parlants gardent leur nom, avec le statut « conservé ».
  - `tools/` reste `tools/`. Un nom français ou minifié y reçoit un nom cible anglais.
  - Un fichier qui garde son nom figure quand même, avec cible = actuel, pour que l'exhaustivité se lise mécaniquement.
  - Un fichier rangé hors de la liste indicative du §3 est marqué dans la colonne doute et repris dans la note « Découvert ».
- Le document cite les noms ; il ne recopie pas le glossaire. Le glossaire n'est ni modifié ni supprimé.

### Les consignes aux agents

- **`CLAUDE.md`** : retirer de la section « Le portage » la phrase du portage littéral, fonction par fonction, de `pico8/lua.lua`, avec sa clause « pas une réécriture depuis le document de conception ». La remplacer par la décision du §2 en une phrase : le C est la référence, le jeu est couvert par les e2e, `pico8/lua.lua` reste comme archive. Dans la section « Commentaires », retirer la branche « le sens d'un nom minifié (→ glossaire) ». La règle passe de quatre à trois destinations, et le reste de la phrase ne change pas. Le reste du `CLAUDE.md` (description des structs, `TROU-7`, build) n'est pas touché : il relève de l'étape 5.
- **`fix-bug`** : remplacer la contrainte « This is a literal port… judged against `pico8/lua.lua` ». Un bug se juge désormais contre le comportement établi du C, couvert par les tests et les e2e, et contre les conventions du port citées depuis un site réel ; `lua.lua` peut se consulter comme archive pour comprendre une intention. Supprimer la ligne « the meaning of a minified name… → `docs/glossaire-noms.md` ». La ligne `TROU-n` reste. Le fichier est écrit en anglais : on le modifie dans sa langue actuelle, sans le traduire, conformément à la règle des changements chirurgicaux.
- Un grep a été fait sur les autres skills (`help-me-debug`, `tdd`, `check-e2e`, `prd-autopilot`, `plan-pilot`…). Aucun ne contient la règle du portage littéral ni la branche du glossaire, et ils ne sont donc pas touchés.
- `reprendre-les-commentaires` parle d'un glossaire au sens générique de sa méthode. L'étape 4 l'encadre et l'étape 5 le met à jour, donc il n'est pas touché ici.

### La note « Découvert »

- Une note courte, ajoutée sous l'entrée 0 du §6 de `docs/plan-5.md`, en français. Elle contient :
  - les noms qui ont résisté au §4 ;
  - les fichiers rangés autrement que dans la liste indicative du §3 ;
  - les concepts à double sens ;
  - les résidus de la règle du portage littéral restés hors périmètre. Au moins la ligne du `README` qui l'énonce encore : le `README` est mis à jour à l'étape 5 et le `CLAUDE.md` y renvoie pour la cible.
- Si un constat contredit le §3 ou le §4, le plan est corrigé dans le même commit.

## Implementation Chunks

1. **Consignes aux agents** : les modifications du `CLAUDE.md` et du skill `fix-bug`. Ce morceau est indépendant des autres et passe en premier, parce que tout agent lancé ensuite, `fix-bug` compris, doit lire la nouvelle règle.
2. **Vocabulaire** : `docs/vocabulaire.md` avec le statut « à valider » et ses sections concepts, en-tête commun et vtable, remplies.
3. **Arborescence résolue** : la section arborescence du même document. Elle dépend du morceau 2, car les noms cibles des fichiers dérivent des noms de concepts (`pico8_jc*` → `slipway*`, tests compris).
4. **Contrôle et note** : le contrôle mécanique décrit ci-dessous, puis la note « Découvert » sous l'entrée 0 du plan, et la correction du plan si un constat le contredit.

## Testing Decisions

- L'étape ne touche ni `src/`, ni `tests/`, ni `tools/`, ni le `Makefile` : « aucun code n'est touché ». Ses « tests » sont donc des contrôles mécaniques, lancés depuis le terminal pendant l'étape. Ils ne sont ni ajoutés à `tools/` ni branchés dans `make test` : écrire un outil reviendrait à toucher au code, et l'étape 1 rejoue ces contrôles comme condition d'entrée.
- Contrôles à passer avant de déclarer l'étape finie :
  1. **Exhaustivité de l'arborescence** : l'ensemble des chemins actuels de la table est exactement l'ensemble rendu par `git ls-files src tests tools`, chaque chemin une seule fois, sans chemin inconnu.
  2. **Unicité des cibles** : aucun chemin cible (répertoire + nom) n'apparaît deux fois, même à la casse près.
  3. **Répertoires cibles légitimes** : toute cible sous `src/` tombe dans l'un des six répertoires du §3 ; toute cible de `tests/` reste à plat sous `tests/`, sauf `tests/e2e/` et `tests/fixtures/` qui restent où ils sont ; toute cible de `tools/` reste sous `tools/`.
  4. **Couverture de l'en-tête et de la vtable** : chaque membre de `pico8_struct_header_t` et de `struct pico8_struct_class`, extrait de l'en-tête C qui les déclare, a une ligne dans la section correspondante.
  5. **Statut** : le document contient exactement une ligne `Statut : à valider`.
  6. **Consignes** : plus aucune occurrence de la règle du portage littéral ni de la branche « nom minifié → glossaire », ni dans `CLAUDE.md` ni dans `.claude/skills/fix-bug/` (recherche de `lua.lua`, `littéral`/`literal port`, `glossaire`).
  7. **Invariant** : `make test-all` vert ; `git diff --stat` ne montre aucun fichier sous `src/`, `tests/`, `tools/`, ni le `Makefile`.
- Le contrôle 4 est la seule vérification qui lit le code. Elle extrait les membres de l'en-tête C qui les déclare, si bien qu'un membre ajouté ou oublié la fait échouer.
- La pratique existante du dépôt sert de modèle : les vérifications par listes attendues de `tools/` (`constants_check.sh` contre `constants_expected_ecarts.txt`, `class_coverage_net_expected.txt`) comparent une liste écrite à ce que le code dit vraiment. Le contrôle d'arborescence fait la même chose, mais une seule fois et sans fichier ajouté.
- Il n'y a pas de test unitaire : il n'y a aucun comportement de code à décrire.

## Out of Scope

- Tout changement sous `src/`, `tests/`, `tools/` et dans le `Makefile` : aucun déplacement, aucun renommage, aucun script de contrôle versionné. Les déplacements sont le travail de l'étape 1.
- L'outil de renommage (2a), le TSV complet `docs/renommage.tsv` (2b) et l'extraction exhaustive par l'AST clang.
- La suppression ou la modification de `docs/glossaire-noms.md` : il reste jusqu'à l'étape 5.
- La mise à jour du `README`, de `docs/trous-de-portage.md`, de `TROU-N` → `GAP-N`, de la nouvelle arborescence et des chemins dans le `CLAUDE.md`, et des skills qui citent des symboles ou des chemins (dont `reprendre-les-commentaires`) : tout cela relève de l'étape 5.
- Le passage du statut à « validé » : il revient à l'humain seul.
- La rédaction de la PRD de l'étape 1, qui n'est écrite qu'après la validation.
- La traduction des docs, du `CLAUDE.md` ou des skills, et la réécriture en français du skill `fix-bug`.

## Further Notes

- Pour plan-pilot, cette étape est la phase 1 (tronçon 1). Le tronçon s'arrête après elle : la reprise suppose que l'humain a passé `docs/vocabulaire.md` à `Statut : validé` et que les commits de l'étape sont poussés sur la branche du chantier.
- La condition d'entrée de l'étape 1 propage d'abord les corrections humaines aux noms qui en dérivent, puis rejoue les contrôles 1 à 3 sur la version validée. C'est pour cela qu'ils sont décrits ici avec précision.
- Le brief demandait de soumettre les points « à vérifier avec l'utilisateur » de `to-prd` à un agent `guideline` neuf. Aucun outil de lancement d'agent n'était disponible dans cette session. Le skill `guideline` a donc été chargé et appliqué directement à ces trois points :
  - **Les modules attendus** : un document, deux fichiers de consignes et la note.
  - **Ce qu'on teste** : des contrôles mécaniques non versionnés, plutôt qu'un script dans `tools/`. Raison : simplicité, et le texte de l'étape interdit de toucher au code.
  - **Le découpage** : quatre morceaux dans l'ordre de dépendance.
