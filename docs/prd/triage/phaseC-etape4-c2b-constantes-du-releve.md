---
title: "Plan 2, Phase C2b : les 61 constantes recopiées du relevé, les 16 corrections attribuées et re-baselinées"
description: "Les 61 appels pico8_fixed_from_double(<littéral>) du cœur deviennent des entiers 16.16 bruts recopiés de tools/probe_out.txt, ce qui corrige les 16 constantes décalées d'un LSB. La forme change d'abord, sans changer de valeur, puis les valeurs changent dans un commit séparé, avec la première frame divergente de chaque rejeu rattachée à une constante et un rapport versionné avant le re-baseline."
status: needs-triage
---

## Problem Statement

Le relevé PICO-8 (`tools/probe_out.txt`, bloc A) a montré que PICO-8 **tronque** les littéraux décimaux (`floor(v × 65536)`). Or le cœur C les convertit à l'exécution par `pico8_fixed_from_double`, qui arrondit au plus proche. **16 des 34 littéraux distincts** en sortent décalés d'un LSB : `0.001`, `0.0032`, `0.005`, `0.031`, `0.05`, `0.08`, `0.0977`, `0.1`, `0.19`, `0.27`, `0.55`, `0.6`, `0.8`, `0.85`, `0.99`, `1.08`. C'est le premier bug trouvé par ce chantier (§3.1, décision 12).

C2a a livré l'outillage, et ce bug reste entier :

1. **Le bug est toujours là.** `make constants-check` passe parce que les 16 écarts figurent dans `tools/constants_expected_ecarts.txt`, une liste de défaillances **attendues**. Le port calcule encore `0.27` en 17695 au lieu de 17694.
2. **La valeur est dérivée, pas recopiée.** Même les 18 constantes justes le sont « par hasard » (§3.1) : elles passent par une règle d'arrondi, et c'est précisément ce que §9 interdit (« une constante vient du relevé, jamais d'une règle d'arrondi supposée »). Le plan dit « les 34 constantes remplacées par les entiers de `tools/probe_out.txt` », et la table de propriété affecte les **61** jetons `constante` à C2b.
3. **Ce sont 61 jetons flottants du cœur.** Tant qu'ils existent, l'inventaire compte 61 constantes, et les fichiers du cœur passent un `double` à une fonction externe, un cas que la garde `-mgeneral-regs-only` ne voit pas sur x86_64 (angle mort de B2).
4. **La correction fera diverger le filet**, et la sortie de la Phase C exige que la **première** frame divergente de chaque fichier d'événements soit rattachée à **une** constante du relevé, le reste étiqueté cascade, avec un rapport de divergence versionné dans le commit. La passe de C2a (`make constant-variants`) a produit ces chiffres, mais sa synthèse est restée sous `tmp/`, non versionnée. Rien de versionné ne permet encore de vérifier une attribution.
5. **Le contrôle des constantes deviendrait vide.** Le contrat de C2a le dit : une fois les appels `pico8_fixed_from_double(<littéral>)` retirés, `constants-check` échouerait sur « zéro constante ». Il doit être rebranché sur la nouvelle forme et continuer de prouver que **chaque** valeur brute égale le relevé.

## Solution

Chaque constante du cœur s'écrit sous la forme d'**un entier 16.16 brut, recopié du bloc A, avec le littéral Lua d'origine à côté, dans le même appel de macro**. Par exemple, `0.27` devient `PICO8_FIXED_LIT(0x0000451e, 0.27)`. Le premier argument est la valeur : les chiffres hexa du relevé (`0x0000.451e`) sans le point. Le second est le littéral tel qu'il s'écrit dans `pico8/lua.lua`. Le compilateur ne l'évalue pas, mais un script le lit. Aucun flottant n'est calculé et aucun commentaire n'est ajouté, et le lien avec le cart et le relevé se lit d'un coup d'œil ou d'un `grep`.

Le travail se fait en quatre commits, dans cet ordre :

