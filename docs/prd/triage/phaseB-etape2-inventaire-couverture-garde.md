---
title: "Plan 2, Phase B2 : inventaire des sites flottants, couverture sous le filet, garde -mgeneral-regs-only"
description: "Avant la Phase C, il faut une liste des sites flottants du cœur qui se régénère au lieu de numéros de ligne qui dérivent, le nombre de passages du filet sur chacun, et une garde de compilation dont on a constaté l'échec sur le code actuel."
status: needs-triage
---

## Problem Statement

`docs/plan-2.md` §3 décrit les sites flottants du cœur porté par des comptes (61 constantes, 76 casts, 5 sites à réécrire) et des numéros de ligne cités en prose. Ces numéros ont déjà dérivé : le plan situe le `pow` en `economy_formulas.c:196-197`, il est aujourd'hui en ligne 170. Les Phases C et D doivent énoncer leur sortie en « compte de la catégorie X = 0 », mais aucun outil ne produit ces comptes. On ne peut les relire que dans une prose qui vieillit.

B1 a posé le filet : les empreintes par frame des 8 fichiers d'événements de `FRAMES_EVENTS`. Il ne dit pas **quels sites** ce filet exerce. Une mesure faite sur un build de couverture jetable (grep des motifs flottants du cœur, rejeu des fichiers d'événements) a donné 141 lignes concernées. L'`events.json` racine seul en atteint 80, la racine plus les fixtures 103, et **26 ne sont atteintes par rien**. Si la Phase C transforme l'un de ces 26 sites, `frames.hash` restera identique quoi qu'elle fasse. Le critère §6.4 passerait alors sans rien avoir protégé.

Enfin, §6.3 fait de `-mgeneral-regs-only` le seul critère qui prouve l'objectif PSX. Sur cet hôte, `nm` ne voit aucun symbole soft-float, et ce avant tout changement. Mais la garde n'a jamais été posée, et personne n'a constaté qu'elle échoue aujourd'hui. Or un critère qu'on n'a jamais vu échouer ne prouve rien.

## Solution

Trois outils versionnés, tous lancés par des cibles `make` hors de la suite. Chacun écrit un rapport texte versionné, et le relancer sur le même arbre produit le même rapport, à l'octet près :

1. **Inventaire.** Un script lit les sources du cœur et émet une ligne par site flottant. Chaque site porte une identité stable qui ne dépend pas du numéro de ligne, et une catégorie déterminée par une règle mécanique tirée du plan. Le rapport se termine par les comptes par catégorie. Un site que le plan ne permet pas de classer mécaniquement est marqué « à auditer », jamais deviné.
2. **Couverture.** Un build `--coverage` du pilote de rejeu repasse les 8 fichiers du filet. Le rapport joint l'inventaire aux compteurs `gcov` et donne pour chaque site son nombre de passages. Un site jamais atteint sort en `NON COUVERT`, jamais en silence.
3. **Garde.** Chaque objet du cœur est compilé avec `-mgeneral-regs-only`, sans édition de liens. Le rapport liste les fichiers en échec. La cible sort en échec tant que la liste n'est pas vide. C'est l'état attendu jusqu'à la Phase E, et il est consigné dans le plan.

`make test` ne lance aucun de ces trois diagnostics. Il lance en revanche les tests des trois scripts sur des sources et des sorties `gcov` fabriquées, comme `test_frames_check` teste `frames_check.sh`.

## User Stories

