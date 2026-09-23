---
title: "Plan 2, Phase C2a : l'outillage d'attribution, le rapport de divergence et le test des constantes contre le relevé"
description: "Avant de corriger les 16 constantes décalées d'un LSB (C2b), il faut trois outils : un harnais qui applique une constante à la fois et consigne les frames qu'elle fait diverger, un rapport de divergence complet écrit avant tout re-baseline, et un test qui compare chaque constante du C à sa valeur hexa relevée sur PICO-8. C2a ne change aucune constante."
status: needs-triage
---

## Problem Statement

Le relevé PICO-8 (`tools/probe_out.txt`, bloc A) a montré que PICO-8 **tronque** les littéraux décimaux (`floor(v × 65536)`), alors que `pico8_fixed_from_double` arrondit au plus proche. Résultat : **16 des 34 littéraux distincts** du cœur sont décalés d'un LSB (`0.001`, `0.0032`, `0.005`, `0.031`, `0.05`, `0.08`, `0.0977`, `0.1`, `0.19`, `0.27`, `0.55`, `0.6`, `0.8`, `0.85`, `0.99`, `1.08`). Ils se répartissent sur les 61 jetons `constante` de `tools/float_inventory.txt`. La correction est l'étape C2b. Contrairement à C1, elle **fera** diverger `frames.hash`, et la sortie de la Phase C exige que la **première** frame divergente de chaque fichier d'événements soit rattachée à une constante précise du relevé, et le reste étiqueté cascade.

Aujourd'hui, rien ne permet de faire ce rattachement ni d'en garder la trace :

1. **Pas d'attribution.** Appliquer les 16 corrections ensemble donne un seul paquet de frames divergentes, sans rien qui dise quelle constante a fait basculer quelle frame. Il faudrait appliquer chaque constante **seule** et rejouer tout le filet (les 8 fichiers de `FRAMES_EVENTS`). Aucun script ne le fait, et le pilote de rejeu écrit toujours son dump texte au même chemin fixe (`tmp/pico8_replay_dump.txt`), que deux tests de la suite relisent aussi. Plusieurs rejeux de variantes s'y écraseraient mutuellement.
2. **La trace disparaît au re-baseline.** `tools/frames_check.sh` ne cite que les 10 premières frames divergentes (`MAX_LISTED=10`), sur la sortie d'erreur. `make frames-baseline` écrase ensuite les références sans rien écrire. Une fois C2b re-baseliné, rien ne dit plus quelles frames avaient divergé, ni en quelles plages. D1, D2, D3 et D4 auront le même problème : quatre re-baselines de plus, chacun avec son rapport exigé par le plan.
3. **Aucune valeur numérique n'est testée.** Le filet ne protège que les constantes qu'il atteint. D'après `tools/float_coverage.txt`, 23 des 61 jetons `constante` sont NON COUVERT : une erreur sur l'un d'eux passerait inaperçue. Le §5 du plan promettait que le relevé deviendrait une **fixture**, et aucun test ne compare encore une constante du C à sa valeur relevée.

## Solution

C2a livre trois outils, n'en applique aucun au code du cœur, et laisse `make frames-check` identique sur les 8 fichiers.