1. **L'attribution versionnée, d'abord.** On relance `make constant-variants` sur l'arbre actuel, qui est la dernière fois où le harnais de C2a a prise sur les constantes (il agit sur la forme `pico8_fixed_from_double(<littéral>)`). Sa synthèse est versionnée à côté des rapports de divergence. On obtient ainsi, par constante et par fichier d'événements, la première frame divergente et les plages.
2. **La forme change, et aucune valeur ne bouge.** Les 61 sites passent à la macro, chacun avec **la valeur que le C calcule aujourd'hui**. Pour les 18 littéraux justes, c'est déjà le relevé. Pour les 16 décalés, c'est la valeur arrondie actuelle. Le comparateur est rebranché pour lire la macro. La liste des 16 défaillances attendues **ne change pas**, et elle reste exacte : c'est la preuve que la forme n'a changé aucune valeur. `make frames-check` reste identique sur les 8 fichiers, sans re-baseline.
3. **Les 16 valeurs changent, en un commit séparé.** Ces 16 sites prennent l'hexa du relevé, et la liste des défaillances attendues est vidée. La suite est la suivante :
   - `make frames-report ETAPE=C2b` ;
   - revue headless des premières frames divergentes ;
   - attribution de chaque première frame divergente à une constante, d'après la synthèse versionnée du commit 1 ;
   - `make frames-baseline ETAPE=C2b`.

   Le commit porte le rapport, l'attribution et les références re-baselinées.
4. **La consignation.** On régénère inventaire, couverture, garde et observation. On ajoute un « État de C2b » à `docs/plan-2.md` et on met à jour la ligne de la table de propriété. On corrige `docs/trous-de-portage.md` (TROU-1), mais seulement pour les faits établis.

## User Stories