1. En tant qu'agent qui ouvre la Phase C, je veux régénérer la liste des sites flottants du cœur par une seule commande, pour ne pas travailler sur des numéros de ligne périmés.
2. En tant qu'agent, je veux que chaque site ait une identité (fichier, fonction englobante, catégorie, rang dans la fonction) qui survive à un ajout de lignes plus haut dans le fichier, pour qu'un diff de l'inventaire montre les sites réellement ajoutés ou retirés et pas des lignes décalées.
3. En tant qu'agent, je veux que le numéro de ligne figure quand même dans chaque entrée, pour aller au site depuis le rapport, sans qu'il serve d'identité.
4. En tant qu'agent, je veux que les 61 appels `pico8_fixed_from_double(<littéral>)` forment la catégorie « constante », avec le littéral dans l'entrée, pour que C2 retrouve dans le rapport chaque littéral à remplacer par la valeur de `tools/probe_out.txt`.
5. En tant qu'agent, je veux que les 76 `(int)pico8_fixed_to_double(…)` forment la catégorie « cast », pour que C1 et D1 partent de la liste exacte.
6. En tant qu'agent, je veux que chaque cast porte une population (a), (b) ou (c) uniquement si une règle du plan permet de la déduire du code. Faute de telle règle, il porte « à auditer ». Ainsi l'inventaire n'affirme pas une preuve de signe ou une correspondance Lua que personne n'a faite.
7. En tant qu'agent, je veux que les 5 sites à réécrire (`pow`, `from_double` sur une expression non littérale, `%.4f`, `sin` et `cos` de la libc) forment chacun une entrée de la catégorie « réécrit », pour que D2 et D3 les retrouvent par leur nature et non par leur ligne.
8. En tant qu'agent, je veux que tout autre jeton flottant du cœur (mot-clé `double`/`float`, `pico8_fixed_to_double` hors cast `(int)`, définitions de `pico8_fixed.c`) tombe dans une catégorie « autre, à auditer », pour que l'inventaire couvre tout ce que voit le grep du §6.1 et n'en laisse rien sortir.
9. En tant qu'agent, je veux que chaque jeton flottant du cœur apparaisse dans exactement une catégorie, pour que la somme des comptes vaille le nombre total de jetons et qu'un trou de classification se voie.
10. En tant qu'agent, je veux que l'inventaire se termine par une ligne de compte par catégorie, pour énoncer la sortie d'une phase en « compte de X = 0 » d'après le rapport.
11. En tant qu'agent, je veux que l'inventaire ne lise que les sources du cœur tel que §4 le définit, pour que les outils d'hôte, qui gardent la libc par décision du plan, ne gonflent pas les comptes.
12. En tant qu'agent, je veux que la liste des sources du cœur soit définie une seule fois dans le `Makefile`, à partir de la closure du moteur moins les outils d'hôte, et que l'inventaire, la couverture et la garde s'en servent tous trois, pour que les trois outils parlent du même périmètre.
13. En tant qu'agent, je veux que l'inventaire soit versionné, pour qu'un commit de Phase C montre dans son diff les sites qu'il fait disparaître.
14. En tant qu'agent, je veux que lancer deux fois l'inventaire sur le même arbre donne deux fichiers identiques, pour que « reproductible » soit vérifié et non supposé.
15. En tant qu'agent, je veux un build de couverture qui ait son propre arbre d'objets, pour qu'un objet instrumenté ne se mélange pas à ceux de la suite, conformément à la règle « un arbre d'objets par jeu de flags » du `Makefile`.
16. En tant qu'agent, je veux que le build de couverture rejoue exactement les fichiers de `FRAMES_EVENTS`, sans en recopier la liste, pour que « couvert » veuille dire « couvert par le filet que `frames-check` compare ».
17. En tant qu'agent, je veux que les compteurs de couverture soient remis à zéro avant chaque passe, pour qu'une passe précédente ne gonfle pas le rapport.
18. En tant qu'agent, je veux voir pour chaque site de l'inventaire son nombre de passages sous le filet, pour savoir avant de toucher un site si `frames.hash` le protège.
19. En tant qu'agent, je veux qu'un site jamais atteint sorte en `NON COUVERT`, pour appliquer la règle « jamais un passage silencieux » du plan : test unitaire au niveau valeur, fixture scriptée, ou entrée explicite « non protégé par le filet ».
20. En tant qu'agent, je veux que le rapport de couverture se termine par le nombre de sites couverts et non couverts, par catégorie, pour que le début de chaque phase compare ces nombres à ceux de la phase précédente.
21. En tant qu'agent, je veux que le rapport de couverture soit versionné et reproductible sur une même chaîne de compilation, pour que la relance en début de phase se lise comme un diff.
22. En tant qu'agent, je veux que le rapport de couverture nomme la version du compilateur qui l'a produit, pour qu'un diff causé par un changement de `gcov` ne se lise pas comme un changement de couverture.
23. En tant qu'agent, je veux une cible de garde qui compile chaque source du cœur avec `-mgeneral-regs-only`, sans jamais lier ni exécuter le résultat, pour poser le critère §6.3 sur le code actuel.
24. En tant qu'agent, je veux que la garde liste tous les fichiers en échec et non s'arrêter au premier, pour que la liste consignée soit complète.
25. En tant qu'agent, je veux que la liste des fichiers en échec soit un fichier versionné, pour que « elle échoue sur le code actuel » soit consigné et daté par un commit, et que sa décroissance en Phases C à E se lise dans l'historique.
26. En tant qu'agent, je veux que la cible de garde sorte en échec tant que cette liste n'est pas vide, pour que la Phase E n'ait rien à changer à la cible : elle passe le jour où le cœur n'a plus de flottant.
27. En tant qu'agent, je veux que `make test` reste vert malgré l'échec de la garde, pour que la suite ne soit pas rouge pendant toutes les Phases C et D.
28. En tant qu'agent, je veux que le plan consigne que la garde est aveugle, sur cet hôte x86_64, aux appels `pico8_fixed_from_double(<littéral>)`. GCC y passe l'argument `double` sur la pile sans erreur ; mesuré : `pico8_galaxy.c`, `pico8_geom.c`, `pico8_vec.c`, `pico8_slipway.c`, `pico8_wormhole.c`, `pico8_ob.c`, `pico8_game_data.c` et `pico8_entity_geom.c` compilent sous la garde alors qu'ils contiennent des constantes. Ainsi personne ne prendra la liste de la garde pour l'inventaire, et §6.3 ne sera pas cru plus fort qu'il ne l'est ici.
29. En tant que mainteneur, je veux que chaque script soit testé sur des entrées fabriquées qui exercent ce qu'il doit empêcher (un jeton non classé, un site non couvert, un fichier flottant qui passerait la garde), pour qu'une régression des outils ne rende pas le filet aveugle.
30. En tant que mainteneur, je veux que les trois cibles figurent dans la légende en tête du `Makefile`, avec la mention « hors suite » comme pour `ref-audit`, pour savoir lesquelles `make test` lance.
31. En tant qu'agent qui mènera C et D, je veux que la « Règle pour C et D, issue de B2 » puisse s'appliquer telle quelle : lister les sites touchés avec leur statut de couverture en citant le rapport de couverture, sans nouvel outil.