- **(iii) Le comparateur de constantes et son test, dans `make test`.** Un outil lit les jetons `constante` de l'inventaire. Il évalue la valeur que le C produit pour chaque littéral, en compilant le littéral tel qu'il figure dans la source, et la compare à l'hexa du bloc A de `tools/probe_out.txt`, lu par analyse du fichier et jamais recopié. Il sort une table par littéral : relevé, valeur C, statut (`ok` / `ECART`), sites. Un fichier versionné de **défaillances attendues** liste les 16 littéraux décalés. Le test passe si les littéraux en `ECART` sont exactement ceux de la liste. Il échoue dans trois cas : un littéral listé qui passe (liste périmée), un littéral non listé en écart (régression), un ensemble vide de constantes vérifiées. C2b videra la liste dans le commit qui corrige les constantes.
- **(ii) Le rapport de divergence.** Un script lit une référence et un fichier d'empreintes courant, et écrit, pour chaque fichier d'événements, la première frame divergente, leur nombre, **toutes** les plages et un éventuel écart de longueur, sans plafond. `make frames-report ETAPE=<id>` l'écrit dans un fichier versionné, un par étape. `make frames-baseline` **refuse** de re-baseliner quand une référence diverge sans `ETAPE=`. Avec `ETAPE=`, il écrit d'abord le rapport, puis copie les références. La trace précède donc mécaniquement le re-baseline.
- **(i) Le harnais d'attribution, hors suite.** `make constant-variants` prend, dans la table du comparateur, les littéraux en `ECART` (16 aujourd'hui, calculés et non listés à la main). Pour chacun, il construit une **variante** : copie de `src/` dans un répertoire propre sous `tmp/`, où les seuls appels `pico8_fixed_from_double(<ce littéral>)` sont remplacés par la valeur brute du relevé. Il compile un pilote de rejeu depuis cette copie, rejoue les 8 fichiers de `FRAMES_EVENTS` avec capture, empreintes et dump texte **dans le répertoire de la variante**, puis écrit le rapport de divergence de la variante contre les références versionnées. Une synthèse par littéral et par fichier d'événements (première frame, nombre, plages, sites) clôt la passe. C2b s'appuiera dessus pour attribuer chaque frame. Le harnais est testé sur un cas fabriqué : un mini-arbre source où une constante connue fait basculer une frame connue, et une autre constante ne change rien.

## User Stories