1. En tant qu'agent qui mène C2b, je veux que chaque constante du cœur soit la valeur brute du relevé, et non le résultat d'une conversion `double` → 16.16, pour que le port calcule exactement ce que calcule PICO-8 (§3.1, §9).
2. En tant qu'agent, je veux que les 61 sites passent tous à la forme brute, les 18 justes compris, pour qu'aucune constante ne dépende encore d'une règle d'arrondi, même quand cette règle tombe juste par hasard.
3. En tant que lecteur du code, je veux voir le littéral Lua d'origine à côté de chaque valeur brute, pour retrouver la ligne de `pico8/lua.lua` et la ligne du bloc A sans décoder de l'hexa.
4. En tant que mainteneur, je veux que ce littéral soit un argument de macro et non un commentaire, pour qu'un script puisse le lire et le vérifier et que la règle de `CLAUDE.md` sur les commentaires ne soit pas contournée.
5. En tant qu'agent, je veux que la valeur brute s'écrive avec les chiffres hexa du relevé, point retiré (`0x0000.451e` → `0x0000451e`), pour que la comparaison avec `tools/probe_out.txt` se fasse à l'œil et par script sur le même texte.
6. En tant qu'agent, je veux que la macro n'évalue jamais son second argument, pour qu'aucun flottant ne soit compilé ni calculé, même replié.
7. En tant qu'agent, je veux un test unitaire qui vérifie la valeur de la macro sur des cas du relevé (fraction seule, partie entière non nulle, bit de poids fort de la fraction à 1), pour qu'une erreur de cast dans la macro soit attrapée hors du filet.
8. En tant qu'agent, je veux changer la forme dans un commit où aucune valeur ne change, pour qu'une régression de la conversion de forme ne se cache pas derrière une divergence légitime (même raisonnement que la séparation C1/C2).
9. En tant qu'agent, je veux que les valeurs écrites au commit de forme soient tirées mécaniquement de la colonne « hexa C » du comparateur, et non tapées à la main, pour que « aucune valeur ne change » soit vrai par construction.
10. En tant qu'agent, je veux qu'après le commit de forme le comparateur donne une table identique à celle d'avant (mêmes valeurs C, mêmes statuts, mêmes comptes), et que la liste des 16 défaillances attendues reste exacte, pour que ce soit la preuve que la forme n'a changé aucune valeur.
11. En tant qu'agent, je veux que `make frames-check` soit identique sur les 8 fichiers après le commit de forme, sans `frames-baseline`, pour que le filet confirme la preuve au niveau valeur.
12. En tant qu'agent, je veux que `constants-check` lise les sites de la macro dans les sources du cœur, et plus seulement les jetons `constante` de l'inventaire, pour qu'il continue de vérifier les 61 sites après leur sortie de l'inventaire.
13. En tant qu'agent, je veux que le comparateur calcule la valeur de chaque site de macro en compilant le texte exact de l'appel, pour tester la macro telle que le cœur l'expand, comme C2a le fait pour `pico8_fixed_from_double`.
14. En tant qu'agent, je veux que le comparateur continue aussi de lire les jetons `constante` de l'inventaire, pour qu'un `pico8_fixed_from_double(0.27)` réintroduit plus tard soit vérifié et signalé en écart (régression), et non ignoré.
15. En tant qu'agent, je veux qu'un appel de macro mal formé (valeur qui n'est pas un hexa à 8 chiffres, littéral qui n'est pas un nombre) fasse échouer le comparateur, pour qu'aucun site n'échappe à la vérification par sa forme.
16. En tant qu'agent, je veux qu'un littéral de macro absent du bloc A fasse échouer le comparateur, pour qu'aucune constante ne soit ajoutée sans source.
17. En tant qu'agent, je veux que deux sites d'un même littéral avec deux valeurs différentes soient en écart, pour qu'une incohérence entre sites ne passe pas parce qu'un des deux est juste.
18. En tant qu'agent, je veux que le contrôle échoue toujours s'il ne vérifie aucune constante, toutes formes confondues, pour que le contrat de C2a tienne.
19. En tant qu'agent, je veux qu'après le commit de valeurs le comparateur imprime `litteraux 34 | sites 61 | ok 34 | ecart 0`, et que la liste des défaillances attendues soit vide, pour que la correction se lise sans interprétation.
20. En tant qu'agent, je veux relancer le harnais de C2a et versionner sa synthèse **avant** de changer la forme, pour que l'attribution repose sur un fichier du dépôt et non sur `tmp/`, et qu'on puisse la vérifier après coup.
21. En tant qu'agent, je veux que la synthèse versionnée soit comparée aux chiffres de l'« État de C2a » (0.1, 0.05, 0.001 divergent, les 13 autres non), pour qu'un changement survenu entre C2a et C2b soit un constat consigné et non une surprise.
22. En tant qu'agent, je veux que les 16 valeurs corrigées soient lues dans la colonne « relevé » du comparateur, c'est-à-dire dans le bloc A, pour que la valeur écrite soit exactement celle du relevé.
23. En tant qu'agent, je veux lancer `make frames-report ETAPE=C2b` avant tout re-baseline, pour relire le rapport complet de la correction combinée avant de décider.
24. En tant qu'agent, je veux rattacher, pour chaque fichier d'événements divergent, la première frame divergente à la seule constante dont la variante diverge la première, et exactement à cette frame, pour satisfaire la sortie de la Phase C.
25. En tant qu'agent, je veux que l'attribution échoue, sans être forcée, si la première frame combinée ne correspond à la première frame d'aucune variante, ou si deux variantes divergent ensemble à cette frame, ou si une variante diverge plus tôt, pour que toute interaction entre constantes soit consignée comme constat.
26. En tant qu'agent, je veux vérifier sans fenêtre que la première frame divergente combinée est **identique octet pour octet** à la même frame de la variante attribuée, pour que l'attribution ne repose pas que sur des numéros de frame.
27. En tant qu'agent, je veux extraire cette frame avant et après la correction (`pico8_frames frame`), comparer les 128 lignes et consigner le nombre de lignes et de pixels qui changent, pour que la revue des frames divergentes existe sous une forme qu'un agent sait produire.
28. En tant qu'agent, je veux que toutes les frames divergentes qui suivent la première soient étiquetées cascade, avec les nombres par variante en regard, pour que la sortie de phase soit remplie sans prétendre à une attribution frame par frame que le plan ne demande pas.
29. En tant qu'agent, je veux que les fichiers d'événements conformes le restent et figurent comme tels dans le rapport, pour qu'une absence de divergence se lise.
30. En tant qu'agent, je veux lancer `make frames-baseline ETAPE=C2b` seulement après l'attribution, pour que le rapport, l'attribution et les références re-baselinées tiennent dans le même commit, dans cet ordre.
31. En tant qu'agent, je veux que `make test` soit vert à la fin de chaque commit, pour ne jamais laisser la suite rouge entre deux commits.
32. En tant qu'agent, je veux régénérer l'inventaire, la couverture, la garde et l'observation après la correction et consigner leurs deltas, pour que la sortie s'énonce en « compte de constantes = 0 » (règle de B2).
33. En tant qu'agent, je veux consigner le delta de la garde tel qu'il est mesuré, même nul, pour que l'angle mort de x86_64 sur les arguments `double` ne soit pas pris pour un progrès ou une absence de progrès.
34. En tant que mainteneur, je veux un « État de C2b » dans `docs/plan-2.md` et la ligne des constantes de la table de propriété marquée faite, pour que D parte d'un état consigné.
35. En tant que mainteneur, je veux que TROU-1 ne soit corrigé que pour ce qui est établi (les littéraux ne passent plus par le flottant), et laissé ouvert pour `sin`/`cos` et `^0.75`, pour que le document des trous ne dise rien de faux.
36. En tant qu'humain, je veux que C2b ne lance jamais l'application raylib et ne pose jamais de signature `screencast-sign`, pour que la vérification reste headless et que la signature reste un acte humain (CLAUDE.md, `Makefile`).