## Implementation Decisions

- **Périmètre du cœur.** Une seule variable du `Makefile` définit les sources du cœur : la closure du moteur (`ENGINE_SRCS`) moins les outils d'hôte que §4 exclut (`pico8_assets.c`, `pico8_dump.c`). Ces deux fichiers ne contiennent aujourd'hui aucun site flottant, mais §4 les exclut par principe. Les trois outils prennent cette liste en argument au lieu d'avoir chacun la leur. `pico8_audio.c` reste dans le cœur : il est atteignable depuis `pico8__update60`, même si l'audio est hors périmètre fonctionnel.
- **Langage des scripts.** POSIX `sh` et `awk`, comme `tools/frames_check.sh` et `tools/ref_audit_report.sh`. Aucune dépendance nouvelle.
- **Unité d'inventaire.** Une occurrence de jeton, pas une ligne. `pico8_layout.c` a deux casts sur une même ligne, qui font deux sites.
- **Identité d'un site.** `fichier`, `fonction englobante`, `catégorie`, `rang de l'occurrence parmi celles de même catégorie dans cette fonction`. On ajoute l'extrait du jeton (le littéral pour une constante, l'argument pour un cast). Le numéro de ligne est imprimé à part et ne sert jamais de clé. Pour trouver la fonction englobante, il suffit du style du dépôt (définition de fonction en colonne 0 terminée par `{`). Aucun parseur C.
- **Règles de catégorie, tirées du plan et d'aucune autre source :**
  - *constante* : `pico8_fixed_from_double(` suivi d'un littéral numérique, puis `)` ;
  - *cast* : `(int)` suivi de `pico8_fixed_to_double(` ;
  - *réécrit* : `pow(`, `sin(` et `cos(` de la libc (pas `pico8_fixed_sin`/`cos`), la chaîne de format `%.4f`, et `pico8_fixed_from_double(` sur une expression non littérale ;
  - *autre, à auditer* : tout autre jeton du motif flottant (`\b(double|float)\b`, `pico8_fixed_to_double` hors cast, `pico8_fixed_from_double` hors des deux règles ci-dessus).
- **Populations (a)/(b)/(c).** L'inventaire imprime la colonne population, mais tous les casts y portent « à auditer ». Le plan ne donne aucune règle dérivable du code seul : (a) exige la correspondance avec un `flr(` du Lua, et le C n'a aucun lien vers `lua.lua`. (b) exige une preuve de signe par site. (c) se définit comme le reste de (a) et (b). Les comptes (a), (b) et (c) valent donc 0 en B2, et le rapport dit pourquoi. B2 n'introduit pas de fichier d'annotations : c'est à la Phase C d'en décider, quand elle fera les audits. Une fois qu'un cast est transformé, il sort de l'inventaire : le compte « cast » décroît de lui-même.
- **Contrôles internes de l'inventaire.** Le script s'arrête en erreur si une ligne du cœur correspond au motif flottant large et qu'aucune catégorie ne la prend. La somme des catégories doit valoir le nombre de jetons trouvés.
- **Valeurs attendues sur le code actuel** (contrôle d'acceptation, pas assertion figée dans un test) : 61 constantes, 76 casts, 5 réécrits. Si un compte diffère, la règle est fausse ou le plan l'est. On tranche en relisant les sites, pas en ajustant la règle pour retomber sur le nombre.
- **Build de couverture.** Il a son propre arbre d'objets (`--coverage`, sans optimisation) et son propre binaire de rejeu, lié aux mêmes sources que `pico8_replay`. Il rejoue chaque entrée de `FRAMES_EVENTS` à partir de la variable elle-même. Les `.gcda` sont effacés avant la passe. `gcov` est lancé sur les sources du cœur. Le script de jointure lit les sorties `gcov` et l'inventaire, et écrit pour chaque site son compte de passages, ou `NON COUVERT`.
- **Granularité de la couverture.** C'est la ligne, pas l'expression. Deux sites sur une même ligne reçoivent le même compte. Un site dans une branche de ternaire (`pico8_audio.c`) reçoit le compte de la ligne, même si la branche flottante n'est jamais prise. Cette limite est écrite dans l'en-tête du rapport. B2 ne tente pas d'y remédier par la couverture de branches.
- **Garde.** C'est une cible `make` dédiée. Elle compile chaque source du cœur avec les `CFLAGS` du dépôt plus `-mgeneral-regs-only`, vers une sortie jetable. Elle ne lie jamais : sur x86_64, ces objets ont une ABI incohérente (arguments `double` passés sur la pile). Elle écrit la liste triée des fichiers en échec dans un fichier versionné et sort en échec si la liste n'est pas vide. Aujourd'hui, mesuré avec `cc` (GCC 13.3), 32 fichiers échouent. Il n'y a pas de cliquet dans `make test` : le plan demande de relancer la garde en début de chaque phase, pas à chaque commit, et le diff du fichier versionné suffit à voir la liste bouger.
- **Consignation dans le plan.** Dans `docs/plan-2.md`, la Phase B reçoit l'état de B2 : les comptes par catégorie, les comptes couverts et non couverts, le nombre de fichiers en échec sous la garde, et l'angle mort x86_64 de la garde sur les constantes (story 28). On y renvoie aux rapports versionnés plutôt que de recopier les listes. Les numéros de ligne cités en §3 ne sont pas réécrits : c'est le rôle de l'inventaire de les remplacer, et le plan y renvoie.
- **Aucun site flottant n'est modifié.**

## Implementation Chunks

1. **Périmètre et inventaire.** Variable du cœur dans le `Makefile`, script d'inventaire, cible, rapport versionné, et son test sur des sources fabriquées. Contrôle d'acceptation : 61/76/5 sur le code actuel, deux exécutions identiques.
2. **Couverture.** Arbre d'objets `--coverage`, binaire de rejeu instrumenté, cible, script de jointure inventaire × `gcov`, rapport versionné, et le test de la jointure sur une sortie `gcov` fabriquée. Il dépend du chunk 1, dont il consomme l'inventaire.
3. **Garde et consignation.** Cible `-mgeneral-regs-only`, liste versionnée des fichiers en échec, test du script sur deux sources fabriquées (l'une flottante, l'autre entière), légende du `Makefile`, mise à jour de `docs/plan-2.md`. Il dépend du chunk 1 pour la variable du cœur.

## Testing Decisions

- Un bon test porte sur ce que le script rend (le rapport, le code de sortie) à partir d'entrées qu'il fabrique. Il n'inspecte ni les expressions `awk` ni les fichiers intermédiaires. Chaque test exerce d'abord ce que l'outil doit empêcher : un jeton qui échapperait à toute catégorie, un site non atteint qui passerait pour couvert, un fichier flottant que la garde laisserait passer.
- **Inventaire** : une petite source C fabriquée contenant un spécimen de chaque catégorie. On y met deux casts sur une même ligne, deux fonctions qui ont chacune une constante, un `pico8_fixed_sin` (qui ne doit pas être pris pour `sin`) et un `double` isolé. Les assertions portent sur les catégories et les identités émises. Puis on ajoute des lignes vides en tête de la source : les identités doivent rester les mêmes et seuls les numéros de ligne changer. Enfin, deux exécutions doivent donner une sortie identique.
- **Jointure de couverture** : une sortie `gcov` fabriquée (le format texte `compte:ligne:source`, avec `#####` pour une ligne jamais exécutée) et un inventaire fabriqué. On vérifie les comptes rapportés, qu'une ligne `#####` sort en `NON COUVERT`, et qu'un site dont la ligne manque dans `gcov` sort aussi en `NON COUVERT` plutôt que de disparaître. Le build de couverture réel n'entre pas dans `make test` (coût, et un compteur n'est pas un résultat de test). Son caractère reproductible se contrôle à l'acceptation en le lançant deux fois.
- **Garde** : deux sources fabriquées, l'une qui renvoie un `double`, l'autre purement entière. Le script doit lister la première seule et sortir en échec, puis sortir en succès avec seulement la seconde.
- Ces trois tests entrent dans `unit`, donc dans `make test`. Les trois diagnostics réels en restent dehors.
- **Précédents** : `tests/test_frames_check.c`, un test C qui pilote un script `sh` sur des fichiers fabriqués et lit son journal ; `tools/ref_audit_report.sh`, pour la jointure « inventaire statique × passage dynamique » avec son `NOT COVERED`.

## Out of Scope

- Toute modification d'un site flottant, d'une constante ou d'un cast. Tout cela appartient aux Phases C et D.
- Le classement réel des casts en (a)/(b)/(c) : la correspondance avec les `flr(` du Lua, les preuves de signe, l'audit des clés de table fractionnaires. C'est le travail de la Phase C (et de D4 pour les clés).
- Le build d'audit C1 (égalité ancienne et nouvelle expression à chaque site).
- L'écriture de tests ou de fixtures pour les sites `NON COUVERT`. La règle issue de B2 s'applique site par site pendant les phases C et D. B2 se borne à rendre la liste disponible.
- Une garde en `mipsel` PSn00bSDK soft-float, qui fermerait l'angle mort x86_64. On la consigne comme alternative (§6.3 la nomme déjà), sans la poser.
- Le volet `malloc` (§8).
- `pico8_fixed_sin`/`cos`/`rnd` (clôture du §5). Le `sin`/`cos` de la libc qu'ils appellent figure dans l'inventaire comme site « réécrit », puisque §3 le compte parmi les cinq, mais B2 n'y touche pas.
- Lancer l'application raylib.

## Further Notes

- La mesure de l'orchestrateur (141 lignes, dont 26 atteintes par rien) date d'un build jetable qui travaillait par ligne. L'inventaire travaille par jeton, donc son total diffère de 141. C'est attendu : le rapport de B2 remplace ce chiffre, il n'a pas à le retrouver.
- Le §3.3 dit déjà « `nm` était déjà vide avant tout changement ». La mesure de B2 ajoute que `-mgeneral-regs-only` n'est pas étanche non plus sur x86_64 : il rattrape les retours et opérations `double`, pas les arguments `double` passés à une fonction externe. §6.3 reste le bon critère pour les 32 fichiers qu'il voit. L'inventaire, lui, voit tout.