1. En tant qu'agent qui mène C2b, je veux appliquer chaque constante décalée seule et rejouer tout le filet, pour savoir quelles frames chacune fait diverger avant de les appliquer ensemble.
2. En tant qu'agent, je veux que la liste des constantes à essayer vienne d'un calcul (valeur C ≠ relevé), et non d'une liste recopiée, pour qu'un décompte faux ne passe pas inaperçu (§9, traçabilité).
3. En tant qu'agent, je veux que chaque variante ait son propre répertoire de sortie (sources copiées, binaire, captures, empreintes, dump, rapport), pour que deux variantes ne partagent aucun chemin et puissent être relues après coup.
4. En tant qu'agent, je veux qu'une variante ne modifie jamais `src/` en place, pour que l'arbre de travail reste celui de C1 tant que C2b n'a pas commencé.
5. En tant qu'agent, je veux qu'une variante remplace exactement les occurrences de son littéral, et échoue si le nombre de remplacements diffère du nombre de sites que l'inventaire donne pour ce littéral, pour ne jamais attribuer une frame à une variante mal construite.
6. En tant qu'agent, je veux qu'un littéral n'en remplace pas un autre dont il est un préfixe textuel (`0.1` et `0.12`, `0.125`, `0.1255`), pour que la variante `0.1` ne touche que `0.1`.
7. En tant qu'agent, je veux que la valeur de remplacement soit la valeur brute du bloc A, lue dans `tools/probe_out.txt`, pour que la variante soit exactement ce que C2b écrira.
8. En tant qu'agent, je veux que le pilote de rejeu accepte un chemin de dump texte optionnel, et garde le chemin actuel quand il est absent, pour que les rejeux de variantes n'écrasent pas `tmp/pico8_replay_dump.txt`, que relisent deux tests de la suite.
9. En tant qu'agent, je veux que les rejeux de variantes se comparent aux références versionnées `<events>.frames.hash`, pour que chaque divergence se lise contre le même filet que `frames-check`.
10. En tant qu'agent, je veux une synthèse par littéral et par fichier d'événements (première frame divergente, nombre, plages, sites de l'inventaire concernés), pour rattacher la première frame divergente globale de chaque fichier à une constante.
11. En tant qu'agent, je veux qu'une variante sans aucune divergence soit consignée comme telle (« 0 frame »), et non comme un échec, pour distinguer « constante non atteinte par le filet » de « constante sans effet visible ».
12. En tant qu'agent, je veux qu'un rejeu de variante qui échoue (compilation, `abort`, capture) fasse échouer le harnais en nommant la variante, pour qu'une variante cassée ne passe jamais pour une variante sans divergence.
13. En tant qu'agent, je veux pouvoir lancer le harnais sur un seul littéral nommé, pour rejouer une variante isolée pendant l'enquête sur une frame.
14. En tant qu'agent, je veux que le harnais soit testé sur un arbre source fabriqué où une constante connue fait basculer une frame connue, et une constante exacte ne change rien, pour que « aucune divergence » ne puisse pas être vert par construction.
15. En tant qu'agent, je veux que le harnais reste hors de `make test`, comme `float-coverage` et `float-audit`, pour que la suite reste rapide ; seul son test sur cas fabriqué est dans la suite.
16. En tant qu'agent, je veux un rapport de divergence qui liste toutes les frames divergentes en plages, sans plafond, pour garder la trace complète que `frames_check.sh` tronque à 10.
17. En tant qu'agent, je veux que le rapport donne, par fichier d'événements, la première frame divergente, le nombre de frames divergentes, les plages et l'éventuel écart de longueur, pour que la sortie de phase se vérifie en le lisant.
18. En tant qu'agent, je veux que les fichiers d'événements conformes figurent aussi dans le rapport (« conforme, N frames »), pour qu'une absence de ligne ne se confonde pas avec une absence de divergence.
19. En tant qu'agent, je veux que `make frames-baseline` refuse de re-baseliner une référence divergente tant qu'aucune étape n'est nommée, pour que la trace ne puisse pas être effacée par oubli.
20. En tant qu'agent, je veux que `make frames-baseline ETAPE=<id>` écrive le rapport de cette étape **avant** de copier les références, pour que la preuve et le re-baseline tiennent dans le même commit, dans le bon ordre.
21. En tant qu'agent, je veux `make frames-report ETAPE=<id>`, qui écrit le rapport sans re-baseliner, pour le relire et le commenter avant de décider du re-baseline.
22. En tant que mainteneur, je veux un fichier de rapport par étape (C2b, D1, D2, D3, D4) dans un seul répertoire versionné, pour relire chaque étape sans fouiller l'historique git et pour que D ne réécrive pas la trace de C2b.
23. En tant qu'agent, je veux que la première pose d'une référence (fichier d'événements sans référence) ne demande pas d'étape, pour que `frames-baseline` garde son usage actuel quand rien ne diverge.
24. En tant qu'agent, je veux que `make frames-check` reste identique (sortie, code de retour, `MAX_LISTED`), pour que C2a ne change pas la porte de C1.
25. En tant qu'agent, je veux un test qui compare chaque constante `pico8_fixed_from_double(<littéral>)` du cœur à son hexa du bloc A, pour que les constantes NON COUVERT par le filet soient protégées au niveau valeur (règle de B2).
26. En tant qu'agent, je veux que ce test soit piloté par l'inventaire (les jetons `constante` de `tools/float_inventory.txt`) et par l'analyse de `tools/probe_out.txt`, sans valeur recopiée, pour que le relevé devienne une fixture (§5).
27. En tant qu'agent, je veux que la valeur C d'un littéral soit obtenue en compilant ce littéral tel qu'il est écrit dans la source, passé à la vraie fonction de conversion, pour tester ce que le cœur calcule et non une réimplémentation.
28. En tant qu'agent, je veux que ce test soit rouge aujourd'hui pour exactement les 16 constantes connues sans casser `make test`, par une liste versionnée de défaillances attendues, pour que l'état connu soit consigné et qu'une dérive dans un sens ou dans l'autre échoue.
29. En tant qu'agent, je veux que le test échoue si un littéral listé passe, pour que C2b doive vider la liste dans le commit qui corrige les constantes, et qu'une liste périmée ne masque rien.
30. En tant qu'agent, je veux que le test échoue si un littéral non listé est en écart, pour qu'une constante ajoutée ou modifiée à tort soit attrapée.
31. En tant qu'agent, je veux que le test échoue si un littéral de l'inventaire est absent du bloc A, ou si la liste nomme un littéral absent de l'inventaire, pour qu'aucune constante ne soit ignorée en silence.
32. En tant qu'agent, je veux que le test échoue s'il ne vérifie aucune constante, pour que C2b, en faisant disparaître les appels `pico8_fixed_from_double(<littéral>)`, doive rebrancher le test sur la nouvelle forme au lieu de le laisser devenir vide.
33. En tant qu'agent, je veux que le comparateur imprime une ligne par littéral (relevé, valeur C, statut, nombre de sites) et des comptes, pour que « 16 écarts, 18 ok, 34 littéraux, 61 sites » se lise sans interprétation.
34. En tant qu'agent, je veux que le harnais et le test partagent le même comparateur, pour que « les constantes à corriger » et « les constantes en écart » soient la même liste par construction.
35. En tant que mainteneur, je veux que la légende en tête du `Makefile` mentionne `frames-report`, `constant-variants`, le nouveau contrôle de `make test` et l'argument `ETAPE=` de `frames-baseline`, pour que les cibles soient trouvables.
36. En tant que mainteneur, je veux que `docs/plan-2.md` reçoive un « État de C2a » : les cibles, la table du comparateur (16/18/34/61), et le résultat d'une passe réelle du harnais sur les 16 variantes, pour que C2b parte d'un état consigné.
37. En tant qu'humain, je veux que C2a ne lance jamais l'application raylib, pour que la vérification reste entièrement headless (CLAUDE.md).

## Implementation Decisions

- **Le comparateur de constantes (module profond, partagé par (i) et (iii)).**
  - *Entrées* : l'inventaire (lignes de catégorie `constante` ; le littéral est l'extrait, dernière colonne) et `tools/probe_out.txt` (bloc A, de `## a literals` jusqu'au bloc suivant, lignes `<littéral> 0xIIII.FFFF`).
  - *Valeur C* : un script engendre, sous `tmp/`, un petit programme C qui contient, pour chaque littéral distinct, l'appel `pico8_fixed_from_double(<littéral>)` écrit **avec le texte exact de l'extrait**. Le programme est lié au vrai module fixe et imprime la valeur brute en hexa. On n'utilise pas `strtod` dans un script : le compilateur lit le même jeton que dans le cœur, ce qui ferme la question de l'équivalence des deux lectures.
  - *Sortie* : une ligne par littéral distinct (littéral, hexa relevé, hexa C, `ok`/`ECART`, nombre de sites), puis des comptes (`littéraux`, `sites`, `ok`, `ecart`). La comparaison se fait sur la valeur brute 32 bits : `0xIIII.FFFF` du relevé, la valeur rendue par le C.
  - *Erreurs* : un littéral de l'inventaire absent du bloc A, un bloc A introuvable, zéro constante. Chacune fait échouer l'outil avec un message qui nomme la cause.
  - Le comparateur ne lit que les `.c` du cœur, par l'inventaire. Il hérite donc de la définition de « cœur » de `CORE_SRCS`.

- **Le contrôle des constantes, dans `make test`.**
  - Un fichier versionné, édité à la main, liste les littéraux en défaillance attendue : un par ligne, les 16 de §3.1. On recopie des **noms** de littéraux, jamais des valeurs.
  - Le contrôle passe si l'ensemble des littéraux en `ECART` est **égal** à la liste. Sinon, il imprime séparément les listés qui passent (« liste périmée : retirer ») et les non listés en écart (« régression »). Il échoue aussi quand la liste nomme un littéral absent de l'inventaire ou quand le comparateur échoue (dont le cas « zéro constante »).
  - La cible est ajoutée à `test`, à côté de `float-inventory-check`. Le fichier C engendré et son binaire restent sous `tmp/`.
  - *Contrat pour C2b* (consigné dans l'« État de C2a ») : le commit de C2b qui corrige les constantes vide la liste. Il rebranche aussi le comparateur sur la forme retenue pour les constantes corrigées, puisque les appels `pico8_fixed_from_double(<littéral>)` disparaissent et que le contrôle échouerait sinon sur « zéro constante ». C2a ne choisit pas cette forme.

- **Le rapport de divergence (module profond, partagé par (i) et (ii)).**
  - *Script* POSIX `sh`/`awk`, frère de `tools/frames_check.sh`, qu'il ne modifie pas. Il reçoit des triplets (fichier d'événements, référence, courant) et écrit sur la sortie standard, pour chacun :
    - soit `conforme (N frames)` ;
    - soit la première frame divergente, le nombre de frames divergentes, les plages et l'écart de longueur éventuel ;
    - soit `aucune reference`.
  - *Plages* : les numéros de frame (à partir de 0, comme `frames_check.sh`) sont regroupés en intervalles `a-b`, ou `a` pour une frame isolée, séparés par des espaces, sans plafond. Une frame présente d'un seul côté (écart de longueur) compte comme divergente et entre dans les plages. Ainsi, la troncature d'un rejeu est visible dans les plages.
  - *Sortie* déterministe : même entrée, même octets. Pas de date ni de chemin absolu. Un en-tête nomme l'étape quand elle est fournie.
  - Il rend toujours 0 quand il a pu lire ses entrées : c'est un rapport, pas une porte. Un fichier courant absent est une erreur.

- **Les rapports versionnés et `frames-baseline`.**
  - Un répertoire versionné sous `tools/` reçoit **un fichier par étape**, nommé par l'étape (`C2b`, `D1`… ; ce que `ETAPE=` reçoit). On l'a préféré à un fichier unique réécrit à chaque étape, parce que chaque rapport reste lisible à côté des autres et que D ne réécrit pas la preuve de C2b.
  - `make frames-report ETAPE=<id>` capture les empreintes courantes (mêmes prérequis que `frames-baseline`), écrit à côté puis renomme le rapport de l'étape. Sans `ETAPE=`, la cible échoue en le demandant.
  - `make frames-baseline` : avant de copier, la cible compare chaque référence existante à la capture courante. Si au moins une diverge et que `ETAPE=` est absent, elle échoue sans rien copier, en nommant les fichiers divergents et en indiquant `ETAPE=`. Avec `ETAPE=`, elle écrit le rapport de l'étape puis copie. Quand rien ne diverge ou qu'aucune référence n'existe, le comportement ne change pas et aucun rapport n'est exigé. Réécrire le rapport d'une étape déjà présente est permis (on reprend l'étape) ; l'historique git garde l'ancien.
  - `make frames-check` n'est pas touché.

- **Le pilote de rejeu : un chemin de dump optionnel.** `main.c` accepte un troisième argument optionnel, le chemin du dump texte. Absent, le chemin reste `tmp/pico8_replay_dump.txt`, et tous les appelants actuels (Makefile, tests) sont inchangés. On l'a préféré à deux autres options :
  - lancer les variantes dans un autre répertoire courant : le pilote lit ses assets par chemin relatif (`pico8/gfx.json`), ce qui imposerait des liens symboliques cachés ;
  - réécrire le chemin dans la copie de `main.c` : ce serait un deuxième remplacement textuel, sans test.

  `main.c` est un outil d'hôte hors du cœur (§4) : ce changement ne touche ni une constante ni l'inventaire. La règle `$(FRAMES_DIR)/%.bin` du `Makefile` et son commentaire sur `make -j` restent tels quels (voir Further Notes).

- **Le harnais d'attribution.**
  - *Forme d'une variante : remplacement textuel sur une copie.* On a écarté deux autres formes :
    - un `-D` de surcharge, parce qu'il exigerait un point d'accroche dans `src/`, donc une édition du cœur ;
    - un patch versionné, fragile aux numéros de ligne et redondant avec l'inventaire.

    Le harnais copie les `.c`/`.h` de `src/` dans `<sortie>/<variante>/src/`. Il remplace, dans la copie seulement, chaque occurrence exacte du jeton `pico8_fixed_from_double(<littéral>)` par un cast de la valeur brute du relevé en `pico8_fixed_t`. La correspondance est exacte sur la parenthèse fermante, donc `0.1` ne touche pas `0.12`. Il vérifie ensuite que le nombre de remplacements égale le nombre de sites de l'inventaire pour ce littéral, et qu'il n'y en a aucun dans un fichier que l'inventaire ne nomme pas. En cas d'écart, la variante échoue.
  - *Nom d'une variante* : dérivé du littéral de façon réversible et sûre pour un chemin (par exemple `0.27` → `lit-0.27`). Sortie par défaut : un répertoire dédié sous `tmp/`, effacé et recréé pour chaque variante rejouée, jamais partagé.
  - *Compilation* : le pilote de rejeu est compilé depuis la copie, avec les mêmes `CFLAGS` et la même liste de sources que `pico8_replay` (`main.c`, `pico8_screencast.c`, `ENGINE_SRCS`). Le `Makefile` passe cette liste au script. On compile tout l'arbre de la copie, sans réutiliser `tmp/obj/`, au prix du temps : c'est plus simple et on ne mélange aucun objet de deux arbres différents (règle « un arbre d'objets par jeu de flags »).
  - *Rejeu* : pour chaque fichier de `FRAMES_EVENTS`, le binaire de la variante est lancé depuis la racine du dépôt avec une capture, et un dump dans le répertoire de la variante. Les empreintes sont calculées par l'outil `pico8_frames` existant (`hashes`). Le rapport de divergence de la variante contre les références `<events>.frames.hash` est écrit dans son répertoire.
  - *Synthèse* : un fichier de synthèse dans le répertoire de sortie. Pour chaque variante et chaque fichier d'événements, il donne la première frame divergente, le nombre et les plages ; pour chaque variante, les sites de l'inventaire (fichier, fonction, rang) que la variante a touchés. Il n'est pas versionné par C2a : C2b décide de ce qu'il en consigne.
  - *Sélection* : par défaut, les littéraux en `ECART` du comparateur. Un littéral nommé (`LITTERAL=`) restreint la passe à cette variante, qu'il soit en écart ou non. Une variante d'un littéral exact est permise, et elle doit alors donner 0 divergence.
  - *Paramètres du script*, pour que le test puisse l'appeler sur un arbre fabriqué : racine des sources, liste des sources à compiler, inventaire, relevé, outil d'empreintes, triplets (événements, référence), répertoire de sortie, littéral facultatif. `make constant-variants` passe les vraies valeurs.
  - *Échecs* : une compilation, un rejeu (code non nul, `abort`) ou une capture qui échoue fait échouer la passe en nommant la variante et le fichier d'événements. Une variante sans divergence n'est pas un échec.
  - Hors de `make test`, déclarée `.PHONY`, avec une ligne dans la légende. Le coût attendu est de plusieurs minutes pour 16 variantes (16 compilations complètes et 16 × 8 rejeux, dont les 9 729 frames d'`events.json`). On l'accepte : le harnais sert une fois par étape divergente.

- **Consignation.** Un « État de C2a » est ajouté à `docs/plan-2.md`, sur le modèle des états précédents, avec :
  - les cibles et le répertoire des rapports ;
  - les comptes du comparateur ;
  - la liste des défaillances attendues et le contrat C2b ;
  - pour chaque variante réelle, la première frame divergente et le nombre de frames divergentes par fichier d'événements, ou « 0 », lus dans la synthèse et non recopiés à la main.

  Ce dernier point n'est pas une attribution de C2b : c'est la preuve que l'outil a tourné sur le vrai filet.

- **Commentaires** selon `CLAUDE.md` : le contrat des scripts (codes de sortie ; le rapport rend 0), le piège du remplacement par préfixe, l'aide à la lecture des commandes du `Makefile`. Le récit va dans les messages de commit.

## Implementation Chunks

1. **Le comparateur de constantes et son contrôle.** Le générateur du programme C, l'extraction du bloc A, la table, le fichier des 16 défaillances attendues, la cible ajoutée à `make test`, et le test du contrôle sur entrées fabriquées. Acceptation : `make test` vert ; le comparateur imprime 34 littéraux, 61 sites, 16 écarts, 18 ok, et les 16 écarts sont ceux de §3.1 ; retirer un littéral de la liste, ou en ajouter un exact, fait échouer le contrôle (vérifié par le test fabriqué, pas en modifiant la liste réelle) ; `frames-check` identique.
2. **Le rapport de divergence, `frames-report` et la garde de `frames-baseline`.** Le script, sa cible, le répertoire versionné (vide ou avec un fichier d'amorce, au choix de l'implémenteur), la garde de `frames-baseline`, et le test du script sur empreintes fabriquées. Acceptation : `make test` vert ; sur l'arbre actuel, `make frames-report ETAPE=essai` écrit un rapport « conforme » pour les 8 fichiers, puis le fichier d'essai est retiré avant le commit ; `frames-baseline` sans `ETAPE=` passe quand rien ne diverge ; `frames-check` identique.
3. **Le chemin de dump optionnel du pilote de rejeu.** L'argument, et un test qui vérifie que le dump va au chemin demandé et que, sans l'argument, le comportement est celui d'avant (dans le test existant du pilote headless ou à côté). Acceptation : `make test` vert, `frames-check` identique.
4. **Le harnais d'attribution.** Le script, la cible `constant-variants`, la légende, et le test sur arbre fabriqué. Acceptation : le test fabriqué est vert dans `make test` ; une passe réelle `make constant-variants` aboutit sur les 16 variantes sans échec de construction et produit la synthèse ; `src/` est inchangé après la passe (`git status` propre sous `src/`).
5. **Consignation.** L'« État de C2a » dans `docs/plan-2.md`, avec les chiffres de la passe réelle. Acceptation : les chiffres sont ceux de la synthèse et du comparateur.

## Testing Decisions

- Un bon test porte sur ce que l'outil rend (une table, un rapport, un code de sortie, un fichier écrit) à partir d'entrées qu'il fabrique. Il exerce d'abord ce que l'outil doit empêcher. Il n'inspecte ni les expressions `awk` ni les fichiers intermédiaires. Précédents : `tests/test_frames_check.c`, `tests/test_float_inventory_check.c`, `tests/test_float_observe.c`, `tests/test_float_audit.c`, qui lancent un script sur des fichiers fabriqués sous `tmp/`.
- **Comparateur et contrôle** (nouveau test dans `unit`), sur un inventaire et un relevé fabriqués, avec le vrai module fixe :
  - un littéral exact (`0.5`) donne `ok` ;
  - un littéral décalé (`0.27`, relevé `0x0000.451e`) donne `ECART` ;
  - la liste égale aux écarts donne un code 0 ;
  - un listé qui passe donne un échec « périmée » ;
  - un écart non listé donne un échec « régression » ;
  - un littéral absent du bloc A donne une erreur ;
  - un listé absent de l'inventaire donne une erreur ;
  - un inventaire sans constante donne une erreur ;
  - un bloc A suivi d'un autre bloc : les lignes du bloc suivant ne sont pas lues comme des littéraux ;
  - deux sites d'un même littéral sont comptés deux fois dans `sites` et une fois dans `littéraux`.
- **Rapport de divergence** (nouveau test dans `unit`), sur des empreintes fabriquées :
  - un fichier conforme ;
  - une frame isolée, qui donne une plage `a` ;
  - des frames contiguës, qui donnent `a-b` ;
  - plusieurs plages ;
  - plus de 10 frames divergentes, toutes listées (le cas que `frames_check.sh` tronque) ;
  - un courant plus court et un courant plus long que la référence ;
  - une référence absente ;
  - la sortie identique d'une exécution à l'autre.
- **Garde de `frames-baseline`** : pas de test automatique de la cible `make` elle-même, faute de précédent pour tester une cible qui réécrit des fichiers versionnés. La logique « divergence sans étape → refus » vit dans le script (un mode ou un code de sortie dédié) et se teste là. La cible est vérifiée à la main pendant le chunk 2, avec le résultat consigné dans le message de commit.
- **Chemin de dump** : extension de `tests/test_pico8_headless_capture.c`, ou test voisin sur son patron. Avec l'argument, le dump est écrit au chemin donné et `tmp/pico8_replay_dump.txt` n'est pas touché (vérifié par date ou contenu témoin). Sans l'argument, le comportement est inchangé.
- **Harnais** (nouveau test dans `unit`), sur un arbre fabriqué sous `tmp/` :
  - un pilote minuscule de même interface (événements, capture, dump) qui écrit quelques frames au format de capture existant (lié à `pico8_screencast.c`, `pico8_mem.c` et au vrai `pico8_fixed.c`) ;
  - la frame K n'a un pixel différent que si `pico8_fixed_from_double(0.27)` vaut la valeur tronquée ;
  - l'arbre contient aussi un `pico8_fixed_from_double(0.5)` et un `pico8_fixed_from_double(0.125)`, pour le piège de préfixe ;
  - un inventaire fabriqué, et des références calculées par le test sur l'arbre non modifié.

  Assertions :
  - la variante `0.27` diverge exactement sur la frame K (première = K, nombre = 1) ;
  - la variante `0.5` ne diverge sur aucune frame ;
  - l'arbre fabriqué d'origine est intact après la passe ;
  - chaque variante a écrit ses fichiers dans son propre répertoire, sans rien écrire hors du répertoire de sortie ;
  - un inventaire qui annonce deux sites pour `0.27` alors que l'arbre n'en a qu'un fait échouer la variante ;
  - un pilote fabriqué qui échoue fait échouer la passe en nommant la variante.
- La passe réelle du harnais n'est pas dans `make test`. On l'accepte ainsi : elle aboutit une fois au chunk 4, et ses chiffres sont consignés au chunk 5.
- `make test` complet vert à chaque chunk, avec `frames-check` identique sur les 8 fichiers et **aucun `frames-baseline`** pendant C2a.

## Out of Scope

- **Toute modification d'une constante dans `src/`**, y compris le choix de la forme des constantes corrigées : C2b.
- **L'attribution elle-même** : rattacher la première frame divergente globale à une constante et étiqueter la cascade, versionner la synthèse du harnais, vider la liste des défaillances attendues, re-baseliner. Tout cela relève de C2b.
- **Des variantes par site** (un seul des sites d'un littéral) ou une variante « toutes les constantes ensemble » : non demandées. Le grain est le littéral, soit une ligne du relevé.
- **La modification de `tools/frames_check.sh`**, de `MAX_LISTED` ou de la sortie de `frames-check`.
- **La suppression du chemin fixe du dump dans la règle de capture du filet** et du commentaire `make -j` associé.
- **Les casts D1 et les 17 « à auditer »**, les réécrits (D2, D3), les clés de table (D4), `sin`/`cos`/`rnd` et les définitions `from_double`/`to_double` (décision ouverte).
- **Le bloc C `tostr`** et sa relance d'une ligne.
- **Le volet `malloc`** (§8).
- **Lancer l'application raylib.**

## Further Notes

- Le calcul fait en rédigeant ce PRD, arrondi au plus proche contre le relevé, donne bien 16 écarts sur les 34 littéraux distincts, et les 34 figurent dans le bloc A. Le comparateur doit retrouver ce chiffre ; s'il ne le retrouve pas, c'est un constat à consigner, pas une raison d'ajuster la liste.
- Sur les 61 sites `constante`, 23 sont NON COUVERT (`tools/float_coverage.txt`). Les variantes dont tous les sites sont non couverts donneront 0 divergence. Pour elles, le contrôle (iii) est la seule protection, et c'est ce que le plan lui demande.
- Un littéral peut avoir plusieurs sites dans plusieurs fonctions (`0.1` en a 5). La synthèse les nomme, mais l'attribution reste au grain du littéral. Si C2b a besoin de séparer deux sites d'un même littéral, le harnais pourra accepter une clé de site ; ce n'est pas demandé ici.
- Les références `<events>.frames.hash` sont celles de C1, identiques à la Phase B. Le harnais les lit sans jamais les écrire.
- La règle de capture du filet écrit encore le dump au chemin fixe, d'où le commentaire sur `make -j` dans le `Makefile`. Le nouvel argument permettrait de la corriger ; C2a ne le fait pas (changement chirurgical), et le constat est mentionné ici seulement.