## Implementation Decisions

- **La forme des constantes : une macro de valeur brute qui porte le littéral d'origine.**
  - Elle est définie dans l'en-tête du module fixe, sous la forme `PICO8_FIXED_LIT(raw, lua)` (le nom définitif est laissé à l'implémenteur, mais il ne contient ni `double` ni `float`, pour que l'inventaire et le grep du §6.1 ne le voient pas).
  - Elle s'expand en `raw` converti en `pico8_fixed_t` via `uint32_t`, sur le modèle de `PICO8_OB_NUMBER_MASK`. Elle **n'utilise pas** `lua` : le second argument n'est jamais compilé en expression.
  - `raw` s'écrit en hexa à 8 chiffres, minuscules, égal aux chiffres du bloc A sans le point.
  - `lua` est le littéral exactement tel qu'il s'écrit dans le bloc A et dans `pico8/lua.lua` : un jeton numérique nu, pas une chaîne, pour que `grep 0.27` trouve les trois fichiers.
  - Options écartées :
    - un entier brut nu : perd le lien au cart ;
    - un entier avec commentaire `/* 0.27 */` : hors des quatre destinations de commentaire de `CLAUDE.md`, et invérifiable par script ;
    - une constante nommée par valeur : 34 noms à inventer, sans gain ;
    - ne convertir que les 16 constantes décalées : contraire au texte du plan (« les 34 constantes remplacées ») et à la table de propriété (61 jetons en C2b), et les 18 restantes dépendraient encore d'une règle d'arrondi.
  - La macro est le **contrat** qui se dit par un commentaire de contrat C d'une ligne (ce que vaut chaque argument, le second jamais évalué), puisque son nom ne le dit pas.

- **Les valeurs du commit de forme, et §9.** Au commit de forme, les 16 sites décalés reçoivent la valeur que le C calcule aujourd'hui (l'arrondi), lue dans la colonne « hexa C » du comparateur. C'est l'unique moment où une valeur non relevée est écrite en dur. Deux raisons l'acceptent :
  - le comparateur la signale comme `ECART` et la liste des défaillances attendues la nomme, donc rien n'est caché ;
  - elle disparaît au commit suivant.

  En contrepartie, la forme et la valeur changent dans deux commits distincts, et le premier est prouvé neutre deux fois : même table du comparateur, et `frames-check` identique. La conversion des 61 sites est faite par un script ponctuel, non versionné, qui tire les valeurs du comparateur. Son principe va dans le message de commit, et le nombre de sites convertis par fichier doit égaler l'inventaire.

- **Le comparateur de constantes, rebranché (module profond, partagé par le contrôle et, en lecture, par le harnais).**
  - *Entrées* : il garde les quatre existantes (inventaire, relevé, module fixe, répertoire de travail) et reçoit en plus la liste des `.c` du cœur (`CORE_SRCS`, passée par le `Makefile`). Sans cette liste, il se comporte comme en C2a, et le harnais et ses tests restent inchangés.
  - *Sites de macro* : il cherche chaque occurrence de l'appel de macro dans les `.c` donnés. Une occurrence qui n'a pas la forme stricte (hexa à 8 chiffres, puis un littéral numérique) est une erreur qui nomme le fichier et la ligne. Le fichier d'en-tête qui définit la macro n'est pas scanné.
  - *Valeur C* : le programme engendré contient, pour chaque texte d'appel distinct, **l'appel tel qu'il est écrit dans la source**. Il est compilé contre le vrai en-tête et le vrai module fixe, comme pour `pico8_fixed_from_double`. On teste donc l'expansion réelle de la macro.
  - *Table* : le format reste celui de C2a, `<littéral> | <hexa relevé> | <hexa C> | ok|ECART | <sites>`, et la ligne des comptes aussi, pour que `constants_check.sh` et `constant_variants.sh` la lisent sans changement.
    - Un littéral présent sous les deux formes, ou sur plusieurs sites, occupe une seule ligne. Son nombre de sites additionne toutes les formes.
    - Il est `ECART` si **une** de ses valeurs diffère du relevé. La colonne « hexa C » montre alors les valeurs distinctes.
  - *Erreurs* : les erreurs de C2a restent (bloc A absent, littéral absent du bloc A, zéro constante, programme qui ne compile pas). « Zéro constante » compte maintenant les deux formes ensemble. S'y ajoute l'appel de macro mal formé.

- **Le contrôle des constantes.** Il ne change pas de logique : il reste dans `make test` et l'égalité « ensemble des `ECART` = liste » reste la règle. La cible `constants-check` passe `CORE_SRCS` au comparateur et prend ces sources, ainsi que l'en-tête du module fixe, en prérequis.
  - Au commit de forme, la liste garde ses 16 littéraux.
  - Au commit de valeurs, elle est vidée, en gardant son en-tête (`# TROU-1` compris tant que TROU-1 reste ouvert). Le contrôle doit alors imprimer `litteraux 34 | sites 61 | ok 34 | ecart 0`.

- **L'inventaire.** `tools/float_inventory.sh` n'est pas modifié. La macro ne contient aucun des jetons qu'il cherche, donc les 61 lignes `constante` disparaissent d'elles-mêmes. On régénère l'inventaire (`make float-inventory`) **dans chaque commit qui change la source**, sinon `float-inventory-check` fait échouer `make test`. Chiffres attendus après le commit de forme :
  - constantes 0 ;
  - casts 39 ;
  - réécrits 5 ;
  - autres 23 ;
  - total 67.

  Ces chiffres se mesurent et ne se recopient pas.

- **Le harnais d'attribution est gelé sur l'ancienne forme.** Il n'est pas adapté à la macro : sa seule utilité restante est l'attribution de C2b, qui se fait au commit 1, avant le changement de forme. Après C2b, la liste des `ECART` est vide et `make constant-variants` ne sélectionne plus rien. Ce comportement est consigné dans l'« État de C2b » comme une limite connue, pas corrigé. Son test sur arbre fabriqué reste vert, puisque le comparateur appelé sans liste de sources se comporte comme avant.

- **L'attribution versionnée.**
  - La synthèse de `make constant-variants` (16 variantes, passe complète, environ 4 minutes) est copiée dans le répertoire des rapports de divergence, sous un nom qui la rattache à l'étape (par exemple `C2b-variantes.txt`), à côté du futur `C2b.txt`.
  - Avant de la versionner, on vérifie qu'elle ne contient aucun chemin sous `tmp/` qui ne voudrait rien hors de la machine. Les noms de fichiers d'événements y sont relatifs au dépôt. Si des chemins de travail y figurent, la synthèse est versionnée telle quelle et le constat est noté : C2b ne modifie pas le harnais pour si peu.
  - Ses chiffres sont comparés à la table de l'« État de C2a ». Tout écart est un constat consigné avant de continuer.

- **La règle d'attribution, mécanique.** Pour chaque fichier d'événements, soit F la première frame divergente de `C2b.txt` :
  - Le fichier est rattaché à la constante `L` si et seulement si **exactement une** variante a pour première frame divergente F sur ce fichier, **et** aucune variante n'a de première frame inférieure à F.
  - Toutes les autres frames divergentes du fichier sont étiquetées **cascade**. La consignation met en regard le nombre de frames divergentes combiné et celui de chaque variante non nulle, pour information. Elle ne prétend pas rattacher les frames suivantes (l'effet de `0.1` à la frame 457 d'`events.json` tombe dans la cascade de `0.001`).
  - Si la règle échoue (aucune variante à F, deux à F, une plus tôt), on ne re-baseline pas. Le cas est consigné comme constat dans l'« État de C2b », et on repasse la main (voir Further Notes).
  - Attendu d'après l'« État de C2a », qui est une prédiction et non une consigne :
    - 5 fichiers rattachés à `0.001` : `events.json` à la frame 291, `phase4_playthrough` à 41, `built_planet_panel` à 41, `empty_planet_cleared` à 39, `slipway_drag` à 38 ;
    - 3 fichiers conformes.

  L'attribution est écrite dans l'« État de C2b ». Le rapport `C2b.txt` reste la sortie brute de `frames_report.sh`, qui n'est pas modifié.

- **La revue headless des frames divergentes.** Aucun humain ne regarde. Ce qu'un agent vérifie, pour chaque fichier divergent et à sa frame F :
  1. La frame F de la capture **d'avant** la correction (extraite avant l'édition des valeurs, depuis les captures courantes du filet), la frame F de la capture **d'après**, et la frame F de la capture de la variante attribuée (`tmp/constant_variants/lit-<L>/frames/…`), toutes par `pico8_frames frame`.
  2. `après` et `variante` doivent être **identiques** octet pour octet. C'est la preuve la plus forte que la frame F est l'effet de `L` seule.
  3. `avant` et `après` sont comparées ligne à ligne. On consigne le nombre de lignes différentes sur 128, le nombre de pixels différents, et la boîte englobante (lignes et colonnes min/max) des pixels changés.
  4. Rien de plus : décider que « ce pixel est juste » n'est pas du ressort d'un agent. Le filet reste une base de non-régression, pas un oracle (§5). La justesse vient du relevé, qui est vérifié au niveau valeur par `constants-check`.

  Ces chiffres sont écrits dans l'« État de C2b », une ligne par fichier. Les dumps restent sous `tmp/` et ne sont pas versionnés.

- **L'ordre au commit de valeurs.**
  1. Extraire les frames « avant » (après avoir lancé une fois `make frames-check` pour que les captures courantes existent).
  2. Éditer les 16 valeurs et vider la liste.
  3. `make constants-check` : vert, 0 écart.
  4. `make frames-report ETAPE=C2b`.
  5. Revue headless et attribution.
  6. `make frames-baseline ETAPE=C2b`, qui réécrit le même rapport puis copie les références. On vérifie que le rapport réécrit est identique à celui de l'étape 4.
  7. `make test` vert.

  Tout cela tient dans un seul commit : sources, liste vidée, inventaire, `C2b.txt`, références `*.frames.hash`.

- **La signature d'écran.** `events.json` diverge. Aujourd'hui `events.json.sha256` n'existe pas, donc `screencast-check` ne compare rien et reste vert. C2b ne crée pas ce fichier et ne lance pas `screencast-sign`, qui est un acte humain. Si le fichier existe au moment de l'implémentation, C2b s'arrête sur ce point et le consigne au lieu de re-signer.

- **La consignation.**
  - On régénère :
    - `make float-inventory` (déjà fait par commit) ;
    - `make float-coverage` : les 61 lignes `constante` disparaissent, et les totaux « couverts / non couverts » baissent de 38 et de 23 ;
    - `make float-guard`, avec son delta ;
    - `make float-observe` : casts seuls, attendu inchangé à 39.
  - Pour la garde, on consigne le delta mesuré. Il est probablement nul, puisqu'un argument `double` passé à une fonction externe ne fait pas échouer `-mgeneral-regs-only` sur x86_64 (angle mort de B2), et on le dit.
  - L'« État de C2b » est écrit sur le modèle des états C0, C1 et C2a. Il contient :
    - la forme retenue ;
    - les deux preuves du commit de forme ;
    - les comptes du comparateur ;
    - l'attribution par fichier, la revue headless par fichier et les nombres de cascade ;
    - les deltas d'inventaire, de couverture, de garde et d'observation ;
    - la limite du harnais gelé ;
    - le fait que 13 des 16 corrections ne sont vues par aucune frame du filet et ne sont protégées que par `constants-check`.
  - Dans la table de propriété, la ligne des constantes est marquée « C2b — ✅ faite ». La table reste la photographie de C0, comme C1 l'a fait.
  - TROU-1 est mis à jour sur les seuls faits : les littéraux décimaux ne passent plus par `pico8_fixed_from_double` et valent le relevé. `pico8_fixed_from_double` reste défini (décision ouverte) et sert encore à `^0.75`. `sin`/`cos` et `^0.75` restent non bit-exacts. TROU-1 **n'est pas refermé**. La liste « Fichiers » est ajustée seulement si un fichier n'est plus concerné : `pico8_game_data.c` y est cité pour `bq=0.0977`.
  - Un test existant qui vérifierait une valeur calculée avec l'ancienne constante n'est pas contourné. Une référence recalculée doit citer le relevé.

- **Commentaires** selon `CLAUDE.md` :
  - le contrat de la macro, en une ligne ;
  - le contrat du nouvel argument du comparateur ;
  - l'aide à la lecture de la commande modifiée du `Makefile`.

  Aucun commentaire par site. Le récit va dans les messages de commit.

## Implementation Chunks

1. **L'attribution versionnée.** `make constant-variants` en passe complète sur l'arbre actuel, et la synthèse est copiée dans le répertoire des rapports. Aucune source ne change. Critères d'acceptation :
   - les 16 variantes se construisent et se rejouent ;
   - la synthèse versionnée redonne les chiffres de l'« État de C2a », ou l'écart est consigné ;
   - `make test` est vert ;
   - `src/` est inchangé.
2. **La forme, sans changement de valeur.** Dans ce chunk :
   - la macro et son test unitaire de valeur ;
   - le comparateur qui lit les sites de macro, avec son test sur entrées fabriquées étendu ;
   - `constants-check` qui passe `CORE_SRCS` ;
   - la conversion des 61 sites aux valeurs actuelles ;
   - l'inventaire régénéré.

   Critères d'acceptation :
   - la table du comparateur est identique à celle d'avant (mêmes valeurs C et statuts, `litteraux 34 | sites 61 | ok 18 | ecart 16`) ;
   - la liste des défaillances attendues n'est pas modifiée ;
   - `make frames-check` est identique sur les 8 fichiers, sans `frames-baseline` ;
   - l'inventaire compte 0 constante ;
   - `grep pico8_fixed_from_double(` sur les `.c` du cœur ne trouve plus que les définitions, `ec_d` et `sin`/`cos` ;
   - `make test` est vert.
3. **Les 16 valeurs, le rapport, l'attribution et le re-baseline.** Suivre l'ordre de la section précédente. Critères d'acceptation :
   - `constants-check` imprime `ok 34 | ecart 0` avec une liste vide ;
   - `tools/frames_reports/C2b.txt` est versionné ;
   - chaque fichier divergent est rattaché à une seule constante par la règle, et sa frame F est identique à celle de la variante ;
   - les références sont re-baselinées ;
   - `make test` est vert.

   Si la règle échoue, le chunk s'arrête avant le re-baseline.
4. **La consignation.**
   - Couverture, garde et observation régénérées.
   - « État de C2b », ligne de la table de propriété, TROU-1.

   Critères d'acceptation :
   - chaque chiffre de l'état est lu dans un fichier régénéré ou versionné ;
   - `make test` est vert.

## Testing Decisions

- Un bon test porte sur ce que l'outil rend (une table, un code de sortie, une valeur) à partir d'entrées qu'il fabrique, et exerce d'abord ce que l'outil doit empêcher. Précédents : `tests/test_constants_check.c` (comparateur et contrôle sur inventaire, relevé et module fixe fabriqués ou réels), `tests/test_pico8_fixed.c`.
- **La macro** (dans le test du module fixe) :
  - valeur d'une fraction seule (`0x0000451e` = 17694) ;
  - partie entière non nulle (`0x000a8000`) ;
  - bit de poids fort de la fraction à 1 (`0x0000fd70`) ;
  - égalité avec `pico8_fixed_from_double` sur un littéral exact (`0.5`), pour ancrer les deux formes sur la même échelle.
- **Le comparateur rebranché** (extension du test existant, sur des sources fabriquées) :
  - un site de macro juste donne `ok` ;
  - un site de macro faux d'un LSB donne `ECART` ;
  - deux sites du même littéral, un juste et un faux, donnent `ECART` et comptent pour 2 sites ;
  - un littéral présent sous les deux formes occupe une ligne et additionne ses sites ;
  - un `pico8_fixed_from_double(<l>)` décalé resté dans l'inventaire donne toujours `ECART` ;
  - une macro mal formée (hexa à 7 chiffres, second argument non numérique) donne une erreur qui nomme le fichier ;
  - un littéral de macro absent du bloc A donne une erreur ;
  - aucune constante sous aucune forme donne une erreur ;
  - sans liste de sources, la sortie est celle de C2a (non-régression du harnais).
- **Contrôle** : les cas de C2a restent (liste périmée, régression, listé absent). Leur précédent reste valable, et l'un d'eux est rejoué avec des sites de macro.
- **Le harnais** : aucun changement, son test fabriqué doit rester vert.
- **Pas de test automatique** de la conversion des 61 sites ni de l'attribution. Ce sont des actes ponctuels. Leur preuve est la table du comparateur (identique au chunk 2, `ecart 0` au chunk 3), `frames-check`, et la règle d'attribution appliquée et consignée.
- `make test` est complet et vert à la fin de chaque chunk. `frames-check` est identique aux chunks 1 et 2. Au chunk 3, un seul `frames-baseline ETAPE=C2b`, précédé de son rapport.

## Out of Scope

- **Le volet flottant hors constantes** :
  - les 22 casts D1 et les 17 « à auditer » ;
  - `tostr` (D2), `^0.75` et `pico8_fixed_from_double(ec_d)` (D3) ;
  - les clés de table (D4).
- **`pico8_fixed_from_double` et `_to_double` eux-mêmes**, et `sin`/`cos`/`rnd`. Ils relèvent de la décision ouverte de la table de propriété et ne sont ni supprimés ni réécrits. Les tests qui les utilisent (`tests/test_pico8_fixed.c`…) sont hors cœur (§4) et restent tels quels.
- **L'adaptation du harnais d'attribution** à la nouvelle forme, ainsi que des variantes par site ou combinées.
- **La modification de `frames_check.sh`, de `frames_report.sh`** ou du format des rapports.
- **Une attribution frame par frame au-delà de la première** : le plan demande la première, et la suite est de la cascade.
- **La signature `screencast-sign`** et la création de `events.json.sha256`.
- **La garde `mipsel` soft-float** du §6.3.
- **Le volet `malloc`** (§8).
- **Lancer l'application raylib.**

## Further Notes

- Les chiffres de C2a laissent prévoir une attribution simple. Seul `0.001` (site `pico8_g__mm`, `pico8_probe.c`) est premier sur les 5 fichiers divergents, et ses premières frames (291, 41, 41, 39, 38) précèdent toujours celles de `0.1` (457, 75, 75, 75, 77) et de `0.05` (2373). Une frame F combinée différente de ces chiffres signalerait une interaction entre constantes. La règle ne la force pas.
- Si la règle échoue au chunk 3, le fichier concerné n'est pas re-baseliné par défaut. Les options sont les suivantes, et aucune n'est choisie ici :
  - une variante « toutes sauf L » ;
  - une variante par paire ;
  - une consignation de l'interaction comme constat accepté par l'humain.
- 13 des 16 corrections ne font diverger aucune frame du filet. Pour elles, `constants-check` est la seule protection. Le rebranchement du comparateur est donc la pièce qui porte la correction, pas un détail d'outillage.
- Le site de `0.001` est `NON COUVERT` dans la couverture (granularité à la ligne), alors qu'il fait diverger 5 fichiers. Ce constat de C2a ne change rien ici, mais il rappelle que la couverture ne suffit pas à dire qu'une constante est hors d'atteinte.
- La règle « `pico8_fixed_from_double(<littéral>)` interdit dans le cœur » n'a pas besoin d'un nouveau garde-fou. Un tel appel réapparaîtrait comme jeton `constante` de l'inventaire (`float-inventory-check` échoue tant que la copie versionnée n'est pas régénérée), puis serait vérifié par le comparateur.
